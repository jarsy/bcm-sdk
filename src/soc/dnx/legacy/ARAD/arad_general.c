#include <soc/mcm/memregs.h>
#if defined(BCM_88690_A0)
/* $Id: jer2_arad_general.c,v 1.8 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/


#ifdef _ERR_MSG_MODULE_NAME
  #error "_ERR_MSG_MODULE_NAME redefined"
#endif

#define _ERR_MSG_MODULE_NAME BSL_SOC_COMMON

/*************
 * INCLUDES  *
 *************/
/* { */
#include <shared/bsl.h>
#include <soc/dnxc/legacy/error.h>
#include <soc/dnx/legacy/dnx_config_defs.h>
#include <soc/dnx/legacy/drv.h>
#include <soc/dnx/legacy/SAND/Utils/sand_header.h>
#include <soc/dnx/legacy/ARAD/arad_api_general.h>
#include <soc/dnx/legacy/ARAD/arad_general.h>
#include <soc/dnx/legacy/ARAD/arad_api_framework.h>
#include <soc/dnx/legacy/ARAD/arad_chip_defines.h>
#include <soc/dnx/legacy/ARAD/arad_chip_regs.h>
#include <soc/dnx/legacy/ARAD/arad_chip_tbls.h>
#include <soc/dnx/legacy/ARAD/arad_reg_access.h>
#include <soc/dnx/legacy/ARAD/arad_mgmt.h>

#include <soc/dnx/legacy/SAND/Management/sand_error_code.h>
#include <soc/dnx/legacy/SAND/Management/sand_chip_descriptors.h>
#include <soc/dnx/legacy/SAND/Management/sand_device_management.h>
#include <soc/dnx/legacy/SAND/Management/sand_low_level.h>
#include <soc/dnx/legacy/SAND/Utils/sand_os_interface.h>
#include <soc/dnx/legacy/SAND/Utils/sand_bitstream.h>
#include <soc/dnx/legacy/SAND/SAND_FM/sand_mem_access.h>
#include <soc/dnx/legacy/SAND/Utils/sand_conv.h>
#include <soc/dnx/legacy/SAND/Management/sand_low_level.h>
#include <soc/dnx/legacy/mbcm.h>

/* } */

/*************
 * DEFINES   *
 *************/
/* { */

/*
 * Numeric correction used in Fabric Multicast rate calculation
 */
#define JER2_ARAD_PA_FMC_RATE_DELTA_CONST(is_for_ips)               (102) /* Both for IPS and FMC */

  /* The constant is 7 only for Arad-B and for FMC, otherwise 102 */
#define JER2_ARAD_FMC_RATE_DELTA_CONST(is_for_ips)             ((is_for_ips == TRUE)? JER2_ARAD_PA_FMC_RATE_DELTA_CONST(is_for_ips) : 7)

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

typedef DNX_SAND_RET (*JER2_ARAD_SEND_MESSAGE_TO_QUEUE_FUNC)(DNX_SAND_IN  uint32 msg);

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


uint8
  jer2_arad_is_multicast_id_valid(
    DNX_SAND_IN  int                unit,
    DNX_SAND_IN  uint32                multicast_id
  )
{
  if (multicast_id <= JER2_ARAD_MAX_MC_ID(unit))
  {
    return TRUE;
  }
  return FALSE;
}

uint8
  jer2_arad_is_queue_valid(
    DNX_SAND_IN  int                unit,
    DNX_SAND_IN  uint32                queue
  )
{
/*
  uint32
    queue_max;
*/
  /*
  jer2_arad_mgmt_nof_queues_get(unit, &queue_max);
  */
  if (queue < SOC_DNX_DEFS_GET(unit, nof_queues))
  {
    return TRUE;
  }
  return FALSE;
}

uint8
  jer2_arad_is_flow_valid(
    DNX_SAND_IN  int                unit,
    DNX_SAND_IN  uint32                flow
  )
{
  uint8
    is_in_range;

  is_in_range = flow < (SOC_DNX_DEFS_GET(unit, nof_flows));

  return is_in_range;
}

/*
 *  Internal Rate to clock conversion.
 *  Used for rate configuration, e.g. IPS (IssMaxCrRate),
 *  FMC (FmcMaxCrRate), Guaranteed/Best Effort FMC (GfmcMaxCrRate/BfmcMaxCrRate)
 */
uint32
  jer2_arad_intern_rate2clock(
    DNX_SAND_IN  int unit,
    DNX_SAND_IN  uint32  rate_kbps,
    DNX_SAND_IN  uint8 is_for_ips,
    DNX_SAND_OUT uint32  *clk_interval
  )
{
  uint32
    res;
  DNX_SAND_RET
    ret = DNX_SAND_OK;
  uint32
    cr_size = 0,
    ticks_per_sec = 0,
    interval = 0;

  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_INTERN_RATE2CLOCK);

  DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(
    res,10,exit,JER2_ARAD_GET_ERR_TEXT_001,MBCM_DNX_DRIVER_CALL(unit, mbcm_dnx_mgmt_credit_worth_get, (unit, &cr_size))) ;

  ticks_per_sec = jer2_arad_chip_ticks_per_sec_get(unit);

  if (rate_kbps == 0)
  {
    *clk_interval = 0;
  }
  else
  {
    ret = dnx_sand_kbits_per_sec_to_clocks(
            rate_kbps,
            cr_size  * 8 /*clock resolution is 1/8*/,
            ticks_per_sec,
            &interval
          );
    DNX_SAND_CHECK_FUNC_RESULT(ret, 20, exit);

    *clk_interval = interval - JER2_ARAD_FMC_RATE_DELTA_CONST(is_for_ips);
  }

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_intern_rate2clock()",0,0);
}

/*
 *  Internal Rate to clock conversion.
 *  Used for rate configuration, e.g. IPS (IssMaxCrRate),
 *  FMC (FmcMaxCrRate), Guaranteed/Best Effort FMC (GfmcMaxCrRate/BfmcMaxCrRate)
 */
uint32
  jer2_arad_intern_clock2rate(
    DNX_SAND_IN  int unit,
    DNX_SAND_IN  uint32  clk_interval,
    DNX_SAND_IN  uint8 is_for_ips,
    DNX_SAND_OUT uint32  *rate_kbps
  )
{
  uint32
    res;
  DNX_SAND_RET
    ret = DNX_SAND_OK;
  uint32
    cr_size = 0,
    ticks_per_sec = 0,
    interval = 0;

  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_INTERN_CLOCK2RATE);

  DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(
    res,15,exit,JER2_ARAD_GET_ERR_TEXT_001,MBCM_DNX_DRIVER_CALL(unit, mbcm_dnx_mgmt_credit_worth_get, (unit, &cr_size))) ;

  ticks_per_sec = jer2_arad_chip_ticks_per_sec_get(unit);

  if (clk_interval == 0)
  {
    *rate_kbps = 0;
  }
  else
  {
    interval = (clk_interval + JER2_ARAD_FMC_RATE_DELTA_CONST(is_for_ips));

    ret = dnx_sand_clocks_to_kbits_per_sec(
            interval,
            cr_size * 8 /*clock resolution is 1/8*/,
            ticks_per_sec,
            rate_kbps
          );
    DNX_SAND_CHECK_FUNC_RESULT(ret, 20, exit);
  }

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_intern_clock2rate()",0,0);
}



/* } */

#include <soc/dnx/legacy/SAND/Utils/sand_footer.h>

#endif /* of #if defined(BCM_88690_A0) */
