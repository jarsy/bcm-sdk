/* $Id: ppc_api_frwrd_fec.h,v 1.15 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/
/******************************************************************
*
* FILENAME:       DuneDriver/ppc/include/soc_ppc_api_frwrd_fec.h
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

#ifndef __SOC_PPC_API_FRWRD_FEC_INCLUDED__
/* { */
#define __SOC_PPC_API_FRWRD_FEC_INCLUDED__

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


#define SOC_PPC_FEC_LB_CRC_17111      0x00
#define SOC_PPC_FEC_LB_CRC_10491      0x01
#define SOC_PPC_FEC_LB_CRC_155F5      0x02
#define SOC_PPC_FEC_LB_CRC_19715      0x03
#define SOC_PPC_FEC_LB_CRC_13965      0x04
#define SOC_PPC_FEC_LB_CRC_1698D      0x05
#define SOC_PPC_FEC_LB_CRC_1105D      0x06
#define SOC_PPC_FEC_LB_KEY            0x07    /* Use LB-Key-Packet-Data directly */
#define SOC_PPC_FEC_LB_ROUND_ROBIN    0x08    /* Use counter incremented every packet */
#define SOC_PPC_FEC_LB_2_CLOCK        0x09    /* User counter incremented every two clocks */
#define SOC_PPC_FEC_LB_CRC_10861      0x0A
#define SOC_PPC_FEC_LB_CRC_10285      0x0B
#define SOC_PPC_FEC_LB_CRC_101A1      0x0C
#define SOC_PPC_FEC_LB_CRC_12499      0x0D
#define SOC_PPC_FEC_LB_CRC_1F801      0x0E
#define SOC_PPC_FEC_LB_CRC_172E1      0x0F
#define SOC_PPC_FEC_LB_CRC_1EB21      0x10
/* Arad+ hash functions */
#define SOC_PPC_FEC_LB_CRC_0x8003       0x11
#define SOC_PPC_FEC_LB_CRC_0x8011       0x12
#define SOC_PPC_FEC_LB_CRC_0x8423       0x13
#define SOC_PPC_FEC_LB_CRC_0x8101       0x14
#define SOC_PPC_FEC_LB_CRC_0x84a1       0x15
#define SOC_PPC_FEC_LB_CRC_0x9019       0x16


