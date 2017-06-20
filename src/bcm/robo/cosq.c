/*
 * $Id: cosq.c,v 1.103 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * COS Queue Management
 * Purpose: API to set different cosq, priorities, and scheduler registers.
 */

#include <shared/bsl.h>

#include <bcm/error.h>
#include <bcm/types.h>
#include <bcm/cosq.h>
#include <bcm/eav.h>
#include <bcm/stack.h>

#include <shared/pbmp.h>
#include <shared/types.h>

#include <bcm_int/robo/cosq.h>
#include <bcm_int/robo/port.h>

#include <soc/types.h>
#include <soc/drv.h>
#include <soc/mem.h>
#include <soc/debug.h>

#if defined(BCM_POLAR_SUPPORT) || defined(BCM_NORTHSTAR_SUPPORT)
/*
 *      Q5 -------------------------> -------
 *      Q4 -------------------------> |SCH1  |
 *      Q3 --->  -------                               |      |
 *      Q2 --->  |SCH2 |---Q6-------> -------- 
 *      Q1 --->  |     |
 *      Q0 --->  ------- 
 *
 *
 *      PS1. SCH1 use LEVEL2_QOS_PRI_CTL register and SCH2 use LEVEL1_QOS_PRI_CTL
 *           There are 4 COS queues for SCH2. They are real COS0 ~ COSQ3.
 *           There are 3 COS queues for SCH1. They are COSQ4, COSQ5 and COSQ6. 
 *             - Q0 of SCH1 represents the COSQ6  (the output of SCH2)
 *             - Q1 of SCH1 represents the COSQ4
 *             - Q2 of SCH1 represents the COSQ5
 *      PS2. User need to use SCHEDULER gport to program SCH1
 *      PS3. The SCH2 COS queues don't support leaky bucket shaper 
 *
 */

/* Array to keep track of the number of COSQs for scheduler gport */
typedef struct num_port_cosq_s {
    uint8  numq;
    uint8  *member_queues;
} num_port_cosq_t;

STATIC num_port_cosq_t *_num_port_cosq[SOC_MAX_NUM_DEVICES] = {NULL};

STATIC int
_bcm_robo_cosq_gport_delete(int unit, bcm_port_t port)
{
    int weights[NUM_COS(unit)];
    uint32 weight_value;
    int i, cosq;
    bcm_pbmp_t pbmp;
    uint32 sch2_output_cosq, sch1_num_cosq, sch2_num_cosq;
    uint32 reg_value;

    BCM_IF_ERROR_RETURN(DRV_DEV_PROP_GET(unit, 
                         DRV_DEV_PROP_SCH1_NUM_COSQ, &sch1_num_cosq));
    BCM_IF_ERROR_RETURN(DRV_DEV_PROP_GET(unit, 
                         DRV_DEV_PROP_SCH2_NUM_COSQ, &sch2_num_cosq));
    BCM_IF_ERROR_RETURN(DRV_DEV_PROP_GET(unit, 
                         DRV_DEV_PROP_SCH2_OUTPUT_COSQ, &sch2_output_cosq));

    BCM_PBMP_PORT_SET(pbmp, port);

    /* Clear bandwidth of all Stage1 COSQs */
    for (i = 0; i < sch1_num_cosq; i++) {
        if (i == 0) {
            cosq = sch2_output_cosq;
        } else {
            cosq = i + (sch2_num_cosq - 1);
        }
        BCM_IF_ERROR_RETURN(DRV_RATE_SET
                (unit, pbmp, cosq,
                DRV_RATE_CONTROL_DIRECTION_EGRESSS_PER_QUEUE,
                0, 0, 0, 0));
    }

    /* Disable low queue (queue 0 ~3), queue 4 or 5 AVB shaping mode */
    BCM_IF_ERROR_RETURN
        (REG_READ_LOW_QUEUE_AVB_SHAPING_MODEr(unit, &reg_value));
    reg_value &= ~(1 << port);
    BCM_IF_ERROR_RETURN(REG_WRITE_LOW_QUEUE_AVB_SHAPING_MODEr(unit,&reg_value));

    BCM_IF_ERROR_RETURN
        (REG_READ_QUEUE4_AVB_SHAPING_MODEr(unit, &reg_value));
    reg_value &= ~(1 << port);
    BCM_IF_ERROR_RETURN(REG_WRITE_QUEUE4_AVB_SHAPING_MODEr(unit, &reg_value));

    BCM_IF_ERROR_RETURN
        (REG_READ_QUEUE5_AVB_SHAPING_MODEr(unit, &reg_value));
    reg_value &= ~(1 << port);
    BCM_IF_ERROR_RETURN(REG_WRITE_QUEUE5_AVB_SHAPING_MODEr(unit, &reg_value));

    /* Clear weights of all Stage1 COSQs */
    weights[0] = 1;
    weights[1] = 2;
    weights[2] = 4;
    
    for (i = 0; i < sch1_num_cosq; i++){
        weight_value = weights[i];
            if (i == 0) {
                cosq = sch2_output_cosq;
            } else {
                cosq = i + (sch2_num_cosq - 1);
            }                            
            BCM_IF_ERROR_RETURN(DRV_QUEUE_WRR_WEIGHT_SET
                        (unit, 0, pbmp, cosq, weight_value));
    }

    _num_port_cosq[unit][port].numq = 0;
    if (_num_port_cosq[unit][port].member_queues != NULL) {
        sal_free(_num_port_cosq[unit][port].member_queues);
        _num_port_cosq[unit][port].member_queues = NULL;
    }

    return BCM_E_NONE;
}
#endif /* BCM_POLAR_SUPPORT || BCM_NORTHSTAR_SUPPORT */

/*
 * Function:
 *      bcm_robo_cosq_detach
 * Purpose:
 *      Discard all COS schedule/mapping state.
 * Parameters:
 *      unit - RoboSwitch unit number.
 * Returns:
 *      BCM_E_XXX
 * Note:
 *      This API is designed mainly for Tucana Chip and not suitable
 *      for Robo Chip.
 */
int 
bcm_robo_cosq_detach(int unit)
{
#if defined(BCM_POLAR_SUPPORT) || defined(BCM_NORTHSTAR_SUPPORT)
    bcm_port_t port = 0;
#endif /* BCM_POLAR_SUPPORT || BCM_NORTHSTAR_SUPPORT */

    LOG_VERBOSE(BSL_LS_BCM_COMMON,
                (BSL_META_U(unit,
                            "BCM API : bcm_robo_cosq_detach()..\n")));

#if defined(BCM_POLAR_SUPPORT) || defined(BCM_NORTHSTAR_SUPPORT)
    if (SOC_IS_POLAR(unit) || SOC_IS_NORTHSTAR(unit)) {
        if (_num_port_cosq[unit]) {
    
            BCM_PBMP_ITER(PBMP_ALL(unit), port) {
                if (_num_port_cosq[unit][port].numq) {
                    BCM_IF_ERROR_RETURN
                        (_bcm_robo_cosq_gport_delete(unit, port));
                }
            }
    
            sal_free(_num_port_cosq[unit]);
            _num_port_cosq[unit] = NULL;
        }
    }
#endif /* BCM_POLAR_SUPPORT || BCM_NORTHSTAR_SUPPORT */
#ifdef BCM_TB_SUPPORT
    if (SOC_IS_TBX(unit)) {
        BCM_IF_ERROR_RETURN(DRV_QUEUE_QOS_CONTROL_SET
                (unit, -1, DRV_QOS_CTL_SW_SHADOW, FALSE));
    }
#endif
    return BCM_E_NONE;
}

/*
 * Function:
 *      bcm_robo_cosq_init
 * Purpose:
 *      Initialize (clear) all COS schedule/mapping state.
 * Parameters:
 *      unit - RoboSwitch unit number.
 * Returns:
 *      BCM_E_XXX
 */

int
bcm_robo_cosq_init(int unit)
{  
    bcm_cos_t   prio;
    int         num_cos;
    int         map_queue = 0;
#ifdef BCM_TB_SUPPORT
    bcm_port_t port = 0;
    int dp = 0;
    int srccp = 0;
    pcp2dptc_entry_t  entry_pcp2dptc;
    uint32  field_val32 = 0, count = 0;
    int  index = 0;
#endif
    pbmp_t      t_pbm;

    LOG_VERBOSE(BSL_LS_BCM_COMMON,
                (BSL_META_U(unit,
                            "BCM API : bcm_robo_cosq_init()..\n")));
    /* 
     *  Setting default queue number 
     */
    num_cos = soc_property_get(unit, spn_BCM_NUM_COS, _BCM_COS_DEFAULT(unit));

    if (num_cos < 1) {
        num_cos = 1;
    } else if (num_cos > NUM_COS(unit)) {
        num_cos = NUM_COS(unit);
    }

    BCM_IF_ERROR_RETURN(bcm_cosq_config_set(unit, num_cos));

    /* Qos 1P Enable */
    BCM_PBMP_CLEAR(t_pbm); 
    BCM_PBMP_ASSIGN(t_pbm, PBMP_ALL(unit));
#if defined(BCM_POLAR_SUPPORT) || defined(BCM_NORTHSTAR_SUPPORT) || \
    defined(BCM_NORTHSTARPLUS_SUPPORT) || defined(BCM_STARFIGHTER3_SUPPORT)
    if(SOC_IS_POLAR(unit) || SOC_IS_NORTHSTAR(unit) ||
            SOC_IS_NORTHSTARPLUS(unit) || SOC_IS_STARFIGHTER3(unit)) {
        /*  Default setting for TC_SEL_TABLE/QOS_1P_EN/QOS_TOS_DIF_EN */
        BCM_IF_ERROR_RETURN(DRV_QUEUE_MAPPING_TYPE_SET
                        (unit, t_pbm, 
                        DRV_QUEUE_MAP_NONE, TRUE));
    }
#endif /* BCM_POLAR_SUPPORT || BCM_NORTHSTAR_SUPPORT || BCM_NORTHSTARPLUS_SUPPORT
          BCM_STARFIGHTER3_SUPPORT */

#if defined(BCM_POLAR_SUPPORT) || defined(BCM_NORTHSTAR_SUPPORT)
    if(SOC_IS_POLAR(unit) || SOC_IS_NORTHSTAR(unit)) {
        if (_num_port_cosq[unit] == NULL) {
            _num_port_cosq[unit] = 
                sal_alloc(sizeof(num_port_cosq_t) * BCM_PBMP_PORT_MAX, 
                "_num_port_cosq");
            if (_num_port_cosq[unit] == NULL) {
                sal_free(_num_port_cosq[unit]);
                return BCM_E_MEMORY;
            }
        }
        sal_memset(_num_port_cosq[unit], 0, 
            sizeof(num_port_cosq_t) * BCM_PBMP_PORT_MAX);
    }
#endif /* BCM_POLAR_SUPPORT || BCM_NORTHSTAR_SUPPORT */

    BCM_IF_ERROR_RETURN(DRV_QUEUE_MAPPING_TYPE_SET
                    (unit, t_pbm, 
                    DRV_QUEUE_MAP_PRIO, TRUE));
    /* 
     *  Setting default priority mapping 
     */
    for (prio = 0; prio <= _BCM_PRIO_MAX(unit); prio++) {
        map_queue = prio / ((int)((_BCM_PRIO_MAX(unit)+1)/num_cos));
        BCM_IF_ERROR_RETURN(bcm_cosq_mapping_set(unit, prio, map_queue));
    }

    /* 
     * Setting default Priority Threshold and the queuing mode
     */
    /* this section is not implemented in Robo5324/5338 */
    
    /* Setting default mapping of reason code and cosq, this feature so far 
     * is supported on bcm5395/53115 only
     */
    if (SOC_IS_ROBO_ARCH_VULCAN(unit)) {
        /* default mapping on reason_code will be :
         *  1. Mirroring, Default: 0
         *  2. SA Learning, Default: 0
         *  3. Switching /Flooding, Default: 1
         *  4. Protocol Termination, Default: 3
         *  5. Protocol Snooping, Default: 2
         *  6. Exception Processing, Default: 2 
         */
        BCM_IF_ERROR_RETURN(DRV_QUEUE_RX_REASON_SET
                (unit, DRV_RX_REASON_MIRRORING, 0));
        BCM_IF_ERROR_RETURN(DRV_QUEUE_RX_REASON_SET
                (unit, DRV_RX_REASON_SA_LEARNING, 0));
        BCM_IF_ERROR_RETURN(DRV_QUEUE_RX_REASON_SET
                (unit, DRV_RX_REASON_SWITCHING, 1));
        BCM_IF_ERROR_RETURN(DRV_QUEUE_RX_REASON_SET
                (unit, DRV_RX_REASON_PROTO_TERM, 3));
        BCM_IF_ERROR_RETURN(DRV_QUEUE_RX_REASON_SET
                (unit, DRV_RX_REASON_PROTO_SNOOP, 2));
        BCM_IF_ERROR_RETURN(DRV_QUEUE_RX_REASON_SET
                (unit, DRV_RX_REASON_EXCEPTION, 2));
    }
    
#ifdef BCM_TB_SUPPORT
    sal_memset(&entry_pcp2dptc, 0, sizeof (entry_pcp2dptc));
    if (SOC_IS_TBX(unit)) {
        /* Initialize DP = DP1 in 1P2TCDP and DSCP2TCDP mapping */
        BCM_PBMP_CLEAR(t_pbm); 
        BCM_PBMP_ASSIGN(t_pbm, PBMP_ALL(unit));

        dp = _BCM_COLOR_ENCODING(unit, bcmColorGreen);
        prio = BCM_PRIO_GREEN;

        /* Initialize Port n Default Port QoS Configuration for DP = DP1 */
        BCM_PBMP_ITER(t_pbm, port) {
            BCM_IF_ERROR_RETURN(bcm_port_control_set
                (unit, port, bcmPortControlDropPrecedence, dp));
        }

        /* Initialize DP = DP1 in 1P2TCDP mapping */
        index = 0;
        BCM_IF_ERROR_RETURN(DRV_MEM_READ
            (unit, DRV_MEM_1P_TO_TCDP, (uint32)index, 1, (uint32 *)&entry_pcp2dptc));

        field_val32 = (uint32)dp;
        BCM_IF_ERROR_RETURN(DRV_MEM_FIELD_SET
            (unit, DRV_MEM_1P_TO_TCDP, DRV_MEM_FIELD_DP,
            (uint32 *)&entry_pcp2dptc, (uint32 *)&field_val32));

        BCM_IF_ERROR_RETURN(DRV_MEM_LENGTH_GET
            (unit, DRV_MEM_1P_TO_TCDP, (uint32 *)&count));

        BCM_IF_ERROR_RETURN(DRV_MEM_FILL
            (unit, DRV_MEM_1P_TO_TCDP, 
            (uint32)index, count, (uint32 *)&entry_pcp2dptc));

        /* Initialize DP = DP1 in DSCP2TCDP mapping (Global) */
        port = -1;
        for (srccp = 0 ; srccp <= 63 ; srccp++) {
            BCM_IF_ERROR_RETURN(bcm_port_dscp_map_set
                (unit, port, srccp, srccp, prio));
        }

        /* Allocate and update sw shadow */
        BCM_IF_ERROR_RETURN(DRV_QUEUE_QOS_CONTROL_SET
                (unit, -1, DRV_QOS_CTL_SW_SHADOW, TRUE));
    }
#endif

#if defined(BCM_NORTHSTARPLUS_SUPPORT)  || defined(BCM_STARFIGHTER3_SUPPORT)
    if (SOC_IS_NORTHSTARPLUS(unit) || SOC_IS_STARFIGHTER3(unit)) {
        /* WRED profiles initialization */
        BCM_IF_ERROR_RETURN(DRV_WRED_INIT(unit));
    }
#endif /* BCM_NORTHSTARPLUS_SUPPORT || BCM_STARFIGHTER3_SUPPORT */
    
    return BCM_E_NONE;

}

/*
 * Function:
 *      bcm_robo_cosq_config_set
 * Purpose:
 *      Set the number of COS queues
 * Parameters:
 *      unit - RoboSwitch unit number.
 *      numq - number of COS queues (2, 3, or 4).
 * Returns:
 *      BCM_E_XXX
 */

int
bcm_robo_cosq_config_set(int unit, bcm_cos_queue_t numq)
{   
    uint8   drv_value;
    
    LOG_VERBOSE(BSL_LS_BCM_COMMON,
                (BSL_META_U(unit,
                            "BCM API : bcm_robo_cosq_config_set()..\n")));

    if (!BCM_COSQ_QUEUE_VALID(unit, numq - 1)) {
        return (BCM_E_PARAM);
    }
    
    drv_value = numq;
    BCM_IF_ERROR_RETURN(DRV_QUEUE_COUNT_SET
                    (unit, 0, drv_value));

    return BCM_E_NONE;
}

/*
 * Function:
 *      bcm_robo_cosq_config_get
 * Purpose:
 *      Get the number of cos queues
 * Parameters:
 *      unit - RoboSwitch unit number.
 *      numq - (Output) number of cosq
 * Returns:
 *      BCM_E_XXX
 */

int
bcm_robo_cosq_config_get(int unit, bcm_cos_queue_t *numq)
{

    uint8   drv_value;
    
    LOG_VERBOSE(BSL_LS_BCM_COMMON,
                (BSL_META_U(unit,
                            "BCM API : bcm_robo_cosq_config_get()..\n")));
    BCM_IF_ERROR_RETURN(DRV_QUEUE_COUNT_GET
                    (unit, 0, &drv_value));
                    
    *numq = (bcm_cos_queue_t)drv_value;

    return BCM_E_NONE;
}

/*
 * Function:
 *      bcm_robo_cosq_mapping_set
 * Purpose:
 *      Set which cosq a given priority should fall into
 * Parameters:
 *      unit - RoboSwitch unit number.
 *      priority - Priority value to map
 *      cosq - COS queue to map to
 * Returns:
 *      BCM_E_XXX
 */

