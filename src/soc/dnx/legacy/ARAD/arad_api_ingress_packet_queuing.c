
#include <soc/mcm/memregs.h>
#if defined(BCM_88690_A0)
/* $Id: jer2_arad_api_ingress_packet_queuing.c,v 1.21 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/


#ifdef _ERR_MSG_MODULE_NAME
  #error "_ERR_MSG_MODULE_NAME redefined"
#endif

#define _ERR_MSG_MODULE_NAME BSL_SOC_INGRESS
/*************
 * INCLUDES  *
 *************/
/* { */
#include <shared/bsl.h>
#include <soc/dnxc/legacy/error.h>
#include <soc/dnx/legacy/cosq.h>
#include <soc/dnx/legacy/SAND/Utils/sand_header.h>

#include <soc/dnx/legacy/ARAD/arad_ingress_packet_queuing.h>
#include <soc/dnx/legacy/ARAD/arad_api_general.h>
#include <soc/dnx/legacy/SAND/Management/sand_general_macros.h>

#include <soc/dnx/legacy/ARAD/arad_sw_db.h>
#include <soc/dnx/legacy/ARAD/arad_general.h>

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
*     Sets the Explicit Flow Unicast packets mapping to queue.
*     Doesn't affect packets that arrive with destination_id
*     in the header.
*     Details: in the H file. (search for prototype)
*********************************************************************/
int
  jer2_arad_ipq_explicit_mapping_mode_info_set(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  JER2_ARAD_IPQ_EXPLICIT_MAPPING_MODE_INFO *info
  )
{
    uint32 res;

    DNXC_INIT_FUNC_DEFS;

    res = jer2_arad_ipq_explicit_mapping_mode_info_verify(unit, info);
    DNXC_SAND_IF_ERR_EXIT(res);

    res = jer2_arad_ipq_explicit_mapping_mode_info_set_unsafe(unit, info);
    DNXC_SAND_IF_ERR_EXIT(res);

exit:
    DNXC_FUNC_RETURN;
}


/*********************************************************************
*     Sets the Explicit Flow Unicast packets mapping to queue.
*     Doesn't affect packets that arrive with destination_id
*     in the header.
*     Details: in the H file. (search for prototype)
*********************************************************************/
int
  jer2_arad_ipq_explicit_mapping_mode_info_get(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_OUT JER2_ARAD_IPQ_EXPLICIT_MAPPING_MODE_INFO *info
  )
{
    uint32  res;
    DNXC_INIT_FUNC_DEFS;

    res = jer2_arad_ipq_explicit_mapping_mode_info_get_unsafe(unit, info);
    DNXC_SAND_IF_ERR_EXIT(res);

exit:
    DNXC_FUNC_RETURN;
}

/*********************************************************************
*     Maps the embedded traffic class in the packet header to
*     a logical traffic class. This logical traffic class will
*     be further used for traffic management. Note that a class
*     that is mapped to class '0' is equivalent to disabling
*     adding the class.
*     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  jer2_arad_ipq_traffic_class_map_set(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  JER2_ARAD_IPQ_TR_CLS          tr_cls_ndx,
    DNX_SAND_IN  JER2_ARAD_IPQ_TR_CLS          new_class
  )
{
  uint32  res;
  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_IPQ_TRAFFIC_CLASS_MAP_SET);
  DNX_SAND_CHECK_DRIVER_AND_DEVICE;

  res = jer2_arad_ipq_traffic_class_map_verify(
    unit,
    tr_cls_ndx,
    new_class
  );
  DNX_SAND_CHECK_FUNC_RESULT(res, 10, exit);

   DNX_SAND_TAKE_DEVICE_SEMAPHORE;

  res = jer2_arad_ipq_traffic_class_map_set_unsafe(
    unit,
    tr_cls_ndx,
    new_class
  );
  DNX_SAND_CHECK_FUNC_RESULT(res, 100, exit_semaphore);

exit_semaphore:
  DNX_SAND_GIVE_DEVICE_SEMAPHORE;
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_ipq_traffic_class_map_set()",0,0);
}

/*********************************************************************
*     Maps the embedded traffic class in the packet header to
*     a logical traffic class. This logical traffic class will
*     be further used for traffic management. Note that a class
*     that is mapped to class '0' is equivalent to disabling
*     adding the class.
*     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  jer2_arad_ipq_traffic_class_map_get(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  JER2_ARAD_IPQ_TR_CLS          tr_cls_ndx,
    DNX_SAND_OUT JER2_ARAD_IPQ_TR_CLS          *new_class
  )
{
  uint32  res;
  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_IPQ_TRAFFIC_CLASS_MAP_GET);
  DNX_SAND_CHECK_DRIVER_AND_DEVICE;

  DNX_SAND_CHECK_NULL_INPUT(new_class);

   DNX_SAND_TAKE_DEVICE_SEMAPHORE;

  res = jer2_arad_ipq_traffic_class_map_get_unsafe(
    unit,
    tr_cls_ndx,
    new_class
  );
  DNX_SAND_CHECK_FUNC_RESULT(res, 100, exit_semaphore);

exit_semaphore:
  DNX_SAND_GIVE_DEVICE_SEMAPHORE;
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_ipq_traffic_class_map_get()",0,0);
}

/********************************************************************* 
 *      Sets multicast packets traffic class mapping to strict
 *      priority between high-low priority.
 *********************************************************************/
