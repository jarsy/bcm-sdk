/* $Id: arad_pp_api_llp_filter.c,v 1.5 Broadcom SDK $
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
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_llp_filter.h>

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
*     Sets ingress VLAN membership; which incoming local ports
 *     belong to the VLAN. Packets received on a port that is
 *     not a member of the VLAN the packet is classified to be
 *     filtered.
 *     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  soc_ppd_llp_filter_ingress_vlan_membership_set(
    SOC_SAND_IN  int                                     unit,
	SOC_SAND_IN   int                                    core_id,
    SOC_SAND_IN  SOC_SAND_PP_VLAN_ID                               vid_ndx,
    SOC_SAND_IN  uint32                                      ports[SOC_PPC_VLAN_MEMBERSHIP_BITMAP_SIZE]
  )
{
  uint32
    res = SOC_SAND_OK;

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_LLP_FILTER_INGRESS_VLAN_MEMBERSHIP_SET);

  SOC_SAND_CHECK_DRIVER_AND_DEVICE;

  res = arad_pp_llp_filter_ingress_vlan_membership_set_verify(
          unit,
          vid_ndx,
          ports
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  SOC_SAND_TAKE_DEVICE_SEMAPHORE;

  res = arad_pp_llp_filter_ingress_vlan_membership_set_unsafe(
          unit,
		  core_id,
          vid_ndx,
          ports
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit_semaphore);

exit_semaphore:
  SOC_SAND_GIVE_DEVICE_SEMAPHORE;
  ARAD_PP_DO_NOTHING_AND_EXIT;
exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in soc_ppd_llp_filter_ingress_vlan_membership_set()", vid_ndx, 0);
}

/*********************************************************************
*     Sets ingress VLAN membership; which incoming local ports
 *     belong to the VLAN. Packets received on a port that is
 *     not a member of the VLAN the packet is classified to be
 *     filtered.
 *     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  soc_ppd_llp_filter_ingress_vlan_membership_get(
    SOC_SAND_IN  int                                 unit,
	SOC_SAND_IN   int                                core_id,
    SOC_SAND_IN  SOC_SAND_PP_VLAN_ID                           vid_ndx,
    SOC_SAND_OUT uint32                                  ports[SOC_PPC_VLAN_MEMBERSHIP_BITMAP_SIZE]
  )
{
  uint32
    res = SOC_SAND_OK;

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_LLP_FILTER_INGRESS_VLAN_MEMBERSHIP_GET);

  SOC_SAND_CHECK_DRIVER_AND_DEVICE;

  res = arad_pp_llp_filter_ingress_vlan_membership_get_verify(
          unit,
          vid_ndx
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  SOC_SAND_TAKE_DEVICE_SEMAPHORE;

  res = arad_pp_llp_filter_ingress_vlan_membership_get_unsafe(
          unit,
		  core_id,
          vid_ndx,
          ports
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit_semaphore);

exit_semaphore:
  SOC_SAND_GIVE_DEVICE_SEMAPHORE;
  ARAD_PP_DO_NOTHING_AND_EXIT;
exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in soc_ppd_llp_filter_ingress_vlan_membership_get()", vid_ndx, 0);
}

/*********************************************************************
*     Add a local port as a member in a VLAN.
 *     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  soc_ppd_llp_filter_ingress_vlan_membership_port_add(
    SOC_SAND_IN  int                                 unit,
	SOC_SAND_IN   int                                core_id,
    SOC_SAND_IN  SOC_SAND_PP_VLAN_ID                           vid_ndx,
    SOC_SAND_IN  SOC_PPC_PORT                              local_port
  )
{
  uint32
    res = SOC_SAND_OK;

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_LLP_FILTER_INGRESS_VLAN_MEMBERSHIP_PORT_ADD);

  SOC_SAND_CHECK_DRIVER_AND_DEVICE;

  res = arad_pp_llp_filter_ingress_vlan_membership_port_add_verify(
          unit,
          vid_ndx,
          local_port
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  SOC_SAND_TAKE_DEVICE_SEMAPHORE;

  res = arad_pp_llp_filter_ingress_vlan_membership_port_add_unsafe(
          unit,
		  core_id,
          vid_ndx,
          local_port
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit_semaphore);

exit_semaphore:
  SOC_SAND_GIVE_DEVICE_SEMAPHORE;
  ARAD_PP_DO_NOTHING_AND_EXIT;
exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in soc_ppd_llp_filter_ingress_vlan_membership_port_add()", vid_ndx, 0);
}

/*********************************************************************
*     Remove a local port from the VLAN membership.
 *     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  soc_ppd_llp_filter_ingress_vlan_membership_port_remove(
    SOC_SAND_IN  int                                 unit,
	SOC_SAND_IN   int                                core_id,
    SOC_SAND_IN  SOC_SAND_PP_VLAN_ID                           vid_ndx,
    SOC_SAND_IN  SOC_PPC_PORT                              local_port
  )
{
  uint32
    res = SOC_SAND_OK;

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_LLP_FILTER_INGRESS_VLAN_MEMBERSHIP_PORT_REMOVE);

  SOC_SAND_CHECK_DRIVER_AND_DEVICE;

  res = arad_pp_llp_filter_ingress_vlan_membership_port_remove_verify(
          unit,
          vid_ndx,
          local_port
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  SOC_SAND_TAKE_DEVICE_SEMAPHORE;

  res = arad_pp_llp_filter_ingress_vlan_membership_port_remove_unsafe(
          unit,
		  core_id,
          vid_ndx,
          local_port
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit_semaphore);

exit_semaphore:
  SOC_SAND_GIVE_DEVICE_SEMAPHORE;
  ARAD_PP_DO_NOTHING_AND_EXIT;
exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in soc_ppd_llp_filter_ingress_vlan_membership_port_remove()", vid_ndx, 0);
}

/*********************************************************************
*     Sets acceptable frame type on incoming port.
 *     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  soc_ppd_llp_filter_ingress_acceptable_frames_set(
    SOC_SAND_IN  int                                     unit,
    SOC_SAND_IN  uint32                                      port_profile_ndx,
    SOC_SAND_IN  SOC_SAND_PP_ETHERNET_FRAME_VLAN_FORMAT            vlan_format_ndx,
    SOC_SAND_IN  SOC_PPC_ACTION_PROFILE                        *action_profile,
    SOC_SAND_OUT SOC_SAND_SUCCESS_FAILURE                          *success
  )
{
  uint32
    res = SOC_SAND_OK;

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_LLP_FILTER_INGRESS_ACCEPTABLE_FRAMES_SET);

  SOC_SAND_CHECK_DRIVER_AND_DEVICE;

  SOC_SAND_CHECK_NULL_INPUT(action_profile);
  SOC_SAND_CHECK_NULL_INPUT(success);

  res = arad_pp_llp_filter_ingress_acceptable_frames_set_verify(
          unit,
          port_profile_ndx,
          vlan_format_ndx,
          action_profile
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  SOC_SAND_TAKE_DEVICE_SEMAPHORE;

  res = arad_pp_llp_filter_ingress_acceptable_frames_set_unsafe(
          unit,
          port_profile_ndx,
          vlan_format_ndx,
          action_profile,
          success
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit_semaphore);

exit_semaphore:
  SOC_SAND_GIVE_DEVICE_SEMAPHORE;
  ARAD_PP_DO_NOTHING_AND_EXIT;
exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in soc_ppd_llp_filter_ingress_acceptable_frames_set()", port_profile_ndx, 0);
}

/*********************************************************************
*     Sets acceptable frame type on incoming port.
 *     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  soc_ppd_llp_filter_ingress_acceptable_frames_get(
    SOC_SAND_IN  int                                     unit,
    SOC_SAND_IN  uint32                                      port_profile_ndx,
    SOC_SAND_IN  SOC_SAND_PP_ETHERNET_FRAME_VLAN_FORMAT            vlan_format_ndx,
    SOC_SAND_OUT SOC_PPC_ACTION_PROFILE                        *action_profile
  )
{
  uint32
    res = SOC_SAND_OK;

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_LLP_FILTER_INGRESS_ACCEPTABLE_FRAMES_GET);

  SOC_SAND_CHECK_DRIVER_AND_DEVICE;

  SOC_SAND_CHECK_NULL_INPUT(action_profile);

  res = arad_pp_llp_filter_ingress_acceptable_frames_get_verify(
          unit,
          port_profile_ndx,
          vlan_format_ndx
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  SOC_SAND_TAKE_DEVICE_SEMAPHORE;

  res = arad_pp_llp_filter_ingress_acceptable_frames_get_unsafe(
          unit,
          port_profile_ndx,
          vlan_format_ndx,
          action_profile
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit_semaphore);

exit_semaphore:
  SOC_SAND_GIVE_DEVICE_SEMAPHORE;
  ARAD_PP_DO_NOTHING_AND_EXIT;
exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in soc_ppd_llp_filter_ingress_acceptable_frames_get()", port_profile_ndx, 0);
}

/*********************************************************************
*     Set a designated VLAN for a port. Incoming Trill packet
 *     will be checked if it has this T-VID; otherwise, packet
 *     will be dropped.
 *     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  soc_ppd_llp_filter_designated_vlan_set(
    SOC_SAND_IN  int                                     unit,
	SOC_SAND_IN   int                                    core_id,
    SOC_SAND_IN  SOC_PPC_PORT                                  local_port_ndx,
    SOC_SAND_IN  SOC_SAND_PP_VLAN_ID                               vid,
    SOC_SAND_IN  uint8                                     accept,
    SOC_SAND_OUT SOC_SAND_SUCCESS_FAILURE                          *success
  )
{
  uint32
    res = SOC_SAND_OK;

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_LLP_FILTER_DESIGNATED_VLAN_SET);

  SOC_SAND_CHECK_DRIVER_AND_DEVICE;

  SOC_SAND_CHECK_NULL_INPUT(success);

  res = arad_pp_llp_filter_designated_vlan_set_verify(
          unit,
          local_port_ndx,
          vid,
          accept
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  SOC_SAND_TAKE_DEVICE_SEMAPHORE;

  res = arad_pp_llp_filter_designated_vlan_set_unsafe(
          unit,
		  core_id,
          local_port_ndx,
          vid,
          accept,
          success
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit_semaphore);

exit_semaphore:
  SOC_SAND_GIVE_DEVICE_SEMAPHORE;
  ARAD_PP_DO_NOTHING_AND_EXIT;
exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in soc_ppd_llp_filter_designated_vlan_set()", local_port_ndx, 0);
}

/*********************************************************************
*     Set a designated VLAN for a port. Incoming Trill packet
 *     will be checked if it has this T-VID; otherwise, packet
 *     will be dropped.
 *     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  soc_ppd_llp_filter_designated_vlan_get(
    SOC_SAND_IN  int                                   unit,
	SOC_SAND_IN   int                                  core_id,
    SOC_SAND_IN  SOC_PPC_PORT                                local_port_ndx,
    SOC_SAND_OUT SOC_SAND_PP_VLAN_ID                             *vid,
    SOC_SAND_OUT uint8                                   *accept
  )
{
  uint32
    res = SOC_SAND_OK;

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_LLP_FILTER_DESIGNATED_VLAN_GET);

  SOC_SAND_CHECK_DRIVER_AND_DEVICE;

  SOC_SAND_CHECK_NULL_INPUT(vid);
  SOC_SAND_CHECK_NULL_INPUT(accept);

  res = arad_pp_llp_filter_designated_vlan_get_verify(
          unit,
          local_port_ndx
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  SOC_SAND_TAKE_DEVICE_SEMAPHORE;

  res = arad_pp_llp_filter_designated_vlan_get_unsafe(
          unit,
		  core_id,
          local_port_ndx,
          vid,
          accept
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit_semaphore);

exit_semaphore:
  SOC_SAND_GIVE_DEVICE_SEMAPHORE;
  ARAD_PP_DO_NOTHING_AND_EXIT;
exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in soc_ppd_llp_filter_designated_vlan_get()", local_port_ndx, 0);
}

/* } */

#include <soc/dpp/SAND/Utils/sand_footer.h>



