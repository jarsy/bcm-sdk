/*
 * $Id: $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include <shared/bsl.h>

#include <soc/drv.h>
#include <soc/debug.h>

#define NORTHSTAR_QUEUE_DSCP2TC  0x0
#define NORTHSTAR_QUEUE_PCP2TC   0x1
#define NORTHSTAR_QUEUE_DA2TC    0x2
#define NORTHSTAR_QUEUE_PID2TC   0x3

#define NORTHSTAR_QUEUE_LEVEL1_ALL_WRR     0x0
#define NORTHSTAR_QUEUE_LEVEL1_1_STRICT    0x1
#define NORTHSTAR_QUEUE_LEVEL1_2_STRICT    0x2
#define NORTHSTAR_QUEUE_LEVEL1_ALL_STRICT  0x3

#define NORTHSTAR_QUEUE_LEVEL2_ALL_WRR     0x0
#define NORTHSTAR_QUEUE_LEVEL2_1_STRICT    0x1
#define NORTHSTAR_QUEUE_LEVEL2_ALL_STRICT  0x2

/*
 *  Function : drv_northstar_queue_port_prio_to_queue_set
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
drv_northstar_queue_port_prio_to_queue_set(int unit, uint8 port, 
    uint8 prio, uint8 queue_n)
{
    uint32  reg_value, temp, additional_soc_port_num;
    /* Using 32b port variable to compare against SOC_MAX_NUM_PORTS,
       which may be bigger the uint8 and then reuslts in a compilation error */
    uint32  port32 = port;

    LOG_VERBOSE(BSL_LS_SOC_COMMON, \
                (BSL_META_U(unit, \
                            "drv_northstar_queue_port_prio_to_queue_set: \
                            unit %d, port = %d, prio = %d, queue_n = %d\n"), 
                 unit, port, prio, queue_n));

    /* Check port number */
    if (port32 > (SOC_MAX_NUM_PORTS - 1)) {
         return SOC_E_PARAM;
    }

    SOC_IF_ERROR_RETURN(DRV_DEV_PROP_GET
        (unit, DRV_DEV_PROP_ADDITIONAL_SOC_PORT_NUM, &additional_soc_port_num));

    if (IS_GE_PORT(unit, port) && (port == additional_soc_port_num)) {
        SOC_IF_ERROR_RETURN(REG_READ_P7_TC2COS_MAPr
            (unit, &reg_value));
    } else if (IS_GE_PORT(unit, port)) {
        SOC_IF_ERROR_RETURN(REG_READ_PN_TC2COS_MAPr
            (unit, port, &reg_value));
    } else {
        SOC_IF_ERROR_RETURN(REG_READ_IMP_TC2COS_MAPr
            (unit, &reg_value));
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

    if (IS_GE_PORT(unit, port) && (port == additional_soc_port_num)) {
        SOC_IF_ERROR_RETURN(REG_WRITE_P7_TC2COS_MAPr
            (unit, &reg_value));
    } else if (IS_GE_PORT(unit, port)) {
        SOC_IF_ERROR_RETURN(REG_WRITE_PN_TC2COS_MAPr
            (unit, port, &reg_value));
    } else {
        SOC_IF_ERROR_RETURN(REG_WRITE_IMP_TC2COS_MAPr
            (unit, &reg_value));
    }

     return SOC_E_NONE;
}

/*
 *  Function : drv_northstar_queue_port_prio_to_queue_get
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
drv_northstar_queue_port_prio_to_queue_get(int unit, uint8 port, 
    uint8 prio, uint8 *queue_n)
{
    uint32  reg_value, temp, additional_soc_port_num;
    /* Using 32b port variable to compare against SOC_MAX_NUM_PORTS,
       which may be bigger the uint8 and then reuslts in a compilation error */
    uint32  port32 = port;

    /* Check port number */
    if (port32 > (SOC_MAX_NUM_PORTS - 1)) {
         return SOC_E_PARAM;
    }

    SOC_IF_ERROR_RETURN(DRV_DEV_PROP_GET
        (unit, DRV_DEV_PROP_ADDITIONAL_SOC_PORT_NUM, &additional_soc_port_num));

    if (IS_GE_PORT(unit, port) && (port == additional_soc_port_num)) {
        SOC_IF_ERROR_RETURN(REG_READ_P7_TC2COS_MAPr
            (unit, &reg_value));
    } else if (IS_GE_PORT(unit, port)) {
        SOC_IF_ERROR_RETURN(REG_READ_PN_TC2COS_MAPr
            (unit, port, &reg_value));
    } else {
        SOC_IF_ERROR_RETURN(REG_READ_IMP_TC2COS_MAPr
            (unit, &reg_value));
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

    LOG_VERBOSE(BSL_LS_SOC_COMMON, \
                (BSL_META_U(unit, \
                            "drv_northstar_queue_port_prio_to_queue_get: \
                            unit %d, port = %d, prio = %d, *queue_n = %d\n"), 
                 unit, port, prio, *queue_n));

     return SOC_E_NONE;
}

