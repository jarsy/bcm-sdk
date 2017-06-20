/*
 * $Id: jer2_jer_reset.c, v1 21/09/2014 09:55:39 azarrin $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 */

/*************
 * INCLUDES  *
 *************/

/* SOC dnxc includes */
#include <soc/dnxc/legacy/error.h>
#include <soc/dnxc/legacy/dnxc_cmic.h>

/* SOC DNX includes */
#include <soc/dnx/legacy/drv.h>
#include <soc/dnx/legacy/mbcm.h>
#include <soc/dnx/legacy/ARAD/arad_mgmt.h>
#include <soc/dnx/legacy/ARAD/arad_chip_regs.h>
#include <soc/dnx/legacy/JER/jer_init.h>
#include <soc/dnx/legacy/JER/jer_fabric.h>
#include <soc/dnx/legacy/JER/jer_reset.h>
#include <soc/dnx/legacy/JER/jer_defs.h>
#include <soc/dnx/legacy/JER/jer_mgmt.h>
#include <soc/dnx/legacy/JER/jer_regs.h>
#include <shared/swstate/sw_state.h>

/*************
 * DEFINES   *
 *************/
#ifdef _ERR_MSG_MODULE_NAME 
    #error "_ERR_MSG_MODULE_NAME redefined" 
#endif
#define _ERR_MSG_MODULE_NAME BSL_SOC_INIT

/*************
 * TYPE DEFS *
 *************/


/*************
 * FUNCTIONS *
 *************/
int soc_jer2_jer_reset_blocks_poll_init_finish(int unit)
{
    int i;

    DNXC_INIT_FUNC_DEFS;
    
    for(i = 0; i < SOC_DNX_CONFIG(unit)->core_mode.nof_active_cores; i++) {
        /* ING */
        DNXC_IF_ERR_EXIT(soc_dnx_polling(unit, JER2_ARAD_TIMEOUT, JER2_ARAD_MIN_POLLS, IRE_RESET_STATUS_REGISTERr, i, 0, CTXT_STATUS_INIT_DONEf, 0x1));
        if (!SOC_IS_QAX(unit)) { 
            DNXC_IF_ERR_EXIT(soc_dnx_polling(unit, JER2_ARAD_TIMEOUT, JER2_ARAD_MIN_POLLS, IRE_RESET_STATUS_REGISTERr, i, 0, CTXT_MAP_INIT_DONEf, 0x1));

            DNXC_IF_ERR_EXIT(soc_dnx_polling(unit, JER2_ARAD_TIMEOUT, JER2_ARAD_MIN_POLLS, IDR_RESET_STATUS_REGISTERr, i, 0, CONTEXT_STATUS_INIT_DONEf, 0x1));
            DNXC_IF_ERR_EXIT(soc_dnx_polling(unit, JER2_ARAD_TIMEOUT, JER2_ARAD_MIN_POLLS, IDR_RESET_STATUS_REGISTERr, i, 0, CHUNK_STATUS_INIT_DONEf, 0x1));
            DNXC_IF_ERR_EXIT(soc_dnx_polling(unit, JER2_ARAD_TIMEOUT, JER2_ARAD_MIN_POLLS, IDR_RESET_STATUS_REGISTERr, i, 0, WORD_INDEX_FIFO_INIT_DONEf, 0x1));
            DNXC_IF_ERR_EXIT(soc_dnx_polling(unit, JER2_ARAD_TIMEOUT, JER2_ARAD_MIN_POLLS, IDR_RESET_STATUS_REGISTERr, i, 0, FREE_PCB_FIFO_INIT_DONEf, 0x1));

            DNXC_IF_ERR_EXIT(soc_dnx_polling(unit, JER2_ARAD_TIMEOUT, JER2_ARAD_MIN_POLLS, IRR_RESET_STATUS_REGISTERr, i, 0, FPF_0_INIT_DONEf, 0x1));
            DNXC_IF_ERR_EXIT(soc_dnx_polling(unit, JER2_ARAD_TIMEOUT, JER2_ARAD_MIN_POLLS, IRR_RESET_STATUS_REGISTERr, i, 0, FPF_1_INIT_DONEf, 0x1));
            DNXC_IF_ERR_EXIT(soc_dnx_polling(unit, JER2_ARAD_TIMEOUT, JER2_ARAD_MIN_POLLS, IRR_RESET_STATUS_REGISTERr, i, 0, IS_FPF_0_INIT_DONEf, 0x1));
            DNXC_IF_ERR_EXIT(soc_dnx_polling(unit, JER2_ARAD_TIMEOUT, JER2_ARAD_MIN_POLLS, IRR_RESET_STATUS_REGISTERr, i, 0, IS_FPF_1_INIT_DONEf, 0x1));
            DNXC_IF_ERR_EXIT(soc_dnx_polling(unit, JER2_ARAD_TIMEOUT, JER2_ARAD_MIN_POLLS, IRR_RESET_STATUS_REGISTERr, i, 0, DESTINATION_TABLE_INIT_DONEf, 0x1));
            DNXC_IF_ERR_EXIT(soc_dnx_polling(unit, JER2_ARAD_TIMEOUT, JER2_ARAD_MIN_POLLS, IRR_RESET_STATUS_REGISTERr, i, 0, LAG_MAPPING_INIT_DONEf, 0x1));
            DNXC_IF_ERR_EXIT(soc_dnx_polling(unit, JER2_ARAD_TIMEOUT, JER2_ARAD_MIN_POLLS, IRR_RESET_STATUS_REGISTERr, i, 0, LAG_TO_LAG_RANGE_INIT_DONEf, 0x1));
            DNXC_IF_ERR_EXIT(soc_dnx_polling(unit, JER2_ARAD_TIMEOUT, JER2_ARAD_MIN_POLLS, IRR_RESET_STATUS_REGISTERr, i, 0, MCDB_INIT_DONEf, 0x1));
            DNXC_IF_ERR_EXIT(soc_dnx_polling(unit, JER2_ARAD_TIMEOUT, JER2_ARAD_MIN_POLLS, IRR_RESET_STATUS_REGISTERr, i, 0, STACK_FEC_RESOLVE_INIT_DONEf, 0x1));
            DNXC_IF_ERR_EXIT(soc_dnx_polling(unit, JER2_ARAD_TIMEOUT, JER2_ARAD_MIN_POLLS, IRR_RESET_STATUS_REGISTERr, i, 0, STACK_TRUNK_RESOLVE_INIT_DONEf, 0x1));

            DNXC_IF_ERR_EXIT(soc_dnx_polling(unit, JER2_ARAD_TIMEOUT, JER2_ARAD_MIN_POLLS, IQM_IQM_INITr, i, 0, IQC_INITf, 0x0));
            DNXC_IF_ERR_EXIT(soc_dnx_polling(unit, JER2_ARAD_TIMEOUT, JER2_ARAD_MIN_POLLS, IQM_IQM_INITr, i, 0, STE_INITf, 0x0));
            DNXC_IF_ERR_EXIT(soc_dnx_polling(unit, JER2_ARAD_TIMEOUT, JER2_ARAD_MIN_POLLS, IQM_IQM_INITr, i, 0, PDM_INITf, 0x0));

            DNXC_IF_ERR_EXIT(soc_dnx_polling(unit, JER2_ARAD_TIMEOUT, JER2_ARAD_MIN_POLLS, MRPS_INIT_SEQ_ONr, i, 0, MCDA_INIT_SEQ_ONf, 0x1));
            DNXC_IF_ERR_EXIT(soc_dnx_polling(unit, JER2_ARAD_TIMEOUT, JER2_ARAD_MIN_POLLS, MTRPS_EM_INIT_SEQ_ONr, i, 0, MCDA_INIT_SEQ_ONf, 0x1));
            DNXC_IF_ERR_EXIT(soc_dnx_polling(unit, JER2_ARAD_TIMEOUT, JER2_ARAD_MIN_POLLS, IHB_ISEM_RESET_STATUS_REGISTERr, i, 0, ISEM_KEYT_RESET_DONEf, 0x1));
        }

        DNXC_IF_ERR_EXIT(soc_dnx_polling(unit, JER2_ARAD_TIMEOUT, JER2_ARAD_MIN_POLLS, IPS_IPS_GENERAL_CONFIGURATIONSr, i, 0, IPS_INIT_TRIGGERf, 0x0));
        
        /* EGR */
        DNXC_IF_ERR_EXIT(soc_dnx_polling(unit, JER2_ARAD_TIMEOUT, JER2_ARAD_MIN_POLLS, EGQ_EGQ_BLOCK_INIT_STATUSr, i, 0, EGQ_BLOCK_INITf, 0x0));
    }
    
    if (!SOC_IS_FLAIR(unit)) {
        /* ING */
        DNXC_IF_ERR_EXIT(soc_dnx_polling(unit, JER2_ARAD_TIMEOUT, JER2_ARAD_MIN_POLLS, PPDB_A_OEMA_RESET_STATUS_REGISTERr, REG_PORT_ANY, 0, OEMA_KEYT_RESET_DONEf, 0x1));
        DNXC_IF_ERR_EXIT(soc_dnx_polling(unit, JER2_ARAD_TIMEOUT, JER2_ARAD_MIN_POLLS, PPDB_A_OEMB_RESET_STATUS_REGISTERr, REG_PORT_ANY, 0, OEMB_KEYT_RESET_DONEf, 0x1));

        DNXC_IF_ERR_EXIT(soc_dnx_polling(unit, JER2_ARAD_TIMEOUT, JER2_ARAD_MIN_POLLS, PPDB_B_LARGE_EM_RESET_STATUS_REGISTERr, REG_PORT_ANY, 0, LARGE_EM_KEYT_RESET_DONEf, 0x1));

        if (!SOC_IS_JERICHO_PLUS_A0(unit)) {
            DNXC_IF_ERR_EXIT(soc_dnx_polling(unit, JER2_ARAD_TIMEOUT, JER2_ARAD_MIN_POLLS, EDB_GLEM_RESET_STATUS_REGISTERr, REG_PORT_ANY, 0, GLEM_KEYT_RESET_DONEf, 0x1)); 
            DNXC_IF_ERR_EXIT(soc_dnx_polling(unit, JER2_ARAD_TIMEOUT, JER2_ARAD_MIN_POLLS, EDB_ESEM_RESET_STATUS_REGISTERr, REG_PORT_ANY, 0, ESEM_KEYT_RESET_DONEf, 0x1));
        }

        DNXC_IF_ERR_EXIT(soc_dnx_polling(unit, JER2_ARAD_TIMEOUT, JER2_ARAD_MIN_POLLS, OAMP_REMOTE_MEP_EXACT_MATCH_RESET_STATUS_REGISTERr, REG_PORT_ANY, 0, REMOTE_MEP_EXACT_MATCH_KEYT_RESET_DONEf, 0x1));
    }

exit:
    DNXC_FUNC_RETURN;  
}



/*********************************************************************
 *     Resets the end-to-end scheduler. The reset is performed
 *     by clearing the internal scheduler pipes, and then
 *     performing soft-reset.
 *     Details: in the H file. (search for prototype)
 *********************************************************************/
