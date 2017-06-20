/*
 * $Id: sbZfKaQmQueueStateEntryConsole.c,v 1.3 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include <shared/bsl.h>

#include "sbTypes.h"
#include <soc/sbx/sbWrappers.h>
#include "sbZfKaQmQueueStateEntryConsole.hx"



/* Print members in struct */
void
sbZfKaQmQueueStateEntry_Print(sbZfKaQmQueueStateEntry_t *pFromStruct) {
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("KaQmQueueStateEntry:: allocatedbuffscnt=0x%05x"), (unsigned int)  pFromStruct->m_nAllocatedBuffsCnt));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" qtailptr=0x%07x"), (unsigned int)  pFromStruct->m_nQTailPtr));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("KaQmQueueStateEntry:: qheadptr=0x%07x"), (unsigned int)  pFromStruct->m_nQHeadPtr));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" nobuffsallocated=0x%01x"), (unsigned int)  pFromStruct->m_nNoBuffsAllocated));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" overflow=0x%01x"), (unsigned int)  pFromStruct->m_nOverflow));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("KaQmQueueStateEntry:: minbuffers=0x%04x"), (unsigned int)  pFromStruct->m_nMinBuffers));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" maxbuffers=0x%04x"), (unsigned int)  pFromStruct->m_nMaxBuffers));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" local=0x%01x"), (unsigned int)  pFromStruct->m_nLocal));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("KaQmQueueStateEntry:: queuedepthinline16b=0x%07x"), (unsigned int)  pFromStruct->m_nQueueDepthInLine16B));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" anemicwatermarksel=0x%01x"), (unsigned int)  pFromStruct->m_nAnemicWatermarkSel));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("KaQmQueueStateEntry:: qetype=0x%01x"), (unsigned int)  pFromStruct->m_nQeType));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" enable=0x%01x"), (unsigned int)  pFromStruct->m_nEnable));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

}

/* SPrint members in struct */
char *
sbZfKaQmQueueStateEntry_SPrint(sbZfKaQmQueueStateEntry_t *pFromStruct, char *pcToString, uint32 lStrSize) {
  uint32 WrCnt = 0x0;

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"KaQmQueueStateEntry:: allocatedbuffscnt=0x%05x", (unsigned int)  pFromStruct->m_nAllocatedBuffsCnt);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," qtailptr=0x%07x", (unsigned int)  pFromStruct->m_nQTailPtr);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"KaQmQueueStateEntry:: qheadptr=0x%07x", (unsigned int)  pFromStruct->m_nQHeadPtr);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," nobuffsallocated=0x%01x", (unsigned int)  pFromStruct->m_nNoBuffsAllocated);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," overflow=0x%01x", (unsigned int)  pFromStruct->m_nOverflow);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"KaQmQueueStateEntry:: minbuffers=0x%04x", (unsigned int)  pFromStruct->m_nMinBuffers);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," maxbuffers=0x%04x", (unsigned int)  pFromStruct->m_nMaxBuffers);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," local=0x%01x", (unsigned int)  pFromStruct->m_nLocal);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"KaQmQueueStateEntry:: queuedepthinline16b=0x%07x", (unsigned int)  pFromStruct->m_nQueueDepthInLine16B);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," anemicwatermarksel=0x%01x", (unsigned int)  pFromStruct->m_nAnemicWatermarkSel);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"KaQmQueueStateEntry:: qetype=0x%01x", (unsigned int)  pFromStruct->m_nQeType);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," enable=0x%01x", (unsigned int)  pFromStruct->m_nEnable);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  /* assert if we've overrun the buffer */
  SB_ASSERT(WrCnt < lStrSize);
  return(pcToString);

}

/* validate members in struct (1 = PASS, 0 = FAIL) */
int
sbZfKaQmQueueStateEntry_Validate(sbZfKaQmQueueStateEntry_t *pZf) {

  if (pZf->m_nAllocatedBuffsCnt > 0x1ffff) return 0;
  if (pZf->m_nQTailPtr > 0x1ffffff) return 0;
  if (pZf->m_nQHeadPtr > 0x1ffffff) return 0;
  if (pZf->m_nNoBuffsAllocated > 0x1) return 0;
  if (pZf->m_nOverflow > 0x1) return 0;
  if (pZf->m_nMinBuffers > 0x3fff) return 0;
  if (pZf->m_nMaxBuffers > 0x3fff) return 0;
  if (pZf->m_nLocal > 0x1) return 0;
  if (pZf->m_nQueueDepthInLine16B > 0x1ffffff) return 0;
  if (pZf->m_nAnemicWatermarkSel > 0x7) return 0;
  if (pZf->m_nQeType > 0x1) return 0;
  if (pZf->m_nEnable > 0x1) return 0;

  return 1; /* success */

}

/* populate field from string and value */
int
sbZfKaQmQueueStateEntry_SetField(sbZfKaQmQueueStateEntry_t *s, char* name, int value) {

  if (SB_STRCMP(name, "") == 0 ) {
    return -1;
  } else if (SB_STRCMP(name, "m_nallocatedbuffscnt") == 0) {
    s->m_nAllocatedBuffsCnt = value;
  } else if (SB_STRCMP(name, "m_nqtailptr") == 0) {
    s->m_nQTailPtr = value;
  } else if (SB_STRCMP(name, "m_nqheadptr") == 0) {
    s->m_nQHeadPtr = value;
  } else if (SB_STRCMP(name, "m_nnobuffsallocated") == 0) {
    s->m_nNoBuffsAllocated = value;
  } else if (SB_STRCMP(name, "m_noverflow") == 0) {
    s->m_nOverflow = value;
  } else if (SB_STRCMP(name, "m_nminbuffers") == 0) {
    s->m_nMinBuffers = value;
  } else if (SB_STRCMP(name, "m_nmaxbuffers") == 0) {
    s->m_nMaxBuffers = value;
  } else if (SB_STRCMP(name, "m_nlocal") == 0) {
    s->m_nLocal = value;
  } else if (SB_STRCMP(name, "m_nqueuedepthinline16b") == 0) {
    s->m_nQueueDepthInLine16B = value;
  } else if (SB_STRCMP(name, "m_nanemicwatermarksel") == 0) {
    s->m_nAnemicWatermarkSel = value;
  } else if (SB_STRCMP(name, "m_nqetype") == 0) {
    s->m_nQeType = value;
  } else if (SB_STRCMP(name, "m_nenable") == 0) {
    s->m_nEnable = value;
  } else {
    /* string failed to match any field--ignored */
    return -1;
  }

  return(0);

}
