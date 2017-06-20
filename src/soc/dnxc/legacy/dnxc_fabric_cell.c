/*
 * $Id: dnxc_fabric_cell.c,v 1.13 Broadcom SDK $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */

#ifdef _ERR_MSG_MODULE_NAME
  #error "_ERR_MSG_MODULE_NAME redefined"
#endif

#define _ERR_MSG_MODULE_NAME BSL_SOC_FABRIC

#include <shared/bsl.h>

#include <soc/dnxc/legacy/error.h>

#include <soc/dnxc/legacy/dnxc_fabric_cell.h>
#ifdef BCM_DNXF_SUPPORT
#include <soc/dnxf/cmn/dnxf_drv.h>
#endif /* BCM_DNXF_SUPPORT */
#ifdef BCM_DNX_SUPPORT
#include <soc/dnx/legacy/drv.h>
#endif /* BCM_DNX_SUPPORT */
#ifdef BCM_DNX_SUPPORT
#include <soc/dnx/drv.h>
#endif /* BCM_DNX_SUPPORT */

int
dnxc_soc_inband_route_set(
                     int unit,
                     int route_id,
                     dnxc_soc_fabric_inband_route_t* route
                     )
{
  DNXC_INIT_FUNC_DEFS;

  if (!SOC_UNIT_VALID(unit)) {
	  DNXC_EXIT_WITH_ERR(SOC_E_UNIT, (_BSL_DNXC_MSG("Invalid unit")));
  }

#ifdef BCM_DNX_SUPPORT
  if(SOC_IS_DNX(unit))
  {
    DNXC_IF_ERR_EXIT
		(soc_dnx_fabric_inband_route_set(unit, route_id, route));
  }
#endif
exit:
  DNXC_FUNC_RETURN;
}

int
dnxc_soc_inband_route_get(
                     int unit,
                     int route_id,
                     dnxc_soc_fabric_inband_route_t* route
                     )
{
  DNXC_INIT_FUNC_DEFS;

  if (!SOC_UNIT_VALID(unit)) {
	  DNXC_EXIT_WITH_ERR(SOC_E_UNIT, (_BSL_DNXC_MSG("Invalid unit")));
  }
#ifdef BCM_DNX_SUPPORT
  if(SOC_IS_DNX(unit))
  {
    DNXC_IF_ERR_EXIT
		(soc_dnx_fabric_inband_route_get(unit, route_id, route));
  }
#endif

exit:
  DNXC_FUNC_RETURN;
}

int
dnxc_soc_inband_route_group_set(
                           int unit,
                           int group_id,
                           int flags,
                           int route_count,
                           int *route_ids
                           )
{
#ifdef BCM_DNX_SUPPORT
  if(SOC_IS_DNX(unit))
  {
    return soc_dnx_fabric_inband_route_group_set(unit, group_id, flags, route_count, route_ids);
  }
#endif
  return SOC_E_UNIT;
}

int
dnxc_soc_inband_route_group_get(
                           int unit,
                           int group_id,
                           int flags,
                           int route_count_max,
                           int *route_count,
                           int *route_ids
                           )
{
#ifdef BCM_DNX_SUPPORT
  if(SOC_IS_DNX(unit))
  {
    return soc_dnx_fabric_inband_route_group_get(unit, group_id, flags, route_count_max, route_count, route_ids);
  }
#endif
  return SOC_E_UNIT;
}

int 
dnxc_soc_inband_mem_read(
                        int unit, 
                        uint32 flags, 
                        int route_id, 
                        int max_count,  /*number of memories*/
                        soc_mem_t *mem, 
                        int *copyno,
                        int *index,
                        void **entry_data, 
                        int *array_count
)
{
#ifdef BCM_DNX_SUPPORT
  if(SOC_IS_DNX(unit))
  {
    return soc_dnx_fabric_mem_read(unit, flags, route_id, max_count, mem, copyno, index, entry_data, array_count);
  }
#endif
  return SOC_E_UNIT;
}

