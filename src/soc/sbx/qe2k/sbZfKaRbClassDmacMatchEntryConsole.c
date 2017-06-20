/*
 * $Id: sbZfKaRbClassDmacMatchEntryConsole.c,v 1.3 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include <shared/bsl.h>

#include "sbTypes.h"
#include <soc/sbx/sbWrappers.h>
#include "sbZfKaRbClassDmacMatchEntryConsole.hx"



/* Print members in struct */
void
sbZfKaRbClassDmacMatchEntry_Print(sbZfKaRbClassDmacMatchEntry_t *pFromStruct) {
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("KaRbClassDmacMatchEntry:: dmacdatalsb=0x%08x"), (unsigned int)  pFromStruct->m_nDmacDataLsb));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" dmacdatarsv=0x%04x"), (unsigned int)  pFromStruct->m_nDmacDataRsv));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("KaRbClassDmacMatchEntry:: dmacdatamsb=0x%04x"), (unsigned int)  pFromStruct->m_nDmacDataMsb));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" dmacmasklsb=0x%08x"), (unsigned int)  pFromStruct->m_nDmacMaskLsb));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("KaRbClassDmacMatchEntry:: dmacmaskrsv=0x%04x"), (unsigned int)  pFromStruct->m_nDmacMaskRsv));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" dmacmaskmsb=0x%04x"), (unsigned int)  pFromStruct->m_nDmacMaskMsb));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("KaRbClassDmacMatchEntry:: dmacreserve=0x%07x"), (unsigned int)  pFromStruct->m_nDmacReserve));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" dmacenb=0x%01x"), (unsigned int)  pFromStruct->m_nDmacEnable));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" dmacdp=0x%01x"), (unsigned int)  pFromStruct->m_nDmacDp));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("KaRbClassDmacMatchEntry:: dmaclsb=0x%01x"), (unsigned int)  pFromStruct->m_nDmacLsb));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

}

/* SPrint members in struct */
char *
sbZfKaRbClassDmacMatchEntry_SPrint(sbZfKaRbClassDmacMatchEntry_t *pFromStruct, char *pcToString, uint32 lStrSize) {
  uint32 WrCnt = 0x0;

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"KaRbClassDmacMatchEntry:: dmacdatalsb=0x%08x", (unsigned int)  pFromStruct->m_nDmacDataLsb);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," dmacdatarsv=0x%04x", (unsigned int)  pFromStruct->m_nDmacDataRsv);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"KaRbClassDmacMatchEntry:: dmacdatamsb=0x%04x", (unsigned int)  pFromStruct->m_nDmacDataMsb);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," dmacmasklsb=0x%08x", (unsigned int)  pFromStruct->m_nDmacMaskLsb);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"KaRbClassDmacMatchEntry:: dmacmaskrsv=0x%04x", (unsigned int)  pFromStruct->m_nDmacMaskRsv);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," dmacmaskmsb=0x%04x", (unsigned int)  pFromStruct->m_nDmacMaskMsb);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"KaRbClassDmacMatchEntry:: dmacreserve=0x%07x", (unsigned int)  pFromStruct->m_nDmacReserve);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," dmacenb=0x%01x", (unsigned int)  pFromStruct->m_nDmacEnable);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," dmacdp=0x%01x", (unsigned int)  pFromStruct->m_nDmacDp);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"KaRbClassDmacMatchEntry:: dmaclsb=0x%01x", (unsigned int)  pFromStruct->m_nDmacLsb);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  /* assert if we've overrun the buffer */
  SB_ASSERT(WrCnt < lStrSize);
  return(pcToString);

}

/* validate members in struct (1 = PASS, 0 = FAIL) */
int
sbZfKaRbClassDmacMatchEntry_Validate(sbZfKaRbClassDmacMatchEntry_t *pZf) {

  /* pZf->m_nDmacDataLsb implicitly masked by data type */
  if (pZf->m_nDmacDataRsv > 0xffff) return 0;
  if (pZf->m_nDmacDataMsb > 0xffff) return 0;
  /* pZf->m_nDmacMaskLsb implicitly masked by data type */
  if (pZf->m_nDmacMaskRsv > 0xffff) return 0;
  if (pZf->m_nDmacMaskMsb > 0xffff) return 0;
  if (pZf->m_nDmacReserve > 0x1ffffff) return 0;
  if (pZf->m_nDmacEnable > 0x1) return 0;
  if (pZf->m_nDmacDp > 0x3) return 0;
  if (pZf->m_nDmacLsb > 0xf) return 0;

  return 1; /* success */

}

/* populate field from string and value */
int
sbZfKaRbClassDmacMatchEntry_SetField(sbZfKaRbClassDmacMatchEntry_t *s, char* name, int value) {

  if (SB_STRCMP(name, "") == 0 ) {
    return -1;
  } else if (SB_STRCMP(name, "m_ndmacdatalsb") == 0) {
    s->m_nDmacDataLsb = value;
  } else if (SB_STRCMP(name, "m_ndmacdatarsv") == 0) {
    s->m_nDmacDataRsv = value;
  } else if (SB_STRCMP(name, "m_ndmacdatamsb") == 0) {
    s->m_nDmacDataMsb = value;
  } else if (SB_STRCMP(name, "m_ndmacmasklsb") == 0) {
    s->m_nDmacMaskLsb = value;
  } else if (SB_STRCMP(name, "m_ndmacmaskrsv") == 0) {
    s->m_nDmacMaskRsv = value;
  } else if (SB_STRCMP(name, "m_ndmacmaskmsb") == 0) {
    s->m_nDmacMaskMsb = value;
  } else if (SB_STRCMP(name, "m_ndmacreserve") == 0) {
    s->m_nDmacReserve = value;
  } else if (SB_STRCMP(name, "m_ndmacenable") == 0) {
    s->m_nDmacEnable = value;
  } else if (SB_STRCMP(name, "m_ndmacdp") == 0) {
    s->m_nDmacDp = value;
  } else if (SB_STRCMP(name, "m_ndmaclsb") == 0) {
    s->m_nDmacLsb = value;
  } else {
    /* string failed to match any field--ignored */
    return -1;
  }

  return(0);

}
