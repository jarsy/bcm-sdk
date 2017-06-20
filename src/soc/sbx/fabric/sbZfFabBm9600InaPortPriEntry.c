/*
 * $Id: sbZfFabBm9600InaPortPriEntry.c,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


#include "sbTypes.h"
#include "sbZfFabBm9600InaPortPriEntry.hx"
#include "sbWrappers.h"
#include <sal/types.h>



/* Pack from struct into array of bytes */
uint32
sbZfFabBm9600InaPortPriEntry_Pack(sbZfFabBm9600InaPortPriEntry_t *pFrom,
                                  uint8 *pToData,
                                  uint32 nMaxToDataIndex) {
#ifdef SAND_BIG_ENDIAN_HOST
  int i;
  int size = SB_ZF_FAB_BM9600_INAPORTPRIENTRY_SIZE_IN_BYTES;

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on bigword endian */

  /* Pack Member: m_uPri_15 */
  (pToData)[12] |= ((pFrom)->m_uPri_15 & 0x0f) <<4;

  /* Pack Member: m_uNextpri_15 */
  (pToData)[12] |= ((pFrom)->m_uNextpri_15 & 0x0f);

  /* Pack Member: m_uPri_14 */
  (pToData)[13] |= ((pFrom)->m_uPri_14 & 0x0f) <<4;

  /* Pack Member: m_uNextpri_14 */
  (pToData)[13] |= ((pFrom)->m_uNextpri_14 & 0x0f);

  /* Pack Member: m_uPri_13 */
  (pToData)[14] |= ((pFrom)->m_uPri_13 & 0x0f) <<4;

  /* Pack Member: m_uNextpri_13 */
  (pToData)[14] |= ((pFrom)->m_uNextpri_13 & 0x0f);

  /* Pack Member: m_uPri_12 */
  (pToData)[15] |= ((pFrom)->m_uPri_12 & 0x0f) <<4;

  /* Pack Member: m_uNextpri_12 */
  (pToData)[15] |= ((pFrom)->m_uNextpri_12 & 0x0f);

  /* Pack Member: m_uPri_11 */
  (pToData)[8] |= ((pFrom)->m_uPri_11 & 0x0f) <<4;

  /* Pack Member: m_uNextpri_11 */
  (pToData)[8] |= ((pFrom)->m_uNextpri_11 & 0x0f);

  /* Pack Member: m_uPri_10 */
  (pToData)[9] |= ((pFrom)->m_uPri_10 & 0x0f) <<4;

  /* Pack Member: m_uNextpri_10 */
  (pToData)[9] |= ((pFrom)->m_uNextpri_10 & 0x0f);

  /* Pack Member: m_uPri_9 */
  (pToData)[10] |= ((pFrom)->m_uPri_9 & 0x0f) <<4;

  /* Pack Member: m_uNextpri_9 */
  (pToData)[10] |= ((pFrom)->m_uNextpri_9 & 0x0f);

  /* Pack Member: m_uPri_8 */
  (pToData)[11] |= ((pFrom)->m_uPri_8 & 0x0f) <<4;

  /* Pack Member: m_uNextpri_8 */
  (pToData)[11] |= ((pFrom)->m_uNextpri_8 & 0x0f);

  /* Pack Member: m_uPri_7 */
  (pToData)[4] |= ((pFrom)->m_uPri_7 & 0x0f) <<4;

  /* Pack Member: m_uNextpri_7 */
  (pToData)[4] |= ((pFrom)->m_uNextpri_7 & 0x0f);

  /* Pack Member: m_uPri_6 */
  (pToData)[5] |= ((pFrom)->m_uPri_6 & 0x0f) <<4;

  /* Pack Member: m_uNextpri_6 */
  (pToData)[5] |= ((pFrom)->m_uNextpri_6 & 0x0f);

  /* Pack Member: m_uPri_5 */
  (pToData)[6] |= ((pFrom)->m_uPri_5 & 0x0f) <<4;

  /* Pack Member: m_uNextpri_5 */
  (pToData)[6] |= ((pFrom)->m_uNextpri_5 & 0x0f);

  /* Pack Member: m_uPri_4 */
  (pToData)[7] |= ((pFrom)->m_uPri_4 & 0x0f) <<4;

  /* Pack Member: m_uNextpri_4 */
  (pToData)[7] |= ((pFrom)->m_uNextpri_4 & 0x0f);

  /* Pack Member: m_uPri_3 */
  (pToData)[0] |= ((pFrom)->m_uPri_3 & 0x0f) <<4;

  /* Pack Member: m_uNextpri_3 */
  (pToData)[0] |= ((pFrom)->m_uNextpri_3 & 0x0f);

  /* Pack Member: m_uPri_2 */
  (pToData)[1] |= ((pFrom)->m_uPri_2 & 0x0f) <<4;

  /* Pack Member: m_uNextpri_2 */
  (pToData)[1] |= ((pFrom)->m_uNextpri_2 & 0x0f);

  /* Pack Member: m_uPri_1 */
  (pToData)[2] |= ((pFrom)->m_uPri_1 & 0x0f) <<4;

  /* Pack Member: m_uNextpri_1 */
  (pToData)[2] |= ((pFrom)->m_uNextpri_1 & 0x0f);

  /* Pack Member: m_uPri_0 */
  (pToData)[3] |= ((pFrom)->m_uPri_0 & 0x0f) <<4;

  /* Pack Member: m_uNextpri_0 */
  (pToData)[3] |= ((pFrom)->m_uNextpri_0 & 0x0f);
#else
  int i;
  int size = SB_ZF_FAB_BM9600_INAPORTPRIENTRY_SIZE_IN_BYTES;

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on little endian */

  /* Pack Member: m_uPri_15 */
  (pToData)[15] |= ((pFrom)->m_uPri_15 & 0x0f) <<4;

  /* Pack Member: m_uNextpri_15 */
  (pToData)[15] |= ((pFrom)->m_uNextpri_15 & 0x0f);

  /* Pack Member: m_uPri_14 */
  (pToData)[14] |= ((pFrom)->m_uPri_14 & 0x0f) <<4;

  /* Pack Member: m_uNextpri_14 */
  (pToData)[14] |= ((pFrom)->m_uNextpri_14 & 0x0f);

  /* Pack Member: m_uPri_13 */
  (pToData)[13] |= ((pFrom)->m_uPri_13 & 0x0f) <<4;

  /* Pack Member: m_uNextpri_13 */
  (pToData)[13] |= ((pFrom)->m_uNextpri_13 & 0x0f);

  /* Pack Member: m_uPri_12 */
  (pToData)[12] |= ((pFrom)->m_uPri_12 & 0x0f) <<4;

  /* Pack Member: m_uNextpri_12 */
  (pToData)[12] |= ((pFrom)->m_uNextpri_12 & 0x0f);

  /* Pack Member: m_uPri_11 */
  (pToData)[11] |= ((pFrom)->m_uPri_11 & 0x0f) <<4;

  /* Pack Member: m_uNextpri_11 */
  (pToData)[11] |= ((pFrom)->m_uNextpri_11 & 0x0f);

  /* Pack Member: m_uPri_10 */
  (pToData)[10] |= ((pFrom)->m_uPri_10 & 0x0f) <<4;

  /* Pack Member: m_uNextpri_10 */
  (pToData)[10] |= ((pFrom)->m_uNextpri_10 & 0x0f);

  /* Pack Member: m_uPri_9 */
  (pToData)[9] |= ((pFrom)->m_uPri_9 & 0x0f) <<4;

  /* Pack Member: m_uNextpri_9 */
  (pToData)[9] |= ((pFrom)->m_uNextpri_9 & 0x0f);

  /* Pack Member: m_uPri_8 */
  (pToData)[8] |= ((pFrom)->m_uPri_8 & 0x0f) <<4;

  /* Pack Member: m_uNextpri_8 */
  (pToData)[8] |= ((pFrom)->m_uNextpri_8 & 0x0f);

  /* Pack Member: m_uPri_7 */
  (pToData)[7] |= ((pFrom)->m_uPri_7 & 0x0f) <<4;

  /* Pack Member: m_uNextpri_7 */
  (pToData)[7] |= ((pFrom)->m_uNextpri_7 & 0x0f);

  /* Pack Member: m_uPri_6 */
  (pToData)[6] |= ((pFrom)->m_uPri_6 & 0x0f) <<4;

  /* Pack Member: m_uNextpri_6 */
  (pToData)[6] |= ((pFrom)->m_uNextpri_6 & 0x0f);

  /* Pack Member: m_uPri_5 */
  (pToData)[5] |= ((pFrom)->m_uPri_5 & 0x0f) <<4;

  /* Pack Member: m_uNextpri_5 */
  (pToData)[5] |= ((pFrom)->m_uNextpri_5 & 0x0f);

  /* Pack Member: m_uPri_4 */
  (pToData)[4] |= ((pFrom)->m_uPri_4 & 0x0f) <<4;

  /* Pack Member: m_uNextpri_4 */
  (pToData)[4] |= ((pFrom)->m_uNextpri_4 & 0x0f);

  /* Pack Member: m_uPri_3 */
  (pToData)[3] |= ((pFrom)->m_uPri_3 & 0x0f) <<4;

  /* Pack Member: m_uNextpri_3 */
  (pToData)[3] |= ((pFrom)->m_uNextpri_3 & 0x0f);

  /* Pack Member: m_uPri_2 */
  (pToData)[2] |= ((pFrom)->m_uPri_2 & 0x0f) <<4;

  /* Pack Member: m_uNextpri_2 */
  (pToData)[2] |= ((pFrom)->m_uNextpri_2 & 0x0f);

  /* Pack Member: m_uPri_1 */
  (pToData)[1] |= ((pFrom)->m_uPri_1 & 0x0f) <<4;

  /* Pack Member: m_uNextpri_1 */
  (pToData)[1] |= ((pFrom)->m_uNextpri_1 & 0x0f);

  /* Pack Member: m_uPri_0 */
  (pToData)[0] |= ((pFrom)->m_uPri_0 & 0x0f) <<4;

  /* Pack Member: m_uNextpri_0 */
  (pToData)[0] |= ((pFrom)->m_uNextpri_0 & 0x0f);
#endif

  return SB_ZF_FAB_BM9600_INAPORTPRIENTRY_SIZE_IN_BYTES;
}




