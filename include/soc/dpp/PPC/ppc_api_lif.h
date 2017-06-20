/* $Id: ppc_api_lif.h,v 1.28 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/
/******************************************************************
*
* FILENAME:       DuneDriver/ppc/include/soc_ppc_api_lif.h
*
* MODULE PREFIX:  soc_ppc_api_lif
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

#ifndef __SOC_PPC_API_LIF_INCLUDED__
/* { */
#define __SOC_PPC_API_LIF_INCLUDED__

/*************
 * INCLUDES  *
 *************/
/* { */

#include <soc/dpp/SAND/Utils/sand_header.h>

#include <soc/dpp/SAND/Management/sand_general_macros.h>
#include <soc/dpp/SAND/Management/sand_error_code.h>

#include <soc/dpp/PPC/ppc_api_general.h>
#include <soc/dpp/PPC/ppc_api_llp_parse.h>

#include <soc/dpp/ARAD/ARAD_PP/arad_pp_occupation_mgmt.h>
/* } */
/*************
 * DEFINES   *
 *************/
/* { */

/*     Used when the AC mapping is only according to the Port
 *     and outer VID, or according to the Port only.           */
#define  SOC_PPC_LIF_IGNORE_INNER_VID (0xFFFFFFFF)

/*     Used when the AC mapping is only according to the port
 *     (port default).                                         */
#define  SOC_PPC_LIF_IGNORE_OUTER_VID (0xFFFFFFFF)

/*     Used when the AC mapping is only according to the port
 *     and outer VID and innver VID                                  */
#define  SOC_PPC_LIF_IGNORE_TUNNEL_VID (0xFFFFFFFF)

/*     Used when the AC mapping is only according to the port
 *     and outer VID and innver VID                                  */
#define  SOC_PPC_LIF_IGNORE_ETHER_TYPE (0xFFFF)

/*     Used when the AC mapping ignores Outer-PCP                   */
#define  SOC_PPC_LIF_IGNORE_OUTER_PCP (0xFF)

/*     LIF VLAN range maximal size.                                  */
#define  SOC_PPC_LIF_VLAN_RANGE_MAX_SIZE (32)

/*     AC Group maximal size.                                  */
#define  SOC_PPC_LIF_AC_GROUP_MAX_SIZE (8)

/*     Indicates that the I-SID to VSI mapping is only
 *     according to I-SID, as opposed to I-SID x I-SID Domain
 *     ID mapping to VSI                                       */
#define  SOC_PPC_ISID_DM_DISABLE          (0xFFFF)

/* Invalid global lif */
#define SOC_PPC_INVALID_GLOBAL_LIF      (0x3ffff)

/*     VSI MAX Number of supported services */
#define  SOC_PPC_VSI_MAX_NOF_ARAD           (32768)

/*     VSI that should be used for P2P service                  */
#define  SOC_PPC_VSI_P2P_SERVICE            (16 * 1024 - 2)
#define  SOC_PPC_VSI_P2P_SERVICE_ARAD       (SOC_PPC_VSI_MAX_NOF_ARAD - 2)

/*     VSI Equal to initial VID                                 */
#define  SOC_PPC_VSI_EQ_IN_VID              (SOC_PPC_VSI_MAX_NOF_ARAD - 1)


/* VSI assignment mode */

/* VSI equal to VSI in LIF table */
#define  SOC_PPC_VSI_EQ_VSI_BASE          (0x0)
/* VSI equal to LIF-index */
#define  SOC_PPC_VSI_EQ_IN_LIF          (0x3)
/* VSI equal to VSI in LIF table + VID */
#define  SOC_PPC_VSI_EQ_IN_VID_PLUS_VSI_BASE (0x1)
/* VSI equal to VLAN */


/*     VC label indexes */
#define  SOC_PPC_VC_LABEL_INDEXED_LABEL_INDEX_SHIFT                      20
#define  SOC_PPC_VC_LABEL_INDEXED_LABEL_INDEX_MASK                       0x3
#define  SOC_PPC_VC_LABEL_INDEXED_LABEL_VALUE_SHIFT                      0
#define  SOC_PPC_VC_LABEL_INDEXED_LABEL_VALUE_MASK                       0xFFFFF

/* Enable/Disable l3 source bind function */
#define  SOC_PPC_LIF_AC_SPOOF_MODE_ENABLE  (1)
#define  SOC_PPC_FLP_COS_PROFILE_ANTI_SPOOFING       (0x1 << 5)
#define  SOC_PPC_LIF_AC_SPOOF_MODE_ENABLE_SET                   (SOC_PPC_LIF_AC_SPOOF_MODE_ENABLE << 5)
#define  SOC_PPC_LIF_AC_SPOOF_MODE_DISABLE_SET                  (~SOC_PPC_LIF_AC_SPOOF_MODE_ENABLE_SET)

/* Enable/Disable vmac function in AC */
#define SOC_PPC_LIF_AC_VAMC_COS_PROFILE_DISABLE         0
#define SOC_PPC_LIF_AC_VAMC_COS_PROFILE_ENABLE          1
#define SOC_PPC_LIF_AC_VAMC_COS_PROFILE_LSB             5
#define SOC_PPC_LIF_AC_VAMC_COS_PROFILE_MSB             5
#define SOC_PPC_LIF_AC_VAMC_COS_PROFILE_LSB_NOF_BITS    (SOC_PPC_LIF_AC_VAMC_COS_PROFILE_MSB - \
                                                         SOC_PPC_LIF_AC_VAMC_COS_PROFILE_LSB + 1)
#define SOC_PPC_FLP_COS_PROFILE_VMAC                    (0x1 << 5)

/* Enable/Disable local switching function */
#define  SOC_PPC_LIF_AC_LOCAL_SWITCHING_COS_PROFILE  (0x1 << 5)

/* Default null entry outlif id */
#define SOC_PPC_LIF_NULL_LOCAL_OUTLIF_ID (0)



/* } */
/*************
 * MACROS    *
 *************/
/* { */
/*     VC label indexes Macros */
#define SOC_PPC_VC_LABEL_INDEXED_SET(label, vc_label_value, vc_label_index)               \
          ((label) = (((vc_label_index) & SOC_PPC_VC_LABEL_INDEXED_LABEL_INDEX_MASK)  << SOC_PPC_VC_LABEL_INDEXED_LABEL_INDEX_SHIFT)  | \
        (((vc_label_value) & SOC_PPC_VC_LABEL_INDEXED_LABEL_VALUE_MASK) << SOC_PPC_VC_LABEL_INDEXED_LABEL_VALUE_SHIFT))

#define SOC_PPC_VC_LABEL_INDEXED_LABEL_VALUE_GET(label)   \
        (((label) >> SOC_PPC_VC_LABEL_INDEXED_LABEL_VALUE_SHIFT) & SOC_PPC_VC_LABEL_INDEXED_LABEL_VALUE_MASK)

#define SOC_PPC_VC_LABEL_INDEXED_LABEL_INDEX_GET(label)   \
        (((label) >> SOC_PPC_VC_LABEL_INDEXED_LABEL_INDEX_SHIFT) & SOC_PPC_VC_LABEL_INDEXED_LABEL_INDEX_MASK)
/* } */
/*************
 * TYPE DEFS *
 *************/
/* { */

