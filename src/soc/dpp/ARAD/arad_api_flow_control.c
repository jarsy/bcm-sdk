
#include <soc/mcm/memregs.h>
#if defined(BCM_88650_A0)
/* $Id: arad_api_flow_control.c,v 1.18 Broadcom SDK $
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
#include <soc/dcmn/error.h>
#include <soc/dpp/SAND/Utils/sand_header.h>

#include <soc/dpp/SAND/Management/sand_general_macros.h>
#include <soc/dpp/SAND/Management/sand_error_code.h>
#include <soc/dpp/SAND/Utils/sand_os_interface.h>

#include <soc/dpp/ARAD/arad_api_framework.h>
#include <soc/dpp/ARAD/arad_flow_control.h>


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
  arad_fc_gen_inbnd_glb_hp_set(
    SOC_SAND_IN  int                      unit,
    SOC_SAND_IN  SOC_TMC_FC_INGR_GEN_GLB_HP_MODE     fc_mode
  )
{
  uint32
    res = SOC_SAND_OK;

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_FC_GEN_INBND_GLB_HP_SET);

  SOC_SAND_CHECK_DRIVER_AND_DEVICE;

  res = arad_fc_gen_inbnd_glb_hp_set_verify(
          unit,
          fc_mode
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  SOC_SAND_TAKE_DEVICE_SEMAPHORE;

  res = arad_fc_gen_inbnd_glb_hp_set_unsafe(
          unit,
          fc_mode
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit_semaphore);

exit_semaphore:
  SOC_SAND_GIVE_DEVICE_SEMAPHORE;
  ARAD_DO_NOTHING_AND_EXIT;
exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_fc_gen_inbnd_glb_hp_set()", 0, 0);
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
  arad_fc_gen_inbnd_glb_hp_get(
    SOC_SAND_IN  int                      unit,
    SOC_SAND_OUT SOC_TMC_FC_INGR_GEN_GLB_HP_MODE     *fc_mode
  )
{
  uint32
    res = SOC_SAND_OK;

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_FC_GEN_INBND_GLB_HP_GET);

  SOC_SAND_CHECK_DRIVER_AND_DEVICE;

  SOC_SAND_CHECK_NULL_INPUT(fc_mode);

  res = arad_fc_gen_inbnd_glb_hp_get_verify(
          unit
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  SOC_SAND_TAKE_DEVICE_SEMAPHORE;

  res = arad_fc_gen_inbnd_glb_hp_get_unsafe(
          unit,
          fc_mode
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit_semaphore);

exit_semaphore:
  SOC_SAND_GIVE_DEVICE_SEMAPHORE;
  ARAD_DO_NOTHING_AND_EXIT;
exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_fc_gen_inbnd_glb_hp_get()", 0, 0);
}

/*********************************************************************
*     Map the 3-LSB of the CMN CPID (represent TC), to the FC
 *     indication to generate when using Class Based Flow
 *     Control
 *     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  arad_fc_gen_inbnd_cnm_map_set(
    SOC_SAND_IN  int                      unit,
    SOC_SAND_IN  uint32                      cpid_tc_ndx,
    SOC_SAND_IN  uint8                      enable_ll,
    SOC_SAND_IN  uint32                      fc_class
  )
{
  uint32
    res = SOC_SAND_OK;

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_FC_GEN_INBND_CNM_MAP_SET);

  SOC_SAND_CHECK_DRIVER_AND_DEVICE;

  res = arad_fc_gen_inbnd_cnm_map_set_verify(
          unit,
          cpid_tc_ndx,
          enable_ll,
          fc_class
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  SOC_SAND_TAKE_DEVICE_SEMAPHORE;

  res = arad_fc_gen_inbnd_cnm_map_set_unsafe(
          unit,
          cpid_tc_ndx,
          enable_ll,
          fc_class
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit_semaphore);

exit_semaphore:
  SOC_SAND_GIVE_DEVICE_SEMAPHORE;
  ARAD_DO_NOTHING_AND_EXIT;
exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_fc_gen_inbnd_cnm_map_set()", cpid_tc_ndx, 0);
}

/*********************************************************************
*     Map the 3-LSB of the CMN CPID (represent TC), to the FC
 *     indication to generate when using Class Based Flow
 *     Control
 *     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  arad_fc_gen_inbnd_cnm_map_get(
    SOC_SAND_IN  int                      unit,
    SOC_SAND_IN  uint32                      cpid_tc_ndx,
    SOC_SAND_OUT uint8                      *enable_ll,
    SOC_SAND_OUT uint32                      *fc_class
  )
{
  uint32
    res = SOC_SAND_OK;

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_FC_GEN_INBND_CNM_MAP_GET);

  SOC_SAND_CHECK_DRIVER_AND_DEVICE;

  SOC_SAND_CHECK_NULL_INPUT(enable_ll);
  SOC_SAND_CHECK_NULL_INPUT(fc_class);

  res = arad_fc_gen_inbnd_cnm_map_get_verify(
          unit,
          cpid_tc_ndx
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  SOC_SAND_TAKE_DEVICE_SEMAPHORE;

  res = arad_fc_gen_inbnd_cnm_map_get_unsafe(
          unit,
          cpid_tc_ndx,
          enable_ll,
          fc_class
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit_semaphore);

exit_semaphore:
  SOC_SAND_GIVE_DEVICE_SEMAPHORE;
  ARAD_DO_NOTHING_AND_EXIT;
exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_fc_gen_inbnd_cnm_map_get()", cpid_tc_ndx, 0);
}


/*********************************************************************
*     Diagnostics - get the Out-Of-Band interface status
 *     Details: in the H file. (search for prototype)
*********************************************************************/




