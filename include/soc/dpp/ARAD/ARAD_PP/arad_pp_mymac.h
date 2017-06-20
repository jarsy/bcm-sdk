/* $Id: arad_pp_mymac.h,v 1.10 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/

#ifndef __ARAD_PP_MYMAC_INCLUDED__
/* { */
#define __ARAD_PP_MYMAC_INCLUDED__

/*************
 * INCLUDES  *
 *************/
/* { */

#include <soc/dpp/SAND/Utils/sand_header.h>
#include <soc/dcmn/error.h>

#include <soc/dpp/SAND/Management/sand_general_macros.h>
#include <soc/dpp/SAND/Management/sand_error_code.h>

#include <soc/dpp/ARAD/ARAD_PP/arad_pp_framework.h>

#include <soc/dpp/PPC/ppc_api_mymac.h>


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
   * Auto generated. Do not edit following section {
   */
  ARAD_PP_MYMAC_MSB_SET = ARAD_PP_PROC_DESC_BASE_MYMAC_FIRST,
  ARAD_PP_MYMAC_MSB_SET_PRINT,
  ARAD_PP_MYMAC_MSB_SET_UNSAFE,
  ARAD_PP_MYMAC_MSB_SET_VERIFY,
  ARAD_PP_MYMAC_MSB_GET,
  ARAD_PP_MYMAC_MSB_GET_PRINT,
  ARAD_PP_MYMAC_MSB_GET_VERIFY,
  ARAD_PP_MYMAC_MSB_GET_UNSAFE,
  ARAD_PP_MYMAC_VSI_LSB_SET,
  ARAD_PP_MYMAC_VSI_LSB_SET_PRINT,
  ARAD_PP_MYMAC_VSI_LSB_SET_UNSAFE,
  ARAD_PP_MYMAC_VSI_LSB_SET_VERIFY,
  ARAD_PP_MYMAC_VSI_LSB_GET,
  ARAD_PP_MYMAC_VSI_LSB_GET_PRINT,
  ARAD_PP_MYMAC_VSI_LSB_GET_VERIFY,
  ARAD_PP_MYMAC_VSI_LSB_GET_UNSAFE,
  SOC_PPC_MYMAC_VRRP_INFO_SET,
  SOC_PPC_MYMAC_VRRP_INFO_SET_PRINT,
  SOC_PPC_MYMAC_VRRP_INFO_SET_UNSAFE,
  SOC_PPC_MYMAC_VRRP_INFO_SET_VERIFY,
  SOC_PPC_MYMAC_VRRP_INFO_GET,
  SOC_PPC_MYMAC_VRRP_INFO_GET_PRINT,
  SOC_PPC_MYMAC_VRRP_INFO_GET_VERIFY,
  SOC_PPC_MYMAC_VRRP_INFO_GET_UNSAFE,
  ARAD_PP_MYMAC_VRRP_MAC_SET,
  ARAD_PP_MYMAC_VRRP_MAC_SET_PRINT,
  ARAD_PP_MYMAC_VRRP_MAC_SET_UNSAFE,
  ARAD_PP_MYMAC_VRRP_MAC_SET_VERIFY,
  ARAD_PP_MYMAC_VRRP_MAC_GET,
  ARAD_PP_MYMAC_VRRP_MAC_GET_PRINT,
  ARAD_PP_MYMAC_VRRP_MAC_GET_VERIFY,
  ARAD_PP_MYMAC_VRRP_MAC_GET_UNSAFE,
  SOC_PPC_MYMAC_TRILL_INFO_SET,
  SOC_PPC_MYMAC_TRILL_INFO_SET_PRINT,
  SOC_PPC_MYMAC_TRILL_INFO_SET_UNSAFE,
  SOC_PPC_MYMAC_TRILL_INFO_SET_VERIFY,
  SOC_PPC_MYMAC_TRILL_INFO_GET,
  SOC_PPC_MYMAC_TRILL_INFO_GET_PRINT,
  SOC_PPC_MYMAC_TRILL_INFO_GET_VERIFY,
  SOC_PPC_MYMAC_TRILL_INFO_GET_UNSAFE,
  ARAD_PP_LIF_MY_BMAC_MSB_SET,
  ARAD_PP_LIF_MY_BMAC_MSB_SET_PRINT,
  ARAD_PP_LIF_MY_BMAC_MSB_SET_UNSAFE,
  ARAD_PP_LIF_MY_BMAC_MSB_SET_VERIFY,
  ARAD_PP_LIF_MY_BMAC_MSB_GET,
  ARAD_PP_LIF_MY_BMAC_MSB_GET_PRINT,
  ARAD_PP_LIF_MY_BMAC_MSB_GET_VERIFY,
  ARAD_PP_LIF_MY_BMAC_MSB_GET_UNSAFE,
  ARAD_PP_LIF_MY_BMAC_PORT_LSB_SET,
  ARAD_PP_LIF_MY_BMAC_PORT_LSB_SET_PRINT,
  ARAD_PP_LIF_MY_BMAC_PORT_LSB_SET_UNSAFE,
  ARAD_PP_LIF_MY_BMAC_PORT_LSB_SET_VERIFY,
  ARAD_PP_LIF_MY_BMAC_PORT_LSB_GET,
  ARAD_PP_LIF_MY_BMAC_PORT_LSB_GET_PRINT,
  ARAD_PP_LIF_MY_BMAC_PORT_LSB_GET_VERIFY,
  ARAD_PP_LIF_MY_BMAC_PORT_LSB_GET_UNSAFE,
  ARAD_PP_MYMAC_GET_PROCS_PTR,
  ARAD_PP_MYMAC_GET_ERRS_PTR,
  /*
   * } Auto generated. Do not edit previous section.
   */

  /*
   * Last element. Do no touch.
   */
  ARAD_PP_MYMAC_PROCEDURE_DESC_LAST
} ARAD_PP_MYMAC_PROCEDURE_DESC;

