/*
 * $Id: trunk.c,v 1.21 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:    trunk.c
 * Purpose: BCM level APIs for Link Aggregation (a.k.a Trunking)
 */

#define SBX_HASH_DEFINED 1
#include <shared/bsl.h>

#include <soc/drv.h>
#include <soc/sbx/sbx_drv.h>
#include <soc/sbx/fe2k_common/sbFeISupport.h>

#ifdef BCM_CALADAN3_G3P1_SUPPORT
#include <soc/sbx/caladan3/ocm.h>
#include <soc/sbx/g3p1/g3p1_defs.h>
#include <soc/sbx/g3p1/g3p1_int.h>
#include <soc/sbx/g3p1/g3p1_tmu.h>
#include <soc/sbx/g3p1/g3p1_ppe_tables.h>
#include <soc/sbx/g3p1/g3p1.h>
#include <soc/sbx/caladan3.h>
#endif

#include <bcm/types.h>
#include <bcm/error.h>
#include <bcm/trunk.h>
#include <bcm_int/sbx/caladan3/trunk.h>
#include <bcm/vlan.h>
#include <bcm/stack.h>
#include <bcm/pkt.h>

#include <bcm_int/control.h>
#include <bcm_int/common/lock.h>
#include <bcm_int/sbx/error.h>
#include <bcm_int/sbx/trunk.h>
#include <bcm_int/sbx/stack.h>
#include <bcm_int/sbx/caladan3/vlan.h>
#include <bcm_int/sbx/caladan3/wb_db_trunk.h>


/*
 * One trunk control entry for each SOC device containing trunk book keeping
 * info for that device.
 */
static trunk_cntl_t bcm_trunk_control[BCM_MAX_NUM_UNITS];

#define TRUNK_MIN_MEMBERS   0   /* Minimum number of ports in a trunk */
#define TRUNK_NUM_VIDS      4095

#define TRUNK_CNTL(unit)        bcm_trunk_control[unit]
#define TRUNK_INFO(unit, tid)   bcm_trunk_control[unit].t_info[tid]

#if 0 /* Super mutex  */
extern void _vlan_lock_take();
extern void _vlan_lock_release();
#define TRUNK_DB_LOCK(unit) _vlan_lock_take(unit)
#define TRUNK_DB_UNLOCK(unit) _vlan_lock_release(unit);
#else
#define TRUNK_DB_LOCK(unit)                                                 \
        do {                                                                \
            if (NULL != TRUNK_CNTL(unit).lock)                              \
                sal_mutex_take(TRUNK_CNTL(unit).lock, sal_mutex_FOREVER);   \
        } while (0);

#define TRUNK_DB_UNLOCK(unit)                           \
        do {                                            \
            if (NULL != TRUNK_CNTL(unit).lock)          \
                sal_mutex_give(TRUNK_CNTL(unit).lock);  \
        } while (0);
#endif

/*
 * Cause a routine to return BCM_E_INIT if trunking subsystem is not
 * initialized to an acceptable initialization level (il_).
 */
#define TRUNK_CHECK_INIT(u_, il_)                                 \
    do {                                                          \
        if (!BCM_UNIT_VALID(u_)) return BCM_E_UNIT;               \
        if (u_ >= BCM_MAX_NUM_UNITS) return BCM_E_UNIT;           \
        if (TRUNK_CNTL(u_).init_state < (il_)) return BCM_E_INIT; \
    } while (0);

/*
 * Make sure TID is within valid range.
 */
#define TRUNK_CHECK_TID(unit, tid) \
    if (((tid) < 0) || ((tid) >= TRUNK_CNTL(unit).ngroups)) \
        return BCM_E_BADID;
/*
 * TID is in range, check to make sure it is actually in use.
 */
#define TRUNK_TID_VALID(unit, tid)                          \
    (TRUNK_INFO((unit), (tid)).trunk_id != BCM_TRUNK_INVALID)

#define TRUNK_PORTCNT_VALID(unit, port_cnt)                  \
    ((port_cnt >= TRUNK_MIN_MEMBERS) && (port_cnt <= TRUNK_CNTL(unit).nports))

#define G2_FE_HANDLER_GET(unit, fe)  \
    if ((fe = (sbG2Fe_t *)SOC_SBX_CONTROL(unit)->drv) == NULL) {  \
        return BCM_E_INIT;  \
    }
#define G3P1_FE_HANDLER_GET(unit, fe)  \
    if ((fe = (soc_sbx_g3p1_state_t*)SOC_SBX_CONTROL(unit)->drv) == NULL) {  \
        return BCM_E_INIT;  \
    }



static int        _ngroups          = SBX_MAX_TRUNKS;
static int        _nports           = BCM_TRUNK_MAX_PORTCNT;

#define QE_SPI_SUBPORT_GET(funit, fport) ((fport) + SOC_PORT_MIN((funit), spi_subport))

/*
 * Fixed offsets
 */
/* total system ports including CPU */
#define TRUNK_SBX_HASH_SIZE             3
#define TRUNK_SBX_FIXED_PORTCNT         (1<<TRUNK_SBX_HASH_SIZE)

#define TRUNK_INDEX_SET(tid, offset)                \
    (TRUNK_SBX_FIXED_PORTCNT > (offset)) ?          \
     ((((tid))<<(TRUNK_SBX_HASH_SIZE)) | (offset)) :  \
     -1

/* forward declaration */
int bcm_caladan3_trunk_psc_set(int unit, bcm_trunk_t tid, int psc);



typedef struct recursive_mutex_s {
    sal_sem_t       sem;
    sal_thread_t    owner;
    int             recurse_count;
    char           *desc;
} recursive_mutex_t;


#ifdef BCM_WARM_BOOT_SUPPORT
trunk_cntl_t *bcm_sbx_caladan3_trunk_cntl_ptr_get(int unit)
{
    return &bcm_trunk_control[unit];
}

#endif /* BCM_WARM_BOOT_SUPPORT */
/*
 * Function:
 *    _bcm_caladan3_trunk_debug
 * Purpose:
 *      Displays trunk information maintained by software.
 * Parameters:
 *      unit - Device unit number
 * Returns:
 *      None
 */

void
_bcm_caladan3_trunk_debug(int unit)
{
#ifdef BROADCOM_DEBUG
    trunk_cntl_t       *tc;
    trunk_private_t    *ti;
    recursive_mutex_t  *lock;
    int                 index, jindex;
    int                 invIdx = 0;
    bcm_trunk_t         invalidList[SBX_MAX_TRUNKS];
    int                 funit, fnode, fport;
    int                 mymod, fmod;

    if (!BCM_UNIT_VALID(unit)) {
        return;
    } else if (unit >= BCM_MAX_NUM_UNITS) {
        /* trunk_control is sized by BCM_MAX_NUM_UNITS, not BCM_CONTROL_MAX,
         * or BCM_UNITS_MAX 
         */
        return;
    }

    tc = &TRUNK_CNTL(unit);
    LOG_CLI(("--- Debug ---\n"));
    LOG_CLI(("\nSW Information TRUNK - Unit %d\n", unit));
    LOG_CLI(("  Initialized         : %s\n", 
             tc->init_state ? ((tc->init_state == ts_recovering) ?
                               "Recovering" : "True")
             : "False"));
    LOG_CLI(("  Lock                : 0x%08X\n", (uint32)tc->lock));
    if (tc->init_state != ts_none) {
        lock = (recursive_mutex_t *)((int)tc->lock + 12);
        LOG_CLI(("    Desc              : %s\n", lock->desc));
        LOG_CLI(("    Owner             : 0x%08X\n", (uint32)lock->owner));
        LOG_CLI(("    Count             : %d\n", lock->recurse_count));
    }
    LOG_CLI(("  Trunk groups        : %d\n", tc->ngroups));
    LOG_CLI(("  Trunk max ports     : %d\n", tc->nports));
    LOG_CLI(("  Port Select Criteria: 0x%x\n", tc->psc));

    for (index = 0; index < tc->ngroups; index++) {
        ti = &TRUNK_INFO(unit, index);
        if (ti->trunk_id == BCM_TRUNK_INVALID) {
            invalidList[invIdx++] = index;
        } else {
            LOG_CLI(("\n  Trunk %d\n", index));
            LOG_CLI(("      ID              : %d\n", ti->trunk_id));
            LOG_CLI(("      in use          : %d\n", ti->in_use));
            LOG_CLI(("      number of ports : %d\n", ti->num_ports));
            if (0 < ti->num_ports) {
                LOG_CLI(("         ports        : %d:%d  flags: %x", ti->tm[0], ti->tp[0], ti->member_flags[0]));
                for (jindex = 1; jindex < ti->num_ports; jindex++)
                    LOG_CLI((",  %d:%d  flags: %x", ti->tm[jindex], ti->tp[jindex], ti->member_flags[jindex]));
                LOG_CLI(("\n"));
            }
        }
    }
    LOG_CLI(("\nUnused Trunks: "));
    for (index = 0; index < invIdx; index++) {
        LOG_CLI(("%d ", invalidList[index]));
    }
    
    if (SOC_IS_SBX_CALADAN3(unit)) {
        
        return;
    }
    
    if (SOC_IS_SBX_FE(unit)) {
        (void)bcm_stk_my_modid_get(unit, &invIdx);
        (void)soc_sbx_node_port_get(unit, invIdx, 0, &funit, &fnode, &fport);
        (void)bcm_stk_modid_get(unit,  &mymod);
        (void)bcm_stk_modid_get(funit, &fmod);
        LOG_CLI(("\nModule Mapping, FE(unit=%d) => Module %d\n", unit, mymod));
        LOG_CLI(("                QE(unit=%d) => Module %d\n", funit, fmod));
        LOG_CLI(("\nPort Mapping, FE(unit:port) <=> QE(unit:port)  SPI-subport\n"));
        for (index = 0; index < 23; index++) {
            (void)soc_sbx_node_port_get(unit, invIdx, index, &funit, &fnode, &fport);
            if (SOC_IS_SBX_QE(funit)) {
                LOG_CLI(("\t%2d:%2d <=> %2d:%2d   %d\n", unit, index,
                         funit, fport, QE_SPI_SUBPORT_GET(funit, fport)));
            }
        }
    }
    LOG_CLI(("\n\n"));
#endif /* BROADCOM_DEBUG */

    return;
}


#ifdef BCM_CALADAN3_G3P1_SUPPORT
static
int
_bcm_caladan3_g3p1_trunk_outHdr_hw_set(int unit, uint32 eIndex,
                                       uint32 eteAddr)
{
    int                    rv = BCM_E_NONE;
    soc_sbx_g3p1_oi2e_t    sbxOutHdrIdx;

    soc_sbx_g3p1_oi2e_t_init(&sbxOutHdrIdx);
    sbxOutHdrIdx.eteptr = eteAddr;
    eIndex -= SBX_RAW_OHI_BASE;
    rv = soc_sbx_g3p1_oi2e_set(unit, eIndex, &sbxOutHdrIdx);

    return rv;
}
#endif /* def BCM_CALADAN3_G3P1_SUPPORT */

