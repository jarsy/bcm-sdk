/*
 * $Id: sbZfKaQmQueueByteAdjEntry.c,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


#include "sbTypes.h"
#include "sbZfKaQmQueueByteAdjEntry.hx"
#include "sbWrappers.h"
#include <sal/types.h>



/* Pack from struct into array of bytes */
uint32
sbZfKaQmQueueByteAdjEntry_Pack(sbZfKaQmQueueByteAdjEntry_t *pFrom,
                               uint8 *pToData,
                               uint32 nMaxToDataIndex) {
#ifdef SAND_BIG_ENDIAN_HOST
  int i;
  int size = SB_ZF_ZFKAQMQUEUEBYTEADJENTRY_SIZE_IN_BYTES;

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on bigword endian */

  /* Pack Member: m_nSign */
  (pToData)[3] |= ((pFrom)->m_nSign & 0x01) <<6;

  /* Pack Member: m_nBytes */
  (pToData)[3] |= ((pFrom)->m_nBytes & 0x3f);
#else
  int i;
  int size = SB_ZF_ZFKAQMQUEUEBYTEADJENTRY_SIZE_IN_BYTES;

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on little endian */

  /* Pack Member: m_nSign */
  (pToData)[0] |= ((pFrom)->m_nSign & 0x01) <<6;

  /* Pack Member: m_nBytes */
  (pToData)[0] |= ((pFrom)->m_nBytes & 0x3f);
#endif

  return SB_ZF_ZFKAQMQUEUEBYTEADJENTRY_SIZE_IN_BYTES;
}




/* Unpack from array of bytes into struct */
void
sbZfKaQmQueueByteAdjEntry_Unpack(sbZfKaQmQueueByteAdjEntry_t *pToStruct,
                                 uint8 *pFromData,
                                 uint32 nMaxToDataIndex) {
  COMPILER_UINT64 tmp;

  (void) tmp;

#ifdef SAND_BIG_ENDIAN_HOST

  /* Unpack operation based on bigword endian */

  /* Unpack Member: m_nSign */
  (pToStruct)->m_nSign =  (uint32)  ((pFromData)[3] >> 6) & 0x01;

  /* Unpack Member: m_nBytes */
  (pToStruct)->m_nBytes =  (uint32)  ((pFromData)[3] ) & 0x3f;
#else

  /* Unpack operation based on little endian */

  /* Unpack Member: m_nSign */
  (pToStruct)->m_nSign =  (uint32)  ((pFromData)[0] >> 6) & 0x01;

  /* Unpack Member: m_nBytes */
  (pToStruct)->m_nBytes =  (uint32)  ((pFromData)[0] ) & 0x3f;
#endif

}



/* initialize an instance of this zframe */
void
sbZfKaQmQueueByteAdjEntry_InitInstance(sbZfKaQmQueueByteAdjEntry_t *pFrame) {

  pFrame->m_nSign =  (unsigned int)  0;
  pFrame->m_nBytes =  (unsigned int)  0;

}
