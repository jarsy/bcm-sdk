/* 
 * $Id: sbx_link.c,v 1.7.6.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:        sbx_link.c
 * Purpose:     Hardware Linkscan module
 *
 */

#include <soc/types.h>
#include <soc/error.h>
#include <soc/drv.h>
#include <soc/sbx/sbx_drv.h>
#include <soc/sbx/link.h>

#include <soc/linkctrl.h>


static soc_linkctrl_port_info_t  _soc_sbx_linkctrl_port_info[] = {
    {            -1,   -1,   -1,  -1 }   /* Last */
};



/*
 * Function:
 *     _soc_sbx_linkctrl_linkscan_hw_init
 * Purpose:
 *     Initialize hardware linkscan.
 * Parameters:
 *     unit  - Device number
 * Returns:
 *     SOC_E_XXX
 */
STATIC int
_soc_sbx_linkctrl_linkscan_hw_init(int unit)
{

    if (SOC_IS_SIRIUS(unit) || SOC_IS_CALADAN3(unit)) { 
            return (soc_linkscan_hw_init(unit));
    }

    return SOC_E_NONE;
}

/*
 * Function:
 *     _soc_sbx_linkctrl_linkscan_config
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
_soc_sbx_linkctrl_linkscan_config(int unit, pbmp_t hw_mii_pbm,
                                     pbmp_t hw_direct_pbm)
{
    COMPILER_REFERENCE(unit);

    if (SOC_PBMP_NOT_NULL(hw_mii_pbm) || SOC_PBMP_NOT_NULL(hw_direct_pbm)) {
         if (SOC_IS_SIRIUS(unit) || SOC_IS_CALADAN3(unit)) {
             return (soc_linkscan_config(unit, hw_mii_pbm, hw_direct_pbm));
         } 
    }

    return SOC_E_NONE;
}

/*
 * Function:
 *     _soc_sbx_linkctrl_linkscan_pause
 * Purpose:
 *     Pause link scanning, without disabling it.
 *     This call is used to pause scanning temporarily.
 * Parameters:
 *     unit  - Device number
 * Returns:
 *     SOC_E_XXX
 */
STATIC int
_soc_sbx_linkctrl_linkscan_pause(int unit)
{
    if (SOC_IS_SIRIUS(unit) || SOC_IS_CALADAN3(unit)) {
        soc_linkscan_pause(unit);
    }

    return SOC_E_NONE;
}

/*
 * Function:
 *     _soc_sbx_linkctrl_linkscan_continue
 * Purpose:
 *     Continue link scanning after it has been paused.
 * Parameters:
 *     unit  - Device number
 * Returns:
 *     SOC_E_XXX
 */
STATIC int
_soc_sbx_linkctrl_linkscan_continue(int unit)
{
    if (SOC_IS_SIRIUS(unit) || SOC_IS_CALADAN3(unit)) {
        soc_linkscan_continue(unit);
    }

    return SOC_E_NONE;
}

/*
 * Function:    
 *     _soc_sbx_linkctrl_update
 * Purpose:
 *     Update the forwarding state in device.
 * Parameters:  
 *      unit - Device unit number
 * Returns:
 *     SOC_E_XXX
 */
STATIC int
_soc_sbx_linkctrl_update(int unit)
{
    return SOC_E_NONE;
}

STATIC int
_soc_sbx_linkctrl_hw_link_get(int unit, soc_pbmp_t *hw_link)
{

    if (SOC_IS_SBX_SIRIUS(unit)) {
        uint32              link_stat;
        uint32              link_pbmp;
        soc_pbmp_t          tmp_pbmp;

        if (NULL == hw_link) {
            return SOC_E_PARAM;
        }

        SOC_PBMP_CLEAR(tmp_pbmp);
        SOC_IF_ERROR_RETURN(READ_CMIC_LINK_STATr(unit, &link_stat));
        link_pbmp = soc_reg_field_get(unit, CMIC_LINK_STATr,
                                      link_stat, PORT_BITMAPf);
        SOC_PBMP_WORD_SET(tmp_pbmp, 0, link_pbmp);
        /* coverity[result_independent_of_operands] */
        SOC_IF_ERROR_RETURN(READ_CMIC_LINK_STAT_HIr(unit, &link_stat));
        SOC_PBMP_WORD_SET(tmp_pbmp, 1,
                          soc_reg_field_get(unit, CMIC_LINK_STATr,
                                            link_stat, PORT_BITMAPf));
        SOC_PBMP_ASSIGN(*hw_link, tmp_pbmp);
    }

    if (SOC_IS_CALADAN3(unit)) {
        return soc_linkscan_hw_link_get(unit, hw_link);
    }

    return SOC_E_NONE;
}


/*
 * Link Control Driver - SBX
 */
soc_linkctrl_driver_t  soc_linkctrl_driver_sbx = {
    _soc_sbx_linkctrl_port_info,            /* port mapping */
    _soc_sbx_linkctrl_linkscan_hw_init,     /* ld_linkscan_hw_init */
    _soc_sbx_linkctrl_linkscan_config,      /* ld_linkscan_config */
    _soc_sbx_linkctrl_linkscan_pause,       /* ld_linkscan_pause */
    _soc_sbx_linkctrl_linkscan_continue,    /* ld_linkscan_continue */
    _soc_sbx_linkctrl_update,               /* ld_update */
    _soc_sbx_linkctrl_hw_link_get           /* ld_hw_link_get */
};