/*
 * Function:
 *    _bcm_caladan3_trunk_outHdr_set
 * Purpose:
 *      Set an Out Header as specified.
 * Parameters:
 *      unit        - Device unit number.
 *      eIndex      - entry to modify
 *      eteAddr     - L2 ETE Entry address (index)
 * Returns:
 *      BCM_E_NONE      - Success.
 *      BCM_E_XXXXX     - As set by lower layers of software
 */
int
_bcm_caladan3_trunk_outHdr_set(int unit, uint32 eIndex, uint32 eteAddr)
{
    int rv = BCM_E_UNAVAIL;

    LOG_DEBUG(BSL_LS_BCM_TRUNK,
              (BSL_META_U(unit,
                          "%s(%d, %d, %d) - Enter\n"),
               FUNCTION_NAME(), unit, eIndex, eteAddr));
    switch (SOC_SBX_CONTROL(unit)->ucodetype)
    {
#ifdef BCM_CALADAN3_G3P1_SUPPORT
    case SOC_SBX_UCODE_TYPE_G3P1:
        rv = _bcm_caladan3_g3p1_trunk_outHdr_hw_set(unit, eIndex, eteAddr);
        break;
#endif /* def BCM_CALADAN3_G3P1_SUPPORT */
    default:
        rv = BCM_E_UNAVAIL;
    }

    LOG_DEBUG(BSL_LS_BCM_TRUNK,
              (BSL_META_U(unit,
                          "\tProgrammed OutHdrIdx %d - ETE:%d (%s)\n"),
               eIndex, eteAddr, bcm_errmsg(rv)));
    if (BCM_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_BCM_TRUNK,
                  (BSL_META_U(unit,
                              "Cannot set outHdrIdx2Etc[%d]: %s (%d)\n"),
                   eIndex, bcm_errmsg(rv), rv));
    }

    LOG_DEBUG(BSL_LS_BCM_TRUNK,
              (BSL_META_U(unit,
                          "%s(%d, %d, %d) - Exit(%s)\n"),
               FUNCTION_NAME(), unit,
               eIndex, eteAddr, bcm_errmsg(rv)));

    return rv;
}


#ifdef BCM_CALADAN3_G3P1_SUPPORT
static
int
_bcm_caladan3_g3p1_trunk_fte_hw_set(int unit, uint32 eIndex, bcm_trunk_t tid,
                                    uint32 outHdrIdx)
{
    soc_sbx_g3p1_ft_t    sbxFte;
    int                  rv;

    soc_sbx_g3p1_ft_t_init(&sbxFte);

    if (tid == BCM_TRUNK_INVALID) {
        rv = soc_sbx_g3p1_ft_set(unit, eIndex, &sbxFte);

    } else {
        rv = soc_sbx_g3p1_ft_get(unit, eIndex, &sbxFte);
        if (BCM_SUCCESS(rv)) {

            sbxFte.qid     = 0;
            sbxFte.lag     = 1;
            sbxFte.lagsize = 3;  /* 2^3  */
            sbxFte.lagbase = TRUNK_INDEX_SET(tid, 0);
            sbxFte.oi      = outHdrIdx;

            rv = soc_sbx_g3p1_ft_set(unit, eIndex, &sbxFte);
        }
    }

    return rv;
}
#endif /* def BCM_CALADAN3_G3P1_SUPPORT */

/*
 * Function:
 *    _bcm_caladan3_trunk_fte_set
 * Purpose:
 *      Set an L2 FTE entry for Trunking.
 * Parameters:
 *      unit      - Device unit number.
 *      eIndex    - entry to modify
 *      tid       - trunk id
 *      outHdrIdx - OutHeader to program
 *
 * Notes:
 *   If tid == BCM_TRUNK_INVALID, the FT entry will be invalidated.
 * Returns:
 *      BCM_E_NONE      - Success.
 *      BCM_E_XXXXX     - As set by lower layers of software
 */
static
int
_bcm_caladan3_trunk_fte_set(int unit, uint32 eIndex, bcm_trunk_t tid,
                          uint32 outHdrIdx)
{
    int rv = BCM_E_UNAVAIL;

    LOG_DEBUG(BSL_LS_BCM_TRUNK,
              (BSL_META_U(unit,
                          "%s(%d, %d, %d, %d) - Enter\n"),
               FUNCTION_NAME(), unit,
               eIndex, tid, outHdrIdx));

    switch (SOC_SBX_CONTROL(unit)->ucodetype)
    {
#ifdef BCM_CALADAN3_G3P1_SUPPORT
    case SOC_SBX_UCODE_TYPE_G3P1:
        rv = _bcm_caladan3_g3p1_trunk_fte_hw_set(unit, eIndex, tid, outHdrIdx);
        break;
#endif /* def BCM_CALADAN3_G3P1_SUPPORT */
    default:
        rv = BCM_E_UNAVAIL;
    }

    if (BCM_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_BCM_TRUNK,
                  (BSL_META_U(unit,
                              "Cannot set fte[%d]: %s (%d)\n"),
                   eIndex,
                   bcm_errmsg(rv), rv));
    }

    LOG_DEBUG(BSL_LS_BCM_TRUNK,
              (BSL_META_U(unit,
                          "%s(%d, %d, %d, %d) - Exit(%s)\n"),
               FUNCTION_NAME(), unit,
               eIndex, tid, outHdrIdx, bcm_errmsg(rv)));
    return rv;
}




/*
 * Function:
 *      _bcm_caladan3_trunk_fte_inValidate
 * Purpose:
 *      Invalidate and clear an L2 FTE entry.
 * Parameters:
 *      unit    - Device unit number.
 *      eIndex  - entry to modify
 * Returns:
 *      BCM_E_NONE      - Success.
 *      BCM_E_XXXXX     - As set by lower layers of software
 */
static
int
_bcm_caladan3_trunk_fte_inValidate(int unit, uint32 eIndex)
{
    int rv;

    LOG_DEBUG(BSL_LS_BCM_TRUNK,
              (BSL_META_U(unit,
                          "%s(%d, %d) - Enter\n"),
               FUNCTION_NAME(), unit, eIndex));

    rv = _bcm_caladan3_trunk_fte_set(unit, eIndex, BCM_TRUNK_INVALID, 0);

    LOG_DEBUG(BSL_LS_BCM_TRUNK,
              (BSL_META_U(unit,
                          "%s(%d, %d) - Exit(%s)\n"),
               FUNCTION_NAME(), unit, eIndex,
               bcm_errmsg(rv)));
    return rv;
}


#ifdef BCM_CALADAN3_G3P1_SUPPORT
static
int
_bcm_caladan3_g3p1_trunk_ingLag_hw_set(int unit, uint32 eIndex, uint32 qid,
                                     uint32 outHdrIdx)
{
    int                  rv;
    soc_sbx_g3p1_lag_t   sbxLag;

    soc_sbx_g3p1_lag_t_init(&sbxLag);
    sbxLag.qid = qid;
    sbxLag.oi  = outHdrIdx;
    rv = soc_sbx_g3p1_lag_set(unit, eIndex, &sbxLag);

    return rv;;
}
#endif /* def BCM_CALADAN3_G3P1_SUPPORT */

/*
 * Function:
 *    _bcm_caladan3_trunk_ingLag_set
 * Purpose:
 *      Set an ingrLag entry for Trunking.
 * Parameters:
 *      unit      - Device unit number.
 *      eIndex    - entry to modify
 *      qid       - Queue for this entry to point to
 *      outHdrIdx - OutHeader to program
 * Returns:
 *      BCM_E_NONE      - Success.
 *      BCM_E_XXXXX     - As set by lower layers of software
 */
static
int
_bcm_caladan3_trunk_ingLag_set(int unit, uint32 eIndex, uint32 qid,
                             uint32 outHdrIdx)
{
    int rv = BCM_E_UNAVAIL;

    LOG_DEBUG(BSL_LS_BCM_TRUNK,
              (BSL_META_U(unit,
                          "%s(%d, %d, %d, %d) - Enter\n"),
               FUNCTION_NAME(), unit,
               eIndex, qid, outHdrIdx));

    switch (SOC_SBX_CONTROL(unit)->ucodetype)
    {
#ifdef BCM_CALADAN3_G3P1_SUPPORT
    case SOC_SBX_UCODE_TYPE_G3P1:
        rv = _bcm_caladan3_g3p1_trunk_ingLag_hw_set(unit, eIndex,
                                                    qid, outHdrIdx);
        break;
#endif
    default:
        rv = BCM_E_UNAVAIL;
    }

    LOG_DEBUG(BSL_LS_BCM_TRUNK,
              (BSL_META_U(unit,
                          "\tProgrammed IngressLag %d - QID:%d  OHI:%d (%s)\n"),
               eIndex, qid, outHdrIdx, bcm_errmsg(rv)));

    LOG_DEBUG(BSL_LS_BCM_TRUNK,
              (BSL_META_U(unit,
                          "%s(%d, %d, %d, %d) - Exit(%s)\n"),
               FUNCTION_NAME(), unit,
               eIndex, qid, outHdrIdx, bcm_errmsg(rv)));
    return rv;
}


#ifdef BCM_CALADAN3_G3P1_SUPPORT
static
int
_bcm_caladan3_g3p1_trunk_port2Etc_hw_set(int unit, uint32 port, uint32 sid)
{
    int                 rv;
    soc_sbx_g3p1_lp_t   sbxLogicalPort;

    rv = soc_sbx_g3p1_lp_get(unit, port, &sbxLogicalPort);

    if (BCM_SUCCESS(rv)) {
        sbxLogicalPort.pid = sid;

        rv = soc_sbx_g3p1_lp_set(unit, port, &sbxLogicalPort);
    }

    return rv;
}
#endif /* def BCM_CALADAN3_G3P1_SUPPORT */

/*
 * Function:
 *    _bcm_caladan3_trunk_port2Etc_set
 * Purpose:
 *      Set an port2Etc entry for Trunking.
 * Parameters:
 *      unit      - Device unit number.
 *      eIndex    - entry to modify
 *      sid       - Source ID value of port2Etc value to program.
 * Returns:
 *      BCM_E_NONE      - Success.
 *      BCM_E_XXXXX     - As set by lower layers of software
 */
static
int
_bcm_caladan3_trunk_port2Etc_set(int unit, uint32 port, uint32 sid)
{
    int rv = BCM_E_UNAVAIL;

    LOG_DEBUG(BSL_LS_BCM_TRUNK,
              (BSL_META_U(unit,
                          "%s(%d, %d, %d) - Enter\n"),
               FUNCTION_NAME(), unit,
               port, sid));

    switch (SOC_SBX_CONTROL(unit)->ucodetype)
    {
#ifdef BCM_CALADAN3_G3P1_SUPPORT
    case SOC_SBX_UCODE_TYPE_G3P1:
        rv = _bcm_caladan3_g3p1_trunk_port2Etc_hw_set(unit, port, sid);
        break;
#endif
    default:
        rv = BCM_E_UNAVAIL;
    }

    LOG_DEBUG(BSL_LS_BCM_TRUNK,
              (BSL_META_U(unit,
                          "\tProgrammed port2Etc %d - SID:%d (%s)\n"),
               port, sid, bcm_errmsg(rv)));

    if (BCM_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_BCM_TRUNK,
                  (BSL_META_U(unit,
                              "Cannot set port2Etc[%d]: %s (%d)\n"),
                   port,
                   bcm_errmsg(rv), rv));
    }

    LOG_DEBUG(BSL_LS_BCM_TRUNK,
              (BSL_META_U(unit,
                          "%s(%d, %d, %d) - Exit(%s)\n"),
               FUNCTION_NAME(), unit,
               port, sid, bcm_errmsg(rv)));
    return rv;
}


