/*
 * $Id: stat.c,v 1.13.24.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * Broadcom CALADAN3 Statistics API.
 */

#include <shared/bsl.h>

#include <soc/sbx/sbx_drv.h>
#include <soc/sbx/counter.h>
#include <soc/sbx/caladan3_counter.h>

#include <bcm/error.h>
#include <bcm/stat.h>

#include <bcm/error.h>

#include <bcm_int/control.h>
#include <bcm_int/common/lock.h>
#include <bcm_int/sbx/error.h>

#include <bcm_int/sbx/stat.h>
#include <bcm_int/sbx/caladan3/stat.h>


#include <soc/sbx/g3p1/g3p1_int.h>
#include <soc/sbx/g3p1/g3p1_defs.h>



#ifndef PLISIM
#define COUNTER_FLAGS_DEFAULT	SOC_COUNTER_F_DMA
#else
#define COUNTER_FLAGS_DEFAULT	0
#endif

#define REG_MATH_DECL \
        uint64 reg_val

#define REG_ADD(unit, port, reg, val)                                      \
    if (SOC_REG_IS_VALID(unit, reg) && SOC_REG_IS_COUNTER(unit, reg)) {    \
        SOC_IF_ERROR_RETURN(soc_counter_get(unit, port, reg,               \
                                            0, &reg_val));                 \
        COMPILER_64_ADD_64(val, reg_val);                                  \
    }
#define REG_SUB(unit, port, reg, val)                                      \
    if (SOC_REG_IS_VALID(unit, reg) && SOC_REG_IS_COUNTER(unit, reg)) {    \
        SOC_IF_ERROR_RETURN(soc_counter_get(unit, port, reg,               \
                                            0, &reg_val));                 \
        if (COMPILER_64_GT(val, reg_val)) {                                \
            COMPILER_64_SUB_64(val, reg_val);                              \
        } else {                                                           \
            COMPILER_64_ZERO(val);                                         \
        }                                                                  \
    }

#define REG_ADD_CONTROLLED_COUNTER(unit, port, ctr_id, val)                \
    SOC_IF_ERROR_RETURN(soc_counter_get(unit, port, ctr_id,                \
                                        0, &reg_val));                     \
    COMPILER_64_ADD_64(val, reg_val);                                      \

/*
 * Function:
 *     _bcm_caladan3_sw_counter_init
 * Description:
 *     Initialize and start the counter collection, software
 *     accumulation process, on given unit.
 * Parameters:
 *     unit  - Device number
 * Returns:
 *     BCM_E_NONE - Success
 *     BCM_E_XXX  - Failure
 */
STATIC int
_bcm_caladan3_sw_counter_init(int unit)
{
    pbmp_t       pbmp, pbmp2;
    sal_usecs_t  interval;
    uint32       flags;

    if (!SOC_UNIT_VALID(unit)) {
        return BCM_E_UNIT;
    }

    if (soc_property_get_str(unit, spn_BCM_STAT_PBMP) == NULL) {
      SOC_PBMP_ASSIGN(pbmp, PBMP_ALL(unit));
    } else {
      pbmp = soc_property_get_pbmp(unit, spn_BCM_STAT_PBMP, 0);
      SOC_PBMP_ASSIGN(pbmp2, SOC_PORT_BITMAP(unit, hg));
      SOC_PBMP_OR(pbmp2, SOC_PORT_BITMAP(unit, ge));
      SOC_PBMP_OR(pbmp2, SOC_PORT_BITMAP(unit, xe));
      SOC_PBMP_AND(pbmp, pbmp2);
    }

    interval = (SAL_BOOT_BCMSIM) ?
               SOC_COUNTER_INTERVAL_SIM_DEFAULT : SOC_COUNTER_INTERVAL_DEFAULT;
    interval = soc_property_get(unit, spn_BCM_STAT_INTERVAL, interval);

    flags = soc_property_get(unit, spn_BCM_STAT_FLAGS, COUNTER_FLAGS_DEFAULT);

    SOC_IF_ERROR_RETURN(soc_sbx_caladan3_counter_init(unit, flags, interval, pbmp));

    return BCM_E_NONE;

}


int
bcm_caladan3_stat_detach(int unit)
{
    bcm_stat_info_t            *si;
    int                         result = BCM_E_NONE;

    LOG_DEBUG(BSL_LS_BCM_STAT,
              (BSL_META_U(unit,
                          "%s(%d) - Enter\n"),
               FUNCTION_NAME(), unit));

    if (!BCM_UNIT_VALID(unit) || unit >= BCM_MAX_NUM_UNITS) {
        /* stat_control is sized by BCM_MAX_NUM_UNITS, not BCM_CONTROL_MAX,
         * or BCM_UNITS_MAX
         */
        return BCM_E_UNIT;
    }

    si = &STAT_CNTL(unit);

    if (NULL != si->segInfo) {
        if (si->segInfo[CALADAN3_G3P1_COUNTER_INGRESS].pList != NULL) {
            result = shr_aidxres_list_destroy(si->segInfo[CALADAN3_G3P1_COUNTER_INGRESS].pList);
        }

        if (si->segInfo[CALADAN3_G3P1_COUNTER_EGRESS].pList != NULL) {
            result = shr_aidxres_list_destroy(si->segInfo[CALADAN3_G3P1_COUNTER_EGRESS].pList);
        }

        sal_free(si->segInfo);
    }

    return result;
}


