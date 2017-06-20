/* $Id: sand_tcm.c,v 1.12 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/
/* $Id: sand_tcm.c,v 1.12 Broadcom SDK $
 */


#include <shared/bsl.h>
#include <soc/dpp/drv.h>



#include <soc/dpp/SAND/Utils/sand_tcm.h>
#include <soc/dpp/SAND/Utils/sand_framework.h>
#include <soc/dpp/SAND/Utils/sand_os_interface.h>
#include <soc/dpp/SAND/Utils/sand_delta_list.h>
#include <soc/dpp/SAND/Utils/sand_bitstream.h>
#include <soc/dpp/SAND/Utils/sand_trace.h>
#include <soc/dpp/SAND/Utils/sand_integer_arithmetic.h>

#include <soc/dpp/SAND/SAND_FM/sand_chip_defines.h>

#include <soc/dpp/SAND/Management/sand_chip_descriptors.h>
#include <soc/dpp/SAND/Management/sand_general_params.h>
#include <soc/dpp/SAND/Management/sand_error_code.h>
/*
 */

#define DEBUG_TCM 0

#if SOC_SAND_DEBUG

#if DEBUG_TCM
/*
 * {
 */
  #include <stdio.h>
  typedef struct
  {
    uint32   time_signature ;
    uint32   item ;
    uint32   when ;
    uint32   func ;
    uint32   buffer ;
    uint32   size ;
    char            *str ;
  } SOC_TCM_DEBUG_STRUCT ;
  /*
   */
  #define SOC_TCM_DEBUG_TABLE_SIZE  50
  SOC_TCM_DEBUG_STRUCT  Tcm_debug_table[SOC_TCM_DEBUG_TABLE_SIZE] ;
  uint32      Tcm_debug_table_index = 0 ;
  /*
   */
  void soc_sand_tcm_debug_table_print(void)
  {
    uint32 jjj ;
    uint32 time_to_print ;
    /*
     */
    LOG_CLI((BSL_META("                        TCM debug table          \r\n"
"--------------------------------------------------------------------------------\r\n")));
    for (jjj=0 ; jjj<Tcm_debug_table_index ; ++jjj)
    {
      if(jjj>0)
      {
        time_to_print =
          Tcm_debug_table[jjj].time_signature -
            Tcm_debug_table[jjj-1].time_signature;
      }
      else
      {
        time_to_print = Tcm_debug_table[jjj].time_signature;
      }
      LOG_CLI((BSL_META("%4d: %6s: time(%3u): %5lu, item: 0x%lu8, occur: %ld, func: 0x%lu8\r\n"),
jjj+1, Tcm_debug_table[jjj].str,
               (Tcm_debug_table[jjj].time_signature - Tcm_debug_table[jjj-1].time_signature),
               time_to_print, Tcm_debug_table[jjj].item,
               Tcm_debug_table[jjj].when, Tcm_debug_table[jjj].func ));
    }
    soc_sand_tcm_debug_table_clear() ;
  }

  void soc_sand_tcm_debug_table_clear(void)
  {
    Tcm_debug_table_index = 0 ;
  }
/*
 * }
 */
#else /* DEBUG_TCM */
/*
 * {
 */

STATIC
  void
    soc_sand_tcm_debug_table(void)
{
  LOG_INFO(BSL_LS_SOC_COMMON,
           (BSL_META("define DEBUG_TCM to 1 in order to get tcm debuging\r\n")));
}

void
  soc_sand_tcm_debug_table_print(void)
{
  soc_sand_tcm_debug_table() ;
}
void soc_sand_tcm_debug_table_clear(void)
{
  soc_sand_tcm_debug_table() ;
}

/*
 * }
 */
#endif /* DEBUG_TCM */

#endif /* SOC_SAND_DEBUG */

/*
 * This the delta list used by the polling engine. Each callcak that requires
 * polling is entered into the list, into the right place, according to its
 * sleep interval. A designated task, pops the first item from the list,
 * call it action, and go back to sleep for the time remaining till next poll
 * action and so on. The list is never empty, because there is a default action
 * of serving interrupts.
 */
static
  SOC_SAND_DELTA_LIST
    *Soc_sand_callback_delta_list   = NULL ;

/*****************************************************
*NAME:
*   soc_sand_get_tcm_task_priority
*   soc_sand_set_tcm_task_priority
*   soc_sand_get_tcm_task_stack_size
*   soc_sand_set_tcm_task_stack_size
*DATE:
*   04/OCT/2002
*FUNCTION:
* get/set utils for the tcm task params
*INPUT:
*  SOC_SAND_DIRECT:
*  SOC_SAND_INDIRECT:
*OUTPUT:
*  SOC_SAND_DIRECT:
*  SOC_SAND_INDIRECT:
*SEE ALSO:
*****************************************************/
static
  uint32
    Soc_sand_tcm_task_priority              = SOC_SAND_TCM_DEFAULT_TASK_PRIORITY ;
/*
 */
static
  uint32
    Soc_sand_tcm_task_stack_size            = SOC_SAND_TCM_DEFAULT_TASK_BYTE_SIZE ;
/*
 */
uint32 soc_sand_tcm_get_task_priority( void )
{
  return Soc_sand_tcm_task_priority ;
}
/*
 */
SOC_SAND_RET soc_sand_tcm_set_task_priority( uint32 soc_tcmtask_priority )
{
  Soc_sand_tcm_task_priority = soc_tcmtask_priority ;
  return SOC_SAND_OK ;
}
/*
 */
uint32 soc_sand_tcm_get_task_stack_size( void )
{
  return Soc_sand_tcm_task_stack_size ;
}
/*
 */
SOC_SAND_RET soc_sand_tcm_set_task_stack_size( uint32 soc_tcmtask_stack_size )
{
  Soc_sand_tcm_task_stack_size = soc_tcmtask_stack_size ;
  return SOC_SAND_OK ;
}
/*
 * This is the mean of interacting with the interrupt handler
 * When an interrupt occures it sends a message to this queue.
 */
static
  void
    *Soc_sand_tcm_msg_queue = (void *)0 ;
/*
 * Send a message to TCM queue from a task that wishes
 * to load a new entry into 'delta list'.
 */
