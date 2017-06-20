/* $Id: sand_module_management.c,v 1.9 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 * $
*/



#include <shared/bsl.h>

#include <soc/dnx/legacy/SAND/Utils/sand_framework.h>
#include <soc/dnx/legacy/SAND/Utils/sand_os_interface.h>
#include <soc/dnx/legacy/SAND/Utils/sand_trace.h>

#include <soc/dnx/legacy/SAND/Management/sand_module_management.h>
#include <soc/dnx/legacy/SAND/Management/sand_general_params.h>
#include <soc/dnx/legacy/SAND/Management/sand_error_code.h>
#include <soc/dnx/legacy/SAND/Management/sand_chip_descriptors.h>
#include <soc/dnx/legacy/SAND/Management/sand_callback_handles.h>
#include <soc/dnx/legacy/SAND/Management/sand_general_macros.h>

#include <soc/dnx/legacy/SAND/SAND_FM/sand_mem_access.h>

#define DNX_SAND_MODULE_MANAGEMENT_PRINTFS 0

/* $Id: sand_module_management.c,v 1.9 Broadcom SDK $
 * Static, general parameters of the DNX_SAND driver
 *
 * Dnx_soc_sand_start_module_shut_down_mutex
 * ------------------------------
 * the flag acts as test & set mutex making dnx_sand_module_open
 * and dnx_sand_module_close atomic. lets take dnx_sand_module_open() for instance:
 * at the begining of the method, under "disable intrrupts"
 * the first task to enter sets Inside dnx_sand_module_open to TRUE
 * (after checking that it was FALSE before, hence test & set)
 * ("disable interrupts" gurantees no context switching).
 * We set the flag back to FALSE before returning, after the
 * global Dnx_soc_sand_driver_is_started is alredy started, which will block
 * the rest of the tasks trying to start the driver when already started.
 * The same apply for dnx_sand_module_close()
 */
int Dnx_soc_sand_start_module_shut_down_mutex = FALSE ;

/*****************************************************
*NAME
* dnx_sand_module_init_all
*TYPE:
*  PROC
*DATE:
*  15-Jan-03
*FUNCTION:
*  Call to all files in DNX_SAND which have an initilization
*  function.
*INPUT:
*  DNX_SAND_DIRECT:
*    void -
*  DNX_SAND_INDIRECT:
*    None.
*OUTPUT:
*  DNX_SAND_DIRECT:
*    DNX_SAND_RET -
*      If not DNX_SAND_OK Error occurred in the opening of inner module.
*  DNX_SAND_INDIRECT:
*    NON
*REMARKS:
*  None.
*SEE ALSO:
*****************************************************/
static
  DNX_SAND_RET
    dnx_sand_module_init_all(
      void
    )
{
  DNX_SAND_RET
    ex ;
  uint32
    err = 0 ;
  ex = dnx_sand_trace_init() ;
  if (ex != DNX_SAND_OK)
  {
    err = 1 ;
    goto exit ;
  }
exit:
  DNX_SAND_ERROR_REPORT(ex,NULL,0,0,DNX_SAND_MODULE_INIT_ALL,
        "General error in dnx_sand_module_init_all()",err,0,0,0,0,0) ;
  return ex ;
}
/*****************************************************
*NAME
* dnx_sand_module_end_all
*TYPE:
*  PROC
*DATE:
*  15-Jan-03
*FUNCTION:
*  Call to all files in DNX_SAND which have an ending function.
*INPUT:
*  DNX_SAND_DIRECT:
*    void -
*  DNX_SAND_INDIRECT:
*    None.
*OUTPUT:
*  DNX_SAND_DIRECT:
*    DNX_SAND_RET -
*      If not DNX_SAND_OK Error occurred in the opening of inner module.
*  DNX_SAND_INDIRECT:
*    NON
*REMARKS:
*  None.
*SEE ALSO:
*****************************************************/
static
  DNX_SAND_RET
    dnx_sand_module_end_all(
      void
    )
{
  DNX_SAND_RET
    ex ;
  uint32
    err = 0 ;
  ex = dnx_sand_trace_end() ;
  if (ex != DNX_SAND_OK)
  {
    err = 1 ;
    goto exit ;
  }
exit:
  DNX_SAND_ERROR_REPORT(ex,NULL,0,0,DNX_SAND_MODULE_END_ALL,
        "General error in dnx_sand_module_end_all()",err,0,0,0,0,0) ;
  return ex ;
}
/*****************************************************
 * See details in dnx_sand_module_management.h
 *****************************************************/
