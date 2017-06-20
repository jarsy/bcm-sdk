/*
 * $Id: sbZfKaQsPriLutAddr.c,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


#include "sbTypes.h"
#include "sbZfKaQsPriLutAddr.hx"
#include "sbWrappers.h"
#include <sal/types.h>



/* Pack from struct into array of bytes */
uint32
sbZfKaQsPriLutAddr_Pack(sbZfKaQsPriLutAddr_t *pFrom,
                        uint8 *pToData,
                        uint32 nMaxToDataIndex) {
#ifdef SAND_BIG_ENDIAN_HOST
  int i;
  int size = SB_ZF_ZFKAQSPRILUTADDR_SIZE_IN_BYTES;

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on bigword endian */

  /* Pack Member: m_nReserved */
  (pToData)[2] |= ((pFrom)->m_nReserved & 0x07) <<5;
  (pToData)[1] |= ((pFrom)->m_nReserved >> 3) &0xFF;
  (pToData)[0] |= ((pFrom)->m_nReserved >> 11) &0xFF;

  /* Pack Member: m_nShaped */
  (pToData)[2] |= ((pFrom)->m_nShaped & 0x01) <<4;

  /* Pack Member: m_nDepth */
  (pToData)[2] |= ((pFrom)->m_nDepth & 0x07) <<1;

  /* Pack Member: m_nAnemicAged */
  (pToData)[2] |= ((pFrom)->m_nAnemicAged & 0x01);

  /* Pack Member: m_nQType */
  (pToData)[3] |= ((pFrom)->m_nQType & 0x0f) <<4;

  /* Pack Member: m_nEfAged */
  (pToData)[3] |= ((pFrom)->m_nEfAged & 0x01) <<3;

  /* Pack Member: m_nCreditLevel */
  (pToData)[3] |= ((pFrom)->m_nCreditLevel & 0x01) <<2;

  /* Pack Member: m_nHoldTs */
  (pToData)[3] |= ((pFrom)->m_nHoldTs & 0x01) <<1;

  /* Pack Member: m_nPktLen */
  (pToData)[3] |= ((pFrom)->m_nPktLen & 0x01);
#else
  int i;
  int size = SB_ZF_ZFKAQSPRILUTADDR_SIZE_IN_BYTES;

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on little endian */

  /* Pack Member: m_nReserved */
  (pToData)[1] |= ((pFrom)->m_nReserved & 0x07) <<5;
  (pToData)[2] |= ((pFrom)->m_nReserved >> 3) &0xFF;
  (pToData)[3] |= ((pFrom)->m_nReserved >> 11) &0xFF;

  /* Pack Member: m_nShaped */
  (pToData)[1] |= ((pFrom)->m_nShaped & 0x01) <<4;

  /* Pack Member: m_nDepth */
  (pToData)[1] |= ((pFrom)->m_nDepth & 0x07) <<1;

  /* Pack Member: m_nAnemicAged */
  (pToData)[1] |= ((pFrom)->m_nAnemicAged & 0x01);

  /* Pack Member: m_nQType */
  (pToData)[0] |= ((pFrom)->m_nQType & 0x0f) <<4;

  /* Pack Member: m_nEfAged */
  (pToData)[0] |= ((pFrom)->m_nEfAged & 0x01) <<3;

  /* Pack Member: m_nCreditLevel */
  (pToData)[0] |= ((pFrom)->m_nCreditLevel & 0x01) <<2;

  /* Pack Member: m_nHoldTs */
  (pToData)[0] |= ((pFrom)->m_nHoldTs & 0x01) <<1;

  /* Pack Member: m_nPktLen */
  (pToData)[0] |= ((pFrom)->m_nPktLen & 0x01);
#endif

  return SB_ZF_ZFKAQSPRILUTADDR_SIZE_IN_BYTES;
}




