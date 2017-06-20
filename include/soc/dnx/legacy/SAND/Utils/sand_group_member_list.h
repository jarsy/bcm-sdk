/* $Id: sand_group_member_list.h,v 1.5 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/


#ifndef __DNX_SAND_GROUP_MEMBER_LIST_INCLUDED__
/* { */
#define __DNX_SAND_GROUP_MEMBER_LIST_INCLUDED__

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

/* $Id: sand_group_member_list.h,v 1.5 Broadcom SDK $
 * indicates NULL pointer.
 */
#define DNX_SAND_GROUP_MEM_LL_END DNX_SAND_U32_MAX

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

typedef  uint32 DNX_SAND_GROUP_MEM_LL_GROUP_ID;
typedef  uint32 DNX_SAND_GROUP_MEM_LL_MEMBER_ID;

/*
 * the ADT of group_member_list, use this type to manipulate the GROUP_MEMBER_LIST
 */
typedef uint32  DNX_SAND_GROUP_MEM_LL_ID;


typedef struct
{
  DNX_SAND_GROUP_MEM_LL_MEMBER_ID first_member;
}DNX_SAND_GROUP_MEM_LL_GROUP_ENTRY;


typedef struct
{
 /*
  *  the next member in the linked list.
  */
  DNX_SAND_GROUP_MEM_LL_MEMBER_ID next_member;
 /*
  *  the previous member in the linked list.
  */
  DNX_SAND_GROUP_MEM_LL_MEMBER_ID prev_member;
 /*
  *  the group id this member belongs to.
  */
  DNX_SAND_GROUP_MEM_LL_GROUP_ID group_id;

}DNX_SAND_GROUP_MEM_LL_MEMBER_ENTRY;



