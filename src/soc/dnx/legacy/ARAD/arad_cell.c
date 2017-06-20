/* $Id: jer2_arad_cell.c,v 1.28 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/

#ifdef _ERR_MSG_MODULE_NAME

  #error "_ERR_MSG_MODULE_NAME redefined"
#endif

#define _ERR_MSG_MODULE_NAME BSL_SOC_FABRIC

#include <shared/bsl.h>
#include <shared/swstate/access/sw_state_access.h>

#include <soc/mem.h>
#include <soc/dnxc/legacy/error.h>
#include <soc/dnx/legacy/mbcm.h>
#include <soc/dnx/legacy/ARAD/arad_cell.h>
#include <soc/mcm/memregs.h>

#include <soc/dnx/legacy/fabric.h>
#include <soc/dnx/legacy/drv.h>
#include <soc/dnx/legacy/ARAD/arad_api_framework.h>
#include <soc/dnx/legacy/ARAD/arad_fabric.h>
#include <soc/dnx/legacy/ARAD/arad_sw_db.h>
#include <soc/dnx/legacy/ARAD/arad_chip_regs.h>

#include <soc/dnxc/legacy/vsc256_fabric_cell.h>

#if defined(BCM_88690_A0)



#define JER2_ARAD_CELL_PATH_LINK_FIP_SWITCH_POSITION      (0)
#define JER2_ARAD_CELL_PATH_LINK_FE1_SWITCH_POSITION      (1)
#define JER2_ARAD_CELL_PATH_LINK_FE2_SWITCH_POSITION      (2)
#define JER2_ARAD_CELL_PATH_LINK_FE3_SWITCH_POSITION      (3)
#define JER2_ARAD_CELL_MAX_NOF_TRIES_WAITING_FOR_ACK      (1000)
#define JER2_ARAD_CELL_ADDRESS_POSITION_0                 (0)
#define JER2_ARAD_CELL_ADDRESS_POSITION_1                 (1)
#define JER2_ARAD_CELL_ADDRESS_POSITION_2                 (2)
#define JER2_ARAD_CELL_ADDRESS_POSITION_3                 (3)
#define JER2_ARAD_CELL_ADDRESS_POSITION_4                 (4)
#define JER2_ARAD_CELL_WRITE_POSITION_0                   (0)
#define JER2_ARAD_CELL_WRITE_POSITION_1                   (1)
#define JER2_ARAD_CELL_WRITE_POSITION_2                   (2)
#define JER2_ARAD_CELL_WRITE_POSITION_4                   (4)
#define JER2_ARAD_FE600_RTP_INDIRECT_RW_ADDR              (0x0441 * 4)
#define JER2_ARAD_FE600_RTP_INDIRECT_WRITE_DATA0          (0x0420 * 4)
#define JER2_ARAD_FE600_RTP_INDIRECT_WRITE_DATA1          (0x0421 * 4)
#define JER2_ARAD_FE600_RTP_INDIRECT_WRITE_DATA2          (0x0422 * 4)
#define JER2_ARAD_FE600_RTP_INDIRECT_RW_TRIGGER           (0x0440 * 4)
#define JER2_ARAD_FE600_RTP_INDIRECT_READ_DATA0           (0x0430 * 4)
#define JER2_ARAD_FE600_RTP_INDIRECT_READ_DATA1           (0x0431 * 4)
#define JER2_ARAD_FE600_RTP_INDIRECT_READ_DATA2           (0x0432 * 4)
#define JER2_ARAD_CELL_NOF_CELL_IDENTS                    (0x1ff)

#define DNX_SAND_FAILURE(_sand_ret) \
    ((dnx_handle_sand_result(_sand_ret)) < 0)

/*
 * The function check if a route is valid
 */

int
soc_dnx_jer2_arad_fabric_inband_is_valid(
  int unit, 
  int id,
  uint32 *is_valid
)
{
  uint32
    reg;

  DNXC_INIT_FUNC_DEFS;

  DNXC_IF_ERR_EXIT(READ_FDT_IN_BAND_MEMm(unit, MEM_BLOCK_ANY, id, &reg));
  *is_valid = soc_FDT_IN_BAND_MEMm_field32_get(unit, &reg, VALIDf);

exit:
  DNXC_FUNC_RETURN;
}

/*
 * The function set a group in the hardware. 
 * It takes any of the routes in the array route_ids, and set its group to group_id 
 */
int
soc_dnx_jer2_arad_fabric_inband_route_group_set(
  int unit,
  int group_id,
  int flags,
  int route_count, 
  int *route_ids
)
{
  uint32
    group_id0 = group_id,
    route_index;
  uint32
    reg_32;
  uint32 
    res;

  DNX_SAND_INIT_ERROR_DEFINITIONS(0);

  /* Setting the group_id*/
  for (route_index = 0; route_index <route_count; route_index++){

    DNX_SAND_SOC_IF_ERROR_RETURN(res, 10, exit, READ_FDT_IN_BAND_MEMm(unit, MEM_BLOCK_ANY, route_ids[route_index], &reg_32));
    soc_FDT_IN_BAND_MEMm_field_set(unit, &reg_32, DESTINATION_GROUPf, &group_id0);
    DNX_SAND_SOC_IF_ERROR_RETURN(res, 20, exit, WRITE_FDT_IN_BAND_MEMm(unit, MEM_BLOCK_ANY, route_ids[route_index], &reg_32));
  }


exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in soc_dnx_jer2_arad_fabric_inband_route_group_set()",0,0);
}

/*
 *The function takes struct soc-fabric_inband_route_t and set a 32-register in order to write it in the hardware
 */
static
int dnxc_soc_fabric_inband_route_t_2reg_32(int unit, const dnxc_soc_fabric_inband_route_t* route, uint32 *reg){
  int rv;
  uint32 fields;
  DNX_TMC_SR_CELL_LINK_LIST sr_link_list;
  DNXC_INIT_FUNC_DEFS;

  DNXC_NULL_CHECK(reg);
  *reg = 0;
  fields = 0;

  soc_FDT_IN_BAND_MEMm_field32_set(unit, reg, DESTINATION_GROUPf, fields);
  rv = soc_dnx_fabric_inband_route2sr_link_list(unit, route, &sr_link_list);
  if(rv) {
    return rv;
  }

  switch(sr_link_list.dest_entity_type)
  {
  case DNX_SAND_FE1_ENTITY: 
  case DNX_SAND_FE3_ENTITY: 
      fields = 0; 
      break;
  case DNX_SAND_FE2_ENTITY: 
      fields = 1; 
      break;
  default: 
      return SOC_E_PARAM;
  }

  /* 
   *  If we have a send from fip to fe2, we have only the link which goes out from fip. 
   *  If we have a send from fip to fe1 and then to fe2 and fe3, we have 3 links:
   *  the first is from fip, the second is from fe1, and the third is from fe2.
   */
  soc_FDT_IN_BAND_MEMm_field32_set(0, reg, DESTINATION_LEVELf, fields);
  if(route->number_of_hops >= 1) {
    fields = sr_link_list.path_links[0];
    soc_FDT_IN_BAND_MEMm_field32_set(0, reg, FIP_SWITCHf, fields);
  }
  if(route->number_of_hops >= 2) {
    fields = sr_link_list.path_links[1];
    soc_FDT_IN_BAND_MEMm_field32_set(0, reg, FE_1_SWITCHf, fields);
  }
  if(route->number_of_hops >= 3) {
    fields = sr_link_list.path_links[2];
    soc_FDT_IN_BAND_MEMm_field32_set(0, reg, FE_2_SWITCHf, fields);
  }

  if(route->number_of_hops == 0) {/* i.e. invalid the route*/
    fields = FALSE;
  }
  else
  {
    fields = TRUE;
  }

  soc_FDT_IN_BAND_MEMm_field32_set(0, reg, VALIDf, fields);

exit:
  DNXC_FUNC_RETURN;
}

/*
 * The function set route in the hardware
 */
int
soc_dnx_jer2_arad_fabric_inband_route_set(
  int unit, 
  int route_id, 
  dnxc_soc_fabric_inband_route_t *route
)
{
  int 
    rv;  
  uint32
    reg;

  rv = dnxc_soc_fabric_inband_route_t_2reg_32(unit, route, &reg);
  if(SOC_FAILURE(rv)) {
    goto soc_dnx_attach_error;
  }
  rv = WRITE_FDT_IN_BAND_MEMm(unit, MEM_BLOCK_ANY, route_id, &reg);
  if(SOC_FAILURE(rv)) {
    goto soc_dnx_attach_error;
  }
  
  return SOC_E_NONE;  

soc_dnx_attach_error:
    LOG_ERROR(BSL_LS_SOC_FABRIC,
              (BSL_META_U(unit,
                          "soc_dnx_fabric_inband_route_set: unit %d failed (%s)\n"), 
                          unit, soc_errmsg(rv)));    
    return rv;
}

/*
 * The function gets a group_id and returns the ids of its group in the array route_ids. 
 */
int
soc_dnx_jer2_arad_fabric_inband_route_group_get( 
  int unit, 
  int group_id, 
  int flags, 
  int route_count_max, 
  int *route_count, 
  int *route_ids 
) 
{
  uint32
    index,
    reg,
    cur_route_group,
    is_valid;
  uint32
    res;

  DNX_SAND_INIT_ERROR_DEFINITIONS(0);
  *route_count = 0;
  for (index = 0; index < SOC_DNX_JER2_ARAD_NUM_OF_ENTRIES; ++index)
  {
    DNX_SAND_SOC_IF_ERROR_RETURN(res, 10, exit, READ_FDT_IN_BAND_MEMm(unit, MEM_BLOCK_ANY, index, &reg));
    soc_FDT_IN_BAND_MEMm_field_get(unit, &reg, DESTINATION_GROUPf, &cur_route_group);
    soc_FDT_IN_BAND_MEMm_field_get(unit, &reg, VALIDf, &is_valid);
    if((cur_route_group == group_id) && is_valid) {
      route_ids[(*route_count)++] = index;
    }
  }

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in soc_dnx_jer2_arad_fabric_inband_route_group_get()", 0, 0);
}

/*
 * The function tranlate from the structure sr_link_list to dnxc_soc_fabric_inband_route_t
 */
