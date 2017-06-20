/* $Id: sand_mem_access.c,v 1.8 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/




#include <shared/bsl.h>
#include <soc/dnx/legacy/drv.h>



#include <soc/dnx/legacy/SAND/SAND_FM/sand_mem_access.h>

#include <soc/dnx/legacy/SAND/Management/sand_low_level.h>
#include <soc/dnx/legacy/SAND/Management/sand_error_code.h>
#include <soc/dnx/legacy/SAND/Management/sand_chip_descriptors.h>
#include <soc/dnx/legacy/SAND/Utils/sand_os_interface.h>

#include <soc/dnx/legacy/SAND/SAND_FM/sand_indirect_access.h>
#include <soc/dnx/legacy/SAND/Management/sand_general_macros.h>

/* $Id: sand_mem_access.c,v 1.8 Broadcom SDK $
 * {  Interrupt_mask_address
 */
/*
 * This is the interrupt mask adrress,
 * it can not be adressed thrugh the 'dnx_sand_mem_write'
 * only through the logical write.
 */
uint32
  Soc_interrupt_mask_address_arr[DNX_SAND_MAX_DEVICE] ;

/*
 * Set the interrupt mask address of the device.
 */
DNX_SAND_RET
  dnx_sand_mem_interrupt_mask_address_set(
    DNX_SAND_IN int  unit,
    DNX_SAND_IN uint32 interrupt_mask_address
  )
{
  if (unit < DNX_SAND_MAX_DEVICE)
  {
    Soc_interrupt_mask_address_arr[unit] = interrupt_mask_address ;
  }
  return DNX_SAND_OK ;
}

/*
 * Get the interrupt mask address of the device.
 */
uint32
  dnx_sand_mem_interrupt_mask_address_get(
    DNX_SAND_IN int  unit
  )
{
  uint32 ret ;
  ret = 0xFFFFFFFF ;

  if (unit < DNX_SAND_MAX_DEVICE)
  {
    ret = Soc_interrupt_mask_address_arr[unit] ;
  }

  return ret ;
}

/*
 * Clears unit interrupt mask address information.
 */
DNX_SAND_RET
  dnx_sand_mem_interrupt_mask_address_clear(
    DNX_SAND_IN int  unit
  )
{
  DNX_SAND_RET
    ex ;
  ex = dnx_sand_mem_interrupt_mask_address_set(unit, 0xFFFFFFFF) ;
  DNX_SAND_ERROR_REPORT(ex,NULL,0,0,DNX_SAND_MEM_INTERRUPT_MASK_ADDRESS_CLEAR,
        "General error in dnx_sand_mem_interrupt_mask_address_clear()",0,0,0,0,0,0) ;
  return ex ;
}

/*
 * Clears all devices interrupt mask address information.
 */
DNX_SAND_RET
  dnx_sand_mem_interrupt_mask_address_clear_all(
    void
  )
{
  unsigned
    int
      device_i ;
  for (device_i=0 ; device_i<DNX_SAND_MAX_DEVICE ; device_i++)
  {
    dnx_sand_mem_interrupt_mask_address_clear(device_i) ;
  }
  return DNX_SAND_OK ;
}
/*
 * }
 */

