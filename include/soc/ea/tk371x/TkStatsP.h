/*
 * $Id: TkStatsP.h,v 1.5 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:        name.h
 * Purpose:     Purpose of the file
 */

#ifndef _EA_SOC_TkSdkStatsP_H_
#define _EA_SOC_TkSdkStatsP_H_

#include <sal/types.h>

/* maps StatId to OAM attribute leaf */
#define STATSID_NUM                 59

uint16 const ponStatsAttrLeafGroup[] = {
    OamAttrMacOctetsRxOk,
    OamAttrMacFramesRxOk,
    OamExtAttrRxUnicastFrames,
    OamAttrMacMcastFramesRxOk,
    OamAttrMacBcastFramesRxOk,
    OamAttrMacFcsErr,
    OamAttrOamEmulCrc8Err,
    OamAttrPhySymbolErrDuringCarrier,
    OamExtAttrRxFrameTooShort,
    OamExtAttrRxFrame64,
    OamExtAttrRxFrame65_127,
    OamExtAttrRxFrame128_255,
    OamExtAttrRxFrame256_511,
    OamExtAttrRxFrame512_1023,
    OamExtAttrRxFrame1024_1518,
    OamExtAttrRxFrame1519Plus,
    OamExtAttrRxBytesDropped,
    OamExtAttrRxFramesDropped,
    OamExtAttrRxBytesDelayed,
    OamExtAttrRxDelay,
    OamAttrOamLocalErrFrameSecsEvent,
    OamAttrFecCorrectedBlocks,
    OamAttrFecUncorrectableBlocks,  
    OamAttrMacOctetsTxOk,
    OamAttrMacFramesTxOk,
    OamExtAttrTxUnicastFrames,
    OamAttrMacMcastFramesTxOk,
    OamAttrMacBcastFramesTxOk,
    OamExtAttrTxFrame64,
    OamExtAttrTxFrame65_127,
    OamExtAttrTxFrame128_255,
    OamExtAttrTxFrame256_511,
    OamExtAttrTxFrame512_1023,
    OamExtAttrTxFrame1024_1518,
    OamExtAttrTxFrame1519Plus,
    OamExtAttrTxBytesDropped,
    OamExtAttrTxFramesDropped,
    OamExtAttrTxBytesDelayed,
    OamExtAttrTxDelay,
    OamExtAttrTxBytesUnused,
    0xFFFF
};

uint16 const uniStatsAttrLeafGroup[] = {
    OamAttrMacOctetsRxOk,
    OamAttrMacFramesRxOk,
    OamExtAttrRxUnicastFrames,
    OamAttrMacMcastFramesRxOk,
    OamAttrMacBcastFramesRxOk,
    OamAttrMacFcsErr,
    OamExtAttrRxFrameTooShort,
    OamAttrMacFrameTooLong,
    OamAttrMacInRangeLenErr,
    OamAttrMacAlignErr,
    OamExtAttrRxFrame64,
    OamExtAttrRxFrame65_127,
    OamExtAttrRxFrame128_255,
    OamExtAttrRxFrame256_511,
    OamExtAttrRxFrame512_1023,
    OamExtAttrRxFrame1024_1518,
    OamExtAttrRxFrame1519Plus,
    OamAttrMacCtrlPauseRx,
    OamExtAttrRxBytesDropped,
    
    OamAttrMacOctetsTxOk,
    OamAttrMacFramesTxOk,
    OamExtAttrTxUnicastFrames,
    OamAttrMacMcastFramesTxOk,
    OamAttrMacBcastFramesTxOk,
    OamAttrMacSingleCollFrames,
    OamAttrMacMultipleCollFrames,
    OamAttrMacLateCollisions,
    OamAttrMacExcessiveCollisions,
    OamExtAttrTxFrame64,
    OamExtAttrTxFrame65_127,
    OamExtAttrTxFrame128_255,
    OamExtAttrTxFrame256_511,
    OamExtAttrTxFrame512_1023,
    OamExtAttrTxFrame1024_1518,
    OamExtAttrTxFrame1519Plus,
    OamExtAttrTxBytesDropped,
    OamExtAttrTxFramesDropped,
    OamAttrMacCtrlPauseTx,
    0xFFFF
};

