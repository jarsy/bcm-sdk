/*
 * $Id: sbZfSbQe2000ElibFMVTConsole.c,v 1.5 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include <shared/bsl.h>

#include "sbTypesGlue.h"
#include <soc/sbx/sbWrappers.h>
#include "sbZfSbQe2000ElibFMVTConsole.hx"



/* Print members in struct */
void
sbZfSbQe2000ElibFMVT_Print(sbZfSbQe2000ElibFMVT_t *pFromStruct) {
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("SbQe2000ElibFMVT:: rsvd=0x%01x"), (unsigned int)  pFromStruct->m_nReserved));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" port_map2=0x%05x%08x"),  COMPILER_64_HI(pFromStruct->m_nnPortMap2), COMPILER_64_LO(pFromStruct->m_nnPortMap2)));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" mvtd_a2=0x%04x"), (unsigned int)  pFromStruct->m_nMvtda2));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("SbQe2000ElibFMVT:: mvtd_b2=0x%01x"), (unsigned int)  pFromStruct->m_nMvtdb2));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" next2=0x%04x"), (unsigned int)  pFromStruct->m_nNext2));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" knockout2=0x%01x"), (unsigned int)  pFromStruct->m_nKnockout2));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("SbQe2000ElibFMVT:: port_map1=0x%05x%08x"),  COMPILER_64_HI(pFromStruct->m_nnPortMap1), COMPILER_64_LO(pFromStruct->m_nnPortMap1)));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" mvtd_a1=0x%04x"), (unsigned int)  pFromStruct->m_nMvtda1));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" mvtd_b1=0x%01x"), (unsigned int)  pFromStruct->m_nMvtdb1));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("SbQe2000ElibFMVT:: next1=0x%04x"), (unsigned int)  pFromStruct->m_nNext1));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" knockout1=0x%01x"), (unsigned int)  pFromStruct->m_nKnockout1));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" port_map0=0x%05x%08x"),  COMPILER_64_HI(pFromStruct->m_nnPortMap0), COMPILER_64_LO(pFromStruct->m_nnPortMap0)));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("SbQe2000ElibFMVT:: mvtd_a0=0x%04x"), (unsigned int)  pFromStruct->m_nMvtda0));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" mvtd_b0=0x%01x"), (unsigned int)  pFromStruct->m_nMvtdb0));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" next0=0x%04x"), (unsigned int)  pFromStruct->m_nNext0));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" knockout0=0x%01x"), (unsigned int)  pFromStruct->m_nKnockout0));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

}

/* SPrint members in struct */
char *
sbZfSbQe2000ElibFMVT_SPrint(sbZfSbQe2000ElibFMVT_t *pFromStruct, char *pcToString, uint32 lStrSize) {
  uint32 WrCnt = 0x0;

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"SbQe2000ElibFMVT:: rsvd=0x%01x", (unsigned int)  pFromStruct->m_nReserved);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," port_map2=0x%05x%08x",  COMPILER_64_HI(pFromStruct->m_nnPortMap2), COMPILER_64_LO(pFromStruct->m_nnPortMap2));
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," mvtd_a2=0x%04x", (unsigned int)  pFromStruct->m_nMvtda2);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"SbQe2000ElibFMVT:: mvtd_b2=0x%01x", (unsigned int)  pFromStruct->m_nMvtdb2);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," next2=0x%04x", (unsigned int)  pFromStruct->m_nNext2);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," knockout2=0x%01x", (unsigned int)  pFromStruct->m_nKnockout2);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"SbQe2000ElibFMVT:: port_map1=0x%05x%08x",  COMPILER_64_HI(pFromStruct->m_nnPortMap1), COMPILER_64_LO(pFromStruct->m_nnPortMap1));
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," mvtd_a1=0x%04x", (unsigned int)  pFromStruct->m_nMvtda1);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," mvtd_b1=0x%01x", (unsigned int)  pFromStruct->m_nMvtdb1);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"SbQe2000ElibFMVT:: next1=0x%04x", (unsigned int)  pFromStruct->m_nNext1);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," knockout1=0x%01x", (unsigned int)  pFromStruct->m_nKnockout1);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," port_map0=0x%05x%08x",  COMPILER_64_HI(pFromStruct->m_nnPortMap0), COMPILER_64_LO(pFromStruct->m_nnPortMap0));
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"SbQe2000ElibFMVT:: mvtd_a0=0x%04x", (unsigned int)  pFromStruct->m_nMvtda0);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," mvtd_b0=0x%01x", (unsigned int)  pFromStruct->m_nMvtdb0);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," next0=0x%04x", (unsigned int)  pFromStruct->m_nNext0);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," knockout0=0x%01x", (unsigned int)  pFromStruct->m_nKnockout0);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  /* assert if we've overrun the buffer */
  SB_ASSERT(WrCnt < lStrSize);
  return(pcToString);

}

