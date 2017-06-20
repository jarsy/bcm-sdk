/*
 * $Id: ramon_fe1600_link.c,v 1.7 Broadcom SDK $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * SOC RAMON_FE1600 STAT
 */
 

#ifdef _ERR_MSG_MODULE_NAME
  #error "_ERR_MSG_MODULE_NAME redefined"
#endif

#define _ERR_MSG_MODULE_NAME BSL_SOC_PORT
#include <shared/bsl.h>
#include <soc/dnxc/legacy/error.h>
#include <soc/defs.h>
#include <soc/error.h>
#include <soc/mcm/allenum.h>
#include <soc/mcm/memregs.h>

#include <soc/dnxf/cmn/dnxf_drv.h>
#include <soc/dnxf/cmn/dnxf_defs.h>
#include <shared/bitop.h>

#if defined(BCM_88790_A0)

#include <soc/dnxf/fe1600/fe1600_defs.h>
#include <soc/dnxf/fe1600/fe1600_link.h>

/*
 * Function:
 *     _soc_ramon_fe1600_linkctrl_linkscan_hw_init
 * Purpose:
 *     Initialize hardware linkscan.
 * Parameters:
 *     unit  - Device number
 * Returns:
 *     SOC_E_XXX
 */
STATIC int
_soc_ramon_fe1600_linkctrl_linkscan_hw_init(int unit)
{
    uint32 reg_val;

    DNXC_INIT_FUNC_DEFS;
	SOC_RAMON_FE1600_ONLY(unit);

    /*assume already init for easy reload and warmboot*/
    if(SOC_IS_RELOADING(unit) || SOC_WARM_BOOT(unit)) {
        SOC_EXIT;
    }

    /*disable hw linkscan - will enable when config hw linkscan*/
    DNXC_IF_ERR_EXIT(soc_pci_write(unit, CMIC_SCHAN_CTRL, SC_MIIM_LINK_SCAN_EN_CLR));

    /*set all ports to be internal PHYs and not external*/
    DNXC_IF_ERR_EXIT(WRITE_CMIC_MIIM_INT_SEL_MAPr(unit, 0xffffffff));
    DNXC_IF_ERR_EXIT(WRITE_CMIC_MIIM_INT_SEL_MAP_HIr(unit, 0xffffffff));
    DNXC_IF_ERR_EXIT(WRITE_CMIC_MIIM_INT_SEL_MAP_HI_2r(unit, 0xffffffff));
    DNXC_IF_ERR_EXIT(WRITE_CMIC_MIIM_INT_SEL_MAP_HI_3r(unit, 0xffffffff));

    DNXC_IF_ERR_EXIT(READ_CMIC_CONFIGr(unit, &reg_val));
    soc_reg_field_set(unit, CMIC_CONFIGr, &reg_val, OVER_RIDE_EXT_MDIO_MSTR_CNTRLf,1);
    soc_reg_field_set(unit, CMIC_CONFIGr, &reg_val, STOP_LS_ON_FIRST_CHANGEf,0);
    soc_reg_field_set(unit, CMIC_CONFIGr, &reg_val, STOP_LS_ON_CHANGEf,0);
    DNXC_IF_ERR_EXIT(WRITE_CMIC_CONFIGr(unit, reg_val));

  
    /*select ports to be scanned*/
    DNXC_IF_ERR_EXIT(WRITE_CMIC_SCAN_PORTSr(unit, 0xffffffff));
    DNXC_IF_ERR_EXIT(WRITE_CMIC_SCAN_PORTS_HIr(unit, 0xffffffff));
    DNXC_IF_ERR_EXIT(WRITE_CMIC_SCAN_PORTS_HI_2r(unit, 0xffffffff));
    DNXC_IF_ERR_EXIT(WRITE_CMIC_SCAN_PORTS_HI_3r(unit, 0xffffffff));

exit:
    DNXC_FUNC_RETURN;
}

/*
 * Function:
 *      _soc__ramon_fe1600_linkscan_ports_write
 * Purpose:
 *      Writes the CMIC_SCAN_PORTS register(s) of the device with the
 *      provided HW linkscan port configuration.
 * Parameters:
 *      unit - StrataSwitch unit #.
 *      hw_mii_pbm - Scan ports.
 * Returns:
 *      Nothing
 * Notes:
 *      Assumes interrupt suspension handled in the calling function
 */

STATIC int
_soc_ramon_fe1600_linkscan_ports_write(int unit, pbmp_t hw_mii_pbm)
{
    uint32      link_pbmp;
    DNXC_INIT_FUNC_DEFS;
	SOC_RAMON_FE1600_ONLY(unit);

    link_pbmp = SOC_PBMP_WORD_GET(hw_mii_pbm, 0); 
    DNXC_IF_ERR_EXIT(WRITE_CMIC_SCAN_PORTSr(unit, link_pbmp));

    link_pbmp = SOC_PBMP_WORD_GET(hw_mii_pbm, 1); 
    DNXC_IF_ERR_EXIT(WRITE_CMIC_SCAN_PORTS_HIr(unit, link_pbmp));
        
    link_pbmp = SOC_PBMP_WORD_GET(hw_mii_pbm, 2); 
    DNXC_IF_ERR_EXIT(WRITE_CMIC_SCAN_PORTS_HI_2r(unit, link_pbmp));
        
    link_pbmp = SOC_PBMP_WORD_GET(hw_mii_pbm, 3); 
    DNXC_IF_ERR_EXIT(WRITE_CMIC_SCAN_PORTS_HI_3r(unit, link_pbmp));
        
exit:
    DNXC_FUNC_RETURN;  
}

