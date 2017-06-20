/* $Id: sand_tcm.h,v 1.6 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/
#ifndef DNX_SAND_CALLBACKS_H
#define DNX_SAND_CALLBACKS_H
#ifdef  __cplusplus
extern "C" {
#endif

#include <soc/dnx/legacy/SAND/Utils/sand_framework.h>
#include <soc/dnx/legacy/SAND/Utils/sand_delta_list.h>

/* $Id: sand_tcm.h,v 1.6 Broadcom SDK $
 *  The TCM task default stack size.
 *  The run time value may be set/test with
 *    dnx_sand_tcm_get_task_stack_size()
 *    dnx_sand_tcm_set_task_stack_size()
 */
#define DNX_SAND_TCM_DEFAULT_TASK_BYTE_SIZE   50*1024
/*
 *  The TCM task default task priority.
 *  The run time value may be set/test with
 *    dnx_sand_tcm_get_task_priority()
 *    dnx_sand_tcm_set_task_priority()
 */
#define DNX_SAND_TCM_DEFAULT_TASK_PRIORITY    70
/*
 * Maximal number of messages in 'wake up' queue
 * of TCM task. Make sure to allow one entry
 * for interrupt (for each device), one for tasks registering
 * (for each device) deferred services and one for system error handling.
 * we multiply this number by 2 - the serve as a threshold for
 * for myaterious queue overloading. the tcm, before reading
 * asks what's the queue size - and if it is DNX_SAND_TCM_MSG_QUEUE_NUM_MSGS,
 * something bad happend - we are pumped with too much messages.
 */
#define DNX_SAND_TCM_MSG_QUEUE_NUM_MSGS   (2 * (DNX_SAND_MAX_DEVICE + DNX_SAND_MAX_DEVICE + 1))
/*
 *  Number of system ticks to wait for 'wake up' queue
 *  of TCM task to have a free entry before returning
 *  with error to the caller.
 */
#define SOC_TCM_MSG_Q_SEND_TIMEOUT         20
/*
 * Size of one message in 'wake up' queue
 *  of TCM task.
 */
#define SOC_TCM_MSG_QUEUE_SIZE_MSG  sizeof(uint32)
/*
 * Types of messages which may be queued on the TCM queue:
 * Senders may be:
 * Interrupt Service Routine
 * Task loading new entry into delta list
 * Task indicating a new error/event has been loaded
 *   into system wide errors queue.
 * Interrupt service routine simply loads 'device id'.
 */
#define DNX_SAND_NEW_DELTA_MESSAGE             (DNX_SAND_MAX_DEVICE+1)
#define DNX_SAND_NEW_ERROR_MESSAGE             (DNX_SAND_MAX_DEVICE+2)
typedef
  uint32 (* DNX_SAND_TCM_CALLBACK)(
               uint32 *buffer,
               uint32 size
  ) ;
/*
 */
typedef struct dnx_sand_tcm_callback_str
{
  DNX_SAND_TCM_CALLBACK   func ;
  uint32       *buffer ;
  uint32       size ;
} dnx_sand_tcm_callback_str_t;
/*
 */
typedef enum
{
  DNX_SAND_ONCE,
  DNX_SAND_POLLING
} DNX_SAND_RECURRENCE ;
/*
 */
#define DNX_SAND_INFINITE_POLLING     0xffffffff
/*
 */
#define CALLBACK_ITEM_VALID_WORD  0xA5A5A5A5
/*
 */
typedef struct
{
  uint32     valid_word ;
  uint32     when ;
  DNX_SAND_TCM_CALLBACK func ;
  uint32     *buffer ;
  uint32     size ;
  DNX_SAND_RECURRENCE   recurr ;
  uint32     times ;
  uint32      marked_for_removal ;
} DNX_SAND_CALLBACK_LIST_ITEM ;

uint32
  dnx_sand_tcm_get_task_priority(
    void
    ) ;
/*
 */

sal_thread_t
  dnx_sand_tcm_get_is_started(
  void
  );
/*
 */
DNX_SAND_RET
  dnx_sand_tcm_set_task_priority(
    uint32 soc_tcmtask_priority
  );
/*
 */
uint32
  dnx_sand_tcm_get_task_stack_size(
    void
  ) ;
/*
 */
DNX_SAND_RET
  dnx_sand_tcm_set_task_stack_size(
    uint32 soc_tcmtask_stack_size
  ) ;
/*
 */
DNX_SAND_RET
  dnx_sand_tcm_send_message_to_q_from_task(
    uint32 msg
  ) ;
/*
 */
DNX_SAND_RET
  dnx_sand_tcm_send_message_to_q_from_int(
    uint32 msg
  );
/*
 */
DNX_SAND_RET
  dnx_sand_tcm_callback_engine_start(
    void
  ) ;
/*
 */
DNX_SAND_RET
  dnx_sand_tcm_callback_engine_stop(
    void
  ) ;
/*
 */
DNX_SAND_RET
  dnx_sand_tcm_set_enable_flag(
    uint32 enable_flag
  );
uint32
  dnx_sand_tcm_get_enable_flag(
    void
  );

/*
 */
DNX_SAND_RET
  dnx_sand_tcm_set_request_to_die_flag(
    uint32 request_to_die_flag
  );
uint32
  dnx_sand_tcm_get_request_to_die_flag(
    void
  );
/*
 */
DNX_SAND_RET
  dnx_sand_tcm_register_polling_callback(
    DNX_SAND_IN     DNX_SAND_TCM_CALLBACK      func,  /* callback to call*/
    DNX_SAND_INOUT  uint32      *buffer,/* buffer to pass to the callback */
    DNX_SAND_IN     uint32      size,  /* buffer size */
    DNX_SAND_IN     DNX_SAND_RECURRENCE      poll,  /* to poll or not to poll */
    DNX_SAND_IN     uint32      poll_interval, /* if polling - this is the time interval */
    DNX_SAND_IN     uint32      poll_times,  /* if polling - how many times */
    DNX_SAND_INOUT  uint32           *handle
  ) ;
/*
 */
DNX_SAND_RET
  dnx_sand_tcm_unregister_polling_callback(
    DNX_SAND_IN     uint32            handle
  ) ;
/*
 */

DNX_SAND_RET
  dnx_sand_tcm_callback_delta_list_take_mutex(
    void
  );
DNX_SAND_RET
  dnx_sand_tcm_callback_delta_list_give_mutex(
    void
  );

#if DNX_SAND_DEBUG
void dnx_sand_tcm_debug_table_print(void) ;
void dnx_sand_tcm_debug_table_clear(void) ;
/*
 * Prints the TCM delta list.
 */
void
  dnx_sand_tcm_print_delta_list(
    void
  ) ;
/*
 * Prints the TCM info.
 */
void
  dnx_sand_tcm_general_status_print(
    void
  );
#endif /* DNX_SAND_DEBUG */

#ifdef  __cplusplus
}
#endif

#endif
