/*
 * $Id: sbZfKaQmDemandCfgDataEntry.c,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


#include "sbTypes.h"
#include "sbZfKaQmDemandCfgDataEntry.hx"
#include "sbWrappers.h"
#include <sal/types.h>



/* Pack from struct into array of bytes */
uint32
sbZfKaQmDemandCfgDataEntry_Pack(sbZfKaQmDemandCfgDataEntry_t *pFrom,
                                uint8 *pToData,
                                uint32 nMaxToDataIndex) {
#ifdef SAND_BIG_ENDIAN_HOST
  int i;
  int size = SB_ZF_ZFKAQMDEMANDCFGDATAENTRY_SIZE_IN_BYTES;

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on bigword endian */

  /* Pack Member: m_nRateDeltaMaxIdx */
  (pToData)[3] |= ((pFrom)->m_nRateDeltaMaxIdx & 0x3f) <<1;

  /* Pack Member: m_nQlaDemandMask */
  (pToData)[3] |= ((pFrom)->m_nQlaDemandMask & 0x01);
#else
  int i;
  int size = SB_ZF_ZFKAQMDEMANDCFGDATAENTRY_SIZE_IN_BYTES;

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on little endian */

  /* Pack Member: m_nRateDeltaMaxIdx */
  (pToData)[0] |= ((pFrom)->m_nRateDeltaMaxIdx & 0x3f) <<1;

  /* Pack Member: m_nQlaDemandMask */
  (pToData)[0] |= ((pFrom)->m_nQlaDemandMask & 0x01);
#endif

  return SB_ZF_ZFKAQMDEMANDCFGDATAENTRY_SIZE_IN_BYTES;
}




/* Unpack from array of bytes into struct */
void
sbZfKaQmDemandCfgDataEntry_Unpack(sbZfKaQmDemandCfgDataEntry_t *pToStruct,
                                  uint8 *pFromData,
                                  uint32 nMaxToDataIndex) {
  COMPILER_UINT64 tmp;

  (void) tmp;

#ifdef SAND_BIG_ENDIAN_HOST

  /* Unpack operation based on bigword endian */

  /* Unpack Member: m_nRateDeltaMaxIdx */
  (pToStruct)->m_nRateDeltaMaxIdx =  (uint32)  ((pFromData)[3] >> 1) & 0x3f;

  /* Unpack Member: m_nQlaDemandMask */
  (pToStruct)->m_nQlaDemandMask =  (uint32)  ((pFromData)[3] ) & 0x01;
#else

  /* Unpack operation based on little endian */

  /* Unpack Member: m_nRateDeltaMaxIdx */
  (pToStruct)->m_nRateDeltaMaxIdx =  (uint32)  ((pFromData)[0] >> 1) & 0x3f;

  /* Unpack Member: m_nQlaDemandMask */
  (pToStruct)->m_nQlaDemandMask =  (uint32)  ((pFromData)[0] ) & 0x01;
#endif

}



/* initialize an instance of this zframe */
void
sbZfKaQmDemandCfgDataEntry_InitInstance(sbZfKaQmDemandCfgDataEntry_t *pFrame) {

  pFrame->m_nRateDeltaMaxIdx =  (unsigned int)  0;
  pFrame->m_nQlaDemandMask =  (unsigned int)  0;

}
