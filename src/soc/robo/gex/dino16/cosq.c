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
 *  Function : drv_dino16_queue_port_prio_to_queue_set
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
drv_dino16_queue_port_prio_to_queue_set(int unit, uint8 port, uint8 prio, 
    uint8 queue_n)
{
    uint32  reg_value, temp;
    /* Using 32b port variable to compare against SOC_MAX_NUM_PORTS,
       which may be bigger the uint8 and then reuslts in a compilation error */
    uint32  port32 = port;

    LOG_VERBOSE(BSL_LS_SOC_COMMON, \
                (BSL_META_U(unit, \
                            "drv_dino16_queue_port_prio_to_queue_set: \
                            unit %d, port = %d, prio = %d, queue_n = %d\n"), 
                 unit, port, prio, queue_n));

    /* Check port number */
    if (port32 > (SOC_MAX_NUM_PORTS - 1)) {
        return SOC_E_PARAM;
    }

    SOC_IF_ERROR_RETURN(REG_READ_QOS_RX_CTRL_Pr
        (unit, port, &reg_value));

    temp = queue_n;
    switch (prio) {
        case 0:
            soc_QOS_RX_CTRL_Pr_field_set(unit, &reg_value, 
                PRT000_TO_QIDf, &temp);
            break;
        case 1:         
            soc_QOS_RX_CTRL_Pr_field_set(unit, &reg_value, 
                PRT001_TO_QIDf, &temp);
            break;
        case 2:         
            soc_QOS_RX_CTRL_Pr_field_set(unit, &reg_value, 
                PRT010_TO_QIDf, &temp);
            break;
        case 3:
            soc_QOS_RX_CTRL_Pr_field_set(unit, &reg_value, 
                PRT011_TO_QIDf, &temp);
            break;
        case 4:
            soc_QOS_RX_CTRL_Pr_field_set(unit, &reg_value, 
                PRT100_TO_QIDf, &temp);
            break;
        case 5:
            soc_QOS_RX_CTRL_Pr_field_set(unit, &reg_value, 
                PRT101_TO_QIDf, &temp);
            break;
        case 6:         
            soc_QOS_RX_CTRL_Pr_field_set(unit, &reg_value, 
                PRT110_TO_QIDf, &temp);
            break;
        case 7:
            soc_QOS_RX_CTRL_Pr_field_set(unit, &reg_value, 
                PRT111_TO_QIDf, &temp);
            break;
        default:
            return SOC_E_PARAM;
    }        

    SOC_IF_ERROR_RETURN(REG_WRITE_QOS_RX_CTRL_Pr
        (unit, port, &reg_value));

     return SOC_E_NONE;
}

/*
 *  Function : drv_dino16_queue_port_prio_to_queue_get
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
drv_dino16_queue_port_prio_to_queue_get(int unit, uint8 port, uint8 prio, 
    uint8 *queue_n)
{
    uint32  reg_value, temp = 0;
    /* Using 32b port variable to compare against SOC_MAX_NUM_PORTS,
       which may be bigger the uint8 and then reuslts in a compilation error */
    uint32  port32 = port;

    /* Check port number */
    if (port32 > (SOC_MAX_NUM_PORTS - 1)) {
        return SOC_E_PARAM;
    }

    SOC_IF_ERROR_RETURN(REG_READ_QOS_RX_CTRL_Pr
        (unit, port, &reg_value));

    switch (prio) {
       case 0:
            soc_QOS_RX_CTRL_Pr_field_get(unit, &reg_value, 
                PRT000_TO_QIDf, &temp);
            break;
        case 1:         
            soc_QOS_RX_CTRL_Pr_field_get(unit, &reg_value, 
                PRT001_TO_QIDf, &temp);
            break;
        case 2:         
            soc_QOS_RX_CTRL_Pr_field_get(unit, &reg_value, 
                PRT010_TO_QIDf, &temp);
            break;
        case 3:
            soc_QOS_RX_CTRL_Pr_field_get(unit, &reg_value, 
                PRT011_TO_QIDf, &temp);
            break;
        case 4:
            soc_QOS_RX_CTRL_Pr_field_get(unit, &reg_value, 
                PRT100_TO_QIDf, &temp);
            break;
        case 5:
            soc_QOS_RX_CTRL_Pr_field_get(unit, &reg_value, 
                PRT101_TO_QIDf, &temp);
            break;
        case 6:         
            soc_QOS_RX_CTRL_Pr_field_get(unit, &reg_value, 
                PRT110_TO_QIDf, &temp);
            break;
        case 7:
            soc_QOS_RX_CTRL_Pr_field_get(unit, &reg_value, 
                PRT111_TO_QIDf, &temp);
            break;
        default:
            return SOC_E_PARAM;
    }
    *queue_n = temp;

    LOG_VERBOSE(BSL_LS_SOC_COMMON, \
                (BSL_META_U(unit, \
                            "drv_dino16_queue_port_prio_to_queue_get: \
                            unit %d, port = %d, prio = %d, *queue_n = %d\n"), 
                 unit, port, prio, *queue_n));

     return SOC_E_NONE;
}

