/*
 * $Id: sbZfKaEgMemQCtlEntry.c,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


#include "sbTypes.h"
#include "sbZfKaEgMemQCtlEntry.hx"
#include "sbWrappers.h"
#include <sal/types.h>



/* Pack from struct into array of bytes */
uint32
sbZfKaEgMemQCtlEntry_Pack(sbZfKaEgMemQCtlEntry_t *pFrom,
                          uint8 *pToData,
                          uint32 nMaxToDataIndex) {
#ifdef SAND_BIG_ENDIAN_HOST
  int i;
  int size = SB_ZF_ZFKAEGMEMQCTLENTRY_SIZE_IN_BYTES;

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on bigword endian */

  /* Pack Member: m_nDropOnFull */
  (pToData)[0] |= ((pFrom)->m_nDropOnFull & 0x01) <<1;

  /* Pack Member: m_nWptr */
  (pToData)[1] |= ((pFrom)->m_nWptr & 0x1f) <<3;
  (pToData)[0] |= ((pFrom)->m_nWptr >> 5) & 0x01;

  /* Pack Member: m_nRptr */
  (pToData)[2] |= ((pFrom)->m_nRptr & 0x07) <<5;
  (pToData)[1] |= ((pFrom)->m_nRptr >> 3) & 0x07;

  /* Pack Member: m_nSize */
  (pToData)[2] |= ((pFrom)->m_nSize & 0x07) <<2;

  /* Pack Member: m_nBase */
  (pToData)[3] |= ((pFrom)->m_nBase) & 0xFF;
  (pToData)[2] |= ((pFrom)->m_nBase >> 8) & 0x03;
#else
  int i;
  int size = SB_ZF_ZFKAEGMEMQCTLENTRY_SIZE_IN_BYTES;

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on little endian */

  /* Pack Member: m_nDropOnFull */
  (pToData)[3] |= ((pFrom)->m_nDropOnFull & 0x01) <<1;

  /* Pack Member: m_nWptr */
  (pToData)[2] |= ((pFrom)->m_nWptr & 0x1f) <<3;
  (pToData)[3] |= ((pFrom)->m_nWptr >> 5) & 0x01;

  /* Pack Member: m_nRptr */
  (pToData)[1] |= ((pFrom)->m_nRptr & 0x07) <<5;
  (pToData)[2] |= ((pFrom)->m_nRptr >> 3) & 0x07;

  /* Pack Member: m_nSize */
  (pToData)[1] |= ((pFrom)->m_nSize & 0x07) <<2;

  /* Pack Member: m_nBase */
  (pToData)[0] |= ((pFrom)->m_nBase) & 0xFF;
  (pToData)[1] |= ((pFrom)->m_nBase >> 8) & 0x03;
#endif

  return SB_ZF_ZFKAEGMEMQCTLENTRY_SIZE_IN_BYTES;
}




/* Unpack from array of bytes into struct */
void
sbZfKaEgMemQCtlEntry_Unpack(sbZfKaEgMemQCtlEntry_t *pToStruct,
                            uint8 *pFromData,
                            uint32 nMaxToDataIndex) {
  COMPILER_UINT64 tmp;

  (void) tmp;

#ifdef SAND_BIG_ENDIAN_HOST

  /* Unpack operation based on bigword endian */

  /* Unpack Member: m_nDropOnFull */
  (pToStruct)->m_nDropOnFull =  (uint32)  ((pFromData)[0] >> 1) & 0x01;

  /* Unpack Member: m_nWptr */
  (pToStruct)->m_nWptr =  (uint32)  ((pFromData)[1] >> 3) & 0x1f;
  (pToStruct)->m_nWptr |=  (uint32)  ((pFromData)[0] & 0x01) << 5;

  /* Unpack Member: m_nRptr */
  (pToStruct)->m_nRptr =  (uint32)  ((pFromData)[2] >> 5) & 0x07;
  (pToStruct)->m_nRptr |=  (uint32)  ((pFromData)[1] & 0x07) << 3;

  /* Unpack Member: m_nSize */
  (pToStruct)->m_nSize =  (uint32)  ((pFromData)[2] >> 2) & 0x07;

  /* Unpack Member: m_nBase */
  (pToStruct)->m_nBase =  (uint32)  (pFromData)[3] ;
  (pToStruct)->m_nBase |=  (uint32)  ((pFromData)[2] & 0x03) << 8;
#else

  /* Unpack operation based on little endian */

  /* Unpack Member: m_nDropOnFull */
  (pToStruct)->m_nDropOnFull =  (uint32)  ((pFromData)[3] >> 1) & 0x01;

  /* Unpack Member: m_nWptr */
  (pToStruct)->m_nWptr =  (uint32)  ((pFromData)[2] >> 3) & 0x1f;
  (pToStruct)->m_nWptr |=  (uint32)  ((pFromData)[3] & 0x01) << 5;

  /* Unpack Member: m_nRptr */
  (pToStruct)->m_nRptr =  (uint32)  ((pFromData)[1] >> 5) & 0x07;
  (pToStruct)->m_nRptr |=  (uint32)  ((pFromData)[2] & 0x07) << 3;

  /* Unpack Member: m_nSize */
  (pToStruct)->m_nSize =  (uint32)  ((pFromData)[1] >> 2) & 0x07;

  /* Unpack Member: m_nBase */
  (pToStruct)->m_nBase =  (uint32)  (pFromData)[0] ;
  (pToStruct)->m_nBase |=  (uint32)  ((pFromData)[1] & 0x03) << 8;
#endif

}



/* initialize an instance of this zframe */
void
sbZfKaEgMemQCtlEntry_InitInstance(sbZfKaEgMemQCtlEntry_t *pFrame) {

  pFrame->m_nDropOnFull =  (unsigned int)  0;
  pFrame->m_nWptr =  (unsigned int)  0;
  pFrame->m_nRptr =  (unsigned int)  0;
  pFrame->m_nSize =  (unsigned int)  0;
  pFrame->m_nBase =  (unsigned int)  0;

}
