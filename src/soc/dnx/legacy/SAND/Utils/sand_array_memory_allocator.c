/* $Id: sand_array_memory_allocator.c,v 1.7 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/


/*************
* INCLUDES  *
*************/
/* { */


#include <shared/bsl.h>
#include <soc/dnx/legacy/drv.h>



#include <soc/dnx/legacy/SAND/Utils/sand_header.h>

#include <soc/dnx/legacy/SAND/Utils/sand_array_memory_allocator.h>
#include <soc/dnx/legacy/SAND/Management/sand_error_code.h>
#include <soc/dnx/legacy/SAND/Management/sand_general_macros.h>
#include <soc/dnx/legacy/SAND/Utils/sand_os_interface.h>
#include <soc/dnx/legacy/SAND/Utils/sand_integer_arithmetic.h>

/* } */

/*************
* DEFINES   *
*************/
/* { */

#define DNX_SAND_ARR_MEM_ALLOCATOR_LIST_END      0xFFFFF

#define DNX_SAND_ARR_MEM_ALLOCATOR_FREE_ENTRY_LEN      2

#define DNX_SAND_ARR_MEM_ALLOCATOR_ENTRY_SIZE_LSB      0
#define DNX_SAND_ARR_MEM_ALLOCATOR_ENTRY_SIZE_MSB      19
#define DNX_SAND_ARR_MEM_ALLOCATOR_ENTRY_NEXT_PTR_LSB  20
#define DNX_SAND_ARR_MEM_ALLOCATOR_ENTRY_NEXT_PTR_MSB  29
#define DNX_SAND_ARR_MEM_ALLOCATOR_ENTRY_PREV_PTR_LSB  0
#define DNX_SAND_ARR_MEM_ALLOCATOR_ENTRY_PREV_PTR_MSB  19
#define DNX_SAND_ARR_MEM_ALLOCATOR_ENTRY_NEXT_PTR_SIZE 10
#define DNX_SAND_ARR_MEM_ALLOCATOR_ENTRY_USE_LSB      31
#define DNX_SAND_ARR_MEM_ALLOCATOR_ENTRY_USE_MSB      31


/* flags control copying a block of memory */
#define DNX_SAND_ARR_MEM_ALLOCATOR_MOVE_COPY           (0x1)
#define DNX_SAND_ARR_MEM_ALLOCATOR_MOVE_FREE           (0x2)
#define DNX_SAND_ARR_MEM_ALLOCATOR_MOVE_UPDATE_CONT    (0x4)
#define DNX_SAND_ARR_MEM_ALLOCATOR_MOVE_UPDATE_REF     (0x8)
#define DNX_SAND_ARR_MEM_ALLOCATOR_MOVE_BLK_SIZE_VLD   (0x10)


/*************
*  MACROS   *
*************/
/* { */
#define DNX_SAND_ARR_MEM_ALLOCATOR_ENTRY_IS_USED(entry)   \
            DNX_SAND_GET_BITS_RANGE(entry,DNX_SAND_ARR_MEM_ALLOCATOR_ENTRY_USE_MSB,DNX_SAND_ARR_MEM_ALLOCATOR_ENTRY_USE_LSB)

#define DNX_SAND_ARR_MEM_ALLOCATOR_ENTRY_SIZE(entry)   \
            DNX_SAND_GET_BITS_RANGE(entry,DNX_SAND_ARR_MEM_ALLOCATOR_ENTRY_SIZE_MSB,DNX_SAND_ARR_MEM_ALLOCATOR_ENTRY_SIZE_LSB)

#define DNX_SAND_ARR_MEM_ALLOCATOR_FREE_PREV_PTR(entry)   \
            DNX_SAND_GET_BITS_RANGE(entry,DNX_SAND_ARR_MEM_ALLOCATOR_ENTRY_PREV_PTR_MSB,DNX_SAND_ARR_MEM_ALLOCATOR_ENTRY_PREV_PTR_LSB)

#define DNX_SAND_ARR_MEM_ALLOCATOR_FREE_NEXT_PTR(entry)   \
  DNX_SAND_GET_BITS_RANGE(entry,DNX_SAND_ARR_MEM_ALLOCATOR_ENTRY_NEXT_PTR_MSB,DNX_SAND_ARR_MEM_ALLOCATOR_ENTRY_NEXT_PTR_LSB)

/* $Id: sand_array_memory_allocator.c,v 1.7 Broadcom SDK $
 * MACRO given address get the next aligned address
 */
#define DNX_SAND_ARR_MEM_ALLOCATOR_GET_ALIGNED(addr,align)  \
        (addr == 0)? 0:((((addr) - 1)/(align) + 1) * (align))

#define DNX_SAND_ARR_MEM_ALLOCATOR_CACHE_INST(_inst) (DNX_SAND_BIT(31)|(_inst))

#define DNX_SAND_ARR_MEM_ALLOCATOR_ACTIVE_INST(_arr_mem_info, _inst) \
      (_arr_mem_info->arr_mem_allocator_data.cache_enabled)?DNX_SAND_ARR_MEM_ALLOCATOR_CACHE_INST(_inst):_inst
      
/* return actual size of memory, reduce size needed for defragment */
#define DNX_SAND_ARR_MEM_ALLOCATOR_LAST_INDEX(_arr_mem_info)   (((_arr_mem_info)->support_defragment)?((_arr_mem_info)->nof_entries - 1 - (_arr_mem_info)->max_block_size):((_arr_mem_info)->nof_entries-1))

/* } */


/*************
* TYPE DEFS *
*************/
/* { */
typedef struct
{
 /*
  * array to include entries
  */
  uint8 use;
  DNX_SAND_ARR_MEM_ALLOCATOR_PTR indx1;
  DNX_SAND_ARR_MEM_ALLOCATOR_PTR indx2;
  DNX_SAND_ARR_MEM_ALLOCATOR_PTR prev;
  DNX_SAND_ARR_MEM_ALLOCATOR_PTR next;
  uint32 size;
} DNX_SAND_ARR_MEM_ALLOCATOR_ENTRY_INFO;


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

/************************************************************************/
/* internal functions                                                   */
/************************************************************************/

static uint32
  dnx_sand_arr_mem_allocator_get_next_free(
    DNX_SAND_IN  DNX_SAND_ARR_MEM_ALLOCATOR_INFO     *arr_mem_info,
    DNX_SAND_IN  uint8                down,
    DNX_SAND_INOUT  DNX_SAND_ARR_MEM_ALLOCATOR_PTR   *free_ptr
  )
{
  DNX_SAND_ARR_MEM_ALLOCATOR_PTR
    tmp_ptr_lsb,
    tmp_ptr_msb,
    tmp_ptr;
  DNX_SAND_ARR_MEM_ALLOCATOR_ENTRY
    entry;
  uint32
    block_size,
    next_ptr;
  uint32
    res;

  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);

  tmp_ptr = *free_ptr;
  tmp_ptr_lsb = 0;
  tmp_ptr_msb = 0;

  res = arr_mem_info->entry_get_fun(
          arr_mem_info->instance_prim_handle,
          DNX_SAND_ARR_MEM_ALLOCATOR_ACTIVE_INST(arr_mem_info,arr_mem_info->instance_sec_handle),
          tmp_ptr,
          &entry
        );
  DNX_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  block_size = DNX_SAND_ARR_MEM_ALLOCATOR_ENTRY_SIZE(entry);
  next_ptr = tmp_ptr + block_size - 1;

  if (down)
  {
    tmp_ptr_lsb = DNX_SAND_ARR_MEM_ALLOCATOR_FREE_NEXT_PTR(entry);
  }

  res = arr_mem_info->entry_get_fun(
          arr_mem_info->instance_prim_handle,
          DNX_SAND_ARR_MEM_ALLOCATOR_ACTIVE_INST(arr_mem_info,arr_mem_info->instance_sec_handle),
          next_ptr,
          &entry
        );
  DNX_SAND_CHECK_FUNC_RESULT(res, 20, exit);
  if (down)
  {
    tmp_ptr_msb = DNX_SAND_ARR_MEM_ALLOCATOR_FREE_NEXT_PTR(entry);
  }
  else
  {
    *free_ptr = DNX_SAND_ARR_MEM_ALLOCATOR_FREE_PREV_PTR(entry);
    goto exit;
  }

  tmp_ptr = tmp_ptr_lsb;

  tmp_ptr |=
    DNX_SAND_SET_BITS_RANGE(
      tmp_ptr_msb,
      DNX_SAND_ARR_MEM_ALLOCATOR_ENTRY_NEXT_PTR_SIZE * 2 - 1,
      DNX_SAND_ARR_MEM_ALLOCATOR_ENTRY_NEXT_PTR_SIZE
    );

  *free_ptr = tmp_ptr;

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in dnx_sand_arr_mem_allocator_get_next_free()",0,0);
}

static uint32
  dnx_sand_arr_mem_allocator_header_get(
    DNX_SAND_INOUT  DNX_SAND_ARR_MEM_ALLOCATOR_INFO        *arr_mem_info,
    DNX_SAND_IN  DNX_SAND_ARR_MEM_ALLOCATOR_PTR            footer_ptr,
    DNX_SAND_OUT  DNX_SAND_ARR_MEM_ALLOCATOR_PTR           *header_ptr
  )
{
  DNX_SAND_ARR_MEM_ALLOCATOR_PTR
    prev;
  DNX_SAND_ARR_MEM_ALLOCATOR_ENTRY
    entry;
  uint32
    res;

  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(arr_mem_info);
  DNX_SAND_CHECK_NULL_INPUT(header_ptr);

  res = arr_mem_info->entry_get_fun(
          arr_mem_info->instance_prim_handle,
          DNX_SAND_ARR_MEM_ALLOCATOR_ACTIVE_INST(arr_mem_info,arr_mem_info->instance_sec_handle),
          footer_ptr,
          &entry
        );
  DNX_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  prev = DNX_SAND_ARR_MEM_ALLOCATOR_FREE_PREV_PTR(entry);

  if (prev == DNX_SAND_ARR_MEM_ALLOCATOR_LIST_END)
  {
    res = arr_mem_info->free_get_fun(
            arr_mem_info->instance_prim_handle,
            DNX_SAND_ARR_MEM_ALLOCATOR_ACTIVE_INST(arr_mem_info,arr_mem_info->instance_sec_handle),
            header_ptr
          );
    DNX_SAND_CHECK_FUNC_RESULT(res, 10, exit);
  }
  else
  {
   res = dnx_sand_arr_mem_allocator_get_next_free(
            arr_mem_info,
            TRUE,
            &prev
          );
    DNX_SAND_CHECK_FUNC_RESULT(res, 10, exit);
    *header_ptr = prev;
  }

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in dnx_sand_arr_mem_allocator_header_get()",0,0);
}


static uint32
  dnx_sand_arr_mem_allocator_set_next_free(
    DNX_SAND_IN  DNX_SAND_ARR_MEM_ALLOCATOR_INFO     *arr_mem_info,
    DNX_SAND_IN  DNX_SAND_ARR_MEM_ALLOCATOR_PTR      block_ptr,
    DNX_SAND_IN  uint8                       down,
    DNX_SAND_IN  DNX_SAND_ARR_MEM_ALLOCATOR_PTR      free_ptr
  )
{
  DNX_SAND_ARR_MEM_ALLOCATOR_PTR
    tmp_ptr;
  DNX_SAND_ARR_MEM_ALLOCATOR_ENTRY
    entry;
  uint32
    block_size,
    next_ptr;
  uint32
    res;

  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);

  tmp_ptr = free_ptr;

  if (block_ptr == DNX_SAND_ARR_MEM_ALLOCATOR_LIST_END && down)
  {
    res = arr_mem_info->free_set_fun(
            arr_mem_info->instance_prim_handle,
            DNX_SAND_ARR_MEM_ALLOCATOR_ACTIVE_INST(arr_mem_info,arr_mem_info->instance_sec_handle),
            free_ptr
          );
    DNX_SAND_CHECK_FUNC_RESULT(res, 10, exit);
    goto exit;
  }
  else if (block_ptr == DNX_SAND_ARR_MEM_ALLOCATOR_LIST_END)
  {
	  goto exit;
  }

  res = arr_mem_info->entry_get_fun(
          arr_mem_info->instance_prim_handle,
          DNX_SAND_ARR_MEM_ALLOCATOR_ACTIVE_INST(arr_mem_info,arr_mem_info->instance_sec_handle),
          block_ptr,
          &entry
        );
  DNX_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  block_size = DNX_SAND_ARR_MEM_ALLOCATOR_ENTRY_SIZE(entry);

  if (down)
  {
    res = dnx_sand_set_field(
            &entry,
            DNX_SAND_ARR_MEM_ALLOCATOR_ENTRY_NEXT_PTR_MSB,
            DNX_SAND_ARR_MEM_ALLOCATOR_ENTRY_NEXT_PTR_LSB,
            tmp_ptr
          );
    DNX_SAND_CHECK_FUNC_RESULT(res, 10, exit);

    res = arr_mem_info->entry_set_fun(
          arr_mem_info->instance_prim_handle,
          DNX_SAND_ARR_MEM_ALLOCATOR_ACTIVE_INST(arr_mem_info,arr_mem_info->instance_sec_handle),
          block_ptr,
          &entry
        );
    DNX_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  }
  next_ptr = block_ptr + block_size - 1;

  res = arr_mem_info->entry_get_fun(
          arr_mem_info->instance_prim_handle,
          DNX_SAND_ARR_MEM_ALLOCATOR_ACTIVE_INST(arr_mem_info,arr_mem_info->instance_sec_handle),
          next_ptr,
          &entry
        );
  DNX_SAND_CHECK_FUNC_RESULT(res, 20, exit);
  if (down)
  {
    res = dnx_sand_set_field(
            &entry,
            DNX_SAND_ARR_MEM_ALLOCATOR_ENTRY_NEXT_PTR_MSB,
            DNX_SAND_ARR_MEM_ALLOCATOR_ENTRY_NEXT_PTR_LSB,
            tmp_ptr >> DNX_SAND_ARR_MEM_ALLOCATOR_ENTRY_NEXT_PTR_SIZE
          );
    DNX_SAND_CHECK_FUNC_RESULT(res, 10, exit);
  }
  else
  {
    res = dnx_sand_set_field(
            &entry,
            DNX_SAND_ARR_MEM_ALLOCATOR_ENTRY_PREV_PTR_MSB,
            DNX_SAND_ARR_MEM_ALLOCATOR_ENTRY_PREV_PTR_LSB,
            tmp_ptr
          );
    DNX_SAND_CHECK_FUNC_RESULT(res, 10, exit);
  }

  res = arr_mem_info->entry_set_fun(
          arr_mem_info->instance_prim_handle,
          DNX_SAND_ARR_MEM_ALLOCATOR_ACTIVE_INST(arr_mem_info,arr_mem_info->instance_sec_handle),
          next_ptr,
          &entry
        );
  DNX_SAND_CHECK_FUNC_RESULT(res, 10, exit);


exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in dnx_sand_arr_mem_allocator_set_next_free()",0,0);
}


static uint32
  dnx_sand_arr_mem_allocator_remove_free_entry(
    DNX_SAND_IN  DNX_SAND_ARR_MEM_ALLOCATOR_INFO     *arr_mem_info,
    DNX_SAND_INOUT  DNX_SAND_ARR_MEM_ALLOCATOR_PTR   free_ptr
  )
{
  DNX_SAND_ARR_MEM_ALLOCATOR_PTR
    next,
    prev;
  uint32
    res;

  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);

  next = free_ptr;
  res = dnx_sand_arr_mem_allocator_get_next_free(
          arr_mem_info,
          TRUE,
          &next
        );
  DNX_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  prev = free_ptr;
  res = dnx_sand_arr_mem_allocator_get_next_free(
          arr_mem_info,
          FALSE,
          &prev
        );
  DNX_SAND_CHECK_FUNC_RESULT(res, 20, exit);

  res = dnx_sand_arr_mem_allocator_set_next_free(
          arr_mem_info,
          next,
          FALSE,
          prev
        );
  DNX_SAND_CHECK_FUNC_RESULT(res, 30, exit);

  res = dnx_sand_arr_mem_allocator_set_next_free(
          arr_mem_info,
          prev,
          TRUE,
          next
        );
  DNX_SAND_CHECK_FUNC_RESULT(res, 40, exit);


exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in dnx_sand_arr_mem_allocator_remove_free_entry()",0,0);
}

static uint32
  dnx_sand_arr_mem_allocator_upadte_list(
    DNX_SAND_INOUT  DNX_SAND_ARR_MEM_ALLOCATOR_INFO        *arr_mem_info,
    DNX_SAND_IN  DNX_SAND_ARR_MEM_ALLOCATOR_PTR            prev,
    DNX_SAND_IN  DNX_SAND_ARR_MEM_ALLOCATOR_PTR            curr,
    DNX_SAND_IN  DNX_SAND_ARR_MEM_ALLOCATOR_PTR            next
  )
{
  uint32
    res;

  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(arr_mem_info);
 /*
  * update next to point to me
  */

  if (next != DNX_SAND_ARR_MEM_ALLOCATOR_LIST_END)
  {
    res = dnx_sand_arr_mem_allocator_set_next_free(
            arr_mem_info,
            next,
            FALSE,
            curr
          );
    DNX_SAND_CHECK_FUNC_RESULT(res, 10, exit);
  }
 /*
  * update prev to point to me
  */
  if (prev != DNX_SAND_ARR_MEM_ALLOCATOR_LIST_END)
  {
    res = dnx_sand_arr_mem_allocator_set_next_free(
            arr_mem_info,
            prev,
            TRUE,
            curr
          );
    DNX_SAND_CHECK_FUNC_RESULT(res, 10, exit);
  }
  else
  {
    res = arr_mem_info->free_set_fun(
            arr_mem_info->instance_prim_handle,
            DNX_SAND_ARR_MEM_ALLOCATOR_ACTIVE_INST(arr_mem_info,arr_mem_info->instance_sec_handle),
            curr
          );
    DNX_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  }

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in dnx_sand_arr_mem_allocator_upadte_list()",0,0);
}


static uint32
  dnx_sand_arr_mem_allocator_build_free_entry(
    DNX_SAND_IN  uint32                 size,
    DNX_SAND_IN   DNX_SAND_ARR_MEM_ALLOCATOR_PTR     next_ptr,
    DNX_SAND_IN   DNX_SAND_ARR_MEM_ALLOCATOR_PTR     prev_ptr,
    DNX_SAND_OUT  DNX_SAND_ARR_MEM_ALLOCATOR_ENTRY   entry[DNX_SAND_ARR_MEM_ALLOCATOR_FREE_ENTRY_LEN]
  )
{
  uint32
    free_ptr_part,
    indx;

  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(entry);


  for (indx = 0; indx < 2; ++indx)
  {
    if (indx == 0)
    {
      entry[indx] = size;
    }
    else
    {
      entry[indx] = prev_ptr;
    }

    free_ptr_part =
      DNX_SAND_GET_BITS_RANGE(
        next_ptr,
        (DNX_SAND_ARR_MEM_ALLOCATOR_ENTRY_NEXT_PTR_SIZE * (indx + 1)) - 1,
        DNX_SAND_ARR_MEM_ALLOCATOR_ENTRY_NEXT_PTR_SIZE * indx
      );
    entry[indx] |=
      DNX_SAND_SET_BITS_RANGE(
        free_ptr_part,
        DNX_SAND_ARR_MEM_ALLOCATOR_ENTRY_NEXT_PTR_MSB,
        DNX_SAND_ARR_MEM_ALLOCATOR_ENTRY_NEXT_PTR_LSB
      );
  }
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in dnx_sand_arr_mem_allocator_build_free_entry()",0,0);
}

static uint32
  dnx_sand_arr_mem_allocator_build_used_entry(
    DNX_SAND_IN  uint32                 size,
    DNX_SAND_OUT  DNX_SAND_ARR_MEM_ALLOCATOR_ENTRY   *entry
  )
{

  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(entry);

  *entry = size;
  *entry |=
    DNX_SAND_SET_BITS_RANGE(
      TRUE,
      DNX_SAND_ARR_MEM_ALLOCATOR_ENTRY_USE_MSB,
      DNX_SAND_ARR_MEM_ALLOCATOR_ENTRY_USE_LSB
    );
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in dnx_sand_arr_mem_allocator_build_used_entry()",0,0);
}

/* allocate given entry (aligned_ptr)
   free_block_ptr points to start of free-block*/
static
uint32
  dnx_sand_arr_mem_allocator_malloc_entry(
    DNX_SAND_INOUT   DNX_SAND_ARR_MEM_ALLOCATOR_INFO     *arr_mem_info,
    DNX_SAND_IN  DNX_SAND_ARR_MEM_ALLOCATOR_PTR          prev_ptr,
    DNX_SAND_IN  DNX_SAND_ARR_MEM_ALLOCATOR_PTR          free_block_ptr,
    DNX_SAND_IN  DNX_SAND_ARR_MEM_ALLOCATOR_PTR          aligned_ptr,
    DNX_SAND_IN  DNX_SAND_ARR_MEM_ALLOCATOR_PTR          next_ptr,
    DNX_SAND_IN  uint32                            alloced_size
  )
{
  DNX_SAND_ARR_MEM_ALLOCATOR_PTR
    new_prev,
    prev = prev_ptr,
    next = next_ptr,
    aligned = aligned_ptr,
    tmp_ptr = free_block_ptr,
    new_next;
  uint32
    block_size,
    tmp_size = alloced_size,
    align_free_size;
  DNX_SAND_ARR_MEM_ALLOCATOR_ENTRY
    free_entry[DNX_SAND_ARR_MEM_ALLOCATOR_FREE_ENTRY_LEN],
    align_free_entry[DNX_SAND_ARR_MEM_ALLOCATOR_FREE_ENTRY_LEN],
    used_entry;
  uint32
    res;

  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(DNX_SAND_ARR_MEM_ALLOCATOR_MALLOC);
 /*
  * read the free block entries
  */
  res = arr_mem_info->entry_get_fun(
          arr_mem_info->instance_prim_handle,
          DNX_SAND_ARR_MEM_ALLOCATOR_ACTIVE_INST(arr_mem_info,arr_mem_info->instance_sec_handle),
          tmp_ptr,
          &(free_entry[0])
        );
  DNX_SAND_CHECK_FUNC_RESULT(res, 30, exit);

  block_size = DNX_SAND_ARR_MEM_ALLOCATOR_ENTRY_SIZE(free_entry[0]);
  align_free_size = aligned - tmp_ptr;

 /*
  * build and write the used block entries
  */
  res = dnx_sand_arr_mem_allocator_build_used_entry(
          tmp_size,
          &used_entry
        );
  DNX_SAND_CHECK_FUNC_RESULT(res, 40, exit);

  res = arr_mem_info->entry_set_fun(
          arr_mem_info->instance_prim_handle,
          DNX_SAND_ARR_MEM_ALLOCATOR_ACTIVE_INST(arr_mem_info,arr_mem_info->instance_sec_handle),
          aligned,
          &used_entry
        );
  DNX_SAND_CHECK_FUNC_RESULT(res, 50, exit);

  res = arr_mem_info->entry_set_fun(
          arr_mem_info->instance_prim_handle,
          DNX_SAND_ARR_MEM_ALLOCATOR_ACTIVE_INST(arr_mem_info,arr_mem_info->instance_sec_handle),
          aligned + tmp_size - 1,
          &used_entry
        );
  DNX_SAND_CHECK_FUNC_RESULT(res, 60, exit);

 /*
  * build and write the free block entries
  */
 /*
  * if were alignment
  */
  if (align_free_size != 0)
  {
   /*
    * build free block on the beginning of the free block.
    */
    res = dnx_sand_arr_mem_allocator_build_free_entry(
          align_free_size,
          next,
          prev,
          align_free_entry
        );
    DNX_SAND_CHECK_FUNC_RESULT(res, 70, exit);

    res = arr_mem_info->entry_set_fun(
            arr_mem_info->instance_prim_handle,
            DNX_SAND_ARR_MEM_ALLOCATOR_ACTIVE_INST(arr_mem_info,arr_mem_info->instance_sec_handle),
            tmp_ptr,
            &(align_free_entry[0])
          );
    DNX_SAND_CHECK_FUNC_RESULT(res, 80, exit);

    res = arr_mem_info->entry_set_fun(
            arr_mem_info->instance_prim_handle,
            DNX_SAND_ARR_MEM_ALLOCATOR_ACTIVE_INST(arr_mem_info,arr_mem_info->instance_sec_handle),
            tmp_ptr + align_free_size - 1,
            &(align_free_entry[1])
          );
    DNX_SAND_CHECK_FUNC_RESULT(res, 90, exit);

    block_size -= align_free_size;
   /*
    * the prev of the block to allocate from is me.
    */
    prev = tmp_ptr;
   /*
    * the pointer to the memory to use is in aligned.
    */
    tmp_ptr = aligned;
  }
  block_size -= tmp_size;
 /*
  * update free list
  */
  if (block_size > 0)
  {
    new_prev = tmp_ptr + tmp_size;
    new_next = tmp_ptr + tmp_size;
  }
  else
  {
    new_prev = prev;
    new_next = next;
  }
  if (prev == DNX_SAND_ARR_MEM_ALLOCATOR_LIST_END)
  {
    res = arr_mem_info->free_set_fun(
            arr_mem_info->instance_prim_handle,
            DNX_SAND_ARR_MEM_ALLOCATOR_ACTIVE_INST(arr_mem_info,arr_mem_info->instance_sec_handle),
            new_next
          );
    DNX_SAND_CHECK_FUNC_RESULT(res, 100, exit);
  }
  else
  {
    res = dnx_sand_arr_mem_allocator_set_next_free(
            arr_mem_info,
            prev,
            TRUE,
            new_next
          );
    DNX_SAND_CHECK_FUNC_RESULT(res, 110, exit);
  }

  if (next != DNX_SAND_ARR_MEM_ALLOCATOR_LIST_END)
  {
    res = dnx_sand_arr_mem_allocator_set_next_free(
            arr_mem_info,
            next,
            FALSE,
            new_prev
          );
    DNX_SAND_CHECK_FUNC_RESULT(res, 120, exit);
  }

  if (block_size > 0)
  {
   /*
    * if was no alignment
    */
    if (align_free_size == 0)
    {
      res = dnx_sand_set_field(
              &(free_entry[0]),
              DNX_SAND_ARR_MEM_ALLOCATOR_ENTRY_SIZE_MSB,
              DNX_SAND_ARR_MEM_ALLOCATOR_ENTRY_SIZE_LSB,
              block_size
            );
      DNX_SAND_CHECK_FUNC_RESULT(res, 130, exit);

      res = arr_mem_info->entry_set_fun(
              arr_mem_info->instance_prim_handle,
              DNX_SAND_ARR_MEM_ALLOCATOR_ACTIVE_INST(arr_mem_info,arr_mem_info->instance_sec_handle),
              tmp_ptr + tmp_size,
              &(free_entry[0])
            );
      DNX_SAND_CHECK_FUNC_RESULT(res, 140, exit);
    }
    else /* there need to calculate again*/
    {
      /*
       * This calculation works even if was no alignment
       * but there is no need and to save time.
       */
      res = dnx_sand_arr_mem_allocator_build_free_entry(
              block_size,
              next,
              prev,
              free_entry
            );
      DNX_SAND_CHECK_FUNC_RESULT(res, 150, exit);

      res = arr_mem_info->entry_set_fun(
              arr_mem_info->instance_prim_handle,
              DNX_SAND_ARR_MEM_ALLOCATOR_ACTIVE_INST(arr_mem_info,arr_mem_info->instance_sec_handle),
              tmp_ptr + tmp_size,
              &(free_entry[0])
            );
      DNX_SAND_CHECK_FUNC_RESULT(res, 160, exit);

      res = arr_mem_info->entry_set_fun(
              arr_mem_info->instance_prim_handle,
              DNX_SAND_ARR_MEM_ALLOCATOR_ACTIVE_INST(arr_mem_info,arr_mem_info->instance_sec_handle),
              tmp_ptr + tmp_size + block_size - 1,
              &(free_entry[1])
            );
      DNX_SAND_CHECK_FUNC_RESULT(res, 170, exit);
    }
  }
  exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in dnx_sand_arr_mem_allocator_malloc()",0,0);
}

/*
 * move block of entries
 */
