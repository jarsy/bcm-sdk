/*
 * $Id: stat.c,v 1.14 Broadcom SDK $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:     stat.c
 * Purpose:
 *
 */
#include <bcm_int/ea/stat.h>
#include <bcm_int/ea/tk371x/port.h>
#include <bcm_int/ea/init.h>
#include <bcm/stat.h>
#include <bcm/error.h>
#include <soc/debug.h>
#include <soc/error.h>
#include <soc/types.h>
#include <soc/debug.h>
#include <soc/drv.h>
#include <soc/ea/tk371x/counter.h>
#include <soc/ea/tk371x/onu.h>
#include <soc/ea/tk371x/ea_drv.h>

STATIC _bcm_ea_stat_table_entry_t stat_table[_BCM_EA_STAT_PORT_MAX][_BCM_EA_S_MAX];
STATIC _bcm_ea_stat_table_entry_t init_val[_BCM_EA_S_MAX] = {
	{snmpIfInOctets, 					socEaStatMacOctetsRxOkId, 0},
	{snmpEtherStatsRXNoErrors, 			socEaStatMacFramesRxOk, 0},
	{snmpIfInUcastPkts, 				socEaStatMacRxUnicastFrames, 0},
	{snmpIfInMulticastPkts, 			socEaStatMacMcastFramesRxOk, 0},
	{snmpIfInBroadcastPkts, 			socEaStatMacBcastFramesRxOk, 0},
	{snmpDot3StatsFCSErrors, 			socEaStatMacFcsErr, 0},
	{snmpDot3OmpEmulationCRC8Errors, 	socEaStatOamEmulCrc8Err, 0},
	{snmpDot3StatsSymbolErrors, 		socEaStatPhySymbolErrDuringCarrier, 0},
	{snmpEtherStatsUndersizePkts, 		socEaStatAttrRxFrameTooShort, 0},
	{snmpDot3StatsFrameTooLongs, 		socEaStatMacFrameTooLong, 0},
	{snmpDot3StatsInRangeLengthError, 	socEaStatMacInRangeLenErr, 0},
	{snmpDot3StatsAlignmentErrors, 		socEaStatMacAlignErr, 0},
	{snmpEtherStatsPkts64Octets, 		socEaStatRxFrame64, 0},
	{snmpEtherStatsPkts65to127Octets, 	socEaStatRxFrame65_127, 0},
	{snmpEtherStatsPkts128to255Octets, 	socEaStatRxFrame128_255, 0},
	{snmpEtherStatsPkts256to511Octets, 	socEaStatRxFrame256_511, 0},
	{snmpEtherStatsPkts512to1023Octets, socEaStatRxFrame512_1023, 0},
	{snmpEtherStatsPkts1024to1518Octets,socEaStatRxFrame1024_1518, 0},
	{snmpEtherRxOversizePkts, 			socEaStatRxFrame1519Plus, 0},
	{snmpBcmPonInDroppedOctets,    		socEaStatRxBytesDropped, 0},
	{snmpIfInDiscards, 					socEaStatRxFramesDropped, 0},
	{snmpBcmPonInDelayedOctets,     	socEaStatRxBytesDelayed, 0},
	{snmpBcmPonInDelayedHundredUs,		socEaStatRxDelay, 0},
	{snmpDot3InPauseFrames, 			socEaStatMacCtrlPauseRx, 0},
	{snmpBcmPonInFrameErrors, 			socEaStatOamLocalErrFrameSecsEvent, 0},
	{snmpBcmPonInOamFrames, 			socEaStatMpcpMACCtrlFramesRx, 0},
	{snmpDot3MpcpRxGate, 				socEaStatMpcpRxGate, 0},
	{snmpDot3MpcpRxRegister, 			socEaStatMpcpRxRegister, 0},
	{snmpDot3EponFecCorrectedBlocks, 	socEaStatFecCorrectedBlocks, 0},
	{snmpDot3EponFecUncorrectableBlocks,socEaStatFecUncorrectableBlocks, 0},
	{snmpIfOutOctets, 					socEaStatMacOctetsTxOk, 0},
	{snmpEtherStatsTXNoErrors, 			socEaStatMacFramesTxOk, 0},
	{snmpIfOutUcastPkts, 				socEaStatTxUnicastFrames, 0},
	{snmpIfOutMulticastPkts, 			socEaStatMacMcastFramesTxOk, 0},
	{snmpIfOutBroadcastPkts, 			socEaStatMacBcastFramesTxOk, 0},
	{snmpDot3StatsSingleCollisionFrames,socEaStatMacSingleCollFrames, 0},
	{snmpDot3StatsMultipleCollisionFrames, socEaStatMacMultipleCollFrames, 0},
	{snmpDot3StatsLateCollisions, 		socEaStatMacLateCollisions, 0},
	{snmpDot3StatsExcessiveCollisions, 	socEaStatMacExcessiveCollisions, 0},
	{snmpBcmTransmittedPkts64Octets,	socEaStatTxFrame64, 0},
	{snmpBcmTransmittedPkts65to127Octets, socEaStatTxFrame65_127, 0},
	{snmpBcmTransmittedPkts128to255Octets, socEaStatTxFrame128_255, 0},
	{snmpBcmTransmittedPkts256to511Octets, socEaStatTxFrame256_511, 0},
	{snmpBcmTransmittedPkts512to1023Octets, socEaStatTxFrame512_1023, 0},
	{snmpBcmTransmittedPkts1024to1518Octets, socEaStatTxFrame1024_1518, 0},
	{snmpBcmTransmittedPkts1519to2047Octets, socEaStatTxFrame1519Plus, 0},
	{snmpBcmPonOutDroppedOctets, 	    socEaStatTxBytesDropped, 0},
	{snmpIfOutDiscards, 				socEaStatTxFramesDropped, 0},
	{snmpBcmPonOutDelayedOctets, 		socEaStatTxBytesDelayed, 0},
	{snmpBcmPonOutDelayedHundredUs, 	socEaStatTxDelay, 0},
	{snmpBcmPonOutUnusedOctets, 			socEaStatTxBytesUnused, 0},
	{snmpDot3OutPauseFrames, 			socEaStatMacCtrlPauseTx, 0},
	{snmpBcmPonOutOamFrames, 			socEaStatMpcpMACCtrlFramesTx, 0},
	{snmpDot3MpcpTxRegAck, 				socEaStatMpcpTxRegAck, 0},
	{snmpDot3MpcpTxRegRequest, 			socEaStatMpcpTxRegRequest, 0},
	{snmpDot3MpcpTxReport, 				socEaStatMpcpTxReport, 0},
	{-1, 								-1, 					0}
};


STATIC int
_bcm_ea_stat_zero(bcm_port_t port)
{
	int i = 0;

	for (i = 0; i < _BCM_EA_S_MAX; i++){
		stat_table[port][i].value = 0;
	}
	return BCM_E_NONE;
}

STATIC int
_bcm_ea_stat_check_rv_value(int rv){
	if (OK == rv){
		return BCM_E_NONE;
	}else if (ERROR == rv){
		return BCM_E_FAIL;
	}else{
		return BCM_E_PARAM;
	}
}

