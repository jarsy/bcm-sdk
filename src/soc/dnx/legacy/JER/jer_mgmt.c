/*
 * $Id: $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File: jer2_jer_mgmt.c
 */

#ifdef _ERR_MSG_MODULE_NAME
  #error "_ERR_MSG_MODULE_NAME redefined"
#endif

#define _ERR_MSG_MODULE_NAME BSL_SOC_INIT
#include <shared/swstate/access/sw_state_access.h>
#include <shared/bsl.h>
#include <soc/dnx/legacy/drv.h>
#include <soc/dnx/legacy/ARAD/arad_mgmt.h>
#include <soc/dnx/legacy/ARAD/arad_tbl_access.h>
#include <soc/dnx/legacy/JER/jer_mgmt.h>
#include <soc/dnx/legacy/JER/jer_defs.h>
#include <soc/dnx/legacy/JER/jer_fabric.h>

/*
 * PVT monitor
 */

#define _SOC_JER2_JER_ECI_PVT_MON_CONTROL_REG_POWERDOWN_BIT   (32)
#define _SOC_JER2_JER_ECI_PVT_MON_CONTROL_REG_RESET_BIT       (33)

#define _SOC_JER2_JER_ECI_PVT_MON_CONTROL_REG_RESET_BIT       (33)
#define _SOC_JER2_JER_PVT_MON_NOF                             (SOC_IS_QAX(unit) ? 2 : 4)
#define _SOC_JER2_JER_PVT_FACTOR                              (49103)
#define _SOC_JER2_JER_PVT_BASE                                (41205000)

#define JER2_JER_FAPID_BIT_OFFSET_IN_DQCQ_MAP 16
#define JER2_JER_MGMT_NOF_CORES 2
#define JER2_JER_MGMT_NOF_PROCESSOR_IDS 18
#define JER2_JER_CR_VAL_BMP_NOF_BITS 16

jer2_jer_mgmt_dma_fifo_source_channels_t jer2_jer_mgmt_dma_fifo_source_channels_db[SOC_MAX_NUM_DEVICES];


/*********************************************************************
*     Bytes-worth of a single credit. It should be configured
*     the same in all the FAPs in the systems, and should be
*     set before programming the scheduler.
*     Details: in the H file. (search for prototype)
*********************************************************************/
int
  jer2_jer_mgmt_credit_worth_set(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  uint32              credit_worth
  )
{
  uint32
    fld_val = 0;
  soc_reg_t credit_value_reg;
  soc_field_t credit_value_lcl_field;

  DNXC_INIT_FUNC_DEFS;

  DNXC_SAND_IF_ERR_EXIT(jer2_arad_mgmt_credit_worth_verify(unit, credit_worth)) ;

  fld_val = credit_worth;
  /*
   * Jericho: Note that the register is different than in Arad and the fields
   * are marked '0' and '1' while in Arad, the same fields are marked '1' and '2'.
   */
  credit_value_reg = SOC_IS_JERICHO_PLUS_A0(unit) ? IPST_CREDIT_CONFIG_1r : IPS_IPS_CREDIT_CONFIGr ;
  credit_value_lcl_field = CREDIT_VALUE_0f;
  DNXC_IF_ERR_EXIT(soc_reg_field32_modify(unit, credit_value_reg, REG_PORT_ANY, credit_value_lcl_field, fld_val)) ;

exit:
  DNXC_FUNC_RETURN;
}
/*********************************************************************
*     Bytes-worth of a single credit. It should be configured
*     the same in all the FAPs in the systems, and should be
*     set before programming the scheduler.
*     Details: in the H file. (search for prototype)
*********************************************************************/
int
   jer2_jer_mgmt_credit_worth_get(
            DNX_SAND_IN  int                 unit,
            DNX_SAND_OUT uint32              *credit_worth
           )
{
  uint32
      reg_val,
      fld_val = 0;
  soc_reg_t credit_value_reg;
  soc_field_t credit_value_lcl_field;

  DNXC_INIT_FUNC_DEFS;
  DNXC_NULL_CHECK(credit_worth);

  /*
   * Jericho: Note that the register is different than in Arad and the fields
   * are marked '0' and '1' while in Arad, the same fields are marked '1' and '2'.
   */
  credit_value_reg = !SOC_IS_JERICHO_PLUS_A0(unit) ? IPS_IPS_CREDIT_CONFIGr : IPST_CREDIT_CONFIG_1r;
  credit_value_lcl_field = CREDIT_VALUE_0f ;
  /*
   * Read selected register and, then, extract the filed.
   */
  DNXC_IF_ERR_EXIT(soc_reg32_get(unit, credit_value_reg, REG_PORT_ANY, 0, &reg_val)) ;
  fld_val = soc_reg_field_get(unit, credit_value_reg, reg_val, credit_value_lcl_field) ;
  *credit_worth = fld_val;
exit:
  DNXC_FUNC_RETURN;
}

/*
 * Jericho only: set local and remote (0 and 1) credit worth values
 */
uint32
  jer2_jer_mgmt_credit_worth_remote_set(
    DNX_SAND_IN  int    unit,
    DNX_SAND_IN  uint32    credit_worth_remote
  )
{
    uint32 res, reg_val;
    uint16 nof_remote_faps_with_remote_credit_value;
    DNX_SAND_INIT_ERROR_DEFINITIONS(0);
    DNX_SAND_SOC_IF_ERROR_RETURN(res, 5, exit, sw_state_access[unit].dnx.soc.jer2_jericho.tm.nof_remote_faps_with_remote_credit_value.get(unit, &nof_remote_faps_with_remote_credit_value));

    if (credit_worth_remote < DNX_TMC_CREDIT_SIZE_BYTES_MIN || credit_worth_remote > DNX_TMC_CREDIT_SIZE_BYTES_MAX) {
        LOG_ERROR(BSL_LS_SOC_MANAGEMENT, (BSL_META_U(unit, "Remote size %d is not between %u..%u") , credit_worth_remote, DNX_TMC_CREDIT_SIZE_BYTES_MIN, DNX_TMC_CREDIT_SIZE_BYTES_MAX));
        DNX_SAND_SET_ERROR_CODE(DNX_SAND_VALUE_OUT_OF_RANGE_ERR, 10, exit);
    } else {
        uint32 arg_local, arg_remote;
        jer2_jer_mgmt_credit_worth_remote_get(unit, &arg_remote); /*need to be in sand_error???*/
        jer2_jer_mgmt_credit_worth_get(unit, &arg_local); /*need to be in sand_error???*/
        if (credit_worth_remote != arg_remote) { /* are we changing the value? */
            if (nof_remote_faps_with_remote_credit_value) { /* is the current value being used (by remote FAPs)? */
                if (credit_worth_remote != arg_local) {
                    LOG_ERROR(BSL_LS_SOC_MANAGEMENT,
                              (BSL_META_U(unit,"The Remote credit value is assigned to remote devices. To change the value you must first assign the local credit value to these devices.")));
                    DNX_SAND_SET_ERROR_CODE(DNX_SAND_VALUE_OUT_OF_RANGE_ERR, 20, exit);
                } else { /* the local and (previous) remote values are equal, so we can just mark all FAPs as using the local value */
                    DNX_SAND_SOC_IF_ERROR_RETURN(res, 30, exit, jer2_jer_mgmt_change_all_faps_credit_worth_unsafe(unit, DNX_TMC_FAP_CREDIT_VALUE_LOCAL));
                }
            } else {
                DNX_SAND_SOC_IF_ERROR_RETURN(res, 100, exit, READ_IPS_IPS_CREDIT_CONFIGr(unit, &reg_val));
                soc_reg_field_set(unit, IPS_IPS_CREDIT_CONFIGr, &reg_val, CREDIT_VALUE_1f, credit_worth_remote);
                DNX_SAND_SOC_IF_ERROR_RETURN(res, 1100, exit, WRITE_IPS_IPS_CREDIT_CONFIGr(unit, reg_val));
            }
        }
    }

exit:
    DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_jer_mgmt_credit_worth_remote_set()", unit, 0);
}

