/*
 * $Id: sbZfKaQmQueueStateEntry.c,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


#include "sbTypes.h"
#include "sbZfKaQmQueueStateEntry.hx"
#include "sbWrappers.h"
#include <sal/types.h>



/* Pack from struct into array of bytes */
uint32
sbZfKaQmQueueStateEntry_Pack(sbZfKaQmQueueStateEntry_t *pFrom,
                             uint8 *pToData,
                             uint32 nMaxToDataIndex) {
#ifdef SAND_BIG_ENDIAN_HOST
  int i;
  int size = SB_ZF_ZFKAQMQUEUESTATEENTRY_SIZE_IN_BYTES;

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on bigword endian */

  /* Pack Member: m_nAllocatedBuffsCnt */
  (pToData)[14] |= ((pFrom)->m_nAllocatedBuffsCnt & 0x01) <<7;
  (pToData)[13] |= ((pFrom)->m_nAllocatedBuffsCnt >> 1) &0xFF;
  (pToData)[12] |= ((pFrom)->m_nAllocatedBuffsCnt >> 9) &0xFF;

  /* Pack Member: m_nQTailPtr */
  (pToData)[9] |= ((pFrom)->m_nQTailPtr & 0x03) <<6;
  (pToData)[8] |= ((pFrom)->m_nQTailPtr >> 2) &0xFF;
  (pToData)[15] |= ((pFrom)->m_nQTailPtr >> 10) &0xFF;
  (pToData)[14] |= ((pFrom)->m_nQTailPtr >> 18) & 0x7f;

  /* Pack Member: m_nQHeadPtr */
  (pToData)[4] |= ((pFrom)->m_nQHeadPtr & 0x07) <<5;
  (pToData)[11] |= ((pFrom)->m_nQHeadPtr >> 3) &0xFF;
  (pToData)[10] |= ((pFrom)->m_nQHeadPtr >> 11) &0xFF;
  (pToData)[9] |= ((pFrom)->m_nQHeadPtr >> 19) & 0x3f;

  /* Pack Member: m_nNoBuffsAllocated */
  (pToData)[4] |= ((pFrom)->m_nNoBuffsAllocated & 0x01) <<4;

  /* Pack Member: m_nOverflow */
  (pToData)[4] |= ((pFrom)->m_nOverflow & 0x01) <<3;

  /* Pack Member: m_nMinBuffers */
  (pToData)[6] |= ((pFrom)->m_nMinBuffers & 0x07) <<5;
  (pToData)[5] |= ((pFrom)->m_nMinBuffers >> 3) &0xFF;
  (pToData)[4] |= ((pFrom)->m_nMinBuffers >> 11) & 0x07;

  /* Pack Member: m_nMaxBuffers */
  (pToData)[0] |= ((pFrom)->m_nMaxBuffers & 0x01) <<7;
  (pToData)[7] |= ((pFrom)->m_nMaxBuffers >> 1) &0xFF;
  (pToData)[6] |= ((pFrom)->m_nMaxBuffers >> 9) & 0x1f;

  /* Pack Member: m_nLocal */
  (pToData)[0] |= ((pFrom)->m_nLocal & 0x01) <<6;

  /* Pack Member: m_nQueueDepthInLine16B */
  (pToData)[3] |= ((pFrom)->m_nQueueDepthInLine16B & 0x07) <<5;
  (pToData)[2] |= ((pFrom)->m_nQueueDepthInLine16B >> 3) &0xFF;
  (pToData)[1] |= ((pFrom)->m_nQueueDepthInLine16B >> 11) &0xFF;
  (pToData)[0] |= ((pFrom)->m_nQueueDepthInLine16B >> 19) & 0x3f;

  /* Pack Member: m_nAnemicWatermarkSel */
  (pToData)[3] |= ((pFrom)->m_nAnemicWatermarkSel & 0x07) <<2;

  /* Pack Member: m_nQeType */
  (pToData)[3] |= ((pFrom)->m_nQeType & 0x01) <<1;

  /* Pack Member: m_nEnable */
  (pToData)[3] |= ((pFrom)->m_nEnable & 0x01);
#else
  int i;
  int size = SB_ZF_ZFKAQMQUEUESTATEENTRY_SIZE_IN_BYTES;

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on little endian */

  /* Pack Member: m_nAllocatedBuffsCnt */
  (pToData)[13] |= ((pFrom)->m_nAllocatedBuffsCnt & 0x01) <<7;
  (pToData)[14] |= ((pFrom)->m_nAllocatedBuffsCnt >> 1) &0xFF;
  (pToData)[15] |= ((pFrom)->m_nAllocatedBuffsCnt >> 9) &0xFF;

  /* Pack Member: m_nQTailPtr */
  (pToData)[10] |= ((pFrom)->m_nQTailPtr & 0x03) <<6;
  (pToData)[11] |= ((pFrom)->m_nQTailPtr >> 2) &0xFF;
  (pToData)[12] |= ((pFrom)->m_nQTailPtr >> 10) &0xFF;
  (pToData)[13] |= ((pFrom)->m_nQTailPtr >> 18) & 0x7f;

  /* Pack Member: m_nQHeadPtr */
  (pToData)[7] |= ((pFrom)->m_nQHeadPtr & 0x07) <<5;
  (pToData)[8] |= ((pFrom)->m_nQHeadPtr >> 3) &0xFF;
  (pToData)[9] |= ((pFrom)->m_nQHeadPtr >> 11) &0xFF;
  (pToData)[10] |= ((pFrom)->m_nQHeadPtr >> 19) & 0x3f;

  /* Pack Member: m_nNoBuffsAllocated */
  (pToData)[7] |= ((pFrom)->m_nNoBuffsAllocated & 0x01) <<4;

  /* Pack Member: m_nOverflow */
  (pToData)[7] |= ((pFrom)->m_nOverflow & 0x01) <<3;

  /* Pack Member: m_nMinBuffers */
  (pToData)[5] |= ((pFrom)->m_nMinBuffers & 0x07) <<5;
  (pToData)[6] |= ((pFrom)->m_nMinBuffers >> 3) &0xFF;
  (pToData)[7] |= ((pFrom)->m_nMinBuffers >> 11) & 0x07;

  /* Pack Member: m_nMaxBuffers */
  (pToData)[3] |= ((pFrom)->m_nMaxBuffers & 0x01) <<7;
  (pToData)[4] |= ((pFrom)->m_nMaxBuffers >> 1) &0xFF;
  (pToData)[5] |= ((pFrom)->m_nMaxBuffers >> 9) & 0x1f;

  /* Pack Member: m_nLocal */
  (pToData)[3] |= ((pFrom)->m_nLocal & 0x01) <<6;

  /* Pack Member: m_nQueueDepthInLine16B */
  (pToData)[0] |= ((pFrom)->m_nQueueDepthInLine16B & 0x07) <<5;
  (pToData)[1] |= ((pFrom)->m_nQueueDepthInLine16B >> 3) &0xFF;
  (pToData)[2] |= ((pFrom)->m_nQueueDepthInLine16B >> 11) &0xFF;
  (pToData)[3] |= ((pFrom)->m_nQueueDepthInLine16B >> 19) & 0x3f;

  /* Pack Member: m_nAnemicWatermarkSel */
  (pToData)[0] |= ((pFrom)->m_nAnemicWatermarkSel & 0x07) <<2;

  /* Pack Member: m_nQeType */
  (pToData)[0] |= ((pFrom)->m_nQeType & 0x01) <<1;

  /* Pack Member: m_nEnable */
  (pToData)[0] |= ((pFrom)->m_nEnable & 0x01);
#endif

  return SB_ZF_ZFKAQMQUEUESTATEENTRY_SIZE_IN_BYTES;
}




