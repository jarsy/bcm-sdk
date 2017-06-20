
/* $Id: arad_pp_frwrd_ip_tcam.h,v 1.19 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/
/******************************************************************
*
* FILENAME:       DuneDriver/ARAD/ARAD_PP/include/arad_pp_frwrd_ip_tcam.h
*
* MODULE PREFIX:  arad_pp
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

#ifndef __ARAD_PP_FRWRD_IP_TCAM_INCLUDED__
/* { */
#define __ARAD_PP_FRWRD_IP_TCAM_INCLUDED__

/*************
 * INCLUDES  *
 *************/
/* { */

#include <soc/dpp/SAND/Utils/sand_header.h>

#include <soc/dpp/SAND/Management/sand_general_macros.h>
#include <soc/dpp/SAND/Management/sand_error_code.h>
#include <soc/dpp/SAND/Utils/sand_hashtable.h>

#include <soc/dpp/PPC/ppc_api_frwrd_ipv4.h>
#include <soc/dpp/PPC/ppc_api_frwrd_ipv6.h>
#include <soc/dpp/PPC/ppc_api_l3_src_bind.h>
#include <soc/dpp/PPC/ppc_api_frwrd_trill.h>
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_framework.h>
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_kbp_entry_mgmt.h>

#include <soc/dpp/ARAD/arad_sw_db_tcam_mgmt.h>
#include <soc/dpp/ARAD/arad_tcam.h>


#if defined(INCLUDE_KBP) && !defined(BCM_88030)
#include <soc/dpp/ARAD/arad_kbp.h>
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_lem_access.h>
#include <shared/swstate/sw_state.h>
#include <shared/swstate/sw_state_workarounds.h>
#endif
/* } */
/*************
 * DEFINES   *
 *************/
/* { */





/* length of the key stream that
* is big enough to contain either ipv6 uc / ipvc mc / ipv4 mc
* key stream 
* and also ELK key  
*/
#define ARAD_PP_IP_TCAM_KEY_LEN_BYTES 80
#define ARAD_PP_IP_TCAM_KEY_LEN_LONGS (ARAD_PP_IP_TCAM_KEY_LEN_BYTES / sizeof(uint32))

#define ARAD_PP_FRWRD_IP_TCAM_MIN_BANKS                          (0)

#define ARAD_PP_FRWRD_IPV4_MC_PREFIX_SIZE                        (0)
#define ARAD_PP_FRWRD_IPV6_UC_PREFIX_SIZE                        (SOC_IS_JERICHO(unit)? 0: 4)
#define ARAD_PP_FRWRD_IPV6_MC_PREFIX_SIZE                        (4)

/* priority of each key field in TCAM */
#define ARAD_PP_IPMC_DIP_PRIO    (32)
#define ARAD_PP_IPMC_RIF_PRIO    (10)

#define ARAD_PP_IP_TCAM_ENTRY_KEY_TYPE_IPV6_UC_OR_VPN 0
#define ARAD_PP_IP_TCAM_ENTRY_KEY_TYPE_IPV6_MC 1
#define ARAD_PP_IP_TCAM_ENTRY_KEY_TYPE_NOF_BITS 1

#define ARAD_PP_FRWRD_IPV6_SUPPORT (1)

/* } */
/*************
 * MACROS    *
 *************/
/* { */

#define ARAD_PP_IP_BIT_STREAM_SET_ANY_FIELD(input_bitstream, nof_bits, output_stream) \
  res = soc_sand_bitstream_set_any_field(     \
    input_bitstream,                        \
    start_bit,                              \
    nof_bits,                               \
    output_stream                           \
  );                                      \
  SOC_SAND_CHECK_FUNC_RESULT(res,  23, exit); \
  start_bit += nof_bits;

#define ARAD_PP_IPV6_TCAM_ENTRY_TO_VRF_AND_SUBNET(key_param, vrf_ndx_param, subnet_param) \
  if(key_param->type == ARAD_IP_TCAM_ENTRY_TYPE_IPV6_UC)                   \
  {                                                                   \
    vrf_ndx_param = &key_param->vrf_ndx;                                     \
    subnet_param = &key_param->key.ipv6_uc.subnet;                                      \
  }                                                                   \
  else if(key->type == ARAD_IP_TCAM_ENTRY_TYPE_IPV6_VPN)  \
  {                                                       \
    vrf_ndx_param = &key_param->vrf_ndx;                         \
    subnet_param = &key_param->key.ipv6_vpn.subnet;                          \
  } \
  else \
  { \
    vrf_ndx_param = NULL; \
    subnet_param = NULL; \
  }

/* } */
/*************
 * TYPE DEFS *
 *************/
/* { */

typedef union arad_pp_frwrd_ip_tcam_key_u
  {
    SOC_PPC_FRWRD_IPV4_MC_ROUTE_KEY ipv4_mc;
    SOC_PPC_FRWRD_IPV6_UC_ROUTE_KEY ipv6_uc;
    SOC_PPC_FRWRD_IPV6_MC_ROUTE_KEY ipv6_mc;
    SOC_PPC_FRWRD_IPV6_VPN_ROUTE_KEY ipv6_vpn;
#if defined(INCLUDE_KBP) && !defined(BCM_88030)
    ARAD_PP_FRWRD_IP_ELK_FWD_KEY elk_fwd;
#endif /* defined(INCLUDE_KBP) && !defined(BCM_88030) */
    SOC_PPC_SRC_BIND_IPV6_ENTRY ipv6_static_src_bind;
    SOC_PPC_TRILL_MC_ROUTE_KEY trill_mc;
    SOC_PPC_IPV6_COMPRESSED_ENTRY ip6_compression;
  } arad_pp_frwrd_ip_tcam_key_t;

