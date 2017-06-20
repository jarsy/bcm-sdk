/*
 * $Id: sbZfFabQe2000LnaFullRemapEntry.c,v 1.3 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


#include <shared/bsl.h>

#include "sbTypes.h"
#include "sbZfFabQe2000LnaFullRemapEntry.hx"
#include "sbWrappers.h"
#include <sal/types.h>



uint32
sbZfFabQe2000LnaFullRemapEntry_GetnRemap(sbZfFabQe2000LnaFullRemapEntry_t *pFrom, UINT nIndex) {
  return pFrom->m_nRemap[nIndex];
}

void sbZfFabQe2000LnaFullRemapEntry_SetnRemap(sbZfFabQe2000LnaFullRemapEntry_t *pFrom, UINT nIndex, uint32 value) {
  pFrom->m_nRemap[nIndex] = value;
}




/* Pack from struct into array of bytes */
uint32
sbZfFabQe2000LnaFullRemapEntry_Pack(sbZfFabQe2000LnaFullRemapEntry_t *pFrom,
                                    uint8 *pToData,
                                    uint32 nMaxToDataIndex) {
#ifdef SAND_BIG_ENDIAN_HOST
  int i;
  int size = SB_ZF_FAB_QE2000_LNA_FULL_REMAP_ENTRY_SIZE_IN_BYTES;

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on bigword endian */

  /* Pack Member: m_nRemap[0] */
  (pToData)[3] |= ((pFrom)->m_nRemap[0] & 0x3f);

  /* Pack Member: m_nRemap[1] */
  (pToData)[3] |= ((pFrom)->m_nRemap[1] & 0x03) <<6;
  (pToData)[2] |= ((pFrom)->m_nRemap[1] >> 2) & 0x0f;

  /* Pack Member: m_nRemap[2] */
  (pToData)[2] |= ((pFrom)->m_nRemap[2] & 0x0f) <<4;
  (pToData)[1] |= ((pFrom)->m_nRemap[2] >> 4) & 0x03;

  /* Pack Member: m_nRemap[3] */
  (pToData)[1] |= ((pFrom)->m_nRemap[3] & 0x3f) <<2;

  /* Pack Member: m_nRemap[4] */
  (pToData)[0] |= ((pFrom)->m_nRemap[4] & 0x3f);

  /* Pack Member: m_nRemap[5] */
  (pToData)[0] |= ((pFrom)->m_nRemap[5] & 0x03) <<6;
  (pToData)[7] |= ((pFrom)->m_nRemap[5] >> 2) & 0x0f;

  /* Pack Member: m_nRemap[6] */
  (pToData)[7] |= ((pFrom)->m_nRemap[6] & 0x0f) <<4;
  (pToData)[6] |= ((pFrom)->m_nRemap[6] >> 4) & 0x03;

  /* Pack Member: m_nRemap[7] */
  (pToData)[6] |= ((pFrom)->m_nRemap[7] & 0x3f) <<2;

  /* Pack Member: m_nRemap[8] */
  (pToData)[5] |= ((pFrom)->m_nRemap[8] & 0x3f);

  /* Pack Member: m_nRemap[9] */
  (pToData)[5] |= ((pFrom)->m_nRemap[9] & 0x03) <<6;
  (pToData)[4] |= ((pFrom)->m_nRemap[9] >> 2) & 0x0f;

  /* Pack Member: m_nRemap[10] */
  (pToData)[4] |= ((pFrom)->m_nRemap[10] & 0x0f) <<4;
  (pToData)[11] |= ((pFrom)->m_nRemap[10] >> 4) & 0x03;

  /* Pack Member: m_nRemap[11] */
  (pToData)[11] |= ((pFrom)->m_nRemap[11] & 0x3f) <<2;

  /* Pack Member: m_nRemap[12] */
  (pToData)[10] |= ((pFrom)->m_nRemap[12] & 0x3f);

  /* Pack Member: m_nRemap[13] */
  (pToData)[10] |= ((pFrom)->m_nRemap[13] & 0x03) <<6;
  (pToData)[9] |= ((pFrom)->m_nRemap[13] >> 2) & 0x0f;

  /* Pack Member: m_nRemap[14] */
  (pToData)[9] |= ((pFrom)->m_nRemap[14] & 0x0f) <<4;
  (pToData)[8] |= ((pFrom)->m_nRemap[14] >> 4) & 0x03;

  /* Pack Member: m_nRemap[15] */
  (pToData)[8] |= ((pFrom)->m_nRemap[15] & 0x3f) <<2;

  /* Pack Member: m_nRemap[16] */
  (pToData)[15] |= ((pFrom)->m_nRemap[16] & 0x3f);

  /* Pack Member: m_nRemap[17] */
  (pToData)[15] |= ((pFrom)->m_nRemap[17] & 0x03) <<6;
  (pToData)[14] |= ((pFrom)->m_nRemap[17] >> 2) & 0x0f;

  /* Pack Member: m_nRemap[18] */
  (pToData)[14] |= ((pFrom)->m_nRemap[18] & 0x0f) <<4;
  (pToData)[13] |= ((pFrom)->m_nRemap[18] >> 4) & 0x03;

  /* Pack Member: m_nRemap[19] */
  (pToData)[13] |= ((pFrom)->m_nRemap[19] & 0x3f) <<2;

  /* Pack Member: m_nRemap[20] */
  (pToData)[12] |= ((pFrom)->m_nRemap[20] & 0x3f);

  /* Pack Member: m_nRemap[21] */
  (pToData)[12] |= ((pFrom)->m_nRemap[21] & 0x03) <<6;
  (pToData)[19] |= ((pFrom)->m_nRemap[21] >> 2) & 0x0f;

  /* Pack Member: m_nRemap[22] */
  (pToData)[19] |= ((pFrom)->m_nRemap[22] & 0x0f) <<4;
  (pToData)[18] |= ((pFrom)->m_nRemap[22] >> 4) & 0x03;

  /* Pack Member: m_nRemap[23] */
  (pToData)[18] |= ((pFrom)->m_nRemap[23] & 0x3f) <<2;

  /* Pack Member: m_nRemap[24] */
  (pToData)[17] |= ((pFrom)->m_nRemap[24] & 0x3f);
#else
  int i;
  int size = SB_ZF_FAB_QE2000_LNA_FULL_REMAP_ENTRY_SIZE_IN_BYTES;

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on little endian */

  /* Pack Member: m_nRemap[0] */
  (pToData)[0] |= ((pFrom)->m_nRemap[0] & 0x3f);

  /* Pack Member: m_nRemap[1] */
  (pToData)[0] |= ((pFrom)->m_nRemap[1] & 0x03) <<6;
  (pToData)[1] |= ((pFrom)->m_nRemap[1] >> 2) & 0x0f;

  /* Pack Member: m_nRemap[2] */
  (pToData)[1] |= ((pFrom)->m_nRemap[2] & 0x0f) <<4;
  (pToData)[2] |= ((pFrom)->m_nRemap[2] >> 4) & 0x03;

  /* Pack Member: m_nRemap[3] */
  (pToData)[2] |= ((pFrom)->m_nRemap[3] & 0x3f) <<2;

  /* Pack Member: m_nRemap[4] */
  (pToData)[3] |= ((pFrom)->m_nRemap[4] & 0x3f);

  /* Pack Member: m_nRemap[5] */
  (pToData)[3] |= ((pFrom)->m_nRemap[5] & 0x03) <<6;
  (pToData)[4] |= ((pFrom)->m_nRemap[5] >> 2) & 0x0f;

  /* Pack Member: m_nRemap[6] */
  (pToData)[4] |= ((pFrom)->m_nRemap[6] & 0x0f) <<4;
  (pToData)[5] |= ((pFrom)->m_nRemap[6] >> 4) & 0x03;

  /* Pack Member: m_nRemap[7] */
  (pToData)[5] |= ((pFrom)->m_nRemap[7] & 0x3f) <<2;

  /* Pack Member: m_nRemap[8] */
  (pToData)[6] |= ((pFrom)->m_nRemap[8] & 0x3f);

  /* Pack Member: m_nRemap[9] */
  (pToData)[6] |= ((pFrom)->m_nRemap[9] & 0x03) <<6;
  (pToData)[7] |= ((pFrom)->m_nRemap[9] >> 2) & 0x0f;

  /* Pack Member: m_nRemap[10] */
  (pToData)[7] |= ((pFrom)->m_nRemap[10] & 0x0f) <<4;
  (pToData)[8] |= ((pFrom)->m_nRemap[10] >> 4) & 0x03;

  /* Pack Member: m_nRemap[11] */
  (pToData)[8] |= ((pFrom)->m_nRemap[11] & 0x3f) <<2;

  /* Pack Member: m_nRemap[12] */
  (pToData)[9] |= ((pFrom)->m_nRemap[12] & 0x3f);

  /* Pack Member: m_nRemap[13] */
  (pToData)[9] |= ((pFrom)->m_nRemap[13] & 0x03) <<6;
  (pToData)[10] |= ((pFrom)->m_nRemap[13] >> 2) & 0x0f;

  /* Pack Member: m_nRemap[14] */
  (pToData)[10] |= ((pFrom)->m_nRemap[14] & 0x0f) <<4;
  (pToData)[11] |= ((pFrom)->m_nRemap[14] >> 4) & 0x03;

  /* Pack Member: m_nRemap[15] */
  (pToData)[11] |= ((pFrom)->m_nRemap[15] & 0x3f) <<2;

  /* Pack Member: m_nRemap[16] */
  (pToData)[12] |= ((pFrom)->m_nRemap[16] & 0x3f);

  /* Pack Member: m_nRemap[17] */
  (pToData)[12] |= ((pFrom)->m_nRemap[17] & 0x03) <<6;
  (pToData)[13] |= ((pFrom)->m_nRemap[17] >> 2) & 0x0f;

  /* Pack Member: m_nRemap[18] */
  (pToData)[13] |= ((pFrom)->m_nRemap[18] & 0x0f) <<4;
  (pToData)[14] |= ((pFrom)->m_nRemap[18] >> 4) & 0x03;

  /* Pack Member: m_nRemap[19] */
  (pToData)[14] |= ((pFrom)->m_nRemap[19] & 0x3f) <<2;

  /* Pack Member: m_nRemap[20] */
  (pToData)[15] |= ((pFrom)->m_nRemap[20] & 0x3f);

  /* Pack Member: m_nRemap[21] */
  (pToData)[15] |= ((pFrom)->m_nRemap[21] & 0x03) <<6;
  (pToData)[16] |= ((pFrom)->m_nRemap[21] >> 2) & 0x0f;

  /* Pack Member: m_nRemap[22] */
  (pToData)[16] |= ((pFrom)->m_nRemap[22] & 0x0f) <<4;
  (pToData)[17] |= ((pFrom)->m_nRemap[22] >> 4) & 0x03;

  /* Pack Member: m_nRemap[23] */
  (pToData)[17] |= ((pFrom)->m_nRemap[23] & 0x3f) <<2;

  /* Pack Member: m_nRemap[24] */
  (pToData)[18] |= ((pFrom)->m_nRemap[24] & 0x3f);
#endif

  return SB_ZF_FAB_QE2000_LNA_FULL_REMAP_ENTRY_SIZE_IN_BYTES;
}




