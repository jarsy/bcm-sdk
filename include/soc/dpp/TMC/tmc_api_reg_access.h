/* $Id: soc_tmcapi_reg_access.h,v 1.3 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/

#ifndef __SOC_TMC_API_REG_ACCESS_H_INCLUDED__
/* { */
#define __SOC_TMC_API_REG_ACCESS_H_INCLUDED__

/*************
 * INCLUDES  *
 *************/
/* { */
#include <soc/dpp/SAND/Utils/sand_header.h>
#include <soc/dpp/TMC/tmc_api_general.h>

/* } */

/*************
 * DEFINES   *
 *************/
/* { */

#define SOC_TMC_DEFAULT_INSTANCE   0xFF

/* } */

/*************
 *  MACROS   *
 *************/
/* { */

/* $Id: soc_tmcapi_reg_access.h,v 1.3 Broadcom SDK $
* calculate address from block base address, block index
* and the index of that block (can be non-zero
* for blocks with multiple instances.
*/

/*
* Soc_sand field manipulations based on tmc fields
* (must contain valid msb and lsb) {
*/
#define SOC_TMC_FLD_NOF_BITS(fld)                                               \
  ((fld).msb - (fld).lsb + 1)
#define SOC_TMC_FLD_LSB(fld)                                                    \
  (fld).lsb
#define SOC_TMC_FLD_MASK(fld)                                                   \
  (uint32)((SOC_SAND_BIT((fld).msb) - SOC_SAND_BIT((fld).lsb)) + SOC_SAND_BIT((fld).msb))
#define SOC_TMC_FLD_SHIFT(fld)                                                  \
  (fld).lsb
#define SOC_TMC_FLD_MAX(fld)                                                    \
  (SOC_TMC_FLD_MASK(fld)>>SOC_TMC_FLD_SHIFT(fld))
/*
* Take value and put it in its proper location within a 'long'
* register (other bits are zeroed).
*/
/*
* Get a value out of location within a 'long' register (and make sure it
* is not effected by bits outside its predefined mask).
*/

/*
* Take value from buff, and split it to two buffers (used when a field is split in HW
*/

/*
* Build value from two fields (used when a field is split in HW
*/

/*
* Soc_sand field manipulations }
*/

/*
* Within an array of registers with identical fields-
* get the index of the register to access.
*/

/*
* Within an array of registers with identical fields-
* get the index of the field (inside the register) to access.
*/

/*
* Set a variable of type SOC_TMC_REG_FIELD
*/
#define SOC_TMC_FLD_DEFINE(fld_var,fld_offset,fld_size) \
{                                                     \
  fld_var.lsb = fld_offset;                           \
  fld_var.msb = (fld_offset) + (fld_size) - 1;            \
}

/* } */

/*************
 * TYPE DEFS *
 *************/
/* { */
typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  Base address of the register in the device address space
   *  - relative to the device start address. Units: Bytes.
   */
  uint32 base;
  /*
   *  A device internal block (i.e. Network Interface) may
   *  have multiple instances with identical register
   *  structure, but starting at different offsets inside the
   *  device address space. This field is the offset between
   *  two adjacent instances of the device internal blocks of
   *  the same type. Units: Bytes.
   */
  uint16 step;

} SOC_TMC_REG_ADDR;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  Register address of the register this field belongs to.
   */
  SOC_TMC_REG_ADDR addr;
  /*
   *  Field Most Significant Bit in the register.
   */
  uint8 msb;
  /*
   *  Field Least Significant Bit in the register.
   */
  uint8 lsb;

} SOC_TMC_REG_FIELD;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  A structure containing the device register address
   *  information - base and step.
   */
  SOC_TMC_REG_ADDR addr;
  /*
   *  Value of the specified register.
   */
  uint32 val;

} SOC_TMC_REG_INFO;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  The expected value. The polling occurs until the polled
   *  field indicates the expected value, or until timeout.
   */
  uint32 expected_value;
  /*
   *  Number of busy-wait iterations to poll for the
   *  indication.
   */
  uint32 busy_wait_nof_iters;
  /*
   *  Number of timer-delayed iterations to poll for the
   *  indication.
   */
  uint32 timer_nof_iters;
  /*
   *  The minimal delay of each timer-delayed iteration, in
   *  milliseconds.
   */
  uint32 timer_delay_msec;

} SOC_TMC_POLL_INFO;

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

/*****************************************************
*NAME
*  soc_tmcfield_from_reg_get
*TYPE:
*  PROC
*DATE:
*  01/10/2007
*FUNCTION:
*  Gets field bits from an input buffer and puts them
*  in the output buffer
*INPUT:
*  SOC_SAND_IN  uint32          *reg_buffer,
*    Input buffer from which the function reads -
*    the register to read
*  SOC_SAND_IN  SOC_TMC_REG_FIELD  *field,
*    The field from which the bits are taken
*  SOC_SAND_IN  uint32          *fld_buffer
*    Output buffer to which the function writes -
*    the field to write.
*OUTPUT:
*****************************************************/
uint32
  soc_tmcfield_from_reg_get(
    SOC_SAND_IN  uint32          *reg_buffer,
    SOC_SAND_IN  SOC_TMC_REG_FIELD     *field,
    SOC_SAND_OUT uint32          *fld_buffer
  );

void
  SOC_TMC_REG_ADDR_clear(
    SOC_SAND_OUT SOC_TMC_REG_ADDR *info
  );

void
  SOC_TMC_REG_FIELD_clear(
    SOC_SAND_OUT SOC_TMC_REG_FIELD *info
  );

void
  SOC_TMC_REG_INFO_clear(
    SOC_SAND_OUT SOC_TMC_REG_INFO *info
  );

void
  SOC_TMC_POLL_INFO_clear(
    SOC_SAND_OUT SOC_TMC_POLL_INFO *info
  );

#if SOC_TMC_DEBUG_IS_LVL1

void
  SOC_TMC_REG_ADDR_print(
    SOC_SAND_IN  SOC_TMC_REG_ADDR *info
  );

void
  SOC_TMC_REG_FIELD_print(
    SOC_SAND_IN  SOC_TMC_REG_FIELD *info
  );

void
  SOC_TMC_REG_INFO_print(
    SOC_SAND_IN  SOC_TMC_REG_INFO *info
  );

void
  SOC_TMC_POLL_INFO_print(
    SOC_SAND_IN  SOC_TMC_POLL_INFO *info
  );

#endif /* SOC_TMC_DEBUG_IS_LVL1 */

/* } */

#include <soc/dpp/SAND/Utils/sand_footer.h>

/* } __SOC_TMC_API_REG_ACCESS_H_INCLUDED__*/
#endif
