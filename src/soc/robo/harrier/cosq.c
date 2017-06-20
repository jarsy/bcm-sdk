/*
 * $Id: cosq.c,v 1.1 Broadcom SDK $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include <shared/bsl.h>

#include <soc/robo.h>
#include <soc/drv.h>
#include <soc/debug.h>

/*
 *  Function : drv_harrier_queue_mode_set
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
drv_harrier_queue_mode_set(int unit, soc_pbmp_t bmp, uint32 flag, 
    uint32 mode)
{
    uint32  temp;
    uint64  reg_value64;

    LOG_VERBOSE(BSL_LS_SOC_COMMON, \
                (BSL_META_U(unit, \
                            "drv_harrier_queue_mode_set: \
                            unit %d, bmp = 0x%x, flag = 0x%x, queue mode = %d\n"), 
                 unit, SOC_PBMP_WORD_GET(bmp, 0), flag, mode));

    if (flag != 0) {
        return SOC_E_PARAM;
    }

    SOC_IF_ERROR_RETURN(REG_READ_QOS_CTLr
        (unit, (uint32 *)&reg_value64));

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
    SOC_IF_ERROR_RETURN(soc_QOS_CTLr_field_set
        (unit, (uint32 *)&reg_value64, SCHEDULE_SELECTf, &temp));
    SOC_IF_ERROR_RETURN(REG_WRITE_QOS_CTLr
        (unit, (uint32 *)&reg_value64));

    return SOC_E_NONE;
}

/*
 *  Function : drv_harrier_queue_mode_get
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
drv_harrier_queue_mode_get(int unit, uint32 port, uint32 flag, 
    uint32 *mode)
{
    uint32  temp;
    uint64  reg_value64;

    if (flag != 0) {
        return SOC_E_PARAM;
    }

    /* Check port number */
    if (port > (SOC_MAX_NUM_PORTS - 1)) {
        return SOC_E_PARAM;
    }

    SOC_IF_ERROR_RETURN(REG_READ_QOS_CTLr
        (unit, (uint32 *)&reg_value64));
    SOC_IF_ERROR_RETURN(soc_QOS_CTLr_field_get
        (unit, (uint32 *)&reg_value64, SCHEDULE_SELECTf, &temp));

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
                            "drv_harrier_queue_mode_get: \
                            unit %d, port = %d, flag = 0x%x, queue mode = %d\n"),
                 unit, port, flag, *mode));

    return SOC_E_NONE;
}

/*
 *  Function : drv_harrier_queue_count_set
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
drv_harrier_queue_count_set(int unit, uint32 port_type, uint8 count)
{
    uint32  num_q, temp;
    uint32  max_numq;
    uint64  reg_value64;
    int     cos, prio, ratio, remain;

    LOG_VERBOSE(BSL_LS_SOC_COMMON, \
                (BSL_META_U(unit, \
                            "drv_harrier_queue_count_set: \
                            unit %d, port type = %d, queue count = %d\n"), unit, port_type, count));

    max_numq = NUM_COS(unit);
    if ((count > max_numq) || (count < 1)) {
        return SOC_E_PARAM;
    }
    /* Reserved for 2, 3 queues */
    if ((count == 2) || (count == 3)) {
        return SOC_E_PARAM;
    }

    SOC_IF_ERROR_RETURN(REG_READ_QOS_CTLr
        (unit, (uint32 *)&reg_value64));

    /* Enable CPU_Control_Enable bit */
    temp = 1;
    SOC_IF_ERROR_RETURN(soc_QOS_CTLr_field_set
        (unit, (uint32 *)&reg_value64, CPU_CTRL_ENf, &temp));

    /* Set queue number = 4 or 0 (disable) */
    num_q = (count - 1);
    SOC_IF_ERROR_RETURN(soc_QOS_CTLr_field_set
        (unit, (uint32 *)&reg_value64, QOS_ENf, &num_q));

    /* Write register */
    SOC_IF_ERROR_RETURN(REG_WRITE_QOS_CTLr
        (unit, (uint32 *)&reg_value64));

    /* Map the eight 802.1 priority levels to the active cosqs */
    ratio = (8 / count);
    remain = (8 % count);
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
 *  Function : drv_harrier_queue_count_get
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
drv_harrier_queue_count_get(int unit, uint32 port_type, uint8 *count)
{
    uint32  num_q = 0;
    uint64  reg_value64;

    SOC_IF_ERROR_RETURN(REG_READ_QOS_CTLr
        (unit, (uint32 *)&reg_value64));
    SOC_IF_ERROR_RETURN(soc_QOS_CTLr_field_get
        (unit, (uint32 *)&reg_value64, QOS_ENf, &num_q));
    *count = (num_q + 1);

    LOG_VERBOSE(BSL_LS_SOC_COMMON, \
                (BSL_META_U(unit, \
                            "drv_harrier_queue_count_get: \
                            unit %d, port type = %d, queue count = %d\n"), unit, port_type, *count));

    return SOC_E_NONE;
}

