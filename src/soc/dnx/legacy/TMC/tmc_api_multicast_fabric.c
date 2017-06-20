/* $Id: jer2_tmc_api_multicast_fabric.c,v 1.9 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/
/******************************************************************
*
* FILENAME:       DuneDriver/jer2_tmc/src/soc_jer2_tmcapi_multicast_fabric.c
*
* MODULE PREFIX:  soc_jer2_tmcmult_fabric
*
* FILE DESCRIPTION: refer to H file
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

#include <soc/dnx/legacy/TMC/tmc_api_multicast_fabric.h>
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
  DNX_TMC_MULT_FABRIC_PORT_INFO_clear(
    DNX_SAND_OUT DNX_TMC_MULT_FABRIC_PORT_INFO *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  sal_memset(info, 0x0, sizeof(DNX_TMC_MULT_FABRIC_PORT_INFO));
  info->multicast_class_valid = 0;
  DNX_SAND_MAGIC_NUM_SET;
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  DNX_TMC_MULT_FABRIC_SHAPER_INFO_clear(
    DNX_SAND_OUT DNX_TMC_MULT_FABRIC_SHAPER_INFO *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  sal_memset(info, 0x0, sizeof(DNX_TMC_MULT_FABRIC_SHAPER_INFO));
  info->rate = 0;
  info->max_burst = 0;
  DNX_SAND_MAGIC_NUM_SET;
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  DNX_TMC_MULT_FABRIC_BE_CLASS_INFO_clear(
    DNX_SAND_OUT DNX_TMC_MULT_FABRIC_BE_CLASS_INFO *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  sal_memset(info, 0x0, sizeof(DNX_TMC_MULT_FABRIC_BE_CLASS_INFO));
  DNX_TMC_MULT_FABRIC_PORT_INFO_clear(&(info->be_sch_port));
  info->weight = 0;
  DNX_SAND_MAGIC_NUM_SET;
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  DNX_TMC_MULT_FABRIC_BE_INFO_clear(
    DNX_SAND_OUT DNX_TMC_MULT_FABRIC_BE_INFO *info
  )
{
  uint32 ind;
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  sal_memset(info, 0x0, sizeof(DNX_TMC_MULT_FABRIC_BE_INFO));
  DNX_TMC_MULT_FABRIC_SHAPER_INFO_clear(&(info->be_shaper));
  info->wfq_enable = 0;
  for (ind=0; ind<DNX_TMC_MULT_FABRIC_NOF_BE_CLASSES; ++ind)
  {
    DNX_TMC_MULT_FABRIC_BE_CLASS_INFO_clear(&(info->be_sch_port[ind]));
  }
  DNX_SAND_MAGIC_NUM_SET;
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  DNX_TMC_MULT_FABRIC_GR_INFO_clear(
    DNX_SAND_OUT DNX_TMC_MULT_FABRIC_GR_INFO *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  sal_memset(info, 0x0, sizeof(DNX_TMC_MULT_FABRIC_GR_INFO));
  DNX_TMC_MULT_FABRIC_SHAPER_INFO_clear(&(info->gr_shaper));
  DNX_TMC_MULT_FABRIC_PORT_INFO_clear(&(info->gr_sch_port));
  DNX_SAND_MAGIC_NUM_SET;
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  DNX_TMC_MULT_FABRIC_INFO_clear(
    DNX_SAND_OUT DNX_TMC_MULT_FABRIC_INFO *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  sal_memset(info, 0x0, sizeof(DNX_TMC_MULT_FABRIC_INFO));
  DNX_TMC_MULT_FABRIC_GR_INFO_clear(&(info->guaranteed));
  DNX_TMC_MULT_FABRIC_BE_INFO_clear(&(info->best_effort));
  info->max_rate = 0;
  info->max_burst = 1;
  info->credits_via_sch = 0;
  DNX_SAND_MAGIC_NUM_SET;
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  DNX_TMC_MULT_FABRIC_ACTIVE_LINKS_clear(
    DNX_SAND_IN uint32 unit,
    DNX_SAND_OUT DNX_TMC_MULT_FABRIC_ACTIVE_LINKS *info
  )
{
  uint32 ind;
  DNX_SAND_INIT_ERROR_DEFINITIONS(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  sal_memset(info, 0x0, sizeof(DNX_TMC_MULT_FABRIC_ACTIVE_LINKS));
  for (ind=0; ind<DNX_TMC_MULT_FABRIC_NOF_UINT32S_FOR_ACTIVE_MC_LINKS(unit); ++ind)
  {
    info->bitmap[ind] = 0;
  }
  DNX_SAND_MAGIC_NUM_SET;
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  DNX_TMC_MULT_FABRIC_FLOW_CONTROL_MAP_clear(
    DNX_SAND_OUT DNX_TMC_MULT_FABRIC_FLOW_CONTROL_MAP *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  sal_memset(info, 0x0, sizeof(DNX_TMC_MULT_FABRIC_FLOW_CONTROL_MAP));
  
  info->bfmc0_lb_fc_map = DNX_TMC_MULT_FABRIC_FLOW_CONTROL_DONT_MAP;
  info->bfmc1_lb_fc_map = DNX_TMC_MULT_FABRIC_FLOW_CONTROL_DONT_MAP;
  info->bfmc2_lb_fc_map = DNX_TMC_MULT_FABRIC_FLOW_CONTROL_DONT_MAP;
  info->gfmc_lb_fc_map = DNX_TMC_MULT_FABRIC_FLOW_CONTROL_DONT_MAP;

  DNX_SAND_MAGIC_NUM_SET;

exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}


#if DNX_TMC_DEBUG_IS_LVL1

const char*
  DNX_TMC_MULT_FABRIC_CLS_RNG_to_string(
    DNX_SAND_IN DNX_TMC_MULT_FABRIC_CLS_RNG enum_val
  )
{
  const char* str = NULL;
  switch(enum_val)
  {
  case DNX_TMC_MULT_FABRIC_CLS_MIN:
    str = "MIN";
  break;

  case DNX_TMC_MULT_FABRIC_CLS_MAX:
    str = "MAX";
  break;

  default:
    str = " Unknown";
  }
  return str;
}

void
  DNX_TMC_MULT_FABRIC_PORT_INFO_print(
    DNX_SAND_IN DNX_TMC_MULT_FABRIC_PORT_INFO *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  LOG_CLI((BSL_META_U(unit,
                      "   Class_valid: %d, "),info->multicast_class_valid));
  LOG_CLI((BSL_META_U(unit,
                      "Port_id: %d\n\r"),info->mcast_class_port_id));
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  DNX_TMC_MULT_FABRIC_SHAPER_INFO_print(
    DNX_SAND_IN DNX_TMC_MULT_FABRIC_SHAPER_INFO *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  LOG_CLI((BSL_META_U(unit,
                      "Rate: %u, "),info->rate));
  LOG_CLI((BSL_META_U(unit,
                      "Max_burst: %u.\n\r"),info->max_burst));
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  DNX_TMC_MULT_FABRIC_BE_CLASS_INFO_print(
    DNX_SAND_IN DNX_TMC_MULT_FABRIC_BE_CLASS_INFO *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  LOG_CLI((BSL_META_U(unit,
                      "  Be_sch_port:\n\r")));
  DNX_TMC_MULT_FABRIC_PORT_INFO_print(&(info->be_sch_port));
  LOG_CLI((BSL_META_U(unit,
                      "  Weight: %u\n\r"),info->weight));
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  DNX_TMC_MULT_FABRIC_BE_INFO_print(
    DNX_SAND_IN DNX_TMC_MULT_FABRIC_BE_INFO *info
  )
{
  uint32 ind=0;
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  LOG_CLI((BSL_META_U(unit,
                      " Be_shaper: ")));
  DNX_TMC_MULT_FABRIC_SHAPER_INFO_print(&(info->be_shaper));
  LOG_CLI((BSL_META_U(unit,
                      " Wfq_enable: %d\n\r"),info->wfq_enable));
  for (ind=0; ind<DNX_TMC_MULT_FABRIC_NOF_BE_CLASSES; ++ind)
  {
    LOG_CLI((BSL_META_U(unit,
                        " Be_sch_port[%u]:\n\r"),ind));
    DNX_TMC_MULT_FABRIC_BE_CLASS_INFO_print(&(info->be_sch_port[ind]));
  }
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  DNX_TMC_MULT_FABRIC_GR_INFO_print(
    DNX_SAND_IN DNX_TMC_MULT_FABRIC_GR_INFO *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  LOG_CLI((BSL_META_U(unit,
                      " Gr_shaper: ")));
  DNX_TMC_MULT_FABRIC_SHAPER_INFO_print(&(info->gr_shaper));
  LOG_CLI((BSL_META_U(unit,
                      " Gr_sch_port:\n\r")));
  DNX_TMC_MULT_FABRIC_PORT_INFO_print(&(info->gr_sch_port));
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  DNX_TMC_MULT_FABRIC_INFO_print(
    DNX_SAND_IN DNX_TMC_MULT_FABRIC_INFO *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  LOG_CLI((BSL_META_U(unit,
                      "Guaranteed:\n\r")));
  DNX_TMC_MULT_FABRIC_GR_INFO_print(&(info->guaranteed));
  LOG_CLI((BSL_META_U(unit,
                      "Best_effort:\n\r")));
  DNX_TMC_MULT_FABRIC_BE_INFO_print(&(info->best_effort));
  LOG_CLI((BSL_META_U(unit,
                      "Max_rate:        %u[Kbps]\n\r"),info->max_rate));
  LOG_CLI((BSL_META_U(unit,
                      "Credits_via_sch: %d\n\r"),info->credits_via_sch));
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  DNX_TMC_MULT_FABRIC_ACTIVE_LINKS_print(
    DNX_SAND_IN uint32 unit,
    DNX_SAND_IN DNX_TMC_MULT_FABRIC_ACTIVE_LINKS *info
  )
{
  uint32 ind=0;
  DNX_SAND_INIT_ERROR_DEFINITIONS(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  for (ind=0; ind<DNX_TMC_MULT_FABRIC_NOF_UINT32S_FOR_ACTIVE_MC_LINKS(unit); ++ind)
  {
    LOG_CLI((BSL_META_U(unit,
                        "Bitmap[%u]: %u\n\r"),ind,info->bitmap[ind]));
  }
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

#endif /* DNX_TMC_DEBUG_IS_LVL1 */

/* } */

#include <soc/dnx/legacy/SAND/Utils/sand_footer.h>

