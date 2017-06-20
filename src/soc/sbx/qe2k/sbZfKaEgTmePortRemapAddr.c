/*
 * $Id: sbZfKaEgTmePortRemapAddr.c,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


#include "sbTypes.h"
#include "sbZfKaEgTmePortRemapAddr.hx"
#include "sbWrappers.h"
#include <sal/types.h>



/* Pack from struct into array of bytes */
uint32
sbZfKaEgTmePortRemapAddr_Pack(sbZfKaEgTmePortRemapAddr_t *pFrom,
                              uint8 *pToData,
                              uint32 nMaxToDataIndex) {
#ifdef SAND_BIG_ENDIAN_HOST
  int i;
  int size = SB_ZF_ZFKAEGTMEPORTREMAPADDR_SIZE_IN_BYTES;

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on bigword endian */

  /* Pack Member: m_nNode */
  (pToData)[3] |= ((pFrom)->m_nNode & 0x07) <<5;
  (pToData)[2] |= ((pFrom)->m_nNode >> 3) & 0x03;

  /* Pack Member: m_nPort */
  (pToData)[3] |= ((pFrom)->m_nPort & 0x1f);
#else
  int i;
  int size = SB_ZF_ZFKAEGTMEPORTREMAPADDR_SIZE_IN_BYTES;

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on little endian */

  /* Pack Member: m_nNode */
  (pToData)[0] |= ((pFrom)->m_nNode & 0x07) <<5;
  (pToData)[1] |= ((pFrom)->m_nNode >> 3) & 0x03;

  /* Pack Member: m_nPort */
  (pToData)[0] |= ((pFrom)->m_nPort & 0x1f);
#endif

  return SB_ZF_ZFKAEGTMEPORTREMAPADDR_SIZE_IN_BYTES;
}




/* Unpack from array of bytes into struct */
void
sbZfKaEgTmePortRemapAddr_Unpack(sbZfKaEgTmePortRemapAddr_t *pToStruct,
                                uint8 *pFromData,
                                uint32 nMaxToDataIndex) {
  COMPILER_UINT64 tmp;

  (void) tmp;

#ifdef SAND_BIG_ENDIAN_HOST

  /* Unpack operation based on bigword endian */

  /* Unpack Member: m_nNode */
  (pToStruct)->m_nNode =  (uint32)  ((pFromData)[3] >> 5) & 0x07;
  (pToStruct)->m_nNode |=  (uint32)  ((pFromData)[2] & 0x03) << 3;

  /* Unpack Member: m_nPort */
  (pToStruct)->m_nPort =  (uint32)  ((pFromData)[3] ) & 0x1f;
#else

  /* Unpack operation based on little endian */

  /* Unpack Member: m_nNode */
  (pToStruct)->m_nNode =  (uint32)  ((pFromData)[0] >> 5) & 0x07;
  (pToStruct)->m_nNode |=  (uint32)  ((pFromData)[1] & 0x03) << 3;

  /* Unpack Member: m_nPort */
  (pToStruct)->m_nPort =  (uint32)  ((pFromData)[0] ) & 0x1f;
#endif

}



/* initialize an instance of this zframe */
void
sbZfKaEgTmePortRemapAddr_InitInstance(sbZfKaEgTmePortRemapAddr_t *pFrame) {

  pFrame->m_nNode =  (unsigned int)  0;
  pFrame->m_nPort =  (unsigned int)  0;

}
