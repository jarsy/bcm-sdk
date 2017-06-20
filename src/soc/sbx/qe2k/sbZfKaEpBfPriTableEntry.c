/*
 * $Id: sbZfKaEpBfPriTableEntry.c,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


#include "sbTypes.h"
#include "sbZfKaEpBfPriTableAddr.hx"
#include "sbZfKaEpBfPriTableEntry.hx"
#include "sbWrappers.h"
#include <sal/types.h>



/* Pack from struct into array of bytes */
uint32
sbZfKaEpBfPriTableEntry_Pack(sbZfKaEpBfPriTableEntry_t *pFrom,
                             uint8 *pToData,
                             uint32 nMaxToDataIndex) {
#ifdef SAND_BIG_ENDIAN_HOST
  int i;
  int size = SB_ZF_ZFKAEPBFPRITABLEENTRY_SIZE_IN_BYTES;

  if (size % 4) {
    size += (4 - size %4);
  }

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on bigword endian */

  /* Pack Member: m_nPri7 */
  (pToData)[1] |= ((pFrom)->m_nPri7 & 0x07) <<5;

  /* Pack Member: m_nPri6 */
  (pToData)[1] |= ((pFrom)->m_nPri6 & 0x07) <<2;

  /* Pack Member: m_nPri5 */
  (pToData)[2] |= ((pFrom)->m_nPri5 & 0x01) <<7;
  (pToData)[1] |= ((pFrom)->m_nPri5 >> 1) & 0x03;

  /* Pack Member: m_nPri4 */
  (pToData)[2] |= ((pFrom)->m_nPri4 & 0x07) <<4;

  /* Pack Member: m_nPri3 */
  (pToData)[2] |= ((pFrom)->m_nPri3 & 0x07) <<1;

  /* Pack Member: m_nPri2 */
  (pToData)[3] |= ((pFrom)->m_nPri2 & 0x03) <<6;
  (pToData)[2] |= ((pFrom)->m_nPri2 >> 2) & 0x01;

  /* Pack Member: m_nPri1 */
  (pToData)[3] |= ((pFrom)->m_nPri1 & 0x07) <<3;

  /* Pack Member: m_nPri0 */
  (pToData)[3] |= ((pFrom)->m_nPri0 & 0x07);
#else
  int i;
  int size = SB_ZF_ZFKAEPBFPRITABLEENTRY_SIZE_IN_BYTES;

  if (size % 4) {
    size += (4 - size %4);
  }

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on little endian */

  /* Pack Member: m_nPri7 */
  (pToData)[2] |= ((pFrom)->m_nPri7 & 0x07) <<5;

  /* Pack Member: m_nPri6 */
  (pToData)[2] |= ((pFrom)->m_nPri6 & 0x07) <<2;

  /* Pack Member: m_nPri5 */
  (pToData)[1] |= ((pFrom)->m_nPri5 & 0x01) <<7;
  (pToData)[2] |= ((pFrom)->m_nPri5 >> 1) & 0x03;

  /* Pack Member: m_nPri4 */
  (pToData)[1] |= ((pFrom)->m_nPri4 & 0x07) <<4;

  /* Pack Member: m_nPri3 */
  (pToData)[1] |= ((pFrom)->m_nPri3 & 0x07) <<1;

  /* Pack Member: m_nPri2 */
  (pToData)[0] |= ((pFrom)->m_nPri2 & 0x03) <<6;
  (pToData)[1] |= ((pFrom)->m_nPri2 >> 2) & 0x01;

  /* Pack Member: m_nPri1 */
  (pToData)[0] |= ((pFrom)->m_nPri1 & 0x07) <<3;

  /* Pack Member: m_nPri0 */
  (pToData)[0] |= ((pFrom)->m_nPri0 & 0x07);
#endif

  return SB_ZF_ZFKAEPBFPRITABLEENTRY_SIZE_IN_BYTES;
}




