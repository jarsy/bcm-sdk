/*
 * $Id: cosq.c,v 1.22 Broadcom SDK $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:     cosq.c
 * Purpose:
 *
 */

#include <bcm/types.h>
#include <bcm/cosq.h>
#include <bcm_int/tk371x_dispatch.h>
#include <bcm_int/ea/tk371x/cosq.h>
#include <bcm/error.h>
#include <soc/drv.h>
#include <soc/ea/tk371x/onu.h>
#include <soc/ea/tk371x/field.h>
#include <soc/ea/tk371x/counter.h>
#include <soc/ea/tk371x/misc.h>
#include <soc/ea/tk371x/tm.h>

static bcm_tk371x_cosq_group_t 
cosq_group[BCM_TK371X_COSQ_QUEUE_GROUP_MAX];

static uint64 
cosq_stat_local[SOC_EA_MAX_NUM_PORTS]\
               [BCM_TK371X_COSQ_QUEUE_MAX]\
               [BCM_TK371X_COSQ_STAT_MAX];

static bcm_tk371x_cosq_stat_map_t
cosq_statid_map[BCM_TK371X_COSQ_STAT_MAX] = {
    {bcmCosqGportOutPkts,          socEaStatMacFramesTxOk},  /* OamAttrMacFramesTxOk */
    {bcmCosqGportOutBytes,         socEaStatMacOctetsTxOk},  /* OamAttrMacOctetsTxOk */
    {bcmCosqGportReceivedBytes,    socEaStatMacOctetsRxOkId},  /* OamAttrMacOctetsRxOk */
    {bcmCosqGportReceivedPkts,     socEaStatMacFramesRxOk},  /* OamAttrMacFramesRxOk */
    {bcmCosqGportDroppedBytes,     socEaStatRxBytesDropped},  /* OamExtAttrRxBytesDropped */
    {bcmCosqGportDroppedPkts,      socEaStatRxFramesDropped},  /* OamExtAttrRxFramesDropped */
    {bcmCosqGportDelayedBytes,     socEaStatRxBytesDelayed},  /* OamExtAttrRxBytesDelayed */
    {bcmCosqGportDelayedHundredUs, socEaStatRxDelay}   /* OamExtAttrRxDelay */
};

static _soc_ea_tk_queue_config_info_t def_queue_cfg;

static int tk371x_cosq_detached[SOC_MAX_NUM_DEVICES] = {0};
/*
 * Function:
 *  _bcm_tk371x_cosq_valid
 * Purpose:
 *  check the params and get queue configuration.
 * Parameters:
 *  unit - (IN) Ethernet Access unit number #.
 *  port - (IN) GPORT ID for a queue group.
 *  cosq - (IN) COS Queue
 *  phy_llid - (IN) link offset of PON
 *  phy_llid - (OUT) 
 *  queue_cfg - (IN) configuration for queues of PON
 *  queue_cfg - (OUT) 
 * Returns:
 *  BCM_E_XXX
 */
static int
_bcm_tk371x_cosq_valid(
        int unit,
        bcm_gport_t port,
	    bcm_cos_queue_t cosq,
	    int *phy_llid,
	    int *order_llid,
	    _soc_ea_tk_queue_config_info_t *queue_cfg)
{
    int rv; 
    int real_llid = BCM_TK371X_COSQ_LLID_INVALID; 
    int port_link;
    int link, link_order = 0;
    uint32 group_id;
    
    if (!BCM_GPORT_IS_UCAST_QUEUE_GROUP(port)) {
        return BCM_E_PARAM;       
    }    
    group_id = BCM_GPORT_UCAST_QUEUE_GROUP_QID_GET(port);
    
    if (group_id >= BCM_TK371X_COSQ_QUEUE_GROUP_MAX) {
        return BCM_E_BADID;    
    }
    
    if (cosq_group[group_id].numq == 0) {
        return BCM_E_BADID;  
    }
    
    port_link = cosq_group[group_id].port_link;
 
    /* coverity[result_independent_of_operands] */
    if (!SOC_PORT_VALID(unit, port_link)) {
        return BCM_E_PORT;   
    }
    
    if (IS_PON_PORT(unit, port_link)) {
        return BCM_E_PORT;
    }
    
    rv = _soc_ea_queue_configuration_get(unit, queue_cfg);
    if (rv != OK) {
        return BCM_E_BUSY;       
    }    
    
    if (IS_LLID_PORT(unit, port_link)) {
        real_llid = port_link - SOC_PORT_MIN(unit, llid);
        
        PBMP_LLID_ITER(unit, link) {            
            if (port_link == link) {
                break;
            }
            link_order++;
        }
        if (link_order >= queue_cfg->cntOfLink) {
            return BCM_E_PARAM;       
        } else if (cosq >= queue_cfg->linkInfo[link_order].cntOfUpQ) {
            return BCM_E_PARAM; 
        }  
    /* coverity[overrun-local] */    
    } else if ((port_link > MAX_CNT_OF_PORT) || (cosq >= queue_cfg->portInfo[port_link - 1].cntOfDnQ)) {
        return BCM_E_PARAM;
    }
    
    * phy_llid = real_llid;
    * order_llid = link_order;
    return BCM_E_NONE;    
}


