/* $Id: sand_hashtable.c,v 1.12 Broadcom SDK $
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
#include <soc/dnx/legacy/drv.h>

#include <soc/dnx/legacy/SAND/Utils/sand_header.h>

#include <soc/dnx/legacy/SAND/Utils/sand_hashtable.h>
#include <soc/dnx/legacy/SAND/Utils/sand_os_interface.h>
#include <soc/dnx/legacy/SAND/Utils/sand_integer_arithmetic.h>
#include <soc/dnx/legacy/SAND/Management/sand_error_code.h>
#include <soc/dnx/legacy/SAND/Management/sand_general_macros.h>
#include <soc/dnx/legacy/drv.h>

extern shr_sw_state_t *sw_state[BCM_MAX_NUM_UNITS];

/* } */

/*************
* DEFINES   *
*************/
/* { */

/* $Id: sand_hashtable.c,v 1.12 Broadcom SDK $
 */
/*
 * Maximum number of rehashes before giving up: when trying to add
 * a key, hash the key to get an entry index
 * If the entry in use, try to rehash until finding unused entry.
 * After this number of tries return with a fail.
 */
#define DNX_SAND_HASH_TABLE_MAX_NOF_REHASH  50

/* } */

/*************
*  MACROS   *
*************/
/* { */
#define HASH_TABLE_ACCESS          sw_state_access[unit].dnx.soc.sand.hash_table  
#define HASH_TABLE_ACCESS_DATA     HASH_TABLE_ACCESS.hashs_array.hash_data
#define HASH_TABLE_ACCESS_INFO     HASH_TABLE_ACCESS.hashs_array.init_info
/*
 * Verify specific hash table index is marked as 'occupied'. If not, software goes to
 * exit with error code.
 * 
 * Notes:
 *   'unit' is assumed to be defined in the caller's scope.
 *   'res' is assumed to be defined in the caller's scope.
 *   'exit' is assumed to be defined in the caller's scope.
 */
#define DNX_SAND_HASH_TABLE_VERIFY_HASHTABLE_IS_ACTIVE(_hash_table_index,_err1,_err2) \
  { \
    uint8 bit_val ; \
    uint32 max_nof_hashs ; \
    res = HASH_TABLE_ACCESS.max_nof_hashs.get(unit, &max_nof_hashs) ; \
    DNX_SAND_SOC_CHECK_FUNC_RESULT(res, _err1, exit) ; \
    if (_hash_table_index >= max_nof_hashs) \
    { \
      /* \
       * If hashtable handle is out of range then quit with error. \
       */ \
      bit_val = 0 ; \
    } \
    else \
    { \
      res = HASH_TABLE_ACCESS.occupied_hashs.bit_get(unit, (int)_hash_table_index, &bit_val) ; \
      DNX_SAND_SOC_CHECK_FUNC_RESULT(res, _err1, exit) ; \
    } \
    if (bit_val == 0) \
    { \
      /* \
       * If hashtable structure is not indicated as 'occupied' then quit \
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
#define DNX_SAND_HASH_TABLE_VERIFY_UNIT_IS_LEGAL(unit, _err1) \
  if ((unit < 0) || (unit >= SOC_MAX_NUM_DEVICES)) \
  { \
    /* \
     * If this is an illegal unit identifier, quit \
     * with error. \
     */ \
    DNX_SAND_SET_ERROR_CODE(DNX_SAND_MAX_NUM_DEVICES_OUT_OF_RANGE_ERR, _err1, exit); \
  }
/*
 * Convert input hash table handle to index in 'occupied_hashs' array.
 * Convert input index in 'occupied_hashs' array to hash table handle.
 * Indices go from 0 -> (occupied_hashs - 1)
 * Handles go from 1 -> occupied_hashs
 */
#define DNX_SAND_HASH_TABLE_CONVERT_HANDLE_TO_HASHTABLE_INDEX(_hash_table_index,_handle) (_hash_table_index = _handle - 1)
#define DNX_SAND_HASH_TABLE_CONVERT_HASHTABLE_INDEX_TO_HANDLE(_handle,_hash_table_index) (_handle = _hash_table_index + 1)

/* } */

/*************
* TYPE DEFS *
*************/
/* { */
/*
 * the key and data type, used for malloc.
 */
typedef uint8 DNX_SAND_HASH_TABLE_KEY_TYPE ;


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


/*********************************************************************
* NAME:
*   dnx_sand_hash_table_get_tmp_data_ptr_from_handle
* TYPE:
*   PROC
* DATE:
*   May 18 2015
* FUNCTION:
*   Get value of 'tmp_buf' pointer (See DNX_SAND_HASH_TABLE_T)
*   from handle.
* INPUT:
*   DNX_SAND_IN  int                            unit -
*     Identifier of the device to access.
*   DNX_SAND_IN  DNX_SAND_HASH_TABLE_PTR        hash_table -
*     Handle to the hash table.
*   DNX_SAND_OUT uint8                          **tmp_buf_ptr_ptr -
*     This procedure loads pointed memory by the pointer to the 'tmp_buf'
*     internal workspace buffer.
* REMARKS:
*   This procedure is exceptional. It is added here so we can use
*   the buffer pointed by 'tmp_buf' as a work space whose address
*   is passed to variuos utilities.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
static uint32
  dnx_sand_hash_table_get_tmp_data_ptr_from_handle(
    DNX_SAND_IN  int                          unit,
    DNX_SAND_IN  DNX_SAND_HASH_TABLE_PTR      hash_table,
    DNX_SAND_OUT uint8                        **tmp_buf_ptr_ptr
  )
{
  uint32
    hash_table_index,
    res;

  DNX_SAND_INIT_ERROR_DEFINITIONS(0) ;
  DNX_SAND_HASH_TABLE_VERIFY_UNIT_IS_LEGAL(unit, 2) ;
  DNX_SAND_HASH_TABLE_CONVERT_HANDLE_TO_HASHTABLE_INDEX(hash_table_index,hash_table) ;
  DNX_SAND_HASH_TABLE_VERIFY_HASHTABLE_IS_ACTIVE(hash_table_index,4,6) ;

  *tmp_buf_ptr_ptr = sw_state[unit]->dnx.soc.sand.hash_table->hashs_array[hash_table_index]->hash_data.tmp_buf ;
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in dnx_sand_hash_table_get_tmp_data_ptr_from_handle()",0,0);
}


/*********************************************************************
* NAME:
*   dnx_sand_hash_table_get_tmp2_data_ptr_from_handle
* TYPE:
*   PROC
* DATE:
*   May 18 2015
* FUNCTION:
*   Get value of 'tmp_buf2' pointer (See DNX_SAND_HASH_TABLE_T)
*   from handle.
* INPUT:
*   DNX_SAND_IN  int                            unit -
*     Identifier of the device to access.
*   DNX_SAND_IN  DNX_SAND_HASH_TABLE_PTR        hash_table -
*     Handle to the hash table.
*   DNX_SAND_OUT uint8                          **tmp_buf2_ptr_ptr -
*     This procedure loads pointed memory by the pointer to the 'tmp_buf2'
*     internal workspace buffer.
* REMARKS:
*   This procedure is exceptional. It is added here so we can use
*   the buffer pointed by 'tmp_buf2' as a work space whose address
*   is passed to variuos utilities.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
static uint32
  dnx_sand_hash_table_get_tmp2_data_ptr_from_handle(
    DNX_SAND_IN  int                          unit,
    DNX_SAND_IN  DNX_SAND_HASH_TABLE_PTR      hash_table,
    DNX_SAND_OUT uint8                        **tmp_buf2_ptr_ptr
  )
{
  uint32
    hash_table_index,
    res;

  DNX_SAND_INIT_ERROR_DEFINITIONS(0) ;
  DNX_SAND_HASH_TABLE_VERIFY_UNIT_IS_LEGAL(unit, 2) ;
  DNX_SAND_HASH_TABLE_CONVERT_HANDLE_TO_HASHTABLE_INDEX(hash_table_index,hash_table) ;
  DNX_SAND_HASH_TABLE_VERIFY_HASHTABLE_IS_ACTIVE(hash_table_index,4,6) ;

  *tmp_buf2_ptr_ptr = sw_state[unit]->dnx.soc.sand.hash_table->hashs_array[hash_table_index]->hash_data.tmp_buf2 ;
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in dnx_sand_hash_table_get_tmp2_data_ptr_from_handle()",0,0);
}

/*
 * Get handle to hash table which will be considered illegal
 * by all hash table utilities.
 * Legal values for 'handle' are 1 -> MAX_NOF_HASHS or, to be more precise:
 * 1 -> max_nof_hashs (The value of HASH_TABLE_ACCESS.max_nof_hashs.get(unit, &max_nof_hashs))
 */
uint32
  dnx_sand_hash_table_get_illegal_hashtable_handle(
    void
  )
{
  return ((uint32)(-1)) ;
}

uint32
dnx_sand_hashtable_default_set_entry_fun(
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
dnx_sand_hashtable_default_get_entry_fun(
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

/*********************************************************************
* NAME:
*     dnx_sand_hash_table_get_table_size
* TYPE:
*   PROC
* DATE:
*   Mar 26 2015
* FUNCTION:
*   Get element 'table_size' for hash table.
* INPUT:
*   DNX_SAND_IN     int                           unit -
*     Identifier of the device to access.
*   DNX_SAND_IN  DNX_SAND_HASH_TABLE_PTR          hash_table -
*     Handle to the hash table.
*   DNX_SAND_INOUT uint32                         *table_size_ptr -
*     This procedure loads pointed memory by the info element 'table_size'.
* REMARKS:
*   For external users nots aware of 'new sw state' structures.
* RETURNS:
*   OK or ERROR indication.
*********************************************************************/
uint32
  dnx_sand_hash_table_get_table_size(
    DNX_SAND_IN     int                              unit,
    DNX_SAND_IN    DNX_SAND_HASH_TABLE_PTR           hash_table,
    DNX_SAND_INOUT uint32                            *table_size_ptr
  )
{
  uint32
    hash_table_index,
    res,
    table_size ;
  DNX_SAND_INIT_ERROR_DEFINITIONS(0);
  DNX_SAND_HASH_TABLE_VERIFY_UNIT_IS_LEGAL(unit, 2) ;
  DNX_SAND_HASH_TABLE_CONVERT_HANDLE_TO_HASHTABLE_INDEX(hash_table_index,hash_table) ;
  DNX_SAND_HASH_TABLE_VERIFY_HASHTABLE_IS_ACTIVE(hash_table_index,4,6) ;

  res = HASH_TABLE_ACCESS_INFO.table_size.get(unit, hash_table_index, &table_size) ;
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 10, exit) ;
  *table_size_ptr = table_size ;
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in dnx_sand_hash_table_get_table_size()",0,0);
}

 /*********************************************************************
 * NAME:
 *     dnx_sand_hash_table_simple_hash
 * FUNCTION:
 *    Hash functions maps from key to hash value.
 *    simple and default implementation of a hash function.
 * INPUT:
 *  DNX_SAND_IN  int                            unit -
 *   Identifier of the device to access.
 *  DNX_SAND_IN  DNX_SAND_HASH_TABLE_PTR        hash_table -
 *   Handle to the hash table, needed so the hash function
 *   can get the properties of the hash table to consider in the
 *   calculations
 *  DNX_SAND_IN  DNX_SAND_HASH_TABLE_KEY        key -
 *   key to hash
 *  DNX_SAND_IN  uint32                         seed -
 *   value to use in the hash calculation
 *  DNX_SAND_OUT  uint32                        *hash_val -
 *   the calculated hash value.
 * REMARKS:
 *     None.
 * RETURNS:
 *     OK or ERROR indication.
 *********************************************************************/
uint32
  dnx_sand_hash_table_simple_hash(
    DNX_SAND_IN  int                         unit,
    DNX_SAND_IN  DNX_SAND_HASH_TABLE_PTR     hash_table,
    DNX_SAND_IN  DNX_SAND_HASH_TABLE_KEY     *const key,
    DNX_SAND_IN  uint32                      seed,
    DNX_SAND_OUT uint32                      *hash_val
  )
{
  uint32
    res,
    hash_table_index,
    indx,
    tmp,
    table_width,
    key_size ;

  DNX_SAND_INIT_ERROR_DEFINITIONS(0);
  DNX_SAND_HASH_TABLE_VERIFY_UNIT_IS_LEGAL(unit, 2) ;
  DNX_SAND_HASH_TABLE_CONVERT_HANDLE_TO_HASHTABLE_INDEX(hash_table_index,hash_table) ;
  DNX_SAND_HASH_TABLE_VERIFY_HASHTABLE_IS_ACTIVE(hash_table_index,4,6) ;

  DNX_SAND_CHECK_NULL_INPUT(key);
  DNX_SAND_CHECK_NULL_INPUT(hash_val);

  tmp = 5381 ;
  res = HASH_TABLE_ACCESS_INFO.key_size.get(unit,hash_table_index,&key_size) ;
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 10, exit);
  res = HASH_TABLE_ACCESS_INFO.table_width.get(unit,hash_table_index,&table_width) ;
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 10, exit);

  for (indx = 0 ; indx < key_size ; ++indx)
  {
    tmp = ((tmp  << 5 ) + tmp ) ^ key[indx]  ;
  }
  *hash_val = tmp % table_width ;

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in dnx_sand_hash_table_simple_hash()",0,0);
}

