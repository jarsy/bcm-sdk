/*
 * $Id: sbZfKaEgMemFifoControlEntryConsole.c,v 1.3 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include <shared/bsl.h>

#include "sbTypes.h"
#include <soc/sbx/sbWrappers.h>
#include "sbZfKaEgMemFifoControlEntryConsole.hx"



/* Print members in struct */
void
sbZfKaEgMemFifoControlEntry_Print(sbZfKaEgMemFifoControlEntry_t *pFromStruct) {
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("KaEgMemFifoControlEntry:: tailpage=0x%03x"), (unsigned int)  pFromStruct->m_nTailPage));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" tailoffset=0x%02x"), (unsigned int)  pFromStruct->m_nTailOffset));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" currdepth=0x%04x"), (unsigned int)  pFromStruct->m_nCurrDepth));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("KaEgMemFifoControlEntry:: headoffset=0x%02x"), (unsigned int)  pFromStruct->m_nHeadOffset));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

}

/* SPrint members in struct */
char *
sbZfKaEgMemFifoControlEntry_SPrint(sbZfKaEgMemFifoControlEntry_t *pFromStruct, char *pcToString, uint32 lStrSize) {
  uint32 WrCnt = 0x0;

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"KaEgMemFifoControlEntry:: tailpage=0x%03x", (unsigned int)  pFromStruct->m_nTailPage);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," tailoffset=0x%02x", (unsigned int)  pFromStruct->m_nTailOffset);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," currdepth=0x%04x", (unsigned int)  pFromStruct->m_nCurrDepth);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"KaEgMemFifoControlEntry:: headoffset=0x%02x", (unsigned int)  pFromStruct->m_nHeadOffset);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  /* assert if we've overrun the buffer */
  SB_ASSERT(WrCnt < lStrSize);
  return(pcToString);

}

/* validate members in struct (1 = PASS, 0 = FAIL) */
int
sbZfKaEgMemFifoControlEntry_Validate(sbZfKaEgMemFifoControlEntry_t *pZf) {

  if (pZf->m_nTailPage > 0x3ff) return 0;
  if (pZf->m_nTailOffset > 0x1f) return 0;
  if (pZf->m_nCurrDepth > 0xffff) return 0;
  if (pZf->m_nHeadOffset > 0x1f) return 0;

  return 1; /* success */

}

/* populate field from string and value */
int
sbZfKaEgMemFifoControlEntry_SetField(sbZfKaEgMemFifoControlEntry_t *s, char* name, int value) {

  if (SB_STRCMP(name, "") == 0 ) {
    return -1;
  } else if (SB_STRCMP(name, "m_ntailpage") == 0) {
    s->m_nTailPage = value;
  } else if (SB_STRCMP(name, "m_ntailoffset") == 0) {
    s->m_nTailOffset = value;
  } else if (SB_STRCMP(name, "m_ncurrdepth") == 0) {
    s->m_nCurrDepth = value;
  } else if (SB_STRCMP(name, "m_nheadoffset") == 0) {
    s->m_nHeadOffset = value;
  } else {
    /* string failed to match any field--ignored */
    return -1;
  }

  return(0);

}
