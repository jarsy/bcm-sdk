/*
 * $Id: sbZfFabBm9600InaPortPriEntryConsole.c,v 1.3 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include <shared/bsl.h>

#include "sbTypes.h"
#include <soc/sbx/sbWrappers.h>
#include "sbZfFabBm9600InaPortPriEntryConsole.hx"



/* Print members in struct */
void
sbZfFabBm9600InaPortPriEntry_Print(sbZfFabBm9600InaPortPriEntry_t *pFromStruct) {
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("FabBm9600InaPortPriEntry:: pri_15=0x%01x"), (unsigned int)  pFromStruct->m_uPri_15));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" nextpri_15=0x%01x"), (unsigned int)  pFromStruct->m_uNextpri_15));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" pri_14=0x%01x"), (unsigned int)  pFromStruct->m_uPri_14));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("FabBm9600InaPortPriEntry:: nextpri_14=0x%01x"), (unsigned int)  pFromStruct->m_uNextpri_14));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" pri_13=0x%01x"), (unsigned int)  pFromStruct->m_uPri_13));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" nextpri_13=0x%01x"), (unsigned int)  pFromStruct->m_uNextpri_13));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("FabBm9600InaPortPriEntry:: pri_12=0x%01x"), (unsigned int)  pFromStruct->m_uPri_12));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" nextpri_12=0x%01x"), (unsigned int)  pFromStruct->m_uNextpri_12));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" pri_11=0x%01x"), (unsigned int)  pFromStruct->m_uPri_11));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("FabBm9600InaPortPriEntry:: nextpri_11=0x%01x"), (unsigned int)  pFromStruct->m_uNextpri_11));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" pri_10=0x%01x"), (unsigned int)  pFromStruct->m_uPri_10));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" nextpri_10=0x%01x"), (unsigned int)  pFromStruct->m_uNextpri_10));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("FabBm9600InaPortPriEntry:: pri_9=0x%01x"), (unsigned int)  pFromStruct->m_uPri_9));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" nextpri_9=0x%01x"), (unsigned int)  pFromStruct->m_uNextpri_9));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" pri_8=0x%01x"), (unsigned int)  pFromStruct->m_uPri_8));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" nextpri_8=0x%01x"), (unsigned int)  pFromStruct->m_uNextpri_8));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("FabBm9600InaPortPriEntry:: pri_7=0x%01x"), (unsigned int)  pFromStruct->m_uPri_7));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" nextpri_7=0x%01x"), (unsigned int)  pFromStruct->m_uNextpri_7));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" pri_6=0x%01x"), (unsigned int)  pFromStruct->m_uPri_6));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" nextpri_6=0x%01x"), (unsigned int)  pFromStruct->m_uNextpri_6));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("FabBm9600InaPortPriEntry:: pri_5=0x%01x"), (unsigned int)  pFromStruct->m_uPri_5));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" nextpri_5=0x%01x"), (unsigned int)  pFromStruct->m_uNextpri_5));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" pri_4=0x%01x"), (unsigned int)  pFromStruct->m_uPri_4));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" nextpri_4=0x%01x"), (unsigned int)  pFromStruct->m_uNextpri_4));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("FabBm9600InaPortPriEntry:: pri_3=0x%01x"), (unsigned int)  pFromStruct->m_uPri_3));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" nextpri_3=0x%01x"), (unsigned int)  pFromStruct->m_uNextpri_3));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" pri_2=0x%01x"), (unsigned int)  pFromStruct->m_uPri_2));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" nextpri_2=0x%01x"), (unsigned int)  pFromStruct->m_uNextpri_2));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("FabBm9600InaPortPriEntry:: pri_1=0x%01x"), (unsigned int)  pFromStruct->m_uPri_1));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" nextpri_1=0x%01x"), (unsigned int)  pFromStruct->m_uNextpri_1));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" pri_0=0x%01x"), (unsigned int)  pFromStruct->m_uPri_0));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" nextpri_0=0x%01x"), (unsigned int)  pFromStruct->m_uNextpri_0));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

}

