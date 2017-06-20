
/*
 * $Id: dnxf_fabric_cell.c,v 1.14 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * SOC DNXF FABRIC CELL
 */

#ifdef _ERR_MSG_MODULE_NAME
  #error "_ERR_MSG_MODULE_NAME redefined"
#endif

#define _ERR_MSG_MODULE_NAME BSL_SOC_FABRIC
#include <shared/bsl.h>
#include <soc/dnxc/legacy/error.h>

#include <soc/dnxf/cmn/dnxf_drv.h>
#include <soc/dnxf/cmn/dnxf_defs.h>
#include <soc/dnxf/cmn/dnxf_fabric_cell.h>
#include <soc/dnxf/cmn/dnxf_fabric_source_routed_cell.h>
#include <soc/dnxf/cmn/mbcm.h>
#include <bcm_int/control.h>
#include <soc/dnxc/legacy/dnxc_fabric_cell.h>
#include <soc/dnxc/legacy/dnxc_cells_buffer.h>
#include <soc/dnxc/legacy/dnxc_captured_buffer.h>

/* Static functions */
static void clear_data_out_strcute (soc_dnxc_captured_control_cell_t* data_out)
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
soc_dnxf_cell_filter_set(int unit, uint32 flags, uint32 array_size, soc_dnxc_filter_type_t* filter_type_arr, uint32* filter_type_val) 
{
    int rc;
    DNXC_INIT_FUNC_DEFS;

    if (!SOC_UNIT_VALID(unit)) {
        DNXC_EXIT_WITH_ERR(SOC_E_UNIT, (_BSL_DNXC_MSG("Invalid unit")));
    }

    DNXC_NULL_CHECK(filter_type_arr);
    DNXC_NULL_CHECK(filter_type_val);

    DNXF_UNIT_LOCK_TAKE_DNXC(unit);
    rc = MBCM_DNXF_DRIVER_CALL(unit,mbcm_dnxf_cell_filter_set,(unit, flags, array_size, filter_type_arr, filter_type_val));
    DNXC_IF_ERR_EXIT(rc);

exit:
    DNXF_UNIT_LOCK_RELEASE_DNXC(unit);
    DNXC_FUNC_RETURN;
}

int 
soc_dnxf_cell_filter_count_get(int unit, int *count)
{
    int rc;
    DNXC_INIT_FUNC_DEFS;

    if (!SOC_UNIT_VALID(unit)) {
        DNXC_EXIT_WITH_ERR(SOC_E_UNIT, (_BSL_DNXC_MSG("Invalid unit")));
    }

    DNXC_NULL_CHECK(count);

    DNXF_UNIT_LOCK_TAKE_DNXC(unit);
    rc = MBCM_DNXF_DRIVER_CALL(unit,mbcm_dnxf_cell_filter_count_get,(unit, count));
    DNXC_IF_ERR_EXIT(rc);

exit:
    DNXF_UNIT_LOCK_RELEASE_DNXC(unit);
    DNXC_FUNC_RETURN;

}

int 
soc_dnxf_cell_filter_clear(int unit)
{
    int rc;
    DNXC_INIT_FUNC_DEFS;

    if (!SOC_UNIT_VALID(unit)) {
        DNXC_EXIT_WITH_ERR(SOC_E_UNIT, (_BSL_DNXC_MSG("Invalid unit")));
    }

    DNXF_UNIT_LOCK_TAKE_DNXC(unit);

    rc = MBCM_DNXF_DRIVER_CALL_NO_ARGS(unit, mbcm_dnxf_cell_filter_clear);
    DNXC_IF_ERR_EXIT(rc);

exit:
    DNXF_UNIT_LOCK_RELEASE_DNXC(unit);
    DNXC_FUNC_RETURN;
}

int 
soc_dnxf_cell_filter_receive(int unit, dnxc_captured_cell_t* data_out)
{

    int rv, is_empty;
    DNXC_INIT_FUNC_DEFS;

    if (!SOC_UNIT_VALID(unit)) {
        DNXC_EXIT_WITH_ERR(SOC_E_UNIT, (_BSL_DNXC_MSG("Invalid unit")));
    }

    DNXC_NULL_CHECK(data_out);
    DNXF_UNIT_LOCK_TAKE_DNXC(unit);

    /*Check if cell exist in data base */
    rv = dnxc_captured_buffer_is_empty(unit, &SOC_DNXF_CONTROL(unit)->captured_cells_buffer, &is_empty);
    DNXC_IF_ERR_EXIT(rv);

    /*Load cells to data base */
    /*Stop loading if captured cell loaded or buffer is empty*/
    while (is_empty) 
    {
        rv = soc_dnxf_fabric_cell_load(unit);
        if (rv == SOC_E_EMPTY)
        {
            break;
        }
        DNXC_IF_ERR_EXIT(rv);

        rv = dnxc_captured_buffer_is_empty(unit, &SOC_DNXF_CONTROL(unit)->captured_cells_buffer, &is_empty);
        DNXC_IF_ERR_EXIT(rv);
    }
    if (is_empty)
    {
        DNXC_EXIT_WITH_ERR_NO_MSG(SOC_E_EMPTY);
    }

    /*Pull cell from data base*/
    rv = dnxc_captured_buffer_get(unit, &SOC_DNXF_CONTROL(unit)->captured_cells_buffer, data_out);
    DNXC_IF_ERR_EXIT(rv);


exit:
    DNXF_UNIT_LOCK_RELEASE_DNXC(unit);
    DNXC_FUNC_RETURN;
}

