
/*
 * $Id: stg.c,v 1.60 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:        stg.c
 * Purpose:     Spanning tree group support
 *
 * Multiple spanning trees (MST) is supported in
 * BCM5690, BCM5324 etc.  STGs are created and VLANs may be added 
 * to each STG.
 *
 * Per-VLAN spanning tree (PVST) is supported in BCM5632.  This module
 * supports PVST on BCM5632 by having 4k virtual STGs and allowing only
 * a single VLAN per STG.  Before the application can create a second
 * STG, it must remove all but one VLAN from the default STG.
 */

#include <shared/bsl.h>

#include <sal/core/libc.h>

#include <shared/bitop.h>

#include <soc/drv.h>
#include <soc/mem.h>
#include <soc/debug.h>

#include <bcm/types.h>
#include <bcm/error.h>
#include <bcm/port.h>
#include <bcm/stg.h>
#include <bcm/vlan.h>

#include <bcm_int/common/lock.h>
#include <bcm_int/control.h>
#include <bcm_int/robo/stg.h>
#include <bcm_int/robo/vlan.h>

#define VLAN_NULL		0
/*
 * The STG info structure is protected by BCM_LOCK.
 * The hardware PTABLE and hardware STG table are protected by
 * memory locks in the lower level.
 */

typedef struct bcm_stg_info_s {
    int		init;		/* TRUE if STG module has been initialized */
    soc_mem_t	stg_mem;	/* STG table memory, INVALIDm if none */
    bcm_stg_t	stg_min;	/* STG table min index */
    bcm_stg_t	stg_max;	/* STG table max index */
    bcm_stg_t	stg_defl;	/* Default STG */
    SHR_BITDCL	*stg_bitmap;	/* Bitmap of allocated STGs */
    int		stg_count;	/* Number STGs allocated */

    /* STG reverse map - keep a linked list of VLANs in each STG */
    bcm_vlan_t	*vlan_first;	/* Indexed by STG (also links free list) */
    bcm_vlan_t	*vlan_next;	/* Indexed by VLAN ID */
} bcm_stg_info_t;

static bcm_stg_info_t robo_stg_info[BCM_MAX_NUM_UNITS];


#ifdef STP_BY_EAP_BLK

static sal_mutex_t stp_8021x_state_Mutex[BCM_MAX_NUM_UNITS] = NULL;
static int bcm_shadow_stg_status[BCM_MAX_NUM_UNITS][SOC_MAX_NUM_PORTS];
/* First array is STP, the other is 802.1x */
int stp_8021x_block_state[BCM_MAX_NUM_UNITS][2][SOC_MAX_NUM_PORTS]; /* block = 1, non-block = 0 */
extern bcm_pbmp_t  bcm5324_pvlan_value[BCM_MAX_NUM_UNITS][SOC_MAX_NUM_PORTS];
bcm_pbmp_t  stp_non_forward_pbmp[BCM_MAX_NUM_UNITS];
void bcm_robo_stp_8021x_lock(int unit);
void bcm_robo_stp_8021x_unlock(int unit);
void bcm_robo_port_vlan_remove(int unit, bcm_port_t rem_port);
void bcm_robo_port_vlan_add(int unit, bcm_port_t add_port);
#endif /* STP_BY_EAP_BLK */

#define CHECK_INIT(unit, si) do {				\
	    if (!SOC_UNIT_VALID(unit)) return BCM_E_UNIT;	\
	    if (si->init == 0) return BCM_E_INIT;		\
	    if (si->init < 0) return si->init;			\
	} while (0);


#define CHECK_STG(si, stg)   					\
        if ((stg) < si->stg_min || (stg) > si->stg_max) 	\
	    return BCM_E_BADID

/*
 * Allocation bitmap macros
 */
#define STG_BITMAP_TST(si, stg)		SHR_BITGET(si->stg_bitmap, stg)
#define STG_BITMAP_SET(si, stg)		SHR_BITSET(si->stg_bitmap, stg)
#define STG_BITMAP_CLR(si, stg)		SHR_BITCLR(si->stg_bitmap, stg)

/* 
 * STG-to-VLAN Reverse Map
 *
 *   The Spanning Tree Group map is a datastructure that forms one
 *   linked list per spanning tree group to keep track of all the VLANs
 *   in each Spanning Tree Group.  Otherwise, to answer the question
 *   'what VLANs are in STG x' would require searching all 4k entries of
 *   the VLAN table on 5690.
 *
 *   vlan_first[stg_num] is the number of the first VLAN in STG stg_num.
 *
 *   vlan_next[vlan_id] has one entry per VLAN ID and contains the number
 *   of the next VLAN in the STG that contains vlan_id.
 */

/* ---------------- Internal Routines ---------------- */

extern int _drv_mstp_enable_set(int unit, int enable);

/*
 * Function:
 *	_bcm_robo_stg_rmap_add (internal)
 * Purpose:
 *	Add VLAN to STG linked list
 */

STATIC void
_bcm_robo_stg_rmap_add(int unit, bcm_stg_t stg, bcm_vlan_t vid)
{
    bcm_stg_info_t	*si = &robo_stg_info[unit];

    si->vlan_next[vid] = si->vlan_first[stg];
    si->vlan_first[stg] = vid;
}

/*
 * Function:
 *	_bcm_robo_stg_rmap_delete (internal)
 * Purpose:
 *	Remove VLAN from STG linked list; return boolean TRUE if found
 */

STATIC void
_bcm_robo_stg_rmap_delete(int unit, bcm_stg_t stg, bcm_vlan_t vid)
{
    bcm_stg_info_t	*si = &robo_stg_info[unit];
    bcm_vlan_t	  *vp;

    assert(vid != VLAN_NULL);

    vp = &si->vlan_first[stg];

    while (*vp != VLAN_NULL) {
        if (*vp == vid) {
            *vp = si->vlan_next[*vp];
        } else {
            vp = &si->vlan_next[*vp];
        }
    }
}


/*
 * Function:
 *	_bcm_robo_stg_vid_compar
 * Purpose:
 *	Internal utility routine for sorting on VLAN ID.
 */

