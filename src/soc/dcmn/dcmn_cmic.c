/*
 * $Id: dcmn_cmic.c,v 1.0 Broadcom SDK $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * SOC DCMN IPROC
 */

#ifdef _ERR_MSG_MODULE_NAME
  #error "_ERR_MSG_MODULE_NAME redefined"
#endif /* _ERR_MSG_MODULE_NAME */

#define _ERR_MSG_MODULE_NAME BSL_SOC_INIT

#include <shared/bsl.h>
#include <soc/defs.h>
#include <soc/drv.h>
#include <soc/dcmn/dcmn_cmic.h>
#ifdef BCM_DPP_SUPPORT
#include <soc/dpp/port_sw_db.h>
#endif /* BCM_DPP_SUPPORT */
#include <soc/dcmn/error.h>

int soc_dcmn_cmic_device_hard_reset(int unit, int reset_action)
{
    uint32 
        reg32_val = 0;
    soc_timeout_t 
        to;
    uint32 usleep = 1000;

    SOCDNX_INIT_FUNC_DEFS;

    if (!SOC_REG_IS_VALID(unit, CMIC_CPS_RESETr)) {
        SOC_EXIT;
    }

    if ((reset_action == SOC_DCMN_RESET_ACTION_IN_RESET) || (reset_action == SOC_DCMN_RESET_ACTION_INOUT_RESET)) {
        SOCDNX_IF_ERR_EXIT(WRITE_CMIC_CPS_RESETr(unit, 0x1)); 

#ifdef PLISIM
        if (SAL_BOOT_PLISIM) {
            SOC_EXIT;
        }
#endif
#if defined(BCM_DPP_SUPPORT)
        
        if (SOC_IS_DPP(unit) && SOC_DPP_CONFIG(unit)->emulation_system != 0 ) {
            usleep*=1000;
        }
#endif /* BCM_DPP_SUPPORT */
#if defined(BCM_DNX_SUPPORT)
        
        if (SOC_IS_DNX(unit) ) {
            usleep*=1000;
        }
#endif /* BCM_DPP_SUPPORT */
        sal_usleep(usleep); /* delay of 1ms that is much more than the required time for the reset to finish */
        /* Wait for the reset to finish, although due to the delay above it should have already ended */
        soc_timeout_init(&to, 100000, 100);
        for (;;) {
            SOCDNX_IF_ERR_EXIT(READ_CMIC_CPS_RESETr(unit, &reg32_val));
            if (reg32_val == 0x0) {
                break;
            }
            if (soc_timeout_check(&to)) {
                SOCDNX_EXIT_WITH_ERR(_SHR_E_INIT, (_BSL_SOC_MSG("Error: CPS reset field not asserted correctly.")));
                break;
            }
        }   
    }

exit:
    SOCDNX_FUNC_RETURN;
}

int
soc_dcmn_cmic_sbus_timeout_set(int unit, uint32 core_freq_khz, int schan_timeout)
{
    
    uint32 freq_mhz = core_freq_khz / 1000;
    uint32 ticks,
           max_uint = 0xFFFFFFFF,
           max_ticks= 0x3FFFFF;
    
    SOCDNX_INIT_FUNC_DEFS;

    /* configure ticks to be a HW timeout that is 75% of SW timeout.
     * units:
     *  schanTimeout is in microsecond
     *  frequency is recieved in KHz, and modified to be in MHz.
     *  after the modification: ticks = frequency * Timeout 
     */

    if ((max_uint / freq_mhz) > schan_timeout) { /* make sure ticks can be represented in 32 bits*/
        ticks = freq_mhz * schan_timeout;
        ticks = ((ticks / 100) * 75); /* make sure hardware timeout is smaller than software*/
    } else {
        ticks = max_ticks;
    }

    SOCDNX_IF_ERR_EXIT(WRITE_CMIC_SBUS_TIMEOUTr(unit, ticks));
 
exit:
    SOCDNX_FUNC_RETURN;
}

