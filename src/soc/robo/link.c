/*
 * $Id: link.c,v 1.27 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * Hardware Linkscan module
 *
 * Hardware linkscan is available, but its use is not recommended
 * because a software linkscan task is very low overhead and much more
 * flexible.
 *
 * If hardware linkscan is used, each MII operation must temporarily
 * disable it and wait for the current scan to complete, increasing the
 * latency.  PHY status register 1 may contain clear-on-read bits that
 * will be cleared by hardware linkscan and not seen later.  Special
 * support is provided for the Serdes MAC.
 */

#include <shared/bsl.h>

#include <sal/core/libc.h>
#include <sal/core/boot.h>

#include <soc/mem.h>
#include <soc/debug.h>
#include <soc/cm.h>

#include <soc/robo/mcm/driver.h>
#include <soc/error.h>
#include <soc/cmic.h>
#include <soc/register.h>

#ifdef  BCM_NORTHSTARPLUS_SUPPORT
#include "gex/northstarplus/robo_northstarplus.h"
#endif /* BCM_NORTHSTARPLUS_SUPPORT */

/*
 * Function:    
 *      soc_robo_wan_port_link_get
 * Purpose:
 *      Get current linking state of WAN port.
 * Parameters:  
 *      unit - RoboSwitch unit #.
 *      *link - current link status of WAN port.
 * Returns:
 *      SOC_E_XXX
 * Note : 
 *      1. Get link status of WAN port.
 */
int
soc_robo_wan_port_link_get(int unit, int *link)
{
#ifdef WAN_PORT_SUPPORT
    uint32 reg_addr, reg_len, reg_value, link_sts;

    if (SOC_IS_ROBO5397(unit)) {
        reg_addr = (DRV_SERVICES(unit)->reg_addr)
                        (unit, E_MIISTSr, 0, 0);
        reg_len = (DRV_SERVICES(unit)->reg_length_get)
                        (unit, E_MIISTSr);
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_read)
                            (unit, reg_addr, &reg_value, reg_len));
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_get)
            (unit, E_MIISTSr, &reg_value, LINK_STAf, &link_sts));
        *link = (link_sts) ? TRUE: FALSE;
    }
#endif
    return SOC_E_NONE;
}

/*
 * Function:    
 *      soc_robo_wan_port_link_sw_update
 * Purpose:
 *      Update the linking state of WAN port.
 * Parameters:  
 *      unit - RoboSwitch unit #.
 *      link - link status to be override.
 * Returns:
 *      SOC_E_XXX
 * Note : 
 *      1. Do software override to Mac.
 */
int
soc_robo_wan_port_link_sw_update(int unit, int link)
{
#ifdef WAN_PORT_SUPPORT
    uint32 reg_addr, reg_len, reg_value;

    if (SOC_IS_ROBO5397(unit)) {
        reg_addr = (DRV_SERVICES(unit)->reg_addr)
                        (unit, STS_OVERRIDE_WAN_Pr, 0, 0);
        reg_len = (DRV_SERVICES(unit)->reg_length_get)
                        (unit, STS_OVERRIDE_WAN_Pr);
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_read)
                            (unit, reg_addr, &reg_value, reg_len));
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
            (unit, STS_OVERRIDE_WAN_Pr, &reg_value, LINK_STSf, &link));
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_write)
            (unit, reg_addr, &reg_value, reg_len));
    }
#endif
    return SOC_E_NONE;
}

/*
 * Function:    
 *      soc_robo_link_sw_update
 * Purpose:
 *      Update the linking state in the chip on the assigned port
 * Parameters:  
 *      unit - RoboSwitch unit #.
 *      port - Port to process.
 * Returns:
 *      SOC_E_XXX
 * Note : 
 *      1. return SOC_E_PARAM is the port is CPU port.
 */

