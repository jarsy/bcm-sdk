/* $Id: sand_delta_list.c,v 1.5 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/
/* $Id: sand_delta_list.c,v 1.5 Broadcom SDK $
 */


#include <shared/bsl.h>
#include <soc/dnx/legacy/drv.h>



#include <soc/dnx/legacy/SAND/Management/sand_error_code.h>
#include <soc/dnx/legacy/SAND/Utils/sand_delta_list.h>
#include <soc/dnx/legacy/SAND/Management/sand_chip_descriptors.h>
#include <soc/dnx/legacy/SAND/Utils/sand_os_interface.h>
#define PRINT_LIST 0

/*****************************************************
*NAME:
* dnx_sand_delta_list_print
*DATE:
* 27/OCT/2002
*FUNCTION:
*   goes over the list from head to tail and prints
*   it to screen using stdio dnx_sand_os_printf
*INPUT:
*  DNX_SAND_DIRECT:
*    DNX_SAND_IN  delta_list      *plist -
*     the delta list to print.
*  DNX_SAND_INDIRECT:
*OUTPUT:
*  DNX_SAND_DIRECT:
*  DNX_SAND_INDIRECT:
*    prints to screen
*REMARKS:
* used for debug puposes.
*SEE ALSO:
*****************************************************/
void
  dnx_sand_delta_list_print(
    DNX_SAND_DELTA_LIST  *plist
  )
{
  DNX_SAND_DELTA_LIST_NODE *iter ;
  int nof_item ;

  nof_item = 0 ;
  /*
   */
  LOG_CLI((BSL_META("  dnx_sand_delta_list_print():\r\n")));
  if (NULL == plist)
  {
    LOG_CLI((BSL_META("plist is NULL. \r\n")));
    goto exit ;
  }

  if (NULL == plist->head)
  {
    LOG_CLI((BSL_META("Empty list: \r\n")));
  }
  /*
   */
  LOG_CLI((BSL_META("List = 0x%p, head = 0x%p, head time = 0x%X and mutex = 0x%lX\r\n"),
plist, plist->head, (uint32)plist->head_time, (unsigned long)plist->mutex_id));
  /*
   */
  iter = plist->head ;
  while(NULL != iter)
  {
    LOG_CLI((BSL_META("iter = 0x%p, next = 0x%p data = 0x%X and delta time = 0x%X\r\n"),
iter, iter->next, PTR_TO_INT(iter->data), PTR_TO_INT(iter->num)));
    /*
     */
    iter = iter->next ;
    nof_item ++ ;
  }
  LOG_CLI((BSL_META("number of items in the list: %d\r\n"), nof_item));
  goto exit ;
  /*
   */
exit:
  return ;
}

/*****************************************************
*NAME:
* dnx_sand_delta_list_get_owner
*DATE:
* 30/OCT/2002
*FUNCTION:
*   Return the owner task ID.
*INPUT:
*  DNX_SAND_DIRECT:
*    DNX_SAND_INOUT  DNX_SAND_DELTA_LIST      *plist -
*     the delta list to check.
*  DNX_SAND_INDIRECT:
*OUTPUT:
*  DNX_SAND_DIRECT:
*     owner task ID
*     Zero when has no owner.
*  DNX_SAND_INDIRECT:
*REMARKS:
*SEE ALSO:
*****************************************************/
sal_thread_t
  dnx_sand_delta_list_get_owner(
    DNX_SAND_DELTA_LIST  *plist
  )
{
  if(!plist)
    return 0;
  else
    return plist->mutex_owner;
}

