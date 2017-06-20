/*
 * $Id: sbZfKaQsE2QEntry.c,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


#include "sbTypes.h"
#include "sbZfKaQsE2QEntry.hx"
#include "sbWrappers.h"
#include <sal/types.h>



/* Pack from struct into array of bytes */
uint32
sbZfKaQsE2QEntry_Pack(sbZfKaQsE2QEntry_t *pFrom,
                      uint8 *pToData,
                      uint32 nMaxToDataIndex) {
#ifdef SAND_BIG_ENDIAN_HOST
  int i;
  int size = SB_ZF_ZFKAQSE2QENTRY_SIZE_IN_BYTES;

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on bigword endian */

  /* Pack Member: m_nReserved */
  (pToData)[2] |= ((pFrom)->m_nReserved & 0x01) <<7;
  (pToData)[1] |= ((pFrom)->m_nReserved >> 1) &0xFF;
  (pToData)[0] |= ((pFrom)->m_nReserved >> 9) &0xFF;

  /* Pack Member: m_nEnable */
  (pToData)[2] |= ((pFrom)->m_nEnable & 0x01) <<6;

  /* Pack Member: m_nQueueNum */
  (pToData)[3] |= ((pFrom)->m_nQueueNum) & 0xFF;
  (pToData)[2] |= ((pFrom)->m_nQueueNum >> 8) & 0x3f;
#else
  int i;
  int size = SB_ZF_ZFKAQSE2QENTRY_SIZE_IN_BYTES;

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on little endian */

  /* Pack Member: m_nReserved */
  (pToData)[1] |= ((pFrom)->m_nReserved & 0x01) <<7;
  (pToData)[2] |= ((pFrom)->m_nReserved >> 1) &0xFF;
  (pToData)[3] |= ((pFrom)->m_nReserved >> 9) &0xFF;

  /* Pack Member: m_nEnable */
  (pToData)[1] |= ((pFrom)->m_nEnable & 0x01) <<6;

  /* Pack Member: m_nQueueNum */
  (pToData)[0] |= ((pFrom)->m_nQueueNum) & 0xFF;
  (pToData)[1] |= ((pFrom)->m_nQueueNum >> 8) & 0x3f;
#endif

  return SB_ZF_ZFKAQSE2QENTRY_SIZE_IN_BYTES;
}




/* Unpack from array of bytes into struct */
void
sbZfKaQsE2QEntry_Unpack(sbZfKaQsE2QEntry_t *pToStruct,
                        uint8 *pFromData,
                        uint32 nMaxToDataIndex) {
  COMPILER_UINT64 tmp;

  (void) tmp;

#ifdef SAND_BIG_ENDIAN_HOST

  /* Unpack operation based on bigword endian */

  /* Unpack Member: m_nReserved */
  (pToStruct)->m_nReserved =  (uint32)  ((pFromData)[2] >> 7) & 0x01;
  (pToStruct)->m_nReserved |=  (uint32)  (pFromData)[1] << 1;
  (pToStruct)->m_nReserved |=  (uint32)  (pFromData)[0] << 9;

  /* Unpack Member: m_nEnable */
  (pToStruct)->m_nEnable =  (uint8)  ((pFromData)[2] >> 6) & 0x01;

  /* Unpack Member: m_nQueueNum */
  (pToStruct)->m_nQueueNum =  (uint32)  (pFromData)[3] ;
  (pToStruct)->m_nQueueNum |=  (uint32)  ((pFromData)[2] & 0x3f) << 8;
#else

  /* Unpack operation based on little endian */

  /* Unpack Member: m_nReserved */
  (pToStruct)->m_nReserved =  (uint32)  ((pFromData)[1] >> 7) & 0x01;
  (pToStruct)->m_nReserved |=  (uint32)  (pFromData)[2] << 1;
  (pToStruct)->m_nReserved |=  (uint32)  (pFromData)[3] << 9;

  /* Unpack Member: m_nEnable */
  (pToStruct)->m_nEnable =  (uint8)  ((pFromData)[1] >> 6) & 0x01;

  /* Unpack Member: m_nQueueNum */
  (pToStruct)->m_nQueueNum =  (uint32)  (pFromData)[0] ;
  (pToStruct)->m_nQueueNum |=  (uint32)  ((pFromData)[1] & 0x3f) << 8;
#endif

}



/* initialize an instance of this zframe */
void
sbZfKaQsE2QEntry_InitInstance(sbZfKaQsE2QEntry_t *pFrame) {

  pFrame->m_nReserved =  (unsigned int)  0;
  pFrame->m_nEnable =  (unsigned int)  0;
  pFrame->m_nQueueNum =  (unsigned int)  0;

}
