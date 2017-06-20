/*
 * $Id: jer_init.c Exp $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 */
#ifdef _ERR_MSG_MODULE_NAME 
    #error "_ERR_MSG_MODULE_NAME redefined" 
#endif
#define _ERR_MSG_MODULE_NAME BSL_SOC_INIT

/* 
 * Includes
 */ 

#include <shared/bsl.h>

/* SAL includes */
#include <sal/appl/sal.h>

/* SOC includes */
#include <soc/debug.h>
#include <soc/error.h>
#include <soc/iproc.h>
#include <soc/mem.h>

/* SOC DPP includes */
#include <soc/dpp/drv.h>
#include <soc/dpp/mbcm.h>
#include <soc/dcmn/dcmn_mem.h>

/* SOC DPP JER includes */
#include <soc/dpp/JER/jer_drv.h>
#include <soc/dpp/JER/jer_defs.h>
#include <soc/dpp/JER/jer_init.h>
#include <soc/dpp/JER/jer_ocb_dram_buffers.h>
#include <soc/dpp/JER/jer_mgmt.h>
#include <soc/dpp/JER/jer_reset.h>
#include <soc/dpp/JER/jer_fabric.h>
#include <soc/dpp/JER/jer_trunk.h>
#include <soc/dpp/JER/jer_egr_queuing.h>
#include <soc/dpp/JER/jer_tdm.h>
#include <soc/dpp/JER/jer_ingress_packet_queuing.h>
#include <soc/dpp/JER/jer_ingress_traffic_mgmt.h>

#include <soc/dpp/JER/JER_PP/jer_pp_ing_protection.h>
#include <soc/dpp/JER/JER_PP/jer_pp_eg_protection.h>
#include <soc/dpp/JER/JER_PP/jer_pp_lif.h>
#include <soc/dpp/JER/JER_PP/jer_pp_oam.h>
#include <soc/dpp/JER/JER_PP/jer_pp_eg_encap.h>

#include <soc/dpp/JER/jer_regs.h>
#include <soc/dpp/JER/jer_tbls.h>
#include <soc/dpp/JER/jer_ofp_rates.h>
#include <soc/dpp/JER/jer_sch.h>
#include <soc/dpp/JER/jer_stat_if.h>
#include <soc/dpp/JER/jer_dram.h>
#include <soc/dpp/JER/jer_ports.h>

#include <soc/dpp/JER/JER_PP/jer_pp_lbp_init.h>

#if defined(BCM_88675_A0) && defined(INCLUDE_KBP) && !defined(BCM_88030)
#include <soc/dpp/JER/JER_PP/jer_pp_kaps.h>
#endif /* defined(BCM_88675_A0) && defined(INCLUDE_KBP) && !defined(BCM_88030) */

#include <soc/dpp/JER/JER_PP/jer_pp_mymac.h>

/* SOC DPP Arad includes */ 
#include <soc/dpp/ARAD/arad_chip_regs.h>
#include <soc/dpp/ARAD/arad_egr_prog_editor.h>
#include <soc/dpp/ARAD/arad_init.h>
#include <soc/dpp/ARAD/arad_dram.h>
#include <soc/dpp/ARAD/arad_parser.h>
#include <soc/dpp/ARAD/arad_ports.h>
#include <soc/dpp/ARAD/arad_pmf_low_level.h>
#include <soc/dpp/ARAD/arad_mgmt.h>
#if defined(INCLUDE_KBP) && !defined(BCM_88030)
#include <soc/dpp/ARAD/arad_kbp.h>
#endif
#include <soc/dpp/ARAD/arad_cnt.h>
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_flp_init.h>
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_isem_access.h>
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_trap_mgmt.h>
#include <soc/dpp/JER/jer_flow_control.h>
#include <soc/dpp/multicast_imp.h>

#include <soc/dpp/QAX/qax_init.h>
#include <soc/dpp/QAX/qax_tbls.h>
#include <soc/dpp/QAX/qax_link_bonding.h>
#include <soc/dpp/QAX/qax_stat_if.h>

#include <soc/dpp/port_sw_db.h>

#ifdef PLISIM
    #include <soc/dpp/ARAD/arad_sim_em.h>
#endif

/* alternating <block>_INDIRECT_COMMANDr and <block>_INDIRECT_FORCE_BUBBLEr for bubble configuration */
static soc_reg_t jer_blocks_with_standart_bubble_mechanism[] = {
    IRE_INDIRECT_COMMANDr,        IRE_INDIRECT_FORCE_BUBBLEr,
    IDR_INDIRECT_COMMANDr,        IDR_INDIRECT_FORCE_BUBBLEr,
    MRPS_INDIRECT_COMMANDr,       MRPS_INDIRECT_FORCE_BUBBLEr,
    IRR_INDIRECT_COMMANDr,        IRR_INDIRECT_FORCE_BUBBLEr,
    OCB_INDIRECT_COMMANDr,        OCB_INDIRECT_FORCE_BUBBLEr,
    IQM_INDIRECT_COMMANDr,        IQM_INDIRECT_FORCE_BUBBLEr,
    IQMT_INDIRECT_COMMANDr,       IQMT_INDIRECT_FORCE_BUBBLEr,
    CRPS_INDIRECT_COMMANDr,       CRPS_INDIRECT_FORCE_BUBBLEr,
    IPS_INDIRECT_COMMANDr,        IPS_INDIRECT_FORCE_BUBBLEr,
    IPST_INDIRECT_COMMANDr,       IPST_INDIRECT_FORCE_BUBBLEr,
    IPT_INDIRECT_COMMANDr,        IPT_INDIRECT_FORCE_BUBBLEr,
    RTP_INDIRECT_COMMANDr,        RTP_INDIRECT_FORCE_BUBBLEr,
    EGQ_INDIRECT_COMMANDr,        EGQ_INDIRECT_FORCE_BUBBLEr,
    EPNI_INDIRECT_COMMANDr,       EPNI_INDIRECT_FORCE_BUBBLEr,
    OLP_INDIRECT_COMMANDr,        OLP_INDIRECT_FORCE_BUBBLEr,
    IHP_INDIRECT_COMMANDr,        IHP_INDIRECT_FORCE_BUBBLEr,
    IHB_INDIRECT_COMMANDr,        IHB_INDIRECT_FORCE_BUBBLEr,

    /* last reg in array */
    INVALIDr
};


/* 
 * Init functions
 */

int soc_jer_init_prepare_internal_data(int unit)
{

    SOCDNX_INIT_FUNC_DEFS;

    /* Calculate boundaries */
    if (SOC_IS_QAX(unit)) {
        SOCDNX_IF_ERR_EXIT(soc_qax_dram_buffer_conf_calc(unit));
    } else {
        SOCDNX_IF_ERR_EXIT(soc_jer_ocb_dram_buffer_conf_calc(unit)); 
    }

exit:
    SOCDNX_FUNC_RETURN;
}


/* excludes irrelevant blocks which are missing due to partial emul compilation */
STATIC void soc_jer_init_exclude_blocks(int unit)
{
    SOC_INFO(unit).block_valid[CLP_BLOCK(unit, 3)] = FALSE;
    SOC_INFO(unit).block_valid[CLP_BLOCK(unit, 4)] = FALSE;
    SOC_INFO(unit).block_valid[CLP_BLOCK(unit, 5)] = FALSE;
    SOC_INFO(unit).block_valid[NBIL_BLOCK(unit, 0)] = FALSE;
    SOC_INFO(unit).block_valid[NBIL_BLOCK(unit, 1)] = FALSE;
    SOC_INFO(unit).block_valid[MMU_BLOCK(unit)] = FALSE;
    SOC_INFO(unit).block_valid[DRCA_BLOCK(unit)] = FALSE;
    SOC_INFO(unit).block_valid[DRCB_BLOCK(unit)] = FALSE;
    SOC_INFO(unit).block_valid[DRCC_BLOCK(unit)] = FALSE;
    SOC_INFO(unit).block_valid[DRCD_BLOCK(unit)] = FALSE;
    SOC_INFO(unit).block_valid[DRCE_BLOCK(unit)] = FALSE;
    SOC_INFO(unit).block_valid[DRCF_BLOCK(unit)] = FALSE;
    SOC_INFO(unit).block_valid[DRCG_BLOCK(unit)] = FALSE;
    SOC_INFO(unit).block_valid[DRCH_BLOCK(unit)] = FALSE;
    SOC_INFO(unit).block_valid[DRCBROADCAST_BLOCK(unit)] = FALSE;
}

int soc_jer_init_blocks_init_global_conf(int unit)
{
    uint32 reg32_val, field_val;
    ARAD_MGMT_INIT* init;

    SOCDNX_INIT_FUNC_DEFS;

    init = &SOC_DPP_CONFIG(unit)->arad->init;

    /*Petra-b in system */
    if (SOC_DPP_CONFIG(unit)->tm.is_petrab_in_system) {
        SOCDNX_IF_ERR_EXIT(READ_ECI_GLOBAL_SYS_HEADER_CFGr(unit, &reg32_val));
        soc_reg_field_set(unit, ECI_GLOBAL_SYS_HEADER_CFGr, &reg32_val, SYSTEM_HEADERS_MODEf, 1);
        SOCDNX_IF_ERR_EXIT(WRITE_ECI_GLOBAL_SYS_HEADER_CFGr(unit, reg32_val));
    }
    else if (soc_property_get(unit, spn_SYSTEM_IS_ARAD_IN_SYSTEM, 0)) {
        SOCDNX_IF_ERR_EXIT(READ_ECI_GLOBAL_SYS_HEADER_CFGr(unit, &reg32_val));
        soc_reg_field_set(unit, ECI_GLOBAL_SYS_HEADER_CFGr, &reg32_val, SYSTEM_HEADERS_MODEf, 2);
        SOCDNX_IF_ERR_EXIT(WRITE_ECI_GLOBAL_SYS_HEADER_CFGr(unit, reg32_val));
    }

    /* FTMH LB and Stacking mode */
    SOCDNX_IF_ERR_EXIT(READ_ECI_GLOBAL_SYS_HEADER_CFGr(unit, &reg32_val));
    field_val = init->fabric.ftmh_lb_ext_mode == ARAD_MGMT_FTMH_LB_EXT_MODE_DISABLED ? 0 : 1;
    soc_reg_field_set(unit, ECI_GLOBAL_SYS_HEADER_CFGr, &reg32_val, FTMH_LB_KEY_EXT_ENf, field_val);
    field_val = init->fabric.ftmh_stacking_ext_mode == 0 ? 0 : 1;
    soc_reg_field_set(unit, ECI_GLOBAL_SYS_HEADER_CFGr, &reg32_val, FTMH_STACKING_EXT_ENABLEf, field_val);
    SOCDNX_IF_ERR_EXIT(WRITE_ECI_GLOBAL_SYS_HEADER_CFGr(unit, reg32_val));

    /* Configure core mode, not in clear-channel mode */
    SOCDNX_IF_ERR_EXIT(READ_ECI_GLOBAL_GENERAL_CFG_2r(unit, &reg32_val));
    if (SOC_DPP_CONFIG(unit)->core_mode.nof_active_cores == 1) {
        soc_reg_field_set(unit, ECI_GLOBAL_GENERAL_CFG_2r, &reg32_val, TURBO_PIPEf, 1);
        soc_reg_field_set(unit, ECI_GLOBAL_GENERAL_CFG_2r, &reg32_val, DUAL_FAP_MODEf, 0);
    } else {
        soc_reg_field_set(unit, ECI_GLOBAL_GENERAL_CFG_2r, &reg32_val, TURBO_PIPEf, 0);
        soc_reg_field_set(unit, ECI_GLOBAL_GENERAL_CFG_2r, &reg32_val, DUAL_FAP_MODEf, 1);
    }
    soc_reg_field_set(unit, ECI_GLOBAL_GENERAL_CFG_2r, &reg32_val, FMC_ENf, 1);
    SOCDNX_IF_ERR_EXIT(WRITE_ECI_GLOBAL_GENERAL_CFG_2r(unit, reg32_val));

    SOCDNX_IF_ERR_EXIT(READ_ECI_GLOBAL_GENERAL_CFG_1r(unit, &reg32_val));
      /*
       *  Mesh Mode
       */
    if (SOC_DPP_CONFIG(unit)->arad->init.fabric.connect_mode == ARAD_FABRIC_CONNECT_MODE_MESH || 
        /*treating single fap as mesh for tables configuration*/
        SOC_DPP_CONFIG(unit)->arad->init.fabric.connect_mode == ARAD_FABRIC_CONNECT_MODE_SINGLE_FAP )
    {
        soc_reg_field_set(unit, ECI_GLOBAL_GENERAL_CFG_1r, &reg32_val, MESH_MODEf, 1);
    }
    else
    {
        /* Mesh Not enabled, also for BACK2BACK devices */
        soc_reg_field_set(unit, ECI_GLOBAL_GENERAL_CFG_1r, &reg32_val, MESH_MODEf, 0);
    }


    if(SOC_DPP_CONFIG(unit)->arad->init.fabric.is_128_in_system) {
        field_val = 0x1;
    } else if (SOC_DPP_CONFIG(unit)->arad->init.fabric.system_contains_multiple_pipe_device) {
        field_val = 0x2;
    } else {
        field_val = 0x0;
    }   
    soc_reg_field_set(unit, ECI_GLOBAL_GENERAL_CFG_1r, &reg32_val, RESERVED_QTSf, field_val);
    soc_reg_field_set(unit, ECI_GLOBAL_GENERAL_CFG_1r, &reg32_val, TDM_ATTRIBUTEf, 0x1);
    SOCDNX_IF_ERR_EXIT(WRITE_ECI_GLOBAL_GENERAL_CFG_1r(unit, reg32_val));
    
    SOCDNX_IF_ERR_EXIT(READ_ECI_GLOBAL_GENERAL_CFG_3r(unit, &reg32_val));
    soc_reg_field_set(unit, ECI_GLOBAL_GENERAL_CFG_3r, &reg32_val, PACKET_CRC_ENf, 1);
    soc_reg_field_set(unit, ECI_GLOBAL_GENERAL_CFG_3r, &reg32_val, OPPORTUNISTIC_CRC_ENf, 1);
    soc_reg_field_set(unit, ECI_GLOBAL_GENERAL_CFG_3r, &reg32_val, TOD_MODEf, 3);
    SOCDNX_IF_ERR_EXIT(WRITE_ECI_GLOBAL_GENERAL_CFG_3r(unit, reg32_val));
    
    SOCDNX_IF_ERR_EXIT(READ_ECI_GLOBAL_GENERAL_CFG_4r(unit, &reg32_val));
    soc_reg_field_set(unit, ECI_GLOBAL_GENERAL_CFG_4r, &reg32_val, IPS_DEQ_CMD_RESf, 1);
    soc_reg_field_set(unit, ECI_GLOBAL_GENERAL_CFG_4r, &reg32_val, SINGLE_STAT_PORTf, 0);
    SOCDNX_IF_ERR_EXIT(WRITE_ECI_GLOBAL_GENERAL_CFG_4r(unit, reg32_val));

    /*
     * Voltage Configuration for flow control & syncE
     */
    SOCDNX_IF_ERR_EXIT(READ_ECI_PAD_CONFIGURATION_REGISTERr(unit, &reg32_val));
    if (init->ex_vol_mod == ARAD_MGMT_EXT_VOL_MOD_HSTL_1p8V) {
        soc_reg_field_set(unit, ECI_PAD_CONFIGURATION_REGISTERr, &reg32_val, MODE_HV_2f, 0);
    } else if (init->ex_vol_mod == ARAD_MGMT_EXT_VOL_MOD_3p3V) {
        soc_reg_field_set(unit, ECI_PAD_CONFIGURATION_REGISTERr, &reg32_val, MODE_HV_2f, 1);
    } 
    SOCDNX_IF_ERR_EXIT(WRITE_ECI_PAD_CONFIGURATION_REGISTERr(unit, reg32_val));

exit:
    SOCDNX_FUNC_RETURN;
}