/*****************************************************
*NAME:
* dnx_sand_delta_list_is_empty
*DATE:
* 30/OCT/2002
*FUNCTION:
*   checks whether the delat list us empty
*INPUT:
*  DNX_SAND_DIRECT:
*    DNX_SAND_INOUT  DNX_SAND_DELTA_LIST      *plist -
*     the delta list to check.
*  DNX_SAND_INDIRECT:
*OUTPUT:
*  DNX_SAND_DIRECT:
*     TRUE   - list is empty
*     FALSE  - list is not empty
*  DNX_SAND_INDIRECT:
*REMARKS:
*SEE ALSO:
*****************************************************/
int
  dnx_sand_delta_list_is_empty(
    DNX_SAND_IN   DNX_SAND_DELTA_LIST      *plist
  )
{
  if (!plist || ! plist->head )
  {
    return TRUE ;
  }
  return FALSE ;
}
/*****************************************************
*NAME:
* dnx_sand_take_delta_list_mutex
* dnx_sand_give_delta_list_mutex
*DATE:
* 10/NOV/2002
*FUNCTION:
 * takes / gives the delta list semaphore.
*INPUT:
*  DNX_SAND_DIRECT:
*    DNX_SAND_INOUT  DNX_SAND_DELTA_LIST      *plist -
*     the delta list to take/give its semaphore.
*  DNX_SAND_INDIRECT:
*OUTPUT:
*  DNX_SAND_DIRECT:
*   Non-Zero in case of an error
*  DNX_SAND_INDIRECT:
*REMARKS:
* allows for multiple taking of the same task
*SEE ALSO:
*****************************************************/
DNX_SAND_RET
  dnx_sand_delta_list_take_mutex(
    DNX_SAND_INOUT   DNX_SAND_DELTA_LIST      *plist
  )
{
  DNX_SAND_RET
    ex ;
  uint32
    err = 0 ;
  /*
   * we get an error if it's a null list,
   * or a list with no mutex
   */
  if ( !plist || !plist->mutex_id )
  {
    ex = DNX_SAND_ERR ;
    err = 1 ;
    goto exit ;
  }

 /*
   * First lets check if the mutex is already ours.
   * In that case we only have to increament its counter
   * and return with OK
   */
  if (dnx_sand_os_get_current_tid() == plist->mutex_owner)
  {
    ex = DNX_SAND_OK ;
    plist->mutex_counter ++ ;
    goto exit ;
  }
  /*
   * if not, we have to wait till it's given back.
   * Notice, that we might not be the first in line,
   * and others might set ownership before us,
   * still it couldn't be us, so the above check
   * (owner_tid != my_tid) is still valid
   */
  if (DNX_SAND_OK != (ex = dnx_sand_os_mutex_take(plist->mutex_id, (long)DNX_SAND_INFINITE_TIMEOUT)))
  {
    err = 2 ;
    goto exit ;
  }
  /*
   * And now, after waiting the mutex is rightfully ours
   * (for the first time) we can set it's ownership and counter
   */
  plist->mutex_owner   = dnx_sand_os_get_current_tid() ;
  plist->mutex_counter = 1 ;
  ex = DNX_SAND_OK ;
exit:
  DNX_SAND_ERROR_REPORT(ex,NULL,0,0,DNX_SAND_DELTA_LIST_TAKE_MUTEX,
        "General error in dnx_sand_delta_list_take_mutex()",err,0,0,0,0,0) ;
  return ex ;
}
/*
 */