int
bcm_caladan3_stat_init(int unit)
{
    bcm_stat_info_t         *si = NULL;
    int                     result = BCM_E_NONE;
    int                     lrp_bypass = 0;
	int 					size;
	uint32					ingress_max_counters = 0;
	uint32					egress_max_counters = 0;

    LOG_DEBUG(BSL_LS_BCM_STAT,
              (BSL_META_U(unit,
                          "%s(%d) - Enter\n"),
               FUNCTION_NAME(), unit));

    if (!BCM_UNIT_VALID(unit) || unit >= BCM_MAX_NUM_UNITS) {
        return BCM_E_UNIT;
    }

    lrp_bypass = soc_property_get(unit, spn_LRP_BYPASS, 0);
    if (!lrp_bypass) {
        switch (SOC_SBX_CONTROL(unit)->ucodetype) {
        case SOC_SBX_UCODE_TYPE_T3P1:
        break;			

        case SOC_SBX_UCODE_TYPE_G3P1:
            result = soc_sbx_g3p1_constant_get(unit, "ingress_max_counters", &ingress_max_counters);
            if (BCM_FAILURE(result)) {
                LOG_ERROR(BSL_LS_BCM_STAT,
                          (BSL_META_U(unit,
                                      "bcm_stat_init: soc_sbx_g3p1_constant_get (ingress_max_counters) failed: %s\n"),
                           bcm_errmsg(result)));
                return BCM_E_INTERNAL;
            }
            result = soc_sbx_g3p1_constant_get(unit, "egress_max_counters", &egress_max_counters);
            if (BCM_FAILURE(result)) {
                LOG_ERROR(BSL_LS_BCM_STAT,
                          (BSL_META_U(unit,
                                      "bcm_stat_init: soc_sbx_g3p1_constant_get (egress_max_counters) failed: %s\n"),
                           bcm_errmsg(result)));
                return BCM_E_INTERNAL;
            }
        break;

        default:
            SBX_UNKNOWN_UCODE_WARN(unit);
            return BCM_E_INTERNAL;
        }
    }

    si = &STAT_CNTL(unit);

    if (!lrp_bypass) {
        if (si->segInfo != NULL) {
            result = bcm_caladan3_stat_detach(unit);
            BCM_IF_ERROR_RETURN(result);
            si->segInfo = NULL;
        }

        if (SOC_IS_SBX_G3P1(unit)){
            size = CALADAN3_G3P1_COUNTER_MAX * sizeof(bcm_stat_sbx_info_t);
            if (NULL == si->segInfo) {
                si->segInfo = sal_alloc(size, "Counter Segments");
                if (NULL == si->segInfo) {
                    return BCM_E_MEMORY;
                }
            }
            sal_memset(si->segInfo, 0, size);
            
            /* Create lists for ingress and egress counter allocation */
            result = shr_aidxres_list_create(&si->segInfo[CALADAN3_G3P1_COUNTER_INGRESS].pList,
                1, ingress_max_counters-1,
                1, ingress_max_counters-1,
                7, "ingctr");
            if (BCM_FAILURE(result)) {
                LOG_ERROR(BSL_LS_BCM_STAT,
                          (BSL_META_U(unit,
                                      "bcm_stat_init: shr_aidxres_list_create (ingress counters )failed: %s\n"),
                           bcm_errmsg(result)));
                return result;
            }
            result = shr_aidxres_list_create(&si->segInfo[CALADAN3_G3P1_COUNTER_EGRESS].pList,
                1, egress_max_counters-1,
                1, egress_max_counters-1,
                7, "egrctr");
     		    
            if (BCM_FAILURE(result)) {
                LOG_ERROR(BSL_LS_BCM_STAT,
                          (BSL_META_U(unit,
                                      "bcm_stat_init: shr_aidxres_list_create (egress counters )failed: %s\n"),
                           bcm_errmsg(result)));
                return result;
            }
        }
    }
    
    si->init = TRUE;


    result = _bcm_caladan3_sw_counter_init(unit);

    LOG_VERBOSE(BSL_LS_BCM_STAT,
                (BSL_META_U(unit,
                            "bcm_stat_init: unit=%d rv=%d(%s)\n"),
                 unit, result, bcm_errmsg(result)));

    return result;
}


