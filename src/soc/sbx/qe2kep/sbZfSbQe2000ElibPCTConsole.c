/*
 * $Id: sbZfSbQe2000ElibPCTConsole.c,v 1.5 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include <shared/bsl.h>

#include "sbTypesGlue.h"
#include <soc/sbx/sbWrappers.h>
#include "sbZfSbQe2000ElibPCTConsole.hx"



/* Print members in struct */
void
sbZfSbQe2000ElibPCT_Print(sbZfSbQe2000ElibPCT_t *pFromStruct) {
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("SbQe2000ElibPCT:: pktcnt15=0x%08x"),   COMPILER_64_LO(pFromStruct->m_PktClass15)));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" bytecnt15=0x%01x%08x"),  COMPILER_64_HI(pFromStruct->m_ByteClass15), COMPILER_64_LO(pFromStruct->m_ByteClass15)));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("SbQe2000ElibPCT:: pktcnt14=0x%08x"),   COMPILER_64_LO(pFromStruct->m_PktClass14)));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" bytecnt14=0x%01x%08x"),  COMPILER_64_HI(pFromStruct->m_ByteClass14), COMPILER_64_LO(pFromStruct->m_ByteClass14)));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("SbQe2000ElibPCT:: pktcnt13=0x%08x"),   COMPILER_64_LO(pFromStruct->m_PktClass13)));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" bytecnt13=0x%01x%08x"),  COMPILER_64_HI(pFromStruct->m_ByteClass13), COMPILER_64_LO(pFromStruct->m_ByteClass13)));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("SbQe2000ElibPCT:: pktcnt12=0x%08x"),  COMPILER_64_LO(pFromStruct->m_PktClass12)));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" bytecnt12=0x%01x%08x"),  COMPILER_64_HI(pFromStruct->m_ByteClass12), COMPILER_64_LO(pFromStruct->m_ByteClass12)));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("SbQe2000ElibPCT:: pktcnt11=0x%08x"),  COMPILER_64_LO(pFromStruct->m_PktClass11)));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" bytecnt11=0x%01x%08x"),  COMPILER_64_HI(pFromStruct->m_ByteClass11), COMPILER_64_LO(pFromStruct->m_ByteClass11)));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("SbQe2000ElibPCT:: pktcnt10=0x%08x"),  COMPILER_64_LO(pFromStruct->m_PktClass10)));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" bytecnt10=0x%01x%08x"),  COMPILER_64_HI(pFromStruct->m_ByteClass10), COMPILER_64_LO(pFromStruct->m_ByteClass10)));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("SbQe2000ElibPCT:: pktcnt09=0x%08x"),  COMPILER_64_LO(pFromStruct->m_PktClass9)));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" bytecnt09=0x%01x%08x"),  COMPILER_64_HI(pFromStruct->m_ByteClass9), COMPILER_64_LO(pFromStruct->m_ByteClass9)));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("SbQe2000ElibPCT:: pktcnt08=0x%08x"),  COMPILER_64_LO(pFromStruct->m_PktClass8)));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" bytecnt08=0x%01x%08x"),  COMPILER_64_HI(pFromStruct->m_ByteClass8), COMPILER_64_LO(pFromStruct->m_ByteClass8)));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("SbQe2000ElibPCT:: pktcnt07=0x%08x"),  COMPILER_64_LO(pFromStruct->m_PktClass7)));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" bytecnt07=0x%01x%08x"),  COMPILER_64_HI(pFromStruct->m_ByteClass7), COMPILER_64_LO(pFromStruct->m_ByteClass7)));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("SbQe2000ElibPCT:: pktcnt06=0x%08x"),  COMPILER_64_LO(pFromStruct->m_PktClass6)));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" bytecnt06=0x%01x%08x"),  COMPILER_64_HI(pFromStruct->m_ByteClass6), COMPILER_64_LO(pFromStruct->m_ByteClass6)));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("SbQe2000ElibPCT:: pktcnt05=0x%08x"),  COMPILER_64_LO(pFromStruct->m_PktClass5)));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" bytecnt05=0x%01x%08x"),  COMPILER_64_HI(pFromStruct->m_ByteClass5), COMPILER_64_LO(pFromStruct->m_ByteClass5)));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("SbQe2000ElibPCT:: pktcnt04=0x%08x"),  COMPILER_64_LO(pFromStruct->m_PktClass4)));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" bytecnt04=0x%01x%08x"),  COMPILER_64_HI(pFromStruct->m_ByteClass4), COMPILER_64_LO(pFromStruct->m_ByteClass4)));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("SbQe2000ElibPCT:: pktcnt03=0x%08x"),  COMPILER_64_LO(pFromStruct->m_PktClass3)));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" bytecnt03=0x%01x%08x"),  COMPILER_64_HI(pFromStruct->m_ByteClass3), COMPILER_64_LO(pFromStruct->m_ByteClass3)));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("SbQe2000ElibPCT:: pktcnt02=0x%08x"),  COMPILER_64_LO(pFromStruct->m_PktClass2)));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" bytecnt02=0x%01x%08x"),  COMPILER_64_HI(pFromStruct->m_ByteClass2), COMPILER_64_LO(pFromStruct->m_ByteClass2)));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("SbQe2000ElibPCT:: pktcnt01=0x%08x"),  COMPILER_64_LO(pFromStruct->m_PktClass1)));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" bytecnt01=0x%01x%08x"),  COMPILER_64_HI(pFromStruct->m_ByteClass1), COMPILER_64_LO(pFromStruct->m_ByteClass1)));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("SbQe2000ElibPCT:: pktcnt00=0x%08x"),  COMPILER_64_LO(pFromStruct->m_PktClass0)));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" bytecnt00=0x%01x%08x"),  COMPILER_64_HI(pFromStruct->m_ByteClass0), COMPILER_64_LO(pFromStruct->m_ByteClass0)));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

}