int soc_jer_init_blocks_init_general_conf(int unit)
{
    uint32 reg32_val, preference;
    soc_reg_above_64_val_t reg_above_64_val;

    SOCDNX_INIT_FUNC_DEFS;

    SOCDNX_IF_ERR_EXIT(READ_ECI_GP_CONTROL_9r(unit, reg_above_64_val));
    soc_reg_above_64_field32_set(unit, ECI_GP_CONTROL_9r, reg_above_64_val, PIR_TXI_CREDITS_INIT_VALUEf, 0X0);
    soc_reg_above_64_field32_set(unit, ECI_GP_CONTROL_9r, reg_above_64_val, PIR_RSTNf, 0X1);
    soc_reg_above_64_field32_set(unit, ECI_GP_CONTROL_9r, reg_above_64_val, PIR_OAMP_STRICT_PRIORITYf, 0X0);
    soc_reg_above_64_field32_set(unit, ECI_GP_CONTROL_9r, reg_above_64_val, PIR_EGQ_1_ENABLEf, 0X1);
    soc_reg_above_64_field32_set(unit, ECI_GP_CONTROL_9r, reg_above_64_val, PIR_OAMP_ENABLEf, 0X1);
    soc_reg_above_64_field32_set(unit, ECI_GP_CONTROL_9r, reg_above_64_val, PIR_EGQ_0_RXI_RESET_Nf, 0X1);
    soc_reg_above_64_field32_set(unit, ECI_GP_CONTROL_9r, reg_above_64_val, PIR_EGQ_0_ENABLEf, 0X1);
    soc_reg_above_64_field32_set(unit, ECI_GP_CONTROL_9r, reg_above_64_val, PMH_SYNCE_RSTNf, 0X0);
    soc_reg_above_64_field32_set(unit, ECI_GP_CONTROL_9r, reg_above_64_val, PIR_TXI_CREDITS_INITf, 0X0);
    soc_reg_above_64_field32_set(unit, ECI_GP_CONTROL_9r, reg_above_64_val, PIR_OAMP_RXI_RESET_Nf, 0X1);
    soc_reg_above_64_field32_set(unit, ECI_GP_CONTROL_9r, reg_above_64_val, PIR_EGQ_1_RXI_RESET_Nf, 0X1);
    SOCDNX_IF_ERR_EXIT(WRITE_ECI_GP_CONTROL_9r(unit, reg_above_64_val));

    /* Take NIF TXIs in EGQ out of reset */
    SOCDNX_IF_ERR_EXIT(soc_jer_reset_nif_txi_oor(unit));

    /* 
     * IQM configurations
     */
    reg32_val = 0;
    soc_reg_field_set(unit, IQMT_BDBLL_BANK_SIZESr, &reg32_val, BDB_LIST_SIZE_2f, 0X8);
    soc_reg_field_set(unit, IQMT_BDBLL_BANK_SIZESr, &reg32_val, BDB_LIST_SIZE_3f, 0X8);
    soc_reg_field_set(unit, IQMT_BDBLL_BANK_SIZESr, &reg32_val, BDB_LIST_SIZE_0f, 0X8);
    soc_reg_field_set(unit, IQMT_BDBLL_BANK_SIZESr, &reg32_val, BDB_LIST_SIZE_5f, 0X8);
    soc_reg_field_set(unit, IQMT_BDBLL_BANK_SIZESr, &reg32_val, BDB_LIST_SIZE_4f, 0X8);
    soc_reg_field_set(unit, IQMT_BDBLL_BANK_SIZESr, &reg32_val, BDB_LIST_SIZE_1f, 0X8);
    SOCDNX_IF_ERR_EXIT(WRITE_IQMT_BDBLL_BANK_SIZESr(unit, reg32_val));

    reg32_val = 0;
    soc_reg_field_set(unit, IQMT_IQM_BDB_OFFSETr, &reg32_val, IQM_1_BDB_OFFSETf, 0X0);
    soc_reg_field_set(unit, IQMT_IQM_BDB_OFFSETr, &reg32_val, IQM_0_BDB_OFFSETf, 0X0);
    SOCDNX_IF_ERR_EXIT(WRITE_IQMT_IQM_BDB_OFFSETr(unit, reg32_val));

    reg32_val = 0;
    SOCDNX_IF_ERR_EXIT(soc_dpp_prop_parse_admission_precedence_preference(unit, &preference));
    soc_reg_field_set(unit, IQM_IQM_ENABLERSr, &reg32_val, FLMC_4K_REP_ENf, 0X1);
    soc_reg_field_set(unit, IQM_IQM_ENABLERSr, &reg32_val, FLUSC_CNM_PROT_ENf, 0X0); /* 0x1 */
    soc_reg_field_set(unit, IQM_IQM_ENABLERSr, &reg32_val, VSQ_SIZE_MODEf, 0X1);
    soc_reg_field_set(unit, IQM_IQM_ENABLERSr, &reg32_val, DSCRD_DPf, 0X4);
    soc_reg_field_set(unit, IQM_IQM_ENABLERSr, &reg32_val, IGNORE_DPf, preference);
    soc_reg_field_set(unit, IQM_IQM_ENABLERSr, &reg32_val, DEQ_CACHE_ENH_RR_ENf, 0X1);
    soc_reg_field_set(unit, IQM_IQM_ENABLERSr, &reg32_val, CNGQ_ON_BUFF_ENf, 0X1);
    soc_reg_field_set(unit, IQM_IQM_ENABLERSr, &reg32_val, VSQ_ISP_UPD_ENf, 0X1);
    soc_reg_field_set(unit, IQM_IQM_ENABLERSr, &reg32_val, SET_TDM_Q_PER_QSIGNf, 0X1);
    soc_reg_field_set(unit, IQM_IQM_ENABLERSr, &reg32_val, EN_IPT_CD_4_SNOOPf, 0X1);
    soc_reg_field_set(unit, IQM_IQM_ENABLERSr, &reg32_val, FWD_ACT_SELf, 0X0); /* 0x1 */
    soc_reg_field_set(unit, IQM_IQM_ENABLERSr, &reg32_val, VSQ_TH_MODE_SELf, 0X1);
    soc_reg_field_set(unit, IQM_IQM_ENABLERSr, &reg32_val, PDM_INIT_ENf, 0X1);
    soc_reg_field_set(unit, IQM_IQM_ENABLERSr, &reg32_val, ISP_CD_SCND_CP_ENf, 0X1);
    soc_reg_field_set(unit, IQM_IQM_ENABLERSr, &reg32_val, MNUSC_CNM_PROT_ENf, 0X0); /* 0x1 */
    soc_reg_field_set(unit, IQM_IQM_ENABLERSr, &reg32_val, CRPS_CMD_ENf, 0X0); /* 0x1 */
    soc_reg_field_set(unit, IQM_IQM_ENABLERSr, &reg32_val, DSCRD_ALL_PKTf, 0x0);
    SOCDNX_IF_ERR_EXIT(WRITE_IQM_IQM_ENABLERSr(unit, SOC_CORE_ALL, reg32_val));

exit:
    SOCDNX_FUNC_RETURN;
}

int soc_jer_init_blocks_init_conf(int unit)
{

    SOCDNX_INIT_FUNC_DEFS;

    /* ECI Globals configurations */
    SOCDNX_IF_ERR_EXIT(soc_jer_init_blocks_init_global_conf(unit));

    /* General blocks configurations */
    SOCDNX_IF_ERR_EXIT(soc_jer_init_blocks_init_general_conf(unit));


    /* Configure OCB and Dram Buffers */
    SOCDNX_IF_ERR_EXIT(soc_jer_ocb_dram_buffer_conf_set(unit));
    SOCDNX_IF_ERR_EXIT(soc_jer_ocb_conf_set(unit));

exit:
    SOCDNX_FUNC_RETURN;
}

int soc_jer_init_blocks_init(int unit)
{

    SOCDNX_INIT_FUNC_DEFS;

    /* exclude irrelevant blocks which are missing due to partial emul compilation - controlled via custom soc property */
    if (soc_property_suffix_num_get(unit, -1, spn_CUSTOM_FEATURE, "exclude_blocks_missing_in_partial_emul_compilation", 0)) {
        soc_jer_init_exclude_blocks(unit);
    }

    /* access check to blocks after reset*/
    SOCDNX_IF_ERR_EXIT(soc_jer_regs_blocks_access_check(unit));

    /* Init blocks' broadcast IDs */
    SOCDNX_IF_ERR_EXIT(soc_jer_init_brdc_blk_id_set(unit));

    /* Set System FAP ID */
    SOCDNX_IF_ERR_EXIT(MBCM_DPP_DRIVER_CALL(unit, mbcm_dpp_mgmt_system_fap_id_set,(unit, 0))); /* fap id set to 0, can be overwritten later by using API */

   /* set indirect Write mask to allow dynamic access and initialize tables */
    SOCDNX_IF_ERR_EXIT(soc_jer_write_masks_set(unit));


exit:
    SOCDNX_FUNC_RETURN;
}

/*
 * Function:
 *      soc_jer_init_bubble_config_standard_blocks
 * Purpose:
 *      makes all the needed configurations for blocks with the common bubble mechanism
 * Parameters:
 *      unit -  unit number
 *  
 * Returns:
 *      SOC_E_XXX
 */
int soc_jer_init_bubble_config_standard_blocks (int unit)
{
    uint32 reg = 0;
    int iter = 0;

    SOCDNX_INIT_FUNC_DEFS;

    if(soc_property_suffix_num_get(unit, -1, spn_CUSTOM_FEATURE, "bubble_cpu", 1) == 1) {
        /* iterate over bubble registers array and act */
        while (jer_blocks_with_standart_bubble_mechanism[iter] != INVALIDr) {

            /* set block indirect command timeout period */
            SOCDNX_IF_ERR_EXIT(soc_reg_field32_modify(unit, jer_blocks_with_standart_bubble_mechanism[iter++], REG_PORT_ANY, INDIRECT_COMMAND_TIMEOUTf, 0x10));

            /* set block bubble mechanism */
            reg = 0;
            SOCDNX_IF_ERR_EXIT(soc_reg32_get(unit, jer_blocks_with_standart_bubble_mechanism[iter], REG_PORT_ANY, 0, &reg));
            soc_reg_field_set (unit, jer_blocks_with_standart_bubble_mechanism[iter], &reg, FORCE_BUBBLE_PERIODf, 0x8);
            soc_reg_field_set (unit, jer_blocks_with_standart_bubble_mechanism[iter], &reg, FORCE_BUBBLE_ENf, 0x1);
            SOCDNX_IF_ERR_EXIT(soc_reg32_set(unit, jer_blocks_with_standart_bubble_mechanism[iter++], REG_PORT_ANY, 0, reg));

        }
    }
exit:
    SOCDNX_FUNC_RETURN;
}

/*
 * Function:
 *      soc_jer_init_bubble_config_specific_egr
 * Purpose:
 *      special configurations needed for egr
 * Parameters:
 *      unit -  unit number
 *  
 * Returns:
 *      SOC_E_XXX
 * NOTES: 
 *      if the need will arise, this is the way a special bubble configuration function should look like
 */
int soc_jer_init_bubble_config_specific_egr (int unit)
{
    uint32 conf_reg_val;

    SOCDNX_INIT_FUNC_DEFS;

    /* by default bubbles are disabled */
    if(soc_property_suffix_num_get(unit, -1, spn_CUSTOM_FEATURE, "bubble_egr", 0) == 1) {
        /* EGQ_FQP_BUBBLE_CONFIGURATION configuration - used to create bubble in EPNI block */
        SOCDNX_IF_ERR_EXIT(READ_EGQ_FQP_BUBBLE_CONFIGURATIONr(unit, REG_PORT_ANY, &conf_reg_val));
        /* Enable constant bubble generation every 32*BubbleDelay clocks.*/
        soc_reg_field_set(unit, EGQ_FQP_BUBBLE_CONFIGURATIONr, &conf_reg_val, FQP_CONST_BUBBLE_ENf, 1);
        /* The minimum delay between the bubble request to the bubble */
        soc_reg_field_set(unit, EGQ_FQP_BUBBLE_CONFIGURATIONr, &conf_reg_val, FQP_BUBBLE_DELAYf, 31);
        SOCDNX_IF_ERR_EXIT(WRITE_EGQ_FQP_BUBBLE_CONFIGURATIONr(unit, SOC_CORE_ALL, conf_reg_val));
    }
exit:
    SOCDNX_FUNC_RETURN;
}

/*
 * Function:
 *      soc_jer_init_bubble_config_specific_ing
 * Purpose:
 *      special configurations needed for ingress
 * Parameters:
 *      unit -  unit number
 *  
 * Returns:
 *      SOC_E_XXX
 * NOTES: 
 *      if the need will arise, this is the way a special bubble configuration function should look like
 */
int soc_jer_init_bubble_config_specific_ing (int unit)
{
    SOCDNX_INIT_FUNC_DEFS;

    if(soc_property_suffix_num_get(unit, -1, spn_CUSTOM_FEATURE, "bubble_ing", 1) == 1) {
        /* limit packet rate by bubbles injected every SyncCounter clock cycles. */
        SOCDNX_IF_ERR_EXIT(soc_reg_above_64_field32_modify(unit, IHP_IHP_ENABLERSr, SOC_CORE_ALL, 0, FORCE_BUBBLESf,  1));
        /* every SyncCounter number of clocks a bubble will be inserted to the IHP pipe */
        SOCDNX_IF_ERR_EXIT(soc_reg_above_64_field32_modify(unit, IHP_SYNC_COUNTERr, SOC_CORE_ALL, 0, SYNC_COUNTERf,  0x3ff));
    }
exit:
    SOCDNX_FUNC_RETURN;
}

/*
 * Function:
 *      soc_jer_init_bubble_configuration
 * Purpose:
 *      configures the bubble configuration for all relevant blocks
 *      bubble - a mechanism to allow cpu to access S-BUS, even though the design has priority over it.
 *               the method of doing it is forcing a "bubble" of inactivity to allow the cpu transaction enter.
 * Parameters:
 *      unit -  unit number
 *  
 * Returns:
 *      SOC_E_XXX
 */