/*********************************************************************
* NAME:
*     dnx_sand_hash_table_simple_rehash
* FUNCTION:
*    Hash functions maps from key to hash value.
*    Simple and default implementation of a rehash function.
*    Returns the next entry in the hash table.
* INPUT:
*  DNX_SAND_IN  int                            unit -
*   Identifier of the device to access.
*  DNX_SAND_IN  DNX_SAND_HASH_TABLE_PTR        hash_table -
*   Handle to the hash table, needed so the hash function
*   can get the properties of the hash table to consider in the
*   calculations
*  DNX_SAND_IN  DNX_SAND_HASH_TABLE_KEY        key -
*   key to hash
*  DNX_SAND_IN  uint32                         seed -
*    Value to use in the hash calculation
*  DNX_SAND_OUT  uint32                        *hash_val -
*    The calculated hash value.
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  dnx_sand_hash_table_simple_rehash(
    DNX_SAND_IN  int                        unit,
    DNX_SAND_IN  DNX_SAND_HASH_TABLE_PTR    hash_table,
    DNX_SAND_IN  DNX_SAND_HASH_TABLE_KEY    *const key,
    DNX_SAND_IN  uint32                     seed,
    DNX_SAND_OUT uint32                     *hash_val
  )
{
  uint32
    res,
    hash_table_index,
    table_width ;

  DNX_SAND_INIT_ERROR_DEFINITIONS(0) ;
  DNX_SAND_HASH_TABLE_VERIFY_UNIT_IS_LEGAL(unit, 2) ;
  DNX_SAND_HASH_TABLE_CONVERT_HANDLE_TO_HASHTABLE_INDEX(hash_table_index,hash_table) ;
  DNX_SAND_HASH_TABLE_VERIFY_HASHTABLE_IS_ACTIVE(hash_table_index,4,6) ;

  DNX_SAND_CHECK_NULL_INPUT(key) ;
  DNX_SAND_CHECK_NULL_INPUT(hash_val) ;

  res = HASH_TABLE_ACCESS_INFO.table_width.get(unit,hash_table_index,&table_width) ;
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 10, exit);
  *hash_val = (seed + 1) % table_width;
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in dnx_sand_hash_table_simple_hash()",0,0);
}

/*********************************************************************
* NAME:
*     dnx_sand_hash_table_find_entry
* TYPE:
*   PROC
* DATE:
*   Mar 26 2015
* FUNCTION:
*  lookup in the hash table for the given key and return the data inserted with
*  the given key.
* INPUT:
*  DNX_SAND_IN  int                            unit -
*     Identifier of the device to access.
*  DNX_SAND_IN  DNX_SAND_HASH_TABLE_PTR        hash_table -
*     Handle to the hash table.
*   DNX_SAND_IN  DNX_SAND_HASH_TABLE_KEY       key -
*     The key to lookup for
*   DNX_SAND_IN  uint8                         first_empty
*     whether to return the first empty entry .
*   DNX_SAND_OUT  uint32                      *entry -
*     if the key is present in the hash table then return the entry the key found at,
*     otherwise it returns the place where the key suppose to be.
*   DNX_SAND_IN  uint8                        *found -
*     whether the key was found in the hash table
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
static uint32
  dnx_sand_hash_table_find_entry(
    DNX_SAND_IN     int                          unit,
    DNX_SAND_INOUT  DNX_SAND_HASH_TABLE_PTR      hash_table,
    DNX_SAND_IN     DNX_SAND_HASH_TABLE_KEY      *const key,
    DNX_SAND_IN     uint8                        first_empty,
    DNX_SAND_IN     uint8                        alloc_by_index,
    DNX_SAND_OUT    uint32                       *entry,
    DNX_SAND_OUT    uint8                        *found,
    DNX_SAND_OUT    uint32                       *prev,
    DNX_SAND_OUT    uint8                        *first
  );

/************************************************************************/
/*  End of internals                                                    */
/************************************************************************/

/*********************************************************************
* NAME:
*   dnx_sand_hash_table_init
* TYPE:
*   PROC
* DATE:
*   May 19 2015
* FUNCTION:
*   Initialize control structure for ALL hash table instances expected.
* INPUT:
*   DNX_SAND_IN  int unit -
*     Identifier of the device to access.
*   DNX_SAND_IN  uint32 max_nof_hashs -
*     Maximal number of hash tables which can be sustained simultaneously.
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  dnx_sand_hash_table_init(
    DNX_SAND_IN       int                          unit,
    DNX_SAND_IN       uint32                       max_nof_hashs
  )
{
  uint32 res ;

  DNX_SAND_INIT_ERROR_DEFINITIONS(DNX_SAND_HASH_TABLE_INIT) ;
  DNX_SAND_HASH_TABLE_VERIFY_UNIT_IS_LEGAL(unit, 2) ;

  res = HASH_TABLE_ACCESS.alloc(unit);
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 10, exit);

  res = HASH_TABLE_ACCESS.hashs_array.ptr_alloc(unit, max_nof_hashs);
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 20, exit);

  res = HASH_TABLE_ACCESS.max_nof_hashs.set(unit, max_nof_hashs);
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 30, exit);

  res = HASH_TABLE_ACCESS.in_use.set(unit, 0);
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 40, exit);

  res = HASH_TABLE_ACCESS.occupied_hashs.alloc_bitmap(unit, max_nof_hashs);
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 60, exit);

  exit:
    DNX_SAND_EXIT_AND_SEND_ERROR( "error in dnx_sand_hash_table_init()",0,0);
}

/*********************************************************************
* NAME:
*   dnx_sand_hash_table_clear_all_tmps
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
  dnx_sand_hash_table_clear_all_tmps(
    DNX_SAND_IN int unit
  )
{
  uint32
    hash_table_index ;
  uint32
    max_buf_size,
    found,
    res,
    max_nof_hashs,
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

  DNX_SAND_HASH_TABLE_VERIFY_UNIT_IS_LEGAL(unit, 2) ;

  res = HASH_TABLE_ACCESS.in_use.get(unit, &in_use);
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 4, exit);

  res = HASH_TABLE_ACCESS.max_nof_hashs.get(unit, &max_nof_hashs);
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 6, exit);

  if (in_use >= max_nof_hashs)
  {
    /*
     * If number of occupied bitmap structures is beyond the
     * maximum then quit with error.
     */
    DNX_SAND_SET_ERROR_CODE(DNX_SAND_MALLOC_FAIL, 8, exit);
  }
  /*
   * Find occupied hashs (a set bit in 'occupied_hashs') and, for each,
   * fill 'tmp' buffers by zeroes.
   *
   * Currently, 'tmp' buffers are:
   *   sw_state[_unit]->dnx.soc.sand.hash_table->hashs_array[hashs_array_ptr_arr_idx_0]->hash_data.tmp_buf
   *   sw_state[_unit]->dnx.soc.sand.hash_table->hashs_array[hashs_array_ptr_arr_idx_0]->hash_data.tmp_buf2
   */
  found = 0 ;
  offset = 0 ;
  for (hash_table_index = 0 ; hash_table_index < max_nof_hashs ; hash_table_index++)
  {
    res = HASH_TABLE_ACCESS.occupied_hashs.bit_get(unit, hash_table_index, &bit_val);
    DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 12, exit);
    if (bit_val == 1)
    {
      /*
       * 'hash_table_index' is now the index of an occupied entry.
       */
      found++ ;
      res = HASH_TABLE_ACCESS_DATA.tmp_buf.is_allocated(unit,hash_table_index,&is_allocated) ;
      DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 14, exit) ;
      if (!is_allocated)
      {
        /*
         * 'tmp_buf' buffer must be allocated, at this point.
         */
        DNX_SAND_SET_ERROR_CODE(SOC_E_INTERNAL, 16, exit);
      }
      res = HASH_TABLE_ACCESS_DATA.tmp_buf2.is_allocated(unit,hash_table_index,&is_allocated) ;
      DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 18, exit) ;
      if (!is_allocated)
      {
        /*
         * 'tmp_buf2' buffer must be allocated, at this point.
         */
        DNX_SAND_SET_ERROR_CODE(SOC_E_INTERNAL, 20, exit);
      }
      res = HASH_TABLE_ACCESS_INFO.data_size.get(unit, hash_table_index, &data_size) ;
      DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 22, exit) ;
      res = HASH_TABLE_ACCESS_INFO.key_size.get(unit, hash_table_index, &key_size) ;
      DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 24, exit) ;
      res = HASH_TABLE_ACCESS_DATA.ptr_size.get(unit, hash_table_index, &ptr_size) ;
      DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 26, exit) ;
      max_buf_size = data_size ;
      if (key_size > max_buf_size)
      {
        max_buf_size = key_size;
      }
      if (ptr_size > max_buf_size)
      {
        max_buf_size = ptr_size;
      }
      /*
       * Clear 'tmp_buf'
       */
      res = HASH_TABLE_ACCESS_DATA.tmp_buf.memset(unit,hash_table_index,offset,max_buf_size,0) ;
      DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 28, exit) ;
      /*
       * Clear 'tmp_buf2'
       */
      res = HASH_TABLE_ACCESS_DATA.tmp_buf2.memset(unit,hash_table_index,offset,max_buf_size,0) ;
      DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 30, exit) ;
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
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in dnx_sand_hash_table_clear_all_tmps()",0,0);
}