/*
 * Jericho only: set local and remote (0 and 1) credit worth values
 */
uint32
  jer2_jer_mgmt_credit_worth_remote_get(
    DNX_SAND_IN  int    unit,
    DNX_SAND_OUT uint32    *credit_worth_remote
  )
{
    uint32 res, reg_val;
    DNX_SAND_INIT_ERROR_DEFINITIONS(0);

    DNX_SAND_CHECK_NULL_INPUT(credit_worth_remote);
    DNX_SAND_SOC_IF_ERROR_RETURN(res, 100, exit, READ_IPS_IPS_CREDIT_CONFIGr(unit, &reg_val));
    *credit_worth_remote = soc_reg_field_get(unit, IPS_IPS_CREDIT_CONFIGr, reg_val, CREDIT_VALUE_1f);
exit:
    DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_jer_mgmt_credit_worth_remote_get()", unit, 0);
}


/*
 * Jericho only: map the module (fap_id) to the given credit value (local, remote or non mapped).
 * Has no special handling of the local device (should not be used for the local device).
 */
uint32
  jer2_jer_mgmt_module_to_credit_worth_map_set(
    DNX_SAND_IN  int    unit,
    DNX_SAND_IN  uint32    fap_id,
    DNX_SAND_IN  uint32    credit_value_type /* should be one of JER2_JERICHO_FAP_CREDIT_VALUE_* */
  )
{
    uint32 data = 0, fap_bitmap[1], res, remote_bit;
    uint16 nof_remote_faps_with_remote_credit_value;
    soc_mem_t mem;
    DNX_SAND_INIT_ERROR_DEFINITIONS(0);
    if (fap_id > DNX_TMC_NOF_FAPS_IN_SYSTEM) {
        DNX_SAND_SET_ERROR_CODE(JER2_ARAD_FAP_FABRIC_ID_OUT_OF_RANGE_ERR, 10, exit);
    }

    DNX_SAND_SOC_IF_ERROR_RETURN(res, 15, exit, sw_state_access[unit].dnx.soc.jer2_jericho.tm.nof_remote_faps_with_remote_credit_value.get(unit, &nof_remote_faps_with_remote_credit_value));
    DNX_SAND_SOC_IF_ERROR_RETURN(res, 20, exit, READ_IPS_CREDIT_CONFIGr(unit, &data)); 
    soc_reg_field_set(unit, IPS_CREDIT_CONFIGr, &data, CR_VAL_SEL_ENABLEf, TRUE);
    DNX_SAND_SOC_IF_ERROR_RETURN(res, 30, exit, WRITE_IPS_CREDIT_CONFIGr(unit, data)); 

    data = 0;
    
    mem =  SOC_IS_JERICHO_PLUS_A0(unit) ? IPST_CRVSm : IPS_CRVSm;
    DNX_SAND_SOC_IF_ERROR_RETURN(res, 40, exit, soc_mem_read(unit, mem, MEM_BLOCK_ANY, fap_id / JER2_JER_CR_VAL_BMP_NOF_BITS, &data));
    *fap_bitmap = soc_mem_field32_get(unit, mem, &data, CR_VAL_BMPf);

    remote_bit = SHR_BITGET(fap_bitmap,(fap_id % JER2_JER_CR_VAL_BMP_NOF_BITS));
    if (credit_value_type == DNX_TMC_FAP_CREDIT_VALUE_LOCAL) {
        if (remote_bit != DNX_TMC_FAP_CREDIT_VALUE_LOCAL) {
            SHR_BITCLR(fap_bitmap,(fap_id % JER2_JER_CR_VAL_BMP_NOF_BITS));
            --nof_remote_faps_with_remote_credit_value;
        }
    } else if (credit_value_type == DNX_TMC_FAP_CREDIT_VALUE_REMOTE) {
        if (remote_bit != DNX_TMC_FAP_CREDIT_VALUE_REMOTE) {
            SHR_BITSET(fap_bitmap,(fap_id % JER2_JER_CR_VAL_BMP_NOF_BITS));
            ++nof_remote_faps_with_remote_credit_value;
        }
    } else {
        DNX_SAND_SET_ERROR_CODE(JER2_ARAD_CREDIT_TYPE_INVALID_ERR, 40, exit);
    }
    
    soc_mem_field32_set(unit, mem, &data, CR_VAL_BMPf, *fap_bitmap); 
    DNX_SAND_SOC_IF_ERROR_RETURN(res, 50, exit, soc_mem_write(unit, mem, MEM_BLOCK_ANY, fap_id / JER2_JER_CR_VAL_BMP_NOF_BITS, &data));
    DNX_SAND_SOC_IF_ERROR_RETURN(res, 60, exit, sw_state_access[unit].dnx.soc.jer2_jericho.tm.nof_remote_faps_with_remote_credit_value.set(unit, nof_remote_faps_with_remote_credit_value));
exit:
    DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_jer_mgmt_module_to_credit_worth_map_set()", unit, 0);
}

/*
 * Jericho only: Get the mapping the module (fap_id) to the given credit value (local, remote or non mapped).
 * Has no special handling of the local device (should not be used for the local device).
 */
uint32
  jer2_jer_mgmt_module_to_credit_worth_map_get(
    DNX_SAND_IN  int    unit,
    DNX_SAND_IN  uint32    fap_id,
    DNX_SAND_OUT uint32    *credit_value_type /* will be one of JER2_JERICHO_FAP_CREDIT_VALUE_* */
  )
{
    uint32 data = 0, fap_bitmap[1], res;
    soc_mem_t mem;
    DNX_SAND_INIT_ERROR_DEFINITIONS(0);

    if (fap_id > DNX_TMC_NOF_FAPS_IN_SYSTEM) {
        DNX_SAND_SET_ERROR_CODE(JER2_ARAD_FAP_FABRIC_ID_OUT_OF_RANGE_ERR, 30, exit);
    }

    
    mem =  SOC_IS_JERICHO_PLUS_A0(unit) ? IPST_CRVSm : IPS_CRVSm;
    DNX_SAND_SOC_IF_ERROR_RETURN(res, 40, exit, soc_mem_read(unit, mem, MEM_BLOCK_ANY, fap_id / JER2_JER_CR_VAL_BMP_NOF_BITS, &data));
    *fap_bitmap = soc_mem_field32_get(unit, mem, &data, CR_VAL_BMPf);
    *credit_value_type = SHR_BITGET(fap_bitmap, (fap_id % JER2_JER_CR_VAL_BMP_NOF_BITS));

exit:
    DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_jer_mgmt_module_to_credit_worth_map_get()", unit, 0);

}


/*
 * Jericho only: in case the local and remote credit values are equal, change all configure remote FAPS to use the local or remote value.
 * The credit_value_to_use selects to which value we should make the FAPS use:
 *   DNX_TMC_FAP_CREDIT_VALUE_LOCAL  - use the local credit value
 *   DNX_TMC_FAP_CREDIT_VALUE_REMOTE - use the local credit value
 */
