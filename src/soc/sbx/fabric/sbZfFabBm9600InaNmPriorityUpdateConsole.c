/*
 * $Id: $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include <shared/bsl.h>

#include "sbTypes.h"
#include <soc/sbx/sbWrappers.h>
#include "sbZfFabBm9600InaNmPriorityUpdateConsole.hx"



/* Print members in struct */
void
sbZfFabBm9600InaNmPriorityUpdate_Print(sbZfFabBm9600InaNmPriorityUpdate_t *pFromStruct) {
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("FabBm9600InaNmPriorityUpdate:: ina=0x%02x"), (unsigned int)  pFromStruct->m_uIna));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" sysport=0x%03x"), (unsigned int)  pFromStruct->m_uSystemPort));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" eset=0x%03x"), (unsigned int)  pFromStruct->m_uEset));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("FabBm9600InaNmPriorityUpdate:: portaddr=0x%02x"), (unsigned int)  pFromStruct->m_uPortSetAddress));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" portoffs=0x%01x"), (unsigned int)  pFromStruct->m_uPortSetOffset));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" ncu=0x%01x"), (unsigned int)  pFromStruct->m_bNoCriticalUpdate));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" cu=0x%01x"), (unsigned int)  pFromStruct->m_bCriticalUpdate));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("FabBm9600InaNmPriorityUpdate:: mc=0x%01x"), (unsigned int)  pFromStruct->m_bMulticast));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" pri=0x%01x"), (unsigned int)  pFromStruct->m_uPriority));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" maxpri=0x%01x"), (unsigned int)  pFromStruct->m_bMaxPriority));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" nxtpri=0x%01x"), (unsigned int)  pFromStruct->m_uNextPriority));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("FabBm9600InaNmPriorityUpdate:: nxtmax=0x%01x"), (unsigned int)  pFromStruct->m_bNextMaxPriority));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

}

/* SPrint members in struct */
char *
sbZfFabBm9600InaNmPriorityUpdate_SPrint(sbZfFabBm9600InaNmPriorityUpdate_t *pFromStruct, char *pcToString, uint32 lStrSize) {
  uint32 WrCnt = 0x0;

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"FabBm9600InaNmPriorityUpdate:: ina=0x%02x", (unsigned int)  pFromStruct->m_uIna);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," sysport=0x%03x", (unsigned int)  pFromStruct->m_uSystemPort);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," eset=0x%03x", (unsigned int)  pFromStruct->m_uEset);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"FabBm9600InaNmPriorityUpdate:: portaddr=0x%02x", (unsigned int)  pFromStruct->m_uPortSetAddress);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," portoffs=0x%01x", (unsigned int)  pFromStruct->m_uPortSetOffset);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," ncu=0x%01x", (unsigned int)  pFromStruct->m_bNoCriticalUpdate);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," cu=0x%01x", (unsigned int)  pFromStruct->m_bCriticalUpdate);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"FabBm9600InaNmPriorityUpdate:: mc=0x%01x", (unsigned int)  pFromStruct->m_bMulticast);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pri=0x%01x", (unsigned int)  pFromStruct->m_uPriority);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," maxpri=0x%01x", (unsigned int)  pFromStruct->m_bMaxPriority);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," nxtpri=0x%01x", (unsigned int)  pFromStruct->m_uNextPriority);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"FabBm9600InaNmPriorityUpdate:: nxtmax=0x%01x", (unsigned int)  pFromStruct->m_bNextMaxPriority);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  /* assert if we've overrun the buffer */
  SB_ASSERT(WrCnt < lStrSize);
  return(pcToString);

}

/* validate members in struct (1 = PASS, 0 = FAIL) */
int
sbZfFabBm9600InaNmPriorityUpdate_Validate(sbZfFabBm9600InaNmPriorityUpdate_t *pZf) {

  if (pZf->m_uIna > 0x7f) return 0;
  if (pZf->m_uSystemPort > 0xfff) return 0;
  if (pZf->m_uEset > 0x3ff) return 0;
  if (pZf->m_uPortSetAddress > 0xff) return 0;
  if (pZf->m_uPortSetOffset > 0xf) return 0;
  if (pZf->m_bNoCriticalUpdate > 0x1) return 0;
  if (pZf->m_bCriticalUpdate > 0x1) return 0;
  if (pZf->m_bMulticast > 0x1) return 0;
  if (pZf->m_uPriority > 0xf) return 0;
  if (pZf->m_bMaxPriority > 0x1) return 0;
  if (pZf->m_uNextPriority > 0xf) return 0;
  if (pZf->m_bNextMaxPriority > 0x1) return 0;

  return 1; /* success */

}

/* populate field from string and value */
int
sbZfFabBm9600InaNmPriorityUpdate_SetField(sbZfFabBm9600InaNmPriorityUpdate_t *s, char* name, int value) {

  if (SB_STRCMP(name, "") == 0 ) {
    return -1;
  } else if (SB_STRCMP(name, "m_uina") == 0) {
    s->m_uIna = value;
  } else if (SB_STRCMP(name, "m_usystemport") == 0) {
    s->m_uSystemPort = value;
  } else if (SB_STRCMP(name, "m_ueset") == 0) {
    s->m_uEset = value;
  } else if (SB_STRCMP(name, "m_uportsetaddress") == 0) {
    s->m_uPortSetAddress = value;
  } else if (SB_STRCMP(name, "m_uportsetoffset") == 0) {
    s->m_uPortSetOffset = value;
  } else if (SB_STRCMP(name, "nocriticalupdate") == 0) {
    s->m_bNoCriticalUpdate = value;
  } else if (SB_STRCMP(name, "criticalupdate") == 0) {
    s->m_bCriticalUpdate = value;
  } else if (SB_STRCMP(name, "multicast") == 0) {
    s->m_bMulticast = value;
  } else if (SB_STRCMP(name, "m_upriority") == 0) {
    s->m_uPriority = value;
  } else if (SB_STRCMP(name, "maxpriority") == 0) {
    s->m_bMaxPriority = value;
  } else if (SB_STRCMP(name, "m_unextpriority") == 0) {
    s->m_uNextPriority = value;
  } else if (SB_STRCMP(name, "nextmaxpriority") == 0) {
    s->m_bNextMaxPriority = value;
  } else {
    /* string failed to match any field--ignored */
    return -1;
  }

  return(0);

}
