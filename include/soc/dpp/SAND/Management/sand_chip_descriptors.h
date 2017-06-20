/* $Id: sand_chip_descriptors.h,v 1.5 Broadcom SDK $
* $Copyright: (c) 2016 Broadcom.
* Broadcom Proprietary and Confidential. All rights reserved.$
 * $
*/
#ifndef SOC_SAND_CHIP_DESCRIPTORS_H
#define SOC_SAND_CHIP_DESCRIPTORS_H
/* $Id: sand_chip_descriptors.h,v 1.5 Broadcom SDK $
 * {
 */
#ifdef  __cplusplus
extern "C" {
#endif
#include <soc/dpp/SAND/Utils/sand_framework.h>
#include <soc/dpp/SAND/SAND_FM/sand_chip_defines.h>
#include <soc/dpp/SAND/Utils/sand_tcm.h>
/*
 *
 */

extern  uint32
    Soc_sand_array_size;
extern SOC_SAND_DEVICE_DESC
    *Soc_sand_chip_descriptors;
extern uint32 Soc_sand_up_counter;
SOC_SAND_RET
  soc_sand_array_mutex_take(void);

SOC_SAND_RET
  soc_sand_array_mutex_give(void);
/*
 * The initialization routine of the chip descriptors array
 */
SOC_SAND_RET
 soc_sand_init_chip_descriptors(
   uint32 max_devices
 );
/*
 */
SOC_SAND_RET
 soc_sand_delete_chip_descriptors(
   void
 );
/*
 */
SOC_SAND_RET
 soc_sand_remove_chip_descriptor(
   uint32 index
 );
/*
 * negative if error,
 * handle to array if success
 */
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
  );
/*
 */
SOC_SAND_RET
  soc_sand_register_event_callback(
    SOC_SAND_INOUT  SOC_SAND_TCM_CALLBACK func,   /* callback to call*/
    SOC_SAND_INOUT  uint32     *buffer,/* buffer to pass to the callback */
    SOC_SAND_INOUT  uint32     size,   /* buffer size */
    SOC_SAND_IN     uint32      index,  /* unit */
    SOC_SAND_IN     uint32      cause,  /* interrupt bit */
    SOC_SAND_INOUT  uint32     *handle
  );
/*
 */
SOC_SAND_RET
  soc_sand_unregister_event_callback(
    SOC_SAND_IN     uint32            handle
  );
/*
 */
SOC_SAND_RET
  soc_sand_combine_2_event_callback_handles(
    SOC_SAND_IN     uint32            handle_1,
    SOC_SAND_IN     uint32            handle_2,
    SOC_SAND_INOUT  uint32           *handle
  );
/*
 */
int
  soc_sand_take_chip_descriptor_mutex(
    uint32 index
  );
/*
 */
SOC_SAND_RET
  soc_sand_give_chip_descriptor_mutex(
    uint32 index
  );
SOC_SAND_RET
  soc_sand_set_chip_descriptor_ver_info(
    SOC_SAND_IN uint32    index,
    SOC_SAND_IN uint32   dbg_ver,
    SOC_SAND_IN uint32   chip_ver
  );
/*
 * All read / writes to this chip are done with offset from this base
 */
uint32
  *soc_sand_get_chip_descriptor_base_addr(
    uint32 index
  );
/*
 * size of memory space taken by this device
 */
uint32
  soc_sand_get_chip_descriptor_memory_size(
    uint32 index
  );
/*
 * This is an array of callbacks, that do not require polling,
 * but rather act as event notifications. The cause for such
 * events are interrupts, so this is actually an arrray of callbacks,
 * each handles a different interrupt.
 */
soc_sand_tcm_callback_str_t
  *soc_sand_get_chip_descriptor_interrupt_callback_array(
    uint32 index
  );
/*
 * This is the actual connection to the interrupt line. The interrupt serving routine
 * checks what caused the interrupt, and sets the right bit.
 */
uint32
  *soc_sand_get_chip_descriptor_interrupt_bitstream(
    uint32 index
  );
/*
 * The interrupt unmasking method tied up to this chip
 */
SOC_SAND_UNMASK_FUNC_PTR
  soc_sand_get_chip_descriptor_unmask_func(
    uint32 index
  );
