/*
 * $Id: sbZfFabBm9600FoTestInfoConsole.c,v 1.4 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include <shared/bsl.h>

#include "sbTypes.h"
#include <soc/sbx/sbWrappers.h>
#include "sbZfFabBm9600FoTestInfoConsole.hx"



/* Print members in struct */
void
sbZfFabBm9600FoTestInfo_Print(sbZfFabBm9600FoTestInfo_t *pFromStruct) {
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("FabBm9600FoTestInfo:: isactive=0x%01x"), (unsigned int)  pFromStruct->m_bIsActive));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" dutstimeslotsize=0x%08x%08x"),   COMPILER_64_HI(pFromStruct->m_uuDUTTimeslotsize), COMPILER_64_LO(pFromStruct->m_uuDUTTimeslotsize)));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("FabBm9600FoTestInfo:: qefosimtime=0x%08x%08x"),   COMPILER_64_HI(pFromStruct->m_uuSimTimeThatQEFailoverOccurred), COMPILER_64_LO(pFromStruct->m_uuSimTimeThatQEFailoverOccurred)));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("FabBm9600FoTestInfo:: dutbaseaddress=0x%08x"), (unsigned int)  pFromStruct->m_uDUTBaseAddress));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" numfailedqe=0x%02x"), (unsigned int)  pFromStruct->m_uNumQEsThatFailedOver));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("FabBm9600FoTestInfo:: previousactivebm=0x%01x"), (unsigned int)  pFromStruct->m_uPreviousActiveBm));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" fopreviouslyasserted=0x%01x"), (unsigned int)  pFromStruct->m_bFailoverPreviouslyAsserted));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("FabBm9600FoTestInfo:: expectfo=0x%01x"), (unsigned int)  pFromStruct->m_bExpectFailover));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

}

/* SPrint members in struct */
char *
sbZfFabBm9600FoTestInfo_SPrint(sbZfFabBm9600FoTestInfo_t *pFromStruct, char *pcToString, uint32 lStrSize) {
  uint32 WrCnt = 0x0;

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"FabBm9600FoTestInfo:: isactive=0x%01x", (unsigned int)  pFromStruct->m_bIsActive);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," dutstimeslotsize=0x%08x%08x",  COMPILER_64_HI(pFromStruct->m_uuDUTTimeslotsize), COMPILER_64_LO(pFromStruct->m_uuDUTTimeslotsize));
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"FabBm9600FoTestInfo:: qefosimtime=0x%08x%08x",  COMPILER_64_HI(pFromStruct->m_uuSimTimeThatQEFailoverOccurred), COMPILER_64_LO(pFromStruct->m_uuSimTimeThatQEFailoverOccurred));
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"FabBm9600FoTestInfo:: dutbaseaddress=0x%08x", (unsigned int)  pFromStruct->m_uDUTBaseAddress);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," numfailedqe=0x%02x", (unsigned int)  pFromStruct->m_uNumQEsThatFailedOver);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"FabBm9600FoTestInfo:: previousactivebm=0x%01x", (unsigned int)  pFromStruct->m_uPreviousActiveBm);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," fopreviouslyasserted=0x%01x", (unsigned int)  pFromStruct->m_bFailoverPreviouslyAsserted);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"FabBm9600FoTestInfo:: expectfo=0x%01x", (unsigned int)  pFromStruct->m_bExpectFailover);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  /* assert if we've overrun the buffer */
  SB_ASSERT(WrCnt < lStrSize);
  return(pcToString);

}

/* validate members in struct (1 = PASS, 0 = FAIL) */
int
sbZfFabBm9600FoTestInfo_Validate(sbZfFabBm9600FoTestInfo_t *pZf) {

  if (pZf->m_bIsActive > 0x1) return 0;
  /* pZf->m_uuDUTTimeslotsize implicitly masked by data type */
  /* pZf->m_uuSimTimeThatQEFailoverOccurred implicitly masked by data type */
  /* pZf->m_uDUTBaseAddress implicitly masked by data type */
  if (pZf->m_uNumQEsThatFailedOver > 0x7f) return 0;
  if (pZf->m_uPreviousActiveBm > 0x1) return 0;
  if (pZf->m_bFailoverPreviouslyAsserted > 0x1) return 0;
  if (pZf->m_bExpectFailover > 0x1) return 0;

  return 1; /* success */

}

/* populate field from string and value */
int
sbZfFabBm9600FoTestInfo_SetField(sbZfFabBm9600FoTestInfo_t *s, char* name, int value) {

  if (SB_STRCMP(name, "") == 0 ) {
    return -1;
  } else if (SB_STRCMP(name, "isactive") == 0) {
    s->m_bIsActive = value;
  } else if (SB_STRCMP(name, "m_uuduttimeslotsize") == 0) {
    COMPILER_64_SET(s->m_uuDUTTimeslotsize, 0, value);
  } else if (SB_STRCMP(name, "m_uusimtimethatqefailoveroccurred") == 0) {
    COMPILER_64_SET(s->m_uuSimTimeThatQEFailoverOccurred, 0, value);
  } else if (SB_STRCMP(name, "m_udutbaseaddress") == 0) {
    s->m_uDUTBaseAddress = value;
  } else if (SB_STRCMP(name, "m_unumqesthatfailedover") == 0) {
    s->m_uNumQEsThatFailedOver = value;
  } else if (SB_STRCMP(name, "m_upreviousactivebm") == 0) {
    s->m_uPreviousActiveBm = value;
  } else if (SB_STRCMP(name, "failoverpreviouslyasserted") == 0) {
    s->m_bFailoverPreviouslyAsserted = value;
  } else if (SB_STRCMP(name, "expectfailover") == 0) {
    s->m_bExpectFailover = value;
  } else {
    /* string failed to match any field--ignored */
    return -1;
  }

  return(0);

}
