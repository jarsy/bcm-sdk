/* $Id: sand_framework.c,v 1.12 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 * $
*/


#include <shared/bsl.h>
#include <soc/dnx/legacy/drv.h>



#include <soc/dnx/legacy/SAND/Utils/sand_framework.h>
#include <soc/dnx/legacy/SAND/Utils/sand_os_interface.h>

#include <soc/dnx/legacy/SAND/Management/sand_general_params.h>
#include <soc/dnx/legacy/SAND/Management/sand_general_macros.h>
#include <soc/dnx/legacy/SAND/Management/sand_error_code.h>
#include <soc/dnx/legacy/SAND/Management/sand_chip_descriptors.h>

#include <soc/dnx/legacy/SAND/Utils/sand_integer_arithmetic.h>

/* $Id: sand_framework.c,v 1.12 Broadcom SDK $
 */
static
  DNX_SAND_ERROR_HANDLER_PTR
    Dnx_soc_sand_supplied_error_handler      = NULL ;
/*
 */
  char  *Dnx_soc_sand_supplied_error_buffer  = NULL ;

  uint32
    Dnx_soc_sand_supplied_error_handler_is_on = FALSE;
/*
 * Enabling temporary disable the error sending mechanism,
 *   without loosing the error handler data structures and information
 */
DNX_SAND_RET
  dnx_sand_set_user_error_state(
    uint32 onFlag
  )
{
  DNX_SAND_RET
    ex=DNX_SAND_OK ;
  uint32
    err=0 ;

  if(!Dnx_soc_sand_supplied_error_handler && onFlag)
  {
    ex = DNX_SAND_ERR;
    goto exit;
  }

  Dnx_soc_sand_supplied_error_handler_is_on = onFlag;

exit:
  DNX_SAND_ERROR_REPORT(ex,NULL,0,0,DNX_SAND_SET_USER_ERROR_HANDLER,
        "General error in dnx_sand_set_user_error_handler()",err,0,0,0,0,0) ;
  return ex;
}

/*
 */
DNX_SAND_RET
  dnx_sand_set_user_error_handler(
    DNX_SAND_ERROR_HANDLER_PTR  user_error_handler,
    char                    *user_error_buffer
  )
{
  DNX_SAND_RET
    ex ;
  uint32
    err ;
  ex = DNX_SAND_OK ;
  err = 0 ;
  Dnx_soc_sand_supplied_error_handler = user_error_handler ;
  Dnx_soc_sand_supplied_error_buffer  = user_error_buffer ;
  Dnx_soc_sand_supplied_error_handler_is_on = (Dnx_soc_sand_supplied_error_handler != NULL);
  if (Dnx_soc_sand_supplied_error_handler_is_on)
  {
    ex = dnx_sand_init_errors_queue() ;
    if (ex != DNX_SAND_OK)
    {
      err = 1 ;
      goto exit ;
    }
  }
  else
  {
    /*
     * Just making sure that queue is deleted if
     * user specifies a null error handler.
     */
    ex = dnx_sand_delete_errors_queue() ;
    if (ex != DNX_SAND_OK)
    {
      err = 2 ;
      goto exit ;
    }
  }
exit:
  DNX_SAND_ERROR_REPORT(ex,NULL,0,0,DNX_SAND_SET_USER_ERROR_HANDLER,
        "General error in dnx_sand_set_user_error_handler()",err,0,0,0,0,0) ;
  return (ex) ;
}
/*
 * Handling of queue of system wide error reporting scheme.
 * {
 */
/*
 * This flag is raised before a message is sent to
 * the 'TCM' queue to indicate an 'error' message
 * has been loaded into 'errors' queue.
 * It is lowered by TCM task when detecting that
 * message in 'TCM' queue (before taking messages out
 * of 'errors' queue.
 * {
 */
  uint32
      Dnx_soc_sand_errors_msg_queue_flagged = FALSE ;
void
  dnx_soc_set_errors_q_flag(
    void
  )
{
  Dnx_soc_sand_errors_msg_queue_flagged = TRUE ;
  return ;
}
uint32
    dnx_soc_is_errors_q_flag_set(
      void
    )
{
  return (Dnx_soc_sand_errors_msg_queue_flagged) ;
}
void
  dnx_soc_reset_errors_q_flag(
    void
  )
{
  Dnx_soc_sand_errors_msg_queue_flagged = FALSE ;
  return ;
}
/*
 * }
 */
static
  void
    *Dnx_soc_sand_errors_msg_queue = (void *)0 ;