/* SPrint members in struct */
char *
sbZfFabBm9600InaPortPriEntry_SPrint(sbZfFabBm9600InaPortPriEntry_t *pFromStruct, char *pcToString, uint32 lStrSize) {
  uint32 WrCnt = 0x0;

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"FabBm9600InaPortPriEntry:: pri_15=0x%01x", (unsigned int)  pFromStruct->m_uPri_15);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," nextpri_15=0x%01x", (unsigned int)  pFromStruct->m_uNextpri_15);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pri_14=0x%01x", (unsigned int)  pFromStruct->m_uPri_14);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"FabBm9600InaPortPriEntry:: nextpri_14=0x%01x", (unsigned int)  pFromStruct->m_uNextpri_14);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pri_13=0x%01x", (unsigned int)  pFromStruct->m_uPri_13);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," nextpri_13=0x%01x", (unsigned int)  pFromStruct->m_uNextpri_13);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"FabBm9600InaPortPriEntry:: pri_12=0x%01x", (unsigned int)  pFromStruct->m_uPri_12);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," nextpri_12=0x%01x", (unsigned int)  pFromStruct->m_uNextpri_12);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pri_11=0x%01x", (unsigned int)  pFromStruct->m_uPri_11);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"FabBm9600InaPortPriEntry:: nextpri_11=0x%01x", (unsigned int)  pFromStruct->m_uNextpri_11);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pri_10=0x%01x", (unsigned int)  pFromStruct->m_uPri_10);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," nextpri_10=0x%01x", (unsigned int)  pFromStruct->m_uNextpri_10);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"FabBm9600InaPortPriEntry:: pri_9=0x%01x", (unsigned int)  pFromStruct->m_uPri_9);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," nextpri_9=0x%01x", (unsigned int)  pFromStruct->m_uNextpri_9);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pri_8=0x%01x", (unsigned int)  pFromStruct->m_uPri_8);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," nextpri_8=0x%01x", (unsigned int)  pFromStruct->m_uNextpri_8);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"FabBm9600InaPortPriEntry:: pri_7=0x%01x", (unsigned int)  pFromStruct->m_uPri_7);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," nextpri_7=0x%01x", (unsigned int)  pFromStruct->m_uNextpri_7);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pri_6=0x%01x", (unsigned int)  pFromStruct->m_uPri_6);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," nextpri_6=0x%01x", (unsigned int)  pFromStruct->m_uNextpri_6);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"FabBm9600InaPortPriEntry:: pri_5=0x%01x", (unsigned int)  pFromStruct->m_uPri_5);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," nextpri_5=0x%01x", (unsigned int)  pFromStruct->m_uNextpri_5);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pri_4=0x%01x", (unsigned int)  pFromStruct->m_uPri_4);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," nextpri_4=0x%01x", (unsigned int)  pFromStruct->m_uNextpri_4);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"FabBm9600InaPortPriEntry:: pri_3=0x%01x", (unsigned int)  pFromStruct->m_uPri_3);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," nextpri_3=0x%01x", (unsigned int)  pFromStruct->m_uNextpri_3);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pri_2=0x%01x", (unsigned int)  pFromStruct->m_uPri_2);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," nextpri_2=0x%01x", (unsigned int)  pFromStruct->m_uNextpri_2);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"FabBm9600InaPortPriEntry:: pri_1=0x%01x", (unsigned int)  pFromStruct->m_uPri_1);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," nextpri_1=0x%01x", (unsigned int)  pFromStruct->m_uNextpri_1);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pri_0=0x%01x", (unsigned int)  pFromStruct->m_uPri_0);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," nextpri_0=0x%01x", (unsigned int)  pFromStruct->m_uNextpri_0);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  /* assert if we've overrun the buffer */
  SB_ASSERT(WrCnt < lStrSize);
  return(pcToString);

}

/* validate members in struct (1 = PASS, 0 = FAIL) */
int
sbZfFabBm9600InaPortPriEntry_Validate(sbZfFabBm9600InaPortPriEntry_t *pZf) {

  if (pZf->m_uPri_15 > 0xf) return 0;
  if (pZf->m_uNextpri_15 > 0xf) return 0;
  if (pZf->m_uPri_14 > 0xf) return 0;
  if (pZf->m_uNextpri_14 > 0xf) return 0;
  if (pZf->m_uPri_13 > 0xf) return 0;
  if (pZf->m_uNextpri_13 > 0xf) return 0;
  if (pZf->m_uPri_12 > 0xf) return 0;
  if (pZf->m_uNextpri_12 > 0xf) return 0;
  if (pZf->m_uPri_11 > 0xf) return 0;
  if (pZf->m_uNextpri_11 > 0xf) return 0;
  if (pZf->m_uPri_10 > 0xf) return 0;
  if (pZf->m_uNextpri_10 > 0xf) return 0;
  if (pZf->m_uPri_9 > 0xf) return 0;
  if (pZf->m_uNextpri_9 > 0xf) return 0;
  if (pZf->m_uPri_8 > 0xf) return 0;
  if (pZf->m_uNextpri_8 > 0xf) return 0;
  if (pZf->m_uPri_7 > 0xf) return 0;
  if (pZf->m_uNextpri_7 > 0xf) return 0;
  if (pZf->m_uPri_6 > 0xf) return 0;
  if (pZf->m_uNextpri_6 > 0xf) return 0;
  if (pZf->m_uPri_5 > 0xf) return 0;
  if (pZf->m_uNextpri_5 > 0xf) return 0;
  if (pZf->m_uPri_4 > 0xf) return 0;
  if (pZf->m_uNextpri_4 > 0xf) return 0;
  if (pZf->m_uPri_3 > 0xf) return 0;
  if (pZf->m_uNextpri_3 > 0xf) return 0;
  if (pZf->m_uPri_2 > 0xf) return 0;
  if (pZf->m_uNextpri_2 > 0xf) return 0;
  if (pZf->m_uPri_1 > 0xf) return 0;
  if (pZf->m_uNextpri_1 > 0xf) return 0;
  if (pZf->m_uPri_0 > 0xf) return 0;
  if (pZf->m_uNextpri_0 > 0xf) return 0;

  return 1; /* success */

}