STATIC int
_bcm_ea_stat_check_port_val(
		bcm_port_t port,
		bcm_stat_val_t val)
{
	switch (port){
		case _BCM_EA_STAT_EPON:
			switch (val){
				case snmpIfInOctets:
				case snmpEtherStatsRXNoErrors:
				case snmpIfInUcastPkts:
				case snmpIfInMulticastPkts:
				case snmpIfInBroadcastPkts:
				case snmpDot3StatsFCSErrors:
				case snmpDot3OmpEmulationCRC8Errors:
				case snmpDot3StatsSymbolErrors:
				case snmpEtherStatsUndersizePkts:
				case snmpEtherStatsPkts64Octets:
				case snmpEtherStatsPkts65to127Octets:
				case snmpEtherStatsPkts128to255Octets:
				case snmpEtherStatsPkts256to511Octets:
				case snmpEtherStatsPkts512to1023Octets:
				case snmpEtherStatsPkts1024to1518Octets:
				case snmpEtherRxOversizePkts:
				case snmpBcmPonInDroppedOctets:
				case snmpIfInDiscards:
				case snmpBcmPonInDelayedOctets:
				case snmpBcmPonInDelayedHundredUs:
				case snmpBcmPonInFrameErrors:
				case snmpDot3EponFecCorrectedBlocks:
				case snmpDot3EponFecUncorrectableBlocks:
				case snmpIfOutOctets:
				case snmpEtherStatsTXNoErrors:
				case snmpIfOutUcastPkts:
				case snmpIfOutMulticastPkts:
				case snmpIfOutBroadcastPkts:
				case snmpBcmTransmittedPkts64Octets:
				case snmpBcmTransmittedPkts65to127Octets:
				case snmpBcmTransmittedPkts128to255Octets:
				case snmpBcmTransmittedPkts256to511Octets:
				case snmpBcmTransmittedPkts512to1023Octets:
				case snmpBcmTransmittedPkts1024to1518Octets:
				case snmpBcmTransmittedPkts1519to2047Octets:
				case snmpBcmPonOutDroppedOctets:
				case snmpIfOutDiscards:
				case snmpBcmPonOutDelayedOctets:
				case snmpBcmPonOutDelayedHundredUs:
				case snmpBcmPonOutUnusedOctets:
					return BCM_E_NONE;
				default:
					return BCM_E_PARAM;
			}
		case _BCM_EA_STAT_UNI0:
		case _BCM_EA_STAT_UNI1:
			switch (val){
				case snmpIfInOctets:
				case snmpEtherStatsRXNoErrors:
				case snmpIfInUcastPkts:
				case snmpIfInMulticastPkts:
				case snmpIfInBroadcastPkts:
				case snmpDot3StatsFCSErrors:
				case snmpEtherStatsUndersizePkts:
				case snmpDot3StatsFrameTooLongs:
				case snmpDot3StatsInRangeLengthError:
				case snmpDot3StatsAlignmentErrors:
				case snmpEtherStatsPkts64Octets:
				case snmpEtherStatsPkts65to127Octets:
				case snmpEtherStatsPkts128to255Octets:
				case snmpEtherStatsPkts256to511Octets:
				case snmpEtherStatsPkts512to1023Octets:
				case snmpEtherStatsPkts1024to1518Octets:
				case snmpEtherRxOversizePkts:
				case snmpDot3InPauseFrames:
				case snmpIfOutOctets:
				case snmpEtherStatsTXNoErrors:
				case snmpIfOutUcastPkts:
				case snmpIfOutMulticastPkts:
				case snmpIfOutBroadcastPkts:
				case snmpDot3StatsSingleCollisionFrames:
				case snmpDot3StatsMultipleCollisionFrames:
				case snmpDot3StatsLateCollisions:
				case snmpDot3StatsExcessiveCollisions:
				case snmpBcmTransmittedPkts64Octets:
				case snmpBcmTransmittedPkts65to127Octets:
				case snmpBcmTransmittedPkts128to255Octets:
				case snmpBcmTransmittedPkts256to511Octets:
				case snmpBcmTransmittedPkts512to1023Octets:
				case snmpBcmTransmittedPkts1024to1518Octets:
				case snmpBcmTransmittedPkts1519to2047Octets:
			    case snmpBcmPonOutDroppedOctets:
				case snmpIfOutDiscards:
				case snmpDot3OutPauseFrames:
					return BCM_E_NONE;
				default:
					return BCM_E_PARAM;
			}
		case _BCM_EA_STAT_LLID0:
		case _BCM_EA_STAT_LLID1:
		case _BCM_EA_STAT_LLID2:
		case _BCM_EA_STAT_LLID3:
		case _BCM_EA_STAT_LLID4:
		case _BCM_EA_STAT_LLID5:
		case _BCM_EA_STAT_LLID6:
		case _BCM_EA_STAT_LLID7:
			switch (val){
				case snmpIfInOctets:
				case snmpEtherStatsRXNoErrors:
				case snmpIfInUcastPkts:
				case snmpIfInMulticastPkts:
				case snmpIfInBroadcastPkts:
				case snmpDot3StatsFCSErrors:
				case snmpEtherStatsUndersizePkts:
				case snmpEtherStatsPkts64Octets:
				case snmpEtherStatsPkts65to127Octets:
				case snmpEtherStatsPkts128to255Octets:
				case snmpEtherStatsPkts256to511Octets:
				case snmpEtherStatsPkts512to1023Octets:
				case snmpEtherStatsPkts1024to1518Octets:
				case snmpEtherRxOversizePkts:
				case snmpBcmPonInDroppedOctets:
				case snmpIfInDiscards:
				case snmpBcmPonInDelayedOctets:
				case snmpBcmPonInDelayedHundredUs:
				case snmpBcmPonInFrameErrors:
				case snmpBcmPonInOamFrames:
				case snmpDot3MpcpRxGate:
				case snmpDot3MpcpRxRegister:
				case snmpIfOutOctets:
				case snmpEtherStatsTXNoErrors:
				case snmpIfOutUcastPkts:
				case snmpIfOutMulticastPkts:
				case snmpIfOutBroadcastPkts:
				case snmpBcmTransmittedPkts64Octets:
				case snmpBcmTransmittedPkts65to127Octets:
				case snmpBcmTransmittedPkts128to255Octets:
				case snmpBcmTransmittedPkts256to511Octets:
				case snmpBcmTransmittedPkts512to1023Octets:
				case snmpBcmTransmittedPkts1024to1518Octets:
				case snmpBcmTransmittedPkts1519to2047Octets:
				case snmpBcmPonOutDroppedOctets:
				case snmpIfOutDiscards:
				case snmpBcmPonOutDelayedOctets:
				case snmpBcmPonOutDelayedHundredUs:
				case snmpBcmPonOutUnusedOctets:
			    case snmpBcmPonOutOamFrames:
				case snmpDot3MpcpTxRegAck:
				case snmpDot3MpcpTxRegRequest:
				case snmpDot3MpcpTxReport:
					return BCM_E_NONE;
				default:
					return BCM_E_PARAM;
			}
		default:
			return BCM_E_PORT;
	}
}


