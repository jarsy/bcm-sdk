/*
* $Id: jer2_arad_fabric_cell.c,v 1.8 Broadcom SDK $
*
* $Copyright: (c) 2016 Broadcom.
* Broadcom Proprietary and Confidential. All rights reserved.$
*
* SOC JER2_ARAD FABRIC CELL
*/
#ifdef _ERR_MSG_MODULE_NAME
  #error "_ERR_MSG_MODULE_NAME redefined"
#endif

#define _ERR_MSG_MODULE_NAME BSL_SOC_FABRIC


#include <shared/bsl.h>
#include <soc/dnxc/legacy/error.h>
#include <soc/mcm/memregs.h>
#include <soc/error.h>
#include <shared/bitop.h>
#include <soc/dnx/legacy/ARAD/arad_fabric_cell.h>
#include <soc/dnx/legacy/drv.h>
#include <soc/dnx/legacy/ARAD/arad_multicast_fabric.h>
#include <soc/dnx/legacy/ARAD/arad_cell.h>
#include <soc/dnxc/legacy/dnxc_captured_buffer.h>
#include <soc/dnxc/legacy/vsc256_fabric_cell.h>
#include <soc/dnxc/legacy/dnxc_cells_buffer.h>
#include <soc/dnxc/legacy/dnxc_fabric_cell.h>

soc_error_t 
soc_jer2_arad_cell_filter_set(int unit, uint32 array_size, soc_dnxc_filter_type_t* filter_type_arr, uint32* filter_type_val) 
{
    int i, rc;
    soc_reg_above_64_val_t reg_val, mask_val;
    uint32 table_size;

    DNXC_INIT_FUNC_DEFS;
    SOC_REG_ABOVE_64_CLEAR(reg_val);
    SOC_REG_ABOVE_64_CLEAR(mask_val);
    
    for(i=0 ; i<array_size ; i++) {
        switch(filter_type_arr[i]) {
        case soc_dnxc_filter_type_source_id:
            SHR_BITCOPY_RANGE(reg_val, 33, &filter_type_val[i], 0, 11);
            SHR_BITSET_RANGE(reg_val, 33, 11);
            break;
        case soc_dnxc_filter_type_multicast_id:
            rc = jer2_arad_multicast_table_size_get(unit, &table_size);
            DNXC_IF_ERR_EXIT(rc);
            if(filter_type_val[i] >= table_size) {
                DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("multicast id out of range")));
            }
            SHR_BITCOPY_RANGE(reg_val, 44, &filter_type_val[i], 0, 19);
            --table_size;
            SHR_BITCOPY_RANGE(mask_val, 44, &table_size, 0, 19);
            ++table_size;
            break;
        case soc_dnxc_filter_type_priority:
            if((filter_type_val[i] & (~0x3)) != 0) {
                DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("the priority type is out of range")));
            }
            SHR_BITCOPY_RANGE(reg_val, 63, &filter_type_val[i], 0, 2);
            SHR_BITSET_RANGE(mask_val, 63, 2);
            break;
        default:
            DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("unsupported filter type")));
        }
    }

    DNXC_IF_ERR_EXIT(WRITE_FDR_PROGRAMMABLE_DATA_CELL_COUNTER_0r(unit, reg_val));
    DNXC_IF_ERR_EXIT(WRITE_FDR_PROGRAMMABLE_DATA_CELL_COUNTER_MASK_0r(unit, mask_val));

exit:
    DNXC_FUNC_RETURN;
}

soc_error_t 
soc_jer2_arad_cell_filter_clear(int unit)
{
    soc_reg_above_64_val_t cleared_reg;
    uint64 cleared_64;
    DNXC_INIT_FUNC_DEFS;

    SOC_REG_ABOVE_64_CLEAR(cleared_reg);
    COMPILER_64_ZERO(cleared_64);
    
    DNXC_IF_ERR_EXIT(WRITE_FDR_PROGRAMMABLE_DATA_CELL_COUNTER_0r(unit, cleared_reg));
    DNXC_IF_ERR_EXIT(WRITE_FDR_PROGRAMMABLE_DATA_CELL_COUNTER_1r(unit, cleared_64));
    DNXC_IF_ERR_EXIT(WRITE_FDR_PROGRAMMABLE_DATA_CELL_COUNTER_MASK_0r(unit, cleared_reg));
    DNXC_IF_ERR_EXIT(WRITE_FDR_PROGRAMMABLE_DATA_CELL_COUNTER_MASK_1r(unit, cleared_64));
exit:
    DNXC_FUNC_RETURN;
}

