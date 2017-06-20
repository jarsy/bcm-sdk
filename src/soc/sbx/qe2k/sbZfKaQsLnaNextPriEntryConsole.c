/*
 * $Id: sbZfKaQsLnaNextPriEntryConsole.c,v 1.4 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include <shared/bsl.h>

#include "sbTypes.h"
#include <soc/sbx/sbWrappers.h>
#include "sbZfKaQsLnaNextPriEntryConsole.hx"



/* Print members in struct */
void
sbZfKaQsLnaNextPriEntry_Print(sbZfKaQsLnaNextPriEntry_t *pFromStruct) {
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("KaQsLnaNextPriEntry:: selport=0x%02x"), (unsigned int)  pFromStruct->m_nSelPort));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" nextpri24=0x%01x"), (unsigned int)  pFromStruct->m_nNextPri24));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" nextpri23=0x%01x"), (unsigned int)  pFromStruct->m_nNextPri23));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("KaQsLnaNextPriEntry:: nextpri22=0x%01x"), (unsigned int)  pFromStruct->m_nNextPri22));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" nextpri21=0x%01x"), (unsigned int)  pFromStruct->m_nNextPri21));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" nextpri20=0x%01x"), (unsigned int)  pFromStruct->m_nNextPri20));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("KaQsLnaNextPriEntry:: nextpri19=0x%01x"), (unsigned int)  pFromStruct->m_nNextPri19));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" nextpri18=0x%01x"), (unsigned int)  pFromStruct->m_nNextPri18));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" nextpri17=0x%01x"), (unsigned int)  pFromStruct->m_nNextPri17));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("KaQsLnaNextPriEntry:: nextpri16=0x%01x"), (unsigned int)  pFromStruct->m_nNextPri16));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" nextpri15=0x%01x"), (unsigned int)  pFromStruct->m_nNextPri15));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" nextpri14=0x%01x"), (unsigned int)  pFromStruct->m_nNextPri14));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("KaQsLnaNextPriEntry:: nextpri13=0x%01x"), (unsigned int)  pFromStruct->m_nNextPri13));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" nextpri12=0x%01x"), (unsigned int)  pFromStruct->m_nNextPri12));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" nextpri11=0x%01x"), (unsigned int)  pFromStruct->m_nNextPri11));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("KaQsLnaNextPriEntry:: nextpri10=0x%01x"), (unsigned int)  pFromStruct->m_nNextPri10));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" nextpri9=0x%01x"), (unsigned int)  pFromStruct->m_nNextPri9));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" nextpri8=0x%01x"), (unsigned int)  pFromStruct->m_nNextPri8));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" nextpri7=0x%01x"), (unsigned int)  pFromStruct->m_nNextPri7));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("KaQsLnaNextPriEntry:: nextpri6=0x%01x"), (unsigned int)  pFromStruct->m_nNextPri6));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" nextpri5=0x%01x"), (unsigned int)  pFromStruct->m_nNextPri5));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" nextpri4=0x%01x"), (unsigned int)  pFromStruct->m_nNextPri4));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" nextpri3=0x%01x"), (unsigned int)  pFromStruct->m_nNextPri3));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("KaQsLnaNextPriEntry:: nextpri2=0x%01x"), (unsigned int)  pFromStruct->m_nNextPri2));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" nextpri1=0x%01x"), (unsigned int)  pFromStruct->m_nNextPri1));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" nextpri0=0x%01x"), (unsigned int)  pFromStruct->m_nNextPri0));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

}

