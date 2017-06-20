/*
 * $Id: sbZfKaQsLnaPriEntry.c,v 1.3 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


#include "sbTypes.h"
#include "sbZfKaQsLnaPriEntry.hx"
#include "sbWrappers.h"
#include <sal/types.h>



/* Pack from struct into array of bytes */
uint32
sbZfKaQsLnaPriEntry_Pack(sbZfKaQsLnaPriEntry_t *pFrom,
                         uint8 *pToData,
                         uint32 nMaxToDataIndex) {
#ifdef SAND_BIG_ENDIAN_HOST
  int i;
  int size = SB_ZF_ZFKAQSLNAPRIENTRY_SIZE_IN_BYTES;

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on bigword endian */

  /* Pack Member: m_nPri4 */
  (pToData)[19] |= ((pFrom)->m_nPri4) & 0xFF;
  (pToData)[18] |= ((pFrom)->m_nPri4 >> 8) &0xFF;
  (pToData)[17] |= ((pFrom)->m_nPri4 >> 16) &0xFF;
  (pToData)[16] |= ((pFrom)->m_nPri4 >> 24) &0xFF;

  /* Pack Member: m_nPri3 */
  (pToData)[15] |= ((pFrom)->m_nPri3) & 0xFF;
  (pToData)[14] |= ((pFrom)->m_nPri3 >> 8) &0xFF;
  (pToData)[13] |= ((pFrom)->m_nPri3 >> 16) &0xFF;
  (pToData)[12] |= ((pFrom)->m_nPri3 >> 24) &0xFF;

  /* Pack Member: m_nPri2 */
  (pToData)[11] |= ((pFrom)->m_nPri2) & 0xFF;
  (pToData)[10] |= ((pFrom)->m_nPri2 >> 8) &0xFF;
  (pToData)[9] |= ((pFrom)->m_nPri2 >> 16) &0xFF;
  (pToData)[8] |= ((pFrom)->m_nPri2 >> 24) &0xFF;

  /* Pack Member: m_nPri1 */
  (pToData)[7] |= ((pFrom)->m_nPri1) & 0xFF;
  (pToData)[6] |= ((pFrom)->m_nPri1 >> 8) &0xFF;
  (pToData)[5] |= ((pFrom)->m_nPri1 >> 16) &0xFF;
  (pToData)[4] |= ((pFrom)->m_nPri1 >> 24) &0xFF;

  /* Pack Member: m_nPri0 */
  (pToData)[3] |= ((pFrom)->m_nPri0) & 0xFF;
  (pToData)[2] |= ((pFrom)->m_nPri0 >> 8) &0xFF;
  (pToData)[1] |= ((pFrom)->m_nPri0 >> 16) &0xFF;
  (pToData)[0] |= ((pFrom)->m_nPri0 >> 24) &0xFF;
#else
  int i;
  int size = SB_ZF_ZFKAQSLNAPRIENTRY_SIZE_IN_BYTES;

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on little endian */

  /* Pack Member: m_nPri4 */
  (pToData)[16] |= ((pFrom)->m_nPri4) & 0xFF;
  (pToData)[17] |= ((pFrom)->m_nPri4 >> 8) &0xFF;
  (pToData)[18] |= ((pFrom)->m_nPri4 >> 16) &0xFF;
  (pToData)[19] |= ((pFrom)->m_nPri4 >> 24) &0xFF;

  /* Pack Member: m_nPri3 */
  (pToData)[12] |= ((pFrom)->m_nPri3) & 0xFF;
  (pToData)[13] |= ((pFrom)->m_nPri3 >> 8) &0xFF;
  (pToData)[14] |= ((pFrom)->m_nPri3 >> 16) &0xFF;
  (pToData)[15] |= ((pFrom)->m_nPri3 >> 24) &0xFF;

  /* Pack Member: m_nPri2 */
  (pToData)[8] |= ((pFrom)->m_nPri2) & 0xFF;
  (pToData)[9] |= ((pFrom)->m_nPri2 >> 8) &0xFF;
  (pToData)[10] |= ((pFrom)->m_nPri2 >> 16) &0xFF;
  (pToData)[11] |= ((pFrom)->m_nPri2 >> 24) &0xFF;

  /* Pack Member: m_nPri1 */
  (pToData)[4] |= ((pFrom)->m_nPri1) & 0xFF;
  (pToData)[5] |= ((pFrom)->m_nPri1 >> 8) &0xFF;
  (pToData)[6] |= ((pFrom)->m_nPri1 >> 16) &0xFF;
  (pToData)[7] |= ((pFrom)->m_nPri1 >> 24) &0xFF;

  /* Pack Member: m_nPri0 */
  (pToData)[0] |= ((pFrom)->m_nPri0) & 0xFF;
  (pToData)[1] |= ((pFrom)->m_nPri0 >> 8) &0xFF;
  (pToData)[2] |= ((pFrom)->m_nPri0 >> 16) &0xFF;
  (pToData)[3] |= ((pFrom)->m_nPri0 >> 24) &0xFF;
#endif

  return SB_ZF_ZFKAQSLNAPRIENTRY_SIZE_IN_BYTES;
}




