
/*
 * $Id: dfe_fabric_cell.c,v 1.14 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * SOC DFE FABRIC CELL
 */

#ifdef _ERR_MSG_MODULE_NAME
  #error "_ERR_MSG_MODULE_NAME redefined"
#endif

#define _ERR_MSG_MODULE_NAME BSL_SOC_FABRIC
#include <shared/bsl.h>
#include <soc/dcmn/error.h>

#include <soc/dfe/cmn/dfe_drv.h>
#include <soc/dfe/cmn/dfe_defs.h>
#include <soc/dfe/cmn/dfe_fabric_cell.h>
#include <soc/dfe/cmn/dfe_fabric_source_routed_cell.h>
#include <soc/dfe/cmn/mbcm.h>
#include <bcm_int/control.h>
#include <soc/dcmn/dcmn_fabric_cell.h>
#include <soc/dcmn/dcmn_cells_buffer.h>
#include <soc/dcmn/dcmn_captured_buffer.h>

/* Static functions */
void clear_data_out_strcute (soc_dcmn_captured_control_cell_t* data_out)
{
    data_out->dest_device = -1;
    data_out->source_device = -1;
    data_out->dest_port = -1;
    data_out->src_queue_num = -1;
    data_out->dest_queue_num = -1;
    data_out->sub_flow_id = -1;
    data_out->flow_id = -1;
    data_out->reachability_bitmap = -1;
    data_out->base_index = -1;
    data_out->source_link_number = -1;
}

int 
soc_dfe_cell_filter_set(int unit, uint32 flags, uint32 array_size, soc_dcmn_filter_type_t* filter_type_arr, uint32* filter_type_val) 
{
    int rc;
    SOCDNX_INIT_FUNC_DEFS;

    if (!SOC_UNIT_VALID(unit)) {
        SOCDNX_EXIT_WITH_ERR(SOC_E_UNIT, (_BSL_SOCDNX_MSG("Invalid unit")));
    }

    SOCDNX_NULL_CHECK(filter_type_arr);
    SOCDNX_NULL_CHECK(filter_type_val);

    DFE_UNIT_LOCK_TAKE_SOCDNX(unit);
    rc = MBCM_DFE_DRIVER_CALL(unit,mbcm_dfe_cell_filter_set,(unit, flags, array_size, filter_type_arr, filter_type_val));
    SOCDNX_IF_ERR_EXIT(rc);

exit:
    DFE_UNIT_LOCK_RELEASE_SOCDNX(unit);
    SOCDNX_FUNC_RETURN;
}

int 
soc_dfe_cell_filter_count_get(int unit, int *count)
{
    int rc;
    SOCDNX_INIT_FUNC_DEFS;

    if (!SOC_UNIT_VALID(unit)) {
        SOCDNX_EXIT_WITH_ERR(SOC_E_UNIT, (_BSL_SOCDNX_MSG("Invalid unit")));
    }

    SOCDNX_NULL_CHECK(count);

    DFE_UNIT_LOCK_TAKE_SOCDNX(unit);
    rc = MBCM_DFE_DRIVER_CALL(unit,mbcm_dfe_cell_filter_count_get,(unit, count));
    SOCDNX_IF_ERR_EXIT(rc);

exit:
    DFE_UNIT_LOCK_RELEASE_SOCDNX(unit);
    SOCDNX_FUNC_RETURN;

}

int 
soc_dfe_cell_filter_clear(int unit)
{
    int rc;
    SOCDNX_INIT_FUNC_DEFS;

    if (!SOC_UNIT_VALID(unit)) {
        SOCDNX_EXIT_WITH_ERR(SOC_E_UNIT, (_BSL_SOCDNX_MSG("Invalid unit")));
    }

    DFE_UNIT_LOCK_TAKE_SOCDNX(unit);

    rc = MBCM_DFE_DRIVER_CALL_NO_ARGS(unit, mbcm_dfe_cell_filter_clear);
    SOCDNX_IF_ERR_EXIT(rc);

exit:
    DFE_UNIT_LOCK_RELEASE_SOCDNX(unit);
    SOCDNX_FUNC_RETURN;
}

