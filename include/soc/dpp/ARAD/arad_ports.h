/* $Id: arad_ports.h,v 1.50 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/


#ifndef __ARAD_PORTS_INCLUDED__
/* { */
#define __ARAD_PORTS_INCLUDED__

/*************
 * INCLUDES  *
 *************/
/* { */
#include <soc/dpp/SAND/Utils/sand_header.h>
#include <soc/dpp/SAND/Utils/sand_occupation_bitmap.h>

#include <soc/dpp/ARAD/arad_api_ports.h>
#include <soc/dpp/ARAD/arad_api_general.h>
#include <soc/dpp/ARAD/arad_api_mgmt.h>
#include <soc/dcmn/dcmn_port.h>

/* } */
/*************
 * DEFINES   *
 *************/
/* { */

/*
 * Port to interface mapping register value indicating
 * unmapped interface
 */
#define ARAD_PORTS_IF_UNMAPPED_INDICATION 0xff

/*
 *  Number of bits used for a single channel in Map Ofp To Mirr Channel register
 */
#define ARAD_MAP_OFP_TO_MIRR_CH_FLD_SIZE 6

/* 
 *  Max. Number of queue pairs per Port Scheduler (PS)
 */
#define ARAD_MAX_PORT_TCS_PER_PS 8

#define ARAD_PP_PORT_NDX_MAX                             (ARAD_PORT_NOF_PP_PORTS - 1)

#define ARAD_PORT_EG_MIRROR_NOF_VID_MIRROR_INDICES                 (7)

#define ARAD_PORT_INVALID_RCY_PORT                                (-1)

#define ARAD_PORT_INVALID_EGQ_INTERFACE                            (-1)
/*
 * Encoding 8/10 effective rate 
 * The actual traffic rate which can be sent thorogh a link using 8/10 encoding 
 */
#define ARAD_PORT_PCS_8_10_EFFECTIVE_RATE_PERCENT                              (80)

/*Bits 2:0 defines the MSB of System Header profile, by {0x1, VSQ-FC-Type} */
#define ARAD_PORT_INPORT_KEY_SYST_HDR_PROFILE_LSB              (0)
#define ARAD_PORT_INPORT_KEY_SYST_HDR_PROFILE_MSB              (1)
#define ARAD_PORT_INPORT_KEY_SYST_HDR_PROFILE_LENGTH           (ARAD_PORT_INPORT_KEY_SYST_HDR_PROFILE_MSB - ARAD_PORT_INPORT_KEY_SYST_HDR_PROFILE_LSB + 1)
#define ARAD_PORT_INPORT_KEY_SYST_HDR_PROFILE2_VALUE            (0x1)
#define ARAD_PORT_INPORT_KEY_SYST_HDR_PROFILE2_LSB              (2)
#define ARAD_PORT_INPORT_KEY_SYST_HDR_PROFILE2_MSB              (2)
#define ARAD_PORT_INPORT_KEY_SYST_HDR_PROFILE2_LENGTH           (ARAD_PORT_INPORT_KEY_SYST_HDR_PROFILE2_MSB - ARAD_PORT_INPORT_KEY_SYST_HDR_PROFILE2_LSB + 1)

/*for XGS inport key gen var*/
#define ARAD_PORT_INPORT_KEY_DIFFSERV_LSB               (0)
#define ARAD_PORT_INPORT_KEY_DIFFSERV_LENGTH            (1)
#define ARAD_PORT_INPORT_KEY_NOT_DIFFSERV_LSB           (4)
#define ARAD_PORT_INPORT_KEY_NOT_DIFFSERV_LENGTH        (1)
#define ARAD_PORT_INPORT_KEY_RECYCLE_PORT_LSB           (8)
#define ARAD_PORT_INPORT_KEY_RECYCLE_PORT_LENGTH        (20)

/* At least 3*/
#define ARAD_PORT_PTC_KEY_BYTES_TO_RMV_LSB              (8)
#define ARAD_PORT_PTC_KEY_BYTES_TO_RMV_MSB              (15)
#define ARAD_PORT_PTC_KEY_BYTES_TO_RMV_LENGTH           (ARAD_PORT_PTC_KEY_BYTES_TO_RMV_MSB - ARAD_PORT_PTC_KEY_BYTES_TO_RMV_LSB + 1)


#define ARAD_MAPPED_PP_PORT_LENGTH                      (9)


#define ARAD_PORT_MAX_PON_PORT(unit) (SOC_IS_JERICHO(unit) ? (15) : (7))
#define ARAD_PORT_VIRTUAL_PORT_PON_RESERVE_ENTRY_NOF (256)
#define ARAD_PORT_MAX_TUNNEL_ID_IN_MAX_PON_PORT (2047 - ARAD_PORT_VIRTUAL_PORT_PON_RESERVE_ENTRY_NOF)
#define ARAD_PORT_MAX_TUNNEL_ID_IN_AVERAGE_MODE(unit) (SOC_IS_JERICHO(unit) ? 2032 : 2016)


#define ARAD_PORT_LAG_TOTAL_MEMBER_NOF_BITS       (SOC_IS_QUX(unit)? 11 : 14)
#define ARAD_PORT_LAG_GROUP_NOF_BITS              (SOC_IS_QUX(unit)? 5 : 10)


/* } */

/*************
 * MACROS    *
 *************/
/* { */
#define ARAD_PORT_IS_INCOMING(dir) \
  SOC_SAND_NUM2BOOL(((dir) == ARAD_PORT_DIRECTION_INCOMING  ) || ((dir) == ARAD_PORT_DIRECTION_BOTH))

#define ARAD_PORT_IS_OUTGOING(dir) \
  SOC_SAND_NUM2BOOL(((dir) == ARAD_PORT_DIRECTION_OUTGOING  ) || ((dir) == ARAD_PORT_DIRECTION_BOTH))

#define ARAD_BASE_PORT_TC2PS(base_port_tc) \
  ((base_port_tc)/ARAD_MAX_PORT_TCS_PER_PS)

#define ARAD_PS2BASE_PORT_TC(ps) \
  (ps*ARAD_MAX_PORT_TCS_PER_PS)

#define ARAD_IS_START_OF_PS(port_tc) \
  (port_tc % ARAD_MAX_PORT_TCS_PER_PS == 0)

#define ARAD_PS_CPU_FIRST_VALID_QPAIR 192

#define ARAD_PS_CPU_VALID_RANGE(ps) \
  (SOC_SAND_IS_VAL_IN_RANGE(ARAD_PS2BASE_PORT_TC(ps),ARAD_PS_CPU_FIRST_VALID_QPAIR,ARAD_EGR_BASE_Q_PAIRS_NDX_MAX))

#define ARAD_PS_RCPU_VALID_RANGE(ps) \
  (SOC_SAND_IS_VAL_IN_RANGE(ARAD_PS2BASE_PORT_TC(ps),0,96))
/* } */

/*************
 * TYPE DEFS *
 *************/
/* { */

typedef struct 
{
  /* 
   * Bimtap occupation to allocate reassembly context
   */
  SOC_SAND_OCC_BM_PTR
    reassembly_ctxt_occ;

} ARAD_REASSBMEBLY_CTXT;

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

int
  arad_port_parse_header_type_unsafe(
    SOC_SAND_IN int         unit,
    SOC_SAND_IN soc_port_t  port,
    SOC_SAND_IN uint32      port_parser_program_pointer,
    SOC_SAND_OUT ARAD_PORT_HEADER_TYPE * header_type_in
  );

/*********************************************************************
* NAME:
*     arad_ports_init
* FUNCTION:
*     Initialization of the Arad blocks configured in this module.
* INPUT:
*  SOC_SAND_IN  int                 unit -
*     Identifier of the device to access.
* RETURNS:
*   OK or ERROR indication.
* REMARKS:
*   Called as part of the initialization sequence.
*********************************************************************/
uint32
  arad_ports_init(
    SOC_SAND_IN  int                 unit
  );

uint32
  arad_ports_lag_mode_set_unsafe(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  ARAD_PORT_LAG_MODE        lag_mode
  );

uint32
  arad_ports_lag_mode_get_unsafe(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_OUT ARAD_PORT_LAG_MODE        *lag_mode,
    SOC_SAND_OUT uint32                     *sys_lag_port_id_nof_bits
  );

uint32
  arad_ports_lag_nof_lag_groups_get_unsafe(
    SOC_SAND_IN  int                 unit
  );

uint32
  arad_ports_lag_nof_lag_entries_get_unsafe(
    SOC_SAND_IN  int                 unit
  );

uint32
  arad_ports_logical_sys_id_build(
    SOC_SAND_IN  int  unit,
    SOC_SAND_IN  uint8 is_lag_not_phys,
    SOC_SAND_IN  uint32  lag_id,
    SOC_SAND_IN  uint32  lag_member_id,
    SOC_SAND_IN  uint32  sys_phys_port_id,
    SOC_SAND_OUT uint32  *sys_logic_port_id
  );

uint32
  arad_ports_logical_sys_id_parse(
      SOC_SAND_IN  int  unit,
    SOC_SAND_IN  uint32  sys_logic_port_id,
    SOC_SAND_OUT uint8 *is_lag_not_phys,
    SOC_SAND_OUT uint32 *lag_id,
    SOC_SAND_OUT uint32 *lag_member_id,
    SOC_SAND_OUT uint32 *sys_phys_port_id
  );
/*********************************************************************
* NAME:
*   arad_sys_phys_to_local_port_map_set_unsafe
* TYPE:
*   PROC
* FUNCTION:
*   Map System Physical FAP Port to a <mapped_fap_id, mapped_fap_port_id>
*   pair. The mapping is unique - single System Physical
*   Port is mapped to a single local port per specified
*   device. This configuration effects: 1. Resolving
*   destination FAP Id and OFP Id 2. Per-port pruning
* INPUT:
*   SOC_SAND_IN  int                                 unit -
*     Identifier of the device to access.
*   SOC_SAND_IN  uint32                                 sys_phys_port_ndx -
*     The index of system physical port. Range: 0 - 4095.
*   SOC_SAND_IN  uint32                                 mapped_fap_id -
*     The device id of the port that is mapped. Range: 0 -
*     2047.
*   SOC_SAND_IN  uint32                                 mapped_fap_port_id -
*     Local (per device) FAP Port id. Range: 0 - 79.
* REMARKS:
*   1. The mapping is identical for incoming and outgoing
*   FAP Ports. 2. Mapping the device to system fap port must
*   be performed before calling this API.
* RETURNS:
*   OK or ERROR indication.
*********************************************************************/
uint32
  arad_sys_phys_to_local_port_map_set_unsafe(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  uint32                 sys_phys_port_ndx,
    SOC_SAND_IN  uint32                 mapped_fap_id,
    SOC_SAND_IN  uint32                 mapped_fap_port_id
  );

