/*
 * $Id: sbZfSbQe2000ElibCITConsole.c,v 1.5 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include <shared/bsl.h>

#include "sbTypesGlue.h"
#include <soc/sbx/sbWrappers.h>
#include "sbZfSbQe2000ElibCITConsole.hx"



/* Print members in struct */
void
sbZfSbQe2000ElibCIT_Print(sbZfSbQe2000ElibCIT_t *pFromStruct) {
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("SbQe2000ElibCIT:: inst7=0x%08x"), (unsigned int)  pFromStruct->m_Instruction7));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" inst6=0x%08x"), (unsigned int)  pFromStruct->m_Instruction6));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" inst5=0x%08x"), (unsigned int)  pFromStruct->m_Instruction5));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("SbQe2000ElibCIT:: inst4=0x%08x"), (unsigned int)  pFromStruct->m_Instruction4));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" inst3=0x%08x"), (unsigned int)  pFromStruct->m_Instruction3));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" inst2=0x%08x"), (unsigned int)  pFromStruct->m_Instruction2));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("SbQe2000ElibCIT:: inst1=0x%08x"), (unsigned int)  pFromStruct->m_Instruction1));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" inst0=0x%08x"), (unsigned int)  pFromStruct->m_Instruction0));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

}

/* SPrint members in struct */
char *
sbZfSbQe2000ElibCIT_SPrint(sbZfSbQe2000ElibCIT_t *pFromStruct, char *pcToString, uint32 lStrSize) {
  uint32 WrCnt = 0x0;

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"SbQe2000ElibCIT:: inst7=0x%08x", (unsigned int)  pFromStruct->m_Instruction7);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," inst6=0x%08x", (unsigned int)  pFromStruct->m_Instruction6);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," inst5=0x%08x", (unsigned int)  pFromStruct->m_Instruction5);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"SbQe2000ElibCIT:: inst4=0x%08x", (unsigned int)  pFromStruct->m_Instruction4);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," inst3=0x%08x", (unsigned int)  pFromStruct->m_Instruction3);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," inst2=0x%08x", (unsigned int)  pFromStruct->m_Instruction2);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"SbQe2000ElibCIT:: inst1=0x%08x", (unsigned int)  pFromStruct->m_Instruction1);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," inst0=0x%08x", (unsigned int)  pFromStruct->m_Instruction0);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  /* assert if we've overrun the buffer */
  SB_ASSERT(WrCnt < lStrSize);
  return(pcToString);

}

/* validate members in struct (1 = PASS, 0 = FAIL) */
int
sbZfSbQe2000ElibCIT_Validate(sbZfSbQe2000ElibCIT_t *pZf) {

  /* coverity[result_independent_of_operands] */
  if (pZf->m_Instruction7 > 0x0FFFFFFFF) return 0;
  /* coverity[result_independent_of_operands] */
  if (pZf->m_Instruction6 > 0x0FFFFFFFF) return 0;
  /* coverity[result_independent_of_operands] */
  if (pZf->m_Instruction5 > 0x0FFFFFFFF) return 0;
  /* coverity[result_independent_of_operands] */
  if (pZf->m_Instruction4 > 0x0FFFFFFFF) return 0;
  /* coverity[result_independent_of_operands] */
  if (pZf->m_Instruction3 > 0x0FFFFFFFF) return 0;
  /* coverity[result_independent_of_operands] */
  if (pZf->m_Instruction2 > 0x0FFFFFFFF) return 0;
  /* coverity[result_independent_of_operands] */
  if (pZf->m_Instruction1 > 0x0FFFFFFFF) return 0;
  /* coverity[result_independent_of_operands] */
  if (pZf->m_Instruction0 > 0x0FFFFFFFF) return 0;

  return 1; /* success */

}

/* populate field from string and value */
int
sbZfSbQe2000ElibCIT_SetField(sbZfSbQe2000ElibCIT_t *s, char* name, int value) {

  if (SB_STRCMP(name, "") == 0 ) {
    return -1;
  } else if (SB_STRCMP(name, "m_instruction7") == 0) {
    s->m_Instruction7 = value;
  } else if (SB_STRCMP(name, "m_instruction6") == 0) {
    s->m_Instruction6 = value;
  } else if (SB_STRCMP(name, "m_instruction5") == 0) {
    s->m_Instruction5 = value;
  } else if (SB_STRCMP(name, "m_instruction4") == 0) {
    s->m_Instruction4 = value;
  } else if (SB_STRCMP(name, "m_instruction3") == 0) {
    s->m_Instruction3 = value;
  } else if (SB_STRCMP(name, "m_instruction2") == 0) {
    s->m_Instruction2 = value;
  } else if (SB_STRCMP(name, "m_instruction1") == 0) {
    s->m_Instruction1 = value;
  } else if (SB_STRCMP(name, "m_instruction0") == 0) {
    s->m_Instruction0 = value;
  } else {
    /* string failed to match any field--ignored */
    return -1;
  }

  return(0);

}