/* Unpack from array of bytes into struct */
void
sbZfFabQe2000LnaFullRemapEntry_Unpack(sbZfFabQe2000LnaFullRemapEntry_t *pToStruct,
                                      uint8 *pFromData,
                                      uint32 nMaxToDataIndex) {
  COMPILER_UINT64 tmp;

  (void) tmp;

#ifdef SAND_BIG_ENDIAN_HOST

  /* Unpack operation based on bigword endian */

  /* Unpack Member: m_nRemap[0] */
  (pToStruct)->m_nRemap[0] =  (uint32)  ((pFromData)[3] ) & 0x3f;

  /* Unpack Member: m_nRemap[1] */
  (pToStruct)->m_nRemap[1] =  (uint32)  ((pFromData)[3] >> 6) & 0x03;
  (pToStruct)->m_nRemap[1] |=  (uint32)  ((pFromData)[2] & 0x0f) << 2;

  /* Unpack Member: m_nRemap[2] */
  (pToStruct)->m_nRemap[2] =  (uint32)  ((pFromData)[2] >> 4) & 0x0f;
  (pToStruct)->m_nRemap[2] |=  (uint32)  ((pFromData)[1] & 0x03) << 4;

  /* Unpack Member: m_nRemap[3] */
  (pToStruct)->m_nRemap[3] =  (uint32)  ((pFromData)[1] >> 2) & 0x3f;

  /* Unpack Member: m_nRemap[4] */
  (pToStruct)->m_nRemap[4] =  (uint32)  ((pFromData)[0] ) & 0x3f;

  /* Unpack Member: m_nRemap[5] */
  (pToStruct)->m_nRemap[5] =  (uint32)  ((pFromData)[0] >> 6) & 0x03;
  (pToStruct)->m_nRemap[5] |=  (uint32)  ((pFromData)[7] & 0x0f) << 2;

  /* Unpack Member: m_nRemap[6] */
  (pToStruct)->m_nRemap[6] =  (uint32)  ((pFromData)[7] >> 4) & 0x0f;
  (pToStruct)->m_nRemap[6] |=  (uint32)  ((pFromData)[6] & 0x03) << 4;

  /* Unpack Member: m_nRemap[7] */
  (pToStruct)->m_nRemap[7] =  (uint32)  ((pFromData)[6] >> 2) & 0x3f;

  /* Unpack Member: m_nRemap[8] */
  (pToStruct)->m_nRemap[8] =  (uint32)  ((pFromData)[5] ) & 0x3f;

  /* Unpack Member: m_nRemap[9] */
  (pToStruct)->m_nRemap[9] =  (uint32)  ((pFromData)[5] >> 6) & 0x03;
  (pToStruct)->m_nRemap[9] |=  (uint32)  ((pFromData)[4] & 0x0f) << 2;

  /* Unpack Member: m_nRemap[10] */
  (pToStruct)->m_nRemap[10] =  (uint32)  ((pFromData)[4] >> 4) & 0x0f;
  (pToStruct)->m_nRemap[10] |=  (uint32)  ((pFromData)[11] & 0x03) << 4;

  /* Unpack Member: m_nRemap[11] */
  (pToStruct)->m_nRemap[11] =  (uint32)  ((pFromData)[11] >> 2) & 0x3f;

  /* Unpack Member: m_nRemap[12] */
  (pToStruct)->m_nRemap[12] =  (uint32)  ((pFromData)[10] ) & 0x3f;

  /* Unpack Member: m_nRemap[13] */
  (pToStruct)->m_nRemap[13] =  (uint32)  ((pFromData)[10] >> 6) & 0x03;
  (pToStruct)->m_nRemap[13] |=  (uint32)  ((pFromData)[9] & 0x0f) << 2;

  /* Unpack Member: m_nRemap[14] */
  (pToStruct)->m_nRemap[14] =  (uint32)  ((pFromData)[9] >> 4) & 0x0f;
  (pToStruct)->m_nRemap[14] |=  (uint32)  ((pFromData)[8] & 0x03) << 4;

  /* Unpack Member: m_nRemap[15] */
  (pToStruct)->m_nRemap[15] =  (uint32)  ((pFromData)[8] >> 2) & 0x3f;

  /* Unpack Member: m_nRemap[16] */
  (pToStruct)->m_nRemap[16] =  (uint32)  ((pFromData)[15] ) & 0x3f;

  /* Unpack Member: m_nRemap[17] */
  (pToStruct)->m_nRemap[17] =  (uint32)  ((pFromData)[15] >> 6) & 0x03;
  (pToStruct)->m_nRemap[17] |=  (uint32)  ((pFromData)[14] & 0x0f) << 2;

  /* Unpack Member: m_nRemap[18] */
  (pToStruct)->m_nRemap[18] =  (uint32)  ((pFromData)[14] >> 4) & 0x0f;
  (pToStruct)->m_nRemap[18] |=  (uint32)  ((pFromData)[13] & 0x03) << 4;

  /* Unpack Member: m_nRemap[19] */
  (pToStruct)->m_nRemap[19] =  (uint32)  ((pFromData)[13] >> 2) & 0x3f;

  /* Unpack Member: m_nRemap[20] */
  (pToStruct)->m_nRemap[20] =  (uint32)  ((pFromData)[12] ) & 0x3f;

  /* Unpack Member: m_nRemap[21] */
  (pToStruct)->m_nRemap[21] =  (uint32)  ((pFromData)[12] >> 6) & 0x03;
  (pToStruct)->m_nRemap[21] |=  (uint32)  ((pFromData)[19] & 0x0f) << 2;

  /* Unpack Member: m_nRemap[22] */
  (pToStruct)->m_nRemap[22] =  (uint32)  ((pFromData)[19] >> 4) & 0x0f;
  (pToStruct)->m_nRemap[22] |=  (uint32)  ((pFromData)[18] & 0x03) << 4;

  /* Unpack Member: m_nRemap[23] */
  (pToStruct)->m_nRemap[23] =  (uint32)  ((pFromData)[18] >> 2) & 0x3f;

  /* Unpack Member: m_nRemap[24] */
  (pToStruct)->m_nRemap[24] =  (uint32)  ((pFromData)[17] ) & 0x3f;
#else

  /* Unpack operation based on little endian */

  /* Unpack Member: m_nRemap[0] */
  (pToStruct)->m_nRemap[0] =  (uint32)  ((pFromData)[0] ) & 0x3f;

  /* Unpack Member: m_nRemap[1] */
  (pToStruct)->m_nRemap[1] =  (uint32)  ((pFromData)[0] >> 6) & 0x03;
  (pToStruct)->m_nRemap[1] |=  (uint32)  ((pFromData)[1] & 0x0f) << 2;

  /* Unpack Member: m_nRemap[2] */
  (pToStruct)->m_nRemap[2] =  (uint32)  ((pFromData)[1] >> 4) & 0x0f;
  (pToStruct)->m_nRemap[2] |=  (uint32)  ((pFromData)[2] & 0x03) << 4;

  /* Unpack Member: m_nRemap[3] */
  (pToStruct)->m_nRemap[3] =  (uint32)  ((pFromData)[2] >> 2) & 0x3f;

  /* Unpack Member: m_nRemap[4] */
  (pToStruct)->m_nRemap[4] =  (uint32)  ((pFromData)[3] ) & 0x3f;

  /* Unpack Member: m_nRemap[5] */
  (pToStruct)->m_nRemap[5] =  (uint32)  ((pFromData)[3] >> 6) & 0x03;
  (pToStruct)->m_nRemap[5] |=  (uint32)  ((pFromData)[4] & 0x0f) << 2;

  /* Unpack Member: m_nRemap[6] */
  (pToStruct)->m_nRemap[6] =  (uint32)  ((pFromData)[4] >> 4) & 0x0f;
  (pToStruct)->m_nRemap[6] |=  (uint32)  ((pFromData)[5] & 0x03) << 4;

  /* Unpack Member: m_nRemap[7] */
  (pToStruct)->m_nRemap[7] =  (uint32)  ((pFromData)[5] >> 2) & 0x3f;

  /* Unpack Member: m_nRemap[8] */
  (pToStruct)->m_nRemap[8] =  (uint32)  ((pFromData)[6] ) & 0x3f;

  /* Unpack Member: m_nRemap[9] */
  (pToStruct)->m_nRemap[9] =  (uint32)  ((pFromData)[6] >> 6) & 0x03;
  (pToStruct)->m_nRemap[9] |=  (uint32)  ((pFromData)[7] & 0x0f) << 2;

  /* Unpack Member: m_nRemap[10] */
  (pToStruct)->m_nRemap[10] =  (uint32)  ((pFromData)[7] >> 4) & 0x0f;
  (pToStruct)->m_nRemap[10] |=  (uint32)  ((pFromData)[8] & 0x03) << 4;

  /* Unpack Member: m_nRemap[11] */
  (pToStruct)->m_nRemap[11] =  (uint32)  ((pFromData)[8] >> 2) & 0x3f;

  /* Unpack Member: m_nRemap[12] */
  (pToStruct)->m_nRemap[12] =  (uint32)  ((pFromData)[9] ) & 0x3f;

  /* Unpack Member: m_nRemap[13] */
  (pToStruct)->m_nRemap[13] =  (uint32)  ((pFromData)[9] >> 6) & 0x03;
  (pToStruct)->m_nRemap[13] |=  (uint32)  ((pFromData)[10] & 0x0f) << 2;

  /* Unpack Member: m_nRemap[14] */
  (pToStruct)->m_nRemap[14] =  (uint32)  ((pFromData)[10] >> 4) & 0x0f;
  (pToStruct)->m_nRemap[14] |=  (uint32)  ((pFromData)[11] & 0x03) << 4;

  /* Unpack Member: m_nRemap[15] */
  (pToStruct)->m_nRemap[15] =  (uint32)  ((pFromData)[11] >> 2) & 0x3f;

  /* Unpack Member: m_nRemap[16] */
  (pToStruct)->m_nRemap[16] =  (uint32)  ((pFromData)[12] ) & 0x3f;

  /* Unpack Member: m_nRemap[17] */
  (pToStruct)->m_nRemap[17] =  (uint32)  ((pFromData)[12] >> 6) & 0x03;
  (pToStruct)->m_nRemap[17] |=  (uint32)  ((pFromData)[13] & 0x0f) << 2;

  /* Unpack Member: m_nRemap[18] */
  (pToStruct)->m_nRemap[18] =  (uint32)  ((pFromData)[13] >> 4) & 0x0f;
  (pToStruct)->m_nRemap[18] |=  (uint32)  ((pFromData)[14] & 0x03) << 4;

  /* Unpack Member: m_nRemap[19] */
  (pToStruct)->m_nRemap[19] =  (uint32)  ((pFromData)[14] >> 2) & 0x3f;

  /* Unpack Member: m_nRemap[20] */
  (pToStruct)->m_nRemap[20] =  (uint32)  ((pFromData)[15] ) & 0x3f;

  /* Unpack Member: m_nRemap[21] */
  (pToStruct)->m_nRemap[21] =  (uint32)  ((pFromData)[15] >> 6) & 0x03;
  (pToStruct)->m_nRemap[21] |=  (uint32)  ((pFromData)[16] & 0x0f) << 2;

  /* Unpack Member: m_nRemap[22] */
  (pToStruct)->m_nRemap[22] =  (uint32)  ((pFromData)[16] >> 4) & 0x0f;
  (pToStruct)->m_nRemap[22] |=  (uint32)  ((pFromData)[17] & 0x03) << 4;

  /* Unpack Member: m_nRemap[23] */
  (pToStruct)->m_nRemap[23] =  (uint32)  ((pFromData)[17] >> 2) & 0x3f;

  /* Unpack Member: m_nRemap[24] */
  (pToStruct)->m_nRemap[24] =  (uint32)  ((pFromData)[18] ) & 0x3f;
#endif

}



