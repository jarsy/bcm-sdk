/*
 * $Id: sbZfKaSrManualDeskewEntryConsole.c,v 1.3 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include <shared/bsl.h>

#include "sbTypes.h"
#include <soc/sbx/sbWrappers.h>
#include "sbZfKaSrManualDeskewEntryConsole.hx"



/* Print members in struct */
void
sbZfKaSrManualDeskewEntry_Print(sbZfKaSrManualDeskewEntry_t *pFromStruct) {
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("KaSrManualDeskewEntry:: lane16=0x%02x"), (unsigned int)  pFromStruct->m_nLane16));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" lane15=0x%02x"), (unsigned int)  pFromStruct->m_nLane15));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" lane14=0x%02x"), (unsigned int)  pFromStruct->m_nLane14));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" lane13=0x%02x"), (unsigned int)  pFromStruct->m_nLane13));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("KaSrManualDeskewEntry:: lane12=0x%02x"), (unsigned int)  pFromStruct->m_nLane12));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" lane11=0x%02x"), (unsigned int)  pFromStruct->m_nLane11));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" lane10=0x%02x"), (unsigned int)  pFromStruct->m_nLane10));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" lane9=0x%02x"), (unsigned int)  pFromStruct->m_nLane9));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("KaSrManualDeskewEntry:: lane8=0x%02x"), (unsigned int)  pFromStruct->m_nLane8));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" lane7=0x%02x"), (unsigned int)  pFromStruct->m_nLane7));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" lane6=0x%02x"), (unsigned int)  pFromStruct->m_nLane6));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" lane5=0x%02x"), (unsigned int)  pFromStruct->m_nLane5));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("KaSrManualDeskewEntry:: lane4=0x%02x"), (unsigned int)  pFromStruct->m_nLane4));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" lane3=0x%02x"), (unsigned int)  pFromStruct->m_nLane3));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" lane2=0x%02x"), (unsigned int)  pFromStruct->m_nLane2));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" lane1=0x%02x"), (unsigned int)  pFromStruct->m_nLane1));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("KaSrManualDeskewEntry:: lane0=0x%02x"), (unsigned int)  pFromStruct->m_nLane0));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

}

/* SPrint members in struct */
char *
sbZfKaSrManualDeskewEntry_SPrint(sbZfKaSrManualDeskewEntry_t *pFromStruct, char *pcToString, uint32 lStrSize) {
  uint32 WrCnt = 0x0;

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"KaSrManualDeskewEntry:: lane16=0x%02x", (unsigned int)  pFromStruct->m_nLane16);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," lane15=0x%02x", (unsigned int)  pFromStruct->m_nLane15);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," lane14=0x%02x", (unsigned int)  pFromStruct->m_nLane14);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," lane13=0x%02x", (unsigned int)  pFromStruct->m_nLane13);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"KaSrManualDeskewEntry:: lane12=0x%02x", (unsigned int)  pFromStruct->m_nLane12);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," lane11=0x%02x", (unsigned int)  pFromStruct->m_nLane11);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," lane10=0x%02x", (unsigned int)  pFromStruct->m_nLane10);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," lane9=0x%02x", (unsigned int)  pFromStruct->m_nLane9);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"KaSrManualDeskewEntry:: lane8=0x%02x", (unsigned int)  pFromStruct->m_nLane8);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," lane7=0x%02x", (unsigned int)  pFromStruct->m_nLane7);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," lane6=0x%02x", (unsigned int)  pFromStruct->m_nLane6);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," lane5=0x%02x", (unsigned int)  pFromStruct->m_nLane5);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"KaSrManualDeskewEntry:: lane4=0x%02x", (unsigned int)  pFromStruct->m_nLane4);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," lane3=0x%02x", (unsigned int)  pFromStruct->m_nLane3);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," lane2=0x%02x", (unsigned int)  pFromStruct->m_nLane2);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," lane1=0x%02x", (unsigned int)  pFromStruct->m_nLane1);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"KaSrManualDeskewEntry:: lane0=0x%02x", (unsigned int)  pFromStruct->m_nLane0);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  /* assert if we've overrun the buffer */
  SB_ASSERT(WrCnt < lStrSize);
  return(pcToString);

}

