/* $Id: sand_array_memory_allocator.h,v 1.3 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/

#ifndef __SOC_SAND_ARRAY_MEMORY_ALLOCATOR_INCLUDED__
/* { */
#define __SOC_SAND_ARRAY_MEMORY_ALLOCATOR_INCLUDED__

/*************
* INCLUDES  *
*************/
/* { */
#include <soc/dpp/SAND/Utils/sand_header.h>

#include <soc/dpp/SAND/Management/sand_general_macros.h>
#include <soc/dpp/SAND/Utils/sand_framework.h>
#include <soc/dpp/SAND/Management/sand_general_macros.h>


/* } */
/*************
* DEFINES   *
*************/
/* { */

/* $Id: sand_array_memory_allocator.h,v 1.3 Broadcom SDK $
 * indicates NULL pointer.
 */
#define SOC_SAND_ARR_MEM_ALLOCATOR_NULL SOC_SAND_U32_MAX
#define SOC_SAND_ARR_MEM_ALLOCATOR_MIN_MALLOC_SIZE 2

#define SOC_SAND_ARR_MEM_ALLOCATOR_MAX_NOF_REQS 3

/* } */

/*************
* MACROS    *
*************/
/* { */

#define SOC_SAND_ARR_MEM_ALLOCATOR_ITER_END(iter)   \
        ((iter)->offset == SOC_SAND_ARR_MEM_ALLOCATOR_NULL)

#define SOC_SAND_ARR_MEM_ALLOCATOR_IS_CACHED_INST( _inst)   \
        (uint8)SOC_SAND_GET_BITS_RANGE(_inst,31,31)

#define SOC_SAND_ARR_MEM_ALLOCATOR_BANK_INST(_inst)   \
  SOC_SAND_GET_BITS_RANGE(_inst,30,0)


/* } */

/*************
* TYPE DEFS *
*************/
/* { */

typedef  uint32 SOC_SAND_ARR_MEM_ALLOCATOR_ENTRY;
typedef  uint32 SOC_SAND_ARR_MEM_ALLOCATOR_PTR;

/*
 * the ADT of arr_mem_alloc, use this type to manipulate the ARR_MEM_ALLOC
 */
typedef uint32  SOC_SAND_ARR_MEM_ALLOCATOR_ID;



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
*     SOC_SAND_ARR_MEM_ALLOCATOR_ENTRY_SET
* FUNCTION:
*  call back to set information from the SW DB of the device.
*  set the information of entry_ndx
* INPUT:
*  SOC_SAND_IN  uint32             entry_ndx -
*   the place (id) of the entry to set
*  SOC_SAND_IN  SOC_SAND_ARR_MEM_ALLOCATOR_ENTRY       *entry -
*   the information to set for the entry.
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
typedef
  uint32
  (*SOC_SAND_ARR_MEM_ALLOCATOR_ENTRY_SET)(
    SOC_SAND_IN  int                             prime_handle,
    SOC_SAND_IN  uint32                             sec_handle,
    SOC_SAND_IN  uint32                             entry_ndx,
    SOC_SAND_IN  SOC_SAND_ARR_MEM_ALLOCATOR_ENTRY         *entry
  );


