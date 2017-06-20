/* $Id: sand_group_member_list.c,v 1.11 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/
/******************************************************************
*
* FILENAME:       DuneDriver\DNX_SAND\Utils\src\dnx_sand_group_member_list.c
*
* FILE DESCRIPTION:
*
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
#include <soc/dnx/legacy/drv.h>



#include <soc/dnx/legacy/SAND/Utils/sand_header.h>

#include <soc/dnx/legacy/SAND/Utils/sand_group_member_list.h>
#include <soc/dnx/legacy/SAND/Management/sand_error_code.h>
#include <soc/dnx/legacy/SAND/Management/sand_general_macros.h>
#include <soc/dnx/legacy/SAND/Utils/sand_os_interface.h>
#include <soc/dnx/legacy/SAND/Utils/sand_integer_arithmetic.h>
#include <soc/dnx/legacy/drv.h>
#include <soc/dnx/legacy/dnx_wb_engine.h>


/* } */

/*************
* DEFINES   *
*************/
/* { */
/* } */

/*************
*  MACROS   *
*************/
/* { */
#define DNX_SAND_GROUP_MEM_LL_CACHE_INST(_inst) (DNX_SAND_BIT(31)|(_inst))

#define DNX_SAND_GROUP_MEM_LL_ACTIVE_INST(gr_mem_info, _inst) \
  (gr_mem_info->group_members_data.cache_enabled)?DNX_SAND_GROUP_MEM_LL_CACHE_INST(_inst):_inst

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
*   DNX_SAND_IN  DNX_SAND_GROUP_MEM_LL_INFO              *gr_mem_info -
*     information to use in order to create and handle the group_member_list
* REMARKS:
*   None.
* RETURNS:
*   OK or ERROR indication.
*********************************************************************/
uint32
  dnx_sand_group_mem_ll_create(
    DNX_SAND_INOUT   DNX_SAND_GROUP_MEM_LL_INFO      *gr_mem_info
  )
{
  uint32
    res;

  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(DNX_SAND_GROUP_MEM_LL_CREATE);

  DNX_SAND_CHECK_NULL_INPUT(gr_mem_info);
  DNX_SAND_CHECK_NULL_INPUT(gr_mem_info->group_get_fun);
  DNX_SAND_CHECK_NULL_INPUT(gr_mem_info->group_set_fun);
  DNX_SAND_CHECK_NULL_INPUT(gr_mem_info->member_get_fun);
  DNX_SAND_CHECK_NULL_INPUT(gr_mem_info->member_set_fun);

  if (gr_mem_info->nof_groups == 0 || gr_mem_info->nof_groups == DNX_SAND_GROUP_MEM_LL_END )
  {
    DNX_SAND_SET_ERROR_CODE(DNX_SAND_GROUP_MEM_LL_NOF_GROUPS_OUT_OF_RANGE_ERR, 10, exit);
  }
  if (gr_mem_info->nof_elements == 0 || gr_mem_info->nof_elements == DNX_SAND_GROUP_MEM_LL_END )
  {
    DNX_SAND_SET_ERROR_CODE(DNX_SAND_GROUP_MEM_LL_NOF_MEMBERS_OUT_OF_RANGE_ERR, 20, exit);
  }
 /*
  * allocate the group membership
  */
  gr_mem_info->group_members_data.groups = 
       (DNX_SAND_GROUP_MEM_LL_GROUP_ENTRY*) dnx_sand_os_malloc_any_size(sizeof(DNX_SAND_GROUP_MEM_LL_GROUP_ENTRY) * gr_mem_info->nof_groups, "gr_mem_info->group_members_data.groups");
  if (!gr_mem_info->group_members_data.groups)
  {
    DNX_SAND_SET_ERROR_CODE(DNX_SAND_MALLOC_FAIL, 30, exit);
  }
  gr_mem_info->group_members_data.members = 
       (DNX_SAND_GROUP_MEM_LL_MEMBER_ENTRY*) dnx_sand_os_malloc_any_size(sizeof(DNX_SAND_GROUP_MEM_LL_MEMBER_ENTRY) * gr_mem_info->nof_elements, "gr_mem_info->group_members_data.members");
  if (!gr_mem_info->group_members_data.members)
  {
    DNX_SAND_SET_ERROR_CODE(DNX_SAND_MALLOC_FAIL, 40, exit);
  }
  /* initialize the group membership */
  res = dnx_sand_os_memset(
          gr_mem_info->group_members_data.groups,
          0xFF,
          sizeof(DNX_SAND_GROUP_MEM_LL_GROUP_ENTRY) * gr_mem_info->nof_groups
        );
  DNX_SAND_CHECK_FUNC_RESULT(res, 50, exit);

  res = dnx_sand_os_memset(
          gr_mem_info->group_members_data.members,
          0xFF,
          sizeof(DNX_SAND_GROUP_MEM_LL_MEMBER_ENTRY) * gr_mem_info->nof_elements
        );
  DNX_SAND_CHECK_FUNC_RESULT(res, 60, exit);

  if (gr_mem_info->support_caching)
  {
    gr_mem_info->group_members_data.groups_cache = 
          (DNX_SAND_GROUP_MEM_LL_GROUP_ENTRY*) dnx_sand_os_malloc_any_size(sizeof(DNX_SAND_GROUP_MEM_LL_GROUP_ENTRY) * gr_mem_info->nof_groups, "gr_mem_info->group_members_data.groups_cache");
    if (!gr_mem_info->group_members_data.groups_cache)
    {
      DNX_SAND_SET_ERROR_CODE(DNX_SAND_MALLOC_FAIL, 130, exit);
    }
    gr_mem_info->group_members_data.members_cache = 
          (DNX_SAND_GROUP_MEM_LL_MEMBER_ENTRY*) dnx_sand_os_malloc_any_size(sizeof(DNX_SAND_GROUP_MEM_LL_MEMBER_ENTRY) * gr_mem_info->nof_elements, "gr_mem_info->group_members_data.members_cache");
    if (!gr_mem_info->group_members_data.members_cache)
    {
      DNX_SAND_SET_ERROR_CODE(DNX_SAND_MALLOC_FAIL, 140, exit);
    }
    /* initialize the group membership */
    res = dnx_sand_os_memset(
            gr_mem_info->group_members_data.groups_cache,
            0xFF,
            sizeof(DNX_SAND_GROUP_MEM_LL_GROUP_ENTRY) * gr_mem_info->nof_groups
          );
    DNX_SAND_CHECK_FUNC_RESULT(res, 150, exit);

    res = dnx_sand_os_memset(
            gr_mem_info->group_members_data.members_cache,
            0xFF,
            sizeof(DNX_SAND_GROUP_MEM_LL_MEMBER_ENTRY) * gr_mem_info->nof_elements
          );
    DNX_SAND_CHECK_FUNC_RESULT(res, 160, exit);
  }

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in dnx_sand_group_mem_ll_create()",0,0);
}


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
*  DNX_SAND_OUT  DNX_SAND_GROUP_MEM_LL_INFO gr_mem_info -
*     The group_member_list to destroy.
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  dnx_sand_group_mem_ll_destroy(
    DNX_SAND_INOUT   DNX_SAND_GROUP_MEM_LL_INFO      *gr_mem_info
  )
{
  uint32
    res;

  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(DNX_SAND_GROUP_MEM_LL_DESTROY);

  DNX_SAND_CHECK_NULL_INPUT(gr_mem_info);

  /* free group membership used memories*/

  res = dnx_sand_os_free_any_size(
          gr_mem_info->group_members_data.groups
        );
  DNX_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  res = dnx_sand_os_free_any_size(
          gr_mem_info->group_members_data.members
        );
  DNX_SAND_CHECK_FUNC_RESULT(res, 20, exit);

  if (gr_mem_info->support_caching)
  {
    res = dnx_sand_os_free_any_size(
            gr_mem_info->group_members_data.groups_cache
          );
    DNX_SAND_CHECK_FUNC_RESULT(res, 110, exit);

    res = dnx_sand_os_free_any_size(
            gr_mem_info->group_members_data.members_cache
          );
    DNX_SAND_CHECK_FUNC_RESULT(res, 120, exit);
  }
  
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in dnx_sand_group_mem_ll_destroy()",0,0);
}


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
  )
{
  uint32
      res;
  int unit = BSL_UNIT_UNKNOWN;

  DNX_SAND_INIT_ERROR_DEFINITIONS(DNX_SAND_GROUP_MEM_LL_CLEAR);

  DNX_SAND_CHECK_NULL_INPUT(gr_mem_info);


  unit = (gr_mem_info->instance_prim_handle);

  /* initialize the group membership */
  if(gr_mem_info->wb_var_index == SOC_DNX_WB_ENGINE_VAR_NONE) {
      res = dnx_sand_os_memset(
                               gr_mem_info->group_members_data.groups,
                               0xFF,
                               sizeof(DNX_SAND_GROUP_MEM_LL_GROUP_ENTRY) * gr_mem_info->nof_groups
                               );
      DNX_SAND_CHECK_FUNC_RESULT(res, 50, exit);

      res = dnx_sand_os_memset(
                               gr_mem_info->group_members_data.members,
                               0xFF,
                               sizeof(DNX_SAND_GROUP_MEM_LL_MEMBER_ENTRY) * gr_mem_info->nof_elements
                               );
      DNX_SAND_CHECK_FUNC_RESULT(res, 60, exit);

  } else {

      res = SOC_DNX_WB_ENGINE_MEMSET_ARR(unit, gr_mem_info->wb_var_index + DNX_WB_ENGINE_GROUP_MEM_LL_GROUPS, 0xFF);
      DNX_SAND_CHECK_FUNC_RESULT(res, 70, exit);

      res = SOC_DNX_WB_ENGINE_MEMSET_ARR(unit, gr_mem_info->wb_var_index + DNX_WB_ENGINE_GROUP_MEM_LL_MEMBERS, 0xFF);
      DNX_SAND_CHECK_FUNC_RESULT(res, 80, exit);
  }


  if (gr_mem_info->support_caching)
  {
      if(gr_mem_info->wb_var_index == SOC_DNX_WB_ENGINE_VAR_NONE) {

          res = dnx_sand_os_memset(
                                   gr_mem_info->group_members_data.groups_cache,
                                   0xFF,
                                   sizeof(DNX_SAND_GROUP_MEM_LL_GROUP_ENTRY) * gr_mem_info->nof_groups
                                   );
          DNX_SAND_CHECK_FUNC_RESULT(res, 150, exit);

          res = dnx_sand_os_memset(
                                   gr_mem_info->group_members_data.members_cache,
                                   0xFF,
                                   sizeof(DNX_SAND_GROUP_MEM_LL_MEMBER_ENTRY) * gr_mem_info->nof_elements
                                   );
          DNX_SAND_CHECK_FUNC_RESULT(res, 160, exit);

      } else {

          res = SOC_DNX_WB_ENGINE_MEMSET_ARR(unit, gr_mem_info->wb_var_index + DNX_WB_ENGINE_GROUP_MEM_LL_GROUPS_CACHE, 0xFF);
          DNX_SAND_CHECK_FUNC_RESULT(res, 170, exit);

          res = SOC_DNX_WB_ENGINE_MEMSET_ARR(unit, gr_mem_info->wb_var_index + DNX_WB_ENGINE_GROUP_MEM_LL_MEMBERS_CACHE, 0xFF);
          DNX_SAND_CHECK_FUNC_RESULT(res, 180, exit);

      }

  }
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in dnx_sand_group_mem_ll_create()",0,0);
}


