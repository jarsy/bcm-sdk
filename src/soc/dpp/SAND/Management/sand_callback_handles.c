/* $Id: sand_callback_handles.c,v 1.7 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 * $
*/
#include <soc/dpp/SAND/Utils/sand_os_interface.h>
#include <soc/dpp/SAND/Utils/sand_delta_list.h>
#include <soc/dpp/SAND/Utils/sand_tcm.h>
#include <soc/dpp/SAND/Management/sand_chip_descriptors.h>
#include <soc/dpp/SAND/Management/sand_callback_handles.h>
#include <soc/dpp/SAND/Management/sand_error_code.h>
/* $Id: sand_callback_handles.c,v 1.7 Broadcom SDK $
 * Handles utils, for registering and unregistering all device services
 * (both polling and interrupts).
 * {
 */
/*
 * the handles are maintained using a linked list.
 */
  SOC_SAND_DELTA_LIST
    *Soc_sand_handles_list  = NULL ;
/*****************************************************
*NAME:
* soc_sand_handles_get_handles_list
*DATE:
* 19/NOV/2002
*FUNCTION:
*  gets the static Soc_sand_handles_list
*INPUT:
*  SOC_SAND_DIRECT:
*  SOC_SAND_INDIRECT:
*OUTPUT:
*  SOC_SAND_DIRECT:
*  SOC_SAND_INDIRECT:
*REMARKS:
*SEE ALSO:
*****************************************************/
SOC_SAND_DELTA_LIST
  *soc_sand_handles_get_handles_list(
    void
  )
{
  return Soc_sand_handles_list ;
}
/*****************************************************
*NAME:
* soc_sand_handels_init_handles
*DATE:
* 27/OCT/2002
*FUNCTION:
*  initilize handles list
*INPUT:
*  SOC_SAND_DIRECT:
*  SOC_SAND_INDIRECT:
*OUTPUT:
*  SOC_SAND_DIRECT:
*   long  -
*     Non zero if error
*  SOC_SAND_INDIRECT:
*REMARKS:
*SEE ALSO:
*****************************************************/
SOC_SAND_RET
  soc_sand_handles_init_handles(
    void
  )
{
  SOC_SAND_RET
    ex = SOC_SAND_OK ;
  /*
   */
  Soc_sand_handles_list = soc_sand_delta_list_create() ;
  if (!Soc_sand_handles_list)
  {
    ex = SOC_SAND_ERR ;
    goto exit ;
  }
  /*
   */
exit:
  SOC_SAND_ERROR_REPORT(ex,NULL,0,0,SOC_SAND_HANDLES_INIT_HANDLES,
        "General error in soc_sand_handles_init_handles()",0,0,0,0,0,0) ;
  return ex ;
}
/*****************************************************
*NAME:
* soc_sand_handles_shut_down_handles
*DATE:
* 31/OCT/2002
*FUNCTION:
*  destorys handles list
*INPUT:
*  SOC_SAND_DIRECT:
*  SOC_SAND_INDIRECT:
*OUTPUT:
*  SOC_SAND_DIRECT:
*   long  -
*     Non zero if error
*  SOC_SAND_INDIRECT:
*REMARKS:
* The method must be called on an empty list
*SEE ALSO:
*****************************************************/
SOC_SAND_RET
  soc_sand_handles_shut_down_handles(
    void
  )
{
  SOC_SAND_RET
    ex = SOC_SAND_OK ;
  /*
   */
  if (SOC_SAND_OK != soc_sand_delta_list_destroy(Soc_sand_handles_list) )
  {
    ex = SOC_SAND_ERR ;
    goto exit ;
  }
  /*
   */
exit:
  SOC_SAND_ERROR_REPORT(ex,NULL,0,0,SOC_SAND_HANDLES_SHUT_DOWN_HANDLES,
        "General error in soc_sand_handles_shut_down_handles()",0,0,0,0,0,0) ;
  return ex ;
}
/*****************************************************
*NAME:
* soc_sand_handles_register_handle
*DATE:
* 27/OCT/2002
*FUNCTION:
*  registers a new polling / interrupt service in the list
*INPUT:
*  SOC_SAND_DIRECT:
*    uint32  int_or_poll  -
*       1 if polling,
*       2 if interrupt
*       3 if both
*    uint32  unit    -
*       the device id this handle is for
*    uint32  proc_id      -
*       the callback procedure id
*    uint32 soc_sand_polling_handle,
*       the private soc_sand polling handle (as recieved from the TCM)
*    uint32 soc_sand_interrupt_handle,
*       the private soc_sand handle (as recieved from event registration)
*    uint32 *public_handle - pointer to load the allocated
*       handle to the user.
*  SOC_SAND_INDIRECT:
*OUTPUT:
*  SOC_SAND_DIRECT:
*    0 if error, callback handle otherwise
*  SOC_SAND_INDIRECT:
*REMARKS:
*   adds FE_CALLBACK_HANDLE to the linked list
*SEE ALSO:
*****************************************************/
SOC_SAND_RET
  soc_sand_handles_register_handle(
    uint32  int_or_poll,
    int  unit,
    uint32  proc_id,
    uint32 soc_sand_polling_handle,
    uint32 soc_sand_interrupt_handle,
    uint32 *public_handle
  )
{
  SOC_SAND_CALLBACK_HANDLE
    *item ;
  uint32
      err ;
  SOC_SAND_RET
    ex ;
  /*
   */
  item = NULL ;
  ex   = SOC_SAND_ERR ;
  err = 0 ;
  /*
   * list not initialized is an error
   */
  if (!Soc_sand_handles_list)
  {
    err = 1 ;
    goto exit ;
  }
  /*
   * allocate the new item
   */
  item = soc_sand_os_malloc( sizeof(SOC_SAND_CALLBACK_HANDLE) , "item handles_register_handle") ;
  if (!item)
  {
    err = 2 ;
    goto exit ;
  }
  item->int_or_poll           = int_or_poll ;
  item->unit             = unit ;
  item->proc_id               = proc_id ;
  item->soc_sand_polling_handle   = soc_sand_polling_handle ;
  item->soc_sand_interrupt_handle = soc_sand_interrupt_handle ;
  item->valid_word            = SOC_SAND_CALLBACK_HANDLE_VALID_WORD ;
  /*
   */
  if (SOC_SAND_OK != soc_sand_handles_delta_list_take_mutex())
  {
    err = 3 ;
    soc_sand_os_free(item) ;
    goto exit ;
  }
  else /* semaphore taken succesfully */
  {
    if (SOC_SAND_OK != soc_sand_delta_list_insert_d(Soc_sand_handles_list, 0, (void *)item))
    {
      err = 4 ;
      soc_sand_os_free(item) ;
      goto exit ;
    }
  }
  if (SOC_SAND_OK != soc_sand_handles_delta_list_give_mutex())
  {
    err = 5 ;
    goto exit ;
  }
  /*
   */
  *public_handle = PTR_TO_INT(item);
  ex = SOC_SAND_OK ;
  /*
   */
exit:
  SOC_SAND_ERROR_REPORT(ex,NULL,0,0,SOC_SAND_HANDLES_REGISTER_HANDLE,
        "General error in soc_sand_handles_register_handle()",err,0,0,0,0,0) ;
  return ex ;
}
/*****************************************************
*NAME:
* soc_sand_handles_unregister_handle
*DATE:
* 27/OCT/2002
*FUNCTION:
*  deletes a service (interrupt / polling) from its location,
*  and clear its item in the handles list.
*INPUT:
*  SOC_SAND_DIRECT:
*    uint32 fe_service_handle -
*       the handle that was returend from fe_register_handle()
*  SOC_SAND_INDIRECT:
*OUTPUT:
*  SOC_SAND_DIRECT:
*     Non zero if error
*  SOC_SAND_INDIRECT:
*REMARKS:
*   finds the item in the list.
*   if it is interrupt deletes the callback from the callback
*   array at the right chip descriptor.
*   if it is polling, deletes the item from the polling engine.
*   Mutex issues: only single mutex at a time, so no issue.
*SEE ALSO:
*****************************************************/
SOC_SAND_RET
  soc_sand_handles_unregister_handle(
    uint32 soc_sand_service_handle
  )
{
  SOC_SAND_CALLBACK_HANDLE
    *item ;
  SOC_SAND_RET
    ex ;
  uint32
      err ;
  ex = SOC_SAND_OK ;
  err = 0 ;
  /*
   * check that we have a valid handle list and mutex
   */
  if (soc_sand_delta_list_is_empty(Soc_sand_handles_list))
  {
    err = 1 ;
    ex = SOC_SAND_ERR ;
    goto exit ;
  }
  item  = (SOC_SAND_CALLBACK_HANDLE *)INT_TO_PTR(soc_sand_service_handle);
  /*
   * check that we got a valid item
   */
  if(!item || SOC_SAND_CALLBACK_HANDLE_VALID_WORD != item->valid_word)
  {
    err = 2 ;
    ex = SOC_SAND_ERR ;
    goto exit ;
  }
  /*
   */
  switch (item->int_or_poll)
  {
    case 1:
    {
      if (SOC_SAND_OK != soc_sand_tcm_unregister_polling_callback(item->soc_sand_polling_handle))
      {
        err = 3 ;
        ex = SOC_SAND_ERR ;
        goto exit ;
      }
      break ;
    }
    case 2:
    {
      if (SOC_SAND_OK != soc_sand_unregister_event_callback(item->soc_sand_interrupt_handle))
      {
        err = 4 ;
        ex = SOC_SAND_ERR ;
        goto exit ;
      }
      break ;
    }
    case 3:
    {
      if (SOC_SAND_OK != soc_sand_tcm_unregister_polling_callback(item->soc_sand_polling_handle))
      {
        err = 5 ;
        ex = SOC_SAND_ERR ;
      }
      if (SOC_SAND_OK != soc_sand_unregister_event_callback(item->soc_sand_interrupt_handle))
      {
        err = 6 ;
        ex = SOC_SAND_ERR ;
      }
      if (SOC_SAND_ERR == ex)
      {
        goto exit ;
      }
      break ;
    }
    default:
    {
      err = 7 ;
      ex = SOC_SAND_ERR ;
      goto exit ;
    }
  }
  /*
   * successfull removal of polling / interrupt callback.
   * now lets remove it from tha handle list
   */
  if (SOC_SAND_OK != soc_sand_handles_delta_list_take_mutex())
  {
    err = 8 ;
    ex = SOC_SAND_ERR ;
    goto exit ;
  }
  /*
   * handle list semaphore was taken succesfuly -> remove item from list
   * (marking it invalid before)
   */
  item->valid_word = 0 ;
  if (SOC_SAND_OK != soc_sand_delta_list_remove(Soc_sand_handles_list, (void *)item))
  {
    err = 9 ;
    ex = SOC_SAND_ERR ;
    goto exit ;
  }
  /*
   */
  if (SOC_SAND_OK != soc_sand_handles_delta_list_give_mutex())
  {
    err = 10 ;
    ex = SOC_SAND_ERR ;
    goto exit ;
  }
  ex = SOC_SAND_OK ;
exit:
  SOC_SAND_ERROR_REPORT(ex,NULL,0,0,SOC_SAND_HANDLES_UNREGISTER_HANDLE,
        "General error in soc_sand_handles_unregister_handle()",err,0,0,0,0,0) ;
  return ex ;
}
/*****************************************************
*NAME:
* soc_sand_handles_unregister_all_device_handles
*DATE:
* 27/OCT/2002
*FUNCTION:
*  deletes all registered services of a specific unit
*INPUT:
*  SOC_SAND_DIRECT:
*   uint32 unit -
*       the device to unregister
*  SOC_SAND_INDIRECT:
*OUTPUT:
*  SOC_SAND_DIRECT:
*     Non zero if error
*  SOC_SAND_INDIRECT:
*REMARKS:
*   goes over the linked list, and for each item:
*     - if it is of different unit continue to the next.
*     - if it is of our decvice - clear it from the interrupt
*       table, or from the callback engine according to its
*       int_or_poll flag.
*       Mutex issue - since mutex taking order must be the same
*       over all the code (delta list, device, rest of them),
*       it is the responsibility of the upper layer to take
*       them before (list and device).
*       Since they are reentrant by the same task, it's not
*       a problem that soc_sand_unregister_event_callback()
*       and soc_sand_unregister_polling_callback() takes the
*       same mutexes again.
*SEE ALSO:
*****************************************************/
SOC_SAND_RET
  soc_sand_handles_unregister_all_device_handles(
    int unit
  )
{
  SOC_SAND_CALLBACK_HANDLE
    *curr_item ;
  SOC_SAND_DELTA_LIST_NODE
    *curr_node,
    *del_node,
    *prev_node ;
  SOC_SAND_RET
    soc_sand_ret,
    ex ;
  uint32
      err ;
  /*
   */
  curr_item    = NULL ;
  curr_node    = NULL ;
  del_node     = NULL ;
  prev_node    = NULL ;
  ex = SOC_SAND_ERR ;
  err = 0 ;
  if (soc_sand_delta_list_is_empty(Soc_sand_handles_list))
  {
    ex = SOC_SAND_OK ;  /* empty list of handles is legal */
    goto exit ;
  }
  /*
   */
  if (SOC_SAND_OK != soc_sand_handles_delta_list_take_mutex())
  {
    err = 1 ;
    goto exit ;
  }
  /* list semaphore is taken, loop over the list
   * we apply a relaxed error checking, cause we don't
   * want to break out of the loop (due to error) before
   * list termination: we want to at least try and remove
   * every relevant handle
   */
  curr_node = Soc_sand_handles_list->head ;
  while(NULL != curr_node)
  {
    /*
     * get the handle out of each item
     */
    curr_item = (SOC_SAND_CALLBACK_HANDLE *)curr_node->data ;
    /*
     * if it belongs to the right unit
     */
    if (unit == curr_item->unit)
    {
      /*
       * delete the the list item
       */
      del_node  = curr_node ;
      curr_node = curr_node->next ;
      soc_sand_os_free(del_node) ;  /* delete the former curr_node */
      if (prev_node)
      {
        /* we were in the middle of the list */
        prev_node->next = curr_node ;
      }
      else
      {
        /* we were in the begining of the list */
        Soc_sand_handles_list->head = curr_node ;
      }
      /*
       * now delete the handle itself
       */
      switch (curr_item->int_or_poll)
      {
        case 1:
        {
          soc_sand_ret = soc_sand_tcm_unregister_polling_callback(curr_item->soc_sand_polling_handle) ;
          if (SOC_SAND_OK != soc_sand_ret)
          {
            err = 2 ;
            goto exit_semaphore ;
          }
          break ;
        }
        case 2:
        {
          soc_sand_ret = soc_sand_unregister_event_callback(curr_item->soc_sand_interrupt_handle) ;
          if (SOC_SAND_OK != soc_sand_ret)
          {
            err = 3 ;
            goto exit_semaphore ;
          }
          break ;
        }
        case 3:
        {
          soc_sand_ret = soc_sand_tcm_unregister_polling_callback(curr_item->soc_sand_polling_handle) ;
          if (SOC_SAND_OK != soc_sand_ret)
          {
            err = 4 ;
            goto exit_semaphore ;
          }
          soc_sand_ret = soc_sand_unregister_event_callback(curr_item->soc_sand_interrupt_handle) ;
          if (SOC_SAND_OK != soc_sand_ret)
          {
            err = 5 ;
            goto exit_semaphore ;
          }
          break ;
        }
        default:
        {
          break ;
        }
      }
      curr_item->valid_word = 0 ; /* just so no one looking at this item mistakes it to be valid */
      soc_sand_os_free(curr_item) ;
      Soc_sand_handles_list->stats.current_size  -- ;
      Soc_sand_handles_list->stats.no_of_removes ++ ;
    }
    else
    {
      /*
       * if it wasn't the correct device id, just continue the loop.
       */
      prev_node = curr_node ;
      curr_node = curr_node->next ;
    }
  }
exit_semaphore:
  if (SOC_SAND_OK != soc_sand_handles_delta_list_give_mutex())
  {
    err = 6 ;
    goto exit ;
  }
  ex = SOC_SAND_OK ;
exit:
  SOC_SAND_ERROR_REPORT(ex,NULL,0,0,SOC_SAND_HANDLES_UNREGISTER_ALL_DEVICE_HANDLES,
        "General error in soc_sand_handles_unregister_all_device_handles()",err,0,0,0,0,0) ;
  return ex ;
}
/*****************************************************
*NAME:
* soc_sand_handles_search_next_handle
*DATE:
* 19/NOV/2002
*FUNCTION:
*  search for the next matching handle
*INPUT:
*  SOC_SAND_DIRECT:
*    uint32        unit  -
*       the unit of the service to search
*    uint32        proc_id    -
*       the proc_id of the callback to search
*    SOC_SAND_CALLBACK_HANDLE  *current   -
*       the handle that is currently used
*  SOC_SAND_INDIRECT:
*OUTPUT:
*  SOC_SAND_DIRECT:
*     SOC_SAND_CALLBACK_HANDLE *  - address of the next callback handle
*                             in the list. NULL if none.
*  SOC_SAND_INDIRECT:
*REMARKS:
*   goes over the linked list, and for a matching item after current
*SEE ALSO:
*****************************************************/
SOC_SAND_CALLBACK_HANDLE
  *soc_sand_handles_search_next_handle(
    int          unit,
    uint32          proc_id,
    SOC_SAND_CALLBACK_HANDLE  *current
)
{
  SOC_SAND_CALLBACK_HANDLE
    *ex ;
  SOC_SAND_CALLBACK_HANDLE
    *curr_item ;
  SOC_SAND_DELTA_LIST_NODE
    *curr_node ;
  uint32
    current_found ;

  /*
   */
  ex            = NULL ;
  curr_item     = NULL ;
  curr_node     = NULL ;
  current_found = FALSE ;
  if (!current)
  {
    current_found = TRUE ;
  }
  /*
   */
  if (soc_sand_delta_list_is_empty(Soc_sand_handles_list))
  {
    /* empty list of handles is legal */
    goto exit ;
  }
  /*
   * Although upper layer should hold the list mutex througout the
   * iteration, the list mutex allows for multiple taking of the same task
   */
  if (SOC_SAND_OK != soc_sand_handles_delta_list_take_mutex())
  {
    goto exit ;
  }
  /* list semaphore is taken, loop over the list
   * looking for every relevant handle
   */
  curr_node = Soc_sand_handles_list->head ;
  while(NULL != curr_node)
  {
    /*
     * get the handle out of each item
     */
    curr_item = (SOC_SAND_CALLBACK_HANDLE *)curr_node->data ;
    /*
     * if it belongs to the right unit & proc_id
     */
    if (unit == curr_item->unit &&
        proc_id   == curr_item->proc_id)
    {
      if (current_found)
      {
        ex = curr_item ;
        goto exit_semaphore ;
      }
      else
      {
        if (curr_item == current)
        {
          current_found = TRUE ;
        }
      }
    }
    curr_node = curr_node->next ;
  }

exit_semaphore:
  if (SOC_SAND_OK != soc_sand_handles_delta_list_give_mutex())
  {
    ex = NULL ;
    goto exit ;
  }
  /*
   */
exit:
  return ex ;
}

