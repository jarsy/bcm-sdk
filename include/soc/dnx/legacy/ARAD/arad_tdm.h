/* $Id: jer2_arad_tdm.h,v 1.4 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/

#ifndef __JER2_ARAD_TDM_INCLUDED__
/* { */
#define __JER2_ARAD_TDM_INCLUDED__

/*************
 * INCLUDES  *
 *************/
/* { */

#include <soc/dnx/legacy/SAND/Utils/sand_header.h>

#include <soc/dnx/legacy/SAND/Management/sand_general_macros.h>
#include <soc/dnx/legacy/SAND/Management/sand_error_code.h>
#include <soc/dnx/legacy/TMC/tmc_api_tdm.h>

/* } */
/*************
 * DEFINES   *
 *************/
/* { */

/* Define Number of TDM Context MAP */
#define JER2_ARAD_NOF_TDM_CONTEXT_MAP (2)

#define JER2_ARAD_TDM_VERSION_ID                                      (0x2)
#define JER2_ARAD_TDM_CELL_SIZE_MIN                                   (65)
#define JER2_ARAD_TDM_CELL_SIZE_MAX                                   (254)
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
#define JER2_ARAD_TDM_ING_ACTION_ADD                              DNX_TMC_TDM_ING_ACTION_ADD
#define JER2_ARAD_TDM_ING_ACTION_NO_CHANGE                        DNX_TMC_TDM_ING_ACTION_NO_CHANGE
#define JER2_ARAD_TDM_ING_ACTION_CUSTOMER_EMBED                   DNX_TMC_TDM_ING_ACTION_CUSTOMER_EMBED
#define JER2_ARAD_TDM_NOF_ING_ACTIONS                             DNX_TMC_TDM_NOF_ING_ACTIONS
typedef DNX_TMC_TDM_ING_ACTION                                 JER2_ARAD_TDM_ING_ACTION;

#define JER2_ARAD_TDM_EG_ACTION_REMOVE                            DNX_TMC_TDM_EG_ACTION_REMOVE
#define JER2_ARAD_TDM_EG_ACTION_NO_CHANGE                         DNX_TMC_TDM_EG_ACTION_NO_CHANGE
#define JER2_ARAD_TDM_EG_ACTION_CUSTOMER_EXTRACT                  DNX_TMC_TDM_EG_ACTION_CUSTOMER_EXTRACT
#define JER2_ARAD_TDM_NOF_EG_ACTIONS                              DNX_TMC_TDM_NOF_EG_ACTIONS
typedef DNX_TMC_TDM_EG_ACTION                                  JER2_ARAD_TDM_EG_ACTION;

typedef DNX_TMC_TDM_FTMH_OPT_UC                                JER2_ARAD_TDM_FTMH_OPT_UC;
typedef DNX_TMC_TDM_FTMH_OPT_MC                                JER2_ARAD_TDM_FTMH_OPT_MC;
typedef DNX_TMC_TDM_FTMH_STANDARD_UC                           JER2_ARAD_TDM_FTMH_STANDARD_UC;
typedef DNX_TMC_TDM_FTMH_STANDARD_MC                           JER2_ARAD_TDM_FTMH_STANDARD_MC;
typedef DNX_TMC_TDM_FTMH                                       JER2_ARAD_TDM_FTMH;
typedef DNX_TMC_TDM_FTMH_INFO                                  JER2_ARAD_TDM_FTMH_INFO;


typedef DNX_TMC_TDM_DIRECT_ROUTING_INFO                        JER2_ARAD_TDM_DIRECT_ROUTING_INFO;

typedef DNX_TMC_TDM_MC_STATIC_ROUTE_INFO                       JER2_ARAD_TDM_MC_STATIC_ROUTE_INFO;


typedef enum
{
  /*
   *  Optimize Unicast FTMH header.
   */
  JER2_ARAD_TDM_FTMH_INFO_MODE_OPT_UC = 0,
  /*
   *  Optimize Multicast FTMH header.
   */
  JER2_ARAD_TDM_FTMH_INFO_MODE_OPT_MC = 1,
  /*
   *  Standard Unicast ftmh header.
   */
  JER2_ARAD_TDM_FTMH_INFO_MODE_STANDARD_UC = 2,
  /*
   *  Standard Multicast ftmh header.
   */
  JER2_ARAD_TDM_FTMH_INFO_MODE_STANDARD_MC = 3,
  /*
   *  Number of types in JER2_ARAD_TDM_FTMH_INFO_MODE
   */
  JER2_ARAD_TDM_NOF_FTMH_INFO_MODE = 4
}JER2_ARAD_TDM_FTMH_INFO_MODE;

