/*
 * $Id: sbZfKaQsLnaNextPriEntry.c,v 1.3 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


#include "sbTypes.h"
#include "sbZfKaQsLnaNextPriEntry.hx"
#include "sbWrappers.h"
#include <sal/types.h>



/* Pack from struct into array of bytes */
uint32
sbZfKaQsLnaNextPriEntry_Pack(sbZfKaQsLnaNextPriEntry_t *pFrom,
                             uint8 *pToData,
                             uint32 nMaxToDataIndex) {
#ifdef SAND_BIG_ENDIAN_HOST
  int i;
  int size = SB_ZF_ZFKAQSLNANEXTPRIENTRY_SIZE_IN_BYTES;

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on bigword endian */

  /* Pack Member: m_nSelPort */
  (pToData)[15] |= ((pFrom)->m_nSelPort & 0x0f) <<4;
  (pToData)[14] |= ((pFrom)->m_nSelPort >> 4) & 0x03;

  /* Pack Member: m_nNextPri24 */
  (pToData)[15] |= ((pFrom)->m_nNextPri24 & 0x0f);

  /* Pack Member: m_nNextPri23 */
  (pToData)[8] |= ((pFrom)->m_nNextPri23 & 0x0f) <<4;

  /* Pack Member: m_nNextPri22 */
  (pToData)[8] |= ((pFrom)->m_nNextPri22 & 0x0f);

  /* Pack Member: m_nNextPri21 */
  (pToData)[9] |= ((pFrom)->m_nNextPri21 & 0x0f) <<4;

  /* Pack Member: m_nNextPri20 */
  (pToData)[9] |= ((pFrom)->m_nNextPri20 & 0x0f);

  /* Pack Member: m_nNextPri19 */
  (pToData)[10] |= ((pFrom)->m_nNextPri19 & 0x0f) <<4;

  /* Pack Member: m_nNextPri18 */
  (pToData)[10] |= ((pFrom)->m_nNextPri18 & 0x0f);

  /* Pack Member: m_nNextPri17 */
  (pToData)[11] |= ((pFrom)->m_nNextPri17 & 0x0f) <<4;

  /* Pack Member: m_nNextPri16 */
  (pToData)[11] |= ((pFrom)->m_nNextPri16 & 0x0f);

  /* Pack Member: m_nNextPri15 */
  (pToData)[4] |= ((pFrom)->m_nNextPri15 & 0x0f) <<4;

  /* Pack Member: m_nNextPri14 */
  (pToData)[4] |= ((pFrom)->m_nNextPri14 & 0x0f);

  /* Pack Member: m_nNextPri13 */
  (pToData)[5] |= ((pFrom)->m_nNextPri13 & 0x0f) <<4;

  /* Pack Member: m_nNextPri12 */
  (pToData)[5] |= ((pFrom)->m_nNextPri12 & 0x0f);

  /* Pack Member: m_nNextPri11 */
  (pToData)[6] |= ((pFrom)->m_nNextPri11 & 0x0f) <<4;

  /* Pack Member: m_nNextPri10 */
  (pToData)[6] |= ((pFrom)->m_nNextPri10 & 0x0f);

  /* Pack Member: m_nNextPri9 */
  (pToData)[7] |= ((pFrom)->m_nNextPri9 & 0x0f) <<4;

  /* Pack Member: m_nNextPri8 */
  (pToData)[7] |= ((pFrom)->m_nNextPri8 & 0x0f);

  /* Pack Member: m_nNextPri7 */
  (pToData)[0] |= ((pFrom)->m_nNextPri7 & 0x0f) <<4;

  /* Pack Member: m_nNextPri6 */
  (pToData)[0] |= ((pFrom)->m_nNextPri6 & 0x0f);

  /* Pack Member: m_nNextPri5 */
  (pToData)[1] |= ((pFrom)->m_nNextPri5 & 0x0f) <<4;

  /* Pack Member: m_nNextPri4 */
  (pToData)[1] |= ((pFrom)->m_nNextPri4 & 0x0f);

  /* Pack Member: m_nNextPri3 */
  (pToData)[2] |= ((pFrom)->m_nNextPri3 & 0x0f) <<4;

  /* Pack Member: m_nNextPri2 */
  (pToData)[2] |= ((pFrom)->m_nNextPri2 & 0x0f);

  /* Pack Member: m_nNextPri1 */
  (pToData)[3] |= ((pFrom)->m_nNextPri1 & 0x0f) <<4;

  /* Pack Member: m_nNextPri0 */
  (pToData)[3] |= ((pFrom)->m_nNextPri0 & 0x0f);
#else
  int i;
  int size = SB_ZF_ZFKAQSLNANEXTPRIENTRY_SIZE_IN_BYTES;

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on little endian */

  /* Pack Member: m_nSelPort */
  (pToData)[12] |= ((pFrom)->m_nSelPort & 0x0f) <<4;
  (pToData)[13] |= ((pFrom)->m_nSelPort >> 4) & 0x03;

  /* Pack Member: m_nNextPri24 */
  (pToData)[12] |= ((pFrom)->m_nNextPri24 & 0x0f);

  /* Pack Member: m_nNextPri23 */
  (pToData)[11] |= ((pFrom)->m_nNextPri23 & 0x0f) <<4;

  /* Pack Member: m_nNextPri22 */
  (pToData)[11] |= ((pFrom)->m_nNextPri22 & 0x0f);

  /* Pack Member: m_nNextPri21 */
  (pToData)[10] |= ((pFrom)->m_nNextPri21 & 0x0f) <<4;

  /* Pack Member: m_nNextPri20 */
  (pToData)[10] |= ((pFrom)->m_nNextPri20 & 0x0f);

  /* Pack Member: m_nNextPri19 */
  (pToData)[9] |= ((pFrom)->m_nNextPri19 & 0x0f) <<4;

  /* Pack Member: m_nNextPri18 */
  (pToData)[9] |= ((pFrom)->m_nNextPri18 & 0x0f);

  /* Pack Member: m_nNextPri17 */
  (pToData)[8] |= ((pFrom)->m_nNextPri17 & 0x0f) <<4;

  /* Pack Member: m_nNextPri16 */
  (pToData)[8] |= ((pFrom)->m_nNextPri16 & 0x0f);

  /* Pack Member: m_nNextPri15 */
  (pToData)[7] |= ((pFrom)->m_nNextPri15 & 0x0f) <<4;

  /* Pack Member: m_nNextPri14 */
  (pToData)[7] |= ((pFrom)->m_nNextPri14 & 0x0f);

  /* Pack Member: m_nNextPri13 */
  (pToData)[6] |= ((pFrom)->m_nNextPri13 & 0x0f) <<4;

  /* Pack Member: m_nNextPri12 */
  (pToData)[6] |= ((pFrom)->m_nNextPri12 & 0x0f);

  /* Pack Member: m_nNextPri11 */
  (pToData)[5] |= ((pFrom)->m_nNextPri11 & 0x0f) <<4;

  /* Pack Member: m_nNextPri10 */
  (pToData)[5] |= ((pFrom)->m_nNextPri10 & 0x0f);

  /* Pack Member: m_nNextPri9 */
  (pToData)[4] |= ((pFrom)->m_nNextPri9 & 0x0f) <<4;

  /* Pack Member: m_nNextPri8 */
  (pToData)[4] |= ((pFrom)->m_nNextPri8 & 0x0f);

  /* Pack Member: m_nNextPri7 */
  (pToData)[3] |= ((pFrom)->m_nNextPri7 & 0x0f) <<4;

  /* Pack Member: m_nNextPri6 */
  (pToData)[3] |= ((pFrom)->m_nNextPri6 & 0x0f);

  /* Pack Member: m_nNextPri5 */
  (pToData)[2] |= ((pFrom)->m_nNextPri5 & 0x0f) <<4;

  /* Pack Member: m_nNextPri4 */
  (pToData)[2] |= ((pFrom)->m_nNextPri4 & 0x0f);

  /* Pack Member: m_nNextPri3 */
  (pToData)[1] |= ((pFrom)->m_nNextPri3 & 0x0f) <<4;

  /* Pack Member: m_nNextPri2 */
  (pToData)[1] |= ((pFrom)->m_nNextPri2 & 0x0f);

  /* Pack Member: m_nNextPri1 */
  (pToData)[0] |= ((pFrom)->m_nNextPri1 & 0x0f) <<4;

  /* Pack Member: m_nNextPri0 */
  (pToData)[0] |= ((pFrom)->m_nNextPri0 & 0x0f);
#endif

  return SB_ZF_ZFKAQSLNANEXTPRIENTRY_SIZE_IN_BYTES;
}