/* SPrint members in struct */
char *
sbZfSbQe2000ElibPCT_SPrint(sbZfSbQe2000ElibPCT_t *pFromStruct, char *pcToString, uint32 lStrSize) {
  uint32 WrCnt = 0x0;

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"SbQe2000ElibPCT:: pktcnt15=0x%08x",  COMPILER_64_LO(pFromStruct->m_PktClass15));
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," bytecnt15=0x%01x%08x",  COMPILER_64_HI(pFromStruct->m_ByteClass15), COMPILER_64_LO(pFromStruct->m_ByteClass15));
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"SbQe2000ElibPCT:: pktcnt14=0x%08x",  COMPILER_64_LO(pFromStruct->m_PktClass14));
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," bytecnt14=0x%01x%08x",  COMPILER_64_HI(pFromStruct->m_ByteClass14), COMPILER_64_LO(pFromStruct->m_ByteClass14));
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"SbQe2000ElibPCT:: pktcnt13=0x%08x",  COMPILER_64_LO(pFromStruct->m_PktClass13));
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," bytecnt13=0x%01x%08x",  COMPILER_64_HI(pFromStruct->m_ByteClass13), COMPILER_64_LO(pFromStruct->m_ByteClass13));
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"SbQe2000ElibPCT:: pktcnt12=0x%08x",  COMPILER_64_LO(pFromStruct->m_PktClass12));
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," bytecnt12=0x%01x%08x",  COMPILER_64_HI(pFromStruct->m_ByteClass12), COMPILER_64_LO(pFromStruct->m_ByteClass12));
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"SbQe2000ElibPCT:: pktcnt11=0x%08x",  COMPILER_64_LO(pFromStruct->m_PktClass11));
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," bytecnt11=0x%01x%08x",  COMPILER_64_HI(pFromStruct->m_ByteClass11), COMPILER_64_LO(pFromStruct->m_ByteClass11));
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"SbQe2000ElibPCT:: pktcnt10=0x%08x",  COMPILER_64_LO(pFromStruct->m_PktClass10));
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," bytecnt10=0x%01x%08x",  COMPILER_64_HI(pFromStruct->m_ByteClass10), COMPILER_64_LO(pFromStruct->m_ByteClass10));
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"SbQe2000ElibPCT:: pktcnt09=0x%08x",  COMPILER_64_LO(pFromStruct->m_PktClass9));
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," bytecnt09=0x%01x%08x",  COMPILER_64_HI(pFromStruct->m_ByteClass9), COMPILER_64_LO(pFromStruct->m_ByteClass9));
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"SbQe2000ElibPCT:: pktcnt08=0x%08x",  COMPILER_64_LO(pFromStruct->m_PktClass8));
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," bytecnt08=0x%01x%08x",  COMPILER_64_HI(pFromStruct->m_ByteClass8), COMPILER_64_LO(pFromStruct->m_ByteClass8));
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"SbQe2000ElibPCT:: pktcnt07=0x%08x",  COMPILER_64_LO(pFromStruct->m_PktClass7));
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," bytecnt07=0x%01x%08x",  COMPILER_64_HI(pFromStruct->m_ByteClass7), COMPILER_64_LO(pFromStruct->m_ByteClass7));
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"SbQe2000ElibPCT:: pktcnt06=0x%08x",  COMPILER_64_LO(pFromStruct->m_PktClass6));
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," bytecnt06=0x%01x%08x",  COMPILER_64_HI(pFromStruct->m_ByteClass6), COMPILER_64_LO(pFromStruct->m_ByteClass6));
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"SbQe2000ElibPCT:: pktcnt05=0x%08x",  COMPILER_64_LO(pFromStruct->m_PktClass5));
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," bytecnt05=0x%01x%08x",  COMPILER_64_HI(pFromStruct->m_ByteClass5), COMPILER_64_LO(pFromStruct->m_ByteClass5));
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"SbQe2000ElibPCT:: pktcnt04=0x%08x",  COMPILER_64_LO(pFromStruct->m_PktClass4));
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," bytecnt04=0x%01x%08x",  COMPILER_64_HI(pFromStruct->m_ByteClass4), COMPILER_64_LO(pFromStruct->m_ByteClass4));
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"SbQe2000ElibPCT:: pktcnt03=0x%08x",  COMPILER_64_LO(pFromStruct->m_PktClass3));
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," bytecnt03=0x%01x%08x",  COMPILER_64_HI(pFromStruct->m_ByteClass3), COMPILER_64_LO(pFromStruct->m_ByteClass3));
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"SbQe2000ElibPCT:: pktcnt02=0x%08x",  COMPILER_64_LO(pFromStruct->m_PktClass2));
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," bytecnt02=0x%01x%08x",  COMPILER_64_HI(pFromStruct->m_ByteClass2), COMPILER_64_LO(pFromStruct->m_ByteClass2));
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"SbQe2000ElibPCT:: pktcnt01=0x%08x",  COMPILER_64_LO(pFromStruct->m_PktClass1));
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," bytecnt01=0x%01x%08x",  COMPILER_64_HI(pFromStruct->m_ByteClass1), COMPILER_64_LO(pFromStruct->m_ByteClass1));
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"SbQe2000ElibPCT:: pktcnt00=0x%08x",  COMPILER_64_LO(pFromStruct->m_PktClass0));
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," bytecnt00=0x%01x%08x",  COMPILER_64_HI(pFromStruct->m_ByteClass0), COMPILER_64_LO(pFromStruct->m_ByteClass0));
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  /* assert if we've overrun the buffer */
  SB_ASSERT(WrCnt < lStrSize);
  return(pcToString);

}