static int jer2_jer_sch_reset(int unit)
{
    uint32
        mc_conf_0_fld_val,
        mc_conf_1_fld_val,
        ingr_shp_en_fld_val, 
        timeout_val,
        backup_msg_en,
        dlm_enable;
    uint32
        tbl_data[JER2_ARAD_SCH_SCHEDULER_INIT_TBL_ENTRY_SIZE] = {0};
    int sch_nof_instances = 0;
    int i;
    int block_type;

    DNXC_INIT_FUNC_DEFS;

    block_type = SOC_BLK_SCH;
    DNXC_IF_ERR_EXIT(MBCM_DNX_DRIVER_CALL(unit, mbcm_dnx_nof_block_instance, (unit, &block_type, &sch_nof_instances)));
    for (i = 0; i < sch_nof_instances; i++) {

/*        DNXC_IF_ERR_EXIT(soc_reg_above_64_field32_modify(unit, SCH_DVS_CONFIG_1r, i, 0, FORCE_PAUSEf,  0x1));*/
        DNXC_IF_ERR_EXIT(soc_reg_above_64_field32_modify(unit, SCH_SCHEDULER_CONFIGURATION_REGISTERr, i, 0, DISABLE_FABRIC_MSGSf,  0x1));

        DNXC_IF_ERR_EXIT(READ_SCH_SCH_FABRIC_MULTICAST_PORT_CONFIGURATION_REGISTER_0r(unit, i, &mc_conf_0_fld_val));
        DNXC_IF_ERR_EXIT(READ_SCH_SCH_FABRIC_MULTICAST_PORT_CONFIGURATION_REGISTER_1r(unit, i, &mc_conf_1_fld_val));
        DNXC_IF_ERR_EXIT(soc_reg_above_64_field32_modify(unit, SCH_SCH_FABRIC_MULTICAST_PORT_CONFIGURATION_REGISTER_0r, i, 0, MULTICAST_GFMC_ENABLEf,  0x0));
        DNXC_IF_ERR_EXIT(soc_reg_above_64_field32_modify(unit, SCH_SCH_FABRIC_MULTICAST_PORT_CONFIGURATION_REGISTER_0r, i, 0, MULTICAST_BFMC_1_ENABLEf,  0x0));
        DNXC_IF_ERR_EXIT(soc_reg_above_64_field32_modify(unit, SCH_SCH_FABRIC_MULTICAST_PORT_CONFIGURATION_REGISTER_1r, i, 0, MULTICAST_BFMC_2_ENABLEf,  0x0));
        DNXC_IF_ERR_EXIT(soc_reg_above_64_field32_modify(unit, SCH_SCH_FABRIC_MULTICAST_PORT_CONFIGURATION_REGISTER_1r, i, 0, MULTICAST_BFMC_3_ENABLEf,  0x0));

        DNXC_IF_ERR_EXIT(soc_reg_above_64_field32_read(unit, SCH_INGRESS_SHAPING_PORT_CONFIGURATIONr, i, 0, INGRESS_SHAPING_ENABLEf, &ingr_shp_en_fld_val));
        DNXC_IF_ERR_EXIT(soc_reg_above_64_field32_modify(unit, SCH_INGRESS_SHAPING_PORT_CONFIGURATIONr, i, 0, INGRESS_SHAPING_ENABLEf,  0x0));

        DNXC_IF_ERR_EXIT(soc_reg_above_64_field32_read(unit, SCH_DELETE_MECHANISM_CONFIGURATION_REGISTERr, i, 0, DLM_ENABLEf, &dlm_enable));
        DNXC_IF_ERR_EXIT(soc_reg_above_64_field32_modify(unit, SCH_DELETE_MECHANISM_CONFIGURATION_REGISTERr, i, 0, DLM_ENABLEf,  0x0));

        DNXC_IF_ERR_EXIT(soc_reg_above_64_field32_read(unit, SCH_SMP_BACK_UP_MESSAGESr, i, 0, BACKUP_MSG_ENABLEf, &backup_msg_en));
        DNXC_IF_ERR_EXIT(soc_reg_above_64_field32_modify(unit, SCH_SMP_BACK_UP_MESSAGESr, i, 0, BACKUP_MSG_ENABLEf,  0x0));

        /* clear on set to all the sch memory error interrupts */  
        DNXC_IF_ERR_EXIT(WRITE_SCH_ECC_INTERRUPT_REGISTERr(unit, i, 0xffffffff));

        soc_mem_field32_set(unit, SCH_SCHEDULER_INITm, tbl_data, SCH_INITf, 0x0);

        /* keep current timeout and set a new timeout needed for writing this special table */
        DNXC_IF_ERR_EXIT(READ_CMIC_SBUS_TIMEOUTr(unit, &timeout_val));
        DNXC_IF_ERR_EXIT(WRITE_CMIC_SBUS_TIMEOUTr(unit, 0xffffffff));

        /* write the table entry */
        DNXC_IF_ERR_EXIT(WRITE_SCH_SCHEDULER_INITm(unit, i, 0x0, tbl_data));

        /* restore original timeout */
        DNXC_IF_ERR_EXIT(WRITE_CMIC_SBUS_TIMEOUTr(unit, timeout_val));

        /*
        * Recover original configuration
        */
        
        /*DNXC_IF_ERR_EXIT(soc_reg_above_64_field32_modify(unit, SCH_DVS_CONFIG_1r, i, 0, FORCE_PAUSEf,  0x0));*/
        DNXC_IF_ERR_EXIT(soc_reg_above_64_field32_modify(unit, SCH_SCHEDULER_CONFIGURATION_REGISTERr, i, 0, DISABLE_FABRIC_MSGSf,  0x0));

        DNXC_IF_ERR_EXIT(WRITE_SCH_SCH_FABRIC_MULTICAST_PORT_CONFIGURATION_REGISTER_0r(unit, i,  mc_conf_0_fld_val));
        DNXC_IF_ERR_EXIT(WRITE_SCH_SCH_FABRIC_MULTICAST_PORT_CONFIGURATION_REGISTER_1r(unit, i,  mc_conf_1_fld_val));

        DNXC_IF_ERR_EXIT(soc_reg_above_64_field32_modify(unit, SCH_INGRESS_SHAPING_PORT_CONFIGURATIONr, i, 0, INGRESS_SHAPING_ENABLEf,  ingr_shp_en_fld_val));
        DNXC_IF_ERR_EXIT(soc_reg_above_64_field32_modify(unit, SCH_DELETE_MECHANISM_CONFIGURATION_REGISTERr, i, 0, DLM_ENABLEf,  dlm_enable));
        DNXC_IF_ERR_EXIT(soc_reg_above_64_field32_modify(unit, SCH_SMP_BACK_UP_MESSAGESr, i, 0, BACKUP_MSG_ENABLEf,  backup_msg_en));
    }

exit:
    DNXC_FUNC_RETURN;
}

int soc_jer2_jer_reset_nif_txi_oor(int unit) 
{
    soc_reg_above_64_val_t reg_above64_val;

    DNXC_INIT_FUNC_DEFS;

    SOC_REG_ABOVE_64_CREATE_MASK(reg_above64_val, 128, 0);
    DNXC_IF_ERR_EXIT(WRITE_EGQ_INIT_FQP_TXI_NIFr(unit, SOC_CORE_ALL, reg_above64_val));

    sal_usleep(1);

    SOC_REG_ABOVE_64_CLEAR(reg_above64_val);
    DNXC_IF_ERR_EXIT(WRITE_EGQ_INIT_FQP_TXI_NIFr(unit, SOC_CORE_ALL, reg_above64_val));

exit:
    DNXC_FUNC_RETURN;
}

