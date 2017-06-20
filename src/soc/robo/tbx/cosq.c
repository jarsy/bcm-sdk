/*
 * $Id: cosq.c,v 1.6 Broadcom SDK $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include <shared/bsl.h>

#include <soc/robo.h>
#include <soc/drv.h>
#include <soc/debug.h>

#define TBX_DEFAULT_DP_VALID(val)  ((val) >= 0 && (val <= 3))

/* Number of TC */
#define _TBX_TC_COUNT  16
#define _TBX_RX_REASON_INVALID  0xffffffff
#define _TBX_TC_INVALID  0xffffffff

typedef struct _tbx_cosq_rx_reason_info_s 
{
    int  unit;
    uint32  valid;
    uint32  reason_cosq[DRV_RX_REASON_COUNT];
    uint32  tc2cosq_en;
    uint32  tc2cosq_prio[_TBX_TC_COUNT];

    struct _tbx_cosq_rx_reason_info_s *next;
    
} _tbx_cosq_rx_reason_info_t;

static _tbx_cosq_rx_reason_info_t *_tbx_rx_reason_info = NULL;

static int
_drv_tbx_rx_reason_info_update
    (int unit, _tbx_cosq_rx_reason_info_t *reason_info) 
{
    uint32  temp;
    uint32  reg_value;
    uint64  reg_value64;
    int  i;

    /* Update the information from register */

    /* Use TC2Cosq enabled */
    SOC_IF_ERROR_RETURN(REG_READ_QOS_CTLr
        (unit, &reg_value));
    /* This type value definition is :
     *  1: Use generic TC based COS mapping for copying packets to CPU.
     *  0: Use Reason basd COS mapping  for copying packets to CPU.
     */
    SOC_IF_ERROR_RETURN(soc_QOS_CTLr_field_get
        (unit, &reg_value, USE_TCf, &temp));
    reason_info->tc2cosq_en = ((temp) ? TRUE : FALSE);

    /* Rx reason code and cosq mapping */
    SOC_IF_ERROR_RETURN(REG_READ_QOS_REASON_CODEr
        (unit, (void *)&reg_value64));

    for (i = 0; i < DRV_RX_REASON_COUNT; i++) { 
        switch (i) {
            case DRV_RX_REASON_SWITCHING:
                SOC_IF_ERROR_RETURN(soc_QOS_REASON_CODEr_field_get(unit, 
                    (void *)&reg_value64, ARL_KNOWN_DA_TERMINf, &temp));
                break;
            case DRV_RX_REASON_EXCEPTION:
                SOC_IF_ERROR_RETURN(soc_QOS_REASON_CODEr_field_get(unit, 
                    (void *)&reg_value64, ARL_UNKNOWN_DA_FLOODf, &temp));
                break;
            case DRV_RX_REASON_PROTO_TERM:
                SOC_IF_ERROR_RETURN(soc_QOS_REASON_CODEr_field_get
                    (unit, (void *)&reg_value64, ARL_8021_PROT_TRAPf, &temp));
                break;
            case DRV_RX_REASON_PROTO_SNOOP:
                SOC_IF_ERROR_RETURN(soc_QOS_REASON_CODEr_field_get(unit, 
                    (void *)&reg_value64, ARL_APPL_PROT_SNOOPf, &temp));
                break;
            case DRV_RX_REASON_ARL_VALN_DIR_FWD:
                SOC_IF_ERROR_RETURN(soc_QOS_REASON_CODEr_field_get
                    (unit, (void *)&reg_value64, ARL_VLAN_DIR_FWDf, &temp));
                break;
            case DRV_RX_REASON_ARL_CFP_FWD:
                SOC_IF_ERROR_RETURN(soc_QOS_REASON_CODEr_field_get
                    (unit, (void *)&reg_value64, ARL_CFP_FWDf, &temp));
                break;
            case DRV_RX_REASON_ARL_LOOPBACK:
                SOC_IF_ERROR_RETURN(soc_QOS_REASON_CODEr_field_get
                    (unit, (void *)&reg_value64, ARL_LOOPBACKf, &temp));
                break;
            case DRV_RX_REASON_MIRRORING:
                SOC_IF_ERROR_RETURN(soc_QOS_REASON_CODEr_field_get
                    (unit, (void *)&reg_value64, MIRROR_COPYf, &temp));
                break;
            case DRV_RX_REASON_INGRESS_SFLOW:
                SOC_IF_ERROR_RETURN(soc_QOS_REASON_CODEr_field_get
                    (unit, (void *)&reg_value64, INGRESS_SFLOWf, &temp));
                break;
            case DRV_RX_REASON_EGRESS_SFLOW:
                SOC_IF_ERROR_RETURN(soc_QOS_REASON_CODEr_field_get
                    (unit, (void *)&reg_value64, EGRESS_SFLOWf, &temp));
                break;
            case DRV_RX_REASON_SA_MOVEMENT_EVENT:
                SOC_IF_ERROR_RETURN(soc_QOS_REASON_CODEr_field_get
                    (unit, (void *)&reg_value64, SA_MOVEMENT_EVENTf, &temp));
                break;
            case DRV_RX_REASON_SA_UNKNOWN_EVENT:
                SOC_IF_ERROR_RETURN(soc_QOS_REASON_CODEr_field_get
                    (unit, (void *)&reg_value64, SA_UNKNOWN_EVENTf, &temp));
                break;
            case DRV_RX_REASON_SA_OVER_LIMIT_EVENT:
                SOC_IF_ERROR_RETURN(soc_QOS_REASON_CODEr_field_get(unit, 
                    (void *)&reg_value64, SA_OVER_LIMIT_EVENTf, &temp));
                break;
            case DRV_RX_REASON_INP_NON_MEMBER:
                SOC_IF_ERROR_RETURN(soc_QOS_REASON_CODEr_field_get
                    (unit, (void *)&reg_value64, INP_NON_MEMBERf, &temp));
                break;
            case DRV_RX_REASON_VLAN_UNKNOWN:
                SOC_IF_ERROR_RETURN(soc_QOS_REASON_CODEr_field_get
                    (unit, (void *)&reg_value64, VLAN_UNKNOWNf, &temp));
                break;
            default:
                temp = _TBX_RX_REASON_INVALID;
        }

        reason_info->reason_cosq[i] = temp;
    }

    /* TC2COS mapping */
    SOC_IF_ERROR_RETURN(REG_READ_TC2COS_MAPr
        (unit, (void *)&reg_value64));

    temp = _TBX_TC_INVALID;
    for (i = 0; i < _TBX_TC_COUNT; i++) { 
        switch (i) {
            case 0:
                SOC_IF_ERROR_RETURN(soc_TC2COS_MAPr_field_get
                    (unit, (void *)&reg_value64, TC_0000_COSf, &temp));
                break;
            case 1:         
                SOC_IF_ERROR_RETURN(soc_TC2COS_MAPr_field_get
                    (unit, (void *)&reg_value64, TC_0001_COSf, &temp));
                break;
            case 2:         
                SOC_IF_ERROR_RETURN(soc_TC2COS_MAPr_field_get
                    (unit, (void *)&reg_value64, TC_0010_COSf, &temp));
                break;
            case 3:
                SOC_IF_ERROR_RETURN(soc_TC2COS_MAPr_field_get
                    (unit, (void *)&reg_value64, TC_0011_COSf, &temp));
                break;
            case 4:
                SOC_IF_ERROR_RETURN(soc_TC2COS_MAPr_field_get
                    (unit, (void *)&reg_value64, TC_0100_COSf, &temp));
                break;
            case 5:
                SOC_IF_ERROR_RETURN(soc_TC2COS_MAPr_field_get
                    (unit, (void *)&reg_value64, TC_0101_COSf, &temp));
                break;
            case 6:         
                SOC_IF_ERROR_RETURN(soc_TC2COS_MAPr_field_get
                    (unit, (void *)&reg_value64, TC_0110_COSf, &temp));
                break;
            case 7:
                SOC_IF_ERROR_RETURN(soc_TC2COS_MAPr_field_get
                    (unit, (void *)&reg_value64, TC_0111_COSf, &temp));
                break;
            case 8:
                SOC_IF_ERROR_RETURN(soc_TC2COS_MAPr_field_get
                    (unit, (void *)&reg_value64, TC_1000_COSf, &temp));
                break;
            case 9:         
                SOC_IF_ERROR_RETURN(soc_TC2COS_MAPr_field_get
                    (unit, (void *)&reg_value64, TC_1001_COSf, &temp));
                break;
            case 10:         
                SOC_IF_ERROR_RETURN(soc_TC2COS_MAPr_field_get
                    (unit, (void *)&reg_value64, TC_1010_COSf, &temp));
                break;
            case 11:
                SOC_IF_ERROR_RETURN(soc_TC2COS_MAPr_field_get
                    (unit, (void *)&reg_value64, TC_1011_COSf, &temp));
                break;
            case 12:
                SOC_IF_ERROR_RETURN(soc_TC2COS_MAPr_field_get
                    (unit, (void *)&reg_value64, TC_1100_COSf, &temp));
                break;
            case 13:
                SOC_IF_ERROR_RETURN(soc_TC2COS_MAPr_field_get
                    (unit, (void *)&reg_value64, TC_1101_COSf, &temp));
                break;
            case 14:         
                SOC_IF_ERROR_RETURN(soc_TC2COS_MAPr_field_get
                    (unit, (void *)&reg_value64, TC_1110_COSf, &temp));
                break;
            case 15:
                SOC_IF_ERROR_RETURN(soc_TC2COS_MAPr_field_get
                    (unit, (void *)&reg_value64, TC_1111_COSf, &temp));
                break;
        }

        reason_info->tc2cosq_prio[i] = temp;
    }

    return SOC_E_NONE;
}

/*
 *  Function : drv_tbx_queue_mode_set
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
drv_tbx_queue_mode_set(int unit, soc_pbmp_t bmp, uint32 flag, 
    uint32 mode)
{
    uint32  reg_value, temp;

    LOG_VERBOSE(BSL_LS_SOC_COMMON, \
                (BSL_META_U(unit, \
                            "drv_tbx_queue_mode_set: \
                            unit %d, bmp = 0x%x, flag = 0x%x, queue mode = %d\n"),
                 unit, SOC_PBMP_WORD_GET(bmp, 0), flag, mode));

    if (flag != 0) {
        return SOC_E_PARAM;
    }

    SOC_IF_ERROR_RETURN(REG_READ_QOS_CTLr
        (unit, &reg_value));

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
            temp = 5;
            break;
        default:
            return SOC_E_UNAVAIL;
    }

    SOC_IF_ERROR_RETURN(soc_QOS_CTLr_field_set
        (unit, &reg_value, SCHEDULE_SELECTf, &temp));
    SOC_IF_ERROR_RETURN(REG_WRITE_QOS_CTLr
        (unit, &reg_value));

    /* WRR : 1: number of packet.
     *  WDRR : 0: number of 64-bytes.
     */
    if (mode == DRV_QUEUE_MODE_WRR) {
        temp = 1;
    } else {
        temp = 0;
    }
    SOC_IF_ERROR_RETURN(DRV_QUEUE_QOS_CONTROL_SET
        (unit, 0, DRV_QOS_CTL_WDRR_GRANULARTTY, temp));

    return SOC_E_NONE;
}

