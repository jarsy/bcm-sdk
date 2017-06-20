/*
 * $Id: sbZfKaEpCrTableEntry.c,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


#include "sbTypes.h"
#include "sbZfKaEpCrTableEntry.hx"
#include "sbWrappers.h"
#include <sal/types.h>



/* Pack from struct into array of bytes */
uint32
sbZfKaEpCrTableEntry_Pack(sbZfKaEpCrTableEntry_t *pFrom,
                          uint8 *pToData,
                          uint32 nMaxToDataIndex) {
#ifdef SAND_BIG_ENDIAN_HOST
  int i;
  int size = SB_ZF_ZFKAEPCRTABLEENTRY_SIZE_IN_BYTES;

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on bigword endian */

  /* Pack Member: m_nClass15 */
  (pToData)[4] |= ((pFrom)->m_nClass15 & 0x0f) <<4;

  /* Pack Member: m_nClass14 */
  (pToData)[4] |= ((pFrom)->m_nClass14 & 0x0f);

  /* Pack Member: m_nClass13 */
  (pToData)[5] |= ((pFrom)->m_nClass13 & 0x0f) <<4;

  /* Pack Member: m_nClass12 */
  (pToData)[5] |= ((pFrom)->m_nClass12 & 0x0f);

  /* Pack Member: m_nClass11 */
  (pToData)[6] |= ((pFrom)->m_nClass11 & 0x0f) <<4;

  /* Pack Member: m_nClass10 */
  (pToData)[6] |= ((pFrom)->m_nClass10 & 0x0f);

  /* Pack Member: m_nClass9 */
  (pToData)[7] |= ((pFrom)->m_nClass9 & 0x0f) <<4;

  /* Pack Member: m_nClass8 */
  (pToData)[7] |= ((pFrom)->m_nClass8 & 0x0f);

  /* Pack Member: m_nClass7 */
  (pToData)[0] |= ((pFrom)->m_nClass7 & 0x0f) <<4;

  /* Pack Member: m_nClass6 */
  (pToData)[0] |= ((pFrom)->m_nClass6 & 0x0f);

  /* Pack Member: m_nClass5 */
  (pToData)[1] |= ((pFrom)->m_nClass5 & 0x0f) <<4;

  /* Pack Member: m_nClass4 */
  (pToData)[1] |= ((pFrom)->m_nClass4 & 0x0f);

  /* Pack Member: m_nClass3 */
  (pToData)[2] |= ((pFrom)->m_nClass3 & 0x0f) <<4;

  /* Pack Member: m_nClass2 */
  (pToData)[2] |= ((pFrom)->m_nClass2 & 0x0f);

  /* Pack Member: m_nClass1 */
  (pToData)[3] |= ((pFrom)->m_nClass1 & 0x0f) <<4;

  /* Pack Member: m_nClass0 */
  (pToData)[3] |= ((pFrom)->m_nClass0 & 0x0f);
#else
  int i;
  int size = SB_ZF_ZFKAEPCRTABLEENTRY_SIZE_IN_BYTES;

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on little endian */

  /* Pack Member: m_nClass15 */
  (pToData)[7] |= ((pFrom)->m_nClass15 & 0x0f) <<4;

  /* Pack Member: m_nClass14 */
  (pToData)[7] |= ((pFrom)->m_nClass14 & 0x0f);

  /* Pack Member: m_nClass13 */
  (pToData)[6] |= ((pFrom)->m_nClass13 & 0x0f) <<4;

  /* Pack Member: m_nClass12 */
  (pToData)[6] |= ((pFrom)->m_nClass12 & 0x0f);

  /* Pack Member: m_nClass11 */
  (pToData)[5] |= ((pFrom)->m_nClass11 & 0x0f) <<4;

  /* Pack Member: m_nClass10 */
  (pToData)[5] |= ((pFrom)->m_nClass10 & 0x0f);

  /* Pack Member: m_nClass9 */
  (pToData)[4] |= ((pFrom)->m_nClass9 & 0x0f) <<4;

  /* Pack Member: m_nClass8 */
  (pToData)[4] |= ((pFrom)->m_nClass8 & 0x0f);

  /* Pack Member: m_nClass7 */
  (pToData)[3] |= ((pFrom)->m_nClass7 & 0x0f) <<4;

  /* Pack Member: m_nClass6 */
  (pToData)[3] |= ((pFrom)->m_nClass6 & 0x0f);

  /* Pack Member: m_nClass5 */
  (pToData)[2] |= ((pFrom)->m_nClass5 & 0x0f) <<4;

  /* Pack Member: m_nClass4 */
  (pToData)[2] |= ((pFrom)->m_nClass4 & 0x0f);

  /* Pack Member: m_nClass3 */
  (pToData)[1] |= ((pFrom)->m_nClass3 & 0x0f) <<4;

  /* Pack Member: m_nClass2 */
  (pToData)[1] |= ((pFrom)->m_nClass2 & 0x0f);

  /* Pack Member: m_nClass1 */
  (pToData)[0] |= ((pFrom)->m_nClass1 & 0x0f) <<4;

  /* Pack Member: m_nClass0 */
  (pToData)[0] |= ((pFrom)->m_nClass0 & 0x0f);
#endif

  return SB_ZF_ZFKAEPCRTABLEENTRY_SIZE_IN_BYTES;
}




