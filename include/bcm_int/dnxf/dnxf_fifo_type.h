/*
 * $Id: dnxf_fifo_type.h,v 1.3 Broadcom SDK $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * DNXF FIFO TYPES H
 */

#ifndef _BCM_DNXF_FIFO_TYPE_H_
#define _BCM_DNXF_FIFO_TYPE_H_

#include <sal/types.h>

#include <soc/dnxf/cmn/dnxf_drv.h>

typedef soc_dnxf_fifo_type_handle_t _bcm_dnxf_fifo_type_handle_t;

int bcm_dnxf_fifo_type_clear(int unit, _bcm_dnxf_fifo_type_handle_t *h);

int bcm_dnxf_fifo_type_is_rx(int unit, _bcm_dnxf_fifo_type_handle_t h, int* is_rx);
int bcm_dnxf_fifo_type_is_tx(int unit, _bcm_dnxf_fifo_type_handle_t h, int* is_tx);
int bcm_dnxf_fifo_type_is_fe1(int unit, _bcm_dnxf_fifo_type_handle_t h, int* is_fe1);
int bcm_dnxf_fifo_type_is_fe3(int unit, _bcm_dnxf_fifo_type_handle_t h, int* is_fe3);
int bcm_dnxf_fifo_type_is_primary(int unit, _bcm_dnxf_fifo_type_handle_t h, int* is_primary);
int bcm_dnxf_fifo_type_is_secondary(int unit, _bcm_dnxf_fifo_type_handle_t h, int* is_secondary);

int bcm_dnxf_fifo_type_set(int unit, _bcm_dnxf_fifo_type_handle_t* h, int is_rx, int is_tx, int is_fe1, int is_fe3, int is_primary, int is_secondary);

int bcm_dnxf_fifo_type_get_id(int unit, _bcm_dnxf_fifo_type_handle_t h, int* id);
int bcm_dnxf_fifo_type_set_id(int unit, _bcm_dnxf_fifo_type_handle_t* h, int fifo_id);

int bcm_dnxf_fifo_type_set_handle_flag(int unit, _bcm_dnxf_fifo_type_handle_t* h);
int bcm_dnxf_fifo_type_get_handle_flag(int unit, _bcm_dnxf_fifo_type_handle_t h, int *is_handle_flag);

int bcm_dnxf_fifo_type_is_overlap(int unit, _bcm_dnxf_fifo_type_handle_t h1, _bcm_dnxf_fifo_type_handle_t  h2, int* is_overlap);
int bcm_dnxf_fifo_type_add(int unit, soc_dnxf_fabric_link_fifo_type_index_t fifo_type, _bcm_dnxf_fifo_type_handle_t  h2);
int bcm_dnxf_fifo_type_sub(int unit, soc_dnxf_fabric_link_fifo_type_index_t fifo_type, _bcm_dnxf_fifo_type_handle_t  h2);

#endif /*_BCM_DNXF_FIFO_TYPE_H_*/
