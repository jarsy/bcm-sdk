/*
 * $Id: sbZfKaEpVlanIndTableEntry.c,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


#include "sbTypes.h"
#include "sbZfKaEpVlanIndTableEntry.hx"
#include "sbWrappers.h"
#include <sal/types.h>



/* Pack from struct into array of bytes */
uint32
sbZfKaEpVlanIndTableEntry_Pack(sbZfKaEpVlanIndTableEntry_t *pFrom,
                               uint8 *pToData,
                               uint32 nMaxToDataIndex) {
#ifdef SAND_BIG_ENDIAN_HOST
  int i;
  int size = SB_ZF_ZFKAEPVLANINDTABLEENTRY_SIZE_IN_BYTES;

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on bigword endian */

  /* Pack Member: m_nRecord3 */
  (pToData)[5] |= ((pFrom)->m_nRecord3) & 0xFF;
  (pToData)[4] |= ((pFrom)->m_nRecord3 >> 8) &0xFF;

  /* Pack Member: m_nRecord2 */
  (pToData)[7] |= ((pFrom)->m_nRecord2) & 0xFF;
  (pToData)[6] |= ((pFrom)->m_nRecord2 >> 8) &0xFF;

  /* Pack Member: m_nRecord1 */
  (pToData)[1] |= ((pFrom)->m_nRecord1) & 0xFF;
  (pToData)[0] |= ((pFrom)->m_nRecord1 >> 8) &0xFF;

  /* Pack Member: m_nRecord0 */
  (pToData)[3] |= ((pFrom)->m_nRecord0) & 0xFF;
  (pToData)[2] |= ((pFrom)->m_nRecord0 >> 8) &0xFF;
#else
  int i;
  int size = SB_ZF_ZFKAEPVLANINDTABLEENTRY_SIZE_IN_BYTES;

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on little endian */

  /* Pack Member: m_nRecord3 */
  (pToData)[6] |= ((pFrom)->m_nRecord3) & 0xFF;
  (pToData)[7] |= ((pFrom)->m_nRecord3 >> 8) &0xFF;

  /* Pack Member: m_nRecord2 */
  (pToData)[4] |= ((pFrom)->m_nRecord2) & 0xFF;
  (pToData)[5] |= ((pFrom)->m_nRecord2 >> 8) &0xFF;

  /* Pack Member: m_nRecord1 */
  (pToData)[2] |= ((pFrom)->m_nRecord1) & 0xFF;
  (pToData)[3] |= ((pFrom)->m_nRecord1 >> 8) &0xFF;

  /* Pack Member: m_nRecord0 */
  (pToData)[0] |= ((pFrom)->m_nRecord0) & 0xFF;
  (pToData)[1] |= ((pFrom)->m_nRecord0 >> 8) &0xFF;
#endif

  return SB_ZF_ZFKAEPVLANINDTABLEENTRY_SIZE_IN_BYTES;
}




/* Unpack from array of bytes into struct */
void
sbZfKaEpVlanIndTableEntry_Unpack(sbZfKaEpVlanIndTableEntry_t *pToStruct,
                                 uint8 *pFromData,
                                 uint32 nMaxToDataIndex) {
  COMPILER_UINT64 tmp;

  (void) tmp;

#ifdef SAND_BIG_ENDIAN_HOST

  /* Unpack operation based on bigword endian */

  /* Unpack Member: m_nRecord3 */
  (pToStruct)->m_nRecord3 =  (uint32)  (pFromData)[5] ;
  (pToStruct)->m_nRecord3 |=  (uint32)  (pFromData)[4] << 8;

  /* Unpack Member: m_nRecord2 */
  (pToStruct)->m_nRecord2 =  (uint32)  (pFromData)[7] ;
  (pToStruct)->m_nRecord2 |=  (uint32)  (pFromData)[6] << 8;

  /* Unpack Member: m_nRecord1 */
  (pToStruct)->m_nRecord1 =  (uint32)  (pFromData)[1] ;
  (pToStruct)->m_nRecord1 |=  (uint32)  (pFromData)[0] << 8;

  /* Unpack Member: m_nRecord0 */
  (pToStruct)->m_nRecord0 =  (uint32)  (pFromData)[3] ;
  (pToStruct)->m_nRecord0 |=  (uint32)  (pFromData)[2] << 8;
#else

  /* Unpack operation based on little endian */

  /* Unpack Member: m_nRecord3 */
  (pToStruct)->m_nRecord3 =  (uint32)  (pFromData)[6] ;
  (pToStruct)->m_nRecord3 |=  (uint32)  (pFromData)[7] << 8;

  /* Unpack Member: m_nRecord2 */
  (pToStruct)->m_nRecord2 =  (uint32)  (pFromData)[4] ;
  (pToStruct)->m_nRecord2 |=  (uint32)  (pFromData)[5] << 8;

  /* Unpack Member: m_nRecord1 */
  (pToStruct)->m_nRecord1 =  (uint32)  (pFromData)[2] ;
  (pToStruct)->m_nRecord1 |=  (uint32)  (pFromData)[3] << 8;

  /* Unpack Member: m_nRecord0 */
  (pToStruct)->m_nRecord0 =  (uint32)  (pFromData)[0] ;
  (pToStruct)->m_nRecord0 |=  (uint32)  (pFromData)[1] << 8;
#endif

}



/* initialize an instance of this zframe */
void
sbZfKaEpVlanIndTableEntry_InitInstance(sbZfKaEpVlanIndTableEntry_t *pFrame) {

  pFrame->m_nRecord3 =  (unsigned int)  0;
  pFrame->m_nRecord2 =  (unsigned int)  0;
  pFrame->m_nRecord1 =  (unsigned int)  0;
  pFrame->m_nRecord0 =  (unsigned int)  0;

}
