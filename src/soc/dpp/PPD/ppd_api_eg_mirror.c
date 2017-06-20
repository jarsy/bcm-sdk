/* $Id: arad_pp_api_eg_mirror.c,v 1.11 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/

#ifdef _ERR_MSG_MODULE_NAME
  #error "_ERR_MSG_MODULE_NAME redefined"
#endif

#define _ERR_MSG_MODULE_NAME BSL_SOC_EGRESS

/*************
 * INCLUDES  *
 *************/
/* { */
#include <shared/bsl.h>
#include <soc/dcmn/error.h>
#include <soc/dpp/SAND/Utils/sand_header.h>

#include <soc/dpp/SAND/Management/sand_general_macros.h>
#include <soc/dpp/SAND/Management/sand_error_code.h>
#include <soc/dpp/SAND/Utils/sand_os_interface.h>
#include <soc/dpp/SAND/Management/sand_chip_descriptors.h>

#include <soc/dpp/ARAD/ARAD_PP/arad_pp_framework.h>
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_eg_mirror.h>

/* } */
/*************
 * DEFINES   *
 *************/
/* { */

/* } */
/*************
 *  MACROS   *
 *************/
/* { */

/* } */
/*************
 * TYPE DEFS *
 *************/
/* { */

/* } */
/*************
 * GLOBALS   *
 *************/
/* { */

/* } */
/*************
 * FUNCTIONS *
 *************/
/* { */

/*********************************************************************
*     Set outbound mirroring for out-port and VLAN, so all
 *     outgoing packets leave from the given port and with the
 *     given VID will be mirrored or not according to
 *     'enable_mirror'
 *     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  soc_ppd_eg_mirror_port_vlan_add(
    SOC_SAND_IN  int                                     unit,
	SOC_SAND_IN  int                   					 core_id,
    SOC_SAND_IN  SOC_PPC_PORT                                out_port_ndx,
    SOC_SAND_IN  SOC_SAND_PP_VLAN_ID                           vid_ndx,
    SOC_SAND_IN dpp_outbound_mirror_config_t        *config,
    SOC_SAND_OUT SOC_SAND_SUCCESS_FAILURE                          *success


  )
{
  uint32
    res = SOC_SAND_OK;

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_EG_MIRROR_PORT_VLAN_ADD);

  SOC_SAND_CHECK_DRIVER_AND_DEVICE;

  SOC_SAND_CHECK_NULL_INPUT(success);

  res = arad_pp_eg_mirror_port_vlan_add_verify(
          unit,
          out_port_ndx,
          vid_ndx,
          config->mirror_command
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  SOC_SAND_TAKE_DEVICE_SEMAPHORE;

  res = arad_pp_eg_mirror_port_vlan_add_unsafe(
          unit,
		  core_id,
          out_port_ndx,
          vid_ndx,
          success,
          config
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit_semaphore);

exit_semaphore:
  SOC_SAND_GIVE_DEVICE_SEMAPHORE;
  ARAD_PP_DO_NOTHING_AND_EXIT;
exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in soc_ppd_eg_mirror_port_vlan_add()", out_port_ndx, vid_ndx);
}

/*********************************************************************
*     Remove a mirroring for port and VLAN, upon this packet
 *     transmitted out this out_port_ndx and vid_ndx will be
 *     mirrored or not according to default configuration for
 *     out_port_ndx. see soc_ppd_eg_mirror_port_dflt_set()
 *     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  soc_ppd_eg_mirror_port_vlan_remove(
    SOC_SAND_IN  int                                     unit,
	SOC_SAND_IN  int                   					 core_id,
    SOC_SAND_IN  SOC_PPC_PORT                                out_port_ndx,
    SOC_SAND_IN  SOC_SAND_PP_VLAN_ID                           vid_ndx
  )
{
  uint32
    res = SOC_SAND_OK;

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_EG_MIRROR_PORT_VLAN_REMOVE);

  SOC_SAND_CHECK_DRIVER_AND_DEVICE;

  res = arad_pp_eg_mirror_port_vlan_remove_verify(
          unit,
          out_port_ndx,
          vid_ndx
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  SOC_SAND_TAKE_DEVICE_SEMAPHORE;

  res = arad_pp_eg_mirror_port_vlan_remove_unsafe(
          unit,
          core_id,
          out_port_ndx,
          vid_ndx
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit_semaphore);

exit_semaphore:
  SOC_SAND_GIVE_DEVICE_SEMAPHORE;
  ARAD_PP_DO_NOTHING_AND_EXIT;
exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in soc_ppd_eg_mirror_port_vlan_remove()", out_port_ndx, vid_ndx);
}

/*********************************************************************
*     Get the assigned mirroring profile for port and VLAN.
 *     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  soc_ppd_eg_mirror_port_vlan_get(
    SOC_SAND_IN  int                                     unit,
    SOC_SAND_IN  int                                     core_id,
    SOC_SAND_IN  SOC_PPC_PORT                                out_port_ndx,
    SOC_SAND_IN  SOC_SAND_PP_VLAN_ID                           vid_ndx,
    SOC_SAND_OUT dpp_outbound_mirror_config_t        *config

  )
{
  uint32
    res = SOC_SAND_OK;

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_EG_MIRROR_PORT_VLAN_GET);

  SOC_SAND_CHECK_DRIVER_AND_DEVICE;

  SOC_SAND_CHECK_NULL_INPUT(config);

  res = arad_pp_eg_mirror_port_vlan_get_verify(
          unit,
          out_port_ndx,
          vid_ndx
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  SOC_SAND_TAKE_DEVICE_SEMAPHORE;

  res = arad_pp_eg_mirror_port_vlan_get_unsafe(
          unit,
          core_id,
          out_port_ndx,
          vid_ndx,
          config
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit_semaphore);

exit_semaphore:
  SOC_SAND_GIVE_DEVICE_SEMAPHORE;
  ARAD_PP_DO_NOTHING_AND_EXIT;
exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in soc_ppd_eg_mirror_port_vlan_get()", out_port_ndx, vid_ndx);
}

/*********************************************************************
*     Set default mirroring profiles for port
 *     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  soc_ppd_eg_mirror_port_dflt_set(
    SOC_SAND_IN  int                                     unit,
    SOC_SAND_IN  int                                     core_id,
    SOC_SAND_IN  SOC_PPC_PORT                            pp_port,
    SOC_SAND_IN dpp_outbound_mirror_config_t             *config

  )
{
  uint32
    res = SOC_SAND_OK;

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_EG_MIRROR_PORT_DFLT_SET);

  SOC_SAND_CHECK_DRIVER_AND_DEVICE;

  SOC_SAND_CHECK_NULL_INPUT(config);

  res = arad_pp_eg_mirror_port_dflt_set_verify(
          unit,
          pp_port
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  SOC_SAND_TAKE_DEVICE_SEMAPHORE;

  res = arad_pp_eg_mirror_port_dflt_set_unsafe(
          unit,
          core_id,
          pp_port,
          config
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit_semaphore);

exit_semaphore:
  SOC_SAND_GIVE_DEVICE_SEMAPHORE;
  ARAD_PP_DO_NOTHING_AND_EXIT;
exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in soc_ppd_eg_mirror_port_dflt_set()", pp_port, 0);
}

/*********************************************************************
 * Enable or disable mirroring for a port by other (than mirroring) applications.
*********************************************************************/
uint32
  soc_ppd_eg_mirror_port_appl_set(
    SOC_SAND_IN  int        unit,
    SOC_SAND_IN  SOC_PPC_PORT  local_port_ndx,
    SOC_SAND_IN  uint8         enable /* 0 will disable, other values will enable */
  )
{
  uint32 res = SOC_SAND_OK;

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_EG_MIRROR_PORT_APPL_SET);
  SOC_SAND_CHECK_DRIVER_AND_DEVICE;

  SOC_SAND_TAKE_DEVICE_SEMAPHORE;
  res = arad_pp_eg_mirror_port_appl_set_unsafe(unit, local_port_ndx, enable);
  SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit_semaphore);

