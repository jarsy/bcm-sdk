/*
 * $Id: stat.c,v 1.19.74.1 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * Broadcom Sirius Statistics API.
 */

#include <shared/bsl.h>

#include <soc/sbx/sbx_drv.h>
#include <soc/sbx/counter.h>
#include <soc/sbx/sirius_counter.h>

#include <bcm/error.h>
#include <bcm/stat.h>

#include <bcm/error.h>

#include <bcm_int/control.h>
#include <bcm_int/common/lock.h>
#include <bcm_int/sbx/error.h>
#include <bcm_int/sbx/stat.h>


#ifndef PLISIM
#define COUNTER_FLAGS_DEFAULT	SOC_COUNTER_F_DMA
#else
#define COUNTER_FLAGS_DEFAULT	0
#endif


static int bcm_sirius_stat_custom_compute(int unit,
					  bcm_port_t port,
					  bcm_stat_val_t type,
					  uint64 *count);
static int  sirius_counter_enable_flag;

/*
 * Function:
 *     _bcm_sirius_sw_counter_init
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
_bcm_sirius_sw_counter_init(int unit)
{
    pbmp_t       pbmp, pbmp2;
    sal_usecs_t  interval;
    uint32       flags;

    if (!SOC_UNIT_VALID(unit)) {
	return BCM_E_UNIT;
    }

    if (soc_property_get_str(unit, spn_BCM_STAT_PBMP) == NULL) {
      SOC_PBMP_ASSIGN(pbmp, SOC_PORT_BITMAP(unit, hg));
      SOC_PBMP_OR(pbmp, SOC_PORT_BITMAP(unit, ge));
      SOC_PBMP_OR(pbmp, SOC_PORT_BITMAP(unit, xe));
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

    SOC_IF_ERROR_RETURN(soc_sbx_sirius_counter_init(unit, flags, interval, pbmp));

    sirius_counter_enable_flag = 0;

    return BCM_E_NONE;

}


int
bcm_sirius_stat_init(int unit)
{
    int                     result = BCM_E_NONE;
    bcm_stat_info_t         *si = NULL;

    LOG_DEBUG(BSL_LS_BCM_STAT,
              (BSL_META_U(unit,
                          "%s(%d) - Enter\n"),
               FUNCTION_NAME(), unit));

    if (!BCM_UNIT_VALID(unit) || unit >= BCM_MAX_NUM_UNITS) {
        return BCM_E_UNIT;
    }

    si = &STAT_CNTL(unit);

    si->init = TRUE;

    result = _bcm_sirius_sw_counter_init(unit);

    LOG_VERBOSE(BSL_LS_BCM_STAT,
                (BSL_META_U(unit,
                            "bcm_stat_init: unit=%d rv=%d(%s)\n"),
                 unit, result, bcm_errmsg(result)));

    return result;
}


int
bcm_sirius_stat_sync(int unit)
{
  return soc_sbx_counter_sync(unit);
}

/*
 * Function:
 *	bcm_stat_get
 * Description:
 *	Get the specified statistic for a HG port on the StrataSwitch family
 *      of devices.
 * Parameters:
 *	unit - StrataSwitch PCI device unit number (driver internal).
 *	port - zero-based port number
 *	type - SNMP statistics type (see stat.h)
 * Returns:
 *	BCM_E_NONE - Success.
 *	BCM_E_PARAM - Illegal parameter.
 *	BCM_E_INTERNAL - Chip access failure.
 */

