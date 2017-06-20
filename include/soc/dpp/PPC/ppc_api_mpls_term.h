/* $Id: ppc_api_mpls_term.h,v 1.13 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/
/******************************************************************
*
* FILENAME:       DuneDriver/ppc/include/soc_ppc_api_mpls_term.h
*
* MODULE PREFIX:  soc_ppc_mpls
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

#ifndef __SOC_PPC_API_MPLS_TERM_INCLUDED__
/* { */
#define __SOC_PPC_API_MPLS_TERM_INCLUDED__

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

/*     MPLS Labels 0-15 are reserved labels                    */
#define SOC_PPC_MPLS_TERM_NOF_RESERVED_LABELS (16)

/*     Maximum number of terminated                    */
#define SOC_PPC_MPLS_TERM_MAX_NOF_TERM_LABELS (3)

#define SOC_PPC_MPLS_TERM_DISCARD_TTL_0  (0x1) 
#define SOC_PPC_MPLS_TERM_DISCARD_TTL_1  (0x2) 
#define SOC_PPC_MPLS_TERM_HAS_CW        (0x4) 
#define SOC_PPC_MPLS_TERM_SKIP_ETH      (0x8)

/*    MPLS Reserved Label */
#define SOC_PPC_MPLS_TERM_RESERVED_LABEL_ELI                          (7)
#define SOC_PPC_MPLS_TERM_RESERVED_LABEL_GAL                          (13)
#define SOC_PPC_MPLS_TERM_RESERVED_LABEL_IPV4_EXP                     (0)
#define SOC_PPC_MPLS_TERM_RESERVED_LABEL_IPV6_EXP                     (2)
#define SOC_PPC_MPLS_TERM_RESERVED_LABEL_RA                           (1)
#define SOC_PPC_MPLS_TERM_RESERVED_LABEL_OAM_ALERT                    (14)

#define SOC_PPC_MPLS_TERM_FLAG_DUMMY_LABEL   (0x1)
#define SOC_PPC_MPLS_TERM_FLAG_EVPN_IML      (0x2)
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
   *  Pipe-model processing
   */
  SOC_PPC_MPLS_TERM_MODEL_PIPE = 0,
  /*
   *  Uniform-model processing
   */
  SOC_PPC_MPLS_TERM_MODEL_UNIFORM = 1,
  /*
   *  Number of types in SOC_PPC_MPLS_TERM_MODEL_TYPE
   */
  SOC_PPC_NOF_MPLS_TERM_MODEL_TYPES = 2
}SOC_PPC_MPLS_TERM_MODEL_TYPE;

typedef enum
{
  /*
   *  the lookup key is the MPLS label
   */
  SOC_PPC_MPLS_TERM_KEY_TYPE_LABEL = 0,
  /*
   *  the lookup key is the MPLS label + inRIF
   */
  SOC_PPC_MPLS_TERM_KEY_TYPE_LABEL_RIF = 1,
  /*
   *  Number of types in SOC_PPC_MPLS_TERM_MODEL_TYPE
   */
  SOC_PPC_NOF_MPLS_TERM_KEY_TYPES = 2
}SOC_PPC_MPLS_TERM_KEY_TYPE;


typedef enum
{
  /*
   *  Namespace L1
   */
  SOC_PPC_MPLS_TERM_NAMESPACE_L1 = 0,
  /*
   *  Namespace L2
   */
  SOC_PPC_MPLS_TERM_NAMESPACE_L2 = 1,
  /*
   *  Namespace L3
   */
  SOC_PPC_MPLS_TERM_NAMESPACE_L3 = 2,
  /*
   *  Namespaces L1, L3
   */
  SOC_PPC_MPLS_TERM_NAMESPACE_L1_L3 = 3,
  /*
   *  Namespaces L1, L2
   */
  SOC_PPC_MPLS_TERM_NAMESPACE_L1_L2 = 4,  
  /* 
   *  Number of namespaces
   */ 
  SOC_PPC_MPLS_TERM_NOF_NAMESPACE_TYPES = 5  
}SOC_PPC_MPLS_TERM_NAMESPACE_TYPE;


typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  First label in a range. Range: 0-2^20
   */
  SOC_SAND_PP_MPLS_LABEL first_label;
  /*
   *  Last label in a range. Range: 0-2^20
   */
  SOC_SAND_PP_MPLS_LABEL last_label;

} SOC_PPC_MPLS_TERM_LABEL_RANGE;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  Lookup type for tunnel termination may be Label or
   *  (Label, In-RIF)
   */
   SOC_PPC_MPLS_TERM_KEY_TYPE key_type;

} SOC_PPC_MPLS_TERM_LKUP_INFO;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  Pipe/Uniform processing
   */
  SOC_PPC_MPLS_TERM_MODEL_TYPE processing_type;
  /*
   *  RIF Id. When RIF == SOC_PPC_RIF_NULL, the default RIF ID and
   *  RPF enable flag are not updated.
   */
  SOC_PPC_RIF_ID rif;
  /*
   *  Class of Service mapping profile. Setting the profile to
   *  '0' keeps the previous TC and DP values T20E: Ignored.
   */
  uint32 cos_profile;
  /*
   *  When MPLS label is terminated user has to specify what
   *  is the next header type, value can be IP or MPLS.
   *  Valid for Soc_petra-B
   */
  SOC_PPC_L3_NEXT_PRTCL_TYPE next_prtcl;

  /*
   *  When MPLS label is terminated user has to specify what
   *  is the forwarding code..
   *  Used only in ARAD and replaces 'next_prtcl' field.
   *  In ARAD+ next protocol is derived from first nibble of next header.
   */
  SOC_TMC_PKT_FRWRD_TYPE forwarding_code;

  /*
   * lif general profile,
   * used for OAM and PMF setting
   * range: 0-15.
   * Used only in Arad.
   */
  uint32 lif_profile;

  /*
   * points to one of the global tpid profiles, which set by soc_ppd_llp_parse_tpid_profile_info_set
   * Needed in case next header (after termination) is Ethernet
   * used for parsing the Inner L2 header,
   * range: 0 -3.
   * Used only in Arad.
   */
  uint32 tpid_profile;

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
   * Virtual Switch ID.
   * needed in case of EthoIP
   * this is the VSI used for switching according to the internal
   * Ethernet header
   * Used only in Arad.
   */
  SOC_PPC_VSI_ID vsi;

  /*
   * enable Learning on this tunnel relevant in case of EthoIP.
   * Used only in Arad.
   */
  uint8 learn_enable;

  /*
   * VSI assignment mode,
   * used to refine above VSI value
   * range: 0-3.
   * Used only in Arad.
   */
  uint32 vsi_assignment_mode;

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
   * Used only in Arad.
   * range: 0 - 7
   */
  uint8 trap_code_index; /* 3b */

  /* 
   * termination profile, indicates how to 
   * process the terminated mpls-header 
   * see soc_ppd_lif_mpls_term_profile_info_set 
   * Arad only. 
   * Range 0-7. 
   */ 
  uint8 term_profile;

  /*
   * flag to indicate LIF has extension. Only relevant in Jericho.
   */
  uint8 is_extended;

  /*
   * Extension lif id for this lif. Only relevant in Jericho.
   */
  int ext_lif_id;

  /* 
   * Global lif that this mpls term lif is pointing to.
   *
   */
  int global_lif;

  /* 
   * Orientation bit. in Arad, represents one of two orientation types: 
   * spoke and hub (split horizon). In Jericho, together with a bit 
   * in lif profile represents one of 4 groups for split horizon (analogous to 
   * spoke and hub in Arad). 
   * Range 0-1. 
   */ 
  uint8 orientation;

} SOC_PPC_MPLS_TERM_INFO;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  Range of MPLS labels.
   */
  SOC_PPC_MPLS_TERM_LABEL_RANGE range;
  /*
   *  Termination info including RIF, COS-profile. In this
   *  case When RIF == SOC_PPC_RIF_NULL, the default RIF ID is not
   *  updated.
   */
  SOC_PPC_MPLS_TERM_INFO term_info;

} SOC_PPC_MPLS_TERM_LABEL_RANGE_INFO;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  Action profile for a MPLS reserved label, when the label
   *  arrives as the BOS label. The action profile indicates
   *  the forwarding / snooping command for a reserved
   *  label.trap_code range:
   *  SOC_PPC_TRAP_CODE_MPLS_LABEL_VALUE_0 -
   *  SOC_PPC_TRAP_CODE_MPLS_LABEL_VALUE_3. Only trap_code from
   *  action profile is relevant.forward/snoop Strength is set
   *  according to soc_ppd_trap_frwrd_profile_info_set(),
   *  soc_ppd_trap_snoop_profile_info_set().
   */
  SOC_PPC_ACTION_PROFILE bos_action;
  /*
   *  Action profile for a MPLS reserved label, when the label
   *  arrives as a non-BOS label. The action profile indicates
   *  the forwarding / snooping command for a reserved
   *  label.trap_code range:
   *  SOC_PPC_TRAP_CODE_MPLS_LABEL_VALUE_0 -
   *  SOC_PPC_TRAP_CODE_MPLS_LABEL_VALUE_3. Only trap_code from
   *  action profile is relevant.forward/snoop Strength is set
   *  according to soc_ppd_trap_frwrd_profile_info_set(),
   *  soc_ppd_trap_snoop_profile_info_set().
   */
  SOC_PPC_ACTION_PROFILE non_bos_action;
  /*
   *  Termination info including RIF, COS-profile. In this
   *  case When RIF == SOC_PPC_RIF_NULL, the default RIF ID is not
   *  updated.
   */
  SOC_PPC_MPLS_TERM_INFO term_info;

} SOC_PPC_MPLS_TERM_RESERVED_LABEL_INFO;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  Pipe/Uniform processing
   */
  SOC_PPC_MPLS_TERM_MODEL_TYPE processing_type;
  /*
   *  Default RIF Ids, for each range type. Range: 0-4K. When
   *  RIF == SOC_PPC_RIF_NULL, the default RIF ID and RPF enable
   *  flag are not updated.
   */
  SOC_PPC_RIF_ID default_rif;
  /*
   *  TRUE: Unicast RPF is enabledFALSE: Unicast RPF is
   *  disabled
   */
  uint8 uc_rpf_enable;

} SOC_PPC_MPLS_TERM_RESERVED_LABELS_GLBL_INFO;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  Use terminated label in COS parameters termination if
   *  the terminated label is of Pipe model.
   */
  uint8 use_for_pipe;
  /*
   *  Use terminated label in COS parameters termination if
   *  the terminated label is of Uniform model.
   */
  uint8 use_for_uniform;

} SOC_PPC_MPLS_TERM_COS_INFO;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  Incoming EXP in the label of the packet.
   */
  SOC_SAND_PP_MPLS_EXP in_exp;
  /*
   *  Tunnel model (Pipe or uniform)
   */
  SOC_SAND_PP_MPLS_TUNNEL_MODEL model;

} SOC_PPC_MPLS_TERM_LABEL_COS_KEY;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  Traffic Class value. Range: 0 - 7.
   */
  SOC_SAND_PP_TC tc;
  /*
   *  Whether to overwrite the TC value associated with the
   *  packet with this 'tc' value.
   */
  uint8 overwrite_tc;
  /*
   *  Drop Precedence value. Range: 0 - 3.
   */
  SOC_SAND_PP_DP dp;
  /*
   *  Whether to overwrite the DP value associated with the
   *  packet with this 'dp' value.
   */
  uint8 overwrite_dp;

} SOC_PPC_MPLS_TERM_LABEL_COS_VAL;

