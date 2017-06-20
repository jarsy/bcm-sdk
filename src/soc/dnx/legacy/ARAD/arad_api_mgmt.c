#include <soc/mcm/memregs.h>
#if defined(BCM_88690_A0)
/* $Id: jer2_arad_api_mgmt.c,v 1.46 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/

#ifdef _ERR_MSG_MODULE_NAME
  #error "_ERR_MSG_MODULE_NAME redefined"
#endif

#define _ERR_MSG_MODULE_NAME BSL_SOC_MANAGEMENT
/*************
 * INCLUDES  *
 *************/
/* { */
#include <shared/bsl.h>
#include <soc/dnxc/legacy/error.h>
#include <soc/dnx/legacy/dnx_config_defs.h>
#include <soc/dnx/legacy/SAND/Utils/sand_header.h>

#include <soc/dnx/legacy/ARAD/arad_api_mgmt.h>
#include <soc/dnx/legacy/ARAD/arad_mgmt.h>
#include <soc/dnx/legacy/ARAD/arad_init.h>
#include <soc/dnx/legacy/ARAD/arad_api_framework.h>
#include <soc/dnx/legacy/ARAD/arad_general.h>

#include <soc/dnx/legacy/SAND/Management/sand_general_macros.h>
#include <soc/dnx/legacy/SAND/Management/sand_error_code.h>
#include <soc/dnx/legacy/SAND/Utils/sand_os_interface.h>
#include <soc/dnx/legacy/SAND/Management/sand_chip_descriptors.h>
#include <soc/dnx/legacy/drv.h>
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

/*********************************************************************
*     This procedure registers a new device to be taken care
*     of by this device driver. Physical device must be
*     accessible by CPU when this call is made..
*     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  jer2_arad_register_device(
             uint32                  *base_address,
    DNX_SAND_IN  DNX_SAND_RESET_DEVICE_FUNC_PTR reset_device_ptr,
    DNX_SAND_INOUT int                 *unit_ptr
  )
{
  uint32
    res;

  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(JER2_ARAD_REGISTER_DEVICE);

  DNX_SAND_CHECK_NULL_INPUT(unit_ptr);

  res = jer2_arad_register_device_unsafe(
          base_address,
          reset_device_ptr,
          unit_ptr
        );
  DNX_SAND_CHECK_FUNC_RESULT(res, 10, exit);

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_register_device()",0,0);
}

/*********************************************************************
*     Undo jer2_arad_register_device()
*     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  jer2_arad_unregister_device(
    DNX_SAND_IN  int                 unit
  )
{
  uint32
    res;

  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_UNREGISTER_DEVICE);
  DNX_SAND_CHECK_DRIVER_AND_DEVICE;

  res = jer2_arad_unregister_device_unsafe(
    unit
  );
  DNX_SAND_CHECK_FUNC_RESULT(res, 100, exit);

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_unregister_device()",0,0);
}

/*
 * Arad+ only: map the module (fap_id) to the given credit value (local, remote or non mapped).
 * Has no special handling of the local device (should not be used for the local device).
 */
uint32
  jer2_arad_plus_mgmt_module_to_credit_worth_map_set(
    DNX_SAND_IN  int    unit,
    DNX_SAND_IN  uint32    fap_id,
    DNX_SAND_IN  uint32    credit_value_type /* should be one of JER2_ARAD_PLUS_FAP_CREDIT_VALUE_* */
  )
{
    DNX_SAND_INIT_ERROR_DEFINITIONS(0);
    DNX_SAND_CHECK_FUNC_RESULT(jer2_arad_mgmt_system_fap_id_verify(unit, fap_id), 10, exit);
    if (!SOC_IS_ARADPLUS(unit)) {
        DNX_SAND_SET_ERROR_CODE(DNX_SAND_GEN_ERR, 20, exit);
    }
    if (credit_value_type > DNX_TMC_FAP_CREDIT_VALUE_MAX) {
        DNX_SAND_SET_ERROR_CODE(DNX_SAND_GEN_ERR, 30, exit);
    }
    DNX_SAND_CHECK_DRIVER_AND_DEVICE;
    DNX_SAND_TAKE_DEVICE_SEMAPHORE;
    DNX_SAND_CHECK_FUNC_RESULT(jer2_arad_plus_mgmt_module_to_credit_worth_map_set_unsafe(unit, fap_id, credit_value_type), 100, exit_semaphore);
exit_semaphore:
    DNX_SAND_GIVE_DEVICE_SEMAPHORE;
exit:
    DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_plus_mgmt_module_to_credit_worth_map_set()", unit, fap_id);
}
/*
 * Arad+ only: Get the mapping the module (fap_id) to the given credit value (local, remote or non mapped).
 * Has no special handling of the local device (should not be used for the local device).
 */