int
dnxc_soc_inband_mem_write(
                         int unit, 
                         uint32 flags, 
                         int route_id, 
                         int array_count,
                         soc_mem_t *mem, 
                         int *copyno,
                         int *index,
                         void **entry_data 
)
{
#ifdef BCM_DNX_SUPPORT
  if(SOC_IS_DNX(unit))
  {
    return soc_dnx_fabric_mem_write(unit, flags, route_id, array_count, mem, copyno, index, entry_data);
  }
#endif
  return SOC_E_UNIT;
} 

int 
dnxc_soc_inband_reg_read(
                        int unit, 
                        uint32 flags, 
                        int route_id,
                        int max_count, /* The number of registers*/
                        soc_reg_t *reg, 
                        int *port,
                        int *index,
                        uint64 *data, 
                        int *array_count
)
{
#ifdef BCM_DNX_SUPPORT
  if(SOC_IS_DNX(unit))
  {
    return soc_dnx_fabric_reg_read(unit, flags, route_id, max_count, reg, port, index, data, array_count);
  }
#endif
  return SOC_E_UNIT;
}

int 
dnxc_soc_inband_reg_write(
                         int unit, 
                         uint32 flags, 
                         int route_id,
                         int array_count, 
                         soc_reg_t *reg, 
                         int *port,
                         int *index,
                         uint64 *data
)
{
#ifdef BCM_DNX_SUPPORT
  if(SOC_IS_DNX(unit))
  {
    return soc_dnx_fabric_reg_write(unit, flags, route_id, array_count, reg, port, index, data);
  }
#endif
  return SOC_E_UNIT;
}

int
dnxc_soc_inband_reg_above_64_read(
                                 int unit, uint32 flags,
                                 int route_id,
                                 int max_count,
                                 soc_reg_t *reg,
                                 int *port,
                                 int *index,
                                 soc_reg_above_64_val_t *data,
                                 int *array_count
)
{
#ifdef BCM_DNX_SUPPORT
  if(SOC_IS_DNX(unit))
  {
    return soc_dnx_fabric_reg_above_64_read(unit, flags, route_id, max_count, reg, port, index, data, array_count);
  }
#endif
  return SOC_E_UNIT;
}

int
dnxc_soc_inband_reg_above_64_write(
                                  int unit, uint32 flags,
                                  int route_id, int array_count,
                                  soc_reg_t *reg,
                                  int *port,
                                  int *index,
                                  soc_reg_above_64_val_t *data
)
{
#ifdef BCM_DNX_SUPPORT
  if(SOC_IS_DNX(unit))
  {
    return soc_dnx_fabric_reg_above_64_write(unit, flags, route_id, array_count, reg, port, index, data);
  }
#endif
  return SOC_E_UNIT;
}

soc_error_t 
soc_dnxc_parse_captured_cell(int unit, soc_reg_above_64_val_t reg_val, dnxc_captured_cell_t *captured_cell)
{
    uint32 val = 0;
    DNXC_INIT_FUNC_DEFS;

    sal_memset(captured_cell, 0, sizeof(dnxc_captured_cell_t));
    /* parsing of data cell (in this stage in the pipe the parsing is the same for VSC256 and VSC128 */
#define DNXC_PARSING_DATA_CELL_SOURCE_DEVICE_START 543
#define DNXC_PARSING_DATA_CELL_SOURCE_DEVICE_LENGTH 11
    SHR_BITCOPY_RANGE(&val, 0, reg_val, DNXC_PARSING_DATA_CELL_SOURCE_DEVICE_START, DNXC_PARSING_DATA_CELL_SOURCE_DEVICE_LENGTH);
    captured_cell->source_device = val;
#define DNXC_PARSING_DATA_CELL_DEST_START 554
#if 1
#define DNXC_PARSING_DATA_CELL_DEST_LENGTH 16
#endif
    val = 0;
    SHR_BITCOPY_RANGE(&val, 0, reg_val, DNXC_PARSING_DATA_CELL_DEST_START, DNXC_PARSING_DATA_CELL_DEST_LENGTH);
    captured_cell->dest = val;
#define DNXC_PARSING_DATA_CELL_FIRST_CELL_IND_START 536
#define DNXC_PARSING_DATA_CELL_FIRST_CELL_IND_LENGTH 1
    val = 0;
    SHR_BITCOPY_RANGE(&val, 0, reg_val, DNXC_PARSING_DATA_CELL_FIRST_CELL_IND_START, DNXC_PARSING_DATA_CELL_FIRST_CELL_IND_LENGTH);
    captured_cell->is_last_cell = val;

    DNXC_FUNC_RETURN; 
}

