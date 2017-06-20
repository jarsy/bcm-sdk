/*
 * $Id: sbZfKaEbMvtAddressConsole.c,v 1.3 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include <shared/bsl.h>

#include "sbTypes.h"
#include <soc/sbx/sbWrappers.h>
#include "sbZfKaEbMvtAddressConsole.hx"



/* Print members in struct */
void
sbZfKaEbMvtAddress_Print(sbZfKaEbMvtAddress_t *pFromStruct) {
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("KaEbMvtAddress:: reserved=0x%03x"), (unsigned int)  pFromStruct->m_nReserved));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" egress=0x%02x"), (unsigned int)  pFromStruct->m_nEgress));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" offset=0x%04x"), (unsigned int)  pFromStruct->m_nOffset));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

}

/* SPrint members in struct */
char *
sbZfKaEbMvtAddress_SPrint(sbZfKaEbMvtAddress_t *pFromStruct, char *pcToString, uint32 lStrSize) {
  uint32 WrCnt = 0x0;

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"KaEbMvtAddress:: reserved=0x%03x", (unsigned int)  pFromStruct->m_nReserved);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," egress=0x%02x", (unsigned int)  pFromStruct->m_nEgress);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," offset=0x%04x", (unsigned int)  pFromStruct->m_nOffset);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  /* assert if we've overrun the buffer */
  SB_ASSERT(WrCnt < lStrSize);
  return(pcToString);

}

/* validate members in struct (1 = PASS, 0 = FAIL) */
int
sbZfKaEbMvtAddress_Validate(sbZfKaEbMvtAddress_t *pZf) {

  if (pZf->m_nReserved > 0x3ff) return 0;
  if (pZf->m_nEgress > 0x3f) return 0;
  if (pZf->m_nOffset > 0xffff) return 0;

  return 1; /* success */

}

/* populate field from string and value */
int
sbZfKaEbMvtAddress_SetField(sbZfKaEbMvtAddress_t *s, char* name, int value) {

  if (SB_STRCMP(name, "") == 0 ) {
    return -1;
  } else if (SB_STRCMP(name, "m_nreserved") == 0) {
    s->m_nReserved = value;
  } else if (SB_STRCMP(name, "m_negress") == 0) {
    s->m_nEgress = value;
  } else if (SB_STRCMP(name, "m_noffset") == 0) {
    s->m_nOffset = value;
  } else {
    /* string failed to match any field--ignored */
    return -1;
  }

  return(0);

}
