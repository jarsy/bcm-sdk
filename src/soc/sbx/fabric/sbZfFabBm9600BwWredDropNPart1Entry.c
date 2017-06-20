/*
 * $Id: sbZfFabBm9600BwWredDropNPart1Entry.c,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


#include "sbTypes.h"
#include "sbZfFabBm9600BwWredDropNPart1Entry.hx"
#include "sbWrappers.h"
#include <sal/types.h>



/* Pack from struct into array of bytes */
uint32
sbZfFabBm9600BwWredDropNPart1Entry_Pack(sbZfFabBm9600BwWredDropNPart1Entry_t *pFrom,
                                        uint8 *pToData,
                                        uint32 nMaxToDataIndex) {
#ifdef SAND_BIG_ENDIAN_HOST
  int i;
  int size = SB_ZF_FAB_BM9600_BWWREDDROPNPART1ENTRY_SIZE_IN_BYTES;

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on bigword endian */

  /* Pack Member: m_uDp1 */
  (pToData)[0] |= ((pFrom)->m_uDp1 & 0x01) <<7;

  /* Pack Member: m_uEcn1 */
  (pToData)[0] |= ((pFrom)->m_uEcn1 & 0x01) <<6;

  /* Pack Member: m_uReserved1 */
  (pToData)[0] |= ((pFrom)->m_uReserved1 & 0x0f) <<2;

  /* Pack Member: m_uPbDe1 */
  (pToData)[1] |= ((pFrom)->m_uPbDe1) & 0xFF;
  (pToData)[0] |= ((pFrom)->m_uPbDe1 >> 8) & 0x03;

  /* Pack Member: m_uDp0 */
  (pToData)[2] |= ((pFrom)->m_uDp0 & 0x01) <<7;

  /* Pack Member: m_uEcn0 */
  (pToData)[2] |= ((pFrom)->m_uEcn0 & 0x01) <<6;

  /* Pack Member: m_uReserved0 */
  (pToData)[2] |= ((pFrom)->m_uReserved0 & 0x0f) <<2;

  /* Pack Member: m_uPbDe0 */
  (pToData)[3] |= ((pFrom)->m_uPbDe0) & 0xFF;
  (pToData)[2] |= ((pFrom)->m_uPbDe0 >> 8) & 0x03;
#else
  int i;
  int size = SB_ZF_FAB_BM9600_BWWREDDROPNPART1ENTRY_SIZE_IN_BYTES;

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on little endian */

  /* Pack Member: m_uDp1 */
  (pToData)[3] |= ((pFrom)->m_uDp1 & 0x01) <<7;

  /* Pack Member: m_uEcn1 */
  (pToData)[3] |= ((pFrom)->m_uEcn1 & 0x01) <<6;

  /* Pack Member: m_uReserved1 */
  (pToData)[3] |= ((pFrom)->m_uReserved1 & 0x0f) <<2;

  /* Pack Member: m_uPbDe1 */
  (pToData)[2] |= ((pFrom)->m_uPbDe1) & 0xFF;
  (pToData)[3] |= ((pFrom)->m_uPbDe1 >> 8) & 0x03;

  /* Pack Member: m_uDp0 */
  (pToData)[1] |= ((pFrom)->m_uDp0 & 0x01) <<7;

  /* Pack Member: m_uEcn0 */
  (pToData)[1] |= ((pFrom)->m_uEcn0 & 0x01) <<6;

  /* Pack Member: m_uReserved0 */
  (pToData)[1] |= ((pFrom)->m_uReserved0 & 0x0f) <<2;

  /* Pack Member: m_uPbDe0 */
  (pToData)[0] |= ((pFrom)->m_uPbDe0) & 0xFF;
  (pToData)[1] |= ((pFrom)->m_uPbDe0 >> 8) & 0x03;
#endif

  return SB_ZF_FAB_BM9600_BWWREDDROPNPART1ENTRY_SIZE_IN_BYTES;
}




