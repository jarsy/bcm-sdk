/*
 * $Id: $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File: qax_init.c
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
#include <sal/core/thread.h> /* for sal_usleep */
#include <sal/core/libc.h> /* for sal_memset */

/* SOC DPP includes */
#include <soc/dpp/drv.h>
#include <soc/dpp/mbcm.h>
#include <soc/dcmn/dcmn_mem.h>
#include <soc/dpp/JER/jer_reset.h>
#include <soc/dpp/JER/jer_intr.h>
#include <soc/dpp/QAX/qax_init.h>
#include <soc/dpp/QAX/qax_sram.h>
#include <soc/dpp/QAX/qax_mgmt.h>
#include <soc/dpp/QAX/qax_ipsec.h>
#include <soc/dpp/QAX/QAX_PP/qax_pp_lif.h>


/*************
 * DEFINES   *
 *************/
/* { */

#define QAX_MASK_NIF_OVERRIDES

#define QAX_INIT_UNUSED_DTQ_PDQ_FC_TH     (0x2)
#define QAX_INIT_MAX_NUM_OF_DTQS          (0x6)

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

/*DQCQ (PDQ) init struct*/
typedef struct dqcq_init_vals_s {
    int queue_start;
    int queue_size;
    int dqcq_word_fc_threshold;
    int eir_crdt_fc_threshold;
    int dqcq_fc_threshold;
} dqcq_init_vals_t;

/* } */


/* 
 * Init functions
 */

int soc_qax_init_blocks_init_global_conf(int unit)
{
    uint32 reg32_val, field_val;

    SOCDNX_INIT_FUNC_DEFS;

    /*Petra-b in system */
    if (SOC_DPP_CONFIG(unit)->tm.is_petrab_in_system) {
        SOCDNX_IF_ERR_EXIT(READ_ECI_GLOBAL_SYS_HEADER_CFGr(unit, &reg32_val));
        soc_reg_field_set(unit, ECI_GLOBAL_SYS_HEADER_CFGr, &reg32_val, SYSTEM_HEADERS_MODEf, 1);
        SOCDNX_IF_ERR_EXIT(WRITE_ECI_GLOBAL_SYS_HEADER_CFGr(unit, reg32_val));
    }

    /* FTMH LB mode */
    SOCDNX_IF_ERR_EXIT(READ_ECI_GLOBAL_SYS_HEADER_CFGr(unit, &reg32_val));
    field_val = SOC_DPP_CONFIG(unit)->arad->init.fabric.ftmh_lb_ext_mode == ARAD_MGMT_FTMH_LB_EXT_MODE_DISABLED ? 0 : 1;
    soc_reg_field_set(unit, ECI_GLOBAL_SYS_HEADER_CFGr, &reg32_val, FTMH_LB_KEY_EXT_ENf, field_val);
    SOCDNX_IF_ERR_EXIT(WRITE_ECI_GLOBAL_SYS_HEADER_CFGr(unit, reg32_val));

    /* Configure core mode, not in clear-channel mode */
    SOCDNX_IF_ERR_EXIT(READ_ECI_GLOBAL_GENERAL_CFG_2r(unit, &reg32_val));
    soc_reg_field_set(unit, ECI_GLOBAL_GENERAL_CFG_2r, &reg32_val, FORCE_FABRICf, 0);
    soc_reg_field_set(unit, ECI_GLOBAL_GENERAL_CFG_2r, &reg32_val, FORCE_LOCALf, SOC_DPP_CONFIG(unit)->arad->init.fabric.connect_mode == SOC_TMC_FABRIC_CONNECT_MODE_SINGLE_FAP ? 1 : 0);
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
    soc_reg_field_set(unit, ECI_GLOBAL_GENERAL_CFG_3r, &reg32_val, TOD_MODEf, 3);
    SOCDNX_IF_ERR_EXIT(WRITE_ECI_GLOBAL_GENERAL_CFG_3r(unit, reg32_val));

    if (!soc_feature(unit, soc_feature_no_fabric)) {
        /* Init FDT transactions counter setting */
        SOCDNX_IF_ERR_EXIT(READ_TXQ_FDT_PRG_COUNTER_CFGr(unit, 0, &reg32_val));
        soc_reg_field_set(unit, TXQ_FDT_PRG_COUNTER_CFGr, &reg32_val, PRG_N_CNT_CFG_MC_MASKf, 1);
        soc_reg_field_set(unit, TXQ_FDT_PRG_COUNTER_CFGr, &reg32_val, PRG_N_CNT_CFG_MC_VALf, 0);
        soc_reg_field_set(unit, TXQ_FDT_PRG_COUNTER_CFGr, &reg32_val, PRG_N_CNT_CFG_SRAM_MASKf, 0);
        soc_reg_field_set(unit, TXQ_FDT_PRG_COUNTER_CFGr, &reg32_val, PRG_N_CNT_CFG_SRAM_VALf, 1);
        soc_reg_field_set(unit, TXQ_FDT_PRG_COUNTER_CFGr, &reg32_val, PRG_N_CNT_CFG_CTXT_MASKf, 1);
        soc_reg_field_set(unit, TXQ_FDT_PRG_COUNTER_CFGr, &reg32_val, PRG_N_CNT_CFG_CTXT_VALf, 0);
        SOCDNX_IF_ERR_EXIT(WRITE_TXQ_FDT_PRG_COUNTER_CFGr(unit, 0, reg32_val));

        SOCDNX_IF_ERR_EXIT(READ_TXQ_FDT_PRG_COUNTER_CFGr(unit, 1, &reg32_val));
        soc_reg_field_set(unit, TXQ_FDT_PRG_COUNTER_CFGr, &reg32_val, PRG_N_CNT_CFG_MC_MASKf, 1);
        soc_reg_field_set(unit, TXQ_FDT_PRG_COUNTER_CFGr, &reg32_val, PRG_N_CNT_CFG_MC_VALf, 0);
        soc_reg_field_set(unit, TXQ_FDT_PRG_COUNTER_CFGr, &reg32_val, PRG_N_CNT_CFG_SRAM_MASKf, 0);
        soc_reg_field_set(unit, TXQ_FDT_PRG_COUNTER_CFGr, &reg32_val, PRG_N_CNT_CFG_SRAM_VALf, 1);
        soc_reg_field_set(unit, TXQ_FDT_PRG_COUNTER_CFGr, &reg32_val, PRG_N_CNT_CFG_CTXT_MASKf, 1);
        soc_reg_field_set(unit, TXQ_FDT_PRG_COUNTER_CFGr, &reg32_val, PRG_N_CNT_CFG_CTXT_VALf, 0);
        SOCDNX_IF_ERR_EXIT(WRITE_TXQ_FDT_PRG_COUNTER_CFGr(unit, 1, reg32_val));
    }

exit:
    SOCDNX_FUNC_RETURN;
}

