/*
 * $Id: sbZfKaEgPortRemapEntry.c,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


#include "sbTypes.h"
#include "sbZfKaEgPortRemapEntry.hx"
#include "sbWrappers.h"
#include <sal/types.h>



/* Pack from struct into array of bytes */
uint32
sbZfKaEgPortRemapEntry_Pack(sbZfKaEgPortRemapEntry_t *pFrom,
                            uint8 *pToData,
                            uint32 nMaxToDataIndex) {
#ifdef SAND_BIG_ENDIAN_HOST
  int i;
  int size = SB_ZF_ZFKAEGPORTREMAPENTRY_SIZE_IN_BYTES;

  if (size % 4) {
    size += (4 - size %4);
  }

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on bigword endian */

  /* Pack Member: m_nReserved */
  (pToData)[2] |= ((pFrom)->m_nReserved & 0x7f) <<1;

  /* Pack Member: m_nMcFifo */
  (pToData)[2] |= ((pFrom)->m_nMcFifo & 0x01);

  /* Pack Member: m_nFifoEnable */
  (pToData)[3] |= ((pFrom)->m_nFifoEnable & 0x01) <<7;

  /* Pack Member: m_nFifoNum */
  (pToData)[3] |= ((pFrom)->m_nFifoNum & 0x7f);
#else
  int i;
  int size = SB_ZF_ZFKAEGPORTREMAPENTRY_SIZE_IN_BYTES;

  if (size % 4) {
    size += (4 - size %4);
  }

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on little endian */

  /* Pack Member: m_nReserved */
  (pToData)[1] |= ((pFrom)->m_nReserved & 0x7f) <<1;

  /* Pack Member: m_nMcFifo */
  (pToData)[1] |= ((pFrom)->m_nMcFifo & 0x01);

  /* Pack Member: m_nFifoEnable */
  (pToData)[0] |= ((pFrom)->m_nFifoEnable & 0x01) <<7;

  /* Pack Member: m_nFifoNum */
  (pToData)[0] |= ((pFrom)->m_nFifoNum & 0x7f);
#endif

  return SB_ZF_ZFKAEGPORTREMAPENTRY_SIZE_IN_BYTES;
}




/* Unpack from array of bytes into struct */
void
sbZfKaEgPortRemapEntry_Unpack(sbZfKaEgPortRemapEntry_t *pToStruct,
                              uint8 *pFromData,
                              uint32 nMaxToDataIndex) {
  COMPILER_UINT64 tmp;

  (void) tmp;

#ifdef SAND_BIG_ENDIAN_HOST

  /* Unpack operation based on bigword endian */

  /* Unpack Member: m_nReserved */
  (pToStruct)->m_nReserved =  (uint32)  ((pFromData)[2] >> 1) & 0x7f;

  /* Unpack Member: m_nMcFifo */
  (pToStruct)->m_nMcFifo =  (uint8)  ((pFromData)[2] ) & 0x01;

  /* Unpack Member: m_nFifoEnable */
  (pToStruct)->m_nFifoEnable =  (uint8)  ((pFromData)[3] >> 7) & 0x01;

  /* Unpack Member: m_nFifoNum */
  (pToStruct)->m_nFifoNum =  (uint32)  ((pFromData)[3] ) & 0x7f;
#else

  /* Unpack operation based on little endian */

  /* Unpack Member: m_nReserved */
  (pToStruct)->m_nReserved =  (uint32)  ((pFromData)[1] >> 1) & 0x7f;

  /* Unpack Member: m_nMcFifo */
  (pToStruct)->m_nMcFifo =  (uint8)  ((pFromData)[1] ) & 0x01;

  /* Unpack Member: m_nFifoEnable */
  (pToStruct)->m_nFifoEnable =  (uint8)  ((pFromData)[0] >> 7) & 0x01;

  /* Unpack Member: m_nFifoNum */
  (pToStruct)->m_nFifoNum =  (uint32)  ((pFromData)[0] ) & 0x7f;
#endif

}



/* initialize an instance of this zframe */
void
sbZfKaEgPortRemapEntry_InitInstance(sbZfKaEgPortRemapEntry_t *pFrame) {

  pFrame->m_nReserved =  (unsigned int)  0;
  pFrame->m_nMcFifo =  (unsigned int)  0;
  pFrame->m_nFifoEnable =  (unsigned int)  0;
  pFrame->m_nFifoNum =  (unsigned int)  0;

}
