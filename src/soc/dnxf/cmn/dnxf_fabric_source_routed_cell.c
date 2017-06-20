/*
 * $Id: dnxf_fabric_source_routed_cell.c,v 1.7 Broadcom SDK $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * SOC DNXF FABRIC SOURCE ROUTED CELL
 */

#ifdef _ERR_MSG_MODULE_NAME
  #error "_ERR_MSG_MODULE_NAME redefined"
#endif

#define _ERR_MSG_MODULE_NAME BSL_SOC_FABRIC

/*************
 * INCLUDES  *
 *************/
#include <shared/bsl.h>

#include <bcm/fabric.h>

#include <soc/error.h>

#include <soc/dnxc/legacy/error.h>
#include <soc/dnxc/legacy/dnxc_fabric_cell.h>
#include <soc/dnxc/legacy/dnxc_cells_buffer.h>

#include <soc/dnxf/cmn/dnxf_defs.h>
#include <soc/dnxf/cmn/dnxf_fabric_source_routed_cell.h>
#include <soc/dnxf/cmn/dnxf_fabric.h>
#include <soc/dnxf/cmn/dnxf_stack.h>
#include <soc/dnxf/cmn/dnxf_drv.h>
#include <soc/dnxf/cmn/mbcm.h>

/*************
 * Defines  *
 *************/

#define SOC_DNXF_FABRIC_SRC_VSC256_V1_CELL_PATH_LINK_FIP_SWITCH_POSITION      (0)
#define SOC_DNXF_FABRIC_SRC_VSC256_V1_CELL_PATH_LINK_FE1_SWITCH_POSITION      (1)
#define SOC_DNXF_FABRIC_SRC_VSC256_V1_CELL_PATH_LINK_FE2_SWITCH_POSITION      (2)
#define SOC_DNXF_FABRIC_SRC_VSC256_V1_CELL_PATH_LINK_FE3_SWITCH_POSITION      (3)

/*************
 * FUNCTIONS *
 *************/

/*
 * Function:
 *      soc_dnxf_cpu2cpu_write
 * Purpose:
 *      Build and send SR cpu data cell
 * Parameters:
 *      unit                            - (IN)  Unit number.
 *      sr_link_list                    - (IN)  SR cell path
 *      data_in_size                    - (IN)  size of data_in
 *      data_in                         - (IN)  Data to send on cell
 * Returns:
 *      SOC_E_XXX
 */
soc_error_t
soc_dnxf_cpu2cpu_write(
    int                            unit,
    const dnxc_sr_cell_link_list_t  *sr_link_list,
    uint32                         data_in_size,
    uint32                         *data_in
  )
{
    dnxc_vsc256_sr_cell_t cell;
    int rc;
    DNXC_INIT_FUNC_DEFS;

    sal_memset(&cell.payload.data[0], 0x0, sizeof(uint32)*DNXC_VSC256_SR_DATA_CELL_PAYLOAD_MAX_LENGTH_U32);
    cell.header.cell_type = soc_dnxc_fabric_cell_type_sr_cell;

    rc = MBCM_DNXF_DRIVER_CALL(unit,mbcm_dnxf_stk_modid_get,(unit, &cell.header.source_device));
    DNXC_IF_ERR_EXIT(rc);

    cell.header.source_level = sr_link_list->src_entity_type;
    cell.header.destination_level = sr_link_list->dest_entity_type;
    cell.header.fip_link = sr_link_list->path_links[SOC_DNXF_FABRIC_SRC_VSC256_V1_CELL_PATH_LINK_FIP_SWITCH_POSITION];
    cell.header.fe1_link = sr_link_list->path_links[SOC_DNXF_FABRIC_SRC_VSC256_V1_CELL_PATH_LINK_FE1_SWITCH_POSITION];
    cell.header.fe2_link = sr_link_list->path_links[SOC_DNXF_FABRIC_SRC_VSC256_V1_CELL_PATH_LINK_FE2_SWITCH_POSITION];
    cell.header.fe3_link = sr_link_list->path_links[SOC_DNXF_FABRIC_SRC_VSC256_V1_CELL_PATH_LINK_FE3_SWITCH_POSITION];
    cell.header.is_inband = 0;
    cell.header.ack = 0;  
    cell.header.pipe_id = sr_link_list->pipe_id; 
    
    sal_memcpy(cell.payload.data, data_in, 
               data_in_size < DNXC_VSC256_SR_DATA_CELL_PAYLOAD_MAX_LENGTH_U32 ? WORDS2BYTES(data_in_size) : WORDS2BYTES(DNXC_VSC256_SR_DATA_CELL_PAYLOAD_MAX_LENGTH_U32));

    rc = MBCM_DNXF_DRIVER_CALL(unit,mbcm_dnxf_sr_cell_send,(unit, &cell));
    DNXC_IF_ERR_EXIT(rc);
                            
exit:
    DNXC_FUNC_RETURN;

}