int soc_qax_init_blocks_init_general_conf(int unit)
{
    soc_reg_above_64_val_t reg_above_64_val;
    soc_reg_above_64_val_t reg_field_a64;
    uint32                 reg32_val = 0;
    uint32                 user_hdr_size = 0;
    int i;

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
    SOCDNX_IF_ERR_EXIT(soc_jer_reset_nif_txi_oor(unit));
    SOCDNX_IF_ERR_EXIT(WRITE_ECI_OGER_1008r_REG32(unit, 0x20101977));
#ifdef QAX_COMMENT_OUT_CODE_NOT_YET_IMPLEMENTED
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
#endif
    SOCDNX_IF_ERR_EXIT(READ_IPS_UPDATE_INDICATIONr(unit, &reg32_val));
    soc_reg_field_set(unit, IPS_UPDATE_INDICATIONr, &reg32_val, FORCE_NO_UPDATEf, 1);
    SOCDNX_IF_ERR_EXIT(WRITE_IPS_UPDATE_INDICATIONr(unit, reg32_val));

    /* set ITE_ITPP_GENERAL_CFGr */
    /* INVALID CUD value is set to 0, instead of the default 0xfffe for ingress */
    user_hdr_size =
        (soc_property_port_get(unit, 0, spn_FIELD_CLASS_ID_SIZE, 0) +
         soc_property_port_get(unit, 1, spn_FIELD_CLASS_ID_SIZE, 0) +
         soc_property_port_get(unit, 2, spn_FIELD_CLASS_ID_SIZE, 0) +
         soc_property_port_get(unit, 3, spn_FIELD_CLASS_ID_SIZE, 0)) / 8;
    SOC_REG_ABOVE_64_CLEAR(reg_above_64_val);
    soc_reg_above_64_field32_set(unit, ITE_ITPP_GENERAL_CFGr, reg_above_64_val, UDH_FIXED_LENGTHf, user_hdr_size);
    soc_reg_above_64_field32_set(unit, ITE_ITPP_GENERAL_CFGr, reg_above_64_val, ENABLE_EEI_STAMPINGf, 1);
    soc_reg_above_64_field32_set(unit, ITE_ITPP_GENERAL_CFGr, reg_above_64_val, ENABLE_EEI_ADDINGf, 1);
    soc_reg_above_64_field32_set(unit, ITE_ITPP_GENERAL_CFGr, reg_above_64_val, ALWAYS_CHECK_IP_COMPATIBLE_MCf, 1);
    soc_reg_above_64_field32_set(unit, ITE_ITPP_GENERAL_CFGr, reg_above_64_val, CUD_1_INVALID_VALUEf, 0x0);
    soc_reg_above_64_field32_set(unit, ITE_ITPP_GENERAL_CFGr, reg_above_64_val, ENABLE_TERMINATIONf, 1);
    soc_reg_above_64_field32_set(unit, ITE_ITPP_GENERAL_CFGr, reg_above_64_val, CUD_0_INVALID_VALUEf, 0x0);
    soc_reg_above_64_field32_set(unit, ITE_ITPP_GENERAL_CFGr, reg_above_64_val, ENABLE_FALLBACK_TO_BRIDGEf, 0);
    SOCDNX_IF_ERR_EXIT(soc_reg_above_64_set(unit, ITE_ITPP_GENERAL_CFGr, REG_PORT_ANY, 0, reg_above_64_val));
    SOCDNX_IF_ERR_EXIT(WRITE_TAR_INVALID_CUDr(unit, 0));


    /* Setup FTMH on outbound snooping to be kept from the original and not stamped over */
    SOC_REG_ABOVE_64_CLEAR(reg_above_64_val);
    SOCDNX_IF_ERR_EXIT(soc_reg_above_64_get(unit, ITE_STAMPING_FABRIC_HEADER_ENABLEr, REG_PORT_ANY, 0, reg_above_64_val));
    soc_reg_above_64_field32_set(unit, ITE_STAMPING_FABRIC_HEADER_ENABLEr, reg_above_64_val, BACKWARD_MCID_ENf, 0);
    soc_reg_above_64_field32_set(unit, ITE_STAMPING_FABRIC_HEADER_ENABLEr, reg_above_64_val, BACKWARD_IS_MC_ENf, 0);
    SOCDNX_IF_ERR_EXIT(soc_reg_above_64_set(unit, ITE_STAMPING_FABRIC_HEADER_ENABLEr, REG_PORT_ANY, 0, reg_above_64_val));
    SOC_REG_ABOVE_64_CLEAR(reg_above_64_val);
    SOCDNX_IF_ERR_EXIT(soc_reg_above_64_get(unit, ITE_STAMPING_FTMH_OUTLIF_ENABLEr, REG_PORT_ANY, 0, reg_above_64_val));
    soc_reg_above_64_field32_set(unit, ITE_STAMPING_FTMH_OUTLIF_ENABLEr, reg_above_64_val, STAMP_FTMH_OUTLIF_ENf, 0x31);
    SOCDNX_IF_ERR_EXIT(soc_reg_above_64_set(unit, ITE_STAMPING_FTMH_OUTLIF_ENABLEr, REG_PORT_ANY, 0, reg_above_64_val));

    /*UDH*/
    SOC_REG_ABOVE_64_CLEAR(reg_above_64_val); /* set EGQ_MAP_FLEXIBLE_UDH_TYPE_TO_SIZEr */
    soc_reg_above_64_field32_set(unit, EGQ_MAP_FLEXIBLE_UDH_TYPE_TO_SIZEr, reg_above_64_val, MAP_FLEXIBLE_UDH_0_TYPE_TO_SIZEf, 0x8d1);
    soc_reg_above_64_field32_set(unit, EGQ_MAP_FLEXIBLE_UDH_TYPE_TO_SIZEr, reg_above_64_val, MAP_FLEXIBLE_UDH_3_TYPE_TO_SIZEf, 0x8d1);
    soc_reg_above_64_field32_set(unit, EGQ_MAP_FLEXIBLE_UDH_TYPE_TO_SIZEr, reg_above_64_val, MAP_FLEXIBLE_UDH_1_TYPE_TO_SIZEf, 0x8d1);
    soc_reg_above_64_field32_set(unit, EGQ_MAP_FLEXIBLE_UDH_TYPE_TO_SIZEr, reg_above_64_val, MAP_FLEXIBLE_UDH_2_TYPE_TO_SIZEf, 0x8d1);
    SOCDNX_IF_ERR_EXIT(soc_reg_above_64_set(unit, EGQ_MAP_FLEXIBLE_UDH_TYPE_TO_SIZEr, REG_PORT_ANY, 0, reg_above_64_val));

    SOC_REG_ABOVE_64_CLEAR(reg_above_64_val); /* set EPNI_MAP_FLEXIBLE_UDH_TYPE_TO_SIZEr */
    soc_reg_above_64_field32_set(unit, EPNI_MAP_FLEXIBLE_UDH_TYPE_TO_SIZEr, reg_above_64_val, MAP_FLEXIBLE_UDH_0_TYPE_TO_SIZEf, 0x8d1);
    soc_reg_above_64_field32_set(unit, EPNI_MAP_FLEXIBLE_UDH_TYPE_TO_SIZEr, reg_above_64_val, MAP_FLEXIBLE_UDH_3_TYPE_TO_SIZEf, 0x8d1);
    soc_reg_above_64_field32_set(unit, EPNI_MAP_FLEXIBLE_UDH_TYPE_TO_SIZEr, reg_above_64_val, MAP_FLEXIBLE_UDH_1_TYPE_TO_SIZEf, 0x8d1);
    soc_reg_above_64_field32_set(unit, EPNI_MAP_FLEXIBLE_UDH_TYPE_TO_SIZEr, reg_above_64_val, MAP_FLEXIBLE_UDH_2_TYPE_TO_SIZEf, 0x8d1);
    SOCDNX_IF_ERR_EXIT(soc_reg_above_64_set(unit, EPNI_MAP_FLEXIBLE_UDH_TYPE_TO_SIZEr, REG_PORT_ANY, 0, reg_above_64_val));

    SOC_REG_ABOVE_64_CLEAR(reg_above_64_val); /* set ITE_MAP_UDH_TYPE_TO_LENGTHr[0] */
    SOC_REG_ABOVE_64_CLEAR(reg_field_a64);
    reg_field_a64[0]=0xcc520c41; reg_field_a64[1]=0xd62d4941; reg_field_a64[2]=0x0c4183dc; reg_field_a64[3]=0x4941cc52; reg_field_a64[4]=0x83dcd62d; reg_field_a64[5]=0xcc520c41;reg_field_a64[6]=0xd62d4941; reg_field_a64[7]=0x0c4183dc; reg_field_a64[8]=0x4941cc52; reg_field_a64[9]=0x83dcd62d;
    soc_reg_above_64_field_set(unit, ITE_MAP_UDH_TYPE_TO_LENGTHr, reg_above_64_val, TYPE_TO_LENGTHf, reg_field_a64);
    for (i=0; i<4; i++)
    {
        SOCDNX_IF_ERR_EXIT(soc_reg_above_64_set(unit, ITE_MAP_UDH_TYPE_TO_LENGTHr, REG_PORT_ANY, i, reg_above_64_val));
    }

    SOC_REG_ABOVE_64_CLEAR(reg_above_64_val); /* set ITE_STAMPING_USR_DEF_OUTLIF_ENABLEr */
    soc_reg_above_64_field32_set(unit, ITE_STAMPING_USR_DEF_OUTLIF_ENABLEr, reg_above_64_val, STAMP_USR_DEF_OUTLIF_TYPE_ENf, 0x7770);
    SOCDNX_IF_ERR_EXIT(soc_reg_above_64_set(unit, ITE_STAMPING_USR_DEF_OUTLIF_ENABLEr, REG_PORT_ANY, 0, reg_above_64_val));

    SOC_REG_ABOVE_64_CLEAR(reg_above_64_val); /* set ITE_PPH_RES_EN_TERMr */
    soc_reg_above_64_field32_set(unit, ITE_PPH_RES_EN_TERMr, reg_above_64_val, PPH_RES_EN_TERM_MASKf, 3);
    soc_reg_above_64_field32_set(unit, ITE_PPH_RES_EN_TERMr, reg_above_64_val, PPH_RES_EN_TERMf, 3);
    SOCDNX_IF_ERR_EXIT(soc_reg_above_64_set(unit, ITE_PPH_RES_EN_TERMr, REG_PORT_ANY, 0, reg_above_64_val)); 

    SOC_REG_ABOVE_64_CLEAR(reg_above_64_val); /* set EPNI_CFG_LINK_FIFOS_FIXED_LATENCY_SETTINGr */
    soc_reg_above_64_field32_set(unit, EPNI_CFG_LINK_FIFOS_FIXED_LATENCY_SETTINGr, reg_above_64_val, CFG_LINK_P_16_FIXED_LATENCY_SETTINGf, 0xa);
    soc_reg_above_64_field32_set(unit, EPNI_CFG_LINK_FIFOS_FIXED_LATENCY_SETTINGr, reg_above_64_val, CFG_NATIVE_LINK_P_9_FIXED_LATENCY_SETTINGf, 9);
    soc_reg_above_64_field32_set(unit, EPNI_CFG_LINK_FIFOS_FIXED_LATENCY_SETTINGr, reg_above_64_val, CFG_NATIVE_LINK_P_16_FIXED_LATENCY_SETTINGf, 6);
    soc_reg_above_64_field32_set(unit, EPNI_CFG_LINK_FIFOS_FIXED_LATENCY_SETTINGr, reg_above_64_val, CFG_LINK_P_9_FIXED_LATENCY_SETTINGf, 9);
    SOCDNX_IF_ERR_EXIT(soc_reg_above_64_set(unit, EPNI_CFG_LINK_FIFOS_FIXED_LATENCY_SETTINGr, REG_PORT_ANY, 0, reg_above_64_val));

    /* init native default outlif with invalid value */
    SOCDNX_IF_ERR_EXIT(qax_pp_lif_default_native_ac_outlif_init(unit)); 

exit:
    SOCDNX_FUNC_RETURN;
}

