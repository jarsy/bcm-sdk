/*
 * $Id: sbZfKaSrManualDeskewEntry.c,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


#include "sbTypes.h"
#include "sbZfKaSrManualDeskewEntry.hx"
#include "sbWrappers.h"
#include <sal/types.h>



/* Pack from struct into array of bytes */
uint32
sbZfKaSrManualDeskewEntry_Pack(sbZfKaSrManualDeskewEntry_t *pFrom,
                               uint8 *pToData,
                               uint32 nMaxToDataIndex) {
#ifdef SAND_BIG_ENDIAN_HOST
  int i;
  int size = SB_ZF_ZFKASRMANUALDESKEWENTRY_SIZE_IN_BYTES;

  if (size % 4) {
    size += (4 - size %4);
  }

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on bigword endian */

  /* Pack Member: m_nLane16 */
  (pToData)[9] |= ((pFrom)->m_nLane16 & 0x1f);

  /* Pack Member: m_nLane15 */
  (pToData)[10] |= ((pFrom)->m_nLane15 & 0x1f) <<3;

  /* Pack Member: m_nLane14 */
  (pToData)[11] |= ((pFrom)->m_nLane14 & 0x03) <<6;
  (pToData)[10] |= ((pFrom)->m_nLane14 >> 2) & 0x07;

  /* Pack Member: m_nLane13 */
  (pToData)[11] |= ((pFrom)->m_nLane13 & 0x1f) <<1;

  /* Pack Member: m_nLane12 */
  (pToData)[4] |= ((pFrom)->m_nLane12 & 0x0f) <<4;
  (pToData)[11] |= ((pFrom)->m_nLane12 >> 4) & 0x01;

  /* Pack Member: m_nLane11 */
  (pToData)[5] |= ((pFrom)->m_nLane11 & 0x01) <<7;
  (pToData)[4] |= ((pFrom)->m_nLane11 >> 1) & 0x0f;

  /* Pack Member: m_nLane10 */
  (pToData)[5] |= ((pFrom)->m_nLane10 & 0x1f) <<2;

  /* Pack Member: m_nLane9 */
  (pToData)[6] |= ((pFrom)->m_nLane9 & 0x07) <<5;
  (pToData)[5] |= ((pFrom)->m_nLane9 >> 3) & 0x03;

  /* Pack Member: m_nLane8 */
  (pToData)[6] |= ((pFrom)->m_nLane8 & 0x1f);

  /* Pack Member: m_nLane7 */
  (pToData)[7] |= ((pFrom)->m_nLane7 & 0x1f) <<3;

  /* Pack Member: m_nLane6 */
  (pToData)[0] |= ((pFrom)->m_nLane6 & 0x03) <<6;
  (pToData)[7] |= ((pFrom)->m_nLane6 >> 2) & 0x07;

  /* Pack Member: m_nLane5 */
  (pToData)[0] |= ((pFrom)->m_nLane5 & 0x1f) <<1;

  /* Pack Member: m_nLane4 */
  (pToData)[1] |= ((pFrom)->m_nLane4 & 0x0f) <<4;
  (pToData)[0] |= ((pFrom)->m_nLane4 >> 4) & 0x01;

  /* Pack Member: m_nLane3 */
  (pToData)[2] |= ((pFrom)->m_nLane3 & 0x01) <<7;
  (pToData)[1] |= ((pFrom)->m_nLane3 >> 1) & 0x0f;

  /* Pack Member: m_nLane2 */
  (pToData)[2] |= ((pFrom)->m_nLane2 & 0x1f) <<2;

  /* Pack Member: m_nLane1 */
  (pToData)[3] |= ((pFrom)->m_nLane1 & 0x07) <<5;
  (pToData)[2] |= ((pFrom)->m_nLane1 >> 3) & 0x03;

  /* Pack Member: m_nLane0 */
  (pToData)[3] |= ((pFrom)->m_nLane0 & 0x1f);
#else
  int i;
  int size = SB_ZF_ZFKASRMANUALDESKEWENTRY_SIZE_IN_BYTES;

  if (size % 4) {
    size += (4 - size %4);
  }

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on little endian */

  /* Pack Member: m_nLane16 */
  (pToData)[10] |= ((pFrom)->m_nLane16 & 0x1f);

  /* Pack Member: m_nLane15 */
  (pToData)[9] |= ((pFrom)->m_nLane15 & 0x1f) <<3;

  /* Pack Member: m_nLane14 */
  (pToData)[8] |= ((pFrom)->m_nLane14 & 0x03) <<6;
  (pToData)[9] |= ((pFrom)->m_nLane14 >> 2) & 0x07;

  /* Pack Member: m_nLane13 */
  (pToData)[8] |= ((pFrom)->m_nLane13 & 0x1f) <<1;

  /* Pack Member: m_nLane12 */
  (pToData)[7] |= ((pFrom)->m_nLane12 & 0x0f) <<4;
  (pToData)[8] |= ((pFrom)->m_nLane12 >> 4) & 0x01;

  /* Pack Member: m_nLane11 */
  (pToData)[6] |= ((pFrom)->m_nLane11 & 0x01) <<7;
  (pToData)[7] |= ((pFrom)->m_nLane11 >> 1) & 0x0f;

  /* Pack Member: m_nLane10 */
  (pToData)[6] |= ((pFrom)->m_nLane10 & 0x1f) <<2;

  /* Pack Member: m_nLane9 */
  (pToData)[5] |= ((pFrom)->m_nLane9 & 0x07) <<5;
  (pToData)[6] |= ((pFrom)->m_nLane9 >> 3) & 0x03;

  /* Pack Member: m_nLane8 */
  (pToData)[5] |= ((pFrom)->m_nLane8 & 0x1f);

  /* Pack Member: m_nLane7 */
  (pToData)[4] |= ((pFrom)->m_nLane7 & 0x1f) <<3;

  /* Pack Member: m_nLane6 */
  (pToData)[3] |= ((pFrom)->m_nLane6 & 0x03) <<6;
  (pToData)[4] |= ((pFrom)->m_nLane6 >> 2) & 0x07;

  /* Pack Member: m_nLane5 */
  (pToData)[3] |= ((pFrom)->m_nLane5 & 0x1f) <<1;

  /* Pack Member: m_nLane4 */
  (pToData)[2] |= ((pFrom)->m_nLane4 & 0x0f) <<4;
  (pToData)[3] |= ((pFrom)->m_nLane4 >> 4) & 0x01;

  /* Pack Member: m_nLane3 */
  (pToData)[1] |= ((pFrom)->m_nLane3 & 0x01) <<7;
  (pToData)[2] |= ((pFrom)->m_nLane3 >> 1) & 0x0f;

  /* Pack Member: m_nLane2 */
  (pToData)[1] |= ((pFrom)->m_nLane2 & 0x1f) <<2;

  /* Pack Member: m_nLane1 */
  (pToData)[0] |= ((pFrom)->m_nLane1 & 0x07) <<5;
  (pToData)[1] |= ((pFrom)->m_nLane1 >> 3) & 0x03;

  /* Pack Member: m_nLane0 */
  (pToData)[0] |= ((pFrom)->m_nLane0 & 0x1f);
#endif

  return SB_ZF_ZFKASRMANUALDESKEWENTRY_SIZE_IN_BYTES;
}