static int jer2_jer_soft_reset(
    DNX_SAND_IN  int                    unit,
    DNX_SAND_IN  JER2_ARAD_DBG_RST_DOMAIN    rst_domain)
{
    int                     rv;
    int                     jer2_jer_ingress_field_list_size; 
    int                     jer2_jer_plus_ingress_field_list_size; 
    int                     jer2_qax_ingress_field_list_size; 
    int                     jer2_jer_egress_field_list_size; 
    int                     jer2_jer_plus_egress_field_list_size; 
    int                     jer2_qax_egress_field_list_size;
    int                     field_index;
    int                     ingress_field_list_size;
    int                     egress_field_list_size;
    uint32                  reg_val;
    uint32                  res = DNX_SAND_OK;
    uint64                  reg64;
    uint8                   is_traffic_enabled_orig;
    uint8                   is_ingr;
    uint8                   is_egr;
    uint8                   is_ctrl_cells_enabled_orig;
    uint8                   is_fabric = 0;
    uint8                   is_schan_locked = 0;
    soc_reg_above_64_val_t  soft_init_reg_val;
    soc_reg_above_64_val_t  soft_init_reg_val_orig;
    soc_reg_above_64_val_t  fld_above64_val;
    soc_reg_above_64_val_t  reg_above64_val;
    soc_reg_above_64_val_t  queue_is_disable_orig[SOC_DNX_DEFS_MAX(NOF_CORES)];
    soc_reg_above_64_val_t  queue_is_disable;

    soc_field_t             *ingress_field_list;
    soc_field_t             *egress_field_list;
    int core;


    soc_field_t jer2_jer_ingress_field_list[] = {BLOCKS_SOFT_INIT_1f,  BLOCKS_SOFT_INIT_2f,  BLOCKS_SOFT_INIT_3f,  BLOCKS_SOFT_INIT_4f,  BLOCKS_SOFT_INIT_5f,  BLOCKS_SOFT_INIT_6f,  BLOCKS_SOFT_INIT_8f,  BLOCKS_SOFT_INIT_9f, 
                                            BLOCKS_SOFT_INIT_10f, BLOCKS_SOFT_INIT_19f, BLOCKS_SOFT_INIT_20f, BLOCKS_SOFT_INIT_22f, BLOCKS_SOFT_INIT_23f, BLOCKS_SOFT_INIT_24f, BLOCKS_SOFT_INIT_25f, BLOCKS_SOFT_INIT_26f, 
                                            BLOCKS_SOFT_INIT_55f, BLOCKS_SOFT_INIT_56f, BLOCKS_SOFT_INIT_57f, BLOCKS_SOFT_INIT_58f, BLOCKS_SOFT_INIT_59f, BLOCKS_SOFT_INIT_60f, BLOCKS_SOFT_INIT_61f, BLOCKS_SOFT_INIT_62f, 
                                            BLOCKS_SOFT_INIT_63f, BLOCKS_SOFT_INIT_64f};
    soc_field_t jer2_jer_plus_ingress_field_list[] = {BLOCKS_SOFT_INIT_1f,  BLOCKS_SOFT_INIT_2f,  BLOCKS_SOFT_INIT_3f,  BLOCKS_SOFT_INIT_4f,  BLOCKS_SOFT_INIT_5f,  BLOCKS_SOFT_INIT_6f,  BLOCKS_SOFT_INIT_8f,  BLOCKS_SOFT_INIT_9f, 
                                                 BLOCKS_SOFT_INIT_10f, BLOCKS_SOFT_INIT_19f, BLOCKS_SOFT_INIT_20f, BLOCKS_SOFT_INIT_22f, BLOCKS_SOFT_INIT_23f, BLOCKS_SOFT_INIT_24f, BLOCKS_SOFT_INIT_25f, BLOCKS_SOFT_INIT_26f, 
                                                 BLOCKS_SOFT_INIT_59f, BLOCKS_SOFT_INIT_60f, BLOCKS_SOFT_INIT_61f, BLOCKS_SOFT_INIT_62f, BLOCKS_SOFT_INIT_63f, BLOCKS_SOFT_INIT_64f, BLOCKS_SOFT_INIT_65f, BLOCKS_SOFT_INIT_66f, 
                                                 BLOCKS_SOFT_INIT_70f, BLOCKS_SOFT_INIT_71f};
    soc_field_t jer2_qax_ingress_field_list[] = {BLOCKS_SOFT_INIT_23f,  BLOCKS_SOFT_INIT_1f,   BLOCKS_SOFT_INIT_2f,   BLOCKS_SOFT_INIT_9f,  BLOCKS_SOFT_INIT_22f, BLOCKS_SOFT_INIT_24f,  BLOCKS_SOFT_INIT_25f,  BLOCKS_SOFT_INIT_55f, 
                                            BLOCKS_SOFT_INIT_56f,  BLOCKS_SOFT_INIT_57f,  BLOCKS_SOFT_INIT_58f,  BLOCKS_SOFT_INIT_60f, BLOCKS_SOFT_INIT_63f, BLOCKS_SOFT_INIT_64f,  BLOCKS_SOFT_INIT_108f, BLOCKS_SOFT_INIT_10f, 
                                            BLOCKS_SOFT_INIT_19f,  BLOCKS_SOFT_INIT_3f,   BLOCKS_SOFT_INIT_4f,   BLOCKS_SOFT_INIT_29f, BLOCKS_SOFT_INIT_30f, BLOCKS_SOFT_INIT_34f,  BLOCKS_SOFT_INIT_35f,  BLOCKS_SOFT_INIT_36f, 
                                            BLOCKS_SOFT_INIT_37f,  BLOCKS_SOFT_INIT_59f,  BLOCKS_SOFT_INIT_62f,  BLOCKS_SOFT_INIT_65f, BLOCKS_SOFT_INIT_66f, BLOCKS_SOFT_INIT_107f, BLOCKS_SOFT_INIT_109f, BLOCKS_SOFT_INIT_110f, 
                                            BLOCKS_SOFT_INIT_111f, BLOCKS_SOFT_INIT_114f, BLOCKS_SOFT_INIT_115f, BLOCKS_SOFT_INIT_116f};
    soc_field_t jer2_jer_egress_field_list[] = { BLOCKS_SOFT_INIT_33f, BLOCKS_SOFT_INIT_104f, BLOCKS_SOFT_INIT_105f, BLOCKS_SOFT_INIT_106f, BLOCKS_SOFT_INIT_50f, BLOCKS_SOFT_INIT_51f, 
                                            BLOCKS_SOFT_INIT_55f, BLOCKS_SOFT_INIT_56f,  BLOCKS_SOFT_INIT_57f};
    soc_field_t jer2_jer_plus_egress_field_list[] = {BLOCKS_SOFT_INIT_33f, BLOCKS_SOFT_INIT_111f, BLOCKS_SOFT_INIT_112f, BLOCKS_SOFT_INIT_113f, BLOCKS_SOFT_INIT_54f, BLOCKS_SOFT_INIT_55f, 
                                                BLOCKS_SOFT_INIT_59f, BLOCKS_SOFT_INIT_60f,  BLOCKS_SOFT_INIT_61f};
    soc_field_t jer2_qax_egress_field_list[] = { BLOCKS_SOFT_INIT_33f, BLOCKS_SOFT_INIT_97f, BLOCKS_SOFT_INIT_98f, BLOCKS_SOFT_INIT_99f, BLOCKS_SOFT_INIT_20f, BLOCKS_SOFT_INIT_55f, 
                                            BLOCKS_SOFT_INIT_56f, BLOCKS_SOFT_INIT_57f, BLOCKS_SOFT_INIT_53f};


    DNXC_INIT_FUNC_DEFS;

    jer2_jer_ingress_field_list_size         = sizeof(jer2_jer_ingress_field_list)        / sizeof(jer2_jer_ingress_field_list[0]);
    jer2_jer_plus_ingress_field_list_size    = sizeof(jer2_jer_plus_ingress_field_list)   / sizeof(jer2_jer_plus_ingress_field_list[0]);
    jer2_qax_ingress_field_list_size         = sizeof(jer2_qax_ingress_field_list)        / sizeof(jer2_qax_ingress_field_list[0]);
    jer2_jer_egress_field_list_size          = sizeof(jer2_jer_egress_field_list)         / sizeof(jer2_jer_egress_field_list[0]);
    jer2_jer_plus_egress_field_list_size     = sizeof(jer2_jer_plus_egress_field_list)    / sizeof(jer2_jer_plus_egress_field_list[0]);
    jer2_qax_egress_field_list_size          = sizeof(jer2_qax_egress_field_list)         / sizeof(jer2_qax_egress_field_list[0]);

    if (SOC_IS_JERICHO_PLUS(unit)) 
    {
        ingress_field_list      = jer2_jer_plus_ingress_field_list;
        ingress_field_list_size = jer2_jer_plus_ingress_field_list_size;
        egress_field_list       = jer2_jer_plus_egress_field_list;
        egress_field_list_size  = jer2_jer_plus_egress_field_list_size;
    } 
    else if (SOC_IS_QAX(unit)) 
    {
        ingress_field_list      = jer2_qax_ingress_field_list;
        ingress_field_list_size = jer2_qax_ingress_field_list_size;
        egress_field_list       = jer2_qax_egress_field_list;
        egress_field_list_size  = jer2_qax_egress_field_list_size;
    } 
    else 
    {
        ingress_field_list      = jer2_jer_ingress_field_list;
        ingress_field_list_size = jer2_jer_ingress_field_list_size;
        egress_field_list       = jer2_jer_egress_field_list;
        egress_field_list_size  = jer2_jer_egress_field_list_size;
    }

    is_ingr = (rst_domain == JER2_ARAD_DBG_RST_DOMAIN_INGR) || (rst_domain == JER2_ARAD_DBG_RST_DOMAIN_INGR_AND_FABRIC) || (rst_domain == JER2_ARAD_DBG_RST_DOMAIN_FULL) || (rst_domain == JER2_ARAD_DBG_RST_DOMAIN_FULL_AND_FABRIC);
    is_egr  = (rst_domain == JER2_ARAD_DBG_RST_DOMAIN_EGR)  || (rst_domain == JER2_ARAD_DBG_RST_DOMAIN_EGR_AND_FABRIC)  || (rst_domain == JER2_ARAD_DBG_RST_DOMAIN_FULL) || (rst_domain == JER2_ARAD_DBG_RST_DOMAIN_FULL_AND_FABRIC);
    is_fabric = (rst_domain == JER2_ARAD_DBG_RST_DOMAIN_INGR_AND_FABRIC) || (rst_domain == JER2_ARAD_DBG_RST_DOMAIN_EGR_AND_FABRIC) || (rst_domain == JER2_ARAD_DBG_RST_DOMAIN_FULL_AND_FABRIC);

    LOG_VERBOSE(BSL_LS_SOC_INIT, (BSL_META_U(unit, "%s(): Start. is_ingr=%d, is_egr=%d, is_fabric=%d\n"), FUNCTION_NAME(), is_ingr, is_egr, is_fabric));

    /************************************************************************/
    /** Validate Data Path is clean for cpu queue                          **/
    /************************************************************************/
    if (is_egr) {
       for (core = 0 ; core < SOC_DNX_DEFS_GET(unit, nof_cores) ; core++) {
           DNXC_IF_ERR_EXIT(READ_CGM_CGM_QUEUE_IS_DISABLEDr(unit, core, queue_is_disable_orig[core]));
       }
       SOC_REG_ABOVE_64_CREATE_MASK(queue_is_disable, 512, 0);
       DNXC_IF_ERR_EXIT(WRITE_CGM_CGM_QUEUE_IS_DISABLEDr(unit, SOC_CORE_ALL, queue_is_disable));

       sal_usleep(100);
       DNXC_IF_ERR_EXIT(soc_dnx_polling(unit, 100*JER2_ARAD_TIMEOUT, JER2_ARAD_MIN_POLLS, ECI_GP_STATUS_5r, REG_PORT_ANY, 0, PIR_EGQ_0_FIFO_NOT_EMPTY_STICKYf, 0x0));
       sal_usleep(100);
       DNXC_IF_ERR_EXIT(soc_dnx_polling(unit, 100*JER2_ARAD_TIMEOUT, JER2_ARAD_MIN_POLLS, ECI_GP_STATUS_5r, REG_PORT_ANY, 0, PIR_EGQ_1_FIFO_NOT_EMPTY_STICKYf, 0x0));  
    }
    /**************************************************************************/
    /** Disable Traffic                                                      **/
    /**************************************************************************/
    /* Store current traffic-enable-state (just in case: if we got here, it is enabled) */
    res = jer2_jer_mgmt_enable_traffic_get(unit, &is_traffic_enabled_orig);
    DNXC_IF_ERR_EXIT(dnx_handle_sand_result(res));
    LOG_VERBOSE(BSL_LS_SOC_INIT, (BSL_META_U(unit, "%s(): Disable Traffic. is_traffic_enabled_orig=%d\n"), FUNCTION_NAME(), is_traffic_enabled_orig));

    res = jer2_jer_mgmt_enable_traffic_set(unit, FALSE);
    DNXC_IF_ERR_EXIT(dnx_handle_sand_result(res));

    

    if (is_fabric && !soc_feature(unit, soc_feature_no_fabric))
    {
        /* Store current traffic-enable-state (just in case: if we got here, it is enabled) */
        res = jer2_arad_mgmt_all_ctrl_cells_enable_get(unit, &is_ctrl_cells_enabled_orig);
        DNXC_IF_ERR_EXIT(dnx_handle_sand_result(res));

        /* calling unsafe methods is discouraged. in this case we do it because jer2_arad_dbg_dev_reset is always called safely */
        res = jer2_arad_mgmt_all_ctrl_cells_enable_set_unsafe(unit, FALSE, JER2_ARAD_MGMT_ALL_CTRL_CELLS_FLAGS_SOFT_RESET);
        DNXC_IF_ERR_EXIT(dnx_handle_sand_result(res));
    }

    /************************************************************************/
    /** Validate Data Path is clean - active queue = 0                     **/
    /************************************************************************/
     if (is_egr) {
        sal_usleep(100);
        DNXC_IF_ERR_EXIT(soc_dnx_polling(unit, 100*JER2_ARAD_TIMEOUT, JER2_ARAD_MIN_POLLS, CGM_TOTAL_PACKET_DESCRIPTORS_COUNTERr, REG_PORT_ANY, 0, NUMBER_OF_ALLOCATED_PACKET_DESCRIPTORSf, 0x0));
        sal_usleep(100);
    }

    /* This lock is added since threads might access the device during soft reset, causing schan timeout */
    is_schan_locked = 1;
    SCHAN_LOCK(unit);

    /**************************************************************************/
    /** Read original configuration                                          **/
    /**************************************************************************/
    DNXC_IF_ERR_EXIT(READ_ECI_BLOCKS_SOFT_INITr(unit,soft_init_reg_val));
    sal_memcpy(soft_init_reg_val_orig, soft_init_reg_val, sizeof(soc_reg_above_64_val_t));

    LOG_VERBOSE(BSL_LS_SOC_INIT, (BSL_META_U(unit, "%s(): Read original configuration. soft_init_reg_val_orig=0x%x,0x%x,0x%x,0x%x\n"), FUNCTION_NAME(), soft_init_reg_val_orig[0], soft_init_reg_val_orig[1], soft_init_reg_val_orig[2], soft_init_reg_val_orig[3]));
    /**************************************************************************/
    /** IN-RESET                                                             **/
    /**************************************************************************/
    LOG_VERBOSE(BSL_LS_SOC_INIT, (BSL_META_U(unit, "%s(): IN-RESET\n"), FUNCTION_NAME()));

    SOC_REG_ABOVE_64_CLEAR(fld_above64_val);
    SOC_REG_ABOVE_64_CREATE_MASK(fld_above64_val, 1, 0);


    if (is_ingr) 
    {
        
        DNXC_LEGACY_FIXME_ASSERT;
#ifdef FIXME_DNX_LEGACY
        SHR_BIT_ITER(SOC_DNX_CONFIG(unit)->jer2_arad->init.drc_info.dram_bitmap, SOC_DNX_DEFS_GET(unit, hw_dram_interfaces_max), drc_ndx) {*/
            /* need reset PHY_CDR_MONITOR_ENf */
            DNXC_IF_ERR_EXIT(soc_dnx_drc_combo28_cdr_monitor_enable_get(unit, drc_ndx, &enabled));
            if (enabled) {
                SHR_BITSET(drc_monitor_enable_bitmap, drc_ndx);
                DNXC_IF_ERR_EXIT(soc_dnx_drc_combo28_cdr_monitor_enable_set(unit, drc_ndx, 0));
            }
        }
#endif 
        if (!SOC_IS_QAX(unit)) 
        {
            /* IQMT should get reset before the others */
            soc_reg_above_64_field_set(unit, ECI_BLOCKS_SOFT_INITr, soft_init_reg_val, BLOCKS_SOFT_INIT_21f, fld_above64_val);/* IQMT Block ID */
            DNXC_IF_ERR_EXIT(WRITE_ECI_BLOCKS_SOFT_INITr(unit, soft_init_reg_val));
        }

        for (field_index = 0; field_index < ingress_field_list_size; ++field_index)
        {
            soc_reg_above_64_field_set(unit, ECI_BLOCKS_SOFT_INITr, soft_init_reg_val, ingress_field_list[field_index], fld_above64_val);
        }
        if (is_fabric) 
        {
            soc_reg_above_64_field_set(unit, ECI_BLOCKS_SOFT_INITr, soft_init_reg_val, BLOCKS_SOFT_INIT_27f, fld_above64_val);/* FCT Block ID   */
            soc_reg_above_64_field_set(unit, ECI_BLOCKS_SOFT_INITr, soft_init_reg_val, BLOCKS_SOFT_INIT_28f, fld_above64_val);/* FDT Block ID   */
        }
        DNXC_IF_ERR_EXIT(WRITE_ECI_BLOCKS_SOFT_INITr(unit, soft_init_reg_val));

        
        DNXC_LEGACY_FIXME_ASSERT;
#ifdef FIXME_DNX_LEGACY
        DNXC_IF_ERR_EXIT(soc_jer2_jer_dram_init_drc_soft_init(unit, &SOC_DNX_CONFIG(unit)->jer2_arad->init.drc_info, 1));
#endif 

        /* Reseting CMICM TXI credits */ 
        DNXC_IF_ERR_EXIT(WRITE_CMIC_TXBUF_IPINTF_INTERFACE_CREDITSr(unit, 0x40));
        DNXC_IF_ERR_EXIT(WRITE_CMIC_TXBUF_IPINTF_INTERFACE_CREDITSr(unit, 0x0));
    }

    if (is_egr) 
    {
        /* Reset NBI special ports */
		if (!SOC_IS_QUX(unit)) {
            DNXC_IF_ERR_EXIT(READ_NBIH_ADDITIONAL_RESETSr(unit, &reg_val));
            soc_reg_field_set(unit, NBIH_ADDITIONAL_RESETSr, &reg_val, EIF_RSTNf, 0);
            soc_reg_field_set(unit, NBIH_ADDITIONAL_RESETSr, &reg_val, ELK_0_RX_RSTNf, 0);
            soc_reg_field_set(unit, NBIH_ADDITIONAL_RESETSr, &reg_val, ELK_0_TX_RSTNf, 0);
            if (!SOC_IS_QAX(unit)) 
            {
                soc_reg_field_set(unit, NBIH_ADDITIONAL_RESETSr, &reg_val, SIF_RSTNf, 0); 
                soc_reg_field_set(unit, NBIH_ADDITIONAL_RESETSr, &reg_val, ELK_1_RX_RSTNf, 0);
                soc_reg_field_set(unit, NBIH_ADDITIONAL_RESETSr, &reg_val, ELK_1_TX_RSTNf, 0);
            }
            DNXC_IF_ERR_EXIT(WRITE_NBIH_ADDITIONAL_RESETSr(unit,  reg_val));
    
            DNXC_IF_ERR_EXIT(READ_NBIL_ADDITIONAL_RESETSr(unit, 0, &reg_val));
            soc_reg_field_set(unit, NBIL_ADDITIONAL_RESETSr, &reg_val, SIF_RSTNf, 0);
            soc_reg_field_set(unit, NBIL_ADDITIONAL_RESETSr, &reg_val, ELK_0_RX_RSTNf, 0);
            soc_reg_field_set(unit, NBIL_ADDITIONAL_RESETSr, &reg_val, ELK_0_TX_RSTNf, 0);
            if (!SOC_IS_QAX(unit)) 
            {
                soc_reg_field_set(unit, NBIL_ADDITIONAL_RESETSr, &reg_val, ELK_1_RX_RSTNf, 0); 
                soc_reg_field_set(unit, NBIL_ADDITIONAL_RESETSr, &reg_val, ELK_1_TX_RSTNf, 0);
            }
            DNXC_IF_ERR_EXIT(WRITE_NBIL_ADDITIONAL_RESETSr(unit, 0, reg_val));
    
            DNXC_IF_ERR_EXIT(READ_NBIL_ADDITIONAL_RESETSr(unit, 1, &reg_val));
            soc_reg_field_set(unit, NBIL_ADDITIONAL_RESETSr, &reg_val, SIF_RSTNf, 0);
            soc_reg_field_set(unit, NBIL_ADDITIONAL_RESETSr, &reg_val, ELK_0_RX_RSTNf, 0);
            soc_reg_field_set(unit, NBIL_ADDITIONAL_RESETSr, &reg_val, ELK_0_TX_RSTNf, 0);
            if (!SOC_IS_QAX(unit)) 
            {
                soc_reg_field_set(unit, NBIL_ADDITIONAL_RESETSr, &reg_val, ELK_1_RX_RSTNf, 0); 
                soc_reg_field_set(unit, NBIL_ADDITIONAL_RESETSr, &reg_val, ELK_1_TX_RSTNf, 0);
            }
            DNXC_IF_ERR_EXIT(WRITE_NBIL_ADDITIONAL_RESETSr(unit, 1, reg_val));
        } else {
            DNXC_IF_ERR_EXIT(READ_NIF_ADDITIONAL_RESETSr(unit, &reg_val));
            soc_reg_field_set(unit, NIF_ADDITIONAL_RESETSr, &reg_val, EIF_RSTNf, 0);
            soc_reg_field_set(unit, NIF_ADDITIONAL_RESETSr, &reg_val, SIF_RSTNf, 0); 
            DNXC_IF_ERR_EXIT(WRITE_NIF_ADDITIONAL_RESETSr(unit,  reg_val));
        }
        if(!SOC_IS_QAX(unit))
        {
            /* NOTE: EDB must be reset before EGQ/EPNI */
            soc_reg_above_64_field_set(unit, ECI_BLOCKS_SOFT_INITr, soft_init_reg_val, SOC_IS_JERICHO_PLUS_A0(unit) ? BLOCKS_SOFT_INIT_56f : BLOCKS_SOFT_INIT_52f, fld_above64_val);/* EDB */
            DNXC_IF_ERR_EXIT(WRITE_ECI_BLOCKS_SOFT_INITr(unit, soft_init_reg_val));

            /* NOTE: EGQ/EPNI must be reset before NBI */
            soc_reg_above_64_field_set(unit, ECI_BLOCKS_SOFT_INITr, soft_init_reg_val, SOC_IS_JERICHO_PLUS_A0(unit) ? BLOCKS_SOFT_INIT_50f : BLOCKS_SOFT_INIT_46f, fld_above64_val);/* EGQ0 Block ID */
            soc_reg_above_64_field_set(unit, ECI_BLOCKS_SOFT_INITr, soft_init_reg_val, SOC_IS_JERICHO_PLUS_A0(unit) ? BLOCKS_SOFT_INIT_51f : BLOCKS_SOFT_INIT_47f, fld_above64_val);/* EGQ1 Block ID */
            soc_reg_above_64_field_set(unit, ECI_BLOCKS_SOFT_INITr, soft_init_reg_val, SOC_IS_JERICHO_PLUS_A0(unit) ? BLOCKS_SOFT_INIT_52f : BLOCKS_SOFT_INIT_48f, fld_above64_val);/* EPNI0 Block ID */
            soc_reg_above_64_field_set(unit, ECI_BLOCKS_SOFT_INITr, soft_init_reg_val, SOC_IS_JERICHO_PLUS_A0(unit) ? BLOCKS_SOFT_INIT_53f : BLOCKS_SOFT_INIT_49f, fld_above64_val);/* EPNI1 Block ID */
            DNXC_IF_ERR_EXIT(WRITE_ECI_BLOCKS_SOFT_INITr(unit, soft_init_reg_val));
        }

        for (field_index = 0; field_index < egress_field_list_size; ++field_index)
        {
            soc_reg_above_64_field_set(unit, ECI_BLOCKS_SOFT_INITr, soft_init_reg_val, egress_field_list[field_index], fld_above64_val);
        }
        if (is_fabric) 
        {
            if (SOC_IS_JERICHO_PLUS(unit)) 
            {
                DNXC_IF_ERR_EXIT(READ_FDR_FDR_ENABLERS_REGISTER_1r(unit, &reg64));
                soc_reg64_field32_set(unit, FDR_FDR_ENABLERS_REGISTER_1r, &reg64, FIELD_12_12f, 0x1);
                soc_reg64_field32_set(unit, FDR_FDR_ENABLERS_REGISTER_1r, &reg64, FIELD_13_13f, 0x1);
                soc_reg64_field32_set(unit, FDR_FDR_ENABLERS_REGISTER_1r, &reg64, FIELD_14_14f, 0x1);
                DNXC_IF_ERR_EXIT(WRITE_FDR_FDR_ENABLERS_REGISTER_1r(unit, reg64));                
            }
            else
            {             
                DNXC_IF_ERR_EXIT(READ_FDR_FDR_ENABLERS_REGISTER_1r(unit, &reg64));
                soc_reg64_field32_set(unit, FDR_FDR_ENABLERS_REGISTER_1r, &reg64, FIELD_31_31f, 0x1);
                soc_reg64_field32_set(unit, FDR_FDR_ENABLERS_REGISTER_1r, &reg64, FIELD_32_32f, 0x1);
                soc_reg64_field32_set(unit, FDR_FDR_ENABLERS_REGISTER_1r, &reg64, FIELD_33_33f, 0x1);
                DNXC_IF_ERR_EXIT(WRITE_FDR_FDR_ENABLERS_REGISTER_1r(unit, reg64));
            }

            soc_reg_above_64_field_set(unit, ECI_BLOCKS_SOFT_INITr, soft_init_reg_val, BLOCKS_SOFT_INIT_31f, fld_above64_val);/* FCR Block ID   */
            soc_reg_above_64_field_set(unit, ECI_BLOCKS_SOFT_INITr, soft_init_reg_val, BLOCKS_SOFT_INIT_32f, fld_above64_val);/* FDR Block ID   */
        }
        DNXC_IF_ERR_EXIT(WRITE_ECI_BLOCKS_SOFT_INITr(unit, soft_init_reg_val));

        /* Close FCR */
        if (!soc_feature(unit, soc_feature_no_fabric)) {
            DNXC_IF_ERR_EXIT(soc_reg_above_64_field32_modify(unit, FCR_FCR_ENABLERS_AND_FILTER_MATCH_INPUT_LINKr, REG_PORT_ANY, 0, DIS_FL_STSf,  0x1));
            DNXC_IF_ERR_EXIT(soc_reg_above_64_field32_modify(unit, FCR_FCR_ENABLERS_AND_FILTER_MATCH_INPUT_LINKr, REG_PORT_ANY, 0, DIS_CRD_FCRf,  0x1));
            DNXC_IF_ERR_EXIT(soc_reg_above_64_field32_modify(unit, FCR_FCR_ENABLERS_AND_FILTER_MATCH_INPUT_LINKr, REG_PORT_ANY, 0, DIS_SRf,  0x1));
        }
        /* Rest scheduler (in-out) */
        rv = jer2_jer_sch_reset(unit);
        DNXC_IF_ERR_EXIT(rv);
    }

    /**************************************************************************/
    /** OUT-OF-RESET                                                         **/
    /**************************************************************************/
    LOG_VERBOSE(BSL_LS_SOC_INIT, (BSL_META_U(unit, "%s(): OUT-OF-RESET.\n"), FUNCTION_NAME()));

    SOC_REG_ABOVE_64_CLEAR(fld_above64_val);
    if (is_ingr) 
    {
        if (!SOC_IS_QAX(unit)) 
        {
            /* IQMT should get out of reset before the others */
            soc_reg_above_64_field_set(unit, ECI_BLOCKS_SOFT_INITr, soft_init_reg_val, BLOCKS_SOFT_INIT_21f, fld_above64_val);
            DNXC_IF_ERR_EXIT(WRITE_ECI_BLOCKS_SOFT_INITr(unit, soft_init_reg_val));
        }
        for (field_index = 0; field_index < ingress_field_list_size; ++field_index)
        {
            soc_reg_above_64_field_set(unit, ECI_BLOCKS_SOFT_INITr, soft_init_reg_val, ingress_field_list[field_index], fld_above64_val);
        }
        if (is_fabric) 
        {
            soc_reg_above_64_field_set(unit, ECI_BLOCKS_SOFT_INITr, soft_init_reg_val, BLOCKS_SOFT_INIT_27f, fld_above64_val);/* FCT Block ID   */
            soc_reg_above_64_field_set(unit, ECI_BLOCKS_SOFT_INITr, soft_init_reg_val, BLOCKS_SOFT_INIT_28f, fld_above64_val);/* FDT Block ID   */
        }
        DNXC_IF_ERR_EXIT(WRITE_ECI_BLOCKS_SOFT_INITr(unit, soft_init_reg_val));

        /* DPRC - out of soft init*/
        
        DNXC_LEGACY_FIXME_ASSERT;
#ifdef FIXME_DNX_LEGACY
        DNXC_IF_ERR_EXIT(soc_jer2_jer_dram_init_drc_soft_init(unit, &SOC_DNX_CONFIG(unit)->jer2_arad->init.drc_info, 0));
#endif 

        if (SOC_IS_QAX(unit)) 
        {
            /* Check if SQM finished its initialization */
            DNXC_IF_ERR_EXIT(soc_dnx_polling(unit, JER2_ARAD_TIMEOUT, JER2_ARAD_MIN_POLLS, SQM_SQM_INITr, REG_PORT_ANY, 0, PDM_INIT_ONf, 0));
            DNXC_IF_ERR_EXIT(soc_dnx_polling(unit, JER2_ARAD_TIMEOUT, JER2_ARAD_MIN_POLLS, SQM_SQM_INITr, REG_PORT_ANY, 0, QDM_INIT_ONf, 0));
            DNXC_IF_ERR_EXIT(soc_dnx_polling(unit, JER2_ARAD_TIMEOUT, JER2_ARAD_MIN_POLLS, IPS_IPS_GENERAL_CONFIGURATIONSr, REG_PORT_ANY, 0, IPS_INIT_TRIGGERf, 0));
        }
        else
        {
            /* Check if IQM finished its 96K cycles initialization */
            DNXC_IF_ERR_EXIT(soc_dnx_polling(unit, JER2_ARAD_TIMEOUT, JER2_ARAD_MIN_POLLS, IQM_IQM_INITr, 0, 0, IQC_INITf, 0));
            DNXC_IF_ERR_EXIT(soc_dnx_polling(unit, JER2_ARAD_TIMEOUT, JER2_ARAD_MIN_POLLS, IQM_IQM_INITr, 0, 0, STE_INITf, 0));
            DNXC_IF_ERR_EXIT(soc_dnx_polling(unit, JER2_ARAD_TIMEOUT, JER2_ARAD_MIN_POLLS, IQM_IQM_INITr, 0, 0, PDM_INITf, 0));
            if (SOC_DNX_CONFIG(unit)->core_mode.nof_active_cores > 1 )
            {
                /* in single core all PDM banks are assigned to core 0, this causes the PDM init of core 1 to stay in reset, also no point in checking other init fields on core 1 */
                DNXC_IF_ERR_EXIT(soc_dnx_polling(unit, JER2_ARAD_TIMEOUT, JER2_ARAD_MIN_POLLS, IQM_IQM_INITr, 1, 0, PDM_INITf, 0));
                DNXC_IF_ERR_EXIT(soc_dnx_polling(unit, JER2_ARAD_TIMEOUT, JER2_ARAD_MIN_POLLS, IQM_IQM_INITr, 1, 0, IQC_INITf, 0));
                DNXC_IF_ERR_EXIT(soc_dnx_polling(unit, JER2_ARAD_TIMEOUT, JER2_ARAD_MIN_POLLS, IQM_IQM_INITr, 1, 0, STE_INITf, 0));
            }
            DNXC_IF_ERR_EXIT(soc_dnx_polling(unit, JER2_ARAD_TIMEOUT, JER2_ARAD_MIN_POLLS, IPS_IPS_GENERAL_CONFIGURATIONSr, 0, 0, IPS_INIT_TRIGGERf, 0));
            DNXC_IF_ERR_EXIT(soc_dnx_polling(unit, JER2_ARAD_TIMEOUT, JER2_ARAD_MIN_POLLS, IPS_IPS_GENERAL_CONFIGURATIONSr, 1, 0, IPS_INIT_TRIGGERf, 0));

            /* Re-enable auto-gen in IDR */
            /*DNXC_IF_ERR_EXIT(soc_jer2_jer_ocb_dram_buffer_autogen_set(unit));*/
        }
        
        DNXC_LEGACY_FIXME_ASSERT;
#ifdef FIXME_DNX_LEGACY
        SHR_BIT_ITER(SOC_DNX_CONFIG(unit)->jer2_arad->init.drc_info.dram_bitmap, SOC_DNX_DEFS_GET(unit, hw_dram_interfaces_max), drc_ndx) {*/
            /* need reset PHY_CDR_MONITOR_ENf */
            if (SHR_BITGET(drc_monitor_enable_bitmap, drc_ndx)) {
                DNXC_IF_ERR_EXIT(soc_dnx_drc_combo28_cdr_monitor_enable_set(unit, drc_ndx, 1));
            }
        }
#endif 
    }

    if(is_egr) 
    {
        /* NOTE: EDB must be reset before EGQ/EPNI */
        soc_reg_above_64_field_set(unit, ECI_BLOCKS_SOFT_INITr, soft_init_reg_val, SOC_IS_JERICHO_PLUS_A0(unit) ? BLOCKS_SOFT_INIT_56f : BLOCKS_SOFT_INIT_52f, fld_above64_val);/* EDB Block ID */
        DNXC_IF_ERR_EXIT(WRITE_ECI_BLOCKS_SOFT_INITr(unit, soft_init_reg_val));

        /* NOTE: EGQ has to be reset before NBI blocks,
         * due to TXI counter.
         * The correct flow:
         * 1. Reset EGQ
         * 2. Reset egq2nif TXI.
         * 3. Reset NBI.
        */
        soc_reg_above_64_field_set(unit, ECI_BLOCKS_SOFT_INITr, soft_init_reg_val, SOC_IS_JERICHO_PLUS_A0(unit) ? BLOCKS_SOFT_INIT_50f : BLOCKS_SOFT_INIT_46f, fld_above64_val);/* EGQ0 Block ID */
        soc_reg_above_64_field_set(unit, ECI_BLOCKS_SOFT_INITr, soft_init_reg_val, SOC_IS_JERICHO_PLUS_A0(unit) ? BLOCKS_SOFT_INIT_52f : BLOCKS_SOFT_INIT_48f, fld_above64_val);/* EPNI0 Block ID */
        if (!SOC_IS_QAX(unit)) 
        {
            soc_reg_above_64_field_set(unit, ECI_BLOCKS_SOFT_INITr, soft_init_reg_val, SOC_IS_JERICHO_PLUS_A0(unit) ? BLOCKS_SOFT_INIT_51f : BLOCKS_SOFT_INIT_47f, fld_above64_val); /* EGQ1 Block ID */
            soc_reg_above_64_field_set(unit, ECI_BLOCKS_SOFT_INITr, soft_init_reg_val, SOC_IS_JERICHO_PLUS_A0(unit) ? BLOCKS_SOFT_INIT_53f : BLOCKS_SOFT_INIT_49f, fld_above64_val);/* EPNI1 Block ID */
        }
        DNXC_IF_ERR_EXIT(WRITE_ECI_BLOCKS_SOFT_INITr(unit, soft_init_reg_val));

        /* Take NIF TXIs in EGQ out of reset */
        DNXC_IF_ERR_EXIT(soc_jer2_jer_reset_nif_txi_oor(unit));

        for (field_index = 0; field_index < egress_field_list_size; ++field_index)
        {
            soc_reg_above_64_field_set(unit, ECI_BLOCKS_SOFT_INITr, soft_init_reg_val, egress_field_list[field_index], fld_above64_val);
        }
        if (is_fabric) 
        {
            soc_reg_above_64_field_set(unit, ECI_BLOCKS_SOFT_INITr, soft_init_reg_val, BLOCKS_SOFT_INIT_31f, fld_above64_val);/* FCR Block ID   */
            soc_reg_above_64_field_set(unit, ECI_BLOCKS_SOFT_INITr, soft_init_reg_val, BLOCKS_SOFT_INIT_32f, fld_above64_val);/* FDR Block ID   */

            if (SOC_IS_JERICHO_PLUS(unit)) 
            {
                DNXC_IF_ERR_EXIT(READ_FDR_FDR_ENABLERS_REGISTER_1r(unit, &reg64));
                soc_reg64_field32_set(unit, FDR_FDR_ENABLERS_REGISTER_1r, &reg64, FIELD_12_12f, 0);
                soc_reg64_field32_set(unit, FDR_FDR_ENABLERS_REGISTER_1r, &reg64, FIELD_13_13f, 0);
                soc_reg64_field32_set(unit, FDR_FDR_ENABLERS_REGISTER_1r, &reg64, FIELD_14_14f, 0);
                DNXC_IF_ERR_EXIT(WRITE_FDR_FDR_ENABLERS_REGISTER_1r(unit, reg64));                
            }
            else
            {
                DNXC_IF_ERR_EXIT(READ_FDR_FDR_ENABLERS_REGISTER_1r(unit, &reg64));
                soc_reg64_field32_set(unit, FDR_FDR_ENABLERS_REGISTER_1r, &reg64, FIELD_31_31f, 0);
                soc_reg64_field32_set(unit, FDR_FDR_ENABLERS_REGISTER_1r, &reg64, FIELD_32_32f, 0);
                soc_reg64_field32_set(unit, FDR_FDR_ENABLERS_REGISTER_1r, &reg64, FIELD_33_33f, 0);
                DNXC_IF_ERR_EXIT(WRITE_FDR_FDR_ENABLERS_REGISTER_1r(unit, reg64));
            }
        }
        DNXC_IF_ERR_EXIT(WRITE_ECI_BLOCKS_SOFT_INITr(unit, soft_init_reg_val));
        sal_usleep(1);

        /* Reset NBI special ports */
        if (!SOC_IS_QUX(unit)) {
            DNXC_IF_ERR_EXIT(READ_NBIH_ADDITIONAL_RESETSr(unit, &reg_val));
            soc_reg_field_set(unit, NBIH_ADDITIONAL_RESETSr, &reg_val, EIF_RSTNf, 1);
            soc_reg_field_set(unit, NBIH_ADDITIONAL_RESETSr, &reg_val, ELK_0_RX_RSTNf, 1);
            soc_reg_field_set(unit, NBIH_ADDITIONAL_RESETSr, &reg_val, ELK_0_TX_RSTNf, 1);
            if (!SOC_IS_QAX(unit)) 
            {
                soc_reg_field_set(unit, NBIH_ADDITIONAL_RESETSr, &reg_val, ELK_1_RX_RSTNf, 1); 
                soc_reg_field_set(unit, NBIH_ADDITIONAL_RESETSr, &reg_val, SIF_RSTNf, 1);
                soc_reg_field_set(unit, NBIH_ADDITIONAL_RESETSr, &reg_val, ELK_1_TX_RSTNf, 1);
            }
            DNXC_IF_ERR_EXIT(WRITE_NBIH_ADDITIONAL_RESETSr(unit,  reg_val));
    
            DNXC_IF_ERR_EXIT(READ_NBIL_ADDITIONAL_RESETSr(unit, 0, &reg_val));
            soc_reg_field_set(unit, NBIL_ADDITIONAL_RESETSr, &reg_val, SIF_RSTNf, 1);
            soc_reg_field_set(unit, NBIL_ADDITIONAL_RESETSr, &reg_val, ELK_0_RX_RSTNf, 1);
            soc_reg_field_set(unit, NBIL_ADDITIONAL_RESETSr, &reg_val, ELK_0_TX_RSTNf, 1);
            if (!SOC_IS_QAX(unit)) 
            {
                soc_reg_field_set(unit, NBIL_ADDITIONAL_RESETSr, &reg_val, ELK_1_RX_RSTNf, 1); 
                soc_reg_field_set(unit, NBIL_ADDITIONAL_RESETSr, &reg_val, ELK_1_TX_RSTNf, 1);
            }
            DNXC_IF_ERR_EXIT(WRITE_NBIL_ADDITIONAL_RESETSr(unit, 0, reg_val));
    
            DNXC_IF_ERR_EXIT(READ_NBIL_ADDITIONAL_RESETSr(unit, 1, &reg_val));
            soc_reg_field_set(unit, NBIL_ADDITIONAL_RESETSr, &reg_val, SIF_RSTNf, 1);
            soc_reg_field_set(unit, NBIL_ADDITIONAL_RESETSr, &reg_val, ELK_0_RX_RSTNf, 1);
            soc_reg_field_set(unit, NBIL_ADDITIONAL_RESETSr, &reg_val, ELK_0_TX_RSTNf, 1);
            if (!SOC_IS_QAX(unit)) 
            {
                soc_reg_field_set(unit, NBIL_ADDITIONAL_RESETSr, &reg_val, ELK_1_RX_RSTNf, 1); 
                soc_reg_field_set(unit, NBIL_ADDITIONAL_RESETSr, &reg_val, ELK_1_TX_RSTNf, 1);
            }
            DNXC_IF_ERR_EXIT(WRITE_NBIL_ADDITIONAL_RESETSr(unit, 1, reg_val));
        } else {
            DNXC_IF_ERR_EXIT(READ_NIF_ADDITIONAL_RESETSr(unit, &reg_val));
            soc_reg_field_set(unit, NIF_ADDITIONAL_RESETSr, &reg_val, EIF_RSTNf, 1);
            soc_reg_field_set(unit, NIF_ADDITIONAL_RESETSr, &reg_val, SIF_RSTNf, 1);
            DNXC_IF_ERR_EXIT(WRITE_NIF_ADDITIONAL_RESETSr(unit,  reg_val));
        }

        /* reset_ip_to_cmic_credits */
        DNXC_IF_ERR_EXIT(READ_ECI_GP_CONTROL_9r(unit, reg_above64_val));
        SOC_REG_ABOVE_64_CLEAR(fld_above64_val);
        soc_reg_above_64_field_set(unit, ECI_GP_CONTROL_9r, reg_above64_val, PIR_EGQ_0_RXI_RESET_Nf, fld_above64_val);
        soc_reg_above_64_field_set(unit, ECI_GP_CONTROL_9r, reg_above64_val, PIR_EGQ_1_RXI_RESET_Nf, fld_above64_val);
        DNXC_IF_ERR_EXIT(WRITE_ECI_GP_CONTROL_9r(unit, reg_above64_val));
                
        SOC_REG_ABOVE_64_CREATE_MASK(fld_above64_val, 1, 0);
        soc_reg_above_64_field_set(unit, ECI_GP_CONTROL_9r, reg_above64_val, PIR_EGQ_0_RXI_RESET_Nf, fld_above64_val);
        soc_reg_above_64_field_set(unit, ECI_GP_CONTROL_9r, reg_above64_val, PIR_EGQ_1_RXI_RESET_Nf, fld_above64_val);
        DNXC_IF_ERR_EXIT(WRITE_ECI_GP_CONTROL_9r(unit, reg_above64_val));

        /* Reset PackerInterleaver (PIR) credits */
        DNXC_IF_ERR_EXIT(READ_ECI_GP_CONTROL_9r(unit, reg_above64_val));
        SOC_REG_ABOVE_64_CLEAR(fld_above64_val);
        soc_reg_above_64_field_set(unit, ECI_GP_CONTROL_9r, reg_above64_val, PIR_TXI_CREDITS_INIT_VALUEf, fld_above64_val);
        SOC_REG_ABOVE_64_CREATE_MASK(fld_above64_val, 1, 0);
        soc_reg_above_64_field_set(unit, ECI_GP_CONTROL_9r, reg_above64_val, PIR_TXI_CREDITS_INITf, fld_above64_val);
        DNXC_IF_ERR_EXIT(WRITE_ECI_GP_CONTROL_9r(unit, reg_above64_val));
        SOC_REG_ABOVE_64_CLEAR(fld_above64_val);
        soc_reg_above_64_field_set(unit, ECI_GP_CONTROL_9r, reg_above64_val, PIR_TXI_CREDITS_INITf, fld_above64_val);
        DNXC_IF_ERR_EXIT(WRITE_ECI_GP_CONTROL_9r(unit, reg_above64_val));

        /* Sending CPU credits from CMIC to EGQ */
        DNXC_IF_ERR_EXIT(WRITE_CMIC_RXBUF_EPINTF_RELEASE_ALL_CREDITSr(unit, 0x0));
        DNXC_IF_ERR_EXIT(WRITE_CMIC_RXBUF_EPINTF_RELEASE_ALL_CREDITSr(unit, 0x1));

        /* Open FCR */
        if (!soc_feature(unit, soc_feature_no_fabric)) {
            DNXC_IF_ERR_EXIT(soc_reg_above_64_field32_modify(unit, FCR_FCR_ENABLERS_AND_FILTER_MATCH_INPUT_LINKr, REG_PORT_ANY, 0, DIS_FL_STSf,  0x0));
            DNXC_IF_ERR_EXIT(soc_reg_above_64_field32_modify(unit, FCR_FCR_ENABLERS_AND_FILTER_MATCH_INPUT_LINKr, REG_PORT_ANY, 0, DIS_CRD_FCRf,  0x0));
            DNXC_IF_ERR_EXIT(soc_reg_above_64_field32_modify(unit, FCR_FCR_ENABLERS_AND_FILTER_MATCH_INPUT_LINKr, REG_PORT_ANY, 0, DIS_SRf,  0x0));
        }
        /* Resetting CMICM RXI credits */ 
        DNXC_IF_ERR_EXIT(soc_reg_above_64_field32_modify(unit, EGQ_INIT_FQP_TXI_CMICMr, SOC_CORE_ALL, 0, INIT_FQP_TXI_CMICMf,  0x1));
        DNXC_IF_ERR_EXIT(soc_reg_above_64_field32_modify(unit, EGQ_INIT_FQP_TXI_OLPr, SOC_CORE_ALL, 0, INIT_FQP_TXI_OLPf,  0x1));
        DNXC_IF_ERR_EXIT(soc_reg_above_64_field32_modify(unit, EGQ_INIT_FQP_TXI_OAMr, SOC_CORE_ALL, 0, INIT_FQP_TXI_OAMf,  0x1));
        DNXC_IF_ERR_EXIT(soc_reg_above_64_field32_modify(unit, EGQ_INIT_FQP_TXI_RCYr, SOC_CORE_ALL, 0, INIT_FQP_TXI_RCYf,  0x1));

        /* Waiting for longest block to finish */
        rv = soc_dnx_polling(unit, JER2_ARAD_TIMEOUT, JER2_ARAD_MIN_POLLS, EGQ_EGQ_BLOCK_INIT_STATUSr, REG_PORT_ANY, 0, EGQ_BLOCK_INITf, 0);
        if (SOC_FAILURE(rv)) 
        {
            LOG_ERROR(BSL_LS_SOC_INIT, (BSL_META_U(unit, "%s(): Error Validate out-of-reset done indications: EGQ_EGQ_BLOCK_INIT_STATUSr, EGQ_BLOCK_INITf.\n"), FUNCTION_NAME()));
        }

        /* Reactive queues that were active */
        for (core = 0 ; core < SOC_DNX_DEFS_GET(unit, nof_cores) ; core++) {
            DNXC_IF_ERR_EXIT(WRITE_CGM_CGM_QUEUE_IS_DISABLEDr(unit, core, queue_is_disable_orig[core]));
        }
    }

    /**************************************************************************/
    /** OUT-OF-RESET, Revert to original (Soft-init per-block map)           **/
    /**************************************************************************/
    /* soft_init_reg_val_orig is probably 0x0 - take all other blocks out of soft init */
    DNXC_IF_ERR_EXIT(WRITE_ECI_BLOCKS_SOFT_INITr(unit, soft_init_reg_val_orig));

    /* soft_init_reg_val_orig is probably 0x0 - take all other blocks out of soft init */
    SCHAN_UNLOCK(unit);
    is_schan_locked = 0;

    /**************************************************************************/
    /** Restore Configuration if needed                                      **/
    /**************************************************************************/
    if (is_fabric && !soc_feature(unit, soc_feature_no_fabric))
    {
        res = jer2_arad_mgmt_all_ctrl_cells_enable_set_unsafe(unit, is_ctrl_cells_enabled_orig, JER2_ARAD_MGMT_ALL_CTRL_CELLS_FLAGS_SOFT_RESET);
        DNXC_IF_ERR_EXIT(dnx_handle_sand_result(res));
    }

    /**************************************************************************/
    /**  Restore traffic                                                     **/
    /**************************************************************************/

    LOG_VERBOSE(BSL_LS_SOC_INIT, (BSL_META_U(unit, "%s(): Restore traffic.\n"), FUNCTION_NAME()));
    res = jer2_jer_mgmt_enable_traffic_set(unit, is_traffic_enabled_orig);
    DNXC_IF_ERR_EXIT(dnx_handle_sand_result(res));

    /**************************************************************************/
    /**  Clear interrupts                                                    **/
    /**************************************************************************/
    if (!SOC_IS_QAX(unit)) 
    {
        DNXC_IF_ERR_EXIT(WRITE_IQM_INTERRUPT_REGISTERr(unit, SOC_CORE_ALL,  0xffffffff)); 
    }
exit:
    if (is_schan_locked) 
    {
        SCHAN_UNLOCK(unit);
    }   
    DNXC_FUNC_RETURN;
}

