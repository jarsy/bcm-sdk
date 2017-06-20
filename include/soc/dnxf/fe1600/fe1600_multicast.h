/*
 * $Id: ramon_fe1600_multicast.h,v 1.2 Broadcom SDK $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * RAMON_FE1600 MULTICAST H
 */
 
#ifndef _SOC_RAMON_FE1600_MULTICAST_H_
#define _SOC_RAMON_FE1600_MULTICAST_H_

#include <bcm/multicast.h>
#include <soc/dnxf/cmn/dnxf_defs.h>
#include <soc/error.h>

soc_error_t soc_ramon_fe1600_multicast_egress_add(int unit, bcm_multicast_t group, soc_gport_t port);
soc_error_t soc_ramon_fe1600_multicast_egress_delete(int unit, bcm_multicast_t group, soc_gport_t port);
soc_error_t soc_ramon_fe1600_multicast_egress_delete_all(int unit, bcm_multicast_t group);
soc_error_t soc_ramon_fe1600_multicast_egress_get(int unit, bcm_multicast_t group, int port_max, soc_gport_t *port_array, int *port_count);
soc_error_t soc_ramon_fe1600_multicast_egress_set(int unit, bcm_multicast_t group, int port_count, soc_gport_t *port_array);
soc_error_t soc_ramon_fe1600_multicast_table_size_get(int unit, uint32* mc_table_size);
soc_error_t soc_ramon_fe1600_multicast_mode_get(int unit, soc_dnxf_multicast_table_mode_t* multicast_mode);
soc_error_t soc_ramon_fe1600_multicast_table_entry_size_get(int unit, uint32 *entry_size);

#endif /*_SOC_RAMON_FE1600_MULTICAST_H_*/
