/*
* $Id: instr.h,v 1.0 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:        instrumentation.h
 */

#ifndef _BCM_INT_INSTRUMENTATION_H
#define _BCM_INT_INSTRUMENTATION_H

#include <soc/defs.h>

#ifdef BCM_INSTRUMENTATION_SUPPORT

#include <bcm/types.h>
#include <bcm/switch.h>


/* packet trace feature support */
extern int bcm_esw_pkt_trace_init(int unit);
extern int _bcm_esw_pkt_trace_info_get(int unit, uint32 options, uint8 port, 
                                       int len, uint8 *data, 
                                       bcm_switch_pkt_trace_info_t 
                                                            *pkt_trace_info);

extern int _bcm_esw_pkt_trace_src_port_set(int unit, uint32 logical_src_port);
extern int _bcm_esw_pkt_trace_src_port_get(int unit);
extern int _bcm_esw_pkt_trace_src_pipe_get(int unit);
extern int _bcm_esw_pkt_trace_hw_reset(int unit);
extern int _bcm_esw_pkt_trace_cpu_profile_get(int unit, uint32* profile_id);
typedef struct _bcm_switch_pkt_trace_port_info_s {
    uint32 pkt_trace_src_logical_port;
    uint8  pkt_trace_src_pipe;

} _bcm_switch_pkt_trace_port_info_t;


/* for tomahawk */
/* size is in number of double words */
#define TH_PTR_RESULTS_IVP_MAX_INDEX  2 
#define TH_PTR_RESULTS_ISW1_MAX_INDEX 4
#define TH_PTR_RESULTS_ISW2_MAX_INDEX 8
extern int _bcm_th_pkt_trace_info_get(int unit,
                                      bcm_switch_pkt_trace_info_t *pkt_trace_info);
extern int _bcm_th_pkt_trace_src_port_set(int unit, uint32 logical_src_port);
extern int _bcm_th_pkt_trace_src_port_get(int unit);
extern int _bcm_th_pkt_trace_src_pipe_get(int unit);
extern int _bcm_th_pkt_trace_hw_reset(int unit);
extern int _bcm_th_pkt_trace_cpu_profile_init(int unit);
extern int _bcm_th_pkt_trace_cpu_profile_set(int unit,uint32 options);
extern int _bcm_th_pkt_trace_cpu_profile_get(int unit,uint32* profile_id);
extern int _bcm_th_pkt_trace_int_lbport_set(int unit, uint8 port, int enable, int *old_values);

/* end of packet trace feature support */
#endif /* BCM_INSTRUMENTATION_SUPPORT */

#endif
