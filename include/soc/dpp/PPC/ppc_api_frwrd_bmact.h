/* $Id: ppc_api_frwrd_bmact.h,v 1.7 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/
/******************************************************************
*
* FILENAME:       DuneDriver/ppc/include/soc_ppc_api_frwrd_bmact.h
*
* MODULE PREFIX:  soc_ppc_frwrd
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

#ifndef __SOC_PPC_API_FRWRD_BMACT_INCLUDED__
/* { */
#define __SOC_PPC_API_FRWRD_BMACT_INCLUDED__

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

/*     The B-MACT FID is the B-VID                             */
#define  SOC_PPC_BFID_EQUAL_TO_BVID (0)

/*     The B-MACT FID is '0'. Enable shared learning           */
#define  SOC_PPC_BFID_IS_0 (1)

/*     flags for SOC_PPC_BMACT_ENTRY_XXX flags */
#define  SOC_PPC_BMACT_ENTRY_TYPE_FRWRD      (0x1) /* BMACT DB */
#define  SOC_PPC_BMACT_ENTRY_TYPE_LEARN      (0x2) /* MACT Tunnel Learn DB */
#define  SOC_PPC_BMACT_ENTRY_TYPE_MC_DEST    (0x4) /* BMACT DB forwarding is Multicast */
 
 

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
   *  Backbone STP topology ID. PBB dedicated set of topology
   *  Ids. Does not share the I-components domain of STP
   *  topology Ids
   */
  uint32 stp_topology_id;
  /*
   *  SOC_PPC_BFID_EQUAL_TO_BVID: The B-MACT FID is the B-VID. The
   *  B-MACT FID: The B-MACT FID is '0'. Enable shared
   *  learning.
   */
  uint32 b_fid_profile;
  /*
   *  Default forwarding, upon unknown B-DAs.
   */
  SOC_PPC_FRWRD_DECISION_INFO uknown_da_dest;
  /*
   *  Default BVID forwarding decision to apply when there is
   *  no hit in the B-MAC table. The profile ID is part of the
   *  'dflt_frwrd_key', used by
   *  soc_ppd_vsi_default_frwrd_info_set(). Soc_petra only.
   */
  uint32 default_forward_profile;

} SOC_PPC_BMACT_BVID_INFO;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  First B-VID to be treated as PBB-TE
   */
  SOC_SAND_PP_VLAN_ID first_vid;
  /*
   *  Last B-VID to be treated as PBB-TE
   */
  SOC_SAND_PP_VLAN_ID last_vid;

} SOC_PPC_BMACT_PBB_TE_VID_RANGE;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  B-VID
   */
  SOC_SAND_PP_VLAN_ID b_vid;
  /*
   *  B-MAC-Address
   */
  SOC_SAND_PP_MAC_ADDRESS b_mac_addr;
  /* 
   *  Local port ID
   *  Used in case of Learning entry.
   *  Invalid for Petra-B.
   */
  SOC_PPC_PORT local_port_ndx;
  /*
   *  core ID
   *  Used in case of Learning entry.
   *  Valid for Jericho and above.
   */
  uint8 core;
  /* 
   *  flags SOC_PPC_BMACT_ENTRY_KEY_XXX
   *  Indicate if entry add is for BMACT DB (Forwarding)
   *  or for MIM Tunnel Learn DB that is used for
   *  C-SA learn information.
   *  Invalid for Petra-B.
   */
  uint32 flags;
  
} SOC_PPC_BMACT_ENTRY_KEY;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  System port ID. May be single port or LAG. Upon DA lookup:
   *  The destination system port, for packets that forwarded
   *  according to the B-VID * B-DA. Upon SA Lookup: Expected
   *  source system port. When the source port is not the
   *  expected, indicates that B-SA was transplanted, and the
   *  CPU is informed.
   */
  uint32 sys_port_id;
  /*
   *  Multicast ID. May be used as destination if sys_port_id is not used.
   */
  uint32 mc_id;
  /*
   *  I-SID-Domain. When not 'SOC_PPC_ISID_DM_DISABLE', the VSI is
   *  assigned according to the I-SID * I-SID domain
   */
  uint32 i_sid_domain;
  /*
   *  The FEC ID that hold the forwarding information to the
   *  destination port and MIM link-layer encapsulation. C-SA
   *  to be learned in the service MACT, will use it as
   *  forwarding destination. 
   */
  SOC_PPC_FEC_ID sa_learn_fec_id;
  /*
   *  Drop B-SA. TRUE: Drop packets that correspond to the
   *  bmac-key
   */
  uint8 drop_sa;

} SOC_PPC_BMACT_ENTRY_INFO;


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
  SOC_PPC_BMACT_BVID_INFO_clear(
    SOC_SAND_OUT SOC_PPC_BMACT_BVID_INFO *info
  );

void
  SOC_PPC_BMACT_PBB_TE_VID_RANGE_clear(
    SOC_SAND_OUT SOC_PPC_BMACT_PBB_TE_VID_RANGE *info
  );

void
  SOC_PPC_BMACT_ENTRY_KEY_clear(
    SOC_SAND_OUT SOC_PPC_BMACT_ENTRY_KEY *info
  );

void
  SOC_PPC_BMACT_ENTRY_INFO_clear(
    SOC_SAND_OUT SOC_PPC_BMACT_ENTRY_INFO *info
  );

#if SOC_PPC_DEBUG_IS_LVL1

void
  SOC_PPC_BMACT_BVID_INFO_print(
    SOC_SAND_IN  SOC_PPC_BMACT_BVID_INFO *info
  );

void
  SOC_PPC_BMACT_PBB_TE_VID_RANGE_print(
    SOC_SAND_IN  SOC_PPC_BMACT_PBB_TE_VID_RANGE *info
  );

void
  SOC_PPC_BMACT_ENTRY_KEY_print(
    SOC_SAND_IN  SOC_PPC_BMACT_ENTRY_KEY *info
  );

void
  SOC_PPC_BMACT_ENTRY_INFO_print(
    SOC_SAND_IN  SOC_PPC_BMACT_ENTRY_INFO *info
  );

#endif /* SOC_PPC_DEBUG_IS_LVL1 */

/* } */

#include <soc/dpp/SAND/Utils/sand_footer.h>

/* } __SOC_PPC_API_FRWRD_BMACT_INCLUDED__*/
#endif