/*********************************************************************
*     Map System Virtual FAP Port to a <fap_id, port_id>
*     pair.
*     Enable mapping of several Virtual TM ports to the same
*     Local port.
*     Help the user to distribute the rate of a single physical
*     port to different purposes.
*     This configuration effects:
*     Resolving destination FAP Id and OFP Id
*********************************************************************/
uint32
  arad_sys_virtual_port_to_local_port_map_set_unsafe(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  uint32                 sys_phys_port_ndx,
    SOC_SAND_IN  uint32                 mapped_fap_id,
    SOC_SAND_IN  uint32                 mapped_fap_port_id
  );

/*********************************************************************
* NAME:
*     arad_sys_phys_to_local_port_map_verify
* TYPE:
*   PROC
* DATE:
*   Apr 21 2008
* FUNCTION:
*     Map System Physical FAP Port to a <mapped_fap_id, mapped_fap_port_id>
*     pair. The mapping is unique - single System Physical
*     Port is mapped to a single local port per specified
*     device. This configuration effects: 1. Resolving
*     destination FAP Id and OFP Id 2. Per-port pruning
* INPUT:
*  SOC_SAND_IN  int                 unit -
*     Identifier of the device to access.
*  SOC_SAND_IN  uint32                 sys_phys_port_ndx -
*     The index of system physical port. Range: 0 - 4095.
*  SOC_SAND_IN  uint32                 mapped_fap_id -
*     The device id of the port that is mapped.
*  SOC_SAND_IN  uint32                 mapped_fap_port_id -
*     Local (per device) FAP Port id. Range: 0 - 79.
* REMARKS:
*     The mapping is identical for incoming and outgoing FAP
*     Ports.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
int
  arad_sys_phys_to_local_port_map_verify(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  uint32                 sys_phys_port_ndx,
    SOC_SAND_IN  uint32                 mapped_fap_id,
    SOC_SAND_IN  uint32                 mapped_fap_port_id
  );

/*********************************************************************
* NAME:
*     arad_sys_phys_to_local_port_map_get_unsafe
* TYPE:
*   PROC
* DATE:
*   Apr 21 2008
* FUNCTION:
*     Map System Physical FAP Port to a <mapped_fap_id, mapped_fap_port_id>
*     pair. The mapping is unique - single System Physical
*     Port is mapped to a single local port per specified
*     device. This configuration effects: 1. Resolving
*     destination FAP Id and OFP Id 2. Per-port pruning
* INPUT:
*  SOC_SAND_IN  int                 unit -
*     Identifier of the device to access.
*  SOC_SAND_IN  uint32                 sys_phys_port_ndx -
*     The index of system physical port. Range: 0 - 4095.
*  SOC_SAND_OUT uint32                 *mapped_fap_id -
*     The device id of the port that is mapped.
*  SOC_SAND_OUT uint32                 *mapped_fap_port_id -
*     Local (per device) FAP Port id. Range: 0 - 79.
* REMARKS:
*     The mapping is identical for incoming and outgoing FAP
*     Ports.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  arad_sys_phys_to_local_port_map_get_unsafe(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  uint32                 sys_phys_port_ndx,
    SOC_SAND_OUT uint32                 *mapped_fap_id,
    SOC_SAND_OUT uint32                 *mapped_fap_port_id
  );

/*********************************************************************
* NAME:
*     arad_local_to_sys_phys_port_map_get_unsafe
* TYPE:
*   PROC
* FUNCTION:
*     Get a System Physical FAP Port mapped to a FAP port in
*     the local device. The mapping is unique - single System
*     Physical Port is mapped to a single local port per
*     specified device. This configuration effects: 1.
*     Resolving destination FAP Id and OFP Id 2. Per-port
*     pruning
* INPUT:
*  SOC_SAND_IN  int                 unit -
*     Identifier of the device to access.
*  SOC_SAND_IN  uint32                 fap_ndx -
*     Local (per device) FAP Port id. Range: 0 - 79.
*  SOC_SAND_IN  uint32                 fap_local_port_ndx -
*     The device id of the port that is mapped.
*  SOC_SAND_OUT uint32                 *sys_phys_port_id -
*     The index of system physical port. Range: 0 - 4095.
* REMARKS:
*     The mapping is identical for incoming and outgoing FAP
*     Ports.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  arad_local_to_sys_phys_port_map_get_unsafe(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  uint32                 fap_ndx,
    SOC_SAND_IN  uint32                 fap_local_port_ndx,
    SOC_SAND_OUT uint32                 *sys_phys_port_id
  );

/*********************************************************************
* Get the System Physical FAP Port (sysport) mapped to the given modport
*********************************************************************/
uint32
  arad_modport_to_sys_phys_port_map_get_unsafe(
    SOC_SAND_IN  int     unit,
    SOC_SAND_IN  uint32  fap_id,            /* input FAP ID /module */
    SOC_SAND_IN  uint32  tm_port,           /* input TM port, make a modport with fap_id */
    SOC_SAND_OUT uint32  *sys_phys_port_id  /* output sysport */
  );

/**********************************************************************
 * Function:
 *      arad_port_control_pcs_set_unsafe
 * Purpose:
 *      Set link PCS
 * Parameters:
 *      unit    -    (IN)    unit number.
 *      link_ndx    -     (IN)    Number of link from the ARAD device toward the fabric element.
 *      pcs           -    (IN)    PCS for link
 * Returns:
 *      BCM_E_NONE     No Error  
 *      BCM_E_UNAVAIL  Feature unavailable  
 *      BCM_E_PORT     Invalid Port number specified  
 *      BCM_E_XXX      Error occurred  
 **********************************************************************/

int
  arad_port_control_pcs_set(
    SOC_SAND_IN    int              unit,
    SOC_SAND_IN    soc_port_t       port,
    SOC_SAND_IN    soc_dcmn_port_pcs_t    pcs
  );

uint32
  arad_port_control_pcs_set_verify(
    SOC_SAND_IN    int      unit,
    SOC_SAND_IN    uint32      link_ndx,
    SOC_SAND_IN    soc_dcmn_port_pcs_t pcs
  );
/**********************************************************************
 * Function:
 *      arad_port_control_low_latency_set
 * Purpose:
 *      Enable/Disable low latency
 * Parameters:
 *      unit    -    (IN)    unit number.
 *      link_ndx    -     (IN)    Number of link from the ARAD device toward the fabric element.
 *      value       -    (IN)    0 to disable , non 0 to enable
 * Returns:
 *      BCM_E_NONE     No Error  
 *      BCM_E_XXX      Error occurred  
 **********************************************************************/

soc_error_t
  arad_port_control_low_latency_set(
     SOC_SAND_IN int            unit,
     SOC_SAND_IN soc_port_t     port,
     SOC_SAND_IN int            value
     );

/**********************************************************************
 * Function:
 *      arad_port_control_fec_error_detect_set
 * Purpose:
 *      Enable/Disable Error detetctino for FEC 
 * Parameters:
 *      unit    -    (IN)    unit number.
 *      link_ndx    -     (IN)    Number of link from the ARAD device toward the fabric element.
 *      value       -    (IN)    0 to disable , non 0 to enable
 * Returns:
 *      BCM_E_NONE     No Error  
 *      BCM_E_XXX      Error occurred 
 **********************************************************************/

soc_error_t
  arad_port_control_fec_error_detect_set(
     SOC_SAND_IN int        unit,
     SOC_SAND_IN soc_port_t port,
     SOC_SAND_IN int        value
     );

/**********************************************************************
 * Function:
 *      arad_port_control_low_latency_get
 * Purpose:
 *      Get Low Latency state
 * Parameters:
 *      unit    -    (IN)    unit number.
 *      link_ndx    -     (IN)    Number of link from the ARAD device toward the fabric element.
 *      value       -    (OUT)    0 if disabled, 1 if enabled
 * Returns:
 *      BCM_E_NONE     No Error  
 *      BCM_E_XXX      Error occurred  
 **********************************************************************/

soc_error_t
  arad_port_control_low_latency_get(
     SOC_SAND_IN  int           unit,
     SOC_SAND_IN  soc_port_t    port,
     SOC_SAND_OUT int*          value
     );

/**********************************************************************
 * Function:
 *      arad_port_control_fec_error_detect_get
 * Purpose:
 *      Get Error Detect stat
 * Parameters:
 *      unit    -    (IN)    unit number.
 *      link_ndx    -     (IN)    Number of link from the ARAD device toward the fabric element.
 *      value       -    (OUT)    0 if disabled, 1 if enabled
 * Returns:
 *      BCM_E_NONE     No Error  
 *      BCM_E_XXX      Error occurred  
 **********************************************************************/

soc_error_t
  arad_port_control_fec_error_detect_get(
     SOC_SAND_IN  int           unit,
     SOC_SAND_IN  soc_port_t    port,
     SOC_SAND_OUT int*          value
     );

/**********************************************************************
 * Function:
 *      arad_port_control_pcs_get_unsafe
 * Purpose:
 *      Set link PCS
 * Parameters:
 *      unit    -    (IN)    unit number.
 *      link_ndx    -     (IN)    Number of link from the ARAD device toward the fabric element.
 *      pcs           -    (OUT)    PCS of link
 * Returns:
 *      BCM_E_NONE     No Error  
 *      BCM_E_UNAVAIL  Feature unavailable  
 *      BCM_E_PORT     Invalid Port number specified  
 *      BCM_E_XXX      Error occurred  
 **********************************************************************/

int
  arad_port_control_pcs_get(
    SOC_SAND_IN     int              unit,
    SOC_SAND_IN     soc_port_t       port,
    SOC_SAND_OUT soc_dcmn_port_pcs_t *pcs
  );