DNX_SAND_RET
  dnx_sand_delta_list_give_mutex(
    DNX_SAND_INOUT   DNX_SAND_DELTA_LIST      *plist
  )
{
  DNX_SAND_RET
    ex ;
  uint32
    err = 0 ;

  /*
   * we get an error if it's a null list,
   * or a list with no mutex
   */
  if ( !plist || !plist->mutex_id )
  {
    ex = DNX_SAND_ERR ;
    err = 1 ;
    goto exit ;
  }
  /*
   * First lets check if the mutex is not ours to give.
   * In that case it's an error
   */
  if(dnx_sand_os_get_current_tid() != plist->mutex_owner)
  {
    ex = DNX_SAND_ERR ;
    err = 2 ;
    goto exit ;
  }
  /*
   * if we got here, we are the owner of the mutex,
   * so lets decrease one from its counter, and if it
   * is zeroed, lets give the semaphore back.
   */
  plist->mutex_counter -- ;
  if(0 == plist->mutex_counter)
  {
    plist->mutex_owner = 0 ;
    if (DNX_SAND_OK != (ex = dnx_sand_os_mutex_give(plist->mutex_id)))
    {
      ex = DNX_SAND_ERR ;
      err = 3 ;
      goto exit ;
    }
  }
  ex = DNX_SAND_OK ;
exit:
  DNX_SAND_ERROR_REPORT(ex,NULL,0,0,DNX_SAND_DELTA_LIST_GIVE_MUTEX,
        "General error in dnx_sand_delta_list_give_mutex()",err,0,0,0,0,0) ;
  return ex ;
}
/*****************************************************
*NAME:
* dnx_sand_delta_list_get_head_time
*DATE:
* 27/AUG/2002
*FUNCTION:
*   gets the time to wait for the first item in the list
*   without removing it.
*INPUT:
*  DNX_SAND_DIRECT:
*    DNX_SAND_IN  DNX_SAND_DELTA_LIST      *plist,
*  DNX_SAND_INDIRECT:
*OUTPUT:
*  DNX_SAND_DIRECT:
*   the head_time (could be 0)
*   0 is also returned in case of an empty list.
*  DNX_SAND_INDIRECT:
*REMARKS:
*   No semaphore protection is needed, since this
*   method just read, and does not change anything.
*SEE ALSO:
*****************************************************/
uint32
  dnx_sand_delta_list_get_head_time(
    DNX_SAND_IN     DNX_SAND_DELTA_LIST      *plist
  )
{
  if (!plist || !plist->head)
  {
      return 0 ;
  }
  return plist->head_time ;
}
/*****************************************************
*NAME:
* dnx_sand_delta_list_get_statisics
*DATE:
* 31/OCT/2002
*FUNCTION:
* gets the delta list statistics structure
*INPUT:
*  DNX_SAND_DIRECT:
*    DNX_SAND_IN  DNX_SAND_DELTA_LIST      *plist,
*  DNX_SAND_INDIRECT:
*OUTPUT:
*  DNX_SAND_DIRECT:
*   The address of the statistics structure
*  DNX_SAND_INDIRECT:
*REMARKS:
*   No semaphore protection is needed, since this
*   method just read, and does not change anything.
*SEE ALSO:
*****************************************************/
DNX_SAND_DELTA_LIST_STATISTICS
  *dnx_sand_delta_list_get_statisics(
    DNX_SAND_INOUT  DNX_SAND_DELTA_LIST      *plist
  )
{
  if(plist)
 {
   return &(plist->stats) ;
 }
 else
 {
   return NULL ;
 }
}
/*****************************************************
*NAME:
* dnx_sand_delta_list_create
*DATE:
* 30/OCT/2002
*FUNCTION:
*   creates an empty list
*INPUT:
*  DNX_SAND_DIRECT:
*  DNX_SAND_INDIRECT:
*OUTPUT:
*  DNX_SAND_DIRECT:
*   NULL if error,
*   other wise, the address of a newly created
*   and initialized delta list.
*  DNX_SAND_INDIRECT:
*REMARKS:
*SEE ALSO:
*****************************************************/
/*
 */

