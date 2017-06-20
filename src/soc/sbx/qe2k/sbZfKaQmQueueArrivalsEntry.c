/*
 * $Id: sbZfKaQmQueueArrivalsEntry.c,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


#include "sbTypes.h"
#include "sbZfKaQmQueueArrivalsEntry.hx"
#include "sbWrappers.h"
#include <sal/types.h>



/* Pack from struct into array of bytes */
uint32
sbZfKaQmQueueArrivalsEntry_Pack(sbZfKaQmQueueArrivalsEntry_t *pFrom,
                                uint8 *pToData,
                                uint32 nMaxToDataIndex) {
#ifdef SAND_BIG_ENDIAN_HOST
  int i;
  int size = SB_ZF_ZFKAQMQUEUEARRIVALSENTRY_SIZE_IN_BYTES;

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on bigword endian */

  /* Pack Member: m_nArrivals */
  (pToData)[3] |= ((pFrom)->m_nArrivals) & 0xFF;
  (pToData)[2] |= ((pFrom)->m_nArrivals >> 8) &0xFF;
  (pToData)[1] |= ((pFrom)->m_nArrivals >> 16) &0xFF;
  (pToData)[0] |= ((pFrom)->m_nArrivals >> 24) & 0x0f;
#else
  int i;
  int size = SB_ZF_ZFKAQMQUEUEARRIVALSENTRY_SIZE_IN_BYTES;

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on little endian */

  /* Pack Member: m_nArrivals */
  (pToData)[0] |= ((pFrom)->m_nArrivals) & 0xFF;
  (pToData)[1] |= ((pFrom)->m_nArrivals >> 8) &0xFF;
  (pToData)[2] |= ((pFrom)->m_nArrivals >> 16) &0xFF;
  (pToData)[3] |= ((pFrom)->m_nArrivals >> 24) & 0x0f;
#endif

  return SB_ZF_ZFKAQMQUEUEARRIVALSENTRY_SIZE_IN_BYTES;
}




/* Unpack from array of bytes into struct */
void
sbZfKaQmQueueArrivalsEntry_Unpack(sbZfKaQmQueueArrivalsEntry_t *pToStruct,
                                  uint8 *pFromData,
                                  uint32 nMaxToDataIndex) {
  COMPILER_UINT64 tmp;

  (void) tmp;

#ifdef SAND_BIG_ENDIAN_HOST

  /* Unpack operation based on bigword endian */

  /* Unpack Member: m_nArrivals */
  (pToStruct)->m_nArrivals =  (uint32)  (pFromData)[3] ;
  (pToStruct)->m_nArrivals |=  (uint32)  (pFromData)[2] << 8;
  (pToStruct)->m_nArrivals |=  (uint32)  (pFromData)[1] << 16;
  (pToStruct)->m_nArrivals |=  (uint32)  ((pFromData)[0] & 0x0f) << 24;
#else

  /* Unpack operation based on little endian */

  /* Unpack Member: m_nArrivals */
  (pToStruct)->m_nArrivals =  (uint32)  (pFromData)[0] ;
  (pToStruct)->m_nArrivals |=  (uint32)  (pFromData)[1] << 8;
  (pToStruct)->m_nArrivals |=  (uint32)  (pFromData)[2] << 16;
  (pToStruct)->m_nArrivals |=  (uint32)  ((pFromData)[3] & 0x0f) << 24;
#endif

}



/* initialize an instance of this zframe */
void
sbZfKaQmQueueArrivalsEntry_InitInstance(sbZfKaQmQueueArrivalsEntry_t *pFrame) {

  pFrame->m_nArrivals =  (unsigned int)  0;

}
