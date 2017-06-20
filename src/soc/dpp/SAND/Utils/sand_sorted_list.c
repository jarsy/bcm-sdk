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
#define SOC_SAND_SORTED_LIST_HEAD_PLACE_GET(sorted_list_index, head_place, _err1) \
  res = SORTED_LIST_ACCESS_INFO.list_size.get(unit,sorted_list_index,&head_place) ; \
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, _err1, exit) ;
#define SOC_SAND_SORTED_LIST_TAIL_PLACE_GET(sorted_list_index, tail_place, _err1) \
  res = SORTED_LIST_ACCESS_INFO.list_size.get(unit,sorted_list_index,&tail_place) ; \
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, _err1, exit) ; \
  tail_place += 1 ;


/* } */

/*************
*  MACROS   *
*************/
/* { */
#define SORTED_LIST_ACCESS          sw_state_access[unit].dpp.soc.sand.sorted_list  
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
#define SOC_SAND_SORTED_LIST_VERIFY_SORTEDLIST_IS_ACTIVE(_sorted_list_index,_err1,_err2) \
  { \
    uint8 bit_val ; \
    uint32 max_nof_lists ; \
    res = SORTED_LIST_ACCESS.max_nof_lists.get(unit, &max_nof_lists) ; \
    SOC_SAND_SOC_CHECK_FUNC_RESULT(res, _err1, exit) ; \
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
      SOC_SAND_SOC_CHECK_FUNC_RESULT(res, _err1, exit) ; \
    } \
    if (bit_val == 0) \
    { \
      /* \
       * If sortedlist structure is not indicated as 'occupied' then quit \
       * with error. \
       */ \
      SOC_SAND_SET_ERROR_CODE(SOC_SAND_FREE_FAIL, _err2, exit) ; \
    } \
  }
/*
 * Verify specified unit has a legal value. If not, software goes to
 * exit with error code.
 * 
 * Notes:
 *   'exit' is assumed to be defined in the caller's scope.
 */
#define SOC_SAND_SORTED_LIST_VERIFY_UNIT_IS_LEGAL(unit, _err1) \
  if (((int)unit < 0) || ((int)unit >= SOC_MAX_NUM_DEVICES)) \
  { \
    /* \
     * If this is an illegal unit identifier, quit \
     * with error. \
     */ \
    SOC_SAND_SET_ERROR_CODE(SOC_SAND_MAX_NUM_DEVICES_OUT_OF_RANGE_ERR, _err1, exit); \
  }
/*
 * Convert input sorted list handle to index in 'occupied_lists' array.
 * Convert input index in 'occupied_lists' array to sorted list handle.
 * Indices go from 0 -> (occupied_lists - 1)
 * Handles go from 1 -> occupied_lists
 */
#define SOC_SAND_SORTED_LIST_CONVERT_HANDLE_TO_SORTEDLIST_INDEX(_sorted_list_index,_handle) (_sorted_list_index = _handle - 1)
#define SOC_SAND_SORTED_LIST_CONVERT_SORTEDLIST_INDEX_TO_HANDLE(_handle,_sorted_list_index) (_handle = _sorted_list_index + 1)
/* } */

/*************
* TYPE DEFS *
*************/
/* { */
/*
 * the key and data type, used for malloc.
 */
typedef uint8 SOC_SAND_SORTED_LIST_KEY_TYPE;
typedef uint8 SOC_SAND_SORTED_LIST_DATA_TYPE;


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
*   soc_sand_sorted_list_get_tmp_data_ptr_from_handle
* TYPE:
*   PROC
* DATE:
*   May 18 2015
* FUNCTION:
*   Get value of 'tmp_data' pointer (See SOC_SAND_SORTED_LIST_T)
*   from handle.
* INPUT:
*   SOC_SAND_IN  int                             unit -
*     Identifier of the device to access.
*   SOC_SAND_IN  SOC_SAND_SORTED_LIST_PTR        sorted_list -
*     Handle to the sorted list.
*   SOC_SAND_OUT uint8                           **tmp_data_ptr_ptr -
*     This procedure loads pointed memory by the pointer to the 'tmp_data'
*     internal workspace buffer.
* REMARKS:
*   This procedure is exceptional. It is added here so we can use
*   the buffer pointed by 'tmp_data' as a work space whose address
*   is passed to variuos utilities.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
STATIC uint32
  soc_sand_sorted_list_get_tmp_data_ptr_from_handle(
    SOC_SAND_IN  int                          unit,
    SOC_SAND_IN  SOC_SAND_SORTED_LIST_PTR     sorted_list,
    SOC_SAND_OUT uint8                        **tmp_data_ptr_ptr
  )
{
  uint32
    sorted_list_index,
    res;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);
  SOC_SAND_SORTED_LIST_VERIFY_UNIT_IS_LEGAL(unit, 2) ;
  SOC_SAND_SORTED_LIST_CONVERT_HANDLE_TO_SORTEDLIST_INDEX(sorted_list_index,sorted_list) ;
  SOC_SAND_SORTED_LIST_VERIFY_SORTEDLIST_IS_ACTIVE(sorted_list_index,4,6) ;

  *tmp_data_ptr_ptr = sw_state[unit]->dpp.soc.sand.sorted_list->lists_array[sorted_list_index]->list_data.tmp_data ;
exit:
  SOC_SAND_EXIT_AND_SEND_ERROR( "error in soc_sand_sorted_list_get_tmp_data_ptr_from_handle()",0,0);
}

/*********************************************************************
* NAME:
*   soc_sand_sorted_list_get_tmp_key_ptr_from_handle
* TYPE:
*   PROC
* DATE:
*   May 18 2015
* FUNCTION:
*   Get value of 'tmp_key' pointer (See SOC_SAND_SORTED_LIST_T)
*   from handle.
* INPUT:
*   SOC_SAND_IN  int                             unit -
*     Identifier of the device to access.
*   SOC_SAND_IN  SOC_SAND_SORTED_LIST_PTR        sorted_list -
*     Handle to the sorted list.
*   SOC_SAND_OUT uint8                           **tmp_key_ptr_ptr -
*     This procedure loads pointed memory by the pointer to the 'tmp_key'
*     internal workspace buffer.
* REMARKS:
*   This procedure is exceptional. It is added here so we can use
*   the buffer pointed by 'tmp_key' as a work space whose address
*   is passed to variuos utilities.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
STATIC uint32
  soc_sand_sorted_list_get_tmp_key_ptr_from_handle(
    SOC_SAND_IN  int                          unit,
    SOC_SAND_IN  SOC_SAND_SORTED_LIST_PTR     sorted_list,
    SOC_SAND_OUT uint8                        **tmp_key_ptr_ptr
  )
{
  uint32
    sorted_list_index,
    res;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);
  SOC_SAND_SORTED_LIST_VERIFY_UNIT_IS_LEGAL(unit, 2) ;
  SOC_SAND_SORTED_LIST_CONVERT_HANDLE_TO_SORTEDLIST_INDEX(sorted_list_index,sorted_list) ;
  SOC_SAND_SORTED_LIST_VERIFY_SORTEDLIST_IS_ACTIVE(sorted_list_index,4,6) ;

  *tmp_key_ptr_ptr = sw_state[unit]->dpp.soc.sand.sorted_list->lists_array[sorted_list_index]->list_data.tmp_key ;
exit:
  SOC_SAND_EXIT_AND_SEND_ERROR( "error in soc_sand_sorted_list_get_tmp_key_ptr_from_handle()",0,0);
}

/*********************************************************************
* NAME:
*     soc_sand_sorted_list_find_match_entry
* TYPE:
*   PROC
* DATE:
*   May 19 2015
* FUNCTION:
*  lookup in the sorted list for the given key and return the data inserted with
*  the given key.
* INPUT:
*   SOC_SAND_IN  int                             unit -
*     Identifier of the device to access.
*   SOC_SAND_IN  SOC_SAND_SORTED_LIST_PTR        sorted_list -
*     Handle to the sorted list.
*   SOC_SAND_IN  SOC_SAND_SORTED_LIST_KEY*       const key -
*     The key to lookup for
*   SOC_SAND_IN  uint8                    first_empty
*     whether to return the first empty entry .
*   SOC_SAND_OUT  uint32                  *entry -
*     if the key is present in the sorted list then return the entry the key found at,
*     otherwise it returns the place where the key suppose to be.
*   SOC_SAND_IN  uint8                    *found -
*     whether the key was found in the sorted list
*   SOC_SAND_OUT  uint32                  *index_in_indices -
*     Only meaningful if the 'indices' feature is enabled ('indices' element
*     in SOC_SAND_SORTED_LIST_T is allocated).
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/

STATIC uint32
  soc_sand_sorted_list_find_match_entry(
    SOC_SAND_IN int                             unit,
    SOC_SAND_IN SOC_SAND_SORTED_LIST_PTR        sorted_list,
    SOC_SAND_IN SOC_SAND_SORTED_LIST_KEY*       const key,
    SOC_SAND_IN SOC_SAND_SORTED_LIST_DATA*      const data,
    SOC_SAND_IN uint8                           first_match,
    SOC_SAND_OUT  uint8                         *found,
    SOC_SAND_OUT  SOC_SAND_SORTED_LIST_ITER     *prev_node,
    SOC_SAND_OUT  SOC_SAND_SORTED_LIST_ITER     *cur_node,
    SOC_SAND_OUT  uint32                        *index_in_indices
  );

STATIC uint32
  soc_sand_sorted_list_node_alloc(
    SOC_SAND_IN   uint32                       unit,
    SOC_SAND_IN  SOC_SAND_SORTED_LIST_PTR      sorted_list,
    SOC_SAND_IN  SOC_SAND_SORTED_LIST_KEY      *const key,
    SOC_SAND_IN   SOC_SAND_SORTED_LIST_DATA    *const data,
    SOC_SAND_IN   uint32                       prev_node,
    SOC_SAND_IN   uint32                       next_node,
    SOC_SAND_IN   uint32                       index_in_indices,
    SOC_SAND_OUT  uint8                        *found
  );

/*********************************************************************
* NAME:
*     soc_sand_sorted_list_get_next_aux
* TYPE:
*   PROC
* DATE:
*   May 19 2015
* FUNCTION:
*  get the next valid entry (key and data) in the sorted list.
*  start traversing from the place pointed by the given iterator.
* INPUT:
*   SOC_SAND_IN  int                             unit -
*     Identifier of the device to access.
*   SOC_SAND_IN  SOC_SAND_SORTED_LIST_PTR        sorted_list -
*     Handle to the sorted list.
*   SOC_SAND_INOUT  SOC_SAND_SORTED_LIST_ITER    *iter
*     iterator points to the entry to start traverse from.
*   SOC_SAND_OUT  SOC_SAND_SORTED_LIST_KEY       *const key -
*     the sorted list key returned
*   SOC_SAND_OUT  SOC_SAND_SORTED_LIST_DATA      data -
*     the sorted list data returned and associated with the key above.
*   SOC_SAND_INOUT SOC_SAND_SORTED_LIST_ITER     *next_or_prev -
*     This procedure loads pointed memory by the next iterator if
*     'forward' is true or by the previous iterator if 'forward' is
*     false.
* REMARKS:
*     - to start traverse the sorted list from the beginning.
*       use SOC_SAND_SORTED_LIST_ITER_BEGIN(iter)
*     - to check if the iterator get to the end of the table use.
*       use SOC_SAND_SORTED_LIST_ITER_END(unit,iter)
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
STATIC uint32
  soc_sand_sorted_list_get_next_aux(
    SOC_SAND_IN    int                           unit,
    SOC_SAND_IN    SOC_SAND_SORTED_LIST_PTR      sorted_list,
    SOC_SAND_IN    SOC_SAND_SORTED_LIST_ITER     curr,
    SOC_SAND_IN    uint8                         forward,
    SOC_SAND_OUT   SOC_SAND_SORTED_LIST_KEY      *const key,
    SOC_SAND_OUT   SOC_SAND_SORTED_LIST_DATA     *const data,
    SOC_SAND_INOUT SOC_SAND_SORTED_LIST_ITER     *next_or_prev
  );

STATIC uint32
  soc_sand_sorted_list_node_link_set(
    SOC_SAND_IN     int                   unit,
    SOC_SAND_IN  SOC_SAND_SORTED_LIST_PTR sorted_list,
    SOC_SAND_IN  uint32                   node1,
    SOC_SAND_IN  uint32                   node2
  );
/*
 * Compare two input buffers using custom metric:
 * If the first (buffer1) is equal to the second (buffer2)
 *                                        then return zero.
 * If the first (buffer1) is larger than the second (buffer2)
 *                                        then return a positive number.
 * If the first (buffer1) is smaller than the second (buffer2)
 *                                        then return a negative number.
 */
int32
    soc_sand_sorted_list_tcam_cmp_priority(
      SOC_SAND_IN uint8  *buffer1,
      SOC_SAND_IN uint8  *buffer2,
              uint32 size
    )
{
  uint32
    prio_1,
    prio_2;
  ARAD_TCAM_PRIO_LOCATION 
      prio_location_1,
      prio_location_2;

  if ((buffer1 == NULL) || (buffer2 == NULL)) {
      /* Not supposed to compare NULL pointers */
      assert(0);
  }
  /* Key comparison */
  if (size == ARAD_TCAM_DB_LIST_KEY_SIZE) {
      prio_1 = arad_tcam_db_prio_list_priority_value_decode(buffer1);
      prio_2 = arad_tcam_db_prio_list_priority_value_decode(buffer2);
      return (((int32) prio_1) - ((int32) prio_2));
  }
  else if (size == ARAD_TCAM_DB_LIST_DATA_SIZE) { /* Data comparison */
      ARAD_TCAM_PRIO_LOCATION_clear(&prio_location_1);
      ARAD_TCAM_PRIO_LOCATION_clear(&prio_location_2);
      /* buffer1 and buffer2 can't be null. It have been checked above */
      /* coverity[var_deref_model:FALSE] */
      sal_memcpy(&prio_location_1, buffer1, sizeof(ARAD_TCAM_PRIO_LOCATION));
      /* coverity[var_deref_model:FALSE] */
      sal_memcpy(&prio_location_2, buffer2, sizeof(ARAD_TCAM_PRIO_LOCATION));
      return ((prio_location_1.entry_id_first != prio_location_2.entry_id_first)
              || (prio_location_1.entry_id_last != prio_location_2.entry_id_last))? 1 :0;
  } else {
      /* Unknown input formats */
      assert(0);
      return 0;
  }
}

uint32
    soc_sand_sorted_list_default_entry_set(
      SOC_SAND_IN    int prime_handle,
      SOC_SAND_IN    uint32 sec_handle,
      SOC_SAND_INOUT uint8  *buffer,
      SOC_SAND_IN    uint32 offset,
      SOC_SAND_IN    uint32 len,
      SOC_SAND_IN    uint8  *data
    )
{
  sal_memcpy(
    buffer + (offset * len),
    data,
    len
  );
  return SOC_SAND_OK;
}


uint32
    soc_sand_sorted_list_default_entry_get(
      SOC_SAND_IN  int prime_handle,
      SOC_SAND_IN  uint32 sec_handle,
      SOC_SAND_IN  uint8  *buffer,
      SOC_SAND_IN  uint32 offset,
      SOC_SAND_IN  uint32 len,
      SOC_SAND_OUT uint8  *data
    )
{
  sal_memcpy(
    data,
    buffer + (offset * len),
    len
  );
  return SOC_SAND_OK;
}


/************************************************************************/
/*  End of internals                                                    */
/************************************************************************/
/* } */

/*********************************************************************
* NAME:
*   soc_sand_sorted_list_init
* TYPE:
*   PROC
* DATE:
*   May 19 2015
* FUNCTION:
*   Initialize control structure for ALL sorted list instances expected.
* INPUT:
*   SOC_SAND_IN  int unit -
*     Identifier of the device to access.
*   SOC_SAND_IN  uint32 max_nof_lists -
*     Maximal number of sorted lists which can be sustained simultaneously.
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  soc_sand_sorted_list_init(
    SOC_SAND_IN       int                          unit,
    SOC_SAND_IN       uint32                       max_nof_lists
  )
{
  uint32 res ;

  SOC_SAND_INIT_ERROR_DEFINITIONS(SOC_SAND_SORTED_LIST_INIT) ;

  res = SORTED_LIST_ACCESS.alloc(unit);
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 10, exit);

  res = SORTED_LIST_ACCESS.lists_array.ptr_alloc(unit, max_nof_lists);
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 20, exit);

  res = SORTED_LIST_ACCESS.max_nof_lists.set(unit, max_nof_lists);
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 30, exit);

  res = SORTED_LIST_ACCESS.in_use.set(unit, 0);
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 40, exit);

  res = SORTED_LIST_ACCESS.occupied_lists.alloc_bitmap(unit, max_nof_lists);
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 60, exit);

  exit:
    SOC_SAND_EXIT_AND_SEND_ERROR( "error in soc_sand_sorted_list_init()",0,0);
}

/*********************************************************************
* NAME:
*   soc_sand_sorted_list_clear_all_tmps
* TYPE:
*   PROC
* DATE:
*   Aug 02 2015
* FUNCTION:
*   Fill all allocated 'tmp' (sand box) buffers by zeros.
* INPUT:
*   SOC_SAND_IN  int unit -
*     Identifier of the device to access.
* REMARKS:
*   This procedure is to be used at init before 'diff'ing previous sw
*   state buffer with current one. This ensures that such buffers are
*   effectively not 'diff'ed.
* RETURNS:
*   OK or ERROR indication.
*********************************************************************/
uint32
  soc_sand_sorted_list_clear_all_tmps(
    SOC_SAND_IN int unit
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

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);

  SOC_SAND_SORTED_LIST_VERIFY_UNIT_IS_LEGAL(unit, 2) ;

  res = SORTED_LIST_ACCESS.in_use.get(unit, &in_use);
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 4, exit);

  res = SORTED_LIST_ACCESS.max_nof_lists.get(unit, &max_nof_lists);
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 6, exit);

  if (in_use >= max_nof_lists)
  {
    /*
     * If number of occupied bitmap structures is beyond the
     * maximum then quit with error.
     */
    SOC_SAND_SET_ERROR_CODE(SOC_SAND_MALLOC_FAIL, 8, exit);
  }
  /*
   * Find occupied lists (a set bit in 'occupied_lists') and, for each,
   * fill 'tmp' buffers by zeroes.
   *
   * Currently, 'tmp' buffers are:
   *   sw_state[unit]->dpp.soc.sand.sorted_list->lists_array[sorted_list_index]->list_data.tmp_key
   *   sw_state[unit]->dpp.soc.sand.sorted_list->lists_array[sorted_list_index]->list_data.tmp_data
   */
  found = 0 ;
  offset = 0 ;
  for (sorted_list_index = 0 ; sorted_list_index < max_nof_lists ; sorted_list_index++)
  {
    res = SORTED_LIST_ACCESS.occupied_lists.bit_get(unit, sorted_list_index, &bit_val);
    SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 12, exit);
    if (bit_val == 1)
    {
      /*
       * 'sorted_list_index' is now the index of an occupied entry.
       */
      found++ ;
      res = SORTED_LIST_ACCESS_DATA.tmp_key.is_allocated(unit,sorted_list_index,&is_allocated) ;
      SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 14, exit) ;
      if (!is_allocated)
      {
        /*
         * 'tmp_key' buffer must be allocated, at this point.
         */
        SOC_SAND_SET_ERROR_CODE(SOC_E_INTERNAL, 16, exit);
      }
      res = SORTED_LIST_ACCESS_DATA.tmp_data.is_allocated(unit,sorted_list_index,&is_allocated) ;
      SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 24, exit) ;
      if (!is_allocated)
      {
        /*
         * 'tmp_data' buffer must be allocated, at this point.
         */
        SOC_SAND_SET_ERROR_CODE(SOC_E_INTERNAL, 18, exit);
      }
      /*
       * Clear 'tmp_key'
       */
      res = SORTED_LIST_ACCESS_INFO.key_size.get(unit,sorted_list_index,&key_size) ;
      SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 20, exit);
      res = SORTED_LIST_ACCESS_DATA.tmp_key.memset(unit,sorted_list_index,offset,key_size,0) ;
      SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 22, exit) ;
      /*
       * Clear 'tmp_data'
       */
      res = SORTED_LIST_ACCESS_INFO.data_size.get(unit,sorted_list_index,&data_size) ;
      SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 24, exit);
      res = SORTED_LIST_ACCESS_DATA.ptr_size.get(unit,sorted_list_index,&ptr_size) ;
      SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 26, exit) ;
      tmp_size = SOC_SAND_MAX(ptr_size,data_size) ;
      res = SORTED_LIST_ACCESS_DATA.tmp_data.memset(unit,sorted_list_index,offset,tmp_size,0) ;
      SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 28, exit) ;
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
  SOC_SAND_EXIT_AND_SEND_ERROR( "error in soc_sand_sorted_list_clear_all_tmps()",0,0);
}