uint32
  dnx_sand_group_mem_ll_cache_set(
    DNX_SAND_INOUT   DNX_SAND_GROUP_MEM_LL_INFO      *gr_mem_info,
    DNX_SAND_IN uint8                             enable
  )
{
  int unit = BSL_UNIT_UNKNOWN, res;

  DNX_SAND_INIT_ERROR_DEFINITIONS(DNX_SAND_GROUP_MEM_LL_CLEAR);

  DNX_SAND_CHECK_NULL_INPUT(gr_mem_info);


  unit = (gr_mem_info->instance_prim_handle);

  if (!gr_mem_info->support_caching && enable)
  {
    DNX_SAND_SET_ERROR_CODE(DNX_SAND_ERR, 10, exit);
  }

  /* if already enabled and try to enable then do nothing */
  if (gr_mem_info->group_members_data.cache_enabled && enable)
  {
    goto exit;
  }
  
  if(gr_mem_info->wb_var_index == SOC_DNX_WB_ENGINE_VAR_NONE) {
      gr_mem_info->group_members_data.cache_enabled = enable;
  } else {
      res = SOC_DNX_WB_ENGINE_SET_VAR(unit, gr_mem_info->wb_var_index + DNX_WB_ENGINE_GROUP_MEM_LL_CACHE_ENABLED, 
                                      &(enable));
      DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 10, exit);
  }
 /*
  * copy real state to cache state
  */
  if (enable)
  {
      if(gr_mem_info->wb_var_index == SOC_DNX_WB_ENGINE_VAR_NONE) {

          dnx_sand_os_memcpy(gr_mem_info->group_members_data.groups_cache,gr_mem_info->group_members_data.groups,sizeof(DNX_SAND_GROUP_MEM_LL_GROUP_ENTRY) * gr_mem_info->nof_groups);
          dnx_sand_os_memcpy(gr_mem_info->group_members_data.members_cache,gr_mem_info->group_members_data.members, sizeof(DNX_SAND_GROUP_MEM_LL_MEMBER_ENTRY) * gr_mem_info->nof_elements);

      } else {

          res = SOC_DNX_WB_ENGINE_MEMCPY_ARR(unit, gr_mem_info->wb_var_index + DNX_WB_ENGINE_GROUP_MEM_LL_GROUPS_CACHE, 
                                             gr_mem_info->group_members_data.groups,
                                             0,
                                             gr_mem_info->nof_groups);
          DNX_SAND_CHECK_FUNC_RESULT(res, 20, exit);

          res = SOC_DNX_WB_ENGINE_MEMCPY_ARR(unit, gr_mem_info->wb_var_index + DNX_WB_ENGINE_GROUP_MEM_LL_MEMBERS_CACHE, 
                                             gr_mem_info->group_members_data.members,
                                             0,
                                             gr_mem_info->nof_elements);
          DNX_SAND_CHECK_FUNC_RESULT(res, 30, exit);
        
      }
  }

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in dnx_sand_arr_mem_allocator_cache_set()",0,0);
}

