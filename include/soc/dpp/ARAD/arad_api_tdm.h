/* $Id: arad_api_tdm.h,v 1.4 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/

#ifndef __ARAD_API_TDM_INCLUDED__
/* { */
#define __ARAD_API_TDM_INCLUDED__

/*************
 * INCLUDES  *
 *************/
/* { */

#include <soc/dpp/SAND/Utils/sand_header.h>

#include <soc/dpp/SAND/Management/sand_general_macros.h>
#include <soc/dpp/SAND/Management/sand_error_code.h>

#include <soc/dpp/TMC/tmc_api_tdm.h>
#include <soc/dpp/TMC/tmc_api_ingress_traffic_mgmt.h>

#include <soc/dpp/ARAD/arad_api_general.h>

/* } */
/*************
 * DEFINES   *
 *************/
/* { */

/* Queue type for TDM */
#define ARAD_TDM_PUSH_QUEUE_TYPE                            (SOC_TMC_ITM_QT_NDX_15)

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

#define ARAD_TDM_ING_ACTION_ADD                              SOC_TMC_TDM_ING_ACTION_ADD
#define ARAD_TDM_ING_ACTION_NO_CHANGE                        SOC_TMC_TDM_ING_ACTION_NO_CHANGE
#define ARAD_TDM_ING_ACTION_CUSTOMER_EMBED                   SOC_TMC_TDM_ING_ACTION_CUSTOMER_EMBED
#define ARAD_TDM_NOF_ING_ACTIONS                             SOC_TMC_TDM_NOF_ING_ACTIONS
typedef SOC_TMC_TDM_ING_ACTION                                 ARAD_TDM_ING_ACTION;

#define ARAD_TDM_EG_ACTION_REMOVE                            SOC_TMC_TDM_EG_ACTION_REMOVE
#define ARAD_TDM_EG_ACTION_NO_CHANGE                         SOC_TMC_TDM_EG_ACTION_NO_CHANGE
#define ARAD_TDM_EG_ACTION_CUSTOMER_EXTRACT                  SOC_TMC_TDM_EG_ACTION_CUSTOMER_EXTRACT
#define ARAD_TDM_NOF_EG_ACTIONS                              SOC_TMC_TDM_NOF_EG_ACTIONS
typedef SOC_TMC_TDM_EG_ACTION                                  ARAD_TDM_EG_ACTION;

typedef SOC_TMC_TDM_FTMH_OPT_UC                                ARAD_TDM_FTMH_OPT_UC;
typedef SOC_TMC_TDM_FTMH_OPT_MC                                ARAD_TDM_FTMH_OPT_MC;
typedef SOC_TMC_TDM_FTMH_STANDARD_UC                           ARAD_TDM_FTMH_STANDARD_UC;
typedef SOC_TMC_TDM_FTMH_STANDARD_MC                           ARAD_TDM_FTMH_STANDARD_MC;
typedef SOC_TMC_TDM_FTMH                                       ARAD_TDM_FTMH;
typedef SOC_TMC_TDM_FTMH_INFO                                  ARAD_TDM_FTMH_INFO;


typedef SOC_TMC_TDM_DIRECT_ROUTING_INFO                        ARAD_TDM_DIRECT_ROUTING_INFO;

typedef SOC_TMC_TDM_MC_STATIC_ROUTE_INFO                       ARAD_TDM_MC_STATIC_ROUTE_INFO;

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
 *   arad_tdm_ftmh_set
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Configure the FTMH header operation
 *   (added/unchanged/removed) at the ingress and egress,
 *   with the FTMH fields if added.
 * INPUT:
 *   SOC_SAND_IN  int                   unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  uint32                   port_ndx -
 *     FAP Port index. Range: 0 - 79.
 *   SOC_SAND_IN  ARAD_TDM_FTMH_INFO            *info -
 *     Attributes of the FTMH operation functionality
 * REMARKS:
 *   This API is relevant only under a TDM traffic mode.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_tdm_ftmh_set(
    SOC_SAND_IN  int                   unit,
    SOC_SAND_IN  int                   core_id,
    SOC_SAND_IN  uint32                   port_ndx,
    SOC_SAND_IN  ARAD_TDM_FTMH_INFO            *info
  );

/*********************************************************************
*     Gets the configuration set by the "arad_tdm_ftmh_set" API.
 *     Refer to "arad_tdm_ftmh_set" API for details.
*********************************************************************/
uint32
  arad_tdm_ftmh_get(
    SOC_SAND_IN  int                   unit,
    SOC_SAND_IN  int                   core_id,
    SOC_SAND_IN  uint32                   port_ndx,
    SOC_SAND_OUT ARAD_TDM_FTMH_INFO            *info
  );

