/*
 * $Id: cosq.c,v 1.7 Broadcom SDK $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include <shared/bsl.h>

#include <soc/drv.h>
#include <soc/debug.h>

/* Port_QoS_En */
#define GEX_QUEUE_PORT_QOS_DISABLE  0x0
#define GEX_QUEUE_PORT_QOS_ENABLE   0x1

/* QoS_Layer_Sel */
#define GEX_QUEUE_QOS_LAYER_SEL_PRIO  0x0
#define GEX_QUEUE_QOS_LAYER_SEL_DFSV  0x1
#define GEX_QUEUE_QOS_LAYER_SEL_IP    0x2
#define GEX_QUEUE_QOS_LAYER_SEL_ALL   0x3

/*
 *  Function : drv_gex_queue_port_prio_to_queue_set
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
drv_gex_queue_port_prio_to_queue_set(int unit, uint8 port, uint8 prio, 
    uint8 queue_n)
{
    uint32  reg_value, temp;
    /* Using 32b port variable to compare against SOC_MAX_NUM_PORTS,
       which may be bigger the uint8 and then reuslts in a compilation error */
    uint32  port32 = port;

    LOG_VERBOSE(BSL_LS_SOC_COMMON, \
                (BSL_META_U(unit, \
                            "drv_gex_queue_port_prio_to_queue_set: \
                            unit %d, port = %d, prio = %d, queue_n = %d\n"), 
                 unit, port, prio, queue_n));

    /* Check port number */
    if (port32 > (SOC_MAX_NUM_PORTS - 1)) {
        return SOC_E_PARAM;
    }

    SOC_IF_ERROR_RETURN(REG_READ_TC2COS_MAPr
        (unit, &reg_value));

    temp = queue_n;
    switch (prio) {        
        case 0:
            SOC_IF_ERROR_RETURN(soc_TC2COS_MAPr_field_set
                (unit, &reg_value, PRT000_TO_QIDf, &temp));
            break;
        case 1:
            SOC_IF_ERROR_RETURN(soc_TC2COS_MAPr_field_set
                (unit, &reg_value, PRT001_TO_QIDf, &temp));
            break;
        case 2:         
            SOC_IF_ERROR_RETURN(soc_TC2COS_MAPr_field_set
                (unit, &reg_value, PRT010_TO_QIDf, &temp));
            break;
        case 3:
            SOC_IF_ERROR_RETURN(soc_TC2COS_MAPr_field_set
                (unit, &reg_value, PRT011_TO_QIDf, &temp));
            break;
        case 4:
            SOC_IF_ERROR_RETURN(soc_TC2COS_MAPr_field_set
                (unit, &reg_value, PRT100_TO_QIDf, &temp));
            break;
        case 5:
            SOC_IF_ERROR_RETURN(soc_TC2COS_MAPr_field_set
                (unit, &reg_value, PRT101_TO_QIDf, &temp));
            break;
        case 6:         
            SOC_IF_ERROR_RETURN(soc_TC2COS_MAPr_field_set
                (unit, &reg_value, PRT110_TO_QIDf, &temp));
            break;
        case 7:
            SOC_IF_ERROR_RETURN(soc_TC2COS_MAPr_field_set
                (unit, &reg_value, PRT111_TO_QIDf, &temp));
            break;
        default:
            return SOC_E_PARAM;
    }        
    SOC_IF_ERROR_RETURN(REG_WRITE_TC2COS_MAPr
        (unit, &reg_value));

    return SOC_E_NONE;
}

/*
 *  Function : drv_gex_queue_port_prio_to_queue_get
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
drv_gex_queue_port_prio_to_queue_get(int unit, uint8 port, uint8 prio, 
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

    SOC_IF_ERROR_RETURN(REG_READ_TC2COS_MAPr
        (unit, &reg_value));

    switch (prio) {
        case 0:
            SOC_IF_ERROR_RETURN(soc_TC2COS_MAPr_field_get
                (unit, &reg_value, PRT000_TO_QIDf, &temp));
            break;
        case 1:
            SOC_IF_ERROR_RETURN(soc_TC2COS_MAPr_field_get
                (unit, &reg_value, PRT001_TO_QIDf, &temp));
            break;
        case 2:         
            SOC_IF_ERROR_RETURN(soc_TC2COS_MAPr_field_get
                (unit, &reg_value, PRT010_TO_QIDf, &temp));
            break;
        case 3:
            SOC_IF_ERROR_RETURN(soc_TC2COS_MAPr_field_get
                (unit, &reg_value, PRT011_TO_QIDf, &temp));
            break;
        case 4:
            SOC_IF_ERROR_RETURN(soc_TC2COS_MAPr_field_get
                (unit, &reg_value, PRT100_TO_QIDf, &temp));
            break;
        case 5:
            SOC_IF_ERROR_RETURN(soc_TC2COS_MAPr_field_get
                (unit, &reg_value, PRT101_TO_QIDf, &temp));
            break;
        case 6:         
            SOC_IF_ERROR_RETURN(soc_TC2COS_MAPr_field_get
                (unit, &reg_value, PRT110_TO_QIDf, &temp));
            break;
        case 7:
            SOC_IF_ERROR_RETURN(soc_TC2COS_MAPr_field_get
                (unit, &reg_value, PRT111_TO_QIDf, &temp));
            break;
        default:
            return SOC_E_PARAM;
    }
    *queue_n = temp;

    LOG_VERBOSE(BSL_LS_SOC_COMMON, \
                (BSL_META_U(unit, \
                            "drv_gex_queue_port_prio_to_queue_get: \
                            unit %d, port = %d, prio = %d, *queue_n = %d\n"), 
                 unit, port, prio, *queue_n));

    return SOC_E_NONE;
}

/*
 *  Function : drv_gex_queue_port_dfsv_set
 *
 *  Purpose :
 *      Set the queue id for DSCP priority mapping of selected port.
 *
 *  Parameters :
 *      unit     :  RoboSwitch unit number.
 *      port     :  port id.
 *      dscp     :  dscp priority value.
 *      prio     :  internal priority value.
 *      queue_n  :  queue number.
 *
 *  Return :
 *      SOC_E_XXX.
 *
 *  Note :
 */
int
drv_gex_queue_port_dfsv_set(int unit, uint8 port, uint8 dscp, 
    uint8 prio, uint8 queue_n)
{
    uint32  reg_value;
    uint32  bmp = 0;
    /* Using 32b port variable to compare against SOC_MAX_NUM_PORTS,
       which may be bigger the uint8 and then reuslts in a compilation error */
    uint32  port32 = port;

    LOG_VERBOSE(BSL_LS_SOC_COMMON, \
                (BSL_META_U(unit, \
                            "drv_gex_queue_port_dfsv_set: \
                            unit %d, port = %d, dscp = %d, prio = %d, queue_n = %d\n"), 
                 unit, port, dscp, prio, queue_n));

    /* Check port number */
    if (port32 > (SOC_MAX_NUM_PORTS - 1)) {
        return SOC_E_PARAM;
    }

    SOC_IF_ERROR_RETURN(REG_READ_QOS_EN_DIFFSERVr
        (unit, &reg_value));

    SOC_IF_ERROR_RETURN(soc_QOS_EN_DIFFSERVr_field_get
        (unit, &reg_value, QOS_EN_DIFFSERVf, &bmp));

    bmp |= 1U << port;
    SOC_IF_ERROR_RETURN(soc_QOS_EN_DIFFSERVr_field_set
        (unit, &reg_value, QOS_EN_DIFFSERVf, &bmp));
    SOC_IF_ERROR_RETURN(REG_WRITE_QOS_EN_DIFFSERVr
        (unit, &reg_value));

	SOC_IF_ERROR_RETURN(DRV_QUEUE_DFSV_REMAP_SET
        (unit, dscp, prio));

    SOC_IF_ERROR_RETURN(DRV_QUEUE_PORT_PRIO_TO_QUEUE_SET
        (unit, port, prio, queue_n));

    return SOC_E_NONE;
}

/*
 *  Function : drv_gex_queue_port_dfsv_get
 *
 *  Purpose :
 *      Get the internal priority and queue id for DSCP priority mapping of selected port.
 *
 *  Parameters :
 *      unit     :  RoboSwitch unit number.
 *      port     :  port id.
 *      dscp     :  dscp priority value.
 *      prio     :  (OUT) internal priority value.
 *      queue_n  :  (OUT) queue number.
 *
 *  Return :
 *      SOC_E_XXX.
 *
 *  Note :
 */
int
drv_gex_queue_port_dfsv_get(int unit, uint8 port, uint8 dscp, 
    uint8 *prio, uint8 *queue_n)
{
    uint32  reg_value; 
    uint32  bmp = 0;
    soc_pbmp_t  pbmp;
    /* Using 32b port variable to compare against SOC_MAX_NUM_PORTS,
       which may be bigger the uint8 and then reuslts in a compilation error */
    uint32  port32 = port;

    /* Check port number */
    if (port32 > (SOC_MAX_NUM_PORTS - 1)) {
         return SOC_E_PARAM;
    }

    SOC_IF_ERROR_RETURN(REG_READ_QOS_EN_DIFFSERVr
        (unit, &reg_value));
    SOC_IF_ERROR_RETURN(soc_QOS_EN_DIFFSERVr_field_get
        (unit, &reg_value, QOS_EN_DIFFSERVf, &bmp));

    SOC_PBMP_WORD_SET(pbmp, 0, bmp);
    if (!SOC_PBMP_MEMBER(pbmp, port)) {
        return SOC_E_BADID;
    }

	SOC_IF_ERROR_RETURN(DRV_QUEUE_DFSV_REMAP_GET
        (unit, dscp, prio));

    SOC_IF_ERROR_RETURN(DRV_QUEUE_PORT_PRIO_TO_QUEUE_GET
        (unit, port, *prio, queue_n));

    LOG_VERBOSE(BSL_LS_SOC_COMMON, \
                (BSL_META_U(unit, \
                            "drv_gex_queue_port_dfsv_get: \
                            unit %d, port = %d, dscp = %d, *prio = %d, *queue_n = %d\n"), 
                 unit, port, dscp, *prio, *queue_n));

    return SOC_E_NONE;
}

/*
 *  Function : drv_gex_queue_mode_set
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
 */