typedef enum
{
  /*
   * Auto generated. Do not edit following section {
   */
  ARAD_PP_MYMAC_VRRP_ID_NDX_OUT_OF_RANGE_ERR = ARAD_PP_ERR_DESC_BASE_MYMAC_FIRST,
  ARAD_PP_MYMAC_MODE_OUT_OF_RANGE_ERR,
  ARAD_PP_MYMAC_MY_NICK_NAME_OUT_OF_RANGE_ERR,
  /*
   * } Auto generated. Do not edit previous section.
   */

  ARAD_PP_MYMAC_VSI_LSB_OUT_OF_RANGE,
  ARAD_PP_MYMAC_VRRP_IS_DISABLED_ERR,
  ARAD_PP_MYMAC_LOCAL_PORT_OUT_OF_RANGE_ERR,
  ARAD_PP_MYMAC_VSI_OUT_OF_RANGE_ERR,
  ARAD_PP_MY_BMAC_MSB_LOW_BITS_NOT_ZERO_ERR,
  ARAD_PP_MYMAC_SYS_PORT_NON_SINGLE_PORT_NOT_SUPPORTED_ERR,
  ARAD_PP_MYMAC_SYS_PORT_OUT_OF_RANGE,
  ARAD_PP_LIF_MY_BMAC_PORT_LSB_AUX_TABLE_MODE_MISMATCH_ERR,
  ARAD_PP_MYMAC_MODE_IPV6_NOT_SUPPORTED_ERR,
  ARAD_PP_MYMAC_VRRP_ID_NDX_ABOVE_4K_ERR,
  ARAD_PP_MYMAC_VRRP_ID_NDX_ABOVE_2K_ERR,
  ARAD_PP_MYMAC_VRRP_ID_NDX_ABOVE_1K_ERR,
  ARAD_PP_MYMAC_VRRP_ID_NDX_ABOVE_512_ERR,
  ARAD_PP_MYMAC_VRRP_ID_NDX_ABOVE_256_ERR,
  ARAD_PP_MYMAC_VRID_MAP_FULL_ERR,
  ARAD_PP_MYMAC_VRID_MAP_ENTRY_NOT_FOUND_ERR,
  ARAD_PP_MYMAC_CAM_CORRUPTED,
  ARAD_PP_MYMAC_CAM_NOT_FOUND,
  ARAD_PP_MYMAC_OUT_OF_RANGE,

  /*
   * Last element. Do no touch.
   */
  ARAD_PP_MYMAC_ERR_LAST
} ARAD_PP_MYMAC_ERR;

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

uint32
  arad_pp_mymac_init_unsafe(
    SOC_SAND_IN  int                                 unit
  );