uint32
  jer2_arad_ipq_traffic_class_multicast_priority_map_set(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  uint32                 traffic_class,
    DNX_SAND_IN  uint8                  enable
  )
{
  uint32
    res;
  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_IPQ_TRAFFIC_CLASS_MULTICAST_PRIORITY_MAP_SET);

  res = jer2_arad_ipq_traffic_class_multicast_priority_map_set_verify(
      unit,
      traffic_class,
      enable
  );
  DNX_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  res = jer2_arad_ipq_traffic_class_multicast_priority_map_set_unsafe(
     unit,
     traffic_class,
     enable
  );
  DNX_SAND_CHECK_FUNC_RESULT(res, 20, exit);

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_ipq_traffic_class_multicast_priority_map_set()",0,0);
}

/********************************************************************* 
 *      Gets multicast packets traffic class mapping to strict
 *      priority between high-low priority. 
 *********************************************************************/ 
uint32
  jer2_arad_ipq_traffic_class_multicast_priority_map_get(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  uint32                 traffic_class,
    DNX_SAND_OUT uint8                  *enable
  )
{
  uint32
    res;
  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_IPQ_TRAFFIC_CLASS_MULTICAST_PRIORITY_MAP_GET);
  DNX_SAND_CHECK_NULL_INPUT(enable);
  res = jer2_arad_ipq_traffic_class_multicast_priority_map_get_verify(
      unit,
      traffic_class,
      enable
  );
  DNX_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  res = jer2_arad_ipq_traffic_class_multicast_priority_map_get_unsafe(
     unit,
     traffic_class,
     enable
  );
  DNX_SAND_CHECK_FUNC_RESULT(res, 20, exit);

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_ipq_traffic_class_multicast_priority_map_get()",0,0);
}

