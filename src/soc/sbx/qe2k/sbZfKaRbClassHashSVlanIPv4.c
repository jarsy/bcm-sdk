/*
 * $Id: sbZfKaRbClassHashSVlanIPv4.c,v 1.3 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


#include "sbTypes.h"
#include "sbZfKaRbClassHashSVlanIPv4.hx"
#include "sbWrappers.h"
#include <sal/types.h>



/* Pack from struct into array of bytes */
uint32
sbZfKaRbClassHashSVlanIPv4_Pack(sbZfKaRbClassHashSVlanIPv4_t *pFrom,
                                uint8 *pToData,
                                uint32 nMaxToDataIndex) {
#ifdef SAND_BIG_ENDIAN_HOST
  int i;
  int size = SB_ZF_ZFKARBCLASSHASHSVLANIPV4_SIZE_IN_BYTES;

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on bigword endian */

  /* Pack Member: m_nProtocol */
  (pToData)[44] |= ((pFrom)->m_nProtocol) & 0xFF;

  /* Pack Member: m_nPadWord3 */
  (pToData)[36] |= (COMPILER_64_LO((pFrom)->m_nPadWord3) & 0x07) <<5;
  (pToData)[43] |= (COMPILER_64_LO((pFrom)->m_nPadWord3) >> 3) &0xFF;
  (pToData)[42] |= (COMPILER_64_LO((pFrom)->m_nPadWord3) >> 11) &0xFF;
  (pToData)[41] |= (COMPILER_64_LO((pFrom)->m_nPadWord3) >> 19) &0xFF;
  (pToData)[40] |= (COMPILER_64_LO((pFrom)->m_nPadWord3) >> 27) &0x1F;
  (pToData)[40] |= (COMPILER_64_HI((pFrom)->m_nPadWord3)  & 0x7) <<5;
  (pToData)[47] |= (COMPILER_64_HI((pFrom)->m_nPadWord3) >> 3) &0xFF;
  (pToData)[46] |= (COMPILER_64_HI((pFrom)->m_nPadWord3) >> 11) &0xFF;
  (pToData)[45] |= (COMPILER_64_HI((pFrom)->m_nPadWord3) >> 19) &0xFF;

  /* Pack Member: m_nPadWord1 */
  (pToData)[35] |= (COMPILER_64_LO((pFrom)->m_nPadWord1)) & 0xFF;
  (pToData)[34] |= (COMPILER_64_LO((pFrom)->m_nPadWord1) >> 8) &0xFF;
  (pToData)[33] |= (COMPILER_64_LO((pFrom)->m_nPadWord1) >> 16) &0xFF;
  (pToData)[32] |= (COMPILER_64_LO((pFrom)->m_nPadWord1) >> 24) &0xFF;
  (pToData)[39] |= (COMPILER_64_HI((pFrom)->m_nPadWord1)) & 0xFF;
  (pToData)[38] |= (COMPILER_64_HI((pFrom)->m_nPadWord1) >> 8) &0xFF;
  (pToData)[37] |= (COMPILER_64_HI((pFrom)->m_nPadWord1) >> 16) &0xFF;
  (pToData)[36] |= (COMPILER_64_HI((pFrom)->m_nPadWord1) >> 24) & 0x1f;

  /* Pack Member: m_nIpSa */
  (pToData)[31] |= ((pFrom)->m_nIpSa) & 0xFF;
  (pToData)[30] |= ((pFrom)->m_nIpSa >> 8) &0xFF;
  (pToData)[29] |= ((pFrom)->m_nIpSa >> 16) &0xFF;
  (pToData)[28] |= ((pFrom)->m_nIpSa >> 24) &0xFF;

  /* Pack Member: m_nIpDa */
  (pToData)[27] |= ((pFrom)->m_nIpDa) & 0xFF;
  (pToData)[26] |= ((pFrom)->m_nIpDa >> 8) &0xFF;
  (pToData)[25] |= ((pFrom)->m_nIpDa >> 16) &0xFF;
  (pToData)[24] |= ((pFrom)->m_nIpDa >> 24) &0xFF;

  /* Pack Member: m_nSocket */
  (pToData)[23] |= ((pFrom)->m_nSocket) & 0xFF;
  (pToData)[22] |= ((pFrom)->m_nSocket >> 8) &0xFF;
  (pToData)[21] |= ((pFrom)->m_nSocket >> 16) &0xFF;
  (pToData)[20] |= ((pFrom)->m_nSocket >> 24) &0xFF;

  /* Pack Member: m_nPadWord2 */
  (pToData)[19] |= ((pFrom)->m_nPadWord2) & 0xFF;
  (pToData)[18] |= ((pFrom)->m_nPadWord2 >> 8) &0xFF;
  (pToData)[17] |= ((pFrom)->m_nPadWord2 >> 16) &0xFF;
  (pToData)[16] |= ((pFrom)->m_nPadWord2 >> 24) &0xFF;

  /* Pack Member: m_nSpareWord1 */
  (pToData)[11] |= (COMPILER_64_LO((pFrom)->m_nSpareWord1)) & 0xFF;
  (pToData)[10] |= (COMPILER_64_LO((pFrom)->m_nSpareWord1) >> 8) &0xFF;
  (pToData)[9] |= (COMPILER_64_LO((pFrom)->m_nSpareWord1) >> 16) &0xFF;
  (pToData)[8] |= (COMPILER_64_LO((pFrom)->m_nSpareWord1) >> 24) &0xFF;
  (pToData)[15] |= (COMPILER_64_HI((pFrom)->m_nSpareWord1)) & 0xFF;
  (pToData)[14] |= (COMPILER_64_HI((pFrom)->m_nSpareWord1) >> 8) &0xFF;
  (pToData)[13] |= (COMPILER_64_HI((pFrom)->m_nSpareWord1) >> 16) &0xFF;
  (pToData)[12] |= (COMPILER_64_HI((pFrom)->m_nSpareWord1) >> 24) &0xFF;

  /* Pack Member: m_nSpareWord0 */
  (pToData)[3] |= (COMPILER_64_LO((pFrom)->m_nSpareWord0)) & 0xFF;
  (pToData)[2] |= (COMPILER_64_LO((pFrom)->m_nSpareWord0) >> 8) &0xFF;
  (pToData)[1] |= (COMPILER_64_LO((pFrom)->m_nSpareWord0) >> 16) &0xFF;
  (pToData)[0] |= (COMPILER_64_LO((pFrom)->m_nSpareWord0) >> 24) &0xFF;
  (pToData)[7] |= (COMPILER_64_HI((pFrom)->m_nSpareWord0)) & 0xFF;
  (pToData)[6] |= (COMPILER_64_HI((pFrom)->m_nSpareWord0) >> 8) &0xFF;
  (pToData)[5] |= (COMPILER_64_HI((pFrom)->m_nSpareWord0) >> 16) &0xFF;
  (pToData)[4] |= (COMPILER_64_HI((pFrom)->m_nSpareWord0) >> 24) &0xFF;
#else
  int i;
  int size = SB_ZF_ZFKARBCLASSHASHSVLANIPV4_SIZE_IN_BYTES;

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on little endian */

  /* Pack Member: m_nProtocol */
  (pToData)[47] |= ((pFrom)->m_nProtocol) & 0xFF;

  /* Pack Member: m_nPadWord3 */
  (pToData)[39] |= ((pFrom)->m_nPadWord3 & 0x07) <<5;
  (pToData)[40] |= ((pFrom)->m_nPadWord3 >> 3) &0xFF;
  (pToData)[41] |= ((pFrom)->m_nPadWord3 >> 11) &0xFF;
  (pToData)[42] |= ((pFrom)->m_nPadWord3 >> 19) &0xFF;
  (pToData)[43] |= ((pFrom)->m_nPadWord3 >> 27) &0xFF;
  (pToData)[44] |= ((pFrom)->m_nPadWord3 >> 35) &0xFF;
  (pToData)[45] |= ((pFrom)->m_nPadWord3 >> 43) &0xFF;
  (pToData)[46] |= ((pFrom)->m_nPadWord3 >> 51) &0xFF;

  /* Pack Member: m_nPadWord1 */
  (pToData)[32] |= ((pFrom)->m_nPadWord1) & 0xFF;
  (pToData)[33] |= ((pFrom)->m_nPadWord1 >> 8) &0xFF;
  (pToData)[34] |= ((pFrom)->m_nPadWord1 >> 16) &0xFF;
  (pToData)[35] |= ((pFrom)->m_nPadWord1 >> 24) &0xFF;
  (pToData)[36] |= ((pFrom)->m_nPadWord1 >> 32) &0xFF;
  (pToData)[37] |= ((pFrom)->m_nPadWord1 >> 40) &0xFF;
  (pToData)[38] |= ((pFrom)->m_nPadWord1 >> 48) &0xFF;
  (pToData)[39] |= ((pFrom)->m_nPadWord1 >> 56) & 0x1f;

  /* Pack Member: m_nIpSa */
  (pToData)[28] |= ((pFrom)->m_nIpSa) & 0xFF;
  (pToData)[29] |= ((pFrom)->m_nIpSa >> 8) &0xFF;
  (pToData)[30] |= ((pFrom)->m_nIpSa >> 16) &0xFF;
  (pToData)[31] |= ((pFrom)->m_nIpSa >> 24) &0xFF;

  /* Pack Member: m_nIpDa */
  (pToData)[24] |= ((pFrom)->m_nIpDa) & 0xFF;
  (pToData)[25] |= ((pFrom)->m_nIpDa >> 8) &0xFF;
  (pToData)[26] |= ((pFrom)->m_nIpDa >> 16) &0xFF;
  (pToData)[27] |= ((pFrom)->m_nIpDa >> 24) &0xFF;

  /* Pack Member: m_nSocket */
  (pToData)[20] |= ((pFrom)->m_nSocket) & 0xFF;
  (pToData)[21] |= ((pFrom)->m_nSocket >> 8) &0xFF;
  (pToData)[22] |= ((pFrom)->m_nSocket >> 16) &0xFF;
  (pToData)[23] |= ((pFrom)->m_nSocket >> 24) &0xFF;

  /* Pack Member: m_nPadWord2 */
  (pToData)[16] |= ((pFrom)->m_nPadWord2) & 0xFF;
  (pToData)[17] |= ((pFrom)->m_nPadWord2 >> 8) &0xFF;
  (pToData)[18] |= ((pFrom)->m_nPadWord2 >> 16) &0xFF;
  (pToData)[19] |= ((pFrom)->m_nPadWord2 >> 24) &0xFF;

  /* Pack Member: m_nSpareWord1 */
  (pToData)[8] |= ((pFrom)->m_nSpareWord1) & 0xFF;
  (pToData)[9] |= ((pFrom)->m_nSpareWord1 >> 8) &0xFF;
  (pToData)[10] |= ((pFrom)->m_nSpareWord1 >> 16) &0xFF;
  (pToData)[11] |= ((pFrom)->m_nSpareWord1 >> 24) &0xFF;
  (pToData)[12] |= ((pFrom)->m_nSpareWord1 >> 32) &0xFF;
  (pToData)[13] |= ((pFrom)->m_nSpareWord1 >> 40) &0xFF;
  (pToData)[14] |= ((pFrom)->m_nSpareWord1 >> 48) &0xFF;
  (pToData)[15] |= ((pFrom)->m_nSpareWord1 >> 56) &0xFF;

  /* Pack Member: m_nSpareWord0 */
  (pToData)[0] |= ((pFrom)->m_nSpareWord0) & 0xFF;
  (pToData)[1] |= ((pFrom)->m_nSpareWord0 >> 8) &0xFF;
  (pToData)[2] |= ((pFrom)->m_nSpareWord0 >> 16) &0xFF;
  (pToData)[3] |= ((pFrom)->m_nSpareWord0 >> 24) &0xFF;
  (pToData)[4] |= ((pFrom)->m_nSpareWord0 >> 32) &0xFF;
  (pToData)[5] |= ((pFrom)->m_nSpareWord0 >> 40) &0xFF;
  (pToData)[6] |= ((pFrom)->m_nSpareWord0 >> 48) &0xFF;
  (pToData)[7] |= ((pFrom)->m_nSpareWord0 >> 56) &0xFF;
#endif

  return SB_ZF_ZFKARBCLASSHASHSVLANIPV4_SIZE_IN_BYTES;
}




