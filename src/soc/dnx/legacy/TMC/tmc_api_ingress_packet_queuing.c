/* $Id: jer2_tmc_api_ingress_packet_queuing.c,v 1.7 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/
/******************************************************************
*
* FILENAME:       DuneDriver/jer2_tmc/src/soc_jer2_tmcapi_ingress_packet_queuing.c
*
* MODULE PREFIX:  soc_jer2_tmcipq
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

#include <soc/dnx/legacy/TMC/tmc_api_general.h>
#include <soc/dnx/legacy/TMC/tmc_api_ingress_packet_queuing.h>

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
  DNX_TMC_IPQ_EXPLICIT_MAPPING_MODE_INFO_clear(
    DNX_SAND_OUT DNX_TMC_IPQ_EXPLICIT_MAPPING_MODE_INFO *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  sal_memset(info, 0x0, sizeof(DNX_TMC_IPQ_EXPLICIT_MAPPING_MODE_INFO));
  info->base_queue_id = 0;
  /*
   *  Invalid configuration - the user must change it before setting
   */
  info->queue_id_add_not_decrement = FALSE;
  DNX_SAND_MAGIC_NUM_SET;
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  DNX_TMC_IPQ_BASEQ_MAP_INFO_clear(
    DNX_SAND_OUT DNX_TMC_IPQ_BASEQ_MAP_INFO *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  sal_memset(info, 0x0, sizeof(DNX_TMC_IPQ_BASEQ_MAP_INFO));
  info->valid = 0;
  info->base_queue = 0;
  DNX_SAND_MAGIC_NUM_SET;
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  DNX_TMC_IPQ_QUARTET_MAP_INFO_clear(
    DNX_SAND_OUT DNX_TMC_IPQ_QUARTET_MAP_INFO *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  sal_memset(info, 0x0, sizeof(DNX_TMC_IPQ_QUARTET_MAP_INFO));
  info->flow_quartet_index = DNX_TMC_IPQ_INVALID_FLOW_QUARTET;
  info->is_composite = 0;
  info->system_physical_port = DNX_TMC_MAX_SYSTEM_PHYSICAL_PORT_ID;
  DNX_SAND_MAGIC_NUM_SET;
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

#if DNX_TMC_DEBUG_IS_LVL1

const char*
  DNX_TMC_IPQ_TR_CLS_RNG_to_string(
    DNX_SAND_IN DNX_TMC_IPQ_TR_CLS_RNG enum_val
  )
{
  const char* str = NULL;
  switch(enum_val)
  {
  case DNX_TMC_IPQ_TR_CLS_MIN:
    str = "TR_CLS_MIN";
  break;

  case DNX_TMC_IPQ_TR_CLS_MAX:
    str = "TR_CLS_MAX";
  break;

  default:
    str = " Unknown";
  }
  return str;
}

void
  DNX_TMC_IPQ_EXPLICIT_MAPPING_MODE_INFO_print(
    DNX_SAND_IN DNX_TMC_IPQ_EXPLICIT_MAPPING_MODE_INFO *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  LOG_CLI((BSL_META_U(unit,
                      "Base_queue_id: %u\n\r"),info->base_queue_id));
  LOG_CLI((BSL_META_U(unit,
                      "Queue_id_add_not_decrement: %d\n\r"),info->queue_id_add_not_decrement));
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  DNX_TMC_IPQ_BASEQ_MAP_INFO_print(
    DNX_SAND_IN  DNX_TMC_IPQ_BASEQ_MAP_INFO *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  LOG_CLI((BSL_META_U(unit,
                      "valid: %u\n\r"),info->valid));
  LOG_CLI((BSL_META_U(unit,
                      "base_queue: %u\n\r"),info->base_queue));
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  DNX_TMC_IPQ_QUARTET_MAP_INFO_print(
    DNX_SAND_IN DNX_TMC_IPQ_QUARTET_MAP_INFO *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  if (
      (info->flow_quartet_index == DNX_TMC_IPQ_INVALID_FLOW_QUARTET) &&
      (info->is_composite == 0) &&
      (info->system_physical_port == DNX_TMC_MAX_SYSTEM_PHYSICAL_PORT_ID)
     )
  {
    LOG_CLI((BSL_META_U(unit,
                        "The queue quartet is unmapped.\n\r")));
  }
  else
  {
    LOG_CLI((BSL_META_U(unit,
                        "Flow_quartet_index: %u\n\r"),info->flow_quartet_index));
    LOG_CLI((BSL_META_U(unit,
                        "Is_composite: %d\n\r"),info->is_composite));
    LOG_CLI((BSL_META_U(unit,
                        "System_physical_port: %u\n\r"),info->system_physical_port));
    LOG_CLI((BSL_META_U(unit,
                        "Fap_id: %u\n\r"),((unsigned)info->fap_id)));
    LOG_CLI((BSL_META_U(unit,
                        "Fap_port_id: %u\n\r"),((unsigned)info->fap_port_id)));
  }

exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

#endif /* DNX_TMC_DEBUG_IS_LVL1 */

/* } */

#include <soc/dnx/legacy/SAND/Utils/sand_footer.h>