/*****************************************************
*NAME
* soc_sand_handles_is_handle_exist
*TYPE:
*  PROC
*DATE:
*  25-Aug-03
*FUNCTION:
*  Check that a callback function exist or not on
*  a specific device.
*INPUT:
*  SOC_SAND_DIRECT:
*    SOC_SAND_IN  int  unit -
*       The unit of the service to search
*    SOC_SAND_IN  uint32  proc_id -
*       The proc_id of the callback to search
*    SOC_SAND_OUT uint32* callback_exist -
*      Pointer to 'uint32'.
*      TRUE loaded iff the 'proc_id' was found on the device.
*  SOC_SAND_INDIRECT:
*    None.
*OUTPUT:
*  SOC_SAND_DIRECT:
*    SOC_SAND_RET
*      Non zero if error
*  SOC_SAND_INDIRECT:
*    NON
*REMARKS:
*  None.
*SEE ALSO:
*****************************************************/
SOC_SAND_RET
  soc_sand_handles_is_handle_exist(
    SOC_SAND_IN  int  unit,
    SOC_SAND_IN  uint32  proc_id,
    SOC_SAND_OUT uint32* callback_exist
  )
{
  SOC_SAND_RET
    soc_sand_ret;
  SOC_SAND_CALLBACK_HANDLE
    *soc_sand_handle;

  soc_sand_ret = SOC_SAND_OK;

  if(callback_exist == NULL)
  {
    soc_sand_ret = SOC_SAND_NULL_POINTER_ERR;
    goto exit;
  }

  soc_sand_handle = NULL;
  *callback_exist = FALSE;

  /*
   * We may take the handles list mutex, cause it is the only mutex we take.
   * No dead-lock is possible.
   */
  if (SOC_SAND_OK != soc_sand_handles_delta_list_take_mutex() )
  {
    soc_sand_ret = SOC_SAND_SEM_TAKE_FAIL;
    goto exit;
  }
  /*
   * Iterate over the handles list, looking for a matching handle
   */
  soc_sand_handle = soc_sand_handles_search_next_handle(unit, proc_id, soc_sand_handle);
  if(NULL != soc_sand_handle)
  {
      if (SOC_SAND_OK != soc_sand_handles_delta_list_give_mutex())
      {
          soc_sand_ret = SOC_SAND_SEM_GIVE_FAIL;
      }
      *callback_exist = TRUE;
      goto exit;
  }
  if (SOC_SAND_OK != soc_sand_handles_delta_list_give_mutex())
  {
    soc_sand_ret = SOC_SAND_SEM_GIVE_FAIL;
    goto exit;
  }

exit:
  return soc_sand_ret;
}

