/*
 * $Id: g3p1_int.h,v 1.28 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * This file contains aggregated definitions for Guadalupe 3 microcode
 */

#ifndef _SOC_SBX_G3P1_INT_H
#define _SOC_SBX_G3P1_INT_H

#include <soc/types.h>
#include <soc/drv.h>

#if defined(BCM_CALADAN3_SUPPORT) && defined(BCM_CALADAN3_G3P1_SUPPORT)

#include <soc/sbx/sbx_drv.h>
#include <soc/sbx/caladan3/ocm.h>
#include <soc/sbx/caladan3/lrp.h>
#include <soc/sbx/caladan3/util.h>
#include <soc/sbx/caladan3/port.h>
#include <soc/sbx/caladan3/ucodemgr.h>
#include <soc/sbx/caladan3/util.h>
#include <sal/appl/sal.h>
#include <soc/debug.h>
#include <soc/sbx/caladan3/port.h>
#include <soc/sbx/caladan3/ped.h>
#include <soc/sbx/caladan3/ppe.h>
#include <soc/sbx/g3p1/g3p1_ppe_rule_encode.h>
#include <soc/sbx/g3p1/g3p1_ped.h>
#include <soc/sbx/g3p1/g3p1_tmu.h>
#include <soc/sbx/g3p1/g3p1_cop.h>
#include <soc/sbx/g3p1/g3p1_cmu.h>
#include <soc/sbx/g3p1/g3p1_ppe_tables.h>

#include <soc/sbx/caladan3/simintf.h>

/* Bit mask fields used in fieldMask argument to psc hash template get/set functions */
#define SB_G3P1_PSC_MAC_DA    0x00000001
#define SB_G3P1_PSC_MAC_SA    0x00000002
#define SB_G3P1_PSC_IP_DA     0x00000004
#define SB_G3P1_PSC_IP_SA     0x00000008
#define SB_G3P1_PSC_L4SS      0x00000010
#define SB_G3P1_PSC_L4DS      0x00000020
#define SB_G3P1_PSC_VID       0x00000040
#define SB_G3P1_PSC_VID_INNER 0x00000080

#define SOC_SBX_G3P1_BUBBLE_UPDATE_MODE_DISABLED            0
#define SOC_SBX_G3P1_BUBBLE_UPDATE_MODE_RESERVED            1
#define SOC_SBX_G3P1_BUBBLE_UPDATE_MODE_INSERT_COUNT_TIMES  2
#define SOC_SBX_G3P1_BUBBLE_UPDATE_MODE_INSERT_CONTINUOUSLY 3
#define SOC_SBX_G3P1_BUBBLE_UPDATE_MODE_LAST                SOC_SBX_G3P1_BUBBLE_UPDATE_MODE_INSERT_CONTINUOUSLY


typedef struct soc_sbx_g3p1_state_s {
    int unit;
    soc_sbx_caladan3_ucode_pkg_t *ucode;
    void *regSet;
    void *tableinfo;
    void *tmu_mgr;
    void *ppe_mgr;
} soc_sbx_g3p1_state_t;

typedef struct soc_sbx_g3p1_util_timer_event_s {
    soc_sbx_g3p1_cop_timer_segment_t timer_segment;
    uint32 id;
    uint8  forced_timeout;
    uint8  timer_active_when_forced;
} soc_sbx_g3p1_util_timer_event_t;

/**
 *
 * PPE Property table configuration tables
 */
#define SOC_SBX_G3P1_PPE_PROPERTY_TABLE_SEGMENT_MAX 8
typedef struct soc_sbx_g3p1_ppe_ptable_segment_s {
  char *name;
  int segment;
  int seg_id;
  int start;
} soc_sbx_g3p1_ppe_ptable_segment_t;

typedef struct soc_sbx_g3p1_ppe_ptable_cfg_s {
  int mode;
  int portA;
  int portB;
  int portC;
  int portD;
  soc_sbx_g3p1_ppe_ptable_segment_t segment[SOC_SBX_G3P1_PPE_PROPERTY_TABLE_SEGMENT_MAX];
} soc_sbx_g3p1_ppe_ptable_cfg_t;

typedef void (*soc_sbx_g3p1_util_timer_event_callback_f)(int unit, soc_sbx_g3p1_util_timer_event_t *event, void *user_cookie);


#define UTG_SYM_GET(h, p, n, v) \
    soc_sbx_caladan3_ucodemgr_sym_get(h, p, n, v)
#define UTG_SYM_SET(h, p, n, v) \
    soc_sbx_caladan3_ucodemgr_sym_set(h, p, n, v)

/* Hand coded PPE functions */
extern int soc_sbx_g3p1_ppe_property_table_segment_get(int unit, int seg_id,
                                                       uint32 offset, uint8 *data);
extern int soc_sbx_g3p1_ppe_property_table_segment_set(int unit, int seg_id,
                                                       uint32 offset, uint8  data);

extern int soc_sbx_g3p1_ppe_init_ext(int unit);
extern int soc_sbx_g3p1_ppe_entry_lsm_set(int unit, uint8 lsi, soc_sbx_g3p1_lsmac_t *lsm);
extern int soc_sbx_g3p1_ppe_entry_lsm_get(int unit, uint8 lsi, soc_sbx_g3p1_lsmac_t *lsm);