int 
drv_gex_queue_mode_set(int unit, soc_pbmp_t bmp, uint32 flag, 
    uint32 mode)
{
    uint32  reg_value, temp;
    
    LOG_VERBOSE(BSL_LS_SOC_COMMON, \
                (BSL_META_U(unit, \
                            "drv_gex_queue_mode_set: \
                            unit %d, bmp = 0x%x, flag = 0x%x, queue mode = %d\n"),
                 unit, SOC_PBMP_WORD_GET(bmp, 0), flag, mode));

    if (flag != 0) {
        return SOC_E_PARAM;
    }
    
    SOC_IF_ERROR_RETURN(REG_READ_QOS_TX_CTRLr
        (unit, &reg_value));

    switch (mode) {
        case DRV_QUEUE_MODE_WRR:
            temp = 0;
            break;
        case DRV_QUEUE_MODE_1STRICT:
            temp = 1;
            break;
        case DRV_QUEUE_MODE_2STRICT:
            temp = 2;
            break;
        case DRV_QUEUE_MODE_STRICT:
            temp = 3;
            break;
        default:
            return SOC_E_UNAVAIL;
    }
    SOC_IF_ERROR_RETURN(soc_QOS_TX_CTRLr_field_set
        (unit, &reg_value, QOS_PRIORITY_CTRLf, &temp));
    SOC_IF_ERROR_RETURN(REG_WRITE_QOS_TX_CTRLr
        (unit, &reg_value));
    
    return SOC_E_NONE;
}

/*
 *  Function : drv_gex_queue_mode_get
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
drv_gex_queue_mode_get(int unit, uint32 port, uint32 flag, 
    uint32 *mode)
{
    uint32  reg_value, temp;

    if (flag != 0) {
        return SOC_E_PARAM;
    }

    SOC_IF_ERROR_RETURN(REG_READ_QOS_TX_CTRLr
        (unit, &reg_value));
    SOC_IF_ERROR_RETURN(soc_QOS_TX_CTRLr_field_get
        (unit, &reg_value, QOS_PRIORITY_CTRLf, &temp));

    switch (temp) {
        case 0:
            *mode = DRV_QUEUE_MODE_WRR;
            break;
        case 1:
            *mode = DRV_QUEUE_MODE_1STRICT;
            break;
        case 2:
            *mode = DRV_QUEUE_MODE_2STRICT;
            break;
        case 3:
            *mode = DRV_QUEUE_MODE_STRICT;
            break;
        default:
            return SOC_E_INTERNAL;
    }

    LOG_VERBOSE(BSL_LS_SOC_COMMON, \
                (BSL_META_U(unit, \
                            "drv_gex_queue_mode_get: \
                            unit %d, port = %d, flag = 0x%x, queue mode = %d\n"),
                 unit, port, flag, *mode));    

    return SOC_E_NONE;
}

/*
 *  Function : drv_gex_queue_count_set
 *
 *  Purpose :
 *      Set the number of the queues.
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
drv_gex_queue_count_set(int unit, uint32 port_type, uint8 count)
{
    LOG_VERBOSE(BSL_LS_SOC_COMMON, \
                (BSL_META_U(unit, \
                            "drv_gex_queue_count_set: \
                            unit %d, port type = %d, queue count = %d\n"), unit, port_type, count));

    if (count != NUM_COS(unit)) {
        return SOC_E_PARAM;
    }

    return SOC_E_NONE;
}

/*
 *  Function : drv_gex_queue_count_get
 *
 *  Purpose :
 *      Get the number of the queues.
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
drv_gex_queue_count_get(int unit, uint32 port_type, uint8 *count)
{

    
    *count = NUM_COS(unit);

    LOG_VERBOSE(BSL_LS_SOC_COMMON, \
                (BSL_META_U(unit, \
                            "drv_gex_queue_count_get: \
                            unit %d, port type = %d, queue count = %d\n"), unit, port_type, *count));

    return SOC_E_NONE;
}

/* Set WRR weight */
/*
 *  Function : drv_gex_queue_WRR_weight_set
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
 */
int 
drv_gex_queue_WRR_weight_set(int unit, uint32 port_type, 
    soc_pbmp_t bmp, uint8 queue, uint32 weight)
{
    uint32  reg_value, temp;
    uint32  max_weight = 0;

    LOG_VERBOSE(BSL_LS_SOC_COMMON, \
                (BSL_META_U(unit, \
                            "drv_gex_queue_WRR_weight_set: \
                            unit %d, port type = %d, bmp = 0x%x, queue = %d, weight = %d\n"),
                 unit, port_type, SOC_PBMP_WORD_GET(bmp,0), queue, weight));

    SOC_IF_ERROR_RETURN(DRV_DEV_PROP_GET
        (unit, DRV_DEV_PROP_COSQ_MAX_WEIGHT_VALUE, &max_weight));

    if ((weight > max_weight) || (weight < 1)) {
        return SOC_E_PARAM;
    }
    
    SOC_IF_ERROR_RETURN(REG_READ_QOS_WEIGHTr
        (unit, queue, &reg_value));

    temp = weight;
    SOC_IF_ERROR_RETURN(soc_QOS_WEIGHTr_field_set
        (unit, &reg_value, WEIGHTSf, &temp));
    SOC_IF_ERROR_RETURN(REG_WRITE_QOS_WEIGHTr
        (unit, queue, &reg_value));

    return SOC_E_NONE;
}

/*
 *  Function : drv_gex_queue_WRR_weight_get
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
drv_gex_queue_WRR_weight_get(int unit, uint32 port_type, 
    uint32 port, uint8 queue, uint32 *weight)
{
    uint32  reg_value;
    
    SOC_IF_ERROR_RETURN(REG_READ_QOS_WEIGHTr
        (unit, queue, &reg_value));
    
    *weight = reg_value;

    LOG_VERBOSE(BSL_LS_SOC_COMMON, \
                (BSL_META_U(unit, \
                            "drv_gex_queue_WRR_weight_get: \
                            unit %d, port type = %d, port = %d, queue = %d, weight = %d\n"),
                 unit, port_type, port, queue, *weight));

    return SOC_E_NONE;
}

/* Config output queue mapping */
/*
 *  Function : drv_gex_queue_prio_set
 *
 *  Purpose :
 *      Set the queue number of the specific priority vlaue.
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
drv_gex_queue_prio_set(int unit, uint32 port, uint8 prio, uint8 queue_n)
{
    uint32  new_prio = 0;
    uint8   queue_t = 0;
    int     i = 0, temp = 0;

    LOG_VERBOSE(BSL_LS_SOC_COMMON, \
                (BSL_META_U(unit, \
                            "drv_gex_queue_prio_set: \
                            unit %d, port = %d, priority = %d, queue = %d\n"), 
                 unit, port, prio, queue_n));

    if (port == -1) {
        /* Set 1P/1Q Prioryty map */
        SOC_IF_ERROR_RETURN(DRV_QUEUE_PRIO_REMAP_SET
            (unit, -1, prio, prio));
    
        /* GEX family only has a global priority-to-queue id mapping register */
        /*
          * COVERITY
          *
          * Intentional, not a copy-paste error.
          */
        /* coverity[copy_paste_error : FALSE] */
        SOC_IF_ERROR_RETURN(DRV_QUEUE_PORT_PRIO_TO_QUEUE_SET
            (unit, 0, prio, queue_n));
    } else {
        /* 
          * Search and get the first matched queue_n (TC2COS mapping by per-system based) 
          * with related priority value for new_prio.
          */
        for (i = 0; i < 8; i++) {
            if (!temp) {
                SOC_IF_ERROR_RETURN(DRV_QUEUE_PRIO_GET
                    (unit, -1, i, &queue_t));
                if (queue_t == queue_n) {
                    new_prio = i;
                    temp = 1;
                }
            }
        }

        /* Then set the PCP2TC mapping with new_prio value by per-port based */
        SOC_IF_ERROR_RETURN(DRV_QUEUE_PRIO_REMAP_SET
            (unit, port, prio, new_prio));
    }

    return SOC_E_NONE;
}

/*
 *  Function : drv_gex_queue_prio_get
 *
 *  Purpose :
 *      Get the queue number of the specific priority value.
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
drv_gex_queue_prio_get(int unit, uint32 port, uint8 prio, uint8 *queue_n)
{
    uint32  p;
    uint8   new_prio = 0;

    if (port == -1) {
        /* Get GE0's */
        p = 0;
        /* GEX family only has a global priority-to-queue id mapping register */
        SOC_IF_ERROR_RETURN(DRV_QUEUE_PORT_PRIO_TO_QUEUE_GET
            (unit, p, prio, queue_n));
    } else {
        p = port;
        /* Get the new_prio value from PCP2TC mapping by per-port based */
        SOC_IF_ERROR_RETURN(DRV_QUEUE_PRIO_REMAP_GET
            (unit, p, prio, &new_prio));

        /* Transfer the new_prio by TC2COS mapping (per-system based) */
        SOC_IF_ERROR_RETURN(DRV_QUEUE_PORT_PRIO_TO_QUEUE_GET
            (unit, 0, new_prio, queue_n));
    }

    LOG_VERBOSE(BSL_LS_SOC_COMMON, \
                (BSL_META_U(unit, \
                            "drv_gex_queue_prio_get: \
                            unit %d, port = %d, priority = %d, queue = %d\n"),
                 unit, port, prio, *queue_n));

    return SOC_E_NONE;
}