#define SOC_PPC_FEC_VRF_STATE_NOT_CREATED     0
#define SOC_PPC_FEC_VRF_STATE_REGULAR         1
#define SOC_PPC_FEC_VRF_STATE_DOUBLE_CAPACITY 2

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
   *  The FEC entry is used to drop packets.
   *  relevant: Soc_petra-B, Arad
   */
  SOC_PPC_FEC_TYPE_DROP = 0,
  /*
   *  This FEC entry is used for IP MC routing.
   *  relevant: Soc_petra-B, Arad
   */
  SOC_PPC_FEC_TYPE_IP_MC = 1,
  /*
   *  This FEC entry is used for IP UC routing.
   *  relevant: Soc_petra-B
   *  FEC includes: arp-ptr (eep) and outrif.
   *  for Arad use routing instead
   */
  SOC_PPC_FEC_TYPE_IP_UC = 2,
  /*
   *  This FEC entry is used for TRILL MC multicast switching.
   *  relevant: Soc_petra-B
   *  dist_tree_nick is used as destination tree nick
   *  for Arad use routing instead
   */
  SOC_PPC_FEC_TYPE_TRILL_MC = 3,
  /*
   *  This FEC entry is used for TRILL UC multicast switching.
   *  relevant: Soc_petra-B, Arad
   *  in Arad: dist_tree_nick is used as destination nick
   */
  SOC_PPC_FEC_TYPE_TRILL_UC = 4,
  /*
   *  The FEC entry is used for Layer2 Forwarding. The
   *  application may be VPLS / MAC-In-MAC / TRILL, when the
   *  forwarding is according to MACT. When forwarding to TRILL
   *  distribution TREE, the type should be
   *  SOC_PPC_FEC_TYPE_TRILL_MC.
   *  relevant: Soc_petra-B
   *  for Arad use tunneling instead
   */
  SOC_PPC_FEC_TYPE_BRIDGING_INTO_TUNNEL = 5,
  /*
   *  The FEC entry is used for Layer2 Forwarding. The
   *  application may be Bridging with protected-AC. no
   *  tunneling in this case
   *  relevant: Soc_petra-B
   *  for Arad use tunneling instead
   */
  SOC_PPC_FEC_TYPE_BRIDGING_WITH_AC = 6,
  /*
   *  This FEC entry is used for MPLS LSR Switching.
   *  relevant: Soc_petra-B
   *  for Arad use routing instead
   */
  SOC_PPC_FEC_TYPE_MPLS_LSR = 7,
  /*
   *  Number of types in SOC_PPC_FEC_TYPE
   */
  SOC_PPC_NOF_FEC_TYPES_PB = 8,
  /*
   *  the FEC includes destination only.
   */
  SOC_PPC_FEC_TYPE_SIMPLE_DEST = SOC_PPC_NOF_FEC_TYPES_PB,
  /*
   *  FEC is used for Format C,
   *  meaning the EEDB pointer will be passed
   *  inside the EEI.
   *  EEI encoded as {pointer,Out-LIF}
   */
  SOC_PPC_FEC_TYPE_MPLS_LSR_EEI_OUTLIF,
  /*
   *  FEC is used for routing (IP/MPLS),
   *  FEC includes destination, eep (out-lif), out-RIF
   *  used for IP/MPLS routing
   */
  SOC_PPC_FEC_TYPE_ROUTING,
  /*
   *  FEC is used for tunneling,
   *  FEC includes destination and out-lif
   *  used for IP/MPLS/Briding into tunnel
   *  used also for bridging with out-AC
   */
  SOC_PPC_FEC_TYPE_TUNNELING,
  /*
   *  FEC is used as a forwarding group
   *  contains destination and Out-LIF
   */
  SOC_PPC_FEC_TYPE_FORWARD,
  /*
   *  FEC is used for tunneling,
   *  but in this case we want format C,
   *  meaning the EEDB pointer will be passed
   *  inside the EEI.
   *  EEI encoded as {pointer,Out-LIF}
   */
  SOC_PPC_FEC_TYPE_TUNNELING_EEI_OUTLIF,
  /*
   *  FEC is used for tunneling,
   *  but in this case we want format C,
   *  meaning the EEDB pointer will be passed
   *  inside the EEI.
   *  EEI encoded as {pointer, mpls_command}
   */
  SOC_PPC_FEC_TYPE_TUNNELING_EEI_MPLS_COMMAND,
  /*
   *  Number of types in SOC_PPC_FEC_TYPE
   */
  SOC_PPC_NOF_FEC_TYPES_ARAD

}SOC_PPC_FEC_TYPE;

