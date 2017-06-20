/*
 * $Id: $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include <shared/bsl.h>

#include "sbTypes.h"
#include <soc/sbx/sbWrappers.h>
#include "sbZfFabBm9600InaSysportMapEntryConsole.hx"



/* Print members in struct */
void
sbZfFabBm9600InaSysportMapEntry_Print(sbZfFabBm9600InaSysportMapEntry_t *pFromStruct) {
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("FabBm9600InaSysportMapEntry:: portsetaddr=0x%02x"), (unsigned int)  pFromStruct->m_uPortsetAddr));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" offset=0x%01x"), (unsigned int)  pFromStruct->m_uOffset));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

}

/* SPrint members in struct */
char *
sbZfFabBm9600InaSysportMapEntry_SPrint(sbZfFabBm9600InaSysportMapEntry_t *pFromStruct, char *pcToString, uint32 lStrSize) {
  uint32 WrCnt = 0x0;

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"FabBm9600InaSysportMapEntry:: portsetaddr=0x%02x", (unsigned int)  pFromStruct->m_uPortsetAddr);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," offset=0x%01x", (unsigned int)  pFromStruct->m_uOffset);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  /* assert if we've overrun the buffer */
  SB_ASSERT(WrCnt < lStrSize);
  return(pcToString);

}

/* validate members in struct (1 = PASS, 0 = FAIL) */
int
sbZfFabBm9600InaSysportMapEntry_Validate(sbZfFabBm9600InaSysportMapEntry_t *pZf) {

  if (pZf->m_uPortsetAddr > 0xff) return 0;
  if (pZf->m_uOffset > 0xf) return 0;

  return 1; /* success */

}

/* populate field from string and value */
int
sbZfFabBm9600InaSysportMapEntry_SetField(sbZfFabBm9600InaSysportMapEntry_t *s, char* name, int value) {

  if (SB_STRCMP(name, "") == 0 ) {
    return -1;
  } else if (SB_STRCMP(name, "m_uportsetaddr") == 0) {
    s->m_uPortsetAddr = value;
  } else if (SB_STRCMP(name, "m_uoffset") == 0) {
    s->m_uOffset = value;
  } else {
    /* string failed to match any field--ignored */
    return -1;
  }

  return(0);

}
