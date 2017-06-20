/*
* $Id$
* $Copyright: (c) 2016 Broadcom.
* Broadcom Proprietary and Confidential. All rights reserved.$
*
* This file contains structures and functions declarations for 
* In-band cell configuration and Source Routed Cell.
* 
*/
#ifndef _SOC_DNX_FABRIC_CELL_H
#define _SOC_DNX_FABRIC_CELL_H

#include <soc/dnxc/legacy/dnxc_defs.h>
#include <soc/dnxc/legacy/dnxc_fabric_cell.h>

int soc_dnx_cell_filter_set(int unit, uint32 array_size, soc_dnxc_filter_type_t* filter_type_arr, uint32* filter_type_val); 
int soc_dnx_cell_filter_clear(int unit);
int soc_dnx_cell_filter_receive(int unit, dnxc_captured_cell_t* data_out);
int soc_dnx_control_cell_filter_set(int unit, soc_dnxc_control_cell_types_t cell_type, uint32 array_size, soc_dnxc_control_cell_filter_type_t* control_cell_filter_type_arr, uint32* filter_type_val); 
int soc_dnx_control_cell_filter_clear(int unit);
int soc_dnx_control_cell_filter_receive(int unit, soc_dnxc_captured_control_cell_t* data_out);

#endif /* _SOC_DNX_FABRIC_CELL_H */
