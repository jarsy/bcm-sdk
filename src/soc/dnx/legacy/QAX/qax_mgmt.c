/*
 * $Id: $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File: jer2_qax_mgmt.c
 */

#ifdef _ERR_MSG_MODULE_NAME
  #error "_ERR_MSG_MODULE_NAME redefined"
#endif

#define _ERR_MSG_MODULE_NAME BSL_SOC_INIT
#include <shared/bsl.h>
#include <soc/dnx/legacy/drv.h>
#include <soc/dnx/legacy/QAX/qax_mgmt.h>
#if 0
#include <soc/dnx/legacy/JER/jer_defs.h>
#include <soc/dnx/legacy/JER/jer_fabric.h>
#endif /* 0 */

#define JER2_QAX_MGMT_NOF_PROCESSOR_IDS 17

/*********************************************************************
* Set the fabric system ID of the device. Must be unique in the system.
*********************************************************************/
uint32 jer2_qax_mgmt_system_fap_id_set(
    DNX_SAND_IN  int     unit,
    DNX_SAND_IN  uint32  sys_fap_id
  )
{
    uint32 reg32;
    DNXC_INIT_FUNC_DEFS;
    if (sys_fap_id >= JER2_ARAD_NOF_FAPS_IN_SYSTEM) {
        DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("FAP ID %u is illegal, must be under %u."),
          sys_fap_id, JER2_ARAD_NOF_FAPS_IN_SYSTEM));
    }
    /* configure the FAP ID, and TDM source FAP ID */
    DNXC_IF_ERR_EXIT(READ_ECI_GLOBAL_GENERAL_CFG_2r(unit, &reg32));
    soc_reg_field_set(unit, ECI_GLOBAL_GENERAL_CFG_2r, &reg32, DEV_IDf, sys_fap_id);
    DNXC_IF_ERR_EXIT(WRITE_ECI_GLOBAL_GENERAL_CFG_2r(unit, reg32));

    if (SOC_DNX_CONFIG(unit)->tdm.is_bypass &&
        SOC_DNX_CONFIG(unit)->jer2_arad->init.fabric.is_128_in_system &&
        SOC_DNX_CONFIG(unit)->tm.is_petrab_in_system &&
        !SOC_IS_QUX(unit)) {
        DNXC_IF_ERR_EXIT(soc_reg_above_64_field32_modify(unit, FDT_TDM_CONFIGURATIONr,
          REG_PORT_ANY, 0, TDM_SOURCE_FAP_IDf,
          sys_fap_id + SOC_DNX_CONFIG(unit)->jer2_arad->tdm_source_fap_id_offset));
    }

exit:
    DNXC_FUNC_RETURN;
}

/*********************************************************************
* Get the fabric system ID of the device. Must be unique in the system.
*********************************************************************/
uint32
  jer2_qax_mgmt_system_fap_id_get(
    DNX_SAND_IN  int       unit,
    DNX_SAND_OUT uint32    *sys_fap_id
  )
{
  uint32       fld_val;

  DNXC_INIT_FUNC_DEFS;
  DNXC_NULL_CHECK(sys_fap_id);


  DNXC_IF_ERR_EXIT(soc_reg_above_64_field32_read(unit, ECI_GLOBAL_GENERAL_CFG_2r, REG_PORT_ANY, 0, DEV_IDf, &fld_val));
  *sys_fap_id = fld_val;

exit:
    DNXC_FUNC_RETURN;
}

#undef _ERR_MSG_MODULE_NAME

