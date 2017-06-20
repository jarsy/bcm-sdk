/* $Id: sand_group_member_list.h,v 1.5 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/


#ifndef __SOC_SAND_GROUP_MEMBER_LIST_INCLUDED__
/* { */
#define __SOC_SAND_GROUP_MEMBER_LIST_INCLUDED__

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

/* $Id: sand_group_member_list.h,v 1.5 Broadcom SDK $
 * indicates NULL pointer.
 */
#define SOC_SAND_GROUP_MEM_LL_END SOC_SAND_U32_MAX

/* } */

/*************
* MACROS    *
*************/
/* { */


/* } */

/*************
* TYPE DEFS *
*************/
/* { */

typedef  uint32 SOC_SAND_GROUP_MEM_LL_GROUP_ID;
typedef  uint32 SOC_SAND_GROUP_MEM_LL_MEMBER_ID;

/*
 * the ADT of group_member_list, use this type to manipulate the GROUP_MEMBER_LIST
 */
typedef uint32  SOC_SAND_GROUP_MEM_LL_ID;


typedef struct
{
  SOC_SAND_GROUP_MEM_LL_MEMBER_ID first_member;
}SOC_SAND_GROUP_MEM_LL_GROUP_ENTRY;


typedef struct
{
 /*
  *  the next member in the linked list.
  */
  SOC_SAND_GROUP_MEM_LL_MEMBER_ID next_member;
 /*
  *  the previous member in the linked list.
  */
  SOC_SAND_GROUP_MEM_LL_MEMBER_ID prev_member;
 /*
  *  the group id this member belongs to.
  */
  SOC_SAND_GROUP_MEM_LL_GROUP_ID group_id;

}SOC_SAND_GROUP_MEM_LL_MEMBER_ENTRY;



