#include <soc/mcm/memregs.h>
#if defined(BCM_88690_A0)
/* $Id: jer2_arad_sw_db.c,v 1.118 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 * $
*/

#ifdef _ERR_MSG_MODULE_NAME
  #error "_ERR_MSG_MODULE_NAME redefined"
#endif

#define _ERR_MSG_MODULE_NAME BSL_SOC_SWDB

/*************
 * INCLUDES  *
 *************/
/* { */
#include <shared/bsl.h>
#include <shared/swstate/access/sw_state_access.h>
#include <soc/intr.h>
#include <soc/error.h>
#include <soc/dnxc/legacy/error.h>

#include <soc/dnx/legacy/drv.h>
#include <soc/dnx/legacy/dnx_wb_engine.h>
#include <soc/dnx/legacy/SAND/Utils/sand_header.h>
#include <soc/dnx/legacy/ARAD/arad_general.h>
#include <soc/dnx/legacy/ARAD/arad_sw_db.h>

#include <soc/dnx/legacy/SAND/Utils/sand_framework.h>
#include <soc/dnx/legacy/TMC/tmc_api_ingress_traffic_mgmt.h>

#include <soc/dnx/swstate/access/lag_access.h>

#include <soc/shared/sat.h>

#include <soc/dnx/legacy/QAX/qax_multicast_imp.h>

/* } */

/*************
 * DEFINES   *
 *************/
/* { */

/* maximum size of val to be inserted to multiset */
#define JER2_ARAD_SW_DB_MULTISET_MAX_VAL_NOF_BYTES  (16)

#define JER2_ARAD_SW_DB_DRAM_DELETED_BUFF_NONE 0xffffffff

#define JER2_ARAD_SW_1ST_AVAILABLE_HW_QUEUE DNX_TMC_ITM_NOF_QT_STATIC
#define JER2_ARAD_SW_NOF_AVAILABLE_HW_QUEUE DNX_TMC_ITM_NOF_QT_NDXS_JER2_ARAD
#define JER2_ARAD_SW_DB_1ST_DYNAMIC_QUEUE_TYPE(unit) (SOC_IS_JERICHO(unit) ? DNX_TMC_ITM_NOF_QT_STATIC :  2)

#define JER2_ARAD_SW_DB_QUEUE_TYPE_IS_DYNAMIC(unit, user_q_type) \
        (user_q_type >= JER2_ARAD_SW_DB_1ST_DYNAMIC_QUEUE_TYPE(unit) && user_q_type < JER2_ARAD_SW_DB_1ST_DYNAMIC_QUEUE_TYPE(unit) + JER2_ARAD_SW_DB_NOF_LEGAL_DYNAMIC_QUEUE_TYPES(unit))

#define JER2_ARAD_SW_DB_SYSPORT_TO_BASE_QUEUE_VALID   0x1
#define JER2_ARAD_SW_DB_SYSPORT_TO_BASE_QUEUE_SW_ONLY 0x2

uint8 Jer2_arad_sw_db_initialized = FALSE;

/********************************************************************************************
 * Configuration
 * {
 ********************************************************************************************/


/*
 * } Configuration
 */

/*************
 * FUNCTIONS *
 *************/
/* { */

/*********************************************************************************************
 * jer2_arad_sw_db_init
 * {
 *********************************************************************************************/

/********************************************************************************************
 * Initialization
 * {
 ********************************************************************************************/
uint32
  jer2_arad_sw_db_init(void)
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(JER2_ARAD_SW_DB_INIT);

  if (Jer2_arad_sw_db_initialized)
  {
    goto exit;
  }
  for (unit = 0; unit < DNX_SAND_MAX_DEVICE; ++unit)
  {
    Jer2_arad_sw_db.jer2_arad_device_sw_db[unit] = NULL;
  }

  Jer2_arad_sw_db_initialized = TRUE;

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_sw_db_init()",0,0);
}
/********************************************************************************************
 * Configuration
 *********************************************************************************************/
void
  jer2_arad_sw_db_close(void)
{
  Jer2_arad_sw_db_initialized = FALSE;
}

/*********************************************************************************************
 * }
 * jer2_arad_chip_definitions
 * {
 *********************************************************************************************/


/*********************************************************************************************
 * }
 * jer2_arad_egr_ports
 * {
 *********************************************************************************************/
uint32
  jer2_arad_sw_db_dev_egr_ports_init(
    DNX_SAND_IN int      unit
  )
{
  uint32
    res; 
  DNX_SAND_OCC_BM_INIT_INFO
    btmp_init_info;
  int core;
  DNX_SAND_OCC_BM_PTR occ_bm_ptr;
     
  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_SW_DB_DEV_EGR_PORTS_INITIALIZE);

  dnx_sand_SAND_OCC_BM_INIT_INFO_clear(&btmp_init_info);

  /* Bitmap occupation information for Channelize arbiter */
  /* Number of channelize arbiter is NOF - Non channelize arbiters */
  btmp_init_info.size = JER2_ARAD_OFP_RATES_EGQ_NOF_CHAN_ARB - 1;


  /* initialize the data to be mapped to*/
  res = dnx_sand_occ_bm_create(
          unit,
          &btmp_init_info,
          &occ_bm_ptr
        );
  DNX_SAND_CHECK_FUNC_RESULT(res, 50, exit);
  res = sw_state_access[unit].dnx.soc.jer2_arad.tm.jer2_arad_egr_ports.chanif2chan_arb_occ.set(unit, occ_bm_ptr);
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 70, exit);

  btmp_init_info.size = JER2_ARAD_EGQ_NOF_IFCS - (JER2_ARAD_OFP_RATES_EGQ_NOF_CHAN_ARB-1);

  /* initialize the data to be mapped to*/
  res = dnx_sand_occ_bm_create(
          unit,
          &btmp_init_info,
          &occ_bm_ptr
        );
  DNX_SAND_CHECK_FUNC_RESULT(res, 56, exit);
  res = sw_state_access[unit].dnx.soc.jer2_arad.tm.jer2_arad_egr_ports.nonchanif2sch_offset_occ.set(unit, occ_bm_ptr);
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 70, exit);

  /* initialiaze channelized cals occupation bitmap */

  /* coverity explanation: validation made in case device with more cores will use same code */
  /* coverity[identical_branches:FALSE] */
  /* coverity[same_on_both_sides:FALSE] */
  for (core = 0; core < SOC_DNX_DEFS_MAX(NOF_CORES); core++) {

      btmp_init_info.size = SOC_DNX_DEFS_MAX(NOF_CHANNELIZED_CALENDARS);
      if ((core == 0) || (core == 1)) {
      } else {
          /* currently only 2 cores are supported, if more cores are added then new SOC_DNX_WB_ENGINE defines should be added for them */
          DNX_SAND_SET_ERROR_CODE(DNX_SAND_ERR, 57, exit);
      }

      /* initialize the data to be mapped to*/
      res = dnx_sand_occ_bm_create(
              unit,
              &btmp_init_info,
              &occ_bm_ptr
            );
      DNX_SAND_CHECK_FUNC_RESULT(res, 58, exit);
      res = sw_state_access[unit].dnx.soc.jer2_arad.tm.jer2_arad_egr_ports.channelized_cals_occ.set(unit, core, occ_bm_ptr);
      DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 70, exit);
  }

  /* initialiaze modified channelized cals occupation bitmap */

  /* coverity explanation: validation made in case device with more cores will use same code */
  /* coverity[identical_branches:FALSE] */
  /* coverity[same_on_both_sides:FALSE] */
  for (core = 0; core < SOC_DNX_DEFS_MAX(NOF_CORES); core++) {

      btmp_init_info.size = SOC_DNX_DEFS_MAX(NOF_CHANNELIZED_CALENDARS);
      if ((core == 0) || (core == 1)) {
      } else {
          /* currently only 2 cores are supported, if more cores are added then new SOC_DNX_WB_ENGINE defines should be added for them */
          DNX_SAND_SET_ERROR_CODE(DNX_SAND_ERR, 57, exit);
      }

      /* initialize the data to be mapped to*/
      res = dnx_sand_occ_bm_create(
              unit,
              &btmp_init_info,
              &occ_bm_ptr
            );
      DNX_SAND_CHECK_FUNC_RESULT(res, 58, exit);
      res = sw_state_access[unit].dnx.soc.jer2_arad.tm.jer2_arad_egr_ports.modified_channelized_cals_occ.set(unit, core, occ_bm_ptr);
      DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 70, exit);
  }

  /* initialiaze e2e interfaces occupation bitmap */

  /* coverity explanation: validation made in case device with more cores will use same code */
  /* coverity[identical_branches:FALSE] */
  /* coverity[same_on_both_sides:FALSE] */
  for (core = 0; core < SOC_DNX_DEFS_MAX(NOF_CORES); core++) {

      btmp_init_info.size = SOC_DNX_IMP_DEFS_MAX(NOF_CORE_INTERFACES);
      if ((core == 0) || (core == 1)) {
      } else {
          /* currently only 2 cores are supported, if more cores are added then new SOC_DNX_WB_ENGINE defines should be added for them */
          DNX_SAND_SET_ERROR_CODE(DNX_SAND_ERR, 59, exit);
      }

      /* initialize the data to be mapped to*/
      res = dnx_sand_occ_bm_create(
              unit,
              &btmp_init_info,
              &occ_bm_ptr
            );
      DNX_SAND_CHECK_FUNC_RESULT(res, 60, exit);
      res = sw_state_access[unit].dnx.soc.jer2_arad.tm.jer2_arad_egr_ports.e2e_interfaces_occ.set(unit, core, occ_bm_ptr);
      DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 70, exit);
  }

  /* initialiaze modified e2e interfaces occupation bitmap */

  /* coverity explanation: validation made in case device with more cores will use same code */
  /* coverity[identical_branches:FALSE] */
  /* coverity[same_on_both_sides:FALSE] */
  for (core = 0; core < SOC_DNX_DEFS_MAX(NOF_CORES); core++) {

      btmp_init_info.size = SOC_DNX_IMP_DEFS_MAX(NOF_CORE_INTERFACES);
      if ((core == 0) || (core == 1)) {
      } else {
          /* currently only 2 cores are supported, if more cores are added then new SOC_DNX_WB_ENGINE defines should be added for them */
          DNX_SAND_SET_ERROR_CODE(DNX_SAND_ERR, 61, exit);
      }

      /* initialize the data to be mapped to*/
      res = dnx_sand_occ_bm_create(
              unit,
              &btmp_init_info,
              &occ_bm_ptr
            );
      DNX_SAND_CHECK_FUNC_RESULT(res, 62, exit);
      res = sw_state_access[unit].dnx.soc.jer2_arad.tm.jer2_arad_egr_ports.modified_e2e_interfaces_occ.set(unit, core, occ_bm_ptr);
      DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 70, exit);
  }

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_sw_db_dev_egr_ports_init()",0,0);
}

uint32
  jer2_arad_sw_db_reassembly_context_init(
    DNX_SAND_IN int      unit
  )
{
  
  DNXC_LEGACY_FIXME_ASSERT;
#ifdef FIXME_DNX_LEGACY
  uint32
    res; 
  DNX_SAND_OCC_BM_INIT_INFO
    btmp_init_info;
  DNX_SAND_OCC_BM_PTR
    occ_bm_ptr;

  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_SW_DB_REASSEMBLY_CONTEXT_INITIALIZE);

  dnx_sand_SAND_OCC_BM_INIT_INFO_clear(&btmp_init_info);
  
  /* Bitmap occupation information for spoof-id arbiter */
  btmp_init_info.size = SOC_DNX_DEFS_GET(unit, num_of_reassembly_context);

  if (SOC_IS_QUX(unit)) {
    /* For QUX the last context is reserved as unmapped context */
    btmp_init_info.size  -= 1;
  }

  /* initialize the data to be mapped to*/
  res = dnx_sand_occ_bm_create(
          unit,
          &btmp_init_info,
          &occ_bm_ptr
        );
  DNX_SAND_CHECK_FUNC_RESULT(res, 50, exit);
  res = sw_state_access[unit].dnx.soc.jer2_arad.tm.reassembly_ctxt.alloc(unit);
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 50, exit);

  res = sw_state_access[unit].dnx.soc.jer2_arad.tm.reassembly_ctxt.reassembly_ctxt_occ.set(unit, occ_bm_ptr);
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 50, exit);

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_sw_db_reassembly_context_init()",0,0);
#endif 
    return -1;
}

uint32
  jer2_arad_sw_db_egr_ports_get(
    DNX_SAND_IN  int                        unit,
    DNX_SAND_IN  int                        core,
    DNX_SAND_IN  uint32                     base_q_pair,
    DNX_SAND_OUT JER2_ARAD_SW_DB_DEV_EGR_RATE    *val
  )
{
  soc_error_t rv;
  int use_core = core;

  DNX_SAND_INIT_ERROR_DEFINITIONS(0);

  if(core == MEM_BLOCK_ALL) {
      use_core = 0;
  }
  if(use_core < 0 || use_core > SOC_DNX_DEFS_GET(unit, nof_cores)){
      DNX_SAND_SET_ERROR_CODE(DNX_SAND_ERR, 5, exit);
  }

  rv = sw_state_access[unit].dnx.soc.jer2_arad.tm.jer2_arad_egr_ports.rates.get(unit, use_core, base_q_pair, val);
  DNX_SAND_SOC_CHECK_FUNC_RESULT(rv, 10, exit);

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_sw_db_sch_port_rate_get()",0,0);
}

uint32
  jer2_arad_sw_db_sch_port_rate_get(
   DNX_SAND_IN  int               unit,
   DNX_SAND_IN  int               core,
   DNX_SAND_IN  uint32            base_q_pair,
   DNX_SAND_OUT uint32            *rate
   )
{
  soc_error_t rv;
  int use_core = core;

  DNX_SAND_INIT_ERROR_DEFINITIONS(0);

  if(core == MEM_BLOCK_ALL) {
      use_core = 0;
  }
  if(use_core < 0 || use_core > SOC_DNX_DEFS_GET(unit, nof_cores)){
      DNX_SAND_SET_ERROR_CODE(DNX_SAND_ERR, 5, exit);
  }
  
  rv = sw_state_access[unit].dnx.soc.jer2_arad.tm.jer2_arad_egr_ports.rates.sch_rates.get(unit, use_core, base_q_pair, rate);
  DNX_SAND_SOC_CHECK_FUNC_RESULT(rv, 10, exit);
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_sw_db_sch_port_rate_get()",0,0);
}

