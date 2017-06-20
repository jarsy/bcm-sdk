/* $Id: sand_device_management.c,v 1.9 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 * $
*/


#include <soc/dnx/legacy/SAND/Management/sand_device_management.h>
#include <soc/dnx/legacy/SAND/Management/sand_general_params.h>
#include <soc/dnx/legacy/SAND/Management/sand_error_code.h>
#include <soc/dnx/legacy/SAND/Management/sand_chip_descriptors.h>
#include <soc/dnx/legacy/SAND/Management/sand_callback_handles.h>
#include <soc/dnx/legacy/SAND/Management/sand_low_level.h>

#include <soc/dnx/legacy/SAND/SAND_FM/sand_chip_defines.h>
#include <soc/dnx/legacy/SAND/SAND_FM/sand_mem_access.h>

#include <soc/dnx/legacy/SAND/Utils/sand_os_interface.h>

uint8 Dnx_soc_register_with_id = FALSE;

/* $Id: sand_device_management.c,v 1.9 Broadcom SDK $
 * Forward declaration
 */
static DNX_SAND_RET
  dnx_sand_device_mngmnt_load_chip_ver(
    DNX_SAND_IN int       unit,
    DNX_SAND_IN uint32*     base_address,
    DNX_SAND_IN DNX_SAND_DEV_VER_INFO* ver_info
  );



void
  dnx_sand_clear_SAND_DEV_VER_INFO(
    DNX_SAND_OUT DNX_SAND_DEV_VER_INFO* ver_info
  )
{
  if(ver_info == NULL)
  {
    goto exit;
  }

  dnx_sand_os_memset(ver_info, 0x0, sizeof(DNX_SAND_DEV_VER_INFO));

  /*
   * Just set it to invalid offset.
   */
  ver_info->ver_reg_offset = 0xFFFFFFFF;

exit:
  return;
}


/*
 *
 */
DNX_SAND_RET
  dnx_sand_device_register_with_id(DNX_SAND_IN uint8 enable)
{
  Dnx_soc_register_with_id = enable;

  return DNX_SAND_OK;
}

