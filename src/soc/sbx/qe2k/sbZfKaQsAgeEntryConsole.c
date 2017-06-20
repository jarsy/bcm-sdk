/*
 * $Id: sbZfKaQsAgeEntryConsole.c,v 1.3 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include <shared/bsl.h>

#include "sbTypes.h"
#include <soc/sbx/sbWrappers.h>
#include "sbZfKaQsAgeEntryConsole.hx"



/* Print members in struct */
void
sbZfKaQsAgeEntry_Print(sbZfKaQsAgeEntry_t *pFromStruct) {
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("KaQsAgeEntry:: res=0x%06x"), (unsigned int)  pFromStruct->m_nReserved));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" noempty=0x%01x"), (unsigned int)  pFromStruct->m_nNoEmpty));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" anemicevent=0x%01x"), (unsigned int)  pFromStruct->m_nAnemicEvent));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" efevent=0x%01x"), (unsigned int)  pFromStruct->m_nEfEvent));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("KaQsAgeEntry:: cnt=0x%02x"), (unsigned int)  pFromStruct->m_nCnt));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

}

/* SPrint members in struct */
char *
sbZfKaQsAgeEntry_SPrint(sbZfKaQsAgeEntry_t *pFromStruct, char *pcToString, uint32 lStrSize) {
  uint32 WrCnt = 0x0;

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"KaQsAgeEntry:: res=0x%06x", (unsigned int)  pFromStruct->m_nReserved);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," noempty=0x%01x", (unsigned int)  pFromStruct->m_nNoEmpty);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," anemicevent=0x%01x", (unsigned int)  pFromStruct->m_nAnemicEvent);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," efevent=0x%01x", (unsigned int)  pFromStruct->m_nEfEvent);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"KaQsAgeEntry:: cnt=0x%02x", (unsigned int)  pFromStruct->m_nCnt);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  /* assert if we've overrun the buffer */
  SB_ASSERT(WrCnt < lStrSize);
  return(pcToString);

}

/* validate members in struct (1 = PASS, 0 = FAIL) */
int
sbZfKaQsAgeEntry_Validate(sbZfKaQsAgeEntry_t *pZf) {

  if (pZf->m_nReserved > 0x1fffff) return 0;
  if (pZf->m_nNoEmpty > 0x1) return 0;
  if (pZf->m_nAnemicEvent > 0x1) return 0;
  if (pZf->m_nEfEvent > 0x1) return 0;
  if (pZf->m_nCnt > 0xff) return 0;

  return 1; /* success */

}

/* populate field from string and value */
int
sbZfKaQsAgeEntry_SetField(sbZfKaQsAgeEntry_t *s, char* name, int value) {

  if (SB_STRCMP(name, "") == 0 ) {
    return -1;
  } else if (SB_STRCMP(name, "m_nreserved") == 0) {
    s->m_nReserved = value;
  } else if (SB_STRCMP(name, "m_nnoempty") == 0) {
    s->m_nNoEmpty = value;
  } else if (SB_STRCMP(name, "m_nanemicevent") == 0) {
    s->m_nAnemicEvent = value;
  } else if (SB_STRCMP(name, "m_nefevent") == 0) {
    s->m_nEfEvent = value;
  } else if (SB_STRCMP(name, "m_ncnt") == 0) {
    s->m_nCnt = value;
  } else {
    /* string failed to match any field--ignored */
    return -1;
  }

  return(0);

}
