/* $Id: jer2_tmc_api_ingress_traffic_mgmt.c,v 1.12 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/
/******************************************************************
*
* FILENAME:       DuneDriver/jer2_tmc/src/soc_jer2_tmcapi_ingress_traffic_mgmt.c
*
* MODULE PREFIX:  soc_jer2_tmcitm
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

#include <soc/dnx/legacy/TMC/tmc_api_ingress_traffic_mgmt.h>
#include <soc/dnx/legacy/TMC/tmc_api_general.h>

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
  DNX_TMC_ITM_DRAM_BUFFERS_INFO_clear(
    DNX_SAND_OUT DNX_TMC_ITM_DRAM_BUFFERS_INFO *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  sal_memset(info, 0x0, sizeof(DNX_TMC_ITM_DRAM_BUFFERS_INFO));
  info->dbuff_size = DNX_TMC_ITM_NOF_DBUFF_SIZES;
  info->uc_nof_buffs = 0;
  info->full_mc_nof_buffs = 0;
  info->mini_mc_nof_buffs = 0;
  DNX_SAND_MAGIC_NUM_SET;
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  DNX_TMC_ITM_GLOB_RCS_FC_TYPE_clear(
    DNX_SAND_OUT DNX_TMC_ITM_GLOB_RCS_FC_TYPE *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  sal_memset(info, 0x0, sizeof(DNX_TMC_ITM_GLOB_RCS_FC_TYPE));
  DNX_TMC_THRESH_WITH_HYST_INFO_clear(&(info->hp));
  DNX_TMC_THRESH_WITH_HYST_INFO_clear(&(info->lp));
  DNX_SAND_MAGIC_NUM_SET;
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  DNX_TMC_ITM_GLOB_RCS_FC_TH_clear(
    DNX_SAND_OUT DNX_TMC_ITM_GLOB_RCS_FC_TH *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  sal_memset(info, 0x0, sizeof(DNX_TMC_ITM_GLOB_RCS_FC_TH));
  DNX_TMC_ITM_GLOB_RCS_FC_TYPE_clear(&(info->bdbs));
  DNX_TMC_ITM_GLOB_RCS_FC_TYPE_clear(&(info->unicast));
  DNX_TMC_ITM_GLOB_RCS_FC_TYPE_clear(&(info->full_mc));
  DNX_TMC_ITM_GLOB_RCS_FC_TYPE_clear(&(info->mini_mc));
  DNX_TMC_ITM_GLOB_RCS_FC_TYPE_clear(&(info->ocb));
  DNX_TMC_ITM_GLOB_RCS_FC_TYPE_clear(&(info->ocb_p0));
  DNX_TMC_ITM_GLOB_RCS_FC_TYPE_clear(&(info->ocb_p1));
  DNX_TMC_ITM_GLOB_RCS_FC_TYPE_clear(&(info->mix_p0));
  DNX_TMC_ITM_GLOB_RCS_FC_TYPE_clear(&(info->mix_p1));
  
  DNX_SAND_MAGIC_NUM_SET;
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  DNX_TMC_ITM_GLOB_RCS_DROP_TH_clear(
    DNX_SAND_OUT DNX_TMC_ITM_GLOB_RCS_DROP_TH *info
  )
{
  uint32
    ind, ind1;
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  sal_memset(info, 0x0, sizeof(DNX_TMC_ITM_GLOB_RCS_DROP_TH));
  for (ind=0; ind<DNX_TMC_NOF_DROP_PRECEDENCE; ++ind)
  {
    DNX_TMC_THRESH_WITH_HYST_INFO_clear(&(info->bdbs[ind]));
  }
  for (ind=0; ind<DNX_TMC_NOF_DROP_PRECEDENCE; ++ind)
  {
      DNX_TMC_THRESH_WITH_HYST_INFO_clear(&(info->ocb_bdbs[ind]));
  }
  for (ind=0; ind<DNX_TMC_NOF_DROP_PRECEDENCE; ++ind)
  {
    DNX_TMC_THRESH_WITH_HYST_INFO_clear(&(info->bds[ind]));
  }
  for (ind=0; ind<DNX_TMC_NOF_DROP_PRECEDENCE; ++ind)
  {
    DNX_TMC_THRESH_WITH_HYST_INFO_clear(&(info->ocb_bds[ind]));
  }
  for (ind=0; ind<DNX_TMC_NOF_DROP_PRECEDENCE; ++ind)
  {
    DNX_TMC_THRESH_WITH_HYST_INFO_clear(&(info->unicast[ind]));
  }
  for (ind=0; ind<DNX_TMC_NOF_DROP_PRECEDENCE; ++ind)
  {
    DNX_TMC_THRESH_WITH_HYST_INFO_clear(&(info->full_mc[ind]));
  }
  for (ind=0; ind<DNX_TMC_NOF_DROP_PRECEDENCE; ++ind)
  {
    DNX_TMC_THRESH_WITH_HYST_INFO_clear(&(info->mini_mc[ind]));
  }
  for (ind = 0; ind < DNX_TMC_NOF_DROP_PRECEDENCE; ++ind)
  {
    DNX_TMC_THRESH_WITH_HYST_INFO_clear(&(info->mem_excess[ind]));
  }
    for (ind = 0; ind < DNX_TMC_NOF_DROP_PRECEDENCE; ++ind)
  {
    DNX_TMC_THRESH_WITH_HYST_INFO_clear(&(info->ocb_mem_excess[ind]));
  }
  for (ind = 0; ind < DNX_TMC_NOF_DROP_PRECEDENCE; ++ind)
  {
    DNX_TMC_THRESH_WITH_HYST_INFO_clear(&(info->ocb_uc[ind]));
  }
  for (ind = 0; ind < DNX_TMC_NOF_DROP_PRECEDENCE; ++ind)
  {
    DNX_TMC_THRESH_WITH_HYST_INFO_clear(&(info->ocb_mc[ind]));
  }
  for (ind = 0; ind < DNX_TMC_NOF_DROP_PRECEDENCE; ++ind)
  {
    DNX_TMC_THRESH_WITH_HYST_INFO_clear(&(info->ocb_shrd_pool[0][ind]));
  }
  for (ind = 0; ind < DNX_TMC_NOF_DROP_PRECEDENCE; ++ind)
  {
    DNX_TMC_THRESH_WITH_HYST_INFO_clear(&(info->ocb_shrd_pool[1][ind]));
  }
  for (ind = 0; ind < DNX_TMC_NOF_DROP_PRECEDENCE; ++ind)
  {
    DNX_TMC_THRESH_WITH_HYST_INFO_clear(&(info->mix_shrd_pool[0][ind]));
  }
  for (ind = 0; ind < DNX_TMC_NOF_DROP_PRECEDENCE; ++ind)
  {
    DNX_TMC_THRESH_WITH_HYST_INFO_clear(&(info->mix_shrd_pool[1][ind]));
  }
  for (ind1 = 0; ind1 < DNX_TMC_INGRESS_THRESHOLD_NOF_SRAM_TYPES; ++ind1)
  {
      for (ind = 0; ind < DNX_TMC_NOF_DROP_PRECEDENCE; ++ind)
      {
        DNX_TMC_THRESH_WITH_HYST_INFO_clear(&(info->global_free_sram[ind1][ind]));
        DNX_TMC_THRESH_WITH_HYST_INFO_clear(&(info->global_free_sram_only[ind1][ind]));
      }
  }
  for (ind = 0; ind < DNX_TMC_NOF_DROP_PRECEDENCE; ++ind)
  {
    DNX_TMC_THRESH_WITH_HYST_INFO_clear(&(info->global_free_dram[ind]));
  }
  DNX_SAND_MAGIC_NUM_SET;
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  DNX_TMC_ITM_QUEUE_INFO_clear(
    DNX_SAND_OUT DNX_TMC_ITM_QUEUE_INFO *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  sal_memset(info, 0x0, sizeof(DNX_TMC_ITM_QUEUE_INFO));
  info->cr_req_type_ndx = DNX_TMC_ITM_NOF_QT_NDXS;
  info->credit_cls = 0;
  info->rate_cls = 0;
  info->vsq_connection_cls = 0;
  info->vsq_traffic_cls = 0;
  info->signature = 0;
  DNX_SAND_MAGIC_NUM_SET;
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  DNX_TMC_ITM_CATEGORY_RNGS_clear(
    DNX_SAND_OUT DNX_TMC_ITM_CATEGORY_RNGS *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  sal_memset(info, 0x0, sizeof(DNX_TMC_ITM_CATEGORY_RNGS));
  info->vsq_ctgry0_end = 0;
  info->vsq_ctgry1_end = 0;
  info->vsq_ctgry2_end = 0;
  DNX_SAND_MAGIC_NUM_SET;
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  DNX_TMC_ITM_ADMIT_ONE_TEST_TMPLT_clear(
    DNX_SAND_OUT DNX_TMC_ITM_ADMIT_ONE_TEST_TMPLT *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  sal_memset(info, 0x0, sizeof(DNX_TMC_ITM_ADMIT_ONE_TEST_TMPLT));
  info->ctgry_test_en = 0;
  info->ctgry_trffc_test_en = 0;
  info->ctgry2_3_cnctn_test_en = 0;
  info->sttstcs_tag_test_en = 0;
  info->pfc_test_en = 0;
  info->llfc_test_en = 0;
  DNX_SAND_MAGIC_NUM_SET;
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  DNX_TMC_ITM_ADMIT_TEST_TMPLT_INFO_clear(
    DNX_SAND_OUT DNX_TMC_ITM_ADMIT_TEST_TMPLT_INFO *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  sal_memset(info, 0x0, sizeof(DNX_TMC_ITM_ADMIT_TEST_TMPLT_INFO));
  DNX_TMC_ITM_ADMIT_ONE_TEST_TMPLT_clear(&(info->test_a));
  DNX_TMC_ITM_ADMIT_ONE_TEST_TMPLT_clear(&(info->test_b));
  DNX_SAND_MAGIC_NUM_SET;
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  DNX_TMC_ITM_CR_REQUEST_HUNGRY_TH_clear(
    DNX_SAND_OUT DNX_TMC_ITM_CR_REQUEST_HUNGRY_TH *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  sal_memset(info, 0x0, sizeof(DNX_TMC_ITM_CR_REQUEST_HUNGRY_TH));
  info->off_to_slow_th = 0;
  info->off_to_normal_th = 0;
  info->slow_to_normal_th = 0;
  info->normal_to_slow_th = 0;
  info->multiplier = 0;
  DNX_SAND_MAGIC_NUM_SET;
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  DNX_TMC_ITM_CR_REQUEST_BACKOFF_TH_clear(
    DNX_SAND_OUT DNX_TMC_ITM_CR_REQUEST_BACKOFF_TH *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  sal_memset(info, 0x0, sizeof(DNX_TMC_ITM_CR_REQUEST_BACKOFF_TH));
  info->backoff_enter_th = 0;
  info->backoff_exit_th = 0;
  DNX_SAND_MAGIC_NUM_SET;
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  DNX_TMC_ITM_CR_REQUEST_BACKLOG_TH_clear(
    DNX_SAND_OUT DNX_TMC_ITM_CR_REQUEST_BACKLOG_TH *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  sal_memset(info, 0x0, sizeof(DNX_TMC_ITM_CR_REQUEST_BACKLOG_TH));
  info->backlog_enter_th = 0;
  info->backlog_exit_th = 0;
  DNX_SAND_MAGIC_NUM_SET;
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  DNX_TMC_ITM_CR_REQUEST_EMPTY_Q_TH_clear(
    DNX_SAND_OUT DNX_TMC_ITM_CR_REQUEST_EMPTY_Q_TH *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  sal_memset(info, 0x0, sizeof(DNX_TMC_ITM_CR_REQUEST_EMPTY_Q_TH));
  info->satisfied_empty_q_th = 0;
  info->max_credit_balance_empty_q = 0;
  info->exceed_max_empty_q = 0;
  DNX_SAND_MAGIC_NUM_SET;
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  DNX_TMC_ITM_CR_REQUEST_SATISFIED_TH_clear(
    DNX_SAND_OUT DNX_TMC_ITM_CR_REQUEST_SATISFIED_TH *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  sal_memset(info, 0x0, sizeof(DNX_TMC_ITM_CR_REQUEST_SATISFIED_TH));
  DNX_TMC_ITM_CR_REQUEST_BACKOFF_TH_clear(&(info->backoff_th));
  DNX_TMC_ITM_CR_REQUEST_BACKLOG_TH_clear(&(info->backlog_th));
  DNX_TMC_ITM_CR_REQUEST_EMPTY_Q_TH_clear(&(info->empty_queues));
  DNX_SAND_MAGIC_NUM_SET;
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  DNX_TMC_ITM_CR_WD_Q_TH_clear(
    DNX_SAND_OUT DNX_TMC_ITM_CR_WD_Q_TH *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  sal_memset(info, 0x0, sizeof(DNX_TMC_ITM_CR_WD_Q_TH));
  info->cr_wd_stts_msg_gen = 0;
  info->cr_wd_dlt_q_th = 0;
  DNX_SAND_MAGIC_NUM_SET;
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  DNX_TMC_ITM_CR_REQUEST_INFO_clear(
    DNX_SAND_OUT DNX_TMC_ITM_CR_REQUEST_INFO *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  sal_memset(info, 0x0, sizeof(DNX_TMC_ITM_CR_REQUEST_INFO));
  DNX_TMC_ITM_CR_WD_Q_TH_clear(&(info->wd_th));
  DNX_TMC_ITM_CR_REQUEST_HUNGRY_TH_clear(&(info->hungry_th));
  DNX_TMC_ITM_CR_REQUEST_SATISFIED_TH_clear(&(info->satisfied_th));
  info->is_low_latency = FALSE;
  info->is_remote_credit_value = FALSE;
  DNX_SAND_MAGIC_NUM_SET;
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  DNX_TMC_ITM_CR_DISCOUNT_INFO_clear(
    DNX_SAND_OUT DNX_TMC_ITM_CR_DISCOUNT_INFO *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  sal_memset(info, 0x0, sizeof(DNX_TMC_ITM_CR_DISCOUNT_INFO));
  info->discount = 0;
  DNX_SAND_MAGIC_NUM_SET;
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  DNX_TMC_ITM_WRED_QT_DP_INFO_clear(
    DNX_SAND_OUT DNX_TMC_ITM_WRED_QT_DP_INFO *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  sal_memset(info, 0x0, sizeof(DNX_TMC_ITM_WRED_QT_DP_INFO));
  info->ignore_packet_size = 1;
  info->min_avrg_th = 0;
  info->max_avrg_th = 0;
  info->max_packet_size = 0;
  info->max_probability = 100;
  info->wred_en = 0;
  DNX_SAND_MAGIC_NUM_SET;
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  DNX_TMC_ITM_TAIL_DROP_INFO_clear(
    DNX_SAND_OUT DNX_TMC_ITM_TAIL_DROP_INFO *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  sal_memset(info, 0x0, sizeof(DNX_TMC_ITM_TAIL_DROP_INFO));
  info->max_inst_q_size = 0;
  info->max_inst_q_size_bds = 0;
  DNX_SAND_MAGIC_NUM_SET;
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  DNX_TMC_ITM_FADT_DROP_INFO_clear(
    DNX_SAND_OUT DNX_TMC_ITM_FADT_DROP_INFO *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  sal_memset(info, 0x0, sizeof(DNX_TMC_ITM_FADT_DROP_INFO));

exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  DNX_TMC_ITM_DRAM_BOUND_INFO_clear(
    DNX_SAND_OUT DNX_TMC_ITM_DRAM_BOUND_INFO *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  sal_memset(info, 0x0, sizeof(DNX_TMC_ITM_DRAM_BOUND_INFO));

exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

DNX_TMC_ITM_DRAM_BOUND_THRESHOLD* DNX_TMC_ITM_DRAM_BOUND_INFO_thresh_get(
    int                 unit,
    DNX_TMC_ITM_DRAM_BOUND_INFO* info,
    DNX_TMC_INGRESS_DRAM_BOUND_TYPE_E dram_thresh,
    DNX_TMC_INGRESS_THRESHOLD_TYPE_E resource_type)
{
    DNX_TMC_ITM_DRAM_BOUND_THRESHOLD* thresh = NULL;

    switch (resource_type) {
        case DNX_TMC_INGRESS_THRESHOLD_SRAM_BYTES:
            thresh = &info->sram_words_dram_threshold[dram_thresh];
            break;
        case DNX_TMC_INGRESS_THRESHOLD_SRAM_PDS:
            thresh = &info->sram_pds_dram_threshold[dram_thresh];
            break;
        default:
            break;
    }

    return thresh;
}

void
  DNX_TMC_ITM_VSQ_PG_PRM_clear (
     DNX_TMC_ITM_VSQ_PG_PRM  *info
  )
{
    DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
    DNX_SAND_CHECK_NULL_INPUT(info);
    sal_memset(info, 0x0, sizeof(DNX_TMC_ITM_VSQ_PG_PRM));
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);

}

void
  DNX_TMC_ITM_VSQ_SRC_PORT_INFO_clear (
     DNX_TMC_ITM_VSQ_SRC_PORT_INFO  *info
  )
{
    DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
    DNX_SAND_CHECK_NULL_INPUT(info);
    sal_memset(info, 0x0, sizeof(DNX_TMC_ITM_VSQ_SRC_PORT_INFO));
exit:
    DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  DNX_TMC_ITM_VSQ_PG_INFO_clear (
     DNX_TMC_ITM_VSQ_PG_INFO  *info
  )
{
    DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
    DNX_SAND_CHECK_NULL_INPUT(info);
    sal_memset(info, 0x0, sizeof(DNX_TMC_ITM_VSQ_PG_INFO));
exit:
    DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  DNX_TMC_ITM_CR_WD_INFO_clear(
    DNX_SAND_OUT DNX_TMC_ITM_CR_WD_INFO *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  sal_memset(info, 0x0, sizeof(DNX_TMC_ITM_CR_WD_INFO));
  info->min_scan_cycle_period_micro = 0;
  info->max_flow_msg_gen_rate_nano = 0;
  info->bottom_queue = 0;
  info->top_queue = 0;
  DNX_SAND_MAGIC_NUM_SET;
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  DNX_TMC_ITM_VSQ_FC_INFO_clear(
    DNX_SAND_OUT DNX_TMC_ITM_VSQ_FC_INFO *info
  )
{
  int ind;
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  sal_memset(info, 0x0, sizeof(DNX_TMC_ITM_VSQ_FC_INFO));
  DNX_TMC_THRESH_WITH_HYST_INFO_clear(&(info->q_size_fc));
  DNX_TMC_THRESH_WITH_HYST_INFO_clear(&(info->bd_size_fc));
  for (ind = 0; ind < DNX_TMC_INGRESS_THRESHOLD_NOF_TYPES; ++ind) {
      DNX_TMC_THRESH_WITH_HYST_INFO_clear(&(info->size_fc[ind]));
  }
  DNX_SAND_MAGIC_NUM_SET;
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  DNX_TMC_ITM_VSQ_TAIL_DROP_INFO_clear(
    DNX_SAND_OUT DNX_TMC_ITM_VSQ_TAIL_DROP_INFO *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  sal_memset(info, 0x0, sizeof(DNX_TMC_ITM_VSQ_TAIL_DROP_INFO));
  info->max_inst_q_size = 0;
  info->max_inst_q_size_bds = 0;
  DNX_SAND_MAGIC_NUM_SET;
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  DNX_TMC_ITM_VSQ_WRED_GEN_INFO_clear(
    DNX_SAND_OUT DNX_TMC_ITM_VSQ_WRED_GEN_INFO *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  sal_memset(info, 0x0, sizeof(DNX_TMC_ITM_VSQ_WRED_GEN_INFO));
  info->wred_en = 0;
  info->exp_wq = 0;
  DNX_SAND_MAGIC_NUM_SET;
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  DNX_TMC_ITM_STAG_INFO_clear(
    DNX_SAND_OUT DNX_TMC_ITM_STAG_INFO *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  sal_memset(info, 0x0, sizeof(DNX_TMC_ITM_STAG_INFO));
  info->enable_mode = DNX_TMC_ITM_STAG_ENABLE_MODE_DISABLED;
  info->vsq_index_msb = 0;
  info->vsq_index_lsb = 0;
  info->dropp_en = 0;
  info->dropp_lsb = 0;
  DNX_SAND_MAGIC_NUM_SET;
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  DNX_TMC_ITM_INGRESS_SHAPE_Q_RANGE_clear(
    DNX_SAND_OUT DNX_TMC_ITM_INGRESS_SHAPE_Q_RANGE *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  sal_memset(info, 0x0, sizeof(DNX_TMC_ITM_INGRESS_SHAPE_Q_RANGE));
  info->q_num_low = 0;
  info->q_num_high = 0;
  DNX_SAND_MAGIC_NUM_SET;
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  DNX_TMC_ITM_INGRESS_SHAPE_INFO_clear(
    DNX_SAND_OUT DNX_TMC_ITM_INGRESS_SHAPE_INFO *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  sal_memset(info, 0x0, sizeof(DNX_TMC_ITM_INGRESS_SHAPE_INFO));
  info->enable = 0;
  DNX_TMC_ITM_INGRESS_SHAPE_Q_RANGE_clear(&(info->q_range));
  info->rate = 0;
  info->sch_port = 0;
  DNX_SAND_MAGIC_NUM_SET;
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  DNX_TMC_ITM_PRIORITY_MAP_TMPLT_clear(
    DNX_SAND_OUT DNX_TMC_ITM_PRIORITY_MAP_TMPLT *info
  )
{
  uint32 ind;
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  sal_memset(info, 0x0, sizeof(DNX_TMC_ITM_PRIORITY_MAP_TMPLT));
  for (ind=0; ind<DNX_TMC_ITM_PRIO_MAP_SIZE_IN_UINT32S; ++ind)
  {
    info->map[ind] = 0;
  }
  DNX_SAND_MAGIC_NUM_SET;
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  DNX_TMC_ITM_SYS_RED_DROP_PROB_clear(
    DNX_SAND_OUT DNX_TMC_ITM_SYS_RED_DROP_PROB *info
  )
{
  uint32 ind;
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  sal_memset(info, 0x0, sizeof(DNX_TMC_ITM_SYS_RED_DROP_PROB));
  for (ind=0; ind<DNX_TMC_ITM_SYS_RED_DRP_PROBS; ++ind)
  {
    info->drop_probs[ind] = 0;
  }
  DNX_SAND_MAGIC_NUM_SET;
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  DNX_TMC_ITM_SYS_RED_QT_DP_INFO_clear(
    DNX_SAND_OUT DNX_TMC_ITM_SYS_RED_QT_DP_INFO *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  sal_memset(info, 0x0, sizeof(DNX_TMC_ITM_SYS_RED_QT_DP_INFO));
  info->enable = 0;
  info->adm_th = 0;
  info->prob_th = 0;
  info->drp_th = 0;
  info->drp_prob_low = 0;
  info->drp_prob_high = 0;
  DNX_SAND_MAGIC_NUM_SET;
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  DNX_TMC_ITM_SYS_RED_QT_INFO_clear(
    DNX_SAND_OUT DNX_TMC_ITM_SYS_RED_QT_INFO *info
  )
{
  uint32 ind;
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  sal_memset(info, 0x0, sizeof(DNX_TMC_ITM_SYS_RED_QT_INFO));
  for (ind=0; ind<DNX_TMC_ITM_SYS_RED_Q_SIZE_RANGES; ++ind)
  {
    info->queue_size_boundaries[ind] = 0;
  }
  DNX_SAND_MAGIC_NUM_SET;
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  DNX_TMC_ITM_SYS_RED_EG_INFO_clear(
    DNX_SAND_OUT DNX_TMC_ITM_SYS_RED_EG_INFO *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  sal_memset(info, 0x0, sizeof(DNX_TMC_ITM_SYS_RED_EG_INFO));
  info->enable = 0;
  info->aging_timer = 0;
  info->reset_expired_q_size = 0;
  info->aging_only_dec_q_size = 0;
  DNX_SAND_MAGIC_NUM_SET;
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  DNX_TMC_ITM_SYS_RED_GLOB_RCS_THS_clear(
    DNX_SAND_OUT DNX_TMC_ITM_SYS_RED_GLOB_RCS_THS *info
  )
{
  uint32 ind;
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  sal_memset(info, 0x0, sizeof(DNX_TMC_ITM_SYS_RED_GLOB_RCS_THS));
  for (ind=0; ind<DNX_TMC_ITM_SYS_RED_BUFFS_RNGS-1; ++ind)
  {
    info->unicast_rng_ths[ind] = 0;
  }
  for (ind=0; ind<DNX_TMC_ITM_SYS_RED_BUFFS_RNGS-1; ++ind)
  {
    info->multicast_rng_ths[ind] = 0;
  }
  for (ind=0; ind<DNX_TMC_ITM_SYS_RED_BUFFS_RNGS-1; ++ind)
  {
    info->bds_rng_ths[ind] = 0;
  }
  for (ind=0; ind<DNX_TMC_ITM_SYS_RED_BUFFS_RNGS-1; ++ind)
  {
    info->ocb_rng_ths[ind] = 0;
  }
  DNX_SAND_MAGIC_NUM_SET;
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  DNX_TMC_ITM_SYS_RED_GLOB_RCS_VALS_clear(
    DNX_SAND_OUT DNX_TMC_ITM_SYS_RED_GLOB_RCS_VALS *info
  )
{
  uint32 ind;
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  sal_memset(info, 0x0, sizeof(DNX_TMC_ITM_SYS_RED_GLOB_RCS_VALS));
  for (ind=0; ind<DNX_TMC_ITM_SYS_RED_BUFFS_RNGS; ++ind)
  {
    info->unicast_rng_vals[ind] = 0;
  }
  for (ind=0; ind<DNX_TMC_ITM_SYS_RED_BUFFS_RNGS; ++ind)
  {
    info->multicast_rng_vals[ind] = 0;
  }
  for (ind=0; ind<DNX_TMC_ITM_SYS_RED_BUFFS_RNGS; ++ind)
  {
    info->bds_rng_vals[ind] = 0;
  }
  for (ind=0; ind<DNX_TMC_ITM_SYS_RED_BUFFS_RNGS; ++ind)
  {
    info->ocb_rng_vals[ind] = 0;
  }
  DNX_SAND_MAGIC_NUM_SET;
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  DNX_TMC_ITM_SYS_RED_GLOB_RCS_INFO_clear(
    DNX_SAND_OUT DNX_TMC_ITM_SYS_RED_GLOB_RCS_INFO *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  sal_memset(info, 0x0, sizeof(DNX_TMC_ITM_SYS_RED_GLOB_RCS_INFO));
  DNX_TMC_ITM_SYS_RED_GLOB_RCS_THS_clear(&(info->thresholds));
  DNX_TMC_ITM_SYS_RED_GLOB_RCS_VALS_clear(&(info->values));
  DNX_SAND_MAGIC_NUM_SET;
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
DNX_TMC_ITM_TC_MAPPING_clear(
		DNX_SAND_OUT DNX_TMC_ITM_TC_MAPPING *info
	)
{
  uint32
    i;

  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  sal_memset(info, 0x0, sizeof(DNX_TMC_ITM_TC_MAPPING));

  for(i=0; i<DNX_TMC_NOF_TRAFFIC_CLASSES; i++)
  {
    info->new_tc[i] = 0;
  }
  
  DNX_SAND_MAGIC_NUM_SET;
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

#if DNX_TMC_DEBUG_IS_LVL1

const char*
  DNX_TMC_ITM_QT_NDX_to_string(
    DNX_SAND_IN  DNX_TMC_ITM_QT_NDX enum_val
  )
{
  const char* str = NULL;
  switch(enum_val)
  {
  case DNX_TMC_ITM_QT_NDX_00:
    str = "ndx_00";
  break;
  case DNX_TMC_ITM_NOF_QT_NDXS:
    str = " Not initialized";
  break;
  default:
    str = " Unknown";
  }
  return str;
}

const char*
  DNX_TMC_ITM_CR_DISCNT_CLS_NDX_to_string(
    DNX_SAND_IN  DNX_TMC_ITM_CR_DISCNT_CLS_NDX enum_val
  )
{
  const char* str = NULL;
  switch(enum_val)
  {
  case DNX_TMC_ITM_CR_DISCNT_CLS_NDX_00:
    str = "ndx_00";
  break;
  case DNX_TMC_ITM_CR_DISCNT_CLS_NDX_01:
    str = "ndx_01";
  break;
  case DNX_TMC_ITM_CR_DISCNT_CLS_NDX_02:
    str = "ndx_02";
  break;
  case DNX_TMC_ITM_CR_DISCNT_CLS_NDX_03:
    str = "ndx_03";
  break;
  case DNX_TMC_ITM_CR_DISCNT_CLS_NDX_04:
    str = "ndx_04";
  break;
  case DNX_TMC_ITM_CR_DISCNT_CLS_NDX_05:
    str = "ndx_05";
  break;
  case DNX_TMC_ITM_CR_DISCNT_CLS_NDX_06:
    str = "ndx_06";
  break;
  case DNX_TMC_ITM_CR_DISCNT_CLS_NDX_07:
    str = "ndx_07";
  break;
  case DNX_TMC_ITM_CR_DISCNT_CLS_NDX_08:
    str = "ndx_08";
  break;
  case DNX_TMC_ITM_CR_DISCNT_CLS_NDX_09:
    str = "ndx_09";
  break;
  case DNX_TMC_ITM_CR_DISCNT_CLS_NDX_10:
    str = "ndx_10";
  break;
  case DNX_TMC_ITM_CR_DISCNT_CLS_NDX_11:
    str = "ndx_11";
  break;
  case DNX_TMC_ITM_CR_DISCNT_CLS_NDX_12:
    str = "ndx_12";
  break;
  case DNX_TMC_ITM_CR_DISCNT_CLS_NDX_13:
    str = "ndx_13";
  break;
  case DNX_TMC_ITM_CR_DISCNT_CLS_NDX_14:
    str = "ndx_14";
  break;
  case DNX_TMC_ITM_CR_DISCNT_CLS_NDX_15:
    str = "ndx_15";
  break;
  case DNX_TMC_ITM_NOF_CR_DISCNT_CLS_NDXS:
    str = " Not initialized";
  break;
  default:
    str = " Unknown";
  }
  return str;
}

const char*
  DNX_TMC_ITM_DBUFF_SIZE_BYTES_to_string(
    DNX_SAND_IN DNX_TMC_ITM_DBUFF_SIZE_BYTES enum_val
  )
{
  const char* str = NULL;
  switch(enum_val)
  {
  case DNX_TMC_ITM_DBUFF_SIZE_BYTES_256:
    str = "bytes_256";
  break;
  case DNX_TMC_ITM_DBUFF_SIZE_BYTES_512:
    str = "bytes_512";
  break;
  case DNX_TMC_ITM_DBUFF_SIZE_BYTES_1024:
    str = "bytes_1024";
  break;
  case DNX_TMC_ITM_DBUFF_SIZE_BYTES_2048:
    str = "bytes_2048";
  break;
  case DNX_TMC_ITM_NOF_DBUFF_SIZES:
    str = " Not initialized";
  break;
  default:
    str = " Unknown";
  }
  return str;
}

const char*
  DNX_TMC_ITM_VSQ_GROUP_SIZE_to_string(
     DNX_SAND_IN int unit,
     DNX_SAND_IN DNX_TMC_ITM_VSQ_GROUP_SIZE enum_val
  )
{
  const char* str = NULL;
  if(enum_val == DNX_TMC_ITM_VSQ_GROUPA_SZE(unit))
  {
    str = "vsq_groupa_sze";
  } else if (enum_val == DNX_TMC_ITM_VSQ_GROUPB_SZE(unit)) {
    str = "vsq_groupb_sze";
  } else if (enum_val == DNX_TMC_ITM_VSQ_GROUPC_SZE(unit)) {
    str = "vsq_groupc_sze";
  } else if (enum_val == DNX_TMC_ITM_VSQ_GROUPD_SZE(unit)) {
    str = "vsq_groupd_sze";
  } else {
    str = " Unknown";
  }
  return str;
}

const char*
  DNX_TMC_ITM_VSQ_NDX_RNG_to_string(
    DNX_SAND_IN  int unit,
    DNX_SAND_IN  DNX_TMC_ITM_VSQ_NDX_RNG enum_val
  )
{
  const char* str = NULL;
  if (enum_val == DNX_TMC_ITM_VSQ_NDX_MIN(unit))
  {
      str = "vsq_ndx_min";
  } 
  else if (enum_val == DNX_TMC_ITM_VSQ_NDX_MAX(unit)) 
  {
      str = "vsq_ndx_max";
  } 
  else 
  {
      str = " Unknown";
  }   
  return str;
}

const char*
  DNX_TMC_ITM_ADMIT_TSTS_to_string(
    DNX_SAND_IN  DNX_TMC_ITM_ADMIT_TSTS enum_val
  )
{
  const char* str = NULL;
  switch(enum_val)
  {
  case DNX_TMC_ITM_ADMIT_TST_00:
    str = "admit_tst_00";
  break;
  case DNX_TMC_ITM_ADMIT_TST_01:
    str = "admit_tst_01";
  break;
  case DNX_TMC_ITM_ADMIT_TST_02:
    str = "admit_tst_02";
  break;
  case DNX_TMC_ITM_ADMIT_TST_03:
    str = "admit_tst_03";
  break;
  default:
    str = " Unknown";
  }
  return str;
}

const char*
  DNX_TMC_ITM_VSQ_GROUP_to_string(
    DNX_SAND_IN  DNX_TMC_ITM_VSQ_GROUP enum_val
  )
{
  const char* str = NULL;
  switch(enum_val)
  {
  case DNX_TMC_ITM_VSQ_GROUP_CTGRY:
    str = "ctgry";
  break;
  case DNX_TMC_ITM_VSQ_GROUP_CTGRY_TRAFFIC_CLS:
    str = "ctgry_traffic_cls";
  break;
  case DNX_TMC_ITM_VSQ_GROUP_CTGRY_2_3_CNCTN_CLS:
    str = "ctgry_2_3_cnctn_cls";
  break;
  case DNX_TMC_ITM_VSQ_GROUP_STTSTCS_TAG:
    str = "sttstcs_tag";
  break;
  default:
    str = " Unknown";
  }
  return str;
}

const char*
  DNX_TMC_ITM_STAG_ENABLE_MODE_to_string(
    DNX_SAND_IN DNX_TMC_ITM_STAG_ENABLE_MODE enum_val
  )
{
  const char* str = NULL;
  switch(enum_val)
  {
  case DNX_TMC_ITM_STAG_ENABLE_MODE_DISABLED:
    str = "DISABLED";
  break;

  case DNX_TMC_ITM_STAG_ENABLE_MODE_STAT_IF_NO_DEQ:
    str = "STAT_IF_NO_DEQ";
  break;

  case DNX_TMC_ITM_STAG_ENABLE_MODE_ENABLED_WITH_DEQ:
    str = "ENABLED_WITH_DEQ";
  break;

  case DNX_TMC_ITM_NOF_STAG_ENABLE_MODES:
    str = "NOF_ENABLE_MODES";
  break;

  default:
    str = " Unknown";
  }
  return str;
}

void
  DNX_TMC_ITM_DRAM_BUFFERS_INFO_print(
    DNX_SAND_IN DNX_TMC_ITM_DRAM_BUFFERS_INFO *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  LOG_CLI((BSL_META_U(unit,
                      "Dbuff_size %s [Bytes] \n\r"),
           DNX_TMC_ITM_DBUFF_SIZE_BYTES_to_string(info->dbuff_size)
           ));
  LOG_CLI((BSL_META_U(unit,
                      "Uc_nof_buffs: %u[Buffers]\n\r"),info->uc_nof_buffs));
  LOG_CLI((BSL_META_U(unit,
                      "Full_mc_nof_buffs: %u[Buffers]\n\r"),info->full_mc_nof_buffs));
  LOG_CLI((BSL_META_U(unit,
                      "Mini_mc_nof_buffs: %u[Buffers]\n\r"),info->mini_mc_nof_buffs));
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  DNX_TMC_ITM_GLOB_RCS_FC_TYPE_print(
    DNX_SAND_IN DNX_TMC_ITM_GLOB_RCS_FC_TYPE *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  LOG_CLI((BSL_META_U(unit,
                      "Hp: ")));
  DNX_TMC_THRESH_WITH_HYST_INFO_print(&(info->hp));
  LOG_CLI((BSL_META_U(unit,
                      "Lp: ")));
  DNX_TMC_THRESH_WITH_HYST_INFO_print(&(info->lp));
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  DNX_TMC_ITM_GLOB_RCS_FC_TH_print(
    DNX_SAND_IN DNX_TMC_ITM_GLOB_RCS_FC_TH *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  LOG_CLI((BSL_META_U(unit,
                      " +---------------------------------+\n\r")));
  LOG_CLI((BSL_META_U(unit,
                      " |  Resource |   SET    |  CLEAR   |\n\r")));
  LOG_CLI((BSL_META_U(unit,
                      " +---------------------------------+\n\r")));
  LOG_CLI((BSL_META_U(unit,
                      " | BDB-HP    | %8u | %8u |\n\r"), info->bdbs.hp.set, info->bdbs.hp.clear));
  LOG_CLI((BSL_META_U(unit,
                      " | BDB-LP    | %8u | %8u |\n\r"), info->bdbs.lp.set, info->bdbs.lp.clear));
  LOG_CLI((BSL_META_U(unit,
                      " +---------------------------------+\n\r")));
  LOG_CLI((BSL_META_U(unit,
                      " | UC-HP     | %8u | %8u |\n\r"), info->unicast.hp.set, info->unicast.hp.clear));
  LOG_CLI((BSL_META_U(unit,
                      " | UC-LP     | %8u | %8u |\n\r"), info->unicast.lp.set, info->unicast.lp.clear));
  LOG_CLI((BSL_META_U(unit,
                      " +---------------------------------+\n\r")));
  LOG_CLI((BSL_META_U(unit,
                      " | FullMC-HP | %8u | %8u |\n\r"), info->full_mc.hp.set, info->full_mc.hp.clear));
  LOG_CLI((BSL_META_U(unit,
                      " | FullMC-LP | %8u | %8u |\n\r"), info->full_mc.lp.set, info->full_mc.lp.clear));
  LOG_CLI((BSL_META_U(unit,
                      " +---------------------------------+\n\r")));
  LOG_CLI((BSL_META_U(unit,
                      " | MiniMC-HP | %8u | %8u |\n\r"), info->mini_mc.hp.set, info->mini_mc.hp.clear));
  LOG_CLI((BSL_META_U(unit,
                      " | MiniMC-LP | %8u | %8u |\n\r"), info->mini_mc.lp.set, info->mini_mc.lp.clear));
  LOG_CLI((BSL_META_U(unit,
                      " +---------------------------------+\n\r")));
  LOG_CLI((BSL_META_U(unit,
                      " | OCB-HP | %8u | %8u |\n\r"), info->ocb.hp.set, info->ocb.hp.clear));
  LOG_CLI((BSL_META_U(unit,
                      " | OCB-LP | %8u | %8u |\n\r"), info->ocb.lp.set, info->ocb.lp.clear));
  LOG_CLI((BSL_META_U(unit,
                      " +---------------------------------+\n\r")));
  LOG_CLI((BSL_META_U(unit,
                      " | OCB_P0-HP | %8u | %8u |\n\r"), info->ocb_p0.hp.set, info->ocb_p0.hp.clear));
  LOG_CLI((BSL_META_U(unit,
                      " | OCB_P0-LP | %8u | %8u |\n\r"), info->ocb_p0.lp.set, info->ocb_p0.lp.clear));
  LOG_CLI((BSL_META_U(unit,
                      " +---------------------------------+\n\r")));
  LOG_CLI((BSL_META_U(unit,
                      " | OCB_P1-HP | %8u | %8u |\n\r"), info->ocb_p1.hp.set, info->ocb_p1.hp.clear));
  LOG_CLI((BSL_META_U(unit,
                      " | OCB_P1-LP | %8u | %8u |\n\r"), info->ocb_p1.lp.set, info->ocb_p1.lp.clear));
  LOG_CLI((BSL_META_U(unit,
                      " +---------------------------------+\n\r")));
  LOG_CLI((BSL_META_U(unit,
                      " | MIX_P0-HP | %8u | %8u |\n\r"), info->mix_p0.hp.set, info->mix_p0.hp.clear));
  LOG_CLI((BSL_META_U(unit,
                      " | MIX_P0-LP | %8u | %8u |\n\r"), info->mix_p0.lp.set, info->mix_p0.lp.clear));
  LOG_CLI((BSL_META_U(unit,
                      " +---------------------------------+\n\r")));
  LOG_CLI((BSL_META_U(unit,
                      " | MIX_P1-HP | %8u | %8u |\n\r"), info->mix_p1.hp.set, info->mix_p1.hp.clear));
  LOG_CLI((BSL_META_U(unit,
                      " | MIX_P1-LP | %8u | %8u |\n\r"), info->mix_p1.lp.set, info->mix_p1.lp.clear));
  LOG_CLI((BSL_META_U(unit,
                      " +---------------------------------+\n\r")));
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  DNX_TMC_ITM_GLOB_RCS_DROP_TH_print_no_table(
    DNX_SAND_IN DNX_TMC_ITM_GLOB_RCS_DROP_TH *info
  )
{
  uint32 ind=0;
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  for (ind=0; ind<DNX_TMC_NOF_DROP_PRECEDENCE; ++ind)
  {
    LOG_CLI((BSL_META_U(unit,
                        "Bdbs[%u]: "),ind));
    DNX_TMC_THRESH_WITH_HYST_INFO_print(&(info->bdbs[ind]));
  }
  for (ind=0; ind<DNX_TMC_NOF_DROP_PRECEDENCE; ++ind)
  {
    LOG_CLI((BSL_META_U(unit,
                        "Bds[%u]: "),ind));
    DNX_TMC_THRESH_WITH_HYST_INFO_print(&(info->bds[ind]));
  }
  for (ind=0; ind<DNX_TMC_NOF_DROP_PRECEDENCE; ++ind)
  {
    LOG_CLI((BSL_META_U(unit,
                        "Unicast[%u]: "),ind));
    DNX_TMC_THRESH_WITH_HYST_INFO_print(&(info->unicast[ind]));
  }
  for (ind=0; ind<DNX_TMC_NOF_DROP_PRECEDENCE; ++ind)
  {
    LOG_CLI((BSL_META_U(unit,
                        "Full_mc[%u]: "),ind));
    DNX_TMC_THRESH_WITH_HYST_INFO_print(&(info->full_mc[ind]));
  }
  for (ind=0; ind<DNX_TMC_NOF_DROP_PRECEDENCE; ++ind)
  {
    LOG_CLI((BSL_META_U(unit,
                        "Mini_mc[%u]: "),ind));
    DNX_TMC_THRESH_WITH_HYST_INFO_print(&(info->mini_mc[ind]));
  }
  for (ind=0; ind<DNX_TMC_NOF_DROP_PRECEDENCE; ++ind)
  {
    LOG_CLI((BSL_META_U(unit,
                        "Mem_size[%u]: "),ind));
    DNX_TMC_THRESH_WITH_HYST_INFO_print(&(info->mem_excess[ind]));
  }
  for (ind=0; ind<DNX_TMC_NOF_DROP_PRECEDENCE; ++ind)
  {
    LOG_CLI((BSL_META_U(unit,
                        "OCB_UC[%u]: "),ind));
    DNX_TMC_THRESH_WITH_HYST_INFO_print(&(info->ocb_uc[ind]));
  }
  for (ind=0; ind<DNX_TMC_NOF_DROP_PRECEDENCE; ++ind)
  {
    LOG_CLI((BSL_META_U(unit,
                        "OCB_MC[%u]: "),ind));
    DNX_TMC_THRESH_WITH_HYST_INFO_print(&(info->ocb_mc[ind]));
  }
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
   DNX_TMC_ITM_GLOB_RCS_DROP_TH_print(
    DNX_SAND_IN DNX_TMC_ITM_GLOB_RCS_DROP_TH *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  LOG_CLI((BSL_META_U(unit,
                      " +-------------------------------------+\n\r")));
  LOG_CLI((BSL_META_U(unit,
                      " | Resource| DP|   SET     |  CLEAR    |\n\r")));
  LOG_CLI((BSL_META_U(unit,
                      " +-------------------------------------+\n\r")));
  LOG_CLI((BSL_META_U(unit,
                      " | BDB     | 0 | %9u | %9u |\n\r"), info->bdbs[0].set, info->bdbs[0].clear));
  LOG_CLI((BSL_META_U(unit,
                      " | BDB     | 1 | %9u | %9u |\n\r"), info->bdbs[1].set, info->bdbs[1].clear));
  LOG_CLI((BSL_META_U(unit,
                      " | BDB     | 2 | %9u | %9u |\n\r"), info->bdbs[2].set, info->bdbs[2].clear));
  LOG_CLI((BSL_META_U(unit,
                      " | BDB     | 3 | %9u | %9u |\n\r"), info->bdbs[3].set, info->bdbs[3].clear));
  LOG_CLI((BSL_META_U(unit,
                      " +-------------------------------------+\n\r")));
  LOG_CLI((BSL_META_U(unit,
                      " | BD      | 0 | %9u | %9u |\n\r"), info->bds[0].set, info->bds[0].clear));
  LOG_CLI((BSL_META_U(unit,
                      " | BD      | 1 | %9u | %9u |\n\r"), info->bds[1].set, info->bds[1].clear));
  LOG_CLI((BSL_META_U(unit,
                      " | BD      | 2 | %9u | %9u |\n\r"), info->bds[2].set, info->bds[2].clear));
  LOG_CLI((BSL_META_U(unit,
                      " | BD      | 3 | %9u | %9u |\n\r"), info->bds[3].set, info->bds[3].clear));
  LOG_CLI((BSL_META_U(unit,
                      " +-------------------------------------+\n\r")));
  LOG_CLI((BSL_META_U(unit,
                      " | UC      | 0 | %9u | %9u |\n\r"), info->unicast[0].set, info->unicast[0].clear));
  LOG_CLI((BSL_META_U(unit,
                      " | UC      | 1 | %9u | %9u |\n\r"), info->unicast[1].set, info->unicast[1].clear));
  LOG_CLI((BSL_META_U(unit,
                      " | UC      | 2 | %9u | %9u |\n\r"), info->unicast[2].set, info->unicast[2].clear));
  LOG_CLI((BSL_META_U(unit,
                      " | UC      | 3 | %9u | %9u |\n\r"), info->unicast[3].set, info->unicast[3].clear));
  LOG_CLI((BSL_META_U(unit,
                      " +-------------------------------------+\n\r")));
  LOG_CLI((BSL_META_U(unit,
                      " | Full MC | 0 | %9u | %9u |\n\r"), info->full_mc[0].set, info->full_mc[0].clear));
  LOG_CLI((BSL_META_U(unit,
                      " | Full MC | 1 | %9u | %9u |\n\r"), info->full_mc[1].set, info->full_mc[1].clear));
  LOG_CLI((BSL_META_U(unit,
                      " | Full MC | 2 | %9u | %9u |\n\r"), info->full_mc[2].set, info->full_mc[2].clear));
  LOG_CLI((BSL_META_U(unit,
                      " | Full MC | 3 | %9u | %9u |\n\r"), info->full_mc[3].set, info->full_mc[3].clear));
  LOG_CLI((BSL_META_U(unit,
                      " +-------------------------------------+\n\r")));
  LOG_CLI((BSL_META_U(unit,
                      " | Mini-MC | 0 | %9u | %9u |\n\r"), info->mini_mc[0].set, info->mini_mc[0].clear));
  LOG_CLI((BSL_META_U(unit,
                      " | Mini-MC | 1 | %9u | %9u |\n\r"), info->mini_mc[1].set, info->mini_mc[1].clear));
  LOG_CLI((BSL_META_U(unit,
                      " | Mini-MC | 2 | %9u | %9u |\n\r"), info->mini_mc[2].set, info->mini_mc[2].clear));
  LOG_CLI((BSL_META_U(unit,
                      " | Mini-MC | 3 | %9u | %9u |\n\r"), info->mini_mc[3].set, info->mini_mc[3].clear));
  LOG_CLI((BSL_META_U(unit,
                      " +-------------------------------------+\n\r")));
  LOG_CLI((BSL_META_U(unit,
                      " | MemSize | 0 | %9u | %9u |\n\r"), info->mem_excess[0].set, info->mem_excess[0].clear));
  LOG_CLI((BSL_META_U(unit,
                      " | MemSize | 1 | %9u | %9u |\n\r"), info->mem_excess[1].set, info->mem_excess[1].clear));
  LOG_CLI((BSL_META_U(unit,
                      " | MemSize | 2 | %9u | %9u |\n\r"), info->mem_excess[2].set, info->mem_excess[2].clear));
  LOG_CLI((BSL_META_U(unit,
                      " | MemSize | 3 | %9u | %9u |\n\r"), info->mem_excess[3].set, info->mem_excess[3].clear));
  LOG_CLI((BSL_META_U(unit,
                      " +-------------------------------------+\n\r")));
  LOG_CLI((BSL_META_U(unit,
                      " | OCB UC  | 0 | %9u | %9u |\n\r"), info->ocb_uc[0].set, info->ocb_uc[0].clear));
  LOG_CLI((BSL_META_U(unit,
                      " | OCB UC  | 1 | %9u | %9u |\n\r"), info->ocb_uc[1].set, info->ocb_uc[1].clear));
  LOG_CLI((BSL_META_U(unit,
                      " | OCB UC  | 2 | %9u | %9u |\n\r"), info->ocb_uc[2].set, info->ocb_uc[2].clear));
  LOG_CLI((BSL_META_U(unit,
                      " | OCB UC  | 3 | %9u | %9u |\n\r"), info->ocb_uc[3].set, info->ocb_uc[3].clear));
  LOG_CLI((BSL_META_U(unit,
                      " +-------------------------------------+\n\r")));
  LOG_CLI((BSL_META_U(unit,
                      " | OCB MC  | 0 | %9u | %9u |\n\r"), info->ocb_mc[0].set, info->ocb_mc[0].clear));
  LOG_CLI((BSL_META_U(unit,
                      " | OCB MC  | 1 | %9u | %9u |\n\r"), info->ocb_mc[1].set, info->ocb_mc[1].clear));
  LOG_CLI((BSL_META_U(unit,
                      " | OCB MC  | 2 | %9u | %9u |\n\r"), info->ocb_mc[2].set, info->ocb_mc[2].clear));
  LOG_CLI((BSL_META_U(unit,
                      " | OCB MC  | 3 | %9u | %9u |\n\r"), info->ocb_mc[3].set, info->ocb_mc[3].clear));
  LOG_CLI((BSL_META_U(unit,
                      " +-------------------------------------+\n\r")));


exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  DNX_TMC_ITM_QUEUE_INFO_print(
    DNX_SAND_IN DNX_TMC_ITM_QUEUE_INFO *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  LOG_CLI((BSL_META_U(unit,
                      "Cr_req_type_ndx:    %u\n\r"),info->cr_req_type_ndx));
  LOG_CLI((BSL_META_U(unit,
                      "Credit_cls:         %u\n\r"),info->credit_cls));
  LOG_CLI((BSL_META_U(unit,
                      "Rate_cls:           %u\n\r"),info->rate_cls));
  LOG_CLI((BSL_META_U(unit,
                      "Vsq_connection_cls: %u\n\r"),info->vsq_connection_cls));
  LOG_CLI((BSL_META_U(unit,
                      "Vsq_traffic_cls:    %u\n\r"),info->vsq_traffic_cls));
  LOG_CLI((BSL_META_U(unit,
                      "Signature:          %u\n\r"),info->signature));
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  DNX_TMC_ITM_CATEGORY_RNGS_print(
    DNX_SAND_IN DNX_TMC_ITM_CATEGORY_RNGS *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  LOG_CLI((BSL_META_U(unit,
                      "Vsq_ctgry0_end: %u\n\r"),info->vsq_ctgry0_end));
  LOG_CLI((BSL_META_U(unit,
                      "Vsq_ctgry1_end: %u\n\r"),info->vsq_ctgry1_end));
  LOG_CLI((BSL_META_U(unit,
                      "Vsq_ctgry2_end: %u\n\r"),info->vsq_ctgry2_end));
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  DNX_TMC_ITM_ADMIT_ONE_TEST_TMPLT_print(
    DNX_SAND_IN DNX_TMC_ITM_ADMIT_ONE_TEST_TMPLT *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  LOG_CLI((BSL_META_U(unit,
                      "  Ctgry_test_en:          %d\n\r"),info->ctgry_test_en));
  LOG_CLI((BSL_META_U(unit,
                      "  Ctgry_trffc_test_en:    %d\n\r"),info->ctgry_trffc_test_en));
  LOG_CLI((BSL_META_U(unit,
                      "  Ctgry2_3_cnctn_test_en: %d\n\r"),info->ctgry2_3_cnctn_test_en));
  LOG_CLI((BSL_META_U(unit,
                      "  Sttstcs_tag_test_en:    %d\n\r"),info->sttstcs_tag_test_en));
  LOG_CLI((BSL_META_U(unit,
                      "  Pfc_test_en:            %d\n\r"),info->pfc_test_en));
  LOG_CLI((BSL_META_U(unit,
                      "  Llfc_test_en:           %d\n\r"),info->llfc_test_en));
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  DNX_TMC_ITM_ADMIT_TEST_TMPLT_INFO_print(
    DNX_SAND_IN DNX_TMC_ITM_ADMIT_TEST_TMPLT_INFO *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  LOG_CLI((BSL_META_U(unit,
                      "Test_a:\n\r")));
  DNX_TMC_ITM_ADMIT_ONE_TEST_TMPLT_print(&(info->test_a));
  LOG_CLI((BSL_META_U(unit,
                      "Test_b:\n\r")));
  DNX_TMC_ITM_ADMIT_ONE_TEST_TMPLT_print(&(info->test_b));
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  DNX_TMC_ITM_CR_REQUEST_HUNGRY_TH_print(
    DNX_SAND_IN DNX_TMC_ITM_CR_REQUEST_HUNGRY_TH *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  LOG_CLI((BSL_META_U(unit,
                      "  Off_to_slow_th: %d[Bytes]\n\r"),info->off_to_slow_th));
  LOG_CLI((BSL_META_U(unit,
                      "  Off_to_normal_th: %d[Bytes]\n\r"),info->off_to_normal_th));
  LOG_CLI((BSL_META_U(unit,
                      "  Slow_to_normal_th: %d[Bytes]\n\r"),info->slow_to_normal_th));
  LOG_CLI((BSL_META_U(unit,
                      "  Normal_to_slow_th: %d[Bytes]\n\r"),info->normal_to_slow_th));
  LOG_CLI((BSL_META_U(unit,
                      "  Multiplier: %u[Bytes]\n\r"),info->multiplier));
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  DNX_TMC_ITM_CR_REQUEST_BACKOFF_TH_print(
    DNX_SAND_IN DNX_TMC_ITM_CR_REQUEST_BACKOFF_TH *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  LOG_CLI((BSL_META_U(unit,
                      "    Backoff_enter_th: %d[Bytes]\n\r"),info->backoff_enter_th));
  LOG_CLI((BSL_META_U(unit,
                      "    Backoff_exit_th: %d[Bytes]\n\r"),info->backoff_exit_th));
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  DNX_TMC_ITM_CR_REQUEST_BACKLOG_TH_print(
    DNX_SAND_IN DNX_TMC_ITM_CR_REQUEST_BACKLOG_TH *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  LOG_CLI((BSL_META_U(unit,
                      "    Backlog_enter_th: %d[Bytes]\n\r"),info->backlog_enter_th));
  LOG_CLI((BSL_META_U(unit,
                      "    Backlog_exit_th: %d[Bytes]\n\r"),info->backlog_exit_th));
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  DNX_TMC_ITM_CR_REQUEST_EMPTY_Q_TH_print(
    DNX_SAND_IN DNX_TMC_ITM_CR_REQUEST_EMPTY_Q_TH *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  LOG_CLI((BSL_META_U(unit,
                      "    Satisfied_empty_q_th: %d[Bytes]\n\r"),info->satisfied_empty_q_th));
  LOG_CLI((BSL_META_U(unit,
                      "    Max_credit_balance_empty_q: %d[Bytes]\n\r"),info->max_credit_balance_empty_q));
  LOG_CLI((BSL_META_U(unit,
                      "    Exceed_max_empty_q: %d\n\r"),info->exceed_max_empty_q));
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  DNX_TMC_ITM_CR_REQUEST_SATISFIED_TH_print(
    DNX_SAND_IN DNX_TMC_ITM_CR_REQUEST_SATISFIED_TH *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  LOG_CLI((BSL_META_U(unit,
                      "  Backoff_th:\n\r")));
  DNX_TMC_ITM_CR_REQUEST_BACKOFF_TH_print(&(info->backoff_th));
  LOG_CLI((BSL_META_U(unit,
                      "  Backlog_th:\n\r")));
  DNX_TMC_ITM_CR_REQUEST_BACKLOG_TH_print(&(info->backlog_th));
  LOG_CLI((BSL_META_U(unit,
                      "  Empty_queues:\n\r")));
  DNX_TMC_ITM_CR_REQUEST_EMPTY_Q_TH_print(&(info->empty_queues));
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  DNX_TMC_ITM_CR_WD_Q_TH_print(
    DNX_SAND_IN DNX_TMC_ITM_CR_WD_Q_TH *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  LOG_CLI((BSL_META_U(unit,
                      "  Cr_wd_stts_msg_gen: %u[msec]\n\r"),info->cr_wd_stts_msg_gen));
  LOG_CLI((BSL_META_U(unit,
                      "  Cr_wd_dlt_q_th: %u[msec]\n\r"),info->cr_wd_dlt_q_th));
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  DNX_TMC_ITM_CR_REQUEST_INFO_print(
    DNX_SAND_IN DNX_TMC_ITM_CR_REQUEST_INFO *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  LOG_CLI((BSL_META_U(unit,
                      "Wd_th:\n\r")));
  DNX_TMC_ITM_CR_WD_Q_TH_print(&(info->wd_th));
  LOG_CLI((BSL_META_U(unit,
                      "Hungry_th:\n\r")));
  DNX_TMC_ITM_CR_REQUEST_HUNGRY_TH_print(&(info->hungry_th));
  LOG_CLI((BSL_META_U(unit,
                      "Satisfied_th:\n\r")));
  DNX_TMC_ITM_CR_REQUEST_SATISFIED_TH_print(&(info->satisfied_th));
  LOG_CLI((BSL_META_U(unit,
                      "Is_low_latency: %s\n\r"), info->is_low_latency?"TRUE":"FALSE"));
  LOG_CLI((BSL_META_U(unit,
                      "Is_remote_credit_value: %s\n\r"), info->is_remote_credit_value?"TRUE":"FALSE"));
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  DNX_TMC_ITM_CR_DISCOUNT_INFO_print(
    DNX_SAND_IN DNX_TMC_ITM_CR_DISCOUNT_INFO *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  LOG_CLI((BSL_META_U(unit,
                      "Discount: %d[Bytes]\n\r"),info->discount));
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  DNX_TMC_ITM_WRED_QT_DP_INFO_print(
    DNX_SAND_IN DNX_TMC_ITM_WRED_QT_DP_INFO *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  LOG_CLI((BSL_META_U(unit,
                      "Wred_en:            %d\n\r"),info->wred_en));
  LOG_CLI((BSL_META_U(unit,
                      "Ignore_packet_size: %d\n\r"),info->ignore_packet_size));
  LOG_CLI((BSL_META_U(unit,
                      "Min_avrg_th:        %u[Bytes]\n\r"),info->min_avrg_th));
  LOG_CLI((BSL_META_U(unit,
                      "Max_avrg_th:        %u[Bytes]\n\r"),info->max_avrg_th));
  LOG_CLI((BSL_META_U(unit,
                      "Max_packet_size:    %u[Bytes]\n\r"),info->max_packet_size));
  LOG_CLI((BSL_META_U(unit,
                      "Max_probability:    %u[%%]\n\r"),info->max_probability));
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  DNX_TMC_ITM_TAIL_DROP_INFO_print(
    DNX_SAND_IN DNX_TMC_ITM_TAIL_DROP_INFO *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  LOG_CLI((BSL_META_U(unit,
                      "Max_inst_q_size: %u[Bytes]\n\r"),info->max_inst_q_size));
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  DNX_TMC_ITM_CR_WD_INFO_print(
    DNX_SAND_IN DNX_TMC_ITM_CR_WD_INFO *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  LOG_CLI((BSL_META_U(unit,
                      "Min_scan_cycle_period_micro: %u[ms]\n\r"),info->min_scan_cycle_period_micro));
  LOG_CLI((BSL_META_U(unit,
                      "Max_flow_msg_gen_rate_nano:  %u[ns]\n\r"),info->max_flow_msg_gen_rate_nano));
  LOG_CLI((BSL_META_U(unit,
                      "Bottom_queue:                %u\n\r"),info->bottom_queue));
  LOG_CLI((BSL_META_U(unit,
                      "Top_queue:                   %u\n\r"),info->top_queue));
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  DNX_TMC_ITM_VSQ_FC_INFO_print(
    DNX_SAND_IN DNX_TMC_ITM_VSQ_FC_INFO *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  LOG_CLI((BSL_META_U(unit,
                      "Q_size_fc: ")));
  DNX_TMC_THRESH_WITH_HYST_INFO_print(&(info->q_size_fc));
  LOG_CLI((BSL_META_U(unit,
                      "Bd_size_fc: ")));
  DNX_TMC_THRESH_WITH_HYST_INFO_print(&(info->bd_size_fc));
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  DNX_TMC_ITM_VSQ_TAIL_DROP_INFO_print(
    DNX_SAND_IN DNX_TMC_ITM_VSQ_TAIL_DROP_INFO *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  LOG_CLI((BSL_META_U(unit,
                      "Max_inst_q_size: %u[Bytes]\n\r"),info->max_inst_q_size));
  LOG_CLI((BSL_META_U(unit,
                      "Max_inst_q_size_bds: %u[Buffer descriptors]\n\r"),info->max_inst_q_size_bds));
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  DNX_TMC_ITM_VSQ_WRED_GEN_INFO_print(
    DNX_SAND_IN DNX_TMC_ITM_VSQ_WRED_GEN_INFO *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  LOG_CLI((BSL_META_U(unit,
                      "Wred_en: %d\n\r"),info->wred_en));
  LOG_CLI((BSL_META_U(unit,
                      "Exp_wq: %u\n\r"),info->exp_wq));
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  DNX_TMC_ITM_STAG_INFO_print(
    DNX_SAND_IN DNX_TMC_ITM_STAG_INFO *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  LOG_CLI((BSL_META_U(unit,
                      "enable_mode %s \n\r"),
           DNX_TMC_ITM_STAG_ENABLE_MODE_to_string(info->enable_mode)
           ));
  LOG_CLI((BSL_META_U(unit,
                      "  Vsq_index_msb: %u\n\r"),info->vsq_index_msb));
  LOG_CLI((BSL_META_U(unit,
                      "  Vsq_index_lsb: %u\n\r"),info->vsq_index_lsb));
  LOG_CLI((BSL_META_U(unit,
                      "  Dropp_en: %u\n\r"),info->dropp_en));
  LOG_CLI((BSL_META_U(unit,
                      "  Dropp_lsb: %u\n\r"),info->dropp_lsb));
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  DNX_TMC_ITM_INGRESS_SHAPE_Q_RANGE_print(
    DNX_SAND_IN DNX_TMC_ITM_INGRESS_SHAPE_Q_RANGE *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  LOG_CLI((BSL_META_U(unit,
                      "  Q_num_low: %u\n\r"),info->q_num_low));
  LOG_CLI((BSL_META_U(unit,
                      "  Q_num_high: %u\n\r"),info->q_num_high));
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  DNX_TMC_ITM_INGRESS_SHAPE_INFO_print(
    DNX_SAND_IN DNX_TMC_ITM_INGRESS_SHAPE_INFO *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  LOG_CLI((BSL_META_U(unit,
                      "Enable: %u\n\r"),info->enable));
  LOG_CLI((BSL_META_U(unit,
                      "Q_range:\n\r")));
  DNX_TMC_ITM_INGRESS_SHAPE_Q_RANGE_print(&(info->q_range));
  LOG_CLI((BSL_META_U(unit,
                      "Rate: %u[Kbps]\n\r"),info->rate));
  LOG_CLI((BSL_META_U(unit,
                      "Sch_port: %u\n\r"),info->sch_port));
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  DNX_TMC_ITM_PRIORITY_MAP_TMPLT_print(
    DNX_SAND_IN DNX_TMC_ITM_PRIORITY_MAP_TMPLT *info
  )
{
  uint32 ind=0;
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  for (ind=0; ind<DNX_TMC_ITM_PRIO_MAP_SIZE_IN_UINT32S; ++ind)
  {
    LOG_CLI((BSL_META_U(unit,
                        "Map[%u]: %u\n\r"),ind,info->map[ind]));
  }
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  DNX_TMC_ITM_SYS_RED_DROP_PROB_print(
    DNX_SAND_IN DNX_TMC_ITM_SYS_RED_DROP_PROB *info
  )
{
  uint32 ind=0;
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  for (ind=0; ind<DNX_TMC_ITM_SYS_RED_DRP_PROBS; ++ind)
  {
    LOG_CLI((BSL_META_U(unit,
                        "Drop_probs[%u]: %u[.01%%]\n\r"),ind,info->drop_probs[ind]));
  }
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  DNX_TMC_ITM_SYS_RED_QT_DP_INFO_print(
    DNX_SAND_IN DNX_TMC_ITM_SYS_RED_QT_DP_INFO *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  LOG_CLI((BSL_META_U(unit,
                      "Enable:        %d\n\r"),info->enable));
  LOG_CLI((BSL_META_U(unit,
                      "Adm_th:        %u\n\r"),info->adm_th));
  LOG_CLI((BSL_META_U(unit,
                      "Prob_th:       %u\n\r"),info->prob_th));
  LOG_CLI((BSL_META_U(unit,
                      "Drp_th:        %u\n\r"),info->drp_th));
  LOG_CLI((BSL_META_U(unit,
                      "Drp_prob_low:  %u\n\r"),info->drp_prob_low));
  LOG_CLI((BSL_META_U(unit,
                      "Drp_prob_high: %u\n\r"),info->drp_prob_high));
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  DNX_TMC_ITM_SYS_RED_QT_INFO_print(
    DNX_SAND_IN DNX_TMC_ITM_SYS_RED_QT_INFO *info
  )
{
  uint32 ind=0;
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  for (ind=0; ind<DNX_TMC_ITM_SYS_RED_Q_SIZE_RANGES; ++ind)
  {
    LOG_CLI((BSL_META_U(unit,
                        "Queue_size_boundaries[%u]: %u[Bytes]\n\r"),ind,info->queue_size_boundaries[ind]));
  }
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  DNX_TMC_ITM_SYS_RED_EG_INFO_print(
    DNX_SAND_IN DNX_TMC_ITM_SYS_RED_EG_INFO *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  LOG_CLI((BSL_META_U(unit,
                      "Enable:                %d\n\r"),info->enable));
  LOG_CLI((BSL_META_U(unit,
                      "Aging_timer:           %u[ms]\n\r"),info->aging_timer));
  LOG_CLI((BSL_META_U(unit,
                      "Reset_expired_q_size:  %d\n\r"),info->reset_expired_q_size));
  LOG_CLI((BSL_META_U(unit,
                      "Aging_only_dec_q_size: %d\n\r"),info->aging_only_dec_q_size));
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  DNX_TMC_ITM_SYS_RED_GLOB_RCS_THS_print(
    DNX_SAND_IN DNX_TMC_ITM_SYS_RED_GLOB_RCS_THS *info
  )
{
  uint32 ind=0;
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  for (ind=0; ind<DNX_TMC_ITM_SYS_RED_BUFFS_RNGS-1; ++ind)
  {
    LOG_CLI((BSL_META_U(unit,
                        "  Unicast_rng_ths[%u]: %u\n\r"),ind,info->unicast_rng_ths[ind]));
  }
  for (ind=0; ind<DNX_TMC_ITM_SYS_RED_BUFFS_RNGS-1; ++ind)
  {
    LOG_CLI((BSL_META_U(unit,
                        "  Multicast_rng_ths[%u]: %u\n\r"),ind,info->multicast_rng_ths[ind]));
  }
  for (ind=0; ind<DNX_TMC_ITM_SYS_RED_BUFFS_RNGS-1; ++ind)
  {
    LOG_CLI((BSL_META_U(unit,
                        "  Bds_rng_ths[%u]: %u\n\r"),ind,info->bds_rng_ths[ind]));
  }
  for (ind=0; ind<DNX_TMC_ITM_SYS_RED_BUFFS_RNGS-1; ++ind)
  {
    LOG_CLI((BSL_META_U(unit,
                        "  Ocb_rng_ths[%u]: %u\n\r"),ind,info->ocb_rng_ths[ind]));
  }
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  DNX_TMC_ITM_SYS_RED_GLOB_RCS_VALS_print(
    DNX_SAND_IN DNX_TMC_ITM_SYS_RED_GLOB_RCS_VALS *info
  )
{
  uint32 ind=0;
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  for (ind=0; ind<DNX_TMC_ITM_SYS_RED_BUFFS_RNGS; ++ind)
  {
    LOG_CLI((BSL_META_U(unit,
                        "  Unicast_rng_vals[%u]: %u\n\r"),ind,info->unicast_rng_vals[ind]));
  }
  for (ind=0; ind<DNX_TMC_ITM_SYS_RED_BUFFS_RNGS; ++ind)
  {
    LOG_CLI((BSL_META_U(unit,
                        "  Multicast_rng_vals[%u]: %u\n\r"),ind,info->multicast_rng_vals[ind]));
  }
  for (ind=0; ind<DNX_TMC_ITM_SYS_RED_BUFFS_RNGS; ++ind)
  {
    LOG_CLI((BSL_META_U(unit,
                        "  Bds_rng_vals[%u]: %u\n\r"),ind,info->bds_rng_vals[ind]));
  }
  for (ind=0; ind<DNX_TMC_ITM_SYS_RED_BUFFS_RNGS; ++ind)
  {
    LOG_CLI((BSL_META_U(unit,
                        "  Ocb_rng_vals[%u]: %u\n\r"),ind,info->ocb_rng_vals[ind]));
  }
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  DNX_TMC_ITM_SYS_RED_GLOB_RCS_INFO_print(
    DNX_SAND_IN DNX_TMC_ITM_SYS_RED_GLOB_RCS_INFO *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  LOG_CLI((BSL_META_U(unit,
                      "Thresholds:\n\r")));
  DNX_TMC_ITM_SYS_RED_GLOB_RCS_THS_print(&(info->thresholds));
  LOG_CLI((BSL_META_U(unit,
                      "Values:\n\r")));
  DNX_TMC_ITM_SYS_RED_GLOB_RCS_VALS_print(&(info->values));
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

#endif /* DNX_TMC_DEBUG_IS_LVL1 */

/* } */

#include <soc/dnx/legacy/SAND/Utils/sand_footer.h>