/*********************************************************************
* NAME:
 *   arad_pp_mymac_msb_set_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Set the MSB of the MAC address of the device. Used for
 *   ingress termination and egress encapsulation.
 * INPUT:
 *   SOC_SAND_IN  int                                 unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_SAND_PP_MAC_ADDRESS                       *my_mac_msb -
 *     The MSB of the MAC address of the device. The LSBs are
 *     according to VSI.
 * REMARKS:
 *   - Valid for Routing, LSR, VPLS and Trill. - Not valid
 *   for MIM. The B-MyMac is configured by
 *   soc_ppd_lif_my_bmac_msb_set() and
 *   soc_ppd_lif_my_bmac_port_lsb_set()
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_mymac_msb_set_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_SAND_PP_MAC_ADDRESS                       *my_mac_msb
  );

uint32
  arad_pp_mymac_msb_set_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_SAND_PP_MAC_ADDRESS                       *my_mac_msb
  );

uint32
  arad_pp_mymac_msb_get_verify(
    SOC_SAND_IN  int                                 unit
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "arad_pp_mymac_msb_set_unsafe" API.
 *     Refer to "arad_pp_mymac_msb_set_unsafe" API for details.
*********************************************************************/
uint32
  arad_pp_mymac_msb_get_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_OUT SOC_SAND_PP_MAC_ADDRESS                       *my_mac_msb
  );

/*********************************************************************
* NAME:
 *   arad_pp_mymac_vsi_lsb_set_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Set MAC address LSB according to VSI.
 * INPUT:
 *   SOC_SAND_IN  int                                 unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_VSI_ID                              vsi_ndx -
 *     VSI ID. Range: Arad-B: 0-4K. T20E: 0- 64K.
 *   SOC_SAND_IN  SOC_SAND_PP_MAC_ADDRESS                       *my_mac_lsb -
 *     The LSB of the MAC address. MSB is global.
 * REMARKS:
 *   - Ingress (termination): Packets run on this VSI with
 *   this MAC as DA will be forwarded to router/switch
 *   engine. - Egress (SA Encapsulation): Packets that exit
 *   the router/switch via this VSI will have this MAC
 *   address as the packet SA. - Not valid for MIM.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_mymac_vsi_lsb_set_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_VSI_ID                              vsi_ndx,
    SOC_SAND_IN  SOC_SAND_PP_MAC_ADDRESS                       *my_mac_lsb
  );

uint32
  arad_pp_mymac_vsi_lsb_set_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_VSI_ID                              vsi_ndx,
    SOC_SAND_IN  SOC_SAND_PP_MAC_ADDRESS                       *my_mac_lsb
  );

uint32
  arad_pp_mymac_vsi_lsb_get_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_VSI_ID                              vsi_ndx
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "arad_pp_mymac_vsi_lsb_set_unsafe" API.
 *     Refer to "arad_pp_mymac_vsi_lsb_set_unsafe" API for
 *     details.
*********************************************************************/
uint32
  arad_pp_mymac_vsi_lsb_get_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_VSI_ID                              vsi_ndx,
    SOC_SAND_OUT SOC_SAND_PP_MAC_ADDRESS                       *my_mac_lsb
  );

/*********************************************************************
* NAME:
 *   arad_pp_mymac_vrrp_info_set_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Set My-MAC according to Virtual Router Redundancy
 *   Protocol.
 * INPUT:
 *   SOC_SAND_IN  int                                 unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_MYMAC_VRRP_INFO                     *vrrp_info -
 *     VRRP information, including according to which interface
 *     to set the My-MAC lsb (VSI_ALL/VSI_256) and VRID
 *     My-MAC msb match.
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_mymac_vrrp_info_set_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_MYMAC_VRRP_INFO                     *vrrp_info
  );

uint32
  arad_pp_mymac_vrrp_info_set_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_MYMAC_VRRP_INFO                     *vrrp_info
  );

uint32
  arad_pp_mymac_vrrp_info_get_verify(
    SOC_SAND_IN  int                                 unit
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "arad_pp_mymac_vrrp_info_set_unsafe" API.
 *     Refer to "arad_pp_mymac_vrrp_info_set_unsafe" API for
 *     details.
*********************************************************************/
uint32
  arad_pp_mymac_vrrp_info_get_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_OUT SOC_PPC_MYMAC_VRRP_INFO                     *vrrp_info
  );