typedef enum
{
  /*
   *  No RPF check is performed. Should be aligned with the
   *  settings of 'uc_rpf_enable' via soc_ppd_rif_vsid_map_set().
   *  Relevant for: Soc_petra-B, Arad.
   */
  SOC_PPC_FRWRD_FEC_RPF_MODE_NONE = 0x0,
  /*
   *  SIP lookup must succeed. But Out-RIF must not to be
   *  equal to In-RIF. SIP lookup Success means that the SIP
   *  lookup did not return the default FEC entry. Relevant
   *  only when the FEC entry is used for IPv4-UC routing without
   *  VPN (VRF = 0).
   *  if fail packet is trapped/snooped according to
   *  SOC_PPC_TRAP_CODE_UC_LOOSE_RPF_FAIL trap code
   *  Relevant for: Soc_petra-B, Arad.
   */
  SOC_PPC_FRWRD_FEC_RPF_MODE_UC_LOOSE = 0x1,
  /*
   *  SIP lookup must succeed and In-RIF must be equal to
   *  Out-RIF from SIP lookup.
   *  Relevant for: Soc_petra-B, Arad.
   */
  SOC_PPC_FRWRD_FEC_RPF_MODE_UC_STRICT = 0x2,  
  /*
   *  Packet In-RIF is compared against Expected-In-RIF
   *  explicitly specified in the FEC entry.
   *  if fail packet is trapped/snooped according to
   *  SOC_PPC_TRAP_CODE_MC_EXPLICIT_RPF_FAIL trap code
   *  Relevant for: Soc_petra-B, Arad.
   */
  SOC_PPC_FRWRD_FEC_RPF_MODE_MC_EXPLICIT = 0x4,
  /*
   *  Lookup the SIP of the packet. Same as UC RPF. The SIP's
   *  FEC Out-RIF is compared to the In-RIF that the packet
   *  arrives with. When The SIP points to an ECMP
   *  packet is trapped according to SOC_PPC_TRAP_CODE_MC_USE_SIP_RPF_FAIL
   *  if fail packet is trapped/snooped according to
   *  SOC_PPC_TRAP_CODE_MC_USE_SIP_RPF_FAIL trap code
   *  or SOC_PPC_TRAP_CODE_MC_USE_SIP_ECMP if SIP lookup returned ECMP
   *  Relevant for: Soc_petra-B.
   */
  SOC_PPC_FRWRD_FEC_RPF_MODE_MC_USE_SIP = 0x8,
  /*
   *  Lookup the SIP of the packet. Same as UC RPF. The SIP's
   *  FEC Out-RIF is compared to the In-RIF that the packet
   *  arrives with. When The SIP points to an ECMP, the In-RIF
   *  compared with the Out-RIF of the first ECMP's FEC Entry.
   *  ,and in this case packet can be trapped/snooped according
   *  to the trap code SOC_PPC_TRAP_CODE_MC_USE_SIP_ECMP
   *  if fail packet is trapped/snooped according to
   *  SOC_PPC_TRAP_CODE_MC_USE_SIP_AS_IS_RPF_FAIL trap code
   *  Relevant for: Soc_petra-B, Arad.
   */
  SOC_PPC_FRWRD_FEC_RPF_MODE_MC_USE_SIP_WITH_ECMP = 0x10,
  /*
   *  Number of types in SOC_PPC_FRWRD_FEC_RPF_MODE
   */
  SOC_PPC_NOF_FRWRD_FEC_RPF_MODES = 6
}SOC_PPC_FRWRD_FEC_RPF_MODE;

typedef enum
{
  /*
   *  No protection for the FEC entry.
   */
  SOC_PPC_FRWRD_FEC_PROTECT_TYPE_NONE = 0,
  /*
   *  Facility Protection. In this case user has to supply
   *  Working FEC and Protecting FEC. Packets are forwarded
   *  according to: 1. Working FEC: if the destination of this
   *  FEC is System Port and it status is up. 2. (Otherwise)
   *  Protection FEC: if the destination of this FEC is System
   *  Port and it status is up. 3. (Otherwise) packet may be
   *  forwarded/snooped according to action profile. To set
   *  this the forwarding and snooping actions use
   *  soc_ppd_trap_frwrd_profile_info_set(),soc_ppd_trap_snoop_profile_info_set()
   *  with trap code SOC_PPC_TRAP_CODE_FACILITY_INVALIDIn order to
   *  set the status of the system port use
   *  soc_ppd_frwrd_fec_protection_sys_port_status_set(system_port,
   *  is_up)
   */
  SOC_PPC_FRWRD_FEC_PROTECT_TYPE_FACILITY = 1,
  /*
   *  Path Protection. In this case user has to supply Working
   *  FEC, Protecting FEC and OAM instance. Packets are
   *  forwarded according to Working FEC if the status of the
   *  OAM instance is up, and according to the Protection FEC
   *  otherwise. In order to set the status of the OAM instance
   *  use
   *  soc_ppd_frwrd_fec_protection_oam_instance_status_set(oam_id,
   *  is_up)
   */
  SOC_PPC_FRWRD_FEC_PROTECT_TYPE_PATH = 2,
  /*
   *  Number of types in SOC_PPC_FRWRD_FEC_PROTECT_TYPE
   */
  SOC_PPC_NOF_FRWRD_FEC_PROTECT_TYPES = 3
}SOC_PPC_FRWRD_FEC_PROTECT_TYPE;

