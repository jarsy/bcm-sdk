/*
 * $Id: ramon_fe1600_fabric_status.h,v 1.3 Broadcom SDK $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * RAMON_FE1600 FABRIC STATUS H
 */
 
#ifndef _SOC_RAMON_FE1600_FABRIC_STATUS_H_
#define _SOC_RAMON_FE1600_FABRIC_STATUS_H_

#include <soc/dnxc/legacy/fabric.h>
#include <soc/dnxf/cmn/dnxf_defs.h>
#include <soc/error.h>

soc_error_t soc_ramon_fe1600_fabric_link_status_all_get(int unit, int links_array_max_size, uint32* link_status, uint32* errored_token_count, int* links_array_count);
soc_error_t soc_ramon_fe1600_fabric_link_status_get(int unit, soc_port_t link_id, uint32 *link_status, uint32 *errored_token_count);
soc_error_t soc_ramon_fe1600_fabric_link_status_clear(int unit, soc_port_t link);
soc_error_t soc_ramon_fe1600_fabric_reachability_status_get(int unit, int moduleid, int links_max, uint32 *links_array, int *links_count);
soc_error_t soc_ramon_fe1600_fabric_link_connectivity_status_get(int unit, soc_port_t link_id, bcm_fabric_link_connectivity_t *link_partner);

#endif /*_SOC_RAMON_FE1600_FABRIC_STATUS_H_*/
