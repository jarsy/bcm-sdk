/*
 * $Id: sbZfSbQe2000ElibVRT.c,v 1.4 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


#include "sbTypesGlue.h"
#include "sbZfSbQe2000ElibVRT.hx"
#include "sbWrappers.h"
#include <sal/types.h>



/* Pack from struct into array of bytes */
uint32
sbZfSbQe2000ElibVRT_Pack(sbZfSbQe2000ElibVRT_t *pFrom,
                         uint8 *pToData,
                         uint32 nMaxToDataIndex) {
  int i;
  int size = SB_ZF_SB_QE2000_ELIB_VRT_ENTRY_SIZE_IN_BYTES;

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on little endian */

  /* Pack Member: m_nVid0 */
  (pToData)[2] |= ((pFrom)->m_nVid0 & 0x0f) <<4;
  (pToData)[3] |= ((pFrom)->m_nVid0 >> 4) &0xFF;

  /* Pack Member: m_nVid1 */
  (pToData)[10] |= ((pFrom)->m_nVid1 & 0x0f) <<4;
  (pToData)[11] |= ((pFrom)->m_nVid1 >> 4) &0xFF;

  /* Pack Member: m_nOffset */
  (pToData)[9] |= ((pFrom)->m_nOffset & 0x01) <<7;
  (pToData)[10] |= ((pFrom)->m_nOffset >> 1) & 0x0f;

  /* Pack Member: m_nPortState_0 */
  (pToData)[4] |= ((pFrom)->m_nPortState_0 & 0x03);

  /* Pack Member: m_nPortState_1 */
  (pToData)[4] |= ((pFrom)->m_nPortState_1 & 0x03) <<2;

  /* Pack Member: m_nPortState_2 */
  (pToData)[4] |= ((pFrom)->m_nPortState_2 & 0x03) <<4;

  /* Pack Member: m_nPortState_3 */
  (pToData)[4] |= ((pFrom)->m_nPortState_3 & 0x03) <<6;

  /* Pack Member: m_nPortState_4 */
  (pToData)[5] |= ((pFrom)->m_nPortState_4 & 0x03);

  /* Pack Member: m_nPortState_5 */
  (pToData)[5] |= ((pFrom)->m_nPortState_5 & 0x03) <<2;

  /* Pack Member: m_nPortState_6 */
  (pToData)[5] |= ((pFrom)->m_nPortState_6 & 0x03) <<4;

  /* Pack Member: m_nPortState_7 */
  (pToData)[5] |= ((pFrom)->m_nPortState_7 & 0x03) <<6;

  /* Pack Member: m_nPortState_8 */
  (pToData)[6] |= ((pFrom)->m_nPortState_8 & 0x03);

  /* Pack Member: m_nPortState_9 */
  (pToData)[6] |= ((pFrom)->m_nPortState_9 & 0x03) <<2;

  /* Pack Member: m_nPortState_10 */
  (pToData)[6] |= ((pFrom)->m_nPortState_10 & 0x03) <<4;

  /* Pack Member: m_nPortState_11 */
  (pToData)[6] |= ((pFrom)->m_nPortState_11 & 0x03) <<6;

  /* Pack Member: m_nPortState_12 */
  (pToData)[7] |= ((pFrom)->m_nPortState_12 & 0x03);

  /* Pack Member: m_nPortState_13 */
  (pToData)[7] |= ((pFrom)->m_nPortState_13 & 0x03) <<2;

  /* Pack Member: m_nPortState_14 */
  (pToData)[7] |= ((pFrom)->m_nPortState_14 & 0x03) <<4;

  /* Pack Member: m_nPortState_15 */
  (pToData)[7] |= ((pFrom)->m_nPortState_15 & 0x03) <<6;

  /* Pack Member: m_nPortState_16 */
  (pToData)[0] |= ((pFrom)->m_nPortState_16 & 0x03);

  /* Pack Member: m_nPortState_17 */
  (pToData)[0] |= ((pFrom)->m_nPortState_17 & 0x03) <<2;

  /* Pack Member: m_nPortState_18 */
  (pToData)[0] |= ((pFrom)->m_nPortState_18 & 0x03) <<4;

  /* Pack Member: m_nPortState_19 */
  (pToData)[0] |= ((pFrom)->m_nPortState_19 & 0x03) <<6;

  /* Pack Member: m_nPortState_20 */
  (pToData)[1] |= ((pFrom)->m_nPortState_20 & 0x03);

  /* Pack Member: m_nPortState_21 */
  (pToData)[1] |= ((pFrom)->m_nPortState_21 & 0x03) <<2;

  /* Pack Member: m_nPortState_22 */
  (pToData)[1] |= ((pFrom)->m_nPortState_22 & 0x03) <<4;

  /* Pack Member: m_nPortState_23 */
  (pToData)[1] |= ((pFrom)->m_nPortState_23 & 0x03) <<6;

  /* Pack Member: m_nPortState_24 */
  (pToData)[12] |= ((pFrom)->m_nPortState_24 & 0x03);

  /* Pack Member: m_nPortState_25 */
  (pToData)[12] |= ((pFrom)->m_nPortState_25 & 0x03) <<2;

  /* Pack Member: m_nPortState_26 */
  (pToData)[12] |= ((pFrom)->m_nPortState_26 & 0x03) <<4;

  /* Pack Member: m_nPortState_27 */
  (pToData)[12] |= ((pFrom)->m_nPortState_27 & 0x03) <<6;

  /* Pack Member: m_nPortState_28 */
  (pToData)[13] |= ((pFrom)->m_nPortState_28 & 0x03);

  /* Pack Member: m_nPortState_29 */
  (pToData)[13] |= ((pFrom)->m_nPortState_29 & 0x03) <<2;

  /* Pack Member: m_nPortState_30 */
  (pToData)[13] |= ((pFrom)->m_nPortState_30 & 0x03) <<4;

  /* Pack Member: m_nPortState_31 */
  (pToData)[13] |= ((pFrom)->m_nPortState_31 & 0x03) <<6;

  /* Pack Member: m_nPortState_32 */
  (pToData)[14] |= ((pFrom)->m_nPortState_32 & 0x03);

  /* Pack Member: m_nPortState_33 */
  (pToData)[14] |= ((pFrom)->m_nPortState_33 & 0x03) <<2;

  /* Pack Member: m_nPortState_34 */
  (pToData)[14] |= ((pFrom)->m_nPortState_34 & 0x03) <<4;

  /* Pack Member: m_nPortState_35 */
  (pToData)[14] |= ((pFrom)->m_nPortState_35 & 0x03) <<6;

  /* Pack Member: m_nPortState_36 */
  (pToData)[15] |= ((pFrom)->m_nPortState_36 & 0x03);

  /* Pack Member: m_nPortState_37 */
  (pToData)[15] |= ((pFrom)->m_nPortState_37 & 0x03) <<2;

  /* Pack Member: m_nPortState_38 */
  (pToData)[15] |= ((pFrom)->m_nPortState_38 & 0x03) <<4;

  /* Pack Member: m_nPortState_39 */
  (pToData)[15] |= ((pFrom)->m_nPortState_39 & 0x03) <<6;

  /* Pack Member: m_nPortState_40 */
  (pToData)[8] |= ((pFrom)->m_nPortState_40 & 0x03);

  /* Pack Member: m_nPortState_41 */
  (pToData)[8] |= ((pFrom)->m_nPortState_41 & 0x03) <<2;

  /* Pack Member: m_nPortState_42 */
  (pToData)[8] |= ((pFrom)->m_nPortState_42 & 0x03) <<4;

  /* Pack Member: m_nPortState_43 */
  (pToData)[8] |= ((pFrom)->m_nPortState_43 & 0x03) <<6;

  /* Pack Member: m_nPortState_44 */
  (pToData)[9] |= ((pFrom)->m_nPortState_44 & 0x03);

  /* Pack Member: m_nPortState_45 */
  (pToData)[9] |= ((pFrom)->m_nPortState_45 & 0x03) <<2;

  /* Pack Member: m_nPortState_46 */
  (pToData)[9] |= ((pFrom)->m_nPortState_46 & 0x03) <<4;

  /* Pack Member: m_nPortState_47 */
  (pToData)[9] |= ((pFrom)->m_nPortState_47 & 0x03) <<6;

  /* Pack Member: m_nPortState_48 */
  (pToData)[2] |= ((pFrom)->m_nPortState_48 & 0x03);

  /* Pack Member: m_nPortState_49 */
  (pToData)[2] |= ((pFrom)->m_nPortState_49 & 0x03) <<2;

  return SB_ZF_SB_QE2000_ELIB_VRT_ENTRY_SIZE_IN_BYTES;
}