uint32
  jer2_jer_mgmt_change_all_faps_credit_worth_unsafe(
    DNX_SAND_IN  int    unit,
    DNX_SAND_OUT uint8     credit_value_to_use
  )
{
    uint32 res, reg_val;
    uint32 credit_worth_local = 0, credit_worth_remote = 0;
    uint32 data = 0;
    soc_mem_t mem;
    DNX_SAND_INIT_ERROR_DEFINITIONS(0);

    
    mem =  SOC_IS_JERICHO_PLUS_A0(unit) ? IPST_CRVSm : IPS_CRVSm;

    DNX_SAND_SOC_IF_ERROR_RETURN(res, 100, exit, READ_IPS_IPS_CREDIT_CONFIGr(unit, &reg_val));
    credit_worth_local = soc_reg_field_get(unit, IPS_IPS_CREDIT_CONFIGr, reg_val, CREDIT_VALUE_0f);
    credit_worth_remote = soc_reg_field_get(unit, IPS_IPS_CREDIT_CONFIGr, reg_val, CREDIT_VALUE_1f);
    if (credit_worth_local != credit_worth_remote) {
        DNX_SAND_SET_ERROR_CODE(DNX_SAND_GEN_ERR, 110, exit);
    } else if (credit_value_to_use == DNX_TMC_FAP_CREDIT_VALUE_LOCAL) {
        soc_mem_field32_set(unit, mem, &data, CR_VAL_BMPf, 0); 
        res = jer2_arad_fill_table_with_entry(unit, mem, MEM_BLOCK_ANY, &data); 
        DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 115, exit);
    } else if (credit_value_to_use == DNX_TMC_FAP_CREDIT_VALUE_REMOTE) {
        soc_mem_field32_set(unit, mem, &data, CR_VAL_BMPf, 0xffff); 
        res = jer2_arad_fill_table_with_entry(unit, mem, MEM_BLOCK_ANY, &data); 
        DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 120, exit);
    } else {
        DNX_SAND_SET_ERROR_CODE(DNX_SAND_GEN_ERR, 125, exit);
    }
exit:
    DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_jer_mgmt_change_all_faps_credit_worth_unsafe()", unit, credit_value_to_use);
}









/*********************************************************************
* Set the fabric system ID of the device. Must be unique in the system.
*********************************************************************/
uint32 jer2_jer_mgmt_system_fap_id_set(
    DNX_SAND_IN  int     unit,
    DNX_SAND_IN  uint32  sys_fap_id
  )
{
    uint32 reg32, group_ctx, fap_id_plus_1 = sys_fap_id + 1;
    uint64 reg64;
    int is_mesh, offset, i,is_single_core;
    SHR_BITDCLNAME (dqcq_map, 32);
    DNXC_INIT_FUNC_DEFS;
    is_single_core = SOC_DNX_CORE_MODE_IS_SINGLE_CORE(unit);
    if (sys_fap_id >= JER2_ARAD_NOF_FAPS_IN_SYSTEM || (!is_single_core && (sys_fap_id % SOC_DNX_DEFS_GET(unit, nof_cores)))) {
        DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("FAP ID %u is illegal, must be a multiple if %u and under %u."),
          sys_fap_id, SOC_DNX_DEFS_GET(unit, nof_cores), JER2_ARAD_NOF_FAPS_IN_SYSTEM));
    }

    /* configure the IDs of all cores, and configure traffic to local cores not to go through the fabric */
    DNXC_IF_ERR_EXIT(READ_ECI_GLOBAL_GENERAL_CFG_2r(unit, &reg32));
    soc_reg_field_set(unit, ECI_GLOBAL_GENERAL_CFG_2r, &reg32, DEV_ID_0f, sys_fap_id);
    if (!is_single_core) {
        soc_reg_field_set(unit, ECI_GLOBAL_GENERAL_CFG_2r, &reg32, DEV_ID_1f, fap_id_plus_1);
    }
    soc_reg_field_set(unit, ECI_GLOBAL_GENERAL_CFG_2r, &reg32, FORCE_FABRICf, 0);
    DNXC_IF_ERR_EXIT(WRITE_ECI_GLOBAL_GENERAL_CFG_2r(unit, reg32));

    if (SOC_DNX_CONFIG(unit)->tdm.is_bypass &&
        SOC_DNX_CONFIG(unit)->jer2_arad->init.fabric.is_128_in_system &&
        SOC_DNX_CONFIG(unit)->tm.is_petrab_in_system) {
        DNXC_IF_ERR_EXIT(soc_reg_above_64_field32_modify(unit, FDT_TDM_CONFIGURATIONr,
          REG_PORT_ANY, 0, TDM_SOURCE_FAP_IDf,
          sys_fap_id + SOC_DNX_CONFIG(unit)->jer2_arad->tdm_source_fap_id_offset));
    }
    /* set DQCQ map according to system configurations*/
    is_mesh = (SOC_DNX_CONFIG(unit)->jer2_arad->init.fabric.connect_mode == DNX_TMC_FABRIC_CONNECT_MODE_MESH ||
               SOC_DNX_CONFIG(unit)->jer2_arad->init.fabric.connect_mode == DNX_TMC_FABRIC_CONNECT_MODE_SINGLE_FAP);
    if (is_mesh) { /*config dqcq map with group contexts for local destinations */
        for (group_ctx = 0, i = sys_fap_id; i < sys_fap_id + SOC_DNX_CONFIG(unit)->core_mode.nof_active_cores; ++group_ctx, ++i) {
            offset = (i % 16) * SOC_JER2_JER_FABRIC_GROUP_CTX_LENGTH; /*isolate bits 0:3*/
            if (offset < SOC_JER2_JER_FABRIC_STK_FAP_GROUP_SIZE) {
                DNXC_IF_ERR_EXIT(READ_ECI_GLOBAL_DQCQ_MAP_CFG_1r(unit, &reg64)); 
                *dqcq_map = soc_reg64_field32_get(unit,ECI_GLOBAL_DQCQ_MAP_CFG_1r, reg64, DEV_ID_TO_DQCQ_MAP_LSBf);
                SHR_BITCOPY_RANGE(dqcq_map, offset, &group_ctx, 0, SOC_JER2_JER_FABRIC_GROUP_CTX_LENGTH);
                soc_reg64_field32_set(unit, ECI_GLOBAL_DQCQ_MAP_CFG_1r, &reg64, DEV_ID_TO_DQCQ_MAP_LSBf, *dqcq_map);
                DNXC_IF_ERR_EXIT(WRITE_ECI_GLOBAL_DQCQ_MAP_CFG_1r(unit, reg64));
            } else {
                offset -= SOC_JER2_JER_FABRIC_STK_FAP_GROUP_SIZE;
                DNXC_IF_ERR_EXIT(READ_ECI_GLOBAL_DQCQ_MAP_CFG_2r(unit, &reg64)); 
                *dqcq_map = soc_reg64_field32_get(unit,ECI_GLOBAL_DQCQ_MAP_CFG_2r, reg64, DEV_ID_TO_DQCQ_MAP_MSBf);
                SHR_BITCOPY_RANGE(dqcq_map, offset, &group_ctx, 0, SOC_JER2_JER_FABRIC_GROUP_CTX_LENGTH);
                soc_reg64_field32_set(unit, ECI_GLOBAL_DQCQ_MAP_CFG_2r, &reg64, DEV_ID_TO_DQCQ_MAP_MSBf, *dqcq_map);
                DNXC_IF_ERR_EXIT(WRITE_ECI_GLOBAL_DQCQ_MAP_CFG_2r(unit, reg64));
            }
        }
    } else { /*configure dqcq map with fap-id's of all cores*/
        COMPILER_64_ZERO(reg64);
        soc_reg64_field32_set(unit, ECI_GLOBAL_DQCQ_MAP_CFG_1r, &reg64, DEV_ID_TO_DQCQ_MAP_LSBf, sys_fap_id | (sys_fap_id << JER2_JER_FAPID_BIT_OFFSET_IN_DQCQ_MAP));
        soc_reg64_field32_set(unit, ECI_GLOBAL_DQCQ_MAP_CFG_1r, &reg64, DEV_ID_TO_DQCQ_MASKf, 0);
        DNXC_IF_ERR_EXIT(WRITE_ECI_GLOBAL_DQCQ_MAP_CFG_1r(unit, reg64));
        COMPILER_64_ZERO(reg64);
        if (!is_single_core) {
            soc_reg64_field32_set(unit, ECI_GLOBAL_DQCQ_MAP_CFG_2r, &reg64, DEV_ID_TO_DQCQ_MAP_MSBf, fap_id_plus_1 | (fap_id_plus_1 << JER2_JER_FAPID_BIT_OFFSET_IN_DQCQ_MAP));
            soc_reg64_field32_set(unit, ECI_GLOBAL_DQCQ_MAP_CFG_2r, &reg64, DEV_ID_TO_DQCQ_MASK_1f, 0);
            DNXC_IF_ERR_EXIT(WRITE_ECI_GLOBAL_DQCQ_MAP_CFG_2r(unit, reg64));
        }
    }
