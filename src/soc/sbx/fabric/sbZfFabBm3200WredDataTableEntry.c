/*
 * $Id: sbZfFabBm3200WredDataTableEntry.c,v 1.3 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


#include <shared/bsl.h>

#include "sbTypes.h"
#include "sbZfFabBm3200WredDataTableEntry.hx"
#include "sbWrappers.h"
#include <sal/types.h>



/* Pack from struct into array of bytes */
uint32
sbZfFabBm3200WredDataTableEntry_Pack(sbZfFabBm3200WredDataTableEntry_t *pFrom,
                                     uint8 *pToData,
                                     uint32 nMaxToDataIndex) {
#ifdef SAND_BIG_ENDIAN_HOST
  int i;
  int size = SB_ZF_FAB_BM3200_WRED_DATA_TABLE_ENTRY_SIZE_IN_BYTES;

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on bigword endian */

  /* Pack Member: m_nTemplateOdd */
  (pToData)[0] |= ((pFrom)->m_nTemplateOdd) & 0xFF;

  /* Pack Member: m_nReservedOdd */
  (pToData)[1] |= ((pFrom)->m_nReservedOdd & 0x0f) <<4;

  /* Pack Member: m_nGainOdd */
  (pToData)[1] |= ((pFrom)->m_nGainOdd & 0x0f);

  /* Pack Member: m_nTemplateEven */
  (pToData)[2] |= ((pFrom)->m_nTemplateEven) & 0xFF;

  /* Pack Member: m_nReservedEven */
  (pToData)[3] |= ((pFrom)->m_nReservedEven & 0x0f) <<4;

  /* Pack Member: m_nGainEven */
  (pToData)[3] |= ((pFrom)->m_nGainEven & 0x0f);
#else
  int i;
  int size = SB_ZF_FAB_BM3200_WRED_DATA_TABLE_ENTRY_SIZE_IN_BYTES;

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on little endian */

  /* Pack Member: m_nTemplateOdd */
  (pToData)[3] |= ((pFrom)->m_nTemplateOdd) & 0xFF;

  /* Pack Member: m_nReservedOdd */
  (pToData)[2] |= ((pFrom)->m_nReservedOdd & 0x0f) <<4;

  /* Pack Member: m_nGainOdd */
  (pToData)[2] |= ((pFrom)->m_nGainOdd & 0x0f);

  /* Pack Member: m_nTemplateEven */
  (pToData)[1] |= ((pFrom)->m_nTemplateEven) & 0xFF;

  /* Pack Member: m_nReservedEven */
  (pToData)[0] |= ((pFrom)->m_nReservedEven & 0x0f) <<4;

  /* Pack Member: m_nGainEven */
  (pToData)[0] |= ((pFrom)->m_nGainEven & 0x0f);
#endif

  return SB_ZF_FAB_BM3200_WRED_DATA_TABLE_ENTRY_SIZE_IN_BYTES;
}




/* Unpack from array of bytes into struct */
void
sbZfFabBm3200WredDataTableEntry_Unpack(sbZfFabBm3200WredDataTableEntry_t *pToStruct,
                                       uint8 *pFromData,
                                       uint32 nMaxToDataIndex) {
  COMPILER_UINT64 tmp;

  (void) tmp;

#ifdef SAND_BIG_ENDIAN_HOST

  /* Unpack operation based on bigword endian */

  /* Unpack Member: m_nTemplateOdd */
  (pToStruct)->m_nTemplateOdd =  (uint32)  (pFromData)[0] ;

  /* Unpack Member: m_nReservedOdd */
  (pToStruct)->m_nReservedOdd =  (uint32)  ((pFromData)[1] >> 4) & 0x0f;

  /* Unpack Member: m_nGainOdd */
  (pToStruct)->m_nGainOdd =  (uint32)  ((pFromData)[1] ) & 0x0f;

  /* Unpack Member: m_nTemplateEven */
  (pToStruct)->m_nTemplateEven =  (uint32)  (pFromData)[2] ;

  /* Unpack Member: m_nReservedEven */
  (pToStruct)->m_nReservedEven =  (uint32)  ((pFromData)[3] >> 4) & 0x0f;

  /* Unpack Member: m_nGainEven */
  (pToStruct)->m_nGainEven =  (uint32)  ((pFromData)[3] ) & 0x0f;
#else

  /* Unpack operation based on little endian */

  /* Unpack Member: m_nTemplateOdd */
  (pToStruct)->m_nTemplateOdd =  (uint32)  (pFromData)[3] ;

  /* Unpack Member: m_nReservedOdd */
  (pToStruct)->m_nReservedOdd =  (uint32)  ((pFromData)[2] >> 4) & 0x0f;

  /* Unpack Member: m_nGainOdd */
  (pToStruct)->m_nGainOdd =  (uint32)  ((pFromData)[2] ) & 0x0f;

  /* Unpack Member: m_nTemplateEven */
  (pToStruct)->m_nTemplateEven =  (uint32)  (pFromData)[1] ;

  /* Unpack Member: m_nReservedEven */
  (pToStruct)->m_nReservedEven =  (uint32)  ((pFromData)[0] >> 4) & 0x0f;

  /* Unpack Member: m_nGainEven */
  (pToStruct)->m_nGainEven =  (uint32)  ((pFromData)[0] ) & 0x0f;
#endif

}



