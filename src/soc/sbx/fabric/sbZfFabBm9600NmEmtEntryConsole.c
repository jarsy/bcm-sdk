/*
 * $Id: sbZfFabBm9600NmEmtEntryConsole.c,v 1.4 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include <shared/bsl.h>

#include "sbTypes.h"
#include <soc/sbx/sbWrappers.h>
#include "sbZfFabBm9600NmEmtEntryConsole.hx"



/* Print members in struct */
void
sbZfFabBm9600NmEmtEntry_Print(sbZfFabBm9600NmEmtEntry_t *pFromStruct) {
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("FabBm9600NmEmtEntry:: esetfullstatusmode=0x%01x"), (unsigned int)  pFromStruct->m_uEsetFullStatusMode));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" mfem=0x%01x"), (unsigned int)  pFromStruct->m_uMfem));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" esetmember1=0x%02x"), (unsigned int)  pFromStruct->m_uEsetMember1));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("FabBm9600NmEmtEntry:: esetmember0=0x%08x%08x"),   COMPILER_64_HI(pFromStruct->m_uuEsetMember0), COMPILER_64_LO(pFromStruct->m_uuEsetMember0)));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

}

/* SPrint members in struct */
char *
sbZfFabBm9600NmEmtEntry_SPrint(sbZfFabBm9600NmEmtEntry_t *pFromStruct, char *pcToString, uint32 lStrSize) {
  uint32 WrCnt = 0x0;

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"FabBm9600NmEmtEntry:: esetfullstatusmode=0x%01x", (unsigned int)  pFromStruct->m_uEsetFullStatusMode);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," mfem=0x%01x", (unsigned int)  pFromStruct->m_uMfem);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," esetmember1=0x%02x", (unsigned int)  pFromStruct->m_uEsetMember1);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"FabBm9600NmEmtEntry:: esetmember0=0x%08x%08x",   COMPILER_64_HI(pFromStruct->m_uuEsetMember0), COMPILER_64_LO(pFromStruct->m_uuEsetMember0));
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  /* assert if we've overrun the buffer */
  SB_ASSERT(WrCnt < lStrSize);
  return(pcToString);

}

/* validate members in struct (1 = PASS, 0 = FAIL) */
int
sbZfFabBm9600NmEmtEntry_Validate(sbZfFabBm9600NmEmtEntry_t *pZf) {

  if (pZf->m_uEsetFullStatusMode > 0x1) return 0;
  if (pZf->m_uMfem > 0x1) return 0;
  if (pZf->m_uEsetMember1 > 0xff) return 0;
  /* pZf->m_uuEsetMember0 implicitly masked by data type */

  return 1; /* success */

}

/* populate field from string and value */
int
sbZfFabBm9600NmEmtEntry_SetField(sbZfFabBm9600NmEmtEntry_t *s, char* name, int value) {

  if (SB_STRCMP(name, "") == 0 ) {
    return -1;
  } else if (SB_STRCMP(name, "m_uesetfullstatusmode") == 0) {
    s->m_uEsetFullStatusMode = value;
  } else if (SB_STRCMP(name, "m_umfem") == 0) {
    s->m_uMfem = value;
  } else if (SB_STRCMP(name, "m_uesetmember1") == 0) {
    s->m_uEsetMember1 = value;
  } else if (SB_STRCMP(name, "m_uuesetmember0") == 0) {
    COMPILER_64_SET(s->m_uuEsetMember0, 0, value);
  } else {
    /* string failed to match any field--ignored */
    return -1;
  }

  return(0);

}