uint32
  jer2_arad_plus_mgmt_module_to_credit_worth_map_get(
    DNX_SAND_IN  int    unit,
    DNX_SAND_IN  uint32    fap_id,
    DNX_SAND_OUT uint32    *credit_value_type /* will be one of JER2_ARAD_PLUS_FAP_CREDIT_VALUE_* */
  )
{
    DNX_SAND_INIT_ERROR_DEFINITIONS(0);
    DNX_SAND_CHECK_NULL_INPUT(credit_value_type);
    DNX_SAND_CHECK_FUNC_RESULT(jer2_arad_mgmt_system_fap_id_verify(unit, fap_id), 10, exit);
    if (!SOC_IS_ARADPLUS(unit)) {
        DNX_SAND_SET_ERROR_CODE(DNX_SAND_GEN_ERR, 20, exit);
    }
    DNX_SAND_CHECK_DRIVER_AND_DEVICE;
    DNX_SAND_TAKE_DEVICE_SEMAPHORE;
    DNX_SAND_CHECK_FUNC_RESULT(jer2_arad_plus_mgmt_module_to_credit_worth_map_get_unsafe(unit, fap_id, credit_value_type), 100, exit_semaphore);
exit_semaphore:
    DNX_SAND_GIVE_DEVICE_SEMAPHORE;
exit:
    DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_plus_mgmt_module_to_credit_worth_map_get()", unit, fap_id);
}
/*********************************************************************
*     Initialize the device, including:1. Prevent all the
*     control cells. 2. Initialize the device tables and
*     registers to default values. 3. Initialize
*     board-specific hardware interfaces according to
*     configurable information, as passed in 'hw_adjust'. 4.
*     Perform basic device initialization. The configuration
*     can be enabled/disabled as passed in 'enable_info'.
*     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  jer2_arad_mgmt_init_sequence_phase1(
    DNX_SAND_IN     int                 unit,
    DNX_SAND_IN     JER2_ARAD_MGMT_INIT           *init,
    DNX_SAND_IN     uint8                 silent
  )
{
  uint32
    res;

  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_MGMT_INIT_SEQUENCE_PHASE1);
  DNX_SAND_CHECK_DRIVER_AND_DEVICE;

  DNX_SAND_CHECK_NULL_INPUT(init);

  res = jer2_arad_mgmt_init_sequence_phase1_verify(unit, init);
  DNX_SAND_CHECK_FUNC_RESULT(res, 100, exit);

  DNX_SAND_TAKE_DEVICE_SEMAPHORE;

  res = jer2_arad_mgmt_init_sequence_phase1_unsafe(
    unit,
    init,
    silent
  );
  DNX_SAND_CHECK_FUNC_RESULT(res, 110, exit_semaphore);

exit_semaphore:
  DNX_SAND_GIVE_DEVICE_SEMAPHORE;
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_mgmt_init_sequence_phase1()",0,0);
}

/*********************************************************************
*     Out-of-reset sequence. Enable/Disable the device from
*     receiving and transmitting control cells.
*     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  jer2_arad_mgmt_init_sequence_phase2(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  uint8                 silent
  )
{
  uint32
    res;

  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_MGMT_INIT_SEQUENCE_PHASE2);

  if (!SOC_DNX_CONTROL(unit)->is_modid_set_called) {

      DNX_SAND_CHECK_DRIVER_AND_DEVICE;

      DNX_SAND_TAKE_DEVICE_SEMAPHORE;

      res = jer2_arad_mgmt_init_sequence_phase2_unsafe(
        unit,
        silent
      );
      DNX_SAND_CHECK_FUNC_RESULT(res, 100, exit_semaphore);
      SOC_DNX_CONTROL(unit)->is_modid_set_called = 1;

    exit_semaphore:
      DNX_SAND_GIVE_DEVICE_SEMAPHORE;
  }

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_mgmt_init_sequence_phase2()",0,0);
}

/*********************************************************************
*     Set the fabric system ID of the device. Must be unique
*     in the system.
*     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  jer2_arad_mgmt_system_fap_id_set(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  uint32                 sys_fap_id
  )
{
  uint32
    res;

  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_MGMT_SYSTEM_FAP_ID_SET);
 DNX_SAND_CHECK_DRIVER_AND_DEVICE;

  res = jer2_arad_mgmt_system_fap_id_verify(
    unit,
    sys_fap_id
  );
  DNX_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  DNX_SAND_TAKE_DEVICE_SEMAPHORE;

  res = jer2_arad_mgmt_system_fap_id_set_unsafe(
    unit,
    sys_fap_id
  );
  DNX_SAND_CHECK_FUNC_RESULT(res, 100, exit_semaphore);

exit_semaphore:
 DNX_SAND_GIVE_DEVICE_SEMAPHORE;
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_mgmt_system_fap_id_set()",0,0);
}


/*********************************************************************
*     Set the device TM-Domain. Must be unique
*     in a stackable system.
*     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  jer2_arad_mgmt_tm_domain_set(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  uint32                 tm_domain
  )
{
  uint32
    res;

  DNX_SAND_INIT_ERROR_DEFINITIONS(0);
 DNX_SAND_CHECK_DRIVER_AND_DEVICE;

  res = jer2_arad_mgmt_tm_domain_verify(
    unit,
    tm_domain
  );
  DNX_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  DNX_SAND_TAKE_DEVICE_SEMAPHORE;

  res = jer2_arad_mgmt_tm_domain_set_unsafe(
    unit,
    tm_domain
  );
  DNX_SAND_CHECK_FUNC_RESULT(res, 100, exit_semaphore);

exit_semaphore:
 DNX_SAND_GIVE_DEVICE_SEMAPHORE;
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_mgmt_tm_domain_set()",0,0);
}

/*********************************************************************
*     Get the device TM-Domain. Must be unique
*     in a stackable system.
*     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  jer2_arad_mgmt_tm_domain_get(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_OUT uint32                 *tm_domain
  )
{
  uint32
    res;

  DNX_SAND_INIT_ERROR_DEFINITIONS(0);
 DNX_SAND_CHECK_DRIVER_AND_DEVICE;

  DNX_SAND_CHECK_NULL_INPUT(tm_domain);

  DNX_SAND_TAKE_DEVICE_SEMAPHORE;

  res = jer2_arad_mgmt_tm_domain_get_unsafe(
    unit,
    tm_domain
  );
  DNX_SAND_CHECK_FUNC_RESULT(res, 100, exit_semaphore);

exit_semaphore:
 DNX_SAND_GIVE_DEVICE_SEMAPHORE;
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_mgmt_tm_domain_get()",0,0);
}  

/*********************************************************************
*     Enable / Disable the device from receiving and
*     transmitting control cells.
*     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  jer2_arad_mgmt_all_ctrl_cells_enable_set(
    DNX_SAND_IN  int  unit,
    DNX_SAND_IN  uint8  enable
  )
{
  uint32
    res;

  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_MGMT_ALL_CTRL_CELLS_ENABLE_SET);
  DNX_SAND_CHECK_DRIVER_AND_DEVICE;
  DNX_SAND_PCID_LITE_SKIP(unit);

  res = jer2_arad_mgmt_all_ctrl_cells_enable_verify(
    unit,
    enable
  );
  DNX_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  DNX_SAND_TAKE_DEVICE_SEMAPHORE;
  if (!soc_feature(unit, soc_feature_no_fabric)) {
      res = jer2_arad_mgmt_all_ctrl_cells_enable_set_unsafe(
        unit,
        enable,
        JER2_ARAD_MGMT_ALL_CTRL_CELLS_FLAGS_NONE
      );
  }
  DNX_SAND_CHECK_FUNC_RESULT(res, 100, exit_semaphore);

exit_semaphore:
  DNX_SAND_GIVE_DEVICE_SEMAPHORE;
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_mgmt_all_ctrl_cells_enable_set()",0,0);
}

/*********************************************************************
*     Enable / Disable the forcing of (bypass) TDM cells to fabric
*********************************************************************/
uint32
  jer2_arad_force_tdm_bypass_traffic_to_fabric_set(
    DNX_SAND_IN  int     unit,
    DNX_SAND_IN  int     enable
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS(0);
  DNX_SAND_CHECK_DRIVER_AND_DEVICE;

  DNX_SAND_TAKE_DEVICE_SEMAPHORE;
  DNX_SAND_CHECK_FUNC_RESULT(jer2_arad_force_tdm_bypass_traffic_to_fabric_set_unsafe(unit, enable) , 100, exit_semaphore);

exit_semaphore:
  DNX_SAND_GIVE_DEVICE_SEMAPHORE;
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_force_tdm_bypass_traffic_to_fabric_set()", enable, 0);
}

