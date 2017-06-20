/*
 * $Id: cosq.c,v 1.3 Broadcom SDK $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */

#include <shared/bsl.h>

#include <soc/drv.h>
#include <soc/debug.h>
#include "robo_sf3.h"

/*
 *  Function : drv_sf3_queue_prio_remap_set
 *
 *  Purpose :
 *      Set the remapping internal priority of the selected port.
 *
 *  Parameters :
 *      unit     :  RoboSwitch unit number.
 *      port     :  port number.
 *      pre_prio :  previous internal priority value.
 *                  Specical design for StarFighter3. Check it in Note.
 *      prio     :  internal priority value.
 *
 *  Return :
 *      SOC_E_XXX.
 *
 *  Note :
 *  1. The PCP to TC mapping in StarFighter3 must be set indicidully per 
 *      DEI value at 0 and 1. All ROBO device has no such feature.
 *  2. Plan to use the same SOC driver to approach the configuration for 
 *      this new feature and prevent the complex coding style for varient 
 *      ROBO device support.
 *  3. The special design to approach this feature and minimum the impact 
 *      to other ROBO device are :
 *      - the parameter at 'pre_prio' will be used for NorthStar to carray 
 *          both DEI and Priority information.
 *          a. bit 0~2 : 1p/1q tag priority, i.e. PCP
 *          b. bit 3 : DEI value. 0 for DEI_0, 1 for DEI_1
 *          p.s. such deisgn is the with 1p/1q tag for 
 *              -> CTAG={cfi(bit3)+pri(bit2-bit1)}
 *              -> STAG={dei(bit3)+pri(bit2-bit1)}
 *      - DEI=0 for ROBO's legacy PCP2TC support.
 */
int
drv_sf3_queue_prio_remap_set(int unit, uint32 port, uint8 pre_prio, 
    uint8 prio)
{
    uint32  reg_value, temp, p;
    soc_pbmp_t  pbmp;
    int in_dei = FALSE, in_pcp = -1;

    LOG_VERBOSE(BSL_LS_SOC_COMMON,
                (BSL_META_U(unit,
                            "%s:port=%d,pre_prio=%d,prio=%d\n"), 
                 FUNCTION_NAME(), port, pre_prio, prio));

    SOC_PBMP_CLEAR(pbmp);
    if (port == -1) {
        SOC_PBMP_ASSIGN(pbmp, PBMP_ALL(unit));
    } else {
        /* Check port number */
        if (SOC_PBMP_MEMBER(PBMP_ALL(unit), port)){
            SOC_PBMP_PORT_SET(pbmp, port);
        } else {
            return SOC_E_PARAM;
        }
    }

    /* retrieve DEI and PCP */
    if (pre_prio & ~(DRV_SF3_TAG_PRIORITY_MASK)){
        return SOC_E_PARAM;
    }
    DRV_SF3_TAG_PRIORITY_DEI_GET(pre_prio, in_dei);
    DRV_SF3_TAG_PRIORITY_PCP_GET(pre_prio, in_pcp);

    PBMP_ITER(pbmp, p) {
    
        if (in_dei == 1) {
            if (p == 7) {
                SOC_IF_ERROR_RETURN(REG_READ_P7_PCP2TC_DEI1r
                        (unit, &reg_value));
            } else  if (IS_CPU_PORT(unit, p)) {
                SOC_IF_ERROR_RETURN(REG_READ_IMP_PCP2TC_DEI1r
                        (unit, &reg_value));
            } else {
                SOC_IF_ERROR_RETURN(REG_READ_PN_PCP2TC_DEI1r
                    (unit, p, &reg_value));
            }
        } else {
            if (p == 7) {
                SOC_IF_ERROR_RETURN(REG_READ_P7_PCP2TC_DEI0r
                        (unit, &reg_value));
            } else  if (IS_CPU_PORT(unit, p)) {
                SOC_IF_ERROR_RETURN(REG_READ_IMP_PCP2TC_DEI0r
                        (unit, &reg_value));
            } else {
                SOC_IF_ERROR_RETURN(REG_READ_PN_PCP2TC_DEI0r
                    (unit, p, &reg_value));
            }
        }


        temp = (uint32)prio;
        switch (in_pcp) {
            case 0:
                SOC_IF_ERROR_RETURN(soc_PN_PCP2TC_DEI0r_field_set
                    (unit, &reg_value, TAG000_PRI_MAPf, &temp));
                break;
            case 1:
                SOC_IF_ERROR_RETURN(soc_PN_PCP2TC_DEI0r_field_set
                    (unit, &reg_value, TAG001_PRI_MAPf, &temp));
                break;
            case 2:         
                SOC_IF_ERROR_RETURN(soc_PN_PCP2TC_DEI0r_field_set
                    (unit, &reg_value, TAG010_PRI_MAPf, &temp));
                break;
            case 3:
                SOC_IF_ERROR_RETURN(soc_PN_PCP2TC_DEI0r_field_set
                    (unit, &reg_value, TAG011_PRI_MAPf, &temp));
                break;
            case 4:
                SOC_IF_ERROR_RETURN(soc_PN_PCP2TC_DEI0r_field_set
                    (unit, &reg_value, TAG100_PRI_MAPf, &temp));
                break;
            case 5:
                SOC_IF_ERROR_RETURN(soc_PN_PCP2TC_DEI0r_field_set
                    (unit, &reg_value, TAG101_PRI_MAPf, &temp));
                break;
            case 6:         
                SOC_IF_ERROR_RETURN(soc_PN_PCP2TC_DEI0r_field_set
                    (unit, &reg_value, TAG110_PRI_MAPf, &temp));
                break;
            case 7:
                SOC_IF_ERROR_RETURN(soc_PN_PCP2TC_DEI0r_field_set
                    (unit, &reg_value, TAG111_PRI_MAPf, &temp));
                break;
        }        

        if (in_dei == 1) {
            if (p == 7) {
                SOC_IF_ERROR_RETURN(REG_WRITE_P7_PCP2TC_DEI1r
                        (unit, &reg_value));
            } else  if (IS_CPU_PORT(unit, p)) {
                SOC_IF_ERROR_RETURN(REG_WRITE_IMP_PCP2TC_DEI1r
                        (unit, &reg_value));
            } else {
                SOC_IF_ERROR_RETURN(REG_WRITE_PN_PCP2TC_DEI1r
                    (unit, p, &reg_value));
            }
        } else {
            if (p == 7) {
                SOC_IF_ERROR_RETURN(REG_WRITE_P7_PCP2TC_DEI0r
                        (unit, &reg_value));
            } else  if (IS_CPU_PORT(unit, p)) {
                SOC_IF_ERROR_RETURN(REG_WRITE_IMP_PCP2TC_DEI0r
                        (unit, &reg_value));
            } else {
                SOC_IF_ERROR_RETURN(REG_WRITE_PN_PCP2TC_DEI0r
                    (unit, p, &reg_value));
            }
        }

    }

    return SOC_E_NONE;
}