/*
 *  Function : drv_gex_queue_prio_remap_set
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
drv_gex_queue_prio_remap_set(int unit, uint32 port, uint8 pre_prio, 
    uint8 prio)
{
    uint32  reg_addr, reg_value, temp, p;
    int     reg_len;
    soc_pbmp_t  pbmp;
#if defined(BCM_POLAR_SUPPORT) || defined(BCM_NORTHSTAR_SUPPORT)
    uint32 specified_port_num;

    if (SOC_IS_POLAR(unit)) {
        SOC_IF_ERROR_RETURN(DRV_DEV_PROP_GET
            (unit, DRV_DEV_PROP_INTERNAL_CPU_PORT_NUM, &specified_port_num));
    } else if (SOC_IS_NORTHSTAR(unit)) {
        SOC_IF_ERROR_RETURN(DRV_DEV_PROP_GET
            (unit, DRV_DEV_PROP_ADDITIONAL_SOC_PORT_NUM, &specified_port_num));
    }
#endif /* BCM_POLAR_SUPPORT || BCM_NORTHSTAR_SUPPORT */

    LOG_VERBOSE(BSL_LS_SOC_COMMON, \
                (BSL_META_U(unit, \
                            "drv_gex_queue_prio_remap_set: \
                            unit %d, port = %d, pre_prio = %d, prio = %d\n"), 
                 unit, port, pre_prio, prio));

    SOC_PBMP_CLEAR(pbmp);
    if (port == -1) {
        SOC_PBMP_ASSIGN(pbmp, PBMP_ALL(unit));
    } else {
        SOC_PBMP_PORT_SET(pbmp, port);
    }

    PBMP_ITER(pbmp, p) {
        if (IS_GE_PORT(unit, p) || 
            (IS_FE_PORT(unit, p) && SOC_IS_LOTUS(unit))) {
            /* For GE ports */
#if defined(BCM_POLAR_SUPPORT) || defined(BCM_NORTHSTAR_SUPPORT)
            if ((SOC_IS_POLAR(unit) || SOC_IS_NORTHSTAR(unit)) 
                && (p == specified_port_num)) {
                reg_addr = (DRV_SERVICES(unit)->reg_addr)
                    (unit, INDEX(P7_PCP2TCr), p, 0);
            } else 
#endif /* BCM_POLAR_SUPPORT || BCM_NORTHSTAR_SUPPORT */
            {
                reg_addr = (DRV_SERVICES(unit)->reg_addr)
                    (unit, INDEX(PN_PCP2TCr), p, 0);
            }
            reg_len = (DRV_SERVICES(unit)->reg_length_get)
                (unit, INDEX(PN_PCP2TCr));
        } else {  /* IMP port */
            if (SOC_IS_LOTUS(unit) || SOC_IS_STARFIGHTER(unit) ||
                SOC_IS_BLACKBIRD2(unit) || SOC_IS_POLAR(unit) ||
                SOC_IS_NORTHSTAR(unit)) {
                reg_addr = (DRV_SERVICES(unit)->reg_addr)
                    (unit, INDEX(IMP_PCP2TCr), 0, 0);
            	reg_len = (DRV_SERVICES(unit)->reg_length_get)
                    (unit, INDEX(IMP_PCP2TCr));
            } else {
                reg_addr = (DRV_SERVICES(unit)->reg_addr)
                    (unit, INDEX(P8_PCP2TCr), 0, 0);
                reg_len = (DRV_SERVICES(unit)->reg_length_get)
                    (unit, INDEX(P8_PCP2TCr));
            }
        }
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_read)
            (unit, reg_addr, &reg_value, reg_len));
        temp = (uint32)prio;
        switch (pre_prio) {
            case 0:
                SOC_IF_ERROR_RETURN(soc_PN_PCP2TCr_field_set
                    (unit, &reg_value, TAG000_PRI_MAPf, &temp));
                break;
            case 1:
                SOC_IF_ERROR_RETURN(soc_PN_PCP2TCr_field_set
                    (unit, &reg_value, TAG001_PRI_MAPf, &temp));
                break;
            case 2:         
                SOC_IF_ERROR_RETURN(soc_PN_PCP2TCr_field_set
                    (unit, &reg_value, TAG010_PRI_MAPf, &temp));
                break;
            case 3:
                SOC_IF_ERROR_RETURN(soc_PN_PCP2TCr_field_set
                    (unit, &reg_value, TAG011_PRI_MAPf, &temp));
                break;
            case 4:
                SOC_IF_ERROR_RETURN(soc_PN_PCP2TCr_field_set
                    (unit, &reg_value, TAG100_PRI_MAPf, &temp));
                break;
            case 5:
                SOC_IF_ERROR_RETURN(soc_PN_PCP2TCr_field_set
                    (unit, &reg_value, TAG101_PRI_MAPf, &temp));
                break;
            case 6:         
                SOC_IF_ERROR_RETURN(soc_PN_PCP2TCr_field_set
                    (unit, &reg_value, TAG110_PRI_MAPf, &temp));
                break;
            case 7:
                SOC_IF_ERROR_RETURN(soc_PN_PCP2TCr_field_set
                    (unit, &reg_value, TAG111_PRI_MAPf, &temp));
                break;
            default:
                return SOC_E_PARAM;
        }        
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_write)
            (unit, reg_addr, &reg_value, reg_len));
    }

    return SOC_E_NONE;
}

/*
 *  Function : drv_gex_queue_prio_remap_get
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
drv_gex_queue_prio_remap_get(int unit, uint32 port, uint8 pre_prio, 
    uint8 *prio)
{
    uint32  reg_value, temp = 0, p;
#if defined(BCM_POLAR_SUPPORT) || defined(BCM_NORTHSTAR_SUPPORT)
    uint32 specified_port_num;

    if (SOC_IS_POLAR(unit)) {
        SOC_IF_ERROR_RETURN(DRV_DEV_PROP_GET
            (unit, DRV_DEV_PROP_INTERNAL_CPU_PORT_NUM, &specified_port_num));
    } else if (SOC_IS_NORTHSTAR(unit)) {
        SOC_IF_ERROR_RETURN(DRV_DEV_PROP_GET
            (unit, DRV_DEV_PROP_ADDITIONAL_SOC_PORT_NUM, &specified_port_num));
    }
#endif /* BCM_POLAR_SUPPORT || BCM_NORTHSTAR_SUPPORT */

    if (port == -1) {
        /* Get GE0's PN_PCP2TC register value */
        p = 0;
    } else {
        p = port;
    }

#if defined(BCM_POLAR_SUPPORT) || defined(BCM_NORTHSTAR_SUPPORT)
    if ((SOC_IS_POLAR(unit) || SOC_IS_NORTHSTAR(unit))
        && (port == specified_port_num)) {
        SOC_IF_ERROR_RETURN(REG_READ_P7_PCP2TCr
            (unit, &reg_value));
    } else 
#endif /* BCM_POLAR_SUPPORT || BCM_NORTHSTAR_SUPPORT */
    {
        SOC_IF_ERROR_RETURN(REG_READ_PN_PCP2TCr
            (unit, p, &reg_value));
    }

    switch (pre_prio) {        
        case 0:
            SOC_IF_ERROR_RETURN(soc_PN_PCP2TCr_field_get
                (unit, &reg_value, TAG000_PRI_MAPf, &temp));
            break;
        case 1:         
            SOC_IF_ERROR_RETURN(soc_PN_PCP2TCr_field_get
                (unit, &reg_value, TAG001_PRI_MAPf, &temp));
            break;
        case 2:         
            SOC_IF_ERROR_RETURN(soc_PN_PCP2TCr_field_get
                (unit, &reg_value, TAG010_PRI_MAPf, &temp));
            break;
        case 3:
            SOC_IF_ERROR_RETURN(soc_PN_PCP2TCr_field_get
                (unit, &reg_value, TAG011_PRI_MAPf, &temp));
            break;
        case 4:
            SOC_IF_ERROR_RETURN(soc_PN_PCP2TCr_field_get
                (unit, &reg_value, TAG100_PRI_MAPf, &temp));
            break;
        case 5:
            SOC_IF_ERROR_RETURN(soc_PN_PCP2TCr_field_get
                (unit, &reg_value, TAG101_PRI_MAPf, &temp));
            break;
        case 6:         
            SOC_IF_ERROR_RETURN(soc_PN_PCP2TCr_field_get
                (unit, &reg_value, TAG110_PRI_MAPf, &temp));
            break;
        case 7:
            SOC_IF_ERROR_RETURN(soc_PN_PCP2TCr_field_get
                (unit, &reg_value, TAG111_PRI_MAPf, &temp));
            break;
        default:
            return SOC_E_PARAM;
    }        
    *prio = temp;

    LOG_VERBOSE(BSL_LS_SOC_COMMON, \
                (BSL_META_U(unit, \
                            "drv_gex_queue_prio_remap_get: \
                            unit %d, port = %d, pre_prio = %d, *prio = %d\n"), 
                 unit, port, pre_prio, *prio));

     return SOC_E_NONE;
}

/*
 *  Function : drv_gex_queue_dfsv_remap_set
 *
 *  Purpose :
 *      Set the remapping DSCP priority value (global).
 *
 *  Parameters :
 *      unit     :  RoboSwitch unit number.
 *      dscp     :  dscp value.
 *      prio     :  internal priority value.
 *
 *  Return :
 *      SOC_E_XXX.
 *
 *  Note :
 */
int
drv_gex_queue_dfsv_remap_set(int unit, uint8 dscp, uint8 prio)
{
    uint32  reg_addr, temp;
    uint64  reg_value;
    int     reg_len, reg_index = 0;

    LOG_VERBOSE(BSL_LS_SOC_COMMON, \
                (BSL_META_U(unit, \
                            "drv_gex_queue_dfsv_remap_set: \
                            unit %d, dscp = %d, prio = %d\n"), unit, dscp, prio));

    if (dscp < 16) {
        reg_index = INDEX(QOS_DIFF_DSCP0r);        
    } else if ((16 <= dscp) && (dscp < 32)) {
        reg_index = INDEX(QOS_DIFF_DSCP1r);
        dscp -= 16;
    } else if ((32 <= dscp) && (dscp < 48)) {
        reg_index = INDEX(QOS_DIFF_DSCP2r);
        dscp -= 32;
    } else if ((48 <= dscp) && (dscp < 64)) {
        reg_index = INDEX(QOS_DIFF_DSCP3r);
        dscp -= 48;
    }

    reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, reg_index, 0, 0);
    reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, reg_index);
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_read)
        (unit, reg_addr, (uint32 *)&reg_value, reg_len));

    temp = (uint32)prio;
    switch (dscp) {        
        case 0:
            SOC_IF_ERROR_RETURN(soc_QOS_DIFF_DSCP0r_field_set
                (unit, (uint32 *)&reg_value, PRI_DSCP_000000f, &temp));
            break;
        case 1:
            SOC_IF_ERROR_RETURN(soc_QOS_DIFF_DSCP0r_field_set
                (unit, (uint32 *)&reg_value, PRI_DSCP_000001f, &temp));
            break;
        case 2:
            SOC_IF_ERROR_RETURN(soc_QOS_DIFF_DSCP0r_field_set
                (unit, (uint32 *)&reg_value, PRI_DSCP_000010f, &temp));
            break;
        case 3:
            SOC_IF_ERROR_RETURN(soc_QOS_DIFF_DSCP0r_field_set
                (unit, (uint32 *)&reg_value, PRI_DSCP_000011f, &temp));
            break;
        case 4:
            SOC_IF_ERROR_RETURN(soc_QOS_DIFF_DSCP0r_field_set
                (unit, (uint32 *)&reg_value, PRI_DSCP_000100f, &temp));
            break;
        case 5:
            SOC_IF_ERROR_RETURN(soc_QOS_DIFF_DSCP0r_field_set
                (unit, (uint32 *)&reg_value, PRI_DSCP_000101f, &temp));
            break;
        case 6:
            SOC_IF_ERROR_RETURN(soc_QOS_DIFF_DSCP0r_field_set
                (unit, (uint32 *)&reg_value, PRI_DSCP_000110f, &temp));
            break;
        case 7:
            SOC_IF_ERROR_RETURN(soc_QOS_DIFF_DSCP0r_field_set
                (unit, (uint32 *)&reg_value, PRI_DSCP_000111f, &temp));
            break;
        case 8:
            SOC_IF_ERROR_RETURN(soc_QOS_DIFF_DSCP0r_field_set
                (unit, (uint32 *)&reg_value, PRI_DSCP_001000f, &temp));
            break;
        case 9:
            SOC_IF_ERROR_RETURN(soc_QOS_DIFF_DSCP0r_field_set
                (unit, (uint32 *)&reg_value, PRI_DSCP_001001f, &temp));
            break;
        case 10:
            SOC_IF_ERROR_RETURN(soc_QOS_DIFF_DSCP0r_field_set
                (unit, (uint32 *)&reg_value, PRI_DSCP_001010f, &temp));
            break;
        case 11:
            SOC_IF_ERROR_RETURN(soc_QOS_DIFF_DSCP0r_field_set
                (unit, (uint32 *)&reg_value, PRI_DSCP_001011f, &temp));
            break;
        case 12:
            SOC_IF_ERROR_RETURN(soc_QOS_DIFF_DSCP0r_field_set
                (unit, (uint32 *)&reg_value, PRI_DSCP_001100f, &temp));
            break;
        case 13:
            SOC_IF_ERROR_RETURN(soc_QOS_DIFF_DSCP0r_field_set
                (unit, (uint32 *)&reg_value, PRI_DSCP_001101f, &temp));
            break;
        case 14:
            SOC_IF_ERROR_RETURN(soc_QOS_DIFF_DSCP0r_field_set
                (unit, (uint32 *)&reg_value, PRI_DSCP_001110f, &temp));
            break;
        case 15:
            SOC_IF_ERROR_RETURN(soc_QOS_DIFF_DSCP0r_field_set
                (unit, (uint32 *)&reg_value, PRI_DSCP_001111f, &temp));
            break;            
    }

    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_write)
        (unit, reg_addr, (uint32 *)&reg_value, reg_len));

    return SOC_E_NONE;
}

