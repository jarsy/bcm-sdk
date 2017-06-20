/*
 * $Id: sbZfKaQmQueueAgeEntry.c,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


#include "sbTypes.h"
#include "sbZfKaQmQueueAgeEntry.hx"
#include "sbWrappers.h"
#include <sal/types.h>



/* Pack from struct into array of bytes */
uint32
sbZfKaQmQueueAgeEntry_Pack(sbZfKaQmQueueAgeEntry_t *pFrom,
                           uint8 *pToData,
                           uint32 nMaxToDataIndex) {
#ifdef SAND_BIG_ENDIAN_HOST
  int i;
  int size = SB_ZF_ZFKAQMQUEUEAGEENTRY_SIZE_IN_BYTES;

  if (size % 4) {
    size += (4 - size %4);
  }

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on bigword endian */

  /* Pack Member: m_nAge */
  (pToData)[3] |= ((pFrom)->m_nAge & 0x0f);
#else
  int i;
  int size = SB_ZF_ZFKAQMQUEUEAGEENTRY_SIZE_IN_BYTES;

  if (size % 4) {
    size += (4 - size %4);
  }

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on little endian */

  /* Pack Member: m_nAge */
  (pToData)[0] |= ((pFrom)->m_nAge & 0x0f);
#endif

  return SB_ZF_ZFKAQMQUEUEAGEENTRY_SIZE_IN_BYTES;
}




/* Unpack from array of bytes into struct */
void
sbZfKaQmQueueAgeEntry_Unpack(sbZfKaQmQueueAgeEntry_t *pToStruct,
                             uint8 *pFromData,
                             uint32 nMaxToDataIndex) {
  COMPILER_UINT64 tmp;

  (void) tmp;

#ifdef SAND_BIG_ENDIAN_HOST

  /* Unpack operation based on bigword endian */

  /* Unpack Member: m_nAge */
  (pToStruct)->m_nAge =  (uint32)  ((pFromData)[3] ) & 0x0f;
#else

  /* Unpack operation based on little endian */

  /* Unpack Member: m_nAge */
  (pToStruct)->m_nAge =  (uint32)  ((pFromData)[0] ) & 0x0f;
#endif

}



/* initialize an instance of this zframe */
void
sbZfKaQmQueueAgeEntry_InitInstance(sbZfKaQmQueueAgeEntry_t *pFrame) {

  pFrame->m_nAge =  (unsigned int)  0;

}