/*********************************************************************
* NAME:
*     SOC_SAND_ARR_MEM_ALLOCATOR_ENTRY_GET
* FUNCTION:
*  call back to get the information from the SW DB of the device.
*  Get the information of entry_ndx
* INPUT:
*  SOC_SAND_IN  uint32             entry_ndx -
*   the place (id) of the entry to set
*  SOC_SAND_IN  SOC_SAND_ARR_MEM_ALLOCATOR_ENTRY       *entry -
*   the information entry of the entry to get.
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
typedef
  uint32
    (*SOC_SAND_ARR_MEM_ALLOCATOR_ENTRY_GET)(
    SOC_SAND_IN  int                             prime_handle,
    SOC_SAND_IN  uint32                             sec_handle,
    SOC_SAND_IN  uint32                             entry_ndx,
    SOC_SAND_OUT  SOC_SAND_ARR_MEM_ALLOCATOR_ENTRY        *entry
  );


/*********************************************************************
* NAME:
*     SOC_SAND_ARR_MEM_ALLOCATOR_ENTRY_SET
* FUNCTION:
*  call back to write the data to the memory.
* INPUT:
*  SOC_SAND_IN  uint32             offset -
*   the place (id) of the entry to set
*  SOC_SAND_IN  SOC_SAND_ARR_MEM_ALLOCATOR_ENTRY       *data -
*   the information to write
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
typedef
  uint32
  (*SOC_SAND_ARR_MEM_ALLOCATOR_ROW_WRITE)(
    SOC_SAND_IN  int                             prime_handle,
    SOC_SAND_IN  uint32                             sec_handle,
    SOC_SAND_IN  uint32                             offset,
    SOC_SAND_IN  SOC_SAND_ARR_MEM_ALLOCATOR_ENTRY         *data
  );

/*********************************************************************
* NAME:
*     SOC_SAND_ARR_MEM_ALLOCATOR_ENTRY_MOVE_CALL_BACK
* FUNCTION:
*  call back to call when entry is moved
* INPUT:
*  SOC_SAND_IN  SOC_SAND_ARR_MEM_ALLOCATOR_PTR           old_place -
*   moved from
*  SOC_SAND_IN  SOC_SAND_ARR_MEM_ALLOCATOR_PTR           new_place -
*   entry moved to
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
typedef
  uint32
  (*SOC_SAND_ARR_MEM_ALLOCATOR_ENTRY_MOVE_CALL_BACK)(
    SOC_SAND_IN  int                             prime_handle,
    SOC_SAND_IN  uint32                             sec_handle,
    SOC_SAND_IN  SOC_SAND_ARR_MEM_ALLOCATOR_ENTRY         *data,
    SOC_SAND_IN  SOC_SAND_ARR_MEM_ALLOCATOR_PTR           old_place,
    SOC_SAND_IN  SOC_SAND_ARR_MEM_ALLOCATOR_PTR           new_place
  );

/*********************************************************************
* NAME:
*     SOC_SAND_ARR_MEM_ALLOCATOR_ENTRY_SET
* FUNCTION:
*  call back to write the data to the memory.
* INPUT:
*  SOC_SAND_IN  uint32             offset -
*   the place (id) of the entry to set
*  SOC_SAND_OUT  SOC_SAND_ARR_MEM_ALLOCATOR_ENTRY       *data -
*   the information to write
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
typedef
  uint32
  (*SOC_SAND_ARR_MEM_ALLOCATOR_ROW_READ)(
    SOC_SAND_IN  int                             prime_handle,
    SOC_SAND_IN  uint32                             sec_handle,
    SOC_SAND_IN  uint32                             offset,
    SOC_SAND_OUT  SOC_SAND_ARR_MEM_ALLOCATOR_ENTRY        *data
  );


/*********************************************************************
* NAME:
*     SOC_SAND_ARR_MEM_ALLOCATOR_FREE_LIST_SET
* FUNCTION:
*  call back to set information from the SW DB of the device.
*  set the information of entry_ndx
* INPUT:
*  SOC_SAND_IN  uint32             entry_ndx -
*   the place (id) of the entry to set
*  SOC_SAND_IN  SOC_SAND_ARR_MEM_ALLOCATOR_ENTRY       *entry -
*   the information to set for the entry.
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
typedef
  uint32
  (*SOC_SAND_ARR_MEM_ALLOCATOR_FREE_LIST_SET)(
    SOC_SAND_IN  int                             prime_handle,
    SOC_SAND_IN  uint32                             sec_handle,
    SOC_SAND_IN  SOC_SAND_ARR_MEM_ALLOCATOR_PTR           free_ptr
  );


/*********************************************************************
* NAME:
*     SOC_SAND_ARR_MEM_ALLOCATOR_FREE_LIST_GET
* FUNCTION:
*  call back to set information from the SW DB of the device.
*  set the information of entry_ndx
* INPUT:
*  SOC_SAND_IN  uint32             entry_ndx -
*   the place (id) of the entry to set
*  SOC_SAND_IN  SOC_SAND_ARR_MEM_ALLOCATOR_ENTRY       *entry -
*   the information to set for the entry.
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
typedef
  uint32
  (*SOC_SAND_ARR_MEM_ALLOCATOR_FREE_LIST_GET)(
    SOC_SAND_IN  int                             prime_handle,
    SOC_SAND_IN  uint32                             sec_handle,
    SOC_SAND_OUT  SOC_SAND_ARR_MEM_ALLOCATOR_PTR          *free_ptr
  );

typedef struct
{
 /*
  * array to include entries
  */
  SOC_SAND_ARR_MEM_ALLOCATOR_ENTRY *array;

  SOC_SAND_ARR_MEM_ALLOCATOR_PTR free_list;

  uint32  *mem_shadow;
 /*
  * caching management
  */
  uint8 cache_enabled;

  SOC_SAND_ARR_MEM_ALLOCATOR_ENTRY *array_cache;

  SOC_SAND_ARR_MEM_ALLOCATOR_PTR free_list_cache;

  uint32 nof_updates;

  SOC_SAND_ARR_MEM_ALLOCATOR_ENTRY *update_indexes;

  uint32  *mem_shadow_cache;


} SOC_SAND_ARR_MEM_ALLOCATOR_T;