/*********************************************************************
* NAME:
*     arad_port_to_interface_map_set_unsafe
* TYPE:
*   PROC
* DATE:
*   Apr 21 2008
* FUNCTION:
*     Maps the specified FAP Port to interface and channel.
* INPUT:
*  SOC_SAND_IN  int                 unit -
*     Identifier of the device to access.
*  SOC_SAND_IN  uint32                 port_ndx -
*     Fap port index. Range: 0 - 79.
*  SOC_SAND_IN  ARAD_PORT_DIRECTION      direction_ndx -
*     The direction of the mapped port
*     (incoming/outgoing/both).
*  SOC_SAND_IN  ARAD_PORT2IF_MAPPING_INFO *info -
*     Port to Interface mapping configuration.
* REMARKS:
*   1. ch_id is only relevant for channelized interfaces -
*   ignored otherwise. 2. To unmap a port without mapping to
*   another interface - use ARAD_IF_ID_INVALID as
*   info.if_id value. 3. The get function is not symmetric
*   to the set function: both incoming and outgoing settings
*   are returned (direction_ndx is not passed).
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  arad_port_to_dynamic_interface_map_set_unsafe(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  int                    core_id,
    SOC_SAND_IN  uint32                 port_ndx,
    SOC_SAND_IN  ARAD_PORT_DIRECTION      direction_ndx,
    SOC_SAND_IN  ARAD_PORT2IF_MAPPING_INFO *info,
    SOC_SAND_IN  uint8                    is_init
  );
uint32
  arad_port_to_interface_map_set_unsafe(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  int                 core_id,
    SOC_SAND_IN  uint32                 port_ndx,
    SOC_SAND_IN  ARAD_PORT_DIRECTION      direction_ndx,
    SOC_SAND_IN  ARAD_PORT2IF_MAPPING_INFO *info,
    SOC_SAND_IN  uint8               is_init
  );


uint32
  arad_port_to_dynamic_interface_unmap_set_unsafe(
    SOC_SAND_IN  int                    unit,
    SOC_SAND_IN  int                    core_id,
    SOC_SAND_IN  uint32                 tm_port,
    SOC_SAND_IN  ARAD_PORT_DIRECTION    direction_ndx
  );
/*********************************************************************
* NAME:
*     arad_port_to_interface_map_verify
* TYPE:
*   PROC
* DATE:
*   Apr 21 2008
* FUNCTION:
*     Maps the specified FAP Port to interface and channel.
* INPUT:
*  SOC_SAND_IN  int                 unit -
*     Identifier of the device to access.
*  SOC_SAND_IN  uint32                 port_ndx -
*     Fap port index. Range: 0 - 79.
*  SOC_SAND_IN  ARAD_PORT_DIRECTION      direction_ndx -
*     The direction of the mapped port
*     (incoming/outgoing/both).
*  SOC_SAND_IN  ARAD_PORT2IF_MAPPING_INFO *info -
*     Port to Interface mapping configuration.
* REMARKS:
*   1. ch_id is only relevant for channelized interfaces -
*   ignored otherwise. 2. To unmap a port without mapping to
*   another interface - use ARAD_IF_ID_INVALID as
*   info.if_id value. 3. The get function is not symmetric
*   to the set function: both incoming and outgoing settings
*   are returned (direction_ndx is not passed).
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  arad_port_to_interface_map_verify(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  uint32                 port_ndx,
    SOC_SAND_IN  ARAD_PORT_DIRECTION      direction_ndx,
    SOC_SAND_IN  ARAD_PORT2IF_MAPPING_INFO *info
  );

/*********************************************************************
* NAME:
*     arad_port_to_interface_map_get_unsafe
* TYPE:
*   PROC
* DATE:
*   Apr 21 2008
* FUNCTION:
*     Maps the specified FAP Port to interface and channel.
* INPUT:
*  SOC_SAND_IN  int                 unit -
*     Identifier of the device to access.
*  SOC_SAND_IN  uint32                 port_ndx -
*     Fap port index. Range: 0 - 79.
*  SOC_SAND_OUT ARAD_PORT2IF_MAPPING_INFO *info_incoming -
*     Incoming port to Interface mapping configuration.
*  SOC_SAND_OUT ARAD_PORT2IF_MAPPING_INFO *info_outgoing -
*     Outgoing port to Interface mapping configuration.
* RETURNS:
*     OK or ERROR indication.
* REMARKS:
*   1. ch_id is only relevant for channelized interfaces -
*   ignored otherwise. 2. To unmap a port without mapping to
*   another interface - use ARAD_IF_ID_INVALID as
*   info.if_id value. 3. The get function is not symmetric
*   to the set function: both incoming and outgoing settings
*   are returned (direction_ndx is not passed).
*********************************************************************/
int
  arad_port_to_interface_map_get(
    SOC_SAND_IN  int                unit,
    SOC_SAND_IN  int                core_id,
    SOC_SAND_IN  uint32             tm_port,
    SOC_SAND_OUT ARAD_INTERFACE_ID  *if_id,
    SOC_SAND_OUT uint32             *channel_id
  );


/********************************************************************* 
* NAME: 
*   arad_ports_is_port_lag_member_unsafe   
* FUNCTION: 
*   Gives LAG information of . 
* INPUT: 
*   SOC_SAND_IN  int                                 unit - 
*     Identifier of the device to access. 
*   SOC_SAND_IN  uint32                                 port_id - 
*     local port index. 
*   SOC_SAND_OUT  uint8                                *is_in_lag - 
*     set to TRUE if the port is a lag member. 
*   SOC_SAND_OUT  uint32                                *lag_ndx - 
*     is is_in_lag = TRUE, set to LAG index of the port. Range: 0 - 255. 
* 
* RETURNS: 
*   OK or ERROR indication. 
*********************************************************************/ 
   
uint32  
    arad_ports_is_port_lag_member_unsafe( 
              SOC_SAND_IN  int                                 unit, 
              SOC_SAND_IN  int                                 core_id,
              SOC_SAND_IN  uint32                                 port_id, 
              SOC_SAND_OUT uint8                                 *is_in_lag, 
              SOC_SAND_OUT uint32                                 *lag_id); 



int arad_ports_lag_fix_next_member_pionter(int unit, uint32 lag_ndx);


/*********************************************************************
* NAME:
*   arad_ports_lag_set_unsafe
* TYPE:
*   PROC
* FUNCTION:
*   Configure a LAG. A LAG is defined by a group of System
*   Physical Ports that compose it. This configuration
*   affects 1. LAG resolution for queuing at the ingress. 2.
*   LAG-based pruning.
* INPUT:
*   SOC_SAND_IN  int                                 unit -
*     Identifier of the device to access.
*   SOC_SAND_IN  uint32                                 lag_ndx -
*     LAG index. Range: 0 - 255. SOC_SAND_OUT uint32
*   SOC_SAND_IN  SOC_PPC_LAG_INFO                      *new_lag_info -
*     Lag members. Maximal number of out-going LAG members is
*     16. The number of incoming LAG members is not limited,
*     and it can be the number of Local FAP ports in each
*     device (80).
* REMARKS:
*   1. Local to system port mapping must be configured
*   before using this API (Incoming and Outgoing) - for LAG
*   pruning. 2. LAG configuration must be consistent
*   system-wide, for incoming and outgoing ports. 3. The
*   same system port can be added multiple times. This
*   affects the load-balancing, according to the number of
*   times the port appears in the LAG. 4. The _get function
*   is not symmetric to the set function: both incoming and
*   outgoing settings are returned (direction_ndx is not
*   passed). 5. For the INCOMING-LAG, the _get function
*   returns only LAG member sys-ports that are mapped to
*   local FAP ports, on the local device. 6. Setting LAG
*   with a group of system ports, will first clean-up any
*   previous configuration of the LAG. For example setting
*   LAG 1 with system members 1,2,3,4 and then setting the
*   same LAG with members 3,4,5,6 will clean up the effect
*   of the previous configuration and set up the new
*   configuration.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  arad_ports_lag_set_unsafe(
    SOC_SAND_IN  int                        unit,
    SOC_SAND_IN  uint32                     lag_ndx,
    SOC_SAND_IN  SOC_PPC_LAG_INFO        *new_lag_info
  );


/*********************************************************************
* NAME:
*   arad_ports_lag_set_verify
* TYPE:
*   PROC
* FUNCTION:
*   Configure a LAG. A LAG is defined by a group of System
*   Physical Ports that compose it. This configuration
*   affects 1. LAG resolution for queuing at the ingress. 2.
*   LAG-based pruning.
* INPUT:
*   SOC_SAND_IN  int                                 unit -
*     Identifier of the device to access.
*   SOC_SAND_IN  ARAD_PORT_DIRECTION                      direction_ndx -
*     LAG direction - incoming, outgoing or both. For
*     symmetrical LAG configuration - set direction as "both".
*     For different Incoming and Outgoing LAG configuration -
*     per-direction configuration is required.
*   SOC_SAND_IN  uint32                                 lag_ndx -
*     LAG index. Range: 0 - 255. SOC_SAND_OUT uint32
*   SOC_SAND_IN  SOC_PPC_LAG_INFO                      *info -
*     Lag members. Maximal number of out-going LAG members is
*     16. The number of incoming LAG members is not limited,
*     and it can be the number of Local FAP ports in each
*     device (80).
* REMARKS:
*   1. Local to system port mapping must be configured
*   before using this API (Incoming and Outgoing) - for LAG
*   pruning. 2. LAG configuration must be consistent
*   system-wide, for incoming and outgoing ports. 3. The
*   same system port can be added multiple times. This
*   affects the load-balancing, according to the number of
*   times the port appears in the LAG. 4. The _get function
*   is not symmetric to the set function: both incoming and
*   outgoing settings are returned (direction_ndx is not
*   passed). 5. For the INCOMING-LAG, the _get function
*   returns only LAG member sys-ports that are mapped to
*   local FAP ports, on the local device. 6. Setting LAG
*   with a group of system ports, will first clean-up any
*   previous configuration of the LAG. For example setting
*   LAG 1 with system members 1,2,3,4 and then setting the
*   same LAG with members 3,4,5,6 will clean up the effect
*   of the previous configuration and set up the new
*   configuration.
* RETURNS:
*   OK or ERROR indication.
*********************************************************************/
uint32
  arad_ports_lag_verify(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  ARAD_PORT_DIRECTION      direction_ndx,
    SOC_SAND_IN  uint32                 lag_ndx,
    SOC_SAND_IN  SOC_PPC_LAG_INFO      *info
  );

/*********************************************************************
* NAME:
*   arad_ports_lag_get_unsafe
* TYPE:
*   PROC
* FUNCTION:
*   Configure a LAG. A LAG is defined by a group of System
*   Physical Ports that compose it. This configuration
*   affects 1. LAG resolution for queuing at the ingress. 2.
*   LAG-based pruning.
* INPUT:
*   SOC_SAND_IN  int                                 unit -
*     Identifier of the device to access.
*   SOC_SAND_IN  ARAD_PORT_DIRECTION                      direction_ndx -
*     LAG direction - incoming, outgoing or both. For
*     symmetrical LAG configuration - set direction as "both".
*     For different Incoming and Outgoing LAG configuration -
*     per-direction configuration is required.
*   SOC_SAND_IN  uint32                                 lag_ndx -
*     LAG index. Range: 0 - 255. SOC_SAND_OUT uint32
*   SOC_SAND_OUT SOC_PPC_LAG_INFO                      *info -
*     Lag members. Maximal number of out-going LAG members is
*     16. The number of incoming LAG members is not limited,
*     and it can be the number of Local FAP ports in each
*     device (80).
* REMARKS:
*   1. Local to system port mapping must be configured
*   before using this API (Incoming and Outgoing) - for LAG
*   pruning. 2. LAG configuration must be consistent
*   system-wide, for incoming and outgoing ports. 3. The
*   same system port can be added multiple times. This
*   affects the load-balancing, according to the number of
*   times the port appears in the LAG. 4. The _get function
*   is not symmetric to the set function: both incoming and
*   outgoing settings are returned (direction_ndx is not
*   passed). 5. For the INCOMING-LAG, the _get function
*   returns only LAG member sys-ports that are mapped to
*   local FAP ports, on the local device. 6. Setting LAG
*   with a group of system ports, will first clean-up any
*   previous configuration of the LAG. For example setting
*   LAG 1 with system members 1,2,3,4 and then setting the
*   same LAG with members 3,4,5,6 will clean up the effect
*   of the previous configuration and set up the new
*   configuration.
* RETURNS:
*   OK or ERROR indication.
*********************************************************************/
uint32
  arad_ports_lag_get_unsafe(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  uint32                 lag_ndx,
    SOC_SAND_OUT SOC_PPC_LAG_INFO      *info_incoming,
    SOC_SAND_OUT SOC_PPC_LAG_INFO      *info_outgoing
  );

