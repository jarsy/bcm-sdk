/*
 * $Id: port.c,v 1.4 Broadcom SDK $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */

#include <shared/bsl.h>

#include <shared/error.h>
#include <soc/robo.h>
#include <soc/types.h>
#include <soc/debug.h>
#include <soc/robo/mcm/driver.h>
#include <soc/phy.h>
#include <soc/phy/phyctrl.h>
#include <soc/phyreg.h>

/*
 * soc_robo_port_info
 */

soc_robo_port_info_t *soc_robo_port_info[SOC_MAX_NUM_DEVICES];

/*
 *  Function : soc_port_detach
 *
 *  Purpose :
 *      detach the sw port_info database.
 *
 *  Parameters :
 *      unit    :   unit id
 *      okay    :   Output parameter indicates port can be enabled.
 *
 *  Return :
 *      SOC_E_XXX
 *
 *  Note :
 *      If error is returned, the port should not be enabled.
 *
 */
int
drv_robo_port_sw_detach(int unit)
{
    if (soc_robo_port_info[unit] == NULL){
        return SOC_E_NONE;
    }
    
    sal_free(soc_robo_port_info[unit]);
    soc_robo_port_info[unit] = NULL;
    
    return SOC_E_NONE; 
}

/*
 *  Function : soc_port_probe
 *
 *  Purpose :
 *      Probe the phy and set up the phy and mac of the indicated port.
 *
 *  Parameters :
 *      unit        :   unit id
 *      p    :   port to probe.
 *      okay  :   Output parameter indicates port can be enabled.
 *
 *  Return :
 *      SOC_E_XXX
 *
 *  Note :
 *      If error is returned, the port should not be enabled.
 *
 */
int
drv_robo_port_probe(int unit, soc_port_t p, int *okay)
{
    int            rv;
    mac_driver_t    *macd;
    uint32 default_port_medium;

    *okay = FALSE;


    if (soc_robo_port_info[unit] == NULL) 
    {
        soc_robo_port_info[unit] = 
            sal_alloc(sizeof(soc_robo_port_info_t) * SOC_ROBO_MAX_NUM_PORTS, "soc_port_info");
        if (soc_robo_port_info[unit] == NULL) 
        {
            return SOC_E_MEMORY;
        }
        sal_memset(soc_robo_port_info[unit], 0, sizeof(soc_robo_port_info_t) * SOC_ROBO_MAX_NUM_PORTS);
    }
    
#ifdef BCM_NORTHSTARPLUS_SUPPORT
    /*  
     * Since for NS+, the lane 0 is port 5, lane 1 is port 4.
     * The port 5 should be initialized first.
     */
    if (SOC_IS_NORTHSTARPLUS(unit) && (p == 4)) {
        if (SOC_PBMP_MEMBER(PBMP_E_ALL(unit), 5)) {
            if ((soc_phy_addr_int_of_port(unit, 4) & PHY_ADDR_ROBO_INT_SERDES) 
                && (soc_phy_addr_int_of_port(unit, 5) 
                & PHY_ADDR_ROBO_INT_SERDES)) {
                rv = soc_phyctrl_probe(unit, 5);
                if (rv == SOC_E_NONE) {
                    rv = soc_phyctrl_init(unit, 5);
                    if (rv < 0) {
                        return rv;
                    }
                }
            }

        }
            
    }
#endif /* BCM_NORTHSTARPLUS_SUPPORT */

    if ((rv = soc_phyctrl_probe(unit, p)) < 0) {
        return rv;
    } 

    if ((rv = soc_phyctrl_init(unit, p)) < 0) {    
        return rv;
    }

    /*
     * set default phy medium on each port (FE/GE)
     */

    SOC_IF_ERROR_RETURN(DRV_PORT_GET(unit, p, DRV_PORT_PROP_PHY_MEDIUM, 
        &default_port_medium));
    SOC_ROBO_PORT_MEDIUM_MODE(unit, p) = default_port_medium;


    /*
     * Currently initializing MAC after PHY is required.
     */

    LOG_VERBOSE(BSL_LS_SOC_PORT,
                (BSL_META_U(unit,
                            "Init port %d MAC...\n"), p));

    if ((rv = soc_robo_mac_probe(unit, p, &macd)) < 0) {
        LOG_WARN(BSL_LS_SOC_PORT,
                 (BSL_META_U(unit,
                             "Unit %d Port %s: Failed to probe MAC: %s\n"),
                  unit, SOC_PORT_NAME(unit, p), soc_errmsg(rv)));
        return rv;
    }

    SOC_ROBO_PORT_MAC_DRIVER(unit, p) = macd;

    if ((rv = MAC_INIT(macd, unit, p)) < 0) {
        LOG_WARN(BSL_LS_SOC_PORT,
                 (BSL_META_U(unit,
                             "Unit %d Port %s: Failed to initialize MAC: %s\n"),
                  unit, SOC_PORT_NAME(unit, p), soc_errmsg(rv)));
        return rv;
    }


    *okay = TRUE;

    return SOC_E_NONE;
}