/*
 *  Function : drv_dino16_queue_mode_set
 *
 *  Purpose :
 *      Set the queue mode of selected port type.
 *
 *  Parameters :
 *      unit        :   RoboSwitch unit number.
 *      bmp   :   port bitmap, not used for dino16.
 *      flag     :  may include: DRV_QUEUE_FLAG_LEVLE2
 *      mode  :   queue mode.
 *
 *  Return :
 *      SOC_E_XXX.
 *
 *  Note :
 */
int 
drv_dino16_queue_mode_set(int unit, soc_pbmp_t bmp, uint32 flag, 
    uint32 mode)
{
    uint32  reg_value, temp;
    
    LOG_VERBOSE(BSL_LS_SOC_COMMON, \
                (BSL_META_U(unit, \
                            "drv_dino16_queue_mode_set: \
                            unit %d, bmp = 0x%x, flag = 0x%x, queue mode = %d\n"),
                 unit, SOC_PBMP_WORD_GET(bmp,0), flag, mode));

    if (flag != 0) {
        return SOC_E_PARAM;
    }

    SOC_IF_ERROR_RETURN(REG_READ_QOS_TX_CTRLr
        (unit, &reg_value));

    switch (mode) {
        case DRV_QUEUE_MODE_STRICT:
            temp = 1;
            break;
        case DRV_QUEUE_MODE_WRR:
            temp = 0;
            break;
        case DRV_QUEUE_MODE_HYBRID:
        default:
            return SOC_E_UNAVAIL;
    }

    soc_QOS_TX_CTRLr_field_set(unit, &reg_value, 
        HQ_PREEMPTf, &temp);
    SOC_IF_ERROR_RETURN(REG_WRITE_QOS_TX_CTRLr
        (unit, &reg_value));
    
    return SOC_E_NONE;
}

/*
 *  Function : drv_dino16_queue_mode_get
 *
 *  Purpose :
 *      Get the queue mode of selected port type.
 *
 *  Parameters :
 *      unit        :   RoboSwitch unit number.
 *      port   :   port number.
 *      flag     :  may include: DRV_QUEUE_FLAG_LEVLE2
 *      mode     :  (OUT) queue mode.
 *
 *  Return :
 *      SOC_E_XXX.
 *
 *  Note :
 */  
int 
drv_dino16_queue_mode_get(int unit, uint32 port, uint32 flag, 
    uint32 *mode)
{
    uint32  reg_value, temp = 0;

    SOC_IF_ERROR_RETURN(REG_READ_QOS_TX_CTRLr
        (unit, &reg_value));
    soc_QOS_TX_CTRLr_field_get(unit, &reg_value, 
        HQ_PREEMPTf, &temp);
    
    if (temp) {
        *mode = DRV_QUEUE_MODE_STRICT;

    } else {
        *mode = DRV_QUEUE_MODE_WRR;
    }
    
    LOG_VERBOSE(BSL_LS_SOC_COMMON, \
                (BSL_META_U(unit, \
                            "drv_dino16_queue_mode_get: \
                            unit %d, port = %d, flag = 0x%x, queue mode = %d\n"),
                 unit, port, flag, *mode));    

    return SOC_E_NONE;
}

/*
 *  Function : drv_dino16_queue_count_set
 *
 *  Purpose :
 *      Set the number of the queeus.
 *
 *  Parameters :
 *      unit        :   RoboSwitch unit number.
 *      port_type   :   port type.
 *      count  :   number of queues.
 *
 *  Return :
 *      SOC_E_NONE : success.
 *      SOC_E_PARAM : parameters error.
 *
 *  Note :
 */