/*
 * includes the information user has to supply in the arr_mem_alloc creation
 */
typedef struct {
  SOC_SAND_MAGIC_NUM_VAR
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
  SOC_SAND_ARR_MEM_ALLOCATOR_ENTRY_SET entry_set_fun;
  SOC_SAND_ARR_MEM_ALLOCATOR_ENTRY_GET entry_get_fun;
  SOC_SAND_ARR_MEM_ALLOCATOR_FREE_LIST_SET free_set_fun;
  SOC_SAND_ARR_MEM_ALLOCATOR_FREE_LIST_GET free_get_fun;
  SOC_SAND_ARR_MEM_ALLOCATOR_ROW_READ read_fun;
  SOC_SAND_ARR_MEM_ALLOCATOR_ROW_WRITE write_fun;
  SOC_SAND_ARR_MEM_ALLOCATOR_ENTRY_MOVE_CALL_BACK entry_move_fun;

 /*
  * this to be manipulated by the soc_sand, caller shouldn't
  * modify these information, used to interconnect specific device memory
  * with the general algorithm in the soc_sand.
  */
  SOC_SAND_ARR_MEM_ALLOCATOR_T arr_mem_allocator_data;

} SOC_SAND_ARR_MEM_ALLOCATOR_INFO;


typedef struct {
  SOC_SAND_MAGIC_NUM_VAR
 /*
  * the next entry to read
  */
  SOC_SAND_ARR_MEM_ALLOCATOR_PTR offset;
 /*
  * the number of entries from the offset to the end of this block.
  */
  uint32 block_size;

} SOC_SAND_ARR_MEM_ALLOCATOR_ITER;


typedef struct {
  SOC_SAND_MAGIC_NUM_VAR
 /*
  * list of requirement on block sizes
  */
  uint32 block_size[SOC_SAND_ARR_MEM_ALLOCATOR_MAX_NOF_REQS];

  uint32 nof_reqs;

} SOC_SAND_ARR_MEM_ALLOCATOR_REQ_BLOCKS;


typedef struct {
  SOC_SAND_MAGIC_NUM_VAR
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

} SOC_SAND_ARR_MEM_ALLOCATOR_MEM_STATUS;