soc_error_t
soc_dnxf_sr_cell_format_type_get(int unit, const dnxc_vsc256_sr_cell_t* cell, soc_dnxf_fabric_link_cell_size_t* vsc_format)
{
    soc_port_t link;
    soc_error_t rc;
    DNXC_INIT_FUNC_DEFS;

    /* Get link id of the next hop from cell header */
    switch (cell->header.source_level)
    {
    case bcmFabricDeviceTypeFE2:
        link = cell->header.fe2_link;
        break;
    case bcmFabricDeviceTypeFE1:
        link = cell->header.fe1_link;
        break;
    case bcmFabricDeviceTypeFE3:
        link = cell->header.fe3_link;
        break;
    default:
        DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("source level %d is unknown"),cell->header.source_level));
    }

    /* Retreive vsc format given link id */
    rc = MBCM_DNXF_DRIVER_CALL(unit,mbcm_dnxf_fabric_links_cell_format_get,(unit, link, vsc_format));
    DNXC_IF_ERR_EXIT(rc);
    
exit:
    DNXC_FUNC_RETURN;     
}

/*
 * Function:
 *      soc_dnxf_route2sr_link_list
 * Purpose:
 *      transform a route struct to dnxf_sr_cell_link_list_t struct
 * Parameters:
 *      unit           - (IN)  Unit number.
 *      device_entity  - (IN)  Value to translate
 *      actual_entity  - (OUT) Translated value
 * Returns:
 *      SOC_E_XXX
 */