int
bcm_sirius_stat_get(int unit,
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

    switch (type) {
    case snmpIfInOctets:
	REG_ADD(unit, port, IRBYTr, count);
	break;
    case snmpIfInUcastPkts:
	REG_ADD(unit, port, IRPKTr, count);
	REG_SUB(unit, port, IRMCAr, count); /* - multicast */
	REG_SUB(unit, port, IRBCAr, count); /* - broadcast */
        REG_SUB(unit, port, IRFCSr, count); /* - bad FCS */
        REG_SUB(unit, port, IRXCFr, count); /* - good FCS, all MAC ctrl */
        REG_SUB(unit, port, IRFLRr, count); /* - good FCS, bad length */
        REG_SUB(unit, port, IRJBRr, count); /* - oversize, bad FCS */
	REG_SUB(unit, port, IROVRr, count); /* - oversize, good FCS */
	break;
    case snmpIfInNUcastPkts:	/* Multicast frames plus broadcast frames */
	REG_ADD(unit, port, IRMCAr, count); /* + multicast */
	REG_ADD(unit, port, IRBCAr, count); /* + broadcast */
	break;
    case snmpIfInDiscards:	/* Not supported */
	break;
    case snmpIfInErrors: /* RX Errors or Receive packets - non-error frames */
	REG_ADD(unit, port, IRFCSr, count);
	REG_ADD(unit, port, IRJBRr, count);
	REG_ADD(unit, port, IROVRr, count);
	REG_ADD(unit, port, IRUNDr, count);
	REG_ADD(unit, port, IRFRGr, count);
	break;
    case snmpIfInUnknownProtos:	/* Not supported */
	break;
    case snmpIfOutOctets:	/* TX bytes */
	REG_ADD(unit, port, ITBYTr, count);
	break;
    case snmpIfOutUcastPkts:	/* ALL - mcast - bcast */
	REG_ADD(unit, port, ITPKTr, count);
	REG_SUB(unit, port, ITMCAr, count); /* - multicast */
	REG_SUB(unit, port, ITBCAr, count); /* - broadcast */
	break;
    case snmpIfOutNUcastPkts:	/* broadcast frames plus multicast frames */
	REG_ADD(unit, port, ITMCAr, count); /* + multicast */
	REG_ADD(unit, port, ITBCAr, count); /* + broadcast */
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
        REG_ADD(unit, port, IROVRr, count);     /* oversize pkts */
	REG_ADD(unit, port, ITOVRr, count);     /* oversize pkts */
	break;
    case snmpDot1dTpPortInFrames:
	REG_ADD(unit, port, IRPKTr, count);
	break;
    case snmpDot1dTpPortOutFrames:
	REG_ADD(unit, port, ITPKTr, count);
	break;
    case snmpDot1dPortInDiscards:
	break;

	/* *** RFC 1757 *** */

    case snmpEtherStatsDropEvents:
	break;
    case snmpEtherStatsOctets:
	REG_ADD(unit, port, IRBYTr, count);
	REG_ADD(unit, port, ITBYTr, count);
	break;
    case snmpEtherStatsPkts:
	REG_ADD(unit, port, IRPKTr, count);
	REG_ADD(unit, port, ITPKTr, count);
        REG_ADD(unit, port, IRUNDr, count); /* Runts */
        REG_ADD(unit, port, IRFRGr, count); /* Fragments */
	break;
    case snmpEtherStatsBroadcastPkts:	/* Broadcast packets (RX/TX) */
	REG_ADD(unit, port, IRBCAr, count); /* + rx broadcast  */
	REG_ADD(unit, port, ITBCAr, count); /* + tx broadcast */
	break;
    case snmpEtherStatsMulticastPkts:	/* Multicast packets (TX+RX) */
	REG_ADD(unit, port, IRMCAr, count); /* + rx multicast */
	REG_ADD(unit, port, ITMCAr, count); /* + tx multicast */
	break;
    case snmpEtherStatsCRCAlignErrors:
	REG_ADD(unit, port, IRFCSr, count);
	break;
    case snmpEtherStatsUndersizePkts:	/* Undersize frames */
	REG_ADD(unit, port, IRUNDr, count);
	break;
    case snmpEtherStatsOversizePkts:
        REG_ADD(unit, port, IROVRr, count);
	REG_ADD(unit, port, ITOVRr, count);
	break;
    case snmpEtherStatsFragments:
	REG_ADD(unit, port, IRFRGr, count);
	break;
    case snmpEtherStatsJabbers:
	REG_ADD(unit, port, IRJBRr, count);
	break;
    case snmpEtherStatsCollisions:
	/* always 0 */
	break;
    case snmpEtherStatsPkts64Octets:
        REG_ADD(unit, port, IR64r, count);
        REG_ADD(unit, port, IT64r, count);
	break;
    case snmpEtherStatsPkts65to127Octets:
        REG_ADD(unit, port, IR127r, count);
        REG_ADD(unit, port, IT127r, count);
	break;
    case snmpEtherStatsPkts128to255Octets:
        REG_ADD(unit, port, IR255r, count);
        REG_ADD(unit, port, IT255r, count);
	break;
    case snmpEtherStatsPkts256to511Octets:
        REG_ADD(unit, port, IR511r, count);
        REG_ADD(unit, port, IT511r, count);
	break;
    case snmpEtherStatsPkts512to1023Octets:
        REG_ADD(unit, port, IR1023r, count);
        REG_ADD(unit, port, IT1023r, count);
	break;
    case snmpEtherStatsPkts1024to1518Octets:
        REG_ADD(unit, port, IR1518r, count);
        REG_ADD(unit, port, IT1518r, count);
	break;

	/* *** not actually in rfc1757 *** */

    case snmpBcmEtherStatsPkts1519to1522Octets:
	    break;
    case snmpBcmEtherStatsPkts1522to2047Octets:
        REG_ADD(unit, port, IR2047r, count);
        REG_ADD(unit, port, IT2047r, count);
	break;
    case snmpBcmEtherStatsPkts2048to4095Octets:
        REG_ADD(unit, port, IR4095r, count);
        REG_ADD(unit, port, IT4095r, count);
	break;
    case snmpBcmEtherStatsPkts4095to9216Octets:
        REG_ADD(unit, port, IR9216r, count);
        REG_ADD(unit, port, IT9216r, count);
	break;

    case snmpEtherStatsTXNoErrors:
	REG_ADD(unit, port, ITPKTr, count);
	REG_SUB(unit, port, ITFRGr, count);
        REG_SUB(unit, port, ITOVRr, count);
	REG_SUB(unit, port, ITUFLr, count);
	REG_SUB(unit, port, ITERRr, count);
	break;
    case snmpEtherStatsRXNoErrors: /* RPKT - ( RFCS + RXUO + RFLR) */
	REG_ADD(unit, port, IRPKTr, count);
	REG_SUB(unit, port, IRFCSr, count);
	REG_SUB(unit, port, IRJBRr, count);
	REG_SUB(unit, port, IROVRr, count);
	REG_SUB(unit, port, IRUNDr, count);
	REG_SUB(unit, port, IRFRGr, count);
	REG_SUB(unit, port, IRERPKTr, count);
	break;

	/* *** RFC 2665 *** */

    case snmpDot3StatsAlignmentErrors:
	break;
    case snmpDot3StatsFCSErrors:
	REG_ADD(unit, port, IRFCSr, count);
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
	REG_ADD(unit, port, ITUFLr, count);
	REG_ADD(unit, port, ITERRr, count);
	break;
    case snmpDot3StatsCarrierSenseErrors:
	break;
    case snmpDot3StatsFrameTooLongs:
        if (SOC_REG_IS_VALID(unit, IRMEGr) && SOC_REG_IS_VALID(unit, IRMEBr)) {
             REG_ADD(unit, port, IRMEGr, count);
             REG_ADD(unit, port, IRMEBr, count);
        } else {
             REG_ADD(unit, port, IRJBRr, count);
        }
        break;
    case snmpDot3StatsInternalMacReceiveErrors:
	break;
    case snmpDot3StatsSymbolErrors:
	REG_ADD(unit, port, IRERBYTr, count);
	break;
    case snmpDot3ControlInUnknownOpcodes:
	REG_ADD(unit, port, IRXUOr, count);
	break;
    case snmpDot3InPauseFrames:
	REG_ADD(unit, port, IRXPFr, count);
	break;
    case snmpDot3OutPauseFrames:
	REG_ADD(unit, port, ITXPFr, count);
	break;

	/* *** RFC 2233 high capacity versions of RFC1213 objects *** */

    case snmpIfHCInOctets:
	REG_ADD(unit, port, IRBYTr, count);
	break;
    case snmpIfHCInUcastPkts:
	REG_ADD(unit, port, IRPKTr, count);
	REG_SUB(unit, port, IRMCAr, count);
	REG_SUB(unit, port, IRBCAr, count);
        REG_SUB(unit, port, IRFCSr, count); /* - bad FCS */
        REG_SUB(unit, port, IRXCFr, count); /* - good FCS, all MAC ctrl */
        REG_SUB(unit, port, IRFLRr, count); /* - good FCS, bad length */
        REG_SUB(unit, port, IRJBRr, count); /* - oversize, bad FCS */
        REG_SUB(unit, port, IROVRr, count); /* - oversize, good FCS */
	break;
    case snmpIfHCInMulticastPkts:
	REG_ADD(unit, port, IRMCAr, count);
	break;
    case snmpIfHCInBroadcastPkts:
	REG_ADD(unit, port, IRBCAr, count);
	break;
    case snmpIfHCOutOctets:
	REG_ADD(unit, port, ITBYTr, count);
	break;
    case snmpIfHCOutUcastPkts:
	REG_ADD(unit, port, ITPKTr, count);
	REG_SUB(unit, port, ITMCAr, count);
	REG_SUB(unit, port, ITBCAr, count);
	break;
    case snmpIfHCOutMulticastPkts:
	REG_ADD(unit, port, ITMCAr, count);
	break;
    case snmpIfHCOutBroadcastPckts:
	REG_ADD(unit, port, ITBCAr, count);
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
	rv = bcm_sirius_stat_custom_compute(unit, port, type, &count);
        break;
    case snmpBcmCustomTransmit0:
        break;
    case snmpBcmCustomTransmit1:
        break;
    case snmpBcmCustomTransmit2:
        break;
    case snmpBcmCustomTransmit3:
        break;
    case snmpBcmCustomTransmit4:
        break;
    case snmpBcmCustomTransmit5:
        break;
    case snmpBcmCustomTransmit6:
        break;
    case snmpBcmCustomTransmit7:
        break;
    case snmpBcmCustomTransmit8:
        break;
    case snmpBcmCustomTransmit9:
        break;
    case snmpBcmCustomTransmit10:
        break;
    case snmpBcmCustomTransmit11:
        break;
    case snmpBcmCustomTransmit12:
        break;
    case snmpBcmCustomTransmit13:
        break;
    case snmpBcmCustomTransmit14:
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
bcm_sirius_stat_get32(int unit,
                   bcm_port_t port,
                   bcm_stat_val_t type,
                   uint32 *val)
{
    int     rv = BCM_E_NONE;
    uint64  val64;

    if (!SOC_PORT_VALID(unit, port)) {
        return BCM_E_PORT;
    }

    rv = bcm_sirius_stat_get(unit, port, type, &val64);

    COMPILER_64_TO_32_LO(*val, val64);

    return(rv);
}

/*
 * Function:
 *      bcm_stat_multi_get
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
bcm_sirius_stat_multi_get(int unit, bcm_port_t port, int nstat, 
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
	    (bcm_sirius_stat_get(unit, port, stat_arr[stix],
				 &(value_arr[stix])));
    }

    return BCM_E_NONE; 
}


/*
 * Function:
 *      bcm_stat_multi_get32
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
bcm_sirius_stat_multi_get32(int unit, bcm_port_t port, int nstat, 
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
            (bcm_sirius_stat_get32(unit, port, stat_arr[stix],
				   &(value_arr[stix])));
    }

    return BCM_E_NONE; 
}

#define ALLPORTS                        (SB_FAB_DEVICE_SIRIUS_SFI_LINKS+SB_FAB_DEVICE_SIRIUS_SCI_LINKS)

int 
bcm_sirius_stat_clear(int unit,
                   bcm_port_t port)
{
    pbmp_t		pbm;
    soc_sbx_config_t *sbx = SOC_SBX_CFG(unit);
    uint32 *bp = NULL;
    int base = 0;

    if (!SOC_IS_SIRIUS(unit)) {
        return BCM_E_UNAVAIL;
    }

    if (!SOC_PORT_VALID(unit, port)){
        return BCM_E_PORT;
    }

    SOC_PBMP_CLEAR(pbm);
    SOC_PBMP_PORT_ADD(pbm, port);
    SOC_PBMP_AND(pbm, SOC_PORT_BITMAP(unit, hg));
    BCM_IF_ERROR_RETURN(soc_counter_set32_by_port(unit, pbm, 0));

    /*  
     * If SFI or SCI, then clear custom stats
     */

    if (IS_SFI_PORT(unit, port) || IS_SCI_PORT(unit, port)) {
	for (base=0; base <= (bcmDbgCntBIT_INTERLEAVED_PARITY_ODD - bcmDbgCntRX_SYMBOL); base++) {
	    bp = (uint32 *) &sbx->custom_stats[base * ALLPORTS];
	    bp[port] = 0;
	}
    }
    
    return(BCM_E_NONE);
}

/*
 * Function:
 *      bcm_sirius_stat_custom_set 
 * Description:
 *      Set debug counter to count certain packet types.
 * Parameters:
 *      unit  - StrataSwitch PCI device unit number.
 *      port  - Port number
 *      type  - SNMP statistics type.
 *      flags - The counter select value (see stat.h for bit definitions). 
 * Returns:
 *      BCM_E_XXX
 */

int
bcm_sirius_stat_custom_set(int unit,
			   bcm_port_t port,
			   bcm_stat_val_t type,
			   uint32 flags)
{
    int rv = BCM_E_UNAVAIL;

    if (!SOC_IS_SIRIUS(unit)) {
        return BCM_E_UNAVAIL;
    }

    if ((type < snmpBcmCustomReceive0) || (type > snmpBcmCustomReceive8)) {
        return BCM_E_PARAM;
    }

    /*  valid port  */
    if (!IS_SFI_PORT(unit, port) && !IS_SCI_PORT(unit, port)) {
        return BCM_E_PORT;
    }

    return rv;
}

/*
 * Function:
 *      bcm_sirius_stat_custom_get 
 * Description:
 *      Get debug counter select value.
 * Parameters:
 *      unit  - StrataSwitch PCI device unit number.
 *      port  - Port number
 *      type  - SNMP statistics type.
 *      flags - (OUT) The counter select value
 *		      (see stat.h for bit definitions).
 * Returns:
 *      BCM_E_XXX
 */

int
bcm_sirius_stat_custom_get(int unit,
			   bcm_port_t port,
			   bcm_stat_val_t type,
			   uint32 *flags)
{
    int rv = BCM_E_UNAVAIL;

    if (!SOC_IS_SIRIUS(unit)) {
        return BCM_E_UNAVAIL;
    }

    if (flags == NULL) {
	return BCM_E_RESOURCE;
    }

    if ((type < snmpBcmCustomReceive0) || (type > snmpBcmCustomReceive8)) {
        return BCM_E_PARAM;
    }


    /* valid port */
    if (!IS_SFI_PORT(unit, port) && !IS_SCI_PORT(unit, port)) {
        return BCM_E_PORT;
    }

    return rv;
}

/*
 * Function:
 *      bcm_sirius_stat_custom_add 
 * Description:
 *      Add a certain packet type to debug counter to count 
 * Parameters:
 *      unit  - StrataSwitch PCI device unit number.
 *      port  - Port number, -1 = all
 *      type  - SNMP statistics type.
 *      trigger - The counter select value
 *		      (see stat.h for bit definitions).
 * Returns:
 *      BCM_E_XXX
 */
int 
bcm_sirius_stat_custom_add(int unit,
			   bcm_port_t port,
			   bcm_stat_val_t type,
			   bcm_custom_stat_trigger_t trigger)
{
    int rv = BCM_E_NONE;
    soc_sbx_config_t *sbx  = SOC_SBX_CFG(unit);
    int	base = 0;

    if (!SOC_IS_SIRIUS(unit)) {
        return BCM_E_UNAVAIL;
    }

    if ((type < snmpBcmCustomReceive0) || (type > snmpBcmCustomReceive8)) {
        return BCM_E_PARAM;
    }

    if ((trigger < bcmDbgCntRX_SYMBOL) || (trigger > bcmDbgCntBIT_INTERLEAVED_PARITY_ODD)) {
        return BCM_E_PARAM;
    }

    /* port = -1 is valid port in that case */
    if (!IS_SFI_PORT(unit, port) && !IS_SCI_PORT(unit, port) && (port != -1)) {
        return BCM_E_PORT;
    }

    /* Decrement base SFI port to 0 base */
    if (port > 0)
	port -= SOC_PORT_MIN(unit, sfi);

    type -= snmpBcmCustomReceive0;
    base = trigger - bcmDbgCntRX_SYMBOL;

    if (port == -1) {
	SHR_BITSET_RANGE(sbx->soc_sbx_dbg_cntr_rx[type].ports, base * SHR_BITWID, ALLPORTS);
	sbx->soc_sbx_dbg_cntr_rx[type].trigger |= (1 << base);
    } else {
	sbx->soc_sbx_dbg_cntr_rx[type].trigger |= (1 << base);
	SHR_BITSET(sbx->soc_sbx_dbg_cntr_rx[type].ports, ((base * SHR_BITWID) + port));
    }
    
    return rv;
}

/*
 * Function:
 *      bcm_sirius_stat_custom_delete 
 * Description:
 *      Deletes a certain packet type from debug counter  
 * Parameters:
 *      unit  - StrataSwitch PCI device unit number.
 *      port  - Port number, -1 = all 
 *      type  - SNMP statistics type.
 *      trigger - The counter select value
 *		      (see stat.h for bit definitions).
 * Returns:
 *      BCM_E_XXX
 */

int 
bcm_sirius_stat_custom_delete(int unit,
			      bcm_port_t port,
			      bcm_stat_val_t type, 
			      bcm_custom_stat_trigger_t trigger)
{
    int rv = BCM_E_NONE;
    soc_sbx_config_t *sbx  = SOC_SBX_CFG(unit);
    int	base = 0;

    if (!SOC_IS_SIRIUS(unit)) {
        return BCM_E_UNAVAIL;
    }

    if ((type < snmpBcmCustomReceive0) || (type > snmpBcmCustomReceive8)) {
        return BCM_E_PARAM;
    }

    if ((trigger < bcmDbgCntRX_SYMBOL) || (trigger > bcmDbgCntBIT_INTERLEAVED_PARITY_ODD)) {
        return BCM_E_PARAM;
    }

    /* port = -1 is valid port in that case */
    if (!IS_SFI_PORT(unit, port) && !IS_SCI_PORT(unit, port) && (port != -1)) {
        return BCM_E_PORT;
    }

    /* Decrement base SFI port to 0 base */
    if (port > 0)
	port -= SOC_PORT_MIN(unit, sfi);

    type -= snmpBcmCustomReceive0;
    base = trigger - bcmDbgCntRX_SYMBOL;

    if (port == -1) {
	SHR_BITCLR_RANGE(sbx->soc_sbx_dbg_cntr_rx[type].ports, base * SHR_BITWID, ALLPORTS);
	sbx->soc_sbx_dbg_cntr_rx[type].trigger &= ~(1<<base);
    } else {
	SHR_BITCLR(sbx->soc_sbx_dbg_cntr_rx[type].ports, ((base * SHR_BITWID) + port));
	if (sbx->soc_sbx_dbg_cntr_rx[type].ports[base] == 0)
	    sbx->soc_sbx_dbg_cntr_rx[type].trigger &= ~(1<<base);
    }
    
    return rv;
}

/*
 * Function:
 *      bcm_sirius_stat_custom_delete_all
 * Description:
 *      Deletes all packet types from debug counter  
 * Parameters:
 *      unit  - StrataSwitch PCI device unit number.
 *      port  - Port number
 *      type  - SNMP statistics type.
 *
 * Returns:
 *      BCM_E_XXX
 */
int 
bcm_sirius_stat_custom_delete_all(int unit,
				  bcm_port_t port,
				  bcm_stat_val_t type)
{
    int rv = BCM_E_NONE;
    soc_sbx_config_t *sbx  = SOC_SBX_CFG(unit);
    int	base = 0;

    if (!SOC_IS_SIRIUS(unit)) {
        return BCM_E_UNAVAIL;
    }

    if ((type < snmpBcmCustomReceive0) || (type > snmpBcmCustomReceive8)) {
        return BCM_E_PARAM;
    }

    /* port = -1 is valid port in that case */
    if (!IS_SFI_PORT(unit, port) && !IS_SCI_PORT(unit, port) && (port != -1)) {
        return BCM_E_PORT;
    }

    /* Decrement base SFI port to 0 base */
    if (port > 0)
	port -= SOC_PORT_MIN(unit, sfi);

    type -= snmpBcmCustomReceive0;

    for (base=0; base <= (bcmDbgCntBIT_INTERLEAVED_PARITY_ODD - bcmDbgCntRX_SYMBOL); base++) {
	if (port == -1) {
	    SHR_BITCLR_RANGE(sbx->soc_sbx_dbg_cntr_rx[type].ports, base * SHR_BITWID, ALLPORTS);
	    sbx->soc_sbx_dbg_cntr_rx[type].trigger &= ~(1<<base);
	} else {
	    SHR_BITCLR(sbx->soc_sbx_dbg_cntr_rx[type].ports, ((base * SHR_BITWID) + port));
		    if (sbx->soc_sbx_dbg_cntr_rx[type].ports[base] == 0)
			sbx->soc_sbx_dbg_cntr_rx[type].trigger &= ~(1<<base);
	}
    }
    
    return rv;
}

/*
 * Function:
 *      bcm_sirius_stat_custom_check
 * Description:
 *      Check if certain packet types is part of debug counter  
 * Parameters:
 *      unit  - StrataSwitch PCI device unit number.
 *      port  - Port number 
 *      type  - SNMP statistics type.
 *      trigger - The counter select value
 *		      (see stat.h for bit definitions).
 *      result - [OUT] result of a query. 0 if positive , -1 if negative
 *
 * Returns:
 *      BCM_E_XXX
 */
int 
bcm_sirius_stat_custom_check(int unit,
			     bcm_port_t port,
			     bcm_stat_val_t type, 
			     bcm_custom_stat_trigger_t trigger,
			     int *result)
{
    int rv = BCM_E_NONE;
    soc_sbx_config_t *sbx  = SOC_SBX_CFG(unit);
    int	base = 0;
   
    if (result == NULL) {
	return BCM_E_RESOURCE;
    }

    if (!SOC_IS_SIRIUS(unit)) {
        return BCM_E_UNAVAIL;
    }

    if ((type < snmpBcmCustomReceive0) || (type > snmpBcmCustomReceive8)) {
        return BCM_E_PARAM;
    }

    if ((trigger < bcmDbgCntRX_SYMBOL) || (trigger > bcmDbgCntBIT_INTERLEAVED_PARITY_ODD)) {
        return BCM_E_PARAM;
    }

    /* port = -1 is valid port in that case */
    if (!IS_SFI_PORT(unit, port) && !IS_SCI_PORT(unit, port)) {
        return BCM_E_PORT;
    }

    /* Decrement base SFI port to 0 base */
    if (port > 0)
	port -= SOC_PORT_MIN(unit, sfi);

    type -= snmpBcmCustomReceive0;
    base = trigger - bcmDbgCntRX_SYMBOL;

    if (SHR_BITGET(sbx->soc_sbx_dbg_cntr_rx[type].ports, ((base * SHR_BITWID) + port))) {
	*result = 0;
    } else {
	*result = -1;
    }

    return rv;
}

/*
 * Function:
 *      bcm_sirius_stat_custom_compute
 * Description:
 *      Calculate debug counter value.
 * Parameters:
 *      unit  - StrataSwitch PCI device unit number.
 *      port  - Port number
 *      type  - SNMP statistics type.
 *      flags - (OUT) The counter select value
 *		      (see stat.h for bit definitions).
 * Returns:
 *      BCM_E_XXX
 */

int
bcm_sirius_stat_custom_compute(int unit,
			       bcm_port_t port,
			       bcm_stat_val_t type,
			       uint64 *count)
{
    int rv = BCM_E_NONE;
    soc_sbx_config_t *sbx  = SOC_SBX_CFG(unit);
    uint32 *bp = NULL;
    int base = 0, ports = 0, offset = 0, begin = 0, end = 0;
    uint32 value = 0;

    if (!SOC_IS_SIRIUS(unit)) {
        return BCM_E_UNAVAIL;
    }

    if (count == NULL) {
	return BCM_E_RESOURCE;
    }

    if (IS_SFI_PORT(unit, port) || IS_SCI_PORT(unit, port)) {
      begin = port - SOC_PORT_MIN(unit, sfi);
      end = begin + 1;
    } else {
      return BCM_E_PORT;
    }

    offset = type - snmpBcmCustomReceive0;

    for (base=0; base <= (bcmDbgCntBIT_INTERLEAVED_PARITY_ODD - bcmDbgCntRX_SYMBOL); base++) {
	if ((sbx->soc_sbx_dbg_cntr_rx[offset].trigger & (1<<base)) == 0) {
	    continue;
	}
	bp = (uint32 *) &sbx->custom_stats[base * ALLPORTS];
	for (ports = begin; ports < end; ports++) {
	    if (SHR_BITGET(sbx->soc_sbx_dbg_cntr_rx[offset].ports, ((base * SHR_BITWID) + ports))) {
		value += bp[ports];
	    }
	}
    }

    COMPILER_64_ADD_32(*count, value);

    return rv;
}

int
bcm_sirius_stat_scoreboard_get(int unit,
                    uint32 *pBufLost,
                    uint32 *pBufFrees,
                    uint32 *pWatchdogErrs,
                    uint32 *pShortIntervals,
                    uint32 *pScoreboardTicks)

{
  int rv = BCM_E_UNAVAIL;
  return rv;
}
