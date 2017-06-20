/*
 * $Id: sbZfSbQe2000ElibVIT.c,v 1.4 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


#include "sbTypesGlue.h"
#include "sbZfSbQe2000ElibVIT.hx"
#include "sbWrappers.h"
#include <sal/types.h>



/* Pack from struct into array of bytes */
uint32
sbZfSbQe2000ElibVIT_Pack(sbZfSbQe2000ElibVIT_t *pFrom,
                         uint8 *pToData,
                         uint32 nMaxToDataIndex) {
  int i;
  int size = SB_ZF_SB_QE2000_ELIB_VIT_ENTRY_SIZE_IN_BYTES;

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on little endian */

  /* Pack Member: m_record3 */
  (pToData)[6] |= ((pFrom)->m_record3) & 0xFF;
  (pToData)[7] |= ((pFrom)->m_record3 >> 8) &0xFF;

  /* Pack Member: m_record2 */
  (pToData)[4] |= ((pFrom)->m_record2) & 0xFF;
  (pToData)[5] |= ((pFrom)->m_record2 >> 8) &0xFF;

  /* Pack Member: m_record1 */
  (pToData)[2] |= ((pFrom)->m_record1) & 0xFF;
  (pToData)[3] |= ((pFrom)->m_record1 >> 8) &0xFF;

  /* Pack Member: m_record0 */
  (pToData)[0] |= ((pFrom)->m_record0) & 0xFF;
  (pToData)[1] |= ((pFrom)->m_record0 >> 8) &0xFF;

  return SB_ZF_SB_QE2000_ELIB_VIT_ENTRY_SIZE_IN_BYTES;
}




/* Unpack from array of bytes into struct */
void
sbZfSbQe2000ElibVIT_Unpack(sbZfSbQe2000ElibVIT_t *pToStruct,
                           uint8 *pFromData,
                           uint32 nMaxToDataIndex) {
  COMPILER_UINT64 tmp;

  (void) tmp;


  /* Unpack operation based on little endian */

  /* Unpack Member: m_record3 */
  (pToStruct)->m_record3 =  (int32)  (pFromData)[6] ;
  (pToStruct)->m_record3 |=  (int32)  (pFromData)[7] << 8;

  /* Unpack Member: m_record2 */
  (pToStruct)->m_record2 =  (int32)  (pFromData)[4] ;
  (pToStruct)->m_record2 |=  (int32)  (pFromData)[5] << 8;

  /* Unpack Member: m_record1 */
  (pToStruct)->m_record1 =  (int32)  (pFromData)[2] ;
  (pToStruct)->m_record1 |=  (int32)  (pFromData)[3] << 8;

  /* Unpack Member: m_record0 */
  (pToStruct)->m_record0 =  (int32)  (pFromData)[0] ;
  (pToStruct)->m_record0 |=  (int32)  (pFromData)[1] << 8;

}



/* initialize an instance of this zframe */
void
sbZfSbQe2000ElibVIT_InitInstance(sbZfSbQe2000ElibVIT_t *pFrame) {

  pFrame->m_record3 =  (unsigned int)  0;
  pFrame->m_record2 =  (unsigned int)  0;
  pFrame->m_record1 =  (unsigned int)  0;
  pFrame->m_record0 =  (unsigned int)  0;

}
