/*
 * $Id: reset.c, v1 06/06/2016 kkotsev $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 */

/*************
 * INCLUDES  *
 *************/
#if (BCM_DPP_SUPPORT)
/* { */
/* SOC dcmn includes */
#include <soc/dcmn/error.h>
#include <soc/dcmn/dcmn_cmic.h>
#include <soc/dcmn/dcmn_iproc.h>
/* SOC DPP includes */
#include <soc/dpp/drv.h>
#include <soc/dnx/drv.h>
#include <soc/dpp/mbcm.h>


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
/* 
 * take blocks in or out of sbus reset: 
 * is_in_reset = 1 in reset
 * is_in_reset = 0 out of reset
 */
STATIC int soc_dnx_reset_sbus_reset(int unit, int is_in_reset)
{
    soc_reg_above_64_val_t reg_above_64_val;
    SOCDNX_INIT_FUNC_DEFS;
	/* IQMT */     
	SOCDNX_IF_ERR_EXIT(READ_ECI_BLOCKS_SBUS_RESETr(unit, reg_above_64_val));
	soc_reg_above_64_field32_set(unit, ECI_BLOCKS_SBUS_RESETr, reg_above_64_val, BLOCKS_SBUS_RESET_21f, is_in_reset);
	SOCDNX_IF_ERR_EXIT(WRITE_ECI_BLOCKS_SBUS_RESETr(unit, reg_above_64_val));
	/* IQMs */     
	SOCDNX_IF_ERR_EXIT(READ_ECI_BLOCKS_SBUS_RESETr(unit, reg_above_64_val));
	soc_reg_above_64_field32_set(unit, ECI_BLOCKS_SBUS_RESETr, reg_above_64_val, BLOCKS_SBUS_RESET_19f, is_in_reset);
	soc_reg_above_64_field32_set(unit, ECI_BLOCKS_SBUS_RESETr, reg_above_64_val, BLOCKS_SBUS_RESET_20f, is_in_reset);
	SOCDNX_IF_ERR_EXIT(WRITE_ECI_BLOCKS_SBUS_RESETr(unit, reg_above_64_val));
    /* all the rest */
    if (is_in_reset) {
        SOC_REG_ABOVE_64_ALLONES(reg_above_64_val);
    } else {
        SOC_REG_ABOVE_64_CLEAR(reg_above_64_val);
    }
    SOCDNX_IF_ERR_EXIT(WRITE_ECI_BLOCKS_SBUS_RESETr(unit, reg_above_64_val));

exit:
    SOCDNX_FUNC_RETURN;
}
int soc_dnx_reset_blocks_poll_init_finish(int unit)
{
    int i;

    SOCDNX_INIT_FUNC_DEFS;
    
    for(i = 0; i < SOC_DPP_CONFIG(unit)->core_mode.nof_active_cores; i++) {
        /* ING */
        SOCDNX_IF_ERR_EXIT(soc_dpp_polling(unit, ARAD_TIMEOUT, ARAD_MIN_POLLS, IRE_RESET_STATUS_REGISTERr, i, 0, CTXT_STATUS_INIT_DONEf, 0x1));
        if (!SOC_IS_QAX(unit)) { 
            SOCDNX_IF_ERR_EXIT(soc_dpp_polling(unit, ARAD_TIMEOUT, ARAD_MIN_POLLS, IRE_RESET_STATUS_REGISTERr, i, 0, CTXT_MAP_INIT_DONEf, 0x1));

            SOCDNX_IF_ERR_EXIT(soc_dpp_polling(unit, ARAD_TIMEOUT, ARAD_MIN_POLLS, IDR_RESET_STATUS_REGISTERr, i, 0, CONTEXT_STATUS_INIT_DONEf, 0x1));
            SOCDNX_IF_ERR_EXIT(soc_dpp_polling(unit, ARAD_TIMEOUT, ARAD_MIN_POLLS, IDR_RESET_STATUS_REGISTERr, i, 0, CHUNK_STATUS_INIT_DONEf, 0x1));
            SOCDNX_IF_ERR_EXIT(soc_dpp_polling(unit, ARAD_TIMEOUT, ARAD_MIN_POLLS, IDR_RESET_STATUS_REGISTERr, i, 0, WORD_INDEX_FIFO_INIT_DONEf, 0x1));
            SOCDNX_IF_ERR_EXIT(soc_dpp_polling(unit, ARAD_TIMEOUT, ARAD_MIN_POLLS, IDR_RESET_STATUS_REGISTERr, i, 0, FREE_PCB_FIFO_INIT_DONEf, 0x1));

            SOCDNX_IF_ERR_EXIT(soc_dpp_polling(unit, ARAD_TIMEOUT, ARAD_MIN_POLLS, IRR_RESET_STATUS_REGISTERr, i, 0, FPF_0_INIT_DONEf, 0x1));
            SOCDNX_IF_ERR_EXIT(soc_dpp_polling(unit, ARAD_TIMEOUT, ARAD_MIN_POLLS, IRR_RESET_STATUS_REGISTERr, i, 0, FPF_1_INIT_DONEf, 0x1));
            SOCDNX_IF_ERR_EXIT(soc_dpp_polling(unit, ARAD_TIMEOUT, ARAD_MIN_POLLS, IRR_RESET_STATUS_REGISTERr, i, 0, IS_FPF_0_INIT_DONEf, 0x1));
            SOCDNX_IF_ERR_EXIT(soc_dpp_polling(unit, ARAD_TIMEOUT, ARAD_MIN_POLLS, IRR_RESET_STATUS_REGISTERr, i, 0, IS_FPF_1_INIT_DONEf, 0x1));
            SOCDNX_IF_ERR_EXIT(soc_dpp_polling(unit, ARAD_TIMEOUT, ARAD_MIN_POLLS, IRR_RESET_STATUS_REGISTERr, i, 0, DESTINATION_TABLE_INIT_DONEf, 0x1));
            SOCDNX_IF_ERR_EXIT(soc_dpp_polling(unit, ARAD_TIMEOUT, ARAD_MIN_POLLS, IRR_RESET_STATUS_REGISTERr, i, 0, LAG_MAPPING_INIT_DONEf, 0x1));
            SOCDNX_IF_ERR_EXIT(soc_dpp_polling(unit, ARAD_TIMEOUT, ARAD_MIN_POLLS, IRR_RESET_STATUS_REGISTERr, i, 0, LAG_TO_LAG_RANGE_INIT_DONEf, 0x1));
            SOCDNX_IF_ERR_EXIT(soc_dpp_polling(unit, ARAD_TIMEOUT, ARAD_MIN_POLLS, IRR_RESET_STATUS_REGISTERr, i, 0, MCDB_INIT_DONEf, 0x1));
            SOCDNX_IF_ERR_EXIT(soc_dpp_polling(unit, ARAD_TIMEOUT, ARAD_MIN_POLLS, IRR_RESET_STATUS_REGISTERr, i, 0, STACK_FEC_RESOLVE_INIT_DONEf, 0x1));
            SOCDNX_IF_ERR_EXIT(soc_dpp_polling(unit, ARAD_TIMEOUT, ARAD_MIN_POLLS, IRR_RESET_STATUS_REGISTERr, i, 0, STACK_TRUNK_RESOLVE_INIT_DONEf, 0x1));

            SOCDNX_IF_ERR_EXIT(soc_dpp_polling(unit, ARAD_TIMEOUT, ARAD_MIN_POLLS, IQM_IQM_INITr, i, 0, IQC_INITf, 0x0));
            SOCDNX_IF_ERR_EXIT(soc_dpp_polling(unit, ARAD_TIMEOUT, ARAD_MIN_POLLS, IQM_IQM_INITr, i, 0, STE_INITf, 0x0));
            SOCDNX_IF_ERR_EXIT(soc_dpp_polling(unit, ARAD_TIMEOUT, ARAD_MIN_POLLS, IQM_IQM_INITr, i, 0, PDM_INITf, 0x0));

            SOCDNX_IF_ERR_EXIT(soc_dpp_polling(unit, ARAD_TIMEOUT, ARAD_MIN_POLLS, MRPS_INIT_SEQ_ONr, i, 0, MCDA_INIT_SEQ_ONf, 0x1));
            SOCDNX_IF_ERR_EXIT(soc_dpp_polling(unit, ARAD_TIMEOUT, ARAD_MIN_POLLS, MTRPS_EM_INIT_SEQ_ONr, i, 0, MCDA_INIT_SEQ_ONf, 0x1));
            SOCDNX_IF_ERR_EXIT(soc_dpp_polling(unit, ARAD_TIMEOUT, ARAD_MIN_POLLS, IHB_ISEM_RESET_STATUS_REGISTERr, i, 0, ISEM_KEYT_RESET_DONEf, 0x1));
        }

        SOCDNX_IF_ERR_EXIT(soc_dpp_polling(unit, ARAD_TIMEOUT, ARAD_MIN_POLLS, IPS_IPS_GENERAL_CONFIGURATIONSr, i, 0, IPS_INIT_TRIGGERf, 0x0));
        
        /* EGR */
        SOCDNX_IF_ERR_EXIT(soc_dpp_polling(unit, ARAD_TIMEOUT, ARAD_MIN_POLLS, EGQ_EGQ_BLOCK_INIT_STATUSr, i, 0, EGQ_BLOCK_INITf, 0x0));
    }
    
    if (!SOC_IS_FLAIR(unit)) {
        /* ING */
        SOCDNX_IF_ERR_EXIT(soc_dpp_polling(unit, ARAD_TIMEOUT, ARAD_MIN_POLLS, PPDB_A_OEMA_RESET_STATUS_REGISTERr, REG_PORT_ANY, 0, OEMA_KEYT_RESET_DONEf, 0x1));
        SOCDNX_IF_ERR_EXIT(soc_dpp_polling(unit, ARAD_TIMEOUT, ARAD_MIN_POLLS, PPDB_A_OEMB_RESET_STATUS_REGISTERr, REG_PORT_ANY, 0, OEMB_KEYT_RESET_DONEf, 0x1));

        SOCDNX_IF_ERR_EXIT(soc_dpp_polling(unit, ARAD_TIMEOUT, ARAD_MIN_POLLS, PPDB_B_LARGE_EM_RESET_STATUS_REGISTERr, REG_PORT_ANY, 0, LARGE_EM_KEYT_RESET_DONEf, 0x1));

        if (!SOC_IS_JERICHO_PLUS_A0(unit)) {
            SOCDNX_IF_ERR_EXIT(soc_dpp_polling(unit, ARAD_TIMEOUT, ARAD_MIN_POLLS, EDB_GLEM_RESET_STATUS_REGISTERr, REG_PORT_ANY, 0, GLEM_KEYT_RESET_DONEf, 0x1)); 
            SOCDNX_IF_ERR_EXIT(soc_dpp_polling(unit, ARAD_TIMEOUT, ARAD_MIN_POLLS, EDB_ESEM_RESET_STATUS_REGISTERr, REG_PORT_ANY, 0, ESEM_KEYT_RESET_DONEf, 0x1));
        }

        SOCDNX_IF_ERR_EXIT(soc_dpp_polling(unit, ARAD_TIMEOUT, ARAD_MIN_POLLS, OAMP_REMOTE_MEP_EXACT_MATCH_RESET_STATUS_REGISTERr, REG_PORT_ANY, 0, REMOTE_MEP_EXACT_MATCH_KEYT_RESET_DONEf, 0x1));
    }

exit:
    SOCDNX_FUNC_RETURN;  
}

