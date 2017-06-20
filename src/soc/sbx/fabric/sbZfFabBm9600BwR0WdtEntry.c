/*
 * $Id: sbZfFabBm9600BwR0WdtEntry.c,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


#include "sbTypes.h"
#include "sbZfFabBm9600BwR0WdtEntry.hx"
#include "sbWrappers.h"
#include <sal/types.h>



/* Pack from struct into array of bytes */
uint32
sbZfFabBm9600BwR0WdtEntry_Pack(sbZfFabBm9600BwR0WdtEntry_t *pFrom,
                               uint8 *pToData,
                               uint32 nMaxToDataIndex) {
#ifdef SAND_BIG_ENDIAN_HOST
  int i;
  int size = SB_ZF_FAB_BM9600_BWR0WDTENTRY_SIZE_IN_BYTES;

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on bigword endian */

  /* Pack Member: m_uTemplate1 */
  (pToData)[0] |= ((pFrom)->m_uTemplate1) & 0xFF;

  /* Pack Member: m_uSpare1 */
  (pToData)[1] |= ((pFrom)->m_uSpare1 & 0x0f) <<4;

  /* Pack Member: m_uGain1 */
  (pToData)[1] |= ((pFrom)->m_uGain1 & 0x0f);

  /* Pack Member: m_uTemplate0 */
  (pToData)[2] |= ((pFrom)->m_uTemplate0) & 0xFF;

  /* Pack Member: m_uSpare0 */
  (pToData)[3] |= ((pFrom)->m_uSpare0 & 0x0f) <<4;

  /* Pack Member: m_uGain0 */
  (pToData)[3] |= ((pFrom)->m_uGain0 & 0x0f);
#else
  int i;
  int size = SB_ZF_FAB_BM9600_BWR0WDTENTRY_SIZE_IN_BYTES;

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on little endian */

  /* Pack Member: m_uTemplate1 */
  (pToData)[3] |= ((pFrom)->m_uTemplate1) & 0xFF;

  /* Pack Member: m_uSpare1 */
  (pToData)[2] |= ((pFrom)->m_uSpare1 & 0x0f) <<4;

  /* Pack Member: m_uGain1 */
  (pToData)[2] |= ((pFrom)->m_uGain1 & 0x0f);

  /* Pack Member: m_uTemplate0 */
  (pToData)[1] |= ((pFrom)->m_uTemplate0) & 0xFF;

  /* Pack Member: m_uSpare0 */
  (pToData)[0] |= ((pFrom)->m_uSpare0 & 0x0f) <<4;

  /* Pack Member: m_uGain0 */
  (pToData)[0] |= ((pFrom)->m_uGain0 & 0x0f);
#endif

  return SB_ZF_FAB_BM9600_BWR0WDTENTRY_SIZE_IN_BYTES;
}




/* Unpack from array of bytes into struct */
void
sbZfFabBm9600BwR0WdtEntry_Unpack(sbZfFabBm9600BwR0WdtEntry_t *pToStruct,
                                 uint8 *pFromData,
                                 uint32 nMaxToDataIndex) {
  COMPILER_UINT64 tmp;

  (void) tmp;

#ifdef SAND_BIG_ENDIAN_HOST

  /* Unpack operation based on bigword endian */

  /* Unpack Member: m_uTemplate1 */
  (pToStruct)->m_uTemplate1 =  (uint32)  (pFromData)[0] ;

  /* Unpack Member: m_uSpare1 */
  (pToStruct)->m_uSpare1 =  (uint32)  ((pFromData)[1] >> 4) & 0x0f;

  /* Unpack Member: m_uGain1 */
  (pToStruct)->m_uGain1 =  (uint32)  ((pFromData)[1] ) & 0x0f;

  /* Unpack Member: m_uTemplate0 */
  (pToStruct)->m_uTemplate0 =  (uint32)  (pFromData)[2] ;

  /* Unpack Member: m_uSpare0 */
  (pToStruct)->m_uSpare0 =  (uint32)  ((pFromData)[3] >> 4) & 0x0f;

  /* Unpack Member: m_uGain0 */
  (pToStruct)->m_uGain0 =  (uint32)  ((pFromData)[3] ) & 0x0f;
#else

  /* Unpack operation based on little endian */

  /* Unpack Member: m_uTemplate1 */
  (pToStruct)->m_uTemplate1 =  (uint32)  (pFromData)[3] ;

  /* Unpack Member: m_uSpare1 */
  (pToStruct)->m_uSpare1 =  (uint32)  ((pFromData)[2] >> 4) & 0x0f;

  /* Unpack Member: m_uGain1 */
  (pToStruct)->m_uGain1 =  (uint32)  ((pFromData)[2] ) & 0x0f;

  /* Unpack Member: m_uTemplate0 */
  (pToStruct)->m_uTemplate0 =  (uint32)  (pFromData)[1] ;

  /* Unpack Member: m_uSpare0 */
  (pToStruct)->m_uSpare0 =  (uint32)  ((pFromData)[0] >> 4) & 0x0f;

  /* Unpack Member: m_uGain0 */
  (pToStruct)->m_uGain0 =  (uint32)  ((pFromData)[0] ) & 0x0f;
#endif

}



/* initialize an instance of this zframe */
void
sbZfFabBm9600BwR0WdtEntry_InitInstance(sbZfFabBm9600BwR0WdtEntry_t *pFrame) {

  pFrame->m_uTemplate1 =  (unsigned int)  0;
  pFrame->m_uSpare1 =  (unsigned int)  0;
  pFrame->m_uGain1 =  (unsigned int)  0;
  pFrame->m_uTemplate0 =  (unsigned int)  0;
  pFrame->m_uSpare0 =  (unsigned int)  0;
  pFrame->m_uGain0 =  (unsigned int)  0;

}
