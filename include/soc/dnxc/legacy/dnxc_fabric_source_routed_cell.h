/*
 * $Id$
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */

#ifndef SOC_DNXC_FABRIC_SOURCE_ROUTED_CELL_H
#define SOC_DNXC_FABRIC_SOURCE_ROUTED_CELL_H

#include <soc/dnxc/legacy/dnxc_defs.h>
#include <soc/error.h>
#include <soc/dnxc/legacy/fabric.h>

#define DNXC_CELL_NOF_LINKS_IN_PATH_LINKS            (4)

typedef struct dnxc_sr_cell_link_list_s{
  dnxc_fabric_device_type_t  src_entity_type;
  dnxc_fabric_device_type_t  dest_entity_type;
  /* the links of the path: up to four links */
  soc_port_t                path_links[DNXC_CELL_NOF_LINKS_IN_PATH_LINKS];
  int                       pipe_id;
}dnxc_sr_cell_link_list_t;


soc_error_t
soc_dnxc_actual_entity_value(
                            int unit,
                            dnxc_fabric_device_type_t            device_entity,
                            soc_dnxc_device_type_actual_value_t* actual_entity
                            );

soc_error_t
soc_dnxc_device_entity_value(
                            int unit,
                            soc_dnxc_device_type_actual_value_t actual_entity,
                            dnxc_fabric_device_type_t*          device_entity
                            );

#endif /*SOC_DNXC_FABRIC_SOURCE_ROUTED_CELL_H*/
