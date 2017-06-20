/*
 * $Id: sbZfKaQsLnaPortRemapEntry.c,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


#include "sbTypes.h"
#include "sbZfKaQsLnaPortRemapEntry.hx"
#include "sbWrappers.h"
#include <sal/types.h>



/* Pack from struct into array of bytes */
uint32
sbZfKaQsLnaPortRemapEntry_Pack(sbZfKaQsLnaPortRemapEntry_t *pFrom,
                               uint8 *pToData,
                               uint32 nMaxToDataIndex) {
#ifdef SAND_BIG_ENDIAN_HOST
  int i;
  int size = SB_ZF_ZFKAQSPORTREMAPENTRY_SIZE_IN_BYTES;

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on bigword endian */

  /* Pack Member: m_nRes */
  (pToData)[17] |= ((pFrom)->m_nRes & 0x03) <<6;
  (pToData)[16] |= ((pFrom)->m_nRes >> 2) &0xFF;

  /* Pack Member: m_nPort24 */
  (pToData)[17] |= ((pFrom)->m_nPort24 & 0x3f);

  /* Pack Member: m_nPort23 */
  (pToData)[18] |= ((pFrom)->m_nPort23 & 0x3f) <<2;

  /* Pack Member: m_nPort22 */
  (pToData)[19] |= ((pFrom)->m_nPort22 & 0x0f) <<4;
  (pToData)[18] |= ((pFrom)->m_nPort22 >> 4) & 0x03;

  /* Pack Member: m_nPort21 */
  (pToData)[12] |= ((pFrom)->m_nPort21 & 0x03) <<6;
  (pToData)[19] |= ((pFrom)->m_nPort21 >> 2) & 0x0f;

  /* Pack Member: m_nPort20 */
  (pToData)[12] |= ((pFrom)->m_nPort20 & 0x3f);

  /* Pack Member: m_nPort19 */
  (pToData)[13] |= ((pFrom)->m_nPort19 & 0x3f) <<2;

  /* Pack Member: m_nPort18 */
  (pToData)[14] |= ((pFrom)->m_nPort18 & 0x0f) <<4;
  (pToData)[13] |= ((pFrom)->m_nPort18 >> 4) & 0x03;

  /* Pack Member: m_nPort17 */
  (pToData)[15] |= ((pFrom)->m_nPort17 & 0x03) <<6;
  (pToData)[14] |= ((pFrom)->m_nPort17 >> 2) & 0x0f;

  /* Pack Member: m_nPort16 */
  (pToData)[15] |= ((pFrom)->m_nPort16 & 0x3f);

  /* Pack Member: m_nPort15 */
  (pToData)[8] |= ((pFrom)->m_nPort15 & 0x3f) <<2;

  /* Pack Member: m_nPort14 */
  (pToData)[9] |= ((pFrom)->m_nPort14 & 0x0f) <<4;
  (pToData)[8] |= ((pFrom)->m_nPort14 >> 4) & 0x03;

  /* Pack Member: m_nPort13 */
  (pToData)[10] |= ((pFrom)->m_nPort13 & 0x03) <<6;
  (pToData)[9] |= ((pFrom)->m_nPort13 >> 2) & 0x0f;

  /* Pack Member: m_nPort12 */
  (pToData)[10] |= ((pFrom)->m_nPort12 & 0x3f);

  /* Pack Member: m_nPort11 */
  (pToData)[11] |= ((pFrom)->m_nPort11 & 0x3f) <<2;

  /* Pack Member: m_nPort10 */
  (pToData)[4] |= ((pFrom)->m_nPort10 & 0x0f) <<4;
  (pToData)[11] |= ((pFrom)->m_nPort10 >> 4) & 0x03;

  /* Pack Member: m_nPort9 */
  (pToData)[5] |= ((pFrom)->m_nPort9 & 0x03) <<6;
  (pToData)[4] |= ((pFrom)->m_nPort9 >> 2) & 0x0f;

  /* Pack Member: m_nPort8 */
  (pToData)[5] |= ((pFrom)->m_nPort8 & 0x3f);

  /* Pack Member: m_nPort7 */
  (pToData)[6] |= ((pFrom)->m_nPort7 & 0x3f) <<2;

  /* Pack Member: m_nPort6 */
  (pToData)[7] |= ((pFrom)->m_nPort6 & 0x0f) <<4;
  (pToData)[6] |= ((pFrom)->m_nPort6 >> 4) & 0x03;

  /* Pack Member: m_nPort5 */
  (pToData)[0] |= ((pFrom)->m_nPort5 & 0x03) <<6;
  (pToData)[7] |= ((pFrom)->m_nPort5 >> 2) & 0x0f;

  /* Pack Member: m_nPort4 */
  (pToData)[0] |= ((pFrom)->m_nPort4 & 0x3f);

  /* Pack Member: m_nPort3 */
  (pToData)[1] |= ((pFrom)->m_nPort3 & 0x3f) <<2;

  /* Pack Member: m_nPort2 */
  (pToData)[2] |= ((pFrom)->m_nPort2 & 0x0f) <<4;
  (pToData)[1] |= ((pFrom)->m_nPort2 >> 4) & 0x03;

  /* Pack Member: m_nPort1 */
  (pToData)[3] |= ((pFrom)->m_nPort1 & 0x03) <<6;
  (pToData)[2] |= ((pFrom)->m_nPort1 >> 2) & 0x0f;

  /* Pack Member: m_nPort0 */
  (pToData)[3] |= ((pFrom)->m_nPort0 & 0x3f);
#else
  int i;
  int size = SB_ZF_ZFKAQSPORTREMAPENTRY_SIZE_IN_BYTES;

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on little endian */

  /* Pack Member: m_nRes */
  (pToData)[18] |= ((pFrom)->m_nRes & 0x03) <<6;
  (pToData)[19] |= ((pFrom)->m_nRes >> 2) &0xFF;

  /* Pack Member: m_nPort24 */
  (pToData)[18] |= ((pFrom)->m_nPort24 & 0x3f);

  /* Pack Member: m_nPort23 */
  (pToData)[17] |= ((pFrom)->m_nPort23 & 0x3f) <<2;

  /* Pack Member: m_nPort22 */
  (pToData)[16] |= ((pFrom)->m_nPort22 & 0x0f) <<4;
  (pToData)[17] |= ((pFrom)->m_nPort22 >> 4) & 0x03;

  /* Pack Member: m_nPort21 */
  (pToData)[15] |= ((pFrom)->m_nPort21 & 0x03) <<6;
  (pToData)[16] |= ((pFrom)->m_nPort21 >> 2) & 0x0f;

  /* Pack Member: m_nPort20 */
  (pToData)[15] |= ((pFrom)->m_nPort20 & 0x3f);

  /* Pack Member: m_nPort19 */
  (pToData)[14] |= ((pFrom)->m_nPort19 & 0x3f) <<2;

  /* Pack Member: m_nPort18 */
  (pToData)[13] |= ((pFrom)->m_nPort18 & 0x0f) <<4;
  (pToData)[14] |= ((pFrom)->m_nPort18 >> 4) & 0x03;

  /* Pack Member: m_nPort17 */
  (pToData)[12] |= ((pFrom)->m_nPort17 & 0x03) <<6;
  (pToData)[13] |= ((pFrom)->m_nPort17 >> 2) & 0x0f;

  /* Pack Member: m_nPort16 */
  (pToData)[12] |= ((pFrom)->m_nPort16 & 0x3f);

  /* Pack Member: m_nPort15 */
  (pToData)[11] |= ((pFrom)->m_nPort15 & 0x3f) <<2;

  /* Pack Member: m_nPort14 */
  (pToData)[10] |= ((pFrom)->m_nPort14 & 0x0f) <<4;
  (pToData)[11] |= ((pFrom)->m_nPort14 >> 4) & 0x03;

  /* Pack Member: m_nPort13 */
  (pToData)[9] |= ((pFrom)->m_nPort13 & 0x03) <<6;
  (pToData)[10] |= ((pFrom)->m_nPort13 >> 2) & 0x0f;

  /* Pack Member: m_nPort12 */
  (pToData)[9] |= ((pFrom)->m_nPort12 & 0x3f);

  /* Pack Member: m_nPort11 */
  (pToData)[8] |= ((pFrom)->m_nPort11 & 0x3f) <<2;

  /* Pack Member: m_nPort10 */
  (pToData)[7] |= ((pFrom)->m_nPort10 & 0x0f) <<4;
  (pToData)[8] |= ((pFrom)->m_nPort10 >> 4) & 0x03;

  /* Pack Member: m_nPort9 */
  (pToData)[6] |= ((pFrom)->m_nPort9 & 0x03) <<6;
  (pToData)[7] |= ((pFrom)->m_nPort9 >> 2) & 0x0f;

  /* Pack Member: m_nPort8 */
  (pToData)[6] |= ((pFrom)->m_nPort8 & 0x3f);

  /* Pack Member: m_nPort7 */
  (pToData)[5] |= ((pFrom)->m_nPort7 & 0x3f) <<2;

  /* Pack Member: m_nPort6 */
  (pToData)[4] |= ((pFrom)->m_nPort6 & 0x0f) <<4;
  (pToData)[5] |= ((pFrom)->m_nPort6 >> 4) & 0x03;

  /* Pack Member: m_nPort5 */
  (pToData)[3] |= ((pFrom)->m_nPort5 & 0x03) <<6;
  (pToData)[4] |= ((pFrom)->m_nPort5 >> 2) & 0x0f;

  /* Pack Member: m_nPort4 */
  (pToData)[3] |= ((pFrom)->m_nPort4 & 0x3f);

  /* Pack Member: m_nPort3 */
  (pToData)[2] |= ((pFrom)->m_nPort3 & 0x3f) <<2;

  /* Pack Member: m_nPort2 */
  (pToData)[1] |= ((pFrom)->m_nPort2 & 0x0f) <<4;
  (pToData)[2] |= ((pFrom)->m_nPort2 >> 4) & 0x03;

  /* Pack Member: m_nPort1 */
  (pToData)[0] |= ((pFrom)->m_nPort1 & 0x03) <<6;
  (pToData)[1] |= ((pFrom)->m_nPort1 >> 2) & 0x0f;

  /* Pack Member: m_nPort0 */
  (pToData)[0] |= ((pFrom)->m_nPort0 & 0x3f);
#endif

  return SB_ZF_ZFKAQSPORTREMAPENTRY_SIZE_IN_BYTES;
}




