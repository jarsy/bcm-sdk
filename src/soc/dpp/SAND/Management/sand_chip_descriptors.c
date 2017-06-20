/* $Id: sand_chip_descriptors.c,v 1.11 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 * $
*/
/* $Id: sand_chip_descriptors.c,v 1.11 Broadcom SDK $
 */


#include <shared/bsl.h>

#include <soc/dpp/SAND/Management/sand_error_code.h>
#include <soc/dpp/SAND/SAND_FM/sand_chip_defines.h>
#include <soc/dpp/SAND/Management/sand_callback_handles.h>
#include <soc/dpp/SAND/Utils/sand_bitstream.h>
#include <soc/dpp/SAND/Utils/sand_os_interface.h>
#include <soc/dpp/SAND/Management/sand_chip_descriptors.h>
#include <soc/dpp/SAND/Management/sand_device_management.h>
#include <soc/dpp/SAND/Utils/sand_tcm.h>


#define FE200_REV_B_AS_REV_A  0

/*
 * The chips that are handled by this driver
 * and the Array Size
 */
  SOC_SAND_DEVICE_DESC
    *Soc_sand_chip_descriptors = NULL ;
/*
 */
  uint32
    Soc_sand_array_size        = 0 ;
/*
 * a Mutex to guard all register / unregister actions on the array
 * (they must be atomic, cause we can't allow registering the same chip
 * twice, etc).
 */
static
  sal_mutex_t
    Soc_sand_array_mutex       = 0 ;
/*
 * Soc_sand_up_counter implemenmts the logic of each descriptor magic word.
 * Its purpose is to provide us with a unique identifier, so it does it
 * by always advancing a counter when soc_get_magic is called.
 */
  uint32 Soc_sand_up_counter = 0 ;
/*
 */
STATIC
  uint32
    soc_get_magic(
      void
    )
{
  Soc_sand_up_counter ++ ;
  return Soc_sand_up_counter ;
}

SOC_SAND_RET
  soc_sand_array_mutex_take()
{
  return soc_sand_os_mutex_take(Soc_sand_array_mutex, SOC_SAND_INFINITE_TIMEOUT);
}

SOC_SAND_RET
  soc_sand_array_mutex_give()
{
  return soc_sand_os_mutex_give(Soc_sand_array_mutex);
}

/*
 */