soc_error_t
soc_dnxf_route2sr_link_list(
  int unit,
  const soc_dnxc_fabric_route_t *route,
  dnxc_sr_cell_link_list_t *sr_link_list
)
{ 
    int  hop_index, rc;
    bcm_fabric_link_connectivity_t link_partner;
    DNXC_INIT_FUNC_DEFS;
    
    DNXC_NULL_CHECK(route);
    DNXC_NULL_CHECK(sr_link_list);
    
    if(0 == route->number_of_hops)
    {
        DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("zero hops")));
    }
    
    rc = MBCM_DNXF_DRIVER_CALL(unit,mbcm_dnxf_fabric_link_connectivity_status_get,(unit, route->hop_ids[0], &link_partner));
    DNXC_IF_ERR_EXIT(rc);
      
    switch(SOC_DNXF_CONFIG(unit).fabric_device_mode) {
        case soc_dnxf_fabric_device_mode_single_stage_fe2:
        case soc_dnxf_fabric_device_mode_multi_stage_fe2:
            sr_link_list->src_entity_type = bcmFabricDeviceTypeFE2;
            break;
        case soc_dnxf_fabric_device_mode_multi_stage_fe13:
            if(bcmFabricDeviceTypeFE2 == link_partner.device_type) {
                sr_link_list->src_entity_type = bcmFabricDeviceTypeFE1;
            } else {
                sr_link_list->src_entity_type = bcmFabricDeviceTypeFE3;
            }
            break;
        default:
            DNXC_EXIT_WITH_ERR(SOC_E_INTERNAL, (_BSL_DNXC_MSG("failed to get source device")));
    }
    

    switch(route->number_of_hops) 
    {
        case 1:
            if(bcmFabricDeviceTypeFE2 == link_partner.device_type)
                    sr_link_list->dest_entity_type = bcmFabricDeviceTypeFE2;
                else if(bcmFabricDeviceTypeFAP == link_partner.device_type || bcmFabricDeviceTypeFOP == link_partner.device_type )
                    sr_link_list->dest_entity_type = bcmFabricDeviceTypeFOP;
                else if(bcmFabricDeviceTypeFE13 == link_partner.device_type || bcmFabricDeviceTypeFE3 == link_partner.device_type )
                    sr_link_list->dest_entity_type = bcmFabricDeviceTypeFE3;
                else
                {
                    DNXC_EXIT_WITH_ERR(SOC_E_INTERNAL, (_BSL_DNXC_MSG("failed to get destination device")));
                }
              break;
              
        case 2:
                if(bcmFabricDeviceTypeFE2 == link_partner.device_type)
                    sr_link_list->dest_entity_type = bcmFabricDeviceTypeFE3;
                else if(bcmFabricDeviceTypeFE13 == link_partner.device_type || bcmFabricDeviceTypeFE3 == link_partner.device_type )
                    sr_link_list->dest_entity_type = bcmFabricDeviceTypeFOP;
                else
                {
                    DNXC_EXIT_WITH_ERR(SOC_E_INTERNAL, (_BSL_DNXC_MSG("failed to get destination device")));
                }
              break;
              
        case 3:
                if(bcmFabricDeviceTypeFE2 == link_partner.device_type)
                    sr_link_list->dest_entity_type = bcmFabricDeviceTypeFOP;
                else
                {
                    DNXC_EXIT_WITH_ERR(SOC_E_INTERNAL, (_BSL_DNXC_MSG("failed to get destination device")));
                }
              break;
              
        default:
            DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("number_of_hops %d is out of limit"),route->number_of_hops));
    }
    
    /* Initilize link path */
    sr_link_list->path_links[SOC_DNXF_FABRIC_SRC_VSC256_V1_CELL_PATH_LINK_FIP_SWITCH_POSITION] = 0;
    sr_link_list->path_links[SOC_DNXF_FABRIC_SRC_VSC256_V1_CELL_PATH_LINK_FE1_SWITCH_POSITION] = 0;
    sr_link_list->path_links[SOC_DNXF_FABRIC_SRC_VSC256_V1_CELL_PATH_LINK_FE2_SWITCH_POSITION] = 0;
    sr_link_list->path_links[SOC_DNXF_FABRIC_SRC_VSC256_V1_CELL_PATH_LINK_FE3_SWITCH_POSITION] = 0;

    for (hop_index=0; hop_index<route->number_of_hops; hop_index++)
    {
        switch (hop_index)
        {
        case 0:
            if (sr_link_list->src_entity_type == dnxcFabricDeviceTypeFE2)
            {
                sr_link_list->path_links[SOC_DNXF_FABRIC_SRC_VSC256_V1_CELL_PATH_LINK_FE2_SWITCH_POSITION] = (uint8) route->hop_ids[hop_index];
            }
            else if (sr_link_list->src_entity_type == dnxcFabricDeviceTypeFE1)
            {
                sr_link_list->path_links[SOC_DNXF_FABRIC_SRC_VSC256_V1_CELL_PATH_LINK_FE1_SWITCH_POSITION] = (uint8) route->hop_ids[hop_index];
            }
            else if (sr_link_list->src_entity_type == dnxcFabricDeviceTypeFE3) 
            {
                sr_link_list->path_links[SOC_DNXF_FABRIC_SRC_VSC256_V1_CELL_PATH_LINK_FE3_SWITCH_POSITION] = (uint8) route->hop_ids[hop_index];
            } else 
            {
                DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("source entity %d is unknown"),sr_link_list->src_entity_type));
            }
            break;
        case 1:
            if (sr_link_list->src_entity_type == dnxcFabricDeviceTypeFE2)
            {            
                sr_link_list->path_links[SOC_DNXF_FABRIC_SRC_VSC256_V1_CELL_PATH_LINK_FE3_SWITCH_POSITION] = (uint8) route->hop_ids[hop_index];
            }
            else if (sr_link_list->src_entity_type == dnxcFabricDeviceTypeFE1)
            {
                sr_link_list->path_links[SOC_DNXF_FABRIC_SRC_VSC256_V1_CELL_PATH_LINK_FE2_SWITCH_POSITION] = (uint8) route->hop_ids[hop_index];
            }
            else if (sr_link_list->src_entity_type == dnxcFabricDeviceTypeFE3)
            {
                sr_link_list->path_links[SOC_DNXF_FABRIC_SRC_VSC256_V1_CELL_PATH_LINK_FIP_SWITCH_POSITION] = (uint8) route->hop_ids[hop_index];
            }
            else 
            {
                DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("source entity %d is unknown"),sr_link_list->src_entity_type));
            }
            break;
        case 2:
            if (sr_link_list->src_entity_type == dnxcFabricDeviceTypeFE2)
            {            
                sr_link_list->path_links[SOC_DNXF_FABRIC_SRC_VSC256_V1_CELL_PATH_LINK_FIP_SWITCH_POSITION] = (uint8) route->hop_ids[hop_index];
            }
            else if (sr_link_list->src_entity_type == dnxcFabricDeviceTypeFE1)
            {
                sr_link_list->path_links[SOC_DNXF_FABRIC_SRC_VSC256_V1_CELL_PATH_LINK_FE3_SWITCH_POSITION] = (uint8) route->hop_ids[hop_index];
            }
            else if (sr_link_list->src_entity_type == dnxcFabricDeviceTypeFE3)
            {
                sr_link_list->path_links[SOC_DNXF_FABRIC_SRC_VSC256_V1_CELL_PATH_LINK_FE1_SWITCH_POSITION] = (uint8) route->hop_ids[hop_index];
            }
            else 
            {
                DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("source entity %d is unknown"),sr_link_list->src_entity_type));
            }
            break;
        }
    }

    /*Pipe_id*/
    sr_link_list->pipe_id = route->pipe_id;
    
