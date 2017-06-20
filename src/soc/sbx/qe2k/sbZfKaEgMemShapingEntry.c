/*
 * $Id: sbZfKaEgMemShapingEntry.c,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


#include "sbTypes.h"
#include "sbZfKaEgMemShapingEntry.hx"
#include "sbWrappers.h"
#include <sal/types.h>



/* Pack from struct into array of bytes */
uint32
sbZfKaEgMemShapingEntry_Pack(sbZfKaEgMemShapingEntry_t *pFrom,
                             uint8 *pToData,
                             uint32 nMaxToDataIndex) {
#ifdef SAND_BIG_ENDIAN_HOST
  int i;
  int size = SB_ZF_ZFKAEGMEMSHAPINGENTRY_SIZE_IN_BYTES;

  if (size % 4) {
    size += (4 - size %4);
  }

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on bigword endian */

  /* Pack Member: m_nReserved */
  (pToData)[10] |= ((pFrom)->m_nReserved & 0x07) <<5;

  /* Pack Member: m_nBucketDepth */
  (pToData)[5] |= ((pFrom)->m_nBucketDepth & 0x07) <<5;
  (pToData)[4] |= ((pFrom)->m_nBucketDepth >> 3) &0xFF;
  (pToData)[11] |= ((pFrom)->m_nBucketDepth >> 11) &0xFF;
  (pToData)[10] |= ((pFrom)->m_nBucketDepth >> 19) & 0x1f;

  /* Pack Member: m_nShapeRate */
  (pToData)[0] |= ((pFrom)->m_nShapeRate & 0x07) <<5;
  (pToData)[7] |= ((pFrom)->m_nShapeRate >> 3) &0xFF;
  (pToData)[6] |= ((pFrom)->m_nShapeRate >> 11) &0xFF;
  (pToData)[5] |= ((pFrom)->m_nShapeRate >> 19) & 0x1f;

  /* Pack Member: m_nMaxDepth */
  (pToData)[2] |= ((pFrom)->m_nMaxDepth & 0x03) <<6;
  (pToData)[1] |= ((pFrom)->m_nMaxDepth >> 2) &0xFF;
  (pToData)[0] |= ((pFrom)->m_nMaxDepth >> 10) & 0x1f;

  /* Pack Member: m_nPort */
  (pToData)[2] |= ((pFrom)->m_nPort & 0x3f);

  /* Pack Member: m_nHiSide */
  (pToData)[3] |= ((pFrom)->m_nHiSide & 0x01) <<7;

  /* Pack Member: m_nShapeSrc */
  (pToData)[3] |= ((pFrom)->m_nShapeSrc & 0x3f) <<1;

  /* Pack Member: m_nEnable */
  (pToData)[3] |= ((pFrom)->m_nEnable & 0x01);
#else
  int i;
  int size = SB_ZF_ZFKAEGMEMSHAPINGENTRY_SIZE_IN_BYTES;

  if (size % 4) {
    size += (4 - size %4);
  }

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on little endian */

  /* Pack Member: m_nReserved */
  (pToData)[9] |= ((pFrom)->m_nReserved & 0x07) <<5;

  /* Pack Member: m_nBucketDepth */
  (pToData)[6] |= ((pFrom)->m_nBucketDepth & 0x07) <<5;
  (pToData)[7] |= ((pFrom)->m_nBucketDepth >> 3) &0xFF;
  (pToData)[8] |= ((pFrom)->m_nBucketDepth >> 11) &0xFF;
  (pToData)[9] |= ((pFrom)->m_nBucketDepth >> 19) & 0x1f;

  /* Pack Member: m_nShapeRate */
  (pToData)[3] |= ((pFrom)->m_nShapeRate & 0x07) <<5;
  (pToData)[4] |= ((pFrom)->m_nShapeRate >> 3) &0xFF;
  (pToData)[5] |= ((pFrom)->m_nShapeRate >> 11) &0xFF;
  (pToData)[6] |= ((pFrom)->m_nShapeRate >> 19) & 0x1f;

  /* Pack Member: m_nMaxDepth */
  (pToData)[1] |= ((pFrom)->m_nMaxDepth & 0x03) <<6;
  (pToData)[2] |= ((pFrom)->m_nMaxDepth >> 2) &0xFF;
  (pToData)[3] |= ((pFrom)->m_nMaxDepth >> 10) & 0x1f;

  /* Pack Member: m_nPort */
  (pToData)[1] |= ((pFrom)->m_nPort & 0x3f);

  /* Pack Member: m_nHiSide */
  (pToData)[0] |= ((pFrom)->m_nHiSide & 0x01) <<7;

  /* Pack Member: m_nShapeSrc */
  (pToData)[0] |= ((pFrom)->m_nShapeSrc & 0x3f) <<1;

  /* Pack Member: m_nEnable */
  (pToData)[0] |= ((pFrom)->m_nEnable & 0x01);
#endif

  return SB_ZF_ZFKAEGMEMSHAPINGENTRY_SIZE_IN_BYTES;
}




