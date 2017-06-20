/*
 * $Id: sbZfKaRbClassHashInputW0.c,v 1.3 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


#include "sbTypes.h"
#include "sbZfKaRbClassHashInputW0.hx"
#include "sbWrappers.h"
#include <sal/types.h>



/* Pack from struct into array of bytes */
uint32
sbZfKaRbClassHashInputW0_Pack(sbZfKaRbClassHashInputW0_t *pFrom,
                              uint8 *pToData,
                              uint32 nMaxToDataIndex) {
#ifdef SAND_BIG_ENDIAN_HOST
  int i;
  int size = SB_ZF_ZFKARBCLASSHASHINPUTW0_SIZE_IN_BYTES;

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on bigword endian */

  /* Pack Member: m_nnDmac */
  (pToData)[9]  |= (COMPILER_64_LO((pFrom)->m_nnDmac)) & 0xFF;
  (pToData)[8]  |= (COMPILER_64_LO((pFrom)->m_nnDmac) >> 8) &0xFF;
  (pToData)[15] |= (COMPILER_64_LO((pFrom)->m_nnDmac) >> 16) &0xFF;
  (pToData)[14] |= (COMPILER_64_LO((pFrom)->m_nnDmac) >> 24) &0xFF;
  (pToData)[13] |= (COMPILER_64_HI((pFrom)->m_nnDmac)) & 0xFF;
  (pToData)[12] |= (COMPILER_64_HI((pFrom)->m_nnDmac) >> 8) &0xFF;

  /* Pack Member: m_nnSmac */
  (pToData)[7]  |= (COMPILER_64_LO((pFrom)->m_nnSmac)) & 0xFF;
  (pToData)[6]  |= (COMPILER_64_LO((pFrom)->m_nnSmac) >> 8) &0xFF;
  (pToData)[5]  |= (COMPILER_64_LO((pFrom)->m_nnSmac) >> 16) &0xFF;
  (pToData)[4]  |= (COMPILER_64_LO((pFrom)->m_nnSmac) >> 24) &0xFF;
  (pToData)[11] |= (COMPILER_64_HI((pFrom)->m_nnSmac)) & 0xFF;
  (pToData)[10] |= (COMPILER_64_HI((pFrom)->m_nnSmac) >> 8) &0xFF;

  /* Pack Member: m_nSpare1 */
  (pToData)[1] |= ((pFrom)->m_nSpare1 & 0x3f) <<2;
  (pToData)[0] |= ((pFrom)->m_nSpare1 >> 6) &0xFF;

  /* Pack Member: m_nIPort */
  (pToData)[2] |= ((pFrom)->m_nIPort & 0x0f) <<4;
  (pToData)[1] |= ((pFrom)->m_nIPort >> 4) & 0x03;

  /* Pack Member: m_nVlanId */
  (pToData)[3] |= ((pFrom)->m_nVlanId) & 0xFF;
  (pToData)[2] |= ((pFrom)->m_nVlanId >> 8) & 0x0f;
#else
  int i;
  int size = SB_ZF_ZFKARBCLASSHASHINPUTW0_SIZE_IN_BYTES;

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on little endian */

  /* Pack Member: m_nnDmac */
  (pToData)[10] |= ((pFrom)->m_nnDmac) & 0xFF;
  (pToData)[11] |= ((pFrom)->m_nnDmac >> 8) &0xFF;
  (pToData)[12] |= ((pFrom)->m_nnDmac >> 16) &0xFF;
  (pToData)[13] |= ((pFrom)->m_nnDmac >> 24) &0xFF;
  (pToData)[14] |= ((pFrom)->m_nnDmac >> 32) &0xFF;
  (pToData)[15] |= ((pFrom)->m_nnDmac >> 40) &0xFF;

  /* Pack Member: m_nnSmac */
  (pToData)[4] |= ((pFrom)->m_nnSmac) & 0xFF;
  (pToData)[5] |= ((pFrom)->m_nnSmac >> 8) &0xFF;
  (pToData)[6] |= ((pFrom)->m_nnSmac >> 16) &0xFF;
  (pToData)[7] |= ((pFrom)->m_nnSmac >> 24) &0xFF;
  (pToData)[8] |= ((pFrom)->m_nnSmac >> 32) &0xFF;
  (pToData)[9] |= ((pFrom)->m_nnSmac >> 40) &0xFF;

  /* Pack Member: m_nSpare1 */
  (pToData)[2] |= ((pFrom)->m_nSpare1 & 0x3f) <<2;
  (pToData)[3] |= ((pFrom)->m_nSpare1 >> 6) &0xFF;

  /* Pack Member: m_nIPort */
  (pToData)[1] |= ((pFrom)->m_nIPort & 0x0f) <<4;
  (pToData)[2] |= ((pFrom)->m_nIPort >> 4) & 0x03;

  /* Pack Member: m_nVlanId */
  (pToData)[0] |= ((pFrom)->m_nVlanId) & 0xFF;
  (pToData)[1] |= ((pFrom)->m_nVlanId >> 8) & 0x0f;
#endif

  return SB_ZF_ZFKARBCLASSHASHINPUTW0_SIZE_IN_BYTES;
}




