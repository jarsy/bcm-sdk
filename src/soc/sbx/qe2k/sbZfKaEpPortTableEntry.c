/*
 * $Id: sbZfKaEpPortTableEntry.c,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


#include "sbTypes.h"
#include "sbZfKaEpPortTableEntry.hx"
#include "sbWrappers.h"
#include <sal/types.h>



/* Pack from struct into array of bytes */
uint32
sbZfKaEpPortTableEntry_Pack(sbZfKaEpPortTableEntry_t *pFrom,
                            uint8 *pToData,
                            uint32 nMaxToDataIndex) {
#ifdef SAND_BIG_ENDIAN_HOST
  int i;
  int size = SB_ZF_ZFKAEPPORTTABLEENTRY_SIZE_IN_BYTES;

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on bigword endian */

  /* Pack Member: m_nEnable15 */
  (pToData)[4] |= ((pFrom)->m_nEnable15 & 0x01) <<7;

  /* Pack Member: m_nEnable14 */
  (pToData)[4] |= ((pFrom)->m_nEnable14 & 0x01) <<6;

  /* Pack Member: m_nEnable13 */
  (pToData)[4] |= ((pFrom)->m_nEnable13 & 0x01) <<5;

  /* Pack Member: m_nEnable12 */
  (pToData)[4] |= ((pFrom)->m_nEnable12 & 0x01) <<4;

  /* Pack Member: m_nEnable11 */
  (pToData)[4] |= ((pFrom)->m_nEnable11 & 0x01) <<3;

  /* Pack Member: m_nEnable10 */
  (pToData)[4] |= ((pFrom)->m_nEnable10 & 0x01) <<2;

  /* Pack Member: m_nEnable9 */
  (pToData)[4] |= ((pFrom)->m_nEnable9 & 0x01) <<1;

  /* Pack Member: m_nEnable8 */
  (pToData)[4] |= ((pFrom)->m_nEnable8 & 0x01);

  /* Pack Member: m_nEnable7 */
  (pToData)[5] |= ((pFrom)->m_nEnable7 & 0x01) <<7;

  /* Pack Member: m_nEnable6 */
  (pToData)[5] |= ((pFrom)->m_nEnable6 & 0x01) <<6;

  /* Pack Member: m_nEnable5 */
  (pToData)[5] |= ((pFrom)->m_nEnable5 & 0x01) <<5;

  /* Pack Member: m_nEnable4 */
  (pToData)[5] |= ((pFrom)->m_nEnable4 & 0x01) <<4;

  /* Pack Member: m_nEnable3 */
  (pToData)[5] |= ((pFrom)->m_nEnable3 & 0x01) <<3;

  /* Pack Member: m_nEnable2 */
  (pToData)[5] |= ((pFrom)->m_nEnable2 & 0x01) <<2;

  /* Pack Member: m_nEnable1 */
  (pToData)[5] |= ((pFrom)->m_nEnable1 & 0x01) <<1;

  /* Pack Member: m_nEnable0 */
  (pToData)[5] |= ((pFrom)->m_nEnable0 & 0x01);

  /* Pack Member: m_nReserved0 */
  (pToData)[6] |= ((pFrom)->m_nReserved0 & 0x07) <<5;

  /* Pack Member: m_nCountTrans */
  (pToData)[6] |= ((pFrom)->m_nCountTrans & 0x1f);

  /* Pack Member: m_nReserved1 */
  (pToData)[7] |= ((pFrom)->m_nReserved1 & 0x7f) <<1;

  /* Pack Member: m_nPrepend */
  (pToData)[7] |= ((pFrom)->m_nPrepend & 0x01);

  /* Pack Member: m_Instruction */
  (pToData)[3] |= ((pFrom)->m_Instruction) & 0xFF;
  (pToData)[2] |= ((pFrom)->m_Instruction >> 8) &0xFF;
  (pToData)[1] |= ((pFrom)->m_Instruction >> 16) &0xFF;
  (pToData)[0] |= ((pFrom)->m_Instruction >> 24) &0xFF;
#else
  int i;
  int size = SB_ZF_ZFKAEPPORTTABLEENTRY_SIZE_IN_BYTES;

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on little endian */

  /* Pack Member: m_nEnable15 */
  (pToData)[7] |= ((pFrom)->m_nEnable15 & 0x01) <<7;

  /* Pack Member: m_nEnable14 */
  (pToData)[7] |= ((pFrom)->m_nEnable14 & 0x01) <<6;

  /* Pack Member: m_nEnable13 */
  (pToData)[7] |= ((pFrom)->m_nEnable13 & 0x01) <<5;

  /* Pack Member: m_nEnable12 */
  (pToData)[7] |= ((pFrom)->m_nEnable12 & 0x01) <<4;

  /* Pack Member: m_nEnable11 */
  (pToData)[7] |= ((pFrom)->m_nEnable11 & 0x01) <<3;

  /* Pack Member: m_nEnable10 */
  (pToData)[7] |= ((pFrom)->m_nEnable10 & 0x01) <<2;

  /* Pack Member: m_nEnable9 */
  (pToData)[7] |= ((pFrom)->m_nEnable9 & 0x01) <<1;

  /* Pack Member: m_nEnable8 */
  (pToData)[7] |= ((pFrom)->m_nEnable8 & 0x01);

  /* Pack Member: m_nEnable7 */
  (pToData)[6] |= ((pFrom)->m_nEnable7 & 0x01) <<7;

  /* Pack Member: m_nEnable6 */
  (pToData)[6] |= ((pFrom)->m_nEnable6 & 0x01) <<6;

  /* Pack Member: m_nEnable5 */
  (pToData)[6] |= ((pFrom)->m_nEnable5 & 0x01) <<5;

  /* Pack Member: m_nEnable4 */
  (pToData)[6] |= ((pFrom)->m_nEnable4 & 0x01) <<4;

  /* Pack Member: m_nEnable3 */
  (pToData)[6] |= ((pFrom)->m_nEnable3 & 0x01) <<3;

  /* Pack Member: m_nEnable2 */
  (pToData)[6] |= ((pFrom)->m_nEnable2 & 0x01) <<2;

  /* Pack Member: m_nEnable1 */
  (pToData)[6] |= ((pFrom)->m_nEnable1 & 0x01) <<1;

  /* Pack Member: m_nEnable0 */
  (pToData)[6] |= ((pFrom)->m_nEnable0 & 0x01);

  /* Pack Member: m_nReserved0 */
  (pToData)[5] |= ((pFrom)->m_nReserved0 & 0x07) <<5;

  /* Pack Member: m_nCountTrans */
  (pToData)[5] |= ((pFrom)->m_nCountTrans & 0x1f);

  /* Pack Member: m_nReserved1 */
  (pToData)[4] |= ((pFrom)->m_nReserved1 & 0x7f) <<1;

  /* Pack Member: m_nPrepend */
  (pToData)[4] |= ((pFrom)->m_nPrepend & 0x01);

  /* Pack Member: m_Instruction */
  (pToData)[0] |= ((pFrom)->m_Instruction) & 0xFF;
  (pToData)[1] |= ((pFrom)->m_Instruction >> 8) &0xFF;
  (pToData)[2] |= ((pFrom)->m_Instruction >> 16) &0xFF;
  (pToData)[3] |= ((pFrom)->m_Instruction >> 24) &0xFF;
#endif

  return SB_ZF_ZFKAEPPORTTABLEENTRY_SIZE_IN_BYTES;
}




