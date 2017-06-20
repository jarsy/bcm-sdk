/* 
 * $Id: arad_link.c,v 1.3 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:        arad_link.c
 * Purpose:     Hardware Linkscan module
 *
 * These routines will be called by the linkscan module,
 * so they need to be defined and return SOC_E_NONE.
 */
#ifdef _ERR_MSG_MODULE_NAME
  #error "_ERR_MSG_MODULE_NAME redefined"
#endif

#define _ERR_MSG_MODULE_NAME BSL_SOC_PORT

/*************
 * INCLUDES  *
 *************/
/* { */
#include <shared/bsl.h>
#include <soc/dcmn/error.h>
#include <soc/linkctrl.h>
#include <soc/drv.h>
#include <soc/error.h>
#ifdef BCM_CMICM_SUPPORT
#include <soc/cmicm.h>
#endif
#include <soc/mcm/memregs.h>
#include <soc/mcm/cmicm.h>


/*
 * Function:
 *     _soc_arad_linkctrl_linkscan_hw_init
 * Purpose:
 *     Initialize hardware linkscan.
 * Parameters:
 *     unit  - Device number
 * Returns:
 *     SOC_E_NONE
 */
STATIC int
_soc_arad_linkctrl_linkscan_hw_init(int unit)
{

    uint32 reg32_val;
    SOCDNX_INIT_FUNC_DEFS;

    /*assume already init for warmboot*/
    if(SOC_WARM_BOOT(unit)) {
        SOC_EXIT;
    }

    SOCDNX_IF_ERR_EXIT(WRITE_CMIC_MIIM_INT_SEL_MAP_0r(unit,  0xffffffff));
    SOCDNX_IF_ERR_EXIT(WRITE_CMIC_MIIM_INT_SEL_MAP_1r(unit,  0xffffffff));
    SOCDNX_IF_ERR_EXIT(WRITE_CMIC_MIIM_INT_SEL_MAP_2r(unit,  0xffffffff));
    SOCDNX_IF_ERR_EXIT(WRITE_CMIC_MIIM_INT_SEL_MAP_3r(unit,  0xffffffff));

    
    SOCDNX_IF_ERR_EXIT(READ_CMIC_MIIM_SCAN_CTRLr(unit, &reg32_val));
    /*disable linkscan - enable when hw linkscan will be cfg*/
    soc_reg_field_set(unit, CMIC_MIIM_SCAN_CTRLr, &reg32_val, MIIM_LINK_SCAN_ENf, 0);
    soc_reg_field_set(unit, CMIC_MIIM_SCAN_CTRLr, &reg32_val, STOP_LS_ON_FIRST_CHANGEf, 0);
    soc_reg_field_set(unit, CMIC_MIIM_SCAN_CTRLr, &reg32_val, STOP_LS_ON_CHANGEf, 0);
    SOCDNX_IF_ERR_EXIT(WRITE_CMIC_MIIM_SCAN_CTRLr(unit, reg32_val));

    SOCDNX_IF_ERR_EXIT(WRITE_CMIC_MIIM_SCAN_PORTS_0r(unit, 0x0));
    SOCDNX_IF_ERR_EXIT(WRITE_CMIC_MIIM_SCAN_PORTS_1r(unit, 0x0));
    SOCDNX_IF_ERR_EXIT(WRITE_CMIC_MIIM_SCAN_PORTS_2r(unit, 0x0));
    SOCDNX_IF_ERR_EXIT(WRITE_CMIC_MIIM_SCAN_PORTS_3r(unit, 0x0));

exit:
    SOCDNX_FUNC_RETURN;
}