uint16 const llidStatsAttrLeafGroup[] = {
    OamAttrMacOctetsRxOk,
    OamAttrMacFramesRxOk,
    OamExtAttrRxUnicastFrames,
    OamAttrMacMcastFramesRxOk,
    OamAttrMacBcastFramesRxOk,
    OamAttrMacFcsErr,
    OamExtAttrRxFrameTooShort,
    OamExtAttrRxFrame64,
    OamExtAttrRxFrame65_127,
    OamExtAttrRxFrame128_255,
    OamExtAttrRxFrame256_511,
    OamExtAttrRxFrame512_1023,
    OamExtAttrRxFrame1024_1518,
    OamExtAttrRxFrame1519Plus,
    OamExtAttrRxBytesDropped,
    OamExtAttrRxFramesDropped,
    OamExtAttrRxBytesDelayed,
    OamExtAttrRxDelay,
    OamAttrOamLocalErrFrameSecsEvent,
    OamAttrMpcpMACCtrlFramesRx,
    OamAttrMpcpRxGate,
    OamAttrMpcpRxRegister,
    
    OamAttrMacOctetsTxOk,
    OamAttrMacFramesTxOk,
    OamExtAttrTxUnicastFrames,
    OamAttrMacMcastFramesTxOk,
    OamAttrMacBcastFramesTxOk,
    OamExtAttrTxFrame64,
    OamExtAttrTxFrame65_127,
    OamExtAttrTxFrame128_255,
    OamExtAttrTxFrame256_511,
    OamExtAttrTxFrame512_1023,
    OamExtAttrTxFrame1024_1518,
    OamExtAttrTxFrame1519Plus,
    OamExtAttrTxBytesDropped,
    OamExtAttrTxFramesDropped,
    OamExtAttrTxBytesDelayed,
    OamExtAttrTxDelay,
    OamExtAttrTxBytesUnused,
    OamAttrMpcpMACCtrlFramesTx,
    OamAttrMpcpTxRegAck,
    OamAttrMpcpTxRegRequest,
    OamAttrMpcpTxReport,
    0xFFFF
};

uint16 const cosqStatsAttrLeafGroup[] = {
    OamAttrMacOctetsRxOk,
    OamAttrMacOctetsTxOk,
    OamAttrMacFramesRxOk,
    OamAttrMacFramesTxOk,
    OamExtAttrRxBytesDropped,
    OamExtAttrTxBytesDropped,
    OamExtAttrRxFramesDropped,
    OamExtAttrTxFramesDropped,
    OamExtAttrRxBytesDelayed,
    OamExtAttrTxBytesDelayed,
    OamExtAttrRxDelay,
    OamExtAttrTxDelay,
    0xFFFF
};