DNX_SAND_DELTA_LIST
  *dnx_sand_delta_list_create(
  )
{
  DNX_SAND_DELTA_LIST      *plist ;
  /*
   */
  plist = NULL ;
  /*
   * allocate the new list
   */
  plist = (DNX_SAND_DELTA_LIST *)dnx_sand_os_malloc(sizeof(DNX_SAND_DELTA_LIST),"plist delta_list_create") ;
  if(!plist)
  {
    goto exit ;
  }
  /*
   * initialize it
   */
  plist->head       = NULL ;
  plist->head_time  = 0 ;
  plist->mutex_id   = 0 ;
  plist->mutex_id = dnx_sand_os_mutex_create() ;
  plist->mutex_counter        = 0 ;
  plist->mutex_owner          = 0 ;
  plist->stats.current_size   = 0 ;
  plist->stats.no_of_inserts  = 0 ;
  plist->stats.no_of_pops     = 0 ;
  plist->stats.no_of_removes  = 0 ;
  if(!plist->mutex_id)
  {
     dnx_sand_os_free(plist) ;
     plist = NULL ;
     goto exit ;
  }
  /*
   */
exit:
  return plist ;
}
/*****************************************************
*NAME:
* dnx_sand_delta_list_destroy
*DATE:
* 30/OCT/2002
*FUNCTION:
 * destroy an empty list
*INPUT:
*  DNX_SAND_DIRECT:
*  DNX_SAND_INDIRECT:
*OUTPUT:
*  DNX_SAND_DIRECT:
*   NULL if error,
*   other wise, the address of a newly created
*   and initialized delta list.
*  DNX_SAND_INDIRECT:
*REMARKS:
* you can call this method only on an empty list.
* the idea is that this list holds void*, therfore
* someone must free them before, calling remove or pop.
*SEE ALSO:
*****************************************************/
DNX_SAND_RET
  dnx_sand_delta_list_destroy(
    DNX_SAND_INOUT  DNX_SAND_DELTA_LIST      *plist
  )
{
  DNX_SAND_RET
    ex ;
  ex = DNX_SAND_ERR ;
  /*
   * error - NULL list, or non empty list
   */
  if (!plist || plist->head)
  {
    goto exit ;
  }
  /*
   */
  dnx_sand_os_mutex_delete(plist->mutex_id) ;
  plist->head_time = 0 ;
  plist->mutex_id  = 0 ;
  dnx_sand_os_free(plist) ;
  ex = DNX_SAND_OK ;
exit:
  DNX_SAND_ERROR_REPORT(ex,NULL,0,0,DNX_SAND_DELTA_LIST_DESTROY,
        "General error in dnx_sand_delta_list_destroy()",0,0,0,0,0,0) ;
  return ex ;
}
/*
 */
