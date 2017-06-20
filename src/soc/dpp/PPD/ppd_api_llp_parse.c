/* $Id: arad_pp_api_llp_parse.c,v 1.6 Broadcom SDK $
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
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_llp_parse.h>

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
*     Sets the global information for link layer parsing,
 *     including TPID values. Used in ingress to identify VLAN
 *     tags on incoming packets, and used in egress to
 *     construct VLAN tags on outgoing packets.
 *     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  soc_ppd_llp_parse_tpid_values_set(
    SOC_SAND_IN  int                                     unit,
    SOC_SAND_IN  SOC_PPC_LLP_PARSE_TPID_VALUES                   *tpid_vals
  )
{
  uint32
    res = SOC_SAND_OK;

  SOC_SAND_INIT_ERROR_DEFINITIONS(SOC_PPC_LLP_PARSE_TPID_VALUES_SET);

  SOC_SAND_CHECK_DRIVER_AND_DEVICE;

  SOC_SAND_CHECK_NULL_INPUT(tpid_vals);

  res = arad_pp_llp_parse_tpid_values_set_verify(
          unit,
          tpid_vals
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  SOC_SAND_TAKE_DEVICE_SEMAPHORE;

  res = arad_pp_llp_parse_tpid_values_set_unsafe(
          unit,
          tpid_vals
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit_semaphore);

exit_semaphore:
  SOC_SAND_GIVE_DEVICE_SEMAPHORE;
  ARAD_PP_DO_NOTHING_AND_EXIT;
exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in soc_ppd_llp_parse_tpid_values_set()", 0, 0);
}

/*********************************************************************
*     Sets the global information for link layer parsing,
 *     including TPID values. Used in ingress to identify VLAN
 *     tags on incoming packets, and used in egress to
 *     construct VLAN tags on outgoing packets.
 *     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  soc_ppd_llp_parse_tpid_values_get(
    SOC_SAND_IN  int                                     unit,
    SOC_SAND_OUT SOC_PPC_LLP_PARSE_TPID_VALUES                   *tpid_vals
  )
{
  uint32
    res = SOC_SAND_OK;

  SOC_SAND_INIT_ERROR_DEFINITIONS(SOC_PPC_LLP_PARSE_TPID_VALUES_GET);

  SOC_SAND_CHECK_DRIVER_AND_DEVICE;

  SOC_SAND_CHECK_NULL_INPUT(tpid_vals);

  res = arad_pp_llp_parse_tpid_values_get_verify(
          unit
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  SOC_SAND_TAKE_DEVICE_SEMAPHORE;

  res = arad_pp_llp_parse_tpid_values_get_unsafe(
          unit,
          tpid_vals
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit_semaphore);

exit_semaphore:
  SOC_SAND_GIVE_DEVICE_SEMAPHORE;
  ARAD_PP_DO_NOTHING_AND_EXIT;
exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in soc_ppd_llp_parse_tpid_values_get()", 0, 0);
}

/*********************************************************************
*     Sets the TPID profile selection of two TPIDs from the
 *     Global TPIDs.
 *     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  soc_ppd_llp_parse_tpid_profile_info_set(
    SOC_SAND_IN  int                                     unit,
    SOC_SAND_IN  uint32                                      tpid_profile_ndx,
    SOC_SAND_IN  SOC_PPC_LLP_PARSE_TPID_PROFILE_INFO             *tpid_profile_info
  )
{
  uint32
    res = SOC_SAND_OK;

  SOC_SAND_INIT_ERROR_DEFINITIONS(SOC_PPC_LLP_PARSE_TPID_PROFILE_INFO_SET);

  SOC_SAND_CHECK_DRIVER_AND_DEVICE;

  SOC_SAND_CHECK_NULL_INPUT(tpid_profile_info);

  res = arad_pp_llp_parse_tpid_profile_info_set_verify(
          unit,
          tpid_profile_ndx,
          tpid_profile_info
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  SOC_SAND_TAKE_DEVICE_SEMAPHORE;

  res = arad_pp_llp_parse_tpid_profile_info_set_unsafe(
          unit,
          tpid_profile_ndx,
          tpid_profile_info
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit_semaphore);

exit_semaphore:
  SOC_SAND_GIVE_DEVICE_SEMAPHORE;
  ARAD_PP_DO_NOTHING_AND_EXIT;
exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in soc_ppd_llp_parse_tpid_profile_info_set()", tpid_profile_ndx, 0);
}

/*********************************************************************
*     Sets the TPID profile selection of two TPIDs from the
 *     Global TPIDs.
 *     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  soc_ppd_llp_parse_tpid_profile_info_get(
    SOC_SAND_IN  int                                     unit,
    SOC_SAND_IN  uint32                                      tpid_profile_ndx,
    SOC_SAND_OUT SOC_PPC_LLP_PARSE_TPID_PROFILE_INFO             *tpid_profile_info
  )
{
  uint32
    res = SOC_SAND_OK;

  SOC_SAND_INIT_ERROR_DEFINITIONS(SOC_PPC_LLP_PARSE_TPID_PROFILE_INFO_GET);

  SOC_SAND_CHECK_DRIVER_AND_DEVICE;

  SOC_SAND_CHECK_NULL_INPUT(tpid_profile_info);

  res = arad_pp_llp_parse_tpid_profile_info_get_verify(
          unit,
          tpid_profile_ndx
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  SOC_SAND_TAKE_DEVICE_SEMAPHORE;

  res = arad_pp_llp_parse_tpid_profile_info_get_unsafe(
          unit,
          tpid_profile_ndx,
          tpid_profile_info
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit_semaphore);

exit_semaphore:
  SOC_SAND_GIVE_DEVICE_SEMAPHORE;
  ARAD_PP_DO_NOTHING_AND_EXIT;
exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in soc_ppd_llp_parse_tpid_profile_info_get()", tpid_profile_ndx, 0);
}

/*********************************************************************
*     Maps from Port profile to TPID Profile.
 *     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  soc_ppd_llp_parse_port_profile_to_tpid_profile_map_set(
    SOC_SAND_IN  int                                     unit,
    SOC_SAND_IN  uint32                                      port_profile_ndx,
    SOC_SAND_IN  uint32                                      tpid_profile_id
  )
{
  uint32
    res = SOC_SAND_OK;

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_LLP_PARSE_PORT_PROFILE_TO_TPID_PROFILE_MAP_SET);

  SOC_SAND_CHECK_DRIVER_AND_DEVICE;

  res = arad_pp_llp_parse_port_profile_to_tpid_profile_map_set_verify(
          unit,
          port_profile_ndx,
          tpid_profile_id
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  SOC_SAND_TAKE_DEVICE_SEMAPHORE;

  res = arad_pp_llp_parse_port_profile_to_tpid_profile_map_set_unsafe(
          unit,
          port_profile_ndx,
          tpid_profile_id
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit_semaphore);

exit_semaphore:
  SOC_SAND_GIVE_DEVICE_SEMAPHORE;
  ARAD_PP_DO_NOTHING_AND_EXIT;
exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in soc_ppd_llp_parse_port_profile_to_tpid_profile_map_set()", port_profile_ndx, 0);
}

/*********************************************************************
*     Maps from Port profile to TPID Profile.
 *     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  soc_ppd_llp_parse_port_profile_to_tpid_profile_map_get(
    SOC_SAND_IN  int                                     unit,
    SOC_SAND_IN  uint32                                      port_profile_ndx,
    SOC_SAND_OUT uint32                                      *tpid_profile_id
  )
{
  uint32
    res = SOC_SAND_OK;

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_LLP_PARSE_PORT_PROFILE_TO_TPID_PROFILE_MAP_GET);

  SOC_SAND_CHECK_DRIVER_AND_DEVICE;

  SOC_SAND_CHECK_NULL_INPUT(tpid_profile_id);

  res = arad_pp_llp_parse_port_profile_to_tpid_profile_map_get_verify(
          unit,
          port_profile_ndx
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  SOC_SAND_TAKE_DEVICE_SEMAPHORE;

  res = arad_pp_llp_parse_port_profile_to_tpid_profile_map_get_unsafe(
          unit,
          port_profile_ndx,
          tpid_profile_id
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit_semaphore);

exit_semaphore:
  SOC_SAND_GIVE_DEVICE_SEMAPHORE;
  ARAD_PP_DO_NOTHING_AND_EXIT;
exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in soc_ppd_llp_parse_port_profile_to_tpid_profile_map_get()", port_profile_ndx, 0);
}

/*********************************************************************
*     Given the port profile and the parsing information
 *     determine: - Whether this packet format accepted or
 *     denied. - The tag structure of the packet, i.e. what
 *     vlan tags exist on the packet (S-tag, S-C-tag, etc...).
 *     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  soc_ppd_llp_parse_packet_format_info_set(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  uint32                                  port_profile_ndx,
    SOC_SAND_IN  SOC_PPC_LLP_PARSE_INFO                      *parse_key,
    SOC_SAND_IN  SOC_PPC_LLP_PARSE_PACKET_FORMAT_INFO        *format_info
  )
{
  uint32
    res = SOC_SAND_OK;

  SOC_SAND_INIT_ERROR_DEFINITIONS(SOC_PPC_LLP_PARSE_PACKET_FORMAT_INFO_SET);

  SOC_SAND_CHECK_DRIVER_AND_DEVICE;

  SOC_SAND_CHECK_NULL_INPUT(parse_key);
  SOC_SAND_CHECK_NULL_INPUT(format_info);

  res = arad_pp_llp_parse_packet_format_info_set_verify(
          unit,
          port_profile_ndx,
          parse_key,
          format_info
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  SOC_SAND_TAKE_DEVICE_SEMAPHORE;

  res = arad_pp_llp_parse_packet_format_info_set_unsafe(
          unit,
          port_profile_ndx,
          parse_key,
          format_info
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit_semaphore);

exit_semaphore:
  SOC_SAND_GIVE_DEVICE_SEMAPHORE;
  ARAD_PP_DO_NOTHING_AND_EXIT;
exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in soc_ppd_llp_parse_packet_format_info_set()", port_profile_ndx, 0);
}

/*********************************************************************
*     Given the port profile and the parsing information
 *     determine: - Whether this packet format accepted or
 *     denied. - The tag structure of the packet, i.e. what
 *     vlan tags exist on the packet (S-tag, S-C-tag, etc...).
 *     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  soc_ppd_llp_parse_packet_format_info_get(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  uint32                                  port_profile_ndx,
    SOC_SAND_IN  SOC_PPC_LLP_PARSE_INFO                      *parse_key,
    SOC_SAND_OUT SOC_PPC_LLP_PARSE_PACKET_FORMAT_INFO        *format_info
  )
{
  uint32
    res = SOC_SAND_OK;

  SOC_SAND_INIT_ERROR_DEFINITIONS(SOC_PPC_LLP_PARSE_PACKET_FORMAT_INFO_GET);

  SOC_SAND_CHECK_DRIVER_AND_DEVICE;

  SOC_SAND_CHECK_NULL_INPUT(parse_key);
  SOC_SAND_CHECK_NULL_INPUT(format_info);

  res = arad_pp_llp_parse_packet_format_info_get_verify(
          unit,
          port_profile_ndx,
          parse_key
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  SOC_SAND_TAKE_DEVICE_SEMAPHORE;

  res = arad_pp_llp_parse_packet_format_info_get_unsafe(
          unit,
          port_profile_ndx,
          parse_key,
          format_info
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit_semaphore);

exit_semaphore:
  SOC_SAND_GIVE_DEVICE_SEMAPHORE;
  ARAD_PP_DO_NOTHING_AND_EXIT;
exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in soc_ppd_llp_parse_packet_format_info_get()", port_profile_ndx, 0);
}

/* } */

#include <soc/dpp/SAND/Utils/sand_footer.h>



