/*
 * $Id: sbZfSbQe2000ElibFMVT.c,v 1.5 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


#include "sbTypesGlue.h"
#include "sbZfSbQe2000ElibFMVT.hx"
#include "sbWrappers.h"
#include <sal/types.h>



/* Pack from struct into array of bytes */
uint32
sbZfSbQe2000ElibFMVT_Pack(sbZfSbQe2000ElibFMVT_t *pFrom,
                          uint8 *pToData,
                          uint32 nMaxToDataIndex) {
  int i;
  int size = SB_ZF_SB_QE2000_ELIB_FMVT_ENTRY_SIZE_IN_BYTES;

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on little endian */

  /* Pack Member: m_nReserved */
  (pToData)[31] |= ((pFrom)->m_nReserved & 0x01) <<7;

  /* Pack Member: m_nnPortMap2 */
  (pToData)[25] |= (COMPILER_64_LO((pFrom)->m_nnPortMap2) & 0x07) <<5;
  (pToData)[26] |= (COMPILER_64_LO((pFrom)->m_nnPortMap2) >> 3) &0xFF;
  (pToData)[27] |= (COMPILER_64_LO((pFrom)->m_nnPortMap2) >> 11) &0xFF;
  (pToData)[28] |= (COMPILER_64_LO((pFrom)->m_nnPortMap2) >> 19) &0xFF;
  (pToData)[29] |= (COMPILER_64_LO((pFrom)->m_nnPortMap2) >> 27) &0x1f;
  (pToData)[29] |= (COMPILER_64_HI((pFrom)->m_nnPortMap2) & 0x07) <<5;
  (pToData)[30] |= (COMPILER_64_HI((pFrom)->m_nnPortMap2) >> 3) &0xFF;
  (pToData)[31] |= (COMPILER_64_HI((pFrom)->m_nnPortMap2) >> 11) & 0x7f;

  /* Pack Member: m_nMvtda2 */
  (pToData)[23] |= ((pFrom)->m_nMvtda2 & 0x01) <<7;
  (pToData)[24] |= ((pFrom)->m_nMvtda2 >> 1) &0xFF;
  (pToData)[25] |= ((pFrom)->m_nMvtda2 >> 9) & 0x1f;

  /* Pack Member: m_nMvtdb2 */
  (pToData)[23] |= ((pFrom)->m_nMvtdb2 & 0x0f) <<3;

  /* Pack Member: m_nNext2 */
  (pToData)[21] |= ((pFrom)->m_nNext2 & 0x1f) <<3;
  (pToData)[22] |= ((pFrom)->m_nNext2 >> 5) &0xFF;
  (pToData)[23] |= ((pFrom)->m_nNext2 >> 13) & 0x07;

  /* Pack Member: m_nKnockout2 */
  (pToData)[21] |= ((pFrom)->m_nKnockout2 & 0x01) <<2;

  /* Pack Member: m_nnPortMap1 */
  (pToData)[15] |= (COMPILER_64_LO((pFrom)->m_nnPortMap1)) & 0xFF;
  (pToData)[16] |= (COMPILER_64_LO((pFrom)->m_nnPortMap1) >> 8) &0xFF;
  (pToData)[17] |= (COMPILER_64_LO((pFrom)->m_nnPortMap1) >> 16) &0xFF;
  (pToData)[18] |= (COMPILER_64_LO((pFrom)->m_nnPortMap1) >> 24) &0xFF;
  (pToData)[19] |= (COMPILER_64_HI((pFrom)->m_nnPortMap1)) &0xFF;
  (pToData)[20] |= (COMPILER_64_HI((pFrom)->m_nnPortMap1) >> 8) &0xFF;
  (pToData)[21] |= (COMPILER_64_HI((pFrom)->m_nnPortMap1) >> 16) & 0x03;

  /* Pack Member: m_nMvtda1 */
  (pToData)[13] |= ((pFrom)->m_nMvtda1 & 0x3f) <<2;
  (pToData)[14] |= ((pFrom)->m_nMvtda1 >> 6) &0xFF;

  /* Pack Member: m_nMvtdb1 */
  (pToData)[12] |= ((pFrom)->m_nMvtdb1 & 0x03) <<6;
  (pToData)[13] |= ((pFrom)->m_nMvtdb1 >> 2) & 0x03;

  /* Pack Member: m_nNext1 */
  (pToData)[10] |= ((pFrom)->m_nNext1 & 0x03) <<6;
  (pToData)[11] |= ((pFrom)->m_nNext1 >> 2) &0xFF;
  (pToData)[12] |= ((pFrom)->m_nNext1 >> 10) & 0x3f;

  /* Pack Member: m_nKnockout1 */
  (pToData)[10] |= ((pFrom)->m_nKnockout1 & 0x01) <<5;

  /* Pack Member: m_nnPortMap0 */
  (pToData)[4] |= (COMPILER_64_LO((pFrom)->m_nnPortMap0) & 0x1f) <<3;
  (pToData)[5] |= (COMPILER_64_LO((pFrom)->m_nnPortMap0) >> 5) &0xFF;
  (pToData)[6] |= (COMPILER_64_LO((pFrom)->m_nnPortMap0) >> 13) &0xFF;
  (pToData)[7] |= (COMPILER_64_LO((pFrom)->m_nnPortMap0) >> 21) &0xFF;
  (pToData)[8] |= (COMPILER_64_LO((pFrom)->m_nnPortMap0) >> 29) &0x7;
  (pToData)[8] |= (COMPILER_64_HI((pFrom)->m_nnPortMap0) & 0x1f) <<3;
  (pToData)[9] |= (COMPILER_64_HI((pFrom)->m_nnPortMap0) >> 5) &0xFF;
  (pToData)[10] |= (COMPILER_64_HI((pFrom)->m_nnPortMap0) >> 13) & 0x1f;

  /* Pack Member: m_nMvtda0 */
  (pToData)[2] |= ((pFrom)->m_nMvtda0 & 0x07) <<5;
  (pToData)[3] |= ((pFrom)->m_nMvtda0 >> 3) &0xFF;
  (pToData)[4] |= ((pFrom)->m_nMvtda0 >> 11) & 0x07;

  /* Pack Member: m_nMvtdb0 */
  (pToData)[2] |= ((pFrom)->m_nMvtdb0 & 0x0f) <<1;

  /* Pack Member: m_nNext0 */
  (pToData)[0] |= ((pFrom)->m_nNext0 & 0x7f) <<1;
  (pToData)[1] |= ((pFrom)->m_nNext0 >> 7) &0xFF;
  (pToData)[2] |= ((pFrom)->m_nNext0 >> 15) & 0x01;

  /* Pack Member: m_nKnockout0 */
  (pToData)[0] |= ((pFrom)->m_nKnockout0 & 0x01);

  return SB_ZF_SB_QE2000_ELIB_FMVT_ENTRY_SIZE_IN_BYTES;
}




