/*
 * $Id: sbZfKaRbClassPortEnablesEntry.c,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


#include "sbTypes.h"
#include "sbZfKaRbClassPortEnablesEntry.hx"
#include "sbWrappers.h"
#include <sal/types.h>



/* Pack from struct into array of bytes */
uint32
sbZfKaRbClassPortEnablesEntry_Pack(sbZfKaRbClassPortEnablesEntry_t *pFrom,
                                   uint8 *pToData,
                                   uint32 nMaxToDataIndex) {
#ifdef SAND_BIG_ENDIAN_HOST
  int i;
  int size = SB_ZF_ZFKARBCLASSPORTENABLESENTRY_SIZE_IN_BYTES;

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on bigword endian */

  /* Pack Member: m_nReserve */
  (pToData)[1] |= ((pFrom)->m_nReserve & 0x01) <<7;
  (pToData)[0] |= ((pFrom)->m_nReserve >> 1) &0xFF;

  /* Pack Member: m_nUseUser1 */
  (pToData)[1] |= ((pFrom)->m_nUseUser1 & 0x01) <<6;

  /* Pack Member: m_nUseUser0 */
  (pToData)[1] |= ((pFrom)->m_nUseUser0 & 0x01) <<5;

  /* Pack Member: m_nUseVlanPri */
  (pToData)[1] |= ((pFrom)->m_nUseVlanPri & 0x01) <<4;

  /* Pack Member: m_nUseHiPriVlan */
  (pToData)[1] |= ((pFrom)->m_nUseHiPriVlan & 0x01) <<3;

  /* Pack Member: m_nUseDmacMatch */
  (pToData)[1] |= ((pFrom)->m_nUseDmacMatch & 0x01) <<2;

  /* Pack Member: m_nUseLayer4 */
  (pToData)[1] |= ((pFrom)->m_nUseLayer4 & 0x01) <<1;

  /* Pack Member: m_nFlowHashEnable */
  (pToData)[1] |= ((pFrom)->m_nFlowHashEnable & 0x01);

  /* Pack Member: m_nUseHashCos */
  (pToData)[3] |= ((pFrom)->m_nUseHashCos) & 0xFF;
  (pToData)[2] |= ((pFrom)->m_nUseHashCos >> 8) &0xFF;
#else
  int i;
  int size = SB_ZF_ZFKARBCLASSPORTENABLESENTRY_SIZE_IN_BYTES;

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on little endian */

  /* Pack Member: m_nReserve */
  (pToData)[2] |= ((pFrom)->m_nReserve & 0x01) <<7;
  (pToData)[3] |= ((pFrom)->m_nReserve >> 1) &0xFF;

  /* Pack Member: m_nUseUser1 */
  (pToData)[2] |= ((pFrom)->m_nUseUser1 & 0x01) <<6;

  /* Pack Member: m_nUseUser0 */
  (pToData)[2] |= ((pFrom)->m_nUseUser0 & 0x01) <<5;

  /* Pack Member: m_nUseVlanPri */
  (pToData)[2] |= ((pFrom)->m_nUseVlanPri & 0x01) <<4;

  /* Pack Member: m_nUseHiPriVlan */
  (pToData)[2] |= ((pFrom)->m_nUseHiPriVlan & 0x01) <<3;

  /* Pack Member: m_nUseDmacMatch */
  (pToData)[2] |= ((pFrom)->m_nUseDmacMatch & 0x01) <<2;

  /* Pack Member: m_nUseLayer4 */
  (pToData)[2] |= ((pFrom)->m_nUseLayer4 & 0x01) <<1;

  /* Pack Member: m_nFlowHashEnable */
  (pToData)[2] |= ((pFrom)->m_nFlowHashEnable & 0x01);

  /* Pack Member: m_nUseHashCos */
  (pToData)[0] |= ((pFrom)->m_nUseHashCos) & 0xFF;
  (pToData)[1] |= ((pFrom)->m_nUseHashCos >> 8) &0xFF;
#endif

  return SB_ZF_ZFKARBCLASSPORTENABLESENTRY_SIZE_IN_BYTES;
}