/*********************************************************************
*     Check if forcing of (bypass) TDM cells to fabric
*********************************************************************/
uint32
  jer2_arad_force_tdm_bypass_traffic_to_fabric_get(
    DNX_SAND_IN  int     unit,
    DNX_SAND_OUT int     *enable
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS(0);
  DNX_SAND_CHECK_DRIVER_AND_DEVICE;
  DNX_SAND_CHECK_NULL_INPUT(enable);

  DNX_SAND_TAKE_DEVICE_SEMAPHORE;
  DNX_SAND_CHECK_FUNC_RESULT(jer2_arad_force_tdm_bypass_traffic_to_fabric_get_unsafe(unit, enable) , 100, exit_semaphore);

exit_semaphore:
  DNX_SAND_GIVE_DEVICE_SEMAPHORE;
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_force_tdm_bypass_traffic_to_fabric_get()", 0, 0);
}

/*********************************************************************
*     Enable / Disable the device from receiving and
*     transmitting control cells.
*     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  jer2_arad_mgmt_all_ctrl_cells_enable_get(
    DNX_SAND_IN  int  unit,
    DNX_SAND_OUT uint8  *enable
  )
{
  uint32
    res;

  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_MGMT_ALL_CTRL_CELLS_ENABLE_GET);
  DNX_SAND_CHECK_DRIVER_AND_DEVICE;

  DNX_SAND_TAKE_DEVICE_SEMAPHORE;

  res = jer2_arad_mgmt_all_ctrl_cells_enable_get_unsafe(
    unit,
    enable
  );
  DNX_SAND_CHECK_FUNC_RESULT(res, 100, exit_semaphore);

exit_semaphore:
  DNX_SAND_GIVE_DEVICE_SEMAPHORE;
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_mgmt_all_ctrl_cells_enable_get()",0,0);
}


/*********************************************************************
*     Enable / Disable the device from receiving and
*     transmitting traffic.
*     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  jer2_arad_mgmt_enable_traffic_get(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_OUT uint8                 *enable
  )
{
  uint32
    res;

  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_MGMT_ENABLE_TRAFFIC_GET);
  DNX_SAND_CHECK_DRIVER_AND_DEVICE;

  DNX_SAND_CHECK_NULL_INPUT(enable);

  DNX_SAND_TAKE_DEVICE_SEMAPHORE;

  res = jer2_arad_mgmt_enable_traffic_get_unsafe(
    unit,
    enable
  );
  DNX_SAND_CHECK_FUNC_RESULT(res, 100, exit_semaphore);

exit_semaphore:
  DNX_SAND_GIVE_DEVICE_SEMAPHORE;
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_mgmt_enable_traffic_get()",0,0);
}


/*********************************************************************
*     Set the maximal allowed packet size. The limitation can
 *     be performed based on the packet size before or after
 *     the ingress editing (external and internal configuration
 *     mode, accordingly). Packets above the specified value
 *     are dropped.
 *     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  jer2_arad_mgmt_max_pckt_size_set(
    DNX_SAND_IN  int                      unit,
    DNX_SAND_IN  uint32                      port_ndx,
    DNX_SAND_IN  JER2_ARAD_MGMT_PCKT_SIZE_CONF_MODE conf_mode_ndx,
    DNX_SAND_IN  uint32                       max_size
  )
{
  uint32
    res = DNX_SAND_OK;

  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_MGMT_MAX_PCKT_SIZE_SET);

  DNX_SAND_CHECK_DRIVER_AND_DEVICE;

  res = jer2_arad_mgmt_max_pckt_size_set_verify(
          unit,
          port_ndx,
          conf_mode_ndx,
          max_size
        );
  DNX_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  DNX_SAND_TAKE_DEVICE_SEMAPHORE;

  res = jer2_arad_mgmt_max_pckt_size_set_unsafe(
          unit,
          port_ndx,
          conf_mode_ndx,
          max_size
        );
  DNX_SAND_CHECK_FUNC_RESULT(res, 20, exit_semaphore);

exit_semaphore:
  DNX_SAND_GIVE_DEVICE_SEMAPHORE;
  JER2_ARAD_DO_NOTHING_AND_EXIT;
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_mgmt_max_pckt_size_set()", port_ndx, 0);
}

/*********************************************************************
*     Set the maximal allowed packet size. The limitation can
 *     be performed based on the packet size before or after
 *     the ingress editing (external and internal configuration
 *     mode, accordingly). Packets above the specified value
 *     are dropped.
 *     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  jer2_arad_mgmt_max_pckt_size_get(
    DNX_SAND_IN  int                      unit,
    DNX_SAND_IN  uint32                      port_ndx,
    DNX_SAND_IN  JER2_ARAD_MGMT_PCKT_SIZE_CONF_MODE conf_mode_ndx,
    DNX_SAND_OUT uint32                       *max_size
  )
{
  uint32
    res = DNX_SAND_OK;

  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_MGMT_MAX_PCKT_SIZE_GET);

  DNX_SAND_CHECK_DRIVER_AND_DEVICE;

  DNX_SAND_CHECK_NULL_INPUT(max_size);

  res = jer2_arad_mgmt_max_pckt_size_get_verify(
          unit,
          port_ndx,
          conf_mode_ndx
        );
  DNX_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  DNX_SAND_TAKE_DEVICE_SEMAPHORE;

  res = jer2_arad_mgmt_max_pckt_size_get_unsafe(
          unit,
          port_ndx,
          conf_mode_ndx,
          max_size
        );
  DNX_SAND_CHECK_FUNC_RESULT(res, 20, exit_semaphore);

exit_semaphore:
  DNX_SAND_GIVE_DEVICE_SEMAPHORE;
  JER2_ARAD_DO_NOTHING_AND_EXIT;
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_mgmt_max_pckt_size_get()", port_ndx, 0);
}

/*

*/
uint32
  jer2_arad_mgmt_ocb_mc_range_set(
    DNX_SAND_IN  int                      unit,
    DNX_SAND_IN  uint32                      range_ndx,
    DNX_SAND_IN  JER2_ARAD_MGMT_OCB_MC_RANGE         *range
  )
{
  uint32
    res;
  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_MGMT_OCB_MC_RANGE_SET);
  DNX_SAND_ERR_IF_ABOVE_MAX(range_ndx, 1, JER2_ARAD_MGMT_OCB_MC_RANGE_INDEX_OUT_OF_RANGE_ERR, 10, exit);
  DNX_SAND_CHECK_NULL_INPUT(range);
  if (range->min != JER2_ARAD_MGMT_OCB_MC_RANGE_DISABLE) {
      DNX_SAND_ERR_IF_ABOVE_NOF(range->min, SOC_DNX_CONFIG(unit)->tm.nof_mc_ids, JER2_ARAD_MGMT_OCB_MC_RANGE_MIN_OUT_OF_RANGE_ERR, 11, exit);
  }
  if (range->max != JER2_ARAD_MGMT_OCB_MC_RANGE_DISABLE) {
      DNX_SAND_ERR_IF_ABOVE_NOF(range->max, SOC_DNX_CONFIG(unit)->tm.nof_mc_ids, JER2_ARAD_MGMT_OCB_MC_RANGE_MAX_OUT_OF_RANGE_ERR, 12, exit);
  }

  DNX_SAND_TAKE_DEVICE_SEMAPHORE;

  res = jer2_arad_mgmt_ocb_mc_range_set_unsafe(
          unit,
          range_ndx,
          range
        );
  DNX_SAND_CHECK_FUNC_RESULT(res, 20, exit_semaphore);
   