/*
 * Function:
 *      _bcm_caladan3_stat_block_alloc
 * Description:
 *      Allocate counters from a statistics block
 * Parameters:
 *      unit      - device unit number.
 *      type      - one of the defined segment types
 *      count     - number of counters required
 *      start     - (OUT) where to put first of allocated counter block
 * Returns:
 *      BCM_E_NONE      - Success
 *      BCM_E_XXXX      - Failure
 */
int _bcm_caladan3_stat_block_alloc(int unit,
                                  int type,
                                  shr_aidxres_element_t *start,
                                  shr_aidxres_element_t count,
                                  uint32 flags)
{
    /* be sure we're initialised */
    STAT_CHECK_INIT(unit);

    if (type < CALADAN3_G3P1_COUNTER_INGRESS ||
        type > CALADAN3_G3P1_COUNTER_EGRESS) {
        return BCM_E_PARAM;
    }


    if (flags & BCM_CALADAN3_STAT_WITH_ID) {
        if (start == NULL) {
            return BCM_E_PARAM;
        }
        return shr_aidxres_list_reserve_block(STAT_CNTL(unit).segInfo[type].pList,
                                              *start,
                                              count);
    } else {

        /* ask the list for an appropriate free contiguous block */
        return shr_aidxres_list_alloc_block(STAT_CNTL(unit).segInfo[type].pList,
                                            count,
                                            start);
    }
}

/*
 * Function:
 *      _bcm_caladan3_stat_block_free
 * Description:
 *      Free counters from a statistics block
 * Parameters:
 *      unit      - device unit number.
 *      type      - one of the defined segment types
 *      start     - (OUT) where to put first of allocated counter block
 * Returns:
 *      BCM_E_NONE      - Success
 *      BCM_E_XXXX      - Failure
 */
int _bcm_caladan3_stat_block_free(int unit,
                                int type,
                                shr_aidxres_element_t start)
{
    /* be sure we're initialised */
    STAT_CHECK_INIT(unit);

    if (type < CALADAN3_G3P1_COUNTER_INGRESS ||
        type > CALADAN3_G3P1_COUNTER_EGRESS) {
        return BCM_E_PARAM;
    }

    /* return the block to the list */
    return shr_aidxres_list_free(STAT_CNTL(unit).segInfo[type].pList, start);
}


int
bcm_caladan3_stat_sync(int unit)
{
  return soc_sbx_counter_sync(unit);
}

/*
 * Function:
 *	bcm_caladan3_stat_get
 * Description:
 *	Get the specified statistic for a HG port on a Caladan3 device.
 * Parameters:
 *	unit - Caladan3 PCI device unit number (driver internal).
 *	port - zero-based port number
 *	type - SNMP statistics type (see stat.h)
 * Returns:
 *	BCM_E_NONE - Success.
 *	BCM_E_PARAM - Illegal parameter.
 *	BCM_E_INTERNAL - Chip access failure.
 */