uint32
  dnx_sand_group_mem_ll_commit(
    DNX_SAND_INOUT   DNX_SAND_GROUP_MEM_LL_INFO      *gr_mem_info,
    DNX_SAND_IN uint32 flags
  )
{
  int unit = BSL_UNIT_UNKNOWN, res;

  DNX_SAND_INIT_ERROR_DEFINITIONS(DNX_SAND_GROUP_MEM_LL_CLEAR);

  DNX_SAND_CHECK_NULL_INPUT(gr_mem_info);


  unit = (gr_mem_info->instance_prim_handle);

  if (!gr_mem_info->support_caching)
  {
    DNX_SAND_SET_ERROR_CODE(DNX_SAND_ERR, 10, exit);
  }
  /* if no cache just exit */
  if (!gr_mem_info->group_members_data.cache_enabled)
  {
    goto exit;
  }
 /*
  * copy cache state to real state
  */
  if(gr_mem_info->wb_var_index == SOC_DNX_WB_ENGINE_VAR_NONE) {

      dnx_sand_os_memcpy(gr_mem_info->group_members_data.groups,gr_mem_info->group_members_data.groups_cache,sizeof(DNX_SAND_GROUP_MEM_LL_GROUP_ENTRY) * gr_mem_info->nof_groups);
      dnx_sand_os_memcpy(gr_mem_info->group_members_data.members,gr_mem_info->group_members_data.members_cache, sizeof(DNX_SAND_GROUP_MEM_LL_MEMBER_ENTRY) * gr_mem_info->nof_elements);

  } else {

      res = SOC_DNX_WB_ENGINE_MEMCPY_ARR(unit, gr_mem_info->wb_var_index + DNX_WB_ENGINE_GROUP_MEM_LL_GROUPS, 
                                         gr_mem_info->group_members_data.groups_cache,
                                         0,
                                         gr_mem_info->nof_groups);
      DNX_SAND_CHECK_FUNC_RESULT(res, 10, exit);

      res = SOC_DNX_WB_ENGINE_MEMCPY_ARR(unit, gr_mem_info->wb_var_index + DNX_WB_ENGINE_GROUP_MEM_LL_MEMBERS, 
                                         gr_mem_info->group_members_data.members_cache,
                                         0,
                                         gr_mem_info->nof_elements);
      DNX_SAND_CHECK_FUNC_RESULT(res, 20, exit);

  }

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in dnx_sand_group_mem_ll_commit()",0,0);
}


uint32
  dnx_sand_group_mem_ll_rollback(
    DNX_SAND_INOUT   DNX_SAND_GROUP_MEM_LL_INFO      *gr_mem_info,
    DNX_SAND_IN uint32 flags
  )
{
  int unit = BSL_UNIT_UNKNOWN, res;

  DNX_SAND_INIT_ERROR_DEFINITIONS(DNX_SAND_GROUP_MEM_LL_CLEAR);

  DNX_SAND_CHECK_NULL_INPUT(gr_mem_info);


  unit = (gr_mem_info->instance_prim_handle);

  if (!gr_mem_info->support_caching)
  {
    DNX_SAND_SET_ERROR_CODE(DNX_SAND_ERR, 10, exit);
  }

  /* if no cache just exit */
  if (!gr_mem_info->group_members_data.cache_enabled)
  {
    goto exit;
  }
  
 /*
  * copy real state again to cache state
  */
  if(gr_mem_info->wb_var_index == SOC_DNX_WB_ENGINE_VAR_NONE) {

      dnx_sand_os_memcpy(gr_mem_info->group_members_data.groups_cache,gr_mem_info->group_members_data.groups,sizeof(DNX_SAND_GROUP_MEM_LL_GROUP_ENTRY) * gr_mem_info->nof_groups);
      dnx_sand_os_memcpy(gr_mem_info->group_members_data.members_cache,gr_mem_info->group_members_data.members, sizeof(DNX_SAND_GROUP_MEM_LL_MEMBER_ENTRY) * gr_mem_info->nof_elements);

  } else {

      res = SOC_DNX_WB_ENGINE_MEMCPY_ARR(unit, gr_mem_info->wb_var_index + DNX_WB_ENGINE_GROUP_MEM_LL_GROUPS_CACHE, 
                                         gr_mem_info->group_members_data.groups,
                                         0, 
                                         gr_mem_info->nof_groups);
      DNX_SAND_CHECK_FUNC_RESULT(res, 10, exit);

      res = SOC_DNX_WB_ENGINE_MEMCPY_ARR(unit, gr_mem_info->wb_var_index + DNX_WB_ENGINE_GROUP_MEM_LL_MEMBERS_CACHE, 
                                         gr_mem_info->group_members_data.members, 
                                         0,
                                         gr_mem_info->nof_elements);
      DNX_SAND_CHECK_FUNC_RESULT(res, 20, exit);

  }

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in dnx_sand_group_mem_ll_rollback()",0,0);
}

