
/* $Id: arad_pp_mgmt.h,v 1.6 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/

#ifndef __ARAD_PP_MGMT_INCLUDED__
/* { */
#define __ARAD_PP_MGMT_INCLUDED__

/*************
 * INCLUDES  *
 *************/
/* { */
#include <soc/dpp/SAND/Utils/sand_header.h>
#include <soc/dpp/SAND/Management/sand_general_macros.h>
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_general.h>

#include <soc/dpp/ARAD/ARAD_PP/arad_pp_framework.h>

/* } */
/*************
 * DEFINES   *
 *************/
/* { */

#define ARAD_PP_MGMT_MPLS_NOF_ETHER_TYPES 2

#define ARAD_PP_MGMT_IPV4_LPM_BANKS 6

/*
 * whether the ipv4 module support caching operation
 * i.e. mode where user can add/remove many routes
 * to software shdaow and commit all changes in one call
 * see soc_ppd_frwrd_ip_routes_cache_mode_enable_set/
 *     soc_ppd_frwrd_ip_routes_cache_commit
 * used in ARAD_PP_MGMT_IPV4_INFO.flags
 */
#define  ARAD_PP_MGMT_IPV4_OP_MODE_SUPPORT_CACHE (0x1)
/*
 * whether the ipv4 module support defragement operation
 * in Perta-B, IPv4 lpm includes dynamic memory management
 * This flag declare whether the module supports defragement
 * for these memories
 * see soc_ppd_frwrd_ipv4_mem_defrage
 * used in ARAD_PP_MGMT_IPV4_INFO.flags
 */
#define  ARAD_PP_MGMT_IPV4_OP_MODE_SUPPORT_DEFRAG (0x2)

/*
 * if TRUE then all VRFs share the same memory for routes management 
 * and in this case 
 *   - the maximum total routes for all VRFs is max_routes_in_vrf[0]. 
 *   - commit,cache function can be called only with SOC_PPC_FRWRD_IP_ALL_VRFS_ID
 * if FALSE then each VRF use seperate memory for routes management 
 *   - the maximum routes for VRF x is max_routes_in_vrf[x]
 *   - cache function can be called only with SOC_PPC_FRWRD_IP_ALL_VRFS_ID
 *     while commit can be called in particular VRF
 */
#define  ARAD_PP_MGMT_IPV4_SHARED_ROUTES_MEMORY (0x4)


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
  ARAD_PP_MGMT_GET_PROCS_PTR = ARAD_PP_PROC_DESC_BASE_MGMT_FIRST,
  ARAD_PP_MGMT_GET_ERRS_PTR,
  /*
   * } Auto generated. Do not edit previous section.
   */

  ARAD_PP_MGMT_DEVICE_INIT,
  ARAD_PP_MGMT_DEVICE_CLOSE,
  ARAD_PP_MGMT_OPERATION_MODE_SET,
  ARAD_PP_MGMT_OPERATION_MODE_GET,
  ARAD_PP_MGMT_OPERATION_MODE_SET_UNSAFE,
  ARAD_PP_MGMT_OPERATION_MODE_VERIFY,
  ARAD_PP_MGMT_OPERATION_MODE_GET_UNSAFE,
  ARAD_PP_MGMT_ELK_MODE_SET,
  ARAD_PP_MGMT_ELK_MODE_SET_PRINT,
  ARAD_PP_MGMT_ELK_MODE_SET_UNSAFE,
  ARAD_PP_MGMT_ELK_MODE_SET_VERIFY,
  ARAD_PP_MGMT_ELK_MODE_GET,
  ARAD_PP_MGMT_ELK_MODE_GET_PRINT,
  ARAD_PP_MGMT_ELK_MODE_GET_VERIFY,
  ARAD_PP_MGMT_ELK_MODE_GET_UNSAFE,
  ARAD_PP_MGMT_USE_ELK_SET,
  ARAD_PP_MGMT_USE_ELK_SET_PRINT,
  ARAD_PP_MGMT_USE_ELK_SET_UNSAFE,
  ARAD_PP_MGMT_USE_ELK_SET_VERIFY,
  ARAD_PP_MGMT_USE_ELK_GET,
  ARAD_PP_MGMT_USE_ELK_GET_PRINT,
  ARAD_PP_MGMT_USE_ELK_GET_VERIFY,
  ARAD_PP_MGMT_USE_ELK_GET_UNSAFE,
  ARAD_PP_MGMT_PROC_ERR_MECH_INIT,

  /*
   * Last element. Do no touch.
   */
  ARAD_PP_MGMT_PROCEDURE_DESC_LAST
} ARAD_PP_MGMT_PROCEDURE_DESC;

