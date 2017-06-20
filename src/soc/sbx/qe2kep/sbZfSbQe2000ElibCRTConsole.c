/*
 * $Id: sbZfSbQe2000ElibCRTConsole.c,v 1.5 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include <shared/bsl.h>

#include "sbTypesGlue.h"
#include <soc/sbx/sbWrappers.h>
#include "sbZfSbQe2000ElibCRTConsole.hx"



/* Print members in struct */
void
sbZfSbQe2000ElibCRT_Print(sbZfSbQe2000ElibCRT_t *pFromStruct) {
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("SbQe2000ElibCRT:: c15=0x%01x"), (unsigned int)  pFromStruct->m_nClass15));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" c14=0x%01x"), (unsigned int)  pFromStruct->m_nClass14));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" c13=0x%01x"), (unsigned int)  pFromStruct->m_nClass13));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" c12=0x%01x"), (unsigned int)  pFromStruct->m_nClass12));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" c11=0x%01x"), (unsigned int)  pFromStruct->m_nClass11));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" c10=0x%01x"), (unsigned int)  pFromStruct->m_nClass10));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" c09=0x%01x"), (unsigned int)  pFromStruct->m_nClass9));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("SbQe2000ElibCRT:: c08=0x%01x"), (unsigned int)  pFromStruct->m_nClass8));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" c07=0x%01x"), (unsigned int)  pFromStruct->m_nClass7));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" c06=0x%01x"), (unsigned int)  pFromStruct->m_nClass6));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" c05=0x%01x"), (unsigned int)  pFromStruct->m_nClass5));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" c04=0x%01x"), (unsigned int)  pFromStruct->m_nClass4));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" c03=0x%01x"), (unsigned int)  pFromStruct->m_nClass3));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" c02=0x%01x"), (unsigned int)  pFromStruct->m_nClass2));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("SbQe2000ElibCRT:: c01=0x%01x"), (unsigned int)  pFromStruct->m_nClass1));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" c00=0x%01x"), (unsigned int)  pFromStruct->m_nClass0));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

}

/* SPrint members in struct */
char *
sbZfSbQe2000ElibCRT_SPrint(sbZfSbQe2000ElibCRT_t *pFromStruct, char *pcToString, uint32 lStrSize) {
  uint32 WrCnt = 0x0;

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"SbQe2000ElibCRT:: c15=0x%01x", (unsigned int)  pFromStruct->m_nClass15);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," c14=0x%01x", (unsigned int)  pFromStruct->m_nClass14);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," c13=0x%01x", (unsigned int)  pFromStruct->m_nClass13);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," c12=0x%01x", (unsigned int)  pFromStruct->m_nClass12);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," c11=0x%01x", (unsigned int)  pFromStruct->m_nClass11);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," c10=0x%01x", (unsigned int)  pFromStruct->m_nClass10);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," c09=0x%01x", (unsigned int)  pFromStruct->m_nClass9);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"SbQe2000ElibCRT:: c08=0x%01x", (unsigned int)  pFromStruct->m_nClass8);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," c07=0x%01x", (unsigned int)  pFromStruct->m_nClass7);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," c06=0x%01x", (unsigned int)  pFromStruct->m_nClass6);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," c05=0x%01x", (unsigned int)  pFromStruct->m_nClass5);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," c04=0x%01x", (unsigned int)  pFromStruct->m_nClass4);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," c03=0x%01x", (unsigned int)  pFromStruct->m_nClass3);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," c02=0x%01x", (unsigned int)  pFromStruct->m_nClass2);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"SbQe2000ElibCRT:: c01=0x%01x", (unsigned int)  pFromStruct->m_nClass1);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," c00=0x%01x", (unsigned int)  pFromStruct->m_nClass0);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  /* assert if we've overrun the buffer */
  SB_ASSERT(WrCnt < lStrSize);
  return(pcToString);

}

/* validate members in struct (1 = PASS, 0 = FAIL) */
int
sbZfSbQe2000ElibCRT_Validate(sbZfSbQe2000ElibCRT_t *pZf) {

  if (pZf->m_nClass15 > 0xf) return 0;
  if (pZf->m_nClass14 > 0xf) return 0;
  if (pZf->m_nClass13 > 0xf) return 0;
  if (pZf->m_nClass12 > 0xf) return 0;
  if (pZf->m_nClass11 > 0xf) return 0;
  if (pZf->m_nClass10 > 0xf) return 0;
  if (pZf->m_nClass9 > 0xf) return 0;
  if (pZf->m_nClass8 > 0xf) return 0;
  if (pZf->m_nClass7 > 0xf) return 0;
  if (pZf->m_nClass6 > 0xf) return 0;
  if (pZf->m_nClass5 > 0xf) return 0;
  if (pZf->m_nClass4 > 0xf) return 0;
  if (pZf->m_nClass3 > 0xf) return 0;
  if (pZf->m_nClass2 > 0xf) return 0;
  if (pZf->m_nClass1 > 0xf) return 0;
  if (pZf->m_nClass0 > 0xf) return 0;

  return 1; /* success */

}

/* populate field from string and value */
int
sbZfSbQe2000ElibCRT_SetField(sbZfSbQe2000ElibCRT_t *s, char* name, int value) {

  if (SB_STRCMP(name, "") == 0 ) {
    return -1;
  } else if (SB_STRCMP(name, "m_nclass15") == 0) {
    s->m_nClass15 = value;
  } else if (SB_STRCMP(name, "m_nclass14") == 0) {
    s->m_nClass14 = value;
  } else if (SB_STRCMP(name, "m_nclass13") == 0) {
    s->m_nClass13 = value;
  } else if (SB_STRCMP(name, "m_nclass12") == 0) {
    s->m_nClass12 = value;
  } else if (SB_STRCMP(name, "m_nclass11") == 0) {
    s->m_nClass11 = value;
  } else if (SB_STRCMP(name, "m_nclass10") == 0) {
    s->m_nClass10 = value;
  } else if (SB_STRCMP(name, "m_nclass9") == 0) {
    s->m_nClass9 = value;
  } else if (SB_STRCMP(name, "m_nclass8") == 0) {
    s->m_nClass8 = value;
  } else if (SB_STRCMP(name, "m_nclass7") == 0) {
    s->m_nClass7 = value;
  } else if (SB_STRCMP(name, "m_nclass6") == 0) {
    s->m_nClass6 = value;
  } else if (SB_STRCMP(name, "m_nclass5") == 0) {
    s->m_nClass5 = value;
  } else if (SB_STRCMP(name, "m_nclass4") == 0) {
    s->m_nClass4 = value;
  } else if (SB_STRCMP(name, "m_nclass3") == 0) {
    s->m_nClass3 = value;
  } else if (SB_STRCMP(name, "m_nclass2") == 0) {
    s->m_nClass2 = value;
  } else if (SB_STRCMP(name, "m_nclass1") == 0) {
    s->m_nClass1 = value;
  } else if (SB_STRCMP(name, "m_nclass0") == 0) {
    s->m_nClass0 = value;
  } else {
    /* string failed to match any field--ignored */
    return -1;
  }

  return(0);

}