SOC_SAND_RET
  soc_sand_tcm_send_message_to_q_from_task(
    uint32 msg
  )
{
  SOC_SAND_RET
    ex = SOC_SAND_ERR ;
  uint32
    err = 0 ;
  if (!Soc_sand_tcm_msg_queue)
  {
    err = 1 ;
    goto exit ;
  }
  /*
   * Send normal priority message with timeout.
   */
  ex =
    soc_sand_os_msg_q_send(
      Soc_sand_tcm_msg_queue,(char *)&msg,sizeof(msg),
      SOC_TCM_MSG_Q_SEND_TIMEOUT,0) ;
  if (ex != SOC_SAND_OK)
  {
    err = 2 ;
    goto exit ;
  }
exit:
  SOC_SAND_ERROR_REPORT(ex,NULL,0,0,SOC_SAND_TCM_SEND_MESSAGE_TO_Q_FROM_TASK,
        "General error in soc_sand_tcm_send_message_to_q_from_task()",err,0,0,0,0,0) ;
  return ex ;
}
/*
 * Send a message to TCM queue from ISR that wishes
 * to wake TCM up for interrupt handling.
 */
SOC_SAND_RET
  soc_sand_tcm_send_message_to_q_from_int(
    uint32 msg
  )
{
  SOC_SAND_RET
    ex ;
  uint32
      err ;
#define DBG_MSG_Q_GUESS 0
#if DBG_MSG_Q_GUESS
  static uint32 Soc_sand_msg_q_debug_guess[2] = {0,0};
#endif
  ex = SOC_SAND_ERR ;
  err = 0 ;
  if (!Soc_sand_tcm_msg_queue)
  {
    err = 1 ;
    goto exit ;
  }
  /*
   * Send message with NO timeout
   * (no blocking for interrupt).
   */
  ex =
    soc_sand_os_msg_q_send(
      Soc_sand_tcm_msg_queue,(char *)&msg,sizeof(msg),
      SOC_SAND_NO_TIMEOUT,0) ;
  if (ex != SOC_SAND_OK)
  {
    err = 2 ;
    goto exit ;
  }
exit:
#if DBG_MSG_Q_GUESS
  if(err) Msg_q_debug_guess[err-1]++;
#endif
  SOC_SAND_ERROR_REPORT(ex,NULL,0,0,SOC_SAND_TCM_SEND_MESSAGE_TO_Q_FROM_INT,
        "General error in soc_sand_tcm_send_message_to_q_from_int()",err,0,0,0,0,0) ;
  return ex ;
}
/*
 * A flag indicating whether the callback engine is started. It actually holds
 * the task id as returned from task_spawn, and required by task_delete
 */
static sal_thread_t
    Soc_sand_tcm_started  = 0 ;
/*
 */

sal_thread_t
  soc_sand_tcm_get_is_started(
    void
  )
{
  return Soc_sand_tcm_started;
}
/*
 * The flag that handles enabling of TCM main loop at all,
 * allowing the user to hold ALL interrupts for a while
 * (should be handy when debigging - and TCM load is high)
 */
static uint32 Soc_sand_tcm_enable_polling_flag = FALSE;

/*
 */
SOC_SAND_RET
  soc_sand_tcm_set_enable_flag(
    uint32 enable_flag
  )
{
  Soc_sand_tcm_enable_polling_flag = enable_flag;
  return SOC_SAND_OK;
}
/*
 */
uint32
  soc_sand_tcm_get_enable_flag(
    void
  )
{
  return Soc_sand_tcm_enable_polling_flag;
}

/*
 * setting this flag to TRUE causes tcm task to exit, thus killing it
 */
static uint32
  Soc_sand_tcm_request_to_die_flag = FALSE;

/*
 */
SOC_SAND_RET
  soc_sand_tcm_set_request_to_die_flag(
    uint32 request_to_die_flag
  )
{
  Soc_sand_tcm_request_to_die_flag = request_to_die_flag;
  return SOC_SAND_OK;
}

uint32
  soc_sand_tcm_get_request_to_die_flag(
    void
  )
{
  return Soc_sand_tcm_request_to_die_flag;
}

/*
 */

/*****************************************************
*NAME
* soc_sand_tcm_callback_delta_list_take_mutex
* soc_sand_tcm_callback_delta_list_give_mutex
*TYPE:
*  PROC
*DATE:
*  25-Aug-03
*FUNCTIONS:
*  The Soc_sand_callback_delta_list shouldn't be used outside this file
*  Those functions needed to call the delta list functions
*  with the Soc_sand_callback_delta_list.
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
  soc_sand_tcm_callback_delta_list_take_mutex(
    void
  )
{
  return SOC_SAND_OK;
}

SOC_SAND_RET
  soc_sand_tcm_callback_delta_list_give_mutex(
    void
  )
{
  return SOC_SAND_OK;
}
/*
 */