static soc_error_t
soc_jer2_arad_cpu2cpu_read(int unit)
{
    int rc = SOC_E_NONE;
    uint8 success;
    uint32 packed_cpu_data_cell_rcv[JER2_ARAD_DATA_CELL_RECEIVED_UINT32_SIZE] ;
    dnxc_vsc256_sr_cell_t cell;
    dnxc_captured_cell_t captured_cell;
    uint32 cell_type = 0;

    DNXC_INIT_FUNC_DEFS;

/*
 * COVERITY
 *
 * The variable packed_cpu_data_cell_rcv is assigned inside jer2_arad_cell_ack_get.
 */
/* coverity[uninit_use_in_call] */
    rc = jer2_arad_cell_ack_get(unit, FALSE, packed_cpu_data_cell_rcv, &success);
    DNXC_IF_ERR_EXIT(rc);
    SHR_BITCOPY_RANGE(&cell_type, 0, packed_cpu_data_cell_rcv, JER2_ARAD_PARSING_CELL_TYPE_START, JER2_ARAD_PARSING_CELL_TYPE_LENGTH);
    if(cell_type == SR_CELL_TYPE) {
        rc = soc_jer2_arad_parse_cell(unit, FALSE, packed_cpu_data_cell_rcv, &cell);
        DNXC_IF_ERR_EXIT(rc);
        rc = dnxc_cells_buffer_add(unit, &SOC_DNX_CONFIG(unit)->jer2_arad->sr_cells_buffer, &cell);
        DNXC_IF_ERR_EXIT(rc);
    } else {
        rc = soc_dnxc_parse_captured_cell(unit, packed_cpu_data_cell_rcv, &captured_cell);
        DNXC_IF_ERR_EXIT(rc);
        rc = dnxc_captured_buffer_add(unit, &SOC_DNX_CONFIG(unit)->jer2_arad->captured_cells_buffer, &captured_cell);
        DNXC_IF_ERR_EXIT(rc);
    }
exit:
    DNXC_FUNC_RETURN; 
}
soc_error_t 
soc_jer2_arad_cell_filter_receive(int unit,  dnxc_captured_cell_t* data_out)
{
    int rc = SOC_E_NONE, is_empty = TRUE;
    DNXC_INIT_FUNC_DEFS;

    rc = dnxc_captured_buffer_is_empty(unit, &SOC_DNX_CONFIG(unit)->jer2_arad->captured_cells_buffer, &is_empty);
    DNXC_IF_ERR_EXIT(rc);

    if(is_empty) {
        rc = soc_jer2_arad_cpu2cpu_read(unit);
        DNXC_IF_ERR_EXIT(rc);
    }

    rc = dnxc_captured_buffer_get(unit, &SOC_DNX_CONFIG(unit)->jer2_arad->captured_cells_buffer, data_out);
    DNXC_IF_ERR_EXIT(rc);

exit:
    DNXC_FUNC_RETURN; 
}