exit_semaphore:
  DNX_SAND_GIVE_DEVICE_SEMAPHORE;
  JER2_ARAD_DO_NOTHING_AND_EXIT;
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_mgmt_ocb_mc_range_set()", 0, 0);
}

uint32
  jer2_arad_mgmt_ocb_mc_range_get(
    DNX_SAND_IN  int                      unit,
    DNX_SAND_IN  uint32                      range_ndx,
    DNX_SAND_OUT JER2_ARAD_MGMT_OCB_MC_RANGE         *range
  )
{
  uint32
    res;
  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_MGMT_OCB_MC_RANGE_GET);

  DNX_SAND_CHECK_NULL_INPUT(range);
  DNX_SAND_ERR_IF_ABOVE_MAX(range_ndx, 1, JER2_ARAD_MGMT_OCB_MC_RANGE_INDEX_OUT_OF_RANGE_ERR, 10, exit);

  DNX_SAND_TAKE_DEVICE_SEMAPHORE;
  
  res = jer2_arad_mgmt_ocb_mc_range_get_unsafe(
          unit,
          range_ndx,
          range
        );
  DNX_SAND_CHECK_FUNC_RESULT(res, 20, exit_semaphore);

exit_semaphore:
  DNX_SAND_GIVE_DEVICE_SEMAPHORE;
  JER2_ARAD_DO_NOTHING_AND_EXIT;
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_mgmt_ocb_mc_range_get()", 0, 0);
}

