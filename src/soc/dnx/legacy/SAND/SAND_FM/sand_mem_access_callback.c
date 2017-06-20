/* $Id: sand_mem_access_callback.c,v 1.5 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/
/******************************************************************
*
* FILENAME:       dnx_sand_mem_access_callback.c
*
* AUTHOR:         Dune (S.Z.)
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



#include <soc/dnx/legacy/SAND/SAND_FM/sand_mem_access_callback.h>
#include <soc/dnx/legacy/SAND/SAND_FM/sand_mem_access.h>

#include <soc/dnx/legacy/SAND/Utils/sand_os_interface.h>
#include <soc/dnx/legacy/SAND/Utils/sand_integer_arithmetic.h>

#include <soc/dnx/legacy/SAND/Management/sand_error_code.h>
#include <soc/dnx/legacy/SAND/Management/sand_chip_descriptors.h>


/*****************************************************
 * See details in dnx_sand_mem_access_callback.h
 *****************************************************/
uint32
  dnx_sand_mem_read_callback(
    uint32 *buffer,
    uint32 size
  )
{
  DNX_SAND_MEM_READ_CALLBACK_STRUCT  *callback_struct;
  uint32 *new_buf;
  uint32 ex, no_err;
  int res;
  DNX_SAND_RET dnx_sand_ret;

  ex = 0;
  no_err = ex;
  new_buf = NULL;
  callback_struct = NULL;
  dnx_sand_ret = DNX_SAND_OK;
  /*
   */
  if (sizeof(DNX_SAND_MEM_READ_CALLBACK_STRUCT) != size)
  {
    dnx_sand_set_error_code_into_error_word(DNX_SAND_ERR_8001,&ex);
    dnx_sand_set_severity_into_error_word  (1, &ex);
    goto exit;
  }
  /*
   */
  callback_struct = (DNX_SAND_MEM_READ_CALLBACK_STRUCT  *)buffer;

  res = dnx_sand_take_chip_descriptor_mutex(callback_struct->unit);
  if ( DNX_SAND_OK != res )
  {
    if (DNX_SAND_ERR == res)
    {
      dnx_sand_set_error_code_into_error_word(DNX_SAND_SEM_TAKE_FAIL,  &ex);
      goto exit;
    }
    if ( 0 > res )
    {
      dnx_sand_set_error_code_into_error_word(DNX_SAND_ILLEGAL_DEVICE_ID,  &ex);
      goto exit;
    }
  }
  /*
   * OK, all check are done, we can proceed with the physical read
   */
  dnx_sand_ret = dnx_sand_mem_read(
               callback_struct->unit,
               callback_struct->result_ptr,
               callback_struct->offset,
               callback_struct->size,
               callback_struct->indirect
             );
  dnx_sand_set_error_code_into_error_word(dnx_sand_ret, &ex);

  /*
   */
  if ( DNX_SAND_OK != dnx_sand_give_chip_descriptor_mutex(callback_struct->unit) )
  {
    dnx_sand_set_error_code_into_error_word(DNX_SAND_SEM_TAKE_FAIL,&ex);
    goto exit;
  }
  /*
   * There are 2 conditions to call the user supplied callback:
   * 1 - There was an error
   * 2 - The value read from the chip differs from the value
   *     read at the last time (if it is valid)
   */
  if ( (ex != no_err) ||
      !(callback_struct->copy_of_result_is_valid) ||
       (callback_struct->copy_of_result_is_valid &&
        dnx_sand_os_memcmp(
          callback_struct->copy_of_last_result_ptr,
          callback_struct->result_ptr,
          DNX_SAND_MIN(callback_struct->size, DNX_SAND_CALLBACK_BUF_SIZE)
       ))
     )
  {
    if (callback_struct->user_callback_proc)
    {
      res = callback_struct->user_callback_proc(
                                callback_struct->user_callback_id,
                                callback_struct->result_ptr,
                                &new_buf,
                                callback_struct->size,
                                ex,
                                callback_struct->unit,
                                callback_struct->offset,
                                callback_struct->dnx_sand_tcm_callback_id,
                                0
                             );
      dnx_sand_os_memcpy(
        callback_struct->copy_of_last_result_ptr,
        callback_struct->result_ptr,
        DNX_SAND_MIN(callback_struct->size, DNX_SAND_CALLBACK_BUF_SIZE)
      );
      callback_struct->copy_of_result_is_valid = TRUE;
      if (new_buf)
      {
        callback_struct->result_ptr =  new_buf;
      }
      /*
       */
      if (DNX_SAND_OK != dnx_sand_get_error_code_from_error_word(res))
      {
        dnx_sand_set_error_code_into_error_word(DNX_SAND_ERR_8006,  &ex);
        callback_struct->copy_of_result_is_valid = FALSE;
        goto exit;
      }
    }
  }
  /*
   */
exit:
  if (ex != no_err)
  {
    dnx_sand_error_handler(ex, "error in dnx_sand_mem_read_callback()", 0,0,0,0,0,0);
  }
  else
  {
  }
  return ex;
}