/* Unpack from array of bytes into struct */
void
sbZfSbQe2000ElibVRT_Unpack(sbZfSbQe2000ElibVRT_t *pToStruct,
                           uint8 *pFromData,
                           uint32 nMaxToDataIndex) {
  COMPILER_UINT64 tmp;

  (void) tmp;


  /* Unpack operation based on little endian */

  /* Unpack Member: m_nVid0 */
  (pToStruct)->m_nVid0 =  (uint32)  ((pFromData)[2] >> 4) & 0x0f;
  (pToStruct)->m_nVid0 |=  (uint32)  (pFromData)[3] << 4;

  /* Unpack Member: m_nVid1 */
  (pToStruct)->m_nVid1 =  (uint32)  ((pFromData)[10] >> 4) & 0x0f;
  (pToStruct)->m_nVid1 |=  (uint32)  (pFromData)[11] << 4;

  /* Unpack Member: m_nOffset */
  (pToStruct)->m_nOffset =  (uint32)  ((pFromData)[9] >> 7) & 0x01;
  (pToStruct)->m_nOffset |=  (uint32)  ((pFromData)[10] & 0x0f) << 1;

  /* Unpack Member: m_nPortState_0 */
  (pToStruct)->m_nPortState_0 =  (uint32)  ((pFromData)[4] ) & 0x03;

  /* Unpack Member: m_nPortState_1 */
  (pToStruct)->m_nPortState_1 =  (uint32)  ((pFromData)[4] >> 2) & 0x03;

  /* Unpack Member: m_nPortState_2 */
  (pToStruct)->m_nPortState_2 =  (uint32)  ((pFromData)[4] >> 4) & 0x03;

  /* Unpack Member: m_nPortState_3 */
  (pToStruct)->m_nPortState_3 =  (uint32)  ((pFromData)[4] >> 6) & 0x03;

  /* Unpack Member: m_nPortState_4 */
  (pToStruct)->m_nPortState_4 =  (uint32)  ((pFromData)[5] ) & 0x03;

  /* Unpack Member: m_nPortState_5 */
  (pToStruct)->m_nPortState_5 =  (uint32)  ((pFromData)[5] >> 2) & 0x03;

  /* Unpack Member: m_nPortState_6 */
  (pToStruct)->m_nPortState_6 =  (uint32)  ((pFromData)[5] >> 4) & 0x03;

  /* Unpack Member: m_nPortState_7 */
  (pToStruct)->m_nPortState_7 =  (uint32)  ((pFromData)[5] >> 6) & 0x03;

  /* Unpack Member: m_nPortState_8 */
  (pToStruct)->m_nPortState_8 =  (uint32)  ((pFromData)[6] ) & 0x03;

  /* Unpack Member: m_nPortState_9 */
  (pToStruct)->m_nPortState_9 =  (uint32)  ((pFromData)[6] >> 2) & 0x03;

  /* Unpack Member: m_nPortState_10 */
  (pToStruct)->m_nPortState_10 =  (uint32)  ((pFromData)[6] >> 4) & 0x03;

  /* Unpack Member: m_nPortState_11 */
  (pToStruct)->m_nPortState_11 =  (uint32)  ((pFromData)[6] >> 6) & 0x03;

  /* Unpack Member: m_nPortState_12 */
  (pToStruct)->m_nPortState_12 =  (uint32)  ((pFromData)[7] ) & 0x03;

  /* Unpack Member: m_nPortState_13 */
  (pToStruct)->m_nPortState_13 =  (uint32)  ((pFromData)[7] >> 2) & 0x03;

  /* Unpack Member: m_nPortState_14 */
  (pToStruct)->m_nPortState_14 =  (uint32)  ((pFromData)[7] >> 4) & 0x03;

  /* Unpack Member: m_nPortState_15 */
  (pToStruct)->m_nPortState_15 =  (uint32)  ((pFromData)[7] >> 6) & 0x03;

  /* Unpack Member: m_nPortState_16 */
  (pToStruct)->m_nPortState_16 =  (uint32)  ((pFromData)[0] ) & 0x03;

  /* Unpack Member: m_nPortState_17 */
  (pToStruct)->m_nPortState_17 =  (uint32)  ((pFromData)[0] >> 2) & 0x03;

  /* Unpack Member: m_nPortState_18 */
  (pToStruct)->m_nPortState_18 =  (uint32)  ((pFromData)[0] >> 4) & 0x03;

  /* Unpack Member: m_nPortState_19 */
  (pToStruct)->m_nPortState_19 =  (uint32)  ((pFromData)[0] >> 6) & 0x03;

  /* Unpack Member: m_nPortState_20 */
  (pToStruct)->m_nPortState_20 =  (uint32)  ((pFromData)[1] ) & 0x03;

  /* Unpack Member: m_nPortState_21 */
  (pToStruct)->m_nPortState_21 =  (uint32)  ((pFromData)[1] >> 2) & 0x03;

  /* Unpack Member: m_nPortState_22 */
  (pToStruct)->m_nPortState_22 =  (uint32)  ((pFromData)[1] >> 4) & 0x03;

  /* Unpack Member: m_nPortState_23 */
  (pToStruct)->m_nPortState_23 =  (uint32)  ((pFromData)[1] >> 6) & 0x03;

  /* Unpack Member: m_nPortState_24 */
  (pToStruct)->m_nPortState_24 =  (uint32)  ((pFromData)[12] ) & 0x03;

  /* Unpack Member: m_nPortState_25 */
  (pToStruct)->m_nPortState_25 =  (uint32)  ((pFromData)[12] >> 2) & 0x03;

  /* Unpack Member: m_nPortState_26 */
  (pToStruct)->m_nPortState_26 =  (uint32)  ((pFromData)[12] >> 4) & 0x03;

  /* Unpack Member: m_nPortState_27 */
  (pToStruct)->m_nPortState_27 =  (uint32)  ((pFromData)[12] >> 6) & 0x03;

  /* Unpack Member: m_nPortState_28 */
  (pToStruct)->m_nPortState_28 =  (uint32)  ((pFromData)[13] ) & 0x03;

  /* Unpack Member: m_nPortState_29 */
  (pToStruct)->m_nPortState_29 =  (uint32)  ((pFromData)[13] >> 2) & 0x03;

  /* Unpack Member: m_nPortState_30 */
  (pToStruct)->m_nPortState_30 =  (uint32)  ((pFromData)[13] >> 4) & 0x03;

  /* Unpack Member: m_nPortState_31 */
  (pToStruct)->m_nPortState_31 =  (uint32)  ((pFromData)[13] >> 6) & 0x03;

  /* Unpack Member: m_nPortState_32 */
  (pToStruct)->m_nPortState_32 =  (uint32)  ((pFromData)[14] ) & 0x03;

  /* Unpack Member: m_nPortState_33 */
  (pToStruct)->m_nPortState_33 =  (uint32)  ((pFromData)[14] >> 2) & 0x03;

  /* Unpack Member: m_nPortState_34 */
  (pToStruct)->m_nPortState_34 =  (uint32)  ((pFromData)[14] >> 4) & 0x03;

  /* Unpack Member: m_nPortState_35 */
  (pToStruct)->m_nPortState_35 =  (uint32)  ((pFromData)[14] >> 6) & 0x03;

  /* Unpack Member: m_nPortState_36 */
  (pToStruct)->m_nPortState_36 =  (uint32)  ((pFromData)[15] ) & 0x03;

  /* Unpack Member: m_nPortState_37 */
  (pToStruct)->m_nPortState_37 =  (uint32)  ((pFromData)[15] >> 2) & 0x03;

  /* Unpack Member: m_nPortState_38 */
  (pToStruct)->m_nPortState_38 =  (uint32)  ((pFromData)[15] >> 4) & 0x03;

  /* Unpack Member: m_nPortState_39 */
  (pToStruct)->m_nPortState_39 =  (uint32)  ((pFromData)[15] >> 6) & 0x03;

  /* Unpack Member: m_nPortState_40 */
  (pToStruct)->m_nPortState_40 =  (uint32)  ((pFromData)[8] ) & 0x03;

  /* Unpack Member: m_nPortState_41 */
  (pToStruct)->m_nPortState_41 =  (uint32)  ((pFromData)[8] >> 2) & 0x03;

  /* Unpack Member: m_nPortState_42 */
  (pToStruct)->m_nPortState_42 =  (uint32)  ((pFromData)[8] >> 4) & 0x03;

  /* Unpack Member: m_nPortState_43 */
  (pToStruct)->m_nPortState_43 =  (uint32)  ((pFromData)[8] >> 6) & 0x03;

  /* Unpack Member: m_nPortState_44 */
  (pToStruct)->m_nPortState_44 =  (uint32)  ((pFromData)[9] ) & 0x03;

  /* Unpack Member: m_nPortState_45 */
  (pToStruct)->m_nPortState_45 =  (uint32)  ((pFromData)[9] >> 2) & 0x03;

  /* Unpack Member: m_nPortState_46 */
  (pToStruct)->m_nPortState_46 =  (uint32)  ((pFromData)[9] >> 4) & 0x03;

  /* Unpack Member: m_nPortState_47 */
  (pToStruct)->m_nPortState_47 =  (uint32)  ((pFromData)[9] >> 6) & 0x03;

  /* Unpack Member: m_nPortState_48 */
  (pToStruct)->m_nPortState_48 =  (uint32)  ((pFromData)[2] ) & 0x03;

  /* Unpack Member: m_nPortState_49 */
  (pToStruct)->m_nPortState_49 =  (uint32)  ((pFromData)[2] >> 2) & 0x03;

}