/* Unpack from array of bytes into struct */
void
sbZfKaQmQueueStateEntry_Unpack(sbZfKaQmQueueStateEntry_t *pToStruct,
                               uint8 *pFromData,
                               uint32 nMaxToDataIndex) {
  COMPILER_UINT64 tmp;

  (void) tmp;

#ifdef SAND_BIG_ENDIAN_HOST

  /* Unpack operation based on bigword endian */

  /* Unpack Member: m_nAllocatedBuffsCnt */
  (pToStruct)->m_nAllocatedBuffsCnt =  (uint32)  ((pFromData)[14] >> 7) & 0x01;
  (pToStruct)->m_nAllocatedBuffsCnt |=  (uint32)  (pFromData)[13] << 1;
  (pToStruct)->m_nAllocatedBuffsCnt |=  (uint32)  (pFromData)[12] << 9;

  /* Unpack Member: m_nQTailPtr */
  (pToStruct)->m_nQTailPtr =  (uint32)  ((pFromData)[9] >> 6) & 0x03;
  (pToStruct)->m_nQTailPtr |=  (uint32)  (pFromData)[8] << 2;
  (pToStruct)->m_nQTailPtr |=  (uint32)  (pFromData)[15] << 10;
  (pToStruct)->m_nQTailPtr |=  (uint32)  ((pFromData)[14] & 0x7f) << 18;

  /* Unpack Member: m_nQHeadPtr */
  (pToStruct)->m_nQHeadPtr =  (uint32)  ((pFromData)[4] >> 5) & 0x07;
  (pToStruct)->m_nQHeadPtr |=  (uint32)  (pFromData)[11] << 3;
  (pToStruct)->m_nQHeadPtr |=  (uint32)  (pFromData)[10] << 11;
  (pToStruct)->m_nQHeadPtr |=  (uint32)  ((pFromData)[9] & 0x3f) << 19;

  /* Unpack Member: m_nNoBuffsAllocated */
  (pToStruct)->m_nNoBuffsAllocated =  (uint8)  ((pFromData)[4] >> 4) & 0x01;

  /* Unpack Member: m_nOverflow */
  (pToStruct)->m_nOverflow =  (uint8)  ((pFromData)[4] >> 3) & 0x01;

  /* Unpack Member: m_nMinBuffers */
  (pToStruct)->m_nMinBuffers =  (uint32)  ((pFromData)[6] >> 5) & 0x07;
  (pToStruct)->m_nMinBuffers |=  (uint32)  (pFromData)[5] << 3;
  (pToStruct)->m_nMinBuffers |=  (uint32)  ((pFromData)[4] & 0x07) << 11;

  /* Unpack Member: m_nMaxBuffers */
  (pToStruct)->m_nMaxBuffers =  (uint32)  ((pFromData)[0] >> 7) & 0x01;
  (pToStruct)->m_nMaxBuffers |=  (uint32)  (pFromData)[7] << 1;
  (pToStruct)->m_nMaxBuffers |=  (uint32)  ((pFromData)[6] & 0x1f) << 9;

  /* Unpack Member: m_nLocal */
  (pToStruct)->m_nLocal =  (uint32)  ((pFromData)[0] >> 6) & 0x01;

  /* Unpack Member: m_nQueueDepthInLine16B */
  (pToStruct)->m_nQueueDepthInLine16B =  (uint32)  ((pFromData)[3] >> 5) & 0x07;
  (pToStruct)->m_nQueueDepthInLine16B |=  (uint32)  (pFromData)[2] << 3;
  (pToStruct)->m_nQueueDepthInLine16B |=  (uint32)  (pFromData)[1] << 11;
  (pToStruct)->m_nQueueDepthInLine16B |=  (uint32)  ((pFromData)[0] & 0x3f) << 19;

  /* Unpack Member: m_nAnemicWatermarkSel */
  (pToStruct)->m_nAnemicWatermarkSel =  (uint32)  ((pFromData)[3] >> 2) & 0x07;

  /* Unpack Member: m_nQeType */
  (pToStruct)->m_nQeType =  (uint32)  ((pFromData)[3] >> 1) & 0x01;

  /* Unpack Member: m_nEnable */
  (pToStruct)->m_nEnable =  (uint8)  ((pFromData)[3] ) & 0x01;
#else

  /* Unpack operation based on little endian */

  /* Unpack Member: m_nAllocatedBuffsCnt */
  (pToStruct)->m_nAllocatedBuffsCnt =  (uint32)  ((pFromData)[13] >> 7) & 0x01;
  (pToStruct)->m_nAllocatedBuffsCnt |=  (uint32)  (pFromData)[14] << 1;
  (pToStruct)->m_nAllocatedBuffsCnt |=  (uint32)  (pFromData)[15] << 9;

  /* Unpack Member: m_nQTailPtr */
  (pToStruct)->m_nQTailPtr =  (uint32)  ((pFromData)[10] >> 6) & 0x03;
  (pToStruct)->m_nQTailPtr |=  (uint32)  (pFromData)[11] << 2;
  (pToStruct)->m_nQTailPtr |=  (uint32)  (pFromData)[12] << 10;
  (pToStruct)->m_nQTailPtr |=  (uint32)  ((pFromData)[13] & 0x7f) << 18;

  /* Unpack Member: m_nQHeadPtr */
  (pToStruct)->m_nQHeadPtr =  (uint32)  ((pFromData)[7] >> 5) & 0x07;
  (pToStruct)->m_nQHeadPtr |=  (uint32)  (pFromData)[8] << 3;
  (pToStruct)->m_nQHeadPtr |=  (uint32)  (pFromData)[9] << 11;
  (pToStruct)->m_nQHeadPtr |=  (uint32)  ((pFromData)[10] & 0x3f) << 19;

  /* Unpack Member: m_nNoBuffsAllocated */
  (pToStruct)->m_nNoBuffsAllocated =  (uint8)  ((pFromData)[7] >> 4) & 0x01;

  /* Unpack Member: m_nOverflow */
  (pToStruct)->m_nOverflow =  (uint8)  ((pFromData)[7] >> 3) & 0x01;

  /* Unpack Member: m_nMinBuffers */
  (pToStruct)->m_nMinBuffers =  (uint32)  ((pFromData)[5] >> 5) & 0x07;
  (pToStruct)->m_nMinBuffers |=  (uint32)  (pFromData)[6] << 3;
  (pToStruct)->m_nMinBuffers |=  (uint32)  ((pFromData)[7] & 0x07) << 11;

  /* Unpack Member: m_nMaxBuffers */
  (pToStruct)->m_nMaxBuffers =  (uint32)  ((pFromData)[3] >> 7) & 0x01;
  (pToStruct)->m_nMaxBuffers |=  (uint32)  (pFromData)[4] << 1;
  (pToStruct)->m_nMaxBuffers |=  (uint32)  ((pFromData)[5] & 0x1f) << 9;

  /* Unpack Member: m_nLocal */
  (pToStruct)->m_nLocal =  (uint32)  ((pFromData)[3] >> 6) & 0x01;

  /* Unpack Member: m_nQueueDepthInLine16B */
  (pToStruct)->m_nQueueDepthInLine16B =  (uint32)  ((pFromData)[0] >> 5) & 0x07;
  (pToStruct)->m_nQueueDepthInLine16B |=  (uint32)  (pFromData)[1] << 3;
  (pToStruct)->m_nQueueDepthInLine16B |=  (uint32)  (pFromData)[2] << 11;
  (pToStruct)->m_nQueueDepthInLine16B |=  (uint32)  ((pFromData)[3] & 0x3f) << 19;

  /* Unpack Member: m_nAnemicWatermarkSel */
  (pToStruct)->m_nAnemicWatermarkSel =  (uint32)  ((pFromData)[0] >> 2) & 0x07;

  /* Unpack Member: m_nQeType */
  (pToStruct)->m_nQeType =  (uint32)  ((pFromData)[0] >> 1) & 0x01;

  /* Unpack Member: m_nEnable */
  (pToStruct)->m_nEnable =  (uint8)  ((pFromData)[0] ) & 0x01;
#endif

}



/* initialize an instance of this zframe */
void
sbZfKaQmQueueStateEntry_InitInstance(sbZfKaQmQueueStateEntry_t *pFrame) {

  pFrame->m_nAllocatedBuffsCnt =  (unsigned int)  0;
  pFrame->m_nQTailPtr =  (unsigned int)  0;
  pFrame->m_nQHeadPtr =  (unsigned int)  0;
  pFrame->m_nNoBuffsAllocated =  (unsigned int)  0;
  pFrame->m_nOverflow =  (unsigned int)  0;
  pFrame->m_nMinBuffers =  (unsigned int)  0;
  pFrame->m_nMaxBuffers =  (unsigned int)  0;
  pFrame->m_nLocal =  (unsigned int)  0;
  pFrame->m_nQueueDepthInLine16B =  (unsigned int)  0;
  pFrame->m_nAnemicWatermarkSel =  (unsigned int)  0;
  pFrame->m_nQeType =  (unsigned int)  0;
  pFrame->m_nEnable =  (unsigned int)  0;

}