void
  arad_ports_lag_mem_id_mark_invalid(
    SOC_SAND_INOUT SOC_PPC_LAG_INFO      *info
  );

/*********************************************************************
* NAME:
*   arad_ports_lag_member_add_unsafe
* TYPE:
*   PROC
* FUNCTION:
*   Add a system port as a member in LAG.
* INPUT:
*   SOC_SAND_IN  int                                 unit -
*     Identifier of the device to access.
*   SOC_SAND_IN  uint32                                 lag_ndx -
*     LAG index. Range: 0 - 255.
*   SOC_SAND_IN  ARAD_PORTS_LAG_MEMBER                    *lag_member -
*     System port to be added as a member, and the
*     member-index.
* REMARKS:
*   1. Replaces arad_ports_lag_sys_port_add (the later kept
*   for backward-compatability).
* RETURNS:
*   OK or ERROR indication.
*********************************************************************/
int
  arad_ports_lag_member_add_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  uint32                              lag_ndx,
    SOC_SAND_IN  ARAD_PORTS_LAG_MEMBER               *add_lag_member
  );



/*********************************************************************
* NAME:
*   arad_ports_lag_sys_port_remove_unsafe
* TYPE:
*   PROC
* FUNCTION:
*   remove a system port from a LAG.
* INPUT:
*   SOC_SAND_IN  int                                 unit -
*     Identifier of the device to access.
*   SOC_SAND_IN  uint32                                 lag_ndx -
*     LAG index. Range: 0 - 255.
*   SOC_SAND_IN  uint32                                 sys_port -
*     system port to be removed as a member.
* REMARKS:
*   If the port cannot be removed (the lag is empty),
*   returns without any action, and without setting an error
*   indication.
* RETURNS:
*   OK or ERROR indication.
*********************************************************************/
int
  arad_ports_lag_sys_port_remove_unsafe(
    SOC_SAND_IN  int                    unit,
    SOC_SAND_IN  uint32                 lag_ndx,
    SOC_SAND_IN  ARAD_PORTS_LAG_MEMBER  *removed_lag_member
  );

/*********************************************************************
* NAME:
*     arad_ports_lag_order_preserve_verify
* TYPE:
*   PROC
* DATE:
*   Oct  3 2007
* FUNCTION:
*     Per-Lag information
* INPUT:
*  SOC_SAND_IN  int                 unit -
*  SOC_SAND_IN  uint32                  rlag_ndx -
*     LAG index. Range: 0 - 255.
*  SOC_SAND_IN  uint8                 is_order_preserving -
*     is_order_preserving If set, the LAG outlif is chosen
*     according to the Hash mechanism, this gives order
*     preserving for all packets. Otherwise LAG Round Robin
*     takes place, and the outlif are chosen sequentially.
*     Note that this case provides better load balancing but
*     does not preserve order of packets.
* RETURNS:
*   OK or ERROR indication.
*********************************************************************/
uint32
  arad_ports_lag_order_preserve_verify(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  uint32                  lag_ndx,
    SOC_SAND_IN  uint8                 is_order_preserving
  );

/*********************************************************************
* NAME:
*     arad_ports_lag_order_preserve_set_unsafe
* TYPE:
*   PROC
* DATE:
*   Oct  3 2007
* FUNCTION:
*     Per-Lag information
* INPUT:
*  SOC_SAND_IN  int                 unit -
*  SOC_SAND_IN  uint32                  lag_ndx -
*     LAG index. Range: 0 - 255.
*  SOC_SAND_IN  uint8                 is_order_preserving -
*     is_order_preserving If set, the LAG outlif is chosen
*     according to the Hash mechanism, this gives order
*     preserving for all packets. Otherwise LAG Round Robin
*     takes place, and the outlif are chosen sequentially.
*     Note that this case provides better load balancing but
*     does not preserve order of packets.
* RETURNS:
*   OK or ERROR indication.
*********************************************************************/
uint32
  arad_ports_lag_order_preserve_set_unsafe(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  uint32                  lag_ndx,
    SOC_SAND_IN  uint8                 is_order_preserving
  );

/*********************************************************************
* NAME:
*     arad_ports_lag_order_preserve_get_unsafe
* TYPE:
*   PROC
* DATE:
*   Oct  3 2007
* FUNCTION:
*     Per-Lag information
* INPUT:
*  SOC_SAND_IN  int                 unit -
*  SOC_SAND_IN  uint32                  lag_ndx -
*     LAG index. Range: 0 - 255.
*  SOC_SAND_OUT uint8                 *is_order_preserving -
*     is_order_preserving If set, the LAG outlif is chosen
*     according to the Hash mechanism, this gives order
*     preserving for all packets. Otherwise LAG Round Robin
*     takes place, and the outlif are chosen sequentially.
*     Note that this case provides better load balancing but
*     does not preserve order of packets.
* RETURNS:
*   OK or ERROR indication.
*********************************************************************/
uint32
  arad_ports_lag_order_preserve_get_unsafe(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  uint32                  lag_ndx,
    SOC_SAND_OUT uint8                 *is_order_preserving
  );

uint32 arad_ports_lag_lb_key_range_set_unsafe(
    SOC_SAND_IN  int                                   unit,
    SOC_SAND_IN  SOC_PPC_LAG_INFO      *info);

/*********************************************************************
* NAME:
*     arad_port_header_type_set_unsafe
* TYPE:
*   PROC
* DATE:
*   Apr 21 2008
* FUNCTION:
*     Configure FAP port header parsing type. The
*     configuration can be for incoming FAP ports, outgoing
*     FAP ports or both.
* INPUT:
*  SOC_SAND_IN  int                 unit -
*     Identifier of the device to access.
*  SOC_SAND_IN  uint32                 port_ndx -
*     FAP Port index. Range: 0 - 79.
*  SOC_SAND_IN  ARAD_PORT_DIRECTION      direction_ndx -
*     The direction of the fap port to configure - incoming,
*     outgoing or both.
*   SOC_SAND_IN  ARAD_PORT_HEADER_TYPE                    header_type -
*     Port header parsing type. Range: 1 - 5. (If the
*     direction is 'incoming', the range is 1 - 4, and for
*     'outgoing', the range is 1 - 3 and 5).
* REMARKS:
*   1. Not all header types are valid for all directions. 2.
*   The get function is not symmetric to the set function:
*   both incoming and outgoing settings are returned
*   (direction_ndx is not passed).
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
int
  arad_port_header_type_set_unsafe(
    SOC_SAND_IN  int                    unit,
    SOC_SAND_IN  int                    core,
    SOC_SAND_IN  uint32                 tm_port,
    SOC_SAND_IN  ARAD_PORT_DIRECTION    direction_ndx,
    SOC_SAND_IN  ARAD_PORT_HEADER_TYPE  header_type
  );

/*********************************************************************
* NAME:
*     arad_port_header_type_verify
* TYPE:
*   PROC
* DATE:
*   Apr 21 2008
* FUNCTION:
*     Configure FAP port header parsing type. The
*     configuration can be for incoming FAP ports, outgoing
*     FAP ports or both.
* INPUT:
*  SOC_SAND_IN  int                 unit -
*     Identifier of the device to access.
*  SOC_SAND_IN  uint32                 port_ndx -
*     FAP Port index. Range: 0 - 79.
*  SOC_SAND_IN  ARAD_PORT_DIRECTION      direction_ndx -
*     The direction of the fap port to configure - incoming,
*     outgoing or both.
*   SOC_SAND_IN  ARAD_PORT_HEADER_TYPE                    header_type -
*     Port header parsing type. Range: 1 - 5. (If the
*     direction is 'incoming', the range is 1 - 4, and for
*     'outgoing', the range is 1 - 3 and 5).
* REMARKS:
*   1. Not all header types are valid for all directions. 2.
*   The get function is not symmetric to the set function:
*   both incoming and outgoing settings are returned
*   (direction_ndx is not passed).
* RETURNS:
*   OK or ERROR indication.
*********************************************************************/
uint32
  arad_port_header_type_verify(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  uint32                 port_ndx,
    SOC_SAND_IN  ARAD_PORT_DIRECTION      direction_ndx,
    SOC_SAND_IN  ARAD_PORT_HEADER_TYPE    header_type
  );

/*********************************************************************
* NAME:
*     arad_port_header_type_get_unsafe
* TYPE:
*   PROC
* DATE:
*   Apr 21 2008
* FUNCTION:
*     Configure FAP port header parsing type. The
*     configuration can be for incoming FAP ports, outgoing
*     FAP ports or both.
* INPUT:
*  SOC_SAND_IN  int                 unit -
*     Identifier of the device to access.
*  SOC_SAND_IN  uint32                 port_ndx -
*     FAP Port index. Range: 0 - 79.
*  SOC_SAND_IN  ARAD_PORT_DIRECTION      direction_ndx -
*     The direction of the fap port to configure - incoming,
*     outgoing or both.
*   SOC_SAND_OUT ARAD_PORT_HEADER_TYPE                    *header_type -
*     Port header parsing type. Range: 1 - 5. (If the
*     direction is 'incoming', the range is 1 - 4, and for
*     'outgoing', the range is 1 - 3 and 5).
* REMARKS:
*   1. Not all header types are valid for all directions. 2.
*   The get function is not symmetric to the set function:
*   both incoming and outgoing settings are returned
*   (direction_ndx is not passed).
* RETURNS:
*   OK or ERROR indication.
*********************************************************************/
int
  arad_port_header_type_get_unsafe(
    SOC_SAND_IN  int                    unit,
    SOC_SAND_IN  int                    core_id,
    SOC_SAND_IN  uint32                 tm_port,
    SOC_SAND_OUT ARAD_PORT_HEADER_TYPE  *header_type_incoming,
    SOC_SAND_OUT ARAD_PORT_HEADER_TYPE  *header_type_outgoing
  );

uint32
  arad_ports_ftmh_extension_verify(
    SOC_SAND_IN  int                   unit,
    SOC_SAND_IN  ARAD_PORTS_FTMH_EXT_OUTLIF ext_option
  );