/*****************************************************
*NAME:
*   soc_sand_tcm_callback_engine_interrupt_serving
*DATE:
*   27/AUG/2002
*FUNCTION:
* The default polling util reponsible for interrupt serving.
*INPUT:
*  SOC_SAND_DIRECT:
*  SOC_SAND_INDIRECT:
*    The Array of non-polling callbacks.
*OUTPUT:
*  SOC_SAND_DIRECT:
*    Returns the result of the called callback.
*  SOC_SAND_INDIRECT:
*    None.
*REMARKS:
*  This basic_func gets activated after the interrupt handler
*  sends a message to the TCM's message queue.
*  It checks on all registered chip's interrupts
*  bit_stream. If it is not 0, then it is time to serve
*  these interrupts, by calling regitered callbacks.
*  The routine is very simple now: go over the bit_stream, if
*  the bit is set, and the a function is registered - call it
*
*SEE ALSO: soc_sand_callback_engine_main
*****************************************************/
STATIC
  int
    soc_sand_tcm_callback_engine_interrupt_serving(
      int unit
      )
{
  uint32             iii, ex = SOC_SAND_OK ;
  uint32             *tmp_bitstream, tmp_mask_bitstream[MAX_INTERRUPT_CAUSE / SOC_SAND_NOF_BITS_IN_UINT32];
  struct soc_sand_tcm_callback_str  *callback_array ;
  SOC_SAND_IS_DEVICE_INTERRUPTS_MASKED are_ints_masked;
  SOC_SAND_GET_DEVICE_INTERRUPTS_MASK  ints_mask_get;
  SOC_SAND_IS_BIT_AUTO_CLEAR_FUNC_PTR  is_bit_ac;
  SOC_SAND_UNMASK_FUNC_PTR             unmask;
  /*
   * unit is valid ?
   */
  if ( !soc_sand_is_chip_descriptor_valid(unit) )
  {
    ex = 101 ;
    goto exit ;
  }
  /*
   * take list & device semaphore
   * It might seem strange to take the list mutex now,
   * but it is designed to prevent dead-locks:
   * The normal TCM polling loop takes the list mutex,
   * pops items which work on a device & therefore take
   * its mutex. so the order is list, device.
   * Other services (register / unregister for instance)
   * keeps this order, inorder not to dead-lock with the
   * TCM main loop.
   * But the interrupt serving, might have switched the order,
   * if we wouldn't take the list semaphore fits. How ?
   * It takes the device's mutex, and then takes the list
   * mutex (to register / unregister a service, for instance).
   * Since both device and list mutexes allow for multiple
   * taking by the same task, there is no problem for us
   * to take the list mutex right now, and prevent a possible
   * dead-lock systematically.
   */
  if (SOC_SAND_OK != soc_sand_tcm_callback_delta_list_take_mutex())
  {
    ex = 102;
    goto exit ;
  }
  if (SOC_SAND_OK != soc_sand_take_chip_descriptor_mutex(unit) )
  {
    ex = 103;
    goto exit_semaphore_delta_list;
  }
  /*
   * check that we're in level
   */
  are_ints_masked = soc_sand_get_chip_descriptor_is_device_interrupts_masked_func(unit);
  if (are_ints_masked != NULL && !are_ints_masked(unit))
  {
    ex = 104;
    goto exit_semaphore_chip_desc_and_delta_list;
  }
  if (!soc_sand_device_is_between_isr_to_tcm(unit))
  {
    ex = 105;
    goto exit_semaphore_chip_desc_and_delta_list;
  }
  /*
   * just to make the code shorter and easier to follow,
   * we hold a local reference to the bitstream
   * mask_bitstream is local to this method (on the stack)
   */
  tmp_bitstream = soc_sand_get_chip_descriptor_interrupt_bitstream(unit);
  ints_mask_get = soc_sand_chip_descriptor_get_interrupts_mask_func(unit);
  if (ints_mask_get != NULL)
  {
    ints_mask_get(unit, tmp_mask_bitstream);
  }

  /*
   * now, using shift of 1 bit at a time, mask that bit with the cause_id bit.
   * if it is set, and there is a callback registered for that interrupt,
   * call the callback.
   */
  callback_array = soc_sand_get_chip_descriptor_interrupt_callback_array(unit) ;
  if( !callback_array || !tmp_bitstream)
  {
    ex = 106 ;
    goto exit_semaphore_chip_desc_and_delta_list;
  }
  /*
   * loop over all causes, and call callbacks if registered
   */
  for (iii=0 ; iii<MAX_INTERRUPT_CAUSE ; ++iii)
  {
    /*
     * Now we can check on the bitstream and callback array guarded
     * from register / unregister_callback / unregister_device, etc
     */
    if ( soc_sand_bitstream_test_bit(tmp_mask_bitstream, iii) &&
         soc_sand_bitstream_test_bit(tmp_bitstream, iii)
       )
    {
      /*
       * No need to give the mutex cause it allows for multiple
       * access of the same task. The same mechanism as in the TCM
       */
      if ( callback_array[iii].func )
      {
        callback_array[iii].func(
          callback_array[iii].buffer,
          callback_array[iii].size
          ) ;
      }
      /*
       * cause been handeld, we can reset it
       */
      soc_sand_bitstream_reset_bit(tmp_bitstream, iii) ;
    }
    /*
     * and any way - if bit is not auto cleared - we have to reset it otherwise it will
     * never get cleared
     */
    is_bit_ac = soc_sand_get_chip_descriptor_is_bit_auto_clear_func(unit);
    if (is_bit_ac != NULL && !is_bit_ac(iii))
    {
      soc_sand_bitstream_reset_bit(tmp_bitstream, iii) ;
    }

    /*
     */
   }
  /*
   * OK, we're done serving interrupts,
   * we can unmask them back
   */
  unmask = soc_sand_get_chip_descriptor_unmask_func(unit);
  if (unmask != NULL && SOC_SAND_OK != unmask(unit, TRUE))
  {
    ex = 107 ;
    goto exit_semaphore_chip_desc_and_delta_list ;
  }
exit_semaphore_chip_desc_and_delta_list:
  /*
   * give device & list semaphores
   */
  if ( SOC_SAND_OK != soc_sand_give_chip_descriptor_mutex(unit) )
  {
    ex = 108 ;
    goto exit ;
  }
exit_semaphore_delta_list:
  if (SOC_SAND_OK != soc_sand_tcm_callback_delta_list_give_mutex() )
  {
    ex = 109 ;
    goto exit ;
  }
  /*
   */
exit:
  return ex ;
}
/*
 */