/* Unpack from array of bytes into struct */
void
sbZfKaEpBfPriTableEntry_Unpack(sbZfKaEpBfPriTableEntry_t *pToStruct,
                               uint8 *pFromData,
                               uint32 nMaxToDataIndex) {
  COMPILER_UINT64 tmp;

  (void) tmp;

#ifdef SAND_BIG_ENDIAN_HOST

  /* Unpack operation based on bigword endian */

  /* Unpack Member: m_nPri7 */
  (pToStruct)->m_nPri7 =  (uint32)  ((pFromData)[1] >> 5) & 0x07;

  /* Unpack Member: m_nPri6 */
  (pToStruct)->m_nPri6 =  (uint32)  ((pFromData)[1] >> 2) & 0x07;

  /* Unpack Member: m_nPri5 */
  (pToStruct)->m_nPri5 =  (uint32)  ((pFromData)[2] >> 7) & 0x01;
  (pToStruct)->m_nPri5 |=  (uint32)  ((pFromData)[1] & 0x03) << 1;

  /* Unpack Member: m_nPri4 */
  (pToStruct)->m_nPri4 =  (uint32)  ((pFromData)[2] >> 4) & 0x07;

  /* Unpack Member: m_nPri3 */
  (pToStruct)->m_nPri3 =  (uint32)  ((pFromData)[2] >> 1) & 0x07;

  /* Unpack Member: m_nPri2 */
  (pToStruct)->m_nPri2 =  (uint32)  ((pFromData)[3] >> 6) & 0x03;
  (pToStruct)->m_nPri2 |=  (uint32)  ((pFromData)[2] & 0x01) << 2;

  /* Unpack Member: m_nPri1 */
  (pToStruct)->m_nPri1 =  (uint32)  ((pFromData)[3] >> 3) & 0x07;

  /* Unpack Member: m_nPri0 */
  (pToStruct)->m_nPri0 =  (uint32)  ((pFromData)[3] ) & 0x07;
#else

  /* Unpack operation based on little endian */

  /* Unpack Member: m_nPri7 */
  (pToStruct)->m_nPri7 =  (uint32)  ((pFromData)[2] >> 5) & 0x07;

  /* Unpack Member: m_nPri6 */
  (pToStruct)->m_nPri6 =  (uint32)  ((pFromData)[2] >> 2) & 0x07;

  /* Unpack Member: m_nPri5 */
  (pToStruct)->m_nPri5 =  (uint32)  ((pFromData)[1] >> 7) & 0x01;
  (pToStruct)->m_nPri5 |=  (uint32)  ((pFromData)[2] & 0x03) << 1;

  /* Unpack Member: m_nPri4 */
  (pToStruct)->m_nPri4 =  (uint32)  ((pFromData)[1] >> 4) & 0x07;

  /* Unpack Member: m_nPri3 */
  (pToStruct)->m_nPri3 =  (uint32)  ((pFromData)[1] >> 1) & 0x07;

  /* Unpack Member: m_nPri2 */
  (pToStruct)->m_nPri2 =  (uint32)  ((pFromData)[0] >> 6) & 0x03;
  (pToStruct)->m_nPri2 |=  (uint32)  ((pFromData)[1] & 0x01) << 2;

  /* Unpack Member: m_nPri1 */
  (pToStruct)->m_nPri1 =  (uint32)  ((pFromData)[0] >> 3) & 0x07;

  /* Unpack Member: m_nPri0 */
  (pToStruct)->m_nPri0 =  (uint32)  ((pFromData)[0] ) & 0x07;
#endif

}



/* initialize an instance of this zframe */
void
sbZfKaEpBfPriTableEntry_InitInstance(sbZfKaEpBfPriTableEntry_t *pFrame) {

  pFrame->m_nPri7 =  (unsigned int)  0;
  pFrame->m_nPri6 =  (unsigned int)  0;
  pFrame->m_nPri5 =  (unsigned int)  0;
  pFrame->m_nPri4 =  (unsigned int)  0;
  pFrame->m_nPri3 =  (unsigned int)  0;
  pFrame->m_nPri2 =  (unsigned int)  0;
  pFrame->m_nPri1 =  (unsigned int)  0;
  pFrame->m_nPri0 =  (unsigned int)  0;

}