/* validate members in struct (1 = PASS, 0 = FAIL) */
int
sbZfSbQe2000ElibPCT_Validate(sbZfSbQe2000ElibPCT_t *pZf) {
  uint64 PktClassMax  = COMPILER_64_INIT(0x0, 0x1fffffff), 
         ByteClassMax = COMPILER_64_INIT(0x7, 0xFFFFFFFF);
  if (COMPILER_64_GT(pZf->m_PktClass15, PktClassMax)) return 0;
  if (COMPILER_64_GT(pZf->m_ByteClass15, ByteClassMax)) return 0;
  if (COMPILER_64_GT(pZf->m_PktClass14, PktClassMax)) return 0;
  if (COMPILER_64_GT(pZf->m_ByteClass14, ByteClassMax)) return 0;
  if (COMPILER_64_GT(pZf->m_PktClass13, PktClassMax)) return 0;
  if (COMPILER_64_GT(pZf->m_ByteClass13, ByteClassMax)) return 0;
  if (COMPILER_64_GT(pZf->m_PktClass12, PktClassMax)) return 0;
  if (COMPILER_64_GT(pZf->m_ByteClass12, ByteClassMax)) return 0;
  if (COMPILER_64_GT(pZf->m_PktClass11, PktClassMax)) return 0;
  if (COMPILER_64_GT(pZf->m_ByteClass11, ByteClassMax)) return 0;
  if (COMPILER_64_GT(pZf->m_PktClass10, PktClassMax)) return 0;
  if (COMPILER_64_GT(pZf->m_ByteClass10, ByteClassMax)) return 0;
  if (COMPILER_64_GT(pZf->m_PktClass9, PktClassMax)) return 0;
  if (COMPILER_64_GT(pZf->m_ByteClass9, ByteClassMax)) return 0;
  if (COMPILER_64_GT(pZf->m_PktClass8, PktClassMax)) return 0;
  if (COMPILER_64_GT(pZf->m_ByteClass8, ByteClassMax)) return 0;
  if (COMPILER_64_GT(pZf->m_PktClass7, PktClassMax)) return 0;
  if (COMPILER_64_GT(pZf->m_ByteClass7, ByteClassMax)) return 0;
  if (COMPILER_64_GT(pZf->m_PktClass6, PktClassMax)) return 0;
  if (COMPILER_64_GT(pZf->m_ByteClass6, ByteClassMax)) return 0;
  if (COMPILER_64_GT(pZf->m_PktClass5, PktClassMax)) return 0;
  if (COMPILER_64_GT(pZf->m_ByteClass5, ByteClassMax)) return 0;
  if (COMPILER_64_GT(pZf->m_PktClass4, PktClassMax)) return 0;
  if (COMPILER_64_GT(pZf->m_ByteClass4, ByteClassMax)) return 0;
  if (COMPILER_64_GT(pZf->m_PktClass3, PktClassMax)) return 0;
  if (COMPILER_64_GT(pZf->m_ByteClass3, ByteClassMax)) return 0;
  if (COMPILER_64_GT(pZf->m_PktClass2, PktClassMax)) return 0;
  if (COMPILER_64_GT(pZf->m_ByteClass2, ByteClassMax)) return 0;
  if (COMPILER_64_GT(pZf->m_PktClass1, PktClassMax)) return 0;
  if (COMPILER_64_GT(pZf->m_ByteClass1, ByteClassMax)) return 0;
  if (COMPILER_64_GT(pZf->m_PktClass0, PktClassMax)) return 0;
  if (COMPILER_64_GT(pZf->m_ByteClass0, ByteClassMax)) return 0;

  return 1; /* success */

}

