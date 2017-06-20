/*
 * $Id: sbZfG2EplibIpSegmentConsole.c,v 1.8 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include <shared/bsl.h>

#include "sbTypesGlue.h"
#include <soc/sbx/sbWrappers.h>
#include "sbZfG2EplibIpSegmentConsole.hx"



/* Print members in struct */
void
sbZfG2EplibIpSegment_Print(sbZfG2EplibIpSegment_t *pFromStruct) {
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("G2EplibIpSegment:: start=0x%01x"), (unsigned int)  pFromStruct->start));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" end=0x%01x"), (unsigned int)  pFromStruct->end));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" width=0x%01x"), (unsigned int)  pFromStruct->width));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" entsize=0x%01x"), (unsigned int)  pFromStruct->entrysize));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

}

/* SPrint members in struct */
char *
sbZfG2EplibIpSegment_SPrint(sbZfG2EplibIpSegment_t *pFromStruct, char *pcToString, uint32 lStrSize) {
  uint32 WrCnt = 0x0;

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"G2EplibIpSegment:: start=0x%01x", (unsigned int)  pFromStruct->start);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," end=0x%01x", (unsigned int)  pFromStruct->end);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," width=0x%01x", (unsigned int)  pFromStruct->width);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," entsize=0x%01x", (unsigned int)  pFromStruct->entrysize);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  /* assert if we've overrun the buffer */
  SB_ASSERT(WrCnt < lStrSize);
  return(pcToString);

}

/* validate members in struct (1 = PASS, 0 = FAIL) */
int
sbZfG2EplibIpSegment_Validate(sbZfG2EplibIpSegment_t *pZf) {

  if (pZf->start > 0x1) return 0;
  if (pZf->end > 0x1) return 0;
  if (pZf->width > 0x1) return 0;
  if (pZf->entrysize > 0x1) return 0;

  return 1; /* success */

}

/* populate field from string and value */
int
sbZfG2EplibIpSegment_SetField(sbZfG2EplibIpSegment_t *s, char* name, int value) {

  if (SB_STRCMP(name, "") == 0 ) {
    return -1;
  } else if (SB_STRCMP(name, "start") == 0) {
    s->start = value;
  } else if (SB_STRCMP(name, "end") == 0) {
    s->end = value;
  } else if (SB_STRCMP(name, "width") == 0) {
    s->width = value;
  } else if (SB_STRCMP(name, "entrysize") == 0) {
    s->entrysize = value;
  } else {
    /* string failed to match any field--ignored */
    return -1;
  }

  return(0);

}