STATIC int
arad_ports_phy_to_cmic_bitmap_map(
   uint32      unit,
   soc_pbmp_t  phy_bitmap,
   soc_pbmp_t  *cmic_bitmap)
{
    uint32 word_cmic, word_phy;
    SOCDNX_INIT_FUNC_DEFS;

    /*Mapping is as follow 
     * 0: (MSB) 43-42-41-40-27-26-25-24-23-22-21-20-19-18-17-16-11-10-9-8-7-6-5-4-3-2-1-15-14-13-12 (LSB) 
     * 1: (MSB) 75-...-44 (LSB) 
     * 2: cleared 
     * 3: cleared 
     */
    SOC_PBMP_CLEAR(*cmic_bitmap);
    
    /*first word cmic bitmap*/
    word_cmic = 0;
    word_phy = SOC_PBMP_WORD_GET(phy_bitmap, 0);
    SHR_BITCOPY_RANGE(&word_cmic, 0, &word_phy, 12, 4);
    SHR_BITCOPY_RANGE(&word_cmic, 4, &word_phy, 0, 12);
    SHR_BITCOPY_RANGE(&word_cmic, 16, &word_phy, 16, 12);
    word_phy = SOC_PBMP_WORD_GET(phy_bitmap, 1);
    SHR_BITCOPY_RANGE(&word_cmic, 28, &word_phy, 7 /*bit 39 == 40*/, 4);
    SOC_PBMP_WORD_SET(*cmic_bitmap, 0, word_cmic);
    /*second word cmic bitmap*/
    word_cmic = 0;
    word_phy = SOC_PBMP_WORD_GET(phy_bitmap, 1);
    SHR_BITCOPY_RANGE(&word_cmic, 0, &word_phy, 11 /*bit 43 == 44*/, 21);
    word_phy = SOC_PBMP_WORD_GET(phy_bitmap, 2);
    SHR_BITCOPY_RANGE(&word_cmic, 21, &word_phy, 0, 11);
    SOC_PBMP_WORD_SET(*cmic_bitmap, 1, word_cmic);

    SOCDNX_FUNC_RETURN;
}

STATIC int
_soc_arad_linkscan_ports_write(int unit, pbmp_t hw_mii_pbm)
{
    uint32      link_pbmp;
    soc_pbmp_t  tmp_pbmp, arad_pbmp;
    soc_port_t  phy_port, port;
    SOCDNX_INIT_FUNC_DEFS;

    SOC_PBMP_CLEAR(tmp_pbmp);
    PBMP_ITER(hw_mii_pbm, port) {
        phy_port = SOC_INFO(unit).port_l2p_mapping[port];
        if (phy_port == 0) {
            continue;
        }
        SOC_PBMP_PORT_ADD(tmp_pbmp, phy_port - 1);
    }

    SOCDNX_IF_ERR_EXIT(arad_ports_phy_to_cmic_bitmap_map(unit, tmp_pbmp, &arad_pbmp));
    SOC_PBMP_ASSIGN(tmp_pbmp, arad_pbmp);

    link_pbmp = SOC_PBMP_WORD_GET(tmp_pbmp, 0); 
    SOCDNX_IF_ERR_EXIT(WRITE_CMIC_MIIM_SCAN_PORTS_0r(unit, link_pbmp));

    link_pbmp = SOC_PBMP_WORD_GET(tmp_pbmp, 1); 
    SOCDNX_IF_ERR_EXIT(WRITE_CMIC_MIIM_SCAN_PORTS_1r(unit, link_pbmp));

    link_pbmp = SOC_PBMP_WORD_GET(tmp_pbmp, 2); 
    SOCDNX_IF_ERR_EXIT(WRITE_CMIC_MIIM_SCAN_PORTS_2r(unit, link_pbmp));

    link_pbmp = SOC_PBMP_WORD_GET(tmp_pbmp, 3); 
    SOCDNX_IF_ERR_EXIT(WRITE_CMIC_MIIM_SCAN_PORTS_3r(unit, link_pbmp));
  
exit:
    SOCDNX_FUNC_RETURN;  
}

/*
 * Function:
 *     _soc_arad_linkctrl_linkscan_pause
 * Purpose:
 *     Pause link scanning, without disabling it.
 *     This call is used to pause scanning temporarily.
 * Parameters:
 *     unit  - Device number
 * Returns:
 *     SOC_E_NONE
 */
