/*
 * $Id: sbZfKaEgMemQCtlEntryConsole.c,v 1.3 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include <shared/bsl.h>

#include "sbTypes.h"
#include <soc/sbx/sbWrappers.h>
#include "sbZfKaEgMemQCtlEntryConsole.hx"



/* Print members in struct */
void
sbZfKaEgMemQCtlEntry_Print(sbZfKaEgMemQCtlEntry_t *pFromStruct) {
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("KaEgMemQCtlEntry:: droponfull=0x%01x"), (unsigned int)  pFromStruct->m_nDropOnFull));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" wptr=0x%02x"), (unsigned int)  pFromStruct->m_nWptr));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" rptr=0x%02x"), (unsigned int)  pFromStruct->m_nRptr));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" ssize=0x%01x"), (unsigned int)  pFromStruct->m_nSize));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" base=0x%03x"), (unsigned int)  pFromStruct->m_nBase));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

}

/* SPrint members in struct */
char *
sbZfKaEgMemQCtlEntry_SPrint(sbZfKaEgMemQCtlEntry_t *pFromStruct, char *pcToString, uint32 lStrSize) {
  uint32 WrCnt = 0x0;

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"KaEgMemQCtlEntry:: droponfull=0x%01x", (unsigned int)  pFromStruct->m_nDropOnFull);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," wptr=0x%02x", (unsigned int)  pFromStruct->m_nWptr);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," rptr=0x%02x", (unsigned int)  pFromStruct->m_nRptr);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," ssize=0x%01x", (unsigned int)  pFromStruct->m_nSize);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," base=0x%03x", (unsigned int)  pFromStruct->m_nBase);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  /* assert if we've overrun the buffer */
  SB_ASSERT(WrCnt < lStrSize);
  return(pcToString);

}

/* validate members in struct (1 = PASS, 0 = FAIL) */
int
sbZfKaEgMemQCtlEntry_Validate(sbZfKaEgMemQCtlEntry_t *pZf) {

  if (pZf->m_nDropOnFull > 0x1) return 0;
  if (pZf->m_nWptr > 0x3f) return 0;
  if (pZf->m_nRptr > 0x3f) return 0;
  if (pZf->m_nSize > 0x7) return 0;
  if (pZf->m_nBase > 0x3ff) return 0;

  return 1; /* success */

}

/* populate field from string and value */
int
sbZfKaEgMemQCtlEntry_SetField(sbZfKaEgMemQCtlEntry_t *s, char* name, int value) {

  if (SB_STRCMP(name, "") == 0 ) {
    return -1;
  } else if (SB_STRCMP(name, "m_ndroponfull") == 0) {
    s->m_nDropOnFull = value;
  } else if (SB_STRCMP(name, "m_nwptr") == 0) {
    s->m_nWptr = value;
  } else if (SB_STRCMP(name, "m_nrptr") == 0) {
    s->m_nRptr = value;
  } else if (SB_STRCMP(name, "m_nsize") == 0) {
    s->m_nSize = value;
  } else if (SB_STRCMP(name, "m_nbase") == 0) {
    s->m_nBase = value;
  } else {
    /* string failed to match any field--ignored */
    return -1;
  }

  return(0);

}