typedef enum
{
  /*
   *  The pointed FEC is not allocated for any use.
   */
  SOC_PPC_FRWRD_FEC_ENTRY_USE_TYPE_NONE = 0,
  /*
   *  The pointed FEC is allocated as one FEC entry (no
   *  protection).
   */
  SOC_PPC_FRWRD_FEC_ENTRY_USE_TYPE_ONE_FEC = 1,
  /*
   *  The pointed FEC is allocated as part of ECMP.
   */
  SOC_PPC_FRWRD_FEC_ENTRY_USE_TYPE_ECMP = 2,
  /*
   *  The pointed FEC is allocated as part of Path protection.
   */
  SOC_PPC_FRWRD_FEC_ENTRY_USE_TYPE_PATH_PROTECT = 3,
  /*
   *  The pointed FEC is allocated as part of Facility
   *  protection.
   */
  SOC_PPC_FRWRD_FEC_ENTRY_USE_TYPE_FACILITY_PROTECT = 4,
  /*
   *  The pointed FEC is allocated as part of Facility
   *  protection.
   */
  SOC_PPC_FRWRD_FEC_ENTRY_USE_TYPE_ECMP_PROTECTED = 5,
  /*
   *  Number of types in SOC_PPC_FRWRD_FEC_ENTRY_USE_TYPE
   */
  SOC_PPC_NOF_FRWRD_FEC_ENTRY_USE_TYPES = 6
}SOC_PPC_FRWRD_FEC_ENTRY_USE_TYPE;

