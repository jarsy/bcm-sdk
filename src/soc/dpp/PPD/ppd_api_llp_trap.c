/* $Id: arad_pp_api_llp_trap.c,v 1.6 Broadcom SDK $
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
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_llp_trap.h>

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
*     Sets port information for Link Layer Traps, including
 *     which reserved Multicast profile and which Traps are
 *     enabled...
 *     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  soc_ppd_llp_trap_port_info_set(
    SOC_SAND_IN  int                                     unit,
    SOC_SAND_IN  int                                     core_id,
    SOC_SAND_IN  SOC_PPC_PORT                                local_port_ndx,
    SOC_SAND_IN  SOC_PPC_LLP_TRAP_PORT_INFO                      *port_info
  )
{
  uint32
    res = SOC_SAND_OK;

  SOC_SAND_INIT_ERROR_DEFINITIONS(SOC_PPC_LLP_TRAP_PORT_INFO_SET);

  SOC_SAND_CHECK_DRIVER_AND_DEVICE;

  SOC_SAND_CHECK_NULL_INPUT(port_info);

  res = arad_pp_llp_trap_port_info_set_verify(
          unit,
          local_port_ndx,
          port_info
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  SOC_SAND_TAKE_DEVICE_SEMAPHORE;

  res = arad_pp_llp_trap_port_info_set_unsafe(
          unit,
          core_id,
          local_port_ndx,
          port_info
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit_semaphore);

exit_semaphore:
  SOC_SAND_GIVE_DEVICE_SEMAPHORE;
  ARAD_PP_DO_NOTHING_AND_EXIT;
exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in soc_ppd_llp_trap_port_info_set()", local_port_ndx, 0);
}

/*********************************************************************
*     Sets port information for Link Layer Traps, including
 *     which reserved Multicast profile and which Traps are
 *     enabled...
 *     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  soc_ppd_llp_trap_port_info_get(
    SOC_SAND_IN  int                                     unit,
    SOC_SAND_IN  int                                     core_id,
    SOC_SAND_IN  SOC_PPC_PORT                                local_port_ndx,
    SOC_SAND_OUT SOC_PPC_LLP_TRAP_PORT_INFO                      *port_info
  )
{
  uint32
    res = SOC_SAND_OK;

  SOC_SAND_INIT_ERROR_DEFINITIONS(SOC_PPC_LLP_TRAP_PORT_INFO_GET);

  SOC_SAND_CHECK_DRIVER_AND_DEVICE;

  SOC_SAND_CHECK_NULL_INPUT(port_info);

  res = arad_pp_llp_trap_port_info_get_verify(
          unit,
          local_port_ndx
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  SOC_SAND_TAKE_DEVICE_SEMAPHORE;

  res = arad_pp_llp_trap_port_info_get_unsafe(
          unit,
          core_id,
          local_port_ndx,
          port_info
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit_semaphore);

exit_semaphore:
  SOC_SAND_GIVE_DEVICE_SEMAPHORE;
  ARAD_PP_DO_NOTHING_AND_EXIT;
exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in soc_ppd_llp_trap_port_info_get()", local_port_ndx, 0);
}

/*********************************************************************
*     Sets information for ARP trapping, including My-IP
 *     addresses (used to Trap ARP Requests)
 *     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  soc_ppd_llp_trap_arp_info_set(
    SOC_SAND_IN  int                                     unit,
    SOC_SAND_IN  SOC_PPC_LLP_TRAP_ARP_INFO                       *arp_info
  )
{
  uint32
    res = SOC_SAND_OK;

  SOC_SAND_INIT_ERROR_DEFINITIONS(SOC_PPC_LLP_TRAP_ARP_INFO_SET);

  SOC_SAND_CHECK_DRIVER_AND_DEVICE;

  SOC_SAND_CHECK_NULL_INPUT(arp_info);

  res = arad_pp_llp_trap_arp_info_set_verify(
          unit,
          arp_info
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  SOC_SAND_TAKE_DEVICE_SEMAPHORE;

  res = arad_pp_llp_trap_arp_info_set_unsafe(
          unit,
          arp_info
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit_semaphore);

exit_semaphore:
  SOC_SAND_GIVE_DEVICE_SEMAPHORE;
  ARAD_PP_DO_NOTHING_AND_EXIT;
exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in soc_ppd_llp_trap_arp_info_set()", 0, 0);
}

/*********************************************************************
*     Sets information for ARP trapping, including My-IP
 *     addresses (used to Trap ARP Requests)
 *     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  soc_ppd_llp_trap_arp_info_get(
    SOC_SAND_IN  int                                     unit,
    SOC_SAND_OUT SOC_PPC_LLP_TRAP_ARP_INFO                       *arp_info
  )
{
  uint32
    res = SOC_SAND_OK;

  SOC_SAND_INIT_ERROR_DEFINITIONS(SOC_PPC_LLP_TRAP_ARP_INFO_GET);

  SOC_SAND_CHECK_DRIVER_AND_DEVICE;

  SOC_SAND_CHECK_NULL_INPUT(arp_info);

  res = arad_pp_llp_trap_arp_info_get_verify(
          unit
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  SOC_SAND_TAKE_DEVICE_SEMAPHORE;

  res = arad_pp_llp_trap_arp_info_get_unsafe(
          unit,
          arp_info
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit_semaphore);

exit_semaphore:
  SOC_SAND_GIVE_DEVICE_SEMAPHORE;
  ARAD_PP_DO_NOTHING_AND_EXIT;
exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in soc_ppd_llp_trap_arp_info_get()", 0, 0);
}

/*********************************************************************
*     Sets Trap information for IEEE reserved multicast
 *     (Ethernet Header. DA matches 01-80-c2-00-00-XX where XX =
 *     8'b00xx_xxxx.
 *     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  soc_ppd_llp_trap_reserved_mc_info_set(
    SOC_SAND_IN  int                                     unit,
    SOC_SAND_IN  SOC_PPC_LLP_TRAP_RESERVED_MC_KEY                *reserved_mc_key,
    SOC_SAND_IN  SOC_PPC_ACTION_PROFILE                          *trap_action
  )
{
  uint32
    res = SOC_SAND_OK;

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_LLP_TRAP_RESERVED_MC_INFO_SET);

  SOC_SAND_CHECK_DRIVER_AND_DEVICE;

  SOC_SAND_CHECK_NULL_INPUT(reserved_mc_key);
  SOC_SAND_CHECK_NULL_INPUT(trap_action);

  res = arad_pp_llp_trap_reserved_mc_info_set_verify(
          unit,
          reserved_mc_key,
          trap_action
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  SOC_SAND_TAKE_DEVICE_SEMAPHORE;

  res = arad_pp_llp_trap_reserved_mc_info_set_unsafe(
          unit,
          reserved_mc_key,
          trap_action
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit_semaphore);

exit_semaphore:
  SOC_SAND_GIVE_DEVICE_SEMAPHORE;
  ARAD_PP_DO_NOTHING_AND_EXIT;
exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in soc_ppd_llp_trap_reserved_mc_info_set()", 0, 0);
}

/*********************************************************************
*     Sets Trap information for IEEE reserved multicast
 *     (Ethernet Header. DA matches 01-80-c2-00-00-XX where XX =
 *     8'b00xx_xxxx.
 *     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  soc_ppd_llp_trap_reserved_mc_info_get(
    SOC_SAND_IN  int                                     unit,
    SOC_SAND_IN  SOC_PPC_LLP_TRAP_RESERVED_MC_KEY                *reserved_mc_key,
    SOC_SAND_OUT SOC_PPC_ACTION_PROFILE                          *trap_action
  )
{
  uint32
    res = SOC_SAND_OK;

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_LLP_TRAP_RESERVED_MC_INFO_GET);

  SOC_SAND_CHECK_DRIVER_AND_DEVICE;

  SOC_SAND_CHECK_NULL_INPUT(reserved_mc_key);
  SOC_SAND_CHECK_NULL_INPUT(trap_action);

  res = arad_pp_llp_trap_reserved_mc_info_get_verify(
          unit,
          reserved_mc_key
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  SOC_SAND_TAKE_DEVICE_SEMAPHORE;

  res = arad_pp_llp_trap_reserved_mc_info_get_unsafe(
          unit,
          reserved_mc_key,
          trap_action
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit_semaphore);

exit_semaphore:
  SOC_SAND_GIVE_DEVICE_SEMAPHORE;
  ARAD_PP_DO_NOTHING_AND_EXIT;
exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in soc_ppd_llp_trap_reserved_mc_info_get()", 0, 0);
}

/*********************************************************************
*     Sets a programmable trap, a trap that may be set to
 *     packets according to L2/L3/L4 attributes.
 *     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  soc_ppd_llp_trap_prog_trap_info_set(
    SOC_SAND_IN  int                                     unit,
    SOC_SAND_IN  uint32                                      prog_trap_ndx,
    SOC_SAND_IN  SOC_PPC_LLP_TRAP_PROG_TRAP_QUALIFIER            *prog_trap_qual
  )
{
  uint32
    res = SOC_SAND_OK;

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_LLP_TRAP_PROG_TRAP_INFO_SET);

  SOC_SAND_CHECK_DRIVER_AND_DEVICE;

  SOC_SAND_CHECK_NULL_INPUT(prog_trap_qual);

  res = arad_pp_llp_trap_prog_trap_info_set_verify(
          unit,
          prog_trap_ndx,
          prog_trap_qual
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  SOC_SAND_TAKE_DEVICE_SEMAPHORE;

  res = arad_pp_llp_trap_prog_trap_info_set_unsafe(
          unit,
          prog_trap_ndx,
          prog_trap_qual
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit_semaphore);

exit_semaphore:
  SOC_SAND_GIVE_DEVICE_SEMAPHORE;
  ARAD_PP_DO_NOTHING_AND_EXIT;
exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in soc_ppd_llp_trap_prog_trap_info_set()", prog_trap_ndx, 0);
}

/*********************************************************************
*     Sets a programmable trap, a trap that may be set to
 *     packets according to L2/L3/L4 attributes.
 *     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  soc_ppd_llp_trap_prog_trap_info_get(
    SOC_SAND_IN  int                                     unit,
    SOC_SAND_IN  uint32                                      prog_trap_ndx,
    SOC_SAND_OUT SOC_PPC_LLP_TRAP_PROG_TRAP_QUALIFIER            *prog_trap_qual
  )
{
  uint32
    res = SOC_SAND_OK;

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_LLP_TRAP_PROG_TRAP_INFO_GET);

  SOC_SAND_CHECK_DRIVER_AND_DEVICE;

  SOC_SAND_CHECK_NULL_INPUT(prog_trap_qual);

  res = arad_pp_llp_trap_prog_trap_info_get_verify(
          unit,
          prog_trap_ndx
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  SOC_SAND_TAKE_DEVICE_SEMAPHORE;

  res = arad_pp_llp_trap_prog_trap_info_get_unsafe(
          unit,
          prog_trap_ndx,
          prog_trap_qual
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit_semaphore);

exit_semaphore:
  SOC_SAND_GIVE_DEVICE_SEMAPHORE;
  ARAD_PP_DO_NOTHING_AND_EXIT;
exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in soc_ppd_llp_trap_prog_trap_info_get()", prog_trap_ndx, 0);
}
/* } */

#include <soc/dpp/SAND/Utils/sand_footer.h>



