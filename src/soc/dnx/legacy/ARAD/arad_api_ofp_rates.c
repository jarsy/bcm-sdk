#include <soc/mcm/memregs.h>
#if defined(BCM_88690_A0)
/* $Id: jer2_arad_api_ofp_rates.c,v 1.7 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/

#ifdef _ERR_MSG_MODULE_NAME
  #error "_ERR_MSG_MODULE_NAME redefined"
#endif

#define _ERR_MSG_MODULE_NAME BSL_SOC_COSQ
/*************
 * INCLUDES  *
 *************/
/* { */
#include <shared/bsl.h>
#include <soc/error.h>
#include <soc/dnxc/legacy/error.h>
#include <soc/dnx/legacy/SAND/Utils/sand_header.h>

#include <soc/dnx/legacy/drv.h>
#include <soc/dnx/legacy/ARAD/arad_api_ofp_rates.h>
#include <soc/dnx/legacy/ARAD/arad_ofp_rates.h>
#include <soc/dnx/legacy/ARAD/arad_scheduler_ports.h>


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


uint32
    jer2_arad_ofp_rates_port2chan_arb(
    DNX_SAND_IN  int                unit,
    DNX_SAND_IN  uint32                fap_port,
    DNX_SAND_OUT  JER2_ARAD_OFP_RATES_EGQ_CHAN_ARB_ID *chan_arb_id
    )
{
   uint32
    res = DNX_SAND_OK;

  DNXC_INIT_FUNC_DEFS
  /* DNX_SAND_CHECK_DRIVER_AND_DEVICE; */;
  /* DNX_SAND_TAKE_DEVICE_SEMAPHORE; */

  DNXC_NULL_CHECK(chan_arb_id);

  res = jer2_arad_ofp_rates_port2chan_arb_unsafe(unit,
                                            fap_port,
                                            chan_arb_id);
  DNXC_IF_ERR_EXIT(res);

  exit:
    DNXC_FUNC_RETURN
}

int
  jer2_arad_ofp_rates_egq_single_port_rate_sw_set(
    DNX_SAND_IN  int                    unit,
    DNX_SAND_IN  int                    core,
    DNX_SAND_IN  uint32                 tm_port,
    DNX_SAND_IN  uint32                 rate
  )
{
  int
    res = DNX_SAND_OK;

  DNXC_INIT_FUNC_DEFS
  /* DNX_SAND_CHECK_DRIVER_AND_DEVICE; */;
  /* DNX_SAND_TAKE_DEVICE_SEMAPHORE; */

  res = jer2_arad_ofp_rates_single_port_verify(
          unit,
          &rate
        );
  DNXC_IF_ERR_EXIT(res);

  res = jer2_arad_ofp_rates_egq_single_port_rate_sw_set_unsafe(
          unit,
          core,
          tm_port,
          rate
        );
  DNXC_IF_ERR_EXIT(res);


  /* DNX_SAND_GIVE_DEVICE_SEMAPHORE; */
exit:
  DNXC_FUNC_RETURN
}

int
  jer2_arad_ofp_rates_egq_single_port_rate_hw_set(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  int                 core,
    DNX_SAND_IN  uint32              tm_port

  )
{
  uint32
    res = DNX_SAND_OK;

  DNXC_INIT_FUNC_DEFS
  /* DNX_SAND_CHECK_DRIVER_AND_DEVICE; */


  res = jer2_arad_ofp_rates_egq_single_port_rate_hw_set_unsafe(
          unit
        );
  DNXC_IF_ERR_EXIT(res);


  /* DNX_SAND_GIVE_DEVICE_SEMAPHORE; */
exit:
  DNXC_FUNC_RETURN
}

int
  jer2_arad_ofp_rates_sch_single_port_rate_hw_set(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  int                 core,
    DNX_SAND_IN  uint32              tm_port

  )
{
  uint32
    res = DNX_SAND_OK;

  DNXC_INIT_FUNC_DEFS
  /* DNX_SAND_CHECK_DRIVER_AND_DEVICE; */


  res = jer2_arad_ofp_rates_sch_single_port_rate_hw_set_unsafe(
          unit
        );
  DNXC_IF_ERR_EXIT(res);


  /* DNX_SAND_GIVE_DEVICE_SEMAPHORE; */
exit:
  DNXC_FUNC_RETURN
}

int
  jer2_arad_ofp_rates_single_port_max_burst_set(
    DNX_SAND_IN  int                    unit,
    DNX_SAND_IN  int                    core,
    DNX_SAND_IN  uint32                 tm_port,
    DNX_SAND_IN  uint32                 max_burst
  )
{
  int
    res = DNX_SAND_OK;

  DNXC_INIT_FUNC_DEFS
  /* DNX_SAND_CHECK_DRIVER_AND_DEVICE; */;

  /* DNX_SAND_TAKE_DEVICE_SEMAPHORE; */

  res = jer2_arad_ofp_rates_single_port_verify(
          unit,
          &max_burst
        );
  DNXC_IF_ERR_EXIT(res);

  res = jer2_arad_ofp_rates_single_port_max_burst_set_unsafe(
          unit,
          core,
          tm_port,
          max_burst
        );
  DNXC_IF_ERR_EXIT(res);


  /* DNX_SAND_GIVE_DEVICE_SEMAPHORE; */
exit:
  DNXC_FUNC_RETURN
}