/* Unpack from array of bytes into struct */
void
sbZfFabBm9600InaPortPriEntry_Unpack(sbZfFabBm9600InaPortPriEntry_t *pToStruct,
                                    uint8 *pFromData,
                                    uint32 nMaxToDataIndex) {
  COMPILER_UINT64 tmp;

  (void) tmp;

#ifdef SAND_BIG_ENDIAN_HOST

  /* Unpack operation based on bigword endian */

  /* Unpack Member: m_uPri_15 */
  (pToStruct)->m_uPri_15 =  (uint32)  ((pFromData)[12] >> 4) & 0x0f;

  /* Unpack Member: m_uNextpri_15 */
  (pToStruct)->m_uNextpri_15 =  (uint32)  ((pFromData)[12] ) & 0x0f;

  /* Unpack Member: m_uPri_14 */
  (pToStruct)->m_uPri_14 =  (uint32)  ((pFromData)[13] >> 4) & 0x0f;

  /* Unpack Member: m_uNextpri_14 */
  (pToStruct)->m_uNextpri_14 =  (uint32)  ((pFromData)[13] ) & 0x0f;

  /* Unpack Member: m_uPri_13 */
  (pToStruct)->m_uPri_13 =  (uint32)  ((pFromData)[14] >> 4) & 0x0f;

  /* Unpack Member: m_uNextpri_13 */
  (pToStruct)->m_uNextpri_13 =  (uint32)  ((pFromData)[14] ) & 0x0f;

  /* Unpack Member: m_uPri_12 */
  (pToStruct)->m_uPri_12 =  (uint32)  ((pFromData)[15] >> 4) & 0x0f;

  /* Unpack Member: m_uNextpri_12 */
  (pToStruct)->m_uNextpri_12 =  (uint32)  ((pFromData)[15] ) & 0x0f;

  /* Unpack Member: m_uPri_11 */
  (pToStruct)->m_uPri_11 =  (uint32)  ((pFromData)[8] >> 4) & 0x0f;

  /* Unpack Member: m_uNextpri_11 */
  (pToStruct)->m_uNextpri_11 =  (uint32)  ((pFromData)[8] ) & 0x0f;

  /* Unpack Member: m_uPri_10 */
  (pToStruct)->m_uPri_10 =  (uint32)  ((pFromData)[9] >> 4) & 0x0f;

  /* Unpack Member: m_uNextpri_10 */
  (pToStruct)->m_uNextpri_10 =  (uint32)  ((pFromData)[9] ) & 0x0f;

  /* Unpack Member: m_uPri_9 */
  (pToStruct)->m_uPri_9 =  (uint32)  ((pFromData)[10] >> 4) & 0x0f;

  /* Unpack Member: m_uNextpri_9 */
  (pToStruct)->m_uNextpri_9 =  (uint32)  ((pFromData)[10] ) & 0x0f;

  /* Unpack Member: m_uPri_8 */
  (pToStruct)->m_uPri_8 =  (uint32)  ((pFromData)[11] >> 4) & 0x0f;

  /* Unpack Member: m_uNextpri_8 */
  (pToStruct)->m_uNextpri_8 =  (uint32)  ((pFromData)[11] ) & 0x0f;

  /* Unpack Member: m_uPri_7 */
  (pToStruct)->m_uPri_7 =  (uint32)  ((pFromData)[4] >> 4) & 0x0f;

  /* Unpack Member: m_uNextpri_7 */
  (pToStruct)->m_uNextpri_7 =  (uint32)  ((pFromData)[4] ) & 0x0f;

  /* Unpack Member: m_uPri_6 */
  (pToStruct)->m_uPri_6 =  (uint32)  ((pFromData)[5] >> 4) & 0x0f;

  /* Unpack Member: m_uNextpri_6 */
  (pToStruct)->m_uNextpri_6 =  (uint32)  ((pFromData)[5] ) & 0x0f;

  /* Unpack Member: m_uPri_5 */
  (pToStruct)->m_uPri_5 =  (uint32)  ((pFromData)[6] >> 4) & 0x0f;

  /* Unpack Member: m_uNextpri_5 */
  (pToStruct)->m_uNextpri_5 =  (uint32)  ((pFromData)[6] ) & 0x0f;

  /* Unpack Member: m_uPri_4 */
  (pToStruct)->m_uPri_4 =  (uint32)  ((pFromData)[7] >> 4) & 0x0f;

  /* Unpack Member: m_uNextpri_4 */
  (pToStruct)->m_uNextpri_4 =  (uint32)  ((pFromData)[7] ) & 0x0f;

  /* Unpack Member: m_uPri_3 */
  (pToStruct)->m_uPri_3 =  (uint32)  ((pFromData)[0] >> 4) & 0x0f;

  /* Unpack Member: m_uNextpri_3 */
  (pToStruct)->m_uNextpri_3 =  (uint32)  ((pFromData)[0] ) & 0x0f;

  /* Unpack Member: m_uPri_2 */
  (pToStruct)->m_uPri_2 =  (uint32)  ((pFromData)[1] >> 4) & 0x0f;

  /* Unpack Member: m_uNextpri_2 */
  (pToStruct)->m_uNextpri_2 =  (uint32)  ((pFromData)[1] ) & 0x0f;

  /* Unpack Member: m_uPri_1 */
  (pToStruct)->m_uPri_1 =  (uint32)  ((pFromData)[2] >> 4) & 0x0f;

  /* Unpack Member: m_uNextpri_1 */
  (pToStruct)->m_uNextpri_1 =  (uint32)  ((pFromData)[2] ) & 0x0f;

  /* Unpack Member: m_uPri_0 */
  (pToStruct)->m_uPri_0 =  (uint32)  ((pFromData)[3] >> 4) & 0x0f;

  /* Unpack Member: m_uNextpri_0 */
  (pToStruct)->m_uNextpri_0 =  (uint32)  ((pFromData)[3] ) & 0x0f;
#else

  /* Unpack operation based on little endian */

  /* Unpack Member: m_uPri_15 */
  (pToStruct)->m_uPri_15 =  (uint32)  ((pFromData)[15] >> 4) & 0x0f;

  /* Unpack Member: m_uNextpri_15 */
  (pToStruct)->m_uNextpri_15 =  (uint32)  ((pFromData)[15] ) & 0x0f;

  /* Unpack Member: m_uPri_14 */
  (pToStruct)->m_uPri_14 =  (uint32)  ((pFromData)[14] >> 4) & 0x0f;

  /* Unpack Member: m_uNextpri_14 */
  (pToStruct)->m_uNextpri_14 =  (uint32)  ((pFromData)[14] ) & 0x0f;

  /* Unpack Member: m_uPri_13 */
  (pToStruct)->m_uPri_13 =  (uint32)  ((pFromData)[13] >> 4) & 0x0f;

  /* Unpack Member: m_uNextpri_13 */
  (pToStruct)->m_uNextpri_13 =  (uint32)  ((pFromData)[13] ) & 0x0f;

  /* Unpack Member: m_uPri_12 */
  (pToStruct)->m_uPri_12 =  (uint32)  ((pFromData)[12] >> 4) & 0x0f;

  /* Unpack Member: m_uNextpri_12 */
  (pToStruct)->m_uNextpri_12 =  (uint32)  ((pFromData)[12] ) & 0x0f;

  /* Unpack Member: m_uPri_11 */
  (pToStruct)->m_uPri_11 =  (uint32)  ((pFromData)[11] >> 4) & 0x0f;

  /* Unpack Member: m_uNextpri_11 */
  (pToStruct)->m_uNextpri_11 =  (uint32)  ((pFromData)[11] ) & 0x0f;

  /* Unpack Member: m_uPri_10 */
  (pToStruct)->m_uPri_10 =  (uint32)  ((pFromData)[10] >> 4) & 0x0f;

  /* Unpack Member: m_uNextpri_10 */
  (pToStruct)->m_uNextpri_10 =  (uint32)  ((pFromData)[10] ) & 0x0f;

  /* Unpack Member: m_uPri_9 */
  (pToStruct)->m_uPri_9 =  (uint32)  ((pFromData)[9] >> 4) & 0x0f;

  /* Unpack Member: m_uNextpri_9 */
  (pToStruct)->m_uNextpri_9 =  (uint32)  ((pFromData)[9] ) & 0x0f;

  /* Unpack Member: m_uPri_8 */
  (pToStruct)->m_uPri_8 =  (uint32)  ((pFromData)[8] >> 4) & 0x0f;

  /* Unpack Member: m_uNextpri_8 */
  (pToStruct)->m_uNextpri_8 =  (uint32)  ((pFromData)[8] ) & 0x0f;

  /* Unpack Member: m_uPri_7 */
  (pToStruct)->m_uPri_7 =  (uint32)  ((pFromData)[7] >> 4) & 0x0f;

  /* Unpack Member: m_uNextpri_7 */
  (pToStruct)->m_uNextpri_7 =  (uint32)  ((pFromData)[7] ) & 0x0f;

  /* Unpack Member: m_uPri_6 */
  (pToStruct)->m_uPri_6 =  (uint32)  ((pFromData)[6] >> 4) & 0x0f;

  /* Unpack Member: m_uNextpri_6 */
  (pToStruct)->m_uNextpri_6 =  (uint32)  ((pFromData)[6] ) & 0x0f;

  /* Unpack Member: m_uPri_5 */
  (pToStruct)->m_uPri_5 =  (uint32)  ((pFromData)[5] >> 4) & 0x0f;

  /* Unpack Member: m_uNextpri_5 */
  (pToStruct)->m_uNextpri_5 =  (uint32)  ((pFromData)[5] ) & 0x0f;

  /* Unpack Member: m_uPri_4 */
  (pToStruct)->m_uPri_4 =  (uint32)  ((pFromData)[4] >> 4) & 0x0f;

  /* Unpack Member: m_uNextpri_4 */
  (pToStruct)->m_uNextpri_4 =  (uint32)  ((pFromData)[4] ) & 0x0f;

  /* Unpack Member: m_uPri_3 */
  (pToStruct)->m_uPri_3 =  (uint32)  ((pFromData)[3] >> 4) & 0x0f;

  /* Unpack Member: m_uNextpri_3 */
  (pToStruct)->m_uNextpri_3 =  (uint32)  ((pFromData)[3] ) & 0x0f;

  /* Unpack Member: m_uPri_2 */
  (pToStruct)->m_uPri_2 =  (uint32)  ((pFromData)[2] >> 4) & 0x0f;

  /* Unpack Member: m_uNextpri_2 */
  (pToStruct)->m_uNextpri_2 =  (uint32)  ((pFromData)[2] ) & 0x0f;

  /* Unpack Member: m_uPri_1 */
  (pToStruct)->m_uPri_1 =  (uint32)  ((pFromData)[1] >> 4) & 0x0f;

  /* Unpack Member: m_uNextpri_1 */
  (pToStruct)->m_uNextpri_1 =  (uint32)  ((pFromData)[1] ) & 0x0f;

  /* Unpack Member: m_uPri_0 */
  (pToStruct)->m_uPri_0 =  (uint32)  ((pFromData)[0] >> 4) & 0x0f;

  /* Unpack Member: m_uNextpri_0 */
  (pToStruct)->m_uNextpri_0 =  (uint32)  ((pFromData)[0] ) & 0x0f;
#endif

}



