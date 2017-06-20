/*
 * $Id: sbZfFabBm9600BwFetchSumEntry.c,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


#include "sbTypes.h"
#include "sbZfFabBm9600BwFetchSumEntry.hx"
#include "sbWrappers.h"
#include <sal/types.h>



/* Pack from struct into array of bytes */
uint32
sbZfFabBm9600BwFetchSumEntry_Pack(sbZfFabBm9600BwFetchSumEntry_t *pFrom,
                                  uint8 *pToData,
                                  uint32 nMaxToDataIndex) {
#ifdef SAND_BIG_ENDIAN_HOST
  int i;
  int size = SB_ZF_FAB_BM9600_BWFETCHSUMENTRY_SIZE_IN_BYTES;

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on bigword endian */

  /* Pack Member: m_uSum */
  (pToData)[3] |= ((pFrom)->m_uSum) & 0xFF;
  (pToData)[2] |= ((pFrom)->m_uSum >> 8) &0xFF;
  (pToData)[1] |= ((pFrom)->m_uSum >> 16) &0xFF;
  (pToData)[0] |= ((pFrom)->m_uSum >> 24) &0xFF;
#else
  int i;
  int size = SB_ZF_FAB_BM9600_BWFETCHSUMENTRY_SIZE_IN_BYTES;

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on little endian */

  /* Pack Member: m_uSum */
  (pToData)[0] |= ((pFrom)->m_uSum) & 0xFF;
  (pToData)[1] |= ((pFrom)->m_uSum >> 8) &0xFF;
  (pToData)[2] |= ((pFrom)->m_uSum >> 16) &0xFF;
  (pToData)[3] |= ((pFrom)->m_uSum >> 24) &0xFF;
#endif

  return SB_ZF_FAB_BM9600_BWFETCHSUMENTRY_SIZE_IN_BYTES;
}




/* Unpack from array of bytes into struct */
void
sbZfFabBm9600BwFetchSumEntry_Unpack(sbZfFabBm9600BwFetchSumEntry_t *pToStruct,
                                    uint8 *pFromData,
                                    uint32 nMaxToDataIndex) {
  COMPILER_UINT64 tmp;

  (void) tmp;

#ifdef SAND_BIG_ENDIAN_HOST

  /* Unpack operation based on bigword endian */

  /* Unpack Member: m_uSum */
  (pToStruct)->m_uSum =  (uint32)  (pFromData)[3] ;
  (pToStruct)->m_uSum |=  (uint32)  (pFromData)[2] << 8;
  (pToStruct)->m_uSum |=  (uint32)  (pFromData)[1] << 16;
  (pToStruct)->m_uSum |=  (uint32)  (pFromData)[0] << 24;
#else

  /* Unpack operation based on little endian */

  /* Unpack Member: m_uSum */
  (pToStruct)->m_uSum =  (uint32)  (pFromData)[0] ;
  (pToStruct)->m_uSum |=  (uint32)  (pFromData)[1] << 8;
  (pToStruct)->m_uSum |=  (uint32)  (pFromData)[2] << 16;
  (pToStruct)->m_uSum |=  (uint32)  (pFromData)[3] << 24;
#endif

}



/* initialize an instance of this zframe */
void
sbZfFabBm9600BwFetchSumEntry_InitInstance(sbZfFabBm9600BwFetchSumEntry_t *pFrame) {

  pFrame->m_uSum =  (unsigned int)  0;

}