/*
 *  Function : drv_gex_queue_dfsv_remap_get
 *
 *  Purpose :
 *      Get the the remapping DSCP priority value (global).
 *
 *  Parameters :
 *      unit     :  RoboSwitch unit number.
 *      dscp     :  dscp value.
 *      prio     :  (OUT) internal priority value.
 *
 *  Return :
 *      SOC_E_XXX.
 *
 *  Note :
 */
int
drv_gex_queue_dfsv_remap_get(int unit, uint8 dscp, uint8 *prio)
{
    uint32  reg_addr, temp = 0;        
    uint64  reg_value;        
    int     reg_len, reg_index = 0;
    
    if (dscp < 16) {
        reg_index = INDEX(QOS_DIFF_DSCP0r);        
    } else if ((16 <= dscp) && (dscp < 32)) {
        reg_index = INDEX(QOS_DIFF_DSCP1r);
        dscp -= 16;
    } else if ((32 <= dscp) && (dscp < 48)) {
        reg_index = INDEX(QOS_DIFF_DSCP2r);
        dscp -= 32;
    } else if ((48 <= dscp) && (dscp < 64)) {
        reg_index = INDEX(QOS_DIFF_DSCP3r);
        dscp -= 48;
    }

    reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, reg_index, 0, 0);
    reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, reg_index);
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_read)
        (unit, reg_addr, (uint32 *)&reg_value, reg_len));
    
    switch (dscp) {        
        case 0:
            SOC_IF_ERROR_RETURN(soc_QOS_DIFF_DSCP0r_field_get
                (unit, (uint32 *)&reg_value, PRI_DSCP_000000f, &temp));
            break;
        case 1:
            SOC_IF_ERROR_RETURN(soc_QOS_DIFF_DSCP0r_field_get
                (unit, (uint32 *)&reg_value, PRI_DSCP_000001f, &temp));
            break;
        case 2:
            SOC_IF_ERROR_RETURN(soc_QOS_DIFF_DSCP0r_field_get
                (unit, (uint32 *)&reg_value, PRI_DSCP_000010f, &temp));
            break;
        case 3:
            SOC_IF_ERROR_RETURN(soc_QOS_DIFF_DSCP0r_field_get
                (unit, (uint32 *)&reg_value, PRI_DSCP_000011f, &temp));
            break;
        case 4:
            SOC_IF_ERROR_RETURN(soc_QOS_DIFF_DSCP0r_field_get
                (unit, (uint32 *)&reg_value, PRI_DSCP_000100f, &temp));
            break;
        case 5:
            SOC_IF_ERROR_RETURN(soc_QOS_DIFF_DSCP0r_field_get
                (unit, (uint32 *)&reg_value, PRI_DSCP_000101f, &temp));
            break;
        case 6:
            SOC_IF_ERROR_RETURN(soc_QOS_DIFF_DSCP0r_field_get
                (unit, (uint32 *)&reg_value, PRI_DSCP_000110f, &temp));
            break;
        case 7:
            SOC_IF_ERROR_RETURN(soc_QOS_DIFF_DSCP0r_field_get
                (unit, (uint32 *)&reg_value, PRI_DSCP_000111f, &temp));
            break;
        case 8:
            SOC_IF_ERROR_RETURN(soc_QOS_DIFF_DSCP0r_field_get
                (unit, (uint32 *)&reg_value, PRI_DSCP_001000f, &temp));
            break;
        case 9:
            SOC_IF_ERROR_RETURN(soc_QOS_DIFF_DSCP0r_field_get
                (unit, (uint32 *)&reg_value, PRI_DSCP_001001f, &temp));
            break;
        case 10:
            SOC_IF_ERROR_RETURN(soc_QOS_DIFF_DSCP0r_field_get
                (unit, (uint32 *)&reg_value, PRI_DSCP_001010f, &temp));
            break;
        case 11:
            SOC_IF_ERROR_RETURN(soc_QOS_DIFF_DSCP0r_field_get
                (unit, (uint32 *)&reg_value, PRI_DSCP_001011f, &temp));
            break;
        case 12:
            SOC_IF_ERROR_RETURN(soc_QOS_DIFF_DSCP0r_field_get
                (unit, (uint32 *)&reg_value, PRI_DSCP_001100f, &temp));
            break;
        case 13:
            SOC_IF_ERROR_RETURN(soc_QOS_DIFF_DSCP0r_field_get
                (unit, (uint32 *)&reg_value, PRI_DSCP_001101f, &temp));
            break;
        case 14:
            SOC_IF_ERROR_RETURN(soc_QOS_DIFF_DSCP0r_field_get
                (unit, (uint32 *)&reg_value, PRI_DSCP_001110f, &temp));
            break;
        case 15:
            SOC_IF_ERROR_RETURN(soc_QOS_DIFF_DSCP0r_field_get
                (unit, (uint32 *)&reg_value, PRI_DSCP_001111f, &temp));
            break;            
    }
    *prio = temp;

    LOG_VERBOSE(BSL_LS_SOC_COMMON, \
                (BSL_META_U(unit, \
                            "drv_gex_queue_dfsv_remap_get: \
                            unit %d, dscp = %d, *prio = %d\n"), unit, dscp, *prio));

    return SOC_E_NONE;
}

/*
 *  Function : drv_gex_queue_dfsv_set
 *
 *  Purpose :
 *      Set the queue id of the specific DSCP priority value.
 *
 *  Parameters :
 *      unit       :  RoboSwitch unit number.
 *      code_point :  DSCP priority value.
 *      queue_n    :  queue number.
 *
 *  Return :
 *      SOC_E_XXX.
 *
 *  Note :
 */
int 
drv_gex_queue_dfsv_set(int unit, uint8 code_point, uint8 queue_n)
{
    uint8  prio, state, queue_num;
    soc_pbmp_t  pbmp;
    soc_port_t  port;

    LOG_VERBOSE(BSL_LS_SOC_COMMON, \
                (BSL_META_U(unit, \
                            "drv_gex_queue_dfsv_set: \
                            unit %d, code_point = %d, queue_n = %d\n"), unit, code_point, queue_n));

    pbmp = PBMP_PORT_ALL(unit);

    queue_num = NUM_COS(unit);
    if (queue_num == 4) {
        switch (queue_n) {
            case 0:
                prio = 1;
                break;
            case 1:
                prio = 3;            
                break;
            case 2:
                prio = 5;
                break;
            case 3:
                prio = 7;
                break;
            default :
                prio = 0;
                break;
        }
    } else {
        /* generially, current robo devices support 4 or 8 cosq */
        if (queue_n < 8) {
            prio = queue_n;
        } else {
            prio = 0;
        }
    }

    PBMP_ITER(pbmp, port) {        
        SOC_IF_ERROR_RETURN(DRV_QUEUE_MAPPING_TYPE_GET
            (unit, port, DRV_QUEUE_MAP_DFSV, &state));
        if (!state) {
            break;
        }
        SOC_IF_ERROR_RETURN(DRV_QUEUE_PORT_DFSV_SET
            (unit, port, code_point, prio, queue_n));
    }

    return SOC_E_NONE;
}

/*
 *  Function : drv_gex_queue_dfsv_get
 *
 *  Purpose :
 *      Get the queue id of the specific DSCP priority value.
 *
 *  Parameters :
 *      unit        :   RoboSwitch unit number.
 *      code_point   :   DSCP priority value.
 *      queue_n  :   (OUT) queue number.
 *
 *  Return :
 *      SOC_E_XXX.
 *
 *  Note :
 */
int 
drv_gex_queue_dfsv_get(int unit, uint8 code_point, uint8 *queue_n)
{
    uint8  prio, state;
    soc_pbmp_t  pbmp;
    soc_port_t  port;

    pbmp = PBMP_PORT_ALL(unit);

    PBMP_ITER(pbmp, port) {        
        SOC_IF_ERROR_RETURN(DRV_QUEUE_MAPPING_TYPE_GET
            (unit, port, DRV_QUEUE_MAP_DFSV, &state));
        if (!state) {
            break;
        }
        SOC_IF_ERROR_RETURN(DRV_QUEUE_PORT_DFSV_GET
            (unit, port, code_point, &prio, queue_n));
        goto exit;
    }

exit:
    LOG_VERBOSE(BSL_LS_SOC_COMMON, \
                (BSL_META_U(unit, \
                            "drv_gex_queue_dfsv_get: \
                            unit %d, code_point = %d, *queue_n = %d\n"), unit, code_point, *queue_n));

    return SOC_E_NONE;
}