/*****************************************************
*NAME:
* dnx_sand_delta_list_insert_d
*DATE:
* 27/AUG/2002
*FUNCTION:
*   inserts an item to the list
*INPUT:
*  DNX_SAND_DIRECT:
*    DNX_SAND_INOUT  DNX_SAND_DELTA_LIST      *plist -
*     the delta list to insert the new item into.
*    DNX_SAND_IN     uint32    nominal_value  -
*     the value used to calculate the new item place
*     in the delta list.
*     More explanations about the delat list structure,
*     and the logic of insert can be found at this file header.
*    DNX_SAND_IN     void            *data  -
*     the data that this item holds.
*  DNX_SAND_INDIRECT:
*OUTPUT:
*  DNX_SAND_DIRECT:
*    Non-zero in case of an error
*  DNX_SAND_INDIRECT:
*    a new item is added to the list.
*REMARKS:
* The call should be protected by semaphore.
* Notice that the list does not support any timing services.
* Furthermore, nominal_value has no units within the list, and it
* is the list user's responsibility to give this meanings.
* For instance, milliseconds, microseconds, system ticks, etc.
* The list only maintain delta between items.
*SEE ALSO:
*****************************************************/
DNX_SAND_RET
  dnx_sand_delta_list_insert_d(
    DNX_SAND_INOUT  DNX_SAND_DELTA_LIST      *plist,
    DNX_SAND_IN     uint32    time,
    DNX_SAND_INOUT  void            *data
  )
{
  DNX_SAND_DELTA_LIST_NODE
    *p_new,
    *iter ;
  uint32
      sum ;
  DNX_SAND_RET
    ex = DNX_SAND_ERR ;
  uint32
    err = 0 ;
  if (NULL == plist)
  {
    err = 1 ;
    goto exit ;
  }
  /*
   * Allocating the new list item
   */
  p_new = dnx_sand_os_malloc(sizeof(DNX_SAND_DELTA_LIST_NODE), "delta_list_insert_d") ;
  if (p_new == NULL)
  {
    err = 2 ;
    goto exit ;
  }
  p_new->data  = (void *)data ;
  /*
   */
  sum = 0 ;
  if (time < plist->head_time)
  {
    /* enter before the head */
    p_new->num       = plist->head_time - time ;
    plist->head_time = time ;
    p_new->next      = plist->head ;
    plist->head      = p_new ;
  }
  else
  {
    /*
     * time > head_time, iterate over the list, and
     * break just when time > sum of all preseding times
     */
    sum += plist->head_time ;
    for (iter=plist->head ; iter!=NULL ; iter=iter->next)
    {
      if ( (sum += iter->num) > time) break ;
    }
    if(iter != NULL)
    {
     /*
      * we're somewhere in the middle
      * (maybe just after the head or just before the tail)
      */
      p_new->num  =  sum - time ;
      p_new->next =  iter->next ;
      iter->num  -=  p_new->num ;
      iter->next  =  p_new ;
    }
    else
    {
     /*
      * iter == NULL empty list, or after the tail
      */
      if (!plist->head)
      {
        /* empty list */
        p_new->num       = 0 ;
        p_new->next      = NULL ;
        plist->head      = p_new ;
        plist->head_time = time ;
      }
      else
      {
        /*
         * entering at the tail
         * we have to sum again, but this time stop before the last null,
         * in order to insert correctly at the tail
         */
        for (sum=plist->head_time, iter=plist->head ; iter->next!=NULL ; iter=iter->next)
        {
          sum += iter->num ;
        }
        p_new->num  = 0 ;
        p_new->next = NULL ;
        iter->num   = time - sum ;
        iter->next  = p_new ;
      }
    }
  }
  plist->stats.current_size  ++ ;
  plist->stats.no_of_inserts ++ ;
  ex = DNX_SAND_OK ;
exit:
#if PRINT_LIST
  LOG_INFO(BSL_LS_SOC_COMMON,
           (BSL_META("\r\nafter insert data = 0x%X and time = 0x%X\r\n"), (uint32)data, (uint32)time));
  LOG_INFO(BSL_LS_SOC_COMMON,
           (BSL_META("===============================================================================\r\n")));
  dnx_sand_delta_list_print(plist) ;
#endif
  DNX_SAND_ERROR_REPORT(ex,NULL,0,0,DNX_SAND_DELTA_LIST_INSERT_D,
        "General error in dnx_sand_delta_list_insert_d()",err,0,0,0,0,0) ;
  return ex ;
}
/*
 */
