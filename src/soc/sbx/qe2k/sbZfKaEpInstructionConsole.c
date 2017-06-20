/*
 * $Id: sbZfKaEpInstructionConsole.c,v 1.3 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include <shared/bsl.h>

#include "sbTypes.h"
#include <soc/sbx/sbWrappers.h>
#include "sbZfKaEpInstructionConsole.hx"



/* Print members in struct */
void
sbZfKaEpInstruction_Print(sbZfKaEpInstruction_t *pFromStruct) {
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("KaEpInstruction:: valid=0x%01x"), (unsigned int)  pFromStruct->m_nValid));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" opcode=0x%02x"), (unsigned int)  pFromStruct->m_nOpcode));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" opav=0x%01x"), (unsigned int)  pFromStruct->m_nOpAVariable));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" opa=0x%02x"), (unsigned int)  pFromStruct->m_nOperandA));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" opbv=0x%01x"), (unsigned int)  pFromStruct->m_nOpBVariable));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" opb=0x%02x"), (unsigned int)  pFromStruct->m_nOperandB));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("KaEpInstruction:: opcv=0x%01x"), (unsigned int)  pFromStruct->m_nOpCVariable));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" opc=0x%03x"), (unsigned int)  pFromStruct->m_nOperandC));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

}

/* SPrint members in struct */
char *
sbZfKaEpInstruction_SPrint(sbZfKaEpInstruction_t *pFromStruct, char *pcToString, uint32 lStrSize) {
  uint32 WrCnt = 0x0;

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"KaEpInstruction:: valid=0x%01x", (unsigned int)  pFromStruct->m_nValid);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," opcode=0x%02x", (unsigned int)  pFromStruct->m_nOpcode);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," opav=0x%01x", (unsigned int)  pFromStruct->m_nOpAVariable);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," opa=0x%02x", (unsigned int)  pFromStruct->m_nOperandA);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," opbv=0x%01x", (unsigned int)  pFromStruct->m_nOpBVariable);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," opb=0x%02x", (unsigned int)  pFromStruct->m_nOperandB);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"KaEpInstruction:: opcv=0x%01x", (unsigned int)  pFromStruct->m_nOpCVariable);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," opc=0x%03x", (unsigned int)  pFromStruct->m_nOperandC);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  /* assert if we've overrun the buffer */
  SB_ASSERT(WrCnt < lStrSize);
  return(pcToString);

}

/* validate members in struct (1 = PASS, 0 = FAIL) */
int
sbZfKaEpInstruction_Validate(sbZfKaEpInstruction_t *pZf) {

  if (pZf->m_nValid > 0x1) return 0;
  if (pZf->m_nOpcode > 0x3f) return 0;
  if (pZf->m_nOpAVariable > 0x1) return 0;
  if (pZf->m_nOperandA > 0x3f) return 0;
  if (pZf->m_nOpBVariable > 0x1) return 0;
  if (pZf->m_nOperandB > 0x3f) return 0;
  if (pZf->m_nOpCVariable > 0x1) return 0;
  if (pZf->m_nOperandC > 0x3ff) return 0;

  return 1; /* success */

}

/* populate field from string and value */
int
sbZfKaEpInstruction_SetField(sbZfKaEpInstruction_t *s, char* name, int value) {

  if (SB_STRCMP(name, "") == 0 ) {
    return -1;
  } else if (SB_STRCMP(name, "m_nvalid") == 0) {
    s->m_nValid = value;
  } else if (SB_STRCMP(name, "m_nopcode") == 0) {
    s->m_nOpcode = value;
  } else if (SB_STRCMP(name, "m_nopavariable") == 0) {
    s->m_nOpAVariable = value;
  } else if (SB_STRCMP(name, "m_noperanda") == 0) {
    s->m_nOperandA = value;
  } else if (SB_STRCMP(name, "m_nopbvariable") == 0) {
    s->m_nOpBVariable = value;
  } else if (SB_STRCMP(name, "m_noperandb") == 0) {
    s->m_nOperandB = value;
  } else if (SB_STRCMP(name, "m_nopcvariable") == 0) {
    s->m_nOpCVariable = value;
  } else if (SB_STRCMP(name, "m_noperandc") == 0) {
    s->m_nOperandC = value;
  } else {
    /* string failed to match any field--ignored */
    return -1;
  }

  return(0);

}
