/* $Id: sand_general_params.c,v 1.8 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 * $
*/
/* $Id: sand_general_params.c,v 1.8 Broadcom SDK $
 */


#include <shared/bsl.h>

#include <soc/dnx/legacy/SAND/Management/sand_general_params.h>
#include <soc/dnx/legacy/SAND/Management/sand_error_code.h>
#include <soc/dnx/legacy/SAND/Management/sand_chip_descriptors.h>
#include <soc/dnx/legacy/SAND/Management/sand_low_level.h>

#include <soc/dnx/legacy/SAND/Utils/sand_tcm.h>
#include <soc/dnx/legacy/SAND/Utils/sand_os_interface.h>
/*{*/
/*
 * Static, general parameters of the driver
 * {
 */
/*
 * Version management of this driver
 * {
 */

/*
 * Driver version - for version releases.
 * Use 'dnx_sand_version_to_string()' to get
 * formatted string of the version.
 */
static const
  unsigned
    long
      Dnx_soc_sand_ver = 7110;

/*****************************************************
*NAME
*  dnx_soc_get_sand_string_ver
*TYPE: PROC
*DATE: 30/JAN/2003
*FUNCTION:
*  Get DNX_SAND driver version of current driver,
*  in string format.
*  Example, if 'Dnx_soc_sand_ver' value is 1300,
*  'string_buff' will be loaded with "1.300"
*CALLING SEQUENCE:
*  dnx_soc_get_sand_string_ver(string_buff)
*INPUT:
*  DNX_SAND_DIRECT:
*    Pointer to buffer of size 'DNX_SAND_VER_STRING_SIZE'.
*  DNX_SAND_INDIRECT:
*    The global variable Dnx_soc_sand_ver.
*OUTPUT:
*  DNX_SAND_DIRECT:
*    None.
*  DNX_SAND_INDIRECT:
*    string_buff
*REMARKS:
*  None.
*SEE ALSO:
*****************************************************/
void
    dnx_soc_get_sand_string_ver(
      char string_buff[DNX_SAND_VER_STRING_SIZE]
    )
{

  if (NULL == string_buff)
  {
    goto exit;
  }

  dnx_sand_version_to_string(Dnx_soc_sand_ver, string_buff);

exit:
  return;
}

/*****************************************************
*NAME
*  dnx_soc_get_sand_ver
*TYPE: PROC
*DATE: 30/JAN/2003
*FUNCTION:
*  Get version of current driver.
*CALLING SEQUENCE:
*  dnx_soc_get_sand_ver()
*INPUT:
*  DNX_SAND_DIRECT:
*    None.
*  DNX_SAND_INDIRECT:
*    The global variable Dnx_soc_sand_ver.
*OUTPUT:
*  DNX_SAND_DIRECT:
*    uint32 -
*      Driver software version.
*  DNX_SAND_INDIRECT:
*    None.
*REMARKS:
*  None.
*SEE ALSO:
*****************************************************/
unsigned
  long
    dnx_soc_get_sand_ver(
      void
    )
{
  int
    ret ;
  ret = Dnx_soc_sand_ver ;
  return (ret) ;
}

/*****************************************************
*NAME
*  dnx_sand_version_to_string
*TYPE: PROC
*DATE: 1/MAY/2004
*FUNCTION:
*  Convert from 'uint32'  to string.
*INPUT:
*  DNX_SAND_DIRECT:
*    DNX_SAND_IN    uint32  version -
*      Version number.
*    DNX_SAND_INOUT char           string_buff[DNX_SAND_VER_STRING_SIZE] -
*      Buffer of size 'DNX_SAND_VER_STRING_SIZE' bytes.
*  DNX_SAND_INDIRECT:
*    None.
*OUTPUT:
*  DNX_SAND_DIRECT:
*    DNX_SAND_INOUT char           string_buff[DNX_SAND_VER_STRING_SIZE] -
*      Loaded with string of the version.
*  DNX_SAND_INDIRECT:
*    None.
*REMARKS:
*  None.
*SEE ALSO:
*****************************************************/
void
  dnx_sand_version_to_string(
    DNX_SAND_IN    uint32  version,
    DNX_SAND_INOUT char           string_buff[DNX_SAND_VER_STRING_SIZE]
  )
{

  if (NULL == string_buff)
  {
    goto exit;
  }

  sal_sprintf(
    string_buff,
    "%u.%03u", version/1000, version%1000
  );

exit:
  return;
}



/*
 * }
 */
/*
 * The params
 * {
 */
uint32  Dnx_soc_sand_max_num_devices                 = DNX_SAND_MAX_DEVICE;
uint32  Dnx_soc_sand_system_ticks_in_ms              = 0;
uint32  Dnx_soc_sand_min_time_between_tcm_activation = DNX_SAND_MIN_TCM_ACTIVATION_PERIOD;
uint32  Dnx_soc_sand_driver_is_started               = 0;
uint32  Dnx_soc_sand_tcm_mockup_interrupts        = 0;
/*
 * }
 * End of params
 */