STATIC int
_soc_arad_linkctrl_linkscan_pause(int unit)
{
    int stall_count;
    pbmp_t pbm;
    uint32 schan_ctrl;
    SOCDNX_INIT_FUNC_DEFS;

    /* First clear HW linkscan ports */
    SOC_PBMP_CLEAR(pbm);
    SOCDNX_IF_ERR_EXIT(_soc_arad_linkscan_ports_write(unit, pbm));

    /* Turn off HW linkscan */
    SOCDNX_IF_ERR_EXIT(READ_CMIC_MIIM_SCAN_CTRLr(unit,&schan_ctrl));
    soc_reg_field_set(unit, CMIC_MIIM_SCAN_CTRLr, &schan_ctrl, MIIM_LINK_SCAN_ENf, 0);
    SOCDNX_IF_ERR_EXIT(WRITE_CMIC_MIIM_SCAN_CTRLr(unit,schan_ctrl));
    if (soc_feature(unit, soc_feature_linkscan_pause_timeout)) {
         soc_timeout_t to;
        /* Wait for Linkscan stopped signal */
        soc_timeout_init(&to, 1000000 /*1 sec*/, 100);  
        while (soc_pci_read(unit, CMIC_MIIM_SCAN_STATUS_OFFSET) & CMIC_MIIM_SCAN_BUSY) {
            if (soc_timeout_check(&to)) {
               SOCDNX_EXIT_WITH_ERR(SOC_E_TIMEOUT, (_BSL_SOCDNX_MSG("pausing hw linkscan failed")));
            }
        }
    } else {
        /* Wait for Linkscan stopped signal */
        while (soc_pci_read(unit, CMIC_MIIM_SCAN_STATUS_OFFSET) & CMIC_MIIM_SCAN_BUSY) {
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
        schan_ctrl = soc_pci_read(unit, CMIC_MIIM_SCAN_STATUS_OFFSET);
    }
   
exit:
    SOCDNX_FUNC_RETURN;
}

/*
 * Function:
 *     _soc_arad_linkctrl_linkscan_continue
 * Purpose:
 *     Continue link scanning after it has been paused.
 * Parameters:
 *     unit  - Device number
 * Returns:
 *     SOC_E_NONE
 */
STATIC int
_soc_arad_linkctrl_linkscan_continue(int unit)
{

    soc_control_t *soc = SOC_CONTROL(unit);
    int cmc = SOC_PCI_CMC(unit);
    uint32 schan_ctrl;
    SOCDNX_INIT_FUNC_DEFS;

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
        soc_reg_field_set(unit, CMIC_CMC0_MIIM_ADDRESSr, &phy_miim_addr, CLAUSE_45_DTYPEf, 0x01);
        soc_reg_field_set(unit, CMIC_CMC0_MIIM_ADDRESSr, &phy_miim_addr, CLAUSE_45_REGADRf, 0x01);
        soc_reg_field_set(unit, CMIC_CMC0_MIIM_ADDRESSr, &phy_miim_addr, CLAUSE_22_REGADRf, 0x01);
        soc_pci_write(unit, CMIC_CMCx_MIIM_ADDRESS_OFFSET(cmc), phy_miim_addr);
    } else {
        soc_pci_write(unit, CMIC_CMCx_MIIM_PARAM_OFFSET(cmc), (uint32) 0x01 << 24);
    }

    SOCDNX_IF_ERR_EXIT(_soc_arad_linkscan_ports_write(unit, soc->hw_linkscan_pbmp));
    SOCDNX_IF_ERR_EXIT(READ_CMIC_MIIM_SCAN_CTRLr(unit,&schan_ctrl));
    soc_reg_field_set(unit, CMIC_MIIM_SCAN_CTRLr, &schan_ctrl, MIIM_LINK_SCAN_ENf, 1);
    SOCDNX_IF_ERR_EXIT(WRITE_CMIC_MIIM_SCAN_CTRLr(unit,schan_ctrl));
          
exit:
    SOCDNX_FUNC_RETURN;
}