/*****************************************************
*NAME
*  dnx_sand_init_errors_queue
*TYPE: PROC
*DATE: 13/FEB/2003
*FUNCTION:
*  Initialize queue of system wide error reporting
*  scheme.
*CALLING SEQUENCE:
*  dnx_sand_init_errors_queue()
*INPUT:
*  DNX_SAND_DIRECT:
*    None.
*  DNX_SAND_INDIRECT:
*    None.
*OUTPUT:
*  DNX_SAND_DIRECT:
*    Int -
*      if non-DNX_SAND_OK then some error has been
*      encountered.
*  DNX_SAND_INDIRECT:
*    None
*REMARKS:
*  None.
*SEE ALSO:
*****************************************************/
DNX_SAND_RET
  dnx_sand_init_errors_queue(
    void
  )
{
  DNX_SAND_RET
    ex ;
  uint32
    err ;
  ex = DNX_SAND_OK ;
  err = 0 ;
  /*
   * create the 'errors' message queue
   */
  Dnx_soc_sand_errors_msg_queue =
    dnx_sand_os_msg_q_create(
            ERRORS_MSG_QUEUE_NUM_MSGS,DNX_SAND_ERRORS_MSG_QUEUE_SIZE_MSG) ;
  if (!Dnx_soc_sand_errors_msg_queue)
  {
    ex = DNX_SAND_ERR ;
    err = 1 ;
    goto exit ;
  }
exit:
  DNX_SAND_ERROR_REPORT(ex,NULL,0,0,DNX_SAND_INIT_ERRORS_QUEUE,
        "General error in dnx_sand_init_errors_queue()",err,0,0,0,0,0) ;
  return (ex) ;
}
/*****************************************************
*NAME
*  dnx_sand_delete_errors_queue
*TYPE: PROC
*DATE: 13/FEB/2003
*FUNCTION:
*  Delete queue of system wide error reporting
*  scheme.
*CALLING SEQUENCE:
*  dnx_sand_delete_errors_queue()
*INPUT:
*  DNX_SAND_DIRECT:
*    None.
*  DNX_SAND_INDIRECT:
*    None.
*OUTPUT:
*  DNX_SAND_DIRECT:
*    Int -
*      if non-DNX_SAND_OK then some error has been
*      encountered.
*  DNX_SAND_INDIRECT:
*    None
*REMARKS:
*  This action is meant to be taken only at module shut
*  down.
*SEE ALSO:
*****************************************************/
DNX_SAND_RET
  dnx_sand_delete_errors_queue(
    void
  )
{
  DNX_SAND_RET
    ex ;
  uint32
    err ;
  ex = DNX_SAND_OK ;
  err = 0 ;
  /*
   * Delete the 'errors' message queue
   */
  if (Dnx_soc_sand_errors_msg_queue)
  {
    ex = dnx_sand_os_msg_q_delete(Dnx_soc_sand_errors_msg_queue) ;
    Dnx_soc_sand_errors_msg_queue = (void *)0 ;
    if (ex != DNX_SAND_OK)
    {
      err = 1 ;
      goto exit ;
    }
  }
exit:
  DNX_SAND_ERROR_REPORT(ex,NULL,0,0,DNX_SAND_DELETE_ERRORS_QUEUE,
        "General error in dnx_sand_delete_errors_queue()",err,0,0,0,0,0) ;
  return (ex) ;
}
/*****************************************************
*NAME
*  dnx_sand_is_errors_queue_msg
*TYPE: PROC
*DATE: 14/FEB/2003
*FUNCTION:
*  Checks message on TCM queue and indicates whether
*  this is a message relating to 'errors' queue
*  being not empty.
*CALLING SEQUENCE:
*  dnx_sand_is_errors_queue_msg(msg)
*INPUT:
*  DNX_SAND_DIRECT:
*    uint32 msg -
*      uint32. Message received on TCM queue.
*  DNX_SAND_INDIRECT:
*    DNX_SAND_NEW_ERROR_MESSAGE
*OUTPUT:
*  DNX_SAND_DIRECT:
*    Int -
*      if non-zero then this is an indication related
*      to 'errors' queue.
*  DNX_SAND_INDIRECT:
*    None.
*REMARKS:
*  None.
*SEE ALSO:
*****************************************************/
int
  dnx_sand_is_errors_queue_msg(
    uint32 msg
  )
{
  int
    ret ;
  if (msg == DNX_SAND_NEW_ERROR_MESSAGE)
  {
    ret = 1 ;
  }
  else
  {
    ret = 0 ;
  }
  return (ret) ;
}
/*****************************************************
*NAME
*  dnx_sand_load_errors_queue
*TYPE: PROC
*DATE: 14/FEB/2003
*FUNCTION:
*  Load queue of system wide errors queue and
*  flag TCM to handle it.
*CALLING SEQUENCE:
*  dnx_sand_load_errors_queue(errors_queue_message)
*INPUT:
*  DNX_SAND_DIRECT:
*    DNX_SAND_ERRORS_QUEUE_MESSAGE
*      *errors_queue_message -
*        Pointer to structure containing info to
*        load into queue.
*  DNX_SAND_INDIRECT:
*    Dnx_soc_sand_errors_msg_queue.
*OUTPUT:
*  DNX_SAND_DIRECT:
*    Int -
*      if non-DNX_SAND_OK then some error has been
*      encountered.
*  DNX_SAND_INDIRECT:
*    TCM queue
*    Dnx_soc_sand_errors_msg_queue_flagged
*    Dnx_soc_sand_errors_msg_queue
*REMARKS:
*  If queue is full then procedure quitely quits. This
*  avoids potential infinite loops.
*
*  If Dnx_soc_sand_errors_msg_queue is found not to have been
*  initialized then this is taken as an indication that
*  user has not specified a system wide error handler
*  so no action is taken.
*SEE ALSO:
*****************************************************/
DNX_SAND_RET
  dnx_sand_load_errors_queue(
    DNX_SAND_ERRORS_QUEUE_MESSAGE *errors_queue_message
  )
{
  DNX_SAND_RET
    ex;
  uint32
      err_x,
      err =0;
  int
    num_in_q ;
  ex = DNX_SAND_OK ;
  err = 0 ;
  dnx_sand_initialize_error_word(DNX_SAND_LOAD_ERRORS_QUEUE,0,&err_x) ;
  if (Dnx_soc_sand_errors_msg_queue)
  {
    num_in_q = dnx_sand_os_msg_q_num_msgs(Dnx_soc_sand_errors_msg_queue) ;
    if (num_in_q >= ERRORS_MSG_QUEUE_NUM_MSGS)
    {
      goto exit ;
    }
    ex =
      dnx_sand_os_msg_q_send(
        Dnx_soc_sand_errors_msg_queue,(char *)errors_queue_message,
        sizeof(*errors_queue_message),DNX_SAND_NO_TIMEOUT,0) ;
    if (ex != DNX_SAND_OK)
    {
      err = 1 ;
      goto exit ;
    }
    if (!dnx_soc_is_errors_q_flag_set())
    {
      uint32
          msg ;
      dnx_soc_set_errors_q_flag() ;
      msg = DNX_SAND_NEW_ERROR_MESSAGE ;
      ex = dnx_sand_tcm_send_message_to_q_from_task(msg) ;
      if (ex != DNX_SAND_OK)
      {
        err = 2 ;
        goto exit ;
      }
    }
  }
exit:
/* Sending an error message to inform that this function        */
/* is not working is useless, because the failure msg mechanism */
/* is using this function to send error messages...             */
/* Therefore the failure message should be sent directlly       */
/* to the user directly. */
  if(err)
  {
    dnx_sand_set_error_code_into_error_word(ex,&err_x) ;
    dnx_sand_invoke_user_error_handler(
      err_x,"error in dnx_sand_load_errors_queue",err,0,0,0,0,0);
  }
  return (ex) ;
}
/*****************************************************
*NAME
*  dnx_sand_unload_errors_queue
*TYPE: PROC
*DATE: 14/FEB/2003
*FUNCTION:
*  Load queue of system wide errors queue and
*  flag TCM to handle it.
*CALLING SEQUENCE:
*  dnx_sand_unload_errors_queue()
*INPUT:
*  DNX_SAND_DIRECT:
*    None.
*  DNX_SAND_INDIRECT:
*    Dnx_soc_sand_errors_msg_queue.
*OUTPUT:
*  DNX_SAND_DIRECT:
*    Int -
*      if non-DNX_SAND_OK then some error has been
*      encountered.
*  DNX_SAND_INDIRECT:
*    Dnx_soc_sand_errors_msg_queue_flagged
*    Dnx_soc_sand_errors_msg_queue
*    User supplied error handler
*REMARKS:
*  If Dnx_soc_sand_errors_msg_queue is found not to have been
*  initialized then this is taken as an indication that
*  user has not specified a system wide error handler
*  so no action is taken.
*SEE ALSO:
*****************************************************/
DNX_SAND_RET
  dnx_sand_unload_errors_queue(
    void
  )
{
  DNX_SAND_RET
    ex ;
  uint32
      err ;
  int
    mesg_received,
    num_in_q ;
  DNX_SAND_ERRORS_QUEUE_MESSAGE
    errors_queue_message ;
  ex = DNX_SAND_OK ;
  err = 0 ;
  if (Dnx_soc_sand_errors_msg_queue)
  {
    dnx_soc_reset_errors_q_flag() ;
    while (TRUE)
    {
      num_in_q = dnx_sand_os_msg_q_num_msgs(Dnx_soc_sand_errors_msg_queue) ;
      if (num_in_q == 0)
      {
        goto exit ;
      }
      else if (num_in_q < 0)
      {
        ex = DNX_SAND_ERR ;
        err = 1 ;
        goto exit ;
      }
      mesg_received =
        dnx_sand_os_msg_q_recv(
          Dnx_soc_sand_errors_msg_queue,
          (unsigned char *)&errors_queue_message,
          sizeof(errors_queue_message),
          (long)DNX_SAND_NO_TIMEOUT) ;

      ex = (mesg_received == 0)?DNX_SAND_OK:DNX_SAND_ERR;

      if (ex != DNX_SAND_OK)
      {
        err = 2 ;
        goto exit ;
      }
      ex =
        dnx_sand_invoke_user_error_handler(
          errors_queue_message.err_id,
          errors_queue_message.error_txt,
          errors_queue_message. param_01,
          errors_queue_message.param_02,
          errors_queue_message.param_03,
          errors_queue_message.param_04,
          errors_queue_message.param_05,
          errors_queue_message.param_06
          ) ;
      if (ex != DNX_SAND_OK)
      {
        err = 3 ;
        goto exit ;
      }
    }
  }
exit:
  DNX_SAND_ERROR_REPORT(ex,NULL,0,0,DNX_SAND_UNLOAD_ERRORS_QUEUE,
        "General error in dnx_sand_unload_errors_queue()",err,0,0,0,0,0) ;
  return (ex) ;
}
/*
 * }
 */

