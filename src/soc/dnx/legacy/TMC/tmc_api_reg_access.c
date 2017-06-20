/* $Id: jer2_tmc_api_reg_access.c,v 1.6 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/
/******************************************************************
*
* FILENAME:       soc_jer2_tmcreg_access.c
*
* MODULE PREFIX:  soc_jer2_tmcreg_access
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

#include <soc/dnx/legacy/SAND/SAND_FM/sand_mem_access.h>
#include <soc/dnx/legacy/SAND/SAND_FM/sand_trigger.h>
#include <soc/dnx/legacy/SAND/Utils/sand_bitstream.h>
#include <soc/dnx/legacy/SAND/Utils/sand_integer_arithmetic.h>
#include <soc/dnx/legacy/SAND/Management/sand_error_code.h>
#include <soc/dnx/legacy/SAND/Utils/sand_os_interface.h>

#include <soc/dnx/legacy/TMC/tmc_api_reg_access.h>
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
/*****************************************************
*NAME
*  soc_jer2_tmcfield_from_reg_get
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
*REMARKS:
*  The fld_buffer is zerrowed before writing to it.
*  In case of a 32-bit buffer (single register),
*  the return value will not be dependant on the previous
*  value of the buffer. Otherwise - if the buffer size
*  is larger then the field size,
*  it should be zerrowed by the user.
*****************************************************/
uint32
  soc_jer2_tmcfield_from_reg_get(
    DNX_SAND_IN  uint32                                *reg_buffer,
    DNX_SAND_IN  DNX_TMC_REG_FIELD                           *field,
    DNX_SAND_OUT uint32                                *fld_buffer
  )
{

  uint32
    out_buff = 0,
    res = DNX_SAND_OK;

  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);

  DNX_SAND_CHECK_NULL_INPUT(reg_buffer);
  DNX_SAND_CHECK_NULL_INPUT(field);
  DNX_SAND_CHECK_NULL_INPUT(fld_buffer);

  res = dnx_sand_bitstream_get_any_field(
    reg_buffer,
    DNX_TMC_FLD_LSB(*field),
    DNX_TMC_FLD_NOF_BITS(*field),
    &out_buff
    );
  DNX_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  *fld_buffer = out_buff;

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in soc_jer2_tmcfield_from_reg_get",0,0);
}

/*****************************************************
*NAME
*  soc_jer2_tmcfield_from_reg_set
*TYPE:
*  PROC
*DATE:
*  01/10/2007
*FUNCTION:
*  Sets field bits in an output buffer after read from
*  an input buffer.
*INPUT:
*  DNX_SAND_IN  uint32          *fld_buffer,
*    Input buffer from which the function reads -
*    the field to set.
*  DNX_SAND_IN  DNX_TMC_REG_FIELD  *field,
*    The field from which the bits are taken
*  DNX_SAND_IN  uint32          *reg_buffer
*    Output buffer to which the function writes -
*    the register to set with the field
*OUTPUT:
* None.
*REMARKS:
* Only the relevant bits in the output reg_buffer are changed -
* the rest is left as is.
*
*****************************************************/
uint32
  soc_jer2_tmcfield_from_reg_set(
    DNX_SAND_IN  uint32                                *fld_buffer,
    DNX_SAND_IN  DNX_TMC_REG_FIELD                           *field,
    DNX_SAND_OUT uint32                                *reg_buffer
  )
{
  uint32
    res = DNX_SAND_OK;

  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);

  DNX_SAND_CHECK_NULL_INPUT(fld_buffer);
  DNX_SAND_CHECK_NULL_INPUT(field);
  DNX_SAND_CHECK_NULL_INPUT(reg_buffer);

  res = dnx_sand_bitstream_set_any_field(
    fld_buffer,
    DNX_TMC_FLD_LSB(*field),
    DNX_TMC_FLD_NOF_BITS(*field),
    reg_buffer
    );
  DNX_SAND_CHECK_FUNC_RESULT(res, 10, exit);

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in soc_jer2_tmcfield_from_reg_get",0,0);
}

void
  DNX_TMC_REG_ADDR_clear(
    DNX_SAND_OUT DNX_TMC_REG_ADDR *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  sal_memset(info, 0x0, sizeof(DNX_TMC_REG_ADDR));
  info->base = 0;
  info->step = 0;
  DNX_SAND_MAGIC_NUM_SET;
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  DNX_TMC_REG_FIELD_clear(
    DNX_SAND_OUT DNX_TMC_REG_FIELD *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  sal_memset(info, 0x0, sizeof(DNX_TMC_REG_FIELD));
  DNX_TMC_REG_ADDR_clear(&(info->addr));
  info->msb = 0;
  info->lsb = 0;
  DNX_SAND_MAGIC_NUM_SET;
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  DNX_TMC_REG_INFO_clear(
    DNX_SAND_OUT DNX_TMC_REG_INFO *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  sal_memset(info, 0x0, sizeof(DNX_TMC_REG_INFO));
  DNX_TMC_REG_ADDR_clear(&(info->addr));
  info->val = 0;
  DNX_SAND_MAGIC_NUM_SET;
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  DNX_TMC_POLL_INFO_clear(
    DNX_SAND_OUT DNX_TMC_POLL_INFO *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  sal_memset(info, 0x0, sizeof(DNX_TMC_POLL_INFO));
  info->expected_value = 0;
  info->busy_wait_nof_iters = 0;
  info->timer_nof_iters = 0;
  info->timer_delay_msec = 0;
  DNX_SAND_MAGIC_NUM_SET;
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

#if DNX_TMC_DEBUG_IS_LVL1

void
  DNX_TMC_REG_ADDR_print(
    DNX_SAND_IN  DNX_TMC_REG_ADDR *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  LOG_CLI((BSL_META_U(unit,
                      "base: %u[Bytes]\n\r"),info->base));
  LOG_CLI((BSL_META_U(unit,
                      "step: %u[Bytes]\n\r"),info->step));
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  DNX_TMC_REG_FIELD_print(
    DNX_SAND_IN  DNX_TMC_REG_FIELD *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  LOG_CLI((BSL_META_U(unit,
                      "addr:")));
  DNX_TMC_REG_ADDR_print(&(info->addr));
  LOG_CLI((BSL_META_U(unit,
                      "msb: %u\n\r"), info->msb));
  LOG_CLI((BSL_META_U(unit,
                      "lsb: %u\n\r"), info->lsb));
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  DNX_TMC_REG_INFO_print(
    DNX_SAND_IN  DNX_TMC_REG_INFO *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  LOG_CLI((BSL_META_U(unit,
                      "addr:")));
  DNX_TMC_REG_ADDR_print(&(info->addr));
  LOG_CLI((BSL_META_U(unit,
                      "val: %u\n\r"),info->val));
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  DNX_TMC_POLL_INFO_print(
    DNX_SAND_IN  DNX_TMC_POLL_INFO *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  LOG_CLI((BSL_META_U(unit,
                      "expected_value: %u\n\r"),info->expected_value));
  LOG_CLI((BSL_META_U(unit,
                      "busy_wait_nof_iters: %u\n\r"),info->busy_wait_nof_iters));
  LOG_CLI((BSL_META_U(unit,
                      "timer_nof_iters: %u\n\r"),info->timer_nof_iters));
  LOG_CLI((BSL_META_U(unit,
                      "timer_delay_msec: %u\n\r"),info->timer_delay_msec));
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

#endif /* DNX_TMC_DEBUG_IS_LVL1 */

/* } */

#include <soc/dnx/legacy/SAND/Utils/sand_footer.h>

