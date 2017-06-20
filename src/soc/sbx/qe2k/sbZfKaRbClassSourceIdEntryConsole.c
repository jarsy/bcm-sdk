/*
 * $Id: sbZfKaRbClassSourceIdEntryConsole.c,v 1.3 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include <shared/bsl.h>

#include "sbTypes.h"
#include <soc/sbx/sbWrappers.h>
#include "sbZfKaRbClassSourceIdEntryConsole.hx"



/* Print members in struct */
void
sbZfKaRbClassSourceIdEntry_Print(sbZfKaRbClassSourceIdEntry_t *pFromStruct) {
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("KaRbClassSourceIdEntry:: res2=0x%01x"), (unsigned int)  pFromStruct->m_nReserved2));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" srcidodd=0x%03x"), (unsigned int)  pFromStruct->m_nSrcIdOdd));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" res1=0x%01x"), (unsigned int)  pFromStruct->m_nReserved1));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" srcideven=0x%03x"), (unsigned int)  pFromStruct->m_nSrcIdEven));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

}

/* SPrint members in struct */
char *
sbZfKaRbClassSourceIdEntry_SPrint(sbZfKaRbClassSourceIdEntry_t *pFromStruct, char *pcToString, uint32 lStrSize) {
  uint32 WrCnt = 0x0;

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"KaRbClassSourceIdEntry:: res2=0x%01x", (unsigned int)  pFromStruct->m_nReserved2);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," srcidodd=0x%03x", (unsigned int)  pFromStruct->m_nSrcIdOdd);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," res1=0x%01x", (unsigned int)  pFromStruct->m_nReserved1);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," srcideven=0x%03x", (unsigned int)  pFromStruct->m_nSrcIdEven);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  /* assert if we've overrun the buffer */
  SB_ASSERT(WrCnt < lStrSize);
  return(pcToString);

}

/* validate members in struct (1 = PASS, 0 = FAIL) */
int
sbZfKaRbClassSourceIdEntry_Validate(sbZfKaRbClassSourceIdEntry_t *pZf) {

  if (pZf->m_nReserved2 > 0xf) return 0;
  if (pZf->m_nSrcIdOdd > 0xfff) return 0;
  if (pZf->m_nReserved1 > 0xf) return 0;
  if (pZf->m_nSrcIdEven > 0xfff) return 0;

  return 1; /* success */

}

/* populate field from string and value */
int
sbZfKaRbClassSourceIdEntry_SetField(sbZfKaRbClassSourceIdEntry_t *s, char* name, int value) {

  if (SB_STRCMP(name, "") == 0 ) {
    return -1;
  } else if (SB_STRCMP(name, "m_nreserved2") == 0) {
    s->m_nReserved2 = value;
  } else if (SB_STRCMP(name, "m_nsrcidodd") == 0) {
    s->m_nSrcIdOdd = value;
  } else if (SB_STRCMP(name, "m_nreserved1") == 0) {
    s->m_nReserved1 = value;
  } else if (SB_STRCMP(name, "m_nsrcideven") == 0) {
    s->m_nSrcIdEven = value;
  } else {
    /* string failed to match any field--ignored */
    return -1;
  }

  return(0);

}
