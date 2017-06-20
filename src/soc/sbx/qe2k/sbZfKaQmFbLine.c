/*
 * $Id: sbZfKaQmFbLine.c,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


#include "sbTypes.h"
#include "sbZfKaQmFbLine.hx"
#include "sbWrappers.h"
#include <sal/types.h>



/* Pack from struct into array of bytes */
uint32
sbZfKaQmFbLine_Pack(sbZfKaQmFbLine_t *pFrom,
                    uint8 *pToData,
                    uint32 nMaxToDataIndex) {
#ifdef SAND_BIG_ENDIAN_HOST
  int i;
  int size = SB_ZF_ZFKAQMFBLINE_SIZE_IN_BYTES;

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on bigword endian */

  /* Pack Member: m_nHec1 */
  (pToData)[12] |= ((pFrom)->m_nHec1) & 0xFF;

  /* Pack Member: m_nHec0 */
  (pToData)[13] |= ((pFrom)->m_nHec0) & 0xFF;

  /* Pack Member: m_nSpare */
  (pToData)[15] |= ((pFrom)->m_nSpare & 0x03) <<6;
  (pToData)[14] |= ((pFrom)->m_nSpare >> 2) &0xFF;

  /* Pack Member: m_nPbExtAddr5 */
  (pToData)[9] |= ((pFrom)->m_nPbExtAddr5 & 0x07) <<5;
  (pToData)[8] |= ((pFrom)->m_nPbExtAddr5 >> 3) &0xFF;
  (pToData)[15] |= ((pFrom)->m_nPbExtAddr5 >> 11) & 0x3f;

  /* Pack Member: m_nPbExtAddr4 */
  (pToData)[11] |= ((pFrom)->m_nPbExtAddr4 & 0x0f) <<4;
  (pToData)[10] |= ((pFrom)->m_nPbExtAddr4 >> 4) &0xFF;
  (pToData)[9] |= ((pFrom)->m_nPbExtAddr4 >> 12) & 0x1f;

  /* Pack Member: m_nPbExtAddr3 */
  (pToData)[5] |= ((pFrom)->m_nPbExtAddr3 & 0x1f) <<3;
  (pToData)[4] |= ((pFrom)->m_nPbExtAddr3 >> 5) &0xFF;
  (pToData)[11] |= ((pFrom)->m_nPbExtAddr3 >> 13) & 0x0f;

  /* Pack Member: m_nPbExtAddr2 */
  (pToData)[7] |= ((pFrom)->m_nPbExtAddr2 & 0x3f) <<2;
  (pToData)[6] |= ((pFrom)->m_nPbExtAddr2 >> 6) &0xFF;
  (pToData)[5] |= ((pFrom)->m_nPbExtAddr2 >> 14) & 0x07;

  /* Pack Member: m_nPbExtAddr1 */
  (pToData)[1] |= ((pFrom)->m_nPbExtAddr1 & 0x7f) <<1;
  (pToData)[0] |= ((pFrom)->m_nPbExtAddr1 >> 7) &0xFF;
  (pToData)[7] |= ((pFrom)->m_nPbExtAddr1 >> 15) & 0x03;

  /* Pack Member: m_nPbExtAddr0 */
  (pToData)[3] |= ((pFrom)->m_nPbExtAddr0) & 0xFF;
  (pToData)[2] |= ((pFrom)->m_nPbExtAddr0 >> 8) &0xFF;
  (pToData)[1] |= ((pFrom)->m_nPbExtAddr0 >> 16) & 0x01;
#else
  int i;
  int size = SB_ZF_ZFKAQMFBLINE_SIZE_IN_BYTES;

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on little endian */

  /* Pack Member: m_nHec1 */
  (pToData)[15] |= ((pFrom)->m_nHec1) & 0xFF;

  /* Pack Member: m_nHec0 */
  (pToData)[14] |= ((pFrom)->m_nHec0) & 0xFF;

  /* Pack Member: m_nSpare */
  (pToData)[12] |= ((pFrom)->m_nSpare & 0x03) <<6;
  (pToData)[13] |= ((pFrom)->m_nSpare >> 2) &0xFF;

  /* Pack Member: m_nPbExtAddr5 */
  (pToData)[10] |= ((pFrom)->m_nPbExtAddr5 & 0x07) <<5;
  (pToData)[11] |= ((pFrom)->m_nPbExtAddr5 >> 3) &0xFF;
  (pToData)[12] |= ((pFrom)->m_nPbExtAddr5 >> 11) & 0x3f;

  /* Pack Member: m_nPbExtAddr4 */
  (pToData)[8] |= ((pFrom)->m_nPbExtAddr4 & 0x0f) <<4;
  (pToData)[9] |= ((pFrom)->m_nPbExtAddr4 >> 4) &0xFF;
  (pToData)[10] |= ((pFrom)->m_nPbExtAddr4 >> 12) & 0x1f;

  /* Pack Member: m_nPbExtAddr3 */
  (pToData)[6] |= ((pFrom)->m_nPbExtAddr3 & 0x1f) <<3;
  (pToData)[7] |= ((pFrom)->m_nPbExtAddr3 >> 5) &0xFF;
  (pToData)[8] |= ((pFrom)->m_nPbExtAddr3 >> 13) & 0x0f;

  /* Pack Member: m_nPbExtAddr2 */
  (pToData)[4] |= ((pFrom)->m_nPbExtAddr2 & 0x3f) <<2;
  (pToData)[5] |= ((pFrom)->m_nPbExtAddr2 >> 6) &0xFF;
  (pToData)[6] |= ((pFrom)->m_nPbExtAddr2 >> 14) & 0x07;

  /* Pack Member: m_nPbExtAddr1 */
  (pToData)[2] |= ((pFrom)->m_nPbExtAddr1 & 0x7f) <<1;
  (pToData)[3] |= ((pFrom)->m_nPbExtAddr1 >> 7) &0xFF;
  (pToData)[4] |= ((pFrom)->m_nPbExtAddr1 >> 15) & 0x03;

  /* Pack Member: m_nPbExtAddr0 */
  (pToData)[0] |= ((pFrom)->m_nPbExtAddr0) & 0xFF;
  (pToData)[1] |= ((pFrom)->m_nPbExtAddr0 >> 8) &0xFF;
  (pToData)[2] |= ((pFrom)->m_nPbExtAddr0 >> 16) & 0x01;
#endif

  return SB_ZF_ZFKAQMFBLINE_SIZE_IN_BYTES;
}