/*
 *  Function : drv_tbx_queue_mode_get
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
drv_tbx_queue_mode_get(int unit, uint32 port, uint32 flag, 
    uint32 *mode)
{
    uint32  reg_value, temp, val = 0;

    if (flag != 0) {
        return SOC_E_PARAM;
    }

    /* Check port number */
    if (port > (SOC_MAX_NUM_PORTS - 1)) {
        return SOC_E_PARAM;
    }

    SOC_IF_ERROR_RETURN(REG_READ_QOS_CTLr
        (unit, &reg_value));
    SOC_IF_ERROR_RETURN(soc_QOS_CTLr_field_get
        (unit, &reg_value, SCHEDULE_SELECTf, &temp));

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
                (unit, 0, DRV_QOS_CTL_WDRR_GRANULARTTY, &val));
            if (val) {
                *mode = DRV_QUEUE_MODE_WRR;
            } else {
                *mode = DRV_QUEUE_MODE_WDRR;
            }
            break;
        default:
            return SOC_E_INTERNAL;
    }

    LOG_VERBOSE(BSL_LS_SOC_COMMON, \
                (BSL_META_U(unit, \
                            "drv_tbx_queue_mode_get: \
                            unit %d, port = %d, flag = 0x%x, queue mode = %d\n"),
                 unit, port, flag, *mode));    

    return SOC_E_NONE;
}

/*
 *  Function : drv_tbx_queue_count_set
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
drv_tbx_queue_count_set(int unit, uint32 port_type, uint8 count)
{
    uint32  reg_value, temp, max_numq;

    LOG_VERBOSE(BSL_LS_SOC_COMMON, \
                (BSL_META_U(unit, \
                            "drv_tbx_queue_count_set: \
                            unit %d, port type = %d, queue count = %d\n"), unit, port_type, count));

    max_numq = NUM_COS(unit);

    /* The number of COS queue is fixed as 8 */
    if (count != max_numq) {
        return SOC_E_PARAM;
    }

    SOC_IF_ERROR_RETURN(REG_READ_QOS_CTLr
        (unit, &reg_value));

    temp = 1;
    SOC_IF_ERROR_RETURN(soc_QOS_CTLr_field_set
        (unit, &reg_value, QOS_ENf, &temp));

    SOC_IF_ERROR_RETURN(REG_WRITE_QOS_CTLr
        (unit, &reg_value));

    return SOC_E_NONE;
}

/*
 *  Function : drv_tbx_queue_count_get
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
drv_tbx_queue_count_get(int unit, uint32 port_type, uint8 *count)
{
    uint32  max_numq;

    max_numq = NUM_COS(unit);

    /* The number of COS queue is fixed as 8 */
    *count = max_numq;

    LOG_VERBOSE(BSL_LS_SOC_COMMON, \
                (BSL_META_U(unit, \
                            "drv_tbx_queue_count_get: \
                            unit %d, port type = %d, queue count = %d\n"), unit, port_type, *count));
 
    return SOC_E_NONE;
}

/* Set WRR weight */
/*
 *  Function : drv_tbx_queue_WRR_weight_set
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
drv_tbx_queue_WRR_weight_set(int unit, uint32 port_type, 
    soc_pbmp_t bmp, uint8 queue, uint32 weight)
{
    uint32  temp;
    uint32  max_weight = 0;
    uint64  reg_value64;

    LOG_VERBOSE(BSL_LS_SOC_COMMON, \
                (BSL_META_U(unit, \
                            "drv_tbx_queue_WRR_weight_set: \
                            unit %d, port type = %d, bmp = 0x%x, queue = %d, weight = %d\n"),
                 unit, port_type, SOC_PBMP_WORD_GET(bmp, 0), queue, weight));

    SOC_IF_ERROR_RETURN(DRV_DEV_PROP_GET
        (unit, DRV_DEV_PROP_COSQ_MAX_WEIGHT_VALUE, &max_weight));

    if ((weight > max_weight) || (weight < 1)) {
        return SOC_E_PARAM;
    }

    SOC_IF_ERROR_RETURN(REG_READ_TXQ_WEIGHT_QUOTA_SZr
        (unit, (void *)&reg_value64));

    temp = weight;
    switch (queue) {
        case 0:
            SOC_IF_ERROR_RETURN(soc_TXQ_WEIGHT_QUOTA_SZr_field_set
                (unit, (void *)&reg_value64, TXQ0_WEIGHT_QUOTAf, &temp));
            break;
        case 1:
            SOC_IF_ERROR_RETURN(soc_TXQ_WEIGHT_QUOTA_SZr_field_set
                (unit, (void *)&reg_value64, TXQ1_WEIGHT_QUOTAf, &temp));
            break;
        case 2:
            SOC_IF_ERROR_RETURN(soc_TXQ_WEIGHT_QUOTA_SZr_field_set
                (unit, (void *)&reg_value64, TXQ2_WEIGHT_QUOTAf, &temp));
            break;
        case 3:
            SOC_IF_ERROR_RETURN(soc_TXQ_WEIGHT_QUOTA_SZr_field_set
                (unit, (void *)&reg_value64, TXQ3_WEIGHT_QUOTAf, &temp));
            break;
        case 4:
            SOC_IF_ERROR_RETURN(soc_TXQ_WEIGHT_QUOTA_SZr_field_set
                (unit, (void *)&reg_value64, TXQ4_WEIGHT_QUOTAf, &temp));
            break;
        case 5:
            SOC_IF_ERROR_RETURN(soc_TXQ_WEIGHT_QUOTA_SZr_field_set
                (unit, (void *)&reg_value64, TXQ5_WEIGHT_QUOTAf, &temp));
            break;
        case 6:
            SOC_IF_ERROR_RETURN(soc_TXQ_WEIGHT_QUOTA_SZr_field_set
                (unit, (void *)&reg_value64, TXQ6_WEIGHT_QUOTAf, &temp));
            break;
        case 7:
            SOC_IF_ERROR_RETURN(soc_TXQ_WEIGHT_QUOTA_SZr_field_set
                (unit, (void *)&reg_value64, TXQ7_WEIGHT_QUOTAf, &temp));
            break;
        default:
            return SOC_E_PARAM;
    }

    SOC_IF_ERROR_RETURN(REG_WRITE_TXQ_WEIGHT_QUOTA_SZr
        (unit, (void *)&reg_value64));

    return SOC_E_NONE;
}

/*
 *  Function : drv_tbx_queue_WRR_weight_get
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
drv_tbx_queue_WRR_weight_get(int unit, uint32 port_type, 
    uint32 port, uint8 queue, uint32 *weight)
{
    uint32  temp;
    uint64  reg_value64;

    SOC_IF_ERROR_RETURN(REG_READ_TXQ_WEIGHT_QUOTA_SZr
        (unit, (void *)&reg_value64));

    switch (queue) {
        case 0:
            SOC_IF_ERROR_RETURN(soc_TXQ_WEIGHT_QUOTA_SZr_field_get
                (unit, (void *)&reg_value64, TXQ0_WEIGHT_QUOTAf, &temp));
            break;
        case 1:
            SOC_IF_ERROR_RETURN(soc_TXQ_WEIGHT_QUOTA_SZr_field_get
                (unit, (void *)&reg_value64, TXQ1_WEIGHT_QUOTAf, &temp));
            break;
        case 2:
            SOC_IF_ERROR_RETURN(soc_TXQ_WEIGHT_QUOTA_SZr_field_get
                (unit, (void *)&reg_value64, TXQ2_WEIGHT_QUOTAf, &temp));
            break;
        case 3:
            SOC_IF_ERROR_RETURN(soc_TXQ_WEIGHT_QUOTA_SZr_field_get
                (unit, (void *)&reg_value64, TXQ3_WEIGHT_QUOTAf, &temp));
            break;
        case 4:
            SOC_IF_ERROR_RETURN(soc_TXQ_WEIGHT_QUOTA_SZr_field_get
                (unit, (void *)&reg_value64, TXQ4_WEIGHT_QUOTAf, &temp));
            break;
        case 5:
            SOC_IF_ERROR_RETURN(soc_TXQ_WEIGHT_QUOTA_SZr_field_get
                (unit, (void *)&reg_value64, TXQ5_WEIGHT_QUOTAf, &temp));
            break;
        case 6:
            SOC_IF_ERROR_RETURN(soc_TXQ_WEIGHT_QUOTA_SZr_field_get
                (unit, (void *)&reg_value64, TXQ6_WEIGHT_QUOTAf, &temp));
            break;
        case 7:
            SOC_IF_ERROR_RETURN(soc_TXQ_WEIGHT_QUOTA_SZr_field_get
                (unit, (void *)&reg_value64, TXQ7_WEIGHT_QUOTAf, &temp));
            break;
        default:
            return SOC_E_PARAM;
    }

    *weight = temp;

    LOG_VERBOSE(BSL_LS_SOC_COMMON, \
                (BSL_META_U(unit, \
                            "drv_tbx_queue_WRR_weight_get: \
                            unit %d, port type = %d, port = %d, queue = %d, weight = %d\n"),
                 unit, port_type, port, queue, *weight));

    return SOC_E_NONE;
}

/* Config output queue mapping */
/*
 *  Function : drv_tbx_queue_prio_set
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
drv_tbx_queue_prio_set(int unit, uint32 port, uint8 prio, uint8 queue_n)
{
    uint32  temp;
    uint64  reg_value64;
	_tbx_cosq_rx_reason_info_t  *reason_info = NULL;

    LOG_VERBOSE(BSL_LS_SOC_COMMON, \
                (BSL_META_U(unit, \
                            "drv_tbx_queue_prio_set: \
                            unit %d, port = %d, priority = %d, queue = %d\n"), 
                 unit, port, prio, queue_n));

    if (port != -1) {
        return SOC_E_UNAVAIL;
    }

    SOC_IF_ERROR_RETURN(REG_READ_TC2COS_MAPr
        (unit, (void *)&reg_value64));

    temp = queue_n;
    switch (prio) {
        case 0:
            SOC_IF_ERROR_RETURN(soc_TC2COS_MAPr_field_set
                (unit, (void *)&reg_value64, TC_0000_COSf, &temp));
            break;
        case 1:         
            SOC_IF_ERROR_RETURN(soc_TC2COS_MAPr_field_set
                (unit, (void *)&reg_value64, TC_0001_COSf, &temp));
            break;
        case 2:         
            SOC_IF_ERROR_RETURN(soc_TC2COS_MAPr_field_set
                (unit, (void *)&reg_value64, TC_0010_COSf, &temp));
            break;
        case 3:
            SOC_IF_ERROR_RETURN(soc_TC2COS_MAPr_field_set
                (unit, (void *)&reg_value64, TC_0011_COSf, &temp));
            break;
        case 4:
            SOC_IF_ERROR_RETURN(soc_TC2COS_MAPr_field_set
                (unit, (void *)&reg_value64, TC_0100_COSf, &temp));
            break;
        case 5:
            SOC_IF_ERROR_RETURN(soc_TC2COS_MAPr_field_set
                (unit, (void *)&reg_value64, TC_0101_COSf, &temp));
            break;
        case 6:         
            SOC_IF_ERROR_RETURN(soc_TC2COS_MAPr_field_set
                (unit, (void *)&reg_value64, TC_0110_COSf, &temp));
            break;
        case 7:
            SOC_IF_ERROR_RETURN(soc_TC2COS_MAPr_field_set
                (unit, (void *)&reg_value64, TC_0111_COSf, &temp));
            break;
        case 8:
            SOC_IF_ERROR_RETURN(soc_TC2COS_MAPr_field_set
                (unit, (void *)&reg_value64, TC_1000_COSf, &temp));
            break;
        case 9:         
            SOC_IF_ERROR_RETURN(soc_TC2COS_MAPr_field_set
                (unit, (void *)&reg_value64, TC_1001_COSf, &temp));
            break;
        case 10:         
            SOC_IF_ERROR_RETURN(soc_TC2COS_MAPr_field_set
                (unit, (void *)&reg_value64, TC_1010_COSf, &temp));
            break;
        case 11:
            SOC_IF_ERROR_RETURN(soc_TC2COS_MAPr_field_set
                (unit, (void *)&reg_value64, TC_1011_COSf, &temp));
            break;
        case 12:
            SOC_IF_ERROR_RETURN(soc_TC2COS_MAPr_field_set
                (unit, (void *)&reg_value64, TC_1100_COSf, &temp));
            break;
        case 13:
            SOC_IF_ERROR_RETURN(soc_TC2COS_MAPr_field_set
                (unit, (void *)&reg_value64, TC_1101_COSf, &temp));
            break;
        case 14:         
            SOC_IF_ERROR_RETURN(soc_TC2COS_MAPr_field_set
                (unit, (void *)&reg_value64, TC_1110_COSf, &temp));
            break;
        case 15:
            SOC_IF_ERROR_RETURN(soc_TC2COS_MAPr_field_set
                (unit, (void *)&reg_value64, TC_1111_COSf, &temp));
            break;
        default:
            return SOC_E_PARAM;
    }

    SOC_IF_ERROR_RETURN(REG_WRITE_TC2COS_MAPr
        (unit, (void *)&reg_value64));

    /* Update sw shadow */
    reason_info = _tbx_rx_reason_info;
    while (reason_info) {
        if ((reason_info->unit == unit) && (reason_info->valid)) {
            /* Found sw shadow */
            reason_info->tc2cosq_prio[prio] = queue_n;
            break;
        }
        reason_info = reason_info->next;
    }

    return SOC_E_NONE;
}