/* Unpack from array of bytes into struct */
void
sbZfKaEpCrTableEntry_Unpack(sbZfKaEpCrTableEntry_t *pToStruct,
                            uint8 *pFromData,
                            uint32 nMaxToDataIndex) {
  COMPILER_UINT64 tmp;

  (void) tmp;

#ifdef SAND_BIG_ENDIAN_HOST

  /* Unpack operation based on bigword endian */

  /* Unpack Member: m_nClass15 */
  (pToStruct)->m_nClass15 =  (uint32)  ((pFromData)[4] >> 4) & 0x0f;

  /* Unpack Member: m_nClass14 */
  (pToStruct)->m_nClass14 =  (uint32)  ((pFromData)[4] ) & 0x0f;

  /* Unpack Member: m_nClass13 */
  (pToStruct)->m_nClass13 =  (uint32)  ((pFromData)[5] >> 4) & 0x0f;

  /* Unpack Member: m_nClass12 */
  (pToStruct)->m_nClass12 =  (uint32)  ((pFromData)[5] ) & 0x0f;

  /* Unpack Member: m_nClass11 */
  (pToStruct)->m_nClass11 =  (uint32)  ((pFromData)[6] >> 4) & 0x0f;

  /* Unpack Member: m_nClass10 */
  (pToStruct)->m_nClass10 =  (uint32)  ((pFromData)[6] ) & 0x0f;

  /* Unpack Member: m_nClass9 */
  (pToStruct)->m_nClass9 =  (uint32)  ((pFromData)[7] >> 4) & 0x0f;

  /* Unpack Member: m_nClass8 */
  (pToStruct)->m_nClass8 =  (uint32)  ((pFromData)[7] ) & 0x0f;

  /* Unpack Member: m_nClass7 */
  (pToStruct)->m_nClass7 =  (uint32)  ((pFromData)[0] >> 4) & 0x0f;

  /* Unpack Member: m_nClass6 */
  (pToStruct)->m_nClass6 =  (uint32)  ((pFromData)[0] ) & 0x0f;

  /* Unpack Member: m_nClass5 */
  (pToStruct)->m_nClass5 =  (uint32)  ((pFromData)[1] >> 4) & 0x0f;

  /* Unpack Member: m_nClass4 */
  (pToStruct)->m_nClass4 =  (uint32)  ((pFromData)[1] ) & 0x0f;

  /* Unpack Member: m_nClass3 */
  (pToStruct)->m_nClass3 =  (uint32)  ((pFromData)[2] >> 4) & 0x0f;

  /* Unpack Member: m_nClass2 */
  (pToStruct)->m_nClass2 =  (uint32)  ((pFromData)[2] ) & 0x0f;

  /* Unpack Member: m_nClass1 */
  (pToStruct)->m_nClass1 =  (uint32)  ((pFromData)[3] >> 4) & 0x0f;

  /* Unpack Member: m_nClass0 */
  (pToStruct)->m_nClass0 =  (uint32)  ((pFromData)[3] ) & 0x0f;
#else

  /* Unpack operation based on little endian */

  /* Unpack Member: m_nClass15 */
  (pToStruct)->m_nClass15 =  (uint32)  ((pFromData)[7] >> 4) & 0x0f;

  /* Unpack Member: m_nClass14 */
  (pToStruct)->m_nClass14 =  (uint32)  ((pFromData)[7] ) & 0x0f;

  /* Unpack Member: m_nClass13 */
  (pToStruct)->m_nClass13 =  (uint32)  ((pFromData)[6] >> 4) & 0x0f;

  /* Unpack Member: m_nClass12 */
  (pToStruct)->m_nClass12 =  (uint32)  ((pFromData)[6] ) & 0x0f;

  /* Unpack Member: m_nClass11 */
  (pToStruct)->m_nClass11 =  (uint32)  ((pFromData)[5] >> 4) & 0x0f;

  /* Unpack Member: m_nClass10 */
  (pToStruct)->m_nClass10 =  (uint32)  ((pFromData)[5] ) & 0x0f;

  /* Unpack Member: m_nClass9 */
  (pToStruct)->m_nClass9 =  (uint32)  ((pFromData)[4] >> 4) & 0x0f;

  /* Unpack Member: m_nClass8 */
  (pToStruct)->m_nClass8 =  (uint32)  ((pFromData)[4] ) & 0x0f;

  /* Unpack Member: m_nClass7 */
  (pToStruct)->m_nClass7 =  (uint32)  ((pFromData)[3] >> 4) & 0x0f;

  /* Unpack Member: m_nClass6 */
  (pToStruct)->m_nClass6 =  (uint32)  ((pFromData)[3] ) & 0x0f;

  /* Unpack Member: m_nClass5 */
  (pToStruct)->m_nClass5 =  (uint32)  ((pFromData)[2] >> 4) & 0x0f;

  /* Unpack Member: m_nClass4 */
  (pToStruct)->m_nClass4 =  (uint32)  ((pFromData)[2] ) & 0x0f;

  /* Unpack Member: m_nClass3 */
  (pToStruct)->m_nClass3 =  (uint32)  ((pFromData)[1] >> 4) & 0x0f;

  /* Unpack Member: m_nClass2 */
  (pToStruct)->m_nClass2 =  (uint32)  ((pFromData)[1] ) & 0x0f;

  /* Unpack Member: m_nClass1 */
  (pToStruct)->m_nClass1 =  (uint32)  ((pFromData)[0] >> 4) & 0x0f;

  /* Unpack Member: m_nClass0 */
  (pToStruct)->m_nClass0 =  (uint32)  ((pFromData)[0] ) & 0x0f;
#endif

}



/* initialize an instance of this zframe */
void
sbZfKaEpCrTableEntry_InitInstance(sbZfKaEpCrTableEntry_t *pFrame) {

  pFrame->m_nClass15 =  (unsigned int)  0;
  pFrame->m_nClass14 =  (unsigned int)  0;
  pFrame->m_nClass13 =  (unsigned int)  0;
  pFrame->m_nClass12 =  (unsigned int)  0;
  pFrame->m_nClass11 =  (unsigned int)  0;
  pFrame->m_nClass10 =  (unsigned int)  0;
  pFrame->m_nClass9 =  (unsigned int)  0;
  pFrame->m_nClass8 =  (unsigned int)  0;
  pFrame->m_nClass7 =  (unsigned int)  0;
  pFrame->m_nClass6 =  (unsigned int)  0;
  pFrame->m_nClass5 =  (unsigned int)  0;
  pFrame->m_nClass4 =  (unsigned int)  0;
  pFrame->m_nClass3 =  (unsigned int)  0;
  pFrame->m_nClass2 =  (unsigned int)  0;
  pFrame->m_nClass1 =  (unsigned int)  0;
  pFrame->m_nClass0 =  (unsigned int)  0;

}