typedef enum
{
  /*
   * Auto generated. Do not edit following section {
   */
  ARAD_PP_MGMT_ELK_MODE_OUT_OF_RANGE_ERR = ARAD_PP_ERR_DESC_BASE_MGMT_FIRST,
  ARAD_PP_MGMT_INGRESS_PKT_RATE_OUT_OF_RANGE_ERR,
  ARAD_PP_MGMT_LKP_TYPE_OUT_OF_RANGE_ERR,
  ARAD_PP_MGMT_USE_ELK_OUT_OF_RANGE_ERR,
  /*
   * } Auto generated. Do not edit previous section.
   */

  /*
   * Last element. Do no touch.
   */
  ARAD_PP_MGMT_ERR_LAST
} ARAD_PP_MGMT_ERR;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR

  /* VSI used for mac-in-mac identification,
   *  i.e. packet associated with this VSI will
   *  has Mac-in-MAC processing
   */
  uint32 mim_vsi;
  
} ARAD_PP_MGMT_P2P_INFO;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR

  /*
   *  If TRUE, then private-VLAN is enabled
   *  split_horizon_filter_enable and pvlan_enable cannot both set to TRUE
   */
  uint8 pvlan_enable;
 /*
  * number of supported VRFs
  * Range 1-256.
  * supported VRF IDs: 0-(nof_vrfs-1)
  */
  uint32 nof_vrfs;
 /*
  * maximum number of routes to support in LPM for IPv4 UC
  * for each VRF.
  * note: this is size to allocate for SW management.
  * maximum number of routes limited also by HW tables. 
  * see ARAD_PP_MGMT_IPV4_SHARED_ROUTES_MEMORY 
  */
  uint32 max_routes_in_vrf[SOC_DPP_DEFS_MAX(NOF_VRFS)];

  /* 
   * how many bits to consider in each phase of LPM algorithm. 
   */
  uint8 bits_in_phase[ARAD_PP_MGMT_IPV4_LPM_BANKS];

  /* 
   * whether to consider bits_in_phase, or to calculate by driver 
   */
  uint8 bits_in_phase_valid;

 /*
  * flags for ipv4 management
  * see ARAD_PP_MGMT_IPV4_OP_MODE
  */
  uint32 flags;

  
} ARAD_PP_MGMT_IPV4_INFO;



typedef struct
{
  /*
   *  MPLS ether types
   */
  SOC_SAND_PP_ETHER_TYPE mpls_ether_types[ARAD_PP_MGMT_MPLS_NOF_ETHER_TYPES];

  /*
   *  MPLS termination with known stack (e.g. adding 2nd label) enables termination
   *  of label only on the required location in the MPLS stack and not over all different locations in the MPLS stack.
   *  In this mode, no need to duplicate MPLS entries for different mpls label index. 
   */
  uint8 mpls_termination_label_index_enable;
  /*
   * Enable indicates device supports fast reroute (FRR) labels. 
   * Disable this mode provides the ability to add up to 64K label entries (and not 32K)   
   */
  uint8 fast_reroute_labels_enable;
  /* 
   * Indication if lookup for Ingress MPLS (termination and forwarding) should include In-RIF
   */
  uint8 lookup_include_inrif;
  /* 
   * Indication if lookup for Ingress MPLS (termination and forwarding) should include VRF
   */
  uint8 lookup_include_vrf;
   
} ARAD_PP_MGMT_MPLS_INFO;