/* Unpack from array of bytes into struct */
void
sbZfSbQe2000ElibFMVT_Unpack(sbZfSbQe2000ElibFMVT_t *pToStruct,
                            uint8 *pFromData,
                            uint32 nMaxToDataIndex) {
  COMPILER_UINT64 tmp;

  (void) tmp;


  /* Unpack operation based on little endian */

  /* Unpack Member: m_nReserved */
  (pToStruct)->m_nReserved =  (uint32)  ((pFromData)[31] >> 7) & 0x01;

  /* Unpack Member: m_nnPortMap2 */
  COMPILER_64_SET((pToStruct)->m_nnPortMap2, 0,  (unsigned int) (pFromData)[25]);
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_nnPortMap2;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[26]);
    COMPILER_64_SHL(tmp, 8);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_nnPortMap2;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[27]);
    COMPILER_64_SHL(tmp, 16);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_nnPortMap2;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[28]);
    COMPILER_64_SHL(tmp, 24);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_nnPortMap2;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[29]);
    COMPILER_64_SHL(tmp, 32);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_nnPortMap2;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[30]);
    COMPILER_64_SHL(tmp, 40);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_nnPortMap2;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[31]);
    COMPILER_64_SHL(tmp, 48);
    COMPILER_64_OR(*tmp0, tmp);
  };

  /* Unpack Member: m_nMvtda2 */
  (pToStruct)->m_nMvtda2 =  (uint32)  ((pFromData)[23] >> 7) & 0x01;
  (pToStruct)->m_nMvtda2 |=  (uint32)  (pFromData)[24] << 1;
  (pToStruct)->m_nMvtda2 |=  (uint32)  ((pFromData)[25] & 0x1f) << 9;

  /* Unpack Member: m_nMvtdb2 */
  (pToStruct)->m_nMvtdb2 =  (uint32)  ((pFromData)[23] >> 3) & 0x0f;

  /* Unpack Member: m_nNext2 */
  (pToStruct)->m_nNext2 =  (uint32)  ((pFromData)[21] >> 3) & 0x1f;
  (pToStruct)->m_nNext2 |=  (uint32)  (pFromData)[22] << 5;
  (pToStruct)->m_nNext2 |=  (uint32)  ((pFromData)[23] & 0x07) << 13;

  /* Unpack Member: m_nKnockout2 */
  (pToStruct)->m_nKnockout2 =  (uint32)  ((pFromData)[21] >> 2) & 0x01;

  /* Unpack Member: m_nnPortMap1 */
  COMPILER_64_SET((pToStruct)->m_nnPortMap1, 0,  (unsigned int) (pFromData)[15]);
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_nnPortMap1;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[16]);
    COMPILER_64_SHL(tmp, 8);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_nnPortMap1;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[17]);
    COMPILER_64_SHL(tmp, 16);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_nnPortMap1;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[18]);
    COMPILER_64_SHL(tmp, 24);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_nnPortMap1;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[19]);
    COMPILER_64_SHL(tmp, 32);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_nnPortMap1;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[20]);
    COMPILER_64_SHL(tmp, 40);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_nnPortMap1;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[21]);
    COMPILER_64_SHL(tmp, 48);
    COMPILER_64_OR(*tmp0, tmp);
  };

  /* Unpack Member: m_nMvtda1 */
  (pToStruct)->m_nMvtda1 =  (uint32)  ((pFromData)[13] >> 2) & 0x3f;
  (pToStruct)->m_nMvtda1 |=  (uint32)  (pFromData)[14] << 6;

  /* Unpack Member: m_nMvtdb1 */
  (pToStruct)->m_nMvtdb1 =  (uint32)  ((pFromData)[12] >> 6) & 0x03;
  (pToStruct)->m_nMvtdb1 |=  (uint32)  ((pFromData)[13] & 0x03) << 2;

  /* Unpack Member: m_nNext1 */
  (pToStruct)->m_nNext1 =  (uint32)  ((pFromData)[10] >> 6) & 0x03;
  (pToStruct)->m_nNext1 |=  (uint32)  (pFromData)[11] << 2;
  (pToStruct)->m_nNext1 |=  (uint32)  ((pFromData)[12] & 0x3f) << 10;

  /* Unpack Member: m_nKnockout1 */
  (pToStruct)->m_nKnockout1 =  (uint32)  ((pFromData)[10] >> 5) & 0x01;

  /* Unpack Member: m_nnPortMap0 */
  COMPILER_64_SET((pToStruct)->m_nnPortMap0, 0,  (unsigned int) (pFromData)[4]);
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_nnPortMap0;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[5]);
    COMPILER_64_SHL(tmp, 8);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_nnPortMap0;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[6]);
    COMPILER_64_SHL(tmp, 16);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_nnPortMap0;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[7]);
    COMPILER_64_SHL(tmp, 24);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_nnPortMap0;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[8]);
    COMPILER_64_SHL(tmp, 32);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_nnPortMap0;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[9]);
    COMPILER_64_SHL(tmp, 40);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_nnPortMap0;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[10]);
    COMPILER_64_SHL(tmp, 48);
    COMPILER_64_OR(*tmp0, tmp);
  };

  /* Unpack Member: m_nMvtda0 */
  (pToStruct)->m_nMvtda0 =  (uint32)  ((pFromData)[2] >> 5) & 0x07;
  (pToStruct)->m_nMvtda0 |=  (uint32)  (pFromData)[3] << 3;
  (pToStruct)->m_nMvtda0 |=  (uint32)  ((pFromData)[4] & 0x07) << 11;

  /* Unpack Member: m_nMvtdb0 */
  (pToStruct)->m_nMvtdb0 =  (uint32)  ((pFromData)[2] >> 1) & 0x0f;

  /* Unpack Member: m_nNext0 */
  (pToStruct)->m_nNext0 =  (uint32)  ((pFromData)[0] >> 1) & 0x7f;
  (pToStruct)->m_nNext0 |=  (uint32)  (pFromData)[1] << 7;
  (pToStruct)->m_nNext0 |=  (uint32)  ((pFromData)[2] & 0x01) << 15;

  /* Unpack Member: m_nKnockout0 */
  (pToStruct)->m_nKnockout0 =  (uint32)  ((pFromData)[0] ) & 0x01;

}