static uint32
  dnx_sand_arr_mem_allocator_move_block(
    DNX_SAND_INOUT  DNX_SAND_ARR_MEM_ALLOCATOR_INFO   *arr_mem_info,
    DNX_SAND_IN  DNX_SAND_ARR_MEM_ALLOCATOR_PTR       from,
    DNX_SAND_IN  DNX_SAND_ARR_MEM_ALLOCATOR_PTR       to,
    DNX_SAND_IN  uint32                         nof_entris,
    DNX_SAND_IN  uint32                         flags
  )
{
  DNX_SAND_ARR_MEM_ALLOCATOR_PTR
    next_free,
    prev_free;
  DNX_SAND_ARR_MEM_ALLOCATOR_ENTRY
    entry;
  uint32
    block_size=0,
    entry_index,
    from_indx,
    data[3],
    to_indx;
  uint32
    res;

  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);


  /* if size given by input parameters */
  if (flags & DNX_SAND_ARR_MEM_ALLOCATOR_MOVE_BLK_SIZE_VLD)
  {
    block_size = nof_entris;
  }
  else
  {
    /* if size is required */
    if (flags & (DNX_SAND_ARR_MEM_ALLOCATOR_MOVE_UPDATE_REF|DNX_SAND_ARR_MEM_ALLOCATOR_MOVE_UPDATE_CONT|DNX_SAND_ARR_MEM_ALLOCATOR_MOVE_COPY))
    {
      res = arr_mem_info->entry_get_fun(
              arr_mem_info->instance_prim_handle,
              DNX_SAND_ARR_MEM_ALLOCATOR_ACTIVE_INST(arr_mem_info,arr_mem_info->instance_sec_handle),
              from,
              &entry
            );
      DNX_SAND_CHECK_FUNC_RESULT(res, 10, exit);
      if (DNX_SAND_ARR_MEM_ALLOCATOR_ENTRY_IS_USED(entry))
      {
        block_size = DNX_SAND_ARR_MEM_ALLOCATOR_ENTRY_SIZE(entry);
      }
      else /*no used entry jump to next block.*/
      {
        block_size = 0;
        DNX_SAND_SET_ERROR_CODE(DNX_SAND_GEN_ERR, 40, exit);
      }
    }
 }
  if (flags & DNX_SAND_ARR_MEM_ALLOCATOR_MOVE_COPY)
  {
    /* get allocated block size, prev-free, next-free */
    prev_free = to;
    res = dnx_sand_arr_mem_allocator_get_next_free(
            arr_mem_info,
            FALSE,
            &prev_free
          );
    DNX_SAND_CHECK_FUNC_RESULT(res, 20, exit);

    next_free = to;
    res = dnx_sand_arr_mem_allocator_get_next_free(
            arr_mem_info,
            TRUE,
            &next_free
          );
    DNX_SAND_CHECK_FUNC_RESULT(res, 30, exit);

    /* assuming, moving to start of empty block */
    res = dnx_sand_arr_mem_allocator_malloc_entry(arr_mem_info,prev_free,to,to,next_free,block_size);
    DNX_SAND_CHECK_FUNC_RESULT(res, 40, exit);
  }

  /* update content*/
  if (flags & DNX_SAND_ARR_MEM_ALLOCATOR_MOVE_UPDATE_CONT)
  {
    from_indx = from;
    to_indx = to;

    for (entry_index = 0 ; entry_index < block_size; ++from_indx,++to_indx,++entry_index)
    {
      res = arr_mem_info->read_fun(
              arr_mem_info->instance_prim_handle,
              DNX_SAND_ARR_MEM_ALLOCATOR_ACTIVE_INST(arr_mem_info,arr_mem_info->instance_sec_handle),
              from_indx,
              data
            );
     DNX_SAND_CHECK_FUNC_RESULT(res, 50, exit);

      res = arr_mem_info->write_fun(
              arr_mem_info->instance_prim_handle,
              DNX_SAND_ARR_MEM_ALLOCATOR_ACTIVE_INST(arr_mem_info,arr_mem_info->instance_sec_handle),
              to_indx,
              data
            );
      DNX_SAND_CHECK_FUNC_RESULT(res, 60, exit);
      }
  }

  
  if (flags & DNX_SAND_ARR_MEM_ALLOCATOR_MOVE_UPDATE_REF && arr_mem_info->entry_move_fun)
  {
    from_indx = from;
    to_indx = to;

    for (entry_index = 0 ; entry_index < block_size; ++from_indx,++to_indx,++entry_index)
    {
      /* read moved value */
      res = arr_mem_info->read_fun(
              arr_mem_info->instance_prim_handle,
              DNX_SAND_ARR_MEM_ALLOCATOR_ACTIVE_INST(arr_mem_info,arr_mem_info->instance_sec_handle),
              to_indx,
              data
            );
     DNX_SAND_CHECK_FUNC_RESULT(res, 70, exit);

     /* call alignment callback */
      res = arr_mem_info->entry_move_fun(
              arr_mem_info->instance_prim_handle,
              DNX_SAND_ARR_MEM_ALLOCATOR_ACTIVE_INST(arr_mem_info,arr_mem_info->instance_sec_handle),
              data,
              from_indx,
              to_indx
            );
        DNX_SAND_CHECK_FUNC_RESULT(res, 80, exit);

    }
  }
  /* remove old */
  if (flags & DNX_SAND_ARR_MEM_ALLOCATOR_MOVE_FREE)
  {
    /* free old block */
    res = dnx_sand_arr_mem_allocator_free(arr_mem_info,from);
    DNX_SAND_CHECK_FUNC_RESULT(res, 90, exit);
  }

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in dnx_sand_arr_mem_allocator_move_block()",0,0);
}




static uint32
  dnx_sand_arr_mem_allocator_get_first_fit(
    DNX_SAND_INOUT  DNX_SAND_ARR_MEM_ALLOCATOR_INFO   *arr_mem_info,
    DNX_SAND_INOUT  uint32                      *size,
    DNX_SAND_IN  uint32                         alignment,
    DNX_SAND_OUT  DNX_SAND_ARR_MEM_ALLOCATOR_PTR      *ptr,
    DNX_SAND_OUT  DNX_SAND_ARR_MEM_ALLOCATOR_PTR      *aligned_ptr,
    DNX_SAND_OUT  DNX_SAND_ARR_MEM_ALLOCATOR_PTR      *prev_ptr,
    DNX_SAND_OUT  DNX_SAND_ARR_MEM_ALLOCATOR_PTR      *next_ptr
  )
{
  DNX_SAND_ARR_MEM_ALLOCATOR_PTR
    free_ptr,
    aligned_start;
  DNX_SAND_ARR_MEM_ALLOCATOR_ENTRY
    entry;
  uint32
    entry_size;
  uint32
    res;

  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);

  DNX_SAND_CHECK_NULL_INPUT(arr_mem_info);
  DNX_SAND_CHECK_NULL_INPUT(ptr);

  *prev_ptr = DNX_SAND_ARR_MEM_ALLOCATOR_LIST_END;
  *next_ptr = DNX_SAND_ARR_MEM_ALLOCATOR_LIST_END;

  res = arr_mem_info->free_get_fun(
          arr_mem_info->instance_prim_handle,
          DNX_SAND_ARR_MEM_ALLOCATOR_ACTIVE_INST(arr_mem_info,arr_mem_info->instance_sec_handle),
          &free_ptr
        );
  DNX_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  while (free_ptr != DNX_SAND_ARR_MEM_ALLOCATOR_LIST_END)
  {
    res = arr_mem_info->entry_get_fun(
            arr_mem_info->instance_prim_handle,
            DNX_SAND_ARR_MEM_ALLOCATOR_ACTIVE_INST(arr_mem_info,arr_mem_info->instance_sec_handle),
            free_ptr,
            &entry
          );
    DNX_SAND_CHECK_FUNC_RESULT(res, 10, exit);

    entry_size = DNX_SAND_ARR_MEM_ALLOCATOR_ENTRY_SIZE(entry);
    if (entry_size >= *size)
    {
      aligned_start = DNX_SAND_ARR_MEM_ALLOCATOR_GET_ALIGNED(free_ptr,alignment);
     /*
      * don't leave behind free blocks of size 1.
      */
      if (aligned_start - free_ptr == 1)
      {
        aligned_start += alignment;
      }
     /*
      * check if entry can contain block with alignment
      */
      if (entry_size >= *size + (aligned_start - free_ptr))
      {
        if (entry_size - (*size + (aligned_start - free_ptr)) == 1)
        {
          ++(*size);
        }
        *ptr = free_ptr;
        *next_ptr = *ptr;
        res = dnx_sand_arr_mem_allocator_get_next_free(
                arr_mem_info,
                TRUE,
                next_ptr
              );
        DNX_SAND_CHECK_FUNC_RESULT(res, 20, exit);
        *aligned_ptr = aligned_start;

        /* if support defragment then check this malloced entry doesn't get into reserved area */
        if (arr_mem_info->support_defragment)
        {
          if (*aligned_ptr + *size - 1 >= arr_mem_info->nof_entries - arr_mem_info->max_block_size)
          {
            *ptr = DNX_SAND_ARR_MEM_ALLOCATOR_NULL;
          }
        }
        
        goto exit;
      }
    }
    *prev_ptr = free_ptr;
    res = dnx_sand_arr_mem_allocator_get_next_free(
            arr_mem_info,
            TRUE,
            &free_ptr
          );
    DNX_SAND_CHECK_FUNC_RESULT(res, 20, exit);
  }

  *ptr = DNX_SAND_ARR_MEM_ALLOCATOR_NULL;

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in dnx_sand_arr_mem_allocator_get_first_fit()",0,0);
}

/*
* } end of internal functions
*/

uint32
  dnx_sand_arr_mem_allocator_create(
    DNX_SAND_INOUT   DNX_SAND_ARR_MEM_ALLOCATOR_INFO      *arr_mem_info
  )
{
  DNX_SAND_ARR_MEM_ALLOCATOR_ENTRY
    entry[DNX_SAND_ARR_MEM_ALLOCATOR_FREE_ENTRY_LEN];
  uint32
    res;

  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(DNX_SAND_ARR_MEM_ALLOCATOR_CREATE);

  DNX_SAND_CHECK_NULL_INPUT(arr_mem_info);
  DNX_SAND_CHECK_NULL_INPUT(arr_mem_info->entry_get_fun);
  DNX_SAND_CHECK_NULL_INPUT(arr_mem_info->entry_set_fun);

  if (arr_mem_info->nof_entries < DNX_SAND_ARR_MEM_ALLOCATOR_MIN_MALLOC_SIZE || arr_mem_info->nof_entries - 1 > DNX_SAND_ARR_MEM_ALLOCATOR_LIST_END )
  {
    DNX_SAND_SET_ERROR_CODE(DNX_SAND_ARR_MEM_ALLOCATOR_NOF_LINES_OUT_OF_RANGE_ERR, 10, exit);
  }
 /*
  * allocate the array
  */
  arr_mem_info->arr_mem_allocator_data.array = 
      (DNX_SAND_ARR_MEM_ALLOCATOR_ENTRY*) dnx_sand_os_malloc_any_size(sizeof(DNX_SAND_ARR_MEM_ALLOCATOR_ENTRY) * arr_mem_info->nof_entries, "arr_mem_info->arr_mem_allocator_data.array");
  if (!arr_mem_info->arr_mem_allocator_data.array)
  {
    DNX_SAND_SET_ERROR_CODE(DNX_SAND_MALLOC_FAIL, 20, exit);
  }
 /*
  * reset memory
  */
  dnx_sand_os_memset(arr_mem_info->arr_mem_allocator_data.array,0x0,sizeof(DNX_SAND_ARR_MEM_ALLOCATOR_ENTRY) * arr_mem_info->nof_entries);

 /*
  * allocate mem shadow
  */
  arr_mem_info->arr_mem_allocator_data.mem_shadow = (uint32*) dnx_sand_os_malloc_any_size(sizeof(uint32) * arr_mem_info->nof_entries, "arr_mem_info->arr_mem_allocator_data.mem_shadow");
  if (!arr_mem_info->arr_mem_allocator_data.mem_shadow)
  {
    DNX_SAND_SET_ERROR_CODE(DNX_SAND_MALLOC_FAIL, 20, exit);
  }
  dnx_sand_os_memset(arr_mem_info->arr_mem_allocator_data.mem_shadow,0x0,sizeof(uint32) * arr_mem_info->nof_entries);



  if (arr_mem_info->support_caching)
  {
   /*
    * allocate cache array
    */
    arr_mem_info->arr_mem_allocator_data.array_cache = 
        (DNX_SAND_ARR_MEM_ALLOCATOR_ENTRY*) dnx_sand_os_malloc_any_size(sizeof(DNX_SAND_ARR_MEM_ALLOCATOR_ENTRY) * arr_mem_info->nof_entries, "arr_mem_info->arr_mem_allocator_data.array_cache");
    if (!arr_mem_info->arr_mem_allocator_data.array_cache)
    {
      DNX_SAND_SET_ERROR_CODE(DNX_SAND_MALLOC_FAIL, 20, exit);
    }
   /*
    * reset memory
    */
    dnx_sand_os_memset(arr_mem_info->arr_mem_allocator_data.array_cache,0x0,sizeof(DNX_SAND_ARR_MEM_ALLOCATOR_ENTRY) * arr_mem_info->nof_entries);

    /* updated indexes */
    arr_mem_info->arr_mem_allocator_data.update_indexes = 
        (DNX_SAND_ARR_MEM_ALLOCATOR_ENTRY*) dnx_sand_os_malloc_any_size(sizeof(DNX_SAND_ARR_MEM_ALLOCATOR_ENTRY) * arr_mem_info->nof_entries, "arr_mem_info->arr_mem_allocator_data.update_indexes");
    if (!arr_mem_info->arr_mem_allocator_data.update_indexes)
    {
      DNX_SAND_SET_ERROR_CODE(DNX_SAND_MALLOC_FAIL, 20, exit);
    }
    arr_mem_info->arr_mem_allocator_data.nof_updates = 0;
   /*
    * allocate mem shadow
    */
    arr_mem_info->arr_mem_allocator_data.mem_shadow_cache = (uint32*) dnx_sand_os_malloc_any_size(sizeof(uint32) * arr_mem_info->nof_entries, "mem_shadow_cache");
    if (!arr_mem_info->arr_mem_allocator_data.mem_shadow_cache)
    {
      DNX_SAND_SET_ERROR_CODE(DNX_SAND_MALLOC_FAIL, 20, exit);
    }
    dnx_sand_os_memset(arr_mem_info->arr_mem_allocator_data.mem_shadow_cache,0x0,sizeof(uint32) * arr_mem_info->nof_entries);
  }

  res = dnx_sand_arr_mem_allocator_build_free_entry(
          DNX_SAND_ARR_MEM_ALLOCATOR_LAST_INDEX(arr_mem_info)+1,
          DNX_SAND_ARR_MEM_ALLOCATOR_LIST_END,
          DNX_SAND_ARR_MEM_ALLOCATOR_LIST_END,
          entry
        );
  DNX_SAND_CHECK_FUNC_RESULT(res, 30, exit);
 /*
  * write to entry zero:
  * free block of whole memory size and next free points to NULL
  */
  res = arr_mem_info->entry_set_fun(
          arr_mem_info->instance_prim_handle,
          DNX_SAND_ARR_MEM_ALLOCATOR_ACTIVE_INST(arr_mem_info,arr_mem_info->instance_sec_handle),
          0,
          &(entry[0])
        );
  DNX_SAND_CHECK_FUNC_RESULT(res, 40, exit);

  res = arr_mem_info->entry_set_fun(
          arr_mem_info->instance_prim_handle,
          DNX_SAND_ARR_MEM_ALLOCATOR_ACTIVE_INST(arr_mem_info,arr_mem_info->instance_sec_handle),
          DNX_SAND_ARR_MEM_ALLOCATOR_LAST_INDEX(arr_mem_info),
          &(entry[1])
        );
  DNX_SAND_CHECK_FUNC_RESULT(res, 50, exit);

  res = arr_mem_info->free_set_fun(
          arr_mem_info->instance_prim_handle,
          DNX_SAND_ARR_MEM_ALLOCATOR_ACTIVE_INST(arr_mem_info,arr_mem_info->instance_sec_handle),
          0
        );
  DNX_SAND_CHECK_FUNC_RESULT(res, 60, exit);

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in dnx_sand_arr_mem_allocator_create()",0,0);
}

