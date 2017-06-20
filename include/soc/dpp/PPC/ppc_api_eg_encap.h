/* $Id: ppc_api_eg_encap.h,v 1.27 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/
/******************************************************************
*
* FILENAME:       DuneDriver/ppc/include/soc_ppc_api_eg_encap.h
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

#ifndef __SOC_PPC_API_EG_ENCAP_INCLUDED__
/* { */
#define __SOC_PPC_API_EG_ENCAP_INCLUDED__

/*************
 * INCLUDES  *
 *************/
/* { */

#include <soc/dpp/SAND/Utils/sand_header.h>

#include <soc/dpp/SAND/Management/sand_general_macros.h>
#include <soc/dpp/SAND/Management/sand_error_code.h>

#include <soc/dpp/PPC/ppc_api_general.h>

#include <soc/dpp/dpp_config_defs.h>

/* } */
/*************
 * DEFINES   *
 *************/
/* { */

/*     Maximum number of tunnels per encapsulation             */
#define  SOC_PPC_EG_ENCAP_MPLS_MAX_NOF_TUNNELS                      (2)

/*     Data entry maximum size in uint32 */
#define  SOC_PPC_EG_ENCAP_DATA_INFO_MAX_SIZE                        (SOC_DPP_DEFS_MAX(NOF_EEDB_PAYLOADS) / SOC_SAND_NOF_BITS_IN_UINT32 + 1)

/*     Data entry program entry size in uint32 */
#define  SOC_PPC_EG_ENCAP_DATA_PROG_ENTRY_MAX_SIZE                  (4)

/*    Number of banks in egress encapsulation                   */ 
#define SOC_PPC_EG_ENCAP_NOF_BANKS(unit)                           (SOC_DPP_DEFS_GET(unit,eg_encap_nof_banks)) 
#define SOC_PPC_EG_ENCAP_BANK_NDX_MAX(unit)                        (SOC_PPC_EG_ENCAP_NOF_BANKS(unit)-1) 

/* next eep pointer is invalid */
#define SOC_PPC_EG_ENCAP_NEXT_EEP_INVALID                        (0xffffffff)


typedef enum
{
    SOC_PPC_EG_ENCAP_ENTRY_UPDATE_INVALID = 0,
    /* update next-out-LIF */
    SOC_PPC_EG_ENCAP_ENTRY_UPDATE_NEXT_LIF = 0x1,
    /* update drop  */
    SOC_PPC_EG_ENCAP_ENTRY_UPDATE_DROP = 0x2,
    /*Update Out-LIF-Profile*/
    SOC_PPC_EG_ENCAP_ENTRY_UPDATE_OUT_LIF_PROFILE = 0x4,
    SOC_PPC_EG_ENCAP_ENTRY_UPDATE_TYPE_MAX
    
}SOC_PPC_EG_ENCAP_ENTRY_UPDATE_TYPE;

#define SOC_PPC_PRGE_DATA_ENTRY_LSBS_ERSPAN        (2)
#define SOC_PPC_PRGE_DATA_ENTRY_LSBS_RSPAN         (1)
#define SOC_PPC_PRGE_DATA_ENTRY_LSBS_PON_TUNNEL    (1)


/* definition for ERSPAN mirroring*/
/* priority value (const) */
#define SOC_PPC_EG_ENCAP_ERSPAN_PRIO_VAL      (0)
/* truncated value (const) */
#define SOC_PPC_EG_ENCAP_ERSPAN_TRUNC_VAL     (0)

/* Defines for header compensation per packet. */
#define PPC_API_EG_ENCAP_PER_PKT_HDR_COMP_NOF_VALUE_LSBS    (6)

/* } */
/*************
 * MACROS    *
 *************/
/* { */

  /* 6LSBytes {Priority(3),Reserved(1),Direction(1),Truncated(1),SPAN-ID(10)} Directly Extracted from EES1[55:8]*/
#define SOC_PPC_EG_ENCAP_EEDB_ERSPAN_FORMAT_SET(__unit, __prio, __trunc, __span_id, __data) \
            (__data)[1] = (((__prio << 21)) | ((__trunc) << 18) | ((__span_id) << 8));                                                    \
          (__data)[0] = SOC_PPC_PRGE_DATA_ENTRY_LSBS_ERSPAN;

#define SOC_PPC_EG_ENCAP_EEDB_DATA_ERSPAN_FORMAT_SET(__unit, __prio, __trunc, __span_id,__eg_encap_data_info)  \
    SOC_PPC_EG_ENCAP_EEDB_ERSPAN_FORMAT_SET(__unit, __prio, __trunc, __span_id, (__eg_encap_data_info)->data_entry)

#define SOC_PPC_EG_ENCAP_DATA_ERSPAN_FORMAT_SPAN_ID_GET(__unit, __eg_encap_data_info)  (((__eg_encap_data_info)->data_entry[1] >> 8)& 0x3ff)

/* Macro determines weather a EEDB entry is used for ERSPAN or RSAPAN*/
#define SOC_PPC_EG_ENCAP_EEDB_IS_FORMAT_ERSPAN(__eg_encap_data_info)  (((__eg_encap_data_info)->data_entry[0]  & 0xff) == SOC_PPC_PRGE_DATA_ENTRY_LSBS_ERSPAN)


/* } */
/*************
 * TYPE DEFS *
 *************/
/* { */

typedef enum
{
  /*
   *  The EEP points to LIF Editing Table.
   */
  SOC_PPC_EG_ENCAP_EEP_TYPE_LIF_EEP = 0,
  /*
   *  The EEP points to Tunnels Editing Table.
   */
  SOC_PPC_EG_ENCAP_EEP_TYPE_TUNNEL_EEP = 1,
  /*
   *  The EEP points to LL encapsulation Table.
   */
  SOC_PPC_EG_ENCAP_EEP_TYPE_LL = 2,
  /*
   *  Number of types in SOC_PPC_EG_ENCAP_EEP_TYPE in Soc_petraB
   */
  SOC_PPC_NOF_EG_ENCAP_EEP_TYPES_PB = 3,
  /* 
   * The EEP points to VSI 
   * valid only for ARAD. 
   */ 
  SOC_PPC_EG_ENCAP_EEP_TYPE_VSI = SOC_PPC_NOF_EG_ENCAP_EEP_TYPES_PB,
  /*
   * The EEP points to Data Entry. 
   * valid only for ARAD         .                            .
   */
  SOC_PPC_EG_ENCAP_EEP_TYPE_DATA = 5,
  /* 
   * The EEP points to ROO LL encapsulation table.
   * valid only for Jericho. In ARAD+ EEP type is data
   */ 
  SOC_PPC_EG_ENCAP_EEP_TYPE_ROO_LL = 6, 
  /* 
   *The EEP points to Trill encapsulation Table. 
   * Valid only for ARAD+.  
   */ 
  SOC_PPC_EG_ENCAP_EEP_TYPE_TRILL = 7, 
  /*
   *  Number of types in SOC_PPC_EG_ENCAP_EEP_TYPE in ARAD
   */
  SOC_PPC_NOF_EG_ENCAP_EEP_TYPES_ARAD = 8
}SOC_PPC_EG_ENCAP_EEP_TYPE;

#define SOC_PPC_NOF_EG_ENCAP_EEP_TYPES_MAX      SOC_SAND_MAX(SOC_PPC_NOF_EG_ENCAP_EEP_TYPES_PB, SOC_PPC_NOF_EG_ENCAP_EEP_TYPES_ARAD)

typedef enum
{
  /*
   *  The EXP of the pushed MPLS label is taken from the push
   *  profile entry soc_ppd_eg_encap_push_profile_info_set()
   */
  SOC_PPC_EG_ENCAP_EXP_MARK_MODE_FROM_PUSH_PROFILE = 0,
  /*
   *  The EXP of the pushed MPLS label is mapped from TC and
   *  DP parameters according to
   *  soc_ppd_eg_encap_push_exp_info_set(). T20E only.
   */
  SOC_PPC_EG_ENCAP_EXP_MARK_MODE_MAP_TC_DP = 1,
  /*
   *  Number of types in SOC_PPC_EG_ENCAP_EXP_MARK_MODE
   */
  SOC_PPC_NOF_EG_ENCAP_EXP_MARK_MODES = 2
}SOC_PPC_EG_ENCAP_EXP_MARK_MODE;