int
  jer2_arad_ofp_rates_egq_single_port_rate_hw_get(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  int                 core,
    DNX_SAND_IN  uint32              tm_port,
    DNX_SAND_OUT uint32              *rate
  )
{
  uint32
    res = DNX_SAND_OK;

  DNXC_INIT_FUNC_DEFS

  /* DNX_SAND_CHECK_DRIVER_AND_DEVICE; */;

  DNXC_NULL_CHECK(rate);

  /* DNX_SAND_TAKE_DEVICE_SEMAPHORE; */

  res = jer2_arad_ofp_rates_egq_single_port_rate_hw_get_unsafe(
          unit,
          core,
          tm_port,
          rate
        );
  DNXC_IF_ERR_EXIT(res);


  /* DNX_SAND_GIVE_DEVICE_SEMAPHORE; */
exit:
  DNXC_FUNC_RETURN
}

int
  jer2_arad_ofp_rates_single_port_max_burst_get(
    DNX_SAND_IN  int                    unit,
    DNX_SAND_IN  int                    core,
    DNX_SAND_IN  uint32                 tm_port,
    DNX_SAND_OUT uint32                 *max_burst
  )
{
  int
    res = DNX_SAND_OK;

  DNXC_INIT_FUNC_DEFS

  /* DNX_SAND_CHECK_DRIVER_AND_DEVICE; */;

  DNXC_NULL_CHECK(max_burst);

  /* DNX_SAND_TAKE_DEVICE_SEMAPHORE; */

  res = jer2_arad_ofp_rates_single_port_max_burst_get_unsafe(
          unit,
          core,
          tm_port,
          max_burst
        );
  DNXC_IF_ERR_EXIT(res);


  /* DNX_SAND_GIVE_DEVICE_SEMAPHORE; */
exit:
  DNXC_FUNC_RETURN
}

int
  jer2_arad_ofp_rates_egq_interface_shaper_set(
    DNX_SAND_IN  int                      unit,
    DNX_SAND_IN  int                      core,
    DNX_SAND_IN  uint32                   tm_port,
    DNX_SAND_IN DNX_TMC_OFP_SHPR_UPDATE_MODE rate_update_mode,
    DNX_SAND_IN  uint32                      if_shaper_rate
  )
{
  uint32
    res = DNX_SAND_OK;

  DNXC_INIT_FUNC_DEFS
  /* DNX_SAND_CHECK_DRIVER_AND_DEVICE; */;

  res = jer2_arad_ofp_rates_interface_shaper_verify(
          unit,
          core,
          tm_port
        );
  DNXC_IF_ERR_EXIT(res);

  /* DNX_SAND_TAKE_DEVICE_SEMAPHORE; */

  res = jer2_arad_ofp_rates_egq_interface_shaper_set_unsafe(
          unit,
          core,
          tm_port,
          rate_update_mode,
          if_shaper_rate
        );
  DNXC_IF_ERR_EXIT(res);


  /* DNX_SAND_GIVE_DEVICE_SEMAPHORE; */
exit:
  DNXC_FUNC_RETURN
}

int
  jer2_arad_ofp_rates_egq_interface_shaper_get(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  int                 core,
    DNX_SAND_IN  uint32              tm_port,
    DNX_SAND_OUT uint32             *if_shaper_rate
  )
{
  uint32
    res = DNX_SAND_OK;

  DNXC_INIT_FUNC_DEFS
  /* DNX_SAND_CHECK_DRIVER_AND_DEVICE; */;

  DNXC_NULL_CHECK(if_shaper_rate);

  /* DNX_SAND_TAKE_DEVICE_SEMAPHORE; */

  res = jer2_arad_ofp_rates_egq_interface_shaper_get_unsafe(
          unit,
          core,
          tm_port,
          if_shaper_rate
        );
  DNXC_IF_ERR_EXIT(res);


  /* DNX_SAND_GIVE_DEVICE_SEMAPHORE; */
exit:
  DNXC_FUNC_RETURN
}

