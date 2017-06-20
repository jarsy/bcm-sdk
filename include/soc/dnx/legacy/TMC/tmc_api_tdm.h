/* $Id: jer2_jer2_jer2_tmc_api_tdm.h,v 1.5 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/
/******************************************************************
*
* FILENAME:       DuneDriver/jer2_jer2_jer2_tmc/include/soc_jer2_jer2_jer2_tmcapi_tdm.h
*
* MODULE PREFIX:  soc_jer2_jer2_jer2_tmctdm
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

#ifndef __DNX_TMC_API_TDM_INCLUDED__
/* { */
#define __DNX_TMC_API_TDM_INCLUDED__

/*************
 * INCLUDES  *
 *************/
/* { */

#include <soc/dnx/legacy/SAND/Utils/sand_header.h>
#include <soc/dnx/legacy/SAND/Utils/sand_u64.h>

#include <soc/dnx/legacy/SAND/Management/sand_general_macros.h>
#include <soc/dnx/legacy/SAND/Management/sand_error_code.h>

#include <soc/dnx/legacy/TMC/tmc_api_general.h>

/* } */
/*************
 * DEFINES   *
 *************/
/* { */
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
   *  Add an FTMH header to all the received TDM cells at this
   *  ITM-Port. If set, the FTMH fields must be configured via
   *  the 'ftmh' parameter and are kept during the cell
   *  switching.
   */
  DNX_TMC_TDM_ING_ACTION_ADD = 0,
  /*
   *  Do not change the FTMH header to all the received TDM
   *  cells at this ITM-Port - the FMTH must be already there.
   */
  DNX_TMC_TDM_ING_ACTION_NO_CHANGE = 1,
  /*
   *  Embed an external customer header in an added Standard
   *  FTMH Header. Relevant only when the FTMH format is
   *  Standard. Must be used for interoperability with a PMC
   *  HyPhy PMM header.
   */
  DNX_TMC_TDM_ING_ACTION_CUSTOMER_EMBED = 2,
  /*
   *  Number of types in DNX_TMC_TDM_ING_ACTION
   */
  DNX_TMC_TDM_NOF_ING_ACTIONS = 3
}DNX_TMC_TDM_ING_ACTION;

typedef enum
{
  /*
   *  Remove the FTMH header to all the transmitted TDM cells
   *  at this OFP-Port.
   */
  DNX_TMC_TDM_EG_ACTION_REMOVE = 0,
  /*
   *  Do not change the FTMH header to all the transmitted TDM
   *  cells at this OFP-Port.
   */
  DNX_TMC_TDM_EG_ACTION_NO_CHANGE = 1,
  /*
   *  Extract an external customer overhead from a removed
   *  Standard FTMH Header. Relevant only when the FTMH format
   *  is Standard. Must be used for interoperability with a
   *  PMC HyPhy PMM header.
   */
  DNX_TMC_TDM_EG_ACTION_CUSTOMER_EXTRACT = 2,
  /*
   *  Number of types in DNX_TMC_TDM_EG_ACTION
   */
  DNX_TMC_TDM_NOF_EG_ACTIONS = 3
}DNX_TMC_TDM_EG_ACTION;

typedef struct
{
  DNX_SAND_MAGIC_NUM_VAR
  /*
   *  Destination interface: the egress interface with its
   *  channel. Range: 0 - 31.
   */
  uint32 dest_if;
  /*
   *  Destination FAP Id. Range: 0 - 1K-1.
   */
  uint32 dest_fap_id;

} DNX_TMC_TDM_FTMH_OPT_UC;

typedef struct
{
  DNX_SAND_MAGIC_NUM_VAR
  /*
   *  Multicast Id. Range: 0 - 16K-1.
   */
  uint32 mc_id;

} DNX_TMC_TDM_FTMH_OPT_MC;

