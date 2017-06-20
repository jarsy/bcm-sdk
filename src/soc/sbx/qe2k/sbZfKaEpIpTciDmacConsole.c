/*
 * $Id: sbZfKaEpIpTciDmacConsole.c,v 1.3 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include <shared/bsl.h>

#include "sbTypes.h"
#include <soc/sbx/sbWrappers.h>
#include "sbZfKaEpIpTciDmacConsole.hx"



/* Print members in struct */
void
sbZfKaEpIpTciDmac_Print(sbZfKaEpIpTciDmac_t *pFromStruct) {
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("KaEpIpTciDmac:: pri=0x%01x"), (unsigned int)  pFromStruct->m_nPri));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" cfi=0x%01x"), (unsigned int)  pFromStruct->m_nCfi));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" vid=0x%03x"), (unsigned int)  pFromStruct->m_nVid));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" dmac=0x%04x%08x"),  COMPILER_64_HI(pFromStruct->m_nnDmac), COMPILER_64_LO(pFromStruct->m_nnDmac)));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

}

/* SPrint members in struct */
char *
sbZfKaEpIpTciDmac_SPrint(sbZfKaEpIpTciDmac_t *pFromStruct, char *pcToString, uint32 lStrSize) {
  uint32 WrCnt = 0x0;

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"KaEpIpTciDmac:: pri=0x%01x", (unsigned int)  pFromStruct->m_nPri);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," cfi=0x%01x", (unsigned int)  pFromStruct->m_nCfi);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," vid=0x%03x", (unsigned int)  pFromStruct->m_nVid);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," dmac=0x%04x%08x",  COMPILER_64_HI(pFromStruct->m_nnDmac), COMPILER_64_LO(pFromStruct->m_nnDmac));
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  /* assert if we've overrun the buffer */
  SB_ASSERT(WrCnt < lStrSize);
  return(pcToString);

}

/* validate members in struct (1 = PASS, 0 = FAIL) */
int
sbZfKaEpIpTciDmac_Validate(sbZfKaEpIpTciDmac_t *pZf) {
  uint64 nnDmacMax = COMPILER_64_INIT(0xFFFF, 0xFFFFFFFF);
  if (pZf->m_nPri > 0x7) return 0;
  if (pZf->m_nCfi > 0x1) return 0;
  if (pZf->m_nVid > 0xfff) return 0;
  if (COMPILER_64_GT(pZf->m_nnDmac,nnDmacMax)) return 0;

  return 1; /* success */

}

/* populate field from string and value */
int
sbZfKaEpIpTciDmac_SetField(sbZfKaEpIpTciDmac_t *s, char* name, int value) {

  if (SB_STRCMP(name, "") == 0 ) {
    return -1;
  } else if (SB_STRCMP(name, "m_npri") == 0) {
    s->m_nPri = value;
  } else if (SB_STRCMP(name, "m_ncfi") == 0) {
    s->m_nCfi = value;
  } else if (SB_STRCMP(name, "m_nvid") == 0) {
    s->m_nVid = value;
  } else if (SB_STRCMP(name, "m_nndmac") == 0) {
    COMPILER_64_SET(s->m_nnDmac,0,value);
  } else {
    /* string failed to match any field--ignored */
    return -1;
  }

  return(0);

}