/*
 *  Function : drv_sf3_queue_prio_remap_get
 *
 *  Purpose :
 *      Get the remapping internal priority of the selected port.
 *
 *  Parameters :
 *      unit     :  RoboSwitch unit number.
 *      port     :  port number.
 *      pre_prio :  previous internal priority value.
 *                  Specical design for StarFighter3. Check it in Note.
 *      prio     :  (OUT) internal priority value.
 *
 *  Return :
 *      SOC_E_XXX.
 *
 *  Note :
 *  1. The PCP to TC mapping in StarFighter3 must be set indicidully per 
 *      DEI value at 0 and 1. All ROBO device has no such feature.
 *  2. Plan to use the same SOC driver to approach the configuration for 
 *      this new feature and prevent the complex coding style for varient 
 *      ROBO device support.
 *  3. The special design to approach this feature and minimum the impact 
 *      to other ROBO device are :
 *      - the parameter at 'pre_prio' will be used for NorthStar to carray 
 *          both DEI and Priority information.
 *          a. bit 0~2 : 1p/1q tag priority, i.e. PCP
 *          b. bit 3 : DEI value. 0 for DEI_0, 1 for DEI_1
 *          p.s. such deisgn is the with 1p/1q tag for 
 *              -> CTAG={cfi(bit3)+pri(bit2-bit1)}
 *              -> STAG={dei(bit3)+pri(bit2-bit1)}
 *      - DEI=0 for ROBO's legacy PCP2TC support.
 */
int
drv_sf3_queue_prio_remap_get(int unit, uint32 port, uint8 pre_prio, 
    uint8 *prio)
{
    uint32  reg_value, temp = 0, p;
    int in_dei = FALSE, in_pcp = -1;

    if (port == -1) {
        /* Get GE0's PN_PCP2TC register value */
        p = 0;
    } else {
        /* Check port number */
        if (SOC_PBMP_MEMBER(PBMP_ALL(unit), port)){
            p = port;
        } else {
            return SOC_E_PARAM;
        }
    }

    /* retrieve DEI and PCP */
    if (pre_prio & ~(DRV_SF3_TAG_PRIORITY_MASK)){
        return SOC_E_PARAM;
    }
    DRV_SF3_TAG_PRIORITY_DEI_GET(pre_prio, in_dei);
    DRV_SF3_TAG_PRIORITY_PCP_GET(pre_prio, in_pcp);

    if (in_dei == 1) {
        if (p == 7) {
            SOC_IF_ERROR_RETURN(REG_READ_P7_PCP2TC_DEI1r
                    (unit, &reg_value));
        } else if (IS_CPU_PORT(unit, p)) {
            SOC_IF_ERROR_RETURN(REG_READ_IMP_PCP2TC_DEI1r
                    (unit, &reg_value));
        } else {
            SOC_IF_ERROR_RETURN(REG_READ_PN_PCP2TC_DEI1r
                (unit, p, &reg_value));
        }
    } else {
        if (p == 7) {
            SOC_IF_ERROR_RETURN(REG_READ_P7_PCP2TC_DEI0r
                    (unit, &reg_value));
        } else if (IS_CPU_PORT(unit, p)) {
            SOC_IF_ERROR_RETURN(REG_READ_IMP_PCP2TC_DEI0r
                    (unit, &reg_value));
        } else {
            SOC_IF_ERROR_RETURN(REG_READ_PN_PCP2TC_DEI0r
                (unit, p, &reg_value));
        }
    }

    switch (in_pcp) {
        case 0:
            SOC_IF_ERROR_RETURN(soc_PN_PCP2TC_DEI0r_field_get
                (unit, &reg_value, TAG000_PRI_MAPf, &temp));
            break;
        case 1:         
            SOC_IF_ERROR_RETURN(soc_PN_PCP2TC_DEI0r_field_get
                (unit, &reg_value, TAG001_PRI_MAPf, &temp));
            break;
        case 2:         
            SOC_IF_ERROR_RETURN(soc_PN_PCP2TC_DEI0r_field_get
                (unit, &reg_value, TAG010_PRI_MAPf, &temp));
            break;
        case 3:
            SOC_IF_ERROR_RETURN(soc_PN_PCP2TC_DEI0r_field_get
                (unit, &reg_value, TAG011_PRI_MAPf, &temp));
            break;
        case 4:
            SOC_IF_ERROR_RETURN(soc_PN_PCP2TC_DEI0r_field_get
                (unit, &reg_value, TAG100_PRI_MAPf, &temp));
            break;
        case 5:
            SOC_IF_ERROR_RETURN(soc_PN_PCP2TC_DEI0r_field_get
                (unit, &reg_value, TAG101_PRI_MAPf, &temp));
            break;
        case 6:         
            SOC_IF_ERROR_RETURN(soc_PN_PCP2TC_DEI0r_field_get
                (unit, &reg_value, TAG110_PRI_MAPf, &temp));
            break;
        case 7:
            SOC_IF_ERROR_RETURN(soc_PN_PCP2TC_DEI0r_field_get
                (unit, &reg_value, TAG111_PRI_MAPf, &temp));
            break;
    }        
    *prio = temp;

    LOG_VERBOSE(BSL_LS_SOC_COMMON,
                (BSL_META_U(unit,
                            "%s:port=%d,pre_prio=%d,*prio=%d\n"), 
                 FUNCTION_NAME(), port, pre_prio, *prio));

     return SOC_E_NONE;
}

