/*
 * $Id: sbZfSbQe2000ElibVlanMemConsole.c,v 1.5 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include <shared/bsl.h>

#include "sbTypesGlue.h"
#include <soc/sbx/sbWrappers.h>
#include "sbZfSbQe2000ElibVlanMemConsole.hx"



/* Print members in struct */
void
sbZfSbQe2000ElibVlanMem_Print(sbZfSbQe2000ElibVlanMem_t *pFromStruct) {
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("SbQe2000ElibVlanMem:: patches=0x%01x"), (unsigned int)  pFromStruct->m_Patches));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" edges=0x%01x"), (unsigned int)  pFromStruct->m_Edges));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" numfree=0x%01x"), (unsigned int)  pFromStruct->m_NumFree));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" numused=0x%01x"), (unsigned int)  pFromStruct->m_NumUsed));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

}

/* SPrint members in struct */
char *
sbZfSbQe2000ElibVlanMem_SPrint(sbZfSbQe2000ElibVlanMem_t *pFromStruct, char *pcToString, uint32 lStrSize) {
  uint32 WrCnt = 0x0;

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"SbQe2000ElibVlanMem:: patches=0x%01x", (unsigned int)  pFromStruct->m_Patches);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," edges=0x%01x", (unsigned int)  pFromStruct->m_Edges);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," numfree=0x%01x", (unsigned int)  pFromStruct->m_NumFree);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," numused=0x%01x", (unsigned int)  pFromStruct->m_NumUsed);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  /* assert if we've overrun the buffer */
  SB_ASSERT(WrCnt < lStrSize);
  return(pcToString);

}

/* validate members in struct (1 = PASS, 0 = FAIL) */
int
sbZfSbQe2000ElibVlanMem_Validate(sbZfSbQe2000ElibVlanMem_t *pZf) {

  if (pZf->m_Patches > 0x1) return 0;
  if (pZf->m_Edges > 0x1) return 0;
  if (pZf->m_NumFree > 0x1) return 0;
  if (pZf->m_NumUsed > 0x1) return 0;

  return 1; /* success */

}

/* populate field from string and value */
int
sbZfSbQe2000ElibVlanMem_SetField(sbZfSbQe2000ElibVlanMem_t *s, char* name, int value) {

  if (SB_STRCMP(name, "") == 0 ) {
    return -1;
  } else if (SB_STRCMP(name, "m_patches") == 0) {
    s->m_Patches = value;
  } else if (SB_STRCMP(name, "m_edges") == 0) {
    s->m_Edges = value;
  } else if (SB_STRCMP(name, "m_numfree") == 0) {
    s->m_NumFree = value;
  } else if (SB_STRCMP(name, "m_numused") == 0) {
    s->m_NumUsed = value;
  } else {
    /* string failed to match any field--ignored */
    return -1;
  }

  return(0);

}
