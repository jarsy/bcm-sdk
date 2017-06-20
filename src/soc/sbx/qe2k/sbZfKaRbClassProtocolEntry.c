/*
 * $Id: sbZfKaRbClassProtocolEntry.c,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


#include "sbTypes.h"
#include "sbZfKaRbClassProtocolEntry.hx"
#include "sbWrappers.h"
#include <sal/types.h>



/* Pack from struct into array of bytes */
uint32
sbZfKaRbClassProtocolEntry_Pack(sbZfKaRbClassProtocolEntry_t *pFrom,
                                uint8 *pToData,
                                uint32 nMaxToDataIndex) {
#ifdef SAND_BIG_ENDIAN_HOST
  int i;
  int size = SB_ZF_ZFKARBCLASSPROTOCOLENTRY_SIZE_IN_BYTES;

  if (size % 4) {
    size += (4 - size %4);
  }

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on bigword endian */

  /* Pack Member: m_nProtocol1UseTos */
  (pToData)[2] |= ((pFrom)->m_nProtocol1UseTos & 0x01) <<7;

  /* Pack Member: m_nProtocol1UseSocketInHash */
  (pToData)[2] |= ((pFrom)->m_nProtocol1UseSocketInHash & 0x01) <<6;

  /* Pack Member: m_nProtocol1Dp */
  (pToData)[2] |= ((pFrom)->m_nProtocol1Dp & 0x03) <<4;

  /* Pack Member: m_nProtocol1Lsb */
  (pToData)[2] |= ((pFrom)->m_nProtocol1Lsb & 0x0f);

  /* Pack Member: m_nProtocol0UseTos */
  (pToData)[3] |= ((pFrom)->m_nProtocol0UseTos & 0x01) <<7;

  /* Pack Member: m_nProtocol0UseSocketInHash */
  (pToData)[3] |= ((pFrom)->m_nProtocol0UseSocketInHash & 0x01) <<6;

  /* Pack Member: m_nProtocol0Dp */
  (pToData)[3] |= ((pFrom)->m_nProtocol0Dp & 0x03) <<4;

  /* Pack Member: m_nProtocol0Lsb */
  (pToData)[3] |= ((pFrom)->m_nProtocol0Lsb & 0x0f);
#else
  int i;
  int size = SB_ZF_ZFKARBCLASSPROTOCOLENTRY_SIZE_IN_BYTES;

  if (size % 4) {
    size += (4 - size %4);
  }

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on little endian */

  /* Pack Member: m_nProtocol1UseTos */
  (pToData)[1] |= ((pFrom)->m_nProtocol1UseTos & 0x01) <<7;

  /* Pack Member: m_nProtocol1UseSocketInHash */
  (pToData)[1] |= ((pFrom)->m_nProtocol1UseSocketInHash & 0x01) <<6;

  /* Pack Member: m_nProtocol1Dp */
  (pToData)[1] |= ((pFrom)->m_nProtocol1Dp & 0x03) <<4;

  /* Pack Member: m_nProtocol1Lsb */
  (pToData)[1] |= ((pFrom)->m_nProtocol1Lsb & 0x0f);

  /* Pack Member: m_nProtocol0UseTos */
  (pToData)[0] |= ((pFrom)->m_nProtocol0UseTos & 0x01) <<7;

  /* Pack Member: m_nProtocol0UseSocketInHash */
  (pToData)[0] |= ((pFrom)->m_nProtocol0UseSocketInHash & 0x01) <<6;

  /* Pack Member: m_nProtocol0Dp */
  (pToData)[0] |= ((pFrom)->m_nProtocol0Dp & 0x03) <<4;

  /* Pack Member: m_nProtocol0Lsb */
  (pToData)[0] |= ((pFrom)->m_nProtocol0Lsb & 0x0f);
#endif

  return SB_ZF_ZFKARBCLASSPROTOCOLENTRY_SIZE_IN_BYTES;
}




