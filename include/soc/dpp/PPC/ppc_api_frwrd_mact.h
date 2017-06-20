/* $Id: ppc_api_frwrd_mact.h,v 1.14 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/
/******************************************************************
*
* FILENAME:       DuneDriver/ppc/include/soc_ppc_api_frwrd_mact.h
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

#ifndef __SOC_PPC_API_FRWRD_MACT_INCLUDED__
/* { */
#define __SOC_PPC_API_FRWRD_MACT_INCLUDED__

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

/*     Indicates iterator over MAC table reached end of the
 *     table.                                                  */
#define  SOC_PPC_FRWRD_MACT_ITER_END (0xFFFFFFFF)

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
  SOC_PPC_FRWRD_MACT_TABLE_SW_ONLY,
  SOC_PPC_FRWRD_MACT_TABLE_SW_HW,
 
  /*
   * Last element. Do no touch.
   */
  SOC_PPC_FRWRD_MACT_TABLE_TYPE_LAST
} SOC_PPC_FRWRD_MACT_TABLE_TYPE;

typedef enum
{
  /* The application ID of oMAC prefix value */
  SOC_PPC_FRWRD_MACT_PREFIX_APP_ID_OMAC = 0,
  /* The application ID of oMAC2vMAC prefix value */
  SOC_PPC_FRWRD_MACT_PREFIX_APP_ID_OMAC_2_VMAC,
  /* The application ID of vMAC prefix value */
  SOC_PPC_FRWRD_MACT_PREFIX_APP_ID_VMAC,
  /* The application ID of vMAC2oMAC prefix value */
  SOC_PPC_FRWRD_MACT_PREFIX_APP_ID_VMAC_2_OMAC,
  /* Number of types in SOC_PPC_FRWRD_MACT_APP_ID*/
  SOC_PPC_NOF_FRWRD_MACT_PREFIX_APP_IDS
}SOC_PPC_FRWRD_MACT_APP_ID;


typedef enum
{
  /*
   *  The MACT is access with the key composed of MACT and
   *  FID, (normal bridging).
   */
  SOC_PPC_FRWRD_MACT_KEY_TYPE_MAC_ADDR = 0,
  /*
   *  The MACT is access with the key composed of IPv4 DIP and
   *  FID; used for IPv4 compatible MC packets when MC routing
   *  is disabled (see soc_ppd_rif_vsid_map_set()) and
   *  ipv4_compatible is enabled (see
   *  soc_ppd_frwrd_mact_ip_compatible_mc_info_set()). In
   *  this case packets are bridged where the lookup key into
   *  the MACT is DIP and FID. FID can be masked. see
   *  soc_ppd_frwrd_mact_ip_compatible_mc_info_set()
   */
  SOC_PPC_FRWRD_MACT_KEY_TYPE_IPV4_MC = 1,

#ifdef BCM_88660_A0
  /*
   * The MACT is accessed with the key composed of
   *  
   * |---- 8b ---||-------- 1b ----------||---- 15b(*) ---||---- 48b ----|
   * |--Padding--||--Is_Destination_Fec--||--Destination--||--Flow_Label-|
   *  
   * Destination - LAG/ECMP. 
   * Is_Destination_Fec - 1 for ECMP, 0 for LAG. 
   * Flow_Label - Hash key for traffic to get the member from. 
   *  
   */
  SOC_PPC_FRWRD_MACT_KEY_TYPE_SLB = 2,

  /*
   *  Number of types in SOC_PPC_FRWRD_MACT_KEY_TYPE
   */
  SOC_PPC_NOF_FRWRD_MACT_KEY_TYPES = 3
#else 
  SOC_PPC_NOF_FRWRD_MACT_KEY_TYPES = 2
#endif /* BCM_88660_A0 */


}SOC_PPC_FRWRD_MACT_KEY_TYPE;