/*********************************************************************
*     Sets the system-port (destination-id) mapping to base queue
*     Doesn't affect packets that arrive with
*     explicit queue-id in the header. Each destination ID is
*     mapped to a base_queue, when the packet is stored in
*     queue: base_queue + class
*     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  jer2_arad_ipq_destination_id_packets_base_queue_id_set(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  int                 core,
    DNX_SAND_IN  uint32              dest_ndx,
    DNX_SAND_IN  uint8               valid,
    DNX_SAND_IN  uint8               sw_only,
    DNX_SAND_IN  uint32              base_queue
  )
{
  uint32 res;
  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_IPQ_DESTINATION_ID_PACKETS_BASE_QUEUE_ID_SET);
  DNX_SAND_CHECK_DRIVER_AND_DEVICE;

  res = jer2_arad_ipq_destination_id_packets_base_queue_id_verify(
        unit,
        core,
        dest_ndx,
        valid,
        sw_only,
        base_queue
  );
  DNX_SAND_CHECK_FUNC_RESULT(res, 10, exit);

   DNX_SAND_TAKE_DEVICE_SEMAPHORE;

  res = jer2_arad_ipq_destination_id_packets_base_queue_id_set_unsafe(
        unit,
        core,
        dest_ndx,
        valid,
        sw_only,
        base_queue
  );
  DNX_SAND_CHECK_FUNC_RESULT(res, 100, exit_semaphore);

exit_semaphore:
  DNX_SAND_GIVE_DEVICE_SEMAPHORE;
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_ipq_destination_id_packets_base_queue_id_set()",0,0);
}

/*********************************************************************
*     Gets the system-port (destination-id) mapping to base queue
*     Doesn't affect packets that arrive with
*     explicit queue-id in the header. Each destination ID is
*     mapped to a base_queue, when the packet is stored in
*     queue: base_queue + class
*     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  jer2_arad_ipq_destination_id_packets_base_queue_id_get(
   DNX_SAND_IN  int                 unit,
   DNX_SAND_IN  int                 core,
   DNX_SAND_IN  uint32              dest_ndx,
   DNX_SAND_OUT uint8               *valid,
   DNX_SAND_OUT uint8               *sw_only,
   DNX_SAND_OUT uint32              *base_queue
  )
{
  uint32  res;
  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_IPQ_DESTINATION_ID_PACKETS_BASE_QUEUE_ID_GET);
  DNX_SAND_CHECK_DRIVER_AND_DEVICE;

  DNX_SAND_CHECK_NULL_INPUT(valid);
  DNX_SAND_CHECK_NULL_INPUT(base_queue);

   DNX_SAND_TAKE_DEVICE_SEMAPHORE;

  res = jer2_arad_ipq_destination_id_packets_base_queue_id_get_unsafe(
       unit,
       core,
       dest_ndx,
       valid,
       sw_only,
       base_queue
  );
  DNX_SAND_CHECK_FUNC_RESULT(res, 100, exit_semaphore);

exit_semaphore:
  DNX_SAND_GIVE_DEVICE_SEMAPHORE;
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_ipq_destination_id_packets_base_queue_id_get()",0,0);
}

/*********************************************************************
*     Sets the stack lag packets mapping to queue
*     information. 
*     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  jer2_arad_ipq_stack_lag_packets_base_queue_id_set(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  uint32                  tmd,
    DNX_SAND_IN  uint32                  entry,
    DNX_SAND_IN  uint32                  base_queue
  )
{
  uint32  res;
  DNX_SAND_INIT_ERROR_DEFINITIONS(0);
  DNX_SAND_CHECK_DRIVER_AND_DEVICE;

  res = jer2_arad_ipq_stack_lag_packets_base_queue_id_verify(unit, tmd, entry, base_queue);
  DNX_SAND_CHECK_FUNC_RESULT(res, 10, exit);

   DNX_SAND_TAKE_DEVICE_SEMAPHORE;

  res = jer2_arad_ipq_stack_lag_packets_base_queue_id_set_unsafe(unit, tmd, entry,base_queue);
  DNX_SAND_CHECK_FUNC_RESULT(res, 100, exit_semaphore);

exit_semaphore:
  DNX_SAND_GIVE_DEVICE_SEMAPHORE;
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_ipq_stack_lag_packets_base_queue_id_set()",0,0);
}

/*********************************************************************
*     Gets the Stack lag packets mapping to queue
*     information.
*     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  jer2_arad_ipq_stack_lag_packets_base_queue_id_get(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  uint32                 tmd,
    DNX_SAND_IN  uint32                 entry,
    DNX_SAND_OUT uint32              *base_queue
  )
{
  uint32  res;
  DNX_SAND_INIT_ERROR_DEFINITIONS(0);
  DNX_SAND_CHECK_DRIVER_AND_DEVICE;

 DNX_SAND_CHECK_NULL_INPUT(base_queue);

   DNX_SAND_TAKE_DEVICE_SEMAPHORE;

  res = jer2_arad_ipq_stack_lag_packets_base_queue_id_get_unsafe(unit, tmd, entry, base_queue);
  DNX_SAND_CHECK_FUNC_RESULT(res, 100, exit_semaphore);

exit_semaphore:
  DNX_SAND_GIVE_DEVICE_SEMAPHORE;
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_ipq_stack_lag_packets_base_queue_id_get()",0,0);
}

/*********************************************************************
*     Sets/Get the stack fec mapping to stack trunk mapping
*     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  jer2_arad_ipq_stack_fec_map_stack_lag_set(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  uint32                  tmd,
    DNX_SAND_IN  uint32                  entry,
    DNX_SAND_IN  uint32                  stack_lag
  )
{
  uint32  res;
  DNX_SAND_INIT_ERROR_DEFINITIONS(0);
  DNX_SAND_CHECK_DRIVER_AND_DEVICE;

  res = jer2_arad_ipq_stack_fec_map_stack_lag_verify(unit, tmd, entry, stack_lag);
  DNX_SAND_CHECK_FUNC_RESULT(res, 10, exit);

   DNX_SAND_TAKE_DEVICE_SEMAPHORE;

  res = jer2_arad_ipq_stack_fec_map_stack_lag_set_unsafe(unit, tmd, entry,stack_lag);
  DNX_SAND_CHECK_FUNC_RESULT(res, 100, exit_semaphore);

exit_semaphore:
  DNX_SAND_GIVE_DEVICE_SEMAPHORE;
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_ipq_stack_fec_map_stack_lag_set()",0,0);
}


uint32
  jer2_arad_ipq_stack_fec_map_stack_lag_get(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  uint32                  tmd,
    DNX_SAND_IN  uint32                  entry,
    DNX_SAND_OUT  uint32*                stack_lag
  )
{
  uint32  res;
  DNX_SAND_INIT_ERROR_DEFINITIONS(0);
  DNX_SAND_CHECK_DRIVER_AND_DEVICE;

  DNX_SAND_CHECK_NULL_INPUT(stack_lag);

  res = jer2_arad_ipq_stack_fec_map_stack_lag_verify(unit, tmd, entry, 0);
  DNX_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  DNX_SAND_TAKE_DEVICE_SEMAPHORE;

  res = jer2_arad_ipq_stack_fec_map_stack_lag_get_unsafe(unit, tmd, entry, stack_lag);
  DNX_SAND_CHECK_FUNC_RESULT(res, 100, exit_semaphore);

exit_semaphore:
  DNX_SAND_GIVE_DEVICE_SEMAPHORE;

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_ipq_stack_fec_map_stack_lag_get()",0,0);
}

/*********************************************************************
*     Map 1K of queues (256 quartets) to be in interdigitated
*     mode or not. Queue Quartets that configured to be in
*     interdigitated mode should only be configured with
*     interdigitated flow quartets, and the other-way around.
*     When interdigitated mode is set, all queue quartets
*     range are reset using jer2_arad_ipq_quartet_reset(), in
*     order to prevent illegal interdigitated/composite state.
*     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  jer2_arad_ipq_queue_interdigitated_mode_set(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  int                 core,
    DNX_SAND_IN  uint32                  k_queue_ndx,
    DNX_SAND_IN  uint8                 is_interdigitated
  )
{
  uint32  res;
  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_IPQ_QUEUE_INTERDIGITATED_MODE_SET);
  DNX_SAND_CHECK_DRIVER_AND_DEVICE;


  res = jer2_arad_ipq_queue_interdigitated_mode_verify(
    unit,
    core,
    k_queue_ndx,
    is_interdigitated
  );
  DNX_SAND_CHECK_FUNC_RESULT(res, 10, exit);

   DNX_SAND_TAKE_DEVICE_SEMAPHORE;

  res = jer2_arad_ipq_queue_interdigitated_mode_set_unsafe(
    unit,
    core,
    k_queue_ndx,
    is_interdigitated
  );
  DNX_SAND_CHECK_FUNC_RESULT(res, 100, exit_semaphore);

exit_semaphore:
  DNX_SAND_GIVE_DEVICE_SEMAPHORE;
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_ipq_queue_interdigitated_mode_set()",0,0);
}

/*********************************************************************
*     Map 1K of queues (256 quartets) to be in interdigitated
*     mode or not. Queue Quartets that configured to be in
*     interdigitated mode should only be configured with
*     interdigitated flow quartets, and the other-way around.
*     When interdigitated mode is set, all queue quartets
*     range are reset using jer2_arad_ipq_quartet_reset(), in
*     order to prevent illegal interdigitated/composite state.
*     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  jer2_arad_ipq_queue_interdigitated_mode_get(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  int                 core,
    DNX_SAND_IN  uint32                  k_queue_ndx,
    DNX_SAND_OUT uint8                 *is_interdigitated
  )
{
  uint32  res;
  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_IPQ_QUEUE_INTERDIGITATED_MODE_GET);
  DNX_SAND_CHECK_DRIVER_AND_DEVICE;

  DNX_SAND_CHECK_NULL_INPUT(is_interdigitated);

   DNX_SAND_TAKE_DEVICE_SEMAPHORE;

  res = jer2_arad_ipq_queue_interdigitated_mode_get_unsafe(
    unit,
    core,
    k_queue_ndx,
    is_interdigitated
  );
  DNX_SAND_CHECK_FUNC_RESULT(res, 100, exit_semaphore);

exit_semaphore:
  DNX_SAND_GIVE_DEVICE_SEMAPHORE;
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_ipq_queue_interdigitated_mode_get()",0,0);
}
/*********************************************************************
*     Map a queues-quartet to system port and flow-quartet(s).
*     This function should only be called after calling the
*     jer2_arad_ipq_queue_interdigitated_mode_set()
*     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  jer2_arad_ipq_queue_to_flow_mapping_set(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  int                 core,
    DNX_SAND_IN  uint32                  queue_quartet_ndx,
    DNX_SAND_IN  JER2_ARAD_IPQ_QUARTET_MAP_INFO *info
  )
{
  uint32  res;
  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_IPQ_QUEUE_TO_FLOW_MAPPING_SET);
  DNX_SAND_CHECK_DRIVER_AND_DEVICE;

  DNX_SAND_CHECK_NULL_INPUT(info);

  DNX_SAND_TAKE_DEVICE_SEMAPHORE;

  res = jer2_arad_ipq_queue_to_flow_mapping_verify(
    unit,
    core,
    queue_quartet_ndx,
    info
  );
  DNX_SAND_CHECK_FUNC_RESULT(res, 10, exit_semaphore);

  res = jer2_arad_ipq_queue_to_flow_mapping_set_unsafe(
    unit,
    core,
    queue_quartet_ndx,
    info
  );
  DNX_SAND_CHECK_FUNC_RESULT(res, 100, exit_semaphore);

exit_semaphore:
  DNX_SAND_GIVE_DEVICE_SEMAPHORE;
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_ipq_queue_to_flow_mapping_set()",0,0);
}

/*********************************************************************
*     Map a queues-quartet to system port and flow-quartet(s).
*     This function should only be called after calling the
*     jer2_arad_ipq_queue_interdigitated_mode_set()
*     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  jer2_arad_ipq_queue_to_flow_mapping_get(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  int                 core,
    DNX_SAND_IN  uint32                  queue_quartet_ndx,
    DNX_SAND_OUT JER2_ARAD_IPQ_QUARTET_MAP_INFO *info
  )
{
  uint32  res;
  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_IPQ_QUEUE_TO_FLOW_MAPPING_GET);
  DNX_SAND_CHECK_DRIVER_AND_DEVICE;

  DNX_SAND_CHECK_NULL_INPUT(info);

   DNX_SAND_TAKE_DEVICE_SEMAPHORE;

  res = jer2_arad_ipq_queue_to_flow_mapping_get_unsafe(
    unit,
    core,
    queue_quartet_ndx,
    info
  );
  DNX_SAND_CHECK_FUNC_RESULT(res, 100, exit_semaphore);

exit_semaphore:
  DNX_SAND_GIVE_DEVICE_SEMAPHORE;
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_ipq_queue_to_flow_mapping_get()",0,0);
}

/*********************************************************************
*     Unmap a queues-quartet, by mapping it to invalid
*     destination. Also, flush all the queues in the quartet.
*     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  jer2_arad_ipq_queue_qrtt_unmap(
    DNX_SAND_IN  int unit,
    DNX_SAND_IN  int core,
    DNX_SAND_IN  uint32  queue_quartet_ndx
  )
{
  uint32
    res = DNX_SAND_OK;

  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_IPQ_QUEUE_QRTT_UNMAP);
  DNX_SAND_CHECK_DRIVER_AND_DEVICE;

   DNX_SAND_TAKE_DEVICE_SEMAPHORE;

  res = jer2_arad_ipq_queue_qrtt_unmap_unsafe(
          unit,
          core,
          queue_quartet_ndx
        );
  DNX_SAND_CHECK_FUNC_RESULT(res, 10, exit_semaphore);

exit_semaphore:
  DNX_SAND_GIVE_DEVICE_SEMAPHORE;
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_ipq_queue_qrtt_unmap()",queue_quartet_ndx,0);
}

/*********************************************************************
*     Resets a quartet to default values.
*     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  jer2_arad_ipq_quartet_reset(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  int                 core,
    DNX_SAND_IN  uint32                  queue_quartet_ndx
  )
{
  uint32  res;
  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_IPQ_QUARTET_RESET);
  DNX_SAND_CHECK_DRIVER_AND_DEVICE;


   DNX_SAND_TAKE_DEVICE_SEMAPHORE;

  res = jer2_arad_ipq_quartet_reset_unsafe(
    unit,
    core,
    queue_quartet_ndx
  );
  DNX_SAND_CHECK_FUNC_RESULT(res, 100, exit_semaphore);

exit_semaphore:
  DNX_SAND_GIVE_DEVICE_SEMAPHORE;
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_ipq_quartet_reset()",0,0);
}

uint32
  jer2_arad_ipq_k_quartet_reset(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  int                 core,
    DNX_SAND_IN  uint32                  queue_k_quartet_ndx,
    DNX_SAND_IN uint32                  region_size
  )
{
  uint32  res;
  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_IPQ_K_QUARTET_RESET);
  DNX_SAND_CHECK_DRIVER_AND_DEVICE;


   DNX_SAND_TAKE_DEVICE_SEMAPHORE;

  res = jer2_arad_ipq_k_quartet_reset_unsafe(
    unit,
    core,
    queue_k_quartet_ndx,
    region_size
  );
  DNX_SAND_CHECK_FUNC_RESULT(res, 100, exit_semaphore);

exit_semaphore:
  DNX_SAND_GIVE_DEVICE_SEMAPHORE;
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_ipq_k_quartet_reset()",0,0);
}

void
  jer2_arad_JER2_ARAD_IPQ_EXPLICIT_MAPPING_MODE_INFO_clear(
    DNX_SAND_OUT JER2_ARAD_IPQ_EXPLICIT_MAPPING_MODE_INFO *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  DNX_TMC_IPQ_EXPLICIT_MAPPING_MODE_INFO_clear(info);
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  jer2_arad_JER2_ARAD_IPQ_BASEQ_MAP_INFO_clear(
    DNX_SAND_OUT JER2_ARAD_IPQ_BASEQ_MAP_INFO *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  DNX_TMC_IPQ_BASEQ_MAP_INFO_clear(info);
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  jer2_arad_JER2_ARAD_IPQ_QUARTET_MAP_INFO_clear(
    DNX_SAND_OUT JER2_ARAD_IPQ_QUARTET_MAP_INFO *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  DNX_TMC_IPQ_QUARTET_MAP_INFO_clear(info);
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

uint32
  jer2_arad_ipq_attached_flow_port_get(
    DNX_SAND_IN  int unit,
    DNX_SAND_IN  int core,
    DNX_SAND_IN  uint32 queue_ndx,
    DNX_SAND_OUT uint32 *flow_id,
    DNX_SAND_OUT uint32 *sys_port
  )
{
  uint32
    res = DNX_SAND_OK;

  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_IPQ_ATTACHED_FLOW_PORT_GET);

  DNX_SAND_CHECK_DRIVER_AND_DEVICE;

  DNX_SAND_CHECK_NULL_INPUT(flow_id);
  DNX_SAND_CHECK_NULL_INPUT(sys_port);

   DNX_SAND_TAKE_DEVICE_SEMAPHORE;

  res = jer2_arad_ipq_attached_flow_port_get_unsafe(
          unit,
          core,
          queue_ndx,
          flow_id,
          sys_port
        );
  DNX_SAND_CHECK_FUNC_RESULT(res, 100, exit_semaphore);

exit_semaphore:
  DNX_SAND_GIVE_DEVICE_SEMAPHORE;
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_ipq_attached_flow_port_get()",0,0);
}


#if JER2_ARAD_DEBUG_IS_LVL1


uint32
  jer2_arad_ips_non_empty_queues_info_get(
    DNX_SAND_IN  int                unit,
    DNX_SAND_IN  int                   core,
    DNX_SAND_IN  uint32                first_queue,
    DNX_SAND_IN  uint32                max_array_size,
    DNX_SAND_OUT dnx_soc_ips_queue_info_t* queues,
    DNX_SAND_OUT uint32*               nof_queues_filled,
    DNX_SAND_OUT uint32*               next_queue,
    DNX_SAND_OUT uint32*               reached_end
  )
{
  uint32
    res = DNX_SAND_OK;

  DNX_SAND_INIT_ERROR_DEFINITIONS(0);

  DNX_SAND_CHECK_DRIVER_AND_DEVICE;

  DNX_SAND_TAKE_DEVICE_SEMAPHORE;

  res = jer2_arad_ips_non_empty_queues_info_get_unsafe(
          unit,
          core,
          first_queue,
          max_array_size,
          queues,
          nof_queues_filled,
          next_queue,
          reached_end
        );
  DNX_SAND_CHECK_FUNC_RESULT(res, 10, exit_semaphore);

exit_semaphore:
  DNX_SAND_GIVE_DEVICE_SEMAPHORE;
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_ips_non_empty_queues_print()",0,0);

}

#endif /* JER2_ARAD_DEBUG_IS_LVL1 */