int soc_dcmn_cmic_pcie_userif_purge_ctrl_init(int unit)
{
    uint32 reg;

    SOCDNX_INIT_FUNC_DEFS;
    
    if (soc_feature(unit, soc_feature_cmicm) && 
        soc_feature(unit, soc_feature_sbusdma)) {
        /* Enable PCIe purge on error */
        SOCDNX_IF_ERR_EXIT(READ_CMIC_PCIE_USERIF_PURGE_CONTROLr(unit, &reg));
        soc_reg_field_set(unit, CMIC_PCIE_USERIF_PURGE_CONTROLr, &reg,
                          ENABLE_PURGE_IF_USERIF_TIMESOUTf, 1);
        soc_reg_field_set(unit, CMIC_PCIE_USERIF_PURGE_CONTROLr, &reg,
                          ENABLE_PIO_PURGE_IF_USERIF_RESETf, 1);
        SOCDNX_IF_ERR_EXIT(WRITE_CMIC_PCIE_USERIF_PURGE_CONTROLr(unit, reg));
    }

exit:
    SOCDNX_FUNC_RETURN;
}

int soc_dcmn_cmic_mdio_config(int unit, int dividend, int divisor, int delay)
{
    uint32 rval;

    SOCDNX_INIT_FUNC_DEFS;
    
    SOCDNX_IF_ERR_EXIT(READ_CMIC_RATE_ADJUST_INT_MDIOr(unit, &rval));
    soc_reg_field_set(unit, CMIC_RATE_ADJUST_INT_MDIOr, &rval, DIVISORf, divisor);
    soc_reg_field_set(unit, CMIC_RATE_ADJUST_INT_MDIOr, &rval, DIVIDENDf, dividend);
    SOCDNX_IF_ERR_EXIT(WRITE_CMIC_RATE_ADJUST_INT_MDIOr(unit, rval));

    SOCDNX_IF_ERR_EXIT(READ_CMIC_MIIM_CONFIGr(unit, &rval));
    soc_reg_field_set(unit, CMIC_MIIM_CONFIGr, &rval, MDIO_OUT_DELAYf, delay);
    SOCDNX_IF_ERR_EXIT(WRITE_CMIC_MIIM_CONFIGr(unit, rval));

exit:
    SOCDNX_FUNC_RETURN;
}


int soc_dcmn_cmic_mdio_set(int unit)
{
    uint32 reg;
    int divisor, dividend, mdio_delay;

    SOCDNX_INIT_FUNC_DEFS;
    
    /* Internal */
    reg = 0;
    dividend = soc_property_get(unit, spn_RATE_INT_MDIO_DIVIDEND, 0x1);
    divisor = soc_property_get(unit, spn_RATE_INT_MDIO_DIVISOR, 0x96);    
    soc_reg_field_set(unit, CMIC_RATE_ADJUST_INT_MDIOr, &reg, DIVISORf, divisor);
    soc_reg_field_set(unit, CMIC_RATE_ADJUST_INT_MDIOr, &reg, DIVIDENDf, dividend);
    SOCDNX_IF_ERR_EXIT(WRITE_CMIC_RATE_ADJUST_INT_MDIOr(unit, reg));

    /* External */
    reg = 0;
    dividend = soc_property_get(unit, spn_RATE_EXT_MDIO_DIVIDEND, 0x1);
    divisor = soc_property_get(unit, spn_RATE_EXT_MDIO_DIVISOR, 0xc);
    soc_reg_field_set(unit, CMIC_RATE_ADJUST_EXT_MDIOr, &reg, DIVISORf, divisor);
    soc_reg_field_set(unit, CMIC_RATE_ADJUST_EXT_MDIOr, &reg, DIVIDENDf, dividend);
    SOCDNX_IF_ERR_EXIT(WRITE_CMIC_RATE_ADJUST_EXT_MDIOr(unit, reg));

    /* Delay */
    reg = 0;
    mdio_delay = 0xf;
    soc_reg_field_set(unit, CMIC_MIIM_CONFIGr, &reg, MDIO_OUT_DELAYf, mdio_delay);
    SOCDNX_IF_ERR_EXIT(WRITE_CMIC_MIIM_CONFIGr(unit, reg));

    /* Set MDIO Master */
    SOCDNX_IF_ERR_EXIT(READ_CMIC_MIIM_SCAN_CTRLr(unit, &reg));
    soc_reg_field_set(unit, CMIC_MIIM_SCAN_CTRLr, &reg, OVER_RIDE_EXT_MDIO_MSTR_CNTRLf, 0x1);
    SOCDNX_IF_ERR_EXIT(WRITE_CMIC_MIIM_SCAN_CTRLr(unit, reg));

exit:
    SOCDNX_FUNC_RETURN;
}