/* populate field from string and value */
int
sbZfSbQe2000ElibPCT_SetField(sbZfSbQe2000ElibPCT_t *s, char* name, int value) {

  if (SB_STRCMP(name, "") == 0 ) {
    return -1;
  } else if (SB_STRCMP(name, "m_pktclass15") == 0) {
    COMPILER_64_SET(s->m_PktClass15,0,value);
  } else if (SB_STRCMP(name, "m_byteclass15") == 0) {
    COMPILER_64_SET(s->m_ByteClass15,0,value);
  } else if (SB_STRCMP(name, "m_pktclass14") == 0) {
    COMPILER_64_SET(s->m_PktClass14,0,value);
  } else if (SB_STRCMP(name, "m_byteclass14") == 0) {
    COMPILER_64_SET(s->m_ByteClass14,0,value);
  } else if (SB_STRCMP(name, "m_pktclass13") == 0) {
    COMPILER_64_SET(s->m_PktClass13,0,value);
  } else if (SB_STRCMP(name, "m_byteclass13") == 0) {
    COMPILER_64_SET(s->m_ByteClass13,0,value);
  } else if (SB_STRCMP(name, "m_pktclass12") == 0) {
    COMPILER_64_SET(s->m_PktClass12,0,value);
  } else if (SB_STRCMP(name, "m_byteclass12") == 0) {
    COMPILER_64_SET(s->m_ByteClass12,0,value);
  } else if (SB_STRCMP(name, "m_pktclass11") == 0) {
    COMPILER_64_SET(s->m_PktClass11,0,value);
  } else if (SB_STRCMP(name, "m_byteclass11") == 0) {
    COMPILER_64_SET(s->m_ByteClass11,0,value);
  } else if (SB_STRCMP(name, "m_pktclass10") == 0) {
    COMPILER_64_SET(s->m_PktClass10,0,value);
  } else if (SB_STRCMP(name, "m_byteclass10") == 0) {
    COMPILER_64_SET(s->m_ByteClass10,0,value);
  } else if (SB_STRCMP(name, "m_pktclass9") == 0) {
    COMPILER_64_SET(s->m_PktClass9,0,value);
  } else if (SB_STRCMP(name, "m_byteclass9") == 0) {
    COMPILER_64_SET(s->m_ByteClass9,0,value);
  } else if (SB_STRCMP(name, "m_pktclass8") == 0) {
    COMPILER_64_SET(s->m_PktClass8,0,value);
  } else if (SB_STRCMP(name, "m_byteclass8") == 0) {
    COMPILER_64_SET(s->m_ByteClass8,0,value);
  } else if (SB_STRCMP(name, "m_pktclass7") == 0) {
    COMPILER_64_SET(s->m_PktClass7,0,value);
  } else if (SB_STRCMP(name, "m_byteclass7") == 0) {
    COMPILER_64_SET(s->m_ByteClass7,0,value);
  } else if (SB_STRCMP(name, "m_pktclass6") == 0) {
    COMPILER_64_SET(s->m_PktClass6,0,value);
  } else if (SB_STRCMP(name, "m_byteclass6") == 0) {
    COMPILER_64_SET(s->m_ByteClass6,0,value);
  } else if (SB_STRCMP(name, "m_pktclass5") == 0) {
    COMPILER_64_SET(s->m_PktClass5,0,value);
  } else if (SB_STRCMP(name, "m_byteclass5") == 0) {
    COMPILER_64_SET(s->m_ByteClass5,0,value);
  } else if (SB_STRCMP(name, "m_pktclass4") == 0) {
    COMPILER_64_SET(s->m_PktClass4,0,value);
  } else if (SB_STRCMP(name, "m_byteclass4") == 0) {
    COMPILER_64_SET(s->m_ByteClass4,0,value);
  } else if (SB_STRCMP(name, "m_pktclass3") == 0) {
    COMPILER_64_SET(s->m_PktClass3,0,value);
  } else if (SB_STRCMP(name, "m_byteclass3") == 0) {
    COMPILER_64_SET(s->m_ByteClass3,0,value);
  } else if (SB_STRCMP(name, "m_pktclass2") == 0) {
    COMPILER_64_SET(s->m_PktClass2,0,value);
  } else if (SB_STRCMP(name, "m_byteclass2") == 0) {
    COMPILER_64_SET(s->m_ByteClass2,0,value);
  } else if (SB_STRCMP(name, "m_pktclass1") == 0) {
    COMPILER_64_SET(s->m_PktClass1,0,value);
  } else if (SB_STRCMP(name, "m_byteclass1") == 0) {
    COMPILER_64_SET(s->m_ByteClass1,0,value);
  } else if (SB_STRCMP(name, "m_pktclass0") == 0) {
    COMPILER_64_SET(s->m_PktClass0,0,value);
  } else if (SB_STRCMP(name, "m_byteclass0") == 0) {
    COMPILER_64_SET(s->m_ByteClass0,0,value);
  } else {
    /* string failed to match any field--ignored */
    return -1;
  }

  return(0);

}
