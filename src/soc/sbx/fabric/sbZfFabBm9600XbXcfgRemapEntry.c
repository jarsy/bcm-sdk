/*
 * $Id: sbZfFabBm9600XbXcfgRemapEntry.c,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


#include "sbTypes.h"
#include "sbZfFabBm9600XbXcfgRemapEntry.hx"
#include "sbWrappers.h"
#include <sal/types.h>



/* Pack from struct into array of bytes */
uint32
sbZfFabBm9600XbXcfgRemapEntry_Pack(sbZfFabBm9600XbXcfgRemapEntry_t *pFrom,
                                   uint8 *pToData,
                                   uint32 nMaxToDataIndex) {
#ifdef SAND_BIG_ENDIAN_HOST
  int i;
  int size = SB_ZF_FAB_BM9600_XBXCFGREMAPENTRY_SIZE_IN_BYTES;

  if (size % 4) {
    size += (4 - size %4);
  }

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on bigword endian */

  /* Pack Member: m_uXcfgB */
  (pToData)[2] |= ((pFrom)->m_uXcfgB) & 0xFF;

  /* Pack Member: m_uXcfgA */
  (pToData)[3] |= ((pFrom)->m_uXcfgA) & 0xFF;
#else
  int i;
  int size = SB_ZF_FAB_BM9600_XBXCFGREMAPENTRY_SIZE_IN_BYTES;

  if (size % 4) {
    size += (4 - size %4);
  }

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on little endian */

  /* Pack Member: m_uXcfgB */
  (pToData)[1] |= ((pFrom)->m_uXcfgB) & 0xFF;

  /* Pack Member: m_uXcfgA */
  (pToData)[0] |= ((pFrom)->m_uXcfgA) & 0xFF;
#endif

  return SB_ZF_FAB_BM9600_XBXCFGREMAPENTRY_SIZE_IN_BYTES;
}




/* Unpack from array of bytes into struct */
void
sbZfFabBm9600XbXcfgRemapEntry_Unpack(sbZfFabBm9600XbXcfgRemapEntry_t *pToStruct,
                                     uint8 *pFromData,
                                     uint32 nMaxToDataIndex) {
  COMPILER_UINT64 tmp;

  (void) tmp;

#ifdef SAND_BIG_ENDIAN_HOST

  /* Unpack operation based on bigword endian */

  /* Unpack Member: m_uXcfgB */
  (pToStruct)->m_uXcfgB =  (uint32)  (pFromData)[2] ;

  /* Unpack Member: m_uXcfgA */
  (pToStruct)->m_uXcfgA =  (uint32)  (pFromData)[3] ;
#else

  /* Unpack operation based on little endian */

  /* Unpack Member: m_uXcfgB */
  (pToStruct)->m_uXcfgB =  (uint32)  (pFromData)[1] ;

  /* Unpack Member: m_uXcfgA */
  (pToStruct)->m_uXcfgA =  (uint32)  (pFromData)[0] ;
#endif

}



/* initialize an instance of this zframe */
void
sbZfFabBm9600XbXcfgRemapEntry_InitInstance(sbZfFabBm9600XbXcfgRemapEntry_t *pFrame) {

  pFrame->m_uXcfgB =  (unsigned int)  0;
  pFrame->m_uXcfgA =  (unsigned int)  0;

}