typedef enum
{
  /*
   *  Inserts an entry according to its key. If in the MAC
   *  table there is already an entry with this key, it is
   *  (always) overwritten by this value.
   */
  SOC_PPC_FRWRD_MACT_ADD_TYPE_INSERT = 0,
  /*
   *  Learns an entry for a given key and value. If in the MAC
   *  table there is already a non static entry with this key,
   *  it is overwritten. If the entry in the MACT is static
   *  and the entry to be learned (value) is dynamic, then no
   *  change will be done for the existing entry. However, if
   *  both are static, the new value overwrites the existing
   *  one.
   */
  SOC_PPC_FRWRD_MACT_ADD_TYPE_LEARN = 1,
  /*
   *  Same as learn (above), with the exception that static
   *  entries are never overridden by the refresh command.
   */
  SOC_PPC_FRWRD_MACT_ADD_TYPE_REFRESH = 2,
  /*
   *  Also called move. Change the payload of an existing key
   */
  SOC_PPC_FRWRD_MACT_ADD_TYPE_TRANSPLANT = 3,
  /*
   *  Number of types in SOC_PPC_FRWRD_MACT_ADD_TYPE
   */
  SOC_PPC_NOF_FRWRD_MACT_ADD_TYPES = 4
}SOC_PPC_FRWRD_MACT_ADD_TYPE;

typedef enum
{
  /*
   *  None: No affect to the MAC Table entries. May be useful
   *  to count entries of the MAC table that match a given
   *  rule.
   */
  SOC_PPC_FRWRD_MACT_TRAVERSE_ACTION_TYPE_NONE = 0,
  /*
   *  Remove matched entries from the MAC Table.
   */
  SOC_PPC_FRWRD_MACT_TRAVERSE_ACTION_TYPE_REMOVE = 1,
  /*
   *  Modify the content of matched entries.
   */
  SOC_PPC_FRWRD_MACT_TRAVERSE_ACTION_TYPE_UPDATE = 2,
  /*
   *  To count entries of the MAC table that match a given
   *  rule.
   */
  SOC_PPC_FRWRD_MACT_TRAVERSE_ACTION_TYPE_COUNT = 3,
  /*
   *  Retrieve matched entries into the MACT event FIFO. To
   *  read entries from the event FIFO use
   *  soc_ppd_trap_mact_event_get()/
   *  soc_ppd_trap_mact_event_parse(). Note: the user may also read
   *  MACT content using soc_ppd_frwrd_mact_get_block(). T20E
   *  only.
   */
  SOC_PPC_FRWRD_MACT_TRAVERSE_ACTION_TYPE_RETRIEVE = 4,
  /*
   *  Number of types in SOC_PPC_FRWRD_MACT_TRAVERSE_ACTION_TYPE
   */
  SOC_PPC_NOF_FRWRD_MACT_TRAVERSE_ACTION_TYPES = 5
}SOC_PPC_FRWRD_MACT_TRAVERSE_ACTION_TYPE;

typedef enum
{
  /*
   *  Compare/replace NONE of the fields. All entries will be
   *  matched/replaced.
   */
  SOC_PPC_FRWRD_MACT_TRAVERSE_MATCH_SELECT_NONE = 0,
  /*
   *  Compare/replace Destination type, see
   *  SOC_PPC_FRWRD_DECISION_TYPE
   */
  SOC_PPC_FRWRD_MACT_TRAVERSE_MATCH_SELECT_DEST_TYPE = (1 << 1),
  /*
   *  Compare/replace Destination value, see dest_id at
   *  SOC_PPC_FRWRD_DECISION_INFO
   */
  SOC_PPC_FRWRD_MACT_TRAVERSE_MATCH_SELECT_DEST_VAL = (1 << 2),
  /*
   *  Compare/replace the additional information, see
   *  additional_info at SOC_PPC_FRWRD_DECISION_INFO.
   */
  SOC_PPC_FRWRD_MACT_TRAVERSE_MATCH_SELECT_ADDITIONAL_INFO = (1 << 3),
  /*
   *  Compare/replace is dynamic field.
   */
  SOC_PPC_FRWRD_MACT_TRAVERSE_MATCH_SELECT_IS_DYNAMIC = (1 << 4),
  /*
   *  Compare/replace drop_when_sa_is_known field.
   */
  SOC_PPC_FRWRD_MACT_TRAVERSE_MATCH_SELECT_DROP_SA = (1 << 5),
  /*
   *  Compare/replace ALL fields, supported on device
   *  REMARK - There may be unexpected results for this, as
   *  some TRAVERSE_MATCH_SELECTS are unrelated to each other.
   *  It is recommended to ask for the required types explicitly
   *  (i.e. A | B).
   */
  SOC_PPC_FRWRD_MACT_TRAVERSE_MATCH_SELECT_ALL = (int)0xFFFFFFFF,
  /*
   *  Number of types in SOC_PPC_FRWRD_MACT_TRAVERSE_MATCH_SELECT (PB)
   */
  SOC_PPC_NOF_FRWRD_MACT_TRAVERSE_MATCH_SELECTS_PB = 7,
  /*
   *  Compare/replace accessed (for SA/DA lookup)
   *  in match rule to act on accessed or non-accessed entries
   *  in action if present then accessed bit will be cleared.
   */
  SOC_PPC_FRWRD_MACT_TRAVERSE_MATCH_SELECT_ACCESSED = (1 << 6),
  /*
   *  Compare/replace group
   *  in match rule to act on entries with given group
   *  in action if present then group will be changed to give value.
   */
  SOC_PPC_FRWRD_MACT_TRAVERSE_MATCH_SELECT_GROUP = (1 << 7),
#ifdef BCM_88660_A0
  /*
   * Compare SLB entries. 
   */
  SOC_PPC_FRWRD_MACT_TRAVERSE_MATCH_SELECT_COMPARE_SLB = (1 << 8),
  /*
   * Replace SLB entries. 
   */
  SOC_PPC_FRWRD_MACT_TRAVERSE_MATCH_SELECT_REPLACE_SLB = (1 << 9),
#endif /* BCM_88660_A0 */

  /*
   *  Number of types in SOC_PPC_FRWRD_MACT_TRAVERSE_MATCH_SELECT (ARAD)
   */
 SOC_PPC_NOF_FRWRD_MACT_TRAVERSE_MATCH_SELECTS_ARAD

}SOC_PPC_FRWRD_MACT_TRAVERSE_MATCH_SELECT;