/*****************************************************
*NAME
* dnx_sand_mem_read
*TYPE:
*  PROC
*DATE:
*  21/02/2007
*FUNCTION:
*  Write direct / indirect datea.
*INPUT:
*  DNX_SAND_DIRECT:
*    DNX_SAND_IN int  unit -
*    DNX_SAND_IN uint32 *result_ptr -
*      The value that was read.
*    DNX_SAND_IN uint32 offset -
*      Offset in device / indirect table
*    DNX_SAND_IN uint32  size -
*      Size of data to write
*    DNX_SAND_IN uint32  indirect -
*      TRUE:  Indirect access, and the offset also indicate
*             The indirect table to access.
*      FALSE: Direct access, the offset is the offset
*             in the core
*  DNX_SAND_INDIRECT:
*    None.
*OUTPUT:
*  DNX_SAND_DIRECT:
*    error indication
*  DNX_SAND_INDIRECT:
*    None.
*REMARKS:
*    None.
*SEE ALSO:
*****************************************************/
DNX_SAND_RET
  dnx_sand_mem_read(
    DNX_SAND_IN  int     unit,
    DNX_SAND_OUT uint32    *result_ptr,
    DNX_SAND_IN  uint32    offset,
    DNX_SAND_IN  uint32     size,
    DNX_SAND_IN  uint32     indirect
  )
{
  uint32
    err;
  DNX_SAND_RET
    ex;
  int
    res;

  ex           = DNX_SAND_OK;
  res          = DNX_SAND_OK;

  dnx_sand_initialize_error_word(DNX_SAND_MEM_READ,0,&err);
  dnx_sand_check_driver_and_device(unit, &err);
  ex = dnx_sand_get_error_code_from_error_word(err);
  if (ex != DNX_SAND_OK)
  {
    err = 1;
    goto exit;
  }
  /*
   * Check 'result_ptr' is not NULL
   */
  if (NULL == result_ptr)
  {
    err = 14;
    ex = DNX_SAND_NULL_POINTER_ERR;
    goto exit;
  }

  res = dnx_sand_take_chip_descriptor_mutex(unit) ;
  if (res)
  {
    if (0 < res)
    {
      ex = DNX_SAND_SEM_TAKE_FAIL;
      err = 4;
      goto exit;
    }
    if (0 > res)
    {
      ex = DNX_SAND_ILLEGAL_DEVICE_ID;
      err = 5;
      goto exit;
    }
  }
  /*
   * semaphore taken - go on with the actual job of writing
   */

   ex = dnx_sand_mem_read_unsafe(
          unit,
          result_ptr,
          offset,
          size,
          indirect
        );

  if (ex!=DNX_SAND_OK)
  {
    ex = DNX_SAND_MEM_READ_010 ;
    goto exit_semaphore;
   }

exit_semaphore:
   if (DNX_SAND_OK != dnx_sand_give_chip_descriptor_mutex(unit))
   {
     ex = DNX_SAND_SEM_GIVE_FAIL ;
     err = 6 ;
     goto exit ;
  }

exit:
  DNX_SAND_ERROR_REPORT(ex,NULL,0,0,DNX_SAND_MEM_READ,
        "General error in dnx_sand_mem_read()",err, offset, size, indirect, 0,0) ;
  return ex ;
}
/*****************************************************
*NAME
* dnx_sand_mem_read_unsafe
*TYPE:
*  PROC
*DATE:
*  01/SEP/2007
*FUNCTION:
*  Write direct / indirect data - unsafe procedure .
*INPUT:
*  DNX_SAND_DIRECT:
*    DNX_SAND_IN int  unit -
*    DNX_SAND_IN uint32 *result_ptr -
*      The value that was read.
*    DNX_SAND_IN uint32 offset -
*      Offset in device / indirect table
*    DNX_SAND_IN uint32  size -
*      Size of data to write
*    DNX_SAND_IN uint32  indirect -
*      TRUE:  Indirect access, and the offset also indicate
*             The indirect table to access.
*      FALSE: Direct access, the offset is the offset
*             in the core
*  DNX_SAND_INDIRECT:
*    None.
*OUTPUT:
*  DNX_SAND_DIRECT:
*    error indication
*  DNX_SAND_INDIRECT:
*    None.
*REMARKS:
*    None.
*SEE ALSO:
*****************************************************/
DNX_SAND_RET
  dnx_sand_mem_read_unsafe(
    DNX_SAND_IN  int     unit,
    DNX_SAND_OUT uint32    *result_ptr,
    DNX_SAND_IN  uint32    offset,
    DNX_SAND_IN  uint32     size,
    DNX_SAND_IN  uint32     indirect
  )
{


  uint32
    *base_address;
  uint32
    err;
  DNX_SAND_RET
    ex;
  DNX_SAND_RET
    res;

  ex = DNX_SAND_OK;
  res = DNX_SAND_OK;


  dnx_sand_initialize_error_word(DNX_SAND_MEM_READ_UNSAFE, 0, &err);

  ex = dnx_sand_get_error_code_from_error_word(err);

  if (ex != DNX_SAND_OK)
  {
    err = 1;
    goto exit;
  }

/*
   * Now a set of declared errors
   */
  if (FALSE == indirect)
  {
    /* Direct access*/
    if (!dnx_sand_is_long_aligned(offset))
    {
      ex = DNX_SAND_MEM_READ_US_001 ;
      err = 2;
      goto exit;
    }
    if (!dnx_sand_is_long_aligned(size))
    {
      ex = DNX_SAND_MEM_READ_US_002 ;
      err = 2;
      goto exit;
    }
    /*
     * Every device should register a method (during device registration
     * that verify that read access to this area is allowed)
     */
    if( dnx_sand_mem_check_read_write_protect_get() &&
        dnx_sand_chip_descriptor_get_is_read_write_protect_func(unit)
      )
    {
      if ( dnx_sand_chip_descriptor_get_is_read_write_protect_func(unit)(
             TRUE /*read*/, offset, size
           )
         )
      {
        ex = DNX_SAND_MEM_READ_US_003;
        err = 3;
        goto exit;
      }
    }
    /*
     * Currently black list of un-accesible addresses includes only the
     * general mask register of the interrupts, so lets make sure it is
     * not within this request.
     */
    if ((offset <= dnx_sand_mem_interrupt_mask_address_get(unit)) &&
         ((offset + (size>>2)) > (dnx_sand_mem_interrupt_mask_address_get(unit)))
       )
    {
      ex = DNX_SAND_MEM_READ_US_003;
      err = 3;
      goto exit;
    }
  }

  base_address = dnx_sand_get_chip_descriptor_base_addr(unit);
  if (indirect)
  {
    res = dnx_sand_indirect_read_from_chip(
            unit,
            result_ptr,
            offset,
            size,
            DNX_SAND_IND_GET_MODULE_BIT_FROM_OFFSET
          );
    if(DNX_SAND_OK != res)
    {
      ex = DNX_SAND_MEM_READ_US_005;
      err = 11;
      goto exit;
    }
  }
  else
  {
    res = dnx_sand_physical_read_from_chip(
            result_ptr,
            base_address,
            offset,
            size
          );

    if (DNX_SAND_OK != res)
    {
      ex = DNX_SAND_MEM_READ_US_006;
      err = 12;
      goto exit;
    }
  }

exit:
  DNX_SAND_ERROR_REPORT(ex,NULL,0,0,DNX_SAND_MEM_READ_UNSAFE,
    "General error in dnx_sand_mem_read_unsafe()",err, offset, size, indirect, 0,0) ;

  return res ;
}