int soc_jer_init_bubble_config (int unit)
{
    SOCDNX_INIT_FUNC_DEFS;

    /* Config bubble mechanism for blocks with the common mechanism */
    SOCDNX_IF_ERR_EXIT(soc_jer_init_bubble_config_standard_blocks(unit));

    /* specific blocks bubble configuration - currently, no such functions */
    SOCDNX_IF_ERR_EXIT(soc_jer_init_bubble_config_specific_egr(unit));
    SOCDNX_IF_ERR_EXIT(soc_jer_init_bubble_config_specific_ing(unit));
    
exit:
    SOCDNX_FUNC_RETURN;
}

/*
 * Function:
 *      soc_jer_write_masks_set
 * Purpose:
 *      set indirect Write mask to allow dynamic access and initialize tables
 * Parameters:
 *      unit -  unit number
 *  
 * Returns:
 *      SOC_E_XXX
 */
int soc_jer_write_masks_set (int unit)
{
    uint32
     wr_mask_val;
    SOCDNX_INIT_FUNC_DEFS;

    wr_mask_val = 0xffffffff;
    SOCDNX_IF_ERR_EXIT(WRITE_PPDB_A_INDIRECT_WR_MASKr(unit, wr_mask_val)); 

    SOCDNX_IF_ERR_EXIT(WRITE_PPDB_B_INDIRECT_WR_MASKr(unit, wr_mask_val)); 

    SOCDNX_IF_ERR_EXIT(WRITE_IHB_INDIRECT_WR_MASKr(unit, SOC_CORE_ALL,wr_mask_val)); 

    SOCDNX_IF_ERR_EXIT(WRITE_IHP_INDIRECT_WR_MASKr(unit,SOC_CORE_ALL, wr_mask_val)); 

    SOCDNX_IF_ERR_EXIT(WRITE_SCH_INDIRECT_WR_MASKr_REG32(unit,SOC_CORE_ALL, wr_mask_val));     

    if (SOC_IS_QUX(unit)) {
        soc_reg_above_64_val_t reg_data;
        soc_reg_t indirect_wr_mask_reg[] = {
                                         EDB_INDIRECT_WR_MASKr,
                                         EPNI_INDIRECT_WR_MASKr, 
                                         CGM_INDIRECT_WR_MASKr,
                                         KAPS_INDIRECT_WR_MASKr,
                                         OAMP_INDIRECT_WR_MASKr
                                      };
        uint32   i;

        reg_data[0] = 0xffffffff;
        reg_data[1] = 0xffffffff;
        reg_data[2] = 0xffffffff;

        for(i = 0; i < sizeof(indirect_wr_mask_reg)/sizeof(soc_reg_t); i++) {
           SOCDNX_IF_ERR_EXIT(soc_reg_above_64_set(unit, indirect_wr_mask_reg[i], REG_PORT_ANY,0,reg_data));
        }

    }
    
exit:
    SOCDNX_FUNC_RETURN;
}

int soc_jer_init_blocks_general(int unit)
{

    SOCDNX_INIT_FUNC_DEFS;

    /* access check to blocks after reset*/
    SOCDNX_IF_ERR_EXIT(soc_jer_ocb_dram_buffer_autogen_set(unit));

    /* Set bubble configuration */
    SOCDNX_IF_ERR_EXIT(soc_jer_init_bubble_config(unit));

    /* Setting revision fixes Bits */
    SOCDNX_IF_ERR_EXIT(jer_mgmt_revision_fixes(unit));

    /* Enable pvt monitor*/
    SOCDNX_IF_ERR_EXIT(jer_mgmt_drv_pvt_monitor_enable(unit));

exit:
    SOCDNX_FUNC_RETURN;
}

#if defined(INCLUDE_KBP) && !defined(BCM_88030)
uint32 jer_kbp_init_jer_interface(int unit)
{
    uint32 offset;
    soc_port_t ilkn_port;
    uint32 reg_val, ilkn_num_lanes, ilkn_metaframe, copy_no;
    int ilkn_rate, core;
    soc_reg_t reg, rst_reg;
    soc_field_t tx_field;
    uint32 one_kbp_ilkn_at_least = 0;
    uint32 res = SOC_SAND_OK, rv = SOC_E_NONE;

    SOCDNX_INIT_FUNC_DEFS;

    SOC_DPP_CORES_ITER(_SHR_CORE_ALL, core) {
        res = arad_kbp_ilkn_interface_param_get(unit, core, &ilkn_port, &ilkn_num_lanes, &ilkn_rate, &ilkn_metaframe);
        soc_sand_update_error_code(res, &rv);
        if (rv == SOC_E_NONE) {
            one_kbp_ilkn_at_least = 1;
        } else {
            continue;
        }
              
        SOCDNX_IF_ERR_EXIT(soc_port_sw_db_protocol_offset_get(unit, ilkn_port, 0, &offset));
        if (SOC_IS_QAX(unit)) {
            switch (offset) {
                case 0:
                    reg = NBIH_ELK_CFGr;
                    rst_reg = NBIH_ADDITIONAL_RESETSr;
                    copy_no = 0;
                    tx_field = ELK_ON_ILKN_PORT_0_ELK_Nf;
                    break;
                case 1:
                    reg = NBIL_ELK_CFGr;
                    rst_reg = NBIL_ADDITIONAL_RESETSr;
                    copy_no = 0;
                    tx_field = ELK_ON_ILKN_ELK_Nf;
                    break;
                case 2:
                    reg = NBIH_ELK_CFGr;
                    rst_reg = NBIH_ADDITIONAL_RESETSr;
                    copy_no = 0;
                    tx_field = ELK_ON_ILKN_PORT_2_ELK_Nf;
                    break;
                case 3:
                    reg = NBIL_ELK_CFGr;
                    rst_reg = NBIL_ADDITIONAL_RESETSr;
                    copy_no = 1;
                    tx_field = ELK_ON_ILKN_ELK_Nf;
                    break;
                default:
                    SOCDNX_EXIT_WITH_ERR(SOC_E_INTERNAL, (_BSL_SOC_MSG("invalid ilkn number, %u"), offset));
            }
        } else {
            switch (offset) {
                case 0:
                    reg = NBIH_ELK_CFGr;
                    rst_reg = NBIH_ADDITIONAL_RESETSr;
                    copy_no = 0;
                    tx_field = ELK_ON_ILKN_PORT_0_ELK_Nf;
                    break;
                case 1:
                    reg = NBIH_ELK_CFGr;
                    rst_reg = NBIH_ADDITIONAL_RESETSr;
                    copy_no = 0;
                    tx_field = ELK_ON_ILKN_PORT_1_ELK_Nf;
                    break;
                case 2:
                    reg = NBIL_ELK_CFGr;
                    rst_reg = NBIL_ADDITIONAL_RESETSr;
                    copy_no = 0;
                    tx_field = ELK_ON_ILKN_PORT_0_ELK_Nf;
                    break;
                case 3:
                    reg = NBIL_ELK_CFGr;
                    rst_reg = NBIL_ADDITIONAL_RESETSr;
                    copy_no = 0;
                    tx_field = ELK_ON_ILKN_PORT_1_ELK_Nf;
                    break;
                case 4:
                    reg = NBIL_ELK_CFGr;
                    rst_reg = NBIL_ADDITIONAL_RESETSr;
                    copy_no = 1;
                    tx_field = ELK_ON_ILKN_PORT_0_ELK_Nf;
                    break;
                case 5:
                    reg = NBIL_ELK_CFGr;
                    rst_reg = NBIL_ADDITIONAL_RESETSr;
                    copy_no = 1;
                    tx_field = ELK_ON_ILKN_PORT_1_ELK_Nf;
                    break;
                default:
                    SOCDNX_EXIT_WITH_ERR(SOC_E_INTERNAL, (_BSL_SOC_MSG("invalid ilkn number, %u"), offset));
            }
        }

        /* preoper ELK tx enable: ELK_CFG */
        SOCDNX_IF_ERR_EXIT(soc_reg32_get(unit, reg, copy_no, core, &reg_val)); 
        soc_reg_field_set(unit, reg, &reg_val, tx_field, 1);
        SOCDNX_IF_ERR_EXIT(soc_reg32_set(unit, reg, copy_no, core, reg_val));
          
        /* preoper ELK rx enable: ELK_CFG */
        SOCDNX_IF_ERR_EXIT(READ_NBIH_ELK_CFGr(unit, core, &reg_val));
        if (SOC_IS_QAX(unit)) {
            switch (offset) {
                case 0:
                case 2:
                    soc_reg_field_set(unit, NBIH_ELK_CFGr, &reg_val, ELK_RX_ACTIVE_NBIH_ELK_Nf, 1);
                    break;
                case 1:
                    soc_reg_field_set(unit, NBIH_ELK_CFGr, &reg_val, ELK_RX_ACTIVE_NBIL_0_ELK_Nf, 1);
                    break;
                case 3:
                    soc_reg_field_set(unit, NBIH_ELK_CFGr, &reg_val, ELK_RX_ACTIVE_NBIL_1_ELK_Nf, 1);
                    break;
                default:
                    SOCDNX_EXIT_WITH_ERR(SOC_E_INTERNAL, (_BSL_SOC_MSG("invalid ilkn number, %u"), offset));
            }
        } else {
            switch (offset/2) {
                case 0:
                    soc_reg_field_set(unit, NBIH_ELK_CFGr, &reg_val, ELK_RX_ACTIVE_NBIH_ELK_Nf, 1);
                    break;
                case 1:
                    soc_reg_field_set(unit, NBIH_ELK_CFGr, &reg_val, ELK_RX_ACTIVE_NBIL_0_ELK_Nf, 1);
                    break;
                case 2:
                    soc_reg_field_set(unit, NBIH_ELK_CFGr, &reg_val, ELK_RX_ACTIVE_NBIL_1_ELK_Nf, 1);
                    break;
                default:
                    SOCDNX_EXIT_WITH_ERR(SOC_E_INTERNAL, (_BSL_SOC_MSG("invalid ilkn number, %u"), offset));
            }
        }
        SOCDNX_IF_ERR_EXIT(WRITE_NBIH_ELK_CFGr(unit, core, reg_val));

        /* proper ELK out of reset: ADDITIONAL_RESETS */
        SOCDNX_IF_ERR_EXIT(soc_reg32_get(unit, rst_reg, copy_no, 0, &reg_val)); 
        if (core == 0) {
            soc_reg_field_set(unit, rst_reg, &reg_val, ELK_0_RX_RSTNf, 1); 
            soc_reg_field_set(unit, rst_reg, &reg_val, ELK_0_TX_RSTNf, 1);
        } else {
            soc_reg_field_set(unit, rst_reg, &reg_val, ELK_1_RX_RSTNf, 1);
            soc_reg_field_set(unit, rst_reg, &reg_val, ELK_1_TX_RSTNf, 1);
        }
        SOCDNX_IF_ERR_EXIT(soc_reg32_set(unit, rst_reg, copy_no, 0, reg_val));
    }

    if (one_kbp_ilkn_at_least == 0) {
        SOCDNX_EXIT_WITH_ERR(SOC_E_INTERNAL, (_BSL_SOC_MSG("invalid ilkn number")));
    }

exit:
    SOCDNX_FUNC_RETURN;
}
#endif /* #if defined(INCLUDE_KBP) && !defined(BCM_88030) */

int soc_jer_init_hw_interfaces_set(int unit)
{
    CONST ARAD_MGMT_INIT *init = NULL;
    SOCDNX_INIT_FUNC_DEFS;
    SOCDNX_PCID_LITE_SKIP(unit);

    init = &SOC_DPP_CONFIG(unit)->arad->init;
    if (init->drc_info.enable) { 
        /* Init Dram */
        SOCDNX_IF_ERR_EXIT(soc_dpp_drc_combo28_dram_init(unit, &SOC_DPP_CONFIG(unit)->arad->init.drc_info));

        /* Init Dram Recovery Mechanism */
        SOCDNX_IF_ERR_EXIT(MBCM_DPP_DRIVER_CALL(unit, mbcm_dpp_dram_recovery_init,(unit)));
    }
#if defined(INCLUDE_KBP) && !defined(BCM_88030)
    if(init->elk.enable) {
        SOCDNX_IF_ERR_EXIT(arad_kbp_init_arad_interface(unit));
        SOCDNX_IF_ERR_EXIT(jer_kbp_init_jer_interface(unit));
    } 
#endif /* #if defined(INCLUDE_KBP) && !defined(BCM_88030) */

exit:
    SOCDNX_FUNC_RETURN;
}

/* Port initialization */
STATIC int soc_jer_init_port(int unit)
{
    uint32 soc_sand_rv = 0;

    SOCDNX_INIT_FUNC_DEFS;
    soc_sand_rv = arad_mgmt_pon_init(unit);
    SOCDNX_SAND_IF_ERR_EXIT(soc_sand_rv);  
exit:
    SOCDNX_FUNC_RETURN;
}

STATIC int soc_jer_init_mesh_topology(int unit)
{
    uint32 soc_sand_rv = 0, reg_val, field_val = 0, gt_size = -1;
    ARAD_INIT_FABRIC *fabric;
    SOCDNX_INIT_FUNC_DEFS;

    fabric = &(SOC_DPP_CONFIG(unit)->arad->init.fabric);

    soc_sand_rv = arad_init_mesh_topology(unit);
    SOCDNX_SAND_IF_ERR_EXIT(soc_sand_rv);

    if(fabric->is_128_in_system) {
        field_val = 0x1;
    } else if (fabric->system_contains_multiple_pipe_device) {
        field_val = 0x2;
    } else {
        field_val = 0x0;
    }

    
    gt_size = soc_property_suffix_num_get(unit,0,spn_CUSTOM_FEATURE, "mesh_topology_size", field_val);

    switch (gt_size) {
    case 0:
        SOCDNX_IF_ERR_EXIT(READ_MESH_TOPOLOGY_MESH_TOPOLOGYr(unit, &reg_val)); 
        soc_reg_field_set(unit, MESH_TOPOLOGY_MESH_TOPOLOGYr, &reg_val, RESERVED_2f, gt_size);
        SOCDNX_IF_ERR_EXIT(WRITE_MESH_TOPOLOGY_MESH_TOPOLOGYr(unit, reg_val));
        break;
    case 1:
        SOCDNX_IF_ERR_EXIT(READ_MESH_TOPOLOGY_MESH_TOPOLOGYr(unit, &reg_val)); 
        soc_reg_field_set(unit, MESH_TOPOLOGY_MESH_TOPOLOGYr, &reg_val, RESERVED_2f, gt_size);
        SOCDNX_IF_ERR_EXIT(WRITE_MESH_TOPOLOGY_MESH_TOPOLOGYr(unit, reg_val));
        break;
    case 2:
        SOCDNX_IF_ERR_EXIT(READ_MESH_TOPOLOGY_MESH_TOPOLOGYr(unit, &reg_val)); 
        soc_reg_field_set(unit, MESH_TOPOLOGY_MESH_TOPOLOGYr, &reg_val, RESERVED_2f, gt_size);
        SOCDNX_IF_ERR_EXIT(WRITE_MESH_TOPOLOGY_MESH_TOPOLOGYr(unit, reg_val));  
        break;
    case 3:
        SOCDNX_IF_ERR_EXIT(READ_MESH_TOPOLOGY_MESH_TOPOLOGYr(unit, &reg_val)); 
        soc_reg_field_set(unit, MESH_TOPOLOGY_MESH_TOPOLOGYr, &reg_val, RESERVED_2f, gt_size);
        SOCDNX_IF_ERR_EXIT(WRITE_MESH_TOPOLOGY_MESH_TOPOLOGYr(unit, reg_val));
        break;
    default:
        SOCDNX_EXIT_WITH_ERR(SOC_E_CONFIG, (_BSL_SOC_MSG("GT size %d in invalid"), gt_size));
        break;
    }
    SOCDNX_IF_ERR_EXIT(READ_MESH_TOPOLOGY_REG_0117r(unit, &reg_val));
    
    soc_reg_field_set(unit, MESH_TOPOLOGY_REG_0117r, &reg_val, FIELD_4_8f, 0xc);
    soc_reg_field_set(unit, MESH_TOPOLOGY_REG_0117r, &reg_val, FIELD_9_9f, SOC_DPP_CONFIG(unit)->arad->init.fabric.fabric_mesh_topology_fast ? 1 : 0);
    SOCDNX_IF_ERR_EXIT(WRITE_MESH_TOPOLOGY_REG_0117r(unit, reg_val));


exit:
    SOCDNX_FUNC_RETURN;
}

