/*
 * $Id: sbZfKaEpBfPriTableAddr.c,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


#include "sbTypes.h"
#include "sbZfKaEpBfPriTableAddr.hx"
#include "sbWrappers.h"
#include <sal/types.h>



/* Pack from struct into array of bytes */
uint32
sbZfKaEpBfPriTableAddr_Pack(sbZfKaEpBfPriTableAddr_t *pFrom,
                            uint8 *pToData,
                            uint32 nMaxToDataIndex) {
#ifdef SAND_BIG_ENDIAN_HOST
  int i;
  int size = SB_ZF_ZFKAEPBFPRITABLEADDR_SIZE_IN_BYTES;

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on bigword endian */

  /* Pack Member: m_nPort */
  (pToData)[3] |= ((pFrom)->m_nPort & 0x03) <<6;
  (pToData)[2] |= ((pFrom)->m_nPort >> 2) & 0x0f;

  /* Pack Member: m_nCos */
  (pToData)[3] |= ((pFrom)->m_nCos & 0x07) <<3;

  /* Pack Member: m_nDp */
  (pToData)[3] |= ((pFrom)->m_nDp & 0x03) <<1;

  /* Pack Member: m_nEcn */
  (pToData)[3] |= ((pFrom)->m_nEcn & 0x01);
#else
  int i;
  int size = SB_ZF_ZFKAEPBFPRITABLEADDR_SIZE_IN_BYTES;

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on little endian */

  /* Pack Member: m_nPort */
  (pToData)[0] |= ((pFrom)->m_nPort & 0x03) <<6;
  (pToData)[1] |= ((pFrom)->m_nPort >> 2) & 0x0f;

  /* Pack Member: m_nCos */
  (pToData)[0] |= ((pFrom)->m_nCos & 0x07) <<3;

  /* Pack Member: m_nDp */
  (pToData)[0] |= ((pFrom)->m_nDp & 0x03) <<1;

  /* Pack Member: m_nEcn */
  (pToData)[0] |= ((pFrom)->m_nEcn & 0x01);
#endif

  return SB_ZF_ZFKAEPBFPRITABLEADDR_SIZE_IN_BYTES;
}




/* Unpack from array of bytes into struct */
void
sbZfKaEpBfPriTableAddr_Unpack(sbZfKaEpBfPriTableAddr_t *pToStruct,
                              uint8 *pFromData,
                              uint32 nMaxToDataIndex) {
  COMPILER_UINT64 tmp;

  (void) tmp;

#ifdef SAND_BIG_ENDIAN_HOST

  /* Unpack operation based on bigword endian */

  /* Unpack Member: m_nPort */
  (pToStruct)->m_nPort =  (uint32)  ((pFromData)[3] >> 6) & 0x03;
  (pToStruct)->m_nPort |=  (uint32)  ((pFromData)[2] & 0x0f) << 2;

  /* Unpack Member: m_nCos */
  (pToStruct)->m_nCos =  (uint32)  ((pFromData)[3] >> 3) & 0x07;

  /* Unpack Member: m_nDp */
  (pToStruct)->m_nDp =  (uint32)  ((pFromData)[3] >> 1) & 0x03;

  /* Unpack Member: m_nEcn */
  (pToStruct)->m_nEcn =  (uint32)  ((pFromData)[3] ) & 0x01;
#else

  /* Unpack operation based on little endian */

  /* Unpack Member: m_nPort */
  (pToStruct)->m_nPort =  (uint32)  ((pFromData)[0] >> 6) & 0x03;
  (pToStruct)->m_nPort |=  (uint32)  ((pFromData)[1] & 0x0f) << 2;

  /* Unpack Member: m_nCos */
  (pToStruct)->m_nCos =  (uint32)  ((pFromData)[0] >> 3) & 0x07;

  /* Unpack Member: m_nDp */
  (pToStruct)->m_nDp =  (uint32)  ((pFromData)[0] >> 1) & 0x03;

  /* Unpack Member: m_nEcn */
  (pToStruct)->m_nEcn =  (uint32)  ((pFromData)[0] ) & 0x01;
#endif

}



/* initialize an instance of this zframe */
void
sbZfKaEpBfPriTableAddr_InitInstance(sbZfKaEpBfPriTableAddr_t *pFrame) {

  pFrame->m_nPort =  (unsigned int)  0;
  pFrame->m_nCos =  (unsigned int)  0;
  pFrame->m_nDp =  (unsigned int)  0;
  pFrame->m_nEcn =  (unsigned int)  0;

}
