/*
 * $Id: counter.h,v 1.5 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:     counter.h
 * Purpose:
 *
 */

#ifndef _SOC_EA_STAT_H
#define _SOC_EA_STAT_H

#include <soc/ea/tk371x/Oam.h>         
#include <soc/ea/tk371x/TkStatsApi.h>

typedef enum _soc_ea_stats_id_e{
	socEaStatMacOctetsRxOkId = OamAttrMacOctetsRxOkIdStatsId,
	socEaStatMacFramesRxOk = OamAttrMacFramesRxOkStatsId,
	socEaStatMacRxUnicastFrames = OamExtAttrRxUnicastFramesStatsId,
	socEaStatMacMcastFramesRxOk = OamAttrMacMcastFramesRxOkStatsId,
	socEaStatMacBcastFramesRxOk = OamAttrMacBcastFramesRxOkStatsId,
	socEaStatMacFcsErr = OamAttrMacFcsErrStatsId,
	socEaStatOamEmulCrc8Err = OamAttrOamEmulCrc8ErrStatsId,
	socEaStatPhySymbolErrDuringCarrier = OamAttrPhySymbolErrDuringCarrierStatsId,
	socEaStatAttrRxFrameTooShort = OamExtAttrRxFrameTooShortStatsId,
	socEaStatMacFrameTooLong = OamAttrMacFrameTooLongStatsId,
	socEaStatMacInRangeLenErr = OamAttrMacInRangeLenErrStatsId,
	socEaStatMacOutOfRangeLenErr = OamAttrMacOutOfRangeLenErrStatsId,
	socEaStatMacAlignErr = OamAttrMacAlignErrStatsId,
	socEaStatRxFrame64 = OamExtAttrRxFrame64StatsId,
	socEaStatRxFrame65_127 = OamExtAttrRxFrame65_127StatsId,
	socEaStatRxFrame128_255 = OamExtAttrRxFrame128_255StatsId,
	socEaStatRxFrame256_511 = OamExtAttrRxFrame256_511StatsId,
	socEaStatRxFrame512_1023 = OamExtAttrRxFrame512_1023StatsId,
	socEaStatRxFrame1024_1518 = OamExtAttrRxFrame1024_1518StatsId,
	socEaStatRxFrame1519Plus = OamExtAttrRxFrame1519PlusStatsId,
	socEaStatRxFramesDropped = OamExtAttrRxFramesDroppedStatsId,
	socEaStatRxBytesDropped = OamExtAttrRxBytesDroppedStatsId,
	socEaStatRxBytesDelayed = OamExtAttrRxBytesDelayedStatsId,
	socEaStatRxDelay = OamExtAttrRxDelayStatsId,
	socEaStatReserved0 = OamAttrReserved0StatsId,
	socEaStatMacCtrlPauseRx = OamAttrMacCtrlPauseRxStatsId,
	socEaStatOamLocalErrFrameSecsEvent = OamAttrOamLocalErrFrameSecsEventStatsId,
	socEaStatReserved1 = OamAttrReserved1StatsId,
	socEaStatReserved2 = OamAttrReserved2StatsId,
	socEaStatMacOctetsTxOk = OamAttrMacOctetsTxOkStatsId,
	socEaStatMacFramesTxOk = OamAttrMacFramesTxOkStatsId,
	socEaStatTxUnicastFrames = OamExtAttrTxUnicastFramesStatsId,
	socEaStatMacMcastFramesTxOk = OamAttrMacMcastFramesTxOkStatsId,
	socEaStatMacBcastFramesTxOk = OamAttrMacBcastFramesTxOkStatsId,
	socEaStatMacSingleCollFrames = OamAttrMacSingleCollFramesStatsId,
	socEaStatMacMultipleCollFrames = OamAttrMacMultipleCollFramesStatsId,
	socEaStatMacLateCollisions = OamAttrMacLateCollisionsStatsId,
	socEaStatMacExcessiveCollisions = OamAttrMacExcessiveCollisionsStatsId,
    socEaStatTxFrame64 = OamExtAttrTxFrame64StatsId,
    socEaStatTxFrame65_127 = OamExtAttrTxFrame65_127StatsId,
    socEaStatTxFrame128_255 = OamExtAttrTxFrame128_255StatsId,
    socEaStatTxFrame256_511 = OamExtAttrTxFrame256_511StatsId,
    socEaStatTxFrame512_1023 = OamExtAttrTxFrame512_1023StatsId,
    socEaStatTxFrame1024_1518 = OamExtAttrTxFrame1024_1518StatsId,
    socEaStatTxFrame1519Plus = OamExtAttrTxFrame1519PlusStatsId,
    socEaStatTxFramesDropped = OamExtAttrTxFramesDroppedStatsId,
    socEaStatTxBytesDropped = OamExtAttrTxBytesDroppedStatsId,
    socEaStatTxBytesDelayed = OamExtAttrTxBytesDelayedStatsId,
    socEaStatTxDelay = OamExtAttrTxDelayStatsId,
    socEaStatTxBytesUnused = OamExtAttrTxBytesUnusedStatsId,
    socEaStatReserved3 = OamAttrReserved3StatsId,
    socEaStatMacCtrlPauseTx = OamAttrMacCtrlPauseTxStatsId,
    socEaStatMpcpMACCtrlFramesTx = OamAttrMpcpMACCtrlFramesTxStatsId,
    socEaStatMpcpMACCtrlFramesRx = OamAttrMpcpMACCtrlFramesRxStatsId,
    socEaStatMpcpTxRegAck = OamAttrMpcpTxRegAckStatsId,
    socEaStatMpcpTxRegRequest = OamAttrMpcpTxRegRequestStatsId,
    socEaStatMpcpTxReport = OamAttrMpcpTxReportStatsId,
    socEaStatMpcpRxGate = OamAttrMpcpRxGateStatsId,
    socEaStatMpcpRxRegister = OamAttrMpcpRxRegisterStatsId,
    socEaStatFecCorrectedBlocks = OamAttrFecCorrectedBlocks,
    socEaStatFecUncorrectableBlocks = OamAttrFecUncorrectableBlocks
}_soc_ea_stats_id_t;