STATIC int
_bcm_ea_stat_sync_pon_val(int unit)
{
	int rv = BCM_E_NONE;
	int soc_rv;
	_soc_ea_port_counter_t pstats;

	soc_rv = _soc_ea_port_counter_get(unit, _BCM_EA_STAT_EPON, &pstats);
	rv = _bcm_ea_stat_check_rv_value(soc_rv);
	if (rv == BCM_E_NONE){
		stat_table[_BCM_EA_STAT_EPON][_BCM_EA_STAT_VAL_MacOctetsRxOkId].value
			= pstats.pon_counter.OamAttrMacOctetsRxOk;
		stat_table[_BCM_EA_STAT_EPON][_BCM_EA_STAT_VAL_MacFramesRxOk].value
			= pstats.pon_counter.OamAttrMacFramesRxOk;
		stat_table[_BCM_EA_STAT_EPON][_BCM_EA_STAT_VAL_MacRxUnicastFrames].value
			= pstats.pon_counter.OamAttrMacFramesRxOk;
		stat_table[_BCM_EA_STAT_EPON][_BCM_EA_STAT_VAL_MacMcastFramesRxOk].value
			= pstats.pon_counter.OamAttrMacMcastFramesRxOk;
		stat_table[_BCM_EA_STAT_EPON][_BCM_EA_STAT_VAL_MacBcastFramesRxOk].value
			= pstats.pon_counter.OamAttrMacBcastFramesRxOk;
		stat_table[_BCM_EA_STAT_EPON][_BCM_EA_STAT_VAL_MacFcsErr].value
			= pstats.pon_counter.OamAttrMacFcsErr;
		stat_table[_BCM_EA_STAT_EPON][_BCM_EA_STAT_VAL_OamEmulCrc8Err].value
			= pstats.pon_counter.OamAttrOamEmulCrc8Err;
		stat_table[_BCM_EA_STAT_EPON][_BCM_EA_STAT_VAL_PhySymbolErrDuringCarrier].value
			= pstats.pon_counter.OamAttrPhySymbolErrDuringCarrier;
		stat_table[_BCM_EA_STAT_EPON][_BCM_EA_STAT_VAL_AttrRxFrameTooShort].value
			= pstats.pon_counter.OamExtAttrRxFrameTooShort;
		stat_table[_BCM_EA_STAT_EPON][_BCM_EA_STAT_VAL_MacFrameTooLong].value = -1;
		stat_table[_BCM_EA_STAT_EPON][_BCM_EA_STAT_VAL_MacInRangeLenErr].value = -1;
		stat_table[_BCM_EA_STAT_EPON][_BCM_EA_STAT_VAL_MacAlignErr].value = -1;
		stat_table[_BCM_EA_STAT_EPON][_BCM_EA_STAT_VAL_RxFrame64].value
			= pstats.pon_counter.OamExtAttrRxFrame64;
		stat_table[_BCM_EA_STAT_EPON][_BCM_EA_STAT_VAL_RxFrame65_127].value
			= pstats.pon_counter.OamExtAttrRxFrame65_127;
		stat_table[_BCM_EA_STAT_EPON][_BCM_EA_STAT_VAL_RxFrame128_255].value
			= pstats.pon_counter.OamExtAttrRxFrame128_255;
		stat_table[_BCM_EA_STAT_EPON][_BCM_EA_STAT_VAL_RxFrame256_511].value
			= pstats.pon_counter.OamExtAttrRxFrame256_511;
		stat_table[_BCM_EA_STAT_EPON][_BCM_EA_STAT_VAL_RxFrame512_1023].value
			= pstats.pon_counter.OamExtAttrRxFrame512_1023;
		stat_table[_BCM_EA_STAT_EPON][_BCM_EA_STAT_VAL_RxFrame1024_1518].value
			= pstats.pon_counter.OamExtAttrRxFrame1024_1518;
		stat_table[_BCM_EA_STAT_EPON][_BCM_EA_STAT_VAL_RxFrame1519Plus].value
			= pstats.pon_counter.OamExtAttrRxFrame1519Plus;
		stat_table[_BCM_EA_STAT_EPON][_BCM_EA_STAT_VAL_RxBytesDropped].value
			= pstats.pon_counter.OamExtAttrRxBytesDropped;
		stat_table[_BCM_EA_STAT_EPON][_BCM_EA_STAT_VAL_RxFramesDropped].value
			= pstats.pon_counter.OamExtAttrRxFramesDropped;
		stat_table[_BCM_EA_STAT_EPON][_BCM_EA_STAT_VAL_RxBytesDelayed].value
			= pstats.pon_counter.OamExtAttrRxBytesDelayed;
		stat_table[_BCM_EA_STAT_EPON][_BCM_EA_STAT_VAL_RxDelay].value
			= pstats.pon_counter.OamExtAttrRxDelay;
		stat_table[_BCM_EA_STAT_EPON][_BCM_EA_STAT_VAL_MacCtrlPauseRx].value = -1;
		stat_table[_BCM_EA_STAT_EPON][_BCM_EA_STAT_VAL_OamLocalErrFrameSecsEvent].value
			= pstats.pon_counter.OamAttrOamLocalErrFrameSecsEvent;
		stat_table[_BCM_EA_STAT_EPON][_BCM_EA_STAT_VAL_MpcpMacCtrlFramesRx].value = -1;
		stat_table[_BCM_EA_STAT_EPON][_BCM_EA_STAT_VAL_MpcpRxGate].value = -1;
		stat_table[_BCM_EA_STAT_EPON][_BCM_EA_STAT_VAL_MpcpRxRegister].value = -1;
		stat_table[_BCM_EA_STAT_EPON][_BCM_EA_STAT_VAL_FecCorrectedBlocks].value
			= pstats.pon_counter.OamAttrFecCorrectedBlocks;
		stat_table[_BCM_EA_STAT_EPON][_BCM_EA_STAT_VAL_FecUncorrectableBlocks].value
			= pstats.pon_counter.OamAttrFecUncorrectableBlocks;
		stat_table[_BCM_EA_STAT_EPON][_BCM_EA_STAT_VAL_MacOctetsTxOk].value
			= pstats.pon_counter.OamAttrMacOctetsTxOk;
		stat_table[_BCM_EA_STAT_EPON][_BCM_EA_STAT_VAL_MacFramesTxOk].value
			= pstats.pon_counter.OamAttrMacFramesTxOk;
		stat_table[_BCM_EA_STAT_EPON][_BCM_EA_STAT_VAL_TxUnicastFrames].value
			= pstats.pon_counter.OamAttrMacFramesTxOk;
		stat_table[_BCM_EA_STAT_EPON][_BCM_EA_STAT_VAL_MacMcastFramesTxOk].value
			= pstats.pon_counter.OamAttrMacMcastFramesTxOk;
		stat_table[_BCM_EA_STAT_EPON][_BCM_EA_STAT_VAL_MacBcastFramesTxOk].value
			= pstats.pon_counter.OamAttrMacBcastFramesTxOk;
		stat_table[_BCM_EA_STAT_EPON][_BCM_EA_STAT_VAL_MacSingleCollFrames].value = -1;
		stat_table[_BCM_EA_STAT_EPON][_BCM_EA_STAT_VAL_MacMultipleCollFrames].value = -1;
		stat_table[_BCM_EA_STAT_EPON][_BCM_EA_STAT_VAL_MacLateCollisions].value = -1;
		stat_table[_BCM_EA_STAT_EPON][_BCM_EA_STAT_VAL_MacExcessiveCollisions].value = -1;
		stat_table[_BCM_EA_STAT_EPON][_BCM_EA_STAT_VAL_TxFrame64].value
			= pstats.pon_counter.OamExtAttrTxFrame64;
		stat_table[_BCM_EA_STAT_EPON][_BCM_EA_STAT_VAL_TxFrame65_127].value
			= pstats.pon_counter.OamExtAttrTxFrame65_127;
		stat_table[_BCM_EA_STAT_EPON][_BCM_EA_STAT_VAL_TxFrame128_255].value
			= pstats.pon_counter.OamExtAttrTxFrame128_255;
		stat_table[_BCM_EA_STAT_EPON][_BCM_EA_STAT_VAL_TxFrame256_511].value
			= pstats.pon_counter.OamExtAttrTxFrame256_511;
		stat_table[_BCM_EA_STAT_EPON][_BCM_EA_STAT_VAL_TxFrame512_1023].value
			= pstats.pon_counter.OamExtAttrTxFrame512_1023;
		stat_table[_BCM_EA_STAT_EPON][_BCM_EA_STAT_VAL_TxFrame1024_1518].value
			= pstats.pon_counter.OamExtAttrTxFrame1024_1518;
		stat_table[_BCM_EA_STAT_EPON][_BCM_EA_STAT_VAL_TxFrame1519Plus].value
			= pstats.pon_counter.OamExtAttrTxFrame1519Plus;
		stat_table[_BCM_EA_STAT_EPON][_BCM_EA_STAT_VAL_TxBytesDropped].value
			= pstats.pon_counter.OamExtAttrTxBytesDropped;
		stat_table[_BCM_EA_STAT_EPON][_BCM_EA_STAT_VAL_TxFramesDropped].value
			= pstats.pon_counter.OamExtAttrTxFramesDropped;
		stat_table[_BCM_EA_STAT_EPON][_BCM_EA_STAT_VAL_TxBytesDelayed].value
			= pstats.pon_counter.OamExtAttrTxBytesDelayed;
		stat_table[_BCM_EA_STAT_EPON][_BCM_EA_STAT_VAL_TxDelay].value
			= pstats.pon_counter.OamExtAttrTxDelay;
		stat_table[_BCM_EA_STAT_EPON][_BCM_EA_STAT_VAL_TxBytesUnused].value
			= pstats.pon_counter.OamExtAttrTxBytesUnused;
		stat_table[_BCM_EA_STAT_EPON][_BCM_EA_STAT_VAL_MacCtrlPauseTx].value = -1;
		stat_table[_BCM_EA_STAT_EPON][_BCM_EA_STAT_VAL_MpcpMacCtrlFramesTx].value = -1;
		stat_table[_BCM_EA_STAT_EPON][_BCM_EA_STAT_VAL_MpcpTxRegAck].value = -1;
		stat_table[_BCM_EA_STAT_EPON][_BCM_EA_STAT_VAL_MpcpTxRegRequest].value = -1;
		stat_table[_BCM_EA_STAT_EPON][_BCM_EA_STAT_VAL_MpcpTxReport].value = -1;
	}
	return rv;
}