/*****************************************************
*NAME:
*   soc_sand_tcm_callback_engine_main
*DATE:
*   27/AUG/2002
*FUNCTION:
* The main method of the polling engine. Basically, it is an
* endless loop that pops items from the delta_list, call
* the job's callback method, and goes to sleep for the indicated
* time (in system ticks), and so on.
*INPUT:
*  SOC_SAND_DIRECT:
*  SOC_SAND_INDIRECT:
*    the delta list of callback items.
*OUTPUT:
*  SOC_SAND_DIRECT:
*    in case of error non-Zero. Other wise should never return.
*  SOC_SAND_INDIRECT:
*    manipulates the delta_list and calls registered callbacks
*    that probably have side affects.
*REMARKS:
*SEE ALSO: soc_sand_callback_engine_start (the spawner)
*****************************************************/
STATIC
  SOC_SAND_RET
    soc_sand_tcm_callback_engine_main(
      void
    )
{
  SOC_SAND_CALLBACK_LIST_ITEM *item ;
  uint32
      msg,
      sleep_period,
      ticks_before_msgq,
      ticks_before,
      ticks_after,
      ii,
      func_res,
      milisec_per_ticks_round_up;
  int
    err,
    recv_reason,
    num_msgs ;
  uint32
    delta_ticks ;
  int delta_ticks_positive;
  uint32 min_time_between_handling;
  min_time_between_handling = 0;
  err = 0 ;
  func_res = SOC_SAND_ERR ;

  soc_sand_tcm_set_request_to_die_flag(FALSE);

  milisec_per_ticks_round_up =
    SOC_SAND_DIV_ROUND_UP(1000,soc_sand_os_get_ticks_per_sec());
  /*
   * The delta list should have been initilized.
   */
  if (NULL == Soc_sand_callback_delta_list)
  {
    err = 1 ;
    goto exit ;
  }
  /*
   * insert / pop should be protected by semaphore
   */
  if (0 == Soc_sand_callback_delta_list->mutex_id)
  {
    err = 2 ;
    goto exit ;
  }
  /*
   * give our spawner a chanse to set the Soc_sand_tcm_started flag, after it spawned
   * us successfully.
   */
  ii = 0 ;
  while (0 == Soc_sand_tcm_started)
  {
    soc_sand_os_task_delay_milisec(milisec_per_ticks_round_up);
    if (ii++ > soc_sand_os_sys_clk_rate_get()  )
    {
      err = 3 ;
      goto exit ;
    }
  }
  /*
   * Endless loop that pops an item from the list, execute it,
   * inserts it back if necessary,
   * and go to sleep for the exact time the poped item tells it.
   */
  while(TRUE)
  {
    if(soc_sand_tcm_get_request_to_die_flag())
    {
      /* cleanup and exit task */
      soc_sand_tcm_set_enable_flag(FALSE);
      goto exit;
    }

    while( !soc_sand_tcm_get_enable_flag() )
    {
      if(soc_sand_tcm_get_request_to_die_flag())
      {
        /* cleanup and exit task */
        soc_sand_tcm_set_enable_flag(FALSE);
        goto exit;
      }
     /*
      * sleep for the min time between activation, before checking again
      */
      soc_sand_os_task_delay_milisec( milisec_per_ticks_round_up * soc_sand_general_get_min_time_between_tcm_activation() );
    }
    /*
     * lets check first that msg_q is not overflowed
     */
    num_msgs = soc_sand_os_msg_q_num_msgs(Soc_sand_tcm_msg_queue);
    /*
     */
    if( -1 == num_msgs || SOC_SAND_TCM_MSG_QUEUE_NUM_MSGS == num_msgs )
    {
      err = 4 ;
      goto exit ;
    }
    if (NULL == Soc_sand_callback_delta_list->head)
    {
      /* the list is empty */
      sleep_period = (uint32)soc_sand_general_get_min_time_between_tcm_activation() ;
    }
    else
    {
      sleep_period = soc_sand_delta_list_get_head_time(Soc_sand_callback_delta_list) ;
    }
    /*
     * if someone wakes us up before time (interrupt or new item at the head)
     * we need to account for the time already spent sleeping.
     */
    soc_sand_os_tick_get(&ticks_before_msgq);
    /*
     * now that we logged the time before waiting on the queue,
     * we can respect user's request regarding min_time_between_handling
     */
    soc_sand_os_task_delay_milisec(milisec_per_ticks_round_up * min_time_between_handling);
#if defined(VXWORKS)
    min_time_between_handling = soc_sand_general_get_min_time_between_tcm_activation();
#else
    min_time_between_handling = 0;
#endif /* VXWORKS */
    /*
     * now it's time for the actual waiting on the queue.
     */
    recv_reason  =
      soc_sand_os_msg_q_recv(
        Soc_sand_tcm_msg_queue,(unsigned char *)&msg,
        sizeof(msg),(long)sleep_period) ;
    /*
     * since our function call can take time, we mesure the time elapsed from
     * wake up time, till just before sleep, and substract it later from head time.
     */
    soc_sand_os_tick_get(&ticks_before) ;
    /*
     * 0 > recv_reason => Error.
     */
    if (0 > recv_reason)
    {
      err = 5 ;
      goto exit ;
    }
    if(0 == recv_reason)
    {
    /*
     * if we got in here it means that 0 == recv_reason, which can be due to 3 reasons:
     * 1. an interrupt occurred, which means that msg has the unit in it,
     * 2. another task entered something to the head (msg > SOC_SAND_MAX_DEVICE).
     *    In this case we just fix delta list timing. at the next loop, the request will be served.
     * 3. another task entered something to the 'errors' queue. In this case
     *    user supplied error handler (if any) is invoked.
     */
      if (SOC_SAND_MAX_DEVICE > msg)
      {
        if (SOC_SAND_OK != (err = soc_sand_tcm_callback_engine_interrupt_serving(msg)) )
        {
          goto exit;
        }
        min_time_between_handling = soc_sand_general_get_min_time_between_tcm_activation();
      }
      else if (soc_sand_is_errors_queue_msg(msg))
      {
        if (SOC_SAND_OK != soc_sand_unload_errors_queue())
        {
          err = 10;
          goto exit;
        }
      }
      /*
       * time after interrupt serving (or not - if just new polling item)
       */
      soc_sand_os_tick_get(&ticks_after) ;
      /*
       * We want to check whether the msg arrived while
       * the q was waiting for the head.
       * in that case, we must substract from the head also the
       * elapsed sleeping time.
       * new head, with time < original head time
       */
      if (SOC_SAND_INFINITE_TIMEOUT != sleep_period)
      {
        /*
         * the list was not empty, so we account the time
         * that passed from before sleeping till now, and
         * that is the time we substract from the head.
         */
        delta_ticks = ticks_after - ticks_before_msgq ;
        delta_ticks_positive =  ticks_after > ticks_before_msgq ;
      }
      else
      {
        /*
         * the list was empty (infinite sleep) so we just have
         * to substract the interrupt handling time (if any)
         */
        delta_ticks =  ticks_after - ticks_before ;
        delta_ticks_positive = ticks_after > ticks_before ;
      }
      /*
       */
      if (delta_ticks_positive)
      {
        if (SOC_SAND_OK != soc_sand_tcm_callback_delta_list_take_mutex() )
        {
          err = 11 ;
          goto exit ;
        }
        else
        {
          if (SOC_SAND_NEW_DELTA_MESSAGE == msg)
          {
            soc_sand_delta_list_decrease_time_from_second_item(
                Soc_sand_callback_delta_list,
                delta_ticks
            );
          }
          else
          {
            soc_sand_delta_list_decrease_time_from_head(
                Soc_sand_callback_delta_list,
                delta_ticks
            ) ;
          }
        }
        if (SOC_SAND_OK != soc_sand_tcm_callback_delta_list_give_mutex() )
        {
          err = 12 ;
          goto exit ;
        }
      }
    }
    /*
     * 0 < recv_reason => time out occured -> it's time to pop an item and take care of it.
     */
    if (0 < recv_reason || 0 == sleep_period)
    {
      if (SOC_SAND_OK != soc_sand_tcm_callback_delta_list_take_mutex() )
      {
        err = 6 ;
        goto exit ;
      }
      /*
       */
      item = (SOC_SAND_CALLBACK_LIST_ITEM *) soc_sand_delta_list_pop_d (Soc_sand_callback_delta_list) ;
      /*
       * NULL item means an empty list - this could happend, because although
       * the queue was not empty before waiting on it, someone might have deleted
       * all it's content in the meanwhile (for instance, removing a device,
       * deltets all it's services, and might clear the queue).
       * If it happends we got nothing to do but go back to sleep...
       */
      if (NULL == item)
      {
        /* empty delta list */
        if (SOC_SAND_OK != soc_sand_tcm_callback_delta_list_give_mutex() )
        {
          err = 7 ;
          goto exit ;
        }
        continue ;
      }
      /*
       * Just for our paranoia, lets check that this is a valid item
       */
      if (CALLBACK_ITEM_VALID_WORD != item->valid_word)
      {
        /* not a valid item */
        err = 8 ;
        goto exit ;
      }
      /*
       * Since items may register a NULL callback, we check on them before calling
       */
      if (NULL != item->func)
      {
        /*
         * func may need the list mutex again (to register / unregister a service)
         * but that's OK, since the list's mutex allow for multiple taking of the same task
         */
#if DEBUG_TCM
  if(SOC_TCM_DEBUG_TABLE_SIZE > Tcm_debug_table_index)
  {
    Tcm_debug_table[Tcm_debug_table_index].time_signature = soc_sand_os_get_time_micro() ;
    Tcm_debug_table[Tcm_debug_table_index].item   = (uint32)item ;
    Tcm_debug_table[Tcm_debug_table_index].func   = (uint32)item->func ;
    Tcm_debug_table[Tcm_debug_table_index].when   = item->when ;
    Tcm_debug_table[Tcm_debug_table_index].str    = "before" ;
    Tcm_debug_table_index++ ;
  }
#endif
        soc_sand_trace_add_entry(PTR_TO_INT(item->func), "A") ;
        func_res = item->func( item->buffer, item->size ) ;
        soc_sand_trace_add_entry(PTR_TO_INT(item->func), "B") ;
#if DEBUG_TCM
  if(SOC_TCM_DEBUG_TABLE_SIZE > Tcm_debug_table_index)
  {
    Tcm_debug_table[Tcm_debug_table_index].time_signature = soc_sand_os_get_time_micro() ;
    Tcm_debug_table[Tcm_debug_table_index].item   = (uint32)item ;
    Tcm_debug_table[Tcm_debug_table_index].func   = (uint32)item->func ;
    Tcm_debug_table[Tcm_debug_table_index].when   = item->when ;
    Tcm_debug_table[Tcm_debug_table_index].str    = "after " ;
    Tcm_debug_table_index++ ;
  }
#endif
        /* we have to mesure the time right now, before re-entering the item to the list,
         * because we want to update head time, if func wasted a lot of time.
         */
        soc_sand_os_tick_get(&ticks_after) ;
        /*
         */
        delta_ticks =  ticks_after - ticks_before ;
        if (ticks_after > ticks_before)
        {
            soc_sand_delta_list_decrease_time_from_head(
              Soc_sand_callback_delta_list,
              delta_ticks
            );
        }
      }
      min_time_between_handling = soc_sand_general_get_min_time_between_tcm_activation();
      /*
       * now, should we renter it ?
       */
      if( (SOC_SAND_POLLING != item->recurr) ||
          (SOC_SAND_OK != soc_sand_get_error_code_from_error_word(func_res) ) ||
          (item->marked_for_removal)
        )
      {
        /* if callback returned error, or it wasn't a polling request
         * to begin with, or if unregister maekd it for removal,
         * then it is time to say goodbye
         */
        item->valid_word = 0 ; /* making sure any one else looking at item sees an invalid CALLBACK_ITEM*/
        soc_sand_os_free(item->buffer);
        soc_sand_os_free(item);
        /*
         * we have to notify the user about the callback removal,
         * so he could deallocate its own buffers - we do it through
         * the common error handler.
         */
        if(SOC_SAND_OK != soc_sand_get_error_code_from_error_word(func_res) )
        { /* the case the callback was deleted due to an error */
          uint32 err_id;
          soc_sand_initialize_error_word(SOC_SAND_TCM_CALLBACK_ENGINE_MAIN,0,&err_id);
          soc_sand_set_error_code_into_error_word(SOC_SAND_TCM_MAIN_ERR_02,&err_id);
          soc_sand_set_severity_into_error_word(SOC_SAND_SVR_FTL,&err_id) ;
          soc_sand_invoke_user_error_handler(
            err_id, "Callback method retuned an error, so it is removed from TCM",
            PTR_TO_INT(&item), func_res, 0,0,0,0
          );
        }
        else
        { /* the case the callback was deleted 'normally' */
          uint32 err_id;
          soc_sand_initialize_error_word(SOC_SAND_TCM_CALLBACK_ENGINE_MAIN,0,&err_id);
          soc_sand_set_error_code_into_error_word(SOC_SAND_TCM_MAIN_ERR_03,&err_id);
          soc_sand_set_severity_into_error_word(SOC_SAND_SVR_MSG,&err_id);
          soc_sand_invoke_user_error_handler(
            err_id, "Callback method is removed from TCM (due to aging, or specific user request)",
            PTR_TO_INT(&item), func_res, 0,0,0,0
          );
        }
      }
      else
      {
        /*
         * OK, so it's a polling, but is it infifnite ?
         */
        if (SOC_SAND_INFINITE_POLLING > item->times)
        {
          item->times-- ;
        }
        if (0 < item->times)
        {
          soc_sand_delta_list_insert_d(Soc_sand_callback_delta_list, item->when, (void *)item) ;
        }
        else /* time is up */
        {
          item->valid_word = 0 ; /* making sure any one else looking at item sees an invalid CALLBACK_ITEM*/
          soc_sand_os_free(item->buffer) ;
          soc_sand_os_free(item) ;
        }
      }
      if (SOC_SAND_OK != soc_sand_tcm_callback_delta_list_give_mutex() )
      {
        err = 9 ;
        goto exit ;
      }
    }
  }
  /*
   */
exit:
  if (err)
  {
    /*
     * This code had better be changed to the standard exit while 'soc_sand_error_handler'
     * should do direct call to external user error collector (and, perhaps, other
     * required actions) for 'fatal' errors.
     */
    uint32
        err_id ;
    soc_sand_initialize_error_word(SOC_SAND_TCM_CALLBACK_ENGINE_MAIN,0,&err_id) ;
    soc_sand_set_error_code_into_error_word(SOC_SAND_TCM_MAIN_ERR_01,&err_id) ;
    soc_sand_set_severity_into_error_word(SOC_SAND_SVR_FTL,&err_id) ;
    soc_sand_invoke_user_error_handler(
          err_id,
          "Error reported in TCM task (see first parameter). No recovery path. Quit.",
          (uint32)err,
          0,0,0,0,0
          ) ;
  }
  Soc_sand_tcm_started = FALSE;
  return SOC_SAND_ERR ;
}
/*
 */
