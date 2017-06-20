/*
 * $Id: sbZfKaEgNotTmePortRemapAddr.c,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


#include "sbTypes.h"
#include "sbZfKaEgNotTmePortRemapAddr.hx"
#include "sbWrappers.h"
#include <sal/types.h>



/* Pack from struct into array of bytes */
uint32
sbZfKaEgNotTmePortRemapAddr_Pack(sbZfKaEgNotTmePortRemapAddr_t *pFrom,
                                 uint8 *pToData,
                                 uint32 nMaxToDataIndex) {
#ifdef SAND_BIG_ENDIAN_HOST
  int i;
  int size = SB_ZF_ZFKAEGNOTTMEPORTREMAPADDR_SIZE_IN_BYTES;

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on bigword endian */

  /* Pack Member: m_nTxdma */
  (pToData)[2] |= ((pFrom)->m_nTxdma & 0x01);

  /* Pack Member: m_nEf */
  (pToData)[3] |= ((pFrom)->m_nEf & 0x01) <<7;

  /* Pack Member: m_nQe1k */
  (pToData)[3] |= ((pFrom)->m_nQe1k & 0x01) <<6;

  /* Pack Member: m_nPort */
  (pToData)[3] |= ((pFrom)->m_nPort & 0x3f);
#else
  int i;
  int size = SB_ZF_ZFKAEGNOTTMEPORTREMAPADDR_SIZE_IN_BYTES;

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on little endian */

  /* Pack Member: m_nTxdma */
  (pToData)[1] |= ((pFrom)->m_nTxdma & 0x01);

  /* Pack Member: m_nEf */
  (pToData)[0] |= ((pFrom)->m_nEf & 0x01) <<7;

  /* Pack Member: m_nQe1k */
  (pToData)[0] |= ((pFrom)->m_nQe1k & 0x01) <<6;

  /* Pack Member: m_nPort */
  (pToData)[0] |= ((pFrom)->m_nPort & 0x3f);
#endif

  return SB_ZF_ZFKAEGNOTTMEPORTREMAPADDR_SIZE_IN_BYTES;
}




/* Unpack from array of bytes into struct */
void
sbZfKaEgNotTmePortRemapAddr_Unpack(sbZfKaEgNotTmePortRemapAddr_t *pToStruct,
                                   uint8 *pFromData,
                                   uint32 nMaxToDataIndex) {
  COMPILER_UINT64 tmp;

  (void) tmp;

#ifdef SAND_BIG_ENDIAN_HOST

  /* Unpack operation based on bigword endian */

  /* Unpack Member: m_nTxdma */
  (pToStruct)->m_nTxdma =  (uint8)  ((pFromData)[2] ) & 0x01;

  /* Unpack Member: m_nEf */
  (pToStruct)->m_nEf =  (uint8)  ((pFromData)[3] >> 7) & 0x01;

  /* Unpack Member: m_nQe1k */
  (pToStruct)->m_nQe1k =  (uint8)  ((pFromData)[3] >> 6) & 0x01;

  /* Unpack Member: m_nPort */
  (pToStruct)->m_nPort =  (uint32)  ((pFromData)[3] ) & 0x3f;
#else

  /* Unpack operation based on little endian */

  /* Unpack Member: m_nTxdma */
  (pToStruct)->m_nTxdma =  (uint8)  ((pFromData)[1] ) & 0x01;

  /* Unpack Member: m_nEf */
  (pToStruct)->m_nEf =  (uint8)  ((pFromData)[0] >> 7) & 0x01;

  /* Unpack Member: m_nQe1k */
  (pToStruct)->m_nQe1k =  (uint8)  ((pFromData)[0] >> 6) & 0x01;

  /* Unpack Member: m_nPort */
  (pToStruct)->m_nPort =  (uint32)  ((pFromData)[0] ) & 0x3f;
#endif

}



/* initialize an instance of this zframe */
void
sbZfKaEgNotTmePortRemapAddr_InitInstance(sbZfKaEgNotTmePortRemapAddr_t *pFrame) {

  pFrame->m_nTxdma =  (unsigned int)  0;
  pFrame->m_nEf =  (unsigned int)  0;
  pFrame->m_nQe1k =  (unsigned int)  0;
  pFrame->m_nPort =  (unsigned int)  0;

}