/* Unpack from array of bytes into struct */
void
sbZfKaRbClassHashSVlanIPv4_Unpack(sbZfKaRbClassHashSVlanIPv4_t *pToStruct,
                                  uint8 *pFromData,
                                  uint32 nMaxToDataIndex) {
  COMPILER_UINT64 tmp;

  (void) tmp;

#ifdef SAND_BIG_ENDIAN_HOST

  /* Unpack operation based on bigword endian */

  /* Unpack Member: m_nProtocol */
  (pToStruct)->m_nProtocol =  (uint32)  (pFromData)[44] ;

  /* Unpack Member: m_nPadWord3 */
  COMPILER_64_SET((pToStruct)->m_nPadWord3, 0,  (unsigned int) (pFromData)[36]);
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_nPadWord3;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[43]);
    COMPILER_64_SHL(tmp, 8);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_nPadWord3;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[42]);
    COMPILER_64_SHL(tmp, 16);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_nPadWord3;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[41]);
    COMPILER_64_SHL(tmp, 24);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_nPadWord3;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[40]);
    COMPILER_64_SHL(tmp, 32);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_nPadWord3;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[47]);
    COMPILER_64_SHL(tmp, 40);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_nPadWord3;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[46]);
    COMPILER_64_SHL(tmp, 48);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_nPadWord3;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[45]);
    COMPILER_64_SHL(tmp, 56);
    COMPILER_64_OR(*tmp0, tmp);
  };

  /* Unpack Member: m_nPadWord1 */
  COMPILER_64_SET((pToStruct)->m_nPadWord1, 0,  (unsigned int) (pFromData)[35]);
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_nPadWord1;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[34]);
    COMPILER_64_SHL(tmp, 8);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_nPadWord1;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[33]);
    COMPILER_64_SHL(tmp, 16);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_nPadWord1;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[32]);
    COMPILER_64_SHL(tmp, 24);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_nPadWord1;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[39]);
    COMPILER_64_SHL(tmp, 32);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_nPadWord1;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[38]);
    COMPILER_64_SHL(tmp, 40);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_nPadWord1;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[37]);
    COMPILER_64_SHL(tmp, 48);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_nPadWord1;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[36]);
    COMPILER_64_SHL(tmp, 56);
    COMPILER_64_OR(*tmp0, tmp);
  };

  /* Unpack Member: m_nIpSa */
  (pToStruct)->m_nIpSa =  (uint32)  (pFromData)[31] ;
  (pToStruct)->m_nIpSa |=  (uint32)  (pFromData)[30] << 8;
  (pToStruct)->m_nIpSa |=  (uint32)  (pFromData)[29] << 16;
  (pToStruct)->m_nIpSa |=  (uint32)  (pFromData)[28] << 24;

  /* Unpack Member: m_nIpDa */
  (pToStruct)->m_nIpDa =  (uint32)  (pFromData)[27] ;
  (pToStruct)->m_nIpDa |=  (uint32)  (pFromData)[26] << 8;
  (pToStruct)->m_nIpDa |=  (uint32)  (pFromData)[25] << 16;
  (pToStruct)->m_nIpDa |=  (uint32)  (pFromData)[24] << 24;

  /* Unpack Member: m_nSocket */
  (pToStruct)->m_nSocket =  (uint32)  (pFromData)[23] ;
  (pToStruct)->m_nSocket |=  (uint32)  (pFromData)[22] << 8;
  (pToStruct)->m_nSocket |=  (uint32)  (pFromData)[21] << 16;
  (pToStruct)->m_nSocket |=  (uint32)  (pFromData)[20] << 24;

  /* Unpack Member: m_nPadWord2 */
  (pToStruct)->m_nPadWord2 =  (uint32)  (pFromData)[19] ;
  (pToStruct)->m_nPadWord2 |=  (uint32)  (pFromData)[18] << 8;
  (pToStruct)->m_nPadWord2 |=  (uint32)  (pFromData)[17] << 16;
  (pToStruct)->m_nPadWord2 |=  (uint32)  (pFromData)[16] << 24;

  /* Unpack Member: m_nSpareWord1 */
  COMPILER_64_SET((pToStruct)->m_nSpareWord1, 0,  (unsigned int) (pFromData)[11]);
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_nSpareWord1;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[10]);
    COMPILER_64_SHL(tmp, 8);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_nSpareWord1;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[9]);
    COMPILER_64_SHL(tmp, 16);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_nSpareWord1;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[8]);
    COMPILER_64_SHL(tmp, 24);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_nSpareWord1;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[15]);
    COMPILER_64_SHL(tmp, 32);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_nSpareWord1;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[14]);
    COMPILER_64_SHL(tmp, 40);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_nSpareWord1;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[13]);
    COMPILER_64_SHL(tmp, 48);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_nSpareWord1;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[12]);
    COMPILER_64_SHL(tmp, 56);
    COMPILER_64_OR(*tmp0, tmp);
  };

  /* Unpack Member: m_nSpareWord0 */
  COMPILER_64_SET((pToStruct)->m_nSpareWord0, 0,  (unsigned int) (pFromData)[3]);
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_nSpareWord0;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[2]);
    COMPILER_64_SHL(tmp, 8);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_nSpareWord0;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[1]);
    COMPILER_64_SHL(tmp, 16);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_nSpareWord0;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[0]);
    COMPILER_64_SHL(tmp, 24);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_nSpareWord0;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[7]);
    COMPILER_64_SHL(tmp, 32);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_nSpareWord0;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[6]);
    COMPILER_64_SHL(tmp, 40);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_nSpareWord0;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[5]);
    COMPILER_64_SHL(tmp, 48);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_nSpareWord0;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[4]);
    COMPILER_64_SHL(tmp, 56);
    COMPILER_64_OR(*tmp0, tmp);
  };