/*********************************************************************
* NAME:
 *   arad_tdm_opt_size_set
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Configure the size limitations for the TDM cells in the
 *   Optimized FTMH TDM traffic mode.
 * INPUT:
 *   SOC_SAND_IN  int                   unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  uint32                    cell_size -
 *     Cell constant size for the TDM cells (includes the
 *     Optimized FTMH). Unit: Bytes. Range: 65 - 128.
 * REMARKS:
 *   Relevant only for an Optimized FTMH TDM traffic mode.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_tdm_opt_size_set(
    SOC_SAND_IN  int                   unit,
    SOC_SAND_IN  uint32                    cell_size
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "arad_tdm_opt_size_set" API.
 *     Refer to "arad_tdm_opt_size_set" API for details.
*********************************************************************/
uint32
  arad_tdm_opt_size_get(
    SOC_SAND_IN  int                   unit,
    SOC_SAND_OUT uint32                    *cell_size
  );

/*********************************************************************
* NAME:
 *   arad_tdm_stand_size_range_set
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Configure the size limitations for the TDM cells in the
 *   Standard FTMH TDM traffic mode.
 * INPUT:
 *   SOC_SAND_IN  int                   unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_SAND_U32_RANGE              *size_range -
 *     TDM cell size range (includes the Standard FTMH). Unit:
 *     Bytes. Range: 65 - 128.
 * REMARKS:
 *   Relevant only for an Standard FTMH TDM traffic mode.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_tdm_stand_size_range_set(
    SOC_SAND_IN  int                   unit,
    SOC_SAND_IN  SOC_SAND_U32_RANGE              *size_range
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "arad_tdm_stand_size_range_set" API.
 *     Refer to "arad_tdm_stand_size_range_set" API for details.
*********************************************************************/
uint32
  arad_tdm_stand_size_range_get(
    SOC_SAND_IN  int                   unit,
    SOC_SAND_OUT SOC_SAND_U32_RANGE              *size_range
  );

/*********************************************************************
* NAME:
 *   arad_tdm_ofp_set
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Set the OFP ports configured as TDM destination.
 * INPUT:
 *   SOC_SAND_IN  int                   unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  uint32                   port_ndx -
 *     FAP Port index. Range: 0 - 79.
 *   SOC_SAND_IN  uint8                   is_tdm -
 *     If True, then the OFP port is configured as a TDM
 *     destination.
 * REMARKS:
 *   Relevant only for a Packet traffic mode.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_tdm_ofp_set(
    SOC_SAND_IN  int                   unit,
    SOC_SAND_IN  uint32                   port_ndx,
    SOC_SAND_IN  uint8                   is_tdm
  );

/*********************************************************************
*     Gets the configuration set by the "arad_tdm_ofp_set" API.
 *     Refer to "arad_tdm_ofp_set" API for details.
*********************************************************************/
uint32
  arad_tdm_ofp_get(
    SOC_SAND_IN  int                   unit,
    SOC_SAND_IN  uint32                   port_ndx,
    SOC_SAND_OUT uint8                   *is_tdm
  );

/*********************************************************************
* NAME:
 *   arad_tdm_direct_routing_set
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Set the TDM direct routing configuration. Up to
 *   36 routing profiles can be defined. 
 * INPUT:
 *   SOC_SAND_IN  int                   unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  uint32                   direct_routing_profile -
 *     TDM direct routing profile index. Mapping profile
 *     index. Range: 0 - 35.
 *   SOC_SAND_IN  ARAD_TDM_DIRECT_ROUTING_INFO *direct_routing_info -
 *     TDM direct routing configuration.
 *   SOC_SAND_IN  uint8 enable_rpt_reachable -
 *      Enables looking at the RPT reachable bitmap.
 * REMARKS:
 *   Each Incoming-Port is mapped to a direct routing profile by the
 *   arad_tdm_direct_routing_profile_map_set API.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_tdm_direct_routing_set(
    SOC_SAND_IN  int                   unit,
    SOC_SAND_IN  uint32                   direct_routing_profile,
    SOC_SAND_IN  ARAD_TDM_DIRECT_ROUTING_INFO *direct_routing_info,
    SOC_SAND_IN  uint8 enable_rpt_reachable
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "arad_tdm_direct_routing_set" API.
 *     Refer to "arad_tdm_direct_routing_set" API for details.
*********************************************************************/
uint32
  arad_tdm_direct_routing_get(
    SOC_SAND_IN  int                   unit,
    SOC_SAND_IN  uint32                   direct_routing_profile,
    SOC_SAND_OUT ARAD_TDM_DIRECT_ROUTING_INFO *direct_routing_info,
    SOC_SAND_OUT uint8 *enable_rpt_reachable
  );

/*********************************************************************
* NAME:
 *   arad_tdm_direct_routing_profile_map_set
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Sets Incoming FAP Port (IFP) routing profile type, per port.
 *   36 routing profiles can be defined. 
 * INPUT:
 *   SOC_SAND_IN  int                   unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  uint32                   port_ndx -
 *     Incoming Fap Port index. Range: 0 - 255.
 *   SOC_SAND_IN  uint32                   direct_routing_profile -
 *     TDM direct routing profile index. Range: 0-35
 * REMARKS:
 *   The profile configuration is set by the
 *   arad_tdm_direct_routing_set API.
 *   API is relavant only when port is in bypass mode (TDM).
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_tdm_direct_routing_profile_map_set(
    SOC_SAND_IN  int                   unit,
    SOC_SAND_IN  uint32                   port_ndx,
    SOC_SAND_IN  uint32                   direct_routing_profile
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "arad_tdm_direct_routing_profile_map_set" API.
 *     Refer to "arad_tdm_direct_routing_profile_map_set" API for details.
*********************************************************************/
uint32
  arad_tdm_direct_routing_profile_map_get(
    SOC_SAND_IN  int                   unit,
    SOC_SAND_IN  uint32                   port_ndx,
    SOC_SAND_OUT uint32                   *direct_routing_profile
  );