typedef enum
{
  /*
   *  when call traverse function, then the call will
   *  - set traverse rule.
   *  - activate flush machine.
   *  - [wait-till finish and clear rule]
   */
  SOC_PPC_FRWRD_MACT_TRAVERSE_STATE_NORMAL = 0,
  /*
   *  when call traverse function, then the call will
   *  - set traverse rule in next available place if exist
   */
  SOC_PPC_FRWRD_MACT_TRAVERSE_STATE_AGGREGATE = 1,
  /*
   *  run traverse with exist rules, only for aggregate mode
   */
  SOC_PPC_FRWRD_MACT_TRAVERSE_STATE_RUN = 2,
  /*
   *  clean rules
   */
  SOC_PPC_FRWRD_MACT_TRAVERSE_STATE_RESET = 3,
  /*
   *  Number of types in SOC_PPC_FRWRD_MACT_TRAVERSE_MODE (ARAD)
   */
 SOC_PPC_NOF_FRWRD_MACT_TRAVERSE_STATES

}SOC_PPC_FRWRD_MACT_TRAVERSE_STATE;


typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  MAC address
   */
  SOC_SAND_PP_MAC_ADDRESS mac;
  /*
   *  Filtering ID. Set according to VSI. See
   *  soc_ppd_frwrd_mact_fid_profile_to_fid_map_set()Range:
   *  Soc_petra: 0 - 16K-1. T20E: 0 - 64K-1
   */
  SOC_PPC_FID fid;

} SOC_PPC_FRWRD_MACT_ENTRY_KEY_MAC_ADDR;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  Packet DIP (Destination IP address). Address has to be
   *  IPv4 MC address i.e. dip[31:28] = 0xE.
   */
  uint32 dip;
  /*
   *  Filtering ID. Set according to VSI. See
   *  soc_ppd_frwrd_mact_fid_profile_to_fid_map_set(). The fid
   *  may be globally masked. See
   *  soc_ppd_frwrd_mact_ip_compatible_mc_info_set(), if fid
   *  is masked, then fid value has to be zero. Range: Soc_petra: 0
   *  - 16K-1. T20E: 0 - 64K-1
   */
  SOC_PPC_FID fid;

} SOC_PPC_FRWRD_MACT_ENTRY_KEY_IPV4_MC;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  Is-destination-fec.
   *  If 1 then the destination specifies an ECMP group.
   *  Otherwise it specifies a LAG group.
   *  This is only semantic (in LEM it doesn't matter whether
   *  the destination is FEC or LAG group - both are numbers).
   */
  uint32 is_destination_fec;
  /*
   * The destination - ECMP or LAG. 
   * The LSB is ignored in flush match. (both values of the LSB (0/1) are matched).
   *  
   * Range: 
   *   Arad+: 15b.
   */
  uint32 destination;
  /*
   * The flow label. This field is ignored in the flush match. 
   *  
   * Range: 
   *   Arad+: 47b. 
   */
  uint32 flow_label[2];
} SOC_PPC_FRWRD_MACT_ENTRY_KEY_SLB;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  MAC address key
   */
  SOC_PPC_FRWRD_MACT_ENTRY_KEY_MAC_ADDR mac;
  /*
   *  IPv4 MC key
   */
  SOC_PPC_FRWRD_MACT_ENTRY_KEY_IPV4_MC ipv4_mc;
