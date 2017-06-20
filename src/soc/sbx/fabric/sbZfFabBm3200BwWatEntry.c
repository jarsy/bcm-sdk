/*
 * $Id: sbZfFabBm3200BwWatEntry.c,v 1.4 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


#include <shared/bsl.h>

#include "sbTypes.h"
#include "sbZfFabBm3200BwWatEntry.hx"
#include "sbWrappers.h"
#include <sal/types.h>



/* Pack from struct into array of bytes */
uint32
sbZfFabBm3200BwWatEntry_Pack(sbZfFabBm3200BwWatEntry_t *pFrom,
                             uint8 *pToData,
                             uint32 nMaxToDataIndex) {
#ifdef SAND_BIG_ENDIAN_HOST
  int i;
  int size = SB_ZF_FAB_BM3200_WAT_ENTRY_SIZE_IN_BYTES;

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on bigword endian */

  /* Pack Member: m_nAccumLen */
  (pToData)[7] |= ((pFrom)->m_nAccumLen) & 0xFF;
  (pToData)[6] |= ((pFrom)->m_nAccumLen >> 8) &0xFF;
  (pToData)[5] |= ((pFrom)->m_nAccumLen >> 16) &0xFF;
  (pToData)[4] |= ((pFrom)->m_nAccumLen >> 24) &0xFF;

  /* Pack Member: m_nAccounted */
  (pToData)[3] |= ((pFrom)->m_nAccounted) & 0xFF;
  (pToData)[2] |= ((pFrom)->m_nAccounted >> 8) &0xFF;
  (pToData)[1] |= ((pFrom)->m_nAccounted >> 16) &0xFF;
  (pToData)[0] |= ((pFrom)->m_nAccounted >> 24) &0xFF;
#else
  int i;
  int size = SB_ZF_FAB_BM3200_WAT_ENTRY_SIZE_IN_BYTES;

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on little endian */

  /* Pack Member: m_nAccumLen */
  (pToData)[4] |= ((pFrom)->m_nAccumLen) & 0xFF;
  (pToData)[5] |= ((pFrom)->m_nAccumLen >> 8) &0xFF;
  (pToData)[6] |= ((pFrom)->m_nAccumLen >> 16) &0xFF;
  (pToData)[7] |= ((pFrom)->m_nAccumLen >> 24) &0xFF;

  /* Pack Member: m_nAccounted */
  (pToData)[0] |= ((pFrom)->m_nAccounted) & 0xFF;
  (pToData)[1] |= ((pFrom)->m_nAccounted >> 8) &0xFF;
  (pToData)[2] |= ((pFrom)->m_nAccounted >> 16) &0xFF;
  (pToData)[3] |= ((pFrom)->m_nAccounted >> 24) &0xFF;
#endif

  return SB_ZF_FAB_BM3200_WAT_ENTRY_SIZE_IN_BYTES;
}




