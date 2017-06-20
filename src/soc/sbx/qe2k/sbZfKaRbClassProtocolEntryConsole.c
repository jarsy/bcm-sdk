/*
 * $Id: sbZfKaRbClassProtocolEntryConsole.c,v 1.3 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include <shared/bsl.h>

#include "sbTypes.h"
#include <soc/sbx/sbWrappers.h>
#include "sbZfKaRbClassProtocolEntryConsole.hx"



/* Print members in struct */
void
sbZfKaRbClassProtocolEntry_Print(sbZfKaRbClassProtocolEntry_t *pFromStruct) {
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("KaRbClassProtocolEntry:: protocol1usetos=0x%01x"), (unsigned int)  pFromStruct->m_nProtocol1UseTos));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" protocol1usesktinhash=0x%01x"), (unsigned int)  pFromStruct->m_nProtocol1UseSocketInHash));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("KaRbClassProtocolEntry:: protocol1dp=0x%01x"), (unsigned int)  pFromStruct->m_nProtocol1Dp));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" protocol1lsb=0x%01x"), (unsigned int)  pFromStruct->m_nProtocol1Lsb));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("KaRbClassProtocolEntry:: protocol0usetos=0x%01x"), (unsigned int)  pFromStruct->m_nProtocol0UseTos));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" protocol0usesktinhash=0x%01x"), (unsigned int)  pFromStruct->m_nProtocol0UseSocketInHash));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("KaRbClassProtocolEntry:: protocol0dp=0x%01x"), (unsigned int)  pFromStruct->m_nProtocol0Dp));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" protocol0lsb=0x%01x"), (unsigned int)  pFromStruct->m_nProtocol0Lsb));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

}

/* SPrint members in struct */
char *
sbZfKaRbClassProtocolEntry_SPrint(sbZfKaRbClassProtocolEntry_t *pFromStruct, char *pcToString, uint32 lStrSize) {
  uint32 WrCnt = 0x0;

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"KaRbClassProtocolEntry:: protocol1usetos=0x%01x", (unsigned int)  pFromStruct->m_nProtocol1UseTos);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," protocol1usesktinhash=0x%01x", (unsigned int)  pFromStruct->m_nProtocol1UseSocketInHash);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"KaRbClassProtocolEntry:: protocol1dp=0x%01x", (unsigned int)  pFromStruct->m_nProtocol1Dp);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," protocol1lsb=0x%01x", (unsigned int)  pFromStruct->m_nProtocol1Lsb);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"KaRbClassProtocolEntry:: protocol0usetos=0x%01x", (unsigned int)  pFromStruct->m_nProtocol0UseTos);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," protocol0usesktinhash=0x%01x", (unsigned int)  pFromStruct->m_nProtocol0UseSocketInHash);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"KaRbClassProtocolEntry:: protocol0dp=0x%01x", (unsigned int)  pFromStruct->m_nProtocol0Dp);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," protocol0lsb=0x%01x", (unsigned int)  pFromStruct->m_nProtocol0Lsb);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  /* assert if we've overrun the buffer */
  SB_ASSERT(WrCnt < lStrSize);
  return(pcToString);

}

/* validate members in struct (1 = PASS, 0 = FAIL) */
int
sbZfKaRbClassProtocolEntry_Validate(sbZfKaRbClassProtocolEntry_t *pZf) {

  if (pZf->m_nProtocol1UseTos > 0x1) return 0;
  if (pZf->m_nProtocol1UseSocketInHash > 0x1) return 0;
  if (pZf->m_nProtocol1Dp > 0x3) return 0;
  if (pZf->m_nProtocol1Lsb > 0xf) return 0;
  if (pZf->m_nProtocol0UseTos > 0x1) return 0;
  if (pZf->m_nProtocol0UseSocketInHash > 0x1) return 0;
  if (pZf->m_nProtocol0Dp > 0x3) return 0;
  if (pZf->m_nProtocol0Lsb > 0xf) return 0;

  return 1; /* success */

}

/* populate field from string and value */
int
sbZfKaRbClassProtocolEntry_SetField(sbZfKaRbClassProtocolEntry_t *s, char* name, int value) {

  if (SB_STRCMP(name, "") == 0 ) {
    return -1;
  } else if (SB_STRCMP(name, "m_nprotocol1usetos") == 0) {
    s->m_nProtocol1UseTos = value;
  } else if (SB_STRCMP(name, "m_nprotocol1usesocketinhash") == 0) {
    s->m_nProtocol1UseSocketInHash = value;
  } else if (SB_STRCMP(name, "m_nprotocol1dp") == 0) {
    s->m_nProtocol1Dp = value;
  } else if (SB_STRCMP(name, "m_nprotocol1lsb") == 0) {
    s->m_nProtocol1Lsb = value;
  } else if (SB_STRCMP(name, "m_nprotocol0usetos") == 0) {
    s->m_nProtocol0UseTos = value;
  } else if (SB_STRCMP(name, "m_nprotocol0usesocketinhash") == 0) {
    s->m_nProtocol0UseSocketInHash = value;
  } else if (SB_STRCMP(name, "m_nprotocol0dp") == 0) {
    s->m_nProtocol0Dp = value;
  } else if (SB_STRCMP(name, "m_nprotocol0lsb") == 0) {
    s->m_nProtocol0Lsb = value;
  } else {
    /* string failed to match any field--ignored */
    return -1;
  }

  return(0);

}