/*****************************************************
*NAME:
* soc_sand_clear_chip_descriptor
*DATE:
* 27/OCT/2002
*FUNCTION:
* clearing one chip descriptor
* This is useful by init, and when unregistering one device.
*INPUT:
*  SOC_SAND_DIRECT:
*     uint32 index -
*       index in the array of the descriptor to clear
*     uint32 init_flag
*       0 - this method is called for init
*       1 - this method is called for romoval
*  SOC_SAND_INDIRECT:
*OUTPUT:
*  SOC_SAND_DIRECT:
*    Non-zero in case of an error
*  SOC_SAND_INDIRECT:
*REMARKS:
* static (private, local) method used by init_descriptors(),
* and by remove_descriptor(). The method can be called on
* a valid entry or on an empty entry.
* Mutex issue: it is assumed that if the device's mutex existed
* before (init_flag = TRUE) clearing this descriptor,
* then the upper layer took it, an it is our responsibility
* to delete it.
*SEE ALSO:
*****************************************************/
STATIC
  SOC_SAND_RET
    soc_sand_clear_chip_descriptor(
      uint32 index,
      uint32 init_flag
  )
{
  int
    iii ;
  SOC_SAND_RET
    ex ;
  uint32
      err = 0 ;
  /*
   * clear pointres, and params
   */
  Soc_sand_chip_descriptors[index].base_addr            = 0 ;
  Soc_sand_chip_descriptors[index].chip_type            = 0 ;
  Soc_sand_chip_descriptors[index].dbg_ver              = 0 ;
  Soc_sand_chip_descriptors[index].chip_ver             = 0 ;
  /*
   * clear interrupt bitstream (cause_id)
   */
  soc_sand_bitstream_clear(Soc_sand_chip_descriptors[index].interrupt_bitstream,
                        SIZE_OF_BITSTRAEM_IN_UINT32S
                      ) ;
  /*
   * clear interrupt callback array
   */
  for (iii = 0 ; iii < MAX_INTERRUPT_CAUSE ; ++iii)
  {
    Soc_sand_chip_descriptors[index].interrupt_callback_array[iii].func   = NULL ;
    /*
     * free the callback buffer
     */
    if (init_flag && Soc_sand_chip_descriptors[index].interrupt_callback_array[iii].buffer)
    {
      soc_sand_os_free(
        Soc_sand_chip_descriptors[index].interrupt_callback_array[iii].buffer
      ) ;
    }
    /*
     */
    Soc_sand_chip_descriptors[index].interrupt_callback_array[iii].buffer = NULL ;
    Soc_sand_chip_descriptors[index].interrupt_callback_array[iii].size   = 0 ;
  }
  Soc_sand_chip_descriptors[index].unmask_ptr                      = NULL ;
  Soc_sand_chip_descriptors[index].is_bit_auto_clear_ptr           = NULL ;
  Soc_sand_chip_descriptors[index].is_device_interrupts_masked_ptr = NULL ;
  Soc_sand_chip_descriptors[index].get_device_interrupts_mask_ptr  = NULL ;
  Soc_sand_chip_descriptors[index].device_interrupt_mask_counter   = 0;
  Soc_sand_chip_descriptors[index].device_is_between_isr_to_tcm    = FALSE;
  Soc_sand_chip_descriptors[index].magic = soc_get_magic() ;
  Soc_sand_chip_descriptors[index].device_at_init = FALSE ;
  Soc_sand_chip_descriptors[index].valid_word = 0 ;
  /*
   * delete chip mutex (if exist)
   * other tasks waiting on the chip mutex get error
   */
  if (init_flag)
  {
    soc_sand_os_mutex_give(Soc_sand_chip_descriptors[index].mutex_id);
    ex = soc_sand_os_mutex_delete(Soc_sand_chip_descriptors[index].mutex_id) ;
    if (ex != SOC_SAND_OK)
    {
      err = 1 ;
      goto exit ;
    }
  }
  Soc_sand_chip_descriptors[index].mutex_id      = 0 ;
  Soc_sand_chip_descriptors[index].mutex_owner   = 0 ;
  Soc_sand_chip_descriptors[index].mutex_counter = 0 ;
  ex = SOC_SAND_OK ;
exit:
  SOC_SAND_ERROR_REPORT(ex,NULL,0,0,SOC_SAND_CLEAR_CHIP_DESCRIPTOR,
        "General error in soc_sand_clear_chip_descriptor()",err,0,0,0,0,0) ;
  return ex ;
}
/*****************************************************
*NAME:
* soc_sand_init_chip_descriptors
*DATE:
* 27/OCT/2002
*FUNCTION:
* The initialization routine of the chip descriptors array
*INPUT:
*  SOC_SAND_DIRECT:
*    uint32 max_devices   -
*       number of devices to allocate space to
*  SOC_SAND_INDIRECT:
*   the array is initilized
*OUTPUT:
*  SOC_SAND_DIRECT:
*    Non-zero in case of an error
*  SOC_SAND_INDIRECT:
*REMARKS:
* allocates the array and its mutex, and then
* simply go over the array and call
* soc_sand_clear_chip_descriptor() for each descriptor
*SEE ALSO:
*****************************************************/
SOC_SAND_RET
  soc_sand_init_chip_descriptors(
    uint32 max_devices
  )
{
  unsigned
    int
      iii ;
  SOC_SAND_RET
    ex ;
  uint32
    err = 0 ;
  ex = SOC_SAND_ERR ;
  /*
   * lets check that the array was deleted before
   * we are starting it now
   */
  if (Soc_sand_chip_descriptors || Soc_sand_array_mutex)
  {
    err = 1 ;
    goto exit ;
  }
  /*
   * OK, time to allocate the new array, and the new mutex
   */
  Soc_sand_chip_descriptors = soc_sand_os_malloc((max_devices * sizeof(SOC_SAND_DEVICE_DESC)), "Soc_sand_chip_descriptors") ;
  if (!Soc_sand_chip_descriptors)
  {
    err = 2 ;
    goto exit ;
  }
  /*
   */
  Soc_sand_array_size = max_devices ;
  /*
   */
  Soc_sand_array_mutex = soc_sand_os_mutex_create() ;
  if (!Soc_sand_array_mutex)
  {
    err = 3 ;
    goto exit ;
  }
  if (SOC_SAND_OK != soc_sand_os_mutex_take(Soc_sand_array_mutex, SOC_SAND_INFINITE_TIMEOUT))
  {
    err = 4 ;
    goto exit ;
  }
  /*
   * go over the array and clear all descriptors
   */
   for (iii = 0 ; iii < Soc_sand_array_size ; ++iii)
   {
     soc_sand_clear_chip_descriptor(iii,FALSE) ;
   }
   /*
    */
   if (SOC_SAND_OK != soc_sand_os_mutex_give(Soc_sand_array_mutex))
   {
     err = 5 ;
     goto exit ;
   }
   ex = SOC_SAND_OK ;
exit:
  SOC_SAND_ERROR_REPORT(ex,NULL,0,0,SOC_SAND_INIT_CHIP_DESCRIPTORS,
        "General error in soc_sand_init_chip_descriptors()",err,0,0,0,0,0) ;
  return ex ;
}
/*****************************************************
*NAME:
* soc_sand_delete_chip_descriptors
*DATE:
* 30/OCT/2002
*FUNCTION:
* The opposite of the initialization routine of the descriptors
*INPUT:
*  SOC_SAND_DIRECT:
*  SOC_SAND_INDIRECT:
*   the array is deleted
*OUTPUT:
*  SOC_SAND_DIRECT:
*    Non-zero in case of an error
*  SOC_SAND_INDIRECT:
*REMARKS:
* The opposite of init. clears the whole array,
* and then de-allocate the array and the mutex
*SEE ALSO:
*****************************************************/
SOC_SAND_RET
 soc_sand_delete_chip_descriptors(
   void
 )
{
  unsigned
    int
      iii ;
  SOC_SAND_RET
    ex ;
  unsigned
    err = 0 ;
  ex = SOC_SAND_ERR ;
  /*
   * lets check that we have an array and mutex to delete
   */
  if (!Soc_sand_chip_descriptors)
  {
    err = 1 ;
    goto exit ;
  }
  if (!Soc_sand_array_mutex)
  {
    err = 2 ;
    goto exit ;
  }
  /*
   * First of all lets clear all descriptors
   */
  /*
   * go over the array and take all mutexes
   */
  for (iii=0 ; iii<Soc_sand_array_size ; ++iii)
  {
    if(soc_sand_is_chip_descriptor_valid(iii))
    {
      if (SOC_SAND_OK != soc_sand_take_chip_descriptor_mutex(iii)){
        err = 3 ;
        goto exit ;
      }
    }
  }
  /*
   * take array mutex
   */
  if (SOC_SAND_OK != soc_sand_os_mutex_take(Soc_sand_array_mutex, SOC_SAND_INFINITE_TIMEOUT))
  {
    err = 4 ;
    goto exit ;
  }
  /*
   * go over the array and clear all descriptors
   */
  for (iii=0 ; iii<Soc_sand_array_size ; ++iii)
  {
    if(soc_sand_is_chip_descriptor_valid(iii))
    {
      /* also deletes the chip mutex */
      soc_sand_clear_chip_descriptor(iii, TRUE) ;
    }
    else
    {
      soc_sand_clear_chip_descriptor(iii, FALSE) ;
    }
  }
  /*
   * Now we can free the allocated array
   */
  soc_sand_os_free(Soc_sand_chip_descriptors) ;
  Soc_sand_chip_descriptors = NULL ;
  Soc_sand_array_size       = 0 ;
  /*
   * And now we can finally delete the mutex (whomever
   * is trying to take it gets an error)
   */
  soc_sand_os_mutex_delete(Soc_sand_array_mutex) ;
  Soc_sand_array_mutex = 0 ;
  ex = SOC_SAND_OK ;
exit:
  SOC_SAND_ERROR_REPORT(ex,NULL,0,0,SOC_SAND_DELETE_CHIP_DESCRIPTORS,
        "General error in soc_sand_delete_chip_descriptors()",err,0,0,0,0,0) ;
  return ex ;
}
/*****************************************************
*NAME:
* soc_sand_remove_chip_descriptor
*DATE:
* 30/OCT/2002
*FUNCTION:
* The opposite of the soc_sand_add_chip_descriptor()
*INPUT:
*  SOC_SAND_DIRECT:
*     uint32 index - index in the array
*  SOC_SAND_INDIRECT:
*   the entry at index is deleted
*OUTPUT:
*  SOC_SAND_DIRECT:
*    Non-zero in case of an error
*  SOC_SAND_INDIRECT:
*REMARKS:
* The opposite of add. clears the entry.
*SEE ALSO:
*****************************************************/
SOC_SAND_RET
 soc_sand_remove_chip_descriptor(
   uint32 index
 )
{
  SOC_SAND_RET
    ex ;
  uint32
    err = 0 ;
  ex = SOC_SAND_ERR ;
  /*
   * check that the array was started successfully
   */
  if (!Soc_sand_chip_descriptors || !Soc_sand_array_mutex)
  {
    err = 1 ;
    goto exit ;
  }
  /*
   * check if index is legal (0 < index < Soc_sand_array_size)
   */
  if (index >= Soc_sand_array_size)
  {
    err = 2 ;
    goto exit ;
  }
  /*
   * the actual clearing of the descriptor should be done under mutex protection
   */
  if (SOC_SAND_OK != soc_sand_os_mutex_take(Soc_sand_array_mutex, SOC_SAND_INFINITE_TIMEOUT))
  {
    err = 3 ;
    goto exit ;
  }
  /*
   */
  soc_sand_clear_chip_descriptor(index, TRUE) ;
  /*
   */
  if (SOC_SAND_OK != soc_sand_os_mutex_give(Soc_sand_array_mutex))
  {
    err = 4 ;
    goto exit ;
  }
  ex = SOC_SAND_OK ;
exit:
  SOC_SAND_ERROR_REPORT(ex,NULL,0,0,SOC_SAND_REMOVE_CHIP_DESCRIPTOR,
        "General error in soc_sand_remove_chip_descriptor()",err,0,0,0,0,0) ;
  return ex ;
}
/*****************************************************
*NAME:
* soc_sand_add_chip_descriptor
*DATE:
* 27/OCT/2002
*FUNCTION:
* adds a chip descriptor to the array of descriptors
*INPUT:
*  SOC_SAND_DIRECT:
*     uint32  *base_address    -
*       start address of the device on the user board
*     uint32  mem_size         -
*       memory size (in bytes) that the device takes
*       in order to make sure it is not overlapping
*       with other devices.
*     DESC_SAND_REG  *reg_file        -
*       logical register table of that device,
*       usefull for logical access to the device.
*       currently not used.
*     SOC_SAND_UNMASK_FUNC_PTR  func_ptr       -
*       a pointer to a mthod that unmask the inettrupt
*       line of this specific device.
*     uint32  chip_type        -
*   	chip_typas defined by Dune in the device's data sheet
*     uint32  handle           -
*   	Specific handle to be used for device. If negative, handle
*   	will be allocated dynamically
*  SOC_SAND_INDIRECT:
*   a descriptor is added to the array
*OUTPUT:
*  SOC_SAND_DIRECT:
*     int -
*     non negative (handle to array) if success
*     negative if error:
*     -1: array or mutex not initialized
*     -2: failed to take the array mutex
*     -3: array is full
*     -4: memory over lap from front
*     -5: memory over lap from back
*     -6: failed to create mutex
*  SOC_SAND_INDIRECT:
*REMARKS:
*   This method does not check that the registered chip
*   is ctually there, and the chip type parameter is saved
*   for the upper layers use.
*   The mothod does not set the "valid word" into the descriptor,
*   but it is rather the upper layer reponsibility to check
*   that the method succeeded, and that the chip is really alive,
*   and only then to set it to be valid
*SEE ALSO:
*****************************************************/
int
  soc_sand_add_chip_descriptor(
    uint32                  *base_address,
    uint32                  mem_size, /*in bytes*/
    SOC_SAND_UNMASK_FUNC_PTR                unmask_func_ptr,
    SOC_SAND_IS_BIT_AUTO_CLEAR_FUNC_PTR     is_bit_ac_func_ptr,
    SOC_SAND_IS_DEVICE_INTERRUPTS_MASKED    is_dev_int_mask_func_ptr,
    SOC_SAND_GET_DEVICE_INTERRUPTS_MASK     get_dev_mask_func_ptr,
    SOC_SAND_MASK_SPECIFIC_INTERRUPT_CAUSE  mask_specific_interrupt_cause_ptr,
    SOC_SAND_RESET_DEVICE_FUNC_PTR     reset_device_ptr,
    SOC_SAND_IS_OFFSET_READ_OR_WRITE_PROTECT_FUNC_PTR is_read_write_protect_ptr,
    uint32                  chip_type,
    SOC_SAND_DEVICE_TYPE               logic_chip_type,
	int                            handle
  )
{
  int
    res ;
  uint32
    iii;
  sal_mutex_t
    mutex ;
  SOC_SAND_RET
    ex ;
  uint32
    err ;

  err = 0 ;
  ex = SOC_SAND_ERR ;
  /*
   */
  iii = 0 ;
  res = 0 ;
  mutex = 0 ;
  /*
   * check that the array was started successfully
   */
  if (!Soc_sand_chip_descriptors || !Soc_sand_array_mutex)
  {
    res = -1 ;
    goto exit ;
  }

  if (handle > 0)
  {
	  if (soc_sand_is_chip_descriptor_valid(handle))
      {
		  /* handle already in use */
		  res = -7;
		  goto exit;
	  }
	  else
	  {
		  iii = handle;
	  }
  }
  else
  {
	  for (iii=0 ; iii<Soc_sand_array_size ; ++iii)
	  {
		if (!soc_sand_is_chip_descriptor_valid(iii))
		{
		   break ;
		}
	  }
  }

  /*
   * check if the array is already full
   */
  if (iii >= Soc_sand_array_size)
  {
    res = -3 ;
    goto exit ;
  }
 /*
  * create the mutex for the new device
  */
  mutex = soc_sand_os_mutex_create() ;
  if (0 == mutex)
  {
    res = -6 ;
    goto exit ;
  }
  ex = SOC_SAND_OK ;
exit:
  if (0 == res)
  {
   Soc_sand_chip_descriptors[iii].mutex_id                          = mutex;
   Soc_sand_chip_descriptors[iii].mutex_id                          = mutex;
   Soc_sand_chip_descriptors[iii].base_addr                         = base_address;
   Soc_sand_chip_descriptors[iii].mem_size                          = mem_size;
   Soc_sand_chip_descriptors[iii].unmask_ptr                        = unmask_func_ptr;
   Soc_sand_chip_descriptors[iii].is_bit_auto_clear_ptr             = is_bit_ac_func_ptr;
   Soc_sand_chip_descriptors[iii].is_device_interrupts_masked_ptr   = is_dev_int_mask_func_ptr;
   Soc_sand_chip_descriptors[iii].get_device_interrupts_mask_ptr    = get_dev_mask_func_ptr;
   Soc_sand_chip_descriptors[iii].mask_specific_interrupt_cause_ptr = mask_specific_interrupt_cause_ptr;
   Soc_sand_chip_descriptors[iii].reset_device_ptr                  = reset_device_ptr;
   Soc_sand_chip_descriptors[iii].is_read_write_protect_ptr         = is_read_write_protect_ptr;
   Soc_sand_chip_descriptors[iii].logic_chip_type                   = logic_chip_type;
   Soc_sand_chip_descriptors[iii].chip_type                         = chip_type;
   Soc_sand_chip_descriptors[iii].magic                             = soc_get_magic();
   Soc_sand_chip_descriptors[iii].valid_word                        = 0;
   res = iii ;
  }
  err = (uint32)(-res) ;
  SOC_SAND_ERROR_REPORT(ex,NULL,0,0,SOC_SAND_ADD_CHIP_DESCRIPTOR,
        "General error in soc_sand_add_chip_descriptor()",err,0,0,0,0,0) ;
  return res ;
}