exit:
    DNXC_FUNC_RETURN;
}

/*
 * Function:
 *      soc_dnxf_sr_cell_send
 * Purpose:
 *      Transmitting data with source routed cell 
 * Parameters:
 *      unit             - (IN)  Unit number.
 *      flags            - (IN) Configuration parameters 
 *      route_id         - (IN) The route, or group of routes
 *      data_in_size     - (IN) data_in size 
 *      data_in          - (IN) data_in size 
 * Returns:
 *      SOC_E_XXX
 */
soc_error_t 
soc_dnxf_sr_cell_send( 
  int unit, 
  uint32 flags, 
  soc_dnxc_fabric_route_t* route, 
  uint32 data_in_size, 
  uint32 *data_in
)
{
    soc_error_t rc = SOC_E_NONE;
    dnxc_sr_cell_link_list_t sr_link_list;
    DNXC_INIT_FUNC_DEFS;

  
    DNXC_NULL_CHECK(route);
    DNXC_NULL_CHECK(data_in);
  
    DNXF_UNIT_LOCK_TAKE_DNXC(unit);

    if(0 == route->number_of_hops) {
        DNXC_EXIT_WITH_ERR(SOC_E_NOT_FOUND, (_BSL_DNXC_MSG("route has zero hops")));
    }
  
    rc = soc_dnxf_route2sr_link_list(unit, route, &sr_link_list);
   DNXC_IF_ERR_EXIT(rc);

    rc = soc_dnxf_cpu2cpu_write(unit, &sr_link_list, data_in_size,data_in);
    DNXC_IF_ERR_EXIT(rc);
    
exit:
    DNXF_UNIT_LOCK_RELEASE_DNXC(unit);
    DNXC_FUNC_RETURN;
}