/*****************************************************
*NAME
* soc_sand_handles_delta_list_take_mutex
* soc_sand_handles_delta_list_give_mutex
* soc_sand_handles_delta_list_get_owner
*TYPE:
*  PROC
*DATE:
*  25-Aug-03
*FUNCTIONS:
*  The Soc_sand_handles_list shouldn't be used outside this file
*  Those functions needed to call the delta list functions
*  with the Soc_sand_handles_list.
*INPUT:
*  SOC_SAND_DIRECT:
*    None
*  SOC_SAND_INDIRECT:
*    None.
*OUTPUT:
*  SOC_SAND_DIRECT:
*    SOC_SAND_RET
*      soc_sand_handles_delta_list_take_mutex
*      soc_sand_handles_delta_list_give_mutex
*         Non zero if error.
*      soc_sand_handles_delta_list_get_owner
*         Task owner.
*  SOC_SAND_INDIRECT:
*    NON
*REMARKS:
*  None.
*SEE ALSO:
*****************************************************/
SOC_SAND_RET
  soc_sand_handles_delta_list_take_mutex(
    void
  )
{
  return
    soc_sand_delta_list_take_mutex(
      Soc_sand_handles_list
    );
}

SOC_SAND_RET
  soc_sand_handles_delta_list_give_mutex(
    void
  )
{
  return
    soc_sand_delta_list_give_mutex(
      Soc_sand_handles_list
    );
}

sal_thread_t
  soc_sand_handles_delta_list_get_owner(
    void
  )
{
  return
    soc_sand_delta_list_get_owner(
      Soc_sand_handles_list
    );
}
/*
 * }
 * End of device serivecs handle utils
 */
