/* 
 * $Id: l2.h,v 1.16 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:        l2.h
 * Purpose:     L2 internal definitions to the BCM library.
 */

#ifndef   _BCM_INT_SBX_L2_H_
#define   _BCM_INT_SBX_L2_H_

#include <shared/bsl.h>

#ifdef BCM_FE2000_P3_SUPPORT
#include <soc/sbx/g2p3/g2p3.h>
#endif /* BCM_FE2000_P3_SUPPORT */

#include <bcm/types.h>
#include <bcm/debug.h>
#include <bcm/l2.h>

#define CHAR_SET   'X'
#define CHAR_CLEAR '-'

#define L2_6B_MAC_FMT       "%2.2x:%2.2x:%2.2x:%2.2x:%2.2x:%2.2x"
#define L2_6B_MAC_PFMT(mac) (mac)[0], (mac)[1], (mac)[2], \
                            (mac)[3], (mac)[4], (mac)[5]


/* G2 Defines and Macros */
#define SB_COMMIT_COMPLETE   0xffffffff  /* Force completion on G2k call */

#define SB_SUCCESS(rv)          ((rv) == SB_OK)
#define SB_FAILURE(rv)          ((rv) != SB_OK)
#define IS_SB_IN_PROGRESS(rv)   ((rv) == SB_IN_PROGRESS)
#define TO_BCM_ERROR(op)        translate_sbx_result(op)


/* G2 FE device handler */

#ifdef BCM_FE2000_P3_SUPPORT
#define G2P3_FE_FROM_UNIT(unit) \
    ((soc_sbx_g2p3_state_t *)SOC_SBX_CONTROL(unit)->drv)
#define G2P3_FE_HANDLER_GET(unit, fe)  \
    if ((fe = G2P3_FE_FROM_UNIT(unit)) == NULL) {  \
        return BCM_E_INIT;  \
    }
#endif /* BCM_FE2000_P3_SUPPORT */


/* G2 FE device handler */

#ifdef BCM_CALADAN3_G3P1_SUPPORT
#define G3P1_FE_FROM_UNIT(unit) \
    ((soc_sbx_g3p1_state_t *)SOC_SBX_CONTROL(unit)->drv)
#define G3P1_FE_HANDLER_GET(unit, fe)  \
    if ((fe = G3P1_FE_FROM_UNIT(unit)) == NULL) {  \
        return BCM_E_INIT;  \
    }

int
_bcm_caladan3_l2_addr_dump(int unit, bcm_mac_t mac, bcm_vlan_t vid,
                         int max_count);

#endif /* BCM_CALADAN3_G3P1_SUPPORT */



extern sal_mutex_t      _l2_mlock[BCM_LOCAL_UNITS_MAX];
#define L2_LOCK(unit)    sal_mutex_take(_l2_mlock[unit], sal_mutex_FOREVER)
#define L2_UNLOCK(unit)  sal_mutex_give(_l2_mlock[unit])

#define L2_DUMP     bsl_printf

int
_bcm_fe2000_l2_addr_dump(int unit, bcm_mac_t mac, bcm_vlan_t vid,
                         int max_count);

int _bcm_fe2000_l2_age_ager_get(int unit);

int _bcm_fe2000_l2_flush_cache(int unit);
int _bcm_fe2000_l2_mcast_get(int unit, bcm_l2_addr_t *l2addr);


/* L2 Ager code */
/* Callbacks for L2 age processing routine are of this form */
typedef void (*age_cb)(int unit,bcm_mac_t mac, int vsid, int delete_flag);

/*
 *   Function
 *      _bcm_caladan3_g3p1_run_ager
 *   Purpose
 *     Walk the L2 MAC table and invokes the callback routine on aged entries
 *
 *   Parameters
 *      (IN) unit   		 : unit number of the device
 *      (IN) aged_mac_hndr   : Callback routine invoked on aged entries
 *      (IN) delete  		 : Whether to delete the entry or only report
 *      (IN) current         : Current age epoch
 *      (IN) entries     	 : Number of entries to process
 *      (IN) age_range       : Range of possible values for entry age [0,age_range-1]
 *
 *   Returns
 *       BCM_E_NONE     - Processed requested number of entries
 *       BCM_E_EMPTY    - Ran out of entries
 *   Notes
 */
int _bcm_caladan3_g3p1_run_ager(int unit,age_cb aged_mac_hndr, int delete, int current, int entries, int age_range);

/*
 *   Function
 *      bcm_fe2000_l2_egress_range_reserve
 *   Purpose
 *     Reserve a range of L2 egress IDs
 *
 *   Parameters
 *      (IN)  unit   : unit number of the device
 *      (IN) type    : resource to set
 *      (IN) highOrLow  : TRUE - set Upper bounds
 *                      : FALSE - set lower bounds
 *      (IN) val    : inclusive bound to set
 *   Returns
 *       BCM_E_NONE - All required resources are allocated
 *       BCM_E_*    - failure
 *   Notes
 */
int bcm_fe2000_l2_egress_range_reserve(int unit, int highOrLow, uint32 val);

/*
 *   Function
 *      bcm_fe2000_l2_egress_range_get
 *   Purpose
 *     Retrieve the range of valid L2 egress IDs
 *
 *   Parameters
 *      (IN)  unit   : unit number of the device
 *      (OUT) first  : first valid ID
 *      (OUT) last   : last valid ID
 *   Returns
 *       BCM_E_NONE - All required resources are allocated
 *       BCM_E_*    - failure
 *   Notes
 */
int bcm_fe2000_l2_egress_range_get(int unit, uint32 *low, uint32 *high);


/*
 * L2 Compare
 * Used internally for 'delete_by_xxx' routines.
 */
#define L2_CMP_VLAN              0x1
#define L2_CMP_MAC               0x2
#define L2_CMP_PORT              0x4
#define L2_CMP_TRUNK             0x8
#define CMP_VLAN(vid1, vid2)    ((vid1) - (vid2))

#define PARAM_NULL_CHECK(arg) \
    if ((arg) == NULL) { return BCM_E_PARAM; }

/*
 * WB sequence numbers
 * Used internally to differentiate L2, L2_CACHE scache
 */
#define L2_WB_L2            0
#define L2_WB_L2CACHE       1
#endif /* _BCM_INT_SBX_L2_H_ */
