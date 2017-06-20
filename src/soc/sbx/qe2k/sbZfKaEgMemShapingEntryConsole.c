/*
 * $Id: sbZfKaEgMemShapingEntryConsole.c,v 1.3 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include <shared/bsl.h>

#include "sbTypes.h"
#include <soc/sbx/sbWrappers.h>
#include "sbZfKaEgMemShapingEntryConsole.hx"



/* Print members in struct */
void
sbZfKaEgMemShapingEntry_Print(sbZfKaEgMemShapingEntry_t *pFromStruct) {
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("KaEgMemShapingEntry:: reserved=0x%01x"), (unsigned int)  pFromStruct->m_nReserved));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" bucketdepth=0x%06x"), (unsigned int)  pFromStruct->m_nBucketDepth));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" shaperate=0x%06x"), (unsigned int)  pFromStruct->m_nShapeRate));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("KaEgMemShapingEntry:: maxdepth=0x%04x"), (unsigned int)  pFromStruct->m_nMaxDepth));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" port=0x%02x"), (unsigned int)  pFromStruct->m_nPort));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" hiside=0x%01x"), (unsigned int)  pFromStruct->m_nHiSide));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" shapesrc=0x%02x"), (unsigned int)  pFromStruct->m_nShapeSrc));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("KaEgMemShapingEntry:: enable=0x%01x"), (unsigned int)  pFromStruct->m_nEnable));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

}

/* SPrint members in struct */
char *
sbZfKaEgMemShapingEntry_SPrint(sbZfKaEgMemShapingEntry_t *pFromStruct, char *pcToString, uint32 lStrSize) {
  uint32 WrCnt = 0x0;

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"KaEgMemShapingEntry:: reserved=0x%01x", (unsigned int)  pFromStruct->m_nReserved);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," bucketdepth=0x%06x", (unsigned int)  pFromStruct->m_nBucketDepth);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," shaperate=0x%06x", (unsigned int)  pFromStruct->m_nShapeRate);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"KaEgMemShapingEntry:: maxdepth=0x%04x", (unsigned int)  pFromStruct->m_nMaxDepth);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," port=0x%02x", (unsigned int)  pFromStruct->m_nPort);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," hiside=0x%01x", (unsigned int)  pFromStruct->m_nHiSide);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," shapesrc=0x%02x", (unsigned int)  pFromStruct->m_nShapeSrc);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"KaEgMemShapingEntry:: enable=0x%01x", (unsigned int)  pFromStruct->m_nEnable);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  /* assert if we've overrun the buffer */
  SB_ASSERT(WrCnt < lStrSize);
  return(pcToString);

}

/* validate members in struct (1 = PASS, 0 = FAIL) */
int
sbZfKaEgMemShapingEntry_Validate(sbZfKaEgMemShapingEntry_t *pZf) {

  if (pZf->m_nReserved > 0x7) return 0;
  if (pZf->m_nBucketDepth > 0xffffff) return 0;
  if (pZf->m_nShapeRate > 0xffffff) return 0;
  if (pZf->m_nMaxDepth > 0x7fff) return 0;
  if (pZf->m_nPort > 0x3f) return 0;
  if (pZf->m_nHiSide > 0x1) return 0;
  if (pZf->m_nShapeSrc > 0x3f) return 0;
  if (pZf->m_nEnable > 0x1) return 0;

  return 1; /* success */

}

/* populate field from string and value */
int
sbZfKaEgMemShapingEntry_SetField(sbZfKaEgMemShapingEntry_t *s, char* name, int value) {

  if (SB_STRCMP(name, "") == 0 ) {
    return -1;
  } else if (SB_STRCMP(name, "m_nreserved") == 0) {
    s->m_nReserved = value;
  } else if (SB_STRCMP(name, "m_nbucketdepth") == 0) {
    s->m_nBucketDepth = value;
  } else if (SB_STRCMP(name, "m_nshaperate") == 0) {
    s->m_nShapeRate = value;
  } else if (SB_STRCMP(name, "m_nmaxdepth") == 0) {
    s->m_nMaxDepth = value;
  } else if (SB_STRCMP(name, "m_nport") == 0) {
    s->m_nPort = value;
  } else if (SB_STRCMP(name, "m_nhiside") == 0) {
    s->m_nHiSide = value;
  } else if (SB_STRCMP(name, "m_nshapesrc") == 0) {
    s->m_nShapeSrc = value;
  } else if (SB_STRCMP(name, "m_nenable") == 0) {
    s->m_nEnable = value;
  } else {
    /* string failed to match any field--ignored */
    return -1;
  }

  return(0);

}
