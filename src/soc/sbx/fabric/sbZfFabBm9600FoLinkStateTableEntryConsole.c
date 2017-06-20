/*
 * $Id: $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include <shared/bsl.h>

#include "sbTypes.h"
#include <soc/sbx/sbWrappers.h>
#include "sbZfFabBm9600FoLinkStateTableEntryConsole.hx"



/* Print members in struct */
void
sbZfFabBm9600FoLinkStateTableEntry_Print(sbZfFabBm9600FoLinkStateTableEntry_t *pFromStruct) {
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("FabBm9600FoLinkStateTableEntry:: sb=0x%01x"), (unsigned int)  pFromStruct->m_uSb));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" error=0x%01x"), (unsigned int)  pFromStruct->m_uError));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" linkstate=0x%08x"), (unsigned int)  pFromStruct->m_uLinkState));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

}

/* SPrint members in struct */
char *
sbZfFabBm9600FoLinkStateTableEntry_SPrint(sbZfFabBm9600FoLinkStateTableEntry_t *pFromStruct, char *pcToString, uint32 lStrSize) {
  uint32 WrCnt = 0x0;

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"FabBm9600FoLinkStateTableEntry:: sb=0x%01x", (unsigned int)  pFromStruct->m_uSb);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," error=0x%01x", (unsigned int)  pFromStruct->m_uError);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," linkstate=0x%08x", (unsigned int)  pFromStruct->m_uLinkState);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  /* assert if we've overrun the buffer */
  SB_ASSERT(WrCnt < lStrSize);
  return(pcToString);

}

/* validate members in struct (1 = PASS, 0 = FAIL) */
int
sbZfFabBm9600FoLinkStateTableEntry_Validate(sbZfFabBm9600FoLinkStateTableEntry_t *pZf) {

  if (pZf->m_uSb > 0x1) return 0;
  if (pZf->m_uError > 0x1) return 0;
  /* pZf->m_uLinkState implicitly masked by data type */

  return 1; /* success */

}

/* populate field from string and value */
int
sbZfFabBm9600FoLinkStateTableEntry_SetField(sbZfFabBm9600FoLinkStateTableEntry_t *s, char* name, int value) {

  if (SB_STRCMP(name, "") == 0 ) {
    return -1;
  } else if (SB_STRCMP(name, "m_usb") == 0) {
    s->m_uSb = value;
  } else if (SB_STRCMP(name, "m_uerror") == 0) {
    s->m_uError = value;
  } else if (SB_STRCMP(name, "m_ulinkstate") == 0) {
    s->m_uLinkState = value;
  } else {
    /* string failed to match any field--ignored */
    return -1;
  }

  return(0);

}