/*****************************************************
*NAME:
* dnx_sand_delta_list_pop_d
*DATE:
* 27/AUG/2002
*FUNCTION:
*   pops (remove) the first item from the list
*   and rteun its void *data field
*INPUT:
*  DNX_SAND_DIRECT:
*    DNX_SAND_INOUT  DNX_SAND_DELTA_LIST      *plist -
*     the list to wrok on.
*  DNX_SAND_INDIRECT:
*OUTPUT:
*  DNX_SAND_DIRECT:
*    NULL in case of an error, or empty list.
*    a void* of the data that the item holds, in case of success.
*  DNX_SAND_INDIRECT:
*    Aa item is popped out of the list
*REMARKS:
*    The call should be protected by a semaphore.
*    More explanations about the delat list structure,
*    and the logic of pop can be found at this file header.
*SEE ALSO:
*****************************************************/
void
  *dnx_sand_delta_list_pop_d(
    DNX_SAND_INOUT  DNX_SAND_DELTA_LIST      *plist
  )
{
  void *data ;
  DNX_SAND_DELTA_LIST_NODE *p_tmp ;
  /*
   */
  data = NULL ;
  if (NULL == plist)
  {
      goto exit ;
  }
  /*
   */
  if (NULL == plist->head)
  {
    /* empty list */
    data = NULL ;
    goto exit ;
  }
  /*
   */
  p_tmp = plist->head ;
  data  = plist->head->data ;
  plist->head_time = p_tmp->num ;
  plist->head = plist->head->next ;
  dnx_sand_os_free (p_tmp) ;
  plist->stats.current_size -- ;
  plist->stats.no_of_pops   ++ ;

  /*
   */
exit:
#if PRINT_LIST
  LOG_INFO(BSL_LS_SOC_COMMON,
           (BSL_META("\r\nafter pop data = 0x%X\r\n"), (uint32)data));
  LOG_INFO(BSL_LS_SOC_COMMON,
           (BSL_META("===============================================================================\r\n")));
  dnx_sand_delta_list_print(plist) ;
#endif
  return data ;
}
/*****************************************************
*NAME:
* dnx_sand_delta_list_decrease_time_from_head
*DATE:
* 27/AUG/2002
*FUNCTION:
*   decrease time from plist->head_time
*INPUT:
*  DNX_SAND_DIRECT:
*    DNX_SAND_INOUT  DNX_SAND_DELTA_LIST      *plist           -
*     the list to work on.
*    DNX_SAND_IN    uint32   time_to_substract
-
*     the time to substract from haed_time.
*  DNX_SAND_INDIRECT:
*OUTPUT:
*  DNX_SAND_DIRECT:
*    Non zero in case of an error.
*    Otherwise it's a success.
*  DNX_SAND_INDIRECT:
*    The head_time is updated
*REMARKS:
* the call should be protected by a semaphore.
* head_time holds (from the user's point of view) the time
* to wait for the first item to pop. This utility let the user
* update this time, in case he didn't wak up for the whole time.
*SEE ALSO:
*****************************************************/
DNX_SAND_RET
  dnx_sand_delta_list_decrease_time_from_head(
    DNX_SAND_INOUT DNX_SAND_DELTA_LIST       *plist,
    DNX_SAND_IN    uint32    time_to_substract

  )
{
  DNX_SAND_DELTA_LIST_NODE *p_curr;
  DNX_SAND_RET        ex;
  uint32   tmp_time_to_substract;
  /**/
  ex = DNX_SAND_ERR ;
  if(!plist || !plist->mutex_id)
  {
    goto exit ;
  }
  /**/
  ex = DNX_SAND_OK;
  if (plist->head_time > time_to_substract)
  {
    plist->head_time -= time_to_substract;
    goto exit;
  }
  /**/
  tmp_time_to_substract = time_to_substract - plist->head_time;
  plist->head_time = 0;
  p_curr = plist->head;
  while(p_curr != NULL && tmp_time_to_substract > 0)
  {
    if(p_curr->num > tmp_time_to_substract)
    {
      p_curr->num -= tmp_time_to_substract;
      goto exit;
    }
    /**/
    tmp_time_to_substract -= p_curr->num;
    p_curr->num = 0;
    p_curr = p_curr->next;
  }
  /**/
exit:
#if PRINT_LIST
  LOG_INFO(BSL_LS_SOC_COMMON,
           (BSL_META("\r\nafter decrease_time_from_head time to decrease = 0x%X and curent head time is 0x%X \r\n"),
(uint32)time_to_substract, (uint32)plist->head_time));
  LOG_INFO(BSL_LS_SOC_COMMON,
           (BSL_META("===============================================================================\r\n")));
  dnx_sand_delta_list_print(plist) ;
#endif
  DNX_SAND_ERROR_REPORT(ex,NULL,0,0,DNX_SAND_DELTA_LIST_DECREASE_TIME_FROM_HEAD,
        "General error in dnx_sand_delta_list_decrease_time_from_head()",0,0,0,0,0,0) ;
  return ex ;
}
/*****************************************************
*NAME:
* dnx_sand_delta_list_decrease_time_from_second_item
*DATE:
* 21/APR/2003
*FUNCTION:
*   decrease time from plist->head->num
*INPUT:
*  DNX_SAND_DIRECT:
*    DNX_SAND_INOUT  DNX_SAND_DELTA_LIST      *plist           -
*     the list to work on.
*    DNX_SAND_IN    uint32   time_to_substract -
*     the time to substract from plist->head->num.
*  DNX_SAND_INDIRECT:
*OUTPUT:
*  DNX_SAND_DIRECT:
*    Non zero in case of an error.
*    Otherwise it's a success.
*  DNX_SAND_INDIRECT:
*    The time is updated
*REMARKS:
* This method is to handle a very special use case of
* the list:
* a task waits head_time, but in the meanwhile another
* task enters something to the head of the list. now,
* this enter do not take into appriciation that the waiting
* task already waited few of the head_time, so when it
* wakes up it must update it - only starting at the second
* place, and not the first.
*SEE ALSO:
*****************************************************/
DNX_SAND_RET
  dnx_sand_delta_list_decrease_time_from_second_item(
    DNX_SAND_INOUT DNX_SAND_DELTA_LIST       *plist,
    DNX_SAND_INOUT  uint32   time_to_substract
  )
{
  DNX_SAND_DELTA_LIST_NODE *p_curr;
  DNX_SAND_RET        ex;
  /**/
  ex = DNX_SAND_OK ;
  if(!plist || !plist->mutex_id)
  {
    ex = DNX_SAND_ERR ;
    goto exit ;
  }
  /**/
  p_curr = plist->head;
  while(p_curr != NULL && time_to_substract > 0)
  {
    if(p_curr->num > time_to_substract)
    {
      p_curr->num -= time_to_substract;
      goto exit;
    }
    /**/
    time_to_substract -= p_curr->num;
    p_curr->num = 0;
    p_curr = p_curr->next;
  }
  /**/
exit:
#if PRINT_LIST
  LOG_INFO(BSL_LS_SOC_COMMON,
           (BSL_META("\r\nafter dnx_sand_delta_list_decrease_time_from_second_item time to decrease = 0x%X and curent head time is 0x%X \r\n"),
(uint32)time_to_substract, (uint32)plist->head_time));
  LOG_INFO(BSL_LS_SOC_COMMON,
           (BSL_META("===============================================================================\r\n")));
  dnx_sand_delta_list_print(plist) ;
#endif
  DNX_SAND_ERROR_REPORT(ex,NULL,0,0,DNX_SAND_DELTA_LIST_DECREASE_TIME_FROM_SECOND_ITEM,
        "General error in dnx_sand_delta_list_decrease_time_from_second_item()",0,0,0,0,0,0) ;
  return ex ;
}
/*****************************************************
*NAME:
* dnx_sand_delta_list_remove
*DATE:
* 08/OCT/2002
*FUNCTION:
*   removes an item from the list
*INPUT:
*  DNX_SAND_DIRECT:
*    DNX_SAND_IN    DNX_SAND_DELTA_LIST                             *plist,

*    DNX_SAND_IN    uint32                            handle

*  DNX_SAND_INDIRECT:
*OUTPUT:
*  DNX_SAND_DIRECT:
*    Non-zero in case of failure (item was not found)
*  DNX_SAND_INDIRECT:
*REMARKS:
*  This call should be protected by a semaphore.
*SEE ALSO:
*****************************************************/
DNX_SAND_RET
  dnx_sand_delta_list_remove(
    DNX_SAND_INOUT DNX_SAND_DELTA_LIST       *plist,
    DNX_SAND_IN    void             *data

  )
{
  DNX_SAND_DELTA_LIST_NODE *p_tmp, *p_prev ;
  DNX_SAND_RET
    ex ;
  uint32
    err = 0 ;
  ex = DNX_SAND_ERR ;
  if(!plist || !plist->head)
  {
    err = 1 ;
    goto exit ;
  }
  p_prev = NULL ;
  p_tmp  = plist->head ;
  while (NULL != p_tmp)
  {
    if (p_tmp->data == data)
    {
      if (NULL == p_prev)
      {
        /* remove from head */
        plist->head_time += p_tmp->num ;
        plist->head       = p_tmp->next ;
      }
      else
      {
        if (p_tmp->next)
        {
          /* p_tmp is in the middle of the list */
          p_prev->num += p_tmp->num ;
          p_prev->next = p_tmp->next ;
        }
        else
        {
         /* p_tmp was the tail (and now p_prev is going to be) */
          p_prev->num  = 0 ;
          p_prev->next = NULL ;
        }
      }
      break ;
    }
    p_prev = p_tmp ;
    p_tmp  = p_tmp->next ;
  }
  if(NULL != p_tmp)
  {
    /* something was found */
    dnx_sand_os_free(p_tmp) ;
    plist->stats.current_size  -- ;
    plist->stats.no_of_removes ++ ;
    ex = DNX_SAND_OK ;
    goto exit ;
  }
exit:
#if PRINT_LIST
  LOG_INFO(BSL_LS_SOC_COMMON,
           (BSL_META("\r\nafter remove data = 0x%X \r\n"), (uint32)data));
  LOG_INFO(BSL_LS_SOC_COMMON,
           (BSL_META("===============================================================================\r\n")));
  dnx_sand_delta_list_print(plist) ;
#endif
  DNX_SAND_ERROR_REPORT(ex,NULL,0,0,DNX_SAND_DELTA_LIST_REMOVE,
        "General error in dnx_sand_delta_list_remove()",err,0,0,0,0,0) ;
  return ex ;
}



