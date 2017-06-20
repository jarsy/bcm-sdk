/* $Id: sand_multi_set.h,v 1.6 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/
#ifndef DNX_SAND_MULTI_SET_H_INCLUDED
/* { */
#define DNX_SAND_MULTI_SET_H_INCLUDED

/*************
* INCLUDES  *
*************/
/* { */
#include <soc/dnx/legacy/SAND/Utils/sand_header.h>

#include <soc/dnx/legacy/SAND/Utils/sand_framework.h>
#include <soc/dnx/legacy/SAND/Management/sand_general_macros.h>
#include <soc/dnx/legacy/SAND/Utils/sand_hashtable.h>
/* } */

/*************
 * DEFINES   *
 *************/
/* { */
/*
 * Maximal number of multi set entries to assign at init.
 * See dnx_sand_multi_set_init(), dnx_sand_multi_set_create()
 */
#define DNX_SAND_MAX_NOF_MULTIS           (200 * SOC_DNX_DEFS_GET(unit, nof_cores))
#define DNX_SAND_MULTI_SET_NULL  DNX_SAND_U32_MAX
/* } */

/*************
 * MACROS    *
 *************/
/* { */

/* $Id: sand_multi_set.h,v 1.6 Broadcom SDK $
 * Reset the multiset iterator to point to the beginning of the multiset
 */
#define DNX_SAND_MULTI_SET_ITER_BEGIN(iter) (iter = 0)
/*
 * Check if the multiset iterator arrives to the end of the multiset
 */
#define DNX_SAND_MULTI_SET_ITER_END(iter)   (iter == DNX_SAND_U32_MAX)

/* } */

/*************
 * TYPE DEFS *
 *************/
/* { */

/*
 * type of the multiset key
 *
 */
typedef uint8 DNX_SAND_MULTI_SET_KEY ;
/*
 * type of the multiset data
 */
typedef uint8* DNX_SAND_MULTI_SET_DATA ;
/*
 * iterator over the multiset, use this type to traverse the multiset.
 */
typedef uint32 DNX_SAND_MULTI_SET_ITER ;
/*
 * This is an identifier of an element of type DNX_SAND_MULTI_SET_INFO
 *
 * Replace: typedef DNX_SAND_MULTI_SET_INFO*  DNX_SAND_MULTI_SET_PTR;
 * because the new software state does not use pointers, only handles.
 * So now, DNX_SAND_MULTI_SET_PTR is just a handle to the 'multi set'
 * structure (actually, index into 'multis_array' {of pointers})
 *
 * Note that the name is kept as is to minimize changes in current code.
 */
typedef uint32 DNX_SAND_MULTI_SET_PTR ;