static soc_error_t 
soc_dnxc_fabric_cell_table_dest_fill(int unit, soc_dnxc_fabric_cell_parse_table_t *parse_table, uint32 nof_lines, soc_dnxc_fabric_cell_info_t *cell_info)
{
    int line;
    DNXC_INIT_FUNC_DEFS;

    for (line = 0; line < nof_lines; line++)
    {
        switch (parse_table[line].dest_id)
        {
            case soc_dnxc_fabric_cell_dest_id_src_cell_type:
                parse_table[line].dest = (uint32 *) &cell_info->cell.sr_cell.header.cell_type;
                break;
            case soc_dnxc_fabric_cell_dest_id_src_src_device:
                parse_table[line].dest = (uint32 *) &cell_info->cell.sr_cell.header.source_device;
                break;
            case soc_dnxc_fabric_cell_dest_id_src_src_level:
                parse_table[line].dest = (uint32 *) &cell_info->cell.sr_cell.header.source_level;
                break;
            case soc_dnxc_fabric_cell_dest_id_src_dest_level:
                parse_table[line].dest = (uint32 *) &cell_info->cell.sr_cell.header.destination_level;
                break;
            case soc_dnxc_fabric_cell_dest_id_src_fip:
                parse_table[line].dest = (uint32 *) &cell_info->cell.sr_cell.header.fip_link;
                break;
            case soc_dnxc_fabric_cell_dest_id_src_fe1:
                parse_table[line].dest = (uint32 *) &cell_info->cell.sr_cell.header.fe1_link;
                break;
            case soc_dnxc_fabric_cell_dest_id_src_fe2:
                parse_table[line].dest = (uint32 *) &cell_info->cell.sr_cell.header.fe2_link;
                break;
            case soc_dnxc_fabric_cell_dest_id_src_fe3:
                parse_table[line].dest = (uint32 *) &cell_info->cell.sr_cell.header.fe3_link;
                break;
            case soc_dnxc_fabric_cell_dest_id_src_is_inband:
                parse_table[line].dest = (uint32 *) &cell_info->cell.sr_cell.header.is_inband;
                break;
            case soc_dnxc_fabric_cell_dest_id_src_ack:
                parse_table[line].dest = (uint32 *) &cell_info->cell.sr_cell.header.ack;
                break;
           case soc_dnxc_fabric_cell_dest_id_src_pipe_id:
                parse_table[line].dest = (uint32 *) &cell_info->cell.sr_cell.header.pipe_id;
                break;
            case soc_dnxc_fabric_cell_dest_id_src_payload:
                parse_table[line].dest = (uint32 *) &cell_info->cell.sr_cell.payload.data;
                break;
            case soc_dnxc_fabric_cell_dest_id_cap_cell_type:
                parse_table[line].dest = (uint32 *) &cell_info->cell.captured_cell.cell_type;
                break;
            case soc_dnxc_fabric_cell_dest_id_cap_src_device:
                parse_table[line].dest = (uint32 *) &cell_info->cell.captured_cell.source_device;
                break;
            case soc_dnxc_fabric_cell_dest_id_cap_cell_dst:
                parse_table[line].dest = (uint32 *) &cell_info->cell.captured_cell.dest;
                break;
            case soc_dnxc_fabric_cell_dest_id_cap_cell_ind:
                parse_table[line].dest = (uint32 *) &cell_info->cell.captured_cell.is_last_cell;
                break;
            case soc_dnxc_fabric_cell_dest_id_cap_cell_size:
                parse_table[line].dest = (uint32 *) &cell_info->cell.captured_cell.cell_size;
                break;
            case soc_dnxc_fabric_cell_dest_id_cap_cell_pipe_index:
                parse_table[line].dest = (uint32 *) &cell_info->cell.captured_cell.pipe_index;
                break;
            case soc_dnxc_fabric_cell_dest_id_cap_cell_payload:
                parse_table[line].dest = (uint32 *) cell_info->cell.captured_cell.data; /* Shall we have & here before cell_info ? */
                break;
            case soc_dnxc_fabric_cell_dest_id_cap_cell_payload_2:
                parse_table[line].dest = (uint32 *) cell_info->cell.captured_cell.data;
                break;

            default:
                DNXC_EXIT_WITH_ERR(SOC_E_UNAVAIL, (_BSL_DNXC_MSG_STR("UNAVAIL line id")));
                break;
        }
        
    }

exit:
    DNXC_FUNC_RETURN;
}