/*
 * Function:
 *      soc_dnxf_sr_cell_payload_receive
 * Purpose:
 *      Receiving data from source routed cell 
 * Parameters:
 *      unit              - (IN)  Unit number.
 *      flags             - (IN)  Configuration parameters 
 *      data_out_max_size - (IN)  Max size of data_out 
 *      data_out_size     - (OUT) data_out size 
 *      data_out          - (OUT) data received 
 * Returns:
 *      SOC_E_XXX
 */
soc_error_t 
soc_dnxf_sr_cell_payload_receive( 
 int unit, 
 uint32 flags, 
 uint32 data_out_max_size, 
 uint32 *data_out_size, 
 uint32 *data_out
)
{
    soc_error_t rc = SOC_E_NONE;
    dnxc_vsc256_sr_cell_t cell;
    DNXC_INIT_FUNC_DEFS;
    
  
    sal_memset(&cell, 0x00, sizeof(dnxc_vsc256_sr_cell_t));
    DNXF_UNIT_LOCK_TAKE_DNXC(unit);

    rc = soc_dnxf_sr_cell_receive(unit, &cell);
    DNXC_IF_ERR_EXIT(rc);

    (*data_out_size) = data_out_max_size<DNXC_VSC256_SR_DATA_CELL_PAYLOAD_MAX_LENGTH_U32 ? data_out_max_size : DNXC_VSC256_SR_DATA_CELL_PAYLOAD_MAX_LENGTH_U32;
    sal_memcpy(data_out, cell.payload.data, WORDS2BYTES(*data_out_size));
  
exit:
    DNXF_UNIT_LOCK_RELEASE_DNXC(unit);
    DNXC_FUNC_RETURN;
}

/*
 * Function:
 *      soc_dnxf_sr_cell_receive
 * Purpose:
 *      Getting source routed cell.
 *      First check the buffer.
 *      if the buffer is empty checking cpu buffer.
 * Parameters:
 *      unit                            - (IN)  Unit number
 *      cell                            - (IN)  The parsed cell
 * Returns:
 *      SOC_E_xxx
 */
soc_error_t 
soc_dnxf_sr_cell_receive(int unit, dnxc_vsc256_sr_cell_t* cell)
{
    int rv, is_empty;
    DNXC_INIT_FUNC_DEFS;
    
    /*Check if cell exist in data base */
    rv = dnxc_cells_buffer_is_empty(unit, &SOC_DNXF_CONTROL(unit)->sr_cells_buffer, &is_empty);
    DNXC_IF_ERR_EXIT(rv);

    /*Load cells to data base */
    /*Stop loading if sr cell loaded or buffer is empty*/
    while (is_empty) 
    {
        rv = soc_dnxf_fabric_cell_load(unit);
        if (rv == SOC_E_EMPTY)
        {
            break;
        }
        DNXC_IF_ERR_EXIT(rv);

        rv = dnxc_cells_buffer_is_empty(unit, &SOC_DNXF_CONTROL(unit)->sr_cells_buffer, &is_empty);
        DNXC_IF_ERR_EXIT(rv);
    }
    if (is_empty)
    {
        DNXC_EXIT_WITH_ERR_NO_MSG(SOC_E_EMPTY);
    }

    /*Pull cell from data base*/
    rv = dnxc_cells_buffer_get(unit, &SOC_DNXF_CONTROL(unit)->sr_cells_buffer, cell);
    DNXC_IF_ERR_EXIT(rv);

exit:
    DNXC_FUNC_RETURN; 
}


#undef _ERR_MSG_MODULE_NAME

