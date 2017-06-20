/* $Id: sand_array_memory_allocator.h,v 1.3 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/

#ifndef __DNX_SAND_ARRAY_MEMORY_ALLOCATOR_INCLUDED__
/* { */
#define __DNX_SAND_ARRAY_MEMORY_ALLOCATOR_INCLUDED__

/*************
* INCLUDES  *
*************/
/* { */
#include <soc/dnx/legacy/SAND/Utils/sand_header.h>

#include <soc/dnx/legacy/SAND/Management/sand_general_macros.h>
#include <soc/dnx/legacy/SAND/Utils/sand_framework.h>
#include <soc/dnx/legacy/SAND/Management/sand_general_macros.h>


/* } */
/*************
* DEFINES   *
*************/
/* { */

/* $Id: sand_array_memory_allocator.h,v 1.3 Broadcom SDK $
 * indicates NULL pointer.
 */
#define DNX_SAND_ARR_MEM_ALLOCATOR_NULL DNX_SAND_U32_MAX
#define DNX_SAND_ARR_MEM_ALLOCATOR_MIN_MALLOC_SIZE 2

#define DNX_SAND_ARR_MEM_ALLOCATOR_MAX_NOF_REQS 3

/* } */

/*************
* MACROS    *
*************/
/* { */

#define DNX_SAND_ARR_MEM_ALLOCATOR_ITER_END(iter)   \
        ((iter)->offset == DNX_SAND_ARR_MEM_ALLOCATOR_NULL)

#define DNX_SAND_ARR_MEM_ALLOCATOR_IS_CACHED_INST( _inst)   \
        (uint8)DNX_SAND_GET_BITS_RANGE(_inst,31,31)

#define DNX_SAND_ARR_MEM_ALLOCATOR_BANK_INST(_inst)   \
  DNX_SAND_GET_BITS_RANGE(_inst,30,0)


/* } */

/*************
* TYPE DEFS *
*************/
/* { */

typedef  uint32 DNX_SAND_ARR_MEM_ALLOCATOR_ENTRY;
typedef  uint32 DNX_SAND_ARR_MEM_ALLOCATOR_PTR;

/*
 * the ADT of arr_mem_alloc, use this type to manipulate the ARR_MEM_ALLOC
 */
typedef uint32  DNX_SAND_ARR_MEM_ALLOCATOR_ID;



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