/* Unpack from array of bytes into struct */
void
sbZfKaQsLnaPriEntry_Unpack(sbZfKaQsLnaPriEntry_t *pToStruct,
                           uint8 *pFromData,
                           uint32 nMaxToDataIndex) {
  COMPILER_UINT64 tmp;

  (void) tmp;

#ifdef SAND_BIG_ENDIAN_HOST

  /* Unpack operation based on bigword endian */

  /* Unpack Member: m_nPri4 */
  (pToStruct)->m_nPri4 =  (uint32)  (pFromData)[19] ;
  (pToStruct)->m_nPri4 |=  (uint32)  (pFromData)[18] << 8;
  (pToStruct)->m_nPri4 |=  (uint32)  (pFromData)[17] << 16;
  (pToStruct)->m_nPri4 |=  (uint32)  (pFromData)[16] << 24;

  /* Unpack Member: m_nPri3 */
  (pToStruct)->m_nPri3 =  (uint32)  (pFromData)[15] ;
  (pToStruct)->m_nPri3 |=  (uint32)  (pFromData)[14] << 8;
  (pToStruct)->m_nPri3 |=  (uint32)  (pFromData)[13] << 16;
  (pToStruct)->m_nPri3 |=  (uint32)  (pFromData)[12] << 24;

  /* Unpack Member: m_nPri2 */
  (pToStruct)->m_nPri2 =  (uint32)  (pFromData)[11] ;
  (pToStruct)->m_nPri2 |=  (uint32)  (pFromData)[10] << 8;
  (pToStruct)->m_nPri2 |=  (uint32)  (pFromData)[9] << 16;
  (pToStruct)->m_nPri2 |=  (uint32)  (pFromData)[8] << 24;

  /* Unpack Member: m_nPri1 */
  (pToStruct)->m_nPri1 =  (uint32)  (pFromData)[7] ;
  (pToStruct)->m_nPri1 |=  (uint32)  (pFromData)[6] << 8;
  (pToStruct)->m_nPri1 |=  (uint32)  (pFromData)[5] << 16;
  (pToStruct)->m_nPri1 |=  (uint32)  (pFromData)[4] << 24;

  /* Unpack Member: m_nPri0 */
  (pToStruct)->m_nPri0 =  (uint32)  (pFromData)[3] ;
  (pToStruct)->m_nPri0 |=  (uint32)  (pFromData)[2] << 8;
  (pToStruct)->m_nPri0 |=  (uint32)  (pFromData)[1] << 16;
  (pToStruct)->m_nPri0 |=  (uint32)  (pFromData)[0] << 24;
#else

  /* Unpack operation based on little endian */

  /* Unpack Member: m_nPri4 */
  (pToStruct)->m_nPri4 =  (uint32)  (pFromData)[16] ;
  (pToStruct)->m_nPri4 |=  (uint32)  (pFromData)[17] << 8;
  (pToStruct)->m_nPri4 |=  (uint32)  (pFromData)[18] << 16;
  (pToStruct)->m_nPri4 |=  (uint32)  (pFromData)[19] << 24;

  /* Unpack Member: m_nPri3 */
  (pToStruct)->m_nPri3 =  (uint32)  (pFromData)[12] ;
  (pToStruct)->m_nPri3 |=  (uint32)  (pFromData)[13] << 8;
  (pToStruct)->m_nPri3 |=  (uint32)  (pFromData)[14] << 16;
  (pToStruct)->m_nPri3 |=  (uint32)  (pFromData)[15] << 24;

  /* Unpack Member: m_nPri2 */
  (pToStruct)->m_nPri2 =  (uint32)  (pFromData)[8] ;
  (pToStruct)->m_nPri2 |=  (uint32)  (pFromData)[9] << 8;
  (pToStruct)->m_nPri2 |=  (uint32)  (pFromData)[10] << 16;
  (pToStruct)->m_nPri2 |=  (uint32)  (pFromData)[11] << 24;

  /* Unpack Member: m_nPri1 */
  (pToStruct)->m_nPri1 =  (uint32)  (pFromData)[4] ;
  (pToStruct)->m_nPri1 |=  (uint32)  (pFromData)[5] << 8;
  (pToStruct)->m_nPri1 |=  (uint32)  (pFromData)[6] << 16;
  (pToStruct)->m_nPri1 |=  (uint32)  (pFromData)[7] << 24;

  /* Unpack Member: m_nPri0 */
  (pToStruct)->m_nPri0 =  (uint32)  (pFromData)[0] ;
  (pToStruct)->m_nPri0 |=  (uint32)  (pFromData)[1] << 8;
  (pToStruct)->m_nPri0 |=  (uint32)  (pFromData)[2] << 16;
  (pToStruct)->m_nPri0 |=  (uint32)  (pFromData)[3] << 24;
#endif

}



/* initialize an instance of this zframe */
void
sbZfKaQsLnaPriEntry_InitInstance(sbZfKaQsLnaPriEntry_t *pFrame) {

  pFrame->m_nPri4 =  (unsigned int)  0;
  pFrame->m_nPri3 =  (unsigned int)  0;
  pFrame->m_nPri2 =  (unsigned int)  0;
  pFrame->m_nPri1 =  (unsigned int)  0;
  pFrame->m_nPri0 =  (unsigned int)  0;

}