#if defined(BCM_POLAR_SUPPORT) || defined(BCM_NORTHSTAR_SUPPORT) || \
    defined(BCM_NORTHSTARPLUS_SUPPORT) || defined(BCM_STARFIGHTER3_SUPPORT)

#define DRV_GEX_TCSEL_TC_A  0   /* 2'b00:DSCP2TC */
#define DRV_GEX_TCSEL_TC_B  1   /* 2'b01:PCP2TC */
#define DRV_GEX_TCSEL_TC_C  2   /* 2'b10:DA2TC */
#define DRV_GEX_TCSEL_TC_D  3   /* 2'b11:PID2TC */
/*
 *  Function : _drv_gex2_ingress_tc_select_set
 *
 *  Purpose :
 *      To serivce the ingress TC selection on new GEX devices.
 *      (new GEX : Polar, NorthStar, NorthStarPlus and Starfighter3)
 *
 *  Parameters :
 *      unit         :  RoboSwitch unit number.
 *      bmp          :  port bitmap.
 *
 *  Return :
 *      SOC_E_XXX.
 *
 *  Note :
 */
static int 
_drv_gex2_ingress_tc_select_set(int unit, soc_pbmp_t bmp,
        uint8 type, uint8 enable)
{
    uint32  reg_value;
    uint32  temp, p;
    soc_pbmp_t in_pbmp;

    LOG_VERBOSE(BSL_LS_SOC_COMMON,
                (BSL_META_U(unit,
                            "%s: unit %d, bmp = 0x%x, type = %d, enable=%d\n"),
                 FUNCTION_NAME(), unit, SOC_PBMP_WORD_GET(bmp, 0), type, enable));

    /* port validation */
    SOC_PBMP_ASSIGN(in_pbmp, bmp);
    SOC_PBMP_REMOVE(in_pbmp, PBMP_ALL(unit));
    if (SOC_PBMP_NOT_NULL(in_pbmp)) {
        return SOC_E_PARAM;
    }

    /* check invalid setting */
    if (((type == DRV_QUEUE_MAP_NONE) || (type == DRV_QUEUE_MAP_MAC)) && 
        (enable == FALSE)) {
        return SOC_E_PARAM;
    }

    if (type == DRV_QUEUE_MAP_NONE) {
        temp = SOC_PBMP_WORD_GET(bmp, 0);
        /* enable QOS_IP_EN register */
        SOC_IF_ERROR_RETURN(REG_READ_QOS_1P_ENr
                (unit, &reg_value));
        SOC_IF_ERROR_RETURN(soc_QOS_1P_ENr_field_set
                (unit, &reg_value, QOS_1P_ENf, &temp));            
        SOC_IF_ERROR_RETURN(REG_WRITE_QOS_1P_ENr
                (unit, &reg_value));

        /* enable QOS_TOS_DIF_EN register */
        SOC_IF_ERROR_RETURN(REG_READ_QOS_EN_DIFFSERVr
                (unit, &reg_value));
        SOC_IF_ERROR_RETURN(soc_QOS_EN_DIFFSERVr_field_set
                (unit, &reg_value, QOS_EN_DIFFSERVf, &temp));
        SOC_IF_ERROR_RETURN(REG_WRITE_QOS_EN_DIFFSERVr
                (unit, &reg_value));
    }

    if ((type == DRV_QUEUE_MAP_NONE) || (type == DRV_QUEUE_MAP_PRIO) || 
            (type == DRV_QUEUE_MAP_DFSV) || (type == DRV_QUEUE_MAP_PORT) || 
            (type == DRV_QUEUE_MAP_MAC)) {

        PBMP_ITER(bmp, p) {
            if (IS_CPU_PORT(unit, p)) {
                SOC_IF_ERROR_RETURN(REG_READ_IMP_TC_SEL_TABLEr
                    (unit, &reg_value));
            } else if (!SOC_IS_STARFIGHTER3(unit) && (p == 7)) {
                SOC_IF_ERROR_RETURN(REG_READ_P7_TC_SEL_TABLEr
                    (unit, &reg_value));
            } else {
                SOC_IF_ERROR_RETURN(REG_READ_TC_SEL_TABLEr
                    (unit, p, &reg_value));
            }
            
            if ((type == DRV_QUEUE_MAP_NONE) || (type == DRV_QUEUE_MAP_MAC)) {
                temp = DRV_GEX_TCSEL_TC_C;
            } else if (type == DRV_QUEUE_MAP_PRIO) {
                temp = (enable) ? DRV_GEX_TCSEL_TC_B : DRV_GEX_TCSEL_TC_C;
            } else if (type == DRV_QUEUE_MAP_DFSV) {
                temp = (enable) ? DRV_GEX_TCSEL_TC_A : DRV_GEX_TCSEL_TC_C;
            } else if (type == DRV_QUEUE_MAP_PORT) {
                temp = (enable) ? DRV_GEX_TCSEL_TC_D : DRV_GEX_TCSEL_TC_C;
            }

            if ((type == DRV_QUEUE_MAP_NONE) || (type == DRV_QUEUE_MAP_PRIO) ||
                    (type == DRV_QUEUE_MAP_PORT)) {
                SOC_IF_ERROR_RETURN(soc_TC_SEL_TABLEr_field_set
                        (unit, &reg_value, TC_SEL_0f, &temp));
                SOC_IF_ERROR_RETURN(soc_TC_SEL_TABLEr_field_set
                        (unit, &reg_value, TC_SEL_1f, &temp));
                SOC_IF_ERROR_RETURN(soc_TC_SEL_TABLEr_field_set
                        (unit, &reg_value, TC_SEL_2f, &temp));
                SOC_IF_ERROR_RETURN(soc_TC_SEL_TABLEr_field_set
                        (unit, &reg_value, TC_SEL_3f, &temp));
                SOC_IF_ERROR_RETURN(soc_TC_SEL_TABLEr_field_set
                        (unit, &reg_value, TC_SEL_4f, &temp));
                SOC_IF_ERROR_RETURN(soc_TC_SEL_TABLEr_field_set
                        (unit, &reg_value, TC_SEL_5f, &temp));
                SOC_IF_ERROR_RETURN(soc_TC_SEL_TABLEr_field_set
                        (unit, &reg_value, TC_SEL_6f, &temp));
                SOC_IF_ERROR_RETURN(soc_TC_SEL_TABLEr_field_set
                        (unit, &reg_value, TC_SEL_7f, &temp));
            } else if (type == DRV_QUEUE_MAP_DFSV) {
                SOC_IF_ERROR_RETURN(soc_TC_SEL_TABLEr_field_set
                        (unit, &reg_value, TC_SEL_1f, &temp));
                SOC_IF_ERROR_RETURN(soc_TC_SEL_TABLEr_field_set
                        (unit, &reg_value, TC_SEL_3f, &temp));
                SOC_IF_ERROR_RETURN(soc_TC_SEL_TABLEr_field_set
                        (unit, &reg_value, TC_SEL_5f, &temp));
                SOC_IF_ERROR_RETURN(soc_TC_SEL_TABLEr_field_set
                        (unit, &reg_value, TC_SEL_7f, &temp));
            } else if (type == DRV_QUEUE_MAP_MAC) {
                SOC_IF_ERROR_RETURN(soc_TC_SEL_TABLEr_field_set
                        (unit, &reg_value, TC_SEL_4f, &temp));
                SOC_IF_ERROR_RETURN(soc_TC_SEL_TABLEr_field_set
                        (unit, &reg_value, TC_SEL_5f, &temp));
                SOC_IF_ERROR_RETURN(soc_TC_SEL_TABLEr_field_set
                        (unit, &reg_value, TC_SEL_6f, &temp));
                SOC_IF_ERROR_RETURN(soc_TC_SEL_TABLEr_field_set
                        (unit, &reg_value, TC_SEL_7f, &temp));
            }

            if (IS_CPU_PORT(unit, p)) {
                SOC_IF_ERROR_RETURN(REG_WRITE_IMP_TC_SEL_TABLEr
                    (unit, &reg_value));
            } else if (!SOC_IS_STARFIGHTER3(unit) && (p == 7)) {
                SOC_IF_ERROR_RETURN(REG_WRITE_P7_TC_SEL_TABLEr
                    (unit, &reg_value));
            } else {
                SOC_IF_ERROR_RETURN(REG_WRITE_TC_SEL_TABLEr
                    (unit, p, &reg_value));
            }
        }
    } else {
        return SOC_E_UNAVAIL;
    }

    return SOC_E_NONE;
}

/*
 *  Function : _drv_gex2_ingress_tc_select_get
 *
 *  Purpose :
 *      To serivce the ingress TC selection on new GEX devices.
 *      (new GEX : Polar, NorthStar, NorthStarPlus and Starfighter3)
 *
 *  Parameters :
 *      unit         :  RoboSwitch unit number.
 *      bmp          :  port bitmap.
 *
 *  Return :
 *      SOC_E_XXX.
 *
 *  Note :
 */
