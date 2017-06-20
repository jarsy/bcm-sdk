/*
 * $Id: sbZfKaEpIpMplsLabels.c,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


#include "sbTypes.h"
#include "sbZfKaEpIpMplsLabels.hx"
#include "sbWrappers.h"
#include <sal/types.h>



/* Pack from struct into array of bytes */
uint32
sbZfKaEpIpMplsLabels_Pack(sbZfKaEpIpMplsLabels_t *pFrom,
                          uint8 *pToData,
                          uint32 nMaxToDataIndex) {
#ifdef SAND_BIG_ENDIAN_HOST
  int i;
  int size = SB_ZF_ZFKAEPIPMPLSLABELS_SIZE_IN_BYTES;

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on bigword endian */

  /* Pack Member: m_nLabel1 */
  (pToData)[6] |= ((pFrom)->m_nLabel1 & 0x0f) <<4;
  (pToData)[5] |= ((pFrom)->m_nLabel1 >> 4) &0xFF;
  (pToData)[4] |= ((pFrom)->m_nLabel1 >> 12) &0xFF;

  /* Pack Member: m_nOp */
  (pToData)[6] |= ((pFrom)->m_nOp & 0x01) <<3;

  /* Pack Member: m_nLink */
  (pToData)[6] |= ((pFrom)->m_nLink & 0x03) <<1;

  /* Pack Member: m_nStack1 */
  (pToData)[6] |= ((pFrom)->m_nStack1 & 0x01);

  /* Pack Member: m_nTttl1 */
  (pToData)[7] |= ((pFrom)->m_nTttl1) & 0xFF;

  /* Pack Member: m_nLabel0 */
  (pToData)[2] |= ((pFrom)->m_nLabel0 & 0x0f) <<4;
  (pToData)[1] |= ((pFrom)->m_nLabel0 >> 4) &0xFF;
  (pToData)[0] |= ((pFrom)->m_nLabel0 >> 12) &0xFF;

  /* Pack Member: m_nExp */
  (pToData)[2] |= ((pFrom)->m_nExp & 0x07) <<1;

  /* Pack Member: m_nStack0 */
  (pToData)[2] |= ((pFrom)->m_nStack0 & 0x01);

  /* Pack Member: m_nTttl0 */
  (pToData)[3] |= ((pFrom)->m_nTttl0) & 0xFF;
#else
  int i;
  int size = SB_ZF_ZFKAEPIPMPLSLABELS_SIZE_IN_BYTES;

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on little endian */

  /* Pack Member: m_nLabel1 */
  (pToData)[5] |= ((pFrom)->m_nLabel1 & 0x0f) <<4;
  (pToData)[6] |= ((pFrom)->m_nLabel1 >> 4) &0xFF;
  (pToData)[7] |= ((pFrom)->m_nLabel1 >> 12) &0xFF;

  /* Pack Member: m_nOp */
  (pToData)[5] |= ((pFrom)->m_nOp & 0x01) <<3;

  /* Pack Member: m_nLink */
  (pToData)[5] |= ((pFrom)->m_nLink & 0x03) <<1;

  /* Pack Member: m_nStack1 */
  (pToData)[5] |= ((pFrom)->m_nStack1 & 0x01);

  /* Pack Member: m_nTttl1 */
  (pToData)[4] |= ((pFrom)->m_nTttl1) & 0xFF;

  /* Pack Member: m_nLabel0 */
  (pToData)[1] |= ((pFrom)->m_nLabel0 & 0x0f) <<4;
  (pToData)[2] |= ((pFrom)->m_nLabel0 >> 4) &0xFF;
  (pToData)[3] |= ((pFrom)->m_nLabel0 >> 12) &0xFF;

  /* Pack Member: m_nExp */
  (pToData)[1] |= ((pFrom)->m_nExp & 0x07) <<1;

  /* Pack Member: m_nStack0 */
  (pToData)[1] |= ((pFrom)->m_nStack0 & 0x01);

  /* Pack Member: m_nTttl0 */
  (pToData)[0] |= ((pFrom)->m_nTttl0) & 0xFF;
#endif

  return SB_ZF_ZFKAEPIPMPLSLABELS_SIZE_IN_BYTES;
}




