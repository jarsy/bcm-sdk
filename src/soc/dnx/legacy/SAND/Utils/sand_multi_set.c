/* $Id: sand_multi_set.c,v 1.15 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/
/******************************************************************
* FILENAME:       dnx_sand_multi_set.c
*
* FILE DESCRIPTION:
*   The bit stream modules
* REMARKS:
* SW License Agreement: Dune Networks (c). CONFIDENTIAL PROPRIETARY INFORMATION.
* Any use of this Software is subject to Software License Agreement
* included in the Driver User Manual of this device.
* Any use of this Software constitutes an agreement to the terms
* of the above Software License Agreement.
******************************************************************/

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

#include <soc/dnx/legacy/SAND/Utils/sand_multi_set.h>
#include <soc/dnx/legacy/SAND/Utils/sand_os_interface.h>
#include <soc/dnx/legacy/SAND/Utils/sand_integer_arithmetic.h>
#include <soc/dnx/legacy/SAND/Management/sand_error_code.h>
#include <soc/dnx/legacy/SAND/Management/sand_general_macros.h>
#include <soc/dnx/legacy/drv.h>
/* } */

extern shr_sw_state_t *sw_state[BCM_MAX_NUM_UNITS];

/*************
* DEFINES   *
*************/
/* { */
#define DNX_SAND_MULTI_SET_HASH_WIDTH_FACTOR 1

/* } */

/*************
*  MACROS   *
*************/
/* { */
#define MULTI_SET_ACCESS          sw_state_access[unit].dnx.soc.sand.multi_set  
#define MULTI_SET_ACCESS_DATA     MULTI_SET_ACCESS.multis_array.multiset_data
#define MULTI_SET_ACCESS_INFO     MULTI_SET_ACCESS.multis_array.init_info
/*
 * Verify specific multi set index is marked as 'occupied'. If not, software goes to
 * exit with error code.
 * 
 * Notes:
 *   'unit' is assumed to be defined in the caller's scope.
 *   'res' is assumed to be defined in the caller's scope.
 *   'exit' is assumed to be defined in the caller's scope.
 */
#define DNX_SAND_MULTI_SET_VERIFY_MULTISET_IS_ACTIVE(_multi_set_index,_err1,_err2) \
  { \
    uint8 bit_val ; \
    uint32 max_nof_multis ; \
    res = MULTI_SET_ACCESS.max_nof_multis.get(unit, &max_nof_multis) ; \
    DNX_SAND_SOC_CHECK_FUNC_RESULT(res, _err1, exit) ; \
    if (_multi_set_index >= max_nof_multis) \
    { \
      /* \
       * If multiset handle is out of range then quit with error. \
       */ \
      bit_val = 0 ; \
    } \
    else \
    { \
      res = MULTI_SET_ACCESS.occupied_multis.bit_get(unit, (int)_multi_set_index, &bit_val) ; \
      DNX_SAND_SOC_CHECK_FUNC_RESULT(res, _err1, exit) ; \
    } \
    if (bit_val == 0) \
    { \
      /* \
       * If multiset structure is not indicated as 'occupied' then quit \
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
#define DNX_SAND_MULTI_SET_VERIFY_UNIT_IS_LEGAL(unit, _err1) \
  if ((unit < 0) || (unit >= SOC_MAX_NUM_DEVICES)) \
  { \
    /* \
     * If this is an illegal unit identifier, quit \
     * with error. \
     */ \
    DNX_SAND_SET_ERROR_CODE(DNX_SAND_MAX_NUM_DEVICES_OUT_OF_RANGE_ERR, _err1, exit); \
  }
/*
 * Convert input multi set handle to index in 'occupied_multis' array.
 * Convert input index in 'occupied_multis' array to multi set handle.
 * Indices go from 0 -> (occupied_multis - 1)
 * Handles go from 1 -> occupied_multis
 */
#define DNX_SAND_MULTI_SET_CONVERT_HANDLE_TO_MULTISET_INDEX(_multi_set_index,_handle) (_multi_set_index = _handle - 1)
#define DNX_SAND_MULTI_SET_CONVERT_MULTISET_INDEX_TO_HANDLE(_handle,_multi_set_index) (_handle = _multi_set_index + 1)
/* } */

/*************
* TYPE DEFS *
*************/
/* { */

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


/*
 * Get handle to multi set which will be considered illegal
 * by all multi set utilities.
 * Legal values for 'handle' are 1 -> MAX_NOF_MULTIS or, to be more precise:
 * 1 -> max_nof_multis (The value of MULTI_SET_ACCESS.max_nof_multis.get(unit, &max_nof_multis))
 */
uint32
  dnx_sand_multi_set_get_illegal_multiset_handle(
    void
  )
{
  return ((uint32)(-1)) ;
}
/*********************************************************************
* NAME:
*     dnx_sand_multi_set_get_member_size
* TYPE:
*   PROC
* DATE:
*   Mar 26 2015
* FUNCTION:
*   Get element 'member_size' for multi set.
* INPUT:
*   DNX_SAND_IN     int                           unit -
*     Identifier of the device to access.
*   DNX_SAND_IN  DNX_SAND_MULTI_SET_PTR           multi_set -
*     Handle to the multi set.
*   DNX_SAND_INOUT uint32                         *member_size_ptr -
*     This procedure loads pointed memory by the info element 'member_size'.
* REMARKS:
*   For external users nots aware of 'new sw state' structures.
* RETURNS:
*   OK or ERROR indication.
*********************************************************************/
uint32
  dnx_sand_multi_set_get_member_size(
    DNX_SAND_IN    int                              unit,
    DNX_SAND_IN    DNX_SAND_MULTI_SET_PTR           multi_set,
    DNX_SAND_INOUT uint32                           *member_size_ptr
  )
{
  uint32
    multi_set_index,
    res,
    member_size ;
  DNX_SAND_INIT_ERROR_DEFINITIONS(0);
  DNX_SAND_MULTI_SET_VERIFY_UNIT_IS_LEGAL(unit, 2) ;
  DNX_SAND_MULTI_SET_CONVERT_HANDLE_TO_MULTISET_INDEX(multi_set_index,multi_set) ;
  DNX_SAND_MULTI_SET_VERIFY_MULTISET_IS_ACTIVE(multi_set_index,4,6) ;

  res = MULTI_SET_ACCESS_INFO.member_size.get(unit, multi_set_index, &member_size) ;
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 10, exit) ;
  *member_size_ptr = member_size ;
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in dnx_sand_multi_set_get_member_size()",0,0);
}

uint32
  dnx_sand_multi_set_default_get_entry(
    DNX_SAND_IN  int                                unit,
    DNX_SAND_IN  uint32                             sec_hanlde,
    DNX_SAND_IN  uint8                             *buffer,
    DNX_SAND_IN  uint32                             offset,
    DNX_SAND_IN  uint32                             len,
    DNX_SAND_OUT uint8                             *data
  )
{
  uint32
    res = DNX_SAND_OK;

  DNX_SAND_INIT_ERROR_DEFINITIONS(0);

  res = dnx_sand_os_memcpy(
    data,
    buffer + (offset * len),
    len
    );
  DNX_SAND_CHECK_FUNC_RESULT(res, 10, exit);

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in dnx_sand_multi_set_default_get_entry()",0,0);
}

uint32
  dnx_sand_multi_set_default_set_entry(
    DNX_SAND_IN  int                                unit,
    DNX_SAND_IN  uint32                             sec_hanlde,
    DNX_SAND_INOUT  uint8                          *buffer,
    DNX_SAND_IN  uint32                             offset,
    DNX_SAND_IN  uint32                             len,
    DNX_SAND_IN  uint8                             *data
  )
{
  uint32
    res = DNX_SAND_OK;

  DNX_SAND_INIT_ERROR_DEFINITIONS(0);

  res = dnx_sand_os_memcpy(
    buffer + (offset * len),
    data,
    len
    );
  DNX_SAND_CHECK_FUNC_RESULT(res, 10, exit);

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in dnx_sand_multi_set_default_set_entry()",0,0);
}

static uint32
  dnx_sand_multi_set_member_add_internal(
    DNX_SAND_IN    int                       unit,
    DNX_SAND_IN    DNX_SAND_MULTI_SET_PTR    multi_set,
    DNX_SAND_IN    DNX_SAND_MULTI_SET_KEY    *const key,
    DNX_SAND_INOUT uint32                    *data_indx,
    DNX_SAND_IN    uint32                    nof_additions,
    DNX_SAND_OUT   uint8                     *first_appear,
    DNX_SAND_OUT   uint8                     *success
  ) ;

