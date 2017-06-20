/*
 * $Id: policer.c,v 1.18 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * Module: Policer management
 */

#include <shared/bsl.h>

#include <soc/drv.h>

#include <shared/bsl.h>

#include <soc/sbx/sbx_drv.h>
#include <soc/sbx/hal_ca_auto.h>
#include <soc/sbx/hal_c2_auto.h>
#include <soc/sbx/hal_ca_c2.h>

#ifdef BCM_CALADAN3_G3P1_SUPPORT
#include <soc/sbx/g3p1/g3p1_int.h>
#include <soc/sbx/g3p1/g3p1_defs.h>
#endif

#include <shared/idxres_fl.h>
#include <shared/idxres_afl.h>

#include <bcm/error.h>
#include <bcm/types.h>
#include <bcm/cosq.h>
#include <bcm/policer.h>

#include <soc/sbx/wb_db_cmn.h>

#include <bcm_int/sbx/caladan3/policer.h>
#include <bcm_int/sbx/caladan3/stat.h>
#include <bcm_int/sbx/stat.h>
#include <bcm_int/sbx/error.h>
#include <bcm_int/sbx/caladan3/wb_db_policer.h>


#define POLICER_ID_INVALID(unit, id) \
  (((id) < 0) || ((id) > _bcm_caladan3_policer_max_id_get((unit))))


#define POLICER_UNIT_LOCK_nr(unit, rv) \
do { \
    if (sal_mutex_take(_policer[unit].lock, sal_mutex_FOREVER)) { \
        LOG_ERROR(BSL_LS_BCM_POLICER, \
                  (BSL_META_U(unit, \
                              "%s: sal_mutex_take failed for unit %d. \n"), \
                   FUNCTION_NAME(), unit)); \
        (rv) = BCM_E_INTERNAL; \
    } \
} while (0)

#define POLICER_UNIT_LOCK(unit) \
do { \
    int pol__rv__ = BCM_E_NONE; \
    POLICER_UNIT_LOCK_nr(unit, pol__rv__);  \
    if (BCM_FAILURE(pol__rv__)) { return pol__rv__; } \
} while (0)

#define POLICER_UNIT_UNLOCK_nr(unit, rv) \
do { \
    if (sal_mutex_give(_policer[unit].lock)) { \
        LOG_ERROR(BSL_LS_BCM_POLICER, \
                  (BSL_META_U(unit, \
                              "%s: sal_mutex_give failed for unit %d. \n"), \
                   FUNCTION_NAME(), unit)); \
        (rv) = BCM_E_INTERNAL; \
    } \
} while (0)

#define POLICER_UNIT_UNLOCK(unit) \
do { \
    int pol__rv__ = BCM_E_NONE; \
    POLICER_UNIT_UNLOCK_nr(unit, pol__rv__);  \
    if (BCM_FAILURE(pol__rv__)) { return pol__rv__; } \
} while (0)

#define POLICER_UNIT_CHECK(unit) \
do { \
    if (!((unit) >= 0 && ((unit) < (BCM_MAX_NUM_UNITS)))) { \
        LOG_ERROR(BSL_LS_BCM_POLICER, \
                  (BSL_META_U(unit, \
                              "%s: Invalid unit \n"),   \
                   FUNCTION_NAME())); \
        return BCM_E_UNIT; \
    } \
} while (0)

#define POLICER_INIT_CHECK(unit) \
do { \
    POLICER_UNIT_CHECK(unit); \
    if (!_policer[unit].lock) { \
        LOG_ERROR(BSL_LS_BCM_POLICER, \
                  (BSL_META_U(unit,  \
                              "%s: Policers unitialized on unit:%d \n"), \
                   FUNCTION_NAME(), unit)); \
        return BCM_E_INIT; \
    } \
} while (0)

#define POLICER_NULL_PARAM_CHECK(_param) \
    if ((_param) == NULL) { return BCM_E_PARAM; }

#define KBITS_TO_BYTES(kbits) ((kbits)*125)
#define BYTES_TO_KBITS(bytes) (((bytes) <= 0) ? 0 : (1 + (((bytes)-1)*8)/1000))

/* Macros to overload info of whether this policer is the base policer or not
   on group_mode member of _bcm_policer_node_t */
#define BASE_POLICER_ID_SIGNATURE 0xBA000000
#define IS_BASE_POLICER(mode) (((mode) & BASE_POLICER_ID_SIGNATURE) \
                               == (BASE_POLICER_ID_SIGNATURE))
#define BASE_POLICER_SET(mode) ((mode) | BASE_POLICER_ID_SIGNATURE)
#define GROUP_MODE_GET(mode) ((mode) & (~BASE_POLICER_ID_SIGNATURE))

#define POLICER_GROUP_MODE_GET(unit, pol_id) \
        ((GROUP_MODE_GET(_policer[(unit)].policers[(pol_id)].group_mode)))


#ifdef BROADCOM_DEBUG
#define POLICER_TIMESTAMP(_t) (_t) = sal_time_usecs();
#else
#define POLICER_TIMESTAMP(_t) 
#endif


/* internal SW state is required for traverse & config get API */
static _bcm_policer_glob_t _policer[BCM_MAX_NUM_UNITS]; /* glob policer data */
static sal_mutex_t         _policer_glob_lock;          /* glob policer lock */


static int _g3p1_policer_group_num_policers[] = {
                        1,  /* bcmPolicerGroupModeSingle */
                        -1, /* bcmPolicerGroupModeTrafficType - Unsupported */
                        -1, /* bcmPolicerGroupModeDlfAll - Unsupported */
                        -1, /* bcmPolicerGroupModeDlfIntPri - Unsupported*/
                        4,  /* bcmPolicerGroupModeTyped */
                        5,  /* bcmPolicerGroupModeTypedAll */
                        -1,  /* bcmPolicerGroupModeTypedIntPri 
                               based on configured cos levels */
                        2,  /* bcmPolicerGroupModeSingleWithControl */
                        -1, /* bcmPolicerGroupModeTrafficTypeWithControl - Unsupported */
                        -1, /* bcmPolicerGroupModeDlfAllWithControl - Unsupported */
                        -1, /* bcmPolicerGroupModeDlfIntPriWithControl - Unsupported*/
                        5,  /* bcmPolicerGroupModeTypedWithControl */
                        6,  /* bcmPolicerGroupModeTypedAllWithControl */
                        -1  /* bcmPolicerGroupModeTypedIntPriWithControl 
                               based on configured cos levels */
                       };


#ifdef BCM_WARM_BOOT_SUPPORT
extern bcm_caladan3_wb_policer_state_scache_info_t
    *_bcm_caladan3_wb_policer_state_scache_info_p[BCM_MAX_NUM_UNITS];
#endif


/*
 * Function
 *      _bcm_caladan3_policer_unit_lock
 * Purpose
 *      Lock the top level policer state for this unit 
 * Parameters
 *      (in) unit       = unit number
 * Returns
 *      bcm_error_t = BCM_E_NONE if no error
 *                    BCM_E_* appropriately if not  
 */
int
_bcm_caladan3_policer_unit_lock(unit)
{
    POLICER_UNIT_CHECK(unit);
    POLICER_UNIT_LOCK(unit);
    return BCM_E_NONE;
}

/*
 * Function
 *      _bcm_caladan3_policer_unit_unlock
 * Purpose
 *      Unlock the top level policer state for this unit 
 * Parameters
 *      (in) unit       = unit number
 * Returns
 *      bcm_error_t = BCM_E_NONE if no error
 *                    BCM_E_* appropriately if not  
 */
int
_bcm_caladan3_policer_unit_unlock(unit)
{
    POLICER_UNIT_CHECK(unit);
    POLICER_UNIT_UNLOCK(unit);
    return BCM_E_NONE;
}

/*
 * Function
 *      _bcm_caladan3_policer_group_num_get
 * Purpose
 *      Get the number of policers in the group mode
 * Parameters
 *      (in) unit       = unit number
 *      (in) mode       = group mode
 *      (out) npolicers = number of policers
 * Returns
 *      bcm_error_t = BCM_E_NONE if no error
 *                    BCM_E_* appropriately if not 
 */
int
_bcm_caladan3_policer_group_num_get(int unit, bcm_policer_group_mode_t mode, 
                                  uint32 *npolicers)
{
    int rv = BCM_E_NONE;
    int num;

    switch (SOC_SBX_CONTROL(unit)->ucodetype) {
#ifdef BCM_CALADAN3_G3P1_SUPPORT
    case SOC_SBX_UCODE_TYPE_G3P1:
        num = _g3p1_policer_group_num_policers[mode];

        if (mode == bcmPolicerGroupModeTypedIntPri) {
            /* based on current cos configuration */
            num = (NUM_COS(unit) + 4);
        } else if (mode == bcmPolicerGroupModeTypedIntPriWithControl) {
            /* based on current cos configuration */
            num = (NUM_COS(unit) + 5);
        }
        break;
#endif /* BCM_CALADAN3_G3P1_SUPPORT */
    default:
        SBX_UNKNOWN_UCODE_WARN(unit);
        return BCM_E_INTERNAL;
    }

    if (num < 0) {
        /* Unsupported mode specified */
        rv = BCM_E_PARAM;
    } else {
        *npolicers = (uint32) num;
    }

    return rv;
}

/*
 * Function
 *      _bcm_caladan3_policer_get_hw
 * Purpose
 *      Retrieve the policer configuration from hardware
 * Parameters
 *      (in) unit       = unit number
 *      (in) policer_id = policer id
 *      (out) pcfg      = policer configuration
 * Returns
 *   BCM_E_*
 */
int
_bcm_caladan3_policer_get_hw(int unit, int policer_id, 
                           bcm_policer_config_t *pcfg,
                           uint32 *profile_id)
{
    int                             rv = BCM_E_NONE;
    soc_sbx_g3p1_policer_segment_t  segment;
    soc_sbx_g3p1_policer_config_t   soc_config;


    /* Get the policer segment */
    if (pcfg->flags & BCM_POLICER_EGRESS) {
        segment = COP_POLICER_SEGMENT_EGRPOL;
    } else {
        if (policer_id >= 1 && policer_id <= BCM_CALADAN3_XT_POLICERS) {
            segment = COP_POLICER_SEGMENT_XTPOL;
        } else {
            segment = COP_POLICER_SEGMENT_INGPOL;
        }
    }

    /* Read policer */
    rv = soc_sbx_g3p1_policer_read(unit, segment, policer_id, &soc_config);
    if (rv != BCM_E_NONE) {
        if (!SOC_WARM_BOOT(unit) || rv != BCM_E_NOT_FOUND) {
            LOG_ERROR(BSL_LS_BCM_POLICER,
                      (BSL_META_U(unit,
                                  "%s: soc_sbx_g3p1_policer_read failed: %u\n"),
                       FUNCTION_NAME(),policer_id));
        }
        return rv;
    }

    /* common conversions first */
    bcm_policer_config_t_init(pcfg);
    pcfg->ckbits_sec   = soc_config.cir;
    pcfg->ckbits_burst = BYTES_TO_KBITS(soc_config.cbs);
    pcfg->pkbits_sec   = soc_config.eir;
    pcfg->pkbits_burst = BYTES_TO_KBITS(soc_config.ebs);

    switch (soc_config.mode) 
    {
    case COP_POLICER_MODE_RFC_2697:
        pcfg->mode = bcmPolicerModeSrTcm;
        break;
    case COP_POLICER_MODE_RFC_2698:
        if (soc_config.cbs_non_decrement && soc_config.ebs_non_decrement) {
            pcfg->mode = bcmPolicerModePassThrough;
        } else {
            pcfg->mode = bcmPolicerModeTrTcm;
        }
        break;
    case COP_POLICER_MODE_RFC_4115:
        if (soc_config.coupling) {
            pcfg->mode = bcmPolicerModeCoupledTrTcmDs;
        } else {
            pcfg->mode = bcmPolicerModeTrTcmDs;
        }
        break;
    default:
        LOG_ERROR(BSL_LS_BCM_POLICER,
                  (BSL_META_U(unit,
                              "Invalid policer mode: %d \n"),
                   soc_config.mode));
        return BCM_E_CONFIG;
    }

    pcfg->flags |= soc_config.ignore_packet_color ? BCM_POLICER_COLOR_BLIND : 0;
    pcfg->flags |= soc_config.drop_on_red ? BCM_POLICER_DROP_RED : 0;

    *profile_id = 0;    /* Get rid of this? */

    
    LOG_VERBOSE(BSL_LS_BCM_POLICER,
                (BSL_META_U(unit,
                            "Read policer id %d:\n"),
                 policer_id));
    LOG_VERBOSE(BSL_LS_BCM_POLICER,
                (BSL_META_U(unit,
                            "rfcMode=%d cbs=%d cir=%d ebs=%d eir=%d\n"), 
                 soc_config.mode, soc_config.cbs, soc_config.cir,
                 soc_config.ebs, soc_config.eir));
    LOG_VERBOSE(BSL_LS_BCM_POLICER,
                (BSL_META_U(unit,
                            "blind=%d dor=%d coupling=%d cbsNoDecr=%d ebsNoDecr=%d\n"),
                 soc_config.ignore_packet_color, soc_config.drop_on_red,
                 soc_config.coupling, soc_config.cbs_non_decrement,
                 soc_config.ebs_non_decrement));
    LOG_VERBOSE(BSL_LS_BCM_POLICER,
                (BSL_META_U(unit,
                            "Converts to bcm_policer_confit_t:\n")));
    LOG_VERBOSE(BSL_LS_BCM_POLICER,
                (BSL_META_U(unit,
                            "ckbits_sec=%d ckbits_burst=%d pkbits_sec=%d pkbits_burst=%d\n\n"),
                 pcfg->ckbits_sec, pcfg->ckbits_burst, 
                 pcfg->pkbits_sec, pcfg->pkbits_burst));

    return BCM_E_NONE;
}


/*
 * Function
 *      _bcm_caladan3_g3p1_policer_lp_decode
 * Purpose
 *      decode policer information from the given logical port
 * Parameters
 *      (in) unit       = unit number
 *      (in) lp         = g3p1 logical port
 *      (out) group_mode= configured policer group mode
 * Returns
 *   BCM_E_*
 */
