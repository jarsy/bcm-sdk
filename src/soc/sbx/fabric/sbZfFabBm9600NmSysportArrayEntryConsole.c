/*
 * $Id: $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include <shared/bsl.h>

#include "sbTypes.h"
#include <soc/sbx/sbWrappers.h>
#include "sbZfFabBm9600NmSysportArrayEntryConsole.hx"



/* Print members in struct */
void
sbZfFabBm9600NmSysportArrayEntry_Print(sbZfFabBm9600NmSysportArrayEntry_t *pFromStruct) {
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("FabBm9600NmSysportArrayEntry:: spa_15=0x%03x"), (unsigned int)  pFromStruct->m_uSpa_15));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" spa_14=0x%03x"), (unsigned int)  pFromStruct->m_uSpa_14));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" spa_13=0x%03x"), (unsigned int)  pFromStruct->m_uSpa_13));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("FabBm9600NmSysportArrayEntry:: spa_12=0x%03x"), (unsigned int)  pFromStruct->m_uSpa_12));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" spa_11=0x%03x"), (unsigned int)  pFromStruct->m_uSpa_11));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" spa_10=0x%03x"), (unsigned int)  pFromStruct->m_uSpa_10));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("FabBm9600NmSysportArrayEntry:: spa_9=0x%03x"), (unsigned int)  pFromStruct->m_uSpa_9));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" spa_8=0x%03x"), (unsigned int)  pFromStruct->m_uSpa_8));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" spa_7=0x%03x"), (unsigned int)  pFromStruct->m_uSpa_7));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("FabBm9600NmSysportArrayEntry:: spa_6=0x%03x"), (unsigned int)  pFromStruct->m_uSpa_6));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" spa_5=0x%03x"), (unsigned int)  pFromStruct->m_uSpa_5));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" spa_4=0x%03x"), (unsigned int)  pFromStruct->m_uSpa_4));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("FabBm9600NmSysportArrayEntry:: spa_3=0x%03x"), (unsigned int)  pFromStruct->m_uSpa_3));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" spa_2=0x%03x"), (unsigned int)  pFromStruct->m_uSpa_2));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" spa_1=0x%03x"), (unsigned int)  pFromStruct->m_uSpa_1));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("FabBm9600NmSysportArrayEntry:: spa_0=0x%03x"), (unsigned int)  pFromStruct->m_uSpa_0));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

}

/* SPrint members in struct */
char *
sbZfFabBm9600NmSysportArrayEntry_SPrint(sbZfFabBm9600NmSysportArrayEntry_t *pFromStruct, char *pcToString, uint32 lStrSize) {
  uint32 WrCnt = 0x0;

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"FabBm9600NmSysportArrayEntry:: spa_15=0x%03x", (unsigned int)  pFromStruct->m_uSpa_15);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," spa_14=0x%03x", (unsigned int)  pFromStruct->m_uSpa_14);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," spa_13=0x%03x", (unsigned int)  pFromStruct->m_uSpa_13);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"FabBm9600NmSysportArrayEntry:: spa_12=0x%03x", (unsigned int)  pFromStruct->m_uSpa_12);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," spa_11=0x%03x", (unsigned int)  pFromStruct->m_uSpa_11);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," spa_10=0x%03x", (unsigned int)  pFromStruct->m_uSpa_10);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"FabBm9600NmSysportArrayEntry:: spa_9=0x%03x", (unsigned int)  pFromStruct->m_uSpa_9);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," spa_8=0x%03x", (unsigned int)  pFromStruct->m_uSpa_8);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," spa_7=0x%03x", (unsigned int)  pFromStruct->m_uSpa_7);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"FabBm9600NmSysportArrayEntry:: spa_6=0x%03x", (unsigned int)  pFromStruct->m_uSpa_6);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," spa_5=0x%03x", (unsigned int)  pFromStruct->m_uSpa_5);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," spa_4=0x%03x", (unsigned int)  pFromStruct->m_uSpa_4);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"FabBm9600NmSysportArrayEntry:: spa_3=0x%03x", (unsigned int)  pFromStruct->m_uSpa_3);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," spa_2=0x%03x", (unsigned int)  pFromStruct->m_uSpa_2);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," spa_1=0x%03x", (unsigned int)  pFromStruct->m_uSpa_1);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"FabBm9600NmSysportArrayEntry:: spa_0=0x%03x", (unsigned int)  pFromStruct->m_uSpa_0);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  /* assert if we've overrun the buffer */
  SB_ASSERT(WrCnt < lStrSize);
  return(pcToString);

}