/* populate field from string and value */
int
sbZfFabBm9600InaPortPriEntry_SetField(sbZfFabBm9600InaPortPriEntry_t *s, char* name, int value) {

  if (SB_STRCMP(name, "") == 0 ) {
    return -1;
  } else if (SB_STRCMP(name, "m_upri_15") == 0) {
    s->m_uPri_15 = value;
  } else if (SB_STRCMP(name, "m_unextpri_15") == 0) {
    s->m_uNextpri_15 = value;
  } else if (SB_STRCMP(name, "m_upri_14") == 0) {
    s->m_uPri_14 = value;
  } else if (SB_STRCMP(name, "m_unextpri_14") == 0) {
    s->m_uNextpri_14 = value;
  } else if (SB_STRCMP(name, "m_upri_13") == 0) {
    s->m_uPri_13 = value;
  } else if (SB_STRCMP(name, "m_unextpri_13") == 0) {
    s->m_uNextpri_13 = value;
  } else if (SB_STRCMP(name, "m_upri_12") == 0) {
    s->m_uPri_12 = value;
  } else if (SB_STRCMP(name, "m_unextpri_12") == 0) {
    s->m_uNextpri_12 = value;
  } else if (SB_STRCMP(name, "m_upri_11") == 0) {
    s->m_uPri_11 = value;
  } else if (SB_STRCMP(name, "m_unextpri_11") == 0) {
    s->m_uNextpri_11 = value;
  } else if (SB_STRCMP(name, "m_upri_10") == 0) {
    s->m_uPri_10 = value;
  } else if (SB_STRCMP(name, "m_unextpri_10") == 0) {
    s->m_uNextpri_10 = value;
  } else if (SB_STRCMP(name, "m_upri_9") == 0) {
    s->m_uPri_9 = value;
  } else if (SB_STRCMP(name, "m_unextpri_9") == 0) {
    s->m_uNextpri_9 = value;
  } else if (SB_STRCMP(name, "m_upri_8") == 0) {
    s->m_uPri_8 = value;
  } else if (SB_STRCMP(name, "m_unextpri_8") == 0) {
    s->m_uNextpri_8 = value;
  } else if (SB_STRCMP(name, "m_upri_7") == 0) {
    s->m_uPri_7 = value;
  } else if (SB_STRCMP(name, "m_unextpri_7") == 0) {
    s->m_uNextpri_7 = value;
  } else if (SB_STRCMP(name, "m_upri_6") == 0) {
    s->m_uPri_6 = value;
  } else if (SB_STRCMP(name, "m_unextpri_6") == 0) {
    s->m_uNextpri_6 = value;
  } else if (SB_STRCMP(name, "m_upri_5") == 0) {
    s->m_uPri_5 = value;
  } else if (SB_STRCMP(name, "m_unextpri_5") == 0) {
    s->m_uNextpri_5 = value;
  } else if (SB_STRCMP(name, "m_upri_4") == 0) {
    s->m_uPri_4 = value;
  } else if (SB_STRCMP(name, "m_unextpri_4") == 0) {
    s->m_uNextpri_4 = value;
  } else if (SB_STRCMP(name, "m_upri_3") == 0) {
    s->m_uPri_3 = value;
  } else if (SB_STRCMP(name, "m_unextpri_3") == 0) {
    s->m_uNextpri_3 = value;
  } else if (SB_STRCMP(name, "m_upri_2") == 0) {
    s->m_uPri_2 = value;
  } else if (SB_STRCMP(name, "m_unextpri_2") == 0) {
    s->m_uNextpri_2 = value;
  } else if (SB_STRCMP(name, "m_upri_1") == 0) {
    s->m_uPri_1 = value;
  } else if (SB_STRCMP(name, "m_unextpri_1") == 0) {
    s->m_uNextpri_1 = value;
  } else if (SB_STRCMP(name, "m_upri_0") == 0) {
    s->m_uPri_0 = value;
  } else if (SB_STRCMP(name, "m_unextpri_0") == 0) {
    s->m_uNextpri_0 = value;
  } else {
    /* string failed to match any field--ignored */
    return -1;
  }

  return(0);

}
