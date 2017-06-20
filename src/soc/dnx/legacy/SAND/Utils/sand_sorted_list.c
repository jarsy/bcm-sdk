/* $Id: sand_sorted_list.c,v 1.8 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/

#ifdef _ERR_MSG_MODULE_NAME
  #error "_ERR_MSG_MODULE_NAME redefined"
#endif

#define _ERR_MSG_MODULE_NAME BSL_SOC_COMMON

/*************
* INCLUDES  *
*************/
/* { */


#include <shared/bsl.h>
#include <soc/dnx/legacy/drv.h>


#include <soc/dnx/legacy/SAND/Utils/sand_sorted_list.h>
#include <shared/swstate/access/sw_state_access.h>


/* } */

extern shr_sw_state_t *sw_state[BCM_MAX_NUM_UNITS];

/*************
* DEFINES   *
*************/
/* { */

/* $Id: sand_sorted_list.c,v 1.8 Broadcom SDK $
 */
/*
 * Get place of the head of the list into variable 'head_place'.
 * Get place of the tail of the list into variable 'tail_place'.
 * Caller is assumed to have declared the following:
 *   variable 'unit'
 *   variable 'res'
 *   address 'exit'
 * If operation fails then software goes to 'exit' with error index 'err1'.
 */
#define DNX_SAND_SORTED_LIST_HEAD_PLACE_GET(sorted_list_index, head_place, _err1) \
  res = SORTED_LIST_ACCESS_INFO.list_size.get(unit,sorted_list_index,&head_place) ; \
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, _err1, exit) ;
#define DNX_SAND_SORTED_LIST_TAIL_PLACE_GET(sorted_list_index, tail_place, _err1) \
  res = SORTED_LIST_ACCESS_INFO.list_size.get(unit,sorted_list_index,&tail_place) ; \
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, _err1, exit) ; \
  tail_place += 1 ;


/* } */

/*************
*  MACROS   *
*************/
/* { */
#define SORTED_LIST_ACCESS          sw_state_access[unit].dnx.soc.sand.sorted_list  
#define SORTED_LIST_ACCESS_DATA     SORTED_LIST_ACCESS.lists_array.list_data
#define SORTED_LIST_ACCESS_INFO     SORTED_LIST_ACCESS.lists_array.init_info

/*
 * Verify specific sorted list index is marked as 'occupied'. If not, software goes to
 * exit with error code.
 * 
 * Notes:
 *   'unit' is assumed to be defined in the caller's scope.
 *   'res' is assumed to be defined in the caller's scope.
 *   'exit' is assumed to be defined in the caller's scope.
 */
#define DNX_SAND_SORTED_LIST_VERIFY_SORTEDLIST_IS_ACTIVE(_sorted_list_index,_err1,_err2) \
  { \
    uint8 bit_val ; \
    uint32 max_nof_lists ; \
    res = SORTED_LIST_ACCESS.max_nof_lists.get(unit, &max_nof_lists) ; \
    DNX_SAND_SOC_CHECK_FUNC_RESULT(res, _err1, exit) ; \
    if (_sorted_list_index >= max_nof_lists) \
    { \
      /* \
       * If sortedlist handle is out of range then quit with error. \
       */ \
      bit_val = 0 ; \
    } \
    else \
    { \
      res = SORTED_LIST_ACCESS.occupied_lists.bit_get(unit, (int)_sorted_list_index, &bit_val) ; \
      DNX_SAND_SOC_CHECK_FUNC_RESULT(res, _err1, exit) ; \
    } \
    if (bit_val == 0) \
    { \
      /* \
       * If sortedlist structure is not indicated as 'occupied' then quit \
       * with error. \
       */ \
      DNX_SAND_SET_ERROR_CODE(DNX_SAND_FREE_FAIL, _err2, exit) ; \
    } \
  }
/*
 * Verify specified unit has a legal value. If not, software goes to
 * exit with error code.
 * 
 * Notes:
 *   'exit' is assumed to be defined in the caller's scope.
 */
#define DNX_SAND_SORTED_LIST_VERIFY_UNIT_IS_LEGAL(unit, _err1) \
  if (((int)unit < 0) || ((int)unit >= SOC_MAX_NUM_DEVICES)) \
  { \
    /* \
     * If this is an illegal unit identifier, quit \
     * with error. \
     */ \
    DNX_SAND_SET_ERROR_CODE(DNX_SAND_MAX_NUM_DEVICES_OUT_OF_RANGE_ERR, _err1, exit); \
  }
/*
 * Convert input sorted list handle to index in 'occupied_lists' array.
 * Convert input index in 'occupied_lists' array to sorted list handle.
 * Indices go from 0 -> (occupied_lists - 1)
 * Handles go from 1 -> occupied_lists
 */
#define DNX_SAND_SORTED_LIST_CONVERT_HANDLE_TO_SORTEDLIST_INDEX(_sorted_list_index,_handle) (_sorted_list_index = _handle - 1)
#define DNX_SAND_SORTED_LIST_CONVERT_SORTEDLIST_INDEX_TO_HANDLE(_handle,_sorted_list_index) (_handle = _sorted_list_index + 1)
/* } */

/*************
* TYPE DEFS *
*************/
/* { */
/*
 * the key and data type, used for malloc.
 */
typedef uint8 DNX_SAND_SORTED_LIST_KEY_TYPE;
typedef uint8 DNX_SAND_SORTED_LIST_DATA_TYPE;


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
/*  Internal functions                                                  */
/************************************************************************/
/* { */

/*********************************************************************
* NAME:
*   dnx_sand_sorted_list_get_tmp_data_ptr_from_handle
* TYPE:
*   PROC
* DATE:
*   May 18 2015
* FUNCTION:
*   Get value of 'tmp_data' pointer (See DNX_SAND_SORTED_LIST_T)
*   from handle.
* INPUT:
*   DNX_SAND_IN  int                             unit -
*     Identifier of the device to access.
*   DNX_SAND_IN  DNX_SAND_SORTED_LIST_PTR        sorted_list -
*     Handle to the sorted list.
*   DNX_SAND_OUT uint8                           **tmp_data_ptr_ptr -
*     This procedure loads pointed memory by the pointer to the 'tmp_data'
*     internal workspace buffer.
* REMARKS:
*   This procedure is exceptional. It is added here so we can use
*   the buffer pointed by 'tmp_data' as a work space whose address
*   is passed to variuos utilities.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
static uint32
  dnx_sand_sorted_list_get_tmp_data_ptr_from_handle(
    DNX_SAND_IN  int                          unit,
    DNX_SAND_IN  DNX_SAND_SORTED_LIST_PTR     sorted_list,
    DNX_SAND_OUT uint8                        **tmp_data_ptr_ptr
  )
{
  uint32
    sorted_list_index,
    res;

  DNX_SAND_INIT_ERROR_DEFINITIONS(0);
  DNX_SAND_SORTED_LIST_VERIFY_UNIT_IS_LEGAL(unit, 2) ;
  DNX_SAND_SORTED_LIST_CONVERT_HANDLE_TO_SORTEDLIST_INDEX(sorted_list_index,sorted_list) ;
  DNX_SAND_SORTED_LIST_VERIFY_SORTEDLIST_IS_ACTIVE(sorted_list_index,4,6) ;

  *tmp_data_ptr_ptr = sw_state[unit]->dnx.soc.sand.sorted_list->lists_array[sorted_list_index]->list_data.tmp_data ;
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in dnx_sand_sorted_list_get_tmp_data_ptr_from_handle()",0,0);
}

/*********************************************************************
* NAME:
*   dnx_sand_sorted_list_get_tmp_key_ptr_from_handle
* TYPE:
*   PROC
* DATE:
*   May 18 2015
* FUNCTION:
*   Get value of 'tmp_key' pointer (See DNX_SAND_SORTED_LIST_T)
*   from handle.
* INPUT:
*   DNX_SAND_IN  int                             unit -
*     Identifier of the device to access.
*   DNX_SAND_IN  DNX_SAND_SORTED_LIST_PTR        sorted_list -
*     Handle to the sorted list.
*   DNX_SAND_OUT uint8                           **tmp_key_ptr_ptr -
*     This procedure loads pointed memory by the pointer to the 'tmp_key'
*     internal workspace buffer.
* REMARKS:
*   This procedure is exceptional. It is added here so we can use
*   the buffer pointed by 'tmp_key' as a work space whose address
*   is passed to variuos utilities.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
static uint32
  dnx_sand_sorted_list_get_tmp_key_ptr_from_handle(
    DNX_SAND_IN  int                          unit,
    DNX_SAND_IN  DNX_SAND_SORTED_LIST_PTR     sorted_list,
    DNX_SAND_OUT uint8                        **tmp_key_ptr_ptr
  )
{
  uint32
    sorted_list_index,
    res;

  DNX_SAND_INIT_ERROR_DEFINITIONS(0);
  DNX_SAND_SORTED_LIST_VERIFY_UNIT_IS_LEGAL(unit, 2) ;
  DNX_SAND_SORTED_LIST_CONVERT_HANDLE_TO_SORTEDLIST_INDEX(sorted_list_index,sorted_list) ;
  DNX_SAND_SORTED_LIST_VERIFY_SORTEDLIST_IS_ACTIVE(sorted_list_index,4,6) ;

  *tmp_key_ptr_ptr = sw_state[unit]->dnx.soc.sand.sorted_list->lists_array[sorted_list_index]->list_data.tmp_key ;
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in dnx_sand_sorted_list_get_tmp_key_ptr_from_handle()",0,0);
}

/*********************************************************************
* NAME:
*     dnx_sand_sorted_list_find_match_entry
* TYPE:
*   PROC
* DATE:
*   May 19 2015
* FUNCTION:
*  lookup in the sorted list for the given key and return the data inserted with
*  the given key.
* INPUT:
*   DNX_SAND_IN  int                             unit -
*     Identifier of the device to access.
*   DNX_SAND_IN  DNX_SAND_SORTED_LIST_PTR        sorted_list -
*     Handle to the sorted list.
*   DNX_SAND_IN  DNX_SAND_SORTED_LIST_KEY*       const key -
*     The key to lookup for
*   DNX_SAND_IN  uint8                    first_empty
*     whether to return the first empty entry .
*   DNX_SAND_OUT  uint32                  *entry -
*     if the key is present in the sorted list then return the entry the key found at,
*     otherwise it returns the place where the key suppose to be.
*   DNX_SAND_IN  uint8                    *found -
*     whether the key was found in the sorted list
*   DNX_SAND_OUT  uint32                  *index_in_indices -
*     Only meaningful if the 'indices' feature is enabled ('indices' element
*     in DNX_SAND_SORTED_LIST_T is allocated).
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/

static uint32
  dnx_sand_sorted_list_find_match_entry(
    DNX_SAND_IN int                             unit,
    DNX_SAND_IN DNX_SAND_SORTED_LIST_PTR        sorted_list,
    DNX_SAND_IN DNX_SAND_SORTED_LIST_KEY*       const key,
    DNX_SAND_IN DNX_SAND_SORTED_LIST_DATA*      const data,
    DNX_SAND_IN uint8                           first_match,
    DNX_SAND_OUT  uint8                         *found,
    DNX_SAND_OUT  DNX_SAND_SORTED_LIST_ITER     *prev_node,
    DNX_SAND_OUT  DNX_SAND_SORTED_LIST_ITER     *cur_node,
    DNX_SAND_OUT  uint32                        *index_in_indices
  );

static uint32
  dnx_sand_sorted_list_node_alloc(
    DNX_SAND_IN   uint32                       unit,
    DNX_SAND_IN  DNX_SAND_SORTED_LIST_PTR      sorted_list,
    DNX_SAND_IN  DNX_SAND_SORTED_LIST_KEY      *const key,
    DNX_SAND_IN   DNX_SAND_SORTED_LIST_DATA    *const data,
    DNX_SAND_IN   uint32                       prev_node,
    DNX_SAND_IN   uint32                       next_node,
    DNX_SAND_IN   uint32                       index_in_indices,
    DNX_SAND_OUT  uint8                        *found
  );

/*********************************************************************
* NAME:
*     dnx_sand_sorted_list_get_next_aux
* TYPE:
*   PROC
* DATE:
*   May 19 2015
* FUNCTION:
*  get the next valid entry (key and data) in the sorted list.
*  start traversing from the place pointed by the given iterator.
* INPUT:
*   DNX_SAND_IN  int                             unit -
*     Identifier of the device to access.
*   DNX_SAND_IN  DNX_SAND_SORTED_LIST_PTR        sorted_list -
*     Handle to the sorted list.
*   DNX_SAND_INOUT  DNX_SAND_SORTED_LIST_ITER    *iter
*     iterator points to the entry to start traverse from.
*   DNX_SAND_OUT  DNX_SAND_SORTED_LIST_KEY       *const key -
*     the sorted list key returned
*   DNX_SAND_OUT  DNX_SAND_SORTED_LIST_DATA      data -
*     the sorted list data returned and associated with the key above.
*   DNX_SAND_INOUT DNX_SAND_SORTED_LIST_ITER     *next_or_prev -
*     This procedure loads pointed memory by the next iterator if
*     'forward' is true or by the previous iterator if 'forward' is
*     false.
* REMARKS:
*     - to start traverse the sorted list from the beginning.
*       use DNX_SAND_SORTED_LIST_ITER_BEGIN(iter)
*     - to check if the iterator get to the end of the table use.
*       use DNX_SAND_SORTED_LIST_ITER_END(unit,iter)
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
static uint32
  dnx_sand_sorted_list_get_next_aux(
    DNX_SAND_IN    int                           unit,
    DNX_SAND_IN    DNX_SAND_SORTED_LIST_PTR      sorted_list,
    DNX_SAND_IN    DNX_SAND_SORTED_LIST_ITER     curr,
    DNX_SAND_IN    uint8                         forward,
    DNX_SAND_OUT   DNX_SAND_SORTED_LIST_KEY      *const key,
    DNX_SAND_OUT   DNX_SAND_SORTED_LIST_DATA     *const data,
    DNX_SAND_INOUT DNX_SAND_SORTED_LIST_ITER     *next_or_prev
  );

static uint32
  dnx_sand_sorted_list_node_link_set(
    DNX_SAND_IN     int                   unit,
    DNX_SAND_IN  DNX_SAND_SORTED_LIST_PTR sorted_list,
    DNX_SAND_IN  uint32                   node1,
    DNX_SAND_IN  uint32                   node2
  );


uint32
    dnx_sand_sorted_list_default_entry_set(
      DNX_SAND_IN    int prime_handle,
      DNX_SAND_IN    uint32 sec_handle,
      DNX_SAND_INOUT uint8  *buffer,
      DNX_SAND_IN    uint32 offset,
      DNX_SAND_IN    uint32 len,
      DNX_SAND_IN    uint8  *data
    )
{
  sal_memcpy(
    buffer + (offset * len),
    data,
    len
  );
  return DNX_SAND_OK;
}


uint32
    dnx_sand_sorted_list_default_entry_get(
      DNX_SAND_IN  int prime_handle,
      DNX_SAND_IN  uint32 sec_handle,
      DNX_SAND_IN  uint8  *buffer,
      DNX_SAND_IN  uint32 offset,
      DNX_SAND_IN  uint32 len,
      DNX_SAND_OUT uint8  *data
    )
{
  sal_memcpy(
    data,
    buffer + (offset * len),
    len
  );
  return DNX_SAND_OK;
}


/************************************************************************/
/*  End of internals                                                    */
/************************************************************************/
/* } */

