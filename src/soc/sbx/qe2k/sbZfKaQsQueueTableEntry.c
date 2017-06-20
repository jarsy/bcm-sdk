/*
 * $Id: sbZfKaQsQueueTableEntry.c,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


#include "sbTypes.h"
#include "sbZfKaQsQueueTableEntry.hx"
#include "sbWrappers.h"
#include <sal/types.h>



/* Pack from struct into array of bytes */
uint32
sbZfKaQsQueueTableEntry_Pack(sbZfKaQsQueueTableEntry_t *pFrom,
                             uint8 *pToData,
                             uint32 nMaxToDataIndex) {
#ifdef SAND_BIG_ENDIAN_HOST
  int i;
  int size = SB_ZF_ZFKAQSQUEUETABLEENTRY_SIZE_IN_BYTES;

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on bigword endian */

  /* Pack Member: m_nCredit */
  (pToData)[7] |= ((pFrom)->m_nCredit & 0x01) <<7;
  (pToData)[6] |= ((pFrom)->m_nCredit >> 1) &0xFF;
  (pToData)[5] |= ((pFrom)->m_nCredit >> 9) &0xFF;
  (pToData)[4] |= ((pFrom)->m_nCredit >> 17) &0xFF;

  /* Pack Member: m_nHpLen */
  (pToData)[7] |= ((pFrom)->m_nHpLen & 0x03) <<5;

  /* Pack Member: m_nDepth */
  (pToData)[7] |= ((pFrom)->m_nDepth & 0x0f) <<1;

  /* Pack Member: m_nQ2Ec */
  (pToData)[1] |= ((pFrom)->m_nQ2Ec) & 0xFF;
  (pToData)[0] |= ((pFrom)->m_nQ2Ec >> 8) &0xFF;
  (pToData)[7] |= ((pFrom)->m_nQ2Ec >> 16) & 0x01;

  /* Pack Member: m_nLocalQ */
  (pToData)[2] |= ((pFrom)->m_nLocalQ & 0x01) <<7;

  /* Pack Member: m_nMaxHoldTs */
  (pToData)[2] |= ((pFrom)->m_nMaxHoldTs & 0x07) <<4;

  /* Pack Member: m_nQueueType */
  (pToData)[2] |= ((pFrom)->m_nQueueType & 0x0f);

  /* Pack Member: m_nShapeRateMSB */
  (pToData)[3] |= ((pFrom)->m_nShapeRateMSB) & 0xFF;
#else
  int i;
  int size = SB_ZF_ZFKAQSQUEUETABLEENTRY_SIZE_IN_BYTES;

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on little endian */

  /* Pack Member: m_nCredit */
  (pToData)[4] |= ((pFrom)->m_nCredit & 0x01) <<7;
  (pToData)[5] |= ((pFrom)->m_nCredit >> 1) &0xFF;
  (pToData)[6] |= ((pFrom)->m_nCredit >> 9) &0xFF;
  (pToData)[7] |= ((pFrom)->m_nCredit >> 17) &0xFF;

  /* Pack Member: m_nHpLen */
  (pToData)[4] |= ((pFrom)->m_nHpLen & 0x03) <<5;

  /* Pack Member: m_nDepth */
  (pToData)[4] |= ((pFrom)->m_nDepth & 0x0f) <<1;

  /* Pack Member: m_nQ2Ec */
  (pToData)[2] |= ((pFrom)->m_nQ2Ec) & 0xFF;
  (pToData)[3] |= ((pFrom)->m_nQ2Ec >> 8) &0xFF;
  (pToData)[4] |= ((pFrom)->m_nQ2Ec >> 16) & 0x01;

  /* Pack Member: m_nLocalQ */
  (pToData)[1] |= ((pFrom)->m_nLocalQ & 0x01) <<7;

  /* Pack Member: m_nMaxHoldTs */
  (pToData)[1] |= ((pFrom)->m_nMaxHoldTs & 0x07) <<4;

  /* Pack Member: m_nQueueType */
  (pToData)[1] |= ((pFrom)->m_nQueueType & 0x0f);

  /* Pack Member: m_nShapeRateMSB */
  (pToData)[0] |= ((pFrom)->m_nShapeRateMSB) & 0xFF;
#endif

  return SB_ZF_ZFKAQSQUEUETABLEENTRY_SIZE_IN_BYTES;
}