typedef struct
{
 /*
  *  to identify whether the size of the LAG is zero or one.
  */
  JER2_ARAD_INTERFACE_ID
    context_map[JER2_ARAD_NOF_TDM_CONTEXT_MAP];  

} JER2_ARAD_TDM;

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

/*********************************************************************
* NAME:
*     jer2_arad_tdm_unit_has_tdm
* FUNCTION:
*     check if unit has at least one port work in tdm mode(bypass or pkt).
* INPUT:
*  DNX_SAND_IN  int                 unit -
*     Identifier of the device to access.
*  DNX_SAND_OUT uint32             *tm_port_found -
*     output 1 if device has tdm port 0 if no
* RETURNS:
*   OK or ERROR indication.
* REMARKS:
*   Called as part of the initialization sequence.
*********************************************************************/

uint32 
  jer2_arad_tdm_unit_has_tdm(
    DNX_SAND_IN int unit,
    DNX_SAND_OUT uint32 *tdm_found
  );

/*********************************************************************
* NAME:
*     jer2_arad_tdm_init
* FUNCTION:
*     Initialization of the TDM configuration depends on the tdm mode.
* INPUT:
*  DNX_SAND_IN  int                 unit -
*     Identifier of the device to access.
* RETURNS:
*   OK or ERROR indication.
* REMARKS:
*   1. Called as part of the initialization sequence.
*********************************************************************/
uint32
  jer2_arad_tdm_init(
    DNX_SAND_IN  int  unit
  );

/*********************************************************************
* NAME:
 *   jer2_arad_tdm_ftmh_set_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Configure the FTMH header operation
 *   (added/unchanged/removed) at the ingress and egress,
 *   with the FTMH fields if added.
 * INPUT:
 *   DNX_SAND_IN  int                   unit -
 *     Identifier of the device to access.
 *   DNX_SAND_IN  uint32                   port_ndx -
 *     FAP Port index. Range: 0 - 79.
 *   DNX_SAND_IN  JER2_ARAD_TDM_FTMH_INFO            *info -
 *     Attributes of the FTMH operation functionality
 * REMARKS:
 *   This API is relevant only under a TDM traffic mode.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  jer2_arad_tdm_ftmh_set_unsafe(
    DNX_SAND_IN  int                   unit,
    DNX_SAND_IN  int                   core_id,
    DNX_SAND_IN  uint32                   port_ndx,
    DNX_SAND_IN  DNX_TMC_TDM_FTMH_INFO            *info
  );

uint32
  jer2_arad_tdm_ftmh_set_verify(
    DNX_SAND_IN  int                   unit,
    DNX_SAND_IN  uint32                   port_ndx,
    DNX_SAND_IN  DNX_TMC_TDM_FTMH_INFO            *info
  );

uint32
  jer2_arad_tdm_ftmh_get_verify(
    DNX_SAND_IN  int                   unit,
    DNX_SAND_IN  uint32                   port_ndx
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "jer2_arad_tdm_ftmh_set_unsafe" API.
 *     Refer to "jer2_arad_tdm_ftmh_set_unsafe" API for details.
*********************************************************************/
uint32
  jer2_arad_tdm_ftmh_get_unsafe(
    DNX_SAND_IN  int                   unit,
    DNX_SAND_IN  int                   core_id,
    DNX_SAND_IN  uint32                   port_ndx,
    DNX_SAND_OUT DNX_TMC_TDM_FTMH_INFO            *info
  );

/*********************************************************************
* NAME:
 *   jer2_arad_tdm_opt_size_set_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Configure the size limitations for the TDM cells in the
 *   Optimized FTMH TDM traffic mode.
 * INPUT:
 *   DNX_SAND_IN  int                   unit -
 *     Identifier of the device to access.
 *   DNX_SAND_IN  uint32                    cell_size -
 *     Cell constant size for the TDM cells (includes the
 *     Optimized FTMH). Unit: Bytes. Range: 65 - 128.
 * REMARKS:
 *   Relevant only for an Optimized FTMH TDM traffic mode.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  jer2_arad_tdm_opt_size_set_unsafe(
    DNX_SAND_IN  int                   unit,
    DNX_SAND_IN  uint32                    cell_size
  );

uint32
  jer2_arad_tdm_opt_size_set_verify(
    DNX_SAND_IN  int                   unit,
    DNX_SAND_IN  uint32                    cell_size
  );

uint32
  jer2_arad_tdm_opt_size_get_verify(
    DNX_SAND_IN  int                   unit
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "jer2_arad_tdm_opt_size_set_unsafe" API.
 *     Refer to "jer2_arad_tdm_opt_size_set_unsafe" API for
 *     details.
*********************************************************************/
uint32
  jer2_arad_tdm_opt_size_get_unsafe(
    DNX_SAND_IN  int                   unit,
    DNX_SAND_OUT uint32                    *cell_size
  );

