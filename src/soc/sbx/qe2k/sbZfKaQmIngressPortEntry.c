/*
 * $Id: sbZfKaQmIngressPortEntry.c,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


#include "sbTypes.h"
#include "sbZfKaQmIngressPortEntry.hx"
#include "sbWrappers.h"
#include <sal/types.h>



/* Pack from struct into array of bytes */
uint32
sbZfKaQmIngressPortEntry_Pack(sbZfKaQmIngressPortEntry_t *pFrom,
                              uint8 *pToData,
                              uint32 nMaxToDataIndex) {
#ifdef SAND_BIG_ENDIAN_HOST
  int i;
  int size = SB_ZF_ZFKAQMINGRESSPORTENTRY_SIZE_IN_BYTES;

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on bigword endian */

  /* Pack Member: m_nIngressSpi4 */
  (pToData)[3] |= ((pFrom)->m_nIngressSpi4 & 0x01) <<6;

  /* Pack Member: m_nIngressPort */
  (pToData)[3] |= ((pFrom)->m_nIngressPort & 0x3f);
#else
  int i;
  int size = SB_ZF_ZFKAQMINGRESSPORTENTRY_SIZE_IN_BYTES;

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on little endian */

  /* Pack Member: m_nIngressSpi4 */
  (pToData)[0] |= ((pFrom)->m_nIngressSpi4 & 0x01) <<6;

  /* Pack Member: m_nIngressPort */
  (pToData)[0] |= ((pFrom)->m_nIngressPort & 0x3f);
#endif

  return SB_ZF_ZFKAQMINGRESSPORTENTRY_SIZE_IN_BYTES;
}




/* Unpack from array of bytes into struct */
void
sbZfKaQmIngressPortEntry_Unpack(sbZfKaQmIngressPortEntry_t *pToStruct,
                                uint8 *pFromData,
                                uint32 nMaxToDataIndex) {
  COMPILER_UINT64 tmp;

  (void) tmp;

#ifdef SAND_BIG_ENDIAN_HOST

  /* Unpack operation based on bigword endian */

  /* Unpack Member: m_nIngressSpi4 */
  (pToStruct)->m_nIngressSpi4 =  (uint8)  ((pFromData)[3] >> 6) & 0x01;

  /* Unpack Member: m_nIngressPort */
  (pToStruct)->m_nIngressPort =  (uint32)  ((pFromData)[3] ) & 0x3f;
#else

  /* Unpack operation based on little endian */

  /* Unpack Member: m_nIngressSpi4 */
  (pToStruct)->m_nIngressSpi4 =  (uint8)  ((pFromData)[0] >> 6) & 0x01;

  /* Unpack Member: m_nIngressPort */
  (pToStruct)->m_nIngressPort =  (uint32)  ((pFromData)[0] ) & 0x3f;
#endif

}



/* initialize an instance of this zframe */
void
sbZfKaQmIngressPortEntry_InitInstance(sbZfKaQmIngressPortEntry_t *pFrame) {

  pFrame->m_nIngressSpi4 =  (unsigned int)  0;
  pFrame->m_nIngressPort =  (unsigned int)  0;

}