static int
sr_link_list2soc_dnx_fabric_route(
  int unit,
  DNX_TMC_SR_CELL_LINK_LIST sr_link_list,
  dnxc_soc_fabric_inband_route_t *route
)
{
  int 
    hop_index;
  uint32 
    dnx_sand_dev_id;
  DNX_TMC_FABRIC_LINKS_CON_STAT_INFO_ARR 
    connectivity_map;
  uint32
    res = 0;
  DNXC_INIT_FUNC_DEFS;
  
  sal_memset(route, 0, sizeof(dnxc_soc_fabric_inband_route_t));

  dnx_sand_dev_id = (unit);
  
  DNXC_NULL_CHECK(route);

  res = jer2_arad_fabric_topology_status_connectivity_get_unsafe(
    0,
    SOC_DNX_DEFS_GET(unit, nof_fabric_links) - 1,
    dnx_sand_dev_id,
    &connectivity_map
  );
  if(DNX_SAND_FAILURE(res)) {
    return SOC_E_INTERNAL;
  }
  
  if ( sr_link_list.dest_entity_type == DNX_SAND_FIP_ENTITY)
  {
    route->number_of_hops = 0;
  }
  else if (sr_link_list.dest_entity_type == DNX_SAND_FE1_ENTITY)
  {
    if (connectivity_map.link_info[sr_link_list.path_links[0]].far_dev_type == DNX_TMC_FAR_DEVICE_TYPE_FE1)
    {
      route->number_of_hops = 1;
    }
    else
    {
      return SOC_E_LIMIT;
    }
  }
  else if(sr_link_list.dest_entity_type == DNX_SAND_FE2_ENTITY)
  {
     if (connectivity_map.link_info[sr_link_list.path_links[0]].far_dev_type == DNX_TMC_FAR_DEVICE_TYPE_FE2)
     {
       route->number_of_hops = 1;
     }
     else if(connectivity_map.link_info[sr_link_list.path_links[0]].far_dev_type == DNX_TMC_FAR_DEVICE_TYPE_FE1)
     {
       route->number_of_hops = 2;
     }
     else
     {
       return SOC_E_LIMIT;
     }
  }
  else if(sr_link_list.dest_entity_type == DNX_SAND_FE3_ENTITY)
  {
    route->number_of_hops = 3;
  }
  else if (sr_link_list.dest_entity_type == DNX_SAND_FOP_ENTITY)
  {
    if (connectivity_map.link_info[sr_link_list.path_links[0]].far_dev_type == DNX_TMC_FAR_DEVICE_TYPE_FE2)
    {
      route->number_of_hops = 2;
    }
    else
    {
      route->number_of_hops = 4;
    }
  }
  
  for (hop_index=0; hop_index<route->number_of_hops; hop_index++)
  {
    route->link_ids[hop_index] = sr_link_list.path_links[hop_index];
  }

exit:
  DNXC_FUNC_RETURN;
}

/*
 * The function get a 32-register of a route data and parse it into a struct dnxc_soc_fabric_inband_route_t
 */
static int
reg_32_2dnxc_soc_fabric_inband_route_t(
  int unit,
  const uint32 reg,
  dnxc_soc_fabric_inband_route_t *route
)
{
  int rv;
  uint32 fields;
  DNX_TMC_SR_CELL_LINK_LIST sr_link_list;
  DNXC_INIT_FUNC_DEFS;
  
  DNXC_NULL_CHECK(route);
  fields = 0;
  sal_memset(&sr_link_list, 0, sizeof(sr_link_list));
  soc_FDT_IN_BAND_MEMm_field_get(0, &reg, VALIDf, &fields);
  if(!fields) {/*i.e. the entry is invalid*/
    sr_link_list.dest_entity_type = DNX_SAND_FIP_ENTITY;/* And hence, the number of hops will be zero.*/
  }
  soc_FDT_IN_BAND_MEMm_field_get(0, &reg, DESTINATION_LEVELf, &fields);
  sr_link_list.dest_entity_type = fields == 1 ? DNX_SAND_FE2_ENTITY : DNX_SAND_FE3_ENTITY;
  soc_FDT_IN_BAND_MEMm_field_get(0, &reg, FIP_SWITCHf, &fields);
  sr_link_list.path_links[0] = fields;
  soc_FDT_IN_BAND_MEMm_field_get(0, &reg, FE_1_SWITCHf, &fields);
  sr_link_list.path_links[1] = fields;
  soc_FDT_IN_BAND_MEMm_field_get(0, &reg, FE_2_SWITCHf, &fields);
  sr_link_list.path_links[2] = fields;
  rv = sr_link_list2soc_dnx_fabric_route(unit, sr_link_list, route);
  if(rv){
    return rv;
  }

exit:
  DNXC_FUNC_RETURN;
}

/*
 * Get a route from the hardware
 */
int
soc_dnx_jer2_arad_fabric_inband_route_get(
  int unit, 
  int route_id, 
  dnxc_soc_fabric_inband_route_t *route
)
{
  int 
    rv;  
  uint32
    reg;
  uint32
    res;

  DNX_SAND_INIT_ERROR_DEFINITIONS(0);

  DNX_SAND_SOC_IF_ERROR_RETURN(res, 10, exit, READ_FDT_IN_BAND_MEMm(unit, MEM_BLOCK_ANY, route_id, &reg));
  rv = reg_32_2dnxc_soc_fabric_inband_route_t(unit, reg, route);
  if(SOC_FAILURE(rv)) {
    goto soc_dnx_attach_error;
  }

  if(route->number_of_hops == 0)/*i.e. the route is invalid */
  {
    rv = SOC_E_PARAM;
    goto soc_dnx_attach_error;
  }
  return SOC_E_NONE;  

soc_dnx_attach_error:
    LOG_ERROR(BSL_LS_SOC_FABRIC,
              (BSL_META_U(unit,
                          "soc_dnx_fabric_inband_route_set: unit %d failed (%s)\n"), 
                          unit, soc_errmsg(rv)));    
    return rv;
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in soc_dnx_jer2_arad_fabric_inband_route_get()", 0, 0);
}



/*
 * The function gets register which contains data-cell and parse in into a structure 
 * vsc256_sr_cell_t 
 */
/*static*/ soc_error_t
soc_jer2_arad_parse_cell(int unit, uint8 is_inband, uint32* buf, dnxc_vsc256_sr_cell_t* cell) 
{
    int rc;
    soc_reg_above_64_val_t header_data;

    DNXC_INIT_FUNC_DEFS;

    SOC_REG_ABOVE_64_CLEAR(header_data);
    if(is_inband)
    {
        SHR_BITCOPY_RANGE(header_data, 0, buf, JER2_ARAD_SR_DATA_CELL_PAYLOAD_LENGTH, 8 * DNXC_VSC256_SR_DATA_CELL_HEADER_SIZE);
    }
    else
    {
        /* The header is bits 582:520 and they are bits 78:16 of the header in vsc256 format*/
        SHR_BITCOPY_RANGE(header_data, JER2_ARAD_RECEIVED_DATA_CELL_HEADER_OFFSET_VSC256, buf,
            JER2_ARAD_RECEIVED_DATA_CELL_HEADER_START, 8 * DNXC_VSC256_SR_DATA_CELL_HEADER_SIZE - JER2_ARAD_RECEIVED_DATA_CELL_HEADER_OFFSET_VSC256);

        /* The cell-type which may lie in bits 87:86 in the header data lies on bits 584:583 in buf
         * and hence,
         */
        SHR_BITCOPY_RANGE(header_data, DNXC_VSC256_SR_DATA_CELL_CELL_TYPE_START, buf, JER2_ARAD_RECEIVED_DATA_CELL_TYPE_START, DNXC_VSC256_SR_DATA_CELL_CELL_TYPE_LENGTH);
    }

    rc = soc_dnxc_vsc256_parse_header(unit, header_data, cell);
    DNXC_IF_ERR_EXIT(rc);

    if(!cell->header.is_inband) {
        SHR_BITCOPY_RANGE(cell->payload.data, 0, buf, 0, JER2_ARAD_RECEIVED_DATA_CELL_SIZE_IN_BITS) ;
    } else {
        rc = soc_dnxc_vsc256_parse_payload(unit, buf, cell, JER2_ARAD_INBAND_PAYLOAD_CELL_OFFSET);
        DNXC_IF_ERR_EXIT(rc);
    }

exit:
    DNXC_FUNC_RETURN; 
}

/*
 * The function takes a structure vsc256_sr_cell_t and build a buffer in oreder to send it.
 */
static soc_error_t 
soc_jer2_arad_sr_cell_format(int unit, const dnxc_vsc256_sr_cell_t* cell, int buf_size_bytes, uint32* buf) 
{
    soc_error_t rc;
    DNXC_INIT_FUNC_DEFS;

    sal_memset(buf, 0, buf_size_bytes);

    if(buf_size_bytes < JER2_ARAD_DATA_CELL_BYTE_SIZE) {
        DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("SR cell minimum buffer size is %d"),JER2_ARAD_DATA_CELL_BYTE_SIZE));
    }

    if(cell->header.is_inband && DNXC_VSC256_CELL_FORMAT_FE600 == cell->payload.inband.cell_format) {
        DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("VSC256 can't be sent in DNX_SAND_FE600 format")));
    }

    /* Build the header*/
    if (SOC_DNX_CONFIG(unit)->jer2_arad->init.fabric.is_fe600) {
        /*VSC128 cell*/
        rc = soc_dnxc_vsc256_to_vsc128_build_header(unit, cell, buf_size_bytes - (JER2_ARAD_SR_DATA_CELL_PAYLOAD_LENGTH/8), buf + (JER2_ARAD_SR_DATA_CELL_PAYLOAD_LENGTH/(8*sizeof(uint32))));
        DNXC_IF_ERR_EXIT(rc);
    } else {
        /*VSC256 cell*/
        rc = soc_dnxc_vsc256_build_header(unit, cell, buf_size_bytes - (JER2_ARAD_SR_DATA_CELL_PAYLOAD_LENGTH/8), buf + ((JER2_ARAD_SR_DATA_CELL_PAYLOAD_LENGTH + SOC_DNX_DEFS_GET(unit, source_routed_cells_header_offset))/(8*sizeof(uint32))));
        DNXC_IF_ERR_EXIT(rc);
    }

    /*
     * bits 1023:0 - payload
     */
    if(!cell->header.is_inband) {/* In inband we can send only 512 bits cell. It should be msb alligned*/
        SHR_BITCOPY_RANGE(buf, JER2_ARAD_SR_DATA_CELL_PAYLOAD_LENGTH - JER2_ARAD_RECEIVED_DATA_CELL_SIZE_IN_BITS,
            cell->payload.data, 0, JER2_ARAD_RECEIVED_DATA_CELL_SIZE_IN_BITS);
    }else {
        rc = soc_dnxc_vsc256_build_payload(unit, cell , buf_size_bytes, buf, JER2_ARAD_INBAND_PAYLOAD_CELL_OFFSET);
        DNXC_IF_ERR_EXIT(rc);
    }

exit:
    DNXC_FUNC_RETURN;  
}

uint32
  jer2_arad_sr_data_cell_to_fe1600_buffer(
    DNX_SAND_IN  int32          unit,
    DNX_SAND_IN  dnxc_vsc256_sr_cell_t          *data_cell,
    DNX_SAND_OUT uint32         *packed_data_cell
  )
{
    return soc_jer2_arad_sr_cell_format(unit, data_cell, JER2_ARAD_DATA_CELL_BYTE_SIZE, packed_data_cell);
}

static uint32
device_entity_value(
    DNX_SAND_IN     DNX_SAND_DEVICE_ENTITY            device_entity,
    DNX_SAND_OUT    dnxc_fabric_device_type_t*     dnxc_device_entity
  )
{
  switch(device_entity)
  {
    case DNX_SAND_DONT_CARE_ENTITY:
      *dnxc_device_entity = dnxcFabricDeviceTypeUnknown;
      return DNX_SAND_OK;
    case DNX_SAND_FE1_ENTITY:
      *dnxc_device_entity = dnxcFabricDeviceTypeFE1;
      return DNX_SAND_OK;
    case DNX_SAND_FE2_ENTITY:
      *dnxc_device_entity = dnxcFabricDeviceTypeFE2;
      return DNX_SAND_OK;
    case DNX_SAND_FE3_ENTITY:
      *dnxc_device_entity = dnxcFabricDeviceTypeFE3;
      return DNX_SAND_OK;
    case DNX_SAND_FAP_ENTITY:
      *dnxc_device_entity = dnxcFabricDeviceTypeFAP;
      return DNX_SAND_OK;
    case DNX_SAND_FOP_ENTITY:
      *dnxc_device_entity = dnxcFabricDeviceTypeFOP;
      return DNX_SAND_OK;
    case DNX_SAND_FIP_ENTITY:
      *dnxc_device_entity = dnxcFabricDeviceTypeFIP;
      return DNX_SAND_OK;
    case DNX_SAND_FE13_ENTITY:
      *dnxc_device_entity = dnxcFabricDeviceTypeFE13;
      return DNX_SAND_OK;
    default:
    {
      return DNX_SAND_ERR;
    }
  }
}