typedef enum
{
  /*
   *  The egress encapsulation entry includes VSI
   */
  SOC_PPC_EG_ENCAP_ENTRY_TYPE_VSI = 0,
  /*
   *  The egress encapsulation entry includes AC
   */
  SOC_PPC_EG_ENCAP_ENTRY_TYPE_AC = 1,
  /*
   *  The egress encapsulation entry includes SWAP command
   */
  SOC_PPC_EG_ENCAP_ENTRY_TYPE_SWAP_CMND = 2,
  /*
   *  The egress encapsulation entry includes PWE (VC
   *  encapsulation)
   */
  SOC_PPC_EG_ENCAP_ENTRY_TYPE_PWE = 3,
  /*
   *  The egress encapsulation entry includes POP command
   */
  SOC_PPC_EG_ENCAP_ENTRY_TYPE_POP_CMND = 4,
  /*
   *  The egress encapsulation entry includes MPLS
   *  encapsulations (tunneling)
   */
  SOC_PPC_EG_ENCAP_ENTRY_TYPE_MPLS_ENCAP = 5,
  /*
   *  The egress encapsulation entry includes IPv4
   *  encapsulations (tunneling)
   */
  SOC_PPC_EG_ENCAP_ENTRY_TYPE_IPV4_ENCAP = 6,
  /*
   *  The egress encapsulation entry includes LL encapsulation
   */
  SOC_PPC_EG_ENCAP_ENTRY_TYPE_LL_ENCAP = 7,
  /*
   *  null entry, no encapsulation. Only points to next
   *  encapsulation.
   */
  SOC_PPC_EG_ENCAP_ENTRY_TYPE_NULL = 8,
  /*
   *  Number of types in SOC_PPC_EG_ENCAP_ENTRY_TYPE
   */
  SOC_PPC_NOF_EG_ENCAP_ENTRY_TYPES_PB = 9,
  /*
   *  data entry.
   */
  SOC_PPC_EG_ENCAP_ENTRY_TYPE_DATA = 9,
  /*
   *  The egress encapsulation entry includes IPv6
   *  encapsulations (tunneling)
   */
  SOC_PPC_EG_ENCAP_ENTRY_TYPE_IPV6_ENCAP = 10,
  /*
   *  The egress encapsulation entry includes Mirror
   *  encapsulations (tunneling)
   */
  SOC_PPC_EG_ENCAP_ENTRY_TYPE_MIRROR_ENCAP = 11,
  /*
   * The egress encapsulation entry includes overlay arp
   * encapsulation (for roo vxlan in arad+ )
   */
  SOC_PPC_EG_ENCAP_ENTRY_TYPE_OVERLAY_ARP_ENCAP = 12, 
  /*
   *
   */
    SOC_PPC_EG_ENCAP_ENTRY_TYPE_ROO_LL_ENCAP = 13, 
  /*
   * The egress encapsulation entry includes Trill
   * encapsulation 
   */
    SOC_PPC_EG_ENCAP_ENTRY_TYPE_TRILL_ENCAP = 14, 
  /*
   *  Number of types in SOC_PPC_EG_ENCAP_ENTRY_TYPE
   */
  SOC_PPC_NOF_EG_ENCAP_ENTRY_TYPES_ARAD = 15 
}SOC_PPC_EG_ENCAP_ENTRY_TYPE;

typedef enum
{
  /*
   * No additional encapsulation
   */
  SOC_PPC_EG_ENCAP_ENCAPSULATION_MODE_NONE = 0,
  /*
   * Ether IP encapsulation 
   */
  SOC_PPC_EG_ENCAP_ENCAPSULATION_MODE_ETHER_IP= 1,
  /*
   * Basic GRE (4B) encapsulation
   */
      SOC_PPC_EG_ENCAP_ENCAPSULATION_MODE_BASIC_GRE = 2,
  /*
   *  Inhance GRE (8B) encapsulation
   */
  SOC_PPC_EG_ENCAP_ENCAPSULATION_MODE_ENHANCE_GRE = 3,
  /* 
   *  VXLAN encapsulation
   */
  SOC_PPC_EG_ENCAP_ENCAPSULATION_MODE_VXLAN = 4,
  /*
   *  Number of types in SOC_PPC_EG_ENCAP_ENCAPSULATION_MODE
   */
  SOC_PPC_NOF_EG_ENCAP_ENCAPSULATION_MODE_TYPES = 5

}SOC_PPC_EG_ENCAP_ENCAPSULATION_MODE;

typedef enum {
    /*
     * PWE 
     */
    SOC_PPC_EG_ENCAP_ACCESS_PHASE_TYPE_TUNNEL_1,
    /*
     * MPLS tunnel, IP tunnel, I-SID, Out-RIF, TRILL
     */
    SOC_PPC_EG_ENCAP_ACCESS_PHASE_TYPE_TUNNEL_2,
    /*
     * Link Layer 
     */
    SOC_PPC_EG_ENCAP_ACCESS_PHASE_TYPE_LL_ARP,
    /*
     * AC
     */
    SOC_PPC_EG_ENCAP_ACCESS_PHASE_TYPE_AC,
    /*
     * Native Link Layer - ARP
     */
    SOC_PPC_EG_ENCAP_ACCESS_PHASE_TYPE_NATIVE_LL_ARP,
    /*
     * Native AC
     */
    SOC_PPC_EG_ENCAP_ACCESS_PHASE_TYPE_NATIVE_AC,
    /*
     * Invalid.
     */
    SOC_PPC_EG_ENCAP_ACCESS_PHASE_TYPE_COUNT
} SOC_PPC_EG_ENCAP_ACCESS_PHASE_TYPE;

typedef enum
{
  /*
   * access phase: Arad-Jer: PWE 
   *               QAX: Native-ARP 
   */
  SOC_PPC_EG_ENCAP_ACCESS_PHASE_ONE = 0,
  /*
   * access phase: Arad-Jer: MPLS tunnel, IP tunnel, I-SID, Out-RIF, TRILL
   *               QAX: PWE
   */ 
  SOC_PPC_EG_ENCAP_ACCESS_PHASE_TWO = 1,
  /*
   * access phase: Arad-Jer: link layer 
   *               QAX: MPLS tunnel, IP tunnel, I-SID, TRILL
   */
  SOC_PPC_EG_ENCAP_ACCESS_PHASE_THREE = 2,
  /*
   *  access phase: Arad-Jer: AC
   *                QAX: link layer 
   */
  SOC_PPC_EG_ENCAP_ACCESS_PHASE_FOUR = 3,
  /*
   * access phase: QAX only: Native AC
   */
  SOC_PPC_EG_ENCAP_ACCESS_PHASE_FIVE = 4,
  /*
   * access phase: QAX only: AC
   */
  SOC_PPC_EG_ENCAP_ACCESS_PHASE_SIX = 5,
  /*
   * Number of members in enum. Not number of phases.
   */
  SOC_PPC_EG_ENCAP_ACCESS_PHASE_COUNT = 6
}SOC_PPC_EG_ENCAP_ACCESS_PHASE;

/*
 *  Number of EEDB phases.
 */
#define SOC_PPC_NOF_EG_ENCAP_ACCESS_PHASE_TYPES(unit)  (SOC_DPP_DEFS_GET(unit, eg_encap_nof_phases))