/* initialize an instance of this zframe */
void
sbZfFabQe2000LnaFullRemapEntry_InitInstance(sbZfFabQe2000LnaFullRemapEntry_t *pFrame) {

  pFrame->m_nRemap[0] =  (unsigned int)  0;
  pFrame->m_nRemap[1] =  (unsigned int)  0;
  pFrame->m_nRemap[2] =  (unsigned int)  0;
  pFrame->m_nRemap[3] =  (unsigned int)  0;
  pFrame->m_nRemap[4] =  (unsigned int)  0;
  pFrame->m_nRemap[5] =  (unsigned int)  0;
  pFrame->m_nRemap[6] =  (unsigned int)  0;
  pFrame->m_nRemap[7] =  (unsigned int)  0;
  pFrame->m_nRemap[8] =  (unsigned int)  0;
  pFrame->m_nRemap[9] =  (unsigned int)  0;
  pFrame->m_nRemap[10] =  (unsigned int)  0;
  pFrame->m_nRemap[11] =  (unsigned int)  0;
  pFrame->m_nRemap[12] =  (unsigned int)  0;
  pFrame->m_nRemap[13] =  (unsigned int)  0;
  pFrame->m_nRemap[14] =  (unsigned int)  0;
  pFrame->m_nRemap[15] =  (unsigned int)  0;
  pFrame->m_nRemap[16] =  (unsigned int)  0;
  pFrame->m_nRemap[17] =  (unsigned int)  0;
  pFrame->m_nRemap[18] =  (unsigned int)  0;
  pFrame->m_nRemap[19] =  (unsigned int)  0;
  pFrame->m_nRemap[20] =  (unsigned int)  0;
  pFrame->m_nRemap[21] =  (unsigned int)  0;
  pFrame->m_nRemap[22] =  (unsigned int)  0;
  pFrame->m_nRemap[23] =  (unsigned int)  0;
  pFrame->m_nRemap[24] =  (unsigned int)  0;

}
#ifdef SB_ZF_INCLUDE_CONSOLE
/*
 * $Id: sbZfFabQe2000LnaFullRemapEntry.c,v 1.3 Broadcom SDK $
 * $Copyright (c) 2012 Broadcom Corporation
 * All rights reserved.$
 */
