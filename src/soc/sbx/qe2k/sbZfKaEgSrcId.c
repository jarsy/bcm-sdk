/*
 * $Id: sbZfKaEgSrcId.c,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


#include "sbTypes.h"
#include "sbZfKaEgSrcId.hx"
#include "sbWrappers.h"
#include <sal/types.h>



/* Pack from struct into array of bytes */
uint32
sbZfKaEgSrcId_Pack(sbZfKaEgSrcId_t *pFrom,
                   uint8 *pToData,
                   uint32 nMaxToDataIndex) {
#ifdef SAND_BIG_ENDIAN_HOST
  int i;
  int size = SB_ZF_ZFKAEGSRCID_SIZE_IN_BYTES;

  if (size % 4) {
    size += (4 - size %4);
  }

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on bigword endian */

  /* Pack Member: m_nSrcId */
  (pToData)[3] |= ((pFrom)->m_nSrcId) & 0xFF;
  (pToData)[2] |= ((pFrom)->m_nSrcId >> 8) & 0x0f;
#else
  int i;
  int size = SB_ZF_ZFKAEGSRCID_SIZE_IN_BYTES;

  if (size % 4) {
    size += (4 - size %4);
  }

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on little endian */

  /* Pack Member: m_nSrcId */
  (pToData)[0] |= ((pFrom)->m_nSrcId) & 0xFF;
  (pToData)[1] |= ((pFrom)->m_nSrcId >> 8) & 0x0f;
#endif

  return SB_ZF_ZFKAEGSRCID_SIZE_IN_BYTES;
}




/* Unpack from array of bytes into struct */
void
sbZfKaEgSrcId_Unpack(sbZfKaEgSrcId_t *pToStruct,
                     uint8 *pFromData,
                     uint32 nMaxToDataIndex) {
  COMPILER_UINT64 tmp;

  (void) tmp;

#ifdef SAND_BIG_ENDIAN_HOST

  /* Unpack operation based on bigword endian */

  /* Unpack Member: m_nSrcId */
  (pToStruct)->m_nSrcId =  (uint32)  (pFromData)[3] ;
  (pToStruct)->m_nSrcId |=  (uint32)  ((pFromData)[2] & 0x0f) << 8;
#else

  /* Unpack operation based on little endian */

  /* Unpack Member: m_nSrcId */
  (pToStruct)->m_nSrcId =  (uint32)  (pFromData)[0] ;
  (pToStruct)->m_nSrcId |=  (uint32)  ((pFromData)[1] & 0x0f) << 8;
#endif

}



/* initialize an instance of this zframe */
void
sbZfKaEgSrcId_InitInstance(sbZfKaEgSrcId_t *pFrame) {

  pFrame->m_nSrcId =  (unsigned int)  0;

}
