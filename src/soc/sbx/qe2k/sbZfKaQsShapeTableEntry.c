/*
 * $Id: sbZfKaQsShapeTableEntry.c,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


#include "sbTypes.h"
#include "sbZfKaQsShapeTableEntry.hx"
#include "sbWrappers.h"
#include <sal/types.h>



/* Pack from struct into array of bytes */
uint32
sbZfKaQsShapeTableEntry_Pack(sbZfKaQsShapeTableEntry_t *pFrom,
                             uint8 *pToData,
                             uint32 nMaxToDataIndex) {
#ifdef SAND_BIG_ENDIAN_HOST
  int i;
  int size = SB_ZF_ZFKAQSSHAPETABLEENTRY_SIZE_IN_BYTES;

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on bigword endian */

  /* Pack Member: m_nShapeRateLS2B */
  (pToData)[5] |= ((pFrom)->m_nShapeRateLS2B) & 0xFF;
  (pToData)[4] |= ((pFrom)->m_nShapeRateLS2B >> 8) &0xFF;

  /* Pack Member: m_nEnable */
  (pToData)[6] |= ((pFrom)->m_nEnable & 0x01) <<7;

  /* Pack Member: m_nShapeMaxBurst */
  (pToData)[0] |= ((pFrom)->m_nShapeMaxBurst) & 0xFF;
  (pToData)[7] |= ((pFrom)->m_nShapeMaxBurst >> 8) &0xFF;
  (pToData)[6] |= ((pFrom)->m_nShapeMaxBurst >> 16) & 0x7f;

  /* Pack Member: m_nShape */
  (pToData)[3] |= ((pFrom)->m_nShape) & 0xFF;
  (pToData)[2] |= ((pFrom)->m_nShape >> 8) &0xFF;
  (pToData)[1] |= ((pFrom)->m_nShape >> 16) &0xFF;
#else
  int i;
  int size = SB_ZF_ZFKAQSSHAPETABLEENTRY_SIZE_IN_BYTES;

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on little endian */

  /* Pack Member: m_nShapeRateLS2B */
  (pToData)[6] |= ((pFrom)->m_nShapeRateLS2B) & 0xFF;
  (pToData)[7] |= ((pFrom)->m_nShapeRateLS2B >> 8) &0xFF;

  /* Pack Member: m_nEnable */
  (pToData)[5] |= ((pFrom)->m_nEnable & 0x01) <<7;

  /* Pack Member: m_nShapeMaxBurst */
  (pToData)[3] |= ((pFrom)->m_nShapeMaxBurst) & 0xFF;
  (pToData)[4] |= ((pFrom)->m_nShapeMaxBurst >> 8) &0xFF;
  (pToData)[5] |= ((pFrom)->m_nShapeMaxBurst >> 16) & 0x7f;

  /* Pack Member: m_nShape */
  (pToData)[0] |= ((pFrom)->m_nShape) & 0xFF;
  (pToData)[1] |= ((pFrom)->m_nShape >> 8) &0xFF;
  (pToData)[2] |= ((pFrom)->m_nShape >> 16) &0xFF;
#endif

  return SB_ZF_ZFKAQSSHAPETABLEENTRY_SIZE_IN_BYTES;
}




/* Unpack from array of bytes into struct */
void
sbZfKaQsShapeTableEntry_Unpack(sbZfKaQsShapeTableEntry_t *pToStruct,
                               uint8 *pFromData,
                               uint32 nMaxToDataIndex) {
  COMPILER_UINT64 tmp;

  (void) tmp;

#ifdef SAND_BIG_ENDIAN_HOST

  /* Unpack operation based on bigword endian */

  /* Unpack Member: m_nShapeRateLS2B */
  (pToStruct)->m_nShapeRateLS2B =  (uint32)  (pFromData)[5] ;
  (pToStruct)->m_nShapeRateLS2B |=  (uint32)  (pFromData)[4] << 8;

  /* Unpack Member: m_nEnable */
  (pToStruct)->m_nEnable =  (uint32)  ((pFromData)[6] >> 7) & 0x01;

  /* Unpack Member: m_nShapeMaxBurst */
  (pToStruct)->m_nShapeMaxBurst =  (uint32)  (pFromData)[0] ;
  (pToStruct)->m_nShapeMaxBurst |=  (uint32)  (pFromData)[7] << 8;
  (pToStruct)->m_nShapeMaxBurst |=  (uint32)  ((pFromData)[6] & 0x7f) << 16;

  /* Unpack Member: m_nShape */
  (pToStruct)->m_nShape =  (uint32)  (pFromData)[3] ;
  (pToStruct)->m_nShape |=  (uint32)  (pFromData)[2] << 8;
  (pToStruct)->m_nShape |=  (uint32)  (pFromData)[1] << 16;
#else

  /* Unpack operation based on little endian */

  /* Unpack Member: m_nShapeRateLS2B */
  (pToStruct)->m_nShapeRateLS2B =  (uint32)  (pFromData)[6] ;
  (pToStruct)->m_nShapeRateLS2B |=  (uint32)  (pFromData)[7] << 8;

  /* Unpack Member: m_nEnable */
  (pToStruct)->m_nEnable =  (uint32)  ((pFromData)[5] >> 7) & 0x01;

  /* Unpack Member: m_nShapeMaxBurst */
  (pToStruct)->m_nShapeMaxBurst =  (uint32)  (pFromData)[3] ;
  (pToStruct)->m_nShapeMaxBurst |=  (uint32)  (pFromData)[4] << 8;
  (pToStruct)->m_nShapeMaxBurst |=  (uint32)  ((pFromData)[5] & 0x7f) << 16;

  /* Unpack Member: m_nShape */
  (pToStruct)->m_nShape =  (uint32)  (pFromData)[0] ;
  (pToStruct)->m_nShape |=  (uint32)  (pFromData)[1] << 8;
  (pToStruct)->m_nShape |=  (uint32)  (pFromData)[2] << 16;
#endif

}



/* initialize an instance of this zframe */
void
sbZfKaQsShapeTableEntry_InitInstance(sbZfKaQsShapeTableEntry_t *pFrame) {

  pFrame->m_nShapeRateLS2B =  (unsigned int)  0;
  pFrame->m_nEnable =  (unsigned int)  0;
  pFrame->m_nShapeMaxBurst =  (unsigned int)  0;
  pFrame->m_nShape =  (unsigned int)  0;

}