int
_bcm_caladan3_g3p1_policer_lp_decode(int unit, soc_sbx_g3p1_lp_t *lp, 
                                   bcm_policer_group_mode_t *group_mode)
{
    int rv = BCM_E_NONE;

    if (lp->typedpolice) {
        *group_mode = bcmPolicerGroupModeTyped;
    } else if (lp->mef) {
        if (lp->mefcos) {
            if (lp->xtpolreplace) {
                *group_mode = bcmPolicerGroupModeTypedIntPriWithControl;
            } else {
                *group_mode = bcmPolicerGroupModeTypedIntPri;
            }
        } else {
            if (lp->xtpolreplace) {
                *group_mode = bcmPolicerGroupModeTypedAllWithControl;
	    } else {
                *group_mode = bcmPolicerGroupModeTypedAll;
	    }
        }
    } else {
        if (lp->xtpolreplace) {
            *group_mode = bcmPolicerGroupModeSingleWithControl;
	} else {
            *group_mode = bcmPolicerGroupModeSingle;
	}
    } 

    return rv;
}

/*
 * Function
 *      _bcm_caladan3_g3p1_policer_lp_program
 * Purpose
 *      Sets the lp struct with the relevant policer info.
 *      The counter information is also programmed.
 * Parameters
 *      (in) unit       = unit number
 *      (in) pol_id     = policer id
 *      (out) lp        = logical port struct which is programmed
 * Returns
 *      bcm_error_t = BCM_E_NONE if no error
 *                    BCM_E_* appropriately if not 
 * Note:
 *      This does not program the HW
 */
#ifdef BCM_CALADAN3_G3P1_SUPPORT
int
_bcm_caladan3_g3p1_policer_lp_program(int unit, bcm_policer_t pol_id,
                                    soc_sbx_g3p1_lp_t *lp)
{
    int                             rv = BCM_E_NONE;
    bcm_policer_group_mode_t        mode;
    uint32                        typedpolice=0, mef=0, mefcos=0, xtpolreplace=0;
    uint32                          typedcount=0, base_cntr=0;
    _bcm_policer_node_g3p1_cookie_t *cookie;

    if (!lp) {
        return BCM_E_PARAM;
    }

    if (pol_id) {
        /* non-zero policer id. check group mode */
        rv = _bcm_caladan3_policer_group_mode_get(unit, pol_id, &mode);

        if (rv == BCM_E_NONE) {
            switch (mode) {
            case bcmPolicerGroupModeSingleWithControl:
                xtpolreplace = 1;
		break;
            case bcmPolicerGroupModeTypedWithControl:
                typedpolice = 1;
                xtpolreplace = 1;
		break;
            case bcmPolicerGroupModeTypedIntPriWithControl:
                mefcos = 1; /* intentional fall thru */
            case bcmPolicerGroupModeTypedAllWithControl:
                mef = 1;
                xtpolreplace = 1;
                break;
            case bcmPolicerGroupModeSingle:
                break;
            case bcmPolicerGroupModeTyped:
                typedpolice = 1;
                break;
            case bcmPolicerGroupModeTypedIntPri:
                mefcos = 1; /* intentional fall thru */
            case bcmPolicerGroupModeTypedAll:
                mef = 1;
                break;
            default:
                rv = BCM_E_CONFIG;
            }
        }
    }

    POLICER_UNIT_LOCK(unit);
    cookie = (_bcm_policer_node_g3p1_cookie_t *)
              _policer[unit].policers[pol_id].cookie;
    if (cookie) {
        base_cntr = cookie->base_cntr;
        typedcount = typedpolice;
    }
    POLICER_UNIT_UNLOCK(unit);

    if (rv == BCM_E_NONE) {
        lp->policer = pol_id;
        lp->typedpolice = typedpolice;
        lp->mef = mef;
        lp->mefcos = mefcos;
        lp->counter = base_cntr;
        lp->typedcount = typedcount;
        lp->xtpolreplace = xtpolreplace;

        /* lp->updaterdp controls if a policer dp is picked up */
        lp->updaterdp = !!pol_id && mef;
        lp->updatefdp = lp->updaterdp;
    }

    return rv;
}
#endif

/*
 * Function
 *      _bcm_caladan3_policer_num_policers_get
 * Purpose
 *      Returns the max policers possible
 * Parameters
 *      (in) unit   = unit number
 * Returns
 *      int - max number of policers
 */
int
_bcm_caladan3_policer_num_policers_get(int unit)
{

    /* Note: currently only supporting ingress policers */
#if 1
    int     rv;
    uint32  ing_pol_count;

    if (soc_property_get(unit, spn_LRP_BYPASS, 1)) {
        rv = soc_sbx_g3p1_constant_get(unit, "num_ing_policers", &ing_pol_count);
        if (rv != BCM_E_NONE) {
            return -1;
        }
    } else {
        ing_pol_count = 64000;
    }

    return ing_pol_count;
#else
    uint32  ing_pol_count, egr_pol_count, exc_pol_count;
    int     rv;

    rv = soc_sbx_g3p1_constant_get(unit, "num_ing_policers", &ing_pol_count);
    if (rv != BCM_E_NONE) {
        return -1;
    }
    rv = soc_sbx_g3p1_constant_get(unit, "num_egr_policers", &egr_pol_count);
    if (rv != BCM_E_NONE) {
        return -1;
    }
    rv = soc_sbx_g3p1_constant_get(unit, "num_exc_policers", &exc_pol_count);
    if (rv != BCM_E_NONE) {
        return -1;
    }

    return ing_pol_count + egr_pol_count + exc_pol_count;
#endif
}

/*
 * Function
 *      _bcm_caladan3_g3p1_policer_stat_mem_get
 * Purpose
 *      Returns the offset at which the counter for a particular stat
 *      type exists. Also indicates if this a pkt or byte counter.
 * Parameters
 *      (in) unit       = unit number
 *      (in) grp_mode   = group mode to which the policer belongs
 *      (in) stat       = stat type which to look at
 *      (in) cos        = cos level. Only applicable for TypedIntPri mode
 *      (out)ctr_offset = offset at which the counter sits
 *      (out)pkt        = whether pkt or not
 * Returns
 *      bcm_error_t = BCM_E_NONE if no error
 *                    BCM_E_* appropriately if not
 */
int
_bcm_caladan3_g3p1_policer_stat_mem_get(int unit, 
                                      bcm_policer_group_mode_t grp_mode, 
                                      bcm_policer_stat_t stat,
                                      int cos,
                                      int *ctr_offset,
                                      int *pkt)
{
    int rv = BCM_E_NONE;
    
    
    switch (grp_mode) {
    /* There is no support for counting control, policers under WithControl are treated at par */
    case bcmPolicerGroupModeSingle:
    case bcmPolicerGroupModeSingleWithControl:
        switch (stat) {
        /* intentional fall thrus for bcmPolicerStat*Packets */
        case bcmPolicerStatPackets:     *pkt = 1;
        case bcmPolicerStatBytes:       *ctr_offset = 0; break;
        case bcmPolicerStatDropPackets: *pkt = 1;
        case bcmPolicerStatDropBytes:   *ctr_offset = 1; break;
        default:
            rv = BCM_E_PARAM;
        }
        break;
    case bcmPolicerGroupModeTyped:
    case bcmPolicerGroupModeTypedWithControl:
        switch (stat) {
        case bcmPolicerStatUnknownUnicastPackets:       *pkt = 1;
        case bcmPolicerStatUnknownUnicastBytes:         *ctr_offset = 0; break;
        case bcmPolicerStatDropUnknownUnicastPackets:   *pkt = 1;
        case bcmPolicerStatDropUnknownUnicastBytes:     *ctr_offset = 1; break;
        case bcmPolicerStatUnicastPackets:              *pkt = 1;
        case bcmPolicerStatUnicastBytes:                *ctr_offset = 2; break;
        case bcmPolicerStatDropUnicastPackets:          *pkt = 1;
        case bcmPolicerStatDropUnicastBytes:            *ctr_offset = 3; break;
        case bcmPolicerStatMulticastPackets:            *pkt = 1;
        case bcmPolicerStatMulticastBytes:              *ctr_offset = 4; break;
        case bcmPolicerStatDropMulticastPackets:        *pkt = 1;
        case bcmPolicerStatDropMulticastBytes:          *ctr_offset = 5; break;
        case bcmPolicerStatBroadcastPackets:            *pkt = 1;
        case bcmPolicerStatBroadcastBytes:              *ctr_offset = 6; break;
        case bcmPolicerStatDropBroadcastPackets:        *pkt = 1;
        case bcmPolicerStatDropBroadcastBytes:          *ctr_offset = 7; break;
        default:
            rv = BCM_E_PARAM;
        }
        break;
    case bcmPolicerGroupModeTypedAll:
    case bcmPolicerGroupModeTypedAllWithControl:
        switch (stat) {
        case bcmPolicerStatDropUnknownUnicastPackets:   *pkt = 1;
        case bcmPolicerStatDropUnknownUnicastBytes:     *ctr_offset = 0; break;
        case bcmPolicerStatDropUnicastPackets:          *pkt = 1;
        case bcmPolicerStatDropUnicastBytes:            *ctr_offset = 1; break;
        case bcmPolicerStatDropMulticastPackets:        *pkt = 1;
        case bcmPolicerStatDropMulticastBytes:          *ctr_offset = 2; break;
        case bcmPolicerStatDropBroadcastPackets:        *pkt = 1;
        case bcmPolicerStatDropBroadcastBytes:          *ctr_offset = 3; break;
        case bcmPolicerStatGreenPackets:                *pkt = 1;
        case bcmPolicerStatGreenBytes:                  *ctr_offset = 4; break;
        case bcmPolicerStatYellowPackets:               *pkt = 1;
        case bcmPolicerStatYellowBytes:                 *ctr_offset = 5; break;
        case bcmPolicerStatRedPackets:                  *pkt = 1;
        case bcmPolicerStatRedBytes:                    *ctr_offset = 6; break;
        default:
            rv = BCM_E_PARAM;
        }
        break;
    case bcmPolicerGroupModeTypedIntPri:
    case bcmPolicerGroupModeTypedIntPriWithControl:
        switch (stat) {
        case bcmPolicerStatDropUnknownUnicastPackets:   *pkt = 1;
        case bcmPolicerStatDropUnknownUnicastBytes:     *ctr_offset = 0; break;
        case bcmPolicerStatDropUnicastPackets:          *pkt = 1;
        case bcmPolicerStatDropUnicastBytes:            *ctr_offset = 1; break;
        case bcmPolicerStatDropMulticastPackets:        *pkt = 1;
        case bcmPolicerStatDropMulticastBytes:          *ctr_offset = 2; break;
        case bcmPolicerStatDropBroadcastPackets:        *pkt = 1;
        case bcmPolicerStatDropBroadcastBytes:          *ctr_offset = 3; break;
        case bcmPolicerStatGreenPackets:        *pkt = 1;
        case bcmPolicerStatGreenBytes:          *ctr_offset = 4 + cos*3; break;
        case bcmPolicerStatYellowPackets:       *pkt = 1;
        case bcmPolicerStatYellowBytes:         *ctr_offset = 5 + cos*3; break;
        case bcmPolicerStatRedPackets:          *pkt = 1;
        case bcmPolicerStatRedBytes:            *ctr_offset = 6 + cos*3; break;
        default:
            rv = BCM_E_PARAM;
        }
        break;
    default:
        rv = BCM_E_PARAM;
    }

    return rv;
}

/*
 * Function
 *      _bcm_caladan3_g3p1_policer_stat_get
 * Purpose
 *      read the ucode counter for the stat type of  policer/cos
 * Parameters
 *      (in) unit       = unit number
 *      (in) policer_id = policer id
 *      (in) stat       = stat type which to look at
 *      (in) cos        = cos level. Only applicable for TypedIntPri mode
 *      (in) use_cookie = whether to use counter from cookie
 *      (in) counter    = base counter when use_cookie = 0
 *      (in)clear       = whether to clear the value or not
 *      (out)val        = stat value
 *
 * Returns
 *      bcm_error_t = BCM_E_NONE if no error
 *                    BCM_E_* appropriately if not
 */
int
_bcm_caladan3_g3p1_policer_stat_get(int unit, bcm_policer_t policer_id, 
                                  bcm_policer_stat_t stat, int cos, 
                                  int use_cookie, uint32 counter,
                                  int clear, uint64 *val)
{
    int                             rv = BCM_E_NONE;
    bcm_policer_group_mode_t        grp_mode;
    _bcm_policer_node_g3p1_cookie_t *cookie;
    int                             ctr_offset, base_ctr = 0, pkts;
    soc_sbx_g3p1_turbo64_count_t    soc_val;
    
    grp_mode = POLICER_GROUP_MODE_GET(unit, policer_id);

    pkts = 0;
    rv = _bcm_caladan3_g3p1_policer_stat_mem_get(unit, grp_mode, stat, cos, 
                                               &ctr_offset, &pkts); 

    if (rv == BCM_E_NONE) {
        if (use_cookie) {
            cookie = (_bcm_policer_node_g3p1_cookie_t *)
                     _policer[unit].policers[policer_id].cookie;
            if (!cookie) {
                /* coverity [assigned_value] */
                rv = BCM_E_INTERNAL;
            } else {
                base_ctr = cookie->base_cntr;
            }
        } else {
            if (counter) {
                rv = BCM_E_INTERNAL;
                return rv;
            }
            base_ctr = counter;
        }
        rv = soc_sbx_g3p1_ingctr_read(unit, (base_ctr + ctr_offset), 1, clear, &soc_val);
    }

    if (rv == BCM_E_NONE) {
        if (pkts) {
            *val = soc_val.packets;
        } else {
            *val = soc_val.bytes;
        }
    }

    return rv;
}

/*
 * Function
 *      _bcm_caladan3_g3p1_policer_stat_set
 * Purpose
 *      set the ucode counter for the stat type of  policer/cos
 * Parameters
 *      (in) unit       = unit number
 *      (in) policer_id = policer id
 *      (in) stat       = stat type which to look at
 *      (in) cos        = cos level. Only applicable for TypedIntPri mode
 *      (in) use_cookie = whether to use counter from cookie
 *      (in) counter    = base counter when use_cookie = 0
 *      (in)val        = stat value to set to
 * Returns
 *      bcm_error_t = BCM_E_NONE if no error
 *                    BCM_E_* appropriately if not
 * NOTE
 *      Set to value of only zero is supported.
 */