STATIC int soc_jer_plus_init_overrides(int unit)
{
 

    SOCDNX_INIT_FUNC_DEFS;


    SOCDNX_IF_ERR_EXIT(WRITE_EDB_ENABLE_DYNAMIC_MEMORY_ACCESSr(unit, 1));
    SOCDNX_IF_ERR_EXIT(WRITE_EGQ_ENABLE_DYNAMIC_MEMORY_ACCESSr(unit, REG_PORT_ANY, 1));
    SOCDNX_IF_ERR_EXIT(WRITE_EPNI_ENABLE_DYNAMIC_MEMORY_ACCESSr(unit, REG_PORT_ANY, 1));
    SOCDNX_IF_ERR_EXIT(WRITE_IHB_ENABLE_DYNAMIC_MEMORY_ACCESSr(unit, REG_PORT_ANY, 1));
    SOCDNX_IF_ERR_EXIT(WRITE_IHP_ENABLE_DYNAMIC_MEMORY_ACCESSr(unit, REG_PORT_ANY, 1));
    SOCDNX_IF_ERR_EXIT(WRITE_OAMP_ENABLE_DYNAMIC_MEMORY_ACCESSr(unit, 1));
    SOCDNX_IF_ERR_EXIT(WRITE_OLP_ENABLE_DYNAMIC_MEMORY_ACCESSr(unit, 1));
    SOCDNX_IF_ERR_EXIT(WRITE_PPDB_A_ENABLE_DYNAMIC_MEMORY_ACCESSr(unit, 1));
    SOCDNX_IF_ERR_EXIT(WRITE_PPDB_B_ENABLE_DYNAMIC_MEMORY_ACCESSr(unit, 1));
    SOCDNX_IF_ERR_EXIT(WRITE_KAPS_BBS_ENABLE_DYNAMIC_MEMORY_ACCESSr(unit, 1));
    SOCDNX_IF_ERR_EXIT(WRITE_KAPS_ENABLE_DYNAMIC_MEMORY_ACCESSr(unit, 1));

exit:
    SOCDNX_FUNC_RETURN;
}

/* Modules initialization */
STATIC int soc_jer_init_functional_init(int unit)
{
    soc_pbmp_t pbmp;
    soc_port_t port_i;
    ARAD_MGMT_INIT* init;
    /* Temp variables for QAX init*/
    uint32 flags;
    int i = 0, index = 0;
    uint32 entry[20];
    int cpu_pp_ports[]  =     {0,     200,   201,   202,   203};
    int cpu_sys_ports[] =     {0xff,  0xc8,  0xc9,  0xca,  0xcb};
    /* Temp variables for QAX init*/
    uint32 soc_sand_rv = 0, rv = 0;    
    uint32 reg32_val;
    uint32 core_freq;
    uint32 slow_port_freq;
    uint32 bus_width;
    uint32 reassembly_timeout;

    SOCDNX_INIT_FUNC_DEFS;

    init = &(SOC_DPP_CONFIG(unit)->arad->init);

    if (SOC_IS_QAX(unit)) {
        if (SOC_DPP_CONFIG(unit)->emulation_system != 3) { /* QAX overrides */
            SOCDNX_IF_ERR_EXIT(soc_qax_enable_dynamic_memories(unit));
        }

        if (SOC_DPP_CONFIG(unit)->emulation_system != 0) { /* QAX define share overrides, emulation only */
            SOCDNX_IF_ERR_EXIT(soc_qax_init_dpp_defs_overrides(unit));
        }
    }
    else if (SOC_IS_JERICHO_PLUS(unit)) {
        if (SOC_DPP_CONFIG(unit)->emulation_system != 3) { /* Jericho+ overrides */
            SOCDNX_IF_ERR_EXIT(soc_jer_plus_init_overrides(unit));
        }
    }

    soc_sand_rv = arad_pmf_low_level_init_unsafe(unit);
    SOCDNX_SAND_IF_ERR_EXIT(soc_sand_rv);

    SOCDNX_IF_ERR_EXIT(MBCM_DPP_DRIVER_CALL(unit, mbcm_dpp_ipt_init,(unit)));

    if (!SOC_IS_QAX(unit) || SOC_IS_QAX_WITH_FABRIC_ENABLED(unit))
    {
        rv = soc_jer_init_mesh_topology(unit);
        SOCDNX_SAND_IF_ERR_EXIT(soc_sand_rv);

        rv = soc_jer_fabric_init(unit);
        SOCDNX_IF_ERR_EXIT(rv);
    }


    /*initialize dynamic reassembly mode*/
    SOC_DPP_CORES_ITER(SOC_CORE_ALL, i) {
        SOCDNX_IF_ERR_EXIT(READ_EGQ_GENERAL_RQP_CONFIGr(unit, i, &reg32_val));
        soc_reg_field_set(unit, EGQ_GENERAL_RQP_CONFIGr, &reg32_val, RCM_MODEf, 2);
        if (!SOC_IS_QUX(unit)) {
            soc_reg_field_set(unit, EGQ_GENERAL_RQP_CONFIGr, &reg32_val, RCM_TDM_RESERVEDf, 0);
        }
        SOCDNX_IF_ERR_EXIT(WRITE_EGQ_GENERAL_RQP_CONFIGr(unit, i, reg32_val));
    }

    soc_sand_rv = arad_parser_init(unit);
    SOCDNX_SAND_IF_ERR_EXIT(soc_sand_rv);    
    DCMN_RUNTIME_DEBUG_PRINT_LOC(unit, "soc_jer_init_functional_init before arad_ports_init");

    soc_sand_rv = arad_ports_init(unit);
    SOCDNX_SAND_IF_ERR_EXIT(soc_sand_rv);
    DCMN_RUNTIME_DEBUG_PRINT_LOC(unit, "soc_jer_init_functional_init after arad_ports_init");

    if (SOC_IS_QAX(unit)) {
        /* temporary code to set system port for cpu port */
        for (i = 0; i < 5; i++) {
            index = 0x7e00 + cpu_pp_ports[i];
            SOCDNX_IF_ERR_EXIT(soc_mem_read(unit, IHP_VIRTUAL_PORT_TABLEm, MEM_BLOCK_ALL, index, entry));
            soc_mem_field32_set(unit, IHP_VIRTUAL_PORT_TABLEm, entry, VIRTUAL_PORT_SRC_SYS_PORTf, cpu_sys_ports[i]);
            SOCDNX_IF_ERR_EXIT(soc_mem_write(unit, IHP_VIRTUAL_PORT_TABLEm, MEM_BLOCK_ALL, index, entry));

            index = 0x7f00 + cpu_pp_ports[i];
            SOCDNX_IF_ERR_EXIT(soc_mem_read(unit, IHP_VIRTUAL_PORT_TABLEm, MEM_BLOCK_ALL, index, entry));
            soc_mem_field32_set(unit, IHP_VIRTUAL_PORT_TABLEm, entry, VIRTUAL_PORT_SRC_SYS_PORTf, cpu_sys_ports[i]);
            SOCDNX_IF_ERR_EXIT(soc_mem_write(unit, IHP_VIRTUAL_PORT_TABLEm, MEM_BLOCK_ALL, index, entry));
        }
    }

    soc_sand_rv = soc_jer_trunk_init(unit);
    SOCDNX_SAND_IF_ERR_EXIT(soc_sand_rv);  

    soc_sand_rv = arad_egr_prog_editor_unsafe(unit);
    SOCDNX_SAND_IF_ERR_EXIT(soc_sand_rv);  

    soc_sand_rv = soc_jer_lbp_init(unit); 
    SOCDNX_SAND_IF_ERR_EXIT(soc_sand_rv); 
    if (SOC_DPP_CONFIG(unit)->arad->init.pp_enable == TRUE) {
        rv = soc_jer_lif_init(unit);
        SOCDNX_IF_ERR_EXIT(rv);
    }

    soc_sand_rv = jer_tdm_init(unit);
    SOCDNX_SAND_IF_ERR_EXIT(soc_sand_rv);

    rv = jer_flow_control_init(unit);
    SOCDNX_IF_ERR_EXIT(rv);

    if (!SOC_IS_QAX(unit))
    {
        if (SOC_DPP_CONFIG(unit)->arad->init.pp_enable == FALSE) {
            soc_sand_rv = arad_pp_isem_access_init_unsafe(unit);
            SOCDNX_SAND_IF_ERR_EXIT(soc_sand_rv);

            /* FLP - init only if TM */
            arad_pp_flp_prog_sel_cam_key_program_tm(unit);
            arad_pp_flp_process_key_program_tm(unit);
        }
    }

    if (SOC_DPP_CONFIG(unit)->arad->init.pp_enable == FALSE) {
        soc_sand_rv = arad_pp_trap_mgmt_init_unsafe(unit);
        SOCDNX_SAND_IF_ERR_EXIT(soc_sand_rv);
    }


    /* Init MRU */
    SOCDNX_IF_ERR_EXIT(jer_mgmt_set_mru_by_dbuff_size(unit));


    SOCDNX_IF_ERR_EXIT(soc_port_sw_db_valid_ports_get(unit, 0, &pbmp));
    SOC_PBMP_ITER(pbmp, port_i) {
        if (SOC_IS_QAX(unit)) {
            SOCDNX_IF_ERR_EXIT(soc_port_sw_db_flags_get(unit, port_i, &flags));
            if (SOC_PORT_IS_LB_MODEM(flags)) {
                continue;
            }
        }
        SOCDNX_IF_ERR_EXIT(arad_ports_header_type_update(unit, port_i));
    }
    DCMN_RUNTIME_DEBUG_PRINT_LOC(unit, "soc_jer_init_functional_init after arad_ports_header_type_update per port");

    /*
     *  In case ERP port is enabled, search for unoccupied NIF interface
     */
    SOCDNX_SAND_IF_ERR_EXIT(arad_ports_init_interfaces_erp_setting(unit, &(init->ports)));

    /* 
     * Ingress reassembly error happens when big packets are sent on slow port
     * Set reassembly timeout such that it is big enough for the slowest port
    */
    core_freq = SOC_DPP_CONFIG(unit)->arad->init.core_freq.frequency/1000; /* kHz -> MHz*/
    bus_width = 256;
    slow_port_freq = 100; /* Mbit/sec */
    reassembly_timeout = bus_width * core_freq * 8 /slow_port_freq;

    if (SOC_IS_QAX(unit)) {
        SOCDNX_IF_ERR_EXIT(WRITE_SPB_REASSEMBLY_TIMEOUTr(unit, reassembly_timeout));
    } 
    else {
        SOCDNX_IF_ERR_EXIT(WRITE_IDR_REASSEMBLY_TIMEOUTr(unit, reassembly_timeout));
    }


    if (SOC_IS_QAX(unit))
    {
#ifdef BCM_LB_SUPPORT
        if (SOC_DPP_CONFIG(unit)->qax->link_bonding_enable) {
            rv = qax_lb_init(unit);
            SOCDNX_IF_ERR_EXIT(rv);
        }
#endif

        SOC_EXIT;
    }

    /* Counters Select Init */
    {
        uint32 reg32 = 0;
        soc_reg_field_set(unit, IPT_CFG_EVENT_CNT_SELr, &reg32, CFG_EVENT_CNT_SELf, 5);
        SOCDNX_IF_ERR_EXIT(WRITE_IPT_CFG_EVENT_CNT_SELr(unit, reg32));

        reg32=0;
        soc_reg_field_set(unit, IPT_CFG_BYTE_CNT_SRC_SELr, &reg32, CFG_BYTE_CNT_SRC_SELf, 1);
        SOCDNX_IF_ERR_EXIT(WRITE_IPT_CFG_BYTE_CNT_SRC_SELr(unit, reg32));
    }


exit:
    SOCDNX_FUNC_RETURN;
}


/*
 * Function:
 *      soc_jer_pp_mgmt_functional_init
 * Purpose:
 *      Perform various module initalization for PP modules that require
 *      Jericho specific configuration
 *      The function is called from arad_pp_mgmt_functional_init_unsafe.
 * Parameters:
 *      unit    - Device Number
 * Returns:
 *      SOC_E_XXX
 */
soc_error_t soc_jer_pp_mgmt_functional_init(
    int unit)
{
    uint32 soc_sand_rv = 0;
    uint32 reg32;
    int rv;
    uint8 non_first_fragments_enable;

    SOCDNX_INIT_FUNC_DEFS;    
        
    reg32 = 0;
    soc_reg_field_set(unit, IHP_IHP_ENABLERSr, &reg32, ENABLE_DATA_PATHf, 1);
    soc_reg_field_set(unit, IHP_IHP_ENABLERSr, &reg32, ENABLE_TCAM_BUBBLESf, 1);
    soc_reg_field_set(unit, IHP_IHP_ENABLERSr, &reg32, ENABLE_MACT_BUBBLESf, 1);

    non_first_fragments_enable = (soc_property_get(unit, spn_FIELD_IP_FIRST_FRAGMENT_PARSED, TRUE)) ? 0x1 : 0x0;
    soc_reg_field_set(unit, IHP_IHP_ENABLERSr, &reg32, NON_FIRST_FRAGMENTS_ENABLEf, non_first_fragments_enable);
    soc_reg_field_set(unit, IHP_IHP_ENABLERSr, &reg32, MAX_HEADER_STACKf, 5);
    soc_reg_field_set(unit, IHP_IHP_ENABLERSr, &reg32, ADDITIONAL_TPID_ENABLEf, 1);
    soc_reg_field_set(unit, IHP_IHP_ENABLERSr, &reg32, LLC_CHECK_LSBf, 1);
    soc_reg_field_set(unit, IHP_IHP_ENABLERSr, &reg32, IP_NEXT_PROTOCOL_GRE_ETHERNETf, 1);
    SOCDNX_IF_ERR_EXIT(WRITE_IHP_IHP_ENABLERSr(unit, SOC_CORE_ALL, reg32));

    /* Protection initialization */
    soc_sand_rv = soc_jer_pp_ing_protection_init(unit);
    SOCDNX_SAND_IF_ERR_EXIT(soc_sand_rv);

    soc_sand_rv = soc_jer_pp_eg_protection_init(unit);
    SOCDNX_SAND_IF_ERR_EXIT(soc_sand_rv);

    /* init ip tunnel */
    if (SOC_IS_JERICHO_B0_AND_ABOVE(unit) && (SOC_JER_PP_EG_ENCAP_IP_TUNNEL_SIZE_PROTOCOL_TEMPLATE_ENABLE == 1)) {
        soc_sand_rv = soc_jer_eg_encap_ip_tunnel_size_protocol_template_init(unit);
        SOCDNX_SAND_IF_ERR_EXIT(soc_sand_rv);
    }

    if (SOC_IS_JERICHO_B0(unit) || SOC_IS_QMX_B0(unit) || SOC_IS_JERICHO_PLUS_A0(unit) || SOC_IS_QUX(unit)) {
        soc_sand_rv = soc_jer_eg_encap_push_2_swap_init(unit);
        SOCDNX_SAND_IF_ERR_EXIT(soc_sand_rv);
    }

    /*CFG_LINK_P_22_FIXED_LATENCY_SETTING should be configured to 0xD regardless of application.*/
    rv = soc_jer_pp_oam_init_from_init_sequence(unit);
    SOCDNX_IF_ERR_EXIT(rv);


exit:
    SOCDNX_FUNC_RETURN;

}



