/*
 * $Id: sbZfKaQmRateDeltaMaxTableEntry.c,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


#include "sbTypes.h"
#include "sbZfKaQmRateDeltaMaxTableEntry.hx"
#include "sbWrappers.h"
#include <sal/types.h>



/* Pack from struct into array of bytes */
uint32
sbZfKaQmRateDeltaMaxTableEntry_Pack(sbZfKaQmRateDeltaMaxTableEntry_t *pFrom,
                                    uint8 *pToData,
                                    uint32 nMaxToDataIndex) {
#ifdef SAND_BIG_ENDIAN_HOST
  int i;
  int size = SB_ZF_ZFKAQMRATEDELTAMAXTABLEENTRY_SIZE_IN_BYTES;

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on bigword endian */

  /* Pack Member: m_nRateDeltaMax */
  (pToData)[3] |= ((pFrom)->m_nRateDeltaMax) & 0xFF;
  (pToData)[2] |= ((pFrom)->m_nRateDeltaMax >> 8) &0xFF;
  (pToData)[1] |= ((pFrom)->m_nRateDeltaMax >> 16) &0xFF;
  (pToData)[0] |= ((pFrom)->m_nRateDeltaMax >> 24) & 0x07;
#else
  int i;
  int size = SB_ZF_ZFKAQMRATEDELTAMAXTABLEENTRY_SIZE_IN_BYTES;

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on little endian */

  /* Pack Member: m_nRateDeltaMax */
  (pToData)[0] |= ((pFrom)->m_nRateDeltaMax) & 0xFF;
  (pToData)[1] |= ((pFrom)->m_nRateDeltaMax >> 8) &0xFF;
  (pToData)[2] |= ((pFrom)->m_nRateDeltaMax >> 16) &0xFF;
  (pToData)[3] |= ((pFrom)->m_nRateDeltaMax >> 24) & 0x07;
#endif

  return SB_ZF_ZFKAQMRATEDELTAMAXTABLEENTRY_SIZE_IN_BYTES;
}




/* Unpack from array of bytes into struct */
void
sbZfKaQmRateDeltaMaxTableEntry_Unpack(sbZfKaQmRateDeltaMaxTableEntry_t *pToStruct,
                                      uint8 *pFromData,
                                      uint32 nMaxToDataIndex) {
  COMPILER_UINT64 tmp;

  (void) tmp;

#ifdef SAND_BIG_ENDIAN_HOST

  /* Unpack operation based on bigword endian */

  /* Unpack Member: m_nRateDeltaMax */
  (pToStruct)->m_nRateDeltaMax =  (uint32)  (pFromData)[3] ;
  (pToStruct)->m_nRateDeltaMax |=  (uint32)  (pFromData)[2] << 8;
  (pToStruct)->m_nRateDeltaMax |=  (uint32)  (pFromData)[1] << 16;
  (pToStruct)->m_nRateDeltaMax |=  (uint32)  ((pFromData)[0] & 0x07) << 24;
#else

  /* Unpack operation based on little endian */

  /* Unpack Member: m_nRateDeltaMax */
  (pToStruct)->m_nRateDeltaMax =  (uint32)  (pFromData)[0] ;
  (pToStruct)->m_nRateDeltaMax |=  (uint32)  (pFromData)[1] << 8;
  (pToStruct)->m_nRateDeltaMax |=  (uint32)  (pFromData)[2] << 16;
  (pToStruct)->m_nRateDeltaMax |=  (uint32)  ((pFromData)[3] & 0x07) << 24;
#endif

}



/* initialize an instance of this zframe */
void
sbZfKaQmRateDeltaMaxTableEntry_InitInstance(sbZfKaQmRateDeltaMaxTableEntry_t *pFrame) {

  pFrame->m_nRateDeltaMax =  (unsigned int)  0;

}