/* Unpack from array of bytes into struct */
void
sbZfKaQsLnaNextPriEntry_Unpack(sbZfKaQsLnaNextPriEntry_t *pToStruct,
                               uint8 *pFromData,
                               uint32 nMaxToDataIndex) {
  COMPILER_UINT64 tmp;

  (void) tmp;

#ifdef SAND_BIG_ENDIAN_HOST

  /* Unpack operation based on bigword endian */

  /* Unpack Member: m_nSelPort */
  (pToStruct)->m_nSelPort =  (uint32)  ((pFromData)[15] >> 4) & 0x0f;
  (pToStruct)->m_nSelPort |=  (uint32)  ((pFromData)[14] & 0x03) << 4;

  /* Unpack Member: m_nNextPri24 */
  (pToStruct)->m_nNextPri24 =  (uint32)  ((pFromData)[15] ) & 0x0f;

  /* Unpack Member: m_nNextPri23 */
  (pToStruct)->m_nNextPri23 =  (uint32)  ((pFromData)[8] >> 4) & 0x0f;

  /* Unpack Member: m_nNextPri22 */
  (pToStruct)->m_nNextPri22 =  (uint32)  ((pFromData)[8] ) & 0x0f;

  /* Unpack Member: m_nNextPri21 */
  (pToStruct)->m_nNextPri21 =  (uint32)  ((pFromData)[9] >> 4) & 0x0f;

  /* Unpack Member: m_nNextPri20 */
  (pToStruct)->m_nNextPri20 =  (uint32)  ((pFromData)[9] ) & 0x0f;

  /* Unpack Member: m_nNextPri19 */
  (pToStruct)->m_nNextPri19 =  (uint32)  ((pFromData)[10] >> 4) & 0x0f;

  /* Unpack Member: m_nNextPri18 */
  (pToStruct)->m_nNextPri18 =  (uint32)  ((pFromData)[10] ) & 0x0f;

  /* Unpack Member: m_nNextPri17 */
  (pToStruct)->m_nNextPri17 =  (uint32)  ((pFromData)[11] >> 4) & 0x0f;

  /* Unpack Member: m_nNextPri16 */
  (pToStruct)->m_nNextPri16 =  (uint32)  ((pFromData)[11] ) & 0x0f;

  /* Unpack Member: m_nNextPri15 */
  (pToStruct)->m_nNextPri15 =  (uint32)  ((pFromData)[4] >> 4) & 0x0f;

  /* Unpack Member: m_nNextPri14 */
  (pToStruct)->m_nNextPri14 =  (uint32)  ((pFromData)[4] ) & 0x0f;

  /* Unpack Member: m_nNextPri13 */
  (pToStruct)->m_nNextPri13 =  (uint32)  ((pFromData)[5] >> 4) & 0x0f;

  /* Unpack Member: m_nNextPri12 */
  (pToStruct)->m_nNextPri12 =  (uint32)  ((pFromData)[5] ) & 0x0f;

  /* Unpack Member: m_nNextPri11 */
  (pToStruct)->m_nNextPri11 =  (uint32)  ((pFromData)[6] >> 4) & 0x0f;

  /* Unpack Member: m_nNextPri10 */
  (pToStruct)->m_nNextPri10 =  (uint32)  ((pFromData)[6] ) & 0x0f;

  /* Unpack Member: m_nNextPri9 */
  (pToStruct)->m_nNextPri9 =  (uint32)  ((pFromData)[7] >> 4) & 0x0f;

  /* Unpack Member: m_nNextPri8 */
  (pToStruct)->m_nNextPri8 =  (uint32)  ((pFromData)[7] ) & 0x0f;

  /* Unpack Member: m_nNextPri7 */
  (pToStruct)->m_nNextPri7 =  (uint32)  ((pFromData)[0] >> 4) & 0x0f;

  /* Unpack Member: m_nNextPri6 */
  (pToStruct)->m_nNextPri6 =  (uint32)  ((pFromData)[0] ) & 0x0f;

  /* Unpack Member: m_nNextPri5 */
  (pToStruct)->m_nNextPri5 =  (uint32)  ((pFromData)[1] >> 4) & 0x0f;

  /* Unpack Member: m_nNextPri4 */
  (pToStruct)->m_nNextPri4 =  (uint32)  ((pFromData)[1] ) & 0x0f;

  /* Unpack Member: m_nNextPri3 */
  (pToStruct)->m_nNextPri3 =  (uint32)  ((pFromData)[2] >> 4) & 0x0f;

  /* Unpack Member: m_nNextPri2 */
  (pToStruct)->m_nNextPri2 =  (uint32)  ((pFromData)[2] ) & 0x0f;

  /* Unpack Member: m_nNextPri1 */
  (pToStruct)->m_nNextPri1 =  (uint32)  ((pFromData)[3] >> 4) & 0x0f;

  /* Unpack Member: m_nNextPri0 */
  (pToStruct)->m_nNextPri0 =  (uint32)  ((pFromData)[3] ) & 0x0f;
#else

  /* Unpack operation based on little endian */

  /* Unpack Member: m_nSelPort */
  (pToStruct)->m_nSelPort =  (uint32)  ((pFromData)[12] >> 4) & 0x0f;
  (pToStruct)->m_nSelPort |=  (uint32)  ((pFromData)[13] & 0x03) << 4;

  /* Unpack Member: m_nNextPri24 */
  (pToStruct)->m_nNextPri24 =  (uint32)  ((pFromData)[12] ) & 0x0f;

  /* Unpack Member: m_nNextPri23 */
  (pToStruct)->m_nNextPri23 =  (uint32)  ((pFromData)[11] >> 4) & 0x0f;

  /* Unpack Member: m_nNextPri22 */
  (pToStruct)->m_nNextPri22 =  (uint32)  ((pFromData)[11] ) & 0x0f;

  /* Unpack Member: m_nNextPri21 */
  (pToStruct)->m_nNextPri21 =  (uint32)  ((pFromData)[10] >> 4) & 0x0f;

  /* Unpack Member: m_nNextPri20 */
  (pToStruct)->m_nNextPri20 =  (uint32)  ((pFromData)[10] ) & 0x0f;

  /* Unpack Member: m_nNextPri19 */
  (pToStruct)->m_nNextPri19 =  (uint32)  ((pFromData)[9] >> 4) & 0x0f;

  /* Unpack Member: m_nNextPri18 */
  (pToStruct)->m_nNextPri18 =  (uint32)  ((pFromData)[9] ) & 0x0f;

  /* Unpack Member: m_nNextPri17 */
  (pToStruct)->m_nNextPri17 =  (uint32)  ((pFromData)[8] >> 4) & 0x0f;

  /* Unpack Member: m_nNextPri16 */
  (pToStruct)->m_nNextPri16 =  (uint32)  ((pFromData)[8] ) & 0x0f;

  /* Unpack Member: m_nNextPri15 */
  (pToStruct)->m_nNextPri15 =  (uint32)  ((pFromData)[7] >> 4) & 0x0f;

  /* Unpack Member: m_nNextPri14 */
  (pToStruct)->m_nNextPri14 =  (uint32)  ((pFromData)[7] ) & 0x0f;

  /* Unpack Member: m_nNextPri13 */
  (pToStruct)->m_nNextPri13 =  (uint32)  ((pFromData)[6] >> 4) & 0x0f;

  /* Unpack Member: m_nNextPri12 */
  (pToStruct)->m_nNextPri12 =  (uint32)  ((pFromData)[6] ) & 0x0f;

  /* Unpack Member: m_nNextPri11 */
  (pToStruct)->m_nNextPri11 =  (uint32)  ((pFromData)[5] >> 4) & 0x0f;

  /* Unpack Member: m_nNextPri10 */
  (pToStruct)->m_nNextPri10 =  (uint32)  ((pFromData)[5] ) & 0x0f;

  /* Unpack Member: m_nNextPri9 */
  (pToStruct)->m_nNextPri9 =  (uint32)  ((pFromData)[4] >> 4) & 0x0f;

  /* Unpack Member: m_nNextPri8 */
  (pToStruct)->m_nNextPri8 =  (uint32)  ((pFromData)[4] ) & 0x0f;

  /* Unpack Member: m_nNextPri7 */
  (pToStruct)->m_nNextPri7 =  (uint32)  ((pFromData)[3] >> 4) & 0x0f;

  /* Unpack Member: m_nNextPri6 */
  (pToStruct)->m_nNextPri6 =  (uint32)  ((pFromData)[3] ) & 0x0f;

  /* Unpack Member: m_nNextPri5 */
  (pToStruct)->m_nNextPri5 =  (uint32)  ((pFromData)[2] >> 4) & 0x0f;

  /* Unpack Member: m_nNextPri4 */
  (pToStruct)->m_nNextPri4 =  (uint32)  ((pFromData)[2] ) & 0x0f;

  /* Unpack Member: m_nNextPri3 */
  (pToStruct)->m_nNextPri3 =  (uint32)  ((pFromData)[1] >> 4) & 0x0f;

  /* Unpack Member: m_nNextPri2 */
  (pToStruct)->m_nNextPri2 =  (uint32)  ((pFromData)[1] ) & 0x0f;

  /* Unpack Member: m_nNextPri1 */
  (pToStruct)->m_nNextPri1 =  (uint32)  ((pFromData)[0] >> 4) & 0x0f;

  /* Unpack Member: m_nNextPri0 */
  (pToStruct)->m_nNextPri0 =  (uint32)  ((pFromData)[0] ) & 0x0f;
#endif

}