/*
 * get/set Dnx_soc_sand_max_num_devices
 * {
 */
uint32
  dnx_sand_general_get_max_num_devices(
    void
  )
{
  return Dnx_soc_sand_max_num_devices ;
}
/*
 */
DNX_SAND_RET
  dnx_sand_general_set_max_num_devices(
    uint32  max_num_devices
  )
{
  DNX_SAND_RET
    ex = DNX_SAND_ERR ;
  uint32
    err = 0 ;
  if (DNX_SAND_MAX_DEVICE < max_num_devices)
  {
    err = 1 ;
    goto exit ;
  }
  Dnx_soc_sand_max_num_devices = max_num_devices ;
  ex = DNX_SAND_OK ;
exit:
  DNX_SAND_ERROR_REPORT(ex,NULL,0,0,DNX_SAND_GENERAL_SET_MAX_NUM_DEVICES,
        "General error in dnx_sand_general_set_max_num_devices()",err,0,0,0,0,0) ;
  return ex ;
}
/*
 * end of get/set Dnx_soc_sand_max_num_devices
 * }
 */
/*
 * get/set System_tick_in_ms
 * {
 */
uint32
  dnx_sand_general_get_system_tick_in_ms(
    void
  )
{
  return Dnx_soc_sand_system_ticks_in_ms ;
}
/*
 */
DNX_SAND_RET
  dnx_sand_general_set_system_tick_in_ms(
    uint32 system_tick_in_ms
  )
{
  Dnx_soc_sand_system_ticks_in_ms = system_tick_in_ms ;
  return DNX_SAND_OK ;
}
/*
 * }
 * get/set System_tick_in_ms
 */
/*
 * get/set Tcm_task_priority
 * {
 */
uint32
  dnx_sand_general_get_tcm_task_priority(
    void
  )
{
  return dnx_sand_tcm_get_task_priority() ;
}
/*
 */
DNX_SAND_RET
  dnx_sand_general_set_tcm_task_priority(
    uint32 soc_tcmtask_priority
  )
{
  dnx_sand_tcm_set_task_priority(soc_tcmtask_priority) ;
  return DNX_SAND_OK ;
}
/*
 * }
 * get/set Tcm_task_priority
 */
/*
 * get/set Min_time_between_polls
 * {
 */
uint32
  dnx_sand_general_get_min_time_between_tcm_activation(
    void
  )
{
  return Dnx_soc_sand_min_time_between_tcm_activation;
}
/*
 */
DNX_SAND_RET
  dnx_sand_general_set_min_time_between_tcm_activation(
    uint32 min_time_between_tcm_activation
  )
{
  if(min_time_between_tcm_activation < DNX_SAND_MIN_TCM_ACTIVATION_PERIOD)
  {
    return DNX_SAND_ERR;
  }
  Dnx_soc_sand_min_time_between_tcm_activation = min_time_between_tcm_activation;
  return DNX_SAND_OK ;
}
/*
 * }
 * get/set Dnx_soc_sand_min_time_between_tcm_activation
 */
/*
 * get/set Dnx_soc_sand_driver_is_started
 * {
 */
uint32
  dnx_sand_general_get_driver_is_started(
    void
  )
{
  return Dnx_soc_sand_driver_is_started;
}
/*
 */
void
  dnx_sand_general_set_driver_is_started(
    uint32  driver_is_started
  )
{
  Dnx_soc_sand_driver_is_started = driver_is_started ;
}
/*
 * }
 * get/set Dnx_soc_sand_driver_is_started
 */
/*
 * get/set Dnx_soc_sand_tcm_mockup_interrupts
 * {
 */
uint32
  dnx_sand_general_get_tcm_mockup_interrupts(
    void
  )
{
  return Dnx_soc_sand_tcm_mockup_interrupts;
}
/*
 */
DNX_SAND_RET
  dnx_sand_general_set_tcm_mockup_interrupts(
    uint32 soc_tcmmockup_interrupts
  )
{
  Dnx_soc_sand_tcm_mockup_interrupts = soc_tcmmockup_interrupts;
  return DNX_SAND_OK;
}
/*
 * }
 * get/set Dnx_soc_sand_tcm_mockup_interrupts
 */
/*
 * End of static params section
 * }
 */

#if DNX_SAND_DEBUG
/*
 * Utility for printing general dnx_sand status
 *  1. Whether the driver is running.
 *  2. Various general driver parameters
 */
void
  dnx_sand_status_print(
    void
  )
{
  if (dnx_sand_general_get_driver_is_started() == FALSE)
  {
    LOG_CLI((BSL_META("DNX_SAND driver is not running\n\r")));
    goto exit;
  }
  LOG_CLI((BSL_META("DNX_SAND driver is running\n\r")));
  /*
   * Prints the registed device info.
   */
  dnx_sand_chip_descriptors_print();
  /*
   * Prints the TCM info.
   */
  dnx_sand_tcm_general_status_print();
  /*
   * Print low access info.
   */
  LOG_CLI((BSL_META("\n\r")));
  dnx_sand_physical_access_print();

exit:
  return;
}
#endif
