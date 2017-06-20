/*
 * $Id: sbZfFabBm3200BwLthrEntry.c,v 1.4 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


#include <shared/bsl.h>

#include "sbTypes.h"
#include "sbZfFabBm3200BwLthrEntry.hx"
#include "sbWrappers.h"
#include <sal/types.h>



/* Pack from struct into array of bytes */
uint32
sbZfFabBm3200BwLthrEntry_Pack(sbZfFabBm3200BwLthrEntry_t *pFrom,
                              uint8 *pToData,
                              uint32 nMaxToDataIndex) {
#ifdef SAND_BIG_ENDIAN_HOST
  int i;
  int size = SB_ZF_FAB_BM3200_LTHR_ENTRY_SIZE_IN_BYTES;

  if (size % 4) {
    size += (4 - size %4);
  }

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on bigword endian */

  /* Pack Member: m_nLengthThresh */
  (pToData)[3] |= ((pFrom)->m_nLengthThresh) & 0xFF;
  (pToData)[2] |= ((pFrom)->m_nLengthThresh >> 8) &0xFF;
#else
  int i;
  int size = SB_ZF_FAB_BM3200_LTHR_ENTRY_SIZE_IN_BYTES;

  if (size % 4) {
    size += (4 - size %4);
  }

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on little endian */

  /* Pack Member: m_nLengthThresh */
  (pToData)[0] |= ((pFrom)->m_nLengthThresh) & 0xFF;
  (pToData)[1] |= ((pFrom)->m_nLengthThresh >> 8) &0xFF;
#endif

  return SB_ZF_FAB_BM3200_LTHR_ENTRY_SIZE_IN_BYTES;
}




/* Unpack from array of bytes into struct */
void
sbZfFabBm3200BwLthrEntry_Unpack(sbZfFabBm3200BwLthrEntry_t *pToStruct,
                                uint8 *pFromData,
                                uint32 nMaxToDataIndex) {
  COMPILER_UINT64 tmp;

  (void) tmp;

#ifdef SAND_BIG_ENDIAN_HOST

  /* Unpack operation based on bigword endian */

  /* Unpack Member: m_nLengthThresh */
  (pToStruct)->m_nLengthThresh =  (uint32)  (pFromData)[3] ;
  (pToStruct)->m_nLengthThresh |=  (uint32)  (pFromData)[2] << 8;
#else

  /* Unpack operation based on little endian */

  /* Unpack Member: m_nLengthThresh */
  (pToStruct)->m_nLengthThresh =  (uint32)  (pFromData)[0] ;
  (pToStruct)->m_nLengthThresh |=  (uint32)  (pFromData)[1] << 8;
#endif

}



/* initialize an instance of this zframe */
void
sbZfFabBm3200BwLthrEntry_InitInstance(sbZfFabBm3200BwLthrEntry_t *pFrame) {

  pFrame->m_nLengthThresh =  (unsigned int)  0;

}
#ifdef SB_ZF_INCLUDE_CONSOLE
/*
 * $Id: sbZfFabBm3200BwLthrEntry.c,v 1.4 Broadcom SDK $
 * $Copyright (c) 2012 Broadcom Corporation
 * All rights reserved.$
 */
#include "sbTypes.h"
#include <soc/sbx/sbWrappers.h>
#include "sbZfFabBm3200BwLthrEntry.hx"



/* Print members in struct */
void
sbZfFabBm3200BwLthrEntry_Print(sbZfFabBm3200BwLthrEntry_t *pFromStruct) {
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("FabBm3200BwLthrEntry:: lengththresh=0x%04x"), (unsigned int)  pFromStruct->m_nLengthThresh));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

}

/* SPrint members in struct */
int
sbZfFabBm3200BwLthrEntry_SPrint(sbZfFabBm3200BwLthrEntry_t *pFromStruct, char *pcToString, uint32 lStrSize) {
  uint32 WrCnt = 0x0;

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"FabBm3200BwLthrEntry:: lengththresh=0x%04x", (unsigned int)  pFromStruct->m_nLengthThresh);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  /* assert if we've overrun the buffer */
  SB_ASSERT(WrCnt < lStrSize);
  return(WrCnt);

}

/* validate members in struct (1 = PASS, 0 = FAIL) */
int
sbZfFabBm3200BwLthrEntry_Validate(sbZfFabBm3200BwLthrEntry_t *pZf) {

  if (pZf->m_nLengthThresh > 0xffff) return 0;

  return 1; /* success */

}

/* populate field from string and value */
int
sbZfFabBm3200BwLthrEntry_SetField(sbZfFabBm3200BwLthrEntry_t *s, char* name, int value) {

  if (SB_STRCMP(name, "") == 0 ) {
    return -1;
  } else if (SB_STRCMP(name, "m_nlengththresh") == 0) {
    s->m_nLengthThresh = value;
  } else {
    /* string failed to match any field--ignored */
    return -1;
  }

  return(0);

}


#endif /* ifdef SB_ZF_INCLUDE_CONSOLE */