uint32
  soc_sand_sorted_list_create(
    SOC_SAND_IN     int                             unit,
    SOC_SAND_INOUT  SOC_SAND_SORTED_LIST_PTR        *sorted_list_ptr,
    SOC_SAND_IN     SOC_SAND_SORTED_LIST_INIT_INFO  init_info
  )
{
  SOC_SAND_OCC_BM_INIT_INFO
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
  SOC_SAND_SORTED_LIST_PTR
    sorted_list ;
  SOC_SAND_OCC_BM_PTR
    memory_use ;
  int32
    offset ;

  SOC_SAND_INIT_ERROR_DEFINITIONS(SOC_SAND_SORTED_LIST_CREATE) ;

  SOC_SAND_SORTED_LIST_VERIFY_UNIT_IS_LEGAL(unit, 2) ;
  SOC_SAND_CHECK_NULL_INPUT(sorted_list_ptr);

  res = SORTED_LIST_ACCESS.in_use.get(unit, &in_use);
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 4, exit);

  res = SORTED_LIST_ACCESS.max_nof_lists.get(unit, &max_nof_lists);
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 6, exit);

  if (in_use >= max_nof_lists)
  {
    /*
     * If number of occupied bitmap structures is beyond the
     * maximum then quit with error.
     */
    SOC_SAND_SET_ERROR_CODE(SOC_SAND_MALLOC_FAIL, 8, exit);
  }
  /*
   * Increment number of 'in_use' to cover the one we now intend to capture.
   */
  res = SORTED_LIST_ACCESS.in_use.set(unit, (in_use + 1));
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 10, exit);
  /*
   * Find a free list (a cleared bit in 'occupied_lists'). At this point,
   * there must be one.
   */
  found = 0 ;
  for (sorted_list_index = 0 ; sorted_list_index < max_nof_lists ; sorted_list_index++)
  {
    res = SORTED_LIST_ACCESS.occupied_lists.bit_get(unit, sorted_list_index, &bit_val);
    SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 12, exit);
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
    SOC_SAND_SET_ERROR_CODE(SOC_SAND_MALLOC_FAIL, 14, exit);
  }
  res = SORTED_LIST_ACCESS.occupied_lists.bit_set(unit, sorted_list_index);
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 16, exit);

  res = SORTED_LIST_ACCESS.lists_array.alloc(unit, sorted_list_index);
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 18, exit);
  /*
   * Note that legal handles start at '1', not at '0'.
   */
  SOC_SAND_SORTED_LIST_CONVERT_SORTEDLIST_INDEX_TO_HANDLE(sorted_list,sorted_list_index) ;
  /*
   * Set output of this procedure.
   */
  *sorted_list_ptr = sorted_list;

  soc_sand_SAND_SORTED_LIST_INFO_clear(unit,sorted_list) ;
  res = SORTED_LIST_ACCESS_INFO.prime_handle.set(unit, sorted_list_index, init_info.prime_handle) ;
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 22, exit);

  res = SORTED_LIST_ACCESS_INFO.sec_handle.set(unit,sorted_list_index,init_info.sec_handle) ;
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 24, exit);
  res = SORTED_LIST_ACCESS_INFO.list_size.set(unit,sorted_list_index,init_info.list_size) ;
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 26, exit);
  res = SORTED_LIST_ACCESS_INFO.key_size.set(unit,sorted_list_index,init_info.key_size) ;
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 28, exit);
  res = SORTED_LIST_ACCESS_INFO.data_size.set(unit,sorted_list_index,init_info.data_size) ;
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 30, exit);
  if (init_info.get_entry_fun != NULL || init_info.set_entry_fun != NULL )
  {
    SOC_SAND_SET_ERROR_CODE(SOC_SAND_VALUE_OUT_OF_RANGE_ERR, 31, exit);
  }
  res = SORTED_LIST_ACCESS_INFO.get_entry_fun.set(unit,sorted_list_index,init_info.get_entry_fun) ;
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 32, exit);
  res = SORTED_LIST_ACCESS_INFO.set_entry_fun.set(unit,sorted_list_index,init_info.set_entry_fun) ;
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 34, exit);
  res = SORTED_LIST_ACCESS_INFO.cmp_func_type.set(unit,sorted_list_index,init_info.cmp_func_type) ;
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 35, exit);

  res = SORTED_LIST_ACCESS_INFO.key_size.get(unit,sorted_list_index,&key_size) ;
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 36, exit);
  res = SORTED_LIST_ACCESS_INFO.list_size.get(unit,sorted_list_index,&list_size) ;
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 38, exit);
  res = SORTED_LIST_ACCESS_INFO.data_size.get(unit,sorted_list_index,&data_size) ;
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 40, exit);

  if (list_size == 0 || key_size == 0 || data_size == 0)
  {
    SOC_SAND_SET_ERROR_CODE(SOC_SAND_VALUE_OUT_OF_RANGE_ERR, 42, exit);
  }
  /*
   * calculate the size of pointers (list head and next) according to table size.
   */
  ptr_size = (soc_sand_log2_round_up(list_size + 2) + (SOC_SAND_NOF_BITS_IN_BYTE - 1)) / SOC_SAND_NOF_BITS_IN_BYTE;
  null_ptr = list_size + 1;
  res = SORTED_LIST_ACCESS_DATA.ptr_size.set(unit,sorted_list_index,ptr_size) ;
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 44, exit) ;
  res = SORTED_LIST_ACCESS_DATA.null_ptr.set(unit,sorted_list_index,null_ptr) ;
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 46, exit) ;

  tmp_size = SOC_SAND_MAX(ptr_size,data_size) ;
  /*
   * Allocate the temps buffer.
   */
  res = SORTED_LIST_ACCESS_DATA.tmp_data.alloc(unit,sorted_list_index,tmp_size) ;
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 48, exit) ;
  res = SORTED_LIST_ACCESS_DATA.tmp_key.alloc(unit,sorted_list_index,key_size) ;
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 50, exit) ;
  /*
   * allocate buffer for keys
   */
  offset = 0 ;
  res = SORTED_LIST_ACCESS_DATA.keys.alloc(unit,sorted_list_index,list_size * key_size) ;
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 52, exit) ;
  res = SORTED_LIST_ACCESS_DATA.keys.memset(unit,sorted_list_index,offset,list_size * key_size,0) ;
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 54, exit) ;
  /*
   * allocate buffer for next array (to build the linked list) one additional for head.
   * which is the last in the next pointers array.
   */
  res = SORTED_LIST_ACCESS_DATA.next.alloc(unit,sorted_list_index,(list_size + 2) * ptr_size) ;
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 56, exit) ;
  res = SORTED_LIST_ACCESS_DATA.next.memset(unit,sorted_list_index,offset,(list_size + 2) * ptr_size,0xFF) ;
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 58, exit) ;
  /*
   * allocate buffer for prev array (to build the linked list)
   */
  res = SORTED_LIST_ACCESS_DATA.prev.alloc(unit,sorted_list_index,(list_size + 2) * ptr_size) ;
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 56, exit) ;
  res = SORTED_LIST_ACCESS_DATA.prev.memset(unit,sorted_list_index,offset,(list_size + 2) * ptr_size,0xFF) ;
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 58, exit) ;
  /*
   * allocate buffer for the data array
   */
  res = SORTED_LIST_ACCESS_DATA.data.alloc(unit,sorted_list_index,list_size * data_size) ;
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 60, exit) ;
  res = SORTED_LIST_ACCESS_DATA.data.memset(unit,sorted_list_index,offset,list_size * data_size,0) ;
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 62, exit) ;
  {
    uint32
      head_place,
      tail_place ;
    /*
     * connect the head with the tail.
     */
    SOC_SAND_SORTED_LIST_HEAD_PLACE_GET(sorted_list_index,head_place,64) ;
    SOC_SAND_SORTED_LIST_TAIL_PLACE_GET(sorted_list_index,tail_place,66) ;
    res =
      soc_sand_sorted_list_node_link_set(
        unit,
        sorted_list,
        head_place,
        tail_place
      );
    SOC_SAND_CHECK_FUNC_RESULT(res, 70, exit);
  }
  /*
   * initialize the data to be mapped to
   */
  soc_sand_SAND_OCC_BM_INIT_INFO_clear(&btmp_init_info);
  btmp_init_info.size = list_size ;

  /* if(SOC_DPP_WB_ENGINE_VAR_NONE != init_info_ptr->wb_var_index) */

  res = soc_sand_occ_bm_create(
          unit,
          &btmp_init_info,
          &memory_use
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 80, exit);
  res = SORTED_LIST_ACCESS_DATA.memory_use.set(unit,sorted_list_index,memory_use) ;
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 82, exit) ;
#if ENHANCED_SORTED_LIST_SEARCH
/* { */
  {
    /*
     * Initialize 'indices' array. Currently, we add this feature to all sorted lists!
     */
    res = SORTED_LIST_ACCESS_DATA.indices.alloc(unit,sorted_list_index,list_size) ;
    SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 84, exit) ;
    res = SORTED_LIST_ACCESS_DATA.num_elements_on_indices.set(unit,sorted_list_index,0) ;
    SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 88, exit) ;
  }
/* } */
#endif
exit:
  SOC_SAND_EXIT_AND_SEND_ERROR( "error in soc_sand_sorted_list_create()",0,0);
}

uint32
  soc_sand_sorted_list_clear(
    SOC_SAND_IN     int                           unit,
    SOC_SAND_IN     SOC_SAND_SORTED_LIST_PTR      sorted_list
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
  SOC_SAND_OCC_BM_PTR
    memory_use ;
  int32
    offset ;

  SOC_SAND_INIT_ERROR_DEFINITIONS(SOC_SAND_HASH_TABLE_CREATE);
  SOC_SAND_SORTED_LIST_VERIFY_UNIT_IS_LEGAL(unit, 2) ;
  SOC_SAND_SORTED_LIST_CONVERT_HANDLE_TO_SORTEDLIST_INDEX(sorted_list_index,sorted_list) ;
  SOC_SAND_SORTED_LIST_VERIFY_SORTEDLIST_IS_ACTIVE(sorted_list_index,4,6) ;

  /* if(SOC_DPP_WB_ENGINE_VAR_NONE == init_info_ptr->wb_var_index)  */
  {
    offset = 0 ;
    res = SORTED_LIST_ACCESS_DATA.ptr_size.get(unit,sorted_list_index,&ptr_size) ;
    SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 12, exit) ;
    res = SORTED_LIST_ACCESS_INFO.key_size.get(unit,sorted_list_index,&key_size) ;
    SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 14, exit);
    res = SORTED_LIST_ACCESS_INFO.list_size.get(unit,sorted_list_index,&list_size) ;
    SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 16, exit);
    res = SORTED_LIST_ACCESS_INFO.data_size.get(unit,sorted_list_index,&data_size) ;
    SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 18, exit);

    res = SORTED_LIST_ACCESS_DATA.keys.memset(unit,sorted_list_index,offset,list_size * key_size,0x00) ;
    SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 20, exit) ;

    res = SORTED_LIST_ACCESS_DATA.next.memset(unit,sorted_list_index,offset,(list_size + 2) * ptr_size,0xFF) ;
    SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 22, exit) ;

    res = SORTED_LIST_ACCESS_DATA.prev.memset(unit,sorted_list_index,offset,(list_size + 2) * ptr_size,0xFF) ;
    SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 24, exit) ;

    res = SORTED_LIST_ACCESS_DATA.data.memset(unit,sorted_list_index,offset,list_size * data_size,0x00) ;
    SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 26, exit) ;
  } 
  { 
    /*
     * connect the head with the tail.
     */
    uint32
      head_place,
      tail_place ;
    SOC_SAND_SORTED_LIST_HEAD_PLACE_GET(sorted_list_index,head_place,54) ;
    SOC_SAND_SORTED_LIST_TAIL_PLACE_GET(sorted_list_index,tail_place,56) ;
    res = soc_sand_sorted_list_node_link_set(
          unit,
          sorted_list,
          head_place,
          tail_place
        );
    SOC_SAND_CHECK_FUNC_RESULT(res, 30, exit) ;
  }

  res = SORTED_LIST_ACCESS_DATA.memory_use.get(unit,sorted_list_index,&memory_use) ;
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 32, exit) ;
  res = soc_sand_occ_bm_clear(
          unit,
          memory_use
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 36, exit);
  {
    /*
     * Initialize 'indices' array. Currently, we add this feature to all sorted lists!
     */
    res = SORTED_LIST_ACCESS_DATA.num_elements_on_indices.set(unit,sorted_list_index,0) ;
    SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 40, exit) ;
  }

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR( "error in soc_sand_sorted_list_clear()",0,0);
}

uint32
  soc_sand_sorted_list_destroy(
    SOC_SAND_IN     int                         unit,
    SOC_SAND_IN     SOC_SAND_SORTED_LIST_PTR    sorted_list
    )
{
  uint32
    res,
    sorted_list_index ;
  SOC_SAND_OCC_BM_PTR
    memory_use ;
  uint8
    bit_val ;
  uint32
    in_use ;
  uint8
    is_allocated ;

  SOC_SAND_INIT_ERROR_DEFINITIONS(SOC_SAND_SORTED_LIST_DESTROY);
  SOC_SAND_SORTED_LIST_VERIFY_UNIT_IS_LEGAL(unit, 2) ;
  SOC_SAND_SORTED_LIST_CONVERT_HANDLE_TO_SORTEDLIST_INDEX(sorted_list_index,sorted_list) ;
  SOC_SAND_SORTED_LIST_VERIFY_SORTEDLIST_IS_ACTIVE(sorted_list_index,4,6) ;
  /*
   * First, mark this sorted list as 'released'
   */
  res = SORTED_LIST_ACCESS.in_use.get(unit, &in_use);
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 4, exit);
  if ((int)in_use < 0)
  {
    /*
     * If number of occupied sortedlist structures goes below zero then quit
     * with error.
     */
    SOC_SAND_SET_ERROR_CODE(SOC_SAND_FREE_FAIL, 8, exit);
  }
  /*
   * Decrement number of 'in_use' to cover the one we now intend to release.
   */
  res = SORTED_LIST_ACCESS.in_use.set(unit, (in_use - 1));
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 10, exit) ;
  /*
   * Mark specific sorted list as 'not occupied'
   */
  res = SORTED_LIST_ACCESS.occupied_lists.bit_get(unit, sorted_list_index, &bit_val);
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 12, exit);
  if (bit_val == 0)
  {
    /*
     * If sorted list structure is not indicated as 'occupied' then quit
     * with error.
     */
    SOC_SAND_SET_ERROR_CODE(SOC_SAND_FREE_FAIL, 14, exit) ;
  }
  res = SORTED_LIST_ACCESS.occupied_lists.bit_clear(unit, sorted_list_index);
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 14, exit);
  /*
   * Free the temps buffer.
   */
  res = SORTED_LIST_ACCESS_DATA.tmp_data.free(unit,sorted_list_index) ;
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 18, exit) ;
  res = SORTED_LIST_ACCESS_DATA.tmp_key.free(unit,sorted_list_index) ;
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 20, exit) ;

  res = SORTED_LIST_ACCESS_DATA.next.free(unit,sorted_list_index) ;
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 24, exit) ;
  res = SORTED_LIST_ACCESS_DATA.keys.free(unit,sorted_list_index) ;
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 28, exit) ;

  res = SORTED_LIST_ACCESS_DATA.prev.free(unit,sorted_list_index) ;
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 32, exit) ;
  res = SORTED_LIST_ACCESS_DATA.data.free(unit,sorted_list_index) ;
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 36, exit) ;

  res = SORTED_LIST_ACCESS_DATA.indices.is_allocated(unit,sorted_list_index,&is_allocated) ;
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 38, exit) ;
  if (is_allocated)
  {
    /*
     * Free 'indices' array if it has been allocated
     */
    res = SORTED_LIST_ACCESS_DATA.indices.free(unit,sorted_list_index) ;
    SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 40, exit) ;
    res = SORTED_LIST_ACCESS_DATA.num_elements_on_indices.set(unit,sorted_list_index,0) ;
    SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 42, exit) ;
  }
  res = SORTED_LIST_ACCESS_DATA.memory_use.get(unit,sorted_list_index,&memory_use) ;
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 44, exit) ;
  res = soc_sand_occ_bm_destroy(
    unit,
    memory_use
    );
  res = SORTED_LIST_ACCESS.lists_array.free(unit, sorted_list_index);
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 48, exit);

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR( "error in soc_sand_sorted_list_destroy()",0,0);
}

uint32
  soc_sand_sorted_list_entry_add(
    SOC_SAND_IN     int                           unit,
    SOC_SAND_IN     SOC_SAND_SORTED_LIST_PTR      sorted_list,
    SOC_SAND_IN     SOC_SAND_SORTED_LIST_KEY      *const key,
    SOC_SAND_IN     SOC_SAND_SORTED_LIST_DATA     *const data,
    SOC_SAND_OUT    uint8                         *success
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

  SOC_SAND_INIT_ERROR_DEFINITIONS(SOC_SAND_SORTED_LIST_ENTRY_ADD);
  SOC_SAND_SORTED_LIST_VERIFY_UNIT_IS_LEGAL(unit, 2) ;
  SOC_SAND_SORTED_LIST_CONVERT_HANDLE_TO_SORTEDLIST_INDEX(sorted_list_index,sorted_list) ;
  SOC_SAND_SORTED_LIST_VERIFY_SORTEDLIST_IS_ACTIVE(sorted_list_index,4,6) ;

  SOC_SAND_CHECK_NULL_INPUT(key);
  SOC_SAND_CHECK_NULL_INPUT(data);
  SOC_SAND_CHECK_NULL_INPUT(success);

  res = soc_sand_sorted_list_find_match_entry(
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
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);
  if (found)
  {
    SOC_SAND_SET_ERROR_CODE(SOC_SAND_SORTED_LIST_KEY_DATA_ALREADY_EXIST_ERR, 20, exit);
  }
  /*
   * Allocate new node.
   */
  res = soc_sand_sorted_list_node_alloc(
          unit,
          sorted_list,
          key,
          data,
          prev_node,
          curr_node,
          from_top_index_in_indices,
          success
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 30, exit) ;

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR( "error in soc_sand_sorted_list_entry_add()",0,0);
}