uint32
  dnx_sand_module_open(
    uint32      max_num_devices,
    uint32      system_tick_in_ms,
    uint32      soc_tcmtask_priority,
    uint32      min_time_between_tcm_activation,
    uint32      soc_tcmmockup_interrupts,
    DNX_SAND_ERROR_HANDLER_PTR error_handler,
    char              *error_description,
    uint32      *is_already_opened
    )
{
  DNX_SAND_RET err;
  uint32 ex;
  DNX_SAND_INTERRUPT_INIT_DEFS;
  err = DNX_SAND_OK ;

  /*
   * see above for further explanations. This is the mechanism making sure
   * dnx_sand_module_open() is atomic
   */
  DNX_SAND_INTERRUPTS_STOP;
  {
    if (Dnx_soc_sand_start_module_shut_down_mutex)
    {
      err = DNX_SAND_DRIVER_BUSY ;
      goto exit_busy ;
    }
    Dnx_soc_sand_start_module_shut_down_mutex = TRUE ;
  }
  /*
   * we cannot start an already started driver
   * but we don't want to return an error on this.
   * meaning it's legal to try and open an already
   * openned driver (that way both fap & fe drivers
   * can try and open it without getting an error,
   * and with no regard to the order.
   */
  if (dnx_sand_general_get_driver_is_started())
  {
    if (is_already_opened)
    {
      *is_already_opened = DNX_SAND_DRIVER_ALREADY_STARTED;
    }
    goto exit ;
  }
  else
  {
    if (is_already_opened)
    {
      *is_already_opened = DNX_SAND_OK ;
    }
  }
  /*
   * First add list of DNX_SAND errors and procedures to
   * all-system errors pool and procedure descriptors pool.
   */
  if (dnx_sand_add_sand_errors())
  {
    err = DNX_SAND_USER_ADD_SAND_ERRORS_ERR_01 ;
    goto exit ;
  }
  if (dnx_sand_add_sand_procedures())
  {
    err = DNX_SAND_USER_ADD_SAND_PROCEDURES_ERR_01 ;
    goto exit ;
  }
  /*
   * Now implement error handler to make sure
   * setting the supplied user error handler (might be NULL)
   */
  if (dnx_sand_set_user_error_handler(error_handler, error_description) != DNX_SAND_OK)
  {
    err = DNX_SAND_USER_ERROR_HANDLER_ERR_01 ;
    goto exit ;
  }
  /*
   * End all inner modules - if exist.
   */
  err =  dnx_sand_module_end_all() ;
  if (err != DNX_SAND_OK)
  {
    err = DNX_SAND_MODULE_END_ALL_ERR_01 ;
    goto exit ;
  }
  /*
   * Initilaize all inner modules.
   */
  err = dnx_sand_module_init_all() ;
  if (err != DNX_SAND_OK)
  {
    err = DNX_SAND_MODULE_INIT_ALL_ERR_01 ;
    goto exit ;
  }
  /*
   * setting global static parameter Max_num_devices
   */
  if (dnx_sand_general_set_max_num_devices(max_num_devices) != DNX_SAND_OK)
  {
    err = DNX_SAND_MAX_NUM_DEVICES_OUT_OF_RANGE_ERR ;
    goto exit ;
  }
  /*
   * setting global static parameter System_tick_in_ms
   */
  if (dnx_sand_general_set_system_tick_in_ms(system_tick_in_ms) != DNX_SAND_OK)
  {
    err = DNX_SAND_SYSTEM_TICK_ERR_01 ;
    goto exit ;
  }
  /*
   * setting global static parameter Tcm_task_priority
   */
  if (dnx_sand_general_set_tcm_task_priority(soc_tcmtask_priority) != DNX_SAND_OK)
  {
    err = DNX_SAND_TCM_TASK_PRIORITY_ERR_01 ;
    goto exit ;
  }
  /*
   * setting global static parameter Min_time_between_polls
   */
  if (dnx_sand_general_set_min_time_between_tcm_activation(min_time_between_tcm_activation) != DNX_SAND_OK)
  {
    err = DNX_SAND_MIN_TIME_BETWEEN_POLLS_ERR_01 ;
    goto exit ;
  }
  /*
   * setting global static parameter Tcm_mockup_interrupts
   */
  if (dnx_sand_general_set_tcm_mockup_interrupts(soc_tcmmockup_interrupts) != DNX_SAND_OK)
  {
    err = DNX_SAND_TCM_MOCKUP_INTERRUPTS_ERR_01;
    goto exit;
  }
  /*
   * initilizes the aray of chip descriptors
   */
  if (dnx_sand_init_chip_descriptors(max_num_devices) != DNX_SAND_OK)
  {
    err = DNX_SAND_INIT_CHIP_DESCRIPTORS_ERR_01 ;
    goto exit ;
  }
  /*
   * initilizes the linked list of callback handles
   */
  if (dnx_sand_handles_init_handles() != DNX_SAND_OK)
  {
    err = DNX_SAND_HANDLES_INIT_ERR_01 ;
    goto exit ;
  }
  /*
   * initilizes the Timed Callback Machine (TCM) including:
   * a task that wait on a message queue, and a delta-list.
   */
  /*
   * Clear the indirect information.
   */
  if (dnx_sand_indirect_clear_info_all() != DNX_SAND_OK)
  {
    err = DNX_SAND_INDIRECT_SET_INFO_ERR_01 ;
    goto exit ;
  }
  /*
   * Clear the interrupt mask address information.
   */
  if (dnx_sand_mem_interrupt_mask_address_clear_all() != DNX_SAND_OK)
  {
    err = DNX_SAND_INTERRUPT_CLEAR_ALL_ERR_01 ;
    goto exit ;
  }
  /*
   * after setting this flag the rest of the API can operate
   */
  dnx_sand_general_set_driver_is_started(TRUE) ;
exit:
  Dnx_soc_sand_start_module_shut_down_mutex = FALSE ;
exit_busy:
  dnx_sand_initialize_error_word(DNX_SAND_MODULE_OPEN,0,&ex) ;
  if ( err != DNX_SAND_OK )
  {
    dnx_sand_module_end_all() ;
    dnx_sand_set_error_code_into_error_word(err,&ex) ;
#if DNX_SAND_MODULE_MANAGEMENT_PRINTFS
    LOG_INFO(BSL_LS_SOC_MANAGEMENT,
             (BSL_META("dnx_sand_module_open(): failure. ex = 0x%X \r\n"), (uint32)ex));
#endif
  }
  else
  {
#if DNX_SAND_MODULE_MANAGEMENT_PRINTFS
    LOG_INFO(BSL_LS_SOC_MANAGEMENT,
             (BSL_META("dnx_sand_module_open(): success\r\n")));
#endif
  }
  DNX_SAND_ERROR_REPORT(err,NULL,0,0,DNX_SAND_MODULE_OPEN,
        "error in dnx_sand_module_open(): Use error code",0,0,0,0,0,0) ;
  DNX_SAND_INTERRUPTS_START_IF_STOPPED;
  return (ex) ;
}
/*****************************************************
 * See details in dnx_sand_module_management.h
 *****************************************************/
