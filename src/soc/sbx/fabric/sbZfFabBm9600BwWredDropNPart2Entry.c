/*
 * $Id: sbZfFabBm9600BwWredDropNPart2Entry.c,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


#include "sbTypes.h"
#include "sbZfFabBm9600BwWredDropNPart2Entry.hx"
#include "sbWrappers.h"
#include <sal/types.h>



/* Pack from struct into array of bytes */
uint32
sbZfFabBm9600BwWredDropNPart2Entry_Pack(sbZfFabBm9600BwWredDropNPart2Entry_t *pFrom,
                                        uint8 *pToData,
                                        uint32 nMaxToDataIndex) {
#ifdef SAND_BIG_ENDIAN_HOST
  int i;
  int size = SB_ZF_FAB_BM9600_BWWREDDROPNPART2ENTRY_SIZE_IN_BYTES;

  if (size % 4) {
    size += (4 - size %4);
  }

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on bigword endian */

  /* Pack Member: m_uDp2 */
  (pToData)[2] |= ((pFrom)->m_uDp2 & 0x01) <<7;

  /* Pack Member: m_uEcn2 */
  (pToData)[2] |= ((pFrom)->m_uEcn2 & 0x01) <<6;

  /* Pack Member: m_uReserved2 */
  (pToData)[2] |= ((pFrom)->m_uReserved2 & 0x0f) <<2;

  /* Pack Member: m_uPbDe2 */
  (pToData)[3] |= ((pFrom)->m_uPbDe2) & 0xFF;
  (pToData)[2] |= ((pFrom)->m_uPbDe2 >> 8) & 0x03;
#else
  int i;
  int size = SB_ZF_FAB_BM9600_BWWREDDROPNPART2ENTRY_SIZE_IN_BYTES;

  if (size % 4) {
    size += (4 - size %4);
  }

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on little endian */

  /* Pack Member: m_uDp2 */
  (pToData)[1] |= ((pFrom)->m_uDp2 & 0x01) <<7;

  /* Pack Member: m_uEcn2 */
  (pToData)[1] |= ((pFrom)->m_uEcn2 & 0x01) <<6;

  /* Pack Member: m_uReserved2 */
  (pToData)[1] |= ((pFrom)->m_uReserved2 & 0x0f) <<2;

  /* Pack Member: m_uPbDe2 */
  (pToData)[0] |= ((pFrom)->m_uPbDe2) & 0xFF;
  (pToData)[1] |= ((pFrom)->m_uPbDe2 >> 8) & 0x03;
#endif

  return SB_ZF_FAB_BM9600_BWWREDDROPNPART2ENTRY_SIZE_IN_BYTES;
}




/* Unpack from array of bytes into struct */
void
sbZfFabBm9600BwWredDropNPart2Entry_Unpack(sbZfFabBm9600BwWredDropNPart2Entry_t *pToStruct,
                                          uint8 *pFromData,
                                          uint32 nMaxToDataIndex) {
  COMPILER_UINT64 tmp;

  (void) tmp;

#ifdef SAND_BIG_ENDIAN_HOST

  /* Unpack operation based on bigword endian */

  /* Unpack Member: m_uDp2 */
  (pToStruct)->m_uDp2 =  (uint32)  ((pFromData)[2] >> 7) & 0x01;

  /* Unpack Member: m_uEcn2 */
  (pToStruct)->m_uEcn2 =  (uint32)  ((pFromData)[2] >> 6) & 0x01;

  /* Unpack Member: m_uReserved2 */
  (pToStruct)->m_uReserved2 =  (uint32)  ((pFromData)[2] >> 2) & 0x0f;

  /* Unpack Member: m_uPbDe2 */
  (pToStruct)->m_uPbDe2 =  (uint32)  (pFromData)[3] ;
  (pToStruct)->m_uPbDe2 |=  (uint32)  ((pFromData)[2] & 0x03) << 8;
#else

  /* Unpack operation based on little endian */

  /* Unpack Member: m_uDp2 */
  (pToStruct)->m_uDp2 =  (uint32)  ((pFromData)[1] >> 7) & 0x01;

  /* Unpack Member: m_uEcn2 */
  (pToStruct)->m_uEcn2 =  (uint32)  ((pFromData)[1] >> 6) & 0x01;

  /* Unpack Member: m_uReserved2 */
  (pToStruct)->m_uReserved2 =  (uint32)  ((pFromData)[1] >> 2) & 0x0f;

  /* Unpack Member: m_uPbDe2 */
  (pToStruct)->m_uPbDe2 =  (uint32)  (pFromData)[0] ;
  (pToStruct)->m_uPbDe2 |=  (uint32)  ((pFromData)[1] & 0x03) << 8;
#endif

}



/* initialize an instance of this zframe */
void
sbZfFabBm9600BwWredDropNPart2Entry_InitInstance(sbZfFabBm9600BwWredDropNPart2Entry_t *pFrame) {

  pFrame->m_uDp2 =  (unsigned int)  0;
  pFrame->m_uEcn2 =  (unsigned int)  0;
  pFrame->m_uReserved2 =  (unsigned int)  0;
  pFrame->m_uPbDe2 =  (unsigned int)  0;

}
