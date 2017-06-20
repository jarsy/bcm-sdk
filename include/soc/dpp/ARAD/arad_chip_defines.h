/* $Id: arad_chip_defines.h,v 1.7 Broadcom SDK $
* $Copyright: (c) 2016 Broadcom.
* Broadcom Proprietary and Confidential. All rights reserved.$
 * $Copyright
 * $
*/


#ifndef __ARAD_CHIP_DEFINES_INCLUDED__
/* { */
#define __ARAD_CHIP_DEFINES_INCLUDED__

/*************
 * INCLUDES  *
 *************/
/* { */
#include <soc/dpp/SAND/Utils/sand_header.h>

#include <soc/dpp/SAND/Utils/sand_framework.h>
#include <soc/dpp/TMC/tmc_api_reg_access.h>
#include <soc/dpp/dpp_config_defs.h>

/* } */

/*************
 * DEFINES   *
 *************/
/* { */

#define ARAD_EXPECTED_CHIP_TYPE     0x0
#define ARAD_EXPECTED_CHIP_VER_01   0x01
#define ARAD_EXPECTED_CHIP_VER      0x01

#define ARAD_TOTAL_SIZE_OF_REGS     0x1000 /* Not real */

/*
 * Number of local ports in a device
 */
#define ARAD_NOF_LOCAL_PORTS(unit) SOC_DPP_DEFS_GET(unit, nof_logical_ports)
#define ARAD_NOF_LOCAL_PORTS_MAX 512

/* Device Version managment { */

#define ARAD_REV_MATCH(val_actual, val_expected) \
  (SOC_SAND_NUM2BOOL((val_actual) == (val_expected)))

typedef enum
{
  /*
   *  Arad revision: A-0
   */
  ARAD_REV_A0=0,
  /*
   *  Arad revision: A-1
   */
  ARAD_REV_A1=1,
  /*
   */
  ARAD_NOF_REVS
}ARAD_REV;


/* Device Version managment } */

/*
 * Number of ticks the ARAD device clock ticks per second
 * (about a tick every 1.67 nano-seconds). This value should
 * be changed if different on the specific board.
 */
#define ARAD_DFLT_TICKS_PER_SEC               (600000000)


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

} __ATTRIBUTE_PACKED__ ARAD_CHIP_DEFINITIONS;


typedef struct
{
  uint16      msb;
  uint16      lsb;
} __ATTRIBUTE_PACKED__ ARAD_TBL_FIELD;

typedef SOC_TMC_REG_ADDR                                       ARAD_REG_ADDR;
typedef SOC_TMC_REG_FIELD                                      ARAD_REG_FIELD;
typedef SOC_TMC_REG_INFO                                       ARAD_REG_INFO;


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
 *  Read arad revision field.
 *  Notes:
 *  1. The function does not take device semaphore.
 *  2. The function does not check the register access success/fail indication.
 */
uint32
  arad_revision_fld_get(
    SOC_SAND_IN  int unit
  );

uint32
  arad_chip_defines_init(int unit);

/*
 *   Clock Parameters. {
 */

/*****************************************************
*NAME
*  arad_chip_time_to_ticks
*TYPE:
*  PROC
*DATE:
*  23/12/2007
*FUNCTION:
*  This procedure is used to convert from time values to machine
*  clocks value.
*INPUT:
*  SOC_SAND_DIRECT:
*     SOC_SAND_IN  uint32        time_value -
*       the value of the time to convert to ticks.
*     SOC_SAND_IN  uint8       is_nano -
*        whether the time is given in nanoseconds or in milliseconds
*     SOC_SAND_IN  uint32       result_granularity -
*        the 'resolution' of the result.
*     SOC_SAND_IN  uint8       is_round_up -
         If TRUE, the result is rounded up, otherwise - down
*  SOC_SAND_INDIRECT:
*    None.
*OUTPUT:
*  SOC_SAND_DIRECT:
*  SOC_SAND_INDIRECT:
*     SOC_SAND_OUT uint32        *result -
*         the result, the number of machine ticks for the given time.
*REMARKS:
*  None.
*SEE ALSO:
*****************************************************/
uint32
  arad_chip_time_to_ticks(
    SOC_SAND_IN  int       unit,
    SOC_SAND_IN  uint32        time_value,
    SOC_SAND_IN  uint8       is_nano,
    SOC_SAND_IN  uint32       result_granularity,
    SOC_SAND_IN  uint8       is_round_up,
    SOC_SAND_OUT uint32        *result
  );

/*****************************************************
*NAME
*  arad_ticks_to_time
*TYPE:
*  PROC
*DATE:
*  23/12/2007
*FUNCTION:
*  TThis procedure is used to convert from machine to
*  time values.
*INPUT:
*  SOC_SAND_DIRECT:
*     SOC_SAND_IN  uint32        time_value -
*       the value of the time to convert to ticks.
*     SOC_SAND_IN  uint8       is_nano -
*        whether the time is given in nanoseconds or in milliseconds
*     SOC_SAND_IN  uint32       result_granularity -
*        the 'resolution' of the result.
*     SOC_SAND_IN  uint8       is_round_up -
         If TRUE, the result is rounded up, otherwise - down
*  SOC_SAND_INDIRECT:
*    None.
*OUTPUT:
*  SOC_SAND_DIRECT:
*  SOC_SAND_INDIRECT:
*     SOC_SAND_OUT uint32        *result -
*         the result, the number of machine ticks for the given time.
*REMARKS:
*  None.
*SEE ALSO:
*****************************************************/
uint32
  arad_ticks_to_time(
    SOC_SAND_IN  int       unit,
    SOC_SAND_IN  uint32        ticks_value,
    SOC_SAND_IN  uint8       is_nano,
    SOC_SAND_IN  uint32       result_granularity,
    SOC_SAND_OUT uint32        *result
  );

uint32
  arad_chip_ticks_per_sec_get(
    SOC_SAND_IN int unit
  );

void
  arad_chip_kilo_ticks_per_sec_set(
    SOC_SAND_IN int unit,
    SOC_SAND_IN uint32  clck_freq_khz
  );

uint32
  arad_chip_kilo_ticks_per_sec_get(
    SOC_SAND_IN int unit
  );

uint32
  arad_chip_mega_ticks_per_sec_get(
    SOC_SAND_IN int unit
  );

/*
 *  END Clock Parameters. }
 */

/*
 *  Get Arad device revision
 */
ARAD_REV
  arad_revision_get(
    SOC_SAND_IN  int  unit
  );

const char*
  arad_ARAD_REV_to_string(
    SOC_SAND_IN ARAD_REV enum_val
  );

void
  arad_ARAD_REG_INFO_clear(
    SOC_SAND_OUT ARAD_REG_INFO *info
  );

/* } */


#include <soc/dpp/SAND/Utils/sand_footer.h>


/* } __ARAD_CHIP_DEFINES_INCLUDED__*/
#endif

