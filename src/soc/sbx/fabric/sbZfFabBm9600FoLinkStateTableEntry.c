/*
 * $Id: sbZfFabBm9600FoLinkStateTableEntry.c,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


#include "sbTypes.h"
#include "sbZfFabBm9600FoLinkStateTableEntry.hx"
#include "sbWrappers.h"
#include <sal/types.h>



/* Pack from struct into array of bytes */
uint32
sbZfFabBm9600FoLinkStateTableEntry_Pack(sbZfFabBm9600FoLinkStateTableEntry_t *pFrom,
                                        uint8 *pToData,
                                        uint32 nMaxToDataIndex) {
#ifdef SAND_BIG_ENDIAN_HOST
  int i;
  int size = SB_ZF_FAB_BM9600_FOLINKSTATETABLEENTRY_SIZE_IN_BYTES;

  if (size % 4) {
    size += (4 - size %4);
  }

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on bigword endian */

  /* Pack Member: m_uSb */
  (pToData)[7] |= ((pFrom)->m_uSb & 0x01) <<1;

  /* Pack Member: m_uError */
  (pToData)[7] |= ((pFrom)->m_uError & 0x01);

  /* Pack Member: m_uLinkState */
  (pToData)[3] |= ((pFrom)->m_uLinkState) & 0xFF;
  (pToData)[2] |= ((pFrom)->m_uLinkState >> 8) &0xFF;
  (pToData)[1] |= ((pFrom)->m_uLinkState >> 16) &0xFF;
  (pToData)[0] |= ((pFrom)->m_uLinkState >> 24) &0xFF;
#else
  int i;
  int size = SB_ZF_FAB_BM9600_FOLINKSTATETABLEENTRY_SIZE_IN_BYTES;

  if (size % 4) {
    size += (4 - size %4);
  }

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on little endian */

  /* Pack Member: m_uSb */
  (pToData)[4] |= ((pFrom)->m_uSb & 0x01) <<1;

  /* Pack Member: m_uError */
  (pToData)[4] |= ((pFrom)->m_uError & 0x01);

  /* Pack Member: m_uLinkState */
  (pToData)[0] |= ((pFrom)->m_uLinkState) & 0xFF;
  (pToData)[1] |= ((pFrom)->m_uLinkState >> 8) &0xFF;
  (pToData)[2] |= ((pFrom)->m_uLinkState >> 16) &0xFF;
  (pToData)[3] |= ((pFrom)->m_uLinkState >> 24) &0xFF;
#endif

  return SB_ZF_FAB_BM9600_FOLINKSTATETABLEENTRY_SIZE_IN_BYTES;
}




/* Unpack from array of bytes into struct */
void
sbZfFabBm9600FoLinkStateTableEntry_Unpack(sbZfFabBm9600FoLinkStateTableEntry_t *pToStruct,
                                          uint8 *pFromData,
                                          uint32 nMaxToDataIndex) {
  COMPILER_UINT64 tmp;

  (void) tmp;

#ifdef SAND_BIG_ENDIAN_HOST

  /* Unpack operation based on bigword endian */

  /* Unpack Member: m_uSb */
  (pToStruct)->m_uSb =  (uint32)  ((pFromData)[7] >> 1) & 0x01;

  /* Unpack Member: m_uError */
  (pToStruct)->m_uError =  (uint32)  ((pFromData)[7] ) & 0x01;

  /* Unpack Member: m_uLinkState */
  (pToStruct)->m_uLinkState =  (uint32)  (pFromData)[3] ;
  (pToStruct)->m_uLinkState |=  (uint32)  (pFromData)[2] << 8;
  (pToStruct)->m_uLinkState |=  (uint32)  (pFromData)[1] << 16;
  (pToStruct)->m_uLinkState |=  (uint32)  (pFromData)[0] << 24;
#else

  /* Unpack operation based on little endian */

  /* Unpack Member: m_uSb */
  (pToStruct)->m_uSb =  (uint32)  ((pFromData)[4] >> 1) & 0x01;

  /* Unpack Member: m_uError */
  (pToStruct)->m_uError =  (uint32)  ((pFromData)[4] ) & 0x01;

  /* Unpack Member: m_uLinkState */
  (pToStruct)->m_uLinkState =  (uint32)  (pFromData)[0] ;
  (pToStruct)->m_uLinkState |=  (uint32)  (pFromData)[1] << 8;
  (pToStruct)->m_uLinkState |=  (uint32)  (pFromData)[2] << 16;
  (pToStruct)->m_uLinkState |=  (uint32)  (pFromData)[3] << 24;
#endif

}



/* initialize an instance of this zframe */
void
sbZfFabBm9600FoLinkStateTableEntry_InitInstance(sbZfFabBm9600FoLinkStateTableEntry_t *pFrame) {

  pFrame->m_uSb =  (unsigned int)  0;
  pFrame->m_uError =  (unsigned int)  0;
  pFrame->m_uLinkState =  (unsigned int)  0;

}