#ifdef BCM_CALADAN3_G3P1_SUPPORT
static
int
_bcm_caladan3_g3p1_trunk_egrPort2Etc_hw_set(int unit, uint32 eIndex,
                                          uint32 sid)
{
    int                 rv;
    soc_sbx_g3p1_ep2e_t sbxEgrPort;

    rv = soc_sbx_g3p1_ep2e_get(unit, eIndex, &sbxEgrPort);

    if (BCM_SUCCESS(rv)) {
        sbxEgrPort.pid = sid;
        rv = soc_sbx_g3p1_ep2e_set(unit, eIndex, &sbxEgrPort);
    }

    return rv;
}
#endif /* def BCM_CALADAN3_G3P1_SUPPORT */


/*
 * Function:
 *    _bcm_caladan3_trunk_egrPort2Etc_set
 * Purpose:
 *      Set an egrPort2Etc entry for Trunking.
 * Parameters:
 *      unit      - Device unit number.
 *      eIndex    - entry to modify
 *      sid       - Source ID value of egrPort2Etc value to program.
 * Returns:
 *      BCM_E_NONE      - Success.
 *      BCM_E_XXXXX     - As set by lower layers of software
 */
static
int
_bcm_caladan3_trunk_egrPort2Etc_set(int unit, uint32 eIndex, uint32 sid)
{
    int rv = BCM_E_UNAVAIL;

    LOG_DEBUG(BSL_LS_BCM_TRUNK,
              (BSL_META_U(unit,
                          "%s(%d, %d, %d) - Enter\n"),
               FUNCTION_NAME(), unit,
               eIndex, sid));


    switch (SOC_SBX_CONTROL(unit)->ucodetype)
    {
#ifdef BCM_CALADAN3_G3P1_SUPPORT
    case SOC_SBX_UCODE_TYPE_G3P1:
        rv = _bcm_caladan3_g3p1_trunk_egrPort2Etc_hw_set(unit, eIndex, sid);
        break;
#endif /* def BCM_CALADAN3_G3P1_SUPPORT */
    default:
        rv = BCM_E_UNAVAIL;
    }

    if (BCM_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_BCM_TRUNK,
                  (BSL_META_U(unit,
                              "Cannot set egrPort2Etc[%d]: %s (%d)\n"),
                   eIndex,
                   bcm_errmsg(rv), rv));
    }

    LOG_DEBUG(BSL_LS_BCM_TRUNK,
              (BSL_META_U(unit,
                          "%s(%d, %d, %d) - Exit(%s)\n"),
               FUNCTION_NAME(), unit,
               eIndex, sid, bcm_errmsg(rv)));
    return rv;
}

/*
 * Function:
 *    _bcm_caladan3_trunk_set
 * Purpose:
 *      Add ports to a trunk group.
 * Parameters:
 *      unit       - Device unit number.
 *      tid        - The trunk ID to be affected.
 *      t_add_info - Information on the trunk group.
 * Returns:
 *      BCM_E_NONE      - Success.
 *      BCM_E_PARAM     - Invalid ports specified.
 *      BCM_E_XXXXX     - As set by lower layers of software
 * Notes:
 *      the following fields of the bcm_trunk_add_info_t structure are ignored
 *      on SBX:
 *          flags
 *          dlf_index
 *          mc_index
 *          ipmc_index
 */

int
_bcm_caladan3_trunk_set(int unit, bcm_trunk_t tid, bcm_trunk_add_info_t *add_info, int *port_change)
{
    trunk_cntl_t       *tc;
    trunk_private_t    *ti;
    bcm_port_t          port;
    bcm_module_t        mymod, module;
    bcm_trunk_t         test_tid;
    trunk_private_t     *removed = NULL, *added = NULL;
    int                 index, jindex, tableIndex, num_active_ports = 0;
    int                 psc;
    uint32            qid;
    uint32            sid=0;
    uint32            eteAddr=0;
    uint32            outHdrIndex;
    uint32            member_flags;
    int                 result = BCM_E_NONE;
    int                 fab_unit, fab_node, fab_port;

    LOG_DEBUG(BSL_LS_BCM_TRUNK,
              (BSL_META_U(unit,
                          "%s(%d, %d, [n=%d - "),
               FUNCTION_NAME(), unit, tid, add_info->num_ports));
    for (index = 0; index < add_info->num_ports; index++) {
        LOG_DEBUG(BSL_LS_BCM_TRUNK,
                  (BSL_META_U(unit,
                              " %d:%d flags:%x"),
                   add_info->tm[index], add_info->tp[index], add_info->member_flags[index]));
    }
    LOG_DEBUG(BSL_LS_BCM_TRUNK,
              (BSL_META_U(unit,
                          "]) - Enter\n")));

    tc = &TRUNK_CNTL(unit);
    ti = &TRUNK_INFO(unit, tid);

    result = bcm_stk_modid_get(unit, &mymod);
    if (BCM_E_NONE != result) {
        return result;
    }

    added =  sal_alloc(sizeof(trunk_private_t), "Trunk-private added");
    if (added == NULL) {
        LOG_ERROR(BSL_LS_BCM_TRUNK,
                  (BSL_META_U(unit,
                              "Cannot allocate trunk-private add memory\n")));
        return BCM_E_MEMORY;
    }
    removed =  sal_alloc(sizeof(trunk_private_t), "Trunk-private removed");
    if (removed == NULL) {
        LOG_ERROR(BSL_LS_BCM_TRUNK,
                  (BSL_META_U(unit,
                              "Cannot allocate trunk-private removed memory\n")));
        sal_free(added);
        return BCM_E_MEMORY;
    }
    sal_memset(added, 0, sizeof(trunk_private_t));
    sal_memset(removed, 0, sizeof(trunk_private_t));

    /* assume all current port are to be removed */
    removed->num_ports = ti->num_ports;
    sal_memcpy(removed->tm, ti->tm, sizeof(bcm_module_t) * BCM_TRUNK_MAX_PORTCNT);
    sal_memcpy(removed->tp, ti->tp, sizeof(bcm_module_t) * BCM_TRUNK_MAX_PORTCNT);

    /*
     * Make sure the ports/nodes supplied are valid.
     * Make sure the ports don't belong to a different trunk.
     * Populate removed lists.
     */
    for (index = 0; index < add_info->num_ports; index++) {
        module = add_info->tm[index];
        if (!SOC_SBX_MODID_ADDRESSABLE(unit, module)) {
            sal_free(added);
            sal_free(removed);
            return BCM_E_PARAM;
        }

        port = add_info->tp[index];
        if (module == mymod && FALSE == SOC_PORT_VALID(unit, port)) {
            sal_free(added);
            sal_free(removed);
            return BCM_E_PARAM;
        }

        member_flags = add_info->member_flags[index];

        result = bcm_trunk_find(unit, module, port, &test_tid);
        if (BCM_E_NONE == result) {     /* mod:port was found */
            if (tid == test_tid) {      /* and it is already in this trunk */
                /* take this mod:port out of the removal list */
                for (tableIndex=0; tableIndex<removed->num_ports; tableIndex++) {
                    if ((port   == removed->tp[tableIndex]) &&
                        (module == removed->tm[tableIndex])) {
                        removed->num_ports -= 1;
                        removed->tp[tableIndex] = removed->tp[removed->num_ports];
                        removed->tm[tableIndex] = removed->tm[removed->num_ports];
                        break;
                    }
                }
            } else {                    /* what? port is in another trunk. */
                LOG_ERROR(BSL_LS_BCM_TRUNK,
                          (BSL_META_U(unit,
                                      "Port %d:%d exists in trunk %d\n"),
                           module, port,
                           test_tid));
                sal_free(added);
                sal_free(removed);
                return BCM_E_PARAM;
            }
        } else {
            added->member_flags[added->num_ports] = member_flags;
            added->tp[added->num_ports] = port;
            added->tm[added->num_ports] = module;
            added->num_ports++;
        }
    }

    if (added->num_ports == 0 && removed->num_ports == 0) {
        *port_change = 0;
    } else {
        *port_change = 1;
    }

    if (add_info->psc <= 0) {
        if (tc->psc == _BCM_TRUNK_PSC_UNDEFINED) {
            psc = BCM_TRUNK_PSC_MACDA | BCM_TRUNK_PSC_MACSA | \
                  BCM_TRUNK_PSC_L4DS | BCM_TRUNK_PSC_L4SS | \
                  BCM_TRUNK_PSC_IPDA | BCM_TRUNK_PSC_IPSA;
        }else{
            psc = tc->psc;
        }
    } else {
        psc = add_info->psc;
    }

    result = bcm_caladan3_trunk_psc_set(unit, tid, psc);

    /* Remove ports that have been taken out of Trunk */
    if (BCM_E_NONE == result) {
        for (index = 0; index < removed->num_ports ; index++) {
            module = removed->tm[index];
            if (module == mymod) {
                /* only program port2Etc on this module */

                port = removed->tp[index];
                switch(SOC_SBX_CONTROL(unit)->ucodetype) {
                case SOC_SBX_UCODE_TYPE_G3P1:

                    result = soc_sbx_node_port_get(unit, mymod, port,
                                                   &fab_unit,
                                                   &fab_node,
                                                   &fab_port);
                    sid = SOC_SBX_PORT_SID(unit, fab_node, fab_port);
                    break;
                default:
                    LOG_ERROR(BSL_LS_BCM_TRUNK,
                              (BSL_META_U(unit,
                                          "unknown ucode type.\n")));
                    result = BCM_E_CONFIG;
                    break;
                }

                if (BCM_FAILURE(result)) {
                    LOG_ERROR(BSL_LS_BCM_TRUNK,
                              (BSL_META_U(unit,
                                          "Failed to compute sid.\n")));
                    break;
                }

                /* port2Etc.SID */
                result = _bcm_caladan3_trunk_port2Etc_set(unit, port, sid);
                if (BCM_E_NONE != result) break;

                /* egrPort2Etc.SID */
                result = _bcm_caladan3_trunk_egrPort2Etc_set(unit, port, sid);
                if (BCM_E_NONE != result) break;
            }
        }
    }

    /* if there are no members in the lag, no point in setting these tables */
    if (BCM_E_NONE == result) {
        for (jindex = 0; jindex < add_info->num_ports; jindex++) {
            if ((add_info->member_flags[jindex] & BCM_TRUNK_MEMBER_EGRESS_DISABLE) == 0) {
                num_active_ports++;
            }
            LOG_VERBOSE(BSL_LS_BCM_TRUNK,
                        (BSL_META_U(unit,
                                    "Number of active ports %d\n"),
                         num_active_ports));
        }

        if (0 < num_active_ports) {
            index = 0;
            for (jindex = 0; jindex < TRUNK_SBX_FIXED_PORTCNT; jindex++) {
                while (add_info->member_flags[index] 
                             & BCM_TRUNK_MEMBER_EGRESS_DISABLE) {
                    index = (index + 1) % add_info->num_ports;
                }
                port    = add_info->tp[index];
                module  = add_info->tm[index];
                result  = soc_sbx_node_port_get(unit, module, port,
                                                &fab_unit, &fab_node, &fab_port);
                if (BCM_E_NONE != result) break;
                qid  = SOC_SBX_NODE_PORT_TO_QID(unit,fab_node, fab_port, 
                                                NUM_COS(unit));

                /* Member Flag: egress_drop */
                if (add_info->member_flags[index] & BCM_TRUNK_MEMBER_EGRESS_DROP) {
                   qid  = (SBX_QID_END - 1) - NUM_COS(unit);
                   LOG_VERBOSE(BSL_LS_BCM_TRUNK,
                               (BSL_META_U(unit,
                                           "%s(BCM_TRUNK_MEMBER_EGRESS_DROP on qid %x) \n"),
                                FUNCTION_NAME(),qid));
                }

                /* ingrLag table - outHdrIdx is ignored */

                tableIndex = TRUNK_INDEX_SET(tid, jindex);
                result = _bcm_caladan3_trunk_ingLag_set(unit, tableIndex, qid, 0);
                if (BCM_E_NONE != result) break;
                index   = (index + 1) % add_info->num_ports;
            }
        } else if (0 == num_active_ports) {
            for (jindex = 0; jindex < TRUNK_SBX_FIXED_PORTCNT; jindex++) {
                tableIndex = TRUNK_INDEX_SET(tid, jindex);
                result = _bcm_caladan3_trunk_ingLag_set(unit, tableIndex, 0,
                                                      tc->invalid_oi);
                if (BCM_E_NONE != result) break;
            }
        }
    }

    if (BCM_E_NONE == result) {
        for (index = 0; index < added->num_ports ; index++) {
            module = added->tm[index];
            if (module == mymod) {
                /* only program port2Etc on this module */

                port = added->tp[index];
                member_flags = added->member_flags[index];
                /* port2Etc.SID */
                switch(SOC_SBX_CONTROL(unit)->ucodetype) {
                case SOC_SBX_UCODE_TYPE_G3P1:
                    sid = SOC_SBX_TRUNK_FTE(unit, tid);
                    break;
                default:
                    LOG_ERROR(BSL_LS_BCM_TRUNK,
                              (BSL_META_U(unit,
                                          "unknonw ucode type.\n")));
                    result = BCM_E_CONFIG;
                    break;
                }
                if (BCM_E_NONE != result) break;

                /* Member Flag: ingress_disable */
                if (add_info->member_flags[index] & BCM_TRUNK_MEMBER_INGRESS_DISABLE) {
                   LOG_VERBOSE(BSL_LS_BCM_TRUNK,
                               (BSL_META_U(unit,
                                           "%s(BCM_TRUNK_MEMBER_INGRESS_DISABLE set) \n"),
                                FUNCTION_NAME()));
                   } else {
                   result = _bcm_caladan3_trunk_port2Etc_set(unit, port, sid);
                   if (BCM_E_NONE != result) break;

                   /* egrPort2Etc.SID */
                   result = _bcm_caladan3_trunk_egrPort2Etc_set(unit, port, sid);
                   if (BCM_E_NONE != result) break;
               }
            }
        }
    }


    /*
     * Adjust outHdr mapping to one of member lag ports. Always pick the first
     * port in the list as it is always known to exist and be valid.
     */
#ifdef BCM_CALADAN3_G3P1_SUPPORT
    if (SOC_SBX_CONTROL(unit)->ucodetype ==  SOC_SBX_UCODE_TYPE_G3P1) {
        soc_sbx_g3p1_ete_t sbxEte;

          /* make sure there's an ete entry behind the eteAddr? */
        if (BCM_SUCCESS(result)) {
            eteAddr = SOC_SBX_PORT_ETE(unit, add_info->tp[0]);
            result = soc_sbx_g3p1_ete_get(unit, eteAddr, &sbxEte);
   
            if (BCM_SUCCESS(result)) {
                eteAddr = SOC_SBX_TRUNK_ETE(unit, add_info->tp[0]);
                result = soc_sbx_g3p1_ete_get(unit, eteAddr, &sbxEte);
            }
        }

        /* VLAN based static LAG support */
        if (BCM_SUCCESS(result)) {
            uint32 static_lag;
            static_lag = (add_info->psc == BCM_TRUNK_PSC_VLANINDEX) ? 1 : 0;
            result = soc_sbx_g3p1_static_lag_set(unit, static_lag);                                                 
	}
    }
#endif /* BCM_CALADAN3_G3P1_SUPPORT */


    if (BCM_E_NONE == result) {
        outHdrIndex = SOC_SBX_TRUNK_TO_OHI(tid);
        result = _bcm_caladan3_trunk_outHdr_set(unit, outHdrIndex, eteAddr);
    }

    if (BCM_E_NONE == result) {
        ti->num_ports = add_info->num_ports;
        for (index = 0; index < add_info->num_ports; index++) {
            ti->tm[index] = add_info->tm[index];
            ti->tp[index] = add_info->tp[index];
            ti->member_flags[index] = add_info->member_flags[index];
        }
    }

    sal_free(added);
    sal_free(removed);

    LOG_DEBUG(BSL_LS_BCM_TRUNK,
              (BSL_META_U(unit,
                          "%s(%d, %d, *) - Exit(%s)\n"),
               FUNCTION_NAME(), unit, tid, bcm_errmsg(result)));
    return result;
}


