/* $Id: sand_user_callback.h,v 1.3 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/


#ifndef __SOC_SAND_USER_CALLBACK_H_INCLUDED__
/* { */
#define __SOC_SAND_USER_CALLBACK_H_INCLUDED__
#ifdef  __cplusplus
extern "C" {
#endif

#include <soc/dpp/SAND/Utils/sand_framework.h>

/* $Id: sand_user_callback.h,v 1.3 Broadcom SDK $
 * Pointer to user callback procedure that may be called
 * when driver detects specific condition, previously
 * related to this procedure by the user.
 * Callback procedure returns int and gets, as parameters,
 * uint32, uint32 *, uint32 **,
 *     uint32, uint32, uint32, uint32,
 *     uint32, uint32
 *
 * INPUT:
 *   first parameter - uint32 -
 *     User identifier of callback. This parameter is returned to the
 *     user with value indicated by the user when specifying this
 *     callback procedure. It allows the user to create one callback
 *     procedure to handle a few cases (using 'switch').
 *   second parameter - uint32 * -
 *      Pointer to (user supplied) buffer containing required results
 *      as per action specified forthis callback procedure.
 *      Buffer is of size SOC_SAND_CALLBACK_BUF_SIZE and is replaced by
 *      this procedure (using third parameter  below) every time this
 *      callback procedure is invoked.
 *      The contents of this buffer vary per callback.
 *    third parameter - uint32 ** -
 *      This callback procedure loads memory pointed by third parameter
 *      by pointer to buffer to be used as second parameter on the next
 *      invocation of this callback procedure. Buffer is, therefore,
 *      under the same restrictions as for second parameter (size
 *      SOC_SAND_CALLBACK_BUF_SIZE, including terminating null).
 *    Fourth parameter - uint32 -
 *      Size of meaningful results, loaded into buffer pointed by second
 *      parameter. Must be smaller than maximal size (SOC_SAND_CALLBACK_BUF_SIZE).
 *    Fifth parameter - uint32 -
 *      Error return value. See formatting rules in ERROR RETURN VALUES.
 *      If error code is not SOC_SAND_OK then this parameter indicates
 *      source of error and whether all other parameters are meaningful.
 *    Sixth parameter - uint32 unit -
 *      Value of 'unit' for which this polling
 *      transaction has been started.
 *    Seventh parameter - uint32 polling_transaction_id -
 *      Value of 'polling_transaction_id' assigned to
 *      this polling transaction.
 *    Eighth parameter - uint32 param_01 -
 *      Meaning-less if not other specified.
 *    Ninth parameter - uint32 param_02 -
 *      Meaning-less if not other specified.
 */
typedef int (*SOC_SAND_USER_CALLBACK_FUNC)(uint32, uint32 *,uint32 **,
  uint32,
  uint32, uint32,
  uint32, uint32, uint32) ;


typedef struct
{

  /*
   * User supplied callback function.
   */
  SOC_SAND_USER_CALLBACK_FUNC callback_func;

  /*
   * User supplied buffer. This buffer size and
   * type is determined by the 'function_id'.
   */
  uint32*    result_ptr;

  /*
   * Follwing are six paramers. There meaning is induced by
   * the 'function_id'. There numbering first, seconed...
   * is done with out the result buffer and the unit
   * parameter.
   * Example: In function 'soc_sand_read_mem'
   * param1 is 'offset'
   * param2 is 'size'
   * param3 is 'indirect'
   */
  uint32      param1;
  uint32      param2;
  uint32      param3;
  uint32      param4;
  uint32      param5;
  uint32      param6;

  /*
   * Interrupt or poll
   * TRUE:  asked for interrupt calling scheme.
   * FALSE: asked for polling   calling scheme.
   */
  uint32      interrupt_not_poll;

  /*
   * if 'interrupt_not_poll' is TRUE:
   *   minimal elapsed time, in system ticks, between
   *   two consecutive interrupts of the kind related
   *   to this service. If set to '0' then no limit
   *   is required.
   *   See 'system_tick_in_ms' in 'soc_sand_module_open'.
   *   This parameter may not be smaller than
   *   SOC_SAND_MIN_TCM_ACTIVATION_PERIOD (apart from
   *   being set to '0')
   *   This parameter is used to limit real time burden
   *   of interrupts due to this service.
   * if 'interrupt_not_poll' is FALSE:
   *   Period, in system ticks, at which to poll for
   *   received source routed data cells.
   *   See 'system_tick_in_ms' in 'soc_sand_module_open'.
   *   This parameter may not be smaller than
   *   SOC_SAND_MIN_TCM_ACTIVATION_PERIOD.
   */
  uint32      callback_rate;

  /*
   * Identifier of callback. This parameter is returned
   * to the caller via callback, as is. Caller may use
   * it to identify specific actions within one
   * callback procedure (using 'switch').
   * This parameter is only meaningful if input
   * parameter 'activate_polling' is non-zero.
   */
  uint32   user_callback_id;

} SOC_SAND_USER_CALLBACK;

#ifdef  __cplusplus
}
#endif

/* } __SOC_SAND_USER_CALLBACK_H_INCLUDED__*/
#endif