DNX_SAND_RET
  dnx_sand_device_register(
    uint32                 *base_address,
    uint32                 mem_size, /*in bytes*/
    DNX_SAND_UNMASK_FUNC_PTR               unmask_func_ptr,
    DNX_SAND_IS_BIT_AUTO_CLEAR_FUNC_PTR    is_bit_ac_func_ptr,
    DNX_SAND_IS_DEVICE_INTERRUPTS_MASKED   is_dev_int_mask_func_ptr,
    DNX_SAND_GET_DEVICE_INTERRUPTS_MASK    get_dev_mask_func_ptr,
    DNX_SAND_MASK_SPECIFIC_INTERRUPT_CAUSE mask_specific_interrupt_cause_ptr,
    DNX_SAND_RESET_DEVICE_FUNC_PTR    reset_device_ptr,
    DNX_SAND_IS_OFFSET_READ_OR_WRITE_PROTECT_FUNC_PTR is_read_write_protect_ptr,
    DNX_SAND_DEV_VER_INFO             *ver_info,
    DNX_SAND_INDIRECT_MODULE          *indirect_module,
    uint32                 interrupt_mask_address,/* If the general mask bit do not exist ,
                                                          * give 0xFFFFFFFF (invalid address) as the
                                                          * address (as in DNX_SAND_FAP10M)
                                                          */
    int                  *unit_ptr
  )
{
  DNX_SAND_RET
    ex;
  int
    err,
    result,
    handle;
  uint32
    version_reg;

  ex     = DNX_SAND_OK;
  result = DNX_SAND_OK;
  err = 0;
  version_reg = 0;

  /*
   */
  if (!dnx_sand_general_get_driver_is_started())
  {
    ex = DNX_SAND_DRIVER_NOT_STARTED;
    err = 1;
    goto exit;
  }
  /*
   */
#ifdef DNX_SAND_CHECK_BASE_ADDRESS
  if (!base_address || !dnx_sand_is_long_aligned((uint32)base_address))
  {
    /*
     * base_address was null or not long aligned
     */
    ex = DNX_SAND_REGISTER_DEVICE_004;
    err = 2;
    goto exit;
  }
#endif
  if (!unit_ptr)
  {
    /*
     * user supplied buffer is NULL
     */
    ex = DNX_SAND_REGISTER_DEVICE_NULL_BUFFER;
    err = 3;
    goto exit;
  }

  if (DNX_SAND_OK != dnx_sand_array_mutex_take())
  {
    err = 50;
    goto exit;
  }

  /*
   */
  if (Dnx_soc_register_with_id)
  {
    handle = *unit_ptr;
  }
  else
  {
    handle = -1;
  }

  result = dnx_sand_add_chip_descriptor (
              base_address,
              mem_size,
              unmask_func_ptr,
              is_bit_ac_func_ptr,
              is_dev_int_mask_func_ptr,
              get_dev_mask_func_ptr,
              mask_specific_interrupt_cause_ptr,
              reset_device_ptr,
              is_read_write_protect_ptr,
              ver_info->chip_type,
              ver_info->logic_chip_type,
              handle
           );
  /*
   */
  if (0 > result)
  {
    switch (result)
    {
      case -1:  /* descriptors array / mutex are not initialized */
      {
        ex = DNX_SAND_DESC_ARRAY_NOT_INIT;
        err = 4;
        break;
      }
      case -2:  /* failed to take the array semaphore */
      {
        ex = DNX_SAND_SEM_TAKE_FAIL;
        err = 5;
        break;
      }
      case -3:  /* reached max size */
      {
        ex = DNX_SAND_REGISTER_DEVICE_001;
        err = 6;
        break;
      }
      case -6:  /* failed to create the device semaphore */
      {
        ex = DNX_SAND_SEM_CREATE_FAIL;
        err = 8;
        break;
      }
      default:
      {
        /* unknown error */
        ex = DNX_SAND_ADD_CHIP_DESCRIPTOR_ERR;
        err = 9;
        break;
      }
    }
    goto exit_semaphore;
  }

  /* Do not perform the following for CMIC-based devices */
  if (ver_info->cmic_skip_verif == FALSE) {
	  /*
	   * OK, let's check that the device is alive.
	   * No need to take it's mutex' cause no one can access
	   * that mutex before the device descriptor is valid
	   */
	  ex = dnx_sand_physical_read_from_chip(
			 &version_reg, base_address, ver_info->ver_reg_offset, DNX_SAND_REG_SIZE_BYTES
		   );
	  if (ex != DNX_SAND_OK)
	  {
		/* device was not accessable */
		err = 10;
		goto exit_semaphore;
	  }
	
	  ex = dnx_sand_check_chip_type_decide_big_endian(
			 result,
			 version_reg,
			 ver_info->chip_type_shift,
			 ver_info->chip_type_mask
		   );
	  if(ex != DNX_SAND_OK)
	  {
		/* Chip type as reported by chip itself does not match registered chip type. */
		ex = DNX_SAND_REGISTER_DEVICE_005;
		err = 11;
		goto exit_semaphore;
	  }
  }

  ex = dnx_sand_device_mngmnt_load_chip_ver(result, base_address, ver_info);
  if(ex != DNX_SAND_OK)
  {
	err = 20;
	goto exit_semaphore;
  }


  /* Set indirect information */
  if (dnx_sand_indirect_set_info(result,indirect_module))
  {
	ex = DNX_SAND_ERR;
	err = 30;
	goto exit_semaphore;
  }
  /*
   * Set interrupt mask address information.
   */
  if (dnx_sand_mem_interrupt_mask_address_set(result,interrupt_mask_address))
  {
	ex = DNX_SAND_ERR;
	err = 40;
	goto exit_semaphore;
  }
  /*
   */
  dnx_sand_set_chip_descriptor_valid(result);
  /*
   */
  *unit_ptr = result;
  /*
   */
exit_semaphore:

  if (DNX_SAND_OK != dnx_sand_array_mutex_give())
  {
	err = 52;
	goto exit;
  }
exit:

  DNX_SAND_ERROR_REPORT(ex,NULL,0,0,DNX_SAND_DEVICE_REGISTER,
        "General error in dnx_sand_device_register()",err,0,0,0,0,0) ;
  return ex;
}
/*
 */