/*
 *  Function : drv_tbx_queue_prio_get
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
drv_tbx_queue_prio_get(int unit, uint32 port, uint8 prio, uint8 *queue_n)
{
    uint32  temp;
    uint32  found;
    uint64  reg_value64;
	_tbx_cosq_rx_reason_info_t  *reason_info = NULL;

    if (port != -1) {
        return SOC_E_UNAVAIL;
    }

    found = 0;
    reason_info = _tbx_rx_reason_info;
    while (reason_info) {
        if ((reason_info->unit == unit) && (reason_info->valid)) {
            /* Found sw shadow */
            found = 1;
            break;
        }
        reason_info = reason_info->next;
    }

    if (found) {
        /* Get from sw shadow */
    	temp = reason_info->tc2cosq_prio[prio];
    	if (temp == _TBX_TC_INVALID) {
            return SOC_E_PARAM;
    	} else {
            *queue_n = temp;
    	}
    } else {
        /* Get from register */
        SOC_IF_ERROR_RETURN(REG_READ_TC2COS_MAPr
            (unit, (void *)&reg_value64));

        switch (prio) {
            case 0:
                SOC_IF_ERROR_RETURN(soc_TC2COS_MAPr_field_get
                    (unit, (void *)&reg_value64, TC_0000_COSf, &temp));
                break;
            case 1:         
                SOC_IF_ERROR_RETURN(soc_TC2COS_MAPr_field_get
                    (unit, (void *)&reg_value64, TC_0001_COSf, &temp));
                break;
            case 2:         
                SOC_IF_ERROR_RETURN(soc_TC2COS_MAPr_field_get
                    (unit, (void *)&reg_value64, TC_0010_COSf, &temp));
                break;
            case 3:
                SOC_IF_ERROR_RETURN(soc_TC2COS_MAPr_field_get
                    (unit, (void *)&reg_value64, TC_0011_COSf, &temp));
                break;
            case 4:
                SOC_IF_ERROR_RETURN(soc_TC2COS_MAPr_field_get
                    (unit, (void *)&reg_value64, TC_0100_COSf, &temp));
                break;
            case 5:
                SOC_IF_ERROR_RETURN(soc_TC2COS_MAPr_field_get
                    (unit, (void *)&reg_value64, TC_0101_COSf, &temp));
                break;
            case 6:         
                SOC_IF_ERROR_RETURN(soc_TC2COS_MAPr_field_get
                    (unit, (void *)&reg_value64, TC_0110_COSf, &temp));
                break;
            case 7:
                SOC_IF_ERROR_RETURN(soc_TC2COS_MAPr_field_get
                    (unit, (void *)&reg_value64, TC_0111_COSf, &temp));
                break;
            case 8:
                SOC_IF_ERROR_RETURN(soc_TC2COS_MAPr_field_get
                    (unit, (void *)&reg_value64, TC_1000_COSf, &temp));
                break;
            case 9:         
                SOC_IF_ERROR_RETURN(soc_TC2COS_MAPr_field_get
                    (unit, (void *)&reg_value64, TC_1001_COSf, &temp));
                break;
            case 10:         
                SOC_IF_ERROR_RETURN(soc_TC2COS_MAPr_field_get
                    (unit, (void *)&reg_value64, TC_1010_COSf, &temp));
                break;
            case 11:
                SOC_IF_ERROR_RETURN(soc_TC2COS_MAPr_field_get
                    (unit, (void *)&reg_value64, TC_1011_COSf, &temp));
                break;
            case 12:
                SOC_IF_ERROR_RETURN(soc_TC2COS_MAPr_field_get
                    (unit, (void *)&reg_value64, TC_1100_COSf, &temp));
                break;
            case 13:
                SOC_IF_ERROR_RETURN(soc_TC2COS_MAPr_field_get
                    (unit, (void *)&reg_value64, TC_1101_COSf, &temp));
                break;
            case 14:         
                SOC_IF_ERROR_RETURN(soc_TC2COS_MAPr_field_get
                    (unit, (void *)&reg_value64, TC_1110_COSf, &temp));
                break;
            case 15:
                SOC_IF_ERROR_RETURN(soc_TC2COS_MAPr_field_get
                    (unit, (void *)&reg_value64, TC_1111_COSf, &temp));
                break;
            default:
                return SOC_E_PARAM;
        }

        *queue_n = temp;
    }

    LOG_VERBOSE(BSL_LS_SOC_COMMON, \
                (BSL_META_U(unit, \
                            "drv_tbx_queue_prio_get: \
                            unit %d, port = %d, priority = %d, queue = %d\n"), 
                 unit, port, prio, *queue_n));

    return SOC_E_NONE;
}