#ifdef BCM_88660_A0
  /*
   * SLB key
   */
  SOC_PPC_FRWRD_MACT_ENTRY_KEY_SLB slb;
#endif /* BCM_88660_A0 */

} SOC_PPC_FRWRD_MACT_ENTRY_KEY_VAL;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  MACT key type (MAC address and FID (normal bridging), or
   *  DIF and FID (compatible MC packets bridging)
   */
  SOC_PPC_FRWRD_MACT_KEY_TYPE key_type;
  /*
   *  MACT value, <MACT,FID> (normal bridging) or <DIP,FID>
   *  (IPv4 compatible MC) or <Is-Destination-FEC,Destination,Flow-Label> (Stateful load balancing)
   *  
   */
  SOC_PPC_FRWRD_MACT_ENTRY_KEY_VAL key_val;

} SOC_PPC_FRWRD_MACT_ENTRY_KEY;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  Is the entry dynamic (i.e. not static) and participates
   *  in the aging process.
   */
  uint8 is_dynamic;
  /*
   *  The age status of the entry used for Aging and indicates
   *  how fresh is the entry. Higher value indicates a more
   *  fresh recent entry. ForDuring an insertion -
   *  soc_ppd_frwrd_mact_entry_add() - may beif ignored then the
   *  maximum value is set. For During get a lookup -
   *  soc_ppd_frwrd_mact_entry_get() - return the age status is
   *  returned, and an age_status = 0 indicates that the entry
   *  is aged out but not deleted. Range: (Soc_petra-B) 0 - 6.
   */
  uint8 age_status;

} SOC_PPC_FRWRD_MACT_ENTRY_AGING_INFO;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  Forwarding decision
   */
  SOC_PPC_FRWRD_DECISION_INFO forward_decision;
  /*
   *  Soc_petra-B only. Has to be FALSE for T20E. If TRUE - For an
   *  SA lookup match, then the packet is forwarded according
   *  to a special action profile - see
   *  soc_ppd_frwrd_mact_trap_info_set(). If FALSE - SA
   *  lookup match does not affect forwarding. Drop according
   *  to SA, is done by the device in one of the following:1.
   *  SA-based VID-assignment is activated.2. Device performs
   *  Ingress Learning. Use soc_ppd_frwrd_mact_port_info_set
   *  to set the action to perform when such an SA is
   *  encountered.
   */
  uint8 drop_when_sa_is_known;

} SOC_PPC_FRWRD_MACT_ENTRY_FRWRD_INFO;

typedef struct {
  SOC_SAND_MAGIC_NUM_VAR
  
  /* 
   * [COMPARE] 
   *   There are 4 types of entries - {FEC,LAG} {FEC,XXX} {XXX,LAG} and {XXX,XXX} (XXX - ignore).
   *   match_X_entries means that entries matching this format will be matched. May be masked (0 - ignore bit).
   *
   *   If mask.match_X_entries is 0 or if match_X_entries is 0, then X must be ignored (X's mask = 0). 
   *   In other words, X entries may be matched even without mask data. 
   *   But if X entries should not be matched (or if they may be matched), then the X data must be ignored. 
   *
   *   At least one match_X_entries must be valid and unmasked (match_X_entries = mask.match_X_entries = 1).
   * 
   * [REPLACE]
   *   For replace, this value is ignored.
   *
   * 1 bit (0/1).
   */ 
  uint8 match_lag_entries;
  uint8 match_fec_entries;

  /* 
   * [COMPARE/REPLACE] 
   * The lag group and member to search for (COMPARE) or replace with (REPLACE). MSB - group, LSB - member ({GROUP,MEMBER}). May be masked (0 - ignore bit). 
   * 15 bits.
   */
  uint32 lag_group_and_member;

  /* 
   * [COMPARE/REPLACE] 
   * The FEC to search for (COMPARE) or replace with (REPLACE). May be masked (0 - ignore bit). 
   * 15 bits.
   */
  uint32 fec;

} SOC_PPC_FRWRD_MACT_ENTRY_VALUE_SLB;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  The forwarding information of the entry.
   */
  SOC_PPC_FRWRD_MACT_ENTRY_FRWRD_INFO frwrd_info;
  /*
   *  The aging information of the entry.
   */
  SOC_PPC_FRWRD_MACT_ENTRY_AGING_INFO aging_info;
  /*
   *  whether this entry was accessed upon traffic lookup
   *  in update/set has to be zero.
   *  Arad only.
   */
  uint8 accessed;
  /*
   *  user defined field, user can set when adding static entry
   *  and later used in traverse as 
   *  Arad only.
   */
  uint32 group;
