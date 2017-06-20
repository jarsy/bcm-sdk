/*
 * $Id: sbZfFabBm9600NmPortsetLinkEntry.c,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


#include "sbTypes.h"
#include "sbZfFabBm9600NmPortsetLinkEntry.hx"
#include "sbWrappers.h"
#include <sal/types.h>



/* Pack from struct into array of bytes */
uint32
sbZfFabBm9600NmPortsetLinkEntry_Pack(sbZfFabBm9600NmPortsetLinkEntry_t *pFrom,
                                     uint8 *pToData,
                                     uint32 nMaxToDataIndex) {
#ifdef SAND_BIG_ENDIAN_HOST
  int i;
  int size = SB_ZF_FAB_BM9600_NMPORTSETLINKENTRY_SIZE_IN_BYTES;

  if (size % 4) {
    size += (4 - size %4);
  }

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on bigword endian */

  /* Pack Member: m_uIndex */
  (pToData)[2] |= ((pFrom)->m_uIndex) & 0xFF;

  /* Pack Member: m_uNxtPtr */
  (pToData)[3] |= ((pFrom)->m_uNxtPtr) & 0xFF;
#else
  int i;
  int size = SB_ZF_FAB_BM9600_NMPORTSETLINKENTRY_SIZE_IN_BYTES;

  if (size % 4) {
    size += (4 - size %4);
  }

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on little endian */

  /* Pack Member: m_uIndex */
  (pToData)[1] |= ((pFrom)->m_uIndex) & 0xFF;

  /* Pack Member: m_uNxtPtr */
  (pToData)[0] |= ((pFrom)->m_uNxtPtr) & 0xFF;
#endif

  return SB_ZF_FAB_BM9600_NMPORTSETLINKENTRY_SIZE_IN_BYTES;
}




/* Unpack from array of bytes into struct */
void
sbZfFabBm9600NmPortsetLinkEntry_Unpack(sbZfFabBm9600NmPortsetLinkEntry_t *pToStruct,
                                       uint8 *pFromData,
                                       uint32 nMaxToDataIndex) {
  COMPILER_UINT64 tmp;

  (void) tmp;

#ifdef SAND_BIG_ENDIAN_HOST

  /* Unpack operation based on bigword endian */

  /* Unpack Member: m_uIndex */
  (pToStruct)->m_uIndex =  (uint32)  (pFromData)[2] ;

  /* Unpack Member: m_uNxtPtr */
  (pToStruct)->m_uNxtPtr =  (uint32)  (pFromData)[3] ;
#else

  /* Unpack operation based on little endian */

  /* Unpack Member: m_uIndex */
  (pToStruct)->m_uIndex =  (uint32)  (pFromData)[1] ;

  /* Unpack Member: m_uNxtPtr */
  (pToStruct)->m_uNxtPtr =  (uint32)  (pFromData)[0] ;
#endif

}



/* initialize an instance of this zframe */
void
sbZfFabBm9600NmPortsetLinkEntry_InitInstance(sbZfFabBm9600NmPortsetLinkEntry_t *pFrame) {

  pFrame->m_uIndex =  (unsigned int)  0;
  pFrame->m_uNxtPtr =  (unsigned int)  0;

}