/*****************************************************
*NAME
* dnx_sand_mem_write
*TYPE:
*  PROC
*DATE:
*  21/02/2007
*FUNCTION:
*  Write direct / indirect data.
*INPUT:
*  DNX_SAND_DIRECT:
*    DNX_SAND_IN int  unit -
*    DNX_SAND_IN uint32 *result_ptr -
*      The value to write.
*    DNX_SAND_IN uint32 offset -
*      Offset in device / indirect table
*    DNX_SAND_IN uint32  size -
*      Size of data to write
*    DNX_SAND_IN uint32  indirect -
*      TRUE:  Indirect access, and the offset also indicate
*             The indirect table to access.
*      FALSE: Direct access, the offset is the offset
*             in the core
*  DNX_SAND_INDIRECT:
*    None.
*OUTPUT:
*  DNX_SAND_DIRECT:
*    error indication
*  DNX_SAND_INDIRECT:
*    None.
*REMARKS:
*    None.
*SEE ALSO:
*****************************************************/
DNX_SAND_RET
  dnx_sand_mem_write(
    DNX_SAND_IN int  unit,
    DNX_SAND_IN uint32 *result_ptr,
    DNX_SAND_IN uint32 offset,
    DNX_SAND_IN uint32  size,
    DNX_SAND_IN uint32  indirect
  )
{

  uint32
    err;
  DNX_SAND_RET
    ex;
  int
    res;

  ex = DNX_SAND_OK;
  res = DNX_SAND_OK;

  dnx_sand_initialize_error_word(DNX_SAND_MEM_WRITE, 0, &err);

  dnx_sand_check_driver_and_device(unit, &err);

  ex = dnx_sand_get_error_code_from_error_word(err);

  if (ex != DNX_SAND_OK)
  {
    err = 1;
    goto exit;
  }

  res = dnx_sand_take_chip_descriptor_mutex(unit) ;

  if (res)
  {
    if (0 < res)
    {
      ex = DNX_SAND_SEM_TAKE_FAIL ;
      err = 2 ;
      goto exit ;
    }
    if (0 > res)
    {
      ex = DNX_SAND_ILLEGAL_DEVICE_ID ;
      err = 3 ;
      goto exit ;
    }
  }
  /*
   * semaphore taken - go on with the actual job of writing
   */

   ex = dnx_sand_mem_write_unsafe(
          unit,
          result_ptr,
          offset,
          size,
          indirect
        );


   if (ex!=DNX_SAND_OK)
   {
      ex = DNX_SAND_MEM_WRITE_010 ;
      goto exit_semaphore;
   }

exit_semaphore:
  if (DNX_SAND_OK != dnx_sand_give_chip_descriptor_mutex(unit))
  {
    ex = DNX_SAND_SEM_GIVE_FAIL ;
    err = 5;
    goto exit ;
  }

exit:
  DNX_SAND_ERROR_REPORT(ex,NULL,0,0,DNX_SAND_MEM_WRITE,
        "General error in dnx_sand_mem_write()",err, offset, size, indirect, 0,0) ;

  return ex ;
}

/*****************************************************
*NAME
* dnx_sand_mem_write_unsafe
*TYPE:
*  PROC
*DATE:
*  21/02/2007
*FUNCTION:
*  Write direct / indirect data - unsafe procedure.
*INPUT:
*  DNX_SAND_DIRECT:
*    DNX_SAND_IN int  unit -
*    DNX_SAND_IN uint32 *result_ptr -
*      The value to write.
*    DNX_SAND_IN uint32 offset -
*      Offset in device / indirect table
*    DNX_SAND_IN uint32  size -
*      Size of data to write
*    DNX_SAND_IN uint32  indirect -
*      TRUE:  Indirect access, and the offset also indicate
*             The indirect table to access.
*      FALSE: Direct access, the offset is the offset
*             in the core
*  DNX_SAND_INDIRECT:
*    None.
*OUTPUT:
*  DNX_SAND_DIRECT:
*    error indication
*  DNX_SAND_INDIRECT:
*    None.
*REMARKS:
*    None.
*SEE ALSO:
*****************************************************/