/*********************************************************************
* NAME:
*   dnx_sand_multi_set_init
* TYPE:
*   PROC
* DATE:
*   May 19 2015
* FUNCTION:
*   Initialize control structure for ALL multi set instances expected.
* INPUT:
*   DNX_SAND_IN  int unit -
*     Identifier of the device to access.
*   DNX_SAND_IN  uint32 max_nof_multis -
*     Maximal number of multi sets which can be sustained simultaneously.
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  dnx_sand_multi_set_init(
    DNX_SAND_IN       int                          unit,
    DNX_SAND_IN       uint32                       max_nof_multis
  )
{
  uint32 res ;

  DNX_SAND_INIT_ERROR_DEFINITIONS(DNX_SAND_MULTI_SET_INIT) ;
  DNX_SAND_MULTI_SET_VERIFY_UNIT_IS_LEGAL(unit, 2) ;

  res = MULTI_SET_ACCESS.alloc(unit);
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 10, exit);

  res = MULTI_SET_ACCESS.multis_array.ptr_alloc(unit, max_nof_multis);
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 20, exit);

  res = MULTI_SET_ACCESS.max_nof_multis.set(unit, max_nof_multis);
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 30, exit);

  res = MULTI_SET_ACCESS.in_use.set(unit, 0);
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 40, exit);

  res = MULTI_SET_ACCESS.occupied_multis.alloc_bitmap(unit, max_nof_multis);
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 60, exit);

  exit:
    DNX_SAND_EXIT_AND_SEND_ERROR( "error in dnx_sand_multi_set_init()",0,0);
}

/*********************************************************************
* NAME:
*     dnx_sand_multi_set_create
* TYPE:
*   PROC
* DATE:
*   June 01 2015
* FUNCTION:
*     Creates a new Multi set instance.
* INPUT:
*   DNX_SAND_IN    int                           unit -
*     Identifier of the device to access.
*   DNX_SAND_INOUT DNX_SAND_MULTI_SET_PTR        *multi_set -
*     This procedure loads pointed memory by handle of the newly created
*     multi set.
*   DNX_SAND_IN    DNX_SAND_MULTI_SET_INIT_INFO  init_info -
*     Information to use in order to create the mutli-set (size, duplications...)
* REMARKS:
*   None.
* RETURNS:
*   OK or ERROR indication.
*********************************************************************/
uint32
  dnx_sand_multi_set_create(
    DNX_SAND_IN     int                           unit,
    DNX_SAND_INOUT  DNX_SAND_MULTI_SET_PTR        *multi_set_ptr,
    DNX_SAND_IN     DNX_SAND_MULTI_SET_INIT_INFO  init_info
  )
{
  DNX_SAND_HASH_TABLE_INIT_INFO
    hash_init ;
  DNX_SAND_MULTI_SET_PTR
    multi_set;
  uint32
    res,
    multi_set_index,
    max_duplications,
    found,
    max_nof_multis ;
  uint32
    in_use ;
  uint8
    bit_val ;
  int
    prime_handle ;
  uint32
    member_size,
    sec_handle,
    nof_members ;
  DNX_SAND_MULTISET_SW_DB_ENTRY_GET
    get_entry_fun ;
  DNX_SAND_MULTISET_SW_DB_ENTRY_SET
    set_entry_fun ;
  uint32
    counter_size,
    global_counter ;
  DNX_SAND_HASH_TABLE_PTR
    hash_table ;

  DNX_SAND_INIT_ERROR_DEFINITIONS(DNX_SAND_MULTI_SET_CREATE);
  DNX_SAND_MULTI_SET_VERIFY_UNIT_IS_LEGAL(unit, 2) ;

  DNX_SAND_CHECK_NULL_INPUT(multi_set_ptr);

  res = MULTI_SET_ACCESS.in_use.get(unit, &in_use);
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 4, exit);

  res = MULTI_SET_ACCESS.max_nof_multis.get(unit, &max_nof_multis);
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 6, exit);

  if (in_use >= max_nof_multis)
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
  res = MULTI_SET_ACCESS.in_use.set(unit, (in_use + 1));
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 10, exit);
  /*
   * Find a free multi (a cleared bit in 'occupied_multis'). At this point,
   * there must be one.
   */
  found = 0 ;
  for (multi_set_index = 0 ; multi_set_index < max_nof_multis ; multi_set_index++)
  {
    res = MULTI_SET_ACCESS.occupied_multis.bit_get(unit, multi_set_index, &bit_val);
    DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 12, exit);
    if (bit_val == 0)
    {
      /*
       * 'multi_set_index' is now the index of a free entry.
       */
      found = 1 ;
      break ;
    }
  }
  if (!found)
  {
    DNX_SAND_SET_ERROR_CODE(DNX_SAND_MALLOC_FAIL, 14, exit);
  }
  res = MULTI_SET_ACCESS.occupied_multis.bit_set(unit, multi_set_index);
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 16, exit);

  res = MULTI_SET_ACCESS.multis_array.alloc(unit, multi_set_index);
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 18, exit);
  /*
   * Note that legal handles start at '1', not at '0'.
   */
  DNX_SAND_MULTI_SET_CONVERT_MULTISET_INDEX_TO_HANDLE(multi_set,multi_set_index) ;
  /*
   * Set output of this procedure.
   */
  *multi_set_ptr = multi_set ;
  /*
   * Load 'init' section of multi set as per input.
   * Make sure all callbacks are NULL - Currently, we only use defaults
   */
  if (init_info.get_entry_fun != NULL || init_info.set_entry_fun != NULL )
  {
    DNX_SAND_SET_ERROR_CODE(DNX_SAND_VALUE_OUT_OF_RANGE_ERR, 22, exit);
  }
  if (init_info.nof_members == 0 || init_info.member_size == 0)
  {
    DNX_SAND_SET_ERROR_CODE(DNX_SAND_VALUE_OUT_OF_RANGE_ERR, 26, exit);
  }
  res = MULTI_SET_ACCESS_INFO.set(unit, multi_set_index, &init_info) ;
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 30, exit) ;
  res = MULTI_SET_ACCESS_INFO.max_duplications.get(unit, multi_set_index, &max_duplications) ;
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 34, exit) ;
  if (max_duplications == DNX_SAND_U32_MAX)
  {
    max_duplications = DNX_SAND_U32_MAX - 1;
    res = MULTI_SET_ACCESS_INFO.max_duplications.set(unit, multi_set_index, max_duplications) ;
    DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 34, exit) ;
  }
  counter_size = 0 ;
  hash_table = dnx_sand_hash_table_get_illegal_hashtable_handle() ;

  res = MULTI_SET_ACCESS_DATA.counter_size.set(unit, multi_set_index, counter_size) ;
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 38, exit) ;
  res = MULTI_SET_ACCESS_DATA.hash_table.set(unit, multi_set_index, hash_table) ;
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 40, exit) ;
  /*
   * create hash-table.
   */
  dnx_sand_os_memset(&hash_init, 0x0, sizeof(DNX_SAND_HASH_TABLE_INIT_INFO));
  res = MULTI_SET_ACCESS_INFO.member_size.get(unit, multi_set_index, &member_size) ;
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 42, exit) ;
  res = MULTI_SET_ACCESS_INFO.prime_handle.get(unit, multi_set_index, &prime_handle) ;
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 44, exit) ;
  res = MULTI_SET_ACCESS_INFO.sec_handle.get(unit, multi_set_index, &sec_handle) ;
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 46, exit) ;
  res = MULTI_SET_ACCESS_INFO.nof_members.get(unit, multi_set_index, &nof_members) ;
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 50, exit) ;
  res = MULTI_SET_ACCESS_INFO.get_entry_fun.get(unit, multi_set_index, &get_entry_fun) ;
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 54, exit) ;
  res = MULTI_SET_ACCESS_INFO.set_entry_fun.get(unit, multi_set_index, &set_entry_fun) ;
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 58, exit) ;
  /*
   * Note: We take input 'init_info.global_max' as is unless it is zero (in
   * which case we ignore it).
   */

  hash_init.data_size = member_size ;
  hash_init.key_size = member_size ;
  hash_init.prime_handle = prime_handle ;
  hash_init.sec_handle = sec_handle ;
  hash_init.table_size = nof_members ;
  hash_init.table_width = nof_members * DNX_SAND_MULTI_SET_HASH_WIDTH_FACTOR ;
  hash_init.get_entry_fun = get_entry_fun ;
  hash_init.set_entry_fun = set_entry_fun ;

  global_counter = 0 ;
  res = MULTI_SET_ACCESS_DATA.global_counter.set(unit, multi_set_index, global_counter) ;
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 66, exit) ;
  res = dnx_sand_hash_table_create(
            unit,
            &hash_table,
            hash_init
          );
  DNX_SAND_CHECK_FUNC_RESULT(res, 70, exit) ;
  res = MULTI_SET_ACCESS_DATA.hash_table.set(unit, multi_set_index, hash_table) ;
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 71, exit) ;
  /*
   * If there is need to manage the duplications, then allocate array to
   * manage the reference counter.
   */
  if (max_duplications > 1)
  {
    /*
     * calculate the size of pointers (list head and next) according to table size.
     */
    counter_size = (dnx_sand_log2_round_up(max_duplications+1) + (DNX_SAND_NOF_BITS_IN_BYTE - 1)) / DNX_SAND_NOF_BITS_IN_BYTE;
    res = MULTI_SET_ACCESS_DATA.counter_size.set(unit, multi_set_index, counter_size) ;
    DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 72, exit) ;
    res = MULTI_SET_ACCESS_DATA.ref_counter.alloc(unit, multi_set_index, nof_members * counter_size) ;
    DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 74, exit) ;
    /*
     * Fill 'ref_counter' by zeroes starting at offset '0'.
     */
    res = MULTI_SET_ACCESS_DATA.ref_counter.memset(unit, multi_set_index, 0, nof_members * counter_size, 0x00) ;
    DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 78, exit) ;
  }

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in dnx_sand_multi_set_create()",0,0);
}