/*********************************************************************
*     Configures a single Port-Priority rate to enable bandwidth
*     limiting of a single Port-priority in the
*     end-to-end scheduler and in the egress processor. It is done
*     by setting the calendar in the case of egress processor, 
*     shapers and setting shapers in end-to-end scheduler. 
*     The function re-calculates the appropriate values 
*     from the current values and the updated info. 
*     It also saves the values in the software database 
*     for single-entry changes in the future.
*     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
    jer2_arad_ofp_rates_port_priority_max_burst_for_fc_queues_set(
    DNX_SAND_IN  int unit,
    DNX_SAND_IN  uint32 max_burst_fc_queues
    )
{
  uint32
    res = DNX_SAND_OK;

  DNXC_INIT_FUNC_DEFS
  /* DNX_SAND_CHECK_DRIVER_AND_DEVICE; */;

  res = jer2_arad_ofp_rates_port_priority_max_burst_for_fc_queues_verify(
          unit,
          max_burst_fc_queues
          );
  DNXC_IF_ERR_EXIT(res);

  /* DNX_SAND_TAKE_DEVICE_SEMAPHORE; */

  res = jer2_arad_ofp_rates_port_priority_max_burst_for_fc_queues_set_unsafe(
          unit,
          max_burst_fc_queues
        );
  DNXC_IF_ERR_EXIT(res);


  /* DNX_SAND_GIVE_DEVICE_SEMAPHORE; */
exit:
  DNXC_FUNC_RETURN
}

uint32
    jer2_arad_ofp_rates_port_priority_max_burst_for_empty_queues_set(
    DNX_SAND_IN  int unit,
    DNX_SAND_IN  uint32 max_burst_empty_queues
    )
{
  uint32
    res = DNX_SAND_OK;

  DNXC_INIT_FUNC_DEFS
  /* DNX_SAND_CHECK_DRIVER_AND_DEVICE; */;

  res = jer2_arad_ofp_rates_port_priority_max_burst_for_empty_queues_verify(
          unit,
          max_burst_empty_queues
          );
  DNXC_IF_ERR_EXIT(res);

  /* DNX_SAND_TAKE_DEVICE_SEMAPHORE; */

  res = jer2_arad_ofp_rates_port_priority_max_burst_for_empty_queues_set_unsafe(
          unit,
          max_burst_empty_queues
        );
  DNXC_IF_ERR_EXIT(res);


  /* DNX_SAND_GIVE_DEVICE_SEMAPHORE; */
exit:
  DNXC_FUNC_RETURN
}

uint32
    jer2_arad_ofp_rates_port_priority_max_burst_for_fc_queues_get(
    DNX_SAND_IN  int unit,
    DNX_SAND_OUT  uint32 *max_burst_fc_queues
    )
{
  uint32
    res = DNX_SAND_OK;

  DNXC_INIT_FUNC_DEFS
  /* DNX_SAND_CHECK_DRIVER_AND_DEVICE; */;

  DNXC_NULL_CHECK(max_burst_fc_queues);  

  /* DNX_SAND_TAKE_DEVICE_SEMAPHORE; */

  res = jer2_arad_ofp_rates_port_priority_max_burst_for_fc_queues_get_unsafe(
          unit,
          max_burst_fc_queues
        );
  DNXC_IF_ERR_EXIT(res);


  /* DNX_SAND_GIVE_DEVICE_SEMAPHORE; */
exit:
  DNXC_FUNC_RETURN
}

uint32
    jer2_arad_ofp_rates_port_priority_max_burst_for_empty_queues_get(
    DNX_SAND_IN  int unit,
    DNX_SAND_OUT  uint32 *max_burst_empty_queues
    )
{
  uint32
    res = DNX_SAND_OK;

  DNXC_INIT_FUNC_DEFS
  /* DNX_SAND_CHECK_DRIVER_AND_DEVICE; */;

  DNXC_NULL_CHECK(max_burst_empty_queues);  

  /* DNX_SAND_TAKE_DEVICE_SEMAPHORE; */

  res = jer2_arad_ofp_rates_port_priority_max_burst_for_empty_queues_get_unsafe(
          unit,
          max_burst_empty_queues
        );
  DNXC_IF_ERR_EXIT(res);


  /* DNX_SAND_GIVE_DEVICE_SEMAPHORE; */
exit:
  DNXC_FUNC_RETURN
}

uint32
    jer2_arad_ofp_rates_egq_tcg_max_burst_set(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  int                 core,
    DNX_SAND_IN  uint32              tm_port,   
    DNX_SAND_IN  uint32              tcg_ndx, 
    DNX_SAND_IN  uint32              max_burst
    )
{
  uint32
    res = DNX_SAND_OK;

  DNXC_INIT_FUNC_DEFS
  /* DNX_SAND_CHECK_DRIVER_AND_DEVICE; */;

  res = jer2_arad_ofp_rates_tcg_shaper_verify(unit,
                                         core,
                                         tm_port,
                                         tcg_ndx,
                                         max_burst,
                                         JER2_ARAD_OFP_RATES_GENERIC_FUNC_STATE_TCG_EGQ_BURST);
  DNXC_IF_ERR_EXIT(res);

  /* DNX_SAND_TAKE_DEVICE_SEMAPHORE; */

  res = jer2_arad_ofp_rates_egq_tcg_max_burst_set_unsafe(unit,
                                                    core, 
                                                    tm_port,  
                                                    tcg_ndx,   
                                                    max_burst);

  DNXC_IF_ERR_EXIT(res);


  /* DNX_SAND_GIVE_DEVICE_SEMAPHORE; */
exit:
  DNXC_FUNC_RETURN
}