typedef enum
{
  /*
   *  Get all valid (Allocated) FEC entries
   */
  SOC_PPC_FRWRD_FEC_MATCH_RULE_TYPE_ALL = 0,
  /*
   *  Returns all FEC entries of application type (Routing
   *  UC/MC, Bridging). Application type is specified by value
   *  - see SOC_PPC_FRWRD_FEC_MATCH_RULE.
   */
  SOC_PPC_FRWRD_FEC_MATCH_RULE_TYPE_APP_TYPE = 1,
  /*
   *  Get all FEC entries that have been accessed lately, i.e.
   *  the sticky bit is up from the last read.
   */
  SOC_PPC_FRWRD_FEC_MATCH_RULE_TYPE_ACCESSED = 2,
  /*
   *  Get all path protected FEC entries. According to its
   *  value, may return all FEC entries that use a specific
   *  OAM instance.
   */
  SOC_PPC_FRWRD_FEC_MATCH_RULE_TYPE_PATH_PROTECTED = 3,
  /*
   *  Get all facility protected FEC entries. According to its
   *  value, may return all FEC entries that use a specific
   *  system port.
   */
  SOC_PPC_FRWRD_FEC_MATCH_RULE_TYPE_FACILITY_PROTECTED = 4,
  /*
   *  Number of types in SOC_PPC_FRWRD_FEC_MATCH_RULE_TYPE
   */
  SOC_PPC_NOF_FRWRD_FEC_MATCH_RULE_TYPES = 5
}SOC_PPC_FRWRD_FEC_MATCH_RULE_TYPE;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  Array that includes the supported sizes of ECMPs. Range:
   *  1-288.16 ECMP sizes are supported. The sizes are between
   *  1 and 288. Soc_petra-B: ecmp_sizes[0] has to be 1. This
   *  is the default obtained by _clear.
   */
  uint32 ecmp_sizes[SOC_DPP_DEFS_MAX(ECMP_MAX_SIZE)];
  /*
   *  Number of valid entries in ecmp_sizes. Range: 1 - 16.
   */
  uint32 ecmp_sizes_nof_entries;

} SOC_PPC_FRWRD_FEC_GLBL_INFO;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  RPF mode (explicit/SIP/SIP_AS_IS).
   *  Soc_petra-B:
   *  - relevant only for IP-UC/IP-MC FEC entry type
   *  - one mode can be used, has to be compatible with MC or UC
   *  Arad:
   *  - relevant for all FEC entries
   *  - will be used only for IP packets
   *  - per entry two modes can be used 1 for MC, and 1 for UC.
   */
  SOC_PPC_FRWRD_FEC_RPF_MODE rpf_mode;
  /*
   *  Expected In-RIF used in the MC RPF check
   */
  SOC_PPC_RIF_ID expected_in_rif;

} SOC_PPC_FRWRD_FEC_ENTRY_RPF_INFO;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  Attachment Circuit ID. For Bridging with ACRange:
   *  Soc_petraB: 0 - 16K-1. T20E
   */
  SOC_PPC_AC_ID out_ac_id;
  /*
   *  Out-RIF. For IP-UC, MPLS applications. For IP-MC out-RIF
   *  is set per copy from the Multicast Group.
   */
  SOC_PPC_RIF_ID out_rif;
  /*
   *  Dist-Tree-Nick for TRILL Multicast.
   *  for TRILL-unicast this is the dest-nick
   */
  uint32 dist_tree_nick;

} SOC_PPC_FRWRD_FEC_ENTRY_APP_INFO;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  The type of the FEC entry may be one of the following:
   *  IP UC/ MC, TRILL UC / MC, Bridging, MPLS LSR, Drop, or
   *  Trap.
   */
  SOC_PPC_FEC_TYPE type;
  /*
   *  Destination ID and destination Type. Cannot be
   *  trap/FEC/Router. For IP-MC has to be MULTICAST.
   */
  SOC_SAND_PP_DESTINATION_ID dest;
  /*
   *  Egress Encapsulation pointer. According to FEC
   *  Type:Bridging: Expected to point to Tunnel/ PWE / MinM
   *  interfaceIP UC to RIF-VSI: Expected to point to the
   *  link-layer next-hop information. IP UC to RIF-Tunnel:
   *  Expected to point to the Tunnel information. TRILL-UC:
   *  Expected to point to the link-layer next-hop
   *  information. MPLS LSR: In Soc_petraB - Expected to point to:
   *  An MPLS Tunnel, for Swap & Push. Next hop DA, for Swap.
   *  The rest of the LL encapsulation (SA and VID) is
   *  deduced from the Out-RIF assigned by this FEC entry (see
   *  app_info field)In T20E - Expected to point to: An MPLS
   *  Tunnel. If only Swap action is required, the indexed
   *  Tunnel should be null, providing only the index to the
   *  link-layer encapsulation information and the outgoing
   *  core VSID.
   *  Range 2-
   */
  uint32 eep;
  /*
   *  According to FEC Type:TRAP: TRAP infoTRILL MC:
   *  Distribution TREE. IP UC and MPLS LSR: Out-RIFBridging
   *  with AC: out-AC-idPetra-B only.
   */
  SOC_PPC_FRWRD_FEC_ENTRY_APP_INFO app_info;
  /*
   *  RPF type and information
   */
  SOC_PPC_FRWRD_FEC_ENTRY_RPF_INFO rpf_info;
  /*
   *  Trap information <cpu-code, forwarding strength, snoop
   *  strength>.relevant if 'dest' is of type trap.
   */
  SOC_PPC_ACTION_PROFILE trap_info;

} SOC_PPC_FRWRD_FEC_ENTRY_INFO;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  If entry is accessed and this set to TRUE, trap the
   *  packet with trap code SOC_PPC_TRAP_CODE_FEC_ENTRY_ACCESSED,
   *  and reset this field.
   *  Soc_petra-B: Can be set to first 64 entries only.
   *  Arad: Can be set to lsat 64 entries only.
   */
  uint8 trap_if_accessed;
  /*
   *  Indicates that entry was accessed and its content was
   *  used for forwarding a packet. This field is read only,
   *  and cleared on read.
   */
  uint8 accessed;

} SOC_PPC_FRWRD_FEC_ENTRY_ACCESSED_INFO;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  OAM instance ID. Relvant for PATH protected FECs. If the
   *  OAM instance is TRUE (UP), then packets will be
   *  forwarded according to working FEC; otherwise, packets
   *  will be forwarded according to protect FEC. See
   *  soc_ppd_frwrd_fec_protection_oam_instance_status_set() to
   *  configure the status of the OAM instance. Range:
   *  Soc_petra-B:0-4K-1, T20E:0-16K-1.
   */
  uint32 oam_instance_id;

} SOC_PPC_FRWRD_FEC_PROTECT_INFO;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   * size of ECMP. 
   * number of FEC entries in ECMP.
   * working and protected FEC consider as 2 entries
   * Arad: Range: 1-511
   */
  uint32 size;
  /*
   *  pointer to the first FEC in the ECMP group 
   *  Arad: Range: 0 - 32K-1
   *  can be TRUE only if start_from_bos is TRUE
   */
  uint32 base_fec_id;
  /*
   *  if TRUE, then each FEC-Entry in the ECMP group 
   *  is protected.
   */
  uint8 is_protected;
  /*
   *  the FEC entry to use for RPF check.
   *  select one of the ECMP members.
   * Arad: Range: 1-511
   */
  uint32 rpf_fec_index;