/* initialize an instance of this zframe */
void
sbZfSbQe2000ElibFMVT_InitInstance(sbZfSbQe2000ElibFMVT_t *pFrame) {

  pFrame->m_nReserved =  (unsigned int)  0;
  COMPILER_64_ZERO(pFrame->m_nnPortMap2);
  pFrame->m_nMvtda2 =  (unsigned int)  0;
  pFrame->m_nMvtdb2 =  (unsigned int)  0;
  pFrame->m_nNext2 =  (unsigned int)  0;
  pFrame->m_nKnockout2 =  (unsigned int)  0;
  COMPILER_64_ZERO(pFrame->m_nnPortMap1);
  pFrame->m_nMvtda1 =  (unsigned int)  0;
  pFrame->m_nMvtdb1 =  (unsigned int)  0;
  pFrame->m_nNext1 =  (unsigned int)  0;
  pFrame->m_nKnockout1 =  (unsigned int)  0;
  COMPILER_64_ZERO(pFrame->m_nnPortMap0);
  pFrame->m_nMvtda0 =  (unsigned int)  0;
  pFrame->m_nMvtdb0 =  (unsigned int)  0;
  pFrame->m_nNext0 =  (unsigned int)  0;
  pFrame->m_nKnockout0 =  (unsigned int)  0;

}