/*********************************************************************
* NAME:
*     dnx_sand_hash_table_create
* TYPE:
*   PROC
* DATE:
*   Mar 26 2015
* FUNCTION:
*     Creates a new Hash table instance.
* INPUT:
*   DNX_SAND_IN     int                           unit -
*     Identifier of the device to access.
*   DNX_SAND_IN  DNX_SAND_HASH_TABLE_PTR          *hash_table_ptr -
*     This procedure loads pointed memory by the handle
*     to the newly created hash table.
*   DNX_SAND_IN  DNX_SAND_HASH_TABLE_INIT_INFO    init_info -
*     information to use in order to create the hash table (size, hash function...)
* REMARKS:
*   None.
* RETURNS:
*   OK or ERROR indication.
*********************************************************************/
uint32
  dnx_sand_hash_table_create(
    DNX_SAND_IN     int                               unit,
    DNX_SAND_INOUT  DNX_SAND_HASH_TABLE_PTR           *hash_table_ptr,
    DNX_SAND_IN     DNX_SAND_HASH_TABLE_INIT_INFO     init_info
  )
{
  DNX_SAND_HASH_TABLE_PTR
    hash_table ;
  DNX_SAND_OCC_BM_INIT_INFO
    btmp_init_info ;
  uint32
    max_buf_size ;
  uint32
    found,
    res ;
  uint32
    hash_table_index,
    table_size,
    table_width,
    data_size,
    key_size,
    ptr_size,
    null_ptr,
    max_nof_hashs ;
  uint32
    in_use ;
  uint8
    bit_val ;
  int32
    offset ;
  DNX_SAND_OCC_BM_PTR
    memory_use ;

  DNX_SAND_INIT_ERROR_DEFINITIONS(DNX_SAND_HASH_TABLE_CREATE);

  DNX_SAND_HASH_TABLE_VERIFY_UNIT_IS_LEGAL(unit, 2) ;

  DNX_SAND_CHECK_NULL_INPUT(hash_table_ptr) ;

  res = HASH_TABLE_ACCESS.in_use.get(unit, &in_use);
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 4, exit);

  res = HASH_TABLE_ACCESS.max_nof_hashs.get(unit, &max_nof_hashs);
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 6, exit);

  if (in_use >= max_nof_hashs)
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
  res = HASH_TABLE_ACCESS.in_use.set(unit, (in_use + 1));
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 10, exit);
  /*
   * Find a free hash (a cleared bit in 'occupied_hashs'). At this point,
   * there must be one.
   */
  found = 0 ;
  for (hash_table_index = 0 ; hash_table_index < max_nof_hashs ; hash_table_index++)
  {
    res = HASH_TABLE_ACCESS.occupied_hashs.bit_get(unit, hash_table_index, &bit_val);
    DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 12, exit);
    if (bit_val == 0)
    {
      /*
       * 'hash_table_index' is now the index of a free entry.
       */
      found = 1 ;
      break ;
    }
  }
  if (!found)
  {
    DNX_SAND_SET_ERROR_CODE(DNX_SAND_MALLOC_FAIL, 14, exit);
  }
  res = HASH_TABLE_ACCESS.occupied_hashs.bit_set(unit, hash_table_index);
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 16, exit);

  res = HASH_TABLE_ACCESS.hashs_array.alloc(unit, hash_table_index);
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 18, exit);
  /*
   * Note that legal handles start at '1', not at '0'.
   */
  DNX_SAND_HASH_TABLE_CONVERT_HASHTABLE_INDEX_TO_HANDLE(hash_table,hash_table_index) ;
  /*
   * Set output of this procedure.
   */
  *hash_table_ptr = hash_table ;
  
  dnx_sand_SAND_HASH_TABLE_INFO_clear(unit,hash_table) ;
  /*
   * Load 'init' section of hash table as per input.
   * Make sure all callbacks are NULL - Currently, we only use defaults
   */
  if (init_info.get_entry_fun != NULL || init_info.set_entry_fun != NULL )
  {
    DNX_SAND_SET_ERROR_CODE(DNX_SAND_VALUE_OUT_OF_RANGE_ERR, 7, exit);
  }

  res = HASH_TABLE_ACCESS_INFO.set(unit, hash_table_index, &init_info) ;
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 20, exit) ;

  res = HASH_TABLE_ACCESS_INFO.table_size.get(unit, hash_table_index, &table_size) ;
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 24, exit) ;
  res = HASH_TABLE_ACCESS_INFO.table_width.get(unit, hash_table_index, &table_width) ;
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 26, exit) ;
  res = HASH_TABLE_ACCESS_INFO.key_size.get(unit, hash_table_index, &key_size) ;
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 30, exit) ;
  res = HASH_TABLE_ACCESS_INFO.data_size.get(unit, hash_table_index, &data_size) ;
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 32, exit) ;

  if (table_size == 0 || table_width == 0 || key_size == 0 || data_size == 0)
  {
    DNX_SAND_SET_ERROR_CODE(DNX_SAND_HASH_TABLE_SIZE_OUT_OF_RANGE_ERR, 10, exit);
  }
  /*
   * calculate the size of pointers (list head and next) according to table size.
   */
  ptr_size = (dnx_sand_log2_round_up(table_size + 1) + (DNX_SAND_NOF_BITS_IN_BYTE - 1)) / DNX_SAND_NOF_BITS_IN_BYTE ;
  null_ptr = DNX_SAND_BITS_MASK((ptr_size * DNX_SAND_NOF_BITS_IN_BYTE - 1),0) ;

  res = HASH_TABLE_ACCESS_DATA.ptr_size.set(unit, hash_table_index, ptr_size) ;
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 36, exit) ;
  res = HASH_TABLE_ACCESS_DATA.null_ptr.set(unit, hash_table_index, null_ptr) ;
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 40, exit) ;
  /*
   * allocate buffer for keys
   */
  offset = 0 ;
  res = HASH_TABLE_ACCESS_DATA.keys.alloc(unit,hash_table_index,table_size * key_size) ;
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 42, exit) ;
  res = HASH_TABLE_ACCESS_DATA.keys.memset(unit,hash_table_index,offset,table_size * key_size,0x00) ;
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 46, exit) ;
  /*
   * allocate buffer for next array (to build the linked list)
   */
  res = HASH_TABLE_ACCESS_DATA.next.alloc(unit,hash_table_index,table_size * ptr_size) ;
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 50, exit) ;
  res = HASH_TABLE_ACCESS_DATA.next.memset(unit,hash_table_index,offset,table_size * ptr_size,0xFF) ;
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 54, exit) ;
  /*
   * allocate buffer for lists_head (to build the linked list)
   */
  res = HASH_TABLE_ACCESS_DATA.lists_head.alloc(unit,hash_table_index,table_width * ptr_size) ;
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 58, exit) ;
  res = HASH_TABLE_ACCESS_DATA.lists_head.memset(unit,hash_table_index,offset,table_width * ptr_size,0xFF) ;
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 62, exit) ;

  max_buf_size = data_size ;
  if (key_size > max_buf_size)
  {
    max_buf_size = key_size;
  }
  if (ptr_size > max_buf_size)
  {
    max_buf_size = ptr_size;
  }
  /*
   * allocate buffer for temporary (workspace) buffer (no. 1 )
   */
  res = HASH_TABLE_ACCESS_DATA.tmp_buf.alloc(unit,hash_table_index,max_buf_size) ;
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 82, exit) ;
  res = HASH_TABLE_ACCESS_DATA.tmp_buf.memset(unit,hash_table_index,offset,max_buf_size,0x00) ;
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 86, exit) ;
  /*
   * allocate buffer for temporary (workspace) buffer (no. 2 )
   */
  res = HASH_TABLE_ACCESS_DATA.tmp_buf2.alloc(unit,hash_table_index,max_buf_size) ;
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 90, exit) ;
  res = HASH_TABLE_ACCESS_DATA.tmp_buf2.memset(unit,hash_table_index,offset,max_buf_size,0x00) ;
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 94, exit) ;
  dnx_sand_SAND_OCC_BM_INIT_INFO_clear(&btmp_init_info);
  btmp_init_info.size = table_size;
  /*
   * initialize the data to be mapped to
   */
  res = dnx_sand_occ_bm_create(
          unit,
          &btmp_init_info,
          &(memory_use)
        );
  DNX_SAND_CHECK_FUNC_RESULT(res, 98, exit);
  res = HASH_TABLE_ACCESS_DATA.memory_use.set(unit,hash_table_index,memory_use) ;
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 102, exit) ;
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in dnx_sand_hash_table_create()",0,0);
}

/*********************************************************************
* NAME:
*   dnx_sand_hash_table_destroy
* TYPE:
*   PROC
* DATE:
*   Mar 26 2015
* FUNCTION:
*     Destroy indicated hash table instance.
* INPUT:
*   DNX_SAND_IN     int                           unit -
*     Identifier of the device to access.
*   DNX_SAND_OUT  DNX_SAND_HASH_TABLE_PTR         hash_table -
*     Handle of the hash table to destroy.
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  dnx_sand_hash_table_destroy(
    DNX_SAND_IN     int                        unit,
    DNX_SAND_INOUT  DNX_SAND_HASH_TABLE_PTR    hash_table
  )
{
  uint32
    res,
    hash_table_index ;
  DNX_SAND_OCC_BM_PTR
    memory_use ;
  uint8
    bit_val ;
  uint32
    in_use ;

  DNX_SAND_INIT_ERROR_DEFINITIONS(DNX_SAND_HASH_TABLE_DESTROY);
  DNX_SAND_HASH_TABLE_VERIFY_UNIT_IS_LEGAL(unit, 2) ;
  DNX_SAND_HASH_TABLE_CONVERT_HANDLE_TO_HASHTABLE_INDEX(hash_table_index,hash_table) ;
  DNX_SAND_HASH_TABLE_VERIFY_HASHTABLE_IS_ACTIVE(hash_table_index,4,6) ;
  /*
   * First, mark this hash table as 'released'
   */
  res = HASH_TABLE_ACCESS.in_use.get(unit, &in_use);
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 8, exit);
  if ((int)in_use < 0)
  {
    /*
     * If number of occupied hashtable structures goes below zero then quit
     * with error.
     */
    DNX_SAND_SET_ERROR_CODE(DNX_SAND_FREE_FAIL, 12, exit);
  }
  /*
   * Decrement number of 'in_use' to cover the one we now intend to release.
   */
  res = HASH_TABLE_ACCESS.in_use.set(unit, (in_use - 1));
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 16, exit) ;
  /*
   * Mark specific hash table as 'not occupied'
   */
  res = HASH_TABLE_ACCESS.occupied_hashs.bit_get(unit, hash_table_index, &bit_val);
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 20, exit);
  if (bit_val == 0)
  {
    /*
     * If hash table structure is not indicated as 'occupied' then quit
     * with error.
     */
    DNX_SAND_SET_ERROR_CODE(DNX_SAND_FREE_FAIL, 24, exit) ;
  }
  res = HASH_TABLE_ACCESS.occupied_hashs.bit_clear(unit, hash_table_index);
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 28, exit);

  res = HASH_TABLE_ACCESS_DATA.lists_head.free(unit,hash_table_index) ;
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 32, exit) ;
  res = HASH_TABLE_ACCESS_DATA.next.free(unit,hash_table_index) ;
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 36, exit) ;
  res = HASH_TABLE_ACCESS_DATA.keys.free(unit,hash_table_index) ;
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 40, exit) ;
  res = HASH_TABLE_ACCESS_DATA.tmp_buf.free(unit,hash_table_index) ;
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 44, exit) ;
  res = HASH_TABLE_ACCESS_DATA.tmp_buf2.free(unit,hash_table_index) ;
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 48, exit) ;
  res = HASH_TABLE_ACCESS_DATA.memory_use.get(unit,hash_table_index,&memory_use) ;
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 52, exit) ;

  res = dnx_sand_occ_bm_destroy(unit,memory_use);
  DNX_SAND_CHECK_FUNC_RESULT(res, 56, exit);

  res = HASH_TABLE_ACCESS.hashs_array.free(unit,hash_table_index) ;
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 60, exit) ;
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in dnx_sand_hash_table_destroy()",0,0);
}

