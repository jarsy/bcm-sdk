/* $Id: ppd_api_mymac.h,v 1.10 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/
/******************************************************************
*
* FILENAME:       DuneDriver/ppd/include/soc_ppd_api_mymac.h
*
* MODULE PREFIX:  soc_ppd_mymac
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

#ifndef __SOC_PPD_API_MYMAC_INCLUDED__
/* { */
#define __SOC_PPD_API_MYMAC_INCLUDED__

/*************
 * INCLUDES  *
 *************/
/* { */

#include <soc/dpp/SAND/Utils/sand_header.h>

#include <soc/dpp/SAND/Management/sand_general_macros.h>
#include <soc/dpp/SAND/Management/sand_error_code.h>

#include <soc/dpp/PPD/ppd_api_framework.h>
#include <soc/dpp/PPC/ppc_api_mymac.h>

#include <soc/dpp/PPD/ppd_api_general.h>

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
  SOC_PPD_MYMAC_MSB_SET = SOC_PPD_PROC_DESC_BASE_MYMAC_FIRST,
  SOC_PPD_MYMAC_MSB_SET_PRINT,
  SOC_PPD_MYMAC_MSB_GET,
  SOC_PPD_MYMAC_MSB_GET_PRINT,
  SOC_PPD_MYMAC_VSI_LSB_SET,
  SOC_PPD_MYMAC_VSI_LSB_SET_PRINT,
  SOC_PPD_MYMAC_VSI_LSB_GET,
  SOC_PPD_MYMAC_VSI_LSB_GET_PRINT,
  SOC_PPD_MYMAC_VRRP_INFO_SET,
  SOC_PPD_MYMAC_VRRP_INFO_SET_PRINT,
  SOC_PPD_MYMAC_VRRP_INFO_GET,
  SOC_PPD_MYMAC_VRRP_INFO_GET_PRINT,
  SOC_PPD_MYMAC_VRRP_MAC_SET,
  SOC_PPD_MYMAC_VRRP_MAC_SET_PRINT,
  SOC_PPD_MYMAC_VRRP_MAC_GET,
  SOC_PPD_MYMAC_VRRP_MAC_GET_PRINT,
  SOC_PPD_MYMAC_TRILL_INFO_SET,
  SOC_PPD_MYMAC_TRILL_INFO_SET_PRINT,
  SOC_PPD_MYMAC_TRILL_INFO_GET,
  SOC_PPD_MYMAC_TRILL_INFO_GET_PRINT,
  SOC_PPD_LIF_MY_BMAC_MSB_SET,
  SOC_PPD_LIF_MY_BMAC_MSB_SET_PRINT,
  SOC_PPD_LIF_MY_BMAC_MSB_GET,
  SOC_PPD_LIF_MY_BMAC_MSB_GET_PRINT,
  SOC_PPD_LIF_MY_BMAC_PORT_LSB_SET,
  SOC_PPD_LIF_MY_BMAC_PORT_LSB_SET_PRINT,
  SOC_PPD_LIF_MY_BMAC_PORT_LSB_GET,
  SOC_PPD_LIF_MY_BMAC_PORT_LSB_GET_PRINT,
  SOC_PPD_MYMAC_GET_PROCS_PTR,
  /*
   * } Auto generated. Do not edit previous section.
   */
  /*
   * Last element. Do no touch.
   */
  SOC_PPD_MYMAC_PROCEDURE_DESC_LAST
} SOC_PPD_MYMAC_PROCEDURE_DESC;


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
 *   soc_ppd_mymac_msb_set
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Set the MSB of the MAC address of the device. Used for
 *   ingress termination and egress encapsulation.
 * INPUT:
 *   SOC_SAND_IN  int                               unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_SAND_PP_MAC_ADDRESS                     *my_mac_msb -
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
  soc_ppd_mymac_msb_set(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  SOC_SAND_PP_MAC_ADDRESS                     *my_mac_msb
  );

/*********************************************************************
*     Gets the configuration set by the "soc_ppd_mymac_msb_set"
 *     API.
 *     Refer to "soc_ppd_mymac_msb_set" API for details.
*********************************************************************/
uint32
  soc_ppd_mymac_msb_get(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_OUT SOC_SAND_PP_MAC_ADDRESS                     *my_mac_msb
  );

/*********************************************************************
* NAME:
 *   soc_ppd_mymac_vsi_lsb_set
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Set MAC address LSB according to VSI.
 * INPUT:
 *   SOC_SAND_IN  int                               unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_VSI_ID                              vsi_ndx -
 *     VSI ID. Range: Soc_petra-B: 0-4K. T20E: 0- 64K.
 *   SOC_SAND_IN  SOC_SAND_PP_MAC_ADDRESS                     *my_mac_lsb -
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
  soc_ppd_mymac_vsi_lsb_set(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  SOC_PPC_VSI_ID                              vsi_ndx,
    SOC_SAND_IN  SOC_SAND_PP_MAC_ADDRESS                     *my_mac_lsb
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "soc_ppd_mymac_vsi_lsb_set" API.
 *     Refer to "soc_ppd_mymac_vsi_lsb_set" API for details.
*********************************************************************/
uint32
  soc_ppd_mymac_vsi_lsb_get(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  SOC_PPC_VSI_ID                              vsi_ndx,
    SOC_SAND_OUT SOC_SAND_PP_MAC_ADDRESS                     *my_mac_lsb
  );