typedef enum
{
  /*
   *  The AC is mapped from the port number only. Outer and
   *  inner tags are ignored in the mapping. Use
   *  SOC_PPC_LIF_IGNORE_OUTER_VID and SOC_PPC_LIF_IGNORE_INNER_VID
   *  for the VIDs in the AC key when adding AC for the port.
   */
  SOC_PPC_L2_LIF_AC_MAP_KEY_TYPE_PORT = 0,
  /*
   *  The AC is mapped according to Port x Vlan. Inner tag is
   *  ignored in the mapping. SOC_PPC_LIF_IGNORE_INNER_VID for
   *  the inner VID in the AC key when adding AC.
   */
  SOC_PPC_L2_LIF_AC_MAP_KEY_TYPE_PORT_VLAN = 1,
  /*
   *  The AC is mapped according to Port x Vlan x Vlan.
   *  In Soc_petra-B, Outer-VLAN is actually the initial-VID results from VID
   *  assignment.
   *  In ARAD, Outer-VLAN is the VLAN resolved from the packet.
   */
  SOC_PPC_L2_LIF_AC_MAP_KEY_TYPE_PORT_VLAN_VLAN = 2,
  /*
   *  The AC is mapped according to raw value calculated based
   *  on packet attributes. In this key only the 'raw_key'
   *  field is considered from the key.
   */
  SOC_PPC_L2_LIF_AC_MAP_KEY_TYPE_RAW = 3,
  /*
   *  The AC is mapped according to raw value x Vlan calculated based
   *  on packet attributes. In this key, the 'raw_key' and the outer tag
   *  fields are considered from the key.
   */
  SOC_PPC_L2_LIF_AC_MAP_KEY_TYPE_RAW_VLAN = 4,
  /*
   *  The AC is mapped according to raw value x Vlan x Vlan calculated based
   *  on packet attributes. In this key, the 'raw_key', the outer tag and
   *  the inner tag fields are considered from the key.
   */
  SOC_PPC_L2_LIF_AC_MAP_KEY_TYPE_RAW_VLAN_VLAN = 5,
  /*
   *  The AC is mapped according to Port x compressed Vlan. Inner tag is
   *  ignored in the mapping. SOC_PPC_LIF_IGNORE_INNER_VID for
   *  the inner VID in the AC key when adding AC.
   *  see soc_ppd_l2_lif_vlan_compression_add
   */
  SOC_PPC_L2_LIF_AC_MAP_KEY_TYPE_PORT_COMP_VLAN = 6,
  /*
   *  The AC is mapped according to Port x compressed Vlan x compressed Vlan.
   *  In Soc_petra-B, Outer-VLAN is actually the initial-VID results from VID
   *  assignment.
   *  In ARAD, Outer-VLAN is the VLAN resolved from the packet.
   *  see soc_ppd_l2_lif_vlan_compression_add
   */
  SOC_PPC_L2_LIF_AC_MAP_KEY_TYPE_PORT_COMP_VLAN_COMP_VLAN = 7,
  /* 
   * The AC is mapped according to default settings. 
   * Default settings are being set according to global variables. 
   * In case of simple vlan translation: depends on number of vlan tags. 
   * In case of non simple vlan translation: AC might be mapped to 
   * port x vlan or port x vlan x vlan. 
   * Invalid for Soc_petra-B. 
   */ 
  SOC_PPC_L2_LIF_AC_MAP_KEY_TYPE_DEFAULT = 8,
  /*
   *  Number of types in SOC_PPC_L2_LIF_AC_MAP_KEY_TYPE
   */
  SOC_PPC_NOF_L2_LIF_AC_MAP_KEY_TYPES_PB = 8,
  /* 
   *  The AC is mapped according to Port x Initial-Vlan. Inner tag is
   *  ignored in the mapping. SOC_PPC_LIF_IGNORE_INNER_VID for
   *  the inner VID in the AC key when adding AC.
   *  Initial-VLAN is mainly used for untagged packet and when SA based is set (per port).
   *  Invalid for Soc_petra-B.
   */
  SOC_PPC_L2_LIF_AC_MAP_KEY_TYPE_PORT_INITIAL_VLAN = 9,
  /*
   *  The AC is mapped according to Port x PCP x Vlan . Inner tag is
   *  ignored in the mapping. SOC_PPC_LIF_IGNORE_INNER_VID for
   *  the inner VID in the AC key when adding AC.
   *  Only avaiable when PCP lookup global property is enabled.
   *  In this case also outer_pcp is used.
   *  Invalid for Soc_petra-B.
   */
  SOC_PPC_L2_LIF_AC_MAP_KEY_TYPE_PORT_PCP_VLAN = 10,
  /*
   *  The AC is mapped according to Port x Outer-PCP x Vlan x Vlan.
   *  Only aviable when PCP lookup global property is enabled                         .
   *  In this case also outer_pcp is used.                                                                                .
   *  Invalid for Soc_petra-B.                                                            .
   */
  SOC_PPC_L2_LIF_AC_MAP_KEY_TYPE_PORT_PCP_VLAN_VLAN = 11,

  /*
   *  The AC is mapped according to Port x Tunnel_ID. Outer and 
   *  Inner tag is ignored in the mapping.  Use
   *  SOC_PPC_LIF_IGNORE_OUTER_VID and SOC_PPC_LIF_IGNORE_INNER_VID
   *  for the VIDs in the AC key when adding AC for the port.
   *  Only aviable when port is PON-port.
   *  In this case also Tunnel_ID is used.
   *  Invalid for Soc_petra-B.
   */
  SOC_PPC_L2_LIF_AC_MAP_KEY_TYPE_PORT_TUNNEL = 12,

  /*
   *  The AC is mapped according to Port x Tunnel_ID x Vlan. Inner tag is
   *  ignored in the mapping. SOC_PPC_LIF_IGNORE_INNER_VID for
   *  the inner VID in the AC key when adding AC.
   *  Only aviable when port is PON-port.
   *  In this case also Tunnel_ID is used.
   *  Invalid for Soc_petra-B.
   */
  SOC_PPC_L2_LIF_AC_MAP_KEY_TYPE_PORT_TUNNEL_COMP_VLAN = 13,

  /*
   *  The AC is mapped according to Port x Tunnel_ID x Vlan x Vlan. 
   *  Only aviable when port is PON-port.
   *  In this case also Tunnel_ID is used.
   *  Invalid for Soc_petra-B.
   */
  SOC_PPC_L2_LIF_AC_MAP_KEY_TYPE_PORT_TUNNEL_COMP_VLAN_COMP_VLAN = 14,

  /*
   *  The AC is mapped according to  Port x Tunnel_ID x PCP x EtherType x Vlan x Vlan. Inner tag is
   *  ignored in the mapping.
   *  Only aviable when port is PON-port.
   *  In this case also Tunnel_ID, outer_pcp, ether_type is used.
   *  Invalid for Soc_petra-B.
   */
  SOC_PPC_L2_LIF_AC_MAP_KEY_TYPE_PORT_TUNNEL_PCP_ETHER_TYPE_COMP_VLAN_COMP_VLAN = 15,

  /*
   *  The AC is mapped according to Port x Tunnel_ID. Outer tag and Inner tag is
   *  ignored in the mapping.
   *  TLS-PORT-TUNNEL is mainly used for transparent packets per PON port.
   *  Only aviable when port is PON-port.
   *  Invalid for Soc_petra-B.
   */
  SOC_PPC_L2_LIF_AC_MAP_KEY_TYPE_TLS_PORT_TUNNEL = 16,

  /*
   * The AC is not mapped at all.
  */

  SOC_PPC_L2_LIF_AC_MAP_KEY_TYPE_NONE = 17,

  /* 
   *  The AC is mapped according to Port x Designated-Vlan. Inner tag is
   *  ignored in the mapping. SOC_PPC_LIF_IGNORE_INNER_VID for
   *  the inner VID in the AC key when adding AC.
   *  Designated -VLAN is mainly used for TRILL UC and MC egress RBridges (per port). 
   *  Invalid for Soc_petra-B.
   */
  SOC_PPC_L2_LIF_AC_MAP_KEY_TYPE_TRILL_DESIGNATED_VLAN = 18,
  /* 
   * The AC is mapped according to Flexible QinQ port single tag
   */ 
  SOC_PPC_L2_LIF_AC_MAP_KEY_TYPE_FLEXIBLE_Q_IN_Q_PORT_INITIAL_VID = 19,

  /* 
   * The AC is mapped according to Flexible QinQ port double tag
   */ 
  SOC_PPC_L2_LIF_AC_MAP_KEY_TYPE_FLEXIBLE_Q_IN_Q_PORT_INITIAL_VID_VID = 20,
  /* 
   * The AC is mapped according to untagged packets (port lookup only)
   */
  SOC_PPC_L2_LIF_AC_MAP_KEY_TYPE_UNTAG = 21,
  SOC_PPC_L2_LIF_AC_MAP_KEY_TYPE_TEST2 = 22,

  /*
   *  Number of types in SOC_PPC_L2_LIF_AC_MAP_KEY_TYPE
   */
  SOC_PPC_NOF_L2_LIF_AC_MAP_KEY_TYPES_ARAD = 22
}SOC_PPC_L2_LIF_AC_MAP_KEY_TYPE;

