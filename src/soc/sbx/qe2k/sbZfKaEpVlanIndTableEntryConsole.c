/*
 * $Id: sbZfKaEpVlanIndTableEntryConsole.c,v 1.3 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include <shared/bsl.h>

#include "sbTypes.h"
#include <soc/sbx/sbWrappers.h>
#include "sbZfKaEpVlanIndTableEntryConsole.hx"



/* Print members in struct */
void
sbZfKaEpVlanIndTableEntry_Print(sbZfKaEpVlanIndTableEntry_t *pFromStruct) {
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("KaEpVlanIndTableEntry:: record3=0x%04x"), (unsigned int)  pFromStruct->m_nRecord3));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" record2=0x%04x"), (unsigned int)  pFromStruct->m_nRecord2));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" record1=0x%04x"), (unsigned int)  pFromStruct->m_nRecord1));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("KaEpVlanIndTableEntry:: record0=0x%04x"), (unsigned int)  pFromStruct->m_nRecord0));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

}

/* SPrint members in struct */
char *
sbZfKaEpVlanIndTableEntry_SPrint(sbZfKaEpVlanIndTableEntry_t *pFromStruct, char *pcToString, uint32 lStrSize) {
  uint32 WrCnt = 0x0;

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"KaEpVlanIndTableEntry:: record3=0x%04x", (unsigned int)  pFromStruct->m_nRecord3);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," record2=0x%04x", (unsigned int)  pFromStruct->m_nRecord2);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," record1=0x%04x", (unsigned int)  pFromStruct->m_nRecord1);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"KaEpVlanIndTableEntry:: record0=0x%04x", (unsigned int)  pFromStruct->m_nRecord0);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  /* assert if we've overrun the buffer */
  SB_ASSERT(WrCnt < lStrSize);
  return(pcToString);

}

/* validate members in struct (1 = PASS, 0 = FAIL) */
int
sbZfKaEpVlanIndTableEntry_Validate(sbZfKaEpVlanIndTableEntry_t *pZf) {

  if (pZf->m_nRecord3 > 0xffff) return 0;
  if (pZf->m_nRecord2 > 0xffff) return 0;
  if (pZf->m_nRecord1 > 0xffff) return 0;
  if (pZf->m_nRecord0 > 0xffff) return 0;

  return 1; /* success */

}

/* populate field from string and value */
int
sbZfKaEpVlanIndTableEntry_SetField(sbZfKaEpVlanIndTableEntry_t *s, char* name, int value) {

  if (SB_STRCMP(name, "") == 0 ) {
    return -1;
  } else if (SB_STRCMP(name, "m_nrecord3") == 0) {
    s->m_nRecord3 = value;
  } else if (SB_STRCMP(name, "m_nrecord2") == 0) {
    s->m_nRecord2 = value;
  } else if (SB_STRCMP(name, "m_nrecord1") == 0) {
    s->m_nRecord1 = value;
  } else if (SB_STRCMP(name, "m_nrecord0") == 0) {
    s->m_nRecord0 = value;
  } else {
    /* string failed to match any field--ignored */
    return -1;
  }

  return(0);

}
