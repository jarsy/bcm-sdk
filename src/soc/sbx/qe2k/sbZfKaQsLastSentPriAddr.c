/*
 * $Id: sbZfKaQsLastSentPriAddr.c,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


#include "sbTypes.h"
#include "sbZfKaQsLastSentPriAddr.hx"
#include "sbWrappers.h"
#include <sal/types.h>



/* Pack from struct into array of bytes */
uint32
sbZfKaQsLastSentPriAddr_Pack(sbZfKaQsLastSentPriAddr_t *pFrom,
                             uint8 *pToData,
                             uint32 nMaxToDataIndex) {
#ifdef SAND_BIG_ENDIAN_HOST
  int i;
  int size = SB_ZF_ZFKAQSLASTSENTPRIADDR_SIZE_IN_BYTES;

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on bigword endian */

  /* Pack Member: m_nReserved */
  (pToData)[2] |= ((pFrom)->m_nReserved & 0x07) <<5;
  (pToData)[1] |= ((pFrom)->m_nReserved >> 3) &0xFF;
  (pToData)[0] |= ((pFrom)->m_nReserved >> 11) &0xFF;

  /* Pack Member: m_nMc */
  (pToData)[2] |= ((pFrom)->m_nMc & 0x01) <<4;

  /* Pack Member: m_nNode */
  (pToData)[3] |= ((pFrom)->m_nNode & 0x01) <<7;
  (pToData)[2] |= ((pFrom)->m_nNode >> 1) & 0x0f;

  /* Pack Member: m_nPort */
  (pToData)[3] |= ((pFrom)->m_nPort & 0x7f);
#else
  int i;
  int size = SB_ZF_ZFKAQSLASTSENTPRIADDR_SIZE_IN_BYTES;

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on little endian */

  /* Pack Member: m_nReserved */
  (pToData)[1] |= ((pFrom)->m_nReserved & 0x07) <<5;
  (pToData)[2] |= ((pFrom)->m_nReserved >> 3) &0xFF;
  (pToData)[3] |= ((pFrom)->m_nReserved >> 11) &0xFF;

  /* Pack Member: m_nMc */
  (pToData)[1] |= ((pFrom)->m_nMc & 0x01) <<4;

  /* Pack Member: m_nNode */
  (pToData)[0] |= ((pFrom)->m_nNode & 0x01) <<7;
  (pToData)[1] |= ((pFrom)->m_nNode >> 1) & 0x0f;

  /* Pack Member: m_nPort */
  (pToData)[0] |= ((pFrom)->m_nPort & 0x7f);
#endif

  return SB_ZF_ZFKAQSLASTSENTPRIADDR_SIZE_IN_BYTES;
}




/* Unpack from array of bytes into struct */
void
sbZfKaQsLastSentPriAddr_Unpack(sbZfKaQsLastSentPriAddr_t *pToStruct,
                               uint8 *pFromData,
                               uint32 nMaxToDataIndex) {
  COMPILER_UINT64 tmp;

  (void) tmp;

#ifdef SAND_BIG_ENDIAN_HOST

  /* Unpack operation based on bigword endian */

  /* Unpack Member: m_nReserved */
  (pToStruct)->m_nReserved =  (uint32)  ((pFromData)[2] >> 5) & 0x07;
  (pToStruct)->m_nReserved |=  (uint32)  (pFromData)[1] << 3;
  (pToStruct)->m_nReserved |=  (uint32)  (pFromData)[0] << 11;

  /* Unpack Member: m_nMc */
  (pToStruct)->m_nMc =  (uint32)  ((pFromData)[2] >> 4) & 0x01;

  /* Unpack Member: m_nNode */
  (pToStruct)->m_nNode =  (uint32)  ((pFromData)[3] >> 7) & 0x01;
  (pToStruct)->m_nNode |=  (uint32)  ((pFromData)[2] & 0x0f) << 1;

  /* Unpack Member: m_nPort */
  (pToStruct)->m_nPort =  (uint32)  ((pFromData)[3] ) & 0x7f;
#else

  /* Unpack operation based on little endian */

  /* Unpack Member: m_nReserved */
  (pToStruct)->m_nReserved =  (uint32)  ((pFromData)[1] >> 5) & 0x07;
  (pToStruct)->m_nReserved |=  (uint32)  (pFromData)[2] << 3;
  (pToStruct)->m_nReserved |=  (uint32)  (pFromData)[3] << 11;

  /* Unpack Member: m_nMc */
  (pToStruct)->m_nMc =  (uint32)  ((pFromData)[1] >> 4) & 0x01;

  /* Unpack Member: m_nNode */
  (pToStruct)->m_nNode =  (uint32)  ((pFromData)[0] >> 7) & 0x01;
  (pToStruct)->m_nNode |=  (uint32)  ((pFromData)[1] & 0x0f) << 1;

  /* Unpack Member: m_nPort */
  (pToStruct)->m_nPort =  (uint32)  ((pFromData)[0] ) & 0x7f;
#endif

}



/* initialize an instance of this zframe */
void
sbZfKaQsLastSentPriAddr_InitInstance(sbZfKaQsLastSentPriAddr_t *pFrame) {

  pFrame->m_nReserved =  (unsigned int)  0;
  pFrame->m_nMc =  (unsigned int)  0;
  pFrame->m_nNode =  (unsigned int)  0;
  pFrame->m_nPort =  (unsigned int)  0;

}