/*********************************************************************
* NAME:
*     SOC_SAND_GROUP_MEM_LL_ITER_FUN
* FUNCTION:
* Callback to traverse members of an SW DB object.
* INPUT:
*  SOC_SAND_IN      SOC_SAND_GROUP_MEM_LL_MEMBER_ID     member - 
*    The id of a member of the traversed object.
*  SOC_SAND_INOUT   uint32                              param1 -
*    Parameter to be passed untouched to the callback (opaque).
*  SOC_SAND_INOUT   uint32                              param2 -
*    Parameter to be passed untouched to the callback (opaque).
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
typedef
  uint32
    (*SOC_SAND_GROUP_MEM_LL_ITER_FUN)(
      SOC_SAND_IN     SOC_SAND_GROUP_MEM_LL_MEMBER_ID member,
      SOC_SAND_INOUT  uint32                    param1,
      SOC_SAND_INOUT  uint32                    param2
    );

/*********************************************************************
* NAME:
*     SOC_SAND_GROUP_MEM_LL_ITER_FUN
* FUNCTION:
* Callback to traverse members of an SW DB object.
* INPUT:
*  SOC_SAND_IN      SOC_SAND_GROUP_MEM_LL_MEMBER_ID     member - 
*    The id of a member of the traversed object.
*  SOC_SAND_INOUT   void                                *opaque -
*    Parameter to be passed untouched to the callback (opaque).
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
typedef
  uint32
    (*SOC_SAND_GROUP_MEM_LL_ITER_FUN_POINTER_PARAM)(
      SOC_SAND_IN     SOC_SAND_GROUP_MEM_LL_MEMBER_ID member,
      SOC_SAND_INOUT  void                            *opaque
    );


/*********************************************************************
* NAME:
*     SOC_SAND_GROUP_MEM_LL_MEMBER_ENTRY_SET
* FUNCTION:
*  call back to set information from the SW DB of the device.
*  set the information of member_ndx
* INPUT:
*  SOC_SAND_IN  uint32             member_ndx -
*   the place (id) of the member to set
*  SOC_SAND_IN  SOC_SAND_GROUP_MEM_LL_MEMBER_ENTRY       *member_entry -
*   the information to set for the member.
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
typedef
  uint32
    (*SOC_SAND_GROUP_MEM_LL_MEMBER_ENTRY_SET)(
      SOC_SAND_IN  int                             prime_handle,
      SOC_SAND_IN  uint32                             sec_handle,
      SOC_SAND_IN  uint32                             member_ndx,
      SOC_SAND_IN  SOC_SAND_GROUP_MEM_LL_MEMBER_ENTRY       *member_entry
    );


/*********************************************************************
* NAME:
*     SOC_SAND_GROUP_MEM_LL_MEMBER_ENTRY_GET
* FUNCTION:
*  call back to get the information from the SW DB of the device.
*  Get the information of member_ndx
* INPUT:
*  SOC_SAND_IN  uint32             member_ndx -
*   the place (id) of the member to set
*  SOC_SAND_IN  SOC_SAND_GROUP_MEM_LL_MEMBER_ENTRY       *member_entry -
*   the information entry of the member to get.
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
typedef
  uint32
    (*SOC_SAND_GROUP_MEM_LL_MEMBER_ENTRY_GET)(
      SOC_SAND_IN  int                             prime_handle,
      SOC_SAND_IN  uint32                             sec_handle,
      SOC_SAND_IN  uint32                             member_ndx,
      SOC_SAND_OUT  SOC_SAND_GROUP_MEM_LL_MEMBER_ENTRY      *member_entry
    );


/*********************************************************************
* NAME:
*     SOC_SAND_GROUP_MEM_LL_GROUP_ENTRY_SET
* FUNCTION:
*  call back to set information from the SW DB of the device.
*  set the information of group_ndx
* INPUT:
*  SOC_SAND_IN  uint32             group_ndx -
*   the place (id) of the group to set
*  SOC_SAND_IN  SOC_SAND_GROUP_MEM_LL_GROUP_ENTRY       *group_entry -
*   the information to set for the group.
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
typedef
  uint32
    (*SOC_SAND_GROUP_MEM_LL_GROUP_ENTRY_SET)(
      SOC_SAND_IN  int                             prime_handle,
      SOC_SAND_IN  uint32                             sec_handle,
      SOC_SAND_IN  uint32                             group_ndx,
      SOC_SAND_IN  SOC_SAND_GROUP_MEM_LL_GROUP_ENTRY       *group_entry
    );
/*********************************************************************
* NAME:
*     SOC_SAND_GROUP_MEM_LL_GROUP_ENTRY_GET
* FUNCTION:
*  call back to get the information from the SW DB of the device.
*  Get the information of group_ndx
* INPUT:
*  SOC_SAND_IN  uint32             group_ndx -
*   the place (id) of the group to set
*  SOC_SAND_IN  SOC_SAND_GROUP_MEM_LL_GROUP_ENTRY       *group_entry -
*   the information entry of the group to get.
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
typedef
  uint32
    (*SOC_SAND_GROUP_MEM_LL_GROUP_ENTRY_GET)(
      SOC_SAND_IN  int                             prime_handle,
      SOC_SAND_IN  uint32                             sec_handle,
      SOC_SAND_IN  uint32                             group_ndx,
      SOC_SAND_OUT  SOC_SAND_GROUP_MEM_LL_GROUP_ENTRY      *group_entry
    );

typedef struct
{
  /*
  * array to include groups
  */
  SOC_SAND_GROUP_MEM_LL_GROUP_ENTRY *groups;
  /*
  * array to include members
  */
  SOC_SAND_GROUP_MEM_LL_MEMBER_ENTRY *members;
 /*
  * cache management
  */
  uint8 cache_enabled;
 /*
  * array to include groups
  */
  SOC_SAND_GROUP_MEM_LL_GROUP_ENTRY *groups_cache;
  /*
  * array to include members
  */
  SOC_SAND_GROUP_MEM_LL_MEMBER_ENTRY *members_cache;

} SOC_SAND_GROUP_MEM_LL_T;

typedef SOC_SAND_GROUP_MEM_LL_T  *SOC_SAND_GROUP_MEM_LL_PTR;

/*
 * includes the information user has to supply in the group_member_list creation
 */