/* Enable/disable selected mapping - prio/diffserv/tos */
/*
 *  Function : drv_tbx_queue_mapping_type_set
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
drv_tbx_queue_mapping_type_set(int unit, soc_pbmp_t bmp, 
    uint32 mapping_type, uint8 state)
{
    uint32  reg_value, temp, val_32;

    LOG_VERBOSE(BSL_LS_SOC_COMMON, \
                (BSL_META_U(unit, \
                            "drv_tbx_queue_mapping_type_set: \
                            unit %d, bmp = 0x%x 0x%x, type = %d, %sable\n"),
                 unit, SOC_PBMP_WORD_GET(bmp, 0), SOC_PBMP_WORD_GET(bmp, 1),
                 mapping_type, (state) ? "en" : "dis"));

    temp = 0;
    switch (mapping_type) {
        case DRV_QUEUE_MAP_NONE:
            /* Disable 802.1P and TOS/DiffServ-based QoS */
            val_32 = SOC_PBMP_WORD_GET(bmp, 0);

            /* Disable TRUST_S1P_CTLr */
            SOC_IF_ERROR_RETURN(REG_READ_TRUST_S1P_CTLr
                (unit, &reg_value));
            SOC_IF_ERROR_RETURN(soc_TRUST_S1P_CTLr_field_get
                (unit, &reg_value, TRUST_S1Pf, &temp));

            temp &= ~val_32;
            SOC_IF_ERROR_RETURN(soc_TRUST_S1P_CTLr_field_set
                (unit, &reg_value, TRUST_S1Pf, &temp));
            SOC_IF_ERROR_RETURN(REG_WRITE_TRUST_S1P_CTLr
                (unit, &reg_value)); 

            /* Disable TRUST_C1P_CTLr */
            SOC_IF_ERROR_RETURN(REG_READ_TRUST_C1P_CTLr
                (unit, &reg_value));
            SOC_IF_ERROR_RETURN(soc_TRUST_C1P_CTLr_field_get
                (unit, &reg_value, TRUST_C1Pf, &temp));

            temp &= ~val_32;
            SOC_IF_ERROR_RETURN(soc_TRUST_C1P_CTLr_field_set
                (unit, &reg_value, TRUST_C1Pf, &temp));
            SOC_IF_ERROR_RETURN(REG_WRITE_TRUST_C1P_CTLr
                (unit, &reg_value));

            /* Disable TRUST_DSCP_CTLr */
            SOC_IF_ERROR_RETURN(REG_READ_TRUST_DSCP_CTLr
                (unit, &reg_value));
            SOC_IF_ERROR_RETURN(soc_TRUST_DSCP_CTLr_field_get
                (unit, &reg_value, QOS_TOS_DIFF_ENf, &temp));

            temp &= ~val_32;
            SOC_IF_ERROR_RETURN(soc_TRUST_DSCP_CTLr_field_set
                (unit, &reg_value, QOS_TOS_DIFF_ENf, &temp));
            SOC_IF_ERROR_RETURN(REG_WRITE_TRUST_DSCP_CTLr
                (unit, &reg_value));
            break;
        case DRV_QUEUE_MAP_PRIO:
            /* Enable/Disable 802.1P-based QoS */

            /* Enable / Disable both TRUST_S1P_CTLr and TRUST_C1P_CTLr */
            val_32 = SOC_PBMP_WORD_GET(bmp, 0);

            /* Enable / Disable TRUST_S1P_CTLr */
            SOC_IF_ERROR_RETURN(REG_READ_TRUST_S1P_CTLr
                (unit, &reg_value));
            SOC_IF_ERROR_RETURN(soc_TRUST_S1P_CTLr_field_get
                (unit, &reg_value, TRUST_S1Pf, &temp));

            if (state) {
                temp |= val_32;
            } else {
                temp &= ~val_32;
            }
            SOC_IF_ERROR_RETURN(soc_TRUST_S1P_CTLr_field_set
                (unit, &reg_value, TRUST_S1Pf, &temp));
            SOC_IF_ERROR_RETURN(REG_WRITE_TRUST_S1P_CTLr
                (unit, &reg_value)); 
    
            /* Enable / Disable TRUST_C1P_CTLr */
            SOC_IF_ERROR_RETURN(REG_READ_TRUST_C1P_CTLr
                (unit, &reg_value));
            SOC_IF_ERROR_RETURN(soc_TRUST_C1P_CTLr_field_get
                (unit, &reg_value, TRUST_C1Pf, &temp));

            if (state) {
                temp |= val_32;
            } else {
                temp &= ~val_32;
            }
            SOC_IF_ERROR_RETURN(soc_TRUST_C1P_CTLr_field_set
                (unit, &reg_value, TRUST_C1Pf, &temp));
            SOC_IF_ERROR_RETURN(REG_WRITE_TRUST_C1P_CTLr
                (unit, &reg_value));
            break;
        case DRV_QUEUE_MAP_PRIO_S1P:
            /* Enable/Disable 802.1P-based QoS */
    
            /* Enable / Disable TRUST_S1P_CTLr */
            SOC_IF_ERROR_RETURN(REG_READ_TRUST_S1P_CTLr
                (unit, &reg_value));
            SOC_IF_ERROR_RETURN(soc_TRUST_S1P_CTLr_field_get
                (unit, &reg_value, TRUST_S1Pf, &temp));

            val_32 = SOC_PBMP_WORD_GET(bmp, 0);
            if (state) {
                temp |= val_32;
            } else {
                temp &= ~val_32;
            }
            SOC_IF_ERROR_RETURN(soc_TRUST_S1P_CTLr_field_set
                (unit, &reg_value, TRUST_S1Pf, &temp));
            SOC_IF_ERROR_RETURN(REG_WRITE_TRUST_S1P_CTLr
                (unit, &reg_value));     
            break;
        case DRV_QUEUE_MAP_PRIO_C1P:
            /* Enable/Disable 802.1P-based QoS */
    
            /* Enable / Disable TRUST_C1P_CTLr */
            SOC_IF_ERROR_RETURN(REG_READ_TRUST_C1P_CTLr
                (unit, &reg_value));
            SOC_IF_ERROR_RETURN(soc_TRUST_C1P_CTLr_field_get
                (unit, &reg_value, TRUST_C1Pf, &temp));

            val_32 = SOC_PBMP_WORD_GET(bmp, 0);
            if (state) {
                temp |= val_32;
            } else {
                temp &= ~val_32;
            }
            SOC_IF_ERROR_RETURN(soc_TRUST_C1P_CTLr_field_set
                (unit, &reg_value, TRUST_C1Pf, &temp));
            SOC_IF_ERROR_RETURN(REG_WRITE_TRUST_C1P_CTLr
                (unit, &reg_value));
            break;
        case DRV_QUEUE_MAP_DFSV:
            /* Enable/Disable TOS/DiffServ-based QoS */
    
            /* Enable / Disable TRUST_DSCP_CTLr */
            SOC_IF_ERROR_RETURN(REG_READ_TRUST_DSCP_CTLr
                (unit, &reg_value));
            SOC_IF_ERROR_RETURN(soc_TRUST_DSCP_CTLr_field_get
                (unit, &reg_value, QOS_TOS_DIFF_ENf, &temp));

            val_32 = SOC_PBMP_WORD_GET(bmp, 0);
            if (state) {
                temp |= val_32;
            } else {
                temp &= ~val_32;
            }
            SOC_IF_ERROR_RETURN(soc_TRUST_DSCP_CTLr_field_set
                (unit, &reg_value, QOS_TOS_DIFF_ENf, &temp));
            SOC_IF_ERROR_RETURN(REG_WRITE_TRUST_DSCP_CTLr
                (unit, &reg_value));
            break;
        case DRV_QUEUE_MAP_PORT:
            /* TBX have no force port based priority solution :
             *  - the default port priority can be retrieved only if all  
             *      other TC decision processes been disabled.
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
 *  Function : drv_tbx_queue_mapping_type_get
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
drv_tbx_queue_mapping_type_get(int unit, uint32 port, 
    uint32 mapping_type, uint8 *state)
{
    uint32  reg_value, temp;
    soc_pbmp_t  pbmp;

    switch (mapping_type) {
        case DRV_QUEUE_MAP_NONE:
            return SOC_E_PARAM;
        case DRV_QUEUE_MAP_PRIO:
            SOC_IF_ERROR_RETURN(REG_READ_TRUST_S1P_CTLr
                (unit, &reg_value));
            SOC_IF_ERROR_RETURN(soc_TRUST_S1P_CTLr_field_get
                (unit, &reg_value, TRUST_S1Pf, &temp));

            SOC_PBMP_CLEAR(pbmp);
            SOC_PBMP_WORD_SET(pbmp, 0, temp);
            if (SOC_PBMP_MEMBER(pbmp, port)) {
                *state = TRUE;
                break;
            } 

            SOC_IF_ERROR_RETURN(REG_READ_TRUST_C1P_CTLr
                (unit, &reg_value));
            SOC_IF_ERROR_RETURN(soc_TRUST_C1P_CTLr_field_get
                (unit, &reg_value, TRUST_C1Pf, &temp));

            SOC_PBMP_CLEAR(pbmp);
            SOC_PBMP_WORD_SET(pbmp, 0, temp);
            if (SOC_PBMP_MEMBER(pbmp, port)) {
                *state = TRUE;
                break;
            } 

            *state = FALSE;
            break;
        case DRV_QUEUE_MAP_PRIO_S1P:
            SOC_IF_ERROR_RETURN(REG_READ_TRUST_S1P_CTLr
                (unit, &reg_value));
            SOC_IF_ERROR_RETURN(soc_TRUST_S1P_CTLr_field_get
                (unit, &reg_value, TRUST_S1Pf, &temp));

            SOC_PBMP_CLEAR(pbmp);
            SOC_PBMP_WORD_SET(pbmp, 0, temp);
            if (SOC_PBMP_MEMBER(pbmp, port)) {
                *state = TRUE;
            } else {
                *state = FALSE;
            }
            break;
        case DRV_QUEUE_MAP_PRIO_C1P:
            SOC_IF_ERROR_RETURN(REG_READ_TRUST_C1P_CTLr
                (unit, &reg_value));
            SOC_IF_ERROR_RETURN(soc_TRUST_C1P_CTLr_field_get
                (unit, &reg_value, TRUST_C1Pf, &temp));

            SOC_PBMP_CLEAR(pbmp);
            SOC_PBMP_WORD_SET(pbmp, 0, temp);
            if (SOC_PBMP_MEMBER(pbmp, port)) {
                *state = TRUE;
            } else {
                *state = FALSE;
            }
            break;
        case DRV_QUEUE_MAP_DFSV:
            SOC_IF_ERROR_RETURN(REG_READ_TRUST_DSCP_CTLr
                (unit, &reg_value));
            SOC_IF_ERROR_RETURN(soc_TRUST_DSCP_CTLr_field_get
                (unit, &reg_value, QOS_TOS_DIFF_ENf, &temp));

            SOC_PBMP_CLEAR(pbmp);
            SOC_PBMP_WORD_SET(pbmp, 0, temp);
            if (SOC_PBMP_MEMBER(pbmp, port)) {
                *state = TRUE;
            } else {
                *state = FALSE;
            }
            break;
        case DRV_QUEUE_MAP_PORT:
            /* TBX have no force port based priority solution :
             *  - the default port priority can be retrieved only if all  
             *      other TC decision processes been disabled.
             */
        case DRV_QUEUE_MAP_TOS:
        case DRV_QUEUE_MAP_MAC:
        case DRV_QUEUE_MAP_HYBRID:
            return SOC_E_UNAVAIL;
        default:
            return SOC_E_PARAM;
    }

    LOG_VERBOSE(BSL_LS_SOC_COMMON, \
                (BSL_META_U(unit, \
                            "drv_tbx_queue_mapping_type_get: \
                            unit %d, port = %d, type = %d, %sable\n"),
                 unit, port, mapping_type, (*state) ? "en" : "dis"));

    return SOC_E_NONE;
}

static int _tbx_dscp2tcdp_map_reg[] = {
    INDEX(DSCP2TC_DP_MAP0r),
    INDEX(DSCP2TC_DP_MAP1r),
    INDEX(DSCP2TC_DP_MAP2r),
    INDEX(DSCP2TC_DP_MAP3r),
    INDEX(DSCP2TC_DP_MAP4r),
    INDEX(DSCP2TC_DP_MAP5r),
    INDEX(DSCP2TC_DP_MAP6r),
    INDEX(DSCP2TC_DP_MAP7r)
};
static int _tbx_dscp2tcdp_map_field[] = {
    INDEX(PRI_DSCP_000000f),
    INDEX(PRI_DSCP_000001f),
    INDEX(PRI_DSCP_000010f),
    INDEX(PRI_DSCP_000011f),
    INDEX(PRI_DSCP_000100f),
    INDEX(PRI_DSCP_000101f),
    INDEX(PRI_DSCP_000110f),
    INDEX(PRI_DSCP_000111f)
};

static int _tbx_tcdp2dscp_map_reg[] = {
    INDEX(TC_DP2DSCP_MAP0r),
    INDEX(TC_DP2DSCP_MAP1r),
    INDEX(TC_DP2DSCP_MAP2r),
    INDEX(TC_DP2DSCP_MAP3r),
    INDEX(TC_DP2DSCP_MAP4r),
    INDEX(TC_DP2DSCP_MAP5r),
    INDEX(TC_DP2DSCP_MAP6r),
    INDEX(TC_DP2DSCP_MAP7r)
};

