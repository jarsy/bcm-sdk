/*
 * $Id: sbZfKaRbClassHashVlanIPv4Console.c,v 1.3 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include <shared/bsl.h>

#include "sbTypes.h"
#include <soc/sbx/sbWrappers.h>
#include "sbZfKaRbClassHashVlanIPv4Console.hx"



/* Print members in struct */
void
sbZfKaRbClassHashVlanIPv4_Print(sbZfKaRbClassHashVlanIPv4_t *pFromStruct) {
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("KaRbClassHashVlanIPv4:: protocol=0x%02x"), (unsigned int)  pFromStruct->m_nProtocol));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" pad3=0x%03x%08x"),  COMPILER_64_HI(pFromStruct->m_nPadWord3), COMPILER_64_LO(pFromStruct->m_nPadWord3)));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("KaRbClassHashVlanIPv4:: pad1=0x%08x%08x"),  COMPILER_64_HI(pFromStruct->m_nPadWord1), COMPILER_64_LO(pFromStruct->m_nPadWord1)));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" ipsa=0x%08x"), (unsigned int)  pFromStruct->m_nIpSa));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("KaRbClassHashVlanIPv4:: ipda=0x%08x"), (unsigned int)  pFromStruct->m_nIpDa));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" skt=0x%08x"), (unsigned int)  pFromStruct->m_nSocket));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" pad2=0x%04x%08x"),  COMPILER_64_HI(pFromStruct->m_nPadWord2), COMPILER_64_LO(pFromStruct->m_nPadWord2)));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("KaRbClassHashVlanIPv4:: word1=0x%08x%08x"),  COMPILER_64_HI(pFromStruct->m_nSpareWord1), COMPILER_64_LO(pFromStruct->m_nSpareWord1)));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" word0=0x%08x%08x"),  COMPILER_64_HI(pFromStruct->m_nSpareWord0), COMPILER_64_LO(pFromStruct->m_nSpareWord0)));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

}

/* SPrint members in struct */
char *
sbZfKaRbClassHashVlanIPv4_SPrint(sbZfKaRbClassHashVlanIPv4_t *pFromStruct, char *pcToString, uint32 lStrSize) {
  uint32 WrCnt = 0x0;

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"KaRbClassHashVlanIPv4:: protocol=0x%02x", (unsigned int)  pFromStruct->m_nProtocol);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pad3=0x%03x%08x",  COMPILER_64_HI(pFromStruct->m_nPadWord3), COMPILER_64_LO(pFromStruct->m_nPadWord3));
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"KaRbClassHashVlanIPv4:: pad1=0x%08x%08x",  COMPILER_64_HI(pFromStruct->m_nPadWord1), COMPILER_64_LO(pFromStruct->m_nPadWord1));
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," ipsa=0x%08x", (unsigned int)  pFromStruct->m_nIpSa);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"KaRbClassHashVlanIPv4:: ipda=0x%08x", (unsigned int)  pFromStruct->m_nIpDa);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," skt=0x%08x", (unsigned int)  pFromStruct->m_nSocket);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pad2=0x%04x%08x",  COMPILER_64_HI(pFromStruct->m_nPadWord2), COMPILER_64_LO(pFromStruct->m_nPadWord2));
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"KaRbClassHashVlanIPv4:: word1=0x%08x%08x",  COMPILER_64_HI(pFromStruct->m_nSpareWord1), COMPILER_64_LO(pFromStruct->m_nSpareWord1));
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," word0=0x%08x%08x",  COMPILER_64_HI(pFromStruct->m_nSpareWord0), COMPILER_64_LO(pFromStruct->m_nSpareWord0));
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  /* assert if we've overrun the buffer */
  SB_ASSERT(WrCnt < lStrSize);
  return(pcToString);

}

/* validate members in struct (1 = PASS, 0 = FAIL) */
int
sbZfKaRbClassHashVlanIPv4_Validate(sbZfKaRbClassHashVlanIPv4_t *pZf) {
  uint64 nPadWord3Max = COMPILER_64_INIT(0x000001FF, 0xFFFFFFFF),
         nPadWord1Max = COMPILER_64_INIT(0x7FFFFFFF, 0xFFFFFFFF), 
         nPadWord2Max = COMPILER_64_INIT(0x0000FFFF, 0xFFFFFFFF);
  if (pZf->m_nProtocol > 0xff) return 0;
  if (COMPILER_64_GT(pZf->m_nPadWord3, nPadWord3Max)) return 0;
  if (COMPILER_64_GT(pZf->m_nPadWord1, nPadWord1Max)) return 0;
  /* pZf->m_nIpSa implicitly masked by data type */
  /* pZf->m_nIpDa implicitly masked by data type */
  /* pZf->m_nSocket implicitly masked by data type */
  if (COMPILER_64_GT(pZf->m_nPadWord2, nPadWord2Max)) return 0;
  /* pZf->m_nSpareWord1 implicitly masked by data type */
  /* pZf->m_nSpareWord0 implicitly masked by data type */

  return 1; /* success */

}

/* populate field from string and value */
int
sbZfKaRbClassHashVlanIPv4_SetField(sbZfKaRbClassHashVlanIPv4_t *s, char* name, int value) {

  if (SB_STRCMP(name, "") == 0 ) {
    return -1;
  } else if (SB_STRCMP(name, "m_nprotocol") == 0) {
    s->m_nProtocol = value;
  } else if (SB_STRCMP(name, "m_npadword3") == 0) {
    COMPILER_64_SET(s->m_nPadWord3,0,value);
  } else if (SB_STRCMP(name, "m_npadword1") == 0) {
    COMPILER_64_SET(s->m_nPadWord1,0,value);
  } else if (SB_STRCMP(name, "m_nipsa") == 0) {
    s->m_nIpSa = value;
  } else if (SB_STRCMP(name, "m_nipda") == 0) {
    s->m_nIpDa = value;
  } else if (SB_STRCMP(name, "m_nsocket") == 0) {
    s->m_nSocket = value;
  } else if (SB_STRCMP(name, "m_npadword2") == 0) {
    COMPILER_64_SET(s->m_nPadWord2,0,value);
  } else if (SB_STRCMP(name, "m_nspareword1") == 0) {
    COMPILER_64_SET(s->m_nSpareWord1,0,value);
  } else if (SB_STRCMP(name, "m_nspareword0") == 0) {
    COMPILER_64_SET(s->m_nSpareWord0,0,value);
  } else {
    /* string failed to match any field--ignored */
    return -1;
  }

  return(0);

}
