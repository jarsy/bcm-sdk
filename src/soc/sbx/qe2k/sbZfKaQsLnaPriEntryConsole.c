/*
 * $Id: sbZfKaQsLnaPriEntryConsole.c,v 1.4 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include <shared/bsl.h>

#include "sbTypes.h"
#include <soc/sbx/sbWrappers.h>
#include "sbZfKaQsLnaPriEntryConsole.hx"



/* Print members in struct */
void
sbZfKaQsLnaPriEntry_Print(sbZfKaQsLnaPriEntry_t *pFromStruct) {
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("KaQsLnaPriEntry:: pri4=0x%08x"), (unsigned int)  pFromStruct->m_nPri4));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" pri3=0x%08x"), (unsigned int)  pFromStruct->m_nPri3));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" pri2=0x%08x"), (unsigned int)  pFromStruct->m_nPri2));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("KaQsLnaPriEntry:: pri1=0x%08x"), (unsigned int)  pFromStruct->m_nPri1));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" pri0=0x%08x"), (unsigned int)  pFromStruct->m_nPri0));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

}

/* SPrint members in struct */
char *
sbZfKaQsLnaPriEntry_SPrint(sbZfKaQsLnaPriEntry_t *pFromStruct, char *pcToString, uint32 lStrSize) {
  uint32 WrCnt = 0x0;

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"KaQsLnaPriEntry:: pri4=0x%08x", (unsigned int)  pFromStruct->m_nPri4);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pri3=0x%08x", (unsigned int)  pFromStruct->m_nPri3);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pri2=0x%08x", (unsigned int)  pFromStruct->m_nPri2);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"KaQsLnaPriEntry:: pri1=0x%08x", (unsigned int)  pFromStruct->m_nPri1);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pri0=0x%08x", (unsigned int)  pFromStruct->m_nPri0);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  /* assert if we've overrun the buffer */
  SB_ASSERT(WrCnt < lStrSize);
  return(pcToString);

}

/* validate members in struct (1 = PASS, 0 = FAIL) */
int
sbZfKaQsLnaPriEntry_Validate(sbZfKaQsLnaPriEntry_t *pZf) {

  /* pZf->m_nPri4 implicitly masked by data type */
  /* pZf->m_nPri3 implicitly masked by data type */
  /* pZf->m_nPri2 implicitly masked by data type */
  /* pZf->m_nPri1 implicitly masked by data type */
  /* pZf->m_nPri0 implicitly masked by data type */

  return 1; /* success */

}

/* populate field from string and value */
int
sbZfKaQsLnaPriEntry_SetField(sbZfKaQsLnaPriEntry_t *s, char* name, int value) {

  if (SB_STRCMP(name, "") == 0 ) {
    return -1;
  } else if (SB_STRCMP(name, "m_npri4") == 0) {
    s->m_nPri4 = value;
  } else if (SB_STRCMP(name, "m_npri3") == 0) {
    s->m_nPri3 = value;
  } else if (SB_STRCMP(name, "m_npri2") == 0) {
    s->m_nPri2 = value;
  } else if (SB_STRCMP(name, "m_npri1") == 0) {
    s->m_nPri1 = value;
  } else if (SB_STRCMP(name, "m_npri0") == 0) {
    s->m_nPri0 = value;
  } else {
    /* string failed to match any field--ignored */
    return -1;
  }

  return(0);

}