int
_bcm_caladan3_g3p1_policer_stat_set(int unit, bcm_policer_t policer_id, 
                                  bcm_policer_stat_t stat, int cos,
                                  int use_cookie, uint32 counter,
                                  uint64 val)
{
    int     rv = BCM_E_NONE;
    uint64  temp;
    int     clear=1;

    /* only set to zero is supported */
    if (COMPILER_64_HI(val) || COMPILER_64_LO(val)) {
        return BCM_E_PARAM;
    }

    /* call the get with clear set to 1 */
    rv = _bcm_caladan3_g3p1_policer_stat_get(unit, policer_id, stat, cos,
                                           use_cookie, counter, clear,
                                           &temp);

    return rv;
}

/*
 * Function
 *      _bcm_caladan3_policer_max_id_get
 * Purpose
 *      Return max valid id
 * Parameters
 *      (in) unit       = unit number
 * Returns
 *      int - max possible policer id
 */
int
_bcm_caladan3_policer_max_id_get(int unit)
{
    return _bcm_caladan3_policer_num_policers_get(unit) - 1;
}

/*
 * Function
 *      _bcm_caladan3_policer_group_mode_get
 * Purpose
 *      Get the policer group mode for the given policer ID
 * Parameters
 *      (in) unit     = unit number
 *      (in) id       = id of policer mode to get
 *     (out) grp_mode = group mode
 * Returns
 *      bcm_error_t = BCM_E_NONE if no error
 *                    BCM_E_* appropriately if not
 */
int
_bcm_caladan3_policer_group_mode_get(int unit, bcm_policer_t id,
                                   bcm_policer_group_mode_t *grp_mode)
{
    _bcm_policer_node_t     *cur;
    int                      rv = BCM_E_NONE;

    POLICER_INIT_CHECK(unit);
    if (POLICER_ID_INVALID(unit, id)) {
        LOG_ERROR(BSL_LS_BCM_POLICER,
                  (BSL_META_U(unit,
                              "%s: invalid policer id %d. \n"),
                   FUNCTION_NAME(), id));
        return BCM_E_PARAM;
    }

    POLICER_UNIT_LOCK(unit);
    cur = &_policer[unit].policers[id];
    if (cur->pol_cfg == NULL) {
        rv = BCM_E_NOT_FOUND;
    }
    *grp_mode = GROUP_MODE_GET(cur->group_mode);
    POLICER_UNIT_UNLOCK(unit);

    return rv;
}

/* Function
 *      _bcm_caladan3_policer_id_check
 * Purpose
 *      To check if the specified policer id is valid
 * Parameters
 *      (in) unit   = unit number
 *      (in) id     = policer id to check
 * Returns
 *      bcm_error_t = BCM_E_NONE if its valid
 *                    BCM_E_RESOURCE if its invalid or already in use
 */
int
_bcm_caladan3_policer_id_check(int unit, bcm_policer_t id)
{
    int rv = BCM_E_RESOURCE;

    if (POLICER_ID_INVALID(unit, id)) {
        return rv;
    }

    if (id <= BCM_CALADAN3_RESERVED_POLICERS) {
        /* reserved policer id space */
        rv = shr_idxres_list_elem_state(_policer[unit].res_idlist, id);
    } else {
        rv = shr_aidxres_list_elem_state(_policer[unit].idlist, id);
    }

    return ((rv == BCM_E_NOT_FOUND) ? BCM_E_NONE: BCM_E_RESOURCE);
}

/* Function
 *      _bcm_caladan3_policer_id_assign
 * Purpose
 *      Assign a policer id
 * Parameters
 *      (in) unit   = unit number
 *      (out) id    = policer id
 * Returns
 *      bcm_error_t = BCM_E_NONE if no error
 *                    BCM_E_* appropriately if not
 */
int
_bcm_caladan3_policer_id_assign(int unit, bcm_policer_t *id)
{
    if (!id) {
        return BCM_E_PARAM;
    }
    /* always assign from non-reserved id space */
    return shr_aidxres_list_alloc(_policer[unit].idlist, (uint32 *) id);
}

/* Function
 *      _bcm_caladan3_policer_id_free
 * Purpose
 *      Return a policer id to available pool
 * Parameters
 *      (in) unit   = unit number
 *      (out) id    = policer id
 * Returns
 *      bcm_error_t = BCM_E_NONE if no error
 *                    BCM_E_* appropriately if not
 */
int
_bcm_caladan3_policer_id_free(int unit, bcm_policer_t id)
{
    if (id <= BCM_CALADAN3_RESERVED_POLICERS) {
        return shr_idxres_list_free(_policer[unit].res_idlist, id);
    } else {
        return shr_aidxres_list_free(_policer[unit].idlist, id);
    }
}

/* Function
 *      _bcm_caladan3_policer_id_reserve
 * Purpose
 *      Mark the specified id as used
 * Parameters
 *      (in) unit   = unit number
 *      (out) id    = policer id
 * Returns
 *      bcm_error_t = BCM_E_NONE if no error
 *                    BCM_E_* appropriately if not
 */
int
_bcm_caladan3_policer_id_reserve(int unit, bcm_policer_t id)
{
    if (id <= BCM_CALADAN3_RESERVED_POLICERS) {
        return shr_idxres_list_reserve(_policer[unit].res_idlist, id, id);
    }

    return shr_aidxres_list_reserve(_policer[unit].idlist, id, id);
}


/*  Function
 *      _bcm_caladan3_policer_state_alloc
 *  Purpose
 *      Allocate internal state storage
 *  Parameters
 *      (in) unit     = unit number
 *      (in) id       = policer id of policer
 *      (in) grp_mode = policer group mode of policer
 *  Returns
 *      bcm_error_t = BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 */
int
_bcm_caladan3_policer_state_alloc(int unit, bcm_policer_t id,
                         bcm_policer_group_mode_t grp_mode, int base_policer)
{
    int                     status = BCM_E_NONE;
    _bcm_policer_node_t     *cur;
#if defined(BCM_WARM_BOOT_SUPPORT)
    bcm_caladan3_wb_policer_state_scache_info_t *wb_info_ptr = NULL;

    wb_info_ptr = BCM_CALADAN3_POLICER_SCACHE_INFO_PTR(unit);
    if(wb_info_ptr == NULL) {
        LOG_ERROR(BSL_LS_BCM_POLICER,
                  (BSL_META_U(unit,
                              "Warm boot not initialized for unit %d \n"),
                   unit));
        return BCM_E_INTERNAL;
    }
#endif


    cur = &_policer[unit].policers[id];

    if (cur->pol_cfg) {
        LOG_ERROR(BSL_LS_BCM_POLICER,
                  (BSL_META_U(unit,
                              "%s: Policer with id (%d) already exists.\n"),
                   FUNCTION_NAME(), id));
        return BCM_E_PARAM;
    }
    /* insert at top of list */
    cur->prev = NULL;
    cur->next = _policer[unit].pol_head;
    cur->id = id;
    cur->group_mode = (base_policer ? BASE_POLICER_SET(grp_mode): grp_mode);
    cur->pol_cfg = (bcm_policer_config_t *)
                    sal_alloc(sizeof(bcm_policer_config_t), "pol cfg");

#if defined(BCM_WARM_BOOT_SUPPORT)
    if (!SOC_WARM_BOOT(unit)) {
        SBX_WB_DB_SYNC_VARIABLE_OFFSET(uint32, 1,
            wb_info_ptr->group_mode_offset + (id * sizeof(uint32)),
            cur->group_mode);
    }
#endif

    if (cur->pol_cfg) {
        if (_policer[unit].pol_head) {
            _policer[unit].pol_head->prev = cur;
        }
        _policer[unit].pol_head = cur;
        _policer[unit].pol_count++;
    } else {
        LOG_ERROR(BSL_LS_BCM_POLICER,
                  (BSL_META_U(unit,
                              "%s: sal_alloc failed \n"),
                   FUNCTION_NAME()));
        status = BCM_E_MEMORY;
    }

    return status;
}

/*  Function
 *      _bcm_caladan3_policer_state_set
 *  Purpose
 *      update the given policer id in internal software state
 *  Parameters
 *      (in) unit   = unit number
 *      (in) id     = policer id to update
 *      (in) pol_cfg= update policer with this config
 *  Returns
 *      bcm_error_t = BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 */
int
_bcm_caladan3_policer_state_set(int unit, bcm_policer_t id,
                       bcm_policer_config_t *pol_cfg)
{
    int                     status = BCM_E_NONE;
    _bcm_policer_node_t     *cur;
#if defined(BCM_WARM_BOOT_SUPPORT)
    bcm_caladan3_wb_policer_state_scache_info_t *wb_info_ptr = NULL;

    wb_info_ptr = BCM_CALADAN3_POLICER_SCACHE_INFO_PTR(unit);
    if(wb_info_ptr == NULL) {
        LOG_ERROR(BSL_LS_BCM_POLICER,
                  (BSL_META_U(unit,
                              "%s: Warm boot not initialized for unit.\n"),
                   FUNCTION_NAME()));
        return BCM_E_INTERNAL;
    }
#endif

    if (!pol_cfg) {
        LOG_ERROR(BSL_LS_BCM_POLICER,
                  (BSL_META_U(unit,
                              "%s: pol_cfg NULL.\n"),
                   FUNCTION_NAME()));
        return BCM_E_PARAM;
    }

    cur = &_policer[unit].policers[id];

    if (cur->pol_cfg == NULL) {
        LOG_ERROR(BSL_LS_BCM_POLICER,
                  (BSL_META_U(unit,
                              "%s: Policer with id (%d) not found.\n"),
                   FUNCTION_NAME(), id));
        return BCM_E_PARAM;
    }

    sal_memcpy(cur->pol_cfg, pol_cfg, sizeof(bcm_policer_config_t));

#if defined(BCM_WARM_BOOT_SUPPORT)
    if (!SOC_WARM_BOOT(unit)) {
        SBX_WB_DB_SYNC_VARIABLE_OFFSET(uint32, 1,
            wb_info_ptr->flags_offset + (id * sizeof(uint32)),
            pol_cfg->flags);
    }

#endif
    return status;

}

/*  Function
 *      _bcm_caladan3_policer_state_remove
 *  Purpose
 *      Remove the given policer id in internal software state
 *  Parameters
 *      (in) unit   = unit number
 *      (in) id     = policer id to remove
 *  Returns
 *      bcm_error_t = BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 * Notes:
 *   assumes - lock is taken, id & unit are valid
 */
int
_bcm_caladan3_policer_state_remove(int unit, bcm_policer_t id)
{
    int                     status = BCM_E_NONE;
    int                     idx;
    int                     success = 0;
    _bcm_policer_node_t     *cur;
    _bcm_policer_node_t     *prev;
    _bcm_policer_node_t     *next;

    cur = _policer[unit].pol_head;
    for (idx = 0; ((idx < _policer[unit].pol_count) && (cur)); idx++) {
        if (cur->id == id) {
            /* found the node to delete */
            next = cur->next;
            prev = cur->prev;
            if (prev) {
                prev->next = next; /* update the next of prev node */
            } else {
                _policer[unit].pol_head = next;
            }

            if (next) {
                next->prev = prev; /* update the prev of next node */
            }

            if (cur->pol_cfg) {
                sal_free(cur->pol_cfg);
            }
            sal_memset(cur, 0, sizeof(_bcm_policer_node_t));
            _policer[unit].pol_count--;
            success = 1;
            break;
        }
        cur = cur->next;
    }

    if (!success) {
        LOG_ERROR(BSL_LS_BCM_POLICER,
                  (BSL_META_U(unit,
                              "%s: failed for policer id %d. \n"),
                   FUNCTION_NAME(), id));
        status = BCM_E_PARAM;
    }

    return status;
}


/*
 * Function
 *      _bcm_caladan3_policer_hw_create
 * Purpose
 *      Map bcm policer config to SB policer config, commit to hardware
 * Parameters
 *      (in) unit       = unit number
 *      (in) policer_id = location to commit policer config
 *      (in) pol_cfg    = policer config to commit
 * Returns
 *      bcm_error_t = BCM_E_NONE if no error
 *                    BCM_E_* appropriately if not
 * Note:
 *      Assumes unit, policer_id and policer config are valid
 */
int
_bcm_caladan3_policer_hw_create(int unit, uint32 policer_id,
                       bcm_policer_config_t *pol_cfg)
{
    int                             rv;
    soc_sbx_g3p1_policer_config_t   config;
    soc_sbx_g3p1_policer_segment_t  segment;
    uint32                          policer;

    soc_sbx_g3p1_policer_config_init(&config);

    /* Set the policer segment */
    if (pol_cfg->flags & BCM_POLICER_EGRESS) {
        segment = COP_POLICER_SEGMENT_EGRPOL;
    } else {
        if (policer_id >= 1 && policer_id <= BCM_CALADAN3_XT_POLICERS) {
            segment = COP_POLICER_SEGMENT_XTPOL;
        } else {
            segment = COP_POLICER_SEGMENT_INGPOL;
        }
    }
    
    config.policer_id = policer_id;
    if (pol_cfg->flags & BCM_POLICER_COLOR_BLIND) {
        config.ignore_packet_color = TRUE;
    }
    if (pol_cfg->flags & BCM_POLICER_DROP_RED) {
        config.drop_on_red = TRUE;
    }
    switch (pol_cfg->mode) {
        case bcmPolicerModeSrTcm:
            config.mode = COP_POLICER_MODE_RFC_2697;
            break;
        case bcmPolicerModeTrTcm:
            config.mode = COP_POLICER_MODE_RFC_2698;
            break;
        case bcmPolicerModeTrTcmDs:
            config.mode = COP_POLICER_MODE_RFC_4115;
            break;
        case bcmPolicerModePassThrough:
            config.mode = COP_POLICER_MODE_RFC_2698;
            config.cbs_non_decrement = TRUE;
            config.ebs_non_decrement = TRUE;
            break;
        /* These modes need work */
        case bcmPolicerModeCommitted:
        case bcmPolicerModePeak:
        case bcmPolicerModeGreen:
        case bcmPolicerModeSrTcmModified:
        case bcmPolicerModeCoupledTrTcmDs:
        case bcmPolicerModeCascade:
        case bcmPolicerModeCoupledCascade:
        case bcmPolicerModeCount:
        default:
            config.mode = COP_POLICER_MODE_RFC_2697;
    }
    config.cir = pol_cfg->ckbits_sec;
    config.cbs = KBITS_TO_BYTES(pol_cfg->ckbits_burst);
    config.eir = pol_cfg->pkbits_sec;
    config.ebs = KBITS_TO_BYTES(pol_cfg->pkbits_burst);

    rv = soc_sbx_g3p1_policer_create(unit, segment, &config, &policer);
    if (rv != BCM_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_POLICER,
                  (BSL_META_U(unit,
                              "%s: soc_sbx_g3p1_policer_create failed: %u\n"),
                   FUNCTION_NAME(),policer));
    }

    return rv;
}