STATIC int
_bcm_robo_stg_vid_compar(void *a, void *b)
{
    uint16	a16, b16;

    a16 = *(uint16 *)a;
    b16 = *(uint16 *)b;

    if (a16 < b16) return -1;
    if (a16 > b16) return 1;
    return 0;
}

/*
 * Function:
 *	_bcm_robo_stg_stp_set
 * Purpose:
 *	Set the spanning tree state for a port in specified STG.
 * Parameters:
 *	unit - RoboSwitch unit number.
 *      stg - STG ID.
 *	port - RoboSwitch port number.
 *	stp_state - State to place port in.
 */

int
_bcm_robo_stg_stp_set(int unit, bcm_stg_t stg, 
                bcm_port_t port, int stp_state)
{
#ifdef STP_BY_EAP_BLK
    uint32  temp;
    uint64  reg_value64;
    uint32  ctrl_reg_value;
#ifdef BCM_POLAR_SUPPORT
    uint32 internal_cpu_port_num;

    if (SOC_IS_POLAR(unit)) {
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->dev_prop_get)
                (unit, DRV_DEV_PROP_INTERNAL_CPU_PORT_NUM, 
                 &internal_cpu_port_num));
    }

    if (SOC_IS_POLAR(unit) && (port == internal_cpu_port_num)) {
        SOC_IF_ERROR_RETURN(REG_READ_PORT_EAP_CON_P7r
            (unit, (uint32 *)&reg_value64));
    } else 
#endif /* BCM_POLAR_SUPPORT */
    {
        SOC_IF_ERROR_RETURN(REG_READ_PORT_EAP_CONr
            (unit, port, (uint32 *)&reg_value64));
    }
    
    SOC_IF_ERROR_RETURN(REG_READ_EAP_GLO_CONr
        (unit, &ctrl_reg_value));

    switch(stp_state) {
        case BCM_STG_STP_DISABLE:
            temp = 1;
            SOC_IF_ERROR_RETURN(soc_PORT_EAP_CONr_field_set
                (unit, (uint32 *)&reg_value64, EAP_BLK_MODEf, &temp));
            SOC_IF_ERROR_RETURN(soc_PORT_EAP_CONr_field_set
                (unit, (uint32 *)&reg_value64, EAP_ENf, &temp));
            /*
             * For disable state, this port should not any receive BPDUs.
             * Since this register is global, we can't configure it per-port.
             * So we enable it, to prevent other ports in blocking didn't
             * any BPDUs
             */
            temp = 1;
            if (SOC_IS_VULCAN(unit)){
                SOC_IF_ERROR_RETURN(soc_EAP_GLO_CONr_field_set
                    (unit, &ctrl_reg_value, EN_BPDUf, &temp));
            } else {
                SOC_IF_ERROR_RETURN(soc_EAP_GLO_CONr_field_set
                    (unit, &ctrl_reg_value, EN_MAC_BPDUf, &temp));
            }
            if (SOC_IS_VULCAN(unit)){
                SOC_IF_ERROR_RETURN(soc_EAP_GLO_CONr_field_set
                    (unit, &ctrl_reg_value, EN_RMCf, &temp));
            } else {
                SOC_IF_ERROR_RETURN(soc_EAP_GLO_CONr_field_set
                    (unit, &ctrl_reg_value, EN_MAC_02_04_0Ff, &temp));
            }
            bcm_robo_port_vlan_remove(unit, port);
            bcm_robo_stp_8021x_lock(unit);
            stp_8021x_block_state[unit][0][port] = 1; /* STP block */
            bcm_robo_stp_8021x_unlock(unit);
            break;
        case BCM_STG_STP_BLOCK:
        case BCM_STG_STP_LISTEN:
        case BCM_STG_STP_LEARN:
            if (SOC_IS_VULCAN(unit)){
                temp = 3;   /* enable the simplified mode */
                SOC_IF_ERROR_RETURN(soc_PORT_EAP_CONr_field_set
                    (unit, (uint32 *)&reg_value64, EAP_MODEf, &temp));
            } else {
                temp = 1;
                SOC_IF_ERROR_RETURN(soc_PORT_EAP_CONr_field_set
                    (unit, (uint32 *)&reg_value64, EAP_ENf, &temp));
            }
            temp = 1;
            SOC_IF_ERROR_RETURN(soc_PORT_EAP_CONr_field_set
                (unit, (uint32 *)&reg_value64, EAP_BLK_MODEf, &temp));
            if (SOC_IS_VULCAN(unit)){
                SOC_IF_ERROR_RETURN(soc_EAP_GLO_CONr_field_set
                    (unit, &ctrl_reg_value, EN_BPDUf, &temp));
            } else {
                SOC_IF_ERROR_RETURN(soc_EAP_GLO_CONr_field_set
                    (unit, &ctrl_reg_value, EN_MAC_BPDUf, &temp));
            }

            if (SOC_IS_VULCAN(unit)){
                SOC_IF_ERROR_RETURN(soc_EAP_GLO_CONr_field_set
                    (unit, &ctrl_reg_value, EN_RMCf, &temp));
            } else {
                SOC_IF_ERROR_RETURN(soc_EAP_GLO_CONr_field_set
                    (unit, &ctrl_reg_value, EN_MAC_02_04_0Ff, &temp));
            }
            bcm_robo_port_vlan_remove(unit, port);
            bcm_robo_stp_8021x_lock(unit);
            stp_8021x_block_state[unit][0][port] = 1; /* STP block */
            bcm_robo_stp_8021x_unlock(unit);
            break;
        case BCM_STG_STP_FORWARD:
            BCM_IF_ERROR_RETURN(DRV_MSTP_PORT_SET
                    (unit, stg, port, DRV_PORTST_FORWARD));
            bcm_robo_port_vlan_add(unit, port);
            bcm_robo_stp_8021x_lock(unit);
            if (stp_8021x_block_state[unit][1][port]) {
                temp = 1;
            } else {
                temp = 0;
            }
            stp_8021x_block_state[unit][0][port] = 0; /* STP non-block */
            bcm_robo_stp_8021x_unlock(unit);
            SOC_IF_ERROR_RETURN(soc_PORT_EAP_CONr_field_set
                (unit, (uint32 *)&reg_value64, EAP_BLK_MODEf, &temp));
            temp = 1;
            SOC_IF_ERROR_RETURN(soc_PORT_EAP_CONr_field_set
                (unit, (uint32 *)&reg_value64, EAP_ENf, &temp));
            break;
        default:
            return BCM_E_PARAM;
            
    }