/* initialize an instance of this zframe */
void
sbZfSbQe2000ElibVRT_InitInstance(sbZfSbQe2000ElibVRT_t *pFrame) {

  pFrame->m_nVid0 =  (unsigned int)  0;
  pFrame->m_nVid1 =  (unsigned int)  0;
  pFrame->m_nOffset =  (unsigned int)  0;
  pFrame->m_nPortState_0 =  (unsigned int)  0;
  pFrame->m_nPortState_1 =  (unsigned int)  0;
  pFrame->m_nPortState_2 =  (unsigned int)  0;
  pFrame->m_nPortState_3 =  (unsigned int)  0;
  pFrame->m_nPortState_4 =  (unsigned int)  0;
  pFrame->m_nPortState_5 =  (unsigned int)  0;
  pFrame->m_nPortState_6 =  (unsigned int)  0;
  pFrame->m_nPortState_7 =  (unsigned int)  0;
  pFrame->m_nPortState_8 =  (unsigned int)  0;
  pFrame->m_nPortState_9 =  (unsigned int)  0;
  pFrame->m_nPortState_10 =  (unsigned int)  0;
  pFrame->m_nPortState_11 =  (unsigned int)  0;
  pFrame->m_nPortState_12 =  (unsigned int)  0;
  pFrame->m_nPortState_13 =  (unsigned int)  0;
  pFrame->m_nPortState_14 =  (unsigned int)  0;
  pFrame->m_nPortState_15 =  (unsigned int)  0;
  pFrame->m_nPortState_16 =  (unsigned int)  0;
  pFrame->m_nPortState_17 =  (unsigned int)  0;
  pFrame->m_nPortState_18 =  (unsigned int)  0;
  pFrame->m_nPortState_19 =  (unsigned int)  0;
  pFrame->m_nPortState_20 =  (unsigned int)  0;
  pFrame->m_nPortState_21 =  (unsigned int)  0;
  pFrame->m_nPortState_22 =  (unsigned int)  0;
  pFrame->m_nPortState_23 =  (unsigned int)  0;
  pFrame->m_nPortState_24 =  (unsigned int)  0;
  pFrame->m_nPortState_25 =  (unsigned int)  0;
  pFrame->m_nPortState_26 =  (unsigned int)  0;
  pFrame->m_nPortState_27 =  (unsigned int)  0;
  pFrame->m_nPortState_28 =  (unsigned int)  0;
  pFrame->m_nPortState_29 =  (unsigned int)  0;
  pFrame->m_nPortState_30 =  (unsigned int)  0;
  pFrame->m_nPortState_31 =  (unsigned int)  0;
  pFrame->m_nPortState_32 =  (unsigned int)  0;
  pFrame->m_nPortState_33 =  (unsigned int)  0;
  pFrame->m_nPortState_34 =  (unsigned int)  0;
  pFrame->m_nPortState_35 =  (unsigned int)  0;
  pFrame->m_nPortState_36 =  (unsigned int)  0;
  pFrame->m_nPortState_37 =  (unsigned int)  0;
  pFrame->m_nPortState_38 =  (unsigned int)  0;
  pFrame->m_nPortState_39 =  (unsigned int)  0;
  pFrame->m_nPortState_40 =  (unsigned int)  0;
  pFrame->m_nPortState_41 =  (unsigned int)  0;
  pFrame->m_nPortState_42 =  (unsigned int)  0;
  pFrame->m_nPortState_43 =  (unsigned int)  0;
  pFrame->m_nPortState_44 =  (unsigned int)  0;
  pFrame->m_nPortState_45 =  (unsigned int)  0;
  pFrame->m_nPortState_46 =  (unsigned int)  0;
  pFrame->m_nPortState_47 =  (unsigned int)  0;
  pFrame->m_nPortState_48 =  (unsigned int)  0;
  pFrame->m_nPortState_49 =  (unsigned int)  0;

}