/*
 * Functions:
 *  _bcm_tk371x_cosq_state_sync
 * Purpose:
 *  sync the stat result from chip.
 * Parameters:
 *  unit - (IN) Ethernet Access unit number #.
 * Returns:
 *  BCM_E_XXX
 */
int 
_bcm_tk371x_cosq_stat_sync(
        int unit)
{
    int   port_llid;
    int   port;
    int   link_off, link_order;
    int   queue;
    uint64 value;
    uint8  map_idx;
    int    rv;
    _soc_ea_tk_queue_config_info_t queue_cfg; 
    
    if (tk371x_cosq_detached[unit]) {
        return BCM_E_INIT;
    }
    
    rv = _soc_ea_queue_configuration_get(unit, &queue_cfg);
    if (rv != OK) {
        return BCM_E_BUSY;       
    } 
    
    link_order = 0;
    PBMP_ALL_ITER(unit, port_llid) {
        if (IS_PON_PORT(unit, port_llid)) {
            continue;
        }     
        if (IS_E_PORT(unit, port_llid)) {
            port = port_llid ;
            link_off = 0;
        } else {
            link_off = port_llid - SOC_PORT_MIN(unit, llid);          
            link_order++;
            port = 0;
        }
        
        for (map_idx = 0; map_idx < BCM_TK371X_COSQ_STAT_MAX; map_idx++) {
            for (queue = 0; queue < BCM_TK371X_COSQ_QUEUE_MAX; queue++) {
                
                if (port > 0) {
                    if (queue >= queue_cfg.portInfo[port -1].cntOfDnQ) {
                        break;   
                    }
                } else {
                    if (queue >= queue_cfg.linkInfo[link_order].cntOfUpQ) {
                        break;   
                    } 
                }
                                   
                rv = _soc_ea_queue_stats_get(unit, link_off, port, queue, 
                                             cosq_statid_map[map_idx].tk_id, 
                                             &value);
                if (rv != OK) {
                    sal_memset(cosq_stat_local, 0, sizeof(cosq_stat_local));
                    return BCM_E_BUSY;
                }
                cosq_stat_local[port_llid - 1][queue][map_idx] = value;
            }
        }
    }
    return BCM_E_NONE;
}


int
_bcm_tk371x_cosq_gport_get(
	    int unit,
	    bcm_port_t port,
	    int cosq,
	    bcm_gport_t *gport)
{
    uint32 group_id;
    int port_link;
      
    port_link = port;
    
    if (tk371x_cosq_detached[unit]) {
        return BCM_E_INIT;
    }
    /* coverity[unsigned_compare] */
    /* coverity[result_independent_of_operands] */
    if (!SOC_PORT_VALID(unit, port_link)) {
        return BCM_E_PORT;   
    }
    
    if (IS_PON_PORT(unit, port_link)) {
        return BCM_E_PORT;
    }
    /* coverity[assignment] */
    group_id =  port_link - 1;
    /* coverity[overrun-local] */
    if (cosq_group[group_id].numq == 0) {
        return BCM_E_BADID;
    }
    if (cosq >= cosq_group[group_id].numq) {
        return BCM_E_PARAM;
    }       

    BCM_GPORT_UCAST_QUEUE_GROUP_SET(*gport, group_id);
    
	return BCM_E_NONE;
}

/*
 * Function:
 *  bcm_tk371x_cosq_config_get
 * Purpose:
 *  get the number of Class of Service Queue(COSQs)
 * Parameters:
 *  unit - ethernet device number #.
 *  numq - Number of Class of Service Queues
 * Returns:
 *  BCM_E_XXX
 */
int
bcm_tk371x_cosq_config_get(
	    int unit,
	    int *numq)
{
    int rv;
    
    if (tk371x_cosq_detached[unit]) {
        return BCM_E_INIT;
    }
    rv= _soc_ea_chip_info_sync(unit, socEaChipInfoOnuInfo,
        SOC_EA_SYNC_FLAG_NORMAL);
    if (rv != OK) {
        return BCM_E_FAIL;  
    }
    *numq = SOC_EA_PRIVATE(unit)->onu_info.up_queue_count;                  
	return BCM_E_NONE;
}

int
bcm_tk371x_cosq_config_set(
	    int unit,
	    int numq)
{
    return BCM_E_UNAVAIL;
}



/*
 * Function:
 *  bcm_tk371x_cosq_control_get
 * Purpose:
 *  get the control status of one queue.
 * Parameters:
 *  unit - (IN) Ethernet Access unit number #.
 *  port - (IN) GPORT ID for a queue group.
 *  cosq - (IN) COS Queue
 *  type - (IN) type of operation
 *  arg  - (IN) Argument whose meaning is dependent on type
 *  arg  - (OUT) Argument whose meaning is dependent on type
 * Returns:
 *  BCM_E_XXX
 */