/*
 *  Function : soc_port_sw_init
 *
 *  Purpose :
 *      Check if sw port_info database.
 *
 *  Parameters :
 *      unit        :   unit id
 *
 *  Return :
 *      SOC_E_XXX
 *
 *  Note :
 *      If error is returned, the port should not be enabled.
 *
 */
int
drv_robo_port_sw_init_status_get(int unit)
{
    if (soc_robo_port_info[unit] == NULL) {
        return SOC_E_INIT;
    } else {
        return SOC_E_NONE;
    }
}

uint32
drv_robo_port_sample_rate_get(int unit, uint32 val)
{
    int i,  tmp1 = 0, tmp2 = 0;
    uint32  ret_val = 0, cap = 0, ing_max = 0;
    
    /* Get the maximum value can be set in the register's field */
    /* current design check ingress max value only :
     *  - the ingress/egress RMON/SFLOW feature currently supported in ROBO 
     *      chips in bcm53242/bcm53262/bcm5328x
     */
    if (DRV_DEV_PROP_GET(
            unit, DRV_DEV_PROP_MAX_INGRESS_SFLOW_VALUE, &ing_max)) {
        cap = 0;
        return cap;
    } else {
        if (ing_max == 0){
            cap = 0;
            return cap;
        }
    } 
    cap = ing_max;
    
    /* Get the value to be set, "2" is the base and "i" is the exponenet */
    tmp1 = 1;
    for (i = 0; i < cap; i++) {
        if (tmp1 == val) {
            /* If 2^i equal to the "val", then "i" is the value to be set */
            return i;
        } else {
            /* 
             * Check if the "val" is between 2^i and 2^(i+1)
             * If yes, the nearest exponent is found and stop the loop.
             */
            tmp2 = tmp1 * 2;
            if ((val > tmp1) && (val < tmp2)) {
                /* get the nearest i */
                break;
            }
        }
        tmp1 *= 2;
    }

    /* Check whether 2^i or 2^(i+1) is closer to "val" */
    if ((val - tmp1) <= (tmp2 - val)) {
        ret_val = i;
    } else {
        ret_val = i + 1;
    }

    return ret_val;
}


/*
 *  Function : drv_port_advertise_set
 *
 *  Purpose :
 *      Set the advertise capability to the specific ports.
 *
 *  Parameters :
 *      unit        :   unit id
 *      bmp   :   port bitmap.
 *      prop_mask  :   port advertise capability.
 *
 *  Return :
 *      SOC_E_XXX
 *
 *  Note :
 *      
 *
 */