/* Unpack from array of bytes into struct */
void
sbZfKaSrManualDeskewEntry_Unpack(sbZfKaSrManualDeskewEntry_t *pToStruct,
                                 uint8 *pFromData,
                                 uint32 nMaxToDataIndex) {
  COMPILER_UINT64 tmp;

  (void) tmp;

#ifdef SAND_BIG_ENDIAN_HOST

  /* Unpack operation based on bigword endian */

  /* Unpack Member: m_nLane16 */
  (pToStruct)->m_nLane16 =  (uint32)  ((pFromData)[9] ) & 0x1f;

  /* Unpack Member: m_nLane15 */
  (pToStruct)->m_nLane15 =  (uint32)  ((pFromData)[10] >> 3) & 0x1f;

  /* Unpack Member: m_nLane14 */
  (pToStruct)->m_nLane14 =  (uint32)  ((pFromData)[11] >> 6) & 0x03;
  (pToStruct)->m_nLane14 |=  (uint32)  ((pFromData)[10] & 0x07) << 2;

  /* Unpack Member: m_nLane13 */
  (pToStruct)->m_nLane13 =  (uint32)  ((pFromData)[11] >> 1) & 0x1f;

  /* Unpack Member: m_nLane12 */
  (pToStruct)->m_nLane12 =  (uint32)  ((pFromData)[4] >> 4) & 0x0f;
  (pToStruct)->m_nLane12 |=  (uint32)  ((pFromData)[11] & 0x01) << 4;

  /* Unpack Member: m_nLane11 */
  (pToStruct)->m_nLane11 =  (uint32)  ((pFromData)[5] >> 7) & 0x01;
  (pToStruct)->m_nLane11 |=  (uint32)  ((pFromData)[4] & 0x0f) << 1;

  /* Unpack Member: m_nLane10 */
  (pToStruct)->m_nLane10 =  (uint32)  ((pFromData)[5] >> 2) & 0x1f;

  /* Unpack Member: m_nLane9 */
  (pToStruct)->m_nLane9 =  (uint32)  ((pFromData)[6] >> 5) & 0x07;
  (pToStruct)->m_nLane9 |=  (uint32)  ((pFromData)[5] & 0x03) << 3;

  /* Unpack Member: m_nLane8 */
  (pToStruct)->m_nLane8 =  (uint32)  ((pFromData)[6] ) & 0x1f;

  /* Unpack Member: m_nLane7 */
  (pToStruct)->m_nLane7 =  (uint32)  ((pFromData)[7] >> 3) & 0x1f;

  /* Unpack Member: m_nLane6 */
  (pToStruct)->m_nLane6 =  (uint32)  ((pFromData)[0] >> 6) & 0x03;
  (pToStruct)->m_nLane6 |=  (uint32)  ((pFromData)[7] & 0x07) << 2;

  /* Unpack Member: m_nLane5 */
  (pToStruct)->m_nLane5 =  (uint32)  ((pFromData)[0] >> 1) & 0x1f;

  /* Unpack Member: m_nLane4 */
  (pToStruct)->m_nLane4 =  (uint32)  ((pFromData)[1] >> 4) & 0x0f;
  (pToStruct)->m_nLane4 |=  (uint32)  ((pFromData)[0] & 0x01) << 4;

  /* Unpack Member: m_nLane3 */
  (pToStruct)->m_nLane3 =  (uint32)  ((pFromData)[2] >> 7) & 0x01;
  (pToStruct)->m_nLane3 |=  (uint32)  ((pFromData)[1] & 0x0f) << 1;

  /* Unpack Member: m_nLane2 */
  (pToStruct)->m_nLane2 =  (uint32)  ((pFromData)[2] >> 2) & 0x1f;

  /* Unpack Member: m_nLane1 */
  (pToStruct)->m_nLane1 =  (uint32)  ((pFromData)[3] >> 5) & 0x07;
  (pToStruct)->m_nLane1 |=  (uint32)  ((pFromData)[2] & 0x03) << 3;

  /* Unpack Member: m_nLane0 */
  (pToStruct)->m_nLane0 =  (uint32)  ((pFromData)[3] ) & 0x1f;
#else

  /* Unpack operation based on little endian */

  /* Unpack Member: m_nLane16 */
  (pToStruct)->m_nLane16 =  (uint32)  ((pFromData)[10] ) & 0x1f;

  /* Unpack Member: m_nLane15 */
  (pToStruct)->m_nLane15 =  (uint32)  ((pFromData)[9] >> 3) & 0x1f;

  /* Unpack Member: m_nLane14 */
  (pToStruct)->m_nLane14 =  (uint32)  ((pFromData)[8] >> 6) & 0x03;
  (pToStruct)->m_nLane14 |=  (uint32)  ((pFromData)[9] & 0x07) << 2;

  /* Unpack Member: m_nLane13 */
  (pToStruct)->m_nLane13 =  (uint32)  ((pFromData)[8] >> 1) & 0x1f;

  /* Unpack Member: m_nLane12 */
  (pToStruct)->m_nLane12 =  (uint32)  ((pFromData)[7] >> 4) & 0x0f;
  (pToStruct)->m_nLane12 |=  (uint32)  ((pFromData)[8] & 0x01) << 4;

  /* Unpack Member: m_nLane11 */
  (pToStruct)->m_nLane11 =  (uint32)  ((pFromData)[6] >> 7) & 0x01;
  (pToStruct)->m_nLane11 |=  (uint32)  ((pFromData)[7] & 0x0f) << 1;

  /* Unpack Member: m_nLane10 */
  (pToStruct)->m_nLane10 =  (uint32)  ((pFromData)[6] >> 2) & 0x1f;

  /* Unpack Member: m_nLane9 */
  (pToStruct)->m_nLane9 =  (uint32)  ((pFromData)[5] >> 5) & 0x07;
  (pToStruct)->m_nLane9 |=  (uint32)  ((pFromData)[6] & 0x03) << 3;

  /* Unpack Member: m_nLane8 */
  (pToStruct)->m_nLane8 =  (uint32)  ((pFromData)[5] ) & 0x1f;

  /* Unpack Member: m_nLane7 */
  (pToStruct)->m_nLane7 =  (uint32)  ((pFromData)[4] >> 3) & 0x1f;

  /* Unpack Member: m_nLane6 */
  (pToStruct)->m_nLane6 =  (uint32)  ((pFromData)[3] >> 6) & 0x03;
  (pToStruct)->m_nLane6 |=  (uint32)  ((pFromData)[4] & 0x07) << 2;

  /* Unpack Member: m_nLane5 */
  (pToStruct)->m_nLane5 =  (uint32)  ((pFromData)[3] >> 1) & 0x1f;

  /* Unpack Member: m_nLane4 */
  (pToStruct)->m_nLane4 =  (uint32)  ((pFromData)[2] >> 4) & 0x0f;
  (pToStruct)->m_nLane4 |=  (uint32)  ((pFromData)[3] & 0x01) << 4;

  /* Unpack Member: m_nLane3 */
  (pToStruct)->m_nLane3 =  (uint32)  ((pFromData)[1] >> 7) & 0x01;
  (pToStruct)->m_nLane3 |=  (uint32)  ((pFromData)[2] & 0x0f) << 1;

  /* Unpack Member: m_nLane2 */
  (pToStruct)->m_nLane2 =  (uint32)  ((pFromData)[1] >> 2) & 0x1f;

  /* Unpack Member: m_nLane1 */
  (pToStruct)->m_nLane1 =  (uint32)  ((pFromData)[0] >> 5) & 0x07;
  (pToStruct)->m_nLane1 |=  (uint32)  ((pFromData)[1] & 0x03) << 3;

  /* Unpack Member: m_nLane0 */
  (pToStruct)->m_nLane0 =  (uint32)  ((pFromData)[0] ) & 0x1f;
#endif

}