/* initialize an instance of this zframe */
void
sbZfKaQsLnaNextPriEntry_InitInstance(sbZfKaQsLnaNextPriEntry_t *pFrame) {

  pFrame->m_nSelPort =  (unsigned int)  0;
  pFrame->m_nNextPri24 =  (unsigned int)  0;
  pFrame->m_nNextPri23 =  (unsigned int)  0;
  pFrame->m_nNextPri22 =  (unsigned int)  0;
  pFrame->m_nNextPri21 =  (unsigned int)  0;
  pFrame->m_nNextPri20 =  (unsigned int)  0;
  pFrame->m_nNextPri19 =  (unsigned int)  0;
  pFrame->m_nNextPri18 =  (unsigned int)  0;
  pFrame->m_nNextPri17 =  (unsigned int)  0;
  pFrame->m_nNextPri16 =  (unsigned int)  0;
  pFrame->m_nNextPri15 =  (unsigned int)  0;
  pFrame->m_nNextPri14 =  (unsigned int)  0;
  pFrame->m_nNextPri13 =  (unsigned int)  0;
  pFrame->m_nNextPri12 =  (unsigned int)  0;
  pFrame->m_nNextPri11 =  (unsigned int)  0;
  pFrame->m_nNextPri10 =  (unsigned int)  0;
  pFrame->m_nNextPri9 =  (unsigned int)  0;
  pFrame->m_nNextPri8 =  (unsigned int)  0;
  pFrame->m_nNextPri7 =  (unsigned int)  0;
  pFrame->m_nNextPri6 =  (unsigned int)  0;
  pFrame->m_nNextPri5 =  (unsigned int)  0;
  pFrame->m_nNextPri4 =  (unsigned int)  0;
  pFrame->m_nNextPri3 =  (unsigned int)  0;
  pFrame->m_nNextPri2 =  (unsigned int)  0;
  pFrame->m_nNextPri1 =  (unsigned int)  0;
  pFrame->m_nNextPri0 =  (unsigned int)  0;

}