/*********************************************************************
* NAME:
*     arad_ports_ftmh_extension_set_unsafe
* TYPE:
*   PROC
* DATE:
*   May 12 2008
* FUNCTION:
*     This function sets a system wide configuration of the
*     ftmh. The FTMH has 3 options for the FTMH-extension:
*     always allow, never allow, allow only when the packet is
*     multicast.
* INPUT:
*  SOC_SAND_IN  int                  unit -
*     Identifier of the device to access.
*  SOC_SAND_IN  ARAD_PORTS_FTMH_EXT_OUTLIF ext_option -
*     There 3 options for the FTMH extension.
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  arad_ports_ftmh_extension_set_unsafe(
    SOC_SAND_IN  int                   unit,
    SOC_SAND_IN  ARAD_PORTS_FTMH_EXT_OUTLIF ext_option
  );

/*********************************************************************
* NAME:
*     arad_ports_ftmh_extension_get_unsafe
* TYPE:
*   PROC
* DATE:
*   May 12 2008
* FUNCTION:
*     This function sets a system wide configuration of the
*     ftmh. The FTMH has 3 options for the FTMH-extension:
*     always allow, never allow, allow only when the packet is
*     multicast.
* INPUT:
*  SOC_SAND_IN  int                   unit -
*     Identifier of the device to access.
*  SOC_SAND_OUT ARAD_PORTS_FTMH_EXT_OUTLIF *ext_option -
*     There 3 options for the FTMH extension.
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  arad_ports_ftmh_extension_get_unsafe(
    SOC_SAND_IN  int                   unit,
    SOC_SAND_OUT ARAD_PORTS_FTMH_EXT_OUTLIF *ext_option
  );

/*********************************************************************
* NAME:
*     arad_ports_otmh_extension_set_unsafe
* TYPE:
*   PROC
* DATE:
*   May 12 2008
* FUNCTION:
*     This function sets what extensions are to be added to
*     the OTMH per port. The OTMH has 3 optional extensions:
*     Outlif (always allow/ never allow/ allow only when the
*     packet is multicast.) Source Sys-Port and Destination
*     Sys-Port.
* INPUT:
*  SOC_SAND_IN  int                 unit -
*     Identifier of the device to access.
*  SOC_SAND_IN  ARAD_FAP_PORT_ID         port_ndx -
*     Local port index. Port Index. Range: 0 - 79.
*  SOC_SAND_IN  ARAD_PORTS_OTMH_EXTENSIONS_EN *info -
*     There 3 options for the OTMH-outlif extension, src-port
*     & dest-port extensions Enable/Disable.
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
int
  arad_ports_otmh_extension_set(
    SOC_SAND_IN  int                        unit,
    SOC_SAND_IN  int                        core,
    SOC_SAND_IN  uint32                     tm_port,
    SOC_SAND_IN  ARAD_PORTS_OTMH_EXTENSIONS_EN *info
  );

/*********************************************************************
* NAME:
*     arad_ports_otmh_extension_get_unsafe
* TYPE:
*   PROC
* DATE:
*   May 12 2008
* FUNCTION:
*     This function sets what extensions are to be added to
*     the OTMH per port. The OTMH has 3 optional extensions:
*     Outlif (always allow/ never allow/ allow only when the
*     packet is multicast.) Source Sys-Port and Destination
*     Sys-Port.
* INPUT:
*  SOC_SAND_IN  int                 unit -
*     Identifier of the device to access.
*  SOC_SAND_IN  ARAD_FAP_PORT_ID         port_ndx -
*     Local port index. Port Index. Range: 0 - 79.
*  SOC_SAND_OUT ARAD_PORTS_OTMH_EXTENSIONS_EN *info -
*     There 3 options for the OTMH-outlif extension, src-port
*     & dest-port extensions Enable/Disable.
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  arad_ports_otmh_extension_get_unsafe(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  int                 core_id,
    SOC_SAND_IN  uint32              tm_port,
    SOC_SAND_OUT ARAD_PORTS_OTMH_EXTENSIONS_EN *info
  );

/*********************************************************************
* NAME:
*     arad_port_egr_hdr_credit_discount_type_set_unsafe
* TYPE:
*   PROC
* FUNCTION:
*     Per discount type, set the available egress credit
*     compensation value to adjust the credit rate for the
*     various headers: PP (if present), FTMH, DRAM-CRC,
*     Ethernet-IPG, NIF-CRC.
* INPUT:
*  SOC_SAND_IN  int                 unit -
*     Identifier of the device to access.
*  SOC_SAND_IN  ARAD_PORT_HEADER_TYPE    port_hdr_type_ndx -
*     The port header type for which the credit discount is
*     configured (TM/ETH/RAW).
*  SOC_SAND_IN  ARAD_PORT_EGR_HDR_CR_DISCOUNT_TYPE cr_discnt_type_ndx -
*     The preset (A/B) that is configured.
*  SOC_SAND_IN  ARAD_PORT_EGR_HDR_CR_DISCOUNT_INFO *info -
*     The discount values (signed, can be negative) for the
*     specified discount type and port header type.
* REMARKS:
*     1. The configuration is per port header type. In
*     practice, each port will use the configuration
*     accordingly to its header type. For example, if all
*     ports are TM-ports, the configuration for ETH ports is
*     irrelevant. 2. Credit discount should also be configured
*     in the ingress, using arad_itm_cr_discount_set API. 3.
*     This API only configures the available presets. The
*     specific preset that is used, per port, is configured
*     using the arad_port_egr_hdr_credit_discount_select_set
*     API.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/

uint32
  arad_port_egr_hdr_credit_discount_type_set_unsafe(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  int                 core,
    SOC_SAND_IN  ARAD_PORT_HEADER_TYPE    port_hdr_type_ndx,
    SOC_SAND_IN  ARAD_PORT_EGR_HDR_CR_DISCOUNT_TYPE cr_discnt_type_ndx,
    SOC_SAND_IN  ARAD_PORT_EGR_HDR_CR_DISCOUNT_INFO *info
  );

/*********************************************************************
* NAME:
*     arad_port_egr_hdr_credit_discount_type_verify
* TYPE:
*   PROC
* FUNCTION:
*     Per discount type, set the available egress credit
*     compensation value to adjust the credit rate for the
*     various headers: PP (if present), FTMH, DRAM-CRC,
*     Ethernet-IPG, NIF-CRC.
* INPUT:
*  SOC_SAND_IN  int                 unit -
*     Identifier of the device to access.
*  SOC_SAND_IN  ARAD_PORT_HEADER_TYPE    port_hdr_type_ndx -
*     The port header type for which the credit discount is
*     configured (TM/ETH/RAW).
*  SOC_SAND_IN  ARAD_PORT_EGR_HDR_CR_DISCOUNT_TYPE cr_discnt_type_ndx -
*     The preset (A/B) that is configured.
*  SOC_SAND_IN  ARAD_PORT_EGR_HDR_CR_DISCOUNT_INFO *info -
*     The discount values (signed, can be negative) for the
*     specified discount type and port header type.
* REMARKS:
*     1. The configuration is per port header type. In
*     practice, each port will use the configuration
*     accordingly to its header type. For example, if all
*     ports are TM-ports, the configuration for ETH ports is
*     irrelevant. 2. Credit discount should also be configured
*     in the ingress, using arad_itm_cr_discount_set API. 3.
*     This API only configures the available presets. The
*     specific preset that is used, per port, is configured
*     using the arad_port_egr_hdr_credit_discount_select_set
*     API.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/

uint32
  arad_port_egr_hdr_credit_discount_type_verify(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  ARAD_PORT_HEADER_TYPE    port_hdr_type_ndx,
    SOC_SAND_IN  ARAD_PORT_EGR_HDR_CR_DISCOUNT_TYPE cr_discnt_type_ndx,
    SOC_SAND_IN  ARAD_PORT_EGR_HDR_CR_DISCOUNT_INFO *info
  );

/*********************************************************************
* NAME:
*     arad_port_egr_hdr_credit_discount_type_get_unsafe
* TYPE:
*   PROC
* FUNCTION:
*     Per discount type, set the available egress credit
*     compensation value to adjust the credit rate for the
*     various headers: PP (if present), FTMH, DRAM-CRC,
*     Ethernet-IPG, NIF-CRC.
* INPUT:
*  SOC_SAND_IN  int                 unit -
*     Identifier of the device to access.
*  SOC_SAND_IN  ARAD_PORT_HEADER_TYPE    port_hdr_type_ndx -
*     The port header type for which the credit discount is
*     configured (TM/ETH/RAW).
*  SOC_SAND_IN  ARAD_PORT_EGR_HDR_CR_DISCOUNT_TYPE cr_discnt_type_ndx -
*     The preset (A/B) that is configured.
*  SOC_SAND_OUT ARAD_PORT_EGR_HDR_CR_DISCOUNT_INFO *info -
*     The discount values (signed, can be negative) for the
*     specified discount type and port header type.
* REMARKS:
*     1. The configuration is per port header type. In
*     practice, each port will use the configuration
*     accordingly to its header type. For example, if all
*     ports are TM-ports, the configuration for ETH ports is
*     irrelevant. 2. Credit discount should also be configured
*     in the ingress, using arad_itm_cr_discount_set API. 3.
*     This API only configures the available presets. The
*     specific preset that is used, per port, is configured
*     using the arad_port_egr_hdr_credit_discount_select_set
*     API.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/

uint32
  arad_port_egr_hdr_credit_discount_type_get_unsafe(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  ARAD_PORT_HEADER_TYPE    port_hdr_type_ndx,
    SOC_SAND_IN  ARAD_PORT_EGR_HDR_CR_DISCOUNT_TYPE cr_discnt_type_ndx,
    SOC_SAND_OUT ARAD_PORT_EGR_HDR_CR_DISCOUNT_INFO *info
  );

/*********************************************************************
* NAME:
*     arad_port_egr_hdr_credit_discount_select_set_unsafe
* TYPE:
*   PROC
* FUNCTION:
*     Select from the available egress credit compensation
*     values to adjust the credit rate for the various
*     headers: PP (if present), FTMH, DRAM-CRC, Ethernet-IPG,
*     NIF-CRC. This API selects the discount type. The values
*     per port header type and discount type are configured
*     using arad_port_egr_hdr_credit_discount_type_set API.
* INPUT:
*  SOC_SAND_IN  int                 unit -
*     Identifier of the device to access.
*  SOC_SAND_IN  uint32                 port_ndx -
*     The index of the port to configure. Range: 0 - 79.
*  SOC_SAND_IN  ARAD_PORT_EGR_HDR_CR_DISCOUNT_TYPE cr_discnt_type -
*     The preset (A/B) that is selected.
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/

uint32
  arad_port_egr_hdr_credit_discount_select_set_unsafe(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  int                 core,
    SOC_SAND_IN  uint32              tm_port,
    SOC_SAND_IN  ARAD_PORT_EGR_HDR_CR_DISCOUNT_TYPE cr_discnt_type
  );

/*********************************************************************
* NAME:
*     arad_port_egr_hdr_credit_discount_select_verify
* TYPE:
*   PROC
* FUNCTION:
*     Select from the available egress credit compensation
*     values to adjust the credit rate for the various
*     headers: PP (if present), FTMH, DRAM-CRC, Ethernet-IPG,
*     NIF-CRC. This API selects the discount type. The values
*     per port header type and discount type are configured
*     using arad_port_egr_hdr_credit_discount_type_set API.
* INPUT:
*  SOC_SAND_IN  int                 unit -
*     Identifier of the device to access.
*  SOC_SAND_IN  uint32                 port_ndx -
*     The index of the port to configure. Range: 0 - 79.
*  SOC_SAND_IN  ARAD_PORT_EGR_HDR_CR_DISCOUNT_TYPE cr_discnt_type -
*     The preset (A/B) that is selected.
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/

uint32
  arad_port_egr_hdr_credit_discount_select_verify(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  uint32                 port_ndx,
    SOC_SAND_IN  ARAD_PORT_EGR_HDR_CR_DISCOUNT_TYPE cr_discnt_type
  );

/*********************************************************************
* NAME:
*     arad_port_egr_hdr_credit_discount_select_get_unsafe
* TYPE:
*   PROC
* FUNCTION:
*     Select from the available egress credit compensation
*     values to adjust the credit rate for the various
*     headers: PP (if present), FTMH, DRAM-CRC, Ethernet-IPG,
*     NIF-CRC. This API selects the discount type. The values
*     per port header type and discount type are configured
*     using arad_port_egr_hdr_credit_discount_type_set API.
* INPUT:
*  SOC_SAND_IN  int                 unit -
*     Identifier of the device to access.
*  SOC_SAND_IN  uint32                 port_ndx -
*     The index of the port to configure. Range: 0 - 79.
*  SOC_SAND_OUT ARAD_PORT_EGR_HDR_CR_DISCOUNT_TYPE *cr_discnt_type -
*     The preset (A/B) that is selected.
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/

uint32
  arad_port_egr_hdr_credit_discount_select_get_unsafe(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  int                 core,
    SOC_SAND_IN  uint32              tm_port,
    SOC_SAND_OUT ARAD_PORT_EGR_HDR_CR_DISCOUNT_TYPE *cr_discnt_type
  );

uint32 arad_port_stacking_info_verify(
    SOC_SAND_IN  int                              unit,
    SOC_SAND_IN  uint32                              local_port_ndx,
    SOC_SAND_IN  uint32                              is_stacking,
    SOC_SAND_IN  uint32                              peer_tmd);

uint32 arad_port_stacking_info_set_unsafe(
    SOC_SAND_IN  int                              unit,
    SOC_SAND_IN  int                              core_id,
    SOC_SAND_IN  uint32                              local_port_ndx,
    SOC_SAND_IN  uint32                              is_stacking,
    SOC_SAND_IN  uint32                              peer_tmd);

uint32 arad_port_stacking_info_get_unsafe(
    SOC_SAND_IN  int                              unit,
    SOC_SAND_IN  int                              core_id,
    SOC_SAND_IN  uint32                              local_port_ndx,
    SOC_SAND_OUT  uint32                              *is_stacking,
    SOC_SAND_OUT  uint32                              *peer_tmd);

uint32 arad_port_stacking_route_history_bitmap_set_unsafe(
    SOC_SAND_IN  int                              unit,
    SOC_SAND_IN  int                              core_id,
    SOC_SAND_IN  uint32                           tm_port,
    SOC_SAND_IN  SOC_TMC_STACK_EGR_PROG_TM_PORT_PROFILE_STACK tm_port_profile_stack,
    SOC_SAND_IN  uint32                              bitmap);

/*********************************************************************
* NAME:
 *   arad_port_pp_port_set_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Configure the Port profile for ports of type TM and Raw.
 * INPUT:
 *   SOC_SAND_IN  int                    unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  uint32                    pp_port_ndx -
 *     TM Port Profile Index. Range: 0 - 63.
 *   SOC_SAND_IN  ARAD_PORT_PP_PORT_INFO     *info -
 *     Attributes of the TM Port Profile. Ignored for Raw
 *     Ports.
 *   SOC_SAND_OUT SOC_SAND_SUCCESS_FAILURE         *success -
 *     If True, then the TM Port Profile is added. Otherwise,
 *     not enough resources may be available.
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
int
  arad_port_pp_port_set_unsafe(
    SOC_SAND_IN  int                        unit,
    SOC_SAND_IN  int                        core,
    SOC_SAND_IN  uint32                     pp_port,
    SOC_SAND_IN  ARAD_PORT_PP_PORT_INFO     *info,
    SOC_SAND_OUT SOC_SAND_SUCCESS_FAILURE   *success
  );

uint32
  arad_port_pp_port_set_verify(
    SOC_SAND_IN  int                    unit,
    SOC_SAND_IN  uint32                    pp_port_ndx,
    SOC_SAND_IN  ARAD_PORT_PP_PORT_INFO     *info
  );

/*********************************************************************
* NAME:
 *   arad_port_pp_port_get_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Get the Port profile settings.
 * INPUT:
 *   SOC_SAND_IN  int                    unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  uint32                    pp_port_ndx -
 *     TM Port Profile Index. Range: 0 - 63.
 *   SOC_SAND_OUT ARAD_PORT_PP_PORT_INFO     *info -
 *     Attributes of the TM Port Profile
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
int
  arad_port_pp_port_get_unsafe(
    SOC_SAND_IN  int                        unit,
    SOC_SAND_IN  int                        core,
    SOC_SAND_IN  uint32                     pp_port,
    SOC_SAND_OUT ARAD_PORT_PP_PORT_INFO     *info
  );

uint32
  arad_port_pp_port_get_verify(
    SOC_SAND_IN  int                    unit,
    SOC_SAND_IN  uint32                    pp_port_ndx
  );

/*********************************************************************
* NAME:
 *   arad_port_to_pp_port_map_set_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Map the Port to its Port profile for ports of type TM
 *   and Raw.
 * INPUT:
 *   SOC_SAND_IN  int                       unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  uint32                       port_ndx -
 *     TM Port Index. Range: 0 - 79.
 *   SOC_SAND_IN  uint32    pp_port -
 *     Mapping of the TM Port to its Profile
 * REMARKS:
 *   TM and Raw Ports can be mapped only to existing TM Port
 *   Profile. To add a new TM Port Profile, use the
 *   arad_port_pp_port_set API.
 * RETURNS:
 *   OK or ERROR indication.
 *********************************************************************/