/*********************************************************************
* NAME:
*     dnx_sand_multi_set_destroy
* TYPE:
*   PROC
* DATE:
*   June 01 2015
* FUNCTION:
*     free the mutli-set instance.
* INPUT:
*  DNX_SAND_IN   int                        unit -
*     Identifier of the device to access.
*  DNX_SAND_IN   DNX_SAND_MULTI_SET_PTR     multi_set -
*    Handle to the mutli-set to destroy.
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  dnx_sand_multi_set_destroy(
    DNX_SAND_IN     int                     unit,
    DNX_SAND_IN     DNX_SAND_MULTI_SET_PTR  multi_set
  )
{
  uint32
    multi_set_index,
    res ;
  uint8
    bit_val ;
  uint32
    in_use ;
  DNX_SAND_HASH_TABLE_PTR
    hash_table ;
  uint32
    max_duplications,
    global_counter ;

  DNX_SAND_INIT_ERROR_DEFINITIONS(DNX_SAND_MULTI_SET_DESTROY);
  DNX_SAND_MULTI_SET_VERIFY_UNIT_IS_LEGAL(unit, 2) ;
  DNX_SAND_MULTI_SET_CONVERT_HANDLE_TO_MULTISET_INDEX(multi_set_index,multi_set) ;
  DNX_SAND_MULTI_SET_VERIFY_MULTISET_IS_ACTIVE(multi_set_index,4,6) ;
  /*
   * First, mark this multi set as 'released'
   */
  res = MULTI_SET_ACCESS.in_use.get(unit, &in_use);
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 8, exit);
  if ((int)in_use < 0)
  {
    /*
     * If number of occupied multiset structures goes below zero then quit
     * with error.
     */
    DNX_SAND_SET_ERROR_CODE(DNX_SAND_FREE_FAIL, 12, exit);
  }
  /*
   * Decrement number of 'in_use' to cover the one we now intend to release.
   */
  res = MULTI_SET_ACCESS.in_use.set(unit, (in_use - 1));
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 16, exit) ;
  /*
   * Mark specific multi set as 'not occupied'
   */
  res = MULTI_SET_ACCESS.occupied_multis.bit_get(unit, multi_set_index, &bit_val);
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 20, exit);
  if (bit_val == 0)
  {
    /*
     * If multi set structure is not indicated as 'occupied' then quit
     * with error.
     */
    DNX_SAND_SET_ERROR_CODE(DNX_SAND_FREE_FAIL, 24, exit) ;
  }
  res = MULTI_SET_ACCESS.occupied_multis.bit_clear(unit, multi_set_index);
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 28, exit);

  res = MULTI_SET_ACCESS_DATA.hash_table.get(unit, multi_set_index, &hash_table) ;
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 32, exit) ;
  res = dnx_sand_hash_table_destroy(
          unit,
          hash_table
        );
  DNX_SAND_CHECK_FUNC_RESULT(res, 36, exit) ;
  res = MULTI_SET_ACCESS_INFO.max_duplications.get(unit, multi_set_index, &max_duplications) ;
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 40, exit) ;
  /*
   * if there is need to manage the duplications, then allocate array to
   * manage the reference counter.
   */
  if (max_duplications > 1)
  {
    res = MULTI_SET_ACCESS_DATA.ref_counter.free(unit, multi_set_index) ;
    DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 44, exit) ;
  }
  global_counter = 0 ;
  res = MULTI_SET_ACCESS_DATA.global_counter.set(unit, multi_set_index, global_counter) ;
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 48, exit) ;
  res = MULTI_SET_ACCESS.multis_array.free(unit,multi_set_index) ;
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 52, exit) ;
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in dnx_sand_multi_set_destroy()",0,0);
}

/*********************************************************************
* NAME:
*     dnx_sand_multi_set_member_add
* TYPE:
*   PROC
* DATE:
*   June 01 2015
* FUNCTION:
*   Add a member to the mutli-set, if already exist then
*   then update the occurrences of this member.
* INPUT:
*   DNX_SAND_IN  int                           unit -
*     Identifier of the device to access.
*   DNX_SAND_IN  DNX_SAND_MULTI_SET_PTR        multi_set -
*    Handle to the mutli-set to add a member to.
*   DNX_SAND_IN  DNX_SAND_MULTI_SET_KEY        key -
*     The member to add.
*   DNX_SAND_OUT  uint32                       *data_indx -
*     Index identifies the place of the added member.
*     the given key
*   DNX_SAND_OUT  uint8                        *success -
*     whether the add operation success, this may be FALSE,
*     if there are no more space available in the multi-set
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  dnx_sand_multi_set_member_add(
    DNX_SAND_IN    int                      unit,
    DNX_SAND_IN    DNX_SAND_MULTI_SET_PTR   multi_set,
    DNX_SAND_IN    DNX_SAND_MULTI_SET_KEY   *const key,
    DNX_SAND_OUT   uint32                   *data_indx,
    DNX_SAND_OUT   uint8                    *first_appear,
    DNX_SAND_OUT   uint8                    *success
  )
{
  int nof_additions ;
  uint32
    multi_set_index,
    res ;

  DNX_SAND_INIT_ERROR_DEFINITIONS(DNX_SAND_MULTI_SET_MEMBER_ADD);
  DNX_SAND_MULTI_SET_VERIFY_UNIT_IS_LEGAL(unit, 2) ;
  DNX_SAND_MULTI_SET_CONVERT_HANDLE_TO_MULTISET_INDEX(multi_set_index,multi_set) ;
  DNX_SAND_MULTI_SET_VERIFY_MULTISET_IS_ACTIVE(multi_set_index,4,6) ;

  nof_additions = 1 ;
  *data_indx = DNX_SAND_U32_MAX ;  /* In this function, the data index wasn't given. Set to DNX_SAND_U32_MAX to indicate. */
  res = dnx_sand_multi_set_member_add_internal(
                unit,
                multi_set,
                key,
                data_indx,
                nof_additions,
                first_appear,
                success);
  DNX_SAND_CHECK_FUNC_RESULT(res, 10, exit);

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in dnx_sand_multi_set_member_add()",0,0);
}

