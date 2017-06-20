/* $Id$
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/
/******************************************************************
*
* FILENAME:       DuneDriver/jer2_jer2_jer2_tmc/include/soc_jer2_jer2_jer2_tmcapi_cell.h
*
* MODULE PREFIX:  soc_jer2_jer2_jer2_tmccell
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

#ifndef __DNX_TMC_API_CELL_INCLUDED__
/* { */
#define __DNX_TMC_API_CELL_INCLUDED__

/*************
 * INCLUDES  *
 *************/
/* { */

#include <soc/dnx/legacy/SAND/Utils/sand_header.h>

#include <soc/dnx/legacy/SAND/Management/sand_general_macros.h>
#include <soc/dnx/legacy/SAND/Management/sand_error_code.h>

#include <soc/dnx/legacy/TMC/tmc_api_general.h>
#include <soc/dnx/legacy/SAND/SAND_FM/sand_cell.h>

/* } */
/*************
 * DEFINES   *
 *************/
/* { */

#define DNX_TMC_CELL_NOF_LINKS_IN_PATH_LINKS            (4)

#define DNX_TMC_CELL_MC_NOF_LINKS                       (2)
#define DNX_TMC_CELL_MC_DATA_IN_UINT32S                   (3)
#define DNX_TMC_CELL_MC_NOF_CHANGES                     (7)

/* } */

/*************
 * MACROS    *
 *************/
/* { */

/* } */

/*************
 * TYPE DEFS *
 *************/
/* { */

typedef enum
{
  /*
   *  The fabric destination of the inband cell is located at
   *  the first or third stage in a multi-stage system: FE1 or
   *  FE3.
   */
  DNX_TMC_CELL_FE_LOCATION_FE1 = 0,
  /*
   *  The fabric destination of the inband cell is located in
   *  a single-stage system or at the second stage in a
   *  multi-stage system: FE2.
   */
  DNX_TMC_CELL_FE_LOCATION_FE2 = 1,
  /*
   *  Total number of fabric locations.
   */
  DNX_TMC_CELL_NOF_FE_LOCATIONS = 2
}DNX_TMC_CELL_FE_LOCATION;

typedef struct
{
  DNX_SAND_MAGIC_NUM_VAR
  /*
   *  The kind of destination entity type: FE1, FE2, FE3, or
   *  FOP in case of a FAP device.
   */
  DNX_SAND_DEVICE_ENTITY  dest_entity_type;
  /*
   *  The list of the links composing the path used by the
   *  source-routed data cell.
   */
  uint8            path_links[DNX_TMC_CELL_NOF_LINKS_IN_PATH_LINKS];

} DNX_TMC_SR_CELL_LINK_LIST;

typedef struct
{
  DNX_SAND_MAGIC_NUM_VAR
  /*
   *  Links to get to the fabric. For an FE1 fabric (in
   *  multi-stage) or FE2 fabric (in single stage), only the
   *  first value must be configured. For an FE2 fabric in
   *  multi-stage system, both values must be set. For each
   *  value, Range: 0 - 63.
   */
  uint32 data[DNX_TMC_CELL_MC_DATA_IN_UINT32S];

} DNX_TMC_CELL_MC_TBL_DATA;

typedef struct
{
  DNX_SAND_MAGIC_NUM_VAR
  /*
   *  The fabric stage location: FE1 or FE2.
   */
  DNX_TMC_CELL_FE_LOCATION fe_location;
  /*
   *  Links to get to the fabric. For an FE1 fabric (in
   *  multi-stage) or FE2 fabric (in single stage), only the
   *  first value must be configured. For an FE2 fabric in
   *  multi-stage system, both values must be set. For each
   *  value, Range: 0 - 63.
   */
  uint32 path_links[DNX_TMC_CELL_MC_NOF_LINKS];
  /*
   *  Filter indicating the field changes to perform on the
   *  inband cell (in comparison with the previous inband cell
   *  sent via tmd_cell_mc_tbl_write). If True, the field is
   *  updated, otherwise the previous value is set. The filter
   *  mapping is: 0 - 'mc_id_ndx', 1 - 'fe_location', 2 -
   *  'path_links[0]', 3 - 'path_links[1]', 4 - 'data[0]', 5 -
   *  'data[1]', 6 - 'data[2]'. For a read operation, only the
   *  indexes 0 to 4 are relevant.
   */
  uint8 filter[DNX_TMC_CELL_MC_NOF_CHANGES];

} DNX_TMC_CELL_MC_TBL_INFO;

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
  );

void
  DNX_TMC_CELL_MC_TBL_DATA_clear(
    DNX_SAND_OUT DNX_TMC_CELL_MC_TBL_DATA *info
  );

void
  DNX_TMC_CELL_MC_TBL_INFO_clear(
    DNX_SAND_OUT DNX_TMC_CELL_MC_TBL_INFO *info
  );

#if DNX_TMC_DEBUG_IS_LVL1

const char*
  DNX_TMC_CELL_FE_LOCATION_to_string(
    DNX_SAND_IN  DNX_TMC_CELL_FE_LOCATION enum_val
  );

void
  DNX_TMC_SR_CELL_LINK_LIST_print(
    DNX_SAND_IN  DNX_TMC_SR_CELL_LINK_LIST *info
  );

void
  DNX_TMC_CELL_MC_TBL_DATA_print(
    DNX_SAND_IN  DNX_TMC_CELL_MC_TBL_DATA *info
  );

void
  DNX_TMC_CELL_MC_TBL_INFO_print(
    DNX_SAND_IN  DNX_TMC_CELL_MC_TBL_INFO *info
  );

#endif /* DNX_TMC_DEBUG_IS_LVL1 */

/* } */

#include <soc/dnx/legacy/SAND/Utils/sand_footer.h>

/* } __DNX_TMC_API_CELL_INCLUDED__*/
#endif
