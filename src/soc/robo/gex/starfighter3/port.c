/*
 * $Id: port.c,v 1.4 Broadcom SDK $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */

#include <soc/robo.h>
#include <soc/drv.h>
#include <soc/debug.h>
#include "../robo_gex.h"

#define _SF3_TC2COLOR_TABLE_OP_READ     (0x0)
#define _SF3_TC2COLOR_TABLE_OP_WRITE    (0x1)

STATIC int
_drv_starfighter3_port_pri_color_mapping(int unit, int op_mode, int port, 
        uint32 tc, uint32 dei, uint32 *color)
{
    uint32 reg_val, fld_val;

    reg_val = 0;

    /* Port */
    fld_val = port;
    soc_TC2COLORr_field_set(unit, &reg_val, 
        TC2COLOR_MAP_ING_PORTf , &fld_val);
    
    /* TC */
    fld_val = tc;
    soc_TC2COLORr_field_set(unit, &reg_val, 
        TC2COLOR_MAP_TCf , &fld_val);

    /* DEI */
    fld_val = dei;
    soc_TC2COLORr_field_set(unit, &reg_val, 
        TC2COLOR_MAP_DEIf , &fld_val);

    if (op_mode == _SF3_TC2COLOR_TABLE_OP_READ) {
        /* Read the TC2COLOR table */
        fld_val = op_mode;
        soc_TC2COLORr_field_set(unit, &reg_val, 
            TC2COLOR_MAP_RWf , &fld_val);
        SOC_IF_ERROR_RETURN(
            REG_WRITE_TC2COLORr(unit, &reg_val));

        /* Get the value of color */
        SOC_IF_ERROR_RETURN(
            REG_READ_TC2COLORr(unit, &reg_val));
        soc_TC2COLORr_field_get(unit, &reg_val, 
            TC2COLOR_MAP_COLORf , &fld_val);
        *color = fld_val;
        
    } else {
        /* COLOR */
        fld_val = *color;
        soc_TC2COLORr_field_set(unit, &reg_val, 
            TC2COLOR_MAP_COLORf , &fld_val);

        /* Write the TC2COLOR table */
        fld_val = op_mode;
        soc_TC2COLORr_field_set(unit, &reg_val, 
            TC2COLOR_MAP_RWf , &fld_val);
        SOC_IF_ERROR_RETURN(
            REG_WRITE_TC2COLORr(unit, &reg_val));
    }

    return SOC_E_NONE;
}