/*****************************************************
*NAME
*  dnx_sand_error_handler
*TYPE: PROC
*DATE: 13/FEB/2003
*FUNCTION:
*  This procedure collects errors/events from the
*  whole system and takes care of sending them
*  toe the user, if required.
*CALLING SEQUENCE:
*  dnx_sand_error_handler(
*    err_id,error_txt,param_01,param_02,
*    param_03,param_04,param_05,param_06)
*INPUT:
*  DNX_SAND_DIRECT:
*    uint32 err_id -
*      Identifier of error. See ERROR RETURN VALUES
*      in dnx_sand_error_code.h
*    char          *error_txt -
*      Null terminated string describing error. This
*      string is copied by this procedure so it does
*      NOT have to be static.
*    uint32 param_01 -
*      General parameters related to error description.
*    uint32 param_02 -
*      General parameters related to error description.
*    uint32 param_03 -
*      General parameters related to error description.
*    uint32 param_04 -
*      General parameters related to error description.
*    uint32 param_05 -
*      General parameters related to error description.
*    uint32 param_06 -
*      General parameters related to error description.
*  DNX_SAND_INDIRECT:
*    None.
*OUTPUT:
*  DNX_SAND_DIRECT:
*    Int -
*      if non-DNX_SAND_OK then some error has been
*      encountered.
*  DNX_SAND_INDIRECT:
*    See error_txt.
*REMARKS:
*  None.
*SEE ALSO:
*****************************************************/
DNX_SAND_RET
  dnx_sand_error_handler(
    uint32 err_id,
    const char    *error_txt,
    uint32 param_01,
    uint32 param_02,
    uint32 param_03,
    uint32 param_04,
    uint32 param_05,
    uint32 param_06
  )
{
  DNX_SAND_RET
    ex ;
#if (/* SAND_LOW_LEVEL_SIMULATION && */ DNX_SAND_ALLOW_DRIVER_TO_PRINT_ERRORS)  
  int
    ret;
#endif
  DNX_SAND_ERRORS_QUEUE_MESSAGE
    *errors_queue_message = NULL;
    ex = DNX_SAND_OK ;

#if (/* SAND_LOW_LEVEL_SIMULATION && */ DNX_SAND_ALLOW_DRIVER_TO_PRINT_ERRORS)
    if (!(dnx_sand_no_error_printing_get()))
    {
      ret = dnx_sand_general_display_err(err_id, error_txt);
      ex = (uint16)ret;
      LOG_INFO(BSL_LS_SOC_COMMON,
               (BSL_META("exit place: %d, params: %d %d %d\n\r"),param_01,param_02,param_03,param_04));
    }
#endif
  if (Dnx_soc_sand_supplied_error_handler_is_on)
  {
    uint32
        size ;
    errors_queue_message = sal_alloc(sizeof(DNX_SAND_ERRORS_QUEUE_MESSAGE), "dnx_sand_error_handler.errors_queue_message");
    if(errors_queue_message == NULL) {
        ex = 1;
        goto exit; 
    }
    errors_queue_message->err_id = err_id ;
    size = sizeof(errors_queue_message->error_txt) ;
    dnx_sand_os_strncpy(
      errors_queue_message->error_txt,error_txt,size) ;
    errors_queue_message->error_txt[size - 1] = 0 ;
    errors_queue_message->param_01 = param_01 ;
    errors_queue_message->param_02 = param_02 ;
    errors_queue_message->param_03 = param_03 ;
    errors_queue_message->param_04 = param_04 ;
    errors_queue_message->param_05 = param_05 ;
    errors_queue_message->param_06 = param_06 ;
    ex = dnx_sand_load_errors_queue(errors_queue_message) ;
    if (ex != DNX_SAND_OK)
    {
      goto exit ;
    }
  }
exit:
  if(errors_queue_message) {
    sal_free(errors_queue_message);
  }
/* Sending an error message to inform that the error msg      */
/* mechanism is not working is useless...                     */
/* The function 'dnx_sand_load_errors_queue' will send a failure  */
/* message to the user directly, when it fail                 */
  return (ex) ;
}
/*****************************************************
*NAME
*  dnx_sand_invoke_user_error_handler
*TYPE: PROC
*DATE: 14/FEB/2003
*FUNCTION:
*  This procedure sends error reports via user
*  supplied error handler.
*CALLING SEQUENCE:
*  dnx_sand_invoke_user_error_handler(
*    err_id,error_txt,param_01,param_02,
*    param_03,param_04,param_05,param_06)
*INPUT:
*  DNX_SAND_DIRECT:
*    uint32 err_id -
*      Identifier of error. See ERROR RETURN VALUES
*      in dnx_sand_error_code.h
*    char          *error_txt -
*      Null terminated string describing error. This
*      string is copied by this procedure so it does
*      NOT have to be static.
*    uint32 param_01 -
*      General parameters related to error description.
*    uint32 param_02 -
*      General parameters related to error description.
*    uint32 param_03 -
*      General parameters related to error description.
*    uint32 param_04 -
*      General parameters related to error description.
*    uint32 param_05 -
*      General parameters related to error description.
*    uint32 param_06 -
*      General parameters related to error description.
*  DNX_SAND_INDIRECT:
*    None.
*OUTPUT:
*  DNX_SAND_DIRECT:
*    Int -
*      if non-DNX_SAND_OK then some error has been
*      encountered.
*  DNX_SAND_INDIRECT:
*    See error_txt.
*REMARKS:
*  None.
*SEE ALSO:
*****************************************************/
DNX_SAND_RET
  dnx_sand_invoke_user_error_handler(
    uint32 err_id,
    const char    *error_txt,
    uint32 param_01,
    uint32 param_02,
    uint32 param_03,
    uint32 param_04,
    uint32 param_05,
    uint32 param_06
  )
{
  DNX_SAND_RET
    ex ;
  char* new_err_description ;

  ex = DNX_SAND_OK ;
  if (Dnx_soc_sand_supplied_error_handler)
  {
    new_err_description = NULL ;
    if (Dnx_soc_sand_supplied_error_buffer)
    {
      dnx_sand_os_strncpy(Dnx_soc_sand_supplied_error_buffer,error_txt,DNX_SAND_CALLBACK_BUF_SIZE) ;
      Dnx_soc_sand_supplied_error_buffer[DNX_SAND_CALLBACK_BUF_SIZE - 1] = 0 ;
    }
    ex = (DNX_SAND_RET)Dnx_soc_sand_supplied_error_handler(
            err_id,
            Dnx_soc_sand_supplied_error_buffer,
            &new_err_description,
            param_01,
            param_02,
            param_03,
            param_04,
            param_05,
            param_06
          ) ;
    if (new_err_description)
    {
      Dnx_soc_sand_supplied_error_buffer = new_err_description ;
    }
  }
  return ex ;
}
/*
 */