typedef struct { 

  /* see SOC_PPC_MPLS_TERM_ */ 
  uint32 flags; 
  /* number of headers to terminated 1-3 */ 
  uint32 nof_headers; 
  /* from which of the terminated labels to take TTL and EXP values 0-1 */ 
  uint32 ttl_exp_index; 

#ifdef BCM_88660_A0
  /* 
   * If set then the device will check and use the LIF.Expect-BOS field.
   * If used then the expect-bos checking is strict -- i.e. if expect-bos
   * is set and we get a label without bos then it is trapped.
   * Alternatively if expect-bos is not set but a packet with BOS is recieved
   * then it is trapped.
   */
  uint32 check_bos;

  /* 
   * This field is relevant for Jericho devices and on. 
   * On Arad devices this bit is taken from the lif table. 
   */
  uint32 expect_bos;
#endif /* BCM_88660_A0 */

} SOC_PPC_MPLS_TERM_PROFILE_INFO; 

/* Represents an MPLS range out of 8 possible ranges */
typedef struct {
  /* Specifies the lower limit of the label range */
  bcm_mpls_label_t label_low;
  
  /* Specifies the upper limit of the label range */
  bcm_mpls_label_t label_high;
  
  /* Label expected BOS value */
  uint32 bos_value;
  
  /* Mask label BOS check */
  uint32 bos_value_mask;

} SOC_PPC_MPLS_TERM_RANGE_ACTION_INFO;