typedef struct 
{
  SOC_SAND_MAGIC_NUM_VAR
 /*
  * this flag determines the behavior of the group member instance.
  * if auto_remove is TRUE then when trying to add element to a group
  * when it already a member in another group then this member is removed
  * from the old group and added to the new group. if this FALSE then in this
  * case the add fails.
  */
  uint8 auto_remove;
 /*
  * number of groups
  */
  uint32 nof_groups;
 /*
  * primary handle identify the created instance
  */
  uint32 instance_prim_handle;
 /*
  * secondary handle identify the created instance
  */
  uint32 instance_sec_handle;
 /*
  * number of elements, these elements to be set as members
  * in the different groups.
  */
  uint32 nof_elements;
 /*
  * support-caching
  */
  uint8  support_caching;
 /*
  * call backs to set/get the data from the device SW database.
  */
  SOC_SAND_GROUP_MEM_LL_GROUP_ENTRY_SET group_set_fun;
  SOC_SAND_GROUP_MEM_LL_GROUP_ENTRY_GET group_get_fun;
  SOC_SAND_GROUP_MEM_LL_MEMBER_ENTRY_SET member_set_fun;
  SOC_SAND_GROUP_MEM_LL_MEMBER_ENTRY_GET member_get_fun;

 /*
  * saving warm boot variable index get/set of memory allocator variables.
  */
  int32 wb_var_index;

 /*
  * this to be manipulated by the soc_sand, caller shouldn't
  * modify these information, used to interconnect specific device memory
  * with the general algorithm in the soc_sand.
  */
  SOC_SAND_GROUP_MEM_LL_T group_members_data;

} SOC_SAND_GROUP_MEM_LL_INFO;

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
*     soc_sand_group_mem_ll_create
* TYPE:
*   PROC
* DATE:
*   Mar  6 2008
* FUNCTION:
*     Creates a new group_member_list instance.
* INPUT:
*   SOC_SAND_INOUT  SOC_SAND_GROUP_MEM_LL_INFO              *gr_mem_info -
*     information to use in order to create the group_member_list
* REMARKS:
*   None.
* RETURNS:
*   OK or ERROR indication.
*********************************************************************/
uint32
  soc_sand_group_mem_ll_create(
    SOC_SAND_INOUT   SOC_SAND_GROUP_MEM_LL_INFO      *gr_mem_info
  );


/*********************************************************************
* NAME:
*     soc_sand_group_mem_ll_destroy
* TYPE:
*   PROC
* DATE:
*   Mar  6 2008
* FUNCTION:
*     free the group_member_list instance.
* INPUT:
*  SOC_SAND_INOUT  SOC_SAND_GROUP_MEM_LL_INFO gr_mem_info -
*     The group_member_list to destroy.
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  soc_sand_group_mem_ll_destroy(
    SOC_SAND_INOUT   SOC_SAND_GROUP_MEM_LL_INFO      *gr_mem_info
  );

/*********************************************************************
* NAME:
*     soc_sand_group_mem_ll_clear
* TYPE:
*   PROC
* DATE:
*   Mar  6 2008
* FUNCTION:
*     Clear group_member_list instance without freeing the memory.
* INPUT:
*   SOC_SAND_IN  SOC_SAND_GROUP_MEM_LL_INFO              *gr_mem_info -
*     gr_mem instance
* REMARKS:
*   None.
* RETURNS:
*   OK or ERROR indication.
*********************************************************************/
uint32
  soc_sand_group_mem_ll_clear(
    SOC_SAND_INOUT   SOC_SAND_GROUP_MEM_LL_INFO      *gr_mem_info
  );

/*********************************************************************
* NAME:
*     soc_sand_group_mem_ll_cache_set
* TYPE:
*   PROC
* DATE:
*   Mar  6 2008
* FUNCTION:
*     enable/disable changes
* INPUT:
*   SOC_SAND_INOUT  SOC_SAND_GROUP_MEM_LL_INFO              *gr_mem_info -
*     information to use in order to create the arr_mem_alloc
* REMARKS:
*   None.
* RETURNS:
*   OK or ERROR indication.
*********************************************************************/
uint32
  soc_sand_group_mem_ll_cache_set(
    SOC_SAND_INOUT   SOC_SAND_GROUP_MEM_LL_INFO      *gr_mem_info,
    SOC_SAND_IN uint8                             enable
  );