int
  dnx_sand_is_long_aligned (
    uint32 word_to_check
  )
{
  if (word_to_check & DNX_SAND_UINT32_ALIGN_MASK)
  {
    return FALSE ;
  }
  return TRUE ;
}
/*
 */
void
  dnx_sand_check_driver_and_device(
    int  unit,
    uint32 *error_word
  )
{
  if ( !dnx_sand_general_get_driver_is_started() )
  {
    dnx_sand_set_error_code_into_error_word(DNX_SAND_DRIVER_NOT_STARTED, error_word) ;
    goto exit ;
  }
  /*
   */
  if ( !dnx_sand_is_chip_descriptor_valid(unit) )
  {
    dnx_sand_set_error_code_into_error_word(DNX_SAND_ILLEGAL_DEVICE_ID, error_word) ;
    goto exit ;
  }
  /*
   */
exit:
  return ;
}

uint32
  dnx_sand_get_index_of_max_member_in_array(
    DNX_SAND_IN     uint32                     array[],
    DNX_SAND_IN     uint32                    len
  )
{
  uint32
      index,
      index_of_max,
      max_val;

  index_of_max = 0;
  max_val = 0;

  for (index = 0; index < len; index++)
  {
    if (array[index] > max_val)
    {
      max_val = array[index];
      index_of_max = index;
    }
  }
  return index_of_max;
}