/*********************************************************************
* NAME:
*     dnx_sand_multi_set_member_add_at_index
* TYPE:
*   PROC
* DATE:
*   June 01 2015
* FUNCTION:
*  Add one member to the mutli-set to a specific index, if already exist then
*  then update the occurrences of this member.
* INPUT:
*   DNX_SAND_IN  int                           unit -
*     Identifier of the device to access.
*   DNX_SAND_IN  DNX_SAND_MULTI_SET_PTR        multi_set -
*     The multi-set to add a member to.
*   DNX_SAND_IN  DNX_SAND_MULTI_SET_KEY        key -
*     The member to add.
*   DNX_SAND_OUT  uint32                       *data_indx -
*     Index identifies the place of the added member.
*     The function assumes that it already exists. 
*   DNX_SAND_OUT  uint8                        *success -
*     whether the add operation success, this may be FALSE,
*     if there are no more space available in the multi-set
* Remarks: 
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  dnx_sand_multi_set_member_add_at_index(
    DNX_SAND_IN    int                         unit,
    DNX_SAND_IN    DNX_SAND_MULTI_SET_PTR      multi_set,
    DNX_SAND_IN    DNX_SAND_MULTI_SET_KEY      *const key,
    DNX_SAND_IN    uint32                      data_indx,
    DNX_SAND_OUT   uint8                       *first_appear,
    DNX_SAND_OUT   uint8                       *success
  )
{
  uint32
    multi_set_index,
    res;
  const int nof_additions = 1 ;

  DNX_SAND_INIT_ERROR_DEFINITIONS(DNX_SAND_MULTI_SET_MEMBER_ADD);
  DNX_SAND_MULTI_SET_VERIFY_UNIT_IS_LEGAL(unit, 2) ;
  DNX_SAND_MULTI_SET_CONVERT_HANDLE_TO_MULTISET_INDEX(multi_set_index,multi_set) ;
  DNX_SAND_MULTI_SET_VERIFY_MULTISET_IS_ACTIVE(multi_set_index,4,6) ;

  res = dnx_sand_multi_set_member_add_at_index_nof_additions(
                unit,
                multi_set,
                key,
                data_indx,
                nof_additions,
                first_appear,
                success);
  DNX_SAND_CHECK_FUNC_RESULT(res, 10, exit);
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in dnx_sand_multi_set_member_add()",0,0);
}

/*********************************************************************
* NAME:
*     dnx_sand_multi_set_member_add_at_index_nof_additions
* TYPE:
*   PROC
* DATE:
*   June 01 2015
* FUNCTION:
*  Add nof members to the mutli-set to a specific index, if already exist then
*  then update the occurrences of this member.
* INPUT:
*   DNX_SAND_IN  int                           unit -
*     Identifier of the device to access.
*   DNX_SAND_IN  DNX_SAND_MULTI_SET_PTR        multi_set -
*     The multi-set to add a member to.
*   DNX_SAND_IN  DNX_SAND_MULTI_SET_KEY        key -
*     The member to add.
*   DNX_SAND_OUT  uint32                       *data_indx -
*     Index identifies the place of the added member.
*     The function assumes that it already exists. 
*   DNX_SAND_IN  uint32                        nof_additions -
*     Declare nof_additions to add the given key.
*     If given DNX_SAND_U32_MAX, the maximum number of entries will be added.
*   DNX_SAND_OUT  uint8                        *success -
*     whether the add operation success, this may be FALSE,
*     if there are no more space available in the multi-set
* Remarks: 
*     Similar to  dnx_sand_multi_set_member_add_at_index,
*     where nof_additions can be more than 1
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  dnx_sand_multi_set_member_add_at_index_nof_additions(
    DNX_SAND_IN    int                         unit,
    DNX_SAND_INOUT DNX_SAND_MULTI_SET_PTR      multi_set,
    DNX_SAND_IN    DNX_SAND_MULTI_SET_KEY      *const key,
    DNX_SAND_IN    uint32                      data_indx,
    DNX_SAND_IN    uint32                      nof_additions,
    DNX_SAND_OUT   uint8                       *first_appear,
    DNX_SAND_OUT   uint8                       *success
  )
{
  uint32
    multi_set_index,
    res;
  uint32
    data_index_ptr ;
  DNX_SAND_INIT_ERROR_DEFINITIONS(DNX_SAND_MULTI_SET_MEMBER_ADD) ;
  DNX_SAND_MULTI_SET_VERIFY_UNIT_IS_LEGAL(unit, 2) ;
  DNX_SAND_MULTI_SET_CONVERT_HANDLE_TO_MULTISET_INDEX(multi_set_index,multi_set) ;
  DNX_SAND_MULTI_SET_VERIFY_MULTISET_IS_ACTIVE(multi_set_index,4,6) ;

  data_index_ptr = data_indx ;
  res = dnx_sand_multi_set_member_add_internal(
              unit,
              multi_set,
              key,
              &data_index_ptr,
              nof_additions,
              first_appear,
              success);
  DNX_SAND_CHECK_FUNC_RESULT(res, 10, exit);
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in dnx_sand_multi_set_member_add_at_index_ref_count()",0,0);
}

/*********************************************************************
* NAME:
*     dnx_sand_multi_set_member_add_internal
* TYPE:
*   PROC
* DATE:
*   June 01 2015
* FUNCTION:
*  Add nof members to the mutli-set to either a specific index or a new index. If already exist then
*  then update the occurrences of this member.
* INPUT:
*   DNX_SAND_IN  int                           unit -
*     Identifier of the device to access.
*   DNX_SAND_INOUT  DNX_SAND_MULTI_SET_PTR     multi_set -
*     The multi-set to add a member to.
*   DNX_SAND_IN  DNX_SAND_MULTI_SET_KEY        key -
*     The member to add.
*   DNX_SAND_OUT  uint32                       *data_indx -
*     Index identifies the place of the added member.
*     If it's a new object, should be -1.
*   DNX_SAND_IN  uint32                        nof_additions -
*     Declare nof_additions to add the given key.
*     If given DNX_SAND_U32_MAX, the maximum number of entries will be added.
*   DNX_SAND_OUT  uint8                        *success -
*     whether the add operation success, this may be FALSE,
*     if there are no more space available in the multi-set
* Remarks: 
*     Similar to  dnx_sand_multi_set_member_add_at_index,
*     where nof_additions can be more than 1
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
static uint32
  dnx_sand_multi_set_member_add_internal(
    DNX_SAND_IN    int                       unit,
    DNX_SAND_IN    DNX_SAND_MULTI_SET_PTR    multi_set,
    DNX_SAND_IN    DNX_SAND_MULTI_SET_KEY    *const key,
    DNX_SAND_INOUT uint32                    *data_indx,
    DNX_SAND_IN    uint32                    nof_additions,
    DNX_SAND_OUT   uint8                     *first_appear,
    DNX_SAND_OUT   uint8                     *success
  )
{
  uint8
    tmp_cnt[sizeof(uint32)];
  uint32
    max_duplications,
    ref_count,
    counter_size,
    global_counter,
    global_max,
    found_index, 
    adjusted_additions ;
  uint8
    exist,
    with_id ;
  uint32
    multi_set_index,
    res;
  DNX_SAND_HASH_TABLE_PTR
    hash_table ;

  DNX_SAND_INIT_ERROR_DEFINITIONS(DNX_SAND_MULTI_SET_MEMBER_ADD);
  DNX_SAND_MULTI_SET_VERIFY_UNIT_IS_LEGAL(unit, 2) ;
  DNX_SAND_MULTI_SET_CONVERT_HANDLE_TO_MULTISET_INDEX(multi_set_index,multi_set) ;
  DNX_SAND_MULTI_SET_VERIFY_MULTISET_IS_ACTIVE(multi_set_index,4,6) ;

  DNX_SAND_CHECK_NULL_INPUT(key);
  DNX_SAND_CHECK_NULL_INPUT(data_indx);
  DNX_SAND_CHECK_NULL_INPUT(first_appear);
  DNX_SAND_CHECK_NULL_INPUT(success);

  with_id = (*data_indx != DNX_SAND_U32_MAX) ;
  if (nof_additions == 0)
  {
    *success = FALSE ;
    goto exit ;
  }

  /* In case of singlton set, it's enough to just check if the entry exists. */
  res = MULTI_SET_ACCESS_INFO.max_duplications.get(unit, multi_set_index, &max_duplications) ;
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 10, exit) ;
  res = MULTI_SET_ACCESS_DATA.hash_table.get(unit, multi_set_index, &hash_table) ;
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 14, exit) ;
  if (max_duplications <= 1)
  {
    res = dnx_sand_hash_table_entry_lookup(
            unit,
            hash_table,
            key,
            &found_index,
            &exist
          );
    DNX_SAND_CHECK_FUNC_RESULT(res, 18, exit);
    if (with_id)
    {
      /* if allocating by index and entry already exist in a different index, return error. */ 
      if (exist && found_index != *data_indx)
      {
        *first_appear = FALSE;
        *success = FALSE;
        goto exit;
      }
    }
    else 
    {
      /* If allocating for the first time, keep the index. */
      *data_indx = found_index;
    }
    if (exist && found_index != DNX_SAND_HASH_TABLE_NULL)
    {
      *first_appear = FALSE;
      *success = TRUE;
      goto exit;
    }
  }

  /* If given index, add at index. Otherwise, add without index. */
  if (with_id) {
      found_index = *data_indx;

      res = dnx_sand_hash_table_entry_add_at_index(
              unit,
              hash_table,
              key,
              found_index,
              success
            );
      DNX_SAND_CHECK_FUNC_RESULT(res, 22, exit);
  }
  else
  {
    res = dnx_sand_hash_table_entry_add(
              unit,
              hash_table,
              key,
              &found_index,
              success
            );
    DNX_SAND_CHECK_FUNC_RESULT(res, 26, exit);
  }

  /* If it's a singleton, and we didn't find it in the lookup, then it appears first. */
  if (max_duplications <= 1)
  {
    *first_appear = TRUE;
    goto exit;
  }
  if(!*success || found_index == DNX_SAND_HASH_TABLE_NULL)
  {
      *success = FALSE;
      *first_appear = (with_id) ? FALSE : TRUE; /* If we were given the index, we assume the user allocated the entry before the call. 
                                                            If we weren't given the index, we assume the operation failed because the table is 
                                                            full. */
      goto exit;
  }
  
  /* Save the index. */
  *data_indx = found_index;
  res = MULTI_SET_ACCESS_DATA.counter_size.get(unit, multi_set_index, &counter_size) ;
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 30, exit) ;
  /*
   * Copy 'counter_size' bytes from 'MULTI_SET_ACCESS_DATA.ref_counter' buffer at offset
   * '(*data_indx) * counter_size' into 'tmp_cnt' buffer.
   */
  res = MULTI_SET_ACCESS_DATA.ref_counter.memread(unit,multi_set_index,tmp_cnt,(*data_indx) * counter_size, counter_size) ;
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 34, exit) ;
  ref_count = 0;
  res = dnx_sand_U8_to_U32(
          tmp_cnt,
          counter_size,
          &ref_count
        );
  DNX_SAND_CHECK_FUNC_RESULT(res, 38, exit);

  /* Set if maximum entries is required. */
  if (nof_additions == DNX_SAND_U32_MAX) {
      adjusted_additions  = (max_duplications - ref_count);
  }
  else
  {
      adjusted_additions = nof_additions;
  }
  /*
   * Same value referenced more than initialized ed value times, return
   * operation fail.
   */
  if ((ref_count + adjusted_additions) > DNX_SAND_BITS_MASK((DNX_SAND_NOF_BITS_IN_BYTE * counter_size - 1),0))
  {
    /* Overflowing the counter. */
    *success = FALSE;
    goto exit;
  }
  if ((ref_count + adjusted_additions) > max_duplications) {    /* Adding more entries than there are available */
      *success = FALSE;
      goto exit;
  }
  res = MULTI_SET_ACCESS_DATA.global_counter.get(unit, multi_set_index, &global_counter) ;
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 42, exit) ;
  /*
   * Just to keep coverity happy...
   */
  global_max = 0 ;
  res = MULTI_SET_ACCESS_INFO.global_max.get(unit, multi_set_index, &global_max) ;
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 44, exit) ;
  if ((int)global_max > 0)
  {
    if ((global_counter + adjusted_additions) > global_max)
    {
      /* Adding more entries than the global counter allows. */
      res = _SHR_E_FULL ;
      DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 45, exit) ;
    }
  }
  if (ref_count == 0)
  {
    *first_appear = TRUE;
  }
  else
  {
    *first_appear = FALSE;
  }
  ref_count += adjusted_additions ;

  /* Update global counter. */
  global_counter += adjusted_additions;
  res = MULTI_SET_ACCESS_DATA.global_counter.set(unit, multi_set_index, global_counter) ;
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 46, exit) ;
  res = dnx_sand_U32_to_U8(
          &ref_count,
          counter_size,
          tmp_cnt
        );
  DNX_SAND_CHECK_FUNC_RESULT(res, 50, exit);
  /*
   * Copy 'counter_size' bytes from 'tmp_cnt' buffer into 'MULTI_SET_ACCESS_DATA.ref_counter'
   * buffer at offset '(*data_indx) * counter_size'.
   */
  res = MULTI_SET_ACCESS_DATA.ref_counter.memwrite(unit,multi_set_index,tmp_cnt,(*data_indx) * counter_size, counter_size) ;
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 54, exit) ;
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in dnx_sand_multi_set_member_add_at_index_ref_count()",0,0);
}