#ifdef BCM_88660_A0
  SOC_PPC_FRWRD_MACT_ENTRY_VALUE_SLB slb_info;
#endif /* BCM_88660_A0 */
} SOC_PPC_FRWRD_MACT_ENTRY_VALUE;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  Seconds
   */
  uint32 sec;
  /*
   *  Milliseconds. Range: 0 - 999.
   */
  uint32 mili_sec;

} SOC_PPC_FRWRD_MACT_TIME;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  FID.
   */
  SOC_PPC_FID fid;
  /*
   *  Mask over each bit of the FID. Indicates whether to
   *  compare it or to ignore it while traversing the MACT DB.
   *  Set bit to zero to mask (ignore) the corresponding bit
   *  in the FID.
   */
  SOC_PPC_FID fid_mask;
  /*
   *  MAC address to compare.
   */
  SOC_SAND_PP_MAC_ADDRESS mac;
  /*
   *  Mask over each bit of the MAC address. Indicates whether
   *  to compare it or to ignore it while traversing the MACT
   *  DB. Set bit to zero to mask (ignore) corresponding bit
   *  in the MAC-address. Relevant only for Soc_petra-B, in T20E
   *  has to be zero.
   */
  SOC_SAND_PP_MAC_ADDRESS mac_mask;

} SOC_PPC_FRWRD_MACT_TRAVERSE_MATCH_KEY_RULE_MAC;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  DIP.
   */
  uint32 dip;
  /*
   *  Mask over each bit of the DIP. Indicates whether to
   *  compare it or to ignore it while traversing the MACT DB.
   *  Set bit to zero to mask (ignore) the corresponding bit
   *  in the DIP.
   */
  uint32 dip_mask;
  /*
   *  FID.
   */
  SOC_PPC_FID fid;
  /*
   *  Mask over each bit of the FID. Indicates whether to
   *  compare it or to ignore it while traversing the MACT DB.
   *  Set bit to zero to mask (ignore) the corresponding bit
   *  in the FID.
   */
  SOC_PPC_FID fid_mask;

} SOC_PPC_FRWRD_MACT_TRAVERSE_MATCH_KEY_RULE_IPV4_MC;

typedef struct {
  SOC_SAND_MAGIC_NUM_VAR

  /* The values. */
  SOC_PPC_FRWRD_MACT_ENTRY_KEY_SLB value;
  /* 
   * The mask. Any bit that is zero in the mask is ignored in the search 
   * (both values are hit).
   */
  SOC_PPC_FRWRD_MACT_ENTRY_KEY_SLB mask;
} SOC_PPC_FRWRD_MACT_TRAVERSE_MATCH_KEY_RULE_SLB;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  Key rule for the MAC addresses
   */
  SOC_PPC_FRWRD_MACT_TRAVERSE_MATCH_KEY_RULE_MAC mac;
  /*
   *  Key rule for the IPv4 Compatible Multicast addresses
   */
  SOC_PPC_FRWRD_MACT_TRAVERSE_MATCH_KEY_RULE_IPV4_MC ipv4_mc;
#ifdef BCM_88660_A0
  /*
   *  Key rule for the SLB entries.
   */
  SOC_PPC_FRWRD_MACT_TRAVERSE_MATCH_KEY_RULE_SLB slb;
