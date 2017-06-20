/*
 * $Id: $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include <shared/bsl.h>

#include "sbTypes.h"
#include <soc/sbx/sbWrappers.h>
#include "sbZfKaEpIpPortVridSmacTableEntryConsole.hx"



/* Print members in struct */
void
sbZfKaEpIpPortVridSmacTableEntry_Print(sbZfKaEpIpPortVridSmacTableEntry_t *pFromStruct) {
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("KaEpIpPortVridSmacTableEntry:: res=0x%04x"), (unsigned int)  pFromStruct->m_nReserved));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" smac=0x%04x%08x"),  COMPILER_64_HI(pFromStruct->m_nnSmac), COMPILER_64_LO(pFromStruct->m_nnSmac)));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

}

/* SPrint members in struct */
char *
sbZfKaEpIpPortVridSmacTableEntry_SPrint(sbZfKaEpIpPortVridSmacTableEntry_t *pFromStruct, char *pcToString, uint32 lStrSize) {
  uint32 WrCnt = 0x0;

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"KaEpIpPortVridSmacTableEntry:: res=0x%04x", (unsigned int)  pFromStruct->m_nReserved);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," smac=0x%04x%08x",   COMPILER_64_HI(pFromStruct->m_nnSmac), COMPILER_64_LO(pFromStruct->m_nnSmac));
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  /* assert if we've overrun the buffer */
  SB_ASSERT(WrCnt < lStrSize);
  return(pcToString);

}

/* validate members in struct (1 = PASS, 0 = FAIL) */
int
sbZfKaEpIpPortVridSmacTableEntry_Validate(sbZfKaEpIpPortVridSmacTableEntry_t *pZf) {
  uint64 nnSmacMax = COMPILER_64_INIT(0xFFFF, 0xFFFFFFFF);
  if (pZf->m_nReserved > 0xffff) return 0;
  if (COMPILER_64_GT(pZf->m_nnSmac, nnSmacMax)) return 0;

  return 1; /* success */

}

/* populate field from string and value */
int
sbZfKaEpIpPortVridSmacTableEntry_SetField(sbZfKaEpIpPortVridSmacTableEntry_t *s, char* name, int value) {

  if (SB_STRCMP(name, "") == 0 ) {
    return -1;
  } else if (SB_STRCMP(name, "m_nreserved") == 0) {
    s->m_nReserved = value;
  } else if (SB_STRCMP(name, "m_nnsmac") == 0) {
    COMPILER_64_SET(s->m_nnSmac,0,value);
  } else {
    /* string failed to match any field--ignored */
    return -1;
  }

  return(0);

}