int 
drv_dino16_queue_count_set(int unit, uint32 port_type, uint8 count)
{
    uint32	reg_value;
    uint32	num_q;
    uint32  max_numq;
    int		cos, prio, ratio, remain;

    LOG_VERBOSE(BSL_LS_SOC_COMMON, \
                (BSL_META_U(unit, \
                            "drv_dino16_queue_count_set: \
                            unit %d, port type = %d, queue count = %d\n"),
                 unit, port_type, count));

    max_numq = NUM_COS(unit);
    if ((count > max_numq) || (count < 1)) {
        return SOC_E_PARAM;
    }

    SOC_IF_ERROR_RETURN(REG_READ_QOS_TX_CTRLr
        (unit, &reg_value));

    /* set queue number */
    num_q = count - 1;
    soc_QOS_TX_CTRLr_field_set(unit, &reg_value, 
        QOS_MODEf, &num_q);

    /* write register */
    SOC_IF_ERROR_RETURN(REG_WRITE_QOS_TX_CTRLr
        (unit, &reg_value));

    /* Map the eight 802.1 priority levels to the active cosqs */
    ratio = 8 / count;
    remain = 8 % count;
    cos = 0;
    for (prio = 0; prio < 8; prio++) {
        SOC_IF_ERROR_RETURN(DRV_QUEUE_PRIO_SET
            (unit, -1, prio, cos));
	   if ((prio + 1) == (((cos + 1) * ratio) +
	   	                  ((remain < (count - cos)) ? 0 :
	   	                  (remain - (count- cos) + 1)))) {
           cos++;
	   }
    }

    return SOC_E_NONE;
}

/*
 *  Function : drv_dino16_queue_count_get
 *
 *  Purpose :
 *      Get the number of the queeus.
 *
 *  Parameters :
 *      unit        :   RoboSwitch unit number.
 *      port_type   :   port type.
 *      count    :  (OUT) number of queues.
 *
 *  Return :
 *      SOC_E_NONE : success.
 *
 *  Note :
 */
int 
drv_dino16_queue_count_get(int unit, uint32 port_type, uint8 *count)
{
    uint32	reg_value;
    uint32	num_q = 0;

    SOC_IF_ERROR_RETURN(REG_READ_QOS_TX_CTRLr
        (unit, &reg_value));
    soc_QOS_TX_CTRLr_field_get(unit, &reg_value, 
        QOS_MODEf, &num_q);

    *count = num_q + 1;

    LOG_VERBOSE(BSL_LS_SOC_COMMON, \
                (BSL_META_U(unit, \
                            "drv_dino16_queue_count_get: \
                            unit %d, port type = %d, queue count = %d\n"),
                 unit, port_type, *count));
    
    return SOC_E_NONE;
}

/* config output queue mapping */
/*
 *  Function : drv_dino16_queue_prio_set
 *
 *  Purpose :
 *      Set the priority value of the specific queue.
 *
 *  Parameters :
 *      unit        :   RoboSwitch unit number.
 *      prio   :   priority value.
 *      queue_n  :   queue number.
 *
 *  Return :
 *      SOC_E_XXX.
 *
 *  Note :
 */
int 
drv_dino16_queue_prio_set(int unit, uint32 port, uint8 prio, uint8 queue_n)
{
    uint32  p;
    soc_pbmp_t  pbmp;

    LOG_VERBOSE(BSL_LS_SOC_COMMON, \
                (BSL_META_U(unit, \
                            "drv_dino16_queue_prio_set: \
                            unit %d, port = %d, priority = %d, queue = %d\n"),
                 unit, port, prio, queue_n));

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
 *  Function : drv_dino16_queue_prio_get
 *
 *  Purpose :
 *      Get the priority value of the specific queue.
 *
 *  Parameters :
 *      unit        :   RoboSwitch unit number.
 *      prio   :   priority value.
 *      queue_n  :   queue number.
 *
 *  Return :
 *      SOC_E_XXX.
 *
 *  Note :
 *      
 *
 */
int 
drv_dino16_queue_prio_get(int unit, uint32 port, uint8 prio, uint8 *queue_n)
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
        (unit, -1, prio, &new_prio));
    
    /* Transfer the new_prio by TC2COS mapping (per-system based) */
    SOC_IF_ERROR_RETURN(DRV_QUEUE_PORT_PRIO_TO_QUEUE_GET
        (unit, p, new_prio, queue_n));

    LOG_VERBOSE(BSL_LS_SOC_COMMON, \
                (BSL_META_U(unit, \
                            "drv_dino16_queue_prio_get: \
                            unit %d, port = %d, priority = %d, queue = %d\n"), 
                 unit, port, prio, *queue_n));

     return SOC_E_NONE;
}

/*
 *  Function : drv_dino16_queue_prio_remap_set
 *
 *  Purpose :
 *      Set the remapping internal priority of the selected port.
 *
 *  Parameters :
 *      unit     :  RoboSwitch unit number.
 *      port     :  port number.
 *      pre_prio :  previous internal priority value.
 *      prio     :  internal priority value.
 *
 *  Return :
 *      SOC_E_XXX.
 *
 *  Note :
 */