/*********************************************************************
* NAME:
*     dnx_sand_group_mem_ll_member_add
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
  )
{
  uint32
    indx;
  DNX_SAND_GROUP_MEM_LL_GROUP_ENTRY
    grp_entry;
  DNX_SAND_GROUP_MEM_LL_MEMBER_ENTRY
    member_entry;
  DNX_SAND_GROUP_MEM_LL_MEMBER_ID
    prev,
    curr,
    next;
  uint32
    res;

  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(DNX_SAND_GROUP_MEM_LL_MEMBERS_SET);

  DNX_SAND_CHECK_NULL_INPUT(gr_mem_info);
  DNX_SAND_CHECK_NULL_INPUT(members);

  if (group_ndx > gr_mem_info->nof_groups)
  {
    DNX_SAND_SET_ERROR_CODE(DNX_SAND_GROUP_MEM_LL_GROUP_ID_OUT_OF_RANGE_ERR, 10, exit);
  }

  for (indx = 0; indx < nof_members; ++indx)
  {
    if (members[indx] > gr_mem_info->nof_elements)
    {
      DNX_SAND_SET_ERROR_CODE(DNX_SAND_GROUP_MEM_LL_MEMBER_ID_OUT_OF_RANGE_ERR, 20, exit);
    }
   /*
    * check if the element is already member in any group.
    */
    res = gr_mem_info->member_get_fun(
            gr_mem_info->instance_prim_handle,
            DNX_SAND_GROUP_MEM_LL_ACTIVE_INST(gr_mem_info,gr_mem_info->instance_sec_handle),
            members[indx],
            &member_entry
          );
    DNX_SAND_CHECK_FUNC_RESULT(res, 30, exit);
   /*
    * if already member in this group then there is nothing to do.
    */
    if (member_entry.group_id == group_ndx)
    {
      continue;
    }
   /*
    * if already member in a another group
    */
    if (member_entry.group_id != DNX_SAND_GROUP_MEM_LL_END)
    {
     /*
      * if auto_remove FALSE then return ERROR
      */
      if (!gr_mem_info->auto_remove)
      {
        DNX_SAND_SET_ERROR_CODE(DNX_SAND_GROUP_MEM_LL_ILLEGAL_ADD_ERR, 40, exit);
      }
      else
      {
       /*
        * if already member in a group and auto_remove TRUE then
        * remove from the "old group"
        */
        res = dnx_sand_group_mem_ll_member_remove(
                gr_mem_info,
                members[indx]
              );
        DNX_SAND_CHECK_FUNC_RESULT(res, 50, exit);
      }
    }
  }
  grp_entry.first_member = members[0];
  res = gr_mem_info->group_set_fun(
          gr_mem_info->instance_prim_handle,
          DNX_SAND_GROUP_MEM_LL_ACTIVE_INST(gr_mem_info,gr_mem_info->instance_sec_handle),
          group_ndx,
          &grp_entry
        );
  DNX_SAND_CHECK_FUNC_RESULT(res, 60, exit);

  prev = DNX_SAND_GROUP_MEM_LL_END;
  member_entry.group_id = group_ndx;
  for (indx = 0; indx < nof_members; ++indx)
  {
    curr = members[indx];
    next = (indx + 1 < nof_members)? members[indx+1] : DNX_SAND_GROUP_MEM_LL_END;
    member_entry.next_member = next;
    member_entry.prev_member = prev;
    res = gr_mem_info->member_set_fun(
            gr_mem_info->instance_prim_handle,
            DNX_SAND_GROUP_MEM_LL_ACTIVE_INST(gr_mem_info,gr_mem_info->instance_sec_handle),
            curr,
            &member_entry
          );
    DNX_SAND_CHECK_FUNC_RESULT(res, 70, exit);
    prev = curr;
  }
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in dnx_sand_group_mem_ll_members_set()",0,0);
}


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
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  dnx_sand_group_mem_ll_member_add(
    DNX_SAND_INOUT   DNX_SAND_GROUP_MEM_LL_INFO     *gr_mem_info,
    DNX_SAND_IN  DNX_SAND_GROUP_MEM_LL_GROUP_ID     group_ndx,
    DNX_SAND_IN  DNX_SAND_GROUP_MEM_LL_MEMBER_ID    member_id
  )
{
  DNX_SAND_GROUP_MEM_LL_GROUP_ENTRY
    grp_entry;
  DNX_SAND_GROUP_MEM_LL_MEMBER_ENTRY
    member_entry;
  DNX_SAND_GROUP_MEM_LL_MEMBER_ID
    next;
  uint32
    res;

  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(DNX_SAND_GROUP_MEM_LL_MEMBER_ADD);

  DNX_SAND_CHECK_NULL_INPUT(gr_mem_info);

  if (group_ndx > gr_mem_info->nof_groups)
  {
    DNX_SAND_SET_ERROR_CODE(DNX_SAND_GROUP_MEM_LL_GROUP_ID_OUT_OF_RANGE_ERR, 10, exit);
  }
  if (member_id > gr_mem_info->nof_elements)
  {
    DNX_SAND_SET_ERROR_CODE(DNX_SAND_GROUP_MEM_LL_MEMBER_ID_OUT_OF_RANGE_ERR, 20, exit);
  }
 /*
  * check if the element is already member in any group.
  */
  res = gr_mem_info->member_get_fun(
          gr_mem_info->instance_prim_handle,
          DNX_SAND_GROUP_MEM_LL_ACTIVE_INST(gr_mem_info,gr_mem_info->instance_sec_handle),
          member_id,
          &member_entry
        );
  DNX_SAND_CHECK_FUNC_RESULT(res, 30, exit);
 /*
  * if already member in this group then there is nothing to do.
  */
  if (member_entry.group_id == group_ndx)
  {
    goto exit;
  }
 /*
  * if already member in a another group
  */
  if (member_entry.group_id != DNX_SAND_GROUP_MEM_LL_END)
  {
   /*
    * if auto_remove FALSE then return ERROR
    */
    if (!gr_mem_info->auto_remove)
    {
      DNX_SAND_SET_ERROR_CODE(DNX_SAND_GROUP_MEM_LL_ILLEGAL_ADD_ERR, 40, exit);
    }
    else
    {
     /*
      * if already member in a group and auto_remove TRUE then
      * remove from the "old group"
      */
      res = dnx_sand_group_mem_ll_member_remove(
              gr_mem_info,
              member_id
            );
      DNX_SAND_CHECK_FUNC_RESULT(res, 50, exit);
    }
  }

  res = gr_mem_info->group_get_fun(
          gr_mem_info->instance_prim_handle,
          DNX_SAND_GROUP_MEM_LL_ACTIVE_INST(gr_mem_info,gr_mem_info->instance_sec_handle),
          group_ndx,
          &grp_entry
        );
  DNX_SAND_CHECK_FUNC_RESULT(res, 60, exit);

 /*
  * update the group information
  *   - number of members
  *   - add new element
  */
  next = grp_entry.first_member;
  grp_entry.first_member = member_id;
  res = gr_mem_info->group_set_fun(
          gr_mem_info->instance_prim_handle,
          DNX_SAND_GROUP_MEM_LL_ACTIVE_INST(gr_mem_info,gr_mem_info->instance_sec_handle),
          group_ndx,
          &grp_entry
        );
  DNX_SAND_CHECK_FUNC_RESULT(res, 70, exit);

 /*
  * update member id
  */
  member_entry.prev_member = DNX_SAND_GROUP_MEM_LL_END;
  member_entry.next_member = next;
  member_entry.group_id = group_ndx;
  res = gr_mem_info->member_set_fun(
          gr_mem_info->instance_prim_handle,
          DNX_SAND_GROUP_MEM_LL_ACTIVE_INST(gr_mem_info,gr_mem_info->instance_sec_handle),
          member_id,
          &member_entry
        );
  DNX_SAND_CHECK_FUNC_RESULT(res, 80, exit);
 /*
  * if next is not NULL then update it.
  */
  if (next != DNX_SAND_GROUP_MEM_LL_END)
  {
    res = gr_mem_info->member_get_fun(
            gr_mem_info->instance_prim_handle,
            DNX_SAND_GROUP_MEM_LL_ACTIVE_INST(gr_mem_info,gr_mem_info->instance_sec_handle),
            next,
            &member_entry
          );
    DNX_SAND_CHECK_FUNC_RESULT(res, 90, exit);

    member_entry.prev_member = member_id;

    res = gr_mem_info->member_set_fun(
            gr_mem_info->instance_prim_handle,
            DNX_SAND_GROUP_MEM_LL_ACTIVE_INST(gr_mem_info,gr_mem_info->instance_sec_handle),
            next,
            &member_entry
          );
    DNX_SAND_CHECK_FUNC_RESULT(res, 100, exit);
  }

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in dnx_sand_group_mem_ll_member_add()",0,0);
}


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
  )
{
  DNX_SAND_GROUP_MEM_LL_GROUP_ENTRY
    grp_entry;
  DNX_SAND_GROUP_MEM_LL_MEMBER_ENTRY
    member_entry;
  DNX_SAND_GROUP_MEM_LL_MEMBER_ID
    prev,
    next;
  DNX_SAND_GROUP_MEM_LL_GROUP_ID
    group_id;
  uint32
    res;

  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(DNX_SAND_GROUP_MEM_LL_MEMBER_REMOVE);

  DNX_SAND_CHECK_NULL_INPUT(gr_mem_info);

  if (member_id > gr_mem_info->nof_elements)
  {
    DNX_SAND_SET_ERROR_CODE(DNX_SAND_GROUP_MEM_LL_MEMBER_ID_OUT_OF_RANGE_ERR, 10, exit);
  }
 /*
  * get the element info.
  */
  res = gr_mem_info->member_get_fun(
          gr_mem_info->instance_prim_handle,
          DNX_SAND_GROUP_MEM_LL_ACTIVE_INST(gr_mem_info,gr_mem_info->instance_sec_handle),
          member_id,
          &member_entry
        );
  DNX_SAND_CHECK_FUNC_RESULT(res, 20, exit);

  prev = member_entry.prev_member;
  next = member_entry.next_member;
  group_id = member_entry.group_id;
 /*
  * if already not member in any group then there is nothing to do.
  */
  if (group_id == DNX_SAND_GROUP_MEM_LL_END)
  {
    goto exit;
  }
 /*
  * delete member.
  */
  member_entry.group_id = DNX_SAND_GROUP_MEM_LL_END;
  member_entry.next_member = DNX_SAND_GROUP_MEM_LL_END;
  member_entry.prev_member = DNX_SAND_GROUP_MEM_LL_END;

  res = gr_mem_info->member_set_fun(
          gr_mem_info->instance_prim_handle,
          DNX_SAND_GROUP_MEM_LL_ACTIVE_INST(gr_mem_info,gr_mem_info->instance_sec_handle),
          member_id,
          &member_entry
        );
  DNX_SAND_CHECK_FUNC_RESULT(res, 30, exit);


 /*
  * update the group information
  *   - number of members
  *   - remove element
  */
  if (prev == DNX_SAND_GROUP_MEM_LL_END)
  {
    res = gr_mem_info->group_get_fun(
            gr_mem_info->instance_prim_handle,
            DNX_SAND_GROUP_MEM_LL_ACTIVE_INST(gr_mem_info,gr_mem_info->instance_sec_handle),
            group_id,
            &grp_entry
          );
    DNX_SAND_CHECK_FUNC_RESULT(res, 40, exit);
    grp_entry.first_member = next;
     res = gr_mem_info->group_set_fun(
            gr_mem_info->instance_prim_handle,
            DNX_SAND_GROUP_MEM_LL_ACTIVE_INST(gr_mem_info,gr_mem_info->instance_sec_handle),
            group_id,
            &grp_entry
          );
    DNX_SAND_CHECK_FUNC_RESULT(res, 50, exit);
  }
 /*
  * update prev if it not NULL
  */
  if (prev != DNX_SAND_GROUP_MEM_LL_END)
  {
    res = gr_mem_info->member_get_fun(
            gr_mem_info->instance_prim_handle,
            DNX_SAND_GROUP_MEM_LL_ACTIVE_INST(gr_mem_info,gr_mem_info->instance_sec_handle),
            prev,
            &member_entry
          );
    DNX_SAND_CHECK_FUNC_RESULT(res, 70, exit);

    member_entry.next_member = next;

    res = gr_mem_info->member_set_fun(
            gr_mem_info->instance_prim_handle,
            DNX_SAND_GROUP_MEM_LL_ACTIVE_INST(gr_mem_info,gr_mem_info->instance_sec_handle),
            prev,
            &member_entry
          );
    DNX_SAND_CHECK_FUNC_RESULT(res, 80, exit);

  }
 /*
  * update next if it not NULL
  */
  if (next != DNX_SAND_GROUP_MEM_LL_END)
  {
    res = gr_mem_info->member_get_fun(
            gr_mem_info->instance_prim_handle,
            DNX_SAND_GROUP_MEM_LL_ACTIVE_INST(gr_mem_info,gr_mem_info->instance_sec_handle),
            next,
            &member_entry
          );
    DNX_SAND_CHECK_FUNC_RESULT(res, 90, exit);

    member_entry.prev_member = prev;

    res = gr_mem_info->member_set_fun(
            gr_mem_info->instance_prim_handle,
            DNX_SAND_GROUP_MEM_LL_ACTIVE_INST(gr_mem_info,gr_mem_info->instance_sec_handle),
            next,
            &member_entry
          );
    DNX_SAND_CHECK_FUNC_RESULT(res, 100, exit);
  }

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in dnx_sand_group_mem_ll_member_remove()",0,0);
}


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
*   DNX_SAND_IN  uint32                       max_members -
*     the maximum number of members to get.
*   DNX_SAND_INOUT   DNX_SAND_GROUP_MEM_LL_MEMBER_ID *iter -
*     iterator to to get the membership in iterations (blocks), in case of
*     big membership.
*     set iter = 0, to start from the first member in the group.
*     use iter == DNX_SAND_GROUP_MEM_LL_END to check if the iterator
*     reached the end of the membership.
*   DNX_SAND_IN  uint32                       nof_members -
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
  )
{
 uint32
    indx;
  DNX_SAND_GROUP_MEM_LL_GROUP_ENTRY
    grp_entry;
  DNX_SAND_GROUP_MEM_LL_MEMBER_ENTRY
    member_entry;
  DNX_SAND_GROUP_MEM_LL_MEMBER_ID
    tmp_iter;
  uint32
    res;

  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(DNX_SAND_GROUP_MEM_LL_MEMBERS_GET);

  DNX_SAND_CHECK_NULL_INPUT(gr_mem_info);

  if (group_ndx > gr_mem_info->nof_groups)
  {
    DNX_SAND_SET_ERROR_CODE(DNX_SAND_GROUP_MEM_LL_GROUP_ID_OUT_OF_RANGE_ERR, 10, exit);
  }

  tmp_iter = *iter;
  *nof_members = 0;

  res = gr_mem_info->group_get_fun(
          gr_mem_info->instance_prim_handle,
          DNX_SAND_GROUP_MEM_LL_ACTIVE_INST(gr_mem_info,gr_mem_info->instance_sec_handle),
          group_ndx,
          &grp_entry
        );
  DNX_SAND_CHECK_FUNC_RESULT(res, 20, exit);

  if (tmp_iter == 0)
  {
    tmp_iter = grp_entry.first_member;
  }

  for (indx = 0; indx < max_members && tmp_iter != DNX_SAND_GROUP_MEM_LL_END ; ++indx)
  {
    res = gr_mem_info->member_get_fun(
            gr_mem_info->instance_prim_handle,
            DNX_SAND_GROUP_MEM_LL_ACTIVE_INST(gr_mem_info,gr_mem_info->instance_sec_handle),
            tmp_iter,
            &member_entry
          );
    DNX_SAND_CHECK_FUNC_RESULT(res, 30, exit);

    if (member_entry.group_id == DNX_SAND_GROUP_MEM_LL_END)
    {
      tmp_iter = DNX_SAND_GROUP_MEM_LL_END;
      break;
    }
    members[indx] = tmp_iter;
    tmp_iter = member_entry.next_member;
    *nof_members += 1;
  }

  *iter = tmp_iter;

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in dnx_sand_group_mem_ll_members_get()",0,0);
}


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
  )
{
  DNX_SAND_GROUP_MEM_LL_MEMBER_ID
    iter=0;
  DNX_SAND_GROUP_MEM_LL_MEMBER_ID
    member;
  uint32
    nof_members=0;
  uint32
    res;
  
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(DNX_SAND_GROUP_MEM_LL_IS_GROUP_EMPTY);

  res = dnx_sand_group_mem_ll_members_get(
          gr_mem_info,
          group_ndx,
          1,
          &iter,
          &member,
          &nof_members
        );
  DNX_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  *is_empty = (uint8)(nof_members == 0);

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in dnx_sand_group_mem_ll_is_group_empty()",0,0);
}
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
  )
{
  DNX_SAND_GROUP_MEM_LL_MEMBER_ENTRY
    member_entry;
  uint32
    res;

  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(DNX_SAND_GROUP_MEM_LL_MEMBER_GET_GROUP);

  DNX_SAND_CHECK_NULL_INPUT(gr_mem_info);

  if (member_ndx > gr_mem_info->nof_elements)
  {
    DNX_SAND_SET_ERROR_CODE(DNX_SAND_GROUP_MEM_LL_MEMBER_ID_OUT_OF_RANGE_ERR, 10, exit);
  }

  res = gr_mem_info->member_get_fun(
          gr_mem_info->instance_prim_handle,
          DNX_SAND_GROUP_MEM_LL_ACTIVE_INST(gr_mem_info,gr_mem_info->instance_sec_handle),
          member_ndx,
          &member_entry
        );
  DNX_SAND_CHECK_FUNC_RESULT(res, 20, exit);


  if( member_entry.group_id != DNX_SAND_GROUP_MEM_LL_END)
  {
    *group_ndx = member_entry.group_id;
    *valid = TRUE;
  }
  else
  {
    *valid = FALSE;
  }

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in dnx_sand_group_mem_ll_member_get_group()",0,0);
}



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
  )
{
  DNX_SAND_GROUP_MEM_LL_GROUP_ENTRY
    grp_entry;
  DNX_SAND_GROUP_MEM_LL_MEMBER_ENTRY
    member_entry;
  DNX_SAND_GROUP_MEM_LL_MEMBER_ID
    tmp_iter,
    next;
  uint32
    res;

  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(DNX_SAND_GROUP_MEM_LL_MEMBER_CLEAR);

  DNX_SAND_CHECK_NULL_INPUT(gr_mem_info);

  if (group_ndx > gr_mem_info->nof_groups)
  {
    DNX_SAND_SET_ERROR_CODE(DNX_SAND_GROUP_MEM_LL_GROUP_ID_OUT_OF_RANGE_ERR, 10, exit);
  }

  res = gr_mem_info->group_get_fun(
          gr_mem_info->instance_prim_handle,
          DNX_SAND_GROUP_MEM_LL_ACTIVE_INST(gr_mem_info,gr_mem_info->instance_sec_handle),
          group_ndx,
          &grp_entry
        );
  DNX_SAND_CHECK_FUNC_RESULT(res, 20, exit);

  tmp_iter = grp_entry.first_member;
  grp_entry.first_member = DNX_SAND_GROUP_MEM_LL_END;

  res = gr_mem_info->group_set_fun(
          gr_mem_info->instance_prim_handle,
          DNX_SAND_GROUP_MEM_LL_ACTIVE_INST(gr_mem_info,gr_mem_info->instance_sec_handle),
          group_ndx,
          &grp_entry
        );
  DNX_SAND_CHECK_FUNC_RESULT(res, 30, exit);

  while (tmp_iter != DNX_SAND_GROUP_MEM_LL_END)
  {
    res = gr_mem_info->member_get_fun(
            gr_mem_info->instance_prim_handle,
            DNX_SAND_GROUP_MEM_LL_ACTIVE_INST(gr_mem_info,gr_mem_info->instance_sec_handle),
            tmp_iter,
            &member_entry
          );
    DNX_SAND_CHECK_FUNC_RESULT(res, 40, exit);
    next = member_entry.next_member;
   /*
    * delete member.
    */
    member_entry.group_id = DNX_SAND_GROUP_MEM_LL_END;
    member_entry.next_member = DNX_SAND_GROUP_MEM_LL_END;
    member_entry.prev_member = DNX_SAND_GROUP_MEM_LL_END;

    res = gr_mem_info->member_set_fun(
            gr_mem_info->instance_prim_handle,
            DNX_SAND_GROUP_MEM_LL_ACTIVE_INST(gr_mem_info,gr_mem_info->instance_sec_handle),
            tmp_iter,
            &member_entry
          );
    DNX_SAND_CHECK_FUNC_RESULT(res, 50, exit);
    tmp_iter = next;
  }

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in dnx_sand_group_mem_ll_group_clear()",0,0);
}