/*********************************************************************
* NAME:
*     DNX_SAND_GROUP_MEM_LL_ITER_FUN
* FUNCTION:
* Callback to traverse members of an SW DB object.
* INPUT:
*  DNX_SAND_IN      DNX_SAND_GROUP_MEM_LL_MEMBER_ID     member - 
*    The id of a member of the traversed object.
*  DNX_SAND_INOUT   uint32                              param1 -
*    Parameter to be passed untouched to the callback (opaque).
*  DNX_SAND_INOUT   uint32                              param2 -
*    Parameter to be passed untouched to the callback (opaque).
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
typedef
  uint32
    (*DNX_SAND_GROUP_MEM_LL_ITER_FUN)(
      DNX_SAND_IN     DNX_SAND_GROUP_MEM_LL_MEMBER_ID member,
      DNX_SAND_INOUT  uint32                    param1,
      DNX_SAND_INOUT  uint32                    param2
    );

/*********************************************************************
* NAME:
*     DNX_SAND_GROUP_MEM_LL_ITER_FUN
* FUNCTION:
* Callback to traverse members of an SW DB object.
* INPUT:
*  DNX_SAND_IN      DNX_SAND_GROUP_MEM_LL_MEMBER_ID     member - 
*    The id of a member of the traversed object.
*  DNX_SAND_INOUT   void                                *opaque -
*    Parameter to be passed untouched to the callback (opaque).
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
typedef
  uint32
    (*DNX_SAND_GROUP_MEM_LL_ITER_FUN_POINTER_PARAM)(
      DNX_SAND_IN     DNX_SAND_GROUP_MEM_LL_MEMBER_ID member,
      DNX_SAND_INOUT  void                            *opaque
    );


/*********************************************************************
* NAME:
*     DNX_SAND_GROUP_MEM_LL_MEMBER_ENTRY_SET
* FUNCTION:
*  call back to set information from the SW DB of the device.
*  set the information of member_ndx
* INPUT:
*  DNX_SAND_IN  uint32             member_ndx -
*   the place (id) of the member to set
*  DNX_SAND_IN  DNX_SAND_GROUP_MEM_LL_MEMBER_ENTRY       *member_entry -
*   the information to set for the member.
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
typedef
  uint32
    (*DNX_SAND_GROUP_MEM_LL_MEMBER_ENTRY_SET)(
      DNX_SAND_IN  int                             prime_handle,
      DNX_SAND_IN  uint32                             sec_handle,
      DNX_SAND_IN  uint32                             member_ndx,
      DNX_SAND_IN  DNX_SAND_GROUP_MEM_LL_MEMBER_ENTRY       *member_entry
    );


/*********************************************************************
* NAME:
*     DNX_SAND_GROUP_MEM_LL_MEMBER_ENTRY_GET
* FUNCTION:
*  call back to get the information from the SW DB of the device.
*  Get the information of member_ndx
* INPUT:
*  DNX_SAND_IN  uint32             member_ndx -
*   the place (id) of the member to set
*  DNX_SAND_IN  DNX_SAND_GROUP_MEM_LL_MEMBER_ENTRY       *member_entry -
*   the information entry of the member to get.
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
typedef
  uint32
    (*DNX_SAND_GROUP_MEM_LL_MEMBER_ENTRY_GET)(
      DNX_SAND_IN  int                             prime_handle,
      DNX_SAND_IN  uint32                             sec_handle,
      DNX_SAND_IN  uint32                             member_ndx,
      DNX_SAND_OUT  DNX_SAND_GROUP_MEM_LL_MEMBER_ENTRY      *member_entry
    );


/*********************************************************************
* NAME:
*     DNX_SAND_GROUP_MEM_LL_GROUP_ENTRY_SET
* FUNCTION:
*  call back to set information from the SW DB of the device.
*  set the information of group_ndx
* INPUT:
*  DNX_SAND_IN  uint32             group_ndx -
*   the place (id) of the group to set
*  DNX_SAND_IN  DNX_SAND_GROUP_MEM_LL_GROUP_ENTRY       *group_entry -
*   the information to set for the group.
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
typedef
  uint32
    (*DNX_SAND_GROUP_MEM_LL_GROUP_ENTRY_SET)(
      DNX_SAND_IN  int                             prime_handle,
      DNX_SAND_IN  uint32                             sec_handle,
      DNX_SAND_IN  uint32                             group_ndx,
      DNX_SAND_IN  DNX_SAND_GROUP_MEM_LL_GROUP_ENTRY       *group_entry
    );
/*********************************************************************
* NAME:
*     DNX_SAND_GROUP_MEM_LL_GROUP_ENTRY_GET
* FUNCTION:
*  call back to get the information from the SW DB of the device.
*  Get the information of group_ndx
* INPUT:
*  DNX_SAND_IN  uint32             group_ndx -
*   the place (id) of the group to set
*  DNX_SAND_IN  DNX_SAND_GROUP_MEM_LL_GROUP_ENTRY       *group_entry -
*   the information entry of the group to get.
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
typedef
  uint32
    (*DNX_SAND_GROUP_MEM_LL_GROUP_ENTRY_GET)(
      DNX_SAND_IN  int                             prime_handle,
      DNX_SAND_IN  uint32                             sec_handle,
      DNX_SAND_IN  uint32                             group_ndx,
      DNX_SAND_OUT  DNX_SAND_GROUP_MEM_LL_GROUP_ENTRY      *group_entry
    );

typedef struct
{
  /*
  * array to include groups
  */
  DNX_SAND_GROUP_MEM_LL_GROUP_ENTRY *groups;
  /*
  * array to include members
  */
  DNX_SAND_GROUP_MEM_LL_MEMBER_ENTRY *members;
 /*
  * cache management
  */
  uint8 cache_enabled;
 /*
  * array to include groups
  */
  DNX_SAND_GROUP_MEM_LL_GROUP_ENTRY *groups_cache;
  /*
  * array to include members
  */
  DNX_SAND_GROUP_MEM_LL_MEMBER_ENTRY *members_cache;

} DNX_SAND_GROUP_MEM_LL_T;

typedef DNX_SAND_GROUP_MEM_LL_T  *DNX_SAND_GROUP_MEM_LL_PTR;

/*
 * includes the information user has to supply in the group_member_list creation
 */