int soc_qax_init_blocks_init_conf(int unit)
{

    SOCDNX_INIT_FUNC_DEFS;

    /* ECI Globals configurations */
    SOCDNX_IF_ERR_EXIT(soc_qax_init_blocks_init_global_conf(unit));

    /* General blocks configurations */
    SOCDNX_IF_ERR_EXIT(soc_qax_init_blocks_init_general_conf(unit));

    /* Configure Sram and Dram buffers */

    /* Configure Sram buffers */
    SOCDNX_IF_ERR_EXIT( soc_qax_sram_conf_set(unit));

    /* Configure Dram Buffers */
    SOCDNX_IF_ERR_EXIT(soc_qax_dram_buffer_conf_set(unit));

    /* Setting revision fixes bits */
    SOCDNX_IF_ERR_EXIT(qax_mgmt_revision_fixes(unit));

    /* Init IPSEC block */
    SOCDNX_IF_ERR_EXIT(soc_qax_ipsec_init(unit));

exit:
    SOCDNX_FUNC_RETURN;
}

/* Clear the portion of the memory entry that is used by the specific memory */
STATIC void clear_entry(int unit, void *entry, soc_mem_t mem) {
    sal_memset(entry, 0, (soc_mem_entry_bytes(unit, mem)+3) & 0xfc);
}
int soc_qax_enable_dynamic_memories(int unit)
{

    SOCDNX_INIT_FUNC_DEFS;

    
    SOCDNX_IF_ERR_EXIT(WRITE_CFC_ENABLE_DYNAMIC_MEMORY_ACCESSr(unit, 1));
    SOCDNX_IF_ERR_EXIT(WRITE_CGM_ENABLE_DYNAMIC_MEMORY_ACCESSr(unit, 1));
    SOCDNX_IF_ERR_EXIT(WRITE_CRPS_ENABLE_DYNAMIC_MEMORY_ACCESSr(unit, 1));
    SOCDNX_IF_ERR_EXIT(WRITE_DDP_ENABLE_DYNAMIC_MEMORY_ACCESSr(unit, 1));
    SOCDNX_IF_ERR_EXIT(WRITE_DQM_ENABLE_DYNAMIC_MEMORY_ACCESSr(unit, 1));
    SOCDNX_IF_ERR_EXIT(WRITE_ECI_ENABLE_DYNAMIC_MEMORY_ACCESSr(unit, 1));
    SOCDNX_IF_ERR_EXIT(WRITE_EDB_ENABLE_DYNAMIC_MEMORY_ACCESSr(unit, 1));
    SOCDNX_IF_ERR_EXIT(WRITE_EGQ_ENABLE_DYNAMIC_MEMORY_ACCESSr(unit, REG_PORT_ANY, 1));
    SOCDNX_IF_ERR_EXIT(WRITE_EPNI_ENABLE_DYNAMIC_MEMORY_ACCESSr(unit, REG_PORT_ANY, 1));
    if (!soc_feature(unit, soc_feature_no_fabric)) {
        SOCDNX_IF_ERR_EXIT(WRITE_FCR_ENABLE_DYNAMIC_MEMORY_ACCESSr(unit, 1));
        SOCDNX_IF_ERR_EXIT(WRITE_FDT_ENABLE_DYNAMIC_MEMORY_ACCESSr(unit, 1));
    }
    SOCDNX_IF_ERR_EXIT(WRITE_IEP_ENABLE_DYNAMIC_MEMORY_ACCESSr(unit, 1));
    SOCDNX_IF_ERR_EXIT(WRITE_IHB_ENABLE_DYNAMIC_MEMORY_ACCESSr(unit, REG_PORT_ANY, 1));
    SOCDNX_IF_ERR_EXIT(WRITE_IHP_ENABLE_DYNAMIC_MEMORY_ACCESSr(unit, REG_PORT_ANY, 1));
    SOCDNX_IF_ERR_EXIT(WRITE_IMP_ENABLE_DYNAMIC_MEMORY_ACCESSr(unit, 1));
    SOCDNX_IF_ERR_EXIT(WRITE_IPSEC_ENABLE_DYNAMIC_MEMORY_ACCESSr(unit, 1));
    SOCDNX_IF_ERR_EXIT(WRITE_IPS_ENABLE_DYNAMIC_MEMORY_ACCESSr(unit, REG_PORT_ANY, 1));
    SOCDNX_IF_ERR_EXIT(WRITE_IRE_ENABLE_DYNAMIC_MEMORY_ACCESSr(unit, 1));
    SOCDNX_IF_ERR_EXIT(WRITE_ITE_ENABLE_DYNAMIC_MEMORY_ACCESSr(unit, 1));
    SOCDNX_IF_ERR_EXIT(WRITE_KAPS_ENABLE_DYNAMIC_MEMORY_ACCESSr(unit, 1));
    SOCDNX_IF_ERR_EXIT(WRITE_MMU_ENABLE_DYNAMIC_MEMORY_ACCESSr(unit, 1));

    if (!SOC_IS_QUX(unit)) {
        SOCDNX_IF_ERR_EXIT(WRITE_NBIH_ENABLE_DYNAMIC_MEMORY_ACCESSr(unit, 1));
        SOCDNX_IF_ERR_EXIT(WRITE_NBIL_ENABLE_DYNAMIC_MEMORY_ACCESSr(unit, 0, 1));
    }
    SOCDNX_IF_ERR_EXIT(WRITE_OAMP_ENABLE_DYNAMIC_MEMORY_ACCESSr(unit, 1));
    SOCDNX_IF_ERR_EXIT(WRITE_OLP_ENABLE_DYNAMIC_MEMORY_ACCESSr(unit, 1));
    SOCDNX_IF_ERR_EXIT(WRITE_PEM_ENABLE_DYNAMIC_MEMORY_ACCESSr(unit, 1));
    SOCDNX_IF_ERR_EXIT(WRITE_PPDB_A_ENABLE_DYNAMIC_MEMORY_ACCESSr(unit, 1));
    SOCDNX_IF_ERR_EXIT(WRITE_PPDB_B_ENABLE_DYNAMIC_MEMORY_ACCESSr(unit, 1));
    SOCDNX_IF_ERR_EXIT(WRITE_PTS_ENABLE_DYNAMIC_MEMORY_ACCESSr(unit, 1));

    SOCDNX_IF_ERR_EXIT(WRITE_SPB_ENABLE_DYNAMIC_MEMORY_ACCESSr(unit, 1));
    SOCDNX_IF_ERR_EXIT(WRITE_SQM_ENABLE_DYNAMIC_MEMORY_ACCESSr(unit, 1));
    SOCDNX_IF_ERR_EXIT(WRITE_TAR_ENABLE_DYNAMIC_MEMORY_ACCESSr(unit, 1));
    SOCDNX_IF_ERR_EXIT(WRITE_TXQ_ENABLE_DYNAMIC_MEMORY_ACCESSr(unit, 1));

exit:
    SOCDNX_FUNC_RETURN;

}