DNX_SAND_RET
  dnx_sand_mem_write_unsafe(
    DNX_SAND_IN int  unit,
    DNX_SAND_IN uint32 *result_ptr,
    DNX_SAND_IN uint32 offset,
    DNX_SAND_IN uint32  size,
    DNX_SAND_IN uint32  indirect
  )
{

  uint32 *base_address, err;
  DNX_SAND_RET ex;


  /**/
  ex = DNX_SAND_OK;

  dnx_sand_initialize_error_word(DNX_SAND_MEM_WRITE_UNSAFE, 0, &err);
  ex = dnx_sand_get_error_code_from_error_word(err);
  if (ex != DNX_SAND_OK)
  {
    err = 1;
    goto exit;
  }
  /*
   * Now a set of declared errors
   */
  if (FALSE == indirect)
  {    /* Direct access*/
    /*
     * direct access should be to long aligned offsets
     */
    if (!dnx_sand_is_long_aligned(offset))
    {
      ex = DNX_SAND_MEM_WRITE_US_001 ;
      err = 2 ;
      goto exit ;
    }
    /*
     * Every device should register a method (during device registration
     * that verify that read access to this area is allowed)
     */
    if( dnx_sand_mem_check_read_write_protect_get() &&
        dnx_sand_chip_descriptor_get_is_read_write_protect_func(unit)
      )
    {
      if ( dnx_sand_chip_descriptor_get_is_read_write_protect_func(unit)(
             FALSE /*write*/, offset, size
           )
         )
      {
        ex = DNX_SAND_MEM_WRITE_US_003;
        err = 3;
        goto exit;
      }
    }
    /*
     * Currently black list of un-accesible addresses includes only the
     * general mask register of the interrupts, so lets make sure it is
     * not within this request.
     */
    if (
     (offset <= dnx_sand_mem_interrupt_mask_address_get(unit)) &&
         ((offset + (size>>2)) > (dnx_sand_mem_interrupt_mask_address_get(unit)))
       )
    {
      ex = DNX_SAND_MEM_WRITE_US_003;
      err = 4;
      goto exit;
    }
  }
  /*
   */

  base_address = dnx_sand_get_chip_descriptor_base_addr(unit) ;

  if (indirect)
  {
    ex = dnx_sand_indirect_write_to_chip(
           unit,
           result_ptr,
           offset,
           size,
           DNX_SAND_IND_GET_MODULE_BIT_FROM_OFFSET
         );


    if(DNX_SAND_OK != ex)
    {
      ex = DNX_SAND_MEM_WRITE_US_005;
      err = 5;
      goto exit ;
    }
  }
  else
  {
    ex = dnx_sand_physical_write_to_chip(
          result_ptr,
          base_address,
          offset,
          size
         );

    if(DNX_SAND_OK != ex)
    {
      ex = DNX_SAND_MEM_WRITE_US_006 ;
      err = 6;
      goto exit ;
    }
  }
  /*
   */
exit:
  DNX_SAND_ERROR_REPORT(ex,NULL,0,0,DNX_SAND_MEM_WRITE_UNSAFE,
        "General error in dnx_sand_mem_write_unsafe()",err, offset, size, indirect, 0,0) ;

  return ex ;
}


/*
 * {
 * Field access function. Uses dnx_sand_mem_read/dnx_sand_mem_write to read/write/update
 * specific field in a register.
 */

/*****************************************************
*NAME
*  dnx_sand_read_field
*TYPE:
*  PROC
*DATE:
*  03-Feb-03
*FUNCTION:
*  Read specific field from a register.
*  Uses dnx_sand_mem_read to read specific field in a register.
*  Only in the direct memory.
*INPUT:
*  DNX_SAND_DIRECT:
*    DNX_SAND_IN  int  unit -
*       Dnx_soc_sand-driver device-identifier (retuned from XXXXX_register_device() .
*    DNX_SAND_IN  uint32 offset -
*       Offset in the device memory (direct region)
*    DNX_SAND_IN  uint32 shift -
*       start bit of the field in the register
*    DNX_SAND_IN  uint32 mask -
*       the field mask
*    DNX_SAND_OUT uint32* data -
*       loaded with field data
*  DNX_SAND_INDIRECT:
*    None.
*OUTPUT:
*  DNX_SAND_DIRECT:
*    DNX_SAND_RET -
*      See formatting rules in ERROR RETURN VALUES.
*  DNX_SAND_INDIRECT:
*    data
*REMARKS:
*  None.
*SEE ALSO:
*****************************************************/
DNX_SAND_RET
  dnx_sand_read_field(
    DNX_SAND_IN  int  unit,
    DNX_SAND_IN  uint32 offset,
    DNX_SAND_IN  uint32 shift,
    DNX_SAND_IN  uint32 mask,
    DNX_SAND_OUT uint32* data
  )
{
  DNX_SAND_RET
    dnx_sand_ret;
  uint32
    reg_val,
    exit_point;

  /*
   */
  dnx_sand_ret = DNX_SAND_OK;
  exit_point = 0;

  if(NULL == data)
  {
    dnx_sand_ret = DNX_SAND_NULL_POINTER_ERR;
    exit_point = 10;
    goto exit;
  }

  /*
   * take device semaphore
   */
  if (DNX_SAND_OK != dnx_sand_take_chip_descriptor_mutex(unit) )
  {
    dnx_sand_ret = DNX_SAND_SEM_TAKE_FAIL;
    exit_point = 20;
    goto exit;
  }

  /*
   * Get the whole register
   */
  dnx_sand_ret = dnx_sand_mem_read(unit,
                           &reg_val,
                           offset,
                           sizeof(uint32),
                           FALSE) ;
  if (dnx_sand_ret != DNX_SAND_OK)
  {
    exit_point = 30;
    goto exit_semaphore;
  }

  /*
   * get the needed field.
   */
  *data = DNX_SAND_GET_FLD_FROM_PLACE(reg_val, shift, mask) ;

exit_semaphore:
  /*
   * give device semaphore
   */
  if (DNX_SAND_OK != dnx_sand_give_chip_descriptor_mutex(unit) )
  {
    dnx_sand_ret = DNX_SAND_SEM_GIVE_FAIL;
    goto exit;
  }
exit:
  DNX_SAND_ERROR_REPORT(dnx_sand_ret,NULL,0,0,DNX_SAND_READ_FIELD,
        "General error in dnx_sand_read_field()",exit_point, unit, offset, shift, 0,0) ;
  return dnx_sand_ret;
}