static int 
_drv_gex2_ingress_tc_select_get(int unit, int port, uint8 type, uint8 *enable)
{
    uint32  reg_value;
    uint32  temp, expect_val, is_unexpected;

    LOG_VERBOSE(BSL_LS_SOC_COMMON,
                (BSL_META_U(unit,
                            "%s: unit %d, port = 0x%x, type = %d\n"),
                 FUNCTION_NAME(), unit, port, type));

    /* port validation */
    if (!SOC_PBMP_MEMBER(PBMP_ALL(unit), port)){
        return SOC_E_PARAM;
    }

    /* NONE is for set only */
    if (type == DRV_QUEUE_MAP_NONE) {
        return SOC_E_PARAM;
    }

    if ((type == DRV_QUEUE_MAP_MAC) || (type == DRV_QUEUE_MAP_PRIO) || 
            (type == DRV_QUEUE_MAP_DFSV) || (type == DRV_QUEUE_MAP_PORT)) {

        if (IS_CPU_PORT(unit, port)) {
            SOC_IF_ERROR_RETURN(REG_READ_IMP_TC_SEL_TABLEr
                (unit, &reg_value));
        } else if (!SOC_IS_STARFIGHTER3(unit) && (port == 7)) {
            SOC_IF_ERROR_RETURN(REG_READ_P7_TC_SEL_TABLEr
                (unit, &reg_value));
        } else {
            SOC_IF_ERROR_RETURN(REG_READ_TC_SEL_TABLEr
                (unit, port, &reg_value));
        }
            
        if (type == DRV_QUEUE_MAP_MAC) {
            expect_val = DRV_GEX_TCSEL_TC_C;
        } else if (type == DRV_QUEUE_MAP_PRIO) {
            expect_val = DRV_GEX_TCSEL_TC_B;
        } else if (type == DRV_QUEUE_MAP_DFSV) {
            expect_val = DRV_GEX_TCSEL_TC_A;
        } else {
            /* DRV_QUEUE_MAP_PORT */
            expect_val = DRV_GEX_TCSEL_TC_D;
        }

        is_unexpected = FALSE;
        if ((type == DRV_QUEUE_MAP_PRIO) || (type == DRV_QUEUE_MAP_PORT)) {
            SOC_IF_ERROR_RETURN(soc_TC_SEL_TABLEr_field_get
                    (unit, &reg_value, TC_SEL_0f, &temp));
            is_unexpected |= (temp != expect_val);
            SOC_IF_ERROR_RETURN(soc_TC_SEL_TABLEr_field_get
                    (unit, &reg_value, TC_SEL_1f, &temp));
            is_unexpected |= (temp != expect_val);
            SOC_IF_ERROR_RETURN(soc_TC_SEL_TABLEr_field_get
                    (unit, &reg_value, TC_SEL_2f, &temp));
            is_unexpected |= (temp != expect_val);
            SOC_IF_ERROR_RETURN(soc_TC_SEL_TABLEr_field_get
                    (unit, &reg_value, TC_SEL_3f, &temp));
            is_unexpected |= (temp != expect_val);
            SOC_IF_ERROR_RETURN(soc_TC_SEL_TABLEr_field_get
                    (unit, &reg_value, TC_SEL_4f, &temp));
            is_unexpected |= (temp != expect_val);
            SOC_IF_ERROR_RETURN(soc_TC_SEL_TABLEr_field_get
                    (unit, &reg_value, TC_SEL_5f, &temp));
            is_unexpected |= (temp != expect_val);
            SOC_IF_ERROR_RETURN(soc_TC_SEL_TABLEr_field_get
                    (unit, &reg_value, TC_SEL_6f, &temp));
            is_unexpected |= (temp != expect_val);
            SOC_IF_ERROR_RETURN(soc_TC_SEL_TABLEr_field_get
                    (unit, &reg_value, TC_SEL_7f, &temp));
            is_unexpected |= (temp != expect_val);

        } else if (type == DRV_QUEUE_MAP_DFSV) {
            SOC_IF_ERROR_RETURN(soc_TC_SEL_TABLEr_field_get
                    (unit, &reg_value, TC_SEL_1f, &temp));
            is_unexpected |= (temp != expect_val);
            SOC_IF_ERROR_RETURN(soc_TC_SEL_TABLEr_field_get
                    (unit, &reg_value, TC_SEL_3f, &temp));
            is_unexpected |= (temp != expect_val);
            SOC_IF_ERROR_RETURN(soc_TC_SEL_TABLEr_field_get
                    (unit, &reg_value, TC_SEL_5f, &temp));
            is_unexpected |= (temp != expect_val);
            SOC_IF_ERROR_RETURN(soc_TC_SEL_TABLEr_field_get
                    (unit, &reg_value, TC_SEL_7f, &temp));
            is_unexpected |= (temp != expect_val);
        } else if (type == DRV_QUEUE_MAP_MAC) {
            SOC_IF_ERROR_RETURN(soc_TC_SEL_TABLEr_field_get
                    (unit, &reg_value, TC_SEL_4f, &temp));
            is_unexpected |= (temp != expect_val);
            SOC_IF_ERROR_RETURN(soc_TC_SEL_TABLEr_field_get
                    (unit, &reg_value, TC_SEL_5f, &temp));
            is_unexpected |= (temp != expect_val);
            SOC_IF_ERROR_RETURN(soc_TC_SEL_TABLEr_field_get
                    (unit, &reg_value, TC_SEL_6f, &temp));
            is_unexpected |= (temp != expect_val);
            SOC_IF_ERROR_RETURN(soc_TC_SEL_TABLEr_field_get
                    (unit, &reg_value, TC_SEL_7f, &temp));
            is_unexpected |= (temp != expect_val);
        }
        
        /* TRUE if expected setting on all fields */
        *enable = (is_unexpected) ? FALSE : TRUE;
    } else {
        return SOC_E_UNAVAIL;
    }

    return SOC_E_NONE;
}

#endif /* BCM_POLAR_SUPPORT || BCM_NORTHSTAR_SUPPORT || \
 *  BCM_NORTHSTARPLUS_SUPPORT || BCM_STARFIGHTER3_SUPPORT */

/* Enable/disable selected mapping - prio/diffserv/tos */
/*
 *  Function : drv_gex_queue_mapping_type_set
 *
 *  Purpose :
 *      Set the state to the specific queue mapping type.
 *
 *  Parameters :
 *      unit         :  RoboSwitch unit number.
 *      bmp          :  port bitmap.
 *      mapping_type :  queue mapping type (prio/tos/diffserv).
 *      state        :  The state of the selected mapping type.
 *
 *  Return :
 *      SOC_E_XXX.
 *
 *  Note :
 */