#include "sbTypes.h"
#include <soc/sbx/sbWrappers.h>
#include "sbZfFabQe2000LnaFullRemapEntry.hx"



/* Print members in struct */
void
sbZfFabQe2000LnaFullRemapEntry_Print(sbZfFabQe2000LnaFullRemapEntry_t *pFromStruct) {
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("FabQe2000LnaFullRemapEntry:: remap0=0x%02x"), (unsigned int)  pFromStruct->m_nRemap[0]));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" remap1=0x%02x"), (unsigned int)  pFromStruct->m_nRemap[1]));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" remap2=0x%02x"), (unsigned int)  pFromStruct->m_nRemap[2]));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("FabQe2000LnaFullRemapEntry:: remap3=0x%02x"), (unsigned int)  pFromStruct->m_nRemap[3]));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" remap4=0x%02x"), (unsigned int)  pFromStruct->m_nRemap[4]));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" remap5=0x%02x"), (unsigned int)  pFromStruct->m_nRemap[5]));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("FabQe2000LnaFullRemapEntry:: remap6=0x%02x"), (unsigned int)  pFromStruct->m_nRemap[6]));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" remap7=0x%02x"), (unsigned int)  pFromStruct->m_nRemap[7]));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" remap8=0x%02x"), (unsigned int)  pFromStruct->m_nRemap[8]));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("FabQe2000LnaFullRemapEntry:: remap9=0x%02x"), (unsigned int)  pFromStruct->m_nRemap[9]));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" remap10=0x%02x"), (unsigned int)  pFromStruct->m_nRemap[10]));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" remap11=0x%02x"), (unsigned int)  pFromStruct->m_nRemap[11]));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("FabQe2000LnaFullRemapEntry:: remap12=0x%02x"), (unsigned int)  pFromStruct->m_nRemap[12]));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" remap13=0x%02x"), (unsigned int)  pFromStruct->m_nRemap[13]));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" remap14=0x%02x"), (unsigned int)  pFromStruct->m_nRemap[14]));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("FabQe2000LnaFullRemapEntry:: remap15=0x%02x"), (unsigned int)  pFromStruct->m_nRemap[15]));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" remap16=0x%02x"), (unsigned int)  pFromStruct->m_nRemap[16]));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" remap17=0x%02x"), (unsigned int)  pFromStruct->m_nRemap[17]));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("FabQe2000LnaFullRemapEntry:: remap18=0x%02x"), (unsigned int)  pFromStruct->m_nRemap[18]));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" remap19=0x%02x"), (unsigned int)  pFromStruct->m_nRemap[19]));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" remap20=0x%02x"), (unsigned int)  pFromStruct->m_nRemap[20]));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("FabQe2000LnaFullRemapEntry:: remap21=0x%02x"), (unsigned int)  pFromStruct->m_nRemap[21]));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" remap22=0x%02x"), (unsigned int)  pFromStruct->m_nRemap[22]));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" remap23=0x%02x"), (unsigned int)  pFromStruct->m_nRemap[23]));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("FabQe2000LnaFullRemapEntry:: remap24=0x%02x"), (unsigned int)  pFromStruct->m_nRemap[24]));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

}

