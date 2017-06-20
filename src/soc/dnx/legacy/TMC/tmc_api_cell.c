/* $Id: jer2_tmc_api_cell.c,v 1.7 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/
/******************************************************************
*
* FILENAME:       DuneDriver/jer2_tmc/src/soc_jer2_tmcapi_cell.c
*
* MODULE PREFIX:  jer2_tmc
*
* FILE DESCRIPTION:
*
* REMARKS:
* SW License Agreement: Dune Networks (c). CONFIDENTIAL PROPRIETARY INFORMATION.
* Any use of this Software is subject to Software License Agreement
* included in the Driver User Manual of this device.
* Any use of this Software constitutes an agreement to the terms
* of the above Software License Agreement.
******************************************************************/

/*************
 * INCLUDES  *
 *************/
/* { */



#include <shared/bsl.h>

#include <soc/dnx/legacy/SAND/Utils/sand_header.h>

#include <soc/dnx/legacy/SAND/Management/sand_general_macros.h>
#include <soc/dnx/legacy/SAND/Utils/sand_os_interface.h>
#include <soc/dnx/legacy/SAND/Management/sand_error_code.h>
#include <soc/dnx/legacy/SAND/SAND_FM/sand_cell.h>
#include <soc/dnx/legacy/SAND/Management/sand_low_level.h>

#include <soc/dnx/legacy/TMC/tmc_api_cell.h>

/* } */

/*************
 * DEFINES   *
 *************/
/* { */

/* } */

/*************
 *  MACROS   *
 *************/
/* { */

/* } */

/*************
 * TYPE DEFS *
 *************/
/* { */

/* } */

/*************
 * GLOBALS   *
 *************/
/* { */

/* } */

/*************
 * FUNCTIONS *
 *************/
/* { */

void
  DNX_TMC_SR_CELL_LINK_LIST_clear(
    DNX_SAND_OUT DNX_TMC_SR_CELL_LINK_LIST *info
  )
{
  uint32
    ind;

  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  sal_memset(info, 0x0, sizeof(DNX_TMC_SR_CELL_LINK_LIST));
  for (ind = 0; ind < DNX_TMC_CELL_NOF_LINKS_IN_PATH_LINKS; ++ind)
  {
    info->path_links[ind] = 0;
  }
  DNX_SAND_MAGIC_NUM_SET;
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  DNX_TMC_CELL_MC_TBL_DATA_clear(
    DNX_SAND_OUT DNX_TMC_CELL_MC_TBL_DATA *info
  )
{
  uint32
    ind;

  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  sal_memset(info, 0x0, sizeof(DNX_TMC_CELL_MC_TBL_DATA));
  for (ind = 0; ind < DNX_TMC_CELL_MC_DATA_IN_UINT32S; ++ind)
  {
    info->data[ind] = 0;
  }
  DNX_SAND_MAGIC_NUM_SET;
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  DNX_TMC_CELL_MC_TBL_INFO_clear(
    DNX_SAND_OUT DNX_TMC_CELL_MC_TBL_INFO *info
  )
{
  uint32
    ind;

  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  sal_memset(info, 0x0, sizeof(DNX_TMC_CELL_MC_TBL_INFO));
  info->fe_location = DNX_TMC_CELL_NOF_FE_LOCATIONS;
  for (ind = 0; ind < DNX_TMC_CELL_MC_NOF_LINKS; ++ind)
  {
    info->path_links[ind] = 0;
  }
  for (ind = 0; ind < DNX_TMC_CELL_MC_NOF_CHANGES; ++ind)
  {
    info->filter[ind] = 0;
  }
  DNX_SAND_MAGIC_NUM_SET;
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

#if DNX_TMC_DEBUG_IS_LVL1

const char*
  DNX_TMC_CELL_FE_LOCATION_to_string(
    DNX_SAND_IN  DNX_TMC_CELL_FE_LOCATION enum_val
  )
{
  const char* str = NULL;
  switch(enum_val)
  {
  case DNX_TMC_CELL_FE_LOCATION_FE1:
    str = "fe1";
  break;
  case DNX_TMC_CELL_FE_LOCATION_FE2:
    str = "fe2";
  break;
  case DNX_TMC_CELL_NOF_FE_LOCATIONS:
    str = "nof_fe_locations";
  break;
  default:
    str = " Unknown";
  }
  return str;
}

void
  DNX_TMC_SR_CELL_LINK_LIST_print(
    DNX_SAND_IN  DNX_TMC_SR_CELL_LINK_LIST *info
  )
{
  uint32
    ind;

  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  for (ind = 0; ind < DNX_TMC_CELL_NOF_LINKS_IN_PATH_LINKS; ++ind)
  {
    LOG_CLI((BSL_META_U(unit,
                        "path_links[%u]: %u\n\r"), ind,info->path_links[ind]));
  }
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  DNX_TMC_CELL_MC_TBL_DATA_print(
    DNX_SAND_IN  DNX_TMC_CELL_MC_TBL_DATA *info
  )
{
  uint32
    ind;

  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  for (ind = 0; ind < DNX_TMC_CELL_MC_DATA_IN_UINT32S; ++ind)
  {
    LOG_CLI((BSL_META_U(unit,
                        "data[%u]: %u\n\r"),ind,info->data[ind]));
  }
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  DNX_TMC_CELL_MC_TBL_INFO_print(
    DNX_SAND_IN  DNX_TMC_CELL_MC_TBL_INFO *info
  )
{
  uint32
    ind;

  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  LOG_CLI((BSL_META_U(unit,
                      "fe_location %s "), DNX_TMC_CELL_FE_LOCATION_to_string(info->fe_location)));
  for (ind = 0; ind < DNX_TMC_CELL_MC_NOF_LINKS; ++ind)
  {
    LOG_CLI((BSL_META_U(unit,
                        "path_links[%u]: %u\n\r"),ind,info->path_links[ind]));
  }
  for (ind = 0; ind < DNX_TMC_CELL_MC_NOF_CHANGES; ++ind)
  {
    LOG_CLI((BSL_META_U(unit,
                        "filter[%u]: %u\n\r"),ind,info->filter[ind]));
  }
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

#endif /* DNX_TMC_DEBUG_IS_LVL1 */

/* } */

#include <soc/dnx/legacy/SAND/Utils/sand_footer.h>