int
bcm_caladan3_stat_get(int unit,
                 bcm_port_t port,
                 bcm_stat_val_t type,
                 uint64 *val)
{
  uint64 count;
  uint32 rv = BCM_E_NONE;
  REG_MATH_DECL;       /* Required for use of the REG_* macros */

  if (!SOC_PORT_VALID(unit, port)) {
      return BCM_E_PORT;
  }

    COMPILER_REFERENCE(&count);  /* Work around PPC compiler bug */
    COMPILER_64_ZERO(count);

    if (port == CMIC_PORT(unit)) {
        switch (type) {
        case snmpIfInOctets:
            COMPILER_64_SET(*val, 0, SOC_CONTROL(unit)->stat.dma_rbyt);
            break;
        case snmpIfInUcastPkts:
            COMPILER_64_SET(*val, 0, SOC_CONTROL(unit)->stat.dma_rpkt);
            break;
        case snmpIfOutOctets:
            COMPILER_64_SET(*val, 0, SOC_CONTROL(unit)->stat.dma_tbyt);
            break;
        case snmpIfOutUcastPkts:
            COMPILER_64_SET(*val, 0, SOC_CONTROL(unit)->stat.dma_tpkt);
            break;
        default:
            COMPILER_64_ZERO(*val);
            break;
        }
        return (BCM_E_NONE);
    }else if (IS_IL_PORT(unit, port)) {
        /* Interlaken port, should get counter using soc_caladan3_countrolled_counters[] array*/
        switch (type) {
        case snmpIfOutUcastPkts:
            /* TPKTr map to counter index 0 in soc_caladan3_countrolled_counters[] */
            REG_ADD_CONTROLLED_COUNTER(unit, port, 0, count);
            break;

        case snmpIfOutOctets:
            /* TBYTr map to counter index 1 in soc_caladan3_countrolled_counters[] */
            REG_ADD_CONTROLLED_COUNTER(unit, port, 1, count);
            break;
        
        case snmpIfOutErrors:
            /* Tx errors map to counter index 2 in soc_caladan3_countrolled_counters[] */
            REG_ADD_CONTROLLED_COUNTER(unit, port, 2, count);
            break;
            
        case snmpEtherTxOversizePkts:
            /* TOVRr errors map to counter index 3 in soc_caladan3_countrolled_counters[] */
            REG_ADD_CONTROLLED_COUNTER(unit, port, 3, count);
            break;
            
        case snmpIfInUcastPkts:
            /* RPKTr map to counter index 5 in soc_caladan3_countrolled_counters[] */
            REG_ADD_CONTROLLED_COUNTER(unit, port, 5, count);
            break;

        case snmpIfInOctets:
            /* RBYTr map to counter index 6 in soc_caladan3_countrolled_counters[] */
            REG_ADD_CONTROLLED_COUNTER(unit, port, 6, count);; 
            break;
            
        case snmpIfInErrors:
            /* Rx error map to counter index 6 in soc_caladan3_countrolled_counters[] */
            REG_ADD_CONTROLLED_COUNTER(unit, port, 7, count);; 
            break;
        case snmpEtherRxOversizePkts:
             /* ROVRr map to counter index 8 in soc_caladan3_countrolled_counters[] */
            REG_ADD_CONTROLLED_COUNTER(unit, port, 8, count);; 
            break;
            
        case snmpEtherStatsCRCAlignErrors:
             /* RFCSr map to counter index 10 in soc_caladan3_countrolled_counters[] */
            REG_ADD_CONTROLLED_COUNTER(unit, port, 10, count);; 
            break;
            
        default:
            break;
        }
        *val = count;
        return (BCM_E_NONE);
    }
    
    switch (type) {
    case snmpIfInOctets:
	REG_ADD(unit, port, RBYTr, count);
	break;
    case snmpIfInUcastPkts:
	REG_ADD(unit, port, RUCAr, count); /* - valid rx pkt count */
	break;
    case snmpIfInNUcastPkts:	/* Multicast frames plus broadcast frames */
	REG_ADD(unit, port, RMCAr, count); /* + multicast */
	REG_ADD(unit, port, RBCAr, count); /* + broadcast */
	break;
    case snmpIfInDiscards:	/* Not supported */
	break;
    case snmpIfInErrors: /* RX Errors or Receive packets - non-error frames */
	REG_ADD(unit, port, RFCSr, count);
	REG_ADD(unit, port, RJBRr, count);
	REG_ADD(unit, port, RUNDr, count);
	REG_ADD(unit, port, RFRGr, count);
	break;
    case snmpIfInUnknownProtos:	/* Not supported */
	break;
    case snmpIfOutOctets:	/* TX bytes */
	REG_ADD(unit, port, TBYTr, count);
	break;
    case snmpIfOutUcastPkts:	/* ALL - mcast - bcast */
        REG_ADD(unit, port, TUCAr, count);  /* valid tx pkt count */
	break;
    case snmpIfOutNUcastPkts:	/* broadcast frames plus multicast frames */
	REG_ADD(unit, port, TMCAr, count); /* + multicast */
	REG_ADD(unit, port, TBCAr, count); /* + broadcast */
	break;
    case snmpIfOutDiscards:	/* Not supported */
	break;
    case snmpIfOutErrors:   /* Not supported */
	break;
    case snmpIfOutQLen:
      break;
    case snmpIpInReceives:
	break;
    case snmpIpInHdrErrors:
	break;
    case snmpIpForwDatagrams:
	break;
    case snmpIpInDiscards:
	break;

	/* *** RFC 1493 *** */

    case snmpDot1dBasePortDelayExceededDiscards:
	break;
    case snmpDot1dBasePortMtuExceededDiscards:
        REG_ADD(unit, port, ROVRr, count);     /* oversize pkts */
	REG_ADD(unit, port, TOVRr, count);     /* oversize pkts */
	break;
    case snmpDot1dTpPortInFrames:
	REG_ADD(unit, port, RPKTr, count);
	break;
    case snmpDot1dTpPortOutFrames:
	REG_ADD(unit, port, TPKTr, count);
	break;
    case snmpDot1dPortInDiscards:
	break;

	/* *** RFC 1757 *** */

    case snmpEtherStatsDropEvents:
	break;
    case snmpEtherStatsOctets:
	REG_ADD(unit, port, RBYTr, count);
	REG_ADD(unit, port, TBYTr, count);
	break;
    case snmpEtherStatsPkts:
	REG_ADD(unit, port, RPKTr, count);
	REG_ADD(unit, port, TPKTr, count);
        REG_ADD(unit, port, RUNDr, count); /* Runts */
        REG_ADD(unit, port, RFRGr, count); /* Fragments */
	break;
    case snmpEtherStatsBroadcastPkts:	/* Broadcast packets (RX/TX) */
	REG_ADD(unit, port, RBCAr, count); /* + rx broadcast  */
	REG_ADD(unit, port, TBCAr, count); /* + tx broadcast */
	break;
    case snmpIfInBroadcastPkts:     /* Broadcast packets (RX) */
       REG_ADD(unit, port, RBCAr, count); /* + rx broadcast  */
       break;
    case snmpIfOutBroadcastPkts:     /* Broadcast packets (TX) */
       REG_ADD(unit, port, TBCAr, count); /* + tx broadcast  */
       break;
    case snmpEtherStatsMulticastPkts:	/* Multicast packets (TX+RX) */
	REG_ADD(unit, port, RMCAr, count); /* + rx multicast */
	REG_ADD(unit, port, TMCAr, count); /* + tx multicast */
	break;
    case snmpIfInMulticastPkts:     /* Multicast packets RX */
       REG_ADD(unit, port, RMCAr, count); /* + rx multicast */
       break;
    case snmpIfOutMulticastPkts:     /* Multicast packets TX */
       REG_ADD(unit, port, TMCAr, count); /* + tx multicast */
       break;
    case snmpEtherStatsCRCAlignErrors:
	REG_ADD(unit, port, RFCSr, count);
	break;
    case snmpEtherStatsUndersizePkts:	/* Undersize frames */
	REG_ADD(unit, port, RUNDr, count);
	break;
    case snmpEtherStatsOversizePkts:
        REG_ADD(unit, port, ROVRr, count);
	REG_ADD(unit, port, TOVRr, count);
	break;
    case snmpEtherRxOversizePkts:
        REG_ADD(unit, port, ROVRr, count);
       break;
    case snmpEtherTxOversizePkts:
        REG_ADD(unit, port, TOVRr, count);
       break;
    case snmpEtherStatsFragments:
	REG_ADD(unit, port, RFRGr, count);
	break;
    case snmpEtherStatsJabbers:
	REG_ADD(unit, port, RJBRr, count);
	break;
    case snmpEtherStatsCollisions:
	/* always 0 */
	break;
    case snmpEtherStatsPkts64Octets:
        REG_ADD(unit, port, R64r, count);
        REG_ADD(unit, port, T64r, count);
	break;
    case snmpBcmReceivedPkts64Octets:
        REG_ADD(unit, port, R64r, count);
       break;
    case snmpBcmTransmittedPkts64Octets:
        REG_ADD(unit, port, T64r, count);
       break;
    case snmpEtherStatsPkts65to127Octets:
        REG_ADD(unit, port, R127r, count);
        REG_ADD(unit, port, T127r, count);
	break;
    case snmpBcmReceivedPkts65to127Octets:
        REG_ADD(unit, port, R127r, count);
       break;
    case snmpBcmTransmittedPkts65to127Octets:
        REG_ADD(unit, port, T127r, count);
       break;
    case snmpEtherStatsPkts128to255Octets:
        REG_ADD(unit, port, R255r, count);
        REG_ADD(unit, port, T255r, count);
	break;
    case snmpBcmReceivedPkts128to255Octets:
        REG_ADD(unit, port, R255r, count);
       break;
    case snmpBcmTransmittedPkts128to255Octets:
        REG_ADD(unit, port, T255r, count);
       break;
    case snmpEtherStatsPkts256to511Octets:
        REG_ADD(unit, port, R511r, count);
        REG_ADD(unit, port, T511r, count);
	break;
    case snmpBcmReceivedPkts256to511Octets:
        REG_ADD(unit, port, R511r, count);
       break;
    case snmpBcmTransmittedPkts256to511Octets:
        REG_ADD(unit, port, T511r, count);
       break;
    case snmpEtherStatsPkts512to1023Octets:
        REG_ADD(unit, port, R1023r, count);
        REG_ADD(unit, port, T1023r, count);
	break;
    case snmpBcmReceivedPkts512to1023Octets:
        REG_ADD(unit, port, R1023r, count);
       break;
    case snmpBcmTransmittedPkts512to1023Octets:
        REG_ADD(unit, port, T1023r, count);
       break;
    case snmpEtherStatsPkts1024to1518Octets:
        REG_ADD(unit, port, R1518r, count);
        REG_ADD(unit, port, T1518r, count);
	break;
    case snmpBcmReceivedPkts1024to1518Octets:
        REG_ADD(unit, port, R1518r, count);
       break;
    case snmpBcmTransmittedPkts1024to1518Octets:
        REG_ADD(unit, port, T1518r, count);
       break;

	/* *** not actually in rfc1757 *** */

    case snmpBcmEtherStatsPkts1519to1522Octets:
        REG_ADD(unit, port, R2047r, count);
        REG_ADD(unit, port, T2047r, count);
        REG_ADD(unit, port, R4095r, count);
        REG_ADD(unit, port, T4095r, count);
        REG_ADD(unit, port, R9216r, count);
        REG_ADD(unit, port, T9216r, count);
        REG_ADD(unit, port, R16383r, count);
        REG_ADD(unit, port, T16383r, count);
        break;
    case snmpBcmReceivedPkts1519to2047Octets:
        REG_ADD(unit, port, R2047r, count);
       break;
    case snmpBcmTransmittedPkts1519to2047Octets:
        REG_ADD(unit, port, T2047r, count);
       break;
    case snmpBcmReceivedPkts2048to4095Octets:
        REG_ADD(unit, port, R4095r, count);
       break;
    case snmpBcmTransmittedPkts2048to4095Octets:
        REG_ADD(unit, port, T4095r, count);
       break;
    case snmpBcmReceivedPkts4095to9216Octets:
        REG_ADD(unit, port, R9216r, count);
       break;
    case snmpBcmTransmittedPkts4095to9216Octets:
        REG_ADD(unit, port, T9216r, count);
       break;
    case snmpBcmEtherStatsPkts1522to2047Octets:
        REG_ADD(unit, port, R2047r, count);
        REG_ADD(unit, port, T2047r, count);
	break;
    case snmpBcmEtherStatsPkts2048to4095Octets:
        REG_ADD(unit, port, R4095r, count);
        REG_ADD(unit, port, T4095r, count);
	break;
    case snmpBcmEtherStatsPkts4095to9216Octets:
        REG_ADD(unit, port, R9216r, count);
        REG_ADD(unit, port, T9216r, count);
	break;
    case snmpBcmEtherStatsPkts9217to16383Octets:
        REG_ADD(unit, port, R16383r, count);
        REG_ADD(unit, port, T16383r, count);
       break;
    case snmpBcmReceivedPkts9217to16383Octets:
        REG_ADD(unit, port, R16383r, count);
       break;
    case snmpBcmTransmittedPkts9217to16383Octets:
        REG_ADD(unit, port, T16383r, count);
       break;
    case snmpEtherStatsTXNoErrors:
	REG_ADD(unit, port, TPKTr, count);
	REG_SUB(unit, port, TFRGr, count);
        REG_SUB(unit, port, TOVRr, count);
	REG_SUB(unit, port, TUFLr, count);
	REG_SUB(unit, port, TERRr, count);
	break;
    case snmpEtherStatsRXNoErrors: /* RPKT - ( RFCS + RXUO + RFLR) */
	REG_ADD(unit, port, RPKTr, count);
	REG_SUB(unit, port, RFCSr, count);
	REG_SUB(unit, port, RJBRr, count);
	REG_SUB(unit, port, ROVRr, count);
	REG_SUB(unit, port, RUNDr, count);
	REG_SUB(unit, port, RFRGr, count);
	REG_SUB(unit, port, RERPKTr, count);
	break;
	/* *** RFC 2665 *** */
    case snmpDot3StatsAlignmentErrors:
	break;
    case snmpDot3StatsFCSErrors:
	REG_ADD(unit, port, RFCSr, count);
	break;
    case snmpDot3StatsSingleCollisionFrames:
	break;
    case snmpDot3StatsMultipleCollisionFrames:
	break;
    case snmpDot3StatsSQETTestErrors:
	/* always 0 */
	break;
    case snmpDot3StatsDeferredTransmissions:
	break;
    case snmpDot3StatsLateCollisions:
	break;
    case snmpDot3StatsExcessiveCollisions:
	break;
    case snmpDot3StatsInternalMacTransmitErrors:
	REG_ADD(unit, port, TUFLr, count);
	REG_ADD(unit, port, TERRr, count);
	break;
    case snmpDot3StatsCarrierSenseErrors:
	break;
    case snmpDot3StatsFrameTooLongs:
        REG_ADD(unit, port, RMTUEr, count);
        break;
    case snmpDot3StatsInternalMacReceiveErrors:
	break;
    case snmpDot3StatsSymbolErrors:
	REG_ADD(unit, port, RERPKTr, count);
	break;
    case snmpDot3ControlInUnknownOpcodes:
	REG_ADD(unit, port, RXUOr, count);
	break;
    case snmpDot3InPauseFrames:
	REG_ADD(unit, port, RXPFr, count);
	break;
    case snmpDot3OutPauseFrames:
	REG_ADD(unit, port, TXPFr, count);
	break;

	/* *** RFC 2233 high capacity versions of RFC1213 objects *** */

    case snmpIfHCInOctets:
	REG_ADD(unit, port, RBYTr, count);
	break;
    case snmpIfHCInUcastPkts:
	REG_ADD(unit, port, RPKTr, count);
	REG_SUB(unit, port, RMCAr, count);
	REG_SUB(unit, port, RBCAr, count);
        REG_SUB(unit, port, RFCSr, count); /* - bad FCS */
        REG_SUB(unit, port, RXCFr, count); /* - good FCS, all MAC ctrl */
        REG_SUB(unit, port, RFLRr, count); /* - good FCS, bad length */
        REG_SUB(unit, port, RJBRr, count); /* - oversize, bad FCS */
        REG_SUB(unit, port, ROVRr, count); /* - oversize, good FCS */
	break;
    case snmpIfHCInMulticastPkts:
	REG_ADD(unit, port, RMCAr, count);
	break;
    case snmpIfHCInBroadcastPkts:
	REG_ADD(unit, port, RBCAr, count);
	break;
    case snmpIfHCOutOctets:
	REG_ADD(unit, port, TBYTr, count);
	break;
    case snmpIfHCOutUcastPkts:
	REG_ADD(unit, port, TPKTr, count);
	REG_SUB(unit, port, TMCAr, count);
	REG_SUB(unit, port, TBCAr, count);
	break;
    case snmpIfHCOutMulticastPkts:
	REG_ADD(unit, port, TMCAr, count);
	break;
    case snmpIfHCOutBroadcastPckts:
	REG_ADD(unit, port, TBCAr, count);
	break;

        /* *** RFC 2465 *** */

    case snmpIpv6IfStatsInReceives:
        break;
    case snmpIpv6IfStatsInHdrErrors:
        break;
    case snmpIpv6IfStatsInAddrErrors:
        break;
    case snmpIpv6IfStatsInDiscards:
        break;
    case snmpIpv6IfStatsOutForwDatagrams:
        break;
    case snmpIpv6IfStatsOutDiscards:
        break;
    case snmpIpv6IfStatsInMcastPkts:
        break;
    case snmpIpv6IfStatsOutMcastPkts:
        break;
    case snmpIeee8021PfcRequests:
        break;
    case snmpIeee8021PfcIndications:
        break;
	/* IPMC counters (broadcom specific) */
    case snmpBcmIPMCBridgedPckts:
	break;
    case snmpBcmIPMCRoutedPckts:
	break;
    case snmpBcmIPMCInDroppedPckts:
	break;
    case snmpBcmIPMCOutDroppedPckts:
	break;
    case snmpBcmCustomReceive0:
    case snmpBcmCustomReceive1:
    case snmpBcmCustomReceive2:
    case snmpBcmCustomReceive3:
    case snmpBcmCustomReceive4:
    case snmpBcmCustomReceive5:
    case snmpBcmCustomReceive6:
    case snmpBcmCustomReceive7:
    case snmpBcmCustomReceive8:
    case snmpBcmCustomTransmit0:
    case snmpBcmCustomTransmit1:
    case snmpBcmCustomTransmit2:
    case snmpBcmCustomTransmit3:
    case snmpBcmCustomTransmit4:
    case snmpBcmCustomTransmit5:
    case snmpBcmCustomTransmit6:
    case snmpBcmCustomTransmit7:
    case snmpBcmCustomTransmit8:
    case snmpBcmCustomTransmit9:
    case snmpBcmCustomTransmit10:
    case snmpBcmCustomTransmit11:
    case snmpBcmCustomTransmit12:
    case snmpBcmCustomTransmit13:
    case snmpBcmCustomTransmit14:
        break;

        /* *** RFC 1284 - unsupported in C3 *** */
      case snmpDot3StatsInRangeLengthError:
          break;

          /* *** RFC 4837 - unsupported in C3 *** */
      case snmpDot3OmpEmulationCRC8Errors:
      case snmpDot3MpcpRxGate:
      case snmpDot3MpcpRxRegister:
      case snmpDot3MpcpTxRegRequest:
      case snmpDot3MpcpTxRegAck:
      case snmpDot3MpcpTxReport:
      case snmpDot3EponFecCorrectedBlocks:
      case snmpDot3EponFecUncorrectableBlocks:
          break;

          /* EA (broadcom specific) - unsupported in C3 */
      case snmpBcmPonInDroppedOctets:
      case snmpBcmPonOutDroppedOctets:
      case snmpBcmPonInDelayedOctets:
      case snmpBcmPonOutDelayedOctets:
      case snmpBcmPonInDelayedHundredUs:
      case snmpBcmPonOutDelayedHundredUs:
      case snmpBcmPonInFrameErrors:
      case snmpBcmPonInOamFrames:
      case snmpBcmPonOutOamFrames:
      case snmpBcmPonOutUnusedOctets:
         break;

        /* Unsupported in C3 */
    case snmpBcmTxControlCells:
    case snmpBcmTxDataCells:
    case snmpBcmTxDataBytes:
    case snmpBcmRxCrcErrors:
    case snmpBcmRxFecCorrectable:
    case snmpBcmRxBecCrcErrors:
    case snmpBcmRxDisparityErrors:
    case snmpBcmRxControlCells:
    case snmpBcmRxDataCells:
    case snmpBcmRxDataBytes:
    case snmpBcmRxDroppedRetransmittedControl:
    case snmpBcmTxBecRetransmit:
    case snmpBcmRxBecRetransmit:
    case snmpBcmTxAsynFifoRate:
    case snmpBcmRxAsynFifoRate:
    case snmpBcmRxFecUncorrectable:
    case snmpBcmRxBecRxFault:
    case snmpBcmRxCodeErrors:
    case snmpBcmRxLlfcPrimary:
    case snmpBcmRxLlfcSecondary:

        break;


    default:
        LOG_VERBOSE(BSL_LS_BCM_STAT,
                    (BSL_META_U(unit,
                                "bcm_stat_get: Statistic not supported: %d\n"),
                     type));
	return BCM_E_PARAM;
    }

    *val = count;

    return rv;
}