/*********************************************************************
* NAME:
 *   soc_ppd_mymac_vrrp_info_set
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Set My-MAC according to Virtual Router Redundancy
 *   Protocol.
 * INPUT:
 *   SOC_SAND_IN  int                               unit -
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
  soc_ppd_mymac_vrrp_info_set(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  SOC_PPC_MYMAC_VRRP_INFO                     *vrrp_info
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "soc_ppd_mymac_vrrp_info_set" API.
 *     Refer to "soc_ppd_mymac_vrrp_info_set" API for details.
*********************************************************************/
uint32
  soc_ppd_mymac_vrrp_info_get(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_OUT SOC_PPC_MYMAC_VRRP_INFO                     *vrrp_info
  );

/*********************************************************************
* NAME:
 *   soc_ppd_mymac_vrrp_mac_set
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Enable / Disable MyMac/MyVRID according to VRRP-ID and
 *   Mac Address LSB.
 * INPUT:
 *   SOC_SAND_IN  int                               unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  uint32                                vrrp_id_ndx -
 *     VRRP-ID. The port or VSI, according to which to
 *     configure the MAC. Depending on the VRRP_MODE.
 *   SOC_SAND_IN  SOC_SAND_PP_MAC_ADDRESS                     *vrrp_mac_lsb_key -
 *     The LSB of the MAC address. MSB is global.
 *   SOC_SAND_IN  uint8                               enable -
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
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_mymac_vrrp_mac_set(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  uint32                                vrrp_id_ndx,
    SOC_SAND_IN  SOC_SAND_PP_MAC_ADDRESS                     *vrrp_mac_lsb_key,
    SOC_SAND_IN  uint8                               enable
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "soc_ppd_mymac_vrrp_mac_set" API.
 *     Refer to "soc_ppd_mymac_vrrp_mac_set" API for details.
*********************************************************************/
uint32
  soc_ppd_mymac_vrrp_mac_get(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  uint32                                vrrp_id_ndx,
    SOC_SAND_IN  SOC_SAND_PP_MAC_ADDRESS                     *vrrp_mac_lsb_key,
    SOC_SAND_OUT uint8                               *enable
  );

/*********************************************************************
* NAME:
 *   soc_ppd_mymac_trill_info_set
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Set TRILL My-Nick-Name and reserved Nick-Name.
 * INPUT:
 *   SOC_SAND_IN  int                               unit -
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
  soc_ppd_mymac_trill_info_set(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  SOC_PPC_MYMAC_TRILL_INFO                    *trill_info
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "soc_ppd_mymac_trill_info_set" API.
 *     Refer to "soc_ppd_mymac_trill_info_set" API for details.
*********************************************************************/
uint32
  soc_ppd_mymac_trill_info_get(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_OUT SOC_PPC_MYMAC_TRILL_INFO                    *trill_info
  );

/*********************************************************************
* NAME:
 *   soc_ppd_lif_my_bmac_msb_set
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Set the MSB of the My-B-MAC. My-B-MAC forwards the
 *   packets to the I Component, and is added as the SA when
 *   sending toward the backbone network.
 * INPUT:
 *   SOC_SAND_IN  int                               unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_SAND_PP_MAC_ADDRESS                     *my_bmac_msb -
 *     The MSB of the MAC address of the device. The LSBs are
 *     according to VSI.
 * REMARKS:
 *   - Relevant Only for PBP ports.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_lif_my_bmac_msb_set(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  SOC_SAND_PP_MAC_ADDRESS                     *my_bmac_msb
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "soc_ppd_lif_my_bmac_msb_set" API.
 *     Refer to "soc_ppd_lif_my_bmac_msb_set" API for details.
*********************************************************************/
uint32
  soc_ppd_lif_my_bmac_msb_get(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_OUT SOC_SAND_PP_MAC_ADDRESS                     *my_bmac_msb
  );

/*********************************************************************
* NAME:
 *   soc_ppd_lif_my_bmac_port_lsb_set
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Set the My-B-MAC LSB according to the source system
 *   port.
 * INPUT:
 *   SOC_SAND_IN  int                               unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_SAND_PP_SYS_PORT_ID                     *src_sys_port_ndx -
 *     Physical system port.
 *   SOC_SAND_IN  SOC_SAND_PP_MAC_ADDRESS                     *my_mac_lsb -
 *     The LSB of the MAC address. MSB is global according to
 *     soc_ppd_lif_my_bmac_msb_set().
 *   SOC_SAND_IN  uint8                               enable -
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
  soc_ppd_lif_my_bmac_port_lsb_set(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  SOC_SAND_PP_SYS_PORT_ID                     *src_sys_port_ndx,
    SOC_SAND_IN  SOC_SAND_PP_MAC_ADDRESS                     *my_mac_lsb,
    SOC_SAND_IN  uint8                               enable
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "soc_ppd_lif_my_bmac_port_lsb_set" API.
 *     Refer to "soc_ppd_lif_my_bmac_port_lsb_set" API for details.
*********************************************************************/
uint32
  soc_ppd_lif_my_bmac_port_lsb_get(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  SOC_SAND_PP_SYS_PORT_ID                     *src_sys_port_ndx,
    SOC_SAND_INOUT SOC_SAND_PP_MAC_ADDRESS                     *my_mac_lsb,
    SOC_SAND_OUT uint8                               *enable
  );

/* } */

#include <soc/dpp/SAND/Utils/sand_footer.h>

/* } __SOC_PPD_API_MYMAC_INCLUDED__*/
#endif