typedef struct {
    uint64 *pon_counter;
    uint64 *ge_counter;
    uint64 *fe_counter;
    uint64 *llid_counter;
    uint64 *cosq_counter;
} soc_ea_counter_t;

typedef TkExtAllStats 			_soc_ea_ext_all_stats_t;
typedef TkMacAllPortStats1		_soc_ea_mac_all_port_stats_t;

typedef TkPonStatsGroup         _soc_ea_pon_counter_group_t;
typedef TkUniStatsGroup         _soc_ea_uni_counter_group_t;
typedef TkLlidStatsGroup        _soc_ea_llid_counter_group_t;
typedef TkCosqStatsGroup        _soc_ea_cosq_counter_group_t;

typedef union {
    _soc_ea_pon_counter_group_t pon_counter;
    _soc_ea_uni_counter_group_t uni_counter;
    _soc_ea_llid_counter_group_t llid_counter;
} _soc_ea_port_counter_t;

#define _soc_ea_epon_stats_get		TkExtOamGetEponStats
#define _soc_ea_uni_stats_get		TkExtOamGetUniStats
#define _soc_ea_link_stats_get		TkExtOamGetLinkStats
#define _soc_ea_queue_stats_get		TkExtOamGetQueueStats
#define _soc_ea_clr_stats_set		TkExtOamSetClrStats
#define _soc_ea_uni_all_stats_get	TkExtOamGetUniAllStats
#define _soc_ea_port_stats_get		TkExtOamGetPortStats
#define _soc_ea_pon_port_stats_get	TkExtOamGetPONPortStats
#define _soc_ea_uni_port_stats_get	TkExtOamGetUNIPortStats

#define _soc_ea_cosq_counter_get(unit, port, link, queue, counter) \
    TkExtOamGetCosqStatsGroup(unit,0,port, link, queue, counter)
    
/*
  * Function:
  *      soc_ea_counter_attach
  * Purpose:
  *      Initialize counter module.
  * Notes:
  *      Allocates counter collection buffers.
  */
 int
 _soc_ea_counter_attach(int unit);

 /*
  * Function:
  *      soc_ea_counter_detach
  * Purpose:
  *      Finalize counter module.
  * Notes:
  *      Stops counter task if running.
  *      Deallocates counter collection buffers.
  */
 int
 _soc_ea_counter_detach(int unit);
 
 /*
 * Function:
 *  soc_ea_counter_sync
 * Purpose:
 *  Force an immediate counter update
 * Parameters:
 *  unit - uint number.
 * Returns:
 *  SOC_E_XXX
 * Notes:
 *  Ensures that ALL counter activity that occurred before the sync
 *  is reflected in the results of any soc_ea_counter_get()-type
 *  routine that is called after the sync.
 */

int
_soc_ea_counter_sync(int unit);

 /*
 * Function:
 *  _soc_ea_port_counter_get
 * Purpose:
 *  Get the port counter
 * Parameters:
 *  unit - uint number.
 *  port - pon, ge,fe
 *  count - union data structure pointer
 * Returns:
 *  SOC_E_XXX
 * Notes:
 */
int 
_soc_ea_port_counter_get(int unit, int port, 
    _soc_ea_port_counter_t *count);

 /*
 * Function:
 *  _soc_ea_llid_counter_get
 * Purpose:
 *  Get the llid counter
 * Parameters:
 *  unit - uint number.
 *  llid - llid index
 *  count - union data structure pointer
 * Returns:
 *  SOC_E_XXX
 * Notes:
 */
int 
_soc_ea_llid_counter_get(int unit, int llid_id, 
    _soc_ea_port_counter_t *count);
 
#endif /* _SOC_EA_HAL_STATS_H */
