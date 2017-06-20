/*
 * $Id: sbZfKaQsAgeThreshLutEntry.c,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


#include "sbTypes.h"
#include "sbZfKaQsAgeThreshLutEntry.hx"
#include "sbWrappers.h"
#include <sal/types.h>



/* Pack from struct into array of bytes */
uint32
sbZfKaQsAgeThreshLutEntry_Pack(sbZfKaQsAgeThreshLutEntry_t *pFrom,
                               uint8 *pToData,
                               uint32 nMaxToDataIndex) {
#ifdef SAND_BIG_ENDIAN_HOST
  int i;
  int size = SB_ZF_ZFKAQSAGETHRESHLUTENTRY_SIZE_IN_BYTES;

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on bigword endian */

  /* Pack Member: m_nReserved */
  (pToData)[1] |= ((pFrom)->m_nReserved) & 0xFF;
  (pToData)[0] |= ((pFrom)->m_nReserved >> 8) &0xFF;

  /* Pack Member: m_nAnemicThresh */
  (pToData)[2] |= ((pFrom)->m_nAnemicThresh) & 0xFF;

  /* Pack Member: m_nEfThresh */
  (pToData)[3] |= ((pFrom)->m_nEfThresh) & 0xFF;
#else
  int i;
  int size = SB_ZF_ZFKAQSAGETHRESHLUTENTRY_SIZE_IN_BYTES;

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on little endian */

  /* Pack Member: m_nReserved */
  (pToData)[2] |= ((pFrom)->m_nReserved) & 0xFF;
  (pToData)[3] |= ((pFrom)->m_nReserved >> 8) &0xFF;

  /* Pack Member: m_nAnemicThresh */
  (pToData)[1] |= ((pFrom)->m_nAnemicThresh) & 0xFF;

  /* Pack Member: m_nEfThresh */
  (pToData)[0] |= ((pFrom)->m_nEfThresh) & 0xFF;
#endif

  return SB_ZF_ZFKAQSAGETHRESHLUTENTRY_SIZE_IN_BYTES;
}




/* Unpack from array of bytes into struct */
void
sbZfKaQsAgeThreshLutEntry_Unpack(sbZfKaQsAgeThreshLutEntry_t *pToStruct,
                                 uint8 *pFromData,
                                 uint32 nMaxToDataIndex) {
  COMPILER_UINT64 tmp;

  (void) tmp;

#ifdef SAND_BIG_ENDIAN_HOST

  /* Unpack operation based on bigword endian */

  /* Unpack Member: m_nReserved */
  (pToStruct)->m_nReserved =  (uint32)  (pFromData)[1] ;
  (pToStruct)->m_nReserved |=  (uint32)  (pFromData)[0] << 8;

  /* Unpack Member: m_nAnemicThresh */
  (pToStruct)->m_nAnemicThresh =  (uint32)  (pFromData)[2] ;

  /* Unpack Member: m_nEfThresh */
  (pToStruct)->m_nEfThresh =  (uint32)  (pFromData)[3] ;
#else

  /* Unpack operation based on little endian */

  /* Unpack Member: m_nReserved */
  (pToStruct)->m_nReserved =  (uint32)  (pFromData)[2] ;
  (pToStruct)->m_nReserved |=  (uint32)  (pFromData)[3] << 8;

  /* Unpack Member: m_nAnemicThresh */
  (pToStruct)->m_nAnemicThresh =  (uint32)  (pFromData)[1] ;

  /* Unpack Member: m_nEfThresh */
  (pToStruct)->m_nEfThresh =  (uint32)  (pFromData)[0] ;
#endif

}



/* initialize an instance of this zframe */
void
sbZfKaQsAgeThreshLutEntry_InitInstance(sbZfKaQsAgeThreshLutEntry_t *pFrame) {

  pFrame->m_nReserved =  (unsigned int)  0;
  pFrame->m_nAnemicThresh =  (unsigned int)  0;
  pFrame->m_nEfThresh =  (unsigned int)  0;

}