/* Unpack from array of bytes into struct */
void
sbZfKaRbClassPortEnablesEntry_Unpack(sbZfKaRbClassPortEnablesEntry_t *pToStruct,
                                     uint8 *pFromData,
                                     uint32 nMaxToDataIndex) {
  COMPILER_UINT64 tmp;

  (void) tmp;

#ifdef SAND_BIG_ENDIAN_HOST

  /* Unpack operation based on bigword endian */

  /* Unpack Member: m_nReserve */
  (pToStruct)->m_nReserve =  (uint32)  ((pFromData)[1] >> 7) & 0x01;
  (pToStruct)->m_nReserve |=  (uint32)  (pFromData)[0] << 1;

  /* Unpack Member: m_nUseUser1 */
  (pToStruct)->m_nUseUser1 =  (uint8)  ((pFromData)[1] >> 6) & 0x01;

  /* Unpack Member: m_nUseUser0 */
  (pToStruct)->m_nUseUser0 =  (uint8)  ((pFromData)[1] >> 5) & 0x01;

  /* Unpack Member: m_nUseVlanPri */
  (pToStruct)->m_nUseVlanPri =  (uint8)  ((pFromData)[1] >> 4) & 0x01;

  /* Unpack Member: m_nUseHiPriVlan */
  (pToStruct)->m_nUseHiPriVlan =  (uint8)  ((pFromData)[1] >> 3) & 0x01;

  /* Unpack Member: m_nUseDmacMatch */
  (pToStruct)->m_nUseDmacMatch =  (uint8)  ((pFromData)[1] >> 2) & 0x01;

  /* Unpack Member: m_nUseLayer4 */
  (pToStruct)->m_nUseLayer4 =  (uint8)  ((pFromData)[1] >> 1) & 0x01;

  /* Unpack Member: m_nFlowHashEnable */
  (pToStruct)->m_nFlowHashEnable =  (uint8)  ((pFromData)[1] ) & 0x01;

  /* Unpack Member: m_nUseHashCos */
  (pToStruct)->m_nUseHashCos =  (uint32)  (pFromData)[3] ;
  (pToStruct)->m_nUseHashCos |=  (uint32)  (pFromData)[2] << 8;
#else

  /* Unpack operation based on little endian */

  /* Unpack Member: m_nReserve */
  (pToStruct)->m_nReserve =  (uint32)  ((pFromData)[2] >> 7) & 0x01;
  (pToStruct)->m_nReserve |=  (uint32)  (pFromData)[3] << 1;

  /* Unpack Member: m_nUseUser1 */
  (pToStruct)->m_nUseUser1 =  (uint8)  ((pFromData)[2] >> 6) & 0x01;

  /* Unpack Member: m_nUseUser0 */
  (pToStruct)->m_nUseUser0 =  (uint8)  ((pFromData)[2] >> 5) & 0x01;

  /* Unpack Member: m_nUseVlanPri */
  (pToStruct)->m_nUseVlanPri =  (uint8)  ((pFromData)[2] >> 4) & 0x01;

  /* Unpack Member: m_nUseHiPriVlan */
  (pToStruct)->m_nUseHiPriVlan =  (uint8)  ((pFromData)[2] >> 3) & 0x01;

  /* Unpack Member: m_nUseDmacMatch */
  (pToStruct)->m_nUseDmacMatch =  (uint8)  ((pFromData)[2] >> 2) & 0x01;

  /* Unpack Member: m_nUseLayer4 */
  (pToStruct)->m_nUseLayer4 =  (uint8)  ((pFromData)[2] >> 1) & 0x01;

  /* Unpack Member: m_nFlowHashEnable */
  (pToStruct)->m_nFlowHashEnable =  (uint8)  ((pFromData)[2] ) & 0x01;

  /* Unpack Member: m_nUseHashCos */
  (pToStruct)->m_nUseHashCos =  (uint32)  (pFromData)[0] ;
  (pToStruct)->m_nUseHashCos |=  (uint32)  (pFromData)[1] << 8;
#endif

}



/* initialize an instance of this zframe */
void
sbZfKaRbClassPortEnablesEntry_InitInstance(sbZfKaRbClassPortEnablesEntry_t *pFrame) {

  pFrame->m_nReserve =  (unsigned int)  0;
  pFrame->m_nUseUser1 =  (unsigned int)  0;
  pFrame->m_nUseUser0 =  (unsigned int)  0;
  pFrame->m_nUseVlanPri =  (unsigned int)  0;
  pFrame->m_nUseHiPriVlan =  (unsigned int)  0;
  pFrame->m_nUseDmacMatch =  (unsigned int)  0;
  pFrame->m_nUseLayer4 =  (unsigned int)  0;
  pFrame->m_nFlowHashEnable =  (unsigned int)  0;
  pFrame->m_nUseHashCos =  (unsigned int)  0;

}
