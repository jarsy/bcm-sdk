/*
 * $Id: recovery.h,v 1.7 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#ifndef _BCM_INT_SBX_FE2000_RECOVERY_H_
#define _BCM_INT_SBX_FE2000_RECOVERY_H_
#ifdef BCM_WARM_BOOT_SUPPORT

#include <sal/types.h>

/* Private FE2000 interface for recovery storage access.  Provides a single 
 * collection point for usable on-chip bits used during a warmboot.  Any 
 * usable bit for recovery that is not interfaced by this module is 
 * not supported. 
 */

/* _bcm_fe2000_recovery_var_t defines the list of internal state that can be 
 * recovered without external storage.  The recovery module takes advantage of
 * all unused on-chip memory to store the state 
 */
typedef enum _bcm_fe2000_recovery_var_e {
#define RECOVERY_ENTRY_VAR_PLACE(x)  x ,
#include <bcm_int/sbx/fe2000/recovery_vars.h>

    fe2k_rcvr_num_elements
} _bcm_fe2000_recovery_var_t;

#define RCVR_FAST_PV2E_IDX(port__, vid__)   ((port__) * BCM_VLAN_COUNT + (vid__))

/* Vlan must init first during a warmboot.  When it does, it will scan the soft
 * copy of VSIs per port,vid.  All TB VSIs will be recovered as well as VLAN
 * GPORTS.  All remaining unknown VSIs will be inserted into the rc_non_tb_vsi
 * list for faster access by the remaining BCM modules to recover.
 */
typedef struct rc_non_tb_vsi_s {
    dq_t      dql;   /* dq-link; double-linked list connection */
    uint16    port;
    uint16    vid;
} rc_non_tb_vsi_t;


int _bcm_fe2000_recovery_init(int unit);
int _bcm_fe2000_recovery_commit(int unit);
int _bcm_fe2000_recovery_done(int unit);
int _bcm_fe2000_recovery_deinit(int unit);

/*  _bcm_fe2000_recovery_var_t primative accessors
 */
int _bcm_fe2000_recovery_var_get(int unit, 
                                 _bcm_fe2000_recovery_var_t var, uint32 *val);
int _bcm_fe2000_recovery_var_set(int unit, 
                                 _bcm_fe2000_recovery_var_t var, uint32 val);


/* Accessors to cached fast memory table reads, available only during a 
 * warmboot.
 */
uint32 *_bcm_fe2000_recovery_pv2e_vlan_get(int unit);
uint32 *_bcm_fe2000_recovery_pv2e_lpi_get(int unit);

int _bcm_fe2000_recovery_non_tb_vsi_insert(int unit, uint16 port, uint16 vid);
int _bcm_fe2000_recovery_non_tb_vsi_head_get(int unit, rc_non_tb_vsi_t **head);


#endif /* BCM_WARM_BOOT_SUPPORT */
#endif  /* _BCM_INT_SBX_FE2000_RECOVERY_H_ */
