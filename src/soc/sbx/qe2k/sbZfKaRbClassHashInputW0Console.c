/*
 * $Id: sbZfKaRbClassHashInputW0Console.c,v 1.3 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include <shared/bsl.h>

#include "sbTypes.h"
#include <soc/sbx/sbWrappers.h>
#include "sbZfKaRbClassHashInputW0Console.hx"



/* Print members in struct */
void
sbZfKaRbClassHashInputW0_Print(sbZfKaRbClassHashInputW0_t *pFromStruct) {
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("KaRbClassHashInputW0:: dmac=0x%04x%08x"),   COMPILER_64_HI(pFromStruct->m_nnDmac), COMPILER_64_LO(pFromStruct->m_nnDmac)));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" smac=0x%04x%08x"),   COMPILER_64_HI(pFromStruct->m_nnSmac), COMPILER_64_LO(pFromStruct->m_nnSmac)));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" sp1=0x%04x"), (unsigned int)  pFromStruct->m_nSpare1));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("KaRbClassHashInputW0:: iport=0x%02x"), (unsigned int)  pFromStruct->m_nIPort));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" vlanid=0x%03x"), (unsigned int)  pFromStruct->m_nVlanId));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

}

/* SPrint members in struct */
char *
sbZfKaRbClassHashInputW0_SPrint(sbZfKaRbClassHashInputW0_t *pFromStruct, char *pcToString, uint32 lStrSize) {
  uint32 WrCnt = 0x0;

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"KaRbClassHashInputW0:: dmac=0x%04x%08x",   COMPILER_64_HI(pFromStruct->m_nnDmac), COMPILER_64_LO(pFromStruct->m_nnDmac));
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," smac=0x%04x%08x",   COMPILER_64_HI(pFromStruct->m_nnSmac), COMPILER_64_LO(pFromStruct->m_nnSmac));
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," sp1=0x%04x", (unsigned int)  pFromStruct->m_nSpare1);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"KaRbClassHashInputW0:: iport=0x%02x", (unsigned int)  pFromStruct->m_nIPort);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," vlanid=0x%03x", (unsigned int)  pFromStruct->m_nVlanId);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  /* assert if we've overrun the buffer */
  SB_ASSERT(WrCnt < lStrSize);
  return(pcToString);

}

/* validate members in struct (1 = PASS, 0 = FAIL) */
int
sbZfKaRbClassHashInputW0_Validate(sbZfKaRbClassHashInputW0_t *pZf) {
  uint64 nnMacMax = COMPILER_64_INIT(0xFFFF, 0xFFFFFFFF);
  if (COMPILER_64_GT(pZf->m_nnDmac, nnMacMax)) return 0;
  if (COMPILER_64_GT(pZf->m_nnSmac, nnMacMax)) return 0;
  if (pZf->m_nSpare1 > 0x3fff) return 0;
  if (pZf->m_nIPort > 0x3f) return 0;
  if (pZf->m_nVlanId > 0xfff) return 0;

  return 1; /* success */

}

/* populate field from string and value */
int
sbZfKaRbClassHashInputW0_SetField(sbZfKaRbClassHashInputW0_t *s, char* name, int value) {

  if (SB_STRCMP(name, "") == 0 ) {
    return -1;
  } else if (SB_STRCMP(name, "m_nndmac") == 0) {
    COMPILER_64_SET(s->m_nnDmac,0,value);
  } else if (SB_STRCMP(name, "m_nnsmac") == 0) {
    COMPILER_64_SET(s->m_nnSmac,0,value);
  } else if (SB_STRCMP(name, "m_nspare1") == 0) {
    s->m_nSpare1 = value;
  } else if (SB_STRCMP(name, "m_niport") == 0) {
    s->m_nIPort = value;
  } else if (SB_STRCMP(name, "m_nvlanid") == 0) {
    s->m_nVlanId = value;
  } else {
    /* string failed to match any field--ignored */
    return -1;
  }

  return(0);

}