/*
 * Function
 *      _bcm_caladan3_is_stat_enabled
 * Purpose
 *      check if stats are enabled on a policer
 * Parameters
 *      (in) unit           = unit number
 *      (in) policer_id     = policer id to check
 *      (out) monitor_id    = if enabled...monitor id being used
 * Returns
 *      int                 = 0 if not enabled
 *                            1 if enabled
 */
int
_bcm_caladan3_is_stat_enabled(int unit, bcm_policer_t policer_id, 
                            int *monitor_id)
{
    int idx = 0;

    if (SOC_IS_SBX_G3P1(unit) &&
        IS_BASE_POLICER(_policer[unit].policers[policer_id].group_mode) &&
        _policer[unit].policers[policer_id].cookie) {
        /* G2P3 supports ucode policer stats */
        return 1;
    } else {
        /* default to HW monitors */
        for (idx=0; idx<BCM_CALADAN3_NUM_MONITORS; idx++) {
            if (_policer[unit].mon_use_map[idx] == policer_id) {
                *monitor_id = idx;
                return 1;
            }
        }
    }

    return 0;
}

/*
 * Function
 *      _bcm_caladan3_is_monitor_stat_enabled
 * Purpose
 *      check if monitor is attached to this policer
 * Parameters
 *      (in) unit           = unit number
 *      (in) policer_id     = policer id to check
 *      (out) monitor_id    = if enabled...monitor id being used
 * Returns
 *      int                 = 0 if not enabled
 *                            1 if enabled
 */
int
_bcm_caladan3_is_monitor_stat_enabled(int unit, bcm_policer_t policer_id, 
                                    int *monitor_id)
{
    int idx = 0;

    for (idx=0; idx<BCM_CALADAN3_NUM_MONITORS; idx++) {
        if (_policer[unit].mon_use_map[idx] == policer_id) {
            *monitor_id = idx;
            return 1;
        }
    }

    return 0;
}

/*
 * Function
 *      _bcm_caladan3_monitor_mem_lookup
 * Purpose
 *      Get the memory location of the counter
 * Parameters
 *      (in) unit           = unit number
 *      (in) stat           = stat type
 *      (out) mem           = location of the stat in memory
 * Returns
 *      bcm_error_t         = BCM_E_* appropriately
 */
int
_bcm_caladan3_monitor_mem_lookup(int unit, bcm_policer_stat_t stat, int *mem)
{
    int rv = BCM_E_NONE;

    switch (stat) {
    case bcmPolicerStatGreenToGreenPackets: *mem = 1; break;
    case bcmPolicerStatGreenToGreenBytes: *mem = 2; break;
    case bcmPolicerStatGreenToYellowPackets: *mem = 3; break;
    case bcmPolicerStatGreenToYellowBytes: *mem = 4; break;
    case bcmPolicerStatGreenToRedPackets: *mem = 5; break;
    case bcmPolicerStatGreenToRedBytes: *mem = 6; break;
    case bcmPolicerStatGreenToDropPackets: *mem = 7; break;
    case bcmPolicerStatGreenToDropBytes: *mem = 8; break;
    case bcmPolicerStatYellowToGreenPackets: *mem = 9; break;
    case bcmPolicerStatYellowToGreenBytes: *mem = 10; break;
    case bcmPolicerStatYellowToYellowPackets: *mem = 11; break;
    case bcmPolicerStatYellowToYellowBytes: *mem = 12; break;
    case bcmPolicerStatYellowToRedPackets: *mem = 13; break;
    case bcmPolicerStatYellowToRedBytes: *mem = 14; break;
    case bcmPolicerStatYellowToDropPackets: *mem = 15; break;
    case bcmPolicerStatYellowToDropBytes: *mem = 16; break;
    case bcmPolicerStatRedToGreenPackets: *mem = 17; break;
    case bcmPolicerStatRedToGreenBytes: *mem = 18; break;
    case bcmPolicerStatRedToYellowPackets: *mem = 19; break;
    case bcmPolicerStatRedToYellowBytes: *mem = 20; break;
    case bcmPolicerStatRedToRedPackets: *mem = 21; break;
    case bcmPolicerStatRedToRedBytes: *mem = 22; break;
    case bcmPolicerStatRedToDropPackets: *mem = 23; break;
    case bcmPolicerStatRedToDropBytes: *mem = 24; break;
    default: rv = BCM_E_PARAM; break;
    }

    return rv;
}

/*
 * Function
 *      _bcm_caladan3_monitor_stat_get
 * Purpose
 *      get the specified stat value
 * Parameters
 *      (in) unit           = unit number
 *      (in) monitor_id     = monitor id
 *      (in) stat           = stat type
 *      (out)val            = stat value
 * Returns
 *      bcm_error_t         = BCM_E_* appropriately
 */
int
_bcm_caladan3_monitor_stat_get(int unit, int monitor_id, 
                            bcm_policer_stat_t stat, uint64 *val)
{
  uint32      counter_pair[2] = {0,0};
    int         mem;
    int         rv;
#ifdef BCM_FE2000_SUPPORT
    sbhandle    sbh;
#endif

    if (monitor_id >= BCM_CALADAN3_NUM_MONITORS) {
        LOG_ERROR(BSL_LS_BCM_POLICER,
                  (BSL_META_U(unit,
                              "Invalid monitor id %d. Valid values 0-7\n"),
                   monitor_id));
        return BCM_E_INTERNAL;
    }

    /* ERRATA work around. Green-to-Green Packet counter cannot be supported */
    if ((stat == bcmPolicerStatGreenToGreenPackets) && 
        (SOC_INFO(unit).chip_type == SOC_INFO_CHIP_TYPE_CALADAN3)) {
        return BCM_E_UNAVAIL;
    }

    rv = _bcm_caladan3_monitor_mem_lookup(unit,stat, &mem);
    if (rv != BCM_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_POLICER,
                  (BSL_META_U(unit,
                              "Invalid stat type %d. Valid stats 0-%d \n"),
                   stat, 
                   bcmPolicerStatCount));
        return rv;
    }

    if (mem % 2) {
        /* packets */
        COMPILER_64_SET(*val, 0, (counter_pair[1] >> 3));
    } else {
        /* bytes */
        COMPILER_64_SET(*val, (counter_pair[1] & 0x7), counter_pair[0]);
    }

    return BCM_E_NONE;
}

/*
 * Function
 *      _bcm_caladan3_monitor_stat_set
 * Purpose
 *      Set the counter value
 * Parameters
 *      (in) unit           = unit number
 *      (in) monitor_id     = monitor id
 *      (in) stat           = stat type
 *      (in)val             = stat value
 * Returns
 *      bcm_error_t         = BCM_E_* appropriately
 * Notes
 *      The set to value can only be zero. And the counter pair is set 
 *      when either stat value is indicated to be set.
 */
int
_bcm_caladan3_monitor_stat_set(int unit, int monitor_id, 
                            bcm_policer_stat_t stat, uint64 val)
{
    uint32      counter_pair[2];
    int         mem;
    int         rv;
#ifdef BCM_FE2000_SUPPORT
    sbhandle    sbh;
#endif

    if (monitor_id >= BCM_CALADAN3_NUM_MONITORS) {
        LOG_ERROR(BSL_LS_BCM_POLICER,
                  (BSL_META_U(unit,
                              "Invalid monitor id %d. Valid values 0-7\n"),
                   monitor_id));
        return BCM_E_INTERNAL;
    }

    rv = _bcm_caladan3_monitor_mem_lookup(unit,stat, &mem);
    if (rv != BCM_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_POLICER,
                  (BSL_META_U(unit,
                              "Invalid stat type %d. Valid stats 0-%d \n"),
                   stat, 
                   bcmPolicerStatCount));
        return rv;
    }

    COMPILER_64_TO_32_HI(counter_pair[1], val);
    COMPILER_64_TO_32_LO(counter_pair[0], val);

    if ((counter_pair[0] != 0) || (counter_pair[1] != 0)) {
        LOG_ERROR(BSL_LS_BCM_POLICER,
                  (BSL_META_U(unit,
                              "Counter can only be set to zero \n")));
        return BCM_E_PARAM;
    }

    return BCM_E_NONE;
}

/*
 * Function
 *      _bcm_caladan3_attach_monitor
 * Purpose
 *      Finds a free monitor resource and attaches to a policer
 * Parameters
 *      (in) unit           = unit number
 *      (in) policer_id     = id of the policer to detach monitor from
 * Returns
 *      bcm_error_t         = BCM_E_* appropriately
 */
int
_bcm_caladan3_attach_monitor(int unit, bcm_policer_t policer_id)
{
    int             monitor_id = 0;
    int             stat;
    uint64          val64;
    
    for (monitor_id=0; monitor_id<BCM_CALADAN3_NUM_MONITORS; monitor_id++) {
        if (_policer[unit].mon_use_map[monitor_id] == -1) {
            break;
        }
    }

    if (monitor_id >= BCM_CALADAN3_NUM_MONITORS) {
        LOG_ERROR(BSL_LS_BCM_POLICER,
                  (BSL_META_U(unit,
                              "No HW resources to enable stats on policer id %d \n"),
                   policer_id));
        return BCM_E_RESOURCE;
    }

    COMPILER_64_ZERO(val64);
    for (stat=0; stat<bcmPolicerStatRedToDropBytes; stat++) {
        /* clear the PM count memory of this monitor */
        _bcm_caladan3_monitor_stat_set(unit, monitor_id, stat, val64);
    }

    _policer[unit].mon_use_map[monitor_id] = policer_id;

    return BCM_E_NONE;
}

/*
 * Function
 *      _bcm_caladan3_g3p1_num_counters_get
 * Purpose
 *      Returns the number of counters in this policer group mode
 * Parameters
 *      (in) unit       = unit number
 *      (in) mode       = group mode 
 *      (out)ncounters  = number of counters in this mode
 * Returns
 *      bcm_error_t         = BCM_E_* appropriately
 */
int
_bcm_caladan3_g3p1_num_counters_get(int unit, bcm_policer_group_mode_t mode, 
                                  int *ncounters)
{
    int rv = BCM_E_NONE;
    int num_cos;

    if (!ncounters) {
        return BCM_E_PARAM;
    }

    num_cos = NUM_COS(unit);


    
    switch (mode) {

    case bcmPolicerGroupModeSingle: 
    case bcmPolicerGroupModeSingleWithControl: 
        *ncounters = 2; /* drop/pass */
        break;
    case bcmPolicerGroupModeTyped:
    case bcmPolicerGroupModeTypedWithControl:
        *ncounters = 8; /* drop/pass per traffic (unknown uc, uc, mc, bc) */
        break;
    case bcmPolicerGroupModeTypedAll:
    case bcmPolicerGroupModeTypedAllWithControl:
        *ncounters = 7; /* drop per traffic (unknown uc, uc, mc, bc) + 
                           G + Y + R */
        break;
    case bcmPolicerGroupModeTypedIntPri:
    case bcmPolicerGroupModeTypedIntPriWithControl:
        *ncounters = 4 + 3*num_cos; 
        /* drop per traffic (unknown uc, uc, mc, bc) + (G + Y + R)*cos */
        break; 
    default:
        rv = BCM_E_INTERNAL;
    }

    return rv;
}

/*
 * Function
 *      _bcm_caladan3_g3p1_free_counters
 * Purpose
 *      Frees the counter resources associated with this policer id
 * Parameters
 *      (in) unit       = unit number
 *      (in) policer_id = policer id 
 *      (in) use_cookie = inform to use policer cookie
 *      (in) counter    = base counter if use_cookie = 0
 * Returns
 *      bcm_error_t         = BCM_E_* appropriately
 * NOTE:
 *      Also updates HW to reflect the freed counter resources
 */
int
_bcm_caladan3_g3p1_free_counters(int unit,
                               bcm_policer_t policer_id,
                               int use_cookie,
                               uint32 counter)
{
    int                                 rv = BCM_E_NONE;
    soc_sbx_g3p1_lp_t                   lp;
    _bcm_policer_node_g3p1_cookie_t     *cookie = NULL;
    int                                 lpi, num_ctrs, lpi_max;
    uint32                              base_cntr;
    bcm_policer_group_mode_t            grp_mode;
    
    if (!IS_BASE_POLICER(_policer[unit].policers[policer_id].group_mode)) {
        LOG_ERROR(BSL_LS_BCM_POLICER,
                  (BSL_META_U(unit,
                              "%s: Invalid policer id. Not a base policer. \n"), 
                   FUNCTION_NAME()));
        return BCM_E_PARAM;
    }

    if (use_cookie) {
        cookie = (_bcm_policer_node_g3p1_cookie_t *) 
              _policer[unit].policers[policer_id].cookie;
        if (!cookie || !cookie->base_cntr) {
            return BCM_E_INTERNAL;
        }
        base_cntr  = cookie->base_cntr;
    } else {
        if (counter == 0) {
            return BCM_E_INTERNAL;
        }
        base_cntr  = counter;
    }

    if (use_cookie) {
        lpi_max = soc_sbx_g3p1_lp_table_size_get(unit);

        
        for (lpi=0; ((lpi<lpi_max) && (rv == BCM_E_NONE)); lpi++) {
            soc_sbx_g3p1_lp_t_init(&lp);
            rv = soc_sbx_g3p1_lp_get(unit, lpi, &lp);
            if (lp.policer == policer_id) {
                lp.counter = 0;
                lp.typedcount = 0;
                rv = soc_sbx_g3p1_lp_set(unit, lpi, &lp);
            }
        }
    }

    /* free the resources */
    if (rv == BCM_E_NONE) {
        grp_mode = POLICER_GROUP_MODE_GET(unit, policer_id);
        rv = _bcm_caladan3_g3p1_num_counters_get(unit, grp_mode, &num_ctrs);
    }
    if (rv == BCM_E_NONE) {
        rv = _bcm_caladan3_stat_block_free(unit, CALADAN3_G3P1_COUNTER_INGRESS, 
                                         base_cntr);
    }

    if (rv == BCM_E_NONE) {
        if (use_cookie) {
            sal_free(cookie);
            _policer[unit].policers[policer_id].cookie = 0;
        }
    }
    
    return rv;
}