/* Set WRR weight */
/*
 *  Function : drv_harrier_queue_WRR_weight_set
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
drv_harrier_queue_WRR_weight_set(int unit, uint32 port_type, 
    soc_pbmp_t bmp, uint8 queue, uint32 weight)
{
    uint32  reg_addr, reg_value, temp;
    uint32  reg_index, fld_index;
    uint32  max_weight = 0;
    int     reg_len;

    LOG_VERBOSE(BSL_LS_SOC_COMMON, \
                (BSL_META_U(unit, \
                            "drv_harrier_queue_WRR_weight_set: \
                            unit %d, port type = %d, bmp = 0x%x, queue = %d, weight = %d\n"),
                 unit, port_type, SOC_PBMP_WORD_GET(bmp, 0), queue, weight));

    SOC_IF_ERROR_RETURN(DRV_DEV_PROP_GET
        (unit, DRV_DEV_PROP_COSQ_MAX_WEIGHT_VALUE, &max_weight));

    if ((weight > max_weight) || (weight < 1)) {
        return SOC_E_PARAM;
    }

    switch (queue) {
        case 0:
            reg_index = INDEX(FCON_Q0_TXDSC_CTRL_3r);
            fld_index = INDEX(Q0_QUOTA_SIZEf);
            break;
        case 1:
            reg_index = INDEX(FCON_Q1_TXDSC_CTRL_3r);
            fld_index = INDEX(Q1_QUOTA_SIZEf);
            break;
        case 2:
            reg_index = INDEX(FCON_Q2_TXDSC_CTRL_3r);
            fld_index = INDEX(Q2_QUOTA_SIZEf);
            break;
        case 3:
            reg_index = INDEX(FCON_Q3_TXDSC_CTRL_3r);
            fld_index = INDEX(Q3_QUOTA_SIZEf);
            break;
        default:
            return SOC_E_PARAM;
            break;
    }

    reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, reg_index, 0, 0);
    reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, reg_index);
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_read)
        (unit, reg_addr, &reg_value, reg_len));

    temp = weight;
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
        (unit, reg_index, &reg_value, fld_index, &temp));
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_write)
        (unit, reg_addr, &reg_value, reg_len));

    return SOC_E_NONE;
}

/*
 *  Function : drv_harrier_queue_WRR_weight_get
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
drv_harrier_queue_WRR_weight_get(int unit, uint32 port_type, 
    uint32 port, uint8 queue, uint32 *weight)
{
    uint32  reg_addr, reg_value, temp;
    uint32  reg_index, fld_index;
    int     reg_len;

    switch (queue) {
        case 0:
            reg_index = INDEX(FCON_Q0_TXDSC_CTRL_3r);
            fld_index = INDEX(Q0_QUOTA_SIZEf);
            break;
        case 1:
            reg_index = INDEX(FCON_Q1_TXDSC_CTRL_3r);
            fld_index = INDEX(Q1_QUOTA_SIZEf);
            break;
        case 2:
            reg_index = INDEX(FCON_Q2_TXDSC_CTRL_3r);
            fld_index = INDEX(Q2_QUOTA_SIZEf);
            break;
        case 3:
            reg_index = INDEX(FCON_Q3_TXDSC_CTRL_3r);
            fld_index = INDEX(Q3_QUOTA_SIZEf);
            break;
        default:
            return SOC_E_PARAM;
            break;
    }

    reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, reg_index, 0, 0);
    reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, reg_index);
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_read)
        (unit, reg_addr, &reg_value, reg_len));
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_get)
        (unit, reg_index, &reg_value, fld_index, &temp));
    *weight = temp;

    LOG_VERBOSE(BSL_LS_SOC_COMMON, \
                (BSL_META_U(unit, \
                            "drv_harrier_queue_WRR_weight_get: \
                            unit %d, port type = %d, port = %d, queue = %d, weight = %d\n"),
                 unit, port_type, port, queue, *weight));

    return SOC_E_NONE;
}

/* Config output queue mapping */
/*
 *  Function : drv_harrier_queue_prio_set
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
drv_harrier_queue_prio_set(int unit, uint32 port, uint8 prio, 
    uint8 queue_n)
{
    uint32  reg_value, temp;

    LOG_VERBOSE(BSL_LS_SOC_COMMON, \
                (BSL_META_U(unit, \
                            "drv_harrier_queue_prio_set: \
                            unit %d, port = %d, priority = %d, queue = %d\n"), 
                 unit, port, prio, queue_n));

    if (port != -1) {
        return SOC_E_UNAVAIL;
    }

    SOC_IF_ERROR_RETURN(REG_READ_QOS_TCI_THr
        (unit, &reg_value));

    temp = queue_n;
    switch (prio) {
        case 0:
            SOC_IF_ERROR_RETURN(soc_QOS_TCI_THr_field_set
                (unit, &reg_value, PRITAG_000f, &temp));
            break;
        case 1:
            SOC_IF_ERROR_RETURN(soc_QOS_TCI_THr_field_set
                (unit, &reg_value, PRITAG_001f, &temp));
            break;
        case 2:         
            SOC_IF_ERROR_RETURN(soc_QOS_TCI_THr_field_set
                (unit, &reg_value, PRITAG_010f, &temp));
            break;
        case 3:
            SOC_IF_ERROR_RETURN(soc_QOS_TCI_THr_field_set
                (unit, &reg_value, PRITAG_011f, &temp));
            break;
        case 4:
            SOC_IF_ERROR_RETURN(soc_QOS_TCI_THr_field_set
                (unit, &reg_value, PRITAG_100f, &temp));
            break;
        case 5:
            SOC_IF_ERROR_RETURN(soc_QOS_TCI_THr_field_set
                (unit, &reg_value, PRITAG_101f, &temp));
            break;
        case 6:         
            SOC_IF_ERROR_RETURN(soc_QOS_TCI_THr_field_set
                (unit, &reg_value, PRITAG_110f, &temp));
            break;
        case 7:
            SOC_IF_ERROR_RETURN(soc_QOS_TCI_THr_field_set
                (unit, &reg_value, PRITAG_111f, &temp));
            break;
        default:
            return SOC_E_PARAM;
    }

    SOC_IF_ERROR_RETURN(REG_WRITE_QOS_TCI_THr
        (unit, &reg_value));

    return SOC_E_NONE;
}

/*
 *  Function : drv_harrier_queue_prio_get
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
drv_harrier_queue_prio_get(int unit, uint32 port, uint8 prio, 
    uint8 *queue_n)
{
    uint32  reg_value;
    uint32  temp = 0;

    if (port != -1) {
        return SOC_E_UNAVAIL;
    }

    SOC_IF_ERROR_RETURN(REG_READ_QOS_TCI_THr
        (unit, &reg_value));

    switch (prio) {
        case 0:
            SOC_IF_ERROR_RETURN(soc_QOS_TCI_THr_field_get
                (unit, &reg_value, PRITAG_000f, &temp));
            break;
        case 1:
            SOC_IF_ERROR_RETURN(soc_QOS_TCI_THr_field_get
                (unit, &reg_value, PRITAG_001f, &temp));
            break;
        case 2:         
            SOC_IF_ERROR_RETURN(soc_QOS_TCI_THr_field_get
                (unit, &reg_value, PRITAG_010f, &temp));
            break;
        case 3:
            SOC_IF_ERROR_RETURN(soc_QOS_TCI_THr_field_get
                (unit, &reg_value, PRITAG_011f, &temp));
            break;
        case 4:
            SOC_IF_ERROR_RETURN(soc_QOS_TCI_THr_field_get
                (unit, &reg_value, PRITAG_100f, &temp));
            break;
        case 5:
            SOC_IF_ERROR_RETURN(soc_QOS_TCI_THr_field_get
                (unit, &reg_value, PRITAG_101f, &temp));
            break;
        case 6:         
            SOC_IF_ERROR_RETURN(soc_QOS_TCI_THr_field_get
                (unit, &reg_value, PRITAG_110f, &temp));
            break;
        case 7:
            SOC_IF_ERROR_RETURN(soc_QOS_TCI_THr_field_get
                (unit, &reg_value, PRITAG_111f, &temp));
            break;
        default:
            return SOC_E_PARAM;
    }

    *queue_n = temp;

    LOG_VERBOSE(BSL_LS_SOC_COMMON, \
                (BSL_META_U(unit, \
                            "drv_harrier_queue_prio_get: \
                            unit %d, port = %d, priority = %d, queue = %d\n"), 
                 unit, port, prio, *queue_n));

    return SOC_E_NONE;
}

/*
 *  Function : drv_harrier_queue_dfsv_set
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
drv_harrier_queue_dfsv_set(int unit, uint8 code_point, uint8 queue_n)
{
    uint32  temp, t_code_point;
    uint64  reg_value;

    LOG_VERBOSE(BSL_LS_SOC_COMMON, \
                (BSL_META_U(unit, \
                            "drv_harrier_queue_dfsv_set: \
                            unit %d, dscp value = %d, queue = %d\n"), unit, code_point, queue_n));

    if (code_point > 31) {
        SOC_IF_ERROR_RETURN(REG_READ_QOS_DIFF_DSCP2r
            (unit, (uint32 *)&reg_value));
        t_code_point = (code_point - 32);
    } else {
        SOC_IF_ERROR_RETURN(REG_READ_QOS_DIFF_DSCP1r
            (unit, (uint32 *)&reg_value));
        t_code_point = code_point;
    }

    temp = queue_n;
    switch (t_code_point) {        
        case 0:
            SOC_IF_ERROR_RETURN(soc_QOS_DIFF_DSCP1r_field_set
                (unit, (uint32 *)&reg_value, PRI_DSCP_000000f, &temp));
            break;
        case 1: 
            SOC_IF_ERROR_RETURN(soc_QOS_DIFF_DSCP1r_field_set
                (unit, (uint32 *)&reg_value, PRI_DSCP_000001f, &temp));
            break;
        case 2:         
            SOC_IF_ERROR_RETURN(soc_QOS_DIFF_DSCP1r_field_set
                (unit, (uint32 *)&reg_value, PRI_DSCP_000010f, &temp));
            break;
        case 3:
            SOC_IF_ERROR_RETURN(soc_QOS_DIFF_DSCP1r_field_set
                (unit, (uint32 *)&reg_value, PRI_DSCP_000011f, &temp));
            break;
        case 4:
            SOC_IF_ERROR_RETURN(soc_QOS_DIFF_DSCP1r_field_set
                (unit, (uint32 *)&reg_value, PRI_DSCP_000100f, &temp));
            break;
        case 5:
            SOC_IF_ERROR_RETURN(soc_QOS_DIFF_DSCP1r_field_set
                (unit, (uint32 *)&reg_value, PRI_DSCP_000101f, &temp));
            break;
        case 6:         
            SOC_IF_ERROR_RETURN(soc_QOS_DIFF_DSCP1r_field_set
                (unit, (uint32 *)&reg_value, PRI_DSCP_000110f, &temp));
            break;
        case 7:
            SOC_IF_ERROR_RETURN(soc_QOS_DIFF_DSCP1r_field_set
                (unit, (uint32 *)&reg_value, PRI_DSCP_000111f, &temp));
            break;
        case 8:
            SOC_IF_ERROR_RETURN(soc_QOS_DIFF_DSCP1r_field_set
                (unit, (uint32 *)&reg_value, PRI_DSCP_001000f, &temp));
            break;
        case 9:   
            SOC_IF_ERROR_RETURN(soc_QOS_DIFF_DSCP1r_field_set
                (unit, (uint32 *)&reg_value, PRI_DSCP_001001f, &temp));
            break;
        case 10:         
            SOC_IF_ERROR_RETURN(soc_QOS_DIFF_DSCP1r_field_set
                (unit, (uint32 *)&reg_value, PRI_DSCP_001010f, &temp));
            break;
        case 11:
            SOC_IF_ERROR_RETURN(soc_QOS_DIFF_DSCP1r_field_set
                (unit, (uint32 *)&reg_value, PRI_DSCP_001011f, &temp));
            break;
        case 12:
            SOC_IF_ERROR_RETURN(soc_QOS_DIFF_DSCP1r_field_set
                (unit, (uint32 *)&reg_value, PRI_DSCP_001100f, &temp));
            break;
        case 13:
            SOC_IF_ERROR_RETURN(soc_QOS_DIFF_DSCP1r_field_set
                (unit, (uint32 *)&reg_value, PRI_DSCP_001101f, &temp));
            break;
        case 14:         
            SOC_IF_ERROR_RETURN(soc_QOS_DIFF_DSCP1r_field_set
                (unit, (uint32 *)&reg_value, PRI_DSCP_001110f, &temp));
            break;
        case 15:
            SOC_IF_ERROR_RETURN(soc_QOS_DIFF_DSCP1r_field_set
                (unit, (uint32 *)&reg_value, PRI_DSCP_001111f, &temp));
            break;
        case 16:
            SOC_IF_ERROR_RETURN(soc_QOS_DIFF_DSCP1r_field_set
                (unit, (uint32 *)&reg_value, PRI_DSCP_010000f, &temp));
            break;
        case 17:         
            SOC_IF_ERROR_RETURN(soc_QOS_DIFF_DSCP1r_field_set
                (unit, (uint32 *)&reg_value, PRI_DSCP_010001f, &temp));
            break;
        case 18:         
            SOC_IF_ERROR_RETURN(soc_QOS_DIFF_DSCP1r_field_set
                (unit, (uint32 *)&reg_value, PRI_DSCP_010010f, &temp));
            break;
        case 19:
            SOC_IF_ERROR_RETURN(soc_QOS_DIFF_DSCP1r_field_set
                (unit, (uint32 *)&reg_value, PRI_DSCP_010011f, &temp));
            break;
        case 20:
            SOC_IF_ERROR_RETURN(soc_QOS_DIFF_DSCP1r_field_set
                (unit, (uint32 *)&reg_value, PRI_DSCP_010100f, &temp));
            break;
        case 21:
            SOC_IF_ERROR_RETURN(soc_QOS_DIFF_DSCP1r_field_set
                (unit, (uint32 *)&reg_value, PRI_DSCP_010101f, &temp));
            break;
        case 22:        
            SOC_IF_ERROR_RETURN(soc_QOS_DIFF_DSCP1r_field_set
                (unit, (uint32 *)&reg_value, PRI_DSCP_010110f, &temp));
            break;
        case 23:
            SOC_IF_ERROR_RETURN(soc_QOS_DIFF_DSCP1r_field_set
                (unit, (uint32 *)&reg_value, PRI_DSCP_010111f, &temp));
            break;
        case 24:
            SOC_IF_ERROR_RETURN(soc_QOS_DIFF_DSCP1r_field_set
                (unit, (uint32 *)&reg_value, PRI_DSCP_011000f, &temp));
            break;
        case 25:         
            SOC_IF_ERROR_RETURN(soc_QOS_DIFF_DSCP1r_field_set
                (unit, (uint32 *)&reg_value, PRI_DSCP_011001f, &temp));
            break;
        case 26:         
            SOC_IF_ERROR_RETURN(soc_QOS_DIFF_DSCP1r_field_set
                (unit, (uint32 *)&reg_value, PRI_DSCP_011010f, &temp));
            break;
        case 27:
            SOC_IF_ERROR_RETURN(soc_QOS_DIFF_DSCP1r_field_set
                (unit, (uint32 *)&reg_value, PRI_DSCP_011011f, &temp));
            break;
        case 28:
            SOC_IF_ERROR_RETURN(soc_QOS_DIFF_DSCP1r_field_set
                (unit, (uint32 *)&reg_value, PRI_DSCP_011100f, &temp));
            break;
        case 29:
            SOC_IF_ERROR_RETURN(soc_QOS_DIFF_DSCP1r_field_set
                (unit, (uint32 *)&reg_value, PRI_DSCP_011101f, &temp));
            break;
        case 30:         
            SOC_IF_ERROR_RETURN(soc_QOS_DIFF_DSCP1r_field_set
                (unit, (uint32 *)&reg_value, PRI_DSCP_011110f, &temp));
            break;
        case 31:
            SOC_IF_ERROR_RETURN(soc_QOS_DIFF_DSCP1r_field_set
                (unit, (uint32 *)&reg_value, PRI_DSCP_011111f, &temp));
            break;
        default:
            return SOC_E_PARAM;
    }

    if (code_point > 31) {
        SOC_IF_ERROR_RETURN(REG_WRITE_QOS_DIFF_DSCP2r
            (unit, (uint32 *)&reg_value));
    } else {
        SOC_IF_ERROR_RETURN(REG_WRITE_QOS_DIFF_DSCP1r
            (unit, (uint32 *)&reg_value));
    }

    return SOC_E_NONE;
}

/*
 *  Function : drv_harrier_queue_dfsv_get
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
drv_harrier_queue_dfsv_get(int unit, uint8 code_point, uint8 *queue_n)
{
    uint32  temp = 0, t_code_point;
    uint64  reg_value;

    sal_memset(&reg_value, 0, 8);

    if (code_point > 31) {
        SOC_IF_ERROR_RETURN(REG_READ_QOS_DIFF_DSCP2r
            (unit, (uint32 *)&reg_value));
        t_code_point = (code_point - 32);
    } else {
        SOC_IF_ERROR_RETURN(REG_READ_QOS_DIFF_DSCP1r
            (unit, (uint32 *)&reg_value));
        t_code_point = code_point;
    }

    switch (t_code_point) {        
        case 0:
            SOC_IF_ERROR_RETURN(soc_QOS_DIFF_DSCP1r_field_get
                (unit, (uint32 *)&reg_value, PRI_DSCP_000000f, &temp));
            break;
        case 1:       
            SOC_IF_ERROR_RETURN(soc_QOS_DIFF_DSCP1r_field_get
                (unit, (uint32 *)&reg_value, PRI_DSCP_000000f, &temp));
            break;
        case 2:         
            SOC_IF_ERROR_RETURN(soc_QOS_DIFF_DSCP1r_field_get
                (unit, (uint32 *)&reg_value, PRI_DSCP_000010f, &temp));
            break;
        case 3:
            SOC_IF_ERROR_RETURN(soc_QOS_DIFF_DSCP1r_field_get
                (unit, (uint32 *)&reg_value, PRI_DSCP_000011f, &temp));
            break;
        case 4:
            SOC_IF_ERROR_RETURN(soc_QOS_DIFF_DSCP1r_field_get
                (unit, (uint32 *)&reg_value, PRI_DSCP_000100f, &temp));
            break;
        case 5:
            SOC_IF_ERROR_RETURN(soc_QOS_DIFF_DSCP1r_field_get
                (unit, (uint32 *)&reg_value, PRI_DSCP_000101f, &temp));
            break;
        case 6:         
            SOC_IF_ERROR_RETURN(soc_QOS_DIFF_DSCP1r_field_get
                (unit, (uint32 *)&reg_value, PRI_DSCP_000110f, &temp));
            break;
        case 7:
            SOC_IF_ERROR_RETURN(soc_QOS_DIFF_DSCP1r_field_get
                (unit, (uint32 *)&reg_value, PRI_DSCP_000111f, &temp));
            break;
        case 8:
            SOC_IF_ERROR_RETURN(soc_QOS_DIFF_DSCP1r_field_get
                (unit, (uint32 *)&reg_value, PRI_DSCP_001000f, &temp));
            break;
        case 9:         
            SOC_IF_ERROR_RETURN(soc_QOS_DIFF_DSCP1r_field_get
                (unit, (uint32 *)&reg_value, PRI_DSCP_001001f, &temp));
            break;
        case 10:         
            SOC_IF_ERROR_RETURN(soc_QOS_DIFF_DSCP1r_field_get
                (unit, (uint32 *)&reg_value, PRI_DSCP_001010f, &temp));
            break;
        case 11:
            SOC_IF_ERROR_RETURN(soc_QOS_DIFF_DSCP1r_field_get
                (unit, (uint32 *)&reg_value, PRI_DSCP_001011f, &temp));
            break;
        case 12:
            SOC_IF_ERROR_RETURN(soc_QOS_DIFF_DSCP1r_field_get
                (unit, (uint32 *)&reg_value, PRI_DSCP_001100f, &temp));
            break;
        case 13:
            SOC_IF_ERROR_RETURN(soc_QOS_DIFF_DSCP1r_field_get
                (unit, (uint32 *)&reg_value, PRI_DSCP_001101f, &temp));
            break;
        case 14:         
            SOC_IF_ERROR_RETURN(soc_QOS_DIFF_DSCP1r_field_get
                (unit, (uint32 *)&reg_value, PRI_DSCP_001110f, &temp));
            break;
        case 15:
            SOC_IF_ERROR_RETURN(soc_QOS_DIFF_DSCP1r_field_get
                (unit, (uint32 *)&reg_value, PRI_DSCP_001111f, &temp));
            break;
        case 16:
            SOC_IF_ERROR_RETURN(soc_QOS_DIFF_DSCP1r_field_get
                (unit, (uint32 *)&reg_value, PRI_DSCP_010000f, &temp));
            break;
        case 17:         
            SOC_IF_ERROR_RETURN(soc_QOS_DIFF_DSCP1r_field_get
                (unit, (uint32 *)&reg_value, PRI_DSCP_010001f, &temp));
            break;
        case 18:         
            SOC_IF_ERROR_RETURN(soc_QOS_DIFF_DSCP1r_field_get
                (unit, (uint32 *)&reg_value, PRI_DSCP_010010f, &temp));
            break;
        case 19:
            SOC_IF_ERROR_RETURN(soc_QOS_DIFF_DSCP1r_field_get
                (unit, (uint32 *)&reg_value, PRI_DSCP_010011f, &temp));
            break;
        case 20:
            SOC_IF_ERROR_RETURN(soc_QOS_DIFF_DSCP1r_field_get
                (unit, (uint32 *)&reg_value, PRI_DSCP_010100f, &temp));
            break;
        case 21:
            SOC_IF_ERROR_RETURN(soc_QOS_DIFF_DSCP1r_field_get
                (unit, (uint32 *)&reg_value, PRI_DSCP_010101f, &temp));
            break;
        case 22:         
            SOC_IF_ERROR_RETURN(soc_QOS_DIFF_DSCP1r_field_get
                (unit, (uint32 *)&reg_value, PRI_DSCP_010110f, &temp));
            break;
        case 23:
            SOC_IF_ERROR_RETURN(soc_QOS_DIFF_DSCP1r_field_get
                (unit, (uint32 *)&reg_value, PRI_DSCP_010111f, &temp));
            break;
        case 24:
            SOC_IF_ERROR_RETURN(soc_QOS_DIFF_DSCP1r_field_get
                (unit, (uint32 *)&reg_value, PRI_DSCP_011000f, &temp));
            break;
        case 25:         
            SOC_IF_ERROR_RETURN(soc_QOS_DIFF_DSCP1r_field_get
                (unit, (uint32 *)&reg_value, PRI_DSCP_011001f, &temp));
            break;
        case 26:         
            SOC_IF_ERROR_RETURN(soc_QOS_DIFF_DSCP1r_field_get
                (unit, (uint32 *)&reg_value, PRI_DSCP_011010f, &temp));
            break;
        case 27:
            SOC_IF_ERROR_RETURN(soc_QOS_DIFF_DSCP1r_field_get
                (unit, (uint32 *)&reg_value, PRI_DSCP_011011f, &temp));
            break;
        case 28:
            SOC_IF_ERROR_RETURN(soc_QOS_DIFF_DSCP1r_field_get
                (unit, (uint32 *)&reg_value, PRI_DSCP_011100f, &temp));
            break;
        case 29:
            SOC_IF_ERROR_RETURN(soc_QOS_DIFF_DSCP1r_field_get
                (unit, (uint32 *)&reg_value, PRI_DSCP_011101f, &temp));
            break;
        case 30:         
            SOC_IF_ERROR_RETURN(soc_QOS_DIFF_DSCP1r_field_get
                (unit, (uint32 *)&reg_value, PRI_DSCP_011110f, &temp));
            break;
        case 31:
            SOC_IF_ERROR_RETURN(soc_QOS_DIFF_DSCP1r_field_get
                (unit, (uint32 *)&reg_value, PRI_DSCP_011111f, &temp));
            break;
        default:
            return SOC_E_PARAM;
    }

    *queue_n = temp;

    LOG_VERBOSE(BSL_LS_SOC_COMMON, \
                (BSL_META_U(unit, \
                            "drv_harrier_queue_dfsv_get: \
                            unit %d, dscp value = %d, queue = %d\n"), unit, code_point, *queue_n));

    return SOC_E_NONE;
}

/* Enable/disable selected mapping - prio/diffserv/tos */
/*
 *  Function : drv_harrier_queue_mapping_type_set
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
drv_harrier_queue_mapping_type_set(int unit, soc_pbmp_t bmp, 
    uint32 mapping_type, uint8 state)
{
    uint32  temp = 0, val_32;
    uint64  reg_value64, pbmp_value, temp64;

    LOG_VERBOSE(BSL_LS_SOC_COMMON, \
                (BSL_META_U(unit, \
                            "drv_harrier_queue_mapping_type_set: \
                            unit %d, bmp = 0x%x 0x%x, type = %d, %sable\n"),
                 unit, SOC_PBMP_WORD_GET(bmp, 0), SOC_PBMP_WORD_GET(bmp, 1),
                 mapping_type, (state) ? "en" : "dis"));

    COMPILER_64_ZERO(temp64);
    switch (mapping_type) {
        case DRV_QUEUE_MAP_NONE:
            /* Disable QOS_IP_EN register */
            SOC_IF_ERROR_RETURN(REG_READ_QOS_1P_ENr
                (unit, (uint32 *)&reg_value64));

            if (SOC_INFO(unit).port_num > 32) {
                SOC_IF_ERROR_RETURN(soc_QOS_1P_ENr_field_get
                    (unit, (uint32 *)&reg_value64, 
                    QOS_1P_ENf, (uint32 *)&temp64));

                SOC_IF_ERROR_RETURN(soc_robo_64_pbmp_to_val
                    (unit, &bmp, &pbmp_value));
                COMPILER_64_NOT(pbmp_value);
                COMPILER_64_AND(temp64, pbmp_value);

                SOC_IF_ERROR_RETURN(soc_QOS_1P_ENr_field_set
                    (unit, (uint32 *)&reg_value64, 
                    QOS_1P_ENf, (uint32 *)&temp64));
            } else {
                SOC_IF_ERROR_RETURN(soc_QOS_1P_ENr_field_get
                    (unit, (uint32 *)&reg_value64, 
                    QOS_1P_ENf, (uint32 *)&temp));

                val_32 = SOC_PBMP_WORD_GET(bmp, 0);
                temp &= ~val_32;

                SOC_IF_ERROR_RETURN(soc_QOS_1P_ENr_field_set
                    (unit, (uint32 *)&reg_value64, 
                    QOS_1P_ENf, (uint32 *)&temp));
            }
            SOC_IF_ERROR_RETURN(REG_WRITE_QOS_1P_ENr
                (unit, (uint32 *)&reg_value64));

            /* Disable QOS_TOS_DIF_EN register */
            SOC_IF_ERROR_RETURN(REG_READ_QOS_TOS_DIF_ENr
                (unit, (uint32 *)&reg_value64));

            if (SOC_INFO(unit).port_num > 32) {
                SOC_IF_ERROR_RETURN(soc_QOS_TOS_DIF_ENr_field_get
                    (unit, (uint32 *)&reg_value64, 
                    QOS_TOS_DIFF_ENf, (uint32 *)&temp64));

                SOC_IF_ERROR_RETURN(soc_robo_64_pbmp_to_val
                    (unit, &bmp, &pbmp_value));
                COMPILER_64_NOT(pbmp_value);
                COMPILER_64_AND(temp64, pbmp_value);

                SOC_IF_ERROR_RETURN(soc_QOS_TOS_DIF_ENr_field_set
                    (unit, (uint32 *)&reg_value64, 
                    QOS_TOS_DIFF_ENf, (uint32 *)&temp64));
            } else {
                SOC_IF_ERROR_RETURN(soc_QOS_TOS_DIF_ENr_field_get
                    (unit, (uint32 *)&reg_value64, 
                    QOS_TOS_DIFF_ENf, (uint32 *)&temp));

                val_32 = SOC_PBMP_WORD_GET(bmp, 0);
                temp &= ~val_32;

                SOC_IF_ERROR_RETURN(soc_QOS_TOS_DIF_ENr_field_set
                    (unit, (uint32 *)&reg_value64, 
                    QOS_TOS_DIFF_ENf, (uint32 *)&temp));
            }
            SOC_IF_ERROR_RETURN(REG_WRITE_QOS_TOS_DIF_ENr
                (unit, (uint32 *)&reg_value64));
            break;
        case DRV_QUEUE_MAP_PRIO:
            /* Enable/disable QOS_1P_ENr */
            SOC_IF_ERROR_RETURN(REG_READ_QOS_1P_ENr
                (unit, (uint32 *)&reg_value64));

            if (SOC_INFO(unit).port_num > 32) {
                SOC_IF_ERROR_RETURN(soc_QOS_1P_ENr_field_get
                    (unit, (uint32 *)&reg_value64, 
                    QOS_1P_ENf, (uint32 *)&temp64));

                SOC_IF_ERROR_RETURN(soc_robo_64_pbmp_to_val
                    (unit, &bmp, &pbmp_value));
                if (state) {
                    COMPILER_64_OR(temp64, pbmp_value);
                } else {
                    COMPILER_64_NOT(pbmp_value);
                    COMPILER_64_AND(temp64, pbmp_value);
                }

                SOC_IF_ERROR_RETURN(soc_QOS_1P_ENr_field_set
                    (unit, (uint32 *)&reg_value64, 
                    QOS_1P_ENf, (uint32 *)&temp64));
            } else {
                SOC_IF_ERROR_RETURN(soc_QOS_1P_ENr_field_get
                    (unit, (uint32 *)&reg_value64, 
                    QOS_1P_ENf, (uint32 *)&temp));

                val_32 = SOC_PBMP_WORD_GET(bmp, 0);
                if (state) {
                    temp |= val_32;
                } else {
                    temp &= ~val_32;
                }

                SOC_IF_ERROR_RETURN(soc_QOS_1P_ENr_field_set
                    (unit, (uint32 *)&reg_value64, 
                    QOS_1P_ENf, (uint32 *)&temp));
            }
            SOC_IF_ERROR_RETURN(REG_WRITE_QOS_1P_ENr
                (unit, (uint32 *)&reg_value64));
    
            /* Don't need to disable QOS_TOS_DIF_ENr : 
              * the setting is separated and chosen by QoS priority decision flowchart
              */
            break;
        case DRV_QUEUE_MAP_DFSV:
            /* No need to switch TOS/DFSV, Harrier removes TOS function */
    
            /* Enable / diaable QOS_TOS_DIF_ENr */
            SOC_IF_ERROR_RETURN(REG_READ_QOS_TOS_DIF_ENr
                (unit, (uint32 *)&reg_value64));

            if (SOC_INFO(unit).port_num > 32) {
                SOC_IF_ERROR_RETURN(soc_QOS_TOS_DIF_ENr_field_get
                    (unit, (uint32 *)&reg_value64, 
                    QOS_TOS_DIFF_ENf, (uint32 *)&temp64));

                SOC_IF_ERROR_RETURN(soc_robo_64_pbmp_to_val
                    (unit, &bmp, &pbmp_value));
                if (state) {
                    COMPILER_64_OR(temp64, pbmp_value);
                } else {
                    COMPILER_64_NOT(pbmp_value);
                    COMPILER_64_AND(temp64, pbmp_value);
                }

                SOC_IF_ERROR_RETURN(soc_QOS_TOS_DIF_ENr_field_set
                    (unit, (uint32 *)&reg_value64, 
                    QOS_TOS_DIFF_ENf, (uint32 *)&temp64));
            } else {
                SOC_IF_ERROR_RETURN(soc_QOS_TOS_DIF_ENr_field_get
                    (unit, (uint32 *)&reg_value64, 
                    QOS_TOS_DIFF_ENf, (uint32 *)&temp));

                val_32 = SOC_PBMP_WORD_GET(bmp, 0);
                if (state) {
                    temp |= val_32;
                } else {
                    temp &= ~val_32;
                }

                SOC_IF_ERROR_RETURN(soc_QOS_TOS_DIF_ENr_field_set
                    (unit, (uint32 *)&reg_value64, 
                    QOS_TOS_DIFF_ENf, (uint32 *)&temp));
            }
            SOC_IF_ERROR_RETURN(REG_WRITE_QOS_TOS_DIF_ENr
                (unit, (uint32 *)&reg_value64));

            /* Don't need to disable QOS_1P_ENr : 
              * the setting is separated and chosen by QoS priority decision flowchart
              */
            break;
        case DRV_QUEUE_MAP_PORT:
            /* Harrier have no force port based priority solution :
             *  - the default port priority can be retrieved only if all other
             *      TC decision process been disabled. (like protocol based, mac 
             *      based, vlan based,.. etc.)
             */
        case DRV_QUEUE_MAP_TOS:
        case DRV_QUEUE_MAP_MAC:
        case DRV_QUEUE_MAP_HYBRID:
            return SOC_E_UNAVAIL;
        default :
            return SOC_E_PARAM;
    }

    return SOC_E_NONE;
}