int 
drv_gex_queue_mapping_type_set(int unit, soc_pbmp_t bmp, 
    uint32 mapping_type, uint8 state)
{
    uint32  reg_value;
    uint32  temp;

    LOG_VERBOSE(BSL_LS_SOC_COMMON, \
                (BSL_META_U(unit, \
                            "drv_gex_queue_mapping_type_set: \
                            unit %d, bmp = 0x%x, type = %d, %sable\n"),
                 unit, SOC_PBMP_WORD_GET(bmp, 0), mapping_type, (state) ? "en" : "dis"));

#if defined(BCM_POLAR_SUPPORT) || defined(BCM_NORTHSTAR_SUPPORT) || \
    defined(BCM_NORTHSTARPLUS_SUPPORT) || defined(BCM_STARFIGHTER3_SUPPORT)

    if (SOC_IS_POLAR(unit) || SOC_IS_NORTHSTAR(unit) ||
            SOC_IS_NORTHSTARPLUS(unit) || SOC_IS_STARFIGHTER3(unit)) {
        if ((mapping_type == DRV_QUEUE_MAP_NONE) || 
                (mapping_type == DRV_QUEUE_MAP_PRIO) || 
                (mapping_type == DRV_QUEUE_MAP_DFSV) || 
                (mapping_type == DRV_QUEUE_MAP_PORT) || 
                (mapping_type == DRV_QUEUE_MAP_MAC)) {
            return _drv_gex2_ingress_tc_select_set(unit, bmp, mapping_type, state);
        } else {
            LOG_VERBOSE(BSL_LS_SOC_COMMON,
                        (BSL_META_U(unit,
                                    "Unavailable mapping type %d!\n"),mapping_type));
            return SOC_E_UNAVAIL;
        }
    }
#endif /* BCM_POLAR_SUPPORT || BCM_NORTHSTAR_SUPPORT || BCM_NORTHSTARPLUS_SUPPORT
          || BCM_STARFIGHTER3_SUPPORT */

    switch (mapping_type) {
        case DRV_QUEUE_MAP_NONE:

            if (!(SOC_IS_STARFIGHTER3(unit))) {
                /* Set port_qos_en=1 && qos_layer_sel = 0 */
                SOC_IF_ERROR_RETURN(REG_READ_QOS_GLOBAL_CTRLr
                    (unit, &reg_value));

                temp = GEX_QUEUE_PORT_QOS_ENABLE;
                SOC_IF_ERROR_RETURN(soc_QOS_GLOBAL_CTRLr_field_set
                    (unit, &reg_value, PORT_QOS_ENf, &temp));

                temp = GEX_QUEUE_QOS_LAYER_SEL_PRIO;
                SOC_IF_ERROR_RETURN(soc_QOS_GLOBAL_CTRLr_field_set
                    (unit, &reg_value, QOS_LAYER_SELf, &temp));        

                SOC_IF_ERROR_RETURN(REG_WRITE_QOS_GLOBAL_CTRLr
                    (unit, &reg_value));
            }
    
            /* Disable QOS_IP_EN register */
            SOC_IF_ERROR_RETURN(REG_READ_QOS_1P_ENr
                (unit, &reg_value));
            SOC_IF_ERROR_RETURN(soc_QOS_1P_ENr_field_get
                (unit, &reg_value, QOS_1P_ENf, &temp));
            temp &= ~(SOC_PBMP_WORD_GET(bmp, 0));
            SOC_IF_ERROR_RETURN(soc_QOS_1P_ENr_field_set
                (unit, &reg_value, QOS_1P_ENf, &temp));
            SOC_IF_ERROR_RETURN(REG_WRITE_QOS_1P_ENr
                (unit, &reg_value));
    
            /* Disable QOS_TOS_DIF_EN register */
            SOC_IF_ERROR_RETURN(REG_READ_QOS_EN_DIFFSERVr
                (unit, &reg_value));
            SOC_IF_ERROR_RETURN(soc_QOS_EN_DIFFSERVr_field_get
                (unit, &reg_value, QOS_EN_DIFFSERVf, &temp));
            temp &= ~(SOC_PBMP_WORD_GET(bmp, 0));
            SOC_IF_ERROR_RETURN(soc_QOS_EN_DIFFSERVr_field_set
                (unit, &reg_value, QOS_EN_DIFFSERVf, &temp));
            SOC_IF_ERROR_RETURN(REG_WRITE_QOS_EN_DIFFSERVr
                (unit, &reg_value));
            break;
        case DRV_QUEUE_MAP_PRIO:
            if (!(SOC_IS_STARFIGHTER3(unit))) {
                /* Set port_qos_en=0 && qos_layer_sel = 0b (layer2 only) */
                SOC_IF_ERROR_RETURN(REG_READ_QOS_GLOBAL_CTRLr
                    (unit, &reg_value));

                temp = GEX_QUEUE_PORT_QOS_DISABLE;
                SOC_IF_ERROR_RETURN(soc_QOS_GLOBAL_CTRLr_field_set
                    (unit, &reg_value, PORT_QOS_ENf, &temp));

                temp = GEX_QUEUE_QOS_LAYER_SEL_PRIO;
                SOC_IF_ERROR_RETURN(soc_QOS_GLOBAL_CTRLr_field_set
                    (unit, &reg_value, QOS_LAYER_SELf, &temp));

                SOC_IF_ERROR_RETURN(REG_WRITE_QOS_GLOBAL_CTRLr
                    (unit, &reg_value));
            }
    
            /* Enable/disable QOS_1P_ENr */
            SOC_IF_ERROR_RETURN(REG_READ_QOS_1P_ENr
                (unit, &reg_value));
            SOC_IF_ERROR_RETURN(soc_QOS_1P_ENr_field_get
                (unit, &reg_value, QOS_1P_ENf, &temp));
            if (state) {
                temp |= (SOC_PBMP_WORD_GET(bmp, 0));
            } else {
                temp &= ~(SOC_PBMP_WORD_GET(bmp, 0));
            }
            SOC_IF_ERROR_RETURN(soc_QOS_1P_ENr_field_set
                (unit, &reg_value, QOS_1P_ENf, &temp));
            SOC_IF_ERROR_RETURN(REG_WRITE_QOS_1P_ENr
                (unit, &reg_value));
            /* Don't need to disable QOS_TOS_DIF_ENr, 
              * the priority of QOS_1P_ENr > QOS_TOS_DIF_ENr
              * by TC Decision Tree : port_qos_en=0 && qos_layer_sel = 0b 
              */
            break;
        case DRV_QUEUE_MAP_DFSV:
            if (!(SOC_IS_STARFIGHTER3(unit))) {
                /* Set port_qos_en=0 && qos_layer_sel = 10b */
                SOC_IF_ERROR_RETURN(
                    REG_READ_QOS_GLOBAL_CTRLr(unit, &reg_value));

                temp = GEX_QUEUE_PORT_QOS_DISABLE;
                SOC_IF_ERROR_RETURN(soc_QOS_GLOBAL_CTRLr_field_set
                    (unit, &reg_value, PORT_QOS_ENf, &temp));

                temp = GEX_QUEUE_QOS_LAYER_SEL_IP;
                SOC_IF_ERROR_RETURN(soc_QOS_GLOBAL_CTRLr_field_set
                    (unit, &reg_value, QOS_LAYER_SELf, &temp));

                SOC_IF_ERROR_RETURN(REG_WRITE_QOS_GLOBAL_CTRLr
                    (unit, &reg_value));
            }
    
            /* Enable / diaable QOS_TOS_DIF_ENr */
            SOC_IF_ERROR_RETURN(
                REG_READ_QOS_EN_DIFFSERVr(unit, &reg_value));
            SOC_IF_ERROR_RETURN(soc_QOS_EN_DIFFSERVr_field_get
                (unit, &reg_value, QOS_EN_DIFFSERVf, &temp));
            if (state) {
                temp |= (SOC_PBMP_WORD_GET(bmp, 0));
            } else {
                temp &= ~(SOC_PBMP_WORD_GET(bmp, 0));
            }
            SOC_IF_ERROR_RETURN(soc_QOS_EN_DIFFSERVr_field_set
                (unit, &reg_value, QOS_EN_DIFFSERVf, &temp));
            SOC_IF_ERROR_RETURN(REG_WRITE_QOS_EN_DIFFSERVr
                (unit, &reg_value));
            /* Don't need to disable QOS_1P_ENr, 
              * the priority of QOS_TOS_DIF_ENr > QOS_1P_ENr
              * by TC Decision Tree : port_qos_en=0 && qos_layer_sel = 10b 
              */
            break;
        case DRV_QUEUE_MAP_PORT:
            /* enable/disable port based priority :
             *  - port_qos_en=0
             */
            if (SOC_PBMP_NEQ(bmp, PBMP_ALL(unit))) {
                return SOC_E_UNAVAIL;   /* system based configuration only */
            }
            if (SOC_IS_STARFIGHTER3(unit)) {
                return SOC_E_UNAVAIL;   
            }
            SOC_IF_ERROR_RETURN(REG_READ_QOS_GLOBAL_CTRLr
                (unit, &reg_value));

            if (state != 0) {
                temp = GEX_QUEUE_PORT_QOS_ENABLE;
            } else {
                temp = GEX_QUEUE_PORT_QOS_DISABLE;
            }
            SOC_IF_ERROR_RETURN(soc_QOS_GLOBAL_CTRLr_field_set
                (unit, &reg_value, PORT_QOS_ENf, &temp));
            SOC_IF_ERROR_RETURN(REG_WRITE_QOS_GLOBAL_CTRLr
                (unit, &reg_value));
            break;
        case DRV_QUEUE_MAP_HYBRID:
            if (!(SOC_IS_STARFIGHTER3(unit))) {
                /* Set port_qos_en=1 && qos_layer_sel = 3  */
                SOC_IF_ERROR_RETURN(
                    REG_READ_QOS_GLOBAL_CTRLr(unit, &reg_value));

                temp = GEX_QUEUE_PORT_QOS_ENABLE;
                SOC_IF_ERROR_RETURN(soc_QOS_GLOBAL_CTRLr_field_set
                    (unit, &reg_value, PORT_QOS_ENf, &temp));

                temp = GEX_QUEUE_QOS_LAYER_SEL_ALL;
                SOC_IF_ERROR_RETURN(soc_QOS_GLOBAL_CTRLr_field_set
                    (unit, &reg_value, QOS_LAYER_SELf, &temp));
                SOC_IF_ERROR_RETURN(REG_WRITE_QOS_GLOBAL_CTRLr
                    (unit, &reg_value));
            }

             /* Enable QOS_IP_EN register */
            SOC_IF_ERROR_RETURN(REG_READ_QOS_1P_ENr
                (unit, &reg_value));
            SOC_IF_ERROR_RETURN(soc_QOS_1P_ENr_field_get
                (unit, &reg_value, QOS_1P_ENf, &temp));
            temp |= (SOC_PBMP_WORD_GET(bmp, 0));
            SOC_IF_ERROR_RETURN(soc_QOS_1P_ENr_field_set
                (unit, &reg_value, QOS_1P_ENf, &temp));
            SOC_IF_ERROR_RETURN(REG_WRITE_QOS_1P_ENr
                (unit, &reg_value));
    
            /* Enable QOS_TOS_DIF_EN register */
            SOC_IF_ERROR_RETURN(REG_READ_QOS_EN_DIFFSERVr
                (unit, &reg_value));
            SOC_IF_ERROR_RETURN(soc_QOS_EN_DIFFSERVr_field_get
                (unit, &reg_value, QOS_EN_DIFFSERVf, &temp));
            temp |= (SOC_PBMP_WORD_GET(bmp, 0));
            SOC_IF_ERROR_RETURN(soc_QOS_EN_DIFFSERVr_field_set
                (unit, &reg_value, QOS_EN_DIFFSERVf, &temp));
            SOC_IF_ERROR_RETURN(REG_WRITE_QOS_EN_DIFFSERVr
                (unit, &reg_value));
            break;
        case DRV_QUEUE_MAP_TOS:
        case DRV_QUEUE_MAP_MAC:
            return SOC_E_UNAVAIL;
        default :
            return SOC_E_PARAM;
    }

    return SOC_E_NONE;
}

/*
 *  Function : drv_gex_queue_mapping_type_get
 *
 *  Purpose :
 *      Get the state to the specific queue mapping type.
 *
 *  Parameters :
 *      unit         :  RoboSwitch unit number.
 *      port         :  port number.
 *      mapping_type :  queue mapping type (prio/tos/diffserv).
 *      state        :  (OUT) The state of the selected mapping type.
 *
 *  Return :
 *      SOC_E_XXX.
 *
 *  Note :
 */