/*********************************************************************
* NAME:
*     dnx_sand_multi_set_member_remove
* TYPE:
*   PROC
* DATE:
*   June 01 2015
* FUNCTION:
*  Remove a member from a mutli-set, if the member is not belong to the multi-set
*  the operation has no effect.
* INPUT:
*  DNX_SAND_IN  int                           unit -
*     Identifier of the device to access.
*  DNX_SAND_IN  DNX_SAND_MULTI_SET_PTR        multi_set -
*     The mutli-set instance.
*  DNX_SAND_IN  DNX_SAND_MULTI_SET_KEY        *const key -
*     The key to remove from the mutli-set
*  DNX_SAND_OUT  uint32                       *data_indx -
*     where the key was found.
*  DNX_SAND_OUT  uint8                        *last_appear -
*     whether this was the last occurrence of this key in the multiset.
*     so no more occurrences of this key after this remove.
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  dnx_sand_multi_set_member_remove(
    DNX_SAND_IN    int                      unit,
    DNX_SAND_IN    DNX_SAND_MULTI_SET_PTR   multi_set,
    DNX_SAND_IN    DNX_SAND_MULTI_SET_KEY   *const key,
    DNX_SAND_OUT   uint32                   *data_indx,
    DNX_SAND_OUT   uint8                    *last_appear
  )
{
  uint8
    found ;
  uint32
    multi_set_index,
    res ;
  DNX_SAND_HASH_TABLE_PTR
    hash_table ;

  DNX_SAND_INIT_ERROR_DEFINITIONS(DNX_SAND_MULTI_SET_MEMBER_REMOVE);
  DNX_SAND_MULTI_SET_VERIFY_UNIT_IS_LEGAL(unit, 2) ;
  DNX_SAND_MULTI_SET_CONVERT_HANDLE_TO_MULTISET_INDEX(multi_set_index,multi_set) ;
  DNX_SAND_MULTI_SET_VERIFY_MULTISET_IS_ACTIVE(multi_set_index,4,6) ;

  DNX_SAND_CHECK_NULL_INPUT(key);
  res = MULTI_SET_ACCESS_DATA.hash_table.get(unit, multi_set_index, &hash_table) ;
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 14, exit) ;
  res = dnx_sand_hash_table_entry_lookup(
          unit,
          hash_table,
          key,
          data_indx,
          &found
        );
  DNX_SAND_CHECK_FUNC_RESULT(res, 18, exit);
  if (!found)
  {
    *data_indx = DNX_SAND_MULTI_SET_NULL ;
    *last_appear = FALSE ;
    goto exit ;
  }
  res = dnx_sand_multi_set_member_remove_by_index_multiple(unit, multi_set, *data_indx, 1, last_appear);
  DNX_SAND_CHECK_FUNC_RESULT(res, 20, exit);

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in dnx_sand_multi_set_member_remove()",0,0);
}

/*********************************************************************
* NAME:
*     dnx_sand_multi_set_member_remove_by_index
* TYPE:
*   PROC
* DATE:
*   June 01 2015
* FUNCTION:
*  Remove a member from a mutli-set, if the member is not belong to the multi-set
*  the operation has no effect.
* INPUT:
*  DNX_SAND_IN  int                         unit -
*    Identifier of the device to access.
*  DNX_SAND_IN  DNX_SAND_MULTI_SET_PTR      multi_set -
*    The mutli-set instance.
*  DNX_SAND_IN  uint32                      data_indx - 
*    The index of the entry to be deleted.
*  DNX_SAND_OUT  uint8                      *last_appear -
*    Will be set if it's the last appearance of the entry. 
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  dnx_sand_multi_set_member_remove_by_index(
    DNX_SAND_IN    int                       unit,
    DNX_SAND_INOUT DNX_SAND_MULTI_SET_PTR    multi_set,
    DNX_SAND_IN    uint32                    data_indx,
    DNX_SAND_OUT   uint8                     *last_appear
  )
{
  uint32
    multi_set_index,
    res ;

  DNX_SAND_INIT_ERROR_DEFINITIONS(DNX_SAND_MULTI_SET_MEMBER_REMOVE_BY_INDEX);
  DNX_SAND_MULTI_SET_VERIFY_UNIT_IS_LEGAL(unit, 2) ;
  DNX_SAND_MULTI_SET_CONVERT_HANDLE_TO_MULTISET_INDEX(multi_set_index,multi_set) ;
  DNX_SAND_MULTI_SET_VERIFY_MULTISET_IS_ACTIVE(multi_set_index,4,6) ;
  res = dnx_sand_multi_set_member_remove_by_index_multiple(unit, multi_set, data_indx, 1, last_appear);
  DNX_SAND_CHECK_FUNC_RESULT(res, 10, exit);

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in dnx_sand_multi_set_member_remove_by_index()",0,0);
}

/*********************************************************************
* NAME:
*     dnx_sand_multi_set_member_remove_by_index_multiple
* TYPE:
*   PROC
* DATE:
*   June 01 2015
* FUNCTION:
*  Remove a member from a mutli-set, if the member is not belong to the multi-set
*  the operation has no effect.
* INPUT:
*  DNX_SAND_IN  int                           unit -
*     Identifier of the device to access.
*  DNX_SAND_IN  DNX_SAND_MULTI_SET_PTR        multi_set -
*     The mutli-set instance.
*  DNX_SAND_IN  uint32                        data_indx - 
*    The index of the entry to be deleted.
*  DNX_SAND_IN  uint32                        remove_amount -
*    How many references of the entry to be removed. 
*  DNX_SAND_OUT  uint8                        *last_appear -
*    Will be set if it's the last appearance of the entry. 
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  dnx_sand_multi_set_member_remove_by_index_multiple(
    DNX_SAND_IN    int                       unit,
    DNX_SAND_IN    DNX_SAND_MULTI_SET_PTR    multi_set,
    DNX_SAND_IN    uint32                    data_indx,
    DNX_SAND_IN    uint32                    remove_amount,
    DNX_SAND_OUT   uint8                     *last_appear
  )
{
  uint32
    max_duplications,
    counter_size,
    global_counter,
    ref_count,
    adjusted_remove_amount ;
  uint8
    tmp_cnt[sizeof(uint32)] ;
  uint32
    multi_set_index,
    res ;
  DNX_SAND_HASH_TABLE_PTR
    hash_table ;

  DNX_SAND_INIT_ERROR_DEFINITIONS(DNX_SAND_MULTI_SET_MEMBER_REMOVE_BY_INDEX);
  DNX_SAND_MULTI_SET_VERIFY_UNIT_IS_LEGAL(unit, 2) ;
  DNX_SAND_MULTI_SET_CONVERT_HANDLE_TO_MULTISET_INDEX(multi_set_index,multi_set) ;
  DNX_SAND_MULTI_SET_VERIFY_MULTISET_IS_ACTIVE(multi_set_index,4,6) ;

  DNX_SAND_CHECK_NULL_INPUT(last_appear);
  ref_count = 0;
  res = MULTI_SET_ACCESS_INFO.max_duplications.get(unit, multi_set_index, &max_duplications) ;
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 10, exit) ;
  res = MULTI_SET_ACCESS_DATA.counter_size.get(unit, multi_set_index, &counter_size) ;
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 30, exit) ;
  if (max_duplications > 1)
  {
    /*
     * Copy 'counter_size' bytes from 'MULTI_SET_ACCESS_DATA.ref_counter' buffer at offset
     * 'data_indx * counter_size' into 'tmp_cnt' buffer.
     */
    res = MULTI_SET_ACCESS_DATA.ref_counter.memread(unit,multi_set_index,tmp_cnt,data_indx * counter_size, counter_size) ;
    DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 34, exit) ;
    dnx_sand_U8_to_U32(
      tmp_cnt,
      counter_size,
      &ref_count
    );
    if (ref_count == 0)
    {
      *last_appear = TRUE;
      goto exit;
    }

    /* Remove all entries */
    if ((remove_amount > ref_count))
    {
      adjusted_remove_amount = ref_count;
    }
    else
    {
      adjusted_remove_amount = remove_amount;
    }
    res = MULTI_SET_ACCESS_DATA.global_counter.get(unit, multi_set_index, &global_counter) ;
    DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 38, exit) ;
    global_counter -= adjusted_remove_amount ;
    res = MULTI_SET_ACCESS_DATA.global_counter.set(unit, multi_set_index, global_counter) ;
    DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 42, exit) ;
    ref_count -= adjusted_remove_amount;
    *last_appear = (ref_count == 0) ? TRUE : FALSE;
    res = dnx_sand_U32_to_U8(
            &ref_count,
            counter_size,
            tmp_cnt
          );
    DNX_SAND_CHECK_FUNC_RESULT(res, 46, exit);
    /*
     * Copy 'counter_size' bytes from 'tmp_cnt' buffer into 'MULTI_SET_ACCESS_DATA.ref_counter'
     * buffer at offset 'data_indx * counter_size'.
     */
    res = MULTI_SET_ACCESS_DATA.ref_counter.memwrite(unit,multi_set_index,tmp_cnt,data_indx * counter_size, counter_size) ;
    DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 54, exit) ;
  }
  if (*last_appear || max_duplications <= 1)
  {
    res = MULTI_SET_ACCESS_DATA.hash_table.get(unit, multi_set_index, &hash_table) ;
    DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 58, exit) ;
    res = dnx_sand_hash_table_entry_remove_by_index(
            unit,
            hash_table,
            data_indx
          );
    DNX_SAND_CHECK_FUNC_RESULT(res, 62, exit);
  }
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in dnx_sand_multi_set_member_remove_by_index()",0,0);
}

/*********************************************************************
* NAME:
*   dnx_sand_multi_set_member_lookup
* TYPE:
*   PROC
* DATE:
*   June 01 2015
* FUNCTION:
*   Lookup in the mutli-set for a member and return the occurrences/duplications of this
*   member in the multi-set, and the index identifying this member place.
*   the given key.
* INPUT:
*   DNX_SAND_IN  int                         unit -
*     Identifier of the device to access.
*   DNX_SAND_IN  DNX_SAND_MULTI_SET_PTR      multi_set -
*     The mutli-set.
*   DNX_SAND_IN  DNX_SAND_MULTI_SET_KEY      key -
*     The member to lookup
*   DNX_SAND_OUT  uint32                     *data_indx -
*     index identifying this member place.
*   DNX_SAND_OUT  uint32                     *ref_count -
*     the occurrences/duplications of this member in the multi-set
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  dnx_sand_multi_set_member_lookup(
    DNX_SAND_IN    int                     unit,
    DNX_SAND_IN    DNX_SAND_MULTI_SET_PTR  multi_set,
    DNX_SAND_IN    DNX_SAND_MULTI_SET_KEY  *const key,
    DNX_SAND_OUT   uint32                  *data_indx,
    DNX_SAND_OUT   uint32                  *ref_count
  )
{
  uint8
    tmp_cnt[sizeof(uint32)];
  uint8
    found ;
  uint32
    ref_count_lcl[1];
  uint32
    max_duplications,
    counter_size ;
  uint32
    multi_set_index,
    res ;
  DNX_SAND_HASH_TABLE_PTR
    hash_table ;

  DNX_SAND_INIT_ERROR_DEFINITIONS(DNX_SAND_MULTI_SET_MEMBER_LOOKUP) ;
  DNX_SAND_MULTI_SET_VERIFY_UNIT_IS_LEGAL(unit, 2) ;
  DNX_SAND_MULTI_SET_CONVERT_HANDLE_TO_MULTISET_INDEX(multi_set_index,multi_set) ;
  DNX_SAND_MULTI_SET_VERIFY_MULTISET_IS_ACTIVE(multi_set_index,4,6) ;

  DNX_SAND_CHECK_NULL_INPUT(key) ;
  DNX_SAND_CHECK_NULL_INPUT(data_indx) ;

  res = MULTI_SET_ACCESS_DATA.hash_table.get(unit, multi_set_index, &hash_table) ;
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 10, exit) ;
  res = dnx_sand_hash_table_entry_lookup(
          unit,
          hash_table,
          key,
          data_indx,
          &found
        );
  DNX_SAND_CHECK_FUNC_RESULT(res, 14, exit);
  if (!found)
  {
    *ref_count = 0;
    goto exit;
  }
  res = MULTI_SET_ACCESS_INFO.max_duplications.get(unit, multi_set_index, &max_duplications) ;
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 18, exit) ;
  if (max_duplications > 1)
  {
    *ref_count_lcl = 0;
    res = MULTI_SET_ACCESS_DATA.counter_size.get(unit, multi_set_index, &counter_size) ;
    DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 22, exit) ;
    /*
     * Copy 'counter_size' bytes from 'MULTI_SET_ACCESS_DATA.ref_counter' buffer at offset
     * '(*data_indx) * counter_size' into 'tmp_cnt' buffer.
     */
    res = MULTI_SET_ACCESS_DATA.ref_counter.memread(unit,multi_set_index,tmp_cnt,(*data_indx) * counter_size, counter_size) ;
    DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 26, exit) ;
    res = dnx_sand_U8_to_U32(
            tmp_cnt,
            counter_size,
            ref_count_lcl
          );
    DNX_SAND_CHECK_FUNC_RESULT(res, 30, exit);
    *ref_count = *ref_count_lcl ;
  }
  else
  {
    *ref_count = 1 ;
  }
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in dnx_sand_multi_set_member_lookup()",0,0);
}

