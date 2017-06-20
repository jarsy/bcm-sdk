/*
 * $Id: t3p1_int.h,v 1.1 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * This file contains aggregated definitions for Guadalupe 3 microcode
 */

#ifndef _SOC_SBX_T3P1_INT_H
#define _SOC_SBX_T3P1_INT_H

#include <soc/types.h>
#include <soc/drv.h>

#ifdef BCM_CALADAN3_SUPPORT

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
#include <soc/sbx/t3p1/t3p1_ppe_rule_encode.h>
#include <soc/sbx/t3p1/t3p1_ped.h>
#include <soc/sbx/t3p1/t3p1_tmu.h>
#include <soc/sbx/t3p1/t3p1_cop.h>
#include <soc/sbx/t3p1/t3p1_cmu.h>
#include <soc/sbx/t3p1/t3p1_ppe_tables.h>

#include <soc/sbx/caladan3/simintf.h>

/* Bit mask fields used in fieldMask argument to psc hash template get/set functions */

#define SOC_SBX_T3P1_BUBBLE_UPDATE_MODE_DISABLED            0
#define SOC_SBX_T3P1_BUBBLE_UPDATE_MODE_RESERVED            1
#define SOC_SBX_T3P1_BUBBLE_UPDATE_MODE_INSERT_COUNT_TIMES  2
#define SOC_SBX_T3P1_BUBBLE_UPDATE_MODE_INSERT_CONTINUOUSLY 3
#define SOC_SBX_T3P1_BUBBLE_UPDATE_MODE_LAST                SOC_SBX_T3P1_BUBBLE_UPDATE_MODE_INSERT_CONTINUOUSLY


typedef struct soc_sbx_t3p1_state_s {
    int unit;
    soc_sbx_caladan3_ucode_pkg_t *ucode;
    void *regSet;
    void *tableinfo;
    void *tmu_mgr;
    void *ppe_mgr;
} soc_sbx_t3p1_state_t;

typedef struct soc_sbx_t3p1_util_timer_event_s {
    soc_sbx_t3p1_cop_timer_segment_t timer_segment;
    uint32 id;
    uint8  forced_timeout;
    uint8  timer_active_when_forced;
} soc_sbx_t3p1_util_timer_event_t;

/**
 *
 * PPE Property table configuration tables
 */
#define SOC_SBX_T3P1_PPE_PROPERTY_TABLE_SEGMENT_MAX 8
typedef struct soc_sbx_t3p1_ppe_ptable_segment_s {
  char *name;
  int segment;
  int seg_id;
  int start;
} soc_sbx_t3p1_ppe_ptable_segment_t;

typedef struct soc_sbx_t3p1_ppe_ptable_cfg_s {
  int mode;
  int portA;
  int portB;
  int portC;
  int portD;
  soc_sbx_t3p1_ppe_ptable_segment_t segment[SOC_SBX_T3P1_PPE_PROPERTY_TABLE_SEGMENT_MAX];
} soc_sbx_t3p1_ppe_ptable_cfg_t;

typedef void (*soc_sbx_t3p1_util_timer_event_callback_f)(int unit, soc_sbx_t3p1_util_timer_event_t *event, void *user_cookie);


#define UTG_SYM_GET(h, p, n, v) \
    soc_sbx_caladan3_ucodemgr_sym_get(h, p, n, v)
#define UTG_SYM_SET(h, p, n, v) \
    soc_sbx_caladan3_ucodemgr_sym_set(h, p, n, v)

/* Hand coded PPE functions */
extern int soc_sbx_t3p1_ppe_property_table_segment_get(int unit, int seg_id,
                                                       uint32 offset, uint8 *data);
extern int soc_sbx_t3p1_ppe_property_table_segment_set(int unit, int seg_id,
                                                       uint32 offset, uint8  data);

extern int soc_sbx_t3p1_ppe_entry_p2e_get(int unit, int iport, soc_sbx_t3p1_p2e_t *e);
extern int soc_sbx_t3p1_ppe_entry_p2e_set(int unit, int iport, soc_sbx_t3p1_p2e_t *e);
extern int soc_sbx_t3p1_ppe_entry_ep2e_set(int unit, int iport, soc_sbx_t3p1_ep2e_t *e);
extern int soc_sbx_t3p1_ppe_entry_ep2e_get(int unit, int iport, soc_sbx_t3p1_ep2e_t *e);

extern uint32 soc_sbx_t3p1_util_crc32_word(uint32 x);
extern int soc_sbx_t3p1_util_register_timer_callback(int unit, soc_sbx_t3p1_util_timer_event_callback_f cb, void *user_cookie);

#endif /* BCM_CALADAN3_SUPPORT */
#endif
