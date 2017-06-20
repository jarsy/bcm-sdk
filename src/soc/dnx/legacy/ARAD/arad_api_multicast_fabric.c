#include <soc/mcm/memregs.h>
#if defined(BCM_88690_A0)
/* $Id: jer2_arad_api_multicast_fabric.c,v 1.8 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/


#ifdef _ERR_MSG_MODULE_NAME
  #error "_ERR_MSG_MODULE_NAME redefined"
#endif

#define _ERR_MSG_MODULE_NAME BSL_SOC_FABRIC
/*************
 * INCLUDES  *
 *************/
/* { */
#include <shared/bsl.h>
#include <soc/dnxc/legacy/error.h>
#include <soc/dnx/legacy/SAND/Utils/sand_header.h>

#include <soc/dnx/legacy/ARAD/arad_multicast_fabric.h>
#include <soc/dnx/legacy/ARAD/arad_api_general.h>

#include <soc/dnx/legacy/SAND/Management/sand_general_macros.h>
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
*     Maps the embedded traffic class in the packet header to
*     a multicast class (0..3). This multicast class will be
*     further used for egress/fabric replication.
*     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  jer2_arad_mult_fabric_traffic_class_to_multicast_cls_map_set(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  JER2_ARAD_TR_CLS         tr_cls_ndx,
    DNX_SAND_IN  JER2_ARAD_MULT_FABRIC_CLS     new_mult_cls
  )
{
  uint32  res;
  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_MULT_FABRIC_TRAFFIC_CLASS_TO_MULTICAST_CLS_MAP_SET);
  DNX_SAND_CHECK_DRIVER_AND_DEVICE;


  res = jer2_arad_mult_fabric_traffic_class_to_multicast_cls_map_verify(
    unit,
    tr_cls_ndx,
    new_mult_cls
  );
  DNX_SAND_CHECK_FUNC_RESULT(res, 10, exit);

   DNX_SAND_TAKE_DEVICE_SEMAPHORE;

  res = jer2_arad_mult_fabric_traffic_class_to_multicast_cls_map_set_unsafe(
    unit,
    tr_cls_ndx,
    new_mult_cls
  );
  DNX_SAND_CHECK_FUNC_RESULT(res, 100, exit_semaphore);

exit_semaphore:
  DNX_SAND_GIVE_DEVICE_SEMAPHORE;
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_mult_fabric_traffic_class_to_multicast_cls_map_set()",0,0);
}

