/*
 * $Id: ramon_fe1600_fabric_links.h,v 1.9 Broadcom SDK $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * RAMON_FE1600 FABRIC LINKS H
 */
 
#ifndef _SOC_RAMON_FE1600_FABRIC_LINKS_H_
#define _SOC_RAMON_FE1600_FABRIC_LINKS_H_

#include <soc/error.h>

#include <soc/dnxc/legacy/dnxc_defs.h>
#include <soc/dnxc/legacy/fabric.h>

#include <soc/dnxf/cmn/dnxf_defs.h>
#include <soc/dnxf/cmn/dnxf_fabric.h>


soc_error_t soc_ramon_fe1600_fabric_links_nof_links_get (int unit, int* nof_links);
soc_error_t soc_ramon_fe1600_fabric_links_validate_link(int unit, soc_port_t link);
soc_error_t soc_ramon_fe1600_fabric_links_isolate_set(int unit, soc_port_t link, soc_dnxc_isolation_status_t val);
soc_error_t soc_ramon_fe1600_fabric_links_bmp_isolate_set(int unit, soc_pbmp_t link, soc_dnxc_isolation_status_t val);
soc_error_t soc_ramon_fe1600_fabric_links_isolate_get(int unit, soc_port_t link, soc_dnxc_isolation_status_t* val);
soc_error_t soc_ramon_fe1600_fabric_links_cell_format_verify(int unit, soc_port_t link, soc_dnxf_fabric_link_cell_size_t val);
soc_error_t soc_ramon_fe1600_fabric_links_cell_format_set(int unit, soc_port_t link, soc_dnxf_fabric_link_cell_size_t val);
soc_error_t soc_ramon_fe1600_fabric_links_cell_format_get(int unit, soc_port_t link, soc_dnxf_fabric_link_cell_size_t* val);
soc_error_t soc_ramon_fe1600_fabric_links_cell_interleaving_set(int unit, soc_port_t link, int val);
soc_error_t soc_ramon_fe1600_fabric_links_cell_interleaving_get(int unit, soc_port_t link, int* val);
soc_error_t soc_ramon_fe1600_fabric_links_weight_validate(int unit, int val);
soc_error_t soc_ramon_fe1600_fabric_links_weight_set(int unit, soc_port_t link, int is_prim, int val);
soc_error_t soc_ramon_fe1600_fabric_links_weight_get(int unit, soc_port_t link, int is_prim, int* val);
soc_error_t soc_ramon_fe1600_fabric_links_secondary_only_set(int unit, soc_port_t link, int val);
soc_error_t soc_ramon_fe1600_fabric_links_secondary_only_get(int unit, soc_port_t link, int* val);
soc_error_t soc_ramon_fe1600_fabric_links_llf_control_source_set(int unit, soc_port_t link, soc_dnxc_fabric_pipe_t val);
soc_error_t soc_ramon_fe1600_fabric_links_llf_control_source_get(int unit, soc_port_t link, soc_dnxc_fabric_pipe_t* val);
soc_error_t soc_ramon_fe1600_fabric_links_aldwp_config(int unit, soc_port_t port);
soc_error_t soc_ramon_fe1600_fabric_links_aldwp_init(int unit);
soc_error_t soc_ramon_fe1600_fabric_link_device_mode_get(int unit,soc_port_t port, int is_rx, soc_dnxf_fabric_link_device_mode_t *device_mode);
soc_error_t soc_ramon_fe1600_fabric_links_pcp_enable_get(int unit, soc_port_t port, int *enable);
soc_error_t soc_ramon_fe1600_fabric_link_repeater_enable_get(int unit, soc_port_t port, int *enable, int *empty_cell_size);

#endif /*_SOC_RAMON_FE1600_FABRIC_LINKS_H_*/