#if DNX_SAND_DEBUG
/* { */

/*****************************************************
*NAME
* dnx_sand_general_display_err
*TYPE:
*  PROC
*DATE:
*  22/10/2007
*FUNCTION:
*  Call the device relevant display error function.
*INPUT:
*  DNX_SAND_DIRECT:
*    uint32 err_id -
*      32 bits of module ID, function ID and Global Error ID
*    char          *error_txt -
*  DNX_SAND_INDIRECT:
*    None.
*OUTPUT:
*  DNX_SAND_DIRECT:
*    error indication
*  DNX_SAND_INDIRECT:
*    None.
*REMARKS:
*    This function should only be called when
*    'DNX_SAND_ERROR_CODE_OLD_MODE' is not defined.
*SEE ALSO:
*****************************************************/
int
  dnx_sand_general_display_err(
    uint32 err_id,
    const char    *error_txt
  )
{
  int
    ret=0;
  uint32
    module_id;
  module_id = DNX_SAND_GET_FLD_FROM_PLACE(err_id,DNX_SAND_MODULE_ID_SHIFT, DNX_SAND_MODULE_ID_MASK);
  switch(module_id)
  {
  case DNX_SAND_MODULE_IDENTIFIER_DNX:
    ret = dnx_sand_disp_result(err_id);
    break;
  default:
    LOG_CLI((BSL_META("Failed to parse Error ID 0x%x \n\r"
                      " Error Text %s\n\r"),
             err_id,
             error_txt
             ));
  }
  return ret;
}