/* validate members in struct (1 = PASS, 0 = FAIL) */
int
sbZfSbQe2000ElibFMVT_Validate(sbZfSbQe2000ElibFMVT_t *pZf) {
  uint64 nnPortMapMax = COMPILER_64_INIT(0x3FFFF, 0xFFFFFFFF);
  if (pZf->m_nReserved > 0x1) return 0;
  if (COMPILER_64_GT(pZf->m_nnPortMap2, nnPortMapMax)) return 0;
  if (pZf->m_nMvtda2 > 0x3fff) return 0;
  if (pZf->m_nMvtdb2 > 0xf) return 0;
  if (pZf->m_nNext2 > 0xffff) return 0;
  if (pZf->m_nKnockout2 > 0x1) return 0;
  if (COMPILER_64_GT(pZf->m_nnPortMap1, nnPortMapMax)) return 0;
  if (pZf->m_nMvtda1 > 0x3fff) return 0;
  if (pZf->m_nMvtdb1 > 0xf) return 0;
  if (pZf->m_nNext1 > 0xffff) return 0;
  if (pZf->m_nKnockout1 > 0x1) return 0;
  if (COMPILER_64_GT(pZf->m_nnPortMap0, nnPortMapMax)) return 0;
  if (pZf->m_nMvtda0 > 0x3fff) return 0;
  if (pZf->m_nMvtdb0 > 0xf) return 0;
  if (pZf->m_nNext0 > 0xffff) return 0;
  if (pZf->m_nKnockout0 > 0x1) return 0;

  return 1; /* success */

}

/* populate field from string and value */
int
sbZfSbQe2000ElibFMVT_SetField(sbZfSbQe2000ElibFMVT_t *s, char* name, int value) {

  if (SB_STRCMP(name, "") == 0 ) {
    return -1;
  } else if (SB_STRCMP(name, "m_nreserved") == 0) {
    s->m_nReserved = value;
  } else if (SB_STRCMP(name, "m_nnportmap2") == 0) {
    COMPILER_64_SET(s->m_nnPortMap2,0, value);
  } else if (SB_STRCMP(name, "m_nmvtda2") == 0) {
    s->m_nMvtda2 = value;
  } else if (SB_STRCMP(name, "m_nmvtdb2") == 0) {
    s->m_nMvtdb2 = value;
  } else if (SB_STRCMP(name, "m_nnext2") == 0) {
    s->m_nNext2 = value;
  } else if (SB_STRCMP(name, "m_nknockout2") == 0) {
    s->m_nKnockout2 = value;
  } else if (SB_STRCMP(name, "m_nnportmap1") == 0) {
    COMPILER_64_SET(s->m_nnPortMap1,0,value);
  } else if (SB_STRCMP(name, "m_nmvtda1") == 0) {
    s->m_nMvtda1 = value;
  } else if (SB_STRCMP(name, "m_nmvtdb1") == 0) {
    s->m_nMvtdb1 = value;
  } else if (SB_STRCMP(name, "m_nnext1") == 0) {
    s->m_nNext1 = value;
  } else if (SB_STRCMP(name, "m_nknockout1") == 0) {
    s->m_nKnockout1 = value;
  } else if (SB_STRCMP(name, "m_nnportmap0") == 0) {
    COMPILER_64_SET(s->m_nnPortMap0,0,value);
  } else if (SB_STRCMP(name, "m_nmvtda0") == 0) {
    s->m_nMvtda0 = value;
  } else if (SB_STRCMP(name, "m_nmvtdb0") == 0) {
    s->m_nMvtdb0 = value;
  } else if (SB_STRCMP(name, "m_nnext0") == 0) {
    s->m_nNext0 = value;
  } else if (SB_STRCMP(name, "m_nknockout0") == 0) {
    s->m_nKnockout0 = value;
  } else {
    /* string failed to match any field--ignored */
    return -1;
  }

  return(0);

}