UINT sbZfSbQe2000ElibVRTGetPortState(sbZfSbQe2000ElibVRT_t *zfVRT, UINT nPort)
{
    switch (nPort)
    {
        case 0: return zfVRT->m_nPortState_0;
        case 1: return zfVRT->m_nPortState_1;
        case 2: return zfVRT->m_nPortState_2;
        case 3: return zfVRT->m_nPortState_3;
        case 4: return zfVRT->m_nPortState_4;
        case 5: return zfVRT->m_nPortState_5;
        case 6: return zfVRT->m_nPortState_6;
        case 7: return zfVRT->m_nPortState_7;
        case 8: return zfVRT->m_nPortState_8;
        case 9: return zfVRT->m_nPortState_9;
        case 10: return zfVRT->m_nPortState_10;
        case 11: return zfVRT->m_nPortState_11;
        case 12: return zfVRT->m_nPortState_12;
        case 13: return zfVRT->m_nPortState_13;
        case 14: return zfVRT->m_nPortState_14;
        case 15: return zfVRT->m_nPortState_15;
        case 16: return zfVRT->m_nPortState_16;
        case 17: return zfVRT->m_nPortState_17;
        case 18: return zfVRT->m_nPortState_18;
        case 19: return zfVRT->m_nPortState_19;
        case 20: return zfVRT->m_nPortState_20;
        case 21: return zfVRT->m_nPortState_21;
        case 22: return zfVRT->m_nPortState_22;
        case 23: return zfVRT->m_nPortState_23;
        case 24: return zfVRT->m_nPortState_24;
        case 25: return zfVRT->m_nPortState_25;
        case 26: return zfVRT->m_nPortState_26;
        case 27: return zfVRT->m_nPortState_27;
        case 28: return zfVRT->m_nPortState_28;
        case 29: return zfVRT->m_nPortState_29;
        case 30: return zfVRT->m_nPortState_30;
        case 31: return zfVRT->m_nPortState_31;
        case 32: return zfVRT->m_nPortState_32;
        case 33: return zfVRT->m_nPortState_33;
        case 34: return zfVRT->m_nPortState_34;
        case 35: return zfVRT->m_nPortState_35;
        case 36: return zfVRT->m_nPortState_36;
        case 37: return zfVRT->m_nPortState_37;
        case 38: return zfVRT->m_nPortState_38;
        case 39: return zfVRT->m_nPortState_39;
        case 40: return zfVRT->m_nPortState_40;
        case 41: return zfVRT->m_nPortState_41;
        case 42: return zfVRT->m_nPortState_42;
        case 43: return zfVRT->m_nPortState_43;
        case 44: return zfVRT->m_nPortState_44;
        case 45: return zfVRT->m_nPortState_45;
        case 46: return zfVRT->m_nPortState_46;
        case 47: return zfVRT->m_nPortState_47;
        case 48: return zfVRT->m_nPortState_48;
        case 49: return zfVRT->m_nPortState_49;
        default: SB_ASSERT(0);
    }
    return 0;
}


