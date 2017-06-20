
#include <soc/mcm/memregs.h>
#if defined(BCM_88690_A0)
/* $Id: jer2_arad_api_flow_control.c,v 1.18 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/
#ifdef _ERR_MSG_MODULE_NAME
  #error "_ERR_MSG_MODULE_NAME redefined"
#endif

#define _ERR_MSG_MODULE_NAME BSL_SOC_FLOWCONTROL
/*************
 * INCLUDES  *
 *************/
/* { */
#include <shared/bsl.h>
#include <soc/dnxc/legacy/error.h>
#include <soc/dnx/legacy/SAND/Utils/sand_header.h>

#include <soc/dnx/legacy/SAND/Management/sand_general_macros.h>
#include <soc/dnx/legacy/SAND/Management/sand_error_code.h>
#include <soc/dnx/legacy/SAND/Utils/sand_os_interface.h>

#include <soc/dnx/legacy/ARAD/arad_api_framework.h>
#include <soc/dnx/legacy/ARAD/arad_flow_control.h>


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
*     Inband Flow Control Generation Configuration, based on
 *     Arad Ingress state indications.
 *     Details: in the H file. (search for prototype)
*********************************************************************/

/*********************************************************************
*     Enable/disable Flow Control generation, based on Ingress
 *     Global Resources - high priority, via NIF. Flow Control
 *     generation may be either Link Level or Class Based. For
 *     Link Level - Flow Control will be generated on all
 *     links. For Class Based - Flow Control will be generated
 *     on all Flow Control Classes.
 *     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  jer2_arad_fc_gen_inbnd_glb_hp_set(
    DNX_SAND_IN  int                      unit,
    DNX_SAND_IN  DNX_TMC_FC_INGR_GEN_GLB_HP_MODE     fc_mode
  )
{
  uint32
    res = DNX_SAND_OK;

  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_FC_GEN_INBND_GLB_HP_SET);

  DNX_SAND_CHECK_DRIVER_AND_DEVICE;

  res = jer2_arad_fc_gen_inbnd_glb_hp_set_verify(
          unit,
          fc_mode
        );
  DNX_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  DNX_SAND_TAKE_DEVICE_SEMAPHORE;

  res = jer2_arad_fc_gen_inbnd_glb_hp_set_unsafe(
          unit,
          fc_mode
        );
  DNX_SAND_CHECK_FUNC_RESULT(res, 20, exit_semaphore);

exit_semaphore:
  DNX_SAND_GIVE_DEVICE_SEMAPHORE;
  JER2_ARAD_DO_NOTHING_AND_EXIT;
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_fc_gen_inbnd_glb_hp_set()", 0, 0);
}

/*********************************************************************
*     Enable/disable Flow Control generation, based on Ingress
 *     Global Resources - high priority, via NIF. Flow Control
 *     generation may be either Link Level or Class Based. For
 *     Link Level - Flow Control will be generated on all
 *     links. For Class Based - Flow Control will be generated
 *     on all Flow Control Classes.
 *     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  jer2_arad_fc_gen_inbnd_glb_hp_get(
    DNX_SAND_IN  int                      unit,
    DNX_SAND_OUT DNX_TMC_FC_INGR_GEN_GLB_HP_MODE     *fc_mode
  )
{
  uint32
    res = DNX_SAND_OK;

  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_FC_GEN_INBND_GLB_HP_GET);

  DNX_SAND_CHECK_DRIVER_AND_DEVICE;

  DNX_SAND_CHECK_NULL_INPUT(fc_mode);

  res = jer2_arad_fc_gen_inbnd_glb_hp_get_verify(
          unit
        );
  DNX_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  DNX_SAND_TAKE_DEVICE_SEMAPHORE;

  res = jer2_arad_fc_gen_inbnd_glb_hp_get_unsafe(
          unit,
          fc_mode
        );
  DNX_SAND_CHECK_FUNC_RESULT(res, 20, exit_semaphore);

exit_semaphore:
  DNX_SAND_GIVE_DEVICE_SEMAPHORE;
  JER2_ARAD_DO_NOTHING_AND_EXIT;
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_fc_gen_inbnd_glb_hp_get()", 0, 0);
}

/*********************************************************************
*     Map the 3-LSB of the CMN CPID (represent TC), to the FC
 *     indication to generate when using Class Based Flow
 *     Control
 *     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  jer2_arad_fc_gen_inbnd_cnm_map_set(
    DNX_SAND_IN  int                      unit,
    DNX_SAND_IN  uint32                      cpid_tc_ndx,
    DNX_SAND_IN  uint8                      enable_ll,
    DNX_SAND_IN  uint32                      fc_class
  )
{
  uint32
    res = DNX_SAND_OK;

  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_FC_GEN_INBND_CNM_MAP_SET);

  DNX_SAND_CHECK_DRIVER_AND_DEVICE;

  res = jer2_arad_fc_gen_inbnd_cnm_map_set_verify(
          unit,
          cpid_tc_ndx,
          enable_ll,
          fc_class
        );
  DNX_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  DNX_SAND_TAKE_DEVICE_SEMAPHORE;

  res = jer2_arad_fc_gen_inbnd_cnm_map_set_unsafe(
          unit,
          cpid_tc_ndx,
          enable_ll,
          fc_class
        );
  DNX_SAND_CHECK_FUNC_RESULT(res, 20, exit_semaphore);

exit_semaphore:
  DNX_SAND_GIVE_DEVICE_SEMAPHORE;
  JER2_ARAD_DO_NOTHING_AND_EXIT;
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_fc_gen_inbnd_cnm_map_set()", cpid_tc_ndx, 0);
}

/*********************************************************************
*     Map the 3-LSB of the CMN CPID (represent TC), to the FC
 *     indication to generate when using Class Based Flow
 *     Control
 *     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  jer2_arad_fc_gen_inbnd_cnm_map_get(
    DNX_SAND_IN  int                      unit,
    DNX_SAND_IN  uint32                      cpid_tc_ndx,
    DNX_SAND_OUT uint8                      *enable_ll,
    DNX_SAND_OUT uint32                      *fc_class
  )
{
  uint32
    res = DNX_SAND_OK;

  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_FC_GEN_INBND_CNM_MAP_GET);

  DNX_SAND_CHECK_DRIVER_AND_DEVICE;

  DNX_SAND_CHECK_NULL_INPUT(enable_ll);
  DNX_SAND_CHECK_NULL_INPUT(fc_class);

  res = jer2_arad_fc_gen_inbnd_cnm_map_get_verify(
          unit,
          cpid_tc_ndx
        );
  DNX_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  DNX_SAND_TAKE_DEVICE_SEMAPHORE;

  res = jer2_arad_fc_gen_inbnd_cnm_map_get_unsafe(
          unit,
          cpid_tc_ndx,
          enable_ll,
          fc_class
        );
  DNX_SAND_CHECK_FUNC_RESULT(res, 20, exit_semaphore);

exit_semaphore:
  DNX_SAND_GIVE_DEVICE_SEMAPHORE;
  JER2_ARAD_DO_NOTHING_AND_EXIT;
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_fc_gen_inbnd_cnm_map_get()", cpid_tc_ndx, 0);
}


/*********************************************************************
*     Diagnostics - get the Out-Of-Band interface status
 *     Details: in the H file. (search for prototype)
*********************************************************************/