/* SPrint members in struct */
char *
sbZfKaQsLnaNextPriEntry_SPrint(sbZfKaQsLnaNextPriEntry_t *pFromStruct, char *pcToString, uint32 lStrSize) {
  uint32 WrCnt = 0x0;

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"KaQsLnaNextPriEntry:: selport=0x%02x", (unsigned int)  pFromStruct->m_nSelPort);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," nextpri24=0x%01x", (unsigned int)  pFromStruct->m_nNextPri24);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," nextpri23=0x%01x", (unsigned int)  pFromStruct->m_nNextPri23);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"KaQsLnaNextPriEntry:: nextpri22=0x%01x", (unsigned int)  pFromStruct->m_nNextPri22);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," nextpri21=0x%01x", (unsigned int)  pFromStruct->m_nNextPri21);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," nextpri20=0x%01x", (unsigned int)  pFromStruct->m_nNextPri20);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"KaQsLnaNextPriEntry:: nextpri19=0x%01x", (unsigned int)  pFromStruct->m_nNextPri19);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," nextpri18=0x%01x", (unsigned int)  pFromStruct->m_nNextPri18);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," nextpri17=0x%01x", (unsigned int)  pFromStruct->m_nNextPri17);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"KaQsLnaNextPriEntry:: nextpri16=0x%01x", (unsigned int)  pFromStruct->m_nNextPri16);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," nextpri15=0x%01x", (unsigned int)  pFromStruct->m_nNextPri15);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," nextpri14=0x%01x", (unsigned int)  pFromStruct->m_nNextPri14);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"KaQsLnaNextPriEntry:: nextpri13=0x%01x", (unsigned int)  pFromStruct->m_nNextPri13);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," nextpri12=0x%01x", (unsigned int)  pFromStruct->m_nNextPri12);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," nextpri11=0x%01x", (unsigned int)  pFromStruct->m_nNextPri11);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"KaQsLnaNextPriEntry:: nextpri10=0x%01x", (unsigned int)  pFromStruct->m_nNextPri10);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," nextpri9=0x%01x", (unsigned int)  pFromStruct->m_nNextPri9);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," nextpri8=0x%01x", (unsigned int)  pFromStruct->m_nNextPri8);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," nextpri7=0x%01x", (unsigned int)  pFromStruct->m_nNextPri7);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"KaQsLnaNextPriEntry:: nextpri6=0x%01x", (unsigned int)  pFromStruct->m_nNextPri6);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," nextpri5=0x%01x", (unsigned int)  pFromStruct->m_nNextPri5);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," nextpri4=0x%01x", (unsigned int)  pFromStruct->m_nNextPri4);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," nextpri3=0x%01x", (unsigned int)  pFromStruct->m_nNextPri3);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"KaQsLnaNextPriEntry:: nextpri2=0x%01x", (unsigned int)  pFromStruct->m_nNextPri2);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," nextpri1=0x%01x", (unsigned int)  pFromStruct->m_nNextPri1);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," nextpri0=0x%01x", (unsigned int)  pFromStruct->m_nNextPri0);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  /* assert if we've overrun the buffer */
  SB_ASSERT(WrCnt < lStrSize);
  return(pcToString);

}

/* validate members in struct (1 = PASS, 0 = FAIL) */
int
sbZfKaQsLnaNextPriEntry_Validate(sbZfKaQsLnaNextPriEntry_t *pZf) {

  if (pZf->m_nSelPort > 0x3f) return 0;
  if (pZf->m_nNextPri24 > 0xf) return 0;
  if (pZf->m_nNextPri23 > 0xf) return 0;
  if (pZf->m_nNextPri22 > 0xf) return 0;
  if (pZf->m_nNextPri21 > 0xf) return 0;
  if (pZf->m_nNextPri20 > 0xf) return 0;
  if (pZf->m_nNextPri19 > 0xf) return 0;
  if (pZf->m_nNextPri18 > 0xf) return 0;
  if (pZf->m_nNextPri17 > 0xf) return 0;
  if (pZf->m_nNextPri16 > 0xf) return 0;
  if (pZf->m_nNextPri15 > 0xf) return 0;
  if (pZf->m_nNextPri14 > 0xf) return 0;
  if (pZf->m_nNextPri13 > 0xf) return 0;
  if (pZf->m_nNextPri12 > 0xf) return 0;
  if (pZf->m_nNextPri11 > 0xf) return 0;
  if (pZf->m_nNextPri10 > 0xf) return 0;
  if (pZf->m_nNextPri9 > 0xf) return 0;
  if (pZf->m_nNextPri8 > 0xf) return 0;
  if (pZf->m_nNextPri7 > 0xf) return 0;
  if (pZf->m_nNextPri6 > 0xf) return 0;
  if (pZf->m_nNextPri5 > 0xf) return 0;
  if (pZf->m_nNextPri4 > 0xf) return 0;
  if (pZf->m_nNextPri3 > 0xf) return 0;
  if (pZf->m_nNextPri2 > 0xf) return 0;
  if (pZf->m_nNextPri1 > 0xf) return 0;
  if (pZf->m_nNextPri0 > 0xf) return 0;

  return 1; /* success */

}