/*********************************************************************
* NAME:
 *   arad_pp_mymac_vrrp_mac_set_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Enable / Disable MyMac/MyVRID according to VRRP-ID and
 *   Mac Address LSB.
 * INPUT:
 *   SOC_SAND_IN  int                                 unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  uint32                                  vrrp_id_ndx -
 *     VRRP-ID. The port or VSI, according to which to
 *     configure the MAC. Depending on the VRRP_MODE.
 *   SOC_SAND_IN  SOC_SAND_PP_MAC_ADDRESS                       *vrrp_mac_lsb_key -
 *     The LSB of the MAC address. MSB is global.
 *   SOC_SAND_IN  uint8                                 enable -
 *     Whether to enable or disable the given MAC for the given
 *     vrrp_id
 * REMARKS:
 *   - The MAC address MSB is according to the value set by
 *   soc_ppd_mymac_vrrp_info_set()- The MAC address is identified as
 *   MyMAC, according to
 *   'soc_ppd_my_mac_vrrp_mac_set'.- vrrp-id type and
 *   vrrp_mac_lsb range according to vrrp-mode set by
 *   soc_ppd_mymac_vrrp_info_set(): - Per VSI: VRRP-ID is the VSID,
 *   and MAC address LSBs are the bits [0:0] - Per 256 VSIs:
 *   VRRP-ID is the VSID[7:0], and MAC address LSBs are the
 *   bits [4:0]
 *   - Note: bit 0 of enable shows whether to enable/disable this address
 *           bit 1 of enable shows whether to do this for ipv4 or ipv6
 *           (ARAD only)
 *  - It is assumed that this function is called with legal values: vrrp_id_ndx in legal
 *      range and ipv6 enabled only in case the distinction is enabled in the device.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_mymac_vrrp_mac_set_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  uint32                                 vrrp_id_ndx,
    SOC_SAND_IN  SOC_SAND_PP_MAC_ADDRESS                *vrrp_mac_lsb_key,
    SOC_SAND_IN  uint8                                  enable
  );

uint32
  arad_pp_mymac_vrrp_mac_set_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  uint32                                 vrrp_id_ndx,
    SOC_SAND_IN  SOC_SAND_PP_MAC_ADDRESS                *vrrp_mac_lsb_key,
    SOC_SAND_IN  uint8                                  enable
  );

uint32
  arad_pp_mymac_vrrp_mac_get_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  uint32                                 vrrp_id_ndx,
    SOC_SAND_IN  SOC_SAND_PP_MAC_ADDRESS                *vrrp_mac_lsb_key
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "arad_pp_mymac_vrrp_mac_set_unsafe" API.
 *     Refer to "arad_pp_mymac_vrrp_mac_set_unsafe" API for
 *     details.
 *    Note: bit 0 of enable shows if address enabled for ipv4
 *          bit 1 of enable shows if address enabled for ipv6
 *          (ARAD only)
*********************************************************************/
uint32
  arad_pp_mymac_vrrp_mac_get_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  uint32                                 vrrp_id_ndx,
    SOC_SAND_IN  SOC_SAND_PP_MAC_ADDRESS                *vrrp_mac_lsb_key,
    SOC_SAND_OUT uint8                                  *enable
  );

/*********************************************************************
* NAME:
 *   arad_pp_mymac_trill_info_set_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Set TRILL My-Nick-Name and reserved Nick-Name.
 * INPUT:
 *   SOC_SAND_IN  int                                 unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_MYMAC_TRILL_INFO                    *trill_info -
 *     Trill information.
 * REMARKS:
 *   - More TRILL encapsulation settings are under
 *   soc_ppd_eg_encap_trill_info_set() - MyMAC is according to:
 *   soc_ppd_my_mac_lsb_vsi_info_set(); soc_ppd_mymac_msb_info_set()
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_mymac_trill_info_set_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_MYMAC_TRILL_INFO                    *trill_info
  );

uint32
  arad_pp_mymac_trill_info_set_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_MYMAC_TRILL_INFO                    *trill_info
  );

uint32
  arad_pp_mymac_trill_info_get_verify(
    SOC_SAND_IN  int                                 unit
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "arad_pp_mymac_trill_info_set_unsafe" API.
 *     Refer to "arad_pp_mymac_trill_info_set_unsafe" API for
 *     details.
*********************************************************************/
uint32
  arad_pp_mymac_trill_info_get_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_OUT SOC_PPC_MYMAC_TRILL_INFO                    *trill_info
  );

/*********************************************************************
* NAME:
 *   arad_pp_lif_my_bmac_msb_set_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Set the MSB of the My-B-MAC. My-B-MAC forwards the
 *   packets to the I Component, and is added as the SA when
 *   sending toward the backbone network.
 * INPUT:
 *   SOC_SAND_IN  int                                 unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_SAND_PP_MAC_ADDRESS                       *my_bmac_msb -
 *     The MSB of the MAC address of the device. The LSBs are
 *     according to VSI.
 * REMARKS:
 *   - Relevant Only for PBP ports.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_lif_my_bmac_msb_set_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_SAND_PP_MAC_ADDRESS                       *my_bmac_msb
  );

uint32
  arad_pp_lif_my_bmac_msb_set_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_SAND_PP_MAC_ADDRESS                       *my_bmac_msb
  );

uint32
  arad_pp_lif_my_bmac_msb_get_verify(
    SOC_SAND_IN  int                                 unit
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "arad_pp_lif_my_bmac_msb_set_unsafe" API.
 *     Refer to "arad_pp_lif_my_bmac_msb_set_unsafe" API for
 *     details.
*********************************************************************/
uint32
  arad_pp_lif_my_bmac_msb_get_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_OUT SOC_SAND_PP_MAC_ADDRESS                       *my_bmac_msb
  );