/*
 * Function:
 *    bcm_caladan3_trunk_vlan_remove_port
 * Purpose:
 *      When a port is removed from a VLAN, the vlan code calls this function
 *      to fix up the McPort2Etc table.
 * Parameters:
 *      unit - Device unit number.
 *      vid  - Vlan port is being removed from.
 *      port - The port being removed.
 * Returns:
 *      BCM_E_NONE      - Success or port not part of any lag.
 *      BCM_E_INIT      - Trunking software not initialized
 *      BCM_E_XXXXX     - As set by lower layers of software
 * Notes:
 *      If the port is not a member of any lag, no action is taken
 */

int
bcm_caladan3_trunk_vlan_remove_port(int unit, bcm_vlan_t vid, bcm_port_t port)
{
    LOG_VERBOSE(BSL_LS_BCM_TRUNK,
                (BSL_META_U(unit,
                            "%s(unit=%d, vid=%d, port=%d) - "
                            "This API is deprecated.\n"),
                 FUNCTION_NAME(), unit, vid, port));

    return BCM_E_NONE;
}

/*
 * Function:
 *      _bcm_trunk_create_id
 * Purpose:
 *      Create the software data structure for this trunk ID and program the
 *      hardware for this TID. User must call bcm_trunk_set() to finish setting
 *      up this trunk.
 * Parameters:
 *      unit - Device unit number.
 *      tid - The trunk ID.
 * Returns:
 *      BCM_E_NONE      - Success.
 *      BCM_E_INIT      - Trunking software not initialized
 *      BCM_E_EXISTS    - TID already used
 *      BCM_E_BADID     - TID out of range
 *      BCM_E_XXXXX     - As set by lower layers of software
 */

STATIC int
_bcm_caladan3_trunk_create_id(int unit, bcm_trunk_t tid)
{
    trunk_private_t        *ti;
    uint32                eIndex;
    uint32                outHdrIdx;
    int                     result = BCM_E_EXISTS;

    LOG_VERBOSE(BSL_LS_BCM_TRUNK,
                (BSL_META_U(unit,
                            "%s(%d, %d) - Enter\n"),
                 FUNCTION_NAME(), unit, tid));

    TRUNK_CHECK_INIT(unit, ts_init);
    TRUNK_CHECK_TID(unit, tid);

    TRUNK_DB_LOCK(unit);

    /* Go twiddle the chip */

    eIndex = SOC_SBX_TRUNK_FTE(unit, tid);
    outHdrIdx = 0;

    ti = &TRUNK_INFO(unit, tid);
    if (ti->trunk_id == BCM_TRUNK_INVALID) {
        result = _bcm_caladan3_trunk_fte_set(unit, eIndex, tid, outHdrIdx);

        if (BCM_E_NONE == result) {
            ti->trunk_id  = tid;
            ti->in_use    = FALSE;
            ti->num_ports = 0;
        }
    }

    TRUNK_DB_UNLOCK(unit);

    LOG_VERBOSE(BSL_LS_BCM_TRUNK,
                (BSL_META_U(unit,
                            "%s(%d, %d) - Exit(%s)\n"),
                 FUNCTION_NAME(), unit, tid, bcm_errmsg(result)));

    return result;
}

/*
 * Function:
 *    bcm_trunk_create
 * Purpose:
 *      Allocate an available Trunk ID from the pool
 *      bcm_trunk_create_id.
 * Parameters:
 *      unit - Device unit number.
 *      flags - Flags.
 *      tid - (IN/Out) The trunk ID, IN if BCM_TRUNK_FLAG_WITH_ID flag is set.
 * Returns:
 *      BCM_E_NONE      - Success.
 *      BCM_E_INIT      - Trunking software not initialized
 *      BCM_E_FULL      - Trunk table full, no more trunks available.
 *      BCM_E_XXXXX     - As set by lower layers of software
 */

int
bcm_caladan3_trunk_create(int unit, uint32 flags, bcm_trunk_t *tid)
{
    trunk_cntl_t       *tc;
    trunk_private_t    *ti;
    int                 result = BCM_E_FULL;
    int                 index;

    LOG_VERBOSE(BSL_LS_BCM_TRUNK,
                (BSL_META_U(unit,
                            "%s(%d, *) - Enter\n"),
                 FUNCTION_NAME(), unit));

    if (flags & BCM_TRUNK_FLAG_WITH_ID) {
        return _bcm_caladan3_trunk_create_id(unit, *tid);
    }

    TRUNK_CHECK_INIT(unit, ts_init);
    TRUNK_DB_LOCK(unit);

    *tid = 0;
    tc = &TRUNK_CNTL(unit);
    ti = &TRUNK_INFO(unit, *tid);

    for (index = 0; index < tc->ngroups; index++) {
        if (BCM_TRUNK_INVALID == ti->trunk_id) {
            result = bcm_trunk_create(unit, BCM_TRUNK_FLAG_WITH_ID, &index);
            if (BCM_E_NONE == result)
                *tid = index;
            break;
        }
        ti++;
    }

    TRUNK_DB_UNLOCK(unit);

    LOG_VERBOSE(BSL_LS_BCM_TRUNK,
                (BSL_META_U(unit,
                            "%s(%d, %d) - Exit(%s)\n"),
                 FUNCTION_NAME(), unit, *tid, bcm_errmsg(result)));

    return result;
}