/* validate members in struct (1 = PASS, 0 = FAIL) */
int
sbZfFabBm9600NmSysportArrayEntry_Validate(sbZfFabBm9600NmSysportArrayEntry_t *pZf) {

  if (pZf->m_uSpa_15 > 0xfff) return 0;
  if (pZf->m_uSpa_14 > 0xfff) return 0;
  if (pZf->m_uSpa_13 > 0xfff) return 0;
  if (pZf->m_uSpa_12 > 0xfff) return 0;
  if (pZf->m_uSpa_11 > 0xfff) return 0;
  if (pZf->m_uSpa_10 > 0xfff) return 0;
  if (pZf->m_uSpa_9 > 0xfff) return 0;
  if (pZf->m_uSpa_8 > 0xfff) return 0;
  if (pZf->m_uSpa_7 > 0xfff) return 0;
  if (pZf->m_uSpa_6 > 0xfff) return 0;
  if (pZf->m_uSpa_5 > 0xfff) return 0;
  if (pZf->m_uSpa_4 > 0xfff) return 0;
  if (pZf->m_uSpa_3 > 0xfff) return 0;
  if (pZf->m_uSpa_2 > 0xfff) return 0;
  if (pZf->m_uSpa_1 > 0xfff) return 0;
  if (pZf->m_uSpa_0 > 0xfff) return 0;

  return 1; /* success */

}

/* populate field from string and value */
int
sbZfFabBm9600NmSysportArrayEntry_SetField(sbZfFabBm9600NmSysportArrayEntry_t *s, char* name, int value) {

  if (SB_STRCMP(name, "") == 0 ) {
    return -1;
  } else if (SB_STRCMP(name, "m_uspa_15") == 0) {
    s->m_uSpa_15 = value;
  } else if (SB_STRCMP(name, "m_uspa_14") == 0) {
    s->m_uSpa_14 = value;
  } else if (SB_STRCMP(name, "m_uspa_13") == 0) {
    s->m_uSpa_13 = value;
  } else if (SB_STRCMP(name, "m_uspa_12") == 0) {
    s->m_uSpa_12 = value;
  } else if (SB_STRCMP(name, "m_uspa_11") == 0) {
    s->m_uSpa_11 = value;
  } else if (SB_STRCMP(name, "m_uspa_10") == 0) {
    s->m_uSpa_10 = value;
  } else if (SB_STRCMP(name, "m_uspa_9") == 0) {
    s->m_uSpa_9 = value;
  } else if (SB_STRCMP(name, "m_uspa_8") == 0) {
    s->m_uSpa_8 = value;
  } else if (SB_STRCMP(name, "m_uspa_7") == 0) {
    s->m_uSpa_7 = value;
  } else if (SB_STRCMP(name, "m_uspa_6") == 0) {
    s->m_uSpa_6 = value;
  } else if (SB_STRCMP(name, "m_uspa_5") == 0) {
    s->m_uSpa_5 = value;
  } else if (SB_STRCMP(name, "m_uspa_4") == 0) {
    s->m_uSpa_4 = value;
  } else if (SB_STRCMP(name, "m_uspa_3") == 0) {
    s->m_uSpa_3 = value;
  } else if (SB_STRCMP(name, "m_uspa_2") == 0) {
    s->m_uSpa_2 = value;
  } else if (SB_STRCMP(name, "m_uspa_1") == 0) {
    s->m_uSpa_1 = value;
  } else if (SB_STRCMP(name, "m_uspa_0") == 0) {
    s->m_uSpa_0 = value;
  } else {
    /* string failed to match any field--ignored */
    return -1;
  }

  return(0);

}
