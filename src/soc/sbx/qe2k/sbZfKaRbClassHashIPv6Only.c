/*
 * $Id: sbZfKaRbClassHashIPv6Only.c,v 1.3 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


#include "sbTypes.h"
#include "sbZfKaRbClassHashIPv6Only.hx"
#include "sbWrappers.h"
#include <sal/types.h>



/* Pack from struct into array of bytes */
uint32
sbZfKaRbClassHashIPv6Only_Pack(sbZfKaRbClassHashIPv6Only_t *pFrom,
                               uint8 *pToData,
                               uint32 nMaxToDataIndex) {
#ifdef SAND_BIG_ENDIAN_HOST
  int i;
  int size = SB_ZF_ZFKARBCLASSHASHIPV6ONLY_SIZE_IN_BYTES;

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on bigword endian */

  /* Pack Member: m_nProtocol */
  (pToData)[44] |= ((pFrom)->m_nProtocol) & 0xFF;

  /* Pack Member: m_nFlow */
  (pToData)[47] |= ((pFrom)->m_nFlow & 0x0f) <<4;
  (pToData)[46] |= ((pFrom)->m_nFlow >> 4) &0xFF;
  (pToData)[45] |= ((pFrom)->m_nFlow >> 12) &0xFF;

  /* Pack Member: m_nPadWord1 */
  (pToData)[41] |= ((pFrom)->m_nPadWord1) & 0xFF;
  (pToData)[40] |= ((pFrom)->m_nPadWord1 >> 8) &0xFF;
  (pToData)[47] |= ((pFrom)->m_nPadWord1 >> 16) & 0x0f;

  /* Pack Member: m_nnIpSaHigh */
  (pToData)[33] |= (COMPILER_64_LO((pFrom)->m_nnIpSaHigh)) & 0xFF;
  (pToData)[32] |= (COMPILER_64_LO((pFrom)->m_nnIpSaHigh) >> 8) &0xFF;
  (pToData)[39] |= (COMPILER_64_LO((pFrom)->m_nnIpSaHigh) >> 16) &0xFF;
  (pToData)[38] |= (COMPILER_64_LO((pFrom)->m_nnIpSaHigh) >> 24) &0xFF;
  (pToData)[37] |= (COMPILER_64_HI((pFrom)->m_nnIpSaHigh)) & 0xFF;
  (pToData)[36] |= (COMPILER_64_HI((pFrom)->m_nnIpSaHigh) >> 8) &0xFF;
  (pToData)[43] |= (COMPILER_64_HI((pFrom)->m_nnIpSaHigh) >> 16) &0xFF;
  (pToData)[42] |= (COMPILER_64_HI((pFrom)->m_nnIpSaHigh) >> 24) &0xFF;

  /* Pack Member: m_nnIpSaLow */
  (pToData)[25] |= (COMPILER_64_LO((pFrom)->m_nnIpSaLow)) & 0xFF;
  (pToData)[24] |= (COMPILER_64_LO((pFrom)->m_nnIpSaLow) >> 8) &0xFF;
  (pToData)[31] |= (COMPILER_64_LO((pFrom)->m_nnIpSaLow) >> 16) &0xFF;
  (pToData)[30] |= (COMPILER_64_LO((pFrom)->m_nnIpSaLow) >> 24) &0xFF;
  (pToData)[29] |= (COMPILER_64_HI((pFrom)->m_nnIpSaLow)) & 0xFF;
  (pToData)[28] |= (COMPILER_64_HI((pFrom)->m_nnIpSaLow) >> 8) &0xFF;
  (pToData)[35] |= (COMPILER_64_HI((pFrom)->m_nnIpSaLow) >> 16) &0xFF;
  (pToData)[34] |= (COMPILER_64_HI((pFrom)->m_nnIpSaLow) >> 24) &0xFF;

  /* Pack Member: m_nnIpDaHigh */
  (pToData)[17] |= (COMPILER_64_LO((pFrom)->m_nnIpDaHigh)) & 0xFF;
  (pToData)[16] |= (COMPILER_64_LO((pFrom)->m_nnIpDaHigh) >> 8) &0xFF;
  (pToData)[23] |= (COMPILER_64_LO((pFrom)->m_nnIpDaHigh) >> 16) &0xFF;
  (pToData)[22] |= (COMPILER_64_LO((pFrom)->m_nnIpDaHigh) >> 24) &0xFF;
  (pToData)[21] |= (COMPILER_64_HI((pFrom)->m_nnIpDaHigh)) & 0xFF;
  (pToData)[20] |= (COMPILER_64_HI((pFrom)->m_nnIpDaHigh) >> 8) &0xFF;
  (pToData)[27] |= (COMPILER_64_HI((pFrom)->m_nnIpDaHigh) >> 16) &0xFF;
  (pToData)[26] |= (COMPILER_64_HI((pFrom)->m_nnIpDaHigh) >> 24) &0xFF;

  /* Pack Member: m_nnIpDaLow */
  (pToData)[9]  |= (COMPILER_64_LO((pFrom)->m_nnIpDaLow)) & 0xFF;
  (pToData)[8]  |= (COMPILER_64_LO((pFrom)->m_nnIpDaLow) >> 8) &0xFF;
  (pToData)[15] |= (COMPILER_64_LO((pFrom)->m_nnIpDaLow) >> 16) &0xFF;
  (pToData)[14] |= (COMPILER_64_LO((pFrom)->m_nnIpDaLow) >> 24) &0xFF;
  (pToData)[13] |= (COMPILER_64_HI((pFrom)->m_nnIpDaLow)) & 0xFF;
  (pToData)[12] |= (COMPILER_64_HI((pFrom)->m_nnIpDaLow) >> 8) &0xFF;
  (pToData)[19] |= (COMPILER_64_HI((pFrom)->m_nnIpDaLow) >> 16) &0xFF;
  (pToData)[18] |= (COMPILER_64_HI((pFrom)->m_nnIpDaLow) >> 24) &0xFF;

  /* Pack Member: m_nSocket */
  (pToData)[5] |= ((pFrom)->m_nSocket) & 0xFF;
  (pToData)[4] |= ((pFrom)->m_nSocket >> 8) &0xFF;
  (pToData)[11] |= ((pFrom)->m_nSocket >> 16) &0xFF;
  (pToData)[10] |= ((pFrom)->m_nSocket >> 24) &0xFF;

  /* Pack Member: m_nPadWord3 */
  (pToData)[3] |= (COMPILER_64_LO((pFrom)->m_nPadWord3)) & 0xFF;
  (pToData)[2] |= (COMPILER_64_LO((pFrom)->m_nPadWord3) >> 8) &0xFF;
  (pToData)[1] |= (COMPILER_64_LO((pFrom)->m_nPadWord3) >> 16) &0xFF;
  (pToData)[0] |= (COMPILER_64_LO((pFrom)->m_nPadWord3) >> 24) &0xFF;
  (pToData)[7] |= (COMPILER_64_HI((pFrom)->m_nPadWord3)) & 0xFF;
  (pToData)[6] |= (COMPILER_64_HI((pFrom)->m_nPadWord3) >> 8) &0xFF;
#else                                                 
  int i;                                              
  int size = SB_ZF_ZFKARBCLASSHASHIPV6ONLY_SIZE_IN_BYTES;

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on little endian */

  /* Pack Member: m_nProtocol */
  (pToData)[47] |= ((pFrom)->m_nProtocol) & 0xFF;

  /* Pack Member: m_nFlow */
  (pToData)[44] |= ((pFrom)->m_nFlow & 0x0f) <<4;
  (pToData)[45] |= ((pFrom)->m_nFlow >> 4) &0xFF;
  (pToData)[46] |= ((pFrom)->m_nFlow >> 12) &0xFF;

  /* Pack Member: m_nPadWord1 */
  (pToData)[42] |= ((pFrom)->m_nPadWord1) & 0xFF;
  (pToData)[43] |= ((pFrom)->m_nPadWord1 >> 8) &0xFF;
  (pToData)[44] |= ((pFrom)->m_nPadWord1 >> 16) & 0x0f;

  /* Pack Member: m_nnIpSaHigh */
  (pToData)[34] |= ((pFrom)->m_nnIpSaHigh) & 0xFF;
  (pToData)[35] |= ((pFrom)->m_nnIpSaHigh >> 8) &0xFF;
  (pToData)[36] |= ((pFrom)->m_nnIpSaHigh >> 16) &0xFF;
  (pToData)[37] |= ((pFrom)->m_nnIpSaHigh >> 24) &0xFF;
  (pToData)[38] |= ((pFrom)->m_nnIpSaHigh >> 32) &0xFF;
  (pToData)[39] |= ((pFrom)->m_nnIpSaHigh >> 40) &0xFF;
  (pToData)[40] |= ((pFrom)->m_nnIpSaHigh >> 48) &0xFF;
  (pToData)[41] |= ((pFrom)->m_nnIpSaHigh >> 56) &0xFF;

  /* Pack Member: m_nnIpSaLow */
  (pToData)[26] |= ((pFrom)->m_nnIpSaLow) & 0xFF;
  (pToData)[27] |= ((pFrom)->m_nnIpSaLow >> 8) &0xFF;
  (pToData)[28] |= ((pFrom)->m_nnIpSaLow >> 16) &0xFF;
  (pToData)[29] |= ((pFrom)->m_nnIpSaLow >> 24) &0xFF;
  (pToData)[30] |= ((pFrom)->m_nnIpSaLow >> 32) &0xFF;
  (pToData)[31] |= ((pFrom)->m_nnIpSaLow >> 40) &0xFF;
  (pToData)[32] |= ((pFrom)->m_nnIpSaLow >> 48) &0xFF;
  (pToData)[33] |= ((pFrom)->m_nnIpSaLow >> 56) &0xFF;

  /* Pack Member: m_nnIpDaHigh */
  (pToData)[18] |= ((pFrom)->m_nnIpDaHigh) & 0xFF;
  (pToData)[19] |= ((pFrom)->m_nnIpDaHigh >> 8) &0xFF;
  (pToData)[20] |= ((pFrom)->m_nnIpDaHigh >> 16) &0xFF;
  (pToData)[21] |= ((pFrom)->m_nnIpDaHigh >> 24) &0xFF;
  (pToData)[22] |= ((pFrom)->m_nnIpDaHigh >> 32) &0xFF;
  (pToData)[23] |= ((pFrom)->m_nnIpDaHigh >> 40) &0xFF;
  (pToData)[24] |= ((pFrom)->m_nnIpDaHigh >> 48) &0xFF;
  (pToData)[25] |= ((pFrom)->m_nnIpDaHigh >> 56) &0xFF;

  /* Pack Member: m_nnIpDaLow */
  (pToData)[10] |= ((pFrom)->m_nnIpDaLow) & 0xFF;
  (pToData)[11] |= ((pFrom)->m_nnIpDaLow >> 8) &0xFF;
  (pToData)[12] |= ((pFrom)->m_nnIpDaLow >> 16) &0xFF;
  (pToData)[13] |= ((pFrom)->m_nnIpDaLow >> 24) &0xFF;
  (pToData)[14] |= ((pFrom)->m_nnIpDaLow >> 32) &0xFF;
  (pToData)[15] |= ((pFrom)->m_nnIpDaLow >> 40) &0xFF;
  (pToData)[16] |= ((pFrom)->m_nnIpDaLow >> 48) &0xFF;
  (pToData)[17] |= ((pFrom)->m_nnIpDaLow >> 56) &0xFF;

  /* Pack Member: m_nSocket */
  (pToData)[6] |= ((pFrom)->m_nSocket) & 0xFF;
  (pToData)[7] |= ((pFrom)->m_nSocket >> 8) &0xFF;
  (pToData)[8] |= ((pFrom)->m_nSocket >> 16) &0xFF;
  (pToData)[9] |= ((pFrom)->m_nSocket >> 24) &0xFF;

  /* Pack Member: m_nPadWord3 */
  (pToData)[0] |= ((pFrom)->m_nPadWord3) & 0xFF;
  (pToData)[1] |= ((pFrom)->m_nPadWord3 >> 8) &0xFF;
  (pToData)[2] |= ((pFrom)->m_nPadWord3 >> 16) &0xFF;
  (pToData)[3] |= ((pFrom)->m_nPadWord3 >> 24) &0xFF;
  (pToData)[4] |= ((pFrom)->m_nPadWord3 >> 32) &0xFF;
  (pToData)[5] |= ((pFrom)->m_nPadWord3 >> 40) &0xFF;
#endif

  return SB_ZF_ZFKARBCLASSHASHIPV6ONLY_SIZE_IN_BYTES;
}




