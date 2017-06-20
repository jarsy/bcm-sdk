/*
 * $Id: sbZfKaRbClassDefaultQEntry.c,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


#include "sbTypes.h"
#include "sbZfKaRbClassDefaultQEntry.hx"
#include "sbWrappers.h"
#include <sal/types.h>



/* Pack from struct into array of bytes */
uint32
sbZfKaRbClassDefaultQEntry_Pack(sbZfKaRbClassDefaultQEntry_t *pFrom,
                                uint8 *pToData,
                                uint32 nMaxToDataIndex) {
#ifdef SAND_BIG_ENDIAN_HOST
  int i;
  int size = SB_ZF_ZFKARBCLASSDEFAULTQENTRY_SIZE_IN_BYTES;

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on bigword endian */

  /* Pack Member: m_nSpare1 */
  (pToData)[1] |= ((pFrom)->m_nSpare1 & 0x3f) <<2;
  (pToData)[0] |= ((pFrom)->m_nSpare1 >> 6) &0xFF;

  /* Pack Member: m_nDefaultDp */
  (pToData)[1] |= ((pFrom)->m_nDefaultDp & 0x03);

  /* Pack Member: m_nSpare0 */
  (pToData)[2] |= ((pFrom)->m_nSpare0 & 0x03) <<6;

  /* Pack Member: m_nDefaultQ */
  (pToData)[3] |= ((pFrom)->m_nDefaultQ) & 0xFF;
  (pToData)[2] |= ((pFrom)->m_nDefaultQ >> 8) & 0x3f;
#else
  int i;
  int size = SB_ZF_ZFKARBCLASSDEFAULTQENTRY_SIZE_IN_BYTES;

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on little endian */

  /* Pack Member: m_nSpare1 */
  (pToData)[2] |= ((pFrom)->m_nSpare1 & 0x3f) <<2;
  (pToData)[3] |= ((pFrom)->m_nSpare1 >> 6) &0xFF;

  /* Pack Member: m_nDefaultDp */
  (pToData)[2] |= ((pFrom)->m_nDefaultDp & 0x03);

  /* Pack Member: m_nSpare0 */
  (pToData)[1] |= ((pFrom)->m_nSpare0 & 0x03) <<6;

  /* Pack Member: m_nDefaultQ */
  (pToData)[0] |= ((pFrom)->m_nDefaultQ) & 0xFF;
  (pToData)[1] |= ((pFrom)->m_nDefaultQ >> 8) & 0x3f;
#endif

  return SB_ZF_ZFKARBCLASSDEFAULTQENTRY_SIZE_IN_BYTES;
}




/* Unpack from array of bytes into struct */
void
sbZfKaRbClassDefaultQEntry_Unpack(sbZfKaRbClassDefaultQEntry_t *pToStruct,
                                  uint8 *pFromData,
                                  uint32 nMaxToDataIndex) {
  COMPILER_UINT64 tmp;

  (void) tmp;

#ifdef SAND_BIG_ENDIAN_HOST

  /* Unpack operation based on bigword endian */

  /* Unpack Member: m_nSpare1 */
  (pToStruct)->m_nSpare1 =  (uint32)  ((pFromData)[1] >> 2) & 0x3f;
  (pToStruct)->m_nSpare1 |=  (uint32)  (pFromData)[0] << 6;

  /* Unpack Member: m_nDefaultDp */
  (pToStruct)->m_nDefaultDp =  (uint32)  ((pFromData)[1] ) & 0x03;

  /* Unpack Member: m_nSpare0 */
  (pToStruct)->m_nSpare0 =  (uint32)  ((pFromData)[2] >> 6) & 0x03;

  /* Unpack Member: m_nDefaultQ */
  (pToStruct)->m_nDefaultQ =  (uint32)  (pFromData)[3] ;
  (pToStruct)->m_nDefaultQ |=  (uint32)  ((pFromData)[2] & 0x3f) << 8;
#else

  /* Unpack operation based on little endian */

  /* Unpack Member: m_nSpare1 */
  (pToStruct)->m_nSpare1 =  (uint32)  ((pFromData)[2] >> 2) & 0x3f;
  (pToStruct)->m_nSpare1 |=  (uint32)  (pFromData)[3] << 6;

  /* Unpack Member: m_nDefaultDp */
  (pToStruct)->m_nDefaultDp =  (uint32)  ((pFromData)[2] ) & 0x03;

  /* Unpack Member: m_nSpare0 */
  (pToStruct)->m_nSpare0 =  (uint32)  ((pFromData)[1] >> 6) & 0x03;

  /* Unpack Member: m_nDefaultQ */
  (pToStruct)->m_nDefaultQ =  (uint32)  (pFromData)[0] ;
  (pToStruct)->m_nDefaultQ |=  (uint32)  ((pFromData)[1] & 0x3f) << 8;
#endif

}



/* initialize an instance of this zframe */
void
sbZfKaRbClassDefaultQEntry_InitInstance(sbZfKaRbClassDefaultQEntry_t *pFrame) {

  pFrame->m_nSpare1 =  (unsigned int)  0;
  pFrame->m_nDefaultDp =  (unsigned int)  0;
  pFrame->m_nSpare0 =  (unsigned int)  0;
  pFrame->m_nDefaultQ =  (unsigned int)  0;

}