uint32
    jer2_arad_ofp_rates_sch_tcg_max_burst_set(
    DNX_SAND_IN  int                   unit,
    DNX_SAND_IN  int                   core,
    DNX_SAND_IN  uint32                tm_port,   
    DNX_SAND_IN  uint32                tcg_ndx, 
    DNX_SAND_IN  uint32                max_burst
    )
{
  uint32
    res = DNX_SAND_OK;

  DNXC_INIT_FUNC_DEFS
  /* DNX_SAND_CHECK_DRIVER_AND_DEVICE; */;

  res = jer2_arad_ofp_rates_tcg_shaper_verify(unit,
                                         core,
                                         tm_port,
                                         tcg_ndx,
                                         max_burst,
                                         JER2_ARAD_OFP_RATES_GENERIC_FUNC_STATE_TCG_SCH_BURST);
  DNXC_IF_ERR_EXIT(res);

  /* DNX_SAND_TAKE_DEVICE_SEMAPHORE; */

  res = jer2_arad_ofp_rates_sch_tcg_max_burst_set_unsafe(unit,
                                                    core,
                                                    tm_port,  
                                                    tcg_ndx,   
                                                    max_burst);

  DNXC_IF_ERR_EXIT(res);


  /* DNX_SAND_GIVE_DEVICE_SEMAPHORE; */
exit:
  DNXC_FUNC_RETURN
}

uint32
    jer2_arad_ofp_rates_egq_tcg_rate_sw_set(
    DNX_SAND_IN  int                   unit,
    DNX_SAND_IN  int                   core,
    DNX_SAND_IN  uint32                tm_port,   
    DNX_SAND_IN  uint32                tcg_ndx, 
    DNX_SAND_IN  uint32                rate
    )
{
  uint32
    res = DNX_SAND_OK;

  DNXC_INIT_FUNC_DEFS
  /* DNX_SAND_CHECK_DRIVER_AND_DEVICE; */;

  res = jer2_arad_ofp_rates_tcg_shaper_verify(unit,
                                         core,
                                         tm_port,
                                         tcg_ndx,
                                         rate,
                                         JER2_ARAD_OFP_RATES_GENERIC_FUNC_STATE_TCG_EGQ_RATE);
  DNXC_IF_ERR_EXIT(res);

  /* DNX_SAND_TAKE_DEVICE_SEMAPHORE; */

  res = jer2_arad_ofp_rates_egq_tcg_rate_sw_set_unsafe(unit,
                                                  core, 
                                                  tm_port,  
                                                  tcg_ndx,   
                                                  rate);

  DNXC_IF_ERR_EXIT(res);


  /* DNX_SAND_GIVE_DEVICE_SEMAPHORE; */
exit:
  DNXC_FUNC_RETURN
}

uint32
    jer2_arad_ofp_rates_egq_tcg_rate_hw_set(
    DNX_SAND_IN  int                   unit
    )
{
  uint32
    res = DNX_SAND_OK;
  int
    core;

  DNXC_INIT_FUNC_DEFS
  /* DNX_SAND_CHECK_DRIVER_AND_DEVICE; */;

  /* DNX_SAND_TAKE_DEVICE_SEMAPHORE; */
  SOC_DNX_CORES_ITER(SOC_CORE_ALL, core) {
      res = jer2_arad_ofp_rates_egq_tcg_rate_hw_set_unsafe(unit, core);
  }

  DNXC_IF_ERR_EXIT(res);


  /* DNX_SAND_GIVE_DEVICE_SEMAPHORE; */
exit:
  DNXC_FUNC_RETURN
}

uint32
    jer2_arad_ofp_rates_sch_tcg_rate_set(
    DNX_SAND_IN  int                unit,
    DNX_SAND_IN  int                core,
    DNX_SAND_IN  uint32             tm_port,   
    DNX_SAND_IN  uint32             tcg_ndx, 
    DNX_SAND_IN  uint32             rate
    )
{
  uint32
    res = DNX_SAND_OK;

  DNXC_INIT_FUNC_DEFS
  /* DNX_SAND_CHECK_DRIVER_AND_DEVICE; */;

  res = jer2_arad_ofp_rates_tcg_shaper_verify(unit,
                                         core,
                                         tm_port,
                                         tcg_ndx,
                                         rate,
                                         JER2_ARAD_OFP_RATES_GENERIC_FUNC_STATE_TCG_SCH_RATE);
  DNXC_IF_ERR_EXIT(res);

  /* DNX_SAND_TAKE_DEVICE_SEMAPHORE; */

  res = jer2_arad_ofp_rates_sch_tcg_rate_set_unsafe(unit,
                                               core, 
                                               tm_port,  
                                               tcg_ndx,   
                                               rate);

  DNXC_IF_ERR_EXIT(res);


  /* DNX_SAND_GIVE_DEVICE_SEMAPHORE; */
exit:
  DNXC_FUNC_RETURN
}