/*********************************************************************
* NAME:
*     DNX_SAND_MULTISET_SW_DB_ENTRY_SET
* FUNCTION:
*  call back to set the entry information from the SW DB of the device.
* INPUT:
*  DNX_SAND_IN  int              prime_handle -
*   handle of the multiset to identify the multiset instance
*  DNX_SAND_IN  int              prime_handle -
*   secondary identifier to data to be set in the multiset instance.
*  DNX_SAND_IN  uint32                   offset -
*   offset of the entry in the memory "array".
*  DNX_SAND_IN  uint32                   len -
*   the length in bytes (uint8) of the entry to write.
*  DNX_SAND_IN uint8                     *data -
*   the information of to write.
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
typedef
  uint32
    (*DNX_SAND_MULTISET_SW_DB_ENTRY_SET)(
      DNX_SAND_IN  int                   prime_handle,
      DNX_SAND_IN  uint32                   sec_handle,
      DNX_SAND_INOUT  uint8                 *buffer,
      DNX_SAND_IN  uint32                   offset,
      DNX_SAND_IN  uint32                   len,
      DNX_SAND_IN uint8                     *data
    );

/*********************************************************************
* NAME:
*     DNX_SAND_MULTISET_SW_DB_ENTRY_GET
* FUNCTION:
*  call back to get the entry information from the SW DB of the device.
* INPUT:
*  DNX_SAND_IN  int              prime_handle -
*   handle of the multiset to identify the multiset instance
*  DNX_SAND_IN  int              prime_handle -
*   secondary identifier to data to be set in the multiset instance.
*  DNX_SAND_IN  uint32                   offset -
*   offset of the entry in the memory "array".
*  DNX_SAND_IN  uint32                   len -
*   the length in bytes (uint8) of the entry to write.
*  DNX_SAND_OUT uint8                     *data -
*   the information to read.
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
typedef
  uint32
    (*DNX_SAND_MULTISET_SW_DB_ENTRY_GET)(
      DNX_SAND_IN  int                   prime_handle,
      DNX_SAND_IN  uint32                   sec_handle,
      DNX_SAND_IN  uint8                    *buffer,
      DNX_SAND_IN  uint32                   offset,
      DNX_SAND_IN  uint32                   len,
      DNX_SAND_OUT uint8*                   data
    );

/*********************************************************************
* NAME:
*     DNX_SAND_MULTI_SET_PRINT_VAL
* FUNCTION:
*  use to print the given value, needed only in the print API
* INPUT:
*  DNX_SAND_IN  int              prime_handle -
*   handle of the multiset to identify the multiset instance
*  DNX_SAND_IN  int              prime_handle -
*   secondary identifier to data to be set in the multiset instance.
*  DNX_SAND_IN  uint32                   offset -
*   offset of the entry in the memory "array".
*  DNX_SAND_IN  uint32                   len -
*   the length in bytes (uint8) of the entry to write.
*  DNX_SAND_OUT uint8                     *data -
*   the information to read.
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
typedef
  void
    (*DNX_SAND_MULTI_SET_PRINT_VAL)(
      DNX_SAND_IN  uint8                    *buffer
    );

 /*
  * includes the information user has to supply in the multiset creation
  */
 typedef struct {
  /*
   * handle of the multiset to identify the multiset instance
   */
  int prime_handle ;
  /*
   * handle of the multiset to identify the multiset instance
   */
  uint32 sec_handle ;
  /*
   * Number of different elements can be inserted to the set.
   * the member can be any values in the range of member size.
   * not only 0,1,...,nof_members - 1
   */
  uint32 nof_members ;
  /*
   * size of the member (in bytes)
   */
  uint32 member_size ;
  /*
   * The maximum duplications/occurrences of a value in the multi_set.
   * if 1  then the multi_set act as Set i.e.
   * no matter how many time an element was added
   * one remove operation will remove it from the set.
   * otherwise the member is not removed till
   * all the add operation reverted with remove operation
   * i.e. # removes = # add. if a value added more than this
   * number the operation fail.
   */
  uint32 max_duplications ;
  /*
   * The maximum number of duplications/occurrences of all values in the multi_set. 
   * If none was given at startup, global_max = (max_duplications == 1) ?  nof_members  : max_duplications;
   */
  uint32 global_max ;
  DNX_SAND_MULTISET_SW_DB_ENTRY_GET get_entry_fun ;
  DNX_SAND_MULTISET_SW_DB_ENTRY_SET set_entry_fun ;
} DNX_SAND_MULTI_SET_INIT_INFO ;

typedef struct
{
  /*
   * Array to include reference counting of the used entries.
   * relevant and allocated only if no_duplication = FALSE
   */
  SW_STATE_BUFF           *ref_counter ;
  /*
   * The size of the counter in bytes.
   */
  uint32                  counter_size ;
  /*
   * Global counter of all refernces to all entries.
   */
  uint32                  global_counter;
  /*
   * Multiset to manage the mapping of the element to indexes
   */
  DNX_SAND_HASH_TABLE_PTR hash_table ;

} DNX_SAND_MULTI_SET_T ;

typedef struct
{
  DNX_SAND_MULTI_SET_INIT_INFO   init_info ;
  DNX_SAND_MULTI_SET_T           multiset_data ;
} DNX_SAND_MULTI_SET_INFO ;

/*
 * Control Structure for all created multi sets. Each multi set is pointed
 * by multis_array. See dnx_sand_multi_set_init()
 */
typedef struct dnx_sand_sw_state_multi_set_s
{
	uint32                      max_nof_multis ;
	uint32                      in_use ;
	PARSER_HINT_ARR_PTR DNX_SAND_MULTI_SET_INFO   **multis_array ;
	PARSER_HINT_ARR     SHR_BITDCL                *occupied_multis ;
} dnx_sand_sw_state_multi_set_t ;

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
  ) ;
