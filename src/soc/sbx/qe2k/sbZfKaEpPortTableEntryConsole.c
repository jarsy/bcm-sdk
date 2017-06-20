/*
 * $Id: sbZfKaEpPortTableEntryConsole.c,v 1.3 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include <shared/bsl.h>

#include "sbTypes.h"
#include <soc/sbx/sbWrappers.h>
#include "sbZfKaEpPortTableEntryConsole.hx"



/* Print members in struct */
void
sbZfKaEpPortTableEntry_Print(sbZfKaEpPortTableEntry_t *pFromStruct) {
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("KaEpPortTableEntry:: enable15=0x%01x"), (unsigned int)  pFromStruct->m_nEnable15));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" enable14=0x%01x"), (unsigned int)  pFromStruct->m_nEnable14));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" enable13=0x%01x"), (unsigned int)  pFromStruct->m_nEnable13));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" enable12=0x%01x"), (unsigned int)  pFromStruct->m_nEnable12));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("KaEpPortTableEntry:: enable11=0x%01x"), (unsigned int)  pFromStruct->m_nEnable11));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" enable10=0x%01x"), (unsigned int)  pFromStruct->m_nEnable10));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" enable9=0x%01x"), (unsigned int)  pFromStruct->m_nEnable9));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" enable8=0x%01x"), (unsigned int)  pFromStruct->m_nEnable8));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("KaEpPortTableEntry:: enable7=0x%01x"), (unsigned int)  pFromStruct->m_nEnable7));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" enable6=0x%01x"), (unsigned int)  pFromStruct->m_nEnable6));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" enable5=0x%01x"), (unsigned int)  pFromStruct->m_nEnable5));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" enable4=0x%01x"), (unsigned int)  pFromStruct->m_nEnable4));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("KaEpPortTableEntry:: enable3=0x%01x"), (unsigned int)  pFromStruct->m_nEnable3));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" enable2=0x%01x"), (unsigned int)  pFromStruct->m_nEnable2));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" enable1=0x%01x"), (unsigned int)  pFromStruct->m_nEnable1));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" enable0=0x%01x"), (unsigned int)  pFromStruct->m_nEnable0));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("KaEpPortTableEntry:: resrv0=0x%01x"), (unsigned int)  pFromStruct->m_nReserved0));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" trans=0x%02x"), (unsigned int)  pFromStruct->m_nCountTrans));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" resrv1=0x%02x"), (unsigned int)  pFromStruct->m_nReserved1));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" prepend=0x%01x"), (unsigned int)  pFromStruct->m_nPrepend));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("KaEpPortTableEntry:: inst=0x%08x"), (unsigned int)  pFromStruct->m_Instruction));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

}

/* SPrint members in struct */
char *
sbZfKaEpPortTableEntry_SPrint(sbZfKaEpPortTableEntry_t *pFromStruct, char *pcToString, uint32 lStrSize) {
  uint32 WrCnt = 0x0;

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"KaEpPortTableEntry:: enable15=0x%01x", (unsigned int)  pFromStruct->m_nEnable15);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," enable14=0x%01x", (unsigned int)  pFromStruct->m_nEnable14);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," enable13=0x%01x", (unsigned int)  pFromStruct->m_nEnable13);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," enable12=0x%01x", (unsigned int)  pFromStruct->m_nEnable12);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"KaEpPortTableEntry:: enable11=0x%01x", (unsigned int)  pFromStruct->m_nEnable11);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," enable10=0x%01x", (unsigned int)  pFromStruct->m_nEnable10);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," enable9=0x%01x", (unsigned int)  pFromStruct->m_nEnable9);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," enable8=0x%01x", (unsigned int)  pFromStruct->m_nEnable8);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"KaEpPortTableEntry:: enable7=0x%01x", (unsigned int)  pFromStruct->m_nEnable7);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," enable6=0x%01x", (unsigned int)  pFromStruct->m_nEnable6);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," enable5=0x%01x", (unsigned int)  pFromStruct->m_nEnable5);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," enable4=0x%01x", (unsigned int)  pFromStruct->m_nEnable4);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"KaEpPortTableEntry:: enable3=0x%01x", (unsigned int)  pFromStruct->m_nEnable3);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," enable2=0x%01x", (unsigned int)  pFromStruct->m_nEnable2);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," enable1=0x%01x", (unsigned int)  pFromStruct->m_nEnable1);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," enable0=0x%01x", (unsigned int)  pFromStruct->m_nEnable0);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"KaEpPortTableEntry:: resrv0=0x%01x", (unsigned int)  pFromStruct->m_nReserved0);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," trans=0x%02x", (unsigned int)  pFromStruct->m_nCountTrans);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," resrv1=0x%02x", (unsigned int)  pFromStruct->m_nReserved1);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," prepend=0x%01x", (unsigned int)  pFromStruct->m_nPrepend);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"KaEpPortTableEntry:: inst=0x%08x", (unsigned int)  pFromStruct->m_Instruction);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  /* assert if we've overrun the buffer */
  SB_ASSERT(WrCnt < lStrSize);
  return(pcToString);

}