/*********************************************************************
* NAME:
 *   arad_tdm_port_packet_crc_set
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Enable generating and removing fabric CRC Per FAP Port.   
 * INPUT:
 *   SOC_SAND_IN  int                   unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  uint32                   port_ndx -
 *     Incoming Fap Port index. Range: 0 - 255.
 *   SOC_SAND_IN  uint8                   is_enable -
 *     Enable / Disable Packet CRC.
 * REMARKS:
 *   API is relavant only when port is in bypass mode (TDM).
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_tdm_port_packet_crc_set(
    SOC_SAND_IN  int                   unit,
    SOC_SAND_IN  soc_port_t               port_ndx,
    SOC_SAND_IN  uint8                    is_enable,         /* value to configure (is CRC added in fabric) */
    SOC_SAND_IN  uint8                    configure_ingress, /* should ingress be configured */
    SOC_SAND_IN  uint8                    configure_egress   /* should egress be configured */
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "arad_tdm_port_packet_crc_set" API.
 *     Refer to "arad_tdm_port_packet_crc_set" API for details.
*********************************************************************/
uint32
  arad_tdm_port_packet_crc_get(
    SOC_SAND_IN  int                   unit,
    SOC_SAND_IN  soc_port_t                   port_ndx,
    SOC_SAND_OUT uint8                    *is_ingress_enabled,
    SOC_SAND_OUT uint8                    *is_egress_enabled
  );

void
  ARAD_TDM_FTMH_OPT_UC_clear(
    SOC_SAND_OUT ARAD_TDM_FTMH_OPT_UC *info
  );

void
  ARAD_TDM_FTMH_OPT_MC_clear(
    SOC_SAND_OUT ARAD_TDM_FTMH_OPT_MC *info
  );

void
  ARAD_TDM_FTMH_STANDARD_UC_clear(
    SOC_SAND_OUT ARAD_TDM_FTMH_STANDARD_UC *info
  );

void
  ARAD_TDM_FTMH_STANDARD_MC_clear(
    SOC_SAND_OUT ARAD_TDM_FTMH_STANDARD_MC *info
  );

void
  ARAD_TDM_FTMH_clear(
    SOC_SAND_OUT ARAD_TDM_FTMH *info
  );

void
  ARAD_TDM_FTMH_INFO_clear(
    SOC_SAND_OUT ARAD_TDM_FTMH_INFO *info
  );
void
  ARAD_TDM_MC_STATIC_ROUTE_INFO_clear(
    SOC_SAND_OUT ARAD_TDM_MC_STATIC_ROUTE_INFO *info
  );
void
  ARAD_TDM_DIRECT_ROUTING_INFO_clear(
    SOC_SAND_OUT ARAD_TDM_DIRECT_ROUTING_INFO *info
  );

#if ARAD_DEBUG_IS_LVL1

const char*
  ARAD_TDM_ING_ACTION_to_string(
    SOC_SAND_IN  ARAD_TDM_ING_ACTION enum_val
  );

const char*
  ARAD_TDM_EG_ACTION_to_string(
    SOC_SAND_IN  ARAD_TDM_EG_ACTION enum_val
  );

void
  ARAD_TDM_FTMH_OPT_UC_print(
    SOC_SAND_IN  ARAD_TDM_FTMH_OPT_UC *info
  );

void
  ARAD_TDM_FTMH_OPT_MC_print(
    SOC_SAND_IN  ARAD_TDM_FTMH_OPT_MC *info
  );

void
  ARAD_TDM_FTMH_STANDARD_UC_print(
    SOC_SAND_IN  ARAD_TDM_FTMH_STANDARD_UC *info
  );

void
  ARAD_TDM_FTMH_STANDARD_MC_print(
    SOC_SAND_IN  ARAD_TDM_FTMH_STANDARD_MC *info
  );

void
  ARAD_TDM_FTMH_print(
    SOC_SAND_IN  ARAD_TDM_FTMH *info
  );

void
  ARAD_TDM_FTMH_INFO_print(
    SOC_SAND_IN  ARAD_TDM_FTMH_INFO *info
  );
void
  ARAD_TDM_MC_STATIC_ROUTE_INFO_print(
    SOC_SAND_IN  ARAD_TDM_MC_STATIC_ROUTE_INFO *info
  );

void
  ARAD_TDM_DIRECT_ROUTING_INFO_print(
    SOC_SAND_IN  ARAD_TDM_DIRECT_ROUTING_INFO *info
  );

#endif /* ARAD_DEBUG_IS_LVL1 */

/* } */

#include <soc/dpp/SAND/Utils/sand_footer.h>

/* } __ARAD_API_TDM_INCLUDED__*/
#endif