/*********************************************************************
* NAME:
*   dnx_sand_sorted_list_init
* TYPE:
*   PROC
* DATE:
*   May 19 2015
* FUNCTION:
*   Initialize control structure for ALL sorted list instances expected.
* INPUT:
*   DNX_SAND_IN  int unit -
*     Identifier of the device to access.
*   DNX_SAND_IN  uint32 max_nof_lists -
*     Maximal number of sorted lists which can be sustained simultaneously.
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  dnx_sand_sorted_list_init(
    DNX_SAND_IN       int                          unit,
    DNX_SAND_IN       uint32                       max_nof_lists
  )
{
  uint32 res ;

  DNX_SAND_INIT_ERROR_DEFINITIONS(DNX_SAND_SORTED_LIST_INIT) ;

  res = SORTED_LIST_ACCESS.alloc(unit);
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 10, exit);

  res = SORTED_LIST_ACCESS.lists_array.ptr_alloc(unit, max_nof_lists);
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 20, exit);

  res = SORTED_LIST_ACCESS.max_nof_lists.set(unit, max_nof_lists);
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 30, exit);

  res = SORTED_LIST_ACCESS.in_use.set(unit, 0);
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 40, exit);

  res = SORTED_LIST_ACCESS.occupied_lists.alloc_bitmap(unit, max_nof_lists);
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 60, exit);

  exit:
    DNX_SAND_EXIT_AND_SEND_ERROR( "error in dnx_sand_sorted_list_init()",0,0);
}

/*********************************************************************
* NAME:
*   dnx_sand_sorted_list_clear_all_tmps
* TYPE:
*   PROC
* DATE:
*   Aug 02 2015
* FUNCTION:
*   Fill all allocated 'tmp' (sand box) buffers by zeros.
* INPUT:
*   DNX_SAND_IN  int unit -
*     Identifier of the device to access.
* REMARKS:
*   This procedure is to be used at init before 'diff'ing previous sw
*   state buffer with current one. This ensures that such buffers are
*   effectively not 'diff'ed.
* RETURNS:
*   OK or ERROR indication.
*********************************************************************/
uint32
  dnx_sand_sorted_list_clear_all_tmps(
    DNX_SAND_IN int unit
  )
{
  uint32
    sorted_list_index ;
  uint32
    found,
    res,
    max_nof_lists,
    tmp_size,
    key_size,
    data_size,
    ptr_size,
    in_use ;
  uint8
    bit_val ;
  uint8
    is_allocated ;
  int32
    offset ;

  DNX_SAND_INIT_ERROR_DEFINITIONS(0);

  DNX_SAND_SORTED_LIST_VERIFY_UNIT_IS_LEGAL(unit, 2) ;

  res = SORTED_LIST_ACCESS.in_use.get(unit, &in_use);
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 4, exit);

  res = SORTED_LIST_ACCESS.max_nof_lists.get(unit, &max_nof_lists);
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 6, exit);

  if (in_use >= max_nof_lists)
  {
    /*
     * If number of occupied bitmap structures is beyond the
     * maximum then quit with error.
     */
    DNX_SAND_SET_ERROR_CODE(DNX_SAND_MALLOC_FAIL, 8, exit);
  }
  /*
   * Find occupied lists (a set bit in 'occupied_lists') and, for each,
   * fill 'tmp' buffers by zeroes.
   *
   * Currently, 'tmp' buffers are:
   *   sw_state[unit]->dnx.soc.sand.sorted_list->lists_array[sorted_list_index]->list_data.tmp_key
   *   sw_state[unit]->dnx.soc.sand.sorted_list->lists_array[sorted_list_index]->list_data.tmp_data
   */
  found = 0 ;
  offset = 0 ;
  for (sorted_list_index = 0 ; sorted_list_index < max_nof_lists ; sorted_list_index++)
  {
    res = SORTED_LIST_ACCESS.occupied_lists.bit_get(unit, sorted_list_index, &bit_val);
    DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 12, exit);
    if (bit_val == 1)
    {
      /*
       * 'sorted_list_index' is now the index of an occupied entry.
       */
      found++ ;
      res = SORTED_LIST_ACCESS_DATA.tmp_key.is_allocated(unit,sorted_list_index,&is_allocated) ;
      DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 14, exit) ;
      if (!is_allocated)
      {
        /*
         * 'tmp_key' buffer must be allocated, at this point.
         */
        DNX_SAND_SET_ERROR_CODE(SOC_E_INTERNAL, 16, exit);
      }
      res = SORTED_LIST_ACCESS_DATA.tmp_data.is_allocated(unit,sorted_list_index,&is_allocated) ;
      DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 24, exit) ;
      if (!is_allocated)
      {
        /*
         * 'tmp_data' buffer must be allocated, at this point.
         */
        DNX_SAND_SET_ERROR_CODE(SOC_E_INTERNAL, 18, exit);
      }
      /*
       * Clear 'tmp_key'
       */
      res = SORTED_LIST_ACCESS_INFO.key_size.get(unit,sorted_list_index,&key_size) ;
      DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 20, exit);
      res = SORTED_LIST_ACCESS_DATA.tmp_key.memset(unit,sorted_list_index,offset,key_size,0) ;
      DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 22, exit) ;
      /*
       * Clear 'tmp_data'
       */
      res = SORTED_LIST_ACCESS_INFO.data_size.get(unit,sorted_list_index,&data_size) ;
      DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 24, exit);
      res = SORTED_LIST_ACCESS_DATA.ptr_size.get(unit,sorted_list_index,&ptr_size) ;
      DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 26, exit) ;
      tmp_size = DNX_SAND_MAX(ptr_size,data_size) ;
      res = SORTED_LIST_ACCESS_DATA.tmp_data.memset(unit,sorted_list_index,offset,tmp_size,0) ;
      DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 28, exit) ;
      if (found >= in_use)
      {
        /*
         * If all allocated entries have been treated. Quit.
         */
        break ;
      }
    }
  }

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in dnx_sand_sorted_list_clear_all_tmps()",0,0);
}

uint32
  dnx_sand_sorted_list_create(
    DNX_SAND_IN     int                             unit,
    DNX_SAND_INOUT  DNX_SAND_SORTED_LIST_PTR        *sorted_list_ptr,
    DNX_SAND_IN     DNX_SAND_SORTED_LIST_INIT_INFO  init_info
  )
{
  DNX_SAND_OCC_BM_INIT_INFO
    btmp_init_info ;
  uint32
    tmp_size ;
  uint32
    sorted_list_index ;
  uint32
    found,
    res,
    data_size,
    list_size,
    key_size,
    null_ptr,
    ptr_size,
    max_nof_lists ;
  uint32
    in_use ;
  uint8
    bit_val ;
  DNX_SAND_SORTED_LIST_PTR
    sorted_list ;
  DNX_SAND_OCC_BM_PTR
    memory_use ;
  int32
    offset ;

  DNX_SAND_INIT_ERROR_DEFINITIONS(DNX_SAND_SORTED_LIST_CREATE) ;

  DNX_SAND_SORTED_LIST_VERIFY_UNIT_IS_LEGAL(unit, 2) ;
  DNX_SAND_CHECK_NULL_INPUT(sorted_list_ptr);

  res = SORTED_LIST_ACCESS.in_use.get(unit, &in_use);
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 4, exit);

  res = SORTED_LIST_ACCESS.max_nof_lists.get(unit, &max_nof_lists);
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 6, exit);

  if (in_use >= max_nof_lists)
  {
    /*
     * If number of occupied bitmap structures is beyond the
     * maximum then quit with error.
     */
    DNX_SAND_SET_ERROR_CODE(DNX_SAND_MALLOC_FAIL, 8, exit);
  }
  /*
   * Increment number of 'in_use' to cover the one we now intend to capture.
   */
  res = SORTED_LIST_ACCESS.in_use.set(unit, (in_use + 1));
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 10, exit);
  /*
   * Find a free list (a cleared bit in 'occupied_lists'). At this point,
   * there must be one.
   */
  found = 0 ;
  for (sorted_list_index = 0 ; sorted_list_index < max_nof_lists ; sorted_list_index++)
  {
    res = SORTED_LIST_ACCESS.occupied_lists.bit_get(unit, sorted_list_index, &bit_val);
    DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 12, exit);
    if (bit_val == 0)
    {
      /*
       * 'sorted_list_index' is now the index of a free entry.
       */
      found = 1 ;
      break ;
    }
  }
  if (!found)
  {
    DNX_SAND_SET_ERROR_CODE(DNX_SAND_MALLOC_FAIL, 14, exit);
  }
  res = SORTED_LIST_ACCESS.occupied_lists.bit_set(unit, sorted_list_index);
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 16, exit);

  res = SORTED_LIST_ACCESS.lists_array.alloc(unit, sorted_list_index);
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 18, exit);
  /*
   * Note that legal handles start at '1', not at '0'.
   */
  DNX_SAND_SORTED_LIST_CONVERT_SORTEDLIST_INDEX_TO_HANDLE(sorted_list,sorted_list_index) ;
  /*
   * Set output of this procedure.
   */
  *sorted_list_ptr = sorted_list;

  dnx_sand_SAND_SORTED_LIST_INFO_clear(unit,sorted_list) ;
  res = SORTED_LIST_ACCESS_INFO.prime_handle.set(unit, sorted_list_index, init_info.prime_handle) ;
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 22, exit);

  res = SORTED_LIST_ACCESS_INFO.sec_handle.set(unit,sorted_list_index,init_info.sec_handle) ;
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 24, exit);
  res = SORTED_LIST_ACCESS_INFO.list_size.set(unit,sorted_list_index,init_info.list_size) ;
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 26, exit);
  res = SORTED_LIST_ACCESS_INFO.key_size.set(unit,sorted_list_index,init_info.key_size) ;
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 28, exit);
  res = SORTED_LIST_ACCESS_INFO.data_size.set(unit,sorted_list_index,init_info.data_size) ;
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 30, exit);
  if (init_info.get_entry_fun != NULL || init_info.set_entry_fun != NULL )
  {
    DNX_SAND_SET_ERROR_CODE(DNX_SAND_VALUE_OUT_OF_RANGE_ERR, 31, exit);
  }
  res = SORTED_LIST_ACCESS_INFO.get_entry_fun.set(unit,sorted_list_index,init_info.get_entry_fun) ;
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 32, exit);
  res = SORTED_LIST_ACCESS_INFO.set_entry_fun.set(unit,sorted_list_index,init_info.set_entry_fun) ;
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 34, exit);
  res = SORTED_LIST_ACCESS_INFO.cmp_func_type.set(unit,sorted_list_index,init_info.cmp_func_type) ;
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 35, exit);

  res = SORTED_LIST_ACCESS_INFO.key_size.get(unit,sorted_list_index,&key_size) ;
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 36, exit);
  res = SORTED_LIST_ACCESS_INFO.list_size.get(unit,sorted_list_index,&list_size) ;
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 38, exit);
  res = SORTED_LIST_ACCESS_INFO.data_size.get(unit,sorted_list_index,&data_size) ;
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 40, exit);

  if (list_size == 0 || key_size == 0 || data_size == 0)
  {
    DNX_SAND_SET_ERROR_CODE(DNX_SAND_VALUE_OUT_OF_RANGE_ERR, 42, exit);
  }
  /*
   * calculate the size of pointers (list head and next) according to table size.
   */
  ptr_size = (dnx_sand_log2_round_up(list_size + 2) + (DNX_SAND_NOF_BITS_IN_BYTE - 1)) / DNX_SAND_NOF_BITS_IN_BYTE;
  null_ptr = list_size + 1;
  res = SORTED_LIST_ACCESS_DATA.ptr_size.set(unit,sorted_list_index,ptr_size) ;
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 44, exit) ;
  res = SORTED_LIST_ACCESS_DATA.null_ptr.set(unit,sorted_list_index,null_ptr) ;
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 46, exit) ;

  tmp_size = DNX_SAND_MAX(ptr_size,data_size) ;
  /*
   * Allocate the temps buffer.
   */
  res = SORTED_LIST_ACCESS_DATA.tmp_data.alloc(unit,sorted_list_index,tmp_size) ;
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 48, exit) ;
  res = SORTED_LIST_ACCESS_DATA.tmp_key.alloc(unit,sorted_list_index,key_size) ;
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 50, exit) ;
  /*
   * allocate buffer for keys
   */
  offset = 0 ;
  res = SORTED_LIST_ACCESS_DATA.keys.alloc(unit,sorted_list_index,list_size * key_size) ;
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 52, exit) ;
  res = SORTED_LIST_ACCESS_DATA.keys.memset(unit,sorted_list_index,offset,list_size * key_size,0) ;
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 54, exit) ;
  /*
   * allocate buffer for next array (to build the linked list) one additional for head.
   * which is the last in the next pointers array.
   */
  res = SORTED_LIST_ACCESS_DATA.next.alloc(unit,sorted_list_index,(list_size + 2) * ptr_size) ;
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 56, exit) ;
  res = SORTED_LIST_ACCESS_DATA.next.memset(unit,sorted_list_index,offset,(list_size + 2) * ptr_size,0xFF) ;
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 58, exit) ;
  /*
   * allocate buffer for prev array (to build the linked list)
   */
  res = SORTED_LIST_ACCESS_DATA.prev.alloc(unit,sorted_list_index,(list_size + 2) * ptr_size) ;
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 56, exit) ;
  res = SORTED_LIST_ACCESS_DATA.prev.memset(unit,sorted_list_index,offset,(list_size + 2) * ptr_size,0xFF) ;
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 58, exit) ;
  /*
   * allocate buffer for the data array
   */
  res = SORTED_LIST_ACCESS_DATA.data.alloc(unit,sorted_list_index,list_size * data_size) ;
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 60, exit) ;
  res = SORTED_LIST_ACCESS_DATA.data.memset(unit,sorted_list_index,offset,list_size * data_size,0) ;
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 62, exit) ;
  {
    uint32
      head_place,
      tail_place ;
    /*
     * connect the head with the tail.
     */
    DNX_SAND_SORTED_LIST_HEAD_PLACE_GET(sorted_list_index,head_place,64) ;
    DNX_SAND_SORTED_LIST_TAIL_PLACE_GET(sorted_list_index,tail_place,66) ;
    res =
      dnx_sand_sorted_list_node_link_set(
        unit,
        sorted_list,
        head_place,
        tail_place
      );
    DNX_SAND_CHECK_FUNC_RESULT(res, 70, exit);
  }
  /*
   * initialize the data to be mapped to
   */
  dnx_sand_SAND_OCC_BM_INIT_INFO_clear(&btmp_init_info);
  btmp_init_info.size = list_size ;

  /* if(SOC_DNX_WB_ENGINE_VAR_NONE != init_info_ptr->wb_var_index) */

  res = dnx_sand_occ_bm_create(
          unit,
          &btmp_init_info,
          &memory_use
        );
  DNX_SAND_CHECK_FUNC_RESULT(res, 80, exit);
  res = SORTED_LIST_ACCESS_DATA.memory_use.set(unit,sorted_list_index,memory_use) ;
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 82, exit) ;
#if ENHANCED_SORTED_LIST_SEARCH
/* { */
  {
    /*
     * Initialize 'indices' array. Currently, we add this feature to all sorted lists!
     */
    res = SORTED_LIST_ACCESS_DATA.indices.alloc(unit,sorted_list_index,list_size) ;
    DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 84, exit) ;
    res = SORTED_LIST_ACCESS_DATA.num_elements_on_indices.set(unit,sorted_list_index,0) ;
    DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 88, exit) ;
  }
/* } */
#endif
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in dnx_sand_sorted_list_create()",0,0);
}