/* QAX define share overrides for emulation */
int soc_qax_init_dpp_defs_overrides(int unit)
{
    SOCDNX_INIT_FUNC_DEFS;
    
    if (SOC_IS_QUX(unit)) {
        SOC_DPP_DEFS_SET(unit, nof_pm4x25, 0);
        SOC_DPP_DEFS_SET(unit, nof_pm4x10, 2);
        SOC_DPP_DEFS_SET(unit, nof_pm4x10q, 0);
        SOC_DPP_DEFS_SET(unit, pmh_base_lane, 0);
        SOC_DPP_DEFS_SET(unit, pml0_base_lane, 0);
        SOC_DPP_DEFS_SET(unit, nof_pms_per_nbi, 9);
        SOC_DPP_DEFS_SET(unit, nof_instances_nbil, 1);
        SOC_DPP_DEFS_SET(unit, nof_lanes_per_nbi, 36);
        SOC_DPP_DEFS_SET(unit, nof_ports_nbih, 0);
        SOC_DPP_DEFS_SET(unit, nof_ports_nbil, 36);
    }
    else {
        SOC_DPP_DEFS_SET(unit, nof_pm4x25, 4);
        SOC_DPP_DEFS_SET(unit, nof_pm4x10, 4);
        SOC_DPP_DEFS_SET(unit, nof_pm4x10q, 3);
        SOC_DPP_DEFS_SET(unit, pmh_base_lane, 0);
        SOC_DPP_DEFS_SET(unit, pml0_base_lane, 16);
        SOC_DPP_DEFS_SET(unit, nof_pms_per_nbi, 4);
        SOC_DPP_DEFS_SET(unit, nof_instances_nbil, 1);
        SOC_DPP_DEFS_SET(unit, nof_lanes_per_nbi, 16);
        SOC_DPP_DEFS_SET(unit, nof_ports_nbih, 16);
        SOC_DPP_DEFS_SET(unit, nof_ports_nbil, 52);
    }

    SOCDNX_FUNC_RETURN;
}

/*
 * DQCQ (PDQ) Configuration -
 * 16 contexts:
 * - Mesh: (Local0, Dest0, Dest1/MC) x SRAM/DRAM x (HP, LP)
 * - Fabric: (Local0, UC, MC) x  SRAM/DRAM x (HP, LP)
 * - For Mesh and Fabric: Delete x SRAM/DRAM + (SRAM to DRAM) x (HP, LP)
 */