/*********************************************************************
* NAME:
*     dnx_sand_multi_set_get_next
* TYPE:
*   PROC
* DATE:
*   June 01 2015
* FUNCTION:
*  get the next valid entry (key and data) in the multiset.
*  start traversing from the place pointed by the given iterator.
* INPUT:
*  DNX_SAND_IN  int                           unit -
*     Identifier of the device to access.
*  DNX_SAND_IN  DNX_SAND_MULTI_SET_PTR        multi_set -
*     The multiset.
*  DNX_SAND_INOUT  DNX_SAND_MULTI_SET_ITER    *iter
*     iterator points to the entry to start traverse from.
*   DNX_SAND_OUT  DNX_SAND_MULTI_SET_KEY*     const key -
*     the multiset key returned
*   DNX_SAND_OUT  DNX_SAND_MULTI_SET_DATA     data -
*     the multiset data returned and associated with the key above.
* REMARKS:
*     - to start traverse the multiset from the beginning.
*       use DNX_SAND_MULTI_SET_ITER_BEGIN(iter)
*     - to check if the iterator get to the end of the table use.
*       use DNX_SAND_MULTI_SET_ITER_END(iter)
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  dnx_sand_multi_set_get_next(
    DNX_SAND_IN    int                      unit,
    DNX_SAND_IN    DNX_SAND_MULTI_SET_PTR   multi_set,
    DNX_SAND_INOUT DNX_SAND_MULTI_SET_ITER  *iter,
    DNX_SAND_OUT   DNX_SAND_MULTI_SET_KEY   *key,
    DNX_SAND_OUT   uint32                   *data_indx,
    DNX_SAND_OUT   uint32                   *ref_count
  )
{
  uint32
    counter_size,
    max_duplications,
    tmp_ref_count;
  uint8
    tmp_cnt[sizeof(uint32)];
  uint32
    multi_set_index,
    res ;
  DNX_SAND_HASH_TABLE_PTR
    hash_table ;

  DNX_SAND_INIT_ERROR_DEFINITIONS(DNX_SAND_MULTI_SET_MEMBER_LOOKUP);
  DNX_SAND_MULTI_SET_VERIFY_UNIT_IS_LEGAL(unit, 2) ;
  DNX_SAND_MULTI_SET_CONVERT_HANDLE_TO_MULTISET_INDEX(multi_set_index,multi_set) ;
  DNX_SAND_MULTI_SET_VERIFY_MULTISET_IS_ACTIVE(multi_set_index,4,6) ;
  /*
   * traverse the mutli-set and print
   */
  if (DNX_SAND_HASH_TABLE_ITER_IS_END(iter))
  {
    goto exit ;
  }
  res = MULTI_SET_ACCESS_DATA.hash_table.get(unit, multi_set_index, &hash_table) ;
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 10, exit) ;
  res = dnx_sand_hash_table_get_next(
          unit,
          hash_table,
          iter,
          key,
          data_indx
        ) ;
  DNX_SAND_CHECK_FUNC_RESULT(res, 10, exit);
  if (DNX_SAND_HASH_TABLE_ITER_IS_END(iter))
  {
    goto exit ;
  }
  tmp_ref_count = 1;
  res = MULTI_SET_ACCESS_INFO.max_duplications.get(unit, multi_set_index, &max_duplications) ;
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 18, exit) ;
  if (max_duplications > 1)
  {
    tmp_ref_count = 0;
    res = MULTI_SET_ACCESS_DATA.counter_size.get(unit, multi_set_index, &counter_size) ;
    DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 22, exit) ;
    /*
     * Copy 'counter_size' bytes from 'MULTI_SET_ACCESS_DATA.ref_counter' buffer at offset
     * '(*data_indx) * counter_size' into 'tmp_cnt' buffer.
     */
    res = MULTI_SET_ACCESS_DATA.ref_counter.memread(unit,multi_set_index,tmp_cnt,(*data_indx) * counter_size, counter_size) ;
    DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 26, exit) ;
    res = dnx_sand_U8_to_U32(
           tmp_cnt,
           counter_size,
           &tmp_ref_count
        );
    DNX_SAND_CHECK_FUNC_RESULT(res, 30, exit);
  }
  *ref_count = tmp_ref_count;
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in dnx_sand_multi_set_get_next()",0,0);
}

/*********************************************************************
* NAME:
*     dnx_sand_multi_set_get_by_index
* TYPE:
*   PROC
* DATE:
*   June 01 2015
* FUNCTION:
*  get the next valid entry (key and data) in the multiset.
*  start traversing from the place pointed by the given iterator.
* INPUT:
*  DNX_SAND_IN  int                       unit -
*     Identifier of the device to access.
*  DNX_SAND_IN  DNX_SAND_MULTI_SET_PTR    multi_set -
*     The multiset.
*  DNX_SAND_IN  uint32                    data_indx -
*     where the key was found.
*   DNX_SAND_OUT  DNX_SAND_MULTI_SET_KEY* key -
*     the multiset key returned
*   DNX_SAND_OUT  DNX_SAND_MULTI_SET_DATA data -
*     the multiset data returned and associated with the key above.
* REMARKS:
*     - to start traverse the multiset from the beginning.
*       use DNX_SAND_MULTI_SET_ITER_BEGIN(iter)
*     - to check if the iterator get to the end of the table use.
*       use DNX_SAND_MULTI_SET_ITER_END(iter)
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  dnx_sand_multi_set_get_by_index(
    DNX_SAND_IN   int                      unit,
    DNX_SAND_IN   DNX_SAND_MULTI_SET_PTR   multi_set,
    DNX_SAND_IN   uint32                   data_indx,
    DNX_SAND_OUT  DNX_SAND_MULTI_SET_KEY   *key,
    DNX_SAND_OUT  uint32                   *ref_count
  )
{
  uint32
    counter_size,
    max_duplications,
    tmp_ref_count;
  uint8
    found;
  uint8
    tmp_cnt[sizeof(uint32)];
  uint32
    multi_set_index,
    res ;
  DNX_SAND_HASH_TABLE_PTR
    hash_table ;

  DNX_SAND_INIT_ERROR_DEFINITIONS(DNX_SAND_MULTI_SET_MEMBER_LOOKUP);
  DNX_SAND_MULTI_SET_VERIFY_UNIT_IS_LEGAL(unit, 2) ;
  DNX_SAND_MULTI_SET_CONVERT_HANDLE_TO_MULTISET_INDEX(multi_set_index,multi_set) ;
  DNX_SAND_MULTI_SET_VERIFY_MULTISET_IS_ACTIVE(multi_set_index,4,6) ;

  res = MULTI_SET_ACCESS_DATA.hash_table.get(unit, multi_set_index, &hash_table) ;
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 10, exit) ;
  res = dnx_sand_hash_table_get_by_index(
          unit,
          hash_table,
          data_indx,
          key,
          &found
        );
  DNX_SAND_CHECK_FUNC_RESULT(res, 14, exit);
  if (!found)
  {
    *ref_count = 0;
    goto exit;
  }
  tmp_ref_count = 1;
  res = MULTI_SET_ACCESS_INFO.max_duplications.get(unit, multi_set_index, &max_duplications) ;
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 18, exit) ;
  if (max_duplications > 1)
  {
    res = MULTI_SET_ACCESS_DATA.counter_size.get(unit, multi_set_index, &counter_size) ;
    DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 22, exit) ;
    tmp_ref_count = 0 ;
    /*
     * Copy 'counter_size' bytes from 'MULTI_SET_ACCESS_DATA.ref_counter' buffer at offset
     * 'data_indx * counter_size' into 'tmp_cnt' buffer.
     */
    res = MULTI_SET_ACCESS_DATA.ref_counter.memread(unit,multi_set_index,tmp_cnt,data_indx * counter_size, counter_size) ;
    DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 26, exit) ;
    res = dnx_sand_U8_to_U32(
           tmp_cnt,
           counter_size,
           &tmp_ref_count
        );
    DNX_SAND_CHECK_FUNC_RESULT(res, 30, exit);
  }
  *ref_count = tmp_ref_count;

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in dnx_sand_multi_set_get_next()",0,0);
}