int 
drv_port_advertise_set(int unit, soc_pbmp_t bmp, uint32 prop_mask)
{
    int             rv = SOC_E_NONE;
    int             port;

    LOG_INFO(BSL_LS_SOC_PORT,
             (BSL_META_U(unit,
                         "drv_port_advertise_set: unit = %d, bmp = %x %x, adv_value = %x\n"),
              unit, SOC_PBMP_WORD_GET(bmp, 0), SOC_PBMP_WORD_GET(bmp, 1), prop_mask));

    SOC_ROBO_PORT_INIT(unit);
    PBMP_ITER(bmp, port) {
        SOC_IF_ERROR_RETURN
            (soc_phyctrl_adv_local_set(unit, port, prop_mask));
    }

    return rv;
}

/*
 *  Function : drv_port_advertise_get
 *
 *  Purpose :
 *      Get the advertise capability to the specific ports.
 *
 *  Parameters :
 *      unit        :   unit id
 *      port_n   :   port number.
 *      prop_val  :   port advertise capability.
 *
 *  Return :
 *      SOC_E_XXX
 *
 *  Note :
 *      
 *
 */
int 
drv_port_advertise_get(int unit, int port_n, uint32 *prop_val)
{
    int     rv = SOC_E_NONE;

    LOG_INFO(BSL_LS_SOC_PORT,
             (BSL_META_U(unit,
                         "drv_port_advertise_get\n")));

    SOC_ROBO_PORT_INIT(unit);
    rv = soc_phyctrl_adv_local_get(unit, port_n, prop_val);
    LOG_INFO(BSL_LS_SOC_PORT,
             (BSL_META_U(unit,
                         "drv_port_advertise_get: unit = %d, port = %d, adv_value = %x\n"),
              unit, port_n, *prop_val));

    return rv;
}

/*
 *  Function : drv_port_sw_mac_update
 *
 *  Purpose :
 *      Update the port state and SW override into MAC.
 *
 *  Parameters :
 *      unit    :   unit id
 *      port    :   port number.
 *
 *  Return :
 *      None.
 *
 *  Note :
 *      
 *
 */
int 
drv_port_sw_mac_update(int unit, soc_pbmp_t bmp)
{
    int port, duplex, speed;
    int rv;
    mac_driver_t *p_mac = NULL;      

    LOG_INFO(BSL_LS_SOC_PORT,
             (BSL_META_U(unit,
                         "drv_port_sw_mac_update: unit = %d, bmp = %x %x\n"),
              unit, SOC_PBMP_WORD_GET(bmp, 0), SOC_PBMP_WORD_GET(bmp, 1)));
        
    PBMP_ITER(bmp, port) {
        rv = soc_phyctrl_speed_get(unit, port, &speed);
        p_mac = SOC_ROBO_PORT_MAC_DRIVER(unit, port);
        if (SOC_FAILURE(rv) && (SOC_E_UNAVAIL != rv)) {
            LOG_WARN(BSL_LS_SOC_COMMON,
                     (BSL_META_U(unit,
                                 "u=%d p=%d phyctrl_speed_get rv=%d\n"),unit, port, rv));
            return rv;
        }
        if (SOC_E_UNAVAIL == rv ) {
            /* If PHY driver doesn't support speed_get, don't change 
             * MAC speed. E.g, Null PHY driver 
             */
            rv = SOC_E_NONE;
        } else {            
            if (p_mac != NULL) {
                rv =  (MAC_SPEED_SET(p_mac, unit, port, speed));
            }
        }
        if (SOC_FAILURE(rv)) {
            LOG_WARN(BSL_LS_SOC_COMMON,
                     (BSL_META_U(unit,
                                 "u=%d p=%d MAC_SPEED_SET speed=%d rv=%d\n"),
                      unit, port, speed, rv));
            return rv;
        }

        rv =   (soc_phyctrl_duplex_get(unit, port, &duplex));
        if (SOC_FAILURE(rv)) {
            LOG_WARN(BSL_LS_SOC_COMMON,
                     (BSL_META_U(unit,
                                 "u=%d p=%d phyctrl_duplex_get rv=%d\n"),
                      unit, port, rv));
            return rv;
        }
        if (p_mac != NULL) {
            rv = (MAC_DUPLEX_SET(p_mac, unit, port, duplex));
        }
        if (SOC_FAILURE(rv)) {
            LOG_WARN(BSL_LS_SOC_COMMON,
                     (BSL_META_U(unit,
                                 "u=%d p=%d MAC_DUPLEX_SET %s sp=%d rv=%d\n"), 
                      unit, port, 
                      duplex ? "FULL" : "HALF", speed, rv));
            return rv;
        }            
    }
    
    return SOC_E_NONE;

}