uint32
  dnx_sand_arr_mem_allocator_clear(
    DNX_SAND_INOUT   DNX_SAND_ARR_MEM_ALLOCATOR_INFO      *arr_mem_info
  )
{
  DNX_SAND_ARR_MEM_ALLOCATOR_ENTRY
    entry[DNX_SAND_ARR_MEM_ALLOCATOR_FREE_ENTRY_LEN];
  uint32
    res;

  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(DNX_SAND_ARR_MEM_ALLOCATOR_CLEAR);

  DNX_SAND_CHECK_NULL_INPUT(arr_mem_info);
  DNX_SAND_CHECK_NULL_INPUT(arr_mem_info->entry_get_fun);
  DNX_SAND_CHECK_NULL_INPUT(arr_mem_info->entry_set_fun);

  if (arr_mem_info->nof_entries < DNX_SAND_ARR_MEM_ALLOCATOR_MIN_MALLOC_SIZE || arr_mem_info->nof_entries - 1 > DNX_SAND_ARR_MEM_ALLOCATOR_LIST_END )
  {
    DNX_SAND_SET_ERROR_CODE(DNX_SAND_ARR_MEM_ALLOCATOR_NOF_LINES_OUT_OF_RANGE_ERR, 10, exit);
  }
 /*
  * reset memory
  */
  if (arr_mem_info->support_caching && arr_mem_info->arr_mem_allocator_data.cache_enabled)
  {
    dnx_sand_os_memset(arr_mem_info->arr_mem_allocator_data.array_cache,0x0,sizeof(DNX_SAND_ARR_MEM_ALLOCATOR_ENTRY) * arr_mem_info->nof_entries);
    dnx_sand_os_memset(arr_mem_info->arr_mem_allocator_data.mem_shadow_cache,0x0,sizeof(uint32) * arr_mem_info->nof_entries);
    arr_mem_info->arr_mem_allocator_data.nof_updates = 0;
  }
  else
  {
    dnx_sand_os_memset(arr_mem_info->arr_mem_allocator_data.array,0x0,sizeof(DNX_SAND_ARR_MEM_ALLOCATOR_ENTRY) * arr_mem_info->nof_entries);
    dnx_sand_os_memset(arr_mem_info->arr_mem_allocator_data.mem_shadow,0x0,sizeof(uint32) * arr_mem_info->nof_entries);
  }
 /*
  * allocate the array
  */
  res = dnx_sand_arr_mem_allocator_build_free_entry(
          DNX_SAND_ARR_MEM_ALLOCATOR_LAST_INDEX(arr_mem_info)+1,
          DNX_SAND_ARR_MEM_ALLOCATOR_LIST_END,
          DNX_SAND_ARR_MEM_ALLOCATOR_LIST_END,
          entry
        );
  DNX_SAND_CHECK_FUNC_RESULT(res, 20, exit);
 /*
  * write to entry zero:
  * free block of whole memory size and next free points to NULL
  */
  res = arr_mem_info->entry_set_fun(
          arr_mem_info->instance_prim_handle,
          DNX_SAND_ARR_MEM_ALLOCATOR_ACTIVE_INST(arr_mem_info,arr_mem_info->instance_sec_handle),
          0,
          &(entry[0])
        );
  DNX_SAND_CHECK_FUNC_RESULT(res, 30, exit);

  res = arr_mem_info->entry_set_fun(
          arr_mem_info->instance_prim_handle,
          DNX_SAND_ARR_MEM_ALLOCATOR_ACTIVE_INST(arr_mem_info,arr_mem_info->instance_sec_handle),
          DNX_SAND_ARR_MEM_ALLOCATOR_LAST_INDEX(arr_mem_info),
          &(entry[1])
        );
  DNX_SAND_CHECK_FUNC_RESULT(res, 40, exit);

  res = arr_mem_info->free_set_fun(
          arr_mem_info->instance_prim_handle,
          DNX_SAND_ARR_MEM_ALLOCATOR_ACTIVE_INST(arr_mem_info,arr_mem_info->instance_sec_handle),
          0
        );
  DNX_SAND_CHECK_FUNC_RESULT(res, 50, exit);

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in dnx_sand_arr_mem_allocator_clear()",0,0);
}

uint32
  dnx_sand_arr_mem_allocator_destroy(
    DNX_SAND_INOUT   DNX_SAND_ARR_MEM_ALLOCATOR_INFO      *arr_mem_info
  )
{
  uint32
    res;

  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(DNX_SAND_ARR_MEM_ALLOCATOR_DESTROY);

  DNX_SAND_CHECK_NULL_INPUT(arr_mem_info);

  /* free array used memories*/

  res = dnx_sand_os_free_any_size(
    arr_mem_info->arr_mem_allocator_data.array
    );
  DNX_SAND_CHECK_FUNC_RESULT(res, 10, exit);
  res = dnx_sand_os_free_any_size(
    arr_mem_info->arr_mem_allocator_data.mem_shadow
    );
  DNX_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  if (arr_mem_info->support_caching)
  {
    res = dnx_sand_os_free_any_size(
      arr_mem_info->arr_mem_allocator_data.array_cache
      );
    DNX_SAND_CHECK_FUNC_RESULT(res, 20, exit);
    res = dnx_sand_os_free_any_size(
      arr_mem_info->arr_mem_allocator_data.update_indexes
      );
    DNX_SAND_CHECK_FUNC_RESULT(res, 30, exit);
      res = dnx_sand_os_free_any_size(
        arr_mem_info->arr_mem_allocator_data.mem_shadow_cache
    );
  DNX_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  }

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in dnx_sand_arr_mem_allocator_destroy()",0,0);
}

uint32
  dnx_sand_arr_mem_allocator_cache_set(
    DNX_SAND_INOUT   DNX_SAND_ARR_MEM_ALLOCATOR_INFO      *arr_mem_info,
    DNX_SAND_IN uint8                             enable
  )
{

  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(DNX_SAND_ARR_MEM_ALLOCATOR_MALLOC);

  DNX_SAND_CHECK_NULL_INPUT(arr_mem_info);
  if (!arr_mem_info->support_caching && enable)
  {
    DNX_SAND_SET_ERROR_CODE(DNX_SAND_ERR, 10, exit);
  }

  /* if already enabled and try to enable then do nothing */
  if (arr_mem_info->arr_mem_allocator_data.cache_enabled && enable)
  {
    goto exit;
  }
  
  arr_mem_info->arr_mem_allocator_data.cache_enabled = enable;

 /*
  * copy real state to cache state
  */
  if (enable)
  {
    arr_mem_info->arr_mem_allocator_data.free_list_cache = arr_mem_info->arr_mem_allocator_data.free_list;
    dnx_sand_os_memcpy(arr_mem_info->arr_mem_allocator_data.array_cache,arr_mem_info->arr_mem_allocator_data.array,sizeof(DNX_SAND_ARR_MEM_ALLOCATOR_ENTRY) * arr_mem_info->nof_entries);
    dnx_sand_os_memcpy(arr_mem_info->arr_mem_allocator_data.mem_shadow_cache,arr_mem_info->arr_mem_allocator_data.mem_shadow,sizeof(uint32) * arr_mem_info->nof_entries);
  }

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in dnx_sand_arr_mem_allocator_cache_set()",0,0);
}


uint32
  dnx_sand_arr_mem_allocator_commit(
    DNX_SAND_INOUT   DNX_SAND_ARR_MEM_ALLOCATOR_INFO      *arr_mem_info,
    DNX_SAND_IN uint32                             flags
  )
{
  uint32
    indx,
    nof_updates,
    offset,
    data[3];
  uint8 
    cache_org=FALSE;
  uint32
    res;

  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(DNX_SAND_ARR_MEM_ALLOCATOR_MALLOC);

  DNX_SAND_CHECK_NULL_INPUT(arr_mem_info);


  if (!arr_mem_info->support_caching)
  {
    DNX_SAND_SET_ERROR_CODE(DNX_SAND_ERR, 10, exit);
  }
  /* if no cache just exit */
  if (!arr_mem_info->arr_mem_allocator_data.cache_enabled)
  {
    goto exit;
  }

  cache_org = arr_mem_info->arr_mem_allocator_data.cache_enabled;

 /* disable caching  */
  arr_mem_info->arr_mem_allocator_data.cache_enabled = FALSE;

  nof_updates = arr_mem_info->arr_mem_allocator_data.nof_updates;
  /* walk over all changes and do them */
  for (indx = 0; indx < nof_updates; ++indx)
  {
    offset = arr_mem_info->arr_mem_allocator_data.update_indexes[indx];

    res = arr_mem_info->read_fun(
            arr_mem_info->instance_prim_handle,
            DNX_SAND_ARR_MEM_ALLOCATOR_CACHE_INST(arr_mem_info->instance_sec_handle),
            offset,
            data
          );
      DNX_SAND_CHECK_FUNC_RESULT(res, 20, exit);

    res = arr_mem_info->write_fun(
            arr_mem_info->instance_prim_handle,
            DNX_SAND_ARR_MEM_ALLOCATOR_ACTIVE_INST(arr_mem_info,arr_mem_info->instance_sec_handle),
            offset,
            data
          );
    DNX_SAND_CHECK_FUNC_RESULT(res, 20, exit);
  }

  /* updates are done */
  arr_mem_info->arr_mem_allocator_data.nof_updates = 0; 
  
 /*
  * copy cache state to real state
  */
  arr_mem_info->arr_mem_allocator_data.free_list = arr_mem_info->arr_mem_allocator_data.free_list_cache;
  dnx_sand_os_memcpy(arr_mem_info->arr_mem_allocator_data.array,arr_mem_info->arr_mem_allocator_data.array_cache,sizeof(DNX_SAND_ARR_MEM_ALLOCATOR_ENTRY) * arr_mem_info->nof_entries);
  dnx_sand_os_memcpy(arr_mem_info->arr_mem_allocator_data.mem_shadow,arr_mem_info->arr_mem_allocator_data.mem_shadow_cache,sizeof(uint32) * arr_mem_info->nof_entries);
  
exit:
  arr_mem_info->arr_mem_allocator_data.cache_enabled = cache_org;
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in dnx_sand_arr_mem_allocator_commit()",0,0);
}


uint32
  dnx_sand_arr_mem_allocator_rollback(
    DNX_SAND_INOUT   DNX_SAND_ARR_MEM_ALLOCATOR_INFO      *arr_mem_info,
    DNX_SAND_IN uint32                             flags
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(DNX_SAND_ARR_MEM_ALLOCATOR_MALLOC);

  DNX_SAND_CHECK_NULL_INPUT(arr_mem_info);
  if (!arr_mem_info->support_caching)
  {
    DNX_SAND_SET_ERROR_CODE(DNX_SAND_ERR, 10, exit);
  }
  /* if no cache just exit */
  if (!arr_mem_info->arr_mem_allocator_data.cache_enabled)
  {
    goto exit;
  }
  arr_mem_info->arr_mem_allocator_data.nof_updates = 0;

  arr_mem_info->arr_mem_allocator_data.free_list_cache = arr_mem_info->arr_mem_allocator_data.free_list;
  dnx_sand_os_memcpy(arr_mem_info->arr_mem_allocator_data.array_cache,arr_mem_info->arr_mem_allocator_data.array,sizeof(DNX_SAND_ARR_MEM_ALLOCATOR_ENTRY) * arr_mem_info->nof_entries);
  dnx_sand_os_memcpy(arr_mem_info->arr_mem_allocator_data.mem_shadow_cache,arr_mem_info->arr_mem_allocator_data.mem_shadow,sizeof(uint32) * arr_mem_info->nof_entries);
    
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in dnx_sand_arr_mem_allocator_rollback()",0,0);
}


uint32
  dnx_sand_arr_mem_allocator_malloc(
    DNX_SAND_INOUT   DNX_SAND_ARR_MEM_ALLOCATOR_INFO     *arr_mem_info,
    DNX_SAND_IN  uint32                            size,
    DNX_SAND_IN  uint32                            alignment,
    DNX_SAND_OUT  DNX_SAND_ARR_MEM_ALLOCATOR_PTR         *ptr
  )
{
  DNX_SAND_ARR_MEM_ALLOCATOR_PTR
    prev,
    aligned = 0,
    tmp_ptr = 0,
    next;
  uint32
    tmp_size = size,
    act_alignment;
  uint32
    res;

  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(DNX_SAND_ARR_MEM_ALLOCATOR_MALLOC);

  DNX_SAND_CHECK_NULL_INPUT(arr_mem_info);
  DNX_SAND_CHECK_NULL_INPUT(ptr);

  if(tmp_size < DNX_SAND_ARR_MEM_ALLOCATOR_MIN_MALLOC_SIZE)
  {
    tmp_size = DNX_SAND_ARR_MEM_ALLOCATOR_MIN_MALLOC_SIZE;
  }

  if (tmp_size > arr_mem_info->nof_entries)
  {
    DNX_SAND_SET_ERROR_CODE(DNX_SAND_ARR_MEM_ALLOCATOR_MALLOC_SIZE_OUT_OF_RANGE_ERR, 10, exit);
  }
  if (arr_mem_info->support_defragment && tmp_size > arr_mem_info->max_block_size )
  {
    DNX_SAND_SET_ERROR_CODE(DNX_SAND_ARR_MEM_ALLOCATOR_MALLOC_SIZE_OUT_OF_RANGE_ERR, 15, exit);
  }
  
  act_alignment = (alignment > 0)? alignment: 1;

  if (tmp_size % 2)
  {
	  tmp_size+=1;
  }
  
 /*
  * get pointer to the first fit block
  * block with size equal or bigger than the
  * required malloc size
  */
  res = dnx_sand_arr_mem_allocator_get_first_fit(
          arr_mem_info,
          &tmp_size,
          act_alignment,
          &tmp_ptr,
          &aligned,
          &prev,
          &next
        );
  DNX_SAND_CHECK_FUNC_RESULT(res, 20, exit);


  if (tmp_ptr == DNX_SAND_ARR_MEM_ALLOCATOR_NULL)
  {
    *ptr = DNX_SAND_ARR_MEM_ALLOCATOR_NULL;
    goto exit;
  }
 /*
  * now update the management info
  */
  res = dnx_sand_arr_mem_allocator_malloc_entry(arr_mem_info,prev,tmp_ptr,aligned,next,tmp_size);
  DNX_SAND_CHECK_FUNC_RESULT(res, 20, exit);

  *ptr = aligned;

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in dnx_sand_arr_mem_allocator_malloc()",0,0);
}

