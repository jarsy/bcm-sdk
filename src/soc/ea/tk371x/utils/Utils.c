/*
 * $Id: Utils.c,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:     Utils.c
 * Purpose: 
 *
 */

#include <soc/ea/tk371x/TkTypes.h>
#include <soc/ea/tk371x/TkOsUtil.h>
#include <soc/ea/tk371x/TkOsAlloc.h>
#include <soc/ea/tk371x/Oam.h>
#include <soc/ea/tk371x/TkUtils.h>
#include <soc/ea/tk371x/TkDebug.h>

void
htonll(uint64 src,uint64 *dst)
{
    uint32 testVal = 0x01020304;
    uint32 tmp64bits[2] = {0x0};
    uint32 ret64bits[2] = {0x0};
    
	if(soc_htonl(testVal) == testVal){
        sal_memcpy((void *)dst,(void *)&src, sizeof(uint64));   
        return;
    }

    sal_memcpy((void *)tmp64bits,(void *)&src, sizeof(uint64));

    ret64bits[0] = soc_htonl(tmp64bits[1]);
    ret64bits[1] = soc_htonl(tmp64bits[0]);

    sal_memcpy((void *)dst,(void *)ret64bits, sizeof(uint64));
    return ;
}

void
InitBufInfo(BufInfo * buf, uint16 size, uint8 * start)
{
    buf->len = size;
    buf->curr = start;
    buf->start = start;
    /* buf->onBufFullCall = NULL;*/
}

Bool
BufSkip(BufInfo * buf, uint16 len)
{
    if ((buf->start + buf->len) >= (buf->curr + len)) {
        buf->curr += len;
        return TRUE;
    }

    return FALSE;
}

uint16
BufGetUsed(const BufInfo * buf)
{
    return (uint16) (buf->curr - buf->start);
}

Bool
BufRead(BufInfo * buf, uint8 * to, uint16 len)
{
    if ((buf->start + buf->len) >= (buf->curr + len)) {
        sal_memcpy(to, buf->curr, len);
        buf->curr += len;
        return TRUE;
    }

    return FALSE;
}

Bool
BufReadU8(BufInfo * buf, uint8 * val)
{
    return BufRead(buf, val, sizeof(*val));
}

Bool
BufReadU16(BufInfo * buf, uint16 * val)
{
    return BufRead(buf, (uint8 *) val, sizeof(*val));
}

uint16
BufGetRemainingSize(const BufInfo * buf)
{
    return (uint16) ((buf->start + buf->len) - buf->curr);
}

Bool
BufWrite(BufInfo * buf, const uint8 * from, uint16 len)
{
    if ((buf->start + buf->len) >= (buf->curr + len)) {
        sal_memcpy(buf->curr, from, len);
        buf->curr += len;
        return TRUE;
    } else {
        return FALSE;
    }
}

Bool
BufWriteU16(BufInfo * buf, uint16 val)
{
    return BufWrite(buf, (uint8 *) & val, sizeof(val));
}

/*******************************************************************************
 * Miscellaneous Stuff
 ******************************************************************************/
void
BufDump(char *title, uint8 * buf, uint16 len)
{
    uint16          i;
    uint8           brk;

    if (title != NULL)
        TkDbgPrintf(("%s", title));
    TkDbgPrintf(("\nSz=%d\n", len));

    if (len > 0)
        TkDbgPrintf(("%02X", *buf));
    for (i = 1; i < len; i++) {
        if (0 == (i % 16))
            brk = '\n';
        else if (0 == (i % 8))
            brk = '-';
        else
            brk = ' ';
        TkDbgPrintf(("%c%02X", brk, *(char *)INT_TO_PTR(PTR_TO_INT(buf) + i)));
    }
    TkDbgPrintf(("\n"));
}

void
conditonBubbleSort(OamNewRuleCondition * pData, int32 count)
{
    int32           i;
    int32           j;
    OamNewRuleCondition tmp;

    for (i = 0; i < count; i++) {
        for (j = i + 1; j < count; j++) {
            if (pData[i].field > pData[j].field) {
                sal_memcpy(&tmp, &pData[i], sizeof(OamNewRuleCondition));
                sal_memcpy(&pData[i], &pData[j],
                           sizeof(OamNewRuleCondition));
                sal_memcpy(&pData[j], &tmp, sizeof(OamNewRuleCondition));
            }

        }
    }
    return;
}