/*********************************************************************
* NAME:
*     soc_sand_group_mem_ll_commit
* TYPE:
*   PROC
* DATE:
*   Mar  6 2008
* FUNCTION:
*     commit changes
* INPUT:
*   SOC_SAND_INOUT  SOC_SAND_GROUP_MEM_LL_INFO              *gr_mem_info -
*     information to use in order to create the arr_mem_alloc
* REMARKS:
*   None.
* RETURNS:
*   OK or ERROR indication.
*********************************************************************/
uint32
  soc_sand_group_mem_ll_commit(
    SOC_SAND_INOUT   SOC_SAND_GROUP_MEM_LL_INFO      *gr_mem_info,
    SOC_SAND_IN uint32 flags
  );

/*********************************************************************
* NAME:
*     soc_sand_group_mem_ll_rollback
* TYPE:
*   PROC
* DATE:
*   Mar  6 2008
* FUNCTION:
*     roll back changes
* INPUT:
*   SOC_SAND_INOUT  SOC_SAND_GROUP_MEM_LL_INFO              *gr_mem_info -
*     information to use in order to create the arr_mem_alloc
* REMARKS:
*   None.
* RETURNS:
*   OK or ERROR indication.
*********************************************************************/
uint32
  soc_sand_group_mem_ll_rollback(
    SOC_SAND_INOUT   SOC_SAND_GROUP_MEM_LL_INFO      *gr_mem_info,
    SOC_SAND_IN uint32 flags
  );