/* 
 * take blocks in or out of sbus reset: 
 * is_in_reset = 1 in reset
 * is_in_reset = 0 out of reset
 */
static int soc_jer2_jer_reset_sbus_reset(int unit, int is_in_reset)
{
    soc_reg_above_64_val_t reg_above_64_val;
    DNXC_INIT_FUNC_DEFS;
    if((SOC_IS_JERICHO(unit)) && (!SOC_IS_QAX(unit))) 
    {
        /* IQMT */     
        DNXC_IF_ERR_EXIT(READ_ECI_BLOCKS_SBUS_RESETr(unit, reg_above_64_val));
        soc_reg_above_64_field32_set(unit, ECI_BLOCKS_SBUS_RESETr, reg_above_64_val, BLOCKS_SBUS_RESET_21f, is_in_reset);
        DNXC_IF_ERR_EXIT(WRITE_ECI_BLOCKS_SBUS_RESETr(unit, reg_above_64_val));
        /* IQMs */     
        DNXC_IF_ERR_EXIT(READ_ECI_BLOCKS_SBUS_RESETr(unit, reg_above_64_val));
        soc_reg_above_64_field32_set(unit, ECI_BLOCKS_SBUS_RESETr, reg_above_64_val, BLOCKS_SBUS_RESET_19f, is_in_reset);
        soc_reg_above_64_field32_set(unit, ECI_BLOCKS_SBUS_RESETr, reg_above_64_val, BLOCKS_SBUS_RESET_20f, is_in_reset);
        DNXC_IF_ERR_EXIT(WRITE_ECI_BLOCKS_SBUS_RESETr(unit, reg_above_64_val));
    }
    /* all the rest */
    if (is_in_reset) {
        SOC_REG_ABOVE_64_ALLONES(reg_above_64_val);
    } else {
        SOC_REG_ABOVE_64_CLEAR(reg_above_64_val);
    }
    DNXC_IF_ERR_EXIT(WRITE_ECI_BLOCKS_SBUS_RESETr(unit, reg_above_64_val));

exit:
    DNXC_FUNC_RETURN;
}