/*********************************************************************
* NAME:
*   dnx_sand_hash_table_entry_add
* TYPE:
*   PROC
* DATE:
*   Mar 26 2015
* FUNCTION:
*  Insert an entry into the hash table, if already exist then
*  the operation returns an error.
* INPUT:
*   DNX_SAND_IN  int                        unit -
*     Identifier of the device to access.
*   DNX_SAND_IN  DNX_SAND_HASH_TABLE_PTR hash_table -
*     Handle to the hash table to add a key to.
*   DNX_SAND_IN  DNX_SAND_HASH_TABLE_KEY    key -
*     The key to add into the hash table
*   DNX_SAND_IN  uint8                      occupied -
*     the data to add into the hash table and to be associated with
*     the given key
*   DNX_SAND_OUT  uint8                     *success -
*     whether the add operation success, this may be false.
*     if after trying to rehash the key DNX_SAND_HASH_TABLE_MAX_NOF_REHASH times
*     and in all tries fill in in_use entry. to solve this problem try
*     to enlarge the hash table size or use better hash function.
* REMARKS:
*     = if there is already a key with the same key in the hash table error is returned.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  dnx_sand_hash_table_entry_add(
    DNX_SAND_IN    int                        unit,
    DNX_SAND_IN    DNX_SAND_HASH_TABLE_PTR    hash_table,
    DNX_SAND_IN    DNX_SAND_HASH_TABLE_KEY    *const key,
    DNX_SAND_OUT   uint32                     *data_indx,
    DNX_SAND_OUT   uint8                      *success
  )
{
  uint8
    found,
    first;
  uint32
    key_size,
    entry_offset,
    prev_entry;
  uint32
    res,
    hash_table_index ;

  DNX_SAND_INIT_ERROR_DEFINITIONS(DNX_SAND_HASH_TABLE_ENTRY_ADD) ;
  DNX_SAND_HASH_TABLE_VERIFY_UNIT_IS_LEGAL(unit, 2) ;
  DNX_SAND_HASH_TABLE_CONVERT_HANDLE_TO_HASHTABLE_INDEX(hash_table_index,hash_table) ;
  DNX_SAND_HASH_TABLE_VERIFY_HASHTABLE_IS_ACTIVE(hash_table_index,4,6) ;

  DNX_SAND_CHECK_NULL_INPUT(key);
  DNX_SAND_CHECK_NULL_INPUT(data_indx);
  DNX_SAND_CHECK_NULL_INPUT(success);
  /*
   * Check to see whether the entry exists
   */
  res = dnx_sand_hash_table_find_entry(
          unit,
          hash_table,
          key,
          TRUE,
          FALSE,
          &entry_offset,
          &found,
          &prev_entry,
          &first
        );
  DNX_SAND_CHECK_FUNC_RESULT(res, 10, exit);
  if (found)
  {
    *data_indx = entry_offset;
  }
  if (entry_offset == DNX_SAND_HASH_TABLE_NULL)
  {
    *success = FALSE;
    *data_indx = DNX_SAND_HASH_TABLE_NULL;
    goto exit;
  }
  res = HASH_TABLE_ACCESS_INFO.key_size.get(unit, hash_table_index, &key_size) ;
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 14, exit) ;
  /*
   * Copy 'key_size' bytes from input 'key' buffer into 'HASH_TABLE_ACCESS_DATA.keys'
   * buffer at offset 'entry_offset * key_size'.
   */
  res = HASH_TABLE_ACCESS_DATA.keys.memwrite(unit,hash_table_index,key,key_size * entry_offset,key_size) ;
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 26, exit) ;
  *success = TRUE;
  *data_indx = entry_offset;

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in dnx_sand_hash_table_entry_add()",0,0);
}

uint32
  dnx_sand_hash_table_entry_add_at_index(
    DNX_SAND_IN     int                          unit,
    DNX_SAND_IN     DNX_SAND_HASH_TABLE_PTR      hash_table,
    DNX_SAND_IN     DNX_SAND_HASH_TABLE_KEY      *const key,
    DNX_SAND_IN     uint32                       data_indx,
    DNX_SAND_OUT    uint8                        *success
  )
{
  uint8
    found,
    indx_in_use,
    first ;
  uint32
    entry_offset,
    prev_entry ;
  uint32
    old_index ;
  uint32
    res,
    hash_table_index,
    key_size ;
  DNX_SAND_OCC_BM_PTR
    memory_use ;

  DNX_SAND_INIT_ERROR_DEFINITIONS(DNX_SAND_HASH_TABLE_ENTRY_ADD);
  DNX_SAND_HASH_TABLE_VERIFY_UNIT_IS_LEGAL(unit, 2) ;
  DNX_SAND_HASH_TABLE_CONVERT_HANDLE_TO_HASHTABLE_INDEX(hash_table_index,hash_table) ;
  DNX_SAND_HASH_TABLE_VERIFY_HASHTABLE_IS_ACTIVE(hash_table_index,4,6) ;

  DNX_SAND_CHECK_NULL_INPUT(key);
  DNX_SAND_CHECK_NULL_INPUT(success);

  res = HASH_TABLE_ACCESS_DATA.memory_use.get(unit,hash_table_index,&memory_use) ;
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 12, exit) ;
  res = HASH_TABLE_ACCESS_INFO.key_size.get(unit, hash_table_index, &key_size) ;
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 14, exit) ;

  /* check if this key already exist */
  res = dnx_sand_hash_table_entry_lookup(
          unit,
          hash_table,
          key,
          &old_index,
          &found
        );
  DNX_SAND_CHECK_FUNC_RESULT(res, 16, exit);

  if (found && old_index != data_indx)
  {
    *success = FALSE;
    goto exit;
  }
  if (found && old_index == data_indx)
  {
    /* found in required index, done */
    *success = TRUE;
    goto exit;
  }

  res = dnx_sand_occ_bm_is_occupied(
          unit,
          memory_use,
          data_indx,
          &indx_in_use
        );
  DNX_SAND_CHECK_FUNC_RESULT(res, 50, exit);

  if (indx_in_use && !found)
  {
    /* index already in use for other usage */
    *success = FALSE;
    goto exit;
  }
  
  entry_offset = data_indx;
  /* check to see if the entry exists */
  res = dnx_sand_hash_table_find_entry(
          unit,
          hash_table,
          key,
          TRUE,
          TRUE,
          &entry_offset,
          &found,
          &prev_entry,
          &first
        );
  DNX_SAND_CHECK_FUNC_RESULT(res, 60, exit) ;

  if (entry_offset == DNX_SAND_HASH_TABLE_NULL)
  {
    *success = FALSE;
    goto exit;
  }
  /*
   * Copy 'key_size' bytes from input 'key' buffer into 'HASH_TABLE_ACCESS_DATA.keys'
   * buffer at offset 'entry_offset * key_size'.
   */
  res = HASH_TABLE_ACCESS_DATA.keys.memwrite(unit,hash_table_index,key,key_size * entry_offset,key_size) ;
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 70, exit) ;
  *success = TRUE;
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in dnx_sand_hash_table_entry_add_at_index()",0,0);
}

/*********************************************************************
* NAME:
*     dnx_sand_hash_table_entry_remove
* TYPE:
*   PROC
* DATE:
*   Mar 26 2015
* FUNCTION:
*  Remove an entry from a hash table, if the key is not exist then
*  the operation has no effect.
* INPUT:
*  DNX_SAND_IN  int                        unit -
*     Identifier of the device to access.
*  DNX_SAND_INOUT  DNX_SAND_HASH_TABLE_PTR hash_table -
*    Handle to the hash table to add to.
*  DNX_SAND_IN  DNX_SAND_HASH_TABLE_KEY    key -
*     The key to remove from the hash table
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  dnx_sand_hash_table_entry_remove(
    DNX_SAND_IN     int                          unit,
    DNX_SAND_IN     DNX_SAND_HASH_TABLE_PTR      hash_table,
    DNX_SAND_IN     DNX_SAND_HASH_TABLE_KEY      *const key
  )
{
  uint8
    found,
    first ;
  uint32
    entry_offset,
    prev_entry ;
  uint32
    res,
    null_ptr,
    hash_table_index,
    ptr_size ;
  uint8
    *next_entry ;
  uint8
    *tmp_buf_ptr ;
  DNX_SAND_OCC_BM_PTR
    memory_use ;

  DNX_SAND_INIT_ERROR_DEFINITIONS(DNX_SAND_HASH_TABLE_ENTRY_REMOVE);
  DNX_SAND_HASH_TABLE_VERIFY_UNIT_IS_LEGAL(unit, 2) ;
  DNX_SAND_HASH_TABLE_CONVERT_HANDLE_TO_HASHTABLE_INDEX(hash_table_index,hash_table) ;
  DNX_SAND_HASH_TABLE_VERIFY_HASHTABLE_IS_ACTIVE(hash_table_index,4,6) ;

  DNX_SAND_CHECK_NULL_INPUT(key) ;
  /*
   * check to see whether the entry exists
   */
  res = dnx_sand_hash_table_find_entry(
          unit,
          hash_table,
          key,
          FALSE,
          FALSE,
          &entry_offset,
          &found,
          &prev_entry,
          &first
        ) ;
  DNX_SAND_CHECK_FUNC_RESULT(res, 10, exit);
  /*
   * If the key to remove does not exist in the hash table then this operation has
   * no side effect.
   */
  if (!found)
  {
    goto exit;
  }
  /*
   * Remove node from linked list
   */
  /*
   * Get next pointer
   */
  res = HASH_TABLE_ACCESS_DATA.ptr_size.get(unit, hash_table_index, &ptr_size) ;
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 12, exit) ;
  res = dnx_sand_hash_table_get_tmp_data_ptr_from_handle(unit,hash_table,&tmp_buf_ptr) ;
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 22, exit) ;
  /*
   * Copy 'ptr_size' bytes from 'HASH_TABLE_ACCESS_DATA.next' buffer at offset
   * 'entry_offset * ptr_size' into 'tmp_buf' buffer.
   */
  res = HASH_TABLE_ACCESS_DATA.next.memread(unit,hash_table_index,tmp_buf_ptr,ptr_size * entry_offset,ptr_size) ;
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 24, exit) ;
  next_entry = tmp_buf_ptr ;
  /*
   * if this is first node in the linked list then set head list
   */
  if (first)
  {
    /*
     * Copy 'ptr_size' bytes from 'next_entry' buffer into 'HASH_TABLE_ACCESS_DATA.lists_head'
     * buffer at offset 'prev_entry * ptr_size'.
     */
    res = HASH_TABLE_ACCESS_DATA.lists_head.memwrite(unit,hash_table_index,next_entry,ptr_size * prev_entry,ptr_size) ;
    DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 26, exit) ;
  }
  else
  {
    /*
     * Copy 'ptr_size' bytes from 'next_entry' buffer into 'HASH_TABLE_ACCESS_DATA.next'
     * buffer at offset 'prev_entry * ptr_size'.
     */
    res = HASH_TABLE_ACCESS_DATA.next.memwrite(unit,hash_table_index,next_entry,ptr_size * prev_entry,ptr_size) ;
    DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 28, exit) ;
  }
  /*
   * Now the entry 'entry_offset' is not in use. free it.
   */
  res = HASH_TABLE_ACCESS_DATA.memory_use.get(unit,hash_table_index,&memory_use) ;
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 12, exit) ;
  res = dnx_sand_occ_bm_occup_status_set(
          unit,
          memory_use,
          entry_offset,
          FALSE
        );
  DNX_SAND_CHECK_FUNC_RESULT(res, 30, exit);
  res = HASH_TABLE_ACCESS_DATA.null_ptr.get(unit, hash_table_index, &null_ptr) ;
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 40, exit) ;

  res = dnx_sand_U32_to_U8(
           &(null_ptr),
           ptr_size,
           tmp_buf_ptr
        );
  DNX_SAND_CHECK_FUNC_RESULT(res, 50, exit);
  /*
   * Copy 'ptr_size' bytes from 'tmp_buf_ptr' (tmp_buf buffer) into 'HASH_TABLE_ACCESS_DATA.next'
   * buffer at offset 'entry_offset * ptr_size'.
   */
  res = HASH_TABLE_ACCESS_DATA.next.memwrite(unit,hash_table_index,tmp_buf_ptr,ptr_size * entry_offset,ptr_size) ;
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 60, exit) ;
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in dnx_sand_hash_table_entry_remove()",0,0);
}
/*********************************************************************
* NAME:
*     dnx_sand_hash_table_entry_remove_by_index
* TYPE:
*   PROC
* DATE:
*   Mar 26 2015
* FUNCTION:
*   Remove an entry from a hash table given the handler identifying
*   this entry.
* INPUT:
*  DNX_SAND_IN  int                        unit -
*     Identifier of the device to access.
*  DNX_SAND_INOUT  DNX_SAND_HASH_TABLE_PTR hash_table -
*     The hash table to add to.
*  DNX_SAND_IN  uint32                     indx -
*     index of the entry to remove.
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  dnx_sand_hash_table_entry_remove_by_index(
    DNX_SAND_IN     int                     unit,
    DNX_SAND_IN     DNX_SAND_HASH_TABLE_PTR hash_table,
    DNX_SAND_IN     uint32                  data_indx
  )
{
  uint8
    *cur_key ;
  uint8
    in_use ;
  uint32
    res,
    key_size,
    table_size,
    hash_table_index ;
  DNX_SAND_OCC_BM_PTR
    memory_use ;
  uint8
    *tmp_buf2_ptr = NULL;

  DNX_SAND_INIT_ERROR_DEFINITIONS(DNX_SAND_HASH_TABLE_ENTRY_REMOVE_BY_INDEX);
  DNX_SAND_HASH_TABLE_VERIFY_UNIT_IS_LEGAL(unit, 2) ;
  DNX_SAND_HASH_TABLE_CONVERT_HANDLE_TO_HASHTABLE_INDEX(hash_table_index,hash_table) ;
  DNX_SAND_HASH_TABLE_VERIFY_HASHTABLE_IS_ACTIVE(hash_table_index,4,6) ;

  res = HASH_TABLE_ACCESS_INFO.table_size.get(unit, hash_table_index, &table_size) ;
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 10, exit) ;
  res = HASH_TABLE_ACCESS_INFO.key_size.get(unit, hash_table_index, &key_size) ;
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 14, exit) ;
  res = HASH_TABLE_ACCESS_DATA.memory_use.get(unit,hash_table_index,&memory_use) ;
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 16, exit) ;
  /*
   * Check whether the entry 'entry_offset' is in use.
   */
  res = dnx_sand_occ_bm_is_occupied(
          unit,
          memory_use,
          data_indx,
          &in_use
        );
  DNX_SAND_CHECK_FUNC_RESULT(res, 20, exit);
  if (!in_use)
  {
    /*
     * If entry 'entry_offset' is not in use, quit.
     */
    goto exit;
  }
  res = dnx_sand_hash_table_get_tmp2_data_ptr_from_handle(unit,hash_table,&tmp_buf2_ptr) ;
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 22, exit) ;
  /*
   * Copy 'key_size' bytes from 'HASH_TABLE_ACCESS_DATA.keys' buffer at offset
   * 'data_indx * key_size' into 'tmp_buf2' buffer.
   */
  res = HASH_TABLE_ACCESS_DATA.keys.memread(unit,hash_table_index,tmp_buf2_ptr,key_size * data_indx,key_size) ;
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 26, exit) ;
  cur_key = tmp_buf2_ptr ;
  res = dnx_sand_hash_table_entry_remove(
          unit,
          hash_table,
          cur_key
        );
  DNX_SAND_CHECK_FUNC_RESULT(res, 40, exit);
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in dnx_sand_hash_table_entry_remove_by_index()", data_indx, (hash_table) ? table_size : 0);
}

