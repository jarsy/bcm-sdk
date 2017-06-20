/*
 * $Id: port.c,v 1.3 Broadcom SDK $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
 
#include <shared/bsl.h>

#include <soc/robo.h>
#include <soc/drv.h>
#include <soc/debug.h>
#include "../robo_gex.h"

/*
 *  Function : drv_blackbird2_port_pri_mapop_set
 *
 *  Purpose :
 *      Port basis priority mapping operation configuration set
 *
 *  Parameters :
 *      unit        :   unit id
 *      port        :   port id.
 *      op_type     :   operation type
 *      pri_old     :   old priority.
 *      pri_new     :   new priority.
 *      cfi_new     :   new cfi.
 *
 *  Return :
 *      SOC_E_XXX
 *
 *  Note :
 *  1. This driver service is designed for priority operation exchange.
 *  2. Priority type could be dot1p, DSCP, port based.
 *
 */
int 
drv_blackbird2_port_pri_mapop_set(int unit, int port, int op_type, 
                uint32 pri_old, uint32 cfi_old, uint32 pri_new, uint32 cfi_new)
{
    uint32  temp;
    uint64  reg_value64;

    switch (op_type) {
        case DRV_PORT_OP_PCP2TC :
            temp = pri_new & DOT1P_PRI_MASK;
            SOC_IF_ERROR_RETURN(DRV_QUEUE_PRIO_REMAP_SET
                (unit, port, pri_old, (uint8)temp));
            break;
        case DRV_PORT_OP_NORMAL_TC2PCP:
            SOC_IF_ERROR_RETURN(
                REG_READ_EGRESS_NAVB_PKT_TC2PCP_MAPr(
                unit, port, (uint32 *)&reg_value64));

            if (cfi_new != 0) {
                /* BCM53128 didn't support CFI remarking */
                return SOC_E_UNAVAIL;
            }
            temp = ((cfi_new & DOT1P_CFI_MASK) << 3) | 
                    (pri_new & DOT1P_PRI_MASK);
            
            /* assigning the field-id */
            switch(pri_old) {
                case 0:
                    SOC_IF_ERROR_RETURN(
                        soc_EGRESS_NAVB_PKT_TC2PCP_MAPr_field_set
                        (unit, (uint32 *)&reg_value64, PCP_FOR_RV0_TC0f, &temp));
                    break;
                case 1:
                    SOC_IF_ERROR_RETURN(
                        soc_EGRESS_NAVB_PKT_TC2PCP_MAPr_field_set
                        (unit, (uint32 *)&reg_value64, PCP_FOR_RV0_TC1f, &temp));
                    break;
                case 2:
                    SOC_IF_ERROR_RETURN(
                        soc_EGRESS_NAVB_PKT_TC2PCP_MAPr_field_set
                        (unit, (uint32 *)&reg_value64, PCP_FOR_RV0_TC2f, &temp));
                    break;
                case 3:
                    SOC_IF_ERROR_RETURN(
                        soc_EGRESS_NAVB_PKT_TC2PCP_MAPr_field_set
                        (unit, (uint32 *)&reg_value64, PCP_FOR_RV0_TC3f, &temp));
                    break;
                case 4:
                    SOC_IF_ERROR_RETURN(
                        soc_EGRESS_NAVB_PKT_TC2PCP_MAPr_field_set
                        (unit, (uint32 *)&reg_value64, PCP_FOR_RV0_TC4f, &temp));
                    break;
                case 5:
                    SOC_IF_ERROR_RETURN(
                        soc_EGRESS_NAVB_PKT_TC2PCP_MAPr_field_set
                        (unit, (uint32 *)&reg_value64, PCP_FOR_RV0_TC5f, &temp));
                    break;
                case 6:
                    SOC_IF_ERROR_RETURN(
                        soc_EGRESS_NAVB_PKT_TC2PCP_MAPr_field_set
                        (unit, (uint32 *)&reg_value64, PCP_FOR_RV0_TC6f, &temp));
                    break;
                case 7:
                    SOC_IF_ERROR_RETURN(
                        soc_EGRESS_NAVB_PKT_TC2PCP_MAPr_field_set
                        (unit, (uint32 *)&reg_value64, PCP_FOR_RV0_TC7f, &temp));
                    break;
                default :
                    return SOC_E_PARAM;
                    break;
            }
                
            SOC_IF_ERROR_RETURN(
                REG_WRITE_EGRESS_NAVB_PKT_TC2PCP_MAPr(
                unit, port, (uint32 *)&reg_value64));
            break;
        case DRV_PORT_OP_OUTBAND_TC2PCP:
            return SOC_E_UNAVAIL;
        default:
            break;
    }
    
    return SOC_E_NONE;
}

