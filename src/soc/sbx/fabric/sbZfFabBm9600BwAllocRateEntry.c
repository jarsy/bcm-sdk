/*
 * $Id: sbZfFabBm9600BwAllocRateEntry.c,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


#include "sbTypes.h"
#include "sbZfFabBm9600BwAllocRateEntry.hx"
#include "sbWrappers.h"
#include <sal/types.h>



/* Pack from struct into array of bytes */
uint32
sbZfFabBm9600BwAllocRateEntry_Pack(sbZfFabBm9600BwAllocRateEntry_t *pFrom,
                                   uint8 *pToData,
                                   uint32 nMaxToDataIndex) {
#ifdef SAND_BIG_ENDIAN_HOST
  int i;
  int size = SB_ZF_FAB_BM9600_BWALLOCRATEENTRY_SIZE_IN_BYTES;

  if (size % 4) {
    size += (4 - size %4);
  }

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on bigword endian */

  /* Pack Member: m_uRate */
  (pToData)[3] |= ((pFrom)->m_uRate) & 0xFF;
  (pToData)[2] |= ((pFrom)->m_uRate >> 8) &0xFF;
  (pToData)[1] |= ((pFrom)->m_uRate >> 16) &0xFF;
#else
  int i;
  int size = SB_ZF_FAB_BM9600_BWALLOCRATEENTRY_SIZE_IN_BYTES;

  if (size % 4) {
    size += (4 - size %4);
  }

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on little endian */

  /* Pack Member: m_uRate */
  (pToData)[0] |= ((pFrom)->m_uRate) & 0xFF;
  (pToData)[1] |= ((pFrom)->m_uRate >> 8) &0xFF;
  (pToData)[2] |= ((pFrom)->m_uRate >> 16) &0xFF;
#endif

  return SB_ZF_FAB_BM9600_BWALLOCRATEENTRY_SIZE_IN_BYTES;
}




/* Unpack from array of bytes into struct */
void
sbZfFabBm9600BwAllocRateEntry_Unpack(sbZfFabBm9600BwAllocRateEntry_t *pToStruct,
                                     uint8 *pFromData,
                                     uint32 nMaxToDataIndex) {
  COMPILER_UINT64 tmp;

  (void) tmp;

#ifdef SAND_BIG_ENDIAN_HOST

  /* Unpack operation based on bigword endian */

  /* Unpack Member: m_uRate */
  (pToStruct)->m_uRate =  (uint32)  (pFromData)[3] ;
  (pToStruct)->m_uRate |=  (uint32)  (pFromData)[2] << 8;
  (pToStruct)->m_uRate |=  (uint32)  (pFromData)[1] << 16;
#else

  /* Unpack operation based on little endian */

  /* Unpack Member: m_uRate */
  (pToStruct)->m_uRate =  (uint32)  (pFromData)[0] ;
  (pToStruct)->m_uRate |=  (uint32)  (pFromData)[1] << 8;
  (pToStruct)->m_uRate |=  (uint32)  (pFromData)[2] << 16;
#endif

}



/* initialize an instance of this zframe */
void
sbZfFabBm9600BwAllocRateEntry_InitInstance(sbZfFabBm9600BwAllocRateEntry_t *pFrame) {

  pFrame->m_uRate =  (unsigned int)  0;

}
