/*
 * $Id: mirror.c,v 1.1 Broadcom SDK $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include <shared/bsl.h>

#include <soc/drv.h>
#include <soc/debug.h>

/*
 *  Function : drv_harrier_mirror_set
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
drv_harrier_mirror_set(int unit, uint32 enable, soc_pbmp_t mport_bmp, 
    soc_pbmp_t ingress_bmp, soc_pbmp_t egress_bmp)
{
    uint64  reg_value;
    uint32  temp32;

    LOG_INFO(BSL_LS_SOC_PORT, \
             (BSL_META_U(unit, \
                         "drv_harrier_mirror_set: unit %d, \
                         %sable, mport_bmp = 0x%x, ingress_bmp = 0x%x, egress_bmp = 0x%x\n"),
              unit, (enable) ? "en" : "dis", \
              SOC_PBMP_WORD_GET(mport_bmp, 0), \
              SOC_PBMP_WORD_GET(ingress_bmp, 0), \
              SOC_PBMP_WORD_GET(egress_bmp, 0)));

    /* Check ingress mirror */
    SOC_IF_ERROR_RETURN(REG_READ_IGMIRCTLr
       (unit, (uint32 *)&reg_value));
    /* Write ingress mirror mask */
    temp32 = SOC_PBMP_WORD_GET(ingress_bmp, 0);
    SOC_IF_ERROR_RETURN(soc_IGMIRCTLr_field_set
        (unit, (uint32 *)&reg_value, IN_MIR_MSKf, &temp32));
    SOC_IF_ERROR_RETURN(REG_WRITE_IGMIRCTLr
        (unit, (uint32 *)&reg_value));

    /* Check egress mirror */
    SOC_IF_ERROR_RETURN(REG_READ_EGMIRCTLr
        (unit, (uint32 *)&reg_value));
    /* Write egress mirror mask */
    temp32 = SOC_PBMP_WORD_GET(egress_bmp, 0);
    SOC_IF_ERROR_RETURN(soc_EGMIRCTLr_field_set
        (unit, (uint32 *)&reg_value, OUT_MIR_MSKf, &temp32));
    SOC_IF_ERROR_RETURN(REG_WRITE_EGMIRCTLr
        (unit, (uint32 *)&reg_value));

    /* Check mirror control */
    SOC_IF_ERROR_RETURN(REG_READ_MIRCAPCTLr
        (unit, (uint32 *)&reg_value));

    /* Enable/Disable mirror */
    if (enable) {
        temp32 = 1;
    } else {
        temp32 = 0;

        /* Clear mirror-to ports mask when disable mirror */
        SOC_PBMP_CLEAR(mport_bmp);
    }
    SOC_IF_ERROR_RETURN(soc_MIRCAPCTLr_field_set
        (unit, (uint32 *)&reg_value, MIR_ENf, &temp32));

    /* Write mirror-to ports mask */
    temp32 = SOC_PBMP_WORD_GET(mport_bmp, 0);
    SOC_IF_ERROR_RETURN(soc_MIRCAPCTLr_field_set
        (unit, (uint32 *)&reg_value, SMIR_CAP_PORTf, &temp32));

    SOC_IF_ERROR_RETURN(REG_WRITE_MIRCAPCTLr
        (unit, (uint32 *)&reg_value));

    return SOC_E_NONE;
}

/*
 *  Function : drv_harrier_mirror_get
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
drv_harrier_mirror_get(int unit, uint32 *enable, soc_pbmp_t *mport_bmp, 
    soc_pbmp_t *ingress_bmp, soc_pbmp_t *egress_bmp)
{
    uint64  reg_value;
    uint32  temp32 = 0;

    /* Ingress mask */
    SOC_IF_ERROR_RETURN(REG_READ_IGMIRCTLr
        (unit, (uint32 *)&reg_value));
    SOC_IF_ERROR_RETURN(soc_IGMIRCTLr_field_get
        (unit, (uint32 *)&reg_value, IN_MIR_MSKf, &temp32));
    SOC_PBMP_WORD_SET(*ingress_bmp, 0, temp32);

    /* Egress mask */
    SOC_IF_ERROR_RETURN(REG_READ_EGMIRCTLr
        (unit, (uint32 *)&reg_value));
    SOC_IF_ERROR_RETURN(soc_EGMIRCTLr_field_get
        (unit, (uint32 *)&reg_value, OUT_MIR_MSKf, &temp32));
    SOC_PBMP_WORD_SET(*egress_bmp, 0, temp32);

    /* Enable value */
    SOC_IF_ERROR_RETURN(REG_READ_MIRCAPCTLr
        (unit, (uint32 *)&reg_value));
    temp32 = 0;
    SOC_IF_ERROR_RETURN(soc_MIRCAPCTLr_field_get
        (unit, (uint32 *)&reg_value, MIR_ENf, &temp32));
    /* Enable/Disable mirror */
    if (temp32) {
        *enable = TRUE;
    } else {
        *enable = FALSE;
    }

    /* Monitor port */
    SOC_PBMP_CLEAR(*mport_bmp);
    SOC_IF_ERROR_RETURN(soc_MIRCAPCTLr_field_get
        (unit, (uint32 *)&reg_value, SMIR_CAP_PORTf, &temp32));
    SOC_PBMP_WORD_SET(*mport_bmp, 0, temp32);

    LOG_INFO(BSL_LS_SOC_PORT, \
             (BSL_META_U(unit, \
                         "drv_harrier_mirror_get : unit %d, \
                         %sable, mport_bmp = 0x%x, ingress_bmp = 0x%x, egress_bmp = 0x%x\n"),
              unit, (*enable) ? "en" : "dis", \
              SOC_PBMP_WORD_GET(*mport_bmp, 0), \
              SOC_PBMP_WORD_GET(*ingress_bmp, 0), \
              SOC_PBMP_WORD_GET(*egress_bmp, 0)));

    return SOC_E_NONE;
}