/*********************************************************************
* NAME:
 *   jer2_arad_tdm_stand_size_range_set_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Configure the size limitations for the TDM cells in the
 *   Standard FTMH TDM traffic mode.
 * INPUT:
 *   DNX_SAND_IN  int                   unit -
 *     Identifier of the device to access.
 *   DNX_SAND_IN  DNX_SAND_U32_RANGE              *size_range -
 *     TDM cell size range (includes the Standard FTMH). Unit:
 *     Bytes. Range: 65 - 128.
 * REMARKS:
 *   Relevant only for an Standard FTMH TDM traffic mode.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  jer2_arad_tdm_stand_size_range_set_unsafe(
    DNX_SAND_IN  int                   unit,
    DNX_SAND_IN  DNX_SAND_U32_RANGE              *size_range
  );

uint32
  jer2_arad_tdm_stand_size_range_set_verify(
    DNX_SAND_IN  int                   unit,
    DNX_SAND_IN  DNX_SAND_U32_RANGE              *size_range
  );

uint32
  jer2_arad_tdm_stand_size_range_get_verify(
    DNX_SAND_IN  int                   unit
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "jer2_arad_tdm_stand_size_range_set_unsafe" API.
 *     Refer to "jer2_arad_tdm_stand_size_range_set_unsafe" API for
 *     details.
*********************************************************************/
uint32
  jer2_arad_tdm_stand_size_range_get_unsafe(
    DNX_SAND_IN  int                   unit,
    DNX_SAND_OUT DNX_SAND_U32_RANGE              *size_range
  );

/*********************************************************************
* NAME:
 *   jer2_arad_tdm_ofp_set_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Set the OFP ports configured as TDM destination.
 * INPUT:
 *   DNX_SAND_IN  int                   unit -
 *     Identifier of the device to access.
 *   DNX_SAND_IN  uint32                   port_ndx -
 *     FAP Port index. Range: 0 - 79.
 *   DNX_SAND_IN  uint8                   is_tdm -
 *     If True, then the OFP port is configured as a TDM
 *     destination.
 * REMARKS:
 *   Relevant only for a Packet traffic mode.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
int
  jer2_arad_tdm_ofp_set_unsafe(
    DNX_SAND_IN  int                   unit,
    DNX_SAND_IN  soc_port_t            port,
    DNX_SAND_IN  uint8                 is_tdm
  );

uint32
  jer2_arad_tdm_ofp_set_verify(
    DNX_SAND_IN  int                   unit,
    DNX_SAND_IN  uint32                   port_ndx,
    DNX_SAND_IN  uint8                   is_tdm
  );

uint32
  jer2_arad_tdm_ofp_get_verify(
    DNX_SAND_IN  int                   unit,
    DNX_SAND_IN  uint32                   port_ndx
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "jer2_arad_tdm_ofp_set_unsafe" API.
 *     Refer to "jer2_arad_tdm_ofp_set_unsafe" API for details.
*********************************************************************/
uint32
  jer2_arad_tdm_ofp_get_unsafe(
    DNX_SAND_IN  int                   unit,
    DNX_SAND_IN  uint32                   port_ndx,
    DNX_SAND_OUT uint8                   *is_tdm
  );

