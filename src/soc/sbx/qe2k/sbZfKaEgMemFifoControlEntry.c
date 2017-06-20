/*
 * $Id: sbZfKaEgMemFifoControlEntry.c,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


#include "sbTypes.h"
#include "sbZfKaEgMemFifoControlEntry.hx"
#include "sbWrappers.h"
#include <sal/types.h>



/* Pack from struct into array of bytes */
uint32
sbZfKaEgMemFifoControlEntry_Pack(sbZfKaEgMemFifoControlEntry_t *pFrom,
                                 uint8 *pToData,
                                 uint32 nMaxToDataIndex) {
#ifdef SAND_BIG_ENDIAN_HOST
  int i;
  int size = SB_ZF_ZFKAEGMEMFIFOCONTRILENTRY_SIZE_IN_BYTES;

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on bigword endian */

  /* Pack Member: m_nTailPage */
  (pToData)[0] |= ((pFrom)->m_nTailPage & 0x3f) <<2;
  (pToData)[7] |= ((pFrom)->m_nTailPage >> 6) & 0x0f;

  /* Pack Member: m_nTailOffset */
  (pToData)[1] |= ((pFrom)->m_nTailOffset & 0x07) <<5;
  (pToData)[0] |= ((pFrom)->m_nTailOffset >> 3) & 0x03;

  /* Pack Member: m_nCurrDepth */
  (pToData)[3] |= ((pFrom)->m_nCurrDepth & 0x07) <<5;
  (pToData)[2] |= ((pFrom)->m_nCurrDepth >> 3) &0xFF;
  (pToData)[1] |= ((pFrom)->m_nCurrDepth >> 11) & 0x1f;

  /* Pack Member: m_nHeadOffset */
  (pToData)[3] |= ((pFrom)->m_nHeadOffset & 0x1f);
#else
  int i;
  int size = SB_ZF_ZFKAEGMEMFIFOCONTRILENTRY_SIZE_IN_BYTES;

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on little endian */

  /* Pack Member: m_nTailPage */
  (pToData)[3] |= ((pFrom)->m_nTailPage & 0x3f) <<2;
  (pToData)[4] |= ((pFrom)->m_nTailPage >> 6) & 0x0f;

  /* Pack Member: m_nTailOffset */
  (pToData)[2] |= ((pFrom)->m_nTailOffset & 0x07) <<5;
  (pToData)[3] |= ((pFrom)->m_nTailOffset >> 3) & 0x03;

  /* Pack Member: m_nCurrDepth */
  (pToData)[0] |= ((pFrom)->m_nCurrDepth & 0x07) <<5;
  (pToData)[1] |= ((pFrom)->m_nCurrDepth >> 3) &0xFF;
  (pToData)[2] |= ((pFrom)->m_nCurrDepth >> 11) & 0x1f;

  /* Pack Member: m_nHeadOffset */
  (pToData)[0] |= ((pFrom)->m_nHeadOffset & 0x1f);
#endif

  return SB_ZF_ZFKAEGMEMFIFOCONTRILENTRY_SIZE_IN_BYTES;
}




/* Unpack from array of bytes into struct */
void
sbZfKaEgMemFifoControlEntry_Unpack(sbZfKaEgMemFifoControlEntry_t *pToStruct,
                                   uint8 *pFromData,
                                   uint32 nMaxToDataIndex) {
  COMPILER_UINT64 tmp;

  (void) tmp;

#ifdef SAND_BIG_ENDIAN_HOST

  /* Unpack operation based on bigword endian */

  /* Unpack Member: m_nTailPage */
  (pToStruct)->m_nTailPage =  (uint32)  ((pFromData)[0] >> 2) & 0x3f;
  (pToStruct)->m_nTailPage |=  (uint32)  ((pFromData)[7] & 0x0f) << 6;

  /* Unpack Member: m_nTailOffset */
  (pToStruct)->m_nTailOffset =  (uint32)  ((pFromData)[1] >> 5) & 0x07;
  (pToStruct)->m_nTailOffset |=  (uint32)  ((pFromData)[0] & 0x03) << 3;

  /* Unpack Member: m_nCurrDepth */
  (pToStruct)->m_nCurrDepth =  (uint32)  ((pFromData)[3] >> 5) & 0x07;
  (pToStruct)->m_nCurrDepth |=  (uint32)  (pFromData)[2] << 3;
  (pToStruct)->m_nCurrDepth |=  (uint32)  ((pFromData)[1] & 0x1f) << 11;

  /* Unpack Member: m_nHeadOffset */
  (pToStruct)->m_nHeadOffset =  (uint32)  ((pFromData)[3] ) & 0x1f;
#else

  /* Unpack operation based on little endian */

  /* Unpack Member: m_nTailPage */
  (pToStruct)->m_nTailPage =  (uint32)  ((pFromData)[3] >> 2) & 0x3f;
  (pToStruct)->m_nTailPage |=  (uint32)  ((pFromData)[4] & 0x0f) << 6;

  /* Unpack Member: m_nTailOffset */
  (pToStruct)->m_nTailOffset =  (uint32)  ((pFromData)[2] >> 5) & 0x07;
  (pToStruct)->m_nTailOffset |=  (uint32)  ((pFromData)[3] & 0x03) << 3;

  /* Unpack Member: m_nCurrDepth */
  (pToStruct)->m_nCurrDepth =  (uint32)  ((pFromData)[0] >> 5) & 0x07;
  (pToStruct)->m_nCurrDepth |=  (uint32)  (pFromData)[1] << 3;
  (pToStruct)->m_nCurrDepth |=  (uint32)  ((pFromData)[2] & 0x1f) << 11;

  /* Unpack Member: m_nHeadOffset */
  (pToStruct)->m_nHeadOffset =  (uint32)  ((pFromData)[0] ) & 0x1f;
#endif

}



/* initialize an instance of this zframe */
void
sbZfKaEgMemFifoControlEntry_InitInstance(sbZfKaEgMemFifoControlEntry_t *pFrame) {

  pFrame->m_nTailPage =  (unsigned int)  0;
  pFrame->m_nTailOffset =  (unsigned int)  0;
  pFrame->m_nCurrDepth =  (unsigned int)  0;
  pFrame->m_nHeadOffset =  (unsigned int)  0;

}