/*
 *  Function : drv_port_oper_mode_set
 *
 *  Purpose :
 *      Set operation mode to  specific ports.
 *      (This is not suitable for ROBO)
 *
 *  Parameters :
 *      unit        :   unit id
 *      bmp   :   port bitmap.
 *      oper_mode  :   operation mode.
 *
 *  Return :
 *      SOC_E_XXX
 *
 *  Note :
 *      
 *
 */
 int 
 drv_port_oper_mode_set(int unit, soc_pbmp_t bmp, uint32 oper_mode)
{
    LOG_INFO(BSL_LS_SOC_PORT,
             (BSL_META_U(unit,
                         "drv_port_oper_mode_set : unit %d, bmp = %x,  mode = %d not support\n"),
              unit, SOC_PBMP_WORD_GET(bmp, 0), oper_mode));
    return SOC_E_UNAVAIL;
}

 /*
 *  Function : drv_port_oper_mode_get
 *
 *  Purpose :
 *      Get operation mode to specific ports.
 *      (This is not suitable for ROBO)
 *
 *  Parameters :
 *      unit        :   unit id
 *      bmp   :   port bitmap.
 *      oper_mode  :   operation mode.
 *
 *  Return :
 *      SOC_E_XXX
 *
 *  Note :
 *      
 *
 */
int 
drv_port_oper_mode_get(int unit, int port_n, uint32 *oper_mode)
{
    LOG_INFO(BSL_LS_SOC_PORT,
             (BSL_META_U(unit,
                         "drv_port_oper_mode_get : unit %d, port = %x,  mode = %d not support\n"),
              unit, port_n, *oper_mode));
    return SOC_E_UNAVAIL;
}

/*
 *  Function : drv_port_bitmap_get
 *
 *  Purpose :
 *      Get the port bitmap of the selected port type.
 *
 *  Parameters :
 *      unit        :   unit id
 *      port_type   :   port type.
 *      bitmap  :   port bitmap.
 *
 *  Return :
 *      SOC_E_XXX
 *
 *  Note :
 *      The GE type will include 1G and 2.5G ports.
 *
 */
int
drv_port_bitmap_get(int unit, uint32 port_type, soc_pbmp_t * bitmap)
{
    switch (port_type) {
        case DRV_PORT_TYPE_10_100:
            *bitmap = PBMP_FE_ALL(unit);
            break;
        case DRV_PORT_TYPE_G:
            /* 2.5G port will be grouped here as well */
            *bitmap = PBMP_GE_ALL(unit);
            break;
        case DRV_PORT_TYPE_XG:
            *bitmap = PBMP_XE_ALL(unit);
            break;
        case DRV_PORT_TYPE_CPU:
            *bitmap = PBMP_CMIC(unit);
            break;
        case DRV_PORT_TYPE_MGNT:
            *bitmap = PBMP_SPI(unit);
            break;
        case DRV_PORT_TYPE_ALL:
            *bitmap = PBMP_ALL(unit);
            break;
    }
    LOG_INFO(BSL_LS_SOC_PORT,
             (BSL_META_U(unit,
                         "drv_port_bitmap_get: unit = %d, port type = %d, bmp = %x %x\n"),
              unit, port_type, SOC_PBMP_WORD_GET(*bitmap, 0), SOC_PBMP_WORD_GET(*bitmap, 1)));

    return SOC_E_NONE;
}