/* Unpack from array of bytes into struct */
void
sbZfKaQsPriLutAddr_Unpack(sbZfKaQsPriLutAddr_t *pToStruct,
                          uint8 *pFromData,
                          uint32 nMaxToDataIndex) {
  COMPILER_UINT64 tmp;

  (void) tmp;

#ifdef SAND_BIG_ENDIAN_HOST

  /* Unpack operation based on bigword endian */

  /* Unpack Member: m_nReserved */
  (pToStruct)->m_nReserved =  (uint32)  ((pFromData)[2] >> 5) & 0x07;
  (pToStruct)->m_nReserved |=  (uint32)  (pFromData)[1] << 3;
  (pToStruct)->m_nReserved |=  (uint32)  (pFromData)[0] << 11;

  /* Unpack Member: m_nShaped */
  (pToStruct)->m_nShaped =  (uint8)  ((pFromData)[2] >> 4) & 0x01;

  /* Unpack Member: m_nDepth */
  (pToStruct)->m_nDepth =  (uint32)  ((pFromData)[2] >> 1) & 0x07;

  /* Unpack Member: m_nAnemicAged */
  (pToStruct)->m_nAnemicAged =  (uint8)  ((pFromData)[2] ) & 0x01;

  /* Unpack Member: m_nQType */
  (pToStruct)->m_nQType =  (uint32)  ((pFromData)[3] >> 4) & 0x0f;

  /* Unpack Member: m_nEfAged */
  (pToStruct)->m_nEfAged =  (uint8)  ((pFromData)[3] >> 3) & 0x01;

  /* Unpack Member: m_nCreditLevel */
  (pToStruct)->m_nCreditLevel =  (uint32)  ((pFromData)[3] >> 2) & 0x01;

  /* Unpack Member: m_nHoldTs */
  (pToStruct)->m_nHoldTs =  (uint8)  ((pFromData)[3] >> 1) & 0x01;

  /* Unpack Member: m_nPktLen */
  (pToStruct)->m_nPktLen =  (uint32)  ((pFromData)[3] ) & 0x01;
#else

  /* Unpack operation based on little endian */

  /* Unpack Member: m_nReserved */
  (pToStruct)->m_nReserved =  (uint32)  ((pFromData)[1] >> 5) & 0x07;
  (pToStruct)->m_nReserved |=  (uint32)  (pFromData)[2] << 3;
  (pToStruct)->m_nReserved |=  (uint32)  (pFromData)[3] << 11;

  /* Unpack Member: m_nShaped */
  (pToStruct)->m_nShaped =  (uint8)  ((pFromData)[1] >> 4) & 0x01;

  /* Unpack Member: m_nDepth */
  (pToStruct)->m_nDepth =  (uint32)  ((pFromData)[1] >> 1) & 0x07;

  /* Unpack Member: m_nAnemicAged */
  (pToStruct)->m_nAnemicAged =  (uint8)  ((pFromData)[1] ) & 0x01;

  /* Unpack Member: m_nQType */
  (pToStruct)->m_nQType =  (uint32)  ((pFromData)[0] >> 4) & 0x0f;

  /* Unpack Member: m_nEfAged */
  (pToStruct)->m_nEfAged =  (uint8)  ((pFromData)[0] >> 3) & 0x01;

  /* Unpack Member: m_nCreditLevel */
  (pToStruct)->m_nCreditLevel =  (uint32)  ((pFromData)[0] >> 2) & 0x01;

  /* Unpack Member: m_nHoldTs */
  (pToStruct)->m_nHoldTs =  (uint8)  ((pFromData)[0] >> 1) & 0x01;

  /* Unpack Member: m_nPktLen */
  (pToStruct)->m_nPktLen =  (uint32)  ((pFromData)[0] ) & 0x01;
#endif

}



/* initialize an instance of this zframe */
void
sbZfKaQsPriLutAddr_InitInstance(sbZfKaQsPriLutAddr_t *pFrame) {

  pFrame->m_nReserved =  (unsigned int)  0;
  pFrame->m_nShaped =  (unsigned int)  0;
  pFrame->m_nDepth =  (unsigned int)  0;
  pFrame->m_nAnemicAged =  (unsigned int)  0;
  pFrame->m_nQType =  (unsigned int)  0;
  pFrame->m_nEfAged =  (unsigned int)  0;
  pFrame->m_nCreditLevel =  (unsigned int)  0;
  pFrame->m_nHoldTs =  (unsigned int)  0;
  pFrame->m_nPktLen =  (unsigned int)  0;

}
