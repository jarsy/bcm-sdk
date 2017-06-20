/*
 * $Id: sbZfSbQe2000ElibPTConsole.c,v 1.5 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include <shared/bsl.h>

#include "sbTypesGlue.h"
#include <soc/sbx/sbWrappers.h>
#include "sbZfSbQe2000ElibPTConsole.hx"



/* Print members in struct */
void
sbZfSbQe2000ElibPT_Print(sbZfSbQe2000ElibPT_t *pFromStruct) {
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("SbQe2000ElibPT:: enable15=0x%01x"), (unsigned int)  pFromStruct->m_bClassEnb15));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" enable14=0x%01x"), (unsigned int)  pFromStruct->m_bClassEnb14));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" enable13=0x%01x"), (unsigned int)  pFromStruct->m_bClassEnb13));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" enable12=0x%01x"), (unsigned int)  pFromStruct->m_bClassEnb12));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("SbQe2000ElibPT:: enable11=0x%01x"), (unsigned int)  pFromStruct->m_bClassEnb11));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" enable10=0x%01x"), (unsigned int)  pFromStruct->m_bClassEnb10));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" enable9=0x%01x"), (unsigned int)  pFromStruct->m_bClassEnb9));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" enable8=0x%01x"), (unsigned int)  pFromStruct->m_bClassEnb8));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("SbQe2000ElibPT:: enable7=0x%01x"), (unsigned int)  pFromStruct->m_bClassEnb7));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" enable6=0x%01x"), (unsigned int)  pFromStruct->m_bClassEnb6));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" enable5=0x%01x"), (unsigned int)  pFromStruct->m_bClassEnb5));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" enable4=0x%01x"), (unsigned int)  pFromStruct->m_bClassEnb4));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("SbQe2000ElibPT:: enable3=0x%01x"), (unsigned int)  pFromStruct->m_bClassEnb3));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" enable2=0x%01x"), (unsigned int)  pFromStruct->m_bClassEnb2));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" enable1=0x%01x"), (unsigned int)  pFromStruct->m_bClassEnb1));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" enable0=0x%01x"), (unsigned int)  pFromStruct->m_bClassEnb0));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" trans=0x%02x"), (unsigned int)  pFromStruct->m_nCountTrans));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("SbQe2000ElibPT:: resrv1=0x%02x"), (unsigned int)  pFromStruct->m_nReserved1));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" prepend=0x%01x"), (unsigned int)  pFromStruct->m_bPrepend));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" valid=0x%01x"), (unsigned int)  pFromStruct->m_bInstValid));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" inst=0x%08x"), (unsigned int)  pFromStruct->m_Instruction));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

}

/* SPrint members in struct */
char *
sbZfSbQe2000ElibPT_SPrint(sbZfSbQe2000ElibPT_t *pFromStruct, char *pcToString, uint32 lStrSize) {
  uint32 WrCnt = 0x0;

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"SbQe2000ElibPT:: enable15=0x%01x", (unsigned int)  pFromStruct->m_bClassEnb15);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," enable14=0x%01x", (unsigned int)  pFromStruct->m_bClassEnb14);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," enable13=0x%01x", (unsigned int)  pFromStruct->m_bClassEnb13);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," enable12=0x%01x", (unsigned int)  pFromStruct->m_bClassEnb12);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"SbQe2000ElibPT:: enable11=0x%01x", (unsigned int)  pFromStruct->m_bClassEnb11);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," enable10=0x%01x", (unsigned int)  pFromStruct->m_bClassEnb10);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," enable9=0x%01x", (unsigned int)  pFromStruct->m_bClassEnb9);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," enable8=0x%01x", (unsigned int)  pFromStruct->m_bClassEnb8);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"SbQe2000ElibPT:: enable7=0x%01x", (unsigned int)  pFromStruct->m_bClassEnb7);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," enable6=0x%01x", (unsigned int)  pFromStruct->m_bClassEnb6);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," enable5=0x%01x", (unsigned int)  pFromStruct->m_bClassEnb5);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," enable4=0x%01x", (unsigned int)  pFromStruct->m_bClassEnb4);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"SbQe2000ElibPT:: enable3=0x%01x", (unsigned int)  pFromStruct->m_bClassEnb3);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," enable2=0x%01x", (unsigned int)  pFromStruct->m_bClassEnb2);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," enable1=0x%01x", (unsigned int)  pFromStruct->m_bClassEnb1);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," enable0=0x%01x", (unsigned int)  pFromStruct->m_bClassEnb0);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," trans=0x%02x", (unsigned int)  pFromStruct->m_nCountTrans);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"SbQe2000ElibPT:: resrv1=0x%02x", (unsigned int)  pFromStruct->m_nReserved1);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," prepend=0x%01x", (unsigned int)  pFromStruct->m_bPrepend);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," valid=0x%01x", (unsigned int)  pFromStruct->m_bInstValid);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," inst=0x%08x", (unsigned int)  pFromStruct->m_Instruction);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  /* assert if we've overrun the buffer */
  SB_ASSERT(WrCnt < lStrSize);
  return(pcToString);

}