/*
 *  Function : drv_sf3_queue_port_prio_to_queue_set
 *
 *  Purpose :
 *      Set the queue id for TC2COS mapping of selected port.
 *
 *  Parameters :
 *      unit     :  RoboSwitch unit number.
 *      port     :  port id.
 *      prio     :  internal priority value.
 *      queue_n  :  queue number.
 *
 *  Return :
 *      SOC_E_XXX.
 *
 *  Note :
 */
int
drv_sf3_queue_port_prio_to_queue_set(int unit, uint8 port, 
    uint8 prio, uint8 queue_n)
{
    uint32  reg_value, temp;
    /* Using 32b port variable to compare against SOC_MAX_NUM_PORTS,
       which may be bigger the uint8 and then reuslts in a compilation error */
    uint32  port32 = port;

    LOG_VERBOSE(BSL_LS_SOC_COMMON,
                (BSL_META_U(unit,
                            "%s:unit %d,port=%d,prio=%d,queue_n=%d\n"), 
                 FUNCTION_NAME(), unit, port32, prio, queue_n));

    /* Check port number */
    if (!SOC_PBMP_MEMBER(PBMP_ALL(unit), port32)){
        return SOC_E_PARAM;
    }

    if (IS_CPU_PORT(unit, port32)) {
        SOC_IF_ERROR_RETURN(REG_READ_IMP_TC2COS_MAPr
            (unit, &reg_value));
    } else {
        SOC_IF_ERROR_RETURN(REG_READ_PN_TC2COS_MAPr
            (unit, port32, &reg_value));
    }

    temp = queue_n;
    switch (prio) {
        case 0:
            SOC_IF_ERROR_RETURN(soc_PN_TC2COS_MAPr_field_set
                (unit, &reg_value, PRT000_TO_QIDf, &temp));
            break;
        case 1:         
            SOC_IF_ERROR_RETURN(soc_PN_TC2COS_MAPr_field_set
                (unit, &reg_value, PRT001_TO_QIDf, &temp));
            break;
        case 2:         
            SOC_IF_ERROR_RETURN(soc_PN_TC2COS_MAPr_field_set
                (unit, &reg_value, PRT010_TO_QIDf, &temp));
            break;
        case 3:
            SOC_IF_ERROR_RETURN(soc_PN_TC2COS_MAPr_field_set
                (unit, &reg_value, PRT011_TO_QIDf, &temp));
            break;
        case 4:
            SOC_IF_ERROR_RETURN(soc_PN_TC2COS_MAPr_field_set
                (unit, &reg_value, PRT100_TO_QIDf, &temp));
            break;
        case 5:
            SOC_IF_ERROR_RETURN(soc_PN_TC2COS_MAPr_field_set
                (unit, &reg_value, PRT101_TO_QIDf, &temp));
            break;
        case 6:         
            SOC_IF_ERROR_RETURN(soc_PN_TC2COS_MAPr_field_set
                (unit, &reg_value, PRT110_TO_QIDf, &temp));
            break;
        case 7:
            SOC_IF_ERROR_RETURN(soc_PN_TC2COS_MAPr_field_set
                (unit, &reg_value, PRT111_TO_QIDf, &temp));
            break;
        default:
            return SOC_E_PARAM;
    }        

    if (IS_CPU_PORT(unit, port32)) {
        SOC_IF_ERROR_RETURN(REG_WRITE_IMP_TC2COS_MAPr
            (unit, &reg_value));
    } else {
        SOC_IF_ERROR_RETURN(REG_WRITE_PN_TC2COS_MAPr
            (unit, port32, &reg_value));
    }

     return SOC_E_NONE;
}

/*
 *  Function : drv_sf3_queue_port_prio_to_queue_get
 *
 *  Purpose :
 *      Get the queue id for TC2COS mapping of selected port.
 *
 *  Parameters :
 *      unit     :  RoboSwitch unit number.
 *      port     :  port id.
 *      prio     :  internal priority value.
 *      queue_n  :  (OUT) queue number.
 *
 *  Return :
 *      SOC_E_XXX.
 *
 *  Note :
 */
int
drv_sf3_queue_port_prio_to_queue_get(int unit, uint8 port, 
    uint8 prio, uint8 *queue_n)
{
    uint32  reg_value, temp;
    /* Using 32b port variable to compare against SOC_MAX_NUM_PORTS,
       which may be bigger the uint8 and then reuslts in a compilation error */
    uint32  port32 = port;

    /* Check port number */
    if (!SOC_PBMP_MEMBER(PBMP_ALL(unit), port32)){
        return SOC_E_PARAM;
    }

    if (IS_CPU_PORT(unit, port32)) {
        SOC_IF_ERROR_RETURN(REG_READ_IMP_TC2COS_MAPr
            (unit, &reg_value));
    } else {
        SOC_IF_ERROR_RETURN(REG_READ_PN_TC2COS_MAPr
            (unit, port32, &reg_value));
    }

    switch (prio) {
       case 0:
            SOC_IF_ERROR_RETURN(soc_PN_TC2COS_MAPr_field_get
                (unit, &reg_value, PRT000_TO_QIDf, &temp));
            break;
        case 1:         
            SOC_IF_ERROR_RETURN(soc_PN_TC2COS_MAPr_field_get
                (unit, &reg_value, PRT001_TO_QIDf, &temp));
            break;
        case 2:         
            SOC_IF_ERROR_RETURN(soc_PN_TC2COS_MAPr_field_get
                (unit, &reg_value, PRT010_TO_QIDf, &temp));
            break;
        case 3:
            SOC_IF_ERROR_RETURN(soc_PN_TC2COS_MAPr_field_get
                (unit, &reg_value, PRT011_TO_QIDf, &temp));
            break;
        case 4:
            SOC_IF_ERROR_RETURN(soc_PN_TC2COS_MAPr_field_get
                (unit, &reg_value, PRT100_TO_QIDf, &temp));
            break;
        case 5:
            SOC_IF_ERROR_RETURN(soc_PN_TC2COS_MAPr_field_get
                (unit, &reg_value, PRT101_TO_QIDf, &temp));
            break;
        case 6:         
            SOC_IF_ERROR_RETURN(soc_PN_TC2COS_MAPr_field_get
                (unit, &reg_value, PRT110_TO_QIDf, &temp));
            break;
        case 7:
            SOC_IF_ERROR_RETURN(soc_PN_TC2COS_MAPr_field_get
                (unit, &reg_value, PRT111_TO_QIDf, &temp));
            break;
        default:
            return SOC_E_PARAM;
    }
    *queue_n = temp;

    LOG_VERBOSE(BSL_LS_SOC_COMMON,
                (BSL_META_U(unit,
                            "%s:unit %d,port=%d,prio=%d,*queue_n=%d\n"), 
                 FUNCTION_NAME(), unit, port, prio, *queue_n));

    return SOC_E_NONE;
}