/* } */

/*
 * Assign a TC Mapping Profile to a specific UC Destination or Flow ID.
 * The profile can me configured using jer2_arad_ipq_tc_profile_map_set()
 * 
 * dest_ndx - Destination Index - either a UC Destination or Flow ID
 * is_flow_mdx - 1 for Flow ID, 0 for UC Destination
 * tc_profile - Profile ID to map
 */
uint32
  jer2_arad_ipq_tc_profile_set(
    DNX_SAND_IN	  int	  unit,
    DNX_SAND_IN   int     core,
    DNX_SAND_IN	  uint8	  is_flow_ndx,
    DNX_SAND_IN	  uint32	  dest_ndx,
    DNX_SAND_IN   uint32	  tc_profile
  )
{
  uint32
    res = DNX_SAND_OK;

  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_IPQ_TC_PROFILE_SET);
  DNX_SAND_CHECK_DRIVER_AND_DEVICE;

  DNX_SAND_TAKE_DEVICE_SEMAPHORE;

  res = jer2_arad_ipq_tc_profile_verify(
            unit,
            is_flow_ndx,
            dest_ndx,
            tc_profile
          );
  DNX_SAND_CHECK_FUNC_RESULT(res,10,exit_semaphore);
  
  res = jer2_arad_ipq_tc_profile_set_unsafe(
          unit,
          core,
          is_flow_ndx,
          dest_ndx,
          tc_profile
        );
  DNX_SAND_CHECK_FUNC_RESULT(res,20,exit_semaphore);