uint32
  dnx_sand_group_mem_ll_get_size_for_save(
    DNX_SAND_IN  DNX_SAND_GROUP_MEM_LL_INFO      *gr_mem_info,
    DNX_SAND_OUT  uint32                   *size
  )
{
  uint32
    total_size=0;

  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);

  DNX_SAND_CHECK_NULL_INPUT(gr_mem_info);
  DNX_SAND_CHECK_NULL_INPUT(size);

  /* copy init info */
  total_size += sizeof(DNX_SAND_GROUP_MEM_LL_INFO);

  total_size += gr_mem_info->nof_groups * sizeof(DNX_SAND_GROUP_MEM_LL_GROUP_ENTRY);
  total_size += gr_mem_info->nof_elements * sizeof(DNX_SAND_GROUP_MEM_LL_MEMBER_ENTRY);
  if (gr_mem_info->group_members_data.cache_enabled)
  {
    total_size += gr_mem_info->nof_groups * sizeof(DNX_SAND_GROUP_MEM_LL_GROUP_ENTRY);
    total_size += gr_mem_info->nof_elements * sizeof(DNX_SAND_GROUP_MEM_LL_MEMBER_ENTRY);
  }
  
  
  *size = total_size;

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in dnx_sand_group_mem_ll_save()",0,0);
}