typedef enum
{
  /*
   *  The default forwarding is according to the VSI
   */
  SOC_PPC_L2_LIF_DFLT_FRWRD_SRC_VSI = 0,
  /*
   *  The default forwarding is according to LIF
   */
  SOC_PPC_L2_LIF_DFLT_FRWRD_SRC_LIF = 1,
  /*
   *  Number of types in SOC_PPC_L2_LIF_DFLT_FRWRD_SRC
   */
  SOC_PPC_NOF_L2_LIF_DFLT_FRWRD_SRCS = 2
}SOC_PPC_L2_LIF_DFLT_FRWRD_SRC;

typedef enum
{
  /*
   * AC multi-point service
   */
  SOC_PPC_L2_LIF_AC_SERVICE_TYPE_MP = 0,
  /*
   * AC to AC service
   */
  SOC_PPC_L2_LIF_AC_SERVICE_TYPE_AC2AC = 1,
  /*
   * AC to PWE service
   */
  SOC_PPC_L2_LIF_AC_SERVICE_TYPE_AC2PWE = 2,
  /*
   *  AC to ISID service (MAC in MAC)
   */
  SOC_PPC_L2_LIF_AC_SERVICE_TYPE_AC2PBB = 3,
  /*
   *  Number of types in SOC_PPC_L2_LIF_AC_SERVICE_TYPE
   */
  SOC_PPC_NOF_L2_LIF_AC_SERVICE_TYPES = 4
}SOC_PPC_L2_LIF_AC_SERVICE_TYPE;

typedef enum
{
  /*
   * PWE multi-point service
   */
  SOC_PPC_L2_LIF_PWE_SERVICE_TYPE_MP = 0,
  /*
   * PWE point to point service
   */
  SOC_PPC_L2_LIF_PWE_SERVICE_TYPE_P2P = 1,
  /*
   *  User defined program
   */
  SOC_PPC_L2_LIF_PWE_SERVICE_TYPE_P2P_CUSTOM = 2,
  /*
   *  Number of types in SOC_PPC_L2_LIF_PWE_SERVICE_TYPE
   */
  SOC_PPC_NOF_L2_LIF_PWE_SERVICE_TYPES = 3
}SOC_PPC_L2_LIF_PWE_SERVICE_TYPE;

typedef enum
{
  /*
   * ISID point to point service
   */
  SOC_PPC_L2_LIF_ISID_SERVICE_TYPE_P2P = 0,
  /*
   * ISID multi-point service
   */
  SOC_PPC_L2_LIF_ISID_SERVICE_TYPE_MP = 1,
  /*
   *  Number of types in SOC_PPC_L2_LIF_ISID_SERVICE_TYPE
   */
  SOC_PPC_NOF_L2_LIF_ISID_SERVICE_TYPES = 2
}SOC_PPC_L2_LIF_ISID_SERVICE_TYPE;

typedef enum
{
  /*
   *  Learning is disabled
   */
  SOC_PPC_L2_LIF_AC_LEARN_DISABLE = 0,
  /*
   *  MAC Addresses are learned on source system port/LAG
   */
  SOC_PPC_L2_LIF_AC_LEARN_SYS_PORT = 1,
  /*
   *  MAC Addresses are learned with a specific forwarding
   *  decision (e.g., specific TM-Flow)
   */
  SOC_PPC_L2_LIF_AC_LEARN_INFO = 2,
  /*
   *  MAC Addresses are learned on source system port/LAG
   *  on the incomming LIF
   */
  SOC_PPC_L2_LIF_AC_LEARN_SYS_PORT_LIF = 3,
  /*
   *  Number of types in SOC_PPC_L2_LIF_AC_LEARN_TYPE
   */
  SOC_PPC_NOF_L2_LIF_AC_LEARN_TYPES = 4
}SOC_PPC_L2_LIF_AC_LEARN_TYPE;

typedef enum
{
  /*
   *  Handle the L2CP packet as normal packet (including vlan
   *  editing)
   */
  SOC_PPC_L2_LIF_L2CP_HANDLE_TYPE_NORMAL = 0,
  /*
   *  The packet is forwarded Transparently (with no vlan
   *  editing) i.e. the content of a Service Frame is
   *  delivered unaltered.
   */
  SOC_PPC_L2_LIF_L2CP_HANDLE_TYPE_TUNNEL = 1,
  /*
   *  Packet is trapped and assigned SOC_PPC_TRAP_CODE_L2CP_PEER
   *  trap code
   */
  SOC_PPC_L2_LIF_L2CP_HANDLE_TYPE_PEER = 2,
  /*
   *  Packet is dropped, assigned SOC_PPC_TRAP_CODE_L2CP_DROP trap
   *  code
   */
  SOC_PPC_L2_LIF_L2CP_HANDLE_TYPE_DROP = 3,
  /*
   *  Number of types in SOC_PPC_L2_LIF_L2CP_HANDLE_TYPE
   */
  SOC_PPC_NOF_L2_LIF_L2CP_HANDLE_TYPES = 4
} SOC_PPC_L2_LIF_L2CP_HANDLE_TYPE;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  First MPLS label, to be used as In-VC label.
   */
  SOC_SAND_PP_MPLS_LABEL first_label;
  /*
   *  Last MPLS label, to be used as In-VC label.
   */
  SOC_SAND_PP_MPLS_LABEL last_label;

} SOC_PPC_L2_LIF_IN_VC_RANGE;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  Port Profile ID. Range 0 - 7. set according to SOC_PPC_port_info_set()
   */
  uint32 port_profile;
  /*
   *  Parsing information specifying what TPIDs exist on the packet
   */
  SOC_PPC_LLP_PARSE_INFO pkt_parse_info;

} SOC_PPC_L2_LIF_AC_KEY_QUALIFIER;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  LIF: Use the following fields in case of unknown MAC
   *  destination. VSI: Use VSI default forwarding in case of
   *  unknown MAC destination.
   */
  SOC_PPC_L2_LIF_DFLT_FRWRD_SRC default_frwd_type;
  /*
   *  Default LIF forwarding decision to apply when there is
   *  no hit in the MAC table.
   */
  SOC_PPC_FRWRD_DECISION_INFO default_forwarding;

} SOC_PPC_L2_LIF_DEFAULT_FORWARDING_INFO;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  When enabled, packets arriving with this PWE are
   *  dynamically learned
   */
  uint8 enable_learning;
  /*
   *  Learning information. MAC addresses learnt on this PWE
   *  are inserted to the MACT with this information as their
   *  forwarding decision.
   *  Soc_petra-B:
   *    in ingress learning only system-port can be learned
   */
  SOC_PPC_FRWRD_DECISION_INFO learn_info;

} SOC_PPC_L2_LIF_PWE_LEARN_RECORD;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *
   */
  uint8 inner_vid_valid;
  /*
   *
   */
  uint16 inner_vid;
  /*
   *
   */
  uint8 outer_vid_valid;
  /*
   *
   */
  uint16 outer_vid;
  /*
   *
   */ 
uint16  vlan_domain;
  /*
   *
   */ 