/*********************************************************************
* NAME:
*   dnx_sand_hash_table_entry_lookup
* TYPE:
*   PROC
* DATE:
*   Mar 26 2015
* FUNCTION:
*  lookup the hash table for the given key and return the data inserted with
*  the given key.
* INPUT:
*   DNX_SAND_IN  int                           unit -
*     Identifier of the device to access.
*   DNX_SAND_IN  DNX_SAND_HASH_TABLE_PTR       hash_table -
*     Handle to the hash table.
*   DNX_SAND_IN  DNX_SAND_HASH_TABLE_KEY       key -
*     The key to lookup
*   DNX_SAND_OUT  DNX_SAND_HASH_TABLE_DATA    *data -
*     The data associated with the given key, valid only if
*     'found' is true.
*   DNX_SAND_IN  uint8                        *found -
*     whether the key was found in the hash table
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  dnx_sand_hash_table_entry_lookup(
    DNX_SAND_IN     int                         unit,
    DNX_SAND_INOUT  DNX_SAND_HASH_TABLE_PTR     hash_table,
    DNX_SAND_IN     DNX_SAND_HASH_TABLE_KEY     *const key,
    DNX_SAND_OUT    uint32                      *data_indx,
    DNX_SAND_OUT    uint8                       *found
  )
{
  uint8
    is_found,
    first ;
  uint32
    entry_offset,
    prev_entry ;
  uint32
    hash_table_index,
    res ;

  DNX_SAND_INIT_ERROR_DEFINITIONS(DNX_SAND_HASH_TABLE_ENTRY_LOOKUP);
  DNX_SAND_HASH_TABLE_VERIFY_UNIT_IS_LEGAL(unit, 2) ;
  DNX_SAND_HASH_TABLE_CONVERT_HANDLE_TO_HASHTABLE_INDEX(hash_table_index,hash_table) ;
  DNX_SAND_HASH_TABLE_VERIFY_HASHTABLE_IS_ACTIVE(hash_table_index,4,6) ;

  DNX_SAND_CHECK_NULL_INPUT(key);
  DNX_SAND_CHECK_NULL_INPUT(data_indx);
  DNX_SAND_CHECK_NULL_INPUT(found);

  /* Check to see whether the entry exists */
  res = dnx_sand_hash_table_find_entry(
          unit,
          hash_table,
          key,
          FALSE,
          FALSE,
          &entry_offset,
          &is_found,
          &prev_entry,
          &first
        );
  DNX_SAND_CHECK_FUNC_RESULT(res, 10, exit);
  /*
   * if the key to remove is not exist in the hash table then this operation has
   * no side effect.
   */
  if (!is_found)
  {
    *found = FALSE;
    *data_indx = DNX_SAND_HASH_TABLE_NULL;
    goto exit;
  }
  *found = TRUE;
  *data_indx = entry_offset;

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in dnx_sand_hash_table_entry_lookup()",0,0);
}

/*********************************************************************
* NAME:
*     dnx_sand_hash_table_entry_get_by_index
* TYPE:
*   PROC
* DATE:
*   Mar 26 2015
* FUNCTION:
*  return an entry from a hash table given the index identify
*  this entry.
* INPUT:
*  DNX_SAND_IN  int                        unit -
*     Identifier of the device to access.
*  DNX_SAND_IN  DNX_SAND_HASH_TABLE_PTR    hash_table -
*     The hash table to add to.
*  DNX_SAND_IN  uint32                     data_indx -
*     index of the entry to return.
*  DNX_SAND_OUT  DNX_SAND_HASH_TABLE_KEY   *const key -
*     key resides in this entry
*  DNX_SAND_OUT  uint8                     *found -
*     whether this key is valid
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  dnx_sand_hash_table_get_by_index(
    DNX_SAND_IN  int                           unit,
    DNX_SAND_IN  DNX_SAND_HASH_TABLE_PTR       hash_table,
    DNX_SAND_IN  uint32                        data_indx,
    DNX_SAND_OUT DNX_SAND_HASH_TABLE_KEY       *key,
    DNX_SAND_OUT uint8                         *found
  )
{
  uint32
    table_size,
    key_size,
    hash_table_index = 0,
    res ;
  DNX_SAND_OCC_BM_PTR
    memory_use ;

  DNX_SAND_INIT_ERROR_DEFINITIONS(DNX_SAND_HASH_TABLE_ENTRY_LOOKUP);
  DNX_SAND_HASH_TABLE_VERIFY_UNIT_IS_LEGAL(unit, 2) ;
  DNX_SAND_HASH_TABLE_CONVERT_HANDLE_TO_HASHTABLE_INDEX(hash_table_index,hash_table) ;
  DNX_SAND_HASH_TABLE_VERIFY_HASHTABLE_IS_ACTIVE(hash_table_index,4,6) ;

  res = HASH_TABLE_ACCESS_INFO.table_size.get(unit, hash_table_index, &table_size) ;
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 10, exit) ;
  res = HASH_TABLE_ACCESS_INFO.key_size.get(unit, hash_table_index, &key_size) ;
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 14, exit) ;
  res = HASH_TABLE_ACCESS_DATA.memory_use.get(unit,hash_table_index,&memory_use) ;
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 16, exit) ;

  if (data_indx > table_size)
  {
    DNX_SAND_SET_ERROR_CODE(DNX_SAND_VALUE_OUT_OF_RANGE_ERR, 10, exit);
  }
  /*
   * check if the entry 'entry_offset' is in use.
   */
  res = dnx_sand_occ_bm_is_occupied(
          unit,
          memory_use,
          data_indx,
          found
        );
  DNX_SAND_CHECK_FUNC_RESULT(res, 20, exit);
  /*
   * check if the entry 'entry_offset' is in use.
   */
  if (!*found)
  {
    goto exit;
  }
  /*
   * Copy 'key_size' bytes from 'HASH_TABLE_ACCESS_DATA.keys' buffer at offset
   * 'data_indx * key_size' into input 'key' buffer.
   */
  res = HASH_TABLE_ACCESS_DATA.keys.memread(unit,hash_table_index,key,key_size * data_indx,key_size) ;
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 26, exit) ;
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in dnx_sand_hash_table_entry_get_by_index()",data_indx,(hash_table_index)? table_size: 0) ;
}

/*********************************************************************
* NAME:
*   dnx_sand_hash_table_clear
* TYPE:
*   PROC
* DATE:
*   Mar 26 2015
* FUNCTION:
*   Clear the hash table content without setting free the memory
* INPUT:
*   DNX_SAND_IN int                        unit -
*     Identifier of the device to access.
*   DNX_SAND_IN DNX_SAND_HASH_TABLE_PTR    hash_table -
*     Handle of the hash table instance
* REMARKS:
*   None.
* RETURNS:
*   OK or ERROR indication.
*********************************************************************/
uint32
  dnx_sand_hash_table_clear(
    DNX_SAND_IN     int                        unit,
    DNX_SAND_INOUT  DNX_SAND_HASH_TABLE_PTR    hash_table
  )
{
  uint32
    max_buf_size,
    data_size,
    ptr_size,
    table_width,
    table_size,
    key_size,
    hash_table_index,
    res ;
  DNX_SAND_OCC_BM_PTR
    memory_use ;
  int32
    offset ;

  DNX_SAND_INIT_ERROR_DEFINITIONS(DNX_SAND_HASH_TABLE_CREATE);
  DNX_SAND_HASH_TABLE_VERIFY_UNIT_IS_LEGAL(unit, 2) ;
  DNX_SAND_HASH_TABLE_CONVERT_HANDLE_TO_HASHTABLE_INDEX(hash_table_index,hash_table) ;
  DNX_SAND_HASH_TABLE_VERIFY_HASHTABLE_IS_ACTIVE(hash_table_index,4,6) ;

  res = HASH_TABLE_ACCESS_INFO.table_size.get(unit, hash_table_index, &table_size) ;
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 10, exit) ;
  res = HASH_TABLE_ACCESS_INFO.key_size.get(unit, hash_table_index, &key_size) ;
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 14, exit) ;
  res = HASH_TABLE_ACCESS_DATA.memory_use.get(unit, hash_table_index, &memory_use) ;
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 16, exit) ;
  res = HASH_TABLE_ACCESS_DATA.ptr_size.get(unit, hash_table_index, &ptr_size) ;
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 18, exit) ;
  res = HASH_TABLE_ACCESS_INFO.table_width.get(unit, hash_table_index, &table_width) ;
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 20, exit) ;
  res = HASH_TABLE_ACCESS_INFO.data_size.get(unit, hash_table_index, &data_size) ;
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 22, exit) ;
  offset = 0 ;
  /*
   * Clear buffer for keys array
   */
  res = HASH_TABLE_ACCESS_DATA.keys.memset(unit,hash_table_index,offset,table_size * key_size,0x00) ;
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 24, exit) ;
  /*
   * clear buffer for next array (to build the linked list)
   */
  res = HASH_TABLE_ACCESS_DATA.next.memset(unit,hash_table_index,offset,table_size * ptr_size,0xFF) ;
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 28, exit) ;
  /*
   * clear buffer for lists_head (to build the linked list)
   */
  res = HASH_TABLE_ACCESS_DATA.lists_head.memset(unit,hash_table_index,offset,table_width * ptr_size,0xFF) ;
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 34, exit) ;

  max_buf_size = data_size ;
  if (key_size > max_buf_size)
  {
    max_buf_size = key_size;
  }
  if (ptr_size > max_buf_size)
  {
    max_buf_size = ptr_size;
  }
  /*
   * Clear buffers for workspace
   */
  res = HASH_TABLE_ACCESS_DATA.tmp_buf.memset(unit,hash_table_index,offset,max_buf_size,0x00) ;
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 38, exit) ;
  res = HASH_TABLE_ACCESS_DATA.tmp_buf2.memset(unit,hash_table_index,offset,max_buf_size,0x00) ;
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 42, exit) ;

  /* initialize the data to be mapped to*/

  res = dnx_sand_occ_bm_clear(
          unit,
          memory_use
        );
  DNX_SAND_CHECK_FUNC_RESULT(res,50, exit) ;
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in dnx_sand_hash_table_clear()",0,0) ;
}

