/* $Id: ppc_api_lif_table.h,v 1.12 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/
/******************************************************************
*
* FILENAME:       DuneDriver/ppc/include/soc_ppc_api_lif_table.h
*
* MODULE PREFIX:  soc_ppc_lif
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

#ifndef __SOC_PPC_API_LIF_TABLE_INCLUDED__
/* { */
#define __SOC_PPC_API_LIF_TABLE_INCLUDED__

/*************
 * INCLUDES  *
 *************/
/* { */

#include <soc/dpp/SAND/Utils/sand_header.h>

#include <soc/dpp/SAND/Management/sand_general_macros.h>
#include <soc/dpp/SAND/Management/sand_error_code.h>

#include <soc/dpp/PPC/ppc_api_general.h>
#include <soc/dpp/PPC/ppc_api_lif.h>
#include <soc/dpp/PPC/ppc_api_rif.h>

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
   *  Empty entry. The LIF entry is not used
   */
  SOC_PPC_LIF_ENTRY_TYPE_EMPTY = 0x1,
  /*
   *  Attachement Circuit entry. The AC is not part of ACs
   *  group
   */
  SOC_PPC_LIF_ENTRY_TYPE_AC = 0x2,
  /*
   *  First AC in ACs groups. ACs group is used for attaching a
   *  packet with an AC according to the packet's QoS params
   */
  SOC_PPC_LIF_ENTRY_TYPE_FIRST_AC_IN_GROUP = 0x4,
  /*
   *  Middle AC in ACs groups. ACs group is used for attaching
   *  a packet with an AC according to the packet's QoS params
   */
  SOC_PPC_LIF_ENTRY_TYPE_MIDDLE_AC_IN_GROUP = 0x8,
  /*
   *  Pseudo Wire entry
   */
  SOC_PPC_LIF_ENTRY_TYPE_PWE = 0x10,
  /*
   *  Mac-in-Mac ISID attributes
   */
  SOC_PPC_LIF_ENTRY_TYPE_ISID = 0x20,
  /*
   *  Router Interface derived from IP tunnel
   */
  SOC_PPC_LIF_ENTRY_TYPE_IP_TUNNEL_RIF = 0x40,
  /*
   *  Router Interface derived from MPLS tunnel
   */
  SOC_PPC_LIF_ENTRY_TYPE_MPLS_TUNNEL_RIF = 0x80,
  /*
   *  TRILL Nick-name
   */
  SOC_PPC_LIF_ENTRY_TYPE_TRILL_NICK = 0x100,
  /*
   *  Extender Port entry
   */
  SOC_PPC_LIF_ENTRY_TYPE_EXTENDER = 0x200,
  /*
   *  ALL
   */
  SOC_PPC_LIF_ENTRY_TYPE_ALL = (int)0xFFFFFFFF,
  /*
   *  Number of types in SOC_PPC_LIF_ENTRY_TYPE
   */
  SOC_PPC_NOF_LIF_ENTRY_TYPES = 10
}SOC_PPC_LIF_ENTRY_TYPE;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  Attachment circuit attributes. Valid for types: AC, First
   *  AC in a group, middle AC in a group
   */
  SOC_PPC_L2_LIF_AC_INFO ac;
  /*
   *  Pseudo Wire attributes.
   */
  SOC_PPC_L2_LIF_PWE_INFO pwe;
  /*
   *  Backbone Service Instance attributes
   */
  SOC_PPC_L2_LIF_ISID_INFO isid;
  /*
   *  Router Interface information relevant for IP/MPLS
   *  tunnel-RIF
   */
  SOC_PPC_RIF_INFO rif;
  /*
   *  Router Interface information relevant for IP
   *  tunnel-term-RIF
   */
  SOC_PPC_RIF_IP_TERM_INFO ip_term_info;
  /*
   *  Router Interface information relevant for MPLS
   *  tunnel-term-RIF
   */
  SOC_PPC_MPLS_TERM_INFO mpls_term_info;
  /*
   * Trill attributes
   */
  SOC_PPC_L2_LIF_TRILL_INFO trill;
  /*
   * Extender attributes
   */
  SOC_PPC_L2_LIF_EXTENDER_INFO extender;

} SOC_PPC_LIF_ENTRY_PER_TYPE_INFO;