void
Tk2BufU8(uint8 * buf, uint8 val)
{
    *buf = val;
}

void
Tk2BufU16(uint8 * buf, uint16 val)
{
    union {
        uint8           arryU8[2];
        uint16          arryU16;
    } tmpVal;

    tmpVal.arryU16 = soc_htons(val);

    sal_memcpy(buf, &(tmpVal.arryU8[0]), sizeof(uint16));

    return;
}

void
Tk2BufU32(uint8 * buf, uint32 val)
{
    union {
        uint8           arryU8[4];
        uint16          arryU16[2];
        uint32          arryU32;
    } tmpVal;

    tmpVal.arryU32 = soc_htonl(val);

    sal_memcpy(buf, &(tmpVal.arryU8[0]), sizeof(uint32));
}


/*******************************************************************************
 * OAM Handling Stuff
 ******************************************************************************/

#define IsErrorResponse(x)  ((x) > 128)

/*******************************************************************************
* InitBufInfo
*/
Bool
AddOamTlv(BufInfo * bufInfo,
          OamVarBranch branch, uint16 leaf, uint8 len, const uint8 * value)
{
    Bool            ret = FALSE;
    OamVarContainer *var = (OamVarContainer *) bufInfo->curr;

    /* non-zero length requires a value*/
    if ((len != 0) && (value == NULL)) {
        return ret;
    }

    if ((len <= 0x80) &&
        ((bufInfo->start + bufInfo->len) >
         (bufInfo->curr + len + sizeof(OamVarContainer))
        )) {
        var->branch = branch;
        var->leaf = soc_htons(leaf);
        if (len == 0) {
            var->length = 0x80;
        } else {
            var->length = len & 0x7F;   /* 0x80 is encoded as 0*/
            sal_memcpy(var->value, value, len); /* lint !e419 !e669 !e670*/
        }

        /*
         * move current pointer. Note that OamVarContainer includes an
         * extra
         * byte that is subtracted out.
         */
        bufInfo->curr += (sizeof(OamVarContainer) - 1) + len;
        ret = TRUE;
    }

    return ret;
}


Bool
FormatBranchLeaf(BufInfo * bufInfo, uint8 branch, OamAttrLeaf leaf)
{
    Bool            ret = FALSE;
    OamVarContainer *var = (OamVarContainer *) bufInfo->curr;
    uint8           len = sizeof(OamVarBranch) + sizeof(OamAttrLeaf);
    if (((bufInfo->start + bufInfo->len) > (bufInfo->curr + len))) {
        var->branch = branch;
        var->leaf = soc_htons(leaf);

        /* move current pointer.*/
        bufInfo->curr += len;
        ret = TRUE;
    }

    return ret;
}

void
CloseVarMsg(BufInfo * pBufInfo) 
{
    (void) BufWriteU16(pBufInfo, 0);
}


Bool
OamAddAttrLeaf(BufInfo * bufInfo, uint16 leaf)
{
    return FormatBranchLeaf(bufInfo, OamBranchAttribute,
                            (OamAttrLeaf) leaf);
}

Bool
OamAddCtcExtAttrLeaf(BufInfo * bufInfo, uint16 leaf)
{
    return FormatBranchLeaf(bufInfo,
                            OamCtcBranchExtAttribute, (OamAttrLeaf) leaf);
}