/*
 * Printing utility.
 * Convert from enumerator to string.
 */
const char*
  dnx_sand_SAND_OP_to_str(
    DNX_SAND_IN DNX_SAND_OP      dnx_sand_op,
    DNX_SAND_IN uint32 short_format
  )
{
  const char
    *str;

  switch(dnx_sand_op)
  {
  case DNX_SAND_NOP:
    if(short_format)
    {
      str = "NOP";
    }
    else
    {
      str = "DNX_SAND_NOP";
    }
    break;

  case DNX_SAND_OP_AND:
    if(short_format)
    {
      str = "AND";
    }
    else
    {
      str = "DNX_SAND_OP_AND";
    }
    break;

  case DNX_SAND_OP_OR:
    if(short_format)
    {
      str = "OR";
    }
    else
    {
      str = "DNX_SAND_OP_OR";
    }
    break;

  case DNX_SAND_NOF_SAND_OP:
    if(short_format)
    {
      str = "NOF_OP";
    }
    else
    {
      str = "DNX_SAND_NOF_SAND_OP";
    }
    break;

  default:
    str = "dnx_sand_SAND_OP_to_str input parameters error (dnx_sand_op)";
  }

  return str;
}

/*
 * Print HEX buffer.
 */
/*****************************************************
*NAME
* dnx_sand_print_hex_buff
*TYPE:
*  PROC
*DATE:
*  14-Sep-03
*FUNCTION:
*  Printing aid utility.
*  Prints byte buffer in HEX format.
*INPUT:
*  DNX_SAND_DIRECT:
*    DNX_SAND_IN char*        buff -
*      Byte buffer.
*    DNX_SAND_IN uint32 buff_byte_size -
*      Number of bytes in the buffer
*    DNX_SAND_IN uint32 nof_bytes_per_line -
*      Number of byte to print in one line.
*  DNX_SAND_INDIRECT:
*    None.
*OUTPUT:
*  DNX_SAND_DIRECT:
*    None.
*  DNX_SAND_INDIRECT:
*    NON
*REMARKS:
*  None.
*SEE ALSO:
*****************************************************/
void
  dnx_sand_print_hex_buff(
    DNX_SAND_IN char*        buff,
    DNX_SAND_IN uint32 buff_byte_size,
    DNX_SAND_IN uint32 nof_bytes_per_line
  )
{
  uint32
    byte_i;

  if(NULL == buff)
  {
    goto exit;
  }

  for (byte_i=0; byte_i<buff_byte_size; byte_i++)
  {
    if( (byte_i != 0)  &&
        (byte_i%4 == 0) &&
        ((byte_i%nof_bytes_per_line) != 0)
      )
    {
      LOG_CLI((BSL_META(" ")));
    }
    if( (byte_i != 0)  &&
        ((byte_i%nof_bytes_per_line) == 0)
      )
    {
      LOG_CLI((BSL_META("\n\r")));
    }
    if((byte_i%nof_bytes_per_line) == 0)
    {
      LOG_CLI((BSL_META("%3u-%3u:"),
               byte_i,
               DNX_SAND_MIN(byte_i + (nof_bytes_per_line-1), buff_byte_size-1)
               ));
    }

    LOG_CLI((BSL_META("%02X"), (buff[byte_i]&0xFF)));
  }
  LOG_CLI((BSL_META("\n\r")));

exit:
  return;
}