STATIC int
_bcm_ea_stat_sync_uni_val(int unit, bcm_port_t port)
{
	int rv = BCM_E_NONE;
	_soc_ea_port_counter_t pstats;

	rv = _soc_ea_port_counter_get(unit, port, &pstats);
	rv = _bcm_ea_stat_check_rv_value(rv);
	if (BCM_E_NONE == rv){
		stat_table[port][_BCM_EA_STAT_VAL_MacOctetsRxOkId].value
			= pstats.uni_counter.OamAttrMacOctetsRxOk;
		stat_table[port][_BCM_EA_STAT_VAL_MacFramesRxOk].value
			= pstats.uni_counter.OamAttrMacFramesRxOk;
		stat_table[port][_BCM_EA_STAT_VAL_MacRxUnicastFrames].value
			= pstats.uni_counter.OamAttrMacFramesRxOk;
		stat_table[port][_BCM_EA_STAT_VAL_MacMcastFramesRxOk].value
			= pstats.uni_counter.OamAttrMacMcastFramesRxOk;
		stat_table[port][_BCM_EA_STAT_VAL_MacBcastFramesRxOk].value
			= pstats.uni_counter.OamAttrMacBcastFramesRxOk;
		stat_table[port][_BCM_EA_STAT_VAL_MacFcsErr].value
			= pstats.uni_counter.OamAttrMacFcsErr;
		stat_table[port][_BCM_EA_STAT_VAL_OamEmulCrc8Err].value = -1;
		stat_table[port][_BCM_EA_STAT_VAL_PhySymbolErrDuringCarrier].value = -1;
		stat_table[port][_BCM_EA_STAT_VAL_AttrRxFrameTooShort].value
			= pstats.uni_counter.OamExtAttrRxFrameTooShort;
		stat_table[port][_BCM_EA_STAT_VAL_MacFrameTooLong].value
			= pstats.uni_counter.OamAttrMacFrameTooLong;
		stat_table[port][_BCM_EA_STAT_VAL_MacInRangeLenErr].value
			= pstats.uni_counter.OamAttrMacInRangeLenErr;
		stat_table[port][_BCM_EA_STAT_VAL_MacAlignErr].value
			= pstats.uni_counter.OamAttrMacAlignErr;
		stat_table[port][_BCM_EA_STAT_VAL_RxFrame64].value
			= pstats.uni_counter.OamExtAttrRxFrame64;
		stat_table[port][_BCM_EA_STAT_VAL_RxFrame65_127].value
			= pstats.uni_counter.OamExtAttrRxFrame65_127;
		stat_table[port][_BCM_EA_STAT_VAL_RxFrame128_255].value
			= pstats.uni_counter.OamExtAttrRxFrame128_255;
		stat_table[port][_BCM_EA_STAT_VAL_RxFrame256_511].value
			= pstats.uni_counter.OamExtAttrRxFrame256_511;
		stat_table[port][_BCM_EA_STAT_VAL_RxFrame512_1023].value
			= pstats.uni_counter.OamExtAttrRxFrame512_1023;
		stat_table[port][_BCM_EA_STAT_VAL_RxFrame1024_1518].value
			= pstats.uni_counter.OamExtAttrRxFrame1024_1518;
		stat_table[port][_BCM_EA_STAT_VAL_RxFrame1519Plus].value
			= pstats.uni_counter.OamExtAttrRxFrame1519Plus;
		stat_table[port][_BCM_EA_STAT_VAL_RxBytesDropped].value
			= pstats.uni_counter.OamExtAttrRxBytesDropped;
		stat_table[port][_BCM_EA_STAT_VAL_RxFramesDropped].value = -1;
		stat_table[port][_BCM_EA_STAT_VAL_RxBytesDelayed].value = -1;
		stat_table[port][_BCM_EA_STAT_VAL_RxDelay].value = -1;
		stat_table[port][_BCM_EA_STAT_VAL_MacCtrlPauseRx].value
			= pstats.uni_counter.OamAttrMacCtrlPauseRx;
		stat_table[port][_BCM_EA_STAT_VAL_OamLocalErrFrameSecsEvent].value = -1;
		stat_table[port][_BCM_EA_STAT_VAL_MpcpMacCtrlFramesRx].value = -1;
		stat_table[port][_BCM_EA_STAT_VAL_MpcpRxGate].value = -1;
		stat_table[port][_BCM_EA_STAT_VAL_MpcpRxRegister].value = -1;
		stat_table[port][_BCM_EA_STAT_VAL_FecCorrectedBlocks].value = -1;
		stat_table[port][_BCM_EA_STAT_VAL_FecUncorrectableBlocks].value = -1;
		stat_table[port][_BCM_EA_STAT_VAL_MacOctetsTxOk].value
			= pstats.uni_counter.OamAttrMacOctetsTxOk;
		stat_table[port][_BCM_EA_STAT_VAL_MacFramesTxOk].value
			= pstats.uni_counter.OamAttrMacFramesTxOk;
		stat_table[port][_BCM_EA_STAT_VAL_TxUnicastFrames].value
			= pstats.uni_counter.OamAttrMacFramesTxOk;
		stat_table[port][_BCM_EA_STAT_VAL_MacMcastFramesTxOk].value
			= pstats.uni_counter.OamAttrMacMcastFramesTxOk;
		stat_table[port][_BCM_EA_STAT_VAL_MacBcastFramesTxOk].value
			= pstats.uni_counter.OamAttrMacBcastFramesTxOk;
		stat_table[port][_BCM_EA_STAT_VAL_MacSingleCollFrames].value
			= pstats.uni_counter.OamAttrMacSingleCollFrames;
		stat_table[port][_BCM_EA_STAT_VAL_MacMultipleCollFrames].value
			= pstats.uni_counter.OamAttrMacMultipleCollFrames;
		stat_table[port][_BCM_EA_STAT_VAL_MacLateCollisions].value
			= pstats.uni_counter.OamAttrMacLateCollisions;
		stat_table[port][_BCM_EA_STAT_VAL_MacExcessiveCollisions].value
			= pstats.uni_counter.OamAttrMacExcessiveCollisions;
		stat_table[port][_BCM_EA_STAT_VAL_TxFrame64].value
			= pstats.uni_counter.OamExtAttrTxFrame64;
		stat_table[port][_BCM_EA_STAT_VAL_TxFrame65_127].value
			= pstats.uni_counter.OamExtAttrTxFrame65_127;
		stat_table[port][_BCM_EA_STAT_VAL_TxFrame128_255].value
			= pstats.uni_counter.OamExtAttrTxFrame128_255;
		stat_table[port][_BCM_EA_STAT_VAL_TxFrame256_511].value
			= pstats.uni_counter.OamExtAttrTxFrame256_511;
		stat_table[port][_BCM_EA_STAT_VAL_TxFrame512_1023].value
			= pstats.uni_counter.OamExtAttrTxFrame256_511;
		stat_table[port][_BCM_EA_STAT_VAL_TxFrame1024_1518].value
			= pstats.uni_counter.OamExtAttrTxFrame1024_1518 ;
		stat_table[port][_BCM_EA_STAT_VAL_TxFrame1519Plus].value
			= pstats.uni_counter.OamExtAttrTxFrame1519Plus;
		stat_table[port][_BCM_EA_STAT_VAL_TxBytesDropped].value
			= pstats.uni_counter.OamExtAttrTxBytesDropped;
		stat_table[port][_BCM_EA_STAT_VAL_TxFramesDropped].value
			= pstats.uni_counter.OamExtAttrTxFramesDropped;
		stat_table[port][_BCM_EA_STAT_VAL_TxBytesDelayed].value = -1;
		stat_table[port][_BCM_EA_STAT_VAL_TxDelay].value = -1;
		stat_table[port][_BCM_EA_STAT_VAL_TxBytesUnused].value = -1;
		stat_table[port][_BCM_EA_STAT_VAL_MacCtrlPauseTx].value
			= pstats.uni_counter.OamAttrMacCtrlPauseTx;
		stat_table[port][_BCM_EA_STAT_VAL_MpcpMacCtrlFramesTx].value = -1;
		stat_table[port][_BCM_EA_STAT_VAL_MpcpTxRegAck].value = -1;
		stat_table[port][_BCM_EA_STAT_VAL_MpcpTxRegRequest].value = -1;
		stat_table[port][_BCM_EA_STAT_VAL_MpcpTxReport].value = -1;
	}
	return rv;
}