/*
 * Function:    
 *     _soc_ramon_fe1600_linkscan_pause
 * Purpose:
 *     Pauses link scanning
 * Parameters:  
 *      unit - Device unit number
 * Returns:
 *     SOC_E_XXX
 */
STATIC int
_soc_ramon_fe1600_linkscan_pause(int unit)
{
    int stall_count;
    pbmp_t pbm;
    uint32 schan_ctrl;
    DNXC_INIT_FUNC_DEFS;
	SOC_RAMON_FE1600_ONLY(unit);

    /* First clear HW linkscan ports */
    SOC_PBMP_CLEAR(pbm);
    DNXC_IF_ERR_EXIT(_soc_ramon_fe1600_linkscan_ports_write(unit, pbm));
        
    /* Turn off HW linkscan */
    soc_pci_write(unit, CMIC_SCHAN_CTRL, SC_MIIM_LINK_SCAN_EN_CLR);

    if (soc_feature(unit, soc_feature_linkscan_pause_timeout)) {
        soc_timeout_t to;
        /* Wait for Linkscan stopped signal */
        soc_timeout_init(&to, 1000000 /*1 sec*/, 100);  
        while (soc_pci_read(unit, CMIC_SCHAN_CTRL) & SC_MIIM_SCAN_BUSY_TST) {
            if (soc_timeout_check(&to)) {
                DNXC_EXIT_WITH_ERR(SOC_E_TIMEOUT, (_BSL_DNXC_MSG("pausing hw linkscan failed")));
            }
        }
    } else {
        /* Wait for Linkscan stopped signal */
        while (soc_pci_read(unit, CMIC_SCHAN_CTRL) & SC_MIIM_SCAN_BUSY_TST) {
           /* Nothing */
        }
    }
            
    COMPILER_REFERENCE(schan_ctrl);
    /* Wait > 1us for last HW linkscan operation to complete. */
    for (stall_count = 0; stall_count < 4; stall_count++) {
        /* We're using this PCI operation to pass some time
         * since we can't use sal_usleep safely with the
         * interrupts suspended.  We only record the read value
         * to prevent any complaint about an uninspected return
         * value.
         */
        schan_ctrl = soc_pci_read(unit, CMIC_SCHAN_CTRL);
    }
   
exit:   
    DNXC_FUNC_RETURN;
}

/*
 * Function:    
 *     _soc_ramon_fe1600_linkscan_continue
 * Purpose:
 *     Continue link scanning
 * Parameters:  
 *      unit - Device unit number
 * Returns:
 *     SOC_E_XXX
 */
STATIC int
_soc_ramon_fe1600_linkscan_continue(int unit)
{
    soc_control_t *soc = SOC_CONTROL(unit);
    DNXC_INIT_FUNC_DEFS;
	SOC_RAMON_FE1600_ONLY(unit);

    /*
    * NOTE: whenever hardware linkscan is running, the PHY_REG_ADDR
    * field of the MIIM_PARAM register must be set to 1 (PHY Link
    * Status register address).
    */
    if (soc_feature(unit, soc_feature_phy_cl45))  {
        /*
        ** Clause 22 Register 0x01 (MII_STAT) for FE/GE.
        ** Clause 45 Register 0x01 (MII_STAT) Devad = 0x1 (PMA_PMD) 
        ** for XE.
        */
        uint32 phy_miim_addr = 0;
        soc_reg_field_set(unit, CMIC_MIIM_ADDRESSr, &phy_miim_addr, CLAUSE_45_DTYPEf, 0x01);
        soc_reg_field_set(unit, CMIC_MIIM_ADDRESSr, &phy_miim_addr, CLAUSE_45_REGADRf, 0x01);
        soc_reg_field_set(unit, CMIC_MIIM_ADDRESSr, &phy_miim_addr, CLAUSE_22_REGADRf, 0x01);
        DNXC_IF_ERR_EXIT(WRITE_CMIC_MIIM_ADDRESSr(unit, phy_miim_addr));
    } else {
        soc_pci_write(unit, CMIC_MIIM_PARAM, (uint32) 0x01 << 24);
    }
    DNXC_IF_ERR_EXIT(_soc_ramon_fe1600_linkscan_ports_write(unit, soc->hw_linkscan_pbmp));
    soc_pci_write(unit, CMIC_SCHAN_CTRL, SC_MIIM_LINK_SCAN_EN_SET);
    
            
exit:  
    DNXC_FUNC_RETURN;
}