/* Unpack from array of bytes into struct */
void
sbZfKaEpPortTableEntry_Unpack(sbZfKaEpPortTableEntry_t *pToStruct,
                              uint8 *pFromData,
                              uint32 nMaxToDataIndex) {
  COMPILER_UINT64 tmp;

  (void) tmp;

#ifdef SAND_BIG_ENDIAN_HOST

  /* Unpack operation based on bigword endian */

  /* Unpack Member: m_nEnable15 */
  (pToStruct)->m_nEnable15 =  (uint8)  ((pFromData)[4] >> 7) & 0x01;

  /* Unpack Member: m_nEnable14 */
  (pToStruct)->m_nEnable14 =  (uint8)  ((pFromData)[4] >> 6) & 0x01;

  /* Unpack Member: m_nEnable13 */
  (pToStruct)->m_nEnable13 =  (uint8)  ((pFromData)[4] >> 5) & 0x01;

  /* Unpack Member: m_nEnable12 */
  (pToStruct)->m_nEnable12 =  (uint8)  ((pFromData)[4] >> 4) & 0x01;

  /* Unpack Member: m_nEnable11 */
  (pToStruct)->m_nEnable11 =  (uint8)  ((pFromData)[4] >> 3) & 0x01;

  /* Unpack Member: m_nEnable10 */
  (pToStruct)->m_nEnable10 =  (uint8)  ((pFromData)[4] >> 2) & 0x01;

  /* Unpack Member: m_nEnable9 */
  (pToStruct)->m_nEnable9 =  (uint8)  ((pFromData)[4] >> 1) & 0x01;

  /* Unpack Member: m_nEnable8 */
  (pToStruct)->m_nEnable8 =  (uint8)  ((pFromData)[4] ) & 0x01;

  /* Unpack Member: m_nEnable7 */
  (pToStruct)->m_nEnable7 =  (uint8)  ((pFromData)[5] >> 7) & 0x01;

  /* Unpack Member: m_nEnable6 */
  (pToStruct)->m_nEnable6 =  (uint8)  ((pFromData)[5] >> 6) & 0x01;

  /* Unpack Member: m_nEnable5 */
  (pToStruct)->m_nEnable5 =  (uint8)  ((pFromData)[5] >> 5) & 0x01;

  /* Unpack Member: m_nEnable4 */
  (pToStruct)->m_nEnable4 =  (uint8)  ((pFromData)[5] >> 4) & 0x01;

  /* Unpack Member: m_nEnable3 */
  (pToStruct)->m_nEnable3 =  (uint8)  ((pFromData)[5] >> 3) & 0x01;

  /* Unpack Member: m_nEnable2 */
  (pToStruct)->m_nEnable2 =  (uint8)  ((pFromData)[5] >> 2) & 0x01;

  /* Unpack Member: m_nEnable1 */
  (pToStruct)->m_nEnable1 =  (uint8)  ((pFromData)[5] >> 1) & 0x01;

  /* Unpack Member: m_nEnable0 */
  (pToStruct)->m_nEnable0 =  (uint8)  ((pFromData)[5] ) & 0x01;

  /* Unpack Member: m_nReserved0 */
  (pToStruct)->m_nReserved0 =  (int32)  ((pFromData)[6] >> 5) & 0x07;

  /* Unpack Member: m_nCountTrans */
  (pToStruct)->m_nCountTrans =  (int32)  ((pFromData)[6] ) & 0x1f;

  /* Unpack Member: m_nReserved1 */
  (pToStruct)->m_nReserved1 =  (int32)  ((pFromData)[7] >> 1) & 0x7f;

  /* Unpack Member: m_nPrepend */
  (pToStruct)->m_nPrepend =  (int32)  ((pFromData)[7] ) & 0x01;

  /* Unpack Member: m_Instruction */
  (pToStruct)->m_Instruction =  (uint32)  (pFromData)[3] ;
  (pToStruct)->m_Instruction |=  (uint32)  (pFromData)[2] << 8;
  (pToStruct)->m_Instruction |=  (uint32)  (pFromData)[1] << 16;
  (pToStruct)->m_Instruction |=  (uint32)  (pFromData)[0] << 24;
#else

  /* Unpack operation based on little endian */

  /* Unpack Member: m_nEnable15 */
  (pToStruct)->m_nEnable15 =  (uint8)  ((pFromData)[7] >> 7) & 0x01;

  /* Unpack Member: m_nEnable14 */
  (pToStruct)->m_nEnable14 =  (uint8)  ((pFromData)[7] >> 6) & 0x01;

  /* Unpack Member: m_nEnable13 */
  (pToStruct)->m_nEnable13 =  (uint8)  ((pFromData)[7] >> 5) & 0x01;

  /* Unpack Member: m_nEnable12 */
  (pToStruct)->m_nEnable12 =  (uint8)  ((pFromData)[7] >> 4) & 0x01;

  /* Unpack Member: m_nEnable11 */
  (pToStruct)->m_nEnable11 =  (uint8)  ((pFromData)[7] >> 3) & 0x01;

  /* Unpack Member: m_nEnable10 */
  (pToStruct)->m_nEnable10 =  (uint8)  ((pFromData)[7] >> 2) & 0x01;

  /* Unpack Member: m_nEnable9 */
  (pToStruct)->m_nEnable9 =  (uint8)  ((pFromData)[7] >> 1) & 0x01;

  /* Unpack Member: m_nEnable8 */
  (pToStruct)->m_nEnable8 =  (uint8)  ((pFromData)[7] ) & 0x01;

  /* Unpack Member: m_nEnable7 */
  (pToStruct)->m_nEnable7 =  (uint8)  ((pFromData)[6] >> 7) & 0x01;

  /* Unpack Member: m_nEnable6 */
  (pToStruct)->m_nEnable6 =  (uint8)  ((pFromData)[6] >> 6) & 0x01;

  /* Unpack Member: m_nEnable5 */
  (pToStruct)->m_nEnable5 =  (uint8)  ((pFromData)[6] >> 5) & 0x01;

  /* Unpack Member: m_nEnable4 */
  (pToStruct)->m_nEnable4 =  (uint8)  ((pFromData)[6] >> 4) & 0x01;

  /* Unpack Member: m_nEnable3 */
  (pToStruct)->m_nEnable3 =  (uint8)  ((pFromData)[6] >> 3) & 0x01;

  /* Unpack Member: m_nEnable2 */
  (pToStruct)->m_nEnable2 =  (uint8)  ((pFromData)[6] >> 2) & 0x01;

  /* Unpack Member: m_nEnable1 */
  (pToStruct)->m_nEnable1 =  (uint8)  ((pFromData)[6] >> 1) & 0x01;

  /* Unpack Member: m_nEnable0 */
  (pToStruct)->m_nEnable0 =  (uint8)  ((pFromData)[6] ) & 0x01;

  /* Unpack Member: m_nReserved0 */
  (pToStruct)->m_nReserved0 =  (int32)  ((pFromData)[5] >> 5) & 0x07;

  /* Unpack Member: m_nCountTrans */
  (pToStruct)->m_nCountTrans =  (int32)  ((pFromData)[5] ) & 0x1f;

  /* Unpack Member: m_nReserved1 */
  (pToStruct)->m_nReserved1 =  (int32)  ((pFromData)[4] >> 1) & 0x7f;

  /* Unpack Member: m_nPrepend */
  (pToStruct)->m_nPrepend =  (int32)  ((pFromData)[4] ) & 0x01;

  /* Unpack Member: m_Instruction */
  (pToStruct)->m_Instruction =  (uint32)  (pFromData)[0] ;
  (pToStruct)->m_Instruction |=  (uint32)  (pFromData)[1] << 8;
  (pToStruct)->m_Instruction |=  (uint32)  (pFromData)[2] << 16;
  (pToStruct)->m_Instruction |=  (uint32)  (pFromData)[3] << 24;
#endif

}



