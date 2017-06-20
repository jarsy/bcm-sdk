/*
 * $Id: sbZfKaQsDepthHplenEntryConsole.c,v 1.3 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include <shared/bsl.h>

#include "sbTypes.h"
#include <soc/sbx/sbWrappers.h>
#include "sbZfKaQsDepthHplenEntryConsole.hx"



/* Print members in struct */
void
sbZfKaQsDepthHplenEntry_Print(sbZfKaQsDepthHplenEntry_t *pFromStruct) {
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("KaQsDepthHplenEntry:: res=0x%07x"), (unsigned int)  pFromStruct->m_nReserved));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" hplen=0x%01x"), (unsigned int)  pFromStruct->m_nHplen));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" depth=0x%01x"), (unsigned int)  pFromStruct->m_nDepth));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

}

/* SPrint members in struct */
char *
sbZfKaQsDepthHplenEntry_SPrint(sbZfKaQsDepthHplenEntry_t *pFromStruct, char *pcToString, uint32 lStrSize) {
  uint32 WrCnt = 0x0;

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"KaQsDepthHplenEntry:: res=0x%07x", (unsigned int)  pFromStruct->m_nReserved);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," hplen=0x%01x", (unsigned int)  pFromStruct->m_nHplen);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," depth=0x%01x", (unsigned int)  pFromStruct->m_nDepth);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  /* assert if we've overrun the buffer */
  SB_ASSERT(WrCnt < lStrSize);
  return(pcToString);

}

/* validate members in struct (1 = PASS, 0 = FAIL) */
int
sbZfKaQsDepthHplenEntry_Validate(sbZfKaQsDepthHplenEntry_t *pZf) {

  if (pZf->m_nReserved > 0x3ffffff) return 0;
  if (pZf->m_nHplen > 0x3) return 0;
  if (pZf->m_nDepth > 0xf) return 0;

  return 1; /* success */

}

/* populate field from string and value */
int
sbZfKaQsDepthHplenEntry_SetField(sbZfKaQsDepthHplenEntry_t *s, char* name, int value) {

  if (SB_STRCMP(name, "") == 0 ) {
    return -1;
  } else if (SB_STRCMP(name, "m_nreserved") == 0) {
    s->m_nReserved = value;
  } else if (SB_STRCMP(name, "m_nhplen") == 0) {
    s->m_nHplen = value;
  } else if (SB_STRCMP(name, "m_ndepth") == 0) {
    s->m_nDepth = value;
  } else {
    /* string failed to match any field--ignored */
    return -1;
  }

  return(0);

}
