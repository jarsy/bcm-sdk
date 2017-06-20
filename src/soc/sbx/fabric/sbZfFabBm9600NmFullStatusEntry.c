/*
 * $Id: sbZfFabBm9600NmFullStatusEntry.c,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


#include "sbTypes.h"
#include "sbZfFabBm9600NmFullStatusEntry.hx"
#include "sbWrappers.h"
#include <sal/types.h>



/* Pack from struct into array of bytes */
uint32
sbZfFabBm9600NmFullStatusEntry_Pack(sbZfFabBm9600NmFullStatusEntry_t *pFrom,
                                    uint8 *pToData,
                                    uint32 nMaxToDataIndex) {
#ifdef SAND_BIG_ENDIAN_HOST
  int i;
  int size = SB_ZF_FAB_BM9600_NMFULLSTATUSENTRY_SIZE_IN_BYTES;

  if (size % 4) {
    size += (4 - size %4);
  }

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on bigword endian */

  /* Pack Member: m_FullStatus8 */
  (pToData)[35] |= ((pFrom)->m_FullStatus8) & 0xFF;
  (pToData)[34] |= ((pFrom)->m_FullStatus8 >> 8) & 0x03;

  /* Pack Member: m_FullStatus7 */
  (pToData)[31] |= ((pFrom)->m_FullStatus7) & 0xFF;
  (pToData)[30] |= ((pFrom)->m_FullStatus7 >> 8) &0xFF;
  (pToData)[29] |= ((pFrom)->m_FullStatus7 >> 16) &0xFF;
  (pToData)[28] |= ((pFrom)->m_FullStatus7 >> 24) &0xFF;

  /* Pack Member: m_FullStatus6 */
  (pToData)[27] |= ((pFrom)->m_FullStatus6) & 0xFF;
  (pToData)[26] |= ((pFrom)->m_FullStatus6 >> 8) &0xFF;
  (pToData)[25] |= ((pFrom)->m_FullStatus6 >> 16) &0xFF;
  (pToData)[24] |= ((pFrom)->m_FullStatus6 >> 24) &0xFF;

  /* Pack Member: m_FullStatus5 */
  (pToData)[23] |= ((pFrom)->m_FullStatus5) & 0xFF;
  (pToData)[22] |= ((pFrom)->m_FullStatus5 >> 8) &0xFF;
  (pToData)[21] |= ((pFrom)->m_FullStatus5 >> 16) &0xFF;
  (pToData)[20] |= ((pFrom)->m_FullStatus5 >> 24) &0xFF;

  /* Pack Member: m_FullStatus4 */
  (pToData)[19] |= ((pFrom)->m_FullStatus4) & 0xFF;
  (pToData)[18] |= ((pFrom)->m_FullStatus4 >> 8) &0xFF;
  (pToData)[17] |= ((pFrom)->m_FullStatus4 >> 16) &0xFF;
  (pToData)[16] |= ((pFrom)->m_FullStatus4 >> 24) &0xFF;

  /* Pack Member: m_FullStatus3 */
  (pToData)[15] |= ((pFrom)->m_FullStatus3) & 0xFF;
  (pToData)[14] |= ((pFrom)->m_FullStatus3 >> 8) &0xFF;
  (pToData)[13] |= ((pFrom)->m_FullStatus3 >> 16) &0xFF;
  (pToData)[12] |= ((pFrom)->m_FullStatus3 >> 24) &0xFF;

  /* Pack Member: m_FullStatus2 */
  (pToData)[11] |= ((pFrom)->m_FullStatus2) & 0xFF;
  (pToData)[10] |= ((pFrom)->m_FullStatus2 >> 8) &0xFF;
  (pToData)[9] |= ((pFrom)->m_FullStatus2 >> 16) &0xFF;
  (pToData)[8] |= ((pFrom)->m_FullStatus2 >> 24) &0xFF;

  /* Pack Member: m_FullStatus1 */
  (pToData)[7] |= ((pFrom)->m_FullStatus1) & 0xFF;
  (pToData)[6] |= ((pFrom)->m_FullStatus1 >> 8) &0xFF;
  (pToData)[5] |= ((pFrom)->m_FullStatus1 >> 16) &0xFF;
  (pToData)[4] |= ((pFrom)->m_FullStatus1 >> 24) &0xFF;

  /* Pack Member: m_FullStatus0 */
  (pToData)[3] |= ((pFrom)->m_FullStatus0) & 0xFF;
  (pToData)[2] |= ((pFrom)->m_FullStatus0 >> 8) &0xFF;
  (pToData)[1] |= ((pFrom)->m_FullStatus0 >> 16) &0xFF;
  (pToData)[0] |= ((pFrom)->m_FullStatus0 >> 24) &0xFF;
#else
  int i;
  int size = SB_ZF_FAB_BM9600_NMFULLSTATUSENTRY_SIZE_IN_BYTES;

  if (size % 4) {
    size += (4 - size %4);
  }

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on little endian */

  /* Pack Member: m_FullStatus8 */
  (pToData)[32] |= ((pFrom)->m_FullStatus8) & 0xFF;
  (pToData)[33] |= ((pFrom)->m_FullStatus8 >> 8) & 0x03;

  /* Pack Member: m_FullStatus7 */
  (pToData)[28] |= ((pFrom)->m_FullStatus7) & 0xFF;
  (pToData)[29] |= ((pFrom)->m_FullStatus7 >> 8) &0xFF;
  (pToData)[30] |= ((pFrom)->m_FullStatus7 >> 16) &0xFF;
  (pToData)[31] |= ((pFrom)->m_FullStatus7 >> 24) &0xFF;

  /* Pack Member: m_FullStatus6 */
  (pToData)[24] |= ((pFrom)->m_FullStatus6) & 0xFF;
  (pToData)[25] |= ((pFrom)->m_FullStatus6 >> 8) &0xFF;
  (pToData)[26] |= ((pFrom)->m_FullStatus6 >> 16) &0xFF;
  (pToData)[27] |= ((pFrom)->m_FullStatus6 >> 24) &0xFF;

  /* Pack Member: m_FullStatus5 */
  (pToData)[20] |= ((pFrom)->m_FullStatus5) & 0xFF;
  (pToData)[21] |= ((pFrom)->m_FullStatus5 >> 8) &0xFF;
  (pToData)[22] |= ((pFrom)->m_FullStatus5 >> 16) &0xFF;
  (pToData)[23] |= ((pFrom)->m_FullStatus5 >> 24) &0xFF;

  /* Pack Member: m_FullStatus4 */
  (pToData)[16] |= ((pFrom)->m_FullStatus4) & 0xFF;
  (pToData)[17] |= ((pFrom)->m_FullStatus4 >> 8) &0xFF;
  (pToData)[18] |= ((pFrom)->m_FullStatus4 >> 16) &0xFF;
  (pToData)[19] |= ((pFrom)->m_FullStatus4 >> 24) &0xFF;

  /* Pack Member: m_FullStatus3 */
  (pToData)[12] |= ((pFrom)->m_FullStatus3) & 0xFF;
  (pToData)[13] |= ((pFrom)->m_FullStatus3 >> 8) &0xFF;
  (pToData)[14] |= ((pFrom)->m_FullStatus3 >> 16) &0xFF;
  (pToData)[15] |= ((pFrom)->m_FullStatus3 >> 24) &0xFF;

  /* Pack Member: m_FullStatus2 */
  (pToData)[8] |= ((pFrom)->m_FullStatus2) & 0xFF;
  (pToData)[9] |= ((pFrom)->m_FullStatus2 >> 8) &0xFF;
  (pToData)[10] |= ((pFrom)->m_FullStatus2 >> 16) &0xFF;
  (pToData)[11] |= ((pFrom)->m_FullStatus2 >> 24) &0xFF;

  /* Pack Member: m_FullStatus1 */
  (pToData)[4] |= ((pFrom)->m_FullStatus1) & 0xFF;
  (pToData)[5] |= ((pFrom)->m_FullStatus1 >> 8) &0xFF;
  (pToData)[6] |= ((pFrom)->m_FullStatus1 >> 16) &0xFF;
  (pToData)[7] |= ((pFrom)->m_FullStatus1 >> 24) &0xFF;

  /* Pack Member: m_FullStatus0 */
  (pToData)[0] |= ((pFrom)->m_FullStatus0) & 0xFF;
  (pToData)[1] |= ((pFrom)->m_FullStatus0 >> 8) &0xFF;
  (pToData)[2] |= ((pFrom)->m_FullStatus0 >> 16) &0xFF;
  (pToData)[3] |= ((pFrom)->m_FullStatus0 >> 24) &0xFF;
#endif

  return SB_ZF_FAB_BM9600_NMFULLSTATUSENTRY_SIZE_IN_BYTES;
}




