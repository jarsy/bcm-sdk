/*
 * $Id: ramon_fabric_topology.h,v 1.4.132.1 Broadcom SDK $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * RAMON FABRIC TOPOLOGY H
 */
 
#ifndef _SOC_RAMON_FABRIC_TOPOLOGY_H_
#define _SOC_RAMON_FABRIC_TOPOLOGY_H_

#include <soc/dnxf/cmn/dnxf_defs.h>
#include <soc/dnxc/legacy/dnxc_defs.h>
#include <soc/error.h>
#include <soc/types.h>

soc_error_t soc_ramon_fabric_topology_reachability_mask_set(int unit, soc_pbmp_t active_links, soc_dnxc_isolation_status_t val);
soc_error_t soc_ramon_fabric_topology_nof_links_to_min_nof_links_default(int unit, int nof_links, int *min_nof_links);
soc_error_t soc_ramon_fabric_topology_min_nof_links_set(int unit, int min_nof_links);
soc_error_t soc_ramon_fabric_topology_min_nof_links_get(int unit, int *min_nof_links);
soc_error_t soc_ramon_fabric_topology_isolate_set(int unit, soc_dnxc_isolation_status_t val);
soc_error_t soc_ramon_fabric_topology_mesh_topology_reset(int unit);


#endif