uint32
  dnx_sand_module_close(
    void
  )
{
  DNX_SAND_RET err;
  uint32 ex;
  DNX_SAND_INTERRUPT_INIT_DEFS;

  err = DNX_SAND_OK ;

  /*
   * see above for further explanations. This is the mechanism making sure
   * dnx_sand_module_close() is atomic
   */
  DNX_SAND_INTERRUPTS_STOP;
  {
    if (Dnx_soc_sand_start_module_shut_down_mutex)
    {
      err = DNX_SAND_DRIVER_BUSY ;
      goto exit ;
    }
    Dnx_soc_sand_start_module_shut_down_mutex = TRUE ;
  }

  /* If any devices are still registered, don't close */
  if (dnx_sand_is_any_chip_descriptor_taken()) {
      goto exit;
  }

  /*
   * We can only shut_down started drivers
   */
  if (!dnx_sand_general_get_driver_is_started())
  {
    err = DNX_SAND_DRIVER_NOT_STARTED ;
    goto exit ;
  }
  /*
   * Mark driver as shut down - no other API calls can enter
   */
  dnx_sand_general_set_driver_is_started(FALSE) ;

  DNX_SAND_INTERRUPTS_START_IF_STOPPED;
  /*
   * stop the TCM (delete task, empty the delta-list)
   */
  /*
   * delete the callback handles linked list
   */
  dnx_sand_handles_shut_down_handles() ;
  /*
   * clears the array of chip descriptors
   */
  dnx_sand_delete_chip_descriptors() ;
  /*
   * Setting the interrupt mask addresses to non-valid address
   */
  dnx_sand_mem_interrupt_mask_address_clear_all() ;
  /*
   * Clear the indirect information.
   */
  dnx_sand_indirect_clear_info_all() ;
  /*
   * Make sure user error handler has been cleared
   * from driver's memory. Also, delete errors queue.
   * Note that it is the user's responsibility to free memory
   * allocated for error description!
   */
  dnx_sand_set_user_error_handler((DNX_SAND_ERROR_HANDLER_PTR)0,(char *)0) ;
  /*
   * close pool of all-system sorted errors.
   */
  dnx_sand_close_all_error_pool() ;
  /*
   * close pool of all-system sorted procedure descriptors.
   */
  dnx_sand_close_all_proc_id_pool() ;
  dnx_sand_module_end_all() ;
exit:
  Dnx_soc_sand_start_module_shut_down_mutex = FALSE ;
  dnx_sand_initialize_error_word(DNX_SAND_MODULE_CLOSE,0,&ex) ;
  if (err != DNX_SAND_OK)
  {
    dnx_sand_set_error_code_into_error_word(err,&ex) ;
  }
  DNX_SAND_ERROR_REPORT(err,NULL,0,0,DNX_SAND_MODULE_CLOSE,
        "error in dnx_sand_module_close(): Use error code",0,0,0,0,0,0) ;
  DNX_SAND_INTERRUPTS_START_IF_STOPPED;
  return (ex) ;
}

