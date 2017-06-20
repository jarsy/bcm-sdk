/*
 * $Id: sbZfKaQmWredParamEntry.c,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


#include "sbTypes.h"
#include "sbZfKaQmWredParamEntry.hx"
#include "sbWrappers.h"
#include <sal/types.h>



/* Pack from struct into array of bytes */
uint32
sbZfKaQmWredParamEntry_Pack(sbZfKaQmWredParamEntry_t *pFrom,
                            uint8 *pToData,
                            uint32 nMaxToDataIndex) {
#ifdef SAND_BIG_ENDIAN_HOST
  int i;
  int size = SB_ZF_ZFKAQMWREDPARAMENTRY_SIZE_IN_BYTES;

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on bigword endian */

  /* Pack Member: m_nTMaxExceeded2 */
  (pToData)[7] |= ((pFrom)->m_nTMaxExceeded2 & 0x01) <<3;

  /* Pack Member: m_nEcnExceeded2 */
  (pToData)[7] |= ((pFrom)->m_nEcnExceeded2 & 0x01) <<2;

  /* Pack Member: m_nPDrop2 */
  (pToData)[0] |= ((pFrom)->m_nPDrop2) & 0xFF;
  (pToData)[7] |= ((pFrom)->m_nPDrop2 >> 8) & 0x03;

  /* Pack Member: m_nTMaxExceeded1 */
  (pToData)[1] |= ((pFrom)->m_nTMaxExceeded1 & 0x01) <<7;

  /* Pack Member: m_nEcnExceeded1 */
  (pToData)[1] |= ((pFrom)->m_nEcnExceeded1 & 0x01) <<6;

  /* Pack Member: m_nPDrop1 */
  (pToData)[2] |= ((pFrom)->m_nPDrop1 & 0x0f) <<4;
  (pToData)[1] |= ((pFrom)->m_nPDrop1 >> 4) & 0x3f;

  /* Pack Member: m_nTMaxExceeded0 */
  (pToData)[2] |= ((pFrom)->m_nTMaxExceeded0 & 0x01) <<3;

  /* Pack Member: m_nEcnExceeded0 */
  (pToData)[2] |= ((pFrom)->m_nEcnExceeded0 & 0x01) <<2;

  /* Pack Member: m_nPDrop0 */
  (pToData)[3] |= ((pFrom)->m_nPDrop0) & 0xFF;
  (pToData)[2] |= ((pFrom)->m_nPDrop0 >> 8) & 0x03;
#else
  int i;
  int size = SB_ZF_ZFKAQMWREDPARAMENTRY_SIZE_IN_BYTES;

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on little endian */

  /* Pack Member: m_nTMaxExceeded2 */
  (pToData)[4] |= ((pFrom)->m_nTMaxExceeded2 & 0x01) <<3;

  /* Pack Member: m_nEcnExceeded2 */
  (pToData)[4] |= ((pFrom)->m_nEcnExceeded2 & 0x01) <<2;

  /* Pack Member: m_nPDrop2 */
  (pToData)[3] |= ((pFrom)->m_nPDrop2) & 0xFF;
  (pToData)[4] |= ((pFrom)->m_nPDrop2 >> 8) & 0x03;

  /* Pack Member: m_nTMaxExceeded1 */
  (pToData)[2] |= ((pFrom)->m_nTMaxExceeded1 & 0x01) <<7;

  /* Pack Member: m_nEcnExceeded1 */
  (pToData)[2] |= ((pFrom)->m_nEcnExceeded1 & 0x01) <<6;

  /* Pack Member: m_nPDrop1 */
  (pToData)[1] |= ((pFrom)->m_nPDrop1 & 0x0f) <<4;
  (pToData)[2] |= ((pFrom)->m_nPDrop1 >> 4) & 0x3f;

  /* Pack Member: m_nTMaxExceeded0 */
  (pToData)[1] |= ((pFrom)->m_nTMaxExceeded0 & 0x01) <<3;

  /* Pack Member: m_nEcnExceeded0 */
  (pToData)[1] |= ((pFrom)->m_nEcnExceeded0 & 0x01) <<2;

  /* Pack Member: m_nPDrop0 */
  (pToData)[0] |= ((pFrom)->m_nPDrop0) & 0xFF;
  (pToData)[1] |= ((pFrom)->m_nPDrop0 >> 8) & 0x03;
#endif

  return SB_ZF_ZFKAQMWREDPARAMENTRY_SIZE_IN_BYTES;
}