uint32
  soc_sand_sorted_list_entry_add_by_iter(
    SOC_SAND_IN     int                           unit,
    SOC_SAND_IN     SOC_SAND_SORTED_LIST_PTR      sorted_list,
    SOC_SAND_IN     SOC_SAND_SORTED_LIST_ITER     pos,
    SOC_SAND_IN     uint8                         before,
    SOC_SAND_IN     SOC_SAND_SORTED_LIST_KEY      *const key,
    SOC_SAND_IN     SOC_SAND_SORTED_LIST_DATA     *const data,
    SOC_SAND_OUT    uint8                         *success
  )
{
  SOC_SAND_SORTED_LIST_ITER
    prev_node,
    next_node;
  SOC_SAND_SORTED_LIST_KEY_CMP 
    cmp_key_fun;
  uint32
    key_size,
    res;
  uint32
    sorted_list_index ;
  SOC_SAND_SORTED_LIST_CMP_FUNC_TYPE
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

  SOC_SAND_INIT_ERROR_DEFINITIONS(SOC_SAND_SORTED_LIST_ENTRY_ADD_BY_ITER);
  SOC_SAND_SORTED_LIST_VERIFY_UNIT_IS_LEGAL(unit, 2) ;
  SOC_SAND_SORTED_LIST_CONVERT_HANDLE_TO_SORTEDLIST_INDEX(sorted_list_index,sorted_list) ;
  SOC_SAND_SORTED_LIST_VERIFY_SORTEDLIST_IS_ACTIVE(sorted_list_index,4,6) ;


  res = SORTED_LIST_ACCESS_INFO.cmp_func_type.get(unit,sorted_list_index,&cmp_func_type) ;
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 8, exit);

  if (cmp_func_type == SOC_SAND_SORTED_LIST_CMP_FUNC_TYPE_TCAM) {
      cmp_key_fun = soc_sand_sorted_list_tcam_cmp_priority ;
  }
  else {
      cmp_key_fun = (SOC_SAND_SORTED_LIST_KEY_CMP)soc_sand_os_memcmp;
  }
  /*
   * Verify we are not adding after the end or before the beginning.
   */
  if (
      (before &&  pos == SOC_SAND_SORTED_LIST_ITER_BEGIN(unit,sorted_list)) ||
      ((!before && pos == SOC_SAND_SORTED_LIST_ITER_END(unit,sorted_list)))
     )
  {
    SOC_SAND_SET_ERROR_CODE(SOC_SAND_SORTED_LIST_ILLEGAL_ITER_ERR, 10, exit);
  }
 /*
  * Verify that this addition keeps the order.
  */
  prev_node = pos;
  next_node = pos;

  if (before)
  {
    res =
      soc_sand_sorted_list_get_next_aux(
        unit,
        sorted_list,
        pos,
        FALSE,
        NULL,
        NULL,&prev_node
      );
    SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 10, exit);
  }
  else
  {
    res =
      soc_sand_sorted_list_get_next_aux(
        unit,
        sorted_list,
        pos,
        TRUE,
        NULL,
        NULL,&next_node
      );
    SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 12, exit);
  }
  res = soc_sand_sorted_list_get_tmp_key_ptr_from_handle(unit,sorted_list,&tmp_key_ptr) ;
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 14, exit);
  res = SORTED_LIST_ACCESS_INFO.key_size.get(unit,sorted_list_index,&key_size) ;
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 16, exit) ;
  if (prev_node != SOC_SAND_SORTED_LIST_ITER_END(unit,sorted_list) &&  prev_node != SOC_SAND_SORTED_LIST_ITER_BEGIN(unit,sorted_list))
  {
    res = soc_sand_sorted_list_entry_value(
            unit,
            sorted_list,
            prev_node,
            tmp_key_ptr,
            NULL
          );
    SOC_SAND_CHECK_FUNC_RESULT(res, 18, exit) ;

    if (cmp_key_fun(tmp_key_ptr, key, key_size) > 0 )
    {
      SOC_SAND_SET_ERROR_CODE(SOC_SAND_SORTED_LIST_ILLEGAL_ITER_ERR, 18, exit);
    }
  }
  if (next_node != SOC_SAND_SORTED_LIST_ITER_END(unit,sorted_list) &&  next_node != SOC_SAND_SORTED_LIST_ITER_BEGIN(unit,sorted_list))
  {
    res = soc_sand_sorted_list_entry_value(
            unit,
            sorted_list,
            next_node,
            tmp_key_ptr,
            NULL
          );
    SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);

    if (cmp_key_fun(key, tmp_key_ptr, key_size) > 0 )
    {
      SOC_SAND_SET_ERROR_CODE(SOC_SAND_SORTED_LIST_ILLEGAL_ITER_ERR, 22, exit);
    }
  }
  {
    uint32
        num_elements_on_indices ;
    res = SORTED_LIST_ACCESS_DATA.indices.is_allocated(unit,sorted_list_index,&is_allocated) ;
    SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 24, exit) ;
    if (is_allocated)
    {
      /*
       * Feature is enabled. Find location of element on 'indices' array.
       */
      res = SORTED_LIST_ACCESS_DATA.num_elements_on_indices.get(unit,sorted_list_index,&num_elements_on_indices) ;
      SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 28, exit) ;
      if ((next_node == SOC_SAND_SORTED_LIST_ITER_END(unit,sorted_list)) || (next_node == SOC_SAND_SORTED_LIST_ITER_BEGIN(unit,sorted_list)))
      {
        from_top_index_in_indices = 0 ;
      }
      else if (prev_node == SOC_SAND_SORTED_LIST_ITER_END(unit,sorted_list) ||  prev_node == SOC_SAND_SORTED_LIST_ITER_BEGIN(unit,sorted_list))
      {
        from_top_index_in_indices = num_elements_on_indices - 1 ;
      }
      else
      {
        /*
         * Get the  index on 'indices' corresponding to previous element sorted list.
         * Use it to add entry into 'indices'.
         */
        res = soc_sand_sorted_list_get_index_from_iter(unit,sorted_list,prev_node,&from_top_index_in_indices) ;
        SOC_SAND_CHECK_FUNC_RESULT(res, 32, exit) ;
        from_top_index_in_indices = num_elements_on_indices - 1 - from_top_index_in_indices ;
      }
    }
  }
  res = soc_sand_sorted_list_node_alloc(
          unit,
          sorted_list,
          key,
          data,
          prev_node,
          next_node,
          from_top_index_in_indices,
          success
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 24, exit);
exit:
  SOC_SAND_EXIT_AND_SEND_ERROR( "error in soc_sand_sorted_list_entry_add_by_iter()",0,0);
}

uint32
  soc_sand_sorted_list_entry_update(
    SOC_SAND_IN     int                           unit,
    SOC_SAND_IN     SOC_SAND_SORTED_LIST_PTR      sorted_list,
    SOC_SAND_IN     SOC_SAND_SORTED_LIST_ITER     iter,
    SOC_SAND_IN     SOC_SAND_SORTED_LIST_KEY      *const key,
    SOC_SAND_IN     SOC_SAND_SORTED_LIST_DATA     *const data
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

  SOC_SAND_INIT_ERROR_DEFINITIONS(SOC_SAND_SORTED_LIST_ENTRY_UPDATE);
  SOC_SAND_SORTED_LIST_VERIFY_UNIT_IS_LEGAL(unit, 2) ;
  SOC_SAND_SORTED_LIST_CONVERT_HANDLE_TO_SORTEDLIST_INDEX(sorted_list_index,sorted_list) ;
  SOC_SAND_SORTED_LIST_VERIFY_SORTEDLIST_IS_ACTIVE(sorted_list_index,4,6) ;

  SOC_SAND_CHECK_NULL_INPUT(data);
  SOC_SAND_CHECK_NULL_INPUT(key);

  res = SORTED_LIST_ACCESS_INFO.list_size.get(unit,sorted_list_index,&list_size) ;
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 8, exit);
  if (iter == SOC_SAND_SORTED_LIST_NULL || iter >= list_size)
  {
    SOC_SAND_SET_ERROR_CODE(SOC_SAND_VALUE_OUT_OF_RANGE_ERR, 10, exit);
  }
  /*
   * Check to see if the entry exists
   */
  if (data)
  {
    res = SORTED_LIST_ACCESS_INFO.data_size.get(unit,sorted_list_index,&data_size) ;
    SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 18, exit);
    /* if(SOC_DPP_WB_ENGINE_VAR_NONE == init_info_ptr->wb_var_index) */
    {
      /*
       * Copy 'data_size' bytes from input 'data' buffer into 'SORTED_LIST_ACCESS_DATA.data'
       * buffer at offset 'data_size * iter'.
       */
      res = SORTED_LIST_ACCESS_DATA.data.memwrite(unit,sorted_list_index,data,iter * data_size, data_size) ;
      SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 26, exit) ;
    }
  }
  if (key)
  {
    res = SORTED_LIST_ACCESS_INFO.key_size.get(unit,sorted_list_index,&key_size) ;
    SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 28, exit);
    /* if(SOC_DPP_WB_ENGINE_VAR_NONE == init_info_ptr->wb_var_index) */
    {
      /*
       * Copy 'key_size' bytes from input 'key' buffer into 'SORTED_LIST_ACCESS_DATA.keys'
       * buffer at offset 'key_size * iter'.
       */
      res = SORTED_LIST_ACCESS_DATA.keys.memwrite(unit,sorted_list_index,key,iter * key_size, key_size) ;
      SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 26, exit) ;
    }
  }
exit:
  SOC_SAND_EXIT_AND_SEND_ERROR( "error in soc_sand_sorted_list_entry_update()",0,0);
}

uint32
  soc_sand_sorted_list_entry_get_data(
    SOC_SAND_IN     int                              unit,
    SOC_SAND_IN     SOC_SAND_SORTED_LIST_PTR         sorted_list,
    SOC_SAND_IN     SOC_SAND_SORTED_LIST_ITER        iter,
    SOC_SAND_OUT    SOC_SAND_SORTED_LIST_DATA        *const data
  )
{
  uint32
    res;
  uint32
    sorted_list_index ;
  uint32
    data_size,
    list_size ;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);
  SOC_SAND_SORTED_LIST_VERIFY_UNIT_IS_LEGAL(unit, 2) ;
  SOC_SAND_SORTED_LIST_CONVERT_HANDLE_TO_SORTEDLIST_INDEX(sorted_list_index,sorted_list) ;
  SOC_SAND_SORTED_LIST_VERIFY_SORTEDLIST_IS_ACTIVE(sorted_list_index,4,6) ;

  SOC_SAND_CHECK_NULL_INPUT(data);

  res = SORTED_LIST_ACCESS_INFO.list_size.get(unit,sorted_list_index,&list_size) ;
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 8, exit);
  if (iter == SOC_SAND_SORTED_LIST_ITER_END(unit,sorted_list) || iter >= list_size)
  {
    SOC_SAND_SET_ERROR_CODE(SOC_SAND_VALUE_OUT_OF_RANGE_ERR, 10, exit);
  }
  res = SORTED_LIST_ACCESS_INFO.data_size.get(unit,sorted_list_index,&data_size) ;
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 18, exit);
  /*
   * Copy 'data_size' bytes from 'SORTED_LIST_ACCESS_DATA.data' buffer at offset
   * 'data_size * iter' into input 'data' buffer.
   */
  res = SORTED_LIST_ACCESS_DATA.data.memread(unit,sorted_list_index,data,iter * data_size, data_size) ;
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 26, exit) ;
exit:
  SOC_SAND_EXIT_AND_SEND_ERROR( "error in soc_sand_sorted_list_entry_get_data()",0,0);
}

uint32
  soc_sand_sorted_list_entry_remove_by_iter(
    SOC_SAND_IN     int                             unit,
    SOC_SAND_IN     SOC_SAND_SORTED_LIST_PTR        sorted_list,
    SOC_SAND_IN     SOC_SAND_SORTED_LIST_ITER       iter
  )
{
  SOC_SAND_SORTED_LIST_ITER
    prev,
    next;
  uint32
    res;
  uint32
    sorted_list_index,
    null_ptr,
    ptr_size ;
  SOC_SAND_OCC_BM_PTR
    memory_use ;
  uint32
    index_in_indices ;
  uint8
    is_indices_enabled ;

  SOC_SAND_INIT_ERROR_DEFINITIONS(SOC_SAND_SORTED_LIST_ENTRY_REMOVE_BY_ITER);
  SOC_SAND_SORTED_LIST_VERIFY_UNIT_IS_LEGAL(unit, 2) ;
  SOC_SAND_SORTED_LIST_CONVERT_HANDLE_TO_SORTEDLIST_INDEX(sorted_list_index,sorted_list) ;
  SOC_SAND_SORTED_LIST_VERIFY_SORTEDLIST_IS_ACTIVE(sorted_list_index,4,6) ;

  res = SORTED_LIST_ACCESS_DATA.indices.is_allocated(unit,sorted_list_index,&is_indices_enabled) ;
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 8, exit) ;

  if (is_indices_enabled)
  {
    /*
     * Get the corresponding index on 'indices' before removing from sorted list.
     * Use it, at end of procedure, to remove entry from 'indices'.
     */
    res = soc_sand_sorted_list_get_index_from_iter(unit,sorted_list,iter,&index_in_indices) ;
    SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit) ;
  }
  res = soc_sand_sorted_list_get_next_aux(
           unit,
           sorted_list,
           iter,
           TRUE,
           NULL,
           NULL,&next
         ) ;
  SOC_SAND_CHECK_FUNC_RESULT(res, 12, exit);
  res = soc_sand_sorted_list_get_next_aux(
           unit,
           sorted_list,
           iter,
           FALSE,
           NULL,
           NULL,&prev
         ) ;
  SOC_SAND_CHECK_FUNC_RESULT(res, 14, exit);
  res = soc_sand_sorted_list_node_link_set(
            unit,
            sorted_list,
            prev,
            next
          ) ;
  SOC_SAND_CHECK_FUNC_RESULT(res, 16, exit);

  res = SORTED_LIST_ACCESS_DATA.null_ptr.get(unit,sorted_list_index,&null_ptr) ;
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 20, exit) ;
  res = SORTED_LIST_ACCESS_DATA.ptr_size.get(unit,sorted_list_index,&ptr_size) ;
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 22, exit) ;

  /* if(SOC_DPP_WB_ENGINE_VAR_NONE == init_info_ptr->wb_var_index) */
  {
    /*
     * Copy 'ptr_size' bytes from '&null_ptr' (treated as a small buffer) into
     * 'SORTED_LIST_ACCESS_DATA.next' buffer at offset 'ptr_size * iter'.
     */
    res = SORTED_LIST_ACCESS_DATA.next.memwrite(unit,sorted_list_index,(uint8 *)&null_ptr,iter * ptr_size, ptr_size) ;
    SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 24, exit) ;
  }
  {
    /*
     * Copy 'ptr_size' bytes from '&null_ptr' (treated as a small buffer) into
     * 'SORTED_LIST_ACCESS_DATA.prev' buffer at offset 'ptr_size * iter'.
     */
    res = SORTED_LIST_ACCESS_DATA.prev.memwrite(unit,sorted_list_index,(uint8 *)&null_ptr,iter * ptr_size, ptr_size) ;
    SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 26, exit) ;
  }
  res = SORTED_LIST_ACCESS_DATA.memory_use.get(unit,sorted_list_index,&memory_use) ;
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 28, exit) ;
  res = soc_sand_occ_bm_occup_status_set(
          unit,
          memory_use,
          iter,
          FALSE
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 60, exit);
  if (is_indices_enabled)
  {
    uint32
      ii,
      num_elements_on_indices,
      node_ptr ;

    res = SORTED_LIST_ACCESS_DATA.num_elements_on_indices.get(unit,sorted_list_index,&num_elements_on_indices) ;
    SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 34, exit) ;
    if (index_in_indices < (num_elements_on_indices - 1))
    {
      for (ii = index_in_indices + 1 ; ii < num_elements_on_indices ; ii++)
      {
        res = SORTED_LIST_ACCESS_DATA.indices.get(unit,sorted_list_index,ii,&node_ptr) ;
        SOC_SAND_CHECK_FUNC_RESULT(res, 38, exit);
        res = SORTED_LIST_ACCESS_DATA.indices.set(unit,sorted_list_index,ii - 1,node_ptr) ;
        SOC_SAND_CHECK_FUNC_RESULT(res, 42, exit);
      }
    }
    num_elements_on_indices -= 1 ;
    res = SORTED_LIST_ACCESS_DATA.num_elements_on_indices.set(unit,sorted_list_index,num_elements_on_indices) ;
    SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 34, exit) ;
  }
exit:
  SOC_SAND_EXIT_AND_SEND_ERROR( "error in soc_sand_sorted_list_entry_remove_by_iter()",0,0);
}

uint32
  soc_sand_sorted_list_entry_lookup(
    SOC_SAND_IN     int                          unit,
    SOC_SAND_IN     SOC_SAND_SORTED_LIST_PTR     sorted_list,
    SOC_SAND_IN     SOC_SAND_SORTED_LIST_KEY     *const key,
    SOC_SAND_IN     SOC_SAND_SORTED_LIST_DATA    *const data,
    SOC_SAND_OUT    uint8                        *found,
    SOC_SAND_OUT    SOC_SAND_SORTED_LIST_ITER    *iter
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

  SOC_SAND_INIT_ERROR_DEFINITIONS(SOC_SAND_SORTED_LIST_ENTRY_LOOKUP);
  SOC_SAND_SORTED_LIST_VERIFY_UNIT_IS_LEGAL(unit, 2) ;
  SOC_SAND_SORTED_LIST_CONVERT_HANDLE_TO_SORTEDLIST_INDEX(sorted_list_index,sorted_list) ;
  SOC_SAND_SORTED_LIST_VERIFY_SORTEDLIST_IS_ACTIVE(sorted_list_index,4,6) ;

  SOC_SAND_CHECK_NULL_INPUT(key) ;
  SOC_SAND_CHECK_NULL_INPUT(iter) ;
  /*
   * Check to see whether the entry exists
   */
  if (data != NULL)
  {
    res = soc_sand_sorted_list_find_match_entry(
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
    SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);
  }
  else
  {
    res = soc_sand_sorted_list_find_match_entry(
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
    SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);

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
    SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 30, exit) ;
    *iter = null_ptr;
  }
exit:
  SOC_SAND_EXIT_AND_SEND_ERROR( "error in soc_sand_sorted_list_entry_lookup()",0,0);
}


uint32
  soc_sand_sorted_list_entry_value(
    SOC_SAND_IN     int                          unit,
    SOC_SAND_IN     SOC_SAND_SORTED_LIST_PTR     sorted_list,
    SOC_SAND_IN     SOC_SAND_SORTED_LIST_ITER    iter,
    SOC_SAND_OUT    SOC_SAND_SORTED_LIST_KEY     *const key,
    SOC_SAND_OUT    SOC_SAND_SORTED_LIST_DATA    *const data
  )
{
  uint32
    res;
  uint32
    sorted_list_index,
    key_size,
    data_size,
    list_size ;

  SOC_SAND_INIT_ERROR_DEFINITIONS(SOC_SAND_SORTED_LIST_ENTRY_VALUE);
  SOC_SAND_SORTED_LIST_VERIFY_UNIT_IS_LEGAL(unit, 2) ;
  SOC_SAND_SORTED_LIST_CONVERT_HANDLE_TO_SORTEDLIST_INDEX(sorted_list_index,sorted_list) ;
  SOC_SAND_SORTED_LIST_VERIFY_SORTEDLIST_IS_ACTIVE(sorted_list_index,4,6) ;

  res = SORTED_LIST_ACCESS_INFO.list_size.get(unit,sorted_list_index,&list_size) ;
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 8, exit);

  if (iter == SOC_SAND_SORTED_LIST_ITER_END(unit,sorted_list) || iter >= list_size)
  {
    SOC_SAND_SET_ERROR_CODE(SOC_SAND_VALUE_OUT_OF_RANGE_ERR, 10, exit);
  }
  if (key != NULL)
  {
    res = SORTED_LIST_ACCESS_INFO.key_size.get(unit,sorted_list_index,&key_size) ;
    SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 18, exit);
    /*
     * read keys
     */
    /*
     * Copy 'key_size' bytes from 'SORTED_LIST_ACCESS_DATA.keys' buffer at offset
     * 'key_size * iter' into input 'key' buffer.
     */
    res = SORTED_LIST_ACCESS_DATA.keys.memread(unit,sorted_list_index,key,iter * key_size, key_size) ;
    SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 26, exit) ;
  }
  if (data != NULL)
  {
    res = SORTED_LIST_ACCESS_INFO.data_size.get(unit,sorted_list_index,&data_size) ;
    SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 28, exit);
    /*
     * read data
     */
    /*
     * Copy 'data_size' bytes from 'SORTED_LIST_ACCESS_DATA.data' buffer at offset
     * 'data_size * iter' into input 'data' buffer.
     */
    res = SORTED_LIST_ACCESS_DATA.data.memread(unit,sorted_list_index,data,iter * data_size, data_size) ;
    SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 38, exit) ;
  }