exit:
    DNXC_FUNC_RETURN;
}


/*
 * get the FAP ID of the (core 0 of the) device
 */
uint32
  jer2_jer_mgmt_system_fap_id_get(
    DNX_SAND_IN  int       unit,
    DNX_SAND_OUT uint32    *sys_fap_id
  )
{
  uint32  fld_val = 0;

  DNXC_INIT_FUNC_DEFS;
  DNXC_NULL_CHECK(sys_fap_id);


  /* in Jericho the device ID is represented by the ID of core 0 */

  DNXC_IF_ERR_EXIT(soc_reg_above_64_field32_read(unit, ECI_GLOBAL_GENERAL_CFG_2r, REG_PORT_ANY, 0, DEV_ID_0f, &fld_val));
  *sys_fap_id = fld_val;

exit:
    DNXC_FUNC_RETURN;
}

uint32
  soc_jer2_jer_init_ctrl_cells_enable_set(
    DNX_SAND_IN  int  unit,
    DNX_SAND_IN  uint8  enable,
    DNX_SAND_IN  uint32  flags

  )
{
    DNXC_INIT_FUNC_DEFS;

/*exit:*/
    DNXC_FUNC_RETURN;
}

/*
 * Note: This procedure is based on soc_fe3200_drv_temperature_monitor_get(). 
 * Apparently, they both use the same silicon piece.
 */
int 
   jer2_jer_mgmt_temp_pvt_get(int unit, int temperature_max, soc_switch_temperature_monitor_t *temperature_array, int *temperature_count)
{
    int ii;
    uint32 reg32_val;
    int peak, curr;
    soc_reg_t temp_reg[] = {ECI_PVT_MON_A_THERMAL_DATAr, ECI_PVT_MON_B_THERMAL_DATAr, ECI_PVT_MON_C_THERMAL_DATAr, ECI_PVT_MON_D_THERMAL_DATAr};
    soc_field_t curr_field[] = {THERMAL_DATA_Af, THERMAL_DATA_Bf, THERMAL_DATA_Cf, THERMAL_DATA_Df};
    soc_field_t peak_field[] = {PEAK_THERMAL_DATA_Af, PEAK_THERMAL_DATA_Bf, PEAK_THERMAL_DATA_Cf, PEAK_THERMAL_DATA_Df};

    DNXC_INIT_FUNC_DEFS;

    if (temperature_max < _SOC_JER2_JER_PVT_MON_NOF)
    {
        DNXC_EXIT_WITH_ERR(SOC_E_FULL, (_BSL_DNXC_MSG("Array size should be equal to or larger than %d.\n"), _SOC_JER2_JER_PVT_MON_NOF));
    }

    for (ii = 0; ii < _SOC_JER2_JER_PVT_MON_NOF; ii++)
    {
        DNXC_IF_ERR_EXIT(soc_reg32_get(unit, temp_reg[ii], REG_PORT_ANY, 0, &reg32_val));

        curr = soc_reg_field_get(unit, temp_reg[ii], reg32_val, curr_field[ii]);
        /*curr [0.1 C] = 4120.5 - curr * 4.9103*/
        temperature_array[ii].curr =  (_SOC_JER2_JER_PVT_BASE - curr * _SOC_JER2_JER_PVT_FACTOR) / 10000;

        peak = soc_reg_field_get(unit, temp_reg[ii], reg32_val, peak_field[ii]);
        /*peak [0.1 C] = 4120.5 - peak * 4.9103*/
        temperature_array[ii].peak = (_SOC_JER2_JER_PVT_BASE - peak * _SOC_JER2_JER_PVT_FACTOR) / 10000;
    }

    *temperature_count = _SOC_JER2_JER_PVT_MON_NOF;

exit:
    DNXC_FUNC_RETURN; 
}

/* 
 * PVT
 */

/*
 * Note: This procedure is based on soc_fe3200_drv_pvt_monitor_enable(). 
 * Apparently, they both use the same silicon piece.
 */
int jer2_jer_mgmt_drv_pvt_monitor_enable(int unit)
{
    uint64 reg64_val;
    soc_reg_t pvt_monitors[] = {ECI_PVT_MON_A_CONTROL_REGr, ECI_PVT_MON_B_CONTROL_REGr, ECI_PVT_MON_C_CONTROL_REGr, ECI_PVT_MON_D_CONTROL_REGr, ECI_PVT_MON_CONTROL_REGr};
    int pvt_index;

    DNXC_INIT_FUNC_DEFS;

    /* Init */
    COMPILER_64_ZERO(reg64_val);
    for (pvt_index = 0; pvt_index < (sizeof(pvt_monitors) / sizeof(soc_reg_t)); pvt_index++) {
        if (!SOC_REG_IS_VALID(unit, pvt_monitors[pvt_index])) continue;
        DNXC_IF_ERR_EXIT(soc_reg_set(unit, pvt_monitors[pvt_index], REG_PORT_ANY, 0, reg64_val));
    }

    /* Powerdown */
    COMPILER_64_BITSET(reg64_val, _SOC_JER2_JER_ECI_PVT_MON_CONTROL_REG_POWERDOWN_BIT);
    for (pvt_index = 0; pvt_index < (sizeof(pvt_monitors) / sizeof(soc_reg_t)); pvt_index++) {
        if (!SOC_REG_IS_VALID(unit, pvt_monitors[pvt_index])) continue;
        DNXC_IF_ERR_EXIT(soc_reg_set(unit, pvt_monitors[pvt_index], REG_PORT_ANY, 0, reg64_val));
    }

    /* Powerup */
    COMPILER_64_ZERO(reg64_val);
    for (pvt_index = 0; pvt_index < (sizeof(pvt_monitors) / sizeof(soc_reg_t)); pvt_index++) {
        if (!SOC_REG_IS_VALID(unit, pvt_monitors[pvt_index])) continue;
        DNXC_IF_ERR_EXIT(soc_reg_set(unit, pvt_monitors[pvt_index], REG_PORT_ANY, 0, reg64_val));
    }

    /* Reset */
    COMPILER_64_BITSET(reg64_val, _SOC_JER2_JER_ECI_PVT_MON_CONTROL_REG_RESET_BIT);
    for (pvt_index = 0; pvt_index < (sizeof(pvt_monitors) / sizeof(soc_reg_t)); pvt_index++) {
        if (!SOC_REG_IS_VALID(unit, pvt_monitors[pvt_index])) continue;
        DNXC_IF_ERR_EXIT(soc_reg_set(unit, pvt_monitors[pvt_index], REG_PORT_ANY, 0, reg64_val));
    }

exit:
    DNXC_FUNC_RETURN;
}