typedef struct 
{
  DNX_SAND_MAGIC_NUM_VAR
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
  DNX_SAND_GROUP_MEM_LL_GROUP_ENTRY_SET group_set_fun;
  DNX_SAND_GROUP_MEM_LL_GROUP_ENTRY_GET group_get_fun;
  DNX_SAND_GROUP_MEM_LL_MEMBER_ENTRY_SET member_set_fun;
  DNX_SAND_GROUP_MEM_LL_MEMBER_ENTRY_GET member_get_fun;

 /*
  * saving warm boot variable index get/set of memory allocator variables.
  */
  int32 wb_var_index;

 /*
  * this to be manipulated by the dnx_sand, caller shouldn't
  * modify these information, used to interconnect specific device memory
  * with the general algorithm in the dnx_sand.
  */
  DNX_SAND_GROUP_MEM_LL_T group_members_data;

} DNX_SAND_GROUP_MEM_LL_INFO;

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
*     dnx_sand_group_mem_ll_create
* TYPE:
*   PROC
* DATE:
*   Mar  6 2008
* FUNCTION:
*     Creates a new group_member_list instance.
* INPUT:
*   DNX_SAND_INOUT  DNX_SAND_GROUP_MEM_LL_INFO              *gr_mem_info -
*     information to use in order to create the group_member_list
* REMARKS:
*   None.
* RETURNS:
*   OK or ERROR indication.
*********************************************************************/
uint32
  dnx_sand_group_mem_ll_create(
    DNX_SAND_INOUT   DNX_SAND_GROUP_MEM_LL_INFO      *gr_mem_info
  );


/*********************************************************************
* NAME:
*     dnx_sand_group_mem_ll_destroy
* TYPE:
*   PROC
* DATE:
*   Mar  6 2008
* FUNCTION:
*     free the group_member_list instance.
* INPUT:
*  DNX_SAND_INOUT  DNX_SAND_GROUP_MEM_LL_INFO gr_mem_info -
*     The group_member_list to destroy.
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  dnx_sand_group_mem_ll_destroy(
    DNX_SAND_INOUT   DNX_SAND_GROUP_MEM_LL_INFO      *gr_mem_info
  );

/*********************************************************************
* NAME:
*     dnx_sand_group_mem_ll_clear
* TYPE:
*   PROC
* DATE:
*   Mar  6 2008
* FUNCTION:
*     Clear group_member_list instance without freeing the memory.
* INPUT:
*   DNX_SAND_IN  DNX_SAND_GROUP_MEM_LL_INFO              *gr_mem_info -
*     gr_mem instance
* REMARKS:
*   None.
* RETURNS:
*   OK or ERROR indication.
*********************************************************************/
uint32
  dnx_sand_group_mem_ll_clear(
    DNX_SAND_INOUT   DNX_SAND_GROUP_MEM_LL_INFO      *gr_mem_info
  );

/*********************************************************************
* NAME:
*     dnx_sand_group_mem_ll_cache_set
* TYPE:
*   PROC
* DATE:
*   Mar  6 2008
* FUNCTION:
*     enable/disable changes
* INPUT:
*   DNX_SAND_INOUT  DNX_SAND_GROUP_MEM_LL_INFO              *gr_mem_info -
*     information to use in order to create the arr_mem_alloc
* REMARKS:
*   None.
* RETURNS:
*   OK or ERROR indication.
*********************************************************************/
uint32
  dnx_sand_group_mem_ll_cache_set(
    DNX_SAND_INOUT   DNX_SAND_GROUP_MEM_LL_INFO      *gr_mem_info,
    DNX_SAND_IN uint8                             enable
  );

/*********************************************************************
* NAME:
*     dnx_sand_group_mem_ll_commit
* TYPE:
*   PROC
* DATE:
*   Mar  6 2008
* FUNCTION:
*     commit changes
* INPUT:
*   DNX_SAND_INOUT  DNX_SAND_GROUP_MEM_LL_INFO              *gr_mem_info -
*     information to use in order to create the arr_mem_alloc
* REMARKS:
*   None.
* RETURNS:
*   OK or ERROR indication.
*********************************************************************/
uint32
  dnx_sand_group_mem_ll_commit(
    DNX_SAND_INOUT   DNX_SAND_GROUP_MEM_LL_INFO      *gr_mem_info,
    DNX_SAND_IN uint32 flags
  );

