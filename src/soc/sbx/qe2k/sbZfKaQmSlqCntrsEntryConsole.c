/*
 * $Id: sbZfKaQmSlqCntrsEntryConsole.c,v 1.3 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include <shared/bsl.h>

#include "sbTypes.h"
#include <soc/sbx/sbWrappers.h>
#include "sbZfKaQmSlqCntrsEntryConsole.hx"



/* Print members in struct */
void
sbZfKaQmSlqCntrsEntry_Print(sbZfKaQmSlqCntrsEntry_t *pFromStruct) {
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("KaQmSlqCntrsEntry:: data1=0x%08x"), (unsigned int)  pFromStruct->m_nData1));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" data0=0x%08x"), (unsigned int)  pFromStruct->m_nData0));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

}

/* SPrint members in struct */
char *
sbZfKaQmSlqCntrsEntry_SPrint(sbZfKaQmSlqCntrsEntry_t *pFromStruct, char *pcToString, uint32 lStrSize) {
  uint32 WrCnt = 0x0;

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"KaQmSlqCntrsEntry:: data1=0x%08x", (unsigned int)  pFromStruct->m_nData1);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," data0=0x%08x", (unsigned int)  pFromStruct->m_nData0);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  /* assert if we've overrun the buffer */
  SB_ASSERT(WrCnt < lStrSize);
  return(pcToString);

}

/* validate members in struct (1 = PASS, 0 = FAIL) */
int
sbZfKaQmSlqCntrsEntry_Validate(sbZfKaQmSlqCntrsEntry_t *pZf) {

  /* pZf->m_nData1 implicitly masked by data type */
  /* pZf->m_nData0 implicitly masked by data type */

  return 1; /* success */

}

/* populate field from string and value */
int
sbZfKaQmSlqCntrsEntry_SetField(sbZfKaQmSlqCntrsEntry_t *s, char* name, int value) {

  if (SB_STRCMP(name, "") == 0 ) {
    return -1;
  } else if (SB_STRCMP(name, "m_ndata1") == 0) {
    s->m_nData1 = value;
  } else if (SB_STRCMP(name, "m_ndata0") == 0) {
    s->m_nData0 = value;
  } else {
    /* string failed to match any field--ignored */
    return -1;
  }

  return(0);

}