DNX_SAND_RET
  dnx_sand_device_unregister(
    int unit
  )
{
  DNX_SAND_RET ex;
  int res;
  uint32 err;
  ex = DNX_SAND_OK;
  err = 0;
  /*
   */
  dnx_sand_check_driver_and_device(unit,&err);
  if (err)
  {
    ex = (DNX_SAND_RET)err;
    err = 1;
    goto exit;
  }
  /*
   * Take mutexes in the right order -> delta_list, device, rest of them
   */
  if (DNX_SAND_OK != dnx_sand_tcm_callback_delta_list_take_mutex())
  {
    ex = DNX_SAND_SEM_TAKE_FAIL;
    err = 2;
    goto exit;
  }
  res = dnx_sand_take_chip_descriptor_mutex(unit);
  if( DNX_SAND_OK != res)
  {
    if (DNX_SAND_ERR == res)
    {
      dnx_sand_tcm_callback_delta_list_give_mutex();
      ex = DNX_SAND_SEM_TAKE_FAIL;
      err = 3;
      goto exit;
    }
    if ( 0 > res )
    {
      dnx_sand_tcm_callback_delta_list_give_mutex();
      ex = DNX_SAND_ILLEGAL_DEVICE_ID;
      err = 4;
      goto exit;
    }
  }
  /*
   * semaphore taken successfully
   */
  if (dnx_sand_handles_unregister_all_device_handles(unit))
  {
    /* There is no need to chek the returned value of dnx_sand_give_chip_descriptor_mutex, we are in failure*/
    /* coverity[unchecked_value : FALSE]*/
    dnx_sand_give_chip_descriptor_mutex(unit);
    dnx_sand_tcm_callback_delta_list_give_mutex();
    ex = DNX_SAND_UNREGISTER_DEVICE_001;
    err = 5;
    goto exit;
  }
  /*
   * Clear device indirect information.
   */
  dnx_sand_indirect_clear_info(unit);
  /*
   * Clear device interrupt mask address information.
   */
  dnx_sand_mem_interrupt_mask_address_clear(unit);
  /*
   * clears all registered interrupt callbacks (already cleared by the unregister handles)
   * deletes the mutex (all waiting tasks get error return value).
   * that's why we're giving back the mutex only in case of an error.
   */
  if (dnx_sand_remove_chip_descriptor(unit))
  {
    ex = DNX_SAND_UNREGISTER_DEVICE_002;
    if (DNX_SAND_OK != dnx_sand_give_chip_descriptor_mutex(unit))
    {
      ex = DNX_SAND_SEM_GIVE_FAIL;
      err = 6;
      goto exit;
    }
    err = 7;
    goto exit;
  }
  /*
   * Give list mutex back
   */
  if (DNX_SAND_OK != dnx_sand_tcm_callback_delta_list_give_mutex())
  {
    ex = DNX_SAND_SEM_GIVE_FAIL;
    err = 8;
    goto exit;
  }
exit:
  DNX_SAND_ERROR_REPORT(ex,NULL,0,0,DNX_SAND_DEVICE_REGISTER,
        "General error in dnx_sand_device_unregister()",err,0,0,0,0,0);
  return ex;
}


/*****************************************************
*NAME
* dnx_sand_device_mngmnt_load_chip_ver
*TYPE:
*  PROC
*DATE:
*  18-Nov-04
*FUNCTION:
*  1. Read device version register.
*  2. Set it in the driver chip descriptor.
*INPUT:
*  DNX_SAND_DIRECT:
*    DNX_SAND_IN int       unit -
*      Driver device ID.
*    DNX_SAND_IN uint32*     base_address -
*      Device base address.
*    DNX_SAND_IN DNX_SAND_DEV_VER_INFO* ver_info -
*      Pointer to the device's version information structure.
*  DNX_SAND_INDIRECT:
*    None.
*OUTPUT:
*  DNX_SAND_DIRECT:
*    DNX_SAND_RET.
*  DNX_SAND_INDIRECT:
*    None
*REMARKS:
*  None.
*SEE ALSO:
*****************************************************/
static DNX_SAND_RET
  dnx_sand_device_mngmnt_load_chip_ver(
    DNX_SAND_IN int       unit,
    DNX_SAND_IN uint32*     base_address,
    DNX_SAND_IN DNX_SAND_DEV_VER_INFO* ver_info
  )
{
  DNX_SAND_RET
    dnx_sand_ret;
  uint32
    err,
    version_reg,
    dbg_ver,
    chip_ver;

  err = 0;
  version_reg = 0;
  dnx_sand_ret = DNX_SAND_OK;

  /*
   * 1. Read device version register
   */
  dnx_sand_ret = dnx_sand_physical_read_from_chip(
         &version_reg, base_address, ver_info->ver_reg_offset, sizeof(uint32)
       );
  if (dnx_sand_ret != DNX_SAND_OK)
  {
    /* device was not accessable */
    err = 10;
    goto exit;
  }

  dbg_ver  = DNX_SAND_GET_FLD_FROM_PLACE(version_reg, ver_info->dbg_ver_shift, ver_info->dbg_ver_mask);
  chip_ver = DNX_SAND_GET_FLD_FROM_PLACE(version_reg, ver_info->chip_ver_shift, ver_info->chip_ver_mask);


  /*
   * 2. Set it in the driver chip descriptor.
   */
  dnx_sand_ret = dnx_sand_set_chip_descriptor_ver_info(unit, dbg_ver, chip_ver);
  if (dnx_sand_ret != DNX_SAND_OK)
  {
    err = 20;
    goto exit;
  }


exit:
  DNX_SAND_ERROR_REPORT(dnx_sand_ret,NULL,0,0,DNX_SAND_DEVICE_MNGMNT_LOAD_CHIP_VER,
        "General error in dnx_sand_device_mngmnt_load_chip_ver()",err,unit,0,0,0,0);
  return dnx_sand_ret;
}