uint32
    jer2_arad_ofp_rates_egq_port_priority_max_burst_set(
    DNX_SAND_IN  int                   unit,
    DNX_SAND_IN  int                   core,
    DNX_SAND_IN  uint32                tm_port,    
    DNX_SAND_IN  uint32                port_priority_ndx, 
    DNX_SAND_IN  uint32                max_burst
    )
{
  uint32
    res = DNX_SAND_OK;

  DNXC_INIT_FUNC_DEFS
  /* DNX_SAND_CHECK_DRIVER_AND_DEVICE; */;

  res = jer2_arad_ofp_rates_port_priority_shaper_verify(
                                         unit,
                                         core,
                                         tm_port,
                                         port_priority_ndx,
                                         max_burst,
                                         JER2_ARAD_OFP_RATES_GENERIC_FUNC_STATE_PTC_EGQ_BURST);
  DNXC_IF_ERR_EXIT(res);

  /* DNX_SAND_TAKE_DEVICE_SEMAPHORE; */

  res = jer2_arad_ofp_rates_egq_port_priority_max_burst_set_unsafe(
                                                    unit,
                                                    core, 
                                                    tm_port,  
                                                    port_priority_ndx,   
                                                    max_burst);

  DNXC_IF_ERR_EXIT(res);


  /* DNX_SAND_GIVE_DEVICE_SEMAPHORE; */
exit:
  DNXC_FUNC_RETURN
}

uint32
    jer2_arad_ofp_rates_sch_port_priority_max_burst_set(
    DNX_SAND_IN  int                   unit,
    DNX_SAND_IN  int                   core,
    DNX_SAND_IN  uint32                tm_port,   
    DNX_SAND_IN  uint32                port_priority_ndx, 
    DNX_SAND_IN  uint32                max_burst
    )
{
  uint32
    res = DNX_SAND_OK;

  DNXC_INIT_FUNC_DEFS
  /* DNX_SAND_CHECK_DRIVER_AND_DEVICE; */;

  res = jer2_arad_ofp_rates_port_priority_shaper_verify(unit,
                                         core,
                                         tm_port,
                                         port_priority_ndx,
                                         max_burst,
                                         JER2_ARAD_OFP_RATES_GENERIC_FUNC_STATE_PTC_SCH_BURST);
  DNXC_IF_ERR_EXIT(res);

  /* DNX_SAND_TAKE_DEVICE_SEMAPHORE; */

  res = jer2_arad_ofp_rates_sch_port_priority_max_burst_set_unsafe(unit,
                                                    core,           
                                                    tm_port,  
                                                    port_priority_ndx,   
                                                    max_burst);

  DNXC_IF_ERR_EXIT(res);


  /* DNX_SAND_GIVE_DEVICE_SEMAPHORE; */
exit:
  DNXC_FUNC_RETURN
}

uint32
    jer2_arad_ofp_rates_egq_port_priority_rate_sw_set(
    DNX_SAND_IN  int                   unit,
    DNX_SAND_IN  int                   core,
    DNX_SAND_IN  uint32                tm_port,   
    DNX_SAND_IN  uint32                port_priority_ndx, 
    DNX_SAND_IN  uint32                rate
    )
{
  uint32
    res = DNX_SAND_OK;

  DNXC_INIT_FUNC_DEFS
  /* DNX_SAND_CHECK_DRIVER_AND_DEVICE; */;

  res = jer2_arad_ofp_rates_port_priority_shaper_verify(unit,
                                         core,
                                         tm_port,
                                         port_priority_ndx,
                                         rate,
                                         JER2_ARAD_OFP_RATES_GENERIC_FUNC_STATE_PTC_EGQ_RATE);
  DNXC_IF_ERR_EXIT(res);

  /* DNX_SAND_TAKE_DEVICE_SEMAPHORE; */

  res = jer2_arad_ofp_rates_egq_port_priority_rate_sw_set_unsafe(unit,
                                                            core, 
                                                            tm_port,  
                                                            port_priority_ndx,   
                                                            rate);

  DNXC_IF_ERR_EXIT(res);


  /* DNX_SAND_GIVE_DEVICE_SEMAPHORE; */
exit:
  DNXC_FUNC_RETURN
}


