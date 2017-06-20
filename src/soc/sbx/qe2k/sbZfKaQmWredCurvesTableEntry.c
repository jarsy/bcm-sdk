/*
 * $Id: sbZfKaQmWredCurvesTableEntry.c,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


#include "sbTypes.h"
#include "sbZfKaQmWredCurvesTableEntry.hx"
#include "sbWrappers.h"
#include <sal/types.h>



/* Pack from struct into array of bytes */
uint32
sbZfKaQmWredCurvesTableEntry_Pack(sbZfKaQmWredCurvesTableEntry_t *pFrom,
                                  uint8 *pToData,
                                  uint32 nMaxToDataIndex) {
#ifdef SAND_BIG_ENDIAN_HOST
  int i;
  int size = SB_ZF_ZFKAQMWREDCURVESTABLEENTRY_SIZE_IN_BYTES;

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on bigword endian */

  /* Pack Member: m_nTmin */
  (pToData)[5] |= ((pFrom)->m_nTmin) & 0xFF;
  (pToData)[4] |= ((pFrom)->m_nTmin >> 8) &0xFF;

  /* Pack Member: m_nTmax */
  (pToData)[7] |= ((pFrom)->m_nTmax) & 0xFF;
  (pToData)[6] |= ((pFrom)->m_nTmax >> 8) &0xFF;

  /* Pack Member: m_nTecn */
  (pToData)[1] |= ((pFrom)->m_nTecn) & 0xFF;
  (pToData)[0] |= ((pFrom)->m_nTecn >> 8) &0xFF;

  /* Pack Member: m_nScale */
  (pToData)[2] |= ((pFrom)->m_nScale & 0x0f) <<4;

  /* Pack Member: m_nSlope */
  (pToData)[3] |= ((pFrom)->m_nSlope) & 0xFF;
  (pToData)[2] |= ((pFrom)->m_nSlope >> 8) & 0x0f;
#else
  int i;
  int size = SB_ZF_ZFKAQMWREDCURVESTABLEENTRY_SIZE_IN_BYTES;

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on little endian */

  /* Pack Member: m_nTmin */
  (pToData)[6] |= ((pFrom)->m_nTmin) & 0xFF;
  (pToData)[7] |= ((pFrom)->m_nTmin >> 8) &0xFF;

  /* Pack Member: m_nTmax */
  (pToData)[4] |= ((pFrom)->m_nTmax) & 0xFF;
  (pToData)[5] |= ((pFrom)->m_nTmax >> 8) &0xFF;

  /* Pack Member: m_nTecn */
  (pToData)[2] |= ((pFrom)->m_nTecn) & 0xFF;
  (pToData)[3] |= ((pFrom)->m_nTecn >> 8) &0xFF;

  /* Pack Member: m_nScale */
  (pToData)[1] |= ((pFrom)->m_nScale & 0x0f) <<4;

  /* Pack Member: m_nSlope */
  (pToData)[0] |= ((pFrom)->m_nSlope) & 0xFF;
  (pToData)[1] |= ((pFrom)->m_nSlope >> 8) & 0x0f;
#endif

  return SB_ZF_ZFKAQMWREDCURVESTABLEENTRY_SIZE_IN_BYTES;
}




/* Unpack from array of bytes into struct */
void
sbZfKaQmWredCurvesTableEntry_Unpack(sbZfKaQmWredCurvesTableEntry_t *pToStruct,
                                    uint8 *pFromData,
                                    uint32 nMaxToDataIndex) {
  COMPILER_UINT64 tmp;

  (void) tmp;

#ifdef SAND_BIG_ENDIAN_HOST

  /* Unpack operation based on bigword endian */

  /* Unpack Member: m_nTmin */
  (pToStruct)->m_nTmin =  (uint32)  (pFromData)[5] ;
  (pToStruct)->m_nTmin |=  (uint32)  (pFromData)[4] << 8;

  /* Unpack Member: m_nTmax */
  (pToStruct)->m_nTmax =  (uint32)  (pFromData)[7] ;
  (pToStruct)->m_nTmax |=  (uint32)  (pFromData)[6] << 8;

  /* Unpack Member: m_nTecn */
  (pToStruct)->m_nTecn =  (uint32)  (pFromData)[1] ;
  (pToStruct)->m_nTecn |=  (uint32)  (pFromData)[0] << 8;

  /* Unpack Member: m_nScale */
  (pToStruct)->m_nScale =  (uint32)  ((pFromData)[2] >> 4) & 0x0f;

  /* Unpack Member: m_nSlope */
  (pToStruct)->m_nSlope =  (uint32)  (pFromData)[3] ;
  (pToStruct)->m_nSlope |=  (uint32)  ((pFromData)[2] & 0x0f) << 8;
#else

  /* Unpack operation based on little endian */

  /* Unpack Member: m_nTmin */
  (pToStruct)->m_nTmin =  (uint32)  (pFromData)[6] ;
  (pToStruct)->m_nTmin |=  (uint32)  (pFromData)[7] << 8;

  /* Unpack Member: m_nTmax */
  (pToStruct)->m_nTmax =  (uint32)  (pFromData)[4] ;
  (pToStruct)->m_nTmax |=  (uint32)  (pFromData)[5] << 8;

  /* Unpack Member: m_nTecn */
  (pToStruct)->m_nTecn =  (uint32)  (pFromData)[2] ;
  (pToStruct)->m_nTecn |=  (uint32)  (pFromData)[3] << 8;

  /* Unpack Member: m_nScale */
  (pToStruct)->m_nScale =  (uint32)  ((pFromData)[1] >> 4) & 0x0f;

  /* Unpack Member: m_nSlope */
  (pToStruct)->m_nSlope =  (uint32)  (pFromData)[0] ;
  (pToStruct)->m_nSlope |=  (uint32)  ((pFromData)[1] & 0x0f) << 8;
#endif

}



/* initialize an instance of this zframe */
void
sbZfKaQmWredCurvesTableEntry_InitInstance(sbZfKaQmWredCurvesTableEntry_t *pFrame) {

  pFrame->m_nTmin =  (unsigned int)  0;
  pFrame->m_nTmax =  (unsigned int)  0;
  pFrame->m_nTecn =  (unsigned int)  0;
  pFrame->m_nScale =  (unsigned int)  0;
  pFrame->m_nSlope =  (unsigned int)  0;

}
