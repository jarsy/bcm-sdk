/*
 * $Id: sbZfKaQsQ2EcEntry.c,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


#include "sbTypes.h"
#include "sbZfKaQsQ2EcEntry.hx"
#include "sbWrappers.h"
#include <sal/types.h>



/* Pack from struct into array of bytes */
uint32
sbZfKaQsQ2EcEntry_Pack(sbZfKaQsQ2EcEntry_t *pFrom,
                       uint8 *pToData,
                       uint32 nMaxToDataIndex) {
#ifdef SAND_BIG_ENDIAN_HOST
  int i;
  int size = SB_ZF_ZFKAQSQ2ECENTRY_SIZE_IN_BYTES;

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on bigword endian */

  /* Pack Member: m_nReserved */
  (pToData)[1] |= ((pFrom)->m_nReserved & 0x7f) <<1;
  (pToData)[0] |= ((pFrom)->m_nReserved >> 7) &0xFF;

  /* Pack Member: m_nMc */
  (pToData)[1] |= ((pFrom)->m_nMc & 0x01);

  /* Pack Member: m_nNode */
  (pToData)[2] |= ((pFrom)->m_nNode & 0x3f) <<2;

  /* Pack Member: m_nPort */
  (pToData)[3] |= ((pFrom)->m_nPort & 0x0f) <<4;
  (pToData)[2] |= ((pFrom)->m_nPort >> 4) & 0x03;

  /* Pack Member: m_nCos */
  (pToData)[3] |= ((pFrom)->m_nCos & 0x0f);
#else
  int i;
  int size = SB_ZF_ZFKAQSQ2ECENTRY_SIZE_IN_BYTES;

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on little endian */

  /* Pack Member: m_nReserved */
  (pToData)[2] |= ((pFrom)->m_nReserved & 0x7f) <<1;
  (pToData)[3] |= ((pFrom)->m_nReserved >> 7) &0xFF;

  /* Pack Member: m_nMc */
  (pToData)[2] |= ((pFrom)->m_nMc & 0x01);

  /* Pack Member: m_nNode */
  (pToData)[1] |= ((pFrom)->m_nNode & 0x3f) <<2;

  /* Pack Member: m_nPort */
  (pToData)[0] |= ((pFrom)->m_nPort & 0x0f) <<4;
  (pToData)[1] |= ((pFrom)->m_nPort >> 4) & 0x03;

  /* Pack Member: m_nCos */
  (pToData)[0] |= ((pFrom)->m_nCos & 0x0f);
#endif

  return SB_ZF_ZFKAQSQ2ECENTRY_SIZE_IN_BYTES;
}




/* Unpack from array of bytes into struct */
void
sbZfKaQsQ2EcEntry_Unpack(sbZfKaQsQ2EcEntry_t *pToStruct,
                         uint8 *pFromData,
                         uint32 nMaxToDataIndex) {
  COMPILER_UINT64 tmp;

  (void) tmp;

#ifdef SAND_BIG_ENDIAN_HOST

  /* Unpack operation based on bigword endian */

  /* Unpack Member: m_nReserved */
  (pToStruct)->m_nReserved =  (uint32)  ((pFromData)[1] >> 1) & 0x7f;
  (pToStruct)->m_nReserved |=  (uint32)  (pFromData)[0] << 7;

  /* Unpack Member: m_nMc */
  (pToStruct)->m_nMc =  (uint32)  ((pFromData)[1] ) & 0x01;

  /* Unpack Member: m_nNode */
  (pToStruct)->m_nNode =  (uint32)  ((pFromData)[2] >> 2) & 0x3f;

  /* Unpack Member: m_nPort */
  (pToStruct)->m_nPort =  (uint32)  ((pFromData)[3] >> 4) & 0x0f;
  (pToStruct)->m_nPort |=  (uint32)  ((pFromData)[2] & 0x03) << 4;

  /* Unpack Member: m_nCos */
  (pToStruct)->m_nCos =  (uint32)  ((pFromData)[3] ) & 0x0f;
#else

  /* Unpack operation based on little endian */

  /* Unpack Member: m_nReserved */
  (pToStruct)->m_nReserved =  (uint32)  ((pFromData)[2] >> 1) & 0x7f;
  (pToStruct)->m_nReserved |=  (uint32)  (pFromData)[3] << 7;

  /* Unpack Member: m_nMc */
  (pToStruct)->m_nMc =  (uint32)  ((pFromData)[2] ) & 0x01;

  /* Unpack Member: m_nNode */
  (pToStruct)->m_nNode =  (uint32)  ((pFromData)[1] >> 2) & 0x3f;

  /* Unpack Member: m_nPort */
  (pToStruct)->m_nPort =  (uint32)  ((pFromData)[0] >> 4) & 0x0f;
  (pToStruct)->m_nPort |=  (uint32)  ((pFromData)[1] & 0x03) << 4;

  /* Unpack Member: m_nCos */
  (pToStruct)->m_nCos =  (uint32)  ((pFromData)[0] ) & 0x0f;
#endif

}



/* initialize an instance of this zframe */
void
sbZfKaQsQ2EcEntry_InitInstance(sbZfKaQsQ2EcEntry_t *pFrame) {

  pFrame->m_nReserved =  (unsigned int)  0;
  pFrame->m_nMc =  (unsigned int)  0;
  pFrame->m_nNode =  (unsigned int)  0;
  pFrame->m_nPort =  (unsigned int)  0;
  pFrame->m_nCos =  (unsigned int)  0;

}