/* Unpack from array of bytes into struct */
void
sbZfKaRbClassProtocolEntry_Unpack(sbZfKaRbClassProtocolEntry_t *pToStruct,
                                  uint8 *pFromData,
                                  uint32 nMaxToDataIndex) {
  COMPILER_UINT64 tmp;

  (void) tmp;

#ifdef SAND_BIG_ENDIAN_HOST

  /* Unpack operation based on bigword endian */

  /* Unpack Member: m_nProtocol1UseTos */
  (pToStruct)->m_nProtocol1UseTos =  (uint8)  ((pFromData)[2] >> 7) & 0x01;

  /* Unpack Member: m_nProtocol1UseSocketInHash */
  (pToStruct)->m_nProtocol1UseSocketInHash =  (uint8)  ((pFromData)[2] >> 6) & 0x01;

  /* Unpack Member: m_nProtocol1Dp */
  (pToStruct)->m_nProtocol1Dp =  (uint32)  ((pFromData)[2] >> 4) & 0x03;

  /* Unpack Member: m_nProtocol1Lsb */
  (pToStruct)->m_nProtocol1Lsb =  (uint32)  ((pFromData)[2] ) & 0x0f;

  /* Unpack Member: m_nProtocol0UseTos */
  (pToStruct)->m_nProtocol0UseTos =  (uint8)  ((pFromData)[3] >> 7) & 0x01;

  /* Unpack Member: m_nProtocol0UseSocketInHash */
  (pToStruct)->m_nProtocol0UseSocketInHash =  (uint8)  ((pFromData)[3] >> 6) & 0x01;

  /* Unpack Member: m_nProtocol0Dp */
  (pToStruct)->m_nProtocol0Dp =  (uint32)  ((pFromData)[3] >> 4) & 0x03;

  /* Unpack Member: m_nProtocol0Lsb */
  (pToStruct)->m_nProtocol0Lsb =  (uint32)  ((pFromData)[3] ) & 0x0f;
#else

  /* Unpack operation based on little endian */

  /* Unpack Member: m_nProtocol1UseTos */
  (pToStruct)->m_nProtocol1UseTos =  (uint8)  ((pFromData)[1] >> 7) & 0x01;

  /* Unpack Member: m_nProtocol1UseSocketInHash */
  (pToStruct)->m_nProtocol1UseSocketInHash =  (uint8)  ((pFromData)[1] >> 6) & 0x01;

  /* Unpack Member: m_nProtocol1Dp */
  (pToStruct)->m_nProtocol1Dp =  (uint32)  ((pFromData)[1] >> 4) & 0x03;

  /* Unpack Member: m_nProtocol1Lsb */
  (pToStruct)->m_nProtocol1Lsb =  (uint32)  ((pFromData)[1] ) & 0x0f;

  /* Unpack Member: m_nProtocol0UseTos */
  (pToStruct)->m_nProtocol0UseTos =  (uint8)  ((pFromData)[0] >> 7) & 0x01;

  /* Unpack Member: m_nProtocol0UseSocketInHash */
  (pToStruct)->m_nProtocol0UseSocketInHash =  (uint8)  ((pFromData)[0] >> 6) & 0x01;

  /* Unpack Member: m_nProtocol0Dp */
  (pToStruct)->m_nProtocol0Dp =  (uint32)  ((pFromData)[0] >> 4) & 0x03;

  /* Unpack Member: m_nProtocol0Lsb */
  (pToStruct)->m_nProtocol0Lsb =  (uint32)  ((pFromData)[0] ) & 0x0f;
#endif

}



/* initialize an instance of this zframe */
void
sbZfKaRbClassProtocolEntry_InitInstance(sbZfKaRbClassProtocolEntry_t *pFrame) {

  pFrame->m_nProtocol1UseTos =  (unsigned int)  0;
  pFrame->m_nProtocol1UseSocketInHash =  (unsigned int)  0;
  pFrame->m_nProtocol1Dp =  (unsigned int)  0;
  pFrame->m_nProtocol1Lsb =  (unsigned int)  0;
  pFrame->m_nProtocol0UseTos =  (unsigned int)  0;
  pFrame->m_nProtocol0UseSocketInHash =  (unsigned int)  0;
  pFrame->m_nProtocol0Dp =  (unsigned int)  0;
  pFrame->m_nProtocol0Lsb =  (unsigned int)  0;

}
