/*
 * $Id: OamUtils.h,v 1.4 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:     TkOamUtils.h
 * Purpose: 
 *
 */

#ifndef _SOC_EA_TkOamUtils_H
#define _SOC_EA_TkOamUtils_H

#if defined(__cplusplus)
extern "C"  {
#endif

#include <soc/ea/tk371x/TkTypes.h>
#include <soc/ea/tk371x/TkUtils.h>
#include <soc/ea/tk371x/Oam.h>

#define OnuOamPass           0x01

#define OnuHostIfPhyIfMask   0xF0
#define OnuHostIfPhyIfSft    4
#define OnuHostIfLinkMask    0x0F

#define OamReservedFlagShift 8
#define OamReservedFlagMask  0xFF00
#define OamFlagMask          0x00FF

#define OamFlagLinkMask      0xF0
#define OamFlagLinkShift     4

#define OamFlagSrcIfMask     0x0C
#define OamFlagSrcIfShift    2


/* Teknovus OUI */
extern const IeeeOui    TeknovusOui;

/* CTC OUI */
extern const IeeeOui    CTCOui;


/*
 * OAM flag handling
 */
uint8   GetSourceForFlag (uint16 flags);
void    AttachOamFlag (uint8 source, uint16 * flags);
void    AttachOamFlagNoPass (uint8 source, uint16 * flags);

uint8 * OamFillExtHeader (uint16 flags, const IeeeOui * oui, uint8 * TxFrame);

int32   OamEthSend (uint8 pathId, MacAddr * dstAddr, uint8 * pDataBuf, 
                    uint32 dataLen);

/* send TK extension OAM message(Get/Set) and wait for response from the ONU */
int     TkOamRequest (uint8 pathId, uint8 linkId, OamFrame * txBuf, 
                      uint32 txLen, OamPdu * rxBuf, uint32 * pRxLen);

int     TkOamNoResRequest (uint8 pathId, uint8 linkId, OamFrame * txBuf,
                           uint32 txLen);

/* send TK extension OAM Get message with object instance and Get response 
 * from the ONU 
 */
int     TkExtOamObjGet (uint8 pathId, uint8 linkId, uint8 object,
                        OamObjIndex * index, uint8 branch, uint16 leaf,
                        uint8 * pRxBuf, uint32 * pRxLen);

/* send TK extension OAM Get message with object instance and Get response 
 * from the ONU 
 */
int     TkExtOamObjActGet (uint8 pathId, uint8 linkId, uint8 object,
                       OamObjIndex * index, uint8 branch, uint16 leaf,
                       uint8 paramLen, uint8 * params, uint8 * pRxBuf, 
                       uint32 * pRxLen);

/* send TK extension OAM Get message with object instance and Get response
 * (multi TLVs) from the ONU 
 */
int     TkExtOamObjGetMulti (uint8 pathId, uint8 linkId, uint8 object,
                OamObjIndex * index, uint8 branch, uint16 leaf,
                uint8 * pRxBuf, uint32 * pRxLen);

/* send TK extension OAM Get message without object instance and Get response
 * from the ONU 
 */
int     TkExtOamGet (uint8 pathId, uint8 linkId, uint8 branch, uint16 leaf,
                uint8 * pRxBuf, uint32 * pRxLen);

/* send TK extension OAM Get message without object instance and Get response
 * (multi TLVs) from the ONU 
 */
int     TkExtOamGetMulti (uint8 pathId, uint8 linkId, uint8 branch, uint16 leaf,
                uint8 * pRxBuf, uint32 * pRxLen);

/* send TK extension OAM Set message with object instance and Get return code */
uint8   TkExtOamObjSet (uint8 pathId, uint8 linkId, uint8 object,
                OamObjIndex * index, uint8 branch, uint16 leaf,
                uint8 * pTxBuf, uint8 txLen);

/* send TK extension OAM Set message without object instance and Get return code
 * from the ONU 
 */
uint8   TkExtOamSet (uint8 pathId, uint8 linkId, uint8 branch, uint16 leaf,
                uint8 * pTxBuf, uint8 txLen);

/* send OAM response message to OLT */
int32   TkExtOamResponse (uint8 pathId, uint8 linkId, OamFrame * txBuf,
                uint32 txLen);

uint8   TxOamDeliver (uint8 pathId, uint8 linkId, BufInfo * bufInfo,
                uint8 * pRxBuf, uint32 * pRxLen);

uint8   OamContSize (OamVarContainer const *cont);
OamVarContainer * NextCont (OamVarContainer const *cont);
uint8   TkOamMsgPrepare (uint8 pathId, BufInfo * bufInfo, uint8 tkOpCode);

void    TkExtOamSetReplyTimeout (uint8 pathId, uint32 val);
uint32  TkExtOamGetReplyTimeout (uint8 pathId);

#if defined(__cplusplus)
}
#endif

#endif /* _SOC_EA_TkOamUtils_H */