int
bcm_tk371x_cosq_control_get(
	    int unit,
	    bcm_gport_t port,
	    bcm_cos_queue_t cosq,
	    bcm_cosq_control_t type,
	    int *arg)
{
    uint32 port_llid;
    int group_id;
    uint8  real_q;
    int real_llid = BCM_TK371X_COSQ_LLID_INVALID;
    int order_llid;
    _soc_ea_tk_queue_config_info_t queue_cfg;    
    
    if (tk371x_cosq_detached[unit]) {
        return BCM_E_INIT;
    }
    BCM_IF_ERROR_RETURN(
        _bcm_tk371x_cosq_valid(unit, port, cosq, &real_llid, 
                               &order_llid, &queue_cfg));
    
    group_id = BCM_GPORT_UCAST_QUEUE_GROUP_QID_GET(port);
    if (group_id < 0) {
        return BCM_E_PARAM;
    }
    port_llid = cosq_group[group_id].port_link;
    real_q   =  cosq;
      
    switch (type) {
        case bcmCosqControlFabricConnectMinUtilization:
        case bcmCosqControlFabricConnectMaxTime: 
        case bcmCosqControlBandwidthBurstMax:    
        case bcmCosqControlBandwidthBurstMin:    
        case bcmCosqControlFabricConnectMax: 
            return BCM_E_UNAVAIL;
            break; 
              
        case bcmCosqControlPortTxqBackpressure:
            if ((real_llid == 0) && (real_q == 0)) {
                * arg =  TRUE;
            } else {
                * arg = FALSE;     
            }    
            break;  
            
        case bcmCosqControlCopyPktToCpuUseTC:    
        case bcmCosqControlWdrrGranularity:      
        case bcmCosqControlDpValueDlf:          
        case bcmCosqControlDpChangeDlf:         
        case bcmCosqControlDpChangeXoff: 
        case bcmCosqControlEavClassAWindow: 
        case bcmCosqControlEgressRateBurstAccumulateControl:
        case bcmCosqControlEgressRateType:
            return BCM_E_UNAVAIL;
            break;
            
        case bcmCosqControlSchedulable: 
            * arg = TRUE;
            break;
            
        case bcmCosqControlEEETxQCongestionThreshold:
            if (real_llid != BCM_TK371X_COSQ_LLID_INVALID) {
                * arg = 
                    queue_cfg.linkInfo[order_llid].sizeOfUpQ[real_q] * 1000;   
            } else {
                * arg = 
                    queue_cfg.portInfo[port_llid - 1].sizeOfDnQ[real_q] * 1000;
            }
            break;
            
        case bcmCosqControlPacketLengthAdjust:
        case bcmCosqControlCongestionManagedQueue:
        case bcmCosqControlCongestionFeedbackWeight: 
        case bcmCosqControlCongestionSetPoint:  
        case bcmCosqControlCongestionSampleBytesMin:
        case bcmCosqControlCongestionSampleBytesMax: 
        case bcmCosqControlFabricPortIngressScheduler:
        case bcmCosqControlDropLimitAlpha:      
        case bcmCosqControlDropLimitBytes:      
        case bcmCosqControlResumeLimitBytes:    
        case bcmCosqControlYellowDropLimitBytes: 
        case bcmCosqControlRedDropLimitBytes:   
        case bcmCosqControlQselOffset: 
            return BCM_E_UNAVAIL;  
            break;  
                  
        case bcmCosqControlEgressFlowControlThreshold0:
            if (real_llid != BCM_TK371X_COSQ_LLID_INVALID) {
                * arg = 
                    queue_cfg.linkInfo[order_llid].sizeOfUpQ[real_q] * 1000; 
            } else {
                return BCM_E_UNAVAIL;    
            }
            break;
            
        case bcmCosqControlEgressFlowControlThreshold1: 
        case bcmCosqControlEgressFlowControlThreshold2: 
        case bcmCosqControlEgressFlowControlThreshold3:             
        case bcmCosqControlFabricPortScheduler:  
        case bcmCosqControlClassMap:
            return BCM_E_UNAVAIL;
            break;
            
        case bcmCosqControlSchedulerAdoptAllPriority:
            * arg = TRUE;
            break;
            
        case bcmCosqControlEgressPool:
        case bcmCosqControlEgressPoolLimitBytes: 
        case bcmCosqControlEgressPoolYellowLimitBytes:
        case bcmCosqControlEgressPoolRedLimitBytes: 
        case bcmCosqControlEgressPoolLimitEnable:
            return BCM_E_UNAVAIL; 
            break; 
           
        default:
            return BCM_E_PARAM;
            break;    
    }
    
	return BCM_E_NONE;
}

int
bcm_tk371x_cosq_control_set(
	    int unit,
	    bcm_gport_t port,
	    bcm_cos_queue_t cosq,
	    bcm_cosq_control_t type,
	    int arg)
{
	return BCM_E_UNAVAIL;
}

/*
 * Function:
 *  bcm_tk371x_cosq_gport_get
 * Purpose:
 *  get features of specific gport.
 * Parameters:
 *  unit - (IN) Ethernet Access unit number #.
 *  gport - (IN) GPORT ID for a queue group.
 *  physical_port - (IN) physical port or link.
 *  physical_port - (OUT) 
 *  num_cos_levels - (IN) number of queues.
 *  num_cos_levels - (OUT) 
 *  flags  - (IN) configured flags for this gport.
 *  flags  - (OUT) 
 * Returns:
 *  BCM_E_XXX
 */
