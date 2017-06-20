/*
 * $Id: sbZfSbQe2000ElibPCT.c,v 1.5 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


#include "sbTypesGlue.h"
#include "sbZfSbQe2000ElibPCT.hx"
#include "sbWrappers.h"
#include <sal/types.h>



/* Pack from struct into array of bytes */
uint32
sbZfSbQe2000ElibPCT_Pack(sbZfSbQe2000ElibPCT_t *pFrom,
                         uint8 *pToData,
                         uint32 nMaxToDataIndex) {
  int i;
  int size = SB_ZF_SB_QE2000_ELIB_PCT_ENTRY_SIZE_IN_BYTES;

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on little endian */

  /* Pack Member: m_PktClass15 */
  (pToData)[124] |= (COMPILER_64_LO((pFrom)->m_PktClass15) & 0x1f) <<3;
  (pToData)[125] |= (COMPILER_64_LO((pFrom)->m_PktClass15) >> 5) &0xFF;
  (pToData)[126] |= (COMPILER_64_LO((pFrom)->m_PktClass15) >> 13) &0xFF;
  (pToData)[127] |= (COMPILER_64_LO((pFrom)->m_PktClass15) >> 21) &0xFF;

  /* Pack Member: m_ByteClass15 */
  (pToData)[120] |= (COMPILER_64_LO((pFrom)->m_ByteClass15)) & 0xFF;
  (pToData)[121] |= (COMPILER_64_LO((pFrom)->m_ByteClass15) >> 8) &0xFF;
  (pToData)[122] |= (COMPILER_64_LO((pFrom)->m_ByteClass15) >> 16) &0xFF;
  (pToData)[123] |= (COMPILER_64_LO((pFrom)->m_ByteClass15) >> 24) &0xFF;
  (pToData)[124] |= (COMPILER_64_HI((pFrom)->m_ByteClass15)) & 0x07;

  /* Pack Member: m_PktClass14 */
  (pToData)[116] |= (COMPILER_64_LO((pFrom)->m_PktClass14) & 0x1f) <<3;
  (pToData)[117] |= (COMPILER_64_LO((pFrom)->m_PktClass14) >> 5) &0xFF;
  (pToData)[118] |= (COMPILER_64_LO((pFrom)->m_PktClass14) >> 13) &0xFF;
  (pToData)[119] |= (COMPILER_64_LO((pFrom)->m_PktClass14) >> 21) &0xFF;

  /* Pack Member: m_ByteClass14 */
  (pToData)[112] |= (COMPILER_64_LO((pFrom)->m_ByteClass14)) & 0xFF;
  (pToData)[113] |= (COMPILER_64_LO((pFrom)->m_ByteClass14) >> 8) &0xFF;
  (pToData)[114] |= (COMPILER_64_LO((pFrom)->m_ByteClass14) >> 16) &0xFF;
  (pToData)[115] |= (COMPILER_64_LO((pFrom)->m_ByteClass14) >> 24) &0xFF;
  (pToData)[116] |= (COMPILER_64_HI((pFrom)->m_ByteClass14)) & 0x07;

  /* Pack Member: m_PktClass13 */
  (pToData)[108] |= (COMPILER_64_LO((pFrom)->m_PktClass13) & 0x1f) <<3;
  (pToData)[109] |= (COMPILER_64_LO((pFrom)->m_PktClass13) >> 5) &0xFF;
  (pToData)[110] |= (COMPILER_64_LO((pFrom)->m_PktClass13) >> 13) &0xFF;
  (pToData)[111] |= (COMPILER_64_LO((pFrom)->m_PktClass13) >> 21) &0xFF;

  /* Pack Member: m_ByteClass13 */
  (pToData)[104] |= (COMPILER_64_LO((pFrom)->m_ByteClass13)) & 0xFF;
  (pToData)[105] |= (COMPILER_64_LO((pFrom)->m_ByteClass13) >> 8) &0xFF;
  (pToData)[106] |= (COMPILER_64_LO((pFrom)->m_ByteClass13) >> 16) &0xFF;
  (pToData)[107] |= (COMPILER_64_LO((pFrom)->m_ByteClass13) >> 24) &0xFF;
  (pToData)[108] |= (COMPILER_64_HI((pFrom)->m_ByteClass13)) & 0x07;

  /* Pack Member: m_PktClass12 */
  (pToData)[100] |= (COMPILER_64_LO((pFrom)->m_PktClass12) & 0x1f) <<3;
  (pToData)[101] |= (COMPILER_64_LO((pFrom)->m_PktClass12) >> 5) &0xFF;
  (pToData)[102] |= (COMPILER_64_LO((pFrom)->m_PktClass12) >> 13) &0xFF;
  (pToData)[103] |= (COMPILER_64_LO((pFrom)->m_PktClass12) >> 21) &0xFF;

  /* Pack Member: m_ByteClass12 */
  (pToData)[96] |= (COMPILER_64_LO((pFrom)->m_ByteClass12)) & 0xFF;
  (pToData)[97] |= (COMPILER_64_LO((pFrom)->m_ByteClass12) >> 8) &0xFF;
  (pToData)[98] |= (COMPILER_64_LO((pFrom)->m_ByteClass12) >> 16) &0xFF;
  (pToData)[99] |= (COMPILER_64_LO((pFrom)->m_ByteClass12) >> 24) &0xFF;
  (pToData)[100] |= (COMPILER_64_HI((pFrom)->m_ByteClass12)) & 0x07;

  /* Pack Member: m_PktClass11 */
  (pToData)[92] |= (COMPILER_64_LO((pFrom)->m_PktClass11) & 0x1f) <<3;
  (pToData)[93] |= (COMPILER_64_LO((pFrom)->m_PktClass11) >> 5) &0xFF;
  (pToData)[94] |= (COMPILER_64_LO((pFrom)->m_PktClass11) >> 13) &0xFF;
  (pToData)[95] |= (COMPILER_64_LO((pFrom)->m_PktClass11) >> 21) &0xFF;

  /* Pack Member: m_ByteClass11 */
  (pToData)[88] |= (COMPILER_64_LO((pFrom)->m_ByteClass11)) & 0xFF;
  (pToData)[89] |= (COMPILER_64_LO((pFrom)->m_ByteClass11) >> 8) &0xFF;
  (pToData)[90] |= (COMPILER_64_LO((pFrom)->m_ByteClass11) >> 16) &0xFF;
  (pToData)[91] |= (COMPILER_64_LO((pFrom)->m_ByteClass11) >> 24) &0xFF;
  (pToData)[92] |= (COMPILER_64_HI((pFrom)->m_ByteClass11)) & 0x07;

  /* Pack Member: m_PktClass10 */
  (pToData)[84] |= (COMPILER_64_LO((pFrom)->m_PktClass10) & 0x1f) <<3;
  (pToData)[85] |= (COMPILER_64_LO((pFrom)->m_PktClass10) >> 5) &0xFF;
  (pToData)[86] |= (COMPILER_64_LO((pFrom)->m_PktClass10) >> 13) &0xFF;
  (pToData)[87] |= (COMPILER_64_LO((pFrom)->m_PktClass10) >> 21) &0xFF;

  /* Pack Member: m_ByteClass10 */
  (pToData)[80] |= (COMPILER_64_LO((pFrom)->m_ByteClass10)) & 0xFF;
  (pToData)[81] |= (COMPILER_64_LO((pFrom)->m_ByteClass10) >> 8) &0xFF;
  (pToData)[82] |= (COMPILER_64_LO((pFrom)->m_ByteClass10) >> 16) &0xFF;
  (pToData)[83] |= (COMPILER_64_LO((pFrom)->m_ByteClass10) >> 24) &0xFF;
  (pToData)[84] |= (COMPILER_64_HI((pFrom)->m_ByteClass10)) & 0x07;

  /* Pack Member: m_PktClass9 */
  (pToData)[76] |= (COMPILER_64_LO((pFrom)->m_PktClass9) & 0x1f) <<3;
  (pToData)[77] |= (COMPILER_64_LO((pFrom)->m_PktClass9) >> 5) &0xFF;
  (pToData)[78] |= (COMPILER_64_LO((pFrom)->m_PktClass9) >> 13) &0xFF;
  (pToData)[79] |= (COMPILER_64_LO((pFrom)->m_PktClass9) >> 21) &0xFF;

  /* Pack Member: m_ByteClass9 */
  (pToData)[72] |= (COMPILER_64_LO((pFrom)->m_ByteClass9)) & 0xFF;
  (pToData)[73] |= (COMPILER_64_LO((pFrom)->m_ByteClass9) >> 8) &0xFF;
  (pToData)[74] |= (COMPILER_64_LO((pFrom)->m_ByteClass9) >> 16) &0xFF;
  (pToData)[75] |= (COMPILER_64_LO((pFrom)->m_ByteClass9) >> 24) &0xFF;
  (pToData)[76] |= (COMPILER_64_HI((pFrom)->m_ByteClass9)) & 0x07;

  /* Pack Member: m_PktClass8 */
  (pToData)[68] |= (COMPILER_64_LO((pFrom)->m_PktClass8) & 0x1f) <<3;
  (pToData)[69] |= (COMPILER_64_LO((pFrom)->m_PktClass8) >> 5) &0xFF;
  (pToData)[70] |= (COMPILER_64_LO((pFrom)->m_PktClass8) >> 13) &0xFF;
  (pToData)[71] |= (COMPILER_64_LO((pFrom)->m_PktClass8) >> 21) &0xFF;

  /* Pack Member: m_ByteClass8 */
  (pToData)[64] |= (COMPILER_64_LO((pFrom)->m_ByteClass8)) & 0xFF;
  (pToData)[65] |= (COMPILER_64_LO((pFrom)->m_ByteClass8) >> 8) &0xFF;
  (pToData)[66] |= (COMPILER_64_LO((pFrom)->m_ByteClass8) >> 16) &0xFF;
  (pToData)[67] |= (COMPILER_64_LO((pFrom)->m_ByteClass8) >> 24) &0xFF;
  (pToData)[68] |= (COMPILER_64_HI((pFrom)->m_ByteClass8)) & 0x07;

  /* Pack Member: m_PktClass7 */
  (pToData)[60] |= (COMPILER_64_LO((pFrom)->m_PktClass7) & 0x1f) <<3;
  (pToData)[61] |= (COMPILER_64_LO((pFrom)->m_PktClass7) >> 5) &0xFF;
  (pToData)[62] |= (COMPILER_64_LO((pFrom)->m_PktClass7) >> 13) &0xFF;
  (pToData)[63] |= (COMPILER_64_LO((pFrom)->m_PktClass7) >> 21) &0xFF;

  /* Pack Member: m_ByteClass7 */
  (pToData)[56] |= (COMPILER_64_LO((pFrom)->m_ByteClass7)) & 0xFF;
  (pToData)[57] |= (COMPILER_64_LO((pFrom)->m_ByteClass7) >> 8) &0xFF;
  (pToData)[58] |= (COMPILER_64_LO((pFrom)->m_ByteClass7) >> 16) &0xFF;
  (pToData)[59] |= (COMPILER_64_LO((pFrom)->m_ByteClass7) >> 24) &0xFF;
  (pToData)[60] |= (COMPILER_64_HI((pFrom)->m_ByteClass7)) & 0x07;

  /* Pack Member: m_PktClass6 */
  (pToData)[52] |= (COMPILER_64_LO((pFrom)->m_PktClass6) & 0x1f) <<3;
  (pToData)[53] |= (COMPILER_64_LO((pFrom)->m_PktClass6) >> 5) &0xFF;
  (pToData)[54] |= (COMPILER_64_LO((pFrom)->m_PktClass6) >> 13) &0xFF;
  (pToData)[55] |= (COMPILER_64_LO((pFrom)->m_PktClass6) >> 21) &0xFF;

  /* Pack Member: m_ByteClass6 */
  (pToData)[48] |= (COMPILER_64_LO((pFrom)->m_ByteClass6)) & 0xFF;
  (pToData)[49] |= (COMPILER_64_LO((pFrom)->m_ByteClass6) >> 8) &0xFF;
  (pToData)[50] |= (COMPILER_64_LO((pFrom)->m_ByteClass6) >> 16) &0xFF;
  (pToData)[51] |= (COMPILER_64_LO((pFrom)->m_ByteClass6) >> 24) &0xFF;
  (pToData)[52] |= (COMPILER_64_HI((pFrom)->m_ByteClass6)) & 0x07;

  /* Pack Member: m_PktClass5 */
  (pToData)[44] |= (COMPILER_64_LO((pFrom)->m_PktClass5) & 0x1f) <<3;
  (pToData)[45] |= (COMPILER_64_LO((pFrom)->m_PktClass5) >> 5) &0xFF;
  (pToData)[46] |= (COMPILER_64_LO((pFrom)->m_PktClass5) >> 13) &0xFF;
  (pToData)[47] |= (COMPILER_64_LO((pFrom)->m_PktClass5) >> 21) &0xFF;

  /* Pack Member: m_ByteClass5 */
  (pToData)[40] |= (COMPILER_64_LO((pFrom)->m_ByteClass5)) & 0xFF;
  (pToData)[41] |= (COMPILER_64_LO((pFrom)->m_ByteClass5) >> 8) &0xFF;
  (pToData)[42] |= (COMPILER_64_LO((pFrom)->m_ByteClass5) >> 16) &0xFF;
  (pToData)[43] |= (COMPILER_64_LO((pFrom)->m_ByteClass5) >> 24) &0xFF;
  (pToData)[44] |= (COMPILER_64_HI((pFrom)->m_ByteClass5)) & 0x07;

  /* Pack Member: m_PktClass4 */
  (pToData)[36] |= (COMPILER_64_LO((pFrom)->m_PktClass4) & 0x1f) <<3;
  (pToData)[37] |= (COMPILER_64_LO((pFrom)->m_PktClass4) >> 5) &0xFF;
  (pToData)[38] |= (COMPILER_64_LO((pFrom)->m_PktClass4) >> 13) &0xFF;
  (pToData)[39] |= (COMPILER_64_LO((pFrom)->m_PktClass4) >> 21) &0xFF;

  /* Pack Member: m_ByteClass4 */
  (pToData)[32] |= (COMPILER_64_LO((pFrom)->m_ByteClass4)) & 0xFF;
  (pToData)[33] |= (COMPILER_64_LO((pFrom)->m_ByteClass4) >> 8) &0xFF;
  (pToData)[34] |= (COMPILER_64_LO((pFrom)->m_ByteClass4) >> 16) &0xFF;
  (pToData)[35] |= (COMPILER_64_LO((pFrom)->m_ByteClass4) >> 24) &0xFF;
  (pToData)[36] |= (COMPILER_64_HI((pFrom)->m_ByteClass4)) & 0x07;

  /* Pack Member: m_PktClass3 */
  (pToData)[28] |= (COMPILER_64_LO((pFrom)->m_PktClass3) & 0x1f) <<3;
  (pToData)[29] |= (COMPILER_64_LO((pFrom)->m_PktClass3) >> 5) &0xFF;
  (pToData)[30] |= (COMPILER_64_LO((pFrom)->m_PktClass3) >> 13) &0xFF;
  (pToData)[31] |= (COMPILER_64_LO((pFrom)->m_PktClass3) >> 21) &0xFF;

  /* Pack Member: m_ByteClass3 */
  (pToData)[24] |= (COMPILER_64_LO((pFrom)->m_ByteClass3)) & 0xFF;
  (pToData)[25] |= (COMPILER_64_LO((pFrom)->m_ByteClass3) >> 8) &0xFF;
  (pToData)[26] |= (COMPILER_64_LO((pFrom)->m_ByteClass3) >> 16) &0xFF;
  (pToData)[27] |= (COMPILER_64_LO((pFrom)->m_ByteClass3) >> 24) &0xFF;
  (pToData)[28] |= (COMPILER_64_HI((pFrom)->m_ByteClass3)) & 0x07;

  /* Pack Member: m_PktClass2 */
  (pToData)[20] |= (COMPILER_64_LO((pFrom)->m_PktClass2) & 0x1f) <<3;
  (pToData)[21] |= (COMPILER_64_LO((pFrom)->m_PktClass2) >> 5) &0xFF;
  (pToData)[22] |= (COMPILER_64_LO((pFrom)->m_PktClass2) >> 13) &0xFF;
  (pToData)[23] |= (COMPILER_64_LO((pFrom)->m_PktClass2) >> 21) &0xFF;

  /* Pack Member: m_ByteClass2 */
  (pToData)[16] |= (COMPILER_64_LO((pFrom)->m_ByteClass2)) & 0xFF;
  (pToData)[17] |= (COMPILER_64_LO((pFrom)->m_ByteClass2) >> 8) &0xFF;
  (pToData)[18] |= (COMPILER_64_LO((pFrom)->m_ByteClass2) >> 16) &0xFF;
  (pToData)[19] |= (COMPILER_64_LO((pFrom)->m_ByteClass2) >> 24) &0xFF;
  (pToData)[20] |= (COMPILER_64_HI((pFrom)->m_ByteClass2)) & 0x07;

  /* Pack Member: m_PktClass1 */
  (pToData)[12] |= (COMPILER_64_LO((pFrom)->m_PktClass1) & 0x1f) <<3;
  (pToData)[13] |= (COMPILER_64_LO((pFrom)->m_PktClass1) >> 5) &0xFF;
  (pToData)[14] |= (COMPILER_64_LO((pFrom)->m_PktClass1) >> 13) &0xFF;
  (pToData)[15] |= (COMPILER_64_LO((pFrom)->m_PktClass1) >> 21) &0xFF;

  /* Pack Member: m_ByteClass1 */
  (pToData)[8] |= (COMPILER_64_LO((pFrom)->m_ByteClass1)) & 0xFF;
  (pToData)[9] |= (COMPILER_64_LO((pFrom)->m_ByteClass1) >> 8) &0xFF;
  (pToData)[10] |= (COMPILER_64_LO((pFrom)->m_ByteClass1) >> 16) &0xFF;
  (pToData)[11] |= (COMPILER_64_LO((pFrom)->m_ByteClass1) >> 24) &0xFF;
  (pToData)[12] |= (COMPILER_64_HI((pFrom)->m_ByteClass1)) & 0x07;

  /* Pack Member: m_PktClass0 */
  (pToData)[4] |= (COMPILER_64_LO((pFrom)->m_PktClass0) & 0x1f) <<3;
  (pToData)[5] |= (COMPILER_64_LO((pFrom)->m_PktClass0) >> 5) &0xFF;
  (pToData)[6] |= (COMPILER_64_LO((pFrom)->m_PktClass0) >> 13) &0xFF;
  (pToData)[7] |= (COMPILER_64_LO((pFrom)->m_PktClass0) >> 21) &0xFF;

  /* Pack Member: m_ByteClass0 */
  (pToData)[0] |= (COMPILER_64_LO((pFrom)->m_ByteClass0)) & 0xFF;
  (pToData)[1] |= (COMPILER_64_LO((pFrom)->m_ByteClass0) >> 8) &0xFF;
  (pToData)[2] |= (COMPILER_64_LO((pFrom)->m_ByteClass0) >> 16) &0xFF;
  (pToData)[3] |= (COMPILER_64_LO((pFrom)->m_ByteClass0) >> 24) &0xFF;
  (pToData)[4] |= (COMPILER_64_HI((pFrom)->m_ByteClass0)) & 0x07;

  return SB_ZF_SB_QE2000_ELIB_PCT_ENTRY_SIZE_IN_BYTES;
}