/* initialize an instance of this zframe */
void
sbZfKaSrManualDeskewEntry_InitInstance(sbZfKaSrManualDeskewEntry_t *pFrame) {

  pFrame->m_nLane16 =  (unsigned int)  0;
  pFrame->m_nLane15 =  (unsigned int)  0;
  pFrame->m_nLane14 =  (unsigned int)  0;
  pFrame->m_nLane13 =  (unsigned int)  0;
  pFrame->m_nLane12 =  (unsigned int)  0;
  pFrame->m_nLane11 =  (unsigned int)  0;
  pFrame->m_nLane10 =  (unsigned int)  0;
  pFrame->m_nLane9 =  (unsigned int)  0;
  pFrame->m_nLane8 =  (unsigned int)  0;
  pFrame->m_nLane7 =  (unsigned int)  0;
  pFrame->m_nLane6 =  (unsigned int)  0;
  pFrame->m_nLane5 =  (unsigned int)  0;
  pFrame->m_nLane4 =  (unsigned int)  0;
  pFrame->m_nLane3 =  (unsigned int)  0;
  pFrame->m_nLane2 =  (unsigned int)  0;
  pFrame->m_nLane1 =  (unsigned int)  0;
  pFrame->m_nLane0 =  (unsigned int)  0;

}