int
bcm_tk371x_cosq_gport_get(
	    int unit,
	    bcm_gport_t gport,
	    bcm_gport_t *physical_port,
	    int *num_cos_levels,
	    uint32 *flags)
{
    uint32 group_id;
    
    if (tk371x_cosq_detached[unit]) {
        return BCM_E_INIT;
    }
    if (BCM_GPORT_IS_UCAST_QUEUE_GROUP(gport)) {
        group_id = BCM_GPORT_UCAST_QUEUE_GROUP_QID_GET(gport); 
        if ((group_id < BCM_TK371X_COSQ_QUEUE_GROUP_MAX) && 
            (cosq_group[group_id].numq > 0)) {
            * num_cos_levels = cosq_group[group_id].numq;
            * flags = cosq_group[group_id].flags;
            BCM_GPORT_LOCAL_SET(* physical_port, cosq_group[group_id].port_link);
            return BCM_E_NONE;
        }
    }
	return BCM_E_PARAM;
}

/*
 * Function:
 *  bcm_tk371x_cosq_gport_add
 * Purpose:
 *  configure features for specific gport.
 * Parameters:
 *  unit - (IN) Ethernet Access unit number #.
 *  port - (IN) physical port or link.
 *  numq - (IN) number of queues.
 *  flags - (IN) 
 *  gport - (IN) GPORT ID for a queue group.
 *  gport - (OUT) 
 * Returns:
 *  BCM_E_XXX
 */
int
bcm_tk371x_cosq_gport_add(
	    int unit,
	    bcm_gport_t port,
	    int numq,
	    uint32 flags,
	    bcm_gport_t *gport)
{
    uint32 group_id;
    int port_link; 
    int link_order = 0;
	int link;
    int rv;
    int queue_idx = 0;
    _soc_ea_tk_queue_config_info_t queue_cfg; 
    
    if (tk371x_cosq_detached[unit]) {
        return BCM_E_INIT;
    }     
    
     if (((flags & BCM_COSQ_GPORT_UCAST_QUEUE_GROUP) 
         !=  BCM_COSQ_GPORT_UCAST_QUEUE_GROUP) &&  
         ((flags & BCM_COSQ_GPORT_MCAST_QUEUE_GROUP) 
         !=  BCM_COSQ_GPORT_MCAST_QUEUE_GROUP)) {
        return BCM_E_PARAM; 
    }
    
    port_link = BCM_GPORT_LOCAL_GET(port);
    /* coverity[unsigned_compare] */  
    /* coverity[negative_returns] */
    /* coverity[result_independent_of_operands] */
    if (!SOC_PORT_VALID(unit, port_link)) {
        return BCM_E_PORT;   
    }
    
    if (IS_PON_PORT(unit, port_link)) {
        return BCM_E_PORT;
    }
    
    if ((numq <= 0) || (numq > BCM_TK371X_COSQ_QUEUE_MAX)) {
        return BCM_E_PARAM;
    }
    
    rv = _soc_ea_queue_configuration_get(unit, &queue_cfg);
    if (rv != OK) {
        return BCM_E_BUSY;       
    }    
    
    if (IS_LLID_PORT(unit, port_link)) {
        PBMP_LLID_ITER(unit, link) {            
            if (port_link == link) {
                break;
            }
            link_order++;
        }
        if (link_order >= queue_cfg.cntOfLink) {
            return BCM_E_PARAM;       
        } else {
            if (numq > MAX_CNT_OF_UP_QUEUE) {
                numq = MAX_CNT_OF_UP_QUEUE;
            }
            if (numq > queue_cfg.linkInfo[link_order].cntOfUpQ) {

                for (queue_idx = queue_cfg.linkInfo[link_order].cntOfUpQ; 
                     queue_idx < numq; queue_idx++) {
                    queue_cfg.linkInfo[link_order].sizeOfUpQ[queue_idx] = 4;
                }
            }
            if (numq != queue_cfg.linkInfo[link_order].cntOfUpQ) {
                queue_cfg.linkInfo[link_order].cntOfUpQ = numq; 
                rv = _soc_ea_queue_configuration_set(unit, &queue_cfg);
                if (rv != OK) {
                    return BCM_E_FAIL;
                }  
            }
        } 
           
    } else {
        /* coverity[overrun-local] */
        if (numq > MAX_CNT_OF_UP_QUEUE) {
            numq = MAX_CNT_OF_UP_QUEUE;
        }
        if (port_link > MAX_CNT_OF_PORT) {
        	return BCM_E_PORT;
        }
        /* coverity[overrun-local] */
        if (numq > queue_cfg.portInfo[port_link - 1].cntOfDnQ) {
            for (queue_idx = queue_cfg.portInfo[port_link - 1].cntOfDnQ; 
                 queue_idx < numq; queue_idx++) {
                queue_cfg.portInfo[port_link - 1].sizeOfDnQ[queue_idx] = 4;
            }
        }
        if (numq != queue_cfg.portInfo[port_link - 1].cntOfDnQ) {
            queue_cfg.portInfo[port_link - 1].cntOfDnQ = numq;
            rv = _soc_ea_queue_configuration_set(unit, &queue_cfg);
            if (rv != OK) {
                return BCM_E_FAIL;
            }  
        }
    } 
    
    /* assign group index with local port */
    /* coverity[assignment] */
    group_id =  port_link - 1;
    
    /* coverity[overrun-local] */
    cosq_group[group_id].port_link = port_link;
    cosq_group[group_id].numq = numq;
    cosq_group[group_id].flags = flags;
   

    BCM_GPORT_UCAST_QUEUE_GROUP_SET(*gport, group_id);
    
	return BCM_E_NONE;
}