int 
soc_dfe_cell_filter_receive(int unit, dcmn_captured_cell_t* data_out)
{

    int rv, is_empty;
    SOCDNX_INIT_FUNC_DEFS;

    if (!SOC_UNIT_VALID(unit)) {
        SOCDNX_EXIT_WITH_ERR(SOC_E_UNIT, (_BSL_SOCDNX_MSG("Invalid unit")));
    }

    SOCDNX_NULL_CHECK(data_out);
    DFE_UNIT_LOCK_TAKE_SOCDNX(unit);

    /*Check if cell exist in data base */
    rv = dcmn_captured_buffer_is_empty(unit, &SOC_DFE_CONTROL(unit)->captured_cells_buffer, &is_empty);
    SOCDNX_IF_ERR_EXIT(rv);

    /*Load cells to data base */
    /*Stop loading if captured cell loaded or buffer is empty*/
    while (is_empty) 
    {
        rv = soc_dfe_fabric_cell_load(unit);
        if (rv == SOC_E_EMPTY)
        {
            break;
        }
        SOCDNX_IF_ERR_EXIT(rv);

        rv = dcmn_captured_buffer_is_empty(unit, &SOC_DFE_CONTROL(unit)->captured_cells_buffer, &is_empty);
        SOCDNX_IF_ERR_EXIT(rv);
    }
    if (is_empty)
    {
        SOCDNX_EXIT_WITH_ERR_NO_MSG(SOC_E_EMPTY);
    }

    /*Pull cell from data base*/
    rv = dcmn_captured_buffer_get(unit, &SOC_DFE_CONTROL(unit)->captured_cells_buffer, data_out);
    SOCDNX_IF_ERR_EXIT(rv);


exit:
    DFE_UNIT_LOCK_RELEASE_SOCDNX(unit);
    SOCDNX_FUNC_RETURN;
}

/*
 * Function:
 *      soc_dfe_control_cell_filter_set
 * Purpose:
 *      Set filter in order to capture a control cell (credit, flow status, reachability)
 * Parameters:
 *      unit                            - (IN)  Unit number
 *      cell_type                       - (IN)  Control cell type (credit, flow status or reachability cell
 *      array_size                      - (IN)  Number of filters
 *      control_cell_filter_type_arr    - (IN)  The filters array
 *      filter_type_val                 - (OUT) The filter that we should us to capture the control cell
 *  
 * Returns:
 *      SOC_E_xxx
 */
int soc_dfe_control_cell_filter_set(int unit, uint32 flags, soc_dcmn_control_cell_types_t cell_type, uint32 array_size, soc_dcmn_control_cell_filter_type_t* control_cell_filter_type_arr, uint32* filter_type_val) 
{
    int rc;
    SOCDNX_INIT_FUNC_DEFS;

    if (!SOC_UNIT_VALID(unit)) {
        SOCDNX_EXIT_WITH_ERR(SOC_E_UNIT, (_BSL_SOCDNX_MSG("Invalid unit")));
    }
    if (array_size != 0) {
        SOCDNX_NULL_CHECK(control_cell_filter_type_arr);
        SOCDNX_NULL_CHECK(filter_type_val);
    }

    DFE_UNIT_LOCK_TAKE_SOCDNX(unit);
    rc = MBCM_DFE_DRIVER_CALL(unit,mbcm_dfe_control_cell_filter_set,(unit, flags, cell_type, array_size, control_cell_filter_type_arr, filter_type_val));
    SOCDNX_IF_ERR_EXIT(rc);

exit:
    DFE_UNIT_LOCK_RELEASE_SOCDNX(unit);
    SOCDNX_FUNC_RETURN;
}

/*
 * Function:
 *      soc_dfe_control_cell_filter_clear
 * Purpose:
 *      Clear the control cell of filter and mask
 * Parameters:
 *      unit                            - (IN)  Unit number
 *  
 * Returns:
 *      SOC_E_xxx
 */
int 
soc_dfe_control_cell_filter_clear(int unit)
{
    int rc;
    SOCDNX_INIT_FUNC_DEFS;

    if (!SOC_UNIT_VALID(unit))
    {
        SOCDNX_EXIT_WITH_ERR(SOC_E_UNIT, (_BSL_SOCDNX_MSG("Invalid unit")));
    }

    DFE_UNIT_LOCK_TAKE_SOCDNX(unit);
    rc = MBCM_DFE_DRIVER_CALL_NO_ARGS(unit, mbcm_dfe_control_cell_filter_clear);
    SOCDNX_IF_ERR_EXIT(rc);

exit:
    DFE_UNIT_LOCK_RELEASE_SOCDNX(unit);
    SOCDNX_FUNC_RETURN;
}