/*
 *  Function : drv_harrier_queue_mapping_type_get
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
drv_harrier_queue_mapping_type_get(int unit, uint32 port, 
    uint32 mapping_type, uint8 *state)
{
    uint32  temp = 0;
    uint64  reg_value64, temp64;
    soc_pbmp_t  pbmp;

    COMPILER_64_ZERO(temp64);
    switch (mapping_type) {
        case DRV_QUEUE_MAP_NONE:
            return SOC_E_PARAM;
        case DRV_QUEUE_MAP_PRIO:
            SOC_IF_ERROR_RETURN(REG_READ_QOS_1P_ENr
                (unit, (uint32 *)&reg_value64));

            if (SOC_INFO(unit).port_num > 32) {
                SOC_IF_ERROR_RETURN(soc_QOS_1P_ENr_field_get
                    (unit, (uint32 *)&reg_value64, 
                    QOS_1P_ENf, (uint32 *)&temp64));

                SOC_PBMP_CLEAR(pbmp);
                SOC_IF_ERROR_RETURN(soc_robo_64_val_to_pbmp
                    (unit, &pbmp, temp64));
            } else {
                SOC_IF_ERROR_RETURN(soc_QOS_1P_ENr_field_get
                    (unit, (uint32 *)&reg_value64, 
                    QOS_1P_ENf, (uint32 *)&temp));

                SOC_PBMP_CLEAR(pbmp);
                SOC_PBMP_WORD_SET(pbmp, 0, temp);
            }

            if (SOC_PBMP_MEMBER(pbmp, port)) {
                *state = TRUE;
            } else {
                *state = FALSE;
            }
            break;
        case DRV_QUEUE_MAP_DFSV:
            SOC_IF_ERROR_RETURN(REG_READ_QOS_TOS_DIF_ENr
                (unit, (uint32 *)&reg_value64));

            if (SOC_INFO(unit).port_num > 32) {
                SOC_IF_ERROR_RETURN(soc_QOS_TOS_DIF_ENr_field_get
                    (unit, (uint32 *)&reg_value64, 
                    QOS_TOS_DIFF_ENf, (uint32 *)&temp64));

                SOC_PBMP_CLEAR(pbmp);
                SOC_IF_ERROR_RETURN(soc_robo_64_val_to_pbmp
                    (unit, &pbmp, temp64));
            } else {
                SOC_IF_ERROR_RETURN(soc_QOS_TOS_DIF_ENr_field_get
                    (unit, (uint32 *)&reg_value64, 
                    QOS_TOS_DIFF_ENf, (uint32 *)&temp));

                SOC_PBMP_CLEAR(pbmp);
                SOC_PBMP_WORD_SET(pbmp, 0, temp);
            }

            if (SOC_PBMP_MEMBER(pbmp, port)) {
                *state = TRUE;
            } else {
                *state = FALSE;
            }
            break;
        case DRV_QUEUE_MAP_TOS:
        case DRV_QUEUE_MAP_PORT:
            /* Harrier have no force port based priority solution :
             *  - the default port priority can be retrieved only if all other
             *      TC decision process been disabled. (like protocol based, mac 
             *      based, vlan based,.. etc.)
             */
        case DRV_QUEUE_MAP_MAC:
        case DRV_QUEUE_MAP_HYBRID:
            return SOC_E_UNAVAIL;
        default:
            return SOC_E_PARAM;
    }

    LOG_VERBOSE(BSL_LS_SOC_COMMON, \
                (BSL_META_U(unit, \
                            "drv_harrier_queue_mapping_type_get: \
                            unit %d, port = %d, type = %d, %sable\n"),
                 unit, port, mapping_type, (*state) ? "en" : "dis"));

    return SOC_E_NONE;
}

