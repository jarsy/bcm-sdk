/*
 * $Id: age.c,v 1.5 Broadcom SDK $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */

#include <shared/bsl.h>

#include <soc/drv.h>
#include <soc/debug.h>

/*
 *  Function : drv_age_timer_set
 *
 *  Purpose :
 *      Set the age timer status and value.
 *
 *  Parameters :
 *      unit        :   RoboSwitch unit number.
 *      enable   :   enable status.
 *      age_time  :   age timer value.
 *
 *  Return :
 *      SOC_E_XXX.
 *
 *  Note :
 *      
 *
 */
int 
drv_age_timer_set(int unit, uint32 enable, uint32 age_time)
{
    uint32  reg_value, temp;

    LOG_VERBOSE(BSL_LS_SOC_COMMON,
                (BSL_META_U(unit,
                            "drv_age_timer_set: unit %d, %sable, age_time = %d\n"),
                 unit, enable ? "en" : "dis", age_time));
    SOC_IF_ERROR_RETURN(REG_READ_SPTAGTr(unit, &reg_value));
    if (enable) {
        temp = age_time;
        /* 
         * Since the time unit of BCM53101 is half of second,
         * we need to translate the value of age time 
         */
        if (SOC_IS_LOTUS(unit)) {
            temp = temp * 2;
        }
    } else {
        temp = 0;
    }
    soc_SPTAGTr_field_set(unit, &reg_value, AGE_TIMEf, &temp);
    SOC_IF_ERROR_RETURN(REG_WRITE_SPTAGTr(unit, &reg_value));

    return SOC_E_NONE;
}

/*
 *  Function : drv_age_timer_get
 *
 *  Purpose :
 *      Get the age timer status and value.
 *
 *  Parameters :
 *      unit        :   RoboSwitch unit number.
 *      enable   :   enable status.
 *      age_time  :   age timer value.
 *
 *  Return :
 *      SOC_E_XXX.
 *
 *  Note :
 *      
 *
 */
int 
drv_age_timer_get(int unit, uint32 *enable, uint32 *age_time)
{
    uint32	reg_value = 0, temp = 0;

    SOC_IF_ERROR_RETURN(REG_READ_SPTAGTr(unit, &reg_value));
    soc_SPTAGTr_field_get(unit, &reg_value, AGE_TIMEf, &temp);
    
    if (temp) {
        *enable = TRUE;
        *age_time = temp;
        /* 
         * Since the time unit of BCM53101 is half of second,
         * we need to translate the value of age time 
         */
        if (SOC_IS_LOTUS(unit)) {
            *age_time = (temp / 2);
        }
    } else {
        *enable = FALSE;
        *age_time = 0;
    }

    LOG_VERBOSE(BSL_LS_SOC_COMMON,
                (BSL_META_U(unit,
                            "drv_age_timer_get: unit %d, %sable, age_time = %d\n"),
                 unit, *enable ? "en" : "dis", *age_time));

    return SOC_E_NONE;
}