int
bcm_robo_cosq_mapping_set(int unit, bcm_cos_t priority, bcm_cos_queue_t cosq)
{
    LOG_VERBOSE(BSL_LS_BCM_COMMON,
                (BSL_META_U(unit,
                            "BCM API : bcm_robo_cosq_mapping_set()..\n")));
    if (!BCM_COSQ_QUEUE_VALID(unit, cosq)) {
        return (BCM_E_PARAM);
    }
    
    if (!_BCM_COSQ_PRIO_VALID(unit, priority)) {
        return (BCM_E_PARAM);
    }
    
    BCM_IF_ERROR_RETURN(DRV_QUEUE_PRIO_SET
                (unit, -1, priority, cosq));

    return (BCM_E_NONE);
}
                
/*
 * Function:
 *      bcm_robo_cosq_mapping_get
 * Purpose:
 *      Determine which COS queue a given priority currently maps to.
 * Parameters:
 *      unit - RoboSwitch unit number.
 *      priority - Priority value
 *      cosq - (Output) COS queue number
 * Returns:
 *      BCM_E_XXX
 */

int
bcm_robo_cosq_mapping_get(int unit, bcm_cos_t priority, bcm_cos_queue_t *cosq)
{
    uint8       t_cosq = 0;
    
    LOG_VERBOSE(BSL_LS_BCM_COMMON,
                (BSL_META_U(unit,
                            "BCM API : bcm_robo_cosq_mapping_get()..\n")));

    if (!_BCM_COSQ_PRIO_VALID(unit, priority)) {
        return (BCM_E_PARAM);
    }

    BCM_IF_ERROR_RETURN(DRV_QUEUE_PRIO_GET
                (unit, -1, priority, &t_cosq));

    *cosq = t_cosq;
    return (BCM_E_NONE);
}


int
bcm_robo_cosq_port_mapping_set(int unit, bcm_port_t port,
                   bcm_cos_t priority, bcm_cos_queue_t cosq)
{
    bcm_port_t  loc_port;

    LOG_VERBOSE(BSL_LS_BCM_COMMON,
                (BSL_META_U(unit,
                            "BCM API : bcm_robo_cosq_port_mapping_set()..\n")));

    if (!BCM_COSQ_QUEUE_VALID(unit, cosq)) {
        return (BCM_E_PARAM);
    }
    
    if (!_BCM_COSQ_PRIO_VALID(unit, priority)) {
        return (BCM_E_PARAM);
    }

    if (BCM_GPORT_IS_SET(port)) {
        if ((BCM_GPORT_MODPORT_MODID_GET(port) == _SHR_GPORT_MODID_MASK) &&
            (BCM_GPORT_MODPORT_PORT_GET(port) == _SHR_GPORT_PORT_MASK)) {
            /* System configuration for gport type */
            loc_port = -1;
        } else {
            BCM_IF_ERROR_RETURN(bcm_port_local_get(unit, port, &loc_port));
        }
    } else {
        loc_port = port;
    }

    if (loc_port != -1 && !SOC_PORT_VALID(unit, loc_port)) {
        return BCM_E_PORT;
    }
    
    BCM_IF_ERROR_RETURN(DRV_QUEUE_PRIO_SET
                (unit, loc_port, priority, cosq));

    return (BCM_E_NONE);
}
                
int
bcm_robo_cosq_port_mapping_get(int unit, bcm_port_t port,
                   bcm_cos_t priority, bcm_cos_queue_t *cosq)
{
    uint8       t_cosq = 0;
    bcm_port_t  loc_port;
    
    LOG_VERBOSE(BSL_LS_BCM_COMMON,
                (BSL_META_U(unit,
                            "BCM API : bcm_robo_cosq_port_mapping_get()..\n")));

    if (!_BCM_COSQ_PRIO_VALID(unit, priority)) {
        return (BCM_E_PARAM);
    }

    if (BCM_GPORT_IS_SET(port)) {
        if ((BCM_GPORT_MODPORT_MODID_GET(port) == _SHR_GPORT_MODID_MASK) &&
            (BCM_GPORT_MODPORT_PORT_GET(port) == _SHR_GPORT_PORT_MASK)) {
            /* System configuration for gport type */
            loc_port = -1;
        } else {
            BCM_IF_ERROR_RETURN(bcm_port_local_get(unit, port, &loc_port));
        }
    } else {
        loc_port = port;
    }

    if (loc_port != -1 && !SOC_PORT_VALID(unit, loc_port)) {
        return BCM_E_PORT;
    }

    BCM_IF_ERROR_RETURN(DRV_QUEUE_PRIO_GET
                (unit, loc_port, priority, &t_cosq));

    *cosq = t_cosq;
    return (BCM_E_NONE);
}

int
bcm_robo_cosq_port_mapping_multi_set(int unit, bcm_port_t port,int count,
                   bcm_cos_t *priority, bcm_cos_queue_t *cosq)
{
    return BCM_E_UNAVAIL;
}

int
bcm_robo_cosq_port_mapping_multi_get(int unit, bcm_port_t port,int count,
                   bcm_cos_t *priority, bcm_cos_queue_t *cosq)
{
    return BCM_E_UNAVAIL;
}
/*
 * Function:
 *      bcm_robo_cosq_port_bandwidth_get
 * Purpose:
 *      Retrieve bandwidth values for given COS policy.
 * Parameters:
 *      unit - RoboSwitch unit number.
 *  port - port to configure, -1 for any port.
 *      cosq - COS queue to configure, -1 for any COS queue.
 *      kbits_sec_min - (OUT) minimum bandwidth, kbits/sec.
 *      kbits_sec_max - (OUT) maximum bandwidth, kbits/sec.
 *      flags - (OUT) may include:
 *              BCM_COSQ_BW_EXCESS_PREF
 *              BCM_COSQ_BW_MINIMUM_PREF
 * Returns:
 *      BCM_E_XXX
 * Note:
 *      This API is designed mainly for Tucana Chip and not suitable
 *      for Robo Chip.
 *
 *      Per queue egress rate control is implemented in this API
 *      for Robo chips. 
 *      Egress rate control need 2 parameters, rate limit and 
 *      bucket size. Although the meaning is not exactly the same,
 *      the kbits_sec_min and kbits_sec_max parameters of this API 
 *      are used to represent the rate limit and bucket size, respectively.
 *
 *      For TB Chip, the kbits_sec_min and kbits_sec_max parameters
 *      are used to represent the minimum and maximum rate limit, respectively.
 *      And the bucket size is the same as max rate limit in this API.
 */
int 
bcm_robo_cosq_port_bandwidth_get(int unit, bcm_port_t port,
                                   bcm_cos_queue_t cosq,
                                   uint32 *kbits_sec_min,
                                   uint32 *kbits_sec_max,
                                   uint32 *flags)
{
    uint32 temp, temp2;
    bcm_port_t loc_port;
    bcm_cos_queue_t loc_cos;
    uint32 kbits_sec_burst;    /* Dummy variable */
    uint32 rate_flags = 0;
    
    LOG_VERBOSE(BSL_LS_BCM_COMMON,
                (BSL_META_U(unit,
                            "BCM API : bcm_robo_cosq_port_bandwidth_get()\n")));

#if defined(BCM_POLAR_SUPPORT) || defined(BCM_NORTHSTAR_SUPPORT)
    if (SOC_IS_POLAR(unit) || SOC_IS_NORTHSTAR(unit)) {
        /* Polar doesn't support Leaky Bucket Shaper for SCH2.
         * And the Leaky Bucket Shaper for SCH1 is done by 
         *     gport bandwidth APIs 
         */
        return BCM_E_UNAVAIL;
    }
#endif /* BCM_POLAR_SUPPORT || BCM_NORTHSTAR_SUPPORT */

    if (!SOC_UNIT_VALID(unit)) {
        return BCM_E_UNIT;
    }
    if ((kbits_sec_min == NULL) || (kbits_sec_max == NULL) || (flags == NULL)) {
        return BCM_E_PARAM;
    }

    if (BCM_GPORT_IS_SET(port)) {
        if ((BCM_GPORT_MODPORT_MODID_GET(port) == _SHR_GPORT_MODID_MASK) &&
            (BCM_GPORT_MODPORT_PORT_GET(port) == _SHR_GPORT_PORT_MASK)) {
            /* System configuration for gport type */
            loc_port = -1;
        } else {
            BCM_IF_ERROR_RETURN(bcm_port_local_get(unit, port, &loc_port));
        }
    } else {
        loc_port = port;
    }

    if (loc_port < 0) {
        loc_port = SOC_PORT_MIN(unit, all);
    }

    if (!SOC_PORT_VALID(unit, loc_port)) {
        return BCM_E_PORT; 
    }

    if (cosq < 0) {
        loc_cos = 0;
    } else {
        loc_cos = cosq;
    }

    switch (loc_cos) {
        case 0:
        case 1:
        case 2:
        case 3:
        case 6:
        case 7:
            BCM_IF_ERROR_RETURN(DRV_RATE_GET
                                (unit, loc_port, loc_cos,
                                DRV_RATE_CONTROL_DIRECTION_EGRESSS_PER_QUEUE,
                                &rate_flags, kbits_sec_min, 
                                kbits_sec_max, &kbits_sec_burst));
            break;
        case 4:
            if (SOC_IS_TBX(unit) || SOC_IS_NORTHSTARPLUS(unit) ||
               (SOC_IS_STARFIGHTER3(unit))) {
#if defined(BCM_TB_SUPPORT) || defined(BCM_NORTHSTARPLUS_SUPPORT) || \
    defined(BCM_STARFIGHTER3_SUPPORT)
                BCM_IF_ERROR_RETURN(DRV_RATE_GET
                                    (unit, loc_port, loc_cos,
                                    DRV_RATE_CONTROL_DIRECTION_EGRESSS_PER_QUEUE,
                                    &rate_flags, kbits_sec_min, 
                                    kbits_sec_max, &kbits_sec_burst));
#endif /* BCM_TB_SUPPORT || BCM_NORTHSTARPLUS_SUPPORT ||
          BCM_STARFIGHTER3_SUPPORT */
            } else if (soc_feature(unit, soc_feature_eav_support)) {
                /* Get current Macro Slot time */
                BCM_IF_ERROR_RETURN(DRV_EAV_TIME_SYNC_GET
                    (unit, DRV_EAV_TIME_SYNC_MACRO_SLOT_PERIOD, &temp, &temp2));
                /*
                 * kbit/sec =(bytes/slot time) * 8 * 1000 / (macro slot time * 1024)
                 */
                BCM_IF_ERROR_RETURN(DRV_EAV_QUEUE_CONTROL_GET
                    (unit, loc_port, DRV_EAV_QUEUE_Q4_BANDWIDTH, &temp2));
                *kbits_sec_min = (temp2 * 8 * 1000) / (temp * 1024); 

                    /* unused parameter kbits_sec_max : return (-1) */
                    *kbits_sec_max = -1;
                    *flags = 0;
            } else {
                return BCM_E_UNAVAIL;
            }
            break;
        case 5:
            if (SOC_IS_TBX(unit) || SOC_IS_NORTHSTARPLUS(unit) ||
               (SOC_IS_STARFIGHTER3(unit))) {
#if defined(BCM_TB_SUPPORT) || defined(BCM_NORTHSTARPLUS_SUPPORT) || \
    defined(BCM_STARFIGHTER3_SUPPORT)
                BCM_IF_ERROR_RETURN(DRV_RATE_GET
                                    (unit, loc_port, loc_cos,
                                    DRV_RATE_CONTROL_DIRECTION_EGRESSS_PER_QUEUE,
                                    &rate_flags, kbits_sec_min, 
                                    kbits_sec_max, &kbits_sec_burst));
#endif /* BCM_TB_SUPPORT || BCM_NORTHSTARPLUS_SUPPORT ||
          BCM_STARFIGHTER3_SUPPORT */
            } else if (soc_feature(unit, soc_feature_eav_support)) {
                /*
                 * Class 5 slot time is 125 us.
                 * kbits/sec = (bytes/slot * 8 * 8000) / 1024
                 */
                BCM_IF_ERROR_RETURN(DRV_EAV_QUEUE_CONTROL_GET
                    (unit, loc_port, DRV_EAV_QUEUE_Q5_BANDWIDTH, &temp));
                *kbits_sec_min = (temp * 8 * 8000) / 1024;

                /* unused parameter kbits_sec_max : return (-1) */
                *kbits_sec_max = -1;
            } else {
                return BCM_E_UNAVAIL;
            }
            break;
        default:
            return BCM_E_PARAM;
    }

    if (rate_flags & DRV_RATE_CONTROL_FLAG_EGRESS_AVB_MODE) {
        *flags |= BCM_COSQ_BW_EAV_MODE;
    }
    if (rate_flags & DRV_RATE_CONTROL_FLAG_EGRESS_PACKET_BASED) {
        *flags |= BCM_COSQ_BW_PACKET_MODE;
    }

    return BCM_E_NONE;
}

/*
 * Function:
 *      bcm_robo_cosq_port_bandwidth_set
 * Purpose:
 *      Set bandwidth values for given COS policy.
 * Parameters:
 *      unit - RoboSwitch unit number.
 *  port - port to configure, -1 for all ports.
 *      cosq - COS queue to configure, -1 for all COS queues.
 *      kbits_sec_min - minimum bandwidth, kbits/sec.
 *      kbits_sec_max - maximum bandwidth, kbits/sec.
 *      flags - may include:
 *              BCM_COSQ_BW_EXCESS_PREF
 *              BCM_COSQ_BW_MINIMUM_PREF
 * Returns:
 *      BCM_E_XXX
 * Note:
 *      This API is designed mainly for Tucana Chip and not suitable
 *      for Robo Chip.
 *
 *      Per queue egress rate control is implemented in this API
 *      for Robo chips. 
 *      Egress rate control need 2 parameters, rate limit and 
 *      bucket size. Although the meaning is not exactly the same,
 *      the kbits_sec_min and kbits_sec_max parameters of this API 
 *      are used to represent the rate limit and bucket size, respectively.
 *
 *      For TB Chip, the kbits_sec_min and kbits_sec_max parameters
 *      are used to represent the minimum and maximum rate limit, respectively.
 *      And the bucket size is the same as max rate limit in this API.
 */
