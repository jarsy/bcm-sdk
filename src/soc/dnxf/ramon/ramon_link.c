 /* 
 * $Id: ramon_link.c,v 1.3 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:        ramon_link.c
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
#include <shared/bsl.h>
#include <soc/linkctrl.h>
#include <soc/drv.h>
#include <soc/error.h>
#include <soc/cmicm.h>
#include <soc/mcm/memregs.h>
#include <soc/mcm/cmicm.h>

#include <soc/dnxc/legacy/error.h>

#include <soc/dnxf/ramon/ramon_link.h>


/*
 * Function:
 *     _soc_ramon_linkctrl_linkscan_hw_init
 * Purpose:
 *     Initialize hardware linkscan.
 * Parameters:
 *     unit  - Device number
 * Returns:
 *     SOC_E_NONE
 */
STATIC int
_soc_ramon_linkctrl_linkscan_hw_init(int unit)
{

    uint32 reg32_val;
    DNXC_INIT_FUNC_DEFS;

    /*assume already init for warmboot*/
    if(SOC_WARM_BOOT(unit)) {
        SOC_EXIT;
    }

    /*Set internal phy link scan*/
    DNXC_IF_ERR_EXIT(WRITE_CMIC_MIIM_INT_SEL_MAP_0r(unit,  0xffffffff));
    DNXC_IF_ERR_EXIT(WRITE_CMIC_MIIM_INT_SEL_MAP_1r(unit,  0xffffffff));
    DNXC_IF_ERR_EXIT(WRITE_CMIC_MIIM_INT_SEL_MAP_2r(unit,  0xffffffff));
    DNXC_IF_ERR_EXIT(WRITE_CMIC_MIIM_INT_SEL_MAP_3r(unit,  0xffffffff));
    DNXC_IF_ERR_EXIT(WRITE_CMIC_MIIM_INT_SEL_MAP_4r(unit,  0xffffffff));
    DNXC_IF_ERR_EXIT(WRITE_CMIC_MIIM_INT_SEL_MAP_5r(unit,  0xffffffff));

    
    DNXC_IF_ERR_EXIT(READ_CMIC_MIIM_SCAN_CTRLr(unit, &reg32_val));
    /*disable linkscan - enable when hw linkscan will be cfg*/
    soc_reg_field_set(unit, CMIC_MIIM_SCAN_CTRLr, &reg32_val, MIIM_LINK_SCAN_ENf, 0);
    soc_reg_field_set(unit, CMIC_MIIM_SCAN_CTRLr, &reg32_val, STOP_LS_ON_FIRST_CHANGEf, 0);
    soc_reg_field_set(unit, CMIC_MIIM_SCAN_CTRLr, &reg32_val, STOP_LS_ON_CHANGEf, 0);
    soc_reg_field_set(unit, CMIC_MIIM_SCAN_CTRLr, &reg32_val, OVER_RIDE_EXT_MDIO_MSTR_CNTRLf, 1);
    DNXC_IF_ERR_EXIT(WRITE_CMIC_MIIM_SCAN_CTRLr(unit, reg32_val));

    /*Do not scan any port*/
    DNXC_IF_ERR_EXIT(WRITE_CMIC_MIIM_SCAN_PORTS_0r(unit, 0x0));
    DNXC_IF_ERR_EXIT(WRITE_CMIC_MIIM_SCAN_PORTS_1r(unit, 0x0));
    DNXC_IF_ERR_EXIT(WRITE_CMIC_MIIM_SCAN_PORTS_2r(unit, 0x0));
    DNXC_IF_ERR_EXIT(WRITE_CMIC_MIIM_SCAN_PORTS_3r(unit, 0x0));
    DNXC_IF_ERR_EXIT(WRITE_CMIC_MIIM_SCAN_PORTS_4r(unit, 0x0));
    DNXC_IF_ERR_EXIT(WRITE_CMIC_MIIM_SCAN_PORTS_5r(unit, 0x0));

exit:
    DNXC_FUNC_RETURN;
}