/* validate members in struct (1 = PASS, 0 = FAIL) */
int
sbZfKaEpPortTableEntry_Validate(sbZfKaEpPortTableEntry_t *pZf) {

  if (pZf->m_nEnable15 > 0x1) return 0;
  if (pZf->m_nEnable14 > 0x1) return 0;
  if (pZf->m_nEnable13 > 0x1) return 0;
  if (pZf->m_nEnable12 > 0x1) return 0;
  if (pZf->m_nEnable11 > 0x1) return 0;
  if (pZf->m_nEnable10 > 0x1) return 0;
  if (pZf->m_nEnable9 > 0x1) return 0;
  if (pZf->m_nEnable8 > 0x1) return 0;
  if (pZf->m_nEnable7 > 0x1) return 0;
  if (pZf->m_nEnable6 > 0x1) return 0;
  if (pZf->m_nEnable5 > 0x1) return 0;
  if (pZf->m_nEnable4 > 0x1) return 0;
  if (pZf->m_nEnable3 > 0x1) return 0;
  if (pZf->m_nEnable2 > 0x1) return 0;
  if (pZf->m_nEnable1 > 0x1) return 0;
  if (pZf->m_nEnable0 > 0x1) return 0;
  if (pZf->m_nReserved0 > 0x7) return 0;
  if (pZf->m_nCountTrans > 0x1f) return 0;
  if (pZf->m_nReserved1 > 0x7f) return 0;
  if (pZf->m_nPrepend > 0x1) return 0;
  /* pZf->m_Instruction implicitly masked by data type */

  return 1; /* success */

}

/* populate field from string and value */
int
sbZfKaEpPortTableEntry_SetField(sbZfKaEpPortTableEntry_t *s, char* name, int value) {

  if (SB_STRCMP(name, "") == 0 ) {
    return -1;
  } else if (SB_STRCMP(name, "m_nenable15") == 0) {
    s->m_nEnable15 = value;
  } else if (SB_STRCMP(name, "m_nenable14") == 0) {
    s->m_nEnable14 = value;
  } else if (SB_STRCMP(name, "m_nenable13") == 0) {
    s->m_nEnable13 = value;
  } else if (SB_STRCMP(name, "m_nenable12") == 0) {
    s->m_nEnable12 = value;
  } else if (SB_STRCMP(name, "m_nenable11") == 0) {
    s->m_nEnable11 = value;
  } else if (SB_STRCMP(name, "m_nenable10") == 0) {
    s->m_nEnable10 = value;
  } else if (SB_STRCMP(name, "m_nenable9") == 0) {
    s->m_nEnable9 = value;
  } else if (SB_STRCMP(name, "m_nenable8") == 0) {
    s->m_nEnable8 = value;
  } else if (SB_STRCMP(name, "m_nenable7") == 0) {
    s->m_nEnable7 = value;
  } else if (SB_STRCMP(name, "m_nenable6") == 0) {
    s->m_nEnable6 = value;
  } else if (SB_STRCMP(name, "m_nenable5") == 0) {
    s->m_nEnable5 = value;
  } else if (SB_STRCMP(name, "m_nenable4") == 0) {
    s->m_nEnable4 = value;
  } else if (SB_STRCMP(name, "m_nenable3") == 0) {
    s->m_nEnable3 = value;
  } else if (SB_STRCMP(name, "m_nenable2") == 0) {
    s->m_nEnable2 = value;
  } else if (SB_STRCMP(name, "m_nenable1") == 0) {
    s->m_nEnable1 = value;
  } else if (SB_STRCMP(name, "m_nenable0") == 0) {
    s->m_nEnable0 = value;
  } else if (SB_STRCMP(name, "m_nreserved0") == 0) {
    s->m_nReserved0 = value;
  } else if (SB_STRCMP(name, "m_ncounttrans") == 0) {
    s->m_nCountTrans = value;
  } else if (SB_STRCMP(name, "m_nreserved1") == 0) {
    s->m_nReserved1 = value;
  } else if (SB_STRCMP(name, "m_nprepend") == 0) {
    s->m_nPrepend = value;
  } else if (SB_STRCMP(name, "m_instruction") == 0) {
    s->m_Instruction = value;
  } else {
    /* string failed to match any field--ignored */
    return -1;
  }

  return(0);

}