/* validate members in struct (1 = PASS, 0 = FAIL) */
int
sbZfSbQe2000ElibPT_Validate(sbZfSbQe2000ElibPT_t *pZf) {

  if (pZf->m_bClassEnb15 > 0x1) return 0;
  if (pZf->m_bClassEnb14 > 0x1) return 0;
  if (pZf->m_bClassEnb13 > 0x1) return 0;
  if (pZf->m_bClassEnb12 > 0x1) return 0;
  if (pZf->m_bClassEnb11 > 0x1) return 0;
  if (pZf->m_bClassEnb10 > 0x1) return 0;
  if (pZf->m_bClassEnb9 > 0x1) return 0;
  if (pZf->m_bClassEnb8 > 0x1) return 0;
  if (pZf->m_bClassEnb7 > 0x1) return 0;
  if (pZf->m_bClassEnb6 > 0x1) return 0;
  if (pZf->m_bClassEnb5 > 0x1) return 0;
  if (pZf->m_bClassEnb4 > 0x1) return 0;
  if (pZf->m_bClassEnb3 > 0x1) return 0;
  if (pZf->m_bClassEnb2 > 0x1) return 0;
  if (pZf->m_bClassEnb1 > 0x1) return 0;
  if (pZf->m_bClassEnb0 > 0x1) return 0;
  if (pZf->m_nCountTrans > 0xff) return 0;
  if (pZf->m_nReserved1 > 0x7f) return 0;
  if (pZf->m_bPrepend > 0x1) return 0;
  if (pZf->m_bInstValid > 0x1) return 0;
  if (pZf->m_Instruction > 0x7fffffff) return 0;

  return 1; /* success */

}

/* populate field from string and value */
int
sbZfSbQe2000ElibPT_SetField(sbZfSbQe2000ElibPT_t *s, char* name, int value) {

  if (SB_STRCMP(name, "") == 0 ) {
    return -1;
  } else if (SB_STRCMP(name, "classenb15") == 0) {
    s->m_bClassEnb15 = value;
  } else if (SB_STRCMP(name, "classenb14") == 0) {
    s->m_bClassEnb14 = value;
  } else if (SB_STRCMP(name, "classenb13") == 0) {
    s->m_bClassEnb13 = value;
  } else if (SB_STRCMP(name, "classenb12") == 0) {
    s->m_bClassEnb12 = value;
  } else if (SB_STRCMP(name, "classenb11") == 0) {
    s->m_bClassEnb11 = value;
  } else if (SB_STRCMP(name, "classenb10") == 0) {
    s->m_bClassEnb10 = value;
  } else if (SB_STRCMP(name, "classenb9") == 0) {
    s->m_bClassEnb9 = value;
  } else if (SB_STRCMP(name, "classenb8") == 0) {
    s->m_bClassEnb8 = value;
  } else if (SB_STRCMP(name, "classenb7") == 0) {
    s->m_bClassEnb7 = value;
  } else if (SB_STRCMP(name, "classenb6") == 0) {
    s->m_bClassEnb6 = value;
  } else if (SB_STRCMP(name, "classenb5") == 0) {
    s->m_bClassEnb5 = value;
  } else if (SB_STRCMP(name, "classenb4") == 0) {
    s->m_bClassEnb4 = value;
  } else if (SB_STRCMP(name, "classenb3") == 0) {
    s->m_bClassEnb3 = value;
  } else if (SB_STRCMP(name, "classenb2") == 0) {
    s->m_bClassEnb2 = value;
  } else if (SB_STRCMP(name, "classenb1") == 0) {
    s->m_bClassEnb1 = value;
  } else if (SB_STRCMP(name, "classenb0") == 0) {
    s->m_bClassEnb0 = value;
  } else if (SB_STRCMP(name, "m_ncounttrans") == 0) {
    s->m_nCountTrans = value;
  } else if (SB_STRCMP(name, "m_nreserved1") == 0) {
    s->m_nReserved1 = value;
  } else if (SB_STRCMP(name, "prepend") == 0) {
    s->m_bPrepend = value;
  } else if (SB_STRCMP(name, "instvalid") == 0) {
    s->m_bInstValid = value;
  } else if (SB_STRCMP(name, "m_instruction") == 0) {
    s->m_Instruction = value;
  } else {
    /* string failed to match any field--ignored */
    return -1;
  }

  return(0);

}