int
bcm_caladan3_stat_get32(int unit,
                   bcm_port_t port,
                   bcm_stat_val_t type,
                   uint32 *val)
{
    int     rv = BCM_E_NONE;
    uint64  val64;

    if (!SOC_PORT_VALID(unit, port)) {
        return BCM_E_PORT;
    }

    rv = bcm_caladan3_stat_get(unit, port, type, &val64);

    COMPILER_64_TO_32_LO(*val, val64);

    return(rv);
}

/*
 * Function:
 *      bcm_caladan3_stat_multi_get
 * Purpose:
 *      Get the specified statistics from the device.
 * Parameters:
 *      unit - (IN) Unit number.
 *      port - (IN) <UNDEF>
 *      nstat - (IN) Number of elements in stat array
 *      stat_arr - (IN) Array of SNMP statistics types defined in bcm_stat_val_t
 *      value_arr - (OUT) Collected statistics values
 * Returns:
 *      BCM_E_xxx
 * Notes:
 */
int 
bcm_caladan3_stat_multi_get(int unit, bcm_port_t port, int nstat,
			  bcm_stat_val_t *stat_arr, uint64 *value_arr)
{
    int stix;

    if (nstat <= 0) {
        return BCM_E_PARAM;
    }

    if ((NULL == stat_arr) || (NULL == value_arr)) {
        return BCM_E_PARAM;
    }
    
    for (stix = 0; stix < nstat; stix++) {
        BCM_IF_ERROR_RETURN
	    (bcm_caladan3_stat_get(unit, port, stat_arr[stix],
				 &(value_arr[stix])));
    }

    return BCM_E_NONE; 
}


