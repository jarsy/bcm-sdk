/*
 * $Id: sbZfKaEgMemFifoParamEntry.c,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


#include "sbTypes.h"
#include "sbZfKaEgMemShapingEntry.hx"
#include "sbZfKaEgMemFifoParamEntry.hx"
#include "sbWrappers.h"
#include <sal/types.h>



/* Pack from struct into array of bytes */
uint32
sbZfKaEgMemFifoParamEntry_Pack(sbZfKaEgMemFifoParamEntry_t *pFrom,
                               uint8 *pToData,
                               uint32 nMaxToDataIndex) {
#ifdef SAND_BIG_ENDIAN_HOST
  int i;
  int size = SB_ZF_ZFKAEGMEMFIFOPARAMENTRY_SIZE_IN_BYTES;

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on bigword endian */

  /* Pack Member: m_nThreshHi */
  (pToData)[0] |= ((pFrom)->m_nThreshHi & 0x3f) <<2;
  (pToData)[7] |= ((pFrom)->m_nThreshHi >> 6) & 0x0f;

  /* Pack Member: m_nThreshLo */
  (pToData)[1] |= ((pFrom)->m_nThreshLo) & 0xFF;
  (pToData)[0] |= ((pFrom)->m_nThreshLo >> 8) & 0x03;

  /* Pack Member: m_nShaper1 */
  (pToData)[2] |= ((pFrom)->m_nShaper1) & 0xFF;

  /* Pack Member: m_nShaper0 */
  (pToData)[3] |= ((pFrom)->m_nShaper0) & 0xFF;
#else
  int i;
  int size = SB_ZF_ZFKAEGMEMFIFOPARAMENTRY_SIZE_IN_BYTES;

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on little endian */

  /* Pack Member: m_nThreshHi */
  (pToData)[3] |= ((pFrom)->m_nThreshHi & 0x3f) <<2;
  (pToData)[4] |= ((pFrom)->m_nThreshHi >> 6) & 0x0f;

  /* Pack Member: m_nThreshLo */
  (pToData)[2] |= ((pFrom)->m_nThreshLo) & 0xFF;
  (pToData)[3] |= ((pFrom)->m_nThreshLo >> 8) & 0x03;

  /* Pack Member: m_nShaper1 */
  (pToData)[1] |= ((pFrom)->m_nShaper1) & 0xFF;

  /* Pack Member: m_nShaper0 */
  (pToData)[0] |= ((pFrom)->m_nShaper0) & 0xFF;
#endif

  return SB_ZF_ZFKAEGMEMFIFOPARAMENTRY_SIZE_IN_BYTES;
}




/* Unpack from array of bytes into struct */
void
sbZfKaEgMemFifoParamEntry_Unpack(sbZfKaEgMemFifoParamEntry_t *pToStruct,
                                 uint8 *pFromData,
                                 uint32 nMaxToDataIndex) {
  COMPILER_UINT64 tmp;

  (void) tmp;

#ifdef SAND_BIG_ENDIAN_HOST

  /* Unpack operation based on bigword endian */

  /* Unpack Member: m_nThreshHi */
  (pToStruct)->m_nThreshHi =  (uint32)  ((pFromData)[0] >> 2) & 0x3f;
  (pToStruct)->m_nThreshHi |=  (uint32)  ((pFromData)[7] & 0x0f) << 6;

  /* Unpack Member: m_nThreshLo */
  (pToStruct)->m_nThreshLo =  (uint32)  (pFromData)[1] ;
  (pToStruct)->m_nThreshLo |=  (uint32)  ((pFromData)[0] & 0x03) << 8;

  /* Unpack Member: m_nShaper1 */
  (pToStruct)->m_nShaper1 =  (uint32)  (pFromData)[2] ;

  /* Unpack Member: m_nShaper0 */
  (pToStruct)->m_nShaper0 =  (uint32)  (pFromData)[3] ;
#else

  /* Unpack operation based on little endian */

  /* Unpack Member: m_nThreshHi */
  (pToStruct)->m_nThreshHi =  (uint32)  ((pFromData)[3] >> 2) & 0x3f;
  (pToStruct)->m_nThreshHi |=  (uint32)  ((pFromData)[4] & 0x0f) << 6;

  /* Unpack Member: m_nThreshLo */
  (pToStruct)->m_nThreshLo =  (uint32)  (pFromData)[2] ;
  (pToStruct)->m_nThreshLo |=  (uint32)  ((pFromData)[3] & 0x03) << 8;

  /* Unpack Member: m_nShaper1 */
  (pToStruct)->m_nShaper1 =  (uint32)  (pFromData)[1] ;

  /* Unpack Member: m_nShaper0 */
  (pToStruct)->m_nShaper0 =  (uint32)  (pFromData)[0] ;
#endif

}



/* initialize an instance of this zframe */
void
sbZfKaEgMemFifoParamEntry_InitInstance(sbZfKaEgMemFifoParamEntry_t *pFrame) {

  pFrame->m_nThreshHi =  (unsigned int)  0;
  pFrame->m_nThreshLo =  (unsigned int)  0;
  pFrame->m_nShaper1 =  (unsigned int)  0;
  pFrame->m_nShaper0 =  (unsigned int)  0;

}