/*
 * Function:
 *     _soc_arad_linkctrl_linkscan_config
 * Purpose:
 *     Set ports to hardware linkscan.
 * Parameters:
 *     unit          - Device number
 *     hw_mii_pbm    - Port bit map of ports to scan with MIIM registers
 *     hw_direct_pbm - Port bit map of ports to scan using NON MII
 * Returns:
 *     SOC_E_NONE
 */
STATIC int
_soc_arad_linkctrl_linkscan_config(int unit, pbmp_t hw_mii_pbm,
                                    pbmp_t hw_direct_pbm)
{
    uint32 reg32_val;
    soc_control_t *soc = SOC_CONTROL(unit);
    pbmp_t pbm;
    SOCDNX_INIT_FUNC_DEFS;

    if (SOC_PBMP_NOT_NULL(hw_mii_pbm) || SOC_PBMP_NOT_NULL(hw_direct_pbm)) {

        /*unmask linkscan status*/
        soc_cmicm_cmcx_intr1_enable(unit, 0, IRQ_CMCx_LINK_STAT_MOD);

        /*clear linkscan status*/
        reg32_val = 0;
        soc_reg_field_set(unit, CMIC_MIIM_CLR_SCAN_STATUSr, &reg32_val, CLR_LINK_STATUS_CHANGEf, 1);
        SOCDNX_IF_ERR_EXIT(WRITE_CMIC_MIIM_CLR_SCAN_STATUSr(unit, reg32_val));
        
     } else {

        /*mask linkscan status*/
        soc_cmicm_cmcx_intr1_disable(unit, 0, IRQ_CMCx_LINK_STAT_MOD);

        /*clear linkscan status*/
        reg32_val = 0;
        soc_reg_field_set(unit, CMIC_MIIM_CLR_SCAN_STATUSr, &reg32_val, CLR_LINK_STATUS_CHANGEf, 1);
        SOCDNX_IF_ERR_EXIT(WRITE_CMIC_MIIM_CLR_SCAN_STATUSr(unit, reg32_val));
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
 
exit:
    SOCDNX_FUNC_RETURN;
}

/*
 * Function:    
 *     _soc_arad_linkctrl_update
 * Purpose:
 *     Update the forwarding state in device.
 * Parameters:  
 *      unit - Device unit number
 * Returns:
 *     SOC_E_NONE
 */
STATIC int
_soc_arad_linkctrl_update(int unit)
{
    SOCDNX_INIT_FUNC_DEFS;
    
    SOCDNX_FUNC_RETURN;
}

STATIC int
_soc_arad_linkctrl_hw_link_get(int unit, soc_pbmp_t *hw_link)
{
    uint32 reg_val;
    uint32 link_changed;
    SOCDNX_INIT_FUNC_DEFS;

    SOCDNX_NULL_CHECK(hw_link);

    link_changed = 0;
    reg_val  = 0;
    SOCDNX_IF_ERR_EXIT(READ_CMIC_MIIM_SCAN_STATUSr(unit, &reg_val));
    link_changed = soc_reg_field_get(unit, CMIC_MIIM_SCAN_STATUSr , reg_val,LINK_STATUS_CHANGEf);

    SOC_PBMP_CLEAR(*hw_link);
    if (link_changed) {
        SOC_PBMP_ASSIGN(*hw_link, PBMP_PORT_ALL(unit));
    }

exit:
    SOCDNX_FUNC_RETURN;
    
}



/*
 * Link Control Driver - Soc_petra
 */
CONST soc_linkctrl_driver_t  soc_linkctrl_driver_arad = {
    NULL,                                     /* port mapping */
    _soc_arad_linkctrl_linkscan_hw_init,     /* ld_linkscan_hw_init */
    _soc_arad_linkctrl_linkscan_config,      /* ld_linkscan_config */
    _soc_arad_linkctrl_linkscan_pause,       /* ld_linkscan_pause */
    _soc_arad_linkctrl_linkscan_continue,    /* ld_linkscan_continue */
    _soc_arad_linkctrl_update,                /* ld_update */
    _soc_arad_linkctrl_hw_link_get           /* ld_hw_link_get */

};