/* Config output queue mapping */
/*
 *  Function : drv_sf3_queue_prio_set
 *
 *  Purpose :
 *      Set the priority value of the specific queue.
 *
 *  Parameters :
 *      unit     :  RoboSwitch unit number.
 *      port     :  port number.
 *      prio     :  internal priority value.
 *      queue_n  :  queue number.
 *
 *  Return :
 *      SOC_E_XXX.
 *
 *  Note :
 */
int 
drv_sf3_queue_prio_set(int unit, uint32 port, uint8 prio, 
    uint8 queue_n)
{
    uint32  p;
    soc_pbmp_t  pbmp;

    LOG_VERBOSE(BSL_LS_SOC_COMMON,
                (BSL_META_U(unit,
                            "%s:unit %d,port=%d,priority=%d,queue=%d\n"),
                 FUNCTION_NAME(), unit, port, prio, queue_n));

    SOC_IF_ERROR_RETURN(DRV_QUEUE_PRIO_REMAP_SET
        (unit, port, prio, prio));

    SOC_PBMP_CLEAR(pbmp);
    if (port == -1) {
        SOC_PBMP_ASSIGN(pbmp, PBMP_ALL(unit));
    } else {
        /* Check port number */
        if (SOC_PBMP_MEMBER(PBMP_ALL(unit), port)){
            SOC_PBMP_PORT_SET(pbmp, port);
        } else {
            return SOC_E_PARAM;
        }
    }

    PBMP_ITER(pbmp, p) {
        SOC_IF_ERROR_RETURN(DRV_QUEUE_PORT_PRIO_TO_QUEUE_SET
            (unit, p, prio, queue_n));
    }

    return SOC_E_NONE;
}

/*
 *  Function : drv_sf3_queue_prio_get
 *
 *  Purpose :
 *      Get the priority value of the specific queue.
 *
 *  Parameters :
 *      unit     :  RoboSwitch unit number.
 *      port     :  port number.
 *      prio     :  internal priority value.
 *      queue_n  :  (OUT) queue number.
 *
 *  Return :
 *      SOC_E_XXX.
 *
 *  Note :
 */
int 
drv_sf3_queue_prio_get(int unit, uint32 port, uint8 prio, 
    uint8 *queue_n)
{
    uint32  p;
    uint8   new_prio = 0;

    if (port == -1) {
        /* Get GE0's PN_PCP2TC register value */
        p = 0;
    } else {
        /* Check port number */
        if (SOC_PBMP_MEMBER(PBMP_ALL(unit), port)){
            p = port;
        } else {
            return SOC_E_PARAM;
        }
    }

    /* Get the new_prio value from PCP2TC mapping by per-port based */
    SOC_IF_ERROR_RETURN(DRV_QUEUE_PRIO_REMAP_GET
        (unit, p, prio, &new_prio));
    
    /* Transfer the new_prio by TC2COS mapping (per-system based) */
    SOC_IF_ERROR_RETURN(DRV_QUEUE_PORT_PRIO_TO_QUEUE_GET
        (unit, p, new_prio, queue_n));

    LOG_VERBOSE(BSL_LS_SOC_COMMON,
                (BSL_META_U(unit,
                            "%s:unit %d,port=%d,priority=%d,queue=%d\n"), 
                 FUNCTION_NAME(), unit, port, prio, *queue_n));

    return SOC_E_NONE;
}

/*
 *  Function : drv_sf3_queue_mode_set
 *
 *  Purpose :
 *      Set the queue mode of selected port type.
 *
 *  Parameters :
 *      unit     :  RoboSwitch unit number.
 *      bmp      :  port bitmap.
 *      flags    :  flags
 *      mode     :  queue mode.
 *
 *  Return :
 *      SOC_E_XXX.
 *
 *  Note :
 *
 */
