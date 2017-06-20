/*
 * $Id: $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include <shared/bsl.h>

#include "sbTypes.h"
#include <soc/sbx/sbWrappers.h>
#include "sbZfFabBm9600NmFullStatusEntryConsole.hx"



/* Print members in struct */
void
sbZfFabBm9600NmFullStatusEntry_Print(sbZfFabBm9600NmFullStatusEntry_t *pFromStruct) {
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("FabBm9600NmFullStatusEntry:: fs8=0x%03x"), (unsigned int)  pFromStruct->m_FullStatus8));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" fs7=0x%08x"), (unsigned int)  pFromStruct->m_FullStatus7));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" fs6=0x%08x"), (unsigned int)  pFromStruct->m_FullStatus6));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("FabBm9600NmFullStatusEntry:: fs5=0x%08x"), (unsigned int)  pFromStruct->m_FullStatus5));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" fs4=0x%08x"), (unsigned int)  pFromStruct->m_FullStatus4));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" fs3=0x%08x"), (unsigned int)  pFromStruct->m_FullStatus3));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("FabBm9600NmFullStatusEntry:: fs2=0x%08x"), (unsigned int)  pFromStruct->m_FullStatus2));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" fs1=0x%08x"), (unsigned int)  pFromStruct->m_FullStatus1));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" fs0=0x%08x"), (unsigned int)  pFromStruct->m_FullStatus0));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

}

/* SPrint members in struct */
char *
sbZfFabBm9600NmFullStatusEntry_SPrint(sbZfFabBm9600NmFullStatusEntry_t *pFromStruct, char *pcToString, uint32 lStrSize) {
  uint32 WrCnt = 0x0;

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"FabBm9600NmFullStatusEntry:: fs8=0x%03x", (unsigned int)  pFromStruct->m_FullStatus8);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," fs7=0x%08x", (unsigned int)  pFromStruct->m_FullStatus7);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," fs6=0x%08x", (unsigned int)  pFromStruct->m_FullStatus6);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"FabBm9600NmFullStatusEntry:: fs5=0x%08x", (unsigned int)  pFromStruct->m_FullStatus5);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," fs4=0x%08x", (unsigned int)  pFromStruct->m_FullStatus4);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," fs3=0x%08x", (unsigned int)  pFromStruct->m_FullStatus3);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"FabBm9600NmFullStatusEntry:: fs2=0x%08x", (unsigned int)  pFromStruct->m_FullStatus2);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," fs1=0x%08x", (unsigned int)  pFromStruct->m_FullStatus1);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," fs0=0x%08x", (unsigned int)  pFromStruct->m_FullStatus0);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  /* assert if we've overrun the buffer */
  SB_ASSERT(WrCnt < lStrSize);
  return(pcToString);

}

/* validate members in struct (1 = PASS, 0 = FAIL) */
int
sbZfFabBm9600NmFullStatusEntry_Validate(sbZfFabBm9600NmFullStatusEntry_t *pZf) {

  if (pZf->m_FullStatus8 > 0x3ff) return 0;
  /* pZf->m_FullStatus7 implicitly masked by data type */
  /* pZf->m_FullStatus6 implicitly masked by data type */
  /* pZf->m_FullStatus5 implicitly masked by data type */
  /* pZf->m_FullStatus4 implicitly masked by data type */
  /* pZf->m_FullStatus3 implicitly masked by data type */
  /* pZf->m_FullStatus2 implicitly masked by data type */
  /* pZf->m_FullStatus1 implicitly masked by data type */
  /* pZf->m_FullStatus0 implicitly masked by data type */

  return 1; /* success */

}

/* populate field from string and value */
int
sbZfFabBm9600NmFullStatusEntry_SetField(sbZfFabBm9600NmFullStatusEntry_t *s, char* name, int value) {

  if (SB_STRCMP(name, "") == 0 ) {
    return -1;
  } else if (SB_STRCMP(name, "m_fullstatus8") == 0) {
    s->m_FullStatus8 = value;
  } else if (SB_STRCMP(name, "m_fullstatus7") == 0) {
    s->m_FullStatus7 = value;
  } else if (SB_STRCMP(name, "m_fullstatus6") == 0) {
    s->m_FullStatus6 = value;
  } else if (SB_STRCMP(name, "m_fullstatus5") == 0) {
    s->m_FullStatus5 = value;
  } else if (SB_STRCMP(name, "m_fullstatus4") == 0) {
    s->m_FullStatus4 = value;
  } else if (SB_STRCMP(name, "m_fullstatus3") == 0) {
    s->m_FullStatus3 = value;
  } else if (SB_STRCMP(name, "m_fullstatus2") == 0) {
    s->m_FullStatus2 = value;
  } else if (SB_STRCMP(name, "m_fullstatus1") == 0) {
    s->m_FullStatus1 = value;
  } else if (SB_STRCMP(name, "m_fullstatus0") == 0) {
    s->m_FullStatus0 = value;
  } else {
    /* string failed to match any field--ignored */
    return -1;
  }

  return(0);

}
