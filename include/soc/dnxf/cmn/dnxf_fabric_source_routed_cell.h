/*
 * $Id: dnxf_fabric_source_routed_cell.h,v 1.4 Broadcom SDK $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * DNXF FABRIC SOURCE ROUTED CELL H
 */
 
#ifndef _SOC_DNXF_FABRIC_SOURCE_ROUTED_CELL_H_
#define _SOC_DNXF_FABRIC_SOURCE_ROUTED_CELL_H_

/*************
 * INCLUDES *
 *************/
#include <bcm/fabric.h>
#include <soc/error.h>
#include <soc/dnxf/cmn/dnxf_defs.h>
#include <soc/dnxc/legacy/vsc256_fabric_cell.h>
#include <soc/dnxc/legacy/fabric.h>
#include <soc/dnxc/legacy/dnxc_fabric_source_routed_cell.h>

/*************
 * DEFINES   *
 *************/

#define DNXF_CELL_NOF_BYTES_IN_UINT32                (4)
#define DNXF_CELL_NOF_DATA_WORDS_IN_INDIRECT_CELL    (3)


/*************
 * TYPE DEFS *
 *************/


/*************
 * FUNCTIONS *
 *************/

soc_error_t soc_dnxf_cpu2cpu_write(int unit, const dnxc_sr_cell_link_list_t  *sr_link_list, uint32 data_in_size, uint32 *data_in); 
soc_error_t soc_dnxf_route2sr_link_list(int unit, const soc_dnxc_fabric_route_t *route, dnxc_sr_cell_link_list_t *sr_link_list);

soc_error_t soc_dnxf_sr_cell_format_type_get(int unit,  const dnxc_vsc256_sr_cell_t* cell, soc_dnxf_fabric_link_cell_size_t* vsc_format);
soc_error_t soc_dnxf_sr_cell_receive(int unit, dnxc_vsc256_sr_cell_t* cell);
soc_error_t soc_dnxf_sr_cell_payload_receive(int unit, uint32 flags, uint32 data_out_max_size, uint32 *data_out_size, uint32 *data_out);
soc_error_t soc_dnxf_sr_cell_send(int unit, uint32 flags, soc_dnxc_fabric_route_t* route, uint32 data_in_size, uint32 *data_in);


#endif /*_SOC_DNXF_FABRIC_SOURCE_ROUTED_CELL_H_*/
