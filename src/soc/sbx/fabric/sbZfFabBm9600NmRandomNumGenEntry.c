/*
 * $Id: sbZfFabBm9600NmRandomNumGenEntry.c,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


#include "sbTypes.h"
#include "sbZfFabBm9600NmRandomNumGenEntry.hx"
#include "sbWrappers.h"
#include <sal/types.h>



/* Pack from struct into array of bytes */
uint32
sbZfFabBm9600NmRandomNumGenEntry_Pack(sbZfFabBm9600NmRandomNumGenEntry_t *pFrom,
                                      uint8 *pToData,
                                      uint32 nMaxToDataIndex) {
#ifdef SAND_BIG_ENDIAN_HOST
  int i;
  int size = SB_ZF_FAB_BM9600_NMRANDOMNUMGENENTRY_SIZE_IN_BYTES;

  if (size % 4) {
    size += (4 - size %4);
  }

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on bigword endian */

  /* Pack Member: m_uSeed */
  (pToData)[3] |= ((pFrom)->m_uSeed & 0x7f);
#else
  int i;
  int size = SB_ZF_FAB_BM9600_NMRANDOMNUMGENENTRY_SIZE_IN_BYTES;

  if (size % 4) {
    size += (4 - size %4);
  }

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on little endian */

  /* Pack Member: m_uSeed */
  (pToData)[0] |= ((pFrom)->m_uSeed & 0x7f);
#endif

  return SB_ZF_FAB_BM9600_NMRANDOMNUMGENENTRY_SIZE_IN_BYTES;
}




/* Unpack from array of bytes into struct */
void
sbZfFabBm9600NmRandomNumGenEntry_Unpack(sbZfFabBm9600NmRandomNumGenEntry_t *pToStruct,
                                        uint8 *pFromData,
                                        uint32 nMaxToDataIndex) {
  COMPILER_UINT64 tmp;

  (void) tmp;

#ifdef SAND_BIG_ENDIAN_HOST

  /* Unpack operation based on bigword endian */

  /* Unpack Member: m_uSeed */
  (pToStruct)->m_uSeed =  (uint32)  ((pFromData)[3] ) & 0x7f;
#else

  /* Unpack operation based on little endian */

  /* Unpack Member: m_uSeed */
  (pToStruct)->m_uSeed =  (uint32)  ((pFromData)[0] ) & 0x7f;
#endif

}



/* initialize an instance of this zframe */
void
sbZfFabBm9600NmRandomNumGenEntry_InitInstance(sbZfFabBm9600NmRandomNumGenEntry_t *pFrame) {

  pFrame->m_uSeed =  (unsigned int)  0;

}
