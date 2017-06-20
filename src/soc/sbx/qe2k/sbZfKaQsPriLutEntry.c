/*
 * $Id: sbZfKaQsPriLutEntry.c,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


#include "sbTypes.h"
#include "sbZfKaQsPriLutEntry.hx"
#include "sbWrappers.h"
#include <sal/types.h>



/* Pack from struct into array of bytes */
uint32
sbZfKaQsPriLutEntry_Pack(sbZfKaQsPriLutEntry_t *pFrom,
                         uint8 *pToData,
                         uint32 nMaxToDataIndex) {
#ifdef SAND_BIG_ENDIAN_HOST
  int i;
  int size = SB_ZF_ZFKAQSPRILUTENTRY_SIZE_IN_BYTES;

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on bigword endian */

  /* Pack Member: m_nReserved */
  (pToData)[2] |= ((pFrom)->m_nReserved) & 0xFF;
  (pToData)[1] |= ((pFrom)->m_nReserved >> 8) &0xFF;
  (pToData)[0] |= ((pFrom)->m_nReserved >> 16) &0xFF;

  /* Pack Member: m_nCPri */
  (pToData)[3] |= ((pFrom)->m_nCPri & 0x0f) <<4;

  /* Pack Member: m_nNPri */
  (pToData)[3] |= ((pFrom)->m_nNPri & 0x0f);
#else
  int i;
  int size = SB_ZF_ZFKAQSPRILUTENTRY_SIZE_IN_BYTES;

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on little endian */

  /* Pack Member: m_nReserved */
  (pToData)[1] |= ((pFrom)->m_nReserved) & 0xFF;
  (pToData)[2] |= ((pFrom)->m_nReserved >> 8) &0xFF;
  (pToData)[3] |= ((pFrom)->m_nReserved >> 16) &0xFF;

  /* Pack Member: m_nCPri */
  (pToData)[0] |= ((pFrom)->m_nCPri & 0x0f) <<4;

  /* Pack Member: m_nNPri */
  (pToData)[0] |= ((pFrom)->m_nNPri & 0x0f);
#endif

  return SB_ZF_ZFKAQSPRILUTENTRY_SIZE_IN_BYTES;
}




/* Unpack from array of bytes into struct */
void
sbZfKaQsPriLutEntry_Unpack(sbZfKaQsPriLutEntry_t *pToStruct,
                           uint8 *pFromData,
                           uint32 nMaxToDataIndex) {
  COMPILER_UINT64 tmp;

  (void) tmp;

#ifdef SAND_BIG_ENDIAN_HOST

  /* Unpack operation based on bigword endian */

  /* Unpack Member: m_nReserved */
  (pToStruct)->m_nReserved =  (uint32)  (pFromData)[2] ;
  (pToStruct)->m_nReserved |=  (uint32)  (pFromData)[1] << 8;
  (pToStruct)->m_nReserved |=  (uint32)  (pFromData)[0] << 16;

  /* Unpack Member: m_nCPri */
  (pToStruct)->m_nCPri =  (uint32)  ((pFromData)[3] >> 4) & 0x0f;

  /* Unpack Member: m_nNPri */
  (pToStruct)->m_nNPri =  (uint32)  ((pFromData)[3] ) & 0x0f;
#else

  /* Unpack operation based on little endian */

  /* Unpack Member: m_nReserved */
  (pToStruct)->m_nReserved =  (uint32)  (pFromData)[1] ;
  (pToStruct)->m_nReserved |=  (uint32)  (pFromData)[2] << 8;
  (pToStruct)->m_nReserved |=  (uint32)  (pFromData)[3] << 16;

  /* Unpack Member: m_nCPri */
  (pToStruct)->m_nCPri =  (uint32)  ((pFromData)[0] >> 4) & 0x0f;

  /* Unpack Member: m_nNPri */
  (pToStruct)->m_nNPri =  (uint32)  ((pFromData)[0] ) & 0x0f;
#endif

}



/* initialize an instance of this zframe */
void
sbZfKaQsPriLutEntry_InitInstance(sbZfKaQsPriLutEntry_t *pFrame) {

  pFrame->m_nReserved =  (unsigned int)  0;
  pFrame->m_nCPri =  (unsigned int)  0;
  pFrame->m_nNPri =  (unsigned int)  0;

}