STATIC int
soc_qax_dqcf_contexts_init(int unit)
{
    uint32 entry_buf[20];
    soc_reg_above_64_val_t above64;
    int i, num_of_queues_to_init;

    /* In DRAM/SRAM: Queue[n+1].start = Queue[n].start + Queue[n].size + 1 */

    dqcq_init_vals_t dqcq_init_vals[] = {
            /* queue_start, queue_size, dqcq_word_fc_threshold, eir_crdt_fc_threshold, dqcq_fc_threshold*/
            {0x0,   0x1ff,  0x17f4, 0xaa, 0xa9},    /*PDQ_SRAM_LOC_HP 0*/
            {0x0,   0x91,   0x6cc,  0x48, 0x48},    /*PDQ_DRAM_LOC_HP 1*/
            {0x200, 0x1ff,  0x17f4, 0xaa, 0xa9},    /*PDQ_SRAM_LOC_LP 2*/
            {0x92,  0x91,   0x6cc,  0x48, 0x48},    /*PDQ_DRAM_LOC_LP 3*/
            {0x400, 0x1ff,  0x17f4, 0xaa, 0xa9},    /*PDQ_SRAM_FUC_HP 4*/
            {0x124, 0x91,   0x6cc,  0x48, 0x48},    /*PDQ_DRAM_FUC_HP 5*/
            {0x600, 0x1ff,  0x17f4, 0xaa, 0xa9},    /*PDQ_SRAM_FUC_LP 6*/
            {0x1b6, 0x91,   0x6cc,  0x48, 0x48},    /*PDQ_DRAM_FUC_LP 7*/
            {0x800, 0x1ff,  0x17f4, 0xaa, 0xa9},    /*PDQ_SRAM_FMC_HP 8*/
            {0x248, 0x91,   0x6cc,  0x48, 0x48},    /*PDQ_DRAM_FMC_HP 9*/
            {0xa00, 0x1ff,  0x17f4, 0xaa, 0xa9},    /*PDQ_SRAM_FMC_LP 10*/
            {0x2da, 0x91,   0x6cc,  0x48, 0x48},    /*PDQ_DRAM_FMC_LP 11*/
            {0xc00, 0x54,   0x3f0,  0x1c, 0x3a},    /*PDQ_SRAM_DEL 12*/
            {0x36c, 0x91,   0x6cc,  0x48, 0x48},    /*PDQ_DRAM_DEL 13*/
            {0xc55, 0x154,  0xff0,  0x71, 0x70}     /*PDQ_SRAM_TO_DRAM_HP 14*/
    };
    SOCDNX_INIT_FUNC_DEFS;

    num_of_queues_to_init = sizeof(dqcq_init_vals) / sizeof(*dqcq_init_vals);
    for (i = 0; i < num_of_queues_to_init; ++i) {
        clear_entry(unit, entry_buf, PTS_PER_PDQ_CFGm);
        soc_mem_field32_set(unit, PTS_PER_PDQ_CFGm, entry_buf, QUEUE_STARTf, dqcq_init_vals[i].queue_start);
        soc_mem_field32_set(unit, PTS_PER_PDQ_CFGm, entry_buf, QUEUE_SIZEf, dqcq_init_vals[i].queue_size);
        soc_mem_field32_set(unit, PTS_PER_PDQ_CFGm, entry_buf, DQCQ_WORD_FC_THf, dqcq_init_vals[i].dqcq_word_fc_threshold);
        soc_mem_field32_set(unit, PTS_PER_PDQ_CFGm, entry_buf, EIR_CRDT_FC_THf, dqcq_init_vals[i].eir_crdt_fc_threshold);
        soc_mem_field32_set(unit, PTS_PER_PDQ_CFGm, entry_buf, DQCQ_FC_THf, dqcq_init_vals[i].dqcq_fc_threshold);
        SOCDNX_IF_ERR_EXIT(soc_mem_array_write(unit, PTS_PER_PDQ_CFGm, 0, MEM_BLOCK_ALL, i, entry_buf));
    }

    /*PDQ_DRAM_UNUSED 15*/

    /*PDQ_SRAM_TO_DRAM_LP 16*/
    clear_entry(unit, entry_buf, PTS_PER_PDQ_CFGm);
    soc_mem_field32_set(unit, PTS_PER_PDQ_CFGm, entry_buf, QUEUE_STARTf, 0xdaa);
    soc_mem_field32_set(unit, PTS_PER_PDQ_CFGm, entry_buf, QUEUE_SIZEf, 0x254);
    soc_mem_field32_set(unit, PTS_PER_PDQ_CFGm, entry_buf, DQCQ_WORD_FC_THf, 0x1bf0);
    soc_mem_field32_set(unit, PTS_PER_PDQ_CFGm, entry_buf, EIR_CRDT_FC_THf, 0xc6);
    soc_mem_field32_set(unit, PTS_PER_PDQ_CFGm, entry_buf, DQCQ_FC_THf, 0xc5);
    SOCDNX_IF_ERR_EXIT(soc_mem_array_write(unit, PTS_PER_PDQ_CFGm, 0, MEM_BLOCK_ALL, 16, entry_buf));

    /*set PTS_PDQ_MC_THr*/
    SOC_REG_ABOVE_64_CLEAR(above64);
    soc_reg_above_64_field32_set(unit, PTS_PDQ_MC_THr, above64, PDQ_MC_GFMC_DRAM_THf, 0x6c);
    soc_reg_above_64_field32_set(unit, PTS_PDQ_MC_THr, above64, PDQ_MC_BFMC_DRAM_THf, 0x6c);
    soc_reg_above_64_field32_set(unit, PTS_PDQ_MC_THr, above64, PDQ_MC_GFMC_SRAM_THf, 0x17f);
    soc_reg_above_64_field32_set(unit, PTS_PDQ_MC_THr, above64, PDQ_MC_BFMC_SRAM_THf, 0x17f);
    SOCDNX_IF_ERR_EXIT(soc_reg_above_64_set(unit, PTS_PDQ_MC_THr, REG_PORT_ANY, 0, above64));

    /*PTS_PDQ_OVTH_FC_MASKr has a field per DQCQ (PDQ), which holds a mask: from which DTQs it can receive flow-control notifications.
    Per DTQ: there are 2 configurable flow-control thresholds sent to PDQs, in TXQ_PER_DTQ_CFGm: DTQ_PDQ_FC_1_THf (a little full) DTQ_PDQ_FC_2_THf (very full).
    The mask- from bit0: First threshold:            DTQ SRAM0, DTQ DRAM0, DTQ SRAM1, DTQ DRAM1, DTQ SRAM2, DTQ DRAM2,
    Followed by second threshold in the same manner: DTQ SRAM0, DTQ DRAM0, ...
    Local, SRAM->DRAM and UC PDQs always have the same mask, independent of DTQ mode.
    MC mask depends on DTQ mode - set in DTQ functions*/
    SOCDNX_IF_ERR_EXIT(soc_reg_above_64_get(unit, PTS_PDQ_OVTH_FC_MASKr, REG_PORT_ANY, 0, above64));

    /*Local PDQ gets FC: SRAM PDQ from SRAM DTQ, DRAM from DRAM. HP gets only second threshold FC, LP gets both thresholds*/
    soc_reg_above_64_field32_set(unit, PTS_PDQ_OVTH_FC_MASKr, above64, SRAM_LOC_HP_OVTH_MASKf, 0x4);
    soc_reg_above_64_field32_set(unit, PTS_PDQ_OVTH_FC_MASKr, above64, DRAM_LOC_HP_OVTH_MASKf, 0x8);
    soc_reg_above_64_field32_set(unit, PTS_PDQ_OVTH_FC_MASKr, above64, SRAM_LOC_LP_OVTH_MASKf, 0x5);
    soc_reg_above_64_field32_set(unit, PTS_PDQ_OVTH_FC_MASKr, above64, DRAM_LOC_LP_OVTH_MASKf, 0xa);

    /*SRAM to DRAM*/
    soc_reg_above_64_field32_set(unit, PTS_PDQ_OVTH_FC_MASKr, above64, SRAM_TO_DRAM_HP_OVTH_MASKf, 0x1);
    soc_reg_above_64_field32_set(unit, PTS_PDQ_OVTH_FC_MASKr, above64, SRAM_TO_DRAM_LP_OVTH_MASKf, 0x1);

    /*UC PDQ gets FC from first DTQ, which is always UC: SRAM PDQ SRAM DTQ, DRAM from DRAM. HP gets only second threshold FC, LP gets both thresholds*/
    soc_reg_above_64_field32_set(unit, PTS_PDQ_OVTH_FC_MASKr, above64, SRAM_FUC_HP_OVTH_MASKf, 0x40);
    soc_reg_above_64_field32_set(unit, PTS_PDQ_OVTH_FC_MASKr, above64, DRAM_FUC_HP_OVTH_MASKf, 0x80);
    soc_reg_above_64_field32_set(unit, PTS_PDQ_OVTH_FC_MASKr, above64, SRAM_FUC_LP_OVTH_MASKf, 0x41);
    soc_reg_above_64_field32_set(unit, PTS_PDQ_OVTH_FC_MASKr, above64, DRAM_FUC_LP_OVTH_MASKf, 0x82);

    SOCDNX_IF_ERR_EXIT(soc_reg_above_64_set(unit, PTS_PDQ_OVTH_FC_MASKr, REG_PORT_ANY, 0, above64));

    exit:
        SOCDNX_FUNC_RETURN;
}