/*
 * Builds the data cell fields from the input.
 */
static uint32
jer2_arad_build_data_cell_for_fe1600(
    DNX_SAND_IN  int                        unit,
    DNX_SAND_IN  DNX_TMC_SR_CELL_LINK_LIST          *sr_link_list,
    DNX_SAND_IN  soc_reg_t                        *regs,
    DNX_SAND_IN  soc_mem_t                        *mems,
    DNX_SAND_IN  int32                         *port_or_copyno,
    DNX_SAND_IN  int32                         *index,
    DNX_SAND_IN  uint32                         *data_in,
    DNX_SAND_IN  uint32                         nof_words,
    DNX_SAND_IN  uint32                        is_write,
    DNX_SAND_IN  uint32                        is_inband,
    DNX_SAND_IN  uint8                        is_memory,
    DNX_SAND_OUT dnxc_vsc256_sr_cell_t                 *data_cell_sent
  )
{
  uint8
    i,
    at;
  uint16
    cell_id;
  int32
    block,
    index_valid;    
  uint32
    maddr,
    blkoff;
  soc_mem_info_t
    *meminfo;
  dnxc_fabric_device_type_t
    dnxc_device_entity;
  DNXC_INIT_FUNC_DEFS;

  /*
   * Construction of the cell information
   */
  data_cell_sent->header.cell_type = 1 ;
  data_cell_sent->header.source_device = (uint32) unit ;
  /* casting allowed: the unit should be in 11 bits*/
  data_cell_sent->header.source_level = dnxcFabricDeviceTypeFIP ;
  if(device_entity_value(sr_link_list->dest_entity_type, &dnxc_device_entity))
  {
    return DNX_SAND_ERR;
  }
  data_cell_sent->header.destination_level = dnxc_device_entity;
  
  data_cell_sent->header.fip_link =
    sr_link_list->path_links[JER2_ARAD_CELL_PATH_LINK_FIP_SWITCH_POSITION] ;
  /* casting allowed: only 6 significant bits */
  data_cell_sent->header.fe1_link =
    sr_link_list->path_links[JER2_ARAD_CELL_PATH_LINK_FE1_SWITCH_POSITION] ;
  /* casting allowed: only 6 significant bits */
  data_cell_sent->header.fe2_link =
    sr_link_list->path_links[JER2_ARAD_CELL_PATH_LINK_FE2_SWITCH_POSITION] ;
  /* casting allowed: only 7 significant bits in tmp_cell_info */
  data_cell_sent->header.fe3_link =
    sr_link_list->path_links[JER2_ARAD_CELL_PATH_LINK_FE3_SWITCH_POSITION] ;
  /* casting allowed: only 6 significant bits in tmp_cell_info */

  data_cell_sent->header.ack = 0 ;

  data_cell_sent->payload.inband.cell_format = DNXC_VSC256_CELL_FORMAT_FE1600;
  DNXC_IF_ERR_EXIT(sw_state_access[unit].dnx.soc.jer2_arad.tm.cell.current_cell_ident.get(unit, &cell_id));
  ++cell_id;
  DNXC_IF_ERR_EXIT(sw_state_access[unit].dnx.soc.jer2_arad.tm.cell.current_cell_ident.set(unit, cell_id));
  data_cell_sent->payload.inband.cell_id = cell_id;
  data_cell_sent->payload.inband.seq_num = 0;
  data_cell_sent->payload.inband.nof_commands = nof_words;

  if (is_inband)
  {
    if(nof_words > DNXC_VSC256_MAX_COMMANDS_ARRAY_SIZE) {
      DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("Number of command above the maximum")));
    }
    data_cell_sent->header.is_inband = TRUE;
    
    if (!is_memory) /* Direct command */
    {
      for(i = 0; i < nof_words; ++i) {
        if (!SOC_REG_IS_VALID(unit, regs[i])) {
          DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("Invalid register %s"), SOC_REG_NAME(unit, regs[i])));
        }

        if(is_write)
        {
          data_cell_sent->payload.inband.commands[i].opcode = soc_dnxc_inband_reg_write;
        }
        else
        {
          data_cell_sent->payload.inband.commands[i].opcode = soc_dnxc_inband_reg_read;
        }

        maddr = soc_reg_addr_get(unit, regs[i], port_or_copyno[i], index[i],
                                 SOC_REG_ADDR_OPTION_WRITE, &block, &at);
        data_cell_sent->payload.inband.commands[i].schan_block = ((maddr >> SOC_BLOCK_BP) & 0xf) | (((maddr >> SOC_BLOCK_MSB_BP) & 0x1) << 4);
        if(SOC_REG_IS_ABOVE_64(unit, regs[i])) {
          data_cell_sent->payload.inband.commands[i].length = SOC_REG_ABOVE_64_INFO(unit, regs[i]).size;
        }
        else if(SOC_REG_IS_64(unit, regs[i])) {
            data_cell_sent->payload.inband.commands[i].length = 8;
        } else {
            data_cell_sent->payload.inband.commands[i].length = 4;
        }
        
        if(data_cell_sent->payload.inband.commands[i].length > 16) {
          DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("can't read more than 128 bits data")));
        }

        /*clear block from address*/
        blkoff = ((data_cell_sent->payload.inband.commands[i].schan_block & 0xf) << SOC_BLOCK_BP) | (((data_cell_sent->payload.inband.commands[i].schan_block >> 4) & 0x3) << SOC_BLOCK_MSB_BP);
        data_cell_sent->payload.inband.commands[i].offset = maddr - blkoff;

        if(is_write) {
          sal_memcpy(
            data_cell_sent->payload.inband.commands[i].data,
            &data_in[i*DNXC_VSC256_COMMAND_DATA_SIZE_U32],
            data_cell_sent->payload.inband.commands[i].length);
        }
      }
    }
    else /* Indirect command */
    {
      for(i = 0; i < nof_words; ++i) {
        if (!soc_mem_is_valid(unit, mems[i])) {
         DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("Invalid memory %s"), SOC_MEM_NAME(unit, mems[i])));
        }
        meminfo = &SOC_MEM_INFO(unit, mems[i]);

        if (port_or_copyno[i] == MEM_BLOCK_ANY) {
          block = SOC_MEM_BLOCK_ANY(unit, mems[i]);
        } else {
          block = port_or_copyno[i];
        }

        if (!SOC_MEM_BLOCK_VALID(unit, mems[i], block)) {
          DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("invalid block %d for memory %s"), block, SOC_MEM_NAME(unit, mems[i])));
        }
    
        index_valid = (index[i] >= 0 && index[i] <= soc_mem_index_max(unit, mems[i]));
        if (!index_valid) {
          DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("invalid index %d for memory %s"), index[i], SOC_MEM_NAME(unit, mems[i])));
        }
        if (is_write)
        {
          data_cell_sent->payload.inband.commands[i].opcode = soc_dnxc_inband_mem_write;
        }
        else
        {
          data_cell_sent->payload.inband.commands[i].opcode = soc_dnxc_inband_mem_read;
        }
        maddr = soc_mem_addr_get(unit, mems[i], 0, block, index[i], &at);
        data_cell_sent->payload.inband.commands[i].schan_block = ((maddr >> SOC_BLOCK_BP) & 0xf) | (((maddr >> SOC_BLOCK_MSB_BP) & 0x1) << 4);
        data_cell_sent->payload.inband.commands[i].length = meminfo->bytes;

        /*clear block from address*/
        blkoff = ((data_cell_sent->payload.inband.commands[i].schan_block & 0xf) << SOC_BLOCK_BP) | (((data_cell_sent->payload.inband.commands[i].schan_block >> 4) & 0x3) << SOC_BLOCK_MSB_BP);
        data_cell_sent->payload.inband.commands[i].offset = maddr - blkoff;

        if(is_write) {
          if(meminfo->bytes > MAX_DATA_SIZE*sizeof(uint32)) {
            DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("can't read more than 128 byte data")));
          }
          sal_memcpy(
            data_cell_sent->payload.inband.commands[i].data,
            &data_in[i*DNXC_VSC256_COMMAND_DATA_SIZE_U32],
            meminfo->bytes
          );
        }
      }
    }
  }
  else /* Regular source routed cell, not inband*/
  {
    if(nof_words > JER2_ARAD_SR_DATA_CELL_PAYLOAD_LENGTH/8) {
      DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("Cell\'s size above the maximum")));
    }
    data_cell_sent->header.is_inband = FALSE ;
    dnx_sand_os_memcpy(&(data_cell_sent->payload.data), data_in, nof_words);        
  }

  return DNX_SAND_OK;

exit:
  DNXC_FUNC_RETURN; 
}

/*
 * Builds the data cell fields from the input.
 */