/* SPrint members in struct */
int
sbZfFabQe2000LnaFullRemapEntry_SPrint(sbZfFabQe2000LnaFullRemapEntry_t *pFromStruct, char *pcToString, uint32 lStrSize) {
  uint32 WrCnt = 0x0;

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"FabQe2000LnaFullRemapEntry:: remap0=0x%02x", (unsigned int)  pFromStruct->m_nRemap[0]);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," remap1=0x%02x", (unsigned int)  pFromStruct->m_nRemap[1]);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," remap2=0x%02x", (unsigned int)  pFromStruct->m_nRemap[2]);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"FabQe2000LnaFullRemapEntry:: remap3=0x%02x", (unsigned int)  pFromStruct->m_nRemap[3]);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," remap4=0x%02x", (unsigned int)  pFromStruct->m_nRemap[4]);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," remap5=0x%02x", (unsigned int)  pFromStruct->m_nRemap[5]);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"FabQe2000LnaFullRemapEntry:: remap6=0x%02x", (unsigned int)  pFromStruct->m_nRemap[6]);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," remap7=0x%02x", (unsigned int)  pFromStruct->m_nRemap[7]);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," remap8=0x%02x", (unsigned int)  pFromStruct->m_nRemap[8]);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"FabQe2000LnaFullRemapEntry:: remap9=0x%02x", (unsigned int)  pFromStruct->m_nRemap[9]);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," remap10=0x%02x", (unsigned int)  pFromStruct->m_nRemap[10]);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," remap11=0x%02x", (unsigned int)  pFromStruct->m_nRemap[11]);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"FabQe2000LnaFullRemapEntry:: remap12=0x%02x", (unsigned int)  pFromStruct->m_nRemap[12]);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," remap13=0x%02x", (unsigned int)  pFromStruct->m_nRemap[13]);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," remap14=0x%02x", (unsigned int)  pFromStruct->m_nRemap[14]);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"FabQe2000LnaFullRemapEntry:: remap15=0x%02x", (unsigned int)  pFromStruct->m_nRemap[15]);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," remap16=0x%02x", (unsigned int)  pFromStruct->m_nRemap[16]);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," remap17=0x%02x", (unsigned int)  pFromStruct->m_nRemap[17]);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"FabQe2000LnaFullRemapEntry:: remap18=0x%02x", (unsigned int)  pFromStruct->m_nRemap[18]);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," remap19=0x%02x", (unsigned int)  pFromStruct->m_nRemap[19]);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," remap20=0x%02x", (unsigned int)  pFromStruct->m_nRemap[20]);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"FabQe2000LnaFullRemapEntry:: remap21=0x%02x", (unsigned int)  pFromStruct->m_nRemap[21]);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," remap22=0x%02x", (unsigned int)  pFromStruct->m_nRemap[22]);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," remap23=0x%02x", (unsigned int)  pFromStruct->m_nRemap[23]);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"FabQe2000LnaFullRemapEntry:: remap24=0x%02x", (unsigned int)  pFromStruct->m_nRemap[24]);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  /* assert if we've overrun the buffer */
  SB_ASSERT(WrCnt < lStrSize);
  return(WrCnt);

}