/*********************************************************************
*     Maps the embedded traffic class in the packet header to
*     a multicast class (0..3). This multicast class will be
*     further used for egress/fabric replication.
*     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  jer2_arad_mult_fabric_traffic_class_to_multicast_cls_map_get(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  JER2_ARAD_TR_CLS         tr_cls_ndx,
    DNX_SAND_OUT JER2_ARAD_MULT_FABRIC_CLS     *new_mult_cls
  )
{
  uint32  res;
  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_MULT_FABRIC_TRAFFIC_CLASS_TO_MULTICAST_CLS_MAP_GET);
  DNX_SAND_CHECK_DRIVER_AND_DEVICE;

  DNX_SAND_CHECK_NULL_INPUT(new_mult_cls);

   DNX_SAND_TAKE_DEVICE_SEMAPHORE;

  res = jer2_arad_mult_fabric_traffic_class_to_multicast_cls_map_get_unsafe(
    unit,
    tr_cls_ndx,
    new_mult_cls
  );
  DNX_SAND_CHECK_FUNC_RESULT(res, 100, exit_semaphore);

exit_semaphore:
  DNX_SAND_GIVE_DEVICE_SEMAPHORE;
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_mult_fabric_traffic_class_to_multicast_cls_map_get()",0,0);
}
/*********************************************************************
*     This procedure configures the base queue of the
*     multicast egress/fabric.
*     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  jer2_arad_mult_fabric_base_queue_set(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  uint32                  queue_id
  )
{
  uint32  res;
  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_MULT_FABRIC_BASE_QUEUE_SET);
  DNX_SAND_CHECK_DRIVER_AND_DEVICE;


  res = jer2_arad_mult_fabric_base_queue_verify(
    unit,
    queue_id
  );
  DNX_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  DNX_SAND_TAKE_DEVICE_SEMAPHORE;

  res = jer2_arad_mult_fabric_base_queue_set_unsafe(
    unit,
    queue_id
  );
  DNX_SAND_CHECK_FUNC_RESULT(res, 100, exit_semaphore);

exit_semaphore:
  DNX_SAND_GIVE_DEVICE_SEMAPHORE;
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_mult_fabric_base_queue_set()",0,0);
}

/*********************************************************************
*     This procedure configures the base queue of the
*     multicast egress/fabric.
*     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  jer2_arad_mult_fabric_base_queue_get(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_OUT uint32                  *queue_id
  )
{
  uint32  res;
  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_MULT_FABRIC_BASE_QUEUE_GET);
  DNX_SAND_CHECK_DRIVER_AND_DEVICE;

  DNX_SAND_CHECK_NULL_INPUT(queue_id);

   DNX_SAND_TAKE_DEVICE_SEMAPHORE;

  res = jer2_arad_mult_fabric_base_queue_get_unsafe(
    unit,
    queue_id
  );
  DNX_SAND_CHECK_FUNC_RESULT(res, 100, exit_semaphore);

exit_semaphore:
  DNX_SAND_GIVE_DEVICE_SEMAPHORE;
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_mult_fabric_base_queue_get()",0,0);
}

/*********************************************************************
*     Configure the Enhanced Fabric Multicast Queue
*     configuration: the fabric multicast queues are defined
*     in a configured range, and the credits are coming to
*     these queues according to a scheduler scheme.
*     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  jer2_arad_mult_fabric_enhanced_set(
    DNX_SAND_IN  int                                 unit,
    DNX_SAND_IN  int                                 core_id,
    DNX_SAND_IN  DNX_SAND_U32_RANGE                  *queue_range
  )
{
  int
    res = SOC_E_NONE;
  DNXC_INIT_FUNC_DEFS;
  DNXC_NULL_CHECK(queue_range);

  res = jer2_arad_mult_fabric_enhanced_set_verify(
          unit,
          queue_range
        );
  DNXC_IF_ERR_EXIT(res);

  res = jer2_arad_mult_fabric_enhanced_set_unsafe(
          unit,
          queue_range
        );
  DNXC_IF_ERR_EXIT(res);

exit:
  DNXC_FUNC_RETURN;
}

/*********************************************************************
*     Configure the Enhanced Fabric Multicast Queue
*     configuration: the fabric multicast queues are defined
*     in a configured range, and the credits are coming to
*     these queues according to a scheduler scheme.
*     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  jer2_arad_mult_fabric_enhanced_get(
    DNX_SAND_IN  int                                 unit,
    DNX_SAND_IN  int                                 core_id,
    DNX_SAND_OUT DNX_SAND_U32_RANGE                            *queue_range
  )
{
  int
    res = SOC_E_NONE;
  DNXC_INIT_FUNC_DEFS;
  DNXC_NULL_CHECK(queue_range);

  res = jer2_arad_mult_fabric_enhanced_get_unsafe(
          unit,
          queue_range
        );
  DNXC_IF_ERR_EXIT(res);

exit:
  DNXC_FUNC_RETURN;
}

/*********************************************************************
* NAME:
*     jer2_arad_mult_fabric_active_links_set\get 
* TYPE:
*   PROC
* FUNCTION:
*   This procedure sets the FMQ with GCI LB level
* INPUT:
*   DNX_SAND_IN  int                                 unit -
*     Identifier of the device to access.
*   DNX_SAND_IN JER2_ARAD_MULT_FABRIC_FLOW_CONTROL_MAP       fc_map (set) - 
*   DNX_SAND_OUT JER2_ARAD_MULT_FABRIC_FLOW_CONTROL_MAP      fc_map (get) - 
*     See in struct description.
* REMARKS:
*   None.
* RETURNS:
*   OK or ERROR indication.
*********************************************************************/

uint32
  jer2_arad_mult_fabric_flow_control_set(
    DNX_SAND_IN  int                                 unit,
    DNX_SAND_IN  JER2_ARAD_MULT_FABRIC_FLOW_CONTROL_MAP      *fc_map
  )
{
  uint32
    res = DNX_SAND_OK;

  DNX_SAND_INIT_ERROR_DEFINITIONS(0);
  DNX_SAND_CHECK_DRIVER_AND_DEVICE;

  res = jer2_arad_mult_fabric_flow_control_set_verify(unit, fc_map);
  DNX_SAND_CHECK_FUNC_RESULT(res, 1, exit);

  DNX_SAND_TAKE_DEVICE_SEMAPHORE;

  res = jer2_arad_mult_fabric_flow_control_set_unsafe(unit, fc_map);
  DNX_SAND_CHECK_FUNC_RESULT(res, 2, exit_semaphore);

exit_semaphore:
  DNX_SAND_GIVE_DEVICE_SEMAPHORE;
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_mult_fabric_flow_control_set()",0,0);
}

uint32
  jer2_arad_mult_fabric_flow_control_get(
    DNX_SAND_IN  int                                 unit,
    DNX_SAND_OUT JER2_ARAD_MULT_FABRIC_FLOW_CONTROL_MAP     *fc_map
  )
{
  uint32
    res = DNX_SAND_OK;

  DNX_SAND_INIT_ERROR_DEFINITIONS(0);
  DNX_SAND_CHECK_DRIVER_AND_DEVICE;

  res = jer2_arad_mult_fabric_flow_control_get_verify(unit, fc_map);
  DNX_SAND_CHECK_FUNC_RESULT(res, 1, exit);

  DNX_SAND_TAKE_DEVICE_SEMAPHORE;

  res = jer2_arad_mult_fabric_flow_control_get_unsafe(unit, fc_map);
  DNX_SAND_CHECK_FUNC_RESULT(res, 2, exit_semaphore);

exit_semaphore:
  DNX_SAND_GIVE_DEVICE_SEMAPHORE;
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_mult_fabric_flow_control_get()",0,0);
}