#else

  /* Unpack operation based on little endian */

  /* Unpack Member: m_nProtocol */
  (pToStruct)->m_nProtocol =  (uint32)  (pFromData)[47] ;

  /* Unpack Member: m_nPadWord3 */
  COMPILER_64_SET((pToStruct)->m_nPadWord3, 0,  (unsigned int) (pFromData)[39]);
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_nPadWord3;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[40]);
    COMPILER_64_SHL(tmp, 8);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_nPadWord3;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[41]);
    COMPILER_64_SHL(tmp, 16);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_nPadWord3;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[42]);
    COMPILER_64_SHL(tmp, 24);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_nPadWord3;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[43]);
    COMPILER_64_SHL(tmp, 32);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_nPadWord3;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[44]);
    COMPILER_64_SHL(tmp, 40);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_nPadWord3;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[45]);
    COMPILER_64_SHL(tmp, 48);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_nPadWord3;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[46]);
    COMPILER_64_SHL(tmp, 56);
    COMPILER_64_OR(*tmp0, tmp);
  };

  /* Unpack Member: m_nPadWord1 */
  COMPILER_64_SET((pToStruct)->m_nPadWord1, 0,  (unsigned int) (pFromData)[32]);
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_nPadWord1;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[33]);
    COMPILER_64_SHL(tmp, 8);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_nPadWord1;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[34]);
    COMPILER_64_SHL(tmp, 16);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_nPadWord1;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[35]);
    COMPILER_64_SHL(tmp, 24);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_nPadWord1;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[36]);
    COMPILER_64_SHL(tmp, 32);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_nPadWord1;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[37]);
    COMPILER_64_SHL(tmp, 40);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_nPadWord1;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[38]);
    COMPILER_64_SHL(tmp, 48);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_nPadWord1;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[39]);
    COMPILER_64_SHL(tmp, 56);
    COMPILER_64_OR(*tmp0, tmp);
  };

  /* Unpack Member: m_nIpSa */
  (pToStruct)->m_nIpSa =  (uint32)  (pFromData)[28] ;
  (pToStruct)->m_nIpSa |=  (uint32)  (pFromData)[29] << 8;
  (pToStruct)->m_nIpSa |=  (uint32)  (pFromData)[30] << 16;
  (pToStruct)->m_nIpSa |=  (uint32)  (pFromData)[31] << 24;

  /* Unpack Member: m_nIpDa */
  (pToStruct)->m_nIpDa =  (uint32)  (pFromData)[24] ;
  (pToStruct)->m_nIpDa |=  (uint32)  (pFromData)[25] << 8;
  (pToStruct)->m_nIpDa |=  (uint32)  (pFromData)[26] << 16;
  (pToStruct)->m_nIpDa |=  (uint32)  (pFromData)[27] << 24;

  /* Unpack Member: m_nSocket */
  (pToStruct)->m_nSocket =  (uint32)  (pFromData)[20] ;
  (pToStruct)->m_nSocket |=  (uint32)  (pFromData)[21] << 8;
  (pToStruct)->m_nSocket |=  (uint32)  (pFromData)[22] << 16;
  (pToStruct)->m_nSocket |=  (uint32)  (pFromData)[23] << 24;

  /* Unpack Member: m_nPadWord2 */
  (pToStruct)->m_nPadWord2 =  (uint32)  (pFromData)[16] ;
  (pToStruct)->m_nPadWord2 |=  (uint32)  (pFromData)[17] << 8;
  (pToStruct)->m_nPadWord2 |=  (uint32)  (pFromData)[18] << 16;
  (pToStruct)->m_nPadWord2 |=  (uint32)  (pFromData)[19] << 24;

  /* Unpack Member: m_nSpareWord1 */
  COMPILER_64_SET((pToStruct)->m_nSpareWord1, 0,  (unsigned int) (pFromData)[8]);
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_nSpareWord1;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[9]);
    COMPILER_64_SHL(tmp, 8);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_nSpareWord1;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[10]);
    COMPILER_64_SHL(tmp, 16);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_nSpareWord1;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[11]);
    COMPILER_64_SHL(tmp, 24);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_nSpareWord1;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[12]);
    COMPILER_64_SHL(tmp, 32);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_nSpareWord1;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[13]);
    COMPILER_64_SHL(tmp, 40);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_nSpareWord1;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[14]);
    COMPILER_64_SHL(tmp, 48);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_nSpareWord1;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[15]);
    COMPILER_64_SHL(tmp, 56);
    COMPILER_64_OR(*tmp0, tmp);
  };

  /* Unpack Member: m_nSpareWord0 */
  COMPILER_64_SET((pToStruct)->m_nSpareWord0, 0,  (unsigned int) (pFromData)[0]);
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_nSpareWord0;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[1]);
    COMPILER_64_SHL(tmp, 8);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_nSpareWord0;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[2]);
    COMPILER_64_SHL(tmp, 16);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_nSpareWord0;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[3]);
    COMPILER_64_SHL(tmp, 24);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_nSpareWord0;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[4]);
    COMPILER_64_SHL(tmp, 32);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_nSpareWord0;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[5]);
    COMPILER_64_SHL(tmp, 40);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_nSpareWord0;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[6]);
    COMPILER_64_SHL(tmp, 48);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_nSpareWord0;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[7]);
    COMPILER_64_SHL(tmp, 56);
    COMPILER_64_OR(*tmp0, tmp);
  };
#endif

}



/* initialize an instance of this zframe */
void
sbZfKaRbClassHashSVlanIPv4_InitInstance(sbZfKaRbClassHashSVlanIPv4_t *pFrame) {

  pFrame->m_nProtocol =  (unsigned int)  0;
  COMPILER_64_ZERO(pFrame->m_nPadWord3);
  COMPILER_64_ZERO(pFrame->m_nPadWord1);
  pFrame->m_nIpSa =  (unsigned int)  0;
  pFrame->m_nIpDa =  (unsigned int)  0;
  pFrame->m_nSocket =  (unsigned int)  0;
  pFrame->m_nPadWord2 =  (unsigned int)  0;
  COMPILER_64_ZERO(pFrame->m_nSpareWord1);
  COMPILER_64_ZERO(pFrame->m_nSpareWord0);

}