STATIC int
_bcm_ea_stat_sync_llid_val(int unit, bcm_port_t llid)
{
	int rv = BCM_E_NONE;
	int soc_rv;
	_soc_ea_port_counter_t pstats;

	soc_rv = _soc_ea_llid_counter_get(unit, (llid - _BCM_EA_STAT_UNI1), &pstats);
	rv = _bcm_ea_stat_check_rv_value(soc_rv);
	if (BCM_E_NONE != rv){
		/*
		 * case snmpIfInOctets: UNI/PON/LLID
		 * {snmpIfInOctets, 					_BCM_EA_S_MacOctetsRxOkId, 0},
		 */
		stat_table[llid][_BCM_EA_STAT_VAL_MacOctetsRxOkId].value
			= pstats.llid_counter.OamAttrMacOctetsRxOk;
		/*
		 *case snmpEtherStatsRXNoErrors:UNI/PON/LLID
		 *{snmpEtherStatsRXNoErrors, 			_BCM_EA_S_MacFramesRxOk, 0},
		 */
		stat_table[llid][_BCM_EA_STAT_VAL_MacFramesRxOk].value
			= pstats.llid_counter.OamAttrMacFramesRxOk;
		/*
		 * case snmpIfInUcastPkts:UNI/PON/LLID
		 *{snmpIfInUcastPkts, 				_BCM_EA_S_MacRxUnicastFrames, 0},
		 */
		stat_table[llid][_BCM_EA_STAT_VAL_MacRxUnicastFrames].value
			= pstats.llid_counter.OamAttrMacFramesRxOk;
		/*
		 * case snmpIfInMulticastPkts:UNI/PON/LLID
		 *{snmpIfInMulticastPkts, 			_BCM_EA_S_MacMcastFramesRxOk, 0},
		 */
		stat_table[llid][_BCM_EA_STAT_VAL_MacMcastFramesRxOk].value
			= pstats.llid_counter.OamAttrMacMcastFramesRxOk;
		/*
		 * case snmpIfInBroadcastPkts:UNI/PON/LLID
		 *{snmpIfInBroadcastPkts, 			_BCM_EA_S_MacBcastFramesRxOk, 0},
		 */
		stat_table[llid][_BCM_EA_STAT_VAL_MacBcastFramesRxOk].value
			= pstats.llid_counter.OamAttrMacBcastFramesRxOk;
		/*
		 * case snmpDot3StatsFCSErrors:UNI/PON/LLID
		 *{snmpDot3StatsFCSErrors, 			_BCM_EA_S_MacFcsErr, 0},
		 */
		stat_table[llid][_BCM_EA_STAT_VAL_MacFcsErr].value
			= pstats.llid_counter.OamAttrMacFcsErr;
		/*
		 * case snmpDot3OmpEmulationCRC8Errors: PON
		 *{snmpDot3OmpEmulationCRC8Errors, 	_BCM_EA_S_OamEmulCrc8Err, 0},
		 */
		stat_table[llid][_BCM_EA_STAT_VAL_OamEmulCrc8Err].value = -1;
		/*
		 * case snmpDot3StatsSymbolErrors: PON
		 *{snmpDot3StatsSymbolErrors, 		_BCM_EA_S_PhySymbolErrDuringCarrier, 0},
		 */
		stat_table[llid][_BCM_EA_STAT_VAL_PhySymbolErrDuringCarrier].value = -1;
		/*
		 * case snmpEtherStatsUndersizePkts: UNI/PON/LLID
		 *{snmpEtherStatsUndersizePkts, 		_BCM_EA_S_AttrRxFrameTooShort, 0},
		 */
		stat_table[llid][_BCM_EA_STAT_VAL_AttrRxFrameTooShort].value
			= pstats.llid_counter.OamExtAttrRxFrameTooShort;
		/*
		 *UNI
		 *{snmpDot3StatsFrameTooLongs, 		_BCM_EA_S_MacFrameTooLong, 0},
		 */
		stat_table[llid][_BCM_EA_STAT_VAL_MacFrameTooLong].value = -1;
		/*
		 * UNI
		 * {snmpDot3StatsInRangeLengthError, 	_BCM_EA_S_MacInRangeLenErr, 0},
		 */
		stat_table[llid][_BCM_EA_STAT_VAL_MacInRangeLenErr].value = -1;
		/*
		 * UNI
		 *{snmpDot3StatsAlignmentErrors, 		_BCM_EA_S_MacAlignErr, 0},
		 */
		stat_table[llid][_BCM_EA_STAT_VAL_MacAlignErr].value = -1;
		/*
		 * case snmpEtherStatsPkts64Octets: UNI/PON/LLID
		 *{snmpEtherStatsPkts64Octets, 		_BCM_EA_S_RxFrame64, 0},
		 */
		stat_table[llid][_BCM_EA_STAT_VAL_RxFrame64].value
			= pstats.llid_counter.OamExtAttrRxFrame64;
		/*
		 * case snmpEtherStatsPkts65to127Octets: UNI/PON/LLID
		 * {snmpEtherStatsPkts65to127Octets, 	_BCM_EA_S_RxFrame65_127, 0},
		 */
		stat_table[llid][_BCM_EA_STAT_VAL_RxFrame65_127].value
			= pstats.llid_counter.OamExtAttrRxFrame65_127;
		/*
		 * case snmpEtherStatsPkts128to255Octets:UNI/PON/LLID
		 * {snmpEtherStatsPkts128to255Octets, 	_BCM_EA_S_RxFrame128_255, 0},
		 */
		stat_table[llid][_BCM_EA_STAT_VAL_RxFrame128_255].value
			= pstats.llid_counter.OamExtAttrRxFrame128_255;
		/*
		 * case snmpEtherStatsPkts256to511Octets:UNI/PON/LLID
		 * {snmpEtherStatsPkts256to511Octets, 	_BCM_EA_S_RxFrame256_511, 0},
		 */
		stat_table[llid][_BCM_EA_STAT_VAL_RxFrame256_511].value
			= pstats.llid_counter.OamExtAttrRxFrame256_511;
		/*
		 * case snmpEtherStatsPkts512to1023Octets:UNI/PON/LLID
		 * {snmpEtherStatsPkts512to1023Octets, _BCM_EA_S_RxFrame512_1023, 0},
		 */
		stat_table[llid][_BCM_EA_STAT_VAL_RxFrame512_1023].value
			= pstats.llid_counter.OamExtAttrRxFrame512_1023;
		/*
		 * case snmpEtherStatsPkts1024to1518Octets:UNI/PON/LLID
		 * {snmpEtherStatsPkts1024to1518Octets,_BCM_EA_S_RxFrame1024_1518, 0},
		 */
		stat_table[llid][_BCM_EA_STAT_VAL_RxFrame1024_1518].value
			= pstats.llid_counter.OamExtAttrRxFrame1024_1518;
		/*
		 * case snmpEtherRxOversizePkts:UNI/PON/LLID
		 * {snmpEtherRxOversizePkts, 			_BCM_EA_S_RxFrame1519Plus, 0},
		 */
		stat_table[llid][_BCM_EA_STAT_VAL_RxFrame1519Plus].value
			= pstats.llid_counter.OamExtAttrRxFrame1519Plus;
		/*
		 * case snmpBcmInDroppedOctets: PON/LLID
		 *	{snmpBcmInDroppedOctets, 			_BCM_EA_S_RxBytesDropped, 0},
		 */
		stat_table[llid][_BCM_EA_STAT_VAL_RxBytesDropped].value
			= pstats.llid_counter.OamExtAttrRxBytesDropped;
		/*
		 * case snmpIfInDiscards: PON/LLID
		 * {snmpIfInDiscards, 					_BCM_EA_S_RxFramesDropped, 0},
		 */
		stat_table[llid][_BCM_EA_STAT_VAL_RxFramesDropped].value
			= pstats.llid_counter.OamExtAttrRxFramesDropped;
		/*
		 * case snmpBcmInDelayedOctets: PON/LLID
		 * {snmpBcmInDelayedOctets, 			_BCM_EA_S_RxBytesDelayed, 0},
		 */
		stat_table[llid][_BCM_EA_STAT_VAL_RxBytesDelayed].value
			= pstats.llid_counter.OamExtAttrRxBytesDelayed;
		/*
		 * case snmpBcmInDelayedHundredUs: PON/LLID
		 * {snmpBcmInDelayedHundredUs, 		_BCM_EA_S_RxDelay, 0},
		 */
		stat_table[llid][_BCM_EA_STAT_VAL_RxDelay].value
			= pstats.llid_counter.OamExtAttrRxDelay;
		/*
		 * UNI
		 * {snmpDot3InPauseFrames, 			_BCM_EA_S_MacCtrlPauseRx, 0},
		 */
		stat_table[llid][_BCM_EA_STAT_VAL_MacCtrlPauseRx].value = -1;
		/*
		 * case snmpBcmOamInFrameErrors: PON/LLID
		 *{snmpBcmOamInFrameErrors, 			_BCM_EA_S_OamLocalErrFrameSecsEvent, 0},
		 */
		stat_table[llid][_BCM_EA_STAT_VAL_OamLocalErrFrameSecsEvent].value
			= pstats.llid_counter.OamAttrOamLocalErrFrameSecsEvent;
		/*
		 * LLID
		 *{snmpDot3MpcpMACCtrlFramesReceived, _BCM_EA_S_MpcpMacCtrlFramesRx, 0},
		 */
		stat_table[llid][_BCM_EA_STAT_VAL_MpcpMacCtrlFramesRx].value
			= pstats.llid_counter.OamAttrMpcpMACCtrlFramesRx;
		/*
		 * LLID
		 *{snmpDot3MpcpRxGate, 				_BCM_EA_S_MpcpRxGate, 0},
		 */
		stat_table[llid][_BCM_EA_STAT_VAL_MpcpRxGate].value
			= pstats.llid_counter.OamAttrMpcpRxGate ;
		/*
		 * LLID
		 * {snmpDot3MpcpRxRegister, 			_BCM_EA_S_MpcpRxRegister, 0},
		 */
		stat_table[llid][_BCM_EA_STAT_VAL_MpcpRxRegister].value
			= pstats.llid_counter.OamAttrMpcpRxRegister;
		/*
		 * case snmpDot3EponFecCorrectedBlocks: //PON
		 * {snmpDot3EponFecCorrectedBlocks, 	_BCM_EA_S_FecCorrectedBlocks, 0},
		 */
		stat_table[llid][_BCM_EA_STAT_VAL_FecCorrectedBlocks].value = -1;
		/*
		 * case snmpDot3EponFecUncorrectableBlocks: PON
		 * {snmpDot3EponFecUncorrectableBlocks,_BCM_EA_S_FecUncorrectableBlocks, 0},
		 */
		stat_table[llid][_BCM_EA_STAT_VAL_FecUncorrectableBlocks].value = -1;
		/*
		 * case snmpIfOutOctets: UNI/PON/LLID/Queue
		 * {snmpIfOutOctets, 					_BCM_EA_S_MacOctetsTxOk, 0},
		 */
		stat_table[llid][_BCM_EA_STAT_VAL_MacOctetsTxOk].value
			= pstats.llid_counter.OamAttrMacOctetsTxOk;
		/*
		 * case snmpEtherStatsTXNoErrors: UNI/PON/LLID/Queue
		 *{snmpEtherStatsTXNoErrors, 			_BCM_EA_S_MacFramesTxOk, 0},
		 */
		stat_table[llid][_BCM_EA_STAT_VAL_MacFramesTxOk].value
			= pstats.llid_counter.OamAttrMacFramesTxOk;
		/*
		 * case snmpIfOutUcastPkts: UNI/PON/LLID
		 *{snmpIfOutUcastPkts, 				_BCM_EA_S_TxUnicastFrames, 0},
		 */
		stat_table[llid][_BCM_EA_STAT_VAL_TxUnicastFrames].value
			= pstats.llid_counter.OamExtAttrTxUnicastFrames;
		/*
		 * case snmpIfOutMulticastPkts:UNI/PON/LLID
		 *{snmpIfOutMulticastPkts, 			_BCM_EA_S_MacMcastFramesTxOk, 0},
		 */
		stat_table[llid][_BCM_EA_STAT_VAL_MacMcastFramesTxOk].value
			= pstats.llid_counter.OamAttrMacMcastFramesTxOk;
		/*
		 * case snmpIfOutBroadcastPkts:UNI/PON/LLID
		 *{snmpIfOutBroadcastPkts, 			_BCM_EA_S_MacBcastFramesTxOk, 0},
		 */
		stat_table[llid][_BCM_EA_STAT_VAL_MacBcastFramesTxOk].value
			= pstats.llid_counter.OamAttrMacBcastFramesTxOk;
		/*
		 * UNI
		 * {snmpDot3StatsSingleCollisionFrames,_BCM_EA_S_MacSingleCollFrames, 0},
		 */
		stat_table[llid][_BCM_EA_STAT_VAL_MacSingleCollFrames].value = -1;
		/*
		 * UNI
	     *{snmpDot3StatsMultipleCollisionFrames, _BCM_EA_S_MacMultipleCollFrames, 0},
		 */
		stat_table[llid][_BCM_EA_STAT_VAL_MacMultipleCollFrames].value = -1;
		/*
		 * UNI
		 *{snmpDot3StatsLateCollisions, 		_BCM_EA_S_MacLateCollisions, 0},
		 */
		stat_table[llid][_BCM_EA_STAT_VAL_MacLateCollisions].value = -1;
		/*
		 * UNI
		 *{snmpDot3StatsExcessiveCollisions, 	_BCM_EA_S_MacExcessiveCollisions, 0},
		 */
		stat_table[llid][_BCM_EA_STAT_VAL_MacExcessiveCollisions].value = -1;
		/*
		 * case snmpBcmTransmittedPkts64Octets:UNI/PON/LLID
		 * {snmpBcmTransmittedPkts64Octets,	_BCM_EA_S_TxFrame64, 0},
		 */
		stat_table[llid][_BCM_EA_STAT_VAL_TxFrame64].value
			= pstats.llid_counter.OamExtAttrTxFrame64;
		/*
		 * case snmpBcmTransmittedPkts65to127Octets:UNI/PON/LLID
		 * {snmpBcmTransmittedPkts65to127Octets, _BCM_EA_S_TxFrame65_127, 0},
		 */
		stat_table[llid][_BCM_EA_STAT_VAL_TxFrame65_127].value
			= pstats.llid_counter.OamExtAttrTxFrame65_127;
		/*
		 * case snmpBcmTransmittedPkts128to255Octets:UNI/PON/LLID
		 * {snmpBcmTransmittedPkts128to255Octets, _BCM_EA_S_TxFrame128_255, 0},
		 */
		stat_table[llid][_BCM_EA_STAT_VAL_TxFrame128_255].value
			= pstats.llid_counter.OamExtAttrTxFrame128_255;
		/*
		 * case snmpBcmTransmittedPkts256to511Octets:UNI/PON/LLID
		 *{snmpBcmTransmittedPkts256to511Octets, _BCM_EA_S_TxFrame256_511, 0},
		 */
		stat_table[llid][_BCM_EA_STAT_VAL_TxFrame256_511].value
			= pstats.llid_counter.OamExtAttrTxFrame256_511;
		/*
		 * case snmpBcmTransmittedPkts512to1023Octets:UNI/PON/LLID
		 *{snmpBcmTransmittedPkts512to1023Octets, _BCM_EA_S_TxFrame512_1023, 0},
		 */
		stat_table[llid][_BCM_EA_STAT_VAL_TxFrame512_1023].value
			= pstats.llid_counter.OamExtAttrTxFrame512_1023;
		/*
		 * case snmpBcmTransmittedPkts1024to1518Octets:UNI/PON/LLID
		 * {snmpBcmTransmittedPkts1024to1518Octets, _BCM_EA_S_TxFrame1024_1518, 0},
		 */
		stat_table[llid][_BCM_EA_STAT_VAL_TxFrame1024_1518].value
			= pstats.llid_counter.OamExtAttrTxFrame1024_1518;
		/*
		 * case snmpBcmTransmittedPkts1519to2047Octets:UNI/PON/LLID
		 * {snmpBcmTransmittedPkts1519to2047Octets, _BCM_EA_S_TxFrame1519Plus, 0},
		 */
		stat_table[llid][_BCM_EA_STAT_VAL_TxFrame1519Plus].value
			= pstats.llid_counter.OamExtAttrTxFrame1519Plus;
		/*
		 * case snmpBcmOutDroppedOctets:UNI/PON/LLID
		 * {snmpBcmOutDroppedOctets, 				_BCM_EA_S_TxBytesDropped, 0},
		 */
		stat_table[llid][_BCM_EA_STAT_VAL_TxBytesDropped].value
			= pstats.llid_counter.OamExtAttrTxBytesDropped;
		/*
		 * case snmpIfOutDiscards:UNI/PON/LLID
		 *{snmpIfOutDiscards, 					_BCM_EA_S_TxFramesDropped, 0},
		 */
		stat_table[llid][_BCM_EA_STAT_VAL_TxFramesDropped].value
			= pstats.llid_counter.OamExtAttrTxFramesDropped;
		/*
		 * case snmpBcmOutDelayedOctets: PON/LLID
		 *	{snmpBcmOutDelayedOctets, _BCM_EA_S_TxBytesDelayed, 0},
		 */
		stat_table[llid][_BCM_EA_STAT_VAL_TxBytesDelayed].value
			= pstats.llid_counter.OamExtAttrTxBytesDelayed;
		/*
		 * case snmpBcmOutDelayedHundredUs: PON/LLID
		 *	{snmpBcmOutDelayedHundredUs, _BCM_EA_S_TxDelay, 0},
		 */
		stat_table[llid][_BCM_EA_STAT_VAL_TxDelay].value
			= pstats.llid_counter.OamExtAttrRxDelay;
		/*
		 * case snmpBcmOutUnusedOctets: PON/LLID
		 *	{snmpBcmOutUnusedOctets, _BCM_EA_S_TxBytesUnused, 0},
		 */
		stat_table[llid][_BCM_EA_STAT_VAL_TxBytesUnused].value
			= pstats.llid_counter.OamExtAttrTxBytesUnused;
		/*
		 * UNI
		 *{snmpDot3OutPauseFrames, _BCM_EA_S_MacCtrlPauseTx, 0},
		 */
		stat_table[llid][_BCM_EA_STAT_VAL_MacCtrlPauseTx].value = -1;
		/*
		 * LLID
		 *	{snmpDot3MpcpMACCtrlFramesTransmitted, _BCM_EA_S_MpcpMacCtrlFramesTx, 0},
		 */
		stat_table[llid][_BCM_EA_STAT_VAL_MpcpMacCtrlFramesTx].value
			= pstats.llid_counter.OamAttrMpcpMACCtrlFramesTx;
		/*
		 * LLID
		 *{snmpDot3MpcpTxRegAck, _BCM_EA_S_MpcpTxRegAck, 0},
		 */
		stat_table[llid][_BCM_EA_STAT_VAL_MpcpTxRegAck].value
			= pstats.llid_counter.OamAttrMpcpTxRegAck;
		/*
		 * LLID
		 *{snmpDot3MpcpTxRegRequest, _BCM_EA_S_MpcpTxRegRequest, 0},
		 */
		stat_table[llid][_BCM_EA_STAT_VAL_MpcpTxRegRequest].value
			= pstats.llid_counter.OamAttrMpcpTxRegRequest;
		/*
		 * LLID
		 *{snmpDot3MpcpTxReport, _BCM_EA_S_MpcpTxRegRequest, 0},
		 */
		stat_table[llid][_BCM_EA_STAT_VAL_MpcpTxReport].value
			=pstats.llid_counter.OamAttrMpcpTxReport;
	}
	return rv;
}

