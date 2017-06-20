/*
 * $Id: sbZfFabBm9600BwR1Wct2BEntry.c,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


#include "sbTypes.h"
#include "sbZfFabBm9600BwR1Wct2BEntry.hx"
#include "sbWrappers.h"
#include <sal/types.h>



/* Pack from struct into array of bytes */
uint32
sbZfFabBm9600BwR1Wct2BEntry_Pack(sbZfFabBm9600BwR1Wct2BEntry_t *pFrom,
                                 uint8 *pToData,
                                 uint32 nMaxToDataIndex) {
#ifdef SAND_BIG_ENDIAN_HOST
  int i;
  int size = SB_ZF_FAB_BM9600_BWR1WCT2BENTRY_SIZE_IN_BYTES;

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on bigword endian */

  /* Pack Member: m_uTEcnDp2 */
  (pToData)[1] |= ((pFrom)->m_uTEcnDp2) & 0xFF;
  (pToData)[0] |= ((pFrom)->m_uTEcnDp2 >> 8) &0xFF;

  /* Pack Member: m_uScaleDp2 */
  (pToData)[2] |= ((pFrom)->m_uScaleDp2 & 0x0f) <<4;

  /* Pack Member: m_uSlopeDp2 */
  (pToData)[3] |= ((pFrom)->m_uSlopeDp2) & 0xFF;
  (pToData)[2] |= ((pFrom)->m_uSlopeDp2 >> 8) & 0x0f;
#else
  int i;
  int size = SB_ZF_FAB_BM9600_BWR1WCT2BENTRY_SIZE_IN_BYTES;

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on little endian */

  /* Pack Member: m_uTEcnDp2 */
  (pToData)[2] |= ((pFrom)->m_uTEcnDp2) & 0xFF;
  (pToData)[3] |= ((pFrom)->m_uTEcnDp2 >> 8) &0xFF;

  /* Pack Member: m_uScaleDp2 */
  (pToData)[1] |= ((pFrom)->m_uScaleDp2 & 0x0f) <<4;

  /* Pack Member: m_uSlopeDp2 */
  (pToData)[0] |= ((pFrom)->m_uSlopeDp2) & 0xFF;
  (pToData)[1] |= ((pFrom)->m_uSlopeDp2 >> 8) & 0x0f;
#endif

  return SB_ZF_FAB_BM9600_BWR1WCT2BENTRY_SIZE_IN_BYTES;
}




/* Unpack from array of bytes into struct */
void
sbZfFabBm9600BwR1Wct2BEntry_Unpack(sbZfFabBm9600BwR1Wct2BEntry_t *pToStruct,
                                   uint8 *pFromData,
                                   uint32 nMaxToDataIndex) {
  COMPILER_UINT64 tmp;

  (void) tmp;

#ifdef SAND_BIG_ENDIAN_HOST

  /* Unpack operation based on bigword endian */

  /* Unpack Member: m_uTEcnDp2 */
  (pToStruct)->m_uTEcnDp2 =  (uint32)  (pFromData)[1] ;
  (pToStruct)->m_uTEcnDp2 |=  (uint32)  (pFromData)[0] << 8;

  /* Unpack Member: m_uScaleDp2 */
  (pToStruct)->m_uScaleDp2 =  (uint32)  ((pFromData)[2] >> 4) & 0x0f;

  /* Unpack Member: m_uSlopeDp2 */
  (pToStruct)->m_uSlopeDp2 =  (uint32)  (pFromData)[3] ;
  (pToStruct)->m_uSlopeDp2 |=  (uint32)  ((pFromData)[2] & 0x0f) << 8;
#else

  /* Unpack operation based on little endian */

  /* Unpack Member: m_uTEcnDp2 */
  (pToStruct)->m_uTEcnDp2 =  (uint32)  (pFromData)[2] ;
  (pToStruct)->m_uTEcnDp2 |=  (uint32)  (pFromData)[3] << 8;

  /* Unpack Member: m_uScaleDp2 */
  (pToStruct)->m_uScaleDp2 =  (uint32)  ((pFromData)[1] >> 4) & 0x0f;

  /* Unpack Member: m_uSlopeDp2 */
  (pToStruct)->m_uSlopeDp2 =  (uint32)  (pFromData)[0] ;
  (pToStruct)->m_uSlopeDp2 |=  (uint32)  ((pFromData)[1] & 0x0f) << 8;
#endif

}



/* initialize an instance of this zframe */
void
sbZfFabBm9600BwR1Wct2BEntry_InitInstance(sbZfFabBm9600BwR1Wct2BEntry_t *pFrame) {

  pFrame->m_uTEcnDp2 =  (unsigned int)  0;
  pFrame->m_uScaleDp2 =  (unsigned int)  0;
  pFrame->m_uSlopeDp2 =  (unsigned int)  0;

}