/* 
 * take blocks in or out of soft reset: 
 * is_in_reset = 1 in reset
 * is_in_reset = 0 out of reset
 */
static int soc_jer2_jer_reset_blocks_reset(int unit, int is_in_reset)
{
    int disable_hard_reset;
    uint32 reg_val;
    soc_reg_above_64_val_t reg_above_64_val;
    soc_field_t field;
    DNXC_INIT_FUNC_DEFS;

    if (SOC_IS_QAX(unit)) 
    {
        /* IQMT */     
        DNXC_IF_ERR_EXIT(READ_ECI_BLOCKS_SOFT_RESETr(unit, reg_above_64_val));
        soc_reg_above_64_field32_set(unit, ECI_BLOCKS_SOFT_RESETr, reg_above_64_val, BLOCKS_SOFT_RESET_21f, is_in_reset);
        DNXC_IF_ERR_EXIT(WRITE_ECI_BLOCKS_SOFT_RESETr(unit, reg_above_64_val));

        /* IQMs */     
        DNXC_IF_ERR_EXIT(READ_ECI_BLOCKS_SOFT_RESETr(unit, reg_above_64_val));
        soc_reg_above_64_field32_set(unit, ECI_BLOCKS_SOFT_RESETr, reg_above_64_val, BLOCKS_SOFT_RESET_19f, is_in_reset);
        soc_reg_above_64_field32_set(unit, ECI_BLOCKS_SOFT_RESETr, reg_above_64_val, BLOCKS_SOFT_RESET_20f, is_in_reset);
        DNXC_IF_ERR_EXIT(WRITE_ECI_BLOCKS_SOFT_RESETr(unit, reg_above_64_val));

        /* IPSEC */
        DNXC_IF_ERR_EXIT(READ_ECI_BLOCKS_SOFT_RESETr(unit, reg_above_64_val));
        soc_reg_above_64_field32_set(unit, ECI_BLOCKS_SOFT_RESETr, reg_above_64_val, BLOCKS_SOFT_RESET_114f, is_in_reset);
        soc_reg_above_64_field32_set(unit, ECI_BLOCKS_SOFT_RESETr, reg_above_64_val, BLOCKS_SOFT_RESET_115f, is_in_reset);
        soc_reg_above_64_field32_set(unit, ECI_BLOCKS_SOFT_RESETr, reg_above_64_val, BLOCKS_SOFT_RESET_116f, is_in_reset);
        DNXC_IF_ERR_EXIT(WRITE_ECI_BLOCKS_SOFT_RESETr(unit, reg_above_64_val));

    }

    /* All the rest */
    if (is_in_reset) {
        SOC_REG_ABOVE_64_ALLONES(reg_above_64_val);
    } else {
        SOC_REG_ABOVE_64_CLEAR(reg_above_64_val);
    }
    DNXC_IF_ERR_EXIT(WRITE_ECI_BLOCKS_SOFT_RESETr(unit, reg_above_64_val));

    /* Take PDM in or out of reset */
    if (SOC_REG_IS_VALID(unit,ECI_GP_CONTROL_9r)) {
        DNXC_IF_ERR_EXIT(READ_ECI_GP_CONTROL_9r(unit, reg_above_64_val));
        soc_reg_above_64_field32_set(unit, ECI_GP_CONTROL_9r, reg_above_64_val, PDM_RSTNf, is_in_reset ? 0 : 1);
        DNXC_IF_ERR_EXIT(WRITE_ECI_GP_CONTROL_9r(unit, reg_above_64_val));
    }

    /* Take MBU in or out of reset */
    DNXC_IF_ERR_EXIT(READ_ECI_ECIC_BLOCKS_RESETr(unit, &reg_val));
    field = SOC_IS_JERICHO_PLUS_A0(unit) || SOC_IS_QUX(unit) ? PNIMI_002f : FIELD_0_0f;
    soc_reg_field_set(unit, ECI_ECIC_BLOCKS_RESETr, &reg_val,field , is_in_reset);
    if (soc_reg_field_valid(unit, ECI_ECIC_BLOCKS_RESETr, TIME_SYNC_RESETf)) {
        soc_reg_field_set(unit, ECI_ECIC_BLOCKS_RESETr, &reg_val, TIME_SYNC_RESETf, is_in_reset);
    }
    if (SOC_IS_FLAIR(unit)) {
        soc_reg_field_set(unit, ECI_ECIC_BLOCKS_RESETr, &reg_val, FGL_ING_RESET_Nf, is_in_reset);
        soc_reg_field_set(unit, ECI_ECIC_BLOCKS_RESETr, &reg_val, FGL_EGR_RESET_Nf, is_in_reset);
    }
    DNXC_IF_ERR_EXIT(WRITE_ECI_ECIC_BLOCKS_RESETr(unit, reg_val));

    disable_hard_reset = soc_property_suffix_num_get(unit, -1, spn_CUSTOM_FEATURE, "init_without_device_hard_reset", 0);
    LOG_VERBOSE(BSL_LS_SOC_INIT, (BSL_META_U(unit, "%s(): disable_hard_reset = %d\n"), FUNCTION_NAME(), disable_hard_reset));
    if (disable_hard_reset == 1) {
        if (is_in_reset == 1) {
            /* Fix for : IRE reset doesn't clear CMIC credits - when IRE going out of reset it always adds credits to the CMIC, so CMIC Credits needs to be reset when ever IRE is reset */
            DNXC_IF_ERR_EXIT(WRITE_CMIC_TXBUF_IPINTF_INTERFACE_CREDITSr(unit, 0x40));
            DNXC_IF_ERR_EXIT(WRITE_CMIC_TXBUF_IPINTF_INTERFACE_CREDITSr(unit, 0x0));

            /* Release all credits of CMIC packet I/F so that it can accept packets */
            DNXC_IF_ERR_EXIT(WRITE_CMIC_RXBUF_EPINTF_RELEASE_ALL_CREDITSr(unit, 0x0));
            DNXC_IF_ERR_EXIT(WRITE_CMIC_RXBUF_EPINTF_RELEASE_ALL_CREDITSr(unit, 0x1));
            DNXC_IF_ERR_EXIT(WRITE_CMIC_RXBUF_EPINTF_RELEASE_ALL_CREDITSr(unit, 0x0));
        }
    }

    if (is_in_reset == 0x0) {
        /* Verify blocks are OOR */
        DNXC_IF_ERR_EXIT(soc_jer2_jer_reset_blocks_poll_init_finish(unit));
    }

exit:
    DNXC_FUNC_RETURN;
}