/*
 *  Function : drv_blackbird2_port_pri_mapop_get
 *
 *  Purpose :
 *      Port basis priority mapping operation configuration get
 *
 *  Parameters :
 *      unit        :   unit id
 *      port        :   port id.
 *      pri_old     :   (in)old priority.
 *      cfi_old     :   (in)old cfi (No used).
 *      pri_new     :   (out)new priority.
 *      cfi_new     :   (out)new cfi.
 *
 *  Return :
 *      SOC_E_XXX
 *
 *  Note :
 *  1. This driver service is designed for priority operation exchange.
 *  2. Priority type could be dot1p, DSCP, port based.
 *
 */
int 
drv_blackbird2_port_pri_mapop_get(int unit, int port, int op_type, 
                uint32 pri_old, uint32 cfi_old, uint32 *pri_new, uint32 *cfi_new)
{
    uint32  temp;
    uint64  reg_value64;
    uint8  temp8 = 0;

    switch (op_type) {
        case DRV_PORT_OP_PCP2TC :
            temp8 = 0;
            SOC_IF_ERROR_RETURN(DRV_QUEUE_PRIO_REMAP_GET
                (unit, port, pri_old, &temp8));
            temp = (uint32)temp8;
            *pri_new = temp;
            break;
        case DRV_PORT_OP_NORMAL_TC2PCP:
            SOC_IF_ERROR_RETURN(
                REG_READ_EGRESS_NAVB_PKT_TC2PCP_MAPr(
                unit, port, (uint32 *)&reg_value64));
            
            /* assigning the field-id */
            switch(pri_old) {
                case 0:
                    SOC_IF_ERROR_RETURN(
                        soc_EGRESS_NAVB_PKT_TC2PCP_MAPr_field_get
                        (unit, (uint32 *)&reg_value64, PCP_FOR_RV0_TC0f, &temp));
                    break;
                case 1:
                    SOC_IF_ERROR_RETURN(
                        soc_EGRESS_NAVB_PKT_TC2PCP_MAPr_field_get
                        (unit, (uint32 *)&reg_value64, PCP_FOR_RV0_TC1f, &temp));
                    break;
                case 2:
                    SOC_IF_ERROR_RETURN(
                        soc_EGRESS_NAVB_PKT_TC2PCP_MAPr_field_get
                        (unit, (uint32 *)&reg_value64, PCP_FOR_RV0_TC2f, &temp));
                    break;
                case 3:
                    SOC_IF_ERROR_RETURN(
                        soc_EGRESS_NAVB_PKT_TC2PCP_MAPr_field_get
                        (unit, (uint32 *)&reg_value64, PCP_FOR_RV0_TC3f, &temp));
                    break;
                case 4:
                    SOC_IF_ERROR_RETURN(
                        soc_EGRESS_NAVB_PKT_TC2PCP_MAPr_field_get
                        (unit, (uint32 *)&reg_value64, PCP_FOR_RV0_TC4f, &temp));
                    break;
                case 5:
                    SOC_IF_ERROR_RETURN(
                        soc_EGRESS_NAVB_PKT_TC2PCP_MAPr_field_get
                        (unit, (uint32 *)&reg_value64, PCP_FOR_RV0_TC5f, &temp));
                    break;
                case 6:
                    SOC_IF_ERROR_RETURN(
                        soc_EGRESS_NAVB_PKT_TC2PCP_MAPr_field_get
                        (unit, (uint32 *)&reg_value64, PCP_FOR_RV0_TC6f, &temp));
                    break;
                case 7:
                    SOC_IF_ERROR_RETURN(
                        soc_EGRESS_NAVB_PKT_TC2PCP_MAPr_field_get
                        (unit, (uint32 *)&reg_value64, PCP_FOR_RV0_TC7f, &temp));
                    break;
                default :
                    return SOC_E_PARAM;
                    break;
            }
                
            *pri_new = temp & DOT1P_PRI_MASK;
            *cfi_new = (temp >> 3) & DOT1P_CFI_MASK;
            break;
        case DRV_PORT_OP_OUTBAND_TC2PCP:
            return SOC_E_UNAVAIL;
        default:
            break;
    }

    return SOC_E_NONE;
}