/* Unpack from array of bytes into struct */
void
sbZfKaQsLnaPortRemapEntry_Unpack(sbZfKaQsLnaPortRemapEntry_t *pToStruct,
                                 uint8 *pFromData,
                                 uint32 nMaxToDataIndex) {
  COMPILER_UINT64 tmp;

  (void) tmp;

#ifdef SAND_BIG_ENDIAN_HOST

  /* Unpack operation based on bigword endian */

  /* Unpack Member: m_nRes */
  (pToStruct)->m_nRes =  (uint32)  ((pFromData)[17] >> 6) & 0x03;
  (pToStruct)->m_nRes |=  (uint32)  (pFromData)[16] << 2;

  /* Unpack Member: m_nPort24 */
  (pToStruct)->m_nPort24 =  (uint32)  ((pFromData)[17] ) & 0x3f;

  /* Unpack Member: m_nPort23 */
  (pToStruct)->m_nPort23 =  (uint32)  ((pFromData)[18] >> 2) & 0x3f;

  /* Unpack Member: m_nPort22 */
  (pToStruct)->m_nPort22 =  (uint32)  ((pFromData)[19] >> 4) & 0x0f;
  (pToStruct)->m_nPort22 |=  (uint32)  ((pFromData)[18] & 0x03) << 4;

  /* Unpack Member: m_nPort21 */
  (pToStruct)->m_nPort21 =  (uint32)  ((pFromData)[12] >> 6) & 0x03;
  (pToStruct)->m_nPort21 |=  (uint32)  ((pFromData)[19] & 0x0f) << 2;

  /* Unpack Member: m_nPort20 */
  (pToStruct)->m_nPort20 =  (uint32)  ((pFromData)[12] ) & 0x3f;

  /* Unpack Member: m_nPort19 */
  (pToStruct)->m_nPort19 =  (uint32)  ((pFromData)[13] >> 2) & 0x3f;

  /* Unpack Member: m_nPort18 */
  (pToStruct)->m_nPort18 =  (uint32)  ((pFromData)[14] >> 4) & 0x0f;
  (pToStruct)->m_nPort18 |=  (uint32)  ((pFromData)[13] & 0x03) << 4;

  /* Unpack Member: m_nPort17 */
  (pToStruct)->m_nPort17 =  (uint32)  ((pFromData)[15] >> 6) & 0x03;
  (pToStruct)->m_nPort17 |=  (uint32)  ((pFromData)[14] & 0x0f) << 2;

  /* Unpack Member: m_nPort16 */
  (pToStruct)->m_nPort16 =  (uint32)  ((pFromData)[15] ) & 0x3f;

  /* Unpack Member: m_nPort15 */
  (pToStruct)->m_nPort15 =  (uint32)  ((pFromData)[8] >> 2) & 0x3f;

  /* Unpack Member: m_nPort14 */
  (pToStruct)->m_nPort14 =  (uint32)  ((pFromData)[9] >> 4) & 0x0f;
  (pToStruct)->m_nPort14 |=  (uint32)  ((pFromData)[8] & 0x03) << 4;

  /* Unpack Member: m_nPort13 */
  (pToStruct)->m_nPort13 =  (uint32)  ((pFromData)[10] >> 6) & 0x03;
  (pToStruct)->m_nPort13 |=  (uint32)  ((pFromData)[9] & 0x0f) << 2;

  /* Unpack Member: m_nPort12 */
  (pToStruct)->m_nPort12 =  (uint32)  ((pFromData)[10] ) & 0x3f;

  /* Unpack Member: m_nPort11 */
  (pToStruct)->m_nPort11 =  (uint32)  ((pFromData)[11] >> 2) & 0x3f;

  /* Unpack Member: m_nPort10 */
  (pToStruct)->m_nPort10 =  (uint32)  ((pFromData)[4] >> 4) & 0x0f;
  (pToStruct)->m_nPort10 |=  (uint32)  ((pFromData)[11] & 0x03) << 4;

  /* Unpack Member: m_nPort9 */
  (pToStruct)->m_nPort9 =  (uint32)  ((pFromData)[5] >> 6) & 0x03;
  (pToStruct)->m_nPort9 |=  (uint32)  ((pFromData)[4] & 0x0f) << 2;

  /* Unpack Member: m_nPort8 */
  (pToStruct)->m_nPort8 =  (uint32)  ((pFromData)[5] ) & 0x3f;

  /* Unpack Member: m_nPort7 */
  (pToStruct)->m_nPort7 =  (uint32)  ((pFromData)[6] >> 2) & 0x3f;

  /* Unpack Member: m_nPort6 */
  (pToStruct)->m_nPort6 =  (uint32)  ((pFromData)[7] >> 4) & 0x0f;
  (pToStruct)->m_nPort6 |=  (uint32)  ((pFromData)[6] & 0x03) << 4;

  /* Unpack Member: m_nPort5 */
  (pToStruct)->m_nPort5 =  (uint32)  ((pFromData)[0] >> 6) & 0x03;
  (pToStruct)->m_nPort5 |=  (uint32)  ((pFromData)[7] & 0x0f) << 2;

  /* Unpack Member: m_nPort4 */
  (pToStruct)->m_nPort4 =  (uint32)  ((pFromData)[0] ) & 0x3f;

  /* Unpack Member: m_nPort3 */
  (pToStruct)->m_nPort3 =  (uint32)  ((pFromData)[1] >> 2) & 0x3f;

  /* Unpack Member: m_nPort2 */
  (pToStruct)->m_nPort2 =  (uint32)  ((pFromData)[2] >> 4) & 0x0f;
  (pToStruct)->m_nPort2 |=  (uint32)  ((pFromData)[1] & 0x03) << 4;

  /* Unpack Member: m_nPort1 */
  (pToStruct)->m_nPort1 =  (uint32)  ((pFromData)[3] >> 6) & 0x03;
  (pToStruct)->m_nPort1 |=  (uint32)  ((pFromData)[2] & 0x0f) << 2;

  /* Unpack Member: m_nPort0 */
  (pToStruct)->m_nPort0 =  (uint32)  ((pFromData)[3] ) & 0x3f;
#else

  /* Unpack operation based on little endian */

  /* Unpack Member: m_nRes */
  (pToStruct)->m_nRes =  (uint32)  ((pFromData)[18] >> 6) & 0x03;
  (pToStruct)->m_nRes |=  (uint32)  (pFromData)[19] << 2;

  /* Unpack Member: m_nPort24 */
  (pToStruct)->m_nPort24 =  (uint32)  ((pFromData)[18] ) & 0x3f;

  /* Unpack Member: m_nPort23 */
  (pToStruct)->m_nPort23 =  (uint32)  ((pFromData)[17] >> 2) & 0x3f;

  /* Unpack Member: m_nPort22 */
  (pToStruct)->m_nPort22 =  (uint32)  ((pFromData)[16] >> 4) & 0x0f;
  (pToStruct)->m_nPort22 |=  (uint32)  ((pFromData)[17] & 0x03) << 4;

  /* Unpack Member: m_nPort21 */
  (pToStruct)->m_nPort21 =  (uint32)  ((pFromData)[15] >> 6) & 0x03;
  (pToStruct)->m_nPort21 |=  (uint32)  ((pFromData)[16] & 0x0f) << 2;

  /* Unpack Member: m_nPort20 */
  (pToStruct)->m_nPort20 =  (uint32)  ((pFromData)[15] ) & 0x3f;

  /* Unpack Member: m_nPort19 */
  (pToStruct)->m_nPort19 =  (uint32)  ((pFromData)[14] >> 2) & 0x3f;

  /* Unpack Member: m_nPort18 */
  (pToStruct)->m_nPort18 =  (uint32)  ((pFromData)[13] >> 4) & 0x0f;
  (pToStruct)->m_nPort18 |=  (uint32)  ((pFromData)[14] & 0x03) << 4;

  /* Unpack Member: m_nPort17 */
  (pToStruct)->m_nPort17 =  (uint32)  ((pFromData)[12] >> 6) & 0x03;
  (pToStruct)->m_nPort17 |=  (uint32)  ((pFromData)[13] & 0x0f) << 2;

  /* Unpack Member: m_nPort16 */
  (pToStruct)->m_nPort16 =  (uint32)  ((pFromData)[12] ) & 0x3f;

  /* Unpack Member: m_nPort15 */
  (pToStruct)->m_nPort15 =  (uint32)  ((pFromData)[11] >> 2) & 0x3f;

  /* Unpack Member: m_nPort14 */
  (pToStruct)->m_nPort14 =  (uint32)  ((pFromData)[10] >> 4) & 0x0f;
  (pToStruct)->m_nPort14 |=  (uint32)  ((pFromData)[11] & 0x03) << 4;

  /* Unpack Member: m_nPort13 */
  (pToStruct)->m_nPort13 =  (uint32)  ((pFromData)[9] >> 6) & 0x03;
  (pToStruct)->m_nPort13 |=  (uint32)  ((pFromData)[10] & 0x0f) << 2;

  /* Unpack Member: m_nPort12 */
  (pToStruct)->m_nPort12 =  (uint32)  ((pFromData)[9] ) & 0x3f;

  /* Unpack Member: m_nPort11 */
  (pToStruct)->m_nPort11 =  (uint32)  ((pFromData)[8] >> 2) & 0x3f;

  /* Unpack Member: m_nPort10 */
  (pToStruct)->m_nPort10 =  (uint32)  ((pFromData)[7] >> 4) & 0x0f;
  (pToStruct)->m_nPort10 |=  (uint32)  ((pFromData)[8] & 0x03) << 4;

  /* Unpack Member: m_nPort9 */
  (pToStruct)->m_nPort9 =  (uint32)  ((pFromData)[6] >> 6) & 0x03;
  (pToStruct)->m_nPort9 |=  (uint32)  ((pFromData)[7] & 0x0f) << 2;

  /* Unpack Member: m_nPort8 */
  (pToStruct)->m_nPort8 =  (uint32)  ((pFromData)[6] ) & 0x3f;

  /* Unpack Member: m_nPort7 */
  (pToStruct)->m_nPort7 =  (uint32)  ((pFromData)[5] >> 2) & 0x3f;

  /* Unpack Member: m_nPort6 */
  (pToStruct)->m_nPort6 =  (uint32)  ((pFromData)[4] >> 4) & 0x0f;
  (pToStruct)->m_nPort6 |=  (uint32)  ((pFromData)[5] & 0x03) << 4;

  /* Unpack Member: m_nPort5 */
  (pToStruct)->m_nPort5 =  (uint32)  ((pFromData)[3] >> 6) & 0x03;
  (pToStruct)->m_nPort5 |=  (uint32)  ((pFromData)[4] & 0x0f) << 2;

  /* Unpack Member: m_nPort4 */
  (pToStruct)->m_nPort4 =  (uint32)  ((pFromData)[3] ) & 0x3f;

  /* Unpack Member: m_nPort3 */
  (pToStruct)->m_nPort3 =  (uint32)  ((pFromData)[2] >> 2) & 0x3f;

  /* Unpack Member: m_nPort2 */
  (pToStruct)->m_nPort2 =  (uint32)  ((pFromData)[1] >> 4) & 0x0f;
  (pToStruct)->m_nPort2 |=  (uint32)  ((pFromData)[2] & 0x03) << 4;

  /* Unpack Member: m_nPort1 */
  (pToStruct)->m_nPort1 =  (uint32)  ((pFromData)[0] >> 6) & 0x03;
  (pToStruct)->m_nPort1 |=  (uint32)  ((pFromData)[1] & 0x0f) << 2;

  /* Unpack Member: m_nPort0 */
  (pToStruct)->m_nPort0 =  (uint32)  ((pFromData)[0] ) & 0x3f;
#endif

}