uint32
    jer2_arad_ofp_rates_egq_port_priority_rate_hw_set(
    DNX_SAND_IN  int                   unit
    )
{
  uint32
    res = DNX_SAND_OK;
  int core;

  DNXC_INIT_FUNC_DEFS
  /* DNX_SAND_CHECK_DRIVER_AND_DEVICE; */;

  /* DNX_SAND_TAKE_DEVICE_SEMAPHORE; */
  SOC_DNX_CORES_ITER(SOC_CORE_ALL, core) {
      res = jer2_arad_ofp_rates_egq_port_priority_rate_hw_set_unsafe(unit, core);
      DNXC_IF_ERR_EXIT(res);
  }


  /* DNX_SAND_GIVE_DEVICE_SEMAPHORE; */
exit:
  DNXC_FUNC_RETURN
}


uint32
    jer2_arad_ofp_rates_sch_port_priority_rate_set(
    DNX_SAND_IN  int                unit,
    DNX_SAND_IN  int                core,
    DNX_SAND_IN  uint32             tm_port,   
    DNX_SAND_IN  uint32             port_priority_ndx, 
    DNX_SAND_IN  uint32             rate
    )
{
  uint32
    res = DNX_SAND_OK;

  DNXC_INIT_FUNC_DEFS
  /* DNX_SAND_CHECK_DRIVER_AND_DEVICE; */;

  res = jer2_arad_ofp_rates_port_priority_shaper_verify(unit,
                                         core,
                                         tm_port,
                                         port_priority_ndx,
                                         rate,
                                         JER2_ARAD_OFP_RATES_GENERIC_FUNC_STATE_PTC_SCH_RATE);
  DNXC_IF_ERR_EXIT(res);

  /* DNX_SAND_TAKE_DEVICE_SEMAPHORE; */

  res = jer2_arad_ofp_rates_sch_port_priority_rate_set_unsafe(unit, 
                                                         core,
                                                         tm_port,  
                                                         port_priority_ndx,   
                                                         rate);

  DNXC_IF_ERR_EXIT(res);


  /* DNX_SAND_GIVE_DEVICE_SEMAPHORE; */
exit:
  DNXC_FUNC_RETURN
}

uint32
    jer2_arad_ofp_rates_sch_port_priority_rate_sw_set(
    DNX_SAND_IN  int                unit,
    DNX_SAND_IN  int                core,
    DNX_SAND_IN  uint32             tm_port,   
    DNX_SAND_IN  uint32             port_priority_ndx, 
    DNX_SAND_IN  uint32             rate
    )
{
  uint32
    res = DNX_SAND_OK;

  DNXC_INIT_FUNC_DEFS
  /* DNX_SAND_CHECK_DRIVER_AND_DEVICE; */;

  res = jer2_arad_ofp_rates_port_priority_shaper_verify(unit,
                                         core,
                                         tm_port,
                                         port_priority_ndx,
                                         rate,
                                         JER2_ARAD_OFP_RATES_GENERIC_FUNC_STATE_PTC_SCH_RATE);
  DNXC_IF_ERR_EXIT(res);

  /* DNX_SAND_TAKE_DEVICE_SEMAPHORE; */

  res = jer2_arad_ofp_rates_sch_port_priority_rate_sw_set_unsafe(unit, 
                                                         core,
                                                         tm_port,  
                                                         port_priority_ndx,   
                                                         rate);

  DNXC_IF_ERR_EXIT(res);


  /* DNX_SAND_GIVE_DEVICE_SEMAPHORE; */
exit:
  DNXC_FUNC_RETURN
}

uint32
    jer2_arad_ofp_rates_sch_port_priority_hw_set(
    DNX_SAND_IN  int                unit,
    DNX_SAND_IN  int                core
    )
{
  uint32
    res = DNX_SAND_OK;
  DNXC_INIT_FUNC_DEFS;


  res = jer2_arad_sch_port_priority_shaper_hw_set_unsafe(unit, core);
  DNXC_IF_ERR_EXIT(res);

exit:
  DNXC_FUNC_RETURN
}




uint32
    jer2_arad_ofp_rates_egq_tcg_max_burst_get(
    DNX_SAND_IN  int                unit,
    DNX_SAND_IN  int                core,
    DNX_SAND_IN  uint32             tm_port,  
    DNX_SAND_IN  uint32             tcg_ndx, 
    DNX_SAND_OUT  uint32            *max_burst
    )
{
  uint32
    res = DNX_SAND_OK;

  DNXC_INIT_FUNC_DEFS
  /* DNX_SAND_CHECK_DRIVER_AND_DEVICE; */;

  DNXC_NULL_CHECK(max_burst);

  /* DNX_SAND_TAKE_DEVICE_SEMAPHORE; */

  res = jer2_arad_ofp_rates_egq_tcg_max_burst_get_unsafe(unit,
                                                    core, 
                                                    tm_port,  
                                                    tcg_ndx,   
                                                    max_burst);

  DNXC_IF_ERR_EXIT(res);


  /* DNX_SAND_GIVE_DEVICE_SEMAPHORE; */
exit:
  DNXC_FUNC_RETURN
}

