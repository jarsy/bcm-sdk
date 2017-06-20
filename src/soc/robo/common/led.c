/*
 * $Id: led.c,v 1.5 Broadcom SDK $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
 
#include <shared/bsl.h>

#include <soc/robo.h>
#include <soc/drv.h>
#include <soc/robo/mcm/driver.h>
#include <soc/error.h>

#include <soc/debug.h>

/* ROBO LED solution about LED mode :
 *  
 * - Can be retrieved from led_mode_map_0 and led_mode_map_1 
 *   at specific port bit. The combined value is the led mode.
 *  >> Mode[1:0]
 *  >> b00 : Off
 *  >> b01 : On
 *  >> b10 : Blink
 *  >> b11 : Auto
 */
#define ROBO_LED_MODE_RETRIEVE(_mode0, _mode1)   \
        (((_mode1) == 0) ? \
            (((_mode0) == 0) ? DRV_LED_MODE_OFF : DRV_LED_MODE_ON) : \
            (((_mode0) == 0) ? DRV_LED_MODE_BLINK : DRV_LED_MODE_AUTO))

#define ROBO_LED_MODE_RESOLVE(mode, _mode0, _mode1)   \
        if((mode) == DRV_LED_MODE_OFF) {\
            (_mode1) = 0;   \
            (_mode0) = 0;   \
        } else if ((mode) == DRV_LED_MODE_ON){  \
            (_mode1) = 0;   \
            (_mode0) = 1;   \
        } else if ((mode) == DRV_LED_MODE_BLINK){   \
            (_mode1) = 1;   \
            (_mode0) = 0;   \
        } else if ((mode) == DRV_LED_MODE_AUTO){    \
            (_mode1) = 1;   \
            (_mode0) = 1;   \
        } else {    \
            (_mode1) = -1;  \
            (_mode0) = -1;  \
        }


/*
 * Function: 
 *	    drv_led_mode_get
 * Purpose:
 *	    Get the port based LED working mode.
 * Parameters:
 *	    port        - (IN) local port id.
 *      led_mode    - (OUT) led function group id
 * 
 * Note:
 *	1. Most of the ROBO chips support the same LED_FUNCITON group architecture
 *      - 2 LED funciton groups.
 *      - bcm5324, bcm5396 and bcm5389 doesn't support this feature.
 *  2. This routine is used to serve the related register read/write in 
 *      32 bits length boundery only.
 */
int
drv_led_mode_get(int unit,int port, uint32 *led_mode)
{
    uint32  mode32_val0 = 0, mode32_val1 = 0;
    uint32  fld32_val = 0;
    int     temp0, temp1;

    /* check if the processing register length is not suitable in this 
     * routine to prevent improper register read/write been proceeded.
     */
    if (DRV_REG_LENGTH_GET(unit, INDEX(LED_FUNC_MAPr)) > 4){
        LOG_WARN(BSL_LS_SOC_COMMON,
                 (BSL_META_U(unit,
                             "%s, Improper driver service been proceeded\n"), 
                  FUNCTION_NAME()));
        return SOC_E_INTERNAL;
    }

    SOC_IF_ERROR_RETURN(REG_READ_LED_MODE_MAP_0r(unit, &mode32_val0));
    if (SOC_IS_LOTUS(unit) || SOC_IS_STARFIGHTER(unit) || 
        SOC_IS_BLACKBIRD2(unit) || SOC_IS_POLAR(unit) ||
        SOC_IS_NORTHSTAR(unit) || SOC_IS_NORTHSTARPLUS(unit) ||
        SOC_IS_STARFIGHTER3(unit)) {
        SOC_IF_ERROR_RETURN(soc_LED_MODE_MAP_0r_field_get
            (unit, &mode32_val0, LED_MODE_MAP0f, &fld32_val));
    } else {
        SOC_IF_ERROR_RETURN(soc_LED_MODE_MAP_0r_field_get
            (unit, &mode32_val0, LED_MODE_MAPf, &fld32_val));
    }
    temp0 = (fld32_val & (0x1 << port)) ? 1 : 0;

    SOC_IF_ERROR_RETURN(REG_READ_LED_MODE_MAP_1r(unit, &mode32_val1));
    if (SOC_IS_LOTUS(unit) || SOC_IS_STARFIGHTER(unit) || 
        SOC_IS_BLACKBIRD2(unit) || SOC_IS_POLAR(unit) ||
        SOC_IS_NORTHSTAR(unit) || SOC_IS_NORTHSTARPLUS(unit) ||
        SOC_IS_STARFIGHTER3(unit)) {
        SOC_IF_ERROR_RETURN(soc_LED_MODE_MAP_1r_field_get
            (unit, &mode32_val1, LED_MODE_MAP1f, &fld32_val));
    } else {
        SOC_IF_ERROR_RETURN(soc_LED_MODE_MAP_1r_field_get
            (unit, &mode32_val1, LED_MODE_MAPf, &fld32_val));
    }
    temp1 = (fld32_val & (0x1 << port)) ? 1 : 0;

    *led_mode = ROBO_LED_MODE_RETRIEVE(temp0, temp1);

    return SOC_E_NONE;
}

