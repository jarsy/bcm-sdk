/*
 * $Id: sbZfKaQsPriLutAddrConsole.c,v 1.3 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include <shared/bsl.h>

#include "sbTypes.h"
#include <soc/sbx/sbWrappers.h>
#include "sbZfKaQsPriLutAddrConsole.hx"



/* Print members in struct */
void
sbZfKaQsPriLutAddr_Print(sbZfKaQsPriLutAddr_t *pFromStruct) {
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("KaQsPriLutAddr:: res=0x%05x"), (unsigned int)  pFromStruct->m_nReserved));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" shaped=0x%01x"), (unsigned int)  pFromStruct->m_nShaped));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" depth=0x%01x"), (unsigned int)  pFromStruct->m_nDepth));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" anemicaged=0x%01x"), (unsigned int)  pFromStruct->m_nAnemicAged));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" qtype=0x%01x"), (unsigned int)  pFromStruct->m_nQType));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("KaQsPriLutAddr:: efaged=0x%01x"), (unsigned int)  pFromStruct->m_nEfAged));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" creditlevel=0x%01x"), (unsigned int)  pFromStruct->m_nCreditLevel));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" holdts=0x%01x"), (unsigned int)  pFromStruct->m_nHoldTs));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" pktlen=0x%01x"), (unsigned int)  pFromStruct->m_nPktLen));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

}

/* SPrint members in struct */
char *
sbZfKaQsPriLutAddr_SPrint(sbZfKaQsPriLutAddr_t *pFromStruct, char *pcToString, uint32 lStrSize) {
  uint32 WrCnt = 0x0;

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"KaQsPriLutAddr:: res=0x%05x", (unsigned int)  pFromStruct->m_nReserved);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," shaped=0x%01x", (unsigned int)  pFromStruct->m_nShaped);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," depth=0x%01x", (unsigned int)  pFromStruct->m_nDepth);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," anemicaged=0x%01x", (unsigned int)  pFromStruct->m_nAnemicAged);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," qtype=0x%01x", (unsigned int)  pFromStruct->m_nQType);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"KaQsPriLutAddr:: efaged=0x%01x", (unsigned int)  pFromStruct->m_nEfAged);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," creditlevel=0x%01x", (unsigned int)  pFromStruct->m_nCreditLevel);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," holdts=0x%01x", (unsigned int)  pFromStruct->m_nHoldTs);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pktlen=0x%01x", (unsigned int)  pFromStruct->m_nPktLen);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  /* assert if we've overrun the buffer */
  SB_ASSERT(WrCnt < lStrSize);
  return(pcToString);

}

/* validate members in struct (1 = PASS, 0 = FAIL) */
int
sbZfKaQsPriLutAddr_Validate(sbZfKaQsPriLutAddr_t *pZf) {

  if (pZf->m_nReserved > 0x7ffff) return 0;
  if (pZf->m_nShaped > 0x1) return 0;
  if (pZf->m_nDepth > 0x7) return 0;
  if (pZf->m_nAnemicAged > 0x1) return 0;
  if (pZf->m_nQType > 0xf) return 0;
  if (pZf->m_nEfAged > 0x1) return 0;
  if (pZf->m_nCreditLevel > 0x1) return 0;
  if (pZf->m_nHoldTs > 0x1) return 0;
  if (pZf->m_nPktLen > 0x1) return 0;

  return 1; /* success */

}

/* populate field from string and value */
int
sbZfKaQsPriLutAddr_SetField(sbZfKaQsPriLutAddr_t *s, char* name, int value) {

  if (SB_STRCMP(name, "") == 0 ) {
    return -1;
  } else if (SB_STRCMP(name, "m_nreserved") == 0) {
    s->m_nReserved = value;
  } else if (SB_STRCMP(name, "m_nshaped") == 0) {
    s->m_nShaped = value;
  } else if (SB_STRCMP(name, "m_ndepth") == 0) {
    s->m_nDepth = value;
  } else if (SB_STRCMP(name, "m_nanemicaged") == 0) {
    s->m_nAnemicAged = value;
  } else if (SB_STRCMP(name, "m_nqtype") == 0) {
    s->m_nQType = value;
  } else if (SB_STRCMP(name, "m_nefaged") == 0) {
    s->m_nEfAged = value;
  } else if (SB_STRCMP(name, "m_ncreditlevel") == 0) {
    s->m_nCreditLevel = value;
  } else if (SB_STRCMP(name, "m_nholdts") == 0) {
    s->m_nHoldTs = value;
  } else if (SB_STRCMP(name, "m_npktlen") == 0) {
    s->m_nPktLen = value;
  } else {
    /* string failed to match any field--ignored */
    return -1;
  }

  return(0);

}
