/*
 * $Id: sbZfKaQsDepthHplenEntry.c,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


#include "sbTypes.h"
#include "sbZfKaQsDepthHplenEntry.hx"
#include "sbWrappers.h"
#include <sal/types.h>



/* Pack from struct into array of bytes */
uint32
sbZfKaQsDepthHplenEntry_Pack(sbZfKaQsDepthHplenEntry_t *pFrom,
                             uint8 *pToData,
                             uint32 nMaxToDataIndex) {
#ifdef SAND_BIG_ENDIAN_HOST
  int i;
  int size = SB_ZF_ZFKAQSDEPTHHPLENENTRY_SIZE_IN_BYTES;

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on bigword endian */

  /* Pack Member: m_nReserved */
  (pToData)[3] |= ((pFrom)->m_nReserved & 0x03) <<6;
  (pToData)[2] |= ((pFrom)->m_nReserved >> 2) &0xFF;
  (pToData)[1] |= ((pFrom)->m_nReserved >> 10) &0xFF;
  (pToData)[0] |= ((pFrom)->m_nReserved >> 18) &0xFF;

  /* Pack Member: m_nHplen */
  (pToData)[3] |= ((pFrom)->m_nHplen & 0x03) <<4;

  /* Pack Member: m_nDepth */
  (pToData)[3] |= ((pFrom)->m_nDepth & 0x0f);
#else
  int i;
  int size = SB_ZF_ZFKAQSDEPTHHPLENENTRY_SIZE_IN_BYTES;

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on little endian */

  /* Pack Member: m_nReserved */
  (pToData)[0] |= ((pFrom)->m_nReserved & 0x03) <<6;
  (pToData)[1] |= ((pFrom)->m_nReserved >> 2) &0xFF;
  (pToData)[2] |= ((pFrom)->m_nReserved >> 10) &0xFF;
  (pToData)[3] |= ((pFrom)->m_nReserved >> 18) &0xFF;

  /* Pack Member: m_nHplen */
  (pToData)[0] |= ((pFrom)->m_nHplen & 0x03) <<4;

  /* Pack Member: m_nDepth */
  (pToData)[0] |= ((pFrom)->m_nDepth & 0x0f);
#endif

  return SB_ZF_ZFKAQSDEPTHHPLENENTRY_SIZE_IN_BYTES;
}




/* Unpack from array of bytes into struct */
void
sbZfKaQsDepthHplenEntry_Unpack(sbZfKaQsDepthHplenEntry_t *pToStruct,
                               uint8 *pFromData,
                               uint32 nMaxToDataIndex) {
  COMPILER_UINT64 tmp;

  (void) tmp;

#ifdef SAND_BIG_ENDIAN_HOST

  /* Unpack operation based on bigword endian */

  /* Unpack Member: m_nReserved */
  (pToStruct)->m_nReserved =  (uint32)  ((pFromData)[3] >> 6) & 0x03;
  (pToStruct)->m_nReserved |=  (uint32)  (pFromData)[2] << 2;
  (pToStruct)->m_nReserved |=  (uint32)  (pFromData)[1] << 10;
  (pToStruct)->m_nReserved |=  (uint32)  (pFromData)[0] << 18;

  /* Unpack Member: m_nHplen */
  (pToStruct)->m_nHplen =  (uint32)  ((pFromData)[3] >> 4) & 0x03;

  /* Unpack Member: m_nDepth */
  (pToStruct)->m_nDepth =  (uint32)  ((pFromData)[3] ) & 0x0f;
#else

  /* Unpack operation based on little endian */

  /* Unpack Member: m_nReserved */
  (pToStruct)->m_nReserved =  (uint32)  ((pFromData)[0] >> 6) & 0x03;
  (pToStruct)->m_nReserved |=  (uint32)  (pFromData)[1] << 2;
  (pToStruct)->m_nReserved |=  (uint32)  (pFromData)[2] << 10;
  (pToStruct)->m_nReserved |=  (uint32)  (pFromData)[3] << 18;

  /* Unpack Member: m_nHplen */
  (pToStruct)->m_nHplen =  (uint32)  ((pFromData)[0] >> 4) & 0x03;

  /* Unpack Member: m_nDepth */
  (pToStruct)->m_nDepth =  (uint32)  ((pFromData)[0] ) & 0x0f;
#endif

}



/* initialize an instance of this zframe */
void
sbZfKaQsDepthHplenEntry_InitInstance(sbZfKaQsDepthHplenEntry_t *pFrame) {

  pFrame->m_nReserved =  (unsigned int)  0;
  pFrame->m_nHplen =  (unsigned int)  0;
  pFrame->m_nDepth =  (unsigned int)  0;

}