/* validate members in struct (1 = PASS, 0 = FAIL) */
int
sbZfFabQe2000LnaFullRemapEntry_Validate(sbZfFabQe2000LnaFullRemapEntry_t *pZf) {

  if (pZf->m_nRemap[0] > 0x3f) return 0;
  if (pZf->m_nRemap[1] > 0x3f) return 0;
  if (pZf->m_nRemap[2] > 0x3f) return 0;
  if (pZf->m_nRemap[3] > 0x3f) return 0;
  if (pZf->m_nRemap[4] > 0x3f) return 0;
  if (pZf->m_nRemap[5] > 0x3f) return 0;
  if (pZf->m_nRemap[6] > 0x3f) return 0;
  if (pZf->m_nRemap[7] > 0x3f) return 0;
  if (pZf->m_nRemap[8] > 0x3f) return 0;
  if (pZf->m_nRemap[9] > 0x3f) return 0;
  if (pZf->m_nRemap[10] > 0x3f) return 0;
  if (pZf->m_nRemap[11] > 0x3f) return 0;
  if (pZf->m_nRemap[12] > 0x3f) return 0;
  if (pZf->m_nRemap[13] > 0x3f) return 0;
  if (pZf->m_nRemap[14] > 0x3f) return 0;
  if (pZf->m_nRemap[15] > 0x3f) return 0;
  if (pZf->m_nRemap[16] > 0x3f) return 0;
  if (pZf->m_nRemap[17] > 0x3f) return 0;
  if (pZf->m_nRemap[18] > 0x3f) return 0;
  if (pZf->m_nRemap[19] > 0x3f) return 0;
  if (pZf->m_nRemap[20] > 0x3f) return 0;
  if (pZf->m_nRemap[21] > 0x3f) return 0;
  if (pZf->m_nRemap[22] > 0x3f) return 0;
  if (pZf->m_nRemap[23] > 0x3f) return 0;
  if (pZf->m_nRemap[24] > 0x3f) return 0;

  return 1; /* success */

}