uint32
  jer2_arad_fc_ilkn_retransmit_cal_set(
    DNX_SAND_IN  int                             unit,
    DNX_SAND_IN  int                   ilkn_ndx,
    DNX_SAND_IN  JER2_ARAD_FC_DIRECTION                  direction_ndx,
    DNX_SAND_IN  JER2_ARAD_FC_ILKN_RETRANSMIT_CAL_CFG    *cal_cfg
  )
{
  uint32
    res = DNX_SAND_OK;

  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_FC_ILKN_RETRANSMIT_CAL_SET);

  DNX_SAND_CHECK_DRIVER_AND_DEVICE;

  DNX_SAND_CHECK_NULL_INPUT(cal_cfg);

  if(SOC_IS_ARAD_B0_AND_ABOVE(unit))
  {
    res = jer2_arad_fc_ilkn_retransmit_cal_set_verify(
          unit,
          ilkn_ndx,
          direction_ndx,
          cal_cfg
        );
    DNX_SAND_CHECK_FUNC_RESULT(res, 10, exit);
  }
  DNX_SAND_TAKE_DEVICE_SEMAPHORE;

  if(SOC_IS_ARAD_B0_AND_ABOVE(unit))
  {
    res = jer2_arad_fc_ilkn_retransmit_cal_set_unsafe(
            unit,
            ilkn_ndx,
            direction_ndx,
            cal_cfg
          );
    DNX_SAND_CHECK_FUNC_RESULT(res, 20, exit_semaphore);
  }

