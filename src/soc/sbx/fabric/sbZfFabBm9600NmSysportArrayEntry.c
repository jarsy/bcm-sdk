/*
 * $Id: sbZfFabBm9600NmSysportArrayEntry.c,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


#include "sbTypes.h"
#include "sbZfFabBm9600NmSysportArrayEntry.hx"
#include "sbWrappers.h"
#include <sal/types.h>



/* Pack from struct into array of bytes */
uint32
sbZfFabBm9600NmSysportArrayEntry_Pack(sbZfFabBm9600NmSysportArrayEntry_t *pFrom,
                                      uint8 *pToData,
                                      uint32 nMaxToDataIndex) {
#ifdef SAND_BIG_ENDIAN_HOST
  int i;
  int size = SB_ZF_FAB_BM9600_NMSYSPORTARRAYENTRY_SIZE_IN_BYTES;

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on bigword endian */

  /* Pack Member: m_uSpa_15 */
  (pToData)[21] |= ((pFrom)->m_uSpa_15 & 0x0f) <<4;
  (pToData)[20] |= ((pFrom)->m_uSpa_15 >> 4) &0xFF;

  /* Pack Member: m_uSpa_14 */
  (pToData)[22] |= ((pFrom)->m_uSpa_14) & 0xFF;
  (pToData)[21] |= ((pFrom)->m_uSpa_14 >> 8) & 0x0f;

  /* Pack Member: m_uSpa_13 */
  (pToData)[16] |= ((pFrom)->m_uSpa_13 & 0x0f) <<4;
  (pToData)[23] |= ((pFrom)->m_uSpa_13 >> 4) &0xFF;

  /* Pack Member: m_uSpa_12 */
  (pToData)[17] |= ((pFrom)->m_uSpa_12) & 0xFF;
  (pToData)[16] |= ((pFrom)->m_uSpa_12 >> 8) & 0x0f;

  /* Pack Member: m_uSpa_11 */
  (pToData)[19] |= ((pFrom)->m_uSpa_11 & 0x0f) <<4;
  (pToData)[18] |= ((pFrom)->m_uSpa_11 >> 4) &0xFF;

  /* Pack Member: m_uSpa_10 */
  (pToData)[12] |= ((pFrom)->m_uSpa_10) & 0xFF;
  (pToData)[19] |= ((pFrom)->m_uSpa_10 >> 8) & 0x0f;

  /* Pack Member: m_uSpa_9 */
  (pToData)[14] |= ((pFrom)->m_uSpa_9 & 0x0f) <<4;
  (pToData)[13] |= ((pFrom)->m_uSpa_9 >> 4) &0xFF;

  /* Pack Member: m_uSpa_8 */
  (pToData)[15] |= ((pFrom)->m_uSpa_8) & 0xFF;
  (pToData)[14] |= ((pFrom)->m_uSpa_8 >> 8) & 0x0f;

  /* Pack Member: m_uSpa_7 */
  (pToData)[9] |= ((pFrom)->m_uSpa_7 & 0x0f) <<4;
  (pToData)[8] |= ((pFrom)->m_uSpa_7 >> 4) &0xFF;

  /* Pack Member: m_uSpa_6 */
  (pToData)[10] |= ((pFrom)->m_uSpa_6) & 0xFF;
  (pToData)[9] |= ((pFrom)->m_uSpa_6 >> 8) & 0x0f;

  /* Pack Member: m_uSpa_5 */
  (pToData)[4] |= ((pFrom)->m_uSpa_5 & 0x0f) <<4;
  (pToData)[11] |= ((pFrom)->m_uSpa_5 >> 4) &0xFF;

  /* Pack Member: m_uSpa_4 */
  (pToData)[5] |= ((pFrom)->m_uSpa_4) & 0xFF;
  (pToData)[4] |= ((pFrom)->m_uSpa_4 >> 8) & 0x0f;

  /* Pack Member: m_uSpa_3 */
  (pToData)[7] |= ((pFrom)->m_uSpa_3 & 0x0f) <<4;
  (pToData)[6] |= ((pFrom)->m_uSpa_3 >> 4) &0xFF;

  /* Pack Member: m_uSpa_2 */
  (pToData)[0] |= ((pFrom)->m_uSpa_2) & 0xFF;
  (pToData)[7] |= ((pFrom)->m_uSpa_2 >> 8) & 0x0f;

  /* Pack Member: m_uSpa_1 */
  (pToData)[2] |= ((pFrom)->m_uSpa_1 & 0x0f) <<4;
  (pToData)[1] |= ((pFrom)->m_uSpa_1 >> 4) &0xFF;

  /* Pack Member: m_uSpa_0 */
  (pToData)[3] |= ((pFrom)->m_uSpa_0) & 0xFF;
  (pToData)[2] |= ((pFrom)->m_uSpa_0 >> 8) & 0x0f;
#else
  int i;
  int size = SB_ZF_FAB_BM9600_NMSYSPORTARRAYENTRY_SIZE_IN_BYTES;

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on little endian */

  /* Pack Member: m_uSpa_15 */
  (pToData)[22] |= ((pFrom)->m_uSpa_15 & 0x0f) <<4;
  (pToData)[23] |= ((pFrom)->m_uSpa_15 >> 4) &0xFF;

  /* Pack Member: m_uSpa_14 */
  (pToData)[21] |= ((pFrom)->m_uSpa_14) & 0xFF;
  (pToData)[22] |= ((pFrom)->m_uSpa_14 >> 8) & 0x0f;

  /* Pack Member: m_uSpa_13 */
  (pToData)[19] |= ((pFrom)->m_uSpa_13 & 0x0f) <<4;
  (pToData)[20] |= ((pFrom)->m_uSpa_13 >> 4) &0xFF;

  /* Pack Member: m_uSpa_12 */
  (pToData)[18] |= ((pFrom)->m_uSpa_12) & 0xFF;
  (pToData)[19] |= ((pFrom)->m_uSpa_12 >> 8) & 0x0f;

  /* Pack Member: m_uSpa_11 */
  (pToData)[16] |= ((pFrom)->m_uSpa_11 & 0x0f) <<4;
  (pToData)[17] |= ((pFrom)->m_uSpa_11 >> 4) &0xFF;

  /* Pack Member: m_uSpa_10 */
  (pToData)[15] |= ((pFrom)->m_uSpa_10) & 0xFF;
  (pToData)[16] |= ((pFrom)->m_uSpa_10 >> 8) & 0x0f;

  /* Pack Member: m_uSpa_9 */
  (pToData)[13] |= ((pFrom)->m_uSpa_9 & 0x0f) <<4;
  (pToData)[14] |= ((pFrom)->m_uSpa_9 >> 4) &0xFF;

  /* Pack Member: m_uSpa_8 */
  (pToData)[12] |= ((pFrom)->m_uSpa_8) & 0xFF;
  (pToData)[13] |= ((pFrom)->m_uSpa_8 >> 8) & 0x0f;

  /* Pack Member: m_uSpa_7 */
  (pToData)[10] |= ((pFrom)->m_uSpa_7 & 0x0f) <<4;
  (pToData)[11] |= ((pFrom)->m_uSpa_7 >> 4) &0xFF;

  /* Pack Member: m_uSpa_6 */
  (pToData)[9] |= ((pFrom)->m_uSpa_6) & 0xFF;
  (pToData)[10] |= ((pFrom)->m_uSpa_6 >> 8) & 0x0f;

  /* Pack Member: m_uSpa_5 */
  (pToData)[7] |= ((pFrom)->m_uSpa_5 & 0x0f) <<4;
  (pToData)[8] |= ((pFrom)->m_uSpa_5 >> 4) &0xFF;

  /* Pack Member: m_uSpa_4 */
  (pToData)[6] |= ((pFrom)->m_uSpa_4) & 0xFF;
  (pToData)[7] |= ((pFrom)->m_uSpa_4 >> 8) & 0x0f;

  /* Pack Member: m_uSpa_3 */
  (pToData)[4] |= ((pFrom)->m_uSpa_3 & 0x0f) <<4;
  (pToData)[5] |= ((pFrom)->m_uSpa_3 >> 4) &0xFF;

  /* Pack Member: m_uSpa_2 */
  (pToData)[3] |= ((pFrom)->m_uSpa_2) & 0xFF;
  (pToData)[4] |= ((pFrom)->m_uSpa_2 >> 8) & 0x0f;

  /* Pack Member: m_uSpa_1 */
  (pToData)[1] |= ((pFrom)->m_uSpa_1 & 0x0f) <<4;
  (pToData)[2] |= ((pFrom)->m_uSpa_1 >> 4) &0xFF;

  /* Pack Member: m_uSpa_0 */
  (pToData)[0] |= ((pFrom)->m_uSpa_0) & 0xFF;
  (pToData)[1] |= ((pFrom)->m_uSpa_0 >> 8) & 0x0f;
#endif

  return SB_ZF_FAB_BM9600_NMSYSPORTARRAYENTRY_SIZE_IN_BYTES;
}