/*********************************************************************
* NAME:
*     dnx_sand_hash_table_get_next
* TYPE:
*   PROC
* DATE:
*   Mar 26 2015
* FUNCTION:
*  get the next valid entry (key and data) in the hash table.
*  start traversing from the place pointed by the given iterator.
* INPUT:
*  DNX_SAND_IN    DNX_SAND_HASH_TABLE_PTR     hash_table -
*    Handle of the hash table.
*  DNX_SAND_INOUT DNX_SAND_HASH_TABLE_ITER    *iter
*    Iterator points to the entry to start traverse from.
*  DNX_SAND_OUT   DNX_SAND_HASH_TABLE_KEY     key -
*     the hash table key returned
*  DNX_SAND_OUT  DNX_SAND_HASH_TABLE_DATA     data -
*     the hash table data returned and associated with the key above.
* REMARKS:
*     - to start traverse the hash table from the beginning.
*       use DNX_SAND_HASH_TABLE_ITER_BEGIN(iter)
*     - to check if the iterator get to the end of the table use.
*       use DNX_SAND_HASH_TABLE_ITER_END(iter)
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  dnx_sand_hash_table_get_next(
    DNX_SAND_IN     int                         unit,
    DNX_SAND_IN     DNX_SAND_HASH_TABLE_PTR     hash_table,
    DNX_SAND_INOUT  DNX_SAND_HASH_TABLE_ITER    *iter,
    DNX_SAND_OUT    DNX_SAND_HASH_TABLE_KEY     *const key,
    DNX_SAND_OUT    uint32                      *data_indx
  )
{
  uint32
    indx ;
  uint32
    res,
    table_size,
    key_size,
    hash_table_index ;
  uint8
    occupied ;
  DNX_SAND_OCC_BM_PTR
    memory_use ;

  DNX_SAND_INIT_ERROR_DEFINITIONS(DNX_SAND_HASH_TABLE_GET_NEXT) ;
  DNX_SAND_HASH_TABLE_VERIFY_UNIT_IS_LEGAL(unit, 2) ;
  DNX_SAND_HASH_TABLE_CONVERT_HANDLE_TO_HASHTABLE_INDEX(hash_table_index,hash_table) ;
  DNX_SAND_HASH_TABLE_VERIFY_HASHTABLE_IS_ACTIVE(hash_table_index,4,6) ;

  DNX_SAND_CHECK_NULL_INPUT(iter);
  DNX_SAND_CHECK_NULL_INPUT(data_indx);

  res = HASH_TABLE_ACCESS_INFO.table_size.get(unit, hash_table_index, &table_size) ;
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 10, exit) ;
  res = HASH_TABLE_ACCESS_INFO.key_size.get(unit, hash_table_index, &key_size) ;
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 14, exit) ;
  res = HASH_TABLE_ACCESS_DATA.memory_use.get(unit, hash_table_index, &memory_use) ;
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 16, exit) ;
  /*
   * traverse the hash table head list.
   */
  for (indx = *iter; indx < table_size   ; ++indx)
  {
    res = dnx_sand_occ_bm_is_occupied(
            unit,
            memory_use,
            indx,
            &occupied
          );
    DNX_SAND_CHECK_FUNC_RESULT(res, 10, exit);

    if (occupied)
    {
      *data_indx = indx ;
      *iter = indx + 1 ;
      /*
       * Copy 'key_size' bytes from 'HASH_TABLE_ACCESS_DATA.keys' buffer at offset
       * 'indx * key_size' into input 'key' buffer.
       */
      res = HASH_TABLE_ACCESS_DATA.keys.memread(unit,hash_table_index,key,key_size * indx,key_size) ;
      DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 26, exit) ;
      goto exit ;
    }
  }
  *iter = DNX_SAND_U32_MAX ;

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in dnx_sand_hash_table_get_next()",0,0) ;
}

uint32
  dnx_sand_hash_table_get_size_for_save(
    DNX_SAND_IN   int                         unit,
    DNX_SAND_IN   DNX_SAND_HASH_TABLE_PTR     hash_table,
    DNX_SAND_OUT  uint32                      *size
  )
{
  uint32
    table_width,
    table_size,
    key_size,
    ptr_size,
    bmp_size,
    total_size ;
  uint32
    hash_table_index,
    res;
  DNX_SAND_OCC_BM_PTR
    memory_use ;

  DNX_SAND_INIT_ERROR_DEFINITIONS(0) ;

  total_size = 0 ;

  DNX_SAND_HASH_TABLE_VERIFY_UNIT_IS_LEGAL(unit, 2) ;
  DNX_SAND_HASH_TABLE_CONVERT_HANDLE_TO_HASHTABLE_INDEX(hash_table_index,hash_table) ;
  DNX_SAND_HASH_TABLE_VERIFY_HASHTABLE_IS_ACTIVE(hash_table_index,4,6) ;

  DNX_SAND_CHECK_NULL_INPUT(size) ;

  res = HASH_TABLE_ACCESS_INFO.table_size.get(unit, hash_table_index, &table_size) ;
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 10, exit) ;
  res = HASH_TABLE_ACCESS_INFO.key_size.get(unit, hash_table_index, &key_size) ;
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 14, exit) ;
  res = HASH_TABLE_ACCESS_DATA.memory_use.get(unit, hash_table_index, &memory_use) ;
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 16, exit) ;
  res = HASH_TABLE_ACCESS_DATA.ptr_size.get(unit, hash_table_index, &ptr_size) ;
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 18, exit) ;
  res = HASH_TABLE_ACCESS_INFO.table_width.get(unit, hash_table_index, &table_width) ;
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 20, exit) ;

  /* init info */
  total_size += sizeof(DNX_SAND_HASH_TABLE_INIT_INFO);

  /* DS data */
 
  total_size += table_size * key_size ;

  total_size += table_size * ptr_size ;

  total_size += table_width * ptr_size ;

  /* initialize the data to be mapped to */

  res = dnx_sand_occ_bm_get_size_for_save(
          unit,
          memory_use,
          &bmp_size
        ) ;
  DNX_SAND_CHECK_FUNC_RESULT(res,22, exit) ;
  total_size += bmp_size ;
  *size = total_size ;
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in dnx_sand_hash_table_get_size_for_save()",0,0);
}

/*********************************************************************
* NAME:
*     dnx_sand_hash_table_save
* TYPE:
*   PROC
* DATE:
*   Mar 26 2015
* FUNCTION:
*     saves the given hash table in the given buffer
* INPUT:
*   DNX_SAND_IN  int                     unit -
*     Identifier of the device to access.
*   DNX_SAND_IN  DNX_SAND_HASH_TABLE_PTR hash_table -
*     The Hash table to save.
*   DNX_SAND_OUT  uint8                 *buffer -
*     Buffer to contain the hash table
* REMARKS:
*   - the size of the buffer has to be at least as the value returned
*     by dnx_sand_hash_table_get_size_for_save.
*   - the hash and rehash functions are not saved.
* RETURNS:
*   OK or ERROR indication.
*********************************************************************/
uint32
  dnx_sand_hash_table_save(
    DNX_SAND_IN  int                         unit,
    DNX_SAND_IN  DNX_SAND_HASH_TABLE_PTR     hash_table,
    DNX_SAND_OUT uint8                       *buffer,
    DNX_SAND_IN  uint32                      buffer_size_bytes,
    DNX_SAND_OUT uint32                      *actual_size_bytes
  )
{
  uint8
    *cur_ptr ;
  uint32
    cur_size, table_size,
    key_size, ptr_size,
    table_width,
    total_size ;
  uint32
    hash_table_index,
    res ;
  DNX_SAND_OCC_BM_PTR
    memory_use ;
  int
    offset ;

  DNX_SAND_INIT_ERROR_DEFINITIONS(0);

  cur_ptr = buffer ;
  total_size = 0 ;

  DNX_SAND_HASH_TABLE_VERIFY_UNIT_IS_LEGAL(unit, 2) ;
  DNX_SAND_HASH_TABLE_CONVERT_HANDLE_TO_HASHTABLE_INDEX(hash_table_index,hash_table) ;
  DNX_SAND_HASH_TABLE_VERIFY_HASHTABLE_IS_ACTIVE(hash_table_index,4,6) ;

  DNX_SAND_CHECK_NULL_INPUT(buffer);
  DNX_SAND_CHECK_NULL_INPUT(actual_size_bytes);

  res = HASH_TABLE_ACCESS_INFO.table_size.get(unit, hash_table_index, &table_size) ;
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 10, exit) ;
  res = HASH_TABLE_ACCESS_INFO.key_size.get(unit, hash_table_index, &key_size) ;
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 14, exit) ;
  res = HASH_TABLE_ACCESS_DATA.memory_use.get(unit, hash_table_index, &memory_use) ;
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 16, exit) ;
  res = HASH_TABLE_ACCESS_DATA.ptr_size.get(unit, hash_table_index, &ptr_size) ;
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 18, exit) ;
  res = HASH_TABLE_ACCESS_INFO.table_width.get(unit, hash_table_index, &table_width) ;
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 20, exit) ;
  offset = 0 ;
  /* copy init info to 'cur_ptr' */
  res = HASH_TABLE_ACCESS_INFO.get(unit, hash_table_index, (DNX_SAND_HASH_TABLE_INIT_INFO *)cur_ptr) ;
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 10, exit) ;
  cur_ptr += sizeof(DNX_SAND_HASH_TABLE_INIT_INFO) ;

  /* copy DS data to 'cur_ptr' */
  res = HASH_TABLE_ACCESS_DATA.keys.memread(unit, hash_table_index, cur_ptr, offset,table_size * key_size) ;
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 10, exit) ;
  cur_ptr += (table_size * key_size) ;

  res = HASH_TABLE_ACCESS_DATA.next.memread(unit, hash_table_index, cur_ptr, offset,table_size * ptr_size) ;
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 10, exit) ;
  cur_ptr += (table_size * ptr_size) ;

  res = HASH_TABLE_ACCESS_DATA.lists_head.memread(unit, hash_table_index, cur_ptr, offset,table_width * ptr_size) ;
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 10, exit) ;
  cur_ptr += (table_width * ptr_size) ;

  /* initialize the data to be mapped to */
  res = dnx_sand_occ_bm_save(
          unit,
          memory_use,
          cur_ptr,
          buffer_size_bytes - total_size,
          &cur_size
        );
  DNX_SAND_CHECK_FUNC_RESULT(res,20, exit);
  cur_ptr += cur_size ;
  total_size += cur_size ;
  *actual_size_bytes = total_size ;
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in dnx_sand_hash_table_save()",0,0);
}