STATIC int
soc_qax_mc_dqcf_flow_control_recieve_mask_init(int unit, int sram_fmc_hp_mask, int dram_fmc_hp_mask, int sram_fmc_lp_mask, int dram_fmc_lp_mask)
{
    soc_reg_above_64_val_t above64;
    SOCDNX_INIT_FUNC_DEFS;

    SOCDNX_IF_ERR_EXIT(soc_reg_above_64_get(unit, PTS_PDQ_OVTH_FC_MASKr, REG_PORT_ANY, 0, above64));
    soc_reg_above_64_field32_set(unit, PTS_PDQ_OVTH_FC_MASKr, above64, SRAM_FMC_HP_OVTH_MASKf, sram_fmc_hp_mask);
    soc_reg_above_64_field32_set(unit, PTS_PDQ_OVTH_FC_MASKr, above64, DRAM_FMC_HP_OVTH_MASKf, dram_fmc_hp_mask);
    soc_reg_above_64_field32_set(unit, PTS_PDQ_OVTH_FC_MASKr, above64, SRAM_FMC_LP_OVTH_MASKf, sram_fmc_lp_mask);
    soc_reg_above_64_field32_set(unit, PTS_PDQ_OVTH_FC_MASKr, above64, DRAM_FMC_LP_OVTH_MASKf, dram_fmc_lp_mask);
    SOCDNX_IF_ERR_EXIT(soc_reg_above_64_set(unit, PTS_PDQ_OVTH_FC_MASKr, REG_PORT_ANY, 0, above64));

    exit:
        SOCDNX_FUNC_RETURN;
}

STATIC int
soc_qax_dtq_single_context_init(int unit, int queue_start, int queue_size, int fc_2_threshold, int fc_1_threshold, int spr_fc_threshold, int queue_index)
{
    uint32 entry_buf[20];
    SOCDNX_INIT_FUNC_DEFS;

    clear_entry(unit, entry_buf, TXQ_PER_DTQ_CFGm);
    soc_mem_field32_set(unit, TXQ_PER_DTQ_CFGm, entry_buf, QUEUE_STARTf, queue_start);
    soc_mem_field32_set(unit, TXQ_PER_DTQ_CFGm, entry_buf, QUEUE_SIZEf, queue_size);
    soc_mem_field32_set(unit, TXQ_PER_DTQ_CFGm, entry_buf, DTQ_PDQ_FC_2_THf, fc_2_threshold);
    soc_mem_field32_set(unit, TXQ_PER_DTQ_CFGm, entry_buf, DTQ_PDQ_FC_1_THf, fc_1_threshold);
    soc_mem_field32_set(unit, TXQ_PER_DTQ_CFGm, entry_buf, DTQ_SPR_FC_THf, spr_fc_threshold);
    SOCDNX_IF_ERR_EXIT(soc_mem_array_write(unit, TXQ_PER_DTQ_CFGm, 0, MEM_BLOCK_ALL, queue_index, entry_buf));

    exit:
        SOCDNX_FUNC_RETURN;
}

/* Contexts:
 *  UC x (SRAM/DRAM) - queues 0, 1
 *  MC x (SRAM/DRAM) - queues 4, 5
 */
STATIC int
soc_qax_dtq_contexts_init_mesh(int unit)
{
    SOCDNX_INIT_FUNC_DEFS;

    /*TXQ DTQ SRAM 0*/
    SOCDNX_IF_ERR_EXIT(soc_qax_dtq_single_context_init(unit, 0, 128, 90, 65, 90, 0));
    /*TXQ DTQ DRAM 0*/
    SOCDNX_IF_ERR_EXIT(soc_qax_dtq_single_context_init(unit, 0, 767, 356, 256, 356, 1));

    /*TXQ DTQ SRAM 2*/
    SOCDNX_IF_ERR_EXIT(soc_qax_dtq_single_context_init(unit, 129, 128, 89, 64, 89, 4));
    /*TXQ DTQ DRAM 2*/
    SOCDNX_IF_ERR_EXIT(soc_qax_dtq_single_context_init(unit, 768, 255, 179, 128, 179, 5));

    /*init MC PDQs' FC masks- from which DTQ each PDQ can receive FC and whether it receives FC notification when DTQ is a little full (TH1) and/or very full (TH2)*/
    SOCDNX_IF_ERR_EXIT(soc_qax_mc_dqcf_flow_control_recieve_mask_init(unit, 0x400, 0x800, 0x410, 0x820));

    exit:
        SOCDNX_FUNC_RETURN;
}

/* Contexts:
 *  All traffic x (SRAM/DRAM) - queues 0, 1
 */