/*
 * Function:
 *      soc_dnxf_control_cell_filter_set
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
int soc_dnxf_control_cell_filter_set(int unit, uint32 flags, soc_dnxc_control_cell_types_t cell_type, uint32 array_size, soc_dnxc_control_cell_filter_type_t* control_cell_filter_type_arr, uint32* filter_type_val) 
{
    int rc;
    DNXC_INIT_FUNC_DEFS;

    if (!SOC_UNIT_VALID(unit)) {
        DNXC_EXIT_WITH_ERR(SOC_E_UNIT, (_BSL_DNXC_MSG("Invalid unit")));
    }
    if (array_size != 0) {
        DNXC_NULL_CHECK(control_cell_filter_type_arr);
        DNXC_NULL_CHECK(filter_type_val);
    }

    DNXF_UNIT_LOCK_TAKE_DNXC(unit);
    rc = MBCM_DNXF_DRIVER_CALL(unit,mbcm_dnxf_control_cell_filter_set,(unit, flags, cell_type, array_size, control_cell_filter_type_arr, filter_type_val));
    DNXC_IF_ERR_EXIT(rc);

exit:
    DNXF_UNIT_LOCK_RELEASE_DNXC(unit);
    DNXC_FUNC_RETURN;
}

/*
 * Function:
 *      soc_dnxf_control_cell_filter_clear
 * Purpose:
 *      Clear the control cell of filter and mask
 * Parameters:
 *      unit                            - (IN)  Unit number
 *  
 * Returns:
 *      SOC_E_xxx
 */
int 
soc_dnxf_control_cell_filter_clear(int unit)
{
    int rc;
    DNXC_INIT_FUNC_DEFS;

    if (!SOC_UNIT_VALID(unit))
    {
        DNXC_EXIT_WITH_ERR(SOC_E_UNIT, (_BSL_DNXC_MSG("Invalid unit")));
    }

    DNXF_UNIT_LOCK_TAKE_DNXC(unit);
    rc = MBCM_DNXF_DRIVER_CALL_NO_ARGS(unit, mbcm_dnxf_control_cell_filter_clear);
    DNXC_IF_ERR_EXIT(rc);

exit:
    DNXF_UNIT_LOCK_RELEASE_DNXC(unit);
    DNXC_FUNC_RETURN;
}

/*
 * Function:
 *      soc_dnxf_control_cell_filter_receive
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
soc_dnxf_control_cell_filter_receive(int unit, soc_dnxc_captured_control_cell_t* data_out)
{
    int rc;
    DNXC_INIT_FUNC_DEFS;

    if (!SOC_UNIT_VALID(unit)) {
        DNXC_EXIT_WITH_ERR(SOC_E_UNIT, (_BSL_DNXC_MSG("Invalid unit")));
    }

    DNXC_NULL_CHECK(data_out);

    /* Clear the strcure, if the output is "-1" this field is not avaialble on this specific control cell type */
    clear_data_out_strcute(data_out);

    DNXF_UNIT_LOCK_TAKE_DNXC(unit);
    rc = MBCM_DNXF_DRIVER_CALL(unit,mbcm_dnxf_control_cell_filter_receive,(unit, data_out));
    DNXC_IF_ERR_EXIT(rc);


exit:
    DNXF_UNIT_LOCK_RELEASE_DNXC(unit);
    DNXC_FUNC_RETURN;

}