extern int soc_sbx_g3p1_ppe_entry_elsmac_set(int unit, int lsi, soc_sbx_g3p1_elsmac_t *elsmac);
extern int soc_sbx_g3p1_ppe_entry_elsmac_get(int unit, int lsi, soc_sbx_g3p1_elsmac_t *elsmac);
extern int soc_sbx_g3p1_ppe_entry_tpid_set(int unit, uint8 tpidi, soc_sbx_g3p1_tpid_t *tpid);
extern int soc_sbx_g3p1_ppe_entry_tpid_get(int unit, uint8 tpidi, soc_sbx_g3p1_tpid_t *tpid);

extern int soc_sbx_g3p1_ppe_queue_psc_set(int unit, uint8 queueid, uint32 psc);
extern int soc_sbx_g3p1_ppe_queue_psc_get(int unit, uint8 queueid, uint32* psc);
extern int soc_sbx_g3p1_ppe_entry_psc_set(int unit, uint32 psc);
extern int soc_sbx_g3p1_ppe_entry_psc_get(int unit, uint32* psc);

extern int soc_sbx_g3p1_ppe_entry_p2e_get(int unit, int iport, soc_sbx_g3p1_p2e_t *e);
extern int soc_sbx_g3p1_ppe_entry_p2e_set(int unit, int iport, soc_sbx_g3p1_p2e_t *e);
extern int soc_sbx_g3p1_ppe_entry_ep2e_set(int unit, int iport, soc_sbx_g3p1_ep2e_t *e);
extern int soc_sbx_g3p1_ppe_entry_ep2e_get(int unit, int iport, soc_sbx_g3p1_ep2e_t *e);
extern int soc_sbx_g3p1_ppe_entry_oam_rx_set(int unit, int irulenum , soc_sbx_g3p1_oam_rx_t *e);
extern int soc_sbx_g3p1_ppe_entry_oam_rx_get(int unit, int irulenum , soc_sbx_g3p1_oam_rx_t *e);
extern int soc_sbx_g3p1_ppe_entry_oam_tx_set(int unit, int irulenum , soc_sbx_g3p1_oam_tx_t *e);
extern int soc_sbx_g3p1_ppe_entry_oam_tx_get(int unit, int irulenum , soc_sbx_g3p1_oam_tx_t *e);
extern int soc_sbx_g3p1_ppe_entry_lsmac_set(int unit, int ilsi, soc_sbx_g3p1_lsmac_t *e);
extern int soc_sbx_g3p1_ppe_entry_lsmac_get(int unit, int ilsi, soc_sbx_g3p1_lsmac_t *e);
extern int soc_sbx_g3p1_ppe_entry_l2cpmac_set(int unit, int ilsi, soc_sbx_g3p1_l2cpmac_t *e);
extern int soc_sbx_g3p1_ppe_entry_l2cpmac_get(int unit, int ilsi, soc_sbx_g3p1_l2cpmac_t *e);

extern int soc_sbx_g3p1_bubble_table_init(int unit);
extern int soc_sbx_g3p1_bubble_entry_set(int unit,  uint32 bubble_idx, uint8 count, uint8 interval_index, 
                                         uint8 task, uint8 stream, uint8 init, uint8 jitter_enable, uint8 update_mode);
extern int soc_sbx_g3p1_bubble_entry_get(int unit,  uint32 bubble_idx, uint8 *count, uint8 *interval_index, 
                                         uint8 *task, uint8 *stream, uint8 *init, uint8 *jitter_enable, uint8 *update_mode);

extern int soc_sbx_g3p1_util_pvv2e_get(int unit, int port, int ovid, int ivid, soc_sbx_g3p1_pvv2edata_t *e);
extern int soc_sbx_g3p1_util_pvv2e_set(int unit, int port, int ovid, int ivid, soc_sbx_g3p1_pvv2edata_t *e);
extern int soc_sbx_g3p1_util_pvv2e_update(int unit, int port, int ovid, int ivid, soc_sbx_g3p1_pvv2edata_t *e);
extern int soc_sbx_g3p1_util_pvv2e_add(int unit, int port, int ovid, int ivid, soc_sbx_g3p1_pvv2edata_t *e);
extern int soc_sbx_g3p1_util_pvv2e_remove(int unit, int port, int ovid, int ivid);
extern int soc_sbx_g3p1_util_pvv2e_first(int unit, int *port, int *ovid, int *ivid);
extern int soc_sbx_g3p1_util_pvv2e_next(int unit, int port, int ovid, int ivid, int *nport, int *novid, int *nivid);

extern int soc_sbx_g3p1_iqsm_stream_get(int unit, uint8 queueid, uint32 *str);
extern int soc_sbx_g3p1_iqsm_stream_set(int unit, uint8 queueid, uint32 str);
extern int soc_sbx_g3p1_iqsm_checker_get(int unit, uint8 queueid, uint32 *checker);
extern int soc_sbx_g3p1_iqsm_checker_set(int unit, uint8 queueid, uint32 checker);

extern int soc_sbx_g3p1_mac_bulk_delete(int unit, uint32 *filter_key, uint32 *filter_key_mask,
                                                   uint32 *filter_value, uint32 *filter_value_mask);

extern uint32 soc_sbx_g3p1_util_crc32_word(uint32 x);
extern int soc_sbx_g3p1_util_register_timer_callback(int unit, soc_sbx_g3p1_util_timer_event_callback_f cb, void *user_cookie);

#endif /* BCM_CALADAN3_SUPPORT */
#endif