#ifdef BCM_POLAR_SUPPORT
    if (SOC_IS_POLAR(unit) && (port == internal_cpu_port_num)) {
        SOC_IF_ERROR_RETURN(REG_WRITE_PORT_EAP_CON_P7r
            (unit, (uint32 *)&reg_value64));
    } else 
#endif /* BCM_POLAR_SUPPORT */
    {
        SOC_IF_ERROR_RETURN(REG_WRITE_PORT_EAP_CONr
            (unit, port, (uint32 *)&reg_value64));
    }
    SOC_IF_ERROR_RETURN(REG_WRITE_EAP_GLO_CONr
        (unit, &ctrl_reg_value));
    bcm_shadow_stg_status[unit][port] = stp_state;


    return BCM_E_NONE;
#else /* !STP_BY_EAP_BLK */
    uint32      port_status;

    switch(stp_state) {
        case BCM_STG_STP_DISABLE:
            port_status = DRV_PORTST_DISABLE;
            break;
        case BCM_STG_STP_BLOCK:
            port_status = DRV_PORTST_BLOCK;
            break;
        case BCM_STG_STP_LISTEN:
            port_status = DRV_PORTST_LISTEN;
            break;
        case BCM_STG_STP_LEARN:
            port_status = DRV_PORTST_LEARN;
            break;
        case BCM_STG_STP_FORWARD:
            port_status = DRV_PORTST_FORWARD;
            break;
        default:
            return BCM_E_PARAM;
    }

    BCM_IF_ERROR_RETURN(DRV_MSTP_PORT_SET
                    (unit, stg, port, port_status));


    return BCM_E_NONE;
#endif /* STP_BY_EAP_BLK */
}

/*
 * Function:
 *	_bcm_robo_stg_stp_get
 * Purpose:
 *	Retrieve the spanning tree state for a port in specified STG.
 * Parameters:
 *	unit - RoboSwitch unit number.
 *      stg - STG ID.
 *	port - RoboSwitch port number.
 *	stp_state - Pointer where state stored.
 */
int
_bcm_robo_stg_stp_get(int unit, bcm_stg_t stg, 
                bcm_port_t port, int *stp_state)
{
#ifdef STP_BY_EAP_BLK

    if ((stg > 1) || (stg < 0)) {
        return SOC_E_PARAM;
    }
    *stp_state = bcm_shadow_stg_status[unit][port];
    
#else /* !STP_BY_EAP_BLK */
    uint32      port_status;

    BCM_IF_ERROR_RETURN(DRV_MSTP_PORT_GET
         (unit, stg, port, &port_status));

    *stp_state = (port_status == DRV_PORTST_DISABLE) ? BCM_STG_STP_DISABLE :
             (port_status == DRV_PORTST_BLOCK) ? BCM_STG_STP_BLOCK :
             (port_status == DRV_PORTST_LEARN) ? BCM_STG_STP_LEARN :
             (port_status == DRV_PORTST_FORWARD) ? BCM_STG_STP_FORWARD :
             BCM_STG_STP_LISTEN;

#endif /* STP_BY_EAP_BLK */

    return BCM_E_NONE;
}			   


/*
 * Function:
 *	_bcm_robo_stg_vlan_add
 * Purpose:
 *	Main part of bcm_stg_vlan_add; assumes locks already done.
 */

STATIC int
_bcm_robo_stg_vlan_add(int unit, bcm_stg_t stg, bcm_vlan_t vid)
{
    bcm_stg_info_t  *si = &robo_stg_info[unit];
    bcm_stg_t  stg_cur;

    /* STG must already exist */

    if (!STG_BITMAP_TST(si, stg)) {
        return BCM_E_NOT_FOUND;
    }

    BCM_IF_ERROR_RETURN(bcm_vlan_stg_get
        (unit, vid, &stg_cur));

    /*
     * If this is being called from bcm_vlan_create(), the rmap will not
     * contain the VLAN but this will not hurt.
     */

    if ((stg_cur != stg) || 
        (stg_cur == BCM_STG_DEFAULT && stg == BCM_STG_DEFAULT)) {
        _bcm_robo_stg_rmap_delete(unit, stg_cur, vid);

        BCM_IF_ERROR_RETURN(_bcm_robo_vlan_mstp_config_set(unit, vid, stg));

        _bcm_robo_stg_rmap_add(unit, stg, vid);
    }

    return BCM_E_NONE;
}

/*
 * Function:
 *	_bcm_robo_stg_vlan_remove
 * Purpose:
 *	Main part of bcm_stg_vlan_remove; assumes lock already done.
 * Returns:
 *      BCM_E_XXX
 */

STATIC int
_bcm_robo_stg_vlan_remove(int unit, bcm_stg_t stg, bcm_vlan_t vid, int destroy)
{
    bcm_stg_info_t  *si = &robo_stg_info[unit];
    int  stg_cur;

    /* STG must already exist */

    if (!STG_BITMAP_TST(si, stg)) {
	    return BCM_E_NOT_FOUND;
    }

    BCM_IF_ERROR_RETURN(bcm_vlan_stg_get
        (unit, vid, &stg_cur));

    if (stg != stg_cur) {
        return BCM_E_NOT_FOUND;	/* Not found in specified STG */
    }

    BCM_IF_ERROR_RETURN(_bcm_robo_vlan_mstp_config_set(unit, vid, si->stg_defl));

    _bcm_robo_stg_rmap_delete(unit, stg, vid);

    if (!destroy) {
	    _bcm_robo_stg_rmap_add(unit, si->stg_defl, vid);
    }

    return BCM_E_NONE;
}