soc_error_t 
soc_jer2_arad_control_cell_filter_set(int unit, soc_dnxc_control_cell_types_t cell_type, uint32 array_size, soc_dnxc_control_cell_filter_type_t* filter_type_arr, uint32* filter_type_val) 
{
    int i = 0;
    soc_reg_above_64_val_t reg_val, mask_val;

    DNXC_INIT_FUNC_DEFS;

    SOC_REG_ABOVE_64_CLEAR(reg_val);
    /* mask of "1"s mean to ignore when using the filter */
    SOC_REG_ABOVE_64_ALLONES(mask_val);

    if ((cell_type == soc_dnxc_flow_status_cell) || (cell_type == soc_dnxc_credit_cell))
    {
        for(i=0 ; i<array_size ; i++) 
        {
            switch(filter_type_arr[i]) {
                case soc_dnxc_filter_control_cell_type_source_device:
                    if((filter_type_val[i] & (~0x7FF)) != 0) {
                        DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("source device id is out of range")));
                    }
                    SHR_BITCOPY_RANGE(reg_val, JER2_ARAD_VSC256_CONTROL_CELL_SOURCE_ID_START, &(filter_type_val[i]), 0, JER2_ARAD_VSC256_CONTROL_CELL_SOURCE_ID_LENGTH);
                    /* add mask for bits 103:93 in the cell*/
                    SHR_BITCLR_RANGE(mask_val, JER2_ARAD_VSC256_CONTROL_CELL_SOURCE_ID_START, JER2_ARAD_VSC256_CONTROL_CELL_SOURCE_ID_LENGTH);
                    break;
                case soc_dnxc_filter_control_cell_type_dest_device:
                    if((filter_type_val[i] & (~0x7FF)) != 0) {
                        DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("dest device id is out of range")));
                    }
                    SHR_BITCOPY_RANGE(reg_val, JER2_ARAD_VSC256_CONTROL_CELL_SOURCE_ID_START, &(filter_type_val[i]), 0, JER2_ARAD_VSC256_CONTROL_CELL_DEST_DEVICE_LENGTH);
                    /* add mask for bits 114:104 in the cell*/
                    SHR_BITCLR_RANGE(mask_val, JER2_ARAD_VSC256_CONTROL_CELL_SOURCE_ID_START, JER2_ARAD_VSC256_CONTROL_CELL_DEST_DEVICE_LENGTH);
                    break;
                default:
                    DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("unsupported filter type")));
            }
        }
    }
    /* The control cell type is reachability cell (no support for dest_device filter) */
    else
    {
        SHR_BITCOPY_RANGE(reg_val, JER2_ARAD_VSC256_CONTROL_CELL_REACHABILITY_SOURCE_ID_START, &(filter_type_val[i]), 0, JER2_ARAD_VSC256_CONTROL_CELL_SOURCE_ID_LENGTH);
        /* add mask for bits 111:101 in the cell*/
        SHR_BITCLR_RANGE(mask_val, JER2_ARAD_VSC256_CONTROL_CELL_REACHABILITY_SOURCE_ID_START, JER2_ARAD_VSC256_CONTROL_CELL_SOURCE_ID_LENGTH);
    }

    /* Add the control cell type to the filter */ 
    SHR_BITCOPY_RANGE(reg_val, JER2_ARAD_VSC256_CONTROL_CELL_CONTROL_TYPE_START, &cell_type, 0, JER2_ARAD_VSC256_CONTROL_CELL_CONTROL_TYPE_LENGTH);
    /* add mask for bits 117:115 in the cell*/
    SHR_BITCLR_RANGE(mask_val, JER2_ARAD_VSC256_CONTROL_CELL_CONTROL_TYPE_START, JER2_ARAD_VSC256_CONTROL_CELL_CONTROL_TYPE_LENGTH);

    /* Set the filter and mask registers*/
    DNXC_IF_ERR_EXIT(WRITE_FCR_PROGRAMMABLE_CONTROL_CELL_COUNTERr(unit, reg_val));
    DNXC_IF_ERR_EXIT(WRITE_FCR_PROGRAMMABLE_CONTROL_CELL_COUNTER_MASK_0r(unit, mask_val));
    
exit:
    DNXC_FUNC_RETURN;
}

soc_error_t 
soc_jer2_arad_control_cell_filter_clear(int unit)
{
    soc_reg_above_64_val_t reg_val;
    DNXC_INIT_FUNC_DEFS;

    SOC_REG_ABOVE_64_CLEAR(reg_val);
    DNXC_IF_ERR_EXIT(WRITE_FCR_PROGRAMMABLE_CONTROL_CELL_COUNTERr(unit, reg_val));
    DNXC_IF_ERR_EXIT(WRITE_FCR_PROGRAMMABLE_CONTROL_CELL_COUNTER_MASK_0r(unit, reg_val));
    
exit:
    DNXC_FUNC_RETURN;
}

