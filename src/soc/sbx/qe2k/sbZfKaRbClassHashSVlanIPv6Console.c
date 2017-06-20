/*
 * $Id: sbZfKaRbClassHashSVlanIPv6Console.c,v 1.3 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include <shared/bsl.h>

#include "sbTypes.h"
#include <soc/sbx/sbWrappers.h>
#include "sbZfKaRbClassHashSVlanIPv6Console.hx"



/* Print members in struct */
void
sbZfKaRbClassHashSVlanIPv6_Print(sbZfKaRbClassHashSVlanIPv6_t *pFromStruct) {
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("KaRbClassHashSVlanIPv6:: protocol=0x%02x"), (unsigned int)  pFromStruct->m_nProtocol));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" flow=0x%05x"), (unsigned int)  pFromStruct->m_nFlow));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" pad2=0x%07x"),  COMPILER_64_LO(pFromStruct->m_nPadWord2)));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("KaRbClassHashSVlanIPv6:: pad1=0x%07x%08x"),  COMPILER_64_HI(pFromStruct->m_nPadWord1), COMPILER_64_LO(pFromStruct->m_nPadWord1)));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" ipsahigh=0x%08x%08x"),  COMPILER_64_HI(pFromStruct->m_nnIpSaHigh), COMPILER_64_LO(pFromStruct->m_nnIpSaHigh)));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("KaRbClassHashSVlanIPv6:: ipsalow=0x%08x%08x"),  COMPILER_64_HI(pFromStruct->m_nnIpSaLow), COMPILER_64_LO(pFromStruct->m_nnIpSaLow)));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("KaRbClassHashSVlanIPv6:: ipdahigh=0x%08x%08x"),  COMPILER_64_HI(pFromStruct->m_nnIpDaHigh), COMPILER_64_LO(pFromStruct->m_nnIpDaHigh)));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("KaRbClassHashSVlanIPv6:: ipdalow=0x%08x%08x"),  COMPILER_64_HI(pFromStruct->m_nnIpDaLow), COMPILER_64_LO(pFromStruct->m_nnIpDaLow)));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" skt=0x%08x"), (unsigned int)  pFromStruct->m_nSocket));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

}

/* SPrint members in struct */
char *
sbZfKaRbClassHashSVlanIPv6_SPrint(sbZfKaRbClassHashSVlanIPv6_t *pFromStruct, char *pcToString, uint32 lStrSize) {
  uint32 WrCnt = 0x0;

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"KaRbClassHashSVlanIPv6:: protocol=0x%02x", (unsigned int)  pFromStruct->m_nProtocol);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," flow=0x%05x", (unsigned int)  pFromStruct->m_nFlow);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pad2=0x%07x",  COMPILER_64_LO(pFromStruct->m_nPadWord2));
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"KaRbClassHashSVlanIPv6:: pad1=0x%07x%08x",  COMPILER_64_HI(pFromStruct->m_nPadWord1), COMPILER_64_LO(pFromStruct->m_nPadWord1));
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," ipsahigh=0x%08x%08x",  COMPILER_64_HI(pFromStruct->m_nnIpSaHigh), COMPILER_64_LO(pFromStruct->m_nnIpSaHigh));
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"KaRbClassHashSVlanIPv6:: ipsalow=0x%08x%08x",  COMPILER_64_HI(pFromStruct->m_nnIpSaLow), COMPILER_64_LO(pFromStruct->m_nnIpSaLow));
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"KaRbClassHashSVlanIPv6:: ipdahigh=0x%08x%08x",  COMPILER_64_HI(pFromStruct->m_nnIpDaHigh), COMPILER_64_LO(pFromStruct->m_nnIpDaHigh));
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"KaRbClassHashSVlanIPv6:: ipdalow=0x%08x%08x",  COMPILER_64_HI(pFromStruct->m_nnIpDaLow), COMPILER_64_LO(pFromStruct->m_nnIpDaLow));
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," skt=0x%08x", (unsigned int)  pFromStruct->m_nSocket);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  /* assert if we've overrun the buffer */
  SB_ASSERT(WrCnt < lStrSize);
  return(pcToString);

}

/* validate members in struct (1 = PASS, 0 = FAIL) */
int
sbZfKaRbClassHashSVlanIPv6_Validate(sbZfKaRbClassHashSVlanIPv6_t *pZf) {
  uint64 nPadWord2Max = COMPILER_64_INIT(0x00000000, 0x07ffffff),
         nPadWord1Max = COMPILER_64_INIT(0x01FFFFFF, 0xFFFFFFFF);
  if (pZf->m_nProtocol > 0xff) return 0;
  if (pZf->m_nFlow > 0xfffff) return 0;
  if (COMPILER_64_GT(pZf->m_nPadWord2, nPadWord2Max)) return 0;
  if (COMPILER_64_GT(pZf->m_nPadWord1,nPadWord1Max)) return 0;
  /* pZf->m_nnIpSaHigh implicitly masked by data type */
  /* pZf->m_nnIpSaLow implicitly masked by data type */
  /* pZf->m_nnIpDaHigh implicitly masked by data type */
  /* pZf->m_nnIpDaLow implicitly masked by data type */
  /* pZf->m_nSocket implicitly masked by data type */

  return 1; /* success */

}

/* populate field from string and value */
int
sbZfKaRbClassHashSVlanIPv6_SetField(sbZfKaRbClassHashSVlanIPv6_t *s, char* name, int value) {

  if (SB_STRCMP(name, "") == 0 ) {
    return -1;
  } else if (SB_STRCMP(name, "m_nprotocol") == 0) {
    s->m_nProtocol = value;
  } else if (SB_STRCMP(name, "m_nflow") == 0) {
    s->m_nFlow = value;
  } else if (SB_STRCMP(name, "m_npadword2") == 0) {
    COMPILER_64_SET(s->m_nPadWord2,0,value);
  } else if (SB_STRCMP(name, "m_npadword1") == 0) {
    COMPILER_64_SET(s->m_nPadWord1,0,value);
  } else if (SB_STRCMP(name, "m_nnipsahigh") == 0) {
    COMPILER_64_SET(s->m_nnIpSaHigh,0,value);
  } else if (SB_STRCMP(name, "m_nnipsalow") == 0) {
    COMPILER_64_SET(s->m_nnIpSaLow,0,value);
  } else if (SB_STRCMP(name, "m_nnipdahigh") == 0) {
    COMPILER_64_SET(s->m_nnIpDaHigh,0,value);
  } else if (SB_STRCMP(name, "m_nnipdalow") == 0) {
    COMPILER_64_SET(s->m_nnIpDaLow,0,value);
  } else if (SB_STRCMP(name, "m_nsocket") == 0) {
    s->m_nSocket = value;
  } else {
    /* string failed to match any field--ignored */
    return -1;
  }

  return(0);

}