/*
 * Function:
 *	_bcm_robo_stg_vlan_destroy
 * Purpose:
 *	Remove a VLAN from a spanning tree group.
 *	The VLAN is NOT placed in the default spanning tree group.
 *      Intended for use when the VLAN is destroyed.
 * Parameters:
 *      unit - RoboSwitch PCI device unit number
 *	stg - STG ID to use
 *      vid - VLAN id to be removed from STG
 * Returns:
 *      BCM_E_XXX
 */

int
_bcm_robo_stg_vlan_destroy(int unit, bcm_stg_t stg, bcm_vlan_t vid)
{
    bcm_stg_info_t	*si = &robo_stg_info[unit];
    int			rv;

    CHECK_INIT(unit, si);
    CHECK_STG(si, stg);
    _ROBO_VLAN_CHK_ID(unit, vid);

    rv = _bcm_robo_stg_vlan_remove(unit, stg, vid, TRUE);

    return rv;
}

/*
 * Function:
 *	_bcm_robo_stg_stp_init
 * Purpose:
 *	Write an entry with the spanning tree state DISABLE for all ports.
 */

int
_bcm_robo_stg_stp_init(int unit, bcm_stg_t stg)
{
    bcm_pbmp_t  bmp;
    bcm_port_t  port;

    bmp = PBMP_ALL(unit);

#if defined(BCM_NORTHSTAR_SUPPORT)||defined(BCM_NORTHSTARPLUS_SUPPORT)
    if (SOC_IS_NORTHSTAR(unit)||SOC_IS_NORTHSTARPLUS(unit)) {
        /* include internal ports, incase they are not in valid pbmp */
        BCM_PBMP_PORT_ADD(bmp, 5);
        BCM_PBMP_PORT_ADD(bmp, 7);
    }
#endif /* BCM_NORTHSTAR_SUPPORT||BCM_NORTHSTARPLUS_SUPPORT */

    PBMP_ITER(bmp, port){
#ifdef BCM_POLAR_SUPPORT
        if (SOC_IS_POLAR(unit) && soc_property_port_get(unit, port, spn_BCM89500_RAPID_BOOT, 0)) {
            BCM_IF_ERROR_RETURN(_bcm_robo_stg_stp_set(unit, stg, 
                        port, BCM_STG_STP_FORWARD));
        } else 
#endif /* BCM_POLAR_SUPPORT */
        {
            if (SOC_IS_NORTHSTAR(unit)||SOC_IS_NORTHSTARPLUS(unit)) {
                BCM_IF_ERROR_RETURN(_bcm_robo_stg_stp_set(unit, stg,
                        port, BCM_STG_STP_FORWARD));
            } else {
                BCM_IF_ERROR_RETURN(_bcm_robo_stg_stp_set(unit, stg, 
                        port, BCM_STG_STP_DISABLE));
            }
        }
    }

   /*
     * For Robo5324/5396 , CPU port should be set as FORWARD by driver.
     */
    if (soc_feature(unit, soc_feature_stg)) {
        bmp = PBMP_CMIC(unit);
        if (SOC_IS_VO(unit)) {
            /* add port 26 which connected with EPON in FWD state*/ 
            BCM_PBMP_OR(bmp, SOC_INFO(unit).s_pbm);
        }
        PBMP_ITER(bmp, port){
            BCM_IF_ERROR_RETURN(_bcm_robo_stg_stp_set(unit, stg, 
                        port, BCM_STG_STP_FORWARD));
        }
    }

        /* enable 802.1s */
    if (soc_feature(unit, soc_feature_mstp)) {
        /* 
         * For NS and NS+, traffic between CPU and network goes through switch.
         * When load SDK through NFS, it is possible that MSTP is enabled here
         * but stp group id of default VLAN doesn't set in time. 
         * Move the MSTP enable to VLAN initialization code, after the default 
         * VLAN is created successfully. 
         */
        if (!(SOC_IS_NORTHSTAR(unit)||SOC_IS_NORTHSTARPLUS(unit))) {
            BCM_IF_ERROR_RETURN(_drv_mstp_enable_set(unit, TRUE));
        }
    }
    
    return BCM_E_NONE;
}

/* ================ Internal Routines ================ */

/*
 * Function:
 *	bcm_stg_init
 * Description:
 *      Initialize the STG module according to Initial System Configuration.
 * Parameters:
 *      unit - RoboSwitch PCI device unit number (driver internal).
 * Returns:
 *      BCM_E_XXX
 */