/* Unpack from array of bytes into struct */
void
sbZfKaEpIpMplsLabels_Unpack(sbZfKaEpIpMplsLabels_t *pToStruct,
                            uint8 *pFromData,
                            uint32 nMaxToDataIndex) {
  COMPILER_UINT64 tmp;

  (void) tmp;

#ifdef SAND_BIG_ENDIAN_HOST

  /* Unpack operation based on bigword endian */

  /* Unpack Member: m_nLabel1 */
  (pToStruct)->m_nLabel1 =  (uint32)  ((pFromData)[6] >> 4) & 0x0f;
  (pToStruct)->m_nLabel1 |=  (uint32)  (pFromData)[5] << 4;
  (pToStruct)->m_nLabel1 |=  (uint32)  (pFromData)[4] << 12;

  /* Unpack Member: m_nOp */
  (pToStruct)->m_nOp =  (uint32)  ((pFromData)[6] >> 3) & 0x01;

  /* Unpack Member: m_nLink */
  (pToStruct)->m_nLink =  (uint32)  ((pFromData)[6] >> 1) & 0x03;

  /* Unpack Member: m_nStack1 */
  (pToStruct)->m_nStack1 =  (uint32)  ((pFromData)[6] ) & 0x01;

  /* Unpack Member: m_nTttl1 */
  (pToStruct)->m_nTttl1 =  (uint32)  (pFromData)[7] ;

  /* Unpack Member: m_nLabel0 */
  (pToStruct)->m_nLabel0 =  (uint32)  ((pFromData)[2] >> 4) & 0x0f;
  (pToStruct)->m_nLabel0 |=  (uint32)  (pFromData)[1] << 4;
  (pToStruct)->m_nLabel0 |=  (uint32)  (pFromData)[0] << 12;

  /* Unpack Member: m_nExp */
  (pToStruct)->m_nExp =  (uint32)  ((pFromData)[2] >> 1) & 0x07;

  /* Unpack Member: m_nStack0 */
  (pToStruct)->m_nStack0 =  (uint32)  ((pFromData)[2] ) & 0x01;

  /* Unpack Member: m_nTttl0 */
  (pToStruct)->m_nTttl0 =  (uint32)  (pFromData)[3] ;
#else

  /* Unpack operation based on little endian */

  /* Unpack Member: m_nLabel1 */
  (pToStruct)->m_nLabel1 =  (uint32)  ((pFromData)[5] >> 4) & 0x0f;
  (pToStruct)->m_nLabel1 |=  (uint32)  (pFromData)[6] << 4;
  (pToStruct)->m_nLabel1 |=  (uint32)  (pFromData)[7] << 12;

  /* Unpack Member: m_nOp */
  (pToStruct)->m_nOp =  (uint32)  ((pFromData)[5] >> 3) & 0x01;

  /* Unpack Member: m_nLink */
  (pToStruct)->m_nLink =  (uint32)  ((pFromData)[5] >> 1) & 0x03;

  /* Unpack Member: m_nStack1 */
  (pToStruct)->m_nStack1 =  (uint32)  ((pFromData)[5] ) & 0x01;

  /* Unpack Member: m_nTttl1 */
  (pToStruct)->m_nTttl1 =  (uint32)  (pFromData)[4] ;

  /* Unpack Member: m_nLabel0 */
  (pToStruct)->m_nLabel0 =  (uint32)  ((pFromData)[1] >> 4) & 0x0f;
  (pToStruct)->m_nLabel0 |=  (uint32)  (pFromData)[2] << 4;
  (pToStruct)->m_nLabel0 |=  (uint32)  (pFromData)[3] << 12;

  /* Unpack Member: m_nExp */
  (pToStruct)->m_nExp =  (uint32)  ((pFromData)[1] >> 1) & 0x07;

  /* Unpack Member: m_nStack0 */
  (pToStruct)->m_nStack0 =  (uint32)  ((pFromData)[1] ) & 0x01;

  /* Unpack Member: m_nTttl0 */
  (pToStruct)->m_nTttl0 =  (uint32)  (pFromData)[0] ;
#endif

}



/* initialize an instance of this zframe */
void
sbZfKaEpIpMplsLabels_InitInstance(sbZfKaEpIpMplsLabels_t *pFrame) {

  pFrame->m_nLabel1 =  (unsigned int)  0;
  pFrame->m_nOp =  (unsigned int)  0;
  pFrame->m_nLink =  (unsigned int)  0;
  pFrame->m_nStack1 =  (unsigned int)  0;
  pFrame->m_nTttl1 =  (unsigned int)  0;
  pFrame->m_nLabel0 =  (unsigned int)  0;
  pFrame->m_nExp =  (unsigned int)  0;
  pFrame->m_nStack0 =  (unsigned int)  0;
  pFrame->m_nTttl0 =  (unsigned int)  0;

}