/*
 * Function:
 *  bcm_tk371x_cosq_gport_delete
 * Purpose:
 *  delete specific gport of queue group.
 * Parameters:
 *  unit - (IN) Ethernet Access unit number #.
 *  gport - (IN) GPORT ID for a queue group.
 * Returns:
 *  BCM_E_XXX
 */
int
bcm_tk371x_cosq_gport_delete(
	    int unit,
	    bcm_gport_t gport)
{
    int group_id;
    
    if (tk371x_cosq_detached[unit]) {
        return BCM_E_INIT;
    }
    group_id  = BCM_GPORT_UCAST_QUEUE_GROUP_QID_GET(gport);
    if ((group_id >= 0) && 
        (group_id < BCM_TK371X_COSQ_QUEUE_GROUP_MAX)) {
        if (cosq_group[group_id].numq == 0) {
           return BCM_E_NOT_FOUND; 
        }
        sal_memset(&cosq_group[group_id], 0, sizeof(bcm_tk371x_cosq_group_t));
        return BCM_E_NONE;      
    }
    
	return BCM_E_PARAM;
}


/*
 * Function:
 *      bcm_tk371x_cosq_gport_enable_get
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_tk371x_cosq_gport_enable_get(
	    int unit,
	    bcm_gport_t gport,
	    bcm_cos_queue_t cosq,
	    int *enable)
{
	return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_tk371x_cosq_gport_enable_set
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_tk371x_cosq_gport_enable_set(
	    int unit,
	    bcm_gport_t gport,
	    bcm_cos_queue_t cosq,
	    int enable)
{        
	return BCM_E_UNAVAIL;
}


/*
 * Function:
 *  bcm_tk371x_cosq_gport_sched_get
 * Purpose:
 *  get parameters of scheduler for specific queue.
 * Parameters:
 *  unit - (IN) Ethernet Access unit number #.
 *  gport - (IN) GPORT ID for a queue group.
 *  cosq - (IN) COS Queue.
 *  mode - (IN) scheduling mode for queue.
 *  mode - (OUT)
 *  weight - (IN) scheduling weight.
 *  weight - (OUT) 
 * Returns:
 *  BCM_E_XXX
 */
int
bcm_tk371x_cosq_gport_sched_get(
	    int unit,
	    bcm_gport_t gport,
	    bcm_cos_queue_t cosq,
	    int *mode,
	    int *weight)
{
    int real_llid = BCM_TK371X_COSQ_LLID_INVALID;
    int order_llid;
    _soc_ea_tk_queue_config_info_t queue_cfg;    
    
    if (tk371x_cosq_detached[unit]) {
        return BCM_E_INIT;
    }
    BCM_IF_ERROR_RETURN(
        _bcm_tk371x_cosq_valid(unit, gport, cosq, &real_llid, &order_llid,
                               &queue_cfg));
        
    * mode = BCM_COSQ_STRICT;
    * weight = 0;
    
	return BCM_E_NONE;
}

/*
 * Function:
 *  bcm_tk371x_cosq_gport_sched_set
 * Purpose:
 *  set parameters of scheduler for specific queue.
 * Parameters:
 *  unit - (IN) Ethernet Access unit number #.
 *  gport - (IN) GPORT ID for a queue group.
 *  cosq - (IN) COS Queue.
 *  mode - (IN) scheduling mode for queue.
 *  weight - (IN) scheduling weight.
 * Returns:
 *  BCM_E_XXX
 */
int
bcm_tk371x_cosq_gport_sched_set(
	    int unit,
	    bcm_gport_t gport,
	    bcm_cos_queue_t cosq,
	    int mode,
	    int weight)
{   
    int real_llid = BCM_TK371X_COSQ_LLID_INVALID;
    int order_llid;
    _soc_ea_tk_queue_config_info_t queue_cfg;
    
    if (tk371x_cosq_detached[unit]) {
        return BCM_E_INIT;
    }   
    if (mode != BCM_COSQ_STRICT) {
	    return BCM_E_UNAVAIL;
    }
    
    BCM_IF_ERROR_RETURN(
        _bcm_tk371x_cosq_valid(unit, gport, cosq, &real_llid, &order_llid,
                               &queue_cfg));
        
    return BCM_E_NONE;   
}