exit:
  SOC_SAND_EXIT_AND_SEND_ERROR( "error in soc_sand_sorted_list_entry_value()",0,0);
}


uint32
  soc_sand_sorted_list_get_next(
    SOC_SAND_IN     int                          unit,
    SOC_SAND_IN     SOC_SAND_SORTED_LIST_PTR     sorted_list,
    SOC_SAND_INOUT  SOC_SAND_SORTED_LIST_ITER    *iter,
    SOC_SAND_OUT    SOC_SAND_SORTED_LIST_KEY     *const key,
    SOC_SAND_OUT    SOC_SAND_SORTED_LIST_DATA    *const data
  )
{
  uint32
    res;
  uint32
    sorted_list_index ;

  SOC_SAND_INIT_ERROR_DEFINITIONS(SOC_SAND_SORTED_LIST_GET_NEXT);
  SOC_SAND_SORTED_LIST_VERIFY_UNIT_IS_LEGAL(unit, 2) ;
  SOC_SAND_SORTED_LIST_CONVERT_HANDLE_TO_SORTEDLIST_INDEX(sorted_list_index,sorted_list) ;
  SOC_SAND_SORTED_LIST_VERIFY_SORTEDLIST_IS_ACTIVE(sorted_list_index,4,6) ;

  if (*iter == SOC_SAND_SORTED_LIST_ITER_END(unit,sorted_list))
  {
    goto exit ;
  }
  res = soc_sand_sorted_list_get_next_aux(
            unit,
            sorted_list,
            *iter,
            TRUE,
            key,
            data,iter
          ) ;
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 10, exit) ;
  if (*iter == SOC_SAND_SORTED_LIST_ITER_END(unit,sorted_list))
  {
    goto exit;
  }
exit:
  SOC_SAND_EXIT_AND_SEND_ERROR( "soc_sand_sorted_list_get_next()",0,0);
}

uint32
  soc_sand_sorted_list_get_prev(
    SOC_SAND_IN     int                           unit,
    SOC_SAND_IN     SOC_SAND_SORTED_LIST_PTR      sorted_list,
    SOC_SAND_INOUT  SOC_SAND_SORTED_LIST_ITER     *iter,
    SOC_SAND_OUT    SOC_SAND_SORTED_LIST_KEY      *const key,
    SOC_SAND_OUT    SOC_SAND_SORTED_LIST_DATA     *const data
  )
{
  uint32
    head_place ;
  uint32
    sorted_list_index ;
  uint32
    res;

  SOC_SAND_INIT_ERROR_DEFINITIONS(SOC_SAND_SORTED_LIST_GET_PREV);
  SOC_SAND_SORTED_LIST_VERIFY_UNIT_IS_LEGAL(unit, 2) ;
  SOC_SAND_SORTED_LIST_CONVERT_HANDLE_TO_SORTEDLIST_INDEX(sorted_list_index,sorted_list) ;
  SOC_SAND_SORTED_LIST_VERIFY_SORTEDLIST_IS_ACTIVE(sorted_list_index,4,6) ;

  SOC_SAND_CHECK_NULL_INPUT(data) ;
  SOC_SAND_CHECK_NULL_INPUT(key);

  SOC_SAND_SORTED_LIST_HEAD_PLACE_GET(sorted_list_index,head_place,12) ;
  if (*iter == head_place /* SOC_SAND_SORTED_LIST_HEAD_PLACE(&(sorted_list->init_info)) */)
  {
    goto exit;
  }
  res = soc_sand_sorted_list_get_next_aux(
            unit,
            sorted_list,
            *iter,
            FALSE,
            key,
            data,iter
          );
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 10, exit) ;
exit:
  SOC_SAND_EXIT_AND_SEND_ERROR( "soc_sand_sorted_list_get_prev()",0,0);
}

uint32
  soc_sand_sorted_list_get_follow(
    SOC_SAND_IN     int                           unit,
    SOC_SAND_IN     SOC_SAND_SORTED_LIST_PTR      sorted_list,
    SOC_SAND_INOUT  SOC_SAND_SORTED_LIST_ITER     *iter,
    SOC_SAND_OUT    uint8                         forward,
    SOC_SAND_OUT    SOC_SAND_SORTED_LIST_KEY      *const key,
    SOC_SAND_OUT    SOC_SAND_SORTED_LIST_DATA     *const data
    )
{
  uint32
    head_place ;
  uint32
    sorted_list_index ;
  uint32
    res;

  SOC_SAND_INIT_ERROR_DEFINITIONS(SOC_SAND_SORTED_LIST_GET_FOLLOW);
  SOC_SAND_SORTED_LIST_VERIFY_UNIT_IS_LEGAL(unit, 2) ;
  SOC_SAND_SORTED_LIST_CONVERT_HANDLE_TO_SORTEDLIST_INDEX(sorted_list_index,sorted_list) ;
  SOC_SAND_SORTED_LIST_VERIFY_SORTEDLIST_IS_ACTIVE(sorted_list_index,4,6) ;

  SOC_SAND_CHECK_NULL_INPUT(key);
  SOC_SAND_CHECK_NULL_INPUT(data);

  SOC_SAND_SORTED_LIST_HEAD_PLACE_GET(sorted_list_index,head_place,12) ;
  if (*iter == head_place /* SOC_SAND_SORTED_LIST_HEAD_PLACE(&(sorted_list->init_info)) */ )
  {
    goto exit;
  }
  res = soc_sand_sorted_list_get_next_aux(
            unit,
            sorted_list,
            *iter,
            forward,
            key,
            data,iter
          );
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 10, exit) ;
exit:
  SOC_SAND_EXIT_AND_SEND_ERROR( "soc_sand_sorted_list_get_follow()",0,0);
}

/*********************************************************************
* NAME:
*     soc_sand_sorted_list_get_size_for_save
* TYPE:
*   PROC
* DATE:
*   May 19 2015
* FUNCTION:
*   Returns the size of the buffer needed to return the sorted list as buffer.
*   in sort to be loaded later
* INPUT:
*   SOC_SAND_IN  int                         unit -
*     Identifier of the device to access.
*   SOC_SAND_IN  SOC_SAND_SORTED_LIST_INFO   *sorted_list -
*     The sorted list to get the size for.
*   SOC_SAND_OUT  uint32                     *size -
*     the size of the buffer.
* REMARKS:
*   None.
* RETURNS:
*   OK or ERROR indication.
*********************************************************************/
uint32
soc_sand_sorted_list_get_size_for_save(
     SOC_SAND_IN   int                            unit,
     SOC_SAND_IN   SOC_SAND_SORTED_LIST_PTR       sorted_list,
     SOC_SAND_OUT  uint32                         *size
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
  SOC_SAND_OCC_BM_PTR
    memory_use ;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);
  SOC_SAND_SORTED_LIST_VERIFY_UNIT_IS_LEGAL(unit, 2) ;
  SOC_SAND_SORTED_LIST_CONVERT_HANDLE_TO_SORTEDLIST_INDEX(sorted_list_index,sorted_list) ;
  SOC_SAND_SORTED_LIST_VERIFY_SORTEDLIST_IS_ACTIVE(sorted_list_index,4,6) ;

  total_size = 0;
  SOC_SAND_CHECK_NULL_INPUT(size);

  res = SORTED_LIST_ACCESS_DATA.ptr_size.get(unit,sorted_list_index,&ptr_size) ;
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 12, exit) ;
  res = SORTED_LIST_ACCESS_INFO.data_size.get(unit,sorted_list_index,&data_size) ;
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 14, exit);
  tmp_size = SOC_SAND_MAX(ptr_size,data_size);

  /* init info */
  total_size += sizeof(SOC_SAND_SORTED_LIST_INIT_INFO);

  /* DS data */
  res = SORTED_LIST_ACCESS_INFO.key_size.get(unit,sorted_list_index,&key_size) ;
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 16, exit);
  res = SORTED_LIST_ACCESS_INFO.list_size.get(unit,sorted_list_index,&list_size) ;
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 18, exit);

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
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 28, exit) ;
  res = soc_sand_occ_bm_get_size_for_save(
    unit,
    memory_use,
    &bmp_size
    );
  SOC_SAND_CHECK_FUNC_RESULT(res,30, exit) ;
  total_size += bmp_size ;

  *size = total_size ;

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR( "error in soc_sand_hash_table_get_size_for_save()",0,0);
}

/*********************************************************************
* NAME:
*     soc_sand_sorted_list_save
* TYPE:
*   PROC
* DATE:
*   May 19 2015
* FUNCTION:
*     saves the given sorted list in the given buffer
* INPUT:
*   SOC_SAND_IN  int                           unit -
*     Identifier of the device to access.
*   SOC_SAND_IN  SOC_SAND_SORTED_LIST_PTR      sorted_list -
*     The sorted list to save.
*   SOC_SAND_OUT  uint8                       *buffer -
*     buffer to include the hast table
*   SOC_SAND_IN  uint32                        buffer_size_bytes,
*   SOC_SAND_OUT uint32                       *actual_size_bytes
* REMARKS:
*   - the size of the buffer has to be at least as the value returned
*     by soc_sand_sorted_list_get_size_for_save.
*   - the hash and rehash functions are not saved.
* RETURNS:
*   OK or ERROR indication.
*********************************************************************/
uint32
soc_sand_sorted_list_save(
    SOC_SAND_IN  int                           unit,
    SOC_SAND_IN  SOC_SAND_SORTED_LIST_PTR      sorted_list,
    SOC_SAND_OUT uint8                         *buffer,
    SOC_SAND_IN  uint32                        buffer_size_bytes,
    SOC_SAND_OUT uint32                        *actual_size_bytes
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
  SOC_SAND_OCC_BM_PTR
    memory_use ;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);
  SOC_SAND_SORTED_LIST_VERIFY_UNIT_IS_LEGAL(unit, 2) ;
  SOC_SAND_SORTED_LIST_CONVERT_HANDLE_TO_SORTEDLIST_INDEX(sorted_list_index,sorted_list) ;
  SOC_SAND_SORTED_LIST_VERIFY_SORTEDLIST_IS_ACTIVE(sorted_list_index,4,6) ;

  SOC_SAND_CHECK_NULL_INPUT(buffer);
  SOC_SAND_CHECK_NULL_INPUT(actual_size_bytes);

  cur_ptr = buffer ;
  total_size = 0 ;

  res = SORTED_LIST_ACCESS_DATA.ptr_size.get(unit,sorted_list_index,&ptr_size) ;
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 12, exit) ;
  res = SORTED_LIST_ACCESS_INFO.data_size.get(unit,sorted_list_index,&data_size) ;
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 14, exit) ;
  tmp_size = SOC_SAND_MAX(ptr_size,data_size) ;
  /*
   * copy init info structure (SOC_SAND_SORTED_LIST_INIT_INFO) into supplied (input) buffer area.
   */
  res = SORTED_LIST_ACCESS_INFO.get(unit,sorted_list_index,(SOC_SAND_SORTED_LIST_INIT_INFO *)cur_ptr) ;
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 16, exit) ;
  cur_ptr += sizeof(SOC_SAND_SORTED_LIST_INIT_INFO) ;
  /*
   * Copy DS data
   */
  res = SORTED_LIST_ACCESS_INFO.key_size.get(unit,sorted_list_index,&key_size) ;
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 16, exit);
  res = SORTED_LIST_ACCESS_INFO.list_size.get(unit,sorted_list_index,&list_size) ;
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 18, exit);

  res = SORTED_LIST_ACCESS_DATA.tmp_data.memwrite(unit,sorted_list_index,cur_ptr,0,tmp_size) ;
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 22, exit) ;
  cur_ptr += tmp_size ;

  res = SORTED_LIST_ACCESS_DATA.tmp_key.memwrite(unit,sorted_list_index,cur_ptr,0,key_size) ;
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 24, exit) ;
  cur_ptr += key_size ;

  res = SORTED_LIST_ACCESS_DATA.keys.memwrite(unit,sorted_list_index,cur_ptr,0,list_size * key_size) ;
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 26, exit) ;
  cur_ptr += (list_size * key_size) ;

  res = SORTED_LIST_ACCESS_DATA.next.memwrite(unit,sorted_list_index,cur_ptr,0,(list_size + 2) * ptr_size) ;
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 28, exit) ;
  cur_ptr += ((list_size + 2) * ptr_size) ;

  res = SORTED_LIST_ACCESS_DATA.prev.memwrite(unit,sorted_list_index,cur_ptr,0,(list_size + 2) * ptr_size) ;
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 30, exit) ;
  cur_ptr += ((list_size + 2) * ptr_size) ;

  res = SORTED_LIST_ACCESS_DATA.data.memwrite(unit,sorted_list_index,cur_ptr,0,list_size * data_size) ;
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 30, exit) ;
  cur_ptr += (list_size * data_size) ;
  /*
   * Initialize the data to be mapped to
   */
  res = SORTED_LIST_ACCESS_DATA.memory_use.get(unit,sorted_list_index,&memory_use) ;
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 28, exit) ;
  res = soc_sand_occ_bm_save(
          unit,
          memory_use,
          cur_ptr,
          buffer_size_bytes - total_size,
          &cur_size
        );
  SOC_SAND_CHECK_FUNC_RESULT(res,36, exit) ;
  cur_ptr += cur_size ;
  total_size += cur_size ;

  *actual_size_bytes = total_size;

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR( "error in soc_sand_hash_table_save()",0,0);
}


/*********************************************************************
* NAME:
*     soc_sand_sorted_list_load
* TYPE:
*   PROC
* DATE:
*   May 19 2015
* FUNCTION:
*   Load sorted list from the given buffer which contains a previously
*   saved sorted list.
* INPUT:
*   SOC_SAND_IN  int                                   unit -
*     Identifier of the device to access.
*   SOC_SAND_IN  uint8                                 **buffer -
*     buffer includes the sorted list
*   SOC_SAND_IN  SOC_SAND_SORTED_LIST_SW_DB_ENTRY_GET  get_entry_fun,
*   SOC_SAND_IN  SOC_SAND_SORTED_LIST_SW_DB_ENTRY_SET  set_entry_fun,
*   SOC_SAND_IN  SOC_SAND_SORTED_LIST_KEY_CMP          cmp_key_fun,
*   SOC_SAND_OUT SOC_SAND_SORTED_LIST_PTR              *sorted_list_ptr -
*     Handle to the sorted list to load.
* REMARKS:
*   - the size of the buffer has to be at least as the value returned
*     by soc_sand_sorted_list_get_size_for_save.
*   - there is need to supply the hash and rehash function (in case they are not
*     the default implementation, cause in the save they are not saved.
*     by soc_sand_sorted_list_get_size_for_save.
* RETURNS:
*   OK or ERROR indication.
*********************************************************************/
uint32
  soc_sand_sorted_list_load(
    SOC_SAND_IN  int                                   unit,
    SOC_SAND_IN  uint8                                 **buffer,
    SOC_SAND_IN  SOC_SAND_SORTED_LIST_SW_DB_ENTRY_GET  get_entry_fun,
    SOC_SAND_IN  SOC_SAND_SORTED_LIST_SW_DB_ENTRY_SET  set_entry_fun,
    SOC_SAND_IN  SOC_SAND_SORTED_LIST_KEY_CMP          cmp_key_fun,                      
    SOC_SAND_OUT SOC_SAND_SORTED_LIST_PTR              *sorted_list_ptr
  )
{
  SOC_SAND_SORTED_LIST_INIT_INFO
    local_init_info ;
  SOC_SAND_IN uint8
    *cur_ptr ;
  uint32
    res;
  uint32
    tmp_size ;
  SOC_SAND_SORTED_LIST_PTR
    sorted_list ;
  uint32
    sorted_list_index,
    ptr_size,
    key_size,
    list_size,
    data_size ;
  SOC_SAND_OCC_BM_PTR
    memory_use ;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);
  SOC_SAND_SORTED_LIST_VERIFY_UNIT_IS_LEGAL(unit, 2) ;

  SOC_SAND_CHECK_NULL_INPUT(buffer);
  SOC_SAND_CHECK_NULL_INPUT(sorted_list_ptr);

  cur_ptr = buffer[0] ;
  /*
   * copy init info (structure SOC_SAND_SORTED_LIST_INIT_INFO) from supplied (input) buffer
   * area into local memory and create sorted list.
   */
  soc_sand_os_memcpy(&local_init_info, cur_ptr, sizeof(SOC_SAND_SORTED_LIST_INIT_INFO));
  cur_ptr += sizeof(SOC_SAND_SORTED_LIST_INIT_INFO);
  local_init_info.get_entry_fun = get_entry_fun ;
  local_init_info.set_entry_fun = set_entry_fun ;
  /*
   * create DS - will not work!! (petra only code)
   */
  res = soc_sand_sorted_list_create(unit, sorted_list_ptr, local_init_info);
  SOC_SAND_CHECK_FUNC_RESULT(res,20, exit) ;
  sorted_list = *sorted_list_ptr ;
  SOC_SAND_SORTED_LIST_CONVERT_HANDLE_TO_SORTEDLIST_INDEX(sorted_list_index,sorted_list) ;

  res = SORTED_LIST_ACCESS_DATA.ptr_size.get(unit,sorted_list_index,&ptr_size) ;
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 12, exit) ;
  res = SORTED_LIST_ACCESS_INFO.data_size.get(unit,sorted_list_index,&data_size) ;
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 14, exit) ;
  tmp_size = SOC_SAND_MAX(ptr_size,data_size);

  /* copy DS data */
  res = SORTED_LIST_ACCESS_INFO.key_size.get(unit,sorted_list_index,&key_size) ;
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 16, exit);
  res = SORTED_LIST_ACCESS_INFO.list_size.get(unit,sorted_list_index,&list_size) ;
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 18, exit);

  res = SORTED_LIST_ACCESS_DATA.tmp_data.memread(unit,sorted_list_index,(uint8 *)cur_ptr,0,tmp_size) ;
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 22, exit) ;
  cur_ptr += tmp_size ;

  res = SORTED_LIST_ACCESS_DATA.tmp_key.memread(unit,sorted_list_index,(uint8 *)cur_ptr,0,key_size) ;
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 24, exit) ;
  cur_ptr += key_size ;

  res = SORTED_LIST_ACCESS_DATA.keys.memread(unit,sorted_list_index,(uint8 *)cur_ptr,0,list_size * key_size) ;
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 26, exit) ;
  cur_ptr += (list_size * key_size) ;

  res = SORTED_LIST_ACCESS_DATA.next.memread(unit,sorted_list_index,(uint8 *)cur_ptr,0,(list_size + 2) * ptr_size) ;
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 28, exit) ;
  cur_ptr += ((list_size + 2) * ptr_size) ;

  res = SORTED_LIST_ACCESS_DATA.prev.memread(unit,sorted_list_index,(uint8 *)cur_ptr,0,(list_size + 2) * ptr_size) ;
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 30, exit) ;
  cur_ptr += ((list_size + 2) * ptr_size) ;

  res = SORTED_LIST_ACCESS_DATA.data.memread(unit,sorted_list_index,(uint8 *)cur_ptr,0,list_size * data_size) ;
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 32, exit) ;
  cur_ptr += (list_size * data_size) ;

  /* Destroy (release) bitmap */
  res = SORTED_LIST_ACCESS_DATA.memory_use.get(unit,sorted_list_index,&memory_use) ;
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 34, exit) ;
  res = soc_sand_occ_bm_destroy(
    unit,
    memory_use
    );
  SOC_SAND_CHECK_FUNC_RESULT(res,36, exit);

  /* Load bitmap */
  res = soc_sand_occ_bm_load(
    unit,
    &cur_ptr,
    &memory_use
    );
  SOC_SAND_CHECK_FUNC_RESULT(res,38, exit);
  res = SORTED_LIST_ACCESS_DATA.memory_use.set(unit,sorted_list_index,memory_use) ;
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 40, exit) ;

  *buffer = cur_ptr ;
