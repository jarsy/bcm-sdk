/*
 * $Id: sbZfKaEiRawSpiReadEntryConsole.c,v 1.3 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include <shared/bsl.h>

#include "sbTypes.h"
#include <soc/sbx/sbWrappers.h>
#include "sbZfKaEiRawSpiReadEntryConsole.hx"



/* Print members in struct */
void
sbZfKaEiRawSpiReadEntry_Print(sbZfKaEiRawSpiReadEntry_t *pFromStruct) {
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("KaEiRawSpiReadEntry:: destchannel=0x%02x"), (unsigned int)  pFromStruct->m_nDestChannel));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" sizemask8=0x%01x"), (unsigned int)  pFromStruct->m_nSizeMask8));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" sizemask7_0=0x%02x"), (unsigned int)  pFromStruct->m_nSizeMask7_0));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("KaEiRawSpiReadEntry:: rb_loopback_only=0x%01x"), (unsigned int)  pFromStruct->m_nRbLoopbackOnly));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" line_ptr=0x%03x"), (unsigned int)  pFromStruct->m_nLinePtr));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" byte_ptr=0x%01x"), (unsigned int)  pFromStruct->m_nBytePtr));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

}

/* SPrint members in struct */
char *
sbZfKaEiRawSpiReadEntry_SPrint(sbZfKaEiRawSpiReadEntry_t *pFromStruct, char *pcToString, uint32 lStrSize) {
  uint32 WrCnt = 0x0;

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"KaEiRawSpiReadEntry:: destchannel=0x%02x", (unsigned int)  pFromStruct->m_nDestChannel);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," sizemask8=0x%01x", (unsigned int)  pFromStruct->m_nSizeMask8);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," sizemask7_0=0x%02x", (unsigned int)  pFromStruct->m_nSizeMask7_0);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"KaEiRawSpiReadEntry:: rb_loopback_only=0x%01x", (unsigned int)  pFromStruct->m_nRbLoopbackOnly);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," line_ptr=0x%03x", (unsigned int)  pFromStruct->m_nLinePtr);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," byte_ptr=0x%01x", (unsigned int)  pFromStruct->m_nBytePtr);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  /* assert if we've overrun the buffer */
  SB_ASSERT(WrCnt < lStrSize);
  return(pcToString);

}

/* validate members in struct (1 = PASS, 0 = FAIL) */
int
sbZfKaEiRawSpiReadEntry_Validate(sbZfKaEiRawSpiReadEntry_t *pZf) {

  if (pZf->m_nDestChannel > 0xff) return 0;
  if (pZf->m_nSizeMask8 > 0x1) return 0;
  if (pZf->m_nSizeMask7_0 > 0xff) return 0;
  if (pZf->m_nRbLoopbackOnly > 0x1) return 0;
  if (pZf->m_nLinePtr > 0x3ff) return 0;
  if (pZf->m_nBytePtr > 0xf) return 0;

  return 1; /* success */

}

/* populate field from string and value */
int
sbZfKaEiRawSpiReadEntry_SetField(sbZfKaEiRawSpiReadEntry_t *s, char* name, int value) {

  if (SB_STRCMP(name, "") == 0 ) {
    return -1;
  } else if (SB_STRCMP(name, "m_ndestchannel") == 0) {
    s->m_nDestChannel = value;
  } else if (SB_STRCMP(name, "m_nsizemask8") == 0) {
    s->m_nSizeMask8 = value;
  } else if (SB_STRCMP(name, "m_nsizemask7_0") == 0) {
    s->m_nSizeMask7_0 = value;
  } else if (SB_STRCMP(name, "m_nrbloopbackonly") == 0) {
    s->m_nRbLoopbackOnly = value;
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