int
drv_dino16_queue_prio_remap_set(int unit, uint32 port, uint8 pre_prio, 
    uint8 prio)
{
    uint32  reg_value, temp;

    LOG_VERBOSE(BSL_LS_SOC_COMMON, \
                (BSL_META_U(unit, \
                            "drv_dino16_queue_prio_remap_set: \
                            unit %d, port = %d, pre_prio = %d, prio = %d\n"), 
                 unit, port, pre_prio, prio));

    if (port != -1) {
        return SOC_E_UNAVAIL;
    }

    SOC_IF_ERROR_RETURN(REG_READ_QOS_1P1Q_PRI_MAPr
        (unit, &reg_value));

    temp = (uint32)prio;
    switch (pre_prio) {        
        case 0:
            soc_QOS_1P1Q_PRI_MAPr_field_set(unit, &reg_value, 
                TAG000_PRI_MAPf, &temp);
            break;
        case 1:         
            soc_QOS_1P1Q_PRI_MAPr_field_set(unit, &reg_value, 
                TAG001_PRI_MAPf, &temp);
            break;
        case 2:         
            soc_QOS_1P1Q_PRI_MAPr_field_set(unit, &reg_value, 
                TAG010_PRI_MAPf, &temp);
            break;
        case 3:
            soc_QOS_1P1Q_PRI_MAPr_field_set(unit, &reg_value, 
                TAG011_PRI_MAPf, &temp);
            break;
        case 4:
            soc_QOS_1P1Q_PRI_MAPr_field_set(unit, &reg_value, 
                TAG100_PRI_MAPf, &temp);
            break;
        case 5:
            soc_QOS_1P1Q_PRI_MAPr_field_set(unit, &reg_value, 
                TAG101_PRI_MAPf, &temp);
            break;
        case 6:         
            soc_QOS_1P1Q_PRI_MAPr_field_set(unit, &reg_value, 
                TAG110_PRI_MAPf, &temp);
            break;
        case 7:
            soc_QOS_1P1Q_PRI_MAPr_field_set(unit, &reg_value, 
                TAG111_PRI_MAPf, &temp);
            break;
        default:
            return SOC_E_PARAM;
    }        

    SOC_IF_ERROR_RETURN(REG_WRITE_QOS_1P1Q_PRI_MAPr
        (unit, &reg_value));

    return SOC_E_NONE;
}

/*
 *  Function : drv_dino16_queue_prio_remap_get
 *
 *  Purpose :
 *      Get the remapping internal priority of the selected port.
 *
 *  Parameters :
 *      unit     :  RoboSwitch unit number.
 *      port     :  port number.
 *      pre_prio :  previous internal priority value.
 *      prio     :  (OUT) internal priority value.
 *
 *  Return :
 *      SOC_E_XXX.
 *
 *  Note :
 */
int
drv_dino16_queue_prio_remap_get(int unit, uint32 port, uint8 pre_prio, 
    uint8 *prio)
{
    uint32  reg_value, temp = 0;

    if (port != -1) {
        return SOC_E_UNAVAIL;
    }

    SOC_IF_ERROR_RETURN(REG_READ_QOS_1P1Q_PRI_MAPr
        (unit, &reg_value));

    switch (pre_prio) {
        case 0:
            soc_QOS_1P1Q_PRI_MAPr_field_get(unit, &reg_value, 
                TAG000_PRI_MAPf, &temp);
            break;
        case 1:         
            soc_QOS_1P1Q_PRI_MAPr_field_get(unit, &reg_value, 
                TAG001_PRI_MAPf, &temp);
            break;
        case 2:         
            soc_QOS_1P1Q_PRI_MAPr_field_get(unit, &reg_value, 
                TAG010_PRI_MAPf, &temp);
            break;
        case 3:
            soc_QOS_1P1Q_PRI_MAPr_field_get(unit, &reg_value, 
                TAG011_PRI_MAPf, &temp);
            break;
        case 4:
            soc_QOS_1P1Q_PRI_MAPr_field_get(unit, &reg_value, 
                TAG100_PRI_MAPf, &temp);
            break;
        case 5:
            soc_QOS_1P1Q_PRI_MAPr_field_get(unit, &reg_value, 
                TAG101_PRI_MAPf, &temp);
            break;
        case 6:         
            soc_QOS_1P1Q_PRI_MAPr_field_get(unit, &reg_value, 
                TAG110_PRI_MAPf, &temp);
            break;
        case 7:
            soc_QOS_1P1Q_PRI_MAPr_field_get(unit, &reg_value, 
                TAG111_PRI_MAPf, &temp);
            break;
        default:
            return SOC_E_PARAM;
    }        
    *prio = temp;

    LOG_VERBOSE(BSL_LS_SOC_COMMON, \
                (BSL_META_U(unit, \
                            "drv_dino16_queue_prio_remap_get: \
                            unit %d, port = %d, pre_prio = %d, *prio = %d\n"), 
                 unit, port, pre_prio, *prio));

    return SOC_E_NONE;
}

