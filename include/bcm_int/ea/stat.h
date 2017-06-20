/*
 * $Id: stat.h,v 1.6 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:     stat.h
 * Purpose:
 *
 */

#ifndef _BCM_INT_EA_STAT_H
#define _BCM_INT_EA_STAT_H
#include <bcm/types.h>
#include <bcm/stat.h>
#include <soc/ea/tk371x/counter.h>

typedef enum _bcm_ea_stat_id_e{
	_BCM_EA_S_MacOctetsRxOkId = socEaStatMacOctetsRxOkId,
	_BCM_EA_S_MacFramesRxOk = socEaStatMacFramesRxOk,
	_BCM_EA_S_MacRxUnicastFrames = socEaStatMacRxUnicastFrames,
	_BCM_EA_S_MacMcastFramesRxOk = socEaStatMacMcastFramesRxOk,
	_BCM_EA_S_MacBcastFramesRxOk = socEaStatMacBcastFramesRxOk,
	_BCM_EA_S_MacFcsErr = socEaStatMacFcsErr,
	_BCM_EA_S_OamEmulCrc8Err = socEaStatOamEmulCrc8Err,
	_BCM_EA_S_PhySymbolErrDuringCarrier = socEaStatPhySymbolErrDuringCarrier,
	_BCM_EA_S_AttrRxFrameTooShort = socEaStatAttrRxFrameTooShort,
	_BCM_EA_S_MacFrameTooLong = socEaStatMacFrameTooLong,
	_BCM_EA_S_MacInRangeLenErr = socEaStatMacInRangeLenErr,
	_BCM_EA_S_MacOutOfRangeLenErr = socEaStatMacOutOfRangeLenErr,
	_BCM_EA_S_MacAlignErr = socEaStatMacAlignErr,
	_BCM_EA_S_RxFrame64 = socEaStatRxFrame64,
	_BCM_EA_S_RxFrame65_127 = socEaStatRxFrame65_127,
	_BCM_EA_S_RxFrame128_255 = socEaStatRxFrame128_255,
	_BCM_EA_S_RxFrame256_511 = socEaStatRxFrame256_511,
	_BCM_EA_S_RxFrame512_1023 = socEaStatRxFrame512_1023,
	_BCM_EA_S_RxFrame1024_1518 = socEaStatRxFrame1024_1518,
	_BCM_EA_S_RxFrame1519Plus = socEaStatRxFrame1519Plus,
	_BCM_EA_S_RxFramesDropped = socEaStatRxFramesDropped,
	_BCM_EA_S_RxBytesDropped = socEaStatRxBytesDropped,
	_BCM_EA_S_RxBytesDelayed = socEaStatRxBytesDelayed,
	_BCM_EA_S_RxDelay = socEaStatRxDelay,
	_BCM_EA_S_Reserved0 = socEaStatReserved0,
	_BCM_EA_S_MacCtrlPauseRx = socEaStatMacCtrlPauseRx,
	_BCM_EA_S_OamLocalErrFrameSecsEvent = socEaStatOamLocalErrFrameSecsEvent,
	_BCM_EA_S_Reserved1 = socEaStatReserved1,
	_BCM_EA_S_Reserved2 = socEaStatReserved2,
	_BCM_EA_S_MacOctetsTxOk = socEaStatMacOctetsTxOk,
	_BCM_EA_S_MacFramesTxOk = socEaStatMacFramesTxOk,
	_BCM_EA_S_TxUnicastFrames = socEaStatTxUnicastFrames,
	_BCM_EA_S_MacMcastFramesTxOk = socEaStatMacMcastFramesTxOk,
	_BCM_EA_S_MacBcastFramesTxOk = socEaStatMacBcastFramesTxOk,
	_BCM_EA_S_MacSingleCollFrames = socEaStatMacSingleCollFrames,
	_BCM_EA_S_MacMultipleCollFrames = socEaStatMacMultipleCollFrames,
	_BCM_EA_S_MacLateCollisions = socEaStatMacLateCollisions,
	_BCM_EA_S_MacExcessiveCollisions = socEaStatMacExcessiveCollisions,
	_BCM_EA_S_TxFrame64 = socEaStatTxFrame64,
	_BCM_EA_S_TxFrame65_127 = socEaStatTxFrame65_127,
	_BCM_EA_S_TxFrame128_255 = socEaStatTxFrame128_255,
	_BCM_EA_S_TxFrame256_511 = socEaStatTxFrame256_511,
	_BCM_EA_S_TxFrame512_1023 = socEaStatTxFrame512_1023,
	_BCM_EA_S_TxFrame1024_1518 = socEaStatTxFrame1024_1518,
	_BCM_EA_S_TxFrame1519Plus = socEaStatTxFrame1519Plus,
	_BCM_EA_S_TxFramesDropped = socEaStatTxFramesDropped,
	_BCM_EA_S_TxBytesDropped = socEaStatTxBytesDropped,
	_BCM_EA_S_TxBytesDelayed = socEaStatTxBytesDelayed,
	_BCM_EA_S_TxDelay = socEaStatTxDelay,
	_BCM_EA_S_TxBytesUnused = socEaStatTxBytesUnused,
	_BCM_EA_S_Reserved3 = socEaStatReserved3,
	_BCM_EA_S_MacCtrlPauseTx = socEaStatMacCtrlPauseTx,
	_BCM_EA_S_MpcpMacCtrlFramesTx = socEaStatMpcpMACCtrlFramesTx,
	_BCM_EA_S_MpcpMacCtrlFramesRx = socEaStatMpcpMACCtrlFramesRx,
	_BCM_EA_S_MpcpTxRegAck = socEaStatMpcpTxRegAck,
	_BCM_EA_S_MpcpTxRegRequest = socEaStatMpcpTxRegRequest,
	_BCM_EA_S_MpcpTxReport = socEaStatMpcpTxReport,
	_BCM_EA_S_MpcpRxGate = socEaStatMpcpRxGate,
	_BCM_EA_S_MpcpRxRegister = socEaStatMpcpRxRegister,
	_BCM_EA_S_FecCorrectedBlocks = socEaStatFecCorrectedBlocks,
	_BCM_EA_S_FecUncorrectableBlocks = socEaStatFecUncorrectableBlocks,
	_BCM_EA_S_MAX = 128,
}_bcm_ea_stat_id_t;