int 
bcm_robo_cosq_port_bandwidth_set(int unit, bcm_port_t port,
                                   bcm_cos_queue_t cosq,
                                   uint32 kbits_sec_min,
                                   uint32 kbits_sec_max,
                                   uint32 flags)
{
    uint32 temp, temp2;
    bcm_pbmp_t pbmp;
    bcm_port_t loc_port;
    int i;
    
    LOG_VERBOSE(BSL_LS_BCM_COMMON,
                (BSL_META_U(unit,
                            "BCM API : bcm_robo_cosq_port_bandwidth_set()\n")));

#if defined(BCM_POLAR_SUPPORT) || defined(BCM_NORTHSTAR_SUPPORT)
    if (SOC_IS_POLAR(unit) || SOC_IS_NORTHSTAR(unit)) {
        /* Polar doesn't support Leaky Bucket Shaper for SCH2.
         * And the Leaky Bucket Shaper for SCH1 is done by 
         * gport bandwidth APIs 
         */
        return BCM_E_UNAVAIL;
    }
#endif /* BCM_POLAR_SUPPORT || BCM_NORTHSTAR_SUPPORT */

    if (!SOC_UNIT_VALID(unit)) {
        return BCM_E_UNIT;
    }
 
    if (BCM_GPORT_IS_SET(port)) {
        if ((BCM_GPORT_MODPORT_MODID_GET(port) == _SHR_GPORT_MODID_MASK) &&
            (BCM_GPORT_MODPORT_PORT_GET(port) == _SHR_GPORT_PORT_MASK)) {
            /* System configuration for gport type */
            loc_port = -1;
        } else {
            BCM_IF_ERROR_RETURN(bcm_port_local_get(unit, port, &loc_port));
        }
    } else {
        loc_port = port;
    }

    if (loc_port < 0) {
        /* System configuration for local port type */
        BCM_PBMP_ASSIGN(pbmp, PBMP_ALL(unit));
    } else {
        if (SOC_PORT_VALID(unit, loc_port)) {
            BCM_PBMP_PORT_SET(pbmp, loc_port);
        } else {
            return BCM_E_PORT; 
        }
    }

    if (cosq < 0) {
        for (i = 0; i < NUM_COS(unit) - 1; i++) {
            BCM_IF_ERROR_RETURN(DRV_RATE_SET
                                (unit, pbmp, i,
                                DRV_RATE_CONTROL_DIRECTION_EGRESSS_PER_QUEUE,
                                0, kbits_sec_min, 
                                kbits_sec_max, kbits_sec_max));
        }
        return BCM_E_NONE;
    }

    temp2 = 0;
    if (flags & BCM_COSQ_BW_PACKET_MODE) {
        temp2 |= DRV_RATE_CONTROL_FLAG_EGRESS_PACKET_BASED;
    }
    if (flags & BCM_COSQ_BW_EAV_MODE) {
        temp2 |= DRV_RATE_CONTROL_FLAG_EGRESS_AVB_MODE;
    }
    switch (cosq) {
        case 0:
        case 1:
        case 2:
        case 3:
        case 6:
        case 7:
            BCM_IF_ERROR_RETURN(DRV_RATE_SET
                                (unit, pbmp, cosq,
                                DRV_RATE_CONTROL_DIRECTION_EGRESSS_PER_QUEUE,
                                temp2, kbits_sec_min, 
                                kbits_sec_max, kbits_sec_max));
            break;
        case 4:
            if (SOC_IS_TBX(unit) || SOC_IS_NORTHSTARPLUS(unit) ||
               (SOC_IS_STARFIGHTER3(unit))) {
#if defined(BCM_TB_SUPPORT) || defined(BCM_NORTHSTARPLUS_SUPPORT) || \
    defined(BCM_STARFIGHTER3_SUPPORT)
                BCM_IF_ERROR_RETURN(DRV_RATE_SET
                                    (unit, pbmp, cosq,
                                    DRV_RATE_CONTROL_DIRECTION_EGRESSS_PER_QUEUE,
                                    temp2, kbits_sec_min, 
                                    kbits_sec_max, kbits_sec_max));
#endif /* BCM_TB_SUPPORT || BCM_NORTHSTARPLUS_SUPPORT ||
          BCM_STARFIGHTER3_SUPPORT */
            } else if (soc_feature(unit, soc_feature_eav_support)) {
                
                 /* Check the maximum valid bandwidth value for EAV Class 4 */
                BCM_IF_ERROR_RETURN(DRV_EAV_QUEUE_CONTROL_GET
                    (unit, loc_port, DRV_EAV_QUEUE_Q4_BANDWIDTH_MAX_VALUE, &temp));

                if (kbits_sec_min > temp) {
                    LOG_ERROR(BSL_LS_BCM_COMMON,
                              (BSL_META_U(unit,
                                          "bcm_robo_cosq_port_bandwidth_set : BW value unsupported. \n")));
                    return  SOC_E_PARAM;
                }
                /* Get current Macro Slot time */
                BCM_IF_ERROR_RETURN(DRV_EAV_TIME_SYNC_GET
                    (unit, DRV_EAV_TIME_SYNC_MACRO_SLOT_PERIOD, &temp, &temp2));
                /*
                 * bytes/slot = kbits/sec * 1024 / (8 * macro slot time * 1000)
                 */
                temp2 = (kbits_sec_min * 1024 * temp) / (8 * 1000);
                BCM_PBMP_ITER(pbmp, loc_port) {
                    BCM_IF_ERROR_RETURN(DRV_EAV_QUEUE_CONTROL_SET
                        (unit, loc_port, DRV_EAV_QUEUE_Q4_BANDWIDTH, temp2));
                }
            } else {
                return BCM_E_UNAVAIL;
            }
            break;
        case 5:
            if (SOC_IS_TBX(unit) || SOC_IS_NORTHSTARPLUS(unit) ||
               (SOC_IS_STARFIGHTER3(unit))) {
#if defined(BCM_TB_SUPPORT) || defined(BCM_NORTHSTARPLUS_SUPPORT) || \
    defined(BCM_STARFIGHTER3_SUPPORT)
                BCM_IF_ERROR_RETURN(DRV_RATE_SET
                                    (unit, pbmp, cosq,
                                    DRV_RATE_CONTROL_DIRECTION_EGRESSS_PER_QUEUE,
                                    temp2, kbits_sec_min, 
                                    kbits_sec_max, kbits_sec_max));
#endif /* BCM_TB_SUPPORT || BCM_NORTHSTARPLUS_SUPPORT ||
          BCM_STARFIGHTER3_SUPPORT */
             } else if (soc_feature(unit, soc_feature_eav_support)) {

                 /* Check the maximum valid bandwidth value for EAV Class 5 */
                BCM_IF_ERROR_RETURN(DRV_EAV_QUEUE_CONTROL_GET
                    (unit, loc_port, DRV_EAV_QUEUE_Q5_BANDWIDTH_MAX_VALUE, &temp));

                if (kbits_sec_min > temp) {
                    LOG_ERROR(BSL_LS_BCM_COMMON,
                              (BSL_META_U(unit,
                                          "bcm_robo_cosq_port_bandwidth_set : BW value unsupported. \n")));
                    return  SOC_E_PARAM;
                }
                /*
                 * Class 5 slot time is 125 us.
                 * bytes/125us = kbit/sec * 1024 /(8 * 8000) 
                 */
                temp = (kbits_sec_min * 1024) / (8 * 8000); 
                BCM_PBMP_ITER(pbmp, loc_port) {
                    BCM_IF_ERROR_RETURN(DRV_EAV_QUEUE_CONTROL_SET
                        (unit, loc_port, DRV_EAV_QUEUE_Q5_BANDWIDTH, temp));
                }
            } else {
                return BCM_E_UNAVAIL;
            }
            break;
        default:
            return BCM_E_PARAM;
    }

    return BCM_E_NONE;
}

/*
 * Function:
 *      bcm_robo_cosq_sched_weight_max_get
 * Purpose:
 *      Retrieve maximum weights for given COS policy.
 * Parameters:
 *      unit - RoboSwitch unit number.
 *      mode - Scheduling mode, one of BCM_COSQ_xxx
 *  weight_max - (output) Maximum weight for COS queue.
 *      0 if mode is BCM_COSQ_STRICT.
 *      1 if mode is BCM_COSQ_ROUND_ROBIN.
 *      -1 if not applicable to mode.
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_robo_cosq_sched_weight_max_get(int unit, int mode,
                     int *weight_max)
{
    uint32 prop_val = 0;
    
    LOG_VERBOSE(BSL_LS_BCM_COMMON,
                (BSL_META_U(unit,
                            "BCM API : bcm_robo_cosq_sched_weight_max_get()..\n")));
    switch (mode) {
    case BCM_COSQ_STRICT:
        *weight_max = BCM_COSQ_WEIGHT_STRICT;
        break;
    case BCM_COSQ_DEFICIT_ROUND_ROBIN:
    case BCM_COSQ_WEIGHTED_ROUND_ROBIN:
        BCM_IF_ERROR_RETURN(DRV_DEV_PROP_GET
                        (unit, DRV_DEV_PROP_COSQ_MAX_WEIGHT_VALUE, &prop_val));
        *weight_max = (int)prop_val;
        break;
    default:
        *weight_max = BCM_COSQ_WEIGHT_UNLIMITED;
        return BCM_E_PARAM;
    }

    return BCM_E_NONE;
}

int
bcm_robo_cosq_port_sched_set(int unit, bcm_pbmp_t pbm,
            int mode, const int weights[], int delay)
{
    uint32      drv_value, weight_value;
    int         i, sp_num;
    bcm_pbmp_t pbm_all = PBMP_ALL(unit);
    uint8  numq;
    uint32 flag = 0;
    
    LOG_VERBOSE(BSL_LS_BCM_COMMON,
                (BSL_META_U(unit,
                            "BCM API : bcm_robo_cosq_port_sched_set()..\n")));

    if (BCM_PBMP_IS_NULL(pbm)) {
        return BCM_E_PORT;
    }

    BCM_PBMP_AND(pbm_all, pbm);
    if (BCM_PBMP_NEQ(pbm_all, pbm)) {
        return BCM_E_PORT;
    }
    
    /* for Robo Chip, we support Strict, WRR and WDRR mode */
    drv_value = (mode == BCM_COSQ_STRICT) ? DRV_QUEUE_MODE_STRICT : 
                (mode == BCM_COSQ_WEIGHTED_ROUND_ROBIN) ? DRV_QUEUE_MODE_WRR :
                (mode == BCM_COSQ_DEFICIT_ROUND_ROBIN) ? 
                DRV_QUEUE_MODE_WDRR : 0;
    
    /* the COSQ mode for Robo allowed Strict & WRR mode only */
    if (drv_value == 0){
        return BCM_E_PARAM;
    }

#if defined(BCM_POLAR_SUPPORT) || defined(BCM_NORTHSTAR_SUPPORT)
    if (SOC_IS_POLAR(unit) || SOC_IS_NORTHSTAR(unit)) {
        flag = DRV_QUEUE_FLAG_LEVLE2;
    }
#endif /* BCM_POLAR_SUPPORT || BCM_NORTHSTAR_SUPPORT */

    if (mode == BCM_COSQ_STRICT) {
        BCM_IF_ERROR_RETURN(DRV_QUEUE_MODE_SET
                        (unit, pbm, flag, drv_value));
        return BCM_E_NONE;
    }

    if (SOC_IS_TBX(unit) || SOC_IS_NORTHSTARPLUS(unit) ||
       (SOC_IS_STARFIGHTER3(unit)) ) {
#if defined(BCM_TB_SUPPORT) || defined (BCM_NORTHSTARPLUS_SUPPORT) || \
    defined(BCM_STARFIGHTER3_SUPPORT)
        /* Now only TB and NSP support WDRR mode */
        /* set the weight if mode is WDRR */
        sp_num = 0;
        for (i = 0; i < NUM_COS(unit); i++) {
            weight_value = weights[i];
            if (weight_value == BCM_COSQ_WEIGHT_STRICT) {
                sp_num ++;
            }
        }

        /* Check the scheduler configuration selections for combination of SP and WDRR */
        if (sp_num >= 5) {
            return BCM_E_PARAM;
        }

        /* Set scheduling combination mode if ROBO chip support it */
        if (sp_num == 1) { 
             /* 1STRICT/7WDRR : COSQ7 = STRICT */
            if (weights[NUM_COS(unit) - 1] == BCM_COSQ_WEIGHT_STRICT) {
                drv_value = DRV_QUEUE_MODE_1STRICT_7WDRR;
            } else {
                return BCM_E_PARAM;
            }
        } else if ((sp_num == 2)) { 
            /* 2STRICT/6WDRR : COSQ7 = COSQ6 = STRICT */
            if ((weights[NUM_COS(unit) - 1] == BCM_COSQ_WEIGHT_STRICT) &&
                 (weights[NUM_COS(unit) - 2] == BCM_COSQ_WEIGHT_STRICT)) {
                drv_value = DRV_QUEUE_MODE_2STRICT_6WDRR;
            } else {
                return BCM_E_PARAM;
            }
        } else if ((sp_num == 3)) { 
            /* 3STRICT/5WDRR : COSQ7 = COSQ6 = COSQ5 = STRICT */
            if ((weights[NUM_COS(unit) - 1] == BCM_COSQ_WEIGHT_STRICT) &&
                 (weights[NUM_COS(unit) - 2] == BCM_COSQ_WEIGHT_STRICT) &&
                 (weights[NUM_COS(unit) - 3] == BCM_COSQ_WEIGHT_STRICT)) {
                drv_value = DRV_QUEUE_MODE_3STRICT_5WDRR;
            } else {
                return BCM_E_PARAM;
            }
        } else if ((sp_num == 4)) { 
            /* 4STRICT/4WDRR : COSQ7 = COSQ6 = COSQ5 = COSQ4 = STRICT */
            if ((weights[NUM_COS(unit) - 1] == BCM_COSQ_WEIGHT_STRICT) &&
                 (weights[NUM_COS(unit) - 2] == BCM_COSQ_WEIGHT_STRICT) &&
                 (weights[NUM_COS(unit) - 3] == BCM_COSQ_WEIGHT_STRICT) &&
                 (weights[NUM_COS(unit) - 4] == BCM_COSQ_WEIGHT_STRICT)) {
                drv_value = DRV_QUEUE_MODE_4STRICT_4WDRR;
            } else {
                return BCM_E_PARAM;
            }
        }

        for (i = 0; i < NUM_COS(unit); i++) {
            weight_value = weights[i];
            if (weight_value != BCM_COSQ_WEIGHT_STRICT) {
                BCM_IF_ERROR_RETURN(DRV_QUEUE_WRR_WEIGHT_SET
                    (unit, 0, pbm, i, weight_value));
            }
        }
#endif  /* BCM_TB_SUPPORT) || (BCM_NORTHSTARPLUS_SUPPORT) */
    } else {
        if (mode == BCM_COSQ_WEIGHTED_ROUND_ROBIN) {
            /* set the weight if mode is WRR */
            sp_num = 0;
#if defined(BCM_POLAR_SUPPORT) || defined(BCM_NORTHSTAR_SUPPORT)
            if (SOC_IS_POLAR(unit) || SOC_IS_NORTHSTAR(unit)) {
                SOC_IF_ERROR_RETURN(DRV_QUEUE_COUNT_GET(unit, 0, &numq));
            } else
#endif /* BCM_POLAR_SUPPORT || BCM_NORTHSTAR_SUPPORT */
            {
                numq = NUM_COS(unit);
            }

            for (i = 0; i < numq; i++){
                weight_value = weights[i];
                /* COSQ3 or COSQ2 is Strict priority if ROBO chip support STRICT/WRR scheduler */
                if (((i == numq - 1) || (i == numq - 2)) && 
                    (weight_value == BCM_COSQ_WEIGHT_STRICT)) {
                    sp_num ++;
                } else {
                        BCM_IF_ERROR_RETURN(DRV_QUEUE_WRR_WEIGHT_SET
                                    (unit, 0, pbm, i, weight_value));
                }
            }
        
            /* Set scheduling combination mode if ROBO chip support it */
            if (sp_num == 1) { /* 1STRICT/3WRR : COSQ3>COS2/COS1/COS0 */
               if (weights[numq - 1] == BCM_COSQ_WEIGHT_STRICT) {
                   drv_value = DRV_QUEUE_MODE_1STRICT;
               } else {
                   return BCM_E_PARAM;
               }
            } else if ((sp_num == 2)) { /* 2STRICT/2WRR : COSQ3>COS2>COS1/COS0 */
                drv_value = DRV_QUEUE_MODE_2STRICT;
            }
        } else {
            return BCM_E_UNAVAIL;
        }
    }

    BCM_IF_ERROR_RETURN(DRV_QUEUE_MODE_SET
                    (unit, pbm, flag, drv_value));    
    
    return BCM_E_NONE;
}

/*
 * Function:
 *      bcm_robo_cosq_sched_set
 * Purpose:
 *      Set up class-of-service policy and corresponding weights and delay
 * Parameters:
 *      unit - RoboSwitch unit number.
 *      mode - Scheduling mode, one of BCM_COSQ_xxx
 *  weights - Weights for each COS queue
 *      Unused if mode is BCM_COSQ_STRICT.
 *      Indicates number of packets sent before going on to
 *      the next COS queue.
 *  delay - Maximum delay in microseconds before returning the
 *      round-robin to the highest priority COS queue
 *      (Unused if mode other than BCM_COSQ_BOUNDED_DELAY)
 * Returns:
 *      BCM_E_XXX
 * Note :
 *      1. Not recommend user to set queue threshold at 
 *          Robo5338/5324/5380/5388.
 *      2. the Strict Mode in RobSwitch actually strict at the higest 
 *          Queue only. the other lower queues will still working at 
 *          WRR mode if set BCM_COSQ_STRICT.
 */

int
bcm_robo_cosq_sched_set(int unit, int mode, const int weights[], int delay)
{
    LOG_VERBOSE(BSL_LS_BCM_COMMON,
                (BSL_META_U(unit,
                            "BCM API : bcm_robo_cosq_sched_set()..\n")));
    /* no CPU port been assigned in this API */
    return (bcm_cosq_port_sched_set(unit,
                            PBMP_ALL(unit),
                            mode, weights, delay));
}


int
bcm_robo_cosq_port_sched_get(int unit, bcm_pbmp_t pbm,
                int *mode, int weights[], int *delay)
{
    uint32      drv_value = 0;
    int         i, port;
    bcm_pbmp_t pbm_all = PBMP_ALL(unit);
    uint8       numq;
    uint32 flag = 0;
    
    LOG_VERBOSE(BSL_LS_BCM_COMMON,
                (BSL_META_U(unit,
                            "BCM API : bcm_robo_cosq_port_sched_get()..\n")));

    if (BCM_PBMP_IS_NULL(pbm)) {
        return BCM_E_PORT;
    }

    BCM_PBMP_AND(pbm_all, pbm);
    if (BCM_PBMP_NEQ(pbm_all, pbm)) {
        return BCM_E_PORT;
    }
    
#if defined(BCM_POLAR_SUPPORT) || defined(BCM_NORTHSTAR_SUPPORT)
    if (SOC_IS_POLAR(unit) || SOC_IS_NORTHSTAR(unit)) {
        flag = DRV_QUEUE_FLAG_LEVLE2;
    }
#endif /* BCM_POLAR_SUPPORT || BCM_NORTHSTAR_SUPPORT */
    
    /* get the cosq schedule at the first port only in pbm */
    BCM_PBMP_ITER(pbm, port){
        BCM_IF_ERROR_RETURN(DRV_QUEUE_MODE_GET
                        (unit, port, flag, &drv_value));
        break;
    }
            
    /* Robo Chip shuld have strict, WRR or WDRR mode only */
    *mode = (drv_value == DRV_QUEUE_MODE_STRICT) ? BCM_COSQ_STRICT : 
            (drv_value == DRV_QUEUE_MODE_1STRICT) ? BCM_COSQ_WEIGHTED_ROUND_ROBIN :
            (drv_value == DRV_QUEUE_MODE_2STRICT) ? BCM_COSQ_WEIGHTED_ROUND_ROBIN :
            (drv_value == DRV_QUEUE_MODE_1STRICT_7WDRR) ? BCM_COSQ_DEFICIT_ROUND_ROBIN :
            (drv_value == DRV_QUEUE_MODE_2STRICT_6WDRR) ? BCM_COSQ_DEFICIT_ROUND_ROBIN :
            (drv_value == DRV_QUEUE_MODE_3STRICT_5WDRR) ? BCM_COSQ_DEFICIT_ROUND_ROBIN :
            (drv_value == DRV_QUEUE_MODE_4STRICT_4WDRR) ? BCM_COSQ_DEFICIT_ROUND_ROBIN :
            (drv_value == DRV_QUEUE_MODE_WDRR) ? BCM_COSQ_DEFICIT_ROUND_ROBIN :
            (drv_value == DRV_QUEUE_MODE_WRR) ? BCM_COSQ_WEIGHTED_ROUND_ROBIN : -1;

    if (*mode == -1) {
        return BCM_E_INTERNAL;
    }
            
    /* get the weight if mode is WRR or WDRR */
#if defined(BCM_POLAR_SUPPORT) || defined(BCM_NORTHSTAR_SUPPORT)
    if (SOC_IS_POLAR(unit) || SOC_IS_NORTHSTAR(unit)) {
        SOC_IF_ERROR_RETURN(DRV_QUEUE_COUNT_GET(unit, 0, &numq));
    } else
#endif /* BCM_POLAR_SUPPORT || BCM_NORTHSTAR_SUPPORT */
    {
        numq = NUM_COS(unit);
    }

    for (i = 0; i < numq; i++){
        BCM_IF_ERROR_RETURN(DRV_QUEUE_WRR_WEIGHT_GET
                        (unit, 0, port, i, &drv_value));
                        
        weights[i] = drv_value;
    }

    return BCM_E_NONE;
}