/* populate field from string and value */
int
sbZfKaQsLnaNextPriEntry_SetField(sbZfKaQsLnaNextPriEntry_t *s, char* name, int value) {

  if (SB_STRCMP(name, "") == 0 ) {
    return -1;
  } else if (SB_STRCMP(name, "m_nselport") == 0) {
    s->m_nSelPort = value;
  } else if (SB_STRCMP(name, "m_nnextpri24") == 0) {
    s->m_nNextPri24 = value;
  } else if (SB_STRCMP(name, "m_nnextpri23") == 0) {
    s->m_nNextPri23 = value;
  } else if (SB_STRCMP(name, "m_nnextpri22") == 0) {
    s->m_nNextPri22 = value;
  } else if (SB_STRCMP(name, "m_nnextpri21") == 0) {
    s->m_nNextPri21 = value;
  } else if (SB_STRCMP(name, "m_nnextpri20") == 0) {
    s->m_nNextPri20 = value;
  } else if (SB_STRCMP(name, "m_nnextpri19") == 0) {
    s->m_nNextPri19 = value;
  } else if (SB_STRCMP(name, "m_nnextpri18") == 0) {
    s->m_nNextPri18 = value;
  } else if (SB_STRCMP(name, "m_nnextpri17") == 0) {
    s->m_nNextPri17 = value;
  } else if (SB_STRCMP(name, "m_nnextpri16") == 0) {
    s->m_nNextPri16 = value;
  } else if (SB_STRCMP(name, "m_nnextpri15") == 0) {
    s->m_nNextPri15 = value;
  } else if (SB_STRCMP(name, "m_nnextpri14") == 0) {
    s->m_nNextPri14 = value;
  } else if (SB_STRCMP(name, "m_nnextpri13") == 0) {
    s->m_nNextPri13 = value;
  } else if (SB_STRCMP(name, "m_nnextpri12") == 0) {
    s->m_nNextPri12 = value;
  } else if (SB_STRCMP(name, "m_nnextpri11") == 0) {
    s->m_nNextPri11 = value;
  } else if (SB_STRCMP(name, "m_nnextpri10") == 0) {
    s->m_nNextPri10 = value;
  } else if (SB_STRCMP(name, "m_nnextpri9") == 0) {
    s->m_nNextPri9 = value;
  } else if (SB_STRCMP(name, "m_nnextpri8") == 0) {
    s->m_nNextPri8 = value;
  } else if (SB_STRCMP(name, "m_nnextpri7") == 0) {
    s->m_nNextPri7 = value;
  } else if (SB_STRCMP(name, "m_nnextpri6") == 0) {
    s->m_nNextPri6 = value;
  } else if (SB_STRCMP(name, "m_nnextpri5") == 0) {
    s->m_nNextPri5 = value;
  } else if (SB_STRCMP(name, "m_nnextpri4") == 0) {
    s->m_nNextPri4 = value;
  } else if (SB_STRCMP(name, "m_nnextpri3") == 0) {
    s->m_nNextPri3 = value;
  } else if (SB_STRCMP(name, "m_nnextpri2") == 0) {
    s->m_nNextPri2 = value;
  } else if (SB_STRCMP(name, "m_nnextpri1") == 0) {
    s->m_nNextPri1 = value;
  } else if (SB_STRCMP(name, "m_nnextpri0") == 0) {
    s->m_nNextPri0 = value;
  } else {
    /* string failed to match any field--ignored */
    return -1;
  }

  return(0);

}