uint32
  dnx_sand_hash_table_load(
    DNX_SAND_IN  int                                   unit,
    DNX_SAND_IN  uint8                                 **buffer,
    DNX_SAND_IN  DNX_SAND_HASH_MAP_SW_DB_ENTRY_SET     set_entry_fun,
    DNX_SAND_IN  DNX_SAND_HASH_MAP_SW_DB_ENTRY_GET     get_entry_fun,
    DNX_SAND_IN  DNX_SAND_HASH_MAP_HASH_FUN_CALL_BACK  hash_function,
    DNX_SAND_IN  DNX_SAND_HASH_MAP_HASH_FUN_CALL_BACK  rehash_function,
    DNX_SAND_OUT DNX_SAND_HASH_TABLE_PTR               *hash_table_ptr
  )
{
  DNX_SAND_IN uint8
    *cur_ptr ;
  DNX_SAND_HASH_TABLE_PTR
    hash_table ;
  uint32
    hash_table_index, table_size,
    key_size, ptr_size,
    table_width,
    res ;
  DNX_SAND_HASH_TABLE_INIT_INFO
    init_info ;
  int
    offset ;
  DNX_SAND_OCC_BM_PTR
    memory_use ;

  DNX_SAND_INIT_ERROR_DEFINITIONS(0);
  DNX_SAND_HASH_TABLE_VERIFY_UNIT_IS_LEGAL(unit, 2) ;

  DNX_SAND_CHECK_NULL_INPUT(buffer) ;

  cur_ptr = (DNX_SAND_IN uint8 *)buffer[0] ;

  /* copy init info from 'cur_ptr' */
  dnx_sand_os_memcpy(&(init_info), cur_ptr, sizeof(DNX_SAND_HASH_TABLE_INIT_INFO)) ;
  cur_ptr += sizeof(DNX_SAND_HASH_TABLE_INIT_INFO) ;
  init_info.hash_function = hash_function ;
  init_info.rehash_function = rehash_function ;
  init_info.set_entry_fun = set_entry_fun ;
  init_info.get_entry_fun = get_entry_fun ;

  /* create DS */

  /* will not work!! (petra only code) */
  res = dnx_sand_hash_table_create(unit, &hash_table, init_info);
  DNX_SAND_CHECK_FUNC_RESULT(res,20, exit);
  *hash_table_ptr = hash_table ;
  DNX_SAND_HASH_TABLE_CONVERT_HANDLE_TO_HASHTABLE_INDEX(hash_table_index,hash_table) ;

  res = HASH_TABLE_ACCESS_INFO.table_size.get(unit, hash_table_index, &table_size) ;
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 10, exit) ;
  res = HASH_TABLE_ACCESS_INFO.key_size.get(unit, hash_table_index, &key_size) ;
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 14, exit) ;
  res = HASH_TABLE_ACCESS_DATA.memory_use.get(unit, hash_table_index, &memory_use) ;
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 16, exit) ;
  res = HASH_TABLE_ACCESS_DATA.ptr_size.get(unit, hash_table_index, &ptr_size) ;
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 18, exit) ;
  res = HASH_TABLE_ACCESS_INFO.table_width.get(unit, hash_table_index, &table_width) ;
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 20, exit) ;
  offset = 0 ;

  /* copy DS data from 'cur_ptr' */

  res = HASH_TABLE_ACCESS_DATA.keys.memwrite(unit, hash_table_index, cur_ptr, offset,table_size * key_size) ;
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 22, exit) ;
  cur_ptr += (table_size * key_size) ;

  res = HASH_TABLE_ACCESS_DATA.next.memwrite(unit, hash_table_index, cur_ptr, offset,table_size * ptr_size) ;
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 24, exit) ;
  cur_ptr += (table_size * ptr_size) ;

  res = HASH_TABLE_ACCESS_DATA.lists_head.memwrite(unit, hash_table_index, cur_ptr, offset,table_width * ptr_size) ;
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 26, exit) ;
  cur_ptr += (table_width * ptr_size) ;

  /* load bitmap from 'cur_ptr' -- Start by destroy */
  res = dnx_sand_occ_bm_destroy(
          unit,
          memory_use
        );
  DNX_SAND_CHECK_FUNC_RESULT(res,30, exit);

  /* load bitmap */
  res = dnx_sand_occ_bm_load(
          unit,
          &cur_ptr,
          &memory_use
        );
  DNX_SAND_CHECK_FUNC_RESULT(res,34, exit);
  res = HASH_TABLE_ACCESS_DATA.memory_use.set(unit, hash_table_index, memory_use) ;
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 36, exit) ;
  *buffer = cur_ptr ;
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in dnx_sand_hash_table_load()",0,0);
}

uint32
  dnx_sand_SAND_HASH_TABLE_INFO_clear(
    DNX_SAND_IN  int                      unit,
    DNX_SAND_IN  DNX_SAND_HASH_TABLE_PTR  hash_table
  )
{
  DNX_SAND_HASH_TABLE_INIT_INFO
    info ;
  uint32
    hash_table_index,
    res ;

  DNX_SAND_INIT_ERROR_DEFINITIONS(0) ;
  DNX_SAND_HASH_TABLE_VERIFY_UNIT_IS_LEGAL(unit, 2) ;
  DNX_SAND_HASH_TABLE_CONVERT_HANDLE_TO_HASHTABLE_INDEX(hash_table_index,hash_table) ;
  DNX_SAND_HASH_TABLE_VERIFY_HASHTABLE_IS_ACTIVE(hash_table_index,4,6) ;

  dnx_sand_os_memset((void *)&info, 0x0, (uint32)sizeof(DNX_SAND_HASH_TABLE_INIT_INFO)) ;
  res = HASH_TABLE_ACCESS_INFO.set(unit, hash_table_index, &info) ;
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 8, exit) ;
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in dnx_sand_SAND_HASH_TABLE_INFO_clear()", 0, 0) ;
}

#if DNX_SAND_DEBUG
/* { */
/*********************************************************************
* NAME:
*     dnx_sand_hash_table_print
* TYPE:
*   PROC
* DATE:
*   Mar 26 2015
* FUNCTION:
*   prints the hash table content, all entries including entries not in use.
* INPUT:
*  DNX_SAND_IN  DNX_SAND_HASH_TABLE_PTR hash_table -
*     The hash table to print.
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  dnx_sand_hash_table_print(
    DNX_SAND_IN  int                     unit,
    DNX_SAND_IN  DNX_SAND_HASH_TABLE_PTR hash_table
  )
{
  uint32
    indx ;
  uint32
    ptr_long,
    print_indx ;
  uint8
    *list_head,
    *cur_key,
    *next ;
  uint32
    table_width,
    ptr_size,
    null_ptr,
    key_size,
    hash_table_index,
    res ;
  uint8
    *tmp_buf_ptr ;

  DNX_SAND_INIT_ERROR_DEFINITIONS(DNX_SAND_HASH_TABLE_PRINT);
  DNX_SAND_HASH_TABLE_VERIFY_UNIT_IS_LEGAL(unit, 2) ;
  DNX_SAND_HASH_TABLE_CONVERT_HANDLE_TO_HASHTABLE_INDEX(hash_table_index,hash_table) ;
  DNX_SAND_HASH_TABLE_VERIFY_HASHTABLE_IS_ACTIVE(hash_table_index,4,6) ;

  res = HASH_TABLE_ACCESS_INFO.key_size.get(unit, hash_table_index, &key_size) ;
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 14, exit) ;
  res = HASH_TABLE_ACCESS_DATA.ptr_size.get(unit, hash_table_index, &ptr_size) ;
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 18, exit) ;
  res = HASH_TABLE_ACCESS_INFO.table_width.get(unit, hash_table_index, &table_width) ;
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 20, exit) ;
  res = HASH_TABLE_ACCESS_DATA.null_ptr.get(unit, hash_table_index, &null_ptr) ;
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 24, exit) ;
  res = dnx_sand_hash_table_get_tmp_data_ptr_from_handle(unit,hash_table,&tmp_buf_ptr) ;
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 22, exit) ;
  /*
   * traverse the hash table head list.
   */
  for (indx = 0; indx < table_width   ; ++indx)
  {
    /*
     * Copy 'ptr_size' bytes from 'HASH_TABLE_ACCESS_DATA.lists_head' buffer at offset
     * 'indx * ptr_size' into 'tmp_buf' buffer.
     */
    res = HASH_TABLE_ACCESS_DATA.lists_head.memread(unit,hash_table_index,tmp_buf_ptr,ptr_size * indx,ptr_size) ;
    DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 26, exit) ;
    list_head = tmp_buf_ptr ;
    ptr_long = 0;

    res = dnx_sand_U8_to_U32(
            list_head,
            ptr_size,
            &ptr_long
          );
    DNX_SAND_CHECK_FUNC_RESULT(res, 20, exit);
    if (ptr_long == null_ptr)
    {
      continue ;
    }
    LOG_CLI((BSL_META_U(unit, " entry %u:  "), indx));

    while (ptr_long != null_ptr)
    {
      /*
       * read keys
       */
      /*
       * Copy 'key_size' bytes from 'HASH_TABLE_ACCESS_DATA.keys' buffer at offset
       * 'ptr_long * key_size' into 'tmp_buf' buffer.
       */
      res = HASH_TABLE_ACCESS_DATA.keys.memread(unit,hash_table_index,tmp_buf_ptr,key_size * ptr_long,key_size) ;
      DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 26, exit) ;
      cur_key = tmp_buf_ptr ;

      LOG_CLI((BSL_META_U(unit,"(0x"))) ;
      for(print_indx = 0; print_indx < key_size; ++print_indx)
      {
         LOG_CLI((BSL_META_U(unit,"%02x"), cur_key[key_size - print_indx - 1]));
      }
      LOG_CLI((BSL_META_U(unit,"--> %u)"), ptr_long - 1));
      LOG_CLI((BSL_META_U(unit,"\t")));
      /*
       * get next node
       */
      /*
       * Copy 'ptr_size' bytes from 'HASH_TABLE_ACCESS_DATA.next' buffer at offset
       * 'ptr_long * ptr_size' into 'tmp_buf' buffer.
       */
      res = HASH_TABLE_ACCESS_DATA.next.memread(unit,hash_table_index,tmp_buf_ptr,ptr_size * ptr_long,ptr_size) ;
      DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 26, exit) ;
      next = tmp_buf_ptr ;
      ptr_long = 0;
      res = dnx_sand_U8_to_U32(
              next,
              ptr_size,
              &ptr_long
            );
      DNX_SAND_CHECK_FUNC_RESULT(res, 50, exit);

      LOG_CLI((BSL_META_U(unit,"ptr_long--> %u)"), ptr_long ));
    }
    LOG_CLI((BSL_META_U(unit,"\n")));
  }
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in dnx_sand_hash_table_print()",0,0);
}


uint32
  dnx_sand_SAND_HASH_TABLE_INFO_print(
    DNX_SAND_IN int                      unit,
    DNX_SAND_IN  DNX_SAND_HASH_TABLE_PTR hash_table
  )
{
  uint32
    res,
    data_size,
    key_size,
    table_size,
    table_width,
    hash_table_index ;

  DNX_SAND_INIT_ERROR_DEFINITIONS(0);
  DNX_SAND_HASH_TABLE_VERIFY_UNIT_IS_LEGAL(unit, 2) ;
  DNX_SAND_HASH_TABLE_CONVERT_HANDLE_TO_HASHTABLE_INDEX(hash_table_index,hash_table) ;
  DNX_SAND_HASH_TABLE_VERIFY_HASHTABLE_IS_ACTIVE(hash_table_index,4,6) ;

  res = HASH_TABLE_ACCESS_INFO.data_size.get(unit, hash_table_index, &data_size) ;
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 18, exit) ;
  res = HASH_TABLE_ACCESS_INFO.key_size.get(unit, hash_table_index, &key_size) ;
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 20, exit) ;
  res = HASH_TABLE_ACCESS_INFO.table_size.get(unit, hash_table_index, &table_size) ;
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 24, exit) ;
  res = HASH_TABLE_ACCESS_INFO.table_width.get(unit, hash_table_index, &table_width) ;
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 28, exit) ;

  LOG_CLI((BSL_META_U(unit,"init_info.data_size  : %u\n"),data_size)) ;
  LOG_CLI((BSL_META_U(unit,"init_info.key_size   : %u\n"),key_size)) ;
  LOG_CLI((BSL_META_U(unit,"init_info.table_size : %u\n"),table_size)) ;
  LOG_CLI((BSL_META_U(unit,"init_info.table_width: %u\n"),table_width)) ;
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in dnx_sand_SAND_HASH_TABLE_INFO_print()", 0, 0);
}