uint16  vlan_domain_valid;
} SOC_PPC_L2_LIF_PWE_ADDITIONAL_INFO;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *
   */
  SOC_PPC_VSI_ID vsid;
  /*
   *  Enable setting the default forwarding according to PWE
   *  and not according to VSI. May be useful for E-LINE and
   *  E-TREE services.
   */
  SOC_PPC_L2_LIF_DEFAULT_FORWARDING_INFO default_frwrd;
  /*
   *  Learning-related information
   */
  SOC_PPC_L2_LIF_PWE_LEARN_RECORD learn_record;
  /*
   *  Hub/Spoke: Orientation HUB.1. H-VPLS support: Split
   *  Horizon filter is not activated when the packet origin
   *  is Spoke and the destination is HUB, nor the other way
   *  around.2. VSI Default forwarding: The VSI default
   *  forwarding, upon unknown MAC DA, is different for HUB
   *  LIF and Spoke LIF. Useful for H-VPLS, E-TREE and E-LINE
   *  services
   */
  SOC_SAND_PP_HUB_SPOKE_ORIENTATION orientation;
  /*
   *  Pointer to TPID profile options table. Needed for the
   *  internal L2 header parsing. The profile table is
   *  configured via 'SOC_PPC_llp_parse_tpid_values_set()'
   */
  uint32 tpid_profile_index;
  /*
   *  Class of Service mapping profile. Setting the profile to
   *  '0' keeps the previous TC and DP values
   */
  uint32 cos_profile;
  /*
   *  Has Control-word. Used by the PWE termination to
   *  determine number of bytes to remove from the header
   *  Used only in PB.
   */
  uint8 has_cw;
  /*
   *  PWE service type. P2P or MP.
   */
  SOC_PPC_L2_LIF_PWE_SERVICE_TYPE service_type;
  /* 
   * VSI assignment mode, 
   * used to refine above VSI value 
   * range: 0-3. 
   * Used only in Arad. 
   */ 
  uint32 vsi_assignment_mode;
  /* 
   * in-lif is used, for hair-pin filter and learning 
   * Used only in Arad. 
   */ 
  uint8 use_lif; 
  /* 
   * lif general profile, 
   * used for OAM and PMF setting 
   * range: 0-15. 
   * Used only in Arad. 
  */ 
  uint32 lif_profile; 
   /* 
   * OAM instance to observe as failover id 
   * Used only in Arad. 
   */ 
  uint32 protection_pointer; 
  /*  
   * OAM instance pass value. 
   * oam_instance_id.value != oam_instance_pass_val; 
   * then packet is dropped otherwise packet is forwarded 
   * range: 0 - 1. 
   * Used only in Arad. 
   */ 
  uint8 protection_pass_value; 
  /* 
   * Default LIF forwarding decision to apply when there is 
   * no hit in the MAC table. The profile ID is part of the 
   * 'dflt_frwrd_key', used by 
   * soc_ppd_lif_default_frwrd_info_set() Soc_petra only. 
   * range: 0 -3. 
   * Used only in Arad. 
   */ 
  uint32 default_forward_profile; 
  /* 
   * Does this LIF support OAM, 
   * Used only in Arad. 
   */ 
  uint8 oam_valid;
  /* 
   * termination profile, indicates how to 
   * process the terminated mpls-header 
   * see soc_ppd_lif_mpls_term_profile_info_set 
   * Arad only. 
   * Range 0-7. 
   */ 
  uint8 term_profile; 
  /* 
   * profile used to set trap action for label 
   * Arad only. 
   * Range 0-7. 
   */ 
  uint8 action_profile;
   /* 
   * Tunnel model (Pipe or uniform) 
   * if Uniform: then TTL and EXP of inner header 
   * are mapped from the terminated MPLS header 
   * if Pipe: then TTL and EXP of inner header are 
   * not affected from the terminated MPLS header 
   * see soc_ppd_eg_qos_params_remark_set() for mapping 
   * Used only for PWE P2P. 
   */ 
  SOC_SAND_PP_MPLS_TUNNEL_MODEL model;
  /*
   *  Has GAL. Used by the PWE termination to
   *  signal PWE with GAL termination.
   */
  uint8 has_gal;
  /*
   *  SEM result
   *  Used in case SEM result and
   *  In-LIF in LIF table are not equal.
   */
  uint32 lif_table_entry_in_lif_field;

  /*
   * flag to indicate LIF has extension. Only relevant in Jericho.
   */
  uint8 is_extended;

  /*
   * Extension lif id for this lif. Only relevant in Jericho.
   */
  int ext_lif_id;

  /*
   * Global lif id for this lif. Only relevant in Jericho.
   */
  int global_lif_id;

  /* An extension of orientation bit to a 2 bit classification. Like orientation,
     this is used for split horizon filtering */
  int network_group;
} SOC_PPC_L2_LIF_PWE_INFO;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  Local inner-port ID.
   */
  SOC_PPC_PORT local_port_ndx;
  /*
   *  Set to TRUE to indicate valid range of outer Vlan-tag.
   *  Otherwise, valid range of inner Vlan-tag.
   */
  uint8 is_outer;
  /*
   *  First VID in the range. When AC is added, with this VID,
   *  the AC is defined for all the VLAN in the range.
   */
  SOC_SAND_PP_VLAN_ID first_vid;
  /*
   *  Last VID in the range.
   */
  SOC_SAND_PP_VLAN_ID last_vid;
  /*
   *  Core
   */
  int           core_id;


} SOC_PPC_L2_VLAN_RANGE_KEY;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  First VID in the range. When AC is added, with this VID,
   *  the AC is defined for all the VLAN in the range.
   */
  SOC_SAND_PP_VLAN_ID first_vid;
  /*
   *  Last VID in the range.
   */
  SOC_SAND_PP_VLAN_ID last_vid;

} SOC_PPC_L2_VLAN_RANGE_INFO;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  All the outer VLAN ranges.
   */
  SOC_PPC_L2_VLAN_RANGE_INFO outer_vlan_range[SOC_PPC_LIF_VLAN_RANGE_MAX_SIZE];

  /*
   *  All the inner VLAN ranges.
   */
  SOC_PPC_L2_VLAN_RANGE_INFO inner_vlan_range[SOC_PPC_LIF_VLAN_RANGE_MAX_SIZE];
  
} SOC_PPC_L2_PORT_VLAN_RANGE_INFO;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  Key type. Determines which attributes of the packet to
   *  consider when associate the packet with AC-ID.
   */
  SOC_PPC_L2_LIF_AC_MAP_KEY_TYPE key_type;
  /*
   *  VLAN Domain ID. The VLAN domain is either a port, or a
   *  group of ports that share the same VLAN IDs space.
   *  Note: in case key type is _Port only, the vlan_domain,
   *  equal to in-pp-port (i.e. in-pp-port default)
   */
  uint32 vlan_domain;
  /*
   *  Outer VID. Set to SOC_PPC_LIF_IGNORE_OUTER_VID in order to
   *  ignore outer_VID, then also the inner VID will be
   *  ignored and the AC is set according to vlan_domain only.
   */
  SOC_SAND_PP_VLAN_ID outer_vid;
  /*
   *  Inner VID. Should be 'SOC_PPC_LIF_IGNORE_INNER_VID' when
   *  the vlan_domain refers to ports that do not support VSI
   *  according to 2 VIDs
   */
  SOC_SAND_PP_VLAN_ID inner_vid;
  /*
   *  tunnel ID. Should be 'SOC_PPC_LIF_IGNORE_INNER_VID' when
   *  the vlan_domain refers to ports that do not support VSI
   *  according to 2 VIDs
   */
  SOC_SAND_PON_TUNNEL_ID tunnel;
  /*
   *  tunnel ID. Should be 'SOC_PPC_LIF_IGNORE_INNER_VID' when
   *  the vlan_domain refers to ports that do not support VSI
   *  according to 2 VIDs
   */
  SOC_SAND_PP_ETHER_TYPE ethertype;
 /*
  *  For a Raw type, this field includes the raw data.
  */
  uint32 raw_key;
  /*
   *  Outer PCP. Must be set in case key type includes PCP.
   *  Invalid for Soc_petra-B
   */
  SOC_SAND_PP_PCP_UP outer_pcp;
  /*
   *  core id valid only vlan domain is port.
   */
  int  core_id;



} SOC_PPC_L2_LIF_AC_KEY;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  Learning may be:- Disabled- Source system port/LAG-
   *  Specific forwarding decision
   */
  SOC_PPC_L2_LIF_AC_LEARN_TYPE learn_type;
  /*
   *  Only relevant when 'learn_type' is
   *  'SOC_PPC_L2_LIF_AC_LEARN_INFO'. Defines the forwarding
   *  decision of packets whose MAC addresses are learned on
   *  this AC. When used, the forwarding decision's Out-LIF ID
   *  must be the AC's 'lif_ndx'. (EEI is not allowed)
   *  Soc_petra-B:
   *    in ingress learning only system-port can be learned
   */
  SOC_PPC_FRWRD_DECISION_INFO learn_info;

} SOC_PPC_L2_LIF_AC_LEARN_RECORD;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  Ingress-VLAN-Edit-Profile. Used together with the
   *  packet's tag structure to set packet's Ingress VLAN
   *  Editing command to perform over the packet. see
   *  SOC_PPC_llp_ing_vlan_edit_command_info_set()Range: 0 - 7.
   */
  uint32 ing_vlan_edit_profile;
  /*
   *  Translated-VID
   */
  SOC_SAND_PP_VLAN_ID vid;
    /* 
   * Translated-VID 2 
   */ 
  SOC_SAND_PP_VLAN_ID vid2;
  /*
   *  PCP profile, used during ingress VLAN editing phase, to
   *  set the PCP/UP of the edited VLAN headers. See
   *  SOC_PPC_ing_vlan_edit_command_info_set ().
   */
  uint32 edit_pcp_profile;

} SOC_PPC_L2_LIF_AC_ING_EDIT_INFO;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  AC service type. One of 4 service types.
   */
  SOC_PPC_L2_LIF_AC_SERVICE_TYPE service_type;
  /*
   *  Virtual Switch ID. If the service type is P2P then use
   *  VSI SOC_PPC_VSI_P2P_SERVICE.
   *  use SOC_PPC_VSI_EQ_IN_VID to set VSI = VID
   *  according to VID packet belongs to each packet
   */
  SOC_PPC_VSI_ID vsid;
  /*
   *  Learning related information
   */
  SOC_PPC_L2_LIF_AC_LEARN_RECORD learn_record;
  /*
   *  Ingress editing information
   */
  SOC_PPC_L2_LIF_AC_ING_EDIT_INFO ing_edit_info;
  /*
   *  Assign AC-based default forwarding action. Multipoint
   *  services are assigned a default forwarding action per
   *  VSID.
   */
  SOC_PPC_L2_LIF_DEFAULT_FORWARDING_INFO default_frwrd;
  /*
   *  Hub/Spoke Orientation. Typically, ACs are Spokes. VSI
   *  default forwarding: The VSI default forwarding, upon
   *  unknown MAC DA, is different for HUB LIF and Spoke LIF.
   *  Useful for E-TREE and E-LINE services. HVPLS: used for
   *  Split-Horizon filtering, when in-AC and out-AC are
   *  defined as HUB then is packet is filtered.
   */
  SOC_SAND_PP_HUB_SPOKE_ORIENTATION orientation;
  /*
   *  Class of Service mapping profile. Setting the profile to
   *  '0' keeps the previous TC and DP values
   */
  uint32 cos_profile;
  /*
   *  Profile to control the trapping editing of Layer 2
   *  control protocols. See SOC_PPC_l2_lif_l2cp_trap_set().
   *  Soc_petra-B only.
   */
  uint32 l2cp_profile;
   /* 
   * lif general profile.
   * used for OAM and PMF setting 
   * range: 0-15. 
   * Used only in Arad. 
  */ 
  uint32 lif_profile;
   /* 
   * OAM instance to observe as failover id 
   * Used only in Arad. 
   */ 
  uint32 protection_pointer; 
  /* 
   * OAM instance pass value. 
   * if oam_instance_id.value != oam_instance_pass_val
   * then packet is dropped otherwise packet is forwarded 
   * range: 0 - 1. 
   * Used only in Arad. 
   */ 
  uint8 protection_pass_value; 
  /* 
   * Does this LIF support OAM, 
   * Used only in Arad. 
  */ 
  uint8 oam_valid;
  /* 
   * VSI assignment mode, 
   * used to refine above VSI value 
   * range: 0-3. 
   * SOC_PPC_VSI_EQ_IN_VID - VSI equals VID 
   * Used only in Arad. 
   */ 
  uint32 vsi_assignment_mode;
  /* 
   * Default LIF forwarding decision to apply when there is 
   * no hit in the MAC table. The profile ID is part of the 
   * 'dflt_frwrd_key', used by 
   * soc_ppd_lif_default_frwrd_info_set() Soc_petra only. 
   * range: 0 -3. 
  * Used only in Arad. 
   */ 
  uint32 default_forward_profile;
  /* 
   * in-lif is used, for hair-pin filter and learning 
   * Used only in Arad. 
   */ 
  uint8 use_lif;      

  /*
   * flag to indicate LIF has extension. Only relevant in Jericho.
   */
  uint8 is_extended;

  /*
   * Extension lif id for this lif. Only relevant in Jericho.
   */
  int ext_lif_id;

  /* 
   * Global lif that this ac lif is pointing to.
   *
   */
  int global_lif;

  /* An extension of orientation bit to a 2 bit classification. Like orientation,
     this is used for split horizon filtering */
  int network_group;

} SOC_PPC_L2_LIF_AC_INFO;

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

} SOC_PPC_L2_LIF_AC_MP_INFO;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  l2cp profile (set according to LIF see
   *  SOC_PPC_l2_lif_ac_add()). Range: 0 - 1.
   */
  uint32 l2cp_profile;
  /*
   *  The 6 lsb of the Destination MAC address (DA[5:0]). The
   *  msb bits DA[47:6] are 01-80-c2-00-00-XX where XX =
   *  8'b00xx_xxxx)Range: 0 - 63.
   */
  uint32 da_mac_address_lsb;

} SOC_PPC_L2_LIF_L2CP_KEY;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  Array of the Attachemt Circuits that maped to port * VID
   *  [*VID]. An AC in the array is attached to a packet
   *  according to QoS attributes and the Op-Code mapping
   */
  SOC_PPC_L2_LIF_AC_INFO acs_info[SOC_PPC_LIF_AC_GROUP_MAX_SIZE];
  /*
   *  Opcode ID. Range: 0-7 inT20E 0-3 in Soc_petraSet the mapping
   *  between the QoS attributes and the LIF table offset. The
   *  opcode determines the QoS attributes that affects the AC
   *  identification, and the mapping of those attributes to
   *  AC Ids. Opcode ID ' 7 '/ ' 3 ' (T20E / Soc_petraB) is
   *  reserved for Non-QoS mapping
   */
  uint32 opcode_id;
  /*
   *  Number of ACs in the LIF table that mapped to the
   *  AC-Key. The driver validates that 'nof_lif_entries'
   *  corresponds to the number of different offsets encoded
   *  by 'opcode_id'.
   */
  uint32 nof_lif_entries;

} SOC_PPC_L2_LIF_AC_GROUP_INFO;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  I-SID value, as arriving in the I-TAG
   */
  SOC_SAND_PP_ISID isid_id;
  /*
   *  I-SID Domain ID. Enable mapping to VSID according to
   *  I-SID * I-SID Domain. The I-SID domain is set according
   *  to the MIM tunnel (B-VID * B-SA)SOC_PPC_ISID_DM_DISABLE,
   *  indicates that the mapping is only according to I-SID
   */
  uint32 isid_domain;

} SOC_PPC_L2_LIF_ISID_KEY;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  Enable setting the default forwarding according to PWE
   *  and not according to VSI. May be useful for E-LINE and
   *  E-TREE services.
   */
  SOC_PPC_L2_LIF_DEFAULT_FORWARDING_INFO default_frwrd;
  /*
   *  Indicate whether learning of customer MACs from this
   *  tunnel and on this service are permitted. Should be
   *  disabled for PBB-TE.
   */
  uint8 learn_enable;
  /*
   *  Pointer to TPID profile options table. Needed for the
   *  internal L2 header parsing. The profile table is
   *  configured via 'SOC_PPC_llp_parse_tpid_values_set()'
   */
  uint32 tpid_profile_index;
  /*
   *  Class of Service mapping profile. Setting the profile to
   *  '0' keeps the previous TC and DP values
   */
  uint32 cos_profile;
   /*
   *  Hub/Spoke: Orientation HUB.1. H-VPLS support: Split
   *  Horizon filter is not activated when the packet origin
   *  is Spoke and the destination is HUB, nor the other way
   *  around.2. VSI Default forwarding: The VSI default
   *  forwarding, upon unknown MAC DA, is different for HUB
   *  LIF and Spoke LIF. Useful for H-VPLS, E-TREE and E-LINE
   *  services
   */
  SOC_SAND_PP_HUB_SPOKE_ORIENTATION orientation;
  /*
   *  ISID service type. P2P or MP..
   */
  SOC_PPC_L2_LIF_ISID_SERVICE_TYPE service_type;
  /* 
   * VSI assignment mode, 
   * used to refine above VSI value 
   * range: 0-3. 
   * Used only in Arad. 
   */ 
  uint32 vsi_assignment_mode; 
  /* 
   * in-lif is used, for hair-pin filter and learning 
   * Used only in Arad. 
   */ 
  uint8 use_lif; 
  /* 
   * OAM instance to observe as failover id 
   * Used only in Arad. 
   */ 
  uint32 protection_pointer; 
  /* 
   * OAM instance pass value. 
   * oam_instance_id.value != oam_instance_pass_val; 
   * then packet is dropped otherwise packet is forwarded 
   * range: 0 - 1. 
   * Used only in Arad. 
   */ 
  uint8 protection_pass_value;
  /* 
   * Does this LIF support OAM, 
   * Used only in Arad. 
   */ 
  uint8 oam_valid; 
  /* 
   * Default LIF forwarding decision to apply when there is 
   * no hit in the MAC table. The profile ID is part of the 
   * 'dflt_frwrd_key', used by 
   * soc_ppd_lif_default_frwrd_info_set() Soc_petra only. 
   * range: 0 -3. 
   * Used only in Arad.  
   */ 
  uint32 default_forward_profile; 

  /*
   * flag to indicate LIF has extension. Only relevant in Jericho.
   */
  uint8 is_extended;

  /*
   * Extension lif id for this lif. Only relevant in Jericho.
   */
  int ext_lif_id;

  /* 
   * Global lif that this ac lif is pointing to.
   *
   */
  int global_lif;

  /* 
   * Lif Profile.
   *
   */
  uint32 lif_profile;

} SOC_PPC_L2_LIF_ISID_INFO;


typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  24 bit from the GRE key
   */
  uint32 vpn_key;
  /*
   *  8 bits, local port vlan domain;
   */
  int match_port_class;
} SOC_PPC_L2_LIF_GRE_KEY;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  Virtual Switch ID. If the service type is P2P then use
   *  VSI SOC_PPC_VSI_P2P_SERVICE.
   *  use SOC_PPC_VSI_EQ_IN_VID to set VSI = VID
   *  according to VID packet belongs to each packet
   */
  SOC_PPC_VSI_ID vsid;

  /*
   * flag to indicate LIF has extension. Only relevant in Jericho.
   */
  uint8 is_extended;

  /*
   * Extension lif id for this lif. Only relevant in Jericho.
   */
  int ext_lif_id;

} SOC_PPC_L2_LIF_GRE_INFO;


typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  24 bit from the VXLAN key
   */
  uint32 vpn_key;
  /*
   *  8 bits, local port vlan domain;
   */
  int match_port_class;
} SOC_PPC_L2_LIF_VXLAN_KEY;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  Virtual Switch ID. If the service type is P2P then use
   *  VSI SOC_PPC_VSI_P2P_SERVICE.
   *  use SOC_PPC_VSI_EQ_IN_VID to set VSI = VID
   *  according to VID packet belongs to each packet
   */
  SOC_PPC_VSI_ID vsid;

  /*
   * flag to indicate LIF has extension. Only relevant in Jericho.
   */
  uint8 is_extended;

  /*
   * Extension lif id for this lif. Only relevant in Jericho.
   */
  int ext_lif_id;

} SOC_PPC_L2_LIF_VXLAN_INFO;
/* 
 * ARAD only structures {
 */
typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  Default Port forwarding decision to apply when there is
   *  no hit in the MAC table. User may set up to 2 different DA-Not-Found-Profile per port.
   *  Set by soc_ppd_api_port_info_set()
   */
  uint32 port_da_not_found_profile;
  /*
   *  Packet MAC address type (UC/MC/BC). The user may set
   *  different defaults for each type.
   */
  SOC_SAND_PP_ETHERNET_DA_TYPE da_type;
  /* 
   * Default LIF forwarding decision to apply when there is 
   * no hit in the MAC table. The profile ID is part of the 
   * 'dflt_frwrd_key'.
   * Set by lif information.
   * range: 0 -3. 
   * 
   */ 
  uint32 lif_default_forward_profile;

} SOC_PPC_L2_LIF_DEFAULT_FRWRD_KEY;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  Destination to forward the packet to,
   *  only type and dest_id are relevant.
   *  frwrd_dest.additional_info is not relevant.
   *  Only used when add_vsi is FALSE.
   */
  SOC_PPC_FRWRD_DECISION_INFO frwrd_dest;

  /* 
   *  Append offset to Destination to forward the packet to.
   *  Only used when add_vsi is TRUE.
   */
  uint32 offset;

  /*
   *  Indicates that the VSI value should be added to the
   *  Forward-Destination, i.e., the Forward-Destination above
   *  is to be treated as a base value. This is useful to
   *  define the default LIF forwarding. Note: This considers
   *  the local VSI and not the system VSI.
   *  In case add_vsi is TRUE, use prameters offset to obtain the value
   *  added to Forward-Destination. In case add_vsi is FALSE, use parameter
   *  frwrd_dest to declare the Forward-Destination.
   */
  uint8 add_vsi;


} SOC_PPC_L2_LIF_DEFAULT_FRWRD_ACTION;

typedef struct { 
   SOC_SAND_MAGIC_NUM_VAR

  /*  Petra-B only: 
   *  The FEC-ID that the TRILL Nickname is attached to.
   */
  SOC_PPC_FEC_ID fec_id;

  /*
   * Indicate whether learning of MAC on this interface is enable. 
   */
  uint32 learn_enable;

  /* 
   * For arad+: the learning info is FEC. See fec_id field
   * For Jericho: Learning info is FEC + EEI. 
   */ 
  SOC_PPC_FRWRD_DECISION_INFO learn_info; 

  uint32 vsi_id;

  /* 
   * OAM instance to observe as failover id 
   * Used only in Arad. 
   */ 
  uint32 protection_pointer; 
  /* 
   * OAM instance pass value. 
   * oam_instance_id.value != oam_instance_pass_val; 
   * then packet is dropped otherwise packet is forwarded 
   * range: 0 - 1. 
   * Used only in Arad. 
   */ 
  uint8 protection_pass_value;

    /*
   * flag to indicate LIF has extension. Only relevant in Jericho.
   */
  uint8 is_extended;

    /*
   * Global lif id for this trill lif. Only relevant in Jericho.
   */
  int global_lif_id;

  /*
   *  Hub/Spoke Orientation. Typically, Trill are Hub.
   *  used for Split-Horizon filtering, when in-Lif and out-Lif are
   *  both defined as HUB then the packet is filtered.
   */
  SOC_SAND_PP_HUB_SPOKE_ORIENTATION orientation;

} SOC_PPC_L2_LIF_TRILL_INFO; 