#endif /* BCM_88660_A0 */
} SOC_PPC_FRWRD_MACT_TRAVERSE_MATCH_KEY_RULE;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  MACT entries will be compared to this value depending on
   *  the compare_mask below.
   */
  SOC_PPC_FRWRD_MACT_ENTRY_VALUE val;
  /*
   *  
   *  bitmap mask for given value
   *  0 for ignore, 1 for consider
   */
  SOC_PPC_FRWRD_MACT_ENTRY_VALUE val_mask;
  /*
   *  Mask that indicates which fields of the entry are
   *  compared. See SOC_PPC_FRWRD_MACT_TRAVERSE_MATCH_SELECT.
   */
  uint32 compare_mask;

} SOC_PPC_FRWRD_MACT_TRAVERSE_MATCH_VALUE_RULE;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  MACT key type (MAC address and FID (normal bridging), or
   *  DIP and FID (compatible MC packets bridging)
   */
  SOC_PPC_FRWRD_MACT_KEY_TYPE key_type;
  /*
   *  Rule over the MACT entry key. Only entries whose keys
   *  match this key_rule will be considered.
   */
  SOC_PPC_FRWRD_MACT_TRAVERSE_MATCH_KEY_RULE key_rule;
  /*
   *  Rule over the MACT entry value. Only entries whose
   *  values match this value_rule will be considered.
   */
  SOC_PPC_FRWRD_MACT_TRAVERSE_MATCH_VALUE_RULE value_rule;

} SOC_PPC_FRWRD_MACT_TRAVERSE_MATCH_RULE;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  Remove or update matched entries.
   */
  SOC_PPC_FRWRD_MACT_TRAVERSE_ACTION_TYPE type;
  /*
   *  Matched entries will be updated to this value depending
   *  on update_mask below. Relevant only if the type is
   *  'update'.
   */
  SOC_PPC_FRWRD_MACT_ENTRY_VALUE updated_val;
  /*
   *  bitmap mask on updated value.
   *  relevant only when update_mask, indicate use-mask.
   *  Arad only.
   */
  SOC_PPC_FRWRD_MACT_ENTRY_VALUE updated_val_mask;
  /*
   *  Bitmap that indicates which fields to update for the
   *  matched entries. See
   *  SOC_PPC_FRWRD_MACT_TRAVERSE_MATCH_SELECT
   */
  uint32 update_mask;

} SOC_PPC_FRWRD_MACT_TRAVERSE_ACTION;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  Rule against whom the MACT entries are compared.
   */
  SOC_PPC_FRWRD_MACT_TRAVERSE_MATCH_RULE rule;
  /*
   *  Action performed over the matched entries.
   */
  SOC_PPC_FRWRD_MACT_TRAVERSE_ACTION action;
  /*
   *  Time needed to finish the current traverse operation. If
   *  there is no active traverse operation, then the time is
   *  equal to zero.
   */
  SOC_PPC_FRWRD_MACT_TIME time_to_finish;
  /*
   *  Number of entries in the MACT that matched the given
   *  rule. Valid only if time_to_finish is zero and the
   *  traverse operation was performed lately.
   */
  uint32 nof_matched_entries;

} SOC_PPC_FRWRD_MACT_TRAVERSE_STATUS_INFO;



typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  traverse mode
   */
  SOC_PPC_FRWRD_MACT_TRAVERSE_STATE state;
  /*
   *  Number of Traverse available entries
   *  in ARAD: traverse can act on up to 8 rules together
   *  ARAD only.
   *  relevant for get function
   */
  uint32 nof_available_rules;
} SOC_PPC_FRWRD_MACT_TRAVERSE_MODE_INFO;


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
  SOC_PPC_FRWRD_MACT_ENTRY_KEY_MAC_ADDR_clear(
    SOC_SAND_OUT SOC_PPC_FRWRD_MACT_ENTRY_KEY_MAC_ADDR *info
  );

void
  SOC_PPC_FRWRD_MACT_ENTRY_KEY_IPV4_MC_clear(
    SOC_SAND_OUT SOC_PPC_FRWRD_MACT_ENTRY_KEY_IPV4_MC *info
  );

void
  SOC_PPC_FRWRD_MACT_ENTRY_KEY_VAL_clear(
    SOC_SAND_OUT SOC_PPC_FRWRD_MACT_ENTRY_KEY_VAL *info
  );

void
  SOC_PPC_FRWRD_MACT_ENTRY_KEY_clear(
    SOC_SAND_OUT SOC_PPC_FRWRD_MACT_ENTRY_KEY *info
  );

