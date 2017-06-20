/*
 * $Id: sbZfKaEpIpV6TciConsole.c,v 1.3 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include <shared/bsl.h>

#include "sbTypes.h"
#include <soc/sbx/sbWrappers.h>
#include "sbZfKaEpIpV6TciConsole.hx"



/* Print members in struct */
void
sbZfKaEpIpV6Tci_Print(sbZfKaEpIpV6Tci_t *pFromStruct) {
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("KaEpIpV6Tci:: rsvd=0x%04x%08x"),   COMPILER_64_HI(pFromStruct->m_nReserved), COMPILER_64_LO(pFromStruct->m_nReserved)));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" pri=0x%01x"), (unsigned int)  pFromStruct->m_nPri));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" cfi=0x%01x"), (unsigned int)  pFromStruct->m_nCfi));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" vid=0x%03x"), (unsigned int)  pFromStruct->m_nVid));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

}

/* SPrint members in struct */
char *
sbZfKaEpIpV6Tci_SPrint(sbZfKaEpIpV6Tci_t *pFromStruct, char *pcToString, uint32 lStrSize) {
  uint32 WrCnt = 0x0;

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"KaEpIpV6Tci:: rsvd=0x%04x%08x",   COMPILER_64_HI(pFromStruct->m_nReserved), COMPILER_64_LO(pFromStruct->m_nReserved));
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pri=0x%01x", (unsigned int)  pFromStruct->m_nPri);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," cfi=0x%01x", (unsigned int)  pFromStruct->m_nCfi);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," vid=0x%03x", (unsigned int)  pFromStruct->m_nVid);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  /* assert if we've overrun the buffer */
  SB_ASSERT(WrCnt < lStrSize);
  return(pcToString);

}

/* validate members in struct (1 = PASS, 0 = FAIL) */
int
sbZfKaEpIpV6Tci_Validate(sbZfKaEpIpV6Tci_t *pZf) {
  uint64 nReservedMax = COMPILER_64_INIT(0xFFFF, 0xFFFFFFFF);
  if (COMPILER_64_GT(pZf->m_nReserved ,nReservedMax)) return 0;
  if (pZf->m_nPri > 0x7) return 0;
  if (pZf->m_nCfi > 0x1) return 0;
  if (pZf->m_nVid > 0xfff) return 0;

  return 1; /* success */

}

/* populate field from string and value */
int
sbZfKaEpIpV6Tci_SetField(sbZfKaEpIpV6Tci_t *s, char* name, int value) {

  if (SB_STRCMP(name, "") == 0 ) {
    return -1;
  } else if (SB_STRCMP(name, "m_nreserved") == 0) {
    COMPILER_64_SET(s->m_nReserved,0,value);
  } else if (SB_STRCMP(name, "m_npri") == 0) {
    s->m_nPri = value;
  } else if (SB_STRCMP(name, "m_ncfi") == 0) {
    s->m_nCfi = value;
  } else if (SB_STRCMP(name, "m_nvid") == 0) {
    s->m_nVid = value;
  } else {
    /* string failed to match any field--ignored */
    return -1;
  }

  return(0);

}