/* validate members in struct (1 = PASS, 0 = FAIL) */
int
sbZfKaSrManualDeskewEntry_Validate(sbZfKaSrManualDeskewEntry_t *pZf) {

  if (pZf->m_nLane16 > 0x1f) return 0;
  if (pZf->m_nLane15 > 0x1f) return 0;
  if (pZf->m_nLane14 > 0x1f) return 0;
  if (pZf->m_nLane13 > 0x1f) return 0;
  if (pZf->m_nLane12 > 0x1f) return 0;
  if (pZf->m_nLane11 > 0x1f) return 0;
  if (pZf->m_nLane10 > 0x1f) return 0;
  if (pZf->m_nLane9 > 0x1f) return 0;
  if (pZf->m_nLane8 > 0x1f) return 0;
  if (pZf->m_nLane7 > 0x1f) return 0;
  if (pZf->m_nLane6 > 0x1f) return 0;
  if (pZf->m_nLane5 > 0x1f) return 0;
  if (pZf->m_nLane4 > 0x1f) return 0;
  if (pZf->m_nLane3 > 0x1f) return 0;
  if (pZf->m_nLane2 > 0x1f) return 0;
  if (pZf->m_nLane1 > 0x1f) return 0;
  if (pZf->m_nLane0 > 0x1f) return 0;

  return 1; /* success */

}

/* populate field from string and value */
int
sbZfKaSrManualDeskewEntry_SetField(sbZfKaSrManualDeskewEntry_t *s, char* name, int value) {

  if (SB_STRCMP(name, "") == 0 ) {
    return -1;
  } else if (SB_STRCMP(name, "m_nlane16") == 0) {
    s->m_nLane16 = value;
  } else if (SB_STRCMP(name, "m_nlane15") == 0) {
    s->m_nLane15 = value;
  } else if (SB_STRCMP(name, "m_nlane14") == 0) {
    s->m_nLane14 = value;
  } else if (SB_STRCMP(name, "m_nlane13") == 0) {
    s->m_nLane13 = value;
  } else if (SB_STRCMP(name, "m_nlane12") == 0) {
    s->m_nLane12 = value;
  } else if (SB_STRCMP(name, "m_nlane11") == 0) {
    s->m_nLane11 = value;
  } else if (SB_STRCMP(name, "m_nlane10") == 0) {
    s->m_nLane10 = value;
  } else if (SB_STRCMP(name, "m_nlane9") == 0) {
    s->m_nLane9 = value;
  } else if (SB_STRCMP(name, "m_nlane8") == 0) {
    s->m_nLane8 = value;
  } else if (SB_STRCMP(name, "m_nlane7") == 0) {
    s->m_nLane7 = value;
  } else if (SB_STRCMP(name, "m_nlane6") == 0) {
    s->m_nLane6 = value;
  } else if (SB_STRCMP(name, "m_nlane5") == 0) {
    s->m_nLane5 = value;
  } else if (SB_STRCMP(name, "m_nlane4") == 0) {
    s->m_nLane4 = value;
  } else if (SB_STRCMP(name, "m_nlane3") == 0) {
    s->m_nLane3 = value;
  } else if (SB_STRCMP(name, "m_nlane2") == 0) {
    s->m_nLane2 = value;
  } else if (SB_STRCMP(name, "m_nlane1") == 0) {
    s->m_nLane1 = value;
  } else if (SB_STRCMP(name, "m_nlane0") == 0) {
    s->m_nLane0 = value;
  } else {
    /* string failed to match any field--ignored */
    return -1;
  }

  return(0);

}
