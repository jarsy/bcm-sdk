/*
 * $Id: sbZfKaQsShapeMaxBurstEntryConsole.c,v 1.3 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include <shared/bsl.h>

#include "sbTypes.h"
#include <soc/sbx/sbWrappers.h>
#include "sbZfKaQsShapeMaxBurstEntryConsole.hx"



/* Print members in struct */
void
sbZfKaQsShapeMaxBurstEntry_Print(sbZfKaQsShapeMaxBurstEntry_t *pFromStruct) {
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("KaQsShapeMaxBurstEntry:: res=0x%02x"), (unsigned int)  pFromStruct->m_nReserved));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" enable=0x%01x"), (unsigned int)  pFromStruct->m_nEnable));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" shapemaxburst=0x%06x"), (unsigned int)  pFromStruct->m_nShapeMaxBurst));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

}

/* SPrint members in struct */
char *
sbZfKaQsShapeMaxBurstEntry_SPrint(sbZfKaQsShapeMaxBurstEntry_t *pFromStruct, char *pcToString, uint32 lStrSize) {
  uint32 WrCnt = 0x0;

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"KaQsShapeMaxBurstEntry:: res=0x%02x", (unsigned int)  pFromStruct->m_nReserved);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," enable=0x%01x", (unsigned int)  pFromStruct->m_nEnable);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," shapemaxburst=0x%06x", (unsigned int)  pFromStruct->m_nShapeMaxBurst);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  /* assert if we've overrun the buffer */
  SB_ASSERT(WrCnt < lStrSize);
  return(pcToString);

}

/* validate members in struct (1 = PASS, 0 = FAIL) */
int
sbZfKaQsShapeMaxBurstEntry_Validate(sbZfKaQsShapeMaxBurstEntry_t *pZf) {

  if (pZf->m_nReserved > 0xff) return 0;
  if (pZf->m_nEnable > 0x1) return 0;
  if (pZf->m_nShapeMaxBurst > 0x7fffff) return 0;

  return 1; /* success */

}

/* populate field from string and value */
int
sbZfKaQsShapeMaxBurstEntry_SetField(sbZfKaQsShapeMaxBurstEntry_t *s, char* name, int value) {

  if (SB_STRCMP(name, "") == 0 ) {
    return -1;
  } else if (SB_STRCMP(name, "m_nreserved") == 0) {
    s->m_nReserved = value;
  } else if (SB_STRCMP(name, "m_nenable") == 0) {
    s->m_nEnable = value;
  } else if (SB_STRCMP(name, "m_nshapemaxburst") == 0) {
    s->m_nShapeMaxBurst = value;
  } else {
    /* string failed to match any field--ignored */
    return -1;
  }

  return(0);

}