uint32
  jer2_arad_sw_db_sch_port_rate_set(
   DNX_SAND_IN  int               unit,
   DNX_SAND_IN  int               core,
   DNX_SAND_IN  uint32            base_q_pair,
   DNX_SAND_IN  uint32            rate
   )
{
  soc_error_t rv;
  int use_core = core;

  int nof_cores = 1, i;

  DNX_SAND_INIT_ERROR_DEFINITIONS(0);

  if(core == MEM_BLOCK_ALL) {
      use_core = 0;
      nof_cores = SOC_DNX_DEFS_GET(unit, nof_cores);
  }

  if(use_core < 0 || use_core > SOC_DNX_DEFS_GET(unit, nof_cores)){
      DNX_SAND_SET_ERROR_CODE(DNX_SAND_ERR, 5, exit);
  }
  
  for(i=0 ; i < nof_cores ; i++) {
      rv = sw_state_access[unit].dnx.soc.jer2_arad.tm.jer2_arad_egr_ports.rates.sch_rates.set(unit, use_core + i, base_q_pair, rate);
      DNX_SAND_SOC_CHECK_FUNC_RESULT(rv, 10, exit);

  }

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_sw_db_sch_port_rate_set()",0,0);
}

uint32
  jer2_arad_sw_db_sch_priority_port_rate_set( 
   DNX_SAND_IN  int               unit,
   DNX_SAND_IN  int               core,
   DNX_SAND_IN  uint32            offset,
   DNX_SAND_IN  uint32            rate,
   DNX_SAND_IN  uint8             valid
   )
{
  soc_error_t rv;
  int use_core = core;

  int nof_cores = 1, i;

  DNX_SAND_INIT_ERROR_DEFINITIONS(0);

  if(core == MEM_BLOCK_ALL) {
      use_core = 0;
      nof_cores = SOC_DNX_DEFS_GET(unit, nof_cores);
  }

  if(use_core < 0 || use_core > SOC_DNX_DEFS_GET(unit, nof_cores)){
      DNX_SAND_SET_ERROR_CODE(DNX_SAND_ERR, 5, exit);
  }
  
  for(i=0 ; i < nof_cores ; i++) {
      rv = sw_state_access[unit].dnx.soc.jer2_arad.tm.jer2_arad_egr_ports.port_priority.priority_shaper_rate.set(unit, use_core + i, offset, rate);
      DNX_SAND_SOC_CHECK_FUNC_RESULT(rv, 10, exit);
      rv = sw_state_access[unit].dnx.soc.jer2_arad.tm.jer2_arad_egr_ports.port_priority.valid.set(unit, use_core + i, offset, valid);
      DNX_SAND_SOC_CHECK_FUNC_RESULT(rv, 20, exit);

  }

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_sw_db_sch_priority_port_rate_set()",0,0);
}


uint32 
  jer2_arad_sw_db_sch_priority_port_rate_get(
   DNX_SAND_IN  int               unit,
   DNX_SAND_IN  int               core,
   DNX_SAND_IN  uint32            offset,
   DNX_SAND_OUT int               *rate,
   DNX_SAND_OUT uint8             *valid
   )
{
  soc_error_t rv;
  int use_core = core;

  DNX_SAND_INIT_ERROR_DEFINITIONS(0);

  if(core == MEM_BLOCK_ALL) {
      use_core = 0;
  }
  if(use_core < 0 || use_core > SOC_DNX_DEFS_GET(unit, nof_cores)){
      DNX_SAND_SET_ERROR_CODE(DNX_SAND_ERR, 2, exit);
  }
  
  rv = sw_state_access[unit].dnx.soc.jer2_arad.tm.jer2_arad_egr_ports.port_priority.priority_shaper_rate.get(unit, use_core, offset, rate);
  DNX_SAND_SOC_CHECK_FUNC_RESULT(rv, 10, exit);
  rv = sw_state_access[unit].dnx.soc.jer2_arad.tm.jer2_arad_egr_ports.port_priority.valid.get(unit, use_core, offset, valid);
  DNX_SAND_SOC_CHECK_FUNC_RESULT(rv, 20, exit);

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_sw_db_sch_priority_port_rate_get()",0,0);
}


uint32
  jer2_arad_sw_db_sch_port_tcg_rate_set( 
   DNX_SAND_IN  int               unit,
   DNX_SAND_IN  int               core,
   DNX_SAND_IN  uint32            offset,
   DNX_SAND_IN  uint32            rate,
   DNX_SAND_IN  uint8             valid
   )
{
  soc_error_t rv;
  int use_core = core;

  int nof_cores = 1, i;

  DNX_SAND_INIT_ERROR_DEFINITIONS(0);

  if(core == MEM_BLOCK_ALL) {
      use_core = 0;
      nof_cores = SOC_DNX_DEFS_GET(unit, nof_cores);
  }

  if(use_core < 0 || use_core > SOC_DNX_DEFS_GET(unit, nof_cores)){
      DNX_SAND_SET_ERROR_CODE(DNX_SAND_ERR, 5, exit);
  }
  
  for(i=0 ; i < nof_cores ; i++) {
      rv = sw_state_access[unit].dnx.soc.jer2_arad.tm.jer2_arad_egr_ports.tcg_shaper.tcg_shaper_rate.set(unit, use_core + i, offset, rate);
      DNX_SAND_SOC_CHECK_FUNC_RESULT(rv, 10, exit);
      rv = sw_state_access[unit].dnx.soc.jer2_arad.tm.jer2_arad_egr_ports.tcg_shaper.valid.set(unit, use_core + i, offset, valid);
      DNX_SAND_SOC_CHECK_FUNC_RESULT(rv, 20, exit);

  }

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_sw_db_sch_priority_port_rate_set()",0,0);
}


uint32 
  jer2_arad_sw_db_sch_port_tcg_rate_get(
   DNX_SAND_IN  int               unit,
   DNX_SAND_IN  int               core,
   DNX_SAND_IN  uint32            offset,
   DNX_SAND_OUT int               *rate,
   DNX_SAND_OUT uint8             *valid
   )
{
  soc_error_t rv;
  int use_core = core;

  DNX_SAND_INIT_ERROR_DEFINITIONS(0);

  if(core == MEM_BLOCK_ALL) {
      use_core = 0;
  }
  if(use_core < 0 || use_core > SOC_DNX_DEFS_GET(unit, nof_cores)){
      DNX_SAND_SET_ERROR_CODE(DNX_SAND_ERR, 5, exit);
  }
  
  rv = sw_state_access[unit].dnx.soc.jer2_arad.tm.jer2_arad_egr_ports.tcg_shaper.tcg_shaper_rate.get(unit, use_core, offset, rate);
  DNX_SAND_SOC_CHECK_FUNC_RESULT(rv, 10, exit);
  rv = sw_state_access[unit].dnx.soc.jer2_arad.tm.jer2_arad_egr_ports.tcg_shaper.valid.get(unit, use_core, offset, valid);
  DNX_SAND_SOC_CHECK_FUNC_RESULT(rv, 20, exit);
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_sw_db_sch_port_tcg_rate_get()",0,0);
}


uint32
  jer2_arad_sw_db_egq_port_rate_get(
   DNX_SAND_IN   int               unit,
   DNX_SAND_IN   int               core,
   DNX_SAND_IN   uint32            base_q_pair,
   DNX_SAND_OUT  uint32           *rate
   )
{
  soc_error_t rv;
  int use_core = core;

  DNX_SAND_INIT_ERROR_DEFINITIONS(0);

  if(core == MEM_BLOCK_ALL) {
      use_core = 0;
  }
  if(use_core < 0 || use_core > SOC_DNX_DEFS_GET(unit, nof_cores)){
      DNX_SAND_SET_ERROR_CODE(DNX_SAND_ERR, 2, exit);
  }
  
  rv = sw_state_access[unit].dnx.soc.jer2_arad.tm.jer2_arad_egr_ports.rates.egq_rates.get(unit, use_core, base_q_pair, rate);
  DNX_SAND_SOC_CHECK_FUNC_RESULT(rv, 10, exit);

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "jer2_arad_sw_db_egq_port_rate_get()",0,0);
}

uint32
  jer2_arad_sw_db_egq_port_rate_set(
   DNX_SAND_IN  int               unit,
   DNX_SAND_IN  int               core,
   DNX_SAND_IN  uint32            base_q_pair,
   DNX_SAND_IN  uint32            rate
   )
{
  soc_error_t rv;
  int nof_cores = 1, i;
  int use_core = core;

  DNX_SAND_INIT_ERROR_DEFINITIONS(0);

  if(core == MEM_BLOCK_ALL) {
      use_core = 0;
      nof_cores = SOC_DNX_DEFS_GET(unit, nof_cores);
  }

  if(use_core < 0 || use_core > SOC_DNX_DEFS_GET(unit, nof_cores)){
      DNX_SAND_SET_ERROR_CODE(DNX_SAND_ERR, 2, exit);
  }

  for(i=0 ; i < nof_cores ; i++) {
      rv = sw_state_access[unit].dnx.soc.jer2_arad.tm.jer2_arad_egr_ports.rates.egq_rates.set(unit, use_core+i, base_q_pair, rate);
      DNX_SAND_SOC_CHECK_FUNC_RESULT(rv, 10, exit);

  }

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "jer2_arad_sw_db_egq_port_rate_set()",0,0);
}

uint32
  jer2_arad_sw_db_is_port_valid_get(
   DNX_SAND_IN  int               unit,
   DNX_SAND_IN  int               core,
   DNX_SAND_IN  uint32            base_q_pair,
   DNX_SAND_OUT uint8             *is_valid
   )
{
  soc_error_t rv;
  int use_core = core;

  DNX_SAND_INIT_ERROR_DEFINITIONS(0);

  if(core == MEM_BLOCK_ALL) {
      use_core = 0;
  }
  if(use_core < 0 || use_core > SOC_DNX_DEFS_GET(unit, nof_cores)){
      DNX_SAND_SET_ERROR_CODE(DNX_SAND_ERR, 2, exit);
  }
  
  rv = sw_state_access[unit].dnx.soc.jer2_arad.tm.jer2_arad_egr_ports.rates.valid.get(unit, use_core, base_q_pair, is_valid);
  DNX_SAND_SOC_CHECK_FUNC_RESULT(rv, 10, exit);

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "jer2_arad_sw_db_is_port_valid_get()",0,0);
}

uint32
  jer2_arad_sw_db_is_port_valid_set(
   DNX_SAND_IN  int               unit,
   DNX_SAND_IN  int               core,
   DNX_SAND_IN  uint32            base_q_pair,
   DNX_SAND_IN  uint8             is_valid
   )
{
  soc_error_t rv;
  int nof_cores = 1, i;
  int use_core = core;

  DNX_SAND_INIT_ERROR_DEFINITIONS(0);

  if(core == MEM_BLOCK_ALL) {
      use_core = 0;
      nof_cores = SOC_DNX_DEFS_GET(unit, nof_cores);
  }

  if(use_core < 0 || use_core > SOC_DNX_DEFS_GET(unit, nof_cores)){
      DNX_SAND_SET_ERROR_CODE(DNX_SAND_ERR, 2, exit);
  }

  for(i=0 ; i< nof_cores ; i++) {
        rv = sw_state_access[unit].dnx.soc.jer2_arad.tm.jer2_arad_egr_ports.rates.valid.set(unit, use_core+i, base_q_pair, is_valid);
        DNX_SAND_SOC_CHECK_FUNC_RESULT(rv, 10, exit);

  }

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "jer2_arad_sw_db_is_port_valid_set()",0,0);
}

uint32
  jer2_arad_sw_db_device_tdm_init(
    DNX_SAND_IN int unit
  )
{
  uint32
    tdm_context_map_id,
    res;
  uint8
    is_allocated;

  DNX_SAND_INIT_ERROR_DEFINITIONS(0);

  res = sw_state_access[unit].dnx.soc.jer2_arad.tm.tdm.is_allocated(unit, &is_allocated);
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 5, exit);

  if(!is_allocated) {
      res = sw_state_access[unit].dnx.soc.jer2_arad.tm.tdm.alloc(unit);
      DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 10, exit);
  }
  for (tdm_context_map_id = 0; tdm_context_map_id < JER2_ARAD_NOF_TDM_CONTEXT_MAP; tdm_context_map_id++)
  {
      res = sw_state_access[unit].dnx.soc.jer2_arad.tm.tdm.context_map.set(unit, tdm_context_map_id, JER2_ARAD_IF_ID_NONE);
      DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 20, exit);
  }
  

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_sw_db_device_tdm_init()",0,0);
}


uint32
  jer2_arad_sw_db_tm_init(
    DNX_SAND_IN int unit
  )
{
  uint8
    is_allocated;
  soc_error_t
    rv;

  DNX_SAND_INIT_ERROR_DEFINITIONS(0);

  rv = sw_state_access[unit].dnx.soc.jer2_arad.tm.tm_info.is_allocated(unit, &is_allocated);
  DNX_SAND_SOC_CHECK_FUNC_RESULT(rv, 5, exit);

  if(!is_allocated) {
      rv = sw_state_access[unit].dnx.soc.jer2_arad.tm.tm_info.alloc(unit);
      DNX_SAND_SOC_CHECK_FUNC_RESULT(rv, 10, exit);
  }

  rv = sw_state_access[unit].dnx.soc.jer2_arad.tm.tm_info.queue_to_rate_class_mapping.is_simple_mode.set(unit, JER2_ARAD_SW_DB_QUEUE_TO_RATE_CLASS_MAPPING_IS_UNDEFINED);
  DNX_SAND_SOC_CHECK_FUNC_RESULT(rv, 20, exit);
 
  /* No init for queue_to_rate_class_mapping_ref_count since this tabele is relevant iff queue_to_rate_class_mapping_is_simple==FALSE */
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_sw_db_tm_init()",0,0);
}

uint32
  jer2_arad_sw_db_cnt_init(
    DNX_SAND_IN int unit
  )
{
  uint32
    res;

  DNX_SAND_INIT_ERROR_DEFINITIONS(0);

  JER2_ARAD_CLEAR(&(Jer2_arad_sw_db.jer2_arad_device_sw_db[unit]->cnt), JER2_ARAD_SW_DB_CNT, 1);

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_sw_db_cnt_init()",0,0);
}
uint32
  jer2_arad_sw_db_dram_init(
    DNX_SAND_IN int unit
  )
{
    
    DNXC_LEGACY_FIXME_ASSERT;
    return -1;
}

/*
 * Interrupts sw db init
*/

static uint32
  jer2_arad_sw_db_interrupts_init(
    DNX_SAND_IN int unit
  )
{
   
    DNXC_LEGACY_FIXME_ASSERT;
    return -1;
}