int
bcm_tk371x_cosq_gport_traverse(
	    int unit,
	    bcm_cosq_gport_traverse_cb cb,
	    void *user_data)
{

    bcm_gport_t port; 
    bcm_gport_t gport;  
    int i;
    int rv;
    
    if (tk371x_cosq_detached[unit]) {
        return BCM_E_INIT;
    }
    
    if (cb == NULL) {
        return BCM_E_PARAM;   
    }        
    for (i = 0; i < BCM_TK371X_COSQ_QUEUE_GROUP_MAX; i++) {
        if (cosq_group[i].numq != 0) {
            BCM_GPORT_LOCAL_SET(port, cosq_group[i].port_link);
            BCM_GPORT_UCAST_QUEUE_GROUP_SET(gport, i);
            rv = cb(unit, port, cosq_group[i].numq, 
                    BCM_COSQ_GPORT_UCAST_QUEUE_GROUP, gport, user_data);
            if (rv != BCM_E_NONE) {
                return rv;
            }
        }
    }
    return BCM_E_NONE;
}

/*
 * Function:
 *  bcm_tk371x_cosq_gport_size_get
 * Purpose:
 *  get size for specific queue.
 * Parameters:
 *  unit - (IN) Ethernet Access unit number #.
 *  gport - (IN) GPORT ID for a queue group.
 *  cosq - (IN) COS Queue.
 *  bytes_min - (IN) commited size.
 *  bytes_min - (OUT)
 *  bytes_max - (IN) maximum size.
 *  bytes_max - (OUT) 
 * Returns:
 *  BCM_E_XXX
 */
int
bcm_tk371x_cosq_gport_size_get(
        int unit,
	    bcm_gport_t gport,
	    bcm_cos_queue_t cosq,
	    uint32 *bytes_min,
	    uint32 *bytes_max)
{
	uint32 port_llid;
	int group_id;
	uint8  real_q;
    int real_llid = BCM_TK371X_COSQ_LLID_INVALID;
    int order_llid;
    _soc_ea_tk_queue_config_info_t queue_cfg;    
    
    if (tk371x_cosq_detached[unit]) {
        return BCM_E_INIT;
    }
    BCM_IF_ERROR_RETURN(
        _bcm_tk371x_cosq_valid(unit, gport, cosq, &real_llid, &order_llid,
                               &queue_cfg));
        
    group_id  = BCM_GPORT_UCAST_QUEUE_GROUP_QID_GET(gport);
    if (group_id < 0) {
        return BCM_E_PARAM;
    }
    port_llid = cosq_group[group_id].port_link;
    real_q    = cosq;
    
    if (real_llid != BCM_TK371X_COSQ_LLID_INVALID) {
        *bytes_min = *bytes_max = 
            queue_cfg.linkInfo[order_llid].sizeOfUpQ[real_q] * 4000;   
    } else {
        *bytes_min = *bytes_max = 
            queue_cfg.portInfo[port_llid - 1].sizeOfDnQ[real_q] * 4000;
    }
    
    return BCM_E_NONE;
}

/*
 * Function:
 *  bcm_tk371x_cosq_gport_size_set
 * Purpose:
 *  set size for specific queue.
 * Parameters:
 *  unit - (IN) Ethernet Access unit number #.
 *  gport - (IN) GPORT ID for a queue group.
 *  cosq - (IN) COS Queue.
 *  bytes_min - (IN) commited size.
 *  bytes_max - (IN) maximum size.
 * Returns:
 *  BCM_E_XXX
 */
int
bcm_tk371x_cosq_gport_size_set(
	    int unit,
	    bcm_gport_t gport,
	    bcm_cos_queue_t cosq,
	    uint32 bytes_min,
	    uint32 bytes_max)
{
    int rv;
    uint32 port_llid;
    int group_id;
    uint8  real_q;
    uint8  real_kb;
    int real_llid = BCM_TK371X_COSQ_LLID_INVALID;
    int order_llid;
    _soc_ea_tk_queue_config_info_t queue_cfg;    
    
    if (tk371x_cosq_detached[unit]) {
        return BCM_E_INIT;
    }
    if (bytes_min > bytes_max) {
        return BCM_E_PARAM;
    }
    
    BCM_IF_ERROR_RETURN(
        _bcm_tk371x_cosq_valid(unit, gport, cosq, &real_llid, &order_llid,
                               &queue_cfg));
        
    group_id  = BCM_GPORT_UCAST_QUEUE_GROUP_QID_GET(gport);
    if (group_id < 0) {
        return BCM_E_PARAM;
    }
    port_llid = cosq_group[group_id].port_link;
    real_q    = cosq;
    real_kb   = (bytes_max % 4000) ? 
                    (bytes_max / 4000 + 1) : (bytes_max / 4000);
    
    if (real_llid != BCM_TK371X_COSQ_LLID_INVALID) {
        /*set upward queue */
        queue_cfg.linkInfo[order_llid].sizeOfUpQ[real_q] = real_kb;                
    } else {
        /*set downward queue */
        queue_cfg.portInfo[port_llid - 1].sizeOfDnQ[real_q] = real_kb;
    }
    
    rv = _soc_ea_queue_configuration_set(unit, &queue_cfg);
    if (rv != OK) {
        return BCM_E_FAIL;
    }
    
 	return BCM_E_NONE;
}