/*********************************************************************
* NAME:
 *   jer2_arad_tdm_ifp_set_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Set the IFP ports configured as TDM destination.
 * INPUT:
 *   DNX_SAND_IN  int                   unit -
 *     Identifier of the device to access.
 *   DNX_SAND_IN  uint32                   port_ndx -
 *     FAP Port index. Range: 0 - 79.
 *   DNX_SAND_IN  uint8                   is_tdm -
 *     If True, then the IFP port is configured as a TDM
 *     destination.
 * REMARKS:
 *   Relevant only for a Packet traffic mode.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
int
  jer2_arad_tdm_ifp_set(
    DNX_SAND_IN  int                   unit,
    DNX_SAND_IN  soc_port_t            port,
    DNX_SAND_IN  uint8                 is_tdm
  );

uint32
  jer2_arad_tdm_ifp_set_verify(
    DNX_SAND_IN  int                   unit,
    DNX_SAND_IN  uint32                   port_ndx,
    DNX_SAND_IN  uint8                   is_tdm
  );

uint32
  jer2_arad_tdm_ifp_get_verify(
    DNX_SAND_IN  int                   unit,
    DNX_SAND_IN  uint32                   port_ndx
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "jer2_arad_tdm_ifp_set_unsafe" API.
 *     Refer to "jer2_arad_tdm_ifp_set_unsafe" API for details.
*********************************************************************/
int
  jer2_arad_tdm_ifp_get(
    DNX_SAND_IN  int             unit,
    DNX_SAND_IN  soc_port_t      port,
    DNX_SAND_OUT uint8           *is_tdm
  );

/*********************************************************************
* NAME:
 *   jer2_arad_tdm_direct_routing_set_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Set the TDM direct routing configuration. Up to
 *   36 routing profiles can be defined. 
 * INPUT:
 *   DNX_SAND_IN  int                   unit -
 *     Identifier of the device to access.
 *   DNX_SAND_IN  uint32                   direct_routing_profile -
 *     TDM direct routing profile index. Mapping profile
 *     index. Range: 0 - 35.
 *   DNX_SAND_IN  JER2_ARAD_TDM_DIRECT_ROUTING_INFO *direct_routing_info -
 *     TDM direct routing configuration.
 *   DNX_SAND_IN  uint8 enable_rpt_reachable -
 *      Enables looking at the RPT reachable bitmap.
 * REMARKS:
 *   Each Incoming-Port is mapped to a direct routing profile by the
 *   jer2_arad_tdm_direct_routing_profile_map_set API.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  jer2_arad_tdm_direct_routing_set_unsafe(
    DNX_SAND_IN  int                   unit,
    DNX_SAND_IN  uint32                   direct_routing_profile,
    DNX_SAND_IN  DNX_TMC_TDM_DIRECT_ROUTING_INFO *direct_routing_info,
    DNX_SAND_IN  uint8 enable_rpt_reachable
  );

uint32
  jer2_arad_tdm_direct_routing_set_verify(
    DNX_SAND_IN  int                   unit,
    DNX_SAND_IN  uint32                   direct_routing_profile,
    DNX_SAND_IN  DNX_TMC_TDM_DIRECT_ROUTING_INFO *direct_routing_info
  );

uint32
  jer2_arad_tdm_direct_routing_get_verify(
    DNX_SAND_IN  int                   unit,
    DNX_SAND_IN  uint32                   direct_routing_profile
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "jer2_arad_tdm_direct_routing_set_unsafe" API.
 *     Refer to "jer2_arad_tdm_direct_routing_set_unsafe" API for
 *     details.
*********************************************************************/
uint32
  jer2_arad_tdm_direct_routing_get_unsafe(
    DNX_SAND_IN  int                   unit,
    DNX_SAND_IN  uint32                   direct_routing_profile,
    DNX_SAND_OUT DNX_TMC_TDM_DIRECT_ROUTING_INFO *direct_routing_info,
    DNX_SAND_OUT uint8 *enable
  );

/*********************************************************************
* NAME:
 *   jer2_arad_tdm_direct_routing_profile_map_set_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Sets Incoming FAP Port (IFP) routing profile type, per port.
 *   36 routing profiles can be defined. 
 * INPUT:
 *   DNX_SAND_IN  int                   unit -
 *     Identifier of the device to access.
 *   DNX_SAND_IN  uint32                   port_ndx -
 *     Incoming Fap Port index. Range: 0 - 255.
 *   DNX_SAND_IN  uint32                   direct_routing_profile -
 *     TDM direct routing profile index. Range: 0-35
 * REMARKS:
 *   The profile configuration is set by the
 *   jer2_arad_tdm_direct_routing_set API.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  jer2_arad_tdm_direct_routing_profile_map_set_unsafe(
    DNX_SAND_IN  int                     unit,
    DNX_SAND_IN  uint32                     port_ndx,
    DNX_SAND_IN  uint32                     direct_routing_profile 
  );

uint32
  jer2_arad_tdm_direct_routing_profile_map_set_verify(
    DNX_SAND_IN  int                     unit,
    DNX_SAND_IN  uint32                     port_ndx,
    DNX_SAND_IN  uint32                     direct_routing_profile 
  );

uint32
  jer2_arad_tdm_direct_routing_profile_map_get_verify(
    DNX_SAND_IN  int                   unit,
    DNX_SAND_IN  uint32                   port_ndx
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "jer2_arad_tdm_direct_routing_profile_map_set_unsafe" API.
 *     Refer to "jer2_arad_tdm_direct_routing_profile_map_set_unsafe" API for
 *     details.
*********************************************************************/
uint32
  jer2_arad_tdm_direct_routing_profile_map_get_unsafe(
    DNX_SAND_IN   int                     unit,
    DNX_SAND_IN   uint32                     port_ndx,
    DNX_SAND_OUT  uint32                    *direct_routing_profile
  );