/* Represents a profile that is attached to one of 8 MPLS label ranges*/
typedef struct {
  /* Tag mode indication */
  uint32 mpls_label_range_tag_mode;
  
  /* Has CW above label indication*/
  uint32 mpls_label_range_has_cw;
  
  /* Outer VID valid indication */
  uint32 mpls_label_range_set_outer_vid;
  
  /* Inner VID valid indication */
  uint32 mpls_label_range_set_inner_vid;
  
  /* If set replace label with Minimum range given */
  uint32 mpls_label_range_use_base ; 

} SOC_PPC_MPLS_TERM_RANGE_PROFILE_INFO;



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
  SOC_PPC_MPLS_TERM_LABEL_RANGE_clear(
    SOC_SAND_OUT SOC_PPC_MPLS_TERM_LABEL_RANGE *info
  );

void
  SOC_PPC_MPLS_TERM_LKUP_INFO_clear(
    SOC_SAND_OUT SOC_PPC_MPLS_TERM_LKUP_INFO *info
  );

void
  SOC_PPC_MPLS_TERM_INFO_clear(
    SOC_SAND_OUT SOC_PPC_MPLS_TERM_INFO *info
  );

void
  SOC_PPC_MPLS_TERM_LABEL_RANGE_INFO_clear(
    SOC_SAND_OUT SOC_PPC_MPLS_TERM_LABEL_RANGE_INFO *info
  );

void
  SOC_PPC_MPLS_TERM_RESERVED_LABEL_INFO_clear(
    SOC_SAND_OUT SOC_PPC_MPLS_TERM_RESERVED_LABEL_INFO *info
  );

void
  SOC_PPC_MPLS_TERM_RESERVED_LABELS_GLBL_INFO_clear(
    SOC_SAND_OUT SOC_PPC_MPLS_TERM_RESERVED_LABELS_GLBL_INFO *info
  );

void
  SOC_PPC_MPLS_TERM_COS_INFO_clear(
    SOC_SAND_OUT SOC_PPC_MPLS_TERM_COS_INFO *info
  );

void
  SOC_PPC_MPLS_TERM_LABEL_COS_KEY_clear(
    SOC_SAND_OUT SOC_PPC_MPLS_TERM_LABEL_COS_KEY *info
  );

