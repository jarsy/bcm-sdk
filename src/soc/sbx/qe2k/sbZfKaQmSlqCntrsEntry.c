/*
 * $Id: sbZfKaQmSlqCntrsEntry.c,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


#include "sbTypes.h"
#include "sbZfKaQmSlqCntrsEntry.hx"
#include "sbWrappers.h"
#include <sal/types.h>



/* Pack from struct into array of bytes */
uint32
sbZfKaQmSlqCntrsEntry_Pack(sbZfKaQmSlqCntrsEntry_t *pFrom,
                           uint8 *pToData,
                           uint32 nMaxToDataIndex) {
#ifdef SAND_BIG_ENDIAN_HOST
  int i;
  int size = SB_ZF_ZFKAQMSLQCNTRSENTRY_SIZE_IN_BYTES;

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on bigword endian */

  /* Pack Member: m_nData1 */
  (pToData)[7] |= ((pFrom)->m_nData1) & 0xFF;
  (pToData)[6] |= ((pFrom)->m_nData1 >> 8) &0xFF;
  (pToData)[5] |= ((pFrom)->m_nData1 >> 16) &0xFF;
  (pToData)[4] |= ((pFrom)->m_nData1 >> 24) &0xFF;

  /* Pack Member: m_nData0 */
  (pToData)[3] |= ((pFrom)->m_nData0) & 0xFF;
  (pToData)[2] |= ((pFrom)->m_nData0 >> 8) &0xFF;
  (pToData)[1] |= ((pFrom)->m_nData0 >> 16) &0xFF;
  (pToData)[0] |= ((pFrom)->m_nData0 >> 24) &0xFF;
#else
  int i;
  int size = SB_ZF_ZFKAQMSLQCNTRSENTRY_SIZE_IN_BYTES;

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on little endian */

  /* Pack Member: m_nData1 */
  (pToData)[4] |= ((pFrom)->m_nData1) & 0xFF;
  (pToData)[5] |= ((pFrom)->m_nData1 >> 8) &0xFF;
  (pToData)[6] |= ((pFrom)->m_nData1 >> 16) &0xFF;
  (pToData)[7] |= ((pFrom)->m_nData1 >> 24) &0xFF;

  /* Pack Member: m_nData0 */
  (pToData)[0] |= ((pFrom)->m_nData0) & 0xFF;
  (pToData)[1] |= ((pFrom)->m_nData0 >> 8) &0xFF;
  (pToData)[2] |= ((pFrom)->m_nData0 >> 16) &0xFF;
  (pToData)[3] |= ((pFrom)->m_nData0 >> 24) &0xFF;
#endif

  return SB_ZF_ZFKAQMSLQCNTRSENTRY_SIZE_IN_BYTES;
}




/* Unpack from array of bytes into struct */
void
sbZfKaQmSlqCntrsEntry_Unpack(sbZfKaQmSlqCntrsEntry_t *pToStruct,
                             uint8 *pFromData,
                             uint32 nMaxToDataIndex) {
  COMPILER_UINT64 tmp;

  (void) tmp;

#ifdef SAND_BIG_ENDIAN_HOST

  /* Unpack operation based on bigword endian */

  /* Unpack Member: m_nData1 */
  (pToStruct)->m_nData1 =  (uint32)  (pFromData)[7] ;
  (pToStruct)->m_nData1 |=  (uint32)  (pFromData)[6] << 8;
  (pToStruct)->m_nData1 |=  (uint32)  (pFromData)[5] << 16;
  (pToStruct)->m_nData1 |=  (uint32)  (pFromData)[4] << 24;

  /* Unpack Member: m_nData0 */
  (pToStruct)->m_nData0 =  (uint32)  (pFromData)[3] ;
  (pToStruct)->m_nData0 |=  (uint32)  (pFromData)[2] << 8;
  (pToStruct)->m_nData0 |=  (uint32)  (pFromData)[1] << 16;
  (pToStruct)->m_nData0 |=  (uint32)  (pFromData)[0] << 24;
#else

  /* Unpack operation based on little endian */

  /* Unpack Member: m_nData1 */
  (pToStruct)->m_nData1 =  (uint32)  (pFromData)[4] ;
  (pToStruct)->m_nData1 |=  (uint32)  (pFromData)[5] << 8;
  (pToStruct)->m_nData1 |=  (uint32)  (pFromData)[6] << 16;
  (pToStruct)->m_nData1 |=  (uint32)  (pFromData)[7] << 24;

  /* Unpack Member: m_nData0 */
  (pToStruct)->m_nData0 =  (uint32)  (pFromData)[0] ;
  (pToStruct)->m_nData0 |=  (uint32)  (pFromData)[1] << 8;
  (pToStruct)->m_nData0 |=  (uint32)  (pFromData)[2] << 16;
  (pToStruct)->m_nData0 |=  (uint32)  (pFromData)[3] << 24;
#endif

}



/* initialize an instance of this zframe */
void
sbZfKaQmSlqCntrsEntry_InitInstance(sbZfKaQmSlqCntrsEntry_t *pFrame) {

  pFrame->m_nData1 =  (unsigned int)  0;
  pFrame->m_nData0 =  (unsigned int)  0;

}