/*****************************************************
*NAME:
*   soc_sand_tcm_callback_engine_start
*DATE:
*   27/AUG/2002
*FUNCTION:
* The init call of the polling & callbacks engine.
* Zeros the globals, alocates and clears the data structures,
* ans spwan the polling task (engine).
*INPUT:
*  SOC_SAND_DIRECT:
*  SOC_SAND_INDIRECT:
*   un initialized globals and data structures.
*OUTPUT:
*  SOC_SAND_DIRECT:
*    Non-zero in case of an error.
*  SOC_SAND_INDIRECT:
*    allocates the List, and the default item, spswns
*    the polling task, and more.
*REMARKS:
*SEE ALSO:
*****************************************************/
SOC_SAND_RET
  soc_sand_tcm_callback_engine_start(
    void
  )
{
  SOC_SAND_RET
    ex = SOC_SAND_ERR ;
  unsigned
    err = 0 ;

  /*
   * create the delta list
   */
  Soc_sand_callback_delta_list = soc_sand_delta_list_create() ;
  if (!Soc_sand_callback_delta_list)
  {
    err = 1 ;
    goto exit ;
  }
  /*
   * create the 'wake up' message queue
   */
  Soc_sand_tcm_msg_queue =
    soc_sand_os_msg_q_create(SOC_SAND_TCM_MSG_QUEUE_NUM_MSGS,SOC_TCM_MSG_QUEUE_SIZE_MSG) ;
  if (!Soc_sand_tcm_msg_queue)
  {
    soc_sand_delta_list_destroy(Soc_sand_callback_delta_list) ;
    err = 2 ;
    goto exit ;
  }
  /*
   * start the TCM task
   */
  Soc_sand_tcm_started = soc_sand_os_task_spawn(
              "dTcmTask",
              (int)soc_sand_tcm_get_task_priority(),
              0,
              (int)soc_sand_tcm_get_task_stack_size(),
              (SOC_SAND_FUNC_PTR) soc_sand_tcm_callback_engine_main,
              0,0,0,0,0,0,0,0,0,0
            ) ;
  /*
   * check that task spawn did not fail
   */
  if (!Soc_sand_tcm_started)
  {
    soc_sand_os_msg_q_delete(Soc_sand_tcm_msg_queue) ;
    soc_sand_delta_list_destroy(Soc_sand_callback_delta_list) ;
    err = 3 ;
    goto exit ;
  }
  soc_sand_tcm_set_enable_flag(TRUE);
  ex = SOC_SAND_OK ;
exit:
  SOC_SAND_ERROR_REPORT(ex,NULL,0,0,SOC_SAND_TCM_CALLBACK_ENGINE_START,
        "General error in soc_sand_tcm_callback_engine_start()",err,0,0,0,0,0) ;
  return ex ;
}
/*
 */
