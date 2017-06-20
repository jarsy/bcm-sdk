/*
 * $Id: sbZfKaQsQueueTableEntryConsole.c,v 1.3 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include <shared/bsl.h>

#include "sbTypes.h"
#include <soc/sbx/sbWrappers.h>
#include "sbZfKaQsQueueTableEntryConsole.hx"



/* Print members in struct */
void
sbZfKaQsQueueTableEntry_Print(sbZfKaQsQueueTableEntry_t *pFromStruct) {
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("KaQsQueueTableEntry:: credit=0x%07x"), (unsigned int)  pFromStruct->m_nCredit));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" hplen=0x%01x"), (unsigned int)  pFromStruct->m_nHpLen));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" depth=0x%01x"), (unsigned int)  pFromStruct->m_nDepth));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" q2ec=0x%05x"), (unsigned int)  pFromStruct->m_nQ2Ec));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("KaQsQueueTableEntry:: localq=0x%01x"), (unsigned int)  pFromStruct->m_nLocalQ));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" maxholdts=0x%01x"), (unsigned int)  pFromStruct->m_nMaxHoldTs));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" queuetype=0x%01x"), (unsigned int)  pFromStruct->m_nQueueType));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("KaQsQueueTableEntry:: shaperatemsb=0x%02x"), (unsigned int)  pFromStruct->m_nShapeRateMSB));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

}

/* SPrint members in struct */
char *
sbZfKaQsQueueTableEntry_SPrint(sbZfKaQsQueueTableEntry_t *pFromStruct, char *pcToString, uint32 lStrSize) {
  uint32 WrCnt = 0x0;

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"KaQsQueueTableEntry:: credit=0x%07x", (unsigned int)  pFromStruct->m_nCredit);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," hplen=0x%01x", (unsigned int)  pFromStruct->m_nHpLen);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," depth=0x%01x", (unsigned int)  pFromStruct->m_nDepth);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," q2ec=0x%05x", (unsigned int)  pFromStruct->m_nQ2Ec);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"KaQsQueueTableEntry:: localq=0x%01x", (unsigned int)  pFromStruct->m_nLocalQ);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," maxholdts=0x%01x", (unsigned int)  pFromStruct->m_nMaxHoldTs);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," queuetype=0x%01x", (unsigned int)  pFromStruct->m_nQueueType);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"KaQsQueueTableEntry:: shaperatemsb=0x%02x", (unsigned int)  pFromStruct->m_nShapeRateMSB);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  /* assert if we've overrun the buffer */
  SB_ASSERT(WrCnt < lStrSize);
  return(pcToString);

}

/* validate members in struct (1 = PASS, 0 = FAIL) */
int
sbZfKaQsQueueTableEntry_Validate(sbZfKaQsQueueTableEntry_t *pZf) {

  if (pZf->m_nCredit > 0x1ffffff) return 0;
  if (pZf->m_nHpLen > 0x3) return 0;
  if (pZf->m_nDepth > 0xf) return 0;
  if (pZf->m_nQ2Ec > 0x1ffff) return 0;
  if (pZf->m_nLocalQ > 0x1) return 0;
  if (pZf->m_nMaxHoldTs > 0x7) return 0;
  if (pZf->m_nQueueType > 0xf) return 0;
  if (pZf->m_nShapeRateMSB > 0xff) return 0;

  return 1; /* success */

}

/* populate field from string and value */
int
sbZfKaQsQueueTableEntry_SetField(sbZfKaQsQueueTableEntry_t *s, char* name, int value) {

  if (SB_STRCMP(name, "") == 0 ) {
    return -1;
  } else if (SB_STRCMP(name, "m_ncredit") == 0) {
    s->m_nCredit = value;
  } else if (SB_STRCMP(name, "m_nhplen") == 0) {
    s->m_nHpLen = value;
  } else if (SB_STRCMP(name, "m_ndepth") == 0) {
    s->m_nDepth = value;
  } else if (SB_STRCMP(name, "m_nq2ec") == 0) {
    s->m_nQ2Ec = value;
  } else if (SB_STRCMP(name, "m_nlocalq") == 0) {
    s->m_nLocalQ = value;
  } else if (SB_STRCMP(name, "m_nmaxholdts") == 0) {
    s->m_nMaxHoldTs = value;
  } else if (SB_STRCMP(name, "m_nqueuetype") == 0) {
    s->m_nQueueType = value;
  } else if (SB_STRCMP(name, "m_nshaperatemsb") == 0) {
    s->m_nShapeRateMSB = value;
  } else {
    /* string failed to match any field--ignored */
    return -1;
  }

  return(0);

}