/*
 * Function
 *      _bcm_caladan3_g3p1_alloc_counters
 * Purpose
 *      Allocates counter resources associated with this policer id
 * Parameters
 *      (in) unit       = unit number
 *      (in) policer_id = policer id 
 *      (in) use_cookie = whether to update policer cookie
 *      (out) counter   = allocated counter if use_cookie = 0
 * Returns
 *      bcm_error_t         = BCM_E_* appropriately
 * NOTE:
 *      Does not updates HW to reflect the allocated counter resources
 */
int
_bcm_caladan3_g3p1_alloc_counters(int unit, 
                                bcm_policer_t policer_id,
                                int use_cookie,
                                uint32 *counter)
{
    int                                 rv = BCM_E_NONE;
    _bcm_policer_node_g3p1_cookie_t     *cookie = NULL;
    uint32                              base_cntr = 0;
    bcm_policer_group_mode_t            grp_mode;
    int                                 num_ctrs = 0, idx, clear;
    soc_sbx_g3p1_turbo64_count_t        soc_val;
    uint32                              stat_flags = 0;
#if defined(BCM_WARM_BOOT_SUPPORT)
    bcm_caladan3_wb_policer_state_scache_info_t *wb_info_ptr = NULL;

    wb_info_ptr = BCM_CALADAN3_POLICER_SCACHE_INFO_PTR(unit);
    if(wb_info_ptr == NULL) {
        LOG_ERROR(BSL_LS_BCM_POLICER,
                  (BSL_META_U(unit,
                              "Warm boot not initialized for unit %d \n"),
                   unit));
        return BCM_E_INTERNAL;
    }
#endif


    if (!IS_BASE_POLICER(_policer[unit].policers[policer_id].group_mode)) {
        LOG_ERROR(BSL_LS_BCM_POLICER,
                  (BSL_META_U(unit,
                              "%s: Invalid policer id. Not a base policer. \n"),
                   FUNCTION_NAME()));
        return BCM_E_PARAM;
    }

    if (!use_cookie && counter == NULL) {
        return BCM_E_INTERNAL;
    }

    if (use_cookie) {
        cookie = (_bcm_policer_node_g3p1_cookie_t *) 
                  sal_alloc(sizeof(_bcm_policer_node_g3p1_cookie_t), 
                            "policer counter cookie");
    }

    if (use_cookie && !cookie) {
        return BCM_E_MEMORY;
    }

    grp_mode = POLICER_GROUP_MODE_GET(unit, policer_id);
    rv = _bcm_caladan3_g3p1_num_counters_get(unit, grp_mode, &num_ctrs);
    
    if (rv == BCM_E_NONE) {
#ifdef BCM_WARM_BOOT_SUPPORT
        if (SOC_WARM_BOOT(unit)) {
            /*
             * stat_flags = BCM_CALADAN3_STAT_WITH_ID;
             * egr_counter = restored egr_counter
             */
        }
#endif /* BCM_WARM_BOOT_SUPPORT */
        rv = _bcm_caladan3_stat_block_alloc(unit, CALADAN3_G3P1_COUNTER_INGRESS, 
                                            &base_cntr, num_ctrs, stat_flags);
    }

    if (rv == BCM_E_NONE) {
        /* clear all the counters */
        clear=1;
        for (idx=0; ((idx<num_ctrs) && (rv == BCM_E_NONE)); idx++) {
            rv = soc_sbx_g3p1_ingctr_read(unit, (base_cntr + idx), 1, clear, &soc_val);
        }
        if (rv == BCM_E_NONE) {
            if (use_cookie) {
                sal_memset(cookie, 0, sizeof(_bcm_policer_node_g3p1_cookie_t));
                cookie->base_cntr = base_cntr;
                _policer[unit].policers[policer_id].cookie = (uint32)cookie;
#if defined(BCM_WARM_BOOT_SUPPORT)
                SBX_WB_DB_SYNC_VARIABLE_OFFSET(uint32, 1,
                    wb_info_ptr->base_counter_offset + (policer_id * sizeof(uint32)),
                    base_cntr);

#endif
            } else {
                *counter = base_cntr;
            }
        } else {
            LOG_ERROR(BSL_LS_BCM_POLICER,
                      (BSL_META_U(unit,
                                  "%s: failed to clear counter %d \n"), 
                       FUNCTION_NAME(), (base_cntr + idx)));
            _bcm_caladan3_stat_block_free(unit, CALADAN3_G3P1_COUNTER_INGRESS, 
                                        base_cntr);
            if (use_cookie) {
                sal_free(cookie);
            }
        }
    } else {
        if (use_cookie) {
            sal_free(cookie);
        }
    }

    return rv;
}

/*
 * Function
 *      _bcm_caladan3_detach_monitor
 * Purpose
 *      Detach a monitor from policer
 * Parameters
 *      (in) unit           = unit number
 *      (in) policer_id     = policer id to detach from
 * Returns
 *      bcm_error_t         = BCM_E_* appropriately
 */
int
_bcm_caladan3_detach_monitor(int unit, bcm_policer_t policer_id)
{
    int         monitor_id = 0;

    for (monitor_id=0; monitor_id<BCM_CALADAN3_NUM_MONITORS; monitor_id++) {
        if (_policer[unit].mon_use_map[monitor_id] == policer_id) {
            break;
        }
    }

    if (monitor_id >= BCM_CALADAN3_NUM_MONITORS) {
        LOG_ERROR(BSL_LS_BCM_POLICER,
                  (BSL_META_U(unit,
                              "Monitor not attached to policer id %d \n"),
                   policer_id));
        return BCM_E_INTERNAL;
    }
    
    _policer[unit].mon_use_map[monitor_id] = -1;

    return BCM_E_NONE;
}

/*
 *  Function
 *      bcm_caladan3_policer_create
 *  Purpose
 *      Create a policer with a given config.
 *  Parameters
 *      (in) unit           = unit number
 *      (in) pol_cfg        = policer config
 *      (out) policer_id    = id of the created policer
 *  Returns
 *      bcm_error_t         = BCM_E_NONE if successful
 *                            BCM_E_* appropriately if not
 *  Notes
 */
int
bcm_caladan3_policer_create(int unit,
                          bcm_policer_config_t *pol_cfg,
                          bcm_policer_t *policer_id)
{
    int             rv;


    POLICER_INIT_CHECK(unit);

    if ((pol_cfg == NULL) || (policer_id == NULL)) {
        return BCM_E_PARAM;
    }

    POLICER_UNIT_LOCK(unit);

    if (pol_cfg->flags & BCM_POLICER_WITH_ID) {
        /* policer_id contains the requested id, validate it */
        rv = _bcm_caladan3_policer_id_check(unit, *policer_id);
        if (BCM_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_BCM_POLICER,
                      (BSL_META_U(unit,
                                  "Invalid policer id specified.\n")));
            POLICER_UNIT_UNLOCK(unit);
            return BCM_E_CONFIG;
        }
        rv = _bcm_caladan3_policer_id_reserve(unit, *policer_id);
        if (BCM_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_BCM_POLICER,
                      (BSL_META_U(unit,
                                  "Faild to reserve policer id specified.\n")));
            POLICER_UNIT_UNLOCK(unit);
            return BCM_E_CONFIG;
        }
    } else {
        /* auto assign a policer id */
        if (_bcm_caladan3_policer_id_assign(unit, policer_id) != BCM_E_NONE) {
            LOG_ERROR(BSL_LS_BCM_POLICER,
                      (BSL_META_U(unit,
                                  "%s: _bcm_caladan3_policer_id_assign failed. \n"), 
                       FUNCTION_NAME()));
            POLICER_UNIT_UNLOCK(unit);
            return BCM_E_INTERNAL;
        }
    }

    /* commit policer to hardware */
    rv = _bcm_caladan3_policer_hw_create(unit, *policer_id, pol_cfg);
    if (BCM_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_BCM_POLICER,
                  (BSL_META_U(unit,
                              "%s: _bcm_caladan3_policer_hw_create failed. \n"), 
                   FUNCTION_NAME()));
        _bcm_caladan3_policer_id_free(unit, *policer_id);
        POLICER_UNIT_UNLOCK(unit);
        return rv;
    }

    /* allcoate and save software state */
    if (BCM_SUCCESS(rv)) {
        rv = _bcm_caladan3_policer_state_alloc(unit, *policer_id,
                                      bcmPolicerGroupModeSingle, 1);
    }

    if (BCM_SUCCESS(rv)) {
        rv = _bcm_caladan3_policer_state_set(unit, *policer_id, pol_cfg);
    }

    if (BCM_FAILURE(rv)) {
        _bcm_caladan3_policer_state_remove(unit, *policer_id);
        /* ensure the id is freed on error, for the case where it is reserved,
         * but not added to the internal state
         */
        _bcm_caladan3_policer_id_free(unit, *policer_id);
    }

    POLICER_UNIT_UNLOCK(unit);
    return rv;
}

/*
 *  This function deletes all the individual member policers
 *  for a policer group.
 */

int
_bcm_caladan3_policer_group_destroy(int unit, soc_sbx_g3p1_policer_segment_t segment, bcm_policer_t policer_id)
{
    bcm_policer_group_mode_t    group_mode;
    int                         rv;
    uint32                      num_policers;
    int                         i;

    /* Make sure the policer id is for the base policer */
    if (!IS_BASE_POLICER(_policer[unit].policers[policer_id].group_mode)) {
        LOG_ERROR(BSL_LS_BCM_POLICER,
                  (BSL_META_U(unit,
                              "%s: Policer %d is not base policer\n"),
                              FUNCTION_NAME(), policer_id));
        return BCM_E_PARAM;
    }

    /* Get the count of policers in the group */
    group_mode = POLICER_GROUP_MODE_GET(unit, policer_id);
    rv = _bcm_caladan3_policer_group_num_get(unit, group_mode, &num_policers);
    if (BCM_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_BCM_POLICER,
                  (BSL_META_U(unit,
                              "_bcm_caladan3_policer_group_num_get failed %d\n"), rv));
        return rv;
    }

    /* Delete the individual policers in the group */
    for (i = 0; i < num_policers; i++) {

        /* Delete policer in hardware */
        rv = soc_sbx_g3p1_policer_delete(unit, segment, policer_id+i);
        if (BCM_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_BCM_POLICER,
                    (BSL_META_U(unit,
                                "%s: Failed to delete policer %d\n"),
                                FUNCTION_NAME(), policer_id+i));
            return rv;
        }

        /* Remove policer state */
        if (_bcm_caladan3_policer_state_remove(unit, policer_id+i) != BCM_E_NONE) {
            LOG_ERROR(BSL_LS_BCM_POLICER,
                    (BSL_META_U(unit,
                                "%s: Failed to remove state for policer %d\n"),
                                FUNCTION_NAME(), policer_id+i));
            rv = BCM_E_INTERNAL;
            return rv;
        }
    }

    /* Free the block of IDs */
    _bcm_caladan3_policer_id_free(unit, policer_id);

    
    return rv;
}

/*
 *  Function
 *      bcm_caladan3_policer_destroy
 *  Purpose
 *      Destroy a policer with specified id
 *  Parameters
 *      (in) unit       = unit number
 *      (in) policer_id = id of policer to destroy
 *  Returns
 *      bcm_error_t     = BCM_E_NONE if successful
 *                        BCM_E_* appropriately if not
 *  Notes
 */
int
bcm_caladan3_policer_destroy(int unit, bcm_policer_t policer_id)
{
    int                             rv = BCM_E_NONE;
    bcm_policer_config_t            pol_cfg;
    soc_sbx_g3p1_policer_segment_t  segment;
    bcm_policer_group_mode_t        group_mode;


    POLICER_INIT_CHECK(unit);

    POLICER_UNIT_LOCK(unit);

    /* Check if the policer has been created */
    rv = shr_aidxres_list_elem_state(_policer[unit].idlist, policer_id);
    if (rv != BCM_E_EXISTS) {
        LOG_ERROR(BSL_LS_BCM_POLICER,
                  (BSL_META_U(unit,
                              "Invalid policer id specified.\n")));
        POLICER_UNIT_UNLOCK(unit);
        return rv;
    }

    /* Get the policer segment */
    rv = bcm_policer_get(unit, policer_id, &pol_cfg);
    if (BCM_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_BCM_POLICER,
                  (BSL_META_U(unit,
                              "Failed to get policer configuration.\n")));
        POLICER_UNIT_UNLOCK(unit);
        return rv;
    }
    if (pol_cfg.flags & BCM_POLICER_EGRESS) {
        segment = COP_POLICER_SEGMENT_EGRPOL;
    } else {
        if (policer_id >= 1 && policer_id <= BCM_CALADAN3_XT_POLICERS) {
            segment = COP_POLICER_SEGMENT_XTPOL;
        } else {
            segment = COP_POLICER_SEGMENT_INGPOL;
        }
    }

    /* Get the mode */
    group_mode = POLICER_GROUP_MODE_GET(unit, policer_id);

    /* If policer is a group call the group delete function */
    if (group_mode != bcmPolicerGroupModeSingle) {
        rv =  _bcm_caladan3_policer_group_destroy(unit, segment, policer_id);
        POLICER_UNIT_UNLOCK(unit);
        return rv;
    }

    /* Delete policer */
    rv = soc_sbx_g3p1_policer_delete(unit, segment, policer_id);
    if (BCM_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_BCM_POLICER,
                  (BSL_META_U(unit,
                              "Failed to delete policer.\n")));
        POLICER_UNIT_UNLOCK(unit);
        return rv;
    }


    /* Remove policer state */
    if (_bcm_caladan3_policer_state_remove(unit, policer_id) != BCM_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_POLICER,
                  (BSL_META_U(unit,
                              "%s: failed. "),
                   FUNCTION_NAME()));
        rv = BCM_E_INTERNAL;
    }

    /* Free the policer ID */
    _bcm_caladan3_policer_id_free(unit, policer_id);

    POLICER_UNIT_UNLOCK(unit);

    return rv;
}