uint16 const    StatIdToAttrLeaf[STATSID_NUM] = {   
    /*
     * Rx stats 
     */
    OamAttrMacOctetsRxOk,       /* 0 - Number of received good bytes, */
    OamAttrMacFramesRxOk,       /* 1 - Number of received frames, */
    OamExtAttrRxUnicastFrames,  /* 2 - Number of received unicast frames, */
    OamAttrMacMcastFramesRxOk,  /* 3 - Number of received multicast
                                 * frames, */
    OamAttrMacBcastFramesRxOk,  /* 4 - Number of received broadcast
                                 * frames, */

    OamAttrMacFcsErr,           /* 5 - Number of received FCS error
                                 * frames, */
    OamAttrOamEmulCrc8Err,      /* 6 - Number of received Crc8 error
                                 * frames, */
    OamAttrPhySymbolErrDuringCarrier,   /* 7 - Number of received LineCode 
                                         * error frames, */

    OamExtAttrRxFrameTooShort,  /* 8 - Number of received TooShort frames, 
                                 */
    OamAttrMacFrameTooLong,     /* 9 - Number of received TooLong frames, */
    OamAttrMacInRangeLenErr,    /* 10 - Number of received InRangeLen
                                 * Error frames, */

    OamAttrMacOutOfRangeLenErr, /* 11 - Number of received OutRangeLen
                                 * Error frames, */
    OamAttrMacAlignErr,         /* 12 - Number of received Align Error
                                 * frames, */

    /*
     * bin sizes available on Ethernet ports only 
     */
    OamExtAttrRxFrame64,        /* 13 - Number of received 64 Bytes
                                 * frames, */
    OamExtAttrRxFrame65_127,    /* 14 - Number of received 65_127 Bytes
                                 * frames, */
    OamExtAttrRxFrame128_255,   /* 15 - Number of received 128_255 Bytes
                                 * frames, */
    OamExtAttrRxFrame256_511,   /* 16 - Number of received 256_511 Bytes
                                 * frames, */
    OamExtAttrRxFrame512_1023,  /* 17 - Number of received 512_1023 Bytes
                                 * frames, */
    OamExtAttrRxFrame1024_1518, /* 18 - Number of received 1024_1518 Bytes 
                                 * frames, */
    OamExtAttrRxFrame1519Plus,  /* 19 - Number of received >1518 Bytes
                                 * frames, */

    OamExtAttrRxFramesDropped,  /* 20 - Number of received Dropped frames, 
                                 */
    OamExtAttrRxBytesDropped,   /* 21 - Number of received Dropped Bytes, */
    OamExtAttrRxBytesDelayed,   /* 22 - Number of received Delayed Bytes, */
    OamExtAttrRxDelay,          /* 23 - Number of received Delayed frames, 
                                 */

    0xffff,                     /* 24 - unused */

    OamAttrMacCtrlPauseRx,      /* 25 - Number of received Pause frames, */
    OamAttrOamLocalErrFrameSecsEvent,   /* 26 - Number of local error
                                         * events, */

    0xffff,                     /* 27 - unused, */
    0xffff,                     /* 28 - unused, */

    /*
     * Tx stats 
     */
    OamAttrMacOctetsTxOk,       /* 29 - Number of transmitted good bytes, */
    OamAttrMacFramesTxOk,       /* 30 - Number of transmitted good frames, 
                                 */
    OamExtAttrTxUnicastFrames,  /* 31 - Number of transmitted unicast
                                 * frames, */
    OamAttrMacMcastFramesTxOk,  /* 32 - Number of transmitted multicast
                                 * frames, */
    OamAttrMacBcastFramesTxOk,  /* 33 - Number of transmitted broadcast
                                 * frames, */

    OamAttrMacSingleCollFrames, /* 34 - Number of transmitted 1 collision
                                 * frames, */
    OamAttrMacMultipleCollFrames,   /* 35 - Number of transmitted multi
                                     * collisions frames, */
    OamAttrMacLateCollisions,   /* 36 - Number of transmitted late
                                 * collision frames, */
    OamAttrMacExcessiveCollisions,  /* 37 - Number of transmitted
                                     * excessive collisions frames, */

    /*
     * bin sizes available on Ethernet ports only 
     */
    OamExtAttrTxFrame64,        /* 38 - Number of transmitted 64 Bytes
                                 * frames, */
    OamExtAttrTxFrame65_127,    /* 39 - Number of transmitted 65_127 Bytes 
                                 * frames, */
    OamExtAttrTxFrame128_255,   /* 40 - Number of transmitted 128_255
                                 * Bytes frames, */
    OamExtAttrTxFrame256_511,   /* 41 - Number of transmitted 256_511
                                 * Bytes frames, */
    OamExtAttrTxFrame512_1023,  /* 42 - Number of transmitted 512_1023
                                 * Bytes frames, */
    OamExtAttrTxFrame1024_1518, /* 43 - Number of transmitted 1024_1518
                                 * Bytes frames, */
    OamExtAttrTxFrame1519Plus,  /* 44 - Number of transmitted >1518 Bytes
                                 * frames, */

    OamExtAttrTxFramesDropped,  /* 45 - Number of transmitted dropped
                                 * frames, */
    OamExtAttrTxBytesDropped,   /* 46 - Number of transmitted dropped
                                 * bytes, */
    OamExtAttrTxBytesDelayed,   /* 47 - Number of transmitted delayed
                                 * bytes, */
    OamExtAttrTxDelay,          /* 48 - Number of transmitted delayed
                                 * frames, */
    OamExtAttrTxBytesUnused,    /* 49 - Number of transmitted granted but
                                 * not sent frames, */

    0xffff,                     /* 50 - unused */

    OamAttrMacCtrlPauseTx,      /* 51 - Number of transmitted Pause
                                 * frames, */
    OamAttrMpcpMACCtrlFramesTx, /* 52 - Number of transmitted Mpcp frames, 
                                 */
    OamAttrMpcpMACCtrlFramesRx, /* 53 - Number of received Mpcp frames, */
    OamAttrMpcpTxRegAck,        /* 54 - Number of transmitted Mpcp RegAck
                                 * frames, */
    OamAttrMpcpTxRegRequest,    /* 55 - Number of transmitted Mpcp
                                 * RegRequest frames, */
    OamAttrMpcpTxReport,        /* 56 - Number of transmitted Mpcp Report
                                 * frames, */
    OamAttrMpcpRxGate,          /* 57 - Number of received Mpcp Gate
                                 * frames, */
    OamAttrMpcpRxRegister       /* 58 - Number of received Mpcp Register
                                 * frames, */
};


#endif	/* !_EA_SOC_TkSdkStatsP_H_ */