uint32
  dnx_sand_sorted_list_clear(
    DNX_SAND_IN     int                           unit,
    DNX_SAND_IN     DNX_SAND_SORTED_LIST_PTR      sorted_list
  )
{
  uint32
    res;
  uint32
    sorted_list_index ;
  uint32
    data_size,
    list_size,
    key_size,
    ptr_size ;
  DNX_SAND_OCC_BM_PTR
    memory_use ;
  int32
    offset ;

  DNX_SAND_INIT_ERROR_DEFINITIONS(DNX_SAND_HASH_TABLE_CREATE);
  DNX_SAND_SORTED_LIST_VERIFY_UNIT_IS_LEGAL(unit, 2) ;
  DNX_SAND_SORTED_LIST_CONVERT_HANDLE_TO_SORTEDLIST_INDEX(sorted_list_index,sorted_list) ;
  DNX_SAND_SORTED_LIST_VERIFY_SORTEDLIST_IS_ACTIVE(sorted_list_index,4,6) ;

  /* if(SOC_DNX_WB_ENGINE_VAR_NONE == init_info_ptr->wb_var_index)  */
  {
    offset = 0 ;
    res = SORTED_LIST_ACCESS_DATA.ptr_size.get(unit,sorted_list_index,&ptr_size) ;
    DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 12, exit) ;
    res = SORTED_LIST_ACCESS_INFO.key_size.get(unit,sorted_list_index,&key_size) ;
    DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 14, exit);
    res = SORTED_LIST_ACCESS_INFO.list_size.get(unit,sorted_list_index,&list_size) ;
    DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 16, exit);
    res = SORTED_LIST_ACCESS_INFO.data_size.get(unit,sorted_list_index,&data_size) ;
    DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 18, exit);

    res = SORTED_LIST_ACCESS_DATA.keys.memset(unit,sorted_list_index,offset,list_size * key_size,0x00) ;
    DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 20, exit) ;

    res = SORTED_LIST_ACCESS_DATA.next.memset(unit,sorted_list_index,offset,(list_size + 2) * ptr_size,0xFF) ;
    DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 22, exit) ;

    res = SORTED_LIST_ACCESS_DATA.prev.memset(unit,sorted_list_index,offset,(list_size + 2) * ptr_size,0xFF) ;
    DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 24, exit) ;

    res = SORTED_LIST_ACCESS_DATA.data.memset(unit,sorted_list_index,offset,list_size * data_size,0x00) ;
    DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 26, exit) ;
  } 
  { 
    /*
     * connect the head with the tail.
     */
    uint32
      head_place,
      tail_place ;
    DNX_SAND_SORTED_LIST_HEAD_PLACE_GET(sorted_list_index,head_place,54) ;
    DNX_SAND_SORTED_LIST_TAIL_PLACE_GET(sorted_list_index,tail_place,56) ;
    res = dnx_sand_sorted_list_node_link_set(
          unit,
          sorted_list,
          head_place,
          tail_place
        );
    DNX_SAND_CHECK_FUNC_RESULT(res, 30, exit) ;
  }

  res = SORTED_LIST_ACCESS_DATA.memory_use.get(unit,sorted_list_index,&memory_use) ;
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 32, exit) ;
  res = dnx_sand_occ_bm_clear(
          unit,
          memory_use
        );
  DNX_SAND_CHECK_FUNC_RESULT(res, 36, exit);
  {
    /*
     * Initialize 'indices' array. Currently, we add this feature to all sorted lists!
     */
    res = SORTED_LIST_ACCESS_DATA.num_elements_on_indices.set(unit,sorted_list_index,0) ;
    DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 40, exit) ;
  }

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in dnx_sand_sorted_list_clear()",0,0);
}

uint32
  dnx_sand_sorted_list_destroy(
    DNX_SAND_IN     int                         unit,
    DNX_SAND_IN     DNX_SAND_SORTED_LIST_PTR    sorted_list
    )
{
  uint32
    res,
    sorted_list_index ;
  DNX_SAND_OCC_BM_PTR
    memory_use ;
  uint8
    bit_val ;
  uint32
    in_use ;
  uint8
    is_allocated ;

  DNX_SAND_INIT_ERROR_DEFINITIONS(DNX_SAND_SORTED_LIST_DESTROY);
  DNX_SAND_SORTED_LIST_VERIFY_UNIT_IS_LEGAL(unit, 2) ;
  DNX_SAND_SORTED_LIST_CONVERT_HANDLE_TO_SORTEDLIST_INDEX(sorted_list_index,sorted_list) ;
  DNX_SAND_SORTED_LIST_VERIFY_SORTEDLIST_IS_ACTIVE(sorted_list_index,4,6) ;
  /*
   * First, mark this sorted list as 'released'
   */
  res = SORTED_LIST_ACCESS.in_use.get(unit, &in_use);
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 4, exit);
  if ((int)in_use < 0)
  {
    /*
     * If number of occupied sortedlist structures goes below zero then quit
     * with error.
     */
    DNX_SAND_SET_ERROR_CODE(DNX_SAND_FREE_FAIL, 8, exit);
  }
  /*
   * Decrement number of 'in_use' to cover the one we now intend to release.
   */
  res = SORTED_LIST_ACCESS.in_use.set(unit, (in_use - 1));
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 10, exit) ;
  /*
   * Mark specific sorted list as 'not occupied'
   */
  res = SORTED_LIST_ACCESS.occupied_lists.bit_get(unit, sorted_list_index, &bit_val);
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 12, exit);
  if (bit_val == 0)
  {
    /*
     * If sorted list structure is not indicated as 'occupied' then quit
     * with error.
     */
    DNX_SAND_SET_ERROR_CODE(DNX_SAND_FREE_FAIL, 14, exit) ;
  }
  res = SORTED_LIST_ACCESS.occupied_lists.bit_clear(unit, sorted_list_index);
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 14, exit);
  /*
   * Free the temps buffer.
   */
  res = SORTED_LIST_ACCESS_DATA.tmp_data.free(unit,sorted_list_index) ;
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 18, exit) ;
  res = SORTED_LIST_ACCESS_DATA.tmp_key.free(unit,sorted_list_index) ;
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 20, exit) ;

  res = SORTED_LIST_ACCESS_DATA.next.free(unit,sorted_list_index) ;
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 24, exit) ;
  res = SORTED_LIST_ACCESS_DATA.keys.free(unit,sorted_list_index) ;
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 28, exit) ;

  res = SORTED_LIST_ACCESS_DATA.prev.free(unit,sorted_list_index) ;
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 32, exit) ;
  res = SORTED_LIST_ACCESS_DATA.data.free(unit,sorted_list_index) ;
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 36, exit) ;

  res = SORTED_LIST_ACCESS_DATA.indices.is_allocated(unit,sorted_list_index,&is_allocated) ;
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 38, exit) ;
  if (is_allocated)
  {
    /*
     * Free 'indices' array if it has been allocated
     */
    res = SORTED_LIST_ACCESS_DATA.indices.free(unit,sorted_list_index) ;
    DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 40, exit) ;
    res = SORTED_LIST_ACCESS_DATA.num_elements_on_indices.set(unit,sorted_list_index,0) ;
    DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 42, exit) ;
  }
  res = SORTED_LIST_ACCESS_DATA.memory_use.get(unit,sorted_list_index,&memory_use) ;
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 44, exit) ;
  res = dnx_sand_occ_bm_destroy(
    unit,
    memory_use
    );
  res = SORTED_LIST_ACCESS.lists_array.free(unit, sorted_list_index);
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 48, exit);

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in dnx_sand_sorted_list_destroy()",0,0);
}

uint32
  dnx_sand_sorted_list_entry_add(
    DNX_SAND_IN     int                           unit,
    DNX_SAND_IN     DNX_SAND_SORTED_LIST_PTR      sorted_list,
    DNX_SAND_IN     DNX_SAND_SORTED_LIST_KEY      *const key,
    DNX_SAND_IN     DNX_SAND_SORTED_LIST_DATA     *const data,
    DNX_SAND_OUT    uint8                         *success
  )
{
  uint8
    found ;
  uint32
    curr_node,
    prev_node ;
  uint32
    res ;
  uint32
    from_top_index_in_indices ;
  uint32
    sorted_list_index ;

  DNX_SAND_INIT_ERROR_DEFINITIONS(DNX_SAND_SORTED_LIST_ENTRY_ADD);
  DNX_SAND_SORTED_LIST_VERIFY_UNIT_IS_LEGAL(unit, 2) ;
  DNX_SAND_SORTED_LIST_CONVERT_HANDLE_TO_SORTEDLIST_INDEX(sorted_list_index,sorted_list) ;
  DNX_SAND_SORTED_LIST_VERIFY_SORTEDLIST_IS_ACTIVE(sorted_list_index,4,6) ;

  DNX_SAND_CHECK_NULL_INPUT(key);
  DNX_SAND_CHECK_NULL_INPUT(data);
  DNX_SAND_CHECK_NULL_INPUT(success);

  res = dnx_sand_sorted_list_find_match_entry(
          unit,
          sorted_list,
          key,
          data,
          FALSE,
          &found,
          &prev_node,
          &curr_node,
          &from_top_index_in_indices
        ) ;
  DNX_SAND_CHECK_FUNC_RESULT(res, 10, exit);
  if (found)
  {
    DNX_SAND_SET_ERROR_CODE(DNX_SAND_SORTED_LIST_KEY_DATA_ALREADY_EXIST_ERR, 20, exit);
  }
  /*
   * Allocate new node.
   */
  res = dnx_sand_sorted_list_node_alloc(
          unit,
          sorted_list,
          key,
          data,
          prev_node,
          curr_node,
          from_top_index_in_indices,
          success
        );
  DNX_SAND_CHECK_FUNC_RESULT(res, 30, exit) ;

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in dnx_sand_sorted_list_entry_add()",0,0);
}

uint32
  dnx_sand_sorted_list_entry_add_by_iter(
    DNX_SAND_IN     int                           unit,
    DNX_SAND_IN     DNX_SAND_SORTED_LIST_PTR      sorted_list,
    DNX_SAND_IN     DNX_SAND_SORTED_LIST_ITER     pos,
    DNX_SAND_IN     uint8                         before,
    DNX_SAND_IN     DNX_SAND_SORTED_LIST_KEY      *const key,
    DNX_SAND_IN     DNX_SAND_SORTED_LIST_DATA     *const data,
    DNX_SAND_OUT    uint8                         *success
  )
{
  DNX_SAND_SORTED_LIST_ITER
    prev_node,
    next_node;
  DNX_SAND_SORTED_LIST_KEY_CMP 
    cmp_key_fun = NULL;
  uint32
    key_size,
    res;
  uint32
    sorted_list_index ;
  DNX_SAND_SORTED_LIST_CMP_FUNC_TYPE
    cmp_func_type ;
  /*
   * Initialize var: Just to make 'coverity' happy.
   */
  uint32
    from_top_index_in_indices = 0 ;
  uint8
    *tmp_key_ptr ;
  uint8
    is_allocated ;

  DNX_SAND_INIT_ERROR_DEFINITIONS(DNX_SAND_SORTED_LIST_ENTRY_ADD_BY_ITER);
  DNX_SAND_SORTED_LIST_VERIFY_UNIT_IS_LEGAL(unit, 2) ;
  DNX_SAND_SORTED_LIST_CONVERT_HANDLE_TO_SORTEDLIST_INDEX(sorted_list_index,sorted_list) ;
  DNX_SAND_SORTED_LIST_VERIFY_SORTEDLIST_IS_ACTIVE(sorted_list_index,4,6) ;


  res = SORTED_LIST_ACCESS_INFO.cmp_func_type.get(unit,sorted_list_index,&cmp_func_type) ;
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 8, exit);

  if (cmp_func_type == DNX_SAND_SORTED_LIST_CMP_FUNC_TYPE_TCAM) {
      
      DNXC_LEGACY_FIXME_ASSERT;
  }
  else {
      cmp_key_fun = (DNX_SAND_SORTED_LIST_KEY_CMP)dnx_sand_os_memcmp;
  }
  /*
   * Verify we are not adding after the end or before the beginning.
   */
  if (
      (before &&  pos == DNX_SAND_SORTED_LIST_ITER_BEGIN(unit,sorted_list)) ||
      ((!before && pos == DNX_SAND_SORTED_LIST_ITER_END(unit,sorted_list)))
     )
  {
    DNX_SAND_SET_ERROR_CODE(DNX_SAND_SORTED_LIST_ILLEGAL_ITER_ERR, 10, exit);
  }
 /*
  * Verify that this addition keeps the order.
  */
  prev_node = pos;
  next_node = pos;

  if (before)
  {
    res =
      dnx_sand_sorted_list_get_next_aux(
        unit,
        sorted_list,
        pos,
        FALSE,
        NULL,
        NULL,&prev_node
      );
    DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 10, exit);
  }
  else
  {
    res =
      dnx_sand_sorted_list_get_next_aux(
        unit,
        sorted_list,
        pos,
        TRUE,
        NULL,
        NULL,&next_node
      );
    DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 12, exit);
  }
  res = dnx_sand_sorted_list_get_tmp_key_ptr_from_handle(unit,sorted_list,&tmp_key_ptr) ;
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 14, exit);
  res = SORTED_LIST_ACCESS_INFO.key_size.get(unit,sorted_list_index,&key_size) ;
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 16, exit) ;
  if (prev_node != DNX_SAND_SORTED_LIST_ITER_END(unit,sorted_list) &&  prev_node != DNX_SAND_SORTED_LIST_ITER_BEGIN(unit,sorted_list))
  {
    res = dnx_sand_sorted_list_entry_value(
            unit,
            sorted_list,
            prev_node,
            tmp_key_ptr,
            NULL
          );
    DNX_SAND_CHECK_FUNC_RESULT(res, 18, exit) ;

    if (cmp_key_fun(tmp_key_ptr, key, key_size) > 0 )
    {
      DNX_SAND_SET_ERROR_CODE(DNX_SAND_SORTED_LIST_ILLEGAL_ITER_ERR, 18, exit);
    }
  }
  if (next_node != DNX_SAND_SORTED_LIST_ITER_END(unit,sorted_list) &&  next_node != DNX_SAND_SORTED_LIST_ITER_BEGIN(unit,sorted_list))
  {
    res = dnx_sand_sorted_list_entry_value(
            unit,
            sorted_list,
            next_node,
            tmp_key_ptr,
            NULL
          );
    DNX_SAND_CHECK_FUNC_RESULT(res, 20, exit);

    if (cmp_key_fun(key, tmp_key_ptr, key_size) > 0 )
    {
      DNX_SAND_SET_ERROR_CODE(DNX_SAND_SORTED_LIST_ILLEGAL_ITER_ERR, 22, exit);
    }
  }
  {
    uint32
        num_elements_on_indices ;
    res = SORTED_LIST_ACCESS_DATA.indices.is_allocated(unit,sorted_list_index,&is_allocated) ;
    DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 24, exit) ;
    if (is_allocated)
    {
      /*
       * Feature is enabled. Find location of element on 'indices' array.
       */
      res = SORTED_LIST_ACCESS_DATA.num_elements_on_indices.get(unit,sorted_list_index,&num_elements_on_indices) ;
      DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 28, exit) ;
      if ((next_node == DNX_SAND_SORTED_LIST_ITER_END(unit,sorted_list)) || (next_node == DNX_SAND_SORTED_LIST_ITER_BEGIN(unit,sorted_list)))
      {
        from_top_index_in_indices = 0 ;
      }
      else if (prev_node == DNX_SAND_SORTED_LIST_ITER_END(unit,sorted_list) ||  prev_node == DNX_SAND_SORTED_LIST_ITER_BEGIN(unit,sorted_list))
      {
        from_top_index_in_indices = num_elements_on_indices - 1 ;
      }
      else
      {
        /*
         * Get the  index on 'indices' corresponding to previous element sorted list.
         * Use it to add entry into 'indices'.
         */
        res = dnx_sand_sorted_list_get_index_from_iter(unit,sorted_list,prev_node,&from_top_index_in_indices) ;
        DNX_SAND_CHECK_FUNC_RESULT(res, 32, exit) ;
        from_top_index_in_indices = num_elements_on_indices - 1 - from_top_index_in_indices ;
      }
    }
  }
  res = dnx_sand_sorted_list_node_alloc(
          unit,
          sorted_list,
          key,
          data,
          prev_node,
          next_node,
          from_top_index_in_indices,
          success
        );
  DNX_SAND_CHECK_FUNC_RESULT(res, 24, exit);
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in dnx_sand_sorted_list_entry_add_by_iter()",0,0);
}

uint32
  dnx_sand_sorted_list_entry_update(
    DNX_SAND_IN     int                           unit,
    DNX_SAND_IN     DNX_SAND_SORTED_LIST_PTR      sorted_list,
    DNX_SAND_IN     DNX_SAND_SORTED_LIST_ITER     iter,
    DNX_SAND_IN     DNX_SAND_SORTED_LIST_KEY      *const key,
    DNX_SAND_IN     DNX_SAND_SORTED_LIST_DATA     *const data
  )
{
  uint32
    res ;
  uint32
    sorted_list_index ;
  uint32
    data_size,
    key_size,
    list_size ;

  DNX_SAND_INIT_ERROR_DEFINITIONS(DNX_SAND_SORTED_LIST_ENTRY_UPDATE);
  DNX_SAND_SORTED_LIST_VERIFY_UNIT_IS_LEGAL(unit, 2) ;
  DNX_SAND_SORTED_LIST_CONVERT_HANDLE_TO_SORTEDLIST_INDEX(sorted_list_index,sorted_list) ;
  DNX_SAND_SORTED_LIST_VERIFY_SORTEDLIST_IS_ACTIVE(sorted_list_index,4,6) ;

  DNX_SAND_CHECK_NULL_INPUT(data);
  DNX_SAND_CHECK_NULL_INPUT(key);

  res = SORTED_LIST_ACCESS_INFO.list_size.get(unit,sorted_list_index,&list_size) ;
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 8, exit);
  if (iter == DNX_SAND_SORTED_LIST_NULL || iter >= list_size)
  {
    DNX_SAND_SET_ERROR_CODE(DNX_SAND_VALUE_OUT_OF_RANGE_ERR, 10, exit);
  }
  /*
   * Check to see if the entry exists
   */
  if (data)
  {
    res = SORTED_LIST_ACCESS_INFO.data_size.get(unit,sorted_list_index,&data_size) ;
    DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 18, exit);
    /* if(SOC_DNX_WB_ENGINE_VAR_NONE == init_info_ptr->wb_var_index) */
    {
      /*
       * Copy 'data_size' bytes from input 'data' buffer into 'SORTED_LIST_ACCESS_DATA.data'
       * buffer at offset 'data_size * iter'.
       */
      res = SORTED_LIST_ACCESS_DATA.data.memwrite(unit,sorted_list_index,data,iter * data_size, data_size) ;
      DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 26, exit) ;
    }
  }
  if (key)
  {
    res = SORTED_LIST_ACCESS_INFO.key_size.get(unit,sorted_list_index,&key_size) ;
    DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 28, exit);
    /* if(SOC_DNX_WB_ENGINE_VAR_NONE == init_info_ptr->wb_var_index) */
    {
      /*
       * Copy 'key_size' bytes from input 'key' buffer into 'SORTED_LIST_ACCESS_DATA.keys'
       * buffer at offset 'key_size * iter'.
       */
      res = SORTED_LIST_ACCESS_DATA.keys.memwrite(unit,sorted_list_index,key,iter * key_size, key_size) ;
      DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 26, exit) ;
    }
  }
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in dnx_sand_sorted_list_entry_update()",0,0);
}

