/*
 * $Id: sbZfSbQe2000ElibPCTSingle.c,v 1.5 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


#include "sbTypesGlue.h"
#include "sbZfSbQe2000ElibPCTSingle.hx"
#include "sbWrappers.h"
#include <sal/types.h>



/* Pack from struct into array of bytes */
uint32
sbZfSbQe2000ElibPCTSingle_Pack(sbZfSbQe2000ElibPCTSingle_t *pFrom,
                               uint8 *pToData,
                               uint32 nMaxToDataIndex) {
  int i;
  int size = SB_ZF_SB_QE2000_ELIB_PCTSINGLE_ENTRY_SIZE_IN_BYTES;

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on little endian */

  /* Pack Member: m_PktClass */
  (pToData)[4] |= (COMPILER_64_LO((pFrom)->m_PktClass) & 0x1f) <<3;
  (pToData)[5] |= (COMPILER_64_LO((pFrom)->m_PktClass) >> 5) &0xFF;
  (pToData)[6] |= (COMPILER_64_LO((pFrom)->m_PktClass) >> 13) &0xFF;
  (pToData)[7] |= (COMPILER_64_LO((pFrom)->m_PktClass) >> 21) &0xFF;

  /* Pack Member: m_ByteClass */
  (pToData)[0] |= (COMPILER_64_LO((pFrom)->m_ByteClass)) & 0xFF;
  (pToData)[1] |= (COMPILER_64_LO((pFrom)->m_ByteClass) >> 8) &0xFF;
  (pToData)[2] |= (COMPILER_64_LO((pFrom)->m_ByteClass) >> 16) &0xFF;
  (pToData)[3] |= (COMPILER_64_LO((pFrom)->m_ByteClass) >> 24) &0xFF;
  (pToData)[4] |= (COMPILER_64_HI((pFrom)->m_ByteClass)) & 0x07;

  return SB_ZF_SB_QE2000_ELIB_PCTSINGLE_ENTRY_SIZE_IN_BYTES;
}




/* Unpack from array of bytes into struct */
void
sbZfSbQe2000ElibPCTSingle_Unpack(sbZfSbQe2000ElibPCTSingle_t *pToStruct,
                                 uint8 *pFromData,
                                 uint32 nMaxToDataIndex) {
  COMPILER_UINT64 tmp;

  (void) tmp;


  /* Unpack operation based on little endian */

  /* Unpack Member: m_PktClass */
  COMPILER_64_SET((pToStruct)->m_PktClass, 0,  (unsigned int) (pFromData)[4]);
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_PktClass;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[5]);
    COMPILER_64_SHL(tmp, 8);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_PktClass;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[6]);
    COMPILER_64_SHL(tmp, 16);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_PktClass;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[7]);
    COMPILER_64_SHL(tmp, 24);
    COMPILER_64_OR(*tmp0, tmp);
  };

  /* Unpack Member: m_ByteClass */
  COMPILER_64_SET((pToStruct)->m_ByteClass, 0,  (unsigned int) (pFromData)[0]);
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_ByteClass;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[1]);
    COMPILER_64_SHL(tmp, 8);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_ByteClass;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[2]);
    COMPILER_64_SHL(tmp, 16);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_ByteClass;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[3]);
    COMPILER_64_SHL(tmp, 24);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_ByteClass;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[4]);
    COMPILER_64_SHL(tmp, 32);
    COMPILER_64_OR(*tmp0, tmp);
  };

}



/* initialize an instance of this zframe */
void
sbZfSbQe2000ElibPCTSingle_InitInstance(sbZfSbQe2000ElibPCTSingle_t *pFrame) {

  COMPILER_64_ZERO(pFrame->m_PktClass);
  COMPILER_64_ZERO(pFrame->m_ByteClass);

}
