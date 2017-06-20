/*
 * $Id: sbZfFabBm9600BwAllocCfgBaseEntry.c,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


#include "sbTypes.h"
#include "sbZfFabBm9600BwAllocCfgBaseEntry.hx"
#include "sbWrappers.h"
#include <sal/types.h>



/* Pack from struct into array of bytes */
uint32
sbZfFabBm9600BwAllocCfgBaseEntry_Pack(sbZfFabBm9600BwAllocCfgBaseEntry_t *pFrom,
                                      uint8 *pToData,
                                      uint32 nMaxToDataIndex) {
#ifdef SAND_BIG_ENDIAN_HOST
  int i;
  int size = SB_ZF_FAB_BM9600_BWALLOCCFGBASEENTRY_SIZE_IN_BYTES;

  if (size % 4) {
    size += (4 - size %4);
  }

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on bigword endian */

  /* Pack Member: m_uEnc */
  (pToData)[1] |= ((pFrom)->m_uEnc & 0x03) <<5;

  /* Pack Member: m_uNumVoqs */
  (pToData)[1] |= ((pFrom)->m_uNumVoqs & 0x1f);

  /* Pack Member: m_uBaseVoq */
  (pToData)[3] |= ((pFrom)->m_uBaseVoq) & 0xFF;
  (pToData)[2] |= ((pFrom)->m_uBaseVoq >> 8) &0xFF;
#else
  int i;
  int size = SB_ZF_FAB_BM9600_BWALLOCCFGBASEENTRY_SIZE_IN_BYTES;

  if (size % 4) {
    size += (4 - size %4);
  }

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on little endian */

  /* Pack Member: m_uEnc */
  (pToData)[2] |= ((pFrom)->m_uEnc & 0x03) <<5;

  /* Pack Member: m_uNumVoqs */
  (pToData)[2] |= ((pFrom)->m_uNumVoqs & 0x1f);

  /* Pack Member: m_uBaseVoq */
  (pToData)[0] |= ((pFrom)->m_uBaseVoq) & 0xFF;
  (pToData)[1] |= ((pFrom)->m_uBaseVoq >> 8) &0xFF;
#endif

  return SB_ZF_FAB_BM9600_BWALLOCCFGBASEENTRY_SIZE_IN_BYTES;
}




/* Unpack from array of bytes into struct */
void
sbZfFabBm9600BwAllocCfgBaseEntry_Unpack(sbZfFabBm9600BwAllocCfgBaseEntry_t *pToStruct,
                                        uint8 *pFromData,
                                        uint32 nMaxToDataIndex) {
  COMPILER_UINT64 tmp;

  (void) tmp;

#ifdef SAND_BIG_ENDIAN_HOST

  /* Unpack operation based on bigword endian */

  /* Unpack Member: m_uEnc */
  (pToStruct)->m_uEnc =  (uint32)  ((pFromData)[1] >> 5) & 0x03;

  /* Unpack Member: m_uNumVoqs */
  (pToStruct)->m_uNumVoqs =  (uint32)  ((pFromData)[1] ) & 0x1f;

  /* Unpack Member: m_uBaseVoq */
  (pToStruct)->m_uBaseVoq =  (uint32)  (pFromData)[3] ;
  (pToStruct)->m_uBaseVoq |=  (uint32)  (pFromData)[2] << 8;
#else

  /* Unpack operation based on little endian */

  /* Unpack Member: m_uEnc */
  (pToStruct)->m_uEnc =  (uint32)  ((pFromData)[2] >> 5) & 0x03;

  /* Unpack Member: m_uNumVoqs */
  (pToStruct)->m_uNumVoqs =  (uint32)  ((pFromData)[2] ) & 0x1f;

  /* Unpack Member: m_uBaseVoq */
  (pToStruct)->m_uBaseVoq =  (uint32)  (pFromData)[0] ;
  (pToStruct)->m_uBaseVoq |=  (uint32)  (pFromData)[1] << 8;
#endif

}



/* initialize an instance of this zframe */
void
sbZfFabBm9600BwAllocCfgBaseEntry_InitInstance(sbZfFabBm9600BwAllocCfgBaseEntry_t *pFrame) {

  pFrame->m_uEnc =  (unsigned int)  0;
  pFrame->m_uNumVoqs =  (unsigned int)  0;
  pFrame->m_uBaseVoq =  (unsigned int)  0;

}