int
soc_robo_link_sw_update(int unit, int link, int port)
{
    uint32 	override;
    uint32  reg_value;
    
    int rv;
#ifdef BCM_NORTHSTARPLUS_SUPPORT
    uint32        mux_reg_addr, mux_reg_val;
#endif /* BCM_NORTHSTARPLUS_SUPPORT */
    
    /* Here we only override the link status
      * Disable phy-polling is done in mac_init
      */
    OVERRIDE_LOCK(unit);
    override = 1;
    if (IS_FE_PORT(unit, port)){
    	if (SOC_IS_LOTUS(unit)) {
    		rv = REG_READ_STS_OVERRIDE_GMIIPr(unit, port, &reg_value);
    	} else { 
        	rv = REG_READ_STS_OVERRIDE_Pr(unit, port, &reg_value);
    	}
        if(SOC_FAILURE(rv)) {
            OVERRIDE_UNLOCK(unit);
        }
        
        if (SOC_IS_LOTUS(unit)) {
        	soc_STS_OVERRIDE_GMIIPr_field_set(unit, &reg_value, LINK_STSf, 
                (uint32 *)&link);        
    	} else {
        	soc_STS_OVERRIDE_Pr_field_set(unit, &reg_value, LINK_STSf, 
                (uint32 *)&link);        
    	}
        if (SOC_IS_TBX(unit)) {
            soc_STS_OVERRIDE_Pr_field_set(unit, &reg_value, SW_OVERRIDEf, &override);
    	} else if (SOC_IS_LOTUS(unit)) {
    		soc_STS_OVERRIDE_GMIIPr_field_set(unit, &reg_value, SW_OVERRIDEf, &override);
        } else {        
            soc_STS_OVERRIDE_Pr_field_set(unit, &reg_value, SW_ORDf, &override);
        }
    
        if (SOC_IS_LOTUS(unit)) {
            rv = REG_WRITE_STS_OVERRIDE_GMIIPr(unit, port, &reg_value);
        } else {
            rv = REG_WRITE_STS_OVERRIDE_Pr(unit, port, &reg_value);
        }
        if(SOC_FAILURE(rv)) {
            OVERRIDE_UNLOCK(unit);
        }

    } else if(IS_GE_PORT(unit, port)) {
        if (soc_feature(unit, soc_feature_robo_ge_serdes_mac_autosync)){
            OVERRIDE_UNLOCK(unit);
            return SOC_E_NONE;
        }         

        if (SOC_IS_VULCAN(unit) || SOC_IS_BLACKBIRD(unit) ||
            SOC_IS_STARFIGHTER(unit) || SOC_IS_BLACKBIRD2(unit) ||
            SOC_IS_POLAR(unit) || SOC_IS_NORTHSTAR(unit) ||
            SOC_IS_NORTHSTARPLUS(unit) || SOC_IS_STARFIGHTER3(unit)) {
            rv = REG_READ_STS_OVERRIDE_GMIIPr(unit, port, &reg_value);
            if(SOC_FAILURE(rv)) {
                OVERRIDE_UNLOCK(unit);
            }

            soc_STS_OVERRIDE_GMIIPr_field_set(unit, &reg_value, LINK_STSf, 
                (uint32 *)&link);
            if (SOC_IS_STARFIGHTER(unit) || SOC_IS_BLACKBIRD2(unit) ||
                SOC_IS_POLAR(unit) || SOC_IS_NORTHSTAR(unit)||
                SOC_IS_NORTHSTARPLUS(unit) || SOC_IS_STARFIGHTER3(unit)) {
                soc_STS_OVERRIDE_GMIIPr_field_set(unit, &reg_value, 
                    SW_OVERRIDEf, &override);
            } else {
                soc_STS_OVERRIDE_GMIIPr_field_set(unit, &reg_value, 
                    SW_OVERRIDE_Rf, &override);
            }
                        
            rv = REG_WRITE_STS_OVERRIDE_GMIIPr(unit, port, &reg_value);
            if(SOC_FAILURE(rv)) {
                OVERRIDE_UNLOCK(unit);
            }
        } else {
            rv = REG_READ_STS_OVERRIDE_GPr(unit, port, &reg_value);
            if(SOC_FAILURE(rv)) {
                OVERRIDE_UNLOCK(unit);
            }
   
            soc_STS_OVERRIDE_GPr_field_set(unit, &reg_value, LINK_STSf, 
                (uint32 *)&link);
            if (SOC_IS_TBX(unit)) {
                soc_STS_OVERRIDE_GPr_field_set(unit, &reg_value, SW_OVERRIDEf, &override);
            } else {
                soc_STS_OVERRIDE_GPr_field_set(unit, &reg_value, SW_ORDf, &override);
            }

            rv = REG_WRITE_STS_OVERRIDE_GPr(unit, port, &reg_value);
            if(SOC_FAILURE(rv)) {
                OVERRIDE_UNLOCK(unit);
            }
        }
    }

    LOG_VERBOSE(BSL_LS_SOC_LINK,
                (BSL_META_U(unit,
                            "soc_robo_link_sw_update: link=%d port=%d\n"), link, port));

    OVERRIDE_UNLOCK(unit);

#ifdef BCM_NORTHSTARPLUS_SUPPORT
    if (SOC_IS_NORTHSTARPLUS(unit) && ((port == 5) || (port == 4))) {
        mux_reg_addr = 0;
        if (port == 5) {
            mux_reg_addr = P5_MUX_REG_ADDR;
        } else {
            mux_reg_addr = P4_MUX_REG_ADDR;
        }
        /* Update the link status */
        mux_reg_val = CMREAD(soc_eth_unit, mux_reg_addr);
        if (link) {
            mux_reg_val |= (MUX_REG_LINK_FIELD_MASK << 
                MUX_REG_LINK_FIELD_SHIFT);
        } else {
            mux_reg_val &= ~(MUX_REG_LINK_FIELD_MASK << 
                MUX_REG_LINK_FIELD_SHIFT);
        }
        CMWRITE(soc_eth_unit, mux_reg_addr, mux_reg_val);
    }
            
#endif /* BCM_NORTHSTARPLUS_SUPPORT */
    return SOC_E_NONE;
    
}