exit_semaphore:
  DNX_SAND_GIVE_DEVICE_SEMAPHORE;
  JER2_ARAD_DO_NOTHING_AND_EXIT;
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_fc_ilkn_retransmit_cal_set()", ilkn_ndx, direction_ndx);
}

uint32
  jer2_arad_fc_ilkn_retransmit_cal_get(
    DNX_SAND_IN  int                             unit,
    DNX_SAND_IN  int                   ilkn_ndx,
    DNX_SAND_IN  JER2_ARAD_FC_DIRECTION                  direction_ndx,
    DNX_SAND_OUT JER2_ARAD_FC_ILKN_RETRANSMIT_CAL_CFG    *cal_cfg
  )
{
  uint32
    res = DNX_SAND_OK;

  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_FC_ILKN_RETRANSMIT_CAL_GET);

  DNX_SAND_CHECK_DRIVER_AND_DEVICE;

  DNX_SAND_CHECK_NULL_INPUT(cal_cfg);

  DNX_SAND_TAKE_DEVICE_SEMAPHORE;

  if(SOC_IS_ARAD_B0_AND_ABOVE(unit))
  {
    res = jer2_arad_fc_ilkn_retransmit_cal_get_unsafe(
            unit,
            ilkn_ndx,
            direction_ndx,
            cal_cfg
          );
    DNX_SAND_CHECK_FUNC_RESULT(res, 20, exit_semaphore);
  }

exit_semaphore:
  DNX_SAND_GIVE_DEVICE_SEMAPHORE;
  JER2_ARAD_DO_NOTHING_AND_EXIT;
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_fc_ilkn_retransmit_cal_get()", ilkn_ndx, direction_ndx);
}

void 
  JER2_ARAD_FC_ILKN_RETRANSMIT_INFO_clear(
    DNX_SAND_OUT JER2_ARAD_FC_ILKN_RETRANSMIT_INFO *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  dnx_sand_os_memset(info, 0x0, sizeof(JER2_ARAD_FC_ILKN_RETRANSMIT_INFO));
  info->enable = 0;
  info->error_indication = 0;
  info->rx_polarity= 0;
  info->tx_polarity = 0;
  DNX_SAND_MAGIC_NUM_SET;
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void 
  JER2_ARAD_FC_ILKN_RETRANSMIT_CAL_CFG_clear(
    DNX_SAND_OUT JER2_ARAD_FC_ILKN_RETRANSMIT_CAL_CFG *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  dnx_sand_os_memset(info, 0x0, sizeof(JER2_ARAD_FC_ILKN_RETRANSMIT_CAL_CFG));
  info->enable = 0;
  info->length = 1;
  DNX_SAND_MAGIC_NUM_SET;
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}


#include <soc/dnx/legacy/SAND/Utils/sand_footer.h>


#endif /* of #if defined(BCM_88690_A0) */