/* 
 * General struct for all the possible TCAM keys: 
 * - the type differentiates between the possible formats 
 * - ipv4_mc / .. / ipv6_vpn corresponds to internal TCAM addition 
 * - elk for all the forwarding tables implemented in the ELK KBP 
 */
typedef struct
{
  ARAD_IP_TCAM_ENTRY_TYPE type;
  arad_pp_frwrd_ip_tcam_key_t key;
  uint32 vrf_ndx;
} ARAD_PP_IP_TCAM_ENTRY_KEY;

#if defined(INCLUDE_KBP) && !defined(BCM_88030)
typedef kbp_entry_t* kbp_entry_handle;
typedef struct
{
  kbp_entry_handle  db_entry;
} ARAD_SW_KBP_HANDLE;
typedef struct
{
  uint8  entry_flag;
} ARAD_SW_KBP_DIAG_FLAGS;
#endif /* defined(INCLUDE_KBP) && !defined(BCM_88030) */

typedef struct {
  SOC_SAND_HASH_TABLE_PTR route_key_to_entry_id;
#if defined(INCLUDE_KBP) && !defined(BCM_88030)
  /* 
   * In ACLs, a location table is needed 
   * to translate the SOC entry ID to the KBP entry handle. 
   */
  PARSER_HINT_ARR ARAD_SW_KBP_HANDLE *location_tbl;  /* for ACL location table */
  uint8 kbp_cache_mode;
  ARAD_SW_KBP_DIAG_FLAGS kbp_diag_flag;
#endif /* defined(INCLUDE_KBP) && !defined(BCM_88030) */
} ARAD_FRWRD_IP;

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

uint32
  arad_pp_ip_tcam_entry_hw_add(
    SOC_SAND_IN  int                     unit,
    SOC_SAND_IN  ARAD_PP_IP_TCAM_ENTRY_KEY  *route_key,
    SOC_SAND_IN  uint8                       is_for_update,
    SOC_SAND_IN  ARAD_TCAM_ACTION           *action,
    SOC_SAND_IN  uint32                     data_indx,
    SOC_SAND_OUT SOC_SAND_SUCCESS_FAILURE   *success
  );

uint32
  arad_pp_frwrd_ip_tcam_lpm_prefix_len_get(
     SOC_SAND_IN  uint8     *mask,
     SOC_SAND_IN  uint32    table_size_in_bytes,
     SOC_SAND_OUT uint32    *prefix_len
  );

uint32
    arad_pp_frwrd_ip_tcam_ip_db_id_get(
      SOC_SAND_IN ARAD_IP_TCAM_ENTRY_TYPE entry_type
    );

uint32
  arad_pp_frwrd_ip_tcam_route_add_unsafe(
    SOC_SAND_IN  int                                     unit,
    SOC_SAND_IN  ARAD_PP_IP_TCAM_ENTRY_KEY                       *route_key,
    SOC_SAND_IN  ARAD_TCAM_ACTION                                *action,
    SOC_SAND_OUT SOC_SAND_SUCCESS_FAILURE                          *success
  );

uint32
  arad_pp_frwrd_ip_tcam_route_get_unsafe(
    SOC_SAND_IN  int                                     unit,
    SOC_SAND_IN  ARAD_PP_IP_TCAM_ENTRY_KEY                 *route_key,
    SOC_SAND_IN  uint8                                     exact_match,
    SOC_SAND_OUT ARAD_TCAM_ACTION                           *action,
    SOC_SAND_OUT uint8                                     *found,
    SOC_SAND_OUT uint8                                     *hit_bit
  );

uint32
  arad_pp_frwrd_ip_tcam_route_get_block_unsafe(
    SOC_SAND_IN  int                                     unit,
    SOC_SAND_INOUT  SOC_PPC_IP_ROUTING_TABLE_RANGE               *block_range_key,
    SOC_SAND_OUT  ARAD_PP_IP_TCAM_ENTRY_KEY                       *route_key,
    SOC_SAND_OUT ARAD_TCAM_ACTION                            *action,
    SOC_SAND_OUT uint32                                      *nof_entries
  );

uint32
  arad_pp_frwrd_ip_tcam_route_remove_unsafe(
    SOC_SAND_IN  int                                     unit,
    SOC_SAND_IN  ARAD_PP_IP_TCAM_ENTRY_KEY                       *route_key
  );

uint32
  arad_pp_frwrd_ip_tcam_routing_table_clear_unsafe(
    SOC_SAND_IN  int                                    unit,
    SOC_SAND_IN  ARAD_PP_IP_TCAM_ENTRY_KEY                       *route_key,
    SOC_SAND_IN  uint8                                    clear_all_vrf
  );

/* } */

uint32
  arad_pp_frwrd_ip_tcam_rewrite_entry(
     SOC_SAND_IN  int                        unit,
     SOC_SAND_IN  uint8                      entry_exists,
     SOC_SAND_IN  ARAD_TCAM_GLOBAL_LOCATION  *global_location,
     SOC_SAND_IN  ARAD_TCAM_LOCATION         *location
  );

#include <soc/dpp/SAND/Utils/sand_footer.h>

/* } __ARAD_PP_FRWRD_IP_TCAM_INCLUDED__*/
#endif