Bool
GetNextOamVar(BufInfo * pBufInfo, tGenOamVar * pOamVar, uint8 * tlvRet)
{
    Bool            more = TRUE;
    tGenOamVar     *temp = (tGenOamVar *) pBufInfo->curr;
    *tlvRet = RcFail;           /* not found by default*/
    if (BufGetRemainingSize(pBufInfo) != 0) {
        switch (*(pBufInfo->curr)) {
           

        case OamBranchAttribute:
        case OamBranchAction:
        case OamCtcBranchExtAttribute:
        case OamCtcBranchExtAction:
        case OamCtcBranchObjInst:
        case OamBranchNameBinding:
            {
                pOamVar->Branch = temp->Branch;
                
                pOamVar->Leaf = soc_htons(temp->Leaf);
                pOamVar->Width = temp->Width;
                pBufInfo->curr =
                    (pBufInfo->curr + sizeof(tGenOamVar)) -
                    sizeof(uint8 *);

                /* Set the data pointer to NULL if we receive an error*/
                /* response*/
                if (IsErrorResponse(temp->Width)) {
                    pOamVar->pValue = NULL;
                    *tlvRet = RcOnuReturnedErr;
                } else {
                    if (pOamVar->Width == 0) {
                        pOamVar->Width = 0x80;
                    } else if (pOamVar->Width == 0x80) {
                        pOamVar->Width = 0;
                    } else {
                        /* nothing*/
                    }

                    pOamVar->pValue = pBufInfo->curr;
                    pBufInfo->curr += pOamVar->Width;
                    *tlvRet = RcOk; /* Found it, no error*/
                }
            }
            break;

        case OamBranchTermination:
            more = FALSE;
            break;

        default:
            more = FALSE;
        }
    } else {
        more = FALSE;
        pOamVar->Width = 0;
    }

    return more;
}

Bool
GetEventTlv(BufInfo * pBufInfo, OamEventTlv ** pOamEventTlv, int *tlvError)
{
    OamEventTlv    *temp;
    Bool            ret = TRUE;

    if (NULL == pBufInfo || NULL == tlvError) {
        return FALSE;
    }

    temp = (OamEventTlv *) pBufInfo->curr;

    if (BufGetRemainingSize(pBufInfo) != 0) {
        switch (*(pBufInfo->curr)) {
        case OamEventErrSymbolPeriod:
        case OamEventErrFrameCount:
        case OamEventErrFramePeriod:
        case OamEventErrFrameSecSummary:
        case OamEventErrVendor:
        case OamEventErrVendorOld:
            {
                if ((pBufInfo->curr +
                     temp->length +
                     sizeof(OamEventTlv)) >
                    (pBufInfo->start + pBufInfo->len)) {
                    *tlvError = ERROR;
                    ret = FALSE;
                } else {
                    *tlvError = OK;
                    *pOamEventTlv = temp;
                    pBufInfo->curr += sizeof(OamEventTlv) + temp->length;
                    ret = TRUE;
                }
            }
            break;
        case OamEventEndOfTlvMarker:
            *tlvError = OK;
            ret = FALSE;
            break;
        default:
            *tlvError = ERROR;
            ret = FALSE;
            break;
        }
    } else {
        ret = FALSE;
        *tlvError = OK;
    }
    return ret;
}

uint8
SearchBranchLeaf(void *oamResp,
                 OamVarBranch branch, uint16 leaf, tGenOamVar * pOamVar)
{
    BufInfo         bufInfo;
    tGenOamVar      var;
    uint8           ret = RcFail;
    uint8           tlvRet = RcOk;

    InitBufInfo(&bufInfo, 1500, (uint8 *) oamResp);
    while (GetNextOamVar(&bufInfo, &var, &tlvRet)) {
        if ((var.Branch == branch)
            && (var.Leaf == leaf)) {
            if (tlvRet == RcOk) {
                *pOamVar = var;
                ret = RcOk;
            }
            break;
        }
    }

    return ret;
}

void
RuleDebug(uint8 * buf, uint32 len)
{
    int32           i;
    for (i = 0; i < len; i++) {
        TkDbgPrintf(("%02x ", buf[i]));
        if ((i + 1) % 16 == 0)
            TkDbgPrintf(("\n"));
    }
    TkDbgPrintf(("\n"));
}

void
CtcPortInstShow(CtcPortInst port)
{
    TkDbgPrintf(("\r\nport::portType:   %d\n", port.portType));
    TkDbgPrintf(("\r\nport::frameIndex: %d\n", port.frameIndex));
    TkDbgPrintf(("\r\nport::slotIndex:  %d\n", port.slotIndex));
    TkDbgPrintf(("\r\nport::portIndex:  %d\n", port.portIndex));
}