/*
 * Function:
 *      soc_dnxf_fabric_cell_load
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
soc_dnxf_fabric_cell_load(int unit)
{
    int rv;
    int is_two_parts = 0;
    soc_dnxc_fabric_cell_entry_t entry;
    soc_dnxc_fabric_cell_entry_t entry_2;
    soc_dnxc_fabric_cell_info_t cell_info;
    DNXC_INIT_FUNC_DEFS;

    sal_memset(&cell_info, 0x00, sizeof(soc_dnxc_fabric_cell_info_t));
    
    /* Get cell entry */
    rv = soc_dnxf_fabric_cell_entry_get(unit, entry);
    if (rv == SOC_E_EMPTY)
    {
        DNXC_EXIT_WITH_ERR_NO_MSG(SOC_E_EMPTY);
    }
    DNXC_IF_ERR_EXIT(rv);
    
    /* Check if cell is in 2 parts */
    DNXC_IF_ERR_EXIT(MBCM_DNXF_DRIVER_CALL(unit, mbcm_dnxf_fabric_cell_is_cell_in_two_parts, (unit, entry, &is_two_parts)));
    if (is_two_parts)
    {
        /* Get second cell */
        rv =  soc_dnxf_fabric_cell_entry_get(unit, entry_2);
        DNXC_IF_ERR_EXIT(rv);  
    }
    /*
     * Parse cell
     */
    rv = soc_dnxf_fabric_cell_parse(unit, entry, entry_2, &cell_info, is_two_parts);
    DNXC_IF_ERR_EXIT(rv);

    /*
     * Add to data base 
     */
    if(cell_info.cell_type == soc_dnxc_fabric_cell_type_sr_cell) 
    {
        rv = dnxc_cells_buffer_add(unit, &SOC_DNXF_CONTROL(unit)->sr_cells_buffer, &cell_info.cell.sr_cell);
        DNXC_IF_ERR_EXIT(rv);
    } 
    else 
    {
        rv = dnxc_captured_buffer_add(unit, &SOC_DNXF_CONTROL(unit)->captured_cells_buffer, &cell_info.cell.captured_cell);
        DNXC_IF_ERR_EXIT(rv);
    }

exit:
    DNXC_FUNC_RETURN; 
}

soc_error_t
soc_dnxf_fabric_cell_entry_get(int unit, soc_dnxc_fabric_cell_entry_t entry)
{
    int rv;
    int channel;
    int found_cell = 0;
    DNXC_INIT_FUNC_DEFS;

    /*Get cell using FIFO DMA support*/
    if (SOC_DNXF_CONFIG(unit).fabric_cell_fifo_dma_enable != SOC_DNXF_PROPERTY_UNAVAIL && SOC_DNXF_CONFIG(unit).fabric_cell_fifo_dma_enable)
    {
        for (channel = SOC_DNXF_DEFS_GET(unit, fifo_dma_fabric_cell_first_channel);
              channel < SOC_DNXF_DEFS_GET(unit, fifo_dma_nof_fabric_cell_channels);
              channel++)
        {
            rv = soc_dnxf_fifo_dma_channel_entry_get(unit, channel, SOC_DNXC_FABRIC_CELL_ENTRY_MAX_SIZE_UINT32 * sizeof(uint32), 
                                                    SOC_DNXF_DEFS_GET(unit, fifo_dma_fabric_cell_nof_entries_per_cell), (uint8 *) entry);
            if (rv == SOC_E_EMPTY)
            {
                /*check next channel*/
                continue;
            }
            DNXC_IF_ERR_EXIT(rv);

            found_cell = 1;
            break;
        }
    } else {
        /*Get cell directly*/
        rv = MBCM_DNXF_DRIVER_CALL(unit,mbcm_dnxf_fabric_cell_get,(unit, entry));
        if (rv == SOC_E_EMPTY)
        {
            DNXC_EXIT_WITH_ERR_NO_MSG(SOC_E_EMPTY);
        }
        DNXC_IF_ERR_EXIT(rv);
        found_cell = 1;
    }
    if (!found_cell)
    {
        DNXC_EXIT_WITH_ERR_NO_MSG(SOC_E_EMPTY);
    }

exit:
    DNXC_FUNC_RETURN;
}

/*
 * Function:
 *      soc_dnxf_fabric_cell_parse
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
soc_dnxf_fabric_cell_parse(int unit, soc_dnxc_fabric_cell_entry_t entry, soc_dnxc_fabric_cell_entry_t entry_2, soc_dnxc_fabric_cell_info_t *cell_info, int is_two_parts)
{
    int rv;
    uint32 nof_lines;
    soc_dnxc_fabric_cell_parse_table_t parse_table[SOC_DNXC_FABRIC_CELL_PARSE_TABLE_MAX_NOF_LINES];
    DNXC_INIT_FUNC_DEFS;

    rv = MBCM_DNXF_DRIVER_CALL(unit, mbcm_dnxf_fabric_cell_type_get, (unit, entry, &cell_info->cell_type));
    DNXC_IF_ERR_EXIT(rv);

    rv = MBCM_DNXF_DRIVER_CALL(unit, mbcm_dnxf_fabric_cell_parse_table_get, (unit, cell_info->cell_type, SOC_DNXC_FABRIC_CELL_PARSE_TABLE_MAX_NOF_LINES, parse_table, &nof_lines, is_two_parts));
    DNXC_IF_ERR_EXIT(rv);

    rv = soc_dnxc_fabric_cell_parser(unit, entry, entry_2, parse_table, nof_lines, cell_info, is_two_parts);
    DNXC_IF_ERR_EXIT(rv);

exit:
    DNXC_FUNC_RETURN; 
}


#undef _ERR_MSG_MODULE_NAME

