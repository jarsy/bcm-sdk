/* $Id$
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/
/******************************************************************
*
* FILENAME:       DuneDriver/ppc/include/soc_ppc_api_vsi.h
*
* MODULE PREFIX:  soc_ppc_vsi
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

#ifndef __SOC_PPC_API_VSI_INCLUDED__
/* { */
#define __SOC_PPC_API_VSI_INCLUDED__

/*************
 * INCLUDES  *
 *************/
/* { */

#include <soc/dpp/SAND/Utils/sand_header.h>

#include <soc/dpp/SAND/Management/sand_general_macros.h>
#include <soc/dpp/SAND/Management/sand_error_code.h>

#include <soc/dpp/PPC/ppc_api_general.h>

/* } */
/*************
 * DEFINES   *
 *************/
/* { */

#define SOC_PPC_VSI_FID_IS_VSID                                (0xffffffff)

#define SOC_PPC_VSI_LOW_CNFG_VALUE                          (1)

#define SOC_PPC_VSI_L2CP_KEY_DA_MAC_ADDRESS_LSB_LSB           (0)
#define SOC_PPC_VSI_L2CP_KEY_DA_MAC_ADDRESS_LSB_MSB           (5)
#define SOC_PPC_VSI_L2CP_KEY_DA_MAC_ADDRESS_LSB_SHIFT         (SOC_PPC_VSI_L2CP_KEY_DA_MAC_ADDRESS_LSB_LSB)
#define SOC_PPC_VSI_L2CP_KEY_DA_MAC_ADDRESS_LSB_MASK          (SOC_SAND_BITS_MASK(SOC_PPC_VSI_L2CP_KEY_DA_MAC_ADDRESS_LSB_MSB, SOC_PPC_VSI_L2CP_KEY_DA_MAC_ADDRESS_LSB_LSB))

#define SOC_PPC_VSI_L2CP_KEY_L2CP_PROFILE_LSB                 (6)
#define SOC_PPC_VSI_L2CP_KEY_L2CP_PROFILE_MSB                 (6)
#define SOC_PPC_VSI_L2CP_KEY_L2CP_PROFILE_SHIFT               (SOC_PPC_VSI_L2CP_KEY_L2CP_PROFILE_LSB)
#define SOC_PPC_VSI_L2CP_KEY_L2CP_PROFILE_MASK                (SOC_SAND_BITS_MASK(SOC_PPC_VSI_L2CP_KEY_L2CP_PROFILE_MSB, SOC_PPC_VSI_L2CP_KEY_L2CP_PROFILE_LSB))


#define SOC_PPC_VSI_L2CP_KEY_ENTRY_OFFSET(l2cp_profile, da_mac_address_lsb)  \
          SOC_SAND_SET_FLD_IN_PLACE(da_mac_address_lsb, SOC_PPC_VSI_L2CP_KEY_DA_MAC_ADDRESS_LSB_SHIFT, SOC_PPC_VSI_L2CP_KEY_DA_MAC_ADDRESS_LSB_MASK) | \
          SOC_SAND_SET_FLD_IN_PLACE(l2cp_profile, SOC_PPC_VSI_L2CP_KEY_L2CP_PROFILE_SHIFT, SOC_PPC_VSI_L2CP_KEY_L2CP_PROFILE_MASK)



typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  l2cp profile ingress (set according to LIF see
   *  SOC_PPC_l2_lif_ac_add()). Range: 0 - 1.
   */
  uint32 l2cp_profile_ingress;
  /*
   *  l2cp profile egress (set according to LIF see
   *  SOC_PPC_l2_lif_ac_add()). Range: 0 - 1.
   */
  uint32 l2cp_profile_egress;
  /*
   *  The 6 lsb of the Destination MAC address (DA[5:0]). The
   *  msb bits DA[47:6] are 01-80-c2-00-00-XX where XX =
   *  8'b00xx_xxxx)Range: 0 - 63.
   */
  uint32 da_mac_address_lsb;

} SOC_PPC_VSI_L2CP_KEY;