void
  SOC_PPC_FRWRD_MACT_ENTRY_AGING_INFO_clear(
    SOC_SAND_OUT SOC_PPC_FRWRD_MACT_ENTRY_AGING_INFO *info
  );

void
  SOC_PPC_FRWRD_MACT_ENTRY_FRWRD_INFO_clear(
    SOC_SAND_OUT SOC_PPC_FRWRD_MACT_ENTRY_FRWRD_INFO *info
  );

void
  SOC_PPC_FRWRD_MACT_ENTRY_VALUE_clear(
    SOC_SAND_OUT SOC_PPC_FRWRD_MACT_ENTRY_VALUE *info
  );

void
  SOC_PPC_FRWRD_MACT_TIME_clear(
    SOC_SAND_OUT SOC_PPC_FRWRD_MACT_TIME *info
  );

void
  SOC_PPC_FRWRD_MACT_TRAVERSE_MATCH_KEY_RULE_MAC_clear(
    SOC_SAND_OUT SOC_PPC_FRWRD_MACT_TRAVERSE_MATCH_KEY_RULE_MAC *info
  );

void
  SOC_PPC_FRWRD_MACT_TRAVERSE_MATCH_KEY_RULE_IPV4_MC_clear(
    SOC_SAND_OUT SOC_PPC_FRWRD_MACT_TRAVERSE_MATCH_KEY_RULE_IPV4_MC *info
  );

void
  SOC_PPC_FRWRD_MACT_TRAVERSE_MATCH_KEY_RULE_clear(
    SOC_SAND_OUT SOC_PPC_FRWRD_MACT_TRAVERSE_MATCH_KEY_RULE *info
  );

void
  SOC_PPC_FRWRD_MACT_TRAVERSE_MATCH_VALUE_RULE_clear(
    SOC_SAND_OUT SOC_PPC_FRWRD_MACT_TRAVERSE_MATCH_VALUE_RULE *info
  );

void
  SOC_PPC_FRWRD_MACT_TRAVERSE_MATCH_RULE_clear(
    SOC_SAND_OUT SOC_PPC_FRWRD_MACT_TRAVERSE_MATCH_RULE *info
  );

void
  SOC_PPC_FRWRD_MACT_TRAVERSE_ACTION_clear(
    SOC_SAND_OUT SOC_PPC_FRWRD_MACT_TRAVERSE_ACTION *info
  );

void
  SOC_PPC_FRWRD_MACT_TRAVERSE_STATUS_INFO_clear(
    SOC_SAND_OUT SOC_PPC_FRWRD_MACT_TRAVERSE_STATUS_INFO *info
  );

void
  SOC_PPC_FRWRD_MACT_ENTRY_KEY_SLB_clear(
    SOC_SAND_OUT  SOC_PPC_FRWRD_MACT_ENTRY_KEY_SLB *info
  );

void
  SOC_PPC_FRWRD_MACT_ENTRY_VALUE_SLB_clear(
    SOC_SAND_OUT  SOC_PPC_FRWRD_MACT_ENTRY_VALUE_SLB *info
  );

void
  SOC_PPC_FRWRD_MACT_TRAVERSE_MATCH_KEY_RULE_SLB_clear(
    SOC_SAND_OUT  SOC_PPC_FRWRD_MACT_TRAVERSE_MATCH_KEY_RULE_SLB *info
  );

#if SOC_PPC_DEBUG_IS_LVL1

const char*
  SOC_PPC_FRWRD_MACT_KEY_TYPE_to_string(
    SOC_SAND_IN  SOC_PPC_FRWRD_MACT_KEY_TYPE enum_val
  );

const char*
  SOC_PPC_FRWRD_MACT_ADD_TYPE_to_string(
    SOC_SAND_IN  SOC_PPC_FRWRD_MACT_ADD_TYPE enum_val
  );

const char*
  SOC_PPC_FRWRD_MACT_TRAVERSE_ACTION_TYPE_to_string(
    SOC_SAND_IN  SOC_PPC_FRWRD_MACT_TRAVERSE_ACTION_TYPE enum_val
  );

const char*
  SOC_PPC_FRWRD_MACT_TRAVERSE_MATCH_SELECT_to_string(
    SOC_SAND_IN  SOC_PPC_FRWRD_MACT_TRAVERSE_MATCH_SELECT enum_val
  );