/* initialize an instance of this zframe */
void
sbZfFabBm9600InaPortPriEntry_InitInstance(sbZfFabBm9600InaPortPriEntry_t *pFrame) {

  pFrame->m_uPri_15 =  (unsigned int)  0;
  pFrame->m_uNextpri_15 =  (unsigned int)  0;
  pFrame->m_uPri_14 =  (unsigned int)  0;
  pFrame->m_uNextpri_14 =  (unsigned int)  0;
  pFrame->m_uPri_13 =  (unsigned int)  0;
  pFrame->m_uNextpri_13 =  (unsigned int)  0;
  pFrame->m_uPri_12 =  (unsigned int)  0;
  pFrame->m_uNextpri_12 =  (unsigned int)  0;
  pFrame->m_uPri_11 =  (unsigned int)  0;
  pFrame->m_uNextpri_11 =  (unsigned int)  0;
  pFrame->m_uPri_10 =  (unsigned int)  0;
  pFrame->m_uNextpri_10 =  (unsigned int)  0;
  pFrame->m_uPri_9 =  (unsigned int)  0;
  pFrame->m_uNextpri_9 =  (unsigned int)  0;
  pFrame->m_uPri_8 =  (unsigned int)  0;
  pFrame->m_uNextpri_8 =  (unsigned int)  0;
  pFrame->m_uPri_7 =  (unsigned int)  0;
  pFrame->m_uNextpri_7 =  (unsigned int)  0;
  pFrame->m_uPri_6 =  (unsigned int)  0;
  pFrame->m_uNextpri_6 =  (unsigned int)  0;
  pFrame->m_uPri_5 =  (unsigned int)  0;
  pFrame->m_uNextpri_5 =  (unsigned int)  0;
  pFrame->m_uPri_4 =  (unsigned int)  0;
  pFrame->m_uNextpri_4 =  (unsigned int)  0;
  pFrame->m_uPri_3 =  (unsigned int)  0;
  pFrame->m_uNextpri_3 =  (unsigned int)  0;
  pFrame->m_uPri_2 =  (unsigned int)  0;
  pFrame->m_uNextpri_2 =  (unsigned int)  0;
  pFrame->m_uPri_1 =  (unsigned int)  0;
  pFrame->m_uNextpri_1 =  (unsigned int)  0;
  pFrame->m_uPri_0 =  (unsigned int)  0;
  pFrame->m_uNextpri_0 =  (unsigned int)  0;

}