STATIC int soc_dnx_reset_blocks_reset(int unit, int is_in_reset)
{
    int disable_hard_reset;
    uint32 reg_val;
    soc_reg_above_64_val_t reg_above_64_val;
    soc_field_t field;
    SOCDNX_INIT_FUNC_DEFS;
    /* All the rest */
    if (is_in_reset) {
        SOC_REG_ABOVE_64_ALLONES(reg_above_64_val);
    } else {
        SOC_REG_ABOVE_64_CLEAR(reg_above_64_val);
    }
    SOCDNX_IF_ERR_EXIT(WRITE_ECI_BLOCKS_SOFT_RESETr(unit, reg_above_64_val));

    /* Take PDM in or out of reset */
    if (SOC_REG_IS_VALID(unit,ECI_GP_CONTROL_9r)) {
        SOCDNX_IF_ERR_EXIT(READ_ECI_GP_CONTROL_9r(unit, reg_above_64_val));
        soc_reg_above_64_field32_set(unit, ECI_GP_CONTROL_9r, reg_above_64_val, PDM_RSTNf, is_in_reset ? 0 : 1);
        SOCDNX_IF_ERR_EXIT(WRITE_ECI_GP_CONTROL_9r(unit, reg_above_64_val));
    }

    /* Take MBU in or out of reset */
    SOCDNX_IF_ERR_EXIT(READ_ECI_ECIC_BLOCKS_RESETr(unit, &reg_val));
    field = SOC_IS_DNX(unit) ? PNIMI_002f : FIELD_0_0f;
    soc_reg_field_set(unit, ECI_ECIC_BLOCKS_RESETr, &reg_val,field , is_in_reset);
    if (soc_reg_field_valid(unit, ECI_ECIC_BLOCKS_RESETr, TIME_SYNC_RESETf)) {
        soc_reg_field_set(unit, ECI_ECIC_BLOCKS_RESETr, &reg_val, TIME_SYNC_RESETf, is_in_reset);
    }
    SOCDNX_IF_ERR_EXIT(WRITE_ECI_ECIC_BLOCKS_RESETr(unit, reg_val));

    disable_hard_reset = soc_property_suffix_num_get(unit, -1, spn_CUSTOM_FEATURE, "init_without_device_hard_reset", 0);
    LOG_VERBOSE(BSL_LS_SOC_INIT, (BSL_META_U(unit, "%s(): disable_hard_reset = %d\n"), FUNCTION_NAME(), disable_hard_reset));
    if (disable_hard_reset == 1) {
        if (is_in_reset == 1) {
            /* Fix for : IRE reset doesn't clear CMIC credits - when IRE going out of reset it always adds credits to the CMIC, so CMIC Credits needs to be reset when ever IRE is reset */
            SOCDNX_IF_ERR_EXIT(WRITE_CMIC_TXBUF_IPINTF_INTERFACE_CREDITSr(unit, 0x40));
            SOCDNX_IF_ERR_EXIT(WRITE_CMIC_TXBUF_IPINTF_INTERFACE_CREDITSr(unit, 0x0));

            /* Release all credits of CMIC packet I/F so that it can accept packets */
            SOCDNX_IF_ERR_EXIT(WRITE_CMIC_RXBUF_EPINTF_RELEASE_ALL_CREDITSr(unit, 0x0));
            SOCDNX_IF_ERR_EXIT(WRITE_CMIC_RXBUF_EPINTF_RELEASE_ALL_CREDITSr(unit, 0x1));
            SOCDNX_IF_ERR_EXIT(WRITE_CMIC_RXBUF_EPINTF_RELEASE_ALL_CREDITSr(unit, 0x0));
        }
    }

    if (is_in_reset == 0x0) {
        /* Verify blocks are OOR */
        SOCDNX_IF_ERR_EXIT(soc_dnx_reset_blocks_poll_init_finish(unit));
    }

exit:
    SOCDNX_FUNC_RETURN;
}

