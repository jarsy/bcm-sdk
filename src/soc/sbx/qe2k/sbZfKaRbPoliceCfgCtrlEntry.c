/*
 * $Id: sbZfKaRbPoliceCfgCtrlEntry.c,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


#include "sbTypes.h"
#include "sbZfKaRbPoliceCfgCtrlEntry.hx"
#include "sbWrappers.h"
#include <sal/types.h>



/* Pack from struct into array of bytes */
uint32
sbZfKaRbPoliceCfgCtrlEntry_Pack(sbZfKaRbPoliceCfgCtrlEntry_t *pFrom,
                                uint8 *pToData,
                                uint32 nMaxToDataIndex) {
#ifdef SAND_BIG_ENDIAN_HOST
  int i;
  int size = SB_ZF_ZFKARBPOLCFGCTRLENTRY_SIZE_IN_BYTES;

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on bigword endian */

  /* Pack Member: m_nReserved */
  (pToData)[0] |= ((pFrom)->m_nReserved & 0x1f) <<3;

  /* Pack Member: m_nEnable */
  (pToData)[0] |= ((pFrom)->m_nEnable & 0x01) <<2;

  /* Pack Member: m_nNotBlind */
  (pToData)[0] |= ((pFrom)->m_nNotBlind & 0x01) <<1;

  /* Pack Member: m_nDropOnRed */
  (pToData)[0] |= ((pFrom)->m_nDropOnRed & 0x01);

  /* Pack Member: m_nEnableMon */
  (pToData)[1] |= ((pFrom)->m_nEnableMon & 0x01) <<7;

  /* Pack Member: m_nMonCntId */
  (pToData)[1] |= ((pFrom)->m_nMonCntId & 0x07) <<4;

  /* Pack Member: m_nIncRate */
  (pToData)[3] |= ((pFrom)->m_nIncRate) & 0xFF;
  (pToData)[2] |= ((pFrom)->m_nIncRate >> 8) &0xFF;
  (pToData)[1] |= ((pFrom)->m_nIncRate >> 16) & 0x0f;
#else
  int i;
  int size = SB_ZF_ZFKARBPOLCFGCTRLENTRY_SIZE_IN_BYTES;

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on little endian */

  /* Pack Member: m_nReserved */
  (pToData)[3] |= ((pFrom)->m_nReserved & 0x1f) <<3;

  /* Pack Member: m_nEnable */
  (pToData)[3] |= ((pFrom)->m_nEnable & 0x01) <<2;

  /* Pack Member: m_nNotBlind */
  (pToData)[3] |= ((pFrom)->m_nNotBlind & 0x01) <<1;

  /* Pack Member: m_nDropOnRed */
  (pToData)[3] |= ((pFrom)->m_nDropOnRed & 0x01);

  /* Pack Member: m_nEnableMon */
  (pToData)[2] |= ((pFrom)->m_nEnableMon & 0x01) <<7;

  /* Pack Member: m_nMonCntId */
  (pToData)[2] |= ((pFrom)->m_nMonCntId & 0x07) <<4;

  /* Pack Member: m_nIncRate */
  (pToData)[0] |= ((pFrom)->m_nIncRate) & 0xFF;
  (pToData)[1] |= ((pFrom)->m_nIncRate >> 8) &0xFF;
  (pToData)[2] |= ((pFrom)->m_nIncRate >> 16) & 0x0f;
#endif

  return SB_ZF_ZFKARBPOLCFGCTRLENTRY_SIZE_IN_BYTES;
}




/* Unpack from array of bytes into struct */
void
sbZfKaRbPoliceCfgCtrlEntry_Unpack(sbZfKaRbPoliceCfgCtrlEntry_t *pToStruct,
                                  uint8 *pFromData,
                                  uint32 nMaxToDataIndex) {
  COMPILER_UINT64 tmp;

  (void) tmp;

#ifdef SAND_BIG_ENDIAN_HOST

  /* Unpack operation based on bigword endian */

  /* Unpack Member: m_nReserved */
  (pToStruct)->m_nReserved =  (uint32)  ((pFromData)[0] >> 3) & 0x1f;

  /* Unpack Member: m_nEnable */
  (pToStruct)->m_nEnable =  (uint8)  ((pFromData)[0] >> 2) & 0x01;

  /* Unpack Member: m_nNotBlind */
  (pToStruct)->m_nNotBlind =  (uint8)  ((pFromData)[0] >> 1) & 0x01;

  /* Unpack Member: m_nDropOnRed */
  (pToStruct)->m_nDropOnRed =  (uint8)  ((pFromData)[0] ) & 0x01;

  /* Unpack Member: m_nEnableMon */
  (pToStruct)->m_nEnableMon =  (uint8)  ((pFromData)[1] >> 7) & 0x01;

  /* Unpack Member: m_nMonCntId */
  (pToStruct)->m_nMonCntId =  (uint32)  ((pFromData)[1] >> 4) & 0x07;

  /* Unpack Member: m_nIncRate */
  (pToStruct)->m_nIncRate =  (uint32)  (pFromData)[3] ;
  (pToStruct)->m_nIncRate |=  (uint32)  (pFromData)[2] << 8;
  (pToStruct)->m_nIncRate |=  (uint32)  ((pFromData)[1] & 0x0f) << 16;
#else

  /* Unpack operation based on little endian */

  /* Unpack Member: m_nReserved */
  (pToStruct)->m_nReserved =  (uint32)  ((pFromData)[3] >> 3) & 0x1f;

  /* Unpack Member: m_nEnable */
  (pToStruct)->m_nEnable =  (uint8)  ((pFromData)[3] >> 2) & 0x01;

  /* Unpack Member: m_nNotBlind */
  (pToStruct)->m_nNotBlind =  (uint8)  ((pFromData)[3] >> 1) & 0x01;

  /* Unpack Member: m_nDropOnRed */
  (pToStruct)->m_nDropOnRed =  (uint8)  ((pFromData)[3] ) & 0x01;

  /* Unpack Member: m_nEnableMon */
  (pToStruct)->m_nEnableMon =  (uint8)  ((pFromData)[2] >> 7) & 0x01;

  /* Unpack Member: m_nMonCntId */
  (pToStruct)->m_nMonCntId =  (uint32)  ((pFromData)[2] >> 4) & 0x07;

  /* Unpack Member: m_nIncRate */
  (pToStruct)->m_nIncRate =  (uint32)  (pFromData)[0] ;
  (pToStruct)->m_nIncRate |=  (uint32)  (pFromData)[1] << 8;
  (pToStruct)->m_nIncRate |=  (uint32)  ((pFromData)[2] & 0x0f) << 16;
#endif

}



/* initialize an instance of this zframe */
void
sbZfKaRbPoliceCfgCtrlEntry_InitInstance(sbZfKaRbPoliceCfgCtrlEntry_t *pFrame) {

  pFrame->m_nReserved =  (unsigned int)  0;
  pFrame->m_nEnable =  (unsigned int)  0;
  pFrame->m_nNotBlind =  (unsigned int)  0;
  pFrame->m_nDropOnRed =  (unsigned int)  0;
  pFrame->m_nEnableMon =  (unsigned int)  0;
  pFrame->m_nMonCntId =  (unsigned int)  0;
  pFrame->m_nIncRate =  (unsigned int)  0;

}
