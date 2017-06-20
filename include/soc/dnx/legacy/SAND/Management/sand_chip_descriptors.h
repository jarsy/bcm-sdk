/* $Id: sand_chip_descriptors.h,v 1.5 Broadcom SDK $
* $Copyright: (c) 2016 Broadcom.
* Broadcom Proprietary and Confidential. All rights reserved.$
 * $
*/
#ifndef DNX_SAND_CHIP_DESCRIPTORS_H
#define DNX_SAND_CHIP_DESCRIPTORS_H
/* $Id: sand_chip_descriptors.h,v 1.5 Broadcom SDK $
 * {
 */
#ifdef  __cplusplus
extern "C" {
#endif
#include <soc/dnx/legacy/SAND/Utils/sand_framework.h>
#include <soc/dnx/legacy/SAND/SAND_FM/sand_chip_defines.h>
#include <soc/dnx/legacy/SAND/Utils/sand_tcm.h>
/*
 *
 */

extern  uint32
    Dnx_soc_sand_array_size;
extern DNX_SAND_DEVICE_DESC
    *Dnx_soc_sand_chip_descriptors;
extern uint32 Dnx_soc_sand_up_counter;
DNX_SAND_RET
  dnx_sand_array_mutex_take(void);

DNX_SAND_RET
  dnx_sand_array_mutex_give(void);
/*
 * The initialization routine of the chip descriptors array
 */
DNX_SAND_RET
 dnx_sand_init_chip_descriptors(
   uint32 max_devices
 );
/*
 */
DNX_SAND_RET
 dnx_sand_delete_chip_descriptors(
   void
 );
/*
 */
DNX_SAND_RET
 dnx_sand_remove_chip_descriptor(
   uint32 index
 );
/*
 * negative if error,
 * handle to array if success
 */
int
  dnx_sand_add_chip_descriptor(
     uint32                  *base_address,
     uint32                  mem_size, /*in bytes*/
     DNX_SAND_UNMASK_FUNC_PTR                unmask_func_ptr,
     DNX_SAND_IS_BIT_AUTO_CLEAR_FUNC_PTR     is_bit_ac_func_ptr,
     DNX_SAND_IS_DEVICE_INTERRUPTS_MASKED    is_dev_int_mask_func_ptr,
     DNX_SAND_GET_DEVICE_INTERRUPTS_MASK     get_dev_mask_func_ptr,
     DNX_SAND_MASK_SPECIFIC_INTERRUPT_CAUSE  mask_specific_interrupt_cause_ptr,
     DNX_SAND_RESET_DEVICE_FUNC_PTR     reset_device_ptr,
     DNX_SAND_IS_OFFSET_READ_OR_WRITE_PROTECT_FUNC_PTR is_read_write_protect_ptr,
     uint32                  chip_type,
     DNX_SAND_DEVICE_TYPE               logic_chip_type,
	 int                            handle
  );
/*
 */
DNX_SAND_RET
  dnx_sand_register_event_callback(
    DNX_SAND_INOUT  DNX_SAND_TCM_CALLBACK func,   /* callback to call*/
    DNX_SAND_INOUT  uint32     *buffer,/* buffer to pass to the callback */
    DNX_SAND_INOUT  uint32     size,   /* buffer size */
    DNX_SAND_IN     uint32      index,  /* unit */
    DNX_SAND_IN     uint32      cause,  /* interrupt bit */
    DNX_SAND_INOUT  uint32     *handle
  );
/*
 */
DNX_SAND_RET
  dnx_sand_unregister_event_callback(
    DNX_SAND_IN     uint32            handle
  );
/*
 */
DNX_SAND_RET
  dnx_sand_combine_2_event_callback_handles(
    DNX_SAND_IN     uint32            handle_1,
    DNX_SAND_IN     uint32            handle_2,
    DNX_SAND_INOUT  uint32           *handle
  );
/*
 */
int
  dnx_sand_take_chip_descriptor_mutex(
    uint32 index
  );
/*
 */
DNX_SAND_RET
  dnx_sand_give_chip_descriptor_mutex(
    uint32 index
  );
DNX_SAND_RET
  dnx_sand_set_chip_descriptor_ver_info(
    DNX_SAND_IN uint32    index,
    DNX_SAND_IN uint32   dbg_ver,
    DNX_SAND_IN uint32   chip_ver
  );
/*
 * All read / writes to this chip are done with offset from this base
 */
uint32
  *dnx_sand_get_chip_descriptor_base_addr(
    uint32 index
  );
/*
 * size of memory space taken by this device
 */
uint32
  dnx_sand_get_chip_descriptor_memory_size(
    uint32 index
  );
/*
 * This is an array of callbacks, that do not require polling,
 * but rather act as event notifications. The cause for such
 * events are interrupts, so this is actually an arrray of callbacks,
 * each handles a different interrupt.
 */
dnx_sand_tcm_callback_str_t
  *dnx_sand_get_chip_descriptor_interrupt_callback_array(
    uint32 index
  );
/*
 * This is the actual connection to the interrupt line. The interrupt serving routine
 * checks what caused the interrupt, and sets the right bit.
 */
uint32
  *dnx_sand_get_chip_descriptor_interrupt_bitstream(
    uint32 index
  );
/*
 * The interrupt unmasking method tied up to this chip
 */
DNX_SAND_UNMASK_FUNC_PTR
  dnx_sand_get_chip_descriptor_unmask_func(
    uint32 index
  );
/*
 */
DNX_SAND_IS_BIT_AUTO_CLEAR_FUNC_PTR
  dnx_sand_get_chip_descriptor_is_bit_auto_clear_func(
    uint32 index
  );
/*
 * Tcm interrupt serving loop must be carried out when device interrupts are masked.
 * if thery are not masked it is a fatal error.
 */
DNX_SAND_IS_DEVICE_INTERRUPTS_MASKED
  dnx_sand_get_chip_descriptor_is_device_interrupts_masked_func(
    uint32 index
  );
/*
 * Every interrupt source in the device architecture has an associated
 * mask bit. the tcm must respect this bit as well, before calling
 * registered callback functions.
 */
DNX_SAND_GET_DEVICE_INTERRUPTS_MASK
  dnx_sand_chip_descriptor_get_interrupts_mask_func(
    uint32 index
  );
/*
 * Every interrupt source in the device architecture has an associated
 * mask bit. this is the mean to reset (mask) it so the source will
 * not generate interrupts.
 */
DNX_SAND_MASK_SPECIFIC_INTERRUPT_CAUSE
  dnx_sand_chip_descriptor_get_mask_specific_interrupt_cause_func(
    uint32 index
  );
/*
 */
DNX_SAND_RESET_DEVICE_FUNC_PTR
  dnx_sand_chip_descriptor_get_reset_device_func(
    uint32 index
  );
/*
 */
DNX_SAND_IS_OFFSET_READ_OR_WRITE_PROTECT_FUNC_PTR
  dnx_sand_chip_descriptor_get_is_read_write_protect_func(
    uint32 index
  );
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
  dnx_sand_device_is_between_isr_to_tcm(
    uint32 index
  );
void
  dnx_sand_device_set_between_isr_to_tcm(
    uint32 index,
  uint32 level
  );
int
  dnx_sand_device_get_interrupt_mask_counter(
    uint32 index
  );
void
  dnx_sand_device_inc_interrupt_mask_counter(
    uint32 index
  );
void
  dnx_sand_device_dec_interrupt_mask_counter(
    uint32 index
  );
/*
 * This is a list of constants representing the chip,
 * might be longer at the future
 */
uint32
  dnx_sand_is_chip_descriptor_chip_ver_bigger_eq(
    uint32   index,
    uint32  chip_ver_bigger_eq
  );
uint32
  dnx_sand_get_chip_descriptor_chip_ver(
    uint32 index
  );
uint32
  dnx_sand_get_chip_descriptor_dbg_ver(
    uint32 index
  );

uint32
  dnx_sand_get_chip_descriptor_chip_type(
    uint32 index
  );

#define DNX_SAND_DEVICE_TYPE_GET(unit) \
          dnx_sand_get_chip_descriptor_logic_chip_type(unit)

uint32
  dnx_sand_get_chip_descriptor_logic_chip_type(
    uint32 index
  );
/*
 */
void
  dnx_sand_set_chip_descriptor_valid(
    uint32 index
  );
/*
 * TRUE if valid FALSE if invalid
 */
uint32
  dnx_sand_is_chip_descriptor_valid(
    uint32 index
  );
/*
 * Write flag indicating initialization stage of this device
 */
DNX_SAND_RET
  dnx_sand_set_chip_descriptor_device_at_init(
    uint32 index,
    uint32 val
  ) ;
/*
 * Read flag indicating initialization stage of this device
 */
DNX_SAND_RET
  dnx_sand_get_chip_descriptor_device_at_init(
    uint32 index,
    uint32 *val
  ) ;
/*
 * a unique uint32 for that instance of descriptor
 */
uint32
  dnx_sand_get_chip_descriptor_magic(
    uint32 index
  );
/*
 * return the owner of the mutex.
 * return (sal_thread_t)(-1) if not exist.
 */
sal_thread_t
  dnx_sand_get_chip_descriptor_mutex_owner(
    uint32 index
  );
/*
 * return true / false
 */
uint32
  dnx_sand_is_any_chip_descriptor_taken(
    void
  );
uint32
  dnx_sand_is_any_chip_descriptor_taken_by_task(
    sal_thread_t task_id
  );


#if DNX_SAND_DEBUG
/* { */

/*
 * Print utility.
 * Prints the chip descriptors info.
 */
void
  dnx_sand_chip_descriptors_print(
    void
  );

/* } */
#endif


#ifdef  __cplusplus
}
#endif

/*
 * }
 */
#endif