/*
 * Function:
 *      bcm_tk371x_cosq_gport_stat_enable_get
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_tk371x_cosq_gport_stat_enable_get(
	    int unit,
	    bcm_gport_t gport,
	    int *enable)
{   
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_tk371x_cosq_gport_stat_enable_set
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_tk371x_cosq_gport_stat_enable_set(
	    int unit,
	    bcm_gport_t gport,
	    int enable)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Functions:
 *  bcm_tk371x_cosq_gport_stat_get
 * Purpose:
 *  get the stat result for one queue.
 * Parameters:
 *  unit - (IN) Ethernet Access unit number #.
 *  gport - (IN) GPORT ID for a queue group.
 *  cosq - (IN) COS Queue.
 *  stat - (IN) stat ID.
 *  value - (IN) stat result.
 *  value - (OUT)
 * Returns:
 *  BCM_E_XXX
 */
int
bcm_tk371x_cosq_gport_stat_get(
	    int unit,
	    bcm_gport_t gport,
	    bcm_cos_queue_t cosq,
	    bcm_cosq_gport_stats_t stat,
	    uint64 *value)
{
    uint32 port_llid;
    int group_id;
    int real_llid = BCM_TK371X_COSQ_LLID_INVALID;
    int order_llid;
    _soc_ea_tk_queue_config_info_t queue_cfg;  
    uint8 map_idx; 
    int link = 0;
    int port = 0; 
    int rv;
    
    if (tk371x_cosq_detached[unit]) {
        return BCM_E_INIT;
    }
    BCM_IF_ERROR_RETURN(
        _bcm_tk371x_cosq_valid(unit, gport, cosq, &real_llid, &order_llid,
                               &queue_cfg));
    
    group_id = BCM_GPORT_UCAST_QUEUE_GROUP_QID_GET(gport);
    if (group_id < 0) {
        return BCM_E_PARAM;
    }
    port_llid = cosq_group[group_id].port_link;           
    
    for (map_idx = 0; map_idx < BCM_TK371X_COSQ_STAT_MAX; map_idx++) {        
        if (cosq_statid_map[map_idx].standard_id == stat) {
            break;
        }
    }    
    if (map_idx >= BCM_TK371X_COSQ_STAT_MAX) {
        return BCM_E_UNAVAIL;   
    }
    
    if (real_llid == BCM_TK371X_COSQ_LLID_INVALID) {
        link = 0;
        port = port_llid;
    } else {
        link = real_llid;
        port = 0;
    }
    rv = _soc_ea_queue_stats_get(unit, link, port, cosq, 
                                 cosq_statid_map[map_idx].tk_id, 
                                 &cosq_stat_local[port_llid - 1][cosq][map_idx]);
    if (rv != OK) {
        return BCM_E_FAIL;
    }
    
    * value = cosq_stat_local[port_llid - 1][cosq][map_idx];  
    return BCM_E_NONE;
}

/*
 * Functions:
 *  bcm_tk371x_cosq_gport_stat_set
 * Purpose:
 *  clear the stat value for one ID.
 * Parameters:
 *  unit - (IN) Ethernet Access unit number #.
 *  gport - (IN) GPORT ID for a queue group.
 *  cosq - (IN) COS Queue.
 *  stat - (IN) stat ID.
 *  value - (IN) stat value.
 * Returns:
 *  BCM_E_XXX
 */
int
bcm_tk371x_cosq_gport_stat_set(
	    int unit,
	    bcm_gport_t gport,
	    bcm_cos_queue_t cosq,
	    bcm_cosq_gport_stats_t stat,
	    uint64 value)
{
    uint32 port_llid;
    int group_id;
    int real_llid = BCM_TK371X_COSQ_LLID_INVALID;
    int order_llid;
    _soc_ea_tk_queue_config_info_t queue_cfg;  
    uint8 map_idx; 
    int link;
    int rv; 
    
    if (tk371x_cosq_detached[unit]) {
        return BCM_E_INIT;
    }
    BCM_IF_ERROR_RETURN(
        _bcm_tk371x_cosq_valid(unit, gport, cosq, &real_llid, &order_llid,
                               &queue_cfg));
    
    group_id = BCM_GPORT_UCAST_QUEUE_GROUP_QID_GET(gport);
    if (group_id < 0) {
        return BCM_E_PARAM;
    }
    port_llid = cosq_group[group_id].port_link;           
    
    for (map_idx = 0; map_idx < BCM_TK371X_COSQ_STAT_MAX; map_idx++) {        
        if (cosq_statid_map[map_idx].standard_id == stat) {
            break;
        }
    }    
    if (map_idx >= BCM_TK371X_COSQ_STAT_MAX) {
        return BCM_E_UNAVAIL;   
    }
    
    if (real_llid == BCM_TK371X_COSQ_LLID_INVALID) {
        link = 0;
    } else {
        link = real_llid;
    }    
    rv = _soc_ea_clr_stats_set(unit, link);
    if (rv != OK) {
        return BCM_E_BUSY;
    }
    cosq_stat_local[port_llid - 1][cosq][map_idx] = value; 
        
	return BCM_E_NONE;
}