#ifdef BCM_88660_A0
  /*
   * Is the load balancing stateful? 
   * 0/1. 
   */
  uint8 is_stateful;
#endif /* BCM_88660_A0 */
} SOC_PPC_FRWRD_FEC_ECMP_INFO;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  The use of the FEC (none/one FEC/ECMP/path or facility
   *  protection)
   */
  SOC_PPC_FRWRD_FEC_ENTRY_USE_TYPE type;
  /*
   *  According to type:- for none MUST be 0- for one FEC MUST
   *  be 1- for protection MUST be 2- for ECMP the size of the
   *  ECMP
   */
  uint32 nof_entries;

} SOC_PPC_FRWRD_FEC_ENTRY_USE_INFO;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  The use of the FEC (none/one FEC/ECMP/path or facility
   *  protection)
   */
  SOC_PPC_FRWRD_FEC_MATCH_RULE_TYPE type;
  /*
   *  A value that is compared with the FEC entry
   *  contents. According to type:- path_protect, then this is
   *  the OAM instance- facility_protect, then this is the
   *  system port- app_type, then this is the application
   *  type- accessed/trap_if_accessed ignored
   */
  uint32 value;

} SOC_PPC_FRWRD_FEC_MATCH_RULE;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  If set the In-PP-Port is used in the CRC vectors
   */
  uint8 use_port_id;
  /*
   *  Initial value for the LB key generation
   */
  uint16 seed;
  /*
   *  Selects one of the following options for the LB key
   *  generation:
   *  0 - Use polynomial 0x17111
   *  1 - Use polynomial 0x10491
   *  2 - Use polynomial 0x155f5
   *  3 - Use polynomial 0x19715
   *  4 - Use polynomial 0x13965
   *  5 - Use polynomial 0x1698d
   *  6 - Use polynomial 0x1105d
   *  7 - Use LB-Key-Packet-Data directly
   *  8 - Use counter incremented every packet
   *  9 - User counter incremented every two clocks
   *  10 - Use polynomial 0x10861 
   *  11 - Use polynomial 0x10285 
   *  12 - Use polynomial 0x101a1 
   *  13 - Use polynomial 0x12499 
   *  14 - se polynomial 0x1f801  
   *  15 - Use polynomial 0x172e1 
   *  16 - Use polynomial 0x1eb21 
   *  Soc_petra-B supports 0-9.
   *  Arad Supports 4-16.
   *  in Arad: cannot use same function for ECMP and LAG hashing
   *  see SOC_PPC_FEC_LB_xxx
   */
  uint8 hash_func_id;
  /*
   * The second hierarchy polynomial selection
   */
  uint8 hash_func_id_second_hier;
  /*
   *  The load balancing key is barrel shifted by this value.
   */
  uint8 key_shift;
  /*
   *  The second hierarchy load balancing key is barrel shifted by this value.
   */
  uint8 key_shift_second_hier;


} SOC_PPC_FRWRD_FEC_ECMP_HASH_GLOBAL_INFO;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  Number of headers to parse. Range: 1-2
   */
  uint8 nof_headers;
  /*
   *  if header type is mpls,
   *  start hashing from BOS label
   */
  uint8 start_from_bos;
  /*
   *  if header type is mpls,
   *  start hashing from header below BOS label
   *  can be TRUE only if start_from_bos is TRUE
   */
  uint8 skip_bos;


} SOC_PPC_FRWRD_FEC_ECMP_HASH_PORT_INFO;


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
  SOC_PPC_FRWRD_FEC_GLBL_INFO_clear(
    SOC_SAND_IN  int                         unit,
    SOC_SAND_OUT SOC_PPC_FRWRD_FEC_GLBL_INFO *info
  );