uint32
  dnx_sand_arr_mem_allocator_free(
    DNX_SAND_INOUT   DNX_SAND_ARR_MEM_ALLOCATOR_INFO     *arr_mem_info,
    DNX_SAND_IN  DNX_SAND_ARR_MEM_ALLOCATOR_PTR          ptr
  )
{
  DNX_SAND_ARR_MEM_ALLOCATOR_PTR
    prev,
    next;
  DNX_SAND_ARR_MEM_ALLOCATOR_ENTRY
    free_entries[DNX_SAND_ARR_MEM_ALLOCATOR_FREE_ENTRY_LEN];
  uint32
    before_size,
    after_size,
    used_block_size;
  DNX_SAND_ARR_MEM_ALLOCATOR_ENTRY
    used_entry,
    prev_entry,
    next_entry;
  uint8
    after_free,
    before_free;
  uint32
    res;

  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(DNX_SAND_ARR_MEM_ALLOCATOR_FREE);

  DNX_SAND_CHECK_NULL_INPUT(arr_mem_info);

  if (ptr > DNX_SAND_ARR_MEM_ALLOCATOR_LAST_INDEX(arr_mem_info))
  {
    DNX_SAND_SET_ERROR_CODE(DNX_SAND_ARR_MEM_ALLOCATOR_POINTER_OF_RANGE_ERR, 10, exit);
  }


  res = arr_mem_info->entry_get_fun(
          arr_mem_info->instance_prim_handle,
          DNX_SAND_ARR_MEM_ALLOCATOR_ACTIVE_INST(arr_mem_info,arr_mem_info->instance_sec_handle),
          ptr,
          &used_entry
        );
  DNX_SAND_CHECK_FUNC_RESULT(res, 20, exit);

  if (!DNX_SAND_ARR_MEM_ALLOCATOR_ENTRY_IS_USED(used_entry))
  {
    /* free not used entry */
    DNX_SAND_SET_ERROR_CODE(DNX_SAND_ARR_MEM_ALLOCATOR_POINTER_OF_RANGE_ERR, 220, exit);
  }

  used_block_size = DNX_SAND_ARR_MEM_ALLOCATOR_ENTRY_SIZE(used_entry);

  before_free = FALSE;
  after_free = FALSE;
  prev_entry = 0;
  next_entry = 0;
 /*
  * find the limits of the free block.
  */

 /*
  * check there is a free block above
  */
  if (ptr > 0)
  {
    res = arr_mem_info->entry_get_fun(
            arr_mem_info->instance_prim_handle,
            DNX_SAND_ARR_MEM_ALLOCATOR_ACTIVE_INST(arr_mem_info,arr_mem_info->instance_sec_handle),
            ptr - 1,
            &prev_entry
          );
    DNX_SAND_CHECK_FUNC_RESULT(res, 30, exit);
   /*
    * prev block is free.
    */
    before_free = (uint8)!(DNX_SAND_ARR_MEM_ALLOCATOR_ENTRY_IS_USED(prev_entry));
  }
  if (ptr + used_block_size <  DNX_SAND_ARR_MEM_ALLOCATOR_LAST_INDEX(arr_mem_info)+1)
  {
    res = arr_mem_info->entry_get_fun(
            arr_mem_info->instance_prim_handle,
            DNX_SAND_ARR_MEM_ALLOCATOR_ACTIVE_INST(arr_mem_info,arr_mem_info->instance_sec_handle),
            ptr + used_block_size,
            &next_entry
          );
    DNX_SAND_CHECK_FUNC_RESULT(res, 40, exit);
   /*
    * next block is free.
    */
    after_free = (uint8)!(DNX_SAND_ARR_MEM_ALLOCATOR_ENTRY_IS_USED(next_entry));
  }

  after_size = 0;
  before_size = 0;
  prev = 0;
  next = 0;

  if (!after_free && !before_free)
  {
    prev = DNX_SAND_ARR_MEM_ALLOCATOR_LIST_END;
    res = arr_mem_info->free_get_fun(
            arr_mem_info->instance_prim_handle,
            DNX_SAND_ARR_MEM_ALLOCATOR_ACTIVE_INST(arr_mem_info,arr_mem_info->instance_sec_handle),
            &next
          );
    DNX_SAND_CHECK_FUNC_RESULT(res, 50, exit);
  }
  else if (after_free && before_free)
  {
    res = dnx_sand_arr_mem_allocator_header_get(
              arr_mem_info,
              ptr - 1,
              &prev
            );
    DNX_SAND_CHECK_FUNC_RESULT(res, 60, exit);

    res = arr_mem_info->entry_get_fun(
            arr_mem_info->instance_prim_handle,
            DNX_SAND_ARR_MEM_ALLOCATOR_ACTIVE_INST(arr_mem_info,arr_mem_info->instance_sec_handle),
            prev,
            &prev_entry
          );
    DNX_SAND_CHECK_FUNC_RESULT(res, 70, exit);

    before_size = DNX_SAND_ARR_MEM_ALLOCATOR_ENTRY_SIZE(prev_entry);

    next = ptr + used_block_size;
    res = arr_mem_info->entry_get_fun(
            arr_mem_info->instance_prim_handle,
            DNX_SAND_ARR_MEM_ALLOCATOR_ACTIVE_INST(arr_mem_info,arr_mem_info->instance_sec_handle),
            next,
            &next_entry
          );
    DNX_SAND_CHECK_FUNC_RESULT(res, 80, exit);
    after_size = DNX_SAND_ARR_MEM_ALLOCATOR_ENTRY_SIZE(next_entry);
   /*
    * remove one of the free blocks. and update list.
    */
    res = dnx_sand_arr_mem_allocator_remove_free_entry(
            arr_mem_info,
            prev
          );
    DNX_SAND_CHECK_FUNC_RESULT(res, 90, exit);

    prev = ptr + used_block_size;
    res = dnx_sand_arr_mem_allocator_get_next_free(
            arr_mem_info,
            FALSE,
            &prev
          );
    DNX_SAND_CHECK_FUNC_RESULT(res, 100, exit);

    next = ptr + used_block_size;
    res = dnx_sand_arr_mem_allocator_get_next_free(
            arr_mem_info,
            TRUE,
            &next
          );
    DNX_SAND_CHECK_FUNC_RESULT(res, 110, exit);
  }
  else
  {
    if (before_free)
    {
      res = dnx_sand_arr_mem_allocator_header_get(
              arr_mem_info,
              ptr - 1,
              &prev
            );
      DNX_SAND_CHECK_FUNC_RESULT(res, 120, exit);

      next = prev;
      res = dnx_sand_arr_mem_allocator_get_next_free(
              arr_mem_info,
              TRUE,
              &next
            );
      DNX_SAND_CHECK_FUNC_RESULT(res, 130, exit);
    }
    if (after_free)
    {
      next = ptr + used_block_size;
      prev = next;
      res = dnx_sand_arr_mem_allocator_get_next_free(
              arr_mem_info,
              FALSE,
              &prev
            );
      DNX_SAND_CHECK_FUNC_RESULT(res, 140, exit);
    }
   /*
    * get next next
    */
    if (after_free)
    {
      res = arr_mem_info->entry_get_fun(
              arr_mem_info->instance_prim_handle,
              DNX_SAND_ARR_MEM_ALLOCATOR_ACTIVE_INST(arr_mem_info,arr_mem_info->instance_sec_handle),
              next,
              &next_entry
            );
      DNX_SAND_CHECK_FUNC_RESULT(res, 150, exit);
      after_size = DNX_SAND_ARR_MEM_ALLOCATOR_ENTRY_SIZE(next_entry);

      res = dnx_sand_arr_mem_allocator_get_next_free(
                arr_mem_info,
                TRUE,
                &next
              );
        DNX_SAND_CHECK_FUNC_RESULT(res, 160, exit);
    }
   /*
    * get prev prev
    */
    if (before_free)
    {
      res = arr_mem_info->entry_get_fun(
              arr_mem_info->instance_prim_handle,
              DNX_SAND_ARR_MEM_ALLOCATOR_ACTIVE_INST(arr_mem_info,arr_mem_info->instance_sec_handle),
              prev,
              &prev_entry
            );
      DNX_SAND_CHECK_FUNC_RESULT(res, 170, exit);

      before_size = DNX_SAND_ARR_MEM_ALLOCATOR_ENTRY_SIZE(prev_entry);

      res = dnx_sand_arr_mem_allocator_get_next_free(
              arr_mem_info,
              FALSE,
              &prev
            );
      DNX_SAND_CHECK_FUNC_RESULT(res, 180, exit);
    }
  }
 /*
  * build the unified block.
  */
  res = dnx_sand_arr_mem_allocator_build_free_entry(
          before_size + used_block_size + after_size,
          next,
          prev,
          free_entries
        );
  DNX_SAND_CHECK_FUNC_RESULT(res, 190, exit);

  res = arr_mem_info->entry_set_fun(
          arr_mem_info->instance_prim_handle,
          DNX_SAND_ARR_MEM_ALLOCATOR_ACTIVE_INST(arr_mem_info,arr_mem_info->instance_sec_handle),
          ptr - before_size,
          &(free_entries[0])
        );
  DNX_SAND_CHECK_FUNC_RESULT(res, 200, exit);

  res = arr_mem_info->entry_set_fun(
          arr_mem_info->instance_prim_handle,
          DNX_SAND_ARR_MEM_ALLOCATOR_ACTIVE_INST(arr_mem_info,arr_mem_info->instance_sec_handle),
          ptr + after_size + used_block_size - 1,
          &(free_entries[1])
        );
  DNX_SAND_CHECK_FUNC_RESULT(res, 210, exit);

  res = dnx_sand_arr_mem_allocator_upadte_list(
          arr_mem_info,
          prev,
          ptr - before_size,
          next
        );
  DNX_SAND_CHECK_FUNC_RESULT(res, 220, exit);

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in dnx_sand_arr_mem_allocator_free()",0,0);
}

uint32
  dnx_sand_arr_mem_allocator_block_size(
    DNX_SAND_INOUT   DNX_SAND_ARR_MEM_ALLOCATOR_INFO     *arr_mem_info,
    DNX_SAND_IN  DNX_SAND_ARR_MEM_ALLOCATOR_PTR          ptr,
    DNX_SAND_OUT  uint32                           *size
  )
{
  DNX_SAND_ARR_MEM_ALLOCATOR_ENTRY
    used_entry;
  uint32
    res;

  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(DNX_SAND_ARR_MEM_ALLOCATOR_FREE);

  DNX_SAND_CHECK_NULL_INPUT(arr_mem_info);

  if (ptr> DNX_SAND_ARR_MEM_ALLOCATOR_LAST_INDEX(arr_mem_info))
  {
    DNX_SAND_SET_ERROR_CODE(DNX_SAND_ARR_MEM_ALLOCATOR_POINTER_OF_RANGE_ERR, 10, exit);
  }

  res = arr_mem_info->entry_get_fun(
          arr_mem_info->instance_prim_handle,
          DNX_SAND_ARR_MEM_ALLOCATOR_ACTIVE_INST(arr_mem_info,arr_mem_info->instance_sec_handle),
          ptr,
          &used_entry
        );
  DNX_SAND_CHECK_FUNC_RESULT(res, 20, exit);

  *size = DNX_SAND_ARR_MEM_ALLOCATOR_ENTRY_SIZE(used_entry);

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in dnx_sand_arr_mem_allocator_array_set()",0,0);
}

/*********************************************************************
* NAME:
*     dnx_sand_arr_mem_allocator_write
* TYPE:
*   PROC
* DATE:
*   May  6 2008
* FUNCTION:
*  write row of data to the memory.
* INPUT:
*   DNX_SAND_INOUT   DNX_SAND_ARR_MEM_ALLOCATOR_INFO     *arr_mem_info -
*     The arr_mem_alloc instance
*   DNX_SAND_IN  DNX_SAND_ARR_MEM_ALLOCATOR_PTR     offset -
*     the entry to write to.
*   DNX_SAND_IN  DNX_SAND_ARR_MEM_ALLOCATOR_ENTRY        *data -
*     to hold the written data.
*     size has to be as row size.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  dnx_sand_arr_mem_allocator_write(
    DNX_SAND_INOUT   DNX_SAND_ARR_MEM_ALLOCATOR_INFO     *arr_mem_info,
    DNX_SAND_IN  DNX_SAND_ARR_MEM_ALLOCATOR_PTR          offset,
    DNX_SAND_IN  DNX_SAND_ARR_MEM_ALLOCATOR_ENTRY        *data
  )
{

  uint32
    res;

  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(DNX_SAND_ARR_MEM_ALLOCATOR_WRITE);

  DNX_SAND_CHECK_NULL_INPUT(arr_mem_info);

  if (offset > DNX_SAND_ARR_MEM_ALLOCATOR_LAST_INDEX(arr_mem_info))
  {
    DNX_SAND_SET_ERROR_CODE(DNX_SAND_ARR_MEM_ALLOCATOR_POINTER_OF_RANGE_ERR, 10, exit);
  }

  res = arr_mem_info->write_fun(
          arr_mem_info->instance_prim_handle,
          DNX_SAND_ARR_MEM_ALLOCATOR_ACTIVE_INST(arr_mem_info,arr_mem_info->instance_sec_handle),
          offset,
          data
        );
  DNX_SAND_CHECK_FUNC_RESULT(res, 20, exit);
 /*
  * if there was caching store this
  */
  if (arr_mem_info->arr_mem_allocator_data.cache_enabled)
  {
    arr_mem_info->arr_mem_allocator_data.update_indexes[arr_mem_info->arr_mem_allocator_data.nof_updates] = offset;
    ++arr_mem_info->arr_mem_allocator_data.nof_updates;
  }
  
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in dnx_sand_arr_mem_allocator_write()",0,0);
}