typedef struct {
  SOC_SAND_MAGIC_NUM_VAR
 /*
  *  entries_to_scan: number of writes to HW 
  *           set to SOC_SAND_TBL_ITER_SCAN_ALL to ignore.
  *  entries_to_act:  number of fragments to unify
  *           set to SOC_SAND_TBL_ITER_SCAN_ALL to perform full defragmentation.
  *  iter: use SOC_SAND_TBL_ITER_IS_END to check if defragmention is fully
  *        done.
  */
  SOC_SAND_TABLE_BLOCK_RANGE             block_range;

} SOC_SAND_ARR_MEM_ALLOCATOR_DEFRAG_INFO;


typedef struct {
  SOC_SAND_MAGIC_NUM_VAR
  /* flags */
  uint32 flags;

  SOC_SAND_ARR_MEM_ALLOCATOR_ENTRY_SET entry_set_fun;
  SOC_SAND_ARR_MEM_ALLOCATOR_ENTRY_GET entry_get_fun;
  SOC_SAND_ARR_MEM_ALLOCATOR_FREE_LIST_SET free_set_fun;
  SOC_SAND_ARR_MEM_ALLOCATOR_FREE_LIST_GET free_get_fun;
  SOC_SAND_ARR_MEM_ALLOCATOR_ROW_READ read_fun;
  SOC_SAND_ARR_MEM_ALLOCATOR_ROW_WRITE write_fun;
  SOC_SAND_ARR_MEM_ALLOCATOR_ENTRY_MOVE_CALL_BACK entry_move_fun;

} SOC_SAND_ARR_MEM_ALLOCATOR_LOAD_INFO;


/*********************************************************************
* NAME:
*     soc_sand_arr_mem_allocator_create
* TYPE:
*   PROC
* DATE:
*   Mar  6 2008
* FUNCTION:
*     Creates a new arr_mem_alloc instance.
* INPUT:
*   SOC_SAND_INOUT  SOC_SAND_ARR_MEM_ALLOCATOR_INFO              *arr_mem_info -
*     information to use in order to create the arr_mem_alloc
* REMARKS:
*   None.
* RETURNS:
*   OK or ERROR indication.
*********************************************************************/
uint32
  soc_sand_arr_mem_allocator_create(
    SOC_SAND_INOUT   SOC_SAND_ARR_MEM_ALLOCATOR_INFO      *arr_mem_info
  );

/*********************************************************************
* NAME:
*     soc_sand_arr_mem_allocator_clear
* TYPE:
*   PROC
* DATE:
*   Mar  6 2008
* FUNCTION:
*     Clear the mem_mngmnt instance, as no allocation was performed
* INPUT:
*   SOC_SAND_IN  SOC_SAND_ARR_MEM_ALLOCATOR_INFO              *arr_mem_info -
*     the mem_mngmnt to clear
* REMARKS:
*   None.
* RETURNS:
*   OK or ERROR indication.
*********************************************************************/
uint32
  soc_sand_arr_mem_allocator_clear(
    SOC_SAND_INOUT   SOC_SAND_ARR_MEM_ALLOCATOR_INFO      *arr_mem_info
  );

/*********************************************************************
* NAME:
*     soc_sand_arr_mem_allocator_destroy
* TYPE:
*   PROC
* DATE:
*   Mar  6 2008
* FUNCTION:
*     free the arr_mem_alloc instance.
* INPUT:
*  SOC_SAND_OUT  SOC_SAND_ARR_MEM_ALLOCATOR_INFO arr_mem_info -
*     The arr_mem_alloc to destroy.
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  soc_sand_arr_mem_allocator_destroy(
  SOC_SAND_INOUT   SOC_SAND_ARR_MEM_ALLOCATOR_INFO      *arr_mem_info
  );

/*********************************************************************
* NAME:
*     soc_sand_arr_mem_allocator_create
* TYPE:
*   PROC
* DATE:
*   Mar  6 2008
* FUNCTION:
*     Creates a new arr_mem_alloc instance.
* INPUT:
*   SOC_SAND_INOUT  SOC_SAND_ARR_MEM_ALLOCATOR_INFO              *arr_mem_info -
*     information to use in order to create the arr_mem_alloc
* REMARKS:
*   None.
* RETURNS:
*   OK or ERROR indication.
*********************************************************************/
uint32
  soc_sand_arr_mem_allocator_cache_set(
    SOC_SAND_INOUT   SOC_SAND_ARR_MEM_ALLOCATOR_INFO      *arr_mem_info,
    SOC_SAND_IN uint8                             enable
  );