exit_semaphore:
  DNX_SAND_GIVE_DEVICE_SEMAPHORE;
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_ipq_tc_profile_set()",0,0);                                 
}

/*
 * Get a TC Mapping Profile from a specific UC Destination or Flow ID.
 * 
 * dest_ndx - Destination Index - either a UC Destination or Flow ID
 * is_flow_mdx - 1 for Flow ID, 0 for UC Destination
 */
uint32
  jer2_arad_ipq_tc_profile_get(
    DNX_SAND_IN	  int         unit,
    DNX_SAND_IN   int         core,
    DNX_SAND_IN	  uint8       is_flow_ndx,
    DNX_SAND_IN	  uint32      dest_ndx,
    DNX_SAND_OUT  uint32      *tc_profile
  )
{
  uint32
    res = DNX_SAND_OK;

  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_IPQ_TC_PROFILE_SET);
  DNX_SAND_CHECK_DRIVER_AND_DEVICE;

  DNX_SAND_TAKE_DEVICE_SEMAPHORE;

  res = jer2_arad_ipq_tc_profile_verify(
          unit,
          is_flow_ndx,
          dest_ndx,
          0
        );
  DNX_SAND_CHECK_FUNC_RESULT(res,10,exit_semaphore);

  res = jer2_arad_ipq_tc_profile_get_unsafe(
          unit,
          core,
          is_flow_ndx,
          dest_ndx,
          tc_profile
        );
  DNX_SAND_CHECK_FUNC_RESULT(res,20,exit_semaphore);

