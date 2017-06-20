/*
 * $Id: sbZfKaEpInstruction.c,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


#include "sbTypes.h"
#include "sbZfKaEpInstruction.hx"
#include "sbWrappers.h"
#include <sal/types.h>



/* Pack from struct into array of bytes */
uint32
sbZfKaEpInstruction_Pack(sbZfKaEpInstruction_t *pFrom,
                         uint8 *pToData,
                         uint32 nMaxToDataIndex) {
#ifdef SAND_BIG_ENDIAN_HOST
  int i;
  int size = SB_ZF_ZFKAEPINSTRUCTION_SIZE_IN_BYTES;

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on bigword endian */

  /* Pack Member: m_nValid */
  (pToData)[0] |= ((pFrom)->m_nValid & 0x01) <<7;

  /* Pack Member: m_nOpcode */
  (pToData)[0] |= ((pFrom)->m_nOpcode & 0x3f) <<1;

  /* Pack Member: m_nOpAVariable */
  (pToData)[0] |= ((pFrom)->m_nOpAVariable & 0x01);

  /* Pack Member: m_nOperandA */
  (pToData)[1] |= ((pFrom)->m_nOperandA & 0x3f) <<2;

  /* Pack Member: m_nOpBVariable */
  (pToData)[1] |= ((pFrom)->m_nOpBVariable & 0x01) <<1;

  /* Pack Member: m_nOperandB */
  (pToData)[2] |= ((pFrom)->m_nOperandB & 0x1f) <<3;
  (pToData)[1] |= ((pFrom)->m_nOperandB >> 5) & 0x01;

  /* Pack Member: m_nOpCVariable */
  (pToData)[2] |= ((pFrom)->m_nOpCVariable & 0x01) <<2;

  /* Pack Member: m_nOperandC */
  (pToData)[3] |= ((pFrom)->m_nOperandC) & 0xFF;
  (pToData)[2] |= ((pFrom)->m_nOperandC >> 8) & 0x03;
#else
  int i;
  int size = SB_ZF_ZFKAEPINSTRUCTION_SIZE_IN_BYTES;

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on little endian */

  /* Pack Member: m_nValid */
  (pToData)[3] |= ((pFrom)->m_nValid & 0x01) <<7;

  /* Pack Member: m_nOpcode */
  (pToData)[3] |= ((pFrom)->m_nOpcode & 0x3f) <<1;

  /* Pack Member: m_nOpAVariable */
  (pToData)[3] |= ((pFrom)->m_nOpAVariable & 0x01);

  /* Pack Member: m_nOperandA */
  (pToData)[2] |= ((pFrom)->m_nOperandA & 0x3f) <<2;

  /* Pack Member: m_nOpBVariable */
  (pToData)[2] |= ((pFrom)->m_nOpBVariable & 0x01) <<1;

  /* Pack Member: m_nOperandB */
  (pToData)[1] |= ((pFrom)->m_nOperandB & 0x1f) <<3;
  (pToData)[2] |= ((pFrom)->m_nOperandB >> 5) & 0x01;

  /* Pack Member: m_nOpCVariable */
  (pToData)[1] |= ((pFrom)->m_nOpCVariable & 0x01) <<2;

  /* Pack Member: m_nOperandC */
  (pToData)[0] |= ((pFrom)->m_nOperandC) & 0xFF;
  (pToData)[1] |= ((pFrom)->m_nOperandC >> 8) & 0x03;
#endif

  return SB_ZF_ZFKAEPINSTRUCTION_SIZE_IN_BYTES;
}




/* Unpack from array of bytes into struct */
void
sbZfKaEpInstruction_Unpack(sbZfKaEpInstruction_t *pToStruct,
                           uint8 *pFromData,
                           uint32 nMaxToDataIndex) {
  COMPILER_UINT64 tmp;

  (void) tmp;

#ifdef SAND_BIG_ENDIAN_HOST

  /* Unpack operation based on bigword endian */

  /* Unpack Member: m_nValid */
  (pToStruct)->m_nValid =  (int32)  ((pFromData)[0] >> 7) & 0x01;

  /* Unpack Member: m_nOpcode */
  (pToStruct)->m_nOpcode =  (int32)  ((pFromData)[0] >> 1) & 0x3f;

  /* Unpack Member: m_nOpAVariable */
  (pToStruct)->m_nOpAVariable =  (int32)  ((pFromData)[0] ) & 0x01;

  /* Unpack Member: m_nOperandA */
  (pToStruct)->m_nOperandA =  (int32)  ((pFromData)[1] >> 2) & 0x3f;

  /* Unpack Member: m_nOpBVariable */
  (pToStruct)->m_nOpBVariable =  (int32)  ((pFromData)[1] >> 1) & 0x01;

  /* Unpack Member: m_nOperandB */
  (pToStruct)->m_nOperandB =  (int32)  ((pFromData)[2] >> 3) & 0x1f;
  (pToStruct)->m_nOperandB |=  (int32)  ((pFromData)[1] & 0x01) << 5;

  /* Unpack Member: m_nOpCVariable */
  (pToStruct)->m_nOpCVariable =  (int32)  ((pFromData)[2] >> 2) & 0x01;

  /* Unpack Member: m_nOperandC */
  (pToStruct)->m_nOperandC =  (int32)  (pFromData)[3] ;
  (pToStruct)->m_nOperandC |=  (int32)  ((pFromData)[2] & 0x03) << 8;
#else

  /* Unpack operation based on little endian */

  /* Unpack Member: m_nValid */
  (pToStruct)->m_nValid =  (int32)  ((pFromData)[3] >> 7) & 0x01;

  /* Unpack Member: m_nOpcode */
  (pToStruct)->m_nOpcode =  (int32)  ((pFromData)[3] >> 1) & 0x3f;

  /* Unpack Member: m_nOpAVariable */
  (pToStruct)->m_nOpAVariable =  (int32)  ((pFromData)[3] ) & 0x01;

  /* Unpack Member: m_nOperandA */
  (pToStruct)->m_nOperandA =  (int32)  ((pFromData)[2] >> 2) & 0x3f;

  /* Unpack Member: m_nOpBVariable */
  (pToStruct)->m_nOpBVariable =  (int32)  ((pFromData)[2] >> 1) & 0x01;

  /* Unpack Member: m_nOperandB */
  (pToStruct)->m_nOperandB =  (int32)  ((pFromData)[1] >> 3) & 0x1f;
  (pToStruct)->m_nOperandB |=  (int32)  ((pFromData)[2] & 0x01) << 5;

  /* Unpack Member: m_nOpCVariable */
  (pToStruct)->m_nOpCVariable =  (int32)  ((pFromData)[1] >> 2) & 0x01;

  /* Unpack Member: m_nOperandC */
  (pToStruct)->m_nOperandC =  (int32)  (pFromData)[0] ;
  (pToStruct)->m_nOperandC |=  (int32)  ((pFromData)[1] & 0x03) << 8;
#endif

}



/* initialize an instance of this zframe */
void
sbZfKaEpInstruction_InitInstance(sbZfKaEpInstruction_t *pFrame) {

  pFrame->m_nValid =  (unsigned int)  0;
  pFrame->m_nOpcode =  (unsigned int)  0;
  pFrame->m_nOpAVariable =  (unsigned int)  0;
  pFrame->m_nOperandA =  (unsigned int)  0;
  pFrame->m_nOpBVariable =  (unsigned int)  0;
  pFrame->m_nOperandB =  (unsigned int)  0;
  pFrame->m_nOpCVariable =  (unsigned int)  0;
  pFrame->m_nOperandC =  (unsigned int)  0;

}