uint32
  dnx_sand_sorted_list_entry_get_data(
    DNX_SAND_IN     int                              unit,
    DNX_SAND_IN     DNX_SAND_SORTED_LIST_PTR         sorted_list,
    DNX_SAND_IN     DNX_SAND_SORTED_LIST_ITER        iter,
    DNX_SAND_OUT    DNX_SAND_SORTED_LIST_DATA        *const data
  )
{
  uint32
    res;
  uint32
    sorted_list_index ;
  uint32
    data_size,
    list_size ;

  DNX_SAND_INIT_ERROR_DEFINITIONS(0);
  DNX_SAND_SORTED_LIST_VERIFY_UNIT_IS_LEGAL(unit, 2) ;
  DNX_SAND_SORTED_LIST_CONVERT_HANDLE_TO_SORTEDLIST_INDEX(sorted_list_index,sorted_list) ;
  DNX_SAND_SORTED_LIST_VERIFY_SORTEDLIST_IS_ACTIVE(sorted_list_index,4,6) ;

  DNX_SAND_CHECK_NULL_INPUT(data);

  res = SORTED_LIST_ACCESS_INFO.list_size.get(unit,sorted_list_index,&list_size) ;
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 8, exit);
  if (iter == DNX_SAND_SORTED_LIST_ITER_END(unit,sorted_list) || iter >= list_size)
  {
    DNX_SAND_SET_ERROR_CODE(DNX_SAND_VALUE_OUT_OF_RANGE_ERR, 10, exit);
  }
  res = SORTED_LIST_ACCESS_INFO.data_size.get(unit,sorted_list_index,&data_size) ;
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 18, exit);
  /*
   * Copy 'data_size' bytes from 'SORTED_LIST_ACCESS_DATA.data' buffer at offset
   * 'data_size * iter' into input 'data' buffer.
   */
  res = SORTED_LIST_ACCESS_DATA.data.memread(unit,sorted_list_index,data,iter * data_size, data_size) ;
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 26, exit) ;
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in dnx_sand_sorted_list_entry_get_data()",0,0);
}

uint32
  dnx_sand_sorted_list_entry_remove_by_iter(
    DNX_SAND_IN     int                             unit,
    DNX_SAND_IN     DNX_SAND_SORTED_LIST_PTR        sorted_list,
    DNX_SAND_IN     DNX_SAND_SORTED_LIST_ITER       iter
  )
{
  DNX_SAND_SORTED_LIST_ITER
    prev,
    next;
  uint32
    res;
  uint32
    sorted_list_index,
    null_ptr,
    ptr_size ;
  DNX_SAND_OCC_BM_PTR
    memory_use ;
  uint32
    index_in_indices ;
  uint8
    is_indices_enabled ;

  DNX_SAND_INIT_ERROR_DEFINITIONS(DNX_SAND_SORTED_LIST_ENTRY_REMOVE_BY_ITER);
  DNX_SAND_SORTED_LIST_VERIFY_UNIT_IS_LEGAL(unit, 2) ;
  DNX_SAND_SORTED_LIST_CONVERT_HANDLE_TO_SORTEDLIST_INDEX(sorted_list_index,sorted_list) ;
  DNX_SAND_SORTED_LIST_VERIFY_SORTEDLIST_IS_ACTIVE(sorted_list_index,4,6) ;

  res = SORTED_LIST_ACCESS_DATA.indices.is_allocated(unit,sorted_list_index,&is_indices_enabled) ;
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 8, exit) ;

  if (is_indices_enabled)
  {
    /*
     * Get the corresponding index on 'indices' before removing from sorted list.
     * Use it, at end of procedure, to remove entry from 'indices'.
     */
    res = dnx_sand_sorted_list_get_index_from_iter(unit,sorted_list,iter,&index_in_indices) ;
    DNX_SAND_CHECK_FUNC_RESULT(res, 10, exit) ;
  }
  res = dnx_sand_sorted_list_get_next_aux(
           unit,
           sorted_list,
           iter,
           TRUE,
           NULL,
           NULL,&next
         ) ;
  DNX_SAND_CHECK_FUNC_RESULT(res, 12, exit);
  res = dnx_sand_sorted_list_get_next_aux(
           unit,
           sorted_list,
           iter,
           FALSE,
           NULL,
           NULL,&prev
         ) ;
  DNX_SAND_CHECK_FUNC_RESULT(res, 14, exit);
  res = dnx_sand_sorted_list_node_link_set(
            unit,
            sorted_list,
            prev,
            next
          ) ;
  DNX_SAND_CHECK_FUNC_RESULT(res, 16, exit);

  res = SORTED_LIST_ACCESS_DATA.null_ptr.get(unit,sorted_list_index,&null_ptr) ;
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 20, exit) ;
  res = SORTED_LIST_ACCESS_DATA.ptr_size.get(unit,sorted_list_index,&ptr_size) ;
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 22, exit) ;

  /* if(SOC_DNX_WB_ENGINE_VAR_NONE == init_info_ptr->wb_var_index) */
  {
    /*
     * Copy 'ptr_size' bytes from '&null_ptr' (treated as a small buffer) into
     * 'SORTED_LIST_ACCESS_DATA.next' buffer at offset 'ptr_size * iter'.
     */
    res = SORTED_LIST_ACCESS_DATA.next.memwrite(unit,sorted_list_index,(uint8 *)&null_ptr,iter * ptr_size, ptr_size) ;
    DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 24, exit) ;
  }
  {
    /*
     * Copy 'ptr_size' bytes from '&null_ptr' (treated as a small buffer) into
     * 'SORTED_LIST_ACCESS_DATA.prev' buffer at offset 'ptr_size * iter'.
     */
    res = SORTED_LIST_ACCESS_DATA.prev.memwrite(unit,sorted_list_index,(uint8 *)&null_ptr,iter * ptr_size, ptr_size) ;
    DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 26, exit) ;
  }
  res = SORTED_LIST_ACCESS_DATA.memory_use.get(unit,sorted_list_index,&memory_use) ;
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 28, exit) ;
  res = dnx_sand_occ_bm_occup_status_set(
          unit,
          memory_use,
          iter,
          FALSE
        );
  DNX_SAND_CHECK_FUNC_RESULT(res, 60, exit);
  if (is_indices_enabled)
  {
    uint32
      ii,
      num_elements_on_indices,
      node_ptr ;

    res = SORTED_LIST_ACCESS_DATA.num_elements_on_indices.get(unit,sorted_list_index,&num_elements_on_indices) ;
    DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 34, exit) ;
    if (index_in_indices < (num_elements_on_indices - 1))
    {
      for (ii = index_in_indices + 1 ; ii < num_elements_on_indices ; ii++)
      {
        res = SORTED_LIST_ACCESS_DATA.indices.get(unit,sorted_list_index,ii,&node_ptr) ;
        DNX_SAND_CHECK_FUNC_RESULT(res, 38, exit);
        res = SORTED_LIST_ACCESS_DATA.indices.set(unit,sorted_list_index,ii - 1,node_ptr) ;
        DNX_SAND_CHECK_FUNC_RESULT(res, 42, exit);
      }
    }
    num_elements_on_indices -= 1 ;
    res = SORTED_LIST_ACCESS_DATA.num_elements_on_indices.set(unit,sorted_list_index,num_elements_on_indices) ;
    DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 34, exit) ;
  }
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in dnx_sand_sorted_list_entry_remove_by_iter()",0,0);
}

uint32
  dnx_sand_sorted_list_entry_lookup(
    DNX_SAND_IN     int                          unit,
    DNX_SAND_IN     DNX_SAND_SORTED_LIST_PTR     sorted_list,
    DNX_SAND_IN     DNX_SAND_SORTED_LIST_KEY     *const key,
    DNX_SAND_IN     DNX_SAND_SORTED_LIST_DATA    *const data,
    DNX_SAND_OUT    uint8                        *found,
    DNX_SAND_OUT    DNX_SAND_SORTED_LIST_ITER    *iter
  )
{
  uint32
    curr_node,
    prev_node;
  uint32
    res;
  uint32
    index_in_indices ;
  uint32
    sorted_list_index,
    null_ptr ;

  DNX_SAND_INIT_ERROR_DEFINITIONS(DNX_SAND_SORTED_LIST_ENTRY_LOOKUP);
  DNX_SAND_SORTED_LIST_VERIFY_UNIT_IS_LEGAL(unit, 2) ;
  DNX_SAND_SORTED_LIST_CONVERT_HANDLE_TO_SORTEDLIST_INDEX(sorted_list_index,sorted_list) ;
  DNX_SAND_SORTED_LIST_VERIFY_SORTEDLIST_IS_ACTIVE(sorted_list_index,4,6) ;

  DNX_SAND_CHECK_NULL_INPUT(key) ;
  DNX_SAND_CHECK_NULL_INPUT(iter) ;
  /*
   * Check to see whether the entry exists
   */
  if (data != NULL)
  {
    res = dnx_sand_sorted_list_find_match_entry(
            unit,
            sorted_list,
            key,
            data,
            FALSE,
            found,
            &prev_node,
            &curr_node,
            &index_in_indices
          );
    DNX_SAND_CHECK_FUNC_RESULT(res, 10, exit);
  }
  else
  {
    res = dnx_sand_sorted_list_find_match_entry(
            unit,
            sorted_list,
            key,
            data,
            TRUE,
            found,
            &prev_node,
            &curr_node,
            &index_in_indices
          );
    DNX_SAND_CHECK_FUNC_RESULT(res, 20, exit);

    if (*found)
    {
      *iter = curr_node;
    }
    else
    {
      *iter = prev_node;
    }
    goto exit;
  }

  if (found)
  {
    *iter = curr_node;
  }
  else
  {
    res = SORTED_LIST_ACCESS_DATA.null_ptr.get(unit,sorted_list_index,&null_ptr) ;
    DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 30, exit) ;
    *iter = null_ptr;
  }
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in dnx_sand_sorted_list_entry_lookup()",0,0);
}


uint32
  dnx_sand_sorted_list_entry_value(
    DNX_SAND_IN     int                          unit,
    DNX_SAND_IN     DNX_SAND_SORTED_LIST_PTR     sorted_list,
    DNX_SAND_IN     DNX_SAND_SORTED_LIST_ITER    iter,
    DNX_SAND_OUT    DNX_SAND_SORTED_LIST_KEY     *const key,
    DNX_SAND_OUT    DNX_SAND_SORTED_LIST_DATA    *const data
  )
{
  uint32
    res;
  uint32
    sorted_list_index,
    key_size,
    data_size,
    list_size ;

  DNX_SAND_INIT_ERROR_DEFINITIONS(DNX_SAND_SORTED_LIST_ENTRY_VALUE);
  DNX_SAND_SORTED_LIST_VERIFY_UNIT_IS_LEGAL(unit, 2) ;
  DNX_SAND_SORTED_LIST_CONVERT_HANDLE_TO_SORTEDLIST_INDEX(sorted_list_index,sorted_list) ;
  DNX_SAND_SORTED_LIST_VERIFY_SORTEDLIST_IS_ACTIVE(sorted_list_index,4,6) ;

  res = SORTED_LIST_ACCESS_INFO.list_size.get(unit,sorted_list_index,&list_size) ;
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 8, exit);

  if (iter == DNX_SAND_SORTED_LIST_ITER_END(unit,sorted_list) || iter >= list_size)
  {
    DNX_SAND_SET_ERROR_CODE(DNX_SAND_VALUE_OUT_OF_RANGE_ERR, 10, exit);
  }
  if (key != NULL)
  {
    res = SORTED_LIST_ACCESS_INFO.key_size.get(unit,sorted_list_index,&key_size) ;
    DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 18, exit);
    /*
     * read keys
     */
    /*
     * Copy 'key_size' bytes from 'SORTED_LIST_ACCESS_DATA.keys' buffer at offset
     * 'key_size * iter' into input 'key' buffer.
     */
    res = SORTED_LIST_ACCESS_DATA.keys.memread(unit,sorted_list_index,key,iter * key_size, key_size) ;
    DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 26, exit) ;
  }
  if (data != NULL)
  {
    res = SORTED_LIST_ACCESS_INFO.data_size.get(unit,sorted_list_index,&data_size) ;
    DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 28, exit);
    /*
     * read data
     */
    /*
     * Copy 'data_size' bytes from 'SORTED_LIST_ACCESS_DATA.data' buffer at offset
     * 'data_size * iter' into input 'data' buffer.
     */
    res = SORTED_LIST_ACCESS_DATA.data.memread(unit,sorted_list_index,data,iter * data_size, data_size) ;
    DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 38, exit) ;
  }
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in dnx_sand_sorted_list_entry_value()",0,0);
}


uint32
  dnx_sand_sorted_list_get_next(
    DNX_SAND_IN     int                          unit,
    DNX_SAND_IN     DNX_SAND_SORTED_LIST_PTR     sorted_list,
    DNX_SAND_INOUT  DNX_SAND_SORTED_LIST_ITER    *iter,
    DNX_SAND_OUT    DNX_SAND_SORTED_LIST_KEY     *const key,
    DNX_SAND_OUT    DNX_SAND_SORTED_LIST_DATA    *const data
  )
{
  uint32
    res;
  uint32
    sorted_list_index ;

  DNX_SAND_INIT_ERROR_DEFINITIONS(DNX_SAND_SORTED_LIST_GET_NEXT);
  DNX_SAND_SORTED_LIST_VERIFY_UNIT_IS_LEGAL(unit, 2) ;
  DNX_SAND_SORTED_LIST_CONVERT_HANDLE_TO_SORTEDLIST_INDEX(sorted_list_index,sorted_list) ;
  DNX_SAND_SORTED_LIST_VERIFY_SORTEDLIST_IS_ACTIVE(sorted_list_index,4,6) ;

  if (*iter == DNX_SAND_SORTED_LIST_ITER_END(unit,sorted_list))
  {
    goto exit ;
  }
  res = dnx_sand_sorted_list_get_next_aux(
            unit,
            sorted_list,
            *iter,
            TRUE,
            key,
            data,iter
          ) ;
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 10, exit) ;
  if (*iter == DNX_SAND_SORTED_LIST_ITER_END(unit,sorted_list))
  {
    goto exit;
  }
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "dnx_sand_sorted_list_get_next()",0,0);
}