/* Unpack from array of bytes into struct */
void
sbZfKaQsQueueTableEntry_Unpack(sbZfKaQsQueueTableEntry_t *pToStruct,
                               uint8 *pFromData,
                               uint32 nMaxToDataIndex) {
  COMPILER_UINT64 tmp;

  (void) tmp;

#ifdef SAND_BIG_ENDIAN_HOST

  /* Unpack operation based on bigword endian */

  /* Unpack Member: m_nCredit */
  (pToStruct)->m_nCredit =  (uint32)  ((pFromData)[7] >> 7) & 0x01;
  (pToStruct)->m_nCredit |=  (uint32)  (pFromData)[6] << 1;
  (pToStruct)->m_nCredit |=  (uint32)  (pFromData)[5] << 9;
  (pToStruct)->m_nCredit |=  (uint32)  (pFromData)[4] << 17;

  /* Unpack Member: m_nHpLen */
  (pToStruct)->m_nHpLen =  (uint32)  ((pFromData)[7] >> 5) & 0x03;

  /* Unpack Member: m_nDepth */
  (pToStruct)->m_nDepth =  (uint32)  ((pFromData)[7] >> 1) & 0x0f;

  /* Unpack Member: m_nQ2Ec */
  (pToStruct)->m_nQ2Ec =  (uint32)  (pFromData)[1] ;
  (pToStruct)->m_nQ2Ec |=  (uint32)  (pFromData)[0] << 8;
  (pToStruct)->m_nQ2Ec |=  (uint32)  ((pFromData)[7] & 0x01) << 16;

  /* Unpack Member: m_nLocalQ */
  (pToStruct)->m_nLocalQ =  (uint32)  ((pFromData)[2] >> 7) & 0x01;

  /* Unpack Member: m_nMaxHoldTs */
  (pToStruct)->m_nMaxHoldTs =  (uint32)  ((pFromData)[2] >> 4) & 0x07;

  /* Unpack Member: m_nQueueType */
  (pToStruct)->m_nQueueType =  (uint32)  ((pFromData)[2] ) & 0x0f;

  /* Unpack Member: m_nShapeRateMSB */
  (pToStruct)->m_nShapeRateMSB =  (uint32)  (pFromData)[3] ;
#else

  /* Unpack operation based on little endian */

  /* Unpack Member: m_nCredit */
  (pToStruct)->m_nCredit =  (uint32)  ((pFromData)[4] >> 7) & 0x01;
  (pToStruct)->m_nCredit |=  (uint32)  (pFromData)[5] << 1;
  (pToStruct)->m_nCredit |=  (uint32)  (pFromData)[6] << 9;
  (pToStruct)->m_nCredit |=  (uint32)  (pFromData)[7] << 17;

  /* Unpack Member: m_nHpLen */
  (pToStruct)->m_nHpLen =  (uint32)  ((pFromData)[4] >> 5) & 0x03;

  /* Unpack Member: m_nDepth */
  (pToStruct)->m_nDepth =  (uint32)  ((pFromData)[4] >> 1) & 0x0f;

  /* Unpack Member: m_nQ2Ec */
  (pToStruct)->m_nQ2Ec =  (uint32)  (pFromData)[2] ;
  (pToStruct)->m_nQ2Ec |=  (uint32)  (pFromData)[3] << 8;
  (pToStruct)->m_nQ2Ec |=  (uint32)  ((pFromData)[4] & 0x01) << 16;

  /* Unpack Member: m_nLocalQ */
  (pToStruct)->m_nLocalQ =  (uint32)  ((pFromData)[1] >> 7) & 0x01;

  /* Unpack Member: m_nMaxHoldTs */
  (pToStruct)->m_nMaxHoldTs =  (uint32)  ((pFromData)[1] >> 4) & 0x07;

  /* Unpack Member: m_nQueueType */
  (pToStruct)->m_nQueueType =  (uint32)  ((pFromData)[1] ) & 0x0f;

  /* Unpack Member: m_nShapeRateMSB */
  (pToStruct)->m_nShapeRateMSB =  (uint32)  (pFromData)[0] ;
#endif

}



/* initialize an instance of this zframe */
void
sbZfKaQsQueueTableEntry_InitInstance(sbZfKaQsQueueTableEntry_t *pFrame) {

  pFrame->m_nCredit =  (unsigned int)  0;
  pFrame->m_nHpLen =  (unsigned int)  0;
  pFrame->m_nDepth =  (unsigned int)  0;
  pFrame->m_nQ2Ec =  (unsigned int)  0;
  pFrame->m_nLocalQ =  (unsigned int)  0;
  pFrame->m_nMaxHoldTs =  (unsigned int)  0;
  pFrame->m_nQueueType =  (unsigned int)  0;
  pFrame->m_nShapeRateMSB =  (unsigned int)  0;

}
