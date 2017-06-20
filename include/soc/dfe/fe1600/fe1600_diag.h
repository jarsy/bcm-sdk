/*
 * $Id: fe1600_diag.h,v 1.11 Broadcom SDK $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * FE1600 DEFS H
 */

#ifndef _SOC_FE1600_DIAG_H_
#define _SOC_FE1600_DIAG_H_
#include <soc/types.h>
#include <soc/dfe/cmn/dfe_diag.h>
#include <soc/error.h>

soc_error_t 
soc_fe1600_counters_get_info(int unit, soc_dfe_counters_info_t* fe_counters_info);

soc_error_t 
soc_fe1600_queues_get_info(int unit,soc_dfe_queues_info_t* fe_queues_info);

soc_error_t
soc_fe1600_diag_fabric_cell_snake_test_interrupts_name_get(int unit, const soc_dfe_diag_flag_str_t **intr_names);


#endif /*_SOC_FE1600_DIAG_H_*/