int 
drv_sf3_queue_mode_set(int unit, soc_pbmp_t bmp, uint32 flags, 
    uint32 mode)
{
    uint32  reg_value, temp, p;
    soc_pbmp_t in_pbmp;

    LOG_VERBOSE(BSL_LS_SOC_COMMON,
                (BSL_META_U(unit,
                            "%s:unit %d,bmp=0x%x,flag=0x%x,queue mode=%d\n"),
                 FUNCTION_NAME(),unit, SOC_PBMP_WORD_GET(bmp, 0), flags, mode));

    if (flags != 0) {
        return SOC_E_PARAM;
    }

    /* port validation */
    SOC_PBMP_ASSIGN(in_pbmp, bmp);
    SOC_PBMP_REMOVE(in_pbmp, PBMP_ALL(unit));
    if (SOC_PBMP_NOT_NULL(in_pbmp)) {
        return SOC_E_PARAM;
    }

    PBMP_ITER(bmp, p) {
        switch (mode) {
            case DRV_QUEUE_MODE_STRICT:
                temp = 0;
                break;
            case DRV_QUEUE_MODE_1STRICT_7WDRR:
                temp = 1;
                break;
            case DRV_QUEUE_MODE_2STRICT_6WDRR:
                temp = 2;
                break;
            case DRV_QUEUE_MODE_3STRICT_5WDRR:
                temp = 3;
                break;
            case DRV_QUEUE_MODE_4STRICT_4WDRR:
                temp = 4;
                break;
            case DRV_QUEUE_MODE_WDRR:
            case DRV_QUEUE_MODE_WRR:
                /* WRR : 1: number of packet.
                *  WDRR : 0: number of 255-bytes.
                */
                temp = (mode == DRV_QUEUE_MODE_WRR) ? 1 : 0;
                SOC_IF_ERROR_RETURN(DRV_QUEUE_QOS_CONTROL_SET
                        (unit, p, DRV_QOS_CTL_WDRR_GRANULARTTY, temp));

                temp = 5;
                break;
            default:
                return SOC_E_UNAVAIL;
        }

        if (IS_CPU_PORT(unit, p)) {
            SOC_IF_ERROR_RETURN(REG_READ_IMP_QOS_PRI_CTLr
                    (unit, &reg_value));
        } else if (p == 7) {
            SOC_IF_ERROR_RETURN(REG_READ_P7_QOS_PRI_CTLr
                    (unit, &reg_value));
        } else {
            SOC_IF_ERROR_RETURN(REG_READ_PN_QOS_PRI_CTLr
                    (unit, p, &reg_value));
        }

        SOC_IF_ERROR_RETURN(soc_PN_QOS_PRI_CTLr_field_set
            (unit, &reg_value, SCHEDULER_SELECTf, &temp));

        if (IS_CPU_PORT(unit, p)) {
            SOC_IF_ERROR_RETURN(REG_WRITE_IMP_QOS_PRI_CTLr
                    (unit, &reg_value));
        } else if (p == 7) {
            SOC_IF_ERROR_RETURN(REG_WRITE_P7_QOS_PRI_CTLr
                    (unit, &reg_value));
        } else {
            SOC_IF_ERROR_RETURN(REG_WRITE_PN_QOS_PRI_CTLr
                    (unit, p, &reg_value));
        }
    }

    return SOC_E_NONE;
}

/*
 *  Function : drv_sf3_queue_mode_get
 *
 *  Purpose :
 *      Get the queue mode of selected port type.
 *
 *  Parameters :
 *      unit     :  RoboSwitch unit number.
 *      port     :  port number.
 *      flags    :  flags
 *      mode     :  (OUT) queue mode.
 *
 *  Return :
 *      SOC_E_XXX.
 *
 *  Note :
 */  
int 
drv_sf3_queue_mode_get(int unit, uint32 port, uint32 flags, 
    uint32 *mode)
{
    uint32  reg_value, temp, val = 0;

    if (flags != 0) {
        return SOC_E_PARAM;
    }

    /* Check port number */
    if (!SOC_PBMP_MEMBER(PBMP_ALL(unit), port)){
        return SOC_E_PARAM;
    }

    if (IS_CPU_PORT(unit, port)) {
        SOC_IF_ERROR_RETURN(REG_READ_IMP_QOS_PRI_CTLr
                (unit, &reg_value));
    } else if (port == 7) {
        SOC_IF_ERROR_RETURN(REG_READ_P7_QOS_PRI_CTLr
                (unit, &reg_value));
    } else {
        SOC_IF_ERROR_RETURN(REG_READ_PN_QOS_PRI_CTLr
                (unit, port, &reg_value));
    }
    SOC_IF_ERROR_RETURN(soc_PN_QOS_PRI_CTLr_field_get
            (unit, &reg_value, SCHEDULER_SELECTf, &temp));

    switch (temp) {
        case 0:
            *mode = DRV_QUEUE_MODE_STRICT;
            break;
        case 1:
            *mode = DRV_QUEUE_MODE_1STRICT_7WDRR;
            break;
        case 2:
            *mode = DRV_QUEUE_MODE_2STRICT_6WDRR;
            break;
        case 3:
            *mode = DRV_QUEUE_MODE_3STRICT_5WDRR;
            break;
        case 4:
            *mode = DRV_QUEUE_MODE_4STRICT_4WDRR;
            break;
        case 5:
            /* WRR : 1: number of packet.
             *  WDRR : 0: number of 64-bytes.
             */
            SOC_IF_ERROR_RETURN(DRV_QUEUE_QOS_CONTROL_GET
                (unit, port, DRV_QOS_CTL_WDRR_GRANULARTTY, &val));
            if (val) {
                *mode = DRV_QUEUE_MODE_WRR;
            } else {
                *mode = DRV_QUEUE_MODE_WDRR;
            }
            break;
        default:
            return SOC_E_INTERNAL;
    }

    LOG_VERBOSE(BSL_LS_SOC_COMMON,
                (BSL_META_U(unit,
                            "%s:unit %d,port=%d,flags=0x%x,mode=%d\n"),
                 FUNCTION_NAME(), unit, port, flags, *mode));    

    return SOC_E_NONE;
}

/*
 *  Function : drv_sf3_queue_WRR_weight_set
 *
 *  Purpose :
 *      Set the weight value to the specific queue.
 *
 *  Parameters :
 *      unit     :  RoboSwitch unit number.
 *      port_type:  port type.
 *      bmp      :  port bitmap.
 *      queue    :  queue number.
 *      weight   :  weight value.
 *
 *  Return :
 *      SOC_E_NONE  :  success.
 *      SOC_E_PARAM :  parameters error.
 *
 *  Note :
 *      
 *
 */