/*
 *  Function
 *      bcm_caladan3_policer_destroy_all
 *  Purpose
 *      Destroy all policers on a given unit
 *  Parameters
 *      (in) unit   = unit number
 *  Returns
 *      bcm_error_t = BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *  Notes
 */
int
bcm_caladan3_policer_destroy_all(int unit)
{
    int                             rv = BCM_E_NONE;
    soc_sbx_g3p1_policer_segment_t  segment;
    bcm_policer_t                   id;
    int                             idx, pol_count;
    _bcm_policer_node_t             *cur;


    POLICER_INIT_CHECK(unit);

    POLICER_UNIT_LOCK(unit);
    cur = _policer[unit].pol_head;
    pol_count = _policer[unit].pol_count;
    for (idx = 0; ((idx < pol_count) && (cur)); idx++) {
        id = cur->id;

        /* Get the policer segment */
        if (cur->pol_cfg->flags & BCM_POLICER_EGRESS) {
            segment = COP_POLICER_SEGMENT_EGRPOL;
        } else {
            if (id >= 1 && id <= BCM_CALADAN3_XT_POLICERS) {
                segment = COP_POLICER_SEGMENT_XTPOL;
            } else {
                segment = COP_POLICER_SEGMENT_INGPOL;
            }
        }

        cur = cur->next;

        /* Delete the policer */
        rv = soc_sbx_g3p1_policer_delete(unit, segment, id);
        if (rv != BCM_E_NONE) {
            LOG_ERROR(BSL_LS_BCM_POLICER,
                      (BSL_META_U(unit,
                                  "soc_sbx_g3p1_policer_delete failed. return value %d (%s)"),
                       rv, bcm_errmsg(rv)));
        } else {
            rv = _bcm_caladan3_policer_state_remove(unit, id);
        }

        if (rv != BCM_E_NONE) {
            LOG_ERROR(BSL_LS_BCM_POLICER,
                      (BSL_META_U(unit,
                                  "%s: failed. "),
                       FUNCTION_NAME()));
            break;
        }
    }
    POLICER_UNIT_UNLOCK(unit);

    return rv;

}


/*
 *  Function
 *      bcm_caladan3_policer_set
 *  Purpose
 *      Set/Override the config of a policer with a given id and
 *      specified config
 *  Parameters
 *      (in) unit       = unit number
 *      (in) policer_id = id of policer whose config to change
 *      (in) pol_cfg    = new config
 *  Returns
 *      bcm_error_t     = BCM_E_NONE if successful
 *                        BCM_E_* appropriately if not
 *  Notes
 */
int
bcm_caladan3_policer_set(int unit, bcm_policer_t policer_id,
                       bcm_policer_config_t *pol_cfg)
{
    int                             rv = BCM_E_CONFIG;
    bcm_policer_group_mode_t        group_mode;
    soc_sbx_g3p1_policer_segment_t  segment;

    POLICER_INIT_CHECK(unit);

    /* Set the policer segment */
    if (pol_cfg->flags & BCM_POLICER_EGRESS) {
        segment = COP_POLICER_SEGMENT_EGRPOL;
    } else {
        if (policer_id >= 1 && policer_id <= BCM_CALADAN3_XT_POLICERS) {
            segment = COP_POLICER_SEGMENT_XTPOL;
        } else {
            segment = COP_POLICER_SEGMENT_INGPOL;
        }
    }

    /* only supported for group_created id's, so must be WITH_ID and
     * the group_mode must not be single
     */
    if (POLICER_ID_INVALID(unit, policer_id) || (pol_cfg == NULL) ||
        policer_id <= BCM_CALADAN3_RESERVED_POLICERS ||
        ((pol_cfg->flags & BCM_POLICER_WITH_ID) == 0))
    {
        return BCM_E_PARAM;
    }

    rv = _bcm_caladan3_policer_group_mode_get(unit, policer_id, &group_mode);
    if (BCM_FAILURE(rv) || group_mode == bcmPolicerGroupModeSingle) {
        LOG_ERROR(BSL_LS_BCM_POLICER,
                  (BSL_META_U(unit,
                              "%s: Failed to get group mode, or invalid mode=%d\n"),
                   FUNCTION_NAME(), group_mode));
        return BCM_E_PARAM;
    }

    /* validate the ID */
    rv = shr_aidxres_list_elem_state(_policer[unit].idlist, policer_id);
    if (rv != BCM_E_EXISTS) {
        LOG_ERROR(BSL_LS_BCM_POLICER,
                  (BSL_META_U(unit,
                              "%s: Invalid policer id:%d\n"), 
                   FUNCTION_NAME(), policer_id));
        return BCM_E_PARAM;
    }

    POLICER_UNIT_LOCK(unit);

    /* commit the policer config to hardware */
    rv = _bcm_caladan3_policer_hw_create(unit, policer_id, pol_cfg);

    /* Policer IDs were stored when the group was created, only need to update
     * the software state with the config
     */
    if (BCM_SUCCESS(rv)) {
        rv = _bcm_caladan3_policer_state_set(unit, policer_id, pol_cfg);
    }

    if (BCM_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_BCM_POLICER,
                  (BSL_META_U(unit,
                              "%s: _bcm_caladan3_policer_hw_create failed."),
                   FUNCTION_NAME()));
        rv = soc_sbx_g3p1_policer_delete(unit, segment, policer_id);
        if (BCM_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_BCM_POLICER,
                      (BSL_META_U(unit,
                                  "%s: soc_sbx_g3p1_policer_delete failed."),
                       FUNCTION_NAME()));
        }
    }

    POLICER_UNIT_UNLOCK(unit);

    return rv;
}

/*
 *  Function
 *      bcm_caladan3_policer_get
 *  Purpose
 *      Retrieve the config of a policer with a given id
 *  Parameters
 *      (in) unit       = unit number
 *      (in) policer_id = id of policer whose config to retrieve
 *      (out) pol_cfg   = config of the policer with specified id
 *  Returns
 *      bcm_error_t     = BCM_E_NONE if successful
 *                        BCM_E_* appropriately if not
 *  Notes
 */
int
bcm_caladan3_policer_get(int unit, bcm_policer_t id,
                       bcm_policer_config_t *pol_cfg)
{
    int                     status = BCM_E_NONE;
    int                     idx;
    int                     success = 0;
    _bcm_policer_node_t     *cur;

    POLICER_INIT_CHECK(unit);
    POLICER_NULL_PARAM_CHECK(pol_cfg);

    if (POLICER_ID_INVALID(unit, id)) {
        LOG_ERROR(BSL_LS_BCM_POLICER,
                  (BSL_META_U(unit,
                              "%s: invalid policer id %d or pol_cfg (%x) \n"),
                   FUNCTION_NAME(), id, (uint32)pol_cfg));
        return BCM_E_PARAM;
    }

    POLICER_UNIT_LOCK(unit);
    cur = _policer[unit].pol_head;
    for (idx = 0; ((idx < _policer[unit].pol_count) && (cur)); idx++) {
        if ((cur->id == id) && (cur->pol_cfg)) {
            /* found the node */
            sal_memcpy(pol_cfg, cur->pol_cfg, sizeof(bcm_policer_config_t));
            success = 1;
            break;
        }
        cur = cur->next;
    }

    if (!success) {
        LOG_ERROR(BSL_LS_BCM_POLICER,
                  (BSL_META_U(unit,
                              "%s: could not find policer id %d. \n"), 
                   FUNCTION_NAME(), id));
        status = BCM_E_PARAM;
    }
    POLICER_UNIT_UNLOCK(unit);

    return status;
}

/*
 *  Function
 *      bcm_caladan3_policer_traverse
 *  Purpose
 *      Retrieve the config of a policer with a given id
 *  Parameters
 *      (in) unit       = unit number
 *      (in) policer_id = id of policer whose config to retrieve
 *      (out) pol_cfg   = config of the policer with specified id
 *  Returns
 *      bcm_error_t     = BCM_E_NONE if successful
 *                        BCM_E_* appropriately if not
 *  Notes
 */
int
bcm_caladan3_policer_traverse(int unit,
                            bcm_policer_traverse_cb traverse_callback,
                            void *cookie)
{
    int                     idx, rv;
    _bcm_policer_node_t     *cur;
    bcm_policer_config_t    *pol_cfg;

    rv = BCM_E_NONE;
    POLICER_INIT_CHECK(unit);

    if (!traverse_callback) {
        LOG_ERROR(BSL_LS_BCM_POLICER,
                  (BSL_META_U(unit,
                              "%s: Invalid callback function \n"),
                   FUNCTION_NAME()));
        return BCM_E_PARAM;
    }

    if (!(pol_cfg = (bcm_policer_config_t *)
                    sal_alloc(sizeof(bcm_policer_config_t), "pol_cfg"))) {
        return BCM_E_MEMORY;
    }

    POLICER_UNIT_LOCK_nr(unit, rv);
    
    if (BCM_SUCCESS(rv)) {
        cur = _policer[unit].pol_head;
        for (idx = 0; ((idx < _policer[unit].pol_count) && (cur)); idx++) {
            sal_memcpy(pol_cfg, cur->pol_cfg, sizeof(bcm_policer_config_t));
            (*traverse_callback)(unit, cur->id, pol_cfg, cookie);
            cur = cur->next;
        }
        POLICER_UNIT_UNLOCK_nr(unit, rv);
    }

    sal_free(pol_cfg);

    return rv;
}

/*  Function
 *      _bcm_caladan3_reserved_policers_init
 *  Purpose
 *      Configure the reserved policers for exceptions & ports
 *  Parameters
 *      (in) unit   = unit number
 *  Returns
 *      bcm_error_t = BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 */
int
_bcm_caladan3_reserved_policers_init(int unit)
{
    bcm_policer_config_t    pol_cfg;
    bcm_policer_t           pol_id;
    int                     idx;
    int                     status = BCM_E_NONE;

    sal_memset(&(pol_cfg), 0, sizeof(pol_cfg));

    pol_cfg.flags = BCM_POLICER_WITH_ID; 
    pol_cfg.mode = bcmPolicerModePassThrough;
    pol_cfg.ckbits_burst = BYTES_TO_KBITS(SBX_DEFAULT_MTU_SIZE);
    
    pol_cfg.ckbits_sec = 1000;
    pol_cfg.pkbits_burst = BYTES_TO_KBITS(SBX_DEFAULT_MTU_SIZE);
    pol_cfg.pkbits_sec = 1000;
    for (idx = 1;
         idx <= (BCM_CALADAN3_RESERVED_POLICERS - BCM_CALADAN3_SPECIAL_POLICERS);
         idx++) {
        pol_id = idx;
        status = bcm_caladan3_policer_create(unit, &pol_cfg, &pol_id);
        if (status != BCM_E_NONE) {
            LOG_ERROR(BSL_LS_BCM_POLICER,
                      (BSL_META_U(unit,
                                  "%s: _bcm_port_policers_init failed. \n"),
                       FUNCTION_NAME()));
            break;
        }
    }
    /*
     *  Special policers require explicit initialisation.
     */
    /* drop everything policer */
    if (BCM_E_NONE == status) {
        LOG_DEBUG(BSL_LS_BCM_POLICER,
                  (BSL_META_U(unit,
                              "initialised unit %d exception+port policers 1..%d\n"),
                   unit,
                   idx - 1));
        pol_cfg.ckbits_burst = 0;
        pol_cfg.ckbits_sec = 0;
        pol_cfg.pkbits_burst = 0;
        pol_cfg.pkbits_sec = 0;
        pol_cfg.mode = bcmPolicerGroupModeSingle;
        pol_cfg.flags = BCM_POLICER_COLOR_BLIND |
                        BCM_POLICER_DROP_RED |
                        BCM_POLICER_WITH_ID;
        pol_id = BCM_CALADAN3_SPEC_POL_DROP_ALL;
        status = bcm_caladan3_policer_create(unit, &pol_cfg, &pol_id);
        if (BCM_E_NONE == status) {
            LOG_DEBUG(BSL_LS_BCM_POLICER,
                      (BSL_META_U(unit,
                                  "initialised unit %d drop policer %d\n"),
                       unit,
                       pol_id));
        } else {
            LOG_ERROR(BSL_LS_BCM_POLICER,
                      (BSL_META_U(unit,
                                  "unable to initialise unit %d drop policer (%d): %d (%s)\n"),
                       unit,
                       BCM_CALADAN3_SPEC_POL_DROP_ALL,
                       status,
                       _SHR_ERRMSG(status)));
        }
    }
    return status;
}

int
bcm_caladan3_policer_detach(int unit)
{

    /* Destroy all policers */
    if (!SOC_WARM_BOOT(unit)) {
        bcm_policer_destroy_all(unit);
    }

    if (_policer[unit].policers) {
        sal_free(_policer[unit].policers);
        _policer[unit].policers = NULL;
    }

    if (_policer[unit].res_idlist) {
        shr_idxres_list_destroy(_policer[unit].res_idlist);
        _policer[unit].res_idlist = NULL;
    }

    if (_policer[unit].idlist != NULL) {
        shr_aidxres_list_destroy(_policer[unit].idlist);
        _policer[unit].idlist = NULL;
    }

    /* sal_mutex_destroy(_policer_glob_lock); */
    sal_mutex_destroy(_policer[unit].lock);
    _policer[unit].lock = NULL;

    return BCM_E_NONE;
}


int
_bcm_caladan3_policer_id_recover(int unit, int id, 
                        bcm_policer_config_t *policer_config)
{
    int                         rv;
    uint32                      profile_id;

    /* read the policer from hardware */
    rv = _bcm_caladan3_policer_get_hw(unit, id, policer_config, &profile_id);
    if (BCM_FAILURE(rv) && (!SOC_WARM_BOOT(unit) || rv != BCM_E_NOT_FOUND)) {
        LOG_ERROR(BSL_LS_BCM_POLICER,
                  (BSL_META_U(unit,
                              "Failed to get policer 0x%04x: %s\n"),
                   id, bcm_errmsg(rv)));
        return rv;
    }

    /* recover the policer soc state (profile & ref counts) */
    /* TBD: the driver profile data can be restored by calling
     * _bcm_caladan3_policer_hw_create().
     */

    return rv;
}

#ifdef BCM_WARM_BOOT_SUPPORT
/*
 * Function
 *      bcm_caladan3_policer_recover
 * Purpose
 *      recover the internal policer state during a warm boot
 * Parameters
 *      (in) unit       = unit number
 * Returns
 *   BCM_E_*
 */