/* initialize an instance of this zframe */
void
sbZfFabBm3200WredDataTableEntry_InitInstance(sbZfFabBm3200WredDataTableEntry_t *pFrame) {

  pFrame->m_nTemplateOdd =  (unsigned int)  0;
  pFrame->m_nReservedOdd =  (unsigned int)  0;
  pFrame->m_nGainOdd =  (unsigned int)  0;
  pFrame->m_nTemplateEven =  (unsigned int)  0;
  pFrame->m_nReservedEven =  (unsigned int)  0;
  pFrame->m_nGainEven =  (unsigned int)  0;

}
#ifdef SB_ZF_INCLUDE_CONSOLE
/*
 * $Id: sbZfFabBm3200WredDataTableEntry.c,v 1.3 Broadcom SDK $
 * $Copyright (c) 2012 Broadcom Corporation
 * All rights reserved.$
 */
#include "sbTypes.h"
#include <soc/sbx/sbWrappers.h>
#include "sbZfFabBm3200WredDataTableEntry.hx"



/* Print members in struct */
void
sbZfFabBm3200WredDataTableEntry_Print(sbZfFabBm3200WredDataTableEntry_t *pFromStruct) {
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("FabBm3200WredDataTableEntry:: template_odd=0x%02x"), (unsigned int)  pFromStruct->m_nTemplateOdd));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" reserved_odd=0x%01x"), (unsigned int)  pFromStruct->m_nReservedOdd));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("FabBm3200WredDataTableEntry:: gain_odd=0x%01x"), (unsigned int)  pFromStruct->m_nGainOdd));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" template_even=0x%02x"), (unsigned int)  pFromStruct->m_nTemplateEven));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("FabBm3200WredDataTableEntry:: reserved_even=0x%01x"), (unsigned int)  pFromStruct->m_nReservedEven));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" gain_even=0x%01x"), (unsigned int)  pFromStruct->m_nGainEven));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

}

/* SPrint members in struct */
int
sbZfFabBm3200WredDataTableEntry_SPrint(sbZfFabBm3200WredDataTableEntry_t *pFromStruct, char *pcToString, uint32 lStrSize) {
  uint32 WrCnt = 0x0;

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"FabBm3200WredDataTableEntry:: template_odd=0x%02x", (unsigned int)  pFromStruct->m_nTemplateOdd);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," reserved_odd=0x%01x", (unsigned int)  pFromStruct->m_nReservedOdd);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"FabBm3200WredDataTableEntry:: gain_odd=0x%01x", (unsigned int)  pFromStruct->m_nGainOdd);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," template_even=0x%02x", (unsigned int)  pFromStruct->m_nTemplateEven);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"FabBm3200WredDataTableEntry:: reserved_even=0x%01x", (unsigned int)  pFromStruct->m_nReservedEven);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," gain_even=0x%01x", (unsigned int)  pFromStruct->m_nGainEven);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  /* assert if we've overrun the buffer */
  SB_ASSERT(WrCnt < lStrSize);
  return(WrCnt);

}

/* validate members in struct (1 = PASS, 0 = FAIL) */
int
sbZfFabBm3200WredDataTableEntry_Validate(sbZfFabBm3200WredDataTableEntry_t *pZf) {

  if (pZf->m_nTemplateOdd > 0xff) return 0;
  if (pZf->m_nReservedOdd > 0xf) return 0;
  if (pZf->m_nGainOdd > 0xf) return 0;
  if (pZf->m_nTemplateEven > 0xff) return 0;
  if (pZf->m_nReservedEven > 0xf) return 0;
  if (pZf->m_nGainEven > 0xf) return 0;

  return 1; /* success */

}

/* populate field from string and value */
int
sbZfFabBm3200WredDataTableEntry_SetField(sbZfFabBm3200WredDataTableEntry_t *s, char* name, int value) {

  if (SB_STRCMP(name, "") == 0 ) {
    return -1;
  } else if (SB_STRCMP(name, "m_ntemplateodd") == 0) {
    s->m_nTemplateOdd = value;
  } else if (SB_STRCMP(name, "m_nreservedodd") == 0) {
    s->m_nReservedOdd = value;
  } else if (SB_STRCMP(name, "m_ngainodd") == 0) {
    s->m_nGainOdd = value;
  } else if (SB_STRCMP(name, "m_ntemplateeven") == 0) {
    s->m_nTemplateEven = value;
  } else if (SB_STRCMP(name, "m_nreservedeven") == 0) {
    s->m_nReservedEven = value;
  } else if (SB_STRCMP(name, "m_ngaineven") == 0) {
    s->m_nGainEven = value;
  } else {
    /* string failed to match any field--ignored */
    return -1;
  }

  return(0);

}


#endif /* ifdef SB_ZF_INCLUDE_CONSOLE */