static int _tbx_tcdp2dscp_map_field[] = {
    INDEX(DPTC_000000f),
    INDEX(DPTC_000001f),
    INDEX(DPTC_000010f),
    INDEX(DPTC_000011f),
    INDEX(DPTC_000100f),
    INDEX(DPTC_000101f),
    INDEX(DPTC_000110f),
    INDEX(DPTC_000111f)
};

/*
 *  Function : drv_tbx_queue_dfsv_remap_set
 *
 *  Purpose :
 *      Set the TC_DP priority value of the specific DSCP mapping to.
 *
 *  Parameters :
 *      unit     :  RoboSwitch unit number.
 *      dscp     :  DSCP priority value.
 *      prio     :  {DP[1:0], TC[3:0]} priority value (0 ~ 255).
 *
 *  Return :
 *      SOC_E_XXX.
 *
 *  Note :
 */
int 
drv_tbx_queue_dfsv_remap_set(int unit, uint8 dscp, uint8 prio)
{
    uint64  reg_value64;
    uint32  temp;
	int     rv;

    /* The parameter prio means {DP[1:0], TC[3:0]} for TB */
    LOG_VERBOSE(BSL_LS_SOC_COMMON, \
                (BSL_META_U(unit, \
                            "drv_tbx_queue_dfsv_remap_set: \
                            unit %d, dscp value = %d, {DP[1:0], TC[3:0]} = %d\n"), 
                 unit, dscp, prio));

    if (SOC_IS_TB_AX(unit)) {
        /* TB A0: DSCP2TCDP access from registers */
        rv = DRV_REG_READ(unit, 
            DRV_REG_ADDR(unit, _tbx_dscp2tcdp_map_reg[dscp / 8], 0 ,0), 
            (void *)&reg_value64,
            DRV_REG_LENGTH_GET(unit, _tbx_dscp2tcdp_map_reg[dscp / 8]));
        SOC_IF_ERROR_RETURN(rv);

        temp = prio;
        /* 
          * DSCP2TC_DP_MAP0/1/2/3/4/5/6/7r have the same fields format (6 bits for each field).
          * Use DSCP2TC_DP_MAP0r to set each register related field's value.
          */
        rv = DRV_REG_FIELD_SET
            (unit, INDEX(DSCP2TC_DP_MAP0r), (void *)&reg_value64, 
            _tbx_dscp2tcdp_map_field[dscp % 8], &temp);
        SOC_IF_ERROR_RETURN(rv);

        rv = DRV_REG_WRITE(unit, 
            DRV_REG_ADDR(unit, _tbx_dscp2tcdp_map_reg[dscp / 8], 0 ,0),
            (void *)&reg_value64,
            DRV_REG_LENGTH_GET(unit, _tbx_dscp2tcdp_map_reg[dscp / 8]));

        return rv;
    } else {
        /* Changed to memory access */
        temp = prio;
        rv = MEM_WRITE_DSCPECN2TCDPm(unit, dscp, &temp);
        return rv;
    }
}

/*
 *  Function : drv_tbx_queue_dfsv_remap_get
 *
 *  Purpose :
 *      Get the TC_DP priority value of the specific DSCP mapping to.
 *
 *  Parameters :
 *      unit     :  RoboSwitch unit number.
 *      dscp     :  DSCP priority value.
 *      prio     :  (OUT) {DP[1:0], TC[3:0]} priority value (0 ~ 255).
 *
 *  Return :
 *      SOC_E_XXX.
 *
 *  Note :
 */
int 
drv_tbx_queue_dfsv_remap_get(int unit, uint8 dscp, uint8 *prio)
{
    uint64  reg_value64;
    uint32  temp;
	int     rv;

    if (SOC_IS_TB_AX(unit)) {
        /* TB A0: DSCP2TCDP access from registers */
        rv = DRV_REG_READ(unit, 
            DRV_REG_ADDR(unit, _tbx_dscp2tcdp_map_reg[dscp / 8], 0 ,0),
            (void *)&reg_value64,
            DRV_REG_LENGTH_GET(unit, _tbx_dscp2tcdp_map_reg[dscp / 8]));
        SOC_IF_ERROR_RETURN(rv);

        /* 
          * DSCP2TC_DP_MAP0/1/2/3/4/5/6/7r have the same fields format (6 bits for each field).
          * Use DSCP2TC_DP_MAP0r to get each register related field's value.
          */
        rv = DRV_REG_FIELD_GET
            (unit, INDEX(DSCP2TC_DP_MAP0r), (void *)&reg_value64, 
            _tbx_dscp2tcdp_map_field[dscp % 8], &temp);
        SOC_IF_ERROR_RETURN(rv);

        *prio = temp;
        LOG_VERBOSE(BSL_LS_SOC_COMMON, \
                    (BSL_META_U(unit, \
                                "drv_tbx_queue_dfsv_remap_get: \
                                unit %d, dscp value = %d, {DP[1:0], TC[3:0]} = %d\n"),
                     unit, dscp, *prio));

        return SOC_E_NONE;
    } else {
        /* Changed to memory access */
        rv = MEM_READ_DSCPECN2TCDPm(unit, dscp, &temp);
        SOC_IF_ERROR_RETURN(rv);

        *prio = temp;

        return rv;
    }
}

/*
 *  Function : drv_tbx_queue_dfsv_unmap_set
 *
 *  Purpose :
 *      Set the DSCP value of the specific TC_DP priority mapping to.
 *
 *  Parameters :
 *      unit     :  RoboSwitch unit number.
 *      prio     :  {DP[1:0], TC[3:0]} priority value (0 ~ 255).
 *      dscp     :  DSCP priority value.
 *
 *  Return :
 *      SOC_E_XXX.
 *
 *  Note :
 */
int 
drv_tbx_queue_dfsv_unmap_set(int unit, uint8 prio, uint8 dscp)
{
    uint64  reg_value64;
    uint32  temp;
    int     rv;

    /* The parameter prio means {DP[1:0], TC[3:0]} for TB */
    LOG_VERBOSE(BSL_LS_SOC_COMMON, \
                (BSL_META_U(unit, \
                            "drv_tbx_queue_dfsv_unmap_set: \
                            unit %d, {DP[1:0], TC[3:0]} = %d, dscp value = %d\n"), 
                 unit, prio, dscp));

    if (SOC_IS_TB_AX(unit)) {
        /* TB A0: TCDP2DSCP access from registers */
        rv = DRV_REG_READ(unit, 
            DRV_REG_ADDR(unit, _tbx_tcdp2dscp_map_reg[prio / 8], 0 ,0),
            (void *)&reg_value64,
            DRV_REG_LENGTH_GET(unit, _tbx_tcdp2dscp_map_reg[prio / 8]));
        SOC_IF_ERROR_RETURN(rv);

        temp = dscp;
        /* 
          * TC_DP2DSCP_MAP0/1/2/3/4/5/6/7r have the same fields format (6 bits for each field).
          * Use TC_DP2DSCP_MAP0r to set each register related field's value.
          */
        rv = DRV_REG_FIELD_SET
            (unit, INDEX(TC_DP2DSCP_MAP0r), (void *)&reg_value64, 
            _tbx_tcdp2dscp_map_field[prio%8], &temp);
        SOC_IF_ERROR_RETURN(rv);

        rv = DRV_REG_WRITE(unit, 
            DRV_REG_ADDR(unit, _tbx_tcdp2dscp_map_reg[prio / 8], 0 ,0),
            (void *)&reg_value64,
            DRV_REG_LENGTH_GET(unit, _tbx_tcdp2dscp_map_reg[prio / 8]));
        SOC_IF_ERROR_RETURN(rv);

        return SOC_E_NONE;
    } else {
        /* Changed to memory access */
        temp = dscp;
        rv = MEM_WRITE_TCDP2DSCPECNm(unit, prio, &temp);
        SOC_IF_ERROR_RETURN(rv);

        return rv;
    }
}

/*
 *  Function : drv_tbx_queue_dfsv_unmap_get
 *
 *  Purpose :
 *      Get the DSCP value of the specific TC_DP priority mapping to.
 *
 *  Parameters :
 *      unit     :  RoboSwitch unit number.
 *      prio     :  {DP[1:0], TC[3:0]} priority value (0 ~ 255).
 *      dscp     :  (OUT) DSCP priority value.
 *
 *  Return :
 *      SOC_E_XXX.
 *
 *  Note :
 */
int 
drv_tbx_queue_dfsv_unmap_get(int unit, uint8 prio, uint8 *dscp)
{
    uint64  reg_value64;
    uint32  temp;
    int     rv;

    /* The parameter prio means {DP[1:0], TC[3:0]} for TB */
    if (SOC_IS_TB_AX(unit)) {
        /* TB A0: TCDP2DSCP access from registers */
        rv = DRV_REG_READ(unit, 
            DRV_REG_ADDR(unit, _tbx_tcdp2dscp_map_reg[prio/ 8], 0 ,0),
            (void *)&reg_value64,
            DRV_REG_LENGTH_GET(unit, _tbx_tcdp2dscp_map_reg[prio / 8]));
        SOC_IF_ERROR_RETURN(rv);

        /* 
          * TC_DP2DSCP_MAP0/1/2/3/4/5/6/7r have the same fields format (6 bits for each field).
          * Use TC_DP2DSCP_MAP0r to get each register related field's value.
          */
        rv = DRV_REG_FIELD_GET
            (unit, INDEX(TC_DP2DSCP_MAP0r), (void *)&reg_value64, 
            _tbx_tcdp2dscp_map_field[prio%8], &temp);
        SOC_IF_ERROR_RETURN(rv);

        *dscp = temp;
        LOG_VERBOSE(BSL_LS_SOC_COMMON, \
                    (BSL_META_U(unit, \
                                "drv_tbx_queue_dfsv_unmap_get: \
                                unit %d, {DP[1:0], TC[3:0]} = %d, dscp value = %d\n"), 
                     unit, prio, *dscp));

        return SOC_E_NONE;
    } else {
        /* TB B0: change to memory access */
        rv = MEM_READ_TCDP2DSCPECNm(unit, prio, &temp);
        SOC_IF_ERROR_RETURN(rv);

        *dscp = temp;

        return rv;
    }
}