int 
drv_sf3_queue_WRR_weight_set(int unit, uint32 port_type, 
    soc_pbmp_t bmp, uint8 queue, uint32 weight)
{
    uint32  temp, p;
    uint32  max_weight = 0;
    uint64  reg_value64;
    soc_pbmp_t in_pbmp;

    LOG_VERBOSE(BSL_LS_SOC_COMMON, \
                (BSL_META_U(unit, \
                            "%s: \
                            unit %d, port type = %d, bmp = 0x%x, queue = %d, weight = %d\n"),
                 FUNCTION_NAME(), unit, port_type, SOC_PBMP_WORD_GET(bmp, 0), 
                 queue, weight));

    SOC_IF_ERROR_RETURN(DRV_DEV_PROP_GET
        (unit, DRV_DEV_PROP_COSQ_MAX_WEIGHT_VALUE, &max_weight));

    if ((weight > max_weight) || (weight < 1)) {
        return SOC_E_PARAM;
    }

    /* port validation */
    SOC_PBMP_ASSIGN(in_pbmp, bmp);
    SOC_PBMP_REMOVE(in_pbmp, PBMP_ALL(unit));
    if (SOC_PBMP_NOT_NULL(in_pbmp)) {
        return SOC_E_PARAM;
    }

    PBMP_ITER(bmp, p) {
        if (IS_CPU_PORT(unit, p)) {
            SOC_IF_ERROR_RETURN(REG_READ_IMP_QOS_WEIGHTr
                    (unit, (uint32 *)&reg_value64));
        } else if (p == 7) {
            SOC_IF_ERROR_RETURN(REG_READ_P7_QOS_WEIGHTr
                    (unit, (uint32 *)&reg_value64));
        } else {
            SOC_IF_ERROR_RETURN(REG_READ_PN_QOS_WEIGHTr
                    (unit, p, (uint32 *)&reg_value64));
        }

        temp = weight;
        switch (queue) {
            case 0:
                SOC_IF_ERROR_RETURN(soc_PN_QOS_WEIGHTr_field_set
                        (unit, (uint32 *)&reg_value64, Q0_WEIGHTf, &temp));
                break;
            case 1:
                SOC_IF_ERROR_RETURN(soc_PN_QOS_WEIGHTr_field_set
                        (unit, (uint32 *)&reg_value64, Q1_WEIGHTf, &temp));
                break;
            case 2:
                SOC_IF_ERROR_RETURN(soc_PN_QOS_WEIGHTr_field_set
                        (unit, (uint32 *)&reg_value64, Q2_WEIGHTf, &temp));
                break;
            case 3:
                SOC_IF_ERROR_RETURN(soc_PN_QOS_WEIGHTr_field_set
                        (unit, (uint32 *)&reg_value64, Q3_WEIGHTf, &temp));
                break;
            case 4:
                SOC_IF_ERROR_RETURN(soc_PN_QOS_WEIGHTr_field_set
                        (unit, (uint32 *)&reg_value64, Q4_WEIGHTf, &temp));
                break;
            case 5:
                SOC_IF_ERROR_RETURN(soc_PN_QOS_WEIGHTr_field_set
                        (unit, (uint32 *)&reg_value64, Q5_WEIGHTf, &temp));
                break;
            case 6:
                SOC_IF_ERROR_RETURN(soc_PN_QOS_WEIGHTr_field_set
                        (unit, (uint32 *)&reg_value64, Q6_WEIGHTf, &temp));
                break;
            case 7:
                SOC_IF_ERROR_RETURN(soc_PN_QOS_WEIGHTr_field_set
                        (unit, (uint32 *)&reg_value64, Q7_WEIGHTf, &temp));
                break;
            default:
                return SOC_E_PARAM;
        }
    
        if (IS_CPU_PORT(unit, p)) {
            SOC_IF_ERROR_RETURN(REG_WRITE_IMP_QOS_WEIGHTr
                    (unit, (uint32 *)&reg_value64));
        } else if (p == 7) {
            SOC_IF_ERROR_RETURN(REG_WRITE_P7_QOS_WEIGHTr
                    (unit, (uint32 *)&reg_value64));
        } else {
            SOC_IF_ERROR_RETURN(REG_WRITE_PN_QOS_WEIGHTr
                    (unit, p, (uint32 *)&reg_value64));
        }
    }

    return SOC_E_NONE;
}

/*
 *  Function : drv_sf3_queue_WRR_weight_get
 *
 *  Purpose :
 *      Get the weight value to the specific queue.
 *
 *  Parameters :
 *      unit     :  RoboSwitch unit number.
 *      port_type:  port type.
 *      port     :  port number.
 *      queue    :  queue number.
 *      weight   :  (OUT) weight value.
 *
 *  Return :
 *      SOC_E_NONE  :  success.
 *
 *  Note :
 */
int 
drv_sf3_queue_WRR_weight_get(int unit, uint32 port_type, 
    uint32 port, uint8 queue, uint32 *weight)
{
    uint32  temp;
    uint64  reg_value64;

    /* Check port number */
    if (!SOC_PBMP_MEMBER(PBMP_ALL(unit), port)){
        return SOC_E_PARAM;
    }

    if (IS_CPU_PORT(unit, port)) {
        SOC_IF_ERROR_RETURN(REG_READ_IMP_QOS_WEIGHTr
                (unit, (uint32 *)&reg_value64));
    } else if (port == 7) {
        SOC_IF_ERROR_RETURN(REG_READ_P7_QOS_WEIGHTr
                (unit, (uint32 *)&reg_value64));
    } else {
        SOC_IF_ERROR_RETURN(REG_READ_PN_QOS_WEIGHTr
                (unit, port, (uint32 *)&reg_value64));
    }

    switch (queue) {
        case 0:
            SOC_IF_ERROR_RETURN(soc_PN_QOS_WEIGHTr_field_get
                (unit, (uint32 *)&reg_value64, Q0_WEIGHTf, &temp));
            break;
        case 1:
            SOC_IF_ERROR_RETURN(soc_PN_QOS_WEIGHTr_field_get
                (unit, (uint32 *)&reg_value64, Q1_WEIGHTf, &temp));
            break;
        case 2:
            SOC_IF_ERROR_RETURN(soc_PN_QOS_WEIGHTr_field_get
                (unit, (uint32 *)&reg_value64, Q2_WEIGHTf, &temp));
            break;
        case 3:
            SOC_IF_ERROR_RETURN(soc_PN_QOS_WEIGHTr_field_get
                (unit, (uint32 *)&reg_value64, Q3_WEIGHTf, &temp));
            break;
        case 4:
            SOC_IF_ERROR_RETURN(soc_PN_QOS_WEIGHTr_field_get
                (unit, (uint32 *)&reg_value64, Q4_WEIGHTf, &temp));
            break;
        case 5:
            SOC_IF_ERROR_RETURN(soc_PN_QOS_WEIGHTr_field_get
                (unit, (uint32 *)&reg_value64, Q5_WEIGHTf, &temp));
            break;
        case 6:
            SOC_IF_ERROR_RETURN(soc_PN_QOS_WEIGHTr_field_get
                (unit, (uint32 *)&reg_value64, Q6_WEIGHTf, &temp));
            break;
        case 7:
            SOC_IF_ERROR_RETURN(soc_PN_QOS_WEIGHTr_field_get
                (unit, (uint32 *)&reg_value64, Q7_WEIGHTf, &temp));
            break;
        default:
            return SOC_E_PARAM;
    }

    *weight = temp;

    LOG_VERBOSE(BSL_LS_SOC_COMMON, \
                (BSL_META_U(unit, \
                            "%s: \
                            unit %d, port type = %d, port = %d, queue = %d, weight = %d\n"),
                 FUNCTION_NAME(), unit, port_type, port, queue, *weight));

    return SOC_E_NONE;
}