soc_error_t
soc_dnxc_fabric_cell_parser(int unit, soc_dnxc_fabric_cell_entry_t entry, soc_dnxc_fabric_cell_entry_t entry_2 , soc_dnxc_fabric_cell_parse_table_t *parse_table, uint32 nof_lines, soc_dnxc_fabric_cell_info_t *cell_info, int is_two_parts)
{
    int line,
        nof_uint32,
        i,
        rv;
    soc_dnxc_fabric_cell_entry_t val;
    DNXC_INIT_FUNC_DEFS;

    rv = soc_dnxc_fabric_cell_table_dest_fill(unit, parse_table, nof_lines, cell_info);
    DNXC_IF_ERR_EXIT(rv);
    
    sal_memset(val, 0x00, sizeof(soc_dnxc_fabric_cell_entry_t));

    LOG_DEBUG(BSL_LS_SOC_PKT,
              (BSL_META_U(unit,
                          "Received Cell Parse\n")));
    LOG_DEBUG(BSL_LS_SOC_PKT,
              (BSL_META_U(unit,
                          "-------------------\n")));
    LOG_DEBUG(BSL_LS_SOC_PKT,
              (BSL_META_U(unit,
                          "Entry print: ")));
    for (i = 0; i < SOC_DNXC_FABRIC_CELL_ENTRY_MAX_SIZE_UINT32; i++)
    {
        LOG_DEBUG(BSL_LS_SOC_PKT,
                  (BSL_META_U(unit,
                              "0x%08x, "), entry[i]));
    }
    LOG_DEBUG(BSL_LS_SOC_PKT,
              (BSL_META_U(unit,
                          "\n")));

    for (line = 0; line < nof_lines; line++)
    {
        /*Copy src to val*/
        if ((parse_table[line].dest_id == soc_dnxc_fabric_cell_dest_id_cap_cell_payload_2) && is_two_parts)
        {
            SHR_BITCOPY_RANGE(val, 0, entry_2, parse_table[line].src_start_bit, parse_table[line].length);
        }
        else 
        {
            
            SHR_BITCOPY_RANGE(val, 0, entry, parse_table[line].src_start_bit, parse_table[line].length); /* Check if we need to read from entry2, double cell */
        }

        /*print val*/
        nof_uint32 = (parse_table[line].length + 31) / 32; /*round up*/
        if(nof_uint32 == 1)
        {
          LOG_DEBUG(BSL_LS_SOC_PKT,
                    (BSL_META_U(unit,
                                "%s: 0x%x"), parse_table[line].field_name, *val));
        } else 
        {
          LOG_DEBUG(BSL_LS_SOC_PKT,
                    (BSL_META_U(unit,
                                "%s: 0x%08x"), parse_table[line].field_name, *val));
          for (i = 1; i < nof_uint32; i++)
          {
              LOG_DEBUG(BSL_LS_SOC_PKT,
                        (BSL_META_U(unit,
                                    ", 0x%08x"), val[i]));
          }
        }
        LOG_DEBUG(BSL_LS_SOC_PKT,
                  (BSL_META_U(unit,
                              "\n")));

        /*Copy val to dest*/
        SHR_BITCOPY_RANGE(parse_table[line].dest, parse_table[line].dest_start_bit, val, 0, parse_table[line].length);  

        SHR_BITCLR_RANGE(val, 0, parse_table[line].length);
    }
    LOG_DEBUG(BSL_LS_SOC_PKT,
              (BSL_META_U(unit,
                          "-------------------\n\n")));

exit:
    DNXC_FUNC_RETURN; 
}

#undef _ERR_MSG_MODULE_NAME