/*
 * Function:
 *      soc_jer_tbls_init
 * Purpose:
 *      initialize all tables relevant for Jericho.
 * Parameters:
 *      unit -  unit number
 *  
 * Returns:
 *      SOC_E_XXX
 */
int soc_jer_tbls_init(
    int unit)
{
    SOCDNX_INIT_FUNC_DEFS;

    /* Zero tables if not running in emulation/simulation */
    if (soc_property_suffix_num_get(unit, -1, spn_CUSTOM_FEATURE, "init_tbls_to_zero", !(
#ifdef PLISIM
            SAL_BOOT_PLISIM || /*not pcid and not emulation*/
#endif
            SOC_DPP_CONFIG(unit)->emulation_system))) {
 
        SOCDNX_IF_ERR_EXIT(soc_jer_dynamic_tbls_reset(unit));
        SOCDNX_IF_ERR_EXIT(soc_jer_static_tbls_reset(unit)); 

        SOCDNX_IF_ERR_EXIT(soc_jer_iqmt_tbls_init(unit)); 

        SOCDNX_IF_ERR_EXIT(soc_jer_ppdb_tbls_init(unit));
    } else {
        SOCDNX_IF_ERR_EXIT(WRITE_FDA_ENABLE_DYNAMIC_MEMORY_ACCESSr(unit, 1));
        SOCDNX_IF_ERR_EXIT(WRITE_PPDB_A_ENABLE_DYNAMIC_MEMORY_ACCESSr(unit, 1));
        SOCDNX_IF_ERR_EXIT(WRITE_PPDB_B_ENABLE_DYNAMIC_MEMORY_ACCESSr(unit, 1));
        SOCDNX_IF_ERR_EXIT(WRITE_EDB_ENABLE_DYNAMIC_MEMORY_ACCESSr(unit, 1));
        if (SOC_DPP_CONFIG(unit)->emulation_system) {
            uint32 data[1] = {0};
            /*
             * Tables reset script used for emulation is not effective on these
             * tables for some reason. This is a small table so this is not too
             * heavy an operation for emulation.
             */
            SOCDNX_IF_ERR_EXIT(jer_fill_and_mark_memory(unit, EGQ_IVEC_TABLEm, data));
            SOCDNX_IF_ERR_EXIT(jer_fill_and_mark_memory(unit, EPNI_IVEC_TABLEm, data));
        }
    }

    SOCDNX_IF_ERR_EXIT(arad_tbl_access_init_unsafe(unit));


    SOCDNX_IF_ERR_EXIT(soc_jer_sch_tbls_init(unit));

    SOCDNX_IF_ERR_EXIT(soc_jer_irr_tbls_init(unit));

    SOCDNX_IF_ERR_EXIT(soc_jer_ire_tbls_init(unit));

    SOCDNX_IF_ERR_EXIT(soc_jer_ihp_tbls_init(unit));

    SOCDNX_IF_ERR_EXIT(soc_jer_ihb_tbls_init(unit));

    SOCDNX_IF_ERR_EXIT(soc_jer_iqm_tbls_init(unit)); 

    SOCDNX_IF_ERR_EXIT(soc_jer_ips_tbls_init(unit));

    SOCDNX_IF_ERR_EXIT(soc_jer_fcr_tbls_init(unit));

    SOCDNX_IF_ERR_EXIT(soc_jer_ipt_tbls_init(unit));

    SOCDNX_IF_ERR_EXIT(soc_jer_fdt_tbls_init(unit));

    SOCDNX_IF_ERR_EXIT(soc_jer_egq_tbls_init(unit));

    SOCDNX_IF_ERR_EXIT(soc_jer_epni_tbls_init(unit));

    SOCDNX_IF_ERR_EXIT(soc_jer_oamp_tbls_init(unit));

    SOCDNX_IF_ERR_EXIT(soc_jer_mrps_tbls_init(unit));

    SOCDNX_IF_ERR_EXIT(soc_jer_mrpsEm_tbls_init(unit));

    SOCDNX_IF_ERR_EXIT(soc_jer_idr_tbls_init(unit));

    SOCDNX_IF_ERR_EXIT(dpp_mult_rplct_tbl_entry_unoccupied_set_all(unit));
    SOCDNX_IF_ERR_EXIT(dpp_mcds_multicast_init2(unit));

    SOCDNX_IF_ERR_EXIT(soc_jer_xlp_tbls_init(unit));
    SOCDNX_IF_ERR_EXIT(soc_jer_clp_tbls_init(unit));


exit:
    SOCDNX_FUNC_RETURN;
}



/*
 * Function:
 *      soc_jer_tbls_deinit
 * Purpose:
 *      de-initialize all tables relevant for Jericho.
 * Parameters:
 *      unit -  unit number
 *  
 * Returns:
 *      SOC_E_XXX
 */
int soc_jer_tbls_deinit(
    int unit)
{

    SOCDNX_INIT_FUNC_DEFS;

    SOCDNX_IF_ERR_EXIT(arad_tbl_access_deinit(unit));

exit:
    SOCDNX_FUNC_RETURN;
}


CONST STATIC int dtq_max_size_contexts[] = {DTQ_0_MAX_SIZE_0f, DTQ_0_MAX_SIZE_1f, DTQ_0_MAX_SIZE_2f, DTQ_0_MAX_SIZE_3f, DTQ_0_MAX_SIZE_4f, DTQ_0_MAX_SIZE_5f,
                                            DTQ_1_MAX_SIZE_0f, DTQ_1_MAX_SIZE_1f, DTQ_1_MAX_SIZE_2f, DTQ_1_MAX_SIZE_3f, DTQ_1_MAX_SIZE_4f, DTQ_1_MAX_SIZE_5f};
CONST STATIC int dtq_start_addr_contexts[] = {DTQ_0_START_0f, DTQ_0_START_1f, DTQ_0_START_2f, DTQ_0_START_3f, DTQ_0_START_4f, DTQ_0_START_5f,
                                              DTQ_1_START_0f, DTQ_1_START_1f, DTQ_1_START_2f, DTQ_1_START_3f, DTQ_1_START_4f, DTQ_1_START_5f};
CONST STATIC int dtq_th_contexts[] = {DTQ_0_TH_0f, DTQ_0_TH_1f, DTQ_0_TH_2f, DTQ_0_TH_3f, DTQ_0_TH_4f, DTQ_0_TH_5f,
                                      DTQ_1_TH_0f, DTQ_1_TH_1f, DTQ_1_TH_2f, DTQ_1_TH_3f, DTQ_1_TH_4f, DTQ_1_TH_5f};
CONST STATIC int dtq_dqcf_th_contexts[] = {DTQ_0_DQCF_TH_0f, DTQ_0_DQCF_TH_1f, DTQ_0_DQCF_TH_2f, DTQ_0_DQCF_TH_3f, DTQ_0_DQCF_TH_4f, DTQ_0_DQCF_TH_5f, 
                                           DTQ_1_DQCF_TH_0f, DTQ_1_DQCF_TH_1f, DTQ_1_DQCF_TH_2f, DTQ_1_DQCF_TH_3f, DTQ_1_DQCF_TH_4f, DTQ_1_DQCF_TH_5f};
CONST STATIC int dqcf_0_max_size_contexts[] = {DQCF_0_MAX_SIZE_0f, DQCF_0_MAX_SIZE_1f, DQCF_0_MAX_SIZE_2f, DQCF_0_MAX_SIZE_3f, DQCF_0_MAX_SIZE_4f, DQCF_0_MAX_SIZE_5f, DQCF_0_MAX_SIZE_6f, 
                                               DQCF_0_MAX_SIZE_7f, DQCF_0_MAX_SIZE_8f, DQCF_0_MAX_SIZE_9f, DQCF_0_MAX_SIZE_10f, DQCF_0_MAX_SIZE_11f, DQCF_0_MAX_SIZE_12f, DQCF_0_MAX_SIZE_13f,
                                               DQCF_0_MAX_SIZE_14f, DQCF_0_MAX_SIZE_15f, DQCF_0_MAX_SIZE_16f, DQCF_0_MAX_SIZE_17f, DQCF_0_MAX_SIZE_18f, DQCF_0_MAX_SIZE_19f};
CONST STATIC int dqcf_1_max_size_contexts[] = {DQCF_1_MAX_SIZE_0f, DQCF_1_MAX_SIZE_1f, DQCF_1_MAX_SIZE_2f, DQCF_1_MAX_SIZE_3f, DQCF_1_MAX_SIZE_4f, DQCF_1_MAX_SIZE_5f, DQCF_1_MAX_SIZE_6f, 
                                               DQCF_1_MAX_SIZE_7f, DQCF_1_MAX_SIZE_8f, DQCF_1_MAX_SIZE_9f, DQCF_1_MAX_SIZE_10f, DQCF_1_MAX_SIZE_11f, DQCF_1_MAX_SIZE_12f, DQCF_1_MAX_SIZE_13f,
                                               DQCF_1_MAX_SIZE_14f, DQCF_1_MAX_SIZE_15f, DQCF_1_MAX_SIZE_16f, DQCF_1_MAX_SIZE_17f, DQCF_1_MAX_SIZE_18f, DQCF_1_MAX_SIZE_19f};
CONST STATIC int dqcf_0_start_addr_contexts[] = {DQCF_0_START_0f, DQCF_0_START_1f, DQCF_0_START_2f, DQCF_0_START_3f, DQCF_0_START_4f, DQCF_0_START_5f, DQCF_0_START_6f, 
                                                 DQCF_0_START_7f, DQCF_0_START_8f, DQCF_0_START_9f, DQCF_0_START_10f, DQCF_0_START_11f, DQCF_0_START_12f, DQCF_0_START_13f, 
                                                 DQCF_0_START_14f, DQCF_0_START_15f, DQCF_0_START_16f, DQCF_0_START_17f, DQCF_0_START_18f, DQCF_0_START_19f};
CONST STATIC int dqcf_1_start_addr_contexts[] = {DQCF_1_START_0f, DQCF_1_START_1f, DQCF_1_START_2f, DQCF_1_START_3f, DQCF_1_START_4f, DQCF_1_START_5f, DQCF_1_START_6f, 
                                                 DQCF_1_START_7f, DQCF_1_START_8f, DQCF_1_START_9f, DQCF_1_START_10f, DQCF_1_START_11f, DQCF_1_START_12f, DQCF_1_START_13f, 
                                                 DQCF_1_START_14f, DQCF_1_START_15f, DQCF_1_START_16f, DQCF_1_START_17f, DQCF_1_START_18f, DQCF_1_START_19f};
CONST STATIC int dqcf_0_th_contexts[] = {DQCF_0_DQCQ_TH_0f, DQCF_0_DQCQ_TH_1f, DQCF_0_DQCQ_TH_2f, DQCF_0_DQCQ_TH_3f, DQCF_0_DQCQ_TH_4f, DQCF_0_DQCQ_TH_5f, DQCF_0_DQCQ_TH_6f, 
                                         DQCF_0_DQCQ_TH_7f, DQCF_0_DQCQ_TH_8f, DQCF_0_DQCQ_TH_9f, DQCF_0_DQCQ_TH_10f, DQCF_0_DQCQ_TH_11f, DQCF_0_DQCQ_TH_12f, DQCF_0_DQCQ_TH_13f, 
                                         DQCF_0_DQCQ_TH_14f, DQCF_0_DQCQ_TH_15f, DQCF_0_DQCQ_TH_16f, DQCF_0_DQCQ_TH_17f, DQCF_0_DQCQ_TH_18f, DQCF_0_DQCQ_TH_19f};
CONST STATIC int dqcf_1_th_contexts[] = {DQCF_1_DQCQ_TH_0f, DQCF_1_DQCQ_TH_1f, DQCF_1_DQCQ_TH_2f, DQCF_1_DQCQ_TH_3f, DQCF_1_DQCQ_TH_4f, DQCF_1_DQCQ_TH_5f, DQCF_1_DQCQ_TH_6f, 
                                         DQCF_1_DQCQ_TH_7f, DQCF_1_DQCQ_TH_8f, DQCF_1_DQCQ_TH_9f, DQCF_1_DQCQ_TH_10f, DQCF_1_DQCQ_TH_11f, DQCF_1_DQCQ_TH_12f, DQCF_1_DQCQ_TH_13f, 
                                         DQCF_1_DQCQ_TH_14f, DQCF_1_DQCQ_TH_15f, DQCF_1_DQCQ_TH_16f, DQCF_1_DQCQ_TH_17f, DQCF_1_DQCQ_TH_18f, DQCF_1_DQCQ_TH_19f};
CONST STATIC int dqcf_0_eir_th_contexts[] = {DQCF_0_EIR_CRDT_TH_0f, DQCF_0_EIR_CRDT_TH_1f, DQCF_0_EIR_CRDT_TH_2f, DQCF_0_EIR_CRDT_TH_3f, DQCF_0_EIR_CRDT_TH_4f, DQCF_0_EIR_CRDT_TH_5f, DQCF_0_EIR_CRDT_TH_6f, 
                                             DQCF_0_EIR_CRDT_TH_7f, DQCF_0_EIR_CRDT_TH_8f, DQCF_0_EIR_CRDT_TH_9f, DQCF_0_EIR_CRDT_TH_10f, DQCF_0_EIR_CRDT_TH_11f, DQCF_0_EIR_CRDT_TH_12f, DQCF_0_EIR_CRDT_TH_13f, 
                                             DQCF_0_EIR_CRDT_TH_14f, DQCF_0_EIR_CRDT_TH_15f, DQCF_0_EIR_CRDT_TH_16f, DQCF_0_EIR_CRDT_TH_17f, DQCF_0_EIR_CRDT_TH_18f, DQCF_0_EIR_CRDT_TH_19f};
