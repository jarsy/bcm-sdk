/*
 * $Id: sbZfSbQe2000ElibMVT.c,v 1.5 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


#include "sbTypesGlue.h"
#include "sbZfSbQe2000ElibMVT.hx"
#include "sbWrappers.h"
#include <sal/types.h>



/* Pack from struct into array of bytes */
uint32
sbZfSbQe2000ElibMVT_Pack(sbZfSbQe2000ElibMVT_t *pFrom,
                         uint8 *pToData,
                         uint32 nMaxToDataIndex) {
  int i;
  int size = SB_ZF_SB_QE2000_ELIB_MVT_ENTRY_SIZE_IN_BYTES;

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on little endian */

  /* Pack Member: m_nReserved */
  (pToData)[10] |= ((pFrom)->m_nReserved & 0x07) <<5;

  /* Pack Member: m_nnPortMap */
  (pToData)[4] |= (COMPILER_64_LO((pFrom)->m_nnPortMap) & 0x1f) <<3;
  (pToData)[5] |= (COMPILER_64_LO((pFrom)->m_nnPortMap) >> 5) &0xFF;
  (pToData)[6] |= (COMPILER_64_LO((pFrom)->m_nnPortMap) >> 13) &0xFF;
  (pToData)[7] |= (COMPILER_64_LO((pFrom)->m_nnPortMap) >> 21) &0xFF;
  (pToData)[8] |= (COMPILER_64_LO((pFrom)->m_nnPortMap) >> 29) &0x7;
  (pToData)[8] |= (COMPILER_64_HI((pFrom)->m_nnPortMap) & 0x1f) <<3;
  (pToData)[9] |= (COMPILER_64_HI((pFrom)->m_nnPortMap) >> 5) &0xFF;
  (pToData)[10] |= (COMPILER_64_HI((pFrom)->m_nnPortMap) >> 13) & 0x1f;

  /* Pack Member: m_nMvtda */
  (pToData)[2] |= ((pFrom)->m_nMvtda & 0x07) <<5;
  (pToData)[3] |= ((pFrom)->m_nMvtda >> 3) &0xFF;
  (pToData)[4] |= ((pFrom)->m_nMvtda >> 11) & 0x07;

  /* Pack Member: m_nMvtdb */
  (pToData)[2] |= ((pFrom)->m_nMvtdb & 0x0f) <<1;

  /* Pack Member: m_nNext */
  (pToData)[0] |= ((pFrom)->m_nNext & 0x7f) <<1;
  (pToData)[1] |= ((pFrom)->m_nNext >> 7) &0xFF;
  (pToData)[2] |= ((pFrom)->m_nNext >> 15) & 0x01;

  /* Pack Member: m_nKnockout */
  (pToData)[0] |= ((pFrom)->m_nKnockout & 0x01);

  return SB_ZF_SB_QE2000_ELIB_MVT_ENTRY_SIZE_IN_BYTES;
}




/* Unpack from array of bytes into struct */
void
sbZfSbQe2000ElibMVT_Unpack(sbZfSbQe2000ElibMVT_t *pToStruct,
                           uint8 *pFromData,
                           uint32 nMaxToDataIndex) {
  COMPILER_UINT64 tmp;

  (void) tmp;


  /* Unpack operation based on little endian */

  /* Unpack Member: m_nReserved */
  (pToStruct)->m_nReserved =  (uint32)  ((pFromData)[10] >> 5) & 0x07;

  /* Unpack Member: m_nnPortMap */
  COMPILER_64_SET((pToStruct)->m_nnPortMap, 0,  (unsigned int) (pFromData)[4]);
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_nnPortMap;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[5]);
    COMPILER_64_SHL(tmp, 8);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_nnPortMap;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[6]);
    COMPILER_64_SHL(tmp, 16);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_nnPortMap;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[7]);
    COMPILER_64_SHL(tmp, 24);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_nnPortMap;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[8]);
    COMPILER_64_SHL(tmp, 32);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_nnPortMap;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[9]);
    COMPILER_64_SHL(tmp, 40);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_nnPortMap;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[10]);
    COMPILER_64_SHL(tmp, 48);
    COMPILER_64_OR(*tmp0, tmp);
  };

  /* Unpack Member: m_nMvtda */
  (pToStruct)->m_nMvtda =  (uint32)  ((pFromData)[2] >> 5) & 0x07;
  (pToStruct)->m_nMvtda |=  (uint32)  (pFromData)[3] << 3;
  (pToStruct)->m_nMvtda |=  (uint32)  ((pFromData)[4] & 0x07) << 11;

  /* Unpack Member: m_nMvtdb */
  (pToStruct)->m_nMvtdb =  (uint32)  ((pFromData)[2] >> 1) & 0x0f;

  /* Unpack Member: m_nNext */
  (pToStruct)->m_nNext =  (uint32)  ((pFromData)[0] >> 1) & 0x7f;
  (pToStruct)->m_nNext |=  (uint32)  (pFromData)[1] << 7;
  (pToStruct)->m_nNext |=  (uint32)  ((pFromData)[2] & 0x01) << 15;

  /* Unpack Member: m_nKnockout */
  (pToStruct)->m_nKnockout =  (uint32)  ((pFromData)[0] ) & 0x01;

}



/* initialize an instance of this zframe */
void
sbZfSbQe2000ElibMVT_InitInstance(sbZfSbQe2000ElibMVT_t *pFrame) {

  pFrame->m_nReserved =  (unsigned int)  0;
  COMPILER_64_ZERO(pFrame->m_nnPortMap);
  pFrame->m_nMvtda =  (unsigned int)  0;
  pFrame->m_nMvtdb =  (unsigned int)  0;
  pFrame->m_nNext =  (unsigned int)  0;
  pFrame->m_nKnockout =  (unsigned int)  0;

}