exit_semaphore:
  DNX_SAND_GIVE_DEVICE_SEMAPHORE;
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_ipq_tc_profile_get()",0,0);                                 
}

/*
 * Configure a TC Mapping Profile
 * The profile can me assigned to a destination using jer2_arad_ipq_tc_profile_set()
 * 
 * profile_ndx - Profile index to configure
 * is_flow_profile - 1 for Flow ID, 0 for UC Destination
 * Mapping:
 *    tr_cls_ndx - Old TC
 *    new_class - New Tc
 */
uint32
  jer2_arad_ipq_tc_profile_map_set(
    DNX_SAND_IN  int                unit,
    DNX_SAND_IN  int                core_id,
    DNX_SAND_IN  JER2_ARAD_IPQ_TR_CLS_PROFILE  profile_ndx,
    DNX_SAND_IN  uint8                is_flow_profile,
    DNX_SAND_IN  JER2_ARAD_IPQ_TR_CLS          tr_cls_ndx,
    DNX_SAND_IN  JER2_ARAD_IPQ_TR_CLS          new_class
  )
{
  uint32
    res = DNX_SAND_OK;

  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_IPQ_TRAFFIC_CLASS_PROFILE_MAP_SET);
  DNX_SAND_CHECK_DRIVER_AND_DEVICE;

  DNX_SAND_TAKE_DEVICE_SEMAPHORE;

  res = jer2_arad_ipq_traffic_class_profile_map_verify(
          unit,
          core_id,
          profile_ndx,
          is_flow_profile,
          tr_cls_ndx,
          0
        );
  DNX_SAND_CHECK_FUNC_RESULT(res,20,exit_semaphore);

  res = jer2_arad_ipq_traffic_class_profile_map_set_unsafe(
          unit,
          core_id,
          profile_ndx,
          is_flow_profile,
          FALSE,
          tr_cls_ndx,
          new_class
        );
  DNX_SAND_CHECK_FUNC_RESULT(res,20,exit_semaphore);