void sbZfSbQe2000ElibVRTSetPortState(sbZfSbQe2000ElibVRT_t *zfVRT, UINT nPort, UINT nState)
{
    switch (nPort)
    {
        case 0: zfVRT->m_nPortState_0 = nState; break;
        case 1: zfVRT->m_nPortState_1 = nState; break;
        case 2: zfVRT->m_nPortState_2 = nState; break;
        case 3: zfVRT->m_nPortState_3 = nState; break;
        case 4: zfVRT->m_nPortState_4 = nState; break;
        case 5: zfVRT->m_nPortState_5 = nState; break;
        case 6: zfVRT->m_nPortState_6 = nState; break;
        case 7: zfVRT->m_nPortState_7 = nState; break;
        case 8: zfVRT->m_nPortState_8 = nState; break;
        case 9: zfVRT->m_nPortState_9 = nState; break;
        case 10: zfVRT->m_nPortState_10 = nState; break;
        case 11: zfVRT->m_nPortState_11 = nState; break;
        case 12: zfVRT->m_nPortState_12 = nState; break;
        case 13: zfVRT->m_nPortState_13 = nState; break;
        case 14: zfVRT->m_nPortState_14 = nState; break;
        case 15: zfVRT->m_nPortState_15 = nState; break;
        case 16: zfVRT->m_nPortState_16 = nState; break;
        case 17: zfVRT->m_nPortState_17 = nState; break;
        case 18: zfVRT->m_nPortState_18 = nState; break;
        case 19: zfVRT->m_nPortState_19 = nState; break;
        case 20: zfVRT->m_nPortState_20 = nState; break;
        case 21: zfVRT->m_nPortState_21 = nState; break;
        case 22: zfVRT->m_nPortState_22 = nState; break;
        case 23: zfVRT->m_nPortState_23 = nState; break;
        case 24: zfVRT->m_nPortState_24 = nState; break;
        case 25: zfVRT->m_nPortState_25 = nState; break;
        case 26: zfVRT->m_nPortState_26 = nState; break;
        case 27: zfVRT->m_nPortState_27 = nState; break;
        case 28: zfVRT->m_nPortState_28 = nState; break;
        case 29: zfVRT->m_nPortState_29 = nState; break;
        case 30: zfVRT->m_nPortState_30 = nState; break;
        case 31: zfVRT->m_nPortState_31 = nState; break;
        case 32: zfVRT->m_nPortState_32 = nState; break;
        case 33: zfVRT->m_nPortState_33 = nState; break;
        case 34: zfVRT->m_nPortState_34 = nState; break;
        case 35: zfVRT->m_nPortState_35 = nState; break;
        case 36: zfVRT->m_nPortState_36 = nState; break;
        case 37: zfVRT->m_nPortState_37 = nState; break;
        case 38: zfVRT->m_nPortState_38 = nState; break;
        case 39: zfVRT->m_nPortState_39 = nState; break;
        case 40: zfVRT->m_nPortState_40 = nState; break;
        case 41: zfVRT->m_nPortState_41 = nState; break;
        case 42: zfVRT->m_nPortState_42 = nState; break;
        case 43: zfVRT->m_nPortState_43 = nState; break;
        case 44: zfVRT->m_nPortState_44 = nState; break;
        case 45: zfVRT->m_nPortState_45 = nState; break;
        case 46: zfVRT->m_nPortState_46 = nState; break;
        case 47: zfVRT->m_nPortState_47 = nState; break;
        case 48: zfVRT->m_nPortState_48 = nState; break;
        case 49: zfVRT->m_nPortState_49 = nState; break;
        default: SB_ASSERT(0);
    }
}