/* Unpack from array of bytes into struct */
void
sbZfKaRbClassHashInputW0_Unpack(sbZfKaRbClassHashInputW0_t *pToStruct,
                                uint8 *pFromData,
                                uint32 nMaxToDataIndex) {
  COMPILER_UINT64 tmp;

  (void) tmp;

#ifdef SAND_BIG_ENDIAN_HOST

  /* Unpack operation based on bigword endian */

  /* Unpack Member: m_nnDmac */
  COMPILER_64_SET((pToStruct)->m_nnDmac, 0,  (unsigned int) (pFromData)[9]);
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_nnDmac;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[8]);
    COMPILER_64_SHL(tmp, 8);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_nnDmac;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[15]);
    COMPILER_64_SHL(tmp, 16);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_nnDmac;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[14]);
    COMPILER_64_SHL(tmp, 24);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_nnDmac;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[13]);
    COMPILER_64_SHL(tmp, 32);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_nnDmac;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[12]);
    COMPILER_64_SHL(tmp, 40);
    COMPILER_64_OR(*tmp0, tmp);
  };

  /* Unpack Member: m_nnSmac */
  COMPILER_64_SET((pToStruct)->m_nnSmac, 0,  (unsigned int) (pFromData)[7]);
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_nnSmac;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[6]);
    COMPILER_64_SHL(tmp, 8);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_nnSmac;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[5]);
    COMPILER_64_SHL(tmp, 16);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_nnSmac;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[4]);
    COMPILER_64_SHL(tmp, 24);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_nnSmac;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[11]);
    COMPILER_64_SHL(tmp, 32);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_nnSmac;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[10]);
    COMPILER_64_SHL(tmp, 40);
    COMPILER_64_OR(*tmp0, tmp);
  };

  /* Unpack Member: m_nSpare1 */
  (pToStruct)->m_nSpare1 =  (uint32)  ((pFromData)[1] >> 2) & 0x3f;
  (pToStruct)->m_nSpare1 |=  (uint32)  (pFromData)[0] << 6;

  /* Unpack Member: m_nIPort */
  (pToStruct)->m_nIPort =  (uint32)  ((pFromData)[2] >> 4) & 0x0f;
  (pToStruct)->m_nIPort |=  (uint32)  ((pFromData)[1] & 0x03) << 4;

  /* Unpack Member: m_nVlanId */
  (pToStruct)->m_nVlanId =  (uint32)  (pFromData)[3] ;
  (pToStruct)->m_nVlanId |=  (uint32)  ((pFromData)[2] & 0x0f) << 8;
#else

  /* Unpack operation based on little endian */

  /* Unpack Member: m_nnDmac */
  COMPILER_64_SET((pToStruct)->m_nnDmac, 0,  (unsigned int) (pFromData)[10]);
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_nnDmac;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[11]);
    COMPILER_64_SHL(tmp, 8);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_nnDmac;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[12]);
    COMPILER_64_SHL(tmp, 16);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_nnDmac;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[13]);
    COMPILER_64_SHL(tmp, 24);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_nnDmac;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[14]);
    COMPILER_64_SHL(tmp, 32);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_nnDmac;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[15]);
    COMPILER_64_SHL(tmp, 40);
    COMPILER_64_OR(*tmp0, tmp);
  };

  /* Unpack Member: m_nnSmac */
  COMPILER_64_SET((pToStruct)->m_nnSmac, 0,  (unsigned int) (pFromData)[4]);
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_nnSmac;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[5]);
    COMPILER_64_SHL(tmp, 8);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_nnSmac;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[6]);
    COMPILER_64_SHL(tmp, 16);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_nnSmac;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[7]);
    COMPILER_64_SHL(tmp, 24);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_nnSmac;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[8]);
    COMPILER_64_SHL(tmp, 32);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_nnSmac;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[9]);
    COMPILER_64_SHL(tmp, 40);
    COMPILER_64_OR(*tmp0, tmp);
  };

  /* Unpack Member: m_nSpare1 */
  (pToStruct)->m_nSpare1 =  (uint32)  ((pFromData)[2] >> 2) & 0x3f;
  (pToStruct)->m_nSpare1 |=  (uint32)  (pFromData)[3] << 6;

  /* Unpack Member: m_nIPort */
  (pToStruct)->m_nIPort =  (uint32)  ((pFromData)[1] >> 4) & 0x0f;
  (pToStruct)->m_nIPort |=  (uint32)  ((pFromData)[2] & 0x03) << 4;

  /* Unpack Member: m_nVlanId */
  (pToStruct)->m_nVlanId =  (uint32)  (pFromData)[0] ;
  (pToStruct)->m_nVlanId |=  (uint32)  ((pFromData)[1] & 0x0f) << 8;
#endif

}



/* initialize an instance of this zframe */
void
sbZfKaRbClassHashInputW0_InitInstance(sbZfKaRbClassHashInputW0_t *pFrame) {

  COMPILER_64_ZERO(pFrame->m_nnDmac);
  COMPILER_64_ZERO(pFrame->m_nnSmac);
  pFrame->m_nSpare1 =  (unsigned int)  0;
  pFrame->m_nIPort =  (unsigned int)  0;
  pFrame->m_nVlanId =  (unsigned int)  0;

}