/* Clear the port-based statistics for the indicated device port. */
int
_bcm_ea_stat_clear(
	    int unit,
	    bcm_port_t port)
{
	int rv = BCM_E_NONE;
	int i = 0;

	if (port < _BCM_TK371X_PON_PORT_BASE ||
			(port > _BCM_TK371X_LLID_PORT_BASE + _BCM_TK371X_MAX_LLID_PORT_NUM)){
		return BCM_E_PORT;
	}
	rv = _soc_ea_clr_stats_set((uint8)unit, 0);
	if (rv == OK){
		rv = BCM_E_NONE;
	}else{
		rv = BCM_E_FAIL;
	}
	for (i = 0; i < _BCM_EA_STAT_PORT_MAX; i++){
		_bcm_ea_stat_zero(i);
	}
	return rv;
}

/* Get the specified statistics from the device. */
int
_bcm_ea_stat_get(
	    int unit,
	    bcm_port_t port,
	    bcm_stat_val_t type,
	    uint64 *value)
{
	int rv = BCM_E_NONE;
	int i = 0;
	int pindex = (int)port;

	rv = _bcm_ea_stat_check_port_val(port, type);
	if (BCM_E_NONE == rv){
		while (-1 != stat_table[pindex][i].stat_id){
			if (type == stat_table[pindex][i].type){
				*value = stat_table[pindex][i].value;
				return BCM_E_NONE;
			}
			i++;
		}
	}
	return rv;
}