/*
 */
SOC_SAND_IS_BIT_AUTO_CLEAR_FUNC_PTR
  soc_sand_get_chip_descriptor_is_bit_auto_clear_func(
    uint32 index
  );
/*
 * Tcm interrupt serving loop must be carried out when device interrupts are masked.
 * if thery are not masked it is a fatal error.
 */
SOC_SAND_IS_DEVICE_INTERRUPTS_MASKED
  soc_sand_get_chip_descriptor_is_device_interrupts_masked_func(
    uint32 index
  );
/*
 * Every interrupt source in the device architecture has an associated
 * mask bit. the tcm must respect this bit as well, before calling
 * registered callback functions.
 */
SOC_SAND_GET_DEVICE_INTERRUPTS_MASK
  soc_sand_chip_descriptor_get_interrupts_mask_func(
    uint32 index
  );
/*
 * Every interrupt source in the device architecture has an associated
 * mask bit. this is the mean to reset (mask) it so the source will
 * not generate interrupts.
 */
SOC_SAND_MASK_SPECIFIC_INTERRUPT_CAUSE
  soc_sand_chip_descriptor_get_mask_specific_interrupt_cause_func(
    uint32 index
  );
/*
 */
SOC_SAND_RESET_DEVICE_FUNC_PTR
  soc_sand_chip_descriptor_get_reset_device_func(
    uint32 index
  );
/*
 */
SOC_SAND_IS_OFFSET_READ_OR_WRITE_PROTECT_FUNC_PTR
  soc_sand_chip_descriptor_get_is_read_write_protect_func(
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
  soc_sand_device_is_between_isr_to_tcm(
    uint32 index
  );
void
  soc_sand_device_set_between_isr_to_tcm(
    uint32 index,
  uint32 level
  );
int
  soc_sand_device_get_interrupt_mask_counter(
    uint32 index
  );
void
  soc_sand_device_inc_interrupt_mask_counter(
    uint32 index
  );
void
  soc_sand_device_dec_interrupt_mask_counter(
    uint32 index
  );
/*
 * This is a list of constants representing the chip,
 * might be longer at the future
 */
uint32
  soc_sand_is_chip_descriptor_chip_ver_bigger_eq(
    uint32   index,
    uint32  chip_ver_bigger_eq
  );
uint32
  soc_sand_get_chip_descriptor_chip_ver(
    uint32 index
  );
uint32
  soc_sand_get_chip_descriptor_dbg_ver(
    uint32 index
  );

uint32
  soc_sand_get_chip_descriptor_chip_type(
    uint32 index
  );

#define SOC_SAND_DEVICE_TYPE_GET(unit) \
          soc_sand_get_chip_descriptor_logic_chip_type(unit)

uint32
  soc_sand_get_chip_descriptor_logic_chip_type(
    uint32 index
  );
/*
 */
void
  soc_sand_set_chip_descriptor_valid(
    uint32 index
  );
/*
 * TRUE if valid FALSE if invalid
 */
uint32
  soc_sand_is_chip_descriptor_valid(
    uint32 index
  );
/*
 * Write flag indicating initialization stage of this device
 */
SOC_SAND_RET
  soc_sand_set_chip_descriptor_device_at_init(
    uint32 index,
    uint32 val
  ) ;
/*
 * Read flag indicating initialization stage of this device
 */
SOC_SAND_RET
  soc_sand_get_chip_descriptor_device_at_init(
    uint32 index,
    uint32 *val
  ) ;
/*
 * a unique uint32 for that instance of descriptor
 */
uint32
  soc_sand_get_chip_descriptor_magic(
    uint32 index
  );
/*
 * return the owner of the mutex.
 * return (sal_thread_t)(-1) if not exist.
 */
sal_thread_t
  soc_sand_get_chip_descriptor_mutex_owner(
    uint32 index
  );
/*
 * return true / false
 */
uint32
  soc_sand_is_any_chip_descriptor_taken(
    void
  );
uint32
  soc_sand_is_any_chip_descriptor_taken_by_task(
    sal_thread_t task_id
  );


#if SOC_SAND_DEBUG
/* { */

/*
 * Print utility.
 * Prints the chip descriptors info.
 */
void
  soc_sand_chip_descriptors_print(
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
