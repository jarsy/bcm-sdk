/*
 * $Id: sbZfSbQe2000ElibCIT.c,v 1.4 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


#include "sbTypesGlue.h"
#include "sbZfSbQe2000ElibCIT.hx"
#include "sbWrappers.h"
#include <sal/types.h>



/* Pack from struct into array of bytes */
uint32
sbZfSbQe2000ElibCIT_Pack(sbZfSbQe2000ElibCIT_t *pFrom,
                         uint8 *pToData,
                         uint32 nMaxToDataIndex) {
  int i;
  int size = SB_ZF_SB_QE2000_ELIB_CIT_ENTRY_SIZE_IN_BYTES;

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on little endian */

  /* Pack Member: m_Instruction7 */
  (pToData)[28] |= ((pFrom)->m_Instruction7) & 0xFF;
  (pToData)[29] |= ((pFrom)->m_Instruction7 >> 8) &0xFF;
  (pToData)[30] |= ((pFrom)->m_Instruction7 >> 16) &0xFF;
  (pToData)[31] |= ((pFrom)->m_Instruction7 >> 24) &0xFF;

  /* Pack Member: m_Instruction6 */
  (pToData)[24] |= ((pFrom)->m_Instruction6) & 0xFF;
  (pToData)[25] |= ((pFrom)->m_Instruction6 >> 8) &0xFF;
  (pToData)[26] |= ((pFrom)->m_Instruction6 >> 16) &0xFF;
  (pToData)[27] |= ((pFrom)->m_Instruction6 >> 24) &0xFF;

  /* Pack Member: m_Instruction5 */
  (pToData)[20] |= ((pFrom)->m_Instruction5) & 0xFF;
  (pToData)[21] |= ((pFrom)->m_Instruction5 >> 8) &0xFF;
  (pToData)[22] |= ((pFrom)->m_Instruction5 >> 16) &0xFF;
  (pToData)[23] |= ((pFrom)->m_Instruction5 >> 24) &0xFF;

  /* Pack Member: m_Instruction4 */
  (pToData)[16] |= ((pFrom)->m_Instruction4) & 0xFF;
  (pToData)[17] |= ((pFrom)->m_Instruction4 >> 8) &0xFF;
  (pToData)[18] |= ((pFrom)->m_Instruction4 >> 16) &0xFF;
  (pToData)[19] |= ((pFrom)->m_Instruction4 >> 24) &0xFF;

  /* Pack Member: m_Instruction3 */
  (pToData)[12] |= ((pFrom)->m_Instruction3) & 0xFF;
  (pToData)[13] |= ((pFrom)->m_Instruction3 >> 8) &0xFF;
  (pToData)[14] |= ((pFrom)->m_Instruction3 >> 16) &0xFF;
  (pToData)[15] |= ((pFrom)->m_Instruction3 >> 24) &0xFF;

  /* Pack Member: m_Instruction2 */
  (pToData)[8] |= ((pFrom)->m_Instruction2) & 0xFF;
  (pToData)[9] |= ((pFrom)->m_Instruction2 >> 8) &0xFF;
  (pToData)[10] |= ((pFrom)->m_Instruction2 >> 16) &0xFF;
  (pToData)[11] |= ((pFrom)->m_Instruction2 >> 24) &0xFF;

  /* Pack Member: m_Instruction1 */
  (pToData)[4] |= ((pFrom)->m_Instruction1) & 0xFF;
  (pToData)[5] |= ((pFrom)->m_Instruction1 >> 8) &0xFF;
  (pToData)[6] |= ((pFrom)->m_Instruction1 >> 16) &0xFF;
  (pToData)[7] |= ((pFrom)->m_Instruction1 >> 24) &0xFF;

  /* Pack Member: m_Instruction0 */
  (pToData)[0] |= ((pFrom)->m_Instruction0) & 0xFF;
  (pToData)[1] |= ((pFrom)->m_Instruction0 >> 8) &0xFF;
  (pToData)[2] |= ((pFrom)->m_Instruction0 >> 16) &0xFF;
  (pToData)[3] |= ((pFrom)->m_Instruction0 >> 24) &0xFF;

  return SB_ZF_SB_QE2000_ELIB_CIT_ENTRY_SIZE_IN_BYTES;
}