/*
 * Function:
 *      bcm_caladan3_stat_multi_get32
 * Purpose:
 *      Get the specified statistics from the device.  The 64-bit
 *      values may be truncated to fit.
 * Parameters:
 *      unit - (IN) Unit number.
 *      port - (IN) <UNDEF>
 *      nstat - (IN) Number of elements in stat array
 *      stat_arr - (IN) Array of SNMP statistics types defined in bcm_stat_val_t
 *      value_arr - (OUT) Collected 32-bit statistics values
 * Returns:
 *      BCM_E_xxx
 * Notes:
 */
int 
bcm_caladan3_stat_multi_get32(int unit, bcm_port_t port, int nstat,
                         bcm_stat_val_t *stat_arr, uint32 *value_arr)
{
    int stix;

    if (nstat <= 0) {
        return BCM_E_PARAM;
    }

    if ((NULL == stat_arr) || (NULL == value_arr)) {
        return BCM_E_PARAM;
    }

    for (stix = 0; stix < nstat; stix++) {
        BCM_IF_ERROR_RETURN
            (bcm_caladan3_stat_get32(unit, port, stat_arr[stix],
				   &(value_arr[stix])));
    }

    return BCM_E_NONE; 
}

#define ALLPORTS                        (SB_FAB_DEVICE_CALADAN3_SFI_LINKS+SB_FAB_DEVICE_CALADAN3_SCI_LINKS)