typedef enum
{
  /*
   * data entry for erspan mirror
   */
  SOC_PPC_EG_ENCAP_DATA_TYPE_MIRROR_ERSPAN = 0,
  /*
   * data entry for IPv6 Tunnel
   */
  SOC_PPC_EG_ENCAP_DATA_TYPE_IPV6_TUNNEL = 1,
  /*
   * data entry for IPv6 Tunnel DIP entry
   */
  SOC_PPC_EG_ENCAP_DATA_TYPE_IPV6_TUNNEL_DIP = 2,
  /*
   * data entry for IPv6 Tunnel SIP entry
   */
  SOC_PPC_EG_ENCAP_DATA_TYPE_IPV6_TUNNEL_SIP = 3,
  /*
   *  Number of types in SOC_PPC_EG_ENCAP_DATA_TYPE
   */
  SOC_PPC_NOF_EG_ENCAP_DATA_TYPES = 4
}SOC_PPC_EG_ENCAP_DATA_TYPE;


typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  Link-Layer-Limit. Entries '0'- ll_limit are used for
   *  link-layer encapsulation. Range: 0-4K-1.
   *  Invalid for ARAD.
   */
  uint32 ll_limit;

  /* IP-Tunnels-Limit.
   * Entries (ll_limit+1)-ip_tnl_limit are used for IP
   * Tunnels encapsulation.
   * Range: (ll_limit + 1)-12K-1
   * Entries (ip_tnl_limit+1)- 12K-1: are used for PWE, MPLS
   * tunnels, where PWE encapsulation entries must be >= 4K.
   * When defining two tunnels in different entries first encap
   * entry must be >= 4K. 
   * Invalid for ARAD. 
   */
  uint32 ip_tnl_limit;

  /* 
   * Set per EEDB bank one of four action access phases. 
   */ 
  SOC_PPC_EG_ENCAP_ACCESS_PHASE bank_access_phase[SOC_DPP_DEFS_MAX(EG_ENCAP_NOF_BANKS)];

} SOC_PPC_EG_ENCAP_RANGE_INFO;


typedef struct
{
    SOC_SAND_MAGIC_NUM_VAR
    /* 
     * Indication whether the Egress Protection information is valid for the Out-LIF.
     * Used in Jericho.
     */ 
    uint32 is_protection_valid; 
    /* 
     * Failure state instance index, used to determine traffic pass for the flow.
     * Used in Jericho.
     */ 
    uint32 protection_pointer; 
    /*  
     * Failure state pass value. 
     * In Coupled mode, if protection_pass_value != out_lif.pass_value, the 
     * packet is dropped. 
     * In De-Coupled mode, if protection_pass_value = 0, the packet is dropped.
     * range: 0 - 1. 
     * Used in Jericho.
     */ 
    uint8 protection_pass_value; 
} SOC_PPC_EG_ENCAP_PROTECTION_INFO;



typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  VLAN ID. Range: 0 - 4095.
   */
  SOC_SAND_PP_VLAN_ID vid;
  /*
   *  Priority Code Point. Refers to the IEEE 802.1p priority.
   *  For C-Tag it is the User Priority. Range: 0 - 7.
   */
  SOC_SAND_PP_PCP_UP pcp;
  /*
   *  Drop Eligibility Indicator. For C-tag, this is the CFI
   *  (Canonical Format Indicator) and has to be 0. Range: 0 -
   *  1.
   */
  SOC_SAND_PP_DEI_CFI dei;

} SOC_PPC_EG_VLAN_EDIT_VLAN_INFO;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  Vlan tag, C-tag, or S-tag to be used for the VLAN
   *  editing. According to
   *  soc_ppd_eg_vlan_edit_command_info_set(), the user may -
   *  select to build/replace the VID of the packet with one
   *  of these tags. - set the UP/PCP of the packet to this
   *  value, or to map it according to COS attributes (see
   *  pcp_profile).
   *  Note: for Soc_petra-B, only the VIDs fields of these Tags are
   *  relevant, the PCP-DEI may be acquired by mapping from
   *  pcp_profile.
   */
  SOC_PPC_EG_VLAN_EDIT_VLAN_INFO vlan_tags[SOC_PPC_VLAN_TAGS_MAX];
  /*
   *  Number of Vlan tags. Soc_petra-B and T20E support 2 tags:
   *  vlan_tags[0] is used for building the outer tag and
   *  vlan_tags[1] for building the inner tag.
   */
  uint32 nof_tags;
  /*
   *  Profile according to which to edit the VLAN tags. See
   *  soc_ppd_eg_vlan_edit_command_info_set(). According to
   *  tag-format which represents the Tag structure of the
   *  packet and this edit-profile, the user may define an
   *  Edit command to build the header of the outgoing
   *  packets. Range:
   *    Up till Jerico: 0 - 15,
   *    Jericho and above: 0-31
   */
  uint32 edit_profile;
  /*
   *  Profile to build the packet PCP-DEI/UP. See COS Mapping
   *  APIs in Egress Edit Module: soc_ppd_eg_vlan_edit_pcp_map_stag_set,
   *  soc_ppd_eg_vlan_edit_pcp_map_ctag_set, soc_ppd_eg_vlan_edit_pcp_map_untagged_set.
   *  These APIs map COS parameters (UP/PCP/DEI/TC/DP) (and
   *  pcp_profile) to PCP and DEI. These profiles let the user
   *  configure up to 16 such mappings. Range: 0 - 15.
   */
  uint32 pcp_profile;

  uint32 lif_profile;

  /* 
   * OAM LIF indication value. 
   * If set, then OAM LIF value is equal to the OutLIF index.
   * ARAD only. 
   */ 
  uint32 oam_lif_set;
#ifdef BCM_88660_A0
  /* 
   * Data entry indication value. 
   * If set, the entire EEDB line is pushed to the data, which is transferred to the programmable editor.
   * When set, all entry is used where first entry is the regular out-AC and second entry 
   * contains data information. ARAD Plus only. 
   */
  uint32 use_as_data_entry;

  /* 
   * Second EEDB entry Data information is passed to the programmable editor for flexiable settings.
   * An example of use in PON application, addition of Tunnel-ID in case of 3-tags manipulation.
   * Tunnel-TPID(16b), PCP-DEI(4b), Tunnel-ID(12b), 2lsbs identifier.
   * ARAD Plus only. 
   */
  uint32 data[SOC_PPC_EG_ENCAP_DATA_INFO_MAX_SIZE];
#endif
} SOC_PPC_EG_AC_VLAN_EDIT_INFO;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  Information to edit packet header.
   */
  SOC_PPC_EG_AC_VLAN_EDIT_INFO edit_info;
  /* 
   * Protection Pointer and pass value to select traffic pass/drop.
   */ 
  SOC_PPC_EG_ENCAP_PROTECTION_INFO protection_info;

} SOC_PPC_EG_AC_INFO;


typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  The label to swap the incoming label. Range: 0 - 1M.
   */
  uint32 swap_label;
  /*
   *  The VSI to assign to the packet, for the processing of
   *  the Link-Layer header.
   */
  SOC_PPC_VSI_ID out_vsi;
  /* 
   * OAM LIF indication value. 
   * If set, then OAM LIF value is equal to the OutLIF index.
   * ARAD only. 
   */ 
  uint32 oam_lif_set;
  /* 
   * Filter value. 
   * If set, packets forwarded to the OutLif will be filtered.
   */ 
  uint32 drop;
  /*
   * outlif profile (Jericho only)
   */
  uint32 outlif_profile;
  /* remark profile*/
  uint32 remark_profile;
} SOC_PPC_EG_ENCAP_SWAP_INFO;


typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  Needed if pop_type is
   *  SOC_PPC_MPLS_COMMAND_TYPE_POP_INTO_ETHERNET. Needed for the
   *  Inner L2 header parsing
   */
  uint32 tpid_profile;
  /*
   *  Has Control-word. Used to determine number of bytes to
   *  remove from the header.
   */
  uint8 has_cw;

} SOC_PPC_EG_ENCAP_POP_INTO_ETH_INFO;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  Command to perform on the packets label.
   *  In ARAD must be SOC_PPC_MPLS_COMMAND_TYPE_POP.
   */
  SOC_PPC_MPLS_COMMAND_TYPE pop_type;
  /*
   *  In Soc_petra-B Relevant only if pop_type is
   *  SOC_PPC_MPLS_COMMAND_TYPE_POP_INTO_ETHERNET. Needed to
   *  process the inner Ethernet Header.
   *  In ARAD always relevant.
   */
  SOC_PPC_EG_ENCAP_POP_INTO_ETH_INFO ethernet_info;
  /*
   *  Type of next header.
   *  Must be of type: IPv4, Ipv6, MPLS, Ethernet.
   */
  SOC_PPC_PKT_FRWRD_TYPE pop_next_header;
  /*
   *  Whether pop label include control world
   *  Arad only.
   */
  SOC_SAND_PP_MPLS_TUNNEL_MODEL model;
  /* 
   * OAM LIF indication value. 
   * If set, then OAM LIF value is equal to the OutLIF index.
   * ARAD only. 
   */ 
  uint32 oam_lif_set;
  /* 
   * Filter value. 
   * If set, packets forwarded to the OutLif will be filtered.
   */ 
  uint32 drop;
  /*
   * outlif profile (Jericho only)
   */
  uint32 outlif_profile;
  /*
   *  Out-VSI, according to which to process follows Ethernet header,
   *  Range: Soc_petra-B : 0 - 4K-1.
   */
  uint32 out_vsi;
} SOC_PPC_EG_ENCAP_POP_INFO;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  Label of the tunnel.
   */
  uint32 tunnel_label;
  /*
   *  Used to construct the label's TTL and EXP. Range: 0 - 7.
   */
  uint32 push_profile;
} SOC_PPC_EG_ENCAP_MPLS_TUNNEL_INFO;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  Tunnels to encapsulate the packets with. The
   *  push_profile of the second tunnel has to be in the range
   *  0-3.
   */
  SOC_PPC_EG_ENCAP_MPLS_TUNNEL_INFO tunnels[SOC_PPC_EG_ENCAP_MPLS_MAX_NOF_TUNNELS];
  /*
   *  Number of labels to encapsulate. Range: 0 - 2.
   */
  uint32 nof_tunnels;
  /*
   *  Tunnel/s orientation Hub or Spoke. Packets forwarded
   *  from hub interface to hub interface will be filtered.
   *  Invalid for ARAD. use soc_ppd_pp_eg_filter_split_horizon_out_lif_orientation_set
   */
  SOC_SAND_PP_HUB_SPOKE_ORIENTATION orientation;
  /*
   *  The VSI to assign to the packet. Used to retrieve the
   *  MPLS IF MAC source address to be set in the encapsulated
   *  link layer header. In Soc_petra-B only, it is also used to
   *  index the egress VLAN editing database (together with
   *  the Out Port).
   */
  SOC_PPC_VSI_ID out_vsi;
  /* 
   * OAM LIF indication value. 
   * If set, then OAM LIF value is equal to the OutLIF index.
   * ARAD only. 
   */ 
  uint32 oam_lif_set;
  /* 
   * Filter value. 
   * If set, packets forwarded to the OutLif will be filtered.
   */ 
  uint32 drop;
  /*
   * outlif profile (Jericho only)
   */
  uint32 outlif_profile;
  /* 
   * Protection Pointer and pass value to select traffic pass/drop.
   */ 
  SOC_PPC_EG_ENCAP_PROTECTION_INFO protection_info;

} SOC_PPC_EG_ENCAP_MPLS_ENCAP_INFO;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  Label to push as VC label. Range: 0 - 1M.
   */
  uint32 label;
  /*
   *  Push profile used to construct the label's TTL and
   *  EXP. Range: 0 - 7.
   */
  uint32 push_profile;
  /*
   * 	PWE/s orientation Hub or Spoke. Packets forwarded from
   *  hub interface to hub interface will be filtered.
   * Useful for H-VPLS, default of clear is HUB. 
   *  Invalid for ARAD. use soc_ppd_pp_eg_filter_split_horizon_out_lif_orientation_set 
   */
  SOC_SAND_PP_HUB_SPOKE_ORIENTATION orientation;
  /* 
   * OAM LIF indication value. 
   * If set, then OAM LIF value is equal to the OutLIF index.
   * ARAD only. 
   */ 
  uint32 oam_lif_set;
  /* 
   * Filter value. 
   * If set, packets forwarded to the OutLif will be filtered.
   */ 
  uint32 drop;
  /* 
   * outlif profile (Jericho only)
   */ 
  uint32 outlif_profile;
  /* 
   * Protection Pointer and pass value to select traffic pass/drop.
   */ 
  SOC_PPC_EG_ENCAP_PROTECTION_INFO protection_info;

  /* Information for mpls label that us binded with pwe */
  SOC_PPC_EG_ENCAP_MPLS_ENCAP_INFO egress_tunnel_label_info;

} SOC_PPC_EG_ENCAP_PWE_INFO;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  IPv4 destination address.
   */
  uint32 dest;
  /*
   *  Select 1 of 16 source address. See
   *  soc_ppd_eg_encap_ipv4_tunnel_glbl_src_ip_set. Range: 0 - 15.
   */
  uint32 src_index;
  /*
   *  Select 1 of 3 TTL values. See
   *  soc_ppd_eg_encap_ipv4_tunnel_glbl_ttl_set. Range: 0 - 3.
   */
  uint8 ttl_index;
  /*
   *  Select 1 of 16 TOS values. See
   *  soc_ppd_eg_encap_ipv4_tunnel_glbl_tos_set. Range: 0 - 15.
   */
  uint8 tos_index;
  /*
   *  Soc_petraB only.
   *  If TRUE, then the IPv4 tunnel has a GRE header. The GRE
   *  header is Constant.
   *  invalid for ARAD.
   */
  uint8 enable_gre;
  /* 
   *  ARAD only. 
   *  Encapsulation mode. 
   */ 
  SOC_PPC_EG_ENCAP_ENCAPSULATION_MODE encapsulation_mode;
  /*
   * Jericho B0, QAX and forward
   * We use encapsulation mode index to differentiate with the already existing encapsulation_mode.
   * In Arad, encapsulation mode was defining the tunnel type (consequently its type is an enum)
   * for Jericho B0, QAX and forward, it's an index, mapped with IP.protocol, encapsulation size, and template (which is the tunnel type)
   */
   int encapsulation_mode_index; 

} SOC_PPC_EG_ENCAP_IPV4_TUNNEL_INFO;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  IPv4 destination address.
   */
  SOC_PPC_EG_ENCAP_IPV4_TUNNEL_INFO dest;
  /*
   *  The VSI to assign to the packet, for the processing of
   *  the Link-Layer header.
   */
  SOC_PPC_VSI_ID out_vsi;
  /* 
   * OAM LIF indication value. 
   * If set, then OAM LIF value is equal to the OutLIF index.
   * ARAD only. 
   */ 
  uint32 oam_lif_set;
  /* 
   * Filter value. 
   * If set, packets forwarded to the OutLif will be filtered.
   */ 
  uint32 drop;
  /*
   * Jericho only!
   * Used as orientation (aka forward group) 
   * In QAX, used also as is_l2_lif for ROO application
   * OutLIF profile value.
   */
  uint32 outlif_profile;

} SOC_PPC_EG_ENCAP_IPV4_ENCAP_INFO;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  DA MAC address.
   */
  SOC_SAND_PP_MAC_ADDRESS dest_mac;
  /*
   *  T20E: used to set Tag for P2P link layer, If TRUE then
   *  Vlan Tag with out_vid and pcp_deiis built over the MAC
   *  header.
   *  ARAD/Soc_petra-B: Is out_vid valid. Relevant ONLY for MIM
   *  tunnel and TRILL adjacent; otherwise, has to be FALSE.
   */
  uint8 out_vid_valid;
  /*
   *  Out-VID to set in the packet. T20E: used to set Tag for
   *  P2P link layer.
   *  ARAD/Soc_petra-B: relevant for MIM tunnel and
   *  TRILL adjacent. In all other applications, it is a
   *  function of the out-VSI.
   */
  SOC_SAND_PP_VLAN_ID out_vid;
  /*
   *  Relevant ONLY for T20E: used to set Tag for P2P link
   *  layer if TRUE then pcp_dei is taken otherwise PCP_DEI is
   *  set according to in-EXP
   */
  uint8 pcp_dei_valid;
  /*
   *  Relevant ONLY for T20E: used to set Tag for P2P link
   *  layer.
   */
  uint8 pcp_dei;
  /*
   *  Relevant ONLY for T20E: used to set TPID of P2P link
   *  layer VLAN Tag. Select one of the four global TPIDs
   *  configured by soc_ppd_llp_parse_tpid_values_set() as the
   *  TPID of the tag of the packet (if exists).
   */
  uint8 tpid_index;
  /* 
   * Select 1 of 4 Remark profiles. See 
   * soc_ppd_qos_remark_profile_set. Range: 0-3 
   * valid only for ARAD. 
   */ 
  uint8 ll_remark_profile; 
  /*
   * Is out_ac valid. 
   * valid only for ARAD.
   */
  uint8 out_ac_valid;
  /*
   *  Next_outlif_lsb.
   *  valid only for ARAD.
   */
  uint32  out_ac_lsb;
  /* 
   * OAM LIF indication value. 
   * If set, then OAM LIF value is equal to the OutLIF index.
   * ARAD only. 
   */ 
  uint32 oam_lif_set;
  /* 
   * Filter value. 
   * If set, packets forwarded to the OutLif will be filtered.
   */ 
  uint32 drop;
  /* 
   * outlif profile.
   * Used as orientation (aka forward group).
   * Valid only for JERICHO.  
   */
  uint32 outlif_profile; 
  /* 
   * native_ll.
   * Used as native ll or normal ll.
   * Valid for QUX, Jericho+, QAX B0 
   */
  uint32 native_ll; 
}SOC_PPC_EG_ENCAP_LL_INFO;




typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  Tunnel model (Pipe or uniform),- if Pipe: then TTL and
   *  EXP of the push tunnel are taken from the values
   *  configured in this entry- if Uniform then the TTL and
   *  EXP of the push label are set according to the network
   *  headers below the pushed label see
   *  soc_ppd_eg_qos_params_remark_set()
   */
  SOC_SAND_PP_MPLS_TUNNEL_MODEL model;
  /*
   *  Has Control-word. Used to determine whether to add CW
   *  under the tunnel header. The value of CW is set
   *  according to soc_ppd_eg_encap_glbl_info_set()
   */
  uint8 has_cw;
  /*
   *  TTL to set in the tunnel header in case tunnel model is
   *  pipe. Range: 0 - 255.
   */
  SOC_SAND_PP_IP_TTL ttl;
  /*
   *  How to generate the EXP of the pushed tunnel, options: -
   *  Set EXP value according to this entry (exp value in this
   *  struct).- Set EXP value by mapping TC and DP (T20E only)
   *  see soc_ppd_eg_encap_push_exp_info_set()
   */
  SOC_PPC_EG_ENCAP_EXP_MARK_MODE exp_mark_mode;
  /*
   *  EXP to set in the pushed label header in case tunnel
   *  model is pipe. Relevant only if exp_mark_mode is set
   *  according to profile entry
   *  (SOC_PPC_EG_ENCAP_EXP_MARK_MODE_FROM_PUSH_PROFILE)Range: 0 -
   *  7. T20E: to set EXP according to TC and DP see
   *  soc_ppd_eg_encap_push_exp_info_set()
   */
  SOC_SAND_PP_MPLS_EXP exp;
  /* 
   *  Select 1 of 16 Remark profiles. See 
   *  soc_ppd_qos_remark_profile_set. Range: 0-15.
   *  Invalid for Soc_petra-B.
   */
  uint8 remark_profile;
  /* 
   *  If set, Entropy label will be added. 
   *  Invalid for Soc_petra-B.
   */
  uint8 add_entropy_label;
  /* 
   *  If set, Entropy label indicator will be added. 
   *  Valid from QAX and above.
   */
  uint8 add_entropy_label_indicator;
  /* From QAX: indicates the ttl model, whether set according to push profile
     or copy from previous header */
  SOC_SAND_PP_MPLS_TUNNEL_MODEL ttl_model;
  /* From QAX: indicates the exp model, whether set according to push profile
     or copy from previous header */
  SOC_SAND_PP_MPLS_TUNNEL_MODEL exp_model;

} SOC_PPC_EG_ENCAP_PUSH_PROFILE_INFO;

typedef struct 
{ 
  SOC_SAND_MAGIC_NUM_VAR 
  /* 
   * Select 1 of 16 Remark profiles. See 
   * soc_ppd_qos_remark_profile_set. Range: 0-15 
   */ 
  uint8 remark_profile; 
  /* 
   * The VSI to assign to the packet, for the processing of 
   * the Link-Layer header. 
   */ 
  SOC_PPC_VSI_ID out_vsi; 
  /* 
   * OAM LIF indication value. 
   * If set, then OAM LIF value is equal to the OutLIF index.
   * ARAD only. 
   */ 
  uint32 oam_lif_set;
  /* 
   * Filter value. 
   * If set, packets forwarded to the OutLif will be filtered.
   */ 
  uint32 drop;
  /*
   * outlif profile (Jericho only)
   */
  uint32 outlif_profile;

  /*
   * outrif profile (QAX only)
   */
  uint32 outrif_profile_index;

} SOC_PPC_EG_ENCAP_VSI_ENCAP_INFO; 


/* ARAD only typedefs { */
typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  Data information.
   */
  uint32 data_entry[SOC_PPC_EG_ENCAP_DATA_INFO_MAX_SIZE];
  /* 
   * OAM LIF indication value. 
   * If set, then OAM LIF value is equal to the OutLIF index.
   */ 
  uint32 oam_lif_set;
  /* 
   * Filter value. 
   * If set, packets forwarded to the OutLif will be filtered.
   */ 
  uint32 drop;
  /*
   * Outlif profile
   */
  uint32 outlif_profile;
} SOC_PPC_EG_ENCAP_DATA_INFO;


typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR

  /*
   *  DA MAC address.
   */
  SOC_SAND_PP_MAC_ADDRESS dest_mac;

  /*
   *  SA MAC address.
   */
  SOC_SAND_PP_MAC_ADDRESS src_mac;

  /* 
   * TPID (valid if not untagged) 
   * arad+ only.
   * (in Jericho, tpid is saved in an additional table (eth type index table))
   */
  uint16 outer_tpid; 

  /*
   * PCP-DEI (valid if not untagged) 
   */
  uint8 pcp_dei;

  /*
   *  Out-VID to set in the packet. (valid if not untagged) 
   */
  SOC_SAND_PP_VLAN_ID out_vid;

  /* 
   * L2 ether_type
   * arad+ only.
   * (in Jericho, ether type is saved in an additional table (eth type index table))
   */ 
  uint16 ether_type;

  /* 
   * for arad+ only:  
   * Relative index from prge-data. Prge-data has a reserved range for overlay arp.
   * (prge-data is used only in roo overlay arp entry for arad+)
   */ 
  uint8 ll_vsi_pointer; 

  /* for jericho: inner vlan */
  SOC_SAND_PP_VLAN_ID inner_vid;

  /* for jericho: inner pcp dei */
  uint8 inner_pcp_dei;

  /* for jericho: pcp-dei-profile (2b) */
  uint8 pcp_dei_profile; 

  /* for jericho: remark_profile (3b) */
  uint8 remark_profile;

  /* for jericho: index for additional table: cfgEtherTypeIndex */
  int eth_type_index; 

  /* for jericho: nof_tags. Indicate if EVE is required */
  int nof_tags; 

}SOC_PPC_EG_ENCAP_OVERLAY_ARP_ENCAP_INFO;

/* used by ROO Link Layer Format:
   additional table for eedb of type roo link layer format */
typedef struct 
{
  uint16 ether_type; 
  /* outer tpid */
  uint16 tpid_0;
  /* inner tpid */ 
  uint16 tpid_1; 
}SOC_PPC_EG_ENCAP_ETHER_TYPE_INDEX_INFO; 


typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  IPv6 destination address.
   */
  SOC_SAND_PP_IPV6_ADDRESS dest;
  /*
   *  IPv6 source address.
   */
  SOC_SAND_PP_IPV6_ADDRESS src;
  /*
   *  Hop limit
   *  soc_ppd_eg_encap_ipv4_tunnel_glbl_src_ip_set. Range: 0 - 255.
   */
  uint8 hop_limit;
  /* 
   * flow label 
   */
  uint16 flow_label;
  /* 
   * next header
   */
  uint8 next_header;

} SOC_PPC_EG_ENCAP_IPV6_TUNNEL_INFO;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  IPV6 destination address.
   */
   SOC_PPC_EG_ENCAP_IPV6_TUNNEL_INFO tunnel;

} SOC_PPC_EG_ENCAP_IPV6_ENCAP_INFO;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  DA MAC address.
   */
  SOC_SAND_PP_MAC_ADDRESS dest;
  /*
   *  SA MAC address.
   */
  SOC_SAND_PP_MAC_ADDRESS src;
  /*
   *  VLAN tag tpid.
   */
  uint16 tpid;
  /*
   *  VLAN tag tpid.
   */
  uint16 vid;
  /*
   *  VLAN tag pcp.
   */
  uint8 pcp;
  /*
   *  L2 EtherType
   */
  uint16 ether_type;
  /*
   *  erspan_id (10 bits) for ERSPAN hdr
   */
  uint16 erspan_id;
  /* 
   *  encap id (4 bits) for IP Tunnel
   */ 
  uint16 encap_id;

} SOC_PPC_EG_ENCAP_MIRROR_TUNNEL_INFO;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  MIRROR destination address.
   */
   SOC_PPC_EG_ENCAP_MIRROR_TUNNEL_INFO tunnel;

} SOC_PPC_EG_ENCAP_MIRROR_ENCAP_INFO;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  progrm Data information.
   */
  uint32 data_entry[SOC_PPC_EG_ENCAP_DATA_PROG_ENTRY_MAX_SIZE];

} SOC_PPC_EG_ENCAP_DATA_PROG_ENTRY_INFO;

/* ARAD only typedefs } */

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  Push profile used to construct the label's TTL and EXP.
   *  Range: 0 - 7. This profile is selected by the push
   *  command, either by encapsulation
   *  soc_ppd_eg_encap_pwe_entry_add() or by forwarding - VPLS see
   *  soc_ppd_frwrd_mact_entry_add when EEI is used as MPLS push
   *  command.- LSR see soc_ppd_frwrd_ilm_add. In T20E, the EXP
   *  value may be mapped from the TC and DP values for each
   *  push_profile. See soc_ppd_eg_encap_push_exp_info_set().
   */
  uint32 push_profile;
  /*
   *  Traffic Class value as calculated by the ingress. See
   *  COS module. Relevant ONLY for T20E; for Soc_petra-B has to
   *  be 0. Range: 0 - 7.
   */
  SOC_SAND_PP_TC tc;
  /*
   *  Drop Precedence value as calculated by the ingress. See
   *  COS module. Relevant ONLY for T20E; for Soc_petra-B has to
   *  be 0. Range: 0 - 3.
   */
  SOC_SAND_PP_DP dp;

} SOC_PPC_EG_ENCAP_PUSH_EXP_KEY;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  Control Word value-to set to PWEs that are configured to
   *  have CW.
   */
  uint32 cw;

} SOC_PPC_EG_ENCAP_PWE_GLBL_INFO;


typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  Ethernet-Type in GRE header for unknown forward code
   */
  uint16 l2_gre_prtcl_type;
  /*
   *  Ethernet-Type in GRE header for unknown forward code
   */
  uint16 unkown_gre_prtcl_type;
  /*
   *  protocol type in IP header for unknown forward code
   */
  uint16 unkown_ip_next_prtcl_type;
  /*
   *  value for EthoIP  Shim 
   */
  uint16 eth_ip_val;
  /*
   *  Ethertype for unknown forward code.(EPNI_ETHERTYPE_DEFAULT,ETHERTYPE_DEFAULT)
   */
  uint16 unkown_ll_ether_type;
  
  /*
   *  Vxlan udp_dest_port
   */
  uint16 vxlan_udp_dest_port;
  /* 
   *  Set Entry label 12 MSBs
   *  Valid for ARAD and above
   */
  uint16 mpls_entry_label_msbs;
  
} SOC_PPC_EG_ENCAP_GLBL_INFO;


/* describe trill eedb entry */
typedef struct 
{
    SOC_SAND_MAGIC_NUM_VAR

    /*
     * my nickname index. 
     */
    uint8 my_nickname_index; 
    /*
     * multicast indication.* 
     */
    uint8 m; 
    /*
     * egress nickname* 
     */
    uint16 nick; 
    /*
     * outlif profile 
     * In QAX, used as is_l2_lif for ROO application
    */
    uint32 outlif_profile;
    /*
    * linklayer eep index
    */
    uint32                       ll_eep_ndx;
    /*
    if link layer eep is valid
    */
    uint8                        ll_eep_is_valid;
    /*
     *  Select 1 of 16 Remark profiles. See
     *  bcm_qos_port_map_set. Range: 0-15.
     *  Invalid for Soc_petra-B.
     */
    uint8 remark_profile;

} SOC_PPC_EG_ENCAP_TRILL_INFO; 


typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  VSI
   */
  SOC_PPC_VSI_ID vsi;
  /*
   *  Information for out AC
   */
  SOC_PPC_EG_AC_INFO out_ac;
  /*
   *  information for swap command
   */
  SOC_PPC_EG_ENCAP_SWAP_INFO swap_info;
  /*
   *  information for PWE (VC push)
   */
  SOC_PPC_EG_ENCAP_PWE_INFO pwe_info;
  /*
   *  information for PP command
   */
  SOC_PPC_EG_ENCAP_POP_INFO pop_info;
  /*
   *  information to encapsulate MPLS tunnels
   */
  SOC_PPC_EG_ENCAP_MPLS_ENCAP_INFO mpls_encap_info;
  /*
   *  information to encapsulate IPV4 tunnels
   */
  SOC_PPC_EG_ENCAP_IPV4_ENCAP_INFO ipv4_encap_info;
  /*
   *  information to encapsulate IPV6 tunnels
   */
  SOC_PPC_EG_ENCAP_IPV6_ENCAP_INFO ipv6_encap_info;
  /*
   *  information to encapsulate Mirror tunnels
   */
  SOC_PPC_EG_ENCAP_MIRROR_ENCAP_INFO mirror_encap_info;
  /*
   *  information to encapsulate LL
   */
  SOC_PPC_EG_ENCAP_LL_INFO ll_info;
  /*
   *  information to encapsulate RIF 
   */
  SOC_PPC_EG_ENCAP_VSI_ENCAP_INFO vsi_info;
  /* 
   *  information to encapsulate DATA
   *  valid for ARAD only.
   */
  SOC_PPC_EG_ENCAP_DATA_INFO data_info;
  /* 
   * information to encapsulate overlay arp.
   * Valid from ARAD+ 
   */ 
  SOC_PPC_EG_ENCAP_OVERLAY_ARP_ENCAP_INFO overlay_arp_encap_info;
  /*
   * information to encapsulate trill
   * Valid from ARAD+. 
   */
  SOC_PPC_EG_ENCAP_TRILL_INFO trill_encap_info; 
} SOC_PPC_EG_ENCAP_ENTRY_VALUE;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  Type of the encapsulation entry
   */
  SOC_PPC_EG_ENCAP_ENTRY_TYPE entry_type;
  /*
   *  Value of the encapsulation entry, according to type
   */
  SOC_PPC_EG_ENCAP_ENTRY_VALUE entry_val;

} SOC_PPC_EG_ENCAP_ENTRY_INFO;


typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /* 
   * Value of internal priority 
   */
  uint32 int_pri;   
  /* 
   * Validity of internal priority 
   */
  uint32 int_pri_valid;   
  /* 
   * Value of color     
   */      
  bcm_color_t color;         
  /* 
   * Validity of color     
   */      
  uint32 color_valid;  
} SOC_PPC_EG_MAP_ENCAP_INTPRI_COLOR_INFO;


typedef struct 
{
    /*
     * ip tunnel template: ip tunnel type.
     * template values match encapsulation mode
     */ 
    SOC_PPC_EG_ENCAP_ENCAPSULATION_MODE ip_tunnel_template; 
    /*
     * encapsulation size: size of the IP tunnel 
     */
    uint32 encapsulation_size; 
    /*
     * IP next protocol (1B) 
     */
    uint8 protocol; 
    /* IP next protocol enable: use field protocol to set IP.protocol
       If disabled, use hardcoded logic */
    uint8 protocol_enable; 
} SOC_PPC_EG_ENCAP_IP_TUNNEL_SIZE_PROTOCOL_TEMPLATE_INFO; 

/* Holds information for reserved label profile in QAX and above. */
typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  TTL to set only if ttl_model == 0x1 (pipe)
   *  Range: 0 - 255.
   */
  SOC_SAND_PP_IP_TTL ttl;
  /*
   *  EXP to set only if exp_model == 0x1 (pipe)
   *  Range: 0 - 7.
   */
  SOC_SAND_PP_MPLS_EXP exp;
  /* From QAX: indicates the ttl model, whether set according to this table (pipe model -> value == 0x1)
     or copy from first label in stack. */
  SOC_SAND_PP_MPLS_TUNNEL_MODEL ttl_model;
  /* From QAX: indicates the exp model, whether set according to to this table (pipe model -> value == 0x1)
     or copy from first label in stack */
  SOC_SAND_PP_MPLS_TUNNEL_MODEL exp_model;
  /* Indicate a label type in the range 0-3:
     0: Given reserved mpls label
     1: Entropy label
     2: Forwarding code: Label will be set according to Label-Per-Code[Forwarding code]
     3: Data entry: label is taken from stack entry below mpls entry */
  uint8 label_type;
  /* Used only for label_type == 0x0. */
  bcm_mpls_label_t label;
} SOC_PPC_EG_ENCAP_ADDITIONAL_LABEL_PROFILE_INFO;



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
  SOC_PPC_EG_ENCAP_RANGE_INFO_clear(
    SOC_SAND_OUT SOC_PPC_EG_ENCAP_RANGE_INFO *info
  );

void
  SOC_PPC_EG_ENCAP_PROTECTION_INFO_clear(
    SOC_SAND_OUT SOC_PPC_EG_ENCAP_PROTECTION_INFO   *info
  );

void
  SOC_PPC_EG_ENCAP_SWAP_INFO_clear(
    SOC_SAND_OUT SOC_PPC_EG_ENCAP_SWAP_INFO *info
  );

void
  SOC_PPC_EG_ENCAP_PWE_INFO_clear(
    SOC_SAND_OUT SOC_PPC_EG_ENCAP_PWE_INFO *info
  );

void
  SOC_PPC_EG_ENCAP_POP_INTO_ETH_INFO_clear(
    SOC_SAND_OUT SOC_PPC_EG_ENCAP_POP_INTO_ETH_INFO *info
  );

void
  SOC_PPC_EG_ENCAP_POP_INFO_clear(
    SOC_SAND_OUT SOC_PPC_EG_ENCAP_POP_INFO *info
  );

void
  SOC_PPC_EG_ENCAP_MPLS_TUNNEL_INFO_clear(
    SOC_SAND_OUT SOC_PPC_EG_ENCAP_MPLS_TUNNEL_INFO *info
  );

void
  SOC_PPC_EG_ENCAP_MPLS_ENCAP_INFO_clear(
    SOC_SAND_OUT SOC_PPC_EG_ENCAP_MPLS_ENCAP_INFO *info
  );

void
  SOC_PPC_EG_ENCAP_IPV4_TUNNEL_INFO_clear(
    SOC_SAND_OUT SOC_PPC_EG_ENCAP_IPV4_TUNNEL_INFO *info
  );

void
  SOC_PPC_EG_ENCAP_IPV4_ENCAP_INFO_clear(
    SOC_SAND_OUT SOC_PPC_EG_ENCAP_IPV4_ENCAP_INFO *info
  );

void
  SOC_PPC_EG_ENCAP_IPV6_TUNNEL_INFO_clear(
    SOC_SAND_OUT SOC_PPC_EG_ENCAP_IPV6_TUNNEL_INFO *info
  );

void
  SOC_PPC_EG_ENCAP_IPV6_ENCAP_INFO_clear(
    SOC_SAND_OUT SOC_PPC_EG_ENCAP_IPV6_ENCAP_INFO *info
  );

void
  SOC_PPC_EG_ENCAP_MIRROR_TUNNEL_INFO_clear(
    SOC_SAND_OUT SOC_PPC_EG_ENCAP_MIRROR_TUNNEL_INFO *info
  );

void
  SOC_PPC_EG_ENCAP_MIRROR_ENCAP_INFO_clear(
    SOC_SAND_OUT SOC_PPC_EG_ENCAP_MIRROR_ENCAP_INFO *info
  );

void
  SOC_PPC_EG_ENCAP_LL_INFO_clear(
    SOC_SAND_OUT SOC_PPC_EG_ENCAP_LL_INFO *info
  );

void
  SOC_PPC_EG_ENCAP_ENTRY_VALUE_clear(
    SOC_SAND_OUT SOC_PPC_EG_ENCAP_ENTRY_VALUE *info
  );

void
  SOC_PPC_EG_ENCAP_PUSH_PROFILE_INFO_clear(
    SOC_SAND_OUT SOC_PPC_EG_ENCAP_PUSH_PROFILE_INFO *info
  );

void
  SOC_PPC_EG_ENCAP_ADDITIONAL_LABEL_PROFILE_INFO_clear(
    SOC_PPC_EG_ENCAP_ADDITIONAL_LABEL_PROFILE_INFO *info
  );

void 
  SOC_PPC_EG_ENCAP_VSI_ENCAP_INFO_clear(
    SOC_SAND_OUT SOC_PPC_EG_ENCAP_VSI_ENCAP_INFO *info
  );

void 
  SOC_PPC_EG_ENCAP_DATA_INFO_clear(
    SOC_SAND_OUT SOC_PPC_EG_ENCAP_DATA_INFO *info
  );

void 
  SOC_PPC_EG_ENCAP_DATA_PROG_ENTRY_INFO_clear(
    SOC_SAND_OUT SOC_PPC_EG_ENCAP_DATA_PROG_ENTRY_INFO *info
  );

void
  SOC_PPC_EG_ENCAP_PUSH_EXP_KEY_clear(
    SOC_SAND_OUT SOC_PPC_EG_ENCAP_PUSH_EXP_KEY *info
  );

void
  SOC_PPC_EG_ENCAP_PWE_GLBL_INFO_clear(
    SOC_SAND_OUT SOC_PPC_EG_ENCAP_PWE_GLBL_INFO *info
  );

void
  SOC_PPC_EG_ENCAP_GLBL_INFO_clear(
    SOC_SAND_OUT SOC_PPC_EG_ENCAP_GLBL_INFO *info
  );

void
  SOC_PPC_EG_ENCAP_ENTRY_INFO_clear(
    SOC_SAND_OUT SOC_PPC_EG_ENCAP_ENTRY_INFO *info
  );

void
  SOC_PPC_EG_ENCAP_OVERLAY_ARP_ENCAP_INFO_clear(
    SOC_SAND_OUT SOC_PPC_EG_ENCAP_OVERLAY_ARP_ENCAP_INFO *ll_encap_info
  );

void 
  SOC_PPC_EG_ENCAP_TRILL_INFO_clear(
     SOC_SAND_OUT SOC_PPC_EG_ENCAP_TRILL_INFO *info
  ); 