/*
 * SW DB multiset
 */
uint32
  jer2_arad_sw_db_buffer_get_entry(
    DNX_SAND_IN  int                             unit,
    DNX_SAND_IN  uint32                             sec_hanlde,
    DNX_SAND_IN  uint8                              *buffer,
    DNX_SAND_IN  uint32                             offset,
    DNX_SAND_IN  uint32                             len,
    DNX_SAND_OUT uint8                              *data
  )
{
  uint32
    res = DNX_SAND_OK;

  DNX_SAND_INIT_ERROR_DEFINITIONS(0);

  res = dnx_sand_os_memcpy(
    data,
    buffer + (offset * len),
    len
    );
  DNX_SAND_CHECK_FUNC_RESULT(res, 10, exit);

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_sw_db_buffer_get_entry()",0,0);
}

uint32
  jer2_arad_sw_db_buffer_set_entry(
    DNX_SAND_IN  int                             unit,
    DNX_SAND_IN  uint32                             sec_hanlde,
    DNX_SAND_INOUT  uint8                           *buffer,
    DNX_SAND_IN  uint32                             offset,
    DNX_SAND_IN  uint32                             len,
    DNX_SAND_IN  uint8                              *data
  )
{
  uint32
    res = DNX_SAND_OK;

  DNX_SAND_INIT_ERROR_DEFINITIONS(0);

  res = dnx_sand_os_memcpy(
    buffer + (offset * len),
    data,
    len
    );
  DNX_SAND_CHECK_FUNC_RESULT(res, 10, exit);

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_sw_db_buffer_set_entry()",0,0);
}


/*
 * SW DB multiset
 */
static int
  jer2_arad_sw_db_multiset_by_type_get(
    DNX_SAND_IN  int                      unit,
	DNX_SAND_IN	 int				      core_id,
    DNX_SAND_IN  uint32                   multiset_type, /* JER2_ARAD_SW_DB_MULTI_SET */
    DNX_SAND_OUT DNX_SAND_MULTI_SET_PTR*  multi_set_info
  )
{

  
    DNXC_LEGACY_FIXME_ASSERT;
    return -1;
}

uint32
  jer2_arad_sw_db_multiset_add(
    DNX_SAND_IN  int                unit,
	DNX_SAND_IN	 int				      core_id,
    DNX_SAND_IN  uint32                multiset_type, /* JER2_ARAD_SW_DB_MULTI_SET */
    DNX_SAND_IN  uint32                 *val,
    DNX_SAND_OUT  uint32                *data_indx,
    DNX_SAND_OUT  uint8               *first_appear,
    DNX_SAND_OUT  DNX_SAND_SUCCESS_FAILURE    *success
  )
{
  DNX_SAND_MULTI_SET_PTR
    multi_set;
  uint8
    tmp_val[JER2_ARAD_SW_DB_MULTISET_MAX_VAL_NOF_BYTES];
  uint8
    add_success;
  uint32
    member_size,
    res = DNX_SAND_OK;

  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_SW_DB_MULTISET_ADD);

  res = jer2_arad_sw_db_multiset_by_type_get(unit, core_id, multiset_type, &multi_set);
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 10, exit);

  res = dnx_sand_multi_set_get_member_size(unit,multi_set,&member_size) ;
  DNX_SAND_CHECK_FUNC_RESULT(res, 8, exit);
  dnx_sand_U32_to_U8(val,member_size,tmp_val);

  res = dnx_sand_multi_set_member_add(
          unit,
          multi_set,
          (DNX_SAND_IN  DNX_SAND_MULTI_SET_KEY*)tmp_val,
          data_indx,
          first_appear,
          &add_success
        );
  DNX_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  if (add_success)
  {
    *success = DNX_SAND_SUCCESS;
  }
  else
  {
    *success = DNX_SAND_FAILURE_OUT_OF_RESOURCES;
  }

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_sw_db_multiset_add()",0,0);
}

uint32
  jer2_arad_sw_db_multiset_remove(
    DNX_SAND_IN  int       unit,
	DNX_SAND_IN	 int	   core_id,
    DNX_SAND_IN  uint32       multiset_type, /* JER2_ARAD_SW_DB_MULTI_SET */
    DNX_SAND_IN  uint32        *val,
    DNX_SAND_OUT  uint32       *data_indx,
    DNX_SAND_OUT  uint8      *last_appear
  )
{
  DNX_SAND_MULTI_SET_PTR
    multi_set;
  uint8
    tmp_val[JER2_ARAD_SW_DB_MULTISET_MAX_VAL_NOF_BYTES];
  uint32
    val_lcl[1],
    res = DNX_SAND_OK;
  uint32
    member_size ;

  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_SW_DB_MULTISET_REMOVE);

  res = jer2_arad_sw_db_multiset_by_type_get(unit, core_id,multiset_type, &multi_set);
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 10, exit);

  *val_lcl = *val;
  res = dnx_sand_multi_set_get_member_size(unit,multi_set,&member_size) ;
  DNX_SAND_CHECK_FUNC_RESULT(res, 8, exit);
  dnx_sand_U32_to_U8(val_lcl,member_size,tmp_val);

  res = dnx_sand_multi_set_member_remove(
          unit,
          multi_set,
          (DNX_SAND_IN DNX_SAND_MULTI_SET_KEY*)tmp_val,
          data_indx,
          last_appear
        );
  DNX_SAND_CHECK_FUNC_RESULT(res, 10, exit);

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_sw_db_multiset_remove()",0,0);
}

uint32
  jer2_arad_sw_db_multiset_lookup(
    DNX_SAND_IN  int              unit,
	DNX_SAND_IN	 int	   		  core_id,
    DNX_SAND_IN  uint32              multiset_type, /* JER2_ARAD_SW_DB_MULTI_SET */
    DNX_SAND_IN  uint32               *val,
    DNX_SAND_OUT  uint32              *data_indx,
    DNX_SAND_OUT  uint32              *ref_count
  )
{
  DNX_SAND_MULTI_SET_PTR
    multi_set;
  uint8
    tmp_val[JER2_ARAD_SW_DB_MULTISET_MAX_VAL_NOF_BYTES];
  uint32
    val_lcl[1],
    res = DNX_SAND_OK;
  uint32
    member_size ;

  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_SW_DB_MULTISET_LOOKUP);

  res = jer2_arad_sw_db_multiset_by_type_get(unit, core_id, multiset_type, &multi_set);
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 10, exit);

  *val_lcl = *val;
  res = dnx_sand_multi_set_get_member_size(unit,multi_set,&member_size) ;
  DNX_SAND_CHECK_FUNC_RESULT(res, 8, exit);
  dnx_sand_U32_to_U8(val_lcl,member_size,tmp_val);

  res = dnx_sand_multi_set_member_lookup(
          unit,
          multi_set,
          (DNX_SAND_IN DNX_SAND_MULTI_SET_KEY*)&tmp_val,
          data_indx,
          ref_count
        );
  DNX_SAND_CHECK_FUNC_RESULT(res, 10, exit);

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_sw_db_multiset_lookup()",0,0);
}

uint32
  jer2_arad_sw_db_multiset_add_by_index(
    DNX_SAND_IN  int                unit,
	DNX_SAND_IN	 int	   			core_id,
    DNX_SAND_IN  uint32                multiset_type, /* JER2_ARAD_SW_DB_MULTI_SET */
    DNX_SAND_IN  uint32                 *val,
    DNX_SAND_OUT  uint32                data_indx,
    DNX_SAND_OUT  uint8               *first_appear,
    DNX_SAND_OUT  DNX_SAND_SUCCESS_FAILURE    *success
  )
{
  DNX_SAND_MULTI_SET_PTR
    multi_set;
  uint8
    tmp_val[JER2_ARAD_SW_DB_MULTISET_MAX_VAL_NOF_BYTES];
  uint8
    add_success;
  uint32
    res = DNX_SAND_OK;
  uint32
    member_size ;

  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_SW_DB_MULTISET_ADD_BY_INDEX);

  res = jer2_arad_sw_db_multiset_by_type_get(unit, core_id, multiset_type, &multi_set);
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 10, exit);

  res = dnx_sand_multi_set_get_member_size(unit,multi_set,&member_size) ;
  DNX_SAND_CHECK_FUNC_RESULT(res, 8, exit);
  dnx_sand_U32_to_U8(val,member_size,tmp_val);
	
  res = dnx_sand_multi_set_member_add_at_index(
          unit,
          multi_set,
          (DNX_SAND_IN	DNX_SAND_MULTI_SET_KEY*)tmp_val,
          data_indx,
          first_appear,
          &add_success
        );
  DNX_SAND_CHECK_FUNC_RESULT(res, 10, exit);
	
  if (add_success)
  {
    *success = DNX_SAND_SUCCESS;
  }
  else
  {
    *success = DNX_SAND_FAILURE_OUT_OF_RESOURCES;
  }

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_sw_db_multiset_add_by_index()",0,0);
}


uint32
  jer2_arad_sw_db_multiset_remove_by_index(
    DNX_SAND_IN  int                unit,
	DNX_SAND_IN	 int	   			core_id,
    DNX_SAND_IN  uint32                multiset_type, /* JER2_ARAD_SW_DB_MULTI_SET */
    DNX_SAND_IN  uint32                 data_indx,
    DNX_SAND_OUT  uint8               *last_appear
  )
{
  DNX_SAND_MULTI_SET_PTR
    multi_set;
  uint32
    res = DNX_SAND_OK;

  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_SW_DB_MULTISET_REMOVE_BY_INDEX);

  res = jer2_arad_sw_db_multiset_by_type_get(unit, core_id, multiset_type, &multi_set);
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 10, exit);

  res = dnx_sand_multi_set_member_remove_by_index(
          unit,
          multi_set,
          data_indx,
          last_appear
        );
  DNX_SAND_CHECK_FUNC_RESULT(res, 10, exit);

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_sw_db_multiset_remove_by_index()",0,0);
}

uint32
  jer2_arad_sw_db_multiset_clear(
    DNX_SAND_IN  int                unit,
	DNX_SAND_IN	 int	   			core_id,
    DNX_SAND_IN  uint32                multiset_type /* JER2_ARAD_SW_DB_MULTI_SET */
  )
{
  DNX_SAND_MULTI_SET_PTR
    multi_set;
  uint32
    res = DNX_SAND_OK;

  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_SW_DB_MULTISET_CLEAR);

  res = jer2_arad_sw_db_multiset_by_type_get(unit, core_id, multiset_type, &multi_set);
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 10, exit);

  res = dnx_sand_multi_set_clear(
          unit,
          multi_set
        );
  DNX_SAND_CHECK_FUNC_RESULT(res, 10, exit);

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_sw_db_multiset_clear()",0,0);
}

uint32
  jer2_arad_sw_db_multiset_get_enable_bit(
    DNX_SAND_IN  int                 	unit,
    DNX_SAND_IN  uint32                	core_id, /* JER2_ARAD_SW_DB_MULTI_SET */
	DNX_SAND_IN  uint32					tbl_offset,
	DNX_SAND_OUT uint8					*enable
  )
{
  
  DNXC_LEGACY_FIXME_ASSERT;
  return -1;
}

/* 
 * set all value of modport2sysport SW DB to invalid.
 */
static uint32 jer2_arad_sw_db_modport2sysport_init(
    DNX_SAND_IN int unit
  )
{
  soc_error_t rv;
  uint32 modport_idx;
  uint8 is_allocated;
  DNX_SAND_INIT_ERROR_DEFINITIONS(0);

  rv = sw_state_access[unit].dnx.soc.jer2_arad.tm.modport2sysport.is_allocated(unit, &is_allocated);
  DNX_SAND_SOC_CHECK_FUNC_RESULT(rv, 10, exit);

  if(!is_allocated) {
      rv = sw_state_access[unit].dnx.soc.jer2_arad.tm.modport2sysport.alloc(unit, JER2_ARAD_NOF_MODPORT);
      DNX_SAND_SOC_CHECK_FUNC_RESULT(rv, 10, exit);
  }

  for(modport_idx = 0; modport_idx < JER2_ARAD_NOF_MODPORT; ++modport_idx) {
      rv = sw_state_access[unit].dnx.soc.jer2_arad.tm.modport2sysport.set(unit, modport_idx, JER2_ARAD_SW_DB_MODPORT2SYSPORT_INVALID_SYSPORT);
      DNX_SAND_SOC_CHECK_FUNC_RESULT(rv, JER2_ARAD_GEN_ERR_NUM_CLEAR, exit);
  }
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_sw_db_modport2sysport_init()", 0, 0);
}


/* 
 * set all value of sysport2modport SW DB to invalid.
 */
static uint32 jer2_arad_sw_db_sysport2modport_init(
    DNX_SAND_IN int unit
  )
{
  soc_error_t rv;
  uint32 sysport_idx;
  JER2_ARAD_MODPORT_INFO modport;
  uint8 is_allocated;
  DNX_SAND_INIT_ERROR_DEFINITIONS(0);

  rv = sw_state_access[unit].dnx.soc.jer2_arad.tm.sysport2modport.is_allocated(unit, &is_allocated);
  DNX_SAND_SOC_CHECK_FUNC_RESULT(rv, 10, exit);

  if(!is_allocated) {
      rv = sw_state_access[unit].dnx.soc.jer2_arad.tm.sysport2modport.alloc(unit, JER2_ARAD_NOF_SYS_PHYS_PORTS_GET(unit));
      DNX_SAND_SOC_CHECK_FUNC_RESULT(rv, 10, exit);
  }

  for(sysport_idx = 0; sysport_idx < JER2_ARAD_NOF_SYS_PHYS_PORTS_GET(unit); ++sysport_idx) {
      modport.fap_id = JER2_ARAD_SW_DB_SYSPORT2MODPORT_INVALID_ID;
      modport.fap_port_id = JER2_ARAD_SW_DB_SYSPORT2MODPORT_INVALID_ID;
      rv = sw_state_access[unit].dnx.soc.jer2_arad.tm.sysport2modport.set(unit, sysport_idx, &modport);
      DNX_SAND_SOC_CHECK_FUNC_RESULT(rv, JER2_ARAD_GEN_ERR_NUM_CLEAR, exit);
  }
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_sw_db_sysport2modport_init()", 0, 0);
}


/* 
 * set all value of queuequartet2sysport SW DB to invalid.
 */