/*
 * Function:
 *      soc_dfe_control_cell_filter_receive
 * Purpose:
 *      Recieve the content of the capture control cell. The capture cell can be one of the following:
 *      1. Credit
 *      2. FSM (Flow Status Message)
 *      3. Reachability cell
 *      4. FE2 filtered cell (Required fo FE2 normal operation.
 *  
 * Parameters:
 *      unit                            - (IN)  Unit number
 *      data_out                       - (OUT) The parsing of the control cell that match the filter
 *  
 * Comments: 
 *      # The data_out strcture includes valid bit that indicate if we were able to capture a cell
 *      For each control cell different fields are relevant:
 *      1. Credit - dest_queue_num, sub_flow_id, flow_id, source_device, dest_device
 *      2. FSM - dest_port, src_queue_num, flow_id, source_device, dest_device
 *      3. Reachability cell - reachability_bitmap, base_index, source_link_number, source_device
 *      4. FE2 filtered cell - cell type only.
 * Returns:
 *      SOC_E_xxx
 */

int 
soc_dfe_control_cell_filter_receive(int unit, soc_dcmn_captured_control_cell_t* data_out)
{
    int rc;
    SOCDNX_INIT_FUNC_DEFS;

    if (!SOC_UNIT_VALID(unit)) {
        SOCDNX_EXIT_WITH_ERR(SOC_E_UNIT, (_BSL_SOCDNX_MSG("Invalid unit")));
    }

    SOCDNX_NULL_CHECK(data_out);

    /* Clear the strcure, if the output is "-1" this field is not avaialble on this specific control cell type */
    clear_data_out_strcute(data_out);

    DFE_UNIT_LOCK_TAKE_SOCDNX(unit);
    rc = MBCM_DFE_DRIVER_CALL(unit,mbcm_dfe_control_cell_filter_receive,(unit, data_out));
    SOCDNX_IF_ERR_EXIT(rc);


exit:
    DFE_UNIT_LOCK_RELEASE_SOCDNX(unit);
    SOCDNX_FUNC_RETURN;

}


/*
 * Function:
 *      soc_dfe_fabric_cell_load
 * Purpose:
 *      Loading source routed cell from cpu buffer.
 *      Use FIFO DMA if supported.
 *      The existed cells pushed to data base
 *      
 * Parameters:
 *      unit                            - (IN)  Unit number
 * Returns:
 *      SOC_E_xxx
 */
soc_error_t 
soc_dfe_fabric_cell_load(int unit)
{
    int rv;
    int is_two_parts = 0;
    soc_dcmn_fabric_cell_entry_t entry;
    soc_dcmn_fabric_cell_entry_t entry_2;
    soc_dcmn_fabric_cell_info_t cell_info;
    SOCDNX_INIT_FUNC_DEFS;

    sal_memset(&cell_info, 0x00, sizeof(soc_dcmn_fabric_cell_info_t));
    
    /* Get cell entry */
    rv = soc_dfe_fabric_cell_entry_get(unit, entry);
    if (rv == SOC_E_EMPTY)
    {
        SOCDNX_EXIT_WITH_ERR_NO_MSG(SOC_E_EMPTY);
    }
    SOCDNX_IF_ERR_EXIT(rv);
    
    /* Check if cell is in 2 parts */
    SOCDNX_IF_ERR_EXIT(MBCM_DFE_DRIVER_CALL(unit, mbcm_dfe_fabric_cell_is_cell_in_two_parts, (unit, entry, &is_two_parts)));
    if (is_two_parts)
    {
        /* Get second cell */
        rv =  soc_dfe_fabric_cell_entry_get(unit, entry_2);
        SOCDNX_IF_ERR_EXIT(rv);  
    }
    /*
     * Parse cell
     */
    rv = soc_dfe_fabric_cell_parse(unit, entry, entry_2, &cell_info, is_two_parts);
    SOCDNX_IF_ERR_EXIT(rv);

    /*
     * Add to data base 
     */
    if(cell_info.cell_type == soc_dcmn_fabric_cell_type_sr_cell) 
    {
        rv = dcmn_cells_buffer_add(unit, &SOC_DFE_CONTROL(unit)->sr_cells_buffer, &cell_info.cell.sr_cell);
        SOCDNX_IF_ERR_EXIT(rv);
    } 
    else 
    {
        rv = dcmn_captured_buffer_add(unit, &SOC_DFE_CONTROL(unit)->captured_cells_buffer, &cell_info.cell.captured_cell);
        SOCDNX_IF_ERR_EXIT(rv);
    }

exit:
    SOCDNX_FUNC_RETURN; 
}