/*********************************************************************
* NAME:
*     soc_sand_arr_mem_allocator_create
* TYPE:
*   PROC
* DATE:
*   Mar  6 2008
* FUNCTION:
*     Creates a new arr_mem_alloc instance.
* INPUT:
*   SOC_SAND_INOUT  SOC_SAND_ARR_MEM_ALLOCATOR_INFO              *arr_mem_info -
*     information to use in order to create the arr_mem_alloc
* REMARKS:
*   None.
* RETURNS:
*   OK or ERROR indication.
*********************************************************************/
uint32
  soc_sand_arr_mem_allocator_commit(
    SOC_SAND_INOUT   SOC_SAND_ARR_MEM_ALLOCATOR_INFO      *arr_mem_info,
    SOC_SAND_IN uint32 flags
  );

/*********************************************************************
* NAME:
*     soc_sand_arr_mem_allocator_create
* TYPE:
*   PROC
* DATE:
*   Mar  6 2008
* FUNCTION:
*     Creates a new arr_mem_alloc instance.
* INPUT:
*   SOC_SAND_INOUT  SOC_SAND_ARR_MEM_ALLOCATOR_INFO              *arr_mem_info -
*     information to use in order to create the arr_mem_alloc
* REMARKS:
*   None.
* RETURNS:
*   OK or ERROR indication.
*********************************************************************/
uint32
  soc_sand_arr_mem_allocator_rollback(
    SOC_SAND_INOUT   SOC_SAND_ARR_MEM_ALLOCATOR_INFO      *arr_mem_info,
    SOC_SAND_IN uint32 flags
  );