/*

*/
uint32
  jer2_arad_mgmt_ocb_voq_eligible_set(
    DNX_SAND_IN  int                    unit,
    DNX_SAND_IN  uint32                    q_category_ndx,
    DNX_SAND_IN  uint32                    rate_class_ndx,
    DNX_SAND_IN  uint32                    tc_ndx,
    DNX_SAND_IN  JER2_ARAD_MGMT_OCB_VOQ_INFO       *info,
    DNX_SAND_OUT JER2_ARAD_MGMT_OCB_VOQ_INFO    *exact_info
  )
{
    
    DNXC_LEGACY_FIXME_ASSERT;
    return -1;
}

uint32
  jer2_arad_mgmt_ocb_voq_eligible_get(
    DNX_SAND_IN  int                    unit,
    DNX_SAND_IN  uint32                    q_category_ndx,
    DNX_SAND_IN  uint32                    rate_class_ndx,
    DNX_SAND_IN  uint32                    tc_ndx,
    DNX_SAND_OUT JER2_ARAD_MGMT_OCB_VOQ_INFO       *info
  )
{
    
    DNXC_LEGACY_FIXME_ASSERT;
    return -1;
}


uint32
  jer2_arad_mgmt_ocb_voq_eligible_dynamic_set(
    DNX_SAND_IN  int                    unit,
    DNX_SAND_IN  uint32                    qid,
    DNX_SAND_IN  uint32                    enable
  )
{
    
    DNXC_LEGACY_FIXME_ASSERT;
    return -1;
}