uint32
  dnx_sand_group_mem_ll_save(
    DNX_SAND_IN  DNX_SAND_GROUP_MEM_LL_INFO     *gr_mem_info,
    DNX_SAND_OUT uint8                    *buffer,
    DNX_SAND_IN  uint32                   buffer_size_bytes,
    DNX_SAND_OUT uint32                   *actual_size_bytes
  )
{
  uint8
    *cur_ptr = (uint8*)buffer;
  uint32
    cur_size,
    total_size=0;
  uint32
    res;
  DNX_SAND_GROUP_MEM_LL_INFO
    *info;

  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);

  DNX_SAND_CHECK_NULL_INPUT(gr_mem_info);
  DNX_SAND_CHECK_NULL_INPUT(buffer);
  DNX_SAND_CHECK_NULL_INPUT(actual_size_bytes);

  /* copy init info */
  DNX_SAND_COPY_TO_BUFF_AND_INC(cur_ptr,gr_mem_info,DNX_SAND_GROUP_MEM_LL_INFO,1);
  
  /*patch zero in places that contain pointers*/
  info = (DNX_SAND_GROUP_MEM_LL_INFO *) (cur_ptr - sizeof(DNX_SAND_GROUP_MEM_LL_INFO));
  dnx_sand_os_memset(&info->group_members_data.groups, 0x0, sizeof(DNX_SAND_GROUP_MEM_LL_GROUP_ENTRY *));
  dnx_sand_os_memset(&info->group_members_data.members, 0x0, sizeof(DNX_SAND_GROUP_MEM_LL_MEMBER_ENTRY *));
  dnx_sand_os_memset(&info->group_members_data.groups_cache, 0x0, sizeof(DNX_SAND_GROUP_MEM_LL_GROUP_ENTRY *));
  dnx_sand_os_memset(&info->group_members_data.members_cache, 0x0, sizeof(DNX_SAND_GROUP_MEM_LL_MEMBER_ENTRY *));  
  dnx_sand_os_memset(&info->group_set_fun, 0x0, sizeof(DNX_SAND_GROUP_MEM_LL_GROUP_ENTRY_SET));
  dnx_sand_os_memset(&info->group_get_fun, 0x0, sizeof(DNX_SAND_GROUP_MEM_LL_GROUP_ENTRY_GET));
  dnx_sand_os_memset(&info->member_set_fun, 0x0, sizeof(DNX_SAND_GROUP_MEM_LL_MEMBER_ENTRY_SET));
  dnx_sand_os_memset(&info->member_get_fun, 0x0, sizeof(DNX_SAND_GROUP_MEM_LL_MEMBER_ENTRY_GET)); 

  /* copy DS data */
  DNX_SAND_COPY_TO_BUFF_AND_INC(cur_ptr,gr_mem_info->group_members_data.groups,DNX_SAND_GROUP_MEM_LL_GROUP_ENTRY,gr_mem_info->nof_groups);
  DNX_SAND_COPY_TO_BUFF_AND_INC(cur_ptr,gr_mem_info->group_members_data.members,DNX_SAND_GROUP_MEM_LL_MEMBER_ENTRY,gr_mem_info->nof_elements);
  if (gr_mem_info->group_members_data.cache_enabled)
  {
    DNX_SAND_COPY_TO_BUFF_AND_INC(cur_ptr,gr_mem_info->group_members_data.groups_cache,DNX_SAND_GROUP_MEM_LL_GROUP_ENTRY,gr_mem_info->nof_groups);
    DNX_SAND_COPY_TO_BUFF_AND_INC(cur_ptr,gr_mem_info->group_members_data.members_cache,DNX_SAND_GROUP_MEM_LL_MEMBER_ENTRY,gr_mem_info->nof_elements);
  }
  
  *actual_size_bytes = total_size;

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in dnx_sand_group_mem_ll_save()",0,0);
}