/*********************************************************************
* NAME:
*     soc_sand_arr_mem_allocator_malloc
* TYPE:
*   PROC
* DATE:
*   May  6 2008
* FUNCTION:
*  malloc block of memory
* INPUT:
*   SOC_SAND_INOUT   SOC_SAND_ARR_MEM_ALLOCATOR_INFO     *arr_mem_info -
*     The arr_mem_alloc instance
*   SOC_SAND_IN  uint32                            size -
*     the size to allocate
*   SOC_SAND_IN  uint32                            alignment -
*     alignment of the base of the allocated block. set to
*     zero/1 for no alignment
*   SOC_SAND_OUT  SOC_SAND_ARR_MEM_ALLOCATOR_PTR         *ptr -
*     to point to the allocated memory.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  soc_sand_arr_mem_allocator_malloc(
    SOC_SAND_INOUT   SOC_SAND_ARR_MEM_ALLOCATOR_INFO     *arr_mem_info,
    SOC_SAND_IN  uint32                            size,
    SOC_SAND_IN  uint32                            alignment,
    SOC_SAND_OUT  SOC_SAND_ARR_MEM_ALLOCATOR_PTR         *ptr
  );


/*********************************************************************
* NAME:
*     soc_sand_arr_mem_allocator_free
* TYPE:
*   PROC
* DATE:
*   May  6 2008
* FUNCTION:
*  free block of memory
* INPUT:
*   SOC_SAND_INOUT   SOC_SAND_ARR_MEM_ALLOCATOR_INFO     *arr_mem_info -
*     The arr_mem_alloc instance
*   SOC_SAND_IN  SOC_SAND_ARR_MEM_ALLOCATOR_PTR          ptr -
*     pointer to the block to free.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  soc_sand_arr_mem_allocator_free(
    SOC_SAND_INOUT   SOC_SAND_ARR_MEM_ALLOCATOR_INFO     *arr_mem_info,
    SOC_SAND_IN  SOC_SAND_ARR_MEM_ALLOCATOR_PTR          ptr
  );

/*********************************************************************
* NAME:
*     soc_sand_arr_mem_allocator_block_size
* TYPE:
*   PROC
* DATE:
*   May  6 2008
* FUNCTION:
*  return the size of allocated memory pointer by ptr.
* INPUT:
*   SOC_SAND_INOUT   SOC_SAND_ARR_MEM_ALLOCATOR_INFO     *arr_mem_info -
*     The arr_mem_alloc instance
*   SOC_SAND_IN  SOC_SAND_ARR_MEM_ALLOCATOR_PTR          ptr -
*     pointer to the block
*   SOC_SAND_OUT  uint32                           *size -
*     size of the allocated block
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  soc_sand_arr_mem_allocator_block_size(
    SOC_SAND_INOUT   SOC_SAND_ARR_MEM_ALLOCATOR_INFO     *arr_mem_info,
    SOC_SAND_IN  SOC_SAND_ARR_MEM_ALLOCATOR_PTR          ptr,
    SOC_SAND_OUT  uint32                           *size
  );
/*********************************************************************
* NAME:
*     soc_sand_arr_mem_allocator_write
* TYPE:
*   PROC
* DATE:
*   May  6 2008
* FUNCTION:
*  write row of data to the memory.
* INPUT:
*   SOC_SAND_INOUT   SOC_SAND_ARR_MEM_ALLOCATOR_INFO     *arr_mem_info -
*     The arr_mem_alloc instance
*   SOC_SAND_IN  SOC_SAND_ARR_MEM_ALLOCATOR_PTR     offset -
*     the entry to write to.
*   SOC_SAND_IN  SOC_SAND_ARR_MEM_ALLOCATOR_ENTRY        *data -
*     to hold the written data.
*     size has to be as row size.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  soc_sand_arr_mem_allocator_write(
    SOC_SAND_INOUT   SOC_SAND_ARR_MEM_ALLOCATOR_INFO     *arr_mem_info,
    SOC_SAND_IN  SOC_SAND_ARR_MEM_ALLOCATOR_PTR          offset,
    SOC_SAND_IN  SOC_SAND_ARR_MEM_ALLOCATOR_ENTRY       *data
  );


/*********************************************************************
* NAME:
*     soc_sand_arr_mem_allocator_read
* TYPE:
*   PROC
* DATE:
*   May  6 2008
* FUNCTION:
*  read row of data from the memory.
* INPUT:
*   SOC_SAND_INOUT   SOC_SAND_ARR_MEM_ALLOCATOR_INFO     *arr_mem_info -
*     The arr_mem_alloc instance
*   SOC_SAND_IN  SOC_SAND_ARR_MEM_ALLOCATOR_PTR     offset -
*     the entry to read from.
*   SOC_SAND_OUT  SOC_SAND_ARR_MEM_ALLOCATOR_ENTRY        *data -
*     to hold the readen data.
*     size has to be as row size.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  soc_sand_arr_mem_allocator_read(
    SOC_SAND_INOUT   SOC_SAND_ARR_MEM_ALLOCATOR_INFO     *arr_mem_info,
    SOC_SAND_IN  SOC_SAND_ARR_MEM_ALLOCATOR_PTR          offset,
    SOC_SAND_OUT  SOC_SAND_ARR_MEM_ALLOCATOR_ENTRY        *data
  );


/*********************************************************************
* NAME:
*     soc_sand_arr_mem_allocator_defrag
* TYPE:
*   PROC
* DATE:
*   May  6 2008
* FUNCTION:
*  perform defragmentation for the memory.
* INPUT:
*   SOC_SAND_INOUT   SOC_SAND_ARR_MEM_ALLOCATOR_INFO     *arr_mem_info -
*     The arr_mem_alloc instance to read from.
*   SOC_SAND_INOUT  SOC_SAND_ARR_MEM_ALLOCATOR_DEFRAG_INFO       *defrag_info -
*     defragmentation info
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  soc_sand_arr_mem_allocator_defrag(
    SOC_SAND_INOUT   SOC_SAND_ARR_MEM_ALLOCATOR_INFO       *arr_mem_info,
    SOC_SAND_INOUT SOC_SAND_ARR_MEM_ALLOCATOR_DEFRAG_INFO  *defrag_info
  );

/*********************************************************************
* NAME:
*     soc_sand_arr_mem_allocator_read_block
* TYPE:
*   PROC
* DATE:
*   May  6 2008
* FUNCTION:
*  read block of entries from the the memory.
* INPUT:
*   SOC_SAND_INOUT   SOC_SAND_ARR_MEM_ALLOCATOR_INFO     *arr_mem_info -
*     The arr_mem_alloc instance to read from.
*   SOC_SAND_INOUT  SOC_SAND_ARR_MEM_ALLOCATOR_ITER       *iter -
*     iterator to traverse the memory.
*     use soc_sand_SAND_ARR_MEM_ALLOCATOR_ITER_clear(iter) to
*     clear the iterator and start from the beginning of the
*     memory.
*     use SOC_SAND_ARR_MEM_ALLOCATOR_ITER_END(iter) to
*     check if the iterator reached the end of the memory.
*   SOC_SAND_IN  uint32                             entries_to_read -
*     number of valid (used) entries to get.
*   SOC_SAND_OUT  SOC_SAND_ARR_MEM_ALLOCATOR_ENTRY        *data -
*     buffer to hold the readen data has to be of size 'entries_to_read'
*     at least.
*   SOC_SAND_OUT  SOC_SAND_ARR_MEM_ALLOCATOR_PTR          *addresses -
*     buffer to hold the addresses of the valid entries.
*     has to be of size 'entries_to_read'
*   SOC_SAND_OUT  uint32                            *nof_entries
*     number of valid entries in data and addresses
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  soc_sand_arr_mem_allocator_read_block(
    SOC_SAND_INOUT   SOC_SAND_ARR_MEM_ALLOCATOR_INFO      *arr_mem_info,
    SOC_SAND_INOUT  SOC_SAND_ARR_MEM_ALLOCATOR_ITER       *iter,
    SOC_SAND_IN  uint32                             entries_to_read,
    SOC_SAND_OUT  SOC_SAND_ARR_MEM_ALLOCATOR_ENTRY        *data,
    SOC_SAND_OUT  SOC_SAND_ARR_MEM_ALLOCATOR_PTR          *addresses,
    SOC_SAND_OUT  uint32                            *nof_entries
  );


uint32
  soc_sand_arr_mem_allocator_get_size_for_save(
    SOC_SAND_IN   SOC_SAND_ARR_MEM_ALLOCATOR_INFO      *arr_mem_info,
    SOC_SAND_IN  uint32                        flags,
    SOC_SAND_OUT  uint32                       *size
  );

uint32
  soc_sand_arr_mem_allocator_save(
    SOC_SAND_IN   SOC_SAND_ARR_MEM_ALLOCATOR_INFO      *arr_mem_info,
    SOC_SAND_IN  uint32                flags,
    SOC_SAND_OUT uint8                 *buffer,
    SOC_SAND_IN  uint32                buffer_size_bytes,
    SOC_SAND_OUT uint32                *actual_size_bytes
  );

uint32
  soc_sand_arr_mem_allocator_load(
    SOC_SAND_IN  uint8                           **buffer,
    SOC_SAND_IN  SOC_SAND_ARR_MEM_ALLOCATOR_LOAD_INFO  *load_info,
    SOC_SAND_OUT SOC_SAND_ARR_MEM_ALLOCATOR_INFO      *arr_mem_info
  );


uint32
  soc_sand_arr_mem_allocator_print_free(
    SOC_SAND_INOUT   SOC_SAND_ARR_MEM_ALLOCATOR_INFO     *arr_mem_info
  );

uint32
  soc_sand_arr_mem_allocator_print_free_by_order(
    SOC_SAND_INOUT   SOC_SAND_ARR_MEM_ALLOCATOR_INFO     *arr_mem_info
  );

uint32
  soc_sand_arr_mem_allocator_is_availabe_blocks(
    SOC_SAND_INOUT   SOC_SAND_ARR_MEM_ALLOCATOR_INFO     *arr_mem_info,
	  SOC_SAND_IN   SOC_SAND_ARR_MEM_ALLOCATOR_REQ_BLOCKS  *req_blocks,
	  SOC_SAND_OUT   uint8                         *available
  );

uint32
  soc_sand_arr_mem_allocator_mem_status_get(
    SOC_SAND_INOUT   SOC_SAND_ARR_MEM_ALLOCATOR_INFO     *arr_mem_info,
	  SOC_SAND_OUT   SOC_SAND_ARR_MEM_ALLOCATOR_MEM_STATUS  *mem_status
  );


void
  soc_sand_SAND_ARR_MEM_ALLOCATOR_INFO_clear(
    SOC_SAND_ARR_MEM_ALLOCATOR_INFO *info
  );

void
  soc_sand_SAND_ARR_MEM_ALLOCATOR_ITER_clear(
    SOC_SAND_ARR_MEM_ALLOCATOR_ITER *info
  );

void
  soc_sand_SAND_ARR_MEM_ALLOCATOR_REQ_BLOCKS_clear(
    SOC_SAND_ARR_MEM_ALLOCATOR_REQ_BLOCKS *info
  );

void
  soc_sand_SAND_ARR_MEM_ALLOCATOR_MEM_STATUS_clear(
    SOC_SAND_ARR_MEM_ALLOCATOR_MEM_STATUS *info
  );

void
  soc_sand_SAND_ARR_MEM_ALLOCATOR_DEFRAG_INFO_clear(
    SOC_SAND_ARR_MEM_ALLOCATOR_DEFRAG_INFO *info
  );


void
  soc_sand_SAND_ARR_MEM_ALLOCATOR_LOAD_INFO_clear(
    SOC_SAND_ARR_MEM_ALLOCATOR_LOAD_INFO *info
  );



#if SOC_SAND_DEBUG
void
  soc_sand_SAND_ARR_MEM_ALLOCATOR_INFO_print(
    SOC_SAND_ARR_MEM_ALLOCATOR_INFO *info
  );

void
  soc_sand_SAND_ARR_MEM_ALLOCATOR_ITER_print(
    SOC_SAND_ARR_MEM_ALLOCATOR_ITER *info
  );

void
  soc_sand_SAND_ARR_MEM_ALLOCATOR_REQ_BLOCKS_print(
    SOC_SAND_IN SOC_SAND_ARR_MEM_ALLOCATOR_REQ_BLOCKS *info
  );

void
  soc_sand_SAND_ARR_MEM_ALLOCATOR_MEM_STATUS_print(
    SOC_SAND_IN SOC_SAND_ARR_MEM_ALLOCATOR_MEM_STATUS *info
  );

void
  soc_sand_SAND_ARR_MEM_ALLOCATOR_DEFRAG_INFO_print(
    SOC_SAND_IN SOC_SAND_ARR_MEM_ALLOCATOR_DEFRAG_INFO *info
  );
#endif /* SOC_SAND_DEBUG */

/* } */

#include <soc/dpp/SAND/Utils/sand_footer.h>

/* } __SOC_SAND_ARR_MEM_ALLOCATOR_INCLUDED__*/
#endif