uint32
  dnx_sand_sorted_list_get_prev(
    DNX_SAND_IN     int                           unit,
    DNX_SAND_IN     DNX_SAND_SORTED_LIST_PTR      sorted_list,
    DNX_SAND_INOUT  DNX_SAND_SORTED_LIST_ITER     *iter,
    DNX_SAND_OUT    DNX_SAND_SORTED_LIST_KEY      *const key,
    DNX_SAND_OUT    DNX_SAND_SORTED_LIST_DATA     *const data
  )
{
  uint32
    head_place ;
  uint32
    sorted_list_index ;
  uint32
    res;

  DNX_SAND_INIT_ERROR_DEFINITIONS(DNX_SAND_SORTED_LIST_GET_PREV);
  DNX_SAND_SORTED_LIST_VERIFY_UNIT_IS_LEGAL(unit, 2) ;
  DNX_SAND_SORTED_LIST_CONVERT_HANDLE_TO_SORTEDLIST_INDEX(sorted_list_index,sorted_list) ;
  DNX_SAND_SORTED_LIST_VERIFY_SORTEDLIST_IS_ACTIVE(sorted_list_index,4,6) ;

  DNX_SAND_CHECK_NULL_INPUT(data) ;
  DNX_SAND_CHECK_NULL_INPUT(key);

  DNX_SAND_SORTED_LIST_HEAD_PLACE_GET(sorted_list_index,head_place,12) ;
  if (*iter == head_place /* DNX_SAND_SORTED_LIST_HEAD_PLACE(&(sorted_list->init_info)) */)
  {
    goto exit;
  }
  res = dnx_sand_sorted_list_get_next_aux(
            unit,
            sorted_list,
            *iter,
            FALSE,
            key,
            data,iter
          );
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 10, exit) ;
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "dnx_sand_sorted_list_get_prev()",0,0);
}

uint32
  dnx_sand_sorted_list_get_follow(
    DNX_SAND_IN     int                           unit,
    DNX_SAND_IN     DNX_SAND_SORTED_LIST_PTR      sorted_list,
    DNX_SAND_INOUT  DNX_SAND_SORTED_LIST_ITER     *iter,
    DNX_SAND_OUT    uint8                         forward,
    DNX_SAND_OUT    DNX_SAND_SORTED_LIST_KEY      *const key,
    DNX_SAND_OUT    DNX_SAND_SORTED_LIST_DATA     *const data
    )
{
  uint32
    head_place ;
  uint32
    sorted_list_index ;
  uint32
    res;

  DNX_SAND_INIT_ERROR_DEFINITIONS(DNX_SAND_SORTED_LIST_GET_FOLLOW);
  DNX_SAND_SORTED_LIST_VERIFY_UNIT_IS_LEGAL(unit, 2) ;
  DNX_SAND_SORTED_LIST_CONVERT_HANDLE_TO_SORTEDLIST_INDEX(sorted_list_index,sorted_list) ;
  DNX_SAND_SORTED_LIST_VERIFY_SORTEDLIST_IS_ACTIVE(sorted_list_index,4,6) ;

  DNX_SAND_CHECK_NULL_INPUT(key);
  DNX_SAND_CHECK_NULL_INPUT(data);

  DNX_SAND_SORTED_LIST_HEAD_PLACE_GET(sorted_list_index,head_place,12) ;
  if (*iter == head_place /* DNX_SAND_SORTED_LIST_HEAD_PLACE(&(sorted_list->init_info)) */ )
  {
    goto exit;
  }
  res = dnx_sand_sorted_list_get_next_aux(
            unit,
            sorted_list,
            *iter,
            forward,
            key,
            data,iter
          );
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 10, exit) ;
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "dnx_sand_sorted_list_get_follow()",0,0);
}

/*********************************************************************
* NAME:
*     dnx_sand_sorted_list_get_size_for_save
* TYPE:
*   PROC
* DATE:
*   May 19 2015
* FUNCTION:
*   Returns the size of the buffer needed to return the sorted list as buffer.
*   in sort to be loaded later
* INPUT:
*   DNX_SAND_IN  int                         unit -
*     Identifier of the device to access.
*   DNX_SAND_IN  DNX_SAND_SORTED_LIST_INFO   *sorted_list -
*     The sorted list to get the size for.
*   DNX_SAND_OUT  uint32                     *size -
*     the size of the buffer.
* REMARKS:
*   None.
* RETURNS:
*   OK or ERROR indication.
*********************************************************************/
uint32
dnx_sand_sorted_list_get_size_for_save(
     DNX_SAND_IN   int                            unit,
     DNX_SAND_IN   DNX_SAND_SORTED_LIST_PTR       sorted_list,
     DNX_SAND_OUT  uint32                         *size
   )
{
  uint32
    bmp_size,
    total_size ;
  uint32
    res;
  uint32
    tmp_size;
  uint32
    sorted_list_index,
    ptr_size,
    key_size,
    list_size,
    data_size ;
  DNX_SAND_OCC_BM_PTR
    memory_use ;

  DNX_SAND_INIT_ERROR_DEFINITIONS(0);
  DNX_SAND_SORTED_LIST_VERIFY_UNIT_IS_LEGAL(unit, 2) ;
  DNX_SAND_SORTED_LIST_CONVERT_HANDLE_TO_SORTEDLIST_INDEX(sorted_list_index,sorted_list) ;
  DNX_SAND_SORTED_LIST_VERIFY_SORTEDLIST_IS_ACTIVE(sorted_list_index,4,6) ;

  total_size = 0;
  DNX_SAND_CHECK_NULL_INPUT(size);

  res = SORTED_LIST_ACCESS_DATA.ptr_size.get(unit,sorted_list_index,&ptr_size) ;
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 12, exit) ;
  res = SORTED_LIST_ACCESS_INFO.data_size.get(unit,sorted_list_index,&data_size) ;
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 14, exit);
  tmp_size = DNX_SAND_MAX(ptr_size,data_size);

  /* init info */
  total_size += sizeof(DNX_SAND_SORTED_LIST_INIT_INFO);

  /* DS data */
  res = SORTED_LIST_ACCESS_INFO.key_size.get(unit,sorted_list_index,&key_size) ;
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 16, exit);
  res = SORTED_LIST_ACCESS_INFO.list_size.get(unit,sorted_list_index,&list_size) ;
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 18, exit);

  total_size += sizeof(uint8) * tmp_size;

  total_size += sizeof(uint8) * key_size;

  total_size += sizeof(uint8) * list_size * key_size;
  
  total_size += sizeof(uint8) * (list_size + 2) * ptr_size;

  total_size += sizeof(uint8) * (list_size + 2) * ptr_size;

  total_size += sizeof(uint8) * list_size * data_size;
  /*
   * Initialize the data to be mapped to
   */
  res = SORTED_LIST_ACCESS_DATA.memory_use.get(unit,sorted_list_index,&memory_use) ;
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 28, exit) ;
  res = dnx_sand_occ_bm_get_size_for_save(
    unit,
    memory_use,
    &bmp_size
    );
  DNX_SAND_CHECK_FUNC_RESULT(res,30, exit) ;
  total_size += bmp_size ;

  *size = total_size ;

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in dnx_sand_hash_table_get_size_for_save()",0,0);
}

/*********************************************************************
* NAME:
*     dnx_sand_sorted_list_save
* TYPE:
*   PROC
* DATE:
*   May 19 2015
* FUNCTION:
*     saves the given sorted list in the given buffer
* INPUT:
*   DNX_SAND_IN  int                           unit -
*     Identifier of the device to access.
*   DNX_SAND_IN  DNX_SAND_SORTED_LIST_PTR      sorted_list -
*     The sorted list to save.
*   DNX_SAND_OUT  uint8                       *buffer -
*     buffer to include the hast table
*   DNX_SAND_IN  uint32                        buffer_size_bytes,
*   DNX_SAND_OUT uint32                       *actual_size_bytes
* REMARKS:
*   - the size of the buffer has to be at least as the value returned
*     by dnx_sand_sorted_list_get_size_for_save.
*   - the hash and rehash functions are not saved.
* RETURNS:
*   OK or ERROR indication.
*********************************************************************/
uint32
dnx_sand_sorted_list_save(
    DNX_SAND_IN  int                           unit,
    DNX_SAND_IN  DNX_SAND_SORTED_LIST_PTR      sorted_list,
    DNX_SAND_OUT uint8                         *buffer,
    DNX_SAND_IN  uint32                        buffer_size_bytes,
    DNX_SAND_OUT uint32                        *actual_size_bytes
  )
{
  uint8 
    *cur_ptr ;
  uint32
    tmp_size;
  uint32
    cur_size,
    total_size ;
  uint32
    res;
  uint32
    sorted_list_index,
    ptr_size,
    key_size,
    list_size,
    data_size ;
  DNX_SAND_OCC_BM_PTR
    memory_use ;

  DNX_SAND_INIT_ERROR_DEFINITIONS(0);
  DNX_SAND_SORTED_LIST_VERIFY_UNIT_IS_LEGAL(unit, 2) ;
  DNX_SAND_SORTED_LIST_CONVERT_HANDLE_TO_SORTEDLIST_INDEX(sorted_list_index,sorted_list) ;
  DNX_SAND_SORTED_LIST_VERIFY_SORTEDLIST_IS_ACTIVE(sorted_list_index,4,6) ;

  DNX_SAND_CHECK_NULL_INPUT(buffer);
  DNX_SAND_CHECK_NULL_INPUT(actual_size_bytes);

  cur_ptr = buffer ;
  total_size = 0 ;

  res = SORTED_LIST_ACCESS_DATA.ptr_size.get(unit,sorted_list_index,&ptr_size) ;
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 12, exit) ;
  res = SORTED_LIST_ACCESS_INFO.data_size.get(unit,sorted_list_index,&data_size) ;
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 14, exit) ;
  tmp_size = DNX_SAND_MAX(ptr_size,data_size) ;
  /*
   * copy init info structure (DNX_SAND_SORTED_LIST_INIT_INFO) into supplied (input) buffer area.
   */
  res = SORTED_LIST_ACCESS_INFO.get(unit,sorted_list_index,(DNX_SAND_SORTED_LIST_INIT_INFO *)cur_ptr) ;
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 16, exit) ;
  cur_ptr += sizeof(DNX_SAND_SORTED_LIST_INIT_INFO) ;
  /*
   * Copy DS data
   */
  res = SORTED_LIST_ACCESS_INFO.key_size.get(unit,sorted_list_index,&key_size) ;
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 16, exit);
  res = SORTED_LIST_ACCESS_INFO.list_size.get(unit,sorted_list_index,&list_size) ;
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 18, exit);

  res = SORTED_LIST_ACCESS_DATA.tmp_data.memwrite(unit,sorted_list_index,cur_ptr,0,tmp_size) ;
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 22, exit) ;
  cur_ptr += tmp_size ;

  res = SORTED_LIST_ACCESS_DATA.tmp_key.memwrite(unit,sorted_list_index,cur_ptr,0,key_size) ;
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 24, exit) ;
  cur_ptr += key_size ;

  res = SORTED_LIST_ACCESS_DATA.keys.memwrite(unit,sorted_list_index,cur_ptr,0,list_size * key_size) ;
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 26, exit) ;
  cur_ptr += (list_size * key_size) ;

  res = SORTED_LIST_ACCESS_DATA.next.memwrite(unit,sorted_list_index,cur_ptr,0,(list_size + 2) * ptr_size) ;
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 28, exit) ;
  cur_ptr += ((list_size + 2) * ptr_size) ;

  res = SORTED_LIST_ACCESS_DATA.prev.memwrite(unit,sorted_list_index,cur_ptr,0,(list_size + 2) * ptr_size) ;
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 30, exit) ;
  cur_ptr += ((list_size + 2) * ptr_size) ;

  res = SORTED_LIST_ACCESS_DATA.data.memwrite(unit,sorted_list_index,cur_ptr,0,list_size * data_size) ;
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 30, exit) ;
  cur_ptr += (list_size * data_size) ;
  /*
   * Initialize the data to be mapped to
   */
  res = SORTED_LIST_ACCESS_DATA.memory_use.get(unit,sorted_list_index,&memory_use) ;
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 28, exit) ;
  res = dnx_sand_occ_bm_save(
          unit,
          memory_use,
          cur_ptr,
          buffer_size_bytes - total_size,
          &cur_size
        );
  DNX_SAND_CHECK_FUNC_RESULT(res,36, exit) ;
  cur_ptr += cur_size ;
  total_size += cur_size ;

  *actual_size_bytes = total_size;

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in dnx_sand_hash_table_save()",0,0);
}


/*********************************************************************
* NAME:
*     dnx_sand_sorted_list_load
* TYPE:
*   PROC
* DATE:
*   May 19 2015
* FUNCTION:
*   Load sorted list from the given buffer which contains a previously
*   saved sorted list.
* INPUT:
*   DNX_SAND_IN  int                                   unit -
*     Identifier of the device to access.
*   DNX_SAND_IN  uint8                                 **buffer -
*     buffer includes the sorted list
*   DNX_SAND_IN  DNX_SAND_SORTED_LIST_SW_DB_ENTRY_GET  get_entry_fun,
*   DNX_SAND_IN  DNX_SAND_SORTED_LIST_SW_DB_ENTRY_SET  set_entry_fun,
*   DNX_SAND_IN  DNX_SAND_SORTED_LIST_KEY_CMP          cmp_key_fun,
*   DNX_SAND_OUT DNX_SAND_SORTED_LIST_PTR              *sorted_list_ptr -
*     Handle to the sorted list to load.
* REMARKS:
*   - the size of the buffer has to be at least as the value returned
*     by dnx_sand_sorted_list_get_size_for_save.
*   - there is need to supply the hash and rehash function (in case they are not
*     the default implementation, cause in the save they are not saved.
*     by dnx_sand_sorted_list_get_size_for_save.
* RETURNS:
*   OK or ERROR indication.
*********************************************************************/
uint32
  dnx_sand_sorted_list_load(
    DNX_SAND_IN  int                                   unit,
    DNX_SAND_IN  uint8                                 **buffer,
    DNX_SAND_IN  DNX_SAND_SORTED_LIST_SW_DB_ENTRY_GET  get_entry_fun,
    DNX_SAND_IN  DNX_SAND_SORTED_LIST_SW_DB_ENTRY_SET  set_entry_fun,
    DNX_SAND_IN  DNX_SAND_SORTED_LIST_KEY_CMP          cmp_key_fun,                      
    DNX_SAND_OUT DNX_SAND_SORTED_LIST_PTR              *sorted_list_ptr
  )
{
  DNX_SAND_SORTED_LIST_INIT_INFO
    local_init_info ;
  DNX_SAND_IN uint8
    *cur_ptr ;
  uint32
    res;
  uint32
    tmp_size ;
  DNX_SAND_SORTED_LIST_PTR
    sorted_list ;
  uint32
    sorted_list_index,
    ptr_size,
    key_size,
    list_size,
    data_size ;
  DNX_SAND_OCC_BM_PTR
    memory_use ;

  DNX_SAND_INIT_ERROR_DEFINITIONS(0);
  DNX_SAND_SORTED_LIST_VERIFY_UNIT_IS_LEGAL(unit, 2) ;

  DNX_SAND_CHECK_NULL_INPUT(buffer);
  DNX_SAND_CHECK_NULL_INPUT(sorted_list_ptr);

  cur_ptr = buffer[0] ;
  /*
   * copy init info (structure DNX_SAND_SORTED_LIST_INIT_INFO) from supplied (input) buffer
   * area into local memory and create sorted list.
   */
  dnx_sand_os_memcpy(&local_init_info, cur_ptr, sizeof(DNX_SAND_SORTED_LIST_INIT_INFO));
  cur_ptr += sizeof(DNX_SAND_SORTED_LIST_INIT_INFO);
  local_init_info.get_entry_fun = get_entry_fun ;
  local_init_info.set_entry_fun = set_entry_fun ;
  /*
   * create DS - will not work!! (petra only code)
   */
  res = dnx_sand_sorted_list_create(unit, sorted_list_ptr, local_init_info);
  DNX_SAND_CHECK_FUNC_RESULT(res,20, exit) ;
  sorted_list = *sorted_list_ptr ;
  DNX_SAND_SORTED_LIST_CONVERT_HANDLE_TO_SORTEDLIST_INDEX(sorted_list_index,sorted_list) ;

  res = SORTED_LIST_ACCESS_DATA.ptr_size.get(unit,sorted_list_index,&ptr_size) ;
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 12, exit) ;
  res = SORTED_LIST_ACCESS_INFO.data_size.get(unit,sorted_list_index,&data_size) ;
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 14, exit) ;
  tmp_size = DNX_SAND_MAX(ptr_size,data_size);

  /* copy DS data */
  res = SORTED_LIST_ACCESS_INFO.key_size.get(unit,sorted_list_index,&key_size) ;
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 16, exit);
  res = SORTED_LIST_ACCESS_INFO.list_size.get(unit,sorted_list_index,&list_size) ;
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 18, exit);

  res = SORTED_LIST_ACCESS_DATA.tmp_data.memread(unit,sorted_list_index,(uint8 *)cur_ptr,0,tmp_size) ;
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 22, exit) ;
  cur_ptr += tmp_size ;

  res = SORTED_LIST_ACCESS_DATA.tmp_key.memread(unit,sorted_list_index,(uint8 *)cur_ptr,0,key_size) ;
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 24, exit) ;
  cur_ptr += key_size ;

  res = SORTED_LIST_ACCESS_DATA.keys.memread(unit,sorted_list_index,(uint8 *)cur_ptr,0,list_size * key_size) ;
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 26, exit) ;
  cur_ptr += (list_size * key_size) ;

  res = SORTED_LIST_ACCESS_DATA.next.memread(unit,sorted_list_index,(uint8 *)cur_ptr,0,(list_size + 2) * ptr_size) ;
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 28, exit) ;
  cur_ptr += ((list_size + 2) * ptr_size) ;

  res = SORTED_LIST_ACCESS_DATA.prev.memread(unit,sorted_list_index,(uint8 *)cur_ptr,0,(list_size + 2) * ptr_size) ;
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 30, exit) ;
  cur_ptr += ((list_size + 2) * ptr_size) ;

  res = SORTED_LIST_ACCESS_DATA.data.memread(unit,sorted_list_index,(uint8 *)cur_ptr,0,list_size * data_size) ;
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 32, exit) ;
  cur_ptr += (list_size * data_size) ;

  /* Destroy (release) bitmap */
  res = SORTED_LIST_ACCESS_DATA.memory_use.get(unit,sorted_list_index,&memory_use) ;
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 34, exit) ;
  res = dnx_sand_occ_bm_destroy(
    unit,
    memory_use
    );
  DNX_SAND_CHECK_FUNC_RESULT(res,36, exit);

  /* Load bitmap */
  res = dnx_sand_occ_bm_load(
    unit,
    &cur_ptr,
    &memory_use
    );
  DNX_SAND_CHECK_FUNC_RESULT(res,38, exit);
  res = SORTED_LIST_ACCESS_DATA.memory_use.set(unit,sorted_list_index,memory_use) ;
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 40, exit) ;

  *buffer = cur_ptr ;
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in dnx_sand_hash_table_load()",0,0);
}