CONST STATIC int dqcf_1_eir_th_contexts[] = {DQCF_1_EIR_CRDT_TH_0f, DQCF_1_EIR_CRDT_TH_1f, DQCF_1_EIR_CRDT_TH_2f, DQCF_1_EIR_CRDT_TH_3f, DQCF_1_EIR_CRDT_TH_4f, DQCF_1_EIR_CRDT_TH_5f, DQCF_1_EIR_CRDT_TH_6f, 
                                             DQCF_1_EIR_CRDT_TH_7f, DQCF_1_EIR_CRDT_TH_8f, DQCF_1_EIR_CRDT_TH_9f, DQCF_1_EIR_CRDT_TH_10f, DQCF_1_EIR_CRDT_TH_11f, DQCF_1_EIR_CRDT_TH_12f, DQCF_1_EIR_CRDT_TH_13f, 
                                             DQCF_1_EIR_CRDT_TH_14f, DQCF_1_EIR_CRDT_TH_15f, DQCF_1_EIR_CRDT_TH_16f, DQCF_1_EIR_CRDT_TH_17f, DQCF_1_EIR_CRDT_TH_18f, DQCF_1_EIR_CRDT_TH_19f};
/* 
 * Data Transmit Queue Configuration - 
 * 12 IPT contexts: 3 pipes x OCB/OCB-DRAM-MIX x 2 IPT cores 
 * Buffer size is 2k entries, to be devided between all valid contexts.
 */
STATIC int
soc_jer_ipt_dtq_contexts_init(int unit, int is_dram, int is_dual_fap, int nof_pipes)
{
    soc_reg_above_64_val_t reg_above_64_val;
    uint32 dtq_max_size[12] = {0};
    uint32 *dtq_0_max_size = dtq_max_size, *dtq_1_max_size = dtq_max_size + 6;
    uint32 ctx_max_size_used, ctx_max_size_unused = 0x2; 
    uint32 total_size_dual =  1 * 1024 - 1;
    uint32 total_size_single =  2 * 1024 - 1;
    uint32 dtq_start_addr;
    int i, is_mesh, nof_unused, nof_used;
    SOCDNX_INIT_FUNC_DEFS;


    is_mesh = SOC_DPP_IS_MESH(unit) || 
        (SOC_DPP_CONFIG(unit)->arad->init.fabric.connect_mode == ARAD_FABRIC_CONNECT_MODE_SINGLE_FAP);
    /*max size configuration*/
    if (is_mesh) {
        nof_unused = (is_dram) ? 0 : 3; 
    } else {
        switch (nof_pipes) {
        case 1:
            nof_unused = (is_dram) ? 4 : 5; 
            break;
        case 2:
            nof_unused = (is_dram) ? 2 : 4; 
            break;
        case 3:
            nof_unused = (is_dram) ? 0 : 3; 
            break;
        default:
            SOCDNX_EXIT_WITH_ERR(SOC_E_CONFIG, (_BSL_SOC_MSG("number of pipes %d in invalid"), nof_pipes));
        }
    }

    nof_used = SOC_JER_FABRIC_DTQ_NOF_CTX_PER_IPT_CORE - nof_unused;
    total_size_dual = total_size_dual - ctx_max_size_unused * nof_unused;
    total_size_single = total_size_single - ctx_max_size_unused * nof_unused;
    ctx_max_size_used = (is_dual_fap) ? (total_size_dual / nof_used) : (total_size_single / nof_used) ;

    for (i = 0; i < SOC_JER_FABRIC_DTQ_NOF_CTX_PER_IPT_CORE; i += 2) {
        if (is_dram) {
            dtq_0_max_size[i] = 2 * ctx_max_size_used / 3; 
            dtq_0_max_size[i + 1] = 4 * ctx_max_size_used / 3;
        } else {
            dtq_0_max_size[i] = ctx_max_size_used; 
            dtq_0_max_size[i + 1] = ctx_max_size_unused;
        }
 
        if (is_dual_fap) {
            if (is_dram) {
                dtq_1_max_size[i] = 2 * ctx_max_size_used / 3; 
                dtq_1_max_size[i + 1] = 4 * ctx_max_size_used / 3;
            } else {
                dtq_1_max_size[i] = ctx_max_size_used; 
                dtq_1_max_size[i + 1] = ctx_max_size_unused;
            }
        }
    }
    if ((!is_mesh) && (nof_pipes * 2 < SOC_JER_FABRIC_DTQ_NOF_CTX_PER_IPT_CORE)) {
        for (i = nof_pipes * 2; i < SOC_JER_FABRIC_DTQ_NOF_CTX_PER_IPT_CORE; ++i) {
            dtq_0_max_size[i] = ctx_max_size_unused;
            if (is_dual_fap) {
                dtq_1_max_size[i] = ctx_max_size_unused;
            }
        }
    }
    SOCDNX_IF_ERR_EXIT(READ_IPT_DTQ_MAX_SIZEr(unit, reg_above_64_val)); 
    for (i = 0; i < SOC_JER_FABRIC_DTQ_NOF_CTX_PER_IPT_CORE * 2; ++i) {
        soc_reg_above_64_field32_set(unit, IPT_DTQ_MAX_SIZEr, reg_above_64_val, dtq_max_size_contexts[i], dtq_max_size[i]);
    }
    SOCDNX_IF_ERR_EXIT(WRITE_IPT_DTQ_MAX_SIZEr(unit, reg_above_64_val)); 

    /*queue start address*/
    dtq_start_addr = 0;
    SOCDNX_IF_ERR_EXIT(READ_IPT_DTQ_STARTr(unit, reg_above_64_val)); 
    soc_reg_above_64_field32_set(unit, IPT_DTQ_STARTr, reg_above_64_val, dtq_start_addr_contexts[0], dtq_start_addr);
    for (i = 1; i < SOC_JER_FABRIC_DTQ_NOF_CTX_PER_IPT_CORE * 2; ++i) {
        dtq_start_addr += dtq_max_size[i - 1];
        soc_reg_above_64_field32_set(unit, IPT_DTQ_STARTr, reg_above_64_val, dtq_start_addr_contexts[i], dtq_start_addr);
    }
    SOCDNX_IF_ERR_EXIT(WRITE_IPT_DTQ_STARTr(unit, reg_above_64_val)); 

    /*threshold configuration*/
    SOCDNX_IF_ERR_EXIT(READ_IPT_DTQ_THr(unit, reg_above_64_val)); 
    for (i = 0; i < SOC_JER_FABRIC_DTQ_NOF_CTX_PER_IPT_CORE * 2; ++i) {
        soc_reg_above_64_field32_set(unit, IPT_DTQ_THr, reg_above_64_val, dtq_th_contexts[i], dtq_max_size[i]/2);
    }
    SOCDNX_IF_ERR_EXIT(WRITE_IPT_DTQ_THr(unit, reg_above_64_val)); 

    SOCDNX_IF_ERR_EXIT(READ_IPT_DTQ_DQCF_THr(unit, reg_above_64_val)); 
    for (i = 0; i < SOC_JER_FABRIC_DTQ_NOF_CTX_PER_IPT_CORE * 2; ++i) {
        soc_reg_above_64_field32_set(unit, IPT_DTQ_DQCF_THr, reg_above_64_val, dtq_dqcf_th_contexts[i], dtq_max_size[i]/2);
    }
    SOCDNX_IF_ERR_EXIT(WRITE_IPT_DTQ_DQCF_THr(unit, reg_above_64_val));

exit:
    SOCDNX_FUNC_RETURN;
}


/*
 * DQCF Configuration - 
 * 20 contexts per IPT core: 
 * - Mesh: (Local0, Local1, Dest0, Dest1, Dest2/MC) x OCB/OCB-DRAM-MIX x (HP, LP) 
 * - Fabric: (Local0, Local1, UC, MC) x  OCB/OCB-DRAM-MIX x (HP, LP) 
 * Buffer size is 8k per core. in single fap mode buffer is 16k. to be devided between all valid contexts. 
 */