exit:
  SOC_SAND_EXIT_AND_SEND_ERROR( "error in soc_sand_hash_table_load()",0,0);
}

uint32
  soc_sand_sorted_list_print(
    SOC_SAND_IN  int                            unit,
    SOC_SAND_IN  SOC_SAND_SORTED_LIST_PTR       sorted_list,
    SOC_SAND_IN  char                           table_header[SOC_SAND_SORTED_LIST_HEADER_SIZE],
    SOC_SAND_IN  SOC_SAND_SORTED_LIST_PRINT_VAL print_key,
    SOC_SAND_IN  SOC_SAND_SORTED_LIST_PRINT_VAL print_data
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

  SOC_SAND_INIT_ERROR_DEFINITIONS(SOC_SAND_SORTED_LIST_PRINT);
  SOC_SAND_SORTED_LIST_VERIFY_UNIT_IS_LEGAL(unit, 2) ;
  SOC_SAND_SORTED_LIST_CONVERT_HANDLE_TO_SORTEDLIST_INDEX(sorted_list_index,sorted_list) ;
  SOC_SAND_SORTED_LIST_VERIFY_SORTEDLIST_IS_ACTIVE(sorted_list_index,4,6) ;
  /*
   * traverse the sorted list head list.
   */
  LOG_CLI((BSL_META_U(unit,"  %s\n"), table_header));
  SOC_SAND_SORTED_LIST_HEAD_PLACE_GET(sorted_list_index,prev,12) ;
  res = soc_sand_sorted_list_get_tmp_data_ptr_from_handle(unit,sorted_list,&tmp_data_ptr) ;
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 10, exit);
  res = soc_sand_sorted_list_get_tmp_key_ptr_from_handle(unit,sorted_list,&tmp_key_ptr) ;
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 10, exit);
  do
  {
    /*
     * read next entry.
     */
    res = soc_sand_sorted_list_get_next_aux(
            unit,
            sorted_list,
            prev,
            TRUE,
            tmp_key_ptr,
            tmp_data_ptr,&curr
          );
    SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 12, exit);
    if (curr == SOC_SAND_SORTED_LIST_ITER_END(unit,sorted_list))
    {
      goto exit;
    }
    LOG_CLI((BSL_META_U(unit,"   %-10u"), curr)) ;
    print_data(tmp_data_ptr);
    print_key(tmp_key_ptr);
    LOG_CLI((BSL_META_U(unit,"\n")));

    prev = curr;
  }
  while(curr != SOC_SAND_SORTED_LIST_ITER_END(unit,sorted_list));

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR( "error in soc_sand_sorted_list_print()",0,0);
}

uint32
  soc_sand_sorted_list_data_cmp(
    SOC_SAND_IN    int                             unit,
    SOC_SAND_IN    SOC_SAND_SORTED_LIST_PTR        sorted_list,
    SOC_SAND_IN    uint32                          data_place,
    SOC_SAND_IN    SOC_SAND_SORTED_LIST_DATA       *const data,
    SOC_SAND_INOUT int32                           *cmp_res_ptr
  )
{
  uint32
    res,
    sorted_list_index,
    data_size ;
  int32
    cmp_res ;
  SOC_SAND_SORTED_LIST_KEY_CMP 
    cmp_key_fun;
  uint8
    *tmp_data_ptr ;
  SOC_SAND_SORTED_LIST_CMP_FUNC_TYPE
    cmp_func_type ;

  SOC_SAND_INIT_ERROR_DEFINITIONS(SOC_SAND_SORTED_LIST_PRINT);
  SOC_SAND_SORTED_LIST_VERIFY_UNIT_IS_LEGAL(unit, 2) ;
  SOC_SAND_SORTED_LIST_CONVERT_HANDLE_TO_SORTEDLIST_INDEX(sorted_list_index,sorted_list) ;
  SOC_SAND_SORTED_LIST_VERIFY_SORTEDLIST_IS_ACTIVE(sorted_list_index,4,6) ;

  res = SORTED_LIST_ACCESS_INFO.cmp_func_type.get(unit,sorted_list_index,&cmp_func_type) ;
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 8, exit);
  if (cmp_func_type == SOC_SAND_SORTED_LIST_CMP_FUNC_TYPE_TCAM)
  {
    cmp_key_fun = soc_sand_sorted_list_tcam_cmp_priority;
  }
  else
  {
    cmp_key_fun = (SOC_SAND_SORTED_LIST_KEY_CMP)soc_sand_os_memcmp;
  }
  {
    /*
     * read data: Copy 'data' to 'tmp_data'
     */
    res = SORTED_LIST_ACCESS_INFO.data_size.get(unit,sorted_list_index,&data_size) ;
    SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 10, exit) ;
    res = soc_sand_sorted_list_get_tmp_data_ptr_from_handle(unit,sorted_list,&tmp_data_ptr) ;
    SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 12, exit);
    /*
     * Copy 'data_size' bytes from 'SORTED_LIST_ACCESS_DATA.data' buffer at offset
     * 'data_size * data_place' into 'tmp_data' buffer.
     */
    res = SORTED_LIST_ACCESS_DATA.data.memread(unit,sorted_list_index,tmp_data_ptr,data_place * data_size, data_size) ;
    SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 14, exit) ;
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
  SOC_SAND_EXIT_AND_SEND_ERROR( "error in soc_sand_sorted_list_data_cmp()",0,0);
}

uint32
  soc_sand_sorted_list_get_next_aux(
    SOC_SAND_IN    int                         unit,
    SOC_SAND_IN    SOC_SAND_SORTED_LIST_PTR    sorted_list,
    SOC_SAND_IN    SOC_SAND_SORTED_LIST_ITER   curr,
    SOC_SAND_IN    uint8                       forward,
    SOC_SAND_OUT   SOC_SAND_SORTED_LIST_KEY    *const key,
    SOC_SAND_OUT   SOC_SAND_SORTED_LIST_DATA   *const data,
    SOC_SAND_INOUT SOC_SAND_SORTED_LIST_ITER   *next_or_prev
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

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);
  SOC_SAND_SORTED_LIST_VERIFY_UNIT_IS_LEGAL(unit, 2) ;
  SOC_SAND_SORTED_LIST_CONVERT_HANDLE_TO_SORTEDLIST_INDEX(sorted_list_index,sorted_list) ;
  SOC_SAND_SORTED_LIST_VERIFY_SORTEDLIST_IS_ACTIVE(sorted_list_index,4,6) ;

  SOC_SAND_CHECK_NULL_INPUT(next_or_prev);

  res = SORTED_LIST_ACCESS_DATA.null_ptr.get(unit,sorted_list_index,&null_ptr) ;
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 8, exit) ;
  /*
   * read next entry.
   */
  if (
    (!forward &&  curr == SOC_SAND_SORTED_LIST_ITER_BEGIN(unit,sorted_list)) ||
    ((forward && curr == SOC_SAND_SORTED_LIST_ITER_END(unit,sorted_list)))
    )
  {
    *next_or_prev = null_ptr ;
    goto exit ;
  }
  res = soc_sand_sorted_list_get_tmp_data_ptr_from_handle(unit,sorted_list,&tmp_data_ptr) ;
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 12, exit);
  res = SORTED_LIST_ACCESS_DATA.ptr_size.get(unit,sorted_list_index,&ptr_size) ;
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 14, exit) ;
  if (forward)
  {
    /*
     * Copy 'ptr_size' bytes from 'SORTED_LIST_ACCESS_DATA.next' buffer at offset
     * 'curr * ptr_size' into 'tmp_data' buffer.
     */
    res = SORTED_LIST_ACCESS_DATA.next.memread(unit,sorted_list_index,tmp_data_ptr,curr * ptr_size,ptr_size) ;
    SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 16, exit) ;
  }
  else
  {
    /*
     * Copy 'ptr_size' bytes from 'SORTED_LIST_ACCESS_DATA.prev' buffer at offset
     * 'curr * ptr_size' into 'tmp_data' buffer.
     */
    res = SORTED_LIST_ACCESS_DATA.prev.memread(unit,sorted_list_index,tmp_data_ptr,curr * ptr_size,ptr_size) ;
    SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 18, exit) ;
  }
  /*
   * Check whether the list node is null.
   */
  ptr_long = 0 ;
  soc_sand_U8_to_U32(
      tmp_data_ptr,
      ptr_size,
      &ptr_long
  ) ;
  if (ptr_long == null_ptr)
  {
    *next_or_prev = null_ptr ;
    goto exit ;
  }
  if (ptr_long == SOC_SAND_SORTED_LIST_ITER_BEGIN(unit,sorted_list))
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
    SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 20, exit) ;
    /*
     * Copy 'key_size' bytes from 'SORTED_LIST_ACCESS_DATA.keys' buffer at offset
     * 'ptr_long * key_size' into input 'key' buffer.
     */
    res = SORTED_LIST_ACCESS_DATA.keys.memread(unit,sorted_list_index,key,ptr_long * key_size,key_size) ;
    SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 22, exit) ;
  }
  if (data != NULL)
  {
    /*
     * read data
     */
    res = SORTED_LIST_ACCESS_INFO.data_size.get(unit,sorted_list_index,&data_size) ;
    SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 24, exit) ;
    /*
     * Copy 'data_size' bytes from 'SORTED_LIST_ACCESS_DATA.data' buffer at offset
     * 'ptr_long * data_size' into input 'data' buffer.
     */
    res = SORTED_LIST_ACCESS_DATA.data.memread(unit,sorted_list_index,data,ptr_long * data_size,data_size) ;
    SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 26, exit) ;
  }
  *next_or_prev = ptr_long ;
  goto exit ;
exit:
  SOC_SAND_EXIT_AND_SEND_ERROR( "error in soc_sand_sorted_list_get_next_aux()",0,0);
}

uint32
  soc_sand_SAND_SORTED_LIST_INFO_clear(
    SOC_SAND_IN int                      unit,
    SOC_SAND_IN SOC_SAND_SORTED_LIST_PTR sorted_list
  )
{
  uint32
    sorted_list_index ;
  uint32
    res;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);
  SOC_SAND_SORTED_LIST_VERIFY_UNIT_IS_LEGAL(unit, 2) ;
  SOC_SAND_SORTED_LIST_CONVERT_HANDLE_TO_SORTEDLIST_INDEX(sorted_list_index,sorted_list) ;
  SOC_SAND_SORTED_LIST_VERIFY_SORTEDLIST_IS_ACTIVE(sorted_list_index,4,6) ;

  res = SORTED_LIST_ACCESS_INFO.data_size.set(unit, sorted_list_index, 0);
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 8, exit);
  res = SORTED_LIST_ACCESS_INFO.get_entry_fun.set(unit, sorted_list_index, 0);
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 10, exit);
  res = SORTED_LIST_ACCESS_INFO.key_size.set(unit, sorted_list_index, 0);
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 12, exit);
  res = SORTED_LIST_ACCESS_INFO.set_entry_fun.set(unit, sorted_list_index, 0);
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 14, exit);
  res = SORTED_LIST_ACCESS_INFO.list_size.set(unit, sorted_list_index, 0);
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 16, exit);
  res = SORTED_LIST_ACCESS_INFO.prime_handle.set(unit, sorted_list_index, 0);
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 18, exit);
  res = SORTED_LIST_ACCESS_INFO.sec_handle.set(unit, sorted_list_index, 0);
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 20, exit);
  res = SORTED_LIST_ACCESS_INFO.cmp_func_type.set(unit, sorted_list_index, SOC_SAND_SORTED_LIST_CMP_FUNC_TYPE_DEFAULT);
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 22, exit);

  res = SORTED_LIST_ACCESS_DATA.ptr_size.set(unit, sorted_list_index, 0);
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 24, exit);
  res = SORTED_LIST_ACCESS_DATA.memory_use.set(unit, sorted_list_index, 0);
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 26, exit);
  res = SORTED_LIST_ACCESS_DATA.null_ptr.set(unit, sorted_list_index, 0);
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 28, exit);

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in soc_sand_SAND_SORTED_LIST_INFO_clear()", 0, 0);
}

STATIC
  void
    soc_sand_db_prio_list_data_encode(
      SOC_SAND_IN  ARAD_TCAM_PRIO_LOCATION *prio_location,
      SOC_SAND_OUT uint8  *data
    )
{
  sal_memcpy(data, prio_location, sizeof(ARAD_TCAM_PRIO_LOCATION));
}
STATIC
  void
    soc_sand_db_prio_list_priority_value_encode(
      SOC_SAND_IN  uint32     priority,
      SOC_SAND_OUT uint8     *data
    )
{
  soc_sand_U32_to_U8(
    &priority,
    sizeof(uint32),
    data
  );
}
uint32
  soc_sand_print_indices(
    int                      unit,
    SOC_SAND_SORTED_LIST_PTR sorted_list
  )
{
  uint32
    res = SOC_SAND_OK ;
  int
    ii ;
  uint32
    node_ptr,
    num_elements_on_indices ;
  static
    int num_invocations = 0 ;
  uint32
    sorted_list_index ;
  
  SOC_SAND_INIT_ERROR_DEFINITIONS(0);
  SOC_SAND_SORTED_LIST_VERIFY_UNIT_IS_LEGAL(unit, 2) ;
  SOC_SAND_SORTED_LIST_CONVERT_HANDLE_TO_SORTEDLIST_INDEX(sorted_list_index,sorted_list) ;
  SOC_SAND_SORTED_LIST_VERIFY_SORTEDLIST_IS_ACTIVE(sorted_list_index,4,6) ;

  num_invocations++ ;
  res = SORTED_LIST_ACCESS_DATA.num_elements_on_indices.get(unit,sorted_list_index,&num_elements_on_indices) ;
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);
  sal_printf("%s(): Invocation %d Sorted_list %d - Display 'indices' of %d entries\r\n",
    __FUNCTION__,num_invocations,(int)sorted_list,num_elements_on_indices) ;
  for (ii = 0 ; ii < num_elements_on_indices ; ii++)
  {
    res = SORTED_LIST_ACCESS_DATA.indices.get(unit,sorted_list_index,ii,&node_ptr) ;
    SOC_SAND_CHECK_FUNC_RESULT(res, 14, exit);
    sal_printf("index %02d value %02d\r\n",ii,node_ptr) ;
  }

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR( "error in soc_sand_print_indices()",0,0);
}
STATIC
  void
    soc_sand_print_key(
      SOC_SAND_IN  uint8                    *buffer
    )
{
    sal_printf("Key: 0x%02X %02X %02X %02X\r\n",buffer[0],buffer[1],buffer[2],buffer[3]) ;
}
STATIC
  void
    soc_sand_print_data(
      SOC_SAND_IN  uint8                    *buffer
    )
{
    sal_printf("Data: 0x%02X %02X %02X %02X   %02X %02X %02X %02X\r\n",
      buffer[0],buffer[1],buffer[2],buffer[3],buffer[4],buffer[5],buffer[6],buffer[7]) ;
}
uint32
  soc_sand_print_list(
    int                      unit,
    SOC_SAND_SORTED_LIST_PTR sorted_list
  )
{
  uint32
      res = SOC_SAND_OK ;
  static
      int num_invocations = 0 ;
  uint32
      sorted_list_index ;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);
  SOC_SAND_SORTED_LIST_VERIFY_UNIT_IS_LEGAL(unit, 2) ;
  SOC_SAND_SORTED_LIST_CONVERT_HANDLE_TO_SORTEDLIST_INDEX(sorted_list_index,sorted_list) ;
  SOC_SAND_SORTED_LIST_VERIFY_SORTEDLIST_IS_ACTIVE(sorted_list_index,4,6) ;

  num_invocations++ ;
  sal_printf("%s(): Invocation %d Sorted_list %d - \r\n",
    __FUNCTION__,num_invocations,(int)sorted_list) ;
  soc_sand_sorted_list_print(unit,sorted_list,"Sorted linked list",soc_sand_print_key,soc_sand_print_data) ;
exit:
  SOC_SAND_EXIT_AND_SEND_ERROR( "error in soc_sand_print_list()",0,0);
}

#if SOC_SAND_DEBUG
/* { */
/*
 * See arad_tcam_db_create_unsafe(), arad_tcam_db_priority_list_entry_add()
 */
