/*
 * $Id: sbZfFabBm3200BwRepErrInfo.c,v 1.4 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


#include <shared/bsl.h>

#include "sbTypes.h"
#include "sbZfFabBm3200BwRepErrInfo.hx"
#include "sbWrappers.h"
#include <sal/types.h>



/* Pack from struct into array of bytes */
uint32
sbZfFabBm3200BwRepErrInfo_Pack(sbZfFabBm3200BwRepErrInfo_t *pFrom,
                               uint8 *pToData,
                               uint32 nMaxToDataIndex) {
#ifdef SAND_BIG_ENDIAN_HOST
  int i;
  int size = SB_ZF_FAB_BM3200_BW_REP_ERR_INFO_SIZE_IN_BYTES;

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on bigword endian */

  /* Pack Member: m_nClient */
  (pToData)[1] |= ((pFrom)->m_nClient & 0x1f) <<3;
  (pToData)[0] |= ((pFrom)->m_nClient >> 5) & 0x01;

  /* Pack Member: m_nTableId */
  (pToData)[2] |= ((pFrom)->m_nTableId & 0x01) <<7;
  (pToData)[1] |= ((pFrom)->m_nTableId >> 1) & 0x07;

  /* Pack Member: m_nOffset */
  (pToData)[3] |= ((pFrom)->m_nOffset) & 0xFF;
  (pToData)[2] |= ((pFrom)->m_nOffset >> 8) & 0x7f;
#else
  int i;
  int size = SB_ZF_FAB_BM3200_BW_REP_ERR_INFO_SIZE_IN_BYTES;

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on little endian */

  /* Pack Member: m_nClient */
  (pToData)[2] |= ((pFrom)->m_nClient & 0x1f) <<3;
  (pToData)[3] |= ((pFrom)->m_nClient >> 5) & 0x01;

  /* Pack Member: m_nTableId */
  (pToData)[1] |= ((pFrom)->m_nTableId & 0x01) <<7;
  (pToData)[2] |= ((pFrom)->m_nTableId >> 1) & 0x07;

  /* Pack Member: m_nOffset */
  (pToData)[0] |= ((pFrom)->m_nOffset) & 0xFF;
  (pToData)[1] |= ((pFrom)->m_nOffset >> 8) & 0x7f;
#endif

  return SB_ZF_FAB_BM3200_BW_REP_ERR_INFO_SIZE_IN_BYTES;
}




/* Unpack from array of bytes into struct */
void
sbZfFabBm3200BwRepErrInfo_Unpack(sbZfFabBm3200BwRepErrInfo_t *pToStruct,
                                 uint8 *pFromData,
                                 uint32 nMaxToDataIndex) {
  COMPILER_UINT64 tmp;

  (void) tmp;

#ifdef SAND_BIG_ENDIAN_HOST

  /* Unpack operation based on bigword endian */

  /* Unpack Member: m_nClient */
  (pToStruct)->m_nClient =  (int32)  ((pFromData)[1] >> 3) & 0x1f;
  (pToStruct)->m_nClient |=  (int32)  ((pFromData)[0] & 0x01) << 5;

  /* Unpack Member: m_nTableId */
  (pToStruct)->m_nTableId =  (int32)  ((pFromData)[2] >> 7) & 0x01;
  (pToStruct)->m_nTableId |=  (int32)  ((pFromData)[1] & 0x07) << 1;

  /* Unpack Member: m_nOffset */
  (pToStruct)->m_nOffset =  (int32)  (pFromData)[3] ;
  (pToStruct)->m_nOffset |=  (int32)  ((pFromData)[2] & 0x7f) << 8;
#else

  /* Unpack operation based on little endian */

  /* Unpack Member: m_nClient */
  (pToStruct)->m_nClient =  (int32)  ((pFromData)[2] >> 3) & 0x1f;
  (pToStruct)->m_nClient |=  (int32)  ((pFromData)[3] & 0x01) << 5;

  /* Unpack Member: m_nTableId */
  (pToStruct)->m_nTableId =  (int32)  ((pFromData)[1] >> 7) & 0x01;
  (pToStruct)->m_nTableId |=  (int32)  ((pFromData)[2] & 0x07) << 1;

  /* Unpack Member: m_nOffset */
  (pToStruct)->m_nOffset =  (int32)  (pFromData)[0] ;
  (pToStruct)->m_nOffset |=  (int32)  ((pFromData)[1] & 0x7f) << 8;
#endif

}



/* initialize an instance of this zframe */
void
sbZfFabBm3200BwRepErrInfo_InitInstance(sbZfFabBm3200BwRepErrInfo_t *pFrame) {

  pFrame->m_nClient =  (unsigned int)  0;
  pFrame->m_nTableId =  (unsigned int)  0;
  pFrame->m_nOffset =  (unsigned int)  0;

}
#ifdef SB_ZF_INCLUDE_CONSOLE
/*
 * $Id: sbZfFabBm3200BwRepErrInfo.c,v 1.4 Broadcom SDK $
 * $Copyright (c) 2012 Broadcom Corporation
 * All rights reserved.$
 */
#include "sbTypes.h"
#include <soc/sbx/sbWrappers.h>
#include "sbZfFabBm3200BwRepErrInfo.hx"



/* Print members in struct */
void
sbZfFabBm3200BwRepErrInfo_Print(sbZfFabBm3200BwRepErrInfo_t *pFromStruct) {
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("FabBm3200BwRepErrInfo:: client=0x%02x"), (unsigned int)  pFromStruct->m_nClient));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" table_id=0x%01x"), (unsigned int)  pFromStruct->m_nTableId));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" offset=0x%04x"), (unsigned int)  pFromStruct->m_nOffset));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

}

/* SPrint members in struct */
int
sbZfFabBm3200BwRepErrInfo_SPrint(sbZfFabBm3200BwRepErrInfo_t *pFromStruct, char *pcToString, uint32 lStrSize) {
  uint32 WrCnt = 0x0;

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"FabBm3200BwRepErrInfo:: client=0x%02x", (unsigned int)  pFromStruct->m_nClient);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," table_id=0x%01x", (unsigned int)  pFromStruct->m_nTableId);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," offset=0x%04x", (unsigned int)  pFromStruct->m_nOffset);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  /* assert if we've overrun the buffer */
  SB_ASSERT(WrCnt < lStrSize);
  return(WrCnt);

}

/* validate members in struct (1 = PASS, 0 = FAIL) */
int
sbZfFabBm3200BwRepErrInfo_Validate(sbZfFabBm3200BwRepErrInfo_t *pZf) {

  if (pZf->m_nClient > 0x3f) return 0;
  if (pZf->m_nTableId > 0xf) return 0;
  if (pZf->m_nOffset > 0x7fff) return 0;

  return 1; /* success */

}

/* populate field from string and value */
int
sbZfFabBm3200BwRepErrInfo_SetField(sbZfFabBm3200BwRepErrInfo_t *s, char* name, int value) {

  if (SB_STRCMP(name, "") == 0 ) {
    return -1;
  } else if (SB_STRCMP(name, "m_nclient") == 0) {
    s->m_nClient = value;
  } else if (SB_STRCMP(name, "m_ntableid") == 0) {
    s->m_nTableId = value;
  } else if (SB_STRCMP(name, "m_noffset") == 0) {
    s->m_nOffset = value;
  } else {
    /* string failed to match any field--ignored */
    return -1;
  }

  return(0);

}


#endif /* ifdef SB_ZF_INCLUDE_CONSOLE */