/*
 *  Function : drv_starfighter3__port_pri_mapop_set
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
drv_starfighter3_port_pri_mapop_set(int unit, int port, int op_type, 
                uint32 pri_old, uint32 cfi_old, uint32 pri_new, uint32 cfi_new)
{
    uint32  temp;
    uint64  reg_value64;
    uint32  color;

    switch (op_type) {
        case DRV_PORT_OP_PCP2TC :
            temp = pri_new & DOT1P_PRI_MASK;
            /* TC */
            SOC_IF_ERROR_RETURN(DRV_QUEUE_PRIO_REMAP_SET
                (unit, port, pri_old, (uint8)temp));
            /* Color */
            color = cfi_new;
            SOC_IF_ERROR_RETURN(
                _drv_starfighter3_port_pri_color_mapping(unit, 
                _SF3_TC2COLOR_TABLE_OP_WRITE, port, pri_new, cfi_old, &color));

            break;
        case DRV_PORT_OP_NORMAL_TC2PCP:
            SOC_IF_ERROR_RETURN(
                REG_READ_PN_EGRESS_PKT_TC2PCP_MAPr(
                unit, port, (uint32 *)&reg_value64));
            
            temp = ((cfi_new & DOT1P_CFI_MASK) << 3) | 
                    (pri_new & DOT1P_PRI_MASK);
            
            /* assigning the field-id */
            switch(pri_old) {
                case 0:
                    SOC_IF_ERROR_RETURN(
                        soc_PN_EGRESS_PKT_TC2PCP_MAPr_field_set
                        (unit, (uint32 *)&reg_value64, PCP_FOR_RV0_TC0f, &temp));
                    break;
                case 1:
                    SOC_IF_ERROR_RETURN(
                        soc_PN_EGRESS_PKT_TC2PCP_MAPr_field_set
                        (unit, (uint32 *)&reg_value64, PCP_FOR_RV0_TC1f, &temp));
                    break;
                case 2:
                    SOC_IF_ERROR_RETURN(
                        soc_PN_EGRESS_PKT_TC2PCP_MAPr_field_set
                        (unit, (uint32 *)&reg_value64, PCP_FOR_RV0_TC2f, &temp));
                    break;
                case 3:
                    SOC_IF_ERROR_RETURN(
                        soc_PN_EGRESS_PKT_TC2PCP_MAPr_field_set
                        (unit, (uint32 *)&reg_value64, PCP_FOR_RV0_TC3f, &temp));
                    break;
                case 4:
                    SOC_IF_ERROR_RETURN(
                        soc_PN_EGRESS_PKT_TC2PCP_MAPr_field_set
                        (unit, (uint32 *)&reg_value64, PCP_FOR_RV0_TC4f, &temp));
                    break;
                case 5:
                    SOC_IF_ERROR_RETURN(
                        soc_PN_EGRESS_PKT_TC2PCP_MAPr_field_set
                        (unit, (uint32 *)&reg_value64, PCP_FOR_RV0_TC5f, &temp));
                    break;
                case 6:
                    SOC_IF_ERROR_RETURN(
                        soc_PN_EGRESS_PKT_TC2PCP_MAPr_field_set
                        (unit, (uint32 *)&reg_value64, PCP_FOR_RV0_TC6f, &temp));
                    break;
                case 7:
                    SOC_IF_ERROR_RETURN(
                        soc_PN_EGRESS_PKT_TC2PCP_MAPr_field_set
                        (unit, (uint32 *)&reg_value64, PCP_FOR_RV0_TC7f, &temp));
                    break;
                default :
                    return SOC_E_PARAM;
                    break;
            }
                
            SOC_IF_ERROR_RETURN(
                REG_WRITE_PN_EGRESS_PKT_TC2PCP_MAPr(
                unit, port, (uint32 *)&reg_value64));
            break;

        case DRV_PORT_OP_NORMAL_TC2CPCP:
            SOC_IF_ERROR_RETURN(
                REG_READ_PN_EGRESS_PKT_TC2CPCP_MAPr(
                unit, port, (uint32 *)&reg_value64));
            
            temp = ((cfi_new & DOT1P_CFI_MASK) << 3) | 
                    (pri_new & DOT1P_PRI_MASK);
            
            /* assigning the field-id */
            switch(pri_old) {
                case 0:
                    SOC_IF_ERROR_RETURN(
                        soc_PN_EGRESS_PKT_TC2CPCP_MAPr_field_set
                        (unit, (uint32 *)&reg_value64, CPCP_FOR_RV0_TC0f, &temp));
                    break;
                case 1:
                    SOC_IF_ERROR_RETURN(
                        soc_PN_EGRESS_PKT_TC2CPCP_MAPr_field_set
                        (unit, (uint32 *)&reg_value64, CPCP_FOR_RV0_TC1f, &temp));
                    break;
                case 2:
                    SOC_IF_ERROR_RETURN(
                        soc_PN_EGRESS_PKT_TC2CPCP_MAPr_field_set
                        (unit, (uint32 *)&reg_value64, CPCP_FOR_RV0_TC2f, &temp));
                    break;
                case 3:
                    SOC_IF_ERROR_RETURN(
                        soc_PN_EGRESS_PKT_TC2CPCP_MAPr_field_set
                        (unit, (uint32 *)&reg_value64, CPCP_FOR_RV0_TC3f, &temp));
                    break;
                case 4:
                    SOC_IF_ERROR_RETURN(
                        soc_PN_EGRESS_PKT_TC2CPCP_MAPr_field_set
                        (unit, (uint32 *)&reg_value64, CPCP_FOR_RV0_TC4f, &temp));
                    break;
                case 5:
                    SOC_IF_ERROR_RETURN(
                        soc_PN_EGRESS_PKT_TC2CPCP_MAPr_field_set
                        (unit, (uint32 *)&reg_value64, CPCP_FOR_RV0_TC5f, &temp));
                    break;
                case 6:
                    SOC_IF_ERROR_RETURN(
                        soc_PN_EGRESS_PKT_TC2CPCP_MAPr_field_set
                        (unit, (uint32 *)&reg_value64, CPCP_FOR_RV0_TC6f, &temp));
                    break;
                case 7:
                    SOC_IF_ERROR_RETURN(
                        soc_PN_EGRESS_PKT_TC2CPCP_MAPr_field_set
                        (unit, (uint32 *)&reg_value64, CPCP_FOR_RV0_TC7f, &temp));
                    break;
                default :
                    return SOC_E_PARAM;
                    break;
            }
                
            SOC_IF_ERROR_RETURN(
                REG_WRITE_PN_EGRESS_PKT_TC2CPCP_MAPr(
                unit, port, (uint32 *)&reg_value64));
            break;
        case DRV_PORT_OP_OUTBAND_TC2PCP:
            SOC_IF_ERROR_RETURN(
                REG_READ_PN_EGRESS_PKT_TC2PCP_MAPr(
                unit, port, (uint32 *)&reg_value64));

            temp = ((cfi_new & DOT1P_CFI_MASK) << 3) | 
                    (pri_new & DOT1P_PRI_MASK);
            
            /* assigning the field-id */
            switch(pri_old) {
                case 0:
                    SOC_IF_ERROR_RETURN(
                        soc_PN_EGRESS_PKT_TC2PCP_MAPr_field_set
                        (unit, (uint32 *)&reg_value64, PCP_FOR_RV1_TC0f, &temp));
                    break;
                case 1:
                       SOC_IF_ERROR_RETURN(
                        soc_PN_EGRESS_PKT_TC2PCP_MAPr_field_set
                        (unit, (uint32 *)&reg_value64, PCP_FOR_RV1_TC1f, &temp));
                    break;
                case 2:
                    SOC_IF_ERROR_RETURN(
                        soc_PN_EGRESS_PKT_TC2PCP_MAPr_field_set
                        (unit, (uint32 *)&reg_value64, PCP_FOR_RV1_TC2f, &temp));
                    break;
                case 3:
                    SOC_IF_ERROR_RETURN(
                        soc_PN_EGRESS_PKT_TC2PCP_MAPr_field_set
                        (unit, (uint32 *)&reg_value64, PCP_FOR_RV1_TC3f, &temp));
                    break;
                case 4:
                    SOC_IF_ERROR_RETURN(
                        soc_PN_EGRESS_PKT_TC2PCP_MAPr_field_set
                        (unit, (uint32 *)&reg_value64, PCP_FOR_RV1_TC4f, &temp));
                    break;
                case 5:
                    SOC_IF_ERROR_RETURN(
                        soc_PN_EGRESS_PKT_TC2PCP_MAPr_field_set
                        (unit, (uint32 *)&reg_value64, PCP_FOR_RV1_TC5f, &temp));
                    break;
                case 6:
                    SOC_IF_ERROR_RETURN(
                        soc_PN_EGRESS_PKT_TC2PCP_MAPr_field_set
                        (unit, (uint32 *)&reg_value64, PCP_FOR_RV1_TC6f, &temp));
                    break;
                case 7:
                    SOC_IF_ERROR_RETURN(
                        soc_PN_EGRESS_PKT_TC2PCP_MAPr_field_set
                        (unit, (uint32 *)&reg_value64, PCP_FOR_RV1_TC7f, &temp));
                    break;
                default :
                    return SOC_E_PARAM;
                    break;
            }

            SOC_IF_ERROR_RETURN(
                REG_WRITE_PN_EGRESS_PKT_TC2PCP_MAPr(
                unit, port, (uint32 *)&reg_value64));
            break;
        case DRV_PORT_OP_OUTBAND_TC2CPCP:
            SOC_IF_ERROR_RETURN(
                REG_READ_PN_EGRESS_PKT_TC2CPCP_MAPr(
                unit, port, (uint32 *)&reg_value64));

            temp = ((cfi_new & DOT1P_CFI_MASK) << 3) | 
                    (pri_new & DOT1P_PRI_MASK);
            
            /* assigning the field-id */
            switch(pri_old) {
                case 0:
                    SOC_IF_ERROR_RETURN(
                        soc_PN_EGRESS_PKT_TC2CPCP_MAPr_field_set
                        (unit, (uint32 *)&reg_value64, CPCP_FOR_RV1_TC0f, &temp));
                    break;
                case 1:
                       SOC_IF_ERROR_RETURN(
                        soc_PN_EGRESS_PKT_TC2CPCP_MAPr_field_set
                        (unit, (uint32 *)&reg_value64, CPCP_FOR_RV1_TC1f, &temp));
                    break;
                case 2:
                    SOC_IF_ERROR_RETURN(
                        soc_PN_EGRESS_PKT_TC2CPCP_MAPr_field_set
                        (unit, (uint32 *)&reg_value64, CPCP_FOR_RV1_TC2f, &temp));
                    break;
                case 3:
                    SOC_IF_ERROR_RETURN(
                        soc_PN_EGRESS_PKT_TC2CPCP_MAPr_field_set
                        (unit, (uint32 *)&reg_value64, CPCP_FOR_RV1_TC3f, &temp));
                    break;
                case 4:
                    SOC_IF_ERROR_RETURN(
                        soc_PN_EGRESS_PKT_TC2CPCP_MAPr_field_set
                        (unit, (uint32 *)&reg_value64, CPCP_FOR_RV1_TC4f, &temp));
                    break;
                case 5:
                    SOC_IF_ERROR_RETURN(
                        soc_PN_EGRESS_PKT_TC2CPCP_MAPr_field_set
                        (unit, (uint32 *)&reg_value64, CPCP_FOR_RV1_TC5f, &temp));
                    break;
                case 6:
                    SOC_IF_ERROR_RETURN(
                        soc_PN_EGRESS_PKT_TC2CPCP_MAPr_field_set
                        (unit, (uint32 *)&reg_value64, CPCP_FOR_RV1_TC6f, &temp));
                    break;
                case 7:
                    SOC_IF_ERROR_RETURN(
                        soc_PN_EGRESS_PKT_TC2CPCP_MAPr_field_set
                        (unit, (uint32 *)&reg_value64, CPCP_FOR_RV1_TC7f, &temp));
                    break;
                default :
                    return SOC_E_PARAM;
                    break;
            }

            SOC_IF_ERROR_RETURN(
                REG_WRITE_PN_EGRESS_PKT_TC2CPCP_MAPr(
                unit, port, (uint32 *)&reg_value64));
            break;
        default:
            break;
    }
    
    return SOC_E_NONE;
}