typedef struct
{
  DNX_SAND_MAGIC_NUM_VAR
  /*
   *  The user-defined fields (i.e., 32 bits Customer Header
   *  to embed). Are embedded to the bits 47:32, 23:20, 19:17
   *  and 15:14 in the header according to a static mapping.
   */
  uint32 user_def;  
  /*
   *  Destination system physical port. Range: 0 - 4K-1.
   *  Invalid for JER2_ARAD.
   */
  uint32 sys_phy_port;
  /* 
   *  Destination FAP ID. Range: 0-2047.
   *  Invalid for Soc_petra-B.
   */
  uint32 dest_fap_id;
  /* 
   *  Destination FAP Port. Range: 0-255.
   *  Invalid for Soc_petra-B.
   */
  uint32 dest_fap_port;
  /* 
   *  The user-define fields for bits 33-DNX_TMC_TDM_MAX_USER_DEFINE_FIELDS.
   *  Are embedded to the bits 57:41 in the header according to a static
   *  mapping. Valid only for JER2_ARAD FTMH (i.e. without Soc_petra-B in system).
   *  Invalid for Soc_petra-B. 
   */
  uint32 user_def_2;

} DNX_TMC_TDM_FTMH_STANDARD_UC;

typedef struct
{
  DNX_SAND_MAGIC_NUM_VAR
  /*
   *  The user-defined fields (i.e., 32 bits Customer Header
   *  to embed). Are embedded to the bits 47:32, 23:20, 19:17
   *  and 15:14 in the header according to a static mapping.
   */
  uint32 user_def;
  /*
   *  Multicast Id. Range: 0 - 16K-1.
   */
  uint32 mc_id;

} DNX_TMC_TDM_FTMH_STANDARD_MC;

typedef union
{
  DNX_SAND_MAGIC_NUM_VAR
  /*
   *  The FTMH header fields to configure of a TDM unicast
   *  cell with an Optimized FTMH.
   */
  DNX_TMC_TDM_FTMH_OPT_UC opt_uc;
  /*
   *  The FTMH header fields to configure of a TDM multicast
   *  cell with an Optimized FTMH.
   */
  DNX_TMC_TDM_FTMH_OPT_MC opt_mc;
  /*
   *  The FTMH header fields to configure of a TDM unicast
   *  cell with a Standard FTMH.
   */
  DNX_TMC_TDM_FTMH_STANDARD_UC standard_uc;
  /*
   *  The FTMH header fields to configure of a TDM multicast
   *  cell with a Standard FTMH.
   */
  DNX_TMC_TDM_FTMH_STANDARD_MC standard_mc;

} DNX_TMC_TDM_FTMH;

typedef struct
{
  DNX_SAND_MAGIC_NUM_VAR
  /*
   *  Action (adding or not) to operate when the TDM cells are
   *  received.
   */
  DNX_TMC_TDM_ING_ACTION action_ing;
  /*
   *  If TRUE, then the destination FTMH to add is of type
   *  multicast. Must be set only if the ingress action is 'ADD'.
   */
  uint8 is_mc;
  /*
   *  FTMH to add to the received TDM cells. Must be set only
   *  if the ingress action is 'ADD'.
   */
  DNX_TMC_TDM_FTMH ftmh;
  /*
   *  Action (removing or not) to operate when the TDM cells
   *  are transmitted.
   */
  DNX_TMC_TDM_EG_ACTION action_eg;
  
} DNX_TMC_TDM_FTMH_INFO;

typedef struct
{
  DNX_SAND_MAGIC_NUM_VAR
  /*
   *  Bitmap of the fabric links to define the set of links
   *  used for specific MC route cell. For each link (in the
   *  range 0 - 35), if its bit is set, then the respective
   *  link is used for this Multicast route.
   */
  DNX_SAND_U64 link_bitmap;

} DNX_TMC_TDM_MC_STATIC_ROUTE_INFO;

