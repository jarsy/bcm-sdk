#include <soc/mcm/memregs.h>
#if defined(BCM_88690_A0)
/* $Id: jer2_arad_chip_defines.c,v 1.8 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 * $
*/


#ifdef _ERR_MSG_MODULE_NAME
  #error "_ERR_MSG_MODULE_NAME redefined"
#endif

#define _ERR_MSG_MODULE_NAME BSL_SOC_CHIP
/*************
 * INCLUDES  *
 *************/
/* { */
#include <shared/bsl.h>
#include <shared/swstate/access/sw_state_access.h>

#include <soc/dnxc/legacy/error.h>
#include <soc/dnx/legacy/SAND/Utils/sand_header.h>

#include <soc/dnx/legacy/ARAD/arad_chip_defines.h>
#include <soc/dnx/legacy/ARAD/arad_general.h>
#include <soc/dnx/legacy/ARAD/arad_reg_access.h>
#include <soc/dnx/legacy/SAND/Utils/sand_integer_arithmetic.h>
#include <soc/dnx/legacy/SAND/Utils/sand_u64.h>

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


/* } */

/*************
 * FUNCTIONS *
 *************/
/* { */

uint32
  jer2_arad_chip_defines_init(int unit)
{
    uint8 is_allocated;
    soc_error_t rv;

    DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_SW_DB_INIT);


    if (!SOC_WARM_BOOT(unit)) {
        rv = sw_state_access[unit].dnx.soc.jer2_arad.tm.chip_definitions.is_allocated(unit, &is_allocated);
        DNX_SAND_SOC_CHECK_FUNC_RESULT(rv, 5, exit);
        if(!is_allocated) {
            rv = sw_state_access[unit].dnx.soc.jer2_arad.tm.chip_definitions.alloc(unit);
            DNX_SAND_SOC_CHECK_FUNC_RESULT(rv, 10, exit);
        }

        rv = sw_state_access[unit].dnx.soc.jer2_arad.tm.chip_definitions.ticks_per_sec.set(unit, JER2_ARAD_DFLT_TICKS_PER_SEC);
        DNX_SAND_SOC_CHECK_FUNC_RESULT(rv, 20, exit);
    }

exit:
    DNX_SAND_EXIT_AND_SEND_ERROR( "error in chip_defines_initialize()",0,0);
}



/*
 *  {  Clock Parameters.
 */

/*********************************************************************
*  This procedure is used to convert from time values to machine
*  clocks value.
*********************************************************************/
uint32
  jer2_arad_chip_time_to_ticks(
    DNX_SAND_IN  int       unit,
    DNX_SAND_IN  uint32        time_value,
    DNX_SAND_IN  uint8       is_nano,
    DNX_SAND_IN  uint32       result_granularity,
    DNX_SAND_IN  uint8       is_round_up,
    DNX_SAND_OUT uint32        *result
  )
{
  uint32
    granularity,
    ticks,
    kilo_ticks_per_sec,
    reminder;
  DNX_SAND_U64
    val1,
    val2;

  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_CHIP_TIME_TO_TICKS);
  DNX_SAND_CHECK_NULL_INPUT(result);

  kilo_ticks_per_sec = jer2_arad_chip_kilo_ticks_per_sec_get(unit);
  /*
   *    We use kilo-ticks-per-sec and not mega- for better accuracy.
   *  The granularity is multiplied by 1000 here to compensate.
   */
  granularity = (is_nano)?(result_granularity*1000000):(result_granularity*1000);

  if (result_granularity == 0)
  {
    DNX_SAND_SET_ERROR_CODE(DNX_SAND_DIV_BY_ZERO_ERR, 10, exit);
  }

  dnx_sand_u64_multiply_longs(time_value, kilo_ticks_per_sec, &val1);
  reminder = dnx_sand_u64_devide_u64_long(&val1, granularity, &val2);

  if (val2.arr[1] != 0)
  {
    DNX_SAND_SET_ERROR_CODE(DNX_SAND_VALUE_OUT_OF_RANGE_ERR, 12, exit);
  }

  ticks = val2.arr[0];

  if ((is_round_up) && (reminder != 0))
  {
    ticks += 1;
  }

  *result = ticks;

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_chip_time_to_ticks()",0,0);
}

/*********************************************************************
*  This procedure is used to convert from machine to
*  time values.
*********************************************************************/
uint32
  jer2_arad_ticks_to_time(
    DNX_SAND_IN  int       unit,
    DNX_SAND_IN  uint32        ticks_value,
    DNX_SAND_IN  uint8       is_nano,
    DNX_SAND_IN  uint32       result_granularity,
    DNX_SAND_OUT uint32        *result
  )
{
  uint32
    mega_ticks_per_sec;
  uint32
    granularity;
  DNX_SAND_U64
    val1,
    val2;

  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_CHIP_TICKS_TO_TIME);
  DNX_SAND_CHECK_NULL_INPUT(result);

  mega_ticks_per_sec = jer2_arad_chip_mega_ticks_per_sec_get(unit);

  granularity = (is_nano)?(result_granularity * 1000):result_granularity;

  dnx_sand_u64_multiply_longs(ticks_value, granularity, &val1);
  dnx_sand_u64_devide_u64_long(&val1, mega_ticks_per_sec, &val2);

  if (val2.arr[1] != 0)
  {
    DNX_SAND_SET_ERROR_CODE(DNX_SAND_VALUE_OUT_OF_RANGE_ERR, 12, exit);
  }

  *result =  val2.arr[0];

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_ticks_to_time()",0,0);
}

uint32
  jer2_arad_chip_ticks_per_sec_get(
    DNX_SAND_IN int unit
  )
{
    uint32 val;
    int rv;

    rv = sw_state_access[unit].dnx.soc.jer2_arad.tm.chip_definitions.ticks_per_sec.get(unit, &val);
    if (SOC_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_SOC_INIT, (BSL_META_U(unit, "Error at jer2_arad_chip_ticks_per_sec_get")));
    }

    return val;
}

void
  jer2_arad_chip_kilo_ticks_per_sec_set(
    DNX_SAND_IN int unit,
    DNX_SAND_IN uint32  clck_freq_khz
  )
{
    int rv;

    rv = sw_state_access[unit].dnx.soc.jer2_arad.tm.chip_definitions.ticks_per_sec.set(unit, clck_freq_khz * 1000);
    if (SOC_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_SOC_INIT, (BSL_META_U(unit, "Error at jer2_arad_chip_ticks_per_sec_set")));
    }
}

uint32
  jer2_arad_chip_kilo_ticks_per_sec_get(
    DNX_SAND_IN int unit
  )
{
  uint32
    tps;

  tps = jer2_arad_chip_ticks_per_sec_get(unit);
  return DNX_SAND_DIV_ROUND_DOWN(tps, 1000);
}

uint32
  jer2_arad_chip_mega_ticks_per_sec_get(
    DNX_SAND_IN int unit
  )
{
  uint32
    tps;

  tps = jer2_arad_chip_ticks_per_sec_get(unit);
  return DNX_SAND_DIV_ROUND_DOWN(tps, (1000*1000));
}

 /*
  *  END Clock Parameters. }
  */

/* } */


void
  jer2_arad_JER2_ARAD_REG_INFO_clear(
    DNX_SAND_OUT JER2_ARAD_REG_INFO *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  dnx_sand_os_memset(info, 0x0, sizeof(JER2_ARAD_REG_INFO));
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}


#include <soc/dnx/legacy/SAND/Utils/sand_footer.h>

#endif /* of #if defined(BCM_88690_A0) */
