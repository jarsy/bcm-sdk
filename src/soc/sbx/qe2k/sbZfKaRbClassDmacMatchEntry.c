/*
 * $Id: sbZfKaRbClassDmacMatchEntry.c,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


#include "sbTypes.h"
#include "sbZfKaRbClassDmacMatchEntry.hx"
#include "sbWrappers.h"
#include <sal/types.h>



/* Pack from struct into array of bytes */
uint32
sbZfKaRbClassDmacMatchEntry_Pack(sbZfKaRbClassDmacMatchEntry_t *pFrom,
                                 uint8 *pToData,
                                 uint32 nMaxToDataIndex) {
#ifdef SAND_BIG_ENDIAN_HOST
  int i;
  int size = SB_ZF_ZFKARBCLASSDMACMATCHENTRY_SIZE_IN_BYTES;

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on bigword endian */

  /* Pack Member: m_nDmacDataLsb */
  (pToData)[19] |= ((pFrom)->m_nDmacDataLsb) & 0xFF;
  (pToData)[18] |= ((pFrom)->m_nDmacDataLsb >> 8) &0xFF;
  (pToData)[17] |= ((pFrom)->m_nDmacDataLsb >> 16) &0xFF;
  (pToData)[16] |= ((pFrom)->m_nDmacDataLsb >> 24) &0xFF;

  /* Pack Member: m_nDmacDataRsv */
  (pToData)[13] |= ((pFrom)->m_nDmacDataRsv) & 0xFF;
  (pToData)[12] |= ((pFrom)->m_nDmacDataRsv >> 8) &0xFF;

  /* Pack Member: m_nDmacDataMsb */
  (pToData)[15] |= ((pFrom)->m_nDmacDataMsb) & 0xFF;
  (pToData)[14] |= ((pFrom)->m_nDmacDataMsb >> 8) &0xFF;

  /* Pack Member: m_nDmacMaskLsb */
  (pToData)[11] |= ((pFrom)->m_nDmacMaskLsb) & 0xFF;
  (pToData)[10] |= ((pFrom)->m_nDmacMaskLsb >> 8) &0xFF;
  (pToData)[9] |= ((pFrom)->m_nDmacMaskLsb >> 16) &0xFF;
  (pToData)[8] |= ((pFrom)->m_nDmacMaskLsb >> 24) &0xFF;

  /* Pack Member: m_nDmacMaskRsv */
  (pToData)[5] |= ((pFrom)->m_nDmacMaskRsv) & 0xFF;
  (pToData)[4] |= ((pFrom)->m_nDmacMaskRsv >> 8) &0xFF;

  /* Pack Member: m_nDmacMaskMsb */
  (pToData)[7] |= ((pFrom)->m_nDmacMaskMsb) & 0xFF;
  (pToData)[6] |= ((pFrom)->m_nDmacMaskMsb >> 8) &0xFF;

  /* Pack Member: m_nDmacReserve */
  (pToData)[3] |= ((pFrom)->m_nDmacReserve & 0x01) <<7;
  (pToData)[2] |= ((pFrom)->m_nDmacReserve >> 1) &0xFF;
  (pToData)[1] |= ((pFrom)->m_nDmacReserve >> 9) &0xFF;
  (pToData)[0] |= ((pFrom)->m_nDmacReserve >> 17) &0xFF;

  /* Pack Member: m_nDmacEnable */
  (pToData)[3] |= ((pFrom)->m_nDmacEnable & 0x01) <<6;

  /* Pack Member: m_nDmacDp */
  (pToData)[3] |= ((pFrom)->m_nDmacDp & 0x03) <<4;

  /* Pack Member: m_nDmacLsb */
  (pToData)[3] |= ((pFrom)->m_nDmacLsb & 0x0f);
#else
  int i;
  int size = SB_ZF_ZFKARBCLASSDMACMATCHENTRY_SIZE_IN_BYTES;

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on little endian */

  /* Pack Member: m_nDmacDataLsb */
  (pToData)[16] |= ((pFrom)->m_nDmacDataLsb) & 0xFF;
  (pToData)[17] |= ((pFrom)->m_nDmacDataLsb >> 8) &0xFF;
  (pToData)[18] |= ((pFrom)->m_nDmacDataLsb >> 16) &0xFF;
  (pToData)[19] |= ((pFrom)->m_nDmacDataLsb >> 24) &0xFF;

  /* Pack Member: m_nDmacDataRsv */
  (pToData)[14] |= ((pFrom)->m_nDmacDataRsv) & 0xFF;
  (pToData)[15] |= ((pFrom)->m_nDmacDataRsv >> 8) &0xFF;

  /* Pack Member: m_nDmacDataMsb */
  (pToData)[12] |= ((pFrom)->m_nDmacDataMsb) & 0xFF;
  (pToData)[13] |= ((pFrom)->m_nDmacDataMsb >> 8) &0xFF;

  /* Pack Member: m_nDmacMaskLsb */
  (pToData)[8] |= ((pFrom)->m_nDmacMaskLsb) & 0xFF;
  (pToData)[9] |= ((pFrom)->m_nDmacMaskLsb >> 8) &0xFF;
  (pToData)[10] |= ((pFrom)->m_nDmacMaskLsb >> 16) &0xFF;
  (pToData)[11] |= ((pFrom)->m_nDmacMaskLsb >> 24) &0xFF;

  /* Pack Member: m_nDmacMaskRsv */
  (pToData)[6] |= ((pFrom)->m_nDmacMaskRsv) & 0xFF;
  (pToData)[7] |= ((pFrom)->m_nDmacMaskRsv >> 8) &0xFF;

  /* Pack Member: m_nDmacMaskMsb */
  (pToData)[4] |= ((pFrom)->m_nDmacMaskMsb) & 0xFF;
  (pToData)[5] |= ((pFrom)->m_nDmacMaskMsb >> 8) &0xFF;

  /* Pack Member: m_nDmacReserve */
  (pToData)[0] |= ((pFrom)->m_nDmacReserve & 0x01) <<7;
  (pToData)[1] |= ((pFrom)->m_nDmacReserve >> 1) &0xFF;
  (pToData)[2] |= ((pFrom)->m_nDmacReserve >> 9) &0xFF;
  (pToData)[3] |= ((pFrom)->m_nDmacReserve >> 17) &0xFF;

  /* Pack Member: m_nDmacEnable */
  (pToData)[0] |= ((pFrom)->m_nDmacEnable & 0x01) <<6;

  /* Pack Member: m_nDmacDp */
  (pToData)[0] |= ((pFrom)->m_nDmacDp & 0x03) <<4;

  /* Pack Member: m_nDmacLsb */
  (pToData)[0] |= ((pFrom)->m_nDmacLsb & 0x0f);
#endif

  return SB_ZF_ZFKARBCLASSDMACMATCHENTRY_SIZE_IN_BYTES;
}