typedef struct
{
    SOC_SAND_MAGIC_NUM_VAR

    /*
     * Extenter Port E-CID. Part of the E-TAG that Identifies the E-Channel 
     * between a Control Bridge and an external Port Extender port
     */
    SOC_SAND_PP_ECID extender_port_vid;

    /*
     * C-VID or Initial-VID. The VID value of the tag that follows the E-TAG within a Port
     * Extender packet or the initial VID in case of untagged packet.
     */
    SOC_SAND_PP_VLAN_ID vid;

    /*
     *  Name Space is a VLAN Domain ID. The VLAN domain is either a port, or a
     *  group of ports that share the same VLAN IDs space. A port in this case
     *  is a Cascaded Physical port on a Control bridge that provides connection
     *  to the Port Extender port.
     */
    uint32 name_space;

    /*
     * Packet is tagged indication
     */
    uint32 is_tagged;
} SOC_PPC_L2_LIF_EXTENDER_KEY;


typedef struct
{
    SOC_SAND_MAGIC_NUM_VAR

    /*
     *  AC service type. One of 4 service types. 2 Service types are possible P2P (AC2AC) or MP.
     */
    SOC_PPC_L2_LIF_AC_SERVICE_TYPE service_type;

    /*
     *  Virtual Switch ID. If the service type is P2P then use
     *  VSI SOC_PPC_VSI_P2P_SERVICE.
     *  use SOC_PPC_VSI_EQ_IN_VID to set VSI = VID
     *  according to VID packet belongs to each packet
     */
    SOC_PPC_VSI_ID vsid;

    /*
     *  Learning related information
     */
    SOC_PPC_L2_LIF_AC_LEARN_RECORD learn_record;

    /*
     *  Assign AC-based default forwarding action. Multipoint
     *  services are assigned a default forwarding action per
     *  VSID.
     */
    SOC_PPC_L2_LIF_DEFAULT_FORWARDING_INFO default_frwrd;

    /*
     *  Class of Service mapping profile. Setting the profile to
     *  '0' keeps the previous TC and DP values
     */
    uint32 cos_profile;

    /* 
     * LIF general profile.
     * used for OAM and PMF setting 
     * range: 0-15. 
     */ 
    uint32 lif_profile;

    /* 
     * OAM instance to observe as failover id 
     */ 
    uint32 protection_pointer; 

    /* 
     * OAM instance pass value. 
     * if oam_instance_id.value != oam_instance_pass_val
     * then packet is dropped otherwise packet is forwarded 
     * range: 0 - 1. 
     */ 
    uint8 protection_pass_value; 

    /* 
     * Indication whether the LIF supports OAM 
     */ 
    uint8 oam_valid;

    /* 
     * VSI assignment mode, 
     * used to refine above VSI value 
     * range: 0-3. 
     */ 
    uint32 vsi_assignment_mode;

    /* 
     * Default LIF forwarding decision to apply when there is 
     * no hit in the MAC table. The profile ID is part of the 
     * 'dflt_frwrd_key', used by 
     * soc_ppd_lif_default_frwrd_info_set() Soc_petra only. 
     * range: 0 -3. 
     */ 
    uint32 default_forward_profile;

    /* 
     * in-lif is used, for hair-pin filter and learning 
     */ 
    uint8 use_lif;      

    /*
     * flag to indicate LIF has extension. Only relevant in Jericho.
     */
    uint8 is_extended;

    /*
     * Extension lif id for this lif. Only relevant in Jericho.
     */
    int ext_lif_id;

    /* 
     * Global lif that this ac lif is pointing to.
     *
     */
    int global_lif;

} SOC_PPC_L2_LIF_EXTENDER_INFO;


/* 
 * ARAD only structures }
 */

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
  SOC_PPC_L2_LIF_AC_KEY_QUALIFIER_clear(
    SOC_SAND_OUT SOC_PPC_L2_LIF_AC_KEY_QUALIFIER *info
  );

void
  SOC_PPC_L2_LIF_IN_VC_RANGE_clear(
    SOC_SAND_OUT SOC_PPC_L2_LIF_IN_VC_RANGE *info
  );

void
  SOC_PPC_L2_LIF_DEFAULT_FORWARDING_INFO_clear(
    SOC_SAND_OUT SOC_PPC_L2_LIF_DEFAULT_FORWARDING_INFO *info
  );

void
  SOC_PPC_L2_LIF_PWE_LEARN_RECORD_clear(
    SOC_SAND_OUT SOC_PPC_L2_LIF_PWE_LEARN_RECORD *info
  );

void
  SOC_PPC_L2_LIF_PWE_ADDITIONAL_INFO_clear(
    SOC_SAND_OUT SOC_PPC_L2_LIF_PWE_ADDITIONAL_INFO *info
  );

void
  SOC_PPC_L2_LIF_PWE_INFO_clear(
    SOC_SAND_OUT SOC_PPC_L2_LIF_PWE_INFO *info
  );

void
  SOC_PPC_L2_VLAN_RANGE_KEY_clear(
    SOC_SAND_OUT SOC_PPC_L2_VLAN_RANGE_KEY *info
  );

void
  SOC_PPC_L2_PORT_VLAN_RANGE_INFO_clear(
    SOC_SAND_OUT SOC_PPC_L2_PORT_VLAN_RANGE_INFO *info
  );

void
  SOC_PPC_L2_LIF_AC_KEY_clear(
    SOC_SAND_OUT SOC_PPC_L2_LIF_AC_KEY *info
  );

void
  SOC_PPC_L2_LIF_AC_LEARN_RECORD_clear(
    SOC_SAND_OUT SOC_PPC_L2_LIF_AC_LEARN_RECORD *info
  );

void
  SOC_PPC_L2_LIF_AC_ING_EDIT_INFO_clear(
    SOC_SAND_OUT SOC_PPC_L2_LIF_AC_ING_EDIT_INFO *info
  );

void
  SOC_PPC_L2_LIF_AC_INFO_clear(
    SOC_SAND_OUT SOC_PPC_L2_LIF_AC_INFO *info
  );

void
  SOC_PPC_L2_LIF_AC_MP_INFO_clear(
    SOC_SAND_OUT SOC_PPC_L2_LIF_AC_MP_INFO *info
  );

void
  SOC_PPC_L2_LIF_L2CP_KEY_clear(
    SOC_SAND_OUT SOC_PPC_L2_LIF_L2CP_KEY *info
  );

void
  SOC_PPC_L2_LIF_AC_GROUP_INFO_clear(
    SOC_SAND_OUT SOC_PPC_L2_LIF_AC_GROUP_INFO *info
  );

void
  SOC_PPC_L2_LIF_ISID_KEY_clear(
    SOC_SAND_OUT SOC_PPC_L2_LIF_ISID_KEY *info
  );

void
  SOC_PPC_L2_LIF_ISID_INFO_clear(
    SOC_SAND_OUT SOC_PPC_L2_LIF_ISID_INFO *info
  );

void 
  SOC_PPC_L2_LIF_TRILL_INFO_clear(
    SOC_SAND_OUT SOC_PPC_L2_LIF_TRILL_INFO *info
  );

void
  SOC_PPC_L2_LIF_TRILL_INFO_print(
    SOC_SAND_IN  SOC_PPC_L2_LIF_TRILL_INFO *info
  );

void
  SOC_PPC_L2_LIF_GRE_KEY_clear(
    SOC_SAND_OUT SOC_PPC_L2_LIF_GRE_KEY *info
  );

void
  SOC_PPC_L2_LIF_GRE_INFO_clear(
    SOC_SAND_OUT SOC_PPC_L2_LIF_GRE_INFO *info
  );

void
  SOC_PPC_L2_LIF_VXLAN_KEY_clear(
    SOC_SAND_OUT SOC_PPC_L2_LIF_VXLAN_KEY *info
  );

