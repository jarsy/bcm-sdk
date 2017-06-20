/*
 * $Id: sbZfSbQe2000ElibVITConsole.c,v 1.5 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include <shared/bsl.h>

#include "sbTypesGlue.h"
#include <soc/sbx/sbWrappers.h>
#include "sbZfSbQe2000ElibVITConsole.hx"



/* Print members in struct */
void
sbZfSbQe2000ElibVIT_Print(sbZfSbQe2000ElibVIT_t *pFromStruct) {
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("SbQe2000ElibVIT:: rec3=0x%04x"), (unsigned int)  pFromStruct->m_record3));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" rec2=0x%04x"), (unsigned int)  pFromStruct->m_record2));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" rec1=0x%04x"), (unsigned int)  pFromStruct->m_record1));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" rec0=0x%04x"), (unsigned int)  pFromStruct->m_record0));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

}

/* SPrint members in struct */
char *
sbZfSbQe2000ElibVIT_SPrint(sbZfSbQe2000ElibVIT_t *pFromStruct, char *pcToString, uint32 lStrSize) {
  uint32 WrCnt = 0x0;

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"SbQe2000ElibVIT:: rec3=0x%04x", (unsigned int)  pFromStruct->m_record3);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," rec2=0x%04x", (unsigned int)  pFromStruct->m_record2);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," rec1=0x%04x", (unsigned int)  pFromStruct->m_record1);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," rec0=0x%04x", (unsigned int)  pFromStruct->m_record0);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  /* assert if we've overrun the buffer */
  SB_ASSERT(WrCnt < lStrSize);
  return(pcToString);

}

/* validate members in struct (1 = PASS, 0 = FAIL) */
int
sbZfSbQe2000ElibVIT_Validate(sbZfSbQe2000ElibVIT_t *pZf) {

  if (pZf->m_record3 > 0xffff) return 0;
  if (pZf->m_record2 > 0xffff) return 0;
  if (pZf->m_record1 > 0xffff) return 0;
  if (pZf->m_record0 > 0xffff) return 0;

  return 1; /* success */

}

/* populate field from string and value */
int
sbZfSbQe2000ElibVIT_SetField(sbZfSbQe2000ElibVIT_t *s, char* name, int value) {

  if (SB_STRCMP(name, "") == 0 ) {
    return -1;
  } else if (SB_STRCMP(name, "m_record3") == 0) {
    s->m_record3 = value;
  } else if (SB_STRCMP(name, "m_record2") == 0) {
    s->m_record2 = value;
  } else if (SB_STRCMP(name, "m_record1") == 0) {
    s->m_record1 = value;
  } else if (SB_STRCMP(name, "m_record0") == 0) {
    s->m_record0 = value;
  } else {
    /* string failed to match any field--ignored */
    return -1;
  }

  return(0);

}