typedef enum _bcm_ea_stat_port_type_e{
	_BCM_EA_STAT_EPON = 0,
	_BCM_EA_STAT_UNI_MIN,
	_BCM_EA_STAT_UNI0 = _BCM_EA_STAT_UNI_MIN,
	_BCM_EA_STAT_UNI1,
	_BCM_EA_STAT_UNI_MAX = _BCM_EA_STAT_UNI1,
	_BCM_EA_STAT_LLID_MIN = 3,
	_BCM_EA_STAT_LLID0 = _BCM_EA_STAT_LLID_MIN,
	_BCM_EA_STAT_LLID1,
	_BCM_EA_STAT_LLID2,
	_BCM_EA_STAT_LLID3,
	_BCM_EA_STAT_LLID4,
	_BCM_EA_STAT_LLID5,
	_BCM_EA_STAT_LLID6,
	_BCM_EA_STAT_LLID7,
	_BCM_EA_STAT_LLID_MAX = _BCM_EA_STAT_LLID7,
	_BCM_EA_STAT_PORT_MAX
}_bcm_ea_stat_port_type_t;

typedef enum _bcm_ea_stat_val_e{
	_BCM_EA_STAT_VAL_MacOctetsRxOkId = 0,
	_BCM_EA_STAT_VAL_MacFramesRxOk,
	_BCM_EA_STAT_VAL_MacRxUnicastFrames,
	_BCM_EA_STAT_VAL_MacMcastFramesRxOk,
	_BCM_EA_STAT_VAL_MacBcastFramesRxOk,
	_BCM_EA_STAT_VAL_MacFcsErr,
	_BCM_EA_STAT_VAL_OamEmulCrc8Err,
	_BCM_EA_STAT_VAL_PhySymbolErrDuringCarrier,
	_BCM_EA_STAT_VAL_AttrRxFrameTooShort,
	_BCM_EA_STAT_VAL_MacFrameTooLong,
	_BCM_EA_STAT_VAL_MacInRangeLenErr,
	_BCM_EA_STAT_VAL_MacAlignErr,
	_BCM_EA_STAT_VAL_RxFrame64,
	_BCM_EA_STAT_VAL_RxFrame65_127,
	_BCM_EA_STAT_VAL_RxFrame128_255,
	_BCM_EA_STAT_VAL_RxFrame256_511,
	_BCM_EA_STAT_VAL_RxFrame512_1023,
	_BCM_EA_STAT_VAL_RxFrame1024_1518,
	_BCM_EA_STAT_VAL_RxFrame1519Plus,
	_BCM_EA_STAT_VAL_RxBytesDropped,
	_BCM_EA_STAT_VAL_RxFramesDropped,
	_BCM_EA_STAT_VAL_RxBytesDelayed,
	_BCM_EA_STAT_VAL_RxDelay,
	_BCM_EA_STAT_VAL_MacCtrlPauseRx,
	_BCM_EA_STAT_VAL_OamLocalErrFrameSecsEvent,
	_BCM_EA_STAT_VAL_MpcpMacCtrlFramesRx,
	_BCM_EA_STAT_VAL_MpcpRxGate,
	_BCM_EA_STAT_VAL_MpcpRxRegister,
	_BCM_EA_STAT_VAL_FecCorrectedBlocks,
	_BCM_EA_STAT_VAL_FecUncorrectableBlocks,
	_BCM_EA_STAT_VAL_MacOctetsTxOk,
	_BCM_EA_STAT_VAL_MacFramesTxOk,
	_BCM_EA_STAT_VAL_TxUnicastFrames,
	_BCM_EA_STAT_VAL_MacMcastFramesTxOk,
	_BCM_EA_STAT_VAL_MacBcastFramesTxOk,
	_BCM_EA_STAT_VAL_MacSingleCollFrames,
	_BCM_EA_STAT_VAL_MacMultipleCollFrames,
	_BCM_EA_STAT_VAL_MacLateCollisions,
	_BCM_EA_STAT_VAL_MacExcessiveCollisions,
	_BCM_EA_STAT_VAL_TxFrame64,
	_BCM_EA_STAT_VAL_TxFrame65_127,
	_BCM_EA_STAT_VAL_TxFrame128_255,
	_BCM_EA_STAT_VAL_TxFrame256_511,
	_BCM_EA_STAT_VAL_TxFrame512_1023,
	_BCM_EA_STAT_VAL_TxFrame1024_1518,
	_BCM_EA_STAT_VAL_TxFrame1519Plus,
	_BCM_EA_STAT_VAL_TxBytesDropped,
	_BCM_EA_STAT_VAL_TxFramesDropped,
	_BCM_EA_STAT_VAL_TxBytesDelayed,
	_BCM_EA_STAT_VAL_TxDelay,
	_BCM_EA_STAT_VAL_TxBytesUnused,
	_BCM_EA_STAT_VAL_MacCtrlPauseTx,
	_BCM_EA_STAT_VAL_MpcpMacCtrlFramesTx,
	_BCM_EA_STAT_VAL_MpcpTxRegAck,
	_BCM_EA_STAT_VAL_MpcpTxRegRequest,
	_BCM_EA_STAT_VAL_MpcpTxReport,
	_BCM_EA_STAT_VAL_MAX
}_bcm_ea_stat_val_t;
typedef struct _bcm_ea_stat_table_entry_s{
	bcm_stat_val_t type;
	_bcm_ea_stat_id_t stat_id;
	uint64 value;
}_bcm_ea_stat_table_entry_t;

extern int _bcm_ea_stat_clear(int unit,bcm_port_t port);
extern int _bcm_ea_stat_get( int unit,bcm_port_t port, bcm_stat_val_t type, uint64 *value);
extern int _bcm_ea_stat_get32(int unit,bcm_port_t port, bcm_stat_val_t type, uint32 *value);
extern void _bcm_ea_stat_init(void);
extern int _bcm_ea_stat_sync(int unit);

#endif /* _BCM_INT_EA_STAT_H */