/*********************************************************************
* NAME:
 *   jer2_arad_tdm_port_packet_crc_set_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Enable generating and removing fabric CRC Per FAP Port.   
 * INPUT:
 *   DNX_SAND_IN  int                   unit -
 *     Identifier of the device to access.
 *   DNX_SAND_IN  uint32                   port_ndx -
 *     Incoming Fap Port index. Range: 0 - 255.
 *   DNX_SAND_IN  uint8                   is_enable -
 *     Enable / Disable Packet CRC.
 * REMARKS:
 *   API is relavant only when port is in bypass mode (TDM).
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  jer2_arad_tdm_port_packet_crc_set_unsafe(
    DNX_SAND_IN  int                     unit,
    DNX_SAND_IN  soc_port_t                 port_ndx,
    DNX_SAND_IN  uint8                      is_enable,         /* value to configure (is CRC added in fabric) */
    DNX_SAND_IN  uint8                      configure_ingress, /* should ingress be configured */
    DNX_SAND_IN  uint8                      configure_egress   /* should egress be configured */
  );

uint32
  jer2_arad_tdm_port_packet_crc_set_verify(
    DNX_SAND_IN  int                     unit,
    DNX_SAND_IN  uint32                     port_ndx,
    DNX_SAND_IN  uint8                     is_enable 
  );

uint32
  jer2_arad_tdm_port_packet_crc_get_verify(
    DNX_SAND_IN  int                   unit,
    DNX_SAND_IN  uint32                   port_ndx
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "jer2_arad_tdm_port_packet_crc_set_unsafe" API.
 *     Refer to "jer2_arad_tdm_port_packet_crc_set_unsafe" API for
 *     details.
*********************************************************************/
uint32
  jer2_arad_tdm_port_packet_crc_get_unsafe(
    DNX_SAND_IN   int                     unit,
    DNX_SAND_IN   soc_port_t                 port_ndx,
    DNX_SAND_OUT  uint8                      *is_ingress_enabled,
    DNX_SAND_OUT  uint8                      *is_egress_enabled
  );

#if JER2_ARAD_DEBUG_IS_LVL1

uint32
  JER2_ARAD_TDM_FTMH_OPT_UC_verify(
    DNX_SAND_IN  JER2_ARAD_TDM_FTMH_OPT_UC *info
  );

uint32
  JER2_ARAD_TDM_FTMH_OPT_MC_verify(
    DNX_SAND_IN  JER2_ARAD_TDM_FTMH_OPT_MC *info
  );

uint32
  JER2_ARAD_TDM_FTMH_STANDARD_UC_verify(
    DNX_SAND_IN  JER2_ARAD_TDM_FTMH_STANDARD_UC *info
  );

uint32
  JER2_ARAD_TDM_FTMH_STANDARD_MC_verify(
    DNX_SAND_IN  JER2_ARAD_TDM_FTMH_STANDARD_MC *info
  );

uint32
  JER2_ARAD_TDM_FTMH_INFO_verify(
    DNX_SAND_IN  JER2_ARAD_TDM_FTMH_INFO *info
  );
uint32
  JER2_ARAD_TDM_MC_STATIC_ROUTE_INFO_verify(
    DNX_SAND_IN  JER2_ARAD_TDM_MC_STATIC_ROUTE_INFO *info
  );
uint32
  JER2_ARAD_TDM_DIRECT_ROUTING_INFO_verify(
    DNX_SAND_IN  uint32 unit,
    DNX_SAND_IN  JER2_ARAD_TDM_DIRECT_ROUTING_INFO *info
  );
#endif /* JER2_ARAD_DEBUG_IS_LVL1 */
/* } */

#include <soc/dnx/legacy/SAND/Utils/sand_footer.h>

/* } __JER2_ARAD_TDM_INCLUDED__*/
#endif

