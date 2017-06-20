/*
 * $Id: sbZfFabBm9600NmPortsetInfoEntry.c,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


#include "sbTypes.h"
#include "sbZfFabBm9600NmPortsetInfoEntry.hx"
#include "sbWrappers.h"
#include <sal/types.h>



/* Pack from struct into array of bytes */
uint32
sbZfFabBm9600NmPortsetInfoEntry_Pack(sbZfFabBm9600NmPortsetInfoEntry_t *pFrom,
                                     uint8 *pToData,
                                     uint32 nMaxToDataIndex) {
#ifdef SAND_BIG_ENDIAN_HOST
  int i;
  int size = SB_ZF_FAB_BM9600_NMPORTSETINFOENTRY_SIZE_IN_BYTES;

  if (size % 4) {
    size += (4 - size %4);
  }

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on bigword endian */

  /* Pack Member: m_uVirtualPort */
  (pToData)[1] |= ((pFrom)->m_uVirtualPort & 0x01) <<2;

  /* Pack Member: m_uVportEopp */
  (pToData)[2] |= ((pFrom)->m_uVportEopp & 0x01) <<7;
  (pToData)[1] |= ((pFrom)->m_uVportEopp >> 1) & 0x03;

  /* Pack Member: m_uStartPort */
  (pToData)[3] |= ((pFrom)->m_uStartPort & 0x01) <<7;
  (pToData)[2] |= ((pFrom)->m_uStartPort >> 1) & 0x7f;

  /* Pack Member: m_uEgNode */
  (pToData)[3] |= ((pFrom)->m_uEgNode & 0x7f);
#else
  int i;
  int size = SB_ZF_FAB_BM9600_NMPORTSETINFOENTRY_SIZE_IN_BYTES;

  if (size % 4) {
    size += (4 - size %4);
  }

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on little endian */

  /* Pack Member: m_uVirtualPort */
  (pToData)[2] |= ((pFrom)->m_uVirtualPort & 0x01) <<2;

  /* Pack Member: m_uVportEopp */
  (pToData)[1] |= ((pFrom)->m_uVportEopp & 0x01) <<7;
  (pToData)[2] |= ((pFrom)->m_uVportEopp >> 1) & 0x03;

  /* Pack Member: m_uStartPort */
  (pToData)[0] |= ((pFrom)->m_uStartPort & 0x01) <<7;
  (pToData)[1] |= ((pFrom)->m_uStartPort >> 1) & 0x7f;

  /* Pack Member: m_uEgNode */
  (pToData)[0] |= ((pFrom)->m_uEgNode & 0x7f);
#endif

  return SB_ZF_FAB_BM9600_NMPORTSETINFOENTRY_SIZE_IN_BYTES;
}




/* Unpack from array of bytes into struct */
void
sbZfFabBm9600NmPortsetInfoEntry_Unpack(sbZfFabBm9600NmPortsetInfoEntry_t *pToStruct,
                                       uint8 *pFromData,
                                       uint32 nMaxToDataIndex) {
  COMPILER_UINT64 tmp;

  (void) tmp;

#ifdef SAND_BIG_ENDIAN_HOST

  /* Unpack operation based on bigword endian */

  /* Unpack Member: m_uVirtualPort */
  (pToStruct)->m_uVirtualPort =  (uint32)  ((pFromData)[1] >> 2) & 0x01;

  /* Unpack Member: m_uVportEopp */
  (pToStruct)->m_uVportEopp =  (uint32)  ((pFromData)[2] >> 7) & 0x01;
  (pToStruct)->m_uVportEopp |=  (uint32)  ((pFromData)[1] & 0x03) << 1;

  /* Unpack Member: m_uStartPort */
  (pToStruct)->m_uStartPort =  (uint32)  ((pFromData)[3] >> 7) & 0x01;
  (pToStruct)->m_uStartPort |=  (uint32)  ((pFromData)[2] & 0x7f) << 1;

  /* Unpack Member: m_uEgNode */
  (pToStruct)->m_uEgNode =  (uint32)  ((pFromData)[3] ) & 0x7f;
#else

  /* Unpack operation based on little endian */

  /* Unpack Member: m_uVirtualPort */
  (pToStruct)->m_uVirtualPort =  (uint32)  ((pFromData)[2] >> 2) & 0x01;

  /* Unpack Member: m_uVportEopp */
  (pToStruct)->m_uVportEopp =  (uint32)  ((pFromData)[1] >> 7) & 0x01;
  (pToStruct)->m_uVportEopp |=  (uint32)  ((pFromData)[2] & 0x03) << 1;

  /* Unpack Member: m_uStartPort */
  (pToStruct)->m_uStartPort =  (uint32)  ((pFromData)[0] >> 7) & 0x01;
  (pToStruct)->m_uStartPort |=  (uint32)  ((pFromData)[1] & 0x7f) << 1;

  /* Unpack Member: m_uEgNode */
  (pToStruct)->m_uEgNode =  (uint32)  ((pFromData)[0] ) & 0x7f;
#endif

}



/* initialize an instance of this zframe */
void
sbZfFabBm9600NmPortsetInfoEntry_InitInstance(sbZfFabBm9600NmPortsetInfoEntry_t *pFrame) {

  pFrame->m_uVirtualPort =  (unsigned int)  0;
  pFrame->m_uVportEopp =  (unsigned int)  0;
  pFrame->m_uStartPort =  (unsigned int)  0;
  pFrame->m_uEgNode =  (unsigned int)  0;

}