static uint32 jer2_arad_sw_db_queuequartet2sysport_init(
    DNX_SAND_IN int unit
  )
{
  soc_error_t rv;
  uint32 queue_quartet;
  uint8 is_allocated;
  DNX_SAND_INIT_ERROR_DEFINITIONS(0);

  rv = sw_state_access[unit].dnx.soc.jer2_arad.tm.tm_info.queuequartet2sysport.is_allocated(unit, &is_allocated);
  DNX_SAND_SOC_CHECK_FUNC_RESULT(rv, 10, exit);

  if(!is_allocated) {
      rv = sw_state_access[unit].dnx.soc.jer2_arad.tm.tm_info.queuequartet2sysport.alloc(unit, (SOC_DNX_DEFS_GET(unit, nof_queues_per_pipe)/4) * SOC_DNX_CONFIG(unit)->core_mode.nof_active_cores);
      DNX_SAND_SOC_CHECK_FUNC_RESULT(rv, 10, exit);
  }

  for(queue_quartet = 0; queue_quartet < ((SOC_DNX_DEFS_GET(unit, nof_queues_per_pipe)/4) * SOC_DNX_CONFIG(unit)->core_mode.nof_active_cores); ++queue_quartet) {
      rv = sw_state_access[unit].dnx.soc.jer2_arad.tm.tm_info.queuequartet2sysport.set(unit, queue_quartet, JER2_ARAD_SW_DB_MODPORT2SYSPORT_INVALID_SYSPORT);
      DNX_SAND_SOC_CHECK_FUNC_RESULT(rv, JER2_ARAD_GEN_ERR_NUM_CLEAR, exit);
  }

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_sw_db_sysport2modport_init()", 0, 0);
}




int
    jer2_arad_sw_db_sw_state_alloc(
        DNX_SAND_IN int     unit
  )
{
    soc_error_t rv;
    uint32 res;
    int core;
    uint8 is_init;
    uint8 is_allocated;
    
    DNXC_INIT_FUNC_DEFS;

    rv = sw_state_access[unit].dnx.soc.jer2_arad.tm.is_allocated(unit, &is_allocated);
    DNXC_IF_ERR_EXIT(rv);

    if(!is_allocated) {
        rv = sw_state_access[unit].dnx.soc.jer2_arad.tm.alloc(unit);
        DNXC_IF_ERR_EXIT(rv);
    }

    res = jer2_arad_sw_db_op_mode_init(unit);
    DNXC_SAND_IF_ERR_EXIT(res);

    res = jer2_arad_sw_db_tm_init(unit);
    DNXC_SAND_IF_ERR_EXIT(res);

    rv = sw_state_access[unit].dnx.soc.jer2_arad.tm.tm_info.sysport2basequeue.is_allocated(unit, &is_allocated);
    DNXC_IF_ERR_EXIT(rv);

    if(!is_allocated) {
        rv = sw_state_access[unit].dnx.soc.jer2_arad.tm.tm_info.sysport2basequeue.alloc(unit, JER2_ARAD_NOF_SYS_PHYS_PORTS_GET(unit) * SOC_DNX_CONFIG(unit)->core_mode.nof_active_cores);
        DNXC_IF_ERR_EXIT(rv);
    }

    rv = sw_state_access[unit].dnx.soc.jer2_arad.tm.jer2_jer_modid_group_map.is_allocated(unit, &is_allocated);
    DNXC_IF_ERR_EXIT(rv);

    if(!is_allocated) {
        rv = sw_state_access[unit].dnx.soc.jer2_arad.tm.jer2_jer_modid_group_map.alloc(unit);
        DNXC_IF_ERR_EXIT(rv);
    }

    if (SOC_IS_ARADPLUS(unit)) {
        rv = sw_state_access[unit].dnx.soc.jer2_arad.tm.jer2_arad_plus.is_allocated(unit, &is_allocated);
        DNXC_IF_ERR_EXIT(rv);

        if(!is_allocated) {
            rv = sw_state_access[unit].dnx.soc.jer2_arad.tm.jer2_arad_plus.alloc(unit);
            DNXC_IF_ERR_EXIT(rv);
        }
    }

    rv = sw_state_access[unit].dnx.soc.jer2_arad.tm.jer2_arad_egr_ports.is_allocated(unit, &is_allocated);
    DNXC_IF_ERR_EXIT(rv);

    if(!is_allocated) {
        rv = sw_state_access[unit].dnx.soc.jer2_arad.tm.jer2_arad_egr_ports.alloc(unit);
        DNXC_IF_ERR_EXIT(rv);
    }

    /* Initalize ERP interface ID to NONE */
    /* We are not going to change macros to avoid such cases */
    /* coverity[same_on_both_sides] */
    for (core = 0; core < SOC_DNX_DEFS_MAX(NOF_CORES); core++) {
        rv = sw_state_access[unit].dnx.soc.jer2_arad.tm.jer2_arad_egr_ports.erp_interface_id.set(unit, core, JER2_ARAD_IF_ID_NONE);
        DNXC_IF_ERR_EXIT(rv);
    }

    rv = lag_state.is_init(unit, &is_init);
    DNXC_IF_ERR_EXIT(rv);

    if(!is_init) {
        rv = lag_state.init(unit);
        DNXC_IF_ERR_EXIT(rv);
    }

    rv = sw_state_access[unit].dnx.soc.jer2_arad.tm.cell.is_allocated(unit, &is_allocated);
    DNXC_IF_ERR_EXIT(rv);

    if(!is_allocated) {
        rv = sw_state_access[unit].dnx.soc.jer2_arad.tm.cell.alloc(unit);
        DNXC_IF_ERR_EXIT(rv);
    }

    res = jer2_arad_sw_db_device_tdm_init(unit);
    DNXC_SAND_IF_ERR_EXIT(res);

    res = jer2_arad_sw_db_dram_init(unit);
    DNXC_SAND_IF_ERR_EXIT(res);

    rv = sw_state_access[unit].dnx.soc.jer2_arad.tm.q_type_ref_count.is_allocated(unit, &is_allocated);
    DNXC_IF_ERR_EXIT(rv);

    if(!is_allocated) {
        rv = sw_state_access[unit].dnx.soc.jer2_arad.tm.q_type_ref_count.alloc(unit, DNX_TMC_ITM_NOF_QT_NDXS);
        DNXC_IF_ERR_EXIT(rv);
    }

    rv = sw_state_access[unit].dnx.soc.jer2_arad.tm.q_type_map.is_allocated(unit, &is_allocated);
    DNXC_IF_ERR_EXIT(rv);

    if(!is_allocated) {
        rv = sw_state_access[unit].dnx.soc.jer2_arad.tm.q_type_map.alloc(unit, JER2_ARAD_SW_DB_NOF_DYNAMIC_QUEUE_TYPES);
        DNXC_IF_ERR_EXIT(rv);
    }

    rv = sw_state_access[unit].dnx.soc.jer2_arad.tm.vsi.is_allocated(unit, &is_allocated);
    DNXC_IF_ERR_EXIT(rv);

    if(!is_allocated) {
        rv = sw_state_access[unit].dnx.soc.jer2_arad.tm.vsi.alloc(unit);
        DNXC_IF_ERR_EXIT(rv);
    }

    res = jer2_arad_sw_db_modport2sysport_init(unit);
    DNXC_SAND_IF_ERR_EXIT(res);

    res = jer2_arad_sw_db_sysport2modport_init(unit);
    DNXC_SAND_IF_ERR_EXIT(res);

    res = jer2_arad_sw_db_queuequartet2sysport_init(unit);
    DNXC_SAND_IF_ERR_EXIT(res);

    res = jer2_arad_sw_db_interrupts_init(unit);
    DNXC_SAND_IF_ERR_EXIT(res);

    rv = sw_state_access[unit].dnx.soc.jer2_arad.tm.phy_ports_info.is_allocated(unit, &is_allocated);
    DNXC_IF_ERR_EXIT(rv);

    if(!is_allocated) {
        rv = sw_state_access[unit].dnx.soc.jer2_arad.tm.phy_ports_info.alloc(unit, SOC_MAX_NUM_PORTS);
        DNXC_IF_ERR_EXIT(rv);
    }

    rv = sw_state_access[unit].dnx.soc.jer2_arad.tm.logical_ports_info.is_allocated(unit, &is_allocated);
    DNXC_IF_ERR_EXIT(rv);

    if(!is_allocated) {
        rv = sw_state_access[unit].dnx.soc.jer2_arad.tm.logical_ports_info.alloc(unit, SOC_MAX_NUM_PORTS);
        DNXC_IF_ERR_EXIT(rv);
    }

    rv = sw_state_access[unit].dnx.soc.jer2_arad.tm.guaranteed_q_resource.is_allocated(unit, &is_allocated);
    DNXC_IF_ERR_EXIT(rv);

    if(!is_allocated) {
        rv = sw_state_access[unit].dnx.soc.jer2_arad.tm.guaranteed_q_resource.alloc(unit, SOC_DNX_DEFS_MAX(NOF_CORES));
        DNXC_IF_ERR_EXIT(rv);
    }


exit:
    DNXC_FUNC_RETURN;
}

int
    jer2_arad_sw_db_sw_state_free(
        DNX_SAND_IN int     unit
  )
{
    soc_error_t rv;
    uint8 is_init;
    
    DNXC_INIT_FUNC_DEFS;

    rv = lag_state.is_init(unit, &is_init);
    DNXC_IF_ERR_EXIT(rv);

    if(is_init) {
        rv = lag_state.deinit(unit);
        DNXC_IF_ERR_EXIT(rv);
    }

exit:
    DNXC_FUNC_RETURN;
}

uint32
  jer2_arad_sw_db_device_init(
    DNX_SAND_IN int     unit
  )
{
  uint32
    res;

  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_SW_DB_DEVICE_INIT);

  JER2_ARAD_ALLOC_ANY_SIZE(Jer2_arad_sw_db.jer2_arad_device_sw_db[unit], JER2_ARAD_SW_DB_DEVICE, 1,"Jer2_arad_sw_db.jer2_arad_device_sw_db[unit]");

  DNX_SAND_SOC_IF_ERROR_RETURN(res, 70, exit, SOC_IS_QAX(unit) ? jer2_qax_mcds_multicast_init(unit) : dnx_mcds_multicast_init(unit));


	if(!SOC_WARM_BOOT(unit)) {
        res = jer2_arad_sw_db_dev_egr_ports_init(unit);
        DNX_SAND_CHECK_FUNC_RESULT(res, 60, exit);
	}

     res = jer2_arad_sw_db_cnt_init(unit);
     DNX_SAND_CHECK_FUNC_RESULT(res, 232, exit);

	if(!SOC_WARM_BOOT(unit)) {
        res = jer2_arad_sw_db_reassembly_context_init(unit);
        DNX_SAND_CHECK_FUNC_RESULT(res, 281, exit);
    }

#if defined(BCM_WARM_BOOT_SUPPORT) && defined (BCM_SAT_SUPPORT)
    if (SOC_IS_JERICHO(unit) && soc_property_get(unit, spn_SAT_ENABLE, 0)) {
        soc_sat_wb_init(unit);
    }
#endif

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_sw_db_jer2_arad_device_init()",unit,0);
}

uint32
  jer2_arad_sw_db_device_close(
    DNX_SAND_IN int unit
  )
{
  uint32
    res;

  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_SW_DB_DEVICE_CLOSE);

  DNX_SAND_SOC_IF_ERROR_RETURN(res, 35, exit, SOC_IS_QAX(unit) ? jer2_qax_mcds_multicast_terminate(unit) : dnx_mcds_multicast_terminate(unit));
  JER2_ARAD_FREE_ANY_SIZE(Jer2_arad_sw_db.jer2_arad_device_sw_db[unit]);
  Jer2_arad_sw_db.jer2_arad_device_sw_db[unit] = NULL;

  JER2_ARAD_DO_NOTHING_AND_EXIT;

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_sw_db_device_close()",0,0);
}
/*
 * Cnt
 */
uint32
  jer2_arad_sw_db_cnt_buff_and_index_set(
    DNX_SAND_IN int                     unit,
    DNX_SAND_IN uint16                     proc_id,
    DNX_SAND_IN uint32                     *buff,
    DNX_SAND_IN uint32                     index
                                 )
{
    uint32 res = DNX_SAND_OK;

    DNX_SAND_INIT_ERROR_DEFINITIONS(0);

    JER2_ARAD_SW_DB_FIELD_SET(res, 
                         unit,
                         cnt.host_buff[proc_id],
                         (&buff)
                         );
    JER2_ARAD_SW_DB_FIELD_SET(res, 
                         unit,
                         cnt.buff_line_ndx[proc_id],
                         (&index)
                         );


exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_sw_db_cnt_buff_and_index_set()",0,0);
}
uint32
  jer2_arad_sw_db_cnt_buff_and_index_get(
    DNX_SAND_IN int                     unit,
    DNX_SAND_IN uint16                     proc_id,
    DNX_SAND_OUT uint32                     **buff,
    DNX_SAND_OUT uint32                     *index
                                 )
{
    /*

    JER2_ARAD_SW_DB_INIT_DEFS;

    JER2_ARAD_SW_DB_FIELD_GET(
                        unit,
                        cnt.host_buff[proc_id],
                        buff);
    JER2_ARAD_SW_DB_FIELD_GET(
                        unit,
                        cnt.(buff_line_ndx[proc_id]),
                        index);
    */
    *buff = Jer2_arad_sw_db.jer2_arad_device_sw_db[unit]->cnt.host_buff[proc_id];
    *index = Jer2_arad_sw_db.jer2_arad_device_sw_db[unit]->cnt.buff_line_ndx[proc_id];
    return *index;

}

uint32
  jer2_arad_sw_db_dram_deleted_buff_list_add(
    DNX_SAND_IN int    unit,
    DNX_SAND_IN uint32     buff
  )
{

    
    DNXC_LEGACY_FIXME_ASSERT;
    return -1;
}

uint32
  jer2_arad_sw_db_dram_deleted_buff_list_remove(
    DNX_SAND_IN int    unit,
    DNX_SAND_IN uint32     buff
  )
{
    
    DNXC_LEGACY_FIXME_ASSERT;
    return -1;
}

int
  jer2_arad_sw_db_dram_deleted_buff_list_get(
    DNX_SAND_IN int    unit,
    DNX_SAND_IN uint32     buff,
    DNX_SAND_OUT uint32*     is_deleted
  )
{
    
    DNXC_LEGACY_FIXME_ASSERT;
    return -1;
}

int 
  jer2_arad_sw_db_dram_deleted_buff_list_get_all(
    DNX_SAND_IN int    unit,
    DNX_SAND_OUT uint32*    buff_list_arr,
    DNX_SAND_IN uint32      arr_size,
    DNX_SAND_OUT uint32*    buff_list_num)
{
    
    DNXC_LEGACY_FIXME_ASSERT;
    return -1;    
}

