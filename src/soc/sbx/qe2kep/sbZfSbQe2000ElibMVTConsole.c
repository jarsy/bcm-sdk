/*
 * $Id: sbZfSbQe2000ElibMVTConsole.c,v 1.5 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include <shared/bsl.h>

#include "sbTypesGlue.h"
#include <soc/sbx/sbWrappers.h>
#include "sbZfSbQe2000ElibMVTConsole.hx"



/* Print members in struct */
void
sbZfSbQe2000ElibMVT_Print(sbZfSbQe2000ElibMVT_t *pFromStruct) {
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("SbQe2000ElibMVT:: reserved=0x%01x"), (unsigned int)  pFromStruct->m_nReserved));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" port_map=0x%05x%08x"),  COMPILER_64_HI(pFromStruct->m_nnPortMap), COMPILER_64_LO(pFromStruct->m_nnPortMap)));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" mvtd_a=0x%04x"), (unsigned int)  pFromStruct->m_nMvtda));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("SbQe2000ElibMVT:: mvtd_b=0x%01x"), (unsigned int)  pFromStruct->m_nMvtdb));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" next=0x%04x"), (unsigned int)  pFromStruct->m_nNext));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" knockout=0x%01x"), (unsigned int)  pFromStruct->m_nKnockout));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

}

/* SPrint members in struct */
char *
sbZfSbQe2000ElibMVT_SPrint(sbZfSbQe2000ElibMVT_t *pFromStruct, char *pcToString, uint32 lStrSize) {
  uint32 WrCnt = 0x0;

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"SbQe2000ElibMVT:: reserved=0x%01x", (unsigned int)  pFromStruct->m_nReserved);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," port_map=0x%05x%08x",   COMPILER_64_HI(pFromStruct->m_nnPortMap), COMPILER_64_LO(pFromStruct->m_nnPortMap));
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," mvtd_a=0x%04x", (unsigned int)  pFromStruct->m_nMvtda);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"SbQe2000ElibMVT:: mvtd_b=0x%01x", (unsigned int)  pFromStruct->m_nMvtdb);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," next=0x%04x", (unsigned int)  pFromStruct->m_nNext);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," knockout=0x%01x", (unsigned int)  pFromStruct->m_nKnockout);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  /* assert if we've overrun the buffer */
  SB_ASSERT(WrCnt < lStrSize);
  return(pcToString);

}

/* validate members in struct (1 = PASS, 0 = FAIL) */
int
sbZfSbQe2000ElibMVT_Validate(sbZfSbQe2000ElibMVT_t *pZf) {
  uint64 nnPortMapMax = COMPILER_64_INIT(0x3FFFF, 0xFFFFFFFF); 
  if (pZf->m_nReserved > 0x7) return 0;
  if (COMPILER_64_GT(pZf->m_nnPortMap,nnPortMapMax)) return 0;
  if (pZf->m_nMvtda > 0x3fff) return 0;
  if (pZf->m_nMvtdb > 0xf) return 0;
  if (pZf->m_nNext > 0xffff) return 0;
  if (pZf->m_nKnockout > 0x1) return 0;

  return 1; /* success */

}

/* populate field from string and value */
int
sbZfSbQe2000ElibMVT_SetField(sbZfSbQe2000ElibMVT_t *s, char* name, int value) {

  if (SB_STRCMP(name, "") == 0 ) {
    return -1;
  } else if (SB_STRCMP(name, "m_nreserved") == 0) {
    s->m_nReserved = value;
  } else if (SB_STRCMP(name, "m_nnportmap") == 0) {
    COMPILER_64_SET(s->m_nnPortMap,0,value);
  } else if (SB_STRCMP(name, "m_nmvtda") == 0) {
    s->m_nMvtda = value;
  } else if (SB_STRCMP(name, "m_nmvtdb") == 0) {
    s->m_nMvtdb = value;
  } else if (SB_STRCMP(name, "m_nnext") == 0) {
    s->m_nNext = value;
  } else if (SB_STRCMP(name, "m_nknockout") == 0) {
    s->m_nKnockout = value;
  } else {
    /* string failed to match any field--ignored */
    return -1;
  }

  return(0);

}