/*
 * Function: 
 *	    drv_led_mode_set
 * Purpose:
 *	    Set the port based LED working mode.
 * Parameters:
 *	    port        - (IN) local port id.
 *      led_mode    - (OUT) led function group id
 * 
 * Note:
 *	1. Most of the ROBO chips support the same LED_FUNCITON group architecture
 *      - 2 LED funciton groups.
 *      - bcm5324, bcm5396 and bcm5389 doesn't support this feature.
 *  2. This routine is used to serve the related register read/write in 
 *      32 bits length boundery only.
 */
int
drv_led_mode_set(int unit,int port, uint32 led_mode)
{
    uint32  mode32_val0 = 0, mode32_val1 = 0;
    uint32  fld32_val = 0;
    int     temp0, temp1;

    /* check if the processing register length is not suitable in this 
     * routine to prevent improper register read/write been proceeded.
     */
    if (DRV_REG_LENGTH_GET(unit, INDEX(LED_FUNC_MAPr)) > 4){
        LOG_WARN(BSL_LS_SOC_COMMON,
                 (BSL_META_U(unit,
                             "%s, Improper driver service been proceeded\n"), 
                  FUNCTION_NAME()));
        return SOC_E_INTERNAL;
    }

    ROBO_LED_MODE_RESOLVE(led_mode, temp0, temp1);
    if (temp0 == -1 && temp1 == -1 ) {
        return SOC_E_PARAM;
    } else {
        SOC_IF_ERROR_RETURN(
                REG_READ_LED_MODE_MAP_0r(unit, &mode32_val0));
        SOC_IF_ERROR_RETURN(
                REG_READ_LED_MODE_MAP_1r(unit, &mode32_val1));
    }
    
    if (SOC_IS_LOTUS(unit) || SOC_IS_STARFIGHTER(unit) || 
        SOC_IS_BLACKBIRD2(unit) || SOC_IS_POLAR(unit) ||
        SOC_IS_NORTHSTAR(unit) || SOC_IS_NORTHSTARPLUS(unit) ||
        SOC_IS_STARFIGHTER3(unit)) {
        SOC_IF_ERROR_RETURN(soc_LED_MODE_MAP_0r_field_get
            (unit, &mode32_val0, LED_MODE_MAP0f, &fld32_val));
        if (temp0 == 0){
            fld32_val &= ~(0x1 << port); 
        } else {
            fld32_val |= (0x1 << port); 
        }
        SOC_IF_ERROR_RETURN(soc_LED_MODE_MAP_0r_field_set
            (unit, &mode32_val0, LED_MODE_MAP0f, &fld32_val));

        SOC_IF_ERROR_RETURN(soc_LED_MODE_MAP_1r_field_get
            (unit, &mode32_val1, LED_MODE_MAP1f, &fld32_val));
        if (temp1 == 0){
            fld32_val &= ~(0x1 << port); 
        } else {
            fld32_val |= (0x1 << port); 
        }
        SOC_IF_ERROR_RETURN(soc_LED_MODE_MAP_1r_field_set
            (unit, &mode32_val1, LED_MODE_MAP1f, &fld32_val));
    } else {
        SOC_IF_ERROR_RETURN(soc_LED_MODE_MAP_0r_field_get
            (unit, &mode32_val0, LED_MODE_MAPf, &fld32_val));
        if (temp0 == 0){
            fld32_val &= ~(0x1 << port); 
        } else {
            fld32_val |= (0x1 << port); 
        }
        SOC_IF_ERROR_RETURN(soc_LED_MODE_MAP_0r_field_set
            (unit, &mode32_val0, LED_MODE_MAPf, &fld32_val));
        
        SOC_IF_ERROR_RETURN(soc_LED_MODE_MAP_1r_field_get
            (unit, &mode32_val1, LED_MODE_MAPf, &fld32_val));
        if (temp1 == 0){
            fld32_val &= ~(0x1 << port); 
        } else {
            fld32_val |= (0x1 << port); 
        }
        SOC_IF_ERROR_RETURN(soc_LED_MODE_MAP_1r_field_set
            (unit, &mode32_val1, LED_MODE_MAPf, &fld32_val));
    }

    SOC_IF_ERROR_RETURN(
            REG_WRITE_LED_MODE_MAP_0r(unit, &mode32_val0));
    SOC_IF_ERROR_RETURN(
            REG_WRITE_LED_MODE_MAP_1r(unit, &mode32_val1));
    
    return SOC_E_NONE;
}

