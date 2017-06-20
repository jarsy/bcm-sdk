/* $Id: sand_callback_handles.c,v 1.7 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 * $
*/
#include <soc/dnx/legacy/SAND/Utils/sand_os_interface.h>
#include <soc/dnx/legacy/SAND/Utils/sand_delta_list.h>
#include <soc/dnx/legacy/SAND/Utils/sand_tcm.h>
#include <soc/dnx/legacy/SAND/Management/sand_chip_descriptors.h>
#include <soc/dnx/legacy/SAND/Management/sand_callback_handles.h>
#include <soc/dnx/legacy/SAND/Management/sand_error_code.h>
/* $Id: sand_callback_handles.c,v 1.7 Broadcom SDK $
 * Handles utils, for registering and unregistering all device services
 * (both polling and interrupts).
 * {
 */
/*
 * the handles are maintained using a linked list.
 */
  DNX_SAND_DELTA_LIST
    *Dnx_soc_sand_handles_list  = NULL ;
/*****************************************************
*NAME:
* dnx_sand_handles_get_handles_list
*DATE:
* 19/NOV/2002
*FUNCTION:
*  gets the static Dnx_soc_sand_handles_list
*INPUT:
*  DNX_SAND_DIRECT:
*  DNX_SAND_INDIRECT:
*OUTPUT:
*  DNX_SAND_DIRECT:
*  DNX_SAND_INDIRECT:
*REMARKS:
*SEE ALSO:
*****************************************************/
DNX_SAND_DELTA_LIST
  *dnx_sand_handles_get_handles_list(
    void
  )
{
  return Dnx_soc_sand_handles_list ;
}
/*****************************************************
*NAME:
* dnx_sand_handels_init_handles
*DATE:
* 27/OCT/2002
*FUNCTION:
*  initilize handles list
*INPUT:
*  DNX_SAND_DIRECT:
*  DNX_SAND_INDIRECT:
*OUTPUT:
*  DNX_SAND_DIRECT:
*   long  -
*     Non zero if error
*  DNX_SAND_INDIRECT:
*REMARKS:
*SEE ALSO:
*****************************************************/
DNX_SAND_RET
  dnx_sand_handles_init_handles(
    void
  )
{
  DNX_SAND_RET
    ex = DNX_SAND_OK ;
  /*
   */
  Dnx_soc_sand_handles_list = dnx_sand_delta_list_create() ;
  if (!Dnx_soc_sand_handles_list)
  {
    ex = DNX_SAND_ERR ;
    goto exit ;
  }
  /*
   */
exit:
  DNX_SAND_ERROR_REPORT(ex,NULL,0,0,DNX_SAND_HANDLES_INIT_HANDLES,
        "General error in dnx_sand_handles_init_handles()",0,0,0,0,0,0) ;
  return ex ;
}
/*****************************************************
*NAME:
* dnx_sand_handles_shut_down_handles
*DATE:
* 31/OCT/2002
*FUNCTION:
*  destorys handles list
*INPUT:
*  DNX_SAND_DIRECT:
*  DNX_SAND_INDIRECT:
*OUTPUT:
*  DNX_SAND_DIRECT:
*   long  -
*     Non zero if error
*  DNX_SAND_INDIRECT:
*REMARKS:
* The method must be called on an empty list
*SEE ALSO:
*****************************************************/
DNX_SAND_RET
  dnx_sand_handles_shut_down_handles(
    void
  )
{
  DNX_SAND_RET
    ex = DNX_SAND_OK ;
  /*
   */
  if (DNX_SAND_OK != dnx_sand_delta_list_destroy(Dnx_soc_sand_handles_list) )
  {
    ex = DNX_SAND_ERR ;
    goto exit ;
  }
  /*
   */
exit:
  DNX_SAND_ERROR_REPORT(ex,NULL,0,0,DNX_SAND_HANDLES_SHUT_DOWN_HANDLES,
        "General error in dnx_sand_handles_shut_down_handles()",0,0,0,0,0,0) ;
  return ex ;
}
/*****************************************************
*NAME:
* dnx_sand_handles_register_handle
*DATE:
* 27/OCT/2002
*FUNCTION:
*  registers a new polling / interrupt service in the list
*INPUT:
*  DNX_SAND_DIRECT:
*    uint32  int_or_poll  -
*       1 if polling,
*       2 if interrupt
*       3 if both
*    uint32  unit    -
*       the device id this handle is for
*    uint32  proc_id      -
*       the callback procedure id
*    uint32 dnx_sand_polling_handle,
*       the private dnx_sand polling handle (as recieved from the TCM)
*    uint32 dnx_sand_interrupt_handle,
*       the private dnx_sand handle (as recieved from event registration)
*    uint32 *public_handle - pointer to load the allocated
*       handle to the user.
*  DNX_SAND_INDIRECT:
*OUTPUT:
*  DNX_SAND_DIRECT:
*    0 if error, callback handle otherwise
*  DNX_SAND_INDIRECT:
*REMARKS:
*   adds FE_CALLBACK_HANDLE to the linked list
*SEE ALSO:
*****************************************************/
DNX_SAND_RET
  dnx_sand_handles_register_handle(
    uint32  int_or_poll,
    int  unit,
    uint32  proc_id,
    uint32 dnx_sand_polling_handle,
    uint32 dnx_sand_interrupt_handle,
    uint32 *public_handle
  )
{
  DNX_SAND_CALLBACK_HANDLE
    *item ;
  uint32
      err ;
  DNX_SAND_RET
    ex ;
  /*
   */
  item = NULL ;
  ex   = DNX_SAND_ERR ;
  err = 0 ;
  /*
   * list not initialized is an error
   */
  if (!Dnx_soc_sand_handles_list)
  {
    err = 1 ;
    goto exit ;
  }
  /*
   * allocate the new item
   */
  item = dnx_sand_os_malloc( sizeof(DNX_SAND_CALLBACK_HANDLE) , "item handles_register_handle") ;
  if (!item)
  {
    err = 2 ;
    goto exit ;
  }
  item->int_or_poll           = int_or_poll ;
  item->unit             = unit ;
  item->proc_id               = proc_id ;
  item->dnx_sand_polling_handle   = dnx_sand_polling_handle ;
  item->dnx_sand_interrupt_handle = dnx_sand_interrupt_handle ;
  item->valid_word            = DNX_SAND_CALLBACK_HANDLE_VALID_WORD ;
  /*
   */
  if (DNX_SAND_OK != dnx_sand_handles_delta_list_take_mutex())
  {
    err = 3 ;
    dnx_sand_os_free(item) ;
    goto exit ;
  }
  else /* semaphore taken succesfully */
  {
    if (DNX_SAND_OK != dnx_sand_delta_list_insert_d(Dnx_soc_sand_handles_list, 0, (void *)item))
    {
      err = 4 ;
      dnx_sand_os_free(item) ;
      goto exit ;
    }
  }
  if (DNX_SAND_OK != dnx_sand_handles_delta_list_give_mutex())
  {
    err = 5 ;
    goto exit ;
  }
  /*
   */
  *public_handle = PTR_TO_INT(item);
  ex = DNX_SAND_OK ;
  /*
   */
exit:
  DNX_SAND_ERROR_REPORT(ex,NULL,0,0,DNX_SAND_HANDLES_REGISTER_HANDLE,
        "General error in dnx_sand_handles_register_handle()",err,0,0,0,0,0) ;
  return ex ;
}
/*****************************************************
*NAME:
* dnx_sand_handles_unregister_handle
*DATE:
* 27/OCT/2002
*FUNCTION:
*  deletes a service (interrupt / polling) from its location,
*  and clear its item in the handles list.
*INPUT:
*  DNX_SAND_DIRECT:
*    uint32 fe_service_handle -
*       the handle that was returend from fe_register_handle()
*  DNX_SAND_INDIRECT:
*OUTPUT:
*  DNX_SAND_DIRECT:
*     Non zero if error
*  DNX_SAND_INDIRECT:
*REMARKS:
*   finds the item in the list.
*   if it is interrupt deletes the callback from the callback
*   array at the right chip descriptor.
*   if it is polling, deletes the item from the polling engine.
*   Mutex issues: only single mutex at a time, so no issue.
*SEE ALSO:
*****************************************************/
DNX_SAND_RET
  dnx_sand_handles_unregister_handle(
    uint32 dnx_sand_service_handle
  )
{
  DNX_SAND_CALLBACK_HANDLE
    *item ;
  DNX_SAND_RET
    ex ;
  uint32
      err ;
  ex = DNX_SAND_OK ;
  err = 0 ;
  /*
   * check that we have a valid handle list and mutex
   */
  if (dnx_sand_delta_list_is_empty(Dnx_soc_sand_handles_list))
  {
    err = 1 ;
    ex = DNX_SAND_ERR ;
    goto exit ;
  }
  item  = (DNX_SAND_CALLBACK_HANDLE *)INT_TO_PTR(dnx_sand_service_handle);
  /*
   * check that we got a valid item
   */
  if(!item || DNX_SAND_CALLBACK_HANDLE_VALID_WORD != item->valid_word)
  {
    err = 2 ;
    ex = DNX_SAND_ERR ;
    goto exit ;
  }
  /*
   */
  switch (item->int_or_poll)
  {
    case 1:
    {
      if (DNX_SAND_OK != dnx_sand_tcm_unregister_polling_callback(item->dnx_sand_polling_handle))
      {
        err = 3 ;
        ex = DNX_SAND_ERR ;
        goto exit ;
      }
      break ;
    }
    case 2:
    {
      if (DNX_SAND_OK != dnx_sand_unregister_event_callback(item->dnx_sand_interrupt_handle))
      {
        err = 4 ;
        ex = DNX_SAND_ERR ;
        goto exit ;
      }
      break ;
    }
    case 3:
    {
      if (DNX_SAND_OK != dnx_sand_tcm_unregister_polling_callback(item->dnx_sand_polling_handle))
      {
        err = 5 ;
        ex = DNX_SAND_ERR ;
      }
      if (DNX_SAND_OK != dnx_sand_unregister_event_callback(item->dnx_sand_interrupt_handle))
      {
        err = 6 ;
        ex = DNX_SAND_ERR ;
      }
      if (DNX_SAND_ERR == ex)
      {
        goto exit ;
      }
      break ;
    }
    default:
    {
      err = 7 ;
      ex = DNX_SAND_ERR ;
      goto exit ;
    }
  }
  /*
   * successfull removal of polling / interrupt callback.
   * now lets remove it from tha handle list
   */
  if (DNX_SAND_OK != dnx_sand_handles_delta_list_take_mutex())
  {
    err = 8 ;
    ex = DNX_SAND_ERR ;
    goto exit ;
  }
  /*
   * handle list semaphore was taken succesfuly -> remove item from list
   * (marking it invalid before)
   */
  item->valid_word = 0 ;
  if (DNX_SAND_OK != dnx_sand_delta_list_remove(Dnx_soc_sand_handles_list, (void *)item))
  {
    err = 9 ;
    ex = DNX_SAND_ERR ;
    goto exit ;
  }
  /*
   */
  if (DNX_SAND_OK != dnx_sand_handles_delta_list_give_mutex())
  {
    err = 10 ;
    ex = DNX_SAND_ERR ;
    goto exit ;
  }
  ex = DNX_SAND_OK ;
exit:
  DNX_SAND_ERROR_REPORT(ex,NULL,0,0,DNX_SAND_HANDLES_UNREGISTER_HANDLE,
        "General error in dnx_sand_handles_unregister_handle()",err,0,0,0,0,0) ;
  return ex ;
}
/*****************************************************
*NAME:
* dnx_sand_handles_unregister_all_device_handles
*DATE:
* 27/OCT/2002
*FUNCTION:
*  deletes all registered services of a specific unit
*INPUT:
*  DNX_SAND_DIRECT:
*   uint32 unit -
*       the device to unregister
*  DNX_SAND_INDIRECT:
*OUTPUT:
*  DNX_SAND_DIRECT:
*     Non zero if error
*  DNX_SAND_INDIRECT:
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
*       a problem that dnx_sand_unregister_event_callback()
*       and dnx_sand_unregister_polling_callback() takes the
*       same mutexes again.
*SEE ALSO:
*****************************************************/
DNX_SAND_RET
  dnx_sand_handles_unregister_all_device_handles(
    int unit
  )
{
  DNX_SAND_CALLBACK_HANDLE
    *curr_item ;
  DNX_SAND_DELTA_LIST_NODE
    *curr_node,
    *del_node,
    *prev_node ;
  DNX_SAND_RET
    dnx_sand_ret,
    ex ;
  uint32
      err ;
  /*
   */
  curr_item    = NULL ;
  curr_node    = NULL ;
  del_node     = NULL ;
  prev_node    = NULL ;
  ex = DNX_SAND_ERR ;
  err = 0 ;
  if (dnx_sand_delta_list_is_empty(Dnx_soc_sand_handles_list))
  {
    ex = DNX_SAND_OK ;  /* empty list of handles is legal */
    goto exit ;
  }
  /*
   */
  if (DNX_SAND_OK != dnx_sand_handles_delta_list_take_mutex())
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
  curr_node = Dnx_soc_sand_handles_list->head ;
  while(NULL != curr_node)
  {
    /*
     * get the handle out of each item
     */
    curr_item = (DNX_SAND_CALLBACK_HANDLE *)curr_node->data ;
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
      dnx_sand_os_free(del_node) ;  /* delete the former curr_node */
      if (prev_node)
      {
        /* we were in the middle of the list */
        prev_node->next = curr_node ;
      }
      else
      {
        /* we were in the begining of the list */
        Dnx_soc_sand_handles_list->head = curr_node ;
      }
      /*
       * now delete the handle itself
       */
      switch (curr_item->int_or_poll)
      {
        case 1:
        {
          dnx_sand_ret = dnx_sand_tcm_unregister_polling_callback(curr_item->dnx_sand_polling_handle) ;
          if (DNX_SAND_OK != dnx_sand_ret)
          {
            err = 2 ;
            goto exit_semaphore ;
          }
          break ;
        }
        case 2:
        {
          dnx_sand_ret = dnx_sand_unregister_event_callback(curr_item->dnx_sand_interrupt_handle) ;
          if (DNX_SAND_OK != dnx_sand_ret)
          {
            err = 3 ;
            goto exit_semaphore ;
          }
          break ;
        }
        case 3:
        {
          dnx_sand_ret = dnx_sand_tcm_unregister_polling_callback(curr_item->dnx_sand_polling_handle) ;
          if (DNX_SAND_OK != dnx_sand_ret)
          {
            err = 4 ;
            goto exit_semaphore ;
          }
          dnx_sand_ret = dnx_sand_unregister_event_callback(curr_item->dnx_sand_interrupt_handle) ;
          if (DNX_SAND_OK != dnx_sand_ret)
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
      dnx_sand_os_free(curr_item) ;
      Dnx_soc_sand_handles_list->stats.current_size  -- ;
      Dnx_soc_sand_handles_list->stats.no_of_removes ++ ;
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
  if (DNX_SAND_OK != dnx_sand_handles_delta_list_give_mutex())
  {
    err = 6 ;
    goto exit ;
  }
  ex = DNX_SAND_OK ;
exit:
  DNX_SAND_ERROR_REPORT(ex,NULL,0,0,DNX_SAND_HANDLES_UNREGISTER_ALL_DEVICE_HANDLES,
        "General error in dnx_sand_handles_unregister_all_device_handles()",err,0,0,0,0,0) ;
  return ex ;
}
/*****************************************************
*NAME:
* dnx_sand_handles_search_next_handle
*DATE:
* 19/NOV/2002
*FUNCTION:
*  search for the next matching handle
*INPUT:
*  DNX_SAND_DIRECT:
*    uint32        unit  -
*       the unit of the service to search
*    uint32        proc_id    -
*       the proc_id of the callback to search
*    DNX_SAND_CALLBACK_HANDLE  *current   -
*       the handle that is currently used
*  DNX_SAND_INDIRECT:
*OUTPUT:
*  DNX_SAND_DIRECT:
*     DNX_SAND_CALLBACK_HANDLE *  - address of the next callback handle
*                             in the list. NULL if none.
*  DNX_SAND_INDIRECT:
*REMARKS:
*   goes over the linked list, and for a matching item after current
*SEE ALSO:
*****************************************************/
DNX_SAND_CALLBACK_HANDLE
  *dnx_sand_handles_search_next_handle(
    int          unit,
    uint32          proc_id,
    DNX_SAND_CALLBACK_HANDLE  *current
)
{
  DNX_SAND_CALLBACK_HANDLE
    *ex ;
  DNX_SAND_CALLBACK_HANDLE
    *curr_item ;
  DNX_SAND_DELTA_LIST_NODE
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
  if (dnx_sand_delta_list_is_empty(Dnx_soc_sand_handles_list))
  {
    /* empty list of handles is legal */
    goto exit ;
  }
  /*
   * Although upper layer should hold the list mutex througout the
   * iteration, the list mutex allows for multiple taking of the same task
   */
  if (DNX_SAND_OK != dnx_sand_handles_delta_list_take_mutex())
  {
    goto exit ;
  }
  /* list semaphore is taken, loop over the list
   * looking for every relevant handle
   */
  curr_node = Dnx_soc_sand_handles_list->head ;
  while(NULL != curr_node)
  {
    /*
     * get the handle out of each item
     */
    curr_item = (DNX_SAND_CALLBACK_HANDLE *)curr_node->data ;
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
  if (DNX_SAND_OK != dnx_sand_handles_delta_list_give_mutex())
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
* dnx_sand_handles_is_handle_exist
*TYPE:
*  PROC
*DATE:
*  25-Aug-03
*FUNCTION:
*  Check that a callback function exist or not on
*  a specific device.
*INPUT:
*  DNX_SAND_DIRECT:
*    DNX_SAND_IN  int  unit -
*       The unit of the service to search
*    DNX_SAND_IN  uint32  proc_id -
*       The proc_id of the callback to search
*    DNX_SAND_OUT uint32* callback_exist -
*      Pointer to 'uint32'.
*      TRUE loaded iff the 'proc_id' was found on the device.
*  DNX_SAND_INDIRECT:
*    None.
*OUTPUT:
*  DNX_SAND_DIRECT:
*    DNX_SAND_RET
*      Non zero if error
*  DNX_SAND_INDIRECT:
*    NON
*REMARKS:
*  None.
*SEE ALSO:
*****************************************************/
DNX_SAND_RET
  dnx_sand_handles_is_handle_exist(
    DNX_SAND_IN  int  unit,
    DNX_SAND_IN  uint32  proc_id,
    DNX_SAND_OUT uint32* callback_exist
  )
{
  DNX_SAND_RET
    dnx_sand_ret;
  DNX_SAND_CALLBACK_HANDLE
    *dnx_sand_handle;

  dnx_sand_ret = DNX_SAND_OK;

  if(callback_exist == NULL)
  {
    dnx_sand_ret = DNX_SAND_NULL_POINTER_ERR;
    goto exit;
  }

  dnx_sand_handle = NULL;
  *callback_exist = FALSE;

  /*
   * We may take the handles list mutex, cause it is the only mutex we take.
   * No dead-lock is possible.
   */
  if (DNX_SAND_OK != dnx_sand_handles_delta_list_take_mutex() )
  {
    dnx_sand_ret = DNX_SAND_SEM_TAKE_FAIL;
    goto exit;
  }
  /*
   * Iterate over the handles list, looking for a matching handle
   */
  dnx_sand_handle = dnx_sand_handles_search_next_handle(unit, proc_id, dnx_sand_handle);
  if(NULL != dnx_sand_handle)
  {
      if (DNX_SAND_OK != dnx_sand_handles_delta_list_give_mutex())
      {
          dnx_sand_ret = DNX_SAND_SEM_GIVE_FAIL;
      }
      *callback_exist = TRUE;
      goto exit;
  }
  if (DNX_SAND_OK != dnx_sand_handles_delta_list_give_mutex())
  {
    dnx_sand_ret = DNX_SAND_SEM_GIVE_FAIL;
    goto exit;
  }

exit:
  return dnx_sand_ret;
}

/*****************************************************
*NAME
* dnx_sand_handles_delta_list_take_mutex
* dnx_sand_handles_delta_list_give_mutex
* dnx_sand_handles_delta_list_get_owner
*TYPE:
*  PROC
*DATE:
*  25-Aug-03
*FUNCTIONS:
*  The Dnx_soc_sand_handles_list shouldn't be used outside this file
*  Those functions needed to call the delta list functions
*  with the Dnx_soc_sand_handles_list.
*INPUT:
*  DNX_SAND_DIRECT:
*    None
*  DNX_SAND_INDIRECT:
*    None.
*OUTPUT:
*  DNX_SAND_DIRECT:
*    DNX_SAND_RET
*      dnx_sand_handles_delta_list_take_mutex
*      dnx_sand_handles_delta_list_give_mutex
*         Non zero if error.
*      dnx_sand_handles_delta_list_get_owner
*         Task owner.
*  DNX_SAND_INDIRECT:
*    NON
*REMARKS:
*  None.
*SEE ALSO:
*****************************************************/
DNX_SAND_RET
  dnx_sand_handles_delta_list_take_mutex(
    void
  )
{
  return
    dnx_sand_delta_list_take_mutex(
      Dnx_soc_sand_handles_list
    );
}

DNX_SAND_RET
  dnx_sand_handles_delta_list_give_mutex(
    void
  )
{
  return
    dnx_sand_delta_list_give_mutex(
      Dnx_soc_sand_handles_list
    );
}

sal_thread_t
  dnx_sand_handles_delta_list_get_owner(
    void
  )
{
  return
    dnx_sand_delta_list_get_owner(
      Dnx_soc_sand_handles_list
    );
}
/*
 * }
 * End of device serivecs handle utils
 */
