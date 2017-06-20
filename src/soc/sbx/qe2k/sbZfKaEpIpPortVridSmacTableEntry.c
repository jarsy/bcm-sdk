/*
 * $Id: sbZfKaEpIpPortVridSmacTableEntry.c,v 1.3 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


#include "sbTypes.h"
#include "sbZfKaEpIpPortVridSmacTableEntry.hx"
#include "sbWrappers.h"
#include <sal/types.h>



/* Pack from struct into array of bytes */
uint32
sbZfKaEpIpPortVridSmacTableEntry_Pack(sbZfKaEpIpPortVridSmacTableEntry_t *pFrom,
                                      uint8 *pToData,
                                      uint32 nMaxToDataIndex) {
#ifdef SAND_BIG_ENDIAN_HOST
  int i;
  int size = SB_ZF_ZFKAEPIP_PORT_VRID_SMAC_TABLEENTRY_SIZE_IN_BYTES;

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on bigword endian */

  /* Pack Member: m_nReserved */
  (pToData)[5] |= ((pFrom)->m_nReserved) & 0xFF;
  (pToData)[4] |= ((pFrom)->m_nReserved >> 8) &0xFF;

  /* Pack Member: m_nnSmac */
  (pToData)[3] |= (COMPILER_64_LO((pFrom)->m_nnSmac)) & 0xFF;
  (pToData)[2] |= (COMPILER_64_LO((pFrom)->m_nnSmac) >> 8) &0xFF;
  (pToData)[1] |= (COMPILER_64_LO((pFrom)->m_nnSmac) >> 16) &0xFF;
  (pToData)[0] |= (COMPILER_64_LO((pFrom)->m_nnSmac) >> 24) &0xFF;
  (pToData)[7] |= (COMPILER_64_HI((pFrom)->m_nnSmac)) & 0xFF;
  (pToData)[6] |= (COMPILER_64_HI((pFrom)->m_nnSmac) >> 8) &0xFF;
#else
  int i;
  int size = SB_ZF_ZFKAEPIP_PORT_VRID_SMAC_TABLEENTRY_SIZE_IN_BYTES;

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on little endian */

  /* Pack Member: m_nReserved */
  (pToData)[6] |= ((pFrom)->m_nReserved) & 0xFF;
  (pToData)[7] |= ((pFrom)->m_nReserved >> 8) &0xFF;

  /* Pack Member: m_nnSmac */
  (pToData)[0] |= ((pFrom)->m_nnSmac) & 0xFF;
  (pToData)[1] |= ((pFrom)->m_nnSmac >> 8) &0xFF;
  (pToData)[2] |= ((pFrom)->m_nnSmac >> 16) &0xFF;
  (pToData)[3] |= ((pFrom)->m_nnSmac >> 24) &0xFF;
  (pToData)[4] |= ((pFrom)->m_nnSmac >> 32) &0xFF;
  (pToData)[5] |= ((pFrom)->m_nnSmac >> 40) &0xFF;
#endif

  return SB_ZF_ZFKAEPIP_PORT_VRID_SMAC_TABLEENTRY_SIZE_IN_BYTES;
}




/* Unpack from array of bytes into struct */
void
sbZfKaEpIpPortVridSmacTableEntry_Unpack(sbZfKaEpIpPortVridSmacTableEntry_t *pToStruct,
                                        uint8 *pFromData,
                                        uint32 nMaxToDataIndex) {
  COMPILER_UINT64 tmp;

  (void) tmp;

#ifdef SAND_BIG_ENDIAN_HOST

  /* Unpack operation based on bigword endian */

  /* Unpack Member: m_nReserved */
  (pToStruct)->m_nReserved =  (uint32)  (pFromData)[5] ;
  (pToStruct)->m_nReserved |=  (uint32)  (pFromData)[4] << 8;

  /* Unpack Member: m_nnSmac */
  COMPILER_64_SET((pToStruct)->m_nnSmac, 0,  (unsigned int) (pFromData)[3]);
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_nnSmac;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[2]);
    COMPILER_64_SHL(tmp, 8);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_nnSmac;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[1]);
    COMPILER_64_SHL(tmp, 16);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_nnSmac;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[0]);
    COMPILER_64_SHL(tmp, 24);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_nnSmac;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[7]);
    COMPILER_64_SHL(tmp, 32);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_nnSmac;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[6]);
    COMPILER_64_SHL(tmp, 40);
    COMPILER_64_OR(*tmp0, tmp);
  };
#else

  /* Unpack operation based on little endian */

  /* Unpack Member: m_nReserved */
  (pToStruct)->m_nReserved =  (uint32)  (pFromData)[6] ;
  (pToStruct)->m_nReserved |=  (uint32)  (pFromData)[7] << 8;

  /* Unpack Member: m_nnSmac */
  COMPILER_64_SET((pToStruct)->m_nnSmac, 0,  (unsigned int) (pFromData)[0]);
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_nnSmac;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[1]);
    COMPILER_64_SHL(tmp, 8);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_nnSmac;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[2]);
    COMPILER_64_SHL(tmp, 16);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_nnSmac;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[3]);
    COMPILER_64_SHL(tmp, 24);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_nnSmac;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[4]);
    COMPILER_64_SHL(tmp, 32);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    volatile uint64 *tmp0 = &(pToStruct)->m_nnSmac;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[5]);
    COMPILER_64_SHL(tmp, 40);
    COMPILER_64_OR(*tmp0, tmp);
  };
#endif

}



/* initialize an instance of this zframe */
void
sbZfKaEpIpPortVridSmacTableEntry_InitInstance(sbZfKaEpIpPortVridSmacTableEntry_t *pFrame) {

  pFrame->m_nReserved =  (unsigned int)  0;
  COMPILER_64_ZERO(pFrame->m_nnSmac);

}