void 
SOC_PPC_EG_ENCAP_IP_TUNNEL_SIZE_PROTOCOL_TEMPLATE_INFO_clear(
   SOC_PPC_EG_ENCAP_IP_TUNNEL_SIZE_PROTOCOL_TEMPLATE_INFO *info
   ); 

#if SOC_PPC_DEBUG_IS_LVL1

const char*
  SOC_PPC_EG_ENCAP_EEP_TYPE_to_string(
    SOC_SAND_IN  SOC_PPC_EG_ENCAP_EEP_TYPE enum_val
  );

const char*
  SOC_PPC_EG_ENCAP_ACCESS_PHASE_to_string(
    SOC_SAND_IN SOC_PPC_EG_ENCAP_ACCESS_PHASE enum_val
  );

const char*
  SOC_PPC_EG_ENCAP_ENCAPSULATION_MODE_to_string(
    SOC_SAND_IN  SOC_PPC_EG_ENCAP_ENCAPSULATION_MODE enum_val
  );

const char*
  SOC_PPC_EG_ENCAP_EXP_MARK_MODE_to_string(
    SOC_SAND_IN  SOC_PPC_EG_ENCAP_EXP_MARK_MODE enum_val
  );

const char*
  SOC_PPC_EG_ENCAP_ENTRY_TYPE_to_string(
    SOC_SAND_IN  SOC_PPC_EG_ENCAP_ENTRY_TYPE enum_val
  );

void
  SOC_PPC_EG_ENCAP_RANGE_INFO_print(
    SOC_SAND_IN  SOC_PPC_EG_ENCAP_RANGE_INFO *info
  );

void
  SOC_PPC_EG_ENCAP_PROTECTION_INFO_print(
    SOC_SAND_IN  SOC_PPC_EG_ENCAP_PROTECTION_INFO *info
  );

void
  SOC_PPC_EG_ENCAP_SWAP_INFO_print(
    SOC_SAND_IN  SOC_PPC_EG_ENCAP_SWAP_INFO *info
  );

void
  SOC_PPC_EG_ENCAP_VSI_ENCAP_INFO_print(
    SOC_SAND_IN  SOC_PPC_EG_ENCAP_VSI_ENCAP_INFO *info
  );

void
  SOC_PPC_EG_ENCAP_DATA_INFO_print(
    SOC_SAND_IN  SOC_PPC_EG_ENCAP_DATA_INFO *info
  );

void
  SOC_PPC_EG_ENCAP_DATA_PROG_ENTRY_INFO_print(
    SOC_SAND_IN  SOC_PPC_EG_ENCAP_DATA_PROG_ENTRY_INFO *info
  );

void
  SOC_PPC_EG_ENCAP_PWE_INFO_print(
    SOC_SAND_IN  SOC_PPC_EG_ENCAP_PWE_INFO *info
  );

void
  SOC_PPC_EG_ENCAP_POP_INTO_ETH_INFO_print(
    SOC_SAND_IN  SOC_PPC_EG_ENCAP_POP_INTO_ETH_INFO *info
  );

void
  SOC_PPC_EG_ENCAP_POP_INFO_print(
    SOC_SAND_IN  SOC_PPC_EG_ENCAP_POP_INFO *info
  );

void
  SOC_PPC_EG_ENCAP_MPLS_TUNNEL_INFO_print(
    SOC_SAND_IN  SOC_PPC_EG_ENCAP_MPLS_TUNNEL_INFO *info
  );

void
  SOC_PPC_EG_ENCAP_MPLS_ENCAP_INFO_print(
    SOC_SAND_IN  SOC_PPC_EG_ENCAP_MPLS_ENCAP_INFO *info
  );

void
  SOC_PPC_EG_ENCAP_IPV4_TUNNEL_INFO_print(
    SOC_SAND_IN  SOC_PPC_EG_ENCAP_IPV4_TUNNEL_INFO *info
  );

void
  SOC_PPC_EG_ENCAP_IPV4_ENCAP_INFO_print(
    SOC_SAND_IN  SOC_PPC_EG_ENCAP_IPV4_ENCAP_INFO *info
  );

void
  SOC_PPC_EG_ENCAP_IPV6_TUNNEL_INFO_print(
    SOC_SAND_IN  SOC_PPC_EG_ENCAP_IPV6_TUNNEL_INFO *info
  );

void
  SOC_PPC_EG_ENCAP_IPV6_ENCAP_INFO_print(
    SOC_SAND_IN  SOC_PPC_EG_ENCAP_IPV6_ENCAP_INFO *info
  );

void
  SOC_PPC_EG_ENCAP_MIRROR_TUNNEL_INFO_print(
    SOC_SAND_IN  SOC_PPC_EG_ENCAP_MIRROR_TUNNEL_INFO *info
  );

void
  SOC_PPC_EG_ENCAP_MIRROR_ENCAP_INFO_print(
    SOC_SAND_IN  SOC_PPC_EG_ENCAP_MIRROR_ENCAP_INFO *info
  );

void
  SOC_PPC_EG_ENCAP_LL_INFO_print(
    SOC_SAND_IN  int                      unit,
    SOC_SAND_IN  SOC_PPC_EG_ENCAP_LL_INFO *info
  );

void
  SOC_PPC_EG_ENCAP_PUSH_PROFILE_INFO_print(
    SOC_SAND_IN  SOC_PPC_EG_ENCAP_PUSH_PROFILE_INFO *info
  );

void
  SOC_PPC_EG_ENCAP_ADDITIONAL_LABEL_PROFILE_INFO_print(
    SOC_SAND_IN  SOC_PPC_EG_ENCAP_ADDITIONAL_LABEL_PROFILE_INFO *info
  );

void
  SOC_PPC_EG_ENCAP_PUSH_EXP_KEY_print(
    SOC_SAND_IN  SOC_PPC_EG_ENCAP_PUSH_EXP_KEY *info
  );

void
  SOC_PPC_EG_ENCAP_PWE_GLBL_INFO_print(
    SOC_SAND_IN  SOC_PPC_EG_ENCAP_PWE_GLBL_INFO *info
  );

void
  SOC_PPC_EG_ENCAP_GLBL_INFO_print(
    SOC_SAND_IN  SOC_PPC_EG_ENCAP_GLBL_INFO *info
  );

void
  SOC_PPC_EG_ENCAP_ENTRY_VALUE_print(
    SOC_SAND_IN  int                         unit,
    SOC_SAND_IN  SOC_PPC_EG_ENCAP_ENTRY_VALUE *info
  );

void
  SOC_PPC_EG_ENCAP_ENTRY_INFO_print(
    SOC_SAND_IN  int                         unit,
    SOC_SAND_IN  SOC_PPC_EG_ENCAP_ENTRY_INFO *info
  );

void
  SOC_PPC_EG_ENCAP_TRILL_INFO_print(
    SOC_SAND_IN  SOC_PPC_EG_ENCAP_TRILL_INFO *info
  );


void
  SOC_PPC_EG_VLAN_EDIT_VLAN_INFO_print(
    SOC_SAND_IN  SOC_PPC_EG_VLAN_EDIT_VLAN_INFO *info
  );

void
  SOC_PPC_EG_AC_VLAN_EDIT_INFO_print(
    SOC_SAND_IN  SOC_PPC_EG_AC_VLAN_EDIT_INFO *info
  );

void
  SOC_PPC_EG_AC_INFO_print(
    SOC_SAND_IN  SOC_PPC_EG_AC_INFO *info
  );


#endif /* SOC_PPC_DEBUG_IS_LVL1 */

void
  SOC_PPC_EG_MAP_ENCAP_INTPRI_COLOR_INFO_clear(
    SOC_SAND_OUT SOC_PPC_EG_MAP_ENCAP_INTPRI_COLOR_INFO *info
  );
/* } */

#include <soc/dpp/SAND/Utils/sand_footer.h>

/* } __SOC_PPC_API_EG_ENCAP_INCLUDED__*/
#endif