int bcm_robo_stg_init(int unit)
{
    bcm_stg_info_t  *si = &robo_stg_info[unit];
    int  alloc_size;
    uint32  drv_value;

    LOG_INFO(BSL_LS_BCM_STP,
             (BSL_META_U(unit,
                         "BCM API : bcm_robo_stg_init()..\n")));
    if (!SOC_UNIT_VALID(unit)) {
        return BCM_E_UNIT;
    }
    
    /* Destroy the STG databse if it has been initialized */
    if (si->init == TRUE) {
        BCM_IF_ERROR_RETURN(bcm_robo_stg_detach(unit));
    }

    /* direct assign the stg table for there is no other integrated currently 
     *   - if integrate other chip is requested, stg_mem assignment may 
     *      through soc_feature().
     */
    if (SOC_IS_STARFIGHTER3(unit)) {
        si->stg_mem = INVALIDm;
    } else {
        si->stg_mem = INDEX(MSPT_TABm);
    }
    
    /* Get the device property on MSTP number */
    si->stg_min = 1;
    if (!soc_feature(unit, soc_feature_mstp)) {
        si->stg_max = 1;
    } else {
        BCM_IF_ERROR_RETURN(DRV_DEV_PROP_GET
            (unit, DRV_DEV_PROP_MSTP_NUM, &drv_value));
        si->stg_max = drv_value;
    }

    alloc_size = SHR_BITALLOCSIZE(si->stg_max+1);
    if (si->stg_bitmap == NULL) {
        si->stg_bitmap = sal_alloc(alloc_size, "STG-bitmap");
    }

    if (si->vlan_first == NULL) {
        si->vlan_first = sal_alloc((si->stg_max + 1) * sizeof (bcm_vlan_t),
        			   "STG-vfirst");
    }

    if (si->vlan_next == NULL) {
        si->vlan_next = sal_alloc(BCM_VLAN_COUNT * sizeof (bcm_vlan_t),
        			  "STG-vnext");
    }

    if (si->stg_bitmap == NULL ||
                si->vlan_first == NULL ||
                si->vlan_next == NULL) {
        if (si->stg_bitmap != NULL) {
            sal_free(si->stg_bitmap);
            si->stg_bitmap = NULL;
        }

        if (si->vlan_first != NULL) {
            sal_free(si->vlan_first);
            si->vlan_first = NULL;
        }

        if (si->vlan_next != NULL) {
            sal_free(si->vlan_next);
            si->vlan_next = NULL;
        }

        return BCM_E_MEMORY;
    }

	/* Memory buffer clear to default. */
    sal_memset(si->stg_bitmap, 0, alloc_size);
    sal_memset(si->vlan_first, 0, (si->stg_max + 1) * sizeof (bcm_vlan_t));
    sal_memset(si->vlan_next, 0, BCM_VLAN_COUNT * sizeof (bcm_vlan_t));

    si->stg_count = 0;

    /*
     * For practical ease of use, the default STG is always 1, even if
     * STG 0 is valid for the chip.
     */

    si->stg_defl = BCM_STG_DEFAULT;

    assert(si->stg_defl >= si->stg_min && si->stg_defl <= si->stg_max);

    si->init = TRUE;	/* > 0 */

#ifdef STP_BY_EAP_BLK
    {
        int i, port;
        
        BCM_PBMP_CLEAR(stp_non_forward_pbmp[unit]);
        if (stp_8021x_state_Mutex[unit] != NULL) {
            sal_mutex_destroy(stp_8021x_state_Mutex[unit]);
            stp_8021x_state_Mutex[unit] = NULL;
        }
        stp_8021x_state_Mutex[unit] = sal_mutex_create("STP_8021X");
        if (stp_8021x_state_Mutex[unit] == NULL) {
            return BCM_E_MEMORY;
        }
        for (i=0; i < SOC_MAX_NUM_PORTS; i++) {
            bcm_shadow_stg_status[unit][i] = BCM_STG_STP_FORWARD;
            stp_8021x_block_state[unit][0][i] = 0; /* Non-block */
            stp_8021x_block_state[unit][1][i] = 0; /* Non-block */
            
        }
        /*
         * Since the default of PORT VLAN CONTROL registers are 
         * different from others', we need to put all ports into this register
         */
        if (SOC_IS_ROBO5324(unit)) {
            BCM_PBMP_ITER(PBMP_ALL(unit), port){
                bcm5324_pvlan_value[unit][port] = PBMP_ALL(unit);
            }
        }
        
    }
#endif /* STP_BY_EAP_BLK */
    
    /*
     * Create default STG and add all VLANs to it.  Calling this routine
     * is safe because it does not reference other BCM driver modules.
     */

    BCM_IF_ERROR_RETURN(bcm_stg_create_id(unit, si->stg_defl));

    _bcm_robo_stg_rmap_add(unit, si->stg_defl, BCM_VLAN_DEFAULT);

    return BCM_E_NONE;
}				

int 
bcm_robo_stg_clear(int unit)
{
    bcm_stg_info_t  *si = &robo_stg_info[unit];
    bcm_stg_t  stg;

    LOG_INFO(BSL_LS_BCM_STP,
             (BSL_META_U(unit,
                         "BCM API : bcm_robo_stg_clear()..\n")));
    CHECK_INIT(unit, si);

    for (stg = si->stg_min; stg <= si->stg_max; stg++) {
        if (STG_BITMAP_TST(si, stg)) {
                bcm_stg_destroy(unit, stg);
        }
    }

    return bcm_stg_init(unit);
}				

int bcm_robo_stg_detach(int unit)
{
    bcm_stg_info_t	*si = &robo_stg_info[unit];
    bcm_stg_t  stg;
    int rv = BCM_E_NONE;

    if ((si->init == 0) || (si->init < 0)) {
        return BCM_E_NONE;
    }

    CHECK_INIT(unit, si);

    for (stg = si->stg_min; stg <= si->stg_max; stg++) {
        if (STG_BITMAP_TST(si, stg)) {
            rv = bcm_stg_destroy(unit, stg);
        }
    }

    if (BCM_SUCCESS(rv))
    {
        sal_free(si->vlan_next);
        sal_free(si->vlan_first);
        sal_free(si->stg_bitmap);

        si->vlan_next = NULL;
        si->vlan_first = NULL;
        si->stg_bitmap = NULL;
    }

    si->init = 0;

    return BCM_E_NONE;
}

/*
 * Function:
 *	bcm_robo_stg_default_get
 * Purpose:
 *	Returns the default STG for the chip, usually 0 or 1 depending
 *	on the architecture.
 * Parameters:
 *      unit - RoboSwitch PCI device unit number.
 *	stg_ptr - (OUT) STG ID for default.
 * Returns:
 *      BCM_E_XXX
 */
int 
bcm_robo_stg_default_get(int unit, bcm_stg_t *stg_ptr)
{
    bcm_stg_info_t	*si = &robo_stg_info[unit];

    LOG_INFO(BSL_LS_BCM_STP,
             (BSL_META_U(unit,
                         "BCM API : bcm_robo_stg_default_get()..\n")));
    CHECK_INIT(unit, si);

    *stg_ptr = si->stg_defl;

    return BCM_E_NONE;
}				
	
/*
 * Function:
 *	bcm_robo_stg_default_set
 * Purpose:
 *	Changes the default STG for the chip.
 * Parameters:
 *      unit - RoboSwitch PCI device unit number.
 *	stg - STG ID to become default.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 *	The specified STG must already exist.
 */
int 
bcm_robo_stg_default_set(int unit, bcm_stg_t stg)
{
    bcm_stg_info_t	*si = &robo_stg_info[unit];

    CHECK_INIT(unit, si);
    CHECK_STG(si, stg);

    LOG_INFO(BSL_LS_BCM_STP,
             (BSL_META_U(unit,
                         "BCM API : bcm_robo_stg_default_set()..\n")));
    if (!STG_BITMAP_TST(si, stg)) {
        return BCM_E_NOT_FOUND;
    }

    if (!soc_feature(unit, soc_feature_mstp)) {
        si->stg_max = BCM_STG_DEFAULT;
    }
    else {
        si->stg_defl = stg;
    }

    return BCM_E_NONE;
}				
	