uint32
  dnx_sand_sorted_list_print(
    DNX_SAND_IN  int                            unit,
    DNX_SAND_IN  DNX_SAND_SORTED_LIST_PTR       sorted_list,
    DNX_SAND_IN  char                           table_header[DNX_SAND_SORTED_LIST_HEADER_SIZE],
    DNX_SAND_IN  DNX_SAND_SORTED_LIST_PRINT_VAL print_key,
    DNX_SAND_IN  DNX_SAND_SORTED_LIST_PRINT_VAL print_data
  )
{
  uint32
    prev,
    curr;
  uint32
    sorted_list_index ;
  uint32
    res;
  uint8
    *tmp_data_ptr ;
  uint8
    *tmp_key_ptr ;

  DNX_SAND_INIT_ERROR_DEFINITIONS(DNX_SAND_SORTED_LIST_PRINT);
  DNX_SAND_SORTED_LIST_VERIFY_UNIT_IS_LEGAL(unit, 2) ;
  DNX_SAND_SORTED_LIST_CONVERT_HANDLE_TO_SORTEDLIST_INDEX(sorted_list_index,sorted_list) ;
  DNX_SAND_SORTED_LIST_VERIFY_SORTEDLIST_IS_ACTIVE(sorted_list_index,4,6) ;
  /*
   * traverse the sorted list head list.
   */
  LOG_CLI((BSL_META_U(unit,"  %s\n"), table_header));
  DNX_SAND_SORTED_LIST_HEAD_PLACE_GET(sorted_list_index,prev,12) ;
  res = dnx_sand_sorted_list_get_tmp_data_ptr_from_handle(unit,sorted_list,&tmp_data_ptr) ;
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 10, exit);
  res = dnx_sand_sorted_list_get_tmp_key_ptr_from_handle(unit,sorted_list,&tmp_key_ptr) ;
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 10, exit);
  do
  {
    /*
     * read next entry.
     */
    res = dnx_sand_sorted_list_get_next_aux(
            unit,
            sorted_list,
            prev,
            TRUE,
            tmp_key_ptr,
            tmp_data_ptr,&curr
          );
    DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 12, exit);
    if (curr == DNX_SAND_SORTED_LIST_ITER_END(unit,sorted_list))
    {
      goto exit;
    }
    LOG_CLI((BSL_META_U(unit,"   %-10u"), curr)) ;
    print_data(tmp_data_ptr);
    print_key(tmp_key_ptr);
    LOG_CLI((BSL_META_U(unit,"\n")));

    prev = curr;
  }
  while(curr != DNX_SAND_SORTED_LIST_ITER_END(unit,sorted_list));

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in dnx_sand_sorted_list_print()",0,0);
}

uint32
  dnx_sand_sorted_list_data_cmp(
    DNX_SAND_IN    int                             unit,
    DNX_SAND_IN    DNX_SAND_SORTED_LIST_PTR        sorted_list,
    DNX_SAND_IN    uint32                          data_place,
    DNX_SAND_IN    DNX_SAND_SORTED_LIST_DATA       *const data,
    DNX_SAND_INOUT int32                           *cmp_res_ptr
  )
{
  uint32
    res,
    sorted_list_index,
    data_size ;
  int32
    cmp_res ;
  DNX_SAND_SORTED_LIST_KEY_CMP 
    cmp_key_fun = NULL;
  uint8
    *tmp_data_ptr ;
  DNX_SAND_SORTED_LIST_CMP_FUNC_TYPE
    cmp_func_type ;

  DNX_SAND_INIT_ERROR_DEFINITIONS(DNX_SAND_SORTED_LIST_PRINT);
  DNX_SAND_SORTED_LIST_VERIFY_UNIT_IS_LEGAL(unit, 2) ;
  DNX_SAND_SORTED_LIST_CONVERT_HANDLE_TO_SORTEDLIST_INDEX(sorted_list_index,sorted_list) ;
  DNX_SAND_SORTED_LIST_VERIFY_SORTEDLIST_IS_ACTIVE(sorted_list_index,4,6) ;

  res = SORTED_LIST_ACCESS_INFO.cmp_func_type.get(unit,sorted_list_index,&cmp_func_type) ;
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 8, exit);
  if (cmp_func_type == DNX_SAND_SORTED_LIST_CMP_FUNC_TYPE_TCAM)
  {
    
      DNXC_LEGACY_FIXME_ASSERT;
  }
  else
  {
    cmp_key_fun = (DNX_SAND_SORTED_LIST_KEY_CMP)dnx_sand_os_memcmp;
  }
  {
    /*
     * read data: Copy 'data' to 'tmp_data'
     */
    res = SORTED_LIST_ACCESS_INFO.data_size.get(unit,sorted_list_index,&data_size) ;
    DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 10, exit) ;
    res = dnx_sand_sorted_list_get_tmp_data_ptr_from_handle(unit,sorted_list,&tmp_data_ptr) ;
    DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 12, exit);
    /*
     * Copy 'data_size' bytes from 'SORTED_LIST_ACCESS_DATA.data' buffer at offset
     * 'data_size * data_place' into 'tmp_data' buffer.
     */
    res = SORTED_LIST_ACCESS_DATA.data.memread(unit,sorted_list_index,tmp_data_ptr,data_place * data_size, data_size) ;
    DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 14, exit) ;
    /*
     * Now compare input 'data' buffer with data stored in sorted list using
     * indicated compare function.
     */
    cmp_res =
      cmp_key_fun(
        tmp_data_ptr,
        data,
        data_size * sizeof(uint8)
      ) ;
    *cmp_res_ptr = cmp_res;
  }
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in dnx_sand_sorted_list_data_cmp()",0,0);
}

uint32
  dnx_sand_sorted_list_get_next_aux(
    DNX_SAND_IN    int                         unit,
    DNX_SAND_IN    DNX_SAND_SORTED_LIST_PTR    sorted_list,
    DNX_SAND_IN    DNX_SAND_SORTED_LIST_ITER   curr,
    DNX_SAND_IN    uint8                       forward,
    DNX_SAND_OUT   DNX_SAND_SORTED_LIST_KEY    *const key,
    DNX_SAND_OUT   DNX_SAND_SORTED_LIST_DATA   *const data,
    DNX_SAND_INOUT DNX_SAND_SORTED_LIST_ITER   *next_or_prev
  )
{
  uint32
    ptr_long;
  uint32
    res;
  uint32
    null_ptr,
    sorted_list_index ;
  uint8
    *tmp_data_ptr ;
  uint32
    ptr_size,
    key_size,
    data_size ;

  DNX_SAND_INIT_ERROR_DEFINITIONS(0);
  DNX_SAND_SORTED_LIST_VERIFY_UNIT_IS_LEGAL(unit, 2) ;
  DNX_SAND_SORTED_LIST_CONVERT_HANDLE_TO_SORTEDLIST_INDEX(sorted_list_index,sorted_list) ;
  DNX_SAND_SORTED_LIST_VERIFY_SORTEDLIST_IS_ACTIVE(sorted_list_index,4,6) ;

  DNX_SAND_CHECK_NULL_INPUT(next_or_prev);

  res = SORTED_LIST_ACCESS_DATA.null_ptr.get(unit,sorted_list_index,&null_ptr) ;
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 8, exit) ;
  /*
   * read next entry.
   */
  if (
    (!forward &&  curr == DNX_SAND_SORTED_LIST_ITER_BEGIN(unit,sorted_list)) ||
    ((forward && curr == DNX_SAND_SORTED_LIST_ITER_END(unit,sorted_list)))
    )
  {
    *next_or_prev = null_ptr ;
    goto exit ;
  }
  res = dnx_sand_sorted_list_get_tmp_data_ptr_from_handle(unit,sorted_list,&tmp_data_ptr) ;
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 12, exit);
  res = SORTED_LIST_ACCESS_DATA.ptr_size.get(unit,sorted_list_index,&ptr_size) ;
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 14, exit) ;
  if (forward)
  {
    /*
     * Copy 'ptr_size' bytes from 'SORTED_LIST_ACCESS_DATA.next' buffer at offset
     * 'curr * ptr_size' into 'tmp_data' buffer.
     */
    res = SORTED_LIST_ACCESS_DATA.next.memread(unit,sorted_list_index,tmp_data_ptr,curr * ptr_size,ptr_size) ;
    DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 16, exit) ;
  }
  else
  {
    /*
     * Copy 'ptr_size' bytes from 'SORTED_LIST_ACCESS_DATA.prev' buffer at offset
     * 'curr * ptr_size' into 'tmp_data' buffer.
     */
    res = SORTED_LIST_ACCESS_DATA.prev.memread(unit,sorted_list_index,tmp_data_ptr,curr * ptr_size,ptr_size) ;
    DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 18, exit) ;
  }
  /*
   * Check whether the list node is null.
   */
  ptr_long = 0 ;
  dnx_sand_U8_to_U32(
      tmp_data_ptr,
      ptr_size,
      &ptr_long
  ) ;
  if (ptr_long == null_ptr)
  {
    *next_or_prev = null_ptr ;
    goto exit ;
  }
  if (ptr_long == DNX_SAND_SORTED_LIST_ITER_BEGIN(unit,sorted_list))
  {
    *next_or_prev = ptr_long ;
    goto exit ;
  }
  if (key != NULL)
  {
    /*
     * Read keys
     */
    res = SORTED_LIST_ACCESS_INFO.key_size.get(unit,sorted_list_index,&key_size) ;
    DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 20, exit) ;
    /*
     * Copy 'key_size' bytes from 'SORTED_LIST_ACCESS_DATA.keys' buffer at offset
     * 'ptr_long * key_size' into input 'key' buffer.
     */
    res = SORTED_LIST_ACCESS_DATA.keys.memread(unit,sorted_list_index,key,ptr_long * key_size,key_size) ;
    DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 22, exit) ;
  }
  if (data != NULL)
  {
    /*
     * read data
     */
    res = SORTED_LIST_ACCESS_INFO.data_size.get(unit,sorted_list_index,&data_size) ;
    DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 24, exit) ;
    /*
     * Copy 'data_size' bytes from 'SORTED_LIST_ACCESS_DATA.data' buffer at offset
     * 'ptr_long * data_size' into input 'data' buffer.
     */
    res = SORTED_LIST_ACCESS_DATA.data.memread(unit,sorted_list_index,data,ptr_long * data_size,data_size) ;
    DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 26, exit) ;
  }
  *next_or_prev = ptr_long ;
  goto exit ;
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in dnx_sand_sorted_list_get_next_aux()",0,0);
}

uint32
  dnx_sand_SAND_SORTED_LIST_INFO_clear(
    DNX_SAND_IN int                      unit,
    DNX_SAND_IN DNX_SAND_SORTED_LIST_PTR sorted_list
  )
{
  uint32
    sorted_list_index ;
  uint32
    res;

  DNX_SAND_INIT_ERROR_DEFINITIONS(0);
  DNX_SAND_SORTED_LIST_VERIFY_UNIT_IS_LEGAL(unit, 2) ;
  DNX_SAND_SORTED_LIST_CONVERT_HANDLE_TO_SORTEDLIST_INDEX(sorted_list_index,sorted_list) ;
  DNX_SAND_SORTED_LIST_VERIFY_SORTEDLIST_IS_ACTIVE(sorted_list_index,4,6) ;

  res = SORTED_LIST_ACCESS_INFO.data_size.set(unit, sorted_list_index, 0);
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 8, exit);
  res = SORTED_LIST_ACCESS_INFO.get_entry_fun.set(unit, sorted_list_index, 0);
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 10, exit);
  res = SORTED_LIST_ACCESS_INFO.key_size.set(unit, sorted_list_index, 0);
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 12, exit);
  res = SORTED_LIST_ACCESS_INFO.set_entry_fun.set(unit, sorted_list_index, 0);
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 14, exit);
  res = SORTED_LIST_ACCESS_INFO.list_size.set(unit, sorted_list_index, 0);
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 16, exit);
  res = SORTED_LIST_ACCESS_INFO.prime_handle.set(unit, sorted_list_index, 0);
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 18, exit);
  res = SORTED_LIST_ACCESS_INFO.sec_handle.set(unit, sorted_list_index, 0);
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 20, exit);
  res = SORTED_LIST_ACCESS_INFO.cmp_func_type.set(unit, sorted_list_index, DNX_SAND_SORTED_LIST_CMP_FUNC_TYPE_DEFAULT);
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 22, exit);

  res = SORTED_LIST_ACCESS_DATA.ptr_size.set(unit, sorted_list_index, 0);
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 24, exit);
  res = SORTED_LIST_ACCESS_DATA.memory_use.set(unit, sorted_list_index, 0);
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 26, exit);
  res = SORTED_LIST_ACCESS_DATA.null_ptr.set(unit, sorted_list_index, 0);
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 28, exit);

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in dnx_sand_SAND_SORTED_LIST_INFO_clear()", 0, 0);
}