/*********************************************************************
*     Enable / Disable the device from receiving and
*     transmitting traffic.
*********************************************************************/
uint32
  jer2_jer_mgmt_enable_traffic_set(
    DNX_SAND_IN  int unit,
    DNX_SAND_IN  uint8 enable
  )
{
    uint32 res;

    DNX_SAND_INIT_ERROR_DEFINITIONS(0);
    DNXC_PCID_LITE_SKIP(unit);

    DNX_SAND_CHECK_DRIVER_AND_DEVICE;
    DNX_SAND_TAKE_DEVICE_SEMAPHORE;

    res = jer2_jer_mgmt_enable_traffic_set_unsafe(unit, enable);
    DNX_SAND_CHECK_FUNC_RESULT(res, 100, exit_semaphore);

exit_semaphore:
    DNX_SAND_GIVE_DEVICE_SEMAPHORE;
exit:
    DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_jer_mgmt_enable_traffic_set()",unit,enable);
}

uint32
  jer2_jer_mgmt_enable_traffic_get(
    DNX_SAND_IN  int unit,
    DNX_SAND_OUT  uint8 *enable
  )
{
    uint32 res;

    DNX_SAND_INIT_ERROR_DEFINITIONS(0);

    DNX_SAND_CHECK_DRIVER_AND_DEVICE;
    DNX_SAND_TAKE_DEVICE_SEMAPHORE;

    res = jer2_jer_mgmt_enable_traffic_get_unsafe(unit, enable);
    DNX_SAND_CHECK_FUNC_RESULT(res, 100, exit_semaphore);

exit_semaphore:
    DNX_SAND_GIVE_DEVICE_SEMAPHORE;
exit:
    DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_jer_mgmt_enable_traffic_get()",unit,enable);
}

