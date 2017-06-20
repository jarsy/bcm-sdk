/*
* $Id$
*
* $Copyright: (c) 2016 Broadcom.
* Broadcom Proprietary and Confidential. All rights reserved.$
*
* JER2_ARAD FABRIC CELL H
*/

#ifndef _SOC_JER2_ARAD_FABRIC_CELL_H_

#define _SOC_JER2_ARAD_FABRIC_CELL_H_

/* Relevant only for Flow Status Message cell */
#define JER2_ARAD_VSC256_CONTROL_CELL_FSM_DEST_PORT_START  0
#define JER2_ARAD_VSC256_CONTROL_CELL_FSM_DEST_PORT_LENGTH  8
#define JER2_ARAD_VSC256_CONTROL_CELL_FSM_SRC_QUEUE_NUM_START  16
#define JER2_ARAD_VSC256_CONTROL_CELL_FSM_SRC_QUEUE_NUM_LENGTH  20
#define JER2_ARAD_VSC256_CONTROL_CELL_FSM_FLOW_ID_START  36
#define JER2_ARAD_VSC256_CONTROL_CELL_FSM_FLOW_ID_LENGTH  20

/* Relevant only for Credit Message cell */
#define JER2_ARAD_VSC256_CONTROL_CELL_CREDIT_DEST_Q_NUM_START  16
#define JER2_ARAD_VSC256_CONTROL_CELL_CREDIT_DEST_Q_NUM_LENGTH  20
#define JER2_ARAD_VSC256_CONTROL_CELL_CREDIT_SUB_FLOW_ID_START  36
#define JER2_ARAD_VSC256_CONTROL_CELL_CREDIT_SUB_FLOW_ID_LENGTH  2
#define JER2_ARAD_VSC256_CONTROL_CELL_CREDIT_FLOW_ID_START  38
#define JER2_ARAD_VSC256_CONTROL_CELL_CREDIT_FLOW_ID_LENGTH  18

/* Relevant only for Reachability cell */
#define JER2_ARAD_VSC256_CONTROL_CELL_REACHABILITY_BITMAP_START  0
#define JER2_ARAD_VSC256_CONTROL_CELL_REACHABILITY_BITMAP_LENGTH  32
#define JER2_ARAD_VSC256_CONTROL_CELL_REACHABILITY_BASE_ID_START  64
#define JER2_ARAD_VSC256_CONTROL_CELL_REACHABILITY_BASE_ID_LENGTH  6
#define JER2_ARAD_VSC256_CONTROL_CELL_REACHABILITY_SRC_LINK_NUM_START  79
#define JER2_ARAD_VSC256_CONTROL_CELL_REACHABILITY_SRC_LINK_NUM_LENGTH  8

#define JER2_ARAD_VSC256_CONTROL_CELL_SOURCE_ID_START  85
#define JER2_ARAD_VSC256_CONTROL_CELL_SOURCE_ID_LENGTH  11
#define JER2_ARAD_VSC256_CONTROL_CELL_DEST_DEVICE_START  96
#define JER2_ARAD_VSC256_CONTROL_CELL_DEST_DEVICE_LENGTH  11
#define JER2_ARAD_VSC256_CONTROL_CELL_CONTROL_TYPE_START  107
#define JER2_ARAD_VSC256_CONTROL_CELL_CONTROL_TYPE_LENGTH  3
#define JER2_ARAD_VSC256_CONTROL_CELL_REACHABILITY_SOURCE_ID_START  93

#define SR_CELL_TYPE 01

#include <soc/dnxc/legacy/dnxc_defs.h>
#include <soc/dnxc/legacy/dnxc_fabric_cell.h>
#include <soc/dnx/legacy/dnx_fabric_cell.h>

soc_error_t soc_jer2_arad_cell_filter_set(int unit, uint32 array_size, soc_dnxc_filter_type_t* filter_type_arr, uint32* filter_type_val);
soc_error_t soc_jer2_arad_cell_filter_clear(int unit);
soc_error_t soc_jer2_arad_cell_filter_receive(int unit,  dnxc_captured_cell_t* data_out);
soc_error_t soc_jer2_arad_control_cell_filter_set(int unit, soc_dnxc_control_cell_types_t cell_type, uint32 array_size, soc_dnxc_control_cell_filter_type_t* filter_type_arr, uint32* filter_type_val);
soc_error_t soc_jer2_arad_control_cell_filter_clear(int unit);
soc_error_t soc_jer2_arad_control_cell_filter_receive(int unit,  soc_dnxc_captured_control_cell_t* data_out);

#endif /* _SOC_JER2_ARAD_FABRIC_CELL_H_ */