int 
drv_gex_queue_mapping_type_get(int unit, uint32 port, 
    uint32 mapping_type, uint8 *state)
{
    uint32  reg_value;
    uint32  temp = 0;

#if defined(BCM_POLAR_SUPPORT) || defined(BCM_NORTHSTAR_SUPPORT) || \
    defined(BCM_NORTHSTARPLUS_SUPPORT) || defined(BCM_STARFIGHTER3_SUPPORT)

    if (SOC_IS_POLAR(unit) || SOC_IS_NORTHSTAR(unit) ||
            SOC_IS_NORTHSTARPLUS(unit) || SOC_IS_STARFIGHTER3(unit)) {
        if ((mapping_type == DRV_QUEUE_MAP_NONE) || 
                (mapping_type == DRV_QUEUE_MAP_PRIO) || 
                (mapping_type == DRV_QUEUE_MAP_DFSV) || 
                (mapping_type == DRV_QUEUE_MAP_PORT) || 
                (mapping_type == DRV_QUEUE_MAP_MAC)) {
            SOC_IF_ERROR_RETURN(_drv_gex2_ingress_tc_select_get
                    (unit, port, mapping_type, state));
            goto func_end;
        } else {
            LOG_VERBOSE(BSL_LS_SOC_COMMON,
                        (BSL_META_U(unit,
                                    "Unavailable mapping type %d!\n"),mapping_type));
            return SOC_E_UNAVAIL;
        }
    }
#endif /* BCM_POLAR_SUPPORT || BCM_NORTHSTAR_SUPPORT || \
 *  BCM_NORTHSTARPLUS_SUPPORT || BCM_STARFIGHTER3_SUPPORT */

    switch (mapping_type)
    {
        case DRV_QUEUE_MAP_NONE:
            return SOC_E_PARAM;
        case DRV_QUEUE_MAP_PRIO:
            if (!(SOC_IS_STARFIGHTER3(unit))) {
                SOC_IF_ERROR_RETURN(REG_READ_QOS_GLOBAL_CTRLr
                    (unit, &reg_value));
                SOC_IF_ERROR_RETURN(soc_QOS_GLOBAL_CTRLr_field_get
                    (unit, &reg_value, PORT_QOS_ENf, &temp));
                if (temp) {
                    *state = FALSE;
                    break;
                }
                SOC_IF_ERROR_RETURN(soc_QOS_GLOBAL_CTRLr_field_get
                    (unit, &reg_value, QOS_LAYER_SELf, &temp));
                if (temp == GEX_QUEUE_QOS_LAYER_SEL_DFSV) {
                    *state = FALSE;
                    break;
                }
            }

            SOC_IF_ERROR_RETURN(REG_READ_QOS_EN_DIFFSERVr
                (unit, &reg_value));
            SOC_IF_ERROR_RETURN(soc_QOS_EN_DIFFSERVr_field_get
                (unit, &reg_value, QOS_EN_DIFFSERVf, &temp));
            if (temp & (0x1 << port)) {
                *state = FALSE;
                break;
            }
            SOC_IF_ERROR_RETURN(REG_READ_QOS_1P_ENr
                (unit, &reg_value));
            SOC_IF_ERROR_RETURN(soc_QOS_1P_ENr_field_get
                (unit, &reg_value, QOS_1P_ENf, &temp));
            if (temp & (0x1 << port)) {
                *state = TRUE;
            } else {
                *state = FALSE;
            }
            break;
        case DRV_QUEUE_MAP_DFSV:
            if (!(SOC_IS_STARFIGHTER3(unit))) {
                SOC_IF_ERROR_RETURN(REG_READ_QOS_GLOBAL_CTRLr
                    (unit, &reg_value));
                SOC_IF_ERROR_RETURN(soc_QOS_GLOBAL_CTRLr_field_get
                    (unit, &reg_value, PORT_QOS_ENf, &temp));
                if (temp) {
                    *state = FALSE;
                    break;
                }
                SOC_IF_ERROR_RETURN(soc_QOS_GLOBAL_CTRLr_field_get
                    (unit, &reg_value, QOS_LAYER_SELf, &temp));
                if (temp == GEX_QUEUE_QOS_LAYER_SEL_PRIO) {
                    *state = FALSE;
                    break;
                }
            }
            SOC_IF_ERROR_RETURN(REG_READ_QOS_EN_DIFFSERVr
                (unit, &reg_value));
            SOC_IF_ERROR_RETURN(soc_QOS_EN_DIFFSERVr_field_get
                (unit, &reg_value, QOS_EN_DIFFSERVf, &temp));
           if (temp & (0x1 << port)) {
                *state = TRUE;
            } else {
                *state = FALSE;
            }
            break;
        case DRV_QUEUE_MAP_PORT:
            /* Enable/disable port based priority :
             *  - port_qos_en = 0
             */
            if (!(SOC_IS_STARFIGHTER3(unit))) {
                SOC_IF_ERROR_RETURN(REG_READ_QOS_GLOBAL_CTRLr
                    (unit, &reg_value));
                SOC_IF_ERROR_RETURN(soc_QOS_GLOBAL_CTRLr_field_get
                    (unit, &reg_value, PORT_QOS_ENf, &temp));
                *state = (temp == GEX_QUEUE_PORT_QOS_ENABLE) ? TRUE : FALSE;
             }
             break;
        case DRV_QUEUE_MAP_HYBRID:
            if (!(SOC_IS_STARFIGHTER3(unit))) {
                SOC_IF_ERROR_RETURN(REG_READ_QOS_GLOBAL_CTRLr
                    (unit, &reg_value));
                SOC_IF_ERROR_RETURN(soc_QOS_GLOBAL_CTRLr_field_get
                    (unit, &reg_value, PORT_QOS_ENf, &temp));
                if (!temp) {
                    *state = FALSE;
                    break;
                }
                SOC_IF_ERROR_RETURN(soc_QOS_GLOBAL_CTRLr_field_get
                    (unit, &reg_value, QOS_LAYER_SELf, &temp));
                if (temp == GEX_QUEUE_QOS_LAYER_SEL_ALL) {
                    *state = TRUE;
                } else {
                    *state = FALSE;
                }
            }
            break;
        case DRV_QUEUE_MAP_TOS:
        case DRV_QUEUE_MAP_MAC:
            return SOC_E_UNAVAIL;
        default:
            return SOC_E_PARAM;
    }

#if defined(BCM_POLAR_SUPPORT) || defined(BCM_NORTHSTAR_SUPPORT) || \
    defined(BCM_NORTHSTARPLUS_SUPPORT) || defined(BCM_STARFIGHTER3_SUPPORT)
func_end : 
#endif /* BCM_POLAR_SUPPORT || BCM_NORTHSTAR_SUPPORT || \
 *  BCM_NORTHSTARPLUS_SUPPORT || BCM_STARFIGHTER3_SUPPORT */

    LOG_VERBOSE(BSL_LS_SOC_COMMON, \
                (BSL_META_U(unit, \
                            "drv_gex_queue_mapping_type_get: \
                            unit %d, port = %d, type = %d, %sable\n"),
                 unit, port, mapping_type, (*state) ? "en" : "dis"));

    return SOC_E_NONE;
}

/*
 *  Function : drv_gex_queue_rx_reason_set
 *
 *  Purpose :
 *      Set the queue id base on the rx reason code.
 *
 *  Parameters :
 *      unit     :  RoboSwitch unit number.
 *      reason   :  rx reason code.
 *      queue    :  queue ID.
 *
 *  Return :
 *      SOC_E_XXX.
 *
 *  Note :
 */
int 
drv_gex_queue_rx_reason_set(int unit, uint8 reason, uint32 queue)
{
    uint32  reg_value, temp;

    LOG_VERBOSE(BSL_LS_SOC_COMMON, \
                (BSL_META_U(unit, \
                            "drv_gex_queue_rx_reason_set: \
                            unit %d, reason = %d, queue = %d\n"), unit, reason, queue));

    SOC_IF_ERROR_RETURN(REG_READ_CPU2COS_MAPr
        (unit, &reg_value));

    temp = queue;
    switch (reason) {
        case DRV_RX_REASON_MIRRORING:
            if (SOC_IS_VULCAN(unit) || SOC_IS_BLACKBIRD(unit)) {
                SOC_IF_ERROR_RETURN(soc_CPU2COS_MAPr_field_set
                    (unit, &reg_value, MIRROR_Rf, &temp));
            } else if (SOC_IS_LOTUS(unit) || SOC_IS_STARFIGHTER(unit) ||
                SOC_IS_BLACKBIRD2(unit) || SOC_IS_POLAR(unit) ||
                SOC_IS_NORTHSTAR(unit) || SOC_IS_NORTHSTARPLUS(unit) ||
                SOC_IS_STARFIGHTER3(unit)) {
                SOC_IF_ERROR_RETURN(soc_CPU2COS_MAPr_field_set
                    (unit, &reg_value, MIRRORf, &temp));
            }
            break;
        case DRV_RX_REASON_SA_LEARNING:
            SOC_IF_ERROR_RETURN(soc_CPU2COS_MAPr_field_set
                (unit, &reg_value, SA_LRNf, &temp));
            break;
        case DRV_RX_REASON_SWITCHING:
           SOC_IF_ERROR_RETURN(soc_CPU2COS_MAPr_field_set
                (unit, &reg_value, SW_FLDf, &temp));
            break;
        case DRV_RX_REASON_PROTO_TERM:
            SOC_IF_ERROR_RETURN(soc_CPU2COS_MAPr_field_set
                (unit, &reg_value, PRTC_TRMNTf, &temp));
            break;
        case DRV_RX_REASON_PROTO_SNOOP:
            SOC_IF_ERROR_RETURN(soc_CPU2COS_MAPr_field_set
                (unit, &reg_value, PRTC_SNOOPf, &temp));
            break;
        case DRV_RX_REASON_EXCEPTION:
            SOC_IF_ERROR_RETURN(soc_CPU2COS_MAPr_field_set
                (unit, &reg_value, EXCPT_PRCSf, &temp));
            break;
        default:
            return SOC_E_UNAVAIL;
    }
    SOC_IF_ERROR_RETURN(REG_WRITE_CPU2COS_MAPr
        (unit, &reg_value));

    return SOC_E_NONE;
}

/*
 *  Function : drv_gex_queue_rx_reason_get
 *
 *  Purpose :
 *      Get the queue id according to the rx reason code.
 *
 *  Parameters :
 *      unit     :  RoboSwitch unit number.
 *      reason   :  rx reason code.
 *      queue    :  (OUT) queue ID.
 *
 *  Return :
 *      SOC_E_XXX.
 *
 *  Note :
 */
int 
drv_gex_queue_rx_reason_get(int unit, uint8 reason, uint32 *queue)
{
    uint32  reg_value, temp = 0;

    SOC_IF_ERROR_RETURN(REG_READ_CPU2COS_MAPr
        (unit, &reg_value));

    switch (reason) {
        case DRV_RX_REASON_MIRRORING:
            if (SOC_IS_VULCAN(unit) || SOC_IS_BLACKBIRD(unit)) {
                SOC_IF_ERROR_RETURN(soc_CPU2COS_MAPr_field_get
                    (unit, &reg_value, MIRROR_Rf, &temp));
            } else if (SOC_IS_LOTUS(unit)|| SOC_IS_STARFIGHTER(unit) ||
                SOC_IS_BLACKBIRD2(unit) || SOC_IS_POLAR(unit) ||
                SOC_IS_NORTHSTAR(unit) || SOC_IS_NORTHSTARPLUS(unit) ||
                SOC_IS_STARFIGHTER3(unit)) {
                SOC_IF_ERROR_RETURN(soc_CPU2COS_MAPr_field_get
                    (unit, &reg_value, MIRRORf, &temp));
            }
            *queue = temp;
            break;
        case DRV_RX_REASON_SA_LEARNING:
            SOC_IF_ERROR_RETURN(soc_CPU2COS_MAPr_field_get
                (unit, &reg_value, SA_LRNf, &temp));
            *queue = temp;
            break;
        case DRV_RX_REASON_SWITCHING:
            SOC_IF_ERROR_RETURN(soc_CPU2COS_MAPr_field_get
                (unit, &reg_value, SW_FLDf, &temp));
            *queue = temp;
            break;
        case DRV_RX_REASON_PROTO_TERM:
            SOC_IF_ERROR_RETURN(soc_CPU2COS_MAPr_field_get
                (unit, &reg_value, PRTC_TRMNTf, &temp));
            *queue = temp;
            break;
        case DRV_RX_REASON_PROTO_SNOOP:
            SOC_IF_ERROR_RETURN(soc_CPU2COS_MAPr_field_get
                (unit, &reg_value, PRTC_SNOOPf, &temp));
            *queue = temp;
            break;
        case DRV_RX_REASON_EXCEPTION:
            SOC_IF_ERROR_RETURN(soc_CPU2COS_MAPr_field_get
                (unit, &reg_value, EXCPT_PRCSf, &temp));
            *queue = temp;
            break;
        default:
            return SOC_E_UNAVAIL;
    }

    LOG_VERBOSE(BSL_LS_SOC_COMMON, \
                (BSL_META_U(unit, \
                            "drv_gex_queue_rx_reason_get: \
                            unit %d, reason = %d, *queue = %d\n"), unit, reason, *queue));

    return SOC_E_NONE;
}