/* initialize an instance of this zframe */
void
sbZfKaEpPortTableEntry_InitInstance(sbZfKaEpPortTableEntry_t *pFrame) {

  pFrame->m_nEnable15 =  (unsigned int)  0;
  pFrame->m_nEnable14 =  (unsigned int)  0;
  pFrame->m_nEnable13 =  (unsigned int)  0;
  pFrame->m_nEnable12 =  (unsigned int)  0;
  pFrame->m_nEnable11 =  (unsigned int)  0;
  pFrame->m_nEnable10 =  (unsigned int)  0;
  pFrame->m_nEnable9 =  (unsigned int)  0;
  pFrame->m_nEnable8 =  (unsigned int)  0;
  pFrame->m_nEnable7 =  (unsigned int)  0;
  pFrame->m_nEnable6 =  (unsigned int)  0;
  pFrame->m_nEnable5 =  (unsigned int)  0;
  pFrame->m_nEnable4 =  (unsigned int)  0;
  pFrame->m_nEnable3 =  (unsigned int)  0;
  pFrame->m_nEnable2 =  (unsigned int)  0;
  pFrame->m_nEnable1 =  (unsigned int)  0;
  pFrame->m_nEnable0 =  (unsigned int)  0;
  pFrame->m_nReserved0 =  (unsigned int)  0;
  pFrame->m_nCountTrans =  (unsigned int)  0;
  pFrame->m_nReserved1 =  (unsigned int)  0;
  pFrame->m_nPrepend =  (unsigned int)  0;
  pFrame->m_Instruction =  (unsigned int)  0;

}