/*
 * Function:
 *    bcm_trunk_psc_set
 * Purpose:
 *      Set the trunk selection criteria.
 * Parameters:
 *      unit - Device unit number.
 *      tid  - The trunk ID to be affected.
 *      psc  - Identify the trunk selection criteria.
 * Returns:
 *      BCM_E_NONE      - Success.
 *      BCM_E_INIT      - Trunking software not initialized
 *      BCM_E_BADID     - TID out of range
 *      BCM_E_NOT_FOUND - TID out of range
 *      BCM_E_PARAM     - psc value specified is not supported
 *      BCM_E_XXXXX     - As set by lower layers of software
 * Notes:
 *      On this platform, port selection criteria is global and cannot be
 *      configured per trunk group. The rule is, last psc_set wins and affects
 *      EVERY trunk group!
 */

int
_bcm_caladan3_g3p1_trunk_psc_set(int unit, bcm_trunk_t tid, int psc)
{
    trunk_cntl_t       *tc;
    int                 result = BCM_E_NOT_FOUND;
    uint32            flags = 0;
    uint32 rc;
    uint32 found = 1;

    LOG_VERBOSE(BSL_LS_BCM_TRUNK,
                (BSL_META_U(unit,
                            "%s(%d, %d, %d) - Enter\n"),
                 FUNCTION_NAME(),
                 unit, tid, psc));
    LOG_DEBUG(BSL_LS_BCM_TRUNK,
              (BSL_META_U(unit,
                          "  Reminder, this is a global action, not "
                           "per Trunk Group.\n")));

    tc = &TRUNK_CNTL(unit);

    if (tc->psc == psc) {
        return BCM_E_NONE;
    }
        
    result = BCM_E_NONE;
       
    /* check for 'standard' psc values */
    switch (psc){

    case BCM_TRUNK_PSC_SRCMAC:
        flags |= SB_G3P1_PSC_MAC_SA;
        break;
    case BCM_TRUNK_PSC_DSTMAC:
        flags |= SB_G3P1_PSC_MAC_DA;
        break;
    case BCM_TRUNK_PSC_SRCDSTMAC:
        flags |= (SB_G3P1_PSC_MAC_SA | SB_G3P1_PSC_MAC_DA);
        break;
    case BCM_TRUNK_PSC_SRCIP:
        flags |= SB_G3P1_PSC_IP_SA;
        break;
    case BCM_TRUNK_PSC_DSTIP:
        flags |= SB_G3P1_PSC_IP_DA;
        break;
    case BCM_TRUNK_PSC_SRCDSTIP:
        flags |= (SB_G3P1_PSC_IP_SA | SB_G3P1_PSC_IP_DA);
        break;
    case BCM_TRUNK_PSC_VLANINDEX:
        flags |= SB_G3P1_PSC_VID;
        break;
    default:
        /* try extended flags below */
        found = 0;
        break;
    }

    if (!found){

        if (psc & ~_BCM_TRUNK_PSC_EXTENDED_FLAGS) {
            LOG_ERROR(BSL_LS_BCM_TRUNK,
                      (BSL_META_U(unit,
                                  "unit %d: [%s] Invalid psc 0x%x\n"),
                       unit, FUNCTION_NAME(), psc));
            return BCM_E_PARAM;
        }

        if (psc & BCM_TRUNK_PSC_MACSA){
            flags |= SB_G3P1_PSC_MAC_SA;
        }
        if (psc & BCM_TRUNK_PSC_MACDA){
            flags |= SB_G3P1_PSC_MAC_DA;
        }
        if (psc & BCM_TRUNK_PSC_IPSA){
            flags |= SB_G3P1_PSC_IP_SA;
        }
        if (psc & BCM_TRUNK_PSC_IPDA){
            flags |= SB_G3P1_PSC_IP_DA;
        }
        if (psc & BCM_TRUNK_PSC_L4SS){
            flags |= SB_G3P1_PSC_L4SS;
        }
        if (psc & BCM_TRUNK_PSC_L4DS){
            flags |= SB_G3P1_PSC_L4DS;
        }
        if (psc & BCM_TRUNK_PSC_VID){
            flags |= SB_G3P1_PSC_VID;
        }
    }

    rc = soc_sbx_g3p1_ppe_entry_psc_set(unit, flags);
    if (rc != SOC_E_NONE) {
        result = translate_sbx_result(rc);
        LOG_ERROR(BSL_LS_BCM_TRUNK,
                  (BSL_META_U(unit,
                              "soc_sbx_g3p1_ppe_entry_psc_set failed: flags 0x%x, %s (0x%x)\n"),
                   flags, bcm_errmsg(result), result));
        return BCM_E_INTERNAL;
    }

    tc->psc = psc;

    LOG_VERBOSE(BSL_LS_BCM_TRUNK,
                (BSL_META_U(unit,
                            "%s(%d, %d, %d) - Exit(%s)\n"),
                 FUNCTION_NAME(), unit, tid, psc, bcm_errmsg(result)));

    return result;
}

int
bcm_caladan3_trunk_psc_set(int unit, bcm_trunk_t tid, int psc)
{
    trunk_cntl_t       *tc;
    int                 result = BCM_E_NOT_FOUND;
    uint32            eType = 0;

    LOG_VERBOSE(BSL_LS_BCM_TRUNK,
                (BSL_META_U(unit,
                            "%s(%d, %d, %d) - Enter\n"),
                 FUNCTION_NAME(),
                 unit, tid, psc));
    LOG_DEBUG(BSL_LS_BCM_TRUNK,
              (BSL_META_U(unit,
                          "  Reminder, this is a global action, not "
                           "per Trunk Group.\n")));

    TRUNK_CHECK_INIT(unit, ts_init);
    TRUNK_CHECK_TID(unit, tid);

    if (TRUNK_TID_VALID(unit, tid)) {

        TRUNK_DB_LOCK(unit);

        tc = &TRUNK_CNTL(unit);

#if SBX_HASH_DEFINED
#ifdef BCM_CALADAN3_G3P1_SUPPORT
        if (SOC_IS_SBX_G3P1(unit)) {
            int rv = _bcm_caladan3_g3p1_trunk_psc_set(unit, tid, psc);
            TRUNK_DB_UNLOCK(unit);
            return rv;
        }
#endif /* BCM_CALADAN3_G3P1_SUPPORT */

        result = BCM_E_NONE;
        if (psc != tc->psc) {
            switch (psc) {
            case BCM_TRUNK_PSC_SRCMAC:
            case BCM_TRUNK_PSC_DSTMAC:
            case BCM_TRUNK_PSC_SRCDSTMAC:
                eType = SB_FE_HASH_OVER_L2;
                break;
            case BCM_TRUNK_PSC_SRCIP:
            case BCM_TRUNK_PSC_DSTIP:
            case BCM_TRUNK_PSC_SRCDSTIP:
                eType = SB_FE_HASH_OVER_L3;
                break;
            case BCM_TRUNK_PSC_L4SS:
            case BCM_TRUNK_PSC_L4DS:
                eType = SB_FE_HASH_OVER_L4;
                break;
            default:
                result = BCM_E_PARAM;
            }

            if (BCM_E_NONE == result) {                
#else /* SBX_HASH_DEFINED */
                tc->psc = psc;
                LOG_DEBUG(BSL_LS_BCM_TRUNK,
                          (BSL_META_U(unit,
                                      "This API currently performs no hardware action.\n")));
#endif /* SBX_HASH_DEFINED */
            }
        }
        TRUNK_DB_UNLOCK(unit);
    }

    LOG_VERBOSE(BSL_LS_BCM_TRUNK,
                (BSL_META_U(unit,
                            "%s(%d, %d, %d %d) - Exit(%s)\n"),
                 FUNCTION_NAME(), unit, tid, psc, eType, bcm_errmsg(result)));

    return result;
}

/*
 * Function:
 *    bcm_trunk_psc_get
 * Purpose:
 *      Get the trunk selection criteria.
 * Parameters:
 *      unit - Device unit number.
 *      tid  - The trunk ID to be used.
 *      psc  - (OUT) Identify the trunk selection criteria.
 * Returns:
 *      BCM_E_NONE      - Success.
 *      BCM_E_INIT      - Trunking software not initialized
 *      BCM_E_BADID     - TID out of range
 *      BCM_E_NOT_FOUND - TID out of range
 */

int
bcm_caladan3_trunk_psc_get(int unit, bcm_trunk_t tid, int *psc)
{
    int                 result = BCM_E_NOT_FOUND;

    LOG_VERBOSE(BSL_LS_BCM_TRUNK,
                (BSL_META_U(unit,
                            "%s(%d, %d, *) - Enter\n"),
                 FUNCTION_NAME(), unit, tid));

    TRUNK_CHECK_INIT(unit, ts_init);
    TRUNK_CHECK_TID(unit, tid);

    if TRUNK_TID_VALID(unit, tid) {
        *psc = TRUNK_CNTL(unit).psc;
        result = BCM_E_NONE;
    }

    LOG_VERBOSE(BSL_LS_BCM_TRUNK,
                (BSL_META_U(unit,
                            "%s(%d, %d, %d) - Exit(%s)\n"),
                 FUNCTION_NAME(), unit, tid, *psc, bcm_errmsg(result)));

    return result;
}


/*
 * Function:
 *    bcm_trunk_chip_info_get
 * Purpose:
 *      Get device specific trunking information.
 * Parameters:
 *      unit    - Device unit number.
 *      ta_info - (OUT) Chip specific Trunk information.
 * Returns:
 *      BCM_E_NONE      - Success.
 *      BCM_E_INIT      - Trunking software not initialized
 */

int
bcm_caladan3_trunk_chip_info_get(int unit, bcm_trunk_chip_info_t *ta_info)
{
    trunk_cntl_t   *tc;
    int             result = BCM_E_NONE;

    LOG_VERBOSE(BSL_LS_BCM_TRUNK,
                (BSL_META_U(unit,
                            "%s(%d, *) - Enter\n"),
                 FUNCTION_NAME(), unit));

    TRUNK_CHECK_INIT(unit, ts_init);

    tc = &TRUNK_CNTL(unit);

    ta_info->trunk_group_count = tc->ngroups;
    ta_info->trunk_id_min = 0;
    ta_info->trunk_id_max = tc->ngroups - 1;
    ta_info->trunk_ports_max = BCM_TRUNK_MAX_PORTCNT;
    ta_info->trunk_fabric_id_min = -1;
    ta_info->trunk_fabric_id_max = -1;
    ta_info->trunk_fabric_ports_max = -1;

    LOG_VERBOSE(BSL_LS_BCM_TRUNK,
                (BSL_META_U(unit,
                            "%s(%d, *) - Exit(%s)\n"),
                 FUNCTION_NAME(), unit, bcm_errmsg(result)));

    return result;
}

/*
 * Function:
 *    bcm_trunk_set
 * Purpose:
 *      Add ports to a trunk group.
 * Parameters:
 *      unit       - Device unit number.
 *      tid        - The trunk ID the ports are added to.
 *      t_add_info - Information on the trunk group.
 * Returns:
 *      BCM_E_NONE      - Success.
 *      BCM_E_INIT      - Trunking software not initialized
 *      BCM_E_BADID     - TID out of range
 *      BCM_E_NOT_FOUND - the specified TID was not found
 *      BCM_E_PARAM     - too or invalid many ports specified
 *      BCM_E_XXXXX     - As set by lower layers of software
 * Notes:
 *      Any existing ports in the trunk group will be replaced with new ones.
 */