/* Configure CMIC. */
int soc_dnx_init_reset_cmic_regs(
    int unit)
{
    uint32 core_freq = 0x0;
    int schan_timeout = 0x0;
    int dividend, divisor;
    int mdio_int_freq, mdio_delay;

    SOCDNX_INIT_FUNC_DEFS;

    /*
     * Map the blocks to their Sbus rings.
	 * The block IDs are bigger than other devices (127).
     * Each digit represents 16 block IDs, because the most important for dnx devices are bits from [10:4].
     */
    SOCDNX_IF_ERR_EXIT(WRITE_CMIC_SBUS_RING_MAP_0_7r(unit, 0x77022222));
    SOCDNX_IF_ERR_EXIT(WRITE_CMIC_SBUS_RING_MAP_8_15r(unit, 0x06654333));


    /* Set SBUS timeout */
    SOCDNX_IF_ERR_EXIT(soc_arad_core_frequency_config_get(unit, SOC_JER_CORE_FREQ_KHZ_DEFAULT, &core_freq));
    SOCDNX_IF_ERR_EXIT(soc_arad_schan_timeout_config_get(unit, &schan_timeout));
    SOCDNX_IF_ERR_EXIT(soc_dcmn_cmic_sbus_timeout_set(unit, core_freq, schan_timeout));

    /* Mdio - internal*/

    /*Dividend values*/

    dividend = soc_property_get(unit, spn_RATE_INT_MDIO_DIVIDEND, -1); 
    if (dividend == -1) 
    {
        /*default value*/
        dividend =  SOC_DPP_IMP_DEFS_GET(unit, mdio_int_dividend_default);

    }

    divisor = soc_property_get(unit, spn_RATE_INT_MDIO_DIVISOR, -1); 
    if (divisor == -1) 
    {
        /*Calc default dividend and divisor*/
        mdio_int_freq = SOC_DPP_IMP_DEFS_GET(unit, mdio_int_freq_default);
        divisor = core_freq * dividend / (2* mdio_int_freq);

    }

    mdio_delay = SOC_DPP_IMP_DEFS_GET(unit, mdio_int_out_delay_default);

    SOCDNX_IF_ERR_EXIT(soc_dcmn_cmic_mdio_config(unit,dividend,divisor,mdio_delay));

    /* Clear SCHAN_ERR */
    SOCDNX_IF_ERR_EXIT(WRITE_CMIC_CMC0_SCHAN_ERRr(unit, 0));
    SOCDNX_IF_ERR_EXIT(WRITE_CMIC_CMC1_SCHAN_ERRr(unit, 0));
    SOCDNX_IF_ERR_EXIT(WRITE_CMIC_CMC2_SCHAN_ERRr(unit, 0));

    /* MDIO configuration */
    SOCDNX_IF_ERR_EXIT(soc_dcmn_cmic_mdio_set(unit));

exit:
    SOCDNX_FUNC_RETURN;
}