/*****************************************************
*NAME:
*   soc_sand_tcm_callback_engine_stop
*DATE:
*   27/AUG/2002
*FUNCTION:
* shuts doen the polling & callbacks engine.
*INPUT:
*  SOC_SAND_DIRECT:
*  SOC_SAND_INDIRECT:
*   the allocated globals and data structures.
*OUTPUT:
*  SOC_SAND_DIRECT:
*    Non-zero in case of an error.
*  SOC_SAND_INDIRECT:
*    de-allocates the List, and the default item,
*    deletes the polling task, and more.
*REMARKS:
*SEE ALSO:
*****************************************************/
SOC_SAND_RET
  soc_sand_tcm_callback_engine_stop(
    void
  )
{
  SOC_SAND_RET
    ret = SOC_SAND_OK;
  SOC_SAND_CALLBACK_LIST_ITEM
    *item;
  uint32
    milisec_per_ticks_round_up = 0;
  uint32
    ex,
    no_err;

  soc_sand_initialize_error_word(0, 0, &ex);
  no_err = ex;

  /*
   */

  milisec_per_ticks_round_up =
    SOC_SAND_DIV_ROUND_UP(1000,soc_sand_os_get_ticks_per_sec());

  if (!Soc_sand_tcm_started)
  {
    goto exit ;
  }
  /*
   * empty the list
   */
  ret = soc_sand_tcm_callback_delta_list_take_mutex(
        );
  soc_sand_set_error_code_into_error_word(
    soc_sand_get_error_code_from_error_word(ret),
    &ex
  );
  if(ex != no_err)
  {
    goto exit;
  }

  while( NULL != (item = (SOC_SAND_CALLBACK_LIST_ITEM *) soc_sand_delta_list_pop_d (Soc_sand_callback_delta_list)) )
  {
    item->valid_word = 0 ;
    if(item->buffer)
    {
      soc_sand_os_free(item->buffer) ;
    }
    soc_sand_os_free(item) ;
  }
  /*
   * The is no need to release this mutex - it going to be deleted.
   * soc_sand_tcm_callback_delta_list_give_mutex() ;
   */
  ret = soc_sand_tcm_set_request_to_die_flag(
          TRUE
        );
  soc_sand_set_error_code_into_error_word(
    soc_sand_get_error_code_from_error_word(ret),
    &ex
  );
  if(ex != no_err)
  {
    goto exit_semaphore;
  }

  soc_sand_os_task_delay_milisec(
    2*milisec_per_ticks_round_up * soc_sand_general_get_min_time_between_tcm_activation()
  );

  if (Soc_sand_tcm_started)
  {
   /*
    * kill the task after all the list is empty.
    */
    ret = soc_sand_os_task_delete(
            Soc_sand_tcm_started
          ) ;
    soc_sand_set_error_code_into_error_word(
      soc_sand_get_error_code_from_error_word(ret),
      &ex
    );
    if(ex != no_err)
    {
      goto exit_semaphore;
    }
  }

  if (Soc_sand_tcm_msg_queue)
  {
    soc_sand_os_msg_q_delete(Soc_sand_tcm_msg_queue) ;
  }

  /*
   * free what remained from the list (head of list and mutex).
   */
  ret = soc_sand_delta_list_destroy(
          Soc_sand_callback_delta_list
        ) ;
  soc_sand_set_error_code_into_error_word(
    soc_sand_get_error_code_from_error_word(ret),
    &ex
  );
  if(ex != no_err)
  {
    goto exit_semaphore;
  }

  goto exit;

exit_semaphore:
  soc_sand_tcm_callback_delta_list_give_mutex();
exit:
  return ret;
}
/*****************************************************
*NAME:
*   soc_sand_tcm_register_polling_callback
*DATE:
*   27/AUG/2002
*FUNCTION:
* Registers a single callback.
*INPUT:
*  SOC_SAND_DIRECT:
*   SOC_SAND_TCM_CALLBACK           func    - the callback to call
*   uint32           *buffer - a buffer to pass to the callback
*                             This buffer should be allocated through 'soc_sand_os_malloc',
*                             by the caller of this function.
*                             It is the TCM responsibility to free this buffer.
*   uint32           size    - the buffer size
*   SOC_SAND_CALLBACK_CAUSE_ID  cause   - 0 => non-event, >0 => SOC_SAND_INT_XX
*   SOC_SAND_RECURRENCE         poll    - to poll or not to poll
*   uint32           poll_interval if polling - this is the time interval
*   uint32           poll_times  if polling - how many times (may be infinite)
*  SOC_SAND_INDIRECT:
*   Callback handlers array or the delta list.
*OUTPUT:
*  SOC_SAND_DIRECT:
*    Non-zero in case of an error.
*  SOC_SAND_INDIRECT:
*REMARKS:
*  There are 2 types of callbacks - event and poll, Event
*  callbacks (cause > 0) are a direct response the an
*  interrupt and therefore are registered at the
*  interrupt handlers array.Poll callbacks are deferred
*  function calls, with a frequency to call them.
*  This is implemented by the polling engine.
*SEE ALSO:
*****************************************************/
SOC_SAND_RET
  soc_sand_tcm_register_polling_callback(
    SOC_SAND_IN     SOC_SAND_TCM_CALLBACK func,
    SOC_SAND_INOUT  uint32           *buffer,
    SOC_SAND_IN     uint32           size,
    SOC_SAND_IN     SOC_SAND_RECURRENCE         poll,
    SOC_SAND_IN     uint32           poll_interval,
    SOC_SAND_IN     uint32           poll_times,
    SOC_SAND_INOUT  uint32           *handle
  )
{
  SOC_SAND_CALLBACK_LIST_ITEM
    *item ;
  int
    insert_at_head_flag ;
  SOC_SAND_RET
    ex = SOC_SAND_ERR ;
  uint32
    err = 0 ;
  if (!Soc_sand_callback_delta_list || !handle)
  {
    err = 1 ;
    goto exit ;
  }
  item = soc_sand_os_malloc(sizeof(SOC_SAND_CALLBACK_LIST_ITEM), "item soc_tcmregister_polling_callback") ;
  if (NULL == item)
  {
    err = 2 ;
    goto exit ;
  }
  item->recurr   = poll ;
  item->times    = poll_times ;
  item->when     = poll_interval ;
  item->func     = func ;
  item->buffer   = (uint32 *)buffer ;
  item->size     = size ;
  /*
   * unregister polling transaction (and only it) mark this field
   * if it doesn't find the item within the list, in order to signal
   * the TCM main loop that it is not supposed to be re inserted into the list
   */
  item->marked_for_removal  = FALSE ;
  /*
   * an indication that this place in memory (item) is really a CALLBACK_ITEM
   */
  item->valid_word = CALLBACK_ITEM_VALID_WORD ;
  /*
   */
  if (SOC_SAND_OK != soc_sand_tcm_callback_delta_list_take_mutex() )
  {
    err = 3 ;
    goto exit ;
  }
  /*
   * if at this phase if we inserting at the head, then we must send a message to the
   * message queue, otherwise the TCM task might wait for too long on the message queue.
   */
  insert_at_head_flag = FALSE ;
  if ( (soc_sand_delta_list_is_empty(Soc_sand_callback_delta_list)) ||
       (soc_sand_delta_list_get_head_time(Soc_sand_callback_delta_list) > poll_interval)
     )
  {
    insert_at_head_flag = TRUE;
  }
  /*
   * insert the item into the list
   */
  if (SOC_SAND_OK != (ex = soc_sand_delta_list_insert_d(Soc_sand_callback_delta_list, item->when, (void *)item)))
  {
    err = 4 ;
    goto exit_sempahore ;
  }
  /*
   * if item has been entered at head of list, send a message to the queue
   * (to make sure (a) an empty list is properly handled and (b) a message
   * with expiration time shorter than currently active is properly handled)
   */
  if (insert_at_head_flag)
  {
    insert_at_head_flag = FALSE;
    ex = soc_sand_tcm_send_message_to_q_from_task((uint32)(SOC_SAND_NEW_DELTA_MESSAGE)) ;
    if (SOC_SAND_OK != ex)
    {
      err = 5 ;
      goto exit_sempahore ;
    }
  }
  /*
   * that's it. we're done with the list
   */
  *handle = PTR_TO_INT(item);
  ex = SOC_SAND_OK ;

exit_sempahore:
  if (SOC_SAND_OK != (ex = soc_sand_tcm_callback_delta_list_give_mutex()))
  {
    err = 5 ;
    goto exit ;
  }
exit:
  SOC_SAND_ERROR_REPORT(ex,NULL,0,0,SOC_SAND_TCM_REGISTER_POLLING_CALLBACK,
        "General error in soc_sand_tcm_register_polling_callback()",err,0,0,0,0,0) ;
  return ex ;
}
/*
 */
