/*
 * $Id: $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include <shared/bsl.h>

#include <soc/drv.h>
#include <soc/debug.h>

/*
 *  Function : drv_gex_mirror_set
 *
 *  Purpose :
 *      Set ingress and egress ports of mirroring.
 *
 *  Parameters :
 *      unit        :  unit id.
 *      enable      :  enable/disable.
 *      mport_bmp   :  monitor port bitmap.
 *      ingress_bmp :  ingress port bitmap.
 *      egress_bmp  :  egress port bitmap.
 *
 *  Return :
 *      SOC_E_XXX
 *
 *  Note :
 */
int 
drv_gex_mirror_set(int unit, uint32 enable, soc_pbmp_t mport_bmp, 
    soc_pbmp_t ingress_bmp, soc_pbmp_t egress_bmp)
{
    uint32  reg_value, temp, mport = 0;
    int  count = 0;

    LOG_INFO(BSL_LS_SOC_PORT, \
             (BSL_META_U(unit, \
                         "drv_gex_mirror_set: unit %d, \
                         %sable, mport_bmp = 0x%x, ingress_bmp = 0x%x, egress_bmp = 0x%x\n"),
              unit, (enable) ? "en" : "dis", \
              SOC_PBMP_WORD_GET(mport_bmp, 0), \
              SOC_PBMP_WORD_GET(ingress_bmp, 0), \
              SOC_PBMP_WORD_GET(egress_bmp, 0)));

    /* Check mirror capture port */
    /* No support multiple mirror-to ports (only one mirror-to port) */
    SOC_PBMP_COUNT(mport_bmp, count);
    if (count >= 2) {
        return SOC_E_UNAVAIL;
    }

    /* Check ingress mirror */
    SOC_IF_ERROR_RETURN(REG_READ_IGMIRCTLr
        (unit, &reg_value));
    /* Write ingress mirror mask */
    temp = SOC_PBMP_WORD_GET(ingress_bmp, 0);
    SOC_IF_ERROR_RETURN(soc_IGMIRCTLr_field_set
        (unit, &reg_value, IN_MIR_MSKf, &temp));
    SOC_IF_ERROR_RETURN(REG_WRITE_IGMIRCTLr
        (unit, &reg_value));

    /* Check egress mirror */
    SOC_IF_ERROR_RETURN(REG_READ_EGMIRCTLr
        (unit, &reg_value));
    /* Write egress mirror mask */
    temp = SOC_PBMP_WORD_GET(egress_bmp, 0);
    SOC_IF_ERROR_RETURN(soc_EGMIRCTLr_field_set
        (unit, &reg_value, OUT_MIR_MSKf, &temp));
    SOC_IF_ERROR_RETURN(REG_WRITE_EGMIRCTLr
        (unit, &reg_value));

    /* Check mirror control */
    SOC_IF_ERROR_RETURN(REG_READ_MIRCAPCTLr
        (unit, &reg_value));

    /* Enable/Disable mirror */
    if (enable) {
        temp = 1;
    } else {
        temp = 0;

        /* Clear mirror-to ports mask when disable mirror */
        SOC_PBMP_CLEAR(mport_bmp);
    }
    SOC_IF_ERROR_RETURN(soc_MIRCAPCTLr_field_set
        (unit, &reg_value, MIR_ENf, &temp));

    /* Write mirror-to ports mask */
    if (SOC_PBMP_IS_NULL(mport_bmp)) {
        mport = 0;
    } else {
        PBMP_ITER(mport_bmp, mport) {
            break;
        }
    }
    temp = mport;
    SOC_IF_ERROR_RETURN(soc_MIRCAPCTLr_field_set
        (unit, &reg_value, SMIR_CAP_PORTf, &temp));

    SOC_IF_ERROR_RETURN(REG_WRITE_MIRCAPCTLr
        (unit, &reg_value));

    return SOC_E_NONE;
}

/*
 *  Function : drv_gex_mirror_get
 *
 *  Purpose :
 *      Get ingress and egress ports of mirroring.
 *
 *  Parameters :
 *      unit        :  unit id.
 *      enable      :  enable/disable.
 *      mport_bmp   :  monitor port bitmap.
 *      ingress_bmp :  ingress port bitmap.
 *      egress_bmp  :  egress port bitmap.
 *
 *  Return :
 *      SOC_E_XXX
 *
 *  Note :
 */
int 
drv_gex_mirror_get(int unit, uint32 *enable, soc_pbmp_t *mport_bmp, 
    soc_pbmp_t *ingress_bmp, soc_pbmp_t *egress_bmp)
{
    uint32  reg_value, temp = 0;

    /* Ingress mask */
    SOC_IF_ERROR_RETURN(REG_READ_IGMIRCTLr
        (unit, &reg_value));
    SOC_IF_ERROR_RETURN(soc_IGMIRCTLr_field_get
        (unit, &reg_value, IN_MIR_MSKf, &temp));
    SOC_PBMP_WORD_SET(*ingress_bmp, 0, temp);

    /* Egress mask */
    SOC_IF_ERROR_RETURN(REG_READ_EGMIRCTLr
        (unit, &reg_value));
    SOC_IF_ERROR_RETURN(soc_EGMIRCTLr_field_get
        (unit, &reg_value, OUT_MIR_MSKf, &temp));
    SOC_PBMP_WORD_SET(*egress_bmp, 0, temp);

    /* Enable value */
    SOC_IF_ERROR_RETURN(REG_READ_MIRCAPCTLr
        (unit, &reg_value));
    temp = 0;
    SOC_IF_ERROR_RETURN(soc_MIRCAPCTLr_field_get
        (unit, &reg_value, MIR_ENf, &temp));
    /* Enable/Disable mirror */
    if (temp) {
        *enable = TRUE;
    } else {
        *enable = FALSE;
    }

    /* Monitor port */
    SOC_PBMP_CLEAR(*mport_bmp);
    SOC_IF_ERROR_RETURN(soc_MIRCAPCTLr_field_get
        (unit, &reg_value, SMIR_CAP_PORTf, &temp));

    if (*enable) {
        SOC_PBMP_PORT_SET(*mport_bmp, temp);
    } else {
        /* When mirror is disabled, temp = 0 return mport_bmp = zero */
        if (temp == 0) {
            SOC_PBMP_CLEAR(*mport_bmp);
        } else {
            SOC_PBMP_PORT_SET(*mport_bmp, temp);
        }
    }

    LOG_INFO(BSL_LS_SOC_PORT, \
             (BSL_META_U(unit, \
                         "drv_gex_mirror_get: unit %d, \
                         %sable, mport_bmp = 0x%x, ingress_bmp = 0x%x, egress_bmp = 0x%x\n"),
              unit, (*enable) ? "en" : "dis", \
              SOC_PBMP_WORD_GET(*mport_bmp, 0), \
              SOC_PBMP_WORD_GET(*ingress_bmp, 0), \
              SOC_PBMP_WORD_GET(*egress_bmp, 0)));

    return SOC_E_NONE;
}