/* populate field from string and value */
int
sbZfFabQe2000LnaFullRemapEntry_SetField(sbZfFabQe2000LnaFullRemapEntry_t *s, char* name, int value) {

  if (SB_STRCMP(name, "") == 0 ) {
    return -1;
  } else if (SB_STRCMP(name, "m_nremap[0]") == 0) {
    s->m_nRemap[0] = value;
  } else if (SB_STRCMP(name, "m_nremap[1]") == 0) {
    s->m_nRemap[1] = value;
  } else if (SB_STRCMP(name, "m_nremap[2]") == 0) {
    s->m_nRemap[2] = value;
  } else if (SB_STRCMP(name, "m_nremap[3]") == 0) {
    s->m_nRemap[3] = value;
  } else if (SB_STRCMP(name, "m_nremap[4]") == 0) {
    s->m_nRemap[4] = value;
  } else if (SB_STRCMP(name, "m_nremap[5]") == 0) {
    s->m_nRemap[5] = value;
  } else if (SB_STRCMP(name, "m_nremap[6]") == 0) {
    s->m_nRemap[6] = value;
  } else if (SB_STRCMP(name, "m_nremap[7]") == 0) {
    s->m_nRemap[7] = value;
  } else if (SB_STRCMP(name, "m_nremap[8]") == 0) {
    s->m_nRemap[8] = value;
  } else if (SB_STRCMP(name, "m_nremap[9]") == 0) {
    s->m_nRemap[9] = value;
  } else if (SB_STRCMP(name, "m_nremap[10]") == 0) {
    s->m_nRemap[10] = value;
  } else if (SB_STRCMP(name, "m_nremap[11]") == 0) {
    s->m_nRemap[11] = value;
  } else if (SB_STRCMP(name, "m_nremap[12]") == 0) {
    s->m_nRemap[12] = value;
  } else if (SB_STRCMP(name, "m_nremap[13]") == 0) {
    s->m_nRemap[13] = value;
  } else if (SB_STRCMP(name, "m_nremap[14]") == 0) {
    s->m_nRemap[14] = value;
  } else if (SB_STRCMP(name, "m_nremap[15]") == 0) {
    s->m_nRemap[15] = value;
  } else if (SB_STRCMP(name, "m_nremap[16]") == 0) {
    s->m_nRemap[16] = value;
  } else if (SB_STRCMP(name, "m_nremap[17]") == 0) {
    s->m_nRemap[17] = value;
  } else if (SB_STRCMP(name, "m_nremap[18]") == 0) {
    s->m_nRemap[18] = value;
  } else if (SB_STRCMP(name, "m_nremap[19]") == 0) {
    s->m_nRemap[19] = value;
  } else if (SB_STRCMP(name, "m_nremap[20]") == 0) {
    s->m_nRemap[20] = value;
  } else if (SB_STRCMP(name, "m_nremap[21]") == 0) {
    s->m_nRemap[21] = value;
  } else if (SB_STRCMP(name, "m_nremap[22]") == 0) {
    s->m_nRemap[22] = value;
  } else if (SB_STRCMP(name, "m_nremap[23]") == 0) {
    s->m_nRemap[23] = value;
  } else if (SB_STRCMP(name, "m_nremap[24]") == 0) {
    s->m_nRemap[24] = value;
  } else {
    /* string failed to match any field--ignored */
    return -1;
  }

  return(0);

}


#endif /* ifdef SB_ZF_INCLUDE_CONSOLE */