/*
 *  Function : drv_northstar_queue_mode_set
 *
 *  Purpose :
 *      Set the queue mode of selected port type.
 *
 *  Parameters :
 *      unit     :  RoboSwitch unit number.
 *      bmp      :  port bitmap.
 *      flag     :  may include: DRV_QUEUE_FLAG_LEVLE2
 *      mode     :  queue mode.
 *
 *  Return :
 *      SOC_E_XXX.
 *
 *  Note :
 *      
 *      Q5 -------------------------> -------
 *      Q4 -------------------------> |SCH1  |
 *      Q3 --->  -------                               |      |
 *      Q2 --->  |SCH2 |---Q6-------> -------- 
 *      Q1 --->  |     |
 *      Q0 --->  ------- 
 *
 *      PS. SCH1 use LEVEL2_QOS_PRI_CTL register and SCH1 use LEVEL2_QOS_PRI_CTL 
 *           There are 3 COS queues for SCH1. They are COSQ4, COSQ5 and 
 *                             COSQ6 (the output of SCH1). 
 *           There are 4 COS queues for SCH2. They are COSQ0 ~ COSQ3.
 *
 */
int 
drv_northstar_queue_mode_set(int unit, soc_pbmp_t bmp, uint32 flag, 
    uint32 mode)
{
    uint32  reg_value, temp, p, additional_soc_port_num;
    
    LOG_VERBOSE(BSL_LS_SOC_COMMON, \
                (BSL_META_U(unit, \
                            "drv_northstar_queue_mode_set: \
                            unit %d, bmp = 0x%x, flag = 0x%x, queue mode = %d\n"),
                 unit, SOC_PBMP_WORD_GET(bmp,0), flag, mode));

    SOC_IF_ERROR_RETURN(DRV_DEV_PROP_GET
        (unit, DRV_DEV_PROP_ADDITIONAL_SOC_PORT_NUM, &additional_soc_port_num));

    PBMP_ITER(bmp, p) {
        if (IS_CPU_PORT(unit, p)) {
            SOC_IF_ERROR_RETURN(REG_READ_IMP_QOS_PRI_CTLr
                (unit, &reg_value));
        } else if (p == additional_soc_port_num) {
            SOC_IF_ERROR_RETURN(REG_READ_P7_QOS_PRI_CTLr
                (unit, &reg_value));
        } else {
            SOC_IF_ERROR_RETURN(REG_READ_PN_QOS_PRI_CTLr
                (unit, p, &reg_value));
        }

        if (flag & DRV_QUEUE_FLAG_LEVLE2) {
            switch (mode) {
                case DRV_QUEUE_MODE_WRR:
                    temp = NORTHSTAR_QUEUE_LEVEL1_ALL_WRR;
                    break;
                case DRV_QUEUE_MODE_1STRICT:
                    temp = NORTHSTAR_QUEUE_LEVEL1_1_STRICT;
                    break;
                case DRV_QUEUE_MODE_2STRICT:
                    temp = NORTHSTAR_QUEUE_LEVEL1_2_STRICT;
                    break;
                case DRV_QUEUE_MODE_STRICT:
                    temp = NORTHSTAR_QUEUE_LEVEL1_ALL_STRICT;
                    break;                
                default:
                    return SOC_E_UNAVAIL;
            }
            SOC_IF_ERROR_RETURN(soc_PN_QOS_PRI_CTLr_field_set
                (unit, &reg_value, LEVEL1_QOS_PRI_CTLf, &temp));
        } else {
            switch (mode) {
                case DRV_QUEUE_MODE_WRR:
                    temp = NORTHSTAR_QUEUE_LEVEL2_ALL_WRR;
                    break;
                case DRV_QUEUE_MODE_1STRICT:
                    temp = NORTHSTAR_QUEUE_LEVEL2_1_STRICT;
                    break;
                case DRV_QUEUE_MODE_STRICT:
                    temp = NORTHSTAR_QUEUE_LEVEL2_ALL_STRICT;
                    break;                
                default:
                    return SOC_E_UNAVAIL;
            }
            SOC_IF_ERROR_RETURN(soc_PN_QOS_PRI_CTLr_field_set
                (unit, &reg_value, LEVEL2_QOS_PRI_CTLf, &temp));
        }
        if (IS_CPU_PORT(unit, p)) {
            SOC_IF_ERROR_RETURN(REG_WRITE_IMP_QOS_PRI_CTLr
                (unit, &reg_value));
        } else if (p == additional_soc_port_num) {
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
 *  Function : drv_northstar_queue_mode_get
 *
 *  Purpose :
 *      Get the queue mode of selected port type.
 *
 *  Parameters :
 *      unit     :  RoboSwitch unit number.
 *      port     :  port number.
 *      flag     :  may include: DRV_QUEUE_FLAG_LEVLE2
 *      mode     :  (OUT) queue mode.
 *
 *  Return :
 *      SOC_E_XXX.
 *
 *  Note :
 */  
int 
drv_northstar_queue_mode_get(int unit, uint32 port, uint32 flag, 
    uint32 *mode)
{
    uint32  reg_value, temp2 = 0, temp1 = 0, additional_soc_port_num;

    SOC_IF_ERROR_RETURN(DRV_DEV_PROP_GET
        (unit, DRV_DEV_PROP_ADDITIONAL_SOC_PORT_NUM, &additional_soc_port_num));

    if (IS_CPU_PORT(unit, port)) {
        SOC_IF_ERROR_RETURN(REG_READ_IMP_QOS_PRI_CTLr
            (unit, &reg_value));
    } else if (port == additional_soc_port_num) {
        SOC_IF_ERROR_RETURN(REG_READ_P7_QOS_PRI_CTLr
            (unit, &reg_value));
    } else {
        SOC_IF_ERROR_RETURN(REG_READ_PN_QOS_PRI_CTLr
            (unit, port, &reg_value));
    }

    if (flag & DRV_QUEUE_FLAG_LEVLE2) {
        SOC_IF_ERROR_RETURN(soc_PN_QOS_PRI_CTLr_field_get
            (unit, &reg_value, LEVEL1_QOS_PRI_CTLf, &temp1));
    } else {
        SOC_IF_ERROR_RETURN(soc_PN_QOS_PRI_CTLr_field_get
            (unit, &reg_value, LEVEL2_QOS_PRI_CTLf, &temp2));
    }

    if(((flag & DRV_QUEUE_FLAG_LEVLE2) && 
         (temp1 == NORTHSTAR_QUEUE_LEVEL1_ALL_STRICT)) || 
         ((!flag) & (temp2 == NORTHSTAR_QUEUE_LEVEL2_ALL_STRICT))) {
        *mode = DRV_QUEUE_MODE_STRICT;
    } else {
        /* BCM API don't care which type of WRR mode */
        *mode = DRV_QUEUE_MODE_WRR;
    }

    LOG_VERBOSE(BSL_LS_SOC_COMMON, \
                (BSL_META_U(unit, \
                            "drv_northstar_queue_mode_get: \
                            unit %d, port = %d, flag = 0x%x, queue mode = %d\n"),
                 unit, port, flag, *mode));    

    return SOC_E_NONE;
}

/*
 *  Function : drv_northstar_queue_WRR_weight_set
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
 *      Q5 -------------------------> -------
 *      Q4 -------------------------> |SCH1  |
 *      Q3 --->  -------                               |      |
 *      Q2 --->  |SCH2 |---Q6-------> -------- 
 *      Q1 --->  |     |
 *      Q0 --->  ------- 
 *
 *      PS. SCH1 use LEVEL2_QOS_PRI_CTL register and SCH1 use LEVEL2_QOS_PRI_CTL 
 *           There are 3 COS queues for SCH1. They are COSQ4, COSQ5 and 
 *                             COSQ6 (the output of SCH1). 
 *           There are 4 COS queues for SCH2. They are COSQ0 ~ COSQ3.
 *
 */
int 
drv_northstar_queue_WRR_weight_set(int unit, uint32 port_type, 
    soc_pbmp_t bmp, uint8 queue, uint32 weight)
{
    uint32  reg_value, p;
    uint32  max_weight = 0, additional_soc_port_num;

    LOG_VERBOSE(BSL_LS_SOC_COMMON, \
                (BSL_META_U(unit, \
                            "drv_northstar_queue_WRR_weight_set: \
                            unit %d, port type = %d, bmp = 0x%x, queue = %d, weight = %d\n"),
                 unit, port_type, SOC_PBMP_WORD_GET(bmp,0), queue, weight));

    SOC_IF_ERROR_RETURN(DRV_DEV_PROP_GET
        (unit, DRV_DEV_PROP_COSQ_MAX_WEIGHT_VALUE, &max_weight));


    SOC_IF_ERROR_RETURN(DRV_DEV_PROP_GET
        (unit, DRV_DEV_PROP_ADDITIONAL_SOC_PORT_NUM, &additional_soc_port_num));

    if ((weight > max_weight) || (weight < 1)) {
        return SOC_E_PARAM;
    }

    PBMP_ITER(bmp, p) {
        if (queue > 3) {
            if (IS_CPU_PORT(unit, p)) {
                SOC_IF_ERROR_RETURN(REG_READ_IMP_LEVEL2_QOS_WEIGHTr
                    (unit, &reg_value));
            } else if (p == additional_soc_port_num) {
                SOC_IF_ERROR_RETURN(REG_READ_P7_LEVEL2_QOS_WEIGHTr
                    (unit, &reg_value));
            } else {
                SOC_IF_ERROR_RETURN(REG_READ_PN_LEVEL2_QOS_WEIGHTr
                    (unit, p, &reg_value));
            }

            if(queue == 6) {
                SOC_IF_ERROR_RETURN(soc_PN_LEVEL2_QOS_WEIGHTr_field_set
                    (unit, &reg_value, LEVEL1_OUTPUT_WEIGHTf, &weight));
            } else if (queue == 4) {
                SOC_IF_ERROR_RETURN(soc_PN_LEVEL2_QOS_WEIGHTr_field_set
                    (unit, &reg_value, Q4_WEIGHTf, &weight));
            } else if (queue == 5) {
                SOC_IF_ERROR_RETURN(soc_PN_LEVEL2_QOS_WEIGHTr_field_set
                    (unit, &reg_value, Q5_WEIGHTf, &weight));
            } else {
                return SOC_E_PARAM;
            }

            if (IS_CPU_PORT(unit, p)) {
                SOC_IF_ERROR_RETURN(REG_WRITE_IMP_LEVEL2_QOS_WEIGHTr
                    (unit, &reg_value));
            } else if (p == additional_soc_port_num) {
                SOC_IF_ERROR_RETURN(REG_WRITE_P7_LEVEL2_QOS_WEIGHTr
                    (unit, &reg_value));
            } else {
                SOC_IF_ERROR_RETURN(REG_WRITE_PN_LEVEL2_QOS_WEIGHTr
                    (unit, p, &reg_value));
            }
        } else {
            if (IS_CPU_PORT(unit, p)) {
                SOC_IF_ERROR_RETURN(REG_READ_IMP_LEVEL1_QOS_WEIGHTr
                    (unit, &reg_value));
            } else if (p == additional_soc_port_num) {
                SOC_IF_ERROR_RETURN(REG_READ_P7_LEVEL1_QOS_WEIGHTr
                    (unit, &reg_value));
            } else {
                SOC_IF_ERROR_RETURN(REG_READ_PN_LEVEL1_QOS_WEIGHTr
                    (unit, p, &reg_value));
            }

            if(queue == 0) {
                SOC_IF_ERROR_RETURN(soc_PN_LEVEL1_QOS_WEIGHTr_field_set
                    (unit, &reg_value, Q0_WEIGHTf, &weight));
            } else if (queue == 1) {
                SOC_IF_ERROR_RETURN(soc_PN_LEVEL1_QOS_WEIGHTr_field_set
                    (unit, &reg_value, Q1_WEIGHTf, &weight));
            } else if (queue == 2) {
                SOC_IF_ERROR_RETURN(soc_PN_LEVEL1_QOS_WEIGHTr_field_set
                    (unit, &reg_value, Q2_WEIGHTf, &weight));
            } else {
                SOC_IF_ERROR_RETURN(soc_PN_LEVEL1_QOS_WEIGHTr_field_set
                    (unit, &reg_value, Q3_WEIGHTf, &weight));
            }

            if (IS_CPU_PORT(unit, p)) {
                SOC_IF_ERROR_RETURN(REG_WRITE_IMP_LEVEL1_QOS_WEIGHTr
                    (unit, &reg_value));
            } else if (p == additional_soc_port_num) {
                SOC_IF_ERROR_RETURN(REG_WRITE_P7_LEVEL1_QOS_WEIGHTr
                    (unit, &reg_value));
            } else {
                SOC_IF_ERROR_RETURN(REG_WRITE_PN_LEVEL1_QOS_WEIGHTr
                    (unit, p, &reg_value));
            }
        }
    }       

    return SOC_E_NONE;
}

/*
 *  Function : drv_northstar_queue_WRR_weight_get
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
drv_northstar_queue_WRR_weight_get(int unit, uint32 port_type, 
    uint32 port, uint8 queue, uint32 *weight)
{
    uint32  reg_value, additional_soc_port_num;

    SOC_IF_ERROR_RETURN(DRV_DEV_PROP_GET
        (unit, DRV_DEV_PROP_ADDITIONAL_SOC_PORT_NUM, &additional_soc_port_num));

    if (queue > 3) {
        if (IS_CPU_PORT(unit, port)) {
            SOC_IF_ERROR_RETURN(REG_READ_IMP_LEVEL2_QOS_WEIGHTr
                (unit, &reg_value));
        } else if (port == additional_soc_port_num) {
            SOC_IF_ERROR_RETURN(REG_READ_P7_LEVEL2_QOS_WEIGHTr
                (unit, &reg_value));
        } else {
            SOC_IF_ERROR_RETURN(REG_READ_PN_LEVEL2_QOS_WEIGHTr
                (unit, port, &reg_value));
        }

        if(queue == 6) {
            SOC_IF_ERROR_RETURN(soc_PN_LEVEL2_QOS_WEIGHTr_field_get
                (unit, &reg_value, LEVEL1_OUTPUT_WEIGHTf, weight));
        } else if (queue == 4) {
            SOC_IF_ERROR_RETURN(soc_PN_LEVEL2_QOS_WEIGHTr_field_get
                (unit, &reg_value, Q4_WEIGHTf, weight));
        } else if (queue == 5) {
            SOC_IF_ERROR_RETURN(soc_PN_LEVEL2_QOS_WEIGHTr_field_get
                (unit, &reg_value, Q5_WEIGHTf, weight));
        } else {
            return SOC_E_PARAM;
        }
    } else {
        if (IS_CPU_PORT(unit, port)) {
            SOC_IF_ERROR_RETURN(REG_READ_IMP_LEVEL1_QOS_WEIGHTr
                (unit, &reg_value));
        } else if (port == additional_soc_port_num) {
            SOC_IF_ERROR_RETURN(REG_READ_P7_LEVEL1_QOS_WEIGHTr
                (unit, &reg_value));
        } else {
            SOC_IF_ERROR_RETURN(REG_READ_PN_LEVEL1_QOS_WEIGHTr
                (unit, port, &reg_value));
        }

        if(queue == 0) {
            SOC_IF_ERROR_RETURN(soc_PN_LEVEL1_QOS_WEIGHTr_field_get
                (unit, &reg_value, Q0_WEIGHTf, weight));
        } else if (queue == 1) {
            SOC_IF_ERROR_RETURN(soc_PN_LEVEL1_QOS_WEIGHTr_field_get
                (unit, &reg_value, Q1_WEIGHTf, weight));
        } else if (queue == 2) {
            SOC_IF_ERROR_RETURN(soc_PN_LEVEL1_QOS_WEIGHTr_field_get
                (unit, &reg_value, Q2_WEIGHTf, weight));
        } else {
            SOC_IF_ERROR_RETURN(soc_PN_LEVEL1_QOS_WEIGHTr_field_get
                (unit, &reg_value, Q3_WEIGHTf, weight));
        }
    }

    LOG_VERBOSE(BSL_LS_SOC_COMMON, \
                (BSL_META_U(unit, \
                            "drv_northstar_queue_WRR_weight_get: \
                            unit %d, port type = %d, port = %d, queue = %d, weight = %d\n"),
                 unit, port_type, port, queue, *weight));
    
    return SOC_E_NONE;
}

/* Config output queue mapping */
/*
 *  Function : drv_northstar_queue_prio_set
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
drv_northstar_queue_prio_set(int unit, uint32 port, uint8 prio, 
    uint8 queue_n)
{
    uint32  p;
    soc_pbmp_t  pbmp;

    LOG_VERBOSE(BSL_LS_SOC_COMMON, \
                (BSL_META_U(unit, \
                            "drv_northstar_queue_prio_set: \
                            unit %d, port = %d, priority = %d, queue = %d\n"),
                 unit, port, prio, queue_n));

    SOC_IF_ERROR_RETURN(DRV_QUEUE_PRIO_REMAP_SET
        (unit, port, prio, prio));

    SOC_PBMP_CLEAR(pbmp);
    if (port == -1) {
        SOC_PBMP_ASSIGN(pbmp, PBMP_ALL(unit));
    } else {
        SOC_PBMP_PORT_SET(pbmp, port);
    }

    PBMP_ITER(pbmp, p) {
        SOC_IF_ERROR_RETURN(DRV_QUEUE_PORT_PRIO_TO_QUEUE_SET
            (unit, p, prio, queue_n));
    }

    return SOC_E_NONE;
}

/*
 *  Function : drv_northstar_queue_prio_get
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
drv_northstar_queue_prio_get(int unit, uint32 port, uint8 prio, 
    uint8 *queue_n)
{
    uint32  p;
    uint8   new_prio = 0;

    if (port == -1) {
        /* Get GE0's PN_PCP2TC register value */
        p = 0;
    } else {
        p = port;
    }

    /* Get the new_prio value from PCP2TC mapping by per-port based */
    SOC_IF_ERROR_RETURN(DRV_QUEUE_PRIO_REMAP_GET
        (unit, p, prio, &new_prio));
    
    /* Transfer the new_prio by TC2COS mapping (per-system based) */
    SOC_IF_ERROR_RETURN(DRV_QUEUE_PORT_PRIO_TO_QUEUE_GET
        (unit, p, new_prio, queue_n));

    LOG_VERBOSE(BSL_LS_SOC_COMMON, \
                (BSL_META_U(unit, \
                            "drv_northstar_queue_prio_get: \
                            unit %d, port = %d, priority = %d, queue = %d\n"), 
                 unit, port, prio, *queue_n));

     return SOC_E_NONE;
}

/*
 *  Function : drv_northstar_queue_mapping_type_set
 *
 *  Purpose :
 *      Set the state to the specific queue mapping type.
 *
 *  Parameters :
 *      unit         :  RoboSwitch unit number.
 *      bmp          :  port bitmap.
 *      mapping_type :  queue mapping type (prio/diffserv).
 *      state        :  The state of the selected mapping type.
 *
 *  Return :
 *      SOC_E_XXX.
 *
 *  Note :
 */
int 
drv_northstar_queue_mapping_type_set(int unit, soc_pbmp_t bmp, 
    uint32 mapping_type, uint8 state)
{
    uint32	reg_value;
    uint32	temp, p, temp1, additional_soc_port_num;

    SOC_IF_ERROR_RETURN(DRV_DEV_PROP_GET
        (unit, DRV_DEV_PROP_ADDITIONAL_SOC_PORT_NUM, &additional_soc_port_num));

    LOG_VERBOSE(BSL_LS_SOC_COMMON, \
                (BSL_META_U(unit, \
                            "drv_northstar_queue_mapping_type_set: \
                            unit %d, bmp = 0x%x, type = %d, %sable\n"),
                 unit, SOC_PBMP_WORD_GET(bmp, 0), mapping_type, (state) ? "en" : "dis"));

    switch (mapping_type) {
        case DRV_QUEUE_MAP_NONE:
            /* Set TC_SEL_0 ~ TC_SEL_3 to PID2TC  and TC_SEL_0 ~ TC_SEL_3 to DA2TC */
            PBMP_ITER(bmp, p) {
                if (IS_CPU_PORT(unit, p)) {
                    SOC_IF_ERROR_RETURN(REG_READ_IMP_TC_SEL_TABLEr
                        (unit, &reg_value));
                } else if (p == additional_soc_port_num) {
                    SOC_IF_ERROR_RETURN(REG_READ_P7_TC_SEL_TABLEr
                        (unit, &reg_value));
                } else {
                    SOC_IF_ERROR_RETURN(REG_READ_TC_SEL_TABLEr
                        (unit, p, &reg_value));
                }

                temp = NORTHSTAR_QUEUE_PID2TC;
                SOC_IF_ERROR_RETURN(soc_TC_SEL_TABLEr_field_set
                    (unit, &reg_value, TC_SEL_0f, &temp));
                SOC_IF_ERROR_RETURN(soc_TC_SEL_TABLEr_field_set
                    (unit, &reg_value, TC_SEL_1f, &temp));
                SOC_IF_ERROR_RETURN(soc_TC_SEL_TABLEr_field_set
                    (unit, &reg_value, TC_SEL_2f, &temp));
                SOC_IF_ERROR_RETURN(soc_TC_SEL_TABLEr_field_set
                    (unit, &reg_value, TC_SEL_3f, &temp));

                temp = NORTHSTAR_QUEUE_DA2TC;
                SOC_IF_ERROR_RETURN(soc_TC_SEL_TABLEr_field_set
                    (unit, &reg_value, TC_SEL_4f, &temp));
                SOC_IF_ERROR_RETURN(soc_TC_SEL_TABLEr_field_set
                    (unit, &reg_value, TC_SEL_5f, &temp));
                SOC_IF_ERROR_RETURN(soc_TC_SEL_TABLEr_field_set
                    (unit, &reg_value, TC_SEL_6f, &temp));
                SOC_IF_ERROR_RETURN(soc_TC_SEL_TABLEr_field_set
                    (unit, &reg_value, TC_SEL_7f, &temp));            
                if (IS_CPU_PORT(unit, p)) {
                    SOC_IF_ERROR_RETURN(REG_WRITE_IMP_TC_SEL_TABLEr
                        (unit, &reg_value));
                } else if (p == additional_soc_port_num) {
                    SOC_IF_ERROR_RETURN(REG_WRITE_P7_TC_SEL_TABLEr
                        (unit, &reg_value));
                } else {
                    SOC_IF_ERROR_RETURN(REG_WRITE_TC_SEL_TABLEr
                        (unit, p, &reg_value));
                }
            }
            /* Disable QOS_IP_EN register */
            SOC_IF_ERROR_RETURN(REG_READ_QOS_1P_ENr
                (unit, &reg_value));
            SOC_IF_ERROR_RETURN(soc_QOS_1P_ENr_field_get
                (unit, &reg_value, QOS_1P_ENf, &temp));
            temp = SOC_PBMP_WORD_GET(bmp, 0);
            SOC_IF_ERROR_RETURN(soc_QOS_1P_ENr_field_set
                (unit, &reg_value, QOS_1P_ENf, &temp));            
            SOC_IF_ERROR_RETURN(REG_WRITE_QOS_1P_ENr
                (unit, &reg_value));
    
            /* Disable QOS_TOS_DIF_EN register */
            SOC_IF_ERROR_RETURN(REG_READ_QOS_EN_DIFFSERVr
                (unit, &reg_value));
            SOC_IF_ERROR_RETURN(soc_QOS_EN_DIFFSERVr_field_get
                (unit, &reg_value, QOS_EN_DIFFSERVf, &temp));
            temp = SOC_PBMP_WORD_GET(bmp, 0);
            SOC_IF_ERROR_RETURN(soc_QOS_EN_DIFFSERVr_field_set
                (unit, &reg_value, QOS_EN_DIFFSERVf, &temp));
            SOC_IF_ERROR_RETURN(REG_WRITE_QOS_EN_DIFFSERVr
                (unit, &reg_value));
            break;
        case DRV_QUEUE_MAP_PRIO:
            /* Set TC_SEL_0 ~ TC_SEL_3 to PCP2TC for DRV_QUEUE_MAP_PRIO */
            PBMP_ITER(bmp, p) {
                if (IS_CPU_PORT(unit, p)) {
                    SOC_IF_ERROR_RETURN( REG_READ_IMP_TC_SEL_TABLEr
                        (unit, &reg_value));
                } else if (p == additional_soc_port_num) {
                    SOC_IF_ERROR_RETURN(REG_READ_P7_TC_SEL_TABLEr
                        (unit, &reg_value));
                } else {
                    SOC_IF_ERROR_RETURN(REG_READ_TC_SEL_TABLEr
                        (unit, p, &reg_value));
                }
                if (state) {
                    temp = NORTHSTAR_QUEUE_PCP2TC;
                } else {
                    temp = NORTHSTAR_QUEUE_PID2TC;
                }
                SOC_IF_ERROR_RETURN(soc_TC_SEL_TABLEr_field_set
                    (unit, &reg_value, TC_SEL_0f, &temp));
                SOC_IF_ERROR_RETURN(soc_TC_SEL_TABLEr_field_set
                    (unit, &reg_value, TC_SEL_1f, &temp));
                SOC_IF_ERROR_RETURN(soc_TC_SEL_TABLEr_field_set
                    (unit, &reg_value, TC_SEL_2f, &temp));
                SOC_IF_ERROR_RETURN(soc_TC_SEL_TABLEr_field_set
                    (unit, &reg_value, TC_SEL_3f, &temp));
                if (IS_CPU_PORT(unit, p)) {
                    SOC_IF_ERROR_RETURN(REG_WRITE_IMP_TC_SEL_TABLEr
                        (unit, &reg_value));
                } else if (p == additional_soc_port_num) {
                    SOC_IF_ERROR_RETURN(REG_WRITE_P7_TC_SEL_TABLEr
                        (unit, &reg_value));
                } else {
                    SOC_IF_ERROR_RETURN(REG_WRITE_TC_SEL_TABLEr
                        (unit, p, &reg_value));
                }
            }
            break;
        case DRV_QUEUE_MAP_DFSV:
            /* Set TC_SEL_0 and TC_SEL_2 to DSCP2TC for DRV_QUEUE_MAP_DFSV */
            PBMP_ITER(bmp, p) {
                if (IS_CPU_PORT(unit, p)) {
                    SOC_IF_ERROR_RETURN(REG_READ_IMP_TC_SEL_TABLEr
                        (unit, &reg_value));
                } else if (p == additional_soc_port_num) {
                    SOC_IF_ERROR_RETURN(REG_READ_P7_TC_SEL_TABLEr
                        (unit, &reg_value));
                } else {
                    SOC_IF_ERROR_RETURN(REG_READ_TC_SEL_TABLEr
                        (unit, p, &reg_value));
                }
                if (state) {
                    temp = NORTHSTAR_QUEUE_DSCP2TC;
                    temp1 = NORTHSTAR_QUEUE_PCP2TC;
                } else {
                    temp = NORTHSTAR_QUEUE_PID2TC;
                    temp1 = NORTHSTAR_QUEUE_PID2TC;
                }
                SOC_IF_ERROR_RETURN(soc_TC_SEL_TABLEr_field_set
                    (unit, &reg_value, TC_SEL_0f, &temp1));
                SOC_IF_ERROR_RETURN(soc_TC_SEL_TABLEr_field_set
                    (unit, &reg_value, TC_SEL_1f, &temp));
                SOC_IF_ERROR_RETURN(soc_TC_SEL_TABLEr_field_set
                    (unit, &reg_value, TC_SEL_2f, &temp1));
                SOC_IF_ERROR_RETURN(soc_TC_SEL_TABLEr_field_set
                    (unit, &reg_value, TC_SEL_3f, &temp));
                if (IS_CPU_PORT(unit, p)) {
                    SOC_IF_ERROR_RETURN(REG_WRITE_IMP_TC_SEL_TABLEr
                        (unit, &reg_value));
                } else if (p == additional_soc_port_num) {
                    SOC_IF_ERROR_RETURN(REG_WRITE_P7_TC_SEL_TABLEr
                        (unit, &reg_value));
                } else {
                    SOC_IF_ERROR_RETURN(REG_WRITE_TC_SEL_TABLEr
                        (unit, p, &reg_value));
                }
            }
            break;
        case DRV_QUEUE_MAP_PORT:
            /* DRV_QUEUE_MAP_PORT is not needed for Northstar.
             * Becaseu it is port based QoS when both DSCP and 1P are disabled.
             */
        case DRV_QUEUE_MAP_HYBRID:
        case DRV_QUEUE_MAP_TOS:
        case DRV_QUEUE_MAP_MAC:
            return SOC_E_UNAVAIL;
        default :
            return SOC_E_PARAM;
    }

    return SOC_E_NONE;
}

/*
 *  Function : drv_northstar_queue_mapping_type_get
 *
 *  Purpose :
 *      Get the state to the specific queue mapping type.
 *
 *  Parameters :
 *      unit         :  RoboSwitch unit number.
 *      port         :  port number.
 *      mapping_type :  queue mapping type (prio/diffserv).
 *      state        :  (OUT) The state of the selected mapping type.
 *
 *  Return :
 *      SOC_E_XXX.
 *
 *  Note :
 */
int 
drv_northstar_queue_mapping_type_get(int unit, uint32 port, 
    uint32 mapping_type, uint8 *state)
{
    uint32  reg_value;
    uint32  temp, additional_soc_port_num;

    SOC_IF_ERROR_RETURN(DRV_DEV_PROP_GET
        (unit, DRV_DEV_PROP_ADDITIONAL_SOC_PORT_NUM, &additional_soc_port_num));

    switch (mapping_type) {
        case DRV_QUEUE_MAP_NONE:
            return SOC_E_PARAM;
        case DRV_QUEUE_MAP_PRIO:
        case DRV_QUEUE_MAP_DFSV:
            if (IS_CPU_PORT(unit, port)) {
                SOC_IF_ERROR_RETURN(REG_READ_IMP_TC_SEL_TABLEr
                    (unit, &reg_value));
            } else if (port == additional_soc_port_num) {
                SOC_IF_ERROR_RETURN(REG_READ_P7_TC_SEL_TABLEr
                    (unit, &reg_value));
            } else {
                SOC_IF_ERROR_RETURN(REG_READ_TC_SEL_TABLEr
                    (unit, port, &reg_value));
            }
            SOC_IF_ERROR_RETURN(soc_TC_SEL_TABLEr_field_get
                (unit, &reg_value, TC_SEL_1f, &temp));
            if(mapping_type == DRV_QUEUE_MAP_PRIO) {
                if (temp == NORTHSTAR_QUEUE_PCP2TC) {
                    *state = TRUE;
                } else {
                    *state = FALSE;
                }
            } else {
                if (temp == NORTHSTAR_QUEUE_DSCP2TC) {
                    *state = TRUE;
                } else {
                    *state = FALSE;
                }
            }
            break;
        case DRV_QUEUE_MAP_PORT:
            /* DRV_QUEUE_MAP_PORT is not needed for Northstar.
             * Becaseu it is port based QoS when both DSCP and 1P are disabled.
             */
        case DRV_QUEUE_MAP_HYBRID:
        case DRV_QUEUE_MAP_TOS:
        case DRV_QUEUE_MAP_MAC:
            return SOC_E_UNAVAIL;
        default:
            return SOC_E_PARAM;
    }

    LOG_VERBOSE(BSL_LS_SOC_COMMON, \
                (BSL_META_U(unit, \
                            "drv_northstar_queue_mapping_type_get: \
                            unit %d, port = %d, type = %d, %sable\n"),
                 unit, port, mapping_type, (*state) ? "en" : "dis"));

    return SOC_E_NONE;
}

/*
 *  Function : drv_northstar_queue_count_set
 *
 *  Purpose :
 *      Set the number of the queeus.
 *
 *  Parameters :
 *      unit     :  RoboSwitch unit number.
 *      port_type:  port type.
 *      count    :  number of queues.
 *
 *  Return :
 *      SOC_E_NONE  :  success.
 *      SOC_E_PARAM :  parameters error.
 *
 *  Note :
 */
int 
drv_northstar_queue_count_set(int unit, uint32 port_type, uint8 count)
{
    uint32  sch2_num_cosq;

    LOG_VERBOSE(BSL_LS_SOC_COMMON, \
                (BSL_META_U(unit, \
                            "drv_northstar_queue_count_set: \
                            unit %d, port type = %d, queue count = %d\n"), unit, port_type, count));

    SOC_IF_ERROR_RETURN(DRV_DEV_PROP_GET
        (unit, DRV_DEV_PROP_SCH2_NUM_COSQ, &sch2_num_cosq));

    if (count != (uint8)sch2_num_cosq) {
        return SOC_E_PARAM;
    }

    return SOC_E_NONE;
}

/*
 *  Function : drv_northstar_queue_count_get
 *
 *  Purpose :
 *      Get the number of the queeus.
 *
 *  Parameters :
 *      unit     :  RoboSwitch unit number.
 *      port_type:  port type.
 *      count    :  (OUT) number of queues.
 *
 *  Return :
 *      SOC_E_NONE  :  success.
 *
 *  Note :
 */
int 
drv_northstar_queue_count_get(int unit, uint32 port_type, uint8 *count)
{
    uint32  sch2_num_cosq;

    SOC_IF_ERROR_RETURN(DRV_DEV_PROP_GET
        (unit, DRV_DEV_PROP_SCH2_NUM_COSQ, &sch2_num_cosq));

    *count = (uint8) sch2_num_cosq;

    LOG_VERBOSE(BSL_LS_SOC_COMMON, \
                (BSL_META_U(unit, \
                            "drv_northstar_queue_count_get: \
                            unit %d, port type = %d, queue count = %d\n"), unit, port_type, *count));
 
    return SOC_E_NONE;
}