/*
 *  Function : drv_tbx_queue_rx_reason_set
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
drv_tbx_queue_rx_reason_set(int unit, uint8 reason, uint32 queue)
{
    uint64  reg_value64;
    uint32  temp;
    _tbx_cosq_rx_reason_info_t  *reason_info = NULL;

    LOG_VERBOSE(BSL_LS_SOC_COMMON, \
                (BSL_META_U(unit, \
                            "drv_tbx_queue_rx_reason_set: \
                            unit %d, reason = %d, queue = %d\n"), unit, reason, queue));

    SOC_IF_ERROR_RETURN(REG_READ_QOS_REASON_CODEr
        (unit, (void *)&reg_value64));

    temp = queue;
    switch (reason) {
        case DRV_RX_REASON_SWITCHING: 
            SOC_IF_ERROR_RETURN(soc_QOS_REASON_CODEr_field_set
                (unit, (void *)&reg_value64, ARL_KNOWN_DA_TERMINf, &temp));
            break;
        case DRV_RX_REASON_EXCEPTION:
            SOC_IF_ERROR_RETURN(soc_QOS_REASON_CODEr_field_set
                (unit, (void *)&reg_value64, ARL_UNKNOWN_DA_FLOODf, &temp));
            break;
        case DRV_RX_REASON_PROTO_TERM: 
            SOC_IF_ERROR_RETURN(soc_QOS_REASON_CODEr_field_set
                (unit, (void *)&reg_value64, ARL_8021_PROT_TRAPf, &temp));
            break;
        case DRV_RX_REASON_PROTO_SNOOP:
            SOC_IF_ERROR_RETURN(soc_QOS_REASON_CODEr_field_set
                (unit, (void *)&reg_value64, ARL_APPL_PROT_SNOOPf, &temp));
            break;
        case DRV_RX_REASON_ARL_VALN_DIR_FWD:
            SOC_IF_ERROR_RETURN(soc_QOS_REASON_CODEr_field_set
                (unit, (void *)&reg_value64, ARL_VLAN_DIR_FWDf, &temp));
            break;
        case DRV_RX_REASON_ARL_CFP_FWD:
            SOC_IF_ERROR_RETURN(soc_QOS_REASON_CODEr_field_set
                (unit, (void *)&reg_value64, ARL_CFP_FWDf, &temp));
            break;
        case DRV_RX_REASON_ARL_LOOPBACK:
            SOC_IF_ERROR_RETURN(soc_QOS_REASON_CODEr_field_set
                (unit, (void *)&reg_value64, ARL_LOOPBACKf, &temp));
            break;
        case DRV_RX_REASON_MIRRORING:
            SOC_IF_ERROR_RETURN(soc_QOS_REASON_CODEr_field_set
                (unit, (void *)&reg_value64, MIRROR_COPYf, &temp));
            break;
        case DRV_RX_REASON_INGRESS_SFLOW:
            SOC_IF_ERROR_RETURN(soc_QOS_REASON_CODEr_field_set
                (unit, (void *)&reg_value64, INGRESS_SFLOWf, &temp));
            break;
        case DRV_RX_REASON_EGRESS_SFLOW:
            SOC_IF_ERROR_RETURN(soc_QOS_REASON_CODEr_field_set
                (unit, (void *)&reg_value64, EGRESS_SFLOWf, &temp));
            break;
        case DRV_RX_REASON_SA_MOVEMENT_EVENT:
            SOC_IF_ERROR_RETURN(soc_QOS_REASON_CODEr_field_set
                (unit, (void *)&reg_value64, SA_MOVEMENT_EVENTf, &temp));
            break;
        case DRV_RX_REASON_SA_UNKNOWN_EVENT:
            SOC_IF_ERROR_RETURN(soc_QOS_REASON_CODEr_field_set
                (unit, (void *)&reg_value64, SA_UNKNOWN_EVENTf, &temp));
            break;
        case DRV_RX_REASON_SA_OVER_LIMIT_EVENT:
            SOC_IF_ERROR_RETURN(soc_QOS_REASON_CODEr_field_set
                (unit, (void *)&reg_value64, SA_OVER_LIMIT_EVENTf, &temp));
            break;
        case DRV_RX_REASON_INP_NON_MEMBER:
            SOC_IF_ERROR_RETURN(soc_QOS_REASON_CODEr_field_set
                (unit, (void *)&reg_value64, INP_NON_MEMBERf, &temp));
            break;
        case DRV_RX_REASON_VLAN_UNKNOWN:
            SOC_IF_ERROR_RETURN(soc_QOS_REASON_CODEr_field_set
                (unit, (void *)&reg_value64, VLAN_UNKNOWNf, &temp));
            break;
        default:
            return SOC_E_UNAVAIL;
    }
    SOC_IF_ERROR_RETURN(REG_WRITE_QOS_REASON_CODEr
        (unit, (void *)&reg_value64));

    /* Update sw shadow */
    reason_info = _tbx_rx_reason_info;
    while (reason_info) {
        if ((reason_info->unit == unit) && (reason_info->valid)) {
            /* Found sw shadow */
            reason_info->reason_cosq[reason] = queue;
            break;
        }
        reason_info = reason_info->next;
    }

    return SOC_E_NONE;
}

/*
 *  Function : drv_tbx_queue_rx_reason_get
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
drv_tbx_queue_rx_reason_get(int unit, uint8 reason, uint32 *queue)
{
    uint64  reg_value64;
    uint32  temp;
    uint32  found;
    _tbx_cosq_rx_reason_info_t  *reason_info = NULL;

    found = 0;
    reason_info = _tbx_rx_reason_info;
    while (reason_info) {
        if ((reason_info->unit == unit) && (reason_info->valid)) {
            /* Found sw shadow */
            found = 1;
            break;
        }
        reason_info = reason_info->next;
    }

    if (found) {
        /* Get from sw shadow */
        temp = reason_info->reason_cosq[reason];
        if (temp == _TBX_RX_REASON_INVALID) {
            return SOC_E_UNAVAIL;
        } else {
            *queue = reason_info->reason_cosq[reason];
        }
    } else {
        /* Get from register */
        SOC_IF_ERROR_RETURN(REG_READ_QOS_REASON_CODEr
            (unit, (void *)&reg_value64));

        switch (reason) {
            case DRV_RX_REASON_SWITCHING:
                SOC_IF_ERROR_RETURN(soc_QOS_REASON_CODEr_field_get(unit, 
                    (void *)&reg_value64, ARL_KNOWN_DA_TERMINf, &temp));
                break;
            case DRV_RX_REASON_EXCEPTION:
                SOC_IF_ERROR_RETURN(soc_QOS_REASON_CODEr_field_get(unit, 
                    (void *)&reg_value64, ARL_UNKNOWN_DA_FLOODf, &temp));
                break;
            case DRV_RX_REASON_PROTO_TERM:
                SOC_IF_ERROR_RETURN(soc_QOS_REASON_CODEr_field_get(unit, 
                    (void *)&reg_value64, ARL_8021_PROT_TRAPf, &temp));
                break;
            case DRV_RX_REASON_PROTO_SNOOP:
                SOC_IF_ERROR_RETURN(soc_QOS_REASON_CODEr_field_get(unit, 
                    (void *)&reg_value64, ARL_APPL_PROT_SNOOPf, &temp));
                break;
            case DRV_RX_REASON_ARL_VALN_DIR_FWD:
                SOC_IF_ERROR_RETURN(soc_QOS_REASON_CODEr_field_get
                    (unit, (void *)&reg_value64, ARL_VLAN_DIR_FWDf, &temp));
                break;
            case DRV_RX_REASON_ARL_CFP_FWD:
                SOC_IF_ERROR_RETURN(soc_QOS_REASON_CODEr_field_get
                    (unit, (void *)&reg_value64, ARL_CFP_FWDf, &temp));
                break;
            case DRV_RX_REASON_ARL_LOOPBACK:
                SOC_IF_ERROR_RETURN(soc_QOS_REASON_CODEr_field_get
                    (unit, (void *)&reg_value64, ARL_LOOPBACKf, &temp));
                break;
            case DRV_RX_REASON_MIRRORING:
                SOC_IF_ERROR_RETURN(soc_QOS_REASON_CODEr_field_get
                    (unit, (void *)&reg_value64, MIRROR_COPYf, &temp));
                break;
            case DRV_RX_REASON_INGRESS_SFLOW:
                SOC_IF_ERROR_RETURN(soc_QOS_REASON_CODEr_field_get
                    (unit, (void *)&reg_value64, INGRESS_SFLOWf, &temp));
                break;
            case DRV_RX_REASON_EGRESS_SFLOW:
                SOC_IF_ERROR_RETURN(soc_QOS_REASON_CODEr_field_get
                    (unit, (void *)&reg_value64, EGRESS_SFLOWf, &temp));
                break;
            case DRV_RX_REASON_SA_MOVEMENT_EVENT:
                SOC_IF_ERROR_RETURN(soc_QOS_REASON_CODEr_field_get
                    (unit, (void *)&reg_value64, SA_MOVEMENT_EVENTf, &temp));
                break;
            case DRV_RX_REASON_SA_UNKNOWN_EVENT:
                SOC_IF_ERROR_RETURN(soc_QOS_REASON_CODEr_field_get
                    (unit, (void *)&reg_value64, SA_UNKNOWN_EVENTf, &temp));
                break;
            case DRV_RX_REASON_SA_OVER_LIMIT_EVENT:
                SOC_IF_ERROR_RETURN(soc_QOS_REASON_CODEr_field_get(unit, 
                    (void *)&reg_value64, SA_OVER_LIMIT_EVENTf, &temp));
                break;
            case DRV_RX_REASON_INP_NON_MEMBER:
                SOC_IF_ERROR_RETURN(soc_QOS_REASON_CODEr_field_get
                    (unit, (void *)&reg_value64, INP_NON_MEMBERf, &temp));
                break;
            case DRV_RX_REASON_VLAN_UNKNOWN:
                SOC_IF_ERROR_RETURN(soc_QOS_REASON_CODEr_field_get
                    (unit, (void *)&reg_value64, VLAN_UNKNOWNf, &temp));
                break;
            default:
                return SOC_E_UNAVAIL;
        }

        *queue = temp;
    }

    LOG_VERBOSE(BSL_LS_SOC_COMMON, \
                (BSL_META_U(unit, \
                            "drv_tbx_queue_rx_reason_get: \
                            unit %d, reason = %d, *queue = %d\n"), unit, reason, *queue));

    return SOC_E_NONE;
}

/*
 *  Function : drv_tbx_queue_port_txq_pause_set
 *
 *  Purpose :
 *      Enable/Disable the capability of pausing traffic scheduling on the specific queue
 *      for IMP and giga ports.
 *
 *  Parameters :
 *      unit     :  RoboSwitch unit number.
 *      port     :  port number.
 *      queue_n  :  queue number.
 *      enable   :  The state of the capability of pausing traffic scheduling on 
 *                      selected TXQ-Qn (n = 0 ~ 7) for IMP and giga ports.
 *
 *  Return :
 *      SOC_E_XXX.
 *
 *  Note :
 */