int
bcm_caladan3_policer_recover(int unit)
{
    int                     rv = BCM_E_NONE;
    int                     max_policers;
    int                     i;
    bcm_policer_config_t    policer_config;
    uint32                  num_policers;

    bcm_caladan3_wb_policer_state_scache_info_t *wb_info_ptr = NULL;
    bcm_policer_group_mode_t            group_mode;
    uint32                              base_cntr;
    _bcm_policer_node_g3p1_cookie_t     *cookie;

    wb_info_ptr = BCM_CALADAN3_POLICER_SCACHE_INFO_PTR(unit);
    if(wb_info_ptr == NULL) {
        LOG_ERROR(BSL_LS_BCM_POLICER,
                  (BSL_META_U(unit,
                              "Warm boot not initialized for unit %d \n"),
                   unit));
        return BCM_E_INTERNAL;
    }

    max_policers = _bcm_caladan3_policer_num_policers_get(unit);

    for (i = 1; i < max_policers; i++) {


        /* Firs try to find an ingress policer */
        bcm_policer_config_t_init(&policer_config);
        rv = _bcm_caladan3_policer_id_recover(unit, i, &policer_config);
        if (BCM_FAILURE(rv) && rv == BCM_E_NOT_FOUND) {

                /* Try to find an egress policer */
                policer_config.flags = BCM_POLICER_EGRESS;
                rv = _bcm_caladan3_policer_id_recover(unit, i, &policer_config);
                if (BCM_FAILURE(rv) && rv == BCM_E_NOT_FOUND) {
                    rv = BCM_E_NONE;
                    continue;
                }
        }
        if (BCM_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_BCM_POLICER,
                  (BSL_META_U(unit,
                              "_bcm_caladan3_policer_id_recover id %d for unit %d \n"),
                   i, unit));
            return rv;
        }

        SBX_WB_DB_RESTORE_VARIABLE_OFFSET(uint32, 1,
            wb_info_ptr->group_mode_offset + (i * sizeof(uint32)),
            group_mode);
        SBX_WB_DB_RESTORE_VARIABLE_OFFSET(uint32, 1,
            wb_info_ptr->flags_offset + (i * sizeof(uint32)),
            policer_config.flags);
        SBX_WB_DB_RESTORE_VARIABLE_OFFSET(uint32, 1,
            wb_info_ptr->base_counter_offset + (i * sizeof(uint32)),
            base_cntr);

        /* If this is a base policer then reserve policer ID(s) */
        if (IS_BASE_POLICER(group_mode)) {

            if (GROUP_MODE_GET(group_mode) != bcmPolicerGroupModeSingle) {

                /* Reserve block of IDs for policer group */
                _bcm_caladan3_policer_group_num_get(unit, GROUP_MODE_GET(group_mode), &num_policers);
                rv = shr_aidxres_list_reserve_block(_policer[unit].idlist, i, num_policers);
                if (BCM_FAILURE(rv)) {
                    LOG_ERROR(BSL_LS_BCM_POLICER,
                            (BSL_META_U(unit,
                                        "Failed to reserve policer block id 0x%04x: %s\n"),
                                        i, bcm_errmsg(rv)));
                    return rv;
                }
            } else {
    
                /* Reserve single ID */
                rv = _bcm_caladan3_policer_id_reserve(unit, i);
                if (BCM_FAILURE(rv)) {
                    LOG_ERROR(BSL_LS_BCM_POLICER,
                            (BSL_META_U(unit,
                                        "Failed to reserve policer id 0x%04x: %s\n"),
                                        i, bcm_errmsg(rv)));
                    return rv;
                }
            }
        }

        if (base_cntr != 0) {
            cookie = sal_alloc(sizeof(_bcm_policer_node_g3p1_cookie_t),
                  "policer counter cookie");
            if (!cookie) {
                LOG_ERROR(BSL_LS_BCM_POLICER,
                          (BSL_META_U(unit,
                                      "%s: failed to allocate cookie\n"),
                           FUNCTION_NAME()));
                return BCM_E_MEMORY;
            }
            cookie->base_cntr = base_cntr;
            _policer[unit].policers[i].cookie = (uint32)cookie;
            /* Reserve the counter number */
            rv = _bcm_caladan3_stat_block_alloc(unit, CALADAN3_G3P1_COUNTER_INGRESS, 
                                            &base_cntr, 1, BCM_CALADAN3_STAT_WITH_ID);
            if (BCM_FAILURE(rv)) {
                LOG_ERROR(BSL_LS_BCM_POLICER,
                          (BSL_META_U(unit,
                                      "%s: failed to allocate counter id %d\n"),
                           FUNCTION_NAME(), base_cntr));
                return rv;
            }
        }

        rv = _bcm_caladan3_policer_state_alloc(unit, i,
                                      group_mode, 1);
        if (BCM_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_BCM_POLICER,
                      (BSL_META_U(unit,
                                  "Failed to allocate policer 0x%04x state: %s\n"),
                       i, bcm_errmsg(rv)));
            return rv;
        }

        rv = _bcm_caladan3_policer_state_set(unit, i, &policer_config);
        if (BCM_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_BCM_POLICER,
                      (BSL_META_U(unit,
                                  "Failed to set policer 0x%04x state: %s\n"),
                       i, bcm_errmsg(rv)));
            return rv;
        }

    }

    return rv;
}
#endif


/*
 *  Function
 *      bcm_caladan3_policer_init
 *  Purpose
 *      Initialise the policer APIs.
 *  Parameters
 *      (in) unit   = unit number
 *  Returns
 *      bcm_error_t = BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *  Notes
 */
int
bcm_caladan3_policer_init(int unit)
{
    int             status = BCM_E_NONE;
    sal_mutex_t     local_lock;
    int             idx;

    POLICER_UNIT_CHECK(unit);

    if (_policer[unit].lock) {
        status = bcm_caladan3_policer_detach(unit);
        BCM_IF_ERROR_RETURN(status);
        /* _policer_glob_lock = NULL; */
    }

    if (!_policer_glob_lock) {
        /* create and initialize the global lock */
        local_lock = sal_mutex_create("_policer_glob_lock");
        if (!local_lock) {
            LOG_ERROR(BSL_LS_BCM_POLICER,
                      (BSL_META_U(unit,
                                  "%s: sal_mutex_create failed. \n"),
                       FUNCTION_NAME()));
            return BCM_E_RESOURCE;
        }
        if (sal_mutex_take(local_lock, sal_mutex_FOREVER) == 0) {
            /* initialize the global struct */
            sal_memset(&(_policer), 0, sizeof(_policer));
            if (sal_mutex_give(local_lock) != 0) {
                LOG_ERROR(BSL_LS_BCM_POLICER,
                          (BSL_META_U(unit,
                                      "%s: sal_mutex_give failed.\n"),
                           FUNCTION_NAME()));
                sal_mutex_destroy(local_lock);
                return BCM_E_INTERNAL;
            }
        } else {
            LOG_ERROR(BSL_LS_BCM_POLICER,
                      (BSL_META_U(unit,
                                  "%s: sal_mutex_take failed. \n"),
                       FUNCTION_NAME()));
            sal_mutex_destroy(local_lock);
            return BCM_E_INTERNAL;
        }
        _policer_glob_lock = local_lock;
    }

    /* get global lock */
    if (sal_mutex_take(_policer_glob_lock, sal_mutex_FOREVER) != 0) {
        LOG_ERROR(BSL_LS_BCM_POLICER,
                  (BSL_META_U(unit,
                              "%s: sal_mutex_take failed. \n"),
                   FUNCTION_NAME()));
        sal_mutex_destroy(_policer_glob_lock); 
        return BCM_E_INTERNAL;
    }

    status = BCM_E_NONE;

    for (idx=0; idx<BCM_CALADAN3_NUM_MONITORS; idx++) {
        /* reset the monitor usage map */
        _policer[unit].mon_use_map[idx] = -1;
    }

    if (_policer[unit].lock == 0 && BCM_SUCCESS(status)) {
        /* Now initialize the per unit policer data */
        local_lock = sal_mutex_create("_policer_unit_lock");
        if (local_lock) {
            _policer[unit].policers = sal_alloc((sizeof(_bcm_policer_node_t) *
                                                 _bcm_caladan3_policer_num_policers_get(unit)),
                                                 "_policer_unit_data");
            if (_policer[unit].policers == 0) {
                status = BCM_E_MEMORY;
            }

            if (status == BCM_E_NONE) {

                /* Start ID managment at 1 to consider 0 as used by ucode -
                 * SOC_SBX_G2P3_NOP_POLICER_ID
                 */
                status = shr_idxres_list_create(& _policer[unit].res_idlist,
                                                1,
                                                BCM_CALADAN3_RESERVED_POLICERS,
                                                0,
                                                BCM_CALADAN3_RESERVED_POLICERS,
                                                "Reserved policers ids");
                if (status != BCM_E_NONE) {
                    LOG_ERROR(BSL_LS_BCM_POLICER,
                              (BSL_META_U(unit,
                                          "%s: shr_idxres_list_create for res policer "
                                           "ids failed on unit %d\n"), 
                               FUNCTION_NAME(), unit));
                    _policer[unit].res_idlist = NULL;
                }
            }

            if (status == BCM_E_NONE) {
                int num_cos = NUM_COS(unit);
                int cos_pow;

                if (num_cos < 1 ) {
                    num_cos = 1;
                }

                /* compute the resource list block factor by finding the
                 * log2(cos), and adding 1.  The block factor is
                 * log2(largest_block_size), the largest block of policers
                 * currenty required is NUM_COS+1.
                 */
                num_cos--;
                for (cos_pow = 0; num_cos; cos_pow++) {
                    num_cos = num_cos >> 1;
                }
                cos_pow++;

                status = shr_aidxres_list_create(& _policer[unit].idlist,
                                                 BCM_CALADAN3_RESERVED_POLICERS+1,
                                                 _bcm_caladan3_policer_max_id_get(unit),
                                                 0,
                                                 _bcm_caladan3_policer_max_id_get(unit),
                                                 cos_pow,
                                                 "Policers ids");

                if (status != BCM_E_NONE) {
                    LOG_ERROR(BSL_LS_BCM_POLICER,
                              (BSL_META_U(unit,
                                          "%s: shr_idxres_list_create for policer ids "
                                           "failed unit %d\n"), FUNCTION_NAME(), unit));
                    _policer[unit].idlist = NULL;
                }
            }

            if (status != BCM_E_NONE) {
                if (_policer[unit].policers) {
                    sal_free(_policer[unit].policers);
                    _policer[unit].policers = NULL;
                }
                if (_policer[unit].res_idlist) {
                    shr_idxres_list_destroy(_policer[unit].res_idlist);
                    _policer[unit].res_idlist = NULL;
                }
                sal_mutex_destroy(local_lock);
                local_lock = 0;
            }
        } else {
            LOG_ERROR(BSL_LS_BCM_POLICER,
                      (BSL_META_U(unit,
                                  "%s: sal_mutex_create failed. \n"),
                       FUNCTION_NAME()));
            status = BCM_E_RESOURCE;
        }

        if (status == BCM_E_NONE) {
            if (sal_mutex_take(local_lock, sal_mutex_FOREVER) == 0) {
                _policer[unit].lock = local_lock;
            } else {
                LOG_ERROR(BSL_LS_BCM_POLICER,
                          (BSL_META_U(unit,
                                      "%s: sal_mutex_take failed \n"),
                           FUNCTION_NAME()));
                status = BCM_E_INTERNAL;
            }
        }
    } else {
        /* lock already created...meaning initialized before */
        /*
        if (sal_mutex_take(_policer[unit].lock, sal_mutex_FOREVER)) {
            LOG_ERROR(BSL_LS_BCM_POLICER,
                      (BSL_META_U(unit,
                                  "%s: sal_mutex_take failed for unit %d. \n"),
                       FUNCTION_NAME(), unit));
            status = BCM_E_INTERNAL; \
        }
        */
        
        LOG_ERROR(BSL_LS_BCM_POLICER,
                  (BSL_META_U(unit,
                              "%s: failed. Previously initialized\n"),
                   FUNCTION_NAME()));
        status = BCM_E_INTERNAL;
    }

    /* now initialize the per unit policer structure */
    if (BCM_SUCCESS(status)) {
        _policer[unit].pol_count = 0;
        _policer[unit].pol_head = NULL;
        sal_memset(_policer[unit].policers, 0, (sizeof(_bcm_policer_node_t) *
                                                _bcm_caladan3_policer_num_policers_get(unit)));
        if (sal_mutex_give(_policer[unit].lock)) {
            LOG_ERROR(BSL_LS_BCM_POLICER,
                      (BSL_META_U(unit,
                                  "%s: sal_mutex_give failed for unit %d. \n"),
                       FUNCTION_NAME(), unit));
            status = BCM_E_INTERNAL;
            return status;
        }
    }

    if (sal_mutex_give(_policer_glob_lock) != 0) {
        LOG_ERROR(BSL_LS_BCM_POLICER,
                  (BSL_META_U(unit,
                              "%s: sal_mutex_give failed. \n"),
                   FUNCTION_NAME()));
        status = BCM_E_INTERNAL;
        return status;
    }
    
#ifdef BCM_WARM_BOOT_SUPPORT
    status = bcm_caladan3_wb_policer_state_init(unit);
#endif

    if (status == BCM_E_NONE && !SOC_WARM_BOOT(unit)) {
        if ((status = _bcm_caladan3_reserved_policers_init(unit)) != BCM_E_NONE) {
            LOG_ERROR(BSL_LS_BCM_POLICER,
                      (BSL_META_U(unit,
                                  "%s: _bcm_port_policers_init failed. \n"), 
                       FUNCTION_NAME()));
        }
    }

    LOG_VERBOSE(BSL_LS_BCM_POLICER,
                (BSL_META_U(unit,
                            "bcm_policer_init: unit=%d rv=%d(%s)\n"),
                 unit, status, bcm_errmsg(status)));

    if (status != BCM_E_NONE) {
        sal_mutex_destroy(_policer_glob_lock);
    }

#ifdef BCM_WARM_BOOT_SUPPORT
    if (SOC_WARM_BOOT(unit)) {
        status = bcm_caladan3_policer_recover(unit);
        if (BCM_FAILURE(status)) {
            LOG_ERROR(BSL_LS_BCM_POLICER,
                      (BSL_META_U(unit,
                                  "Failed to recover policers: %s\n"), 
                       bcm_errmsg(status)));
        }
    }
#endif

    return status;
}