/*****************************************************
*NAME
*  dnx_sand_hash_table_TEST_1
*TYPE:
*  PROC
*DATE:
*  15-Feb-2008
*FUNCTION:
* Verification add and lookup operation act well in a simple test
* 1.  Create hash table
* 2.  Add key and data to the hash table and lookup each time
*     Expect to find the key and the data in each lookup
*
*INPUT:
*  DNX_SAND_DIRECT:
*    DNX_SAND_IN uint32 silent -
*    Indicator.
*    1 - Do not print debuging info.
*    0 - Print various debuging info.
*  DNX_SAND_INDIRECT:
*    None.
*OUTPUT:
*  DNX_SAND_DIRECT:
*    uint32 -
*    Indicator.
*    1 - Test pass.
*    0 - Test fail.
*  DNX_SAND_INDIRECT:
*    NON
*REMARKS:
*
*SEE ALSO:
*****************************************************/
/* }*/
#endif /* DNX_SAND_DEBUG */


/*********************************************************************
* NAME:
*   dnx_sand_hash_table_find_entry
* TYPE:
*   PROC
* DATE:
*   Mar 26 2015
* FUNCTION:
*  lookup in the hash table for the given key and return the data inserted with
*  the given key.
* INPUT:
*  DNX_SAND_IN  int                            unit -
*     Identifier of the device to access.
*  DNX_SAND_IN  DNX_SAND_HASH_TABLE_PTR        hash_table -
*     The hash table.
*   DNX_SAND_IN  DNX_SAND_HASH_TABLE_KEY       key -
*     The key to lookup for
*   DNX_SAND_IN  uint8                         get_first_empty
*     whether to return the first empty entry .
*   DNX_SAND_OUT  uint32                      *entry -
*     if the key is present in the hash table then return the entry the key found at,
*     otherwise it returns the place where the key suppose to be.
*   DNX_SAND_IN  uint8                        *found -
*     whether the key was found in the hash table
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
static uint32
  dnx_sand_hash_table_find_entry(
    DNX_SAND_IN     int                          unit,
    DNX_SAND_INOUT  DNX_SAND_HASH_TABLE_PTR      hash_table,
    DNX_SAND_IN     DNX_SAND_HASH_TABLE_KEY      *const key,
    DNX_SAND_IN     uint8                        get_first_empty,
    DNX_SAND_IN     uint8                        alloc_by_index,
    DNX_SAND_OUT    uint32                       *entry,
    DNX_SAND_OUT    uint8                        *found,
    DNX_SAND_OUT    uint32                       *prev_entry,
    DNX_SAND_OUT    uint8                        *first
  )
{
  uint8
    *cur_key,
    *next;
  uint32
    key_size,
    ptr_size,
    null_ptr,
    hash_val,
    ptr_long,
    next_node;
  uint8
    not_found,
    found_new;
  uint32
    res,
    hash_table_index ;
  uint8
    *tmp_buf_ptr ;
  DNX_SAND_OCC_BM_PTR
    memory_use ;
  DNX_SAND_INIT_ERROR_DEFINITIONS(DNX_SAND_HASH_TABLE_FIND_ENTRY);
  DNX_SAND_HASH_TABLE_VERIFY_UNIT_IS_LEGAL(unit, 2) ;
  DNX_SAND_HASH_TABLE_CONVERT_HANDLE_TO_HASHTABLE_INDEX(hash_table_index,hash_table) ;
  DNX_SAND_HASH_TABLE_VERIFY_HASHTABLE_IS_ACTIVE(hash_table_index,4,6) ;

  DNX_SAND_CHECK_NULL_INPUT(hash_table);
  DNX_SAND_CHECK_NULL_INPUT(key);
  DNX_SAND_CHECK_NULL_INPUT(found);

  res = HASH_TABLE_ACCESS_DATA.ptr_size.get(unit, hash_table_index, &ptr_size) ;
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 10, exit) ;
  res = HASH_TABLE_ACCESS_INFO.key_size.get(unit, hash_table_index, &key_size) ;
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 12, exit) ;
  res = HASH_TABLE_ACCESS_DATA.null_ptr.get(unit, hash_table_index, &null_ptr) ;
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 14, exit) ;
  res = HASH_TABLE_ACCESS_DATA.memory_use.get(unit,hash_table_index,&memory_use) ;
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 16, exit) ;

  ptr_long = 0 ;
  *prev_entry = DNX_SAND_HASH_TABLE_NULL ;
  *first = TRUE ;
  /*
   * hash the key to get list head
   */
  res = dnx_sand_hash_table_simple_hash(
      unit,
      hash_table,
      key,
      0,
      &hash_val
    );
  DNX_SAND_CHECK_FUNC_RESULT(res, 20, exit);
  *prev_entry = hash_val;
  /*
   * Read list head.
   */
  res = dnx_sand_hash_table_get_tmp_data_ptr_from_handle(unit,hash_table,&tmp_buf_ptr) ;
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 22, exit) ;
  /*
   * Copy 'ptr_size' bytes from 'HASH_TABLE_ACCESS_DATA.lists_head' buffer at offset
   * 'hash_val * ptr_size' into 'tmp_buf' buffer.
   */
  res = HASH_TABLE_ACCESS_DATA.lists_head.memread(unit,hash_table_index,tmp_buf_ptr,ptr_size * hash_val,ptr_size) ;
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 26, exit) ;
  /*
   * Check whether the list head is null.
   */
  ptr_long = 0;
  res = dnx_sand_U8_to_U32(
                       tmp_buf_ptr,
                       ptr_size,
                       &ptr_long
                       );
  DNX_SAND_CHECK_FUNC_RESULT(res, 30, exit);
  if (ptr_long == null_ptr)
  {
    if (get_first_empty)
    {
      if (alloc_by_index)
      {
        /* don't check if this index already in use */
        res = dnx_sand_occ_bm_occup_status_set(
                                               unit,
                                               memory_use,
                                               *entry,
                                               TRUE
                                               );
        DNX_SAND_CHECK_FUNC_RESULT(res, 40, exit);
        found_new = TRUE;

        ptr_long = *entry;
      }
      else
      {
        res = dnx_sand_occ_bm_alloc_next(
                                         unit,
                                         memory_use,
                                         &ptr_long,
                                         &found_new
                                         );
        DNX_SAND_CHECK_FUNC_RESULT(res, 50, exit);
      }
      if (!found_new)
      {
        ptr_long = DNX_SAND_HASH_TABLE_NULL;
        *entry = DNX_SAND_HASH_TABLE_NULL;
      }
      else
      {
        /**list_head = 0;*/
        res = dnx_sand_U32_to_U8(
                                 &ptr_long,
                                 ptr_size,
                                 tmp_buf_ptr
                                 );
        DNX_SAND_CHECK_FUNC_RESULT(res, 60, exit);
        /*
         * Copy 'ptr_size' bytes from 'tmp_buf' buffer into 'HASH_TABLE_ACCESS_DATA.lists_head'
         * buffer at offset 'hash_val * ptr_size'.
         */
        res = HASH_TABLE_ACCESS_DATA.lists_head.memwrite(unit,hash_table_index,tmp_buf_ptr,ptr_size * hash_val,ptr_size) ;
        DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 70, exit) ;
        *entry = ptr_long ;
      }
      *found = FALSE ;
      goto exit ;
    }
    *found = FALSE ;
    *entry = DNX_SAND_HASH_TABLE_NULL ;
    goto exit ;
  }
  not_found = TRUE ;


  while (ptr_long != null_ptr)
  {
    /*
     * read keys
     */
    /*
     * Copy 'key_size' bytes from 'HASH_TABLE_ACCESS_DATA.keys' buffer at offset
     * 'ptr_long * key_size' into 'tmp_buf' buffer.
     */
    res = HASH_TABLE_ACCESS_DATA.keys.memread(unit,hash_table_index,tmp_buf_ptr,key_size * ptr_long,key_size) ;
    DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 74, exit) ;
    cur_key = tmp_buf_ptr ;
    not_found =
      (uint8)dnx_sand_os_memcmp(
        cur_key,
        key,
        (key_size * sizeof(DNX_SAND_HASH_TABLE_KEY_TYPE))
      ) ;
    if (not_found == FALSE)
    {
      /*
       * if key was found, quit
       */
      *found = TRUE ;
      *entry = ptr_long ;
      goto exit ;
    }
    /*
     * If wasn't found then look in the next node on the list.
     */
    *first = FALSE ;
    /*
     * Get next node
     */
    /*
     * Copy 'ptr_size' bytes from 'HASH_TABLE_ACCESS_DATA.next' buffer at offset
     * 'ptr_long * ptr_size' into 'tmp_buf' buffer.
     */
    res = HASH_TABLE_ACCESS_DATA.next.memread(unit,hash_table_index,tmp_buf_ptr,ptr_size * ptr_long,ptr_size) ;
    DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 76, exit) ;
    next = tmp_buf_ptr ;
    *prev_entry = ptr_long ;
    ptr_long = 0 ;
    res = dnx_sand_U8_to_U32(
                         next,
                         ptr_size,
                         &ptr_long
                         );
    DNX_SAND_CHECK_FUNC_RESULT(res, 80, exit);
  }
  /*
   * Didn't find anything, return 'not found'
   * but point the entry to the place where the
   * key was supposed to be.
   */
  *found = FALSE;
  /*
   * Yet, if tries make rehashed but fail, then set entry
   * to point to the end of the table or the first empty entry according to the given parameter.
   */
  if (get_first_empty)
  {
    if (alloc_by_index)
    {
      /* don't check if this index already in use */
      res = dnx_sand_occ_bm_occup_status_set(
                                           unit,
                                           memory_use,
                                           *entry,
                                           TRUE
                                           );
      DNX_SAND_CHECK_FUNC_RESULT(res, 84, exit);
      found_new = TRUE;
      next_node = *entry;
    }
    else
    {
      res = dnx_sand_occ_bm_alloc_next(
                                     unit,
                                     memory_use,
                                     &next_node,
                                     &found_new
                                     );
      DNX_SAND_CHECK_FUNC_RESULT(res, 88, exit);
    }
    if (!found_new)
    {
      *entry = DNX_SAND_HASH_TABLE_NULL ;
    }
    else
    {
      res = dnx_sand_U32_to_U8(
                             &next_node,
                             ptr_size,
                             tmp_buf_ptr
                             );
      DNX_SAND_CHECK_FUNC_RESULT(res, 92, exit);
      /*
       * Copy 'ptr_size' bytes from 'tmp_buf' buffer into 'HASH_TABLE_ACCESS_DATA.next'
       * buffer at offset '(*prev_entry) * ptr_size'.
       */
      res = HASH_TABLE_ACCESS_DATA.next.memwrite(unit,hash_table_index,tmp_buf_ptr,ptr_size * (*prev_entry),ptr_size) ;
      DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 96, exit) ;
      *entry = next_node ;
    }
  }
  else
  {
    *entry = DNX_SAND_HASH_TABLE_NULL ;
  }

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in dnx_sand_hash_table_find_entry()",0,0) ;
}

#include <soc/dnx/legacy/SAND/Utils/sand_footer.h>