/* Unpack from array of bytes into struct */
void
sbZfFabBm9600BwWredDropNPart1Entry_Unpack(sbZfFabBm9600BwWredDropNPart1Entry_t *pToStruct,
                                          uint8 *pFromData,
                                          uint32 nMaxToDataIndex) {
  COMPILER_UINT64 tmp;

  (void) tmp;

#ifdef SAND_BIG_ENDIAN_HOST

  /* Unpack operation based on bigword endian */

  /* Unpack Member: m_uDp1 */
  (pToStruct)->m_uDp1 =  (uint32)  ((pFromData)[0] >> 7) & 0x01;

  /* Unpack Member: m_uEcn1 */
  (pToStruct)->m_uEcn1 =  (uint32)  ((pFromData)[0] >> 6) & 0x01;

  /* Unpack Member: m_uReserved1 */
  (pToStruct)->m_uReserved1 =  (uint32)  ((pFromData)[0] >> 2) & 0x0f;

  /* Unpack Member: m_uPbDe1 */
  (pToStruct)->m_uPbDe1 =  (uint32)  (pFromData)[1] ;
  (pToStruct)->m_uPbDe1 |=  (uint32)  ((pFromData)[0] & 0x03) << 8;

  /* Unpack Member: m_uDp0 */
  (pToStruct)->m_uDp0 =  (uint32)  ((pFromData)[2] >> 7) & 0x01;

  /* Unpack Member: m_uEcn0 */
  (pToStruct)->m_uEcn0 =  (uint32)  ((pFromData)[2] >> 6) & 0x01;

  /* Unpack Member: m_uReserved0 */
  (pToStruct)->m_uReserved0 =  (uint32)  ((pFromData)[2] >> 2) & 0x0f;

  /* Unpack Member: m_uPbDe0 */
  (pToStruct)->m_uPbDe0 =  (uint32)  (pFromData)[3] ;
  (pToStruct)->m_uPbDe0 |=  (uint32)  ((pFromData)[2] & 0x03) << 8;
#else

  /* Unpack operation based on little endian */

  /* Unpack Member: m_uDp1 */
  (pToStruct)->m_uDp1 =  (uint32)  ((pFromData)[3] >> 7) & 0x01;

  /* Unpack Member: m_uEcn1 */
  (pToStruct)->m_uEcn1 =  (uint32)  ((pFromData)[3] >> 6) & 0x01;

  /* Unpack Member: m_uReserved1 */
  (pToStruct)->m_uReserved1 =  (uint32)  ((pFromData)[3] >> 2) & 0x0f;

  /* Unpack Member: m_uPbDe1 */
  (pToStruct)->m_uPbDe1 =  (uint32)  (pFromData)[2] ;
  (pToStruct)->m_uPbDe1 |=  (uint32)  ((pFromData)[3] & 0x03) << 8;

  /* Unpack Member: m_uDp0 */
  (pToStruct)->m_uDp0 =  (uint32)  ((pFromData)[1] >> 7) & 0x01;

  /* Unpack Member: m_uEcn0 */
  (pToStruct)->m_uEcn0 =  (uint32)  ((pFromData)[1] >> 6) & 0x01;

  /* Unpack Member: m_uReserved0 */
  (pToStruct)->m_uReserved0 =  (uint32)  ((pFromData)[1] >> 2) & 0x0f;

  /* Unpack Member: m_uPbDe0 */
  (pToStruct)->m_uPbDe0 =  (uint32)  (pFromData)[0] ;
  (pToStruct)->m_uPbDe0 |=  (uint32)  ((pFromData)[1] & 0x03) << 8;
#endif

}



/* initialize an instance of this zframe */
void
sbZfFabBm9600BwWredDropNPart1Entry_InitInstance(sbZfFabBm9600BwWredDropNPart1Entry_t *pFrame) {

  pFrame->m_uDp1 =  (unsigned int)  0;
  pFrame->m_uEcn1 =  (unsigned int)  0;
  pFrame->m_uReserved1 =  (unsigned int)  0;
  pFrame->m_uPbDe1 =  (unsigned int)  0;
  pFrame->m_uDp0 =  (unsigned int)  0;
  pFrame->m_uEcn0 =  (unsigned int)  0;
  pFrame->m_uReserved0 =  (unsigned int)  0;
  pFrame->m_uPbDe0 =  (unsigned int)  0;

}
