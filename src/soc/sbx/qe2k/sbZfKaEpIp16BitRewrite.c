/*
 * $Id: sbZfKaEpIp16BitRewrite.c,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


#include "sbTypes.h"
#include "sbZfKaEpIp16BitRewrite.hx"
#include "sbWrappers.h"
#include <sal/types.h>



/* Pack from struct into array of bytes */
uint32
sbZfKaEpIp16BitRewrite_Pack(sbZfKaEpIp16BitRewrite_t *pFrom,
                            uint8 *pToData,
                            uint32 nMaxToDataIndex) {
#ifdef SAND_BIG_ENDIAN_HOST
  int i;
  int size = SB_ZF_ZFKAEPIP16BITREWRITE_SIZE_IN_BYTES;

  if (size % 4) {
    size += (4 - size %4);
  }

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on bigword endian */

  /* Pack Member: m_nData */
  (pToData)[3] |= ((pFrom)->m_nData) & 0xFF;
  (pToData)[2] |= ((pFrom)->m_nData >> 8) &0xFF;
#else
  int i;
  int size = SB_ZF_ZFKAEPIP16BITREWRITE_SIZE_IN_BYTES;

  if (size % 4) {
    size += (4 - size %4);
  }

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on little endian */

  /* Pack Member: m_nData */
  (pToData)[0] |= ((pFrom)->m_nData) & 0xFF;
  (pToData)[1] |= ((pFrom)->m_nData >> 8) &0xFF;
#endif

  return SB_ZF_ZFKAEPIP16BITREWRITE_SIZE_IN_BYTES;
}




/* Unpack from array of bytes into struct */
void
sbZfKaEpIp16BitRewrite_Unpack(sbZfKaEpIp16BitRewrite_t *pToStruct,
                              uint8 *pFromData,
                              uint32 nMaxToDataIndex) {
  COMPILER_UINT64 tmp;

  (void) tmp;

#ifdef SAND_BIG_ENDIAN_HOST

  /* Unpack operation based on bigword endian */

  /* Unpack Member: m_nData */
  (pToStruct)->m_nData =  (uint32)  (pFromData)[3] ;
  (pToStruct)->m_nData |=  (uint32)  (pFromData)[2] << 8;
#else

  /* Unpack operation based on little endian */

  /* Unpack Member: m_nData */
  (pToStruct)->m_nData =  (uint32)  (pFromData)[0] ;
  (pToStruct)->m_nData |=  (uint32)  (pFromData)[1] << 8;
#endif

}



/* initialize an instance of this zframe */
void
sbZfKaEpIp16BitRewrite_InitInstance(sbZfKaEpIp16BitRewrite_t *pFrame) {

  pFrame->m_nData =  (unsigned int)  0;

}