/*********************************************************************
* NAME:
*     dnx_sand_arr_mem_allocator_read
* TYPE:
*   PROC
* DATE:
*   May  6 2008
* FUNCTION:
*  read row of data from the memory.
* INPUT:
*   DNX_SAND_INOUT   DNX_SAND_ARR_MEM_ALLOCATOR_INFO     *arr_mem_info -
*     The arr_mem_alloc instance
*   DNX_SAND_IN  DNX_SAND_ARR_MEM_ALLOCATOR_PTR     offset -
*     the entry to read from.
*   DNX_SAND_OUT  DNX_SAND_ARR_MEM_ALLOCATOR_ENTRY        *data -
*     to hold the readen data.
*     size has to be as row size.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  dnx_sand_arr_mem_allocator_read(
    DNX_SAND_INOUT   DNX_SAND_ARR_MEM_ALLOCATOR_INFO     *arr_mem_info,
    DNX_SAND_IN  DNX_SAND_ARR_MEM_ALLOCATOR_PTR          offset,
    DNX_SAND_OUT  DNX_SAND_ARR_MEM_ALLOCATOR_ENTRY        *data
  )
{

  uint32
    res;

  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(DNX_SAND_ARR_MEM_ALLOCATOR_READ);

  DNX_SAND_CHECK_NULL_INPUT(arr_mem_info);

  if (offset> DNX_SAND_ARR_MEM_ALLOCATOR_LAST_INDEX(arr_mem_info))
  {
    DNX_SAND_SET_ERROR_CODE(DNX_SAND_ARR_MEM_ALLOCATOR_POINTER_OF_RANGE_ERR, 10, exit);
  }

  res = arr_mem_info->read_fun(
          arr_mem_info->instance_prim_handle,
          DNX_SAND_ARR_MEM_ALLOCATOR_ACTIVE_INST(arr_mem_info,arr_mem_info->instance_sec_handle),
          offset,
          data
        );
  DNX_SAND_CHECK_FUNC_RESULT(res, 20, exit);

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in dnx_sand_arr_mem_allocator_read()",0,0);
}

static uint32
  dnx_sand_arr_mem_allocator_get_next_used_block(
    DNX_SAND_IN  DNX_SAND_ARR_MEM_ALLOCATOR_INFO     *arr_mem_info,
    DNX_SAND_INOUT  DNX_SAND_ARR_MEM_ALLOCATOR_ITER  *iter,
    DNX_SAND_IN  uint32                        pos
  )
{
  DNX_SAND_ARR_MEM_ALLOCATOR_PTR
    indx;
  uint32
    block_size,
    cur_pos=pos;
  DNX_SAND_ARR_MEM_ALLOCATOR_ENTRY
    entry;
  uint32
    res;

  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);

  indx = iter->offset;
  block_size = 0;

  while (block_size == 0 && indx <= DNX_SAND_ARR_MEM_ALLOCATOR_LAST_INDEX(arr_mem_info))
  {
    res = arr_mem_info->entry_get_fun(
            arr_mem_info->instance_prim_handle,
            DNX_SAND_ARR_MEM_ALLOCATOR_ACTIVE_INST(arr_mem_info,arr_mem_info->instance_sec_handle),
            indx,
            &entry
          );
    DNX_SAND_CHECK_FUNC_RESULT(res, 10, exit);
    if (DNX_SAND_ARR_MEM_ALLOCATOR_ENTRY_IS_USED(entry))
    {
      if (!cur_pos)
      {
        block_size = DNX_SAND_ARR_MEM_ALLOCATOR_ENTRY_SIZE(entry);
      }
      else /*no used entry jump to next block.*/
      {
        --cur_pos;
        indx += DNX_SAND_ARR_MEM_ALLOCATOR_ENTRY_SIZE(entry);
      } 

    }
    else /*no used entry jump to next block.*/
    {
      indx += DNX_SAND_ARR_MEM_ALLOCATOR_ENTRY_SIZE(entry);
    }
  }

  iter->block_size = block_size;
  if (indx > DNX_SAND_ARR_MEM_ALLOCATOR_LAST_INDEX(arr_mem_info))
  {
    iter->offset = DNX_SAND_ARR_MEM_ALLOCATOR_NULL;
  }
  else
  {
    iter->offset = indx;
  }


exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in dnx_sand_arr_mem_allocator_get_next_free()",0,0);
}

static uint32
  dnx_sand_arr_mem_allocator_get_next_free_in_order(
    DNX_SAND_IN  DNX_SAND_ARR_MEM_ALLOCATOR_INFO     *arr_mem_info,
    DNX_SAND_INOUT  DNX_SAND_ARR_MEM_ALLOCATOR_ITER  *iter
  )
{
  DNX_SAND_ARR_MEM_ALLOCATOR_PTR
    indx;
  uint32
    block_size;
  DNX_SAND_ARR_MEM_ALLOCATOR_ENTRY
    entry;
  uint32
    res;

  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);

  indx = iter->offset;
  block_size = 1;

  while (indx <= DNX_SAND_ARR_MEM_ALLOCATOR_LAST_INDEX(arr_mem_info))
  {
    res = arr_mem_info->entry_get_fun(
            arr_mem_info->instance_prim_handle,
            DNX_SAND_ARR_MEM_ALLOCATOR_ACTIVE_INST(arr_mem_info,arr_mem_info->instance_sec_handle),
            indx,
            &entry
          );
    DNX_SAND_CHECK_FUNC_RESULT(res, 10, exit);
    if (DNX_SAND_ARR_MEM_ALLOCATOR_ENTRY_IS_USED(entry))/* used entry jump to next block.*/
    {
      indx += DNX_SAND_ARR_MEM_ALLOCATOR_ENTRY_SIZE(entry);
    }
    else 
    {
      block_size = DNX_SAND_ARR_MEM_ALLOCATOR_ENTRY_SIZE(entry);
      break;
    }
  }

  iter->block_size = block_size;
  if (indx > DNX_SAND_ARR_MEM_ALLOCATOR_LAST_INDEX(arr_mem_info))
  {
    iter->offset = DNX_SAND_ARR_MEM_ALLOCATOR_LIST_END;
  }
  else
  {
    iter->offset = indx;
  }


exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in dnx_sand_arr_mem_allocator_get_next_free()",0,0);
}




/*********************************************************************
* NAME:
*     dnx_sand_arr_mem_allocator_read_block
* TYPE:
*   PROC
* DATE:
*   May  6 2008
* FUNCTION:
*  read block of entries from the the memory.
* INPUT:
*   DNX_SAND_INOUT   DNX_SAND_ARR_MEM_ALLOCATOR_INFO     *arr_mem_info -
*     The arr_mem_alloc instance to read from.
*   DNX_SAND_INOUT  DNX_SAND_ARR_MEM_ALLOCATOR_ITER       *iter -
*     iterator to traverse the memory.
*     use dnx_sand_SAND_ARR_MEM_ALLOCATOR_ITER_clear(iter) to
*     clear the iterator and start from the beginning of the
*     memory.
*     use DNX_SAND_ARR_MEM_ALLOCATOR_ITER_END(iter) to
*     check if the iterator reached the end of the memory.
*   DNX_SAND_IN  uint32                             entries_to_read -
*     number of valid (used) entries to get.
*   DNX_SAND_OUT  DNX_SAND_ARR_MEM_ALLOCATOR_ENTRY        *data -
*     buffer to hold the readen data has to be of size 'entries_to_read'
*     at least.
*   DNX_SAND_OUT  DNX_SAND_ARR_MEM_ALLOCATOR_PTR          *addresses -
*     buffer to hold the addresses of the valid entries.
*     has to be of size 'entries_to_read'
*   DNX_SAND_OUT  uint32                            *nof_entries
*     number of valid entries in data and addresses
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  dnx_sand_arr_mem_allocator_read_block(
    DNX_SAND_INOUT   DNX_SAND_ARR_MEM_ALLOCATOR_INFO      *arr_mem_info,
    DNX_SAND_INOUT  DNX_SAND_ARR_MEM_ALLOCATOR_ITER       *iter,
    DNX_SAND_IN  uint32                             entries_to_read,
    DNX_SAND_OUT  DNX_SAND_ARR_MEM_ALLOCATOR_ENTRY        *data,
    DNX_SAND_OUT  DNX_SAND_ARR_MEM_ALLOCATOR_PTR          *addresses,
    DNX_SAND_OUT  uint32                            *nof_entries
  )
{
  uint32
    readen;
  uint32
    res;

  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(DNX_SAND_ARR_MEM_ALLOCATOR_READ_BLOCK);

  DNX_SAND_CHECK_NULL_INPUT(arr_mem_info);
  DNX_SAND_CHECK_NULL_INPUT(iter);
  DNX_SAND_CHECK_NULL_INPUT(data);
  DNX_SAND_CHECK_NULL_INPUT(addresses);
  DNX_SAND_CHECK_NULL_INPUT(nof_entries);

  readen = 0;

  if (iter->offset > DNX_SAND_ARR_MEM_ALLOCATOR_LAST_INDEX(arr_mem_info))
  {
    DNX_SAND_SET_ERROR_CODE(DNX_SAND_ARR_MEM_ALLOCATOR_POINTER_OF_RANGE_ERR, 10, exit);
  }

  for (; iter->offset <= DNX_SAND_ARR_MEM_ALLOCATOR_LAST_INDEX(arr_mem_info) && readen < entries_to_read; ++iter->offset )
  {
    if (iter->block_size == 0)
    {
      res = dnx_sand_arr_mem_allocator_get_next_used_block(
              arr_mem_info,
              iter,
              0
            );
      DNX_SAND_CHECK_FUNC_RESULT(res, 20, exit)
    }
    if(DNX_SAND_ARR_MEM_ALLOCATOR_ITER_END(iter))
    {
      break;
    }
    res = arr_mem_info->read_fun(
            arr_mem_info->instance_prim_handle,
            DNX_SAND_ARR_MEM_ALLOCATOR_ACTIVE_INST(arr_mem_info,arr_mem_info->instance_sec_handle),
            iter->offset,
            &(data[readen])
          );
    DNX_SAND_CHECK_FUNC_RESULT(res, 30, exit);

    addresses[readen] = iter->offset;
    ++readen;
    --(iter->block_size);
  }
  *nof_entries = readen;

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in dnx_sand_arr_mem_allocator_read_block()",0,0);
}


/*********************************************************************
* NAME:
*     dnx_sand_arr_mem_allocator_defrag
* TYPE:
*   PROC
* DATE:
*   May  6 2008
* FUNCTION:
*  perform defragmentation for the memory.
* INPUT:
*   DNX_SAND_INOUT   DNX_SAND_ARR_MEM_ALLOCATOR_INFO     *arr_mem_info -
*     The arr_mem_alloc instance to read from.
*   DNX_SAND_INOUT  DNX_SAND_ARR_MEM_ALLOCATOR_DEFRAG_INFO       *defrag_info -
*     defragmentation info
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  dnx_sand_arr_mem_allocator_defrag(
    DNX_SAND_INOUT   DNX_SAND_ARR_MEM_ALLOCATOR_INFO       *arr_mem_info,
    DNX_SAND_INOUT DNX_SAND_ARR_MEM_ALLOCATOR_DEFRAG_INFO  *defrag_info
  )
{
  uint32
    nof_defrags=0,
    nof_moves = 0,
    moved_to_end = 0,
    max_block_size=0;
  DNX_SAND_ARR_MEM_ALLOCATOR_ITER  
    cur_used,
    first_free_iter;
  uint8
    done = FALSE;
  DNX_SAND_ARR_MEM_ALLOCATOR_PTR
    free_ptr,
    moved_to_ptr,
    move_from_ptr,
    reserved_mem_ptr;
  DNX_SAND_ARR_MEM_ALLOCATOR_ENTRY
    entry;
  uint32
    indx,
    prev_used;
  DNX_SAND_TABLE_BLOCK_RANGE             
    *block_range;
  uint32
    entry_size,
    pos=0,
    flags;
  uint32
    res;

  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);

  DNX_SAND_CHECK_NULL_INPUT(arr_mem_info);
  DNX_SAND_CHECK_NULL_INPUT(defrag_info);


  max_block_size = arr_mem_info->max_block_size;
  block_range = &(defrag_info->block_range);

  if (!arr_mem_info->support_defragment)
  {
    DNX_SAND_SET_ERROR_CODE(DNX_SAND_ERR, 10, exit);
  }

  /* check iter status */
  if (DNX_SAND_TBL_ITER_IS_END(&block_range->iter))
  {
    goto exit;
  }

  first_free_iter.offset = 0;
  res = dnx_sand_arr_mem_allocator_get_next_free_in_order(arr_mem_info,&first_free_iter);
  DNX_SAND_CHECK_FUNC_RESULT(res, 30, exit);
  free_ptr = first_free_iter.offset;

  if (free_ptr == DNX_SAND_ARR_MEM_ALLOCATOR_LIST_END)
  {
    /* no free space */
    DNX_SAND_TBL_ITER_SET_END(&block_range->iter);
    goto exit;
  }
  
  /* get free block-size */
  entry_size = first_free_iter.block_size;

  /* start of area where to move blocks */
  reserved_mem_ptr = arr_mem_info->nof_entries - max_block_size;;
  moved_to_ptr = reserved_mem_ptr;

  flags = DNX_SAND_ARR_MEM_ALLOCATOR_MOVE_UPDATE_CONT|DNX_SAND_ARR_MEM_ALLOCATOR_MOVE_UPDATE_REF|
           DNX_SAND_ARR_MEM_ALLOCATOR_MOVE_FREE|DNX_SAND_ARR_MEM_ALLOCATOR_MOVE_BLK_SIZE_VLD;

  cur_used.offset = free_ptr + entry_size;
  cur_used.block_size = 0;
  pos = 0;
  /* if not enough space move block to end of memory */
  while(entry_size < max_block_size/2) 
  {
    /* get next used and move it */
    res = dnx_sand_arr_mem_allocator_get_next_used_block(arr_mem_info,&cur_used,pos);
    DNX_SAND_CHECK_FUNC_RESULT(res, 30, exit);
    if (cur_used.offset - free_ptr >= max_block_size/2 || cur_used.offset == DNX_SAND_ARR_MEM_ALLOCATOR_NULL)
    {
      break;
    }
   /*
    * move block to end memory, without the management of free list
    */
   /*
    * update references to this area, and free block
    */
    res = dnx_sand_arr_mem_allocator_move_block(
            arr_mem_info,
            cur_used.offset,/* from*/ 
            moved_to_ptr, /* to */
            cur_used.block_size,
            flags
         );
    DNX_SAND_CHECK_FUNC_RESULT(res, 60, exit);
    /*
     * now store data in SW, how many entries in block
     */
      entry = cur_used.block_size;
      res = arr_mem_info->entry_set_fun(
            arr_mem_info->instance_prim_handle,
            DNX_SAND_ARR_MEM_ALLOCATOR_ACTIVE_INST(arr_mem_info,arr_mem_info->instance_sec_handle),
            reserved_mem_ptr + moved_to_end,
            &entry
          );
      DNX_SAND_CHECK_FUNC_RESULT(res, 35, exit);

    ++moved_to_end;
    moved_to_ptr += cur_used.block_size;
    /* addvance curr used block pointer */
    cur_used.offset += cur_used.block_size;
    if (cur_used.offset - free_ptr >= max_block_size/2 || cur_used.offset == DNX_SAND_ARR_MEM_ALLOCATOR_NULL)
    {
      break;
    }
  }

  flags = DNX_SAND_ARR_MEM_ALLOCATOR_MOVE_FREE|DNX_SAND_ARR_MEM_ALLOCATOR_MOVE_COPY|
          DNX_SAND_ARR_MEM_ALLOCATOR_MOVE_UPDATE_CONT|DNX_SAND_ARR_MEM_ALLOCATOR_MOVE_UPDATE_REF;
  /* defragment loop */
  prev_used = cur_used.offset;
  while (!done && nof_defrags < block_range->entries_to_act && nof_moves < block_range->entries_to_scan)
  {
    /* get next used and move it */
    res = dnx_sand_arr_mem_allocator_get_next_used_block(arr_mem_info,&cur_used,0);
    DNX_SAND_CHECK_FUNC_RESULT(res, 10, exit);
    if (cur_used.offset == DNX_SAND_ARR_MEM_ALLOCATOR_NULL)
    {
      done = TRUE;
      break;
    }

    res = dnx_sand_arr_mem_allocator_move_block(arr_mem_info,cur_used.offset,free_ptr,0,flags);
    DNX_SAND_CHECK_FUNC_RESULT(res, 10, exit);
    nof_moves+=cur_used.block_size;
    if (prev_used != cur_used.offset)
    {
      ++nof_defrags;
    }
    
    /* update used pointer*/
    cur_used.offset += cur_used.block_size;
    if (cur_used.offset  > DNX_SAND_ARR_MEM_ALLOCATOR_LAST_INDEX(arr_mem_info))
    {
      done = TRUE;
      break;
    }
    prev_used = cur_used.offset;

    /* update free */
    first_free_iter.offset += cur_used.block_size;
    res = dnx_sand_arr_mem_allocator_get_next_free_in_order(arr_mem_info,&first_free_iter);
    DNX_SAND_CHECK_FUNC_RESULT(res, 30, exit);
    free_ptr = first_free_iter.offset;
  }
  
  /* now move back, the blocks set at end of memory */
  flags = DNX_SAND_ARR_MEM_ALLOCATOR_MOVE_COPY|DNX_SAND_ARR_MEM_ALLOCATOR_MOVE_UPDATE_CONT|
    DNX_SAND_ARR_MEM_ALLOCATOR_MOVE_UPDATE_REF|DNX_SAND_ARR_MEM_ALLOCATOR_MOVE_BLK_SIZE_VLD;

  move_from_ptr = reserved_mem_ptr;
  moved_to_ptr = free_ptr;
  for (indx = 0; indx < moved_to_end; ++indx)
  {
    /* get size */
    res = arr_mem_info->entry_get_fun(
            arr_mem_info->instance_prim_handle,
            DNX_SAND_ARR_MEM_ALLOCATOR_ACTIVE_INST(arr_mem_info,arr_mem_info->instance_sec_handle),
            reserved_mem_ptr + indx,
            &entry
          );
    DNX_SAND_CHECK_FUNC_RESULT(res, 35, exit);

    res = dnx_sand_arr_mem_allocator_move_block(arr_mem_info,move_from_ptr,moved_to_ptr,entry,flags);
    DNX_SAND_CHECK_FUNC_RESULT(res, 10, exit);

    move_from_ptr += entry;
    moved_to_ptr += entry;
  }
  
  if (done)
  {
    DNX_SAND_TBL_ITER_SET_END(&defrag_info->block_range.iter);
  }
  else
  {
    defrag_info->block_range.iter = moved_to_ptr;
  }
  


exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in dnx_sand_arr_mem_allocator_read_block()",0,0);
}


uint32
  dnx_sand_arr_mem_allocator_print_free(
    DNX_SAND_INOUT   DNX_SAND_ARR_MEM_ALLOCATOR_INFO     *arr_mem_info
  )
{
  DNX_SAND_ARR_MEM_ALLOCATOR_PTR
    free_ptr;
  DNX_SAND_ARR_MEM_ALLOCATOR_ENTRY
    entry;
  uint32
    entry_size;
  uint32
    res;

  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);

  DNX_SAND_CHECK_NULL_INPUT(arr_mem_info);

  res = arr_mem_info->free_get_fun(
          arr_mem_info->instance_prim_handle,
          DNX_SAND_ARR_MEM_ALLOCATOR_ACTIVE_INST(arr_mem_info,arr_mem_info->instance_sec_handle),
          &free_ptr
        );
  DNX_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  LOG_CLI((BSL_META_U(unit,
                      "free blocks:\n")));

  while (free_ptr != DNX_SAND_ARR_MEM_ALLOCATOR_LIST_END)
  {
    res = arr_mem_info->entry_get_fun(
            arr_mem_info->instance_prim_handle,
            DNX_SAND_ARR_MEM_ALLOCATOR_ACTIVE_INST(arr_mem_info,arr_mem_info->instance_sec_handle),
            free_ptr,
            &entry
          );
    DNX_SAND_CHECK_FUNC_RESULT(res, 10, exit);

    entry_size = DNX_SAND_ARR_MEM_ALLOCATOR_ENTRY_SIZE(entry);
    LOG_CLI((BSL_META_U(unit,
                        "start:%u   end:%u \n"), free_ptr, free_ptr + entry_size));

    res = dnx_sand_arr_mem_allocator_get_next_free(
            arr_mem_info,
            TRUE,
            &free_ptr
          );
    DNX_SAND_CHECK_FUNC_RESULT(res, 20, exit);
  }

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in dnx_sand_arr_mem_allocator_print_free()",0,0);
}



uint32
  dnx_sand_arr_mem_allocator_print_free_by_order(
    DNX_SAND_INOUT   DNX_SAND_ARR_MEM_ALLOCATOR_INFO     *arr_mem_info
  )
{
  DNX_SAND_ARR_MEM_ALLOCATOR_ITER  iter;
  uint32
    entry_size;
  uint32
    res;

  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);

  DNX_SAND_CHECK_NULL_INPUT(arr_mem_info);

  iter.offset = 0;

  res = dnx_sand_arr_mem_allocator_get_next_free_in_order(
          arr_mem_info,
          &iter
        );
  DNX_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  LOG_CLI((BSL_META_U(unit,
                      "free blocks by order:\n")));

  while (iter.offset != DNX_SAND_ARR_MEM_ALLOCATOR_LIST_END)
  {
    entry_size = iter.block_size;
    LOG_CLI((BSL_META_U(unit,
                        "start:%u   end:%u \n"), iter.offset, iter.offset + entry_size));

    iter.offset += entry_size;
    res = dnx_sand_arr_mem_allocator_get_next_free_in_order(
            arr_mem_info,
            &iter
          );
    DNX_SAND_CHECK_FUNC_RESULT(res, 20, exit);
  }

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in dnx_sand_arr_mem_allocator_print_free_by_order()",0,0);
}


uint32
  dnx_sand_arr_mem_allocator_is_availabe_blocks(
    DNX_SAND_INOUT   DNX_SAND_ARR_MEM_ALLOCATOR_INFO     *arr_mem_info,
	  DNX_SAND_IN   DNX_SAND_ARR_MEM_ALLOCATOR_REQ_BLOCKS  *req_blocks,
	  DNX_SAND_OUT   uint8                         *available
  )
{
  DNX_SAND_ARR_MEM_ALLOCATOR_PTR
    free_ptr;
  DNX_SAND_ARR_MEM_ALLOCATOR_ENTRY
    entry;
  uint32
    block_indx = 0;
  uint32
    entry_size;
  uint32
    res;

  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);

  DNX_SAND_CHECK_NULL_INPUT(arr_mem_info);

  res = arr_mem_info->free_get_fun(
          arr_mem_info->instance_prim_handle,
          DNX_SAND_ARR_MEM_ALLOCATOR_ACTIVE_INST(arr_mem_info,arr_mem_info->instance_sec_handle),
          &free_ptr
        );
  DNX_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  while (free_ptr != DNX_SAND_ARR_MEM_ALLOCATOR_LIST_END)
  {
    res = arr_mem_info->entry_get_fun(
            arr_mem_info->instance_prim_handle,
            DNX_SAND_ARR_MEM_ALLOCATOR_ACTIVE_INST(arr_mem_info,arr_mem_info->instance_sec_handle),
            free_ptr,
            &entry
          );
    DNX_SAND_CHECK_FUNC_RESULT(res, 10, exit);

    entry_size = DNX_SAND_ARR_MEM_ALLOCATOR_ENTRY_SIZE(entry);

	while(entry_size >= req_blocks->block_size[block_indx])
	{
		entry_size -= req_blocks->block_size[block_indx];
		block_indx++;
		if (block_indx >= req_blocks->nof_reqs)
		{
			*available = TRUE;
			goto exit;
		}
	}
    res = dnx_sand_arr_mem_allocator_get_next_free(
            arr_mem_info,
            TRUE,
            &free_ptr
          );
    DNX_SAND_CHECK_FUNC_RESULT(res, 20, exit);
  }

  *available = FALSE; 

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in dnx_sand_arr_mem_allocator_is_availabe_blocks()",0,0);
}


uint32
  dnx_sand_arr_mem_allocator_mem_status_get(
    DNX_SAND_INOUT   DNX_SAND_ARR_MEM_ALLOCATOR_INFO     *arr_mem_info,
	  DNX_SAND_OUT   DNX_SAND_ARR_MEM_ALLOCATOR_MEM_STATUS  *mem_status
  )
{
  DNX_SAND_ARR_MEM_ALLOCATOR_PTR
    free_ptr;
  DNX_SAND_ARR_MEM_ALLOCATOR_ENTRY
    entry;
  uint32
	  total_free = 0,
	  max_free_size = 0;
  uint32
    entry_size;
  uint32
    res;

  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);

  DNX_SAND_CHECK_NULL_INPUT(arr_mem_info);

  dnx_sand_SAND_ARR_MEM_ALLOCATOR_MEM_STATUS_clear(mem_status);

  res = arr_mem_info->free_get_fun(
          arr_mem_info->instance_prim_handle,
          DNX_SAND_ARR_MEM_ALLOCATOR_ACTIVE_INST(arr_mem_info,arr_mem_info->instance_sec_handle),
          &free_ptr
        );
  DNX_SAND_CHECK_FUNC_RESULT(res, 10, exit);


  while (free_ptr != DNX_SAND_ARR_MEM_ALLOCATOR_LIST_END)
  {
    res = arr_mem_info->entry_get_fun(
            arr_mem_info->instance_prim_handle,
            DNX_SAND_ARR_MEM_ALLOCATOR_ACTIVE_INST(arr_mem_info,arr_mem_info->instance_sec_handle),
            free_ptr,
            &entry
          );
    DNX_SAND_CHECK_FUNC_RESULT(res, 10, exit);

    entry_size = DNX_SAND_ARR_MEM_ALLOCATOR_ENTRY_SIZE(entry);
    total_free += entry_size;
	  if(entry_size > max_free_size)
	  {
		  max_free_size = entry_size;
	  }
    ++mem_status->nof_fragments;

    res = dnx_sand_arr_mem_allocator_get_next_free(
            arr_mem_info,
            TRUE,
            &free_ptr
          );
    DNX_SAND_CHECK_FUNC_RESULT(res, 20, exit);
  }

  mem_status->max_free_block_size = max_free_size;
  mem_status->total_free = total_free;
  mem_status->total_size = DNX_SAND_ARR_MEM_ALLOCATOR_LAST_INDEX(arr_mem_info) + 1;

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in dnx_sand_arr_mem_allocator_mem_status_get()",0,0);
}


uint32
  dnx_sand_arr_mem_allocator_get_size_for_save(
    DNX_SAND_IN   DNX_SAND_ARR_MEM_ALLOCATOR_INFO      *arr_mem_info,
    DNX_SAND_IN  uint32                        flags,
    DNX_SAND_OUT  uint32                       *size
  )
{
  const DNX_SAND_ARR_MEM_ALLOCATOR_T
    *mem_data;
  uint32
    cur_size,
    total_size=0;


  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);

  DNX_SAND_CHECK_NULL_INPUT(arr_mem_info);
  DNX_SAND_CHECK_NULL_INPUT(size);


  /* size of init info */
  mem_data = &(arr_mem_info->arr_mem_allocator_data);

  cur_size = sizeof(DNX_SAND_ARR_MEM_ALLOCATOR_INFO);
  total_size += cur_size;

  /* array */
  cur_size = sizeof(DNX_SAND_ARR_MEM_ALLOCATOR_ENTRY) * arr_mem_info->nof_entries;
  total_size += cur_size;
  /* mem-shadow */
  cur_size = sizeof(uint32) * arr_mem_info->nof_entries;
  total_size += cur_size;

  
  if (mem_data->cache_enabled)
  {
    /* array_cache */
    cur_size = sizeof(DNX_SAND_ARR_MEM_ALLOCATOR_ENTRY) * arr_mem_info->nof_entries;
    total_size += cur_size;
    /* mem_shadow_cache */
    cur_size = sizeof(uint32) * arr_mem_info->nof_entries;
    total_size += cur_size;
    /* update_indexes */
    cur_size = sizeof(DNX_SAND_ARR_MEM_ALLOCATOR_ENTRY) * arr_mem_info->nof_entries;
    total_size += cur_size;
  }

  *size= total_size;

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in dnx_sand_arr_mem_allocator_get_size_for_save()",0,0);
}



