/*
 * $Id: $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include <shared/bsl.h>

#include "sbTypes.h"
#include <soc/sbx/sbWrappers.h>
#include "sbZfFabBm9600NmPortsetInfoEntryConsole.hx"



/* Print members in struct */
void
sbZfFabBm9600NmPortsetInfoEntry_Print(sbZfFabBm9600NmPortsetInfoEntry_t *pFromStruct) {
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("FabBm9600NmPortsetInfoEntry:: virtualport=0x%01x"), (unsigned int)  pFromStruct->m_uVirtualPort));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" vporteopp=0x%01x"), (unsigned int)  pFromStruct->m_uVportEopp));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" startport=0x%02x"), (unsigned int)  pFromStruct->m_uStartPort));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("FabBm9600NmPortsetInfoEntry:: egnode=0x%02x"), (unsigned int)  pFromStruct->m_uEgNode));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

}

/* SPrint members in struct */
char *
sbZfFabBm9600NmPortsetInfoEntry_SPrint(sbZfFabBm9600NmPortsetInfoEntry_t *pFromStruct, char *pcToString, uint32 lStrSize) {
  uint32 WrCnt = 0x0;

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"FabBm9600NmPortsetInfoEntry:: virtualport=0x%01x", (unsigned int)  pFromStruct->m_uVirtualPort);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," vporteopp=0x%01x", (unsigned int)  pFromStruct->m_uVportEopp);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," startport=0x%02x", (unsigned int)  pFromStruct->m_uStartPort);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"FabBm9600NmPortsetInfoEntry:: egnode=0x%02x", (unsigned int)  pFromStruct->m_uEgNode);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  /* assert if we've overrun the buffer */
  SB_ASSERT(WrCnt < lStrSize);
  return(pcToString);

}

/* validate members in struct (1 = PASS, 0 = FAIL) */
int
sbZfFabBm9600NmPortsetInfoEntry_Validate(sbZfFabBm9600NmPortsetInfoEntry_t *pZf) {

  if (pZf->m_uVirtualPort > 0x1) return 0;
  if (pZf->m_uVportEopp > 0x7) return 0;
  if (pZf->m_uStartPort > 0xff) return 0;
  if (pZf->m_uEgNode > 0x7f) return 0;

  return 1; /* success */

}

/* populate field from string and value */
int
sbZfFabBm9600NmPortsetInfoEntry_SetField(sbZfFabBm9600NmPortsetInfoEntry_t *s, char* name, int value) {

  if (SB_STRCMP(name, "") == 0 ) {
    return -1;
  } else if (SB_STRCMP(name, "m_uvirtualport") == 0) {
    s->m_uVirtualPort = value;
  } else if (SB_STRCMP(name, "m_uvporteopp") == 0) {
    s->m_uVportEopp = value;
  } else if (SB_STRCMP(name, "m_ustartport") == 0) {
    s->m_uStartPort = value;
  } else if (SB_STRCMP(name, "m_uegnode") == 0) {
    s->m_uEgNode = value;
  } else {
    /* string failed to match any field--ignored */
    return -1;
  }

  return(0);

}