int soc_jer2_jer_reset_blocks_soft_init(int unit, int reset_action)
{
    soc_reg_above_64_val_t reg_above_64_val;
    
    DNXC_INIT_FUNC_DEFS;

    if ((reset_action == SOC_DNX_RESET_ACTION_IN_RESET) || (reset_action == SOC_DNX_RESET_ACTION_INOUT_RESET)) { 
        SOC_REG_ABOVE_64_ALLONES(reg_above_64_val);
        DNXC_IF_ERR_EXIT(WRITE_ECI_BLOCKS_SOFT_INITr(unit, reg_above_64_val));
    }

    sal_usleep(100);

    if ((reset_action == SOC_DNX_RESET_ACTION_OUT_RESET) || (reset_action == SOC_DNX_RESET_ACTION_INOUT_RESET)) {

        /* IQMT */
        DNXC_IF_ERR_EXIT(READ_ECI_BLOCKS_SOFT_INITr(unit, reg_above_64_val));
        soc_reg_above_64_field32_set(unit, ECI_BLOCKS_SOFT_INITr, reg_above_64_val, BLOCKS_SOFT_INIT_21f, 0x0);
        DNXC_IF_ERR_EXIT(WRITE_ECI_BLOCKS_SOFT_INITr(unit, reg_above_64_val));

        /* IQMs */
        DNXC_IF_ERR_EXIT(READ_ECI_BLOCKS_SOFT_INITr(unit, reg_above_64_val));
        soc_reg_above_64_field32_set(unit, ECI_BLOCKS_SOFT_INITr, reg_above_64_val, BLOCKS_SOFT_INIT_19f, 0x0);
        soc_reg_above_64_field32_set(unit, ECI_BLOCKS_SOFT_INITr, reg_above_64_val, BLOCKS_SOFT_INIT_20f, 0x0);
        DNXC_IF_ERR_EXIT(WRITE_ECI_BLOCKS_SOFT_INITr(unit, reg_above_64_val));

        /* EDB */
        DNXC_IF_ERR_EXIT(READ_ECI_BLOCKS_SOFT_INITr(unit, reg_above_64_val));
        soc_reg_above_64_field32_set(unit, ECI_BLOCKS_SOFT_INITr, reg_above_64_val, SOC_IS_JERICHO_PLUS_A0(unit) ? BLOCKS_SOFT_INIT_56f : BLOCKS_SOFT_INIT_52f, 0x0);
        DNXC_IF_ERR_EXIT(WRITE_ECI_BLOCKS_SOFT_INITr(unit, reg_above_64_val));

        /* EGQs */
        DNXC_IF_ERR_EXIT(READ_ECI_BLOCKS_SOFT_INITr(unit, reg_above_64_val));
        soc_reg_above_64_field32_set(unit, ECI_BLOCKS_SOFT_INITr, reg_above_64_val, SOC_IS_JERICHO_PLUS_A0(unit) ? BLOCKS_SOFT_INIT_50f : BLOCKS_SOFT_INIT_46f, 0x0);
        soc_reg_above_64_field32_set(unit, ECI_BLOCKS_SOFT_INITr, reg_above_64_val, SOC_IS_JERICHO_PLUS_A0(unit) ? BLOCKS_SOFT_INIT_51f : BLOCKS_SOFT_INIT_47f, 0x0);
        DNXC_IF_ERR_EXIT(WRITE_ECI_BLOCKS_SOFT_INITr(unit, reg_above_64_val));

        /* EPNIs */
        DNXC_IF_ERR_EXIT(READ_ECI_BLOCKS_SOFT_INITr(unit, reg_above_64_val));
        soc_reg_above_64_field32_set(unit, ECI_BLOCKS_SOFT_INITr, reg_above_64_val, SOC_IS_JERICHO_PLUS_A0(unit) ? BLOCKS_SOFT_INIT_52f : BLOCKS_SOFT_INIT_48f, 0x0);
        soc_reg_above_64_field32_set(unit, ECI_BLOCKS_SOFT_INITr, reg_above_64_val, SOC_IS_JERICHO_PLUS_A0(unit) ? BLOCKS_SOFT_INIT_53f : BLOCKS_SOFT_INIT_49f, 0x0);
        DNXC_IF_ERR_EXIT(WRITE_ECI_BLOCKS_SOFT_INITr(unit, reg_above_64_val));

        /* Take ALL blocks Out of soft reset */
        SOC_REG_ABOVE_64_CLEAR(reg_above_64_val);
        DNXC_IF_ERR_EXIT(WRITE_ECI_BLOCKS_SOFT_INITr(unit, reg_above_64_val));

        /* Validate Soft init finished */
        DNXC_IF_ERR_EXIT(soc_jer2_jer_reset_blocks_poll_init_finish(unit));

        /* Reset IRE - CMIC TXI */
        if (!SOC_IS_QAX(unit)) { 
            DNXC_IF_ERR_EXIT(soc_reg_field32_modify(unit, IRE_SYS_CFG_RESERVED_1r, REG_PORT_ANY, FIELD_0_0f, 0x1));
            DNXC_IF_ERR_EXIT(soc_reg_field32_modify(unit, IRE_SYS_CFG_RESERVED_1r, REG_PORT_ANY, FIELD_0_0f, 0x0));
        }

    }

exit:
    DNXC_FUNC_RETURN;
}

