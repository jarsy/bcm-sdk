/*
 * $Id: sbZfKaEpVlanIndRecord.c,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


#include "sbTypes.h"
#include "sbZfKaEpVlanIndRecord.hx"
#include "sbWrappers.h"
#include <sal/types.h>



/* Pack from struct into array of bytes */
uint32
sbZfKaEpVlanIndRecord_Pack(sbZfKaEpVlanIndRecord_t *pFrom,
                           uint8 *pToData,
                           uint32 nMaxToDataIndex) {
#ifdef SAND_BIG_ENDIAN_HOST
  int i;
  int size = SB_ZF_ZFKAEPVLANINDRECORD_SIZE_IN_BYTES;

  if (size % 4) {
    size += (4 - size %4);
  }

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on bigword endian */

  /* Pack Member: m_nPtr */
  (pToData)[3] |= ((pFrom)->m_nPtr & 0x3f) <<2;
  (pToData)[2] |= ((pFrom)->m_nPtr >> 6) &0xFF;

  /* Pack Member: m_nCMap */
  (pToData)[3] |= ((pFrom)->m_nCMap & 0x03);
#else
  int i;
  int size = SB_ZF_ZFKAEPVLANINDRECORD_SIZE_IN_BYTES;

  if (size % 4) {
    size += (4 - size %4);
  }

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on little endian */

  /* Pack Member: m_nPtr */
  (pToData)[0] |= ((pFrom)->m_nPtr & 0x3f) <<2;
  (pToData)[1] |= ((pFrom)->m_nPtr >> 6) &0xFF;

  /* Pack Member: m_nCMap */
  (pToData)[0] |= ((pFrom)->m_nCMap & 0x03);
#endif

  return SB_ZF_ZFKAEPVLANINDRECORD_SIZE_IN_BYTES;
}




/* Unpack from array of bytes into struct */
void
sbZfKaEpVlanIndRecord_Unpack(sbZfKaEpVlanIndRecord_t *pToStruct,
                             uint8 *pFromData,
                             uint32 nMaxToDataIndex) {
  COMPILER_UINT64 tmp;

  (void) tmp;

#ifdef SAND_BIG_ENDIAN_HOST

  /* Unpack operation based on bigword endian */

  /* Unpack Member: m_nPtr */
  (pToStruct)->m_nPtr =  (uint32)  ((pFromData)[3] >> 2) & 0x3f;
  (pToStruct)->m_nPtr |=  (uint32)  (pFromData)[2] << 6;

  /* Unpack Member: m_nCMap */
  (pToStruct)->m_nCMap =  (uint32)  ((pFromData)[3] ) & 0x03;
#else

  /* Unpack operation based on little endian */

  /* Unpack Member: m_nPtr */
  (pToStruct)->m_nPtr =  (uint32)  ((pFromData)[0] >> 2) & 0x3f;
  (pToStruct)->m_nPtr |=  (uint32)  (pFromData)[1] << 6;

  /* Unpack Member: m_nCMap */
  (pToStruct)->m_nCMap =  (uint32)  ((pFromData)[0] ) & 0x03;
#endif

}



/* initialize an instance of this zframe */
void
sbZfKaEpVlanIndRecord_InitInstance(sbZfKaEpVlanIndRecord_t *pFrame) {

  pFrame->m_nPtr =  (unsigned int)  0;
  pFrame->m_nCMap =  (unsigned int)  0;

}
