/* $Id: soc_jer2_jer2_jer2_tmcapi_reg_access.h,v 1.3 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/

#ifndef __DNX_TMC_API_REG_ACCESS_H_INCLUDED__
/* { */
#define __DNX_TMC_API_REG_ACCESS_H_INCLUDED__

/*************
 * INCLUDES  *
 *************/
/* { */
#include <soc/dnx/legacy/SAND/Utils/sand_header.h>
#include <soc/dnx/legacy/TMC/tmc_api_general.h>

/* } */

/*************
 * DEFINES   *
 *************/
/* { */

#define DNX_TMC_DEFAULT_INSTANCE   0xFF

/* } */

/*************
 *  MACROS   *
 *************/
/* { */

/* $Id: soc_jer2_jer2_jer2_tmcapi_reg_access.h,v 1.3 Broadcom SDK $
* calculate address from block base address, block index
* and the index of that block (can be non-zero
* for blocks with multiple instances.
*/

/*
* Dnx_soc_sand field manipulations based on jer2_jer2_jer2_tmc fields
* (must contain valid msb and lsb) {
*/
#define DNX_TMC_FLD_NOF_BITS(fld)                                               \
  ((fld).msb - (fld).lsb + 1)
#define DNX_TMC_FLD_LSB(fld)                                                    \
  (fld).lsb
#define DNX_TMC_FLD_MASK(fld)                                                   \
  (uint32)((DNX_SAND_BIT((fld).msb) - DNX_SAND_BIT((fld).lsb)) + DNX_SAND_BIT((fld).msb))
#define DNX_TMC_FLD_SHIFT(fld)                                                  \
  (fld).lsb
#define DNX_TMC_FLD_MAX(fld)                                                    \
  (DNX_TMC_FLD_MASK(fld)>>DNX_TMC_FLD_SHIFT(fld))
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
* Dnx_soc_sand field manipulations }
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
* Set a variable of type DNX_TMC_REG_FIELD
*/
#define DNX_TMC_FLD_DEFINE(fld_var,fld_offset,fld_size) \
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
  DNX_SAND_MAGIC_NUM_VAR
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

} DNX_TMC_REG_ADDR;

typedef struct
{
  DNX_SAND_MAGIC_NUM_VAR
  /*
   *  Register address of the register this field belongs to.
   */
  DNX_TMC_REG_ADDR addr;
  /*
   *  Field Most Significant Bit in the register.
   */
  uint8 msb;
  /*
   *  Field Least Significant Bit in the register.
   */
  uint8 lsb;

} DNX_TMC_REG_FIELD;

typedef struct
{
  DNX_SAND_MAGIC_NUM_VAR
  /*
   *  A structure containing the device register address
   *  information - base and step.
   */
  DNX_TMC_REG_ADDR addr;
  /*
   *  Value of the specified register.
   */
  uint32 val;

} DNX_TMC_REG_INFO;

typedef struct
{
  DNX_SAND_MAGIC_NUM_VAR
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

} DNX_TMC_POLL_INFO;

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
*  soc_jer2_jer2_jer2_tmcfield_from_reg_get
*TYPE:
*  PROC
*DATE:
*  01/10/2007
*FUNCTION:
*  Gets field bits from an input buffer and puts them
*  in the output buffer
*INPUT:
*  DNX_SAND_IN  uint32          *reg_buffer,
*    Input buffer from which the function reads -
*    the register to read
*  DNX_SAND_IN  DNX_TMC_REG_FIELD  *field,
*    The field from which the bits are taken
*  DNX_SAND_IN  uint32          *fld_buffer
*    Output buffer to which the function writes -
*    the field to write.
*OUTPUT:
*****************************************************/
uint32
  soc_jer2_jer2_jer2_tmcfield_from_reg_get(
    DNX_SAND_IN  uint32          *reg_buffer,
    DNX_SAND_IN  DNX_TMC_REG_FIELD     *field,
    DNX_SAND_OUT uint32          *fld_buffer
  );

void
  DNX_TMC_REG_ADDR_clear(
    DNX_SAND_OUT DNX_TMC_REG_ADDR *info
  );

void
  DNX_TMC_REG_FIELD_clear(
    DNX_SAND_OUT DNX_TMC_REG_FIELD *info
  );

void
  DNX_TMC_REG_INFO_clear(
    DNX_SAND_OUT DNX_TMC_REG_INFO *info
  );

void
  DNX_TMC_POLL_INFO_clear(
    DNX_SAND_OUT DNX_TMC_POLL_INFO *info
  );

#if DNX_TMC_DEBUG_IS_LVL1

void
  DNX_TMC_REG_ADDR_print(
    DNX_SAND_IN  DNX_TMC_REG_ADDR *info
  );

void
  DNX_TMC_REG_FIELD_print(
    DNX_SAND_IN  DNX_TMC_REG_FIELD *info
  );

void
  DNX_TMC_REG_INFO_print(
    DNX_SAND_IN  DNX_TMC_REG_INFO *info
  );

void
  DNX_TMC_POLL_INFO_print(
    DNX_SAND_IN  DNX_TMC_POLL_INFO *info
  );

#endif /* DNX_TMC_DEBUG_IS_LVL1 */

/* } */

#include <soc/dnx/legacy/SAND/Utils/sand_footer.h>

/* } __DNX_TMC_API_REG_ACCESS_H_INCLUDED__*/
#endif