/*********************************************************************
* NAME:
*     dnx_sand_multi_set_create
* TYPE:
*   PROC
* DATE:
*   June 01 2015
* FUNCTION:
*     Creates a new multiset instance.
* INPUT:
*   DNX_SAND_IN  int                            unit -
*     Identifier of the device to access.
*   DNX_SAND_INOUT  DNX_SAND_MULTI_SET_PTR      *multi_set_ptr -
*     information to use in order to create the multiset (size, member size...)
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
  );

/*********************************************************************
* NAME:
*     dnx_sand_multi_set_destroy
* TYPE:
*   PROC
* DATE:
*   June 01 2015
* FUNCTION:
*     free the multiset instance.
* INPUT:
*  DNX_SAND_IN   int                    unit -
*     Identifier of the device to access.
*  DNX_SAND_IN   DNX_SAND_MULTI_SET_PTR multi_set -
*     The multiset to destroy.
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  dnx_sand_multi_set_destroy(
    DNX_SAND_IN     int                     unit,
    DNX_SAND_INOUT  DNX_SAND_MULTI_SET_PTR  multi_set
  );

/*********************************************************************
* NAME:
*     dnx_sand_multi_set_member_add
* TYPE:
*   PROC
* DATE:
*   June 01 2015
* FUNCTION:
*  Add a member to the mutli-set, if already exist then
*  then update the number occurrences of this member.
* INPUT:
*   DNX_SAND_IN  int                          unit -
*     Identifier of the device to access.
*   DNX_SAND_IN     DNX_SAND_MULTI_SET_PTR    multi_set -
*     The multi-set to add a member to.
*   DNX_SAND_IN  DNX_SAND_MULTI_SET_KEY       key -
*     The member to add.
*   DNX_SAND_OUT  uint32                      *data_indx -
*     Index identifies the place of the added member.
*     the given key
*   DNX_SAND_OUT  uint8                       *first_appear -
*     whether this is the first occurrence of this key in the multiset
*   DNX_SAND_OUT  uint8                       *success -
*     whether the add operation success, this may be FALSE,
*     if there are no more space available in the multi-set
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  dnx_sand_multi_set_member_add(
    DNX_SAND_IN    int                     unit,
    DNX_SAND_IN    DNX_SAND_MULTI_SET_PTR  multi_set,
    DNX_SAND_IN    DNX_SAND_MULTI_SET_KEY  *const key,
    DNX_SAND_OUT   uint32                  *data_indx,
    DNX_SAND_OUT   uint8                   *first_appear,
    DNX_SAND_OUT   uint8                   *success
  );

/*********************************************************************
* NAME:
*     dnx_sand_multi_set_member_add_at_index
* TYPE:
*   PROC
* FUNCTION:
*  Add a member to the mutli-set at a specific index.
*  If the id does not exist (number of occurences is zero)
*  then it is allocated with the supplied key and a single
*  member is added (number of occurences = 1).
*  If it does exist, first the previous index for
*  the key is retrieved.
*  If the previous index is different than the supplied index
*  then an error is returned.
*  If the previous index is identical to the supplied index
*  then the number of occurences is incremented.
* INPUT:
*   DNX_SAND_IN  int                           unit -
*     Identifier of the device to access.
*   DNX_SAND_INOUT  DNX_SAND_MULTI_SET_PTR     multi_set -
*     The multi-set to add a member to.
*   DNX_SAND_IN  DNX_SAND_MULTI_SET_KEY        key -
*     The member to add.
*   DNX_SAND_OUT  uint32                       data_indx -
*     The index at which the member should be added.
*   DNX_SAND_OUT  uint8                       *first_appear -
*     whether this is the first occurrence of this key in the multiset
*   DNX_SAND_OUT  uint8                       *success -
*     whether the add operation success, this may be FALSE,
*     if there are no more space available in the multi-set
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  dnx_sand_multi_set_member_add_at_index(
    DNX_SAND_IN    int                        unit,
    DNX_SAND_IN    DNX_SAND_MULTI_SET_PTR     multi_set,
    DNX_SAND_IN    DNX_SAND_MULTI_SET_KEY     *const key,
    DNX_SAND_IN    uint32                     data_indx,
    DNX_SAND_OUT   uint8                      *first_appear,
    DNX_SAND_OUT   uint8                      *success
  );

/*********************************************************************
* NAME:
*     dnx_sand_multi_set_member_add_at_index_nof_additions
* TYPE:
*   PROC
* DATE:
*   June 01 2015
* FUNCTION:
*  Add nof members to the mutli-set to a specific index, if already exist then
*  then update the occurences of this member.
* INPUT:
*   DNX_SAND_IN  int                           unit -
*     Identifier of the device to access.
*   DNX_SAND_INOUT  DNX_SAND_MULTI_SET_PTR     multi_set -
*     The multi-set to add a member to.
*   DNX_SAND_IN  DNX_SAND_MULTI_SET_KEY        key -
*     The member to add.
*   DNX_SAND_OUT  uint32                      *data_indx -
*     Index identifies the place of the added member.
*     the given key
*   DNX_SAND_IN  uint32                        nof_additions -
*     Declare nof_additions to add the given key.   
*   DNX_SAND_OUT  uint8                       *success -
*     whether the add operation success, this may be FALSE,
*     if there are no more space available in the multi-set
* Remarks: 
*     Similair to  dnx_sand_multi_set_member_add_at_index,
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
  );

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
*  DNX_SAND_IN  int                          unit -
*     Identifier of the device to access.
*  DNX_SAND_IN     DNX_SAND_MULTI_SET_PTR    multi_set -
*     The mutli-set instance.
*  DNX_SAND_IN  DNX_SAND_MULTI_SET_KEY       *const key -
*     The key to remove from the mutli-set
*  DNX_SAND_OUT  uint32                      *data_indx -
*     where the key was found.
*  DNX_SAND_OUT  uint8                       *last_appear -
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
  );


/*********************************************************************
* NAME:
*     dnx_sand_multi_set_member_remove_by_index
* TYPE:
*   PROC
* DATE:
*   June 01 2015
* FUNCTION:
*  Remove a member from a mutli-set according the
*  identifying index.
* INPUT:
*  DNX_SAND_IN  int                         unit -
*    Identifier of the device to access.
*  DNX_SAND_IN     DNX_SAND_MULTI_SET_PTR   multi_set -
*    The mutli-set instance.
*  DNX_SAND_IN  uint32                      data_indx -
*    The index identifying the placement of the data
*    in the mult-set.
*  DNX_SAND_OUT  uint8                      *last_appear -
*     whether this was the last occurrence of this key in the multiset.
*     so no more occurrences of this key after this remove.
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  dnx_sand_multi_set_member_remove_by_index(
    DNX_SAND_IN    int                       unit,
    DNX_SAND_IN    DNX_SAND_MULTI_SET_PTR    multi_set,
    DNX_SAND_IN    uint32                    data_indx,
    DNX_SAND_OUT   uint8                     *last_appear
  );

/*********************************************************************
* NAME:
*     dnx_sand_multi_set_member_remove_multiple
* TYPE:
*   PROC
* FUNCTION:
*  Like remove single member by index but done multiple times (without using a loop).
* INPUT:
*  DNX_SAND_IN  int                        unit -
*     Identifier of the device to access.
*  DNX_SAND_IN     DNX_SAND_MULTI_SET_PTR  multi_set -
*     The mutli-set instance.
*  DNX_SAND_IN  uint32                     data_indx -
*    The index identifying the placement of the data
*    in the mult-set.
*  DNX_SAND_OUT  uint32                    remove_amt -
*    How many members should be removed
*  DNX_SAND_OUT  uint8                     *last_appear -
*     whether this was the last occurrence of this key in the multiset.
*     so no more occurrences of this key after this remove.
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  dnx_sand_multi_set_member_remove_by_index_multiple(
    DNX_SAND_IN    int                      unit,
    DNX_SAND_INOUT DNX_SAND_MULTI_SET_PTR   multi_set,
    DNX_SAND_IN    uint32                   data_indx,
    DNX_SAND_IN    uint32                   remove_amt,
    DNX_SAND_OUT   uint8                    *last_appear
  );

/*********************************************************************
* NAME:
*     dnx_sand_multi_set_member_lookup
* TYPE:
*   PROC
* DATE:
*   June 01 2015
* FUNCTION:
*  lookup in the mutli-set for a member and return the occurrences/duplications of this
*  member in the multi-set, and the index identifying this member place.
*  the given key.
* INPUT:
*  DNX_SAND_IN  int                        unit -
*     Identifier of the device to access.
*  DNX_SAND_IN  DNX_SAND_MULTI_SET_PTR     multi_set -
*     The mutli-set.
*   DNX_SAND_IN  DNX_SAND_MULTI_SET_KEY*   const key -
*     The key to lookup
*   DNX_SAND_OUT  uint32                  *data_indx -
*     index identifying this member place.
*   DNX_SAND_OUT  uint32                  *ref_count -
*     the occurrences/duplications of this member in the multi-set
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  dnx_sand_multi_set_member_lookup(
    DNX_SAND_IN    int                      unit,
    DNX_SAND_IN  DNX_SAND_MULTI_SET_PTR     multi_set,
    DNX_SAND_IN  DNX_SAND_MULTI_SET_KEY     *const key,
    DNX_SAND_OUT  uint32                    *data_indx,
    DNX_SAND_OUT  uint32                    *ref_count
  );

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
*   DNX_SAND_OUT  uint32                      *ref_count -
*     the occurrences/duplications of this member in the multi-set
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
  );

/*********************************************************************
* NAME:
*     dnx_sand_multi_set_get_by_index
* TYPE:
*   PROC
* DATE:
*   June 01 2015
* FUNCTION:
*   Get the number of occurences of an index (zero if does not exist).
* INPUT:
*  DNX_SAND_IN  int                       unit -
*     Identifier of the device to access.
*  DNX_SAND_IN  DNX_SAND_MULTI_SET_PTR    multi_set -
*     The multiset.
*  DNX_SAND_IN  uint32                    data_indx -
*     The index.
*  DNX_SAND_OUT  DNX_SAND_MULTI_SET_KEY   *key -
*     the multiset key returned
*  DNX_SAND_OUT  uint32                   *ref_count -
*     the occurrences/duplications of this member in the multi-set
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
    DNX_SAND_OUT  DNX_SAND_MULTI_SET_KEY  *key,
    DNX_SAND_OUT  uint32                  *ref_count
  );

/*********************************************************************
* NAME:
*     dnx_sand_multi_set_get_size_for_save
* TYPE:
*   PROC
* DATE:
*   June 01 2015
* FUNCTION:
*     returns the size of the buffer needed to return the multiset as buffer.
*     in order to be loaded later
* INPUT:
*   DNX_SAND_IN  int                    unit -
*     Identifier of the device to access.
*   DNX_SAND_IN  DNX_SAND_MULTI_SET_PTR multi_set -
*     The multiset to get the size for.
*   DNX_SAND_OUT  uint32               *size -
*     the size of the buffer.
* REMARKS:
*   None.
* RETURNS:
*   OK or ERROR indication.
*********************************************************************/
uint32
  dnx_sand_multi_set_get_size_for_save(
    DNX_SAND_IN   int                           unit,
    DNX_SAND_IN   DNX_SAND_MULTI_SET_PTR        multi_set,
    DNX_SAND_OUT  uint32                        *size
  );