STATIC int
_soc_ramon_fe1600_linkctrl_update(int unit)
{
    DNXC_INIT_FUNC_DEFS;
	SOC_RAMON_FE1600_ONLY(unit);

    DNXC_FUNC_RETURN;
}

/*
 * Function:
 *     _soc_ramon_fe1600_linkctrl_linkscan_config
 * Purpose:
 *     Set ports to hardware linkscan.
 * Parameters:
 *     unit          - Device number
 *     hw_mii_pbm    - Port bit map of ports to scan with MIIM registers
 *     hw_direct_pbm - Port bit map of ports to scan using NON MII
 * Returns:
 *     SOC_E_XXX
 */
STATIC int
_soc_ramon_fe1600_linkctrl_linkscan_config(int unit, pbmp_t hw_mii_pbm, pbmp_t hw_direct_pbm)
{
    soc_control_t *soc = SOC_CONTROL(unit);
    pbmp_t pbm;
    DNXC_INIT_FUNC_DEFS;   
	SOC_RAMON_FE1600_ONLY(unit);

    if (SOC_PBMP_NOT_NULL(hw_mii_pbm) || SOC_PBMP_NOT_NULL(hw_direct_pbm)) {

        /*unmask linkscan status*/
        soc_intr_enable(unit, IRQ_LINK_STAT_MOD);
        
     } else {

        /*mask linkscan status*/
        soc_intr_disable(unit, IRQ_LINK_STAT_MOD);
     }   

    /* Check if disabling port scanning */
    SOC_PBMP_ASSIGN(pbm, hw_mii_pbm);
    SOC_PBMP_OR(pbm, hw_direct_pbm);
    if (SOC_PBMP_NOT_NULL(pbm)) {
        /*
         * NOTE: we are no longer using CC_LINK_STAT_EN since it is
         * unavailable on 5695 and 5665.  EPC_LINK will be updated by
         * software anyway, it will just take a few extra milliseconds.
         */
        soc->soc_flags |= SOC_F_LSE;
    } else {
        soc->soc_flags &= ~SOC_F_LSE;
    }


    /* The write of the HW linkscan ports is moved to the linkscan
     * continue below.  Note that though the continue function
     * will not write to the CMIC scan ports register if linkscan
     * was disabled above, that is only the case when the port bitmap
     * is empty.  Since linkscan pause clears the bitmap, this is the
     * desired result.
     */
    SOC_PBMP_ASSIGN(soc->hw_linkscan_pbmp, hw_mii_pbm);

    DNXC_FUNC_RETURN;
}

STATIC int
_soc_ramon_fe1600_linkctrl_hw_link_get(int unit, soc_pbmp_t *hw_link)
{
    uint32 reg_val;
    DNXC_INIT_FUNC_DEFS;
	SOC_RAMON_FE1600_ONLY(unit);

    DNXC_NULL_CHECK(hw_link);

    SOC_PBMP_CLEAR(*hw_link);

    DNXC_IF_ERR_EXIT(READ_CMIC_LINK_STATr(unit, &reg_val));
    SOC_PBMP_WORD_SET(*hw_link,0,reg_val);

    DNXC_IF_ERR_EXIT(READ_CMIC_LINK_STAT_HIr(unit, &reg_val));
    SOC_PBMP_WORD_SET(*hw_link,1,reg_val);
    
     DNXC_IF_ERR_EXIT(READ_CMIC_LINK_STAT_HI_2r(unit, &reg_val));
    SOC_PBMP_WORD_SET(*hw_link,2,reg_val);
    
    DNXC_IF_ERR_EXIT(READ_CMIC_LINK_STAT_HI_3r(unit, &reg_val));
    SOC_PBMP_WORD_SET(*hw_link,3,reg_val);  

exit:
    DNXC_FUNC_RETURN;
}

/*
 * Link Control Driver - DNXF
 */
soc_linkctrl_driver_t  soc_linkctrl_driver_ramon_fe1600 = {
    NULL,                                   /* port mapping */
    _soc_ramon_fe1600_linkctrl_linkscan_hw_init,  /* ld_linkscan_hw_init */
    _soc_ramon_fe1600_linkctrl_linkscan_config,   /* ld_linkscan_config */
    _soc_ramon_fe1600_linkscan_pause,             /* ld_linkscan_pause */
    _soc_ramon_fe1600_linkscan_continue,          /* ld_linkscan_continue */
    _soc_ramon_fe1600_linkctrl_update,            /* ld_update */
    _soc_ramon_fe1600_linkctrl_hw_link_get        /* ld_hw_link_get */
};

int
soc_ramon_fe1600_linkctrl_init(int unit) 
{
    DNXC_INIT_FUNC_DEFS;
	SOC_RAMON_FE1600_ONLY(unit);

    DNXC_IF_ERR_EXIT(soc_linkctrl_init(unit, &soc_linkctrl_driver_ramon_fe1600));  

exit:
    DNXC_FUNC_RETURN;
}

#endif /*defined(BCM_88790_A0)*/

#undef _ERR_MSG_MODULE_NAME

