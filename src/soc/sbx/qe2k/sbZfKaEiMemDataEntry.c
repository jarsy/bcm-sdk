/*
 * $Id: sbZfKaEiMemDataEntry.c,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


#include "sbTypes.h"
#include "sbZfKaEiMemDataEntry.hx"
#include "sbWrappers.h"
#include <sal/types.h>



/* Pack from struct into array of bytes */
uint32
sbZfKaEiMemDataEntry_Pack(sbZfKaEiMemDataEntry_t *pFrom,
                          uint8 *pToData,
                          uint32 nMaxToDataIndex) {
#ifdef SAND_BIG_ENDIAN_HOST
  int i;
  int size = SB_ZF_ZFKAEIMEMDATAENTRY_SIZE_IN_BYTES;

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on bigword endian */

  /* Pack Member: m_nDestChannel */
  (pToData)[0] |= ((pFrom)->m_nDestChannel) & 0xFF;

  /* Pack Member: m_nSizeMask */
  (pToData)[2] |= ((pFrom)->m_nSizeMask & 0x01) <<7;
  (pToData)[1] |= ((pFrom)->m_nSizeMask >> 1) &0xFF;

  /* Pack Member: m_nRbOnly */
  (pToData)[2] |= ((pFrom)->m_nRbOnly & 0x01) <<6;

  /* Pack Member: m_nLinePtr */
  (pToData)[3] |= ((pFrom)->m_nLinePtr & 0x0f) <<4;
  (pToData)[2] |= ((pFrom)->m_nLinePtr >> 4) & 0x3f;

  /* Pack Member: m_nBytePtr */
  (pToData)[3] |= ((pFrom)->m_nBytePtr & 0x0f);
#else
  int i;
  int size = SB_ZF_ZFKAEIMEMDATAENTRY_SIZE_IN_BYTES;

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on little endian */

  /* Pack Member: m_nDestChannel */
  (pToData)[3] |= ((pFrom)->m_nDestChannel) & 0xFF;

  /* Pack Member: m_nSizeMask */
  (pToData)[1] |= ((pFrom)->m_nSizeMask & 0x01) <<7;
  (pToData)[2] |= ((pFrom)->m_nSizeMask >> 1) &0xFF;

  /* Pack Member: m_nRbOnly */
  (pToData)[1] |= ((pFrom)->m_nRbOnly & 0x01) <<6;

  /* Pack Member: m_nLinePtr */
  (pToData)[0] |= ((pFrom)->m_nLinePtr & 0x0f) <<4;
  (pToData)[1] |= ((pFrom)->m_nLinePtr >> 4) & 0x3f;

  /* Pack Member: m_nBytePtr */
  (pToData)[0] |= ((pFrom)->m_nBytePtr & 0x0f);
#endif

  return SB_ZF_ZFKAEIMEMDATAENTRY_SIZE_IN_BYTES;
}




/* Unpack from array of bytes into struct */
void
sbZfKaEiMemDataEntry_Unpack(sbZfKaEiMemDataEntry_t *pToStruct,
                            uint8 *pFromData,
                            uint32 nMaxToDataIndex) {
  COMPILER_UINT64 tmp;

  (void) tmp;

#ifdef SAND_BIG_ENDIAN_HOST

  /* Unpack operation based on bigword endian */

  /* Unpack Member: m_nDestChannel */
  (pToStruct)->m_nDestChannel =  (uint32)  (pFromData)[0] ;

  /* Unpack Member: m_nSizeMask */
  (pToStruct)->m_nSizeMask =  (uint32)  ((pFromData)[2] >> 7) & 0x01;
  (pToStruct)->m_nSizeMask |=  (uint32)  (pFromData)[1] << 1;

  /* Unpack Member: m_nRbOnly */
  (pToStruct)->m_nRbOnly =  (uint32)  ((pFromData)[2] >> 6) & 0x01;

  /* Unpack Member: m_nLinePtr */
  (pToStruct)->m_nLinePtr =  (uint32)  ((pFromData)[3] >> 4) & 0x0f;
  (pToStruct)->m_nLinePtr |=  (uint32)  ((pFromData)[2] & 0x3f) << 4;

  /* Unpack Member: m_nBytePtr */
  (pToStruct)->m_nBytePtr =  (uint32)  ((pFromData)[3] ) & 0x0f;
#else

  /* Unpack operation based on little endian */

  /* Unpack Member: m_nDestChannel */
  (pToStruct)->m_nDestChannel =  (uint32)  (pFromData)[3] ;

  /* Unpack Member: m_nSizeMask */
  (pToStruct)->m_nSizeMask =  (uint32)  ((pFromData)[1] >> 7) & 0x01;
  (pToStruct)->m_nSizeMask |=  (uint32)  (pFromData)[2] << 1;

  /* Unpack Member: m_nRbOnly */
  (pToStruct)->m_nRbOnly =  (uint32)  ((pFromData)[1] >> 6) & 0x01;

  /* Unpack Member: m_nLinePtr */
  (pToStruct)->m_nLinePtr =  (uint32)  ((pFromData)[0] >> 4) & 0x0f;
  (pToStruct)->m_nLinePtr |=  (uint32)  ((pFromData)[1] & 0x3f) << 4;

  /* Unpack Member: m_nBytePtr */
  (pToStruct)->m_nBytePtr =  (uint32)  ((pFromData)[0] ) & 0x0f;
#endif

}



/* initialize an instance of this zframe */
void
sbZfKaEiMemDataEntry_InitInstance(sbZfKaEiMemDataEntry_t *pFrame) {

  pFrame->m_nDestChannel =  (unsigned int)  0;
  pFrame->m_nSizeMask =  (unsigned int)  0;
  pFrame->m_nRbOnly =  (unsigned int)  0;
  pFrame->m_nLinePtr =  (unsigned int)  0;
  pFrame->m_nBytePtr =  (unsigned int)  0;

}