/*********************************************************************
* NAME:
*     dnx_sand_multi_set_save
* TYPE:
*   PROC
* DATE:
*   June 01 2015
* FUNCTION:
*     saves the given multiset in the given buffer
* INPUT:
*   DNX_SAND_IN  int                      unit -
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
    DNX_SAND_IN  int                         unit,
    DNX_SAND_IN  DNX_SAND_MULTI_SET_PTR      multi_set,
    DNX_SAND_OUT uint8                       *buffer,
    DNX_SAND_IN  uint32                      buffer_size_bytes,
    DNX_SAND_OUT uint32                      *actual_size_bytes
  );

/*********************************************************************
* NAME:
*     dnx_sand_multi_set_load
* TYPE:
*   PROC
* DATE:
*   June 01 2015
* FUNCTION:
*     load from the given buffer the multiset saved in this buffer.
* INPUT:
*   DNX_SAND_IN  int                                   unit -
*     Identifier of the device to access.
*   DNX_SAND_IN  uint8                                *buffer -
*     buffer includes the hast table
*   DNX_SAND_IN  DNX_SAND_MULTISET_SW_DB_ENTRY_SET     set_function -
*     the hash function of the multiset.
*   DNX_SAND_IN  DNX_SAND_MULTISET_SW_DB_ENTRY_GET     get_function -
*     the hash function of the multiset.
*   DNX_SAND_OUT  DNX_SAND_MULTI_SET_PTR               *multi_set -
*     This procedure load pointed memory by the handle to the
*     newly created (and loaded) multiset.
* REMARKS:
*   - the size of the buffer has to be at least as the value returned
*     by dnx_sand_multi_set_get_size_for_save.
*   - there is need to supply the hash and rehash function (in case they are not
*     the default implementation, cause in the save they are not saved.
*     by dnx_sand_multi_set_get_size_for_save.
*   - this function will update buffer to point
*     to next place, for further loads.
* RETURNS:
*   OK or ERROR indication.
*********************************************************************/
uint32
  dnx_sand_multi_set_load(
    DNX_SAND_IN  int                                  unit,
    DNX_SAND_IN  uint8                                **buffer,
    DNX_SAND_IN  DNX_SAND_MULTISET_SW_DB_ENTRY_SET    set_function,
    DNX_SAND_IN  DNX_SAND_MULTISET_SW_DB_ENTRY_GET    get_function,
    DNX_SAND_OUT DNX_SAND_MULTI_SET_PTR               *multi_set
  );

