/*
 * $Id: sbZfKaQsAgeEntry.c,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


#include "sbTypes.h"
#include "sbZfKaQsAgeEntry.hx"
#include "sbWrappers.h"
#include <sal/types.h>



/* Pack from struct into array of bytes */
uint32
sbZfKaQsAgeEntry_Pack(sbZfKaQsAgeEntry_t *pFrom,
                      uint8 *pToData,
                      uint32 nMaxToDataIndex) {
#ifdef SAND_BIG_ENDIAN_HOST
  int i;
  int size = SB_ZF_ZFKAQSAGEENTRY_SIZE_IN_BYTES;

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on bigword endian */

  /* Pack Member: m_nReserved */
  (pToData)[2] |= ((pFrom)->m_nReserved & 0x1f) <<3;
  (pToData)[1] |= ((pFrom)->m_nReserved >> 5) &0xFF;
  (pToData)[0] |= ((pFrom)->m_nReserved >> 13) &0xFF;

  /* Pack Member: m_nNoEmpty */
  (pToData)[2] |= ((pFrom)->m_nNoEmpty & 0x01) <<2;

  /* Pack Member: m_nAnemicEvent */
  (pToData)[2] |= ((pFrom)->m_nAnemicEvent & 0x01) <<1;

  /* Pack Member: m_nEfEvent */
  (pToData)[2] |= ((pFrom)->m_nEfEvent & 0x01);

  /* Pack Member: m_nCnt */
  (pToData)[3] |= ((pFrom)->m_nCnt) & 0xFF;
#else
  int i;
  int size = SB_ZF_ZFKAQSAGEENTRY_SIZE_IN_BYTES;

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on little endian */

  /* Pack Member: m_nReserved */
  (pToData)[1] |= ((pFrom)->m_nReserved & 0x1f) <<3;
  (pToData)[2] |= ((pFrom)->m_nReserved >> 5) &0xFF;
  (pToData)[3] |= ((pFrom)->m_nReserved >> 13) &0xFF;

  /* Pack Member: m_nNoEmpty */
  (pToData)[1] |= ((pFrom)->m_nNoEmpty & 0x01) <<2;

  /* Pack Member: m_nAnemicEvent */
  (pToData)[1] |= ((pFrom)->m_nAnemicEvent & 0x01) <<1;

  /* Pack Member: m_nEfEvent */
  (pToData)[1] |= ((pFrom)->m_nEfEvent & 0x01);

  /* Pack Member: m_nCnt */
  (pToData)[0] |= ((pFrom)->m_nCnt) & 0xFF;
#endif

  return SB_ZF_ZFKAQSAGEENTRY_SIZE_IN_BYTES;
}




/* Unpack from array of bytes into struct */
void
sbZfKaQsAgeEntry_Unpack(sbZfKaQsAgeEntry_t *pToStruct,
                        uint8 *pFromData,
                        uint32 nMaxToDataIndex) {
  COMPILER_UINT64 tmp;

  (void) tmp;

#ifdef SAND_BIG_ENDIAN_HOST

  /* Unpack operation based on bigword endian */

  /* Unpack Member: m_nReserved */
  (pToStruct)->m_nReserved =  (uint32)  ((pFromData)[2] >> 3) & 0x1f;
  (pToStruct)->m_nReserved |=  (uint32)  (pFromData)[1] << 5;
  (pToStruct)->m_nReserved |=  (uint32)  (pFromData)[0] << 13;

  /* Unpack Member: m_nNoEmpty */
  (pToStruct)->m_nNoEmpty =  (uint8)  ((pFromData)[2] >> 2) & 0x01;

  /* Unpack Member: m_nAnemicEvent */
  (pToStruct)->m_nAnemicEvent =  (uint8)  ((pFromData)[2] >> 1) & 0x01;

  /* Unpack Member: m_nEfEvent */
  (pToStruct)->m_nEfEvent =  (uint8)  ((pFromData)[2] ) & 0x01;

  /* Unpack Member: m_nCnt */
  (pToStruct)->m_nCnt =  (uint32)  (pFromData)[3] ;
#else

  /* Unpack operation based on little endian */

  /* Unpack Member: m_nReserved */
  (pToStruct)->m_nReserved =  (uint32)  ((pFromData)[1] >> 3) & 0x1f;
  (pToStruct)->m_nReserved |=  (uint32)  (pFromData)[2] << 5;
  (pToStruct)->m_nReserved |=  (uint32)  (pFromData)[3] << 13;

  /* Unpack Member: m_nNoEmpty */
  (pToStruct)->m_nNoEmpty =  (uint8)  ((pFromData)[1] >> 2) & 0x01;

  /* Unpack Member: m_nAnemicEvent */
  (pToStruct)->m_nAnemicEvent =  (uint8)  ((pFromData)[1] >> 1) & 0x01;

  /* Unpack Member: m_nEfEvent */
  (pToStruct)->m_nEfEvent =  (uint8)  ((pFromData)[1] ) & 0x01;

  /* Unpack Member: m_nCnt */
  (pToStruct)->m_nCnt =  (uint32)  (pFromData)[0] ;
#endif

}



/* initialize an instance of this zframe */
void
sbZfKaQsAgeEntry_InitInstance(sbZfKaQsAgeEntry_t *pFrame) {

  pFrame->m_nReserved =  (unsigned int)  0;
  pFrame->m_nNoEmpty =  (unsigned int)  0;
  pFrame->m_nAnemicEvent =  (unsigned int)  0;
  pFrame->m_nEfEvent =  (unsigned int)  0;
  pFrame->m_nCnt =  (unsigned int)  0;

}