void
  jer2_arad_JER2_ARAD_HW_PLL_PARAMS_clear(
    DNX_SAND_OUT JER2_ARAD_HW_PLL_PARAMS *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  dnx_sand_os_memset(info, 0x0, sizeof(JER2_ARAD_HW_PLL_PARAMS));
  info->m = 0;
  info->n = 0;
  DNX_SAND_MAGIC_NUM_SET;
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  JER2_ARAD_INIT_PLL_clear(
    DNX_SAND_OUT JER2_ARAD_INIT_PLL *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  
  DNXC_LEGACY_FIXME_ASSERT;
#ifdef FIXME_DNX_LEGACY
  JER2_ARAD_INIT_DRAM_PLL_clear(&info->dram_pll);
#endif 
  info->nif_clk_freq = JER2_ARAD_INIT_SERDES_REF_CLOCK_125;
  info->fabric_clk_freq = JER2_ARAD_INIT_SERDES_REF_CLOCK_125;
  info->synthesizer_clock_freq = JER2_ARAD_INIT_SYNTHESIZER_CLOCK_FREQUENCY_25_MHZ;

exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}
  

void
  jer2_arad_JER2_ARAD_INIT_FABRIC_clear(
    DNX_SAND_OUT JER2_ARAD_INIT_FABRIC *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  dnx_sand_os_memset(info, 0x0, sizeof(JER2_ARAD_INIT_FABRIC));
  info->enable = 0;
  info->connect_mode = JER2_ARAD_FABRIC_NOF_CONNECT_MODES;
  
  DNXC_LEGACY_FIXME_ASSERT;  
#ifdef FIXME_DNX_LEGACY
  info->ftmh_extension = JER2_ARAD_PORTS_FTMH_EXT_OUTLIF_NEVER;
#endif 

  info->ftmh_lb_ext_mode = JER2_ARAD_MGMT_FTMH_LB_EXT_MODE_DISABLED;
  info->ftmh_stacking_ext_mode = 0x0;
  info->is_fe600 = FALSE;
  info->is_128_in_system = FALSE;
  info->dual_pipe_tdm_packet = FALSE;
  DNX_SAND_MAGIC_NUM_SET;
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}


void
  jer2_arad_JER2_ARAD_INIT_CORE_FREQ_clear(
    DNX_SAND_OUT JER2_ARAD_INIT_CORE_FREQ *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  dnx_sand_os_memset(info, 0x0, sizeof(JER2_ARAD_INIT_CORE_FREQ));
  info->enable = 0;
  info->frequency = 0;
  info->system_ref_clock = 0;
  DNX_SAND_MAGIC_NUM_SET;
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  JER2_ARAD_INIT_CREDIT_clear(
    DNX_SAND_OUT JER2_ARAD_INIT_CREDIT *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  dnx_sand_os_memset(info, 0x0, sizeof(JER2_ARAD_INIT_CREDIT));
  info->credit_worth_enable = 0;
  info->credit_worth = 0;
  DNX_SAND_MAGIC_NUM_SET;
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  jer2_arad_JER2_ARAD_MGMT_INIT_clear(
    DNX_SAND_OUT JER2_ARAD_MGMT_INIT *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  dnx_sand_os_memset(info, 0x0, sizeof(JER2_ARAD_MGMT_INIT));
  jer2_arad_JER2_ARAD_INIT_FABRIC_clear(&(info->fabric));
  info->eg_cgm_scheme = JER2_ARAD_NOF_EGR_QUEUING_PARTITION_SCHEMES;
  jer2_arad_JER2_ARAD_INIT_CORE_FREQ_clear(&(info->core_freq));
  info->tdm_mode = JER2_ARAD_MGMT_NOF_TDM_MODES;
  JER2_ARAD_INIT_CREDIT_clear(&(info->credit));
  JER2_ARAD_INIT_PLL_clear(&(info->pll));
  jer2_arad_JER2_ARAD_INIT_FC_clear(&(info->fc));

  DNX_SAND_MAGIC_NUM_SET;

exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  jer2_arad_JER2_ARAD_MGMT_OCB_MC_RANGE_clear(
    DNX_SAND_OUT JER2_ARAD_MGMT_OCB_MC_RANGE *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  dnx_sand_os_memset(info, 0x0, sizeof(JER2_ARAD_MGMT_OCB_MC_RANGE));
  info->min  = 0;
  info->max = 0;

  DNX_SAND_MAGIC_NUM_SET;
  
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  jer2_arad_JER2_ARAD_MGMT_OCB_VOQ_INFO_clear(
    DNX_SAND_OUT JER2_ARAD_MGMT_OCB_VOQ_INFO *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  DNX_TMC_MGMT_OCB_VOQ_INFO_clear(info);
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  jer2_arad_JER2_ARAD_MGMT_PCKT_SIZE_clear(
    DNX_SAND_OUT JER2_ARAD_MGMT_PCKT_SIZE *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  DNX_TMC_MGMT_PCKT_SIZE_clear(info);
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}


void
  jer2_arad_JER2_ARAD_INIT_PORT_HDR_TYPE_clear(
    DNX_SAND_OUT JER2_ARAD_INIT_PORT_HDR_TYPE *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  dnx_sand_os_memset(info, 0x0, sizeof(JER2_ARAD_INIT_PORT_HDR_TYPE));
  info->port_ndx = 0;
  
  DNXC_LEGACY_FIXME_ASSERT;
#ifdef FIXME_DNX_LEGACY
  info->header_type_in = JER2_ARAD_PORT_NOF_HEADER_TYPES;
  info->header_type_out = JER2_ARAD_PORT_NOF_HEADER_TYPES;
#endif 

  info->first_header_size = 0;
  DNX_SAND_MAGIC_NUM_SET;
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  JER2_ARAD_INIT_EGR_Q_PROFILE_MAP_clear(
    DNX_SAND_OUT JER2_ARAD_INIT_EGR_Q_PROFILE_MAP *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  dnx_sand_os_memset(info, 0x0, sizeof(JER2_ARAD_INIT_EGR_Q_PROFILE_MAP));
  info->port_ndx = 0;
  info->conf = 0;
  DNX_SAND_MAGIC_NUM_SET;
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  jer2_arad_JER2_ARAD_INIT_FC_clear(
    DNX_SAND_OUT JER2_ARAD_INIT_FC *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  dnx_sand_os_memset(info, 0x0, sizeof(JER2_ARAD_INIT_FC));


  DNX_SAND_MAGIC_NUM_SET;
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

#if JER2_ARAD_DEBUG_IS_LVL1


void
  jer2_arad_JER2_ARAD_HW_PLL_PARAMS_print(
    DNX_SAND_IN JER2_ARAD_HW_PLL_PARAMS *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  LOG_CLI((BSL_META_U(unit,
                      " m:                     %u\n\r"),info->m));
  LOG_CLI((BSL_META_U(unit,
                      " n:                     %u\n\r"),info->n));
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  jer2_arad_JER2_ARAD_INIT_CORE_FREQ_print(
    DNX_SAND_IN  JER2_ARAD_INIT_CORE_FREQ *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  LOG_CLI((BSL_META_U(unit,
                      "enable: %u\n\r"),info->enable));
  LOG_CLI((BSL_META_U(unit,
                      "frequency: %u[kHz]\n\r"),info->frequency));
  LOG_CLI((BSL_META_U(unit,
                      "system_ref_clock: %u[kHz]\n\r"),info->system_ref_clock));
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  jer2_arad_JER2_ARAD_INIT_FABRIC_print(
    DNX_SAND_IN JER2_ARAD_INIT_FABRIC *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  LOG_CLI((BSL_META_U(unit,
                      " Enable:                      %u\n\r"),info->enable));
  LOG_CLI((BSL_META_U(unit,
                      " Connect_mode:                %s \n\r"),
           jer2_arad_JER2_ARAD_FABRIC_CONNECT_MODE_to_string(info->connect_mode)
           ));
           
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  jer2_arad_JER2_ARAD_MGMT_INIT_print(
    DNX_SAND_IN JER2_ARAD_MGMT_INIT *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  LOG_CLI((BSL_META_U(unit,
                      "Fabric:\n\r")));
  jer2_arad_JER2_ARAD_INIT_FABRIC_print(&(info->fabric));
  LOG_CLI((BSL_META_U(unit,
                      "Core_freq:")));
  jer2_arad_JER2_ARAD_INIT_CORE_FREQ_print(&(info->core_freq));
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}


void
  jer2_arad_JER2_ARAD_INIT_PORT_TO_IF_MAP_print(
    DNX_SAND_IN JER2_ARAD_INIT_PORT_TO_IF_MAP *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  LOG_CLI((BSL_META_U(unit,
                      " Port_ndx: %u\n\r"),info->port_ndx));
  LOG_CLI((BSL_META_U(unit,
                      " Conf:\n\r")));
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  JER2_ARAD_INIT_EGR_Q_PROFILE_MAP_print(
    DNX_SAND_IN  JER2_ARAD_INIT_EGR_Q_PROFILE_MAP *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  LOG_CLI((BSL_META_U(unit,
                      "port_ndx: %u\n\r"),info->port_ndx));
  LOG_CLI((BSL_META_U(unit,
                      "conf: %u\n\r"),info->conf));
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

#endif /* JER2_ARAD_DEBUG_IS_LVL1 */

/* } */

#include <soc/dnx/legacy/SAND/Utils/sand_footer.h>


#endif /* of #if defined(BCM_88690_A0) */