void
  SOC_PPC_L2_LIF_VXLAN_INFO_clear(
    SOC_SAND_OUT SOC_PPC_L2_LIF_VXLAN_INFO *info
  );

void
  SOC_PPC_L2_LIF_DEFAULT_FRWRD_KEY_clear(
    SOC_SAND_OUT SOC_PPC_L2_LIF_DEFAULT_FRWRD_KEY *info
  );

void
  SOC_PPC_L2_LIF_DEFAULT_FRWRD_ACTION_clear(
    SOC_SAND_OUT SOC_PPC_L2_LIF_DEFAULT_FRWRD_ACTION *info
  );

void 
  SOC_PPC_L2_LIF_EXTENDER_KEY_clear(
    SOC_SAND_OUT SOC_PPC_L2_LIF_EXTENDER_KEY *info
  );

void 
  SOC_PPC_L2_LIF_EXTENDER_INFO_clear(
    SOC_SAND_OUT SOC_PPC_L2_LIF_EXTENDER_INFO *info
  );

#if SOC_PPC_DEBUG_IS_LVL1

const char*
  SOC_PPC_L2_LIF_AC_MAP_KEY_TYPE_to_string(
    SOC_SAND_IN  SOC_PPC_L2_LIF_AC_MAP_KEY_TYPE enum_val
  );

const char*
  SOC_PPC_L2_LIF_DFLT_FRWRD_SRC_to_string(
    SOC_SAND_IN  SOC_PPC_L2_LIF_DFLT_FRWRD_SRC enum_val
  );

const char*
  SOC_PPC_L2_LIF_AC_SERVICE_TYPE_to_string(
    SOC_SAND_IN  SOC_PPC_L2_LIF_AC_SERVICE_TYPE enum_val
  );

const char*
  SOC_PPC_L2_LIF_PWE_SERVICE_TYPE_to_string(
    SOC_SAND_IN  SOC_PPC_L2_LIF_PWE_SERVICE_TYPE enum_val
  );

const char*
  SOC_PPC_L2_LIF_ISID_SERVICE_TYPE_to_string(
    SOC_SAND_IN  SOC_PPC_L2_LIF_ISID_SERVICE_TYPE enum_val
  );

const char*
  SOC_PPC_L2_LIF_AC_LEARN_TYPE_to_string(
    SOC_SAND_IN  SOC_PPC_L2_LIF_AC_LEARN_TYPE enum_val
  );

const char*
  SOC_PPC_L2_LIF_VSI_ASSIGNMENT_MODE_to_string(
    SOC_SAND_IN  uint32 val
  );

const char*
  SOC_PPC_L2_LIF_L2CP_HANDLE_TYPE_to_string(
    SOC_SAND_IN  SOC_PPC_L2_LIF_L2CP_HANDLE_TYPE enum_val
  );


void
  SOC_PPC_L2_LIF_AC_KEY_QUALIFIER_print(
    SOC_SAND_IN  SOC_PPC_L2_LIF_AC_KEY_QUALIFIER *info
  );

void
  SOC_PPC_L2_LIF_IN_VC_RANGE_print(
    SOC_SAND_IN  SOC_PPC_L2_LIF_IN_VC_RANGE *info
  );

void
  SOC_PPC_L2_LIF_DEFAULT_FORWARDING_INFO_print(
    SOC_SAND_IN  SOC_PPC_L2_LIF_DEFAULT_FORWARDING_INFO *info
  );

void
  SOC_PPC_L2_LIF_PWE_LEARN_RECORD_print(
    SOC_SAND_IN  SOC_PPC_L2_LIF_PWE_LEARN_RECORD *info
  );

void
  SOC_PPC_L2_LIF_PWE_INFO_print(
    SOC_SAND_IN  SOC_PPC_L2_LIF_PWE_INFO *info
  );

void
  SOC_PPC_L2_VLAN_RANGE_KEY_print(
    SOC_SAND_IN  SOC_PPC_L2_VLAN_RANGE_KEY *info
  );

void
  SOC_PPC_L2_PORT_VLAN_RANGE_INFO_print(
    SOC_SAND_IN  SOC_PPC_L2_PORT_VLAN_RANGE_INFO *info
  );

void
  SOC_PPC_L2_LIF_AC_KEY_print(
    SOC_SAND_IN  SOC_PPC_L2_LIF_AC_KEY *info
  );

void
  SOC_PPC_L2_LIF_AC_LEARN_RECORD_print(
    SOC_SAND_IN  SOC_PPC_L2_LIF_AC_LEARN_RECORD *info
  );

void
  SOC_PPC_L2_LIF_AC_ING_EDIT_INFO_print(
    SOC_SAND_IN  SOC_PPC_L2_LIF_AC_ING_EDIT_INFO *info
  );

void
  SOC_PPC_L2_LIF_AC_INFO_print(
    SOC_SAND_IN  SOC_PPC_L2_LIF_AC_INFO *info
  );

void
  SOC_PPC_L2_LIF_AC_MP_INFO_print(
    SOC_SAND_IN  SOC_PPC_L2_LIF_AC_MP_INFO *info
  );

void
  SOC_PPC_L2_LIF_L2CP_KEY_print(
    SOC_SAND_IN  SOC_PPC_L2_LIF_L2CP_KEY *info
  );

void
  SOC_PPC_L2_LIF_AC_GROUP_INFO_print(
    SOC_SAND_IN  SOC_PPC_L2_LIF_AC_GROUP_INFO *info
  );

void
  SOC_PPC_L2_LIF_ISID_KEY_print(
    SOC_SAND_IN  SOC_PPC_L2_LIF_ISID_KEY *info
  );

void
  SOC_PPC_L2_LIF_ISID_INFO_print(
    SOC_SAND_IN  SOC_PPC_L2_LIF_ISID_INFO *info
  );

void
  SOC_PPC_L2_LIF_GRE_KEY_print(
    SOC_SAND_IN  SOC_PPC_L2_LIF_GRE_KEY *info
  );

void
  SOC_PPC_L2_LIF_GRE_INFO_print(
    SOC_SAND_IN  SOC_PPC_L2_LIF_GRE_INFO *info
  );

void
  SOC_PPC_L2_LIF_VXLAN_KEY_print(
    SOC_SAND_IN  SOC_PPC_L2_LIF_VXLAN_KEY *info
  );

void
  SOC_PPC_L2_LIF_VXLAN_INFO_print(
    SOC_SAND_IN  SOC_PPC_L2_LIF_VXLAN_INFO *info
  );

void
  SOC_PPC_L2_LIF_DEFAULT_FRWRD_KEY_print(
    SOC_SAND_IN SOC_PPC_L2_LIF_DEFAULT_FRWRD_KEY *info
  );

void
  SOC_PPC_L2_LIF_DEFAULT_FRWRD_ACTION_print(
    SOC_SAND_IN SOC_PPC_L2_LIF_DEFAULT_FRWRD_ACTION *info
  );

int
  SOC_PPC_OCC_PROFILE_USAGE_print(
    SOC_SAND_IN int unit,
    SOC_SAND_IN SOC_OCC_MGMT_TYPE profile_type,
    SOC_SAND_IN int size
  );

int
  SOC_PPC_OCC_ALL_PROFILES_print(
    SOC_SAND_IN int unit
  );

void
  SOC_PPC_L2_LIF_EXTENDER_KEY_print(
    SOC_SAND_IN  SOC_PPC_L2_LIF_EXTENDER_KEY *info
  );

void
  SOC_PPC_L2_LIF_EXTENDER_INFO_print(
    SOC_SAND_IN  SOC_PPC_L2_LIF_EXTENDER_INFO *info
  );

#endif /* SOC_PPC_DEBUG_IS_LVL1 */

/* } */

#include <soc/dpp/SAND/Utils/sand_footer.h>

/* } __SOC_PPC_API_LIF_INCLUDED__*/
#endif