/* initialize an instance of this zframe */
void
sbZfKaQsLnaPortRemapEntry_InitInstance(sbZfKaQsLnaPortRemapEntry_t *pFrame) {

  pFrame->m_nRes =  (unsigned int)  0;
  pFrame->m_nPort24 =  (unsigned int)  0;
  pFrame->m_nPort23 =  (unsigned int)  0;
  pFrame->m_nPort22 =  (unsigned int)  0;
  pFrame->m_nPort21 =  (unsigned int)  0;
  pFrame->m_nPort20 =  (unsigned int)  0;
  pFrame->m_nPort19 =  (unsigned int)  0;
  pFrame->m_nPort18 =  (unsigned int)  0;
  pFrame->m_nPort17 =  (unsigned int)  0;
  pFrame->m_nPort16 =  (unsigned int)  0;
  pFrame->m_nPort15 =  (unsigned int)  0;
  pFrame->m_nPort14 =  (unsigned int)  0;
  pFrame->m_nPort13 =  (unsigned int)  0;
  pFrame->m_nPort12 =  (unsigned int)  0;
  pFrame->m_nPort11 =  (unsigned int)  0;
  pFrame->m_nPort10 =  (unsigned int)  0;
  pFrame->m_nPort9 =  (unsigned int)  0;
  pFrame->m_nPort8 =  (unsigned int)  0;
  pFrame->m_nPort7 =  (unsigned int)  0;
  pFrame->m_nPort6 =  (unsigned int)  0;
  pFrame->m_nPort5 =  (unsigned int)  0;
  pFrame->m_nPort4 =  (unsigned int)  0;
  pFrame->m_nPort3 =  (unsigned int)  0;
  pFrame->m_nPort2 =  (unsigned int)  0;
  pFrame->m_nPort1 =  (unsigned int)  0;
  pFrame->m_nPort0 =  (unsigned int)  0;

}
