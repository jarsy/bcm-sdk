/*
 * $Id: sbZfSbQe2000ElibPriTableAddr.c,v 1.4 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


#include "sbTypesGlue.h"
#include "sbZfSbQe2000ElibPriTableAddr.hx"
#include "sbWrappers.h"
#include <sal/types.h>



/* Pack from struct into array of bytes */
uint32
sbZfSbQe2000ElibPriTableAddr_Pack(sbZfSbQe2000ElibPriTableAddr_t *pFrom,
                                  uint8 *pToData,
                                  uint32 nMaxToDataIndex) {
  int i;
  int size = SB_ZF_SB_QE2000_ELIB_PRI_TABLE_ADDR_SIZE_IN_BYTES;

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on big endian */

  /* Pack Member: m_nPort */
  (pToData)[3] |= ((pFrom)->m_nPort & 0x03) <<6;
  (pToData)[2] |= ((pFrom)->m_nPort >> 2) & 0x0f;

  /* Pack Member: m_nCos */
  (pToData)[3] |= ((pFrom)->m_nCos & 0x07) <<3;

  /* Pack Member: m_nDp */
  (pToData)[3] |= ((pFrom)->m_nDp & 0x03) <<1;

  /* Pack Member: m_nEcn */
  (pToData)[3] |= ((pFrom)->m_nEcn & 0x01);

  return SB_ZF_SB_QE2000_ELIB_PRI_TABLE_ADDR_SIZE_IN_BYTES;
}




/* Unpack from array of bytes into struct */
void
sbZfSbQe2000ElibPriTableAddr_Unpack(sbZfSbQe2000ElibPriTableAddr_t *pToStruct,
                                    uint8 *pFromData,
                                    uint32 nMaxToDataIndex) {
  COMPILER_UINT64 tmp;

  (void) tmp;


  /* Unpack operation based on big endian */

  /* Unpack Member: m_nPort */
  (pToStruct)->m_nPort =  (uint32)  ((pFromData)[3] >> 6) & 0x03;
  (pToStruct)->m_nPort |=  (uint32)  ((pFromData)[2] & 0x0f) << 2;

  /* Unpack Member: m_nCos */
  (pToStruct)->m_nCos =  (uint32)  ((pFromData)[3] >> 3) & 0x07;

  /* Unpack Member: m_nDp */
  (pToStruct)->m_nDp =  (uint32)  ((pFromData)[3] >> 1) & 0x03;

  /* Unpack Member: m_nEcn */
  (pToStruct)->m_nEcn =  (uint32)  ((pFromData)[3] ) & 0x01;

}



/* initialize an instance of this zframe */
void
sbZfSbQe2000ElibPriTableAddr_InitInstance(sbZfSbQe2000ElibPriTableAddr_t *pFrame) {

  pFrame->m_nPort =  (unsigned int)  0;
  pFrame->m_nCos =  (unsigned int)  0;
  pFrame->m_nDp =  (unsigned int)  0;
  pFrame->m_nEcn =  (unsigned int)  0;

}
