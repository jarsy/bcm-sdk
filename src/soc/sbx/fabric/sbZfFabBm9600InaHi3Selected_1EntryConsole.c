/*
 * $Id: $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include <shared/bsl.h>

#include "sbTypes.h"
#include <soc/sbx/sbWrappers.h>
#include "sbZfFabBm9600InaHi3Selected_1EntryConsole.hx"



/* Print members in struct */
void
sbZfFabBm9600InaHi3Selected_1Entry_Print(sbZfFabBm9600InaHi3Selected_1Entry_t *pFromStruct) {
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("FabBm9600InaHi3Selected_1Entry:: pri=0x%01x"), (unsigned int)  pFromStruct->m_uPri));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" portsetaddr=0x%02x"), (unsigned int)  pFromStruct->m_uPortsetAddr));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" offset=0x%01x"), (unsigned int)  pFromStruct->m_uOffset));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("FabBm9600InaHi3Selected_1Entry:: sysport=0x%03x"), (unsigned int)  pFromStruct->m_uSysport));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

}

/* SPrint members in struct */
char *
sbZfFabBm9600InaHi3Selected_1Entry_SPrint(sbZfFabBm9600InaHi3Selected_1Entry_t *pFromStruct, char *pcToString, uint32 lStrSize) {
  uint32 WrCnt = 0x0;

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"FabBm9600InaHi3Selected_1Entry:: pri=0x%01x", (unsigned int)  pFromStruct->m_uPri);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," portsetaddr=0x%02x", (unsigned int)  pFromStruct->m_uPortsetAddr);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," offset=0x%01x", (unsigned int)  pFromStruct->m_uOffset);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"FabBm9600InaHi3Selected_1Entry:: sysport=0x%03x", (unsigned int)  pFromStruct->m_uSysport);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  /* assert if we've overrun the buffer */
  SB_ASSERT(WrCnt < lStrSize);
  return(pcToString);

}

/* validate members in struct (1 = PASS, 0 = FAIL) */
int
sbZfFabBm9600InaHi3Selected_1Entry_Validate(sbZfFabBm9600InaHi3Selected_1Entry_t *pZf) {

  if (pZf->m_uPri > 0xf) return 0;
  if (pZf->m_uPortsetAddr > 0xff) return 0;
  if (pZf->m_uOffset > 0xf) return 0;
  if (pZf->m_uSysport > 0xfff) return 0;

  return 1; /* success */

}

/* populate field from string and value */
int
sbZfFabBm9600InaHi3Selected_1Entry_SetField(sbZfFabBm9600InaHi3Selected_1Entry_t *s, char* name, int value) {

  if (SB_STRCMP(name, "") == 0 ) {
    return -1;
  } else if (SB_STRCMP(name, "m_upri") == 0) {
    s->m_uPri = value;
  } else if (SB_STRCMP(name, "m_uportsetaddr") == 0) {
    s->m_uPortsetAddr = value;
  } else if (SB_STRCMP(name, "m_uoffset") == 0) {
    s->m_uOffset = value;
  } else if (SB_STRCMP(name, "m_usysport") == 0) {
    s->m_uSysport = value;
  } else {
    /* string failed to match any field--ignored */
    return -1;
  }

  return(0);

}