uint32
  dnx_sand_multi_set_clear(
    DNX_SAND_IN  int                      unit,
    DNX_SAND_IN  DNX_SAND_MULTI_SET_PTR   multi_set
  )
{
  uint32
    nof_members,
    counter_size,
    max_duplications ;
  uint32
    multi_set_index,
    res ;
  DNX_SAND_HASH_TABLE_PTR
    hash_table ;

  DNX_SAND_INIT_ERROR_DEFINITIONS(DNX_SAND_HASH_TABLE_CREATE);
  DNX_SAND_MULTI_SET_VERIFY_UNIT_IS_LEGAL(unit, 2) ;
  DNX_SAND_MULTI_SET_CONVERT_HANDLE_TO_MULTISET_INDEX(multi_set_index,multi_set) ;
  DNX_SAND_MULTI_SET_VERIFY_MULTISET_IS_ACTIVE(multi_set_index,4,6) ;
  res = MULTI_SET_ACCESS_DATA.hash_table.get(unit, multi_set_index, &hash_table) ;
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 10, exit) ;
  /*
   * clear mutli-set.
   */
  res = dnx_sand_hash_table_clear(
            unit,
            hash_table
          ) ;
  DNX_SAND_CHECK_FUNC_RESULT(res, 14, exit) ;
  /*
   * if there is need to manage the duplications, then clear array to
   * manage the reference counter.
   */
  res = MULTI_SET_ACCESS_INFO.max_duplications.get(unit, multi_set_index, &max_duplications) ;
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 18, exit) ;
  if (max_duplications > 1)
  {
    /*
     * calculate the size of pointers (list head and next) according to table size.
     */
    counter_size = (dnx_sand_log2_round_up(max_duplications+1) + (DNX_SAND_NOF_BITS_IN_BYTE - 1)) / DNX_SAND_NOF_BITS_IN_BYTE;
    res = MULTI_SET_ACCESS_DATA.counter_size.set(unit, multi_set_index, counter_size) ;
    DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 22, exit) ;
    res = MULTI_SET_ACCESS_INFO.nof_members.get(unit, multi_set_index, &nof_members) ;
    DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 26, exit) ;
    /*
     * Fill 'ref_counter' buffer by zeroes
     */
    res = MULTI_SET_ACCESS_DATA.ref_counter.memset(unit,multi_set_index,0,nof_members * counter_size, 0x00) ;
    DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 26, exit) ;
  }

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in dnx_sand_multi_set_clear()",0,0);
}

#if (0)
/* { */
  /*
   * This is done within 'create'
   *
   * Remove this code when porting is done
   */
void
dnx_sand_SAND_MULTI_SET_INFO_clear(
  DNX_SAND_INOUT DNX_SAND_MULTI_SET_INIT_INFO *init_info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  dnx_sand_os_memset(info, 0x0, sizeof(*info));

  init_info->max_duplications = 1;
  init_info->get_entry_fun    = 0;
  init_info->set_entry_fun    = 0;
  init_info->nof_members      = 0;
  init_info->member_size      = 0;
  init_info->prime_handle     = 0;
  init_info->sec_handle       = 0;

  info->multiset_data.counter_size = 0;
  info->multiset_data.ref_counter  = 0;

  info->multiset_data.hash_table = dnx_sand_hash_table_get_illegal_hashtable_handle() ;

exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}
/* } */
#endif

#ifdef DNX_SAND_DEBUG
/* { */

/*********************************************************************
* NAME:
*     dnx_sand_multi_set_print
* TYPE:
*   PROC
* DATE:
*   June 01 2015
* FUNCTION:
*   Prints the mutli-set content, the members in the multi set,
*   the number of occurrences of   each member and the index.
* INPUT:
*  DNX_SAND_IN  DNX_SAND_MULTI_SET_PTR       multi_set -
*     The mutli-set to print.
*  DNX_SAND_IN  DNX_SAND_MULTI_SET_PRINT_VAL print_fun -
*     Call back to print the member.
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  dnx_sand_multi_set_print(
    DNX_SAND_IN  int                          unit,
    DNX_SAND_IN  DNX_SAND_MULTI_SET_PTR       multi_set,
    DNX_SAND_IN  DNX_SAND_MULTI_SET_PRINT_VAL print_fun,
    DNX_SAND_IN  uint8                        clear_on_print
  )
{
  uint32
    data_indx ;
  DNX_SAND_HASH_TABLE_ITER
    iter ;
  uint8
    key[100] ;
  uint32
    max_duplications,
    counter_size,
    ref_count,
    org_ref_count ;
  uint8
    tmp_cnt[sizeof(uint32)] ;
  uint32
    multi_set_index,
    res ;
  DNX_SAND_HASH_TABLE_PTR
    hash_table ;

  DNX_SAND_INIT_ERROR_DEFINITIONS(DNX_SAND_MULTI_SET_PRINT);
  DNX_SAND_MULTI_SET_VERIFY_UNIT_IS_LEGAL(unit, 2) ;
  DNX_SAND_MULTI_SET_CONVERT_HANDLE_TO_MULTISET_INDEX(multi_set_index,multi_set) ;
  DNX_SAND_MULTI_SET_VERIFY_MULTISET_IS_ACTIVE(multi_set_index,4,6) ;
  res = MULTI_SET_ACCESS_DATA.hash_table.get(unit, multi_set_index, &hash_table) ;
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 10, exit) ;
  /*
   * traverse the mutli-set and print
   */
  DNX_SAND_HASH_TABLE_ITER_SET_BEGIN(&iter);

  while (!DNX_SAND_HASH_TABLE_ITER_IS_END(&iter))
  {
    res = dnx_sand_hash_table_get_next(
                                       unit,
                                       hash_table,
                                       &iter,
                                       key,
                                       &data_indx
                                       );
    DNX_SAND_CHECK_FUNC_RESULT(res, 10, exit);
    if (DNX_SAND_HASH_TABLE_ITER_IS_END(&iter))
    {
      goto exit;
    }
    org_ref_count = 1;
    res = MULTI_SET_ACCESS_INFO.max_duplications.get(unit, multi_set_index, &max_duplications) ;
    DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 18, exit) ;
    if (max_duplications > 1)
    {
      ref_count = 0 ;
      res = MULTI_SET_ACCESS_DATA.counter_size.get(unit, multi_set_index, &counter_size) ;
      DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 22, exit) ;
      /*
       * Copy 'counter_size' bytes from 'MULTI_SET_ACCESS_DATA.ref_counter' buffer at offset
       * 'data_indx * counter_size' into 'tmp_cnt' buffer.
       */
      res = MULTI_SET_ACCESS_DATA.ref_counter.memread(unit,multi_set_index,tmp_cnt,data_indx * counter_size, counter_size) ;
      DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 26, exit) ;
      res = dnx_sand_U8_to_U32(
                               tmp_cnt,
                               counter_size,
                               &ref_count
                               );
      DNX_SAND_CHECK_FUNC_RESULT(res, 30, exit);

      org_ref_count = ref_count ;
      if (clear_on_print)
      {
        ref_count = 0 ;
        res = dnx_sand_U32_to_U8(
                                 &ref_count,
                                 counter_size,
                                 tmp_cnt
                                 );
        DNX_SAND_CHECK_FUNC_RESULT(res, 40, exit);
        /*
         * Copy 'counter_size' bytes from 'tmp_cnt' buffer into 'MULTI_SET_ACCESS_DATA.ref_counter'
         * buffer at offset 'data_indx * counter_size'.
         */
        res = MULTI_SET_ACCESS_DATA.ref_counter.memwrite(unit,multi_set_index,tmp_cnt,data_indx * counter_size, counter_size) ;
        DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 44, exit) ;
      }
    }
    if (org_ref_count)
    {
      LOG_CLI((BSL_META_U(unit,"| %-8u|"), data_indx));
      print_fun(key) ;
      LOG_CLI((BSL_META_U(unit,"| %-8u|\n\r"), org_ref_count));
    }
  }

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in dnx_sand_multi_set_print()",0,0);
}

uint32
  dnx_sand_multi_set_get_size_for_save(
    DNX_SAND_IN   int                         unit,
    DNX_SAND_IN   DNX_SAND_MULTI_SET_PTR      multi_set,
    DNX_SAND_OUT  uint32                      *size
  )
{
  uint32
    max_duplications,
    nof_members,
    counter_size,
    cur_size,
    total_size ;
  uint32
    multi_set_index,
    res ;
  DNX_SAND_HASH_TABLE_PTR
    hash_table ;

  DNX_SAND_INIT_ERROR_DEFINITIONS(0);
  DNX_SAND_MULTI_SET_VERIFY_UNIT_IS_LEGAL(unit, 2) ;
  DNX_SAND_MULTI_SET_CONVERT_HANDLE_TO_MULTISET_INDEX(multi_set_index,multi_set) ;
  DNX_SAND_MULTI_SET_VERIFY_MULTISET_IS_ACTIVE(multi_set_index,4,6) ;
  res = MULTI_SET_ACCESS_DATA.hash_table.get(unit, multi_set_index, &hash_table) ;
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 10, exit) ;

  total_size = 0 ;

  DNX_SAND_CHECK_NULL_INPUT(size);

  cur_size = sizeof(DNX_SAND_HASH_TABLE_INIT_INFO);
  total_size += cur_size;

  /* load hash table*/
  res = dnx_sand_hash_table_get_size_for_save(
            unit,
            hash_table,
            &cur_size
          );
  DNX_SAND_CHECK_FUNC_RESULT(res, 14, exit);
  total_size += cur_size;
  /*
   * If there is need to manage the duplications, then copy array to
   * manage the reference counter.
   */
  res = MULTI_SET_ACCESS_INFO.max_duplications.get(unit, multi_set_index, &max_duplications) ;
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 18, exit) ;
  if (max_duplications > 1)
  {
    res = MULTI_SET_ACCESS_DATA.counter_size.get(unit, multi_set_index, &counter_size) ;
    DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 22, exit) ;
    res = MULTI_SET_ACCESS_INFO.nof_members.get(unit, multi_set_index, &nof_members) ;
    DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 26, exit) ;
    total_size += (nof_members * counter_size) ;
  }
  *size= total_size ;
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in dnx_sand_multi_set_get_size_for_save()",0,0);
}

