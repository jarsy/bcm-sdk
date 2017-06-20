/*
 * $Id: sbZfKaQsLnaPortRemapEntryConsole.c,v 1.3 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include <shared/bsl.h>

#include "sbTypes.h"
#include <soc/sbx/sbWrappers.h>
#include "sbZfKaQsLnaPortRemapEntryConsole.hx"



/* Print members in struct */
void
sbZfKaQsLnaPortRemapEntry_Print(sbZfKaQsLnaPortRemapEntry_t *pFromStruct) {
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("KaQsLnaPortRemapEntry:: res=0x%03x"), (unsigned int)  pFromStruct->m_nRes));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" port24=0x%02x"), (unsigned int)  pFromStruct->m_nPort24));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" port23=0x%02x"), (unsigned int)  pFromStruct->m_nPort23));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" port22=0x%02x"), (unsigned int)  pFromStruct->m_nPort22));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("KaQsLnaPortRemapEntry:: port21=0x%02x"), (unsigned int)  pFromStruct->m_nPort21));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" port20=0x%02x"), (unsigned int)  pFromStruct->m_nPort20));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" port19=0x%02x"), (unsigned int)  pFromStruct->m_nPort19));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" port18=0x%02x"), (unsigned int)  pFromStruct->m_nPort18));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("KaQsLnaPortRemapEntry:: port17=0x%02x"), (unsigned int)  pFromStruct->m_nPort17));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" port16=0x%02x"), (unsigned int)  pFromStruct->m_nPort16));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" port15=0x%02x"), (unsigned int)  pFromStruct->m_nPort15));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" port14=0x%02x"), (unsigned int)  pFromStruct->m_nPort14));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("KaQsLnaPortRemapEntry:: port13=0x%02x"), (unsigned int)  pFromStruct->m_nPort13));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" port12=0x%02x"), (unsigned int)  pFromStruct->m_nPort12));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" port11=0x%02x"), (unsigned int)  pFromStruct->m_nPort11));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" port10=0x%02x"), (unsigned int)  pFromStruct->m_nPort10));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("KaQsLnaPortRemapEntry:: port9=0x%02x"), (unsigned int)  pFromStruct->m_nPort9));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" port8=0x%02x"), (unsigned int)  pFromStruct->m_nPort8));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" port7=0x%02x"), (unsigned int)  pFromStruct->m_nPort7));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" port6=0x%02x"), (unsigned int)  pFromStruct->m_nPort6));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("KaQsLnaPortRemapEntry:: port5=0x%02x"), (unsigned int)  pFromStruct->m_nPort5));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" port4=0x%02x"), (unsigned int)  pFromStruct->m_nPort4));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" port3=0x%02x"), (unsigned int)  pFromStruct->m_nPort3));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" port2=0x%02x"), (unsigned int)  pFromStruct->m_nPort2));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("KaQsLnaPortRemapEntry:: port1=0x%02x"), (unsigned int)  pFromStruct->m_nPort1));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" port0=0x%02x"), (unsigned int)  pFromStruct->m_nPort0));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

}

/* SPrint members in struct */
char *
sbZfKaQsLnaPortRemapEntry_SPrint(sbZfKaQsLnaPortRemapEntry_t *pFromStruct, char *pcToString, uint32 lStrSize) {
  uint32 WrCnt = 0x0;

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"KaQsLnaPortRemapEntry:: res=0x%03x", (unsigned int)  pFromStruct->m_nRes);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," port24=0x%02x", (unsigned int)  pFromStruct->m_nPort24);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," port23=0x%02x", (unsigned int)  pFromStruct->m_nPort23);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," port22=0x%02x", (unsigned int)  pFromStruct->m_nPort22);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"KaQsLnaPortRemapEntry:: port21=0x%02x", (unsigned int)  pFromStruct->m_nPort21);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," port20=0x%02x", (unsigned int)  pFromStruct->m_nPort20);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," port19=0x%02x", (unsigned int)  pFromStruct->m_nPort19);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," port18=0x%02x", (unsigned int)  pFromStruct->m_nPort18);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"KaQsLnaPortRemapEntry:: port17=0x%02x", (unsigned int)  pFromStruct->m_nPort17);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," port16=0x%02x", (unsigned int)  pFromStruct->m_nPort16);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," port15=0x%02x", (unsigned int)  pFromStruct->m_nPort15);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," port14=0x%02x", (unsigned int)  pFromStruct->m_nPort14);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"KaQsLnaPortRemapEntry:: port13=0x%02x", (unsigned int)  pFromStruct->m_nPort13);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," port12=0x%02x", (unsigned int)  pFromStruct->m_nPort12);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," port11=0x%02x", (unsigned int)  pFromStruct->m_nPort11);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," port10=0x%02x", (unsigned int)  pFromStruct->m_nPort10);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"KaQsLnaPortRemapEntry:: port9=0x%02x", (unsigned int)  pFromStruct->m_nPort9);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," port8=0x%02x", (unsigned int)  pFromStruct->m_nPort8);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," port7=0x%02x", (unsigned int)  pFromStruct->m_nPort7);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," port6=0x%02x", (unsigned int)  pFromStruct->m_nPort6);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"KaQsLnaPortRemapEntry:: port5=0x%02x", (unsigned int)  pFromStruct->m_nPort5);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," port4=0x%02x", (unsigned int)  pFromStruct->m_nPort4);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," port3=0x%02x", (unsigned int)  pFromStruct->m_nPort3);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," port2=0x%02x", (unsigned int)  pFromStruct->m_nPort2);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"KaQsLnaPortRemapEntry:: port1=0x%02x", (unsigned int)  pFromStruct->m_nPort1);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," port0=0x%02x", (unsigned int)  pFromStruct->m_nPort0);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  /* assert if we've overrun the buffer */
  SB_ASSERT(WrCnt < lStrSize);
  return(pcToString);

}

