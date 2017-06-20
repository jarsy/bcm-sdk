/* $Id: arad_pp_api_llp_mirror.c,v 1.6 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/

#ifdef _ERR_MSG_MODULE_NAME
  #error "_ERR_MSG_MODULE_NAME redefined"
#endif

#define _ERR_MSG_MODULE_NAME BSL_SOC_LLP

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
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_llp_mirror.h>

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
*     Set a mirroring for port and VLAN, so all incoming
 *     packets enter from the given port and identified with
 *     the given VID will be associated with Mirror command
 *     (Enables mirroring (copying) the packets to additional
 *     destination.
 *     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  soc_ppd_llp_mirror_port_vlan_add(
    SOC_SAND_IN  int                                     unit,
	SOC_SAND_IN  int                                     core_id,
    SOC_SAND_IN  SOC_PPC_PORT                                local_port_ndx,
    SOC_SAND_IN  SOC_SAND_PP_VLAN_ID                           vid_ndx,
    SOC_SAND_IN  uint32                                      mirror_profile,
    SOC_SAND_OUT SOC_SAND_SUCCESS_FAILURE                          *success
  )
{
  uint32
    res = SOC_SAND_OK;

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_LLP_MIRROR_PORT_VLAN_ADD);

  SOC_SAND_CHECK_DRIVER_AND_DEVICE;

  SOC_SAND_CHECK_NULL_INPUT(success);

  res = arad_pp_llp_mirror_port_vlan_add_verify(
          unit,
          local_port_ndx,
          vid_ndx,
          mirror_profile
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  SOC_SAND_TAKE_DEVICE_SEMAPHORE;

  res = arad_pp_llp_mirror_port_vlan_add_unsafe(
          unit,
		  core_id,
          local_port_ndx,
          vid_ndx,
          mirror_profile,
          success
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit_semaphore);

exit_semaphore:
  SOC_SAND_GIVE_DEVICE_SEMAPHORE;
  ARAD_PP_DO_NOTHING_AND_EXIT;
exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in soc_ppd_llp_mirror_port_vlan_add()", local_port_ndx, vid_ndx);
}

/*********************************************************************
*     Remove a mirroring for port and VLAN
 *     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  soc_ppd_llp_mirror_port_vlan_remove(
    SOC_SAND_IN  int                                     unit,
	SOC_SAND_IN  int                                     core_id,
    SOC_SAND_IN  SOC_PPC_PORT                                local_port_ndx,
    SOC_SAND_IN  SOC_SAND_PP_VLAN_ID                           vid_ndx
  )
{
  uint32
    res = SOC_SAND_OK;

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_LLP_MIRROR_PORT_VLAN_REMOVE);

  SOC_SAND_CHECK_DRIVER_AND_DEVICE;

  res = arad_pp_llp_mirror_port_vlan_remove_verify(
          unit,
          local_port_ndx,
          vid_ndx
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  SOC_SAND_TAKE_DEVICE_SEMAPHORE;

  res = arad_pp_llp_mirror_port_vlan_remove_unsafe(
          unit,
		  core_id,
          local_port_ndx,
          vid_ndx
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit_semaphore);

exit_semaphore:
  SOC_SAND_GIVE_DEVICE_SEMAPHORE;
  ARAD_PP_DO_NOTHING_AND_EXIT;
exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in soc_ppd_llp_mirror_port_vlan_remove()", local_port_ndx, vid_ndx);
}

/*********************************************************************
*     Get the assigned mirroring profile for port and VLAN.
 *     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  soc_ppd_llp_mirror_port_vlan_get(
    SOC_SAND_IN  int                                     unit,
	SOC_SAND_IN  int                                     core_id,
    SOC_SAND_IN  SOC_PPC_PORT                                local_port_ndx,
    SOC_SAND_IN  SOC_SAND_PP_VLAN_ID                           vid_ndx,
    SOC_SAND_OUT uint32                                      *mirror_profile
  )
{
  uint32
    res = SOC_SAND_OK;

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_LLP_MIRROR_PORT_VLAN_GET);

  SOC_SAND_CHECK_DRIVER_AND_DEVICE;

  SOC_SAND_CHECK_NULL_INPUT(mirror_profile);

  res = arad_pp_llp_mirror_port_vlan_get_verify(
          unit,
          local_port_ndx,
          vid_ndx
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  SOC_SAND_TAKE_DEVICE_SEMAPHORE;

  res = arad_pp_llp_mirror_port_vlan_get_unsafe(
          unit,
		  core_id,
          local_port_ndx,
          vid_ndx,
          mirror_profile
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit_semaphore);

exit_semaphore:
  SOC_SAND_GIVE_DEVICE_SEMAPHORE;
  ARAD_PP_DO_NOTHING_AND_EXIT;
exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in soc_ppd_llp_mirror_port_vlan_get()", local_port_ndx, vid_ndx);
}

/*********************************************************************
*     Set default mirroring profiles for port
 *     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  soc_ppd_llp_mirror_port_dflt_set(
    SOC_SAND_IN  int                                     unit,
	SOC_SAND_IN  int                                     core_id,
    SOC_SAND_IN  SOC_PPC_PORT                                local_port_ndx,
    SOC_SAND_IN  SOC_PPC_LLP_MIRROR_PORT_DFLT_INFO               *dflt_mirroring_info
  )
{
  uint32
    res = SOC_SAND_OK;

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_LLP_MIRROR_PORT_DFLT_SET);

  SOC_SAND_CHECK_DRIVER_AND_DEVICE;

  SOC_SAND_CHECK_NULL_INPUT(dflt_mirroring_info);

  res = arad_pp_llp_mirror_port_dflt_set_verify(
          unit,
          local_port_ndx,
          dflt_mirroring_info
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  SOC_SAND_TAKE_DEVICE_SEMAPHORE;

  res = arad_pp_llp_mirror_port_dflt_set_unsafe(
          unit,
		  core_id,
          local_port_ndx,
          dflt_mirroring_info
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit_semaphore);

exit_semaphore:
  SOC_SAND_GIVE_DEVICE_SEMAPHORE;
  ARAD_PP_DO_NOTHING_AND_EXIT;
exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in soc_ppd_llp_mirror_port_dflt_set()", local_port_ndx, 0);
}

/*********************************************************************
*     Set default mirroring profiles for port
 *     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  soc_ppd_llp_mirror_port_dflt_get(
    SOC_SAND_IN  int                                     unit,
	SOC_SAND_IN  int                                     core_id,
    SOC_SAND_IN  SOC_PPC_PORT                                local_port_ndx,
    SOC_SAND_OUT SOC_PPC_LLP_MIRROR_PORT_DFLT_INFO               *dflt_mirroring_info
  )
{
  uint32
    res = SOC_SAND_OK;

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_LLP_MIRROR_PORT_DFLT_GET);

  SOC_SAND_CHECK_DRIVER_AND_DEVICE;

  SOC_SAND_CHECK_NULL_INPUT(dflt_mirroring_info);

  res = arad_pp_llp_mirror_port_dflt_get_verify(
          unit,
          local_port_ndx
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  SOC_SAND_TAKE_DEVICE_SEMAPHORE;

  res = arad_pp_llp_mirror_port_dflt_get_unsafe(
          unit,
		  core_id,
          local_port_ndx,
          dflt_mirroring_info
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit_semaphore);

exit_semaphore:
  SOC_SAND_GIVE_DEVICE_SEMAPHORE;
  ARAD_PP_DO_NOTHING_AND_EXIT;
exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in soc_ppd_llp_mirror_port_dflt_get()", local_port_ndx, 0);
}

/* } */

#include <soc/dpp/SAND/Utils/sand_footer.h>