/*
 * Function: 
 *	    drv_led_funcgrp_select_get
 * Purpose:
 *	    Get the port based selctions of working LED function group.
 * Parameters:
 *	    port        - (IN) local port id.
 *      led_group   - (OUT) led function group id
 * 
 * Note:
 *	1. Most of the ROBO chips support the same LED_FUNCITON group architecture
 *      - 2 LED funciton groups.
 *      - bcm5324, bcm5396 and bcm5389 doesn't support this feature.
 *  2. This routine is used to serve the related register read/write in 
 *      32 bits length boundery only.
 */
int
drv_led_funcgrp_select_get(int unit,int port, int *led_group)
{
    uint32  reg32_val = 0, fld32_val = 0;
    
    /* check if the processing register length is not suitable in this 
     * routine to prevent improper register read/write been proceeded.
     */
    if (DRV_REG_LENGTH_GET(unit, INDEX(LED_FUNC_MAPr)) > 4){
        LOG_WARN(BSL_LS_SOC_COMMON,
                 (BSL_META_U(unit,
                             "%s, Improper driver service been proceeded\n"), 
                  FUNCTION_NAME()));
        return SOC_E_INTERNAL;
    }

    SOC_IF_ERROR_RETURN(
            REG_READ_LED_FUNC_MAPr(unit, &reg32_val));
    SOC_IF_ERROR_RETURN(soc_LED_FUNC_MAPr_field_get
        (unit, &reg32_val, LED_FUNC_MAPf, &fld32_val));
    *led_group = (fld32_val & (0x1 << port)) ? 
            DRV_LED_FUNCGRP_1 : DRV_LED_FUNCGRP_0;

    return SOC_E_NONE;
}

/*
 * Function: 
 *	    drv_led_funcgrp_select_set
 * Purpose:
 *	    Set the port based selctions of working LED function group.
 * Parameters:
 *	    port        - (IN) local port id
 *      led_group   - (IN) led function group id
 * 
 * Note:
 *	1. Most of the ROBO chips support the same LED_FUNCITON group architecture
 *      - 2 LED funciton groups.
 *      - bcm5324, bcm5396 and bcm5389 doesn't support this feature.
 */
int
drv_led_funcgrp_select_set(int unit,int port, int led_group)
{
    int     ledgrp = 0;
    uint32  reg32_val = 0, fld32_val = 0;
    
    /* check if the processing register length is not suitable in this 
     * routine to prevent improper register read/write been proceeded.
     */
    if (DRV_REG_LENGTH_GET(unit, INDEX(LED_FUNC_MAPr)) > 4){
        LOG_WARN(BSL_LS_SOC_COMMON,
                 (BSL_META_U(unit,
                             "%s, Improper driver service been proceeded\n"), 
                  FUNCTION_NAME()));
        return SOC_E_INTERNAL;
    }

    ledgrp = (led_group == DRV_LED_FUNCGRP_0) ? 0 : 
            (led_group == DRV_LED_FUNCGRP_1) ? 1 : -1;
    if (ledgrp == -1){
        return SOC_E_PARAM;
    }
    
    SOC_IF_ERROR_RETURN(
            REG_READ_LED_FUNC_MAPr(unit, &reg32_val));
    SOC_IF_ERROR_RETURN(soc_LED_FUNC_MAPr_field_get
            (unit, &reg32_val, LED_FUNC_MAPf, &fld32_val));
    if (ledgrp == 0){
        fld32_val &= ~(0x1 << port);
    } else {
        fld32_val |= 0x1 << port;
    }
    SOC_IF_ERROR_RETURN(soc_LED_FUNC_MAPr_field_set
            (unit, &reg32_val, LED_FUNC_MAPf, &fld32_val));
    SOC_IF_ERROR_RETURN(
            REG_WRITE_LED_FUNC_MAPr(unit, &reg32_val));

    return SOC_E_NONE;
}