uint32
  dnx_sand_group_mem_ll_load(
    DNX_SAND_IN  uint8                            **buffer,
    DNX_SAND_IN  DNX_SAND_GROUP_MEM_LL_GROUP_ENTRY_SET  group_set_fun,
    DNX_SAND_IN  DNX_SAND_GROUP_MEM_LL_GROUP_ENTRY_GET  group_get_fun,
    DNX_SAND_IN  DNX_SAND_GROUP_MEM_LL_MEMBER_ENTRY_SET member_set_fun,
    DNX_SAND_IN  DNX_SAND_GROUP_MEM_LL_MEMBER_ENTRY_GET member_get_fun,
    DNX_SAND_OUT DNX_SAND_GROUP_MEM_LL_INFO             *gr_mem_info
  )
{
  DNX_SAND_IN uint8
    *cur_ptr = (DNX_SAND_IN uint8*)buffer[0];
  uint32
    cur_size;
  uint32
    res;

  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);

  DNX_SAND_CHECK_NULL_INPUT(gr_mem_info);
  DNX_SAND_CHECK_NULL_INPUT(buffer);

  dnx_sand_SAND_GROUP_MEM_LL_INFO_clear(gr_mem_info);

  /* copy init info */
  dnx_sand_os_memcpy(gr_mem_info, cur_ptr,sizeof(DNX_SAND_GROUP_MEM_LL_INFO));
  cur_ptr += sizeof(DNX_SAND_GROUP_MEM_LL_INFO);
  gr_mem_info->group_set_fun = group_set_fun;
  gr_mem_info->group_get_fun = group_get_fun;
  gr_mem_info->member_set_fun = member_set_fun;
  gr_mem_info->member_get_fun = member_get_fun;
  /* create DS */
  res = dnx_sand_group_mem_ll_create(gr_mem_info);
  DNX_SAND_CHECK_FUNC_RESULT(res,20, exit);

  /* copy DS data */
  cur_size = sizeof(DNX_SAND_GROUP_MEM_LL_GROUP_ENTRY) * gr_mem_info->nof_groups;
  dnx_sand_os_memcpy(gr_mem_info->group_members_data.groups,cur_ptr, cur_size);
  cur_ptr += cur_size;

  cur_size = sizeof(DNX_SAND_GROUP_MEM_LL_MEMBER_ENTRY) * gr_mem_info->nof_elements;
  dnx_sand_os_memcpy(gr_mem_info->group_members_data.members,cur_ptr,cur_size);
  cur_ptr += cur_size;
  
  /* caching */
  if (gr_mem_info->group_members_data.cache_enabled)
  {
    cur_size = sizeof(DNX_SAND_GROUP_MEM_LL_GROUP_ENTRY) * gr_mem_info->nof_groups;
    dnx_sand_os_memcpy(gr_mem_info->group_members_data.groups_cache,cur_ptr, cur_size);
    cur_ptr += cur_size;

    cur_size = sizeof(DNX_SAND_GROUP_MEM_LL_MEMBER_ENTRY) * gr_mem_info->nof_elements;
    dnx_sand_os_memcpy(gr_mem_info->group_members_data.members_cache,cur_ptr,cur_size);
    cur_ptr += cur_size;
  }

  *buffer = cur_ptr;

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in dnx_sand_group_mem_ll_load()",0,0);
}