/*****************************************************
*NAME:
*   soc_sand_tcm_unregister_polling_callback
*DATE:
*   27/AUG/2002
*FUNCTION:
* unRegisters a single callback.
*INPUT:
*  SOC_SAND_DIRECT:
*    SOC_SAND_IN     uint32            handle - void* of the item
*  SOC_SAND_INDIRECT:
*   Callback handlers array or the delta list.
*OUTPUT:
*  SOC_SAND_DIRECT:
*    Non-zero in case of an error.
*  SOC_SAND_INDIRECT:
*REMARKS:
*SEE ALSO:
*****************************************************/
SOC_SAND_RET
  soc_sand_tcm_unregister_polling_callback(
    SOC_SAND_IN     uint32            handle
  )
{
  SOC_SAND_CALLBACK_LIST_ITEM
    *item ;
  uint32
      *local_buf_ptr ;
  SOC_SAND_RET
    ex ;
  uint32
      err ;
  err = 0 ;
  item = (SOC_SAND_CALLBACK_LIST_ITEM *)INT_TO_PTR(handle);
  ex = SOC_SAND_ERR ;
  /*
   * make sure the list mutex was initilized, and that it's not a NULL item
   */
  if (!Soc_sand_callback_delta_list || !item)
  {
    err = 1 ;
    goto exit ;
  }
 /*
  * make sure the handle we got is valid
  * CALLBACK_ITEM_VALID_WORD is written into item->valid_word
  * in the registration process, and never changed afterwords
  */
  if (CALLBACK_ITEM_VALID_WORD != item->valid_word)
  {
    /* the handle we got is not a valid SOC_SAND_CALLBACK_LIST_ITEM */
    err = 2 ;
    goto exit ;
  }
  /*
   * Item is valid, lets remove it:
   */
  if (SOC_SAND_OK != soc_sand_tcm_callback_delta_list_take_mutex() )
  {
    /* failed to take the list mutex */
    err = 3 ;
    goto exit ;
  }
  /*
   * since we gonna delete item, we make a local copy of the item->buffer pointer,
   * so we could free it later as well.
   */
  local_buf_ptr    = item->buffer ;
  /*
   * lets make sure that if someone else takes a look at item pointer,
   * he won't see a valid CALLBACK_ITEM
   */
  item->valid_word = 0 ;
  if ( SOC_SAND_OK != (ex = soc_sand_delta_list_remove(Soc_sand_callback_delta_list, (void *)item)))
  {
    /*
     * bug - we made sure item is valid and is not popped out.
     */
    err = 5 ;
    goto exit_sempahore ;
  }
  if (SOC_SAND_OK != (ex = soc_sand_os_free(item)))
  {
    err = 6 ;
    goto exit_sempahore ;
  }
  /*
   * item was removed successfully, so let's free its buffer as well.
   */
  if (local_buf_ptr)
  {
    if(SOC_SAND_OK != (ex = soc_sand_os_free(local_buf_ptr)))
    {
      err = 7 ;
      goto exit_sempahore ;
    }
  }
  ex = SOC_SAND_OK ;

exit_sempahore:
  if (SOC_SAND_OK != (ex = soc_sand_tcm_callback_delta_list_give_mutex()))
  {
    err = 8 ;
    goto exit ;
  }

exit:
  SOC_SAND_ERROR_REPORT(ex,NULL,0,0,SOC_SAND_TCM_UNREGISTER_POLLING_CALLBACK,
        "General error in SOC_SAND_TCM_UNREGISTER_POLLING_CALLBACK()",err,0,0,0,0,0);
  if(SOC_SAND_OK == ex)
  {
    uint32 err_id;
    soc_sand_initialize_error_word(SOC_SAND_TCM_UNREGISTER_POLLING_CALLBACK,0,&err_id);
    soc_sand_set_error_code_into_error_word(SOC_SAND_TCM_MAIN_ERR_03,&err_id);
    soc_sand_set_severity_into_error_word(SOC_SAND_SVR_MSG,&err_id);
    soc_sand_invoke_user_error_handler(
      err_id, "Callback method was removed from TCM (due to specific user request)",
      handle, 0,0,0,0,0
    );
  }
  return ex ;
}