/*********************************************************************
*     Enable / Disable the device from receiving and
*     transmitting traffic.
*     Details: in the H file. (search for prototype)
*********************************************************************/
uint32 jer2_jer_mgmt_enable_traffic_set_unsafe(
    DNX_SAND_IN  int  unit,
    DNX_SAND_IN  uint8  enable)
{
    uint32 core_for_write, core_for_read, data;

    DNXC_INIT_FUNC_DEFS;

    /*
     * if in single core mode, use core '0'. Otherwise, read from
     * core '0' and write to all cores. This procedure is currently symmetric.
     */
    core_for_read = 0 ;
    core_for_write = SOC_DNX_CONFIG(unit)->core_mode.nof_active_cores == 1 ? 0 : SOC_CORE_ALL;

    if (enable == FALSE) {

        /*IRE disable*/
        DNXC_IF_ERR_EXIT(READ_IRE_DYNAMIC_CONFIGURATIONr(unit, &data));
        soc_reg_field_set(unit, IRE_DYNAMIC_CONFIGURATIONr, &data, ENABLE_DATA_PATHf, 0x0);
        DNXC_IF_ERR_EXIT(WRITE_IRE_DYNAMIC_CONFIGURATIONr(unit, data));

        if (SOC_IS_QAX(unit)) {
            DNXC_IF_ERR_EXIT(WRITE_CGM_ENABLERSr(unit, 0x00));

            DNXC_IF_ERR_EXIT(READ_SPB_DYNAMIC_CONFIGURATIONr(unit, &data));
            soc_reg_field_set(unit, SPB_DYNAMIC_CONFIGURATIONr, &data, ENABLE_DATA_PATHf, 0x0);
            DNXC_IF_ERR_EXIT(WRITE_SPB_DYNAMIC_CONFIGURATIONr(unit, data));
        } else {
                /*IDR disable*/
            DNXC_IF_ERR_EXIT(READ_IDR_DYNAMIC_CONFIGURATIONr(unit, &data));
            soc_reg_field_set(unit, IDR_DYNAMIC_CONFIGURATIONr, &data, ENABLE_DATA_PATHf, 0x0);
            DNXC_IF_ERR_EXIT(WRITE_IDR_DYNAMIC_CONFIGURATIONr(unit, data));

                /*IRR disable*/
            DNXC_IF_ERR_EXIT(READ_IRR_DYNAMIC_CONFIGURATIONr(unit, &data));
            soc_reg_field_set(unit, IRR_DYNAMIC_CONFIGURATIONr, &data, ENABLE_DATA_PATH_IDR_0f, 0x0);
            soc_reg_field_set(unit, IRR_DYNAMIC_CONFIGURATIONr, &data, ENABLE_DATA_PATH_IDR_1f, 0x0);
            soc_reg_field_set(unit, IRR_DYNAMIC_CONFIGURATIONr, &data, ENABLE_DATA_PATH_IQM_0f, 0x0);
            soc_reg_field_set(unit, IRR_DYNAMIC_CONFIGURATIONr, &data, ENABLE_DATA_PATH_IQM_1f, 0x0);
            soc_reg_field_set(unit, IRR_DYNAMIC_CONFIGURATIONr, &data, ENABLE_DATA_PATH_0f, 0x0);
            soc_reg_field_set(unit, IRR_DYNAMIC_CONFIGURATIONr, &data, ENABLE_DATA_PATH_1f, 0x0);
            DNXC_IF_ERR_EXIT(WRITE_IRR_DYNAMIC_CONFIGURATIONr(unit, data));
        }
        /*EGQ disable */
        if (SOC_IS_QUX(unit)) {
            DNXC_IF_ERR_EXIT(READ_EGQ_GENERAL_RQP_CONFIGr(unit, core_for_read, &data));
            soc_reg_field_set(unit, EGQ_GENERAL_RQP_CONFIGr, &data, DBG_FDA_STOPf, 0x1);
            DNXC_IF_ERR_EXIT(WRITE_EGQ_GENERAL_RQP_CONFIGr(unit, core_for_write, data));
        } else {
            DNXC_IF_ERR_EXIT(READ_EGQ_GENERAL_RQP_DEBUG_CONFIGr(unit, core_for_read, &data));
            soc_reg_field_set(unit, EGQ_GENERAL_RQP_DEBUG_CONFIGr, &data, DBG_FDA_STOPf, 0x1);
            DNXC_IF_ERR_EXIT(WRITE_EGQ_GENERAL_RQP_DEBUG_CONFIGr(unit, core_for_write, data));
        }
                /*
                 *  Stop credit reception from the fabric
                 */
        DNXC_IF_ERR_EXIT(READ_SCH_SCHEDULER_CONFIGURATION_REGISTERr(unit, core_for_read, &data));
        soc_reg_field_set(unit, SCH_SCHEDULER_CONFIGURATION_REGISTERr, &data, DISABLE_FABRIC_MSGSf, 0x1);
        DNXC_IF_ERR_EXIT(WRITE_SCH_SCHEDULER_CONFIGURATION_REGISTERr(unit, core_for_write, data));
                /*
                 *  Stop credit generation
                 */
        DNXC_IF_ERR_EXIT(READ_SCH_DVS_CONFIGr(unit, core_for_read, &data));
        soc_reg_field_set(unit, SCH_DVS_CONFIGr, &data, FORCE_PAUSEf, 0x1);
        DNXC_IF_ERR_EXIT(WRITE_SCH_DVS_CONFIGr(unit, core_for_write, data));
    } else {
    	
        if (SOC_IS_QUX(unit)) {
            /*EGQ enable */
            DNXC_IF_ERR_EXIT(READ_EGQ_GENERAL_RQP_CONFIGr(unit, core_for_read, &data));
            soc_reg_field_set(unit, EGQ_GENERAL_RQP_CONFIGr, &data, DBG_FDA_STOPf, 0x0);
            DNXC_IF_ERR_EXIT(WRITE_EGQ_GENERAL_RQP_CONFIGr(unit,  core_for_write, data));
        } else {
            /*EGQ enable */
            DNXC_IF_ERR_EXIT(READ_EGQ_GENERAL_RQP_DEBUG_CONFIGr(unit, core_for_read, &data));
            soc_reg_field_set(unit, EGQ_GENERAL_RQP_DEBUG_CONFIGr, &data, DBG_FDA_STOPf, 0x0);
            DNXC_IF_ERR_EXIT(WRITE_EGQ_GENERAL_RQP_DEBUG_CONFIGr(unit,  core_for_write, data));
        }
        if (SOC_IS_QAX(unit)) {
            DNXC_IF_ERR_EXIT(READ_SPB_DYNAMIC_CONFIGURATIONr(unit, &data));
            soc_reg_field_set(unit, SPB_DYNAMIC_CONFIGURATIONr, &data, ENABLE_DATA_PATHf, 0x1);
            DNXC_IF_ERR_EXIT(WRITE_SPB_DYNAMIC_CONFIGURATIONr(unit, data));

            DNXC_IF_ERR_EXIT(WRITE_CGM_ENABLERSr(unit, 0xff));
        } else {
                    /*IRR enable*/
            DNXC_IF_ERR_EXIT(READ_IRR_DYNAMIC_CONFIGURATIONr(unit, &data));
            soc_reg_field_set(unit, IRR_DYNAMIC_CONFIGURATIONr, &data, ENABLE_DATA_PATH_IDR_0f, 0x1);
            soc_reg_field_set(unit, IRR_DYNAMIC_CONFIGURATIONr, &data, ENABLE_DATA_PATH_IDR_1f, 0x1);
            soc_reg_field_set(unit, IRR_DYNAMIC_CONFIGURATIONr, &data, ENABLE_DATA_PATH_IQM_0f, 0x1);
            soc_reg_field_set(unit, IRR_DYNAMIC_CONFIGURATIONr, &data, ENABLE_DATA_PATH_IQM_1f, 0x1);
            soc_reg_field_set(unit, IRR_DYNAMIC_CONFIGURATIONr, &data, ENABLE_DATA_PATH_0f, 0x1);
            soc_reg_field_set(unit, IRR_DYNAMIC_CONFIGURATIONr, &data, ENABLE_DATA_PATH_1f, 0x1);
            DNXC_IF_ERR_EXIT(WRITE_IRR_DYNAMIC_CONFIGURATIONr(unit, data));

                        /*IDR enable*/
            DNXC_IF_ERR_EXIT(READ_IDR_DYNAMIC_CONFIGURATIONr(unit, &data));
            soc_reg_field_set(unit, IDR_DYNAMIC_CONFIGURATIONr, &data, ENABLE_DATA_PATHf, 0x1);
            DNXC_IF_ERR_EXIT(WRITE_IDR_DYNAMIC_CONFIGURATIONr(unit, data));
        }

                /*IRE enable*/
        DNXC_IF_ERR_EXIT(READ_IRE_DYNAMIC_CONFIGURATIONr(unit, &data));
        soc_reg_field_set(unit, IRE_DYNAMIC_CONFIGURATIONr, &data, ENABLE_DATA_PATHf, 0x1);
        DNXC_IF_ERR_EXIT(WRITE_IRE_DYNAMIC_CONFIGURATIONr(unit, data));
                /*
                 *  Start credit reception from the fabric
                 */
        DNXC_IF_ERR_EXIT(READ_SCH_SCHEDULER_CONFIGURATION_REGISTERr(unit, core_for_read, &data));
        soc_reg_field_set(unit, SCH_SCHEDULER_CONFIGURATION_REGISTERr, &data, DISABLE_FABRIC_MSGSf, 0x0);
        DNXC_IF_ERR_EXIT(WRITE_SCH_SCHEDULER_CONFIGURATION_REGISTERr(unit, core_for_write, data));
                /*
                 *  Start credit generation
                 */
        DNXC_IF_ERR_EXIT(READ_SCH_DVS_CONFIGr(unit, core_for_read, &data));
        soc_reg_field_set(unit, SCH_DVS_CONFIGr, &data, FORCE_PAUSEf, 0x0);
        DNXC_IF_ERR_EXIT(WRITE_SCH_DVS_CONFIGr(unit, core_for_write, data));
    }

exit:
    DNXC_FUNC_RETURN;
}


uint32 jer2_jer_mgmt_enable_traffic_get_unsafe(
    DNX_SAND_IN  int  unit,
    DNX_SAND_OUT  uint8  *enable)
{
    uint32 reg_val32;
    uint8  enable_curr;

    DNXC_INIT_FUNC_DEFS;

    /*checking if IRE is enabled, if so traffic is enabled, otherwise disabled*/
    DNXC_IF_ERR_EXIT(READ_IRE_DYNAMIC_CONFIGURATIONr(unit, &reg_val32));
    enable_curr = soc_reg_field_get(unit, IRE_DYNAMIC_CONFIGURATIONr, reg_val32, ENABLE_DATA_PATHf);
    enable_curr = DNX_SAND_NUM2BOOL(enable_curr);

    *enable = enable_curr;
exit:
    DNXC_FUNC_RETURN;
}


/*********************************************************************
 * Set the MTU (maximal allowed packet size) for any packet,
 * according to the buffer size.
 *********************************************************************/
uint32 jer2_jer_mgmt_set_mru_by_dbuff_size(
    DNX_SAND_IN  int     unit
  )
{
    uint32 mru = -1;
    soc_mem_t mem = SOC_IS_QAX(unit) ? SPB_CONTEXT_MRUm : IDR_CONTEXT_MRUm;
    uint32 entry[2] = {0};

    DNXC_INIT_FUNC_DEFS;

    
    DNXC_LEGACY_FIXME_ASSERT;
#ifdef FIXME_DNX_LEGACY
    SOC_DNX_CONFIG(unit)->jer2_arad->init.dram.dbuff_size * JER2_ARAD_MGMT_MAX_BUFFERS_PER_PACKET;
#endif 

    if (mru > JER2_ARAD_MGMT_PCKT_SIZE_BYTES_VSC_MAX) {
         mru = JER2_ARAD_MGMT_PCKT_SIZE_BYTES_VSC_MAX;
    }

    soc_mem_field32_set(unit, mem, entry, MAX_ORG_SIZEf, mru);
    soc_mem_field32_set(unit, mem, entry, MAX_SIZEf, mru);
    soc_mem_field32_set(unit, mem, entry, MIN_ORG_SIZEf, 0x20);
    soc_mem_field32_set(unit, mem, entry, MIN_SIZEf, 0x20);

    DNXC_SAND_IF_ERR_EXIT(jer2_arad_fill_table_with_entry(unit, mem, MEM_BLOCK_ANY, entry));

exit:
    DNXC_FUNC_RETURN;
}