exit_semaphore:
  DNX_SAND_GIVE_DEVICE_SEMAPHORE;
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_ipq_traffic_class_profile_map_set()",0,0);                                 
}

/*
 * Get a TC Mapping Profile configuration for a specific TC
 * 
 * profile_ndx - Profile index to configure
 * is_flow_profile - 1 for Flow ID, 0 for UC Destination
 * Mapping:
 *    tr_cls_ndx - Old TC
 *    new_class - New Tc
 */
uint32
  jer2_arad_ipq_tc_profile_map_get(
    DNX_SAND_IN  int                unit,
    DNX_SAND_IN  int                core_id,
    DNX_SAND_IN  JER2_ARAD_IPQ_TR_CLS_PROFILE  profile_ndx,
    DNX_SAND_IN  uint8                is_flow_profile,
    DNX_SAND_IN  JER2_ARAD_IPQ_TR_CLS          tr_cls_ndx,
    DNX_SAND_OUT  JER2_ARAD_IPQ_TR_CLS         *new_class
  )
{
  uint32
    res = DNX_SAND_OK;

  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_IPQ_TRAFFIC_CLASS_PROFILE_MAP_GET);
  DNX_SAND_CHECK_DRIVER_AND_DEVICE;

  DNX_SAND_TAKE_DEVICE_SEMAPHORE;

  res = jer2_arad_ipq_traffic_class_profile_map_verify(
          unit,
          core_id,
          profile_ndx,
          is_flow_profile,
          tr_cls_ndx,
          0
        );
  DNX_SAND_CHECK_FUNC_RESULT(res,20,exit_semaphore);

  res = jer2_arad_ipq_traffic_class_profile_map_get_unsafe(
          unit,
          core_id,
          profile_ndx,
          is_flow_profile,
          FALSE,
          tr_cls_ndx,
          new_class
        );
  DNX_SAND_CHECK_FUNC_RESULT(res,20,exit_semaphore);

exit_semaphore:
  DNX_SAND_GIVE_DEVICE_SEMAPHORE;
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_ipq_traffic_class_profile_map_set()",0,0);                                 
}


#include <soc/dnx/legacy/SAND/Utils/sand_footer.h>


#endif /* of #if defined(BCM_88690_A0) */