/* Unpack from array of bytes into struct */
void
sbZfFabBm9600NmFullStatusEntry_Unpack(sbZfFabBm9600NmFullStatusEntry_t *pToStruct,
                                      uint8 *pFromData,
                                      uint32 nMaxToDataIndex) {
  COMPILER_UINT64 tmp;

  (void) tmp;

#ifdef SAND_BIG_ENDIAN_HOST

  /* Unpack operation based on bigword endian */

  /* Unpack Member: m_FullStatus8 */
  (pToStruct)->m_FullStatus8 =  (uint32)  (pFromData)[35] ;
  (pToStruct)->m_FullStatus8 |=  (uint32)  ((pFromData)[34] & 0x03) << 8;

  /* Unpack Member: m_FullStatus7 */
  (pToStruct)->m_FullStatus7 =  (uint32)  (pFromData)[31] ;
  (pToStruct)->m_FullStatus7 |=  (uint32)  (pFromData)[30] << 8;
  (pToStruct)->m_FullStatus7 |=  (uint32)  (pFromData)[29] << 16;
  (pToStruct)->m_FullStatus7 |=  (uint32)  (pFromData)[28] << 24;

  /* Unpack Member: m_FullStatus6 */
  (pToStruct)->m_FullStatus6 =  (uint32)  (pFromData)[27] ;
  (pToStruct)->m_FullStatus6 |=  (uint32)  (pFromData)[26] << 8;
  (pToStruct)->m_FullStatus6 |=  (uint32)  (pFromData)[25] << 16;
  (pToStruct)->m_FullStatus6 |=  (uint32)  (pFromData)[24] << 24;

  /* Unpack Member: m_FullStatus5 */
  (pToStruct)->m_FullStatus5 =  (uint32)  (pFromData)[23] ;
  (pToStruct)->m_FullStatus5 |=  (uint32)  (pFromData)[22] << 8;
  (pToStruct)->m_FullStatus5 |=  (uint32)  (pFromData)[21] << 16;
  (pToStruct)->m_FullStatus5 |=  (uint32)  (pFromData)[20] << 24;

  /* Unpack Member: m_FullStatus4 */
  (pToStruct)->m_FullStatus4 =  (uint32)  (pFromData)[19] ;
  (pToStruct)->m_FullStatus4 |=  (uint32)  (pFromData)[18] << 8;
  (pToStruct)->m_FullStatus4 |=  (uint32)  (pFromData)[17] << 16;
  (pToStruct)->m_FullStatus4 |=  (uint32)  (pFromData)[16] << 24;

  /* Unpack Member: m_FullStatus3 */
  (pToStruct)->m_FullStatus3 =  (uint32)  (pFromData)[15] ;
  (pToStruct)->m_FullStatus3 |=  (uint32)  (pFromData)[14] << 8;
  (pToStruct)->m_FullStatus3 |=  (uint32)  (pFromData)[13] << 16;
  (pToStruct)->m_FullStatus3 |=  (uint32)  (pFromData)[12] << 24;

  /* Unpack Member: m_FullStatus2 */
  (pToStruct)->m_FullStatus2 =  (uint32)  (pFromData)[11] ;
  (pToStruct)->m_FullStatus2 |=  (uint32)  (pFromData)[10] << 8;
  (pToStruct)->m_FullStatus2 |=  (uint32)  (pFromData)[9] << 16;
  (pToStruct)->m_FullStatus2 |=  (uint32)  (pFromData)[8] << 24;

  /* Unpack Member: m_FullStatus1 */
  (pToStruct)->m_FullStatus1 =  (uint32)  (pFromData)[7] ;
  (pToStruct)->m_FullStatus1 |=  (uint32)  (pFromData)[6] << 8;
  (pToStruct)->m_FullStatus1 |=  (uint32)  (pFromData)[5] << 16;
  (pToStruct)->m_FullStatus1 |=  (uint32)  (pFromData)[4] << 24;

  /* Unpack Member: m_FullStatus0 */
  (pToStruct)->m_FullStatus0 =  (uint32)  (pFromData)[3] ;
  (pToStruct)->m_FullStatus0 |=  (uint32)  (pFromData)[2] << 8;
  (pToStruct)->m_FullStatus0 |=  (uint32)  (pFromData)[1] << 16;
  (pToStruct)->m_FullStatus0 |=  (uint32)  (pFromData)[0] << 24;
#else

  /* Unpack operation based on little endian */

  /* Unpack Member: m_FullStatus8 */
  (pToStruct)->m_FullStatus8 =  (uint32)  (pFromData)[32] ;
  (pToStruct)->m_FullStatus8 |=  (uint32)  ((pFromData)[33] & 0x03) << 8;

  /* Unpack Member: m_FullStatus7 */
  (pToStruct)->m_FullStatus7 =  (uint32)  (pFromData)[28] ;
  (pToStruct)->m_FullStatus7 |=  (uint32)  (pFromData)[29] << 8;
  (pToStruct)->m_FullStatus7 |=  (uint32)  (pFromData)[30] << 16;
  (pToStruct)->m_FullStatus7 |=  (uint32)  (pFromData)[31] << 24;

  /* Unpack Member: m_FullStatus6 */
  (pToStruct)->m_FullStatus6 =  (uint32)  (pFromData)[24] ;
  (pToStruct)->m_FullStatus6 |=  (uint32)  (pFromData)[25] << 8;
  (pToStruct)->m_FullStatus6 |=  (uint32)  (pFromData)[26] << 16;
  (pToStruct)->m_FullStatus6 |=  (uint32)  (pFromData)[27] << 24;

  /* Unpack Member: m_FullStatus5 */
  (pToStruct)->m_FullStatus5 =  (uint32)  (pFromData)[20] ;
  (pToStruct)->m_FullStatus5 |=  (uint32)  (pFromData)[21] << 8;
  (pToStruct)->m_FullStatus5 |=  (uint32)  (pFromData)[22] << 16;
  (pToStruct)->m_FullStatus5 |=  (uint32)  (pFromData)[23] << 24;

  /* Unpack Member: m_FullStatus4 */
  (pToStruct)->m_FullStatus4 =  (uint32)  (pFromData)[16] ;
  (pToStruct)->m_FullStatus4 |=  (uint32)  (pFromData)[17] << 8;
  (pToStruct)->m_FullStatus4 |=  (uint32)  (pFromData)[18] << 16;
  (pToStruct)->m_FullStatus4 |=  (uint32)  (pFromData)[19] << 24;

  /* Unpack Member: m_FullStatus3 */
  (pToStruct)->m_FullStatus3 =  (uint32)  (pFromData)[12] ;
  (pToStruct)->m_FullStatus3 |=  (uint32)  (pFromData)[13] << 8;
  (pToStruct)->m_FullStatus3 |=  (uint32)  (pFromData)[14] << 16;
  (pToStruct)->m_FullStatus3 |=  (uint32)  (pFromData)[15] << 24;

  /* Unpack Member: m_FullStatus2 */
  (pToStruct)->m_FullStatus2 =  (uint32)  (pFromData)[8] ;
  (pToStruct)->m_FullStatus2 |=  (uint32)  (pFromData)[9] << 8;
  (pToStruct)->m_FullStatus2 |=  (uint32)  (pFromData)[10] << 16;
  (pToStruct)->m_FullStatus2 |=  (uint32)  (pFromData)[11] << 24;

  /* Unpack Member: m_FullStatus1 */
  (pToStruct)->m_FullStatus1 =  (uint32)  (pFromData)[4] ;
  (pToStruct)->m_FullStatus1 |=  (uint32)  (pFromData)[5] << 8;
  (pToStruct)->m_FullStatus1 |=  (uint32)  (pFromData)[6] << 16;
  (pToStruct)->m_FullStatus1 |=  (uint32)  (pFromData)[7] << 24;

  /* Unpack Member: m_FullStatus0 */
  (pToStruct)->m_FullStatus0 =  (uint32)  (pFromData)[0] ;
  (pToStruct)->m_FullStatus0 |=  (uint32)  (pFromData)[1] << 8;
  (pToStruct)->m_FullStatus0 |=  (uint32)  (pFromData)[2] << 16;
  (pToStruct)->m_FullStatus0 |=  (uint32)  (pFromData)[3] << 24;
#endif

}



/* initialize an instance of this zframe */
void
sbZfFabBm9600NmFullStatusEntry_InitInstance(sbZfFabBm9600NmFullStatusEntry_t *pFrame) {

  pFrame->m_FullStatus8 =  (unsigned int)  0;
  pFrame->m_FullStatus7 =  (unsigned int)  0;
  pFrame->m_FullStatus6 =  (unsigned int)  0;
  pFrame->m_FullStatus5 =  (unsigned int)  0;
  pFrame->m_FullStatus4 =  (unsigned int)  0;
  pFrame->m_FullStatus3 =  (unsigned int)  0;
  pFrame->m_FullStatus2 =  (unsigned int)  0;
  pFrame->m_FullStatus1 =  (unsigned int)  0;
  pFrame->m_FullStatus0 =  (unsigned int)  0;

}