/*********************************************************************
* NAME:
*     soc_sand_group_mem_ll_members_set
* TYPE:
*   PROC
* DATE:
*   May  6 2008
* FUNCTION:
*  set the  membership of a group.
* INPUT:
*   SOC_SAND_INOUT   SOC_SAND_GROUP_MEM_LL_INFO     *gr_mem_info -
*     The group_member_list instance
*   SOC_SAND_IN  SOC_SAND_GROUP_MEM_LL_GROUP_ID     group_ndx -
*     the group id to set the membership for.
*   SOC_SAND_IN  SOC_SAND_GROUP_MEM_LL_MEMBER_ID    *members -
*     the membership
*   SOC_SAND_IN  uint32                       nof_members -
*     number of members
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  soc_sand_group_mem_ll_members_set(
    SOC_SAND_INOUT   SOC_SAND_GROUP_MEM_LL_INFO     *gr_mem_info,
    SOC_SAND_IN  SOC_SAND_GROUP_MEM_LL_GROUP_ID     group_ndx,
    SOC_SAND_IN  SOC_SAND_GROUP_MEM_LL_MEMBER_ID    *members,
    SOC_SAND_IN  uint32                       nof_members
  );


/*********************************************************************
* NAME:
*     soc_sand_group_mem_ll_member_add
* TYPE:
*   PROC
* DATE:
*   May  6 2008
* FUNCTION:
*  add a member to a group.
* INPUT:
*   SOC_SAND_INOUT   SOC_SAND_GROUP_MEM_LL_INFO     *gr_mem_info -
*     The group_member_list instance
*   SOC_SAND_IN  SOC_SAND_GROUP_MEM_LL_GROUP_ID     group_ndx -
*     the group id to add to.
*   SOC_SAND_IN  SOC_SAND_GROUP_MEM_LL_MEMBER_ID    member_id -
*     the member id to add.
* REMARKS:
*  - if the element is already member in another group then the behaviour
*    depends on the flag
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  soc_sand_group_mem_ll_member_add(
    SOC_SAND_INOUT   SOC_SAND_GROUP_MEM_LL_INFO     *gr_mem_info,
    SOC_SAND_IN  SOC_SAND_GROUP_MEM_LL_GROUP_ID     group_ndx,
    SOC_SAND_IN  SOC_SAND_GROUP_MEM_LL_MEMBER_ID    member_id
  );


/*********************************************************************
* NAME:
*     soc_sand_group_mem_ll_member_remove
* TYPE:
*   PROC
* DATE:
*   May  6 2008
* FUNCTION:
*  remove a member from a group.
* INPUT:
*   SOC_SAND_INOUT   SOC_SAND_GROUP_MEM_LL_INFO     *gr_mem_info -
*     The group_member_list instance
*   SOC_SAND_IN  SOC_SAND_GROUP_MEM_LL_MEMBER_ID    member_id -
*     the member id to remove.
* REMARKS:
*  - remove the member from the group.
*  - since the element can belong to one group there is no need to the group id.
*  - if the element not in any group then this operation has no effect.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  soc_sand_group_mem_ll_member_remove(
    SOC_SAND_INOUT   SOC_SAND_GROUP_MEM_LL_INFO     *gr_mem_info,
    SOC_SAND_IN  SOC_SAND_GROUP_MEM_LL_MEMBER_ID    member_id
  );
/*********************************************************************
* NAME:
*     soc_sand_group_mem_ll_members_get
* TYPE:
*   PROC
* DATE:
*   May  6 2008
* FUNCTION:
*  get "block" of the group membership.
* INPUT:
*   SOC_SAND_INOUT   SOC_SAND_GROUP_MEM_LL_INFO     *gr_mem_info -
*     The group_member_list instance
*   SOC_SAND_IN  SOC_SAND_GROUP_MEM_LL_GROUP_ID     group_ndx -
*     the group id to get the membership of.
*   SOC_SAND_IN  uint32                       max_members -
*     the maximum number of members to get.
*   SOC_SAND_INOUT   SOC_SAND_GROUP_MEM_LL_MEMBER_ID *iter -
*     iterator to to get the membership in iterations (blocks), in case of
*     big membership.
*     set iter = 0, to start from the first member in the group.
*     use iter == SOC_SAND_GROUP_MEM_LL_END to check if the iterator
*     reached the end of the membership.
*   SOC_SAND_OUT    SOC_SAND_GROUP_MEM_LL_MEMBER_ID   *members -
*     members returned
*   SOC_SAND_OUT  uint32                       *nof_members -
*     number of members returned
* REMARKS:
*  - members (array) should point to an allocated memory of size max_members
*    at least.
*  - to get the whole membership in iterations, call this function
*    as many time as needed till iter == SOC_SAND_GROUP_MEM_LL_END be TRUE.
*    when in first call iter = 0, and in the next call iter not touched.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  soc_sand_group_mem_ll_members_get(
    SOC_SAND_INOUT  SOC_SAND_GROUP_MEM_LL_INFO        *gr_mem_info,
    SOC_SAND_IN     SOC_SAND_GROUP_MEM_LL_GROUP_ID    group_ndx,
    SOC_SAND_IN     uint32                      max_members,
    SOC_SAND_INOUT  SOC_SAND_GROUP_MEM_LL_MEMBER_ID   *iter,
    SOC_SAND_OUT    SOC_SAND_GROUP_MEM_LL_MEMBER_ID   *members,
    SOC_SAND_OUT    uint32                      *nof_members
  );


/*********************************************************************
* NAME:
*     soc_sand_group_mem_ll_member_exist
* TYPE:
*   PROC
* DATE:
*   May  6 2008
* FUNCTION:
*  checks if element is a member in a group.
* INPUT:
*   SOC_SAND_INOUT   SOC_SAND_GROUP_MEM_LL_INFO     *gr_mem_info -
*     The group_member_list instance
*   SOC_SAND_IN  SOC_SAND_GROUP_MEM_LL_GROUP_ID     group_ndx -
*     the group id to check if the given member belongs to.
*   SOC_SAND_IN  SOC_SAND_GROUP_MEM_LL_MEMBER_ID    member_ndx -
*     the member id, element to check if it exist in the group membership.
*   SOC_SAND_OUT    uint8                     *exist -
*     the result, is the member (element) is a member in the group.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  soc_sand_group_mem_ll_member_exist(
    SOC_SAND_INOUT  SOC_SAND_GROUP_MEM_LL_INFO        *gr_mem_info,
    SOC_SAND_IN     SOC_SAND_GROUP_MEM_LL_GROUP_ID    group_ndx,
    SOC_SAND_IN     SOC_SAND_GROUP_MEM_LL_MEMBER_ID   member_ndx,
    SOC_SAND_OUT    uint8                     *exist
  );

/*********************************************************************
* NAME:
*     soc_sand_group_mem_ll_is_group_empty
* TYPE:
*   PROC
* DATE:
*   May  6 2008
* FUNCTION:
*  checks if a goup is empty (has no members)
* INPUT:
*   SOC_SAND_INOUT   SOC_SAND_GROUP_MEM_LL_INFO     *gr_mem_info -
*     The group_member_list instance
*   SOC_SAND_IN  SOC_SAND_GROUP_MEM_LL_GROUP_ID     group_ndx -
*     the group id 
*   SOC_SAND_OUT    uint8                     *exist -
*     the result, is the group empty.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  soc_sand_group_mem_ll_is_group_empty(
    SOC_SAND_INOUT  SOC_SAND_GROUP_MEM_LL_INFO        *gr_mem_info,
    SOC_SAND_IN     SOC_SAND_GROUP_MEM_LL_GROUP_ID    group_ndx,
    SOC_SAND_OUT    uint8                     *is_empty
  );
/*********************************************************************
* NAME:
*     soc_sand_group_mem_ll_member_get_group
* TYPE:
*   PROC
* DATE:
*   May  6 2008
* FUNCTION:
*  return the group the member belongs to if any.
* INPUT:
*   SOC_SAND_INOUT   SOC_SAND_GROUP_MEM_LL_INFO     *gr_mem_info -
*     The group_member_list instance
*   SOC_SAND_IN  SOC_SAND_GROUP_MEM_LL_MEMBER_ID    member_ndx -
*     the member id.
*   SOC_SAND_OUT  SOC_SAND_GROUP_MEM_LL_GROUP_ID     group_ndx -
*     the group id to given member belongs to.
*   SOC_SAND_OUT    uint8                     *valid -
*     is the element member in any group.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  soc_sand_group_mem_ll_member_get_group(
    SOC_SAND_INOUT  SOC_SAND_GROUP_MEM_LL_INFO        *gr_mem_info,
    SOC_SAND_IN     SOC_SAND_GROUP_MEM_LL_MEMBER_ID   member_ndx,
    SOC_SAND_OUT    SOC_SAND_GROUP_MEM_LL_GROUP_ID    *group_ndx,
    SOC_SAND_OUT    uint8                     *valid
  );


/*********************************************************************
* NAME:
*     soc_sand_group_mem_ll_group_clear
* TYPE:
*   PROC
* DATE:
*   May  6 2008
* FUNCTION:
*  clear all members in a group.
* INPUT:
*   SOC_SAND_INOUT   SOC_SAND_GROUP_MEM_LL_INFO     *gr_mem_info -
*     The group_member_list instance
*   SOC_SAND_IN  SOC_SAND_GROUP_MEM_LL_GROUP_ID     group_ndx -
*     the group id to print.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  soc_sand_group_mem_ll_group_clear(
    SOC_SAND_INOUT   SOC_SAND_GROUP_MEM_LL_INFO     *gr_mem_info,
    SOC_SAND_IN  SOC_SAND_GROUP_MEM_LL_GROUP_ID     group_ndx
  );

/*********************************************************************
* NAME:
*     soc_sand_group_mem_ll_member_remove
* TYPE:
*   PROC
* DATE:
*   May  6 2008
* FUNCTION:
*  print the members of a group.
* INPUT:
*   SOC_SAND_INOUT   SOC_SAND_GROUP_MEM_LL_INFO     *gr_mem_info -
*     The group_member_list instance
*   SOC_SAND_IN  SOC_SAND_GROUP_MEM_LL_GROUP_ID     group_ndx -
*     the group id to print.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  soc_sand_group_mem_ll_group_print(
    SOC_SAND_INOUT   SOC_SAND_GROUP_MEM_LL_INFO     *gr_mem_info,
    SOC_SAND_IN  SOC_SAND_GROUP_MEM_LL_GROUP_ID     group_ndx
  );

/*********************************************************************
* NAME:
*     soc_sand_group_mem_ll_func_run
* TYPE:
*   PROC
* DATE:
*   May  6 2008
* FUNCTION:members
*  run function over each of the group members
* INPUT:
*   SOC_SAND_INOUT   SOC_SAND_GROUP_MEM_LL_INFO     *gr_mem_info -
*     The group_member_list instance
*   SOC_SAND_IN  SOC_SAND_GROUP_MEM_LL_GROUP_ID     group_ndx -
*     the group id to print.
*   SOC_SAND_IN  SOC_SAND_GROUP_MEM_LL_ITER_FUN     func -
*     function to run.
*   SOC_SAND_INOUT  uint32                    param1-
*     parameter to be passed back to 'func'
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  soc_sand_group_mem_ll_func_run(
    SOC_SAND_INOUT   SOC_SAND_GROUP_MEM_LL_INFO     *gr_mem_info,
    SOC_SAND_IN  SOC_SAND_GROUP_MEM_LL_GROUP_ID     group_ndx,
    SOC_SAND_IN  SOC_SAND_GROUP_MEM_LL_ITER_FUN     func,
    SOC_SAND_INOUT  uint32                    param1,
    SOC_SAND_INOUT  uint32                    param2
  );

/*********************************************************************
* NAME:
*     soc_sand_group_mem_ll_func_run
* TYPE:
*   PROC
* DATE:
*   May  6 2008
* FUNCTION:members
*  run function over each of the group members
* INPUT:
*   SOC_SAND_INOUT   SOC_SAND_GROUP_MEM_LL_INFO     *gr_mem_info -
*     The group_member_list instance
*   SOC_SAND_IN  SOC_SAND_GROUP_MEM_LL_GROUP_ID     group_ndx -
*     the group id to print.
*   SOC_SAND_IN  SOC_SAND_GROUP_MEM_LL_ITER_FUN     func -
*     function to run.
*   SOC_SAND_INOUT  void                            *opaque -
*     parameter to be passed back to 'func' untouched.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  soc_sand_group_mem_ll_func_run_pointer_param(
    SOC_SAND_INOUT    SOC_SAND_GROUP_MEM_LL_INFO                    *gr_mem_info,
    SOC_SAND_IN       SOC_SAND_GROUP_MEM_LL_GROUP_ID                group_ndx,
    SOC_SAND_IN       SOC_SAND_GROUP_MEM_LL_ITER_FUN_POINTER_PARAM  func,
    SOC_SAND_INOUT    void                                          *opaque
  );

void
  soc_sand_SAND_GROUP_MEM_LL_INFO_clear(
    SOC_SAND_GROUP_MEM_LL_INFO *info
  );

uint32
  soc_sand_group_mem_ll_get_size_for_save(
    SOC_SAND_IN  SOC_SAND_GROUP_MEM_LL_INFO      *gr_mem_info,
    SOC_SAND_OUT  uint32                   *size
  );

uint32
  soc_sand_group_mem_ll_save(
    SOC_SAND_IN  SOC_SAND_GROUP_MEM_LL_INFO     *info,
    SOC_SAND_OUT uint8                    *buffer,
    SOC_SAND_IN  uint32                   buffer_size_bytes,
    SOC_SAND_OUT uint32                   *actual_size_bytes
  );

uint32
  soc_sand_group_mem_ll_load(
    SOC_SAND_IN  uint8                            **buffer,
    SOC_SAND_IN  SOC_SAND_GROUP_MEM_LL_GROUP_ENTRY_SET  group_set_fun,
    SOC_SAND_IN  SOC_SAND_GROUP_MEM_LL_GROUP_ENTRY_GET  group_get_fun,
    SOC_SAND_IN  SOC_SAND_GROUP_MEM_LL_MEMBER_ENTRY_SET member_set_fun,
    SOC_SAND_IN  SOC_SAND_GROUP_MEM_LL_MEMBER_ENTRY_GET member_get_fun,
    SOC_SAND_OUT SOC_SAND_GROUP_MEM_LL_INFO             *info
  );

/* } */



#include <soc/dpp/SAND/Utils/sand_footer.h>

/* } __SOC_SAND_GROUP_MEMBER_LIST_INCLUDED__*/
#endif