typedef enum
{
  /*
   *  Handle the L2CP packet as normal packet (including vlan
   *  editing)
   */
  SOC_PPC_VSI_L2CP_HANDLE_TYPE_NORMAL = 0,
  /*
   *  The packet is forwarded Transparently (with no vlan
   *  editing) i.e. the content of a Service Frame is
   *  delivered unaltered.
   */
  SOC_PPC_VSI_L2CP_HANDLE_TYPE_TUNNEL = 1,
  /*
   *  Packet is trapped and assigned SOC_PPC_TRAP_CODE_L2CP_PEER
   *  trap code
   */
  SOC_PPC_VSI_L2CP_HANDLE_TYPE_PEER = 2,
  /*
   *  Packet is dropped, assigned SOC_PPC_TRAP_CODE_L2CP_DROP trap
   *  code
   */
  SOC_PPC_VSI_L2CP_HANDLE_TYPE_DROP = 3,
  /*
   *  Number of types in SOC_PPC_L2_LIF_L2CP_HANDLE_TYPE
   */
  SOC_PPC_VSI_L2CP_HANDLE_TYPES = 4
}SOC_PPC_VSI_L2CP_HANDLE_TYPE;


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

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  Default VSI forwarding decision to apply when there is
   *  no hit in the MAC table. Arad only.
   */
  SOC_PPC_FRWRD_DECISION_INFO default_forwarding;
  /*
   *  Default VSI forwarding decision to apply when there is
   *  no hit in the MAC table. The profile ID is part of the
   *  'dflt_frwrd_key', used by
   *  soc_ppd_vsi_default_frwrd_info_set() Soc_petra only.
   */
  uint32 default_forward_profile;
  /*
   *  STP topology ID. Range: 0-63
   */
  uint32 stp_topology_id;
  /*
   *  Enable/Disable L2 termination due to My MAC. The MyMAC
   *  Address is configured in the MyMAC module.
   */
  uint8 enable_my_mac;
  /*
   *  VSID mapping to MAC table FID (Filtering
   *  ID). SOC_PPC_VSI_FID_IS_VSID: indicates that the FID is the
   *  VSID. Other profiles enable shared learning, by calling
   *  soc_ppd_frwrd_mact_fid_profile_to_fid_map_set()In T20E:
   *  Must be set to 'SOC_PPC_VSI_FID_IS_VSID'
   *  IF fid_profile_id != SOC_PPC_VSI_FID_IS_VSID, then this FID-
   *  profile must be already set
   */
  uint32 fid_profile_id;
  /*
   *  VSI profile that used to set the MACT learning
   *  attributes, including
   *  1. Mac-limit: Limit the number of
   *  MAC addresses learnt on the VSI.
   *  see soc_ppd_frwrd_mact_mac_limit_profile_info_set() to
   *  set the limitation attributes of the profile.
   *  2.event-handle-proofile (Soc_petra-B only): used to give
   *  different handling for MACT events according to VSI.
   *  3. fid-aging profile: used to give different aging profile for MACT
   *  events according to VSI.
   *  
   *  When fid_profile_id is not 'SOC_PPC_VSI_FID_IS_VSID', then the
   *  above attributes are set according to the shared FID.
   */
  uint32 mac_learn_profile_id;

  /* 
   * VSI general profile ingress.
   * Ingress: PMF: 2 ms bits
   *          Trap MEF-L2CP-Profile: 2 ls bits (general for ingress&egress)
   * Range: 0-15. 
   * Arad only. 
   */ 
  uint32 profile_ingress;
  /* 
   * VSI general profile egress.
   * Egress:  MTU: 2 ls bits
   *          Trap MEF-L2CP-Profile: 2 ls bits (general for ingress&egress)
   * Range: 0-3. 
   * Arad only. 
   */ 
  uint32 profile_egress;
  /*
   *  Clear the IHP_VSI_LOW_CFG_2 table in case the vlan is destroyed.
   */
  uint8 clear_on_destroy;
} SOC_PPC_VSI_INFO;


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
  SOC_PPC_VSI_L2CP_KEY_clear(
    SOC_SAND_OUT SOC_PPC_VSI_L2CP_KEY *info
  );

void
  SOC_PPC_VSI_INFO_clear(
    SOC_SAND_OUT SOC_PPC_VSI_INFO *info
  );

#if SOC_PPC_DEBUG_IS_LVL1

void
  SOC_PPC_VSI_INFO_print(
    SOC_SAND_IN  SOC_PPC_VSI_INFO *info
  );

#endif /* SOC_PPC_DEBUG_IS_LVL1 */

/* } */

#include <soc/dpp/SAND/Utils/sand_footer.h>

/* } __SOC_PPC_API_VSI_INCLUDED__*/
#endif

