/*
 * $Id: sbZfFabBm9600BwR0BwpEntry.c,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


#include "sbTypes.h"
#include "sbZfFabBm9600BwR0BwpEntry.hx"
#include "sbWrappers.h"
#include <sal/types.h>



/* Pack from struct into array of bytes */
uint32
sbZfFabBm9600BwR0BwpEntry_Pack(sbZfFabBm9600BwR0BwpEntry_t *pFrom,
                               uint8 *pToData,
                               uint32 nMaxToDataIndex) {
#ifdef SAND_BIG_ENDIAN_HOST
  int i;
  int size = SB_ZF_FAB_BM9600_BWR0BWPENTRY_SIZE_IN_BYTES;

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on bigword endian */

  /* Pack Member: m_uGamma */
  (pToData)[0] |= ((pFrom)->m_uGamma) & 0xFF;

  /* Pack Member: m_uSigma */
  (pToData)[3] |= ((pFrom)->m_uSigma) & 0xFF;
  (pToData)[2] |= ((pFrom)->m_uSigma >> 8) &0xFF;
  (pToData)[1] |= ((pFrom)->m_uSigma >> 16) &0xFF;
#else
  int i;
  int size = SB_ZF_FAB_BM9600_BWR0BWPENTRY_SIZE_IN_BYTES;

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on little endian */

  /* Pack Member: m_uGamma */
  (pToData)[3] |= ((pFrom)->m_uGamma) & 0xFF;

  /* Pack Member: m_uSigma */
  (pToData)[0] |= ((pFrom)->m_uSigma) & 0xFF;
  (pToData)[1] |= ((pFrom)->m_uSigma >> 8) &0xFF;
  (pToData)[2] |= ((pFrom)->m_uSigma >> 16) &0xFF;
#endif

  return SB_ZF_FAB_BM9600_BWR0BWPENTRY_SIZE_IN_BYTES;
}




/* Unpack from array of bytes into struct */
void
sbZfFabBm9600BwR0BwpEntry_Unpack(sbZfFabBm9600BwR0BwpEntry_t *pToStruct,
                                 uint8 *pFromData,
                                 uint32 nMaxToDataIndex) {
  COMPILER_UINT64 tmp;

  (void) tmp;

#ifdef SAND_BIG_ENDIAN_HOST

  /* Unpack operation based on bigword endian */

  /* Unpack Member: m_uGamma */
  (pToStruct)->m_uGamma =  (uint32)  (pFromData)[0] ;

  /* Unpack Member: m_uSigma */
  (pToStruct)->m_uSigma =  (uint32)  (pFromData)[3] ;
  (pToStruct)->m_uSigma |=  (uint32)  (pFromData)[2] << 8;
  (pToStruct)->m_uSigma |=  (uint32)  (pFromData)[1] << 16;
#else

  /* Unpack operation based on little endian */

  /* Unpack Member: m_uGamma */
  (pToStruct)->m_uGamma =  (uint32)  (pFromData)[3] ;

  /* Unpack Member: m_uSigma */
  (pToStruct)->m_uSigma =  (uint32)  (pFromData)[0] ;
  (pToStruct)->m_uSigma |=  (uint32)  (pFromData)[1] << 8;
  (pToStruct)->m_uSigma |=  (uint32)  (pFromData)[2] << 16;
#endif

}



/* initialize an instance of this zframe */
void
sbZfFabBm9600BwR0BwpEntry_InitInstance(sbZfFabBm9600BwR0BwpEntry_t *pFrame) {

  pFrame->m_uGamma =  (unsigned int)  0;
  pFrame->m_uSigma =  (unsigned int)  0;

}