exit_semaphore:
  SOC_SAND_GIVE_DEVICE_SEMAPHORE;
  ARAD_PP_DO_NOTHING_AND_EXIT;
exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in soc_ppd_eg_mirror_port_appl_set()", local_port_ndx, 0);
}

/*********************************************************************
*     Set default mirroring profiles for port
 *     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  soc_ppd_eg_mirror_port_dflt_get(
    SOC_SAND_IN  int                                     unit,
	SOC_SAND_IN  int                   				     core_id,
    SOC_SAND_IN  SOC_PPC_PORT                                local_port_ndx,
    SOC_SAND_OUT dpp_outbound_mirror_config_t        *config

  )
{
  uint32
    res = SOC_SAND_OK;

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_EG_MIRROR_PORT_DFLT_GET);

  SOC_SAND_CHECK_DRIVER_AND_DEVICE;

  SOC_SAND_CHECK_NULL_INPUT(config);

  res = arad_pp_eg_mirror_port_dflt_get_verify(
          unit,
          local_port_ndx
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  SOC_SAND_TAKE_DEVICE_SEMAPHORE;

  res = arad_pp_eg_mirror_port_dflt_get_unsafe(
          unit,
          core_id,
          local_port_ndx,
          config
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit_semaphore);

exit_semaphore:
  SOC_SAND_GIVE_DEVICE_SEMAPHORE;
  ARAD_PP_DO_NOTHING_AND_EXIT;
exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in soc_ppd_eg_mirror_port_dflt_get()", local_port_ndx, 0);
}

/*********************************************************************
*     Get the port information
*     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  soc_ppd_eg_mirror_port_info_get(
    SOC_SAND_IN  int                                     unit,
    SOC_SAND_IN  int                                     core_id,
    SOC_SAND_IN  SOC_PPC_PORT                                local_port_ndx,
    SOC_SAND_OUT SOC_PPC_EG_MIRROR_PORT_INFO                *info
  )
{
  uint32
    res = SOC_SAND_OK;

  SOC_SAND_INIT_ERROR_DEFINITIONS(SOC_PPC_EG_MIRROR_PORT_INFO_GET);

  SOC_SAND_CHECK_DRIVER_AND_DEVICE;

  SOC_SAND_CHECK_NULL_INPUT(info);

  res = arad_pp_eg_mirror_port_info_get_verify(
          unit,
          local_port_ndx
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  SOC_SAND_TAKE_DEVICE_SEMAPHORE;

  res = arad_pp_eg_mirror_port_info_get_unsafe(
          unit,
          core_id,
          local_port_ndx,
          info
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit_semaphore);

exit_semaphore:
  SOC_SAND_GIVE_DEVICE_SEMAPHORE;
  ARAD_PP_DO_NOTHING_AND_EXIT;
exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in soc_ppd_eg_mirror_port_info_get()", local_port_ndx, 0);
}


/*********************************************************************
* Check if mirroring for a port by other (than mirroring) applications is enabled
*********************************************************************/
uint32
  soc_ppd_eg_mirror_port_appl_get(
    SOC_SAND_IN  int        unit,
    SOC_SAND_IN  SOC_PPC_PORT  local_port_ndx,
    SOC_SAND_OUT uint8         *enabled
  )
{
  uint32 res = SOC_SAND_OK;

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_EG_MIRROR_PORT_DFLT_GET);
  SOC_SAND_CHECK_DRIVER_AND_DEVICE;
  SOC_SAND_CHECK_NULL_INPUT(enabled);

  SOC_SAND_TAKE_DEVICE_SEMAPHORE;
  res = arad_pp_eg_mirror_port_appl_get_unsafe(unit, local_port_ndx, enabled);
  SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit_semaphore);

