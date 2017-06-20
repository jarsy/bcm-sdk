/*
 * $Id: sbZfKaPmLastLine.c,v 1.3 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


#include "sbTypes.h"
#include "sbZfKaPmLastLine.hx"
#include "sbWrappers.h"
#include <sal/types.h>



/* Pack from struct into array of bytes */
uint32
sbZfKaPmLastLine_Pack(sbZfKaPmLastLine_t *pFrom,
                      uint8 *pToData,
                      uint32 nMaxToDataIndex) {
#ifdef SAND_BIG_ENDIAN_HOST
  int i;
  int size = SB_ZF_ZFKAPMLASTLINE_SIZE_IN_BYTES;

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on bigword endian */

  /* Pack Member: m_nReserved */
  (pToData)[11] |= (COMPILER_64_LO((pFrom)->m_nReserved) & 0x0f) <<4;
  (pToData)[10] |= (COMPILER_64_LO((pFrom)->m_nReserved) >> 4) &0xFF;
  (pToData)[9]  |= (COMPILER_64_LO((pFrom)->m_nReserved) >> 12) &0xFF;
  (pToData)[8]  |= (COMPILER_64_LO((pFrom)->m_nReserved) >> 20) &0xFF;
  (pToData)[15] |= (COMPILER_64_LO((pFrom)->m_nReserved) >> 28) &0x0F;
  (pToData)[15] |= (COMPILER_64_HI((pFrom)->m_nReserved) & 0x0f) <<4;
  (pToData)[14] |= (COMPILER_64_HI((pFrom)->m_nReserved) >> 4) &0xFF;
  (pToData)[13] |= (COMPILER_64_HI((pFrom)->m_nReserved) >> 12) &0xFF;
  (pToData)[12] |= (COMPILER_64_HI((pFrom)->m_nReserved) >> 20) &0xFF;

  /* Pack Member: m_nHec */
  (pToData)[4] |= ((pFrom)->m_nHec & 0x0f) <<4;
  (pToData)[11] |= ((pFrom)->m_nHec >> 4) & 0x0f;

  /* Pack Member: m_nZero */
  (pToData)[5] |= ((pFrom)->m_nZero & 0x7f) <<1;
  (pToData)[4] |= ((pFrom)->m_nZero >> 7) & 0x0f;

  /* Pack Member: m_nNextBuffer */
  (pToData)[7] |= ((pFrom)->m_nNextBuffer) & 0xFF;
  (pToData)[6] |= ((pFrom)->m_nNextBuffer >> 8) &0xFF;
  (pToData)[5] |= ((pFrom)->m_nNextBuffer >> 16) & 0x01;

  /* Pack Member: m_nTimestamp */
  (pToData)[3] |= ((pFrom)->m_nTimestamp) & 0xFF;
  (pToData)[2] |= ((pFrom)->m_nTimestamp >> 8) &0xFF;
  (pToData)[1] |= ((pFrom)->m_nTimestamp >> 16) &0xFF;
  (pToData)[0] |= ((pFrom)->m_nTimestamp >> 24) &0xFF;
#else
  int i;
  int size = SB_ZF_ZFKAPMLASTLINE_SIZE_IN_BYTES;

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on little endian */

  /* Pack Member: m_nReserved */
  (pToData)[8] |= ((pFrom)->m_nReserved & 0x0f) <<4;
  (pToData)[9] |= ((pFrom)->m_nReserved >> 4) &0xFF;
  (pToData)[10] |= ((pFrom)->m_nReserved >> 12) &0xFF;
  (pToData)[11] |= ((pFrom)->m_nReserved >> 20) &0xFF;
  (pToData)[12] |= ((pFrom)->m_nReserved >> 28) &0xFF;
  (pToData)[13] |= ((pFrom)->m_nReserved >> 36) &0xFF;
  (pToData)[14] |= ((pFrom)->m_nReserved >> 44) &0xFF;
  (pToData)[15] |= ((pFrom)->m_nReserved >> 52) &0xFF;

  /* Pack Member: m_nHec */
  (pToData)[7] |= ((pFrom)->m_nHec & 0x0f) <<4;
  (pToData)[8] |= ((pFrom)->m_nHec >> 4) & 0x0f;

  /* Pack Member: m_nZero */
  (pToData)[6] |= ((pFrom)->m_nZero & 0x7f) <<1;
  (pToData)[7] |= ((pFrom)->m_nZero >> 7) & 0x0f;

  /* Pack Member: m_nNextBuffer */
  (pToData)[4] |= ((pFrom)->m_nNextBuffer) & 0xFF;
  (pToData)[5] |= ((pFrom)->m_nNextBuffer >> 8) &0xFF;
  (pToData)[6] |= ((pFrom)->m_nNextBuffer >> 16) & 0x01;

  /* Pack Member: m_nTimestamp */
  (pToData)[0] |= ((pFrom)->m_nTimestamp) & 0xFF;
  (pToData)[1] |= ((pFrom)->m_nTimestamp >> 8) &0xFF;
  (pToData)[2] |= ((pFrom)->m_nTimestamp >> 16) &0xFF;
  (pToData)[3] |= ((pFrom)->m_nTimestamp >> 24) &0xFF;
#endif

  return SB_ZF_ZFKAPMLASTLINE_SIZE_IN_BYTES;
}