STATIC int 
soc_jer_ipt_dqcf_contexts_init(int unit, int is_dram, int is_dual_fap)
{
    soc_reg_above_64_val_t reg_above_64_val_0, reg_above_64_val_1;
    uint64 reg64_val;
    uint32 dbl_rsrc, reg_val;
    uint32 dqcf_0_max_size[20] = {0}, dqcf_1_max_size[20] = {0};
    uint32 dqcf_0_th_size[20] = {0}, dqcf_1_th_size[20] = {0};
    uint32 ocb_only_size, dram_ocb_size; 
    uint32 dqcf_0_start_addr, dqcf_1_start_addr;
    uint32 ctx_max_size_unused, ctx_max_size_used;
    uint32 size_dual, size_single;
    uint32 nof_unused_single, nof_unused_dual;
    uint32 total_size_dual =  8 * 1024 - 1;
    uint32 total_size_single =  16 * 1024 - 1;
    int is_mesh;
    int i;
    SOCDNX_INIT_FUNC_DEFS;

    ctx_max_size_unused = 0x3;

    /*size configuration*/

    /* 
     * If there is DRAM in the system, 2/3 of memory goes to MIX (OCB and DRAM), 1/3 to OCB. 
     * Else all Memory goes to OCB. MIX contexts are unused.
     *  
     * NOTE: half of contexts are for MIX and half are for OCB only. 
     */

    SOCDNX_IF_ERR_EXIT(READ_IPT_IPT_ENABLESr(unit, &reg_val));
    dbl_rsrc = soc_reg_field_get(unit, IPT_IPT_ENABLESr, reg_val, DBL_RSRC_ENf);
    
    is_mesh = SOC_DPP_IS_MESH(unit) || 
        (SOC_DPP_CONFIG(unit)->arad->init.fabric.connect_mode == ARAD_FABRIC_CONNECT_MODE_SINGLE_FAP);

    if (is_mesh) {
        if (is_dram) { 
            nof_unused_dual = 0; 
            nof_unused_single = 4; /*contexts 4-7 are not used for single mode*/
            size_dual = (total_size_dual - nof_unused_dual * ctx_max_size_unused)/(SOC_JER_FABRIC_DQCF_NOF_CTX_PER_IPT_CORE - nof_unused_dual);
            if (dbl_rsrc) {
                size_single = (total_size_single - nof_unused_single * ctx_max_size_unused) / (SOC_JER_FABRIC_DQCF_NOF_CTX_PER_IPT_CORE - nof_unused_single); 
            } else {
                size_single = (total_size_dual - nof_unused_single * ctx_max_size_unused) / (SOC_JER_FABRIC_DQCF_NOF_CTX_PER_IPT_CORE - nof_unused_single);
            }
            ctx_max_size_used = (is_dual_fap) ? size_dual : size_single;   
            ocb_only_size = 2 * ctx_max_size_used / 3;
            dram_ocb_size = 4 * ctx_max_size_used / 3;        
        } else {
            nof_unused_dual = 10; 
            nof_unused_single = 12; /*contexts 4-7 are not used for single mode*/
            size_dual = (total_size_dual - nof_unused_dual * ctx_max_size_unused)/(SOC_JER_FABRIC_DQCF_NOF_CTX_PER_IPT_CORE - nof_unused_dual);
            if (dbl_rsrc) {
                size_single = (total_size_single - nof_unused_single * ctx_max_size_unused) / (SOC_JER_FABRIC_DQCF_NOF_CTX_PER_IPT_CORE - nof_unused_single); 
            } else {
                size_single = (total_size_dual - nof_unused_single * ctx_max_size_unused) / (SOC_JER_FABRIC_DQCF_NOF_CTX_PER_IPT_CORE - nof_unused_single);
            }
            ctx_max_size_used = (is_dual_fap) ? size_dual : size_single;   
            ocb_only_size = ctx_max_size_used;
            dram_ocb_size = ctx_max_size_unused;  
        }
    } else {
        if (is_dram) {
            /*contexts 8-11 are not used in CLOS*/
            nof_unused_dual = 4;
            nof_unused_single = 8; /*contexts 4-7 are not used for single mode*/
            size_dual = (total_size_dual - nof_unused_dual * ctx_max_size_unused)/(SOC_JER_FABRIC_DQCF_NOF_CTX_PER_IPT_CORE - nof_unused_dual);
            if (dbl_rsrc) {
                size_single = (total_size_single - nof_unused_single * ctx_max_size_unused) / (SOC_JER_FABRIC_DQCF_NOF_CTX_PER_IPT_CORE - nof_unused_single); 
            } else {
                size_single = (total_size_dual - nof_unused_single * ctx_max_size_unused) / (SOC_JER_FABRIC_DQCF_NOF_CTX_PER_IPT_CORE - nof_unused_single);
            }
            ctx_max_size_used = (is_dual_fap) ? size_dual : size_single ;
            ocb_only_size = 2 * ctx_max_size_used / 3;
            dram_ocb_size = 4 * ctx_max_size_used / 3;  
        } else {
            /*contexts 8-11 are not used in CLOS*/
            nof_unused_dual = 12;
            nof_unused_single = 14; /*contexts 4-7 are not used for single mode*/
            size_dual = (total_size_dual - nof_unused_dual * ctx_max_size_unused)/(SOC_JER_FABRIC_DQCF_NOF_CTX_PER_IPT_CORE - nof_unused_dual);
            if (dbl_rsrc) {
                size_single = (total_size_single - nof_unused_single * ctx_max_size_unused) / (SOC_JER_FABRIC_DQCF_NOF_CTX_PER_IPT_CORE - nof_unused_single); 
            } else {
                size_single = (total_size_dual - nof_unused_single * ctx_max_size_unused) / (SOC_JER_FABRIC_DQCF_NOF_CTX_PER_IPT_CORE - nof_unused_single);
            }
            ctx_max_size_used = (is_dual_fap) ? size_dual : size_single ;
            ocb_only_size = ctx_max_size_used;
            dram_ocb_size = ctx_max_size_unused;  
        }
    }

    for (i = 0; i < SOC_JER_FABRIC_DQCF_NOF_CTX_PER_IPT_CORE; i += 4) {
        dqcf_0_max_size[i] = dqcf_0_max_size[1 + i] = ocb_only_size;
        dqcf_0_max_size[2 + i] = dqcf_0_max_size[3 + i] = dram_ocb_size;
        if (is_dual_fap) {
            dqcf_1_max_size[i] = dqcf_1_max_size[1 + i] = ocb_only_size;
            dqcf_1_max_size[2 + i] = dqcf_1_max_size[3 + i] = dram_ocb_size;
        } else {
            dqcf_1_max_size[i] = dqcf_1_max_size[1 + i] = ctx_max_size_unused;
            dqcf_1_max_size[2 + i] = dqcf_1_max_size[3 + i] = ctx_max_size_unused;
        }
    }
    if (!is_dual_fap) {
        dqcf_0_max_size[4] = dqcf_0_max_size[5] = dqcf_0_max_size[6] = dqcf_0_max_size[7] = ctx_max_size_unused; /*contexts 4-7 are not used in single fap mode*/
    }

    if (!is_mesh) {
        dqcf_0_max_size[8] = dqcf_0_max_size[9] = dqcf_0_max_size[10] = dqcf_0_max_size[11] = ctx_max_size_unused; /*contexts 8-11 are not used in CLOS*/
        if (is_dual_fap) {
            dqcf_1_max_size[8] = dqcf_1_max_size[9] = dqcf_1_max_size[10] = dqcf_1_max_size[11] = ctx_max_size_unused;
        }

    }

    SOCDNX_IF_ERR_EXIT(READ_IPT_DQCF_0_MAX_SIZEr(unit, reg_above_64_val_0)); 
    SOCDNX_IF_ERR_EXIT(READ_IPT_DQCF_1_MAX_SIZEr(unit, reg_above_64_val_1)); 
    for (i = 0; i < SOC_JER_FABRIC_DQCF_NOF_CTX_PER_IPT_CORE; ++i) {
        soc_reg_above_64_field32_set(unit, IPT_DQCF_0_MAX_SIZEr, reg_above_64_val_0, dqcf_0_max_size_contexts[i], dqcf_0_max_size[i]);
        soc_reg_above_64_field32_set(unit, IPT_DQCF_1_MAX_SIZEr, reg_above_64_val_1, dqcf_1_max_size_contexts[i], dqcf_1_max_size[i]);
    }
    SOCDNX_IF_ERR_EXIT(WRITE_IPT_DQCF_0_MAX_SIZEr(unit, reg_above_64_val_0)); 
    SOCDNX_IF_ERR_EXIT(WRITE_IPT_DQCF_1_MAX_SIZEr(unit, reg_above_64_val_1)); 

    /*queue start address*/
    dqcf_0_start_addr = 0;
    dqcf_1_start_addr = 0;
    SOCDNX_IF_ERR_EXIT(READ_IPT_DQCF_0_STARTr(unit, reg_above_64_val_0));
    SOCDNX_IF_ERR_EXIT(READ_IPT_DQCF_1_STARTr(unit, reg_above_64_val_1));
    soc_reg_above_64_field32_set(unit, IPT_DQCF_0_STARTr, reg_above_64_val_0, dqcf_0_start_addr_contexts[0], 0);
    soc_reg_above_64_field32_set(unit, IPT_DQCF_1_STARTr, reg_above_64_val_1, dqcf_1_start_addr_contexts[0], 0);
    for (i = 1; i < SOC_JER_FABRIC_DQCF_NOF_CTX_PER_IPT_CORE; ++i) {
        dqcf_0_start_addr += dqcf_0_max_size[i - 1];
        dqcf_1_start_addr += dqcf_1_max_size[i - 1];
        soc_reg_above_64_field32_set(unit, IPT_DQCF_0_STARTr, reg_above_64_val_0, dqcf_0_start_addr_contexts[i], dqcf_0_start_addr);
        soc_reg_above_64_field32_set(unit, IPT_DQCF_1_STARTr, reg_above_64_val_1, dqcf_1_start_addr_contexts[i], dqcf_1_start_addr);
    }
    SOCDNX_IF_ERR_EXIT(WRITE_IPT_DQCF_0_STARTr(unit, reg_above_64_val_0));
    SOCDNX_IF_ERR_EXIT(WRITE_IPT_DQCF_1_STARTr(unit, reg_above_64_val_1));

    /*dqcf threshold configuration*/
    SOCDNX_IF_ERR_EXIT(READ_IPT_DQCF_0_DQCQ_THr(unit, reg_above_64_val_0)); 
    SOCDNX_IF_ERR_EXIT(READ_IPT_DQCF_1_DQCQ_THr(unit, reg_above_64_val_1)); 
    for (i = 0; i < SOC_JER_FABRIC_DQCF_NOF_CTX_PER_IPT_CORE; ++i) {
        dqcf_0_th_size[i] = (dqcf_0_max_size[i] == ctx_max_size_unused) ? ctx_max_size_unused : dqcf_0_max_size[i]/2 - 1;  /* - 1 to make sure the total TH size is < total max size * 1/2*/
        dqcf_1_th_size[i] = (dqcf_1_max_size[i] == ctx_max_size_unused) ? ctx_max_size_unused : dqcf_1_max_size[i]/2 - 1;

        soc_reg_above_64_field32_set(unit, IPT_DQCF_0_DQCQ_THr, reg_above_64_val_0, dqcf_0_th_contexts[i], dqcf_0_th_size[i]); 
        soc_reg_above_64_field32_set(unit, IPT_DQCF_1_DQCQ_THr, reg_above_64_val_1, dqcf_1_th_contexts[i], dqcf_1_th_size[i]);
    }
    SOCDNX_IF_ERR_EXIT(WRITE_IPT_DQCF_0_DQCQ_THr(unit, reg_above_64_val_0));
    SOCDNX_IF_ERR_EXIT(WRITE_IPT_DQCF_1_DQCQ_THr(unit, reg_above_64_val_1));

    /*eir threshold configuration*/
    SOCDNX_IF_ERR_EXIT(READ_IPT_DQCF_0_EIR_CRDT_THr(unit, reg_above_64_val_0)); 
    SOCDNX_IF_ERR_EXIT(READ_IPT_DQCF_1_EIR_CRDT_THr(unit, reg_above_64_val_1)); 
    for (i = 0; i < SOC_JER_FABRIC_DQCF_NOF_CTX_PER_IPT_CORE; ++i) {
        dqcf_0_th_size[i] = (dqcf_0_max_size[i] == ctx_max_size_unused) ? ctx_max_size_unused : 2*dqcf_0_max_size[i]/3 - 1;  /* - 1 to make sure the total TH size is < total max size * 2/3*/
        dqcf_1_th_size[i] = (dqcf_1_max_size[i] == ctx_max_size_unused) ? ctx_max_size_unused : 2*dqcf_1_max_size[i]/3 - 1;

         soc_reg_above_64_field32_set(unit, IPT_DQCF_0_EIR_CRDT_THr, reg_above_64_val_0, dqcf_0_eir_th_contexts[i], dqcf_0_th_size[i]);
         soc_reg_above_64_field32_set(unit, IPT_DQCF_1_EIR_CRDT_THr, reg_above_64_val_1, dqcf_1_eir_th_contexts[i], dqcf_1_th_size[i]);
    }
    SOCDNX_IF_ERR_EXIT(WRITE_IPT_DQCF_0_EIR_CRDT_THr(unit, reg_above_64_val_0));
    SOCDNX_IF_ERR_EXIT(WRITE_IPT_DQCF_1_EIR_CRDT_THr(unit, reg_above_64_val_1));

    /*multicast threshold configuration -fabric mode only*/
    if (!is_mesh) { 
        for (i = 16; i < 19; ++i) {
            dqcf_0_th_size[i] = (dqcf_0_max_size[i] == ctx_max_size_unused) ? ctx_max_size_unused : 2*dqcf_0_max_size[i]/3 - 1;  /* - 1 to make sure the total TH size is < total max size * 2/3*/
            dqcf_1_th_size[i] = (dqcf_1_max_size[i] == ctx_max_size_unused) ? ctx_max_size_unused : 2*dqcf_1_max_size[i]/3 - 1;
        }
        SOCDNX_IF_ERR_EXIT(READ_IPT_DQCF_0_MC_THr(unit, &reg64_val));
        soc_reg64_field32_set(unit, IPT_DQCF_0_MC_THr, &reg64_val, DQCF_0_MC_GFMC_OCB_THf, dqcf_0_th_size[16]);
        soc_reg64_field32_set(unit, IPT_DQCF_0_MC_THr, &reg64_val, DQCF_0_MC_BFMC_OCB_THf, dqcf_0_th_size[17]);
        soc_reg64_field32_set(unit, IPT_DQCF_0_MC_THr, &reg64_val, DQCF_0_MC_GFMC_MIX_THf, dqcf_0_th_size[18]);
        soc_reg64_field32_set(unit, IPT_DQCF_0_MC_THr, &reg64_val, DQCF_0_MC_BFMC_MIX_THf, dqcf_0_th_size[19]);
        SOCDNX_IF_ERR_EXIT(WRITE_IPT_DQCF_0_MC_THr(unit, reg64_val));

        SOCDNX_IF_ERR_EXIT(READ_IPT_DQCF_1_MC_THr(unit, &reg64_val)); 
        soc_reg64_field32_set(unit, IPT_DQCF_1_MC_THr, &reg64_val, DQCF_1_MC_GFMC_OCB_THf, dqcf_1_th_size[16]);
        soc_reg64_field32_set(unit, IPT_DQCF_1_MC_THr, &reg64_val, DQCF_1_MC_BFMC_OCB_THf, dqcf_1_th_size[17]);
        soc_reg64_field32_set(unit, IPT_DQCF_1_MC_THr, &reg64_val, DQCF_1_MC_GFMC_MIX_THf, dqcf_1_th_size[18]);
        soc_reg64_field32_set(unit, IPT_DQCF_1_MC_THr, &reg64_val, DQCF_1_MC_BFMC_MIX_THf, dqcf_1_th_size[19]);
        SOCDNX_IF_ERR_EXIT(WRITE_IPT_DQCF_1_MC_THr(unit, reg64_val));
    }

exit:
    SOCDNX_FUNC_RETURN;
}

/*
 *  IPT block default values
 */
uint32
soc_jer_ipt_contexts_init(
  SOC_SAND_IN int     unit)
{
    uint32 nof_pipes, is_dual_fap, is_dram, reg_val;
    SOCDNX_INIT_FUNC_DEFS;

    /*Get relevant properites */
    nof_pipes = SOC_DPP_CONFIG(unit)->arad->init.fabric.fabric_pipe_map_config.nof_pipes;
    is_dram = SOC_DPP_CONFIG(unit)->arad->init.drc_info.dram_num ? 1 : 0;
    is_dual_fap = (SOC_DPP_CONFIG(unit)->core_mode.nof_active_cores > 1) ? 1 : 0 ;

    /*Enable IPT*/
    SOCDNX_IF_ERR_EXIT(READ_IPT_IPT_ENABLESr(unit, &reg_val));
    soc_reg_field_set(unit, IPT_IPT_ENABLESr, &reg_val, IPT_ENf, 1);
    /* in turbo core mode allow core 0 to use core 1 resources */
    if (SOC_DPP_CONFIG(unit)->core_mode.nof_active_cores == 1) {
        soc_reg_field_set(unit, IPT_IPT_ENABLESr, &reg_val, DBL_RSRC_ENf, 1);
    }
    SOCDNX_IF_ERR_EXIT(WRITE_IPT_IPT_ENABLESr(unit, reg_val));

    /*configure DTQ contexts*/
    SOCDNX_IF_ERR_EXIT(soc_jer_ipt_dtq_contexts_init(unit, is_dram, is_dual_fap, nof_pipes));

    /*configure DQCF contexts*/
    SOCDNX_IF_ERR_EXIT(soc_jer_ipt_dqcf_contexts_init(unit, is_dram, is_dual_fap));

    /* configure INGRESS_LATENCY parameters in IPT */
    SOCDNX_IF_ERR_EXIT(jer_itm_ingress_latency_init(unit));    
exit:
    SOCDNX_FUNC_RETURN;
}

int soc_qax_sqm_tbls_init(int unit)
{
    uint32 table_entry[8] = {0};
    SOCDNX_INIT_FUNC_DEFS;

    table_entry[0] = 0xfffffffe;
    table_entry[1] = 0xffffffff;
    table_entry[2] = 0x7c;
    SOCDNX_IF_ERR_EXIT(dcmn_fill_table_with_entry(unit, SQM_FREE_PDB_MEMORYm, MEM_BLOCK_ANY, table_entry));
exit:
    SOCDNX_FUNC_RETURN;
}

int soc_qax_ihb_tbls_init(int unit)
{
    uint32 table_entry[5] = {0};
    uint64 data;
    int core;

    SOCDNX_INIT_FUNC_DEFS;

    COMPILER_64_SET(data, 0, 0x7ffff);
    if (SOC_MEM_IS_VALID(unit, IHB_FEC_ENTRYm)) {
        soc_mem_field64_set(unit, IHB_FEC_ENTRYm, table_entry, DATA_0f, data);
        soc_mem_field64_set(unit, IHB_FEC_ENTRYm, table_entry, DATA_1f, data);
        soc_mem_field32_set(unit, IHB_FEC_ENTRYm, table_entry, PROTECTION_POINTERf, 0x7fff);

        SOC_DPP_CORES_ITER(SOC_CORE_ALL, core) {
            SOCDNX_IF_ERR_EXIT(dcmn_fill_table_with_entry(unit, IHB_FEC_ENTRYm, IHB_BLOCK(unit, core), table_entry));
        }
    }

exit:
    SOCDNX_FUNC_RETURN;
}

/*
 * Function:
 *      soc_jer_init_sequence_phase1
 * Purpose:
 *     Initialize the device, including:
 *     - Prepare internal data
 *     - Initialize basic configuration before soft reset
 *     - Table Initialization
 *     - Perform Device Soft Reset
 *     - General configuration
 *     - Set Core clock frequency
 *     - Set board-related configuration (hardware adjustments)
 *     - Module configuration
 *     - Set Egress Port mapping
 *     - OFP Rates Initialization
 *     - Scheduler Initialization
 * Parameters:
 *      unit -  unit number
 * Returns:
 *      SOC_E_XXX
 *
 */
