/*
 * $Id: sbZfKaQsPriLutEntryConsole.c,v 1.3 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include <shared/bsl.h>

#include "sbTypes.h"
#include <soc/sbx/sbWrappers.h>
#include "sbZfKaQsPriLutEntryConsole.hx"



/* Print members in struct */
void
sbZfKaQsPriLutEntry_Print(sbZfKaQsPriLutEntry_t *pFromStruct) {
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("KaQsPriLutEntry:: res=0x%06x"), (unsigned int)  pFromStruct->m_nReserved));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" cpri=0x%01x"), (unsigned int)  pFromStruct->m_nCPri));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" npri=0x%01x"), (unsigned int)  pFromStruct->m_nNPri));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

}

/* SPrint members in struct */
char *
sbZfKaQsPriLutEntry_SPrint(sbZfKaQsPriLutEntry_t *pFromStruct, char *pcToString, uint32 lStrSize) {
  uint32 WrCnt = 0x0;

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"KaQsPriLutEntry:: res=0x%06x", (unsigned int)  pFromStruct->m_nReserved);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," cpri=0x%01x", (unsigned int)  pFromStruct->m_nCPri);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," npri=0x%01x", (unsigned int)  pFromStruct->m_nNPri);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  /* assert if we've overrun the buffer */
  SB_ASSERT(WrCnt < lStrSize);
  return(pcToString);

}

/* validate members in struct (1 = PASS, 0 = FAIL) */
int
sbZfKaQsPriLutEntry_Validate(sbZfKaQsPriLutEntry_t *pZf) {

  if (pZf->m_nReserved > 0xffffff) return 0;
  if (pZf->m_nCPri > 0xf) return 0;
  if (pZf->m_nNPri > 0xf) return 0;

  return 1; /* success */

}

/* populate field from string and value */
int
sbZfKaQsPriLutEntry_SetField(sbZfKaQsPriLutEntry_t *s, char* name, int value) {

  if (SB_STRCMP(name, "") == 0 ) {
    return -1;
  } else if (SB_STRCMP(name, "m_nreserved") == 0) {
    s->m_nReserved = value;
  } else if (SB_STRCMP(name, "m_ncpri") == 0) {
    s->m_nCPri = value;
  } else if (SB_STRCMP(name, "m_nnpri") == 0) {
    s->m_nNPri = value;
  } else {
    /* string failed to match any field--ignored */
    return -1;
  }

  return(0);

}