/*
 * Function:
 *      bcm_robo_cosq_sched_get
 * Purpose:
 *      Retrieve class-of-service policy and corresponding weights and delay
 * Parameters:
 *      unit - RoboSwitch unit number.
 *      mode_ptr - (output) Scheduling mode, one of BCM_COSQ_xxx
 *  weights - (output) Weights for each COS queue
 *      Unused if mode is BCM_COSQ_STRICT.
 *  delay - (output) Maximum delay in microseconds before returning
 *      the round-robin to the highest priority COS queue
 *      Unused if mode other than BCM_COSQ_BOUNDED_DELAY.
 * Returns:
 *      BCM_E_XXX
 * Note :
 *      1. Not recommend user to set queue threshold at 
 *          Robo5338/5324/5380/5388.
 *      2. the Strict Mode in RobSwitch actually strict at the higest 
 *          Queue only. the other lower queues will still working at 
 *          WRR mode if set BCM_COSQ_STRICT.
 */

int
bcm_robo_cosq_sched_get(int unit, int *mode, int weights[], int *delay)
{   
    LOG_VERBOSE(BSL_LS_BCM_COMMON,
                (BSL_META_U(unit,
                            "BCM API : bcm_robo_cosq_sched_get()..\n")));
    /* no CPU port been assigned in this API */
    return (bcm_cosq_port_sched_get(unit,
                            PBMP_ALL(unit),
                            mode, weights, delay));
}

#if defined(BCM_NORTHSTARPLUS_SUPPORT) || defined(BCM_STARFIGHTER3_SUPPORT)
/*
 * index: degree, value: contangent(degree) * 100
 * max value is 0xffff (16-bit) at 0 degree
 */
STATIC int
_bcm_robo_cotangent_lookup_table[] =
{
    /*  0.. 5 */  65535, 5728, 2863, 1908, 1430, 1143,
    /*  6..11 */    951,  814,  711,  631,  567,  514,
    /* 12..17 */    470,  433,  401,  373,  348,  327,
    /* 18..23 */    307,  290,  274,  260,  247,  235,
    /* 24..29 */    224,  214,  205,  196,  188,  180,
    /* 30..35 */    173,  166,  160,  153,  148,  142,
    /* 36..41 */    137,  132,  127,  123,  119,  115,
    /* 42..47 */    111,  107,  103,  100,   96,   93,
    /* 48..53 */     90,   86,   83,   80,   78,   75,
    /* 54..59 */     72,   70,   67,   64,   62,   60,
    /* 60..65 */     57,   55,   53,   50,   48,   46,
    /* 66..71 */     44,   42,   40,   38,   36,   34,
    /* 72..77 */     32,   30,   28,   26,   24,   23,
    /* 78..83 */     21,   19,   17,   15,   14,   12,
    /* 84..89 */     10,    8,    6,    5,    3,    1,
    /* 90     */      0
};

/*
 * Given a slope (angle in degrees) from 0 to 90, return the number of bytes
 * in the range from 0% drop probability to 100% drop probability.
 */
STATIC int
_bcm_nsp_angle_to_bytes(int angle)
{
    return (_bcm_robo_cotangent_lookup_table[angle]);
}

#endif /* BCM_NORTHSTARPLUS_SUPPORT || BCM_STARFIGHTER3_SUPPORT */

int
bcm_robo_cosq_discard_set(int unit, uint32 flags)
{
#if defined(BCM_NORTHSTARPLUS_SUPPORT) || defined(BCM_STARFIGHTER3_SUPPORT)
    int             cosq;
    bcm_port_t      port;
    uint32          value;

    if (!(SOC_IS_NORTHSTARPLUS(unit) || SOC_IS_STARFIGHTER3(unit))) {
        return BCM_E_UNAVAIL;
    }
    
    /* For all ports and all queues */
    port = -1;
    cosq = -1;
    
    value = (flags & BCM_COSQ_DISCARD_ENABLE) ? 1 : 0;
    BCM_IF_ERROR_RETURN(
        DRV_WRED_CONTROL_SET(unit, port, cosq, 
            DRV_WRED_CONTROL_ENABLE, value));

    value = (flags & BCM_COSQ_DISCARD_CAP_AVERAGE) ? 1 : 0;
    BCM_IF_ERROR_RETURN(
        DRV_WRED_CONTROL_SET(unit, port, cosq, 
            DRV_WRED_CONTROL_AQD_FAST_CORRECTION, value));
    
    if (flags & BCM_COSQ_DISCARD_NONTCP) {
        return BCM_E_UNAVAIL;
    }
    
    return BCM_E_NONE;
#else /* BCM_NORTHSTARPLUS_SUPPORT || BCM_STARFIGHTER3_SUPPORT */
    return BCM_E_UNAVAIL;
#endif /* !BCM_NORTHSTARPLUS_SUPPORT || !BCM_STARFIGHTER3_SUPPORT */
}

int
bcm_robo_cosq_discard_get(int unit, uint32 *flags)
{
#if defined(BCM_NORTHSTARPLUS_SUPPORT) || defined(BCM_STARFIGHTER3_SUPPORT)
    int             cosq;
    bcm_port_t      port;
    uint32          value;

    if (!(SOC_IS_NORTHSTARPLUS(unit) || SOC_IS_STARFIGHTER3(unit))) {
        return BCM_E_UNAVAIL;
    }
    
    /* For all ports and all queues */
    port = -1;
    cosq = -1;
    *flags = 0;
    BCM_IF_ERROR_RETURN(
        DRV_WRED_CONTROL_GET(unit, port, cosq, 
            DRV_WRED_CONTROL_ENABLE, &value));
    if (value) {
        *flags |= BCM_COSQ_DISCARD_ENABLE;
    }

    BCM_IF_ERROR_RETURN(
        DRV_WRED_CONTROL_GET(unit, port, cosq, 
            DRV_WRED_CONTROL_AQD_FAST_CORRECTION, &value));
    if (value) {
        *flags |= BCM_COSQ_DISCARD_CAP_AVERAGE;
    }
    
    return BCM_E_NONE;
#else /* BCM_NORTHSTARPLUS_SUPPORT || BCM_STARFIGHTER3_SUPPORT */
    return BCM_E_UNAVAIL;
#endif /* !BCM_NORTHSTARPLUS_SUPPORT || !BCM_STARFIGHTER3_SUPPORT */
}

int
bcm_robo_cosq_discard_port_set(int unit, bcm_port_t port,
                                 bcm_cos_queue_t cosq,
                                 uint32 color,
                                 int drop_start,
                                 int drop_slope,
                                 int average_time)
{
#if defined(BCM_NORTHSTARPLUS_SUPPORT) || defined(BCM_STARFIGHTER3_SUPPORT)
    bcm_cosq_gport_discard_t    discard;
    uint32                      queue_size, temp;
    bcm_gport_t                 gport = 0;

    if (!(SOC_IS_NORTHSTARPLUS(unit) || SOC_IS_STARFIGHTER3(unit))) {
        return BCM_E_UNAVAIL;
    }
    
    if (drop_start < 0 || drop_start > 100 ||
        drop_slope < 0 || drop_slope > 90) {
        return BCM_E_PARAM;
    }
    
    sal_memset(&discard, 0, sizeof(bcm_cosq_gport_discard_t));

    discard.flags = color;

    if ((drop_start != 0) && (drop_slope != 0)) {
        discard.flags |= BCM_COSQ_DISCARD_ENABLE;
    }

    BCM_IF_ERROR_RETURN(
        DRV_WRED_CONTROL_GET(unit, port, cosq, 
            DRV_WRED_CONTROL_MAX_QUEUE_SIZE, &queue_size));

    /* Calculate the min and max threshold */
    discard.min_thresh = (queue_size * drop_start) / 100;
    discard.max_thresh = discard.min_thresh + 
        ((discard.min_thresh * _bcm_nsp_angle_to_bytes(drop_slope)) / 100);
    if (discard.max_thresh > queue_size) {
        discard.max_thresh = queue_size;
    }

    discard.drop_probability = 100; /* max */
    /* Use the original value */
    BCM_IF_ERROR_RETURN(
        DRV_WRED_CONTROL_GET(unit, -1, -1, 
            DRV_WRED_CONTROL_AQD_EXPONENT, &temp));
    discard.gain = temp;

    /* Translate port (-1) to gport type */
    if (port == -1 ) {
        _SHR_GPORT_MODPORT_SET(gport, _SHR_GPORT_MODID_MASK, 
            _SHR_GPORT_PORT_MASK);
    } else {
        gport = port;
    }
    BCM_IF_ERROR_RETURN(
        bcm_cosq_gport_discard_set(unit, gport, cosq, &discard));

    /* average time to calculate the queue depth */
    temp = average_time;
    BCM_IF_ERROR_RETURN(
        DRV_WRED_CONTROL_SET(unit, -1, -1, 
            DRV_WRED_CONTROL_AQD_PERIOD, temp));
    
    
    return BCM_E_NONE;
#else /* BCM_NORTHSTARPLUS_SUPPORT || BCM_STARFIGHTER3_SUPPORT */
    return BCM_E_UNAVAIL;
#endif /* !BCM_NORTHSTARPLUS_SUPPORT || !BCM_STARFIGHTER3_SUPPORT */
}

int
bcm_robo_cosq_discard_port_get(int unit, bcm_port_t port,
                                 bcm_cos_queue_t cosq,
                                 uint32 color,
                                 int *drop_start,
                                 int *drop_slope,
                                 int *average_time)
{
#if defined(BCM_NORTHSTARPLUS_SUPPORT) || defined(BCM_STARFIGHTER3_SUPPORT)
    bcm_cosq_gport_discard_t    discard;
    uint32                      queue_size;
    int                         i, temp;
    bcm_gport_t                 gport = 0;

    if (!(SOC_IS_NORTHSTARPLUS(unit) || SOC_IS_STARFIGHTER3(unit))) {
        return BCM_E_UNAVAIL;
    }
    
    sal_memset(&discard, 0, sizeof(bcm_cosq_gport_discard_t));
    
    discard.flags = (color & BCM_COSQ_DISCARD_COLOR_ALL);
    
    /* Translate port (-1) to gport type */
    if (port == -1 ) {
        _SHR_GPORT_MODPORT_SET(gport, _SHR_GPORT_MODID_MASK, 
            _SHR_GPORT_PORT_MASK);
    } else {
        gport = port;
    }
    
    BCM_IF_ERROR_RETURN(
        bcm_cosq_gport_discard_get(unit, gport, cosq, &discard));

    color = (discard.flags & BCM_COSQ_DISCARD_COLOR_ALL);

    /* Return here if it is disabled.*/
    if (!(discard.flags & BCM_COSQ_DISCARD_ENABLE)) {
        *drop_start = 0;
        *drop_slope = 0;
        *average_time = 0;
        return BCM_E_NONE;
    }
    
    BCM_IF_ERROR_RETURN(
        DRV_WRED_CONTROL_GET(unit, port, cosq, 
            DRV_WRED_CONTROL_MAX_QUEUE_SIZE, &queue_size));

    *drop_start = (discard.min_thresh * 100) / queue_size;
    temp = (discard.max_thresh - discard.min_thresh) * 100;
    temp = (temp / discard.min_thresh);

    if (temp < 0) {
        return BCM_E_PARAM;
    }
    for (i = 0; i <= 90; i++) {
        if (temp >= _bcm_robo_cotangent_lookup_table[i]) {
            *drop_slope = i;
            break;
        }
    }

    /* average time */
    BCM_IF_ERROR_RETURN(
        DRV_WRED_CONTROL_GET(unit, -1, -1, 
            DRV_WRED_CONTROL_AQD_PERIOD, (uint32 *)&temp));
    *average_time = temp;
    
    return BCM_E_NONE;
#else /* BCM_NORTHSTARPLUS_SUPPORT || BCM_STARFIGHTER3_SUPPORT */
    return BCM_E_UNAVAIL;
#endif /* !BCM_NORTHSTARPLUS_SUPPORT || !BCM_STARFIGHTER3_SUPPORT */
}

/*
 * Function    : _bcm_robo_cosq_gport_resolve
 * Description : Internal function to get modid, port, and trunk_id
 *               from a bcm_gport_t (global port)
 * Parameters  : (IN)  unit      - BCM device number
 *               (IN)  gport     - Global port identifier
 *               (OUT) modid     - Module ID
 *               (OUT) port      - Port number
 *               (OUT) trunk_id  - Trunk ID
 *               (OUT) id        - HW ID
 * Returns     : BCM_E_XXX
 * Notes       : The modid and port are translated from the
 *               application space to local modid/port space
 */
int 
_bcm_robo_cosq_gport_resolve(int unit, bcm_gport_t gport,
                       bcm_port_t *port)
{
    if (BCM_GPORT_IS_SCHEDULER(gport)) {
        *port = BCM_GPORT_SCHEDULER_GET(gport) & 0xff;
    } else {
        return BCM_E_BADID;
    }
    return BCM_E_NONE;
}

/*
 * Function:
 *      bcm_cosq_gport_add
 * Purpose:
 *       
 * Parameters:
 *      unit - (IN) Unit number.
 *      port - (IN) Physical port.
 *      numq - (IN) Number of COS queues.
 *      flags - (IN) Flags.
 *      gport - (IN/OUT) GPORT ID.
 * Returns:
 *      BCM_E_XXX
 */
