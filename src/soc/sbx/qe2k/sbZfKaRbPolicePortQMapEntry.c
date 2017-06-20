/*
 * $Id: sbZfKaRbPolicePortQMapEntry.c,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


#include "sbTypes.h"
#include "sbZfKaRbPolicePortQMapEntry.hx"
#include "sbWrappers.h"
#include <sal/types.h>



/* Pack from struct into array of bytes */
uint32
sbZfKaRbPolicePortQMapEntry_Pack(sbZfKaRbPolicePortQMapEntry_t *pFrom,
                                 uint8 *pToData,
                                 uint32 nMaxToDataIndex) {
#ifdef SAND_BIG_ENDIAN_HOST
  int i;
  int size = SB_ZF_ZFKARBPOLPORTQMAPENTRY_SIZE_IN_BYTES;

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on bigword endian */

  /* Pack Member: m_nReserved2 */
  (pToData)[0] |= ((pFrom)->m_nReserved2 & 0x7f) <<1;

  /* Pack Member: m_nOddMeter */
  (pToData)[1] |= ((pFrom)->m_nOddMeter) & 0xFF;
  (pToData)[0] |= ((pFrom)->m_nOddMeter >> 8) & 0x01;

  /* Pack Member: m_nReserved1 */
  (pToData)[2] |= ((pFrom)->m_nReserved1 & 0x7f) <<1;

  /* Pack Member: m_nEvenMeter */
  (pToData)[3] |= ((pFrom)->m_nEvenMeter) & 0xFF;
  (pToData)[2] |= ((pFrom)->m_nEvenMeter >> 8) & 0x01;
#else
  int i;
  int size = SB_ZF_ZFKARBPOLPORTQMAPENTRY_SIZE_IN_BYTES;

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on little endian */

  /* Pack Member: m_nReserved2 */
  (pToData)[3] |= ((pFrom)->m_nReserved2 & 0x7f) <<1;

  /* Pack Member: m_nOddMeter */
  (pToData)[2] |= ((pFrom)->m_nOddMeter) & 0xFF;
  (pToData)[3] |= ((pFrom)->m_nOddMeter >> 8) & 0x01;

  /* Pack Member: m_nReserved1 */
  (pToData)[1] |= ((pFrom)->m_nReserved1 & 0x7f) <<1;

  /* Pack Member: m_nEvenMeter */
  (pToData)[0] |= ((pFrom)->m_nEvenMeter) & 0xFF;
  (pToData)[1] |= ((pFrom)->m_nEvenMeter >> 8) & 0x01;
#endif

  return SB_ZF_ZFKARBPOLPORTQMAPENTRY_SIZE_IN_BYTES;
}




/* Unpack from array of bytes into struct */
void
sbZfKaRbPolicePortQMapEntry_Unpack(sbZfKaRbPolicePortQMapEntry_t *pToStruct,
                                   uint8 *pFromData,
                                   uint32 nMaxToDataIndex) {
  COMPILER_UINT64 tmp;

  (void) tmp;

#ifdef SAND_BIG_ENDIAN_HOST

  /* Unpack operation based on bigword endian */

  /* Unpack Member: m_nReserved2 */
  (pToStruct)->m_nReserved2 =  (uint32)  ((pFromData)[0] >> 1) & 0x7f;

  /* Unpack Member: m_nOddMeter */
  (pToStruct)->m_nOddMeter =  (uint32)  (pFromData)[1] ;
  (pToStruct)->m_nOddMeter |=  (uint32)  ((pFromData)[0] & 0x01) << 8;

  /* Unpack Member: m_nReserved1 */
  (pToStruct)->m_nReserved1 =  (uint32)  ((pFromData)[2] >> 1) & 0x7f;

  /* Unpack Member: m_nEvenMeter */
  (pToStruct)->m_nEvenMeter =  (uint32)  (pFromData)[3] ;
  (pToStruct)->m_nEvenMeter |=  (uint32)  ((pFromData)[2] & 0x01) << 8;
#else

  /* Unpack operation based on little endian */

  /* Unpack Member: m_nReserved2 */
  (pToStruct)->m_nReserved2 =  (uint32)  ((pFromData)[3] >> 1) & 0x7f;

  /* Unpack Member: m_nOddMeter */
  (pToStruct)->m_nOddMeter =  (uint32)  (pFromData)[2] ;
  (pToStruct)->m_nOddMeter |=  (uint32)  ((pFromData)[3] & 0x01) << 8;

  /* Unpack Member: m_nReserved1 */
  (pToStruct)->m_nReserved1 =  (uint32)  ((pFromData)[1] >> 1) & 0x7f;

  /* Unpack Member: m_nEvenMeter */
  (pToStruct)->m_nEvenMeter =  (uint32)  (pFromData)[0] ;
  (pToStruct)->m_nEvenMeter |=  (uint32)  ((pFromData)[1] & 0x01) << 8;
#endif

}



/* initialize an instance of this zframe */
void
sbZfKaRbPolicePortQMapEntry_InitInstance(sbZfKaRbPolicePortQMapEntry_t *pFrame) {

  pFrame->m_nReserved2 =  (unsigned int)  0;
  pFrame->m_nOddMeter =  (unsigned int)  0;
  pFrame->m_nReserved1 =  (unsigned int)  0;
  pFrame->m_nEvenMeter =  (unsigned int)  0;

}