/*
* Calling dnx_sand_read_field, and cast to int.
*/
DNX_SAND_RET
  dnx_sand_read_field_int(
    DNX_SAND_IN  int  unit,
    DNX_SAND_IN  uint32 offset,
    DNX_SAND_IN  uint32 shift,
    DNX_SAND_IN  uint32 mask,
    DNX_SAND_OUT uint32* data
  )
{
  DNX_SAND_RET
    dnx_sand_ret;
  uint32
    field;
  dnx_sand_ret = dnx_sand_read_field(
    unit,
    offset,
    shift,
    mask,
    &field
  );
  *data = (uint32)field;
  return dnx_sand_ret;
}

/*****************************************************
*NAME
*  dnx_sand_read_field_ex
*TYPE:
*  PROC
*DATE:
*  31-May-05
*FUNCTION:
*  Read specific field from a register.
*  Uses dnx_sand_mem_read to read a specific field in a register.
*  Supports both direct and indirect memory.
*INPUT:
*  DNX_SAND_DIRECT:
*    DNX_SAND_IN  int  unit -
*       Dnx_soc_sand-driver device-identifier (retuned from XXXXX_register_device() .
*    DNX_SAND_IN  uint32 offset -
*       Offset in the device memory (direct region)
*    DNX_SAND_IN  uint32 shift -
*       start bit of the field in the register
*    DNX_SAND_IN  uint32 mask -
*       the field mask
*    DNX_SAND_IN  uint32  indirect -
*      Flag. If non-zero then indirect access is required.
*      In this case, only the 30 ls bits of 'offset'
*      are used and are interpreted as 'module id' (four
*      bits: 27-30) and offset.
*    DNX_SAND_OUT uint32* data -
*       loaded with field data
*  DNX_SAND_INDIRECT:
*    None.
*OUTPUT:
*  DNX_SAND_DIRECT:
*    DNX_SAND_RET -
*      See formatting rules in ERROR RETURN VALUES.
*  DNX_SAND_INDIRECT:
*    data
*REMARKS:
*  None.
*SEE ALSO:
*****************************************************/
DNX_SAND_RET
  dnx_sand_read_field_ex(
    DNX_SAND_IN  int  unit,
    DNX_SAND_IN  uint32 offset,
    DNX_SAND_IN  uint32 shift,
    DNX_SAND_IN  uint32 mask,
    DNX_SAND_IN  uint32  indirect,
    DNX_SAND_OUT uint32* data
  )
{
  DNX_SAND_RET
    dnx_sand_ret;
  uint32
    reg_val,
    exit_point;

  /*
   */
  dnx_sand_ret = DNX_SAND_OK;
  exit_point = 0;

  if(NULL == data)
  {
    dnx_sand_ret = DNX_SAND_NULL_POINTER_ERR;
    exit_point = 10;
    goto exit;
  }

  /*
   * take device semaphore
   */
  if (DNX_SAND_OK != dnx_sand_take_chip_descriptor_mutex(unit) )
  {
    dnx_sand_ret = DNX_SAND_SEM_TAKE_FAIL;
    exit_point = 20;
    goto exit;
  }

  /*
   * Get the whole register
   */
  dnx_sand_ret = dnx_sand_mem_read(unit,
                           &reg_val,
                           offset,
                           sizeof(uint32),
                           indirect) ;
  if (dnx_sand_ret != DNX_SAND_OK)
  {
    exit_point = 30;
    goto exit_semaphore;
  }

  /*
   * get the needed field.
   */
  *data = DNX_SAND_GET_FLD_FROM_PLACE(reg_val, shift, mask) ;

exit_semaphore:
  /*
   * give device semaphore
   */
  if (DNX_SAND_OK != dnx_sand_give_chip_descriptor_mutex(unit) )
  {
    dnx_sand_ret = DNX_SAND_SEM_GIVE_FAIL;
    goto exit;
  }
exit:
  DNX_SAND_ERROR_REPORT(dnx_sand_ret,NULL,0,0,DNX_SAND_READ_FIELD,
        "General error in dnx_sand_read_field_ex()",exit_point, unit, offset, shift, 0,0) ;
  return dnx_sand_ret;
}