/*********************************************************************
* NAME:
*     DNX_SAND_ARR_MEM_ALLOCATOR_ENTRY_SET
* FUNCTION:
*  call back to set information from the SW DB of the device.
*  set the information of entry_ndx
* INPUT:
*  DNX_SAND_IN  uint32             entry_ndx -
*   the place (id) of the entry to set
*  DNX_SAND_IN  DNX_SAND_ARR_MEM_ALLOCATOR_ENTRY       *entry -
*   the information to set for the entry.
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
typedef
  uint32
  (*DNX_SAND_ARR_MEM_ALLOCATOR_ENTRY_SET)(
    DNX_SAND_IN  int                             prime_handle,
    DNX_SAND_IN  uint32                             sec_handle,
    DNX_SAND_IN  uint32                             entry_ndx,
    DNX_SAND_IN  DNX_SAND_ARR_MEM_ALLOCATOR_ENTRY         *entry
  );


/*********************************************************************
* NAME:
*     DNX_SAND_ARR_MEM_ALLOCATOR_ENTRY_GET
* FUNCTION:
*  call back to get the information from the SW DB of the device.
*  Get the information of entry_ndx
* INPUT:
*  DNX_SAND_IN  uint32             entry_ndx -
*   the place (id) of the entry to set
*  DNX_SAND_IN  DNX_SAND_ARR_MEM_ALLOCATOR_ENTRY       *entry -
*   the information entry of the entry to get.
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
typedef
  uint32
    (*DNX_SAND_ARR_MEM_ALLOCATOR_ENTRY_GET)(
    DNX_SAND_IN  int                             prime_handle,
    DNX_SAND_IN  uint32                             sec_handle,
    DNX_SAND_IN  uint32                             entry_ndx,
    DNX_SAND_OUT  DNX_SAND_ARR_MEM_ALLOCATOR_ENTRY        *entry
  );


/*********************************************************************
* NAME:
*     DNX_SAND_ARR_MEM_ALLOCATOR_ENTRY_SET
* FUNCTION:
*  call back to write the data to the memory.
* INPUT:
*  DNX_SAND_IN  uint32             offset -
*   the place (id) of the entry to set
*  DNX_SAND_IN  DNX_SAND_ARR_MEM_ALLOCATOR_ENTRY       *data -
*   the information to write
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
typedef
  uint32
  (*DNX_SAND_ARR_MEM_ALLOCATOR_ROW_WRITE)(
    DNX_SAND_IN  int                             prime_handle,
    DNX_SAND_IN  uint32                             sec_handle,
    DNX_SAND_IN  uint32                             offset,
    DNX_SAND_IN  DNX_SAND_ARR_MEM_ALLOCATOR_ENTRY         *data
  );

/*********************************************************************
* NAME:
*     DNX_SAND_ARR_MEM_ALLOCATOR_ENTRY_MOVE_CALL_BACK
* FUNCTION:
*  call back to call when entry is moved
* INPUT:
*  DNX_SAND_IN  DNX_SAND_ARR_MEM_ALLOCATOR_PTR           old_place -
*   moved from
*  DNX_SAND_IN  DNX_SAND_ARR_MEM_ALLOCATOR_PTR           new_place -
*   entry moved to
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
typedef
  uint32
  (*DNX_SAND_ARR_MEM_ALLOCATOR_ENTRY_MOVE_CALL_BACK)(
    DNX_SAND_IN  int                             prime_handle,
    DNX_SAND_IN  uint32                             sec_handle,
    DNX_SAND_IN  DNX_SAND_ARR_MEM_ALLOCATOR_ENTRY         *data,
    DNX_SAND_IN  DNX_SAND_ARR_MEM_ALLOCATOR_PTR           old_place,
    DNX_SAND_IN  DNX_SAND_ARR_MEM_ALLOCATOR_PTR           new_place
  );

/*********************************************************************
* NAME:
*     DNX_SAND_ARR_MEM_ALLOCATOR_ENTRY_SET
* FUNCTION:
*  call back to write the data to the memory.
* INPUT:
*  DNX_SAND_IN  uint32             offset -
*   the place (id) of the entry to set
*  DNX_SAND_OUT  DNX_SAND_ARR_MEM_ALLOCATOR_ENTRY       *data -
*   the information to write
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
typedef
  uint32
  (*DNX_SAND_ARR_MEM_ALLOCATOR_ROW_READ)(
    DNX_SAND_IN  int                             prime_handle,
    DNX_SAND_IN  uint32                             sec_handle,
    DNX_SAND_IN  uint32                             offset,
    DNX_SAND_OUT  DNX_SAND_ARR_MEM_ALLOCATOR_ENTRY        *data
  );


/*********************************************************************
* NAME:
*     DNX_SAND_ARR_MEM_ALLOCATOR_FREE_LIST_SET
* FUNCTION:
*  call back to set information from the SW DB of the device.
*  set the information of entry_ndx
* INPUT:
*  DNX_SAND_IN  uint32             entry_ndx -
*   the place (id) of the entry to set
*  DNX_SAND_IN  DNX_SAND_ARR_MEM_ALLOCATOR_ENTRY       *entry -
*   the information to set for the entry.
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
typedef
  uint32
  (*DNX_SAND_ARR_MEM_ALLOCATOR_FREE_LIST_SET)(
    DNX_SAND_IN  int                             prime_handle,
    DNX_SAND_IN  uint32                             sec_handle,
    DNX_SAND_IN  DNX_SAND_ARR_MEM_ALLOCATOR_PTR           free_ptr
  );


/*********************************************************************
* NAME:
*     DNX_SAND_ARR_MEM_ALLOCATOR_FREE_LIST_GET
* FUNCTION:
*  call back to set information from the SW DB of the device.
*  set the information of entry_ndx
* INPUT:
*  DNX_SAND_IN  uint32             entry_ndx -
*   the place (id) of the entry to set
*  DNX_SAND_IN  DNX_SAND_ARR_MEM_ALLOCATOR_ENTRY       *entry -
*   the information to set for the entry.
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
typedef
  uint32
  (*DNX_SAND_ARR_MEM_ALLOCATOR_FREE_LIST_GET)(
    DNX_SAND_IN  int                             prime_handle,
    DNX_SAND_IN  uint32                             sec_handle,
    DNX_SAND_OUT  DNX_SAND_ARR_MEM_ALLOCATOR_PTR          *free_ptr
  );

typedef struct
{
 /*
  * array to include entries
  */
  DNX_SAND_ARR_MEM_ALLOCATOR_ENTRY *array;

  DNX_SAND_ARR_MEM_ALLOCATOR_PTR free_list;

  uint32  *mem_shadow;
 /*
  * caching management
  */
  uint8 cache_enabled;

  DNX_SAND_ARR_MEM_ALLOCATOR_ENTRY *array_cache;

  DNX_SAND_ARR_MEM_ALLOCATOR_PTR free_list_cache;

  uint32 nof_updates;

  DNX_SAND_ARR_MEM_ALLOCATOR_ENTRY *update_indexes;

  uint32  *mem_shadow_cache;


} DNX_SAND_ARR_MEM_ALLOCATOR_T;