/*****************************************************
*NAME
* dnx_sand_get_device_type
*TYPE:
*  PROC
*DATE:
*  18-Nov-04
*FUNCTION:
*  Return the chip definitions:
*   Type and version.
*INPUT:
*  DNX_SAND_DIRECT:
*    uint32       unit -
*      Driver device ID.
*    DNX_SAND_DEVICE_TYPE   *chip_type -
*      DNX_SAND_FAP10M, DNX_SAND_FE200 ....
*    uint32      *chip_ver -
*      The chip revision. Each version has different
*      functionality.
*    uint32      *dbg_ver
*      In case of small changes in the device are
*      made between the revisions.
*  DNX_SAND_INDIRECT:
*    None.
*OUTPUT:
*  DNX_SAND_DIRECT:
*    DNX_SAND_RET.
*  DNX_SAND_INDIRECT:
*    None
*REMARKS:
*  None.
*SEE ALSO:
*****************************************************/

DNX_SAND_RET
  dnx_sand_get_device_type(
    int      unit,
    DNX_SAND_DEVICE_TYPE *chip_type,
    uint32    *chip_ver,
    uint32    *dbg_ver
  )
{
  DNX_SAND_RET
    dnx_sand_ret=DNX_SAND_OK;
  uint32
    err = 0;

  if(!chip_type || !chip_ver || !dbg_ver)
  {
    dnx_sand_ret = DNX_SAND_NULL_POINTER_ERR;
    err = 10;
    goto exit;
  }

  *chip_ver = dnx_sand_get_chip_descriptor_chip_ver(unit);
  *dbg_ver = dnx_sand_get_chip_descriptor_dbg_ver(unit);
  *chip_type = dnx_sand_get_chip_descriptor_logic_chip_type(unit);


exit:
  DNX_SAND_ERROR_REPORT(dnx_sand_ret,NULL,0,0,DNX_SAND_DEVICE_MNGMNT_GET_DEVICE_TYPE,
        "General error in dnx_sand_get_device_type()",err,unit,0,0,0,0);
  return dnx_sand_ret;
}
#ifdef DNX_SAND_DEBUG


/*
 * Printing utility.
 * Convert from enumerator to string.
 */
const char*
  dnx_sand_DEVICE_TYPE_to_str(
    DNX_SAND_IN DNX_SAND_DEVICE_TYPE dev_type
  )
{
  const char
    *str;

  switch(dev_type)
  {
  case DNX_SAND_DEV_FE200:
    str = "DNX_SAND_FE200";
    break;
  case DNX_SAND_DEV_TIMNA:
    str = "TIMNA";
    break;
  case DNX_SAND_DEV_FE600:
    str = "DNX_SAND_FE600";
    break;
  case DNX_SAND_DEV_FAP10M:
    str = "DNX_SAND_FAP10M";
    break;

  case DNX_SAND_DEV_FAP20V:
    str = "DNX_SAND_FAP20V";
    break;
  case DNX_SAND_DEV_FAP21V:
    str = "DNX_SAND_FAP21V";
    break;
  case DNX_SAND_DEV_PETRA:
    str = "SOC_PETRA";
    break;
  case DNX_SAND_DEV_JER2_ARAD:
    str = "JER2_ARAD";
    break;
  case DNX_SAND_DEV_FAP20M:
    str = "DNX_SAND_FAP20M";
    break;
  case DNX_SAND_DEV_T20E:
    str = "T20E";
    break;


  default:
    str = "dnx_sand_DEVICE_TYPE_to_str input parameters error (dev_type)";
  }

  return str;
}

#endif