/* Unpack from array of bytes into struct */
void
sbZfKaPmLastLine_Unpack(sbZfKaPmLastLine_t *pToStruct,
                        uint8 *pFromData,
                        uint32 nMaxToDataIndex) {
  COMPILER_UINT64 tmp;

  (void) tmp;

#ifdef SAND_BIG_ENDIAN_HOST

  /* Unpack operation based on bigword endian */

  /* Unpack Member: m_nReserved */
  COMPILER_64_SET((pToStruct)->m_nReserved, 0,  (unsigned int) (pFromData)[11]);
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_nReserved;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[10]);
    COMPILER_64_SHL(tmp, 8);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_nReserved;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[9]);
    COMPILER_64_SHL(tmp, 16);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_nReserved;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[8]);
    COMPILER_64_SHL(tmp, 24);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_nReserved;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[15]);
    COMPILER_64_SHL(tmp, 32);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_nReserved;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[14]);
    COMPILER_64_SHL(tmp, 40);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_nReserved;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[13]);
    COMPILER_64_SHL(tmp, 48);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_nReserved;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[12]);
    COMPILER_64_SHL(tmp, 56);
    COMPILER_64_OR(*tmp0, tmp);
  };

  /* Unpack Member: m_nHec */
  (pToStruct)->m_nHec =  (uint32)  ((pFromData)[4] >> 4) & 0x0f;
  (pToStruct)->m_nHec |=  (uint32)  ((pFromData)[11] & 0x0f) << 4;

  /* Unpack Member: m_nZero */
  (pToStruct)->m_nZero =  (uint32)  ((pFromData)[5] >> 1) & 0x7f;
  (pToStruct)->m_nZero |=  (uint32)  ((pFromData)[4] & 0x0f) << 7;

  /* Unpack Member: m_nNextBuffer */
  (pToStruct)->m_nNextBuffer =  (uint32)  (pFromData)[7] ;
  (pToStruct)->m_nNextBuffer |=  (uint32)  (pFromData)[6] << 8;
  (pToStruct)->m_nNextBuffer |=  (uint32)  ((pFromData)[5] & 0x01) << 16;

  /* Unpack Member: m_nTimestamp */
  (pToStruct)->m_nTimestamp =  (uint32)  (pFromData)[3] ;
  (pToStruct)->m_nTimestamp |=  (uint32)  (pFromData)[2] << 8;
  (pToStruct)->m_nTimestamp |=  (uint32)  (pFromData)[1] << 16;
  (pToStruct)->m_nTimestamp |=  (uint32)  (pFromData)[0] << 24;
#else

  /* Unpack operation based on little endian */

  /* Unpack Member: m_nReserved */
  COMPILER_64_SET((pToStruct)->m_nReserved, 0,  (unsigned int) (pFromData)[8]);
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_nReserved;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[9]);
    COMPILER_64_SHL(tmp, 8);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_nReserved;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[10]);
    COMPILER_64_SHL(tmp, 16);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_nReserved;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[11]);
    COMPILER_64_SHL(tmp, 24);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_nReserved;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[12]);
    COMPILER_64_SHL(tmp, 32);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_nReserved;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[13]);
    COMPILER_64_SHL(tmp, 40);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_nReserved;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[14]);
    COMPILER_64_SHL(tmp, 48);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_nReserved;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[15]);
    COMPILER_64_SHL(tmp, 56);
    COMPILER_64_OR(*tmp0, tmp);
  };

  /* Unpack Member: m_nHec */
  (pToStruct)->m_nHec =  (uint32)  ((pFromData)[7] >> 4) & 0x0f;
  (pToStruct)->m_nHec |=  (uint32)  ((pFromData)[8] & 0x0f) << 4;

  /* Unpack Member: m_nZero */
  (pToStruct)->m_nZero =  (uint32)  ((pFromData)[6] >> 1) & 0x7f;
  (pToStruct)->m_nZero |=  (uint32)  ((pFromData)[7] & 0x0f) << 7;

  /* Unpack Member: m_nNextBuffer */
  (pToStruct)->m_nNextBuffer =  (uint32)  (pFromData)[4] ;
  (pToStruct)->m_nNextBuffer |=  (uint32)  (pFromData)[5] << 8;
  (pToStruct)->m_nNextBuffer |=  (uint32)  ((pFromData)[6] & 0x01) << 16;

  /* Unpack Member: m_nTimestamp */
  (pToStruct)->m_nTimestamp =  (uint32)  (pFromData)[0] ;
  (pToStruct)->m_nTimestamp |=  (uint32)  (pFromData)[1] << 8;
  (pToStruct)->m_nTimestamp |=  (uint32)  (pFromData)[2] << 16;
  (pToStruct)->m_nTimestamp |=  (uint32)  (pFromData)[3] << 24;
#endif

}



/* initialize an instance of this zframe */
void
sbZfKaPmLastLine_InitInstance(sbZfKaPmLastLine_t *pFrame) {

  COMPILER_64_ZERO(pFrame->m_nReserved);
  pFrame->m_nHec =  (unsigned int)  0;
  pFrame->m_nZero =  (unsigned int)  0;
  pFrame->m_nNextBuffer =  (unsigned int)  0;
  pFrame->m_nTimestamp =  (unsigned int)  0;

}