exit_semaphore:
  SOC_SAND_GIVE_DEVICE_SEMAPHORE;
  ARAD_PP_DO_NOTHING_AND_EXIT;
exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in soc_ppd_eg_mirror_port_dflt_get()", local_port_ndx, 0);
}

/*********************************************************************
*      Set RECYCLE_COMMAND table with trap code
*********************************************************************/
uint32
  soc_ppd_eg_mirror_recycle_command_trap_set(
    SOC_SAND_IN  int        unit,      /* Identifier of the device to access */
    SOC_SAND_IN  uint32        recycle_command, /* Equal to mirror profile */
    SOC_SAND_IN  uint32        trap_code,
    SOC_SAND_IN  uint32        snoop_strength,
    SOC_SAND_IN  uint32        forward_strengh
  )
{
  uint32 res = SOC_SAND_OK;

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_EG_MIRROR_RECYCLE_COMMAND_TRAP_SET);
  SOC_SAND_CHECK_DRIVER_AND_DEVICE;

  res = arad_pp_eg_mirror_recycle_command_trap_set_verify(unit, recycle_command, trap_code, snoop_strength, forward_strengh);
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  SOC_SAND_TAKE_DEVICE_SEMAPHORE;
  res = arad_pp_eg_mirror_recycle_command_trap_set_unsafe(unit, recycle_command, trap_code, snoop_strength, forward_strengh);
  SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit_semaphore);

exit_semaphore:
  SOC_SAND_GIVE_DEVICE_SEMAPHORE;
  ARAD_PP_DO_NOTHING_AND_EXIT;
exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in soc_ppd_eg_mirror_recycle_command_trap_set()", recycle_command, 0);
}

uint32
  soc_ppd_eg_mirror_recycle_command_trap_get(
    SOC_SAND_IN  int        unit,      /* Identifier of the device to access */
    SOC_SAND_IN  uint32        recycle_command, /* Equal to mirror profile */
    SOC_SAND_OUT  uint32       *trap_code,
    SOC_SAND_OUT  uint32       *snoop_strength
  )
{
  uint32 res = SOC_SAND_OK;

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_EG_MIRROR_RECYCLE_COMMAND_TRAP_GET);
  SOC_SAND_CHECK_DRIVER_AND_DEVICE;
  SOC_SAND_CHECK_NULL_INPUT(trap_code);

  res = arad_pp_eg_mirror_recycle_command_trap_get_verify(unit, recycle_command);
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  SOC_SAND_TAKE_DEVICE_SEMAPHORE;
  res = arad_pp_eg_mirror_recycle_command_trap_get_unsafe(unit, recycle_command, trap_code, snoop_strength);
  SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit_semaphore);

exit_semaphore:
  SOC_SAND_GIVE_DEVICE_SEMAPHORE;
  ARAD_PP_DO_NOTHING_AND_EXIT;
exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in soc_ppd_eg_mirror_recycle_command_trap_get()", recycle_command, 0);
}


/* } */

#include <soc/dpp/SAND/Utils/sand_footer.h>