STATIC int
_soc_ramon_linkscan_ports_write(int unit, pbmp_t hw_mii_pbm)
{
    uint32      link_pbmp;
     
    DNXC_INIT_FUNC_DEFS;

   link_pbmp = SOC_PBMP_WORD_GET(hw_mii_pbm, 0); 
   DNXC_IF_ERR_EXIT(WRITE_CMIC_MIIM_SCAN_PORTS_0r(unit, link_pbmp));
   link_pbmp = SOC_PBMP_WORD_GET(hw_mii_pbm, 1); 
   DNXC_IF_ERR_EXIT(WRITE_CMIC_MIIM_SCAN_PORTS_1r(unit, link_pbmp));
   link_pbmp = SOC_PBMP_WORD_GET(hw_mii_pbm, 2); 
   DNXC_IF_ERR_EXIT(WRITE_CMIC_MIIM_SCAN_PORTS_2r(unit, link_pbmp));
   link_pbmp = SOC_PBMP_WORD_GET(hw_mii_pbm, 3); 
   DNXC_IF_ERR_EXIT(WRITE_CMIC_MIIM_SCAN_PORTS_3r(unit, link_pbmp));
   link_pbmp = SOC_PBMP_WORD_GET(hw_mii_pbm, 4); 
   DNXC_IF_ERR_EXIT(WRITE_CMIC_MIIM_SCAN_PORTS_4r(unit, link_pbmp));
   DNXC_IF_ERR_EXIT(WRITE_CMIC_MIIM_SCAN_PORTS_5r(unit, 0));
  
exit:
    DNXC_FUNC_RETURN;  
}

/*
 * Function:
 *     _soc_ramon_linkctrl_linkscan_pause
 * Purpose:
 *     Pause link scanning, without disabling it.
 *     This call is used to pause scanning temporarily.
 * Parameters:
 *     unit  - Device number
 * Returns:
 *     SOC_E_NONE
 */
STATIC int
_soc_ramon_linkctrl_linkscan_pause(int unit)
{
    int stall_count;
    pbmp_t pbm;
    uint32 schan_ctrl;
    soc_timeout_t to;
    uint32 busy_status, reg32_val;
    DNXC_INIT_FUNC_DEFS;

    /* First clear HW linkscan ports */
    SOC_PBMP_CLEAR(pbm);
    DNXC_IF_ERR_EXIT(_soc_ramon_linkscan_ports_write(unit, pbm));

    /* Turn off HW linkscan */
    DNXC_IF_ERR_EXIT(READ_CMIC_MIIM_SCAN_CTRLr(unit,&schan_ctrl));
    soc_reg_field_set(unit, CMIC_MIIM_SCAN_CTRLr, &schan_ctrl, MIIM_LINK_SCAN_ENf, 0);
    DNXC_IF_ERR_EXIT(WRITE_CMIC_MIIM_SCAN_CTRLr(unit,schan_ctrl));
    
    /* Wait for Linkscan stopped signal */
    soc_timeout_init(&to, 1000000 /*1 sec*/, 100);
    busy_status = 1;
    while (busy_status) 
    {
        if (soc_timeout_check(&to)) {
           DNXC_EXIT_WITH_ERR(SOC_E_TIMEOUT, (_BSL_DNXC_MSG("pausing hw linkscan failed")));
        }

        DNXC_IF_ERR_EXIT(READ_CMIC_MIIM_SCAN_STATUSr(unit, &reg32_val));
        busy_status = soc_reg_field_get(unit, CMIC_MIIM_SCAN_STATUSr, reg32_val, MIIM_SCAN_BUSYf);
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
    DNXC_FUNC_RETURN;
}

/*
 * Function:
 *     _soc_ramon_linkctrl_linkscan_continue
 * Purpose:
 *     Continue link scanning after it has been paused.
 * Parameters:
 *     unit  - Device number
 * Returns:
 *     SOC_E_NONE
 */
STATIC int
_soc_ramon_linkctrl_linkscan_continue(int unit)
{

    soc_control_t *soc = SOC_CONTROL(unit);
    uint32 schan_ctrl;
    DNXC_INIT_FUNC_DEFS;

    DNXC_IF_ERR_EXIT(_soc_ramon_linkscan_ports_write(unit, soc->hw_linkscan_pbmp));
    DNXC_IF_ERR_EXIT(READ_CMIC_MIIM_SCAN_CTRLr(unit,&schan_ctrl));
    soc_reg_field_set(unit, CMIC_MIIM_SCAN_CTRLr, &schan_ctrl, MIIM_LINK_SCAN_ENf, 1);
    DNXC_IF_ERR_EXIT(WRITE_CMIC_MIIM_SCAN_CTRLr(unit,schan_ctrl));
          
exit:
    DNXC_FUNC_RETURN;
}

/*
 * Function:
 *     _soc_ramon_linkctrl_linkscan_config
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
_soc_ramon_linkctrl_linkscan_config(int unit, pbmp_t hw_mii_pbm,
                                    pbmp_t hw_direct_pbm)
{
    uint32 reg32_val;
    soc_control_t *soc = SOC_CONTROL(unit);
    pbmp_t pbm;
    DNXC_INIT_FUNC_DEFS;

    if (SOC_PBMP_NOT_NULL(hw_mii_pbm) || SOC_PBMP_NOT_NULL(hw_direct_pbm)) {

        /*unmask linkscan status*/
        soc_cmicm_cmcx_intr1_enable(unit, 0, IRQ_CMCx_LINK_STAT_MOD);

        /*clear linkscan status*/
        reg32_val = 0;
        soc_reg_field_set(unit, CMIC_MIIM_CLR_SCAN_STATUSr, &reg32_val, CLR_LINK_STATUS_CHANGEf, 1);
        DNXC_IF_ERR_EXIT(WRITE_CMIC_MIIM_CLR_SCAN_STATUSr(unit, reg32_val));
        
     } else {

        /*mask linkscan status*/
        soc_cmicm_cmcx_intr1_disable(unit, 0, IRQ_CMCx_LINK_STAT_MOD);

        /*clear linkscan status*/
        reg32_val = 0;
        soc_reg_field_set(unit, CMIC_MIIM_CLR_SCAN_STATUSr, &reg32_val, CLR_LINK_STATUS_CHANGEf, 1);
        DNXC_IF_ERR_EXIT(WRITE_CMIC_MIIM_CLR_SCAN_STATUSr(unit, reg32_val));
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
    DNXC_FUNC_RETURN;
}

