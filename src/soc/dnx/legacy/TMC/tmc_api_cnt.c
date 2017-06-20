/* $Id: jer2_tmc_api_cnt.c,v 1.12 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/
/******************************************************************
*
* FILENAME:       DuneDriver/jer2_tmc/src/soc_jer2_tmcapi_cnt.c
*
* MODULE PREFIX:  jer2_tmc
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

#include <soc/dnx/legacy/SAND/Management/sand_general_macros.h>
#include <soc/dnx/legacy/SAND/Management/sand_error_code.h>
#include <soc/dnx/legacy/SAND/Utils/sand_os_interface.h>

#include <soc/dnx/legacy/TMC/tmc_api_cnt.h>

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
  DNX_TMC_CNT_CUSTOM_MODE_PARAMS_clear(
    DNX_SAND_OUT DNX_TMC_CNT_CUSTOM_MODE_PARAMS *info
  )
{
  int i;
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  info->set_size = 0;
  for( i = 0 ; i < DNX_TMC_CNT_BMAP_OFFSET_COUNT ; i++){
      info->entries_bmaps[i] = 0;
  }
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}




void
  DNX_TMC_CNT_OVERFLOW_clear(
    DNX_SAND_OUT DNX_TMC_CNT_OVERFLOW *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  sal_memset(info, 0x0, sizeof(DNX_TMC_CNT_OVERFLOW));
  info->is_overflow = 0;
  info->last_cnt_id = 0;
  info->is_pckt_overflow = 0;
  info->is_byte_overflow = 0;
  DNX_SAND_MAGIC_NUM_SET;
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  DNX_TMC_CNT_STATUS_clear(
    DNX_SAND_OUT DNX_TMC_CNT_STATUS *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  sal_memset(info, 0x0, sizeof(DNX_TMC_CNT_STATUS));
  info->is_cache_full = 0;
  DNX_TMC_CNT_OVERFLOW_clear(&(info->overflow_cnt));
  info->nof_active_cnts = 0;
  info->is_cnt_id_invalid = 0;
  DNX_SAND_MAGIC_NUM_SET;
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  DNX_TMC_CNT_RESULT_clear(
    DNX_SAND_OUT DNX_TMC_CNT_RESULT *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  sal_memset(info, 0x0, sizeof(DNX_TMC_CNT_RESULT));
  info->counter_id = 0;
  COMPILER_64_ZERO(info->pkt_cnt);
  COMPILER_64_ZERO(info->byte_cnt);
  DNX_SAND_MAGIC_NUM_SET;
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  DNX_TMC_CNT_RESULT_ARR_clear(
    DNX_SAND_OUT DNX_TMC_CNT_RESULT_ARR *info
  )
{

  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  info->nof_counters = 0;
  DNX_SAND_MAGIC_NUM_SET;
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
DNX_TMC_CNT_MODE_EG_clear(
  DNX_SAND_OUT DNX_TMC_CNT_MODE_EG *info
  )
{
    DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
    DNX_SAND_CHECK_NULL_INPUT(info);

    sal_memset(info, 0x0, sizeof(DNX_TMC_CNT_MODE_EG));

    info->resolution = DNX_TMC_CNT_NOF_MODE_EGS;
    info->type = DNX_TMC_CNT_NOF_MODE_EG_TYPES;
    info->base_val = 0;

exit:
    DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}


#if DNX_TMC_DEBUG_IS_LVL1

const char*
  DNX_TMC_CNT_PROCESSOR_ID_to_string(
    DNX_SAND_IN  DNX_TMC_CNT_PROCESSOR_ID enum_val
  )
{
  const char* str = NULL;
  switch(enum_val)
  {
  case DNX_TMC_CNT_PROCESSOR_ID_A:
    str = "a";
  break;
  case DNX_TMC_CNT_PROCESSOR_ID_B:
    str = "b";
  break;
  default:
    str = " Unknown";
  }
  return str;
}

const char*
  DNX_TMC_CNT_MODE_STATISTICS_to_string(
    DNX_SAND_IN  DNX_TMC_CNT_MODE_STATISTICS enum_val
  )
{
  const char* str = NULL;
  switch(enum_val)
  {
  case DNX_TMC_CNT_MODE_STATISTICS_NO_COLOR:
    str = "no_color";
  break;
  case DNX_TMC_CNT_MODE_STATISTICS_COLOR_RES_LOW:
    str = "color_res_low";
  break;
  case DNX_TMC_CNT_MODE_STATISTICS_COLOR_RES_HI:
    str = "color_res_hi";
  break;
  case DNX_TMC_CNT_MODE_STATISTICS_COLOR_RES_ENQ_HI:
    str = "enqueue_hi(green,not_green)";
  break;
  case DNX_TMC_CNT_MODE_STATISTICS_FWD_NO_COLOR:
    str = "fwd_no_color";
  break;
  case DNX_TMC_CNT_MODE_STATISTICS_DROP_NO_COLOR:
    str = "drop_no_color";
  break;
  case DNX_TMC_CNT_MODE_STATISTICS_ALL_NO_COLOR:
    str = "all_no_color";
  break;
  case DNX_TMC_CNT_MODE_STATISTICS_FWD_SIMPLE_COLOR:
    str = "simple_no_color(fwd_green,fwd_not_green)";
    break;
  case DNX_TMC_CNT_MODE_STATISTICS_DROP_SIMPLE_COLOR:
    str = "simple_no_color(drop_green,drop_not_green)";
  break;
  case DNX_TMC_CNT_MODE_STATISTICS_CONFIGURABLE_OFFSETS:
    str = "configurable";
  break;
  default:
    str = " Unknown";
  }
  return str;
}

const char*
  DNX_TMC_CNT_MODE_EG_to_string(
    DNX_SAND_IN  DNX_TMC_CNT_MODE_EG_RES enum_val
  )
{
  const char* str = NULL;
  switch(enum_val)
  {
  case DNX_TMC_CNT_MODE_EG_RES_NO_COLOR:
    str = "no_color";
  break;
  case DNX_TMC_CNT_MODE_EG_RES_COLOR:
    str = "color";
  break;
  default:
    str = " Unknown";
  }
  return str;
}

const char*
  DNX_TMC_CNT_SRC_TYPE_to_string(
    DNX_SAND_IN  DNX_TMC_CNT_SRC_TYPE enum_val
  )
{
  const char* str = NULL;
  switch(enum_val)
  {
  case DNX_TMC_CNT_SRC_TYPE_ING_PP:
    str = "ing_pp";
  break;
  case DNX_TMC_CNT_SRC_TYPE_VOQ:
    str = "voq";
  break;
  case DNX_TMC_CNT_SRC_TYPE_STAG:
    str = "stag";
  break;
  case DNX_TMC_CNT_SRC_TYPE_VSQ:
    str = "vsq";
  break;
  case DNX_TMC_CNT_SRC_TYPE_CNM_ID:
    str = "cnm_id";
  break;
  case DNX_TMC_CNT_SRC_TYPE_EGR_PP:
    str = "egr_pp";
  break;
  case DNX_TMC_CNT_SRC_TYPE_OAM:
    str = "oam";
  break;
  case DNX_TMC_CNT_SRC_TYPE_EPNI:
    str = "epni";
  break;
  case DNX_TMC_CNT_SRC_TYPES_EGQ_TM:
      str = "egq-tm";
  break;
  default:
    str = "Unknown";
  }
  return str;
}
const char*
  DNX_TMC_CNT_MODE_EG_TYPE_to_string(
    DNX_SAND_IN  DNX_TMC_CNT_MODE_EG_TYPE enum_val
  )
{
  const char* str = NULL;
  switch(enum_val)
  {
  case DNX_TMC_CNT_MODE_EG_TYPE_TM:
    str = "tm";
  break;
  case DNX_TMC_CNT_MODE_EG_TYPE_VSI:
    str = "vsi";
  break;
  case DNX_TMC_CNT_MODE_EG_TYPE_OUTLIF:
    str = "outlif";
  break;
  case DNX_TMC_CNT_MODE_EG_TYPE_PMF:
    str = "pmf";
  break;
  default:
    str = " Unknown";
  }
  return str;
}

const char*
  DNX_TMC_CNT_Q_SET_SIZE_to_string(
    DNX_SAND_IN  DNX_TMC_CNT_Q_SET_SIZE enum_val
  )
{
  const char* str = NULL;
  switch(enum_val)
  {
  case DNX_TMC_CNT_Q_SET_SIZE_1_Q:
    str = "size_1_q";
  break;
  case DNX_TMC_CNT_Q_SET_SIZE_2_Q:
    str = "size_2_q";
  break;
  case DNX_TMC_CNT_Q_SET_SIZE_4_Q:
    str = "size_4_q";
  break;
  case DNX_TMC_CNT_Q_SET_SIZE_8_Q:
    str = "size_8_q";
  break;
  default:
    str = " Unknown";
  }
  return str;
}
const char*
  DNX_TMC_CNT_FORMAT_to_string(
    DNX_SAND_IN  DNX_TMC_CNT_FORMAT enum_val
  )
{
  const char* str = NULL;
  switch(enum_val)
  {
  case DNX_TMC_CNT_FORMAT_PKTS_AND_BYTES:
    str = "PKTS_AND_BYTES";
  break;
  case DNX_TMC_CNT_FORMAT_PKTS:
    str = "PKTS";
  break;
  case DNX_TMC_CNT_FORMAT_BYTES:
    str = "BYTES";
  break;
  case DNX_TMC_CNT_FORMAT_MAX_QUEUE_SIZE:
    str = "MAX_QUEUE_SIZE";
  break;
  case DNX_TMC_CNT_FORMAT_PKTS_AND_PKTS:
	str = "PKTS_AND_PKTS";
  break;
  default:
    str = " Unknown";
  }
  return str;
}

void
  DNX_TMC_CNT_Q_SET_SIZE_print(
    DNX_SAND_IN  DNX_TMC_CNT_Q_SET_SIZE q_set_size
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);

  LOG_CLI((BSL_META_U(unit,
                      "q_set_size %s "), DNX_TMC_CNT_Q_SET_SIZE_to_string(q_set_size)));

  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

const char*
  DNX_TMC_CNT_BMAP_OFFSET_MAPPING_to_string(
    DNX_SAND_IN  DNX_TMC_CNT_BMAP_OFFSET_MAPPING enum_val
  )
{
  const char* str = NULL;
  switch(enum_val)
  {
  case DNX_TMC_CNT_BMAP_OFFSET_GREEN_FWD:
    str = "green fwd";
  break;
  case DNX_TMC_CNT_BMAP_OFFSET_GREEN_DROP:
    str = "green drop";
  break;
  case DNX_TMC_CNT_BMAP_OFFSET_YELLOW_FWD:
    str = "yellow fwd";
  break;
  case DNX_TMC_CNT_BMAP_OFFSET_YELLOW_DROP:
    str = "yellow drop";
  break;
  case DNX_TMC_CNT_BMAP_OFFSET_RED_FWD:
    str = "red fwd";
  break;
  case DNX_TMC_CNT_BMAP_OFFSET_RED_DROP:
    str = "red drop";
  break;
  case DNX_TMC_CNT_BMAP_OFFSET_BLACK_FWD:
    str = "black fwd";
  break;
  case DNX_TMC_CNT_BMAP_OFFSET_BLACK_DROP:
    str = "black drop";
  break;
  default:
    str = "Unknown";
  }
  return str;
}

void
  DNX_TMC_CNT_CUSTOM_MODE_PARAMS_print(
    DNX_SAND_IN  DNX_TMC_CNT_CUSTOM_MODE_PARAMS *info
  )
{
  uint32 i, j;
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  LOG_CLI((BSL_META_U(unit,
                      "number of counters in set: %u\n\r"),info->set_size));
  for(i = 0 ; i < info->set_size ; i++){
      LOG_CLI((BSL_META_U(unit,
                          "counter %u(%u):"),i, info->entries_bmaps[i]));
      for(j = 0 ; j < DNX_TMC_CNT_BMAP_OFFSET_COUNT; j++){
          if(SHR_BITGET(&(info->entries_bmaps[i]),j)){
              LOG_CLI((BSL_META_U(unit,
                                  " %s"), DNX_TMC_CNT_BMAP_OFFSET_MAPPING_to_string(j)));
          }
      }
      LOG_CLI((BSL_META_U(unit,
                          "\r\n")));

  }
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}


void DNX_TMC_CNT_MODE_EG_print
(
 DNX_SAND_IN  DNX_TMC_CNT_MODE_EG *info
 )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  LOG_CLI((BSL_META_U(unit,
                      "Egress base_val: %u\n\r"),info->base_val));
  if (info->resolution == DNX_TMC_CNT_MODE_EG_RES_COLOR)
  {
    LOG_CLI((BSL_META_U(unit,
                        "Egress resolution: COLOR\n\r")));
  }
  else
  {
    LOG_CLI((BSL_META_U(unit,
                        "Egress resolution: NO COLOR\n\r")));
  }
  switch (info->type)
  {


  case DNX_TMC_CNT_MODE_EG_TYPE_TM:

    LOG_CLI((BSL_META_U(unit,
                        "Egress type: TM\n\r")));
    break;

  case DNX_TMC_CNT_MODE_EG_TYPE_VSI:

    LOG_CLI((BSL_META_U(unit,
                        "Egress type: VSI\n\r")));

    break;

  case DNX_TMC_CNT_MODE_EG_TYPE_OUTLIF :

    LOG_CLI((BSL_META_U(unit,
                        "Egress type: OUTLIF\n\r")));

    break;

  default:

    break;

  }
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  DNX_TMC_CNT_COUNTERS_INFO_print(
    DNX_SAND_IN  DNX_TMC_CNT_COUNTERS_INFO *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  LOG_CLI((BSL_META_U(unit,
                      "src_type %s "), DNX_TMC_CNT_SRC_TYPE_to_string(info->src_type)));
  LOG_CLI((BSL_META_U(unit,
                      "mode_ing %s "), DNX_TMC_CNT_MODE_STATISTICS_to_string(info->mode_statistics)));
  DNX_TMC_CNT_MODE_EG_print(&(info->mode_eg));
  LOG_CLI((BSL_META_U(unit,
                      "voq_cnt:")));
  DNX_TMC_CNT_Q_SET_SIZE_print(info->q_set_size);
  LOG_CLI((BSL_META_U(unit,
                      "stag_lsb: %u\n\r"),info->stag_lsb));
  LOG_CLI((BSL_META_U(unit,
                      "number of counters in engine: %u\n\r"),info->num_counters));
  LOG_CLI((BSL_META_U(unit,
                      "number of counters-sets in engine: %u\n\r"),info->num_sets));
  if(info->mode_statistics == DNX_TMC_CNT_MODE_STATISTICS_CONFIGURABLE_OFFSETS){
      DNX_TMC_CNT_CUSTOM_MODE_PARAMS_print(&info->custom_mode_params);
  }
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  DNX_TMC_CNT_OVERFLOW_print(
    DNX_SAND_IN  DNX_TMC_CNT_OVERFLOW *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  LOG_CLI((BSL_META_U(unit,
                      "is_overflow: %u\n\r"),info->is_overflow));
  LOG_CLI((BSL_META_U(unit,
                      "last_cnt_id: %u\n\r"),info->last_cnt_id));
  LOG_CLI((BSL_META_U(unit,
                      "is_pckt_overflow: %u\n\r"),info->is_pckt_overflow));
  LOG_CLI((BSL_META_U(unit,
                      "is_byte_overflow: %u\n\r"),info->is_byte_overflow));
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  DNX_TMC_CNT_STATUS_print(
    DNX_SAND_IN  DNX_TMC_CNT_STATUS *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  LOG_CLI((BSL_META_U(unit,
                      "is_cache_full: %u\n\r"),info->is_cache_full));
  LOG_CLI((BSL_META_U(unit,
                      "overflow_cnt:")));
  DNX_TMC_CNT_OVERFLOW_print(&(info->overflow_cnt));
  LOG_CLI((BSL_META_U(unit,
                      "nof_active_cnts: %u\n\r"),info->nof_active_cnts));
  LOG_CLI((BSL_META_U(unit,
                      "is_cnt_id_invalid: %u\n\r"),info->is_cnt_id_invalid));
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  DNX_TMC_CNT_RESULT_print(
    DNX_SAND_IN  DNX_TMC_CNT_RESULT *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  LOG_CLI((BSL_META_U(unit,
                      "counter_id: %u\n\r"),info->counter_id));
  LOG_CLI((BSL_META_U(unit,
                      "pkt_cnt: 0x%08x%08x[Packets]\n\r"), COMPILER_64_HI(info->pkt_cnt),COMPILER_64_LO(info->pkt_cnt)));
  LOG_CLI((BSL_META_U(unit,
                      "byte_cnt: 0x%08x%08x[Bytes]\n\r"), COMPILER_64_HI(info->byte_cnt),COMPILER_64_LO(info->byte_cnt)));
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  DNX_TMC_CNT_RESULT_ARR_print(
    DNX_SAND_IN  DNX_TMC_CNT_RESULT_ARR *info
  )
{
  uint32
    ind;

  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  for (ind = 0; ind < DNX_TMC_CNT_CACHE_LENGTH_JER2_ARAD; ++ind)
  {
    LOG_CLI((BSL_META_U(unit,
                        "cnt_result[%u]:"),ind));
    DNX_TMC_CNT_RESULT_print(&(info->cnt_result[ind]));
  }
  LOG_CLI((BSL_META_U(unit,
                      "nof_counters: %u\n\r"),info->nof_counters));
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
DNX_TMC_CNT_COUNTERS_INFO_clear(
                DNX_SAND_OUT int unit,
                DNX_SAND_OUT DNX_TMC_CNT_COUNTERS_INFO *info)
{
  DNX_SAND_INIT_ERROR_DEFINITIONS(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  sal_memset(info, 0x0, sizeof(DNX_TMC_CNT_COUNTERS_INFO));
  info->src_type = DNX_TMC_CNT_NOF_SRC_TYPES(unit);
  info->mode_statistics = DNX_TMC_CNT_NOF_MODE_INGS;
  info->mode_eg.base_val = 0;
  info->mode_eg.resolution = DNX_TMC_CNT_NOF_MODE_EGS;
  info->q_set_size = DNX_TMC_CNT_NOF_Q_SET_SIZES;
  info->command_id = -1;
  info->format = DNX_TMC_CNT_FORMAT_PKTS_AND_BYTES;
  DNX_TMC_CNT_MODE_EG_clear(&info->mode_eg);
  info->replicated_pkts = DNX_TMC_CNT_REPLICATED_PKTS_ALL;
  info->src_core = -1;
  info->we_val = 0;
  DNX_TMC_CNT_CUSTOM_MODE_PARAMS_clear(&(info->custom_mode_params));
  info->stag_lsb = 0;
  info->num_counters = 0;
  info->num_sets = 0;
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}





#endif /* DNX_TMC_DEBUG_IS_LVL1 */

/* } */

#include <soc/dnx/legacy/SAND/Utils/sand_footer.h>