int
bcm_caladan3_trunk_set_old(int unit, bcm_trunk_t tid, bcm_trunk_add_info_t *add_info)
{
    trunk_cntl_t       *tc;
    trunk_private_t    *ti;
    int                 index, port_change = 0;
    int                 result = BCM_E_NONE;

    LOG_VERBOSE(BSL_LS_BCM_TRUNK,
                (BSL_META_U(unit,
                            "%s(%d, %d, [n=%d - "),
                 FUNCTION_NAME(), unit, tid, add_info->num_ports));
    for (index = 0; index < add_info->num_ports; index++) {
        LOG_VERBOSE(BSL_LS_BCM_TRUNK,
                    (BSL_META_U(unit,
                                " %d:%d flags:%x"),
                     add_info->tm[index], add_info->tp[index], add_info->member_flags[index]));
    }
    LOG_VERBOSE(BSL_LS_BCM_TRUNK,
                (BSL_META_U(unit,
                            "]) - Enter\n")));

    TRUNK_CHECK_INIT(unit, ts_init);
    TRUNK_CHECK_TID(unit, tid);

    TRUNK_DB_LOCK(unit);

    tc = &TRUNK_CNTL(unit);
    ti = &TRUNK_INFO(unit, tid);

    /* make sure trunk is in use */
    if (TRUNK_TID_VALID(unit, tid)) {

        /* Check number of ports in trunk group */
        if (TRUNK_PORTCNT_VALID(unit, add_info->num_ports)) {
            result = _bcm_caladan3_trunk_set(unit, tid, add_info, &port_change);
            if (BCM_E_NONE == result) {
                ti->in_use  = TRUE;
            }
            if (port_change == 0) {
                result = BCM_E_NONE;
            }
        } else {
            result = BCM_E_PARAM;
        }
    } else {
        result = BCM_E_NOT_FOUND;
    }

    /* Call the registered callbacks. */
    for (index = 0; index < _BCM_TRUNK_MAX_CALLBACK; index++) {
        if (tc->callback_func[index]) {
            (tc->callback_func[index])(unit, tid, add_info, tc->callback_user_data[index]);
#if 0
            if (!BCM_SUCCESS(rv)) {
	        result = rv;
                LOG_ERROR(BSL_LS_BCM_TRUNK,
                          (BSL_META_U(unit,
                                      "Callback number %d failed with %d\n"),
                           index, rv));
	    }
#endif
        }
    }

    TRUNK_DB_UNLOCK(unit);

    LOG_VERBOSE(BSL_LS_BCM_TRUNK,
                (BSL_META_U(unit,
                            "%s(%d, %d, *) - Exit(%s)\n"),
                 FUNCTION_NAME(), unit, tid, bcm_errmsg(result)));

    return result;
}

/*
 * Function:
 *  bcm_caladan3_trunk_set
 * Purpose:
 *      Adds ports to a trunk group.
 * Parameters:
 *      unit - Device unit number.
 *      tid - The trunk ID to be affected.
 *      trunk_info - Information on the trunk group.
 *      member_count - Number of trunk members.
 *      member_array - Array of trunk members.
 * Returns:
 *      BCM_E_NONE              Success.
 *      BCM_E_XXX
 * Notes:
 *
 */
int 
bcm_caladan3_trunk_set(int unit, bcm_trunk_t tid, bcm_trunk_info_t *trunk_info,
                       int member_count, bcm_trunk_member_t *member_array)
{
    bcm_trunk_add_info_t *add_info = NULL;
    int index = 0;
    int status = BCM_E_NONE;

    if (member_count > TRUNK_SBX_FIXED_PORTCNT ) {
        LOG_ERROR(BSL_LS_BCM_TRUNK,
                  (BSL_META_U(unit,
                              "Number of members is greater than %d\n"),
                   TRUNK_SBX_FIXED_PORTCNT));
        return BCM_E_PARAM;
    }

    if (!member_array) {
        LOG_ERROR(BSL_LS_BCM_TRUNK,
                  (BSL_META_U(unit,
                              "Member array is NULL\n")));
        return BCM_E_PARAM;
    }

    add_info = sal_alloc(sizeof(bcm_trunk_add_info_t), "add_info");
    if (add_info == NULL) {
        LOG_ERROR(BSL_LS_BCM_TRUNK,
                  (BSL_META_U(unit,
                              "Could not allocate bcm_trunk_add_info_t\n")));
        return BCM_E_MEMORY;
    }

    bcm_trunk_add_info_t_init(add_info);
    add_info->num_ports    = member_count;
    add_info->flags        = trunk_info->flags;
    add_info->psc          = trunk_info->psc;
    add_info->ipmc_psc     = trunk_info->ipmc_psc;
    add_info->dlf_index    = trunk_info->dlf_index;
    add_info->mc_index     = trunk_info->mc_index;
    add_info->ipmc_index   = trunk_info->ipmc_index;
    add_info->dynamic_size = trunk_info->dynamic_size;
    add_info->dynamic_age  = trunk_info->dynamic_age;
    add_info->dynamic_load_exponent = trunk_info->dynamic_load_exponent;
    add_info->dynamic_expected_load_exponent =
                            trunk_info->dynamic_expected_load_exponent;
    for (index = 0; index < member_count; index++) {
        if (!BCM_GPORT_IS_MODPORT(member_array[index].gport)) {
            LOG_ERROR(BSL_LS_BCM_TRUNK,
                      (BSL_META_U(unit,
                                  "Member(%d)'s gport is not modport\n"),
                       index));
            sal_free(add_info);
            return BCM_E_PARAM;
        }
        add_info->member_flags[index] = member_array[index].flags;
        add_info->tp[index] = SOC_GPORT_MODPORT_PORT_GET (member_array[index].gport);
        add_info->tm[index] = SOC_GPORT_MODPORT_MODID_GET(member_array[index].gport);
    }
    status = bcm_caladan3_trunk_set_old(unit, tid, add_info);
    sal_free(add_info);
    return status;
}               

/*
 * Function:
 *    bcm_trunk_get
 * Purpose:
 *      Return information of a given trunk ID.
 * Parameters:
 *      unit   - Device unit number.
 *      tid    - Trunk ID.
 *      t_data - (Out), data about this trunk.
 * Returns:
 *      BCM_E_NONE      - Success.
 *      BCM_E_INIT      - Trunking software not initialized
 *      BCM_E_BADID     - TID out of range
 *      BCM_E_NOT_FOUND - the specified TID was not found
 */

int
bcm_caladan3_trunk_get_old(int unit, bcm_trunk_t tid, bcm_trunk_add_info_t *t_data)
{
    trunk_cntl_t       *tc;
    trunk_private_t    *ti;
    int                 index;
    int                 result = BCM_E_NOT_FOUND;

    LOG_VERBOSE(BSL_LS_BCM_TRUNK,
                (BSL_META_U(unit,
                            "%s(%d, %d, *) - Enter\n"),
                 FUNCTION_NAME(), unit, tid));

    TRUNK_CHECK_INIT(unit, ts_init);
    TRUNK_CHECK_TID(unit, tid);

    TRUNK_DB_LOCK(unit);

    tc = &TRUNK_CNTL(unit);
    ti = &TRUNK_INFO(unit, tid);

    sal_memset(t_data, 0, sizeof(bcm_trunk_add_info_t));
    if (ti->trunk_id != BCM_TRUNK_INVALID) {
        result = BCM_E_NONE;
        t_data->psc = tc->psc;
        t_data->dlf_index = t_data->mc_index = t_data->ipmc_index = -1;
        t_data->num_ports = ti->num_ports;

        for (index = 0; index < ti->num_ports; index++) {
            t_data->member_flags[index] = ti->member_flags[index];
            t_data->tm[index] = ti->tm[index];
            t_data->tp[index] = ti->tp[index];
        }
    }

    TRUNK_DB_UNLOCK(unit);

    LOG_DEBUG(BSL_LS_BCM_TRUNK,
              (BSL_META_U(unit,
                          "%s(%d, %d, [n=%d - "),
               FUNCTION_NAME(), unit, tid, t_data->num_ports));
    for (index = 0; index < t_data->num_ports; index++) {
        LOG_DEBUG(BSL_LS_BCM_TRUNK,
                  (BSL_META_U(unit,
                              " %d:%d flags:%x"),
                   t_data->tm[index], t_data->tp[index], t_data->member_flags[index]));
    }
    LOG_VERBOSE(BSL_LS_BCM_TRUNK,
                (BSL_META_U(unit,
                            "]) - Exit(%s)\n"),
                 bcm_errmsg(result)));

    return result;
}

/*
 * Function:
 *      bcm_caladan3_trunk_get
 * Purpose:
 *      Return a port information of given trunk ID.
 * Parameters:
 *      unit - Device unit number.
 *      tid - Trunk ID.
 *      trunk_info   - (OUT) Place to store returned trunk info.
 *      member_max   - (IN) Size of member_array.
 *      member_array - (OUT) Place to store returned trunk members.
 *      member_count - (OUT) Place to store returned number of trunk members.
 * Returns:
 *      BCM_E_NONE              Success.
 *      BCM_E_XXX
 */
int 
bcm_caladan3_trunk_get(int unit, bcm_trunk_t tid, bcm_trunk_info_t *trunk_info,
                       int member_max, bcm_trunk_member_t *member_array,
                       int *member_count)
{
    bcm_trunk_add_info_t *add_info=NULL;
    int index = 0;
    int status = BCM_E_NONE;

    if (!trunk_info || !member_array || !member_count) {
        LOG_ERROR(BSL_LS_BCM_TRUNK,
                  (BSL_META_U(unit,
                              "Null Pointer\n")));
        return BCM_E_PARAM;
    }

    add_info = sal_alloc(sizeof(bcm_trunk_add_info_t), "add_info");
    if (add_info == NULL) {
        LOG_ERROR(BSL_LS_BCM_TRUNK,
                  (BSL_META_U(unit,
                              "Could not allocate bcm_trunk_add_info_t\n")));
        return BCM_E_MEMORY;
    }

    bcm_trunk_add_info_t_init(add_info);

    status = bcm_caladan3_trunk_get_old(unit, tid, add_info);
    if (status != BCM_E_NONE) {
        sal_free(add_info);
        return status;
    }

    *member_count = add_info->num_ports;
    if (member_max < *member_count) {
        LOG_WARN(BSL_LS_BCM_TRUNK,
                 (BSL_META_U(unit,
                             "member_max(%d) is less than actual number of members(%d)\n"),
                  member_max, *member_count));
        *member_count = member_max;
    }

    trunk_info->flags        = add_info->flags;
    trunk_info->psc          = add_info->psc;
    trunk_info->ipmc_psc     = add_info->ipmc_psc;
    trunk_info->dlf_index    = add_info->dlf_index;
    trunk_info->mc_index     = add_info->mc_index;
    trunk_info->ipmc_index   = add_info->ipmc_index;
    trunk_info->dynamic_size = add_info->dynamic_size;
    trunk_info->dynamic_age  = add_info->dynamic_age;;
    trunk_info->dynamic_load_exponent = add_info->dynamic_load_exponent;
    trunk_info->dynamic_expected_load_exponent =
                                add_info->dynamic_expected_load_exponent;
    for (index = 0; index < (*member_count); index++) {
        member_array[index].flags = add_info->member_flags[index];
        SOC_GPORT_MODPORT_SET(member_array[index].gport, add_info->tm[index], add_info->tp[index]);
    }

    sal_free(add_info);
    return status;
}          