/* Unpack from array of bytes into struct */
void
sbZfKaRbClassHashIPv6Only_Unpack(sbZfKaRbClassHashIPv6Only_t *pToStruct,
                                 uint8 *pFromData,
                                 uint32 nMaxToDataIndex) {
  COMPILER_UINT64 tmp;

  (void) tmp;

#ifdef SAND_BIG_ENDIAN_HOST

  /* Unpack operation based on bigword endian */

  /* Unpack Member: m_nProtocol */
  (pToStruct)->m_nProtocol =  (uint32)  (pFromData)[44] ;

  /* Unpack Member: m_nFlow */
  (pToStruct)->m_nFlow =  (uint32)  ((pFromData)[47] >> 4) & 0x0f;
  (pToStruct)->m_nFlow |=  (uint32)  (pFromData)[46] << 4;
  (pToStruct)->m_nFlow |=  (uint32)  (pFromData)[45] << 12;

  /* Unpack Member: m_nPadWord1 */
  (pToStruct)->m_nPadWord1 =  (uint32)  (pFromData)[41] ;
  (pToStruct)->m_nPadWord1 |=  (uint32)  (pFromData)[40] << 8;
  (pToStruct)->m_nPadWord1 |=  (uint32)  ((pFromData)[47] & 0x0f) << 16;

  /* Unpack Member: m_nnIpSaHigh */
  COMPILER_64_SET((pToStruct)->m_nnIpSaHigh, 0,  (unsigned int) (pFromData)[33]);
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_nnIpSaHigh;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[32]);
    COMPILER_64_SHL(tmp, 8);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_nnIpSaHigh;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[39]);
    COMPILER_64_SHL(tmp, 16);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_nnIpSaHigh;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[38]);
    COMPILER_64_SHL(tmp, 24);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_nnIpSaHigh;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[37]);
    COMPILER_64_SHL(tmp, 32);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_nnIpSaHigh;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[36]);
    COMPILER_64_SHL(tmp, 40);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_nnIpSaHigh;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[43]);
    COMPILER_64_SHL(tmp, 48);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_nnIpSaHigh;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[42]);
    COMPILER_64_SHL(tmp, 56);
    COMPILER_64_OR(*tmp0, tmp);
  };

  /* Unpack Member: m_nnIpSaLow */
  COMPILER_64_SET((pToStruct)->m_nnIpSaLow, 0,  (unsigned int) (pFromData)[25]);
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_nnIpSaLow;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[24]);
    COMPILER_64_SHL(tmp, 8);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_nnIpSaLow;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[31]);
    COMPILER_64_SHL(tmp, 16);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_nnIpSaLow;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[30]);
    COMPILER_64_SHL(tmp, 24);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_nnIpSaLow;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[29]);
    COMPILER_64_SHL(tmp, 32);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_nnIpSaLow;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[28]);
    COMPILER_64_SHL(tmp, 40);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_nnIpSaLow;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[35]);
    COMPILER_64_SHL(tmp, 48);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_nnIpSaLow;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[34]);
    COMPILER_64_SHL(tmp, 56);
    COMPILER_64_OR(*tmp0, tmp);
  };

  /* Unpack Member: m_nnIpDaHigh */
  COMPILER_64_SET((pToStruct)->m_nnIpDaHigh, 0,  (unsigned int) (pFromData)[17]);
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_nnIpDaHigh;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[16]);
    COMPILER_64_SHL(tmp, 8);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_nnIpDaHigh;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[23]);
    COMPILER_64_SHL(tmp, 16);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_nnIpDaHigh;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[22]);
    COMPILER_64_SHL(tmp, 24);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_nnIpDaHigh;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[21]);
    COMPILER_64_SHL(tmp, 32);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_nnIpDaHigh;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[20]);
    COMPILER_64_SHL(tmp, 40);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_nnIpDaHigh;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[27]);
    COMPILER_64_SHL(tmp, 48);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_nnIpDaHigh;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[26]);
    COMPILER_64_SHL(tmp, 56);
    COMPILER_64_OR(*tmp0, tmp);
  };

  /* Unpack Member: m_nnIpDaLow */
  COMPILER_64_SET((pToStruct)->m_nnIpDaLow, 0,  (unsigned int) (pFromData)[9]);
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_nnIpDaLow;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[8]);
    COMPILER_64_SHL(tmp, 8);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_nnIpDaLow;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[15]);
    COMPILER_64_SHL(tmp, 16);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_nnIpDaLow;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[14]);
    COMPILER_64_SHL(tmp, 24);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_nnIpDaLow;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[13]);
    COMPILER_64_SHL(tmp, 32);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_nnIpDaLow;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[12]);
    COMPILER_64_SHL(tmp, 40);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_nnIpDaLow;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[19]);
    COMPILER_64_SHL(tmp, 48);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_nnIpDaLow;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[18]);
    COMPILER_64_SHL(tmp, 56);
    COMPILER_64_OR(*tmp0, tmp);
  };

  /* Unpack Member: m_nSocket */
  (pToStruct)->m_nSocket =  (uint32)  (pFromData)[5] ;
  (pToStruct)->m_nSocket |=  (uint32)  (pFromData)[4] << 8;
  (pToStruct)->m_nSocket |=  (uint32)  (pFromData)[11] << 16;
  (pToStruct)->m_nSocket |=  (uint32)  (pFromData)[10] << 24;

  /* Unpack Member: m_nPadWord3 */
  COMPILER_64_SET((pToStruct)->m_nPadWord3, 0,  (unsigned int) (pFromData)[3]);
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_nPadWord3;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[2]);
    COMPILER_64_SHL(tmp, 8);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_nPadWord3;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[1]);
    COMPILER_64_SHL(tmp, 16);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_nPadWord3;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[0]);
    COMPILER_64_SHL(tmp, 24);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_nPadWord3;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[7]);
    COMPILER_64_SHL(tmp, 32);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_nPadWord3;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[6]);
    COMPILER_64_SHL(tmp, 40);
    COMPILER_64_OR(*tmp0, tmp);
  };