STATIC int
soc_qax_dtq_contexts_init_single_queue_clos(int unit)
{
    SOCDNX_INIT_FUNC_DEFS;

    /*TXQ DTQ SRAM 0*/
    SOCDNX_IF_ERR_EXIT(soc_qax_dtq_single_context_init(unit, 0, 255, 205, 154, 205, 0));

    /*TXQ DTQ DRAM 0*/
    SOCDNX_IF_ERR_EXIT(soc_qax_dtq_single_context_init(unit, 0, 1023, 820, 615, 820, 1));

    /*init MC PDQs' FC masks- from which DTQ each PDQ can receive FC and whether it receives FC notification when DTQ is a little full (TH1) and/or very full (TH2)*/
    SOCDNX_IF_ERR_EXIT(soc_qax_mc_dqcf_flow_control_recieve_mask_init(unit, 0x40, 0x80, 0x41, 0x82));

    exit:
        SOCDNX_FUNC_RETURN;
}

/* Contexts:
 *  UC x (SRAM/DRAM) - queues 0, 1
 *  MC x (SRAM/DRAM) - queues 2, 3
 */
STATIC int
soc_qax_dtq_contexts_init_dual_queue_clos(int unit)
{
    SOCDNX_INIT_FUNC_DEFS;

    /*TXQ DTQ SRAM 0*/
    SOCDNX_IF_ERR_EXIT(soc_qax_dtq_single_context_init(unit, 0, 127, 89, 64, 89, 0));
    /*TXQ DTQ DRAM 0*/
    SOCDNX_IF_ERR_EXIT(soc_qax_dtq_single_context_init(unit, 0, 511, 409, 307, 409, 1));

    /*TXQ DTQ SRAM 1*/
    SOCDNX_IF_ERR_EXIT(soc_qax_dtq_single_context_init(unit, 128, 127, 89, 64, 89, 2));
    /*TXQ DTQ DRAM 1*/
    SOCDNX_IF_ERR_EXIT(soc_qax_dtq_single_context_init(unit, 512, 511, 409, 307, 409, 3));

    /*init MC PDQs' FC masks- from which DTQ each PDQ can receive FC and whether it receives FC notification when DTQ is a little full (TH1) and/or very full (TH2)*/
    SOCDNX_IF_ERR_EXIT(soc_qax_mc_dqcf_flow_control_recieve_mask_init(unit, 0x100, 0x200, 0x104, 0x208));

    exit:
        SOCDNX_FUNC_RETURN;
}

/* Contexts:
 *  UC x (SRAM/DRAM) - queues 0, 1
 *  MC_HP x (SRAM/DRAM) - queues 2, 3
 *  MC_LP x (SRAM/DRAM) - queues 4, 5
 */
STATIC int
soc_qax_dtq_contexts_init_triple_queue_clos(int unit)
{
    SOCDNX_INIT_FUNC_DEFS;

    /*TXQ DTQ SRAM 0*/
    SOCDNX_IF_ERR_EXIT(soc_qax_dtq_single_context_init(unit, 0, 127, 89, 64, 89, 0));
    /*TXQ DTQ DRAM 0*/
    SOCDNX_IF_ERR_EXIT(soc_qax_dtq_single_context_init(unit, 0, 511, 409, 307, 409, 1));

    /*TXQ DTQ SRAM 1*/
    SOCDNX_IF_ERR_EXIT(soc_qax_dtq_single_context_init(unit, 128, 63, 51, 38, 51, 2));
    /*TXQ DTQ DRAM 1*/
    SOCDNX_IF_ERR_EXIT(soc_qax_dtq_single_context_init(unit, 512, 255, 205, 154, 205, 3));

    /*TXQ DTQ SRAM 2*/
    SOCDNX_IF_ERR_EXIT(soc_qax_dtq_single_context_init(unit, 192, 63, 51, 38, 51, 4));
    /*TXQ DTQ DRAM 2*/
    SOCDNX_IF_ERR_EXIT(soc_qax_dtq_single_context_init(unit, 768, 255, 205, 154, 205, 5));

    /*init MC PDQs' FC masks- from which DTQ each PDQ can receive FC and whether it receives FC notification when DTQ is a little full (TH1) and/or very full (TH2)*/
    SOCDNX_IF_ERR_EXIT(soc_qax_mc_dqcf_flow_control_recieve_mask_init(unit, 0x100, 0x200, 0x10, 0x20));

    exit:
        SOCDNX_FUNC_RETURN;
}