/*
 * includes the information user has to supply in the arr_mem_alloc creation
 */
typedef struct {
  DNX_SAND_MAGIC_NUM_VAR
 /*
  * primary handle identify the created instance
  */
  uint32 instance_prim_handle;
 /*
  * secondary handle identify the created instance
  */
  uint32 instance_sec_handle;
 /*
  * number of entries,
  */
  uint32 nof_entries;
 /*
  * allocator support caching
  */
  uint8 support_caching;
 /*
  * allocator support defragment
  */
  uint8 support_defragment;
 /*
  * max block size
  */
  uint32 max_block_size;
 /*
  * call backs to set/get the data from the device SW database.
  */
  DNX_SAND_ARR_MEM_ALLOCATOR_ENTRY_SET entry_set_fun;
  DNX_SAND_ARR_MEM_ALLOCATOR_ENTRY_GET entry_get_fun;
  DNX_SAND_ARR_MEM_ALLOCATOR_FREE_LIST_SET free_set_fun;
  DNX_SAND_ARR_MEM_ALLOCATOR_FREE_LIST_GET free_get_fun;
  DNX_SAND_ARR_MEM_ALLOCATOR_ROW_READ read_fun;
  DNX_SAND_ARR_MEM_ALLOCATOR_ROW_WRITE write_fun;
  DNX_SAND_ARR_MEM_ALLOCATOR_ENTRY_MOVE_CALL_BACK entry_move_fun;

 /*
  * this to be manipulated by the dnx_sand, caller shouldn't
  * modify these information, used to interconnect specific device memory
  * with the general algorithm in the dnx_sand.
  */
  DNX_SAND_ARR_MEM_ALLOCATOR_T arr_mem_allocator_data;

} DNX_SAND_ARR_MEM_ALLOCATOR_INFO;


typedef struct {
  DNX_SAND_MAGIC_NUM_VAR
 /*
  * the next entry to read
  */
  DNX_SAND_ARR_MEM_ALLOCATOR_PTR offset;
 /*
  * the number of entries from the offset to the end of this block.
  */
  uint32 block_size;

} DNX_SAND_ARR_MEM_ALLOCATOR_ITER;


typedef struct {
  DNX_SAND_MAGIC_NUM_VAR
 /*
  * list of requirement on block sizes
  */
  uint32 block_size[DNX_SAND_ARR_MEM_ALLOCATOR_MAX_NOF_REQS];

  uint32 nof_reqs;

} DNX_SAND_ARR_MEM_ALLOCATOR_REQ_BLOCKS;


typedef struct {
  DNX_SAND_MAGIC_NUM_VAR
 /*
  * number of total free entries
  */
  uint32 total_size;
 /*
  * number of total free entries
  */
  uint32 total_free;
 /*
  * number of fragments entries
  */
  uint32 nof_fragments;
 /*
  * Maximum size free block
  */
  uint32 max_free_block_size;

} DNX_SAND_ARR_MEM_ALLOCATOR_MEM_STATUS;


typedef struct {
  DNX_SAND_MAGIC_NUM_VAR
 /*
  *  entries_to_scan: number of writes to HW 
  *           set to DNX_SAND_TBL_ITER_SCAN_ALL to ignore.
  *  entries_to_act:  number of fragments to unify
  *           set to DNX_SAND_TBL_ITER_SCAN_ALL to perform full defragmentation.
  *  iter: use DNX_SAND_TBL_ITER_IS_END to check if defragmention is fully
  *        done.
  */
  DNX_SAND_TABLE_BLOCK_RANGE             block_range;

} DNX_SAND_ARR_MEM_ALLOCATOR_DEFRAG_INFO;