uint32
  soc_sand_exercise_01(int unit)
{
  uint32
    num_elements,
    priority_profile,
    entry_id,
    res = SOC_SAND_OK ;
  uint32
    prio,
    priority[2][8] ;
  uint32
    max_nof_entries ;
  SOC_SAND_SORTED_LIST_PTR
    priorities;
  SOC_SAND_SORTED_LIST_INIT_INFO
    priorities_init_info;
  uint8
    key_buffer[ARAD_TCAM_DB_LIST_KEY_SIZE],
    data_buffer[ARAD_TCAM_DB_LIST_DATA_SIZE] ;
  uint8
    entry_added;
  ARAD_TCAM_PRIO_LOCATION 
    prio_location;
  uint8
    found_equal ;
    
  SOC_SAND_INIT_ERROR_DEFINITIONS(0) ;

  priority_profile = 1 ;
  max_nof_entries = sizeof(priority[0])/sizeof(priority[0][0]) ;
  {
    /*
     * Select priorities for entries.
     */
    priority[0][0] = 16  ;
    priority[0][1] = 17  ;
    priority[0][2] = 15  ;
    priority[0][3] = 19  ;
    priority[0][4] = 18  ;
    priority[0][5] = 20  ;
    priority[0][6] = 21  ;
    priority[0][7] = 22  ;

    priority[1][0] = 44  ;
    priority[1][1] = 34  ;
    priority[1][2] = 30  ;
    priority[1][3] = 38  ;
    priority[1][4] = 40  ;
    priority[1][5] = 32  ;
    priority[1][6] = 42  ;
    priority[1][7] = 36  ;
  }
  /*
   * Initialize the entry priority list
   */
  soc_sand_os_memset(&priorities_init_info, 0x0, sizeof(SOC_SAND_SORTED_LIST_INIT_INFO));
  priorities_init_info.prime_handle  = unit;
  priorities_init_info.sec_handle    = 0;
  priorities_init_info.list_size     = max_nof_entries;
  priorities_init_info.key_size      = ARAD_TCAM_DB_LIST_KEY_SIZE * sizeof(uint8);
  priorities_init_info.data_size     = ARAD_TCAM_DB_LIST_DATA_SIZE * sizeof(uint8);
  priorities_init_info.get_entry_fun = NULL;
  priorities_init_info.set_entry_fun = NULL;
  priorities_init_info.cmp_func_type   = SOC_SAND_SORTED_LIST_CMP_FUNC_TYPE_TCAM;
  res = soc_sand_sorted_list_create(unit, &priorities, priorities_init_info);
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  for (entry_id = 0 ; entry_id < max_nof_entries ; entry_id++)
  {
    soc_sand_db_prio_list_priority_value_encode(priority[priority_profile][entry_id], key_buffer) ;
    ARAD_TCAM_PRIO_LOCATION_clear(&prio_location) ;
    prio_location.entry_id_first = entry_id ;
    prio_location.entry_id_last = entry_id ;
    soc_sand_db_prio_list_data_encode(&prio_location, data_buffer) ;

    res = soc_sand_sorted_list_entry_add(
              unit,
              priorities,
              key_buffer,
              data_buffer,
              &entry_added
            );
    SOC_SAND_CHECK_FUNC_RESULT(res, 14, exit) ;

    soc_sand_print_indices(unit,priorities) ;
    soc_sand_print_list(unit,priorities) ;
  }
  if (0)
  {
    /*
     * Now exercise bisect. Use soc_sand_sorted_list_find_higher_eq_key()
     */
    SOC_SAND_SORTED_LIST_ITER
      iter ;
    prio = priority[priority_profile][1] ;
    soc_sand_db_prio_list_priority_value_encode(prio, key_buffer) ;
    res = soc_sand_sorted_list_find_higher_eq_key(unit,priorities,key_buffer,&iter,&found_equal) ;
    SOC_SAND_CHECK_FUNC_RESULT(res, 18, exit) ;
    sal_printf(
      "%s(): Found matching element (%d found_equal %d) for priority %d\r\n",__FUNCTION__,iter,found_equal,prio) ;
    prio = priority[priority_profile][4] + 1 ;
    soc_sand_db_prio_list_priority_value_encode(prio, key_buffer) ;
    res = soc_sand_sorted_list_find_higher_eq_key(unit,priorities,key_buffer,&iter,&found_equal) ;
    SOC_SAND_CHECK_FUNC_RESULT(res, 22, exit) ;
    sal_printf(
      "%s(): Found matching element (%d found_equal %d) for priority %d\r\n",__FUNCTION__,iter,found_equal,prio) ;
    prio = priority[priority_profile][0] ;
    soc_sand_db_prio_list_priority_value_encode(prio, key_buffer) ;
    res = soc_sand_sorted_list_find_higher_eq_key(unit,priorities,key_buffer,&iter,&found_equal) ;
    SOC_SAND_CHECK_FUNC_RESULT(res, 26, exit) ;
    sal_printf(
      "%s(): Found matching element (%d found_equal %d) for priority %d\r\n",__FUNCTION__,iter,found_equal,prio) ;
    prio = priority[priority_profile][2] ;
    soc_sand_db_prio_list_priority_value_encode(prio, key_buffer) ;
    res = soc_sand_sorted_list_find_higher_eq_key(unit,priorities,key_buffer,&iter,&found_equal) ;
    SOC_SAND_CHECK_FUNC_RESULT(res, 30, exit) ;
    sal_printf(
      "%s(): Found matching element (%d found_equal %d) for priority %d\r\n",__FUNCTION__,iter,found_equal,prio) ;
    prio = priority[priority_profile][0] + 1 ;
    soc_sand_db_prio_list_priority_value_encode(prio, key_buffer) ;
    res = soc_sand_sorted_list_find_higher_eq_key(unit,priorities,key_buffer,&iter,&found_equal) ;
    SOC_SAND_CHECK_FUNC_RESULT(res, 34, exit) ;
    sal_printf(
      "%s(): Found matching element (%d found_equal %d) for priority %d\r\n",__FUNCTION__,iter,found_equal,prio) ;
    prio = priority[priority_profile][2] - 1 ;
    soc_sand_db_prio_list_priority_value_encode(prio, key_buffer) ;
    res = soc_sand_sorted_list_find_higher_eq_key(unit,priorities,key_buffer,&iter,&found_equal) ;
    SOC_SAND_CHECK_FUNC_RESULT(res, 38, exit) ;
    sal_printf(
      "%s(): Found matching element (%d found_equal %d) for priority %d\r\n",__FUNCTION__,iter,found_equal,prio) ;
  }
  if (1)
  {
    /*
     * Now exercise bisect. Use soc_sand_sorted_list_find_lower_eq_key()
     */
    SOC_SAND_SORTED_LIST_ITER
      iter ;
    uint32
      index_on_indices ;
    prio = priority[priority_profile][1] ;
    soc_sand_db_prio_list_priority_value_encode(prio, key_buffer) ;
    res = soc_sand_sorted_list_find_lower_eq_key(unit,priorities,key_buffer,&iter,&index_on_indices,&found_equal) ;
    SOC_SAND_CHECK_FUNC_RESULT(res, 42, exit) ;
    sal_printf(
      "%s(): Found matching element (%d found_equal %d) for priority %d index_on_indices %d\r\n",
        __FUNCTION__,iter,found_equal,prio,index_on_indices) ;
    prio = priority[priority_profile][4] + 1 ;
    soc_sand_db_prio_list_priority_value_encode(prio, key_buffer) ;
    res = soc_sand_sorted_list_find_lower_eq_key(unit,priorities,key_buffer,&iter,&index_on_indices,&found_equal) ;
    SOC_SAND_CHECK_FUNC_RESULT(res, 46, exit) ;
    sal_printf(
      "%s(): Found matching element (%d found_equal %d) for priority %d index_on_indices %d\r\n",
        __FUNCTION__,iter,found_equal,prio,index_on_indices) ;
    prio = priority[priority_profile][0] ;
    soc_sand_db_prio_list_priority_value_encode(prio, key_buffer) ;
    res = soc_sand_sorted_list_find_lower_eq_key(unit,priorities,key_buffer,&iter,&index_on_indices,&found_equal) ;
    SOC_SAND_CHECK_FUNC_RESULT(res, 50, exit) ;
    sal_printf(
      "%s(): Found matching element (%d found_equal %d) for priority %d index_on_indices %d\r\n",
        __FUNCTION__,iter,found_equal,prio,index_on_indices) ;
    prio = priority[priority_profile][2] ;
    soc_sand_db_prio_list_priority_value_encode(prio, key_buffer) ;
    res = soc_sand_sorted_list_find_lower_eq_key(unit,priorities,key_buffer,&iter,&index_on_indices,&found_equal) ;
    SOC_SAND_CHECK_FUNC_RESULT(res, 54, exit) ;
    sal_printf(
      "%s(): Found matching element (%d found_equal %d) for priority %d index_on_indices %d\r\n",
        __FUNCTION__,iter,found_equal,prio,index_on_indices) ;
    prio = priority[priority_profile][0] + 1 ;
    soc_sand_db_prio_list_priority_value_encode(prio, key_buffer) ;
    res = soc_sand_sorted_list_find_lower_eq_key(unit,priorities,key_buffer,&iter,&index_on_indices,&found_equal) ;
    SOC_SAND_CHECK_FUNC_RESULT(res, 58, exit) ;
    sal_printf(
      "%s(): Found matching element (%d found_equal %d) for priority %d index_on_indices %d\r\n",
        __FUNCTION__,iter,found_equal,prio,index_on_indices) ;
    prio = priority[priority_profile][2] - 1 ;
    soc_sand_db_prio_list_priority_value_encode(prio, key_buffer) ;
    res = soc_sand_sorted_list_find_lower_eq_key(unit,priorities,key_buffer,&iter,&index_on_indices,&found_equal) ;
    SOC_SAND_CHECK_FUNC_RESULT(res, 62, exit) ;
    sal_printf(
      "%s(): Found matching element (%d found_equal %d) for priority %d index_on_indices %d\r\n",
        __FUNCTION__,iter,found_equal,prio,index_on_indices) ;
    /*
     * Exercise removal of one element.
     * priority[priority_profile][1]
     */
    prio = priority[priority_profile][1] ;
    soc_sand_db_prio_list_priority_value_encode(prio, key_buffer) ;
    res = soc_sand_sorted_list_find_lower_eq_key(unit,priorities,key_buffer,&iter,&index_on_indices,&found_equal) ;
    SOC_SAND_CHECK_FUNC_RESULT(res, 66, exit) ;
    res = soc_sand_sorted_list_get_num_elements(unit,priorities,&num_elements) ;
    SOC_SAND_CHECK_FUNC_RESULT(res, 70, exit) ;
    sal_printf("%s(): Original num of element %d. Now, remove element %d\r\n",__FUNCTION__,num_elements,iter) ;
    res = soc_sand_sorted_list_entry_remove_by_iter(unit,priorities,iter) ;
    SOC_SAND_CHECK_FUNC_RESULT(res, 74, exit) ;
    res = soc_sand_sorted_list_get_num_elements(unit,priorities,&num_elements) ;
    SOC_SAND_CHECK_FUNC_RESULT(res, 70, exit) ;
    sal_printf("%s(): num of element %d\r\n",__FUNCTION__,num_elements) ;
    soc_sand_print_indices(unit,priorities) ;
    soc_sand_print_list(unit,priorities) ;
    /*
     * Exercise removal of one element.
     * priority[priority_profile][0]
     */
    prio = priority[priority_profile][0] ;
    soc_sand_db_prio_list_priority_value_encode(prio, key_buffer) ;
    res = soc_sand_sorted_list_find_lower_eq_key(unit,priorities,key_buffer,&iter,&index_on_indices,&found_equal) ;
    SOC_SAND_CHECK_FUNC_RESULT(res, 66, exit) ;
    res = soc_sand_sorted_list_get_num_elements(unit,priorities,&num_elements) ;
    SOC_SAND_CHECK_FUNC_RESULT(res, 70, exit) ;
    sal_printf("%s(): Original num of element %d. Now, remove element %d\r\n",__FUNCTION__,num_elements,iter) ;
    res = soc_sand_sorted_list_entry_remove_by_iter(unit,priorities,iter) ;
    SOC_SAND_CHECK_FUNC_RESULT(res, 74, exit) ;
    res = soc_sand_sorted_list_get_num_elements(unit,priorities,&num_elements) ;
    SOC_SAND_CHECK_FUNC_RESULT(res, 70, exit) ;
    sal_printf("%s(): num of element %d\r\n",__FUNCTION__,num_elements) ;
    soc_sand_print_indices(unit,priorities) ;
    soc_sand_print_list(unit,priorities) ;
    /*
     * Exercise removal of one element.
     * priority[priority_profile][2]
     */
    prio = priority[priority_profile][2] ;
    soc_sand_db_prio_list_priority_value_encode(prio, key_buffer) ;
    res = soc_sand_sorted_list_find_lower_eq_key(unit,priorities,key_buffer,&iter,&index_on_indices,&found_equal) ;
    SOC_SAND_CHECK_FUNC_RESULT(res, 66, exit) ;
    res = soc_sand_sorted_list_get_num_elements(unit,priorities,&num_elements) ;
    SOC_SAND_CHECK_FUNC_RESULT(res, 70, exit) ;
    sal_printf("%s(): Original num of element %d. Now, remove element %d\r\n",__FUNCTION__,num_elements,iter) ;
    res = soc_sand_sorted_list_entry_remove_by_iter(unit,priorities,iter) ;
    SOC_SAND_CHECK_FUNC_RESULT(res, 74, exit) ;
    res = soc_sand_sorted_list_get_num_elements(unit,priorities,&num_elements) ;
    SOC_SAND_CHECK_FUNC_RESULT(res, 70, exit) ;
    sal_printf("%s(): num of element %d\r\n",__FUNCTION__,num_elements) ;
    soc_sand_print_indices(unit,priorities) ;
    soc_sand_print_list(unit,priorities) ;
  }
exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in soc_sand_exercise_01()", 0, 0);
}

uint32
  soc_sand_SAND_SORTED_LIST_INFO_print(
    SOC_SAND_IN int                      unit,
    SOC_SAND_IN SOC_SAND_SORTED_LIST_PTR sorted_list
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

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);
  SOC_SAND_SORTED_LIST_VERIFY_UNIT_IS_LEGAL(unit, 2) ;
  SOC_SAND_SORTED_LIST_CONVERT_HANDLE_TO_SORTEDLIST_INDEX(sorted_list_index,sorted_list) ;
  SOC_SAND_SORTED_LIST_VERIFY_SORTEDLIST_IS_ACTIVE(sorted_list_index,4,6) ;

  res = SORTED_LIST_ACCESS_INFO.data_size.get(unit,sorted_list_index,&data_size) ;
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 8, exit) ;
  res = SORTED_LIST_ACCESS_INFO.key_size.get(unit,sorted_list_index,&key_size) ;
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 10, exit) ;
  res = SORTED_LIST_ACCESS_INFO.list_size.get(unit,sorted_list_index,&list_size) ;
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 12, exit) ;
  res = SORTED_LIST_ACCESS_INFO.prime_handle.get(unit,sorted_list_index,&prime_handle) ;
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 14, exit) ;
  res = SORTED_LIST_ACCESS_INFO.sec_handle.get(unit,sorted_list_index,&sec_handle) ;
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 16, exit) ;

  LOG_CLI((BSL_META_U(unit,"init_info.data_size   : %u\n"),data_size)) ;
  LOG_CLI((BSL_META_U(unit,"init_info.key_size    : %u\n"),key_size)) ;
  LOG_CLI((BSL_META_U(unit,"init_info.list_size   : %u\n"),list_size)) ;
  LOG_CLI((BSL_META_U(unit,"init_info.prime_handle: %u\n"),prime_handle)) ;
  LOG_CLI((BSL_META_U(unit,"init_info.sec_handle  : %u\n"),sec_handle)) ;
exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in soc_sand_SAND_SORTED_LIST_INFO_print()", 0, 0) ;
}
/* } */
#endif /* SOC_SAND_DEBUG */
/*
 * Utilities related to the 'indices' feature.
 */
/* { */
/*********************************************************************
* NAME:
*     soc_sand_sorted_list_find_higher_eq_key
* TYPE:
*   PROC
* DATE:
*   Dec 01 2015
* FUNCTION:
*  Get location, on array of available linked list elements, of the first
*  element with priority (key) higher than or equal to specified on input.
*  (Only available when 'indices' feature is enabled on specified
*  sorted list). See REMARKS below.
* INPUT:
*   SOC_SAND_IN  int                              unit -
*     Identifier of the device to access.
*   SOC_SAND_IN  SOC_SAND_SORTED_LIST_PTR         sorted_list -
*     Handle to the sorted list.
*   SOC_SAND_IN  SOC_SAND_SORTED_LIST_KEY         *const key -
*     Key to use as reference. Assumed to be an array of
*     ARAD_TCAM_DB_LIST_KEY_SIZE characters. (Actually, ARAD_TCAM_DB_LIST_KEY_SIZE
*     is assumed to be sizeof(uint32))
*   SOC_SAND_OUT SOC_SAND_SORTED_LIST_ITER        *iter -
*     Index (location), on array of available linked list elements
*     for this linked list, of element with higher/eq priority.
*     If input key is strictly higher than highest element on sorted
*     list (i.e. last valid element) then return:
*       SOC_SAND_SORTED_LIST_ITER_END((unit,sorted_list)
*     If sorted list is empty then return:
*       SOC_SAND_SORTED_LIST_ITER_END(unit,sorted_list)
*   SOC_SAND_OUT uint8                            *found_equal -
*     If matching element was found and its priority was exactly equal
*     to that of input key then this procedure loads pointed memory
*     by a non-zero value. Otherwise, it is loaded by zero.
* REMARKS:
*   Key is converted to priority using
*     arad_tcam_db_prio_list_priority_value_decode()
*
*   This procedure is only valid when 'indices' feature is enabled
*   on specified sorted list.
*
*   Key/priority comparison is done by a procedure as indicated on
*     SORTED_LIST_ACCESS_INFO.cmp_func_type
* RETURNS:
*   OK or ERROR indication.
*********************************************************************/
uint32
  soc_sand_sorted_list_find_higher_eq_key(
    SOC_SAND_IN  int                       unit,
    SOC_SAND_IN  SOC_SAND_SORTED_LIST_PTR  sorted_list,
    SOC_SAND_IN  SOC_SAND_SORTED_LIST_KEY  *const key,
    SOC_SAND_OUT SOC_SAND_SORTED_LIST_ITER *iter,
    SOC_SAND_OUT uint8                     *found_equal
  )
{
  uint32
    sorted_list_index,
    matched,
    index_on_indices,
    top_index_on_indices,
    bottom_index_on_indices,
    res ;
  uint8
    is_allocated ;
  uint32
    num_elements ;
  SOC_SAND_SORTED_LIST_ITER
    local_iter ;
  SOC_SAND_SORTED_LIST_KEY
    element_key[ARAD_TCAM_DB_LIST_KEY_SIZE] ;
  SOC_SAND_SORTED_LIST_KEY_CMP 
    cmp_key_fun ;
  SOC_SAND_SORTED_LIST_CMP_FUNC_TYPE
    cmp_func_type ;
  int32
    compare_res ;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0) ;
  SOC_SAND_SORTED_LIST_VERIFY_UNIT_IS_LEGAL(unit, 2) ;
  SOC_SAND_SORTED_LIST_CONVERT_HANDLE_TO_SORTEDLIST_INDEX(sorted_list_index,sorted_list) ;
  SOC_SAND_SORTED_LIST_VERIFY_SORTEDLIST_IS_ACTIVE(sorted_list_index,4,6) ;

  *iter = SOC_SAND_SORTED_LIST_ITER_END(unit,sorted_list) ;
  *found_equal = 0 ;
  res = SORTED_LIST_ACCESS_DATA.indices.is_allocated(unit,sorted_list_index,&is_allocated) ;
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 10, exit) ;
  if (!is_allocated)
  {
    res = SOC_SAND_SORTED_LIST_ILLEGAL_ITER_ERR ;
    SOC_SAND_CHECK_FUNC_RESULT(res, 14, exit) ;
  }
  res = SORTED_LIST_ACCESS_INFO.cmp_func_type.get(unit,sorted_list_index,&cmp_func_type) ;
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 16, exit);
  if (cmp_func_type == SOC_SAND_SORTED_LIST_CMP_FUNC_TYPE_TCAM)
  {
    cmp_key_fun = soc_sand_sorted_list_tcam_cmp_priority ;
  }
  else
  {
    cmp_key_fun = (SOC_SAND_SORTED_LIST_KEY_CMP)soc_sand_os_memcmp ;
  }
  res = soc_sand_sorted_list_get_num_elements(unit,sorted_list,&num_elements) ;
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 18, exit) ;
  if (num_elements == 0)
  {
    /*
     * List is empty. Return a value of SOC_SAND_SORTED_LIST_ITER_END
     */
    goto exit ;
  }
  /*
   * If input priority is higher than highest on list then return
   * a value of SOC_SAND_SORTED_LIST_ITER_END
   */
  res = soc_sand_sorted_list_get_iter_from_indices(unit,sorted_list,num_elements - 1,&local_iter) ;
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 22, exit) ;
  res = soc_sand_sorted_list_entry_value(unit,sorted_list,local_iter,element_key,NULL) ;
  SOC_SAND_CHECK_FUNC_RESULT(res, 26, exit) ;
  compare_res = cmp_key_fun(key,element_key,sizeof(element_key)) ;
  /*
   * Comparison procedure does: compare_res=key-element_key
   */
  if (compare_res > 0)
  {
    /*
     * List has no higher element. Return a value of SOC_SAND_SORTED_LIST_ITER_END
     */
    goto exit ;
  }
  /*
   * If input priority is lower than lowest on list then point to the lowest
   */
  res = soc_sand_sorted_list_get_iter_from_indices(unit,sorted_list,0,&local_iter) ;
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 30, exit) ;
  res = soc_sand_sorted_list_entry_value(unit,sorted_list,local_iter,element_key,NULL) ;
  SOC_SAND_CHECK_FUNC_RESULT(res, 34, exit) ;
  compare_res = cmp_key_fun(key,element_key,sizeof(element_key)) ;
  if (compare_res <= 0)
  {
    /*
     * First element is already higher than input priority (key).
     */
    if (compare_res == 0)
    {
      *found_equal = 1 ;
    }
    *iter = local_iter ;
    goto exit ;
  }
  /*
   * Input key is within range of the sorted list. Solution MUST be found or there is some error.
   */
  matched = 0 ;
  top_index_on_indices = num_elements - 1 ;
  bottom_index_on_indices = 0 ;
  while (!matched)
  {
    index_on_indices = (top_index_on_indices - bottom_index_on_indices) / 2 ;
    if (index_on_indices == 0)
    {
      /*
       * We have one or two elements to check. Start by checking the lower one:
       */
      res = soc_sand_sorted_list_get_iter_from_indices(unit,sorted_list,bottom_index_on_indices,&local_iter) ;
      SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 38, exit) ;
      res = soc_sand_sorted_list_entry_value(unit,sorted_list,local_iter,element_key,NULL) ;
      SOC_SAND_CHECK_FUNC_RESULT(res, 42, exit) ;
      compare_res = cmp_key_fun(key,element_key,sizeof(element_key)) ;
      if (compare_res <= 0)
      {
        matched = 1 ;
        if (compare_res == 0)
        {
          *found_equal = 1 ;
        }
        *iter = local_iter ;
        goto exit ;
      }
      res = soc_sand_sorted_list_get_iter_from_indices(unit,sorted_list,top_index_on_indices,&local_iter) ;
      SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 46, exit) ;
      res = soc_sand_sorted_list_entry_value(unit,sorted_list,local_iter,element_key,NULL) ;
      SOC_SAND_CHECK_FUNC_RESULT(res, 50, exit) ;
      compare_res = cmp_key_fun(key,element_key,sizeof(element_key)) ;
      if (compare_res <= 0)
      {
        matched = 2 ;
        if (compare_res == 0)
        {
          *found_equal = 1 ;
        }
        *iter = local_iter ;
        goto exit ;
      }
      /*
       * Failed to find matching element. Error!
       */
      res = SOC_SAND_GET_ERR_TXT_ERR ;
      SOC_SAND_CHECK_FUNC_RESULT(res, 52, exit) ;
    }
    index_on_indices += bottom_index_on_indices ;
    /*
     * There are more-than-one elements to be inspected.
     */
    res = soc_sand_sorted_list_get_iter_from_indices(unit,sorted_list,index_on_indices,&local_iter) ;
    SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 38, exit) ;
    res = soc_sand_sorted_list_entry_value(unit,sorted_list,local_iter,element_key,NULL) ;
    SOC_SAND_CHECK_FUNC_RESULT(res, 42, exit) ;
    /*
     * Comparison procedure does: compare_res=key-element_key
     */
    compare_res = cmp_key_fun(key,element_key,sizeof(element_key)) ;
    if (compare_res <= 0)
    {
      /*
       * Found an element which is higher than input. Bisect the lower half.
       */
      top_index_on_indices = index_on_indices ;
      continue ;
    }
    else
    {
      /*
       * Middle element which is lower than input. Bisect the upper half.
       */
      bottom_index_on_indices = index_on_indices + 1 ;
      continue ;
    }
  }
