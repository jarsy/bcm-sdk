/*
 * $Id: arl.c,v 1.10 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:	arl.c
 */
#include <soc/robo/mcm/driver.h>



STATIC int
_drv_gex_hw_learning_set(int unit, soc_pbmp_t pbmp, uint32 value)
{
    int     rv = SOC_E_NONE;
    uint32  reg_v32, fld_v32 = 0;
    soc_pbmp_t current_pbmp, temp_pbmp;

    if ((rv = REG_READ_DIS_LEARNr(unit, &reg_v32)) < 0) {
        return rv;
    }

    soc_DIS_LEARNr_field_get(unit, &reg_v32,
        DIS_LEARNf, &fld_v32);
    SOC_PBMP_WORD_SET(current_pbmp, 0, fld_v32);

    if (value) { /* enable */
        SOC_PBMP_CLEAR(temp_pbmp);
        SOC_PBMP_NEGATE(temp_pbmp, pbmp);
        SOC_PBMP_AND(current_pbmp, temp_pbmp);
    } else { /* disable */
        SOC_PBMP_OR(current_pbmp, pbmp);
    }

    fld_v32 = SOC_PBMP_WORD_GET(current_pbmp, 0);
    soc_DIS_LEARNr_field_set(unit, &reg_v32,
        DIS_LEARNf, &fld_v32);

    if ((rv = REG_WRITE_DIS_LEARNr(unit, &reg_v32)) < 0) {
        return rv;
    }

    return rv;
}

STATIC int
_drv_gex_hw_learning_get(int unit, soc_port_t port, uint32 *value)
{
    int     rv = SOC_E_NONE;
    uint32  reg_v32, fld_v32 = 0;
    soc_pbmp_t current_pbmp;

    if ((rv = REG_READ_DIS_LEARNr(unit, &reg_v32)) < 0) {
        return rv;
    }

    soc_DIS_LEARNr_field_get(unit, &reg_v32,
        DIS_LEARNf, &fld_v32);
    SOC_PBMP_WORD_SET(current_pbmp, 0, fld_v32);

    if (SOC_PBMP_MEMBER(current_pbmp, port)) {
        *value = FALSE; /* This port is in DISABLE SA learn state */
    } else {
        *value = TRUE;
    }

    return rv;
}


STATIC int
_drv_gex_sw_learning_set(int unit, soc_pbmp_t pbmp, uint32 value)
{
    int     rv = SOC_E_NONE;
    uint32  reg_v32, fld_v32 = 0;
    soc_pbmp_t current_pbmp, temp_pbmp;
    
    if ((rv = REG_READ_SFT_LRN_CTLr(unit, &reg_v32)) < 0) {
        return rv;
    }

    soc_SFT_LRN_CTLr_field_get(unit, &reg_v32, 
        SW_LEARN_CNTLf, &fld_v32);
    SOC_PBMP_WORD_SET(current_pbmp, 0, fld_v32);

    if (value) { /* enable */
        SOC_PBMP_OR(current_pbmp, pbmp);
    } else { /* disable */
        SOC_PBMP_CLEAR(temp_pbmp);
        SOC_PBMP_NEGATE(temp_pbmp, pbmp);
        SOC_PBMP_AND(current_pbmp, temp_pbmp);
    }

    fld_v32 = SOC_PBMP_WORD_GET(current_pbmp, 0);
    soc_SFT_LRN_CTLr_field_set(unit, &reg_v32, 
        SW_LEARN_CNTLf, &fld_v32);

    if ((rv = REG_WRITE_SFT_LRN_CTLr(unit, &reg_v32)) < 0) {
        return rv;
    }

    return rv;
}

STATIC int
_drv_gex_sw_learning_get(int unit, soc_port_t port, uint32 *value)
{
    int     rv = SOC_E_NONE;
    uint32  reg_v32, fld_v32 = 0;
    soc_pbmp_t current_pbmp;

    if ((rv = REG_READ_SFT_LRN_CTLr(unit, &reg_v32)) < 0) {
        return rv;
    }

    soc_SFT_LRN_CTLr_field_get(unit, &reg_v32, 
        SW_LEARN_CNTLf, &fld_v32);
    SOC_PBMP_WORD_SET(current_pbmp, 0, fld_v32);

    if (SOC_PBMP_MEMBER(current_pbmp, port)) {
        *value = TRUE; /* This port is in SW learn state */
    } else {
        *value = FALSE;
    }

    return rv;
}


/*
 * Function:
 *  drv_arl_learn_enable_set
 * Purpose:
 *  Setting per port SA learning process.
 * Parameters:
 *  unit    - RoboSwitch unit #
 *  pbmp    - port bitmap
 *  mode   - DRV_PORT_HW_LEARN
 *               DRV_PORT_DISABLE_LEARN
 *               DRV_PORT_SW_LEARN
 */
int
drv_gex_arl_learn_enable_set(int unit, soc_pbmp_t pbmp, uint32 mode)
{
    int     rv = SOC_E_NONE;

    switch (mode ) {
        case DRV_PORT_HW_LEARN:
            /* Disable software learning */
            SOC_IF_ERROR_RETURN(
                _drv_gex_sw_learning_set(unit, pbmp, 0));
            /* Enable hardware learning */
            SOC_IF_ERROR_RETURN(
                _drv_gex_hw_learning_set(unit, pbmp, 1));
            break;
        case DRV_PORT_DISABLE_LEARN:
            /* Disable software learning */
            SOC_IF_ERROR_RETURN(
                _drv_gex_sw_learning_set(unit, pbmp, 0));
            /* Disable hardware learning */
            SOC_IF_ERROR_RETURN(
                _drv_gex_hw_learning_set(unit, pbmp, 0));
       	    break;
       	case DRV_PORT_SW_LEARN:
            /* Enable learning */
            SOC_IF_ERROR_RETURN(
                _drv_gex_hw_learning_set(unit, pbmp, 1));
            /* Enable software learning */
       	    SOC_IF_ERROR_RETURN(
       	        _drv_gex_sw_learning_set(unit, pbmp, 1));
       	    break;

       	/* no support section */
       	case DRV_PORT_HW_SW_LEARN:
       	case DRV_PORT_DROP:
       	case DRV_PORT_SWLRN_DROP:
       	case DRV_PORT_HWLRN_DROP:
       	case DRV_PORT_SWHWLRN_DROP:
            rv = SOC_E_UNAVAIL;
            break;
        default:
            rv = SOC_E_PARAM;
    }

    return rv;
}

/*
 * Function:
 *  drv_arl_learn_enable_get
 * Purpose:
 *  Setting per port SA learning process.
 * Parameters:
 *  unit    - RoboSwitch unit #
 *  port    - port
 *  mode   - Port learn mode
 */
int
drv_gex_arl_learn_enable_get(int unit, soc_port_t port, uint32 *mode)
{
    int     rv = SOC_E_NONE;
    uint32  temp;

    /* Check software learn setting */
    SOC_IF_ERROR_RETURN(
        _drv_gex_sw_learning_get(unit, port, &temp));
    
    if (temp) {
        *mode = DRV_PORT_SW_LEARN;
    } else {
        /* Check hardware learn setting */
        SOC_IF_ERROR_RETURN(
            _drv_gex_hw_learning_get(unit, port, &temp));
        if (temp) {
            *mode = DRV_PORT_HW_LEARN;
        } else {
            *mode = DRV_PORT_DISABLE_LEARN;
        }
    }
    
    return rv;
}

