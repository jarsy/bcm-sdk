/*
 * $Id: sbZfKaRbClassSourceIdEntry.c,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


#include "sbTypes.h"
#include "sbZfKaRbClassSourceIdEntry.hx"
#include "sbWrappers.h"
#include <sal/types.h>



/* Pack from struct into array of bytes */
uint32
sbZfKaRbClassSourceIdEntry_Pack(sbZfKaRbClassSourceIdEntry_t *pFrom,
                                uint8 *pToData,
                                uint32 nMaxToDataIndex) {
#ifdef SAND_BIG_ENDIAN_HOST
  int i;
  int size = SB_ZF_ZFKARBCLASSSOURCEIDENTRY_SIZE_IN_BYTES;

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on bigword endian */

  /* Pack Member: m_nReserved2 */
  (pToData)[0] |= ((pFrom)->m_nReserved2 & 0x0f) <<4;

  /* Pack Member: m_nSrcIdOdd */
  (pToData)[1] |= ((pFrom)->m_nSrcIdOdd) & 0xFF;
  (pToData)[0] |= ((pFrom)->m_nSrcIdOdd >> 8) & 0x0f;

  /* Pack Member: m_nReserved1 */
  (pToData)[2] |= ((pFrom)->m_nReserved1 & 0x0f) <<4;

  /* Pack Member: m_nSrcIdEven */
  (pToData)[3] |= ((pFrom)->m_nSrcIdEven) & 0xFF;
  (pToData)[2] |= ((pFrom)->m_nSrcIdEven >> 8) & 0x0f;
#else
  int i;
  int size = SB_ZF_ZFKARBCLASSSOURCEIDENTRY_SIZE_IN_BYTES;

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on little endian */

  /* Pack Member: m_nReserved2 */
  (pToData)[3] |= ((pFrom)->m_nReserved2 & 0x0f) <<4;

  /* Pack Member: m_nSrcIdOdd */
  (pToData)[2] |= ((pFrom)->m_nSrcIdOdd) & 0xFF;
  (pToData)[3] |= ((pFrom)->m_nSrcIdOdd >> 8) & 0x0f;

  /* Pack Member: m_nReserved1 */
  (pToData)[1] |= ((pFrom)->m_nReserved1 & 0x0f) <<4;

  /* Pack Member: m_nSrcIdEven */
  (pToData)[0] |= ((pFrom)->m_nSrcIdEven) & 0xFF;
  (pToData)[1] |= ((pFrom)->m_nSrcIdEven >> 8) & 0x0f;
#endif

  return SB_ZF_ZFKARBCLASSSOURCEIDENTRY_SIZE_IN_BYTES;
}




/* Unpack from array of bytes into struct */
void
sbZfKaRbClassSourceIdEntry_Unpack(sbZfKaRbClassSourceIdEntry_t *pToStruct,
                                  uint8 *pFromData,
                                  uint32 nMaxToDataIndex) {
  COMPILER_UINT64 tmp;

  (void) tmp;

#ifdef SAND_BIG_ENDIAN_HOST

  /* Unpack operation based on bigword endian */

  /* Unpack Member: m_nReserved2 */
  (pToStruct)->m_nReserved2 =  (uint32)  ((pFromData)[0] >> 4) & 0x0f;

  /* Unpack Member: m_nSrcIdOdd */
  (pToStruct)->m_nSrcIdOdd =  (uint32)  (pFromData)[1] ;
  (pToStruct)->m_nSrcIdOdd |=  (uint32)  ((pFromData)[0] & 0x0f) << 8;

  /* Unpack Member: m_nReserved1 */
  (pToStruct)->m_nReserved1 =  (uint32)  ((pFromData)[2] >> 4) & 0x0f;

  /* Unpack Member: m_nSrcIdEven */
  (pToStruct)->m_nSrcIdEven =  (uint32)  (pFromData)[3] ;
  (pToStruct)->m_nSrcIdEven |=  (uint32)  ((pFromData)[2] & 0x0f) << 8;
#else

  /* Unpack operation based on little endian */

  /* Unpack Member: m_nReserved2 */
  (pToStruct)->m_nReserved2 =  (uint32)  ((pFromData)[3] >> 4) & 0x0f;

  /* Unpack Member: m_nSrcIdOdd */
  (pToStruct)->m_nSrcIdOdd =  (uint32)  (pFromData)[2] ;
  (pToStruct)->m_nSrcIdOdd |=  (uint32)  ((pFromData)[3] & 0x0f) << 8;

  /* Unpack Member: m_nReserved1 */
  (pToStruct)->m_nReserved1 =  (uint32)  ((pFromData)[1] >> 4) & 0x0f;

  /* Unpack Member: m_nSrcIdEven */
  (pToStruct)->m_nSrcIdEven =  (uint32)  (pFromData)[0] ;
  (pToStruct)->m_nSrcIdEven |=  (uint32)  ((pFromData)[1] & 0x0f) << 8;
#endif

}



/* initialize an instance of this zframe */
void
sbZfKaRbClassSourceIdEntry_InitInstance(sbZfKaRbClassSourceIdEntry_t *pFrame) {

  pFrame->m_nReserved2 =  (unsigned int)  0;
  pFrame->m_nSrcIdOdd =  (unsigned int)  0;
  pFrame->m_nReserved1 =  (unsigned int)  0;
  pFrame->m_nSrcIdEven =  (unsigned int)  0;

}