/*
 *  Function : drv_starfighter3_port_pri_mapop_get
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
drv_starfighter3_port_pri_mapop_get(int unit, int port, int op_type, 
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

            /* Color */
            SOC_IF_ERROR_RETURN(
                _drv_starfighter3_port_pri_color_mapping(unit, 
                _SF3_TC2COLOR_TABLE_OP_READ, port, *pri_new, cfi_old, cfi_new));
            
            break;
        case DRV_PORT_OP_NORMAL_TC2PCP:
            SOC_IF_ERROR_RETURN(
                REG_READ_PN_EGRESS_PKT_TC2PCP_MAPr(
                unit, port, (uint32 *)&reg_value64));
            
            /* assigning the field-id */
            switch(pri_old) {
                case 0:
                    SOC_IF_ERROR_RETURN(
                        soc_PN_EGRESS_PKT_TC2PCP_MAPr_field_get
                        (unit, (uint32 *)&reg_value64, PCP_FOR_RV0_TC0f, &temp));
                    break;
                case 1:
                    SOC_IF_ERROR_RETURN(
                        soc_PN_EGRESS_PKT_TC2PCP_MAPr_field_get
                        (unit, (uint32 *)&reg_value64, PCP_FOR_RV0_TC1f, &temp));
                    break;
                case 2:
                    SOC_IF_ERROR_RETURN(
                        soc_PN_EGRESS_PKT_TC2PCP_MAPr_field_get
                        (unit, (uint32 *)&reg_value64, PCP_FOR_RV0_TC2f, &temp));
                    break;
                case 3:
                    SOC_IF_ERROR_RETURN(
                        soc_PN_EGRESS_PKT_TC2PCP_MAPr_field_get
                        (unit, (uint32 *)&reg_value64, PCP_FOR_RV0_TC3f, &temp));
                    break;
                case 4:
                    SOC_IF_ERROR_RETURN(
                        soc_PN_EGRESS_PKT_TC2PCP_MAPr_field_get
                        (unit, (uint32 *)&reg_value64, PCP_FOR_RV0_TC4f, &temp));
                    break;
                case 5:
                    SOC_IF_ERROR_RETURN(
                        soc_PN_EGRESS_PKT_TC2PCP_MAPr_field_get
                        (unit, (uint32 *)&reg_value64, PCP_FOR_RV0_TC5f, &temp));
                    break;
                case 6:
                    SOC_IF_ERROR_RETURN(
                        soc_PN_EGRESS_PKT_TC2PCP_MAPr_field_get
                        (unit, (uint32 *)&reg_value64, PCP_FOR_RV0_TC6f, &temp));
                    break;
                case 7:
                    SOC_IF_ERROR_RETURN(
                        soc_PN_EGRESS_PKT_TC2PCP_MAPr_field_get
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
            SOC_IF_ERROR_RETURN(
                REG_READ_PN_EGRESS_PKT_TC2PCP_MAPr(
                unit, port, (uint32 *)&reg_value64));
            
            /* assigning the field-id */
            switch(pri_old) {
                case 0:
                    SOC_IF_ERROR_RETURN(
                        soc_PN_EGRESS_PKT_TC2PCP_MAPr_field_get
                        (unit, (uint32 *)&reg_value64, PCP_FOR_RV1_TC0f, &temp));
                    break;
                case 1:
                    SOC_IF_ERROR_RETURN(
                        soc_PN_EGRESS_PKT_TC2PCP_MAPr_field_get
                        (unit, (uint32 *)&reg_value64, PCP_FOR_RV1_TC1f, &temp));
                    break;
                case 2:
                    SOC_IF_ERROR_RETURN(
                        soc_PN_EGRESS_PKT_TC2PCP_MAPr_field_get
                        (unit, (uint32 *)&reg_value64, PCP_FOR_RV1_TC2f, &temp));
                    break;
                case 3:
                    SOC_IF_ERROR_RETURN(
                        soc_PN_EGRESS_PKT_TC2PCP_MAPr_field_get
                        (unit, (uint32 *)&reg_value64, PCP_FOR_RV1_TC3f, &temp));
                    break;
                case 4:
                    SOC_IF_ERROR_RETURN(
                        soc_PN_EGRESS_PKT_TC2PCP_MAPr_field_get
                        (unit, (uint32 *)&reg_value64, PCP_FOR_RV1_TC4f, &temp));
                    break;
                case 5:
                    SOC_IF_ERROR_RETURN(
                        soc_PN_EGRESS_PKT_TC2PCP_MAPr_field_get
                        (unit, (uint32 *)&reg_value64, PCP_FOR_RV1_TC5f, &temp));
                    break;
                case 6:
                    SOC_IF_ERROR_RETURN(
                        soc_PN_EGRESS_PKT_TC2PCP_MAPr_field_get
                        (unit, (uint32 *)&reg_value64, PCP_FOR_RV1_TC6f, &temp));
                    break;
                case 7:
                    SOC_IF_ERROR_RETURN(
                        soc_PN_EGRESS_PKT_TC2PCP_MAPr_field_get
                        (unit, (uint32 *)&reg_value64, PCP_FOR_RV1_TC7f, &temp));
                    break;
                default :
                    return SOC_E_PARAM;
                    break;
            }
                
            *pri_new = temp & DOT1P_PRI_MASK;
            *cfi_new = (temp >> 3) & DOT1P_CFI_MASK;
            break;
        default:
            break;
    }

    return SOC_E_NONE;
}