void
  SOC_PPC_FRWRD_FEC_ENTRY_RPF_INFO_clear(
    SOC_SAND_OUT SOC_PPC_FRWRD_FEC_ENTRY_RPF_INFO *info
  );

void
  SOC_PPC_FRWRD_FEC_ENTRY_APP_INFO_clear(
    SOC_SAND_OUT SOC_PPC_FRWRD_FEC_ENTRY_APP_INFO *info
  );

void
  SOC_PPC_FRWRD_FEC_ENTRY_INFO_clear(
    SOC_SAND_OUT SOC_PPC_FRWRD_FEC_ENTRY_INFO *info
  );

void
  SOC_PPC_FRWRD_FEC_ENTRY_ACCESSED_INFO_clear(
    SOC_SAND_OUT SOC_PPC_FRWRD_FEC_ENTRY_ACCESSED_INFO *info
  );

void
  SOC_PPC_FRWRD_FEC_PROTECT_INFO_clear(
    SOC_SAND_OUT SOC_PPC_FRWRD_FEC_PROTECT_INFO *info
  );

void
  SOC_PPC_FRWRD_FEC_ECMP_INFO_clear(
    SOC_SAND_OUT SOC_PPC_FRWRD_FEC_ECMP_INFO *info
  );

void
  SOC_PPC_FRWRD_FEC_ENTRY_USE_INFO_clear(
    SOC_SAND_OUT SOC_PPC_FRWRD_FEC_ENTRY_USE_INFO *info
  );

void
  SOC_PPC_FRWRD_FEC_MATCH_RULE_clear(
    SOC_SAND_OUT SOC_PPC_FRWRD_FEC_MATCH_RULE *info
  );

void
  SOC_PPC_FRWRD_FEC_ECMP_HASH_GLOBAL_INFO_clear(
    SOC_SAND_OUT SOC_PPC_FRWRD_FEC_ECMP_HASH_GLOBAL_INFO *info
  );

void
  SOC_PPC_FRWRD_FEC_ECMP_HASH_PORT_INFO_clear(
    SOC_SAND_OUT SOC_PPC_FRWRD_FEC_ECMP_HASH_PORT_INFO *info
  );

