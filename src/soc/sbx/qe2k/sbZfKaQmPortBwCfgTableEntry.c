/*
 * $Id: sbZfKaQmPortBwCfgTableEntry.c,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


#include "sbTypes.h"
#include "sbZfKaQmPortBwCfgTableEntry.hx"
#include "sbWrappers.h"
#include <sal/types.h>



/* Pack from struct into array of bytes */
uint32
sbZfKaQmPortBwCfgTableEntry_Pack(sbZfKaQmPortBwCfgTableEntry_t *pFrom,
                                 uint8 *pToData,
                                 uint32 nMaxToDataIndex) {
#ifdef SAND_BIG_ENDIAN_HOST
  int i;
  int size = SB_ZF_ZFKAQMPORTBWCFGTABLEENTRY_SIZE_IN_BYTES;

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on bigword endian */

  /* Pack Member: m_nSpQueues */
  (pToData)[6] |= ((pFrom)->m_nSpQueues & 0x1f) <<1;

  /* Pack Member: m_nQueues */
  (pToData)[7] |= ((pFrom)->m_nQueues & 0x0f) <<4;
  (pToData)[6] |= ((pFrom)->m_nQueues >> 4) & 0x01;

  /* Pack Member: m_nBaseQueue */
  (pToData)[1] |= ((pFrom)->m_nBaseQueue & 0x03) <<6;
  (pToData)[0] |= ((pFrom)->m_nBaseQueue >> 2) &0xFF;
  (pToData)[7] |= ((pFrom)->m_nBaseQueue >> 10) & 0x0f;

  /* Pack Member: m_nLineRate */
  (pToData)[3] |= ((pFrom)->m_nLineRate) & 0xFF;
  (pToData)[2] |= ((pFrom)->m_nLineRate >> 8) &0xFF;
  (pToData)[1] |= ((pFrom)->m_nLineRate >> 16) & 0x3f;
#else
  int i;
  int size = SB_ZF_ZFKAQMPORTBWCFGTABLEENTRY_SIZE_IN_BYTES;

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on little endian */

  /* Pack Member: m_nSpQueues */
  (pToData)[5] |= ((pFrom)->m_nSpQueues & 0x1f) <<1;

  /* Pack Member: m_nQueues */
  (pToData)[4] |= ((pFrom)->m_nQueues & 0x0f) <<4;
  (pToData)[5] |= ((pFrom)->m_nQueues >> 4) & 0x01;

  /* Pack Member: m_nBaseQueue */
  (pToData)[2] |= ((pFrom)->m_nBaseQueue & 0x03) <<6;
  (pToData)[3] |= ((pFrom)->m_nBaseQueue >> 2) &0xFF;
  (pToData)[4] |= ((pFrom)->m_nBaseQueue >> 10) & 0x0f;

  /* Pack Member: m_nLineRate */
  (pToData)[0] |= ((pFrom)->m_nLineRate) & 0xFF;
  (pToData)[1] |= ((pFrom)->m_nLineRate >> 8) &0xFF;
  (pToData)[2] |= ((pFrom)->m_nLineRate >> 16) & 0x3f;
#endif

  return SB_ZF_ZFKAQMPORTBWCFGTABLEENTRY_SIZE_IN_BYTES;
}




/* Unpack from array of bytes into struct */
void
sbZfKaQmPortBwCfgTableEntry_Unpack(sbZfKaQmPortBwCfgTableEntry_t *pToStruct,
                                   uint8 *pFromData,
                                   uint32 nMaxToDataIndex) {
  COMPILER_UINT64 tmp;

  (void) tmp;

#ifdef SAND_BIG_ENDIAN_HOST

  /* Unpack operation based on bigword endian */

  /* Unpack Member: m_nSpQueues */
  (pToStruct)->m_nSpQueues =  (uint32)  ((pFromData)[6] >> 1) & 0x1f;

  /* Unpack Member: m_nQueues */
  (pToStruct)->m_nQueues =  (uint32)  ((pFromData)[7] >> 4) & 0x0f;
  (pToStruct)->m_nQueues |=  (uint32)  ((pFromData)[6] & 0x01) << 4;

  /* Unpack Member: m_nBaseQueue */
  (pToStruct)->m_nBaseQueue =  (uint32)  ((pFromData)[1] >> 6) & 0x03;
  (pToStruct)->m_nBaseQueue |=  (uint32)  (pFromData)[0] << 2;
  (pToStruct)->m_nBaseQueue |=  (uint32)  ((pFromData)[7] & 0x0f) << 10;

  /* Unpack Member: m_nLineRate */
  (pToStruct)->m_nLineRate =  (uint32)  (pFromData)[3] ;
  (pToStruct)->m_nLineRate |=  (uint32)  (pFromData)[2] << 8;
  (pToStruct)->m_nLineRate |=  (uint32)  ((pFromData)[1] & 0x3f) << 16;
#else

  /* Unpack operation based on little endian */

  /* Unpack Member: m_nSpQueues */
  (pToStruct)->m_nSpQueues =  (uint32)  ((pFromData)[5] >> 1) & 0x1f;

  /* Unpack Member: m_nQueues */
  (pToStruct)->m_nQueues =  (uint32)  ((pFromData)[4] >> 4) & 0x0f;
  (pToStruct)->m_nQueues |=  (uint32)  ((pFromData)[5] & 0x01) << 4;

  /* Unpack Member: m_nBaseQueue */
  (pToStruct)->m_nBaseQueue =  (uint32)  ((pFromData)[2] >> 6) & 0x03;
  (pToStruct)->m_nBaseQueue |=  (uint32)  (pFromData)[3] << 2;
  (pToStruct)->m_nBaseQueue |=  (uint32)  ((pFromData)[4] & 0x0f) << 10;

  /* Unpack Member: m_nLineRate */
  (pToStruct)->m_nLineRate =  (uint32)  (pFromData)[0] ;
  (pToStruct)->m_nLineRate |=  (uint32)  (pFromData)[1] << 8;
  (pToStruct)->m_nLineRate |=  (uint32)  ((pFromData)[2] & 0x3f) << 16;
#endif

}



/* initialize an instance of this zframe */
void
sbZfKaQmPortBwCfgTableEntry_InitInstance(sbZfKaQmPortBwCfgTableEntry_t *pFrame) {

  pFrame->m_nSpQueues =  (unsigned int)  0;
  pFrame->m_nQueues =  (unsigned int)  0;
  pFrame->m_nBaseQueue =  (unsigned int)  0;
  pFrame->m_nLineRate =  (unsigned int)  0;

}