/*
 * Function:    
 *     _soc_ramon_linkctrl_update
 * Purpose:
 *     Update the forwarding state in device.
 * Parameters:  
 *      unit - Device unit number
 * Returns:
 *     SOC_E_NONE
 */
STATIC int
_soc_ramon_linkctrl_update(int unit)
{
    DNXC_INIT_FUNC_DEFS;
    
    DNXC_FUNC_RETURN;
}

STATIC int
_soc_ramon_linkctrl_hw_link_get(int unit, soc_pbmp_t *hw_link)
{
    DNXC_INIT_FUNC_DEFS;

    DNXC_NULL_CHECK(hw_link);

    SOC_PBMP_ASSIGN(*hw_link, PBMP_PORT_ALL(unit));

exit:
    DNXC_FUNC_RETURN;
    
}

/*
 * Link Control Driver - RAMON
 */
CONST soc_linkctrl_driver_t  soc_linkctrl_driver_ramon = {
    NULL,                                       /* port mapping */
    _soc_ramon_linkctrl_linkscan_hw_init,      /* ld_linkscan_hw_init */
    _soc_ramon_linkctrl_linkscan_config,       /* ld_linkscan_config */
    _soc_ramon_linkctrl_linkscan_pause,        /* ld_linkscan_pause */
    _soc_ramon_linkctrl_linkscan_continue,     /* ld_linkscan_continue */
    _soc_ramon_linkctrl_update,                /* ld_update */
    _soc_ramon_linkctrl_hw_link_get            /* ld_hw_link_get */

};

 
soc_error_t
soc_ramon_linkctrl_init(int unit) 
{
    DNXC_INIT_FUNC_DEFS;

    DNXC_IF_ERR_EXIT(soc_linkctrl_init(unit, &soc_linkctrl_driver_ramon));

exit:
    DNXC_FUNC_RETURN;
}




#undef _ERR_MSG_MODULE_NAME