typedef struct
{
  /*
   *  If TRUE, then SA-authentication is enabled
   */
  uint8 authentication_enable;
  /*
   *  If TRUE, then the System-VSI is enabled
   *  PB: ignored.
   */
  uint8 system_vsi_enable;
  /*
   *  If TRUE, then the Hairpin is enabled
   *  PB: ignored.
   */
  uint8 hairpin_enable;
  /*
   *  If TRUE, then the split_horizon_filter for ACs is enabled
   *  when true then split-horizon is performed for in-AC vs out-AC
   *  if false then split-horizon is performed according to EEP/port.
   *  split_horizon_filter_enable and pvlan_enable cannot both set to TRUE
   */
  uint8 split_horizon_filter_enable;
  /*
   *  IPV4 info
   */
  ARAD_PP_MGMT_IPV4_INFO ipv4_info;
  /*
   *  P2P init info
   */
  ARAD_PP_MGMT_P2P_INFO p2p_info;
  /* 
   * MPLS info
   */
  ARAD_PP_MGMT_MPLS_INFO mpls_info;
  /*
   *  If TRUE, then MAC in MAC is enabled
   */
  uint8 mim_enable;

  /*
   *  If TRUE, then OAM is enabled
   */
  uint8 oam_enable;

  /*
   *  If TRUE, then BFD is enabled
   */
  uint8 bfd_enable;

  /*
   *  If TRUE, then TRILL is enabled
   */
  uint8 trill_enable;

  /*
   *  If TRUE, then bcm_mim_init has been called (when mim_enable=1 it means that bcm_mim_init can be called)
   */
  uint8 mim_initialized;

}ARAD_PP_MGMT_OPERATION_MODE;

typedef enum
{
  /*
   *  External lookups are disabled.
   */
  ARAD_PP_MGMT_ELK_MODE_NONE = 0,
  /*
   *  External lookup is set to Arad-B rev. A1 mode.
   */
  ARAD_PP_MGMT_ELK_MODE_NORMAL = 1,
  /*
   *  External lookup uses short records exclusively
   *  (available in Arad-B rev. B0 only).
   */
  ARAD_PP_MGMT_ELK_MODE_B0_SHORT = 2,
  /*
   *  External lookup uses long records exclusively (available
   *  in Arad-B rev. B0 only).
   */
  ARAD_PP_MGMT_ELK_MODE_B0_LONG = 3,
  /*
   *  External lookup uses both short and long records
   *  (available in Arad-B rev. B0 only).
   */
  ARAD_PP_MGMT_ELK_MODE_B0_BOTH = 4,
  /*
   *  Number of types in ARAD_PP_MGMT_ELK_MODE
   */
  ARAD_PP_NOF_MGMT_ELK_MODES = 5
}ARAD_PP_MGMT_ELK_MODE;

typedef enum
{
  /*
   *  Transparent P2P service.
   */
  ARAD_PP_MGMT_LKP_TYPE_P2P = 0,
  /*
   *  Ethernet bridging.
   */
  ARAD_PP_MGMT_LKP_TYPE_ETH = 1,
  /*
   *  TRILL unicast forwarding.
   */
  ARAD_PP_MGMT_LKP_TYPE_TRILL_UC = 2,
  /*
   *  TRILL multicast forwarding.
   */
  ARAD_PP_MGMT_LKP_TYPE_TRILL_MC = 3,
  /*
   *  IPv4 unicast forwarding.
   */
  ARAD_PP_MGMT_LKP_TYPE_IPV3_UC = 4,
  /*
   *  IPv4 multicast forwarding.
   */
  ARAD_PP_MGMT_LKP_TYPE_IPV4_MC = 5,
  /*
   *  IPv6 unicast forwarding.
   */
  ARAD_PP_MGMT_LKP_TYPE_IPV6_UC = 6,
  /*
   *  IPv6 multicast forwarding.
   */
  ARAD_PP_MGMT_LKP_TYPE_IPV6_MC = 7,
  /*
   *  MPLS forwarding.
   */
  ARAD_PP_MGMT_LKP_TYPE_LSR = 8,
  /*
   *  Number of types in ARAD_PP_MGMT_LKP_TYPE
   */
  ARAD_PP_NOF_MGMT_LKP_TYPES = 9
}ARAD_PP_MGMT_LKP_TYPE;


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
*     arad_pp_mgmt_operation_mode_set_unsafe
* TYPE:
*   PROC
* FUNCTION:
*     Set arad_pp device operation mode. This defines
*     configurations such as support for certain header types
*     etc.
* INPUT:
*  SOC_SAND_IN  int                 unit -
*     Identifier of the device to access.
*  SOC_SAND_IN  ARAD_PP_MGMT_OPERATION_MODE *op_mode -
*     arad_pp device operation mode.
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_mgmt_operation_mode_set_unsafe(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  ARAD_PP_MGMT_OPERATION_MODE *op_mode
  );