static uint32
jer2_arad_build_data_cell_for_fe600(
    DNX_SAND_IN  int                       unit,
    DNX_SAND_IN  DNX_TMC_SR_CELL_LINK_LIST         *sr_link_list,
    DNX_SAND_IN  uint32                        offset,
    DNX_SAND_IN  uint32                        *data_in,
    DNX_SAND_IN  uint32                        nof_words,
    DNX_SAND_IN  uint8                       is_write,
    DNX_SAND_IN  uint8                       is_indirect,
    DNX_SAND_IN  uint8                       is_inband,
    DNX_SAND_OUT DNX_SAND_DATA_CELL                  *data_cell_sent
  )
{
  uint32
    res = DNX_SAND_OK ,
    tmp_field_for_data_cell_add_fields = 0,
    trigger = 0x1 ,
    iter_nof_words = 0 ;

  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_BUILD_DATA_CELL_FOR_FE600);

  /*
   * Construction of the cell information
   */
  data_cell_sent->cell_type = 1 ;
  data_cell_sent->source_id = (uint16) unit ;
  /* casting allowed: the unit should be in 11 bits*/
  data_cell_sent->data_cell.source_routed.src_level = 1 ;
  data_cell_sent->data_cell.source_routed.dest_level =
    (uint8) dnx_sand_actual_entity_value( sr_link_list->dest_entity_type ) ;
  /* casting allowed: only 3 significant bits in the destination level */

  data_cell_sent->data_cell.source_routed.fip_switch =
    sr_link_list->path_links[JER2_ARAD_CELL_PATH_LINK_FIP_SWITCH_POSITION] ;
  /* casting allowed: only 6 significant bits */
  data_cell_sent->data_cell.source_routed.fe1_switch =
    sr_link_list->path_links[JER2_ARAD_CELL_PATH_LINK_FE1_SWITCH_POSITION] ;
  /* casting allowed: only 6 significant bits */
  data_cell_sent->data_cell.source_routed.fe2_switch =
    sr_link_list->path_links[JER2_ARAD_CELL_PATH_LINK_FE2_SWITCH_POSITION] ;
  /* casting allowed: only 7 significant bits in tmp_cell_info */
  data_cell_sent->data_cell.source_routed.fe3_switch =
    sr_link_list->path_links[JER2_ARAD_CELL_PATH_LINK_FE3_SWITCH_POSITION] ;
  /* casting allowed: only 6 significant bits in tmp_cell_info */

  if (is_inband)
  {
    data_cell_sent->data_cell.source_routed.indirect = is_indirect ;
    data_cell_sent->data_cell.source_routed.read_or_write = is_write ;

    data_cell_sent->data_cell.source_routed.inband_cell = 1 ;
    data_cell_sent->data_cell.source_routed.ack = 0 ;

    if (!is_indirect) /* Direct command */
    {
      if (!is_write) /* Direct read command: 1 word in each cell */
      {
        data_cell_sent->data_cell.source_routed.add_wr_cell[JER2_ARAD_CELL_ADDRESS_POSITION_0] = (uint16) offset ;
        /* casting allowed: only 16 significant bit in offset */
      }
      else /* Direct write command: 5 words in each cell */
      {
        for (iter_nof_words = 0 ; iter_nof_words < nof_words ; ++iter_nof_words)
        {
          data_cell_sent->data_cell.source_routed.add_wr_cell[iter_nof_words] = (uint16) offset ;
          /* casting allowed: only 16 significant bit in offset */
          data_cell_sent->data_cell.source_routed.data_wr_cell[iter_nof_words] = data_in[ iter_nof_words ] ;
        }
      }
    }
    else /* Indirect command */
    {
      if (!is_write) /* Indirect read command: 5 words in each cell */
      {
        data_cell_sent->data_cell.source_routed.add_wr_cell[JER2_ARAD_CELL_ADDRESS_POSITION_0] =
          (uint16) JER2_ARAD_FE600_RTP_INDIRECT_RW_ADDR ;
        data_cell_sent->data_cell.source_routed.add_wr_cell[JER2_ARAD_CELL_ADDRESS_POSITION_1] =
          (uint16) JER2_ARAD_FE600_RTP_INDIRECT_RW_TRIGGER ;
        data_cell_sent->data_cell.source_routed.add_wr_cell[JER2_ARAD_CELL_ADDRESS_POSITION_2] =
          (uint16) JER2_ARAD_FE600_RTP_INDIRECT_READ_DATA0 ;
        data_cell_sent->data_cell.source_routed.add_wr_cell[JER2_ARAD_CELL_ADDRESS_POSITION_3] =
          (uint16) JER2_ARAD_FE600_RTP_INDIRECT_READ_DATA1 ;
        data_cell_sent->data_cell.source_routed.add_wr_cell[JER2_ARAD_CELL_ADDRESS_POSITION_4] =
          (uint16) JER2_ARAD_FE600_RTP_INDIRECT_READ_DATA2 ;

        data_cell_sent->data_cell.source_routed.data_wr_cell[JER2_ARAD_CELL_WRITE_POSITION_0] = DNX_SAND_BIT(31)|offset;
        /* read address: bit(31) + base + offset */
        data_cell_sent->data_cell.source_routed.data_wr_cell[JER2_ARAD_CELL_WRITE_POSITION_1] = trigger ;
        /* write trigger */
        for (iter_nof_words = 0 ; iter_nof_words < JER2_ARAD_CELL_NOF_DATA_WORDS_IN_INDIRECT_CELL ; ++iter_nof_words)
        {
          data_cell_sent->data_cell.source_routed.data_wr_cell[JER2_ARAD_CELL_WRITE_POSITION_2 + iter_nof_words] = 0x0 ;
        }
        /* write data equals zero for the three last words */
      }
      else /* Indirect write command: 5 words in each cell */
      {
        data_cell_sent->data_cell.source_routed.add_wr_cell[JER2_ARAD_CELL_ADDRESS_POSITION_0] =
          (uint16) JER2_ARAD_FE600_RTP_INDIRECT_RW_ADDR ;
        data_cell_sent->data_cell.source_routed.add_wr_cell[JER2_ARAD_CELL_ADDRESS_POSITION_1] =
          (uint16) JER2_ARAD_FE600_RTP_INDIRECT_WRITE_DATA0 ;
        data_cell_sent->data_cell.source_routed.add_wr_cell[JER2_ARAD_CELL_ADDRESS_POSITION_2] =
          (uint16) JER2_ARAD_FE600_RTP_INDIRECT_WRITE_DATA1 ;
        data_cell_sent->data_cell.source_routed.add_wr_cell[JER2_ARAD_CELL_ADDRESS_POSITION_3] =
          (uint16) JER2_ARAD_FE600_RTP_INDIRECT_WRITE_DATA2 ;
        data_cell_sent->data_cell.source_routed.add_wr_cell[JER2_ARAD_CELL_ADDRESS_POSITION_4] =
          (uint16) JER2_ARAD_FE600_RTP_INDIRECT_RW_TRIGGER ;

        data_cell_sent->data_cell.source_routed.data_wr_cell[JER2_ARAD_CELL_WRITE_POSITION_0] = offset;
        /* write address: base + offset*/
        for (iter_nof_words = 0 ; iter_nof_words < nof_words ; ++iter_nof_words)
        {
          data_cell_sent->data_cell.source_routed.data_wr_cell[JER2_ARAD_CELL_WRITE_POSITION_1 + iter_nof_words] =
            data_in[ iter_nof_words ] ;
        }
        data_cell_sent->data_cell.source_routed.data_wr_cell[JER2_ARAD_CELL_WRITE_POSITION_4] = trigger ;
        /* write trigger */
      }
    }

    res = sw_state_access[unit].dnx.soc.jer2_arad.tm.cell.current_cell_ident.get(
            unit,
            &(data_cell_sent->data_cell.source_routed.cell_ident)
          );
    DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 8, exit);

    res = sw_state_access[unit].dnx.soc.jer2_arad.tm.cell.current_cell_ident.set(
            unit,
            ( (data_cell_sent->data_cell.source_routed.cell_ident + 1) % JER2_ARAD_CELL_NOF_CELL_IDENTS )
          );
    DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 9, exit);

    data_cell_sent->data_cell.source_routed.cell_format = 0 ;
  }
  else /* Regular source routed cell, not inband*/
  {
    data_cell_sent->data_cell.source_routed.indirect = 0 ;
    data_cell_sent->data_cell.source_routed.read_or_write = 0 ;

    data_cell_sent->data_cell.source_routed.inband_cell = 0 ;
    data_cell_sent->data_cell.source_routed.ack = 0 ;

    for (iter_nof_words = 0 ; iter_nof_words < DNX_SAND_SR_DATA_CELL_NOF_WORDS_IN_PAYLOAD ; ++iter_nof_words)
    {
      data_cell_sent->data_cell.source_routed.data_wr_cell[iter_nof_words] = data_in[ iter_nof_words ] ;
    }

    for (iter_nof_words = 0 ; iter_nof_words < DNX_SAND_SR_DATA_CELL_NOF_WORDS_IN_PAYLOAD ; ++iter_nof_words)
    {
      tmp_field_for_data_cell_add_fields = 0 ;

      res = dnx_sand_bitstream_get_any_field(
              &(data_in[DNX_SAND_SR_DATA_CELL_NOF_WORDS_IN_PAYLOAD]),
              iter_nof_words * DNX_SAND_SR_DATA_CELL_ADDRESS_LENGTH,
              DNX_SAND_SR_DATA_CELL_ADDRESS_LENGTH,
              &(tmp_field_for_data_cell_add_fields)
            );
      DNX_SAND_CHECK_FUNC_RESULT(res, 10, exit);

      data_cell_sent->data_cell.source_routed.add_wr_cell[iter_nof_words] = (uint16) tmp_field_for_data_cell_add_fields;
      /* casting allowed: only 16 significant bits in tmp_field_for_data_cell_add_fields*/
    }

    tmp_field_for_data_cell_add_fields = 0 ;
    res = dnx_sand_bitstream_get_any_field(
            &(data_in[DNX_SAND_SR_DATA_CELL_NOF_WORDS_IN_PAYLOAD]),
            DNX_SAND_SR_DATA_CELL_NOF_WORDS_IN_PAYLOAD * DNX_SAND_SR_DATA_CELL_ADDRESS_LENGTH,
            DNX_SAND_SR_DATA_CELL_NOT_INBAND_CELL_IDENT_LENGTH,
            &(tmp_field_for_data_cell_add_fields)
          );
    DNX_SAND_CHECK_FUNC_RESULT(res, 20, exit);

    data_cell_sent->data_cell.source_routed.cell_ident = (uint16) tmp_field_for_data_cell_add_fields;
    /* casting allowed: only 16 significant bits in tmp_field_for_data_cell_add_fields*/

    data_cell_sent->data_cell.source_routed.cell_format = 0 ;
  }

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_build_data_cell_for_fe600()",0,0);
}


/*
 * Sends a cell and acts on the trigger
 */
