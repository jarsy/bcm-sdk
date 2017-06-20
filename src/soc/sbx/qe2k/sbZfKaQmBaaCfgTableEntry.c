/*
 * $Id: sbZfKaQmBaaCfgTableEntry.c,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


#include "sbTypes.h"
#include "sbZfKaQmBaaCfgTableEntry.hx"
#include "sbWrappers.h"
#include <sal/types.h>



/* Pack from struct into array of bytes */
uint32
sbZfKaQmBaaCfgTableEntry_Pack(sbZfKaQmBaaCfgTableEntry_t *pFrom,
                              uint8 *pToData,
                              uint32 nMaxToDataIndex) {
#ifdef SAND_BIG_ENDIAN_HOST
  int i;
  int size = SB_ZF_ZFKAQMBAACFGTABLEENTRY_SIZE_IN_BYTES;

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on bigword endian */

  /* Pack Member: m_nGamma */
  (pToData)[3] |= ((pFrom)->m_nGamma) & 0xFF;
#else
  int i;
  int size = SB_ZF_ZFKAQMBAACFGTABLEENTRY_SIZE_IN_BYTES;

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on little endian */

  /* Pack Member: m_nGamma */
  (pToData)[0] |= ((pFrom)->m_nGamma) & 0xFF;
#endif

  return SB_ZF_ZFKAQMBAACFGTABLEENTRY_SIZE_IN_BYTES;
}




/* Unpack from array of bytes into struct */
void
sbZfKaQmBaaCfgTableEntry_Unpack(sbZfKaQmBaaCfgTableEntry_t *pToStruct,
                                uint8 *pFromData,
                                uint32 nMaxToDataIndex) {
  COMPILER_UINT64 tmp;

  (void) tmp;

#ifdef SAND_BIG_ENDIAN_HOST

  /* Unpack operation based on bigword endian */

  /* Unpack Member: m_nGamma */
  (pToStruct)->m_nGamma =  (uint32)  (pFromData)[3] ;
#else

  /* Unpack operation based on little endian */

  /* Unpack Member: m_nGamma */
  (pToStruct)->m_nGamma =  (uint32)  (pFromData)[0] ;
#endif

}



/* initialize an instance of this zframe */
void
sbZfKaQmBaaCfgTableEntry_InitInstance(sbZfKaQmBaaCfgTableEntry_t *pFrame) {

  pFrame->m_nGamma =  (unsigned int)  0;

}