/*
 * Function:
 *	bcm_robo_stg_vlan_add
 * Purpose:
 *	Add a VLAN to a spanning tree group.
 * Parameters:
 *      unit - RoboSwitch PCI device unit number
 *	stg - STG ID to use
 *      vid - VLAN id to be added to STG
 * Returns:
 *      BCM_E_XXX
 * Notes:
 *	Spanning tree group ID must have already been created.
 *	The VLAN is removed from the STG it is currently in.
 */
int 
bcm_robo_stg_vlan_add(int unit, bcm_stg_t stg, bcm_vlan_t vid)
{
    bcm_stg_info_t  *si = &robo_stg_info[unit];
    int  rv = 0;

    LOG_INFO(BSL_LS_BCM_STP,
             (BSL_META_U(unit,
                         "BCM API : bcm_robo_stg_vlan_add()..\n")));
    CHECK_INIT(unit, si);
    CHECK_STG(si, stg);
    _ROBO_VLAN_CHK_ID(unit, vid);

    rv = _bcm_robo_stg_vlan_add(unit, stg, vid);

    return rv;
}				
	
/*
 * Function:
 *	bcm_robo_stg_vlan_remove
 * Purpose:
 *	Remove a VLAN from a spanning tree group.
 *	The VLAN is placed in the default spanning tree group.
 * Parameters:
 *      unit - RoboSwitch PCI device unit number
 *	stg - STG ID to use
 *      vid - VLAN id to be removed from STG
 * Returns:
 *      BCM_E_XXX
 */
int 
bcm_robo_stg_vlan_remove(int unit, bcm_stg_t stg, bcm_vlan_t vid)
{
   bcm_stg_info_t  *si = &robo_stg_info[unit];
    int  rv;

    LOG_INFO(BSL_LS_BCM_STP,
             (BSL_META_U(unit,
                         "BCM API : bcm_robo_stg_vlan_remove()..\n")));
    CHECK_INIT(unit, si);
    CHECK_STG(si, stg);
    _ROBO_VLAN_CHK_ID(unit, vid);

    rv = _bcm_robo_stg_vlan_remove(unit, stg, vid, FALSE);

    return rv;
}				

/*
 * Function:
 *  bcm_robo_stg_vlan_remove_all
 * Purpose:
 *  Remove all VLAN from a spanning tree group.
 *  The VLANs are placed in the default spanning tree group.
 * Parameters:
 *      unit - RoboSwitch PCI device unit number
 *  stg - STG ID to clear
 * Returns:
 *      BCM_E_XXX
 */
int 
bcm_robo_stg_vlan_remove_all(int unit, bcm_stg_t stg)
{
    bcm_stg_info_t  *si = &robo_stg_info[unit];
    int  rv = 0;
    bcm_vlan_t	  vid;

    LOG_INFO(BSL_LS_BCM_STP,
             (BSL_META_U(unit,
                         "BCM API : bcm_robo_stg_vlan_remove_all()..\n")));
    CHECK_INIT(unit, si);
    CHECK_STG(si, stg);

    if (!STG_BITMAP_TST(si, stg)) {
        return BCM_E_NOT_FOUND;
    }

    if (stg == si->stg_defl) {		/* Null operation */
        return BCM_E_NONE;
    }

    /* The next call already checks if STG exists */

    while ((vid = si->vlan_first[stg]) != VLAN_NULL) {
        if ((rv = _bcm_robo_stg_vlan_remove(unit, stg, vid, FALSE)) < 0) {
            return rv;
        }
        
        if ((rv = _bcm_robo_stg_vlan_add(unit, si->stg_defl, vid)) < 0) {
            return rv;
        }
    }

    return rv;

}				

/*
 * Function:
 *	bcm_robo_stg_vlan_list
 * Purpose:
 *	Return a list of VLANs in a specified STG.
 * Parameters:
 *      unit - RoboSwitch PCI device unit number
 *	stg - STG ID to list
 *      list - Place where pointer to return array will be stored,
 *              which will be NULL if there are zero VLANs returned.
 *      count - Place where number of entries in array will be stored,
 *              which will be 0 if there are zero VLANs returned.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 *	The caller is responsible for freeing the memory that is
 *	returned, using bcm_stg_vlan_list_destroy().
 */
int 
bcm_robo_stg_vlan_list(int unit, bcm_stg_t stg,
			     bcm_vlan_t **list, int *count)
{
    bcm_stg_info_t  *si = &robo_stg_info[unit];
    bcm_vlan_t	  v;
    int  n;

    LOG_INFO(BSL_LS_BCM_STP,
             (BSL_META_U(unit,
                         "BCM API : bcm_robo_stg_vlan_list()..\n")));
    CHECK_INIT(unit, si);
    CHECK_STG(si, stg);

    *list = NULL;
    *count = 0;

    if (!STG_BITMAP_TST(si, stg)) {
	return BCM_E_NOT_FOUND;
    }

    /* Traverse list once just to get an allocation count */

    v = si->vlan_first[stg];

    while (v != VLAN_NULL) {
	(*count)++;
	v = si->vlan_next[v];
    }

    if (*count == 0) {
	return BCM_E_NONE;		/* Return empty list */
    }

    /* Traverse list a second time to record the VLANs */

    *list = sal_alloc(*count * sizeof (bcm_vlan_t), "bcm_stg_vlan_list");
    if (*list == NULL) {
	return BCM_E_MEMORY;
    }

    v = si->vlan_first[stg];
    n = 0;

    while (v != VLAN_NULL) {
	(*list)[n++] = v;
	v = si->vlan_next[v];
    }

    _shr_sort(*list, *count, sizeof (bcm_vlan_t), _bcm_robo_stg_vid_compar);

    return BCM_E_NONE;
}				
			     
