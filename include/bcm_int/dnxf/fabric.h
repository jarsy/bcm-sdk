/*
 * $Id: fabric.h,v 1.2 Broadcom SDK $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * DNXF FABRIC H
 */
#ifndef _BCM_INT_DNXF_FABRIC_H_
#define _BCM_INT_DNXF_FABRIC_H_

#include <sal/types.h>
#include <bcm/types.h>

#include <bcm_int/dnxf/dnxf_fifo_type.h>
#include <bcm_int/dnxf/dnxf_multicast_id_map.h>

#include <soc/dnxc/legacy/fabric.h>

#include <soc/dnxf/cmn/dnxf_defs.h>


int _bcm_dnxf_fabric_deinit(int unit);
int _bcm_dnxf_fabric_link_flow_status_cell_format_set(int unit, bcm_port_t port, int cell_format);
int _bcm_dnxf_fabric_link_flow_status_cell_format_get(int unit, bcm_port_t port, int *cell_format);
int _bcm_dnxf_fabric_fifo_info_get(int unit, bcm_port_t port, bcm_fabric_pipe_t pipe_id, soc_dnxc_fabric_direction_t  direction, uint32 *fifo_type, bcm_fabric_pipe_t *pipe_get, uint32 *flags_get);
int _bcm_dnxf_fabric_link_repeater_update(int unit, bcm_port_t port);
  
#endif /*_BCM_INT_DNXF_FABRIC_H_*/
