
/* $Id: arad_pp_api_eg_ac.c,v 1.8 Broadcom SDK $
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
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_eg_ac.h>

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
*     Sets the editing information for packets
 *     associated with AC.
 *     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  soc_ppd_eg_ac_info_set(
    SOC_SAND_IN  int                                     unit,
    SOC_SAND_IN  SOC_PPC_AC_ID                               out_ac_ndx,
    SOC_SAND_IN  SOC_PPC_EG_AC_INFO                              *ac_info
  )
{
  uint32
    res = SOC_SAND_OK;

  SOC_SAND_INIT_ERROR_DEFINITIONS(SOC_PPC_EG_AC_INFO_SET);

  SOC_SAND_CHECK_DRIVER_AND_DEVICE;

  SOC_SAND_CHECK_NULL_INPUT(ac_info);

  res = arad_pp_eg_ac_info_set_verify(
          unit,
          out_ac_ndx,
          ac_info
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  SOC_SAND_TAKE_DEVICE_SEMAPHORE;

  res = arad_pp_eg_ac_info_set_unsafe(
          unit,
          out_ac_ndx,
          ac_info
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit_semaphore);

exit_semaphore:
  SOC_SAND_GIVE_DEVICE_SEMAPHORE;
  ARAD_PP_DO_NOTHING_AND_EXIT;
exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in soc_ppd_eg_ac_info_set()", out_ac_ndx, 0);
}

/*********************************************************************
*     Sets the editing information for packets
 *     associated with AC.
 *     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  soc_ppd_eg_ac_info_get(
    SOC_SAND_IN  int                                     unit,
    SOC_SAND_IN  SOC_PPC_AC_ID                               out_ac_ndx,
    SOC_SAND_OUT SOC_PPC_EG_AC_INFO                              *ac_info
  )
{
  uint32
    res = SOC_SAND_OK;

  SOC_SAND_INIT_ERROR_DEFINITIONS(SOC_PPC_EG_AC_INFO_GET);

  SOC_SAND_CHECK_DRIVER_AND_DEVICE;

  SOC_SAND_CHECK_NULL_INPUT(ac_info);

  res = arad_pp_eg_ac_info_get_verify(
          unit,
          out_ac_ndx
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  SOC_SAND_TAKE_DEVICE_SEMAPHORE;

  res = arad_pp_eg_ac_info_get_unsafe(
          unit,
          out_ac_ndx,
          ac_info
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit_semaphore);

exit_semaphore:
  SOC_SAND_GIVE_DEVICE_SEMAPHORE;
  ARAD_PP_DO_NOTHING_AND_EXIT;
exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in soc_ppd_eg_ac_info_get()", out_ac_ndx, 0);
}

/*********************************************************************
*     This function is used to define a Maintenance Point (MP)
 *     on an outgoing Attachment-Circuit (port x VSI) and
 *     MD-level, and to determine the action to perform. If the
 *     MP is one of the 4K accelerated MEPs, the function
 *     configures the related OAMP databases and associates the
 *     AC and MD-Level with a user-provided handle. This handle
 *     is later used by user to access OAMP database for this
 *     MEP.
 *     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  soc_ppd_eg_ac_mp_info_set(
    SOC_SAND_IN  int                          unit,
    SOC_SAND_IN  SOC_PPC_AC_ID                        out_ac_ndx,
    SOC_SAND_IN  SOC_PPC_EG_AC_MP_INFO                *info
  )
{
  uint32
    res = SOC_SAND_OK;

  SOC_SAND_INIT_ERROR_DEFINITIONS(SOC_PPC_EG_AC_MP_INFO_SET);

  SOC_SAND_CHECK_DRIVER_AND_DEVICE;

  SOC_SAND_CHECK_NULL_INPUT(info);

  res = arad_pp_eg_ac_mp_info_set_verify(
          unit,
          out_ac_ndx,
          info
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  SOC_SAND_TAKE_DEVICE_SEMAPHORE;

  res = arad_pp_eg_ac_mp_info_set_unsafe(
          unit,
          out_ac_ndx,
          info
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit_semaphore);

exit_semaphore:
  SOC_SAND_GIVE_DEVICE_SEMAPHORE;
  ARAD_PP_DO_NOTHING_AND_EXIT;
exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in soc_ppd_eg_ac_mp_info_set()", out_ac_ndx, 0);
}

/*********************************************************************
*     This function is used to define a Maintenance Point (MP)
 *     on an outgoing Attachment-Circuit (port x VSI) and
 *     MD-level, and to determine the action to perform. If the
 *     MP is one of the 4K accelerated MEPs, the function
 *     configures the related OAMP databases and associates the
 *     AC and MD-Level with a user-provided handle. This handle
 *     is later used by user to access OAMP database for this
 *     MEP.
 *     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  soc_ppd_eg_ac_mp_info_get(
    SOC_SAND_IN  int                          unit,
    SOC_SAND_IN  SOC_PPC_AC_ID                        out_ac_ndx,
    SOC_SAND_OUT SOC_PPC_EG_AC_MP_INFO                *info
  )
{
  uint32
    res = SOC_SAND_OK;

  SOC_SAND_INIT_ERROR_DEFINITIONS(SOC_PPC_EG_AC_MP_INFO_GET);

  SOC_SAND_CHECK_DRIVER_AND_DEVICE;

  SOC_SAND_CHECK_NULL_INPUT(info);

  res = arad_pp_eg_ac_mp_info_get_verify(
          unit,
          out_ac_ndx
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  SOC_SAND_TAKE_DEVICE_SEMAPHORE;

  res = arad_pp_eg_ac_mp_info_get_unsafe(
          unit,
          out_ac_ndx,
          info
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit_semaphore);

exit_semaphore:
  SOC_SAND_GIVE_DEVICE_SEMAPHORE;
  ARAD_PP_DO_NOTHING_AND_EXIT;
exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in soc_ppd_eg_ac_mp_info_get()", out_ac_ndx, 0);
}

/*********************************************************************
*     Sets the editing information for packets NOT
 *     associated with AC and to be transmitted from VBP port
 *     (not CEP port).
 *     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  soc_ppd_eg_ac_port_vsi_info_add(
    SOC_SAND_IN  int                                     unit,
    SOC_SAND_IN  SOC_PPC_AC_ID                               out_ac_ndx,
    SOC_SAND_IN  SOC_PPC_EG_AC_VBP_KEY                           *vbp_key,
    SOC_SAND_IN  SOC_PPC_EG_AC_INFO                              *ac_info,
    SOC_SAND_OUT SOC_SAND_SUCCESS_FAILURE                          *success
  )
{
  uint32
    res = SOC_SAND_OK;

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_EG_AC_PORT_VSI_INFO_ADD);

  SOC_SAND_CHECK_DRIVER_AND_DEVICE;

  SOC_SAND_CHECK_NULL_INPUT(vbp_key);
  SOC_SAND_CHECK_NULL_INPUT(ac_info);
  SOC_SAND_CHECK_NULL_INPUT(success);

  res = arad_pp_eg_ac_port_vsi_info_add_verify(
          unit,
          out_ac_ndx,
          vbp_key,
          ac_info
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  SOC_SAND_TAKE_DEVICE_SEMAPHORE;

  res = arad_pp_eg_ac_port_vsi_info_add_unsafe(
          unit,
          out_ac_ndx,
          vbp_key,
          ac_info,
          success
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit_semaphore);

exit_semaphore:
  SOC_SAND_GIVE_DEVICE_SEMAPHORE;
  ARAD_PP_DO_NOTHING_AND_EXIT;
exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in soc_ppd_eg_ac_port_vsi_info_add()", out_ac_ndx, 0);
}

/*********************************************************************
*     Removes editing information of packets NOT
 *     associated with AC and to be transmitted from VBP port
 *     (not CEP port).
 *     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  soc_ppd_eg_ac_port_vsi_info_remove(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_EG_AC_VBP_KEY                       *vbp_key,
    SOC_SAND_OUT SOC_PPC_AC_ID                               *out_ac
  )
{
  uint32
    res = SOC_SAND_OK;

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_EG_AC_PORT_VSI_INFO_REMOVE);

  SOC_SAND_CHECK_DRIVER_AND_DEVICE;

  SOC_SAND_CHECK_NULL_INPUT(vbp_key);
  SOC_SAND_CHECK_NULL_INPUT(out_ac);

  res = arad_pp_eg_ac_port_vsi_info_remove_verify(
          unit,
          vbp_key
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  SOC_SAND_TAKE_DEVICE_SEMAPHORE;

  res = arad_pp_eg_ac_port_vsi_info_remove_unsafe(
          unit,
          vbp_key,
          out_ac
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit_semaphore);

exit_semaphore:
  SOC_SAND_GIVE_DEVICE_SEMAPHORE;
  ARAD_PP_DO_NOTHING_AND_EXIT;
exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in soc_ppd_eg_ac_port_vsi_info_remove()", 0, 0);
}

/*********************************************************************
*     Gets the editing information for packets NOT
 *     associated with AC and to be transmitted from VBP port
 *     (not CEP port).
 *     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  soc_ppd_eg_ac_port_vsi_info_get(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_EG_AC_VBP_KEY                       *vbp_key,
    SOC_SAND_OUT SOC_PPC_AC_ID                               *out_ac,
    SOC_SAND_OUT SOC_PPC_EG_AC_INFO                          *ac_info,
    SOC_SAND_OUT uint8                                 *found
  )
{
  uint32
    res = SOC_SAND_OK;

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_EG_AC_PORT_VSI_INFO_GET);

  SOC_SAND_CHECK_DRIVER_AND_DEVICE;

  SOC_SAND_CHECK_NULL_INPUT(vbp_key);
  SOC_SAND_CHECK_NULL_INPUT(out_ac);
  SOC_SAND_CHECK_NULL_INPUT(ac_info);
  SOC_SAND_CHECK_NULL_INPUT(found);

  res = arad_pp_eg_ac_port_vsi_info_get_verify(
          unit,
          vbp_key
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  SOC_SAND_TAKE_DEVICE_SEMAPHORE;

  res = arad_pp_eg_ac_port_vsi_info_get_unsafe(
          unit,
          vbp_key,
          out_ac,
          ac_info,
          found
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit_semaphore);

exit_semaphore:
  SOC_SAND_GIVE_DEVICE_SEMAPHORE;
  ARAD_PP_DO_NOTHING_AND_EXIT;
exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in soc_ppd_eg_ac_port_vsi_info_get()", 0, 0);
}

/*********************************************************************
*     Sets the editing information for packets NOT
 *     associated with AC and to be transmitted from CEP port
 *     (not VBP port).
 *     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  soc_ppd_eg_ac_port_cvid_info_add(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_AC_ID                               out_ac_ndx,
    SOC_SAND_IN  SOC_PPC_EG_AC_CEP_PORT_KEY                  *cep_key,
    SOC_SAND_IN  SOC_PPC_EG_AC_INFO                          *ac_info,
    SOC_SAND_OUT SOC_SAND_SUCCESS_FAILURE                      *success
  )
{
  uint32
    res = SOC_SAND_OK;

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_EG_AC_PORT_CVID_INFO_ADD);

  SOC_SAND_CHECK_DRIVER_AND_DEVICE;

  SOC_SAND_CHECK_NULL_INPUT(cep_key);
  SOC_SAND_CHECK_NULL_INPUT(ac_info);
  SOC_SAND_CHECK_NULL_INPUT(success);

  res = arad_pp_eg_ac_port_cvid_info_add_verify(
          unit,
          out_ac_ndx,
          cep_key,
          ac_info
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  SOC_SAND_TAKE_DEVICE_SEMAPHORE;

  res = arad_pp_eg_ac_port_cvid_info_add_unsafe(
          unit,
          out_ac_ndx,
          cep_key,
          ac_info,
          success
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit_semaphore);

exit_semaphore:
  SOC_SAND_GIVE_DEVICE_SEMAPHORE;
  ARAD_PP_DO_NOTHING_AND_EXIT;
exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in soc_ppd_eg_ac_port_cvid_info_add()", out_ac_ndx, 0);
}

/*********************************************************************
*     Removes editing information of packets NOT
 *     associated with AC and to be transmitted from CEP port
 *     (not VBP port).
 *     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  soc_ppd_eg_ac_port_cvid_info_remove(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_EG_AC_CEP_PORT_KEY                  *cep_key,
    SOC_SAND_OUT SOC_PPC_AC_ID                               *out_ac
  )
{
  uint32
    res = SOC_SAND_OK;

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_EG_AC_PORT_CVID_INFO_REMOVE);

  SOC_SAND_CHECK_DRIVER_AND_DEVICE;

  SOC_SAND_CHECK_NULL_INPUT(cep_key);
  SOC_SAND_CHECK_NULL_INPUT(out_ac);

  res = arad_pp_eg_ac_port_cvid_info_remove_verify(
          unit,
          cep_key
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  SOC_SAND_TAKE_DEVICE_SEMAPHORE;

  res = arad_pp_eg_ac_port_cvid_info_remove_unsafe(
          unit,
          cep_key,
          out_ac
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit_semaphore);

exit_semaphore:
  SOC_SAND_GIVE_DEVICE_SEMAPHORE;
  ARAD_PP_DO_NOTHING_AND_EXIT;
exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in soc_ppd_eg_ac_port_cvid_info_remove()", 0, 0);
}

/*********************************************************************
*     Gets the editing information for packets NOT
 *     associated with AC and to be transmitted from CEP port
 *     (not VBP port).
 *     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  soc_ppd_eg_ac_port_cvid_info_get(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_EG_AC_CEP_PORT_KEY                  *cep_key,
    SOC_SAND_OUT SOC_PPC_AC_ID                               *out_ac,
    SOC_SAND_OUT SOC_PPC_EG_AC_INFO                          *ac_info,
    SOC_SAND_OUT uint8                                 *found
  )
{
  uint32
    res = SOC_SAND_OK;

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_EG_AC_PORT_CVID_INFO_GET);

  SOC_SAND_CHECK_DRIVER_AND_DEVICE;

  SOC_SAND_CHECK_NULL_INPUT(cep_key);
  SOC_SAND_CHECK_NULL_INPUT(out_ac);
  SOC_SAND_CHECK_NULL_INPUT(ac_info);
  SOC_SAND_CHECK_NULL_INPUT(found);

  res = arad_pp_eg_ac_port_cvid_info_get_verify(
          unit,
          cep_key
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  SOC_SAND_TAKE_DEVICE_SEMAPHORE;

  res = arad_pp_eg_ac_port_cvid_info_get_unsafe(
          unit,
          cep_key,
          out_ac,
          ac_info,
          found
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit_semaphore);

exit_semaphore:
  SOC_SAND_GIVE_DEVICE_SEMAPHORE;
  ARAD_PP_DO_NOTHING_AND_EXIT;
exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in soc_ppd_eg_ac_port_cvid_info_get()", 0, 0);
}

/* } */

#include <soc/dpp/SAND/Utils/sand_footer.h>