/*********************************************************************
* NAME:
*   dnx_sand_multi_set_save
* TYPE:
*   PROC
* DATE:
*   June 01 2015
* FUNCTION:
*   saves the given multiset in the given buffer
* INPUT:
*   DNX_SAND_IN  int                     unit -
*     Identifier of the device to access.
*   DNX_SAND_IN  DNX_SAND_MULTI_SET_PTR  multi_set -
*     The multiset to save.
*   DNX_SAND_OUT  uint8                  *buffer -
*     buffer to include the hast table
* REMARKS:
*   - the size of the buffer has to be at least as the value returned
*     by dnx_sand_multi_set_get_size_for_save.
*   - call back functions are not saved.
* RETURNS:
*   OK or ERROR indication.
*********************************************************************/
uint32
  dnx_sand_multi_set_save(
    DNX_SAND_IN  int                        unit,
    DNX_SAND_IN  DNX_SAND_MULTI_SET_PTR     multi_set,
    DNX_SAND_OUT uint8                      *buffer,
    DNX_SAND_IN  uint32                     buffer_size_bytes,
    DNX_SAND_OUT uint32                     *actual_size_bytes
  )
{
  uint8
    *cur_ptr ;
  uint32
    max_duplications,
    nof_members,
    counter_size,
    cur_size,
    total_size ;
  uint32
    multi_set_index,
    res ;
  DNX_SAND_HASH_TABLE_PTR
    hash_table ;

  DNX_SAND_INIT_ERROR_DEFINITIONS(0);
  DNX_SAND_MULTI_SET_VERIFY_UNIT_IS_LEGAL(unit, 2) ;
  DNX_SAND_MULTI_SET_CONVERT_HANDLE_TO_MULTISET_INDEX(multi_set_index,multi_set) ;
  DNX_SAND_MULTI_SET_VERIFY_MULTISET_IS_ACTIVE(multi_set_index,4,6) ;
  res = MULTI_SET_ACCESS_DATA.hash_table.get(unit, multi_set_index, &hash_table) ;
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 10, exit) ;

  DNX_SAND_CHECK_NULL_INPUT(actual_size_bytes) ;

  cur_ptr = (uint8*)buffer ;
  total_size = 0 ;

  /* copy init info */
  /*
   * Copy sizeof(DNX_SAND_MULTI_SET_INIT_INFO) bytes from 'MULTI_SET_ACCESS_INFO' buffer at offset
   * '0' into buffer pointed by 'cur_ptr'.
   */
  res = MULTI_SET_ACCESS_INFO.get(unit,multi_set_index,(DNX_SAND_MULTI_SET_INIT_INFO *)cur_ptr) ;
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 26, exit) ;
  cur_ptr += sizeof(DNX_SAND_MULTI_SET_INIT_INFO) ;

  /* copy DS data */
  res = dnx_sand_hash_table_save(
            unit,
            hash_table,
            cur_ptr,
            buffer_size_bytes - total_size,
            &cur_size
          );
  DNX_SAND_CHECK_FUNC_RESULT(res, 10, exit);
  cur_ptr += cur_size ;
  total_size += cur_size ;
  /*
   * if there is need to manage the duplications, then copy array to
   * manage the reference counter.
   */
  res = MULTI_SET_ACCESS_INFO.max_duplications.get(unit, multi_set_index, &max_duplications) ;
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 18, exit) ;
  if (max_duplications > 1)
  {
    res = MULTI_SET_ACCESS_DATA.counter_size.get(unit, multi_set_index, &counter_size) ;
    DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 22, exit) ;
    res = MULTI_SET_ACCESS_INFO.nof_members.get(unit, multi_set_index, &nof_members) ;
    DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 26, exit) ;
    /*
     * Copy 'nof_members * counter_size' bytes from 'MULTI_SET_ACCESS_DATA.ref_counter' buffer
     * at offset '0' into buffer pointed by 'cur_ptr'.
     */
    res = MULTI_SET_ACCESS_DATA.ref_counter.memread(unit,multi_set_index,cur_ptr, 0, nof_members * counter_size) ;
    DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 26, exit) ;
    cur_ptr += (nof_members * counter_size) ;
    total_size += (nof_members * counter_size) ;
  }

  *actual_size_bytes = total_size ;

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in dnx_sand_multi_set_save()",0,0);
}

uint32
  dnx_sand_multi_set_load(
    DNX_SAND_IN  int                                   unit,
    DNX_SAND_IN  uint8                                 **buffer,
    DNX_SAND_IN  DNX_SAND_MULTISET_SW_DB_ENTRY_SET     set_function,
    DNX_SAND_IN  DNX_SAND_MULTISET_SW_DB_ENTRY_GET     get_function,
    DNX_SAND_OUT DNX_SAND_MULTI_SET_PTR                *multi_set_ptr
  )
{
  DNX_SAND_IN uint8
    *cur_ptr ;
  uint32
    multi_set_index,
    res ;
  DNX_SAND_MULTI_SET_INIT_INFO
    init_info ;
  DNX_SAND_HASH_TABLE_PTR
    hash_table ;
  DNX_SAND_MULTI_SET_PTR
    multi_set ;
  uint32
    max_duplications,
    nof_members,
    counter_size ;

  DNX_SAND_INIT_ERROR_DEFINITIONS(0);
  DNX_SAND_MULTI_SET_VERIFY_UNIT_IS_LEGAL(unit, 2) ;

  DNX_SAND_CHECK_NULL_INPUT(buffer);

  cur_ptr = (DNX_SAND_IN uint8*)buffer[0] ;

  /* copy init info */
  dnx_sand_os_memcpy(&(init_info), cur_ptr,sizeof(DNX_SAND_MULTI_SET_INIT_INFO));
  cur_ptr += sizeof(DNX_SAND_MULTI_SET_INIT_INFO);
  init_info.set_entry_fun = set_function;
  init_info.get_entry_fun = get_function;

  /* create DS - will not work!! (petra only code)*/
  res = dnx_sand_multi_set_create(
          unit,
          multi_set_ptr,
          init_info
        );
  DNX_SAND_CHECK_FUNC_RESULT(res,20, exit);
  multi_set = *multi_set_ptr ;
  DNX_SAND_MULTI_SET_CONVERT_HANDLE_TO_MULTISET_INDEX(multi_set_index,multi_set) ;
  res = MULTI_SET_ACCESS_DATA.hash_table.get(unit, multi_set_index, &hash_table) ;
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 10, exit) ;
  res = dnx_sand_hash_table_destroy(
          unit,
          hash_table
        );
  DNX_SAND_CHECK_FUNC_RESULT(res,20, exit);

  /* load hash table*/
  res = dnx_sand_hash_table_load(
            unit,
            &cur_ptr,
            set_function,
            get_function,
            NULL,
            NULL, 
            &(hash_table)
          );
  DNX_SAND_CHECK_FUNC_RESULT(res, 10, exit);
  /*
   * if there is need to manage the duplications, then copy array to
   * manage the reference counter.
   */
  res = MULTI_SET_ACCESS_INFO.max_duplications.get(unit, multi_set_index, &max_duplications) ;
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 18, exit) ;
  if (max_duplications > 1)
  {
    res = MULTI_SET_ACCESS_DATA.counter_size.get(unit, multi_set_index, &counter_size) ;
    DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 22, exit) ;
    res = MULTI_SET_ACCESS_INFO.nof_members.get(unit, multi_set_index, &nof_members) ;
    DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 26, exit) ;
    /*
     * Copy 'nof_members * counter_size' bytes from buffer pointed by 'cur_ptr'
     * into 'MULTI_SET_ACCESS_DATA.ref_counter' buffer at offset '0'.
     */
    res = MULTI_SET_ACCESS_DATA.ref_counter.memwrite(unit,multi_set_index,cur_ptr, 0, nof_members * counter_size) ;
    DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 26, exit) ;
    cur_ptr += (nof_members * counter_size) ;
  }
  *buffer = cur_ptr ;
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in dnx_sand_multi_set_load()",0,0);
}

uint32
  dnx_sand_SAND_MULTI_SET_INFO_print(
    DNX_SAND_IN  int                   unit,
    DNX_SAND_IN DNX_SAND_MULTI_SET_PTR multi_set
  )
{
  uint32
    multi_set_index,
    res ;
  uint32
    max_duplications,
    nof_members,
    member_size,
    sec_handle,
    counter_size ;
  int
    prime_handle ;

  DNX_SAND_INIT_ERROR_DEFINITIONS(0);
  DNX_SAND_MULTI_SET_VERIFY_UNIT_IS_LEGAL(unit, 2) ;
  DNX_SAND_MULTI_SET_CONVERT_HANDLE_TO_MULTISET_INDEX(multi_set_index,multi_set) ;
  DNX_SAND_MULTI_SET_VERIFY_MULTISET_IS_ACTIVE(multi_set_index,4,6) ;

  res = MULTI_SET_ACCESS_INFO.max_duplications.get(unit, multi_set_index, &max_duplications) ;
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 8, exit) ;
  res = MULTI_SET_ACCESS_INFO.nof_members.get(unit, multi_set_index, &nof_members) ;
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 10, exit) ;
  res = MULTI_SET_ACCESS_INFO.member_size.get(unit, multi_set_index, &member_size) ;
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 12, exit) ;
  res = MULTI_SET_ACCESS_DATA.counter_size.get(unit, multi_set_index, &counter_size) ;
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 14, exit) ;
  res = MULTI_SET_ACCESS_INFO.prime_handle.get(unit, multi_set_index, &prime_handle) ;
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 16, exit) ;
  res = MULTI_SET_ACCESS_INFO.sec_handle.get(unit, multi_set_index, &sec_handle) ;
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 18, exit) ;

  LOG_CLI((BSL_META_U(unit,"init_info.max_duplications: %u\n"),max_duplications));
  LOG_CLI((BSL_META_U(unit,"init_info.nof_members: %u\n"),nof_members));
  LOG_CLI((BSL_META_U(unit,"init_info.member_size: %u\n"),member_size));
  LOG_CLI((BSL_META_U(unit,"multiset_data.counter_size: %u\n"),counter_size));
  LOG_CLI((BSL_META_U(unit,"init_info.prime_handle: %u\n"),prime_handle));
  LOG_CLI((BSL_META_U(unit,"init_info.sec_handle: %u\n"),sec_handle));

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in dnx_sand_SAND_MULTI_SET_INFO_print()", 0, 0);
}
/* } */
#endif /* DNX_SAND_DEBUG */

#include <soc/dnx/legacy/SAND/Utils/sand_footer.h>