/**
 * IMPORTANT NOTE:
 * This struct is too big to be on the stack and therefore must be allocated. 
 * Example: 
 *  
 BCM_ALLOC(lif_info, sizeof(SOC_PPC_LIF_ENTRY_INFO), "bcm_petra_port_class_set.lif_info");
 if (lif_info == NULL) {        
   BCM_ERR_EXIT_MSG(BCM_E_MEMORY, (_BCM_MSG("failed to allocate memory")));
 } 
 */
typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  LIF entry Type
   */
  SOC_PPC_LIF_ENTRY_TYPE type;
  /*
   *  LIF entry information, according to the entry type
   */
  SOC_PPC_LIF_ENTRY_PER_TYPE_INFO value;
  /*
   *  LIF index. Needed when retrieved by the get block
   *  function
   */
  SOC_PPC_LIF_ID index;

} SOC_PPC_LIF_ENTRY_INFO;


typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  Bitmap that indicates which type of entries to retrieve.
   *  See SOC_PPC_LIF_ENTRY_TYPE
   */
  uint32 entries_type_bm;
  /*
   *  return only hit entries, accessed by HW upon packet lookup
   */
  uint8 accessed_only;

} SOC_PPC_LIF_TBL_TRAVERSE_MATCH_RULE;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  was this entry accessed by HW upon packet lookup
   */
  uint8 accessed;

} SOC_PPC_LIF_TABLE_ENTRY_ACCESSED_INFO;


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
  SOC_PPC_LIF_ENTRY_PER_TYPE_INFO_clear(
    SOC_SAND_OUT SOC_PPC_LIF_ENTRY_PER_TYPE_INFO *info
  );

void
  SOC_PPC_LIF_ENTRY_INFO_clear(
    SOC_SAND_OUT SOC_PPC_LIF_ENTRY_INFO *info
  );

void
  SOC_PPC_LIF_TBL_TRAVERSE_MATCH_RULE_clear(
    SOC_SAND_OUT SOC_PPC_LIF_TBL_TRAVERSE_MATCH_RULE *info
  );

void
  SOC_PPC_LIF_TABLE_ENTRY_ACCESSED_INFO_clear(
    SOC_SAND_OUT SOC_PPC_LIF_TABLE_ENTRY_ACCESSED_INFO *info
  );


#if SOC_PPC_DEBUG_IS_LVL1

const char*
  SOC_PPC_LIF_ENTRY_TYPE_to_string(
    SOC_SAND_IN  SOC_PPC_LIF_ENTRY_TYPE enum_val
  );

void
  SOC_PPC_LIF_ENTRY_PER_TYPE_INFO_print(
    SOC_SAND_IN  SOC_PPC_LIF_ENTRY_PER_TYPE_INFO *info,
    SOC_SAND_IN  SOC_PPC_LIF_ENTRY_TYPE  type
  );

void
  SOC_PPC_LIF_ENTRY_INFO_print(
    SOC_SAND_IN  SOC_PPC_LIF_ENTRY_INFO *info
  );

void
  SOC_PPC_LIF_TBL_TRAVERSE_MATCH_RULE_print(
    SOC_SAND_IN  SOC_PPC_LIF_TBL_TRAVERSE_MATCH_RULE *info
  );

void
  SOC_PPC_LIF_TABLE_ENTRY_ACCESSED_INFO_print(
    SOC_SAND_IN  SOC_PPC_LIF_TABLE_ENTRY_ACCESSED_INFO *info
  );


#endif /* SOC_PPC_DEBUG_IS_LVL1 */

/* } */

#include <soc/dpp/SAND/Utils/sand_footer.h>

/* } __SOC_PPC_API_LIF_TABLE_INCLUDED__*/
#endif
