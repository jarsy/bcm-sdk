
#include <soc/mcm/memregs.h>
#if defined(BCM_88650_A0)
/* $Id: arad_pp_api_mgmt.c,v 1.7 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/

#ifdef _ERR_MSG_MODULE_NAME
  #error "_ERR_MSG_MODULE_NAME redefined"
#endif

#define _ERR_MSG_MODULE_NAME BSL_SOC_MANAGEMENT

/*************
 * INCLUDES  *
 *************/
/* { */
#include <shared/bsl.h>
#include <soc/dcmn/error.h>
#include <soc/dpp/SAND/Utils/sand_header.h>

#include <soc/dpp/ARAD/ARAD_PP/arad_pp_mgmt.h>
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_general.h>
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_sw_db.h>

#include <soc/dpp/SAND/Management/sand_device_management.h>

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

uint32
  arad_pp_mgmt_device_close(
    SOC_SAND_IN  int  unit
  )
{
  uint32
    res = SOC_SAND_OK;

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_MGMT_DEVICE_CLOSE);

  SOC_SAND_CHECK_DRIVER_AND_DEVICE;

  SOC_SAND_TAKE_DEVICE_SEMAPHORE;

  res = arad_pp_mgmt_device_close_unsafe(
          unit
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit_semaphore);

exit_semaphore:
  SOC_SAND_GIVE_DEVICE_SEMAPHORE;
  ARAD_PP_DO_NOTHING_AND_EXIT;
exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_mgmt_device_close()", 0, 0);
}