#if SOC_PPC_DEBUG_IS_LVL1

const char*
  SOC_PPC_FEC_TYPE_to_string(
    SOC_SAND_IN  SOC_PPC_FEC_TYPE enum_val
  );

const char*
  SOC_PPC_FEC_TYPE_to_string_short(
    SOC_SAND_IN  SOC_PPC_FEC_TYPE enum_val
  );

const char*
  SOC_PPC_FRWRD_FEC_RPF_MODE_to_string(
    SOC_SAND_IN  SOC_PPC_FRWRD_FEC_RPF_MODE enum_val
  );

const char*
  SOC_PPC_FRWRD_FEC_PROTECT_TYPE_to_string(
    SOC_SAND_IN  SOC_PPC_FRWRD_FEC_PROTECT_TYPE enum_val
  );

const char*
  SOC_PPC_FRWRD_FEC_ENTRY_USE_TYPE_to_string(
    SOC_SAND_IN  SOC_PPC_FRWRD_FEC_ENTRY_USE_TYPE enum_val
  );

const char*
  SOC_PPC_FRWRD_FEC_MATCH_RULE_TYPE_to_string(
    SOC_SAND_IN  SOC_PPC_FRWRD_FEC_MATCH_RULE_TYPE enum_val
  );

void
  SOC_PPC_FRWRD_FEC_GLBL_INFO_print(
    SOC_SAND_IN  int                         unit,
    SOC_SAND_IN  SOC_PPC_FRWRD_FEC_GLBL_INFO *info
  );

void
  SOC_PPC_FRWRD_FEC_ENTRY_RPF_INFO_print(
    SOC_SAND_IN  SOC_PPC_FRWRD_FEC_ENTRY_RPF_INFO *info
  );

void
  SOC_PPC_FRWRD_FEC_ENTRY_APP_INFO_print(
    SOC_SAND_IN  SOC_PPC_FRWRD_FEC_ENTRY_APP_INFO *info
  );

void
  SOC_PPC_FRWRD_FEC_ENTRY_INFO_print(
    SOC_SAND_IN  SOC_PPC_FRWRD_FEC_ENTRY_INFO *info
  );

void
  SOC_PPC_FRWRD_FEC_ENTRY_ACCESSED_INFO_print(
    SOC_SAND_IN  SOC_PPC_FRWRD_FEC_ENTRY_ACCESSED_INFO *info
  );

void
  SOC_PPC_FRWRD_FEC_PROTECT_INFO_print(
    SOC_SAND_IN  SOC_PPC_FRWRD_FEC_PROTECT_INFO *info
  );

void
  SOC_PPC_FRWRD_FEC_ECMP_INFO_print(
    SOC_SAND_IN  SOC_PPC_FRWRD_FEC_ECMP_INFO *info
  );

void
  SOC_PPC_FRWRD_FEC_ENTRY_USE_INFO_print(
    SOC_SAND_IN  SOC_PPC_FRWRD_FEC_ENTRY_USE_INFO *info
  );

void
  SOC_PPC_FRWRD_FEC_MATCH_RULE_print(
    SOC_SAND_IN  SOC_PPC_FRWRD_FEC_MATCH_RULE *info
  );

void
  SOC_PPC_FRWRD_FEC_ECMP_HASH_GLOBAL_INFO_print(
    SOC_SAND_IN  SOC_PPC_FRWRD_FEC_ECMP_HASH_GLOBAL_INFO *info
  );

void
  SOC_PPC_FRWRD_FEC_ECMP_HASH_PORT_INFO_print(
    SOC_SAND_IN  SOC_PPC_FRWRD_FEC_ECMP_HASH_PORT_INFO *info
  );

#endif /* SOC_PPC_DEBUG_IS_LVL1 */

/* } */

#include <soc/dpp/SAND/Utils/sand_footer.h>

/* } __SOC_PPC_API_FRWRD_FEC_INCLUDED__*/
#endif