/*
 */
/*****************************************************
*NAME:
* soc_sand_register_event_callback
*DATE:
* 03/NOV/2002
*FUNCTION:
* adds a callback interrupt handling routine
* in charge of a specific interrupt cause
*INPUT:
*  SOC_SAND_DIRECT:
*    SOC_SAND_IN     SOC_SAND_TCM_CALLBACK      func      - callback to call
*    SOC_SAND_IN     uint32      *buffer   - buffer to pass to the callback
*    SOC_SAND_IN     uint32      size      - buffer size
*    SOC_SAND_IN     uint32         index     - device id
*    SOC_SAND_IN     uint32         cause     - interrupt bit
*    SOC_SAND_INOUT  uint32          *handle   - user suppllied buffer
*  SOC_SAND_INDIRECT:
*OUTPUT:
*  SOC_SAND_DIRECT:
*   Non-zero in case of an error
*  SOC_SAND_INDIRECT:
*   Handle to this specific callback (to be used by unregister_event_callback)
*REMARKS:
*SEE ALSO:
*****************************************************/
SOC_SAND_RET
  soc_sand_register_event_callback(
    SOC_SAND_INOUT  SOC_SAND_TCM_CALLBACK func,   /* callback to call*/
    SOC_SAND_INOUT  uint32     *buffer,/* buffer to pass to the callback */
    SOC_SAND_INOUT  uint32     size,   /* buffer size */
    SOC_SAND_IN     uint32      index,  /* device id */
    SOC_SAND_IN     uint32      cause,  /* interrupt bit */
    SOC_SAND_INOUT  uint32     *handle
  )
{
  uint32
      res ;
  SOC_SAND_RET
    ex = SOC_SAND_ERR ;
  uint32
    err = 0 ;
  /*
   * check that index is legal
   */
  if (index >= Soc_sand_array_size)
  {
    err = 1 ;
    goto exit ;
  }
  /*
   * check that cause is in the right range
   */
  if (cause >= MAX_INTERRUPT_CAUSE)
  {
    err = 2 ;
    goto exit ;
  }
  /*
   * check that descriptor is valid
   */
  if (!soc_sand_is_chip_descriptor_valid(index))
  {
    err = 3 ;
    goto exit ;
  }
  /*
   * Check that no callback is already there
   * (if so, it should have been unregistered)
   */
  if (SOC_SAND_OK != soc_sand_take_chip_descriptor_mutex(index))
  {
    err = 4 ;
    goto exit ;
  }
  if (Soc_sand_chip_descriptors[index].interrupt_callback_array[cause].func)
  {
    if (SOC_SAND_OK != soc_sand_give_chip_descriptor_mutex(index))
    {
      err = 5 ;
      goto exit ;
    }
    err = 6 ;
    goto exit ;
  }
  /*
   * OK, set the callback in its place
   */
  Soc_sand_chip_descriptors[index].interrupt_callback_array[cause].func    = func ;
  Soc_sand_chip_descriptors[index].interrupt_callback_array[cause].buffer  = buffer ;
  Soc_sand_chip_descriptors[index].interrupt_callback_array[cause].size    = size ;
  if (SOC_SAND_OK != soc_sand_give_chip_descriptor_mutex(index))
  {
      err = 7 ;
      goto exit ;
  }
  /*
   * as long a both index and cause are less the 16 bit, we can place
   * them together within a 32 bit handle
   */
  res   = index ;
  res <<= 16 ;
  res  += cause ;
  *handle = res ;
  ex = SOC_SAND_OK ;
exit:
  SOC_SAND_ERROR_REPORT(ex,NULL,0,0,SOC_SAND_REGISTER_EVENT_CALLBACK,
        "General error in soc_sand_register_event_callback()",err,0,0,0,0,0) ;
  return ex ;
}
/*****************************************************
*NAME:
* soc_sand_unregister_event_callback
*DATE:
* 03/NOV/2002
*FUNCTION:
* removes a callback interrupt handling routine
* in charge of a specific interrupt cause
*INPUT:
*  SOC_SAND_DIRECT:
*    SOC_SAND_IN  uint32   handle   -
*       the handle that was passed by register_event_callback
*  SOC_SAND_INDIRECT:
*OUTPUT:
*  SOC_SAND_DIRECT:
*   Non-zero in case of an error
*  SOC_SAND_INDIRECT:
*   The callback is deleted from the table
*REMARKS:
*SEE ALSO:
*****************************************************/
SOC_SAND_RET
  soc_sand_unregister_event_callback(
    SOC_SAND_IN     uint32            handle
  )
{
  SOC_SAND_RET
    ex = SOC_SAND_ERR ;
  unsigned
    int
      index,
      cause1,
      cause2 ;
  unsigned
    err = 0 ;
  /*
   * as long a both index and cause are less the 16 bit, they were
   * put together within a 32 bit handle
   */
  index = handle >> 16 ;
  cause1 = (handle << 24) >> 24 ;
  cause2 = (handle << 16) >> 24 ;
  /*
   * check that index is legal
   */
  if (index >= Soc_sand_array_size)
  {
    err = 1 ;
    goto exit ;
  }
  /*
   * check that descriptor is valid
   */
  if (!soc_sand_is_chip_descriptor_valid(index))
  {
    err = 2 ;
    goto exit ;
  }
  /*
   * Check that no callback is already there
   * (if so, it should have been unregistered)
   */
  if (SOC_SAND_OK != soc_sand_take_chip_descriptor_mutex(index))
  {
    err = 3 ;
    goto exit ;
  }
  /*
   * first, lets mask the specific bit (chain of bits actually) at the device
   */
  soc_sand_chip_descriptor_get_mask_specific_interrupt_cause_func(index)(index, cause1);
  /*
   * free the callback buffer
   */
  if (Soc_sand_chip_descriptors[index].interrupt_callback_array[cause1].buffer)
  {
    soc_sand_os_free(
      Soc_sand_chip_descriptors[index].interrupt_callback_array[cause1].buffer
    ) ;
  }
  /*
   * and zero everything else
   */
  Soc_sand_chip_descriptors[index].interrupt_callback_array[cause1].func    = NULL ;
  Soc_sand_chip_descriptors[index].interrupt_callback_array[cause1].buffer  = NULL ;
  Soc_sand_chip_descriptors[index].interrupt_callback_array[cause1].size    = 0 ;
  /*
   * and if there was a cause2 to, lets do him to
   */
  if (cause2)
  {
  /*
   * first, lets mask the specific bit (chain of bits actually) at the device
   */
  soc_sand_chip_descriptor_get_mask_specific_interrupt_cause_func(index)(index, cause2);
    /*
     * free the callback buffer
     */
    if (Soc_sand_chip_descriptors[index].interrupt_callback_array[cause2].buffer)
    {
      soc_sand_os_free(
        Soc_sand_chip_descriptors[index].interrupt_callback_array[cause2].buffer
      ) ;
    }
    /*
     * and zero everything else
     */
    Soc_sand_chip_descriptors[index].interrupt_callback_array[cause2].func    = NULL ;
    Soc_sand_chip_descriptors[index].interrupt_callback_array[cause2].buffer  = NULL ;
    Soc_sand_chip_descriptors[index].interrupt_callback_array[cause2].size    = 0 ;
  }
  if (soc_sand_give_chip_descriptor_mutex(index) != SOC_SAND_OK)
  {
    err = 4 ;
    goto exit ;
  }
  ex = SOC_SAND_OK ;
exit:
  SOC_SAND_ERROR_REPORT(ex,NULL,0,0,SOC_SAND_UNREGISTER_EVENT_CALLBACK,
        "General error in soc_sand_unregister_event_callback()",err,0,0,0,0,0) ;
  return ex ;
}
/*****************************************************
*NAME:
* soc_sand_combine_2_event_callback_handles
*DATE:
* 18/NOV/2002
*FUNCTION:
* combines 2 interrupts handels of the same unit
*INPUT:
*  SOC_SAND_DIRECT:
*    SOC_SAND_IN     uint32            handle_1  -
*         handle_1 to combine
*    SOC_SAND_IN     uint32            handle_2  -
*         handle_2 to combine
*    SOC_SAND_INOUT  uint32           *handle    -
*         combined handle
*  SOC_SAND_INDIRECT:
*OUTPUT:
*  SOC_SAND_DIRECT:
*   Non-zero in case of an error
*  SOC_SAND_INDIRECT:
*REMARKS:
*SEE ALSO:
*****************************************************/
SOC_SAND_RET
  soc_sand_combine_2_event_callback_handles(
    SOC_SAND_IN     uint32            handle_1,
    SOC_SAND_IN     uint32            handle_2,
    SOC_SAND_INOUT  uint32           *handle
  )
{
  SOC_SAND_RET
    ex ;
  unsigned
    int
      index_1,
      index_2,
      cause_2 ;
  uint32
    err = 0 ;
  /*
   * as long a both index and cause are less the 16 bit, they were
   * put together within a 32 bit handle
   */
  index_1 = handle_1 >> 16 ;
  index_2 = handle_2 >> 16 ;
  cause_2 = (handle_2 << 16) >> 16 ;
  /*
   */
  ex = SOC_SAND_OK ;
  if (!handle_1)
  {
    *handle = handle_2 ;
    goto exit ;
  }
  /*
   */
  if (!handle_2)
  {
    *handle = handle_1 ;
    goto exit ;
  }

  if (index_1 != index_2)
  {
    /*
     * not the same device
     */
    err = 1 ;
    ex = SOC_SAND_ERR ;
    goto exit ;
  }
  /*
   * Since, there are only 256 (8 bit) causes,
   * we can combine to of them together
   */
  cause_2 <<= 8 ;
  *handle = handle_1 | cause_2 ;
exit:
  SOC_SAND_ERROR_REPORT(ex,NULL,0,0,SOC_SAND_COMBINE_2_EVENT_CALLBACK_HANDLES,
        "General error in soc_sand_combine_2_event_callback_handles()",err,0,0,0,0,0) ;
  return ex ;

}

