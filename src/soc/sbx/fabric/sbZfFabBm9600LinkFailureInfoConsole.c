/*
 * $Id: sbZfFabBm9600LinkFailureInfoConsole.c,v 1.4 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include <shared/bsl.h>

#include "sbTypes.h"
#include <soc/sbx/sbWrappers.h>
#include "sbZfFabBm9600LinkFailureInfoConsole.hx"



/* Print members in struct */
void
sbZfFabBm9600LinkFailureInfo_Print(sbZfFabBm9600LinkFailureInfo_t *pFromStruct) {
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("FabBm9600LinkFailureInfo:: qelinkstate=0x%02x"), (unsigned int)  pFromStruct->m_bQeLinkState));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" expectinglinkerror=0x%01x"), (unsigned int)  pFromStruct->m_bExpectingLinkError));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("FabBm9600LinkFailureInfo:: digestscanstart=0x%08x%08x"), COMPILER_64_HI(pFromStruct->m_uDigestScanStartTime), COMPILER_64_HI(pFromStruct->m_uDigestScanStartTime)));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("FabBm9600LinkFailureInfo:: digestscanend=0x%08x%08x"),  COMPILER_64_HI(pFromStruct->m_uDigestScanEndTime), COMPILER_64_LO(pFromStruct->m_uDigestScanEndTime)));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("FabBm9600LinkFailureInfo:: dutbaseaddress=0x%08x"), (unsigned int)  pFromStruct->m_uDUTBaseAddress));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" activebmemode=0x%01x"), (unsigned int)  pFromStruct->m_uActiveBmeFoMode));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("FabBm9600LinkFailureInfo:: activebmescilink=0x%01x"), (unsigned int)  pFromStruct->m_uActiveBmeSciLink));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("FabBm9600LinkFailureInfo:: expectedlinkstate=0x%08x"), (unsigned int)  pFromStruct->m_uExpectedGlobalLinkState));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" inanumber=0x%02x"), (unsigned int)  pFromStruct->m_uInaNumber));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

}

/* SPrint members in struct */
char *
sbZfFabBm9600LinkFailureInfo_SPrint(sbZfFabBm9600LinkFailureInfo_t *pFromStruct, char *pcToString, uint32 lStrSize) {
  uint32 WrCnt = 0x0;

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"FabBm9600LinkFailureInfo:: qelinkstate=0x%02x", (unsigned int)  pFromStruct->m_bQeLinkState);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," expectinglinkerror=0x%01x", (unsigned int)  pFromStruct->m_bExpectingLinkError);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"FabBm9600LinkFailureInfo:: digestscanstart=0x%08x%08x",  COMPILER_64_HI(pFromStruct->m_uDigestScanStartTime),COMPILER_64_LO(pFromStruct->m_uDigestScanStartTime));
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"FabBm9600LinkFailureInfo:: digestscanend=0x%08x%08x",  COMPILER_64_HI(pFromStruct->m_uDigestScanEndTime),COMPILER_64_LO(pFromStruct->m_uDigestScanEndTime));
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"FabBm9600LinkFailureInfo:: dutbaseaddress=0x%08x", (unsigned int)  pFromStruct->m_uDUTBaseAddress);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," activebmemode=0x%01x", (unsigned int)  pFromStruct->m_uActiveBmeFoMode);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"FabBm9600LinkFailureInfo:: activebmescilink=0x%01x", (unsigned int)  pFromStruct->m_uActiveBmeSciLink);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"FabBm9600LinkFailureInfo:: expectedlinkstate=0x%08x", (unsigned int)  pFromStruct->m_uExpectedGlobalLinkState);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," inanumber=0x%02x", (unsigned int)  pFromStruct->m_uInaNumber);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  /* assert if we've overrun the buffer */
  SB_ASSERT(WrCnt < lStrSize);
  return(pcToString);

}

/* validate members in struct (1 = PASS, 0 = FAIL) */
int
sbZfFabBm9600LinkFailureInfo_Validate(sbZfFabBm9600LinkFailureInfo_t *pZf) {

  if (pZf->m_bQeLinkState > 0x1f) return 0;
  if (pZf->m_bExpectingLinkError > 0x1) return 0;
  /* pZf->m_uDigestScanStartTime implicitly masked by data type */
  /* pZf->m_uDigestScanEndTime implicitly masked by data type */
  /* pZf->m_uDUTBaseAddress implicitly masked by data type */
  if (pZf->m_uActiveBmeFoMode > 0x3) return 0;
  if (pZf->m_uActiveBmeSciLink > 0x1) return 0;
  /* pZf->m_uExpectedGlobalLinkState implicitly masked by data type */
  if (pZf->m_uInaNumber > 0x7f) return 0;

  return 1; /* success */

}

/* populate field from string and value */
int
sbZfFabBm9600LinkFailureInfo_SetField(sbZfFabBm9600LinkFailureInfo_t *s, char* name, int value) {

  if (SB_STRCMP(name, "") == 0 ) {
    return -1;
  } else if (SB_STRCMP(name, "qelinkstate") == 0) {
    s->m_bQeLinkState = value;
  } else if (SB_STRCMP(name, "expectinglinkerror") == 0) {
    s->m_bExpectingLinkError = value;
  } else if (SB_STRCMP(name, "m_udigestscanstarttime") == 0) {
    COMPILER_64_SET(s->m_uDigestScanStartTime, 0, value);
  } else if (SB_STRCMP(name, "m_udigestscanendtime") == 0) {
    COMPILER_64_SET(s->m_uDigestScanEndTime, 0, value);
  } else if (SB_STRCMP(name, "m_udutbaseaddress") == 0) {
    s->m_uDUTBaseAddress = value;
  } else if (SB_STRCMP(name, "m_uactivebmefomode") == 0) {
    s->m_uActiveBmeFoMode = value;
  } else if (SB_STRCMP(name, "m_uactivebmescilink") == 0) {
    s->m_uActiveBmeSciLink = value;
  } else if (SB_STRCMP(name, "m_uexpectedgloballinkstate") == 0) {
    s->m_uExpectedGlobalLinkState = value;
  } else if (SB_STRCMP(name, "m_uinanumber") == 0) {
    s->m_uInaNumber = value;
  } else {
    /* string failed to match any field--ignored */
    return -1;
  }

  return(0);

}