/*
 *  Function : drv_sf3_queue_qos_control_set
 *
 *  Purpose :
 *      Set the qos control.
 *
 *  Parameters :
 *      unit         :  RoboSwitch unit number.
 *      port    :  port number
 *      type    :  qos control type.
 *      state        :  The state of the selected type.
 *
 *  Return :
 *      SOC_E_XXX.
 *
 *  Note :
 */
int 
drv_sf3_queue_qos_control_set(int unit, uint32 port, uint32 type, uint32 state)
{
    uint32  reg_value, temp, p;
    soc_pbmp_t pbm;

    LOG_VERBOSE(BSL_LS_SOC_COMMON, \
                (BSL_META_U(unit, \
                            "%s: \
                            unit %d, port = %d, type = 0x%x, state = 0x%x\n"), 
                 FUNCTION_NAME(), unit, port, type, state));

    /* port validation */
    if ((int)port == -1) {
        /* once user request system setting. ALL ports(include CPU) will comply 
         * the same setting. 
         */
        SOC_PBMP_ASSIGN(pbm, PBMP_ALL(unit));
    } else {
        /* Check port number */
        if (SOC_PBMP_MEMBER(PBMP_ALL(unit), port)){
            SOC_PBMP_CLEAR(pbm);
            SOC_PBMP_PORT_ADD(pbm, port);
        } else {
            return SOC_E_PARAM;
        }
    }

    PBMP_ITER(pbm, p) {
        switch (type) {
            case DRV_QOS_CTL_FLOOD_DROP_TCMAP:
                if (IS_CPU_PORT(unit, p)) {
                    SOC_IF_ERROR_RETURN(REG_READ_IMP_TC2COS_MAPr
                            (unit, &reg_value));
                } else if (p == 7) {
                    SOC_IF_ERROR_RETURN(REG_READ_P7_TC2COS_MAPr
                            (unit, &reg_value));
                } else {
                    SOC_IF_ERROR_RETURN(REG_READ_PN_TC2COS_MAPr
                            (unit, p, &reg_value));
                }

                temp = state;  
                SOC_IF_ERROR_RETURN(soc_PN_TC2COS_MAPr_field_set
                        (unit, &reg_value, BCAST_DLF_DROP_TCf, &temp));

                if (IS_CPU_PORT(unit, p)) {
                    SOC_IF_ERROR_RETURN(REG_WRITE_IMP_TC2COS_MAPr
                            (unit, &reg_value));
                } else if (p == 7) {
                    SOC_IF_ERROR_RETURN(REG_WRITE_P7_TC2COS_MAPr
                            (unit, &reg_value));
                } else {
                    SOC_IF_ERROR_RETURN(REG_WRITE_PN_TC2COS_MAPr
                            (unit, p, &reg_value));
                }
                break;

            case DRV_QOS_CTL_WDRR_GRANULARTTY:
            case DRV_QOS_CTL_WDRR_TXQEMPTY:
            case DRV_QOS_CTL_WDRR_NEGCREDIT_CLR:
            case DRV_QOS_CTL_WDRR_BURSTMODE:

                if (IS_CPU_PORT(unit, p)) {
                    SOC_IF_ERROR_RETURN(REG_READ_IMP_QOS_PRI_CTLr
                            (unit, &reg_value));
                } else if (p == 7) {
                    SOC_IF_ERROR_RETURN(REG_READ_P7_QOS_PRI_CTLr
                            (unit, &reg_value));
                } else {
                    SOC_IF_ERROR_RETURN(REG_READ_PN_QOS_PRI_CTLr
                            (unit, p, &reg_value));
                }

                if (type == DRV_QOS_CTL_WDRR_GRANULARTTY) {
                    /* 0 for WDRR; 1 for WRR */
                    temp = (state) ? 1 : 0;  
                    SOC_IF_ERROR_RETURN(soc_PN_QOS_PRI_CTLr_field_set
                        (unit, &reg_value, WDRR_GRANULARITYf, &temp));

                } else if (type == DRV_QOS_CTL_WDRR_TXQEMPTY) {
                    /* 0 for TXQ; 1 for Queue shaper */
                    temp = (state == DRV_COSQ_SCHEDULER_EMPTY_TX_QUEUE) ? 0 : 1;
                    SOC_IF_ERROR_RETURN(soc_PN_QOS_PRI_CTLr_field_set
                        (unit, &reg_value, TXQ_EMPTY_STATUS_SELECTf, &temp));

                } else if (type == DRV_QOS_CTL_WDRR_NEGCREDIT_CLR) {
                    /* 0 for Enabling Clear; 1 for disabling Clear */
                    temp = (state) ? 0 : 1;  
                    SOC_IF_ERROR_RETURN(soc_PN_QOS_PRI_CTLr_field_set
                        (unit, &reg_value, NEGATIVE_CREDIT_CLR_DISABLEf, &temp));

                } else if (type == DRV_QOS_CTL_WDRR_BURSTMODE) {
                    /* 0 for Non-Burst Mode; 1 for Burst Mode */
                    temp = (state) ? 1 : 0;  
                    SOC_IF_ERROR_RETURN(soc_PN_QOS_PRI_CTLr_field_set
                        (unit, &reg_value, ROUNDROBIN_BURST_MODE_ENABLEf, &temp));

                }
        
                if (IS_CPU_PORT(unit, p)) {
                    SOC_IF_ERROR_RETURN(REG_WRITE_IMP_QOS_PRI_CTLr
                            (unit, &reg_value));
                } else if (p == 7) {
                    SOC_IF_ERROR_RETURN(REG_WRITE_P7_QOS_PRI_CTLr
                            (unit, &reg_value));
                } else {
                    SOC_IF_ERROR_RETURN(REG_WRITE_PN_QOS_PRI_CTLr
                            (unit, p, &reg_value));
                }
                break;
    
            default:
                return SOC_E_UNAVAIL;
        }   /* switch (type) */

    }   /* PBMP_ITER */

    return SOC_E_NONE;
}

