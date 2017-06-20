/*
 * $Id: sbx_common.h,v 1.16 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * configuration common to sbx devices
 */

#ifndef _SBX_COMMON_H_
#define _SBX_COMMON_H_

#include <soc/sbx/sbTypes.h>

/* Some compilers do not support 'packed' attribute */
#if defined(VXWORKS) && (CPU == PPC603) && (VX_VERSION == 54)
#define PACK_STRUCT
#else
#define PACK_STRUCT __attribute__ ((__packed__))
#endif

#define SOC_SBX_CONN_FIXED            (0x01)
#define SOC_SBX_CONN_RESERVED         (0x02)

#define SOC_SBX_TEMPLATE_FIXED        (SOC_SBX_CONN_FIXED)
#define SOC_SBX_TEMPLATE_RESERVED     (SOC_SBX_CONN_RESERVED)

typedef struct sbx_conn_template_s {
    sbBool_t    in_use;
    int8      template;
    int8      flags;
    uint32    value;
    uint32    ref_cnt;
} sbx_conn_template_t, sbx_template_t;

typedef struct sbx_conn_template_info_s {
    int                    max_template;
    sbx_conn_template_t   *template;
} sbx_conn_template_info_t, sbx_template_info_t;

typedef int (*sbx_template_compare_f)(void *a, void *b);
typedef void (*sbx_template_copy_f)(void *src, void *dest);
typedef void (*sbx_template_init_f)(void *a);

typedef struct sbx_template_clbk_s {
    sbx_template_compare_f    compare;
    sbx_template_copy_f       update;
    sbx_template_copy_f       get;
    sbx_template_init_f       init;
} sbx_template_clbk_t;



int
soc_sbx_connect_init(int unit, int conn_util_max, int conn_age_max);

extern int
soc_sbx_connect_deinit(int unit);

int
soc_sbx_connect_reload(int unit, uint32 *util, uint32 *age);

int
soc_sbx_wb_connect_state_deinit(int unit);

int
soc_sbx_connect_min_util_template_get(int unit, int utilization, int *template);

int
soc_sbx_connect_min_util_get(int unit, int template, int *utilization);

int
soc_sbx_connect_min_util_alloc(int unit, int flags, int utilization, int *is_allocated, int *template);

int
soc_sbx_connect_min_util_dealloc(int unit, int flags, int utilization, int *is_deallocated, int *template);

int
soc_sbx_connect_max_age_template_get(int unit, int age, int *template);

int
soc_sbx_connect_max_age_get(int unit, int template, int *age);

int
soc_sbx_connect_max_age_alloc(int unit, int flags, int age, int *is_allocated, int *template);

int
soc_sbx_connect_max_age_dealloc(int unit, int flags, int age, int *is_deallocated, int *template);

int
soc_sbx_sched_get_internal_state(int unit, int sched_mode, int int_pri, int *queue_type, int *priority, int *priority2);

int
soc_sbx_sched_get_internal_state_queue_attach(int unit, int sched_mode, int int_pri, int *queue_type, int *priority, int *hungry_priority);

int
soc_sbx_sched_config_params_verify(int unit, int sched_mode, int int_pri);

int
soc_sbx_sched_config_set_params_verify(int unit, int sched_mode, int int_pri);

extern int 
soc_sbx_process_custom_stats(int unit, int links);

#endif /* _SBX_COMMON_H_ */
