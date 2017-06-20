/*
 * $Id: sbZfKaQsShapeRateEntry.c,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


#include "sbTypes.h"
#include "sbZfKaQsShapeRateEntry.hx"
#include "sbWrappers.h"
#include <sal/types.h>



/* Pack from struct into array of bytes */
uint32
sbZfKaQsShapeRateEntry_Pack(sbZfKaQsShapeRateEntry_t *pFrom,
                            uint8 *pToData,
                            uint32 nMaxToDataIndex) {
#ifdef SAND_BIG_ENDIAN_HOST
  int i;
  int size = SB_ZF_ZFKAQSSHAPERATEENTRY_SIZE_IN_BYTES;

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on bigword endian */

  /* Pack Member: m_nReserved */
  (pToData)[0] |= ((pFrom)->m_nReserved) & 0xFF;

  /* Pack Member: m_nMode */
  (pToData)[1] |= ((pFrom)->m_nMode & 0x01) <<7;

  /* Pack Member: m_nShapeRate */
  (pToData)[3] |= ((pFrom)->m_nShapeRate) & 0xFF;
  (pToData)[2] |= ((pFrom)->m_nShapeRate >> 8) &0xFF;
  (pToData)[1] |= ((pFrom)->m_nShapeRate >> 16) & 0x7f;
#else
  int i;
  int size = SB_ZF_ZFKAQSSHAPERATEENTRY_SIZE_IN_BYTES;

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on little endian */

  /* Pack Member: m_nReserved */
  (pToData)[3] |= ((pFrom)->m_nReserved) & 0xFF;

  /* Pack Member: m_nMode */
  (pToData)[2] |= ((pFrom)->m_nMode & 0x01) <<7;

  /* Pack Member: m_nShapeRate */
  (pToData)[0] |= ((pFrom)->m_nShapeRate) & 0xFF;
  (pToData)[1] |= ((pFrom)->m_nShapeRate >> 8) &0xFF;
  (pToData)[2] |= ((pFrom)->m_nShapeRate >> 16) & 0x7f;
#endif

  return SB_ZF_ZFKAQSSHAPERATEENTRY_SIZE_IN_BYTES;
}




/* Unpack from array of bytes into struct */
void
sbZfKaQsShapeRateEntry_Unpack(sbZfKaQsShapeRateEntry_t *pToStruct,
                              uint8 *pFromData,
                              uint32 nMaxToDataIndex) {
  COMPILER_UINT64 tmp;

  (void) tmp;

#ifdef SAND_BIG_ENDIAN_HOST

  /* Unpack operation based on bigword endian */

  /* Unpack Member: m_nReserved */
  (pToStruct)->m_nReserved =  (uint32)  (pFromData)[0] ;

  /* Unpack Member: m_nMode */
  (pToStruct)->m_nMode =  (uint32)  ((pFromData)[1] >> 7) & 0x01;

  /* Unpack Member: m_nShapeRate */
  (pToStruct)->m_nShapeRate =  (uint32)  (pFromData)[3] ;
  (pToStruct)->m_nShapeRate |=  (uint32)  (pFromData)[2] << 8;
  (pToStruct)->m_nShapeRate |=  (uint32)  ((pFromData)[1] & 0x7f) << 16;
#else

  /* Unpack operation based on little endian */

  /* Unpack Member: m_nReserved */
  (pToStruct)->m_nReserved =  (uint32)  (pFromData)[3] ;

  /* Unpack Member: m_nMode */
  (pToStruct)->m_nMode =  (uint32)  ((pFromData)[2] >> 7) & 0x01;

  /* Unpack Member: m_nShapeRate */
  (pToStruct)->m_nShapeRate =  (uint32)  (pFromData)[0] ;
  (pToStruct)->m_nShapeRate |=  (uint32)  (pFromData)[1] << 8;
  (pToStruct)->m_nShapeRate |=  (uint32)  ((pFromData)[2] & 0x7f) << 16;
#endif

}



/* initialize an instance of this zframe */
void
sbZfKaQsShapeRateEntry_InitInstance(sbZfKaQsShapeRateEntry_t *pFrame) {

  pFrame->m_nReserved =  (unsigned int)  0;
  pFrame->m_nMode =  (unsigned int)  0;
  pFrame->m_nShapeRate =  (unsigned int)  0;

}