#if DNX_SAND_DEBUG
/* { */

/*
 * Print utility of DNX_SAND_DELTA_LIST_STATISTICS
 */
void
  dnx_sand_delta_list_print_DELTA_LIST_STATISTICS(
    DNX_SAND_IN DNX_SAND_DELTA_LIST_STATISTICS* delta_list_statistics
  )
{
  if (NULL == delta_list_statistics)
  {
    LOG_CLI((BSL_META("dnx_sand_delta_list_print_DELTA_LIST_STATISTICS got NULL\n\r")));
    goto exit;
  }
  /*
   */
  LOG_CLI((BSL_META(" current_size = %u, no_of_inserts = %u,\n\r"
" no_of_pops = %u, no_of_removes = %u"),
           delta_list_statistics->current_size,
           delta_list_statistics->no_of_inserts,
           delta_list_statistics->no_of_pops,
           delta_list_statistics->no_of_removes
           ));
exit:
  return;
}

/*
 * Print utility of DNX_SAND_DELTA_LIST
 */
void
  dnx_sand_delta_list_print_DELTA_LIST(
    DNX_SAND_IN DNX_SAND_DELTA_LIST* delta_list
  )
{

  if (NULL == delta_list)
  {
    LOG_CLI((BSL_META("dnx_sand_delta_list_print_DELTA_LIST got NULL\n\r")));
    goto exit;
  }

  /*
   */
  LOG_CLI((BSL_META(" head= %08X, head_time= %u,\n\r"
                    " mutex_id = %p, mutex_owner = %p, mutex_counter = %u\n\r"),
           PTR_TO_INT(delta_list->head),
           delta_list->head_time,
           delta_list->mutex_id,
           delta_list->mutex_owner,
           delta_list->mutex_counter
           ));
  dnx_sand_delta_list_print_DELTA_LIST_STATISTICS(
    &(delta_list->stats)
  );

exit:
  return;
}

/* } */
#endif