/* Unpack from array of bytes into struct */
void
sbZfKaQmFbLine_Unpack(sbZfKaQmFbLine_t *pToStruct,
                      uint8 *pFromData,
                      uint32 nMaxToDataIndex) {
  COMPILER_UINT64 tmp;

  (void) tmp;

#ifdef SAND_BIG_ENDIAN_HOST

  /* Unpack operation based on bigword endian */

  /* Unpack Member: m_nHec1 */
  (pToStruct)->m_nHec1 =  (uint32)  (pFromData)[12] ;

  /* Unpack Member: m_nHec0 */
  (pToStruct)->m_nHec0 =  (uint32)  (pFromData)[13] ;

  /* Unpack Member: m_nSpare */
  (pToStruct)->m_nSpare =  (uint32)  ((pFromData)[15] >> 6) & 0x03;
  (pToStruct)->m_nSpare |=  (uint32)  (pFromData)[14] << 2;

  /* Unpack Member: m_nPbExtAddr5 */
  (pToStruct)->m_nPbExtAddr5 =  (uint32)  ((pFromData)[9] >> 5) & 0x07;
  (pToStruct)->m_nPbExtAddr5 |=  (uint32)  (pFromData)[8] << 3;
  (pToStruct)->m_nPbExtAddr5 |=  (uint32)  ((pFromData)[15] & 0x3f) << 11;

  /* Unpack Member: m_nPbExtAddr4 */
  (pToStruct)->m_nPbExtAddr4 =  (uint32)  ((pFromData)[11] >> 4) & 0x0f;
  (pToStruct)->m_nPbExtAddr4 |=  (uint32)  (pFromData)[10] << 4;
  (pToStruct)->m_nPbExtAddr4 |=  (uint32)  ((pFromData)[9] & 0x1f) << 12;

  /* Unpack Member: m_nPbExtAddr3 */
  (pToStruct)->m_nPbExtAddr3 =  (uint32)  ((pFromData)[5] >> 3) & 0x1f;
  (pToStruct)->m_nPbExtAddr3 |=  (uint32)  (pFromData)[4] << 5;
  (pToStruct)->m_nPbExtAddr3 |=  (uint32)  ((pFromData)[11] & 0x0f) << 13;

  /* Unpack Member: m_nPbExtAddr2 */
  (pToStruct)->m_nPbExtAddr2 =  (uint32)  ((pFromData)[7] >> 2) & 0x3f;
  (pToStruct)->m_nPbExtAddr2 |=  (uint32)  (pFromData)[6] << 6;
  (pToStruct)->m_nPbExtAddr2 |=  (uint32)  ((pFromData)[5] & 0x07) << 14;

  /* Unpack Member: m_nPbExtAddr1 */
  (pToStruct)->m_nPbExtAddr1 =  (uint32)  ((pFromData)[1] >> 1) & 0x7f;
  (pToStruct)->m_nPbExtAddr1 |=  (uint32)  (pFromData)[0] << 7;
  (pToStruct)->m_nPbExtAddr1 |=  (uint32)  ((pFromData)[7] & 0x03) << 15;

  /* Unpack Member: m_nPbExtAddr0 */
  (pToStruct)->m_nPbExtAddr0 =  (uint32)  (pFromData)[3] ;
  (pToStruct)->m_nPbExtAddr0 |=  (uint32)  (pFromData)[2] << 8;
  (pToStruct)->m_nPbExtAddr0 |=  (uint32)  ((pFromData)[1] & 0x01) << 16;
#else

  /* Unpack operation based on little endian */

  /* Unpack Member: m_nHec1 */
  (pToStruct)->m_nHec1 =  (uint32)  (pFromData)[15] ;

  /* Unpack Member: m_nHec0 */
  (pToStruct)->m_nHec0 =  (uint32)  (pFromData)[14] ;

  /* Unpack Member: m_nSpare */
  (pToStruct)->m_nSpare =  (uint32)  ((pFromData)[12] >> 6) & 0x03;
  (pToStruct)->m_nSpare |=  (uint32)  (pFromData)[13] << 2;

  /* Unpack Member: m_nPbExtAddr5 */
  (pToStruct)->m_nPbExtAddr5 =  (uint32)  ((pFromData)[10] >> 5) & 0x07;
  (pToStruct)->m_nPbExtAddr5 |=  (uint32)  (pFromData)[11] << 3;
  (pToStruct)->m_nPbExtAddr5 |=  (uint32)  ((pFromData)[12] & 0x3f) << 11;

  /* Unpack Member: m_nPbExtAddr4 */
  (pToStruct)->m_nPbExtAddr4 =  (uint32)  ((pFromData)[8] >> 4) & 0x0f;
  (pToStruct)->m_nPbExtAddr4 |=  (uint32)  (pFromData)[9] << 4;
  (pToStruct)->m_nPbExtAddr4 |=  (uint32)  ((pFromData)[10] & 0x1f) << 12;

  /* Unpack Member: m_nPbExtAddr3 */
  (pToStruct)->m_nPbExtAddr3 =  (uint32)  ((pFromData)[6] >> 3) & 0x1f;
  (pToStruct)->m_nPbExtAddr3 |=  (uint32)  (pFromData)[7] << 5;
  (pToStruct)->m_nPbExtAddr3 |=  (uint32)  ((pFromData)[8] & 0x0f) << 13;

  /* Unpack Member: m_nPbExtAddr2 */
  (pToStruct)->m_nPbExtAddr2 =  (uint32)  ((pFromData)[4] >> 2) & 0x3f;
  (pToStruct)->m_nPbExtAddr2 |=  (uint32)  (pFromData)[5] << 6;
  (pToStruct)->m_nPbExtAddr2 |=  (uint32)  ((pFromData)[6] & 0x07) << 14;

  /* Unpack Member: m_nPbExtAddr1 */
  (pToStruct)->m_nPbExtAddr1 =  (uint32)  ((pFromData)[2] >> 1) & 0x7f;
  (pToStruct)->m_nPbExtAddr1 |=  (uint32)  (pFromData)[3] << 7;
  (pToStruct)->m_nPbExtAddr1 |=  (uint32)  ((pFromData)[4] & 0x03) << 15;

  /* Unpack Member: m_nPbExtAddr0 */
  (pToStruct)->m_nPbExtAddr0 =  (uint32)  (pFromData)[0] ;
  (pToStruct)->m_nPbExtAddr0 |=  (uint32)  (pFromData)[1] << 8;
  (pToStruct)->m_nPbExtAddr0 |=  (uint32)  ((pFromData)[2] & 0x01) << 16;
#endif

}



/* initialize an instance of this zframe */
void
sbZfKaQmFbLine_InitInstance(sbZfKaQmFbLine_t *pFrame) {

  pFrame->m_nHec1 =  (unsigned int)  0;
  pFrame->m_nHec0 =  (unsigned int)  0;
  pFrame->m_nSpare =  (unsigned int)  0;
  pFrame->m_nPbExtAddr5 =  (unsigned int)  0;
  pFrame->m_nPbExtAddr4 =  (unsigned int)  0;
  pFrame->m_nPbExtAddr3 =  (unsigned int)  0;
  pFrame->m_nPbExtAddr2 =  (unsigned int)  0;
  pFrame->m_nPbExtAddr1 =  (unsigned int)  0;
  pFrame->m_nPbExtAddr0 =  (unsigned int)  0;

}