uint32
  jer2_arad_sw_db_op_mode_init(
    DNX_SAND_IN int unit
  )
{
  uint8 is_allocated;
  soc_error_t rv;
  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_SW_DB_REVISION_INIT);

  rv = sw_state_access[unit].dnx.soc.jer2_arad.tm.op_mode.is_allocated(unit, &is_allocated);
  DNX_SAND_SOC_CHECK_FUNC_RESULT(rv, 25, exit);

  if(!is_allocated) {
      rv = sw_state_access[unit].dnx.soc.jer2_arad.tm.op_mode.alloc(unit);
      DNX_SAND_SOC_CHECK_FUNC_RESULT(rv, 25, exit);
  }

  rv = sw_state_access[unit].dnx.soc.jer2_arad.tm.op_mode.is_petrab_in_system.set(unit, FALSE);
  DNX_SAND_SOC_CHECK_FUNC_RESULT(rv, 35, exit);

  rv = sw_state_access[unit].dnx.soc.jer2_arad.tm.op_mode.tdm_mode.set(unit, JER2_ARAD_MGMT_TDM_MODE_PACKET);
  DNX_SAND_SOC_CHECK_FUNC_RESULT(rv, 45, exit);

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_sw_db_op_mode_init()",0,0);
}

void
  jer2_arad_sw_db_is_petrab_in_system_set(
    DNX_SAND_IN int unit,
    DNX_SAND_IN uint8 is_petrab_in_system
  )
{
  sw_state_access[unit].dnx.soc.jer2_arad.tm.op_mode.is_petrab_in_system.set(unit, is_petrab_in_system);
}

uint8
  jer2_arad_sw_db_is_petrab_in_system_get(
    DNX_SAND_IN int unit
  )
{
    uint8 is_petrab_in_system;
    sw_state_access[unit].dnx.soc.jer2_arad.tm.op_mode.is_petrab_in_system.get(unit, &is_petrab_in_system);
    return is_petrab_in_system;
}

void
  jer2_arad_sw_db_tdm_mode_set(
    DNX_SAND_IN int unit,
    DNX_SAND_IN JER2_ARAD_MGMT_TDM_MODE tdm_mode
  )
{
    sw_state_access[unit].dnx.soc.jer2_arad.tm.op_mode.tdm_mode.set(unit, tdm_mode);
}

JER2_ARAD_MGMT_TDM_MODE
  jer2_arad_sw_db_tdm_mode_get(
    DNX_SAND_IN int unit
  )
{
    JER2_ARAD_MGMT_TDM_MODE tdm_mode;
    sw_state_access[unit].dnx.soc.jer2_arad.tm.op_mode.tdm_mode.get(unit, &tdm_mode);
    return tdm_mode;
}

void
  jer2_arad_sw_db_ilkn_tdm_dedicated_queuing_set(
     DNX_SAND_IN int unit,
     DNX_SAND_IN JER2_ARAD_MGMT_ILKN_TDM_DEDICATED_QUEUING_MODE ilkn_tdm_dedicated_queuing
  )
{
    sw_state_access[unit].dnx.soc.jer2_arad.tm.op_mode.ilkn_tdm_dedicated_queuing.set(unit, ilkn_tdm_dedicated_queuing);
}

JER2_ARAD_MGMT_ILKN_TDM_DEDICATED_QUEUING_MODE
  jer2_arad_sw_db_ilkn_tdm_dedicated_queuing_get(
     DNX_SAND_IN int unit
  )
{
    JER2_ARAD_MGMT_ILKN_TDM_DEDICATED_QUEUING_MODE ilkn_tdm_dedicated_queuing;
    sw_state_access[unit].dnx.soc.jer2_arad.tm.op_mode.ilkn_tdm_dedicated_queuing.get(unit, &ilkn_tdm_dedicated_queuing);
    return ilkn_tdm_dedicated_queuing;
}

/*
 * check/set if a (egress) local port has a reassembly context reserved for it
 * for a non mirroring application. 
 */
uint32
  jer2_arad_sw_db_is_port_reserved_for_reassembly_context(
    DNX_SAND_IN int  unit,
    DNX_SAND_IN uint32  local_port,
    DNX_SAND_OUT uint8  *is_reserved /* returns one of: 0 for not reserved, 1 for reserved */
  )
{
  uint32
    port_reserved_reassembly_context;
  soc_error_t
    rv;
  DNX_SAND_INIT_ERROR_DEFINITIONS(0);
  DNX_SAND_ERR_IF_ABOVE_NOF(local_port, SOC_DNX_DEFS_GET(unit, nof_logical_ports), DNX_SAND_VALUE_ABOVE_MAX_ERR, 10, exit);
  DNX_SAND_CHECK_NULL_INPUT(is_reserved);

  rv = sw_state_access[unit].dnx.soc.jer2_arad.tm.jer2_arad_egr_ports.port_reserved_reassembly_context.get(unit, local_port / DNX_SAND_NOF_BITS_IN_UINT32, &port_reserved_reassembly_context);
  DNX_SAND_SOC_CHECK_FUNC_RESULT(rv, 10, exit);
  *is_reserved = (port_reserved_reassembly_context >> (local_port % DNX_SAND_NOF_BITS_IN_UINT32)) & 1;

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_sw_db_is_port_reserved_for_reassembly_context()",local_port,0);
}

uint32
  jer2_arad_sw_db_set_port_reserved_for_reassembly_context(
    DNX_SAND_IN int  unit,
    DNX_SAND_IN uint32  local_port,
    DNX_SAND_IN uint8   reserve /* 0 will cancel reservation, other values will reserve */
  )
{
  soc_error_t
    rv;
  uint32 mask = 1 << (local_port % DNX_SAND_NOF_BITS_IN_UINT32);
  uint32 value;
  DNX_SAND_INIT_ERROR_DEFINITIONS(0);
  DNX_SAND_ERR_IF_ABOVE_NOF(local_port, SOC_DNX_DEFS_GET(unit, nof_logical_ports), DNX_SAND_VALUE_ABOVE_MAX_ERR, 10, exit);

  rv = sw_state_access[unit].dnx.soc.jer2_arad.tm.jer2_arad_egr_ports.port_reserved_reassembly_context.get(unit, local_port / DNX_SAND_NOF_BITS_IN_UINT32, &value);
  DNX_SAND_SOC_CHECK_FUNC_RESULT(rv, 10, exit);
  if (reserve) {
    value |= mask;
  } else {
    value &= ~mask;
  }
  rv = sw_state_access[unit].dnx.soc.jer2_arad.tm.jer2_arad_egr_ports.port_reserved_reassembly_context.set(unit, local_port / DNX_SAND_NOF_BITS_IN_UINT32, value);
  DNX_SAND_SOC_CHECK_FUNC_RESULT(rv, 20, exit);

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_sw_db_set_port_reserved_for_reassembly_context()",local_port,reserve);
}


#ifdef FIXME_DNX_LEGACY 	 
int	 
jer2_arad_sw_db_sw_dump(int unit) 	 
{
    uint32                          i, j;
    int                             core;
    soc_error_t                     rv;
    uint16                          current_cell_ident;
    JER2_ARAD_INTERFACE_ID               context_map;
    JER2_ARAD_SW_DB_DEV_EGR_RATE         rates;
    JER2_ARAD_SW_DB_DEV_RATE             tcg_rate;
    JER2_ARAD_EGR_PROG_TM_PORT_PROFILE   ports_prog_editor_profile;
    JER2_ARAD_SW_DB_DEV_RATE             queue_rate;
    uint32                          calcal_length;
    JER2_ARAD_SW_DB_DEV_EGR_CHAN_ARB     chan_arb;

    DNXC_INIT_FUNC_DEFS;  
    DNXC_LEGACY_FIXME_ASSERT;
        

    LOG_VERBOSE(BSL_LS_SOC_SWDB,
                (BSL_META_U(unit,
                            "\n JER2_ARAD SOC TM:")));
    LOG_VERBOSE(BSL_LS_SOC_SWDB,
                (BSL_META_U(unit,
                            "\n ------------")));

    DNXC_IF_ERR_EXIT(sw_state_access[unit].dnx.soc.jer2_arad.tm.cell.current_cell_ident.get(unit, &current_cell_ident));
    LOG_VERBOSE(BSL_LS_SOC_SWDB,
                (BSL_META_U(unit,
                            "\n current_cell_ident:   %u\n"),  current_cell_ident));

    for (i = 0; i < SOC_DNX_DEFS_GET(unit, nof_cores); i++) {
        rv = sw_state_access[unit].dnx.soc.jer2_arad.tm.jer2_arad_egr_ports.calcal_length.get(unit, i, &calcal_length);
        DNXC_IF_ERR_EXIT(rv);
        LOG_VERBOSE(BSL_LS_SOC_SWDB,
                    (BSL_META_U(unit,
                                "\n calcal_length:        %u\n"),  calcal_length));
    }

    for (i = 0; i < SOC_DNX_DEFS_GET(unit, nof_cores); i++) {
        for (j = 0; j < JER2_ARAD_EGR_NOF_Q_PAIRS; j++) {
            rv = sw_state_access[unit].dnx.soc.jer2_arad.tm.jer2_arad_egr_ports.port_priority_cal.queue_rate.get(unit, i, j, &queue_rate);
            DNXC_IF_ERR_EXIT(rv);
            if(queue_rate.valid) {
                LOG_VERBOSE(BSL_LS_SOC_SWDB,
                            (BSL_META_U(unit,
                                        "\n queue_rate (%03d): valid %hhu egq_rates %u egq_bursts %u\n"), 
                                        i, 
                             queue_rate.valid, 
                             queue_rate.egq_rates, 
                             queue_rate.egq_bursts));
            }
        }
    }

    for (i = 0; i < SOC_DNX_DEFS_GET(unit, nof_cores); i++) {
        for (j = 0; j < SOC_DNX_DEFS_GET(unit, nof_channelized_calendars); j++) {
            rv = sw_state_access[unit].dnx.soc.jer2_arad.tm.jer2_arad_egr_ports.chan_arb.get(unit, i, j, &chan_arb);
            DNXC_IF_ERR_EXIT(rv);
            LOG_VERBOSE(BSL_LS_SOC_SWDB,
                        (BSL_META_U(unit,
                                    "\n nof_calcal_instances (%02d):  %u\n"), i, chan_arb.nof_calcal_instances));
        }
    }

    for (i = 0; i < JER2_ARAD_NOF_FAP_PORTS; i++) {
        rv = sw_state_access[unit].dnx.soc.jer2_arad.tm.jer2_arad_egr_ports.ports_prog_editor_profile.get(unit, i, &ports_prog_editor_profile);
        DNXC_IF_ERR_EXIT(rv);
        if(ports_prog_editor_profile != 0) {
            LOG_VERBOSE(BSL_LS_SOC_SWDB,
                        (BSL_META_U(unit,
                                    "\n ports_prog_editor_profile (%03d):  %hu\n"), i, ports_prog_editor_profile));
        }
    }

    for (i = 0; i < JER2_ARAD_NOF_LAG_GROUPS_MAX; i++) {
        uint8 in_use;
        rv = lag_state.in_use.get(unit, i, &in_use);
        DNXC_IF_ERR_EXIT(rv);
        if(in_use != 0) {
            LOG_VERBOSE(BSL_LS_SOC_SWDB,
                        (BSL_META_U(unit,
                                    "\n in_use (%04d):  %hhu\n"), i, in_use));
        }
    }

    for (i = 0; i < SOC_DNX_DEFS_GET(unit, nof_logical_ports); i++) {
        uint32  local_to_reassembly_context;
        rv = lag_state.local_to_reassembly_context.get(unit, i, &local_to_reassembly_context);
        DNXC_IF_ERR_EXIT(rv);
        if(local_to_reassembly_context != i) {
            LOG_VERBOSE(BSL_LS_SOC_SWDB,
                        (BSL_META_U(unit,
                                    "\n local_to_reassembly_context (%03d):  %u\n"), i, local_to_reassembly_context));
        }
    }

    for (i = 0; i < JER2_ARAD_NOF_TDM_CONTEXT_MAP; i++) {
        DNXC_IF_ERR_EXIT(sw_state_access[unit].dnx.soc.jer2_arad.tm.tdm.context_map.get(unit, i, &context_map));
        if(context_map != i) {
            LOG_VERBOSE(BSL_LS_SOC_SWDB,
                        (BSL_META_U(unit,
                                    "\n tdm_context_map (%03d):  %u\n"), i, context_map));
        }
    }
 
    for (core = 0; core < SOC_DNX_DEFS_GET(unit, nof_cores); core++) {
        for (i = 0; i < JER2_ARAD_EGR_NOF_PS; i++) {
            for (j = 0; j < JER2_ARAD_NOF_TCGS; j++) {
                rv = sw_state_access[unit].dnx.soc.jer2_arad.tm.jer2_arad_egr_ports.tcg_cal.tcg_rate.get(unit, core, i, j, &tcg_rate);
                DNXC_IF_ERR_EXIT(rv);
                if(tcg_rate.valid) {
                    LOG_VERBOSE(BSL_LS_SOC_SWDB,
                                (BSL_META_U(unit,
                                            "\n eg_mult_nof_vlan_bitmaps (%02d, %01d): valid %hhu egq_rates %u egq_bursts %u\n"), 
                                            i, j, 
                                 tcg_rate.valid, 
                                 tcg_rate.egq_rates, 
                                 tcg_rate.egq_bursts));
                }
            }
        }
    }

    for (i = 0; i < SOC_DNX_DEFS_GET(unit, nof_cores); i++) {
        for (j = 0; j < JER2_ARAD_EGR_NOF_BASE_Q_PAIRS; j++) {
            rv = sw_state_access[unit].dnx.soc.jer2_arad.tm.jer2_arad_egr_ports.rates.get(unit, i, j, &rates);
            DNXC_IF_ERR_EXIT(rv);
            if(rates.valid) {
                LOG_VERBOSE(BSL_LS_SOC_SWDB,
                            (BSL_META_U(unit,
                                        "\n rates (%02d, %03d): valid %hhu sch_rates %u egq_rates %u egq_bursts %u\n"), 
                                        i, j, 
                             rates.valid, 
                             rates.sch_rates, 
                             rates.egq_rates, 
                             rates.egq_bursts));
            }
        }
    }

exit:
    DNXC_FUNC_RETURN;
}
#endif 

#define JER2_ARAD_DEVICE_NUMBER_BITS 11
#define JER2_ARAD_DEVICE_NUMBER_MASK 0x7ff