/* Unpack from array of bytes into struct */
void
sbZfFabBm9600NmSysportArrayEntry_Unpack(sbZfFabBm9600NmSysportArrayEntry_t *pToStruct,
                                        uint8 *pFromData,
                                        uint32 nMaxToDataIndex) {
  COMPILER_UINT64 tmp;

  (void) tmp;

#ifdef SAND_BIG_ENDIAN_HOST

  /* Unpack operation based on bigword endian */

  /* Unpack Member: m_uSpa_15 */
  (pToStruct)->m_uSpa_15 =  (uint32)  ((pFromData)[21] >> 4) & 0x0f;
  (pToStruct)->m_uSpa_15 |=  (uint32)  (pFromData)[20] << 4;

  /* Unpack Member: m_uSpa_14 */
  (pToStruct)->m_uSpa_14 =  (uint32)  (pFromData)[22] ;
  (pToStruct)->m_uSpa_14 |=  (uint32)  ((pFromData)[21] & 0x0f) << 8;

  /* Unpack Member: m_uSpa_13 */
  (pToStruct)->m_uSpa_13 =  (uint32)  ((pFromData)[16] >> 4) & 0x0f;
  (pToStruct)->m_uSpa_13 |=  (uint32)  (pFromData)[23] << 4;

  /* Unpack Member: m_uSpa_12 */
  (pToStruct)->m_uSpa_12 =  (uint32)  (pFromData)[17] ;
  (pToStruct)->m_uSpa_12 |=  (uint32)  ((pFromData)[16] & 0x0f) << 8;

  /* Unpack Member: m_uSpa_11 */
  (pToStruct)->m_uSpa_11 =  (uint32)  ((pFromData)[19] >> 4) & 0x0f;
  (pToStruct)->m_uSpa_11 |=  (uint32)  (pFromData)[18] << 4;

  /* Unpack Member: m_uSpa_10 */
  (pToStruct)->m_uSpa_10 =  (uint32)  (pFromData)[12] ;
  (pToStruct)->m_uSpa_10 |=  (uint32)  ((pFromData)[19] & 0x0f) << 8;

  /* Unpack Member: m_uSpa_9 */
  (pToStruct)->m_uSpa_9 =  (uint32)  ((pFromData)[14] >> 4) & 0x0f;
  (pToStruct)->m_uSpa_9 |=  (uint32)  (pFromData)[13] << 4;

  /* Unpack Member: m_uSpa_8 */
  (pToStruct)->m_uSpa_8 =  (uint32)  (pFromData)[15] ;
  (pToStruct)->m_uSpa_8 |=  (uint32)  ((pFromData)[14] & 0x0f) << 8;

  /* Unpack Member: m_uSpa_7 */
  (pToStruct)->m_uSpa_7 =  (uint32)  ((pFromData)[9] >> 4) & 0x0f;
  (pToStruct)->m_uSpa_7 |=  (uint32)  (pFromData)[8] << 4;

  /* Unpack Member: m_uSpa_6 */
  (pToStruct)->m_uSpa_6 =  (uint32)  (pFromData)[10] ;
  (pToStruct)->m_uSpa_6 |=  (uint32)  ((pFromData)[9] & 0x0f) << 8;

  /* Unpack Member: m_uSpa_5 */
  (pToStruct)->m_uSpa_5 =  (uint32)  ((pFromData)[4] >> 4) & 0x0f;
  (pToStruct)->m_uSpa_5 |=  (uint32)  (pFromData)[11] << 4;

  /* Unpack Member: m_uSpa_4 */
  (pToStruct)->m_uSpa_4 =  (uint32)  (pFromData)[5] ;
  (pToStruct)->m_uSpa_4 |=  (uint32)  ((pFromData)[4] & 0x0f) << 8;

  /* Unpack Member: m_uSpa_3 */
  (pToStruct)->m_uSpa_3 =  (uint32)  ((pFromData)[7] >> 4) & 0x0f;
  (pToStruct)->m_uSpa_3 |=  (uint32)  (pFromData)[6] << 4;

  /* Unpack Member: m_uSpa_2 */
  (pToStruct)->m_uSpa_2 =  (uint32)  (pFromData)[0] ;
  (pToStruct)->m_uSpa_2 |=  (uint32)  ((pFromData)[7] & 0x0f) << 8;

  /* Unpack Member: m_uSpa_1 */
  (pToStruct)->m_uSpa_1 =  (uint32)  ((pFromData)[2] >> 4) & 0x0f;
  (pToStruct)->m_uSpa_1 |=  (uint32)  (pFromData)[1] << 4;

  /* Unpack Member: m_uSpa_0 */
  (pToStruct)->m_uSpa_0 =  (uint32)  (pFromData)[3] ;
  (pToStruct)->m_uSpa_0 |=  (uint32)  ((pFromData)[2] & 0x0f) << 8;
#else

  /* Unpack operation based on little endian */

  /* Unpack Member: m_uSpa_15 */
  (pToStruct)->m_uSpa_15 =  (uint32)  ((pFromData)[22] >> 4) & 0x0f;
  (pToStruct)->m_uSpa_15 |=  (uint32)  (pFromData)[23] << 4;

  /* Unpack Member: m_uSpa_14 */
  (pToStruct)->m_uSpa_14 =  (uint32)  (pFromData)[21] ;
  (pToStruct)->m_uSpa_14 |=  (uint32)  ((pFromData)[22] & 0x0f) << 8;

  /* Unpack Member: m_uSpa_13 */
  (pToStruct)->m_uSpa_13 =  (uint32)  ((pFromData)[19] >> 4) & 0x0f;
  (pToStruct)->m_uSpa_13 |=  (uint32)  (pFromData)[20] << 4;

  /* Unpack Member: m_uSpa_12 */
  (pToStruct)->m_uSpa_12 =  (uint32)  (pFromData)[18] ;
  (pToStruct)->m_uSpa_12 |=  (uint32)  ((pFromData)[19] & 0x0f) << 8;

  /* Unpack Member: m_uSpa_11 */
  (pToStruct)->m_uSpa_11 =  (uint32)  ((pFromData)[16] >> 4) & 0x0f;
  (pToStruct)->m_uSpa_11 |=  (uint32)  (pFromData)[17] << 4;

  /* Unpack Member: m_uSpa_10 */
  (pToStruct)->m_uSpa_10 =  (uint32)  (pFromData)[15] ;
  (pToStruct)->m_uSpa_10 |=  (uint32)  ((pFromData)[16] & 0x0f) << 8;

  /* Unpack Member: m_uSpa_9 */
  (pToStruct)->m_uSpa_9 =  (uint32)  ((pFromData)[13] >> 4) & 0x0f;
  (pToStruct)->m_uSpa_9 |=  (uint32)  (pFromData)[14] << 4;

  /* Unpack Member: m_uSpa_8 */
  (pToStruct)->m_uSpa_8 =  (uint32)  (pFromData)[12] ;
  (pToStruct)->m_uSpa_8 |=  (uint32)  ((pFromData)[13] & 0x0f) << 8;

  /* Unpack Member: m_uSpa_7 */
  (pToStruct)->m_uSpa_7 =  (uint32)  ((pFromData)[10] >> 4) & 0x0f;
  (pToStruct)->m_uSpa_7 |=  (uint32)  (pFromData)[11] << 4;

  /* Unpack Member: m_uSpa_6 */
  (pToStruct)->m_uSpa_6 =  (uint32)  (pFromData)[9] ;
  (pToStruct)->m_uSpa_6 |=  (uint32)  ((pFromData)[10] & 0x0f) << 8;

  /* Unpack Member: m_uSpa_5 */
  (pToStruct)->m_uSpa_5 =  (uint32)  ((pFromData)[7] >> 4) & 0x0f;
  (pToStruct)->m_uSpa_5 |=  (uint32)  (pFromData)[8] << 4;

  /* Unpack Member: m_uSpa_4 */
  (pToStruct)->m_uSpa_4 =  (uint32)  (pFromData)[6] ;
  (pToStruct)->m_uSpa_4 |=  (uint32)  ((pFromData)[7] & 0x0f) << 8;

  /* Unpack Member: m_uSpa_3 */
  (pToStruct)->m_uSpa_3 =  (uint32)  ((pFromData)[4] >> 4) & 0x0f;
  (pToStruct)->m_uSpa_3 |=  (uint32)  (pFromData)[5] << 4;

  /* Unpack Member: m_uSpa_2 */
  (pToStruct)->m_uSpa_2 =  (uint32)  (pFromData)[3] ;
  (pToStruct)->m_uSpa_2 |=  (uint32)  ((pFromData)[4] & 0x0f) << 8;

  /* Unpack Member: m_uSpa_1 */
  (pToStruct)->m_uSpa_1 =  (uint32)  ((pFromData)[1] >> 4) & 0x0f;
  (pToStruct)->m_uSpa_1 |=  (uint32)  (pFromData)[2] << 4;

  /* Unpack Member: m_uSpa_0 */
  (pToStruct)->m_uSpa_0 =  (uint32)  (pFromData)[0] ;
  (pToStruct)->m_uSpa_0 |=  (uint32)  ((pFromData)[1] & 0x0f) << 8;
#endif

}