typedef struct
{
  DNX_SAND_MAGIC_NUM_VAR
  /*
   *  Bitmap of the fabric links to define the set of links
   *  used for direct routing. For each link (in the
   *  range 0 - 35), if its bit is set, then the respective
   *  link is used for this Multicast route.
   */
  DNX_SAND_U64 link_bitmap;

} DNX_TMC_TDM_DIRECT_ROUTING_INFO;


#define DNX_TMC_TDM_PUSH_QUEUE_TYPE                            (DNX_TMC_ITM_QT_NDX_15)

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
  DNX_TMC_TDM_FTMH_OPT_UC_clear(
    DNX_SAND_OUT DNX_TMC_TDM_FTMH_OPT_UC *info
  );

void
  DNX_TMC_TDM_FTMH_OPT_MC_clear(
    DNX_SAND_OUT DNX_TMC_TDM_FTMH_OPT_MC *info
  );

void
  DNX_TMC_TDM_FTMH_STANDARD_UC_clear(
    DNX_SAND_OUT DNX_TMC_TDM_FTMH_STANDARD_UC *info
  );

void
  DNX_TMC_TDM_FTMH_STANDARD_MC_clear(
    DNX_SAND_OUT DNX_TMC_TDM_FTMH_STANDARD_MC *info
  );

void
  DNX_TMC_TDM_FTMH_clear(
    DNX_SAND_OUT DNX_TMC_TDM_FTMH *info
  );

void
  DNX_TMC_TDM_FTMH_INFO_clear(
    DNX_SAND_OUT DNX_TMC_TDM_FTMH_INFO *info
  );
void
  DNX_TMC_TDM_MC_STATIC_ROUTE_INFO_clear(
    DNX_SAND_OUT DNX_TMC_TDM_MC_STATIC_ROUTE_INFO *info
  );

void
  DNX_TMC_TDM_DIRECT_ROUTING_INFO_clear(
    DNX_SAND_OUT DNX_TMC_TDM_DIRECT_ROUTING_INFO *info
  );

#if DNX_TMC_DEBUG_IS_LVL1

const char*
  DNX_TMC_TDM_ING_ACTION_to_string(
    DNX_SAND_IN  DNX_TMC_TDM_ING_ACTION enum_val
  );

const char*
  DNX_TMC_TDM_EG_ACTION_to_string(
    DNX_SAND_IN  DNX_TMC_TDM_EG_ACTION enum_val
  );

void
  DNX_TMC_TDM_FTMH_OPT_UC_print(
    DNX_SAND_IN  DNX_TMC_TDM_FTMH_OPT_UC *info
  );

void
  DNX_TMC_TDM_FTMH_OPT_MC_print(
    DNX_SAND_IN  DNX_TMC_TDM_FTMH_OPT_MC *info
  );

void
  DNX_TMC_TDM_FTMH_STANDARD_UC_print(
    DNX_SAND_IN  DNX_TMC_TDM_FTMH_STANDARD_UC *info
  );

void
  DNX_TMC_TDM_FTMH_STANDARD_MC_print(
    DNX_SAND_IN  DNX_TMC_TDM_FTMH_STANDARD_MC *info
  );

void
  DNX_TMC_TDM_FTMH_print(
    DNX_SAND_IN  DNX_TMC_TDM_FTMH *info
  );

void
  DNX_TMC_TDM_FTMH_INFO_print(
    DNX_SAND_IN  DNX_TMC_TDM_FTMH_INFO *info
  );
void
  DNX_TMC_TDM_MC_STATIC_ROUTE_INFO_print(
    DNX_SAND_IN  DNX_TMC_TDM_MC_STATIC_ROUTE_INFO *info
  );

void
  DNX_TMC_TDM_DIRECT_ROUTING_INFO_print(
    DNX_SAND_IN  DNX_TMC_TDM_DIRECT_ROUTING_INFO *info
  );

#endif /* DNX_TMC_DEBUG_IS_LVL1 */

/* } */

#include <soc/dnx/legacy/SAND/Utils/sand_footer.h>

/* } __DNX_TMC_API_TDM_INCLUDED__*/
#endif
