/*
 * $Id: sbZfKaEpIp32BitRewrite.c,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


#include "sbTypes.h"
#include "sbZfKaEpIp32BitRewrite.hx"
#include "sbWrappers.h"
#include <sal/types.h>



/* Pack from struct into array of bytes */
uint32
sbZfKaEpIp32BitRewrite_Pack(sbZfKaEpIp32BitRewrite_t *pFrom,
                            uint8 *pToData,
                            uint32 nMaxToDataIndex) {
#ifdef SAND_BIG_ENDIAN_HOST
  int i;
  int size = SB_ZF_ZFKAEPIP32BITREWRITE_SIZE_IN_BYTES;

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on bigword endian */

  /* Pack Member: m_nData */
  (pToData)[3] |= ((pFrom)->m_nData) & 0xFF;
  (pToData)[2] |= ((pFrom)->m_nData >> 8) &0xFF;
  (pToData)[1] |= ((pFrom)->m_nData >> 16) &0xFF;
  (pToData)[0] |= ((pFrom)->m_nData >> 24) &0xFF;
#else
  int i;
  int size = SB_ZF_ZFKAEPIP32BITREWRITE_SIZE_IN_BYTES;

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on little endian */

  /* Pack Member: m_nData */
  (pToData)[0] |= ((pFrom)->m_nData) & 0xFF;
  (pToData)[1] |= ((pFrom)->m_nData >> 8) &0xFF;
  (pToData)[2] |= ((pFrom)->m_nData >> 16) &0xFF;
  (pToData)[3] |= ((pFrom)->m_nData >> 24) &0xFF;
#endif

  return SB_ZF_ZFKAEPIP32BITREWRITE_SIZE_IN_BYTES;
}




/* Unpack from array of bytes into struct */
void
sbZfKaEpIp32BitRewrite_Unpack(sbZfKaEpIp32BitRewrite_t *pToStruct,
                              uint8 *pFromData,
                              uint32 nMaxToDataIndex) {
  COMPILER_UINT64 tmp;

  (void) tmp;

#ifdef SAND_BIG_ENDIAN_HOST

  /* Unpack operation based on bigword endian */

  /* Unpack Member: m_nData */
  (pToStruct)->m_nData =  (uint32)  (pFromData)[3] ;
  (pToStruct)->m_nData |=  (uint32)  (pFromData)[2] << 8;
  (pToStruct)->m_nData |=  (uint32)  (pFromData)[1] << 16;
  (pToStruct)->m_nData |=  (uint32)  (pFromData)[0] << 24;
#else

  /* Unpack operation based on little endian */

  /* Unpack Member: m_nData */
  (pToStruct)->m_nData =  (uint32)  (pFromData)[0] ;
  (pToStruct)->m_nData |=  (uint32)  (pFromData)[1] << 8;
  (pToStruct)->m_nData |=  (uint32)  (pFromData)[2] << 16;
  (pToStruct)->m_nData |=  (uint32)  (pFromData)[3] << 24;
#endif

}



/* initialize an instance of this zframe */
void
sbZfKaEpIp32BitRewrite_InitInstance(sbZfKaEpIp32BitRewrite_t *pFrame) {

  pFrame->m_nData =  (unsigned int)  0;

}