#if defined(BCM_88650_B0)

uint32
  arad_fc_ilkn_retransmit_cal_set(
    SOC_SAND_IN  int                             unit,
    SOC_SAND_IN  ARAD_NIF_ILKN_ID                   ilkn_ndx,
    SOC_SAND_IN  ARAD_FC_DIRECTION                  direction_ndx,
    SOC_SAND_IN  ARAD_FC_ILKN_RETRANSMIT_CAL_CFG    *cal_cfg
  )
{
  uint32
    res = SOC_SAND_OK;

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_FC_ILKN_RETRANSMIT_CAL_SET);

  SOC_SAND_CHECK_DRIVER_AND_DEVICE;

  SOC_SAND_CHECK_NULL_INPUT(cal_cfg);

  if(SOC_IS_ARAD_B0_AND_ABOVE(unit))
  {
    res = arad_fc_ilkn_retransmit_cal_set_verify(
          unit,
          ilkn_ndx,
          direction_ndx,
          cal_cfg
        );
    SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);
  }
  SOC_SAND_TAKE_DEVICE_SEMAPHORE;

  if(SOC_IS_ARAD_B0_AND_ABOVE(unit))
  {
    res = arad_fc_ilkn_retransmit_cal_set_unsafe(
            unit,
            ilkn_ndx,
            direction_ndx,
            cal_cfg
          );
    SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit_semaphore);
  }

exit_semaphore:
  SOC_SAND_GIVE_DEVICE_SEMAPHORE;
  ARAD_DO_NOTHING_AND_EXIT;
exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_fc_ilkn_retransmit_cal_set()", ilkn_ndx, direction_ndx);
}

uint32
  arad_fc_ilkn_retransmit_cal_get(
    SOC_SAND_IN  int                             unit,
    SOC_SAND_IN  ARAD_NIF_ILKN_ID                   ilkn_ndx,
    SOC_SAND_IN  ARAD_FC_DIRECTION                  direction_ndx,
    SOC_SAND_OUT ARAD_FC_ILKN_RETRANSMIT_CAL_CFG    *cal_cfg
  )
{
  uint32
    res = SOC_SAND_OK;

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_FC_ILKN_RETRANSMIT_CAL_GET);

  SOC_SAND_CHECK_DRIVER_AND_DEVICE;

  SOC_SAND_CHECK_NULL_INPUT(cal_cfg);

  SOC_SAND_TAKE_DEVICE_SEMAPHORE;

  if(SOC_IS_ARAD_B0_AND_ABOVE(unit))
  {
    res = arad_fc_ilkn_retransmit_cal_get_unsafe(
            unit,
            ilkn_ndx,
            direction_ndx,
            cal_cfg
          );
    SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit_semaphore);
  }

exit_semaphore:
  SOC_SAND_GIVE_DEVICE_SEMAPHORE;
  ARAD_DO_NOTHING_AND_EXIT;
exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_fc_ilkn_retransmit_cal_get()", ilkn_ndx, direction_ndx);
}

#endif

void 
  ARAD_FC_ILKN_RETRANSMIT_INFO_clear(
    SOC_SAND_OUT ARAD_FC_ILKN_RETRANSMIT_INFO *info
  )
{
  SOC_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  SOC_SAND_CHECK_NULL_INPUT(info);

  soc_sand_os_memset(info, 0x0, sizeof(ARAD_FC_ILKN_RETRANSMIT_INFO));
  info->enable = 0;
  info->error_indication = 0;
  info->rx_polarity= 0;
  info->tx_polarity = 0;
  SOC_SAND_MAGIC_NUM_SET;
exit:
  SOC_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

#if defined(BCM_88650_B0)
void 
  ARAD_FC_ILKN_RETRANSMIT_CAL_CFG_clear(
    SOC_SAND_OUT ARAD_FC_ILKN_RETRANSMIT_CAL_CFG *info
  )
{
  SOC_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  SOC_SAND_CHECK_NULL_INPUT(info);

  soc_sand_os_memset(info, 0x0, sizeof(ARAD_FC_ILKN_RETRANSMIT_CAL_CFG));
  info->enable = 0;
  info->length = 1;
  SOC_SAND_MAGIC_NUM_SET;
exit:
  SOC_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}
#endif


#include <soc/dpp/SAND/Utils/sand_footer.h>


#endif /* of #if defined(BCM_88650_A0) */

