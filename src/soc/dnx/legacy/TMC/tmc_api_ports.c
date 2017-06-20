/* $Id: jer2_tmc_api_ports.c,v 1.22 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/
/******************************************************************
*
* FILENAME:       DuneDriver/jer2_tmc/src/soc_jer2_tmcapi_ports.c
*
* MODULE PREFIX:  soc_jer2_tmcports
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

/*************
 * INCLUDES  *
 *************/
/* { */


#include <shared/bsl.h>

#include <soc/dnx/legacy/SAND/Utils/sand_header.h>

#include <soc/dnx/legacy/SAND/Utils/sand_os_interface.h>
#include <soc/dnx/legacy/TMC/tmc_api_ports.h>

/* } */

/*************
 * DEFINES   *
 *************/
/* { */

/* } */

/*************
 *  MACROS   *
 *************/
/* { */

/* } */

/*************
 * TYPE DEFS *
 *************/
/* { */

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
  DNX_TMC_PORT2IF_MAPPING_INFO_clear(
    DNX_SAND_OUT DNX_TMC_PORT2IF_MAPPING_INFO *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  sal_memset(info, 0x0, sizeof(DNX_TMC_PORT2IF_MAPPING_INFO));
  info->if_id = DNX_TMC_IF_ID_NONE;
  info->channel_id = 0;
  DNX_SAND_MAGIC_NUM_SET;
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  DNX_TMC_PORTS_LAG_MEMBER_clear(
    DNX_SAND_OUT DNX_TMC_PORTS_LAG_MEMBER *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  sal_memset(info, 0x0, sizeof(DNX_TMC_PORTS_LAG_MEMBER));
  info->sys_port = 0;
  info->member_id = 0;
  DNX_SAND_MAGIC_NUM_SET;
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  DNX_TMC_PORTS_LAG_INFO_JER2_ARAD_clear(
    DNX_SAND_OUT DNX_TMC_PORTS_LAG_INFO_JER2_ARAD *info
  )
{
  uint32 ind;
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  sal_memset(info, 0x0, sizeof(DNX_TMC_PORTS_LAG_INFO_JER2_ARAD));
  info->nof_entries = 0;
  /*
   *  The default behavior is:
   *  member_id is the index in the sys-ports array
   */
  for (ind=0; ind<DNX_TMC_PORTS_LAG_MEMBERS_MAX; ++ind)
  {
    DNX_TMC_PORTS_LAG_MEMBER_clear(&(info->lag_member_sys_ports[ind]));
    info->lag_member_sys_ports[ind].member_id = ind % DNX_TMC_PORTS_LAG_OUT_MEMBERS_MAX;
  }
  DNX_SAND_MAGIC_NUM_SET;
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  DNX_TMC_PORTS_LAG_INFO_PETRAB_clear(
    DNX_SAND_OUT DNX_TMC_PORTS_LAG_INFO *info
  )
{
  uint32 ind;
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  sal_memset(info, 0x0, sizeof(DNX_TMC_PORTS_LAG_INFO));
  info->nof_entries = 0;
  /*
   *  The default behavior is:
   *  member_id is the index in the sys-ports array
   */
  for (ind=0; ind<DNX_TMC_PORTS_LAG_MEMBERS_MAX; ++ind)
  {
    DNX_TMC_PORTS_LAG_MEMBER_clear(&(info->lag_member_sys_ports[ind]));
    info->lag_member_sys_ports[ind].member_id = ind % DNX_TMC_PORTS_LAG_OUT_MEMBERS_MAX;
  }
  DNX_SAND_MAGIC_NUM_SET;
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  DNX_TMC_PORTS_OVERRIDE_INFO_clear(
    DNX_SAND_OUT DNX_TMC_PORTS_OVERRIDE_INFO *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  sal_memset(info, 0x0, sizeof(DNX_TMC_PORTS_OVERRIDE_INFO));
  info->enable = 0;
  info->override_val = 0;
  DNX_SAND_MAGIC_NUM_SET;
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  DNX_TMC_PORT_INBOUND_MIRROR_INFO_clear(
    DNX_SAND_OUT DNX_TMC_PORT_INBOUND_MIRROR_INFO *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  sal_memset(info, 0x0, sizeof(DNX_TMC_PORT_INBOUND_MIRROR_INFO));
  info->enable = 0;
  DNX_TMC_DEST_INFO_clear(&(info->destination));
  DNX_TMC_PORTS_OVERRIDE_INFO_clear(&(info->dp_override));
  DNX_TMC_PORTS_OVERRIDE_INFO_clear(&(info->tc_override));
  DNX_SAND_MAGIC_NUM_SET;
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  DNX_TMC_PORT_OUTBOUND_MIRROR_INFO_clear(
    DNX_SAND_OUT DNX_TMC_PORT_OUTBOUND_MIRROR_INFO *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  sal_memset(info, 0x0, sizeof(DNX_TMC_PORT_OUTBOUND_MIRROR_INFO));
  info->enable = 0;
  info->ifp_id = 0;
  info->skip_port_deafult_enable = 0;
  DNX_SAND_MAGIC_NUM_SET;
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  DNX_TMC_PORT_SNOOP_INFO_clear(
    DNX_SAND_OUT DNX_TMC_PORT_SNOOP_INFO *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  sal_memset(info, 0x0, sizeof(DNX_TMC_PORT_SNOOP_INFO));
  info->enable = 0;
  info->size_bytes = DNX_TMC_PORTS_NOF_SNOOP_SIZES;
  DNX_TMC_DEST_INFO_clear(&(info->destination));
  DNX_TMC_PORTS_OVERRIDE_INFO_clear(&(info->dp_override));
  DNX_TMC_PORTS_OVERRIDE_INFO_clear(&(info->tc_override));
  DNX_SAND_MAGIC_NUM_SET;
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  DNX_TMC_PORTS_ITMH_BASE_clear(
    DNX_SAND_OUT DNX_TMC_PORTS_ITMH_BASE *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  sal_memset(info, 0x0, sizeof(DNX_TMC_PORTS_ITMH_BASE));
  info->pp_header_present = 0;
  info->out_mirror_dis = 0;
  info->snoop_cmd_ndx = 0;
  info->exclude_src = 0;
  info->tr_cls = 0;
  info->dp = 0;
  DNX_TMC_DEST_INFO_clear(&(info->destination));
  DNX_SAND_MAGIC_NUM_SET;
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  DNX_TMC_PORTS_ITMH_EXT_SRC_PORT_clear(
    DNX_SAND_OUT DNX_TMC_PORTS_ITMH_EXT_SRC_PORT *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  sal_memset(info, 0x0, sizeof(DNX_TMC_PORTS_ITMH_EXT_SRC_PORT));
  info->enable = 0;
  info->is_src_sys_port = 0;
  DNX_TMC_DEST_SYS_PORT_INFO_clear(&(info->src_port));
  DNX_TMC_DEST_INFO_clear(&(info->destination));
  DNX_SAND_MAGIC_NUM_SET;
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  DNX_TMC_PORTS_ITMH_clear(
    DNX_SAND_OUT DNX_TMC_PORTS_ITMH *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  sal_memset(info, 0x0, sizeof(DNX_TMC_PORTS_ITMH));
  DNX_TMC_PORTS_ITMH_BASE_clear(&(info->base));
  DNX_TMC_PORTS_ITMH_EXT_SRC_PORT_clear(&(info->extension));
  DNX_SAND_MAGIC_NUM_SET;
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  DNX_TMC_PORTS_ISP_INFO_clear(
    DNX_SAND_OUT DNX_TMC_PORTS_ISP_INFO *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  sal_memset(info, 0x0, sizeof(DNX_TMC_PORTS_ISP_INFO));
  info->enable = FALSE;
  info->queue_id = 0;
  DNX_SAND_MAGIC_NUM_SET;
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
DNX_TMC_PORT_LAG_SYS_PORT_INFO_clear(
    DNX_SAND_OUT DNX_TMC_PORT_LAG_SYS_PORT_INFO *info
  )
{
  uint32
    ind;

  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  sal_memset(info, 0x0, sizeof(DNX_TMC_PORT_LAG_SYS_PORT_INFO));
  info->in_member = 0;
  info->in_lag = 0;
  info->nof_of_out_lags = 0;
  for (ind=0; ind<DNX_TMC_NOF_LAG_GROUPS; ++ind)
  {
    info->out_lags[ind] = 0;
  }

  DNX_SAND_MAGIC_NUM_SET;
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  DNX_TMC_PORTS_STAG_FIELDS_clear(
    DNX_SAND_OUT DNX_TMC_PORTS_STAG_FIELDS   *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  sal_memset(info, 0x0, sizeof(DNX_TMC_PORTS_STAG_FIELDS));
  info->cid = 0;
  info->ifp = 0;
  info->tr_cls = 0;
  info->dp = 0;
  info->data_type = 0;
  DNX_SAND_MAGIC_NUM_SET;
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  DNX_TMC_PORTS_OTMH_EXTENSIONS_EN_clear(
    DNX_SAND_OUT DNX_TMC_PORTS_OTMH_EXTENSIONS_EN *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  sal_memset(info, 0x0, sizeof(DNX_TMC_PORTS_OTMH_EXTENSIONS_EN));
  info->outlif_ext_en = DNX_TMC_PORTS_FTMH_EXT_OUTLIF_IF_MC;
  info->src_ext_en = 0;
  info->dest_ext_en = 0;
  DNX_SAND_MAGIC_NUM_SET;
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  DNX_TMC_PORT_EGR_HDR_CR_DISCOUNT_INFO_clear(
    DNX_SAND_OUT DNX_TMC_PORT_EGR_HDR_CR_DISCOUNT_INFO *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  sal_memset(info, 0x0, sizeof(DNX_TMC_PORT_EGR_HDR_CR_DISCOUNT_INFO));
  info->uc_credit_discount = 0;
  info->mc_credit_discount = 0;
  DNX_SAND_MAGIC_NUM_SET;
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  DNX_TMC_PORT_PP_PORT_INFO_clear(
    DNX_SAND_OUT DNX_TMC_PORT_PP_PORT_INFO *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  sal_memset(info, 0x0, sizeof(DNX_TMC_PORT_PP_PORT_INFO));
  info->is_stag_enabled = 0;
  info->first_header_size = 0;
  info->fc_type = DNX_TMC_PORTS_FC_TYPE_NONE;
  info->header_type = DNX_TMC_PORT_HEADER_TYPE_NONE;
  info->header_type_out = DNX_TMC_PORT_HEADER_TYPE_NONE;
  info->is_snoop_enabled = 0;
  info->is_tm_ing_shaping_enabled = 0;
  info->is_tm_pph_present_enabled = 0;
  info->is_tm_src_syst_port_ext_present = 0;
  info->mirror_profile = 0;
  info->flags = 0;
  DNX_SAND_MAGIC_NUM_SET;
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  DNX_TMC_PORT_COUNTER_INFO_clear(
    DNX_SAND_OUT DNX_TMC_PORT_COUNTER_INFO *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  sal_memset(info, 0x0, sizeof(DNX_TMC_PORT_COUNTER_INFO));
  info->processor_id = DNX_TMC_CNT_NOF_PROCESSOR_IDS;
  info->id = 0;
  DNX_SAND_MAGIC_NUM_SET;
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  DNX_TMC_PORTS_FORWARDING_HEADER_INFO_clear(
    DNX_SAND_OUT DNX_TMC_PORTS_FORWARDING_HEADER_INFO *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  sal_memset(info, 0x0, sizeof(DNX_TMC_PORTS_FORWARDING_HEADER_INFO));
  DNX_TMC_DEST_INFO_clear(&(info->destination));
  info->tr_cls = 0;
  info->dp = 0;
  info->snoop_cmd_ndx = 0;
  DNX_TMC_PORT_COUNTER_INFO_clear(&(info->counter));
  DNX_SAND_MAGIC_NUM_SET;
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  DNX_TMC_PORT_INJECTED_CPU_HEADER_INFO_clear(
    DNX_SAND_OUT DNX_TMC_PORT_INJECTED_CPU_HEADER_INFO *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  sal_memset(info, 0x0, sizeof(DNX_TMC_PORT_INJECTED_CPU_HEADER_INFO));
  info->hdr_format = DNX_TMC_PORT_NOF_INJECTED_HDR_FORMATS;
  info->pp_port_for_tm_traffic = 0;
  DNX_SAND_MAGIC_NUM_SET;
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  DNX_TMC_PORTS_SWAP_INFO_clear(
    DNX_SAND_OUT DNX_TMC_PORTS_SWAP_INFO *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  sal_memset(info, 0x0, sizeof(DNX_TMC_PORTS_SWAP_INFO));

  DNX_SAND_MAGIC_NUM_SET;
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  DNX_TMC_PORTS_PON_TUNNEL_INFO_clear(
    DNX_SAND_OUT DNX_TMC_PORTS_PON_TUNNEL_INFO *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  sal_memset(info, 0x0, sizeof(DNX_TMC_PORTS_PON_TUNNEL_INFO));
  DNX_SAND_MAGIC_NUM_SET;
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  DNX_TMC_PORTS_PROGRAMS_INFO_clear(
    DNX_SAND_OUT DNX_TMC_PORTS_PROGRAMS_INFO *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  sal_memset(info, 0x0, sizeof(DNX_TMC_PORTS_PROGRAMS_INFO));
  DNX_SAND_MAGIC_NUM_SET;
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  DNX_TMC_L2_ENCAP_INFO_clear(
    DNX_SAND_OUT DNX_TMC_L2_ENCAP_INFO *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  sal_memset(info, 0x0, sizeof(DNX_TMC_L2_ENCAP_INFO));
  DNX_SAND_MAGIC_NUM_SET;
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  DNX_TMC_PORTS_APPLICATION_MAPPING_INFO_clear(
    DNX_SAND_OUT DNX_TMC_PORTS_APPLICATION_MAPPING_INFO *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  sal_memset(info, 0x0, sizeof(DNX_TMC_PORTS_APPLICATION_MAPPING_INFO));
  DNX_SAND_MAGIC_NUM_SET;
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

#if DNX_TMC_DEBUG_IS_LVL1

const char*
  DNX_TMC_PORT_DIRECTION_to_string(
    DNX_SAND_IN  DNX_TMC_PORT_DIRECTION enum_val
  )
{
  const char* str = NULL;
  switch(enum_val)
  {
  case DNX_TMC_PORT_DIRECTION_INCOMING:
    str = "incoming";
  break;
  case DNX_TMC_PORT_DIRECTION_OUTGOING:
    str = "outgoing";
  break;
  case DNX_TMC_PORT_DIRECTION_BOTH:
    str = "both";
  break;
  case DNX_TMC_PORT_NOF_DIRECTIONS:
    str = " Not initialized";
  break;
  default:
    str = " Unknown";
  }
  return str;
}

const char*
  DNX_TMC_PORT_HEADER_TYPE_to_string(
    DNX_SAND_IN  DNX_TMC_PORT_HEADER_TYPE enum_val
  )
{
  const char* str = NULL;
  switch(enum_val)
  {
  case DNX_TMC_PORT_HEADER_TYPE_NONE:
    str = "none";
  break;
  case DNX_TMC_PORT_HEADER_TYPE_ETH:
    str = "eth";
  break;
  case DNX_TMC_PORT_HEADER_TYPE_RAW:
    str = "raw";
  break;
  case DNX_TMC_PORT_HEADER_TYPE_TM:
    str = "tm";
  break;
  case DNX_TMC_PORT_HEADER_TYPE_PROG:
    str = "prog";
  break;
  case DNX_TMC_PORT_HEADER_TYPE_CPU:
    str = "cpu";
    break;
  case DNX_TMC_PORT_HEADER_TYPE_STACKING:
    str = "stacking";
    break;
  case DNX_TMC_PORT_HEADER_TYPE_TDM:
    str = "tdm";
    break;
  case DNX_TMC_PORT_HEADER_TYPE_TDM_RAW:
    str = "tdm_raw";
    break;
  case DNX_TMC_PORT_HEADER_TYPE_TDM_PMM:
    str = "tdm_pmm";
    break;
  case DNX_TMC_PORT_HEADER_TYPE_INJECTED:
    str = "injected";
    break;
  case DNX_TMC_PORT_HEADER_TYPE_INJECTED_2:
    str = "injected_2";
    break;
  case DNX_TMC_PORT_HEADER_TYPE_INJECTED_PP:
    str = "injected_pp";
    break;
  case DNX_TMC_PORT_HEADER_TYPE_INJECTED_2_PP:
    str = "injected_2_pp";
    break;
  case DNX_TMC_PORT_NOF_HEADER_TYPES:
    str = " Not initialized";
  break;
  case DNX_TMC_PORT_HEADER_TYPE_UDH_ETH:
    str = " udh_eth";
  break;
  case DNX_TMC_PORT_HEADER_TYPE_MPLS_RAW:
    str = "mpls_raw";
  break;
  case DNX_TMC_PORT_HEADER_TYPE_L2_ENCAP_EXTERNAL_CPU:
    str = "l2_encap_external_cpu";
  break;
  case DNX_TMC_PORT_HEADER_TYPE_MIRROR_RAW:
    str = "mirror_raw";
  break;
  default:
    str = " Unknown";
  }
  return str;
}

const char*
  DNX_TMC_PORTS_SNOOP_SIZE_to_string(
    DNX_SAND_IN  DNX_TMC_PORTS_SNOOP_SIZE enum_val
  )
{
  const char* str = NULL;
  switch(enum_val)
  {
  case DNX_TMC_PORTS_SNOOP_SIZE_BYTES_64:
    str = "bytes_64";
  break;
  case DNX_TMC_PORTS_SNOOP_SIZE_BYTES_192:
    str = "bytes_192";
  break;
  case DNX_TMC_PORTS_SNOOP_SIZE_ALL:
    str = "all";
  break;
  case DNX_TMC_PORTS_NOF_SNOOP_SIZES:
    str = " Not initialized";
  break;
  default:
    str = " Unknown";
  }
  return str;
}

const char*
  DNX_TMC_PORTS_FTMH_EXT_OUTLIF_to_string(
    DNX_SAND_IN  DNX_TMC_PORTS_FTMH_EXT_OUTLIF enum_val
  )
{
  const char* str = NULL;
  switch(enum_val)
  {
  case DNX_TMC_PORTS_FTMH_EXT_OUTLIF_NEVER:
    str = "never";
  break;
  case DNX_TMC_PORTS_FTMH_EXT_OUTLIF_IF_MC:
    str = "if_mc";
  break;
  case DNX_TMC_PORTS_FTMH_EXT_OUTLIF_ALWAYS:
    str = "always";
  break;
  case DNX_TMC_PORTS_FTMH_NOF_EXT_OUTLIFS:
    str = " Not initialized";
  break;
  default:
    str = " Unknown";
  }
  return str;
}

const char*
  DNX_TMC_PORT_EGR_HDR_CR_DISCOUNT_TYPE_to_string(
    DNX_SAND_IN DNX_TMC_PORT_EGR_HDR_CR_DISCOUNT_TYPE enum_val
  )
{
  const char* str = NULL;
  switch(enum_val)
  {
  case DNX_TMC_PORT_EGR_HDR_CR_DISCOUNT_TYPE_A:
    str = "a";
  break;
  case DNX_TMC_PORT_EGR_HDR_CR_DISCOUNT_TYPE_B:
    str = "b";
  break;
  case DNX_TMC_PORT_NOF_EGR_HDR_CR_DISCOUNT_TYPES:
    str = " Not initialized";
  break;
  default:
    str = " Unknown";
  }
  return str;
}

const char*
  DNX_TMC_PORT_INJECTED_HDR_FORMAT_to_string(
    DNX_SAND_IN  DNX_TMC_PORT_INJECTED_HDR_FORMAT enum_val
  )
{
  const char* str = NULL;
  switch(enum_val)
  {
  case DNX_TMC_PORT_INJECTED_HDR_FORMAT_NONE:
    str = "none";
  break;
  case DNX_TMC_PORT_INJECTED_HDR_FORMAT_PP_PORT:
    str = "pp_port";
  break;
  default:
    str = " Unknown";
  }
  return str;
}

const char*
  DNX_TMC_PORTS_VT_PROFILE_to_string(
    DNX_SAND_IN  DNX_TMC_PORTS_VT_PROFILE enum_val
  )
{
  const char* str = NULL;
  switch(enum_val)
  {
  case DNX_TMC_PORTS_VT_PROFILE_NONE:
    str = "none";
  break;  
  case DNX_TMC_PORTS_VT_PROFILE_OVERLAY_RCY:
    str = "overlay_rcy";
  break;
  default:
    str = " Unknown";
  }
  return str;
}

const char*
  DNX_TMC_PORTS_TT_PROFILE_to_string(
    DNX_SAND_IN  DNX_TMC_PORTS_TT_PROFILE enum_val
  )
{
  const char* str = NULL;
  switch(enum_val)
  {
  case DNX_TMC_PORTS_TT_PROFILE_NONE:
    str = "none";
  break;
  case DNX_TMC_PORTS_TT_PROFILE_OVERLAY_RCY:
    str = "overlay_rcy";
  break; 
  case DNX_TMC_PORTS_TT_PROFILE_PON:
    str = "pon";
  break;  
  default:
    str = " Unknown";
  }
  return str;
}

const char*
  DNX_TMC_PORTS_FLP_PROFILE_to_string(
    DNX_SAND_IN  DNX_TMC_PORTS_FLP_PROFILE enum_val
  )
{
  const char* str = NULL;
  switch(enum_val)
  {
  case DNX_TMC_PORTS_FLP_PROFILE_NONE:
    str = "none";
  break;
  case DNX_TMC_PORTS_FLP_PROFILE_OVERLAY_RCY:
    str = "overlay_rcy";
  break;
  case DNX_TMC_PORTS_FLP_PROFILE_PON:
    str = "pon port";
  break;
  case DNX_TMC_PORTS_FLP_PROFILE_OAMP:
    str = "oamp port";
  break;
  default:
    str = " Unknown";
  }
  return str;
}


void
  DNX_TMC_PORT2IF_MAPPING_INFO_print(
    DNX_SAND_IN DNX_TMC_PORT2IF_MAPPING_INFO *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  LOG_CLI((BSL_META_U(unit,
                      "  If_id: %s \n\r"),
           DNX_TMC_INTERFACE_ID_to_string(info->if_id)
           ));
  LOG_CLI((BSL_META_U(unit,
                      "  Channel_id: %u\n\r"),info->channel_id));
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  DNX_TMC_PORTS_LAG_MEMBER_print(
    DNX_SAND_IN  DNX_TMC_PORTS_LAG_MEMBER *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  if (info->member_id == DNX_TMC_FAP_PORT_ID_INVALID)
  {
    LOG_CLI((BSL_META_U(unit,
                        "Sys_port: %4u, Member_id: %s\n\r"),info->sys_port, "Unknown at local FAP"));
  }
  else
  {
    LOG_CLI((BSL_META_U(unit,
                        "Sys_port: %4u, Member_id: %u\n\r"),info->sys_port, info->member_id));
  }

exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  DNX_TMC_PORTS_LAG_INFO_print(
    DNX_SAND_IN DNX_TMC_PORTS_LAG_INFO *info
  )
{
  uint32 ind=0;
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  LOG_CLI((BSL_META_U(unit,
                      "Nof_entries:              %u\n\r"),info->nof_entries));
  for (ind=0; ind<info->nof_entries; ++ind)
  {
    DNX_TMC_PORTS_LAG_MEMBER_print(&(info->lag_member_sys_ports[ind]));
  }
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  DNX_TMC_PORTS_OVERRIDE_INFO_print(
    DNX_SAND_IN DNX_TMC_PORTS_OVERRIDE_INFO *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  LOG_CLI((BSL_META_U(unit,
                      " Enable:       %d\n\r"),info->enable));
  LOG_CLI((BSL_META_U(unit,
                      " Override_val: %u\n\r"),info->override_val));
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  DNX_TMC_PORT_INBOUND_MIRROR_INFO_print(
    DNX_SAND_IN DNX_TMC_PORT_INBOUND_MIRROR_INFO *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  LOG_CLI((BSL_META_U(unit,
                      "Enable: %d\n\r"),info->enable));
  LOG_CLI((BSL_META_U(unit,
                      "Destination: ")));
  DNX_TMC_DEST_INFO_print(&(info->destination));
  LOG_CLI((BSL_META_U(unit,
                      "Dp_override:\n\r")));
  DNX_TMC_PORTS_OVERRIDE_INFO_print(&(info->dp_override));
  LOG_CLI((BSL_META_U(unit,
                      "Tc_override:\n\r")));
  DNX_TMC_PORTS_OVERRIDE_INFO_print(&(info->tc_override));
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  DNX_TMC_PORT_OUTBOUND_MIRROR_INFO_print(
    DNX_SAND_IN DNX_TMC_PORT_OUTBOUND_MIRROR_INFO *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  LOG_CLI((BSL_META_U(unit,
                      "Enable: %d\n\r"),info->enable));
  LOG_CLI((BSL_META_U(unit,
                      "Ifp_id: %u\n\r"),info->ifp_id));
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  DNX_TMC_PORT_SNOOP_INFO_print(
    DNX_SAND_IN DNX_TMC_PORT_SNOOP_INFO *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  LOG_CLI((BSL_META_U(unit,
                      "Enable: %d\n\r"),info->enable));
  LOG_CLI((BSL_META_U(unit,
                      "Size_bytes: %s[Bytes] \n\r"),
           DNX_TMC_PORTS_SNOOP_SIZE_to_string(info->size_bytes)
           ));
  LOG_CLI((BSL_META_U(unit,
                      "Destination: ")));
  DNX_TMC_DEST_INFO_print(&(info->destination));
  LOG_CLI((BSL_META_U(unit,
                      "Dp_override:\n\r")));
  DNX_TMC_PORTS_OVERRIDE_INFO_print(&(info->dp_override));
  LOG_CLI((BSL_META_U(unit,
                      "Tc_override:\n\r")));
  DNX_TMC_PORTS_OVERRIDE_INFO_print(&(info->tc_override));
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  DNX_TMC_PORTS_ITMH_BASE_print(
    DNX_SAND_IN DNX_TMC_PORTS_ITMH_BASE *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  LOG_CLI((BSL_META_U(unit,
                      " Pp_header_present: %d\n\r"),info->pp_header_present));
  LOG_CLI((BSL_META_U(unit,
                      " Out_mirror_dis:    %d\n\r"),info->out_mirror_dis));
  LOG_CLI((BSL_META_U(unit,
                      " Snoop_cmd_ndx:     %u\n\r"),info->snoop_cmd_ndx));
  LOG_CLI((BSL_META_U(unit,
                      " Exclude_src:       %d\n\r"),info->exclude_src));
  LOG_CLI((BSL_META_U(unit,
                      " Tr_cls:            %u\n\r"),info->tr_cls));
  LOG_CLI((BSL_META_U(unit,
                      " Dp:                %u\n\r"),info->dp));
  LOG_CLI((BSL_META_U(unit,
                      " Destination: ")));
  DNX_TMC_DEST_INFO_print(&(info->destination));
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  DNX_TMC_PORT_LAG_SYS_PORT_INFO_print(
    DNX_SAND_IN DNX_TMC_PORT_LAG_SYS_PORT_INFO *info
  )
{
  uint32
    ind;
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  if (info->in_member)
  {
    LOG_CLI((BSL_META_U(unit,
                        "In_lag:     %d\n\r"),info->in_lag));
  }
  else
  {
    LOG_CLI((BSL_META_U(unit,
                        "system port is not member in in-Lag\n\r")));
  }

  if (info->nof_of_out_lags == 0)
  {
    LOG_CLI((BSL_META_U(unit,
                        "system port is not member in Out-Lag\n\r")));
  }
  else
  {
    LOG_CLI((BSL_META_U(unit,
                        "Out_lags:")));
    for (ind=0; ind<DNX_TMC_NOF_LAG_GROUPS; ++ind)
    {
      if (info->out_lags[ind])
      {
        LOG_CLI((BSL_META_U(unit,
                            " %d"),ind));
        if (info->out_lags[ind] > 1)
        {
          LOG_CLI((BSL_META_U(unit,
                              "(%d)"),info->out_lags[ind]));
        }
      }
    }
    LOG_CLI((BSL_META_U(unit,
                        "\n\r")));
  }

exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  DNX_TMC_PORTS_ITMH_EXT_SRC_PORT_print(
    DNX_SAND_IN DNX_TMC_PORTS_ITMH_EXT_SRC_PORT *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  LOG_CLI((BSL_META_U(unit,
                      " Enable: %d\n\r"),info->enable));
  LOG_CLI((BSL_META_U(unit,
                      " Is_src_sys_port: %d\n\r"),info->is_src_sys_port));
  LOG_CLI((BSL_META_U(unit,
                      " Src_port: ")));
  DNX_TMC_DEST_SYS_PORT_INFO_print(&(info->src_port));
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  DNX_TMC_PORTS_ITMH_print(
    DNX_SAND_IN DNX_TMC_PORTS_ITMH *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  LOG_CLI((BSL_META_U(unit,
                      "Base:\n\r")));
  DNX_TMC_PORTS_ITMH_BASE_print(&(info->base));
  LOG_CLI((BSL_META_U(unit,
                      "Extension:\n\r")));
  DNX_TMC_PORTS_ITMH_EXT_SRC_PORT_print(&(info->extension));
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  DNX_TMC_PORTS_ISP_INFO_print(
    DNX_SAND_IN DNX_TMC_PORTS_ISP_INFO *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  LOG_CLI((BSL_META_U(unit,
                      "Enable:   %u\n\r"),info->enable));
  LOG_CLI((BSL_META_U(unit,
                      "Queue_id: %u\n\r"),info->queue_id));
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  DNX_TMC_PORTS_STAG_FIELDS_print(
    DNX_SAND_IN DNX_TMC_PORTS_STAG_FIELDS   *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  LOG_CLI((BSL_META_U(unit,
                      "Cid:       %d\n\r"),info->cid));
  LOG_CLI((BSL_META_U(unit,
                      "Ifp:       %d\n\r"),info->ifp));
  LOG_CLI((BSL_META_U(unit,
                      "Tr_cls:    %d\n\r"),info->tr_cls));
  LOG_CLI((BSL_META_U(unit,
                      "Dp:        %d\n\r"),info->dp));
  LOG_CLI((BSL_META_U(unit,
                      "Data_type: %d\n\r"),info->data_type));
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  DNX_TMC_PORTS_OTMH_EXTENSIONS_EN_print(
    DNX_SAND_IN DNX_TMC_PORTS_OTMH_EXTENSIONS_EN *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  LOG_CLI((BSL_META_U(unit,
                      "Outlif_ext_en: %s\n\r"),
           DNX_TMC_PORTS_FTMH_EXT_OUTLIF_to_string(info->outlif_ext_en)
           ));
  LOG_CLI((BSL_META_U(unit,
                      "Src_ext_en:    %d\n\r"),info->src_ext_en));
  LOG_CLI((BSL_META_U(unit,
                      "Dest_ext_en:   %d\n\r"),info->dest_ext_en));
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  DNX_TMC_PORT_EGR_HDR_CR_DISCOUNT_INFO_print(
    DNX_SAND_IN DNX_TMC_PORT_EGR_HDR_CR_DISCOUNT_INFO *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  LOG_CLI((BSL_META_U(unit,
                      "Uc_credit_discount: %d[Bytes]\n\r"),info->uc_credit_discount));
  LOG_CLI((BSL_META_U(unit,
                      "Mc_credit_discount: %d[Bytes]\n\r"),info->mc_credit_discount));
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

const char*
  DNX_TMC_PORTS_FC_TYPE_to_string(
    DNX_SAND_IN  DNX_TMC_PORTS_FC_TYPE enum_val
  )
{
  const char* str = NULL;
  switch(enum_val)
  {
  case DNX_TMC_PORTS_FC_TYPE_NONE:
    str = "none";
    break;
  case DNX_TMC_PORTS_FC_TYPE_LL:
    str = "ll";
  break;
  case DNX_TMC_PORTS_FC_TYPE_CB2:
    str = "cb2";
  break;
  case DNX_TMC_PORTS_FC_TYPE_CB8:
    str = "cb8";
  break;
  default:
    str = " Unknown Enumerator Value";
  }
  return str;
}

void
  DNX_TMC_PORT_PP_PORT_INFO_print(
    DNX_SAND_IN  DNX_TMC_PORT_PP_PORT_INFO *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  LOG_CLI((BSL_META_U(unit,
                      "is_stag_enabled: %u, "),info->is_stag_enabled));
  LOG_CLI((BSL_META_U(unit,
                      "first_header_size: %u, "),info->first_header_size));
  LOG_CLI((BSL_META_U(unit,
                      "fc_type %s, "), DNX_TMC_PORTS_FC_TYPE_to_string(info->fc_type)));
  LOG_CLI((BSL_META_U(unit,
                      "header_type %s \n\r"), DNX_TMC_PORT_HEADER_TYPE_to_string(info->header_type)));
  LOG_CLI((BSL_META_U(unit,
                      "header_type_out %s \n\r"), DNX_TMC_PORT_HEADER_TYPE_to_string(info->header_type_out)));
  LOG_CLI((BSL_META_U(unit,
                      "    is_snoop_enabled: %u, "),info->is_snoop_enabled));
  LOG_CLI((BSL_META_U(unit,
                      "is_tm_ing_shaping_enabled: %u, "),info->is_tm_ing_shaping_enabled));
  LOG_CLI((BSL_META_U(unit,
                      "is_tm_pph_present_enabled: %u, "),info->is_tm_pph_present_enabled));
  LOG_CLI((BSL_META_U(unit,
                      "is_tm_src_syst_port_ext_present: %u, "),info->is_tm_src_syst_port_ext_present));
  LOG_CLI((BSL_META_U(unit,
                      "mirror_profile: %u\n\r"),info->mirror_profile));
  LOG_CLI((BSL_META_U(unit,
                      "flags:\n\r")));
  if (info->flags & DNX_TMC_PORT_PP_PORT_RCY_OVERLAY_PTCH) {
      LOG_CLI((BSL_META_U(unit,
                          "    DNX_TMC_PORT_PP_PORT_RCY_OVERLAY_PTCH\n\r")));
  }
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  DNX_TMC_PORT_COUNTER_INFO_print(
    DNX_SAND_IN  DNX_TMC_PORT_COUNTER_INFO *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  LOG_CLI((BSL_META_U(unit,
                      "processor_id %s "), DNX_TMC_CNT_PROCESSOR_ID_to_string(info->processor_id)));
  LOG_CLI((BSL_META_U(unit,
                      "id: %u\n\r"),info->id));
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  DNX_TMC_PORTS_FORWARDING_HEADER_INFO_print(
    DNX_SAND_IN  DNX_TMC_PORTS_FORWARDING_HEADER_INFO *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  LOG_CLI((BSL_META_U(unit,
                      "destination:")));
  DNX_TMC_DEST_INFO_print(&(info->destination));
  LOG_CLI((BSL_META_U(unit,
                      "tr_cls: %u\n\r"),info->tr_cls));
  LOG_CLI((BSL_META_U(unit,
                      "dp: %u\n\r"),info->dp));
  LOG_CLI((BSL_META_U(unit,
                      "snoop_cmd_ndx: %u\n\r"),info->snoop_cmd_ndx));
  LOG_CLI((BSL_META_U(unit,
                      "counter:")));
  DNX_TMC_PORT_COUNTER_INFO_print(&(info->counter));
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  DNX_TMC_PORT_INJECTED_CPU_HEADER_INFO_print(
    DNX_SAND_IN  DNX_TMC_PORT_INJECTED_CPU_HEADER_INFO *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  LOG_CLI((BSL_META_U(unit,
                      "hdr_format %s "), DNX_TMC_PORT_INJECTED_HDR_FORMAT_to_string(info->hdr_format)));
  LOG_CLI((BSL_META_U(unit,
                      "pp_port_for_tm_traffic: %u\n\r"),info->pp_port_for_tm_traffic));
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  DNX_TMC_PORTS_SWAP_INFO_print(
    DNX_SAND_IN  DNX_TMC_PORTS_SWAP_INFO *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  LOG_CLI((BSL_META_U(unit,
                      "enable: %d\n\r"),info->enable));
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  DNX_TMC_PORTS_PON_TUNNEL_INFO_print(
    DNX_SAND_IN  DNX_TMC_PORTS_PON_TUNNEL_INFO *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  LOG_CLI((BSL_META_U(unit,
                      "pp_port: %u\n\r"),info->pp_port));
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  DNX_TMC_PORTS_PROGRAMS_INFO_print(
    DNX_SAND_IN  DNX_TMC_PORTS_PROGRAMS_INFO *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  LOG_CLI((BSL_META_U(unit,
                      "ptc_vt_profile %s "), DNX_TMC_PORTS_VT_PROFILE_to_string(info->ptc_vt_profile)));
  LOG_CLI((BSL_META_U(unit,
                      "ptc_tt_profile %s "), DNX_TMC_PORTS_TT_PROFILE_to_string(info->ptc_tt_profile)));
  LOG_CLI((BSL_META_U(unit,
                      "ptc_flp_profile %s "), DNX_TMC_PORTS_FLP_PROFILE_to_string(info->ptc_flp_profile)));
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

#endif /* SOC_PB_DEBUG_IS_LVL1 */

/* } */

#include <soc/dnx/legacy/SAND/Utils/sand_footer.h>