/*
 * Function:
 *    bcm_trunk_destroy
 * Purpose:
 *      Removes a trunk group. Performs hardware steps neccessary to tear
 *      down a create trunk.
 * Parameters:
 *      unit - Device unit number.
 *      tid  - Trunk Id.
 * Returns:
 *      BCM_E_NONE      - Success.
 *      BCM_E_INIT      - Trunking software not initialized
 *      BCM_E_BADID     - TID out of range
 *      BCM_E_NOT_FOUND - Trunk does not exist
 *      BCM_E_XXXXX     - As set by lower layers of software
 * Notes:
 *      The return code of the trunk_set call is purposely ignored.
 */

int
bcm_caladan3_trunk_destroy(int unit, bcm_trunk_t tid)
{
    trunk_cntl_t           *tc;
    trunk_private_t        *ti;
    uint32                eIndex;
    bcm_trunk_add_info_t    *add_info = NULL;
    int                     result = BCM_E_NOT_FOUND;
    uint32                tIndex;

    LOG_VERBOSE(BSL_LS_BCM_TRUNK,
                (BSL_META_U(unit,
                            "%s(%d, %d) - Enter\n"),
                 FUNCTION_NAME(), unit, tid));

    TRUNK_CHECK_INIT(unit, ts_init);
    TRUNK_CHECK_TID(unit, tid);

    TRUNK_DB_LOCK(unit);

    eIndex = SOC_SBX_TRUNK_FTE(unit, tid);

    tc = &TRUNK_CNTL(unit);
    ti = &TRUNK_INFO(unit, tid);

    add_info = sal_alloc(sizeof(bcm_trunk_add_info_t), "add_info");
    if (add_info == NULL) {
        LOG_ERROR(BSL_LS_BCM_TRUNK,
                  (BSL_META_U(unit,
                              "Could not allocate bcm_trunk_add_info_t\n")));
        TRUNK_DB_UNLOCK(unit);
        return BCM_E_MEMORY;
    }

    if (ti->trunk_id != BCM_TRUNK_INVALID) {
        if (0 < ti->num_ports) {
            sal_memset((void *)add_info, 0, sizeof(bcm_trunk_add_info_t));
            add_info->psc = tc->psc;
            (void)bcm_caladan3_trunk_set_old(unit, tid, add_info);
        }

        result = _bcm_caladan3_trunk_fte_inValidate(unit, eIndex);

        if (BCM_E_NONE == result) {
            ti->trunk_id  = BCM_TRUNK_INVALID;
            ti->in_use    = FALSE;
            ti->num_ports = 0;
        }
    }

    /* If the last trunk has been destroyed, reset the PSC criteria to UNDEFINED */
    for (tIndex=0; tIndex < SBX_MAX_TRUNKS; tIndex++){
        ti = &TRUNK_INFO(unit, tIndex);
        if (ti->in_use == TRUE) {
            break;
        }
    }
    if (tIndex == SBX_MAX_TRUNKS){
        tc->psc = _BCM_TRUNK_PSC_UNDEFINED;
    }

    sal_free(add_info);
    TRUNK_DB_UNLOCK(unit);

    LOG_VERBOSE(BSL_LS_BCM_TRUNK,
                (BSL_META_U(unit,
                            "%s(%d, %d) - Exit(%s)\n"),
                 FUNCTION_NAME(), unit, tid, bcm_errmsg(result)));

    return result;
}

/*
 * Function:
 *    bcm_trunk_detach
 * Purpose:
 *      Shuts down the trunk module.
 * Parameters:
 *      unit - Device unit number.
 * Returns:
 *      BCM_E_NONE              Success.
 *      BCM_E_XXX
 */

int
bcm_caladan3_trunk_detach(int unit)
{
    trunk_cntl_t       *tc;
    int                 result = BCM_E_NONE;

    LOG_VERBOSE(BSL_LS_BCM_TRUNK,
                (BSL_META_U(unit,
                            "%s(%d) - Enter\n"),
                 FUNCTION_NAME(), unit));

    /* Don't use TRUNK_CHECK_INIT macro here - If module is not initialized
     * just return OK.
     */
    if (!BCM_UNIT_VALID(unit)) {
        return BCM_E_UNIT;
    } else if (unit >= BCM_MAX_NUM_UNITS) {
        /* trunk_control is sized by BCM_MAX_NUM_UNITS, not BCM_CONTROL_MAX,
         * or BCM_UNITS_MAX 
         */
        return BCM_E_UNIT;
    } else if (TRUNK_CNTL(unit).init_state == ts_none) {
        return BCM_E_NONE;
    }

    tc = &TRUNK_CNTL(unit);

    TRUNK_DB_LOCK(unit);

    tc->init_state = ts_none;

    /* Free trunk_private_t (t_info) structures */
    if (NULL != tc->t_info) {
        sal_free(tc->t_info);
        tc->t_info = NULL;
    }

    /* Set number of ports and groups to zero */
    tc->ngroups = 0;
    tc->nports  = 0;

    TRUNK_DB_UNLOCK(unit);

    /* Destroy LOCK (no more data to protect */
    if (NULL != tc->lock) {
        sal_mutex_destroy(tc->lock);
        tc->lock = NULL;
    }

    LOG_VERBOSE(BSL_LS_BCM_TRUNK,
                (BSL_META_U(unit,
                            "%s(%d) - Exit(%s)\n"),
                 FUNCTION_NAME(), unit, bcm_errmsg(result)));
    return result;
}

/*
 * Function:
 *    bcm_trunk_bitmap_expand
 * Purpose:
 *      Given a port bitmap, if any of the ports are in a trunk,
 *      add all the trunk member ports to the bitmap.
 * Parameters:
 *      unit     - Device unit number.
 *      pbmp_ptr - Input/output port bitmap
 * Returns:
 *      BCM_E_NONE      - Success.
 *      BCM_E_INIT      - Trunking software not initialized
 * Notes:
 */

int
bcm_caladan3_trunk_bitmap_expand(int unit, bcm_pbmp_t *pbmp_ptr)
{
    trunk_cntl_t       *tc;
    trunk_private_t    *ti;
    bcm_trunk_t         tid;
    bcm_port_t          port;
    int                 index;
    bcm_pbmp_t          pbmp, t_pbmp;
    int                 result = BCM_E_NONE;

    LOG_VERBOSE(BSL_LS_BCM_TRUNK,
                (BSL_META_U(unit,
                            "%s(%d, %d) - Enter\n"),
                 FUNCTION_NAME(), unit, SOC_PBMP_WORD_GET(*pbmp_ptr,0)));

    TRUNK_CHECK_INIT(unit, ts_init);

    TRUNK_DB_LOCK(unit);

    tc = &TRUNK_CNTL(unit);

    for (tid = 0; tid < tc->ngroups; tid++) {
        ti = &TRUNK_INFO(unit, tid);
        if ((TRUE == ti->in_use) && (0 < ti->num_ports)) {
            BCM_PBMP_CLEAR(t_pbmp);
            BCM_PBMP_CLEAR(pbmp);
            for (index=0; index<ti->num_ports; index++) {
                /* create the port bitmap of this trunk */
                port = ti->tp[index];
                BCM_PBMP_PORT_ADD(t_pbmp, port);    /* construct temp bitmap */
            }
            BCM_PBMP_ASSIGN(pbmp, t_pbmp);      /* save a copy */
            BCM_PBMP_AND(t_pbmp, *pbmp_ptr);    /* find common ports */

            /* if lists have common member */
            if (TRUE == BCM_PBMP_NOT_NULL(t_pbmp)) {
                BCM_PBMP_OR(*pbmp_ptr, pbmp);   /* add saved member set */
            }
        }
    }
    TRUNK_DB_UNLOCK(unit);

    LOG_VERBOSE(BSL_LS_BCM_TRUNK,
                (BSL_META_U(unit,
                            "%s(%d, %d) - Exit(%s)\n"),
                 FUNCTION_NAME(), unit, SOC_PBMP_WORD_GET(*pbmp_ptr,0), bcm_errmsg(result)));

    return result;
}


/*
 * Function:
 *    bcm_trunk_find
 * Description:
 *      Get trunk id that contains the given system port
 * Parameters:
 *      unit    - Device unit number
 *      modid   - Module ID
 *      port    - Port number
 *      tid     - (OUT) Trunk id
 * Returns:
 *      BCM_E_NONE      - Success.
 *      BCM_E_INIT      - Trunking software not initialized
 *      BCM_E_NOT_FOUND - The module:port combo was not found in a trunk.
 */
int
bcm_caladan3_trunk_find(int unit, bcm_module_t modid, bcm_port_t port,
                      bcm_trunk_t *tid)
{
    trunk_cntl_t       *tc;
    trunk_private_t    *ti;
    bcm_trunk_t         t;
    int                 index;
    int                 result = BCM_E_NOT_FOUND;

    LOG_VERBOSE(BSL_LS_BCM_TRUNK,
                (BSL_META_U(unit,
                            "%s(%d, %d, %d, *) - Enter\n"),
                 FUNCTION_NAME(), unit, modid, port));

    TRUNK_CHECK_INIT(unit, ts_init);

    TRUNK_DB_LOCK(unit);

    tc = &TRUNK_CNTL(unit);

    *tid = BCM_TRUNK_INVALID;
    for (t = 0; t < tc->ngroups; t++) {
        ti = &TRUNK_INFO(unit, t);
        if ((TRUE == ti->in_use) && (0 < ti->num_ports)) {
            for (index = 0; index < ti->num_ports; index++) {
                if ((ti->tm[index] == modid) && (ti->tp[index] == port)) {
                    *tid = ti->trunk_id;
                    result = BCM_E_NONE;
                    break;
                }
            }
        }
        if (BCM_E_NONE == result) {
            break;
        }
    }

    TRUNK_DB_UNLOCK(unit);

    LOG_VERBOSE(BSL_LS_BCM_TRUNK,
                (BSL_META_U(unit,
                            "%s(%d, %d, %d, %d) - Exit(%s)\n"),
                 FUNCTION_NAME(), unit, modid, port, *tid, bcm_errmsg(result)));
    return result;
}

/*
 * Function:
 *    bcm_trunk_egress_set
 * Description:
 *    Set switching only to indicated ports from given trunk.
 * Parameters:
 *    unit - Device unit number.
 *      tid - Trunk Id.  Negative trunk id means set all trunks.
 *    pbmp - bitmap of ports to allow egress.
 * Returns:
 *      BCM_E_xxxx
 */
int
bcm_caladan3_trunk_egress_set(int unit, bcm_trunk_t tid, bcm_pbmp_t pbmp)
{
    LOG_VERBOSE(BSL_LS_BCM_TRUNK,
                (BSL_META_U(unit,
                            "%s(unit %d, tid %d) - This API is not available.\n"),
                 FUNCTION_NAME(), unit, tid));
#ifdef BROADCOM_DEBUG
    _bcm_caladan3_trunk_debug(unit);
#endif
    return BCM_E_UNAVAIL;
}