/*********************************************************************
* NAME:
*     dnx_sand_group_mem_ll_group_print
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
  )
{
  DNX_SAND_GROUP_MEM_LL_GROUP_ENTRY
    grp_entry;
  DNX_SAND_GROUP_MEM_LL_MEMBER_ENTRY
    member_entry;
  DNX_SAND_GROUP_MEM_LL_MEMBER_ID
    tmp_iter;
  uint32
    res;

  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(DNX_SAND_GROUP_MEM_LL_MEMBER_PRINT);

  DNX_SAND_CHECK_NULL_INPUT(gr_mem_info);

  if (group_ndx > gr_mem_info->nof_groups)
  {
    DNX_SAND_SET_ERROR_CODE(DNX_SAND_GROUP_MEM_LL_GROUP_ID_OUT_OF_RANGE_ERR, 10, exit);
  }

  res = gr_mem_info->group_get_fun(
          gr_mem_info->instance_prim_handle,
          DNX_SAND_GROUP_MEM_LL_ACTIVE_INST(gr_mem_info,gr_mem_info->instance_sec_handle),
          group_ndx,
          &grp_entry
        );
  DNX_SAND_CHECK_FUNC_RESULT(res, 20, exit);

  tmp_iter = grp_entry.first_member;
  LOG_CLI((BSL_META("%u: "), group_ndx));

  while (tmp_iter != DNX_SAND_GROUP_MEM_LL_END)
  {
    LOG_CLI((BSL_META("%u, "), tmp_iter));
    res = gr_mem_info->member_get_fun(
            gr_mem_info->instance_prim_handle,
            DNX_SAND_GROUP_MEM_LL_ACTIVE_INST(gr_mem_info,gr_mem_info->instance_sec_handle),
            tmp_iter,
            &member_entry
          );
    DNX_SAND_CHECK_FUNC_RESULT(res, 30, exit);
    if (member_entry.group_id == DNX_SAND_GROUP_MEM_LL_END)
    {
      goto exit;
    }

    tmp_iter = member_entry.next_member;
  }
    LOG_CLI((BSL_META("\n")));
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in dnx_sand_group_mem_ll_group_print()",0,0);
}




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
  )
{
  DNX_SAND_GROUP_MEM_LL_GROUP_ENTRY
    grp_entry;
  DNX_SAND_GROUP_MEM_LL_MEMBER_ENTRY
    member_entry;
  DNX_SAND_GROUP_MEM_LL_MEMBER_ID
    tmp_iter;
  uint32
    res;

  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(DNX_SAND_GROUP_MEM_LL_FUNC_RUN);

  DNX_SAND_CHECK_NULL_INPUT(gr_mem_info);
  DNX_SAND_CHECK_NULL_INPUT(func);


  if (group_ndx > gr_mem_info->nof_groups)
  {
    DNX_SAND_SET_ERROR_CODE(DNX_SAND_GROUP_MEM_LL_GROUP_ID_OUT_OF_RANGE_ERR, 10, exit);
  }

  res = gr_mem_info->group_get_fun(
          gr_mem_info->instance_prim_handle,
          DNX_SAND_GROUP_MEM_LL_ACTIVE_INST(gr_mem_info,gr_mem_info->instance_sec_handle),
          group_ndx,
          &grp_entry
        );
  DNX_SAND_CHECK_FUNC_RESULT(res, 20, exit);

  tmp_iter = grp_entry.first_member;

  while (tmp_iter != DNX_SAND_GROUP_MEM_LL_END)
  {
    res = gr_mem_info->member_get_fun(
            gr_mem_info->instance_prim_handle,
            DNX_SAND_GROUP_MEM_LL_ACTIVE_INST(gr_mem_info,gr_mem_info->instance_sec_handle),
            tmp_iter,
            &member_entry
          );
    DNX_SAND_CHECK_FUNC_RESULT(res, 30, exit);
    if (member_entry.group_id == DNX_SAND_GROUP_MEM_LL_END)
    {
      goto exit;
    }
    res = func(
            tmp_iter,
            param1,
            param2
            );
    DNX_SAND_CHECK_FUNC_RESULT(res, 40, exit);

    tmp_iter = member_entry.next_member;
  }

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in dnx_sand_group_mem_ll_group_print()",0,0);
}

 
uint32
  dnx_sand_group_mem_ll_func_run_pointer_param(
    DNX_SAND_INOUT    DNX_SAND_GROUP_MEM_LL_INFO                    *gr_mem_info,
    DNX_SAND_IN       DNX_SAND_GROUP_MEM_LL_GROUP_ID                group_ndx,
    DNX_SAND_IN       DNX_SAND_GROUP_MEM_LL_ITER_FUN_POINTER_PARAM  func,
    DNX_SAND_INOUT    void                                          *opaque
  )
{
  DNX_SAND_GROUP_MEM_LL_GROUP_ENTRY
    grp_entry;
  DNX_SAND_GROUP_MEM_LL_MEMBER_ENTRY
    member_entry;
  DNX_SAND_GROUP_MEM_LL_MEMBER_ID
    tmp_iter;
  uint32
    res;

  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(DNX_SAND_GROUP_MEM_LL_FUNC_RUN);

  DNX_SAND_CHECK_NULL_INPUT(gr_mem_info);
  DNX_SAND_CHECK_NULL_INPUT(func);


  if (group_ndx > gr_mem_info->nof_groups)
  {
    DNX_SAND_SET_ERROR_CODE(DNX_SAND_GROUP_MEM_LL_GROUP_ID_OUT_OF_RANGE_ERR, 10, exit);
  }

  res = gr_mem_info->group_get_fun(
          gr_mem_info->instance_prim_handle,
          DNX_SAND_GROUP_MEM_LL_ACTIVE_INST(gr_mem_info,gr_mem_info->instance_sec_handle),
          group_ndx,
          &grp_entry
        );
  DNX_SAND_CHECK_FUNC_RESULT(res, 20, exit);

  tmp_iter = grp_entry.first_member;

  while (tmp_iter != DNX_SAND_GROUP_MEM_LL_END)
  {
    res = gr_mem_info->member_get_fun(
            gr_mem_info->instance_prim_handle,
            DNX_SAND_GROUP_MEM_LL_ACTIVE_INST(gr_mem_info,gr_mem_info->instance_sec_handle),
            tmp_iter,
            &member_entry
          );
    DNX_SAND_CHECK_FUNC_RESULT(res, 30, exit);
    if (member_entry.group_id == DNX_SAND_GROUP_MEM_LL_END)
    {
      goto exit;
    }
    res = func(tmp_iter,opaque);
    DNX_SAND_CHECK_FUNC_RESULT(res, 40, exit);

    tmp_iter = member_entry.next_member;
  }

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in dnx_sand_group_mem_ll_func_run_pointer_param()",0,0);
}

void
  dnx_sand_SAND_GROUP_MEM_LL_INFO_clear(
    DNX_SAND_GROUP_MEM_LL_INFO *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  dnx_sand_os_memset(info, 0x0, sizeof(DNX_SAND_GROUP_MEM_LL_INFO));
  info->auto_remove   = TRUE;
  info->nof_elements  = 0;
  info->nof_groups    = 0;
  info->wb_var_index  = SOC_DNX_WB_ENGINE_VAR_NONE;
  DNX_SAND_MAGIC_NUM_SET;
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

#include <soc/dnx/legacy/SAND/Utils/sand_footer.h>