/* initialize an instance of this zframe */
void
sbZfFabBm9600NmSysportArrayEntry_InitInstance(sbZfFabBm9600NmSysportArrayEntry_t *pFrame) {

  pFrame->m_uSpa_15 =  (unsigned int)  0;
  pFrame->m_uSpa_14 =  (unsigned int)  0;
  pFrame->m_uSpa_13 =  (unsigned int)  0;
  pFrame->m_uSpa_12 =  (unsigned int)  0;
  pFrame->m_uSpa_11 =  (unsigned int)  0;
  pFrame->m_uSpa_10 =  (unsigned int)  0;
  pFrame->m_uSpa_9 =  (unsigned int)  0;
  pFrame->m_uSpa_8 =  (unsigned int)  0;
  pFrame->m_uSpa_7 =  (unsigned int)  0;
  pFrame->m_uSpa_6 =  (unsigned int)  0;
  pFrame->m_uSpa_5 =  (unsigned int)  0;
  pFrame->m_uSpa_4 =  (unsigned int)  0;
  pFrame->m_uSpa_3 =  (unsigned int)  0;
  pFrame->m_uSpa_2 =  (unsigned int)  0;
  pFrame->m_uSpa_1 =  (unsigned int)  0;
  pFrame->m_uSpa_0 =  (unsigned int)  0;

}
