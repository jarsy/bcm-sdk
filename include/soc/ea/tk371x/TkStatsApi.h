/*
 * $Id: TkStatsApi.h,v 1.6 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:     TkStatsApi.h
 * Purpose: 
 *
 */

#ifndef _SOC_EA_TkStatsApi_H
#define _SOC_EA_TkStatsApi_H

#if defined(__cplusplus)
extern "C"  {
#endif

#include <soc/ea/tk371x/TkTypes.h>


enum TkStatsId {
    /*  Rx stats */
    OamAttrMacOctetsRxOkIdStatsId           = 0,
    /*  0 - Number of received good bytesStatsId = 0, */
    OamAttrMacFramesRxOkStatsId             = 1,
    /*  1 - Number of received framesStatsId = 0, */
    OamExtAttrRxUnicastFramesStatsId        = 2,
    /*  2 - Number of received unicast framesStatsId = 0, */
    OamAttrMacMcastFramesRxOkStatsId        = 3,
    /*  3 - Number of received multicast framesStatsId = 0, */
    OamAttrMacBcastFramesRxOkStatsId        = 4,
    /*  4 - Number of received broadcast framesStatsId = 0, */

    OamAttrMacFcsErrStatsId                 = 5,
    /*  5 - Number of received FCS error framesStatsId = 0, */
    OamAttrOamEmulCrc8ErrStatsId            = 6,
    /*  6 - Number of received Crc8 error framesStatsId = 0, */
    OamAttrPhySymbolErrDuringCarrierStatsId = 7,
    /*  7 - Number of received LineCode error framesStatsId = 0, */

    OamExtAttrRxFrameTooShortStatsId        = 8,
    /*  8 - Number of received TooShort framesStatsId = 0, */
    OamAttrMacFrameTooLongStatsId           = 9,
    /*  9 - Number of received TooLong framesStatsId = 0, */
    OamAttrMacInRangeLenErrStatsId          = 10,
    /*  10 - Number of received InRangeLen Error framesStatsId = 0, */

    OamAttrMacOutOfRangeLenErrStatsId       = 11,
    /*  11 - Number of received OutRangeLen Error framesStatsId = 0, */
    OamAttrMacAlignErrStatsId               = 12,
    /*  12 - Number of received Align Error framesStatsId = 0, */

    /*  bin sizes available on Ethernet ports only */
    OamExtAttrRxFrame64StatsId              = 13,
    /*  13 - Number of received 64 Bytes framesStatsId = 0, */
    OamExtAttrRxFrame65_127StatsId          = 14,
    /*  14 - Number of received 65_127 Bytes framesStatsId = 0, */
    OamExtAttrRxFrame128_255StatsId         = 15,
    /*  15 - Number of received 128_255 Bytes framesStatsId = 0, */
    OamExtAttrRxFrame256_511StatsId         = 16,
    /*  16 - Number of received 256_511 Bytes framesStatsId = 0, */
    OamExtAttrRxFrame512_1023StatsId        = 17,
    /*  17 - Number of received 512_1023 Bytes framesStatsId = 0, */
    OamExtAttrRxFrame1024_1518StatsId       = 18,
    /*  18 - Number of received 1024_1518 Bytes framesStatsId = 0, */
    OamExtAttrRxFrame1519PlusStatsId        = 19,
    /*  19 - Number of received >1518 Bytes framesStatsId = 0, */

    OamExtAttrRxFramesDroppedStatsId        = 20,
    /*  20 - Number of received Dropped framesStatsId = 0, */
    OamExtAttrRxBytesDroppedStatsId         = 21,
    /*  21 - Number of received Dropped BytesStatsId = 0, */
    OamExtAttrRxBytesDelayedStatsId         = 22,
    /*  22 - Number of received Delayed BytesStatsId = 0, */
    OamExtAttrRxDelayStatsId                = 23,
    /*  23 - Number of received Delayed framesStatsId = 0, */

    OamAttrReserved0StatsId                 = 24,
    /*  24 - unused */

    OamAttrMacCtrlPauseRxStatsId            = 25,
    /*  25 - Number of received Pause framesStatsId = 0, */
    OamAttrOamLocalErrFrameSecsEventStatsId = 26,
    /*  26 - Number of local error eventsStatsId = 0, */

    OamAttrReserved1StatsId                 = 27,
    /*  27 - unusedStatsId = 0, */
    OamAttrReserved2StatsId                 = 28,
    /*  28 - unusedStatsId = 0, */

    /*  Tx stats */
    OamAttrMacOctetsTxOkStatsId             = 29,
    /*  29 - Number of transmitted good bytesStatsId = 0, */
    OamAttrMacFramesTxOkStatsId             = 30,
    /*  30 - Number of transmitted good framesStatsId = 0, */
    OamExtAttrTxUnicastFramesStatsId        = 31,
    /*  31 - Number of transmitted unicast framesStatsId = 0, */
    OamAttrMacMcastFramesTxOkStatsId        = 32,
    /*  32 - Number of transmitted multicast framesStatsId = 0, */
    OamAttrMacBcastFramesTxOkStatsId        = 33,
    /*  33 - Number of transmitted broadcast framesStatsId = 0, */

    OamAttrMacSingleCollFramesStatsId       = 34,
    /*  34 - Number of transmitted 1 collision framesStatsId = 0, */
    OamAttrMacMultipleCollFramesStatsId     = 35,
    /*  35 - Number of transmitted multi collisions framesStatsId = 0, */
    OamAttrMacLateCollisionsStatsId         = 36,
    /*  36 - Number of transmitted late collision framesStatsId = 0, */
    OamAttrMacExcessiveCollisionsStatsId    = 37,
    /*  37 - Number of transmitted excessive collisions framesStatsId = 0, */

    /*  bin sizes available on Ethernet ports only */
    OamExtAttrTxFrame64StatsId              = 38,
    /*  38 - Number of transmitted 64 Bytes framesStatsId = 0, */
    OamExtAttrTxFrame65_127StatsId          = 39,
    /*  39 - Number of transmitted 65_127 Bytes framesStatsId = 0, */
    OamExtAttrTxFrame128_255StatsId         = 40,
    /*  40 - Number of transmitted 128_255 Bytes framesStatsId = 0, */
    OamExtAttrTxFrame256_511StatsId         = 41,
    /*  41 - Number of transmitted 256_511 Bytes framesStatsId = 0, */
    OamExtAttrTxFrame512_1023StatsId        = 42,
    /*  42 - Number of transmitted 512_1023 Bytes framesStatsId = 0, */
    OamExtAttrTxFrame1024_1518StatsId       = 43,
    /*  43 - Number of transmitted 1024_1518 Bytes framesStatsId = 0, */
    OamExtAttrTxFrame1519PlusStatsId        = 44,
    /*  44 - Number of transmitted > 1518 Bytes framesStatsId = 0, */

    OamExtAttrTxFramesDroppedStatsId        = 45,
    /*  45 - Number of transmitted dropped framesStatsId = 0, */
    OamExtAttrTxBytesDroppedStatsId         = 46,
    /*  46 - Number of transmitted dropped bytesStatsId = 0, */
    OamExtAttrTxBytesDelayedStatsId         = 47,
    /*  47 - Number of transmitted delayed bytesStatsId = 0, */
    OamExtAttrTxDelayStatsId                = 48,
    /*  48 - Number of transmitted delayed framesStatsId = 0, */
    OamExtAttrTxBytesUnusedStatsId          = 49,
    /*  49 - Number of transmitted granted but not sent framesStatsId = 0, */

    OamAttrReserved3StatsId                 = 50,
    /*  50 - unused */

    OamAttrMacCtrlPauseTxStatsId            = 51,
    /*  51 - Number of transmitted Pause framesStatsId = 0, */
    OamAttrMpcpMACCtrlFramesTxStatsId       = 52,
    /*  52 - Number of transmitted Mpcp framesStatsId = 0, */
    OamAttrMpcpMACCtrlFramesRxStatsId       = 53,
    /*  53 - Number of received Mpcp framesStatsId = 0, */
    OamAttrMpcpTxRegAckStatsId              = 54,
    /*  54 - Number of transmitted Mpcp RegAck framesStatsId = 0, */
    OamAttrMpcpTxRegRequestStatsId          = 55,
    /*  55 - Number of transmitted Mpcp RegRequest framesStatsId = 0, */
    OamAttrMpcpTxReportStatsId              = 56,
    /*  56 - Number of transmitted Mpcp Report framesStatsId = 0, */
    OamAttrMpcpRxGateStatsId                = 57,
    /*  57 - Number of received Mpcp Gate framesStatsId = 0, */
    OamAttrMpcpRxRegisterStatsId            = 58
    /*  58 - Number of received Mpcp Register framesStatsId = 0, */
};

typedef struct {
    uint64  ifInOctets;
    uint64  ifInUcastPkts;
    uint64  ifInNUcastPkts;
    uint64  ifInDiscards;
    uint64  ifInErrors;
    uint64  ifOutOctets;
    uint64  ifOutUcastPkts;
    uint64  ifOutNUcastPkts;
    uint64  ifOutDiscards;
    uint64  ifOutErrors;
    uint64  reserved1;
    uint64  reserved2;
    uint64  reserved3;
    uint64  reserved4;
} PACK TkExtAllStats;

typedef struct {
    uint64  OamAttrMacFramesTxOk;       /*07 00 02*/
    uint64  OamAttrMacFramesRxOk;       /*07 00 05*/
    uint64  OamAttrMacFcsErr;           /*07 00 06*/
    uint64  OamAttrMacAlignErr;         /*07 00 07*/
    uint64  OamAttrMacOctetsTxOk;       /*07 00 08*/
    uint64  OamAttrMacOctetsRxOk;       /*07 00 0E*/
    uint64  OamAttrMacMcastFramesTxOk;  /*07 00 12*/
    uint64  OamAttrMacBcastFramesTxOk;  /*07 00 13*/
    uint64  OamAttrMacMcastFramesRxOk;  /*07 00 15*/
    uint64  OamAttrMacBcastFramesRxOk;  /*07 00 16*/
    uint64  OamAttrMacInRangeLenErr;    /*07 00 17*/
    uint64  OamAttrMacFrameTooLong;     /*07 00 19*/
    uint64  OamAttrMacCtrlPauseRx;      /*07 00 63*/
    uint64  OamExtAttrRxUnicastFrames;  /*07 00 84*/
    uint64  OamExtAttrTxUnicastFrames;  /*07 00 85*/ 
    uint64  OamExtAttrRxFrameTooShort;  /*07 00 86*/ 
    uint64  OamExtAttrRxFrame64;        /*07 00 87*/
    uint64  OamExtAttrRxFrame65_127;    /*07 00 88*/
    uint64  OamExtAttrRxFrame128_255;   /*07 00 89*/
    uint64  OamExtAttrRxFrame256_511;   /*07 00 8A*/
    uint64  OamExtAttrRxFrame512_1023;  /*07 00 8B*/
    uint64  OamExtAttrRxFrame1024_1518; /*07 00 8C*/
    uint64  OamExtAttrRxFrame1519Plus;  /*07 00 8D*/
    uint64  OamExtAttrTxFrame64;        /*07 00 8E*/
    uint64  OamExtAttrTxFrame65_127;    /*07 00 8F*/
    uint64  OamExtAttrTxFrame128_255;   /*07 00 90*/
    uint64  OamExtAttrTxFrame256_511;   /*07 00 91*/
    uint64  OamExtAttrTxFrame512_1023;  /*07 00 92*/
    uint64  OamExtAttrTxFrame1024_1518; /*07 00 93*/
    uint64  OamExtAttrTxFrame1519Plus;  /*07 00 94*/
    uint64  OamExtAttrTxDelay;          /*07 00 96*/
    uint64  OamExtAttrTxFramesDropped;  /*07 00 97*/
    uint64  OamExtAttrTxBytesDropped;   /*07 00 98*/
    uint64  OamExtAttrTxBytesDelayed;   /*07 00 99*/
    uint64  OamExtAttrTxBytesUnused;    /*07 00 9A*/
    uint64  OamExtAttrRxDelay;          /*07 00 9C*/
    uint64  OamExtAttrRxFramesDropped;  /*07 00 9D*/
    uint64  OamExtAttrRxBytesDropped;   /*07 00 9E*/
    uint64  OamExtAttrRxBytesDelayed;   /*07 00 9F*/
    uint64  OamAttrOamEmulCrc8Err;      /*07 00 f9*/
    uint32  OamAttrFecCorrectedBlocks;  /*07 01 24*/
    uint32  OamAttrFecUncorrectableBlocks;  /*07 01 25*/
} PACK TkMacAllPortStats1;

typedef struct {
    uint64 OamAttrMacOctetsRxOk;
    uint64 OamAttrMacFramesRxOk;
    uint64 OamExtAttrRxUnicastFrames;
    uint64 OamAttrMacMcastFramesRxOk;
    uint64 OamAttrMacBcastFramesRxOk;
    uint64 OamAttrMacFcsErr;
    uint64 OamAttrOamEmulCrc8Err;
    uint64 OamAttrPhySymbolErrDuringCarrier;
    uint64 OamExtAttrRxFrameTooShort;
    uint64 OamAttrMacFrameTooLong;
    uint64 OamAttrMacInRangeLenErr;
    uint64 OamAttrMacAlignErr;
    uint64 OamExtAttrRxFrame64;
    uint64 OamExtAttrRxFrame65_127;
    uint64 OamExtAttrRxFrame128_255;
    uint64 OamExtAttrRxFrame256_511;
    uint64 OamExtAttrRxFrame512_1023;
    uint64 OamExtAttrRxFrame1024_1518;
    uint64 OamExtAttrRxFrame1519Plus;
    uint64 OamExtAttrRxBytesDropped;
    uint64 OamExtAttrRxFramesDropped;
    uint64 OamExtAttrRxBytesDelayed;
    uint64 OamExtAttrRxDelay;
    uint64 OamAttrMacCtrlPauseRx;
    uint64 OamAttrOamLocalErrFrameSecsEvent;
    uint64 OamAttrMpcpMACCtrlFramesRx;
    uint64 OamAttrMpcpRxGate;
    uint64 OamAttrMpcpRxRegister;
    uint64 OamAttrFecCorrectedBlocks;
    uint64 OamAttrFecUncorrectableBlocks;
    
    uint64 OamAttrMacOctetsTxOk;
    uint64 OamAttrMacFramesTxOk;
    uint64 OamExtAttrTxUnicastFrames;
    uint64 OamAttrMacMcastFramesTxOk;
    uint64 OamAttrMacBcastFramesTxOk;
    uint64 OamAttrMacSingleCollFrames;
    uint64 OamAttrMacMultipleCollFrames;
    uint64 OamAttrMacLateCollisions;
    uint64 OamAttrMacExcessiveCollisions;
    uint64 OamExtAttrTxFrame64;
    uint64 OamExtAttrTxFrame65_127;
    uint64 OamExtAttrTxFrame128_255;
    uint64 OamExtAttrTxFrame256_511;
    uint64 OamExtAttrTxFrame512_1023;
    uint64 OamExtAttrTxFrame1024_1518;
    uint64 OamExtAttrTxFrame1519Plus;
    uint64 OamExtAttrTxBytesDropped;
    uint64 OamExtAttrTxFramesDropped;
    uint64 OamExtAttrTxBytesDelayed;
    uint64 OamExtAttrTxDelay;
    uint64 OamExtAttrTxBytesUnused;
    uint64 OamAttrMacCtrlPauseTx;
    uint64 OamAttrMpcpMACCtrlFramesTx;
    uint64 OamAttrMpcpTxRegAck;
    uint64 OamAttrMpcpTxRegRequest;
    uint64 OamAttrMpcpTxReport;
}TkStatAll;

typedef struct{
    uint64 OamAttrMacOctetsRxOk;
    uint64 OamAttrMacFramesRxOk;
    uint64 OamExtAttrRxUnicastFrames;
    uint64 OamAttrMacMcastFramesRxOk;
    uint64 OamAttrMacBcastFramesRxOk;
    uint64 OamAttrMacFcsErr;
    uint64 OamAttrOamEmulCrc8Err;
    uint64 OamAttrPhySymbolErrDuringCarrier;
    uint64 OamExtAttrRxFrameTooShort;
    uint64 OamExtAttrRxFrame64;
    uint64 OamExtAttrRxFrame65_127;
    uint64 OamExtAttrRxFrame128_255;
    uint64 OamExtAttrRxFrame256_511;
    uint64 OamExtAttrRxFrame512_1023;
    uint64 OamExtAttrRxFrame1024_1518;
    uint64 OamExtAttrRxFrame1519Plus;
    uint64 OamExtAttrRxBytesDropped;
    uint64 OamExtAttrRxFramesDropped;
    uint64 OamExtAttrRxBytesDelayed;
    uint64 OamExtAttrRxDelay;
    uint64 OamAttrOamLocalErrFrameSecsEvent;
    uint64 OamAttrFecCorrectedBlocks;
    uint64 OamAttrFecUncorrectableBlocks;
    /*tx*/
    uint64 OamAttrMacOctetsTxOk;
    uint64 OamAttrMacFramesTxOk;
    uint64 OamExtAttrTxUnicastFrames;
    uint64 OamAttrMacMcastFramesTxOk;
    uint64 OamAttrMacBcastFramesTxOk;
    uint64 OamExtAttrTxFrame64;
    uint64 OamExtAttrTxFrame65_127;
    uint64 OamExtAttrTxFrame128_255;
    uint64 OamExtAttrTxFrame256_511;
    uint64 OamExtAttrTxFrame512_1023;
    uint64 OamExtAttrTxFrame1024_1518;
    uint64 OamExtAttrTxFrame1519Plus;
    uint64 OamExtAttrTxBytesDropped;
    uint64 OamExtAttrTxFramesDropped;
    uint64 OamExtAttrTxBytesDelayed;
    uint64 OamExtAttrTxDelay;
    uint64 OamExtAttrTxBytesUnused;
}TkPonStatsGroup;

typedef struct {
    uint64 OamAttrMacOctetsRxOk;
    uint64 OamAttrMacFramesRxOk;
    uint64 OamExtAttrRxUnicastFrames;
    uint64 OamAttrMacMcastFramesRxOk;
    uint64 OamAttrMacBcastFramesRxOk;
    uint64 OamAttrMacFcsErr;
    uint64 OamExtAttrRxFrameTooShort;
    uint64 OamAttrMacFrameTooLong;
    uint64 OamAttrMacInRangeLenErr;
    uint64 OamAttrMacAlignErr;
    uint64 OamExtAttrRxFrame64;
    uint64 OamExtAttrRxFrame65_127;
    uint64 OamExtAttrRxFrame128_255;
    uint64 OamExtAttrRxFrame256_511;
    uint64 OamExtAttrRxFrame512_1023;
    uint64 OamExtAttrRxFrame1024_1518;
    uint64 OamExtAttrRxFrame1519Plus;
    uint64 OamAttrMacCtrlPauseRx;
    uint64 OamExtAttrRxBytesDropped;
    /*tx*/
    uint64 OamAttrMacOctetsTxOk;
    uint64 OamAttrMacFramesTxOk;
    uint64 OamExtAttrTxUnicastFrames;
    uint64 OamAttrMacMcastFramesTxOk;
    uint64 OamAttrMacBcastFramesTxOk;
    uint64 OamAttrMacSingleCollFrames;
    uint64 OamAttrMacMultipleCollFrames;
    uint64 OamAttrMacLateCollisions;
    uint64 OamAttrMacExcessiveCollisions;
    uint64 OamExtAttrTxFrame64;
    uint64 OamExtAttrTxFrame65_127;
    uint64 OamExtAttrTxFrame128_255;
    uint64 OamExtAttrTxFrame256_511;
    uint64 OamExtAttrTxFrame512_1023;
    uint64 OamExtAttrTxFrame1024_1518;
    uint64 OamExtAttrTxFrame1519Plus;
    uint64 OamExtAttrTxBytesDropped;
    uint64 OamExtAttrTxFramesDropped;
    uint64 OamAttrMacCtrlPauseTx;
}TkUniStatsGroup;

typedef struct {
    uint64 OamAttrMacOctetsRxOk;
    uint64 OamAttrMacFramesRxOk;
    uint64 OamExtAttrRxUnicastFrames;
    uint64 OamAttrMacMcastFramesRxOk;
    uint64 OamAttrMacBcastFramesRxOk;
    uint64 OamAttrMacFcsErr;
    uint64 OamExtAttrRxFrameTooShort;
    uint64 OamExtAttrRxFrame64;
    uint64 OamExtAttrRxFrame65_127;
    uint64 OamExtAttrRxFrame128_255;
    uint64 OamExtAttrRxFrame256_511;
    uint64 OamExtAttrRxFrame512_1023;
    uint64 OamExtAttrRxFrame1024_1518;
    uint64 OamExtAttrRxFrame1519Plus;
    uint64 OamExtAttrRxBytesDropped;
    uint64 OamExtAttrRxFramesDropped;
    uint64 OamExtAttrRxBytesDelayed;
    uint64 OamExtAttrRxDelay;
    uint64 OamAttrOamLocalErrFrameSecsEvent;
    uint64 OamAttrMpcpMACCtrlFramesRx;
    uint64 OamAttrMpcpRxGate;
    uint64 OamAttrMpcpRxRegister;
    /*tx*/
    uint64 OamAttrMacOctetsTxOk;
    uint64 OamAttrMacFramesTxOk;
    uint64 OamExtAttrTxUnicastFrames;
    uint64 OamAttrMacMcastFramesTxOk;
    uint64 OamAttrMacBcastFramesTxOk;
    uint64 OamExtAttrTxFrame64;
    uint64 OamExtAttrTxFrame65_127;
    uint64 OamExtAttrTxFrame128_255;
    uint64 OamExtAttrTxFrame256_511;
    uint64 OamExtAttrTxFrame512_1023;
    uint64 OamExtAttrTxFrame1024_1518;
    uint64 OamExtAttrTxFrame1519Plus;
    uint64 OamExtAttrTxBytesDropped;
    uint64 OamExtAttrTxFramesDropped;
    uint64 OamExtAttrTxBytesDelayed;
    uint64 OamExtAttrTxDelay;
    uint64 OamExtAttrTxBytesUnused;
    uint64 OamAttrMpcpMACCtrlFramesTx;
    uint64 OamAttrMpcpTxRegAck;
    uint64 OamAttrMpcpTxRegRequest;
    uint64 OamAttrMpcpTxReport;
}  TkLlidStatsGroup;

typedef struct {
    uint64 OamAttrMacOctetsRxOk;
    uint64 OamAttrMacOctetsTxOk;
    uint64 OamAttrMacFramesRxOk;
    uint64 OamAttrMacFramesTxOk;
    uint64 OamExtAttrRxBytesDropped;
    uint64 OamExtAttrTxBytesDropped;
    uint64 OamExtAttrRxFramesDropped;
    uint64 OamExtAttrTxFramesDropped;
    uint64 OamExtAttrRxBytesDelayed;
    uint64 OamExtAttrTxBytesDelayed;
    uint64 OamExtAttrRxDelay;
    uint64 OamExtAttrTxDelay;
}TkCosqStatsGroup;


/* send TK extension OAM message Get the EPON port stats from the ONU */
int     TkExtOamGetEponStats (uint8 pathId, uint8 LinkId, uint8 StatsId,
                uint64 * pCounter);

/* send TK extension OAM message Get the UNI port stats from the ONU */
int     TkExtOamGetUniStats (uint8 pathId, uint8 LinkId, uint8 port,
                uint8 StatsId, uint64 * pCounter);

/* send TK extension OAM message Get the link stats from the ONU */
int     TkExtOamGetLinkStats (uint8 pathId, uint8 LinkId, uint8 StatsId,
                uint64 * pCounter);

/* send TK extension OAM message Get the queue stats from the ONU */
int     TkExtOamGetQueueStats (uint8 pathId, uint8 LinkId, uint8 port,
                uint8 queue, uint8 StatsId, uint64 * pCounter);

/* send TK extension OAM message Set clear all stats to the ONU */
int     TkExtOamSetClrStats (uint8 pathId, uint8 LinkId);

int     TkExtOamGetUniAllStats (uint8 pathId, uint8 linkId, uint8 port,
                TkExtAllStats * pStats);

int     TkExtOamGetPortStats (uint8 pathId, uint8 linkId, uint8 port,
                TkMacAllPortStats1 * pStats);

int     TkExtOamGetPONPortStats (uint8 pathId, uint8 linkId,
                TkMacAllPortStats1 * pStats);

int     TkExtOamGetUNIPortStats (uint8 pathId, uint8 linkId, uint8 port,
                TkMacAllPortStats1 * pStats);
				
int     TkExtOamGetPortAllStats(uint8 pathId,uint8 linkId, uint8 port, 
                TkStatAll *stats);

int     TkExtOamGetLinkAllStats(uint8 pathId, uint8 linkId,uint8 linkNum, 
                TkStatAll *stats);

int     TkExtOamGetQueueAllStats(uint8 pathId,uint8 linkId, uint16 portNum, 
                uint16 linkNum, uint16 queueNum, TkStatAll *stats);

int     TkExtOamGetPonStatsGroup(uint8 pathId,uint8 linkId, 
                TkPonStatsGroup *stats);

int     TkExtOamGetUniStatsGroup(uint8 pathId,uint8 linkId, uint8 port, 
                TkUniStatsGroup *stats);

int     TkExtOamGetLlidStatsGroup(uint8 pathId,uint8 linkId, uint16 llid, 
                TkLlidStatsGroup *stats);

int     TkExtOamGetCosqStatsGroup(uint8 pathId,uint8 linkId, uint16 portNum, 
    uint16 linkNum, uint16 queueNum, TkCosqStatsGroup *stats);

#ifdef __cplusplus
}
#endif

#endif /* _SOC_EA_TkStatsApi_H */