int 
drv_blackbird2_port_cross_connect_set(int unit,uint32 port,soc_pbmp_t bmp)
{
    int rv = SOC_E_NONE;
    uint32 reg_value = 0;
    uint32 temp = 0;
    uint32  field_val = 0;
    
    LOG_INFO(BSL_LS_SOC_PORT,
             (BSL_META_U(unit,
                         "drv_blackbird2_port_cross_connect_set: unit=%d port = %d, bmp=%x\n"),
              unit, port, SOC_PBMP_WORD_GET(bmp, 0)));
    
    SOC_IF_ERROR_RETURN(
        REG_READ_PORT_CROSS_CONNECTr(unit, port, &reg_value));

    temp = SOC_PBMP_WORD_GET(bmp, 0);
    soc_PORT_CROSS_CONNECTr_field_set(unit, &reg_value, 
                PCC_DESTf, &temp);

    if (SOC_PBMP_IS_NULL(bmp)) {
        temp = 0;
    } else {
        temp = 1;
    }
    /* Configure the port cross connect enable bit */
    soc_PORT_CROSS_CONNECTr_field_set(unit, &reg_value, 
                PCC_ENABLEf, &temp);

    SOC_IF_ERROR_RETURN(
        REG_WRITE_PORT_CROSS_CONNECTr(unit, port, &reg_value));

    /* Configure the preserve ingress packets format */
    SOC_IF_ERROR_RETURN(
        REG_READ_PRESERVE_PKT_FORMATr(unit, &reg_value));

    soc_PRESERVE_PKT_FORMATr_field_get(unit, &reg_value, 
                PRESERVE_PACKET_FORMATf, &field_val);
    if (temp) {
        field_val |= (0x1 << port);
    } else {
        field_val &= ~(0x1 << port);
    }
    soc_PRESERVE_PKT_FORMATr_field_set(unit, &reg_value, 
                PRESERVE_PACKET_FORMATf, &temp);

    SOC_IF_ERROR_RETURN(
        REG_WRITE_PRESERVE_PKT_FORMATr(unit, &reg_value));

    return rv;
}

int 
drv_blackbird2_port_cross_connect_get(int unit,uint32 port,soc_pbmp_t *bmp)
{
    int rv = SOC_E_NONE;
    uint32 reg_value = 0;
    uint32 temp = 0;
    
    LOG_INFO(BSL_LS_SOC_PORT,
             (BSL_META_U(unit,
                         "drv_blackbird2_port_cross_connect_get: unit=%d port = %d\n"),
              unit, port));

    SOC_IF_ERROR_RETURN(
        REG_READ_PORT_CROSS_CONNECTr(unit, port, &reg_value));

    soc_PORT_CROSS_CONNECTr_field_get(unit, &reg_value, 
                PCC_DESTf, &temp);
    
    SOC_PBMP_WORD_SET(*bmp, 0 , temp);
    
    return rv;
}