uint32
  dnx_sand_print_indices(
    int                      unit,
    DNX_SAND_SORTED_LIST_PTR sorted_list
  )
{
  uint32
    res = DNX_SAND_OK ;
  int
    ii ;
  uint32
    node_ptr,
    num_elements_on_indices ;
  static
    int num_invocations = 0 ;
  uint32
    sorted_list_index ;
  
  DNX_SAND_INIT_ERROR_DEFINITIONS(0);
  DNX_SAND_SORTED_LIST_VERIFY_UNIT_IS_LEGAL(unit, 2) ;
  DNX_SAND_SORTED_LIST_CONVERT_HANDLE_TO_SORTEDLIST_INDEX(sorted_list_index,sorted_list) ;
  DNX_SAND_SORTED_LIST_VERIFY_SORTEDLIST_IS_ACTIVE(sorted_list_index,4,6) ;

  num_invocations++ ;
  res = SORTED_LIST_ACCESS_DATA.num_elements_on_indices.get(unit,sorted_list_index,&num_elements_on_indices) ;
  DNX_SAND_CHECK_FUNC_RESULT(res, 10, exit);
  sal_printf("%s(): Invocation %d Sorted_list %d - Display 'indices' of %d entries\r\n",
    __FUNCTION__,num_invocations,(int)sorted_list,num_elements_on_indices) ;
  for (ii = 0 ; ii < num_elements_on_indices ; ii++)
  {
    res = SORTED_LIST_ACCESS_DATA.indices.get(unit,sorted_list_index,ii,&node_ptr) ;
    DNX_SAND_CHECK_FUNC_RESULT(res, 14, exit);
    sal_printf("index %02d value %02d\r\n",ii,node_ptr) ;
  }

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in dnx_sand_print_indices()",0,0);
}
static
  void
    dnx_sand_print_key(
      DNX_SAND_IN  uint8                    *buffer
    )
{
    sal_printf("Key: 0x%02X %02X %02X %02X\r\n",buffer[0],buffer[1],buffer[2],buffer[3]) ;
}
static
  void
    dnx_sand_print_data(
      DNX_SAND_IN  uint8                    *buffer
    )
{
    sal_printf("Data: 0x%02X %02X %02X %02X   %02X %02X %02X %02X\r\n",
      buffer[0],buffer[1],buffer[2],buffer[3],buffer[4],buffer[5],buffer[6],buffer[7]) ;
}
uint32
  dnx_sand_print_list(
    int                      unit,
    DNX_SAND_SORTED_LIST_PTR sorted_list
  )
{
  uint32
      res = DNX_SAND_OK ;
  static
      int num_invocations = 0 ;
  uint32
      sorted_list_index ;

  DNX_SAND_INIT_ERROR_DEFINITIONS(0);
  DNX_SAND_SORTED_LIST_VERIFY_UNIT_IS_LEGAL(unit, 2) ;
  DNX_SAND_SORTED_LIST_CONVERT_HANDLE_TO_SORTEDLIST_INDEX(sorted_list_index,sorted_list) ;
  DNX_SAND_SORTED_LIST_VERIFY_SORTEDLIST_IS_ACTIVE(sorted_list_index,4,6) ;

  num_invocations++ ;
  sal_printf("%s(): Invocation %d Sorted_list %d - \r\n",
    __FUNCTION__,num_invocations,(int)sorted_list) ;
  dnx_sand_sorted_list_print(unit,sorted_list,"Sorted linked list",dnx_sand_print_key,dnx_sand_print_data) ;
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in dnx_sand_print_list()",0,0);
}

#if DNX_SAND_DEBUG
/* { */

uint32
  dnx_sand_SAND_SORTED_LIST_INFO_print(
    DNX_SAND_IN int                      unit,
    DNX_SAND_IN DNX_SAND_SORTED_LIST_PTR sorted_list
  )
{
  uint32
    res,
    key_size,
    list_size,
    data_size,
    sec_handle,
    sorted_list_index ;
  int
    prime_handle ;    

  DNX_SAND_INIT_ERROR_DEFINITIONS(0);
  DNX_SAND_SORTED_LIST_VERIFY_UNIT_IS_LEGAL(unit, 2) ;
  DNX_SAND_SORTED_LIST_CONVERT_HANDLE_TO_SORTEDLIST_INDEX(sorted_list_index,sorted_list) ;
  DNX_SAND_SORTED_LIST_VERIFY_SORTEDLIST_IS_ACTIVE(sorted_list_index,4,6) ;

  res = SORTED_LIST_ACCESS_INFO.data_size.get(unit,sorted_list_index,&data_size) ;
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 8, exit) ;
  res = SORTED_LIST_ACCESS_INFO.key_size.get(unit,sorted_list_index,&key_size) ;
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 10, exit) ;
  res = SORTED_LIST_ACCESS_INFO.list_size.get(unit,sorted_list_index,&list_size) ;
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 12, exit) ;
  res = SORTED_LIST_ACCESS_INFO.prime_handle.get(unit,sorted_list_index,&prime_handle) ;
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 14, exit) ;
  res = SORTED_LIST_ACCESS_INFO.sec_handle.get(unit,sorted_list_index,&sec_handle) ;
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 16, exit) ;

  LOG_CLI((BSL_META_U(unit,"init_info.data_size   : %u\n"),data_size)) ;
  LOG_CLI((BSL_META_U(unit,"init_info.key_size    : %u\n"),key_size)) ;
  LOG_CLI((BSL_META_U(unit,"init_info.list_size   : %u\n"),list_size)) ;
  LOG_CLI((BSL_META_U(unit,"init_info.prime_handle: %u\n"),prime_handle)) ;
  LOG_CLI((BSL_META_U(unit,"init_info.sec_handle  : %u\n"),sec_handle)) ;
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in dnx_sand_SAND_SORTED_LIST_INFO_print()", 0, 0) ;
}
/* } */
#endif /* DNX_SAND_DEBUG */
/*
 * Utilities related to the 'indices' feature.
 */
/* { */

/*********************************************************************
* NAME:
*     dnx_sand_sorted_list_get_num_elements
* TYPE:
*   PROC
* DATE:
*   Dec 01 2015
* FUNCTION:
*  Get current number of elements on specified sorted list. (This is
*  also the number of elements on 'indices' array)
* INPUT:
*   DNX_SAND_IN  int                              unit -
*     Identifier of the device to access.
*   DNX_SAND_IN  DNX_SAND_SORTED_LIST_PTR         sorted_list -
*     Handle to the sorted list.
*   DNX_SAND_OUT uint32                           *num_elements -
*     Number of valid elements currently on the list.
*     Only meaningful if the 'indices' feature is enabled ('indices'
*     element in DNX_SAND_SORTED_LIST_T is allocated). 
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  dnx_sand_sorted_list_get_num_elements(
    DNX_SAND_IN  int                      unit,
    DNX_SAND_IN  DNX_SAND_SORTED_LIST_PTR sorted_list,
    DNX_SAND_OUT uint32                   *num_elements
  )
{
  uint32
    sorted_list_index,
    res ;
  uint8
    is_allocated ;

  DNX_SAND_INIT_ERROR_DEFINITIONS(0) ;
  DNX_SAND_SORTED_LIST_VERIFY_UNIT_IS_LEGAL(unit, 2) ;
  DNX_SAND_SORTED_LIST_CONVERT_HANDLE_TO_SORTEDLIST_INDEX(sorted_list_index,sorted_list) ;
  DNX_SAND_SORTED_LIST_VERIFY_SORTEDLIST_IS_ACTIVE(sorted_list_index,4,6) ;

  res = SORTED_LIST_ACCESS_DATA.indices.is_allocated(unit,sorted_list_index,&is_allocated) ;
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 10, exit) ;
  if (is_allocated)
  {
    res = SORTED_LIST_ACCESS_DATA.num_elements_on_indices.get(unit,sorted_list_index,num_elements) ;
    DNX_SAND_CHECK_FUNC_RESULT(res, 12, exit) ;
  }
  else
  {
    res = DNX_SAND_SORTED_LIST_ILLEGAL_ITER_ERR ;
    DNX_SAND_CHECK_FUNC_RESULT(res, 14, exit) ;
  }
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in dnx_sand_sorted_list_get_num_elements()",0,0);
}
/*********************************************************************
* NAME:
*     dnx_sand_sorted_list_is_indices_enabled
* TYPE:
*   PROC
* DATE:
*   Dec 01 2015
* FUNCTION:
*  Get indication on whether the 'indices' feature is enabled for this
*  sorted list.
* INPUT:
*   DNX_SAND_IN  int                              unit -
*     Identifier of the device to access.
*   DNX_SAND_IN  DNX_SAND_SORTED_LIST_PTR         sorted_list -
*     Handle to the sorted list.
*   DNX_SAND_OUT uint8                            *is_enabled -
*     Indication on whether 'indices' feature is enabled. Non-zero
*     indicates 'enabled'
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  dnx_sand_sorted_list_is_indices_enabled(
    DNX_SAND_IN  int                      unit,
    DNX_SAND_IN  DNX_SAND_SORTED_LIST_PTR sorted_list,
    DNX_SAND_OUT uint8                    *is_enabled
  )
{
  uint32
    sorted_list_index,
    res ;

  DNX_SAND_INIT_ERROR_DEFINITIONS(0) ;
  DNX_SAND_SORTED_LIST_VERIFY_UNIT_IS_LEGAL(unit, 2) ;
  DNX_SAND_SORTED_LIST_CONVERT_HANDLE_TO_SORTEDLIST_INDEX(sorted_list_index,sorted_list) ;
  DNX_SAND_SORTED_LIST_VERIFY_SORTEDLIST_IS_ACTIVE(sorted_list_index,4,6) ;

  res = SORTED_LIST_ACCESS_DATA.indices.is_allocated(unit,sorted_list_index,is_enabled) ;
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 10, exit) ;
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in dnx_sand_sorted_list_is_indices_enabled()",0,0);
}
/*********************************************************************
* NAME:
*     dnx_sand_sorted_list_get_iter_from_indices
* TYPE:
*   PROC
* DATE:
*   Dec 01 2015
* FUNCTION:
*  Given index within 'indices' (which is the ordinal location of
*  a sorted linked list element, starting from low key value), get
*  the corresponding location on array of available linked list
*  elements.
* INPUT:
*   DNX_SAND_IN  int                              unit -
*     Identifier of the device to access.
*   DNX_SAND_IN  DNX_SAND_SORTED_LIST_PTR         sorted_list -
*     Handle to the sorted list.
*   DNX_SAND_IN  uint32                           index_in_indices -
*     Index on 'indices' array to use to get the corresponding index
*     on array of available link-list elements.
*     Only meaningful if the 'indices' feature is enabled ('indices'
*     element in DNX_SAND_SORTED_LIST_T is allocated). Note that this
*     index is counted from the BOTTOM of 'indices'. So, for example,
*     when index=0 then returned entry refers to LOWEST key.
*   DNX_SAND_OUT DNX_SAND_SORTED_LIST_ITER        *iter -
*     Index (location) on array of available linked list elements
*     for this linked list
* REMARKS:
*   Use dnx_sand_sorted_list_entry_value() to get 'key' or 'data'
*   of corresponding linked list element.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  dnx_sand_sorted_list_get_iter_from_indices(
    DNX_SAND_IN  int                       unit,
    DNX_SAND_IN  DNX_SAND_SORTED_LIST_PTR  sorted_list,
    DNX_SAND_IN  uint32                    index_in_indices,
    DNX_SAND_OUT DNX_SAND_SORTED_LIST_ITER *iter
  )
{
  uint32
    sorted_list_index,
    res ;
  uint8
    is_allocated ;

  DNX_SAND_INIT_ERROR_DEFINITIONS(0) ;
  DNX_SAND_SORTED_LIST_VERIFY_UNIT_IS_LEGAL(unit, 2) ;
  DNX_SAND_SORTED_LIST_CONVERT_HANDLE_TO_SORTEDLIST_INDEX(sorted_list_index,sorted_list) ;
  DNX_SAND_SORTED_LIST_VERIFY_SORTEDLIST_IS_ACTIVE(sorted_list_index,4,6) ;

  res = SORTED_LIST_ACCESS_DATA.indices.is_allocated(unit,sorted_list_index,&is_allocated) ;
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 10, exit) ;
  if (is_allocated)
  {
    res = SORTED_LIST_ACCESS_DATA.indices.get(unit,sorted_list_index,index_in_indices,iter) ;
    DNX_SAND_CHECK_FUNC_RESULT(res, 12, exit) ;
  }
  else
  {
    res = DNX_SAND_SORTED_LIST_ILLEGAL_ITER_ERR ;
    DNX_SAND_CHECK_FUNC_RESULT(res, 14, exit) ;
  }
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in dnx_sand_sorted_list_get_iter_from_indices()",0,0);
}
/* } */
static uint32
  dnx_sand_sorted_list_node_link_set(
    DNX_SAND_IN  int                      unit,
    DNX_SAND_IN  DNX_SAND_SORTED_LIST_PTR sorted_list,
    DNX_SAND_IN  uint32                   node1,
    DNX_SAND_IN  uint32                   node2
  )
{
  uint32
    sorted_list_index,
    ptr_size,
    res ;
  uint8
    *tmp_data_ptr ;

  DNX_SAND_INIT_ERROR_DEFINITIONS(0) ;
  DNX_SAND_SORTED_LIST_VERIFY_UNIT_IS_LEGAL(unit, 2) ;
  DNX_SAND_SORTED_LIST_CONVERT_HANDLE_TO_SORTEDLIST_INDEX(sorted_list_index,sorted_list) ;
  DNX_SAND_SORTED_LIST_VERIFY_SORTEDLIST_IS_ACTIVE(sorted_list_index,4,6) ;

  res = dnx_sand_sorted_list_get_tmp_data_ptr_from_handle(unit,sorted_list,&tmp_data_ptr) ;
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 12, exit);
  res = SORTED_LIST_ACCESS_DATA.ptr_size.get(unit,sorted_list_index,&ptr_size) ;
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 14, exit) ;
  res =
    dnx_sand_U32_to_U8(
      &node2,
      ptr_size,
      tmp_data_ptr
    ) ;
  DNX_SAND_CHECK_FUNC_RESULT(res, 10, exit);
  /* if(SOC_DNX_WB_ENGINE_VAR_NONE == init_info_ptr->wb_var_index) */
  {
    /*
     * Copy 'ptr_size' bytes from 'tmp_data_ptr' into
     * 'SORTED_LIST_ACCESS_DATA.next' buffer at offset 'ptr_size * node1'.
     */
    res = SORTED_LIST_ACCESS_DATA.next.memwrite(unit,sorted_list_index,tmp_data_ptr,node1 * ptr_size, ptr_size) ;
    DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 20, exit) ;
  }
  res =
    dnx_sand_U32_to_U8(
      &node1,
      ptr_size,
      tmp_data_ptr
    ) ;
  DNX_SAND_CHECK_FUNC_RESULT(res, 30, exit);
  /* if(SOC_DNX_WB_ENGINE_VAR_NONE == init_info_ptr->wb_var_index) */
  {
    /*
     * Copy 'ptr_size' bytes from 'tmp_data_ptr' into
     * 'SORTED_LIST_ACCESS_DATA.prev' buffer at offset 'ptr_size * node2'.
     */
    res = SORTED_LIST_ACCESS_DATA.prev.memwrite(unit,sorted_list_index,tmp_data_ptr,node2 * ptr_size, ptr_size) ;
    DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 40, exit) ;
  }
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in dnx_sand_sorted_list_node_link_set()",0,0);
}
/*********************************************************************
* NAME:
*     dnx_sand_sorted_list_find_match_entry
* TYPE:
*   PROC
* DATE:
*   May 19 2015
* FUNCTION:
*  lookup in the sorted list for the given key and return the data inserted with
*  the given key.
* INPUT:
*   DNX_SAND_IN  int                              unit -
*     Identifier of the device to access.
*   DNX_SAND_IN  DNX_SAND_SORTED_LIST_PTR         sorted_list -
*     Handle to the sorted list.
*   DNX_SAND_IN  DNX_SAND_SORTED_LIST_KEY         *const key -
*     The key sort to lookup for
*   DNX_SAND_IN  DNX_SAND_SORTED_LIST_DATA        *const data
*     This procedure loads pointed memory by data
*     of matching entry. If set to NULL then this
*     procedure does not load indicated data.
*   DNX_SAND_IN  uint8                            first_match
*     whether to return the first match key. regardless of the data.
*   DNX_SAND_IN  uint8                            *found -
*     whether the data with the given key was found in the sorted list
*   DNX_SAND_OUT  uint32                          *entry -
*     if the key is present in the sorted list then return the entry the key found at,
*     otherwise it returns the place where the key suppose to be.
*   DNX_SAND_OUT  DNX_SAND_SORTED_LIST_ITER       *prev_node
*     iterator points to one node before the searched node.
*   DNX_SAND_OUT  DNX_SAND_SORTED_LIST_ITER       *next_node
*     iterator points to searched node or node after.
*   DNX_SAND_OUT  uint32                          *index_in_indices -
*     Only meaningful if the 'indices' feature is enabled ('indices' element
*     in DNX_SAND_SORTED_LIST_T is allocated). Index in 'indices'
*     array after which the newly matched entry fits (and may be added)
*     assuming 'found' is TRUE. Note that this index is counted from
*     the TOP of 'indices'. So, for example, when index=0 then entry
*     should be added as LAST on 'indices'.
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
static uint32
  dnx_sand_sorted_list_find_match_entry(
    DNX_SAND_IN   int                           unit,
    DNX_SAND_IN   DNX_SAND_SORTED_LIST_PTR      sorted_list,
    DNX_SAND_IN   DNX_SAND_SORTED_LIST_KEY      *const key,
    DNX_SAND_IN   DNX_SAND_SORTED_LIST_DATA     *const data,
    DNX_SAND_IN   uint8                         first_match,
    DNX_SAND_OUT  uint8                         *found,
    DNX_SAND_OUT  DNX_SAND_SORTED_LIST_ITER     *prev_node,
    DNX_SAND_OUT  DNX_SAND_SORTED_LIST_ITER     *cur_node,
    DNX_SAND_OUT  uint32                        *index_in_indices
  )
{
  uint32
    key_size,
    null_ptr,
    prev,
    curr,
    curr_index ;
  DNX_SAND_SORTED_LIST_KEY_CMP 
    cmp_key_fun = NULL;
  DNX_SAND_SORTED_LIST_CMP_FUNC_TYPE
    cmp_func_type ;
  int32
    compare_res ;
  uint32
    sorted_list_index,
    res;
  uint8
    *tmp_key_ptr ;

  DNX_SAND_INIT_ERROR_DEFINITIONS(DNX_SAND_SORTED_LIST_FIND_MATCH_ENTRY);
  /*
   * Just make sure that, in case of error, an illegal value is returned.
   * Good practive though not necessary.
   */
  *index_in_indices = -2 ;
  DNX_SAND_SORTED_LIST_VERIFY_UNIT_IS_LEGAL(unit, 2) ;
  DNX_SAND_SORTED_LIST_CONVERT_HANDLE_TO_SORTEDLIST_INDEX(sorted_list_index,sorted_list) ;
  DNX_SAND_SORTED_LIST_VERIFY_SORTEDLIST_IS_ACTIVE(sorted_list_index,4,6) ;

  DNX_SAND_CHECK_NULL_INPUT(key) ;
  DNX_SAND_CHECK_NULL_INPUT(found) ;
  DNX_SAND_CHECK_NULL_INPUT(prev_node) ;
  DNX_SAND_CHECK_NULL_INPUT(cur_node) ;

  res = SORTED_LIST_ACCESS_INFO.cmp_func_type.get(unit,sorted_list_index,&cmp_func_type) ;
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 8, exit);
  if (cmp_func_type == DNX_SAND_SORTED_LIST_CMP_FUNC_TYPE_TCAM)
  {
    
      DNXC_LEGACY_FIXME_ASSERT;
  }
  else
  {
    cmp_key_fun = (DNX_SAND_SORTED_LIST_KEY_CMP)dnx_sand_os_memcmp ;
  }
  *found = FALSE ;
  DNX_SAND_SORTED_LIST_TAIL_PLACE_GET(sorted_list_index,prev,12) ;

  res = dnx_sand_sorted_list_get_tmp_key_ptr_from_handle(unit,sorted_list,&tmp_key_ptr) ;
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 16, exit);
  res = SORTED_LIST_ACCESS_INFO.key_size.get(unit,sorted_list_index,&key_size) ;
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 18, exit) ;
  /*
   * 'curr_index' is the index into 'indices' array beyond which the new node is
   * to be added. Only meaningful if 'indices' feature is enabled.
   */
  curr_index = 0 ;
  do
  {
    /*
     * Read next entry. (if inverted_order - read previous)
     */
    res = dnx_sand_sorted_list_get_next_aux(
            unit,
            sorted_list,
            prev,
            FALSE, /* Scan the list from the end - faster in TCAM for sorted entries */
            tmp_key_ptr,
            NULL,&curr
          );
    DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 20, exit) ;
    if (curr == DNX_SAND_SORTED_LIST_ITER_BEGIN(unit,sorted_list))
    {
      *found = FALSE;
      *prev_node = DNX_SAND_SORTED_LIST_ITER_BEGIN(unit,sorted_list);
      *cur_node = prev;
      /*
       * We need to insert at the beginning of the array
       */
      *index_in_indices = curr_index ;
      goto exit;
    }
    compare_res =
      cmp_key_fun(
        tmp_key_ptr,
        key,
        (key_size * sizeof(DNX_SAND_SORTED_LIST_KEY_TYPE))
      ) ;
    /*
     * If key was found
     */
    if (compare_res == 0)
    {
      if (first_match)
      {
          *found = TRUE;
          prev = curr;
          res = dnx_sand_sorted_list_get_next_aux(
                 unit,
                 sorted_list,
                 prev,
                 FALSE,
                 tmp_key_ptr,
                 NULL,&curr
               );
          DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 30, exit) ;
          *prev_node = curr;
          *cur_node =  prev;
          *index_in_indices = curr_index ;
          goto exit;
      }
      if (data != NULL)
      {
        res =
          dnx_sand_sorted_list_data_cmp(
            unit,
            sorted_list,
            curr,
            data, &compare_res
          );
         DNX_SAND_CHECK_FUNC_RESULT(res, 40, exit) ;
        if (compare_res)
        {
          *found = TRUE;
          prev = curr;
          res = dnx_sand_sorted_list_get_next_aux(
                   unit,
                   sorted_list,
                   prev,
                   FALSE,
                   tmp_key_ptr,
                   NULL,&curr
                 );
          DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 50, exit) ;
          *prev_node = curr;
          *cur_node =  prev;
          *index_in_indices = curr_index ;
          goto exit;
        }
      }
    }
    else if (compare_res < 0)
    {
      res = SORTED_LIST_ACCESS_DATA.null_ptr.get(unit,sorted_list_index,&null_ptr) ;
      DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 8, exit) ;
      *found = FALSE ;
      *prev_node = (curr == DNX_SAND_SORTED_LIST_ITER_END(unit,sorted_list)) ? null_ptr : curr ;
      *cur_node = prev ;
      *index_in_indices = curr_index ;
      goto exit ;
    }
    prev = curr ;
    curr_index++ ;
  }  while (!(*found)) ;
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in dnx_sand_sorted_list_find_match_entry()",0,0);
}

