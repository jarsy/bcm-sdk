/*
 * $Id: sbZfKaRbPoliceEBSEntryConsole.c,v 1.3 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include <shared/bsl.h>

#include "sbTypes.h"
#include <soc/sbx/sbWrappers.h>
#include "sbZfKaRbPoliceEBSEntryConsole.hx"



/* Print members in struct */
void
sbZfKaRbPoliceEBSEntry_Print(sbZfKaRbPoliceEBSEntry_t *pFromStruct) {
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("KaRbPoliceEBSEntry:: ebs=0x%06x"), (unsigned int)  pFromStruct->m_nEBS));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

}

/* SPrint members in struct */
char *
sbZfKaRbPoliceEBSEntry_SPrint(sbZfKaRbPoliceEBSEntry_t *pFromStruct, char *pcToString, uint32 lStrSize) {
  uint32 WrCnt = 0x0;

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"KaRbPoliceEBSEntry:: ebs=0x%06x", (unsigned int)  pFromStruct->m_nEBS);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  /* assert if we've overrun the buffer */
  SB_ASSERT(WrCnt < lStrSize);
  return(pcToString);

}

/* validate members in struct (1 = PASS, 0 = FAIL) */
int
sbZfKaRbPoliceEBSEntry_Validate(sbZfKaRbPoliceEBSEntry_t *pZf) {

  if (pZf->m_nEBS > 0xffffff) return 0;

  return 1; /* success */

}

/* populate field from string and value */
int
sbZfKaRbPoliceEBSEntry_SetField(sbZfKaRbPoliceEBSEntry_t *s, char* name, int value) {

  if (SB_STRCMP(name, "") == 0 ) {
    return -1;
  } else if (SB_STRCMP(name, "m_nebs") == 0) {
    s->m_nEBS = value;
  } else {
    /* string failed to match any field--ignored */
    return -1;
  }

  return(0);

}