/* Unpack from array of bytes into struct */
void
sbZfKaQmWredParamEntry_Unpack(sbZfKaQmWredParamEntry_t *pToStruct,
                              uint8 *pFromData,
                              uint32 nMaxToDataIndex) {
  COMPILER_UINT64 tmp;

  (void) tmp;

#ifdef SAND_BIG_ENDIAN_HOST

  /* Unpack operation based on bigword endian */

  /* Unpack Member: m_nTMaxExceeded2 */
  (pToStruct)->m_nTMaxExceeded2 =  (uint8)  ((pFromData)[7] >> 3) & 0x01;

  /* Unpack Member: m_nEcnExceeded2 */
  (pToStruct)->m_nEcnExceeded2 =  (uint8)  ((pFromData)[7] >> 2) & 0x01;

  /* Unpack Member: m_nPDrop2 */
  (pToStruct)->m_nPDrop2 =  (uint32)  (pFromData)[0] ;
  (pToStruct)->m_nPDrop2 |=  (uint32)  ((pFromData)[7] & 0x03) << 8;

  /* Unpack Member: m_nTMaxExceeded1 */
  (pToStruct)->m_nTMaxExceeded1 =  (uint8)  ((pFromData)[1] >> 7) & 0x01;

  /* Unpack Member: m_nEcnExceeded1 */
  (pToStruct)->m_nEcnExceeded1 =  (uint8)  ((pFromData)[1] >> 6) & 0x01;

  /* Unpack Member: m_nPDrop1 */
  (pToStruct)->m_nPDrop1 =  (uint32)  ((pFromData)[2] >> 4) & 0x0f;
  (pToStruct)->m_nPDrop1 |=  (uint32)  ((pFromData)[1] & 0x3f) << 4;

  /* Unpack Member: m_nTMaxExceeded0 */
  (pToStruct)->m_nTMaxExceeded0 =  (uint8)  ((pFromData)[2] >> 3) & 0x01;

  /* Unpack Member: m_nEcnExceeded0 */
  (pToStruct)->m_nEcnExceeded0 =  (uint8)  ((pFromData)[2] >> 2) & 0x01;

  /* Unpack Member: m_nPDrop0 */
  (pToStruct)->m_nPDrop0 =  (uint32)  (pFromData)[3] ;
  (pToStruct)->m_nPDrop0 |=  (uint32)  ((pFromData)[2] & 0x03) << 8;
#else

  /* Unpack operation based on little endian */

  /* Unpack Member: m_nTMaxExceeded2 */
  (pToStruct)->m_nTMaxExceeded2 =  (uint8)  ((pFromData)[4] >> 3) & 0x01;

  /* Unpack Member: m_nEcnExceeded2 */
  (pToStruct)->m_nEcnExceeded2 =  (uint8)  ((pFromData)[4] >> 2) & 0x01;

  /* Unpack Member: m_nPDrop2 */
  (pToStruct)->m_nPDrop2 =  (uint32)  (pFromData)[3] ;
  (pToStruct)->m_nPDrop2 |=  (uint32)  ((pFromData)[4] & 0x03) << 8;

  /* Unpack Member: m_nTMaxExceeded1 */
  (pToStruct)->m_nTMaxExceeded1 =  (uint8)  ((pFromData)[2] >> 7) & 0x01;

  /* Unpack Member: m_nEcnExceeded1 */
  (pToStruct)->m_nEcnExceeded1 =  (uint8)  ((pFromData)[2] >> 6) & 0x01;

  /* Unpack Member: m_nPDrop1 */
  (pToStruct)->m_nPDrop1 =  (uint32)  ((pFromData)[1] >> 4) & 0x0f;
  (pToStruct)->m_nPDrop1 |=  (uint32)  ((pFromData)[2] & 0x3f) << 4;

  /* Unpack Member: m_nTMaxExceeded0 */
  (pToStruct)->m_nTMaxExceeded0 =  (uint8)  ((pFromData)[1] >> 3) & 0x01;

  /* Unpack Member: m_nEcnExceeded0 */
  (pToStruct)->m_nEcnExceeded0 =  (uint8)  ((pFromData)[1] >> 2) & 0x01;

  /* Unpack Member: m_nPDrop0 */
  (pToStruct)->m_nPDrop0 =  (uint32)  (pFromData)[0] ;
  (pToStruct)->m_nPDrop0 |=  (uint32)  ((pFromData)[1] & 0x03) << 8;
#endif

}



/* initialize an instance of this zframe */
void
sbZfKaQmWredParamEntry_InitInstance(sbZfKaQmWredParamEntry_t *pFrame) {

  pFrame->m_nTMaxExceeded2 =  (unsigned int)  0;
  pFrame->m_nEcnExceeded2 =  (unsigned int)  0;
  pFrame->m_nPDrop2 =  (unsigned int)  0;
  pFrame->m_nTMaxExceeded1 =  (unsigned int)  0;
  pFrame->m_nEcnExceeded1 =  (unsigned int)  0;
  pFrame->m_nPDrop1 =  (unsigned int)  0;
  pFrame->m_nTMaxExceeded0 =  (unsigned int)  0;
  pFrame->m_nEcnExceeded0 =  (unsigned int)  0;
  pFrame->m_nPDrop0 =  (unsigned int)  0;

}