/* Unpack from array of bytes into struct */
void
sbZfKaRbClassDmacMatchEntry_Unpack(sbZfKaRbClassDmacMatchEntry_t *pToStruct,
                                   uint8 *pFromData,
                                   uint32 nMaxToDataIndex) {
  COMPILER_UINT64 tmp;

  (void) tmp;

#ifdef SAND_BIG_ENDIAN_HOST

  /* Unpack operation based on bigword endian */

  /* Unpack Member: m_nDmacDataLsb */
  (pToStruct)->m_nDmacDataLsb =  (uint32)  (pFromData)[19] ;
  (pToStruct)->m_nDmacDataLsb |=  (uint32)  (pFromData)[18] << 8;
  (pToStruct)->m_nDmacDataLsb |=  (uint32)  (pFromData)[17] << 16;
  (pToStruct)->m_nDmacDataLsb |=  (uint32)  (pFromData)[16] << 24;

  /* Unpack Member: m_nDmacDataRsv */
  (pToStruct)->m_nDmacDataRsv =  (uint32)  (pFromData)[13] ;
  (pToStruct)->m_nDmacDataRsv |=  (uint32)  (pFromData)[12] << 8;

  /* Unpack Member: m_nDmacDataMsb */
  (pToStruct)->m_nDmacDataMsb =  (uint32)  (pFromData)[15] ;
  (pToStruct)->m_nDmacDataMsb |=  (uint32)  (pFromData)[14] << 8;

  /* Unpack Member: m_nDmacMaskLsb */
  (pToStruct)->m_nDmacMaskLsb =  (uint32)  (pFromData)[11] ;
  (pToStruct)->m_nDmacMaskLsb |=  (uint32)  (pFromData)[10] << 8;
  (pToStruct)->m_nDmacMaskLsb |=  (uint32)  (pFromData)[9] << 16;
  (pToStruct)->m_nDmacMaskLsb |=  (uint32)  (pFromData)[8] << 24;

  /* Unpack Member: m_nDmacMaskRsv */
  (pToStruct)->m_nDmacMaskRsv =  (uint32)  (pFromData)[5] ;
  (pToStruct)->m_nDmacMaskRsv |=  (uint32)  (pFromData)[4] << 8;

  /* Unpack Member: m_nDmacMaskMsb */
  (pToStruct)->m_nDmacMaskMsb =  (uint32)  (pFromData)[7] ;
  (pToStruct)->m_nDmacMaskMsb |=  (uint32)  (pFromData)[6] << 8;

  /* Unpack Member: m_nDmacReserve */
  (pToStruct)->m_nDmacReserve =  (uint32)  ((pFromData)[3] >> 7) & 0x01;
  (pToStruct)->m_nDmacReserve |=  (uint32)  (pFromData)[2] << 1;
  (pToStruct)->m_nDmacReserve |=  (uint32)  (pFromData)[1] << 9;
  (pToStruct)->m_nDmacReserve |=  (uint32)  (pFromData)[0] << 17;

  /* Unpack Member: m_nDmacEnable */
  (pToStruct)->m_nDmacEnable =  (uint8)  ((pFromData)[3] >> 6) & 0x01;

  /* Unpack Member: m_nDmacDp */
  (pToStruct)->m_nDmacDp =  (uint32)  ((pFromData)[3] >> 4) & 0x03;

  /* Unpack Member: m_nDmacLsb */
  (pToStruct)->m_nDmacLsb =  (uint32)  ((pFromData)[3] ) & 0x0f;
#else

  /* Unpack operation based on little endian */

  /* Unpack Member: m_nDmacDataLsb */
  (pToStruct)->m_nDmacDataLsb =  (uint32)  (pFromData)[16] ;
  (pToStruct)->m_nDmacDataLsb |=  (uint32)  (pFromData)[17] << 8;
  (pToStruct)->m_nDmacDataLsb |=  (uint32)  (pFromData)[18] << 16;
  (pToStruct)->m_nDmacDataLsb |=  (uint32)  (pFromData)[19] << 24;

  /* Unpack Member: m_nDmacDataRsv */
  (pToStruct)->m_nDmacDataRsv =  (uint32)  (pFromData)[14] ;
  (pToStruct)->m_nDmacDataRsv |=  (uint32)  (pFromData)[15] << 8;

  /* Unpack Member: m_nDmacDataMsb */
  (pToStruct)->m_nDmacDataMsb =  (uint32)  (pFromData)[12] ;
  (pToStruct)->m_nDmacDataMsb |=  (uint32)  (pFromData)[13] << 8;

  /* Unpack Member: m_nDmacMaskLsb */
  (pToStruct)->m_nDmacMaskLsb =  (uint32)  (pFromData)[8] ;
  (pToStruct)->m_nDmacMaskLsb |=  (uint32)  (pFromData)[9] << 8;
  (pToStruct)->m_nDmacMaskLsb |=  (uint32)  (pFromData)[10] << 16;
  (pToStruct)->m_nDmacMaskLsb |=  (uint32)  (pFromData)[11] << 24;

  /* Unpack Member: m_nDmacMaskRsv */
  (pToStruct)->m_nDmacMaskRsv =  (uint32)  (pFromData)[6] ;
  (pToStruct)->m_nDmacMaskRsv |=  (uint32)  (pFromData)[7] << 8;

  /* Unpack Member: m_nDmacMaskMsb */
  (pToStruct)->m_nDmacMaskMsb =  (uint32)  (pFromData)[4] ;
  (pToStruct)->m_nDmacMaskMsb |=  (uint32)  (pFromData)[5] << 8;

  /* Unpack Member: m_nDmacReserve */
  (pToStruct)->m_nDmacReserve =  (uint32)  ((pFromData)[0] >> 7) & 0x01;
  (pToStruct)->m_nDmacReserve |=  (uint32)  (pFromData)[1] << 1;
  (pToStruct)->m_nDmacReserve |=  (uint32)  (pFromData)[2] << 9;
  (pToStruct)->m_nDmacReserve |=  (uint32)  (pFromData)[3] << 17;

  /* Unpack Member: m_nDmacEnable */
  (pToStruct)->m_nDmacEnable =  (uint8)  ((pFromData)[0] >> 6) & 0x01;

  /* Unpack Member: m_nDmacDp */
  (pToStruct)->m_nDmacDp =  (uint32)  ((pFromData)[0] >> 4) & 0x03;

  /* Unpack Member: m_nDmacLsb */
  (pToStruct)->m_nDmacLsb =  (uint32)  ((pFromData)[0] ) & 0x0f;
#endif

}



/* initialize an instance of this zframe */
void
sbZfKaRbClassDmacMatchEntry_InitInstance(sbZfKaRbClassDmacMatchEntry_t *pFrame) {

  pFrame->m_nDmacDataLsb =  (unsigned int)  0;
  pFrame->m_nDmacDataRsv =  (unsigned int)  0;
  pFrame->m_nDmacDataMsb =  (unsigned int)  0;
  pFrame->m_nDmacMaskLsb =  (unsigned int)  0;
  pFrame->m_nDmacMaskRsv =  (unsigned int)  0;
  pFrame->m_nDmacMaskMsb =  (unsigned int)  0;
  pFrame->m_nDmacReserve =  (unsigned int)  0;
  pFrame->m_nDmacEnable =  (unsigned int)  0;
  pFrame->m_nDmacDp =  (unsigned int)  0;
  pFrame->m_nDmacLsb =  (unsigned int)  0;

}