/*
 * Function: 
 *	    drv_led_func_get
 * Purpose:
 *	    Get the combined LED functions in the specific LED function group.
 * Parameters:
 *	    led_group     - (IN) led group id
 *      led_functions - (OUT) bitmap format. 
 *                          Each bit indicated a single LED function.
 * 
 * Note:
 *	1. Most of the ROBO chips support the same LED_FUNCITON group architecture
 *      - 2 LED funciton groups.
 *      - bcm5324, bcm5396 and bcm5389 doesn't support this feature.
 */
int
drv_led_func_get(int unit,int led_group, uint32 *led_functions)
{
#ifdef BCM_POLAR_SUPPORT
    uint32 led_functions_extd = 0;
#endif /* BCM_POLAR_SUPPORT */

    *led_functions = 0;
    if (led_group == DRV_LED_FUNCGRP_0){
        SOC_IF_ERROR_RETURN(
                REG_READ_LED_FUNC0_CTLr(unit, led_functions));        
#ifdef BCM_POLAR_SUPPORT
        if (SOC_IS_POLAR(unit)) {
            SOC_IF_ERROR_RETURN(
                    REG_READ_LED_FUNC0_EXTD_CTLr(unit, &led_functions_extd));
        }
#endif /* BCM_POLAR_SUPPORT */
    } else if (led_group == DRV_LED_FUNCGRP_1){
        SOC_IF_ERROR_RETURN(
                REG_READ_LED_FUNC1_CTLr(unit, led_functions));        
#ifdef BCM_POLAR_SUPPORT
        if (SOC_IS_POLAR(unit)) {
            SOC_IF_ERROR_RETURN(
                    REG_READ_LED_FUNC1_EXTD_CTLr(unit, &led_functions_extd));
        }
#endif /* BCM_POLAR_SUPPORT */
    } else {
        return SOC_E_UNAVAIL;
    }

#ifdef BCM_POLAR_SUPPORT
    *led_functions |= (led_functions_extd << 16);
#endif /* BCM_POLAR_SUPPORT */

    return SOC_E_NONE;
}

/*
 * Function: 
 *	    drv_led_func_set
 * Purpose:
 *	    Set the combined LED functions in the specific LED function group.
 * Parameters:
 *	    led_group     - (IN) led group id
 *      led_functions - (OUT) bitmap format. 
 *                          Each bit indicated a single LED function.
 * 
 * Note:
 *	1. Most of the ROBO chips support the same LED_FUNCITON group architecture
 *      - 2 LED funciton groups.
 *      - bcm5324, bcm5396 and bcm5389 doesn't support this feature.
 */
int
drv_led_func_set(int unit,int led_group, uint32 led_functions)
{
    uint32  supported_led_functions = 0;

    /* check if the assigning led functions is out of supporting list */
    SOC_IF_ERROR_RETURN(DRV_DEV_PROP_GET(unit, 
            DRV_DEV_PROP_SUPPORTED_LED_FUNCTIONS, &supported_led_functions));
    if (led_functions & ~(supported_led_functions)){
        return SOC_E_PARAM;
    }
    
    if (led_group == DRV_LED_FUNCGRP_0){
        SOC_IF_ERROR_RETURN(REG_WRITE_LED_FUNC0_CTLr(unit, &led_functions));
#ifdef BCM_POLAR_SUPPORT
        if (SOC_IS_POLAR(unit) && 
            (led_functions & (DRV_LED_FUNC_SP_100_200 | 
                              DRV_LED_FUNC_100_200_ACT))) {
            /* 200Mbps LED programming for Polar */
            led_functions >>= 16;
            SOC_IF_ERROR_RETURN(
                    REG_WRITE_LED_FUNC0_EXTD_CTLr(unit, &led_functions));
        }
#endif /* BCM_POLAR_SUPPORT */
    } else if (led_group == DRV_LED_FUNCGRP_1){
        SOC_IF_ERROR_RETURN(REG_WRITE_LED_FUNC1_CTLr(unit, &led_functions));
#ifdef BCM_POLAR_SUPPORT
        if (SOC_IS_POLAR(unit) && 
            (led_functions & (DRV_LED_FUNC_SP_100_200 | 
                              DRV_LED_FUNC_100_200_ACT))) {
            /* 200Mbps LED programming for Polar */
            led_functions >>= 16;
            SOC_IF_ERROR_RETURN(
                    REG_WRITE_LED_FUNC1_EXTD_CTLr(unit, &led_functions));
        }
#endif /* BCM_POLAR_SUPPORT */
    } else {
        return SOC_E_UNAVAIL;
    }
    
    return SOC_E_NONE;
}