int 
drv_tbx_queue_port_txq_pause_set(int unit, uint32 port, 
    uint8 queue_n, uint8 enable)
{
    uint32  reg_value;
    uint32  temp;

    LOG_VERBOSE(BSL_LS_SOC_COMMON, \
                (BSL_META_U(unit, \
                            "drv_tbx_queue_port_txq_pause_set: \
                            unit %d, port = %d, queue = %d, state = %d\n"), 
                 unit, port, queue_n, enable));

    /* Check port is IMP port or GE ports */
    if ((!IS_GE_PORT(unit, port)) && (!IS_CPU_PORT(unit, port))) {
        return SOC_E_PARAM;
    }

    switch (port) {
        case 24:
            SOC_IF_ERROR_RETURN(REG_READ_TXQ_BACKPRESSURE_IMP_CTLr
                (unit, &reg_value));
            break;
        case 25:
            SOC_IF_ERROR_RETURN(REG_READ_TXQ_BACKPRESSURE_GE0_CTLr
                (unit, &reg_value));
            break;
        case 26:
            SOC_IF_ERROR_RETURN(REG_READ_TXQ_BACKPRESSURE_GE1_CTLr
                (unit, &reg_value));
            break;
        case 27:
            SOC_IF_ERROR_RETURN(REG_READ_TXQ_BACKPRESSURE_GE2_CTLr
                (unit, &reg_value));
            break;
        case 28:
            SOC_IF_ERROR_RETURN(REG_READ_TXQ_BACKPRESSURE_GE3_CTLr
                (unit, &reg_value));
            break;
        default :
            return SOC_E_PARAM;
    }

    /* 
      * TXQ_BACKPRESSURE_IMP_CTLr/TXQ_BACKPRESSURE_GE0_CTLr
      * TXQ_BACKPRESSURE_GE1_CTLr
      * TXQ_BACKPRESSURE_GE2_CTLr/TXQ_BACKPRESSURE_GE3_CTLr
      * have the same fields format (one bits for each field, TXQ0 ~ 7).
      * Use TXQ_BACKPRESSURE_IMP_CTLr to set each register related field's value.
      */
    if (enable) {
        temp = 1;
    } else {
        temp = 0;
    }
    switch ((queue_n % 8)) {
        case 0:
            SOC_IF_ERROR_RETURN(soc_TXQ_BACKPRESSURE_IMP_CTLr_field_set
                (unit, &reg_value, EN_TXQ0_BP_IMPf, &temp));
            break;
        case 1:         
            SOC_IF_ERROR_RETURN(soc_TXQ_BACKPRESSURE_IMP_CTLr_field_set
                (unit, &reg_value, EN_TXQ1_BP_IMPf, &temp));
            break;
        case 2:         
            SOC_IF_ERROR_RETURN(soc_TXQ_BACKPRESSURE_IMP_CTLr_field_set
                (unit, &reg_value, EN_TXQ2_BP_IMPf, &temp));
            break;
        case 3:
            SOC_IF_ERROR_RETURN(soc_TXQ_BACKPRESSURE_IMP_CTLr_field_set
                (unit, &reg_value, EN_TXQ3_BP_IMPf, &temp));
            break;
        case 4:
            SOC_IF_ERROR_RETURN(soc_TXQ_BACKPRESSURE_IMP_CTLr_field_set
                (unit, &reg_value, EN_TXQ4_BP_IMPf, &temp));
            break;
        case 5:
            SOC_IF_ERROR_RETURN(soc_TXQ_BACKPRESSURE_IMP_CTLr_field_set
                (unit, &reg_value, EN_TXQ5_BP_IMPf, &temp));
            break;
        case 6:         
            SOC_IF_ERROR_RETURN(soc_TXQ_BACKPRESSURE_IMP_CTLr_field_set
                (unit, &reg_value, EN_TXQ6_BP_IMPf, &temp));
            break;
        case 7:
            SOC_IF_ERROR_RETURN(soc_TXQ_BACKPRESSURE_IMP_CTLr_field_set
                (unit, &reg_value, EN_TXQ7_BP_IMPf, &temp));
            break;
        default:
            return SOC_E_PARAM;
    }

    switch (port) {
        case 24:
            SOC_IF_ERROR_RETURN(REG_WRITE_TXQ_BACKPRESSURE_IMP_CTLr
                (unit, &reg_value));
            break;
        case 25:
            SOC_IF_ERROR_RETURN(REG_WRITE_TXQ_BACKPRESSURE_GE0_CTLr
                (unit, &reg_value));
            break;
        case 26:
            SOC_IF_ERROR_RETURN(REG_WRITE_TXQ_BACKPRESSURE_GE1_CTLr
                (unit, &reg_value));
            break;
        case 27:
            SOC_IF_ERROR_RETURN(REG_WRITE_TXQ_BACKPRESSURE_GE2_CTLr
                (unit, &reg_value));
            break;
        case 28:
            SOC_IF_ERROR_RETURN(REG_WRITE_TXQ_BACKPRESSURE_GE3_CTLr
                (unit, &reg_value));
            break;
    }

    return SOC_E_NONE;
}

/*
 *  Function : drv_tbx_queue_port_txq_pause_get
 *
 *  Purpose :
 *      Get the capability of pausing traffic scheduling on the specific queue
 *      for IMP and giga ports.
 *
 *  Parameters :
 *      unit     :  RoboSwitch unit number.
 *      port     :  port number.
 *      queue_n  :  queue number.
 *      enable   :  (OUT) The state of the capability of pausing traffic scheduling on 
 *                      selected TXQ-Qn (n = 0 ~ 7) for IMP and giga ports.
 *
 *  Return :
 *      SOC_E_XXX.
 *
 *  Note :
 */
int 
drv_tbx_queue_port_txq_pause_get(int unit, uint32 port, 
    uint8 queue_n, uint8 *enable)
{
    uint32  reg_value;
    uint32  temp;

    /* check port is IMP port or GE ports */
    if ((!IS_GE_PORT(unit, port)) && (!IS_CPU_PORT(unit, port))) {
        return SOC_E_PARAM;
    }

    switch (port) {
        case 24:
            SOC_IF_ERROR_RETURN(REG_READ_TXQ_BACKPRESSURE_IMP_CTLr
                (unit, &reg_value));
            break;
        case 25:
            SOC_IF_ERROR_RETURN(REG_READ_TXQ_BACKPRESSURE_GE0_CTLr
                (unit, &reg_value));
            break;
        case 26:
            SOC_IF_ERROR_RETURN(REG_READ_TXQ_BACKPRESSURE_GE1_CTLr
                (unit, &reg_value));
            break;
        case 27:
            SOC_IF_ERROR_RETURN(REG_READ_TXQ_BACKPRESSURE_GE2_CTLr
                (unit, &reg_value));
            break;
        case 28:
            SOC_IF_ERROR_RETURN(REG_READ_TXQ_BACKPRESSURE_GE3_CTLr
                (unit, &reg_value));
            break;
        default :
            return SOC_E_PARAM;
    }

    /* 
      * TXQ_BACKPRESSURE_IMP_CTLr/TXQ_BACKPRESSURE_GE0_CTLr
      * TXQ_BACKPRESSURE_GE1_CTLr
      * TXQ_BACKPRESSURE_GE2_CTLr/TXQ_BACKPRESSURE_GE3_CTLr
      * have the same fields format (one bits for each field, TXQ0 ~ 7).
      * Use TXQ_BACKPRESSURE_IMP_CTLr to get each register related field's value.
      */
    switch ((queue_n % 8)) {
        case 0:
            SOC_IF_ERROR_RETURN(soc_TXQ_BACKPRESSURE_IMP_CTLr_field_get
                (unit, &reg_value, EN_TXQ0_BP_IMPf, &temp));
            break;
        case 1:         
            SOC_IF_ERROR_RETURN(soc_TXQ_BACKPRESSURE_IMP_CTLr_field_get
                (unit, &reg_value, EN_TXQ1_BP_IMPf, &temp));
            break;
        case 2:         
            SOC_IF_ERROR_RETURN(soc_TXQ_BACKPRESSURE_IMP_CTLr_field_get
                (unit, &reg_value, EN_TXQ2_BP_IMPf, &temp));
            break;
        case 3:
            SOC_IF_ERROR_RETURN(soc_TXQ_BACKPRESSURE_IMP_CTLr_field_get
                (unit, &reg_value, EN_TXQ3_BP_IMPf, &temp));
            break;
        case 4:
            SOC_IF_ERROR_RETURN(soc_TXQ_BACKPRESSURE_IMP_CTLr_field_get
                (unit, &reg_value, EN_TXQ4_BP_IMPf, &temp));
            break;
        case 5:
            SOC_IF_ERROR_RETURN(soc_TXQ_BACKPRESSURE_IMP_CTLr_field_get
                (unit, &reg_value, EN_TXQ5_BP_IMPf, &temp));
            break;
        case 6:         
            SOC_IF_ERROR_RETURN(soc_TXQ_BACKPRESSURE_IMP_CTLr_field_get
                (unit, &reg_value, EN_TXQ6_BP_IMPf, &temp));
            break;
        case 7:
            SOC_IF_ERROR_RETURN(soc_TXQ_BACKPRESSURE_IMP_CTLr_field_get
                (unit, &reg_value, EN_TXQ7_BP_IMPf, &temp));
            break;
        default:
            return SOC_E_PARAM;
    }

    if (temp) {
        *enable = TRUE;
    } else {
        *enable = FALSE;
    }

    LOG_VERBOSE(BSL_LS_SOC_COMMON, \
                (BSL_META_U(unit, \
                            "drv_tbx_queue_port_txq_pause_get: \
                            unit %d, port = %d, queue = %d, state = %d\n"),
                 unit, port, queue_n, *enable));

    return SOC_E_NONE;
}