/*
Set a fap_id x fap_port_id to system physical port mapping.
Performs allocation inside the data structure if needed.
*/
uint32 jer2_arad_sw_db_modport2sysport_set(
    DNX_SAND_IN int unit,
    DNX_SAND_IN uint32 fap_id,
    DNX_SAND_IN uint32 fap_port_id,
    DNX_SAND_IN JER2_ARAD_SYSPORT sysport
  )
{
  soc_error_t rv;
  uint32 modport;

  DNX_SAND_INIT_ERROR_DEFINITIONS(0);
  DNX_SAND_ERR_IF_ABOVE_NOF(fap_id, JER2_ARAD_NOF_FAPS_IN_SYSTEM, DNX_SAND_VALUE_ABOVE_MAX_ERR, 10, exit);
  DNX_SAND_ERR_IF_ABOVE_NOF(fap_port_id, JER2_ARAD_NOF_FAP_PORTS, DNX_SAND_VALUE_ABOVE_MAX_ERR, 15, exit);
  DNX_SAND_ERR_IF_ABOVE_NOF(sysport, JER2_ARAD_NOF_SYS_PHYS_PORTS_GET(unit), DNX_SAND_VALUE_ABOVE_MAX_ERR, 20, exit);

  DNX_SAND_CHECK_NULL_PTR(Jer2_arad_sw_db.jer2_arad_device_sw_db[unit], DNX_SAND_ERR, exit);

  modport = fap_id | (fap_port_id << JER2_ARAD_DEVICE_NUMBER_BITS);
  DNX_SAND_ERR_IF_ABOVE_NOF(modport, JER2_ARAD_NOF_MODPORT, DNX_SAND_VALUE_ABOVE_MAX_ERR, 20, exit);
  rv = sw_state_access[unit].dnx.soc.jer2_arad.tm.modport2sysport.set(unit, modport, sysport);
  DNX_SAND_SOC_CHECK_FUNC_RESULT(rv, 30, exit);


exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_sw_db_modport2sysport_set()", fap_id, fap_port_id);
}

/*
Set a system physical port to device x port mapping.
Performs allocation inside the data structure if needed.
*/
uint32 jer2_arad_sw_db_sysport2modport_set(
    DNX_SAND_IN int unit,
    DNX_SAND_IN JER2_ARAD_SYSPORT sysport,
    DNX_SAND_IN uint32 fap_id,
    DNX_SAND_IN uint32 fap_port_id
  )
{
  soc_error_t rv;
  JER2_ARAD_MODPORT_INFO modport;

  DNX_SAND_INIT_ERROR_DEFINITIONS(0);
  DNX_SAND_ERR_IF_ABOVE_NOF(fap_id, JER2_ARAD_NOF_FAPS_IN_SYSTEM, DNX_SAND_VALUE_ABOVE_MAX_ERR, 10, exit);
  DNX_SAND_ERR_IF_ABOVE_NOF(fap_port_id, JER2_ARAD_NOF_FAP_PORTS, DNX_SAND_VALUE_ABOVE_MAX_ERR, 15, exit);
  DNX_SAND_ERR_IF_ABOVE_NOF(sysport, JER2_ARAD_NOF_SYS_PHYS_PORTS_GET(unit), DNX_SAND_VALUE_ABOVE_MAX_ERR, 20, exit);
  DNX_SAND_CHECK_NULL_PTR(Jer2_arad_sw_db.jer2_arad_device_sw_db[unit], DNX_SAND_ERR, exit);

  modport.fap_id = fap_id;
  modport.fap_port_id = fap_port_id;
  rv = sw_state_access[unit].dnx.soc.jer2_arad.tm.sysport2modport.set(unit, sysport, &modport);
  DNX_SAND_SOC_CHECK_FUNC_RESULT(rv, 30, exit);

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_sw_db_modport2sysport_set()", fap_id, fap_port_id);
}

uint32 jer2_arad_sw_db_modport2sysport_remove(
    DNX_SAND_IN int unit,
    DNX_SAND_IN uint32 fap_id,
    DNX_SAND_IN uint32 fap_port_id
  )
{
  uint32 res = DNX_SAND_OK;

  DNX_SAND_INIT_ERROR_DEFINITIONS(0);
  res = jer2_arad_sw_db_modport2sysport_set(unit,fap_id, fap_port_id, JER2_ARAD_SW_DB_MODPORT2SYSPORT_INVALID_SYSPORT);
  DNX_SAND_CHECK_FUNC_RESULT(res, 10, exit);

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_sw_db_modport2sysport_remove()", fap_id, fap_port_id);
}

/*
Get a fap_id x fap_port_id to system physical port mapping.
If the mapping does not exist, the value of JER2_ARAD_NOF_SYS_PHYS_PORTS is returned
*/
uint32 jer2_arad_sw_db_modport2sysport_get(
    DNX_SAND_IN int unit,
    DNX_SAND_IN uint32 fap_id,
    DNX_SAND_IN uint32 fap_port_id,
    DNX_SAND_OUT JER2_ARAD_SYSPORT *sysport
  )
{
  uint32 modport;
  JER2_ARAD_SYSPORT modport2sysport;
  soc_error_t rv;
  DNX_SAND_INIT_ERROR_DEFINITIONS(0);
  DNX_SAND_CHECK_NULL_INPUT(sysport);
  DNX_SAND_ERR_IF_ABOVE_NOF(fap_id, JER2_ARAD_NOF_FAPS_IN_SYSTEM, DNX_SAND_VALUE_ABOVE_MAX_ERR, 10, exit);
  DNX_SAND_ERR_IF_ABOVE_NOF(fap_port_id, JER2_ARAD_NOF_FAP_PORTS, DNX_SAND_VALUE_ABOVE_MAX_ERR, 15, exit);
  DNX_SAND_CHECK_NULL_PTR(Jer2_arad_sw_db.jer2_arad_device_sw_db[unit], DNX_SAND_ERR, exit);

  modport = fap_id | (fap_port_id << JER2_ARAD_DEVICE_NUMBER_BITS);
  DNX_SAND_ERR_IF_ABOVE_NOF(modport, JER2_ARAD_NOF_MODPORT, DNX_SAND_VALUE_ABOVE_MAX_ERR, 20, exit);
  rv = sw_state_access[unit].dnx.soc.jer2_arad.tm.modport2sysport.get(unit, modport, &modport2sysport);
  DNX_SAND_SOC_CHECK_FUNC_RESULT(rv, 30, exit);
  if (modport2sysport != JER2_ARAD_SW_DB_MODPORT2SYSPORT_INVALID_SYSPORT) {
      *sysport = modport2sysport;
  } else {
      *sysport = JER2_ARAD_SYS_PHYS_PORT_INVALID(unit);
  }

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_sw_db_modport2sysport_get()", fap_id, fap_port_id);
}

/*
Get system physical port to device x port mapping. 
In direct mapping mode the fap and its port may not be found. In this case their returned value will be JER2_ARAD_SW_DB_MODPORT2SYSPORT_GET_NOT_FOUND. 
*/
uint32 jer2_arad_sw_db_sysport2modport_get(
    DNX_SAND_IN int unit,
    DNX_SAND_IN JER2_ARAD_SYSPORT sysport,
    DNX_SAND_OUT uint32 *fap_id,
    DNX_SAND_OUT uint32 *fap_port_id
  )
{
  JER2_ARAD_MODPORT_INFO modport;
  soc_error_t rv;
  DNX_SAND_INIT_ERROR_DEFINITIONS(0);
  DNX_SAND_CHECK_NULL_INPUT(fap_id);
  DNX_SAND_CHECK_NULL_INPUT(fap_port_id);
  DNX_SAND_CHECK_NULL_PTR(Jer2_arad_sw_db.jer2_arad_device_sw_db[unit], DNX_SAND_ERR, exit);
  DNX_SAND_ERR_IF_ABOVE_NOF(sysport, JER2_ARAD_NOF_SYS_PHYS_PORTS_GET(unit), DNX_SAND_VALUE_ABOVE_MAX_ERR, 10, exit);

  rv = sw_state_access[unit].dnx.soc.jer2_arad.tm.sysport2modport.get(unit, sysport, &modport);
  DNX_SAND_SOC_CHECK_FUNC_RESULT(rv, 30, exit);
  *fap_id = modport.fap_id;
  *fap_port_id = modport.fap_port_id;

  if (*fap_id >= JER2_ARAD_NOF_FAPS_IN_SYSTEM ||
      *fap_port_id >= JER2_ARAD_NOF_FAP_PORTS) {
      *fap_id = *fap_port_id = JER2_ARAD_SW_DB_SYSPORT2MODPORT_INVALID_ID;
  }

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_sw_db_modport2sysport_get()", fap_id, fap_port_id);
}

/*
Get a reverse system physical port to fap_id x fap_port_id mapping.
Works by searching the mapping till finding the system physical port.
If the mapping does not exist, the value of JER2_ARAD_SW_DB_MODPORT2SYSPORT_REVERSE_GET_NOT_FOUND is returned in fap_id and in fap_port_id.
In direct mapping mode the fap and its port may not be found. In this case their returned value will be JER2_ARAD_SW_DB_MODPORT2SYSPORT_REVERSE_GET_NOT_FOUND.
*/
uint32 jer2_arad_sw_db_modport2sysport_reverse_get(
    DNX_SAND_IN int unit,
    DNX_SAND_IN JER2_ARAD_SYSPORT sysport,
    DNX_SAND_OUT uint32 *fap_id,
    DNX_SAND_OUT uint32 *fap_port_id
  )
{
  unsigned modport_i;
  JER2_ARAD_SYSPORT modport2sysport;
  soc_error_t rv;

  DNX_SAND_INIT_ERROR_DEFINITIONS(0);
  DNX_SAND_CHECK_NULL_INPUT(fap_id);
  DNX_SAND_CHECK_NULL_INPUT(fap_port_id);
  DNX_SAND_ERR_IF_ABOVE_NOF(sysport, JER2_ARAD_NOF_SYS_PHYS_PORTS_GET(unit), DNX_SAND_VALUE_ABOVE_MAX_ERR, 20, exit);
  DNX_SAND_CHECK_NULL_PTR(Jer2_arad_sw_db.jer2_arad_device_sw_db[unit], DNX_SAND_ERR, exit);

  for (modport_i = 0; modport_i < JER2_ARAD_NOF_MODPORT; ++modport_i) {
      rv = sw_state_access[unit].dnx.soc.jer2_arad.tm.modport2sysport.get(unit, modport_i, &modport2sysport);
      DNX_SAND_SOC_CHECK_FUNC_RESULT(rv, 30, exit);
      if (modport2sysport == sysport){
          *fap_id = modport_i & JER2_ARAD_DEVICE_NUMBER_MASK;
          *fap_port_id = modport_i >> JER2_ARAD_DEVICE_NUMBER_BITS;
          break;
      }
  }

  if (*fap_id >= JER2_ARAD_NOF_FAPS_IN_SYSTEM ||
      *fap_port_id >= JER2_ARAD_NOF_FAP_PORTS ||
      modport_i == JER2_ARAD_NOF_MODPORT) {
      *fap_id = *fap_port_id = JER2_ARAD_SW_DB_MODPORT2SYSPORT_REVERSE_GET_NOT_FOUND;
  }
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_sw_db_modport2sysport_reverse_get()", sysport, 0);
}

uint32
  jer2_arad_sw_db_sysport2queue_set(
    DNX_SAND_IN int             unit,
    DNX_SAND_IN uint32          core_id,
    DNX_SAND_IN JER2_ARAD_SYSPORT    sysport,
    DNX_SAND_IN uint8           valid,
    DNX_SAND_IN uint8           sw_only,
    DNX_SAND_IN uint32          base_queue
   )
{
    uint8 valid_flags = 0;
    uint32 base_queue_lcl;
    int core_index = 0;
    DNXC_INIT_FUNC_DEFS;

    if (!SOC_UNIT_NUM_VALID(unit)) {
        DNXC_EXIT_WITH_ERR(SOC_E_PARAM,(_BSL_DNXC_MSG("invalid unit")));
    }
    
    if (JER2_ARAD_IS_VOQ_MAPPING_INDIRECT(unit) && JER2_ARAD_MAX_SYSTEM_PHYSICAL_PORT_ID_INDIRECT < sysport) {
        DNXC_EXIT_WITH_ERR(SOC_E_PARAM,(_BSL_DNXC_MSG("invalid sysport: %d"), sysport));
    }
    if (JER2_ARAD_IS_VOQ_MAPPING_DIRECT(unit) && JER2_ARAD_MAX_SYSTEM_PHYSICAL_PORT_ID < sysport){
        DNXC_EXIT_WITH_ERR(SOC_E_PARAM,(_BSL_DNXC_MSG("invalid sysport: %d"), sysport));
    }
    if (core_id != SOC_CORE_ALL && core_id >= SOC_DNX_CONFIG(unit)->core_mode.nof_active_cores) {
        DNXC_EXIT_WITH_ERR(SOC_E_PARAM,(_BSL_DNXC_MSG("invalid core ID: %d"), core_id));
    }

    valid_flags = 0;
    if (valid) {
        valid_flags |= JER2_ARAD_SW_DB_SYSPORT_TO_BASE_QUEUE_VALID;
    }
    if (sw_only) {
        valid_flags |= JER2_ARAD_SW_DB_SYSPORT_TO_BASE_QUEUE_SW_ONLY;
    } 
    SOC_DNX_ASSYMETRIC_CORES_ITER(core_id, core_index) {
        DNXC_IF_ERR_EXIT(sw_state_access[unit].dnx.soc.jer2_arad.tm.tm_info.sysport2basequeue.valid_flags.set(unit, (sysport * SOC_DNX_CONFIG(unit)->core_mode.nof_active_cores) + core_index, valid_flags));
        base_queue_lcl = (valid) ? base_queue : JER2_ARAD_IPQ_DESTINATION_ID_INVALID_QUEUE(unit);
        DNXC_IF_ERR_EXIT(sw_state_access[unit].dnx.soc.jer2_arad.tm.tm_info.sysport2basequeue.base_queue.set(unit, (sysport * SOC_DNX_CONFIG(unit)->core_mode.nof_active_cores) + core_index, base_queue_lcl));
    }

    exit:
  DNXC_FUNC_RETURN;
}

