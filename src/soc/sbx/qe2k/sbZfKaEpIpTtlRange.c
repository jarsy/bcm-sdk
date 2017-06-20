/*
 * $Id: sbZfKaEpIpTtlRange.c,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


#include "sbTypes.h"
#include "sbZfKaEpIpTtlRange.hx"
#include "sbWrappers.h"
#include <sal/types.h>



/* Pack from struct into array of bytes */
uint32
sbZfKaEpIpTtlRange_Pack(sbZfKaEpIpTtlRange_t *pFrom,
                        uint8 *pToData,
                        uint32 nMaxToDataIndex) {
#ifdef SAND_BIG_ENDIAN_HOST
  int i;
  int size = SB_ZF_ZFKAEPIPTTLRANGE_SIZE_IN_BYTES;

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on bigword endian */

  /* Pack Member: m_nHighTtl */
  (pToData)[7] |= ((pFrom)->m_nHighTtl) & 0xFF;
  (pToData)[6] |= ((pFrom)->m_nHighTtl >> 8) &0xFF;
  (pToData)[5] |= ((pFrom)->m_nHighTtl >> 16) &0xFF;
  (pToData)[4] |= ((pFrom)->m_nHighTtl >> 24) &0xFF;

  /* Pack Member: m_nLowTtl */
  (pToData)[3] |= ((pFrom)->m_nLowTtl) & 0xFF;
  (pToData)[2] |= ((pFrom)->m_nLowTtl >> 8) &0xFF;
  (pToData)[1] |= ((pFrom)->m_nLowTtl >> 16) &0xFF;
  (pToData)[0] |= ((pFrom)->m_nLowTtl >> 24) &0xFF;
#else
  int i;
  int size = SB_ZF_ZFKAEPIPTTLRANGE_SIZE_IN_BYTES;

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on little endian */

  /* Pack Member: m_nHighTtl */
  (pToData)[4] |= ((pFrom)->m_nHighTtl) & 0xFF;
  (pToData)[5] |= ((pFrom)->m_nHighTtl >> 8) &0xFF;
  (pToData)[6] |= ((pFrom)->m_nHighTtl >> 16) &0xFF;
  (pToData)[7] |= ((pFrom)->m_nHighTtl >> 24) &0xFF;

  /* Pack Member: m_nLowTtl */
  (pToData)[0] |= ((pFrom)->m_nLowTtl) & 0xFF;
  (pToData)[1] |= ((pFrom)->m_nLowTtl >> 8) &0xFF;
  (pToData)[2] |= ((pFrom)->m_nLowTtl >> 16) &0xFF;
  (pToData)[3] |= ((pFrom)->m_nLowTtl >> 24) &0xFF;
#endif

  return SB_ZF_ZFKAEPIPTTLRANGE_SIZE_IN_BYTES;
}




/* Unpack from array of bytes into struct */
void
sbZfKaEpIpTtlRange_Unpack(sbZfKaEpIpTtlRange_t *pToStruct,
                          uint8 *pFromData,
                          uint32 nMaxToDataIndex) {
  COMPILER_UINT64 tmp;

  (void) tmp;

#ifdef SAND_BIG_ENDIAN_HOST

  /* Unpack operation based on bigword endian */

  /* Unpack Member: m_nHighTtl */
  (pToStruct)->m_nHighTtl =  (uint32)  (pFromData)[7] ;
  (pToStruct)->m_nHighTtl |=  (uint32)  (pFromData)[6] << 8;
  (pToStruct)->m_nHighTtl |=  (uint32)  (pFromData)[5] << 16;
  (pToStruct)->m_nHighTtl |=  (uint32)  (pFromData)[4] << 24;

  /* Unpack Member: m_nLowTtl */
  (pToStruct)->m_nLowTtl =  (uint32)  (pFromData)[3] ;
  (pToStruct)->m_nLowTtl |=  (uint32)  (pFromData)[2] << 8;
  (pToStruct)->m_nLowTtl |=  (uint32)  (pFromData)[1] << 16;
  (pToStruct)->m_nLowTtl |=  (uint32)  (pFromData)[0] << 24;
#else

  /* Unpack operation based on little endian */

  /* Unpack Member: m_nHighTtl */
  (pToStruct)->m_nHighTtl =  (uint32)  (pFromData)[4] ;
  (pToStruct)->m_nHighTtl |=  (uint32)  (pFromData)[5] << 8;
  (pToStruct)->m_nHighTtl |=  (uint32)  (pFromData)[6] << 16;
  (pToStruct)->m_nHighTtl |=  (uint32)  (pFromData)[7] << 24;

  /* Unpack Member: m_nLowTtl */
  (pToStruct)->m_nLowTtl =  (uint32)  (pFromData)[0] ;
  (pToStruct)->m_nLowTtl |=  (uint32)  (pFromData)[1] << 8;
  (pToStruct)->m_nLowTtl |=  (uint32)  (pFromData)[2] << 16;
  (pToStruct)->m_nLowTtl |=  (uint32)  (pFromData)[3] << 24;
#endif

}



/* initialize an instance of this zframe */
void
sbZfKaEpIpTtlRange_InitInstance(sbZfKaEpIpTtlRange_t *pFrame) {

  pFrame->m_nHighTtl =  (unsigned int)  0;
  pFrame->m_nLowTtl =  (unsigned int)  0;

}
