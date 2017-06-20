/*
 * $Id: sbZfFabBm9600InaSysportMapEntry.c,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


#include "sbTypes.h"
#include "sbZfFabBm9600InaSysportMapEntry.hx"
#include "sbWrappers.h"
#include <sal/types.h>



/* Pack from struct into array of bytes */
uint32
sbZfFabBm9600InaSysportMapEntry_Pack(sbZfFabBm9600InaSysportMapEntry_t *pFrom,
                                     uint8 *pToData,
                                     uint32 nMaxToDataIndex) {
#ifdef SAND_BIG_ENDIAN_HOST
  int i;
  int size = SB_ZF_FAB_BM9600_INASYSPORTMAPENTRY_SIZE_IN_BYTES;

  if (size % 4) {
    size += (4 - size %4);
  }

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on bigword endian */

  /* Pack Member: m_uPortsetAddr */
  (pToData)[3] |= ((pFrom)->m_uPortsetAddr & 0x0f) <<4;
  (pToData)[2] |= ((pFrom)->m_uPortsetAddr >> 4) & 0x0f;

  /* Pack Member: m_uOffset */
  (pToData)[3] |= ((pFrom)->m_uOffset & 0x0f);
#else
  int i;
  int size = SB_ZF_FAB_BM9600_INASYSPORTMAPENTRY_SIZE_IN_BYTES;

  if (size % 4) {
    size += (4 - size %4);
  }

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on little endian */

  /* Pack Member: m_uPortsetAddr */
  (pToData)[0] |= ((pFrom)->m_uPortsetAddr & 0x0f) <<4;
  (pToData)[1] |= ((pFrom)->m_uPortsetAddr >> 4) & 0x0f;

  /* Pack Member: m_uOffset */
  (pToData)[0] |= ((pFrom)->m_uOffset & 0x0f);
#endif

  return SB_ZF_FAB_BM9600_INASYSPORTMAPENTRY_SIZE_IN_BYTES;
}




/* Unpack from array of bytes into struct */
void
sbZfFabBm9600InaSysportMapEntry_Unpack(sbZfFabBm9600InaSysportMapEntry_t *pToStruct,
                                       uint8 *pFromData,
                                       uint32 nMaxToDataIndex) {
  COMPILER_UINT64 tmp;

  (void) tmp;

#ifdef SAND_BIG_ENDIAN_HOST

  /* Unpack operation based on bigword endian */

  /* Unpack Member: m_uPortsetAddr */
  (pToStruct)->m_uPortsetAddr =  (uint32)  ((pFromData)[3] >> 4) & 0x0f;
  (pToStruct)->m_uPortsetAddr |=  (uint32)  ((pFromData)[2] & 0x0f) << 4;

  /* Unpack Member: m_uOffset */
  (pToStruct)->m_uOffset =  (uint32)  ((pFromData)[3] ) & 0x0f;
#else

  /* Unpack operation based on little endian */

  /* Unpack Member: m_uPortsetAddr */
  (pToStruct)->m_uPortsetAddr =  (uint32)  ((pFromData)[0] >> 4) & 0x0f;
  (pToStruct)->m_uPortsetAddr |=  (uint32)  ((pFromData)[1] & 0x0f) << 4;

  /* Unpack Member: m_uOffset */
  (pToStruct)->m_uOffset =  (uint32)  ((pFromData)[0] ) & 0x0f;
#endif

}



/* initialize an instance of this zframe */
void
sbZfFabBm9600InaSysportMapEntry_InitInstance(sbZfFabBm9600InaSysportMapEntry_t *pFrame) {

  pFrame->m_uPortsetAddr =  (unsigned int)  0;
  pFrame->m_uOffset =  (unsigned int)  0;

}