typedef struct {
  DNX_SAND_MAGIC_NUM_VAR
  /* flags */
  uint32 flags;

  DNX_SAND_ARR_MEM_ALLOCATOR_ENTRY_SET entry_set_fun;
  DNX_SAND_ARR_MEM_ALLOCATOR_ENTRY_GET entry_get_fun;
  DNX_SAND_ARR_MEM_ALLOCATOR_FREE_LIST_SET free_set_fun;
  DNX_SAND_ARR_MEM_ALLOCATOR_FREE_LIST_GET free_get_fun;
  DNX_SAND_ARR_MEM_ALLOCATOR_ROW_READ read_fun;
  DNX_SAND_ARR_MEM_ALLOCATOR_ROW_WRITE write_fun;
  DNX_SAND_ARR_MEM_ALLOCATOR_ENTRY_MOVE_CALL_BACK entry_move_fun;

} DNX_SAND_ARR_MEM_ALLOCATOR_LOAD_INFO;


/*********************************************************************
* NAME:
*     dnx_sand_arr_mem_allocator_create
* TYPE:
*   PROC
* DATE:
*   Mar  6 2008
* FUNCTION:
*     Creates a new arr_mem_alloc instance.
* INPUT:
*   DNX_SAND_INOUT  DNX_SAND_ARR_MEM_ALLOCATOR_INFO              *arr_mem_info -
*     information to use in order to create the arr_mem_alloc
* REMARKS:
*   None.
* RETURNS:
*   OK or ERROR indication.
*********************************************************************/
uint32
  dnx_sand_arr_mem_allocator_create(
    DNX_SAND_INOUT   DNX_SAND_ARR_MEM_ALLOCATOR_INFO      *arr_mem_info
  );

/*********************************************************************
* NAME:
*     dnx_sand_arr_mem_allocator_clear
* TYPE:
*   PROC
* DATE:
*   Mar  6 2008
* FUNCTION:
*     Clear the mem_mngmnt instance, as no allocation was performed
* INPUT:
*   DNX_SAND_IN  DNX_SAND_ARR_MEM_ALLOCATOR_INFO              *arr_mem_info -
*     the mem_mngmnt to clear
* REMARKS:
*   None.
* RETURNS:
*   OK or ERROR indication.
*********************************************************************/
uint32
  dnx_sand_arr_mem_allocator_clear(
    DNX_SAND_INOUT   DNX_SAND_ARR_MEM_ALLOCATOR_INFO      *arr_mem_info
  );

/*********************************************************************
* NAME:
*     dnx_sand_arr_mem_allocator_destroy
* TYPE:
*   PROC
* DATE:
*   Mar  6 2008
* FUNCTION:
*     free the arr_mem_alloc instance.
* INPUT:
*  DNX_SAND_OUT  DNX_SAND_ARR_MEM_ALLOCATOR_INFO arr_mem_info -
*     The arr_mem_alloc to destroy.
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  dnx_sand_arr_mem_allocator_destroy(
  DNX_SAND_INOUT   DNX_SAND_ARR_MEM_ALLOCATOR_INFO      *arr_mem_info
  );

/*********************************************************************
* NAME:
*     dnx_sand_arr_mem_allocator_create
* TYPE:
*   PROC
* DATE:
*   Mar  6 2008
* FUNCTION:
*     Creates a new arr_mem_alloc instance.
* INPUT:
*   DNX_SAND_INOUT  DNX_SAND_ARR_MEM_ALLOCATOR_INFO              *arr_mem_info -
*     information to use in order to create the arr_mem_alloc
* REMARKS:
*   None.
* RETURNS:
*   OK or ERROR indication.
*********************************************************************/
uint32
  dnx_sand_arr_mem_allocator_cache_set(
    DNX_SAND_INOUT   DNX_SAND_ARR_MEM_ALLOCATOR_INFO      *arr_mem_info,
    DNX_SAND_IN uint8                             enable
  );

/*********************************************************************
* NAME:
*     dnx_sand_arr_mem_allocator_create
* TYPE:
*   PROC
* DATE:
*   Mar  6 2008
* FUNCTION:
*     Creates a new arr_mem_alloc instance.
* INPUT:
*   DNX_SAND_INOUT  DNX_SAND_ARR_MEM_ALLOCATOR_INFO              *arr_mem_info -
*     information to use in order to create the arr_mem_alloc
* REMARKS:
*   None.
* RETURNS:
*   OK or ERROR indication.
*********************************************************************/
uint32
  dnx_sand_arr_mem_allocator_commit(
    DNX_SAND_INOUT   DNX_SAND_ARR_MEM_ALLOCATOR_INFO      *arr_mem_info,
    DNX_SAND_IN uint32 flags
  );