/*****************************************************
*NAME
* dnx_sand_print_bandwidth
*TYPE:
*  PROC
*DATE:
*  13-Sep-04
*FUNCTION:
*  Printing aid utility.
*  Prints Band-Width.
*INPUT:
*  DNX_SAND_DIRECT:
*    DNX_SAND_IN uint32 bw_kbps -
*      Band-Width in KiloBits-Second
*  DNX_SAND_INDIRECT:
*    None.
*OUTPUT:
*  DNX_SAND_DIRECT:
*    None.
*  DNX_SAND_INDIRECT:
*    NON
*REMARKS:
*  None.
*SEE ALSO:
*****************************************************/
void
  dnx_sand_print_bandwidth(
    DNX_SAND_IN uint32 bw_kbps,
    DNX_SAND_IN uint32  short_format
  )
{
  uint32
    tmp;

  tmp = bw_kbps;
  tmp /= 1000;

  LOG_CLI((BSL_META("%u Kbps,  %u.%03u Gbps"),
           bw_kbps,
           tmp/1000,
           tmp%1000
           ));

  if(!short_format)
  {
    LOG_CLI((BSL_META("\n\r")));
  }

  return;
}
#endif

/*****************************************************
*NAME:
* dnx_sand_set_field
*DATE:
* 10/SEP/2007
*FUNCTION:
*  set field onto the register
*INPUT:
*  DNX_SAND_DIRECT:
*    DNX_SAND_INOUT    uint32* reg_val  -
*       the value of the register to be changed
*    DNX_SAND_IN       uint32    ms_bit         -
*       most significant bit where to set the field_val,
*    DNX_SAND_IN       uint32    ls_bit         -
*       less significant bit where to set the field_val
*       Range 0:32.
*    DNX_SAND_IN       uint32    field_val -
*       field to set into reg_val
*  DNX_SAND_INDIRECT:
*OUTPUT:
*  DNX_SAND_DIRECT:
*    Non-zero in case of an error
*  DNX_SAND_INDIRECT:
*    the value of the register after setting the value into it.
*REMARKS:
*    None
*SEE ALSO:
*****************************************************/
DNX_SAND_RET
  dnx_sand_set_field(
    DNX_SAND_INOUT  uint32    *reg_val,
    DNX_SAND_IN  uint32       ms_bit,
    DNX_SAND_IN  uint32       ls_bit,
    DNX_SAND_IN  uint32       field_val
  )
{
  uint32
    tmp_reg;
  DNX_SAND_RET
    dnx_sand_ret;

  dnx_sand_ret = DNX_SAND_OK;
  tmp_reg = *reg_val;

  /*
   * 32 bits at most
   */
  if (ms_bit-ls_bit+1 > 32)
  {
    dnx_sand_ret = DNX_SAND_BIT_STREAM_FIELD_SET_SIZE_RANGE_ERR;
    goto exit;
  }

  tmp_reg &= DNX_SAND_ZERO_BITS_MASK(ms_bit,ls_bit);

  tmp_reg |= DNX_SAND_SET_BITS_RANGE(
              field_val,
              ms_bit,
              ls_bit
            );

  *reg_val = tmp_reg;

exit:
  return dnx_sand_ret;

}

DNX_SAND_RET
  dnx_sand_U8_to_U32(
    DNX_SAND_IN uint8     *u8_val,
    DNX_SAND_IN uint32    nof_bytes,
    DNX_SAND_OUT uint32   *u32_val
  )
{
  uint32
    u8_indx,
    cur_u8_indx,
    u32_indx;
  DNX_SAND_IN uint8
    *cur_u8;
  DNX_SAND_RET
    dnx_sand_ret = DNX_SAND_OK;

  if (!u8_val || !u32_val)
  {
    dnx_sand_ret = DNX_SAND_ERR ;
    goto exit ;
  }

  cur_u8_indx = 0;
  u32_indx = 0;

  for ( cur_u8 = u8_val, u8_indx = 0; u8_indx < nof_bytes; ++u8_indx, ++cur_u8)
  {
     dnx_sand_set_field(
       &(u32_val[u32_indx]),
       (cur_u8_indx + 1) * DNX_SAND_NOF_BITS_IN_BYTE - 1,
       cur_u8_indx * DNX_SAND_NOF_BITS_IN_BYTE,
       *cur_u8
     );

    cur_u8_indx++;
    if (cur_u8_indx >= sizeof(uint32))
    {
      cur_u8_indx = 0;
      ++u32_indx;
    }
  }
exit:
  return dnx_sand_ret;
}