/*****************************************************
*NAME
*  dnx_sand_write_field
*TYPE:
*  PROC
*DATE:
*  03-Feb-03
*FUNCTION:
*  Write specific field from a register (read modify write).
*  Uses dnx_sand_mem_read/dnx_sand_mem_write to read - modify - write
*  specific field in a register.
*  Only in the direct memory.
*INPUT:
*  DNX_SAND_DIRECT:
*    DNX_SAND_IN  int  unit -
*       Dnx_soc_sand-driver device-identifier (retuned from XXXXX_register_device() .
*    DNX_SAND_IN  uint32 offset -
*       Offset in the device memory (direct region)
*    DNX_SAND_IN  uint32 shift -
*       start bit of the field in the register
*    DNX_SAND_IN  uint32 mask -
*       the field mask
*    DNX_SAND_IN uint32  data_to_write -
*       The field data
*  DNX_SAND_INDIRECT:
*    None.
*OUTPUT:
*  DNX_SAND_DIRECT:
*    DNX_SAND_RET -
*      See formatting rules in ERROR RETURN VALUES.
*  DNX_SAND_INDIRECT:
*    NON
*REMARKS:
*  None.
*SEE ALSO:
*****************************************************/
DNX_SAND_RET
  dnx_sand_write_field(
    DNX_SAND_IN  int  unit,
    DNX_SAND_IN  uint32 offset,
    DNX_SAND_IN  uint32 shift,
    DNX_SAND_IN  uint32 mask,
    DNX_SAND_IN  uint32 data_to_write
  )
{
  DNX_SAND_RET
    dnx_sand_ret;
  uint32
    exit_point;

  /*
   */
  dnx_sand_ret = DNX_SAND_OK;
  exit_point = 0;


  /*
   * take device semaphore
   */
  if (DNX_SAND_OK != dnx_sand_take_chip_descriptor_mutex(unit) )
  {
    dnx_sand_ret = DNX_SAND_SEM_TAKE_FAIL;
    exit_point = 20;
    goto exit;
  }

  dnx_sand_ret = dnx_sand_read_modify_write(
               dnx_sand_get_chip_descriptor_base_addr(unit),
               offset,
               shift,
               mask,
               data_to_write
             );
  if (dnx_sand_ret != DNX_SAND_OK)
  {
    exit_point = 40;
    goto exit_semaphore;
  }

exit_semaphore:
  /*
   * give device semaphore
   */
  if (DNX_SAND_OK != dnx_sand_give_chip_descriptor_mutex(unit) )
  {
    dnx_sand_ret = DNX_SAND_SEM_GIVE_FAIL;
    goto exit;
  }
exit:
  DNX_SAND_ERROR_REPORT(dnx_sand_ret,NULL,0,0,DNX_SAND_WRITE_FIELD,
        "General error in dnx_sand_read_field()",exit_point, unit, offset, shift, 0,0) ;
  return dnx_sand_ret;
}

/*****************************************************
*NAME
*  dnx_sand_write_field_ex
*TYPE:
*  PROC
*DATE:
*  31-May-05
*FUNCTION:
*  Write specific field from a register (read modify write).
*  Uses dnx_sand_mem_read/dnx_sand_mem_write or dnx_sand_indirect_read_modify_write
*  to read - modify - write a specific field in a register.
*  Supports both direct and indirect memory.
*INPUT:
*  DNX_SAND_DIRECT:
*    DNX_SAND_IN  int  unit -
*       Dnx_soc_sand-driver device-identifier (retuned from XXXXX_register_device() .
*    DNX_SAND_IN  uint32 offset -
*       Offset in the device memory (direct region)
*    DNX_SAND_IN  uint32 shift -
*       start bit of the field in the register
*    DNX_SAND_IN  uint32 mask -
*       the field mask
*    DNX_SAND_IN  uint32  indirect -
*      Flag. If non-zero then indirect access is required.
*      In this case, only the 30 ls bits of 'offset'
*      are used and are interpreted as 'module id' (four
*      bits: 27-30) and offset.
*    DNX_SAND_IN uint32  data_to_write -
*       The field data
*  DNX_SAND_INDIRECT:
*    None.
*OUTPUT:
*  DNX_SAND_DIRECT:
*    DNX_SAND_RET -
*      See formatting rules in ERROR RETURN VALUES.
*  DNX_SAND_INDIRECT:
*    NON
*REMARKS:
*  None.
*SEE ALSO:
*****************************************************/
DNX_SAND_RET
  dnx_sand_write_field_ex(
    DNX_SAND_IN  int  unit,
    DNX_SAND_IN  uint32 offset,
    DNX_SAND_IN  uint32 shift,
    DNX_SAND_IN  uint32 mask,
    DNX_SAND_IN  uint32  indirect,
    DNX_SAND_IN  uint32 data_to_write
  )
{
  DNX_SAND_RET
    dnx_sand_ret;
  uint32
    exit_point;

  /*
   */
  dnx_sand_ret = DNX_SAND_OK;
  exit_point = 0;


  /*
   * take device semaphore
   */
  if (DNX_SAND_OK != dnx_sand_take_chip_descriptor_mutex(unit) )
  {
    dnx_sand_ret = DNX_SAND_SEM_TAKE_FAIL;
    exit_point = 20;
    goto exit;
  }

  if (!indirect)
  {
    dnx_sand_ret = dnx_sand_read_modify_write(
                 dnx_sand_get_chip_descriptor_base_addr(unit),
                 offset,
                 shift,
                 mask,
                 data_to_write
               );
    if (dnx_sand_ret != DNX_SAND_OK)
    {
      exit_point = 30;
      goto exit_semaphore;
    }
  }
  else
  {
    dnx_sand_ret = dnx_sand_indirect_read_modify_write(
                 unit,
                 offset,
                 shift,
                 mask,
                 data_to_write
                 );
    if (dnx_sand_ret != DNX_SAND_OK)
    {
      exit_point = 40;
      goto exit_semaphore;
    }
  }

exit_semaphore:
  /*
   * give device semaphore
   */
  if (DNX_SAND_OK != dnx_sand_give_chip_descriptor_mutex(unit) )
  {
    dnx_sand_ret = DNX_SAND_SEM_GIVE_FAIL;
    goto exit;
  }
exit:
  DNX_SAND_ERROR_REPORT(dnx_sand_ret,NULL,0,0,DNX_SAND_WRITE_FIELD,
        "General error in dnx_sand_write_field_ex()",exit_point, unit, offset, shift, 0,0) ;
  return dnx_sand_ret;
}