STATIC int soc_dnx_device_blocks_reset(int unit, int reset_action)
{
    SOCDNX_INIT_FUNC_DEFS;

    if ((reset_action == SOC_DPP_RESET_ACTION_IN_RESET) || (reset_action == SOC_DPP_RESET_ACTION_INOUT_RESET)) { 

        /* Sbus Reset*/
        SOCDNX_IF_ERR_EXIT(soc_dnx_reset_sbus_reset(unit, 1));
        
        /* Soft Reset*/
        SOCDNX_IF_ERR_EXIT(soc_dnx_reset_blocks_reset(unit, 1));
    }

    if ((reset_action == SOC_DPP_RESET_ACTION_OUT_RESET) || (reset_action == SOC_DPP_RESET_ACTION_INOUT_RESET)) {

        /* sbus reset */
        SOCDNX_IF_ERR_EXIT(soc_dnx_reset_sbus_reset(unit, 0));
        /* soft reset */
        SOCDNX_IF_ERR_EXIT(soc_dnx_reset_blocks_reset(unit, 0));
    }

exit:
    SOCDNX_FUNC_RETURN;
}

STATIC int soc_dnx_init_reg_access(
    int unit,
    int reset_action)
{
    SOCDNX_INIT_FUNC_DEFS;
	
    /*
     * Reset device.
     * Also enable device access, set default Iproc/CmicD configuration
     * No access allowed before this stage.
     */
    SOCDNX_IF_ERR_EXIT(soc_dnx_init_reset(unit, reset_action));
    /* Enable Access to device blocks */
    SOCDNX_IF_ERR_EXIT(soc_dnx_device_blocks_reset(unit, SOC_DPP_RESET_ACTION_INOUT_RESET));

#ifdef BCM_WARM_BOOT_SUPPORT
    soc_arad_init_empty_scache(unit);
#endif
#ifdef JER2_TO_DO_LIST
    /* Enable ports reg access */
    if ((SOC_CONTROL(unit)->soc_flags & SOC_F_INITED) == 0) {
        SOC_INFO(unit).fabric_logical_port_base = soc_property_get(unit, spn_FABRIC_LOGICAL_PORT_BASE, \
            SOC_DPP_FABRIC_LOGICAL_PORT_BASE_DEFAULT);
        SOCDNX_IF_ERR_EXIT(soc_jer_ports_config(unit));
    }
#endif
    SOC_CONTROL(unit)->soc_flags |= SOC_F_INITED;
exit:
    SOCDNX_FUNC_RETURN;
}