void
  SOC_PPC_MPLS_TERM_LABEL_COS_VAL_clear(
    SOC_SAND_OUT SOC_PPC_MPLS_TERM_LABEL_COS_VAL *info
  );

void
  SOC_PPC_MPLS_TERM_PROFILE_INFO_clear(
    SOC_SAND_OUT SOC_PPC_MPLS_TERM_PROFILE_INFO *info
  );

void
  SOC_PPC_MPLS_TERM_RANGE_ACTION_INFO_clear(
    SOC_SAND_OUT SOC_PPC_MPLS_TERM_RANGE_ACTION_INFO *info
    );

void
  SOC_PPC_MPLS_TERM_RANGE_PROFILE_INFO_clear(
    SOC_SAND_OUT SOC_PPC_MPLS_TERM_RANGE_PROFILE_INFO *info
    );

#if SOC_PPC_DEBUG_IS_LVL1

const char*
  SOC_PPC_MPLS_TERM_MODEL_TYPE_to_string(
    SOC_SAND_IN  SOC_PPC_MPLS_TERM_MODEL_TYPE enum_val
  );

const char*
  SOC_PPC_MPLS_TERM_KEY_TYPE_to_string(
    SOC_SAND_IN  SOC_PPC_MPLS_TERM_KEY_TYPE enum_val
  );

void
  SOC_PPC_MPLS_TERM_LABEL_RANGE_print(
    SOC_SAND_IN  SOC_PPC_MPLS_TERM_LABEL_RANGE *info
  );

void
  SOC_PPC_MPLS_TERM_LKUP_INFO_print(
    SOC_SAND_IN  SOC_PPC_MPLS_TERM_LKUP_INFO *info
  );

void
  SOC_PPC_MPLS_TERM_INFO_print(
    SOC_SAND_IN  SOC_PPC_MPLS_TERM_INFO *info
  );

void
  SOC_PPC_MPLS_TERM_LABEL_RANGE_INFO_print(
    SOC_SAND_IN  SOC_PPC_MPLS_TERM_LABEL_RANGE_INFO *info
  );

void
  SOC_PPC_MPLS_TERM_RESERVED_LABEL_INFO_print(
    SOC_SAND_IN  SOC_PPC_MPLS_TERM_RESERVED_LABEL_INFO *info
  );

void
  SOC_PPC_MPLS_TERM_RESERVED_LABELS_GLBL_INFO_print(
    SOC_SAND_IN  SOC_PPC_MPLS_TERM_RESERVED_LABELS_GLBL_INFO *info
  );

void
  SOC_PPC_MPLS_TERM_COS_INFO_print(
    SOC_SAND_IN  SOC_PPC_MPLS_TERM_COS_INFO *info
  );

void
  SOC_PPC_MPLS_TERM_LABEL_COS_KEY_print(
    SOC_SAND_IN  SOC_PPC_MPLS_TERM_LABEL_COS_KEY *info
  );

void
  SOC_PPC_MPLS_TERM_LABEL_COS_VAL_print(
    SOC_SAND_IN  SOC_PPC_MPLS_TERM_LABEL_COS_VAL *info
  );

void
  SOC_PPC_MPLS_TERM_PROFILE_INFO_print(
    SOC_SAND_IN  SOC_PPC_MPLS_TERM_PROFILE_INFO *info
  );

void
  SOC_PPC_MPLS_TERM_RANGE_ACTION_INFO_print(
    SOC_SAND_IN  SOC_PPC_MPLS_TERM_RANGE_ACTION_INFO *info
  );

void
  SOC_PPC_MPLS_TERM_RANGE_PROFILE_INFO_print(
    SOC_SAND_IN  SOC_PPC_MPLS_TERM_RANGE_PROFILE_INFO *info
  );

#endif /* SOC_PPC_DEBUG_IS_LVL1 */

/* } */

#include <soc/dpp/SAND/Utils/sand_footer.h>

/* } __SOC_PPC_API_MPLS_TERM_INCLUDED__*/
#endif