/*
 * check all device counter
 * return true / false
 */

uint32
  soc_sand_is_any_chip_descriptor_taken_by_task(
    sal_thread_t task_id
  )
{
  uint32
    chip_desc_taken_flag = FALSE,
    index_i;

  for(index_i = 0;index_i < Soc_sand_array_size; index_i++)
  {
    if(Soc_sand_chip_descriptors[index_i].mutex_owner == task_id)
    {
      chip_desc_taken_flag = TRUE;
      goto exit;
    }
  }
exit:
  return chip_desc_taken_flag;
}

uint32
  soc_sand_is_any_chip_descriptor_taken(
    void
  )
{
  uint32
    chip_desc_taken_flag = FALSE,
    index_i;

  for(index_i = 0;index_i < Soc_sand_array_size; index_i++)
  {
    if(soc_sand_is_chip_descriptor_valid(index_i))
    {
      chip_desc_taken_flag = TRUE;
      goto exit;
    }
  }
exit:
  return chip_desc_taken_flag;
}
/*****************************************************
*NAME:
* soc_sand_get_chip_descriptor_mutex_owner
* soc_sand_set_chip_descriptor_mutex_owner
* soc_sand_get_chip_descriptor_mutex_counter
* soc_sand_set_chip_descriptor_mutex_counter
* soc_sand_inc_chip_descriptor_mutex_counter
* soc_sand_dec_chip_descriptor_mutex_counter
*DATE:
* 10/NOV/2002
*FUNCTION:
* mutex_owner / mutex counter get / set metods
*INPUT:
*  SOC_SAND_DIRECT:
*     uint32 index - index in the array
*  SOC_SAND_INDIRECT:
*OUTPUT:
*  SOC_SAND_DIRECT:
*  SOC_SAND_INDIRECT:
*REMARKS:
* These 2 members allows for the same task th take
* the mutex more then once.
*SEE ALSO: soc_sand_take_chip_descriptor_mutex / soc_sand_give_chip_descriptor_mutex
*****************************************************/
sal_thread_t
  soc_sand_get_chip_descriptor_mutex_owner(
    uint32 index
  )
{
  sal_thread_t
    ex ;
  SOC_SAND_RET
    error_code ;
  uint32
    err = 0 ;
  error_code = SOC_SAND_OK ;
  /*
   */
  if (index >= Soc_sand_array_size)
  {
    ex = (sal_thread_t)(-1) ;
    err = 1;
    error_code = SOC_SAND_ERR ;
    goto exit ;
  }
  /*
   */
  ex = Soc_sand_chip_descriptors[index].mutex_owner ;
  /*
   */
exit:
  SOC_SAND_ERROR_REPORT(error_code,NULL,0,0,SOC_SAND_GET_CHIP_DESCRIPTOR_MUTEX_OWNER,
        "General error in soc_sand_get_chip_descriptor_mutex_owner()",err,0,0,0,0,0) ;
  return ex ;
}
/*
 */
STATIC
  SOC_SAND_RET
    soc_sand_set_chip_descriptor_mutex_owner(
      uint32 index,
      sal_thread_t         tid
    )
{
  SOC_SAND_RET
    ex = SOC_SAND_OK ;
  uint32
    err = 0 ;
  if (index >= Soc_sand_array_size)
  {
    err = 1 ;
    ex = SOC_SAND_ERR ;
    goto exit ;
  }
  Soc_sand_chip_descriptors[index].mutex_owner = tid ;
exit:
  SOC_SAND_ERROR_REPORT(ex,NULL,0,0,SOC_SAND_SET_CHIP_DESCRIPTOR_MUTEX_OWNER,
        "General error in soc_sand_set_chip_descriptor_mutex_owner()",err,0,0,0,0,0) ;
  return ex ;
}
/*
 */
STATIC
  int32
    soc_sand_get_chip_descriptor_mutex_counter(
      uint32 index
    )
{
  int32
    ex ;
  SOC_SAND_RET
    error_code ;
  uint32
    err = 0 ;
  error_code = SOC_SAND_OK ;
  if (index >= Soc_sand_array_size)
  {
    ex = -1 ;
    error_code = SOC_SAND_ERR ;
    goto exit ;
  }
  ex = Soc_sand_chip_descriptors[index].mutex_counter ;
exit:
  err = (uint32)(-ex) ;
  SOC_SAND_ERROR_REPORT(error_code,NULL,0,0,SOC_SAND_GET_CHIP_DESCRIPTOR_MUTEX_COUNTER,
        "General error in soc_sand_get_chip_descriptor_mutex_counter()",err,0,0,0,0,0) ;
  return ex ;
}
/*
 */
STATIC
  SOC_SAND_RET
    soc_sand_set_chip_descriptor_mutex_counter(
      uint32 index,
      int32 cnt
    )
{
  SOC_SAND_RET
    ex = SOC_SAND_OK ;
  uint32
    err = 0 ;
  if (index >= Soc_sand_array_size)
  {
    err = 1 ;
    ex = SOC_SAND_ERR ;
    goto exit ;
  }
   Soc_sand_chip_descriptors[index].mutex_counter = cnt ;
exit:
  SOC_SAND_ERROR_REPORT(ex,NULL,0,0,SOC_SAND_SET_CHIP_DESCRIPTOR_MUTEX_COUNTER,
        "General error in soc_sand_set_chip_descriptor_mutex_counter()",err,0,0,0,0,0) ;
  return ex ;
}
/*
 */
STATIC
  SOC_SAND_RET
    soc_sand_inc_chip_descriptor_mutex_counter(
      uint32 index
    )
{
  SOC_SAND_RET
    ex = SOC_SAND_OK ;
  uint32
    err = 0 ;
  if (index >= Soc_sand_array_size)
  {
    err = 1 ;
    ex = SOC_SAND_ERR ;
    goto exit ;
  }
  /*
   */
  Soc_sand_chip_descriptors[index].mutex_counter++ ;
  /*
   */
exit:
  SOC_SAND_ERROR_REPORT(ex,NULL,0,0,SOC_SAND_INC_CHIP_DESCRIPTOR_MUTEX_COUNTER,
        "General error in soc_sand_inc_chip_descriptor_mutex_counter()",err,0,0,0,0,0) ;
  return ex ;
}
/*
 */