static int soc_jer2_jer_device_blocks_reset(int unit, int reset_action)
{
    DNXC_INIT_FUNC_DEFS;

    if ((reset_action == SOC_DNX_RESET_ACTION_IN_RESET) || (reset_action == SOC_DNX_RESET_ACTION_INOUT_RESET)) { 

        /* Sbus Reset*/
        DNXC_IF_ERR_EXIT(soc_jer2_jer_reset_sbus_reset(unit, 1));
        
        /* Soft Reset*/
        DNXC_IF_ERR_EXIT(soc_jer2_jer_reset_blocks_reset(unit, 1));
    }

    if ((reset_action == SOC_DNX_RESET_ACTION_OUT_RESET) || (reset_action == SOC_DNX_RESET_ACTION_INOUT_RESET)) {

        /* sbus reset */
        DNXC_IF_ERR_EXIT(soc_jer2_jer_reset_sbus_reset(unit, 0));

        DNXC_IF_ERR_EXIT(jer2_jer_pll_init(unit));

        /* soft reset */
        DNXC_IF_ERR_EXIT(soc_jer2_jer_reset_blocks_reset(unit, 0));
    }

exit:
    DNXC_FUNC_RETURN;
}

static int soc_jer2_jer_init_reg_access(
    int unit,
    int reset_action)
{
    DNXC_INIT_FUNC_DEFS;

 

     /* init defines  & sw_state data structures*/
    if ((SOC_CONTROL(unit)->soc_flags & SOC_F_INITED) == 0) {
        int sw_state_size = soc_property_get(unit, spn_SW_STATE_MAX_SIZE,
                  SHR_SW_STATE_MAX_DATA_SIZE_IN_BYTES);

        soc_jer2_jer_specific_info_config_derived(unit);
        SOC_DNX_CONFIG(unit)->emulation_system = soc_property_get(unit, spn_DIAG_EMULATOR_PARTIAL_INIT, 0x0);
        soc_dnx_implementation_defines_init(unit);
        DNXC_IF_ERR_RETURN(shr_sw_state_init(unit, 0x0, socSwStateDataBlockRegularInit, sw_state_size));
        DNXC_IF_ERR_EXIT(soc_dnx_info_config_ports(unit));
    }

    /*
     * Reset device.
     * Also enable device access, set default Iproc/CmicD configuration
     * No access allowed before this stage.
     */
    DNXC_IF_ERR_EXIT(soc_jer2_jer_init_reset(unit, reset_action));
    /* Enable Access to device blocks */
    DNXC_IF_ERR_EXIT(soc_jer2_jer_device_blocks_reset(unit, SOC_DNX_RESET_ACTION_INOUT_RESET));

    /* Init blocks' broadcast IDs */
    if ((!SOC_IS_QAX(unit)) && (!SOC_IS_FLAIR(unit))) {
        DNXC_IF_ERR_EXIT(soc_jer2_jer_init_brdc_blk_id_set(unit)); 
    }

    /* Init WB engine for ports reg access if not already initiated */
/*
    if ((SOC_CONTROL(unit)->soc_flags & SOC_F_INITED) == 0) {
        DNXC_IF_ERR_RETURN(soc_dnx_wb_engine_init(unit)); 
    }
*/
#ifdef BCM_WARM_BOOT_SUPPORT
    soc_jer2_arad_init_empty_scache(unit);
#endif

    /* Enable ports reg access */
    if ((SOC_CONTROL(unit)->soc_flags & SOC_F_INITED) == 0) {
        SOC_INFO(unit).fabric_logical_port_base = soc_property_get(unit, spn_FABRIC_LOGICAL_PORT_BASE, \
            SOC_DNX_FABRIC_LOGICAL_PORT_BASE_DEFAULT);
        DNXC_IF_ERR_EXIT(soc_jer2_jer_ports_config(unit));
    }

    SOC_CONTROL(unit)->soc_flags |= SOC_F_INITED;
exit:
    DNXC_FUNC_RETURN;
}