/*********************************************************************
* NAME:
*     dnx_sand_arr_mem_allocator_create
* TYPE:
*   PROC
* DATE:
*   Mar  6 2008
* FUNCTION:
*     Creates a new arr_mem_alloc instance.
* INPUT:
*   DNX_SAND_INOUT  DNX_SAND_ARR_MEM_ALLOCATOR_INFO              *arr_mem_info -
*     information to use in order to create the arr_mem_alloc
* REMARKS:
*   None.
* RETURNS:
*   OK or ERROR indication.
*********************************************************************/
uint32
  dnx_sand_arr_mem_allocator_rollback(
    DNX_SAND_INOUT   DNX_SAND_ARR_MEM_ALLOCATOR_INFO      *arr_mem_info,
    DNX_SAND_IN uint32 flags
  );

/*********************************************************************
* NAME:
*     dnx_sand_arr_mem_allocator_malloc
* TYPE:
*   PROC
* DATE:
*   May  6 2008
* FUNCTION:
*  malloc block of memory
* INPUT:
*   DNX_SAND_INOUT   DNX_SAND_ARR_MEM_ALLOCATOR_INFO     *arr_mem_info -
*     The arr_mem_alloc instance
*   DNX_SAND_IN  uint32                            size -
*     the size to allocate
*   DNX_SAND_IN  uint32                            alignment -
*     alignment of the base of the allocated block. set to
*     zero/1 for no alignment
*   DNX_SAND_OUT  DNX_SAND_ARR_MEM_ALLOCATOR_PTR         *ptr -
*     to point to the allocated memory.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  dnx_sand_arr_mem_allocator_malloc(
    DNX_SAND_INOUT   DNX_SAND_ARR_MEM_ALLOCATOR_INFO     *arr_mem_info,
    DNX_SAND_IN  uint32                            size,
    DNX_SAND_IN  uint32                            alignment,
    DNX_SAND_OUT  DNX_SAND_ARR_MEM_ALLOCATOR_PTR         *ptr
  );


/*********************************************************************
* NAME:
*     dnx_sand_arr_mem_allocator_free
* TYPE:
*   PROC
* DATE:
*   May  6 2008
* FUNCTION:
*  free block of memory
* INPUT:
*   DNX_SAND_INOUT   DNX_SAND_ARR_MEM_ALLOCATOR_INFO     *arr_mem_info -
*     The arr_mem_alloc instance
*   DNX_SAND_IN  DNX_SAND_ARR_MEM_ALLOCATOR_PTR          ptr -
*     pointer to the block to free.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  dnx_sand_arr_mem_allocator_free(
    DNX_SAND_INOUT   DNX_SAND_ARR_MEM_ALLOCATOR_INFO     *arr_mem_info,
    DNX_SAND_IN  DNX_SAND_ARR_MEM_ALLOCATOR_PTR          ptr
  );

/*********************************************************************
* NAME:
*     dnx_sand_arr_mem_allocator_block_size
* TYPE:
*   PROC
* DATE:
*   May  6 2008
* FUNCTION:
*  return the size of allocated memory pointer by ptr.
* INPUT:
*   DNX_SAND_INOUT   DNX_SAND_ARR_MEM_ALLOCATOR_INFO     *arr_mem_info -
*     The arr_mem_alloc instance
*   DNX_SAND_IN  DNX_SAND_ARR_MEM_ALLOCATOR_PTR          ptr -
*     pointer to the block
*   DNX_SAND_OUT  uint32                           *size -
*     size of the allocated block
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  dnx_sand_arr_mem_allocator_block_size(
    DNX_SAND_INOUT   DNX_SAND_ARR_MEM_ALLOCATOR_INFO     *arr_mem_info,
    DNX_SAND_IN  DNX_SAND_ARR_MEM_ALLOCATOR_PTR          ptr,
    DNX_SAND_OUT  uint32                           *size
  );
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
    DNX_SAND_IN  DNX_SAND_ARR_MEM_ALLOCATOR_ENTRY       *data
  );


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
  );


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
  );

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
  );


uint32
  dnx_sand_arr_mem_allocator_get_size_for_save(
    DNX_SAND_IN   DNX_SAND_ARR_MEM_ALLOCATOR_INFO      *arr_mem_info,
    DNX_SAND_IN  uint32                        flags,
    DNX_SAND_OUT  uint32                       *size
  );

uint32
  dnx_sand_arr_mem_allocator_save(
    DNX_SAND_IN   DNX_SAND_ARR_MEM_ALLOCATOR_INFO      *arr_mem_info,
    DNX_SAND_IN  uint32                flags,
    DNX_SAND_OUT uint8                 *buffer,
    DNX_SAND_IN  uint32                buffer_size_bytes,
    DNX_SAND_OUT uint32                *actual_size_bytes
  );

uint32
  dnx_sand_arr_mem_allocator_load(
    DNX_SAND_IN  uint8                           **buffer,
    DNX_SAND_IN  DNX_SAND_ARR_MEM_ALLOCATOR_LOAD_INFO  *load_info,
    DNX_SAND_OUT DNX_SAND_ARR_MEM_ALLOCATOR_INFO      *arr_mem_info
  );


uint32
  dnx_sand_arr_mem_allocator_print_free(
    DNX_SAND_INOUT   DNX_SAND_ARR_MEM_ALLOCATOR_INFO     *arr_mem_info
  );

uint32
  dnx_sand_arr_mem_allocator_print_free_by_order(
    DNX_SAND_INOUT   DNX_SAND_ARR_MEM_ALLOCATOR_INFO     *arr_mem_info
  );

uint32
  dnx_sand_arr_mem_allocator_is_availabe_blocks(
    DNX_SAND_INOUT   DNX_SAND_ARR_MEM_ALLOCATOR_INFO     *arr_mem_info,
	  DNX_SAND_IN   DNX_SAND_ARR_MEM_ALLOCATOR_REQ_BLOCKS  *req_blocks,
	  DNX_SAND_OUT   uint8                         *available
  );

uint32
  dnx_sand_arr_mem_allocator_mem_status_get(
    DNX_SAND_INOUT   DNX_SAND_ARR_MEM_ALLOCATOR_INFO     *arr_mem_info,
	  DNX_SAND_OUT   DNX_SAND_ARR_MEM_ALLOCATOR_MEM_STATUS  *mem_status
  );


void
  dnx_sand_SAND_ARR_MEM_ALLOCATOR_INFO_clear(
    DNX_SAND_ARR_MEM_ALLOCATOR_INFO *info
  );

void
  dnx_sand_SAND_ARR_MEM_ALLOCATOR_ITER_clear(
    DNX_SAND_ARR_MEM_ALLOCATOR_ITER *info
  );

void
  dnx_sand_SAND_ARR_MEM_ALLOCATOR_REQ_BLOCKS_clear(
    DNX_SAND_ARR_MEM_ALLOCATOR_REQ_BLOCKS *info
  );

void
  dnx_sand_SAND_ARR_MEM_ALLOCATOR_MEM_STATUS_clear(
    DNX_SAND_ARR_MEM_ALLOCATOR_MEM_STATUS *info
  );

void
  dnx_sand_SAND_ARR_MEM_ALLOCATOR_DEFRAG_INFO_clear(
    DNX_SAND_ARR_MEM_ALLOCATOR_DEFRAG_INFO *info
  );


void
  dnx_sand_SAND_ARR_MEM_ALLOCATOR_LOAD_INFO_clear(
    DNX_SAND_ARR_MEM_ALLOCATOR_LOAD_INFO *info
  );



#if DNX_SAND_DEBUG
void
  dnx_sand_SAND_ARR_MEM_ALLOCATOR_INFO_print(
    DNX_SAND_ARR_MEM_ALLOCATOR_INFO *info
  );

void
  dnx_sand_SAND_ARR_MEM_ALLOCATOR_ITER_print(
    DNX_SAND_ARR_MEM_ALLOCATOR_ITER *info
  );

void
  dnx_sand_SAND_ARR_MEM_ALLOCATOR_REQ_BLOCKS_print(
    DNX_SAND_IN DNX_SAND_ARR_MEM_ALLOCATOR_REQ_BLOCKS *info
  );

void
  dnx_sand_SAND_ARR_MEM_ALLOCATOR_MEM_STATUS_print(
    DNX_SAND_IN DNX_SAND_ARR_MEM_ALLOCATOR_MEM_STATUS *info
  );

void
  dnx_sand_SAND_ARR_MEM_ALLOCATOR_DEFRAG_INFO_print(
    DNX_SAND_IN DNX_SAND_ARR_MEM_ALLOCATOR_DEFRAG_INFO *info
  );
#endif /* DNX_SAND_DEBUG */

/* } */

#include <soc/dnx/legacy/SAND/Utils/sand_footer.h>

/* } __DNX_SAND_ARR_MEM_ALLOCATOR_INCLUDED__*/
#endif