/*********************************************************************
*     Set arad_pp device operation mode. This defines
*     configurations such as support for certain header types
*     etc.
*     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  arad_pp_mgmt_operation_mode_set(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  ARAD_PP_MGMT_OPERATION_MODE *op_mode
  )
{
  uint32
    res = SOC_SAND_OK;

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_MGMT_OPERATION_MODE_SET);

  SOC_SAND_CHECK_DRIVER_AND_DEVICE;

  SOC_SAND_CHECK_NULL_INPUT(op_mode);

  SOC_SAND_TAKE_DEVICE_SEMAPHORE;

  res = arad_pp_mgmt_operation_mode_set_unsafe(
          unit,
          op_mode
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit_semaphore);

exit_semaphore:
  SOC_SAND_GIVE_DEVICE_SEMAPHORE;
  ARAD_PP_DO_NOTHING_AND_EXIT;
exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_mgmt_operation_mode_set()", 0, 0);
}

/*********************************************************************
*     Set arad_pp device operation mode. This defines
*     configurations such as support for certain header types
*     etc.
*     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  arad_pp_mgmt_operation_mode_get(
    SOC_SAND_IN  int                        unit,
    SOC_SAND_OUT ARAD_PP_MGMT_OPERATION_MODE     *op_mode
  )
{
  uint32
    res = SOC_SAND_OK;

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_MGMT_OPERATION_MODE_GET);

  SOC_SAND_CHECK_DRIVER_AND_DEVICE;

  SOC_SAND_CHECK_NULL_INPUT(op_mode);

  SOC_SAND_TAKE_DEVICE_SEMAPHORE;

  res = arad_pp_mgmt_operation_mode_get_unsafe(
          unit,
          op_mode
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit_semaphore);

exit_semaphore:
  SOC_SAND_GIVE_DEVICE_SEMAPHORE;
  ARAD_PP_DO_NOTHING_AND_EXIT;
exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_mgmt_operation_mode_get()", 0, 0);
}

/*********************************************************************
*     Set the ELK interface mode.
 *     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  arad_pp_mgmt_elk_mode_set(
    SOC_SAND_IN  int           unit,
    SOC_SAND_IN  ARAD_PP_MGMT_ELK_MODE elk_mode,
    SOC_SAND_OUT uint32           *ingress_pkt_rate
  )
{
  uint32
    res = SOC_SAND_OK;

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_MGMT_ELK_MODE_SET);

  SOC_SAND_CHECK_DRIVER_AND_DEVICE;

  SOC_SAND_CHECK_NULL_INPUT(ingress_pkt_rate);

  res = arad_pp_mgmt_elk_mode_set_verify(
          unit,
          elk_mode
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  SOC_SAND_TAKE_DEVICE_SEMAPHORE;

  res = arad_pp_mgmt_elk_mode_set_unsafe(
          unit,
          elk_mode,
          ingress_pkt_rate
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit_semaphore);

exit_semaphore:
  SOC_SAND_GIVE_DEVICE_SEMAPHORE;
  ARAD_PP_DO_NOTHING_AND_EXIT;
exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_mgmt_elk_mode_set()", 0, 0);
}

/*********************************************************************
*     Set the ELK interface mode.
 *     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  arad_pp_mgmt_elk_mode_get(
    SOC_SAND_IN  int           unit,
    SOC_SAND_OUT ARAD_PP_MGMT_ELK_MODE *elk_mode,
    SOC_SAND_OUT uint32           *ingress_pkt_rate
  )
{
  uint32
    res = SOC_SAND_OK;

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_MGMT_ELK_MODE_GET);

  SOC_SAND_CHECK_DRIVER_AND_DEVICE;

  SOC_SAND_CHECK_NULL_INPUT(elk_mode);
  SOC_SAND_CHECK_NULL_INPUT(ingress_pkt_rate);

  res = arad_pp_mgmt_elk_mode_get_verify(
          unit
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  SOC_SAND_TAKE_DEVICE_SEMAPHORE;

  res = arad_pp_mgmt_elk_mode_get_unsafe(
          unit,
          elk_mode,
          ingress_pkt_rate
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit_semaphore);

exit_semaphore:
  SOC_SAND_GIVE_DEVICE_SEMAPHORE;
  ARAD_PP_DO_NOTHING_AND_EXIT;
exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_mgmt_elk_mode_get()", 0, 0);
}

/*********************************************************************
*     Determine whether the specified lookup is externalized
 *     or not.
 *     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  arad_pp_mgmt_use_elk_set(
    SOC_SAND_IN  int           unit,
    SOC_SAND_IN  ARAD_PP_MGMT_LKP_TYPE lkp_type,
    SOC_SAND_IN  uint8           use_elk
  )
{
  uint32
    res = SOC_SAND_OK;

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_MGMT_USE_ELK_SET);

  SOC_SAND_CHECK_DRIVER_AND_DEVICE;

  res = arad_pp_mgmt_use_elk_set_verify(
          unit,
          lkp_type,
          use_elk
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  SOC_SAND_TAKE_DEVICE_SEMAPHORE;

  res = arad_pp_mgmt_use_elk_set_unsafe(
          unit,
          lkp_type,
          use_elk
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit_semaphore);

exit_semaphore:
  SOC_SAND_GIVE_DEVICE_SEMAPHORE;
  ARAD_PP_DO_NOTHING_AND_EXIT;
exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_mgmt_use_elk_set()", 0, 0);
}

/*********************************************************************
*     Determine whether the specified lookup is externalized
 *     or not.
 *     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  arad_pp_mgmt_use_elk_get(
    SOC_SAND_IN  int           unit,
    SOC_SAND_OUT ARAD_PP_MGMT_LKP_TYPE *lkp_type,
    SOC_SAND_OUT uint8           *use_elk
  )
{
  uint32
    res = SOC_SAND_OK;

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_MGMT_USE_ELK_GET);

  SOC_SAND_CHECK_DRIVER_AND_DEVICE;

  SOC_SAND_CHECK_NULL_INPUT(lkp_type);
  SOC_SAND_CHECK_NULL_INPUT(use_elk);

  res = arad_pp_mgmt_use_elk_get_verify(
          unit
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  SOC_SAND_TAKE_DEVICE_SEMAPHORE;

  res = arad_pp_mgmt_use_elk_get_unsafe(
          unit,
          lkp_type,
          use_elk
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit_semaphore);

exit_semaphore:
  SOC_SAND_GIVE_DEVICE_SEMAPHORE;
  ARAD_PP_DO_NOTHING_AND_EXIT;
exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_mgmt_use_elk_get()", 0, 0);
}


/* } */

#include <soc/dpp/SAND/Utils/sand_footer.h>


#endif /* of #if defined(BCM_88650_A0) */