/*
 * Function:
 *	bcm_robo_stg_vlan_list_destroy
 * Purpose:
 *	Destroy a list returned by bcm_stg_vlan_list.
 * Parameters:
 *      unit - RoboSwitch PCI device unit number
 *      list - Pointer to VLAN array to be destroyed.
 *      count - Number of entries in array.
 * Returns:
 *      BCM_E_XXX
 */
int 
bcm_robo_stg_vlan_list_destroy(int unit, bcm_vlan_t *list, int count)
{
    COMPILER_REFERENCE(unit);
    COMPILER_REFERENCE(count);

    LOG_INFO(BSL_LS_BCM_STP,
             (BSL_META_U(unit,
                         "BCM API : bcm_robo_stg_vlan_list_destroy()..\n")));
    if (list != NULL) {
	sal_free(list);
    }

    return BCM_E_NONE;
}				


/*
 * Function:
 *	bcm_robo_stg_create
 * Description:
 *      Create a STG, picking an unused ID and returning it.
 * Parameters:
 *      unit - RoboSwitch PCI device unit number
 *      stg_ptr - (OUT) the STG ID.
 * Returns:
 *      BCM_E_XXX
 */
int 
bcm_robo_stg_create(int unit, bcm_stg_t *stg_ptr) /* Generate new ID */
{
   bcm_stg_info_t  *si = &robo_stg_info[unit];
    bcm_stg_t  stg;
    int  rv;

    LOG_INFO(BSL_LS_BCM_STP,
             (BSL_META_U(unit,
                         "BCM API : bcm_robo_stg_create()..\n")));
    CHECK_INIT(unit, si);

    for (stg = si->stg_min; stg <= si->stg_max; stg++) {
	if (!STG_BITMAP_TST(si, stg)) {
	    break;
	}
    }

    if (stg > si->stg_max) {
	return BCM_E_FULL;
    }

    rv = bcm_stg_create_id(unit, stg);

    *stg_ptr = stg;

    return rv;
}				


/*
 * Function:
 *	bcm_robo_stg_create_id
 * Description:
 *      Create a STG, using a specified ID.
 * Parameters:
 *      unit - RoboSwitch PCI device unit number
 *      stg - STG to create
 * Returns:
 *      BCM_E_XXX
 * Notes:
 *      In the new STG, all ports are in the DISABLED state.
 */
int 
bcm_robo_stg_create_id(int unit, bcm_stg_t stg)	/* Use specific ID */
{
   bcm_stg_info_t  *si = &robo_stg_info[unit];
    int  rv = BCM_E_NONE;

    LOG_INFO(BSL_LS_BCM_STP,
             (BSL_META_U(unit,
                         "BCM API : bcm_robo_stg_create_id()..\n")));
    CHECK_INIT(unit, si);
    CHECK_STG(si, stg);

    if (STG_BITMAP_TST(si, stg)) {
        return BCM_E_EXISTS;
    }

    /* Write an entry with all ports DISABLED */

    if ((rv = _bcm_robo_stg_stp_init(unit, stg)) < 0) {
        return rv;
    }

    STG_BITMAP_SET(si, stg);
    si->stg_count++;

    return rv;
}				

/*
 * Function:
 *	bcm_robo_stg_destroy
 * Description:
 *      Destroy an STG.
 * Parameters:
 *      unit - RoboSwitch PCI device unit number (driver internal).
 *      stg - The STG ID to be destroyed.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 *      The default STG may not be destroyed.
 */
int 
bcm_robo_stg_destroy(int unit, bcm_stg_t stg)
{
    bcm_stg_info_t  *si = &robo_stg_info[unit];
    int  rv;

    LOG_INFO(BSL_LS_BCM_STP,
             (BSL_META_U(unit,
                         "BCM API : bcm_robo_stg_destroy()..\n")));
    CHECK_INIT(unit, si);
    CHECK_STG(si, stg);

    if (stg == si->stg_defl) {
        return BCM_E_NOT_FOUND;
    }

    /* The next call already checks if STG exists */

    rv = bcm_stg_vlan_remove_all(unit, stg);

    if (rv < 0) {
        return rv;
    }

    STG_BITMAP_CLR(si, stg);
    si->stg_count--;

    return BCM_E_NONE;
}				


/*
 * Function:
 *	bcm_robo_stg_list
 * Purpose:
 *	Return a list of defined STGs
 * Parameters:
 *      unit - RoboSwitch PCI device unit number
 *      list - Place where pointer to return array will be stored,
 *              which will be NULL if there are zero STGs returned.
 *      count - Place where number of entries in array will be stored,
 *              which will be 0 if there are zero STGs returned.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 *	The caller is responsible for freeing the memory that is
 *	returned, using bcm_stg_list_destroy().
 */ 
int 
bcm_robo_stg_list(int unit, bcm_stg_t **list, int *count)
{
    bcm_stg_info_t  *si = &robo_stg_info[unit];
    bcm_stg_t  stg;
    int  n;

    LOG_INFO(BSL_LS_BCM_STP,
             (BSL_META_U(unit,
                         "BCM API : bcm_robo_stg_list()..\n")));
    CHECK_INIT(unit, si);

    if (si->stg_count == 0) {
        *count = 0;
        *list = NULL;
        return BCM_E_NONE;		/* Empty list */
    }

    *count = si->stg_count;
    *list = sal_alloc(si->stg_count * sizeof (bcm_stg_t), "bcm_stg_list");

    if (*list == NULL) {
        return BCM_E_MEMORY;
    }

    n = 0;

    for (stg = si->stg_min; stg <= si->stg_max; stg++) {
        if (STG_BITMAP_TST(si, stg)) {
            assert(n < *count);
            (*list)[n++] = stg;
	}
    }

    return BCM_E_NONE;
}
				

/*
 * Function:
 *	bcm_robo_stg_list_destroy
 * Purpose:
 *	Destroy a list returned by bcm_stg_list.
 * Parameters:
 *      unit - RoboSwitch PCI device unit number
 *      list - Place where pointer to return array will be stored,
 *              which will be NULL if there are zero STGs returned.
 *      count - Place where number of entries in array will be stored,
 *              which will be 0 if there are zero STGs returned.
 * Returns:
 *      BCM_E_NONE
 */
