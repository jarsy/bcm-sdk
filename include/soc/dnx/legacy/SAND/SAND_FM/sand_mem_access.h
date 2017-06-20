/* $Id: sand_mem_access.h,v 1.5 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 * $
*/


#ifndef __DNX_SAND_MEM_ACCESS_H_INCLUDED__
/* { */
#define __DNX_SAND_MEM_ACCESS_H_INCLUDED__
#ifdef  __cplusplus
extern "C" {
#endif

#include <soc/dnx/legacy/SAND/Utils/sand_framework.h>


extern uint32
  Soc_interrupt_mask_address_arr[DNX_SAND_MAX_DEVICE] ;
extern uint32
  Dnx_soc_sand_mem_check_read_write_protect;


DNX_SAND_RET
  dnx_sand_mem_read(
    DNX_SAND_IN  int     unit,
    DNX_SAND_OUT uint32    *result_ptr,
    DNX_SAND_IN  uint32    offset,
    DNX_SAND_IN  uint32     size,
    DNX_SAND_IN  uint32     indirect
  ) ;

DNX_SAND_RET
  dnx_sand_mem_read_unsafe(
    DNX_SAND_IN  int     unit,
    DNX_SAND_OUT uint32    *result_ptr,
    DNX_SAND_IN  uint32    offset,
    DNX_SAND_IN  uint32     size,
    DNX_SAND_IN  uint32     indirect
  );


DNX_SAND_RET
  dnx_sand_mem_write(
    DNX_SAND_IN int  unit,
    DNX_SAND_IN uint32 *result_ptr,
    DNX_SAND_IN uint32 offset,
    DNX_SAND_IN uint32  size,
    DNX_SAND_IN uint32  indirect
  ) ;



DNX_SAND_RET
  dnx_sand_mem_write_unsafe(
    DNX_SAND_IN int  unit,
    DNX_SAND_IN uint32 *result_ptr,
    DNX_SAND_IN uint32 offset,
    DNX_SAND_IN uint32  size,
    DNX_SAND_IN uint32  indirect
  );


/* $Id: sand_mem_access.h,v 1.5 Broadcom SDK $
 * {
 * Field access function. Uses dnx_sand_mem_read/dnx_sand_mem_write to read/write/update
 * specific field in a register.
 */

/*****************************************************
 * See details in dnx_sand_mem_access.c
 *****************************************************/
DNX_SAND_RET
  dnx_sand_read_field(
    DNX_SAND_IN  int  unit,
    DNX_SAND_IN  uint32 offset,
    DNX_SAND_IN  uint32 shift,
    DNX_SAND_IN  uint32 mask,
    DNX_SAND_OUT uint32* data
  );

DNX_SAND_RET
  dnx_sand_read_field_int(
    DNX_SAND_IN  int  unit,
    DNX_SAND_IN  uint32 offset,
    DNX_SAND_IN  uint32 shift,
    DNX_SAND_IN  uint32 mask,
    DNX_SAND_OUT uint32* data
  );

DNX_SAND_RET
  dnx_sand_read_field_ex(
    DNX_SAND_IN  int  unit,
    DNX_SAND_IN  uint32 offset,
    DNX_SAND_IN  uint32 shift,
    DNX_SAND_IN  uint32 mask,
    DNX_SAND_IN  uint32  indirect,
    DNX_SAND_OUT uint32* data
  );

/*****************************************************
 * See details in dnx_sand_mem_access.c
 *****************************************************/
DNX_SAND_RET
  dnx_sand_write_field(
    DNX_SAND_IN  int  unit,
    DNX_SAND_IN  uint32 offset,
    DNX_SAND_IN  uint32 shift,
    DNX_SAND_IN  uint32 mask,
    DNX_SAND_IN  uint32 data_to_write
  );
/*****************************************************
 * See details in dnx_sand_mem_access.c
 *****************************************************/
DNX_SAND_RET
  dnx_sand_write_field_ex(
    DNX_SAND_IN  int  unit,
    DNX_SAND_IN  uint32 offset,
    DNX_SAND_IN  uint32 shift,
    DNX_SAND_IN  uint32 mask,
    DNX_SAND_IN  uint32  indirect,
    DNX_SAND_IN  uint32 data_to_write
  );

/*
 * }
 */


/*
 * Set the interrupt mask address of the device.
 */
DNX_SAND_RET
  dnx_sand_mem_interrupt_mask_address_set(
    DNX_SAND_IN int  unit,
    DNX_SAND_IN uint32 interrupt_mask_address
  ) ;

/*
 * Get the interrupt mask address of the device.
 */
uint32
  dnx_sand_mem_interrupt_mask_address_get(
    DNX_SAND_IN int  unit
  ) ;

/*
 * Clears unit interrupt mask address information.
 */
DNX_SAND_RET
  dnx_sand_mem_interrupt_mask_address_clear(
    DNX_SAND_IN int  unit
  ) ;

/*
 * Clears all devices interrupt mask address information.
 */
DNX_SAND_RET
  dnx_sand_mem_interrupt_mask_address_clear_all(
    void
  );

/*
 * Check read/write protect mechanism
 * {
 */


/*****************************************************
 * See details in dnx_sand_mem_access.c
 *****************************************************/
uint32
  dnx_sand_mem_check_read_write_protect_get(
    void
  );

/*****************************************************
 * See details in dnx_sand_mem_access.c
 *****************************************************/
void
  dnx_sand_mem_check_read_write_protect_set(
    DNX_SAND_IN uint32 read_write_protect
  );


#if DNX_SAND_DEBUG

#define DNX_SAND_ADDRESS_ARRAY_MAX_SIZE       0xFFFF
#define DNX_SAND_MAX_NOF_INDIRECT_TABLES      0xFF

#define DNX_SAND_DONT_PRINT_ZEROS DNX_SAND_BIT(0)
#define DNX_SAND_PRINT_BITS       DNX_SAND_BIT(1)
#define DNX_SAND_PRINT_RANGES     DNX_SAND_BIT(2)

#define DNX_SAND_PRINT_START_ADDR_LSB   (3)
#define DNX_SAND_PRINT_START_ADDR_MSB   (3+16)

/*****************************************************
 * See details in dnx_sand_mem_access.c
 *****************************************************/
DNX_SAND_RET
  dnx_sand_print_registers_array(
    DNX_SAND_IN int unit,
    DNX_SAND_IN uint32 address_array[DNX_SAND_ADDRESS_ARRAY_MAX_SIZE],
    DNX_SAND_IN uint32 nof_address,
    DNX_SAND_IN uint32 print_options_bm
  );

/*****************************************************
 * See details in dnx_sand_mem_access.c
 *****************************************************/
DNX_SAND_RET
  dnx_sand_print_block(
    DNX_SAND_IN int  unit,
    DNX_SAND_IN uint32 address_array[DNX_SAND_ADDRESS_ARRAY_MAX_SIZE],
    DNX_SAND_IN uint32 nof_address,
    DNX_SAND_IN uint32 print_options_bm
  );

/*****************************************************
 * See details in dnx_sand_mem_access.c
 *****************************************************/
typedef struct{
  uint32 addr;
  uint32 size;
  const char    *name;
}DNX_SAND_INDIRECT_PRINT_INFO;


typedef DNX_SAND_INDIRECT_PRINT_INFO dnx_indirect_print_info; /* Backward compatibility */

DNX_SAND_RET
  dnx_sand_print_indirect_tables(
    DNX_SAND_IN int             unit,
    DNX_SAND_IN DNX_SAND_INDIRECT_PRINT_INFO print_info[DNX_SAND_MAX_NOF_INDIRECT_TABLES],
    DNX_SAND_IN uint32            print_options_bm
  );

#endif
/*
 * }
 */



#ifdef  __cplusplus
}
#endif

/* } __DNX_SAND_MEM_ACCESS_H_INCLUDED__*/
#endif