int
soc_jer2_jer_device_reset(int unit, int mode, int action)
{
    uint32 enable;
    DNXC_INIT_FUNC_DEFS;

    if (!SOC_IS_JERICHO(unit)) {
        DNXC_EXIT_WITH_ERR(SOC_E_UNAVAIL, (_BSL_DNXC_MSG("Jericho function. invalid Device")));
    }

    switch (mode) {
    case SOC_DNX_RESET_MODE_HARD_RESET:
        DNXC_IF_ERR_RETURN(soc_dnxc_cmic_device_hard_reset(unit, action));
        break;
    case SOC_DNX_RESET_MODE_BLOCKS_RESET:
        DNXC_IF_ERR_RETURN(soc_jer2_jer_device_blocks_reset(unit, action));
        break;
    case SOC_DNX_RESET_MODE_BLOCKS_SOFT_RESET:
        DNXC_IF_ERR_RETURN(jer2_jer_soft_reset(unit, JER2_ARAD_DBG_RST_DOMAIN_FULL));
        break;
    case SOC_DNX_RESET_MODE_BLOCKS_SOFT_INGRESS_RESET:
        DNXC_IF_ERR_RETURN(jer2_jer_soft_reset(unit, JER2_ARAD_DBG_RST_DOMAIN_INGR));
        break;
    case SOC_DNX_RESET_MODE_BLOCKS_SOFT_EGRESS_RESET:
        DNXC_IF_ERR_RETURN(jer2_jer_soft_reset(unit, JER2_ARAD_DBG_RST_DOMAIN_EGR));
        break;
    case SOC_DNX_RESET_MODE_BLOCKS_AND_FABRIC_SOFT_RESET:
        DNXC_IF_ERR_RETURN(jer2_jer_soft_reset(unit, JER2_ARAD_DBG_RST_DOMAIN_FULL_AND_FABRIC));
        break;
    case SOC_DNX_RESET_MODE_BLOCKS_AND_FABRIC_SOFT_INGRESS_RESET:
        DNXC_IF_ERR_RETURN(jer2_jer_soft_reset(unit, JER2_ARAD_DBG_RST_DOMAIN_INGR_AND_FABRIC));
        break;
    case SOC_DNX_RESET_MODE_BLOCKS_AND_FABRIC_SOFT_EGRESS_RESET:
        DNXC_IF_ERR_RETURN(jer2_jer_soft_reset(unit, JER2_ARAD_DBG_RST_DOMAIN_EGR_AND_FABRIC));
        break;
    case SOC_DNX_RESET_MODE_INIT_RESET:
        DNXC_IF_ERR_RETURN(soc_dnx_jer2_jericho_init(unit, action));
        break;
    case SOC_DNX_RESET_MODE_REG_ACCESS:
        DNXC_IF_ERR_RETURN(soc_jer2_jer_init_reg_access(unit, action));
        break;
    case SOC_DNX_RESET_MODE_ENABLE_TRAFFIC:
        enable = (action == SOC_DNX_RESET_ACTION_IN_RESET) ? FALSE : TRUE;
        DNXC_IF_ERR_RETURN(dnx_handle_sand_result(jer2_jer_mgmt_enable_traffic_set(unit, enable)));
        break;
    case SOC_DNX_RESET_MODE_BLOCKS_SOFT_RESET_DIRECT:
        DNXC_IF_ERR_RETURN(soc_jer2_jer_reset_blocks_soft_init(unit, action));
        break;
    default:
        DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("Unknown/Unsupported Reset Mode")));
    }

exit:
    DNXC_FUNC_RETURN;
}