/*********************************************************************
* NAME:
*     dnx_sand_multi_set_clear
* TYPE:
*   PROC
* DATE:
*   June 01 2015
* FUNCTION:
*     Clear the Multiset without freeing the memory
* INPUT:
*  DNX_SAND_IN  int                        unit -
*     Identifier of the device to access.
*  DNX_SAND_IN  DNX_SAND_MULTI_SET_PTR     multi_set -
*     The mutli-set to clear.
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  dnx_sand_multi_set_clear(
    DNX_SAND_IN     int                     unit,
    DNX_SAND_IN     DNX_SAND_MULTI_SET_PTR  multi_set
  );

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
*     prints the mutli-set content, the members in the multi set, the number of occurrences of
*     each member and the index.
* INPUT:
*  DNX_SAND_IN  int                          unit -
*     Identifier of the device to access.
*  DNX_SAND_IN  DNX_SAND_MULTI_SET_PTR       multi_set -
*     The mutli-set to print.
*  DNX_SAND_IN  DNX_SAND_MULTI_SET_PRINT_VAL print_fun -
*     call back to print the member.
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  dnx_sand_multi_set_print(
    DNX_SAND_IN  int                           unit,
    DNX_SAND_IN  DNX_SAND_MULTI_SET_PTR        multi_set,
    DNX_SAND_IN  DNX_SAND_MULTI_SET_PRINT_VAL  print_fun,
    DNX_SAND_IN  uint8                         clear_on_print
  );

/*********************************************************************
* NAME:
*     dnx_sand_multi_set_tests
* TYPE:
*   PROC
* DATE:
*   June 01 2015
* FUNCTION:
*     Tests the multiset module
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
*REMARKS:* SW License Agreement: Dune Networks (c). CONFIDENTIAL PROPRIETARY INFORMATION.
* Any use of this Software is subject to Software License Agreement
* included in the Driver User Manual of this device.
* Any use of this Software constitutes an agreement to the terms
* of the above Software License Agreement.
********************************************************************/
/*uint32
  dnx_sand_multi_set_tests(
    DNX_SAND_IN int unit,
    DNX_SAND_IN uint8 silent
);
*/

uint32
  dnx_sand_SAND_MULTI_SET_INFO_print(
    DNX_SAND_IN  int                   unit,
    DNX_SAND_IN DNX_SAND_MULTI_SET_PTR multi_set
  ) ;
/* } */
#endif /* DNX_SAND_DEBUG */

/* } */

#include <soc/dnx/legacy/SAND/Utils/sand_footer.h>

/* } DNX_SAND_MULTI_SET_H_INCLUDED*/
#endif

