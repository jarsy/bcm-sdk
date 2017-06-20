/* $Id: jer2_arad_chip_defines.h,v 1.7 Broadcom SDK $
* $Copyright: (c) 2016 Broadcom.
* Broadcom Proprietary and Confidential. All rights reserved.$
 * $Copyright
 * $
*/


#ifndef __JER2_ARAD_CHIP_DEFINES_INCLUDED__
/* { */
#define __JER2_ARAD_CHIP_DEFINES_INCLUDED__

/*************
 * INCLUDES  *
 *************/
/* { */
#include <soc/dnx/legacy/SAND/Utils/sand_header.h>

#include <soc/dnx/legacy/SAND/Utils/sand_framework.h>
#include <soc/dnx/legacy/TMC/tmc_api_reg_access.h>
#include <soc/dnx/legacy/dnx_config_defs.h>

/* } */

/*************
 * DEFINES   *
 *************/
/* { */

#define JER2_ARAD_EXPECTED_CHIP_TYPE     0x0
#define JER2_ARAD_EXPECTED_CHIP_VER_01   0x01
#define JER2_ARAD_EXPECTED_CHIP_VER      0x01

#define JER2_ARAD_TOTAL_SIZE_OF_REGS     0x1000 /* Not real */

/*
 * Number of local ports in a device
 */
#define JER2_ARAD_NOF_LOCAL_PORTS(unit) SOC_DNX_DEFS_GET(unit, nof_logical_ports)
#define JER2_ARAD_NOF_LOCAL_PORTS_MAX 512

/* Device Version managment { */

#define JER2_ARAD_REV_MATCH(val_actual, val_expected) \
  (DNX_SAND_NUM2BOOL((val_actual) == (val_expected)))

typedef enum
{
  /*
   *  Arad revision: A-0
   */
  JER2_ARAD_REV_A0=0,
  /*
   *  Arad revision: A-1
   */
  JER2_ARAD_REV_A1=1,
  /*
   */
  JER2_ARAD_NOF_REVS
}JER2_ARAD_REV;


/* Device Version managment } */

/*
 * Number of ticks the JER2_ARAD device clock ticks per second
 * (about a tick every 1.67 nano-seconds). This value should
 * be changed if different on the specific board.
 */
#define JER2_ARAD_DFLT_TICKS_PER_SEC               (600000000)


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

typedef struct
{
  uint32
    ticks_per_sec;

} __ATTRIBUTE_PACKED__ JER2_ARAD_CHIP_DEFINITIONS;


typedef struct
{
  uint16      msb;
  uint16      lsb;
} __ATTRIBUTE_PACKED__ JER2_ARAD_TBL_FIELD;

typedef DNX_TMC_REG_ADDR                                       JER2_ARAD_REG_ADDR;
typedef DNX_TMC_REG_FIELD                                      JER2_ARAD_REG_FIELD;
typedef DNX_TMC_REG_INFO                                       JER2_ARAD_REG_INFO;


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

/*
 *  Read jer2_arad revision field.
 *  Notes:
 *  1. The function does not take device semaphore.
 *  2. The function does not check the register access success/fail indication.
 */
uint32
  jer2_arad_revision_fld_get(
    DNX_SAND_IN  int unit
  );

uint32
  jer2_arad_chip_defines_init(int unit);

/*
 *   Clock Parameters. {
 */

/*****************************************************
*NAME
*  jer2_arad_chip_time_to_ticks
*TYPE:
*  PROC
*DATE:
*  23/12/2007
*FUNCTION:
*  This procedure is used to convert from time values to machine
*  clocks value.
*INPUT:
*  DNX_SAND_DIRECT:
*     DNX_SAND_IN  uint32        time_value -
*       the value of the time to convert to ticks.
*     DNX_SAND_IN  uint8       is_nano -
*        whether the time is given in nanoseconds or in milliseconds
*     DNX_SAND_IN  uint32       result_granularity -
*        the 'resolution' of the result.
*     DNX_SAND_IN  uint8       is_round_up -
         If TRUE, the result is rounded up, otherwise - down
*  DNX_SAND_INDIRECT:
*    None.
*OUTPUT:
*  DNX_SAND_DIRECT:
*  DNX_SAND_INDIRECT:
*     DNX_SAND_OUT uint32        *result -
*         the result, the number of machine ticks for the given time.
*REMARKS:
*  None.
*SEE ALSO:
*****************************************************/
uint32
  jer2_arad_chip_time_to_ticks(
    DNX_SAND_IN  int       unit,
    DNX_SAND_IN  uint32        time_value,
    DNX_SAND_IN  uint8       is_nano,
    DNX_SAND_IN  uint32       result_granularity,
    DNX_SAND_IN  uint8       is_round_up,
    DNX_SAND_OUT uint32        *result
  );

/*****************************************************
*NAME
*  jer2_arad_ticks_to_time
*TYPE:
*  PROC
*DATE:
*  23/12/2007
*FUNCTION:
*  TThis procedure is used to convert from machine to
*  time values.
*INPUT:
*  DNX_SAND_DIRECT:
*     DNX_SAND_IN  uint32        time_value -
*       the value of the time to convert to ticks.
*     DNX_SAND_IN  uint8       is_nano -
*        whether the time is given in nanoseconds or in milliseconds
*     DNX_SAND_IN  uint32       result_granularity -
*        the 'resolution' of the result.
*     DNX_SAND_IN  uint8       is_round_up -
         If TRUE, the result is rounded up, otherwise - down
*  DNX_SAND_INDIRECT:
*    None.
*OUTPUT:
*  DNX_SAND_DIRECT:
*  DNX_SAND_INDIRECT:
*     DNX_SAND_OUT uint32        *result -
*         the result, the number of machine ticks for the given time.
*REMARKS:
*  None.
*SEE ALSO:
*****************************************************/
uint32
  jer2_arad_ticks_to_time(
    DNX_SAND_IN  int       unit,
    DNX_SAND_IN  uint32        ticks_value,
    DNX_SAND_IN  uint8       is_nano,
    DNX_SAND_IN  uint32       result_granularity,
    DNX_SAND_OUT uint32        *result
  );

uint32
  jer2_arad_chip_ticks_per_sec_get(
    DNX_SAND_IN int unit
  );

void
  jer2_arad_chip_kilo_ticks_per_sec_set(
    DNX_SAND_IN int unit,
    DNX_SAND_IN uint32  clck_freq_khz
  );

uint32
  jer2_arad_chip_kilo_ticks_per_sec_get(
    DNX_SAND_IN int unit
  );

uint32
  jer2_arad_chip_mega_ticks_per_sec_get(
    DNX_SAND_IN int unit
  );

/*
 *  END Clock Parameters. }
 */

/*
 *  Get Arad device revision
 */
JER2_ARAD_REV
  jer2_arad_revision_get(
    DNX_SAND_IN  int  unit
  );

const char*
  jer2_arad_JER2_ARAD_REV_to_string(
    DNX_SAND_IN JER2_ARAD_REV enum_val
  );

void
  jer2_arad_JER2_ARAD_REG_INFO_clear(
    DNX_SAND_OUT JER2_ARAD_REG_INFO *info
  );

/* } */


#include <soc/dnx/legacy/SAND/Utils/sand_footer.h>


/* } __JER2_ARAD_CHIP_DEFINES_INCLUDED__*/
#endif