uint32
  jer2_arad_sw_db_sysport2queue_get(
    DNX_SAND_IN  int             unit,
    DNX_SAND_IN  int             core_id,
    DNX_SAND_IN  JER2_ARAD_SYSPORT    sysport,
    DNX_SAND_OUT uint8          *valid,
    DNX_SAND_OUT uint8          *sw_only,
    DNX_SAND_OUT uint32         *base_queue
   )
{
    int core_offset = 0;
    uint8 valid_flags = 0;
    DNXC_INIT_FUNC_DEFS;
    if (!SOC_UNIT_NUM_VALID(unit)) {
        DNXC_EXIT_WITH_ERR(SOC_E_PARAM,(_BSL_DNXC_MSG("invalid unit")));
    }
    if (JER2_ARAD_IS_VOQ_MAPPING_INDIRECT(unit) && JER2_ARAD_MAX_SYSTEM_PHYSICAL_PORT_ID_INDIRECT < sysport) {
        DNXC_EXIT_WITH_ERR(SOC_E_PARAM,(_BSL_DNXC_MSG("invalid sysport: %d"), sysport));
    }
    if (JER2_ARAD_IS_VOQ_MAPPING_DIRECT(unit) && JER2_ARAD_MAX_SYSTEM_PHYSICAL_PORT_ID < sysport){
        DNXC_EXIT_WITH_ERR(SOC_E_PARAM,(_BSL_DNXC_MSG("invalid sysport: %d"), sysport));
    }
    if (core_id != SOC_CORE_ALL && !SOC_DNX_CORE_MODE_IS_SYMMETRIC(unit)){
        if (core_id >= SOC_DNX_CONFIG(unit)->core_mode.nof_active_cores) {
            DNXC_EXIT_WITH_ERR(SOC_E_PARAM,(_BSL_DNXC_MSG("invalid core_id: %d"),sysport));
        }
        core_offset = core_id;
    } else {
        core_offset = 0;
    }
    DNXC_IF_ERR_EXIT(sw_state_access[unit].dnx.soc.jer2_arad.tm.tm_info.sysport2basequeue.valid_flags.get(unit, (sysport * SOC_DNX_CONFIG(unit)->core_mode.nof_active_cores) + core_offset, &valid_flags));
    
    *valid = (valid_flags & JER2_ARAD_SW_DB_SYSPORT_TO_BASE_QUEUE_VALID) ? TRUE : FALSE;
    *sw_only = (valid_flags & JER2_ARAD_SW_DB_SYSPORT_TO_BASE_QUEUE_SW_ONLY) ? TRUE : FALSE;

    if (*valid) {
        DNXC_IF_ERR_EXIT(sw_state_access[unit].dnx.soc.jer2_arad.tm.tm_info.sysport2basequeue.base_queue.get(unit, (sysport * SOC_DNX_CONFIG(unit)->core_mode.nof_active_cores) + core_offset, base_queue));
    } else {
        *base_queue = JER2_ARAD_IPQ_DESTINATION_ID_INVALID_QUEUE(unit);
    }
   
exit:
  DNXC_FUNC_RETURN;
}

uint32
  jer2_arad_sw_db_queuequartet2sysport_set(
    DNX_SAND_IN int             unit,
    DNX_SAND_IN uint32          core_id,
    DNX_SAND_IN uint32          queue_quartet,
    DNX_SAND_IN JER2_ARAD_SYSPORT    sysport
   )
{
    int core_index = 0;
    DNXC_INIT_FUNC_DEFS;

    if (!SOC_UNIT_NUM_VALID(unit)) {
        DNXC_EXIT_WITH_ERR(SOC_E_PARAM,(_BSL_DNXC_MSG("invalid unit")));
    }
    
    if (JER2_ARAD_NOF_SYS_PHYS_PORTS_GET(unit) < sysport) {
        DNXC_EXIT_WITH_ERR(SOC_E_PARAM,(_BSL_DNXC_MSG("invalid sysport: %d"), sysport));
    }

    if (core_id != SOC_CORE_ALL && core_id >= SOC_DNX_CONFIG(unit)->core_mode.nof_active_cores) {
        DNXC_EXIT_WITH_ERR(SOC_E_PARAM,(_BSL_DNXC_MSG("invalid core ID: %d"), core_id));
    }

    SOC_DNX_ASSYMETRIC_CORES_ITER(core_id, core_index) {
        DNXC_IF_ERR_EXIT(sw_state_access[unit].dnx.soc.jer2_arad.tm.tm_info.queuequartet2sysport.set(unit, (queue_quartet * SOC_DNX_CONFIG(unit)->core_mode.nof_active_cores) + core_index, sysport));
    }

exit:
  DNXC_FUNC_RETURN;
}

uint32
  jer2_arad_sw_db_queuequartet2sysport_get(
    DNX_SAND_IN int             unit,
    DNX_SAND_IN uint32          core_id,
    DNX_SAND_IN uint32          queue_quartet,
    DNX_SAND_OUT JER2_ARAD_SYSPORT    *sysport
   )
{
    int core_index = 0;
    DNXC_INIT_FUNC_DEFS;

    if (!SOC_UNIT_NUM_VALID(unit)) {
        DNXC_EXIT_WITH_ERR(SOC_E_PARAM,(_BSL_DNXC_MSG("invalid unit")));
    }
    
    if (core_id != SOC_CORE_ALL && core_id >= SOC_DNX_CONFIG(unit)->core_mode.nof_active_cores) {
        DNXC_EXIT_WITH_ERR(SOC_E_PARAM,(_BSL_DNXC_MSG("invalid core ID: %d"), core_id));
    }

    SOC_DNX_ASSYMETRIC_CORES_ITER(core_id, core_index) {
        DNXC_IF_ERR_EXIT(sw_state_access[unit].dnx.soc.jer2_arad.tm.tm_info.queuequartet2sysport.get(unit, (queue_quartet * SOC_DNX_CONFIG(unit)->core_mode.nof_active_cores) + core_index, sysport));
        if (*sysport == JER2_ARAD_SW_DB_MODPORT2SYSPORT_INVALID_SYSPORT) {
            *sysport = JER2_ARAD_SYS_PHYS_PORT_INVALID(unit);
        }
    }

exit:
  DNXC_FUNC_RETURN;
}

uint32
  jer2_arad_sw_db_queue_type_ref_count_exchange(
    DNX_SAND_IN  int            unit,
    DNX_SAND_IN  int            core,
    DNX_SAND_IN  uint8          orig_q_type,
    DNX_SAND_IN  uint8          new_q_type,
    DNX_SAND_IN  int            nof_additions)
{    
    soc_error_t
        rv;
    uint32 
        orig_q_type_ref_count, new_q_type_ref_count;
    int 
        nof_queue_remaped;
    DNXC_INIT_FUNC_DEFS;
    if (!SOC_UNIT_NUM_VALID(unit)) {
        DNXC_EXIT_WITH_ERR(SOC_E_PARAM,(_BSL_DNXC_MSG("Invalid unit: %d"), unit));
    }
    if((core < 0 || core >= SOC_DNX_CONFIG(unit)->core_mode.nof_active_cores) && core != BCM_CORE_ALL){
        DNXC_EXIT_WITH_ERR(SOC_E_PARAM,(_BSL_DNXC_MSG("Core %d out of range"), core));
    } else if (core == BCM_CORE_ALL) {
        nof_queue_remaped = SOC_DNX_CONFIG(unit)->core_mode.nof_active_cores * nof_additions;
    } else {
        nof_queue_remaped = nof_additions;
    }
    if (orig_q_type >= DNX_TMC_ITM_NOF_QT_NDXS && orig_q_type != DNX_TMC_ITM_QT_NDX_INVALID) {
        DNXC_EXIT_WITH_ERR(SOC_E_PARAM,(_BSL_DNXC_MSG("Invalid queue type: %d"), orig_q_type));
    }
    if (new_q_type >= DNX_TMC_ITM_NOF_QT_NDXS) {
        DNXC_EXIT_WITH_ERR(SOC_E_PARAM,(_BSL_DNXC_MSG("Invalid queue type: %d"), new_q_type));
    }

    if (orig_q_type != DNX_TMC_ITM_QT_NDX_INVALID) {
        /*decrease original q type ref count*/
        rv = sw_state_access[unit].dnx.soc.jer2_arad.tm.q_type_ref_count.get(unit, orig_q_type, &orig_q_type_ref_count);
        DNXC_IF_ERR_EXIT(rv);

        if (!orig_q_type_ref_count) {
            DNXC_EXIT_WITH_ERR(SOC_E_PARAM,(_BSL_DNXC_MSG("No Queues mapped to queue type: %d"), orig_q_type));
        }
        orig_q_type_ref_count -= nof_queue_remaped;

        rv = sw_state_access[unit].dnx.soc.jer2_arad.tm.q_type_ref_count.set(unit, orig_q_type, orig_q_type_ref_count);
        DNXC_IF_ERR_EXIT(rv);
    }

    /*increase original q type ref count*/
    rv = sw_state_access[unit].dnx.soc.jer2_arad.tm.q_type_ref_count.get(unit, new_q_type, &new_q_type_ref_count);
    DNXC_IF_ERR_EXIT(rv);

    new_q_type_ref_count += nof_queue_remaped;

    rv = sw_state_access[unit].dnx.soc.jer2_arad.tm.q_type_ref_count.set(unit, new_q_type, new_q_type_ref_count);
    DNXC_IF_ERR_EXIT(rv);

exit:
    DNXC_FUNC_RETURN;
}

/* Get the hardware queue type mapped to from the user queue type. Returns JER2_ARAD_SW_DB_QUEUE_TYPE_NOT_AVAILABLE in mapped_q_type if not found */
uint32
  jer2_arad_sw_db_queue_type_map_get(
    DNX_SAND_IN  int         unit,
    DNX_SAND_IN  uint8          user_q_type,  /* input user queue type (predefined type or user defined allocated type) */
    DNX_SAND_OUT uint8*         mapped_q_type /* output hardware queue type, 0 if not mapped */
  )
{
  soc_error_t rv;
  DNX_SAND_INIT_ERROR_DEFINITIONS(0);
  DNX_SAND_CHECK_NULL_INPUT(mapped_q_type);

  if (user_q_type == DNX_TMC_ITM_QT_NDX_INVALID) {
       *mapped_q_type = DNX_TMC_ITM_QT_NDX_INVALID;
  } else if (JER2_ARAD_SW_DB_QUEUE_TYPE_IS_DYNAMIC(unit, user_q_type)) {
      rv = sw_state_access[unit].dnx.soc.jer2_arad.tm.q_type_map.get(unit,
        user_q_type - JER2_ARAD_SW_DB_1ST_DYNAMIC_QUEUE_TYPE(unit),
        mapped_q_type);
      DNX_SAND_SOC_CHECK_FUNC_RESULT(rv, 10, exit);
      if (!*mapped_q_type) {
          *mapped_q_type = JER2_ARAD_SW_DB_QUEUE_TYPE_NOT_AVAILABLE;
      }
  } else if (user_q_type >= DNX_TMC_ITM_PREDEFIEND_OFFSET &&
             (user_q_type < DNX_TMC_ITM_PREDEFIEND_OFFSET + DNX_TMC_ITM_NOF_QT_STATIC ||
              user_q_type == DNX_TMC_ITM_PREDEFIEND_OFFSET + DNX_TMC_ITM_QT_PUSH_Q_NDX)) {
      *mapped_q_type = user_q_type - DNX_TMC_ITM_PREDEFIEND_OFFSET;
  } else {
      DNX_SAND_SET_ERROR_CODE(JER2_ARAD_ITM_IPS_QT_RNG_OUT_OF_RANGE_ERR, 10, exit); /* unsupported user queue type */
  }
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_sw_db_queue_type_map_get()", unit, user_q_type);
}

/*
 * Get the hardware queue type mapped to from the user queue type, allocating it if it was not allocated before.
 * Returns JER2_ARAD_SW_DB_QUEUE_TYPE_NOT_AVAILABLE in mapped_q_type if mapping is not possible since all hardware types (credit request profiles) are used.
 * If given a predefined queue type, will just return it as output as it does not use dynamic allocation.
 */
uint32
  jer2_arad_sw_db_queue_type_map_get_alloc(
    DNX_SAND_IN  int         unit,
    DNX_SAND_IN  uint8          user_q_type,  /* input user queue type (predefined type or user defined allocated type) */
    DNX_SAND_OUT uint8*         mapped_q_type /* output hardware queue type, 0 if not mapped */
  )
{
  uint8       q_type_map;
  soc_error_t rv;
    
  DNX_SAND_INIT_ERROR_DEFINITIONS(0);
  DNX_SAND_CHECK_NULL_INPUT(mapped_q_type);

  if (JER2_ARAD_SW_DB_QUEUE_TYPE_IS_DYNAMIC(unit, user_q_type)) {
      rv = sw_state_access[unit].dnx.soc.jer2_arad.tm.q_type_map.get(unit,
        user_q_type - JER2_ARAD_SW_DB_1ST_DYNAMIC_QUEUE_TYPE(unit),
        &q_type_map);
      DNX_SAND_SOC_CHECK_FUNC_RESULT(rv, 10, exit);
      if (SOC_IS_JERICHO(unit)) {
          /*in Jericho there are more profiles so we staticly map between user_q_type an HW, and never try to catch preconfigure profiles*/
          if (!q_type_map) {
                rv = sw_state_access[unit].dnx.soc.jer2_arad.tm.q_type_map.set(
                        unit,
                        user_q_type - JER2_ARAD_SW_DB_1ST_DYNAMIC_QUEUE_TYPE(unit),
                        user_q_type);
                DNX_SAND_SOC_CHECK_FUNC_RESULT(rv, 20, exit);
                *mapped_q_type = user_q_type;
              } else {
                  *mapped_q_type = q_type_map;
              }
          } else {
              if (!q_type_map) { /* if the user queue is not mapped, try to map it */
                  uint8 reverse_mapping[JER2_ARAD_SW_NOF_AVAILABLE_HW_QUEUE - 1 - JER2_ARAD_SW_1ST_AVAILABLE_HW_QUEUE] = {0};
                  uint8 i, hw_q;
                  for (i = JER2_ARAD_SW_DB_1ST_DYNAMIC_QUEUE_TYPE(unit); i < JER2_ARAD_SW_NOF_AVAILABLE_HW_QUEUE - 1; ++i) {
                      rv = sw_state_access[unit].dnx.soc.jer2_arad.tm.q_type_map.get(unit,
                        i - JER2_ARAD_SW_DB_1ST_DYNAMIC_QUEUE_TYPE(unit),
                        &q_type_map);
                      DNX_SAND_SOC_CHECK_FUNC_RESULT(rv, 30, exit);
                      if ((hw_q = q_type_map)) {
                          hw_q -= JER2_ARAD_SW_1ST_AVAILABLE_HW_QUEUE;
                          if ((hw_q >= JER2_ARAD_SW_NOF_AVAILABLE_HW_QUEUE - 1 - JER2_ARAD_SW_1ST_AVAILABLE_HW_QUEUE) || reverse_mapping[hw_q]) {
                              DNX_SAND_SET_ERROR_CODE(JER2_ARAD_INTERNAL_ASSERT_ERR, 99, exit); /* internal error */
                          }
                          reverse_mapping[hw_q] = 1;
                      }
                  }
                  hw_q = 0;
                  for (i = 0; i < JER2_ARAD_SW_NOF_AVAILABLE_HW_QUEUE - 1 - JER2_ARAD_SW_1ST_AVAILABLE_HW_QUEUE; ++i) {
                      if (!reverse_mapping[i]) {
                          hw_q = i + JER2_ARAD_SW_1ST_AVAILABLE_HW_QUEUE;
                          break;
                      }
                  }
                  rv = sw_state_access[unit].dnx.soc.jer2_arad.tm.q_type_map.set(unit,
                    user_q_type - JER2_ARAD_SW_DB_1ST_DYNAMIC_QUEUE_TYPE(unit),
                    hw_q ? hw_q : JER2_ARAD_SW_DB_QUEUE_TYPE_NOT_AVAILABLE /* also handle the case of no available hardware queue */);
                  DNX_SAND_SOC_CHECK_FUNC_RESULT(rv, 40, exit);
              }
              rv = sw_state_access[unit].dnx.soc.jer2_arad.tm.q_type_map.get(unit,
                user_q_type - JER2_ARAD_SW_DB_1ST_DYNAMIC_QUEUE_TYPE(unit),
                mapped_q_type);
              DNX_SAND_SOC_CHECK_FUNC_RESULT(rv, 50, exit);
          }
      } else if ((user_q_type >= DNX_TMC_ITM_PREDEFIEND_OFFSET && user_q_type < DNX_TMC_ITM_PREDEFIEND_OFFSET + DNX_TMC_ITM_NOF_QT_STATIC) ||
                 (user_q_type == DNX_TMC_ITM_QT_PUSH_Q_NDX + DNX_TMC_ITM_PREDEFIEND_OFFSET && !SOC_IS_ARADPLUS_AND_BELOW(unit))) {
          *mapped_q_type = user_q_type - DNX_TMC_ITM_PREDEFIEND_OFFSET;
      } else {
        DNX_SAND_SET_ERROR_CODE(JER2_ARAD_ITM_IPS_QT_RNG_OUT_OF_RANGE_ERR, 10, exit); /* unsupported user queue type */
      }
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_sw_db_queue_type_map_get()", unit, user_q_type);
}