/*
 * Function:
 *      bcm_trunk_init
 * Purpose:
 *      Initializes the trunk module. The hardware and the software data
 *      structures are both set to their initial states with no trunks
 *      configured.
 * Parameters:
 *      unit - Device unit number.
 * Returns:
 *      BCM_E_NONE      - Success.
 *      BCM_E_MEMORY    - Out of memory
 *      BCM_E_XXXXX     - As set by lower layers of software
 */

int
bcm_caladan3_trunk_init(int unit)
{
    trunk_cntl_t               *tc;
    trunk_private_t            *ti;
    int                         alloc_size;
    bcm_trunk_t                 tid;
    uint32                    eIndex;
    uint32                    eteAddr;
    int                         index;
    int                         result = BCM_E_NONE;

    LOG_VERBOSE(BSL_LS_BCM_TRUNK,
                (BSL_META_U(unit,
                            "%s(%d) - Enter\n"),
                 FUNCTION_NAME(), unit));

    if (!BCM_UNIT_VALID(unit) || (unit >= BCM_MAX_NUM_UNITS)) {
        return BCM_E_UNIT;
    }

    tc = &TRUNK_CNTL(unit);


    if (tc->t_info != NULL && tc->lock != NULL) {
        result = bcm_caladan3_trunk_detach (unit);
        if (result != BCM_E_NONE) {
            return result;
        }
        tc->lock = NULL;
        tc->t_info = NULL;
    }
 
    if (NULL == tc->lock) {
        if (NULL == (tc->lock = sal_mutex_create("caladan3_trunk_lock"))) {
            return BCM_E_MEMORY;
        }
    }
    tc->ngroups = _ngroups;
    tc->nports  = _nports;
    tc->psc     = _BCM_TRUNK_PSC_UNDEFINED;

    if (tc->t_info != NULL) {
        sal_free(tc->t_info);
        tc->t_info = NULL;
    }

    /* clear memory for the callback array */
    sal_memset(tc->callback_func, 0, sizeof(tc->callback_func));
    sal_memset(tc->callback_user_data, 0, sizeof(tc->callback_user_data));

    /* alloc memory and clear */
    if (tc->ngroups > 0) {
        alloc_size = tc->ngroups * sizeof(trunk_private_t);
        ti = sal_alloc(alloc_size, "Trunk-private");
        if (NULL == ti) {
            return BCM_E_MEMORY;
        }
        tc->t_info = ti;
        sal_memset(ti, 0, alloc_size);
    } else {
        return BCM_E_INTERNAL;
    }

    /* Init internal structures */
    ti = tc->t_info;
    for (tid = 0; tid < tc->ngroups; tid++) {
        /* disable all trunk group and clear all trunk bitmap */
        ti->trunk_id        = BCM_TRUNK_INVALID;
        ti->in_use          = FALSE;
        ti++;
    }

    tc->invalid_oi = 0;

#ifdef BCM_WARM_BOOT_SUPPORT
    TRUNK_DB_LOCK(unit);
    result = bcm_caladan3_wb_trunk_state_init(unit);
    if (SOC_WARM_BOOT(unit) ) {
        /* recovery complete; initialization complete */
        if (BCM_E_NONE == result) {
            tc->init_state = ts_init;
        }
        TRUNK_DB_UNLOCK(unit);
        return result;
    }
    TRUNK_DB_UNLOCK(unit);
#endif /* BCM_WARM_BOOT_SUPPORT */
    
    /* now go twiddle the chip */
    for (tid = 0; tid < tc->ngroups; tid++) {
        /*
         * One outHdrIdx is consumed per trunk. Should be linked to a
         * unique L2 ETE entry. This allows the Trunk port attributes
         * (tagging options mostly) to be managed.
         */
        
        eteAddr = SOC_SBX_TRUNK_ETE(unit, tid);

#ifdef BCM_CALADAN3_G3P1_SUPPORT
        if (SOC_SBX_CONTROL(unit)->ucodetype ==  SOC_SBX_UCODE_TYPE_G3P1) {
            soc_sbx_g3p1_ete_t sbxEteL2;
            soc_sbx_g3p1_ete_t_init(&sbxEteL2);
            sbxEteL2.mtu = SBX_DEFAULT_MTU_SIZE;
            result = soc_sbx_g3p1_ete_set(unit, eteAddr, &sbxEteL2);
        }
#endif /* BCM_CALADAN3_P3_SUPPORT */

        if (BCM_E_NONE != result) break;

        eIndex  = SOC_SBX_TRUNK_OHI(tid);
        result  = _bcm_caladan3_trunk_outHdr_set(unit, eIndex, eteAddr);
        if (BCM_E_NONE != result) break;

        for (index = 0; index < TRUNK_SBX_FIXED_PORTCNT; index++) {
            eIndex = TRUNK_INDEX_SET(tid, index);
            result = _bcm_caladan3_trunk_ingLag_set(unit, eIndex, 0,
                                                    tc->invalid_oi);
            if (BCM_E_NONE != result) break;
        }
        if (BCM_E_NONE != result) break;

        eIndex = SOC_SBX_TRUNK_FTE(unit, tid);
        result = _bcm_caladan3_trunk_fte_inValidate(unit, eIndex);
        if (BCM_E_NONE != result) break;
    }

    if (BCM_E_NONE == result) {
        tc->init_state = ts_init;
    }

    LOG_VERBOSE(BSL_LS_BCM_TRUNK,
                (BSL_META_U(unit,
                            "bcm_trunk_init: unit=%d result=%d(%s)\n"),
                 unit, result, bcm_errmsg(result)));

    return result;
}

/* Returns the set difference of info_a and info_b.  
 * Specifically, <mod,port> pairs found in info_a not present info_b 
 */
void
bcm_caladan3_trunk_add_info_cmp(bcm_trunk_add_info_t *info_a,
                                bcm_trunk_add_info_t *info_b,
                                int                  *num_ports,
                                bcm_module_t          mods[BCM_TRUNK_MAX_PORTCNT],
                                bcm_port_t            ports[BCM_TRUNK_MAX_PORTCNT])
{
    int idxa, idxb, cnt;
    cnt = 0;

    /* Compare info_a to info_b, build a set of mod/ports that are present in
     * info_a, but not in info_b.
     */
    for (idxa = 0; idxa < info_a->num_ports; idxa++) {
            
        /* scan the last known membership for this port */
        for (idxb = 0; idxb < info_b->num_ports; idxb++) {
            if (info_b->tm[idxb] == info_a->tm[idxa] &&
                info_b->tp[idxb] == info_a->tp[idxa]) {
                break;
            }
        }
        
        /* mod,port not found in info_a; add it to the list */
        if (idxb >= info_b->num_ports) {
            mods[cnt]  = info_a->tm[idxa];
            ports[cnt] = info_a->tp[idxa];
            cnt++;
        }
    }

    *num_ports = cnt;
}


/*
 * Function:
 *      bcm_trunk_change_register
 * Purpose:
 *      Registers a callback routine, to be called whenever bcm_trunk_set
 *      is called.
 * Parameters:
 *      unit      - Device unit number.
 *      callback  - The callback function to call.
 *      user_data - An opaque cookie to pass to callback function.
 *                  whenever it is called.
 * Returns:
 *      BCM_E_NONE  - Success
 *      BCM_E_PARAM - NULL function pointer or bad unit.
 *      BCM_E_FULL  - Cannot register more than _BCM_TRUNK_MAX_CALLBACK callbacks.
 *      BCM_E_EXISTS - Cannot register the same callback twice.
 *      BCM_E_XXXX  - As set by lower layers of software.
 */

int
bcm_caladan3_trunk_change_register(int unit,
                                 bcm_trunk_notify_cb callback,
                                 void *user_data)
{
    int                         rv = BCM_E_FULL;
    trunk_cntl_t               *tc;
    uint32                    i;
    unsigned char *p = (unsigned char *)&callback;
    char addr[16];

    sal_sprintf(addr, "0x%02x%02x%02x%02x", p[0], p[1], p[2], p[3]);
    LOG_VERBOSE(BSL_LS_BCM_TRUNK,
                (BSL_META_U(unit,
                            "%s(%d, %s, %p) - Enter\n"),
                 FUNCTION_NAME(), unit, addr, user_data));

    TRUNK_CHECK_INIT(unit, ts_recovering);

    if (!callback) {
        return BCM_E_PARAM;
    }

    TRUNK_DB_LOCK(unit);

    tc = &TRUNK_CNTL(unit);

    /* find empty entry */
    for (i = 0; i < _BCM_TRUNK_MAX_CALLBACK; i++) {
        if (tc->callback_func[i] == callback) {
	    rv = BCM_E_EXISTS;
            break;
	}
    }

    if (rv != BCM_E_EXISTS) {
        for (i = 0; i < _BCM_TRUNK_MAX_CALLBACK; i++) {
            if (tc->callback_func[i] == NULL) {
                tc->callback_func[i] = callback;
                tc->callback_user_data[i] = user_data;
	        rv = BCM_E_NONE;
                break;
	    }
	}
    }

    TRUNK_DB_UNLOCK(unit);

    LOG_VERBOSE(BSL_LS_BCM_TRUNK,
                (BSL_META_U(unit,
                            "register: unit=%d rv=%d\n"),
                 unit, rv));

    return rv;
}

/*
 * Function:
 *      bcm_trunk_change_unregister
 * Purpose:
 *      Unregisters a callback.
 * Parameters:
 *      unit      - Device unit number.
 *      callback  - The callback function to be unregistered.
 * Returns:
 *      BCM_E_NONE  - Success
 *      BCM_E_PARAM - NULL function pointer or bad unit.
 *      BCM_E_XXXX  - As set by lower layers of software.
 */

int
bcm_caladan3_trunk_change_unregister(int unit, bcm_trunk_notify_cb callback)
{
    int                         rv = BCM_E_NOT_FOUND;
    trunk_cntl_t               *tc;
    uint32                    i;
    unsigned char *p = (unsigned char *)&callback;
    char addr[16];

    sal_sprintf(addr, "0x%02x%02x%02x%02x", p[0], p[1], p[2], p[3]);
    LOG_VERBOSE(BSL_LS_BCM_TRUNK,
                (BSL_META_U(unit,
                            "%s(%d, %s) - Enter\n"),
                 FUNCTION_NAME(), unit, addr));

    TRUNK_CHECK_INIT(unit, ts_recovering);

    if (!callback) {
        return BCM_E_PARAM;
    }

    TRUNK_DB_LOCK(unit);

    tc = &TRUNK_CNTL(unit);

    /* check if exists, and remove */
    for (i = 0; i < _BCM_TRUNK_MAX_CALLBACK; i++) {
        if (tc->callback_func[i] == callback) {
            tc->callback_func[i] = NULL;
            tc->callback_user_data[i] = NULL;
	    rv = BCM_E_NONE;
            break;
	}
    }

    TRUNK_DB_UNLOCK(unit);

    LOG_VERBOSE(BSL_LS_BCM_TRUNK,
                (BSL_META_U(unit,
                            "unregister: unit=%d rv=%d\n"),
                 unit, rv));

    return rv;
}