/*
 * NAME:
 *   jer2_jer_mgmt_dma_fifo_channel_free_find
 * DESCRIPTION: Find a free dma fifo channel to use.
 *          It is allowed to use only channels that is part of the ones that connected to PCI (CPU)
 * INPUT:
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
 */
int jer2_jer_mgmt_dma_fifo_channel_free_find(DNX_SAND_IN int unit, DNX_SAND_IN uint8 skip_pci_cmc, DNX_SAND_OUT int * channel_number)
{
    int i, rv = SOC_E_NONE;    
    uint64 fifo_dma_sel_data;
    uint32 selected_applicaion;
    CONST soc_field_t field_name[NOF_FIFO_DMA_CHANNELS] = {FIFO_DMA_0_SELf, FIFO_DMA_1_SELf, FIFO_DMA_2_SELf, FIFO_DMA_3_SELf, 
                                                                        FIFO_DMA_4_SELf, FIFO_DMA_5_SELf, FIFO_DMA_6_SELf, FIFO_DMA_7_SELf, 
                                                                        FIFO_DMA_8_SELf, FIFO_DMA_9_SELf, FIFO_DMA_10_SELf, FIFO_DMA_11_SELf};
    DNXC_INIT_FUNC_DEFS;

    * channel_number = -1;
    
    /* CMC FIFO DMA Channel source FIFO selection. 
        Possible sources are: 
        0x0 - CRPS 0-3 ,
        0x1 - CRPS 4-7 ,
        0x2 - CRPS 8-11 ,
        0x3 - CRPS 12-15 ,
        0x4 - OAMP Status ,
        0x5 - OAMP Event ,
        0x6 - OLP ,
        0x7 - PPDB CPU REPLY.*/
     rv = READ_ECI_FIFO_DMA_SELr(unit, &fifo_dma_sel_data);
     DNXC_IF_ERR_EXIT(rv);  

    /* if feature cmicm_multi_dma_cmc disable, it is not allowed to have more than one PCI CMC (CPU CMC)*/
    if (!soc_feature(unit, soc_feature_cmicm_multi_dma_cmc) && SOC_PCI_CMCS_NUM(unit) > 1)
    {
         LOG_ERROR(BSL_LS_SOC_STAT,(BSL_META_U(unit, "PCI_CMC_NUM=%d, not allowed when soc_feature_cmicm_multi_dma_cmc disabled \n"), SOC_PCI_CMCS_NUM(unit)));
         DNXC_IF_ERR_EXIT(SOC_E_PARAM);  
    }
    /* run over each field of the register that belong to PCI (CPU) and find free channel */
    /* currently, there is only one parameter that hold the PCI CMC, althought if SOC_PCI_CMCS_NUM>1, there should be more.  */
    /* we assume that the next PCI CMCs will be consecutive to the PCI CMC. (default value for PCI_CMC=0) */
    for(i = SOC_PCI_CMC(unit)*NOF_DMA_FIFO_PER_CMC; i < (SOC_PCI_CMC(unit) + SOC_PCI_CMCS_NUM(unit))*NOF_DMA_FIFO_PER_CMC ;i++)
    {        
        if (i < NOF_FIFO_DMA_CHANNELS)
        {
            selected_applicaion = soc_reg64_field32_get(unit, ECI_FIFO_DMA_SELr, fifo_dma_sel_data, field_name[i]);
            if(selected_applicaion == dnx_dma_fifo_channel_src_reserved)
            {
                if(skip_pci_cmc == FALSE)
                {
                    *channel_number = i;
                    break;                
                }
                else /* force the selected channel not to be in the PCI_CMC */
                {
                    if(i >= (SOC_PCI_CMC(unit)*NOF_DMA_FIFO_PER_CMC + NOF_DMA_FIFO_PER_CMC))
                    {
                        *channel_number = i;
                        break;  
                    }
                }
            }
        }
        else
        {
            LOG_ERROR(BSL_LS_SOC_STAT,(BSL_META_U(unit, "FIFO DMA index out of range. \n")));
            DNXC_IF_ERR_EXIT(SOC_E_PARAM);                            
        }
    }    

exit:
    DNXC_FUNC_RETURN;   
}


/*
 * NAME:
 *   jer2_jer_mgmt_dma_fifo_channel_set
 * DESCRIPTION: set DMA-channel to mapping for a specific application
 * INPUT:
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
 */
int jer2_jer_mgmt_dma_fifo_channel_set(DNX_SAND_IN int unit, DNX_SAND_IN int channel, DNX_SAND_IN dnx_dma_fifo_channel_src_t value) 
{
    int rv = SOC_E_NONE;
    uint64 reg64, field64, val64;
    CONST soc_field_t field_name[NOF_FIFO_DMA_CHANNELS] = {FIFO_DMA_0_SELf, FIFO_DMA_1_SELf, FIFO_DMA_2_SELf, FIFO_DMA_3_SELf, 
                                                                        FIFO_DMA_4_SELf, FIFO_DMA_5_SELf, FIFO_DMA_6_SELf, FIFO_DMA_7_SELf, 
                                                                        FIFO_DMA_8_SELf, FIFO_DMA_9_SELf, FIFO_DMA_10_SELf, FIFO_DMA_11_SELf};
    dnx_dma_fifo_channel_src_t prev_channel_src_val;
    DNXC_INIT_FUNC_DEFS;
   
    if(channel > NOF_FIFO_DMA_CHANNELS)
    {
        LOG_ERROR(BSL_LS_SOC_STAT,(BSL_META_U(unit, "FIFO DMA channel out of range. \n")));
        DNXC_IF_ERR_EXIT(SOC_E_PARAM);       
    }
    rv = READ_ECI_FIFO_DMA_SELr(unit, &reg64);
    DNXC_IF_ERR_EXIT(rv);
    /* get the value store in the HW for the channel before the cahnge */
    val64 = soc_reg64_field_get(unit, ECI_FIFO_DMA_SELr, reg64, field_name[channel]);
    prev_channel_src_val = (dnx_dma_fifo_channel_src_t)COMPILER_64_LO(val64);
    COMPILER_64_SET(field64,0,value);
    soc_reg64_field_set(unit, ECI_FIFO_DMA_SELr, &reg64, field_name[channel], field64);

    rv = WRITE_ECI_FIFO_DMA_SELr(unit, reg64);
    DNXC_IF_ERR_EXIT(rv);  

    /* update DB */
    if(value == dnx_dma_fifo_channel_src_reserved)
    {
        /* if already was reserved, no need to update DB */
        if (prev_channel_src_val < dnx_dma_fifo_channel_src_max)
        {
            jer2_jer_mgmt_dma_fifo_source_channels_db[unit].dma_fifo_source_channels_array[prev_channel_src_val] = -1;
        }
    }
    else
    {
        if (value < dnx_dma_fifo_channel_src_max)
        {
            jer2_jer_mgmt_dma_fifo_source_channels_db[unit].dma_fifo_source_channels_array[value] = channel;
        }
        else
        {
            LOG_ERROR(BSL_LS_SOC_STAT,(BSL_META_U(unit, "value=%d. out of range \n"), value));
            DNXC_IF_ERR_EXIT(SOC_E_PARAM);                 
        }
    }

exit:
    DNXC_FUNC_RETURN;
}