/*********************************************************************
* NAME:
 *   arad_pp_lif_my_bmac_port_lsb_set_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Set the My-B-MAC LSB according to the source system
 *   port.
 * INPUT:
 *   SOC_SAND_IN  int                                 unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_SAND_PP_SYS_PORT_ID                       *src_sys_port_ndx -
 *     Physical system port.
 *   SOC_SAND_IN  SOC_SAND_PP_MAC_ADDRESS                       *my_mac_lsb -
 *     The LSB of the MAC address. MSB is global according to
 *     soc_ppd_lif_my_bmac_msb_set().
 *   SOC_SAND_IN  uint8                                 enable -
 *     When negated, the MAC address LSB is disabled for the
 *     system port
 * REMARKS:
 *   - Encapsulation: MAC-address is the packet B-SA,
 *   according to source system port ID - Termination -
 *   Enable = TRUE: When packet arrives with B-DA ==
 *   MAC-address, the backbone header is terminated and the
 *   packet is assigned to a VSI according to the I-SID. -
 *   Enable = FALSE: When the MAC address is attached to zero
 *   source system ports, the MAC address is not terminated,
 *   and forwarding is according B-MACT.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_lif_my_bmac_port_lsb_set_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_SAND_PP_SYS_PORT_ID                       *src_sys_port_ndx,
    SOC_SAND_IN  SOC_SAND_PP_MAC_ADDRESS                       *my_mac_lsb,
    SOC_SAND_IN  uint8                                 enable
  );

uint32
  arad_pp_lif_my_bmac_port_lsb_set_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_SAND_PP_SYS_PORT_ID                       *src_sys_port_ndx,
    SOC_SAND_IN  SOC_SAND_PP_MAC_ADDRESS                       *my_mac_lsb,
    SOC_SAND_IN  uint8                                 enable
  );

uint32
  arad_pp_lif_my_bmac_port_lsb_get_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_SAND_PP_SYS_PORT_ID                       *src_sys_port_ndx
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "arad_pp_lif_my_bmac_port_lsb_set_unsafe" API.
 *     Refer to "arad_pp_lif_my_bmac_port_lsb_set_unsafe" API for
 *     details.
*********************************************************************/
uint32
  arad_pp_lif_my_bmac_port_lsb_get_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_SAND_PP_SYS_PORT_ID                       *src_sys_port_ndx,
    SOC_SAND_INOUT SOC_SAND_PP_MAC_ADDRESS                       *my_mac_lsb,
    SOC_SAND_OUT uint8                                 *enable
  );

/*********************************************************************
* NAME:
 *   arad_pp_mymac_get_procs_ptr
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Get the pointer to the list of procedures of the
 *   arad_pp_api_mymac module.
 * INPUT:
 * REMARKS:
 *
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
CONST SOC_PROCEDURE_DESC_ELEMENT*
  arad_pp_mymac_get_procs_ptr(void);

/*********************************************************************
* NAME:
 *   arad_pp_mymac_get_errs_ptr
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Get the pointer to the list of errors of the
 *   arad_pp_api_mymac module.
 * INPUT:
 * REMARKS:
 *
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
CONST SOC_ERROR_DESC_ELEMENT*
  arad_pp_mymac_get_errs_ptr(void);

uint32
  SOC_PPC_MYMAC_VRRP_INFO_verify(
    SOC_SAND_IN  SOC_PPC_MYMAC_VRRP_INFO *info
  );

#ifdef BCM_88660_A0

soc_error_t
arad_pp_mymac_vrid_mymac_map_set_to_all_vsi(int unit, int cam_index, int enable);

soc_error_t
arad_pp_mymac_vrrp_cam_info_set(int unit, SOC_PPC_VRRP_CAM_INFO *info);

soc_error_t
arad_pp_mymac_vrrp_cam_info_get(int unit, SOC_PPC_VRRP_CAM_INFO *info);

soc_error_t
arad_pp_mymac_vrrp_cam_info_delete(int unit, uint8 cam_index);

#endif /*BCM_88660_A0*/

uint32
  SOC_PPC_MYMAC_TRILL_INFO_verify(
    SOC_SAND_IN  SOC_PPC_MYMAC_TRILL_INFO *info
  );

/* } */

#include <soc/dpp/SAND/Utils/sand_footer.h>

/* } __ARAD_PP_MYMAC_INCLUDED__*/
#endif