/*
 *  Function
 *      bcm_caladan3_policer_group_create
 *  Purpose
 *      Allocates a group of policers
 *  Parameters
 *      (in) unit       = unit number
 *      (in) mode       = group mode
 *      (out)policer_id = base policer id allocated
 *      (out)npolicers  = number of policers allocated
 *  Returns
 *      bcm_error_t = BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *  NOTE
 *      policers are just allocated. They are not configured.
 */
int
bcm_caladan3_policer_group_create(int unit, bcm_policer_group_mode_t mode,
                                bcm_policer_t *policer_id, int *npolicers)
{
    uint32 num_to_alloc = 0;
    int rv = BCM_E_NONE;

    POLICER_INIT_CHECK(unit);

    rv = _bcm_caladan3_policer_group_num_get(unit, mode, &num_to_alloc);

    if (rv != BCM_E_NONE) {
        return rv;
    }

    POLICER_UNIT_LOCK(unit);

    *npolicers = 0;

    if (num_to_alloc == 1) {
        rv = shr_aidxres_list_alloc(_policer[unit].idlist,
                                    (uint32*) policer_id);
    } else {
        rv = shr_aidxres_list_alloc_block(_policer[unit].idlist,
                                          num_to_alloc,
                                          (uint32*) policer_id);
    }


    if (BCM_SUCCESS(rv)) {
        int i;

        LOG_VERBOSE(BSL_LS_BCM_POLICER,
                    (BSL_META_U(unit,
                                "%s: reserved ids %d-%d\n"),
                     FUNCTION_NAME(), *policer_id,
                     *policer_id + num_to_alloc - 1));
        *npolicers = num_to_alloc;

        /* create the sw state to track these policer ids */
        for (i=0; i<num_to_alloc && BCM_SUCCESS(rv); i++) {
            rv = _bcm_caladan3_policer_state_alloc(unit, *policer_id + i, mode, (!i));
        }

        if (BCM_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_BCM_POLICER,
                      (BSL_META_U(unit,
                                  "%s: failed to create sw state for base policer "
                                   "id=%d\n"), FUNCTION_NAME(), *policer_id));

            /* only need to free sw sate at this point because no hw state
             * has been written
             */
            for (; i>=0; i--) {
                _bcm_caladan3_policer_state_remove(unit, *policer_id + i);
            }
        }
    }

    POLICER_UNIT_UNLOCK(unit);

    return rv;
}

/*
 *  Function
 *      bcm_caladan3_policer_stat_enable_get
 *  Purpose
 *      Check if stats are enabled on the policer
 *  Parameters
 *      (in) unit           = unit number
 *      (in) policer_id     = policer id
 *      (out) enable        = out value
 *  Returns
 *      bcm_error_t = BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 */
int 
bcm_caladan3_policer_stat_enable_get(int unit, bcm_policer_t policer_id,
                                   int *enable)
{
    bcm_policer_config_t    pol_cfg;
    int                     rv = BCM_E_NONE;
    int                     idx = 0;

    POLICER_NULL_PARAM_CHECK(enable);

    /* check if policer exists */
    rv = bcm_caladan3_policer_get(unit, policer_id, &pol_cfg);
    if (rv != BCM_E_NONE) {
        return rv;
    }

    POLICER_UNIT_LOCK(unit);
    *enable = 0;
    if (SOC_IS_SBX_G3P1(unit)) {
        /* G2P3 supports ucode policer stats */
        if (IS_BASE_POLICER(_policer[unit].policers[policer_id].group_mode)) {
            if (_policer[unit].policers[policer_id].cookie) {
                *enable = 1;
            }
        } else {
            LOG_ERROR(BSL_LS_BCM_POLICER,
                      (BSL_META_U(unit,
                                  "%s: Invalid policer id. Not a base policer. \n"), 
                       FUNCTION_NAME()));
            rv = BCM_E_PARAM;
        }
    } else {
        /* default to HW monitors */
        for (idx=0; idx<BCM_CALADAN3_NUM_MONITORS; idx++) {
            if (_policer[unit].mon_use_map[idx] == policer_id) {
                *enable = 1;
                break;
            }
        }
    }
    POLICER_UNIT_UNLOCK(unit);

    return rv;
}

/*
 *  Function
 *      bcm_caladan3_policer_stat_enable_set
 *  Purpose
 *      Enables/disables stats on the policer
 *  Parameters
 *      (in) unit           = unit number
 *      (in) policer_id     = policer id
 *      (in) enable         = enable value
 *  Returns
 *      bcm_error_t = BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 */
int 
bcm_caladan3_policer_stat_enable_set(int unit, bcm_policer_t policer_id,
                                   int enable)
{
    bcm_policer_config_t    pol_cfg;
    int                     rv = BCM_E_NONE;
    int                     monitor_id;

    /* check if policer exists */
    rv = bcm_caladan3_policer_get(unit, policer_id, &pol_cfg);
    if (rv != BCM_E_NONE) {
        return rv;
    }

    /* only 0 & 1 are valid values for enable */
    if ((enable != 0) && (enable != 1)) {
        LOG_ERROR(BSL_LS_BCM_POLICER,
                  (BSL_META_U(unit,
                              "%s: Invalid enable value (%d). valid values - 0,1 \n"),
                   FUNCTION_NAME(), enable));
        return BCM_E_PARAM;
    }

    POLICER_UNIT_LOCK(unit);
    if (SOC_IS_SBX_G3P1(unit)) {
        /* check if this is base policer */
        if (!IS_BASE_POLICER(_policer[unit].policers[policer_id].group_mode)) {
            LOG_ERROR(BSL_LS_BCM_POLICER,
                      (BSL_META_U(unit,
                                  "%s: Invalid policer id. Not a base policer. \n"), 
                       FUNCTION_NAME()));
            rv = BCM_E_PARAM;
        }
    }
    if (rv == BCM_E_NONE) {
        if (_bcm_caladan3_is_stat_enabled(unit, policer_id, &monitor_id)) {
            if (enable == 0) {
                if (SOC_IS_SBX_G3P1(unit)) {
                    /* free the counters from the policers */
                    rv = _bcm_caladan3_g3p1_free_counters(unit, policer_id, 1, 0);
                } else {
                    /* default to using HW monitors */
                    /* detach monitor from policer */
                    rv = _bcm_caladan3_detach_monitor(unit, policer_id);
                }
            } /* else...do nothing. Already enabled */
        } else {
            if (enable == 1) {
                if (SOC_IS_SBX_G3P1(unit)) {
                    rv = _bcm_caladan3_g3p1_alloc_counters(unit, policer_id, 1, NULL);
                } else {
                    /* default to using HW monitors */
                    /* attach a monitor to policer */
                    rv = _bcm_caladan3_attach_monitor(unit, policer_id);
                }
            } /* else...do nothing. Already disabled */
        }
    }
    POLICER_UNIT_UNLOCK(unit);

    return rv;
}

/*
 *  Function
 *      bcm_caladan3_policer_stat_get
 *  Purpose
 *      gets the specified stat on the given policer id
 *  Parameters
 *      (in) unit           = unit number
 *      (in) policer_id     = policer id
 *      (in) stat           = stat type to get
 *      (out) val           = out value
 *  Returns
 *      bcm_error_t = BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 */
int
bcm_caladan3_policer_stat_get(int unit, bcm_policer_t policer_id, bcm_cos_t cos,
                            bcm_policer_stat_t stat, uint64 *val)
{
    bcm_policer_config_t    pol_cfg;
    int                     rv = BCM_E_NONE;
    int                     monitor_id, clear;

    POLICER_NULL_PARAM_CHECK(val);

    /* check if policer exists */
    rv = bcm_caladan3_policer_get(unit, policer_id, &pol_cfg);
    if (rv != BCM_E_NONE) {
        return rv;
    }

    POLICER_UNIT_LOCK(unit);
    if (_bcm_caladan3_is_stat_enabled(unit, policer_id, &monitor_id)) {
        if (SOC_IS_SBX_G3P1(unit)) {
            clear = 0; 
            rv = _bcm_caladan3_g3p1_policer_stat_get(unit, policer_id, stat,
                                                   cos, 1, 0, clear, val);
        } else {
            /* default to HW monitors */
            rv = _bcm_caladan3_monitor_stat_get(unit, monitor_id, stat, val);
        }
        if (rv != BCM_E_NONE) {
            LOG_ERROR(BSL_LS_BCM_POLICER,
                      (BSL_META_U(unit,
                                  "%s: Failed. \n"),
                       FUNCTION_NAME()));
        }
    } else {
        LOG_ERROR(BSL_LS_BCM_POLICER,
                  (BSL_META_U(unit,
                              "%s: Stats not enabled on policer id %d \n"), 
                   FUNCTION_NAME(), policer_id));
        rv = BCM_E_DISABLED;
    }
    POLICER_UNIT_UNLOCK(unit);

    return rv;
}

/*
 *  Function
 *      bcm_caladan3_policer_stat_get32
 *  Purpose
 *      gets the specified stat on the given policer id
 *  Parameters
 *      (in) unit           = unit number
 *      (in) policer_id     = policer id
 *      (in) stat           = stat type to get
 *      (out) val           = out value
 *  Returns
 *      bcm_error_t = BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 */
int 
bcm_caladan3_policer_stat_get32(int unit, bcm_policer_t policer_id, 
                              bcm_cos_t cos, bcm_policer_stat_t stat, 
                              uint32 *val)
{
    uint64  val64;
    int     rv = BCM_E_NONE;

    POLICER_NULL_PARAM_CHECK(val);

    rv = bcm_caladan3_policer_stat_get(unit, policer_id, cos, stat, &val64);
    if (rv == BCM_E_NONE) {
        if (COMPILER_64_HI(val64) > 0) {
            *val = 0xFFFFFFFF;
        } else {
            COMPILER_64_TO_32_LO(*val, val64);
        }
    }
    return rv;
}

/*
 *  Function
 *      bcm_caladan3_policer_stat_set
 *  Purpose
 *      sets the specified stat on the given policer id
 *  Parameters
 *      (in) unit           = unit number
 *      (in) policer_id     = policer id
 *      (in) stat           = stat type to set
 *      (in) val            = value to set to
 *  Returns
 *      bcm_error_t = BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 */
int 
bcm_caladan3_policer_stat_set(int unit, bcm_policer_t policer_id, bcm_cos_t cos,
                            bcm_policer_stat_t stat, uint64 val)
{
    bcm_policer_config_t    pol_cfg;
    int                     rv = BCM_E_NONE;
    int                     monitor_id;

    /* check if policer exists */
    rv = bcm_caladan3_policer_get(unit, policer_id, &pol_cfg);
    if (rv != BCM_E_NONE) {
        return rv;
    }

    POLICER_UNIT_LOCK(unit);
    if (_bcm_caladan3_is_stat_enabled(unit, policer_id, &monitor_id)) {
        if (SOC_IS_SBX_G3P1(unit)) {
            rv = _bcm_caladan3_g3p1_policer_stat_set(unit, policer_id, stat,
                                                   cos, 1, 0, val);
        } else {
            /* default to HW monitors */
            rv = _bcm_caladan3_monitor_stat_set(unit, monitor_id, stat, val);
        }
    } else {
        LOG_ERROR(BSL_LS_BCM_POLICER,
                  (BSL_META_U(unit,
                              "%s: Stats not enabled on policer id %d \n"), 
                   FUNCTION_NAME(), policer_id));
        rv = BCM_E_DISABLED;
    }
    POLICER_UNIT_UNLOCK(unit);

    return rv;
}

/*
 *  Function
 *      bcm_caladan3_policer_stat_set32
 *  Purpose
 *      sets the specified stat on the given policer id
 *  Parameters
 *      (in) unit           = unit number
 *      (in) policer_id     = policer id
 *      (in) stat           = stat type to set
 *      (in) val            = value to set to
 *  Returns
 *      bcm_error_t = BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 */
int 
bcm_caladan3_policer_stat_set32(int unit, bcm_policer_t policer_id, 
                              bcm_cos_t cos, bcm_policer_stat_t stat, 
                              uint32 val)
{
    uint64  val64;
    int     rv;

    COMPILER_64_SET(val64, 0, val);
    rv = bcm_caladan3_policer_stat_set(unit, policer_id, cos, stat, val64);

    return rv;
}

const char *
_bcm_caladan3_policer_group_mode_to_str(bcm_policer_group_mode_t groupMode) 
{
    static const char* strs[] =
        { "bcmPolicerGroupModeSingle",
          "bcmPolicerGroupModeTrafficType",
          "bcmPolicerGroupModeDlfAll",
          "bcmPolicerGroupModeDlfIntPri",
          "bcmPolicerGroupModeTyped",
          "bcmPolicerGroupModeTypedAll",
          "bcmPolicerGroupModeTypedIntPri"} ;

    if (groupMode >= 0 && groupMode <= bcmPolicerGroupModeTypedIntPri) {
        return strs[groupMode];
    }
    return "<unknown>";
}

/*
 * Function
 *      _bcm_caladan3_g3p1_num_counters_get
 * Purpose
 *      Returns the number of counters in this policer group mode
 * Parameters
 *      (in) unit       = unit number
 *      (in) mode       = group mode 
 *      (out)ncounters  = number of counters in this mode
 * Returns
 *      bcm_error_t         = BCM_E_* appropriately
 */
int
_bcm_caladan3_g3p1_num_policer_counters_get(int unit,
                                          bcm_policer_t policer_id,
                                          int *ncounters)
{
    int                     rv = BCM_E_NONE;
    bcm_policer_config_t    pol_cfg;

    /* check if policer exists */
    rv = bcm_caladan3_policer_get(unit, policer_id, &pol_cfg);
    if (BCM_SUCCESS(rv) && ncounters) {
        bcm_policer_group_mode_t  grp_mode;
        POLICER_UNIT_LOCK(unit);

        grp_mode = POLICER_GROUP_MODE_GET(unit, policer_id);
        rv = _bcm_caladan3_g3p1_num_counters_get(unit, grp_mode, ncounters);

        POLICER_UNIT_UNLOCK(unit);
        
    } else {
        rv = BCM_E_PARAM;
    }
    return rv;
}

