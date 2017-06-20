/*
 * $Id: ramon_diag.h,v 1.2.12.2 Broadcom SDK $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * RAMON DEFS H
 */

#ifndef _SOC_RAMON_DIAG_H_
#define _SOC_RAMON_DIAG_H_

/**********************************************************/
/*                  Includes                              */
/**********************************************************/
#include <soc/dnxf/cmn/dnxf_drv.h>
#include <soc/dnxf/cmn/dnxf_diag.h>

/**********************************************************/
/*                  Structures                            */
/**********************************************************/

/**********************************************************/
/*                  Defines                               */
/**********************************************************/


/**********************************************************/
/*                  Constants                              */
/**********************************************************/


/**********************************************************/
/*                  Functions                             */
/**********************************************************/

soc_error_t soc_ramon_diag_fabric_cell_snake_test_interrupts_name_get(int unit, const soc_dnxf_diag_flag_str_t **intr_names);
soc_error_t soc_ramon_counters_get_info(int unit,soc_dnxf_counters_info_t* fe_counters_info);
soc_error_t soc_ramon_diag_cell_pipe_counter_get(int unit, int pipe, uint64 *counter);
soc_error_t soc_ramon_queues_get_info(int unit, soc_dnxf_queues_info_t* fe_queues_info);
soc_error_t soc_ramon_diag_mesh_topology_get(int unit, soc_dnxc_fabric_mesh_topology_diag_t *mesh_topology_diag);


#endif