void
  SOC_PPC_FRWRD_MACT_ENTRY_KEY_MAC_ADDR_print(
    SOC_SAND_IN  SOC_PPC_FRWRD_MACT_ENTRY_KEY_MAC_ADDR *info
  );

void
  SOC_PPC_FRWRD_MACT_ENTRY_KEY_IPV4_MC_print(
    SOC_SAND_IN  SOC_PPC_FRWRD_MACT_ENTRY_KEY_IPV4_MC *info
  );

void
  SOC_PPC_FRWRD_MACT_ENTRY_KEY_VAL_print(
    SOC_SAND_IN  SOC_PPC_FRWRD_MACT_ENTRY_KEY_VAL *info
  );

void
  SOC_PPC_FRWRD_MACT_ENTRY_KEY_print(
    SOC_SAND_IN  SOC_PPC_FRWRD_MACT_ENTRY_KEY *info
  );

void
  SOC_PPC_FRWRD_MACT_ENTRY_AGING_INFO_print(
    SOC_SAND_IN  SOC_PPC_FRWRD_MACT_ENTRY_AGING_INFO *info
  );

void
  SOC_PPC_FRWRD_MACT_ENTRY_FRWRD_INFO_print(
    SOC_SAND_IN  SOC_PPC_FRWRD_MACT_ENTRY_FRWRD_INFO *info
  );

void
  SOC_PPC_FRWRD_MACT_ENTRY_VALUE_print(
    SOC_SAND_IN  SOC_PPC_FRWRD_MACT_ENTRY_VALUE *info
  );

void
  SOC_PPC_FRWRD_MACT_TIME_print(
    SOC_SAND_IN  SOC_PPC_FRWRD_MACT_TIME *info
  );

void
  SOC_PPC_FRWRD_MACT_TRAVERSE_MATCH_KEY_RULE_MAC_print(
    SOC_SAND_IN  SOC_PPC_FRWRD_MACT_TRAVERSE_MATCH_KEY_RULE_MAC *info
  );

void
  SOC_PPC_FRWRD_MACT_TRAVERSE_MATCH_KEY_RULE_IPV4_MC_print(
    SOC_SAND_IN  SOC_PPC_FRWRD_MACT_TRAVERSE_MATCH_KEY_RULE_IPV4_MC *info
  );

void
  SOC_PPC_FRWRD_MACT_TRAVERSE_MATCH_KEY_RULE_print(
    SOC_SAND_IN  SOC_PPC_FRWRD_MACT_TRAVERSE_MATCH_KEY_RULE *info
  );

void
  SOC_PPC_FRWRD_MACT_TRAVERSE_MATCH_VALUE_RULE_print(
    SOC_SAND_IN  SOC_PPC_FRWRD_MACT_TRAVERSE_MATCH_VALUE_RULE *info
  );

void
  SOC_PPC_FRWRD_MACT_TRAVERSE_MATCH_RULE_print(
    SOC_SAND_IN  SOC_PPC_FRWRD_MACT_TRAVERSE_MATCH_RULE *info
  );

void
  SOC_PPC_FRWRD_MACT_TRAVERSE_ACTION_print(
    SOC_SAND_IN  SOC_PPC_FRWRD_MACT_TRAVERSE_ACTION *info
  );

void
  SOC_PPC_FRWRD_MACT_TRAVERSE_STATUS_INFO_print(
    SOC_SAND_IN  SOC_PPC_FRWRD_MACT_TRAVERSE_STATUS_INFO *info
  );

void
  SOC_PPC_FRWRD_MACT_ENTRY_KEY_SLB_print(
    SOC_SAND_IN  SOC_PPC_FRWRD_MACT_ENTRY_KEY_SLB *info
  );

void
  SOC_PPC_FRWRD_MACT_ENTRY_VALUE_SLB_print(
    SOC_SAND_IN  SOC_PPC_FRWRD_MACT_ENTRY_VALUE_SLB *info
  );

void
  SOC_PPC_FRWRD_MACT_TRAVERSE_MATCH_KEY_RULE_SLB_print(
    SOC_SAND_IN  SOC_PPC_FRWRD_MACT_TRAVERSE_MATCH_KEY_RULE_SLB *info
  );

#endif /* SOC_PPC_DEBUG_IS_LVL1 */

/* } */

#include <soc/dpp/SAND/Utils/sand_footer.h>

/* } __SOC_PPC_API_FRWRD_MACT_INCLUDED__*/
#endif