STATIC int
soc_qax_dtq_contexts_init(int unit)
{
    uint32 reg32_val;
    soc_reg_above_64_val_t above64;
    soc_dcmn_fabric_pipe_map_type_t fabric_pipe_map_type;
    soc_fabric_dtq_mode_type_t dtq_mode = SOC_FABRIC_DTQ_MODE_UC_HMC_LMC;  /*same as HW default*/
    int i;
    SOCDNX_INIT_FUNC_DEFS;

    fabric_pipe_map_type = SOC_DPP_CONFIG(unit)->arad->init.fabric.fabric_pipe_map_config.mapping_type;

    /* In unused queues all values of TXQ_PER_DTQ_CFGm are HW set to 0 => Setting TH1 and TH2 to be positive to avoid flow control from unused queues.
       Otherwise queue content(0) >= FC threshold (0) and flow control is issued, causing packets to be stuck at PDQ.
       Used queues initialization by soc_qax_dtq_contexts_init_ follows */
    for (i = 0; i < QAX_INIT_MAX_NUM_OF_DTQS; ++i) {
        SOCDNX_IF_ERR_EXIT(soc_qax_dtq_single_context_init(unit, 0, 0,  QAX_INIT_UNUSED_DTQ_PDQ_FC_TH, QAX_INIT_UNUSED_DTQ_PDQ_FC_TH, 0, i));
    }

    if (SOC_DPP_IS_MESH(unit)){
        dtq_mode = SOC_FABRIC_DTQ_MODE_UC_HMC_LMC; /* In MESH, only UC/not used/MC is supported */
        SOCDNX_IF_ERR_EXIT(soc_qax_dtq_contexts_init_mesh(unit));

    } else if (SOC_DPP_CONFIG(unit)->arad->init.fabric.connect_mode == ARAD_FABRIC_CONNECT_MODE_FE) {

        switch (fabric_pipe_map_type) { /* TDM traffic goes straight to FDT (bypass) without entering DTQs, so DTQ_MODE ignores TDM */
        case soc_dcmn_fabric_pipe_map_triple_uc_hp_mc_lp_mc:
            dtq_mode = SOC_FABRIC_DTQ_MODE_UC_HMC_LMC;
            SOCDNX_IF_ERR_EXIT(soc_qax_dtq_contexts_init_triple_queue_clos(unit));
            break;

        case soc_dcmn_fabric_pipe_map_triple_uc_mc_tdm:
        case soc_dcmn_fabric_pipe_map_dual_uc_mc:
            dtq_mode = SOC_FABRIC_DTQ_MODE_UC_MC;
            SOCDNX_IF_ERR_EXIT(soc_qax_dtq_contexts_init_dual_queue_clos(unit));
            break;

        case soc_dcmn_fabric_pipe_map_dual_tdm_non_tdm:
        case soc_dcmn_fabric_pipe_map_single:
            dtq_mode = SOC_FABRIC_DTQ_MODE_SINGLE_QUEUE;
            SOCDNX_IF_ERR_EXIT(soc_qax_dtq_contexts_init_single_queue_clos(unit));
            break;

        case soc_dcmn_fabric_pipe_map_triple_custom:
        case soc_dcmn_fabric_pipe_map_dual_custom:
        default:
            SOCDNX_EXIT_WITH_ERR(SOC_E_CONFIG, (_BSL_SOC_MSG("custom mode is not supported")));
            break;
        }
    }

    if (SOC_DPP_CONFIG(unit)->arad->init.fabric.connect_mode != ARAD_FABRIC_CONNECT_MODE_SINGLE_FAP) {
        SOCDNX_IF_ERR_EXIT(READ_TXQ_TXQ_GENERAL_CONFIGURATIONr(unit, &reg32_val));
        /* Set DTQ mode according to map_type: 0 = Single Queue, 1 = UC/MC, 2 = UC/HMC/LMC*/
        soc_reg_field_set(unit, TXQ_TXQ_GENERAL_CONFIGURATIONr, &reg32_val, DTQ_MODEf, dtq_mode);
        /*Since we guarantee each dtq queue is going to a specific, unique, pipe -> we can always enable Q0Q12InterleaveEn and Q1Q2InterleaveEn -> all (max) 3 queues can interleave*/
        soc_reg_field_set(unit, TXQ_TXQ_GENERAL_CONFIGURATIONr, &reg32_val, Q_0_Q_12_INTERLEAVE_ENf, 1);
        soc_reg_field_set(unit, TXQ_TXQ_GENERAL_CONFIGURATIONr, &reg32_val, Q_1_Q_2_INTERLEAVE_ENf, 1);
        /*To avoid blocking scenario: SRAM DTQs will send packets to FDT before end-of-bundle(burst), e.g. before receiving the whole bundle. DRAM will send only after end-of-bundle*/
        soc_reg_field_set(unit, TXQ_TXQ_GENERAL_CONFIGURATIONr, &reg32_val, DTQ_DEQUEUE_WHEN_EOB_ON_QUEUEf, 42); /* 101010 */
        soc_reg_field_set(unit, TXQ_TXQ_GENERAL_CONFIGURATIONr, &reg32_val, DTQ_IS_MC_BMAPf, 0);
        SOCDNX_IF_ERR_EXIT(WRITE_TXQ_TXQ_GENERAL_CONFIGURATIONr(unit, reg32_val));
    }

    SOC_REG_ABOVE_64_CLEAR(above64); /* set TXQ_LOCAL_FIFO_CFGr */
    soc_reg_above_64_field32_set(unit, TXQ_LOCAL_FIFO_CFGr, above64, SRAM_DTQ_LOC_SPR_FC_THf, 64);
    soc_reg_above_64_field32_set(unit, TXQ_LOCAL_FIFO_CFGr, above64, SRAM_DTQ_LOC_PDQ_FC_TH_1f, 48);
    soc_reg_above_64_field32_set(unit, TXQ_LOCAL_FIFO_CFGr, above64, SRAM_DTQ_LOC_PDQ_FC_TH_2f, 64);

    soc_reg_above_64_field32_set(unit, TXQ_LOCAL_FIFO_CFGr, above64, DRAM_DTQ_LOC_SPR_FC_THf, 102);
    soc_reg_above_64_field32_set(unit, TXQ_LOCAL_FIFO_CFGr, above64, DRAM_DTQ_LOC_PDQ_FC_TH_1f, 76);
    soc_reg_above_64_field32_set(unit, TXQ_LOCAL_FIFO_CFGr, above64, DRAM_DTQ_LOC_PDQ_FC_TH_2f, 102);

    /*Local DTQs to FDA flow control*/
    soc_reg_above_64_field32_set(unit, TXQ_LOCAL_FIFO_CFGr, above64, SRAM_DTQ_LOC_EGQ_FC_THf, 72);
    soc_reg_above_64_field32_set(unit, TXQ_LOCAL_FIFO_CFGr, above64, SRAM_DTQ_LOC_GEN_RCI_THf, 72);

    soc_reg_above_64_field32_set(unit, TXQ_LOCAL_FIFO_CFGr, above64, DRAM_DTQ_LOC_EGQ_FC_THf, 115);
    soc_reg_above_64_field32_set(unit, TXQ_LOCAL_FIFO_CFGr, above64, DRAM_DTQ_LOC_GEN_RCI_THf, 115);

    SOCDNX_IF_ERR_EXIT(soc_reg_above_64_set(unit, TXQ_LOCAL_FIFO_CFGr, REG_PORT_ANY, 0, above64));

    exit:
       SOCDNX_FUNC_RETURN;
}


uint32
soc_qax_pdq_dtq_contexts_init(
  SOC_SAND_IN int     unit)
{
    soc_reg_above_64_val_t above64;
    uint32 reg32;
    int i;
    SOCDNX_INIT_FUNC_DEFS;

    /* IPT dropping due to large latency feature is by default not supported */
    for(i = 0; i < 8; ++i) { /*8 such registers*/
        SOCDNX_IF_ERR_EXIT(READ_ITE_PACKET_LATENCY_MEASUREMENTr(unit, i, above64));
        soc_reg_above_64_field32_set(unit, ITE_PACKET_LATENCY_MEASUREMENTr, above64, EN_LATENCY_DROPf, 0);
        soc_reg_above_64_field32_set(unit, ITE_PACKET_LATENCY_MEASUREMENTr, above64, EN_LATENCY_ERR_REPORTf, 0);
        SOCDNX_IF_ERR_EXIT(WRITE_ITE_PACKET_LATENCY_MEASUREMENTr(unit, i, above64));
    }

    /*Enable GCI*/
    SOCDNX_IF_ERR_EXIT(READ_PTS_PTS_DEBUG_CONTROLSr(unit, &reg32));
    soc_reg_field_set(unit, PTS_PTS_DEBUG_CONTROLSr, &reg32, GCI_ENf, 0x1);
    SOCDNX_IF_ERR_EXIT(WRITE_PTS_PTS_DEBUG_CONTROLSr(unit, reg32));

    /*configure DTQ contexts*/
    SOCDNX_IF_ERR_EXIT(soc_qax_dtq_contexts_init(unit));

    /*configure DQCF (PDQ) contexts*/
    SOCDNX_IF_ERR_EXIT(soc_qax_dqcf_contexts_init(unit));

    if (SOC_IS_QAX_WITH_FABRIC_ENABLED(unit)) {
        /*Allow DRAM and SRAM contexts from same Q-Pair of the following buffers to interleave between them: FMC-LP,FMC-HP,FUC-LP,FUC-HP*/
        SOCDNX_IF_ERR_EXIT(READ_PTS_PACKET_DEQUEUE_GENERAL_CONFIGSr(unit, &reg32));
        soc_reg_field_set(unit, PTS_PACKET_DEQUEUE_GENERAL_CONFIGSr, &reg32, FAB_MEM_INTERLEAVE_ENf, 0xf);
        SOCDNX_IF_ERR_EXIT(WRITE_PTS_PACKET_DEQUEUE_GENERAL_CONFIGSr(unit, reg32));
    }

    exit:
        SOCDNX_FUNC_RETURN;
}