uint32
    jer2_arad_ofp_rates_sch_tcg_max_burst_get(
    DNX_SAND_IN  int                unit,
    DNX_SAND_IN  int                core,
    DNX_SAND_IN  uint32             tm_port,   
    DNX_SAND_IN  uint32             tcg_ndx, 
    DNX_SAND_OUT uint32             *max_burst
    )
{
  uint32
    res = DNX_SAND_OK;

  DNXC_INIT_FUNC_DEFS
  /* DNX_SAND_CHECK_DRIVER_AND_DEVICE; */;

  DNXC_NULL_CHECK(max_burst);

  /* DNX_SAND_TAKE_DEVICE_SEMAPHORE; */

  res = jer2_arad_ofp_rates_sch_tcg_max_burst_get_unsafe(unit,
                                                    core, 
                                                    tm_port,  
                                                    tcg_ndx,   
                                                    max_burst);

  DNXC_IF_ERR_EXIT(res);


  /* DNX_SAND_GIVE_DEVICE_SEMAPHORE; */
exit:
  DNXC_FUNC_RETURN
}

uint32
    jer2_arad_ofp_rates_egq_tcg_rate_hw_get(
    DNX_SAND_IN  int                unit,
    DNX_SAND_IN  int                core,
    DNX_SAND_IN  uint32             tm_port,   
    DNX_SAND_IN  uint32             tcg_ndx, 
    DNX_SAND_OUT uint32             *rate
    )
{
  uint32
    res = DNX_SAND_OK;

  DNXC_INIT_FUNC_DEFS
  /* DNX_SAND_CHECK_DRIVER_AND_DEVICE; */;

  DNXC_NULL_CHECK(rate);

  /* DNX_SAND_TAKE_DEVICE_SEMAPHORE; */

  res = jer2_arad_ofp_rates_egq_tcg_rate_hw_get_unsafe(unit,
                                                  core, 
                                                  tm_port,  
                                                  tcg_ndx,   
                                                  rate);

  DNXC_IF_ERR_EXIT(res);


  /* DNX_SAND_GIVE_DEVICE_SEMAPHORE; */
exit:
  DNXC_FUNC_RETURN
}

uint32
    jer2_arad_ofp_rates_sch_tcg_rate_get(
    DNX_SAND_IN  int                unit,
    DNX_SAND_IN  int                core,
    DNX_SAND_IN  uint32             tm_port,   
    DNX_SAND_IN  uint32             tcg_ndx, 
    DNX_SAND_OUT uint32             *rate
    )
{
  uint32
    res = DNX_SAND_OK;

  DNXC_INIT_FUNC_DEFS
  /* DNX_SAND_CHECK_DRIVER_AND_DEVICE; */;

  DNXC_NULL_CHECK(rate);

  DNXC_IF_ERR_EXIT(res);

  /* DNX_SAND_TAKE_DEVICE_SEMAPHORE; */

  res = jer2_arad_ofp_rates_sch_tcg_rate_get_unsafe(unit,
                                               core,
                                               tm_port,  
                                               tcg_ndx,   
                                               rate);

  DNXC_IF_ERR_EXIT(res);


  /* DNX_SAND_GIVE_DEVICE_SEMAPHORE; */
exit:
  DNXC_FUNC_RETURN
}

uint32
    jer2_arad_ofp_rates_egq_port_priority_max_burst_get(
    DNX_SAND_IN  int                unit,
    DNX_SAND_IN  int                core,
    DNX_SAND_IN  uint32             tm_port,   
    DNX_SAND_IN  uint32             port_priority_ndx, 
    DNX_SAND_OUT  uint32            *max_burst
    )
{
  uint32
    res = DNX_SAND_OK;

  DNXC_INIT_FUNC_DEFS
  /* DNX_SAND_CHECK_DRIVER_AND_DEVICE; */;

  DNXC_NULL_CHECK(max_burst);

  /* DNX_SAND_TAKE_DEVICE_SEMAPHORE; */

  res = jer2_arad_ofp_rates_egq_port_priority_max_burst_get_unsafe(unit,
                                                    core,
                                                    tm_port,  
                                                    port_priority_ndx,   
                                                    max_burst);

  DNXC_IF_ERR_EXIT(res);


  /* DNX_SAND_GIVE_DEVICE_SEMAPHORE; */
exit:
  DNXC_FUNC_RETURN
}



