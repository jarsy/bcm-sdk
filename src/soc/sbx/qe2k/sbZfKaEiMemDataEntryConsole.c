/*
 * $Id: sbZfKaEiMemDataEntryConsole.c,v 1.3 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include <shared/bsl.h>

#include "sbTypes.h"
#include <soc/sbx/sbWrappers.h>
#include "sbZfKaEiMemDataEntryConsole.hx"



/* Print members in struct */
void
sbZfKaEiMemDataEntry_Print(sbZfKaEiMemDataEntry_t *pFromStruct) {
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("KaEiMemDataEntry:: destchannel=0x%02x"), (unsigned int)  pFromStruct->m_nDestChannel));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" sizemask=0x%03x"), (unsigned int)  pFromStruct->m_nSizeMask));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" rb_only=0x%01x"), (unsigned int)  pFromStruct->m_nRbOnly));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("KaEiMemDataEntry:: lineptr=0x%03x"), (unsigned int)  pFromStruct->m_nLinePtr));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" byteptr=0x%01x"), (unsigned int)  pFromStruct->m_nBytePtr));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

}

/* SPrint members in struct */
char *
sbZfKaEiMemDataEntry_SPrint(sbZfKaEiMemDataEntry_t *pFromStruct, char *pcToString, uint32 lStrSize) {
  uint32 WrCnt = 0x0;

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"KaEiMemDataEntry:: destchannel=0x%02x", (unsigned int)  pFromStruct->m_nDestChannel);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," sizemask=0x%03x", (unsigned int)  pFromStruct->m_nSizeMask);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," rb_only=0x%01x", (unsigned int)  pFromStruct->m_nRbOnly);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"KaEiMemDataEntry:: lineptr=0x%03x", (unsigned int)  pFromStruct->m_nLinePtr);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," byteptr=0x%01x", (unsigned int)  pFromStruct->m_nBytePtr);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  /* assert if we've overrun the buffer */
  SB_ASSERT(WrCnt < lStrSize);
  return(pcToString);

}

/* validate members in struct (1 = PASS, 0 = FAIL) */
int
sbZfKaEiMemDataEntry_Validate(sbZfKaEiMemDataEntry_t *pZf) {

  if (pZf->m_nDestChannel > 0xff) return 0;
  if (pZf->m_nSizeMask > 0x1ff) return 0;
  if (pZf->m_nRbOnly > 0x1) return 0;
  if (pZf->m_nLinePtr > 0x3ff) return 0;
  if (pZf->m_nBytePtr > 0xf) return 0;

  return 1; /* success */

}

/* populate field from string and value */
int
sbZfKaEiMemDataEntry_SetField(sbZfKaEiMemDataEntry_t *s, char* name, int value) {

  if (SB_STRCMP(name, "") == 0 ) {
    return -1;
  } else if (SB_STRCMP(name, "m_ndestchannel") == 0) {
    s->m_nDestChannel = value;
  } else if (SB_STRCMP(name, "m_nsizemask") == 0) {
    s->m_nSizeMask = value;
  } else if (SB_STRCMP(name, "m_nrbonly") == 0) {
    s->m_nRbOnly = value;
  } else if (SB_STRCMP(name, "m_nlineptr") == 0) {
    s->m_nLinePtr = value;
  } else if (SB_STRCMP(name, "m_nbyteptr") == 0) {
    s->m_nBytePtr = value;
  } else {
    /* string failed to match any field--ignored */
    return -1;
  }

  return(0);

}