/*
 * NAME:
 *   jer2_jer_mgmt_dma_fifo_channel_get
 * DESCRIPTION: get DMA-channel that belong for specific application from DB
 * INPUT:
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
 */
int jer2_jer_mgmt_dma_fifo_channel_get (DNX_SAND_IN int unit, DNX_SAND_IN dnx_dma_fifo_channel_src_t source, DNX_SAND_OUT int* channel)
{    
    DNXC_INIT_FUNC_DEFS;

    if (source < dnx_dma_fifo_channel_src_max)
    {
        *channel = jer2_jer_mgmt_dma_fifo_source_channels_db[unit].dma_fifo_source_channels_array[source];
    }
    else
    {
        LOG_ERROR(BSL_LS_SOC_STAT,(BSL_META_U(unit, "source=%d. out of range \n"), source));
        DNXC_IF_ERR_EXIT(SOC_E_PARAM);     
    }

exit:
    DNXC_FUNC_RETURN;    
}


/*
 * NAME:
 *   jer2_jer_mgmt_dma_fifo_source_channels_db_init
 * DESCRIPTION: init the DMA FIFO source channels DB according to HW during init sequence (also in WB mode)
 * INPUT: int unit
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
 */
int jer2_jer_mgmt_dma_fifo_source_channels_db_init (int unit)
{    
    int rv = SOC_E_NONE, i;
    uint64 reg64, val64;
    CONST soc_field_t field_name[NOF_FIFO_DMA_CHANNELS] = {FIFO_DMA_0_SELf, FIFO_DMA_1_SELf, FIFO_DMA_2_SELf, FIFO_DMA_3_SELf, 
                                                                        FIFO_DMA_4_SELf, FIFO_DMA_5_SELf, FIFO_DMA_6_SELf, FIFO_DMA_7_SELf, 
                                                                        FIFO_DMA_8_SELf, FIFO_DMA_9_SELf, FIFO_DMA_10_SELf, FIFO_DMA_11_SELf};
    dnx_dma_fifo_channel_src_t channel;
    DNXC_INIT_FUNC_DEFS;


    rv = READ_ECI_FIFO_DMA_SELr(unit, &reg64);
    DNXC_IF_ERR_EXIT(rv);
    /* init all DB with -1 */
    for(i=0; i < dnx_dma_fifo_channel_src_max; i++)
    {
        jer2_jer_mgmt_dma_fifo_source_channels_db[unit].dma_fifo_source_channels_array[i] = -1;
    }
    
    /* get the value store in the HW for each channel  */
    for(i=0; i < NOF_FIFO_DMA_CHANNELS; i++)
    {
        val64 = soc_reg64_field_get(unit, ECI_FIFO_DMA_SELr, reg64, field_name[i]);
        channel = (dnx_dma_fifo_channel_src_t)COMPILER_64_LO(val64);  
        if(channel != dnx_dma_fifo_channel_src_reserved)
        {
            jer2_jer_mgmt_dma_fifo_source_channels_db[unit].dma_fifo_source_channels_array[channel] = i;           
        }
    }
    
exit:
    DNXC_FUNC_RETURN;    
}



/*
 * Get if queue is OCB eligible.
 */
int
  jer2_jer_mgmt_voq_is_ocb_eligible_get(
    DNX_SAND_IN  int         unit,
    DNX_SAND_IN  int         core_id,
    DNX_SAND_IN  uint32      qid,
    DNX_SAND_OUT uint32      *is_ocb_eligible
  )
{
    uint32 line = 0, bit_in_line = 0, eligibility[1] = {0};
    uint64 reg64_val;

    DNXC_INIT_FUNC_DEFS;

    DNXC_NULL_CHECK(is_ocb_eligible); 
    if ((core_id < 0) || (core_id > SOC_DNX_CONFIG(unit)->core_mode.nof_active_cores)) { 
        DNXC_IF_ERR_EXIT(SOC_E_PARAM); 
    } 

    COMPILER_64_ZERO(reg64_val);

    if (!SOC_IS_QAX(unit)) {  
        /* calculating the queue bit in the table */
        line = qid / 32;
        bit_in_line = qid % 32;

        /* get ocb eligiblity */
        DNXC_IF_ERR_EXIT(READ_IDR_QUEUE_IS_OCB_ELIGIBLEm(unit, core_id, IDR_BLOCK(unit), line, &reg64_val));
        eligibility[0] = soc_mem_field32_get(unit, IDR_QUEUE_IS_OCB_ELIGIBLEm, &reg64_val, ELIGIBILITYf);
        *is_ocb_eligible = SHR_BITGET(eligibility, bit_in_line);
    }
exit:
    DNXC_FUNC_RETURN;    
}

/*
* Get the AVS - Adjustable Voltage Scaling value of the device
*/
int jer2_jer_mgmt_avs_value_get(
            int       unit,
            uint32*      avs_val)
{
    uint32
        reg_val;

    DNXC_INIT_FUNC_DEFS;

    DNXC_NULL_CHECK(avs_val);

    *avs_val = 0;

    DNXC_IF_ERR_EXIT(soc_reg32_get(unit, ECI_ROV_STATUSr, REG_PORT_ANY, 0, &reg_val));
    *avs_val = soc_reg_field_get(unit, ECI_ROV_STATUSr, reg_val, AVS_STATUSf);

exit:
    DNXC_FUNC_RETURN;
}


/*
 * Set if to forward packets whose mirror/snoop copies are dropped in ingress sue to FIFO being full.
 */
int jer2_jer_mgmt_mirror_snoop_forward_original_when_dropped_set(
    DNX_SAND_IN  int         unit,
    DNX_SAND_IN  uint8       enabled
  )
{
    uint32 value;

    DNXC_INIT_FUNC_DEFS;

    DNXC_IF_ERR_EXIT(READ_IRR_STATIC_CONFIGURATIONr(unit, &value));
    soc_reg_field_set(unit, IRR_STATIC_CONFIGURATIONr, &value, FWD_VALID_EN_ON_SNOOP_MIRROR_DROPf, enabled ? 1 : 0);
    DNXC_IF_ERR_EXIT(WRITE_IRR_STATIC_CONFIGURATIONr(unit, value));
exit:
    DNXC_FUNC_RETURN;    
}


/*
 * Get if to forward packets whose mirror/snoop copies are dropped in ingress sue to FIFO being full.
 */
int jer2_jer_mgmt_mirror_snoop_forward_original_when_dropped_get(
    DNX_SAND_IN  int         unit,
    DNX_SAND_OUT uint8       *enabled
  )
{
    uint32 value;

    DNXC_INIT_FUNC_DEFS;
    DNXC_NULL_CHECK(enabled);

    DNXC_IF_ERR_EXIT(READ_IRR_STATIC_CONFIGURATIONr(unit, &value));
    *enabled = soc_reg_field_get(unit, IRR_STATIC_CONFIGURATIONr, value, FWD_VALID_EN_ON_SNOOP_MIRROR_DROPf);

exit:
    DNXC_FUNC_RETURN;    
}


#undef _ERR_MSG_MODULE_NAME