exit:
  SOC_SAND_EXIT_AND_SEND_ERROR( "error in soc_sand_sorted_list_find_higher_eq_key()",0,0);
}

/*********************************************************************
* NAME:
*     soc_sand_sorted_list_find_lower_eq_key
* TYPE:
*   PROC
* DATE:
*   Dec 01 2015
* FUNCTION:
*  Get location, on array of available linked list elements, of the first
*  element with priority (key) lower than or equal to specified on input.
*  (Only available when 'indices' feature is enabled on specified
*  sorted list). See REMARKS below.
* INPUT:
*   SOC_SAND_IN  int                              unit -
*     Identifier of the device to access.
*   SOC_SAND_IN  SOC_SAND_SORTED_LIST_PTR         sorted_list -
*     Handle to the sorted list.
*   SOC_SAND_IN  SOC_SAND_SORTED_LIST_KEY         *const key -
*     Key to use as reference. Assumed to be an array of
*     ARAD_TCAM_DB_LIST_KEY_SIZE characters. (Actually, ARAD_TCAM_DB_LIST_KEY_SIZE
*     is assumed to be sizeof(uint32))
*   SOC_SAND_OUT SOC_SAND_SORTED_LIST_ITER        *iter -
*     Index (location), on array of available linked list elements
*     for this linked list, of element with lower/eq priority.
*     If input key is strictly lower than lowest element on sorted
*     list (i.e. last valid element) then return:
*       SOC_SAND_SORTED_LIST_ITER_BEGIN((unit,sorted_list)
*     If sorted list is empty then return:
*       SOC_SAND_SORTED_LIST_ITER_BEGIN(unit,sorted_list)
*   SOC_SAND_OUT uint32                           *index_p -
*     Index, on array containing pointers (indices) of of available
*     linked list elements arranged by priority. See 'iter' above.
*     If input key is strictly lower than lowest element on sorted
*     If there is no element on sorted list which matches required
*     criterion then return '-1'.
*     Note: In this latter case, '*iter' will be:
*       SOC_SAND_SORTED_LIST_ITER_BEGIN((unit,sorted_list)
*   SOC_SAND_OUT uint8                            *found_equal -
*     If matching element was found and its priority was exactly equal
*     to that of input key then this procedure loads pointed memory
*     by a non-zero value. Otherwise, it is loaded by zero.
* REMARKS:
*   Key is converted to priority using
*     arad_tcam_db_prio_list_priority_value_decode()
*
*   This procedure is only valid when 'indices' feature is enabled
*   on specified sorted list.
*
*   Key/priority comparison is done by a procedure as indicated on
*     SORTED_LIST_ACCESS_INFO.cmp_func_type
* RETURNS:
*   OK or ERROR indication.
*********************************************************************/
uint32
  soc_sand_sorted_list_find_lower_eq_key(
    SOC_SAND_IN  int                       unit,
    SOC_SAND_IN  SOC_SAND_SORTED_LIST_PTR  sorted_list,
    SOC_SAND_IN  SOC_SAND_SORTED_LIST_KEY  *const key,
    SOC_SAND_OUT SOC_SAND_SORTED_LIST_ITER *iter,
    SOC_SAND_OUT uint32                    *index_p,
    SOC_SAND_OUT uint8                     *found_equal
  )
{
  uint32
    sorted_list_index,
    matched,
    index_on_indices,
    top_index_on_indices,
    bottom_index_on_indices,
    res ;
  uint8
    is_allocated ;
  uint32
    num_elements ;
  SOC_SAND_SORTED_LIST_ITER
    local_iter ;
  SOC_SAND_SORTED_LIST_KEY
    element_key[ARAD_TCAM_DB_LIST_KEY_SIZE] ;
  SOC_SAND_SORTED_LIST_KEY_CMP 
    cmp_key_fun ;
  SOC_SAND_SORTED_LIST_CMP_FUNC_TYPE
    cmp_func_type ;
  int32
    compare_res ;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0) ;
  SOC_SAND_SORTED_LIST_VERIFY_UNIT_IS_LEGAL(unit, 2) ;
  SOC_SAND_SORTED_LIST_CONVERT_HANDLE_TO_SORTEDLIST_INDEX(sorted_list_index,sorted_list) ;
  SOC_SAND_SORTED_LIST_VERIFY_SORTEDLIST_IS_ACTIVE(sorted_list_index,4,6) ;

  *iter = SOC_SAND_SORTED_LIST_ITER_BEGIN(unit,sorted_list) ;
  *index_p = -1 ;
  *found_equal = 0 ;
  res = SORTED_LIST_ACCESS_DATA.indices.is_allocated(unit,sorted_list_index,&is_allocated) ;
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 10, exit) ;
  if (!is_allocated)
  {
    res = SOC_SAND_SORTED_LIST_ILLEGAL_ITER_ERR ;
    SOC_SAND_CHECK_FUNC_RESULT(res, 14, exit) ;
  }
  res = SORTED_LIST_ACCESS_INFO.cmp_func_type.get(unit,sorted_list_index,&cmp_func_type) ;
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 16, exit);
  if (cmp_func_type == SOC_SAND_SORTED_LIST_CMP_FUNC_TYPE_TCAM)
  {
    cmp_key_fun = soc_sand_sorted_list_tcam_cmp_priority ;
  }
  else
  {
    cmp_key_fun = (SOC_SAND_SORTED_LIST_KEY_CMP)soc_sand_os_memcmp ;
  }
  res = soc_sand_sorted_list_get_num_elements(unit,sorted_list,&num_elements) ;
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 18, exit) ;
  if (num_elements == 0)
  {
    /*
     * List is empty. Return a value of SOC_SAND_SORTED_LIST_ITER_BEGIN
     */
    goto exit ;
  }
  /*
   * If input priority is lower than lowest on list then return
   * a value of SOC_SAND_SORTED_LIST_ITER_BEGIN
   */
  res = soc_sand_sorted_list_get_iter_from_indices(unit,sorted_list,0,&local_iter) ;
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 22, exit) ;
  res = soc_sand_sorted_list_entry_value(unit,sorted_list,local_iter,element_key,NULL) ;
  SOC_SAND_CHECK_FUNC_RESULT(res, 26, exit) ;
  compare_res = cmp_key_fun(key,element_key,sizeof(element_key)) ;
  /*
   * Comparison procedure does: compare_res=key-element_key
   */
  if (compare_res < 0)
  {
    /*
     * List has no lower element. Return a value of SOC_SAND_SORTED_LIST_ITER_BEGIN
     */
    goto exit ;
  }
  /*
   * If input priority is higher than highest on list then point to the highest
   */
  index_on_indices = num_elements - 1 ;
  res = soc_sand_sorted_list_get_iter_from_indices(unit,sorted_list,index_on_indices,&local_iter) ;
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 30, exit) ;
  res = soc_sand_sorted_list_entry_value(unit,sorted_list,local_iter,element_key,NULL) ;
  SOC_SAND_CHECK_FUNC_RESULT(res, 34, exit) ;
  compare_res = cmp_key_fun(key,element_key,sizeof(element_key)) ;
  /*
   * Comparison procedure does: compare_res=key-element_key
   */
  if (compare_res >= 0)
  {
    /*
     * Last element is lower than input priority (key).
     */
    if (compare_res == 0)
    {
      *found_equal = 1 ;
    }
    *index_p = index_on_indices ;
    *iter = local_iter ;
    goto exit ;
  }
  /*
   * Input key is within range of the sorted list. Solution MUST be found or there is some error.
   */
  matched = 0 ;
  top_index_on_indices = num_elements - 1 ;
  bottom_index_on_indices = 0 ;
  while (!matched)
  {
    index_on_indices = (top_index_on_indices - bottom_index_on_indices) / 2 ;
    if (index_on_indices == 0)
    {
      /*
       * We have one or two elements to check. Start by checking the upper one:
       */
      index_on_indices = top_index_on_indices ;
      res = soc_sand_sorted_list_get_iter_from_indices(unit,sorted_list,index_on_indices,&local_iter) ;
      SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 38, exit) ;
      res = soc_sand_sorted_list_entry_value(unit,sorted_list,local_iter,element_key,NULL) ;
      SOC_SAND_CHECK_FUNC_RESULT(res, 42, exit) ;
      compare_res = cmp_key_fun(key,element_key,sizeof(element_key)) ;
      /*
       * Comparison procedure does: compare_res=key-element_key
       */
      if (compare_res >= 0)
      {
        matched = 1 ;
        if (compare_res == 0)
        {
          *found_equal = 1 ;
        }
        *index_p = index_on_indices ;
        *iter = local_iter ;
        goto exit ;
      }
      index_on_indices = bottom_index_on_indices ;
      res = soc_sand_sorted_list_get_iter_from_indices(unit,sorted_list,index_on_indices,&local_iter) ;
      SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 46, exit) ;
      res = soc_sand_sorted_list_entry_value(unit,sorted_list,local_iter,element_key,NULL) ;
      SOC_SAND_CHECK_FUNC_RESULT(res, 50, exit) ;
      compare_res = cmp_key_fun(key,element_key,sizeof(element_key)) ;
      if (compare_res >= 0)
      {
        matched = 2 ;
        if (compare_res == 0)
        {
          *found_equal = 1 ;
        }
        *index_p = index_on_indices ;
        *iter = local_iter ;
        goto exit ;
      }
      /*
       * Failed to find matching element. Error!
       */
      res = SOC_SAND_GET_ERR_TXT_ERR ;
      SOC_SAND_CHECK_FUNC_RESULT(res, 52, exit) ;
    }
    index_on_indices += bottom_index_on_indices ;
    /*
     * There are more-than-one elements to be inspected.
     */
    res = soc_sand_sorted_list_get_iter_from_indices(unit,sorted_list,index_on_indices,&local_iter) ;
    SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 38, exit) ;
    res = soc_sand_sorted_list_entry_value(unit,sorted_list,local_iter,element_key,NULL) ;
    SOC_SAND_CHECK_FUNC_RESULT(res, 42, exit) ;
    /*
     * Comparison procedure does: compare_res=key-element_key
     */
    compare_res = cmp_key_fun(key,element_key,sizeof(element_key)) ;
    if (compare_res >= 0)
    {
      /*
       * Found an element which is lower than input. Bisect the upper half.
       */
      bottom_index_on_indices = index_on_indices ;
      continue ;
    }
    else
    {
      /*
       * Found an element which is higher than input. Bisect the lower half.
       */
      top_index_on_indices = index_on_indices - 1 ;
      continue ;
    }
  }
exit:
  SOC_SAND_EXIT_AND_SEND_ERROR( "error in soc_sand_sorted_list_find_lower_eq_key()",0,0);
}

/*********************************************************************
* NAME:
*     soc_sand_sorted_list_get_num_elements
* TYPE:
*   PROC
* DATE:
*   Dec 01 2015
* FUNCTION:
*  Get current number of elements on specified sorted list. (This is
*  also the number of elements on 'indices' array)
* INPUT:
*   SOC_SAND_IN  int                              unit -
*     Identifier of the device to access.
*   SOC_SAND_IN  SOC_SAND_SORTED_LIST_PTR         sorted_list -
*     Handle to the sorted list.
*   SOC_SAND_OUT uint32                           *num_elements -
*     Number of valid elements currently on the list.
*     Only meaningful if the 'indices' feature is enabled ('indices'
*     element in SOC_SAND_SORTED_LIST_T is allocated). 
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  soc_sand_sorted_list_get_num_elements(
    SOC_SAND_IN  int                      unit,
    SOC_SAND_IN  SOC_SAND_SORTED_LIST_PTR sorted_list,
    SOC_SAND_OUT uint32                   *num_elements
  )
{
  uint32
    sorted_list_index,
    res ;
  uint8
    is_allocated ;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0) ;
  SOC_SAND_SORTED_LIST_VERIFY_UNIT_IS_LEGAL(unit, 2) ;
  SOC_SAND_SORTED_LIST_CONVERT_HANDLE_TO_SORTEDLIST_INDEX(sorted_list_index,sorted_list) ;
  SOC_SAND_SORTED_LIST_VERIFY_SORTEDLIST_IS_ACTIVE(sorted_list_index,4,6) ;

  res = SORTED_LIST_ACCESS_DATA.indices.is_allocated(unit,sorted_list_index,&is_allocated) ;
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 10, exit) ;
  if (is_allocated)
  {
    res = SORTED_LIST_ACCESS_DATA.num_elements_on_indices.get(unit,sorted_list_index,num_elements) ;
    SOC_SAND_CHECK_FUNC_RESULT(res, 12, exit) ;
  }
  else
  {
    res = SOC_SAND_SORTED_LIST_ILLEGAL_ITER_ERR ;
    SOC_SAND_CHECK_FUNC_RESULT(res, 14, exit) ;
  }
exit:
  SOC_SAND_EXIT_AND_SEND_ERROR( "error in soc_sand_sorted_list_get_num_elements()",0,0);
}
/*********************************************************************
* NAME:
*     soc_sand_sorted_list_is_indices_enabled
* TYPE:
*   PROC
* DATE:
*   Dec 01 2015
* FUNCTION:
*  Get indication on whether the 'indices' feature is enabled for this
*  sorted list.
* INPUT:
*   SOC_SAND_IN  int                              unit -
*     Identifier of the device to access.
*   SOC_SAND_IN  SOC_SAND_SORTED_LIST_PTR         sorted_list -
*     Handle to the sorted list.
*   SOC_SAND_OUT uint8                            *is_enabled -
*     Indication on whether 'indices' feature is enabled. Non-zero
*     indicates 'enabled'
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  soc_sand_sorted_list_is_indices_enabled(
    SOC_SAND_IN  int                      unit,
    SOC_SAND_IN  SOC_SAND_SORTED_LIST_PTR sorted_list,
    SOC_SAND_OUT uint8                    *is_enabled
  )
{
  uint32
    sorted_list_index,
    res ;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0) ;
  SOC_SAND_SORTED_LIST_VERIFY_UNIT_IS_LEGAL(unit, 2) ;
  SOC_SAND_SORTED_LIST_CONVERT_HANDLE_TO_SORTEDLIST_INDEX(sorted_list_index,sorted_list) ;
  SOC_SAND_SORTED_LIST_VERIFY_SORTEDLIST_IS_ACTIVE(sorted_list_index,4,6) ;

  res = SORTED_LIST_ACCESS_DATA.indices.is_allocated(unit,sorted_list_index,is_enabled) ;
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 10, exit) ;
exit:
  SOC_SAND_EXIT_AND_SEND_ERROR( "error in soc_sand_sorted_list_is_indices_enabled()",0,0);
}
/*********************************************************************
* NAME:
*     soc_sand_sorted_list_get_iter_from_indices
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
*   SOC_SAND_IN  int                              unit -
*     Identifier of the device to access.
*   SOC_SAND_IN  SOC_SAND_SORTED_LIST_PTR         sorted_list -
*     Handle to the sorted list.
*   SOC_SAND_IN  uint32                           index_in_indices -
*     Index on 'indices' array to use to get the corresponding index
*     on array of available link-list elements.
*     Only meaningful if the 'indices' feature is enabled ('indices'
*     element in SOC_SAND_SORTED_LIST_T is allocated). Note that this
*     index is counted from the BOTTOM of 'indices'. So, for example,
*     when index=0 then returned entry refers to LOWEST key.
*   SOC_SAND_OUT SOC_SAND_SORTED_LIST_ITER        *iter -
*     Index (location) on array of available linked list elements
*     for this linked list
* REMARKS:
*   Use soc_sand_sorted_list_entry_value() to get 'key' or 'data'
*   of corresponding linked list element.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  soc_sand_sorted_list_get_iter_from_indices(
    SOC_SAND_IN  int                       unit,
    SOC_SAND_IN  SOC_SAND_SORTED_LIST_PTR  sorted_list,
    SOC_SAND_IN  uint32                    index_in_indices,
    SOC_SAND_OUT SOC_SAND_SORTED_LIST_ITER *iter
  )
{
  uint32
    sorted_list_index,
    res ;
  uint8
    is_allocated ;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0) ;
  SOC_SAND_SORTED_LIST_VERIFY_UNIT_IS_LEGAL(unit, 2) ;
  SOC_SAND_SORTED_LIST_CONVERT_HANDLE_TO_SORTEDLIST_INDEX(sorted_list_index,sorted_list) ;
  SOC_SAND_SORTED_LIST_VERIFY_SORTEDLIST_IS_ACTIVE(sorted_list_index,4,6) ;

  res = SORTED_LIST_ACCESS_DATA.indices.is_allocated(unit,sorted_list_index,&is_allocated) ;
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 10, exit) ;
  if (is_allocated)
  {
    res = SORTED_LIST_ACCESS_DATA.indices.get(unit,sorted_list_index,index_in_indices,iter) ;
    SOC_SAND_CHECK_FUNC_RESULT(res, 12, exit) ;
  }
  else
  {
    res = SOC_SAND_SORTED_LIST_ILLEGAL_ITER_ERR ;
    SOC_SAND_CHECK_FUNC_RESULT(res, 14, exit) ;
  }
exit:
  SOC_SAND_EXIT_AND_SEND_ERROR( "error in soc_sand_sorted_list_get_iter_from_indices()",0,0);
}
/*********************************************************************
* NAME:
*     soc_sand_sorted_list_get_index_from_iter
* TYPE:
*   PROC
* DATE:
*   Dec 06 2015
* FUNCTION:
*  Given index on array of available linked list elements for this
*  linked list, get index within 'indices' (which is the ordinal
*  location of a sorted linked list element, starting from low key
*  value).
* INPUT:
*   SOC_SAND_IN  int                              unit -
*     Identifier of the device to access.
*   SOC_SAND_IN  SOC_SAND_SORTED_LIST_PTR         sorted_list -
*     Handle to the sorted list.
*   SOC_SAND_IN SOC_SAND_SORTED_LIST_ITER         iter -
*     Index (location) on array of available linked list elements
*     for this linked list
*   SOC_SAND_OUT  uint32                          *index_in_indices -
*     Index on 'indices' array corresponding to index on array
*     of available link-list elements (iter). If no matching index
*     is found, return '-1'
*     Only meaningful if the 'indices' feature is enabled ('indices'
*     element in SOC_SAND_SORTED_LIST_T is allocated).
* REMARKS:
*   Use soc_sand_sorted_list_entry_value() to get 'key' or 'data'
*   of corresponding linked list element.
*   See 
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  soc_sand_sorted_list_get_index_from_iter(
    SOC_SAND_IN  int                       unit,
    SOC_SAND_IN  SOC_SAND_SORTED_LIST_PTR  sorted_list,
    SOC_SAND_IN  SOC_SAND_SORTED_LIST_ITER iter,
    SOC_SAND_OUT uint32                    *index_in_indices
  )
{
  uint32
    sorted_list_index,
    res ;
  uint8
    is_allocated ;
  uint8
    found_equal ;
  SOC_SAND_SORTED_LIST_ITER
    local_iter ;
  SOC_SAND_SORTED_LIST_KEY
    element_key[ARAD_TCAM_DB_LIST_KEY_SIZE] ;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0) ;
  SOC_SAND_SORTED_LIST_VERIFY_UNIT_IS_LEGAL(unit, 2) ;
  SOC_SAND_SORTED_LIST_CONVERT_HANDLE_TO_SORTEDLIST_INDEX(sorted_list_index,sorted_list) ;
  SOC_SAND_SORTED_LIST_VERIFY_SORTEDLIST_IS_ACTIVE(sorted_list_index,4,6) ;

  res = SORTED_LIST_ACCESS_DATA.indices.is_allocated(unit,sorted_list_index,&is_allocated) ;
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 10, exit) ;
  if (!is_allocated)
  {
    res = SOC_SAND_SORTED_LIST_ILLEGAL_ITER_ERR ;
    SOC_SAND_CHECK_FUNC_RESULT(res, 14, exit) ;
  }
  res = soc_sand_sorted_list_entry_value(unit,sorted_list,iter,element_key,NULL) ;
  SOC_SAND_CHECK_FUNC_RESULT(res, 18, exit) ;
  res = soc_sand_sorted_list_find_lower_eq_key(unit,sorted_list,element_key,&local_iter,index_in_indices,&found_equal) ;
  SOC_SAND_CHECK_FUNC_RESULT(res, 22, exit) ;
  if (!found_equal)
  {
    *index_in_indices = -1 ;
  }
exit:
  SOC_SAND_EXIT_AND_SEND_ERROR( "error in soc_sand_sorted_list_get_index_from_iter()",0,0);
}
/* } */
STATIC uint32
  soc_sand_sorted_list_node_link_set(
    SOC_SAND_IN  int                      unit,
    SOC_SAND_IN  SOC_SAND_SORTED_LIST_PTR sorted_list,
    SOC_SAND_IN  uint32                   node1,
    SOC_SAND_IN  uint32                   node2
  )
{
  uint32
    sorted_list_index,
    ptr_size,
    res ;
  uint8
    *tmp_data_ptr ;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0) ;
  SOC_SAND_SORTED_LIST_VERIFY_UNIT_IS_LEGAL(unit, 2) ;
  SOC_SAND_SORTED_LIST_CONVERT_HANDLE_TO_SORTEDLIST_INDEX(sorted_list_index,sorted_list) ;
  SOC_SAND_SORTED_LIST_VERIFY_SORTEDLIST_IS_ACTIVE(sorted_list_index,4,6) ;

  res = soc_sand_sorted_list_get_tmp_data_ptr_from_handle(unit,sorted_list,&tmp_data_ptr) ;
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 12, exit);
  res = SORTED_LIST_ACCESS_DATA.ptr_size.get(unit,sorted_list_index,&ptr_size) ;
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 14, exit) ;
  res =
    soc_sand_U32_to_U8(
      &node2,
      ptr_size,
      tmp_data_ptr
    ) ;
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);
  /* if(SOC_DPP_WB_ENGINE_VAR_NONE == init_info_ptr->wb_var_index) */
  {
    /*
     * Copy 'ptr_size' bytes from 'tmp_data_ptr' into
     * 'SORTED_LIST_ACCESS_DATA.next' buffer at offset 'ptr_size * node1'.
     */
    res = SORTED_LIST_ACCESS_DATA.next.memwrite(unit,sorted_list_index,tmp_data_ptr,node1 * ptr_size, ptr_size) ;
    SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 20, exit) ;
  }
  res =
    soc_sand_U32_to_U8(
      &node1,
      ptr_size,
      tmp_data_ptr
    ) ;
  SOC_SAND_CHECK_FUNC_RESULT(res, 30, exit);
  /* if(SOC_DPP_WB_ENGINE_VAR_NONE == init_info_ptr->wb_var_index) */
  {
    /*
     * Copy 'ptr_size' bytes from 'tmp_data_ptr' into
     * 'SORTED_LIST_ACCESS_DATA.prev' buffer at offset 'ptr_size * node2'.
     */
    res = SORTED_LIST_ACCESS_DATA.prev.memwrite(unit,sorted_list_index,tmp_data_ptr,node2 * ptr_size, ptr_size) ;
    SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 40, exit) ;
  }