/* Unpack from array of bytes into struct */
void
sbZfKaEgMemShapingEntry_Unpack(sbZfKaEgMemShapingEntry_t *pToStruct,
                               uint8 *pFromData,
                               uint32 nMaxToDataIndex) {
  COMPILER_UINT64 tmp;

  (void) tmp;

#ifdef SAND_BIG_ENDIAN_HOST

  /* Unpack operation based on bigword endian */

  /* Unpack Member: m_nReserved */
  (pToStruct)->m_nReserved =  (uint32)  ((pFromData)[10] >> 5) & 0x07;

  /* Unpack Member: m_nBucketDepth */
  (pToStruct)->m_nBucketDepth =  (uint32)  ((pFromData)[5] >> 5) & 0x07;
  (pToStruct)->m_nBucketDepth |=  (uint32)  (pFromData)[4] << 3;
  (pToStruct)->m_nBucketDepth |=  (uint32)  (pFromData)[11] << 11;
  (pToStruct)->m_nBucketDepth |=  (uint32)  ((pFromData)[10] & 0x1f) << 19;

  /* Unpack Member: m_nShapeRate */
  (pToStruct)->m_nShapeRate =  (uint32)  ((pFromData)[0] >> 5) & 0x07;
  (pToStruct)->m_nShapeRate |=  (uint32)  (pFromData)[7] << 3;
  (pToStruct)->m_nShapeRate |=  (uint32)  (pFromData)[6] << 11;
  (pToStruct)->m_nShapeRate |=  (uint32)  ((pFromData)[5] & 0x1f) << 19;

  /* Unpack Member: m_nMaxDepth */
  (pToStruct)->m_nMaxDepth =  (uint32)  ((pFromData)[2] >> 6) & 0x03;
  (pToStruct)->m_nMaxDepth |=  (uint32)  (pFromData)[1] << 2;
  (pToStruct)->m_nMaxDepth |=  (uint32)  ((pFromData)[0] & 0x1f) << 10;

  /* Unpack Member: m_nPort */
  (pToStruct)->m_nPort =  (uint32)  ((pFromData)[2] ) & 0x3f;

  /* Unpack Member: m_nHiSide */
  (pToStruct)->m_nHiSide =  (uint32)  ((pFromData)[3] >> 7) & 0x01;

  /* Unpack Member: m_nShapeSrc */
  (pToStruct)->m_nShapeSrc =  (uint32)  ((pFromData)[3] >> 1) & 0x3f;

  /* Unpack Member: m_nEnable */
  (pToStruct)->m_nEnable =  (uint32)  ((pFromData)[3] ) & 0x01;
#else

  /* Unpack operation based on little endian */

  /* Unpack Member: m_nReserved */
  (pToStruct)->m_nReserved =  (uint32)  ((pFromData)[9] >> 5) & 0x07;

  /* Unpack Member: m_nBucketDepth */
  (pToStruct)->m_nBucketDepth =  (uint32)  ((pFromData)[6] >> 5) & 0x07;
  (pToStruct)->m_nBucketDepth |=  (uint32)  (pFromData)[7] << 3;
  (pToStruct)->m_nBucketDepth |=  (uint32)  (pFromData)[8] << 11;
  (pToStruct)->m_nBucketDepth |=  (uint32)  ((pFromData)[9] & 0x1f) << 19;

  /* Unpack Member: m_nShapeRate */
  (pToStruct)->m_nShapeRate =  (uint32)  ((pFromData)[3] >> 5) & 0x07;
  (pToStruct)->m_nShapeRate |=  (uint32)  (pFromData)[4] << 3;
  (pToStruct)->m_nShapeRate |=  (uint32)  (pFromData)[5] << 11;
  (pToStruct)->m_nShapeRate |=  (uint32)  ((pFromData)[6] & 0x1f) << 19;

  /* Unpack Member: m_nMaxDepth */
  (pToStruct)->m_nMaxDepth =  (uint32)  ((pFromData)[1] >> 6) & 0x03;
  (pToStruct)->m_nMaxDepth |=  (uint32)  (pFromData)[2] << 2;
  (pToStruct)->m_nMaxDepth |=  (uint32)  ((pFromData)[3] & 0x1f) << 10;

  /* Unpack Member: m_nPort */
  (pToStruct)->m_nPort =  (uint32)  ((pFromData)[1] ) & 0x3f;

  /* Unpack Member: m_nHiSide */
  (pToStruct)->m_nHiSide =  (uint32)  ((pFromData)[0] >> 7) & 0x01;

  /* Unpack Member: m_nShapeSrc */
  (pToStruct)->m_nShapeSrc =  (uint32)  ((pFromData)[0] >> 1) & 0x3f;

  /* Unpack Member: m_nEnable */
  (pToStruct)->m_nEnable =  (uint32)  ((pFromData)[0] ) & 0x01;
#endif

}



/* initialize an instance of this zframe */
void
sbZfKaEgMemShapingEntry_InitInstance(sbZfKaEgMemShapingEntry_t *pFrame) {

  pFrame->m_nReserved =  (unsigned int)  0;
  pFrame->m_nBucketDepth =  (unsigned int)  0;
  pFrame->m_nShapeRate =  (unsigned int)  0;
  pFrame->m_nMaxDepth =  (unsigned int)  0;
  pFrame->m_nPort =  (unsigned int)  0;
  pFrame->m_nHiSide =  (unsigned int)  0;
  pFrame->m_nShapeSrc =  (unsigned int)  0;
  pFrame->m_nEnable =  (unsigned int)  0;

}