#else

  /* Unpack operation based on little endian */

  /* Unpack Member: m_nProtocol */
  (pToStruct)->m_nProtocol =  (uint32)  (pFromData)[47] ;

  /* Unpack Member: m_nFlow */
  (pToStruct)->m_nFlow =  (uint32)  ((pFromData)[44] >> 4) & 0x0f;
  (pToStruct)->m_nFlow |=  (uint32)  (pFromData)[45] << 4;
  (pToStruct)->m_nFlow |=  (uint32)  (pFromData)[46] << 12;

  /* Unpack Member: m_nPadWord1 */
  (pToStruct)->m_nPadWord1 =  (uint32)  (pFromData)[42] ;
  (pToStruct)->m_nPadWord1 |=  (uint32)  (pFromData)[43] << 8;
  (pToStruct)->m_nPadWord1 |=  (uint32)  ((pFromData)[44] & 0x0f) << 16;

  /* Unpack Member: m_nnIpSaHigh */
  COMPILER_64_SET((pToStruct)->m_nnIpSaHigh, 0,  (unsigned int) (pFromData)[34]);
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_nnIpSaHigh;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[35]);
    COMPILER_64_SHL(tmp, 8);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_nnIpSaHigh;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[36]);
    COMPILER_64_SHL(tmp, 16);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_nnIpSaHigh;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[37]);
    COMPILER_64_SHL(tmp, 24);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_nnIpSaHigh;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[38]);
    COMPILER_64_SHL(tmp, 32);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_nnIpSaHigh;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[39]);
    COMPILER_64_SHL(tmp, 40);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_nnIpSaHigh;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[40]);
    COMPILER_64_SHL(tmp, 48);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_nnIpSaHigh;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[41]);
    COMPILER_64_SHL(tmp, 56);
    COMPILER_64_OR(*tmp0, tmp);
  };

  /* Unpack Member: m_nnIpSaLow */
  COMPILER_64_SET((pToStruct)->m_nnIpSaLow, 0,  (unsigned int) (pFromData)[26]);
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_nnIpSaLow;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[27]);
    COMPILER_64_SHL(tmp, 8);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_nnIpSaLow;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[28]);
    COMPILER_64_SHL(tmp, 16);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_nnIpSaLow;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[29]);
    COMPILER_64_SHL(tmp, 24);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_nnIpSaLow;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[30]);
    COMPILER_64_SHL(tmp, 32);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_nnIpSaLow;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[31]);
    COMPILER_64_SHL(tmp, 40);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_nnIpSaLow;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[32]);
    COMPILER_64_SHL(tmp, 48);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_nnIpSaLow;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[33]);
    COMPILER_64_SHL(tmp, 56);
    COMPILER_64_OR(*tmp0, tmp);
  };

  /* Unpack Member: m_nnIpDaHigh */
  COMPILER_64_SET((pToStruct)->m_nnIpDaHigh, 0,  (unsigned int) (pFromData)[18]);
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_nnIpDaHigh;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[19]);
    COMPILER_64_SHL(tmp, 8);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_nnIpDaHigh;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[20]);
    COMPILER_64_SHL(tmp, 16);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_nnIpDaHigh;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[21]);
    COMPILER_64_SHL(tmp, 24);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_nnIpDaHigh;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[22]);
    COMPILER_64_SHL(tmp, 32);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_nnIpDaHigh;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[23]);
    COMPILER_64_SHL(tmp, 40);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_nnIpDaHigh;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[24]);
    COMPILER_64_SHL(tmp, 48);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_nnIpDaHigh;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[25]);
    COMPILER_64_SHL(tmp, 56);
    COMPILER_64_OR(*tmp0, tmp);
  };

  /* Unpack Member: m_nnIpDaLow */
  COMPILER_64_SET((pToStruct)->m_nnIpDaLow, 0,  (unsigned int) (pFromData)[10]);
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_nnIpDaLow;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[11]);
    COMPILER_64_SHL(tmp, 8);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_nnIpDaLow;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[12]);
    COMPILER_64_SHL(tmp, 16);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_nnIpDaLow;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[13]);
    COMPILER_64_SHL(tmp, 24);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_nnIpDaLow;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[14]);
    COMPILER_64_SHL(tmp, 32);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_nnIpDaLow;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[15]);
    COMPILER_64_SHL(tmp, 40);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_nnIpDaLow;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[16]);
    COMPILER_64_SHL(tmp, 48);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_nnIpDaLow;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[17]);
    COMPILER_64_SHL(tmp, 56);
    COMPILER_64_OR(*tmp0, tmp);
  };

  /* Unpack Member: m_nSocket */
  (pToStruct)->m_nSocket =  (uint32)  (pFromData)[6] ;
  (pToStruct)->m_nSocket |=  (uint32)  (pFromData)[7] << 8;
  (pToStruct)->m_nSocket |=  (uint32)  (pFromData)[8] << 16;
  (pToStruct)->m_nSocket |=  (uint32)  (pFromData)[9] << 24;

  /* Unpack Member: m_nPadWord3 */
  COMPILER_64_SET((pToStruct)->m_nPadWord3, 0,  (unsigned int) (pFromData)[0]);
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_nPadWord3;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[1]);
    COMPILER_64_SHL(tmp, 8);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_nPadWord3;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[2]);
    COMPILER_64_SHL(tmp, 16);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_nPadWord3;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[3]);
    COMPILER_64_SHL(tmp, 24);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_nPadWord3;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[4]);
    COMPILER_64_SHL(tmp, 32);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_nPadWord3;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[5]);
    COMPILER_64_SHL(tmp, 40);
    COMPILER_64_OR(*tmp0, tmp);
  };
#endif

}



/* initialize an instance of this zframe */
void
sbZfKaRbClassHashIPv6Only_InitInstance(sbZfKaRbClassHashIPv6Only_t *pFrame) {

  pFrame->m_nProtocol =  (unsigned int)  0;
  pFrame->m_nFlow =  (unsigned int)  0;
  pFrame->m_nPadWord1 =  (unsigned int)  0;
  COMPILER_64_ZERO(pFrame->m_nnIpSaHigh);
  COMPILER_64_ZERO(pFrame->m_nnIpSaLow);
  COMPILER_64_ZERO(pFrame->m_nnIpDaHigh);
  COMPILER_64_ZERO(pFrame->m_nnIpDaLow);
  pFrame->m_nSocket =  (unsigned int)  0;
  COMPILER_64_ZERO(pFrame->m_nPadWord3);

}