int 
bcm_robo_cosq_gport_add(
    int unit, 
    bcm_gport_t port, 
    int numq, 
    uint32 flags, 
    bcm_gport_t *gport)
{
#if defined(BCM_POLAR_SUPPORT) || defined(BCM_NORTHSTAR_SUPPORT)
    bcm_module_t modid;
    bcm_port_t local_port;
    bcm_trunk_t trunk_id;
    int gport_id;
    int local_id;
    uint32 sch1_num_cosq;

    if (SOC_IS_POLAR(unit) || SOC_IS_NORTHSTAR(unit)) {
        BCM_IF_ERROR_RETURN(DRV_DEV_PROP_GET(unit, 
                             DRV_DEV_PROP_SCH1_NUM_COSQ, &sch1_num_cosq));
        /* The numq is fixed as 3 for SCH1 */
        if ((numq <= 0) || (numq != sch1_num_cosq) ||
            (!BCM_GPORT_IS_LOCAL(port) && !BCM_GPORT_IS_MODPORT(port))) {
            return BCM_E_PARAM;
        }
        if (flags & ~BCM_COSQ_GPORT_SCHEDULER) {
            return BCM_E_UNAVAIL;
        }
        BCM_IF_ERROR_RETURN
            (_bcm_robo_gport_resolve(unit, port, &modid,
                                     &local_port, &trunk_id, &local_id));

        if (!_num_port_cosq[unit]) {
            return BCM_E_INIT;
        } else if (_num_port_cosq[unit][local_port].numq) {
            return BCM_E_EXISTS;
        }

        /* Call delete routine to init HW settings */
        BCM_IF_ERROR_RETURN (_bcm_robo_cosq_gport_delete(unit, local_port));

        _num_port_cosq[unit][local_port].numq = numq;
        
        gport_id = local_port;
        BCM_GPORT_SCHEDULER_SET(*gport, gport_id);

        return BCM_E_NONE;
    }
#endif /* BCM_POLAR_SUPPORT || BCM_NORTHSTAR_SUPPORT */
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_cosq_gport_delete
 * Purpose:
 *      
 * Parameters:
 *      unit - (IN) Unit number.
 *      gport - (IN) GPORT ID.
 * Returns:
 *      BCM_E_XXX
 */
int 
bcm_robo_cosq_gport_delete(
    int unit, 
    bcm_gport_t gport)
{
#if defined(BCM_POLAR_SUPPORT) || defined(BCM_NORTHSTAR_SUPPORT)
    bcm_module_t modid;
    bcm_port_t local_port;
    bcm_trunk_t trunk_id;    
    int local_id;

    if (SOC_IS_POLAR(unit) || SOC_IS_NORTHSTAR(unit)) {        
        if (gport == BCM_GPORT_INVALID) {
            return BCM_E_PORT;
        }
        
        if (!BCM_GPORT_IS_SCHEDULER(gport)) {
            return BCM_E_PARAM;
        }
        BCM_IF_ERROR_RETURN
            (_bcm_robo_gport_resolve(unit, gport, &modid,
                                     &local_port, &trunk_id, &local_id));

        if (!_num_port_cosq[unit]) {
            return BCM_E_INIT;
        } else if (_num_port_cosq[unit][local_port].numq == 0) {
            return BCM_E_NOT_FOUND;
        }

        BCM_IF_ERROR_RETURN (_bcm_robo_cosq_gport_delete(unit, local_port));
        return BCM_E_NONE;
    }
#endif /* BCM_POLAR_SUPPORT || BCM_NORTHSTAR_SUPPORT */
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_cosq_gport_traverse
 * Purpose:
 *      Walks through the valid COSQ GPORTs and calls
 *      the user supplied callback function for each entry.
 * Parameters:
 *      unit       - (IN) bcm device.
 *      trav_fn    - (IN) Callback function.
 *      user_data  - (IN) User data to be passed to callback function.
 * Returns:
 *      BCM_E_NONE - Success.
 *      BCM_E_XXX
 */
int 
bcm_robo_cosq_gport_traverse(
    int unit, 
    bcm_cosq_gport_traverse_cb cb, 
    void *user_data)
{
#if defined(BCM_POLAR_SUPPORT) || defined(BCM_NORTHSTAR_SUPPORT)
    bcm_port_t port;
    bcm_gport_t gport = 0, sched_gport = 0;
    bcm_module_t  mymodid = 0;    /* module id to construct a gport */
    int  mod_out, port_out; /* To do a modmap mapping */
    uint32 flags = BCM_COSQ_GPORT_SCHEDULER;
    int rv = SOC_E_NONE;
    
    if (SOC_IS_POLAR(unit) || SOC_IS_NORTHSTAR(unit)) {
        if (_num_port_cosq[unit]) {
            BCM_IF_ERROR_RETURN
                (bcm_stk_my_modid_get(unit, &mymodid));

            BCM_PBMP_ITER(PBMP_ALL(unit), port) {
                if (_num_port_cosq[unit][port].numq) {
                    /* Construct physical port GPORT ID */
                    BCM_IF_ERROR_RETURN
                        (bcm_stk_modmap_map(unit, BCM_STK_MODMAP_GET, 
                        mymodid, port, &mod_out, &port_out));

                    /* Construct physical port GPORT ID */
                    BCM_GPORT_MODPORT_SET(gport, mod_out, port_out);

                    /* Construct scheduler GPORT ID */
                    BCM_GPORT_SCHEDULER_SET(sched_gport, port_out);

                    /* Call application call-back */
                    rv = cb(unit, gport, _num_port_cosq[unit][port].numq,
                            flags, sched_gport, user_data);
#ifdef BCM_CB_ABORT_ON_ERR
                    if (BCM_FAILURE(rv) && SOC_CB_ABORT_ON_ERR(unit)) {
                       return rv;
                    }
#endif /* BCM_CB_ABORT_ON_ERR */
                }
            } 
            return rv;
        } else {
            return BCM_E_INIT;
        }
    }
#endif /* BCM_POLAR_SUPPORT || BCM_NORTHSTAR_SUPPORT */
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_cosq_gport_bandwidth_set
 * Purpose:
 *       
 * Parameters:
 *      unit - (IN) Unit number.
 *      gport - (IN) GPORT ID.
 *      cosq - (IN) COS queue to configure, -1 for all COS queues.
 *      kbits_sec_min - (IN) minimum bandwidth, kbits/sec.
 *      kbits_sec_max - (IN) maximum bandwidth, kbits/sec.
 *      flags - (IN) BCM_COSQ_BW_*
 * Returns:
 *      BCM_E_XXX
 */
int 
bcm_robo_cosq_gport_bandwidth_set(
    int unit, 
    bcm_gport_t gport, 
    bcm_cos_queue_t cosq, 
    uint32 kbits_sec_min, 
    uint32 kbits_sec_max, 
    uint32 flags)
{
#if defined(BCM_POLAR_SUPPORT) || defined(BCM_NORTHSTAR_SUPPORT)
    bcm_module_t modid;
    bcm_port_t local_port;
    bcm_trunk_t trunk_id;
    int i;
    int local_id;
    bcm_pbmp_t pbmp;
    uint32 sch2_output_cosq, sch2_num_cosq;
    uint32 reg_value = 0;
    bcm_cos_queue_t local_cos = 0;
    
    if (SOC_IS_POLAR(unit) || SOC_IS_NORTHSTAR(unit)) {        
        BCM_IF_ERROR_RETURN(DRV_DEV_PROP_GET(unit, 
                             DRV_DEV_PROP_SCH2_NUM_COSQ, &sch2_num_cosq));
        BCM_IF_ERROR_RETURN(DRV_DEV_PROP_GET(unit, 
                             DRV_DEV_PROP_SCH2_OUTPUT_COSQ, &sch2_output_cosq));
        BCM_IF_ERROR_RETURN
            (_bcm_robo_gport_resolve(unit, gport, &modid,
                                     &local_port, &trunk_id, &local_id));

        BCM_PBMP_PORT_SET(pbmp, local_port);
        /* The SCH2 COS queues (queue 0 ~3) don't support leaky bucket shaper */
        if (!BCM_GPORT_IS_SCHEDULER(gport)) {
            return BCM_E_UNAVAIL;
        } else {
            if (_num_port_cosq[unit][local_port].numq == 0) {
                return BCM_E_NOT_FOUND;
            } else if (cosq >= _num_port_cosq[unit][local_port].numq) {
                return BCM_E_PARAM;
            } else if (cosq < 0) {
                for (i = 0; i < _num_port_cosq[unit][local_port].numq; i++) {
                    if (i == 0) {
                        local_cos = sch2_output_cosq;
                    } else {
                        local_cos = i + (sch2_num_cosq - 1);
                    }
                    BCM_IF_ERROR_RETURN(DRV_RATE_SET
                            (unit, pbmp, local_cos,
                            DRV_RATE_CONTROL_DIRECTION_EGRESSS_PER_QUEUE,
                            0, kbits_sec_min, kbits_sec_max, kbits_sec_max));
                }
            } else {
                if (cosq == 0) {
                    local_cos = sch2_output_cosq;
                } else {
                    local_cos = cosq + (sch2_num_cosq - 1);
                }
                BCM_IF_ERROR_RETURN(DRV_RATE_SET
                        (unit, pbmp, local_cos,
                        DRV_RATE_CONTROL_DIRECTION_EGRESSS_PER_QUEUE,
                        0, kbits_sec_min, kbits_sec_max, kbits_sec_max));
            }

            /* Enable/disable low queue (queue 0 ~3), queue 4 or 5 AVB shaping mode */
            if (cosq < 0 || local_cos == sch2_output_cosq) {
                BCM_IF_ERROR_RETURN
                    (REG_READ_LOW_QUEUE_AVB_SHAPING_MODEr(unit, &reg_value));
                if (flags & BCM_COSQ_BW_EAV_MODE) {
                    reg_value |= (1 << local_port);
                } else {
                    reg_value &= ~(1 << local_port);
                }
                BCM_IF_ERROR_RETURN
                    (REG_WRITE_LOW_QUEUE_AVB_SHAPING_MODEr(unit, &reg_value));
            }
    
            if (cosq < 0 || local_cos == sch2_num_cosq) {
                BCM_IF_ERROR_RETURN
                    (REG_READ_QUEUE4_AVB_SHAPING_MODEr(unit, &reg_value));
                if (flags & BCM_COSQ_BW_EAV_MODE) {
                    reg_value |= (1 << local_port);
                } else {
                    reg_value &= ~(1 << local_port);
                }
                BCM_IF_ERROR_RETURN
                    (REG_WRITE_QUEUE4_AVB_SHAPING_MODEr(unit, &reg_value));
            }
    
            if (cosq < 0 || local_cos == (sch2_num_cosq + 1)) {
                BCM_IF_ERROR_RETURN
                    (REG_READ_QUEUE5_AVB_SHAPING_MODEr(unit, &reg_value));
                if (flags & BCM_COSQ_BW_EAV_MODE) {
                    reg_value |= (1 << local_port);
                } else {
                    reg_value &= ~(1 << local_port);
                }
                BCM_IF_ERROR_RETURN
                    (REG_WRITE_QUEUE5_AVB_SHAPING_MODEr(unit, &reg_value));
            }
        }
        return BCM_E_NONE;
    }
#endif /* BCM_POLAR_SUPPORT || BCM_NORTHSTAR_SUPPORT */
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_cosq_gport_bandwidth_get
 * Purpose:
 *       
 * Parameters:
 *      unit - (IN) Unit number.
 *      gport - (IN) GPORT ID.
 *      cosq - (IN) COS queue to configure, -1 for all COS queues.
 *      kbits_sec_min - (OUT) minimum bandwidth, kbits/sec.
 *      kbits_sec_max - (OUT) maximum bandwidth, kbits/sec.
 *      flags - (OUT) BCM_COSQ_BW_*
 * Returns:
 *      BCM_E_XXX
 */
int 
bcm_robo_cosq_gport_bandwidth_get(
    int unit, 
    bcm_gport_t gport, 
    bcm_cos_queue_t cosq, 
    uint32 *kbits_sec_min, 
    uint32 *kbits_sec_max, 
    uint32 *flags)
{
#if defined(BCM_POLAR_SUPPORT) || defined(BCM_NORTHSTAR_SUPPORT)
        bcm_module_t modid;
        bcm_port_t local_port;
        bcm_trunk_t trunk_id;
        int local_id;
        bcm_cos_queue_t local_cos;
        uint32 kbits_sec_burst;    /* Dummy variable */
        uint32 sch2_output_cosq, sch2_num_cosq;
        uint32 reg_value = 0;
		uint32 rate_flags = 0;

        if (SOC_IS_POLAR(unit) || SOC_IS_NORTHSTAR(unit)) {        
            BCM_IF_ERROR_RETURN(DRV_DEV_PROP_GET(unit, 
                             DRV_DEV_PROP_SCH2_NUM_COSQ, &sch2_num_cosq));
            BCM_IF_ERROR_RETURN(DRV_DEV_PROP_GET(unit, 
                             DRV_DEV_PROP_SCH2_OUTPUT_COSQ, &sch2_output_cosq));
            BCM_IF_ERROR_RETURN
                (_bcm_robo_gport_resolve(unit, gport, &modid,
                                         &local_port, &trunk_id, &local_id));

            /* The SCH2 COS queues (queue 0 ~3) don't support leaky bucket shaper */
            if (!BCM_GPORT_IS_SCHEDULER(gport)) {
                return BCM_E_UNAVAIL;
            } else {
                if (_num_port_cosq[unit][local_port].numq == 0) {
                    return BCM_E_NOT_FOUND;
                } else if (cosq >= _num_port_cosq[unit][local_port].numq) {
                    return BCM_E_PARAM;
                } else {
                    if (cosq <= 0) {
                        local_cos = sch2_output_cosq;
                    } else {
                        local_cos = cosq + (sch2_num_cosq - 1);
                    }
                    BCM_IF_ERROR_RETURN(DRV_RATE_GET
                            (unit, local_port, local_cos,
                            DRV_RATE_CONTROL_DIRECTION_EGRESSS_PER_QUEUE,
                            &rate_flags, kbits_sec_min, kbits_sec_max, &kbits_sec_burst));

                    *flags = 0;
                    if (local_cos == sch2_output_cosq) {
                        BCM_IF_ERROR_RETURN
                            (REG_READ_LOW_QUEUE_AVB_SHAPING_MODEr
                            (unit, &reg_value));
                        if (reg_value & (1 << local_port)) {
                            *flags |= BCM_COSQ_BW_EAV_MODE;
                        }
                    }
            
                    if (local_cos == sch2_num_cosq) {
                        BCM_IF_ERROR_RETURN
                            (REG_READ_QUEUE4_AVB_SHAPING_MODEr
                            (unit, &reg_value));
                        if (reg_value & (1 << local_port)) {
                            *flags |= BCM_COSQ_BW_EAV_MODE;
                        }
                    }
            
                    if (local_cos == (sch2_num_cosq + 1)) {
                        BCM_IF_ERROR_RETURN
                            (REG_READ_QUEUE5_AVB_SHAPING_MODEr
                            (unit, &reg_value));
                        if (reg_value & (1 << local_port)) {
                            *flags |= BCM_COSQ_BW_EAV_MODE;
                        }
                    }
                }
            }
            return BCM_E_NONE;
        }
#endif /* BCM_POLAR_SUPPORT || BCM_NORTHSTAR_SUPPORT */

    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_cosq_gport_sched_set
 * Purpose:
 *      
 * Parameters:
 *      unit - (IN) Unit number.
 *      gport - (IN) GPORT ID.
 *      cosq - (IN) COS queue to configure, -1 for all COS queues.
 *      mode - (IN) Scheduling mode, one of BCM_COSQ_xxx
 *	weight - (IN) Weight for the specified COS queue(s)
 *               Unused if mode is BCM_COSQ_STRICT.
 * Returns:
 *      BCM_E_XXX
 */
int 
bcm_robo_cosq_gport_sched_set(
    int unit, 
    bcm_gport_t gport, 
    bcm_cos_queue_t cosq, 
    int mode, 
    int weight)
{
#if defined(BCM_POLAR_SUPPORT) || defined(BCM_NORTHSTAR_SUPPORT)
    bcm_module_t modid;
    bcm_port_t local_port;
    bcm_trunk_t trunk_id;
    int i;
    bcm_cos_queue_t cosq_start = 0;
    int num_weights = 1, weights[BCM_COS_COUNT];
    int local_id;
    uint32 flag = 0;
    uint32 drv_value, weight_value;
    bcm_pbmp_t pbmp;
    int sp_num;
    uint32 sch2_output_cosq, sch2_num_cosq, sch1_num_cosq;

    if (SOC_IS_POLAR(unit) || SOC_IS_NORTHSTAR(unit)) {        
        BCM_IF_ERROR_RETURN(DRV_DEV_PROP_GET(unit, 
                             DRV_DEV_PROP_SCH1_NUM_COSQ, &sch1_num_cosq));
        BCM_IF_ERROR_RETURN(DRV_DEV_PROP_GET(unit, 
                             DRV_DEV_PROP_SCH2_NUM_COSQ, &sch2_num_cosq));
        BCM_IF_ERROR_RETURN(DRV_DEV_PROP_GET(unit, 
                             DRV_DEV_PROP_SCH2_OUTPUT_COSQ, &sch2_output_cosq));
        BCM_IF_ERROR_RETURN
            (_bcm_robo_gport_resolve(unit, gport, &modid,
                                     &local_port, &trunk_id, &local_id));

        if (!_num_port_cosq[unit]) {
            return BCM_E_INIT;
        }
        if (BCM_GPORT_IS_SCHEDULER(gport)) {
            if (_num_port_cosq[unit][local_port].numq == 0) {
                return BCM_E_NOT_FOUND;
            } else if (cosq >= _num_port_cosq[unit][local_port].numq) {
                return BCM_E_PARAM;
            } else if (cosq < 0) {
                cosq_start = 0;
                num_weights = sch1_num_cosq;
                for (i = 0; i < num_weights; i++) {
                    weights[i] = weight;
                }
            } else {
                cosq_start = cosq;
                num_weights = 1;
                weights[0] = weight;
            }
        } else {
            if (cosq >= (int)sch2_num_cosq) {
                return BCM_E_PARAM;
            } else if (cosq < 0) {
                cosq_start = 0;
                num_weights = sch2_num_cosq;
                for (i = 0; i < num_weights; i++) {
                    weights[i] = weight;
                }
            } else {
                cosq_start = cosq;
                num_weights = 1;
                weights[0] = weight;
            }
            flag = DRV_QUEUE_FLAG_LEVLE2;
        }
        
        /* for Robo Chip, we support Strict, WRR and WDRR mode */
        drv_value = (mode == BCM_COSQ_STRICT) ? 
                    DRV_QUEUE_MODE_STRICT : 
                    (mode == BCM_COSQ_WEIGHTED_ROUND_ROBIN) ? 
                    DRV_QUEUE_MODE_WRR : 0;
        
        /* the COSQ mode for Robo allowed Strict & WRR mode only */
        if (drv_value == 0) {
            return BCM_E_PARAM;
        }

        BCM_PBMP_PORT_SET(pbmp, local_port);
        
        if (mode == BCM_COSQ_STRICT) {
            BCM_IF_ERROR_RETURN(DRV_QUEUE_MODE_SET
                            (unit, pbmp, flag, drv_value));
            return BCM_E_NONE;
        } else {
            /* set the weight if mode is WRR */
            if (num_weights == 1) {
                weight_value = weights[0];
                if(!flag) {
                    if(cosq_start == 0) {
                        cosq = sch2_output_cosq;
                    } else {
                        cosq = cosq_start + (sch2_num_cosq - 1);
                    }
                } 
                BCM_IF_ERROR_RETURN(DRV_QUEUE_WRR_WEIGHT_SET
                                    (unit, 0, pbmp, cosq, weight_value));
            } else {
                sp_num = 0;
                if(flag & DRV_QUEUE_FLAG_LEVLE2) {
                    for (i = 0; i < num_weights; i++) {
                        weight_value = weights[i];
                        if (((i == num_weights - 1) && 
                              (weight_value == BCM_COSQ_WEIGHT_STRICT)) ||
                            ((i == num_weights - 2) && 
                             (weight_value == BCM_COSQ_WEIGHT_STRICT))) {
                            sp_num ++;
                        } else {
                            BCM_IF_ERROR_RETURN(DRV_QUEUE_WRR_WEIGHT_SET
                                        (unit, 0, pbmp, i, weight_value));
                        }
                    }
                
                    /* Set scheduling combination mode if ROBO chip support it */
                    if (sp_num == 1) { 
                        /* 1STRICT/2WRR : COSQ3>COS2/COSQ1/COSQ */
                        drv_value = DRV_QUEUE_MODE_1STRICT;
                    } else if (sp_num == 2) { 
                        /* 1STRICT/2WRR : COSQ3>COS2> COSQ1/COSQ0 */
                        drv_value = DRV_QUEUE_MODE_2STRICT;
                    } 
                } else { 
                    for (i = 0; i < num_weights; i++) {
                        weight_value = weights[i];
                        if ((i == num_weights - 1) && 
                            (weight_value == BCM_COSQ_WEIGHT_STRICT)) {
                            sp_num ++;
                        } else {
                            if (i == 0) {
                                cosq = sch2_output_cosq;
                            } else {
                                cosq = i + (sch2_num_cosq - 1);
                            }                            
                            BCM_IF_ERROR_RETURN(DRV_QUEUE_WRR_WEIGHT_SET
                                        (unit, 0, pbmp, cosq, weight_value));
                        }
                    }
                
                    /* Set scheduling combination mode if ROBO chip support it */
                    if (sp_num == 1) { 
                        /* 1STRICT/2WRR : COSQ5>COS4/output of first level scheduling */
                        drv_value = DRV_QUEUE_MODE_1STRICT;
                    }
                }
                BCM_IF_ERROR_RETURN(DRV_QUEUE_MODE_SET
                                (unit, pbmp, flag, drv_value));
            }
        }
        return BCM_E_NONE;
    }
#endif /* BCM_POLAR_SUPPORT || BCM_NORTHSTAR_SUPPORT */
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_cosq_gport_sched_get
 * Purpose:
 *      
 * Parameters:
 *      unit - (IN) Unit number.
 *      gport - (IN) GPORT ID.
 *      cosq - (IN) COS queue
 *      mode - (OUT) Scheduling mode, one of BCM_COSQ_xxx
 *	weight - (OUT) Weight for the specified COS queue(s)
 *               Unused if mode is BCM_COSQ_STRICT.
 * Returns:
 *      BCM_E_XXX
 */
int 
bcm_robo_cosq_gport_sched_get(
    int unit, 
    bcm_gport_t gport, 
    bcm_cos_queue_t cosq, 
    int *mode, 
    int *weight)
{
#if defined(BCM_POLAR_SUPPORT) || defined(BCM_NORTHSTAR_SUPPORT)
    bcm_module_t modid;
    bcm_port_t local_port;
    bcm_trunk_t trunk_id;
    int local_id;
    uint32 flag = 0;
    uint32 drv_value = 0;
    uint32 sch2_output_cosq, sch2_num_cosq;

    if (SOC_IS_POLAR(unit) || SOC_IS_NORTHSTAR(unit)) {
        *mode = *weight = 0;
        
        BCM_IF_ERROR_RETURN(DRV_DEV_PROP_GET(unit, 
                             DRV_DEV_PROP_SCH2_NUM_COSQ, &sch2_num_cosq));
        BCM_IF_ERROR_RETURN(DRV_DEV_PROP_GET(unit, 
                             DRV_DEV_PROP_SCH2_OUTPUT_COSQ, &sch2_output_cosq));
        BCM_IF_ERROR_RETURN
            (_bcm_robo_gport_resolve(unit, gport, &modid,
                                     &local_port, &trunk_id, &local_id));
        if (!_num_port_cosq[unit]) {
            return BCM_E_INIT;
        }
        if (BCM_GPORT_IS_SCHEDULER(gport)) {
            if (_num_port_cosq[unit][local_port].numq == 0) {
                return BCM_E_NOT_FOUND;
            } else if (cosq >= _num_port_cosq[unit][local_port].numq) {
                return BCM_E_PARAM;
            } else if (cosq <= 0) {
                cosq = sch2_output_cosq;
            } else {
                cosq = cosq + (sch2_num_cosq - 1);
            }
        } else {
            if (cosq >= (int)sch2_num_cosq) {
                return BCM_E_PARAM;
            } else if (cosq < 0) {
                cosq = 0;
            }
            flag = DRV_QUEUE_FLAG_LEVLE2;
        }

        /* get the cosq schedule at the first port only in pbm */
        BCM_IF_ERROR_RETURN(DRV_QUEUE_MODE_GET
                            (unit, local_port, flag, &drv_value));
                
        /* Robo Chip shuld have strict, WRR or WDRR mode only */
        *mode = (drv_value == DRV_QUEUE_MODE_STRICT) ? 
                BCM_COSQ_STRICT : 
                (drv_value == DRV_QUEUE_MODE_1STRICT) ? 
                BCM_COSQ_WEIGHTED_ROUND_ROBIN :
                (drv_value == DRV_QUEUE_MODE_2STRICT) ? 
                BCM_COSQ_WEIGHTED_ROUND_ROBIN :
                (drv_value == DRV_QUEUE_MODE_WRR) ? 
                BCM_COSQ_WEIGHTED_ROUND_ROBIN : -1;
        
        if (*mode == -1) {
            return BCM_E_INTERNAL;
        }

        /* get the weight if mode is WRR or WDRR */
        BCM_IF_ERROR_RETURN(DRV_QUEUE_WRR_WEIGHT_GET
                        (unit, 0, local_port, cosq, &drv_value));
                        
        *weight = drv_value;
        return BCM_E_NONE;
    }
#endif /* BCM_POLAR_SUPPORT || BCM_NORTHSTAR_SUPPORT */
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_cosq_gport_attach
 * Purpose:
 *      
 * Parameters:
 *      unit       - (IN) Unit number.
 *      sched_port - (IN) Scheduler GPORT ID.
 *      input_port - (IN) GPORT to attach to.
 *      cosq       - (IN) COS queue to attach to.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_robo_cosq_gport_attach(
    int unit, 
    bcm_gport_t sched_gport, 
    bcm_gport_t input_gport, 
    bcm_cos_queue_t cosq)
{
#if defined(BCM_POLAR_SUPPORT) || defined(BCM_NORTHSTAR_SUPPORT)
    bcm_module_t sched_modid, input_modid;
    bcm_port_t sched_port, input_port;
    bcm_trunk_t trunk_id;
    int local_id;
    uint32 sch2_num_cosq, sch2_output_cosq;

    if (SOC_IS_POLAR(unit) || SOC_IS_NORTHSTAR(unit)) {
        BCM_IF_ERROR_RETURN(DRV_DEV_PROP_GET(unit, 
                             DRV_DEV_PROP_SCH2_NUM_COSQ, &sch2_num_cosq));
        BCM_IF_ERROR_RETURN(DRV_DEV_PROP_GET(unit, 
                             DRV_DEV_PROP_SCH2_OUTPUT_COSQ, &sch2_output_cosq));

        if (sched_gport == BCM_GPORT_INVALID) {
            return BCM_E_PORT;
        }

        if (!BCM_GPORT_IS_SCHEDULER(sched_gport)) {
            return BCM_E_PARAM;
        } else if (!_num_port_cosq[unit]) {
            return BCM_E_INIT;
        } else if (cosq >= 0 && (cosq != sch2_output_cosq) && 
            (cosq != sch2_num_cosq) && (cosq != (sch2_num_cosq + 1))) {
            return BCM_E_PARAM;
        }

        BCM_IF_ERROR_RETURN
            (_bcm_robo_gport_resolve(unit, sched_gport, &sched_modid,
                                     &sched_port, &trunk_id, &local_id));

        BCM_IF_ERROR_RETURN
            (_bcm_robo_gport_resolve(unit, input_gport, &input_modid,
                                     &input_port, &trunk_id, &local_id));

        if (_num_port_cosq[unit][sched_port].numq == 0) {
            /* GPORT has not been added. */
            return BCM_E_NOT_FOUND;
        } else if ((sched_modid != input_modid) || (sched_port != input_port)) {
            /* Be sure the GPORT matches to the physical port */
            return BCM_E_PARAM;
        }

        if (_num_port_cosq[unit][sched_port].member_queues == NULL) {
            _num_port_cosq[unit][sched_port].member_queues = 
                sal_alloc(sizeof(uint8) * NUM_COS(unit), 
                "_num_port_cosq_members");
            if (_num_port_cosq[unit][sched_port].member_queues == NULL) {
                sal_free(_num_port_cosq[unit][sched_port].member_queues);
                return BCM_E_MEMORY;
            }
            sal_memset(_num_port_cosq[unit][sched_port].member_queues, 0, 
                sizeof(uint8) * NUM_COS(unit));
        }

        if (_num_port_cosq[unit][sched_port].member_queues[cosq - 1] == 0) {
            _num_port_cosq[unit][sched_port].member_queues[cosq - 1] = 1;
        }

        return BCM_E_NONE;
    }
#endif /* BCM_POLAR_SUPPORT || BCM_NORTHSTAR_SUPPORT */
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_cosq_gport_detach
 * Purpose:
 *
 * Parameters:
 *      unit       - (IN) Unit number.
 *      sched_port - (IN) Scheduler GPORT ID.
 *      input_port - (IN) GPORT to detach from.
 *      cosq       - (IN) COS queue to detach from.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_robo_cosq_gport_detach(
    int unit, 
    bcm_gport_t sched_gport, 
    bcm_gport_t input_gport, 
    bcm_cos_queue_t cosq)
{
#if defined(BCM_POLAR_SUPPORT) || defined(BCM_NORTHSTAR_SUPPORT)
    bcm_module_t sched_modid, input_modid;
    bcm_port_t sched_port, input_port;
    bcm_trunk_t trunk_id;
    int local_id;
    uint32 sch2_num_cosq, sch2_output_cosq;

    if (SOC_IS_POLAR(unit) || SOC_IS_NORTHSTAR(unit)) {
        BCM_IF_ERROR_RETURN(DRV_DEV_PROP_GET(unit, 
                             DRV_DEV_PROP_SCH2_NUM_COSQ, &sch2_num_cosq));
        BCM_IF_ERROR_RETURN(DRV_DEV_PROP_GET(unit, 
                             DRV_DEV_PROP_SCH2_OUTPUT_COSQ, &sch2_output_cosq));

        if (sched_gport == BCM_GPORT_INVALID) {
            return BCM_E_PORT;
        }

        if (!BCM_GPORT_IS_SCHEDULER(sched_gport)) {
            return BCM_E_PORT;
        } else if (!_num_port_cosq[unit]) {
            return BCM_E_INIT;
        } else if (cosq >= 0 && (cosq != sch2_output_cosq) && 
            (cosq != sch2_num_cosq) && (cosq != (sch2_num_cosq + 1))) {
            return BCM_E_NOT_FOUND;
        }

        BCM_IF_ERROR_RETURN
            (_bcm_robo_gport_resolve(unit, sched_gport, &sched_modid,
                                     &sched_port, &trunk_id, &local_id));

        BCM_IF_ERROR_RETURN
            (_bcm_robo_gport_resolve(unit, input_gport, &input_modid,
                                     &input_port, &trunk_id, &local_id));

        if (_num_port_cosq[unit][sched_port].numq == 0) {
            /* GPORT has not been added. */
            return BCM_E_NOT_FOUND;
        } else if ((sched_modid != input_modid) || (sched_port != input_port)) {
            /* Be sure the GPORT matches to the physical port */
            return BCM_E_PORT;
        }

        if (_num_port_cosq[unit][sched_port].member_queues == NULL) {
            /* GPORT has not been attached. */
            return BCM_E_NOT_FOUND;
        } else {
            _num_port_cosq[unit][sched_port].member_queues[cosq - 1] = 0;
        }
        return BCM_E_NONE;
    }
#endif /* BCM_POLAR_SUPPORT || BCM_NORTHSTAR_SUPPORT */
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_cosq_gport_attach_get
 * Purpose:
 *
 * Parameters:
 *      unit       - (IN) Unit number.
 *      sched_port - (IN) Scheduler GPORT ID.
 *      input_port - (OUT) GPORT attached to.
 *      cosq       - (OUT) COS queue attached to.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_robo_cosq_gport_attach_get(
    int unit, 
    bcm_gport_t sched_gport, 
    bcm_gport_t *input_gport, 
    bcm_cos_queue_t *cosq)
{
#if defined(BCM_POLAR_SUPPORT) || defined(BCM_NORTHSTAR_SUPPORT)
    bcm_module_t sched_modid;
    bcm_port_t sched_port;
    bcm_trunk_t trunk_id;
    int local_id, i;
    uint32 sch2_num_cosq, sch2_output_cosq;
    bcm_cos_queue_t local_cos = 0;

    if (SOC_IS_POLAR(unit) || SOC_IS_NORTHSTAR(unit)) {
        BCM_IF_ERROR_RETURN(DRV_DEV_PROP_GET(unit, 
                             DRV_DEV_PROP_SCH2_NUM_COSQ, &sch2_num_cosq));
        BCM_IF_ERROR_RETURN(DRV_DEV_PROP_GET(unit, 
                             DRV_DEV_PROP_SCH2_OUTPUT_COSQ, &sch2_output_cosq));

        if (sched_gport == BCM_GPORT_INVALID) {
            return BCM_E_PORT;
        }
        
        if (!BCM_GPORT_IS_SCHEDULER(sched_gport) ||
            !cosq || !input_gport) {
            return BCM_E_PARAM;
        } else if (!_num_port_cosq[unit]) {
            return BCM_E_INIT;
        }

        BCM_IF_ERROR_RETURN
            (_bcm_robo_gport_resolve(unit, sched_gport, &sched_modid,
                                     &sched_port, &trunk_id, &local_id));

        if (_num_port_cosq[unit][sched_port].numq == 0) {
            /* GPORT has not been added. */
            return BCM_E_NOT_FOUND;
        } 

        if (_num_port_cosq[unit][sched_port].member_queues == NULL) {
            /* GPORT has not been attached. */
            return BCM_E_NOT_FOUND;
        }

        for (i = 0; i < _num_port_cosq[unit][sched_port].numq; i++) {
            if (i == 0) {
                local_cos = sch2_output_cosq;
            } else {
                local_cos = i + (sch2_num_cosq - 1);
            }

            if (_num_port_cosq[unit][sched_port].member_queues[local_cos - 1]) {
                *cosq = local_cos;
                BCM_GPORT_MODPORT_SET(*input_gport, sched_modid, sched_port);
                return BCM_E_NONE;
            }
        }

        return BCM_E_NOT_FOUND;
    }
#endif /* BCM_POLAR_SUPPORT || BCM_NORTHSTAR_SUPPORT */
    return BCM_E_UNAVAIL;

}

/*
 * Function:
 *      bcm_cosq_gport_get
 * Returns:
 *      BCM_E_XXX
 */
int 
bcm_robo_cosq_gport_get(
    int unit, 
    bcm_gport_t gport, 
    bcm_gport_t *physical_port, 
    int *num_cos_levels, 
    uint32 *flags)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_cosq_gport_size_set
 * Returns:
 *      BCM_E_XXX
 */
int 
bcm_robo_cosq_gport_size_set(
    int unit, 
    bcm_gport_t gport, 
    bcm_cos_queue_t cosq, 
    uint32 bytes_min, 
    uint32 bytes_max)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_cosq_gport_size_get
 * Returns:
 *      BCM_E_XXX
 */
int 
bcm_robo_cosq_gport_size_get(
    int unit, 
    bcm_gport_t gport, 
    bcm_cos_queue_t cosq, 
    uint32 *bytes_min, 
    uint32 *bytes_max)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_cosq_gport_enable_set
 * Returns:
 *      BCM_E_XXX
 */
int 
bcm_robo_cosq_gport_enable_set(
    int unit, 
    bcm_gport_t gport, 
    bcm_cos_queue_t cosq, 
    int enable)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_cosq_gport_enable_get
 * Returns:
 *      BCM_E_XXX
 */
int 
bcm_robo_cosq_gport_enable_get(
    int unit, 
    bcm_gport_t gport, 
    bcm_cos_queue_t cosq, 
    int *enable)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_cosq_gport_stat_enable_set
 * Purpose:
 *      ?
 * Returns:
 *      BCM_E_XXX
 */
int 
bcm_robo_cosq_gport_stat_enable_set(
    int unit, 
    bcm_gport_t gport, 
    int enable)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_cosq_gport_stat_enable_get
 * Returns:
 *      BCM_E_XXX
 */
int 
bcm_robo_cosq_gport_stat_enable_get(
    int unit, 
    bcm_gport_t gport, 
    int *enable)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_cosq_gport_stat_set
 * Returns:
 *      BCM_E_XXX
 */
int 
bcm_robo_cosq_gport_stat_set(
    int unit, 
    bcm_gport_t gport, 
    bcm_cos_queue_t cosq, 
    bcm_cosq_gport_stats_t stat, 
    uint64 value)
{
    bcm_port_t              loc_port;
    drv_wred_counter_t      type;


    BCM_IF_ERROR_RETURN(
        _bcm_robo_port_gport_validate(unit, gport, &loc_port));

    switch (stat) {
        case bcmCosqGportDiscardDroppedPkts:
            type = DRV_WRED_COUNTER_DROP_PACKETS;
            break;
        case bcmCosqGportDiscardDroppedBytes:
            type = DRV_WRED_COUNTER_DROP_BYTES;
            break;
        default:
            return BCM_E_UNAVAIL;
    }
    BCM_IF_ERROR_RETURN(
        DRV_WRED_COUNTER_SET(unit, loc_port, cosq, type, value));

    return BCM_E_NONE;    
}

/*
 * Function:
 *      bcm_cosq_gport_stat_get
 * Returns:
 *      BCM_E_XXX
 */
int 
bcm_robo_cosq_gport_stat_get(
    int unit, 
    bcm_gport_t gport, 
    bcm_cos_queue_t cosq, 
    bcm_cosq_gport_stats_t stat, 
    uint64 *value)
{
    bcm_port_t              loc_port;
    drv_wred_counter_t      type;


    BCM_IF_ERROR_RETURN(
        _bcm_robo_port_gport_validate(unit, gport, &loc_port));

    switch (stat) {
        case bcmCosqGportDiscardDroppedPkts:
            type = DRV_WRED_COUNTER_DROP_PACKETS;
            break;
        case bcmCosqGportDiscardDroppedBytes:
            type = DRV_WRED_COUNTER_DROP_BYTES;
            break;
        default:
            return BCM_E_UNAVAIL;
    }
    BCM_IF_ERROR_RETURN(
        DRV_WRED_COUNTER_GET(unit, loc_port, cosq, type, value));

    return BCM_E_NONE;
}

/*
 * Function:
 *      bcm_esw_cosq_gport_destmod_attach
 * Purpose:
 *      Attach gport mapping from ingress port, dest_modid to
 *      fabric egress port.
 * Parameters:
 *      unit            - (IN) Unit number.
 *      gport           - (IN) GPORT ID
 *      ingress_port    - (IN) Ingress port
 *      dest_modid      - (IN) Destination module ID
 *      fabric_egress_port - (IN) Port number on fabric that 
 *                             is connected to dest_modid
 * Returns:
 *      BCM_E_xxx
 * Notes:
 */
int 
bcm_robo_cosq_gport_destmod_attach(
    int unit, 
    bcm_gport_t gport, 
    bcm_port_t ingress_port, 
    bcm_module_t dest_modid, 
    int fabric_egress_port)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_esw_cosq_gport_destmod_detach
 * Purpose:
 *      Attach gport mapping from ingress port, dest_modid to
 *      fabric egress port.
 * Parameters:
 *      unit            - (IN) Unit number.
 *      gport           - (IN) GPORT ID
 *      ingress_port    - (IN) Ingress port
 *      dest_modid      - (IN) Destination module ID
 *      fabric_egress_port - (IN) Port number on fabric that 
 *                             is connected to dest_modid
 * Returns:
 *      BCM_E_xxx
 * Notes:
 */
int 
bcm_robo_cosq_gport_destmod_detach(
    int unit, 
    bcm_gport_t gport, 
    bcm_port_t ingress_port, 
    bcm_module_t dest_modid, 
    int fabric_egress_port)
{
    return BCM_E_UNAVAIL;
}


int 
bcm_robo_cosq_gport_congestion_config_set(int unit, bcm_gport_t gport, 
                                         bcm_cos_queue_t cosq, 
                                         bcm_cosq_congestion_info_t *config)
{
    return BCM_E_UNAVAIL;
}

int 
bcm_robo_cosq_gport_congestion_config_get(int unit, bcm_gport_t gport, 
                                         bcm_cos_queue_t cosq, 
                                         bcm_cosq_congestion_info_t *config)
{
    return BCM_E_UNAVAIL;
}


int 
bcm_robo_cosq_congestion_mapping_set(
    int unit, 
    int fabric_modid, 
    bcm_cosq_congestion_mapping_info_t *mapping_info)
{

    return BCM_E_UNAVAIL;
}

int 
bcm_robo_cosq_congestion_mapping_get(
    int unit, 
    int fabric_modid, 
    bcm_cosq_congestion_mapping_info_t *mapping_info)
{

    return BCM_E_UNAVAIL;
}

int
bcm_robo_cosq_gport_egress_mapping_set(
        int unit, bcm_port_t gport, bcm_cos_t priority,
        bcm_cos_t cosq, uint32 flags)
{
    return BCM_E_UNAVAIL;
}
int
bcm_robo_cosq_gport_egress_mapping_get(
        int unit, bcm_port_t gport, bcm_cos_t *priority,
        bcm_cos_t *cosq, uint32 flags)
{
    return BCM_E_UNAVAIL;
}
/*
 * Function:
 *      bcm_cosq_gport_discard_set
 * Purpose:
 *
 * Parameters:
 *      unit    - (IN) Unit number.
 *      port    - (IN) GPORT ID.
 *      cosq    - (IN) COS queue to configure
 *      discard - (IN) Discard settings
 * Returns:
 *      BCM_E_XXX
 */
int 
bcm_robo_cosq_gport_discard_set(
    int unit, 
    bcm_gport_t gport, 
    bcm_cos_queue_t cosq, 
    bcm_cosq_gport_discard_t *discard)
{
#if defined(BCM_NORTHSTARPLUS_SUPPORT) || defined(BCM_STARFIGHTER3_SUPPORT)
    bcm_port_t  loc_port = 0;
    bcm_cos_queue_t queue = 0;
    uint32  flags = 0;
    int rv = BCM_E_NONE, wred_idx = 0;
    drv_wred_config_t drv_config;
    drv_wred_map_info_t drv_map;
    
    if (!(SOC_IS_NORTHSTARPLUS(unit) || SOC_IS_STARFIGHTER3(unit))) {
        return BCM_E_UNAVAIL;
    }

    if (discard == NULL) {
        return BCM_E_PARAM;
    }

    if (discard->flags & ~(BCM_COSQ_DISCARD_ENABLE |
                BCM_COSQ_DISCARD_CAP_AVERAGE |
                BCM_COSQ_DISCARD_NONTCP | BCM_COSQ_DISCARD_COLOR_GREEN |
                BCM_COSQ_DISCARD_COLOR_YELLOW | BCM_COSQ_DISCARD_COLOR_RED |
                BCM_COSQ_DISCARD_COLOR_ALL | BCM_COSQ_DISCARD_IFP |
                BCM_COSQ_DISCARD_OUTER_CFI | BCM_COSQ_DISCARD_PORT |
                BCM_COSQ_DISCARD_DEVICE)) {
        return BCM_E_UNAVAIL;
    }

    if (discard->flags & BCM_COSQ_DISCARD_IFP) {
        /* System-wide, default WRED configuration 
         * For field action to override the original settings
         */
        flags = DRV_WRED_CONFIG_FLAGS_DEFAULT_PROFILE;
    } else {
        if (gport == BCM_GPORT_INVALID) { 
            return (BCM_E_PORT);
        }
     
        if ((discard->gain < 0) || (discard->gain > 15) ||
            (discard->drop_probability < 0) || (discard->drop_probability > 100)) {
            return BCM_E_PARAM;
        } 

        if (BCM_GPORT_IS_SET(gport)) {
            if ((BCM_GPORT_MODPORT_MODID_GET(gport) == _SHR_GPORT_MODID_MASK) && 
                (BCM_GPORT_MODPORT_PORT_GET(gport) == _SHR_GPORT_PORT_MASK)) {
                /* System configuration for gport type */ 
                loc_port = -1;
            } else { 
                BCM_IF_ERROR_RETURN(bcm_port_local_get(unit, gport, &loc_port)); 
            }
        } else {
            loc_port = gport;
        } 

        if (loc_port != -1 && !SOC_PORT_VALID(unit, loc_port)) {
            return BCM_E_PORT;
        } 
        
        if (discard->flags & BCM_COSQ_DISCARD_PORT) {
            queue = -1;
        } else {
            if (cosq < -1) {
                return BCM_E_PARAM;
            } else if (cosq != -1) {
                if (cosq > NUM_COS(unit)) {
                    return BCM_E_PARAM;
                }
            }
            queue = cosq;
        }

        if (discard->flags & BCM_COSQ_DISCARD_DEVICE) {
            loc_port = -1;
    }
    }

    if (discard->flags & BCM_COSQ_DISCARD_ENABLE) {
        /* Create WRED profile */
        sal_memset(&drv_config, 0, sizeof(drv_wred_config_t));
        drv_config.drop_prob = discard->drop_probability;
        drv_config.max_threshold = discard->max_thresh;
        drv_config.min_threshold = discard->min_thresh;
        drv_config.gain = discard->gain; 
        rv = DRV_WRED_CONFIG_CREATE(unit, flags, &drv_config, &wred_idx); 
        if ((rv != BCM_E_NONE) && (rv != BCM_E_EXISTS)) {
            return rv;
        }

        /* Configure the mapping table */
        if (flags & DRV_WRED_CONFIG_FLAGS_DEFAULT_PROFILE) {
            /* No need to set the mapping table */
            drv_config.flags |= DRV_WRED_CONFIG_FLAGS_DEFAULT_PROFILE;
            rv = DRV_WRED_CONFIG_SET(unit, wred_idx, &drv_config);
            return BCM_E_NONE;
        } else {
            sal_memset(&drv_map, 0, sizeof(drv_wred_map_info_t));
            drv_map.port = loc_port;
            drv_map.cosq = queue;
            if (discard->flags & BCM_COSQ_DISCARD_COLOR_RED) {
                drv_map.flags |= DRV_WRED_MAP_FLAGS_COLOR_RED; 
            }
            if (discard->flags & BCM_COSQ_DISCARD_COLOR_YELLOW) {
                drv_map.flags |= DRV_WRED_MAP_FLAGS_COLOR_YELLOW; 
            }
            if (discard->flags & BCM_COSQ_DISCARD_COLOR_GREEN) {
                drv_map.flags |= DRV_WRED_MAP_FLAGS_COLOR_GREEN; 
            }
            if (discard->flags & BCM_COSQ_DISCARD_OUTER_CFI) {
                drv_map.flags |= DRV_WRED_MAP_FLAGS_DEI; 
            }
            BCM_IF_ERROR_RETURN(
                DRV_WRED_MAP_ATTACH(unit, wred_idx, &drv_map)); 
        }
    } else {

        sal_memset(&drv_map, 0, sizeof(drv_wred_map_info_t));
        if (!(flags & DRV_WRED_CONFIG_FLAGS_DEFAULT_PROFILE)) {
            
            drv_map.port = loc_port;
            drv_map.cosq = queue;
            if (discard->flags & BCM_COSQ_DISCARD_COLOR_RED) {
                drv_map.flags |= DRV_WRED_MAP_FLAGS_COLOR_RED; 
            }
            if (discard->flags & BCM_COSQ_DISCARD_COLOR_YELLOW) {
                drv_map.flags |= DRV_WRED_MAP_FLAGS_COLOR_YELLOW; 
            }
            if (discard->flags & BCM_COSQ_DISCARD_COLOR_GREEN) {
                drv_map.flags |= DRV_WRED_MAP_FLAGS_COLOR_GREEN; 
            }
            if (discard->flags & BCM_COSQ_DISCARD_OUTER_CFI) {
                drv_map.flags |= DRV_WRED_MAP_FLAGS_DEI; 
            }
            BCM_IF_ERROR_RETURN(
                DRV_WRED_MAP_GET(unit, &wred_idx, &drv_map));
            BCM_IF_ERROR_RETURN(
                DRV_WRED_MAP_DEATTACH(unit, wred_idx, &drv_map));
        } else {
            drv_map.flags = DRV_WRED_CONFIG_FLAGS_DEFAULT_PROFILE;
            /* get the default profile index */
            BCM_IF_ERROR_RETURN(
                DRV_WRED_MAP_GET(unit, &wred_idx, &drv_map));
        }

        /* Try to destroy the WRED configuration  if no one used */
        rv = DRV_WRED_CONFIG_DESTROY(unit,wred_idx);
    }

    return BCM_E_NONE;
#else /* BCM_NORTHSTARPLUS_SUPPORT || BCM_STARFIGHTER3_SUPPORT */
    return BCM_E_UNAVAIL;
#endif /* !BCM_NORTHSTARPLUS_SUPPORT || !BCM_STARFIGHTER3_SUPPORT */
}

/*
 * Function:
 *      bcm_cosq_gport_discard_get
 * Purpose:
 *
 * Parameters:
 *      unit    - (IN) Unit number.
 *      port    - (IN) GPORT ID.
 *      cosq    - (IN) COS queue to get
 *      discard - (IN/OUT) Discard settings
 * Returns:
 *      BCM_E_XXX
 */
int 
bcm_robo_cosq_gport_discard_get(
    int unit, 
    bcm_gport_t gport, 
    bcm_cos_queue_t cosq, 
    bcm_cosq_gport_discard_t *discard)
{
#if defined(BCM_NORTHSTARPLUS_SUPPORT) || defined(BCM_STARFIGHTER3_SUPPORT)
    bcm_port_t  loc_port;
    int wred_idx;
    drv_wred_config_t drv_config;
    drv_wred_map_info_t drv_map;
    int rv = BCM_E_NONE;
 
    if (!(SOC_IS_NORTHSTARPLUS(unit) || SOC_IS_STARFIGHTER3(unit))) {
        return BCM_E_UNAVAIL;
    } 
    
    if (discard == NULL) {
        return BCM_E_PARAM;
    }
    
    if (!(discard->flags & BCM_COSQ_DISCARD_IFP)) {
        if (gport == BCM_GPORT_INVALID) {
            return (BCM_E_PORT);
        }
        
        if (BCM_GPORT_IS_SET(gport)) {
            if ((BCM_GPORT_MODPORT_MODID_GET(gport) == _SHR_GPORT_MODID_MASK) && 
                (BCM_GPORT_MODPORT_PORT_GET(gport) == _SHR_GPORT_PORT_MASK)) {
                /* System configuration for gport type */
                loc_port = -1;
            } else {
                BCM_IF_ERROR_RETURN(bcm_port_local_get(unit, gport, &loc_port));
            }
        } else {
            loc_port = gport;
        } 

        if (loc_port != -1 && !SOC_PORT_VALID(unit, loc_port)) {
            return BCM_E_PORT;
        }

        if (cosq < -1) {
            return BCM_E_PARAM;
        } else if (cosq != -1) {
            if (cosq > NUM_COS(unit)) {
                return BCM_E_PARAM;
            }
        }
    }

    sal_memset(&drv_map, 0, sizeof(drv_wred_map_info_t));
    if (discard->flags & BCM_COSQ_DISCARD_IFP) {
        drv_map.flags |= DRV_WRED_CONFIG_FLAGS_DEFAULT_PROFILE;
    } else {
        drv_map.port = loc_port;
        drv_map.cosq = cosq;
    }
    if (discard->flags & BCM_COSQ_DISCARD_COLOR_RED) {
        drv_map.flags |= DRV_WRED_MAP_FLAGS_COLOR_RED; 
    }
    if (discard->flags & BCM_COSQ_DISCARD_COLOR_YELLOW) {
        drv_map.flags |= DRV_WRED_MAP_FLAGS_COLOR_YELLOW; 
    }
    if (discard->flags & BCM_COSQ_DISCARD_COLOR_GREEN) {
        drv_map.flags |= DRV_WRED_MAP_FLAGS_COLOR_GREEN; 
    }
    if (discard->flags & BCM_COSQ_DISCARD_OUTER_CFI) {
        drv_map.flags |= DRV_WRED_MAP_FLAGS_DEI; 
    }
    
    rv = DRV_WRED_MAP_GET(unit, &wred_idx, &drv_map); 
    if (rv == BCM_E_NOT_FOUND) {
        /* No WRED setting is found */
        return BCM_E_NONE;
    } else {
        BCM_IF_ERROR_RETURN(rv);
    }

    BCM_IF_ERROR_RETURN(
        DRV_WRED_CONFIG_GET(unit, wred_idx, &drv_config)); 
    discard->drop_probability = drv_config.drop_prob;
    discard->max_thresh= drv_config.max_threshold;
    discard->min_thresh= drv_config.min_threshold;
    discard->gain = drv_config.gain;
    discard->flags |= BCM_COSQ_DISCARD_ENABLE;

    return BCM_E_NONE;
#else /* BCM_NORTHSTARPLUS_SUPPORT || BCM_STARFIGHTER3_SUPPORT */
    return BCM_E_UNAVAIL;
#endif /* !BCM_NORTHSTARPLUS_SUPPORT || !BCM_STARFIGHTER3_SUPPORT */
}

/*
 * Function:
 *      bcm_robo_cosq_control_set
 * Purpose:
 *      Set specified feature configuration
 *
 * Parameters:
 *      unit - (IN) Unit number.
 *      port - (IN) GPORT ID.
 *      cosq - (IN) COS queue.
 *      type - (IN) feature
 *      arg  - (IN) feature value
 * Returns:
 *      BCM_E_XXX
 * Notes:
 *      1. This API only supported for TB on ROBO chips now.
 *      2. The port and cosq parameters are not used for cosq control types as system based:
 *          - bcmCosqControlCopyPktToCpuUseTC
 *          - bcmCosqControlWdrrGranularity
 *          - bcmCosqControlDpValueDlf
 *          - bcmCosqControlDpChangeDlf
 *          - bcmCosqControlDpChangeXoff
 */
int
bcm_robo_cosq_control_set(int unit, bcm_gport_t port, bcm_cos_queue_t cosq,
                                           bcm_cosq_control_t type, int arg)
{
    bcm_pbmp_t pbmp;
    uint32  temp = 0, reg_val = 0;
    bcm_port_t  loc_port;

    LOG_VERBOSE(BSL_LS_BCM_COMMON,
                (BSL_META_U(unit,
                            "BCM API : bcm_robo_cosq_control_set()..\n")));

    BCM_PBMP_CLEAR(pbmp);

    if (BCM_GPORT_IS_SET(port)) {
        if ((BCM_GPORT_MODPORT_MODID_GET(port) == _SHR_GPORT_MODID_MASK) && 
            (BCM_GPORT_MODPORT_PORT_GET(port) == _SHR_GPORT_PORT_MASK)) {
            /* System configuration for gport type */
            loc_port = -1;
        } else {
            BCM_IF_ERROR_RETURN(bcm_port_local_get(unit, port, &loc_port));
        }
    } else {
        loc_port = port;
    }

    if (loc_port != -1 && !SOC_PORT_VALID(unit, loc_port)) {
        return BCM_E_PORT;
    }

    if ((type == bcmCosqControlPortTxqBackpressure) ||
         (type == bcmCosqControlEgressRateBurstAccumulateControl) ||
         (type == bcmCosqControlEgressRateType) ||
         (type == bcmCosqControlEavClassAWindow)) {
        /* Per-port based cosq control */
        if (loc_port < 0) {
            return BCM_E_PORT;
        }
    } else if ((type == bcmCosqControlCopyPktToCpuUseTC) ||
        (type == bcmCosqControlDpValueDlf) ||
        (type == bcmCosqControlDpChangeDlf) ||
        (type == bcmCosqControlDpChangeXoff)) {
        /* System based cosq control */
        if ((loc_port != -1) || (cosq != -1)) {
            return BCM_E_PARAM;
        }
    } else if (type == bcmCosqControlWdrrGranularity) {
        if (SOC_IS_TBX(unit)) {
            /* System based cosq control */
            if ((loc_port != -1) || (cosq != -1)) {
                return BCM_E_PARAM;
            }
        } else {
            if (cosq != -1) {
                return BCM_E_PARAM;
            }
        }
    } else if ((type == bcmCosqControlSchedulerTxqEmptySelect) || 
            (type == bcmCosqControlWdrrNegCreditClearEnable) ||
            (type == bcmCosqControlSchedulerBurstModeEnable)){
        if (cosq != -1) {
            return BCM_E_PARAM;
        }
    } else if (type == bcmCosqControlEEETxQCongestionThreshold) {
        /* Per-queue based cosq control */
        if (loc_port != -1) {
            return BCM_E_PARAM;
        }
    }

    if (type == bcmCosqControlWdrrGranularity) {
        /* this value definition is :
        *  1: number of packet(WRR).
        *  0: number of bytes for WdrrGranularity (WDRR).
        */
        temp = (arg) ? 1 : 0;
        BCM_IF_ERROR_RETURN(DRV_QUEUE_QOS_CONTROL_SET
            (unit, loc_port, DRV_QOS_CTL_WDRR_GRANULARTTY, temp));

        return BCM_E_NONE;
    } else if (type == bcmCosqControlSchedulerTxqEmptySelect) {
        if (arg == bcmCosqSchedEmptyTxQueue){
            temp = DRV_COSQ_SCHEDULER_EMPTY_TX_QUEUE;
        } else if (arg == bcmCosqSchedEmptyTxqShaper){
            temp = DRV_COSQ_SCHEDULER_EMPTY_TXQ_SHAPER;
        } else {
            return BCM_E_PARAM;
        }
        BCM_IF_ERROR_RETURN(DRV_QUEUE_QOS_CONTROL_SET
            (unit, loc_port, DRV_QOS_CTL_WDRR_TXQEMPTY, temp));

        return BCM_E_NONE;
    } else if (type == bcmCosqControlWdrrNegCreditClearEnable) {
        /* this value definition is :
        *  1: Enable the clear on Negative Credit in next WDRR service loop.
        *  0: No clear on Negative Credit in next WDRR service loop.
        */
        temp = (arg) ? 1 : 0;
        BCM_IF_ERROR_RETURN(DRV_QUEUE_QOS_CONTROL_SET
            (unit, loc_port, DRV_QOS_CTL_WDRR_NEGCREDIT_CLR, temp));

        return BCM_E_NONE;
    } else if (type == bcmCosqControlSchedulerBurstModeEnable) {
        /* this value definition is :
        *  1: force WRR/WDRR to work at burst mode. 
        *  0: force WRR/WDRR to work at non-burst mode. 
        */
        temp = (arg) ? 1 : 0;
        BCM_IF_ERROR_RETURN(DRV_QUEUE_QOS_CONTROL_SET
            (unit, loc_port, DRV_QOS_CTL_WDRR_BURSTMODE, temp));

        return BCM_E_NONE;
    }

    if (SOC_IS_TBX(unit)) {
#ifdef BCM_TB_SUPPORT
        switch (type) {
            case bcmCosqControlPortTxqBackpressure:
                /* check port is IMP port or GE ports */
                if ((!IS_GE_PORT(unit, loc_port)) && 
                    (!IS_CPU_PORT(unit, loc_port))) {
                    return BCM_E_PARAM;
                }
            
                if (!BCM_COSQ_QUEUE_VALID(unit, cosq)) {
                    return (BCM_E_PARAM);
                }
        
                if (arg) {
                    temp = 1;
                } else {
                    temp = 0;
                }
                BCM_IF_ERROR_RETURN(DRV_QUEUE_PORT_TXQ_PAUSE_SET
                    (unit, loc_port, cosq, (uint8)temp));
                break;
            case bcmCosqControlCopyPktToCpuUseTC:
                /* this type value definition is :
                 *  1: Use generic TC based COS mapping for copying packets to CPU.
                 *  0: Use Reason basd COS mapping  for copying packets to CPU.
                 */
                if (arg) {
                    temp = 1;
                } else {
                    temp = 0;
                }
                BCM_IF_ERROR_RETURN(DRV_QUEUE_QOS_CONTROL_SET
                    (unit, loc_port, DRV_QOS_CTL_USE_TC, temp));
                break;
            case bcmCosqControlDpValueDlf:
                /* this type value definition is :
                 *  DP_CTRL[3:2] value of the unknown unicast/multicast packet (0~ 3)
                 */
                temp = (uint32)arg;
                BCM_IF_ERROR_RETURN(DRV_QUEUE_QOS_CONTROL_SET
                    (unit, loc_port, DRV_QOS_CTL_DP_VALUE_DLF, _BCM_COLOR_ENCODING(unit, temp)));
                break;
            case bcmCosqControlDpChangeDlf:
                /* this type value definition is :
                 *  1: DP=DLF_DP (DP_CTRL[3:2]) if the packet is a DLF packet.
                 *  0: DP is depended on the setting of default port DP.
                 */
                if (arg) {
                    temp = 1;
                } else {
                    temp = 0;
                }
                BCM_IF_ERROR_RETURN(DRV_QUEUE_QOS_CONTROL_SET
                    (unit, loc_port, DRV_QOS_CTL_DP_CHANGE_DLF, temp));
                break;
            case bcmCosqControlDpChangeXoff:
                /* this type value definition is :
                 *  1: DP=DP0, if the port is flow-controllable port.
                 *  0: DP is depended on the setting of DLF DP or the default port DP.
                 */
                if (arg) {
                    temp = 1;
                } else {
                    temp = 0;
                }
                BCM_IF_ERROR_RETURN(DRV_QUEUE_QOS_CONTROL_SET
                    (unit, loc_port, DRV_QOS_CTL_DP_CHANGE_XOFF, temp));
                break;
            case bcmCosqControlEgressRateBurstAccumulateControl:
                /* this type value definition is :
                 *  1: Enable accumulation.
                 *  0: To indicate the value of the Burst Tolerance Size should be reset to zero 
                 *       when there is no packet in the queue waiting to be transmitted.
                 */
                if (cosq != -1) {
                    return BCM_E_PARAM;
                }
        
                BCM_PBMP_PORT_SET(pbmp, loc_port);
                if (arg) {
                    temp = 1;
                } else {
                    temp = 0;
                }
                BCM_IF_ERROR_RETURN(DRV_RATE_CONFIG_SET
                    (unit, pbmp, DRV_RATE_CONFIG_RATE_BAC, temp));
                break;
            case bcmCosqControlEgressRateType:
                /* this type value definition is :
                 *  1: pps
                 *  0: kbps
                 */
                if (cosq != -1) {
                    return BCM_E_PARAM;
                }
        
                BCM_PBMP_PORT_SET(pbmp, loc_port);
                if (arg) {
                    temp = 1;
                } else {
                    temp = 0;
                }
                BCM_IF_ERROR_RETURN(DRV_RATE_CONFIG_SET
                    (unit, pbmp, DRV_RATE_CONFIG_RATE_TYPE, temp));
                break;
            default:
                return BCM_E_UNAVAIL;;
        }

        return BCM_E_NONE;
#endif /* BCM_TB_SUPPORT */    
    }

    if (type == bcmCosqControlEavClassAWindow) {
        /* 
         * 1: Enable the jitter control of EAV class A traffic 
         * 0: Disable the jitter control of EAV class A traffic
         */
        if (arg) {
            temp = 1;
        } else {
            temp = 0;
        }
        BCM_IF_ERROR_RETURN(DRV_EAV_QUEUE_CONTROL_SET
            (unit, loc_port, DRV_EAV_QUEUE_Q5_WINDOW, temp));

        return BCM_E_NONE;
    }

    if ((type == bcmCosqControlEEETxQCongestionThreshold) &&
        soc_feature (unit, soc_feature_eee)) {
        SOC_IF_ERROR_RETURN(
            REG_READ_EEE_TXQ_CONG_THr(unit, cosq, &reg_val));
        temp = arg;
        soc_EEE_TXQ_CONG_THr_field_set(unit, &reg_val, 
            TXQ_CONG_THf, &temp);
        SOC_IF_ERROR_RETURN(
            REG_WRITE_EEE_TXQ_CONG_THr(unit, cosq, &reg_val));
    }

    if (type == bcmCosqControlEgressWredDropCancel) {
    
        if (arg) {
            temp = 1;
        } else {
            temp = 0;
        }
        BCM_IF_ERROR_RETURN(
            DRV_WRED_CONTROL_SET(unit, loc_port, cosq, 
                DRV_WRED_CONTROL_DROP_BYPASS, temp));
        return BCM_E_NONE;
    }

    return BCM_E_UNAVAIL;;
}

/*
 * Function:
 *      bcm_robo_cosq_control_get
 * Purpose:
 *      Get specified feature configuration
 *
 * Parameters:
 *      unit - (IN) Unit number.
 *      port - (IN) GPORT ID.
 *      cosq - (IN) COS queue.
 *      type - (IN) feature
 *      arg  - (OUT) feature value
 * Returns:
 *      BCM_E_XXX
 * Notes:
 *      1. This API only supported for TB on ROBO chips now.
 *      2. The port and cosq parameters are not used for cosq control types as system based:
 *          - bcmCosqControlCopyPktToCpuUseTC
 *          - bcmCosqControlWdrrGranularity
 *          - bcmCosqControlDpValueDlf
 *          - bcmCosqControlDpChangeDlf
 *          - bcmCosqControlDpChangeXoff
 */
int
bcm_robo_cosq_control_get(int unit, bcm_gport_t port, bcm_cos_queue_t cosq,
                                          bcm_cosq_control_t type, int *arg)
{
    uint32  temp = 0, reg_val = 0;
    bcm_port_t  loc_port;

    LOG_VERBOSE(BSL_LS_BCM_COMMON,
                (BSL_META_U(unit,
                            "BCM API : bcm_robo_cosq_control_get()..\n")));

    if (BCM_GPORT_IS_SET(port)) {
        if ((BCM_GPORT_MODPORT_MODID_GET(port) == _SHR_GPORT_MODID_MASK) && 
            (BCM_GPORT_MODPORT_PORT_GET(port) == _SHR_GPORT_PORT_MASK)) {
            /* System configuration for gport type */
            loc_port = -1;
        } else {
            BCM_IF_ERROR_RETURN(bcm_port_local_get(unit, port, &loc_port));
        }
    } else {
        loc_port = port;
    }

    if (loc_port != -1 && !SOC_PORT_VALID(unit, loc_port)) {
        return BCM_E_PORT;
    }

    if ((type == bcmCosqControlPortTxqBackpressure) ||
         (type == bcmCosqControlEgressRateBurstAccumulateControl) ||
         (type == bcmCosqControlEgressRateType) ||
         (type == bcmCosqControlEavClassAWindow)) {
        /* Per-port based cosq control */
        if (loc_port < 0) {
            return BCM_E_PORT;
        }
    } else if ((type == bcmCosqControlCopyPktToCpuUseTC) ||
        (type == bcmCosqControlDpValueDlf) ||
        (type == bcmCosqControlDpChangeDlf) ||
        (type == bcmCosqControlDpChangeXoff)) {
        /* System based cosq control */        
        if ((loc_port != -1) || (cosq != -1)) {
            return BCM_E_PARAM;
        }
    } else if (type == bcmCosqControlWdrrGranularity) {
        if (SOC_IS_TBX(unit)) {
            /* System based cosq control */
            if ((loc_port != -1) || (cosq != -1)) {
                return BCM_E_PARAM;
            }
        } else {
            if (cosq != -1) {
                return BCM_E_PARAM;
            }
        }
    } else if ((type == bcmCosqControlSchedulerTxqEmptySelect) || 
            (type == bcmCosqControlWdrrNegCreditClearEnable) ||
            (type == bcmCosqControlSchedulerBurstModeEnable)){
        if (cosq != -1) {
            return BCM_E_PARAM;
        }
    } else if (type == bcmCosqControlEEETxQCongestionThreshold) {
        /* Per-queue based cosq control */
        if (loc_port != -1) {
            return BCM_E_PARAM;
        }
    }

    if (type == bcmCosqControlWdrrGranularity) {
        /* this value definition is :
        *  1: number of packet(WRR).
        *  0: number of bytes for WdrrGranularity (WDRR).
        */
        BCM_IF_ERROR_RETURN(DRV_QUEUE_QOS_CONTROL_GET
                (unit, loc_port, DRV_QOS_CTL_WDRR_GRANULARTTY, &temp));
        *arg = (temp) ? TRUE : FALSE;
        return BCM_E_NONE;
    } else if (type == bcmCosqControlSchedulerTxqEmptySelect) {
        BCM_IF_ERROR_RETURN(DRV_QUEUE_QOS_CONTROL_GET
                (unit, loc_port, DRV_QOS_CTL_WDRR_TXQEMPTY, &temp));
        if (temp == DRV_COSQ_SCHEDULER_EMPTY_TX_QUEUE){
            *arg = bcmCosqSchedEmptyTxQueue;
        } else if (temp == DRV_COSQ_SCHEDULER_EMPTY_TXQ_SHAPER){
            *arg = bcmCosqSchedEmptyTxqShaper;
        } else {
            return BCM_E_INTERNAL;
        }
        return BCM_E_NONE;
    } else if (type == bcmCosqControlWdrrNegCreditClearEnable) {
        /* this value definition is :
        *  1: Enable the clear on Negative Credit in next WDRR service loop.
        *  0: No clear on Negative Credit in next WDRR service loop.
        */
        BCM_IF_ERROR_RETURN(DRV_QUEUE_QOS_CONTROL_GET
                (unit, loc_port, DRV_QOS_CTL_WDRR_NEGCREDIT_CLR, &temp));
        *arg = (temp) ? TRUE : FALSE;
        return BCM_E_NONE;
    } else if (type == bcmCosqControlSchedulerBurstModeEnable) {
        /* this value definition is :
        *  1: force WRR/WDRR to work at burst mode. 
        *  0: force WRR/WDRR to work at non-burst mode. 
        */
        BCM_IF_ERROR_RETURN(DRV_QUEUE_QOS_CONTROL_GET
                (unit, loc_port, DRV_QOS_CTL_WDRR_BURSTMODE, &temp));
        *arg = (temp) ? TRUE : FALSE;
        return BCM_E_NONE;
    }

    if (SOC_IS_TBX(unit)) {
#ifdef BCM_TB_SUPPORT        
    switch (type) {
    case bcmCosqControlPortTxqBackpressure:
        /* check port is IMP port or GE ports */
        if ((!IS_GE_PORT(unit, loc_port)) && (!IS_CPU_PORT(unit, loc_port))) {
            return BCM_E_PARAM;
        }
    
        if (!BCM_COSQ_QUEUE_VALID(unit, cosq)) {
            return (BCM_E_PARAM);
        }
        BCM_IF_ERROR_RETURN(DRV_QUEUE_PORT_TXQ_PAUSE_GET
            (unit, loc_port, cosq, (uint8 *)&temp));
        break;
    case bcmCosqControlCopyPktToCpuUseTC:
        /* this type value definition is :
         *  1: Use generic TC based COS mapping for copying packets to CPU.
         *  0: Use Reason basd COS mapping  for copying packets to CPU.
         */
        BCM_IF_ERROR_RETURN(DRV_QUEUE_QOS_CONTROL_GET
            (unit, loc_port, DRV_QOS_CTL_USE_TC, &temp));
        break;
    case bcmCosqControlDpValueDlf:
        /* this type value definition is :
         *  DP_CTRL[3:2] value of the unknown unicast/multicast packet (0~ 3)
         */
        BCM_IF_ERROR_RETURN(DRV_QUEUE_QOS_CONTROL_GET
            (unit, loc_port, DRV_QOS_CTL_DP_VALUE_DLF, &temp));
        break;
    case bcmCosqControlDpChangeDlf:
        /* this type value definition is :
         *  1: DP=DLF_DP (DP_CTRL[3:2]) if the packet is a DLF packet.
         *  0: DP is depended on the setting of default port DP.
         */
        BCM_IF_ERROR_RETURN(DRV_QUEUE_QOS_CONTROL_GET
            (unit, loc_port, DRV_QOS_CTL_DP_CHANGE_DLF, &temp));
        break;
    case bcmCosqControlDpChangeXoff:
        /* this type value definition is :
         *  1: DP=DP0, if the port is flow-controllable port.
         *  0: DP is depended on the setting of DLF DP or the default port DP.
         */
        BCM_IF_ERROR_RETURN(DRV_QUEUE_QOS_CONTROL_GET
            (unit, loc_port, DRV_QOS_CTL_DP_CHANGE_XOFF, &temp));
        break;
    case bcmCosqControlEgressRateBurstAccumulateControl:
        /* this type value definition is :
         *  1: Enable accumulation.
         *  0: To indicate the value of the Burst Tolerance Size should be reset to zero 
         *       when there is no packet in the queue waiting to be transmitted.
         */
        if (cosq != -1) {
            return BCM_E_PARAM;
        }
        BCM_IF_ERROR_RETURN(DRV_RATE_CONFIG_GET
            (unit, loc_port, DRV_RATE_CONFIG_RATE_BAC, &temp));
        break;
    case bcmCosqControlEgressRateType:
        /* this type value definition is :
         *  1: pkt / sec (100 pkts/sec).
         *  0: bit / sec (64 * 1000 bits/sec).
         */
        if (cosq != -1) {
            return BCM_E_PARAM;
        }
        BCM_IF_ERROR_RETURN(DRV_RATE_CONFIG_GET
            (unit, loc_port, DRV_RATE_CONFIG_RATE_TYPE, &temp));
        break;
    default:
        return BCM_E_UNAVAIL;;
    }

    *arg = temp;

    return BCM_E_NONE;
#endif /* BCM_TB_SUPPORT */    
    }

    if (type == bcmCosqControlEavClassAWindow) {
        /* 
         * 1: Enable the jitter control of EAV class A traffic 
         * 0: Disable the jitter control of EAV class A traffic
         */
        BCM_IF_ERROR_RETURN(DRV_EAV_QUEUE_CONTROL_GET
            (unit, loc_port, DRV_EAV_QUEUE_Q5_WINDOW, &temp));

        if (temp) {
            *arg = 1;
        } else {
            *arg = 0;
        }

        return BCM_E_NONE;
    }

    if ((type == bcmCosqControlEEETxQCongestionThreshold) &&
        soc_feature (unit, soc_feature_eee)) {
        SOC_IF_ERROR_RETURN(
            REG_READ_EEE_TXQ_CONG_THr(unit, cosq, &reg_val));
        soc_EEE_TXQ_CONG_THr_field_get(unit, &reg_val, 
            TXQ_CONG_THf, &temp);
        *arg = temp;
    }

    if (type == bcmCosqControlEgressWredDropCancel) {
        BCM_IF_ERROR_RETURN(
            DRV_WRED_CONTROL_GET(unit, loc_port, cosq, 
                DRV_WRED_CONTROL_DROP_BYPASS, &temp));
        if (temp) {
            *arg = 1;
        } else {
            *arg = 0;
        }
        
        return BCM_E_NONE;
    }

    return BCM_E_UNAVAIL;;
}


