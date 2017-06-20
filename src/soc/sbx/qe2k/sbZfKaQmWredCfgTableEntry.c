/*
 * $Id: sbZfKaQmWredCfgTableEntry.c,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


#include "sbTypes.h"
#include "sbZfKaQmWredCfgTableEntry.hx"
#include "sbWrappers.h"
#include <sal/types.h>



/* Pack from struct into array of bytes */
uint32
sbZfKaQmWredCfgTableEntry_Pack(sbZfKaQmWredCfgTableEntry_t *pFrom,
                               uint8 *pToData,
                               uint32 nMaxToDataIndex) {
#ifdef SAND_BIG_ENDIAN_HOST
  int i;
  int size = SB_ZF_ZFKAQMWREDCFGTABLEENTRY_SIZE_IN_BYTES;

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on bigword endian */

  /* Pack Member: m_nTemplate */
  (pToData)[3] |= ((pFrom)->m_nTemplate & 0x0f) <<4;

  /* Pack Member: m_nGain */
  (pToData)[3] |= ((pFrom)->m_nGain & 0x0f);
#else
  int i;
  int size = SB_ZF_ZFKAQMWREDCFGTABLEENTRY_SIZE_IN_BYTES;

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on little endian */

  /* Pack Member: m_nTemplate */
  (pToData)[0] |= ((pFrom)->m_nTemplate & 0x0f) <<4;

  /* Pack Member: m_nGain */
  (pToData)[0] |= ((pFrom)->m_nGain & 0x0f);
#endif

  return SB_ZF_ZFKAQMWREDCFGTABLEENTRY_SIZE_IN_BYTES;
}




/* Unpack from array of bytes into struct */
void
sbZfKaQmWredCfgTableEntry_Unpack(sbZfKaQmWredCfgTableEntry_t *pToStruct,
                                 uint8 *pFromData,
                                 uint32 nMaxToDataIndex) {
  COMPILER_UINT64 tmp;

  (void) tmp;

#ifdef SAND_BIG_ENDIAN_HOST

  /* Unpack operation based on bigword endian */

  /* Unpack Member: m_nTemplate */
  (pToStruct)->m_nTemplate =  (uint32)  ((pFromData)[3] >> 4) & 0x0f;

  /* Unpack Member: m_nGain */
  (pToStruct)->m_nGain =  (uint32)  ((pFromData)[3] ) & 0x0f;
#else

  /* Unpack operation based on little endian */

  /* Unpack Member: m_nTemplate */
  (pToStruct)->m_nTemplate =  (uint32)  ((pFromData)[0] >> 4) & 0x0f;

  /* Unpack Member: m_nGain */
  (pToStruct)->m_nGain =  (uint32)  ((pFromData)[0] ) & 0x0f;
#endif

}



/* initialize an instance of this zframe */
void
sbZfKaQmWredCfgTableEntry_InitInstance(sbZfKaQmWredCfgTableEntry_t *pFrame) {

  pFrame->m_nTemplate =  (unsigned int)  0;
  pFrame->m_nGain =  (unsigned int)  0;

}