uint32
    jer2_arad_ofp_rates_sch_port_priority_max_burst_get(
    DNX_SAND_IN  int                unit,
    DNX_SAND_IN  int                core,
    DNX_SAND_IN  uint32             tm_port,   
    DNX_SAND_IN  uint32             port_priority_ndx, 
    DNX_SAND_OUT uint32             *max_burst
    )
{
  uint32
    res = DNX_SAND_OK;

  DNXC_INIT_FUNC_DEFS
  /* DNX_SAND_CHECK_DRIVER_AND_DEVICE; */;
  DNXC_NULL_CHECK(max_burst);
  
  /* DNX_SAND_TAKE_DEVICE_SEMAPHORE; */

  res = jer2_arad_ofp_rates_sch_port_priority_max_burst_get_unsafe(unit,
                                                    core, 
                                                    tm_port,  
                                                    port_priority_ndx,   
                                                    max_burst);

  DNXC_IF_ERR_EXIT(res);


  /* DNX_SAND_GIVE_DEVICE_SEMAPHORE; */
exit:
  DNXC_FUNC_RETURN
}

uint32
    jer2_arad_ofp_rates_egq_port_priority_rate_hw_get(
    DNX_SAND_IN  int                unit,
    DNX_SAND_IN  int                core,
    DNX_SAND_IN  uint32             tm_port,   
    DNX_SAND_IN  uint32             port_priority_ndx, 
    DNX_SAND_OUT uint32             *rate
    )
{
  uint32
    res = DNX_SAND_OK;

  DNXC_INIT_FUNC_DEFS
  /* DNX_SAND_CHECK_DRIVER_AND_DEVICE; */;

  DNXC_NULL_CHECK(rate);

  /* DNX_SAND_TAKE_DEVICE_SEMAPHORE; */

  res = jer2_arad_ofp_rates_egq_port_priority_rate_hw_get_unsafe(unit,
                                                    core, 
                                                    tm_port,  
                                                    port_priority_ndx,   
                                                    rate);

  DNXC_IF_ERR_EXIT(res);


  /* DNX_SAND_GIVE_DEVICE_SEMAPHORE; */
exit:
  DNXC_FUNC_RETURN
}



uint32
    jer2_arad_ofp_rates_sch_port_priority_rate_get(
    DNX_SAND_IN  int                unit,
    DNX_SAND_IN  int                core,
    DNX_SAND_IN  uint32             tm_port,   
    DNX_SAND_IN  uint32             port_priority_ndx, 
    DNX_SAND_OUT uint32             *rate
    )
{
  uint32
    res = DNX_SAND_OK;

  DNXC_INIT_FUNC_DEFS
  /* DNX_SAND_CHECK_DRIVER_AND_DEVICE; */;
  DNXC_NULL_CHECK(rate);

  /* DNX_SAND_TAKE_DEVICE_SEMAPHORE; */

  res = jer2_arad_ofp_rates_sch_port_priority_rate_get_unsafe(unit, 
                                                         core,
                                                         tm_port,  
                                                         port_priority_ndx,   
                                                         rate);

  DNXC_IF_ERR_EXIT(res);


  /* DNX_SAND_GIVE_DEVICE_SEMAPHORE; */
exit:
  DNXC_FUNC_RETURN
}


void
  jer2_arad_JER2_ARAD_OFP_RATES_INTERFACE_SHPR_INFO_clear(
    DNX_SAND_OUT JER2_ARAD_OFP_RATES_INTERFACE_SHPR_INFO *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  DNX_TMC_OFP_RATES_INTERFACE_SHPR_INFO_clear(info);
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  jer2_arad_JER2_ARAD_OFP_RATE_INFO_clear(
    DNX_SAND_OUT JER2_ARAD_OFP_RATE_INFO *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  DNX_TMC_OFP_RATE_INFO_clear(info);
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

#if JER2_ARAD_DEBUG_IS_LVL1

const char*
  jer2_arad_JER2_ARAD_OFP_RATES_CAL_SET_to_string(
    DNX_SAND_IN  JER2_ARAD_OFP_RATES_CAL_SET enum_val
  )
{
  return DNX_TMC_OFP_RATES_CAL_SET_to_string(enum_val);
}

const char*
  jer2_arad_JER2_ARAD_OFP_SHPR_UPDATE_MODE_to_string(
    DNX_SAND_IN  JER2_ARAD_OFP_SHPR_UPDATE_MODE enum_val
  )
{
  return DNX_TMC_OFP_SHPR_UPDATE_MODE_to_string(enum_val);
}

void
  jer2_arad_JER2_ARAD_OFP_RATES_INTERFACE_SHPR_INFO_print(
    DNX_SAND_IN  JER2_ARAD_OFP_RATES_INTERFACE_SHPR_INFO *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  DNX_TMC_OFP_RATES_INTERFACE_SHPR_INFO_print(info);
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  jer2_arad_JER2_ARAD_OFP_RATE_INFO_print(
    DNX_SAND_IN  JER2_ARAD_OFP_RATE_INFO *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  DNX_TMC_OFP_RATE_INFO_print(info);
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

#endif /* JER2_ARAD_DEBUG_IS_LVL1 */

/* } */

#include <soc/dnx/legacy/SAND/Utils/sand_footer.h>


#endif /* of #if defined(BCM_88690_A0) */