/*********************************************************************
* NAME:
*     arad_pp_mgmt_operation_mode_verify
* TYPE:
*   PROC
* FUNCTION:
*     Set arad_pp device operation mode. This defines
*     configurations such as support for certain header types
*     etc.
* INPUT:
*  SOC_SAND_IN  int                 unit -
*     Identifier of the device to access.
*  SOC_SAND_IN  ARAD_PP_MGMT_OPERATION_MODE *op_mode -
*     arad_pp device operation mode.
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_mgmt_operation_mode_verify(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  ARAD_PP_MGMT_OPERATION_MODE *op_mode
  );

/*********************************************************************
* NAME:
*     arad_pp_mgmt_operation_mode_get_unsafe
* TYPE:
*   PROC
* FUNCTION:
*     Set arad_pp device operation mode. This defines
*     configurations such as support for certain header types
*     etc.
* INPUT:
*  SOC_SAND_IN  int                 unit -
*     Identifier of the device to access.
*  SOC_SAND_OUT ARAD_PP_MGMT_OPERATION_MODE *op_mode -
*     arad_pp device operation mode.
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_mgmt_operation_mode_get_unsafe(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_OUT ARAD_PP_MGMT_OPERATION_MODE *op_mode
  );

/*********************************************************************
* NAME:
*     arad_pp_mgmt_device_close_unsafe
* TYPE:
*   PROC
* FUNCTION:
*     Close the Device, and clean HW and SW.
* INPUT:
*  SOC_SAND_IN  int                 unit -
*     Identifier of the device to close.
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_mgmt_device_close_unsafe(
    SOC_SAND_IN  int                 unit
  );

/*********************************************************************
* NAME:
 *   arad_pp_mgmt_elk_mode_set_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Set the ELK interface mode.
 * INPUT:
 *   SOC_SAND_IN  int           unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  ARAD_PP_MGMT_ELK_MODE elk_mode -
 *     The requested operation mode for the ELK interface.
 *   SOC_SAND_OUT uint32           *ingress_pkt_rate -
 *     The effective processing rate of the ingress device in
 *     packets per second.
 * REMARKS:
 *   1. The ELK's physical interface has to be configured
 *   before calling this function. See arad_nif_elk_set().2.
 *   Arad-B rev. B0 modes are not available on earlier
 *   revisions.3. The ingress device's rate is limited by the
 *   processing rate of the ELK interface, affecting all
 *   packets entering the device.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_mgmt_elk_mode_set_unsafe(
    SOC_SAND_IN  int           unit,
    SOC_SAND_IN  ARAD_PP_MGMT_ELK_MODE elk_mode,
    SOC_SAND_OUT uint32           *ingress_pkt_rate
  );

uint32
  arad_pp_mgmt_elk_mode_set_verify(
    SOC_SAND_IN  int           unit,
    SOC_SAND_IN  ARAD_PP_MGMT_ELK_MODE elk_mode
  );

uint32
  arad_pp_mgmt_elk_mode_get_verify(
    SOC_SAND_IN  int           unit
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "arad_pp_mgmt_elk_mode_set_unsafe" API.
 *     Refer to "arad_pp_mgmt_elk_mode_set_unsafe" API for
 *     details.
*********************************************************************/
uint32
  arad_pp_mgmt_elk_mode_get_unsafe(
    SOC_SAND_IN  int           unit,
    SOC_SAND_OUT ARAD_PP_MGMT_ELK_MODE *elk_mode,
    SOC_SAND_OUT uint32           *ingress_pkt_rate
  );