/*
 *  Function : drv_tbx_queue_qos_control_set
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
drv_tbx_queue_qos_control_set(int unit, uint32 port, uint32 type, uint32 state)
{
    uint32  reg_value, temp;
    uint32  found = 0;
    _tbx_cosq_rx_reason_info_t  *reason_info = NULL;
    _tbx_cosq_rx_reason_info_t  *reason_info_prev = NULL;

    LOG_VERBOSE(BSL_LS_SOC_COMMON, \
                (BSL_META_U(unit, \
                            "drv_tbx_queue_qos_control_set: \
                            unit %d, port = %d, type = 0x%x, state = 0x%x\n"), 
                 unit, port, type, state));

    switch (type) {
        case DRV_QOS_CTL_QOS_EN:
            SOC_IF_ERROR_RETURN(REG_READ_QOS_CTLr
                (unit, &reg_value));

            temp = ((state) ? 1 : 0);
            SOC_IF_ERROR_RETURN(soc_QOS_CTLr_field_set
                (unit, &reg_value, QOS_ENf, &temp));
            SOC_IF_ERROR_RETURN(REG_WRITE_QOS_CTLr
                (unit, &reg_value));
            break;
        case DRV_QOS_CTL_USE_TC:
            SOC_IF_ERROR_RETURN(REG_READ_QOS_CTLr
                (unit, &reg_value));
            /* This type value definition is :
             *  1: Use generic TC based COS mapping for copying packets to CPU.
             *  0: Use Reason basd COS mapping  for copying packets to CPU.
             */
            temp = ((state) ? 1 : 0);
            SOC_IF_ERROR_RETURN(soc_QOS_CTLr_field_set
                (unit, &reg_value, USE_TCf, &temp));
            SOC_IF_ERROR_RETURN(REG_WRITE_QOS_CTLr
                (unit, &reg_value));

            /* Update sw shadow */
            found = 0;
            reason_info = _tbx_rx_reason_info;
            while (reason_info) {
                if ((reason_info->unit == unit) && (reason_info->valid)) {
                    /* Found sw shadow */
                    reason_info->tc2cosq_en= state;
                    break;
                }
                reason_info = reason_info->next;
            }
            break;
        case DRV_QOS_CTL_WDRR_GRANULARTTY:
            SOC_IF_ERROR_RETURN(REG_READ_QOS_CTLr
                (unit, &reg_value));
            /* This type value definition is :
             *  1: number of packet.
             *  0: number of 64-bytes (64-byte: TBD).
             */
            temp = ((state) ? 1 : 0);
            SOC_IF_ERROR_RETURN(soc_QOS_CTLr_field_set
                (unit, &reg_value, WDRR_GRANULARITYf, &temp));
            SOC_IF_ERROR_RETURN(REG_WRITE_QOS_CTLr
                (unit, &reg_value));
            break;
        case DRV_QOS_CTL_DP_VALUE_DLF:
            SOC_IF_ERROR_RETURN(REG_READ_DP_CTRLr
                (unit, &reg_value));
            /* This type value definition is :
             *  DP_CTRL[3:2] value of the unknown unicast/multicast packet (0~ 3)
             */
            if (!TBX_DEFAULT_DP_VALID((int)state)) {
                return SOC_E_PARAM;
            }
            temp = state;
            SOC_IF_ERROR_RETURN(soc_DP_CTRLr_field_set
                (unit, &reg_value, DLF_DPf, &temp));
            SOC_IF_ERROR_RETURN(REG_WRITE_DP_CTRLr
                (unit, &reg_value));
            break;
        case DRV_QOS_CTL_DP_CHANGE_DLF:
            SOC_IF_ERROR_RETURN(REG_READ_DP_CTRLr
                (unit, &reg_value));
            /* This type value definition is :
             *  1: DP=DLF_DP (DP_CTRL[3:2]) if the packet is a DLF packet.
             *  0: DP is depended on the setting of default port DP.
             */
            temp = ((state) ? 1 : 0);
            SOC_IF_ERROR_RETURN(soc_DP_CTRLr_field_set
                (unit, &reg_value, SET_DLF_DPf, &temp));
            SOC_IF_ERROR_RETURN(REG_WRITE_DP_CTRLr
                (unit, &reg_value));
            break;
        case DRV_QOS_CTL_DP_CHANGE_XOFF:
            SOC_IF_ERROR_RETURN(REG_READ_DP_CTRLr
                (unit, &reg_value));
            /* This type value definition is :
             *  1: DP=DP0, if the port is flow-controllable port.
             *  0: DP is depended on the setting of DLF DP or the default port DP.
             */
            temp = ((state) ? 1 : 0);
            SOC_IF_ERROR_RETURN(soc_DP_CTRLr_field_set
                (unit, &reg_value, SET_XOFF_DPf, &temp));
            SOC_IF_ERROR_RETURN(REG_WRITE_DP_CTRLr
                (unit, &reg_value));
            break;
        case DRV_QOS_CTL_SW_SHADOW:
            if (state) {
                /* Prevent redundant allocation of sw shadow */
                found = 0;
                reason_info = _tbx_rx_reason_info;
                while (reason_info) {
                    if (reason_info->unit == unit) {
                        /* Found */
                        found = 1;
                        break;
                    }
                    reason_info = reason_info->next;
                }

                if (found) {
                    /* Update the information from register */
                    temp = _drv_tbx_rx_reason_info_update(unit, reason_info);
                    if (temp) {
                    /* update failed, the rx reason info still needs to be get from register */
                        reason_info->valid = 0;
                    } else {
                    /* update successed, the rx reason info can be retrieve from sw table */
                        reason_info->valid = 1;
                    }
                } else {
                    /* Allocate the sw shadow of rx reason cosq mapping information */
                    if ((reason_info = 
                        sal_alloc(sizeof(_tbx_cosq_rx_reason_info_t), 
                            "reason_info_list")) == NULL) {
                        /* Remove all created vm entries */
                        return SOC_E_MEMORY;
                    }

                    /* Update the information from register */
                    reason_info->unit = unit;
                    temp = _drv_tbx_rx_reason_info_update(unit, reason_info);
                    if (temp) {
                    /* update failed, the rx reason info still needs to be get from register */
                        reason_info->valid = 0;
                    } else {
                    /* update successed, the rx reason info can be retrieve from sw table */
                        reason_info->valid = 1;
                    }
    
                    reason_info->next = _tbx_rx_reason_info;
                    _tbx_rx_reason_info = reason_info;
                }
            } else {
                /* free the sw shadow */
                reason_info = _tbx_rx_reason_info;
                while (reason_info) {
                    if (reason_info->unit == unit) {
                        /* Found */
                        /* Remove SW copy */
                        if (reason_info_prev) {
                            reason_info_prev->next = reason_info->next;
                        } else {
                            _tbx_rx_reason_info = reason_info->next;
                        }
                        sal_free(reason_info);
                        return SOC_E_NONE;
                    }
                    reason_info_prev = reason_info;
                    reason_info = reason_info->next;
                }
            }
            break;
        default:
            return SOC_E_UNAVAIL;
    }

    return SOC_E_NONE;
}

/*
 *  Function : drv_tbx_queue_qos_control_get
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
drv_tbx_queue_qos_control_get(int unit, uint32 port, uint32 type, uint32 *state)
{
    uint32  reg_value, temp;
    uint32  found;
    _tbx_cosq_rx_reason_info_t  *reason_info = NULL;

    switch (type) {
        case DRV_QOS_CTL_QOS_EN:
            SOC_IF_ERROR_RETURN(REG_READ_QOS_CTLr
                (unit, &reg_value));
            SOC_IF_ERROR_RETURN(soc_QOS_CTLr_field_get
                (unit, &reg_value, QOS_ENf, &temp));
            *state = ((temp) ? TRUE : FALSE);
            break;
        case DRV_QOS_CTL_USE_TC:
            found = 0;
            reason_info = _tbx_rx_reason_info;
            while (reason_info) {
                if ((reason_info->unit == unit) && (reason_info->valid)) {
                    /* Found sw shadow */
                    found = 1;
                    break;
                }
                reason_info = reason_info->next;
            }

            if (found) {
                /* Get from sw shadow */
                *state = ((reason_info->tc2cosq_en) ? TRUE : FALSE);
            } else {
                /* Get from register */
                SOC_IF_ERROR_RETURN(REG_READ_QOS_CTLr
                    (unit, &reg_value));
                /* This type value definition is :
                 *  1: Use generic TC based COS mapping for copying packets to CPU.
                 *  0: Use Reason basd COS mapping  for copying packets to CPU.
                 */
                SOC_IF_ERROR_RETURN(soc_QOS_CTLr_field_get
                    (unit, &reg_value, USE_TCf, &temp));
                *state = ((temp) ? TRUE : FALSE);
            }
            break;
        case DRV_QOS_CTL_WDRR_GRANULARTTY:
            SOC_IF_ERROR_RETURN(REG_READ_QOS_CTLr
                (unit, &reg_value));
            /* This type value definition is :
             *  1: number of packet.
             *  0: number of 64-bytes (64-byte: TBD).
             */
            SOC_IF_ERROR_RETURN(soc_QOS_CTLr_field_get
                (unit, &reg_value, WDRR_GRANULARITYf, &temp));
            *state = ((temp) ? TRUE : FALSE);
            break;
        case DRV_QOS_CTL_DP_VALUE_DLF:
            SOC_IF_ERROR_RETURN(REG_READ_DP_CTRLr
                (unit, &reg_value));
            /* This type value definition is :
             *  DP_CTRL[3:2] value of the unknown unicast/multicast packet (0~ 3)
             */
            SOC_IF_ERROR_RETURN(soc_DP_CTRLr_field_get
                (unit, &reg_value, DLF_DPf, &temp));
            *state = temp;
            break;
        case DRV_QOS_CTL_DP_CHANGE_DLF:
            SOC_IF_ERROR_RETURN(REG_READ_DP_CTRLr
                (unit, &reg_value));
            /* This type value definition is :
             *  1: DP=DLF_DP (DP_CTRL[3:2]) if the packet is a DLF packet.
             *  0: DP is depended on the setting of default port DP.
             */
            SOC_IF_ERROR_RETURN(soc_DP_CTRLr_field_get
                (unit, &reg_value, SET_DLF_DPf, &temp));
            *state = ((temp) ? TRUE : FALSE);
            break;
        case DRV_QOS_CTL_DP_CHANGE_XOFF:
            SOC_IF_ERROR_RETURN(REG_READ_DP_CTRLr
                (unit, &reg_value));
            /* This type value definition is :
             *  1: DP=DP0, if the port is flow-controllable port.
             *  0: DP is depended on the setting of DLF DP or the default port DP.
             */
            SOC_IF_ERROR_RETURN(soc_DP_CTRLr_field_get
                (unit, &reg_value, SET_XOFF_DPf, &temp));
            *state = ((temp) ? TRUE : FALSE);
            break;
        case DRV_QOS_CTL_SW_SHADOW:
            found = 0;
            reason_info = _tbx_rx_reason_info;
            while (reason_info) {
                if ((reason_info->unit == unit) && (reason_info->valid)) {
                    /* Found sw shadow */
                    *state = 1;
                    break;
                }
                reason_info = reason_info->next;
            }
            break;
        default:
            return SOC_E_INTERNAL;
    }

    LOG_VERBOSE(BSL_LS_SOC_COMMON, \
                (BSL_META_U(unit, \
                            "drv_tbx_queue_qos_control_get: \
                            unit %d, port = %d, type = 0x%x, state = 0x%x\n"), 
                 unit, port, type, *state));    

    return SOC_E_NONE;
}