/* Unpack from array of bytes into struct */
void
sbZfFabBm3200BwWatEntry_Unpack(sbZfFabBm3200BwWatEntry_t *pToStruct,
                               uint8 *pFromData,
                               uint32 nMaxToDataIndex) {
  COMPILER_UINT64 tmp;

  (void) tmp;

#ifdef SAND_BIG_ENDIAN_HOST

  /* Unpack operation based on bigword endian */

  /* Unpack Member: m_nAccumLen */
  (pToStruct)->m_nAccumLen =  (uint32)  (pFromData)[7] ;
  (pToStruct)->m_nAccumLen |=  (uint32)  (pFromData)[6] << 8;
  (pToStruct)->m_nAccumLen |=  (uint32)  (pFromData)[5] << 16;
  (pToStruct)->m_nAccumLen |=  (uint32)  (pFromData)[4] << 24;

  /* Unpack Member: m_nAccounted */
  (pToStruct)->m_nAccounted =  (uint32)  (pFromData)[3] ;
  (pToStruct)->m_nAccounted |=  (uint32)  (pFromData)[2] << 8;
  (pToStruct)->m_nAccounted |=  (uint32)  (pFromData)[1] << 16;
  (pToStruct)->m_nAccounted |=  (uint32)  (pFromData)[0] << 24;
#else

  /* Unpack operation based on little endian */

  /* Unpack Member: m_nAccumLen */
  (pToStruct)->m_nAccumLen =  (uint32)  (pFromData)[4] ;
  (pToStruct)->m_nAccumLen |=  (uint32)  (pFromData)[5] << 8;
  (pToStruct)->m_nAccumLen |=  (uint32)  (pFromData)[6] << 16;
  (pToStruct)->m_nAccumLen |=  (uint32)  (pFromData)[7] << 24;

  /* Unpack Member: m_nAccounted */
  (pToStruct)->m_nAccounted =  (uint32)  (pFromData)[0] ;
  (pToStruct)->m_nAccounted |=  (uint32)  (pFromData)[1] << 8;
  (pToStruct)->m_nAccounted |=  (uint32)  (pFromData)[2] << 16;
  (pToStruct)->m_nAccounted |=  (uint32)  (pFromData)[3] << 24;
#endif

}



/* initialize an instance of this zframe */
void
sbZfFabBm3200BwWatEntry_InitInstance(sbZfFabBm3200BwWatEntry_t *pFrame) {

  pFrame->m_nAccumLen =  (unsigned int)  0;
  pFrame->m_nAccounted =  (unsigned int)  0;

}
#ifdef SB_ZF_INCLUDE_CONSOLE
/*
 * $Id: sbZfFabBm3200BwWatEntry.c,v 1.4 Broadcom SDK $
 * $Copyright (c) 2012 Broadcom Corporation
 * All rights reserved.$
 */
#include "sbTypes.h"
#include <soc/sbx/sbWrappers.h>
#include "sbZfFabBm3200BwWatEntry.hx"



/* Print members in struct */
void
sbZfFabBm3200BwWatEntry_Print(sbZfFabBm3200BwWatEntry_t *pFromStruct) {
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("FabBm3200BwWatEntry:: accumlen=0x%08x"), (unsigned int)  pFromStruct->m_nAccumLen));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" accounted=0x%08x"), (unsigned int)  pFromStruct->m_nAccounted));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

}

/* SPrint members in struct */
int
sbZfFabBm3200BwWatEntry_SPrint(sbZfFabBm3200BwWatEntry_t *pFromStruct, char *pcToString, uint32 lStrSize) {
  uint32 WrCnt = 0x0;

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"FabBm3200BwWatEntry:: accumlen=0x%08x", (unsigned int)  pFromStruct->m_nAccumLen);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," accounted=0x%08x", (unsigned int)  pFromStruct->m_nAccounted);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  /* assert if we've overrun the buffer */
  SB_ASSERT(WrCnt < lStrSize);
  return(WrCnt);

}

/* validate members in struct (1 = PASS, 0 = FAIL) */
int
sbZfFabBm3200BwWatEntry_Validate(sbZfFabBm3200BwWatEntry_t *pZf) {

  /* pZf->m_nAccumLen implicitly masked by data type */
  /* pZf->m_nAccounted implicitly masked by data type */

  return 1; /* success */

}

/* populate field from string and value */
int
sbZfFabBm3200BwWatEntry_SetField(sbZfFabBm3200BwWatEntry_t *s, char* name, int value) {

  if (SB_STRCMP(name, "") == 0 ) {
    return -1;
  } else if (SB_STRCMP(name, "m_naccumlen") == 0) {
    s->m_nAccumLen = value;
  } else if (SB_STRCMP(name, "m_naccounted") == 0) {
    s->m_nAccounted = value;
  } else {
    /* string failed to match any field--ignored */
    return -1;
  }

  return(0);

}


#endif /* ifdef SB_ZF_INCLUDE_CONSOLE */

