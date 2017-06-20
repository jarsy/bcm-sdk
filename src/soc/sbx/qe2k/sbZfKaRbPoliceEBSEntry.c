/*
 * $Id: sbZfKaRbPoliceEBSEntry.c,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


#include "sbTypes.h"
#include "sbZfKaRbPoliceEBSEntry.hx"
#include "sbWrappers.h"
#include <sal/types.h>



/* Pack from struct into array of bytes */
uint32
sbZfKaRbPoliceEBSEntry_Pack(sbZfKaRbPoliceEBSEntry_t *pFrom,
                            uint8 *pToData,
                            uint32 nMaxToDataIndex) {
#ifdef SAND_BIG_ENDIAN_HOST
  int i;
  int size = SB_ZF_ZFKARBPOLEBSENTRY_SIZE_IN_BYTES;

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on bigword endian */

  /* Pack Member: m_nEBS */
  (pToData)[3] |= ((pFrom)->m_nEBS) & 0xFF;
  (pToData)[2] |= ((pFrom)->m_nEBS >> 8) &0xFF;
  (pToData)[1] |= ((pFrom)->m_nEBS >> 16) &0xFF;
#else
  int i;
  int size = SB_ZF_ZFKARBPOLEBSENTRY_SIZE_IN_BYTES;

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on little endian */

  /* Pack Member: m_nEBS */
  (pToData)[0] |= ((pFrom)->m_nEBS) & 0xFF;
  (pToData)[1] |= ((pFrom)->m_nEBS >> 8) &0xFF;
  (pToData)[2] |= ((pFrom)->m_nEBS >> 16) &0xFF;
#endif

  return SB_ZF_ZFKARBPOLEBSENTRY_SIZE_IN_BYTES;
}




/* Unpack from array of bytes into struct */
void
sbZfKaRbPoliceEBSEntry_Unpack(sbZfKaRbPoliceEBSEntry_t *pToStruct,
                              uint8 *pFromData,
                              uint32 nMaxToDataIndex) {
  COMPILER_UINT64 tmp;

  (void) tmp;

#ifdef SAND_BIG_ENDIAN_HOST

  /* Unpack operation based on bigword endian */

  /* Unpack Member: m_nEBS */
  (pToStruct)->m_nEBS =  (uint32)  (pFromData)[3] ;
  (pToStruct)->m_nEBS |=  (uint32)  (pFromData)[2] << 8;
  (pToStruct)->m_nEBS |=  (uint32)  (pFromData)[1] << 16;
#else

  /* Unpack operation based on little endian */

  /* Unpack Member: m_nEBS */
  (pToStruct)->m_nEBS =  (uint32)  (pFromData)[0] ;
  (pToStruct)->m_nEBS |=  (uint32)  (pFromData)[1] << 8;
  (pToStruct)->m_nEBS |=  (uint32)  (pFromData)[2] << 16;
#endif

}



/* initialize an instance of this zframe */
void
sbZfKaRbPoliceEBSEntry_InitInstance(sbZfKaRbPoliceEBSEntry_t *pFrame) {

  pFrame->m_nEBS =  (unsigned int)  0;

}