exit:
  SOC_SAND_EXIT_AND_SEND_ERROR( "error in soc_sand_sorted_list_node_link_set()",0,0);
}
/*********************************************************************
* NAME:
*     soc_sand_sorted_list_find_match_entry
* TYPE:
*   PROC
* DATE:
*   May 19 2015
* FUNCTION:
*  lookup in the sorted list for the given key and return the data inserted with
*  the given key.
* INPUT:
*   SOC_SAND_IN  int                              unit -
*     Identifier of the device to access.
*   SOC_SAND_IN  SOC_SAND_SORTED_LIST_PTR         sorted_list -
*     Handle to the sorted list.
*   SOC_SAND_IN  SOC_SAND_SORTED_LIST_KEY         *const key -
*     The key sort to lookup for
*   SOC_SAND_IN  SOC_SAND_SORTED_LIST_DATA        *const data
*     This procedure loads pointed memory by data
*     of matching entry. If set to NULL then this
*     procedure does not load indicated data.
*   SOC_SAND_IN  uint8                            first_match
*     whether to return the first match key. regardless of the data.
*   SOC_SAND_IN  uint8                            *found -
*     whether the data with the given key was found in the sorted list
*   SOC_SAND_OUT  uint32                          *entry -
*     if the key is present in the sorted list then return the entry the key found at,
*     otherwise it returns the place where the key suppose to be.
*   SOC_SAND_OUT  SOC_SAND_SORTED_LIST_ITER       *prev_node
*     iterator points to one node before the searched node.
*   SOC_SAND_OUT  SOC_SAND_SORTED_LIST_ITER       *next_node
*     iterator points to searched node or node after.
*   SOC_SAND_OUT  uint32                          *index_in_indices -
*     Only meaningful if the 'indices' feature is enabled ('indices' element
*     in SOC_SAND_SORTED_LIST_T is allocated). Index in 'indices'
*     array after which the newly matched entry fits (and may be added)
*     assuming 'found' is TRUE. Note that this index is counted from
*     the TOP of 'indices'. So, for example, when index=0 then entry
*     should be added as LAST on 'indices'.
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
STATIC uint32
  soc_sand_sorted_list_find_match_entry(
    SOC_SAND_IN   int                           unit,
    SOC_SAND_IN   SOC_SAND_SORTED_LIST_PTR      sorted_list,
    SOC_SAND_IN   SOC_SAND_SORTED_LIST_KEY      *const key,
    SOC_SAND_IN   SOC_SAND_SORTED_LIST_DATA     *const data,
    SOC_SAND_IN   uint8                         first_match,
    SOC_SAND_OUT  uint8                         *found,
    SOC_SAND_OUT  SOC_SAND_SORTED_LIST_ITER     *prev_node,
    SOC_SAND_OUT  SOC_SAND_SORTED_LIST_ITER     *cur_node,
    SOC_SAND_OUT  uint32                        *index_in_indices
  )
{
  uint32
    key_size,
    null_ptr,
    prev,
    curr,
    curr_index ;
  SOC_SAND_SORTED_LIST_KEY_CMP 
    cmp_key_fun ;
  SOC_SAND_SORTED_LIST_CMP_FUNC_TYPE
    cmp_func_type ;
  int32
    compare_res ;
  uint32
    sorted_list_index,
    res;
  uint8
    *tmp_key_ptr ;

  SOC_SAND_INIT_ERROR_DEFINITIONS(SOC_SAND_SORTED_LIST_FIND_MATCH_ENTRY);
  /*
   * Just make sure that, in case of error, an illegal value is returned.
   * Good practive though not necessary.
   */
  *index_in_indices = -2 ;
  SOC_SAND_SORTED_LIST_VERIFY_UNIT_IS_LEGAL(unit, 2) ;
  SOC_SAND_SORTED_LIST_CONVERT_HANDLE_TO_SORTEDLIST_INDEX(sorted_list_index,sorted_list) ;
  SOC_SAND_SORTED_LIST_VERIFY_SORTEDLIST_IS_ACTIVE(sorted_list_index,4,6) ;

  SOC_SAND_CHECK_NULL_INPUT(key) ;
  SOC_SAND_CHECK_NULL_INPUT(found) ;
  SOC_SAND_CHECK_NULL_INPUT(prev_node) ;
  SOC_SAND_CHECK_NULL_INPUT(cur_node) ;

  res = SORTED_LIST_ACCESS_INFO.cmp_func_type.get(unit,sorted_list_index,&cmp_func_type) ;
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 8, exit);
  if (cmp_func_type == SOC_SAND_SORTED_LIST_CMP_FUNC_TYPE_TCAM)
  {
    cmp_key_fun = soc_sand_sorted_list_tcam_cmp_priority ;
  }
  else
  {
    cmp_key_fun = (SOC_SAND_SORTED_LIST_KEY_CMP)soc_sand_os_memcmp ;
  }
  *found = FALSE ;
  SOC_SAND_SORTED_LIST_TAIL_PLACE_GET(sorted_list_index,prev,12) ;

  res = soc_sand_sorted_list_get_tmp_key_ptr_from_handle(unit,sorted_list,&tmp_key_ptr) ;
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 16, exit);
  res = SORTED_LIST_ACCESS_INFO.key_size.get(unit,sorted_list_index,&key_size) ;
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 18, exit) ;
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
    res = soc_sand_sorted_list_get_next_aux(
            unit,
            sorted_list,
            prev,
            FALSE, /* Scan the list from the end - faster in TCAM for sorted entries */
            tmp_key_ptr,
            NULL,&curr
          );
    SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 20, exit) ;
    if (curr == SOC_SAND_SORTED_LIST_ITER_BEGIN(unit,sorted_list))
    {
      *found = FALSE;
      *prev_node = SOC_SAND_SORTED_LIST_ITER_BEGIN(unit,sorted_list);
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
        (key_size * sizeof(SOC_SAND_SORTED_LIST_KEY_TYPE))
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
          res = soc_sand_sorted_list_get_next_aux(
                 unit,
                 sorted_list,
                 prev,
                 FALSE,
                 tmp_key_ptr,
                 NULL,&curr
               );
          SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 30, exit) ;
          *prev_node = curr;
          *cur_node =  prev;
          *index_in_indices = curr_index ;
          goto exit;
      }
      if (data != NULL)
      {
        res =
          soc_sand_sorted_list_data_cmp(
            unit,
            sorted_list,
            curr,
            data, &compare_res
          );
         SOC_SAND_CHECK_FUNC_RESULT(res, 40, exit) ;
        if (compare_res)
        {
          *found = TRUE;
          prev = curr;
          res = soc_sand_sorted_list_get_next_aux(
                   unit,
                   sorted_list,
                   prev,
                   FALSE,
                   tmp_key_ptr,
                   NULL,&curr
                 );
          SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 50, exit) ;
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
      SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 8, exit) ;
      *found = FALSE ;
      *prev_node = (curr == SOC_SAND_SORTED_LIST_ITER_END(unit,sorted_list)) ? null_ptr : curr ;
      *cur_node = prev ;
      *index_in_indices = curr_index ;
      goto exit ;
    }
    prev = curr ;
    curr_index++ ;
  }  while (!(*found)) ;
exit:
  SOC_SAND_EXIT_AND_SEND_ERROR( "error in soc_sand_sorted_list_find_match_entry()",0,0);
}

/*********************************************************************
* NAME:
*     soc_sand_sorted_list_node_alloc
* TYPE:
*   PROC
* DATE:
*   May 19 2015
* FUNCTION:
*  Insert an entry into the sorted list, if already exist then
*  the operation returns an error.
* INPUT:
*   SOC_SAND_IN  int                              unit -
*     Identifier of the device to access.
*   SOC_SAND_IN  SOC_SAND_SORTED_LIST_PTR         sorted_list -
*     Handle to the sorted list.
*   SOC_SAND_IN  SOC_SAND_SORTED_LIST_KEY         *key -
*     The key to add into the sorted list
*   SOC_SAND_IN  SOC_SAND_SORTED_LIST_DATA        *data -
*     The data to add into the sorted list and to be associated with
*     the given key
*   SOC_SAND_OUT  uint8                           *success -
*     whether the add operation success, this may be false.
*     if after trying to relist the key SOC_SAND_SORTED_LIST_MAX_NOF_RELIST times
*     and in all tries fill in in_use entry. to solve this problem try
*     to enlarge the sorted list size or use better list function.
*   SOC_SAND_IN  uint32                           from_top_index_in_indices -
*     Only meaningful if the 'indices' feature is enabled ('indices' element
*     in SOC_SAND_SORTED_LIST_T is allocated). Index in 'indices'
*     array after which the newly matched entry fits (and should be added).
*     Note that this index is counted from the TOP of 'indices'. So, for
*     example, when index=0 then entry should be added as LAST on 'indices'.
* REMARKS:
*     = if there is already a key with the same key in the sorted list error is returned.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
STATIC uint32
  soc_sand_sorted_list_node_alloc(
    SOC_SAND_IN   uint32                    unit,
    SOC_SAND_IN  SOC_SAND_SORTED_LIST_PTR   sorted_list,
    SOC_SAND_IN  SOC_SAND_SORTED_LIST_KEY   *const key,
    SOC_SAND_IN  SOC_SAND_SORTED_LIST_DATA  *const data,
    SOC_SAND_IN   uint32                    prev_node,
    SOC_SAND_IN   uint32                    next_node,
    SOC_SAND_IN   uint32                    from_top_index_in_indices,
    SOC_SAND_OUT  uint8                     *found
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
  SOC_SAND_OCC_BM_PTR
    memory_use ;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);
  SOC_SAND_SORTED_LIST_VERIFY_UNIT_IS_LEGAL(unit, 2) ;
  SOC_SAND_SORTED_LIST_CONVERT_HANDLE_TO_SORTEDLIST_INDEX(sorted_list_index,sorted_list) ;
  SOC_SAND_SORTED_LIST_VERIFY_SORTEDLIST_IS_ACTIVE(sorted_list_index,4,6) ;

  SOC_SAND_CHECK_NULL_INPUT(key) ;
  SOC_SAND_CHECK_NULL_INPUT(data) ;
  SOC_SAND_CHECK_NULL_INPUT(found) ;
  /*
   * Check to see whether the entry exists
   */
  res = SORTED_LIST_ACCESS_DATA.memory_use.get(unit,sorted_list_index,&memory_use) ;
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 10, exit) ;
  res =
    soc_sand_occ_bm_alloc_next(
      unit,
      memory_use,
      &new_node_ptr,
      found
    ) ;
  if (*found == FALSE)
  {
    goto exit;
  }
  /* if(SOC_DPP_WB_ENGINE_VAR_NONE == init_info_ptr->wb_var_index) */
  {
    res = SORTED_LIST_ACCESS_INFO.key_size.get(unit,sorted_list_index,&key_size) ;
    SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 18, exit) ;
    /*
     * Copy 'key_size' bytes from input 'keys' buffer into 'SORTED_LIST_ACCESS_DATA.keys'
     * buffer at offset 'key_size * new_node_ptr'.
     */
    res = SORTED_LIST_ACCESS_DATA.keys.memwrite(unit,sorted_list_index,key,new_node_ptr * key_size, key_size) ;
    SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 26, exit) ;
  }
  /* if(SOC_DPP_WB_ENGINE_VAR_NONE == init_info_ptr->wb_var_index) */
  {
    res = SORTED_LIST_ACCESS_INFO.data_size.get(unit,sorted_list_index,&data_size) ;
    SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 8, exit) ;
    /*
     * Copy 'data_size' bytes from input 'data' buffer into 'SORTED_LIST_ACCESS_DATA.data'
     * buffer at offset 'data_size * new_node_ptr'.
     */
    res = SORTED_LIST_ACCESS_DATA.data.memwrite(unit,sorted_list_index,data,new_node_ptr * data_size, data_size) ;
    SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 26, exit) ;
  }
  soc_sand_sorted_list_node_link_set(
    unit,
    sorted_list,
    prev_node,
    new_node_ptr
  ) ;
  soc_sand_sorted_list_node_link_set(
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
    SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 30, exit) ;
    if (is_allocated)
    {
      /*
       * Feature is enabled. Add element on 'indices' array at specified location.
       */
      res = SORTED_LIST_ACCESS_DATA.num_elements_on_indices.get(unit,sorted_list_index,&num_elements_on_indices) ;
      SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 34, exit) ;
      if (num_elements_on_indices == 0)
      {
        /*
         * If the 'indices' array is empty then, obviously, add the first element
         * and ignore 'index_in_indices' (which should be '0').
         */
        res = SORTED_LIST_ACCESS_DATA.indices.set(unit,sorted_list_index,0,new_node_ptr) ;
        SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 38, exit) ;
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
          SOC_SAND_SET_ERROR_CODE(SOC_SAND_VALUE_ABOVE_MAX_ERR, 52, exit) ;
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
            SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 38, exit) ;
            res = SORTED_LIST_ACCESS_DATA.indices.set(unit,sorted_list_index,tmp_index + 1,tmp_on_indices) ;
            SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 38, exit) ;
            num_to_copy-- ;
            tmp_index-- ;
          }
        }
        /*
         * Note that if (index_in_indices == (num_elements_on_indices - 1)) then no 'move'
         * is required.
         */
        res = SORTED_LIST_ACCESS_DATA.indices.set(unit,sorted_list_index,index_in_indices + 1,new_node_ptr) ;
        SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 48, exit) ;
      }
      num_elements_on_indices++ ;
      res = SORTED_LIST_ACCESS_DATA.num_elements_on_indices.set(unit,sorted_list_index,num_elements_on_indices) ;
      SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 56, exit) ;
    }
  }
exit:
  SOC_SAND_EXIT_AND_SEND_ERROR( "error in soc_sand_sorted_list_node_alloc()",0,0);

}

/*********************************************************************
* NAME:
*   soc_sand_sorted_list_get_iter_begin_or_end
* TYPE:
*   PROC
* DATE:
*   May 19 2015
* FUNCTION:
*   Get the first iterator of the sorted list.
* INPUT:
*   SOC_SAND_IN  int                              unit -
*     Identifier of the device to access.
*   SOC_SAND_IN  SOC_SAND_SORTED_LIST_PTR         sorted_list -
*     Handle to the sorted list.
*   SOC_SAND_IN  int                              get_begin -
*     If non-zero then get iterator at the beginning of the list. Otherwise
*     get iterator at end of list
* REMARKS:
*     None.
* RETURNS:
*   First iterator. If an error is encountered, a negative value is returned.
*********************************************************************/
uint32
  soc_sand_sorted_list_get_iter_begin_or_end(
    SOC_SAND_IN   int                           unit,
    SOC_SAND_IN   SOC_SAND_SORTED_LIST_PTR      sorted_list,
    SOC_SAND_IN   int                           get_begin
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
  SOC_SAND_SORTED_LIST_CONVERT_HANDLE_TO_SORTEDLIST_INDEX(sorted_list_index,sorted_list) ;
  {
    uint8 bit_val ;
    uint32 max_nof_lists ;
    res = SORTED_LIST_ACCESS.max_nof_lists.get(unit, &max_nof_lists) ;
    if (res != SOC_E_NONE)
    {
      FUNC_RESULT_SOC_PRINT(res) ;
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
        FUNC_RESULT_SOC_PRINT(res) ;
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
    FUNC_RESULT_SOC_PRINT(res) ;
    goto exit ;
  }
  if (get_begin == 0)
  {
    list_size += 1 ;
  }
exit:
  return (list_size) ;
}

#include <soc/dpp/SAND/Utils/sand_footer.h>
