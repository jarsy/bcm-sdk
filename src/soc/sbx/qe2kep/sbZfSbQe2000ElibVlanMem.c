/*
 * $Id: sbZfSbQe2000ElibVlanMem.c,v 1.4 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


#include "sbTypesGlue.h"
#include "sbZfSbQe2000ElibVlanMem.hx"
#include "sbWrappers.h"
#include <sal/types.h>



/* Pack from struct into array of bytes */
uint32
sbZfSbQe2000ElibVlanMem_Pack(sbZfSbQe2000ElibVlanMem_t *pFrom,
                             uint8 *pToData,
                             uint32 nMaxToDataIndex) {
  int i;
  int size = SB_ZF_SB_QE2000_ELIB_VLAN_MEM_SIZE_IN_BYTES;

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on little endian */

  /* Pack Member: m_Patches */
  (pToData)[0] |= ((pFrom)->m_Patches & 0x01);

  /* Pack Member: m_Edges */
  (pToData)[0] |= ((pFrom)->m_Edges & 0x01);

  /* Pack Member: m_NumFree */
  (pToData)[0] |= ((pFrom)->m_NumFree & 0x01);

  /* Pack Member: m_NumUsed */
  (pToData)[0] |= ((pFrom)->m_NumUsed & 0x01);

  return SB_ZF_SB_QE2000_ELIB_VLAN_MEM_SIZE_IN_BYTES;
}




/* Unpack from array of bytes into struct */
void
sbZfSbQe2000ElibVlanMem_Unpack(sbZfSbQe2000ElibVlanMem_t *pToStruct,
                               uint8 *pFromData,
                               uint32 nMaxToDataIndex) {
  COMPILER_UINT64 tmp;

  (void) tmp;


  /* Unpack operation based on little endian */

  /* Unpack Member: m_Patches */
  (pToStruct)->m_Patches =  (int32)  ((pFromData)[0] ) & 0x01;

  /* Unpack Member: m_Edges */
  (pToStruct)->m_Edges =  (int32)  ((pFromData)[0] ) & 0x01;

  /* Unpack Member: m_NumFree */
  (pToStruct)->m_NumFree =  (int32)  ((pFromData)[0] ) & 0x01;

  /* Unpack Member: m_NumUsed */
  (pToStruct)->m_NumUsed =  (int32)  ((pFromData)[0] ) & 0x01;

}



/* initialize an instance of this zframe */
void
sbZfSbQe2000ElibVlanMem_InitInstance(sbZfSbQe2000ElibVlanMem_t *pFrame) {

  pFrame->m_Patches =  (unsigned int)  0;
  pFrame->m_Edges =  (unsigned int)  0;
  pFrame->m_NumFree =  (unsigned int)  0;
  pFrame->m_NumUsed =  (unsigned int)  0;

}