/*
 * }
 */


/*
 * Check read/write protect mechanism
 * {
 */

uint32
  Dnx_soc_sand_mem_check_read_write_protect = TRUE;

/*
 * Get
 */
uint32
  dnx_sand_mem_check_read_write_protect_get(
    void
  )
{
  uint32
    read_write_protect;
  read_write_protect = Dnx_soc_sand_mem_check_read_write_protect;
  return read_write_protect;
}

/*
 * Get
 */
void
  dnx_sand_mem_check_read_write_protect_set(
    DNX_SAND_IN uint32 read_write_protect
  )
{
  Dnx_soc_sand_mem_check_read_write_protect = read_write_protect;
}

/*
 * }
 */
#if DNX_SAND_DEBUG
/*****************************************************
*NAME
*  dnx_sand_print_registers_array
*TYPE:
*  PROC
*DATE:
*  22-Dec-04
*FUNCTION:
*  Print an array of registers.
*INPUT:
*  DNX_SAND_DIRECT:
*    DNX_SAND_IN  int  unit -
*       Dnx_soc_sand-driver device-identifier (retuned from XXXXX_register_device() .
*    DNX_SAND_IN  uint32 address_array[ADDRESS_ARRAY_MAX_SIZE] -
*       Array of registers.
*    DNX_SAND_IN uint32  nof_address -
*       number of register.
*    DNX_SAND_IN uint32 print_options_bm -
*       Printing options bit field.
*      DNX_SAND_DONT_PRINT_ZEROS: if the data of the register is zero,
*              don't print it.
*      DNX_SAND_PRINT_BITS:  Print all the bits that equal to '1'.
*  DNX_SAND_INDIRECT:
*    None.
*OUTPUT:
*  DNX_SAND_DIRECT:
*    DNX_SAND_RET -
*      See formatting rules in ERROR RETURN VALUES.
*  DNX_SAND_INDIRECT:
*    NON
*REMARKS:
*  None.
*SEE ALSO:
*****************************************************/

DNX_SAND_RET
  dnx_sand_print_registers_array(
    DNX_SAND_IN int  unit,
    DNX_SAND_IN uint32 address_array[DNX_SAND_ADDRESS_ARRAY_MAX_SIZE],
    DNX_SAND_IN uint32 nof_address,
    DNX_SAND_IN uint32 print_options_bm
  )
{
  DNX_SAND_RET
    ret = DNX_SAND_OK;
  uint32
    address_i,
    bit_i;
  uint32
    address,
    data;

  if(nof_address > DNX_SAND_ADDRESS_ARRAY_MAX_SIZE)
  {
    LOG_CLI((BSL_META_U(unit,
                        "nof_address > DNX_SAND_ADDRESS_ARRAY_MAX_SIZE\n\r")));
    ret = DNX_SAND_ERR;
    goto exit;
  }
  LOG_CLI((BSL_META_U(unit,
                      "\n\r\n\r")));
  for(address_i = 0;address_i<nof_address;address_i++)
  {
    address = address_array[address_i];
    dnx_sand_mem_read(unit, &data, address*4, 4, FALSE);

    if((data != 0)                                ||
       !(print_options_bm & DNX_SAND_DONT_PRINT_ZEROS)
      )
    {
      LOG_CLI((BSL_META_U(unit,
                          "Address: 0x%x. Data: 0x%x"),
               address,
               data
               ));
      if(print_options_bm & DNX_SAND_PRINT_BITS)
      {
        LOG_CLI((BSL_META_U(unit,
                            " Bits:")));

        for(bit_i=0;bit_i<32;bit_i++)
        {
          if(data & DNX_SAND_BIT(bit_i))
          {
            LOG_CLI((BSL_META_U(unit,
                                "%d,"),bit_i));
          }
        }
      }
      LOG_CLI((BSL_META_U(unit,
                          ".\n\r")));
    }
  }
  LOG_CLI((BSL_META_U(unit,
                      "\n\r\n\r")));


exit:
  return ret;
}


/*****************************************************
*NAME
*  dnx_sand_print_block
*TYPE:
*  PROC
*DATE:
*  22-Dec-04
*FUNCTION:
*  Print an array of registers.
*INPUT:
*  DNX_SAND_DIRECT:
*    DNX_SAND_IN  int  unit -
*       Dnx_soc_sand-driver device-identifier (returned from XXXXX_register_device() .
*    DNX_SAND_IN  uint32 address_array[DNX_SAND_ADDRESS_ARRAY_MAX_SIZE] -
*       Array of registers.
*    DNX_SAND_IN uint32  nof_address -
*       number of register.
*    DNX_SAND_IN uint32 print_options_bm -
*       Printing options bit field.
*      DNX_SAND_DONT_PRINT_ZEROS: if the data of the register is zero,
*              don't print it.
*      DNX_SAND_PRINT_BITS:  Print all the bits that equal to '1'.
*  DNX_SAND_INDIRECT:
*    None.
*OUTPUT:
*  DNX_SAND_DIRECT:
*    DNX_SAND_RET -
*      See formatting rules in ERROR RETURN VALUES.
*  DNX_SAND_INDIRECT:
*    NON
*REMARKS:
*  None.
*SEE ALSO:
*****************************************************/