/* Get the user queue type mapped from the given hardware queue type. */
uint32
  jer2_arad_sw_db_queue_type_map_reverse_get(
    DNX_SAND_IN  int         unit,
    DNX_SAND_IN  uint8          mapped_q_type,  /* input hardware queue type, 0 if not mapped */
    DNX_SAND_OUT uint8*         user_q_type     /* output user queue type (predefined type or user defined allocated type */
  )
{
  uint8       q_type_map;
  soc_error_t rv;
  DNX_SAND_INIT_ERROR_DEFINITIONS(0);
  DNX_SAND_CHECK_NULL_INPUT(user_q_type);

  if (mapped_q_type < DNX_TMC_ITM_NOF_QT_STATIC || mapped_q_type == DNX_TMC_ITM_QT_PUSH_Q_NDX) {
    *user_q_type = mapped_q_type + DNX_TMC_ITM_PREDEFIEND_OFFSET;
  } else if (SOC_IS_JERICHO(unit)) {
      *user_q_type = mapped_q_type;
  } else {
    uint8 i;
    if (mapped_q_type >= JER2_ARAD_SW_NOF_AVAILABLE_HW_QUEUE) {
        DNX_SAND_SET_ERROR_CODE(JER2_ARAD_INTERNAL_ASSERT_ERR, 20, exit); /* internal error */
    }
    for (i = 0; i < JER2_ARAD_SW_DB_NOF_LEGAL_DYNAMIC_QUEUE_TYPES(unit); ++i) {
      rv = sw_state_access[unit].dnx.soc.jer2_arad.tm.q_type_map.get(unit, i, &q_type_map);
      DNX_SAND_SOC_CHECK_FUNC_RESULT(rv, 10, exit);
      if (q_type_map == mapped_q_type) {
        break;
      }
    }
    if (i >= JER2_ARAD_SW_DB_NOF_LEGAL_DYNAMIC_QUEUE_TYPES(unit)) {
      DNX_SAND_SET_ERROR_CODE(JER2_ARAD_INTERNAL_ASSERT_ERR, 20, exit); /* no user queue type is mapped to this hardware value; */
    }
    *user_q_type = i + JER2_ARAD_SW_DB_1ST_DYNAMIC_QUEUE_TYPE(unit);
  }

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_sw_db_queue_type_map_reverse_get()", unit, mapped_q_type);
}

uint32 
    jer2_arad_sw_db_rate_class_ref_count_get(
       DNX_SAND_IN  int                         unit, 
       DNX_SAND_IN  int                         core_id, 
       DNX_SAND_IN  uint32                      is_ocb_only, 
       DNX_SAND_IN  uint32                      rate_class, 
       DNX_SAND_OUT uint32*                     ref_count)
{
    int rv = SOC_E_NONE;
    uint32 
        dram_mixed_ref_count = 0, dram_mixed_ref_count_sum = 0,
        ocb_only_ref_count = 0, ocb_only_ref_count_sum = 0;
    int core_index;
    DNXC_INIT_FUNC_DEFS;
    if (!SOC_UNIT_NUM_VALID(unit)) {
        DNXC_EXIT_WITH_ERR(SOC_E_PARAM,(_BSL_DNXC_MSG("Invalid unit: %d"), unit));
    }
    if((core_id < 0 || core_id >= SOC_DNX_CONFIG(unit)->core_mode.nof_active_cores) && core_id != BCM_CORE_ALL){
        DNXC_EXIT_WITH_ERR(SOC_E_PARAM,(_BSL_DNXC_MSG("Core %d out of range"), core_id));
    }
    DNXC_NULL_CHECK(ref_count);
    SOC_DNX_CORES_ITER(core_id, core_index) {
        rv = sw_state_access[unit].dnx.soc.jer2_arad.tm.tm_info.queue_to_rate_class_mapping.ref_count.get(unit, core_index, rate_class, &dram_mixed_ref_count);
        DNXC_IF_ERR_EXIT(rv);
        dram_mixed_ref_count_sum += dram_mixed_ref_count;
        rv = sw_state_access[unit].dnx.soc.jer2_arad.tm.tm_info.queue_to_rate_class_mapping.ocb_only_ref_count.get(unit, core_index, rate_class, &ocb_only_ref_count);
        DNXC_IF_ERR_EXIT(rv);
        ocb_only_ref_count_sum += ocb_only_ref_count;
    }
    if (is_ocb_only == TRUE) {
        *ref_count = ocb_only_ref_count_sum;
    } else if (is_ocb_only == FALSE) {
        *ref_count = dram_mixed_ref_count_sum - ocb_only_ref_count_sum;
    } else {
        *ref_count = dram_mixed_ref_count_sum;
    }

exit:
    DNXC_FUNC_RETURN;
}

uint32 
    jer2_arad_sw_db_tm_queue_to_rate_class_mapping_ref_count_exchange(
       DNX_SAND_IN  int                         unit,
       DNX_SAND_IN  int                         core, 
       DNX_SAND_IN  uint32                      is_ocb_only,
       DNX_SAND_IN  uint32                      old_rate_class,
       DNX_SAND_IN  uint32                      new_rate_class,
       DNX_SAND_IN  int                         nof_additions) 
{
    uint32 res = DNX_SAND_OK;
    uint32 new_ref_count, old_ref_count;
    int core_index;
    int nof_queue_remaped;
    DNX_SAND_INIT_ERROR_DEFINITIONS(0);
    if((core < 0 || core >= SOC_DNX_CONFIG(unit)->core_mode.nof_active_cores) && core != BCM_CORE_ALL){
        res = SOC_E_PARAM;
        DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 5, exit);
    } else {
        nof_queue_remaped = nof_additions;
    }
    if (old_rate_class != new_rate_class) {
        SOC_DNX_CORES_ITER(core, core_index) {
            DNX_SAND_ERR_IF_ABOVE_MAX(old_rate_class, DNX_TMC_ITM_NOF_RATE_CLASSES ,JER2_ARAD_ITM_QT_RT_CLS_RNG_OUT_OF_RANGE_ERR, 10, exit);
            DNX_SAND_ERR_IF_ABOVE_MAX(new_rate_class, DNX_TMC_ITM_NOF_RATE_CLASSES ,JER2_ARAD_ITM_QT_RT_CLS_RNG_OUT_OF_RANGE_ERR, 20, exit);
            if (new_rate_class != DNX_TMC_ITM_NOF_RATE_CLASSES) {
                res = sw_state_access[unit].dnx.soc.jer2_arad.tm.tm_info.queue_to_rate_class_mapping.ref_count.get(unit, core_index, new_rate_class, &new_ref_count);
                DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 30, exit);

                DNX_SAND_ERR_IF_ABOVE_MAX(new_ref_count, JER2_ARAD_MAX_QUEUE_ID(unit), JER2_ARAD_ITM_QT_RT_CLS_RNG_OUT_OF_RANGE_ERR, 40, exit);

                res = sw_state_access[unit].dnx.soc.jer2_arad.tm.tm_info.queue_to_rate_class_mapping.ref_count.set(unit, core_index, new_rate_class, new_ref_count + nof_queue_remaped);
                DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 50, exit);
            }

            if (old_rate_class != DNX_TMC_ITM_NOF_RATE_CLASSES) {
                res = sw_state_access[unit].dnx.soc.jer2_arad.tm.tm_info.queue_to_rate_class_mapping.ref_count.get(unit, core_index, old_rate_class, &old_ref_count);
                DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 60, exit);

                res = sw_state_access[unit].dnx.soc.jer2_arad.tm.tm_info.queue_to_rate_class_mapping.ref_count.set(unit, core_index, old_rate_class, old_ref_count - nof_queue_remaped);
                DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 70, exit);
            }

            if (is_ocb_only) {
                if (new_rate_class != DNX_TMC_ITM_NOF_RATE_CLASSES) {
                    res = sw_state_access[unit].dnx.soc.jer2_arad.tm.tm_info.queue_to_rate_class_mapping.ocb_only_ref_count.get(unit, core_index, new_rate_class, &new_ref_count);
                    DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 130, exit);

                    DNX_SAND_ERR_IF_ABOVE_MAX(new_ref_count, JER2_ARAD_MAX_QUEUE_ID(unit), JER2_ARAD_ITM_QT_RT_CLS_RNG_OUT_OF_RANGE_ERR, 140, exit);

                    res = sw_state_access[unit].dnx.soc.jer2_arad.tm.tm_info.queue_to_rate_class_mapping.ocb_only_ref_count.set(unit, core_index, new_rate_class, new_ref_count + nof_queue_remaped);
                    DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 150, exit);
                }

                if (old_rate_class != DNX_TMC_ITM_NOF_RATE_CLASSES) {
                    res = sw_state_access[unit].dnx.soc.jer2_arad.tm.tm_info.queue_to_rate_class_mapping.ocb_only_ref_count.get(unit, core_index, old_rate_class, &old_ref_count);
                    DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 160, exit);

                    res = sw_state_access[unit].dnx.soc.jer2_arad.tm.tm_info.queue_to_rate_class_mapping.ocb_only_ref_count.set(unit, core_index, old_rate_class, old_ref_count - nof_queue_remaped);
                    DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 170, exit);
                }
            }
        }
    }
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_sw_db_tm_queue_to_rate_class_mapping_ref_count_exchange()",0,0);
}

/* Mark the given egress multicast group as open or not in SWDB */
uint32 jer2_arad_sw_db_egress_group_open_set(
    DNX_SAND_IN  int     unit, /* device */
    DNX_SAND_IN  uint32  group_id,  /* multicast ID */
    DNX_SAND_IN  uint8   is_open    /* non zero value will mark the group as open */
)
{
    soc_error_t rv;
  
  DNXC_INIT_FUNC_DEFS;
  if (!SOC_UNIT_NUM_VALID(unit)) {
    DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("invalid unit")));
  } else if (group_id >= SOC_DNX_CONFIG(unit)->tm.nof_mc_ids) {
    DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("input too big")));
  }



  if(is_open) {
      rv = sw_state_access[unit].dnx.soc.jer2_arad.tm.jer2_arad_multicast.egress_groups_open_data.bit_set(unit, group_id);
  } else {
      rv = sw_state_access[unit].dnx.soc.jer2_arad.tm.jer2_arad_multicast.egress_groups_open_data.bit_clear(unit, group_id);
  }
  DNXC_IF_ERR_EXIT(rv);

exit:
  DNXC_FUNC_RETURN;
}


/* Mark all egress multicast groups as open or not in SWDB */
uint32 jer2_arad_sw_db_egress_group_open_set_all(
    DNX_SAND_IN  int     unit, /* device */
    DNX_SAND_IN  uint8   is_open    /* non zero value will mark the group as open */
)
{
    uint32 i;
    uint8 cur_bit;
    soc_error_t rv;
  
    DNXC_INIT_FUNC_DEFS;
    if (!SOC_UNIT_NUM_VALID(unit)) {
        DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("invalid unit")));
    }

    for (i = 0; i < SOC_DNX_CONFIG(unit)->tm.nof_mc_ids; ++i) {
        rv = sw_state_access[unit].dnx.soc.jer2_arad.tm.jer2_arad_multicast.egress_groups_open_data.bit_get(unit, i , &cur_bit);
        DNXC_IF_ERR_EXIT(rv);
        if ((cur_bit ? 1: 0) != (is_open ? 1 : 0)) {
            if(is_open) {
                rv = sw_state_access[unit].dnx.soc.jer2_arad.tm.jer2_arad_multicast.egress_groups_open_data.bit_set(unit, i);
            } else {
                rv = sw_state_access[unit].dnx.soc.jer2_arad.tm.jer2_arad_multicast.egress_groups_open_data.bit_clear(unit, i);
            }
            DNXC_IF_ERR_EXIT(rv);
        }
    }

exit:
  DNXC_FUNC_RETURN;
}

#include <soc/dnx/legacy/SAND/Utils/sand_footer.h>

#endif /* of #if defined(BCM_88690_A0) */