soc_error_t
soc_dfe_fabric_cell_entry_get(int unit, soc_dcmn_fabric_cell_entry_t entry)
{
    int rv;
    int channel;
    int found_cell = 0;
    SOCDNX_INIT_FUNC_DEFS;

    /*Get cell using FIFO DMA support*/
    if (SOC_DFE_CONFIG(unit).fabric_cell_fifo_dma_enable != SOC_DFE_PROPERTY_UNAVAIL && SOC_DFE_CONFIG(unit).fabric_cell_fifo_dma_enable)
    {
        for (channel = SOC_DFE_DEFS_GET(unit, fifo_dma_fabric_cell_first_channel);
              channel < SOC_DFE_DEFS_GET(unit, fifo_dma_nof_fabric_cell_channels);
              channel++)
        {
            rv = soc_dfe_fifo_dma_channel_entry_get(unit, channel, SOC_DCMN_FABRIC_CELL_ENTRY_MAX_SIZE_UINT32 * sizeof(uint32), 
                                                    SOC_DFE_DEFS_GET(unit, fifo_dma_fabric_cell_nof_entries_per_cell), (uint8 *) entry);
            if (rv == SOC_E_EMPTY)
            {
                /*check next channel*/
                continue;
            }
            SOCDNX_IF_ERR_EXIT(rv);

            found_cell = 1;
            break;
        }
    } else {
        /*Get cell directly*/
        rv = MBCM_DFE_DRIVER_CALL(unit,mbcm_dfe_fabric_cell_get,(unit, entry));
        if (rv == SOC_E_EMPTY)
        {
            SOCDNX_EXIT_WITH_ERR_NO_MSG(SOC_E_EMPTY);
        }
        SOCDNX_IF_ERR_EXIT(rv);
        found_cell = 1;
    }
    if (!found_cell)
    {
        SOCDNX_EXIT_WITH_ERR_NO_MSG(SOC_E_EMPTY);
    }

exit:
    SOCDNX_FUNC_RETURN;
}

/*
 * Function:
 *      soc_dfe_fabric_cell_parse
 * Purpose:
 *      parse cpu cell
 * Parameters:
 *      unit            - (IN)  Unit number.
 *      entry           - (IN)  Received data
 *      cell_info       - (OUT) The parse cell 
 * Returns:
 *      SOC_E_XXX
 */
soc_error_t
soc_dfe_fabric_cell_parse(int unit, soc_dcmn_fabric_cell_entry_t entry, soc_dcmn_fabric_cell_entry_t entry_2, soc_dcmn_fabric_cell_info_t *cell_info, int is_two_parts)
{
    int rv;
    uint32 nof_lines;
    soc_dcmn_fabric_cell_parse_table_t parse_table[SOC_DCMN_FABRIC_CELL_PARSE_TABLE_MAX_NOF_LINES];
    SOCDNX_INIT_FUNC_DEFS;

    rv = MBCM_DFE_DRIVER_CALL(unit, mbcm_dfe_fabric_cell_type_get, (unit, entry, &cell_info->cell_type));
    SOCDNX_IF_ERR_EXIT(rv);

    rv = MBCM_DFE_DRIVER_CALL(unit, mbcm_dfe_fabric_cell_parse_table_get, (unit, cell_info->cell_type, SOC_DCMN_FABRIC_CELL_PARSE_TABLE_MAX_NOF_LINES, parse_table, &nof_lines, is_two_parts));
    SOCDNX_IF_ERR_EXIT(rv);

    rv = soc_dcmn_fabric_cell_parser(unit, entry, entry_2, parse_table, nof_lines, cell_info, is_two_parts);
    SOCDNX_IF_ERR_EXIT(rv);

exit:
    SOCDNX_FUNC_RETURN; 
}


#undef _ERR_MSG_MODULE_NAME