/* Unpack from array of bytes into struct */
void
sbZfSbQe2000ElibPCT_Unpack(sbZfSbQe2000ElibPCT_t *pToStruct,
                           uint8 *pFromData,
                           uint32 nMaxToDataIndex) {
  COMPILER_UINT64 tmp;

  (void) tmp;


  /* Unpack operation based on little endian */

  /* Unpack Member: m_PktClass15 */
  COMPILER_64_SET((pToStruct)->m_PktClass15, 0,  (unsigned int) (pFromData)[124]);
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_PktClass15;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[125]);
    COMPILER_64_SHL(tmp, 8);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_PktClass15;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[126]);
    COMPILER_64_SHL(tmp, 16);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_PktClass15;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[127]);
    COMPILER_64_SHL(tmp, 24);
    COMPILER_64_OR(*tmp0, tmp);
  };

  /* Unpack Member: m_ByteClass15 */
  COMPILER_64_SET((pToStruct)->m_ByteClass15, 0,  (unsigned int) (pFromData)[120]);
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_ByteClass15;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[121]);
    COMPILER_64_SHL(tmp, 8);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_ByteClass15;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[122]);
    COMPILER_64_SHL(tmp, 16);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_ByteClass15;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[123]);
    COMPILER_64_SHL(tmp, 24);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_ByteClass15;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[124]);
    COMPILER_64_SHL(tmp, 32);
    COMPILER_64_OR(*tmp0, tmp);
  };

  /* Unpack Member: m_PktClass14 */
  COMPILER_64_SET((pToStruct)->m_PktClass14, 0,  (unsigned int) (pFromData)[116]);
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_PktClass14;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[117]);
    COMPILER_64_SHL(tmp, 8);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_PktClass14;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[118]);
    COMPILER_64_SHL(tmp, 16);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_PktClass14;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[119]);
    COMPILER_64_SHL(tmp, 24);
    COMPILER_64_OR(*tmp0, tmp);
  };

  /* Unpack Member: m_ByteClass14 */
  COMPILER_64_SET((pToStruct)->m_ByteClass14, 0,  (unsigned int) (pFromData)[112]);
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_ByteClass14;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[113]);
    COMPILER_64_SHL(tmp, 8);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_ByteClass14;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[114]);
    COMPILER_64_SHL(tmp, 16);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_ByteClass14;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[115]);
    COMPILER_64_SHL(tmp, 24);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_ByteClass14;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[116]);
    COMPILER_64_SHL(tmp, 32);
    COMPILER_64_OR(*tmp0, tmp);
  };

  /* Unpack Member: m_PktClass13 */
  COMPILER_64_SET((pToStruct)->m_PktClass13, 0,  (unsigned int) (pFromData)[108]);
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_PktClass13;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[109]);
    COMPILER_64_SHL(tmp, 8);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_PktClass13;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[110]);
    COMPILER_64_SHL(tmp, 16);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_PktClass13;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[111]);
    COMPILER_64_SHL(tmp, 24);
    COMPILER_64_OR(*tmp0, tmp);
  };

  /* Unpack Member: m_ByteClass13 */
  COMPILER_64_SET((pToStruct)->m_ByteClass13, 0,  (unsigned int) (pFromData)[104]);
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_ByteClass13;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[105]);
    COMPILER_64_SHL(tmp, 8);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_ByteClass13;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[106]);
    COMPILER_64_SHL(tmp, 16);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_ByteClass13;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[107]);
    COMPILER_64_SHL(tmp, 24);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_ByteClass13;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[108]);
    COMPILER_64_SHL(tmp, 32);
    COMPILER_64_OR(*tmp0, tmp);
  };

  /* Unpack Member: m_PktClass12 */
  COMPILER_64_SET((pToStruct)->m_PktClass12, 0,  (unsigned int) (pFromData)[100]);
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_PktClass12;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[101]);
    COMPILER_64_SHL(tmp, 8);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_PktClass12;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[102]);
    COMPILER_64_SHL(tmp, 16);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_PktClass12;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[103]);
    COMPILER_64_SHL(tmp, 24);
    COMPILER_64_OR(*tmp0, tmp);
  };

  /* Unpack Member: m_ByteClass12 */
  COMPILER_64_SET((pToStruct)->m_ByteClass12, 0,  (unsigned int) (pFromData)[96]);
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_ByteClass12;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[97]);
    COMPILER_64_SHL(tmp, 8);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_ByteClass12;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[98]);
    COMPILER_64_SHL(tmp, 16);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_ByteClass12;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[99]);
    COMPILER_64_SHL(tmp, 24);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_ByteClass12;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[100]);
    COMPILER_64_SHL(tmp, 32);
    COMPILER_64_OR(*tmp0, tmp);
  };

  /* Unpack Member: m_PktClass11 */
  COMPILER_64_SET((pToStruct)->m_PktClass11, 0,  (unsigned int) (pFromData)[92]);
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_PktClass11;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[93]);
    COMPILER_64_SHL(tmp, 8);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_PktClass11;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[94]);
    COMPILER_64_SHL(tmp, 16);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_PktClass11;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[95]);
    COMPILER_64_SHL(tmp, 24);
    COMPILER_64_OR(*tmp0, tmp);
  };

  /* Unpack Member: m_ByteClass11 */
  COMPILER_64_SET((pToStruct)->m_ByteClass11, 0,  (unsigned int) (pFromData)[88]);
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_ByteClass11;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[89]);
    COMPILER_64_SHL(tmp, 8);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_ByteClass11;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[90]);
    COMPILER_64_SHL(tmp, 16);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_ByteClass11;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[91]);
    COMPILER_64_SHL(tmp, 24);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_ByteClass11;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[92]);
    COMPILER_64_SHL(tmp, 32);
    COMPILER_64_OR(*tmp0, tmp);
  };

  /* Unpack Member: m_PktClass10 */
  COMPILER_64_SET((pToStruct)->m_PktClass10, 0,  (unsigned int) (pFromData)[84]);
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_PktClass10;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[85]);
    COMPILER_64_SHL(tmp, 8);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_PktClass10;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[86]);
    COMPILER_64_SHL(tmp, 16);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_PktClass10;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[87]);
    COMPILER_64_SHL(tmp, 24);
    COMPILER_64_OR(*tmp0, tmp);
  };

  /* Unpack Member: m_ByteClass10 */
  COMPILER_64_SET((pToStruct)->m_ByteClass10, 0,  (unsigned int) (pFromData)[80]);
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_ByteClass10;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[81]);
    COMPILER_64_SHL(tmp, 8);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_ByteClass10;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[82]);
    COMPILER_64_SHL(tmp, 16);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_ByteClass10;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[83]);
    COMPILER_64_SHL(tmp, 24);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_ByteClass10;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[84]);
    COMPILER_64_SHL(tmp, 32);
    COMPILER_64_OR(*tmp0, tmp);
  };

  /* Unpack Member: m_PktClass9 */
  COMPILER_64_SET((pToStruct)->m_PktClass9, 0,  (unsigned int) (pFromData)[76]);
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_PktClass9;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[77]);
    COMPILER_64_SHL(tmp, 8);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_PktClass9;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[78]);
    COMPILER_64_SHL(tmp, 16);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_PktClass9;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[79]);
    COMPILER_64_SHL(tmp, 24);
    COMPILER_64_OR(*tmp0, tmp);
  };

  /* Unpack Member: m_ByteClass9 */
  COMPILER_64_SET((pToStruct)->m_ByteClass9, 0,  (unsigned int) (pFromData)[72]);
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_ByteClass9;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[73]);
    COMPILER_64_SHL(tmp, 8);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_ByteClass9;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[74]);
    COMPILER_64_SHL(tmp, 16);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_ByteClass9;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[75]);
    COMPILER_64_SHL(tmp, 24);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_ByteClass9;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[76]);
    COMPILER_64_SHL(tmp, 32);
    COMPILER_64_OR(*tmp0, tmp);
  };

  /* Unpack Member: m_PktClass8 */
  COMPILER_64_SET((pToStruct)->m_PktClass8, 0,  (unsigned int) (pFromData)[68]);
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_PktClass8;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[69]);
    COMPILER_64_SHL(tmp, 8);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_PktClass8;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[70]);
    COMPILER_64_SHL(tmp, 16);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_PktClass8;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[71]);
    COMPILER_64_SHL(tmp, 24);
    COMPILER_64_OR(*tmp0, tmp);
  };

  /* Unpack Member: m_ByteClass8 */
  COMPILER_64_SET((pToStruct)->m_ByteClass8, 0,  (unsigned int) (pFromData)[64]);
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_ByteClass8;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[65]);
    COMPILER_64_SHL(tmp, 8);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_ByteClass8;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[66]);
    COMPILER_64_SHL(tmp, 16);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_ByteClass8;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[67]);
    COMPILER_64_SHL(tmp, 24);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_ByteClass8;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[68]);
    COMPILER_64_SHL(tmp, 32);
    COMPILER_64_OR(*tmp0, tmp);
  };

  /* Unpack Member: m_PktClass7 */
  COMPILER_64_SET((pToStruct)->m_PktClass7, 0,  (unsigned int) (pFromData)[60]);
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_PktClass7;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[61]);
    COMPILER_64_SHL(tmp, 8);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_PktClass7;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[62]);
    COMPILER_64_SHL(tmp, 16);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_PktClass7;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[63]);
    COMPILER_64_SHL(tmp, 24);
    COMPILER_64_OR(*tmp0, tmp);
  };

  /* Unpack Member: m_ByteClass7 */
  COMPILER_64_SET((pToStruct)->m_ByteClass7, 0,  (unsigned int) (pFromData)[56]);
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_ByteClass7;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[57]);
    COMPILER_64_SHL(tmp, 8);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_ByteClass7;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[58]);
    COMPILER_64_SHL(tmp, 16);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_ByteClass7;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[59]);
    COMPILER_64_SHL(tmp, 24);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_ByteClass7;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[60]);
    COMPILER_64_SHL(tmp, 32);
    COMPILER_64_OR(*tmp0, tmp);
  };

  /* Unpack Member: m_PktClass6 */
  COMPILER_64_SET((pToStruct)->m_PktClass6, 0,  (unsigned int) (pFromData)[52]);
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_PktClass6;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[53]);
    COMPILER_64_SHL(tmp, 8);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_PktClass6;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[54]);
    COMPILER_64_SHL(tmp, 16);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_PktClass6;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[55]);
    COMPILER_64_SHL(tmp, 24);
    COMPILER_64_OR(*tmp0, tmp);
  };

  /* Unpack Member: m_ByteClass6 */
  COMPILER_64_SET((pToStruct)->m_ByteClass6, 0,  (unsigned int) (pFromData)[48]);
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_ByteClass6;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[49]);
    COMPILER_64_SHL(tmp, 8);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_ByteClass6;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[50]);
    COMPILER_64_SHL(tmp, 16);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_ByteClass6;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[51]);
    COMPILER_64_SHL(tmp, 24);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_ByteClass6;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[52]);
    COMPILER_64_SHL(tmp, 32);
    COMPILER_64_OR(*tmp0, tmp);
  };

  /* Unpack Member: m_PktClass5 */
  COMPILER_64_SET((pToStruct)->m_PktClass5, 0,  (unsigned int) (pFromData)[44]);
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_PktClass5;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[45]);
    COMPILER_64_SHL(tmp, 8);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_PktClass5;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[46]);
    COMPILER_64_SHL(tmp, 16);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_PktClass5;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[47]);
    COMPILER_64_SHL(tmp, 24);
    COMPILER_64_OR(*tmp0, tmp);
  };

  /* Unpack Member: m_ByteClass5 */
  COMPILER_64_SET((pToStruct)->m_ByteClass5, 0,  (unsigned int) (pFromData)[40]);
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_ByteClass5;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[41]);
    COMPILER_64_SHL(tmp, 8);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_ByteClass5;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[42]);
    COMPILER_64_SHL(tmp, 16);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_ByteClass5;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[43]);
    COMPILER_64_SHL(tmp, 24);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_ByteClass5;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[44]);
    COMPILER_64_SHL(tmp, 32);
    COMPILER_64_OR(*tmp0, tmp);
  };

  /* Unpack Member: m_PktClass4 */
  COMPILER_64_SET((pToStruct)->m_PktClass4, 0,  (unsigned int) (pFromData)[36]);
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_PktClass4;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[37]);
    COMPILER_64_SHL(tmp, 8);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_PktClass4;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[38]);
    COMPILER_64_SHL(tmp, 16);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_PktClass4;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[39]);
    COMPILER_64_SHL(tmp, 24);
    COMPILER_64_OR(*tmp0, tmp);
  };

  /* Unpack Member: m_ByteClass4 */
  COMPILER_64_SET((pToStruct)->m_ByteClass4, 0,  (unsigned int) (pFromData)[32]);
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_ByteClass4;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[33]);
    COMPILER_64_SHL(tmp, 8);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_ByteClass4;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[34]);
    COMPILER_64_SHL(tmp, 16);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_ByteClass4;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[35]);
    COMPILER_64_SHL(tmp, 24);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_ByteClass4;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[36]);
    COMPILER_64_SHL(tmp, 32);
    COMPILER_64_OR(*tmp0, tmp);
  };

  /* Unpack Member: m_PktClass3 */
  COMPILER_64_SET((pToStruct)->m_PktClass3, 0,  (unsigned int) (pFromData)[28]);
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_PktClass3;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[29]);
    COMPILER_64_SHL(tmp, 8);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_PktClass3;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[30]);
    COMPILER_64_SHL(tmp, 16);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_PktClass3;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[31]);
    COMPILER_64_SHL(tmp, 24);
    COMPILER_64_OR(*tmp0, tmp);
  };

  /* Unpack Member: m_ByteClass3 */
  COMPILER_64_SET((pToStruct)->m_ByteClass3, 0,  (unsigned int) (pFromData)[24]);
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_ByteClass3;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[25]);
    COMPILER_64_SHL(tmp, 8);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_ByteClass3;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[26]);
    COMPILER_64_SHL(tmp, 16);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_ByteClass3;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[27]);
    COMPILER_64_SHL(tmp, 24);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_ByteClass3;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[28]);
    COMPILER_64_SHL(tmp, 32);
    COMPILER_64_OR(*tmp0, tmp);
  };

  /* Unpack Member: m_PktClass2 */
  COMPILER_64_SET((pToStruct)->m_PktClass2, 0,  (unsigned int) (pFromData)[20]);
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_PktClass2;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[21]);
    COMPILER_64_SHL(tmp, 8);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_PktClass2;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[22]);
    COMPILER_64_SHL(tmp, 16);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_PktClass2;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[23]);
    COMPILER_64_SHL(tmp, 24);
    COMPILER_64_OR(*tmp0, tmp);
  };

  /* Unpack Member: m_ByteClass2 */
  COMPILER_64_SET((pToStruct)->m_ByteClass2, 0,  (unsigned int) (pFromData)[16]);
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_ByteClass2;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[17]);
    COMPILER_64_SHL(tmp, 8);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_ByteClass2;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[18]);
    COMPILER_64_SHL(tmp, 16);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_ByteClass2;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[19]);
    COMPILER_64_SHL(tmp, 24);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_ByteClass2;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[20]);
    COMPILER_64_SHL(tmp, 32);
    COMPILER_64_OR(*tmp0, tmp);
  };

  /* Unpack Member: m_PktClass1 */
  COMPILER_64_SET((pToStruct)->m_PktClass1, 0,  (unsigned int) (pFromData)[12]);
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_PktClass1;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[13]);
    COMPILER_64_SHL(tmp, 8);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_PktClass1;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[14]);
    COMPILER_64_SHL(tmp, 16);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_PktClass1;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[15]);
    COMPILER_64_SHL(tmp, 24);
    COMPILER_64_OR(*tmp0, tmp);
  };

  /* Unpack Member: m_ByteClass1 */
  COMPILER_64_SET((pToStruct)->m_ByteClass1, 0,  (unsigned int) (pFromData)[8]);
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_ByteClass1;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[9]);
    COMPILER_64_SHL(tmp, 8);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_ByteClass1;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[10]);
    COMPILER_64_SHL(tmp, 16);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_ByteClass1;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[11]);
    COMPILER_64_SHL(tmp, 24);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_ByteClass1;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[12]);
    COMPILER_64_SHL(tmp, 32);
    COMPILER_64_OR(*tmp0, tmp);
  };

  /* Unpack Member: m_PktClass0 */
  COMPILER_64_SET((pToStruct)->m_PktClass0, 0,  (unsigned int) (pFromData)[4]);
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_PktClass0;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[5]);
    COMPILER_64_SHL(tmp, 8);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_PktClass0;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[6]);
    COMPILER_64_SHL(tmp, 16);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_PktClass0;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[7]);
    COMPILER_64_SHL(tmp, 24);
    COMPILER_64_OR(*tmp0, tmp);
  };

  /* Unpack Member: m_ByteClass0 */
  COMPILER_64_SET((pToStruct)->m_ByteClass0, 0,  (unsigned int) (pFromData)[0]);
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_ByteClass0;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[1]);
    COMPILER_64_SHL(tmp, 8);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_ByteClass0;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[2]);
    COMPILER_64_SHL(tmp, 16);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_ByteClass0;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[3]);
    COMPILER_64_SHL(tmp, 24);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_ByteClass0;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[4]);
    COMPILER_64_SHL(tmp, 32);
    COMPILER_64_OR(*tmp0, tmp);
  };

}



