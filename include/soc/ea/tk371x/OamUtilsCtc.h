/*
 * $Id: OamUtilsCtc.h,v 1.3 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:     OamUtilsCtc.h
 * Purpose: 
 *
 */


 
#ifndef _SOC_EA_OamUtilsCtc_H
#define _SOC_EA_OamUtilsCtc_H

#if defined(__cplusplus)
extern "C"  {
#endif

#include <soc/ea/tk371x/TkTypes.h>
#include <soc/ea/tk371x/TkUtils.h>

#define Ctc21ObjPortInstPack(type,frame_id,slot_id,port_id) \
    (type)<<24|(frame_id)<<22|(slot_id)<<16|(port_id)

typedef struct {
    uint8 uniPortType;
    uint8 frameNum;
    uint8 slowNum;
    uint16 uniPortNum;
} CtcOamObjPortInstant;

typedef struct{
    uint16 type;
    CtcOamObjPortInstant inst;
} CtcOamObj21;

uint8   CtcOamMsgPrepare (uint8 pathId, BufInfo * bufInfo, uint8 tkOpCode);

/* send TK extension OAM Get message with object instance and Get response from the ONU */
int     CtcExtOamObjGet (uint8 pathId, uint8 linkId, uint8 objBranch, uint16 objLeaf,
                uint32 objIndex, uint8 branch, uint16 leaf,
                uint8 * pRxBuf, uint32 * pRxLen);

/* send TK extension OAM Get message with object instance and Get response from the ONU */
int     CtcExtOamObjActGet (uint8 pathId, uint8 linkId, uint8 objBranch,
                uint16 objLeaf, uint32 objIndex, uint8 branch,
                uint16 leaf, uint8 paramLen, uint8 * params,
                uint8 * pRxBuf, uint32 * pRxLen);

/* send TK extension OAM Get message with object instance and Get response(multi TLVs) 
   from the ONU */
int     CtcExtOamObjGetMulti (uint8 pathId, uint8 linkId, uint8 objBranch,
                uint16 objLeaf, uint32 objIndex, uint8 branch,
                uint16 leaf, uint8 * pRxBuf, uint32 * pRxLen);

/* send TK extension OAM Get message without object instance and Get response
   from the ONU */
int     CtcExtOamGet (uint8 pathId, uint8 linkId, uint8 branch, uint16 leaf,
                uint8 * pRxBuf, uint32 * pRxLen);

/* send TK extension OAM Get message without object instance and Get response(multi TLVs)
   from the ONU */
int     CtcExtOamGetMulti (uint8 pathId, uint8 linkId, uint8 branch, uint16 leaf,
                uint8 * pRxBuf, uint32 * pRxLen);

/* send TK extension OAM Set message with object instance and Get return code */
uint8   CtcExtOamObjSet (uint8 pathId, uint8 linkId, uint8 objBranch,
                uint16 objLeaf, uint32 objIndex, uint8 branch,
                uint16 leaf, uint8 * pTxBuf, uint8 txLen);

/* send TK extension OAM Set message without object instance and Get return code
   from the ONU */
uint8   CtcExtOamSet (uint8 pathId, uint8 linkId, uint8 branch, uint16 leaf,
                uint8 * pTxBuf, uint8 txLen);

int32   CtcExtOam21ObjInstPack(uint8 type, uint16 index, uint32 *pObjInst);

#if defined(__cplusplus)
}
#endif

#endif /* _SOC_EA_OamUtilsCtc_H */