int
soc_dnx_device_reset(int unit, int mode, int action)
{
/*    uint32 enable;*/
    SOCDNX_INIT_FUNC_DEFS;

    if (!SOC_IS_DNX(unit)) {
        SOCDNX_EXIT_WITH_ERR(SOC_E_UNAVAIL, (_BSL_SOCDNX_MSG("Jericho 2 function. invalid Device")));
    }

    switch (mode) {
    case SOC_DPP_RESET_MODE_HARD_RESET:
        break;
    case SOC_DPP_RESET_MODE_BLOCKS_RESET:
        break;
    case SOC_DPP_RESET_MODE_BLOCKS_SOFT_RESET:
        break;
    case SOC_DPP_RESET_MODE_BLOCKS_SOFT_INGRESS_RESET:
        break;
    case SOC_DPP_RESET_MODE_BLOCKS_SOFT_EGRESS_RESET:
        break;
    case SOC_DPP_RESET_MODE_BLOCKS_AND_FABRIC_SOFT_RESET:
        break;
    case SOC_DPP_RESET_MODE_BLOCKS_AND_FABRIC_SOFT_INGRESS_RESET:
        break;
    case SOC_DPP_RESET_MODE_BLOCKS_AND_FABRIC_SOFT_EGRESS_RESET:
        break;
    case SOC_DPP_RESET_MODE_INIT_RESET:
        break;
    case SOC_DNX_RESET_MODE_REG_ACCESS:
        SOCDNX_IF_ERR_RETURN(soc_dnx_init_reg_access(unit, action));
        break;	
	case SOC_DPP_RESET_MODE_ENABLE_TRAFFIC:
        break;
    case SOC_DPP_RESET_MODE_BLOCKS_SOFT_RESET_DIRECT:
        break;
    default:
        SOCDNX_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_SOCDNX_MSG("Unknown/Unsupported Reset Mode")));
    }