int
  arad_port_to_pp_port_map_set_unsafe(
    SOC_SAND_IN  int                        unit,
    SOC_SAND_IN  soc_port_t                 port,
    SOC_SAND_IN  ARAD_PORT_DIRECTION        direction_ndx
  );

int
  arad_port_to_pp_port_map_set_verify(
    SOC_SAND_IN  int                    unit,
    SOC_SAND_IN  soc_port_t             port,
    SOC_SAND_IN  ARAD_PORT_DIRECTION    direction_ndx
  );

uint32
  arad_port_to_pp_port_map_get_verify(
    SOC_SAND_IN  int                    unit,
    SOC_SAND_IN  uint32                 port_ndx
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "arad_port_to_pp_port_map_set_unsafe" API.
 *     Refer to "arad_port_to_pp_port_map_set_unsafe" API for
 *     details.
*********************************************************************/
uint32
  arad_port_to_pp_port_map_get_unsafe(
    SOC_SAND_IN  int                    unit,
    SOC_SAND_IN  int                    core_id,
    SOC_SAND_IN  uint32                    port_ndx,
    SOC_SAND_OUT uint32                    *pp_port_in,
    SOC_SAND_OUT uint32                    *pp_port_out
  );

uint32
  arad_ports_fap_and_nif_type_match_verify(
    SOC_SAND_IN ARAD_INTERFACE_ID if_id,
    SOC_SAND_IN ARAD_FAP_PORT_ID  fap_port_id
  );

int
  arad_ports_fap_port_id_cud_extension_verify(
    SOC_SAND_IN  int                unit,
    SOC_SAND_IN  ARAD_FAP_PORT_ID   port_id
  );

/*********************************************************************
* NAME:
 *   arad_ports_init_interfaces_context_map
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Initialize base context mapping for all NIF interfaces.
 * INPUT:
 *   SOC_SAND_IN  int                       unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  ARAD_INIT_PORTS    info -
 *     Init information of ports. 
 * REMARKS:
 *   Must be called before port to interface map APIs
 * RETURNS:
 *   OK or ERROR indication.
 *********************************************************************/
uint32
  arad_ports_init_interfaces_context_map(
    SOC_SAND_IN int         unit,
    SOC_SAND_IN ARAD_INIT_PORTS   *info
  );


/*********************************************************************
* NAME:
 *   arad_ports_init_interfaces_dynamic_context_map
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Initialize base context mapping for all NIF interfaces.
 * INPUT:
 *   SOC_SAND_IN  int                       unit -
 *     Identifier of the device to access.
 * REMARKS:
 *   Must be called before port to interface map APIs
 * RETURNS:
 *   OK or ERROR indication.
 *********************************************************************/
uint32
  arad_ports_init_interfaces_dynamic_context_map(
    SOC_SAND_IN int         unit
  );


/*********************************************************************
* NAME:
 *   arad_ports_init_interfaces_erp_setting
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Initialize ERP port so to find suitable interface.
 * INPUT:
 *   SOC_SAND_IN  int                       unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  ARAD_INIT_PORTS    info -
 *     Init information of ports. 
 * REMARKS:
 *   Must be called before port to interface map APIs
 * RETURNS:
 *   OK or ERROR indication.
 *********************************************************************/
uint32
  arad_ports_init_interfaces_erp_setting(
    SOC_SAND_IN int         unit,
    SOC_SAND_IN ARAD_INIT_PORTS   *info
  );

uint32
  ARAD_PORT_PP_PORT_INFO_verify(
    SOC_SAND_IN  ARAD_PORT_PP_PORT_INFO *info
  );

soc_error_t arad_port_prbs_tx_enable_set(int unit, soc_port_t port, soc_dcmn_port_prbs_mode_t mode, int value);
soc_error_t arad_port_prbs_tx_enable_get(int unit, soc_port_t port, soc_dcmn_port_prbs_mode_t mode, int* value);
soc_error_t arad_port_prbs_rx_enable_set(int unit, soc_port_t port, soc_dcmn_port_prbs_mode_t mode, int value);
soc_error_t arad_port_prbs_rx_enable_get(int unit, soc_port_t port, soc_dcmn_port_prbs_mode_t mode, int* value);
soc_error_t arad_port_prbs_rx_status_get(int unit, soc_port_t port, soc_dcmn_port_prbs_mode_t mode, int* value);
soc_error_t arad_port_prbs_tx_invert_data_set(int unit, soc_port_t port, soc_dcmn_port_prbs_mode_t mode, int value);
soc_error_t arad_port_prbs_tx_invert_data_get(int unit, soc_port_t port, soc_dcmn_port_prbs_mode_t mode, int* value);

/*
 * Function:
 *      arad_port_speed_max
 * Purpose:
 *      Get the current operating speed of a port
 * Parameters:
 *      unit - Unit #.
 *      port - port #.
 *      speed - (OUT) Value in megabits/sec (10, 100, etc)
 * Returns:
 *      SOC_E_NONE
 *      SOC_E_XXX
 */
soc_error_t 
arad_port_speed_max(int unit, soc_port_t port, int *speed);
  
/* Allocate and return a free recycle interface channel and a free reassembly context, by allocating an ITM port.
   Indication of OTM port is needed to understand which properties to configure for ITM port. */
uint32
  alloc_reassembly_context_and_recycle_channel_unsafe(
    int unit,
    int core_id,
    uint32 out_pp_port,
    uint32 *channel,
    uint32 *reassembly_ctxt
  );

/* release a used reassembly context, by releasing its ITM port, also release its reassembly context */
uint32
  release_reassembly_context_and_mirror_channel_unsafe(
    int unit,
    int core,
    uint32 out_pp_port,
    uint32 channel
  );

/*********************************************************************
* NAME:
*     arad_port_ingr_map_write_val_unsafe
* TYPE:
*   PROC
* FUNCTION:
*     Write the specified mapping value to a device.
*     Maps/Unmaps Incoming FAP Port to ingress Interface
* INPUT:
*  SOC_SAND_IN  int                 unit -
*     Identifier of the device to access.
*  SOC_SAND_IN  uint32            port_ndx -
*     FAP Port index. Range: 0-255.
*  SOC_SAND_IN  uint8            is_mapped -
*     If TRUE - map. If FALSE - unmap.
*  SOC_SAND_IN  ARAD_PORT2IF_MAPPING_INFO   *map_info
*     Interface and channel to map.
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  arad_port_ingr_map_write_val_unsafe(
    SOC_SAND_IN  int            unit,
    SOC_SAND_IN  uint32            port_ndx,
    SOC_SAND_IN  uint8            is_mapped,
    SOC_SAND_IN  ARAD_PORT2IF_MAPPING_INFO   *map_info
  );

/*********************************************************************
* NAME:
*     arad_port_ingr_reassembly_context_get
* TYPE:
*   PROC
* FUNCTION:
*     Get reassembly context and port termination context of the port.
* INPUT:
*  SOC_SAND_IN  int                 unit -
*     Identifier of the device to access.
*  SOC_SAND_IN  uint32              port -
*     FAP Port.
*  SOC_SAND_OUT  uint32            *port_termination_context -
*     Port termination context.
*  SOC_SAND_OUT  uint32            *reassembly_context -
*     Reassembly context.
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  arad_port_ingr_reassembly_context_get(
    SOC_SAND_IN int         unit,
    SOC_SAND_IN soc_port_t  port,
    SOC_SAND_OUT uint32     *port_termination_context,
    SOC_SAND_OUT uint32     *reassembly_context
  );

/* } */

/*
 * direct lb_key set/get  
*/
uint32 
  arad_port_direct_lb_key_set( 
    SOC_SAND_IN int unit, 
    SOC_SAND_IN int core_id,
    SOC_SAND_IN uint32 local_port,
    SOC_SAND_IN uint32 min_lb_key,
    SOC_SAND_IN uint32 set_min,
    SOC_SAND_IN uint32 max_lb_key,
    SOC_SAND_IN uint32 set_max
   );

uint32 
  arad_port_direct_lb_key_get(
    SOC_SAND_IN int unit, 
    SOC_SAND_IN int core_id,
    SOC_SAND_IN uint32  local_port,
    SOC_SAND_OUT uint32 *min_lb_key,
    SOC_SAND_OUT uint32 *max_lb_key
  );

/*
 * direct lb_key min/max set/get
*/

uint32 
  arad_port_direct_lb_key_min_set_unsafe(
    SOC_SAND_IN int unit, 
    SOC_SAND_IN int core_id,
    SOC_SAND_IN uint32  local_port,
    SOC_SAND_IN uint32 min_lb_key
   );

uint32 
  arad_port_direct_lb_key_max_set_unsafe(
    SOC_SAND_IN int unit,
    SOC_SAND_IN int core_id, 
    SOC_SAND_IN uint32  local_port,
    SOC_SAND_IN uint32 max_lb_key
   );

uint32 
  arad_port_direct_lb_key_min_get_unsafe(
    SOC_SAND_IN int unit, 
    SOC_SAND_IN int core_id, 
    SOC_SAND_IN uint32  local_port,
    uint32* min_lb_key
    );

uint32 
  arad_port_direct_lb_key_max_get_unsafe(
    SOC_SAND_IN int unit, 
    SOC_SAND_IN int core_id, 
    SOC_SAND_IN uint32  local_port,
    uint32* max_lb_key
    );

/*
 * direct lb_key set/get verify
*/

uint32 
  arad_port_direct_lb_key_set_verify(
    SOC_SAND_IN int unit, 
    SOC_SAND_IN uint32  local_port,
    SOC_SAND_IN uint32 lb_key
   );

uint32 
  arad_port_direct_lb_key_get_verify(
    SOC_SAND_IN int unit, 
    SOC_SAND_IN uint32  local_port
    );

#ifdef BCM_88660_A0
/*********************************************************************
* NAME:
*     arad_port_synchronize_lb_key_tables_at_egress_unsafe
* TYPE:
*   PROC
* FUNCTION:
*     Synchronized lb-key range tables according to the used table.
* INPUT:
*  SOC_SAND_IN  int                  unit -
*     Identifier of the device to access.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
   arad_port_synchronize_lb_key_tables_at_egress_unsafe(
   SOC_SAND_IN int unit
    );

/*********************************************************************
* NAME:
*     arad_port_switch_lb_key_tables_unsafe
* TYPE:
*   PROC
* FUNCTION:
*     use the shadow table instead of the primary table.
* INPUT:
*  SOC_SAND_IN  int                  unit -
*     Identifier of the device to access.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/

uint32
  arad_port_switch_lb_key_tables_unsafe(
     SOC_SAND_IN int unit
    );

#endif /* BCM_88660_A0 */

/*
 * Swap get/set functions
 */
/*********************************************************************
* NAME:
*     arad_ports_swap_set_verify
* TYPE:
*   PROC
* FUNCTION:
*     Verify the port swap configuration is valid for the supplied
*     port.
* INPUT:
*  SOC_SAND_IN  int                  unit -
*     Identifier of the device to access.
*  SOC_SAND_IN  ARAD_FAP_PORT_ID        port_ndx -
*     FAP Port index. Range: 0-255.
*  SOC_SAND_IN ARAD_PORTS_SWAP_INFO     *ports_swap_info -
*     A pointer to the port's swap configuration
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32 arad_ports_swap_set_verify(
   SOC_SAND_IN int                   unit,
   SOC_SAND_IN ARAD_FAP_PORT_ID         port_ndx,
   SOC_SAND_IN ARAD_PORTS_SWAP_INFO     *ports_swap_info);

/*********************************************************************
* NAME:
*     arad_ports_swap_set_unsafe
* TYPE:
*   PROC
* FUNCTION:
*     Set the port swap configuration to the HW.
* INPUT:
*  SOC_SAND_IN  int                  unit -
*     Identifier of the device to access.
*  SOC_SAND_IN  ARAD_FAP_PORT_ID        port_ndx -
*     FAP Port index. Range: 0-255.
*  SOC_SAND_IN ARAD_PORTS_SWAP_INFO     *ports_swap_info -
*     A pointer to the port's swap configuration
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32 arad_ports_swap_set_unsafe(
   SOC_SAND_IN int                   unit,
   SOC_SAND_IN ARAD_FAP_PORT_ID         port_ndx,
   SOC_SAND_IN ARAD_PORTS_SWAP_INFO     *ports_swap_info);

/*********************************************************************
* NAME:
*     arad_ports_swap_get_verify
* TYPE:
*   PROC
* FUNCTION:
*     Verify the port swap retrieval is valid for the supplied
*     port.
* INPUT:
*  SOC_SAND_IN  int                  unit -
*     Identifier of the device to access.
*  SOC_SAND_IN  ARAD_FAP_PORT_ID        port_ndx -
*     FAP Port index. Range: 0-255.
*  SOC_SAND_IN ARAD_PORTS_SWAP_INFO     *ports_swap_info -
*     A pointer to the returned port swap configuration
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32 arad_ports_swap_get_verify(
   SOC_SAND_IN int                   unit,
   SOC_SAND_IN ARAD_FAP_PORT_ID         port_ndx,
   SOC_SAND_IN ARAD_PORTS_SWAP_INFO     *ports_swap_info);

/*********************************************************************
* NAME:
*     arad_ports_swap_get_unsafe
* TYPE:
*   PROC
* FUNCTION:
*     Get the port swap configuration from the HW.
* INPUT:
*  SOC_SAND_IN  int                  unit -
*     Identifier of the device to access.
*  SOC_SAND_IN  ARAD_FAP_PORT_ID        port_ndx -
*     FAP Port index. Range: 0-255.
*  SOC_SAND_OUT ARAD_PORTS_SWAP_INFO    *ports_swap_info -
*     A pointer to the returned port swap configuration
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32 arad_ports_swap_get_unsafe(
   SOC_SAND_IN int                   unit,
   SOC_SAND_IN ARAD_FAP_PORT_ID         port_ndx,
   SOC_SAND_OUT ARAD_PORTS_SWAP_INFO    *ports_swap_info);

/*********************************************************************
* NAME:
*     arad_swap_info_set_verify
* TYPE:
*   PROC
* FUNCTION:
*     Verify the general swap configuration is valid.
* INPUT:
*  SOC_SAND_IN  int                  unit -
*     Identifier of the device to access.
*  SOC_SAND_IN ARAD_SWAP_INFO           *swap_info -
*     A pointer to the general swap configuration.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32 arad_swap_info_set_verify(
    SOC_SAND_IN  int             unit,
    SOC_SAND_IN  ARAD_SWAP_INFO     *swap_info);

/*********************************************************************
* NAME:
*     arad_swap_info_set_unsafe
* TYPE:
*   PROC
* FUNCTION:
*     Set the general swap configuration to the HW.
* INPUT:
*  SOC_SAND_IN  int                  unit -
*     Identifier of the device to access.
*  SOC_SAND_IN ARAD_SWAP_INFO           *swap_info -
*     A pointer to the general swap configuration.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32 arad_swap_info_set_unsafe(
    SOC_SAND_IN  int             unit,
    SOC_SAND_IN  ARAD_SWAP_INFO     *swap_info);

/*********************************************************************
* NAME:
*     arad_swap_info_get_verify
* TYPE:
*   PROC
* FUNCTION:
*     Verify the general swap retrieval is valid.
* INPUT:
*  SOC_SAND_IN  int                  unit -
*     Identifier of the device to access.
*  SOC_SAND_IN ARAD_SWAP_INFO           *swap_info -
*     A pointer to the general swap configuration.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32 arad_swap_info_get_verify(
    SOC_SAND_IN  int             unit,
    SOC_SAND_IN  ARAD_SWAP_INFO     *swap_info);

/*********************************************************************
* NAME:
*     arad_swap_info_get_unsafe
* TYPE:
*   PROC
* FUNCTION:
*     Retrieve the general swap configuration.
* INPUT:
*  SOC_SAND_IN  int                  unit -
*     Identifier of the device to access.
*  SOC_SAND_OUT ARAD_SWAP_INFO          *swap_info -
*     A pointer to the returned swap configuration.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32 arad_swap_info_get_unsafe(
    SOC_SAND_IN  int             unit,
    SOC_SAND_OUT ARAD_SWAP_INFO     *swap_info);

uint32
  arad_ports_pon_tunnel_info_set_verify(
    SOC_SAND_IN  int                   unit,
    SOC_SAND_IN  ARAD_FAP_PORT_ID         port_ndx,
    SOC_SAND_IN  ARAD_PON_TUNNEL_ID       tunnel,
    SOC_SAND_IN  ARAD_PORTS_PON_TUNNEL_INFO *info
  );

uint32
  arad_ports_pon_tunnel_info_set_unsafe(
    SOC_SAND_IN  int                   unit,
    SOC_SAND_IN  ARAD_FAP_PORT_ID         port_ndx,
    SOC_SAND_IN  ARAD_PON_TUNNEL_ID       tunnel,
    SOC_SAND_IN  ARAD_PORTS_PON_TUNNEL_INFO *info
  );

uint32
  arad_ports_pon_tunnel_info_get_verify(
    SOC_SAND_IN  int                   unit,
    SOC_SAND_IN  ARAD_FAP_PORT_ID         port_ndx,
    SOC_SAND_IN  ARAD_PON_TUNNEL_ID       tunnel,
    SOC_SAND_IN  ARAD_PORTS_PON_TUNNEL_INFO *info
  );

uint32
  arad_ports_pon_tunnel_info_get_unsafe(
    SOC_SAND_IN  int                   unit,
    SOC_SAND_IN  ARAD_FAP_PORT_ID         port_ndx,
    SOC_SAND_IN  ARAD_PON_TUNNEL_ID       tunnel,
    SOC_SAND_OUT ARAD_PORTS_PON_TUNNEL_INFO *info
  );

uint32
arad_port_encap_config_verify(
    SOC_SAND_IN  int                   unit,
    SOC_SAND_IN  ARAD_FAP_PORT_ID         port_ndx,
    SOC_SAND_IN ARAD_L2_ENCAP_INFO       *info
    );

uint32
arad_port_encap_config_get_unsafe(
    SOC_SAND_IN  int                   unit,
    SOC_SAND_IN  int                    core_id,
    SOC_SAND_IN  ARAD_FAP_PORT_ID         port_ndx,
    SOC_SAND_OUT ARAD_L2_ENCAP_INFO       *info
    );

uint32
arad_port_encap_config_set_unsafe(
    SOC_SAND_IN  int                   unit,
    SOC_SAND_IN  int                    core_id,
    SOC_SAND_IN  ARAD_FAP_PORT_ID         port_ndx,
    SOC_SAND_IN ARAD_L2_ENCAP_INFO       *info
    );

uint32
soc_arad_ports_stop_egq(
    SOC_SAND_IN  int     unit,
    SOC_SAND_IN  soc_port_t     port,
    SOC_SAND_OUT uint32     enable);

int
soc_arad_port_control_tx_nif_enable_set(
    SOC_SAND_IN  int        unit,
    SOC_SAND_IN  soc_port_t port,
    SOC_SAND_IN  int        enable);

int
soc_arad_port_control_tx_nif_enable_get(
    SOC_SAND_IN  int        unit,
    SOC_SAND_IN  soc_port_t port,
    SOC_SAND_OUT int        *enable);

int
  arad_port_rx_enable_get(
   int                         unit,
    soc_port_t                  port_ndx,
    int                       *enable
  );

int
  arad_port_rx_enable_set(
   int                         unit,
    soc_port_t                 port_ndx,
    int                        enable
  );

uint32
  arad_ports_programs_info_set_verify(
    SOC_SAND_IN  int                   unit,
    SOC_SAND_IN  ARAD_FAP_PORT_ID         port_ndx,
    SOC_SAND_IN  ARAD_PORTS_PROGRAMS_INFO *info
  );

uint32
  arad_ports_programs_info_set_unsafe(
    SOC_SAND_IN  int                   unit,
    SOC_SAND_IN  ARAD_FAP_PORT_ID         port_ndx,
    SOC_SAND_IN  ARAD_PORTS_PROGRAMS_INFO *info
  );

uint32
  arad_ports_programs_info_get_verify(
    SOC_SAND_IN  int                   unit,
    SOC_SAND_IN  ARAD_FAP_PORT_ID         port_ndx,
    SOC_SAND_IN  ARAD_PORTS_PROGRAMS_INFO *info
  );

uint32
  arad_ports_programs_info_get_unsafe(
    SOC_SAND_IN  int                   unit,
    SOC_SAND_IN  ARAD_FAP_PORT_ID         port_ndx,
    SOC_SAND_OUT ARAD_PORTS_PROGRAMS_INFO *info
  );
/*********************************************************************
* NAME:
*     arad_port_rate_egress_pps_set
* TYPE:
*   PROC
* FUNCTION:
*     Configure the relevant cell shaper:
*     Supported only by ARAD PLUS fabric links.
*     Shaper configurations is shared per FMAC instance.
* INPUT:
*  SOC_SAND_IN  int                  unit -
*     Identifier of the device to access.
*  SOC_SAND_IN  uint32                  port -
*       Port #
*  SOC_SAND_IN  uint32                  pps -
*       Rate - Cells per second.
*  SOC_SAND_IN  uint32                  burst -
*     Max burst allowed.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32 
  arad_port_rate_egress_pps_set (
    SOC_SAND_IN int unit, 
    SOC_SAND_IN soc_port_t port, 
    SOC_SAND_IN uint32 pps, 
    SOC_SAND_IN uint32 burst
    );
/*********************************************************************
* NAME:
*     arad_port_rate_egress_pps_get
* TYPE:
*   PROC
* FUNCTION:
*     Reterive Configuration of the relevant cell shaper:
*     Supported only by ARAD PLUS fabric links.
*     Shaper configurations is shared per FMAC instance.
* INPUT:
*  SOC_SAND_IN  int                  unit -
*     Identifier of the device to access.
*  SOC_SAND_IN  uint32                  port -
*       Port #
*  SOC_SAND_OUT  uint32                  pps -
*       Rate - Cells per second.
*  SOC_SAND_OUT  uint32                  burst -
*     Max burst allowed.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32 
  arad_port_rate_egress_pps_get (
    SOC_SAND_IN int unit, 
    SOC_SAND_IN soc_port_t port, 
    SOC_SAND_OUT uint32 *pps, 
    SOC_SAND_OUT uint32 *burst
    );

/*********************************************************************
* NAME:
*     arad_ports_application_mapping_info_set_unsafe
* TYPE:
*   PROC
* FUNCTION:
*     Allows different mappings for (packet, TM-PTC-port)
*     to PP port and might also apply opposite.
*     For XGS MAC extender, it allows packet mapping
*     (HG.Port,HG.Modid) to PP port.
*     Might be used in the future for other applications that have
*     not typical Port mappings.
* INPUT:
*  SOC_SAND_IN  int                  unit -
*     Identifier of the device to access.
*  SOC_SAND_IN  uint32                  port -
*       Port #
*  SOC_SAND_IN  uint32                  info -
*       Application mapping information.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32 
  arad_ports_application_mapping_info_set_unsafe (
    SOC_SAND_IN int unit, 
    SOC_SAND_IN ARAD_FAP_PORT_ID port_ndx, 
    SOC_SAND_IN ARAD_PORTS_APPLICATION_MAPPING_INFO *info    
    );

uint32
  arad_ports_application_mapping_info_set_verify(
    SOC_SAND_IN  int                   unit,
    SOC_SAND_IN  ARAD_FAP_PORT_ID         port_ndx,
    SOC_SAND_IN ARAD_PORTS_APPLICATION_MAPPING_INFO *info   
  );
/*********************************************************************
* NAME:
*     arad_ports_application_mapping_info_get
* TYPE:
*   PROC
* FUNCTION:
*     Reterive Configuration of the ports mappings according
*     to application.
* INPUT:
*  SOC_SAND_IN  int                  unit -
*     Identifier of the device to access.
*  SOC_SAND_IN  uint32                  port -
*       Port #
*  SOC_SAND_IN  uint32                  info -
*       Application mapping information.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32 
  arad_ports_application_mapping_info_get_unsafe (
    SOC_SAND_IN int unit, 
    SOC_SAND_IN ARAD_FAP_PORT_ID port_ndx, 
    SOC_SAND_INOUT ARAD_PORTS_APPLICATION_MAPPING_INFO *info    
    );

uint32
  arad_ports_application_mapping_info_get_verify(
    SOC_SAND_IN  int                   unit,
    SOC_SAND_IN ARAD_FAP_PORT_ID          port_ndx, 
    SOC_SAND_IN ARAD_PORTS_APPLICATION_MAPPING_INFO *info    
    );

int
  arad_ports_header_type_update(
      int                   unit,
      soc_port_t            port
   );



int
  arad_ports_reference_clock_set(
      int                   unit,
      soc_port_t            port
   );

int
  arad_ports_port_to_nif_id_get(
      SOC_SAND_IN   int                   unit,
      SOC_SAND_IN   int                   core,
      SOC_SAND_IN   uint32                tm_port,
      SOC_SAND_OUT  ARAD_INTERFACE_ID     *if_id
   );

int
  arad_ports_extender_mapping_enable_set(
      int                   unit,
      soc_port_t            port,
      int                   value
   );

int
  arad_ports_extender_mapping_enable_get(
      int                   unit,
      soc_port_t            port,
      int                   *value
   );

int
  arad_ports_tm_port_var_set(
      int                   unit,
      soc_port_t            port,
      int                   value
   );

int
  arad_ports_tm_port_var_get(
      int                   unit,
      soc_port_t            port,
      int                   *value
   );

int
  arad_ports_mirrored_channel_and_context_map(int unit, 
                                            int core, 
                                            uint32 termination_context,
                                            uint32 reassembly_context, 
                                            uint32 channel
                                            );

int 
  get_recycling_port(
     SOC_SAND_IN int unit,
     SOC_SAND_IN int core_id,
     SOC_SAND_IN uint32  pp_port, 
     SOC_SAND_IN uint32 channel, 
     SOC_SAND_OUT soc_port_t *logical_port
     );


#include <soc/dpp/SAND/Utils/sand_footer.h>

/* } __ARAD_PORTS_INCLUDED__*/
#endif