/* validate members in struct (1 = PASS, 0 = FAIL) */
int
sbZfKaQsLnaPortRemapEntry_Validate(sbZfKaQsLnaPortRemapEntry_t *pZf) {

  if (pZf->m_nRes > 0x3ff) return 0;
  if (pZf->m_nPort24 > 0x3f) return 0;
  if (pZf->m_nPort23 > 0x3f) return 0;
  if (pZf->m_nPort22 > 0x3f) return 0;
  if (pZf->m_nPort21 > 0x3f) return 0;
  if (pZf->m_nPort20 > 0x3f) return 0;
  if (pZf->m_nPort19 > 0x3f) return 0;
  if (pZf->m_nPort18 > 0x3f) return 0;
  if (pZf->m_nPort17 > 0x3f) return 0;
  if (pZf->m_nPort16 > 0x3f) return 0;
  if (pZf->m_nPort15 > 0x3f) return 0;
  if (pZf->m_nPort14 > 0x3f) return 0;
  if (pZf->m_nPort13 > 0x3f) return 0;
  if (pZf->m_nPort12 > 0x3f) return 0;
  if (pZf->m_nPort11 > 0x3f) return 0;
  if (pZf->m_nPort10 > 0x3f) return 0;
  if (pZf->m_nPort9 > 0x3f) return 0;
  if (pZf->m_nPort8 > 0x3f) return 0;
  if (pZf->m_nPort7 > 0x3f) return 0;
  if (pZf->m_nPort6 > 0x3f) return 0;
  if (pZf->m_nPort5 > 0x3f) return 0;
  if (pZf->m_nPort4 > 0x3f) return 0;
  if (pZf->m_nPort3 > 0x3f) return 0;
  if (pZf->m_nPort2 > 0x3f) return 0;
  if (pZf->m_nPort1 > 0x3f) return 0;
  if (pZf->m_nPort0 > 0x3f) return 0;

  return 1; /* success */

}

/* populate field from string and value */
int
sbZfKaQsLnaPortRemapEntry_SetField(sbZfKaQsLnaPortRemapEntry_t *s, char* name, int value) {

  if (SB_STRCMP(name, "") == 0 ) {
    return -1;
  } else if (SB_STRCMP(name, "m_nres") == 0) {
    s->m_nRes = value;
  } else if (SB_STRCMP(name, "m_nport24") == 0) {
    s->m_nPort24 = value;
  } else if (SB_STRCMP(name, "m_nport23") == 0) {
    s->m_nPort23 = value;
  } else if (SB_STRCMP(name, "m_nport22") == 0) {
    s->m_nPort22 = value;
  } else if (SB_STRCMP(name, "m_nport21") == 0) {
    s->m_nPort21 = value;
  } else if (SB_STRCMP(name, "m_nport20") == 0) {
    s->m_nPort20 = value;
  } else if (SB_STRCMP(name, "m_nport19") == 0) {
    s->m_nPort19 = value;
  } else if (SB_STRCMP(name, "m_nport18") == 0) {
    s->m_nPort18 = value;
  } else if (SB_STRCMP(name, "m_nport17") == 0) {
    s->m_nPort17 = value;
  } else if (SB_STRCMP(name, "m_nport16") == 0) {
    s->m_nPort16 = value;
  } else if (SB_STRCMP(name, "m_nport15") == 0) {
    s->m_nPort15 = value;
  } else if (SB_STRCMP(name, "m_nport14") == 0) {
    s->m_nPort14 = value;
  } else if (SB_STRCMP(name, "m_nport13") == 0) {
    s->m_nPort13 = value;
  } else if (SB_STRCMP(name, "m_nport12") == 0) {
    s->m_nPort12 = value;
  } else if (SB_STRCMP(name, "m_nport11") == 0) {
    s->m_nPort11 = value;
  } else if (SB_STRCMP(name, "m_nport10") == 0) {
    s->m_nPort10 = value;
  } else if (SB_STRCMP(name, "m_nport9") == 0) {
    s->m_nPort9 = value;
  } else if (SB_STRCMP(name, "m_nport8") == 0) {
    s->m_nPort8 = value;
  } else if (SB_STRCMP(name, "m_nport7") == 0) {
    s->m_nPort7 = value;
  } else if (SB_STRCMP(name, "m_nport6") == 0) {
    s->m_nPort6 = value;
  } else if (SB_STRCMP(name, "m_nport5") == 0) {
    s->m_nPort5 = value;
  } else if (SB_STRCMP(name, "m_nport4") == 0) {
    s->m_nPort4 = value;
  } else if (SB_STRCMP(name, "m_nport3") == 0) {
    s->m_nPort3 = value;
  } else if (SB_STRCMP(name, "m_nport2") == 0) {
    s->m_nPort2 = value;
  } else if (SB_STRCMP(name, "m_nport1") == 0) {
    s->m_nPort1 = value;
  } else if (SB_STRCMP(name, "m_nport0") == 0) {
    s->m_nPort0 = value;
  } else {
    /* string failed to match any field--ignored */
    return -1;
  }

  return(0);

}
