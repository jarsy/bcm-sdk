/* $Id: ppc_api_eg_ac.h,v 1.11 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/
/******************************************************************
*
* FILENAME:       DuneDriver/ppc/include/soc_ppc_api_eg_ac.h
*
* MODULE PREFIX:  soc_ppc_eg
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

#ifndef __SOC_PPC_API_EG_AC_INCLUDED__
/* { */
#define __SOC_PPC_API_EG_AC_INCLUDED__

/*************
 * INCLUDES  *
 *************/
/* { */

#include <soc/dpp/SAND/Utils/sand_header.h>

#include <soc/dpp/SAND/Management/sand_general_macros.h>
#include <soc/dpp/SAND/Management/sand_error_code.h>

#include <soc/dpp/PPC/ppc_api_general.h>
#include <soc/dpp/PPC/ppc_api_eg_encap.h>

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

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  VLAN Domain ID. The VLAN domain is either a port, or a
   *  group of ports that share the same VLAN IDs space. In
   *  T20E has to be equal to local-port-id.
   */
  uint32 vlan_domain;
  /*
   *  The VSI the packet is associated with. For Tunnel
   *  encapsulated packets, this is the VSI from the Egress
   *  Encapsulation.
   */
  SOC_PPC_VSI_ID vsi;

} SOC_PPC_EG_AC_VBP_KEY;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  VLAN Domain ID. All ports use this vlan domain has to be
   *  of type CEP. The VLAN domain is either a port, or a group
   *  of ports that share the same VLAN IDs space. In T20E has
   *  to be equal to local-port-id.
   */
  uint32 vlan_domain;
  /*
   *  The resolved C-VLAN ID. This is the C-VLAN ID on the
   *  packet (on VLAN tag Header) or a C-VID set according to
   *  the PEP default (PEP-PVID).
   */
  SOC_SAND_PP_VLAN_ID cvid;
  /*
   *  Edit profile to enable different editing according to
   *  PEP. The edit profile is set according to
   *  soc_ppd_eg_edit_pep_info_set(). Used to give different
   *  treatment for packets in the same C-Component, depending
   *  to the PEP they pass from. In Soc_petra-B has to be 0. Range:
   *  0 - 7.
   */
  uint32 pep_edit_profile;
  /* 
   * The VSI the packet is associated with. For Tunnel 
   * encapsulated packets, this is the VSI from the Egress 
   * Encapsulation. 
   */ 
  SOC_PPC_VSI_ID vsi; 

} SOC_PPC_EG_AC_CEP_PORT_KEY;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  C-VLAN ID. Used for CVID translation in the
   *  C-Component. Range: 0 - 4095.
   */
  SOC_SAND_PP_VLAN_ID cvid;
  /*
   *  User priority to set in the packet header. Range: 0 - 7.
   */
  SOC_SAND_PP_PCP_UP up;
  /*
   *  Profile used to build the UP of the packet. See COS
   *  Mapping APIs
   *  below:soc_ppd_eg_vlan_edit_pcp_map_stag_set()soc_ppd_eg_vlan_edit_pcp_map_ctag_set()soc_ppd_eg_vlan_edit_pcp_map_untagged_set()These
   *  APIs map COS parameters (UP/PCP/DEI/TC/DP) (and
   *  pcp_profile) to PCP and DEI. These profiles let the user
   *  configure up to 16 such mappings. Range: 0 - 15.
   */
  uint8 pcp_profile;
  /*
   *  Profile used to set the Egress command.
   */
  uint32 edit_profile;

} SOC_PPC_EG_VLAN_EDIT_CEP_INFO;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  The level of the MEP. Range: 0 - 7.
   */
  SOC_PPC_MP_LEVEL mp_level;
  /*
   *  If True, then enable MP for this ACF.
   */
  uint8 is_valid;

} SOC_PPC_EG_AC_MP_INFO;


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
  SOC_PPC_EG_AC_VBP_KEY_clear(
    SOC_SAND_OUT SOC_PPC_EG_AC_VBP_KEY *info
  );

void
  SOC_PPC_EG_AC_CEP_PORT_KEY_clear(
    SOC_SAND_OUT SOC_PPC_EG_AC_CEP_PORT_KEY *info
  );

void
  SOC_PPC_EG_VLAN_EDIT_CEP_INFO_clear(
    SOC_SAND_OUT SOC_PPC_EG_VLAN_EDIT_CEP_INFO *info
  );

void
  SOC_PPC_EG_VLAN_EDIT_VLAN_INFO_clear(
    SOC_SAND_OUT SOC_PPC_EG_VLAN_EDIT_VLAN_INFO *info
  );

void
  SOC_PPC_EG_AC_VLAN_EDIT_INFO_clear(
    SOC_SAND_OUT SOC_PPC_EG_AC_VLAN_EDIT_INFO *info
  );

void
  SOC_PPC_EG_AC_INFO_clear(
    SOC_SAND_OUT SOC_PPC_EG_AC_INFO *info
  );

void
  SOC_PPC_EG_AC_MP_INFO_clear(
    SOC_SAND_OUT SOC_PPC_EG_AC_MP_INFO *info
  );

#if SOC_PPC_DEBUG_IS_LVL1

void
  SOC_PPC_EG_AC_VBP_KEY_print(
    SOC_SAND_IN  SOC_PPC_EG_AC_VBP_KEY *info
  );

void
  SOC_PPC_EG_AC_CEP_PORT_KEY_print(
    SOC_SAND_IN  SOC_PPC_EG_AC_CEP_PORT_KEY *info
  );

void
  SOC_PPC_EG_VLAN_EDIT_CEP_INFO_print(
    SOC_SAND_IN  SOC_PPC_EG_VLAN_EDIT_CEP_INFO *info
  );

void
  SOC_PPC_EG_AC_MP_INFO_print(
    SOC_SAND_IN  SOC_PPC_EG_AC_MP_INFO *info
  );

#endif /* SOC_PPC_DEBUG_IS_LVL1 */

/* } */

#include <soc/dpp/SAND/Utils/sand_footer.h>

/* } __SOC_PPC_API_EG_AC_INCLUDED__*/
#endif