/* Get the specified statistics from the device. */
int
_bcm_ea_stat_get32(
	    int unit,
	    bcm_port_t port,
	    bcm_stat_val_t type,
	    uint32 *value)
{
	uint64 tmpVal;
	int rv = BCM_E_NONE;

    COMPILER_64_ZERO(tmpVal);
	
	rv = _bcm_ea_stat_get(unit, port, type, &tmpVal);
	*value = (uint32)tmpVal;

	return rv;
}

/* Synchronize software counters with hardware. */
int
_bcm_ea_stat_sync(int unit)
{
	int i = 0;
	int rv;

	rv = _soc_ea_counter_sync(unit);
	if (rv != SOC_E_NONE){
		return rv;
	}
	/*sync EPON port statistics*/
	_bcm_ea_stat_sync_pon_val(unit);
	/*sync UNI ports statistics*/
	for (i = _BCM_EA_STAT_UNI_MIN; i <= _BCM_EA_STAT_UNI_MAX; i++){
		_bcm_ea_stat_sync_uni_val(unit, i);
	}
	/*sync LLID statistics*/
	for (i = _BCM_EA_STAT_LLID_MIN; i <= _BCM_EA_STAT_LLID_MIN; i++){
		_bcm_ea_stat_sync_llid_val(unit, i);
	}
	return BCM_E_NONE;
}


void
_bcm_ea_stat_init(void)
{
	int i = 0;
	int j = 0;

	for (i = 0; i < _BCM_EA_STAT_PORT_MAX; i++){
		j = 0;
		while (init_val[j].stat_id != -1){
			stat_table[i][j].stat_id = init_val[j].stat_id;
			stat_table[i][j].type = init_val[j].type;
			stat_table[i][j].value = init_val[j].value;
			j++;
		}
		stat_table[i][j].stat_id = -1;
		stat_table[i][j].type = -1;
		stat_table[i][j].value = 0;
	}
	for (i = 0; i < _BCM_EA_STAT_PORT_MAX; i++){
		_bcm_ea_stat_zero(i);
	}
}