int 
bcm_robo_stg_list_destroy(int unit, bcm_stg_t *list, int count)
{
    COMPILER_REFERENCE(unit);
    COMPILER_REFERENCE(count);

    LOG_INFO(BSL_LS_BCM_STP,
             (BSL_META_U(unit,
                         "BCM API : bcm_robo_stg_list_destroy()..\n")));
    if (list != NULL) {
        sal_free(list);
    }

    return BCM_E_NONE;
}				

/*
 * Function:
 *      bcm_robo_stg_stp_set
 * Purpose:
 *      Get the Spanning tree state for a port in specified STG.
 * Parameters:
 *      unit - RoboSwitch unit number.
 *      stg - STG ID.
 *      port - RoboSwitch port number.
 *      stp_state - (Out) Pointer to where Spanning Tree State is stored.
 * Returns:
 *      BCM_E_NONE
 *      BCM_E_INTERNAL
 */
int 
bcm_robo_stg_stp_set(int unit, bcm_stg_t stg,
			   bcm_port_t port, int stp_state)
{
    bcm_stg_info_t  *si = &robo_stg_info[unit];
    int  rv;

    LOG_INFO(BSL_LS_BCM_STP,
             (BSL_META_U(unit,
                         "BCM API : bcm_robo_stg_stp_set()..port = %d, state = %d\n"),
              port, stp_state));

    CHECK_INIT(unit, si);
    CHECK_STG(si, stg);

    if (BCM_GPORT_IS_SET(port)) {
        BCM_IF_ERROR_RETURN(bcm_port_local_get(unit, port, &port));
    }

    /* 
     * CPU should be always been Forward status, and not suitable 
     *   for setting stp status 
     */
    if (!SOC_PORT_VALID(unit, port) || !IS_PORT(unit, port)) {
        return BCM_E_PORT;
    }

    if (!STG_BITMAP_TST(si, stg)) {
        return BCM_E_NOT_FOUND;
    }

    rv = _bcm_robo_stg_stp_set(unit, stg, port, stp_state);

    return rv;

}				

/*
 * Function:
 *      bcm_robo_stg_stp_get
 * Purpose:
 *      Get the Spanning tree state for a port in specified STG.
 * Parameters:
 *      unit - RoboSwitch unit number.
 *      stg - STG ID.
 *      port - RoboSwitch port number.
 *      stp_state - (Out) Pointer to where Spanning Tree State is stored.
 * Returns:
 *      BCM_E_NONE
 *      BCM_E_INTERNAL
 */
int 
bcm_robo_stg_stp_get(int unit, bcm_stg_t stg,
			   bcm_port_t port, int *stp_state)
{
    bcm_stg_info_t  *si = &robo_stg_info[unit];
    int  rv;

    CHECK_INIT(unit, si);
    CHECK_STG(si, stg);

    if (BCM_GPORT_IS_SET(port)) {
        BCM_IF_ERROR_RETURN(bcm_port_local_get(unit, port, &port));
    }

    /* 
     * CPU should be always been Forward status, and not suitable 
     *   for getting stp status 
     */
    if (!SOC_PORT_VALID(unit, port) || !IS_PORT(unit, port)) {
        return BCM_E_PORT;
    }

    if (!STG_BITMAP_TST(si, stg)) {
        return BCM_E_NOT_FOUND;
    }

    rv = _bcm_robo_stg_stp_get(unit, stg, port, stp_state);

    LOG_INFO(BSL_LS_BCM_STP,
             (BSL_META_U(unit,
                         "BCM API : bcm_robo_stg_stp_get()..port = %d, state = %d\n"),
              port, *stp_state));

    return rv;
}				
			   
/*
 * Function:
 *     bcm_robo_stg_count_get
 * Purpose:
 *     Get the maximum number of STG groups the chip supports
 * Parameters:
 *     unit - RoboSwitch unit number.
 *     max_stg - max number of STG groups supported by this chip
 * Returns:
 *     BCM_E_xxx
 */
int 
bcm_robo_stg_count_get(int unit, int *max_stg)
{
    bcm_stg_info_t  *si = &robo_stg_info[unit];

    LOG_INFO(BSL_LS_BCM_STP,
             (BSL_META_U(unit,
                         "BCM API : bcm_robo_stg_count_get()..\n")));
    CHECK_INIT(unit, si);

    *max_stg = si->stg_max;

    return BCM_E_NONE;
}				

#ifdef STP_BY_EAP_BLK
void
bcm_robo_stp_8021x_lock(int unit)
{
    sal_mutex_take(stp_8021x_state_Mutex[unit], sal_mutex_FOREVER);
}

void
bcm_robo_stp_8021x_unlock(int unit)
{
    sal_mutex_give(stp_8021x_state_Mutex[unit]);
}

void
bcm_robo_port_vlan_remove(int unit, bcm_port_t rem_port)
{
    bcm_pbmp_t pbmp;
    int port;
    
    BCM_PBMP_PORT_ADD(stp_non_forward_pbmp[unit], rem_port);

    BCM_PBMP_ITER(PBMP_ALL(unit), port){
        BCM_PBMP_CLEAR(pbmp);
        DRV_PORT_VLAN_GET(unit, port, &pbmp);
        BCM_PBMP_PORT_REMOVE(pbmp, rem_port);
        DRV_PORT_VLAN_SET(unit, port, pbmp);
    }
        
}

void
bcm_robo_port_vlan_add(int unit, bcm_port_t add_port)
{
    bcm_pbmp_t pbmp;
    int port;

    BCM_PBMP_PORT_REMOVE(stp_non_forward_pbmp[unit], add_port);
    
    BCM_PBMP_ITER(PBMP_ALL(unit), port){
        BCM_PBMP_CLEAR(pbmp);
        DRV_PORT_VLAN_GET(unit, port, &pbmp);
        BCM_PBMP_PORT_ADD(pbmp, add_port);
        DRV_PORT_VLAN_SET(unit, port, pbmp);
    }
}

#endif /* STP_BY_EAP_BLK */