soc_error_t 
soc_jer2_arad_control_cell_filter_receive(int unit,  soc_dnxc_captured_control_cell_t* data_out)
{
    soc_reg_above_64_val_t  reg_val;
    uint32 interrupt_reg;

    DNXC_INIT_FUNC_DEFS;

    DNXC_IF_ERR_EXIT(READ_FCR_INTERRUPT_REGISTERr(unit, &interrupt_reg));
    if(soc_reg_field_get(unit, FCR_INTERRUPT_REGISTERr, interrupt_reg, CPU_CNT_CELL_FNEf))
    {
        data_out->valid = TRUE;
    }

    if (data_out->valid) 
    {
        DNXC_IF_ERR_EXIT(READ_FCR_CONTROL_CELL_FIFO_BUFFERr(unit, reg_val));
        
        SHR_BITCOPY_RANGE((uint32*)&(data_out->control_type), 0, reg_val, JER2_ARAD_VSC256_CONTROL_CELL_CONTROL_TYPE_LENGTH, JER2_ARAD_VSC256_CONTROL_CELL_CONTROL_TYPE_LENGTH);

        switch(data_out->control_type) {
                case soc_dnxc_flow_status_cell:
                    SHR_BITCOPY_RANGE((uint32*)&(data_out->dest_port), 0, reg_val, JER2_ARAD_VSC256_CONTROL_CELL_FSM_DEST_PORT_START, JER2_ARAD_VSC256_CONTROL_CELL_FSM_DEST_PORT_LENGTH);
                    SHR_BITCOPY_RANGE((uint32*)&(data_out->src_queue_num), 0, reg_val, JER2_ARAD_VSC256_CONTROL_CELL_FSM_SRC_QUEUE_NUM_START, JER2_ARAD_VSC256_CONTROL_CELL_FSM_SRC_QUEUE_NUM_LENGTH);
                    SHR_BITCOPY_RANGE((uint32*)&(data_out->flow_id), 0, reg_val, JER2_ARAD_VSC256_CONTROL_CELL_FSM_FLOW_ID_START, JER2_ARAD_VSC256_CONTROL_CELL_FSM_FLOW_ID_LENGTH);
                    SHR_BITCOPY_RANGE((uint32*)&(data_out->source_device), 0, reg_val, JER2_ARAD_VSC256_CONTROL_CELL_SOURCE_ID_START, JER2_ARAD_VSC256_CONTROL_CELL_SOURCE_ID_LENGTH);
                    SHR_BITCOPY_RANGE((uint32*)&(data_out->dest_device), 0, reg_val, JER2_ARAD_VSC256_CONTROL_CELL_DEST_DEVICE_START, JER2_ARAD_VSC256_CONTROL_CELL_DEST_DEVICE_LENGTH);
                    break;
                case soc_dnxc_credit_cell:
                    SHR_BITCOPY_RANGE((uint32*)&(data_out->dest_queue_num),0 , reg_val, JER2_ARAD_VSC256_CONTROL_CELL_CREDIT_DEST_Q_NUM_START, JER2_ARAD_VSC256_CONTROL_CELL_CREDIT_DEST_Q_NUM_LENGTH);
                    SHR_BITCOPY_RANGE((uint32*)&(data_out->sub_flow_id), 0, reg_val, JER2_ARAD_VSC256_CONTROL_CELL_CREDIT_SUB_FLOW_ID_START, JER2_ARAD_VSC256_CONTROL_CELL_CREDIT_SUB_FLOW_ID_LENGTH);
                    SHR_BITCOPY_RANGE((uint32*)&(data_out->flow_id), 0, reg_val, JER2_ARAD_VSC256_CONTROL_CELL_CREDIT_FLOW_ID_START, JER2_ARAD_VSC256_CONTROL_CELL_CREDIT_FLOW_ID_LENGTH);
                    SHR_BITCOPY_RANGE((uint32*)&(data_out->source_device), 0, reg_val, JER2_ARAD_VSC256_CONTROL_CELL_SOURCE_ID_START, JER2_ARAD_VSC256_CONTROL_CELL_SOURCE_ID_LENGTH);
                    SHR_BITCOPY_RANGE((uint32*)&(data_out->dest_device), 0, reg_val, JER2_ARAD_VSC256_CONTROL_CELL_DEST_DEVICE_START, JER2_ARAD_VSC256_CONTROL_CELL_DEST_DEVICE_LENGTH);
                    break;
                case soc_dnxc_reachability_cell:
                    SHR_BITCOPY_RANGE((uint32*)&(data_out->reachability_bitmap),0 , reg_val, JER2_ARAD_VSC256_CONTROL_CELL_REACHABILITY_BITMAP_START, JER2_ARAD_VSC256_CONTROL_CELL_REACHABILITY_BITMAP_LENGTH);
                    SHR_BITCOPY_RANGE((uint32*)&(data_out->base_index),0 , reg_val, JER2_ARAD_VSC256_CONTROL_CELL_REACHABILITY_BASE_ID_START, JER2_ARAD_VSC256_CONTROL_CELL_REACHABILITY_BASE_ID_LENGTH);
                    SHR_BITCOPY_RANGE((uint32*)&(data_out->source_link_number),0 , reg_val, JER2_ARAD_VSC256_CONTROL_CELL_REACHABILITY_SRC_LINK_NUM_START, JER2_ARAD_VSC256_CONTROL_CELL_REACHABILITY_SRC_LINK_NUM_LENGTH);
                    SHR_BITCOPY_RANGE((uint32*)&(data_out->source_device),0 , reg_val, JER2_ARAD_VSC256_CONTROL_CELL_REACHABILITY_SOURCE_ID_START, JER2_ARAD_VSC256_CONTROL_CELL_SOURCE_ID_LENGTH);
                    break;
                default:
                    DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("unsupported filter type")));
        }

    }

exit:
    DNXC_FUNC_RETURN; 
}