uint32
  jer2_arad_sr_send_cell(
    DNX_SAND_IN  int                unit,
    DNX_SAND_IN  uint32                is_fe1600,
    DNX_SAND_IN  uint32                route_group_id,
    DNX_SAND_IN  uint8                 is_group,
    DNX_SAND_IN  DNX_SAND_DATA_CELL    *data_cell_sent_old_format,
    DNX_SAND_IN  dnxc_vsc256_sr_cell_t      *data_cell_sent
  )
{
  uint32
    tmp_u32_fip_switch = 0 ,
    tmp_u32_output_link_in_five_bits = 0 ,
    tmp_reg,
    tmp_interrupt_reg,
    res = DNX_SAND_OK;
  uint64
      tmp_reg64, field64;
  uint32
    packed_cpu_data_cell_sent[JER2_ARAD_DATA_CELL_U32_SIZE];
  uint8
    is_set;
  soc_timeout_t
    to;
  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_SR_SEND_CELL);

  sal_memset(
          packed_cpu_data_cell_sent,
          0x0,
          JER2_ARAD_DATA_CELL_BYTE_SIZE
        );
  
  /*
   * Pack the cell.
   */
  if (is_fe1600)
  {
    res = jer2_arad_sr_data_cell_to_fe1600_buffer(
          unit,
          data_cell_sent,
          packed_cpu_data_cell_sent
        );
    DNX_SAND_CHECK_FUNC_RESULT(res, 10, exit);
  } 
  else
  {
    res = dnx_sand_data_cell_to_buffer(
          data_cell_sent_old_format,
          packed_cpu_data_cell_sent
        );
    DNX_SAND_CHECK_FUNC_RESULT(res, 10, exit);
  }
 

  /*
   * Copy the data.
   */
   DNX_SAND_SOC_IF_ERROR_RETURN(res, 1000, exit, WRITE_FDT_CPU_DATA_CELL_0r(unit, packed_cpu_data_cell_sent));
   DNX_SAND_SOC_IF_ERROR_RETURN(res, 1010, exit, WRITE_FDT_CPU_DATA_CELL_1r(unit, packed_cpu_data_cell_sent + 256/(8*sizeof(uint32))));
   DNX_SAND_SOC_IF_ERROR_RETURN(res, 1020, exit, WRITE_FDT_CPU_DATA_CELL_2r(unit, packed_cpu_data_cell_sent + 2*256/(8*sizeof(uint32))));
   DNX_SAND_SOC_IF_ERROR_RETURN(res, 1030, exit, WRITE_FDT_CPU_DATA_CELL_3r(unit, packed_cpu_data_cell_sent + 3*256/(8*sizeof(uint32))));
 
  /*
   * Fix the output link which is the fip switch link
   */
  if(is_fe1600){
    tmp_u32_fip_switch = data_cell_sent->header.fip_link ;
  }
  else{
    tmp_u32_fip_switch = data_cell_sent_old_format->data_cell.source_routed.fip_switch ;
  }
  res = dnx_sand_bitstream_set_any_field(
          &(tmp_u32_fip_switch),
          0 ,
          DNX_SAND_DATA_CELL_FIP_SWITCH_LENGTH+1,
          &tmp_u32_output_link_in_five_bits
        );
  DNX_SAND_CHECK_FUNC_RESULT(res, 20, exit);


  if(is_fe1600) {
    if(!is_group){
        DNX_SAND_SOC_IF_ERROR_RETURN(res, 1040, exit, READ_FDT_TRANSMIT_CELL_OUTPUT_LINK_NUMBERr(unit, &tmp_reg));
        soc_reg_field_set(unit, FDT_TRANSMIT_CELL_OUTPUT_LINK_NUMBERr, &tmp_reg, CPU_LINK_NUMf, tmp_u32_output_link_in_five_bits);
        DNX_SAND_SOC_IF_ERROR_RETURN(res, 1050, exit, WRITE_FDT_TRANSMIT_CELL_OUTPUT_LINK_NUMBERr(unit, tmp_reg));

        tmp_reg = 0;
        DNX_SAND_SOC_IF_ERROR_RETURN(res, 1060, exit, READ_FDT_TRANSMIT_DATA_CELL_TRIGGERr(unit, &tmp_reg));
        soc_reg_field_set(unit, FDT_TRANSMIT_DATA_CELL_TRIGGERr, &tmp_reg, CPU_TRGf, 0x1);
        DNX_SAND_SOC_IF_ERROR_RETURN(res, 1070, exit, WRITE_FDT_TRANSMIT_DATA_CELL_TRIGGERr(unit, tmp_reg));
    }
    else{
      /* Clearing the acks*/
      DNX_SAND_SOC_IF_ERROR_RETURN(res, 1080, exit, READ_FDT_IN_BAND_MANAGMENENTr(unit, &tmp_reg64));
      COMPILER_64_SET(field64,0,0x1);    
      soc_reg64_field_set(unit, FDT_IN_BAND_MANAGMENENTr, &tmp_reg64, IN_BAND_ACK_CLEARf, field64);
        
      res = jer2_arad_polling(
              unit,
              JER2_ARAD_TIMEOUT,
              JER2_ARAD_MIN_POLLS,
              FDT_IN_BAND_MANAGMENENTr,
              REG_PORT_ANY,
              0,
              IN_BAND_ACK_CLEARf,
              0
            );
      DNX_SAND_CHECK_FUNC_RESULT(res, 10, exit);
            
      /*Sending the data */
      soc_timeout_init(&to, JER2_ARAD_TIMEOUT, JER2_ARAD_MIN_POLLS);
      for(;;) {
        COMPILER_64_SET(field64,0,data_cell_sent->payload.inband.cell_id);    
        soc_reg64_field_set(unit, FDT_IN_BAND_MANAGMENENTr, &tmp_reg64, IN_BAND_CELL_IDf, field64);
        COMPILER_64_SET(field64,0,route_group_id);    
        soc_reg64_field_set(unit, FDT_IN_BAND_MANAGMENENTr, &tmp_reg64, IN_BAND_DESTINATION_GROUPf, field64);
        COMPILER_64_SET(field64,0,JER2_ARAD_IN_BAND_TIMEOUT_ATTEMPTS_CFG);    
        soc_reg64_field_set(unit, FDT_IN_BAND_MANAGMENENTr, &tmp_reg64, IN_BAND_TIMEOUT_ATTEMPTS_CFGf, field64);
        COMPILER_64_SET(field64,0,0x1);    
        soc_reg64_field_set(unit, FDT_IN_BAND_MANAGMENENTr, &tmp_reg64, IN_BAND_TRIGGERf, field64);
        DNX_SAND_SOC_IF_ERROR_RETURN(res, 1100, exit, WRITE_FDT_IN_BAND_MANAGMENENTr(unit, tmp_reg64));

        DNX_SAND_SOC_IF_ERROR_RETURN(res, 1110, exit, READ_FDT_INTERRUPT_REGISTERr_REG32(unit, &tmp_interrupt_reg));
        is_set = soc_reg_field_get(unit, FDT_INTERRUPT_REGISTERr, tmp_interrupt_reg, IN_BAND_LAST_READ_CNTf);
#ifdef PLISIM
        if (SAL_BOOT_PLISIM) {
            is_set = TRUE;
        }
#endif
        if(is_set) {
          break;
        }
        if (soc_timeout_check(&to)) {
          DNX_SAND_SET_ERROR_CODE(JER2_ARAD_DIAG_DRAM_ACCESS_TIMEOUT_ERR, 20, exit);
        }
      }
    }
  }
  else{
    DNX_SAND_SOC_IF_ERROR_RETURN(res, 1120, exit, READ_FDT_TRANSMIT_CELL_OUTPUT_LINK_NUMBERr(unit, &tmp_reg));
    soc_reg_field_set(unit, FDT_TRANSMIT_CELL_OUTPUT_LINK_NUMBERr, &tmp_reg, CPU_LINK_NUMf, tmp_u32_output_link_in_five_bits);
    DNX_SAND_SOC_IF_ERROR_RETURN(res, 1130, exit, WRITE_FDT_TRANSMIT_CELL_OUTPUT_LINK_NUMBERr(unit, tmp_reg));

    /*
     * Set the trigger.
     */
    DNX_SAND_SOC_IF_ERROR_RETURN(res, 1140, exit, READ_FDT_TRANSMIT_DATA_CELL_TRIGGERr(unit, &tmp_reg));
    soc_reg_field_set(unit, FDT_TRANSMIT_DATA_CELL_TRIGGERr, &tmp_reg, CPU_TRGf, 0x1);
    DNX_SAND_SOC_IF_ERROR_RETURN(res, 1150, exit, WRITE_FDT_TRANSMIT_DATA_CELL_TRIGGERr(unit, tmp_reg));
    
  }

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_sr_send_cell()",0,0);
}

uint32
  jer2_arad_cell_ack_get(
    DNX_SAND_IN  int                     unit,
    DNX_SAND_IN  uint8                      is_inband,
    DNX_SAND_OUT uint32                     *ack,
    DNX_SAND_OUT uint8                      *success
  )
{
  uint32
    reg_val,
    success_a,
    success_b,
    success_c,
    success_d;
  soc_reg_above_64_val_t
    payload_msb,
    payload_lsb,
    header,
    data;
  uint32 res;

  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_CELL_ACK_GET);

  DNX_SAND_CHECK_NULL_INPUT(ack);
  DNX_SAND_CHECK_NULL_INPUT(success);

  if(is_inband)
  {
#ifdef PLISIM
    if (SAL_BOOT_PLISIM) {
        *success = TRUE;
        JER2_ARAD_DO_NOTHING_AND_EXIT;
    }
#endif
    DNX_SAND_SOC_IF_ERROR_RETURN(res, 1250, exit, READ_FDR_INBAND_HEADER_VALIDr(unit, &reg_val));
    *success = soc_reg_field_get(unit, FDR_INBAND_HEADER_VALIDr, reg_val, INBAND_HEADER_VALIDf);
    if(*success)
    {
      DNX_SAND_SOC_IF_ERROR_RETURN(res, 1260, exit, READ_FDR_INBAND_PAYLOAD_MSBr(unit, payload_msb));
      DNX_SAND_SOC_IF_ERROR_RETURN(res, 1270, exit, READ_FDR_INBAND_PAYLOAD_LSBr(unit, payload_lsb));
      DNX_SAND_SOC_IF_ERROR_RETURN(res, 1280, exit, READ_FDR_INBAND_HEADERr(unit, header));
      SHR_BITCOPY_RANGE(ack, 0, payload_lsb, 0, 512);
      SHR_BITCOPY_RANGE(ack, 512, payload_msb, 0, 512);
      SHR_BITCOPY_RANGE(ack, 1024, header, 0, DNXC_VSC256_SR_DATA_CELL_HEADER_SIZE*8);
      JER2_ARAD_DO_NOTHING_AND_EXIT;
    }
    else
    {
      DNX_SAND_SET_ERROR_CODE(DNX_SAND_ERR, 5, exit);
    }
  }
  /*
   * Verification if the cell was received on the A side,
   * or on the B, C or D side
   */

  DNX_SAND_SOC_IF_ERROR_RETURN(res, 1290, exit, READ_FDR_INTERRUPT_REGISTER_1r(unit, &reg_val));
  success_a = soc_reg_field_get(unit, FDR_INTERRUPT_REGISTER_1r, reg_val, PRM_CPUDATACELLFNE_A_0f);
  success_b = soc_reg_field_get(unit, FDR_INTERRUPT_REGISTER_1r, reg_val, PRM_CPUDATACELLFNE_A_1f);
  success_c = soc_reg_field_get(unit, FDR_INTERRUPT_REGISTER_1r, reg_val, PRM_CPUDATACELLFNE_B_0f);
  success_d = soc_reg_field_get(unit, FDR_INTERRUPT_REGISTER_1r, reg_val, PRM_CPUDATACELLFNE_B_1f);

#ifdef PLISIM
  /* NOTE: the header will have changes because when we get cell we should do allignment,
   * However, if we only want to comapre sent cell to received one the data should be the same*/
  if (SAL_BOOT_PLISIM) {
      *success = TRUE;
      DNX_SAND_SOC_IF_ERROR_RETURN(res, 1292, exit, READ_FDT_CPU_DATA_CELL_2r(unit, data));
      SHR_BITCOPY_RANGE(ack, 0, data, 0, 256);
      DNX_SAND_SOC_IF_ERROR_RETURN(res, 1297, exit, READ_FDT_CPU_DATA_CELL_3r(unit, data));
      SHR_BITCOPY_RANGE(ack, 256, data, 0, 256 + DNXC_VSC256_SR_DATA_CELL_HEADER_SIZE*8);
      JER2_ARAD_DO_NOTHING_AND_EXIT;
  }
#endif

  if ( (!success_a) && (!success_b) && (!success_c) && (!success_d) )
  {
    *success = FALSE;
    JER2_ARAD_DO_NOTHING_AND_EXIT;
  }

  if (success_a)
  {
    DNX_SAND_SOC_IF_ERROR_RETURN(res, 1300, exit, READ_FDR_CPU_DATA_CELL_A_PRIMARYr(unit, data));
    sal_memcpy(ack, data,JER2_ARAD_DATA_CELL_RECEIVED_BYTE_SIZE);
  } else if (success_b)
  {
    DNX_SAND_SOC_IF_ERROR_RETURN(res, 1310, exit, READ_FDR_CPU_DATA_CELL_B_PRIMARYr(unit, data));/* Is it regs->fdr.cpu_data_cell_b_reg[0].addr.base) ??? */
    sal_memcpy(ack, data, JER2_ARAD_DATA_CELL_RECEIVED_BYTE_SIZE);
  } else if (success_c)
  {
    DNX_SAND_SOC_IF_ERROR_RETURN(res, 1320, exit, READ_FDR_CPU_DATA_CELL_C_PRIMARYr(unit, data));
    sal_memcpy(ack, data, JER2_ARAD_DATA_CELL_RECEIVED_BYTE_SIZE);
  } else if (success_d)
  {
    DNX_SAND_SOC_IF_ERROR_RETURN(res, 1330, exit, READ_FDR_CPU_DATA_CELL_D_PRIMARYr(unit, data));
    sal_memcpy(ack, data, JER2_ARAD_DATA_CELL_RECEIVED_BYTE_SIZE);
  }

  /*
   *  No Check the received cell is an ACK (only inband)
   */
  *success = TRUE;

exit:
    DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_cell_ack_get()",0,0);
}