STATIC
  SOC_SAND_RET
    soc_sand_dec_chip_descriptor_mutex_counter(
      uint32 index
    )
{
  SOC_SAND_RET
    ex = SOC_SAND_OK ;
  uint32
    err = 0 ;
  if (index >= Soc_sand_array_size)
  {
    err = 1 ;
    ex = SOC_SAND_ERR ;
    goto exit ;
  }
  /*
   */
  Soc_sand_chip_descriptors[index].mutex_counter-- ;
  if (Soc_sand_chip_descriptors[index].mutex_counter < 0)
  {
    err = 2 ;
    ex = SOC_SAND_ERR ;
    goto exit ;
  }
  /*
   */
exit:
  SOC_SAND_ERROR_REPORT(ex,NULL,0,0,SOC_SAND_DEC_CHIP_DESCRIPTOR_MUTEX_COUNTER,
        "General error in soc_sand_dec_chip_descriptor_mutex_counter()",err,0,0,0,0,0) ;
  return ex ;
}
/*****************************************************
*NAME:
* soc_sand_take_chip_descriptor_mutex
* soc_sand_give_chip_descriptor_mutex
*DATE:
* 29/OCT/2002
*FUNCTION:
*  takes / gives the mutex associated with the specific
*  chip, not forgetting several important legitimate
*  values tests.
*INPUT:
*  SOC_SAND_DIRECT:
*    uint32 index - index in the array
*  SOC_SAND_INDIRECT:
*OUTPUT:
*  SOC_SAND_DIRECT:
*    zero      - semaphore taken
*    negative  - error
*    positive  - device was cleared in the middle
*  SOC_SAND_INDIRECT:
*REMARKS:
*  These 2 methods are the one that should be carried
*SEE ALSO:
*****************************************************/
int
  soc_sand_take_chip_descriptor_mutex(
    uint32 index
  )
{
  int
    ret ;
  SOC_SAND_RET
    ex ;
  uint32
      local_magic ;
  sal_thread_t
    current_task_id;

  ex = SOC_SAND_ERR ;
  /*
   * check that index is legal
   */
  current_task_id = soc_sand_os_get_current_tid();
  if (index >= Soc_sand_array_size)
  {
    ex = SOC_SAND_ILLEGAL_DEVICE_ID ;
    ret = -1 ;
    goto exit ;
  }
  /*
   * check that descriptor is valid
   */
  if (!soc_sand_is_chip_descriptor_valid(index))
  {
    ex = SOC_SAND_ILLEGAL_CHIP_DESCRIPTOR ;
    ret = -2 ;
    goto exit ;
  }
  /*
   * if the mutex is already owned by this task, we increase the ownership
   * counter (it gets decreased only with a call to give), and return (OK),
   * since the mutex is already (rightfully) ours.
   */
  if (current_task_id == soc_sand_get_chip_descriptor_mutex_owner(index))
  {
    soc_sand_inc_chip_descriptor_mutex_counter(index) ;
    ret = 0 ;
    ex = SOC_SAND_OK ;
    goto exit ;
  }
  if(current_task_id == soc_sand_handles_delta_list_get_owner())
  {
    soc_sand_inc_chip_descriptor_mutex_counter(index) ;
    ret = -12;
    ex = SOC_SAND_CHIP_DESCRIPTORS_SEM_TAKE_ERR_0 ;
    goto exit ;
  }

  /*
   * if we got here, we have a different task_id then the owner (or maybe
   * there is no owner yet), so lets try and take it.
   *
   * we mark for ourself the last value of magic before trying to take
   * the mutex. This is a classic software defensive messure:
   * We get ready for the possibility that while we are waiting for the
   * semaphore another task will delete the descriptor (making it invalid)
   * and allocate a new one in its place (making it valid, but with different magic).
   *
   * and another point: although the value of chip_descriptor_mutex_owner might change
   * (even several times) before we actually gonna take it, it cannot be our task there,
   * since our task is waiting (infintely) on the mutex's Q.
   */
  local_magic = soc_sand_get_chip_descriptor_magic(index) ;
  if (SOC_SAND_OK != soc_sand_os_mutex_take(Soc_sand_chip_descriptors[index].mutex_id, (long)SOC_SAND_INFINITE_TIMEOUT))
  {
    if (!soc_sand_is_chip_descriptor_valid(index))
    {
      /*
       * we got the != SOC_SAND_OK because someone deleted the device mutex,
       * and cleared the descriptor (hence invalid)
       */
      ex = SOC_SAND_TRYING_TO_ACCESS_DELETED_DEVICE ;
      ret = -3 ;
      goto exit ;
    }
    else
    {
      /*
       * sem_take failed
       */
      ex = SOC_SAND_SEM_TAKE_FAIL ;
      ret = -4 ;
      goto exit ;
    }
  }
  /*
   * And we're not done with the errors.
   * Another possible scenario, is that while we were waiting
   * for the semaphore, someone deleted it, and allocated a new one.
   * So the taking of the mutex succeeded. This is a very difficult
   * bug to catch...
   */
  if(soc_sand_get_chip_descriptor_magic(index) != local_magic)
  {
    soc_sand_os_mutex_give(Soc_sand_chip_descriptors[index].mutex_id) ;
    ex = SOC_SAND_SEM_CHANGED_ON_THE_FLY ;
    ret = -5 ;
    goto exit ;
  }
  /*
   * Only if we got here, the mutex is rightfully ours (for the first
   * time), so lets mark our ownership, and return OK.
   */
  soc_sand_set_chip_descriptor_mutex_owner(index, soc_sand_os_get_current_tid()) ;
  soc_sand_set_chip_descriptor_mutex_counter(index, 1) ;
  ret = 0 ;
  ex = SOC_SAND_OK ;
exit:
  SOC_SAND_ERROR_REPORT(
    ex,NULL,0,0,SOC_SAND_TAKE_CHIP_DESCRIPTOR_MUTEX,
    "General error in soc_sand_take_chip_descriptor_mutex()",
    -ret,0,0,0,0,0
  );
  return ret ;
}
/*
 */
SOC_SAND_RET
  soc_sand_give_chip_descriptor_mutex(
    uint32 index
  )
{
  SOC_SAND_RET
    ex = SOC_SAND_OK ;
  uint32
    err = 0 ;
  if (soc_sand_os_get_current_tid() != soc_sand_get_chip_descriptor_mutex_owner(index))
  {
    err = 1 ;
    ex = SOC_SAND_ERR ;
    goto exit ;
  }
  soc_sand_dec_chip_descriptor_mutex_counter(index) ;
  if (soc_sand_get_chip_descriptor_mutex_counter(index) == 0)
  {
    soc_sand_set_chip_descriptor_mutex_owner(index, 0) ;
    soc_sand_os_mutex_give(Soc_sand_chip_descriptors[index].mutex_id) ;
  }
  else if (soc_sand_get_chip_descriptor_mutex_counter(index) < 0)
  {
    err = 2 ;
    ex = SOC_SAND_ERR ;
    goto exit ;
  }
exit:
  SOC_SAND_ERROR_REPORT(ex,NULL,0,0,SOC_SAND_GIVE_CHIP_DESCRIPTOR_MUTEX,
        "General error in soc_sand_give_chip_descriptor_mutex()",err,0,0,0,0,0) ;
  return ex ;
}


/*****************************************************
*NAME:
* soc_sand_set_chip_descriptor_ver_info
*DATE:
* 17/NOV/2004
*FUNCTION:
* sets a device version to a specific chip descriptor
*INPUT:
*  SOC_SAND_DIRECT:
*     uint32 index - index in the array
*  SOC_SAND_INDIRECT:
*OUTPUT:
*  SOC_SAND_DIRECT:
*   0 - error, otherwise the field that was requested
*  SOC_SAND_INDIRECT:
*REMARKS:
*SEE ALSO:
*****************************************************/
SOC_SAND_RET
  soc_sand_set_chip_descriptor_ver_info(
    SOC_SAND_IN uint32    index,
    SOC_SAND_IN uint32   dbg_ver,
    SOC_SAND_IN uint32   chip_ver
  )
{
  SOC_SAND_RET
    error_code = SOC_SAND_OK ;
  uint32
    err = 0 ;


  if (index >= Soc_sand_array_size)
  {
    error_code = SOC_SAND_ERR ;
    err = 1 ;
    goto exit ;
  }


  Soc_sand_chip_descriptors[index].dbg_ver  = dbg_ver ;
  Soc_sand_chip_descriptors[index].chip_ver = chip_ver ;

exit:
  SOC_SAND_ERROR_REPORT(error_code,NULL,0,0,SOC_SAND_SET_CHIP_DESCRIPTOR_VER_INFO,
        "General error in soc_sand_set_chip_descriptor_ver_info()",err,0,0,0,0,0) ;
  return error_code ;
}
/*****************************************************
*NAME:
* soc_sand_get_chip_descriptor_base_addr
* soc_sand_get_chip_descriptor_memory_size
* soc_sand_get_chip_descriptor_interrupt_callback_array
* soc_sand_get_chip_descriptor_interrupt_bitstream
* soc_sand_get_chip_descriptor_chip_type
* soc_sand_set_chip_descriptor_valid
* soc_sand_is_chip_descriptor_valid
* soc_sand_get_chip_descriptor_device_at_init
* soc_sand_get_chip_descriptor_magic
*DATE:
* 27/OCT/2002
*FUNCTION:
* gets a specific field from a specific chip descriptor
*INPUT:
*  SOC_SAND_DIRECT:
*     uint32 index - index in the array
*  SOC_SAND_INDIRECT:
*OUTPUT:
*  SOC_SAND_DIRECT:
*   0 - error, otherwise the field that was requested
*  SOC_SAND_INDIRECT:
*REMARKS:
*SEE ALSO:
*****************************************************/
uint32
  *soc_sand_get_chip_descriptor_base_addr(
    uint32 index
  )
{
  uint32
      *ex ;
  SOC_SAND_RET
    error_code = SOC_SAND_OK ;
  uint32
      err ;
  err = 0 ;
  if (index >= Soc_sand_array_size)
  {
    error_code = SOC_SAND_ERR ;
    err = 1 ;
    ex = 0 ;
    goto exit ;
  }
  ex = Soc_sand_chip_descriptors[index].base_addr ;
exit:
  SOC_SAND_ERROR_REPORT(error_code,NULL,0,0,SOC_SAND_GET_CHIP_DESCRIPTOR_BASE_ADDR,
        "General error in soc_sand_get_chip_descriptor_base_addr()",err,0,0,0,0,0) ;
  return ex ;
}
/*
 * size of memory space taken by this device
 */