/*********************************************************************
* NAME:
*     dnx_sand_group_mem_ll_rollback
* TYPE:
*   PROC
* DATE:
*   Mar  6 2008
* FUNCTION:
*     roll back changes
* INPUT:
*   DNX_SAND_INOUT  DNX_SAND_GROUP_MEM_LL_INFO              *gr_mem_info -
*     information to use in order to create the arr_mem_alloc
* REMARKS:
*   None.
* RETURNS:
*   OK or ERROR indication.
*********************************************************************/
uint32
  dnx_sand_group_mem_ll_rollback(
    DNX_SAND_INOUT   DNX_SAND_GROUP_MEM_LL_INFO      *gr_mem_info,
    DNX_SAND_IN uint32 flags
  );

/*********************************************************************
* NAME:
*     dnx_sand_group_mem_ll_members_set
* TYPE:
*   PROC
* DATE:
*   May  6 2008
* FUNCTION:
*  set the  membership of a group.
* INPUT:
*   DNX_SAND_INOUT   DNX_SAND_GROUP_MEM_LL_INFO     *gr_mem_info -
*     The group_member_list instance
*   DNX_SAND_IN  DNX_SAND_GROUP_MEM_LL_GROUP_ID     group_ndx -
*     the group id to set the membership for.
*   DNX_SAND_IN  DNX_SAND_GROUP_MEM_LL_MEMBER_ID    *members -
*     the membership
*   DNX_SAND_IN  uint32                       nof_members -
*     number of members
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  dnx_sand_group_mem_ll_members_set(
    DNX_SAND_INOUT   DNX_SAND_GROUP_MEM_LL_INFO     *gr_mem_info,
    DNX_SAND_IN  DNX_SAND_GROUP_MEM_LL_GROUP_ID     group_ndx,
    DNX_SAND_IN  DNX_SAND_GROUP_MEM_LL_MEMBER_ID    *members,
    DNX_SAND_IN  uint32                       nof_members
  );


/*********************************************************************
* NAME:
*     dnx_sand_group_mem_ll_member_add
* TYPE:
*   PROC
* DATE:
*   May  6 2008
* FUNCTION:
*  add a member to a group.
* INPUT:
*   DNX_SAND_INOUT   DNX_SAND_GROUP_MEM_LL_INFO     *gr_mem_info -
*     The group_member_list instance
*   DNX_SAND_IN  DNX_SAND_GROUP_MEM_LL_GROUP_ID     group_ndx -
*     the group id to add to.
*   DNX_SAND_IN  DNX_SAND_GROUP_MEM_LL_MEMBER_ID    member_id -
*     the member id to add.
* REMARKS:
*  - if the element is already member in another group then the behaviour
*    depends on the flag
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  dnx_sand_group_mem_ll_member_add(
    DNX_SAND_INOUT   DNX_SAND_GROUP_MEM_LL_INFO     *gr_mem_info,
    DNX_SAND_IN  DNX_SAND_GROUP_MEM_LL_GROUP_ID     group_ndx,
    DNX_SAND_IN  DNX_SAND_GROUP_MEM_LL_MEMBER_ID    member_id
  );


/*********************************************************************
* NAME:
*     dnx_sand_group_mem_ll_member_remove
* TYPE:
*   PROC
* DATE:
*   May  6 2008
* FUNCTION:
*  remove a member from a group.
* INPUT:
*   DNX_SAND_INOUT   DNX_SAND_GROUP_MEM_LL_INFO     *gr_mem_info -
*     The group_member_list instance
*   DNX_SAND_IN  DNX_SAND_GROUP_MEM_LL_MEMBER_ID    member_id -
*     the member id to remove.
* REMARKS:
*  - remove the member from the group.
*  - since the element can belong to one group there is no need to the group id.
*  - if the element not in any group then this operation has no effect.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  dnx_sand_group_mem_ll_member_remove(
    DNX_SAND_INOUT   DNX_SAND_GROUP_MEM_LL_INFO     *gr_mem_info,
    DNX_SAND_IN  DNX_SAND_GROUP_MEM_LL_MEMBER_ID    member_id
  );
/*********************************************************************
* NAME:
*     dnx_sand_group_mem_ll_members_get
* TYPE:
*   PROC
* DATE:
*   May  6 2008
* FUNCTION:
*  get "block" of the group membership.
* INPUT:
*   DNX_SAND_INOUT   DNX_SAND_GROUP_MEM_LL_INFO     *gr_mem_info -
*     The group_member_list instance
*   DNX_SAND_IN  DNX_SAND_GROUP_MEM_LL_GROUP_ID     group_ndx -
*     the group id to get the membership of.
*   DNX_SAND_IN  uint32                       max_members -
*     the maximum number of members to get.
*   DNX_SAND_INOUT   DNX_SAND_GROUP_MEM_LL_MEMBER_ID *iter -
*     iterator to to get the membership in iterations (blocks), in case of
*     big membership.
*     set iter = 0, to start from the first member in the group.
*     use iter == DNX_SAND_GROUP_MEM_LL_END to check if the iterator
*     reached the end of the membership.
*   DNX_SAND_OUT    DNX_SAND_GROUP_MEM_LL_MEMBER_ID   *members -
*     members returned
*   DNX_SAND_OUT  uint32                       *nof_members -
*     number of members returned
* REMARKS:
*  - members (array) should point to an allocated memory of size max_members
*    at least.
*  - to get the whole membership in iterations, call this function
*    as many time as needed till iter == DNX_SAND_GROUP_MEM_LL_END be TRUE.
*    when in first call iter = 0, and in the next call iter not touched.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  dnx_sand_group_mem_ll_members_get(
    DNX_SAND_INOUT  DNX_SAND_GROUP_MEM_LL_INFO        *gr_mem_info,
    DNX_SAND_IN     DNX_SAND_GROUP_MEM_LL_GROUP_ID    group_ndx,
    DNX_SAND_IN     uint32                      max_members,
    DNX_SAND_INOUT  DNX_SAND_GROUP_MEM_LL_MEMBER_ID   *iter,
    DNX_SAND_OUT    DNX_SAND_GROUP_MEM_LL_MEMBER_ID   *members,
    DNX_SAND_OUT    uint32                      *nof_members
  );


/*********************************************************************
* NAME:
*     dnx_sand_group_mem_ll_member_exist
* TYPE:
*   PROC
* DATE:
*   May  6 2008
* FUNCTION:
*  checks if element is a member in a group.
* INPUT:
*   DNX_SAND_INOUT   DNX_SAND_GROUP_MEM_LL_INFO     *gr_mem_info -
*     The group_member_list instance
*   DNX_SAND_IN  DNX_SAND_GROUP_MEM_LL_GROUP_ID     group_ndx -
*     the group id to check if the given member belongs to.
*   DNX_SAND_IN  DNX_SAND_GROUP_MEM_LL_MEMBER_ID    member_ndx -
*     the member id, element to check if it exist in the group membership.
*   DNX_SAND_OUT    uint8                     *exist -
*     the result, is the member (element) is a member in the group.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  dnx_sand_group_mem_ll_member_exist(
    DNX_SAND_INOUT  DNX_SAND_GROUP_MEM_LL_INFO        *gr_mem_info,
    DNX_SAND_IN     DNX_SAND_GROUP_MEM_LL_GROUP_ID    group_ndx,
    DNX_SAND_IN     DNX_SAND_GROUP_MEM_LL_MEMBER_ID   member_ndx,
    DNX_SAND_OUT    uint8                     *exist
  );

/*********************************************************************
* NAME:
*     dnx_sand_group_mem_ll_is_group_empty
* TYPE:
*   PROC
* DATE:
*   May  6 2008
* FUNCTION:
*  checks if a goup is empty (has no members)
* INPUT:
*   DNX_SAND_INOUT   DNX_SAND_GROUP_MEM_LL_INFO     *gr_mem_info -
*     The group_member_list instance
*   DNX_SAND_IN  DNX_SAND_GROUP_MEM_LL_GROUP_ID     group_ndx -
*     the group id 
*   DNX_SAND_OUT    uint8                     *exist -
*     the result, is the group empty.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  dnx_sand_group_mem_ll_is_group_empty(
    DNX_SAND_INOUT  DNX_SAND_GROUP_MEM_LL_INFO        *gr_mem_info,
    DNX_SAND_IN     DNX_SAND_GROUP_MEM_LL_GROUP_ID    group_ndx,
    DNX_SAND_OUT    uint8                     *is_empty
  );
/*********************************************************************
* NAME:
*     dnx_sand_group_mem_ll_member_get_group
* TYPE:
*   PROC
* DATE:
*   May  6 2008
* FUNCTION:
*  return the group the member belongs to if any.
* INPUT:
*   DNX_SAND_INOUT   DNX_SAND_GROUP_MEM_LL_INFO     *gr_mem_info -
*     The group_member_list instance
*   DNX_SAND_IN  DNX_SAND_GROUP_MEM_LL_MEMBER_ID    member_ndx -
*     the member id.
*   DNX_SAND_OUT  DNX_SAND_GROUP_MEM_LL_GROUP_ID     group_ndx -
*     the group id to given member belongs to.
*   DNX_SAND_OUT    uint8                     *valid -
*     is the element member in any group.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  dnx_sand_group_mem_ll_member_get_group(
    DNX_SAND_INOUT  DNX_SAND_GROUP_MEM_LL_INFO        *gr_mem_info,
    DNX_SAND_IN     DNX_SAND_GROUP_MEM_LL_MEMBER_ID   member_ndx,
    DNX_SAND_OUT    DNX_SAND_GROUP_MEM_LL_GROUP_ID    *group_ndx,
    DNX_SAND_OUT    uint8                     *valid
  );


/*********************************************************************
* NAME:
*     dnx_sand_group_mem_ll_group_clear
* TYPE:
*   PROC
* DATE:
*   May  6 2008
* FUNCTION:
*  clear all members in a group.
* INPUT:
*   DNX_SAND_INOUT   DNX_SAND_GROUP_MEM_LL_INFO     *gr_mem_info -
*     The group_member_list instance
*   DNX_SAND_IN  DNX_SAND_GROUP_MEM_LL_GROUP_ID     group_ndx -
*     the group id to print.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  dnx_sand_group_mem_ll_group_clear(
    DNX_SAND_INOUT   DNX_SAND_GROUP_MEM_LL_INFO     *gr_mem_info,
    DNX_SAND_IN  DNX_SAND_GROUP_MEM_LL_GROUP_ID     group_ndx
  );

/*********************************************************************
* NAME:
*     dnx_sand_group_mem_ll_member_remove
* TYPE:
*   PROC
* DATE:
*   May  6 2008
* FUNCTION:
*  print the members of a group.
* INPUT:
*   DNX_SAND_INOUT   DNX_SAND_GROUP_MEM_LL_INFO     *gr_mem_info -
*     The group_member_list instance
*   DNX_SAND_IN  DNX_SAND_GROUP_MEM_LL_GROUP_ID     group_ndx -
*     the group id to print.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  dnx_sand_group_mem_ll_group_print(
    DNX_SAND_INOUT   DNX_SAND_GROUP_MEM_LL_INFO     *gr_mem_info,
    DNX_SAND_IN  DNX_SAND_GROUP_MEM_LL_GROUP_ID     group_ndx
  );

/*********************************************************************
* NAME:
*     dnx_sand_group_mem_ll_func_run
* TYPE:
*   PROC
* DATE:
*   May  6 2008
* FUNCTION:members
*  run function over each of the group members
* INPUT:
*   DNX_SAND_INOUT   DNX_SAND_GROUP_MEM_LL_INFO     *gr_mem_info -
*     The group_member_list instance
*   DNX_SAND_IN  DNX_SAND_GROUP_MEM_LL_GROUP_ID     group_ndx -
*     the group id to print.
*   DNX_SAND_IN  DNX_SAND_GROUP_MEM_LL_ITER_FUN     func -
*     function to run.
*   DNX_SAND_INOUT  uint32                    param1-
*     parameter to be passed back to 'func'
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  dnx_sand_group_mem_ll_func_run(
    DNX_SAND_INOUT   DNX_SAND_GROUP_MEM_LL_INFO     *gr_mem_info,
    DNX_SAND_IN  DNX_SAND_GROUP_MEM_LL_GROUP_ID     group_ndx,
    DNX_SAND_IN  DNX_SAND_GROUP_MEM_LL_ITER_FUN     func,
    DNX_SAND_INOUT  uint32                    param1,
    DNX_SAND_INOUT  uint32                    param2
  );

/*********************************************************************
* NAME:
*     dnx_sand_group_mem_ll_func_run
* TYPE:
*   PROC
* DATE:
*   May  6 2008
* FUNCTION:members
*  run function over each of the group members
* INPUT:
*   DNX_SAND_INOUT   DNX_SAND_GROUP_MEM_LL_INFO     *gr_mem_info -
*     The group_member_list instance
*   DNX_SAND_IN  DNX_SAND_GROUP_MEM_LL_GROUP_ID     group_ndx -
*     the group id to print.
*   DNX_SAND_IN  DNX_SAND_GROUP_MEM_LL_ITER_FUN     func -
*     function to run.
*   DNX_SAND_INOUT  void                            *opaque -
*     parameter to be passed back to 'func' untouched.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  dnx_sand_group_mem_ll_func_run_pointer_param(
    DNX_SAND_INOUT    DNX_SAND_GROUP_MEM_LL_INFO                    *gr_mem_info,
    DNX_SAND_IN       DNX_SAND_GROUP_MEM_LL_GROUP_ID                group_ndx,
    DNX_SAND_IN       DNX_SAND_GROUP_MEM_LL_ITER_FUN_POINTER_PARAM  func,
    DNX_SAND_INOUT    void                                          *opaque
  );

void
  dnx_sand_SAND_GROUP_MEM_LL_INFO_clear(
    DNX_SAND_GROUP_MEM_LL_INFO *info
  );

uint32
  dnx_sand_group_mem_ll_get_size_for_save(
    DNX_SAND_IN  DNX_SAND_GROUP_MEM_LL_INFO      *gr_mem_info,
    DNX_SAND_OUT  uint32                   *size
  );

uint32
  dnx_sand_group_mem_ll_save(
    DNX_SAND_IN  DNX_SAND_GROUP_MEM_LL_INFO     *info,
    DNX_SAND_OUT uint8                    *buffer,
    DNX_SAND_IN  uint32                   buffer_size_bytes,
    DNX_SAND_OUT uint32                   *actual_size_bytes
  );

uint32
  dnx_sand_group_mem_ll_load(
    DNX_SAND_IN  uint8                            **buffer,
    DNX_SAND_IN  DNX_SAND_GROUP_MEM_LL_GROUP_ENTRY_SET  group_set_fun,
    DNX_SAND_IN  DNX_SAND_GROUP_MEM_LL_GROUP_ENTRY_GET  group_get_fun,
    DNX_SAND_IN  DNX_SAND_GROUP_MEM_LL_MEMBER_ENTRY_SET member_set_fun,
    DNX_SAND_IN  DNX_SAND_GROUP_MEM_LL_MEMBER_ENTRY_GET member_get_fun,
    DNX_SAND_OUT DNX_SAND_GROUP_MEM_LL_INFO             *info
  );

/* } */



#include <soc/dnx/legacy/SAND/Utils/sand_footer.h>

/* } __DNX_SAND_GROUP_MEMBER_LIST_INCLUDED__*/
#endif