static uint32
  jer2_arad_sr_rcv_cell(
    DNX_SAND_IN  int                       unit,
    DNX_SAND_IN  uint8                        is_fe1600,
    DNX_SAND_IN  uint8                        is_inband,
    DNX_SAND_OUT DNX_SAND_DATA_CELL           *data_cell_old_format,
    DNX_SAND_OUT dnxc_vsc256_sr_cell_t             *data_cell,
    DNX_SAND_OUT uint32                       *size, /*Unit: bytes. only in case of not inband*/
    DNX_SAND_OUT uint8                        *success
  )
{
  uint32
    res = DNX_SAND_OK;
  uint32
    packed_cpu_data_cell_rcv[JER2_ARAD_DATA_CELL_U32_SIZE] ;
  uint32
    temp_size;

  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_SR_RCV_CELL);

  if(is_fe1600) {
    DNX_SAND_CHECK_NULL_INPUT(data_cell);
  }
  else{
    DNX_SAND_CHECK_NULL_INPUT(data_cell_old_format);
  }
  DNX_SAND_CHECK_NULL_INPUT(success);

  sal_memset(packed_cpu_data_cell_rcv, 0x0, JER2_ARAD_DATA_CELL_U32_SIZE * sizeof(uint32));
  
  /*
   *  Get the ack
   */
  res = jer2_arad_cell_ack_get(
          unit,
          is_inband,
          packed_cpu_data_cell_rcv,
          success
        );
  DNX_SAND_CHECK_FUNC_RESULT(res, 20, exit);

  if(!is_inband) {
    /* set the size*/
    DNX_SAND_CHECK_NULL_INPUT(size);
    temp_size = 0;
    SHR_BITCOPY_RANGE(&temp_size, 0, packed_cpu_data_cell_rcv, JER2_ARAD_RECEIVED_DATA_CELL_SIZE_IN_BITS, JER2_ARAD_RECUEVED_DATA_CELL_CELL_SIZE_LENGTH_IN_BITS);
    ++temp_size;
#ifdef PLISIM
    /* we didn't got a real cell we got the data back from the FDT cpu registers */
    if (SAL_BOOT_PLISIM) {
        temp_size = JER2_ARAD_RECEIVED_DATA_CELL_SIZE_IN_BITS/8;
    }
#endif
    if(temp_size > JER2_ARAD_RECEIVED_DATA_CELL_SIZE_IN_BITS/8) {
      /* size of data is larger than expected, might be due to duplication*/
      temp_size = JER2_ARAD_RECEIVED_DATA_CELL_SIZE_IN_BITS/8;
    }
    *size = temp_size;
  }

  /*
   * Parsing to a source-routed cell
   */
  if (is_fe1600)
  {
    res = soc_jer2_arad_parse_cell(
          unit,
          is_inband,
          packed_cpu_data_cell_rcv,          
          data_cell
        );
    DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 30, exit);
  } 
  else
  {
    /* We should do allignment of the packet_cpu_data_cell_rcv
       The format of the packet is:
       bit [64:0]  cell_header     = data[584:520];
       int         byte_size       = data[519:512]+1;
       bit [511:0] data_fragement  = data[511:0];
     
       bit [1:0]   cell_type       = cell_header[64:63];
     
       header[55:54] = cell_type;
       header[45:0] = cell_header[45:0];

     */

    SHR_BITCOPY_RANGE(packed_cpu_data_cell_rcv, JER2_ARAD_RECEIVED_DATA_CELL_SIZE_IN_BITS, 
                      packed_cpu_data_cell_rcv, JER2_ARAD_RECEIVED_DATA_CELL_HEADER_START, JER2_ARAD_RECEIVED_DATA_CELL_HEADER_SIZE_IN_BITS_VSC128);

    SHR_BITCOPY_RANGE(packed_cpu_data_cell_rcv, 
                      JER2_ARAD_RECEIVED_DATA_CELL_SIZE_IN_BITS + JER2_ARAD_RECEIVED_DATA_CELL_HEADER_SIZE_IN_BITS_VSC128 - DNXC_VSC256_SR_DATA_CELL_CELL_TYPE_LENGTH, 
                      packed_cpu_data_cell_rcv, 
                      JER2_ARAD_RECEIVED_DATA_CELL_HEADER_START,
                      DNXC_VSC256_SR_DATA_CELL_CELL_TYPE_LENGTH);

     
    res = dnx_sand_buffer_to_data_cell(
          packed_cpu_data_cell_rcv,
          0,
          data_cell_old_format
        );
    DNX_SAND_CHECK_FUNC_RESULT(res, 30, exit);
  } 

  
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_rcv_cell()",0,0);
}

uint32
  jer2_arad_transaction_with_fe1600(
    DNX_SAND_IN  int                       unit,
    DNX_SAND_IN  DNX_TMC_SR_CELL_LINK_LIST        *sr_link_list,
    DNX_SAND_IN  uint32                       route_group_id,
    DNX_SAND_IN  uint8                        is_group,
    DNX_SAND_IN  soc_reg_t                    *regs,
    DNX_SAND_IN  soc_mem_t                    *mems,
    DNX_SAND_IN  int32                        *port_or_copyno,
    DNX_SAND_IN  int32                        *index,
    DNX_SAND_IN  uint32                       *data_in, /*If inband, each 4 entries are 1 unit of 128 bits to be taken*/
    DNX_SAND_IN  uint32                       size,  /* in bytes if not inband, number of regs/mems if inband */
    DNX_SAND_IN  uint8                        is_write,
    DNX_SAND_IN  uint8                        is_inband,
    DNX_SAND_IN  uint8                        is_memory,
    DNX_SAND_OUT soc_reg_above_64_val_t       *data_out, /* The out data will be in units of 128 bits each one*/
    DNX_SAND_OUT uint32                       *data_out_size /* In case of read not inband the actual size read in bytes*/
  )
{
  uint8
    success = FALSE;
  uint32
    i;
  uint32
    start_long = 0 ,
    nof_words = 0 ,    
    res = DNX_SAND_OK ;
  dnxc_vsc256_sr_cell_t
    data_cell_sent,
    data_cell_rcv;

  DNX_SAND_INIT_ERROR_DEFINITIONS(0);

  sal_memset(&data_cell_sent, 0x0, sizeof(dnxc_vsc256_sr_cell_t));
  
  sal_memset(&data_cell_rcv, 0x0, sizeof(dnxc_vsc256_sr_cell_t));
  nof_words = size;
  
  if (is_inband)
  {
    /*
     * Construction of the cell information
     */

    res = jer2_arad_build_data_cell_for_fe1600(
      unit,
      sr_link_list,
      regs,
      mems,
      port_or_copyno,
      index,
      &data_in[start_long],
      nof_words,
      is_write,
      is_inband,
      is_memory,
      &data_cell_sent
    );
    DNX_SAND_CHECK_FUNC_RESULT(res, 10, exit);

    /*
     * Sending and receiving the cell
     */
    res = jer2_arad_sr_send_and_wait_ack(
            unit,
            &data_cell_sent,
            NULL,
            route_group_id,
            is_group,
            &data_cell_rcv,
            NULL
          );
    DNX_SAND_CHECK_FUNC_RESULT(res, 150, exit);

    if (
        data_cell_sent.payload.inband.cell_id
        != data_cell_rcv.payload.inband.cell_id
       )
    {
      DNX_SAND_SET_ERROR_CODE(JER2_ARAD_CELL_DIFFERENT_CELL_IDENT_ERR, 153, exit);
    }

  }
  else if (!is_write) /* Read for a regular source routed cell*/
  {
     res = jer2_arad_sr_rcv_cell(
            unit,
            TRUE,
            FALSE,
            NULL,
            &data_cell_rcv,
            data_out_size,
            &success            
          );
    DNX_SAND_CHECK_FUNC_RESULT(res, 152, exit);

    if (success == FALSE)
    {
      DNX_SAND_SET_ERROR_CODE(JER2_ARAD_CELL_NO_RECEIVED_CELL_ERR, 155, exit);
    }


  }
  else /* Write for a regular source routed cell*/
  {
    /*
     * Construction of the cell information
     */
    res = jer2_arad_build_data_cell_for_fe1600(
            unit,
            sr_link_list,
            regs,
            mems,
            port_or_copyno,
            index,
            &data_in[start_long],
            nof_words,
            is_write,
            is_memory,
            is_inband,
            &data_cell_sent
          );
    DNX_SAND_CHECK_FUNC_RESULT(res, 155, exit);

    res = jer2_arad_sr_send_cell(
            unit,
            TRUE,
            route_group_id,
            is_group,
            NULL,
            &data_cell_sent
          );
    DNX_SAND_CHECK_FUNC_RESULT(res, 158, exit);
  }

  /*
   * Construction of the output in case of read
   */
  if (is_inband)
  {
    if(!is_write) /*read*/
    {
      for (i=0; i < size; i++)
      {
        SOC_REG_ABOVE_64_CLEAR(data_out[i]);
        SHR_BITCOPY_RANGE(data_out[i], 0, data_cell_rcv.payload.inband.commands[i].data, 0, 128);
      }        
    }
  }
  else if (!is_write) /* Regular source routed cell for read, copy of the payload */
  {/* Copy all the payload */
    for(i=0; i < (nof_words*8)/128; ++i) {
      SOC_REG_ABOVE_64_CLEAR(data_out[i]);
      SHR_BITCOPY_RANGE(data_out[i], 0, data_cell_rcv.payload.data, i*128, 128);
    }
  }    
  
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_transaction_with_fe1600()",0,0);
}



/*
 * Builds the data, packs to a buffer, sends the cell,
 * receives the buffer, and decomposes it to the output.
 */