uint32
  soc_sand_get_chip_descriptor_memory_size(
    uint32 index
  )
{
  uint32
      ex ;
  SOC_SAND_RET
    error_code = SOC_SAND_OK ;
  uint32
    err = 0 ;
  if (index >= Soc_sand_array_size)
  {
    error_code = SOC_SAND_ERR ;
    err = 1 ;
    ex = 0 ;
    goto exit ;
  }
  ex = Soc_sand_chip_descriptors[index].mem_size ;
exit:
  SOC_SAND_ERROR_REPORT(error_code,NULL,0,0,SOC_SAND_GET_CHIP_DESCRIPTOR_MEMORY_SIZE,
        "General error in soc_sand_get_chip_descriptor_memory_size()",err,0,0,0,0,0) ;
  return ex ;
}
/*
 * This is an array of callbacks, that do not require polling,
 * but rather act as event notifications. The cause for such
 * events are interrupts, so this is actually an arrray of callbacks,
 * each handles a different interrupt.
 */
struct soc_sand_tcm_callback_str
  *soc_sand_get_chip_descriptor_interrupt_callback_array(
    uint32 index
   )
{
   struct
    soc_sand_tcm_callback_str
      *ex ;
  SOC_SAND_RET
    error_code = SOC_SAND_OK ;
  uint32
    err = 0 ;
  if (index >= Soc_sand_array_size)
  {
    error_code = SOC_SAND_ERR ;
    err = 1 ;
    ex = 0 ;
    goto exit ;
  }
  ex = Soc_sand_chip_descriptors[index].interrupt_callback_array ;
exit:
  SOC_SAND_ERROR_REPORT(error_code,NULL,0,0,SOC_SAND_GET_CHIP_DESCRIPTOR_INTERRUPT_CALLBACK_ARRAY,
        "General error in soc_sand_get_chip_descriptor_interrupt_callback_array()",err,0,0,0,0,0) ;
  return ex ;
}
/*
 * This is the actual connection to the interrupt line. The interrupt serving routine
 * checks what caused the interrupt, and sets the right bit.
 */
uint32
  *soc_sand_get_chip_descriptor_interrupt_bitstream(
    uint32 index
  )
{
  uint32
     *ex ;
  SOC_SAND_RET
    error_code = SOC_SAND_OK ;
  uint32
    err = 0 ;
  if (index >= Soc_sand_array_size)
  {
    error_code = SOC_SAND_ERR ;
    err = 1 ;
    ex = 0 ;
    goto exit ;
  }
  ex = Soc_sand_chip_descriptors[index].interrupt_bitstream ;
exit:
  SOC_SAND_ERROR_REPORT(error_code,NULL,0,0,SOC_SAND_GET_CHIP_DESCRIPTOR_INTERRUPT_CALLBACK_ARRAY,
        "General error in soc_sand_get_chip_descriptor_interrupt_callback_array()",err,0,0,0,0,0) ;
  return ex ;
}
/*
 * The interrupt unmasking method tied up to this chip
 */
SOC_SAND_UNMASK_FUNC_PTR
  soc_sand_get_chip_descriptor_unmask_func(
    uint32 index
  )
{
  SOC_SAND_UNMASK_FUNC_PTR ex;
  SOC_SAND_RET error_code;
  uint32 err;
  /*
   */
  err = 0;
  error_code = SOC_SAND_OK;
  if (index >= Soc_sand_array_size)
  {
    error_code = SOC_SAND_ERR ;
    err = 1 ;
    ex = NULL ;
    goto exit ;
  }
  /*
   */
  ex = Soc_sand_chip_descriptors[index].unmask_ptr ;
  /*
   */
exit:
  SOC_SAND_ERROR_REPORT(error_code,NULL,0,0,SOC_SAND_GET_CHIP_DESCRIPTOR_UNMASK_FUNC,
        "General error in soc_sand_get_chip_descriptor_unmask_func()",err,0,0,0,0,0) ;
  return ex ;
}
/*
 */
SOC_SAND_IS_BIT_AUTO_CLEAR_FUNC_PTR
  soc_sand_get_chip_descriptor_is_bit_auto_clear_func(
    uint32 index
  )
{
  SOC_SAND_IS_BIT_AUTO_CLEAR_FUNC_PTR ex;
  SOC_SAND_RET error_code;
  uint32 err ;
  /*
   */
  error_code = SOC_SAND_OK;
  err = 0 ;
  if (index >= Soc_sand_array_size)
  {
    error_code = SOC_SAND_ERR;
    err = 1;
    ex = NULL;
    goto exit;
  }
  /*
   */
  ex = Soc_sand_chip_descriptors[index].is_bit_auto_clear_ptr;
  /*
   */
exit:
  SOC_SAND_ERROR_REPORT(error_code,NULL,0,0,SOC_SAND_GET_CHIP_DESCRIPTOR_IS_BIT_AC_FUNC,
        "General error in soc_sand_get_chip_descriptor_is_bit_auto_clear_func()",err,0,0,0,0,0) ;
  return ex ;
}
/*
 * Tcm interrupt serving loop must be carried out when device interrupts are masked.
 * if thery are not masked it is a fatal error.
 */
SOC_SAND_IS_DEVICE_INTERRUPTS_MASKED
  soc_sand_get_chip_descriptor_is_device_interrupts_masked_func(
    uint32 index
  )
{
  SOC_SAND_IS_DEVICE_INTERRUPTS_MASKED ex;
  SOC_SAND_RET error_code;
  uint32 err ;
  /*
   */
  error_code = SOC_SAND_OK;
  err = 0 ;
  if (index >= Soc_sand_array_size)
  {
    error_code = SOC_SAND_ERR;
    err = 1;
    ex = NULL;
    goto exit;
  }
  /*
   */
  ex = Soc_sand_chip_descriptors[index].is_device_interrupts_masked_ptr;
  /*
   */
exit:
  SOC_SAND_ERROR_REPORT(error_code,NULL,0,0,SOC_SAND_GET_CHIP_DESCRIPTOR_IS_DEVICE_INT_MASKED_FUNC,
        "General error in soc_sand_get_chip_descriptor_is_device_interrupts_masked_func()",err,0,0,0,0,0) ;
  return ex ;
}
/*
 * Every interrupt source in the device architecture has an associated
 * mask bit. the tcm must respect this bit as well, before calling
 * registered callback functions.
 */
SOC_SAND_GET_DEVICE_INTERRUPTS_MASK
  soc_sand_chip_descriptor_get_interrupts_mask_func(
    uint32 index
  )
{
  SOC_SAND_GET_DEVICE_INTERRUPTS_MASK ex;
  SOC_SAND_RET error_code;
  uint32 err ;
  /*
   */
  error_code = SOC_SAND_OK;
  err = 0 ;
  if (index >= Soc_sand_array_size)
  {
    error_code = SOC_SAND_ERR;
    err = 1;
    ex = NULL;
    goto exit;
  }
  /*
   */
  ex = Soc_sand_chip_descriptors[index].get_device_interrupts_mask_ptr;
  /*
   */
exit:
  SOC_SAND_ERROR_REPORT(error_code,NULL,0,0,SOC_SAND_GET_CHIP_DESCRIPTOR_GET_INT_MASK_FUNC,
        "General error in soc_sand_chip_descriptor_get_interrupts_mask_func()",err,0,0,0,0,0) ;
  return ex ;
}
/*
 * Every interrupt source in the device architecture has an associated
 * mask bit. this is the mean to reset (mask) it so the source will
 * not generate interrupts.
 */
SOC_SAND_MASK_SPECIFIC_INTERRUPT_CAUSE
  soc_sand_chip_descriptor_get_mask_specific_interrupt_cause_func(
    uint32 index
  )
{
  SOC_SAND_MASK_SPECIFIC_INTERRUPT_CAUSE ex;
  SOC_SAND_RET error_code;
  uint32 err ;
  /*
   */
  error_code = SOC_SAND_OK;
  err = 0 ;
  if (index >= Soc_sand_array_size)
  {
    error_code = SOC_SAND_ERR;
    err = 1;
    ex = NULL;
    goto exit;
  }
  /*
   */
  ex = Soc_sand_chip_descriptors[index].mask_specific_interrupt_cause_ptr;
  /*
   */
exit:
  SOC_SAND_ERROR_REPORT(error_code,NULL,0,0,SOC_SAND_GET_CHIP_DESCRIPTOR_GET_INT_MASK_FUNC,
        "General error in soc_sand_chip_descriptor_get_interrupts_mask_func()",err,0,0,0,0,0) ;
  return ex ;
}
/*
 */