DNX_SAND_RET
  dnx_sand_U32_to_U8(
    DNX_SAND_IN uint32  *u32_val,
    DNX_SAND_IN uint32  nof_bytes,
    DNX_SAND_OUT uint8  *u8_val
  )
{
  uint32
    u8_indx,
    cur_u8_indx;
  DNX_SAND_IN uint32
    *cur_u32;

  DNX_SAND_RET
    dnx_sand_ret = DNX_SAND_OK;

  if (!u8_val || !u32_val)
  {
    dnx_sand_ret = DNX_SAND_ERR ;
    goto exit ;
  }

  cur_u8_indx = 0;
  for ( cur_u32 = u32_val, u8_indx = 0; u8_indx < nof_bytes; ++u8_indx)
  {
    u8_val[u8_indx] = (uint8)
      DNX_SAND_GET_BITS_RANGE(
        *cur_u32,
        (cur_u8_indx + 1) * DNX_SAND_NOF_BITS_IN_BYTE - 1,
        cur_u8_indx * DNX_SAND_NOF_BITS_IN_BYTE
       );

    ++cur_u8_indx;
    if (cur_u8_indx >= sizeof(uint32))
    {
      cur_u8_indx = 0;
      ++cur_u32;
    }
  }
exit:
  return dnx_sand_ret;
}

void
  dnx_sand_SAND_U32_RANGE_clear(
    DNX_SAND_OUT DNX_SAND_U32_RANGE *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  dnx_sand_os_memset(info, 0x0, sizeof(DNX_SAND_U32_RANGE));
  info->start = 0;
  info->end = 0;
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  dnx_sand_SAND_TABLE_BLOCK_RANGE_clear(
    DNX_SAND_OUT DNX_SAND_TABLE_BLOCK_RANGE *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  dnx_sand_os_memset(info, 0x0, sizeof(DNX_SAND_TABLE_BLOCK_RANGE));
  info->iter = 0;
  info->entries_to_scan = 0;
  info->entries_to_act = 0;
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

#if DNX_SAND_DEBUG

const char*
  dnx_sand_SAND_SUCCESS_FAILURE_to_string(
    DNX_SAND_IN  DNX_SAND_SUCCESS_FAILURE enum_val
  )
{
  const char* str = NULL;
  switch(enum_val)
  {
  case DNX_SAND_SUCCESS:
    str = "SUCCESS";
  break;
  case DNX_SAND_FAILURE_OUT_OF_RESOURCES:
    str = "FAILURE_OUT_OF_RESOURCES";
  break;
  case DNX_SAND_FAILURE_OUT_OF_RESOURCES_2:
    str = "FAILURE_OUT_OF_RESOURCES_2";
  break;
  case DNX_SAND_FAILURE_OUT_OF_RESOURCES_3:
    str = "FAILURE_OUT_OF_RESOURCES_3";
  break;
  case DNX_SAND_FAILURE_REMOVE_ENTRY_FIRST:
    str = "FAILURE_REMOVE_ENTRY_FIRST";
  break;
  case DNX_SAND_FAILURE_INTERNAL_ERR:
    str = "FAILURE_INTERNAL_ERR";
  break;
  case DNX_SAND_FAILURE_UNKNOWN_ERR:
    str = "FAILURE_UNKNOWN_ERR";
  break;
  default:
    str = " Unknown Enumerator Value";
  }
  return str;
}

void
  dnx_sand_SAND_U32_RANGE_print(
    DNX_SAND_IN  DNX_SAND_U32_RANGE *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  LOG_CLI((BSL_META("%u - %u "),info->start, info->end));
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  dnx_sand_SAND_TABLE_BLOCK_RANGE_print(
    DNX_SAND_IN  DNX_SAND_TABLE_BLOCK_RANGE *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  LOG_CLI((BSL_META("iter: %u\n\r"),info->iter));
  LOG_CLI((BSL_META("entries_to_scan: %u\n\r"),info->entries_to_scan));
  LOG_CLI((BSL_META("entries_to_act: %u\n\r"),info->entries_to_act));
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}



/* } */
#endif /*DNX_SAND_DEBUG*/

