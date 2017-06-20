/*
 * $Id: sbZfKaQsShapeTableEntryConsole.c,v 1.3 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include <shared/bsl.h>

#include "sbTypes.h"
#include <soc/sbx/sbWrappers.h>
#include "sbZfKaQsShapeTableEntryConsole.hx"



/* Print members in struct */
void
sbZfKaQsShapeTableEntry_Print(sbZfKaQsShapeTableEntry_t *pFromStruct) {
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("KaQsShapeTableEntry:: shaperatels2b=0x%04x"), (unsigned int)  pFromStruct->m_nShapeRateLS2B));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" enable=0x%01x"), (unsigned int)  pFromStruct->m_nEnable));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("KaQsShapeTableEntry:: shapemaxburst=0x%06x"), (unsigned int)  pFromStruct->m_nShapeMaxBurst));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" shape=0x%06x"), (unsigned int)  pFromStruct->m_nShape));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

}

/* SPrint members in struct */
char *
sbZfKaQsShapeTableEntry_SPrint(sbZfKaQsShapeTableEntry_t *pFromStruct, char *pcToString, uint32 lStrSize) {
  uint32 WrCnt = 0x0;

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"KaQsShapeTableEntry:: shaperatels2b=0x%04x", (unsigned int)  pFromStruct->m_nShapeRateLS2B);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," enable=0x%01x", (unsigned int)  pFromStruct->m_nEnable);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"KaQsShapeTableEntry:: shapemaxburst=0x%06x", (unsigned int)  pFromStruct->m_nShapeMaxBurst);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," shape=0x%06x", (unsigned int)  pFromStruct->m_nShape);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  /* assert if we've overrun the buffer */
  SB_ASSERT(WrCnt < lStrSize);
  return(pcToString);

}

/* validate members in struct (1 = PASS, 0 = FAIL) */
int
sbZfKaQsShapeTableEntry_Validate(sbZfKaQsShapeTableEntry_t *pZf) {

  if (pZf->m_nShapeRateLS2B > 0xffff) return 0;
  if (pZf->m_nEnable > 0x1) return 0;
  if (pZf->m_nShapeMaxBurst > 0x7fffff) return 0;
  if (pZf->m_nShape > 0xffffff) return 0;

  return 1; /* success */

}

/* populate field from string and value */
int
sbZfKaQsShapeTableEntry_SetField(sbZfKaQsShapeTableEntry_t *s, char* name, int value) {

  if (SB_STRCMP(name, "") == 0 ) {
    return -1;
  } else if (SB_STRCMP(name, "m_nshaperatels2b") == 0) {
    s->m_nShapeRateLS2B = value;
  } else if (SB_STRCMP(name, "m_nenable") == 0) {
    s->m_nEnable = value;
  } else if (SB_STRCMP(name, "m_nshapemaxburst") == 0) {
    s->m_nShapeMaxBurst = value;
  } else if (SB_STRCMP(name, "m_nshape") == 0) {
    s->m_nShape = value;
  } else {
    /* string failed to match any field--ignored */
    return -1;
  }

  return(0);

}