SOC_SAND_RESET_DEVICE_FUNC_PTR
  soc_sand_chip_descriptor_get_reset_device_func(
    uint32 index
  )
{
  SOC_SAND_RESET_DEVICE_FUNC_PTR ex;
  SOC_SAND_RET error_code;
  uint32 err ;
  /*
   */
  error_code = SOC_SAND_OK;
  err = 0 ;
  if (index >= Soc_sand_array_size)
  {
    error_code = SOC_SAND_ERR;
    err = 1;
    ex = NULL;
    goto exit;
  }
  /*
   */
  ex = Soc_sand_chip_descriptors[index].reset_device_ptr;
  /*
   */
exit:
  SOC_SAND_ERROR_REPORT(error_code,NULL,0,0,SOC_SAND_GET_CHIP_DESCRIPTOR_GET_INT_MASK_FUNC,
        "General error in soc_sand_chip_descriptor_get_reset_device_func()",err,0,0,0,0,0) ;
  return ex ;
}
/*
 */
SOC_SAND_IS_OFFSET_READ_OR_WRITE_PROTECT_FUNC_PTR
  soc_sand_chip_descriptor_get_is_read_write_protect_func(
    uint32 index
  )
{
  SOC_SAND_IS_OFFSET_READ_OR_WRITE_PROTECT_FUNC_PTR ex;
  SOC_SAND_RET error_code;
  uint32 err;
  /*
   */
  error_code = SOC_SAND_OK;
  err = 0;
  if (index >= Soc_sand_array_size)
  {
    error_code = SOC_SAND_ERR;
    err = 1;
    ex = NULL;
    goto exit;
  }
  /*
   */
  ex = Soc_sand_chip_descriptors[index].is_read_write_protect_ptr;
  /*
   */
exit:
  SOC_SAND_ERROR_REPORT(error_code,NULL,0,0,SOC_SAND_GET_CHIP_DESCRIPTOR_GET_INT_MASK_FUNC,
        "General error in soc_sand_chip_descriptor_get_is_read_write_protect_func()",err,0,0,0,0,0) ;
  return ex;
}
/*
 * mask / unmask counter and level
 * The logic goes something like this:
 * you can mask the general interrupt bit (fecint) from 2 places:
 * the interrupt handler, or the user code
 * (through fe200_logical_write() api method of this logic field_id)
 * The interrupt is superior to the user - meaning, we don't want
 * the user to unmask them while interrupts are still pending to
 * be served (that's the reason they were masked to begin with)
 * That's why we keep a level - Interrupt level = TRUE, user level = FALSE
 * The same apply to the unmasking only now it's the tcm that unmasks the
 * fecint bit, when done serving interrupts.
 * We maintain the counter in order to know to which state to restore.
 * > 0 => masked, < 0 => unmasked
 */
uint32
  soc_sand_device_is_between_isr_to_tcm(
    uint32 index
  )
{
  uint32 ex;
  SOC_SAND_RET error_code;
  uint32 err ;
  /*
   */
  error_code = SOC_SAND_OK;
  err = 0 ;
  if (index >= Soc_sand_array_size)
  {
    error_code = SOC_SAND_ERR;
    err = 1;
    ex = FALSE;
    goto exit;
  }
  /*
   */
  ex = Soc_sand_chip_descriptors[index].device_is_between_isr_to_tcm;
  /*
   */
exit:
  SOC_SAND_ERROR_REPORT(error_code,NULL,0,0,SOC_SAND_GET_CHIP_DESCRIPTOR_DEV_BETWEEN_ISR_AND_TCM,
        "General error in soc_sand_device_is_between_isr_to_tcm()",err,0,0,0,0,0) ;
  return ex ;
}
void
  soc_sand_device_set_between_isr_to_tcm(
    uint32 index,
  uint32 level
  )
{
  SOC_SAND_RET error_code;
  uint32 err ;
  /*
   */
  error_code = SOC_SAND_OK;
  err = 0 ;
  if (index >= Soc_sand_array_size)
  {
    error_code = SOC_SAND_ERR;
    err = 1;
    goto exit;
  }
  /*
   */
  Soc_sand_chip_descriptors[index].device_is_between_isr_to_tcm = level;
  /*
   */
exit:
  SOC_SAND_ERROR_REPORT(error_code,NULL,0,0,SOC_SAND_SET_CHIP_DESCRIPTOR_DEV_BETWEEN_ISR_AND_TCM,
        "General error in soc_sand_device_set_between_isr_to_tcm()",err,0,0,0,0,0) ;
}
/*
 */
int
  soc_sand_device_get_interrupt_mask_counter(
    uint32 index
  )
{
  int ex;
  SOC_SAND_RET error_code;
  uint32 err ;
  /*
   */
  error_code = SOC_SAND_OK;
  err = 0 ;
  if (index >= Soc_sand_array_size)
  {
    error_code = SOC_SAND_ERR;
    err = 1;
    ex = 0;
    goto exit;
  }
  /*
   */
  ex = Soc_sand_chip_descriptors[index].device_interrupt_mask_counter;
  /*
   */
exit:
  SOC_SAND_ERROR_REPORT(error_code,NULL,0,0,SOC_SAND_GET_CHIP_DESCRIPTOR_INT_MASK_COUNTER,
        "General error in soc_sand_device_get_interrupt_mask_counter()",err,0,0,0,0,0) ;
  return ex ;
}
void
  soc_sand_device_inc_interrupt_mask_counter(
    uint32 index
  )
{
  SOC_SAND_RET error_code;
  uint32 err ;
  /*
   */
  error_code = SOC_SAND_OK;
  err = 0 ;
  if (index >= Soc_sand_array_size)
  {
    error_code = SOC_SAND_ERR;
    err = 1;
    goto exit;
  }
  /*
   */
  Soc_sand_chip_descriptors[index].device_interrupt_mask_counter ++;
  /*
   */
exit:
  SOC_SAND_ERROR_REPORT(error_code,NULL,0,0,SOC_SAND_INC_CHIP_DESCRIPTOR_INT_MASK_COUNTER,
        "General error in soc_sand_device_inc_interrupt_mask_counter()",err,0,0,0,0,0) ;
}
void
  soc_sand_device_dec_interrupt_mask_counter(
    uint32 index
  )
{
  SOC_SAND_RET error_code;
  uint32 err ;
  /*
   */
  error_code = SOC_SAND_OK;
  err = 0 ;
  if (index >= Soc_sand_array_size)
  {
    error_code = SOC_SAND_ERR;
    err = 1;
    goto exit;
  }
  /*
   */
  Soc_sand_chip_descriptors[index].device_interrupt_mask_counter --;
  /*
   */
exit:
  SOC_SAND_ERROR_REPORT(error_code,NULL,0,0,SOC_SAND_DEC_CHIP_DESCRIPTOR_INT_MASK_COUNTER,
        "General error in soc_sand_device_dec_interrupt_mask_counter()",err,0,0,0,0,0) ;
}
/*
 */
uint32
  soc_sand_is_chip_descriptor_chip_ver_bigger_eq(
    uint32   index,
    uint32  chip_ver_bigger_eq
  )
{
  unsigned
    int
      ex ;
  SOC_SAND_RET
    error_code = SOC_SAND_OK ;
  uint32
    err = 0 ;
  if (index >= Soc_sand_array_size)
  {
    error_code = SOC_SAND_ERR ;
    err = 1 ;
    ex = FALSE ;
    goto exit ;
  }

#if FE200_REV_B_AS_REV_A
  /*
   * REV-B act as REV-A
   */
  if (Soc_sand_chip_descriptors[index].chip_type == 0xFE200)
  {
    ex = FALSE;
    goto exit;
  }
#endif

  if(Soc_sand_chip_descriptors[index].chip_ver >= chip_ver_bigger_eq)
  {
    ex = TRUE;
  }
  else
  {
    ex = FALSE;
  }

exit:
  SOC_SAND_ERROR_REPORT(error_code,NULL,0,0,SOC_SAND_IS_CHIP_DESCRIPTOR_CHIP_VER_BIGGER_EQ,
        "General error in soc_sand_is_chip_descriptor_chip_ver_bigger_eq()",err,0,0,0,0,0) ;
  return ex ;
}
/*
 */
#define SIMULATE_REV_B_ON_REV_A 0
uint32
  soc_sand_get_chip_descriptor_chip_ver(
    uint32 index
  )
{
  uint32
      ex ;
  SOC_SAND_RET
    error_code = SOC_SAND_OK ;
  uint32
      err ;
  err = 0 ;
  if (index >= Soc_sand_array_size)
  {
    error_code = SOC_SAND_ERR ;
    err = 1 ;
    ex = 0 ;
    goto exit ;
  }
  ex = Soc_sand_chip_descriptors[index].chip_ver ;
#if SIMULATE_REV_B_ON_REV_A
  ex = 0x02/*FAP10M_EXPECTED_CHIP_VER_02*/;
#endif
exit:
  SOC_SAND_ERROR_REPORT(error_code,NULL,0,0,SOC_SAND_GET_CHIP_DESCRIPTOR_CHIP_VER,
        "General error in soc_sand_get_chip_descriptor_chip_ver()",err,0,0,0,0,0) ;
  return ex ;
}
/*
 */
