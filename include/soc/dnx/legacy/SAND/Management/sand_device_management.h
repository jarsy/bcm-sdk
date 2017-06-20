/* $Id: sand_device_management.h,v 1.5 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 * $
*/


#ifndef __DNX_SAND_DEVICE_MANAGEMENT_H_INCLUDED__
/* { */
#define __DNX_SAND_DEVICE_MANAGEMENT_H_INCLUDED__
#ifdef  __cplusplus
extern "C" {
#endif

#include <soc/dnx/legacy/SAND/SAND_FM/sand_chip_defines.h>

#include <soc/dnx/legacy/SAND/Utils/sand_framework.h>

#include <soc/dnx/legacy/SAND/SAND_FM/sand_indirect_access.h>

/* $Id: sand_device_management.h,v 1.5 Broadcom SDK $
 * Version Information structure.
 * For inner driver usage - not to be used out side the driver.
 * This structure is for passing parameters to 'dnx_sand_device_register()'.
 * {
 */

typedef struct
{
  /*
   * Version register offset
   */
  uint32  ver_reg_offset;

  /*
   * Chip-Type info.
   */

  DNX_SAND_DEVICE_TYPE logic_chip_type;
  uint32    chip_type;
  uint32    chip_type_shift;
  uint32    chip_type_mask;

  /*
   * Debug-Version info.
   */
  uint32  dbg_ver_shift;
  uint32  dbg_ver_mask;

  /*
   * Chip-Version info.
   */
  uint32  chip_ver_shift;
  uint32  chip_ver_mask;

  /* 
   * BCM only: skip verification when CMIC
   */
  uint8 cmic_skip_verif;

} DNX_SAND_DEV_VER_INFO;

extern uint8 Soc_register_with_id;

void
  dnx_sand_clear_SAND_DEV_VER_INFO(
    DNX_SAND_OUT DNX_SAND_DEV_VER_INFO* ver_info
  );
/*
 * }
 */


DNX_SAND_RET
  dnx_sand_device_register_with_id(DNX_SAND_IN uint8 enable);

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
    uint32                 interrupt_mask_address,
    int                  *unit_ptr
  );

DNX_SAND_RET
  dnx_sand_device_unregister(
    int unit
  ) ;

/*
* Return the chip definitions.
*/
DNX_SAND_RET
  dnx_sand_get_device_type(
    int      unit,
    DNX_SAND_DEVICE_TYPE *chip_type,
    uint32    *chip_ver,
    uint32    *dbg_ver
  ) ;
#ifdef DNX_SAND_DEBUG


/*
 * Printing utility.
 * Convert from enumerator to string.
 */
const char*
  dnx_sand_DEVICE_TYPE_to_str(
    DNX_SAND_IN DNX_SAND_DEVICE_TYPE dev_type
  );
#endif


#ifdef  __cplusplus
}
#endif

/* } __DNX_SAND_DEVICE_MANAGEMENT_H_INCLUDED__*/
#endif