/* initialize an instance of this zframe */
void
sbZfSbQe2000ElibPCT_InitInstance(sbZfSbQe2000ElibPCT_t *pFrame) {

  COMPILER_64_ZERO(pFrame->m_PktClass15);
  COMPILER_64_ZERO(pFrame->m_ByteClass15);
  COMPILER_64_ZERO(pFrame->m_PktClass14);
  COMPILER_64_ZERO(pFrame->m_ByteClass14);
  COMPILER_64_ZERO(pFrame->m_PktClass13);
  COMPILER_64_ZERO(pFrame->m_ByteClass13);
  COMPILER_64_ZERO(pFrame->m_PktClass12);
  COMPILER_64_ZERO(pFrame->m_ByteClass12);
  COMPILER_64_ZERO(pFrame->m_PktClass11);
  COMPILER_64_ZERO(pFrame->m_ByteClass11);
  COMPILER_64_ZERO(pFrame->m_PktClass10);
  COMPILER_64_ZERO(pFrame->m_ByteClass10);
  COMPILER_64_ZERO(pFrame->m_PktClass9);
  COMPILER_64_ZERO(pFrame->m_ByteClass9);
  COMPILER_64_ZERO(pFrame->m_PktClass8);
  COMPILER_64_ZERO(pFrame->m_ByteClass8);
  COMPILER_64_ZERO(pFrame->m_PktClass7);
  COMPILER_64_ZERO(pFrame->m_ByteClass7);
  COMPILER_64_ZERO(pFrame->m_PktClass6);
  COMPILER_64_ZERO(pFrame->m_ByteClass6);
  COMPILER_64_ZERO(pFrame->m_PktClass5);
  COMPILER_64_ZERO(pFrame->m_ByteClass5);
  COMPILER_64_ZERO(pFrame->m_PktClass4);
  COMPILER_64_ZERO(pFrame->m_ByteClass4);
  COMPILER_64_ZERO(pFrame->m_PktClass3);
  COMPILER_64_ZERO(pFrame->m_ByteClass3);
  COMPILER_64_ZERO(pFrame->m_PktClass2);
  COMPILER_64_ZERO(pFrame->m_ByteClass2);
  COMPILER_64_ZERO(pFrame->m_PktClass1);
  COMPILER_64_ZERO(pFrame->m_ByteClass1);
  COMPILER_64_ZERO(pFrame->m_PktClass0);
  COMPILER_64_ZERO(pFrame->m_ByteClass0);

}