void
  jer2_arad_JER2_ARAD_MULT_FABRIC_PORT_INFO_clear(
    DNX_SAND_OUT JER2_ARAD_MULT_FABRIC_PORT_INFO *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  DNX_TMC_MULT_FABRIC_PORT_INFO_clear(info);
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  jer2_arad_JER2_ARAD_MULT_FABRIC_SHAPER_INFO_clear(
    DNX_SAND_OUT JER2_ARAD_MULT_FABRIC_SHAPER_INFO *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  DNX_TMC_MULT_FABRIC_SHAPER_INFO_clear(info);
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  jer2_arad_JER2_ARAD_MULT_FABRIC_BE_CLASS_INFO_clear(
    DNX_SAND_OUT JER2_ARAD_MULT_FABRIC_BE_CLASS_INFO *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  DNX_TMC_MULT_FABRIC_BE_CLASS_INFO_clear(info);
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  jer2_arad_JER2_ARAD_MULT_FABRIC_BE_INFO_clear(
    DNX_SAND_OUT JER2_ARAD_MULT_FABRIC_BE_INFO *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  DNX_TMC_MULT_FABRIC_BE_INFO_clear(info);
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  jer2_arad_JER2_ARAD_MULT_FABRIC_GR_INFO_clear(
    DNX_SAND_OUT JER2_ARAD_MULT_FABRIC_GR_INFO *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  DNX_TMC_MULT_FABRIC_GR_INFO_clear(info);
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  jer2_arad_JER2_ARAD_MULT_FABRIC_INFO_clear(
    DNX_SAND_OUT JER2_ARAD_MULT_FABRIC_INFO *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  DNX_TMC_MULT_FABRIC_INFO_clear(info);
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  jer2_arad_JER2_ARAD_MULT_FABRIC_ACTIVE_LINKS_clear(
    DNX_SAND_IN uint32 unit,
    DNX_SAND_OUT JER2_ARAD_MULT_FABRIC_ACTIVE_LINKS *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  DNX_TMC_MULT_FABRIC_ACTIVE_LINKS_clear(unit, info);
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  jer2_arad_JER2_ARAD_MULT_FABRIC_FLOW_CONTROL_MAP_clear(
    DNX_SAND_OUT JER2_ARAD_MULT_FABRIC_FLOW_CONTROL_MAP *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  DNX_TMC_MULT_FABRIC_FLOW_CONTROL_MAP_clear(info);

exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

#if JER2_ARAD_DEBUG_IS_LVL1

const char*
  jer2_arad_JER2_ARAD_MULT_FABRIC_CLS_RNG_to_string(
    DNX_SAND_IN  JER2_ARAD_MULT_FABRIC_CLS_RNG enum_val
  )
{
  return DNX_TMC_MULT_FABRIC_CLS_RNG_to_string(enum_val);
}

void
  jer2_arad_JER2_ARAD_MULT_FABRIC_PORT_INFO_print(
    DNX_SAND_IN  JER2_ARAD_MULT_FABRIC_PORT_INFO *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  DNX_TMC_MULT_FABRIC_PORT_INFO_print(info);
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  jer2_arad_JER2_ARAD_MULT_FABRIC_SHAPER_INFO_print(
    DNX_SAND_IN  JER2_ARAD_MULT_FABRIC_SHAPER_INFO *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  DNX_TMC_MULT_FABRIC_SHAPER_INFO_print(info);
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  jer2_arad_JER2_ARAD_MULT_FABRIC_BE_CLASS_INFO_print(
    DNX_SAND_IN  JER2_ARAD_MULT_FABRIC_BE_CLASS_INFO *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  DNX_TMC_MULT_FABRIC_BE_CLASS_INFO_print(info);
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  jer2_arad_JER2_ARAD_MULT_FABRIC_BE_INFO_print(
    DNX_SAND_IN  JER2_ARAD_MULT_FABRIC_BE_INFO *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  DNX_TMC_MULT_FABRIC_BE_INFO_print(info);
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  jer2_arad_JER2_ARAD_MULT_FABRIC_GR_INFO_print(
    DNX_SAND_IN  JER2_ARAD_MULT_FABRIC_GR_INFO *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  DNX_TMC_MULT_FABRIC_GR_INFO_print(info);
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  jer2_arad_JER2_ARAD_MULT_FABRIC_INFO_print(
    DNX_SAND_IN  JER2_ARAD_MULT_FABRIC_INFO *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  DNX_TMC_MULT_FABRIC_INFO_print(info);
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  jer2_arad_JER2_ARAD_MULT_FABRIC_ACTIVE_LINKS_print(
    DNX_SAND_IN uint32 unit,
    DNX_SAND_IN  JER2_ARAD_MULT_FABRIC_ACTIVE_LINKS *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  DNX_TMC_MULT_FABRIC_ACTIVE_LINKS_print(unit, info);
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

#endif /* JER2_ARAD_DEBUG_IS_LVL1 */
/* } */

#include <soc/dnx/legacy/SAND/Utils/sand_footer.h>


#endif /* of #if defined(BCM_88690_A0) */