static uint32
  jer2_arad_transaction_with_fe600(
    DNX_SAND_IN  int                       unit,
    DNX_SAND_IN  DNX_TMC_SR_CELL_LINK_LIST         *sr_link_list,
    DNX_SAND_IN  uint32                        offset,
    DNX_SAND_IN  uint32                        *data_in,
    DNX_SAND_IN  uint32                        size,
    DNX_SAND_IN  uint8                       is_write,
    DNX_SAND_IN  uint8                       is_indirect,
    DNX_SAND_IN  uint8                       is_inband,
    DNX_SAND_OUT uint32                        *data_out
  )
{
  uint8
    success = FALSE;
  uint32
    iter = 0 ,
    iter_size = 0 ;
  uint32
    start_long = 0 ,
    nof_words = 0 ,
    tmp_field_for_data_cell_add_fields = 0,
    tmp_data_out[JER2_ARAD_CELL_NOF_DATA_WORDS_IN_INDIRECT_CELL] = {0},
    tmp_not_inband_data_out[DNX_SAND_DATA_CELL_PAYLOAD_IN_UINT32S] = {0},
    res = DNX_SAND_OK ;
  DNX_SAND_DATA_CELL
    data_cell_sent,
    data_cell_rcv ;

  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_TRANSACTION_WITH_FE600);

  /*
   * Sending the necessary number of cells according to the size
   */
  while (iter_size < size)
  {
    sal_memset(&data_cell_sent, 0x0, sizeof(DNX_SAND_DATA_CELL));
    
    sal_memset(&data_cell_rcv, 0x0, sizeof(DNX_SAND_DATA_CELL));
    /*
     * Number of words to copy
     */
    if (is_inband)
      {
      if( ( !is_write ) && ( !is_indirect ) ) /* Read direct: one word per cell, i.e. the offset*/
      {
        nof_words = 1  ;
      }
      else if( ( is_write ) && ( !is_indirect ) ) /* Write direct: five words per cell */
      {
        nof_words = DNX_SAND_MIN(DNX_SAND_SR_DATA_CELL_NOF_WORDS_IN_PAYLOAD , (size - iter_size) / JER2_ARAD_CELL_NOF_BYTES_IN_UINT32 );
      }
      else if( ( !is_write ) && ( is_indirect ) ) /* Indirect Read: three words of data_in per cell */
      {
        nof_words = JER2_ARAD_CELL_NOF_DATA_WORDS_IN_INDIRECT_CELL;
      }
      else /* Indirect Write: three words of data_in per cell */
      {
        nof_words = DNX_SAND_MIN(JER2_ARAD_CELL_NOF_DATA_WORDS_IN_INDIRECT_CELL , (size - iter_size) / JER2_ARAD_CELL_NOF_BYTES_IN_UINT32 );
      }
    }
    else /* Simple source routed cell */
    {
      nof_words = 1;
    }
    if (is_inband)
    {
      /*
       * Construction of the cell information
       */
      res = jer2_arad_build_data_cell_for_fe600(
              unit,
              sr_link_list,
              offset,
              data_in + start_long,
              nof_words,
              is_write,
              is_indirect,
              is_inband,
              &data_cell_sent
            );
      DNX_SAND_CHECK_FUNC_RESULT(res, 10, exit);

      /*
       * Sending and receiving the cell
       */
      res = jer2_arad_sr_send_and_wait_ack(
              unit,
              NULL,
              &data_cell_sent,
              FALSE,
              0,
              NULL,
              &data_cell_rcv
            );
      DNX_SAND_CHECK_FUNC_RESULT(res, 150, exit);

      if (
          data_cell_sent.data_cell.source_routed.cell_ident
          != data_cell_rcv.data_cell.source_routed.cell_ident
         )
      {
        DNX_SAND_SET_ERROR_CODE(JER2_ARAD_CELL_DIFFERENT_CELL_IDENT_ERR, 153, exit);
      }

    }
    else if (!is_write) /* Read for a regular source routed cell*/
    {
       res = jer2_arad_sr_rcv_cell(
              unit,
              FALSE,
              is_inband,
              &data_cell_rcv,
              NULL,
              NULL,
              &success
            );
      DNX_SAND_CHECK_FUNC_RESULT(res, 152, exit);

      if (success == FALSE)
      {
        DNX_SAND_SET_ERROR_CODE(JER2_ARAD_CELL_NO_RECEIVED_CELL_ERR, 155, exit);
      }


    }
    else /* Write for a regular source routed cell*/
    {
      /*
       * Construction of the cell information
       */
      res = jer2_arad_build_data_cell_for_fe600(
              unit,
              sr_link_list,
              offset,
              data_in + start_long,
              nof_words,
              is_write,
              is_indirect,
              is_inband,
              &data_cell_sent
            );
      DNX_SAND_CHECK_FUNC_RESULT(res, 155, exit);

      res = jer2_arad_sr_send_cell(
              unit,
              FALSE,
              0,
              FALSE,
              &data_cell_sent,
              NULL
            );
      DNX_SAND_CHECK_FUNC_RESULT(res, 158, exit);
    }

    /*
     * Construction of the output in case of read
     */
    if (is_inband)
    {
      if(!is_write)
      {
        if(is_indirect) /* Indirect read */
        {
          if(start_long != 0)
          {
            DNX_SAND_SET_ERROR_CODE(JER2_ARAD_CELL_WRITE_OUT_OF_BOUNDARY,160,exit);
          }

          tmp_data_out[start_long] =
            data_cell_rcv.data_cell.source_routed.data_wr_cell[JER2_ARAD_CELL_WRITE_POSITION_0] ;
          tmp_data_out[start_long + 1] =
            data_cell_rcv.data_cell.source_routed.data_wr_cell[JER2_ARAD_CELL_WRITE_POSITION_1] ;
          tmp_data_out[start_long + 2] =
            data_cell_rcv.data_cell.source_routed.data_wr_cell[JER2_ARAD_CELL_WRITE_POSITION_2] ;
        }
        else /* Direct read */
        {
          if(start_long >= JER2_ARAD_CELL_NOF_DATA_WORDS_IN_INDIRECT_CELL)
          {
            DNX_SAND_SET_ERROR_CODE(JER2_ARAD_CELL_WRITE_OUT_OF_BOUNDARY,162,exit);
          }

          tmp_data_out[start_long] = data_cell_rcv.data_cell.source_routed.data_wr_cell[JER2_ARAD_CELL_WRITE_POSITION_0] ;
        }
      }
    }
    else if (!is_write) /* Regular source routed cell for read, copy of the payload */
    {
      if (start_long > DNX_SAND_DATA_CELL_PAYLOAD_IN_UINT32S - DNX_SAND_SR_DATA_CELL_NOF_WORDS_IN_PAYLOAD) {
         DNX_SAND_SET_ERROR_CODE(JER2_ARAD_CELL_WRITE_OUT_OF_BOUNDARY,162,exit);
      }
      for (iter = 0 ; iter < DNX_SAND_SR_DATA_CELL_NOF_WORDS_IN_PAYLOAD ; ++iter)
      {        
        tmp_not_inband_data_out[start_long + iter] = data_cell_rcv.data_cell.source_routed.data_wr_cell[iter] ;
      }

      for (iter = 0 ; iter < DNX_SAND_SR_DATA_CELL_NOF_WORDS_IN_PAYLOAD ; ++iter)
      {
        tmp_field_for_data_cell_add_fields = data_cell_rcv.data_cell.source_routed.add_wr_cell[iter];

        res = dnx_sand_bitstream_set_any_field(
                &(tmp_field_for_data_cell_add_fields),
                DNX_SAND_SR_DATA_CELL_ADDRESS_LENGTH * iter,
                DNX_SAND_SR_DATA_CELL_ADDRESS_LENGTH,
                &(tmp_not_inband_data_out[start_long + DNX_SAND_SR_DATA_CELL_NOF_WORDS_IN_PAYLOAD])
              );
        DNX_SAND_CHECK_FUNC_RESULT(res, 160, exit);
      }

      tmp_field_for_data_cell_add_fields = data_cell_rcv.data_cell.source_routed.cell_ident;

      res = dnx_sand_bitstream_set_any_field(
              &(tmp_field_for_data_cell_add_fields),
              DNX_SAND_SR_DATA_CELL_ADDRESS_LENGTH * DNX_SAND_SR_DATA_CELL_NOF_WORDS_IN_PAYLOAD,
              DNX_SAND_SR_DATA_CELL_NOT_INBAND_CELL_IDENT_LENGTH,
              &(tmp_not_inband_data_out[start_long + DNX_SAND_SR_DATA_CELL_NOF_WORDS_IN_PAYLOAD])
            );
      DNX_SAND_CHECK_FUNC_RESULT(res, 170, exit);

    }

    if (is_inband)
    {
      if( ( !is_write ) && ( !is_indirect ) ) /* Read direct: one word per cell*/
      {
        ++start_long  ;
        iter_size = iter_size + JER2_ARAD_CELL_NOF_BYTES_IN_UINT32 ;
      }
      else if( ( is_write ) && ( !is_indirect ) ) /* Write direct: five words per cell */
      {
        start_long = start_long + DNX_SAND_SR_DATA_CELL_NOF_WORDS_IN_PAYLOAD ;
        iter_size = iter_size + JER2_ARAD_CELL_NOF_BYTES_IN_UINT32 * DNX_SAND_SR_DATA_CELL_NOF_WORDS_IN_PAYLOAD ;
      }
      else /* Indirect R/W: three words of data_in per cell */
      {
        start_long = start_long + JER2_ARAD_CELL_NOF_DATA_WORDS_IN_INDIRECT_CELL ;
        iter_size = iter_size + JER2_ARAD_CELL_NOF_BYTES_IN_UINT32 * JER2_ARAD_CELL_NOF_DATA_WORDS_IN_INDIRECT_CELL ;
      }
    }
    else
    {
      start_long += DNX_SAND_DATA_CELL_PAYLOAD_IN_UINT32S;
      iter_size = iter_size + JER2_ARAD_CELL_NOF_BYTES_IN_UINT32 * DNX_SAND_DATA_CELL_PAYLOAD_IN_UINT32S ;
    }
  }
  /*
   * Construction of the data_out in case of reading
   */
  if(
     (!is_write)
     && (is_inband)
    )
  {
    for (iter = 0 ; iter < start_long ; ++iter)
    {
      data_out[iter] = tmp_data_out[iter];
    }
  }
  else if(
          (!is_inband)
          && (!is_write)
         )
  {
    for (iter = 0 ; iter < start_long ; ++iter)
    {
      data_out[iter] = tmp_not_inband_data_out[iter];
    }
  }

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_transaction_with_fe600()",0,0);
}

/*
 * Allows the cpu to generate a direct read command
 */
uint32
  jer2_arad_read_from_fe600_unsafe(
    DNX_SAND_IN  int                       unit,
    DNX_SAND_IN  DNX_TMC_SR_CELL_LINK_LIST         *sr_link_list,
    DNX_SAND_IN  uint32                        size,
    DNX_SAND_IN  uint32                        offset,
    DNX_SAND_OUT uint32                        *data_out
  )
{
  uint32
    res = DNX_SAND_OK;

  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_READ_FROM_FE600_UNSAFE);

  /*
   * Builds the data, packs to a buffer, sends the cell,
   * receives the buffer, and decomposes it to the output.
   */
  res = jer2_arad_transaction_with_fe600(
          unit,
          sr_link_list,
          offset,
          NULL,
          size,
          0,
          0,
          1,
          data_out
        );
  DNX_SAND_CHECK_FUNC_RESULT(res, 10, exit);

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_read_from_fe600_unsafe()",0,0);
}
/*
 * Allows the cpu to generate a direct write command
 */
uint32
  jer2_arad_write_to_fe600_unsafe(
    DNX_SAND_IN  int                       unit,
    DNX_SAND_IN  DNX_TMC_SR_CELL_LINK_LIST        *sr_link_list,
    DNX_SAND_IN  uint32                        size,
    DNX_SAND_IN  uint32                        offset,
    DNX_SAND_IN  uint32                        *data_in
  )
{
  uint32
    res = DNX_SAND_OK;

  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_WRITE_FROM_FE600_UNSAFE);

  /*
   * Construction of the read direct cell information
   */
  /*
   * Builds the data, packs to a buffer, sends the cell,
   * receives the buffer, and decomposes it to the output.
   */
  res = jer2_arad_transaction_with_fe600(
          unit,
          sr_link_list,
          offset,
          data_in,
          size,
          1,
          0,
          1,
          NULL
        );
  DNX_SAND_CHECK_FUNC_RESULT(res, 10, exit);

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_write_to_fe600_unsafe()",0,0);
}


/*
 * Allows the cpu to generate a indirect read command
 */
uint32
  jer2_arad_indirect_read_from_fe600_unsafe(
    DNX_SAND_IN  int                       unit,
    DNX_SAND_IN  DNX_TMC_SR_CELL_LINK_LIST          *sr_link_list,
    DNX_SAND_IN  uint32                        size,
    DNX_SAND_IN  uint32                        offset,
    DNX_SAND_OUT uint32                        *data_out
  )
{
  uint32
    res = DNX_SAND_OK;

  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_INDIRECT_READ_FROM_FE600_UNSAFE);

  /*
   * Builds the data cell, packs to a buffer, sends the cell,
   * receives the buffer, and decomposes it to the output.
   */
  res = jer2_arad_transaction_with_fe600(
          unit,
          sr_link_list,
          offset,
          NULL,
          size ,
          0,
          1,
          1,
          data_out
        );
  DNX_SAND_CHECK_FUNC_RESULT(res, 10, exit);

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_indirect_read_from_fe600_unsafe()",0,0);

}


/*
 * Allows the cpu to generate a indirect write command
 */