DNX_SAND_RET
  dnx_sand_print_block(
    DNX_SAND_IN int  unit,
    DNX_SAND_IN uint32 address_array[DNX_SAND_ADDRESS_ARRAY_MAX_SIZE],
    DNX_SAND_IN uint32 nof_address,
    DNX_SAND_IN uint32 print_options_bm
  )
{
  DNX_SAND_RET
    ret = DNX_SAND_OK;
  uint32
    address_i,
    bit_i,
    same_as_prev = FALSE;
  uint32
    address,
    data;

  if(nof_address > DNX_SAND_ADDRESS_ARRAY_MAX_SIZE)
  {
    LOG_CLI((BSL_META_U(unit,
                        "nof_address > DNX_SAND_ADDRESS_ARRAY_MAX_SIZE\n\r")));
    ret = DNX_SAND_ERR;
    goto exit;
  }
  for(address_i = 0;address_i<nof_address;address_i++)
  {
    address = address_i /* * 4 */;
    data    = address_array[address_i];

    if(print_options_bm & DNX_SAND_PRINT_RANGES)
    {
      /* no overrun.
       * In case that address_i is nof_address - 1, the condition address_i + 1 == nof_address wikll be true avoiding overrun
       */
      /* coverity[overrun-local:FALSE] */
      if((address_i + 1 == nof_address) ||
         (data != address_array[address_i + 1])
        )
      {
        same_as_prev = FALSE;
      }
      else
      {
        if(!same_as_prev)
        {
          same_as_prev = TRUE;
          if((data != 0)                                ||
             !(print_options_bm & DNX_SAND_DONT_PRINT_ZEROS)
            )
          {
            LOG_CLI((BSL_META_U(unit,
                                "  From 0x%04X To "),
                     address + DNX_SAND_GET_BITS_RANGE(print_options_bm, 
                     DNX_SAND_PRINT_START_ADDR_MSB, DNX_SAND_PRINT_START_ADDR_LSB)
                     ));
          }
        }
        if(address_i != (nof_address -1))
        {
          continue;
        }
      }
    }
    if((data != 0)                                      ||
       !(print_options_bm & DNX_SAND_DONT_PRINT_ZEROS)
      )
    {
      LOG_CLI((BSL_META_U(unit,
                          "  0x%04X: 0x%08X"),
               address + DNX_SAND_GET_BITS_RANGE(print_options_bm, 
               DNX_SAND_PRINT_START_ADDR_MSB, DNX_SAND_PRINT_START_ADDR_LSB),
               data
               ));
      if(print_options_bm & DNX_SAND_PRINT_BITS)
      {
        LOG_CLI((BSL_META_U(unit,
                            " Bits:")));

        for(bit_i=0;bit_i<32;bit_i++)
        {
          if(data & DNX_SAND_BIT(bit_i))
          {
            LOG_CLI((BSL_META_U(unit,
                                "%d,"),bit_i));
          }
        }
      }
      LOG_CLI((BSL_META_U(unit,
                          "\n\r")));
    }
  }
  LOG_CLI((BSL_META_U(unit,
                      "\n\r")));


exit:
  return ret;
}

/*******************************************************/
/* Get a device indirect table information,            */
/*  read it and print it                               */
/*******************************************************/
DNX_SAND_RET
  dnx_sand_print_indirect_tables(
    DNX_SAND_IN int             unit,
    DNX_SAND_IN DNX_SAND_INDIRECT_PRINT_INFO print_info[DNX_SAND_MAX_NOF_INDIRECT_TABLES],
    DNX_SAND_IN uint32            print_options_bm
  )
{
  uint32
    *regs_values = NULL,
    max_table_size=0,
    table_i;
  const char*
    proc_name = "dnx_sand_print_indirect_tables";
  DNX_SAND_RET
    dnx_sand_ret = DNX_SAND_OK;


  for(table_i=0;print_info[table_i].addr != (uint32)-1;table_i++)
  {
    if(max_table_size < print_info[table_i].size * 4)
    {
      max_table_size = print_info[table_i].size * 4;
    }
  }
  regs_values = dnx_sand_os_malloc(max_table_size,"regs_values print_indirect_tables");
  if(regs_values == NULL)
  {
    LOG_CLI((BSL_META_U(unit,
                        "%s - Failed to allocate memory: %x\n\r"), proc_name, max_table_size / 4));
    dnx_sand_ret = DNX_SAND_MALLOC_FAIL;
    goto exit;
  }

  for(table_i=0;print_info[table_i].addr != (uint32)-1;table_i++)
  {
    dnx_sand_ret = dnx_sand_mem_read(
                   unit,
                   regs_values,
                   print_info[table_i].addr,
                   print_info[table_i].size*4,
                   TRUE
                 );
    if(dnx_sand_ret != DNX_SAND_OK) {
        return dnx_sand_ret;
    }
    LOG_CLI((BSL_META_U(unit,
                        "%s (0x%08x):\n\r"),
             print_info[table_i].name,
             print_info[table_i].addr
             ));
    {

      dnx_sand_print_block(
        unit,
        regs_values,
        print_info[table_i].size,
        print_options_bm
      );
    }

  }
exit:
  if(NULL != regs_values)
  {
    dnx_sand_os_free(regs_values);
    regs_values = NULL;
  }
  return dnx_sand_ret;
}
#endif