/* Unpack from array of bytes into struct */
void
sbZfSbQe2000ElibCIT_Unpack(sbZfSbQe2000ElibCIT_t *pToStruct,
                           uint8 *pFromData,
                           uint32 nMaxToDataIndex) {
  COMPILER_UINT64 tmp;

  (void) tmp;


  /* Unpack operation based on little endian */

  /* Unpack Member: m_Instruction7 */
  (pToStruct)->m_Instruction7 =  (int32)  (pFromData)[28] ;
  (pToStruct)->m_Instruction7 |=  (int32)  (pFromData)[29] << 8;
  (pToStruct)->m_Instruction7 |=  (int32)  (pFromData)[30] << 16;
  (pToStruct)->m_Instruction7 |=  (int32)  (pFromData)[31] << 24;

  /* Unpack Member: m_Instruction6 */
  (pToStruct)->m_Instruction6 =  (int32)  (pFromData)[24] ;
  (pToStruct)->m_Instruction6 |=  (int32)  (pFromData)[25] << 8;
  (pToStruct)->m_Instruction6 |=  (int32)  (pFromData)[26] << 16;
  (pToStruct)->m_Instruction6 |=  (int32)  (pFromData)[27] << 24;

  /* Unpack Member: m_Instruction5 */
  (pToStruct)->m_Instruction5 =  (int32)  (pFromData)[20] ;
  (pToStruct)->m_Instruction5 |=  (int32)  (pFromData)[21] << 8;
  (pToStruct)->m_Instruction5 |=  (int32)  (pFromData)[22] << 16;
  (pToStruct)->m_Instruction5 |=  (int32)  (pFromData)[23] << 24;

  /* Unpack Member: m_Instruction4 */
  (pToStruct)->m_Instruction4 =  (int32)  (pFromData)[16] ;
  (pToStruct)->m_Instruction4 |=  (int32)  (pFromData)[17] << 8;
  (pToStruct)->m_Instruction4 |=  (int32)  (pFromData)[18] << 16;
  (pToStruct)->m_Instruction4 |=  (int32)  (pFromData)[19] << 24;

  /* Unpack Member: m_Instruction3 */
  (pToStruct)->m_Instruction3 =  (int32)  (pFromData)[12] ;
  (pToStruct)->m_Instruction3 |=  (int32)  (pFromData)[13] << 8;
  (pToStruct)->m_Instruction3 |=  (int32)  (pFromData)[14] << 16;
  (pToStruct)->m_Instruction3 |=  (int32)  (pFromData)[15] << 24;

  /* Unpack Member: m_Instruction2 */
  (pToStruct)->m_Instruction2 =  (int32)  (pFromData)[8] ;
  (pToStruct)->m_Instruction2 |=  (int32)  (pFromData)[9] << 8;
  (pToStruct)->m_Instruction2 |=  (int32)  (pFromData)[10] << 16;
  (pToStruct)->m_Instruction2 |=  (int32)  (pFromData)[11] << 24;

  /* Unpack Member: m_Instruction1 */
  (pToStruct)->m_Instruction1 =  (int32)  (pFromData)[4] ;
  (pToStruct)->m_Instruction1 |=  (int32)  (pFromData)[5] << 8;
  (pToStruct)->m_Instruction1 |=  (int32)  (pFromData)[6] << 16;
  (pToStruct)->m_Instruction1 |=  (int32)  (pFromData)[7] << 24;

  /* Unpack Member: m_Instruction0 */
  (pToStruct)->m_Instruction0 =  (int32)  (pFromData)[0] ;
  (pToStruct)->m_Instruction0 |=  (int32)  (pFromData)[1] << 8;
  (pToStruct)->m_Instruction0 |=  (int32)  (pFromData)[2] << 16;
  (pToStruct)->m_Instruction0 |=  (int32)  (pFromData)[3] << 24;

}



/* initialize an instance of this zframe */
void
sbZfSbQe2000ElibCIT_InitInstance(sbZfSbQe2000ElibCIT_t *pFrame) {

  pFrame->m_Instruction7 =  (unsigned int)  0;
  pFrame->m_Instruction6 =  (unsigned int)  0;
  pFrame->m_Instruction5 =  (unsigned int)  0;
  pFrame->m_Instruction4 =  (unsigned int)  0;
  pFrame->m_Instruction3 =  (unsigned int)  0;
  pFrame->m_Instruction2 =  (unsigned int)  0;
  pFrame->m_Instruction1 =  (unsigned int)  0;
  pFrame->m_Instruction0 =  (unsigned int)  0;

}