int 
bcm_caladan3_stat_clear(int unit,
                   bcm_port_t port)
{
    pbmp_t		pbm;

    if (!SOC_IS_CALADAN3(unit)) {
        return BCM_E_UNAVAIL;
    }

    if (!SOC_PORT_VALID(unit, port)){
        return BCM_E_PORT;
    }
    if (port == CMIC_PORT(unit)) {
        SOC_CONTROL(unit)->stat.dma_rbyt = 0;
        SOC_CONTROL(unit)->stat.dma_rpkt = 0;
        SOC_CONTROL(unit)->stat.dma_tbyt = 0;
        SOC_CONTROL(unit)->stat.dma_tpkt = 0;
        return (BCM_E_NONE);
    }


    SOC_PBMP_CLEAR(pbm);
    SOC_PBMP_PORT_ADD(pbm, port);
    BCM_IF_ERROR_RETURN(soc_counter_set32_by_port(unit, pbm, 0));

    
    return(BCM_E_NONE);
}


int
bcm_caladan3_stat_scoreboard_get(int unit,
                    uint32 *pBufLost,
                    uint32 *pBufFrees,
                    uint32 *pWatchdogErrs,
                    uint32 *pShortIntervals,
                    uint32 *pScoreboardTicks)

{
  int rv = BCM_E_UNAVAIL;
  return rv;
}
