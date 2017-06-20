/*
 * $Id: sbZfSbQe2000ElibPCTSingleConsole.c,v 1.5 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include <shared/bsl.h>

#include "sbTypesGlue.h"
#include <soc/sbx/sbWrappers.h>
#include "sbZfSbQe2000ElibPCTSingleConsole.hx"



/* Print members in struct */
void
sbZfSbQe2000ElibPCTSingle_Print(sbZfSbQe2000ElibPCTSingle_t *pFromStruct) {
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("SbQe2000ElibPCTSingle:: pktcnt=0x%08x"),   COMPILER_64_LO(pFromStruct->m_PktClass)));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" bytecnt=0x%01x%08x"),   COMPILER_64_HI(pFromStruct->m_ByteClass), COMPILER_64_LO(pFromStruct->m_ByteClass)));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

}

/* SPrint members in struct */
char *
sbZfSbQe2000ElibPCTSingle_SPrint(sbZfSbQe2000ElibPCTSingle_t *pFromStruct, char *pcToString, uint32 lStrSize) {
  uint32 WrCnt = 0x0;

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"SbQe2000ElibPCTSingle:: pktcnt=0x%08x", COMPILER_64_LO(pFromStruct->m_PktClass));
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," bytecnt=0x%01x%08x",  COMPILER_64_HI(pFromStruct->m_ByteClass), COMPILER_64_LO(pFromStruct->m_ByteClass));
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  /* assert if we've overrun the buffer */
  SB_ASSERT(WrCnt < lStrSize);
  return(pcToString);

}

/* validate members in struct (1 = PASS, 0 = FAIL) */
int
sbZfSbQe2000ElibPCTSingle_Validate(sbZfSbQe2000ElibPCTSingle_t *pZf) {
  uint64 PktClassMax  = COMPILER_64_INIT(0x0, 0x1fffffff), 
         ByteClassMax = COMPILER_64_INIT(0x7, 0xFFFFFFFF);
  if (COMPILER_64_GT(pZf->m_PktClass, PktClassMax)) return 0;
  if (COMPILER_64_GT(pZf->m_ByteClass, ByteClassMax)) return 0;

  return 1; /* success */

}

/* populate field from string and value */
int
sbZfSbQe2000ElibPCTSingle_SetField(sbZfSbQe2000ElibPCTSingle_t *s, char* name, int value) {

  if (SB_STRCMP(name, "") == 0 ) {
    return -1;
  } else if (SB_STRCMP(name, "m_pktclass") == 0) {
    COMPILER_64_SET(s->m_PktClass, 0, value);
  } else if (SB_STRCMP(name, "m_byteclass") == 0) {
    COMPILER_64_SET(s->m_ByteClass,0,value);
  } else {
    /* string failed to match any field--ignored */
    return -1;
  }

  return(0);

}
