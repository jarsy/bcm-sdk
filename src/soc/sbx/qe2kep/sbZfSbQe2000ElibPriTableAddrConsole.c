/*
 * $Id: sbZfSbQe2000ElibPriTableAddrConsole.c,v 1.5 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include <shared/bsl.h>

#include "sbTypesGlue.h"
#include <soc/sbx/sbWrappers.h>
#include "sbZfSbQe2000ElibPriTableAddrConsole.hx"



/* Print members in struct */
void
sbZfSbQe2000ElibPriTableAddr_Print(sbZfSbQe2000ElibPriTableAddr_t *pFromStruct) {
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("SbQe2000ElibPriTableAddr:: port=0x%02x"), (unsigned int)  pFromStruct->m_nPort));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" cos=0x%01x"), (unsigned int)  pFromStruct->m_nCos));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" dp=0x%01x"), (unsigned int)  pFromStruct->m_nDp));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" ecn=0x%01x"), (unsigned int)  pFromStruct->m_nEcn));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

}

/* SPrint members in struct */
char *
sbZfSbQe2000ElibPriTableAddr_SPrint(sbZfSbQe2000ElibPriTableAddr_t *pFromStruct, char *pcToString, uint32 lStrSize) {
  uint32 WrCnt = 0x0;

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"SbQe2000ElibPriTableAddr:: port=0x%02x", (unsigned int)  pFromStruct->m_nPort);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," cos=0x%01x", (unsigned int)  pFromStruct->m_nCos);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," dp=0x%01x", (unsigned int)  pFromStruct->m_nDp);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," ecn=0x%01x", (unsigned int)  pFromStruct->m_nEcn);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  /* assert if we've overrun the buffer */
  SB_ASSERT(WrCnt < lStrSize);
  return(pcToString);

}

/* validate members in struct (1 = PASS, 0 = FAIL) */
int
sbZfSbQe2000ElibPriTableAddr_Validate(sbZfSbQe2000ElibPriTableAddr_t *pZf) {

  if (pZf->m_nPort > 0x3f) return 0;
  if (pZf->m_nCos > 0x7) return 0;
  if (pZf->m_nDp > 0x3) return 0;
  if (pZf->m_nEcn > 0x1) return 0;

  return 1; /* success */

}

/* populate field from string and value */
int
sbZfSbQe2000ElibPriTableAddr_SetField(sbZfSbQe2000ElibPriTableAddr_t *s, char* name, int value) {

  if (SB_STRCMP(name, "") == 0 ) {
    return -1;
  } else if (SB_STRCMP(name, "m_nport") == 0) {
    s->m_nPort = value;
  } else if (SB_STRCMP(name, "m_ncos") == 0) {
    s->m_nCos = value;
  } else if (SB_STRCMP(name, "m_ndp") == 0) {
    s->m_nDp = value;
  } else if (SB_STRCMP(name, "m_necn") == 0) {
    s->m_nEcn = value;
  } else {
    /* string failed to match any field--ignored */
    return -1;
  }

  return(0);

}