uint32
  soc_sand_get_chip_descriptor_dbg_ver(
    uint32 index
  )
{
  uint32
      ex ;
  SOC_SAND_RET
    error_code = SOC_SAND_OK ;
  uint32
      err ;
  err = 0 ;
  if (index >= Soc_sand_array_size)
  {
    error_code = SOC_SAND_ERR ;
    err = 1 ;
    ex = 0 ;
    goto exit ;
  }
  ex = Soc_sand_chip_descriptors[index].dbg_ver ;
exit:
  SOC_SAND_ERROR_REPORT(error_code,NULL,0,0,SOC_SAND_GET_CHIP_DESCRIPTOR_DBG_VER,
        "General error in soc_sand_get_chip_descriptor_dbg_ver()",err,0,0,0,0,0) ;
  return ex ;
}
/*
 */
uint32
  soc_sand_get_chip_descriptor_chip_type(
    uint32 index
  )
{
  uint32
      ex ;
  SOC_SAND_RET
    error_code = SOC_SAND_OK ;
  uint32
      err ;
  err = 0 ;
  if (index >= Soc_sand_array_size)
  {
    error_code = SOC_SAND_ERR ;
    err = 1 ;
    ex = 0 ;
    goto exit ;
  }
  ex = Soc_sand_chip_descriptors[index].chip_type ;
exit:
  SOC_SAND_ERROR_REPORT(error_code,NULL,0,0,SOC_SAND_GET_CHIP_DESCRIPTOR_CHIP_TYPE,
        "General error in soc_sand_get_chip_descriptor_chip_type()",err,0,0,0,0,0) ;
  return ex ;
}
/*
 */
uint32
  soc_sand_get_chip_descriptor_logic_chip_type(
    uint32 index
  )
{
  uint32
      ex ;
  SOC_SAND_RET
    error_code = SOC_SAND_OK ;
  uint32
      err ;
  err = 0 ;
  if (index >= Soc_sand_array_size)
  {
    error_code = SOC_SAND_ERR ;
    err = 1 ;
    ex = 0 ;
    goto exit ;
  }
  ex = Soc_sand_chip_descriptors[index].logic_chip_type ;
exit:
  SOC_SAND_ERROR_REPORT(error_code,NULL,0,0,SOC_SAND_GET_CHIP_DESCRIPTOR_CHIP_TYPE,
        "General error in soc_sand_get_chip_descriptor_logic_chip_type()",err,0,0,0,0,0) ;
  return ex ;
}
/*
 */
void
  soc_sand_set_chip_descriptor_valid(
    uint32 index
  )
{
  Soc_sand_chip_descriptors[index].valid_word = SOC_SAND_DEVICE_DESC_VALID_WORD ;
}
/*
 * TRUE if valid FALSE if invalid
 */
uint32
  soc_sand_is_chip_descriptor_valid(
    uint32 index
  )
{
  uint32
    ex ;
  SOC_SAND_RET
    error_code = SOC_SAND_OK ;
  uint32
      err ;
  err = 0 ;
  if (index >= Soc_sand_array_size)
  {
    error_code = SOC_SAND_ERR ;
    err = 1 ;
    ex = FALSE ;
    goto exit ;
  }
  /*
   */
  if (SOC_SAND_DEVICE_DESC_VALID_WORD == Soc_sand_chip_descriptors[index].valid_word)
  {
    ex = TRUE ;
  }
  else
  {
    ex = FALSE ;
  }
exit:
  SOC_SAND_ERROR_REPORT(error_code,NULL,0,0,SOC_SAND_IS_CHIP_DESCRIPTOR_VALID,
        "General error in soc_sand_is_chip_descriptor_valid()",err,index,0,0,0,0) ;
  return ex ;
}
/*
 * Write flag indicating initialization stage of this device
 */
SOC_SAND_RET
  soc_sand_set_chip_descriptor_device_at_init(
    uint32 index,
    uint32 val
  )
{
  SOC_SAND_RET error_code;
  uint32 err ;
  error_code = SOC_SAND_OK;
  err = 0 ;
  if (index >= Soc_sand_array_size)
  {
    error_code = SOC_SAND_ERR;
    err = 1;
    goto exit;
  }
  if (val)
  {
    Soc_sand_chip_descriptors[index].device_at_init = TRUE ;
  }
  else
  {
    Soc_sand_chip_descriptors[index].device_at_init = FALSE ;
  }
exit:
  SOC_SAND_ERROR_REPORT(error_code,NULL,0,0,SOC_SAND_SET_CHIP_DESCRIPTOR_DEVICE_AT_INIT,
        "General error in soc_sand_set_chip_descriptor_device_at_init()",err,0,0,0,0,0) ;
  return (error_code) ;
}
/*
 * Read flag indicating initialization stage of this device
 */
SOC_SAND_RET
  soc_sand_get_chip_descriptor_device_at_init(
    uint32 index,
    uint32 *val
  )
{
  SOC_SAND_RET
    error_code = SOC_SAND_OK ;
  uint32
      err ;
  err = 0 ;
  if (index >= Soc_sand_array_size)
  {
    error_code = SOC_SAND_ERR ;
    err = 1 ;
    goto exit ;
  }
  *val = Soc_sand_chip_descriptors[index].device_at_init ;
exit:
  SOC_SAND_ERROR_REPORT(error_code,NULL,0,0,SOC_SAND_GET_CHIP_DESCRIPTOR_DEVICE_AT_INIT,
        "General error in soc_sand_get_chip_descriptor_device_at_init()",err,0,0,0,0,0) ;
  return (error_code) ;
}
/*
 * a unique uint32 for that instance of descriptor
 */
uint32
  soc_sand_get_chip_descriptor_magic(
    uint32 index
  )
{
  uint32
    ex ;
  SOC_SAND_RET
    error_code = SOC_SAND_OK ;
  uint32
      err ;
  err = 0 ;
  if (index >= Soc_sand_array_size)
  {
    error_code = SOC_SAND_ERR ;
    err = 1 ;
    ex = 0 ;
    goto exit ;
  }
  ex = Soc_sand_chip_descriptors[index].magic ;
exit:
  SOC_SAND_ERROR_REPORT(error_code,NULL,0,0,SOC_SAND_GET_CHIP_DESCRIPTOR_MAGIC,
        "General error in soc_sand_get_chip_descriptor_magic()",err,0,0,0,0,0) ;
  return ex ;
}

#if SOC_SAND_DEBUG
/* { */

void
  soc_sand_chip_descriptors_print(
    void
  )
{
  uint32
    descriptor_i;
  /*
   */
  if ( (0    == Soc_sand_array_mutex) ||
       (NULL == Soc_sand_chip_descriptors)
     )
  {
    LOG_CLI((BSL_META("soc_sand_chip_descriptors(): do not have internal params (probably the driver was not started)\n\r")));
    goto exit;
  }
  /*
   */
  if (SOC_SAND_OK != soc_sand_os_mutex_take(Soc_sand_array_mutex, SOC_SAND_INFINITE_TIMEOUT))
  {
    LOG_CLI((BSL_META("soc_sand_chip_descriptors(): soc_sand_os_mutex_take() failed\n\r")));
    goto exit ;
  }
  /*
   */
  for (descriptor_i=0; descriptor_i<Soc_sand_array_size; descriptor_i++)
  {
    if (0 == Soc_sand_chip_descriptors[descriptor_i].valid_word)
    {
      /*
       * No device registered here.
       */
      continue;
    }
    if (FALSE == soc_sand_is_chip_descriptor_valid(descriptor_i))
    {
      /*
       * Error faound.
       */
      LOG_CLI((BSL_META("device[%2u] descriptor not valid\n\r"), descriptor_i));
      continue;
    }
    LOG_CLI((BSL_META("device[%2u] %s (0x%08X), chip_ver:%2u dbg_ver:%2u, \n\r"
"            address:0x%08X. Mutex: counter-%lu, owner-%p\n\r"),
             descriptor_i,
             soc_sand_DEVICE_TYPE_to_str(
             Soc_sand_chip_descriptors[descriptor_i].logic_chip_type
             ),
             Soc_sand_chip_descriptors[descriptor_i].chip_type,
             Soc_sand_chip_descriptors[descriptor_i].chip_ver,
             Soc_sand_chip_descriptors[descriptor_i].dbg_ver,
             PTR_TO_INT(Soc_sand_chip_descriptors[descriptor_i].base_addr),
             (unsigned long)Soc_sand_chip_descriptors[descriptor_i].mutex_counter,
             Soc_sand_chip_descriptors[descriptor_i].mutex_owner
             ));
  }
  /*
   */
  if (SOC_SAND_OK != soc_sand_os_mutex_give(Soc_sand_array_mutex))
  {
    LOG_CLI((BSL_META("soc_sand_chip_descriptors(): soc_sand_os_mutex_give() failed\n\r")));
    goto exit ;
  }
  /*
   */
exit:
  return;
}



/* } */
#endif

