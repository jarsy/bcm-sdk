/* 
 * $Id: policer.h,v 1.9 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:        policer.h
 * Purpose:     Policer internal definitions to the BCM library.
 */

#ifndef _BCM_INT_SBX_FE2000_POLICER_H_
#define _BCM_INT_SBX_FE2000_POLICER_H_

#define BCM_FE2000_XT_POLICERS  0xff /* 1-255 are reserved for exceptions */
#define BCM_FE2000_PORT_POLICERS (SBX_MAX_PORTS * SBX_MAX_COS)
#define BCM_FE2000_SPECIAL_POLICERS     1 /* so far just one: drop all */
#define BCM_FE2000_SPEC_POL_DROP_ALL    (BCM_FE2000_XT_POLICERS + \
                                         BCM_FE2000_PORT_POLICERS + 1)
#define BCM_FE2000_RESERVED_POLICERS    (BCM_FE2000_XT_POLICERS + \
                                         BCM_FE2000_PORT_POLICERS + \
                                         BCM_FE2000_SPECIAL_POLICERS)

#define BCM_FE2000_POLICER_GROUPID       SOC_SBX_G2P3_INGEGRPOL_ID
#define BCM_FE2000_EXC_POLICER_GROUPID   SOC_SBX_G2P3_XTPOL_ID

#define BCM_FE2000_NUM_MONITORS         0x8     /* Num of monitor resources */

extern int
_bcm_policer_max_id_get(int unit);

extern int
_bcm_fe2000_policer_group_mode_get(int unit, bcm_policer_t id,
                                   bcm_policer_group_mode_t *grp_mode);

extern int
_bcm_fe2000_policer_unit_lock(int unit);

extern int
_bcm_fe2000_policer_unit_unlock(int unit);

extern int
_bcm_fe2000_attach_monitor(int unit, bcm_policer_t pol_id);

extern int
_bcm_fe2000_detach_monitor(int unit, bcm_policer_t pol_id);

extern int
_bcm_fe2000_is_monitor_stat_enabled(int unit, bcm_policer_t policer_id, 
                                    int *monitor_id);
extern int
_bcm_fe2000_monitor_stat_get(int unit, int monitor_id, 
                            bcm_policer_stat_t stat, uint64 *val);
extern int
_bcm_fe2000_monitor_stat_set(int unit, int monitor_id, 
                             bcm_policer_stat_t stat, uint64 val);


extern const char *
_bcm_fe2000_policer_group_mode_to_str(bcm_policer_group_mode_t groupMode);

extern int
bcm_fe2000_policer_stat_get(int unit, bcm_policer_t policer_id, bcm_cos_t cos,
                            bcm_policer_stat_t stat, uint64 *val);
extern int
bcm_fe2000_policer_stat_set(int unit, bcm_policer_t policer_id, bcm_cos_t cos,
                            bcm_policer_stat_t stat, uint64 val);

#ifdef BCM_FE2000_P3_SUPPORT

extern int _bcm_fe2000_g2p3_policer_lp_program(int unit, bcm_policer_t pol_id,
                                               soc_sbx_g2p3_lp_t *lp);

extern int _bcm_fe2000_g2p3_policer_stat_mem_get(int unit,
                                            bcm_policer_group_mode_t grp_mode, 
                                            bcm_policer_stat_t stat, int cos,
                                            int *ctr_offset, int *pkt);

extern int
_bcm_fe2000_g2p3_alloc_counters(int unit, 
                                bcm_policer_t policer_id,
                                int use_cookie,
                                uint32 *counter);

extern int
_bcm_fe2000_g2p3_free_counters(int unit,
                               bcm_policer_t policer_id,
                               int use_cookie,
                               uint32 counter);

extern int
_bcm_fe2000_g2p3_policer_stat_get(int unit, bcm_policer_t policer_id, 
                                  bcm_policer_stat_t stat, int cos, 
                                  int use_cookie, uint32 counter,
                                  int clear, uint64 *val);

extern int
_bcm_fe2000_g2p3_policer_stat_set(int unit, bcm_policer_t policer_id, 
                                  bcm_policer_stat_t stat, int cos,
                                  int use_cookie, uint32 counter,
                                  uint64 val);

extern int
_bcm_fe2000_g2p3_num_counters_get(int unit,
                                  bcm_policer_group_mode_t mode,
                                  int *ncounters);


#endif /* BCM_FE2000_P3_SUPPORT */

#endif /* _BCM_INT_SBX_FE2000_POLICER_H_ */
