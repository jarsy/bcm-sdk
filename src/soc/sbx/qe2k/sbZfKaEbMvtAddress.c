/*
 * $Id: sbZfKaEbMvtAddress.c,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


#include "sbTypes.h"
#include "sbZfKaEbMvtAddress.hx"
#include "sbWrappers.h"
#include <sal/types.h>



/* Pack from struct into array of bytes */
uint32
sbZfKaEbMvtAddress_Pack(sbZfKaEbMvtAddress_t *pFrom,
                        uint8 *pToData,
                        uint32 nMaxToDataIndex) {
#ifdef SAND_BIG_ENDIAN_HOST
  int i;
  int size = SB_ZF_ZFKAEBMVTADDRESS_SIZE_IN_BYTES;

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on bigword endian */

  /* Pack Member: m_nReserved */
  (pToData)[1] |= ((pFrom)->m_nReserved & 0x03) <<6;
  (pToData)[0] |= ((pFrom)->m_nReserved >> 2) &0xFF;

  /* Pack Member: m_nEgress */
  (pToData)[1] |= ((pFrom)->m_nEgress & 0x3f);

  /* Pack Member: m_nOffset */
  (pToData)[3] |= ((pFrom)->m_nOffset) & 0xFF;
  (pToData)[2] |= ((pFrom)->m_nOffset >> 8) &0xFF;
#else
  int i;
  int size = SB_ZF_ZFKAEBMVTADDRESS_SIZE_IN_BYTES;

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on little endian */

  /* Pack Member: m_nReserved */
  (pToData)[2] |= ((pFrom)->m_nReserved & 0x03) <<6;
  (pToData)[3] |= ((pFrom)->m_nReserved >> 2) &0xFF;

  /* Pack Member: m_nEgress */
  (pToData)[2] |= ((pFrom)->m_nEgress & 0x3f);

  /* Pack Member: m_nOffset */
  (pToData)[0] |= ((pFrom)->m_nOffset) & 0xFF;
  (pToData)[1] |= ((pFrom)->m_nOffset >> 8) &0xFF;
#endif

  return SB_ZF_ZFKAEBMVTADDRESS_SIZE_IN_BYTES;
}




/* Unpack from array of bytes into struct */
void
sbZfKaEbMvtAddress_Unpack(sbZfKaEbMvtAddress_t *pToStruct,
                          uint8 *pFromData,
                          uint32 nMaxToDataIndex) {
  COMPILER_UINT64 tmp;

  (void) tmp;

#ifdef SAND_BIG_ENDIAN_HOST

  /* Unpack operation based on bigword endian */

  /* Unpack Member: m_nReserved */
  (pToStruct)->m_nReserved =  (uint32)  ((pFromData)[1] >> 6) & 0x03;
  (pToStruct)->m_nReserved |=  (uint32)  (pFromData)[0] << 2;

  /* Unpack Member: m_nEgress */
  (pToStruct)->m_nEgress =  (uint32)  ((pFromData)[1] ) & 0x3f;

  /* Unpack Member: m_nOffset */
  (pToStruct)->m_nOffset =  (uint32)  (pFromData)[3] ;
  (pToStruct)->m_nOffset |=  (uint32)  (pFromData)[2] << 8;
#else

  /* Unpack operation based on little endian */

  /* Unpack Member: m_nReserved */
  (pToStruct)->m_nReserved =  (uint32)  ((pFromData)[2] >> 6) & 0x03;
  (pToStruct)->m_nReserved |=  (uint32)  (pFromData)[3] << 2;

  /* Unpack Member: m_nEgress */
  (pToStruct)->m_nEgress =  (uint32)  ((pFromData)[2] ) & 0x3f;

  /* Unpack Member: m_nOffset */
  (pToStruct)->m_nOffset =  (uint32)  (pFromData)[0] ;
  (pToStruct)->m_nOffset |=  (uint32)  (pFromData)[1] << 8;
#endif

}



/* initialize an instance of this zframe */
void
sbZfKaEbMvtAddress_InitInstance(sbZfKaEbMvtAddress_t *pFrame) {

  pFrame->m_nReserved =  (unsigned int)  0;
  pFrame->m_nEgress =  (unsigned int)  0;
  pFrame->m_nOffset =  (unsigned int)  0;

}