int 
bcm_tk371x_cosq_gport_report_threshold_get(
    int unit, 
    bcm_gport_t gport, 
    bcm_cos_queue_t cosq, 
    bcm_cosq_report_threshold_t *threshold)
{
    int rv;
    int qset_index;
    _soc_ea_tk_queue_config_info_t queue_cfg;
    _soc_ea_oam_ctc_dba_data_t dba;
    int real_llid = BCM_TK371X_COSQ_LLID_INVALID;
    int order_llid;
    
    if (tk371x_cosq_detached[unit]) {
        return BCM_E_INIT;
    }    
    
    BCM_IF_ERROR_RETURN(
        _bcm_tk371x_cosq_valid(unit, gport, cosq, &real_llid, &order_llid,
                               &queue_cfg));
    
    if (real_llid == BCM_TK371X_COSQ_LLID_INVALID) {
        return BCM_E_PORT;
    }
    
    rv = _soc_ea_ctc_dba_cfg_get(unit, real_llid, &dba);    
    if (rv != OK) {
        return BCM_E_FAIL;   
    }
    
    sal_memset(threshold, 0, sizeof(bcm_cosq_report_threshold_t));
    
    for (qset_index = 0; qset_index < dba.num; qset_index++) {
        * (&threshold->threshold0 + qset_index) = 
                dba.set[qset_index].threshold[cosq];
    }
    return BCM_E_NONE;
}

int 
bcm_tk371x_cosq_gport_report_threshold_set(
    int unit, 
    bcm_gport_t gport, 
    bcm_cos_queue_t cosq, 
    bcm_cosq_report_threshold_t *threshold)
{
    int rv;
    int qset_index;
    int q_index;
    _soc_ea_tk_queue_config_info_t queue_cfg;
    _soc_ea_oam_ctc_dba_data_t dba;
    int real_llid = BCM_TK371X_COSQ_LLID_INVALID;
    int order_llid;
        
    if (tk371x_cosq_detached[unit]) {
        return BCM_E_INIT;
    }
    BCM_IF_ERROR_RETURN(
        _bcm_tk371x_cosq_valid(unit, gport, cosq, &real_llid, &order_llid,
                               &queue_cfg));
    
    if (real_llid == BCM_TK371X_COSQ_LLID_INVALID) {
        return BCM_E_PORT;
    }
          
    dba.num = BCM_TK371X_COSQ_REPORT_SET_MAX;
    for (qset_index = 0; qset_index < dba.num; qset_index++) {        
        dba.set[qset_index].report = 0;
        for (q_index = 0; q_index < queue_cfg.linkInfo[order_llid].cntOfUpQ;
             q_index++) {
        dba.set[qset_index].report |= 1 << q_index;
        dba.set[qset_index].threshold[q_index] = 
                * (&threshold->threshold0 + qset_index);
        }
        
    }
    
    rv = _soc_ea_ctc_dba_cfg_set(unit, real_llid, &dba);    
    if (rv != OK) {
        return BCM_E_FAIL;   
    } 
    return BCM_E_NONE;
}



static int 
_bcm_tk371x_cosq_attach(int unit)
{
    int rv;
    int port_link;
    int link_order = 0;
    int group_id = 0;
    
    rv = _soc_ea_queue_configuration_get(unit, &def_queue_cfg);
    if (rv != OK) {
        return BCM_E_FAIL;       
    }
        
    PBMP_LLID_ITER(unit, port_link) {       
        group_id =  port_link - 1;
        if (def_queue_cfg.linkInfo[link_order].cntOfUpQ > 0) {
            cosq_group[group_id].port_link = port_link;
            cosq_group[group_id].numq = 
                def_queue_cfg.linkInfo[link_order].cntOfUpQ;
            cosq_group[group_id].flags = BCM_COSQ_GPORT_UCAST_QUEUE_GROUP; 
        }                                  
        link_order++;
    }
    PBMP_E_ITER(unit, port_link) {       
        group_id =  port_link - 1;
        if (def_queue_cfg.portInfo[group_id].cntOfDnQ > 0) {
            cosq_group[group_id].port_link = port_link;
            cosq_group[group_id].numq = 
                        def_queue_cfg.portInfo[group_id].cntOfDnQ;
            cosq_group[group_id].flags = BCM_COSQ_GPORT_UCAST_QUEUE_GROUP;   
        }                                
    } 
    
    tk371x_cosq_detached[unit] = 0;        
    return BCM_E_NONE;
}

/*
 * Function:
 *  bcm_tk371x_cosq_init
 * Purpose:
 *  initialize cosq module of Tk371x.
 * Parameters:
 *  unit - Ethernet Access number #.
 * Returns:
 *  BCM_E_XXX
 */
int
bcm_tk371x_cosq_init(int unit)
{
    sal_memset(cosq_group, 0, sizeof(cosq_group));
    sal_memset(cosq_stat_local, 0, sizeof(cosq_stat_local));
    return _bcm_tk371x_cosq_attach(unit);
}


int 
bcm_tk371x_cosq_detach(int unit)
{   
    int rv;
    rv = _soc_ea_queue_configuration_set(unit, &def_queue_cfg);
    if (rv == OK) {
        sal_memset(cosq_group, 0, sizeof(cosq_group));
        sal_memset(cosq_stat_local, 0, sizeof(cosq_stat_local));
        tk371x_cosq_detached[unit] = 1; 
        return BCM_E_NONE;
    }
    return BCM_E_FAIL;
}

