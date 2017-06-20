/*
 * $Id: sbZfG2EplibMvtEntryConsole.c,v 1.8 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include <shared/bsl.h>

#include "sbTypesGlue.h"
#include <soc/sbx/sbWrappers.h>
#include "sbZfG2EplibMvtEntryConsole.hx"



/* Print members in struct */
void
sbZfG2EplibMvtEntry_Print(sbZfG2EplibMvtEntry_t *pFromStruct) {
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("G2EplibMvtEntry:: portmask=0x%01x"), COMPILER_64_LO(pFromStruct->ullPortMask)));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" type=0x%01x"), (unsigned int)  pFromStruct->nType));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" mvtda=0x%01x"), (unsigned int)  pFromStruct->ulMvtdA));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" mvtdb=0x%01x"), (unsigned int)  pFromStruct->ulMvtdB));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("G2EplibMvtEntry:: sourceknockout=0x%01x"), (unsigned int)  pFromStruct->bSourceKnockout));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" enablechaining=0x%01x"), (unsigned int)  pFromStruct->bEnableChaining));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" nextmcgroup=0x%01x"), (unsigned int)  pFromStruct->ulNextMcGroup));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

}

/* SPrint members in struct */
char *
sbZfG2EplibMvtEntry_SPrint(sbZfG2EplibMvtEntry_t *pFromStruct, char *pcToString, uint32 lStrSize) {
  uint32 WrCnt = 0x0;

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"G2EplibMvtEntry:: portmask=0x%01x", COMPILER_64_LO(pFromStruct->ullPortMask));
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," type=0x%01x", (unsigned int)  pFromStruct->nType);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," mvtda=0x%01x", (unsigned int)  pFromStruct->ulMvtdA);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," mvtdb=0x%01x", (unsigned int)  pFromStruct->ulMvtdB);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"G2EplibMvtEntry:: sourceknockout=0x%01x", (unsigned int)  pFromStruct->bSourceKnockout);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," enablechaining=0x%01x", (unsigned int)  pFromStruct->bEnableChaining);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," nextmcgroup=0x%01x", (unsigned int)  pFromStruct->ulNextMcGroup);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  /* assert if we've overrun the buffer */
  SB_ASSERT(WrCnt < lStrSize);
  return(pcToString);

}

/* validate members in struct (1 = PASS, 0 = FAIL) */
int
sbZfG2EplibMvtEntry_Validate(sbZfG2EplibMvtEntry_t *pZf) {
  uint64 ullPortMask;
  COMPILER_64_SET(ullPortMask,0,1);
  if (COMPILER_64_GT(pZf->ullPortMask, ullPortMask)) return 0;
  if (pZf->nType > 0x1) return 0;
  if (pZf->ulMvtdA > 0x1) return 0;
  if (pZf->ulMvtdB > 0x1) return 0;
  if (pZf->bSourceKnockout > 0x1) return 0;
  if (pZf->bEnableChaining > 0x1) return 0;
  if (pZf->ulNextMcGroup > 0x1) return 0;

  return 1; /* success */

}

/* populate field from string and value */
int
sbZfG2EplibMvtEntry_SetField(sbZfG2EplibMvtEntry_t *s, char* name, int value) {

  if (SB_STRCMP(name, "") == 0 ) {
    return -1;
  } else if (SB_STRCMP(name, "lportmask") == 0) {
    COMPILER_64_SET(s->ullPortMask,0,value);
  } else if (SB_STRCMP(name, "ntype") == 0) {
    s->nType = value;
  } else if (SB_STRCMP(name, "mvtda") == 0) {
    s->ulMvtdA = value;
  } else if (SB_STRCMP(name, "mvtdb") == 0) {
    s->ulMvtdB = value;
  } else if (SB_STRCMP(name, "sourceknockout") == 0) {
    s->bSourceKnockout = value;
  } else if (SB_STRCMP(name, "enablechaining") == 0) {
    s->bEnableChaining = value;
  } else if (SB_STRCMP(name, "nextmcgroup") == 0) {
    s->ulNextMcGroup = value;
  } else {
    /* string failed to match any field--ignored */
    return -1;
  }

  return(0);

}
