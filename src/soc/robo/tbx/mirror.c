/*
 * $Id: mirror.c,v 1.1 Broadcom SDK $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include <shared/bsl.h>

#include <soc/robo.h>
#include <soc/drv.h>
#include <soc/debug.h>

/*
 *  Function : drv_tbx_mirror_set
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
drv_tbx_mirror_set(int unit, uint32 enable, soc_pbmp_t mport_bmp, 
    soc_pbmp_t ingress_bmp, soc_pbmp_t egress_bmp)
{
    uint32  reg_value, temp;

    LOG_INFO(BSL_LS_SOC_PORT, \
             (BSL_META_U(unit, \
                         "drv_tbx_mirror_set: unit %d, \
                         %sable, mport_bmp = 0x%x, ingress_bmp = 0x%x, egress_bmp = 0x%x\n"),
              unit, (enable) ? "en" : "dis", \
              SOC_PBMP_WORD_GET(mport_bmp, 0), \
              SOC_PBMP_WORD_GET(ingress_bmp, 0), \
              SOC_PBMP_WORD_GET(egress_bmp, 0)));

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
    SOC_IF_ERROR_RETURN(REG_READ_MIRRORCTLr
        (unit, &reg_value));

    /* Enable/Disable mirror */
    if (enable) {
        temp = 1;
    } else {
        temp = 0;

        /* Clear mirror-to ports mask when disable mirror */
        SOC_PBMP_CLEAR(mport_bmp);
    }
    /* Enable/disable both ingress and egress mirroring */
    SOC_IF_ERROR_RETURN(soc_MIRRORCTLr_field_set
        (unit, &reg_value, EN_IN_MIR_FLTRf, &temp));
    SOC_IF_ERROR_RETURN(soc_MIRRORCTLr_field_set
        (unit, &reg_value, EN_OUT_MIR_FLTRf, &temp));
    SOC_IF_ERROR_RETURN(REG_WRITE_MIRRORCTLr
        (unit, &reg_value));

    /* Write mirror-to ports mask */
    SOC_IF_ERROR_RETURN(REG_READ_MIRCAPCTLr
        (unit, &reg_value));
    temp = SOC_PBMP_WORD_GET(mport_bmp, 0);
    SOC_IF_ERROR_RETURN(soc_MIRCAPCTLr_field_set
        (unit, &reg_value, MIR_CAP_PORTf, &temp));
    SOC_IF_ERROR_RETURN(REG_WRITE_MIRCAPCTLr
        (unit, &reg_value));

    return SOC_E_NONE;
}

/*
 *  Function : drv_tbx_mirror_get
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
drv_tbx_mirror_get(int unit, uint32 *enable, soc_pbmp_t *mport_bmp, 
    soc_pbmp_t *ingress_bmp, soc_pbmp_t *egress_bmp)
{
    uint32  reg_value, temp;

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
    SOC_IF_ERROR_RETURN(REG_READ_MIRRORCTLr
        (unit, &reg_value));
    temp = 0;
    SOC_IF_ERROR_RETURN(soc_MIRRORCTLr_field_get
        (unit, &reg_value, EN_IN_MIR_FLTRf, &temp));
    if (!temp) {
        SOC_IF_ERROR_RETURN(soc_MIRRORCTLr_field_get
            (unit, &reg_value, EN_OUT_MIR_FLTRf, &temp));
    }
    /* Enable/Disable mirror */
    if (temp) {
        *enable = TRUE;
    } else {
        *enable = FALSE;
    }

    /* Monitor port */
    SOC_PBMP_CLEAR(*mport_bmp);
    SOC_IF_ERROR_RETURN(REG_READ_MIRCAPCTLr
        (unit, &reg_value));
    SOC_IF_ERROR_RETURN(soc_MIRCAPCTLr_field_get
        (unit, &reg_value, MIR_CAP_PORTf, &temp));
    SOC_PBMP_WORD_SET(*mport_bmp, 0, temp);

    LOG_INFO(BSL_LS_SOC_PORT, \
             (BSL_META_U(unit, \
                         "drv_tbx_mirror_get : unit %d, \
                         %sable, mport_bmp = 0x%x, ingress_bmp = 0x%x, egress_bmp = 0x%x\n"),
              unit, (*enable) ? "en" : "dis", \
              SOC_PBMP_WORD_GET(*mport_bmp, 0), \
              SOC_PBMP_WORD_GET(*ingress_bmp, 0), \
              SOC_PBMP_WORD_GET(*egress_bmp, 0)));

    return SOC_E_NONE;
}
