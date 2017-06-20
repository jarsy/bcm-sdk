/*
 * $Id: TkUtils.h,v 1.1 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:     tkUtils.h
 * Purpose: 
 *
 */

#ifndef _SOC_EA_TKUTILS_H
#define _SOC_EA_TKUTILS_H

#if defined(__cplusplus)
extern "C"  {
#endif

#include <soc/ea/tk371x/TkTypes.h>
#include <soc/ea/tk371x/Oam.h>
#include <soc/ea/tk371x/CtcOam.h>

#define TkMakeU8(x)     (uint8)(*(uint8 *)x)
#define TkMakeU16(x)    (uint16)((*(uint8 *)x<<8)|*((uint8 *)x+1))
#define TkMakeU32(x)    (uint32)((*(uint8 *)x<<24)|(*((uint8 *)x+1)<<16) \
                                |(*((uint8 *)x+2)<<8)|(*((uint8 *)x+3)))


/************************************************************************
 * BufInfo Stuff
 ************************************************************************/

typedef struct {
    uint8     * start;       /* Pointer to the start of the buffer*/
    uint8     * curr;        /* Pointer to the current position in the buffer*/
    uint16      len;         /* Total Length of buffer*/
} BufInfo;

void    InitBufInfo (BufInfo * buf, uint16 size, uint8 * start);
Bool    BufSkip (BufInfo * buf, uint16 len);
Bool    BufRead (BufInfo * buf, uint8 * to, uint16 len);
Bool    BufReadU8 (BufInfo * buf, uint8 * val);
Bool    BufReadU16 (BufInfo * buf, uint16 * val);
Bool    BufWrite (BufInfo * buf, const uint8 * from, uint16 len);
Bool    BufWriteU16 (BufInfo * buf, uint16 val);
uint16  BufGetUsed (const BufInfo * buf);
uint16  BufGetRemainingSize (const BufInfo * buf);

/************************************************************************
 * Miscellaneous Stuff
 ************************************************************************/

void    htonll (uint64 src, uint64 *dst);

void Tk2BufU8 (uint8 * buf, uint8 val);
void Tk2BufU16 (uint8 * buf, uint16 val);
void Tk2BufU32 (uint8 * buf, uint32 val);

void    BufDump (char *title, uint8 * buf, uint16 len);

/************************************************************************
 * OAM Handling Routines
 ************************************************************************/
Bool    AddOamTlv (BufInfo * bufInfo, OamVarBranch branch, uint16 leaf,
                uint8 len, const uint8 * value);
Bool    FormatBranchLeaf (BufInfo * bufInfo, uint8 branch, OamAttrLeaf leaf);
Bool    OamAddAttrLeaf (BufInfo * bufInfo, uint16 leaf);
Bool    OamAddCtcExtAttrLeaf (BufInfo * bufInfo, uint16 leaf);
Bool    GetNextOamVar (BufInfo * pBufInfo, tGenOamVar * pOamVar, uint8 * tlvRet);
Bool    GetEventTlv (BufInfo * pBufInfo, OamEventTlv ** pOamEventTlv, 
                int *tlvError);
uint8   SearchBranchLeaf (void *oamResp, OamVarBranch branch, uint16 leaf,
                tGenOamVar * pOamVar);

void    RuleDebug (uint8 * buf, uint32 len);
void    CtcPortInstShow (CtcPortInst port);
void    conditonBubbleSort (OamNewRuleCondition * pData, int32 count);



#if defined(__cplusplus)
}
#endif

#endif /* _SOC_EA_TKUTILS_H */