/*
 *  Function : drv_sf3_queue_qos_control_get
 *
 *  Purpose :
 *      Get the qos control.
 *
 *  Parameters :
 *      unit         :  RoboSwitch unit number.
 *      port    :  port number
 *      type    :  qos control type.
 *      state        :  (OUT) The state of the selected type.
 *
 *  Return :
 *      SOC_E_XXX.
 *
 *  Note :
 */ 
int 
drv_sf3_queue_qos_control_get(int unit, uint32 port, uint32 type, uint32 *state)
{
    uint32  reg_value, temp, in_port;

    /* port validation */
    if ((int)port == -1) {
        /* get port 0 to report system based value. */
        in_port = 0;
    } else {
        /* Check port number */
        if (SOC_PBMP_MEMBER(PBMP_ALL(unit), port)){
            in_port = port;
        } else {
            return SOC_E_PARAM;
        }
    }

    switch (type) {
            case DRV_QOS_CTL_FLOOD_DROP_TCMAP:
                if (IS_CPU_PORT(unit, in_port)) {
                    SOC_IF_ERROR_RETURN(REG_READ_IMP_TC2COS_MAPr
                            (unit, &reg_value));
                } else if (in_port == 7) {
                    SOC_IF_ERROR_RETURN(REG_READ_P7_TC2COS_MAPr
                            (unit, &reg_value));
                } else {
                    SOC_IF_ERROR_RETURN(REG_READ_PN_TC2COS_MAPr
                            (unit, in_port, &reg_value));
                }

                SOC_IF_ERROR_RETURN(soc_PN_TC2COS_MAPr_field_get
                        (unit, &reg_value, BCAST_DLF_DROP_TCf, &temp));
                *state = temp;  
                break;

        case DRV_QOS_CTL_WDRR_GRANULARTTY:
        case DRV_QOS_CTL_WDRR_TXQEMPTY:
        case DRV_QOS_CTL_WDRR_NEGCREDIT_CLR:
        case DRV_QOS_CTL_WDRR_BURSTMODE:

            if (IS_CPU_PORT(unit, in_port)) {
                SOC_IF_ERROR_RETURN(REG_READ_IMP_QOS_PRI_CTLr
                        (unit, &reg_value));
            } else if (in_port == 7) {
                SOC_IF_ERROR_RETURN(REG_READ_P7_QOS_PRI_CTLr
                        (unit, &reg_value));
            } else {
                SOC_IF_ERROR_RETURN(REG_READ_PN_QOS_PRI_CTLr
                        (unit, in_port, &reg_value));
            }

            if (type == DRV_QOS_CTL_WDRR_GRANULARTTY) {
                SOC_IF_ERROR_RETURN(soc_PN_QOS_PRI_CTLr_field_get
                    (unit, &reg_value, WDRR_GRANULARITYf, &temp));
                /* 0 for WDRR; 1 for WRR */
                *state = (temp) ? 1 : 0; 

            } else if (type == DRV_QOS_CTL_WDRR_TXQEMPTY) {
                SOC_IF_ERROR_RETURN(soc_PN_QOS_PRI_CTLr_field_get
                    (unit, &reg_value, TXQ_EMPTY_STATUS_SELECTf, &temp));
                /* 0 for TXQ; 1 for Queue shaper */
                *state = (temp == 0) ? DRV_COSQ_SCHEDULER_EMPTY_TX_QUEUE : 
                        DRV_COSQ_SCHEDULER_EMPTY_TXQ_SHAPER;

            } else if (type == DRV_QOS_CTL_WDRR_NEGCREDIT_CLR) {
                SOC_IF_ERROR_RETURN(soc_PN_QOS_PRI_CTLr_field_get
                    (unit, &reg_value, NEGATIVE_CREDIT_CLR_DISABLEf, &temp));
                /* 0 for Enabling Clear; 1 for disabling Clear */
                *state = (temp) ? 0 : 1;  

            } else if (type == DRV_QOS_CTL_WDRR_BURSTMODE) {
                SOC_IF_ERROR_RETURN(soc_PN_QOS_PRI_CTLr_field_get
                    (unit, &reg_value, ROUNDROBIN_BURST_MODE_ENABLEf, &temp));
                /* 0 for Non-Burst Mode; 1 for Burst Mode */
                *state = temp;
            }
            break;

        default:
            return SOC_E_UNAVAIL;
    }

    LOG_VERBOSE(BSL_LS_SOC_COMMON, \
                (BSL_META_U(unit, \
                            "%s: \
                            unit %d, port = %d, type = 0x%x, state = 0x%x\n"), 
                 FUNCTION_NAME(), unit, port, type, *state));

    return SOC_E_NONE;
}



