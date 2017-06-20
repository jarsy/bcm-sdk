/*
 * $Id: sbZfFabBm9600InaHi1Selected_1Entry.c,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


#include "sbTypes.h"
#include "sbZfFabBm9600InaHi1Selected_1Entry.hx"
#include "sbWrappers.h"
#include <sal/types.h>



/* Pack from struct into array of bytes */
uint32
sbZfFabBm9600InaHi1Selected_1Entry_Pack(sbZfFabBm9600InaHi1Selected_1Entry_t *pFrom,
                                        uint8 *pToData,
                                        uint32 nMaxToDataIndex) {
#ifdef SAND_BIG_ENDIAN_HOST
  int i;
  int size = SB_ZF_FAB_BM9600_INAHI1SELECTED_1ENTRY_SIZE_IN_BYTES;

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on bigword endian */

  /* Pack Member: m_uPri */
  (pToData)[0] |= ((pFrom)->m_uPri & 0x0f);

  /* Pack Member: m_uPortsetAddr */
  (pToData)[1] |= ((pFrom)->m_uPortsetAddr) & 0xFF;

  /* Pack Member: m_uOffset */
  (pToData)[2] |= ((pFrom)->m_uOffset & 0x0f) <<4;

  /* Pack Member: m_uSysport */
  (pToData)[3] |= ((pFrom)->m_uSysport) & 0xFF;
  (pToData)[2] |= ((pFrom)->m_uSysport >> 8) & 0x0f;
#else
  int i;
  int size = SB_ZF_FAB_BM9600_INAHI1SELECTED_1ENTRY_SIZE_IN_BYTES;

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on little endian */

  /* Pack Member: m_uPri */
  (pToData)[3] |= ((pFrom)->m_uPri & 0x0f);

  /* Pack Member: m_uPortsetAddr */
  (pToData)[2] |= ((pFrom)->m_uPortsetAddr) & 0xFF;

  /* Pack Member: m_uOffset */
  (pToData)[1] |= ((pFrom)->m_uOffset & 0x0f) <<4;

  /* Pack Member: m_uSysport */
  (pToData)[0] |= ((pFrom)->m_uSysport) & 0xFF;
  (pToData)[1] |= ((pFrom)->m_uSysport >> 8) & 0x0f;
#endif

  return SB_ZF_FAB_BM9600_INAHI1SELECTED_1ENTRY_SIZE_IN_BYTES;
}




/* Unpack from array of bytes into struct */
void
sbZfFabBm9600InaHi1Selected_1Entry_Unpack(sbZfFabBm9600InaHi1Selected_1Entry_t *pToStruct,
                                          uint8 *pFromData,
                                          uint32 nMaxToDataIndex) {
  COMPILER_UINT64 tmp;

  (void) tmp;

#ifdef SAND_BIG_ENDIAN_HOST

  /* Unpack operation based on bigword endian */

  /* Unpack Member: m_uPri */
  (pToStruct)->m_uPri =  (uint32)  ((pFromData)[0] ) & 0x0f;

  /* Unpack Member: m_uPortsetAddr */
  (pToStruct)->m_uPortsetAddr =  (uint32)  (pFromData)[1] ;

  /* Unpack Member: m_uOffset */
  (pToStruct)->m_uOffset =  (uint32)  ((pFromData)[2] >> 4) & 0x0f;

  /* Unpack Member: m_uSysport */
  (pToStruct)->m_uSysport =  (uint32)  (pFromData)[3] ;
  (pToStruct)->m_uSysport |=  (uint32)  ((pFromData)[2] & 0x0f) << 8;
#else

  /* Unpack operation based on little endian */

  /* Unpack Member: m_uPri */
  (pToStruct)->m_uPri =  (uint32)  ((pFromData)[3] ) & 0x0f;

  /* Unpack Member: m_uPortsetAddr */
  (pToStruct)->m_uPortsetAddr =  (uint32)  (pFromData)[2] ;

  /* Unpack Member: m_uOffset */
  (pToStruct)->m_uOffset =  (uint32)  ((pFromData)[1] >> 4) & 0x0f;

  /* Unpack Member: m_uSysport */
  (pToStruct)->m_uSysport =  (uint32)  (pFromData)[0] ;
  (pToStruct)->m_uSysport |=  (uint32)  ((pFromData)[1] & 0x0f) << 8;
#endif

}



/* initialize an instance of this zframe */
void
sbZfFabBm9600InaHi1Selected_1Entry_InitInstance(sbZfFabBm9600InaHi1Selected_1Entry_t *pFrame) {

  pFrame->m_uPri =  (unsigned int)  0;
  pFrame->m_uPortsetAddr =  (unsigned int)  0;
  pFrame->m_uOffset =  (unsigned int)  0;
  pFrame->m_uSysport =  (unsigned int)  0;

}
