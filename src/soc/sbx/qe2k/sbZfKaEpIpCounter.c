/*
 * $Id: sbZfKaEpIpCounter.c,v 1.3 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


#include "sbTypes.h"
#include "sbZfKaEpIpCounter.hx"
#include "sbWrappers.h"
#include <sal/types.h>



/* Pack from struct into array of bytes */
uint32
sbZfKaEpIpCounter_Pack(sbZfKaEpIpCounter_t *pFrom,
                       uint8 *pToData,
                       uint32 nMaxToDataIndex) {
#ifdef SAND_BIG_ENDIAN_HOST
  int i;
  int size = SB_ZF_ZFKAEPIPCOUNTER_SIZE_IN_BYTES;

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on bigword endian */

  /* Pack Member: m_nPktCnt */
  (pToData)[7] |= ((pFrom)->m_nPktCnt & 0x1f) <<3;
  (pToData)[6] |= ((pFrom)->m_nPktCnt >> 5) &0xFF;
  (pToData)[5] |= ((pFrom)->m_nPktCnt >> 13) &0xFF;
  (pToData)[4] |= ((pFrom)->m_nPktCnt >> 21) &0xFF;

  /* Pack Member: m_nnByteCnt */
  (pToData)[3] |= (COMPILER_64_LO((pFrom)->m_nnByteCnt)) & 0xFF;
  (pToData)[2] |= (COMPILER_64_LO((pFrom)->m_nnByteCnt) >> 8) &0xFF;
  (pToData)[1] |= (COMPILER_64_LO((pFrom)->m_nnByteCnt) >> 16) &0xFF;
  (pToData)[0] |= (COMPILER_64_LO((pFrom)->m_nnByteCnt) >> 24) &0xFF;
  (pToData)[7] |= (COMPILER_64_HI((pFrom)->m_nnByteCnt)) & 0x07;
#else
  int i;
  int size = SB_ZF_ZFKAEPIPCOUNTER_SIZE_IN_BYTES;

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on little endian */

  /* Pack Member: m_nPktCnt */
  (pToData)[4] |= ((pFrom)->m_nPktCnt & 0x1f) <<3;
  (pToData)[5] |= ((pFrom)->m_nPktCnt >> 5) &0xFF;
  (pToData)[6] |= ((pFrom)->m_nPktCnt >> 13) &0xFF;
  (pToData)[7] |= ((pFrom)->m_nPktCnt >> 21) &0xFF;

  /* Pack Member: m_nnByteCnt */
  (pToData)[0] |= ((pFrom)->m_nnByteCnt) & 0xFF;
  (pToData)[1] |= ((pFrom)->m_nnByteCnt >> 8) &0xFF;
  (pToData)[2] |= ((pFrom)->m_nnByteCnt >> 16) &0xFF;
  (pToData)[3] |= ((pFrom)->m_nnByteCnt >> 24) &0xFF;
  (pToData)[4] |= ((pFrom)->m_nnByteCnt >> 32) & 0x07;
#endif

  return SB_ZF_ZFKAEPIPCOUNTER_SIZE_IN_BYTES;
}




/* Unpack from array of bytes into struct */
void
sbZfKaEpIpCounter_Unpack(sbZfKaEpIpCounter_t *pToStruct,
                         uint8 *pFromData,
                         uint32 nMaxToDataIndex) {
  COMPILER_UINT64 tmp;

  (void) tmp;

#ifdef SAND_BIG_ENDIAN_HOST

  /* Unpack operation based on bigword endian */

  /* Unpack Member: m_nPktCnt */
  (pToStruct)->m_nPktCnt =  (uint32)  ((pFromData)[7] >> 3) & 0x1f;
  (pToStruct)->m_nPktCnt |=  (uint32)  (pFromData)[6] << 5;
  (pToStruct)->m_nPktCnt |=  (uint32)  (pFromData)[5] << 13;
  (pToStruct)->m_nPktCnt |=  (uint32)  (pFromData)[4] << 21;

  /* Unpack Member: m_nnByteCnt */
  COMPILER_64_SET((pToStruct)->m_nnByteCnt, 0,  (unsigned int) (pFromData)[3]);
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_nnByteCnt;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[2]);
    COMPILER_64_SHL(tmp, 8);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_nnByteCnt;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[1]);
    COMPILER_64_SHL(tmp, 16);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_nnByteCnt;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[0]);
    COMPILER_64_SHL(tmp, 24);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_nnByteCnt;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[7]);
    COMPILER_64_SHL(tmp, 32);
    COMPILER_64_OR(*tmp0, tmp);
  };
#else

  /* Unpack operation based on little endian */

  /* Unpack Member: m_nPktCnt */
  (pToStruct)->m_nPktCnt =  (uint32)  ((pFromData)[4] >> 3) & 0x1f;
  (pToStruct)->m_nPktCnt |=  (uint32)  (pFromData)[5] << 5;
  (pToStruct)->m_nPktCnt |=  (uint32)  (pFromData)[6] << 13;
  (pToStruct)->m_nPktCnt |=  (uint32)  (pFromData)[7] << 21;

  /* Unpack Member: m_nnByteCnt */
  COMPILER_64_SET((pToStruct)->m_nnByteCnt, 0,  (unsigned int) (pFromData)[0]);
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_nnByteCnt;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[1]);
    COMPILER_64_SHL(tmp, 8);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_nnByteCnt;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[2]);
    COMPILER_64_SHL(tmp, 16);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_nnByteCnt;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[3]);
    COMPILER_64_SHL(tmp, 24);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_nnByteCnt;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[4]);
    COMPILER_64_SHL(tmp, 32);
    COMPILER_64_OR(*tmp0, tmp);
  };
#endif

}



/* initialize an instance of this zframe */
void
sbZfKaEpIpCounter_InitInstance(sbZfKaEpIpCounter_t *pFrame) {

  pFrame->m_nPktCnt =  (unsigned int)  0;
  COMPILER_64_ZERO(pFrame->m_nnByteCnt);

}
