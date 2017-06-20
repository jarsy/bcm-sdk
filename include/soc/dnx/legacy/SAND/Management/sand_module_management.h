/* $Id: sand_module_management.h,v 1.6 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 * $
*/
/******************************************************************
*
* FILENAME:       dnx_sand_module_management.h
*
* AUTHOR:         Dune (S.Z.)
*
* FILE DESCRIPTION:
* The module management is a set of functions that are
* used by the application to open and close the driver module.
* These functions take care of initializing the driver by allocating
* memory and RTOS resources needed by the driver
* REMARKS:
* SW License Agreement: Dune Networks (c). CONFIDENTIAL PROPRIETARY INFORMATION.
* Any use of this Software is subject to Software License Agreement
* included in the Driver User Manual of this device.
* Any use of this Software constitutes an agreement to the terms
* of the above Software License Agreement.
******************************************************************/
#ifndef __DNX_SAND_MODULE_MANAGEMENT_H_INCLUDED__
/* { */
#define __DNX_SAND_MODULE_MANAGEMENT_H_INCLUDED__
#ifdef  __cplusplus
extern "C" {
#endif
/**/
#include <soc/dnx/legacy/SAND/Utils/sand_framework.h>
#include <soc/dnx/legacy/SAND/SAND_FM/sand_indirect_access.h>

extern int Dnx_soc_sand_start_module_shut_down_mutex;

/*****************************************************
*NAME
*  dnx_sand_module_open
*TYPE:
*  PROC
*DATE:
*  25/SEP/2002
*FUNCTION:
*  Initialize all resources related to the driver
*  (semaphore, callback engine memory, message queue
*  for ISR) and starts the TCM (Timed Callback Machine)
*  task.
*INPUT:
*  DNX_SAND_DIRECT:
*    uint32 max_num_devices -
*      Maximal number of devices allowed to register with
*      this driver on this session. This number can not
*      be larger than DNX_SAND_MAX_DEVICE.
*    uint32 system_tick_in_ms -
*      System tick, in milliseconds. If system tick can
*      not be precisely expressed in milliseconds then some
*      rounding errors (in time calculations) are to
*      be expected when absolute time calculations are
*      made. These are ignored.
*    uint32 soc_tcmtask_priority -
*      Priority to assign to TCM task.
*    uint32 min_time_between_tcm_activation -
*      Minimal time between two consecutive
*      activations of TCM task, in system ticks,
*      on this session, due to standard polling.
*      This limits the CPU load due to TCM task and,
*      accordingly, the precision of time periods
*      requested by the user in commands involved
*      with polling. This number can not be smaller
*      than DNX_SAND_MIN_TCM_ACTIVATION_PERIOD.
*      This parameter is always meaningful since
*      polling is always activated.
*    uint32 soc_tcmmockup_interrupts -
*      In case user does not want to connect the interrupt
*      line of his devices to the cpu, this driver can mock
*      up interrupt handling by direct calling the interrupt
*      handling routine.
*    DNX_SAND_ERROR_HANDLER_PTR error_handler -
*      Pointer to user-supplied procedure to call whenever
*      an error is detected in the driver. If this variable
*      is null then no extra error handling is required
*      and 'error_description' below is not meaningful.
*    char *error_description -
*      Pointer to user-supplied buffer to contain null
*      terminated string with error description. Buffer
*      is of size DNX_SAND_CALLBACK_BUF_SIZE and is replaced
*      by the user every time 'user_error_handler' is invoked.
*      Note bthat at 'dnx_sand_module_close', user error handler
*      is cleared from driver's memory. Also, it is the
*      user's responsibility to free memory allocated for
*      error description!
*    uint32      *is_already_opened -
*      Pointer to a user supplied buffer of uint32 size.
*      If driver has not already been opened, it is loaded
*      by DNX_SAND_OK.
*      Otherwise, in case the driver was already opened,
*      this variable is loaded with the value:
*      DNX_SAND_DRIVER_ALREADY_STARTED.
*      In this case the above parameters passed to this methid
*      are ignored.
*  DNX_SAND_INDIRECT:
*    None.
*OUTPUT:
*  DNX_SAND_DIRECT:
*    uint32 -
*      See formatting rules in ERROR RETURN VALUES in
*      dnx_sand_error_code.h.
*      If error code is not  then
*        specific error codes:
*          DNX_SAND_MIN_TIME_BETWEEN_INT_ERR_01 -
*            Input variable 'min_time_between_int_handlings' out
*            of range.
*          DNX_SAND_MAX_NUM_DEVICES_OUT_OF_RANGE_ERR -
*            Input variable 'max_num_devices' out
*            of range.
*          DNX_SAND_SYSTEM_TICK_ERR_01 -
*            Illegal system tick value (in ms).
*          DNX_SAND_TCM_TASK_PRIORITY_ERR_01 -
*            Input variable 'soc_tcmtask_priority' out
*            of range.
*          DNX_SAND_MIN_TIME_BETWEEN_POLLS_ERR_01 -
*            Input variable 'min_time_between_polls' out
*            of range.
*          DNX_SAND_USER_ERROR_HANDLER_ERR_01 -
*            Procedure dnx_sand_set_user_error_handler() returned with
*            error indication.
*          DNX_SAND_INIT_CHIP_DESCRIPTORS_ERR_01 -
*            Procedure dnx_sand_init_chip_descriptors() returned with
*            error indication.
*          DNX_SAND_HANDLES_INIT_ERR_01 -
*            Procedure dnx_sand_handles_init_handles() returned with
*            error indication.
*          DNX_SAND_TCM_CALLBACK_ENGINE_START_ERR_01 -
*            Procedure dnx_sand_tcm_callback_engine_start() returned with
*            error indication.
*          DNX_SAND_INDIRECT_SET_INFO_ERR_01 -
*            Procedure dnx_sand_indirect_clear_info_all() returned with
*            error indication.
*          DNX_SAND_DRIVER_BUSY -
*            Driver is busy. Try again, after timeout.
*          DNX_SAND_MODULE_END_ALL_ERR_01 -
*            Procedure dnx_sand_module_end_all() returned with
*            error indication.
*          DNX_SAND_MODULE_INIT_ALL_ERR_01 -
*            Procedure dnx_sand_module_init_all() returned with
*            error indication.
*          DNX_SAND_INTERRUPT_CLEAR_ALL_ERR_01 -
*            Action of clearing all devices interrupt mask address
*            information has failed.
*      Otherwise, no error has been detected and driver
*        has been started.
*  DNX_SAND_INDIRECT:
*    If no error condition encountered: allocated resources,
*    started task.
*REMARKS:
*  None
*SEE ALSO:
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
  );
/*****************************************************
*NAME
*  dnx_sand_module_close
*TYPE:
*  PROC
*DATE:
*  25/SEP/2002
*FUNCTION:
*  This procedure releases all resources allocated
*  within the driver module by any of its elements
*  (memory,semaphores, message queues, registered devices).
*  Then it shuts down the TCM task.
*INPUT:
*  DNX_SAND_DIRECT:
*    None.
*  DNX_SAND_INDIRECT:
*    None.
*OUTPUT:
*  DNX_SAND_DIRECT:
*    uint32 -
*      See formatting rules in ERROR RETURN VALUES in
*      dnx_sand_error_code.h.
*      If error code is not DNX_SAND_NO_ERR then
*        specific error codes:
*          DNX_SAND_DRIVER_NOT_STARTED -
*            Can not close a driver which has not been started.
*          DNX_SAND_DRIVER_BUSY -
*            Driver is busy. Try again, after timeout.
*      Otherwise, no error has been detected and driver
*        has been shut down.
*  DNX_SAND_INDIRECT:
*    If no error condition: released resources,
*    task deleted.
*REMARKS:
*  None
*SEE ALSO:
*****************************************************/
uint32
  dnx_sand_module_close(
    void
  );
/**/
#ifdef  __cplusplus
}
#endif
/* } __DNX_SAND_MODULE_MANAGEMENT_H_INCLUDED__*/
#endif