/*********************************************************************
* NAME:
*     dnx_sand_sorted_list_node_alloc
* TYPE:
*   PROC
* DATE:
*   May 19 2015
* FUNCTION:
*  Insert an entry into the sorted list, if already exist then
*  the operation returns an error.
* INPUT:
*   DNX_SAND_IN  int                              unit -
*     Identifier of the device to access.
*   DNX_SAND_IN  DNX_SAND_SORTED_LIST_PTR         sorted_list -
*     Handle to the sorted list.
*   DNX_SAND_IN  DNX_SAND_SORTED_LIST_KEY         *key -
*     The key to add into the sorted list
*   DNX_SAND_IN  DNX_SAND_SORTED_LIST_DATA        *data -
*     The data to add into the sorted list and to be associated with
*     the given key
*   DNX_SAND_OUT  uint8                           *success -
*     whether the add operation success, this may be false.
*     if after trying to relist the key DNX_SAND_SORTED_LIST_MAX_NOF_RELIST times
*     and in all tries fill in in_use entry. to solve this problem try
*     to enlarge the sorted list size or use better list function.
*   DNX_SAND_IN  uint32                           from_top_index_in_indices -
*     Only meaningful if the 'indices' feature is enabled ('indices' element
*     in DNX_SAND_SORTED_LIST_T is allocated). Index in 'indices'
*     array after which the newly matched entry fits (and should be added).
*     Note that this index is counted from the TOP of 'indices'. So, for
*     example, when index=0 then entry should be added as LAST on 'indices'.
* REMARKS:
*     = if there is already a key with the same key in the sorted list error is returned.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
static uint32
  dnx_sand_sorted_list_node_alloc(
    DNX_SAND_IN   uint32                    unit,
    DNX_SAND_IN  DNX_SAND_SORTED_LIST_PTR   sorted_list,
    DNX_SAND_IN  DNX_SAND_SORTED_LIST_KEY   *const key,
    DNX_SAND_IN  DNX_SAND_SORTED_LIST_DATA  *const data,
    DNX_SAND_IN   uint32                    prev_node,
    DNX_SAND_IN   uint32                    next_node,
    DNX_SAND_IN   uint32                    from_top_index_in_indices,
    DNX_SAND_OUT  uint8                     *found
  )
{
  uint32
    new_node_ptr;
  uint32
    index_in_indices,
    sorted_list_index,
    res;
  uint32
    key_size,
    data_size ;
  DNX_SAND_OCC_BM_PTR
    memory_use ;

  DNX_SAND_INIT_ERROR_DEFINITIONS(0);
  DNX_SAND_SORTED_LIST_VERIFY_UNIT_IS_LEGAL(unit, 2) ;
  DNX_SAND_SORTED_LIST_CONVERT_HANDLE_TO_SORTEDLIST_INDEX(sorted_list_index,sorted_list) ;
  DNX_SAND_SORTED_LIST_VERIFY_SORTEDLIST_IS_ACTIVE(sorted_list_index,4,6) ;

  DNX_SAND_CHECK_NULL_INPUT(key) ;
  DNX_SAND_CHECK_NULL_INPUT(data) ;
  DNX_SAND_CHECK_NULL_INPUT(found) ;
  /*
   * Check to see whether the entry exists
   */
  res = SORTED_LIST_ACCESS_DATA.memory_use.get(unit,sorted_list_index,&memory_use) ;
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 10, exit) ;
  res =
    dnx_sand_occ_bm_alloc_next(
      unit,
      memory_use,
      &new_node_ptr,
      found
    ) ;
  if (*found == FALSE)
  {
    goto exit;
  }
  /* if(SOC_DNX_WB_ENGINE_VAR_NONE == init_info_ptr->wb_var_index) */
  {
    res = SORTED_LIST_ACCESS_INFO.key_size.get(unit,sorted_list_index,&key_size) ;
    DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 18, exit) ;
    /*
     * Copy 'key_size' bytes from input 'keys' buffer into 'SORTED_LIST_ACCESS_DATA.keys'
     * buffer at offset 'key_size * new_node_ptr'.
     */
    res = SORTED_LIST_ACCESS_DATA.keys.memwrite(unit,sorted_list_index,key,new_node_ptr * key_size, key_size) ;
    DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 26, exit) ;
  }
  /* if(SOC_DNX_WB_ENGINE_VAR_NONE == init_info_ptr->wb_var_index) */
  {
    res = SORTED_LIST_ACCESS_INFO.data_size.get(unit,sorted_list_index,&data_size) ;
    DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 8, exit) ;
    /*
     * Copy 'data_size' bytes from input 'data' buffer into 'SORTED_LIST_ACCESS_DATA.data'
     * buffer at offset 'data_size * new_node_ptr'.
     */
    res = SORTED_LIST_ACCESS_DATA.data.memwrite(unit,sorted_list_index,data,new_node_ptr * data_size, data_size) ;
    DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 26, exit) ;
  }
  dnx_sand_sorted_list_node_link_set(
    unit,
    sorted_list,
    prev_node,
    new_node_ptr
  ) ;
  dnx_sand_sorted_list_node_link_set(
    unit,
    sorted_list,
    new_node_ptr,
    next_node
  );
  {
    uint8
        is_allocated ;
    uint32
        num_elements_on_indices ;
    res = SORTED_LIST_ACCESS_DATA.indices.is_allocated(unit,sorted_list_index,&is_allocated) ;
    DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 30, exit) ;
    if (is_allocated)
    {
      /*
       * Feature is enabled. Add element on 'indices' array at specified location.
       */
      res = SORTED_LIST_ACCESS_DATA.num_elements_on_indices.get(unit,sorted_list_index,&num_elements_on_indices) ;
      DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 34, exit) ;
      if (num_elements_on_indices == 0)
      {
        /*
         * If the 'indices' array is empty then, obviously, add the first element
         * and ignore 'index_in_indices' (which should be '0').
         */
        res = SORTED_LIST_ACCESS_DATA.indices.set(unit,sorted_list_index,0,new_node_ptr) ;
        DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 38, exit) ;
      }
      else
      {
        index_in_indices = num_elements_on_indices - from_top_index_in_indices - 1 ;
        /*
         * Now 'index_in_indices' is index from the beginning of the array.
         */
        if ((int)index_in_indices < -1)
        {
          /*
           * Error: System requires to write beyond current range of indices.
           */
          DNX_SAND_SET_ERROR_CODE(DNX_SAND_VALUE_ABOVE_MAX_ERR, 52, exit) ;
        }
        else if ((int)index_in_indices < (int)(num_elements_on_indices - 1))
        {
          /*
           * We now need to make space (insert) 'new_node_ptr'. This requires
           * Moving all elements BEYOND 'index_in_indices'. Note that when
           * 'index_in_indices' is '-1' then the new element needs to be inserted as first.
           */
          uint32
            tmp_index,
            tmp_on_indices ;
          uint32
            num_to_copy ;
          num_to_copy = (num_elements_on_indices - 1 - index_in_indices) ;
          tmp_index = num_elements_on_indices - 1 ;
          while (num_to_copy)
          {
            res = SORTED_LIST_ACCESS_DATA.indices.get(unit,sorted_list_index,tmp_index,&tmp_on_indices) ;
            DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 38, exit) ;
            res = SORTED_LIST_ACCESS_DATA.indices.set(unit,sorted_list_index,tmp_index + 1,tmp_on_indices) ;
            DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 38, exit) ;
            num_to_copy-- ;
            tmp_index-- ;
          }
        }
        /*
         * Note that if (index_in_indices == (num_elements_on_indices - 1)) then no 'move'
         * is required.
         */
        res = SORTED_LIST_ACCESS_DATA.indices.set(unit,sorted_list_index,index_in_indices + 1,new_node_ptr) ;
        DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 48, exit) ;
      }
      num_elements_on_indices++ ;
      res = SORTED_LIST_ACCESS_DATA.num_elements_on_indices.set(unit,sorted_list_index,num_elements_on_indices) ;
      DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 56, exit) ;
    }
  }
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in dnx_sand_sorted_list_node_alloc()",0,0);

}

/*********************************************************************
* NAME:
*   dnx_sand_sorted_list_get_iter_begin_or_end
* TYPE:
*   PROC
* DATE:
*   May 19 2015
* FUNCTION:
*   Get the first iterator of the sorted list.
* INPUT:
*   DNX_SAND_IN  int                              unit -
*     Identifier of the device to access.
*   DNX_SAND_IN  DNX_SAND_SORTED_LIST_PTR         sorted_list -
*     Handle to the sorted list.
*   DNX_SAND_IN  int                              get_begin -
*     If non-zero then get iterator at the beginning of the list. Otherwise
*     get iterator at end of list
* REMARKS:
*     None.
* RETURNS:
*   First iterator. If an error is encountered, a negative value is returned.
*********************************************************************/
uint32
  dnx_sand_sorted_list_get_iter_begin_or_end(
    DNX_SAND_IN   int                           unit,
    DNX_SAND_IN   DNX_SAND_SORTED_LIST_PTR      sorted_list,
    DNX_SAND_IN   int                           get_begin
  )
{
  uint32
    res;
  uint32
    sorted_list_index ;
  uint32
    list_size ;

  list_size = (uint32)(-1) ;
  if ((unit < 0) || (unit >= SOC_MAX_NUM_DEVICES))
  { 
    /*
     * If this is an illegal unit identifier, quit
     * with error.
     */
    goto exit ;
  }
  DNX_SAND_SORTED_LIST_CONVERT_HANDLE_TO_SORTEDLIST_INDEX(sorted_list_index,sorted_list) ;
  {
    uint8 bit_val ;
    uint32 max_nof_lists ;
    res = SORTED_LIST_ACCESS.max_nof_lists.get(unit, &max_nof_lists) ;
    if (res != SOC_E_NONE)
    {
      DNX_SAND_FUNC_RESULT_SOC_PRINT(res) ;
      goto exit ;
    }
    if (sorted_list_index >= max_nof_lists)
    {
      /*
       * If sortedlist handle is out of range then quit with error.
       */
      bit_val = 0 ;
    }
    else
    {
      res = SORTED_LIST_ACCESS.occupied_lists.bit_get(unit, (int)sorted_list_index, &bit_val) ;
      if (res != SOC_E_NONE)
      {
        DNX_SAND_FUNC_RESULT_SOC_PRINT(res) ;
        goto exit ;
      }
    }
    if (bit_val == 0)
    {
      /*
       * If sortedlist structure is not indicated as 'occupied' then quit
       * with error.
       */
      goto exit ;
    }
  }
  res = SORTED_LIST_ACCESS_INFO.list_size.get(unit,sorted_list_index,&list_size) ;
  if (res != SOC_E_NONE)
  {
    DNX_SAND_FUNC_RESULT_SOC_PRINT(res) ;
    goto exit ;
  }
  if (get_begin == 0)
  {
    list_size += 1 ;
  }
exit:
  return (list_size) ;
}

#include <soc/dnx/legacy/SAND/Utils/sand_footer.h>