/*
 *  Function : drv_harrier_queue_rx_reason_set
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
drv_harrier_queue_rx_reason_set(int unit, uint8 reason, uint32 queue)
{
    uint32  reg_value, temp;

    LOG_VERBOSE(BSL_LS_SOC_COMMON, \
                (BSL_META_U(unit, \
                            "drv_harrier_queue_rx_reason_set: \
                            unit %d, reason = %d, queue = %d\n"), unit, reason, queue));

    SOC_IF_ERROR_RETURN(REG_READ_QOS_REASON_CODEr
        (unit, &reg_value));

    temp = queue;
    switch (reason) {
        case DRV_RX_REASON_MIRRORING:
            SOC_IF_ERROR_RETURN(soc_QOS_REASON_CODEr_field_set
                (unit, &reg_value, PRI_MIRRORf, &temp));
            break;
        case DRV_RX_REASON_SA_LEARNING:
            SOC_IF_ERROR_RETURN(soc_QOS_REASON_CODEr_field_set
                (unit, &reg_value, PRI_SA_LEARNf, &temp));
            break;
        case DRV_RX_REASON_SWITCHING:
            SOC_IF_ERROR_RETURN(soc_QOS_REASON_CODEr_field_set
                (unit, &reg_value, PRI_SWITCHf, &temp));
            break;
        case DRV_RX_REASON_PROTO_TERM:
            SOC_IF_ERROR_RETURN(soc_QOS_REASON_CODEr_field_set
                (unit, &reg_value, PRI_PROTOCOL_TERMf, &temp));
            break;
        case DRV_RX_REASON_PROTO_SNOOP:
            SOC_IF_ERROR_RETURN(soc_QOS_REASON_CODEr_field_set
                (unit, &reg_value, PRI_PROTOCOL_SNOOPf, &temp));
            break;
        case DRV_RX_REASON_EXCEPTION:
            SOC_IF_ERROR_RETURN(soc_QOS_REASON_CODEr_field_set
                (unit, &reg_value, PRI_EXCEPTf, &temp));
            break;
        default:
            return SOC_E_UNAVAIL;
    }
    SOC_IF_ERROR_RETURN(REG_WRITE_QOS_REASON_CODEr
        (unit, &reg_value));

    return SOC_E_NONE;
}

/*
 *  Function : drv_harrier_queue_rx_reason_get
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
drv_harrier_queue_rx_reason_get(int unit, uint8 reason, uint32 *queue)
{
    uint32  reg_value, temp = 0;

    SOC_IF_ERROR_RETURN(REG_READ_QOS_REASON_CODEr
        (unit, &reg_value));

    switch (reason) {
        case DRV_RX_REASON_MIRRORING:
            SOC_IF_ERROR_RETURN(soc_QOS_REASON_CODEr_field_get
                (unit, &reg_value, PRI_MIRRORf, &temp));
            break;
        case DRV_RX_REASON_SA_LEARNING:
            SOC_IF_ERROR_RETURN(soc_QOS_REASON_CODEr_field_get
                (unit, &reg_value, PRI_SA_LEARNf, &temp));
            break;
        case DRV_RX_REASON_SWITCHING:
            SOC_IF_ERROR_RETURN(soc_QOS_REASON_CODEr_field_get
                (unit, &reg_value, PRI_SWITCHf, &temp));
            break;
        case DRV_RX_REASON_PROTO_TERM:
            SOC_IF_ERROR_RETURN(soc_QOS_REASON_CODEr_field_get
                (unit, &reg_value, PRI_PROTOCOL_TERMf, &temp));
            break;
        case DRV_RX_REASON_PROTO_SNOOP:
            SOC_IF_ERROR_RETURN(soc_QOS_REASON_CODEr_field_get
                (unit, &reg_value, PRI_PROTOCOL_SNOOPf, &temp));
            break;
        case DRV_RX_REASON_EXCEPTION:
            SOC_IF_ERROR_RETURN(soc_QOS_REASON_CODEr_field_get
                (unit, &reg_value, PRI_EXCEPTf, &temp));
            break;
        default:
            return SOC_E_UNAVAIL;
    }
    *queue = temp;

    LOG_VERBOSE(BSL_LS_SOC_COMMON, \
                (BSL_META_U(unit, \
                            "drv_harrier_queue_rx_reason_get: \
                            unit %d, reason = %d, *queue = %d\n"), unit, reason, *queue));

    return SOC_E_NONE;
}