/*********************************************************************
* NAME:
 *   arad_pp_mgmt_use_elk_set_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Determine whether the specified lookup is externalized
 *   or not.
 * INPUT:
 *   SOC_SAND_IN  int           unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  ARAD_PP_MGMT_LKP_TYPE lkp_type -
 *     The lookup type in question.
 *   SOC_SAND_IN  uint8           use_elk -
 *     Whether to use the ELK for that lookup or not.
 * REMARKS:
 *   IPv4 multicast and IPv6 lookups can only be performed
 *   externally in Arad-B rev. B0 modes.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_mgmt_use_elk_set_unsafe(
    SOC_SAND_IN  int           unit,
    SOC_SAND_IN  ARAD_PP_MGMT_LKP_TYPE lkp_type,
    SOC_SAND_IN  uint8           use_elk
  );

uint32
  arad_pp_mgmt_use_elk_set_verify(
    SOC_SAND_IN  int           unit,
    SOC_SAND_IN  ARAD_PP_MGMT_LKP_TYPE lkp_type,
    SOC_SAND_IN  uint8           use_elk
  );

uint32
  arad_pp_mgmt_use_elk_get_verify(
    SOC_SAND_IN  int           unit
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "arad_pp_mgmt_use_elk_set_unsafe" API.
 *     Refer to "arad_pp_mgmt_use_elk_set_unsafe" API for
 *     details.
*********************************************************************/
uint32
  arad_pp_mgmt_use_elk_get_unsafe(
    SOC_SAND_IN  int           unit,
    SOC_SAND_OUT ARAD_PP_MGMT_LKP_TYPE *lkp_type,
    SOC_SAND_OUT uint8           *use_elk
  );

/*********************************************************************
* NAME:
 *   arad_pp_mgmt_get_procs_ptr
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Get the pointer to the list of procedures of the
 *   arad_pp_api_mgmt module.
 * INPUT:
 * REMARKS:
 *
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
CONST SOC_PROCEDURE_DESC_ELEMENT*
  arad_pp_mgmt_get_procs_ptr(void);

/*********************************************************************
* NAME:
 *   arad_pp_mgmt_get_errs_ptr
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Get the pointer to the list of errors of the
 *   arad_pp_api_mgmt module.
 * INPUT:
 * REMARKS:
 *
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
CONST SOC_ERROR_DESC_ELEMENT*
  arad_pp_mgmt_get_errs_ptr(void);

/* } */



void
  ARAD_PP_MGMT_OPERATION_MODE_clear(
    SOC_SAND_OUT ARAD_PP_MGMT_OPERATION_MODE *info
  );

void
  ARAD_PP_MGMT_P2P_INFO_clear(
    SOC_SAND_OUT ARAD_PP_MGMT_P2P_INFO *info
  );
void
  ARAD_PP_MGMT_IPV4_INFO_clear(
    SOC_SAND_OUT ARAD_PP_MGMT_IPV4_INFO *info
  );


/* } */


#if SOC_PPC_DEBUG_IS_LVL1
void
  ARAD_PP_MGMT_OPERATION_MODE_print(
    SOC_SAND_IN  ARAD_PP_MGMT_OPERATION_MODE *info
  );

void
  ARAD_PP_MGMT_P2P_INFO_print(
    SOC_SAND_IN  ARAD_PP_MGMT_P2P_INFO *info
  );

void
  ARAD_PP_MGMT_IPV4_INFO_print(
    SOC_SAND_IN  ARAD_PP_MGMT_IPV4_INFO *info
  );

const char*
  ARAD_PP_MGMT_ELK_MODE_to_string(
    SOC_SAND_IN  ARAD_PP_MGMT_ELK_MODE enum_val
  );

const char*
  ARAD_PP_MGMT_LKP_TYPE_to_string(
    SOC_SAND_IN  ARAD_PP_MGMT_LKP_TYPE enum_val
  );

#endif /* SOC_PPC_DEBUG_IS_LVL1 */

#include <soc/dpp/SAND/Utils/sand_footer.h>

/* } __ARAD_PP_MGMT_INCLUDED__*/
#endif