uint32
  jer2_arad_indirect_write_to_fe600_unsafe(
    DNX_SAND_IN  int                       unit,
    DNX_SAND_IN  DNX_TMC_SR_CELL_LINK_LIST         *sr_link_list,
    DNX_SAND_IN  uint32                        size,
    DNX_SAND_IN  uint32                        offset,
    DNX_SAND_IN  uint32                        *data_in
  )
{
  uint32
    res = DNX_SAND_OK ;

  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_INDIRECT_WRITE_FROM_FE600_UNSAFE);

  /*
   * Builds the data cell, packs to a buffer, sends the cell,
   * receives the buffer, and decomposes it to the output.
   */
  res = jer2_arad_transaction_with_fe600(
          unit,
          sr_link_list,
          offset,
          data_in,
          size ,
          1,
          1,
          1,
          NULL
        );
  DNX_SAND_CHECK_FUNC_RESULT(res, 10, exit);


exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_indirect_write_fe600_unsafe()",0,0);

}

/*
 * Allows the cpu to generate an interaction with a CPU of a DNX_SAND_FE600
 */
uint32
  jer2_arad_cpu2cpu_write_unsafe(
    DNX_SAND_IN  int                       unit,
    DNX_SAND_IN  DNX_TMC_SR_CELL_LINK_LIST          *sr_link_list,
    DNX_SAND_IN  uint32                        size,
    DNX_SAND_IN  uint32                        *data_in
  )
{
  uint32
    res = DNX_SAND_OK;

  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_CPU2CPU_WITH_FE600_WRITE_UNSAFE);

  /*
   * Builds the data cell, packs to a buffer, sends the cell,
   * receives the buffer, and decomposes it to the output.
   */
  res = jer2_arad_transaction_with_fe600(
          unit,
          sr_link_list,
          0,
          data_in,
          size,
          1,
          0,
          0,
          NULL
        );
  DNX_SAND_CHECK_FUNC_RESULT(res, 10, exit);

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_cpu2cpu_write_unsafe()",0,0);

}


/*
 * Allows the cpu to generate an interaction with a CPU of a DNX_SAND_FE600
 */
uint32
  jer2_arad_cpu2cpu_read_unsafe(
    DNX_SAND_IN  int                       unit,
    DNX_SAND_OUT uint32                        *data_out
  )
{
  uint32
    res = DNX_SAND_OK;

  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_CPU2CPU_WITH_FE600_READ_UNSAFE);

  /*
   * Builds the data cell, packs to a buffer, sends the cell,
   * receives the buffer, and decomposes it to the output.
   */
  res = jer2_arad_transaction_with_fe600(
          unit,
          NULL,
          0,
          NULL,
          DNX_SAND_DATA_CELL_PAYLOAD_IN_BYTES,
          0,
          0,
          0,
          data_out
        );
  DNX_SAND_CHECK_FUNC_RESULT(res, 10, exit);

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_cpu2cpu_read_unsafe()",0,0);

}


/*
 * Sends a cell and waits for an ack
 */
uint32
  jer2_arad_sr_send_and_wait_ack(
    DNX_SAND_IN  int                        unit,
    DNX_SAND_IN  dnxc_vsc256_sr_cell_t              *data_cell_sent,
    DNX_SAND_IN  DNX_SAND_DATA_CELL            *data_cell_sent_old,
    DNX_SAND_IN  uint32                        route_group_id,
    DNX_SAND_IN  uint8                         is_group,
    DNX_SAND_OUT dnxc_vsc256_sr_cell_t              *data_cell_rcv,
    DNX_SAND_OUT DNX_SAND_DATA_CELL            *data_cell_rcv_old
  )
{/* It may recieve the adequate structerers;*/
  uint8
    success = FALSE;
  uint32
    try_i = 0 ;
  uint32
    res = DNX_SAND_OK;

  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_SR_SEND_AND_WAIT_ACK);

  if(data_cell_sent) {
    DNX_SAND_CHECK_NULL_INPUT(data_cell_rcv);
    res = jer2_arad_sr_send_cell(
            unit,
            TRUE,
            route_group_id,
            is_group,
            NULL,
            data_cell_sent
          );
  }
  else{
    DNX_SAND_CHECK_NULL_INPUT(data_cell_sent_old);
    DNX_SAND_CHECK_NULL_INPUT(data_cell_rcv_old);
    res = jer2_arad_sr_send_cell(
            unit,
            FALSE,
            0,
            FALSE,
            data_cell_sent_old,
            NULL
          );
  }
  DNX_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  do
  {
    if(data_cell_sent) {
      res = jer2_arad_sr_rcv_cell(
              unit,
              TRUE,
              FALSE,
              NULL,
              data_cell_rcv,
              NULL,
              &success
            );
    }
    else{
      res = jer2_arad_sr_rcv_cell(
              unit,
              FALSE,
              FALSE,
              data_cell_rcv_old,
              NULL,
              NULL,
              &success
            );
    }
    DNX_SAND_CHECK_FUNC_RESULT(res, 20, exit);

  } while((success == FALSE) && (try_i++ < JER2_ARAD_CELL_MAX_NOF_TRIES_WAITING_FOR_ACK));

  if (try_i >= JER2_ARAD_CELL_MAX_NOF_TRIES_WAITING_FOR_ACK)
  {
    DNX_SAND_SET_ERROR_CODE(JER2_ARAD_CELL_NO_RECEIVED_CELL_ERR, 20, exit);
  }

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_send_and_wait_ack()",0,0);
}


uint32
  jer2_arad_fabric_cell_cpu_data_get(
    DNX_SAND_IN  int                     unit,
    DNX_SAND_OUT uint32                  *cell_buffer
  )
{
    uint8 success;
    DNXC_INIT_FUNC_DEFS;

    DNXC_SAND_IF_ERR_EXIT(jer2_arad_cell_ack_get(unit, FALSE, cell_buffer, &success));

    if (success == FALSE){
        DNXC_EXIT_WITH_ERR(SOC_E_EMPTY, (_BSL_DNXC_MSG("No cell was recieved\n")));
        /*LOG_INFO(BSL_LS_SOC_FABRIC,(BSL_META_U(unit, "No cell was recieved\n")));*/
    }
exit:
    DNXC_FUNC_RETURN;
}


static uint32
  jer2_arad_sr_cell_send(
    DNX_SAND_IN  int                unit, 
    DNX_SAND_IN  dnxc_vsc256_sr_cell_t   *data_cell_sent
  )
{
    uint32 packed_cpu_data_cell_sent[JER2_ARAD_DATA_CELL_U32_SIZE];
    uint32 tmp_u32_fip_switch = 0, tmp_u32_output_link_in_five_bits = 0, tmp_reg;
    DNXC_INIT_FUNC_DEFS;

    sal_memset(packed_cpu_data_cell_sent, 0x0, JER2_ARAD_DATA_CELL_BYTE_SIZE);
  
    /*Pack the cell*/
    DNXC_IF_ERR_EXIT(jer2_arad_sr_data_cell_to_fe1600_buffer(unit, data_cell_sent, packed_cpu_data_cell_sent));
 
    /*Copy the data*/
    DNXC_IF_ERR_EXIT(WRITE_FDT_CPU_DATA_CELL_0r(unit, packed_cpu_data_cell_sent));
    DNXC_IF_ERR_EXIT(WRITE_FDT_CPU_DATA_CELL_1r(unit, packed_cpu_data_cell_sent + 256 / (8 * sizeof(uint32))));
    DNXC_IF_ERR_EXIT(WRITE_FDT_CPU_DATA_CELL_2r(unit, packed_cpu_data_cell_sent + 2 * 256 / (8 * sizeof(uint32))));
    DNXC_IF_ERR_EXIT(WRITE_FDT_CPU_DATA_CELL_3r(unit, packed_cpu_data_cell_sent + 3 * 256 / (8 * sizeof(uint32))));
 
    /*Fix the output link which is the fip switch link*/
    tmp_u32_fip_switch = data_cell_sent->header.fip_link;

    DNXC_SAND_IF_ERR_EXIT(dnx_sand_bitstream_set_any_field(&(tmp_u32_fip_switch), 0, DNX_SAND_DATA_CELL_FIP_SWITCH_LENGTH+1, &tmp_u32_output_link_in_five_bits));

    DNXC_IF_ERR_EXIT(READ_FDT_TRANSMIT_CELL_OUTPUT_LINK_NUMBERr(unit, &tmp_reg));
    soc_reg_field_set(unit, FDT_TRANSMIT_CELL_OUTPUT_LINK_NUMBERr, &tmp_reg, CPU_LINK_NUMf, tmp_u32_output_link_in_five_bits);
    DNXC_IF_ERR_EXIT(WRITE_FDT_TRANSMIT_CELL_OUTPUT_LINK_NUMBERr(unit, tmp_reg));

    tmp_reg = 0;
    DNXC_IF_ERR_EXIT(READ_FDT_TRANSMIT_DATA_CELL_TRIGGERr(unit, &tmp_reg));
    soc_reg_field_set(unit, FDT_TRANSMIT_DATA_CELL_TRIGGERr, &tmp_reg, CPU_TRGf, 0x1);
    DNXC_IF_ERR_EXIT(WRITE_FDT_TRANSMIT_DATA_CELL_TRIGGERr(unit, tmp_reg));

exit:
    DNXC_FUNC_RETURN;
}

uint32
  jer2_arad_fabric_cpu2cpu_write(    
    DNX_SAND_IN  int                                    unit,
    DNX_SAND_IN dnxc_sr_cell_link_list_t                *sr_link_list,
    DNX_SAND_IN uint32                                  data_in_size,
    DNX_SAND_IN uint32                                  *data_in
  )
{
    dnxc_vsc256_sr_cell_t cell;
    DNXC_INIT_FUNC_DEFS;

    sal_memset(&cell.payload.data[0], 0x0, sizeof(uint32)*DNXC_VSC256_SR_DATA_CELL_PAYLOAD_MAX_LENGTH_U32);

    cell.header.cell_type = soc_dnxc_fabric_cell_type_sr_cell;
    cell.header.source_device = (uint32) unit;
    cell.header.source_level = dnxcFabricDeviceTypeFIP;
    cell.header.destination_level = sr_link_list->dest_entity_type;
    cell.header.fip_link = sr_link_list->path_links[JER2_ARAD_CELL_PATH_LINK_FIP_SWITCH_POSITION];
    cell.header.fe1_link = sr_link_list->path_links[JER2_ARAD_CELL_PATH_LINK_FE1_SWITCH_POSITION];
    cell.header.fe2_link = sr_link_list->path_links[JER2_ARAD_CELL_PATH_LINK_FE2_SWITCH_POSITION];
    cell.header.fe3_link = sr_link_list->path_links[JER2_ARAD_CELL_PATH_LINK_FE3_SWITCH_POSITION];
    cell.header.ack = 0;
    cell.header.is_inband = 0;
    cell.header.pipe_id = sr_link_list->pipe_id; 

    sal_memcpy(cell.payload.data, data_in, 
               data_in_size < DNXC_VSC256_SR_DATA_CELL_PAYLOAD_MAX_LENGTH_U32 ? WORDS2BYTES(data_in_size) : WORDS2BYTES(DNXC_VSC256_SR_DATA_CELL_PAYLOAD_MAX_LENGTH_U32));

    DNXC_IF_ERR_EXIT(jer2_arad_sr_cell_send(unit, &cell));

exit:
      DNXC_FUNC_RETURN;
}


#undef _ERR_MSG_MODULE_NAME

#endif /* of #if defined(BCM_88690_A0) */