exit:
    SOCDNX_FUNC_RETURN;
}

int soc_dnx_init_reset(
    int unit,
    int reset_action)
{
    int disable_hard_reset = 0x0;

    SOCDNX_INIT_FUNC_DEFS;

    /* Configure PAXB, enabling the access of CMIC */
    SOCDNX_IF_ERR_EXIT(soc_dcmn_iproc_config_paxb(unit));

    /* Arad CPS Reset */
    disable_hard_reset = soc_property_suffix_num_get(unit, -1, spn_CUSTOM_FEATURE, "init_without_device_hard_reset", 0);
    
	if (disable_hard_reset == 0) {
        SOCDNX_IF_ERR_EXIT(soc_dcmn_cmic_device_hard_reset(unit, reset_action));
    }

    SOCDNX_IF_ERR_EXIT(soc_dcmn_iproc_config_paxb(unit));

    /* Config Endianess */
    soc_endian_config(unit);
    soc_pci_ep_config(unit, 0);

    /* Config Default/Basic cmic registers */
    if (soc_feature(unit, soc_feature_cmicm)) {
        SOCDNX_IF_ERR_EXIT(soc_dnx_init_reset_cmic_regs(unit));
    }
exit:
    SOCDNX_FUNC_RETURN;
}

/* } */
#endif
#ifdef BCM_DNX_SUPPORT
/* { */
#ifdef BSL_LOG_MODULE
#error "BSL_LOG_MODULE redefined"
#endif
#define BSL_LOG_MODULE BSL_LS_SOCDNX_INIT

#include <shared/shrextend/shrextend_debug.h>

int
soc_dnx_device_reset(int unit, int mode, int action)
{
  SHR_FUNC_INIT_VARS(unit);
  SHR_IF_ERR_EXIT(_SHR_E_INTERNAL);
exit:
  SHR_FUNC_EXIT;

}/* } */
#endif

