/* 
 * $Id: cosq.h Exp $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:        cosq.h
 * Purpose:     COSQ internal definitions to the BCM library.
 */

#ifndef _BCM_INT_SBX_CALADAN3_COSQ_H_
#define _BCM_INT_SBX_CALADAN3_COSQ_H_

#include <bcm/types.h>
#include <bcm/cosq.h>

typedef struct {
    pbmp_t sq_bmp; /* 128 */
    pbmp_t dq_bmp; /* 128 */
} bcm_c3_cosq_port_queues_t;

int bcm_caladan3_cosq_init(int unit);

int bcm_caladan3_cosq_gport_add(int unit, bcm_gport_t physical_port,
                                int num_cos_levels, uint32 flags, bcm_gport_t *req_gport);

int bcm_caladan3_cosq_gport_get(int unit, bcm_gport_t gport, bcm_gport_t *physical_port,
                                int *num_cos_levels, uint32 *flags);

int bcm_caladan3_cosq_gport_delete(int unit, bcm_gport_t gport);

int bcm_caladan3_cosq_gport_queue_attach(int unit, uint32 flags, bcm_gport_t ingress_queue, 
                                         bcm_cos_t ingress_int_pri, bcm_gport_t egress_queue, 
                                         bcm_cos_t egress_int_pri, int *attach_id);

int bcm_caladan3_cosq_gport_queue_attach_get(int unit, bcm_gport_t ingress_queue, bcm_cos_t ingress_int_pri, 
                                             bcm_gport_t *egress_queue, bcm_cos_t *egress_int_pri, 
                                             int attach_id);

int bcm_caladan3_cosq_gport_queue_detach(int unit, bcm_gport_t ingress_queue, bcm_cos_t ingress_int_pri, 
                                         int attach_id);

int bcm_c3_cosq_queue_delete(int unit, int queue);

int bcm_c3_cosq_info_dump(int unit);

int bcm_c3_cosq_queues_from_port_get(int unit, bcm_port_t port, bcm_c3_cosq_port_queues_t *port_queues);

int bcm_c3_cosq_dest_port_from_sq_get(int unit, int sq_id, bcm_port_t *dest_port, int *dest_dq_id);

int bcm_c3_cosq_src_port_from_dq_get(int unit, int dq_id, bcm_port_t *src_port, int *src_dq_id);

int bcm_caladan3_cosq_src_queue_set(int unit, int queue, uint32 bytes_min, uint32 bytes_max);

int bcm_caladan3_cosq_src_queue_get(int unit, int queue, uint32* bytes_min, uint32* bytes_max);

#endif /* _BCM_INT_SBX_CALADAN3_COSQ_H_ */
