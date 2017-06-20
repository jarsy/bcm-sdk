/* $Id: jer2_tmc_api_ingress_scheduler.c,v 1.9 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/
/******************************************************************
*
* FILENAME:       DuneDriver/jer2_tmc/src/soc_jer2_tmcapi_ingress_scheduler.c
*
* MODULE PREFIX:  soc_jer2_tmcingress_scheduler
*
* FILE DESCRIPTION:  in the H file.
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

#include <soc/dnx/legacy/TMC/tmc_api_ingress_scheduler.h>

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
  DNX_TMC_ING_SCH_SHAPER_clear(
    DNX_SAND_OUT DNX_TMC_ING_SCH_SHAPER *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  sal_memset(info, 0x0, sizeof(DNX_TMC_ING_SCH_SHAPER));
  info->max_rate = 0;
  info->max_burst = 0;

  info->slow_start_enable = 0;
  info->slow_start_rate_phase_0 = 0;
  info->slow_start_rate_phase_1 = 0;
  DNX_SAND_MAGIC_NUM_SET;
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  DNX_TMC_ING_SCH_MESH_CONTEXT_INFO_clear(
    DNX_SAND_OUT DNX_TMC_ING_SCH_MESH_CONTEXT_INFO *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  sal_memset(info, 0x0, sizeof(DNX_TMC_ING_SCH_MESH_CONTEXT_INFO));
  DNX_TMC_ING_SCH_SHAPER_clear(&(info->shaper));
  info->weight = 0;
  info->id = DNX_TMC_ING_NOF_SCH_MESH_CONTEXTSS;
  DNX_SAND_MAGIC_NUM_SET;
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  DNX_TMC_ING_SCH_MESH_INFO_clear(
    DNX_SAND_OUT DNX_TMC_ING_SCH_MESH_INFO *info
  )
{
  uint32
    ind;

  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  sal_memset(info, 0x0, sizeof(DNX_TMC_ING_SCH_MESH_INFO));
  for (ind = 0; ind < DNX_TMC_ING_SCH_MESH_LAST; ++ind)
  {
    DNX_TMC_ING_SCH_MESH_CONTEXT_INFO_clear(&(info->contexts[ind]));
  }
  info->nof_entries = 0;
  DNX_TMC_ING_SCH_SHAPER_clear(&(info->total_rate_shaper));
  DNX_SAND_MAGIC_NUM_SET;
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}
  
void
  DNX_TMC_ING_SCH_MESH_INFO_SHAPERS_dont_touch(
    DNX_SAND_OUT DNX_TMC_ING_SCH_MESH_INFO *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);
  info->contexts[DNX_TMC_ING_SCH_MESH_CON1].shaper.max_rate = DNX_TMC_ING_SCH_DONT_TOUCH;
  info->contexts[DNX_TMC_ING_SCH_MESH_CON2].shaper.max_rate = DNX_TMC_ING_SCH_DONT_TOUCH;
  info->contexts[DNX_TMC_ING_SCH_MESH_CON3].shaper.max_rate = DNX_TMC_ING_SCH_DONT_TOUCH;
  info->contexts[DNX_TMC_ING_SCH_MESH_CON4].shaper.max_rate = DNX_TMC_ING_SCH_DONT_TOUCH;
  info->contexts[DNX_TMC_ING_SCH_MESH_CON5].shaper.max_rate = DNX_TMC_ING_SCH_DONT_TOUCH;
  info->contexts[DNX_TMC_ING_SCH_MESH_CON6].shaper.max_rate = DNX_TMC_ING_SCH_DONT_TOUCH;
  info->contexts[DNX_TMC_ING_SCH_MESH_CON7].shaper.max_rate = DNX_TMC_ING_SCH_DONT_TOUCH;
  info->total_rate_shaper.max_rate=DNX_TMC_ING_SCH_DONT_TOUCH;

exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}  
  

void
  DNX_TMC_ING_SCH_CLOS_WFQ_ELEMENT_clear(
    DNX_SAND_OUT DNX_TMC_ING_SCH_CLOS_WFQ_ELEMENT *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  sal_memset(info, 0x0, sizeof(DNX_TMC_ING_SCH_CLOS_WFQ_ELEMENT));
  info->weight1 = 0;
  info->weight2 = 0;
  DNX_SAND_MAGIC_NUM_SET;
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  DNX_TMC_ING_SCH_CLOS_WFQS_clear(
    DNX_SAND_OUT DNX_TMC_ING_SCH_CLOS_WFQS *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  sal_memset(info, 0x0, sizeof(DNX_TMC_ING_SCH_CLOS_WFQS));
  DNX_TMC_ING_SCH_CLOS_WFQ_ELEMENT_clear(&(info->fabric_hp));
  DNX_TMC_ING_SCH_CLOS_WFQ_ELEMENT_clear(&(info->fabric_lp));
  DNX_TMC_ING_SCH_CLOS_WFQ_ELEMENT_clear(&(info->global_hp));
  DNX_TMC_ING_SCH_CLOS_WFQ_ELEMENT_clear(&(info->global_lp));
  DNX_SAND_MAGIC_NUM_SET;
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  DNX_TMC_ING_SCH_CLOS_HP_SHAPERS_clear(
    DNX_SAND_OUT DNX_TMC_ING_SCH_CLOS_HP_SHAPERS *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  sal_memset(info, 0x0, sizeof(DNX_TMC_ING_SCH_CLOS_HP_SHAPERS));
  DNX_TMC_ING_SCH_SHAPER_clear(&(info->local));
  DNX_TMC_ING_SCH_SHAPER_clear(&(info->fabric_unicast));
  DNX_TMC_ING_SCH_SHAPER_clear(&(info->fabric_multicast));
  DNX_SAND_MAGIC_NUM_SET;
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  DNX_TMC_ING_SCH_CLOS_SHAPERS_clear(
    DNX_SAND_OUT DNX_TMC_ING_SCH_CLOS_SHAPERS *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  sal_memset(info, 0x0, sizeof(DNX_TMC_ING_SCH_CLOS_SHAPERS));
  DNX_TMC_ING_SCH_SHAPER_clear(&(info->local));
  DNX_TMC_ING_SCH_SHAPER_clear(&(info->fabric));
  DNX_TMC_ING_SCH_CLOS_HP_SHAPERS_clear(&(info->hp));
  DNX_SAND_MAGIC_NUM_SET;
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  DNX_TMC_ING_SCH_CLOS_INFO_clear(
    DNX_SAND_OUT DNX_TMC_ING_SCH_CLOS_INFO *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  sal_memset(info, 0x0, sizeof(DNX_TMC_ING_SCH_CLOS_INFO));
  DNX_TMC_ING_SCH_CLOS_SHAPERS_clear(&(info->shapers));
  DNX_TMC_ING_SCH_CLOS_WFQS_clear(&(info->weights));
  DNX_SAND_MAGIC_NUM_SET;
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}
void
  DNX_TMC_ING_SCH_CLOS_INFO_SHAPERS_dont_touch(
    DNX_SAND_OUT DNX_TMC_ING_SCH_CLOS_INFO *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  info->shapers.local.max_rate = DNX_TMC_ING_SCH_DONT_TOUCH;
  info->shapers.hp.local.max_rate = DNX_TMC_ING_SCH_DONT_TOUCH;
  info->shapers.fabric.max_rate = DNX_TMC_ING_SCH_DONT_TOUCH;
  info->shapers.hp.fabric_unicast.max_rate = DNX_TMC_ING_SCH_DONT_TOUCH;
  info->shapers.hp.fabric_multicast.max_rate = DNX_TMC_ING_SCH_DONT_TOUCH;
  info->shapers.hp.fabric_multicast.slow_start_enable = DNX_TMC_ING_SCH_DONT_TOUCH;
  info->shapers.hp.fabric_multicast.slow_start_rate_phase_0 = DNX_TMC_ING_SCH_DONT_TOUCH;
  info->shapers.hp.fabric_multicast.slow_start_rate_phase_1 = DNX_TMC_ING_SCH_DONT_TOUCH;
  info->shapers.lp.fabric_unicast.max_rate = DNX_TMC_ING_SCH_DONT_TOUCH;
  info->shapers.lp.fabric_multicast.max_rate = DNX_TMC_ING_SCH_DONT_TOUCH;
  info->shapers.lp.fabric_multicast.slow_start_enable = DNX_TMC_ING_SCH_DONT_TOUCH;
  info->shapers.lp.fabric_multicast.slow_start_rate_phase_0 = DNX_TMC_ING_SCH_DONT_TOUCH;
  info->shapers.lp.fabric_multicast.slow_start_rate_phase_1 = DNX_TMC_ING_SCH_DONT_TOUCH;

exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}
	

#if DNX_TMC_DEBUG_IS_LVL1

const char*
  DNX_TMC_ING_SCH_MESH_CONTEXTS_to_string(
    DNX_SAND_IN  DNX_TMC_ING_SCH_MESH_CONTEXTS enum_val
  )
{
  const char* str = NULL;
  switch(enum_val)
  {
  case DNX_TMC_ING_SCH_MESH_LOCAL:
    str = "LOCAL";
  break;

  case DNX_TMC_ING_SCH_MESH_CON1:
    str = "CONTEXT1";
  break;

  case DNX_TMC_ING_SCH_MESH_CON2:
    str = "CONTEXT2";
  break;

  case DNX_TMC_ING_SCH_MESH_CON3:
    str = "CONTEXT3";
  break;

  case DNX_TMC_ING_SCH_MESH_CON4:
    str = "CONTEXT4";
  break;

  case DNX_TMC_ING_SCH_MESH_CON5:
    str = "CONTEXT5";
  break;

  case DNX_TMC_ING_SCH_MESH_CON6:
    str = "CONTEXT6";
  break;

  case DNX_TMC_ING_SCH_MESH_CON7:
    str = "CONTEXT7";
  break;

  case DNX_TMC_ING_SCH_MESH_LAST:
    str = " Not initialized";
  break;

  default:
    str = " Unknown";
  }
  return str;
}

void
  DNX_TMC_ING_SCH_SHAPER_print(
    DNX_SAND_IN DNX_TMC_ING_SCH_SHAPER *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  LOG_CLI((BSL_META_U(unit,
                      "Max_rate: %u[Kbps], "),info->max_rate));
  LOG_CLI((BSL_META_U(unit,
                      "Max_burst: %u[Bytes].\n\r"),info->max_burst));

  if (info->slow_start_enable)
  {
      LOG_CLI((BSL_META_U(unit,
                          "Slow rate first phase precent: %u%% "),info->slow_start_rate_phase_0));
      LOG_CLI((BSL_META_U(unit,
                          "Slow rate second phase precent: %u%% "),info->slow_start_rate_phase_1));
  }
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  DNX_TMC_ING_SCH_MESH_CONTEXT_INFO_print(
    DNX_SAND_IN DNX_TMC_ING_SCH_MESH_CONTEXT_INFO *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  LOG_CLI((BSL_META_U(unit,
                      "  Shaper:")));
  DNX_TMC_ING_SCH_SHAPER_print(&(info->shaper));
  LOG_CLI((BSL_META_U(unit,
                      "  Weight: %u\n\r"),info->weight));
  LOG_CLI((BSL_META_U(unit,
                      " Id %s \n\r"),
           DNX_TMC_ING_SCH_MESH_CONTEXTS_to_string(info->id)
           ));
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  DNX_TMC_ING_SCH_MESH_INFO_print(
    DNX_SAND_IN DNX_TMC_ING_SCH_MESH_INFO *info
  )
{
  uint32 ind=0;
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  for (ind=0; ind<DNX_TMC_ING_SCH_MESH_LAST; ++ind)
  {
    LOG_CLI((BSL_META_U(unit,
                        "Contexts[%u]:\n\r"),ind));
    DNX_TMC_ING_SCH_MESH_CONTEXT_INFO_print(&(info->contexts[ind]));
  }
  LOG_CLI((BSL_META_U(unit,
                      "Nof_entries: %u[Entries]\n\r"),info->nof_entries));
  LOG_CLI((BSL_META_U(unit,
                      "Total_rate_shaper: ")));
  DNX_TMC_ING_SCH_SHAPER_print(&(info->total_rate_shaper));
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  DNX_TMC_ING_SCH_CLOS_WFQ_ELEMENT_print(
    DNX_SAND_IN DNX_TMC_ING_SCH_CLOS_WFQ_ELEMENT *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  LOG_CLI((BSL_META_U(unit,
                      "    Weight1: %u, "),info->weight1));
  LOG_CLI((BSL_META_U(unit,
                      "Weight2: %u\n\r"),info->weight2));
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  DNX_TMC_ING_SCH_CLOS_WFQS_print(
    DNX_SAND_IN DNX_TMC_ING_SCH_CLOS_WFQS *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  LOG_CLI((BSL_META_U(unit,
                      "  Fabric_hp:\n\r")));
  DNX_TMC_ING_SCH_CLOS_WFQ_ELEMENT_print(&(info->fabric_hp));
  LOG_CLI((BSL_META_U(unit,
                      "  Fabric_lp:\n\r")));
  DNX_TMC_ING_SCH_CLOS_WFQ_ELEMENT_print(&(info->fabric_lp));
  LOG_CLI((BSL_META_U(unit,
                      "  Global_hp:\n\r")));
  DNX_TMC_ING_SCH_CLOS_WFQ_ELEMENT_print(&(info->global_hp));
  LOG_CLI((BSL_META_U(unit,
                      "  Global_lp:\n\r")));
  DNX_TMC_ING_SCH_CLOS_WFQ_ELEMENT_print(&(info->global_lp));
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  DNX_TMC_ING_SCH_CLOS_HP_SHAPERS_print(
    DNX_SAND_IN  DNX_TMC_ING_SCH_CLOS_HP_SHAPERS *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  LOG_CLI((BSL_META_U(unit,
                      " Local:")));
  DNX_TMC_ING_SCH_SHAPER_print(&(info->local));
  LOG_CLI((BSL_META_U(unit,
                      " Fabric_unicast:")));
  DNX_TMC_ING_SCH_SHAPER_print(&(info->fabric_unicast));
  LOG_CLI((BSL_META_U(unit,
                      " Fabric_multicast:")));
  DNX_TMC_ING_SCH_SHAPER_print(&(info->fabric_multicast));
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  DNX_TMC_ING_SCH_CLOS_SHAPERS_print(
    DNX_SAND_IN DNX_TMC_ING_SCH_CLOS_SHAPERS *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  LOG_CLI((BSL_META_U(unit,
                      "  Local: ")));
  DNX_TMC_ING_SCH_SHAPER_print(&(info->local));
  LOG_CLI((BSL_META_U(unit,
                      "  Fabric: ")));
  DNX_TMC_ING_SCH_SHAPER_print(&(info->fabric));
  LOG_CLI((BSL_META_U(unit,
                      "hp:")));
  DNX_TMC_ING_SCH_CLOS_HP_SHAPERS_print(&(info->hp));
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  DNX_TMC_ING_SCH_CLOS_INFO_print(
    DNX_SAND_IN DNX_TMC_ING_SCH_CLOS_INFO *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  LOG_CLI((BSL_META_U(unit,
                      "Shapers:\n\r")));
  DNX_TMC_ING_SCH_CLOS_SHAPERS_print(&(info->shapers));
  LOG_CLI((BSL_META_U(unit,
                      "Weights:\n\r")));
  DNX_TMC_ING_SCH_CLOS_WFQS_print(&(info->weights));
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

#endif /* DNX_TMC_DEBUG_IS_LVL1 */

/* } */

#include <soc/dnx/legacy/SAND/Utils/sand_footer.h>