#if SOC_SAND_DEBUG

/*****************************************************
*NAME:
*   soc_sand_tcm_print_delta_list
*DATE:
*   15/JAN/2003
*FUNCTION:
* Prints the TCM delta list.
*INPUT:
*  SOC_SAND_DIRECT:
*    void
*  SOC_SAND_INDIRECT:
*   Soc_sand_callback_delta_list.
*OUTPUT:
*  SOC_SAND_DIRECT:
*    None..
*  SOC_SAND_INDIRECT:
*REMARKS:
*SEE ALSO:
*****************************************************/
void
  soc_sand_tcm_print_delta_list(
    void
  )
{
  LOG_CLI((BSL_META("Soc_sand_callback_delta_list:\r\n")));
  soc_sand_delta_list_print(Soc_sand_callback_delta_list) ;
}

/*****************************************************
*NAME:
*   soc_sand_tcm_general_status_print
*DATE:
*   21/MAY/2003
*FUNCTION:
* Prints the TCM info.
*INPUT:
*  SOC_SAND_DIRECT:
*    void
*  SOC_SAND_INDIRECT:
*   Soc_sand_callback_delta_list.
*OUTPUT:
*  SOC_SAND_DIRECT:
*    None..
*  SOC_SAND_INDIRECT:
*REMARKS:
*SEE ALSO:
*****************************************************/
void
  soc_sand_tcm_general_status_print(
    void
  )
{
  if (Soc_sand_callback_delta_list == NULL)
  {
    LOG_CLI((BSL_META("soc_sand_tcm_general_status_print(): do not have internal params (probably the driver was not started)\n\r")));
    goto exit;
  }
  LOG_CLI((BSL_META("TCM is %sstarted and polling is %senabled \r\n"),
((Soc_sand_tcm_started) ? "" : "not "),
           ((soc_sand_tcm_get_enable_flag()) ? "" : "not ")
           ));
  LOG_CLI((BSL_META("TCM info --  Delta List:\n\r")));
  soc_sand_delta_list_print_DELTA_LIST(Soc_sand_callback_delta_list);
  LOG_CLI((BSL_META("\n\r")));

exit:
  return;
}

#endif /* SOC_SAND_DEBUG */