uint32
  dnx_sand_arr_mem_allocator_save(
    DNX_SAND_IN   DNX_SAND_ARR_MEM_ALLOCATOR_INFO      *arr_mem_info,
    DNX_SAND_IN  uint32                flags,
    DNX_SAND_OUT uint8                 *buffer,
    DNX_SAND_IN  uint32                buffer_size_bytes,
    DNX_SAND_OUT uint32                *actual_size_bytes
  )
{
  uint8
    *cur_ptr = (uint8*)buffer;
  uint32
    cur_size,
    total_size=0;
  uint32
    res;
  DNX_SAND_ARR_MEM_ALLOCATOR_INFO
    *info;

  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);

  DNX_SAND_CHECK_NULL_INPUT(arr_mem_info);
  DNX_SAND_CHECK_NULL_INPUT(buffer);
  DNX_SAND_CHECK_NULL_INPUT(actual_size_bytes);

  /* copy init info */
  DNX_SAND_COPY_TO_BUFF_AND_INC(cur_ptr,arr_mem_info,DNX_SAND_ARR_MEM_ALLOCATOR_INFO,1);

  /*patch zero in places that contain pointers*/
  info = (DNX_SAND_ARR_MEM_ALLOCATOR_INFO *) (cur_ptr - sizeof(DNX_SAND_ARR_MEM_ALLOCATOR_INFO));
  dnx_sand_os_memset(&info->arr_mem_allocator_data.array, 0x0, sizeof(DNX_SAND_ARR_MEM_ALLOCATOR_ENTRY *));
  dnx_sand_os_memset(&info->arr_mem_allocator_data.mem_shadow, 0x0, sizeof(uint32 *));
  dnx_sand_os_memset(&info->arr_mem_allocator_data.array_cache, 0x0, sizeof(DNX_SAND_ARR_MEM_ALLOCATOR_ENTRY *));
  dnx_sand_os_memset(&info->arr_mem_allocator_data.mem_shadow_cache, 0x0, sizeof(uint32 *));
  dnx_sand_os_memset(&info->arr_mem_allocator_data.update_indexes, 0x0, sizeof(DNX_SAND_ARR_MEM_ALLOCATOR_ENTRY *));  
  dnx_sand_os_memset(&info->entry_set_fun, 0x0, sizeof(DNX_SAND_ARR_MEM_ALLOCATOR_ENTRY_SET));
  dnx_sand_os_memset(&info->entry_get_fun, 0x0, sizeof(DNX_SAND_ARR_MEM_ALLOCATOR_ENTRY_GET));
  dnx_sand_os_memset(&info->free_set_fun, 0x0, sizeof(DNX_SAND_ARR_MEM_ALLOCATOR_FREE_LIST_SET));
  dnx_sand_os_memset(&info->free_get_fun, 0x0, sizeof(DNX_SAND_ARR_MEM_ALLOCATOR_FREE_LIST_GET));
  dnx_sand_os_memset(&info->read_fun, 0x0, sizeof(DNX_SAND_ARR_MEM_ALLOCATOR_ROW_READ));
  dnx_sand_os_memset(&info->write_fun, 0x0, sizeof(DNX_SAND_ARR_MEM_ALLOCATOR_ROW_WRITE));
  dnx_sand_os_memset(&info->entry_move_fun, 0x0, sizeof(DNX_SAND_ARR_MEM_ALLOCATOR_ENTRY_MOVE_CALL_BACK));

  /* array */
  DNX_SAND_COPY_TO_BUFF_AND_INC(cur_ptr,arr_mem_info->arr_mem_allocator_data.array,DNX_SAND_ARR_MEM_ALLOCATOR_ENTRY,arr_mem_info->nof_entries);

  /* mem-shadow */
  DNX_SAND_COPY_TO_BUFF_AND_INC(cur_ptr,arr_mem_info->arr_mem_allocator_data.mem_shadow,uint32,arr_mem_info->nof_entries);

  if (arr_mem_info->arr_mem_allocator_data.cache_enabled)
  {
    /* array_cache */
    DNX_SAND_COPY_TO_BUFF_AND_INC(cur_ptr,arr_mem_info->arr_mem_allocator_data.array_cache,DNX_SAND_ARR_MEM_ALLOCATOR_ENTRY,arr_mem_info->nof_entries);
    /* mem_shadow_cache */
    DNX_SAND_COPY_TO_BUFF_AND_INC(cur_ptr,arr_mem_info->arr_mem_allocator_data.mem_shadow_cache,uint32,arr_mem_info->nof_entries);
    /* update_indexes */
    DNX_SAND_COPY_TO_BUFF_AND_INC(cur_ptr,arr_mem_info->arr_mem_allocator_data.update_indexes,DNX_SAND_ARR_MEM_ALLOCATOR_ENTRY,arr_mem_info->nof_entries);
  }

  *actual_size_bytes = total_size;

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in dnx_sand_arr_mem_allocator_save()",0,0);
}

uint32
  dnx_sand_arr_mem_allocator_load(
    DNX_SAND_IN  uint8                           **buffer,
    DNX_SAND_IN  DNX_SAND_ARR_MEM_ALLOCATOR_LOAD_INFO  *load_info,
    DNX_SAND_OUT DNX_SAND_ARR_MEM_ALLOCATOR_INFO      *arr_mem_info
  )
{
  DNX_SAND_IN uint8
    *cur_ptr = (DNX_SAND_IN uint8*)buffer[0];
  uint32
    res;
  DNX_SAND_ARR_MEM_ALLOCATOR_PTR
    free_list_holder;

  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);

  DNX_SAND_CHECK_NULL_INPUT(arr_mem_info);
  DNX_SAND_CHECK_NULL_INPUT(load_info);
  DNX_SAND_CHECK_NULL_INPUT(buffer);

  /* copy init info */
  dnx_sand_os_memcpy(arr_mem_info, cur_ptr, sizeof(DNX_SAND_ARR_MEM_ALLOCATOR_INFO));
  cur_ptr += sizeof(DNX_SAND_ARR_MEM_ALLOCATOR_INFO);

  /* save free list for later - it is being overridden by create func */
  free_list_holder = arr_mem_info->arr_mem_allocator_data.free_list;
  
  /* fill callbacks */
  arr_mem_info->entry_set_fun = load_info->entry_set_fun; 
  arr_mem_info->entry_get_fun = load_info->entry_get_fun; 
  arr_mem_info->free_set_fun = load_info->free_set_fun;
  arr_mem_info->free_get_fun = load_info->free_get_fun;
  arr_mem_info->read_fun = load_info->read_fun;
  arr_mem_info->write_fun = load_info->write_fun; 
  arr_mem_info->entry_move_fun = load_info->entry_move_fun; 

  /* create DS */
  res = dnx_sand_arr_mem_allocator_create(arr_mem_info);
  DNX_SAND_CHECK_FUNC_RESULT(res,20, exit);

  /* restore free list - it is being overridden by create func */
  arr_mem_info->arr_mem_allocator_data.free_list = free_list_holder;

  /* array */
  dnx_sand_os_memcpy(arr_mem_info->arr_mem_allocator_data.array, cur_ptr, arr_mem_info->nof_entries * sizeof(DNX_SAND_ARR_MEM_ALLOCATOR_ENTRY));
  cur_ptr += arr_mem_info->nof_entries * sizeof(DNX_SAND_ARR_MEM_ALLOCATOR_ENTRY);

  /* mem-shadow */
  dnx_sand_os_memcpy(arr_mem_info->arr_mem_allocator_data.mem_shadow, cur_ptr, arr_mem_info->nof_entries * sizeof(uint32));
  cur_ptr += arr_mem_info->nof_entries * sizeof(uint32);

  if (arr_mem_info->arr_mem_allocator_data.cache_enabled)
  {
    /* array_cache */
    dnx_sand_os_memcpy(arr_mem_info->arr_mem_allocator_data.array_cache, cur_ptr, arr_mem_info->nof_entries * sizeof(DNX_SAND_ARR_MEM_ALLOCATOR_ENTRY));
    cur_ptr += arr_mem_info->nof_entries * sizeof(DNX_SAND_ARR_MEM_ALLOCATOR_ENTRY);
    /* mem_shadow_cache */
    dnx_sand_os_memcpy(arr_mem_info->arr_mem_allocator_data.mem_shadow_cache, cur_ptr, arr_mem_info->nof_entries * sizeof(uint32));
    cur_ptr += arr_mem_info->nof_entries * sizeof(uint32);
    /* update_indexes */
    dnx_sand_os_memcpy(arr_mem_info->arr_mem_allocator_data.update_indexes, cur_ptr, arr_mem_info->nof_entries * sizeof(DNX_SAND_ARR_MEM_ALLOCATOR_ENTRY));
    cur_ptr += arr_mem_info->nof_entries * sizeof(uint32);
  }
  *buffer = cur_ptr;

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in dnx_sand_arr_mem_allocator_load()",0,0);
}



void
  dnx_sand_SAND_ARR_MEM_ALLOCATOR_INFO_clear(
    DNX_SAND_ARR_MEM_ALLOCATOR_INFO *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  dnx_sand_os_memset(info, 0x0, sizeof(DNX_SAND_ARR_MEM_ALLOCATOR_INFO));
  info->nof_entries = 0;
  info->instance_prim_handle = 0;
  info->instance_sec_handle = 0;
  info->entry_get_fun = 0;
  info->entry_set_fun = 0;
  info->support_caching = 0;
  info->support_defragment = 0;
  DNX_SAND_MAGIC_NUM_SET;
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}


void
  dnx_sand_SAND_ARR_MEM_ALLOCATOR_ITER_clear(
    DNX_SAND_ARR_MEM_ALLOCATOR_ITER *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  dnx_sand_os_memset(info, 0x0, sizeof(DNX_SAND_ARR_MEM_ALLOCATOR_ITER));
  info->block_size = 0;
  info->offset = 0;
  DNX_SAND_MAGIC_NUM_SET;
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  dnx_sand_SAND_ARR_MEM_ALLOCATOR_REQ_BLOCKS_clear(
    DNX_SAND_ARR_MEM_ALLOCATOR_REQ_BLOCKS *info
  )
{
  uint32
    ind;
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  dnx_sand_os_memset(info, 0x0, sizeof(DNX_SAND_ARR_MEM_ALLOCATOR_REQ_BLOCKS));

  for (ind = 0; ind < DNX_SAND_ARR_MEM_ALLOCATOR_MAX_NOF_REQS; ++ind)
  {
     info->block_size[ind] = 0;
  }
  info->nof_reqs = 0;
    
  DNX_SAND_MAGIC_NUM_SET;
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  dnx_sand_SAND_ARR_MEM_ALLOCATOR_MEM_STATUS_clear(
    DNX_SAND_ARR_MEM_ALLOCATOR_MEM_STATUS *info
  )
{

  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  dnx_sand_os_memset(info, 0x0, sizeof(DNX_SAND_ARR_MEM_ALLOCATOR_MEM_STATUS));

  info->total_free = 0;
  info->max_free_block_size = 0;
  info->nof_fragments = 0;
  info->total_size = 0;
    
  DNX_SAND_MAGIC_NUM_SET;
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  dnx_sand_SAND_ARR_MEM_ALLOCATOR_DEFRAG_INFO_clear(
    DNX_SAND_ARR_MEM_ALLOCATOR_DEFRAG_INFO *info
  )
{

  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  dnx_sand_os_memset(info, 0x0, sizeof(DNX_SAND_ARR_MEM_ALLOCATOR_DEFRAG_INFO));

  info->block_range.iter = 0;
  info->block_range.entries_to_scan = DNX_SAND_TBL_ITER_SCAN_ALL;
  info->block_range.entries_to_act = DNX_SAND_TBL_ITER_SCAN_ALL;
    
  DNX_SAND_MAGIC_NUM_SET;
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  dnx_sand_SAND_ARR_MEM_ALLOCATOR_LOAD_INFO_clear(
    DNX_SAND_ARR_MEM_ALLOCATOR_LOAD_INFO *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  dnx_sand_os_memset(info, 0x0, sizeof(DNX_SAND_ARR_MEM_ALLOCATOR_LOAD_INFO));
   
  DNX_SAND_MAGIC_NUM_SET;
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}


#if DNX_SAND_DEBUG
void
  dnx_sand_SAND_ARR_MEM_ALLOCATOR_INFO_print(
    DNX_SAND_ARR_MEM_ALLOCATOR_INFO *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  LOG_CLI((BSL_META_U(unit,
                      "nof_entries: %u\n\r"),info->nof_entries));
  LOG_CLI((BSL_META_U(unit,
                      "instance_prim_handle: %u\n\r"),info->instance_prim_handle));
  LOG_CLI((BSL_META_U(unit,
                      "instance_sec_handle: %u\n\r"),info->instance_sec_handle));
  LOG_CLI((BSL_META_U(unit,
                      "support_caching: %u\n\r"),info->support_caching));
  LOG_CLI((BSL_META_U(unit,
                      "support_defragment: %u\n\r"),info->support_defragment));
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}


void
  dnx_sand_SAND_ARR_MEM_ALLOCATOR_ITER_print(
    DNX_SAND_ARR_MEM_ALLOCATOR_ITER *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  LOG_CLI((BSL_META_U(unit,
                      "block_size: %u\n\r"),info->block_size));
  LOG_CLI((BSL_META_U(unit,
                      "offset: %u\n\r"),info->offset));
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  dnx_sand_SAND_ARR_MEM_ALLOCATOR_REQ_BLOCKS_print(
    DNX_SAND_IN DNX_SAND_ARR_MEM_ALLOCATOR_REQ_BLOCKS *info
  )
{
  uint32
    ind;
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);


  for (ind = 0; ind < DNX_SAND_ARR_MEM_ALLOCATOR_MAX_NOF_REQS; ++ind)
  {
     LOG_CLI((BSL_META_U(unit,
                         "block_size[ind]: %u\n\r"),info->block_size[ind]));
  }
  LOG_CLI((BSL_META_U(unit,
                      "nof_reqs: %u\n\r"),info->nof_reqs));
    
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  dnx_sand_SAND_ARR_MEM_ALLOCATOR_MEM_STATUS_print(
    DNX_SAND_IN DNX_SAND_ARR_MEM_ALLOCATOR_MEM_STATUS *info
  )
{

  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);


  LOG_CLI((BSL_META_U(unit,
                      "total_free: %u\n\r"),info->total_free));
  LOG_CLI((BSL_META_U(unit,
                      "max_free_block_size: %u\n\r"),info->max_free_block_size));
  LOG_CLI((BSL_META_U(unit,
                      "nof_fragments: %u\n\r"),info->nof_fragments));
  LOG_CLI((BSL_META_U(unit,
                      "total_size: %u\n\r"),info->total_size));
    
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  dnx_sand_SAND_ARR_MEM_ALLOCATOR_DEFRAG_INFO_print(
    DNX_SAND_IN DNX_SAND_ARR_MEM_ALLOCATOR_DEFRAG_INFO *info
  )
{

  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);


  LOG_CLI((BSL_META_U(unit,
                      "block_range.iter: %u\n\r"),info->block_range.iter));
  LOG_CLI((BSL_META_U(unit,
                      "block_range.entries_to_scan: %u\n\r"),info->block_range.entries_to_scan));
  LOG_CLI((BSL_META_U(unit,
                      "block_range.entries_to_act: %u\n\r"),info->block_range.entries_to_act));
    
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}
#endif /* DNX_SAND_DEBUG */

#include <soc/dnx/legacy/SAND/Utils/sand_footer.h>