int soc_jer_init_sequence_phase1(int unit)
{
    ARAD_MGMT_INIT* init;
    uint32 rval,is_valid,port_i;
    int core;
    uint64 r64;
    char* slow_rate_max_level_str;
    SOCDNX_INIT_FUNC_DEFS;

    init = &(SOC_DPP_CONFIG(unit)->arad->init);

#ifdef PLISIM
    if (SAL_BOOT_PLISIM) {
      chip_sim_em_init(unit, SOC_SAND_DEV_ARAD, TRUE /* LEM shadow supported */, FALSE /* crash recovery not supported */);
    }
#endif

    DCMN_RUNTIME_DEBUG_PRINT_LOC(unit, "starting oc_jer_init_sequence_phase1");
    /* Prepare internal data */
    SOCDNX_IF_ERR_EXIT(soc_jer_init_prepare_internal_data(unit));

    if (!SOC_WARM_BOOT(unit)) {
        /* Blocks Initial configuration */
        LOG_INFO(BSL_LS_SOC_INIT, (BSL_META_U(unit, "\t+ %d: Blocks Initial configuration\n"),unit));
        SOCDNX_IF_ERR_EXIT(soc_jer_init_blocks_init(unit));
        DCMN_RUNTIME_DEBUG_PRINT(unit);






#ifdef PLISIM
       if (!SAL_BOOT_PLISIM) 
#endif
       {
           if (!SOC_IS_JERICHO_PLUS_ONLY(unit)) {

               uint32 bist_enable = soc_property_get(unit, spn_BIST_ENABLE, 0);
               if (!bist_enable && SOC_DPP_CONFIG(unit)->tm.various_bm & DPP_VARIOUS_BM_FORCE_MBIST_TEST) {
                   bist_enable = 1;
               }

               /* perform MBIST if configured to do so */
               if (bist_enable) {
                   if (!SOC_IS_QUX(unit)) {
                       DCMN_RUNTIME_DEBUG_PRINT(unit);

                       DISPLAY_SW_STATE_MEM ;

                       LOG_INFO(BSL_LS_SOC_INIT, (BSL_META_U(unit, "\t+ %d: running internal memories BIST\n"),unit));
                       SOCDNX_IF_ERR_EXIT(MBCM_DPP_DRIVER_CALL(unit, mbcm_dpp_soc_bist_all,(unit, (bist_enable == 2 ? 1 : 0))));
                   }
               } else if (SOC_IS_QAX(unit)) {
                       SOCDNX_IF_ERR_EXIT(qax_mbist_fix_arm_core(unit, 0));
               }
           }
       }


       /* Initialize all tables     and      
        * Setup Global/Special registers 
        */
       LOG_INFO(BSL_LS_SOC_INIT, (BSL_META_U(unit, "\t+ %d: Configure tables defaults\n"),unit));

        if (SOC_IS_QAX(unit)) {
            SOCDNX_IF_ERR_EXIT(soc_qax_tbls_init(unit));
            if (soc_property_get(unit, spn_LINK_BONDING_ENABLE, 0)) {
                SOCDNX_IF_ERR_EXIT(qax_lb_egr_init(unit));
            }

            SOCDNX_IF_ERR_EXIT(soc_qax_init_blocks_init_conf(unit));
        } else {
            SOCDNX_IF_ERR_EXIT(SOC_IS_JERICHO_PLUS(unit) ? soc_jerplus_tbls_init(unit) : soc_jer_tbls_init(unit));
            SOCDNX_IF_ERR_EXIT(soc_jer_init_blocks_init_conf(unit));
        }

        DCMN_RUNTIME_DEBUG_PRINT(unit);

        /* init ETPP_PIPE_LENGTH */
        SOC_DPP_CORES_ITER(SOC_CORE_ALL, core) {
            SOCDNX_IF_ERR_EXIT(READ_EPNI_ETPP_PIPE_LENGTHr(unit, core, &rval));
            soc_reg_field_set(unit, EPNI_ETPP_PIPE_LENGTHr, &rval, ETPP_PIPE_LENGTHf, 0x28); /* set to 40 instead 30 */
            SOCDNX_IF_ERR_EXIT(WRITE_EPNI_ETPP_PIPE_LENGTHr(unit, core, rval));
        }

        /* configure the recycle interface contexts/FIFOs of core 1 to send the recycle traffic to the same core */
        core = SOC_DPP_CONFIG(unit)->core_mode.nof_active_cores - 1; /* get the ID of the last active core */
        if (core > 0) {
            SOCDNX_IF_ERR_EXIT(READ_EPNI_MAPPING_RECYCLE_TRAFFICTO_INGRESS_PIPEr(unit, core, &rval));
            soc_reg_field_set(unit, EPNI_MAPPING_RECYCLE_TRAFFICTO_INGRESS_PIPEr, &rval, MAPPING_TRAP_TO_INGRESS_PIPEf, core);
            soc_reg_field_set(unit, EPNI_MAPPING_RECYCLE_TRAFFICTO_INGRESS_PIPEr, &rval, MAPPING_RCY_TO_INGRESS_PIPEf, core);
            soc_reg_field_set(unit, EPNI_MAPPING_RECYCLE_TRAFFICTO_INGRESS_PIPEr, &rval, MAPPING_MIRROR_TO_INGRESS_PIPEf, core);
            SOCDNX_IF_ERR_EXIT(WRITE_EPNI_MAPPING_RECYCLE_TRAFFICTO_INGRESS_PIPEr(unit, core, rval));
        }

        /* Soft Init Blocks */
        LOG_INFO(BSL_LS_SOC_INIT, (BSL_META_U(unit, "\t+ %d: Soft init Device Blocks\n"),unit));
        SOCDNX_IF_ERR_EXIT(soc_jer_reset_blocks_soft_init(unit, SOC_DPP_RESET_ACTION_INOUT_RESET));
        DCMN_RUNTIME_DEBUG_PRINT(unit);

        if (SOC_IS_QAX(unit)) {
            SOCDNX_IF_ERR_EXIT(soc_qax_sqm_tbls_init(unit));
            SOCDNX_IF_ERR_EXIT(soc_qax_ihb_tbls_init(unit));
        }

        if (!SOC_IS_QAX(unit)) { 
            /* Blocks General configuration */
            LOG_INFO(BSL_LS_SOC_INIT, (BSL_META_U(unit, "\t+ %d: Blocks General configuration\n"),unit));
            SOCDNX_IF_ERR_EXIT(soc_jer_init_blocks_general(unit));
            DCMN_RUNTIME_DEBUG_PRINT(unit);
        } 
        else
        { /* QAX only. Remove when the if is gone */
            soc_reg_above_64_val_t reg_above_64_val;
                  
            SOCDNX_IF_ERR_EXIT(READ_PPDB_B_LARGE_EM_COUNTER_DB_CNTR_PTR_CONFIGURATIONr(unit, reg_above_64_val));
      
            /* If set, the limit on the number of entries in the MACT is according to FID,  else the limit is according to lif. - Default MACT limit per FID */
            soc_reg_above_64_field32_set(unit, PPDB_B_LARGE_EM_COUNTER_DB_CNTR_PTR_CONFIGURATIONr, reg_above_64_val, LARGE_EM_CFG_LIMIT_MODE_FIDf, 0x1);            
            SOCDNX_IF_ERR_EXIT(WRITE_PPDB_B_LARGE_EM_COUNTER_DB_CNTR_PTR_CONFIGURATIONr(unit, reg_above_64_val));

            /* Enable pvt monitor*/
            SOCDNX_IF_ERR_EXIT(jer_mgmt_drv_pvt_monitor_enable(unit));
        }
        
        /* Set Core clock frequency */
        LOG_INFO(BSL_LS_SOC_INIT, (BSL_META_U(unit, "\t+ %d: Set core clock frequency\n"),unit));
        SOCDNX_IF_ERR_EXIT(arad_mgmt_init_set_core_clock_frequency(unit, init));
        DCMN_RUNTIME_DEBUG_PRINT(unit);

        /* configure default credit worth */
        if (init->credit.credit_worth_enable)
        {
            SOCDNX_IF_ERR_EXIT(jer_mgmt_credit_worth_set(unit, init->credit.credit_worth));
        }

        /* Set board-related configuration (hardware adjustments) */
        DCMN_RUNTIME_DEBUG_PRINT(unit);
        LOG_INFO(BSL_LS_SOC_INIT, (BSL_META_U(unit, "\t+ %d: Set hardware adjustments (Dram)\n"),unit));
        SOCDNX_IF_ERR_EXIT(soc_jer_init_hw_interfaces_set(unit));

        /*
         * Before the port configuration: OTMH extensions configuration
         * must know the egress editor program attributes
         */
        SOCDNX_IF_ERR_EXIT(arad_egr_prog_editor_config_dut_by_queue_database(unit));

        /* Set port init */
        if (SOC_IS_JERICHO(unit) || SOC_IS_QAX(unit)) {
            LOG_INFO(BSL_LS_SOC_INIT, (BSL_META_U(unit, "\t+ %d: Set port configurations \n"),unit));
            SOCDNX_IF_ERR_EXIT(soc_jer_init_port(unit));
            DCMN_RUNTIME_DEBUG_PRINT(unit);
        }

        if (SOC_IS_QAX(unit)) {
            SOCDNX_IF_ERR_EXIT(READ_EGQ_EGRESS_REPLICATION_GENERAL_CONFIGr(unit, SOC_CORE_ALL, &r64));
            soc_reg64_field32_set(unit, EGQ_EGRESS_REPLICATION_GENERAL_CONFIGr, &r64, EGRESS_MC_1_REP_ENABLEf, 15);
            soc_reg64_field32_set(unit, EGQ_EGRESS_REPLICATION_GENERAL_CONFIGr, &r64, EGRESS_MC_2_REP_ENABLEf, 3);
            soc_reg64_field32_set(unit, EGQ_EGRESS_REPLICATION_GENERAL_CONFIGr, &r64, EGRESS_REP_DIRECT_BITMAP_CUDf, 0);
            soc_reg64_field32_set(unit, EGQ_EGRESS_REPLICATION_GENERAL_CONFIGr, &r64, TDM_REP_FORMAT_ENf, 0);
            SOCDNX_IF_ERR_EXIT(WRITE_EGQ_EGRESS_REPLICATION_GENERAL_CONFIGr(unit, SOC_CORE_ALL, r64));
        } else {
        }

        /* Set module init */

        DISPLAY_SW_STATE_MEM ;

        LOG_INFO(BSL_LS_SOC_INIT, (BSL_META_U(unit, "\t+ %d: Set default module configurations \n"),unit));
        SOCDNX_IF_ERR_EXIT(soc_jer_init_functional_init(unit));
        DCMN_RUNTIME_DEBUG_PRINT(unit);

        /* init egr tm (port mapping) */
        LOG_INFO(BSL_LS_SOC_INIT, (BSL_META_U(unit, "\t+ %d: Set Egress Port mapping \n"),unit));
        SOCDNX_IF_ERR_EXIT(soc_jer_egr_tm_init(unit));
        DCMN_RUNTIME_DEBUG_PRINT(unit);

        /* init ofp rates */
        LOG_INFO(BSL_LS_SOC_INIT, (BSL_META_U(unit, "\t+ %d: OFP Rates Initialization\n"),unit));
        SOCDNX_IF_ERR_EXIT(soc_jer_ofp_rates_init(unit));
        DCMN_RUNTIME_DEBUG_PRINT(unit);

        /* init sch */
        LOG_INFO(BSL_LS_SOC_INIT, (BSL_META_U(unit, "\t+ %d: Scheduler Initialization\n"),unit));
        SOCDNX_IF_ERR_EXIT(soc_jer_sch_init(unit));

        LOG_INFO(BSL_LS_SOC_INIT, (BSL_META_U(unit, "\t+ %d: Ingress Packet Queuing Initialization\n"),unit));
        SOCDNX_IF_ERR_EXIT(jer_ipq_init(unit));

        LOG_INFO(BSL_LS_SOC_INIT, (BSL_META_U(unit, "\t+ %d: Ingress Traffic Management Initialization\n"),unit));
        SOCDNX_IF_ERR_EXIT(MBCM_DPP_DRIVER_CALL(unit, mbcm_dpp_itm_init,(unit)));

        /* 2nd myMAC init */
        if (SOC_IS_ROO_ENABLE(unit)) {

            DISPLAY_SW_STATE_MEM ;

            LOG_INFO(BSL_LS_SOC_INIT,
                     (BSL_META_U(unit,
                                 "\t+ %d: Set 2nd myMAC configurations for ROO \n"),unit));
            SOCDNX_IF_ERR_EXIT(soc_jer_mymac_2nd_mymac_init(unit));
        }
        /*Enable 1588 TC*/ 
        if (init->pll.ts_clk_mode == 0x1) {
            SOCDNX_IF_ERR_EXIT(WRITE_CMIC_TIMESYNC_COUNTER_CONFIG_SELECTr(unit, 1));
            SOCDNX_IF_ERR_EXIT(WRITE_CMIC_TIMESYNC_TS0_FREQ_CTRL_FRACr(unit, SOC_DPP_CONFIG(unit)->arad->init.pll.ts_pll_phase_initial_lo));
            SOCDNX_IF_ERR_EXIT(WRITE_CMIC_TIMESYNC_TS0_COUNTER_ENABLEr(unit, 1));
        }

        /* Enable SAT*/
        if (soc_property_get(unit, spn_SAT_ENABLE, 0)) {
            SOCDNX_IF_ERR_EXIT(soc_jer_pp_oam_sat_init(unit));
        }
        		
        SOCDNX_SAND_IF_ERR_EXIT(arad_cnt_init(unit));

        if (init->stat_if.stat_if_enable)
        {
            if(SOC_IS_QAX(unit))
            {
                SOCDNX_IF_ERR_EXIT(soc_qax_stat_if_init_set(unit, &(init->stat_if)));
            }
            else
            {
                SOCDNX_IF_ERR_EXIT(soc_jer_stat_if_init_set(unit, &(init->stat_if)));
            }
            
        }

        /* Enable Ecn DM */
        if (soc_property_get(unit, spn_ECN_DM_ENABLE, 0)) {
            SOCDNX_IF_ERR_EXIT(arad_pp_oam_ecn_init(unit));
        }
        /* init eci_ntp tod clock only if the PLL is lock */
        /* tod is used for ingress latency measurement and OAM */
        if (soc_property_get(unit, spn_PHY_1588_DPLL_FREQUENCY_LOCK, 0)) {

            SOCDNX_IF_ERR_EXIT(soc_jer_pp_oam_init_eci_tod(unit, 1/*init_ntp*/, 0/*don't init_1588*/));
        }
    
        /* Enable timestamp 64 bit and SSP program */
        for (port_i = 0; port_i < SOC_MAX_NUM_PORTS; ++port_i) {
            SOCDNX_IF_ERR_EXIT(soc_port_sw_db_is_valid_port_get(unit, port_i, &is_valid));
            if (!is_valid) {
                continue;
            }
            SOCDNX_IF_ERR_EXIT(soc_jer_port_timestamp_and_ssp_update(unit, port_i));
            SOCDNX_IF_ERR_EXIT(soc_jer_port_oam_ts_init(unit, port_i));
        }

        if (!SOC_IS_QAX(unit)) {
          /* Stacking Configuration */
          if (init->ports.is_stacking_system == 0x1) {
              LOG_INFO(BSL_LS_SOC_INIT, (BSL_META_U(unit, "\t+ %d: Stacking Initialization\n"),unit));
              SOCDNX_IF_ERR_EXIT(arad_mgmt_stk_init(unit, init));
          }
        }
    }
    else
    {
      /* update max_burst_default_value_bucket_width in WB*/
      /*we need to reload max_burst_default_value_bucket_width after WB*/
      slow_rate_max_level_str = soc_property_get_str(unit, spn_SLOW_MAX_RATE_LEVEL);
      if ( (slow_rate_max_level_str == NULL) || (sal_strcmp(slow_rate_max_level_str, "HIGH")==0) )
      {
          /* 7 */
          SOC_DPP_CONFIG(unit)->arad->init.max_burst_default_value_bucket_width = (0x7f)*256;
      } 
      else if (sal_strcmp(slow_rate_max_level_str, "NORMAL") == 0)
      {
          /* 8 */
          SOC_DPP_CONFIG(unit)->arad->init.max_burst_default_value_bucket_width = (0xff)*256;
      } 
      else if (sal_strcmp(slow_rate_max_level_str, "LOW") == 0)
      {
          /* 9 */
          SOC_DPP_CONFIG(unit)->arad->init.max_burst_default_value_bucket_width = (0x1ff)*256;
      }
      else
      {
          SOCDNX_EXIT_WITH_ERR(SOC_E_CONFIG, (_BSL_SOCDNX_MSG("Unsupported properties: slow_rate_max_level should be LOW/NORMAL/HIGH")));
      }
    }
#if defined(INCLUDE_KBP) && !defined(BCM_88030)
    arad_kbp_sw_init(unit);
#endif
    
exit:
    SOCDNX_FUNC_RETURN;
}


