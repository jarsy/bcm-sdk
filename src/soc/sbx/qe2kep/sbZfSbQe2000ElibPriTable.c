/*
 * $Id: sbZfSbQe2000ElibPriTable.c,v 1.4 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


#include "sbTypesGlue.h"
#include "sbZfSbQe2000ElibPriTable.hx"
#include "sbWrappers.h"
#include <sal/types.h>



/* Pack from struct into array of bytes */
uint32
sbZfSbQe2000ElibPriTable_Pack(sbZfSbQe2000ElibPriTable_t *pFrom,
                              uint8 *pToData,
                              uint32 nMaxToDataIndex) {
  int i;
  int size = SB_ZF_SB_QE2000_ELIB_PRI_TABLE_SIZE_IN_BYTES;

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on little endian */

  /* Pack Member: Rsvd7 */
  (pToData)[31] |= ((pFrom)->Rsvd7) & 0xFF;

  /* Pack Member: Pri63 */
  (pToData)[30] |= ((pFrom)->Pri63 & 0x07) <<5;

  /* Pack Member: Pri62 */
  (pToData)[30] |= ((pFrom)->Pri62 & 0x07) <<2;

  /* Pack Member: Pri61 */
  (pToData)[29] |= ((pFrom)->Pri61 & 0x01) <<7;
  (pToData)[30] |= ((pFrom)->Pri61 >> 1) & 0x03;

  /* Pack Member: Pri60 */
  (pToData)[29] |= ((pFrom)->Pri60 & 0x07) <<4;

  /* Pack Member: Pri59 */
  (pToData)[29] |= ((pFrom)->Pri59 & 0x07) <<1;

  /* Pack Member: Pri58 */
  (pToData)[28] |= ((pFrom)->Pri58 & 0x03) <<6;
  (pToData)[29] |= ((pFrom)->Pri58 >> 2) & 0x01;

  /* Pack Member: Pri57 */
  (pToData)[28] |= ((pFrom)->Pri57 & 0x07) <<3;

  /* Pack Member: Pri56 */
  (pToData)[28] |= ((pFrom)->Pri56 & 0x07);

  /* Pack Member: Rsvd6 */
  (pToData)[27] |= ((pFrom)->Rsvd6) & 0xFF;

  /* Pack Member: Pri55 */
  (pToData)[26] |= ((pFrom)->Pri55 & 0x07) <<5;

  /* Pack Member: Pri54 */
  (pToData)[26] |= ((pFrom)->Pri54 & 0x07) <<2;

  /* Pack Member: Pri53 */
  (pToData)[25] |= ((pFrom)->Pri53 & 0x01) <<7;
  (pToData)[26] |= ((pFrom)->Pri53 >> 1) & 0x03;

  /* Pack Member: Pri52 */
  (pToData)[25] |= ((pFrom)->Pri52 & 0x07) <<4;

  /* Pack Member: Pri51 */
  (pToData)[25] |= ((pFrom)->Pri51 & 0x07) <<1;

  /* Pack Member: Pri50 */
  (pToData)[24] |= ((pFrom)->Pri50 & 0x03) <<6;
  (pToData)[25] |= ((pFrom)->Pri50 >> 2) & 0x01;

  /* Pack Member: Pri49 */
  (pToData)[24] |= ((pFrom)->Pri49 & 0x07) <<3;

  /* Pack Member: Pri48 */
  (pToData)[24] |= ((pFrom)->Pri48 & 0x07);

  /* Pack Member: Rsvd5 */
  (pToData)[23] |= ((pFrom)->Rsvd5) & 0xFF;

  /* Pack Member: Pri47 */
  (pToData)[22] |= ((pFrom)->Pri47 & 0x07) <<5;

  /* Pack Member: Pri46 */
  (pToData)[22] |= ((pFrom)->Pri46 & 0x07) <<2;

  /* Pack Member: Pri45 */
  (pToData)[21] |= ((pFrom)->Pri45 & 0x01) <<7;
  (pToData)[22] |= ((pFrom)->Pri45 >> 1) & 0x03;

  /* Pack Member: Pri44 */
  (pToData)[21] |= ((pFrom)->Pri44 & 0x07) <<4;

  /* Pack Member: Pri43 */
  (pToData)[21] |= ((pFrom)->Pri43 & 0x07) <<1;

  /* Pack Member: Pri42 */
  (pToData)[20] |= ((pFrom)->Pri42 & 0x03) <<6;
  (pToData)[21] |= ((pFrom)->Pri42 >> 2) & 0x01;

  /* Pack Member: Pri41 */
  (pToData)[20] |= ((pFrom)->Pri41 & 0x07) <<3;

  /* Pack Member: Pri40 */
  (pToData)[20] |= ((pFrom)->Pri40 & 0x07);

  /* Pack Member: Rsvd4 */
  (pToData)[19] |= ((pFrom)->Rsvd4) & 0xFF;

  /* Pack Member: Pri39 */
  (pToData)[18] |= ((pFrom)->Pri39 & 0x07) <<5;

  /* Pack Member: Pri38 */
  (pToData)[18] |= ((pFrom)->Pri38 & 0x07) <<2;

  /* Pack Member: Pri37 */
  (pToData)[17] |= ((pFrom)->Pri37 & 0x01) <<7;
  (pToData)[18] |= ((pFrom)->Pri37 >> 1) & 0x03;

  /* Pack Member: Pri36 */
  (pToData)[17] |= ((pFrom)->Pri36 & 0x07) <<4;

  /* Pack Member: Pri35 */
  (pToData)[17] |= ((pFrom)->Pri35 & 0x07) <<1;

  /* Pack Member: Pri34 */
  (pToData)[16] |= ((pFrom)->Pri34 & 0x03) <<6;
  (pToData)[17] |= ((pFrom)->Pri34 >> 2) & 0x01;

  /* Pack Member: Pri33 */
  (pToData)[16] |= ((pFrom)->Pri33 & 0x07) <<3;

  /* Pack Member: Pri32 */
  (pToData)[16] |= ((pFrom)->Pri32 & 0x07);

  /* Pack Member: Rsvd3 */
  (pToData)[15] |= ((pFrom)->Rsvd3) & 0xFF;

  /* Pack Member: Pri31 */
  (pToData)[14] |= ((pFrom)->Pri31 & 0x07) <<5;

  /* Pack Member: Pri30 */
  (pToData)[14] |= ((pFrom)->Pri30 & 0x07) <<2;

  /* Pack Member: Pri29 */
  (pToData)[13] |= ((pFrom)->Pri29 & 0x01) <<7;
  (pToData)[14] |= ((pFrom)->Pri29 >> 1) & 0x03;

  /* Pack Member: Pri28 */
  (pToData)[13] |= ((pFrom)->Pri28 & 0x07) <<4;

  /* Pack Member: Pri27 */
  (pToData)[13] |= ((pFrom)->Pri27 & 0x07) <<1;

  /* Pack Member: Pri26 */
  (pToData)[12] |= ((pFrom)->Pri26 & 0x03) <<6;
  (pToData)[13] |= ((pFrom)->Pri26 >> 2) & 0x01;

  /* Pack Member: Pri25 */
  (pToData)[12] |= ((pFrom)->Pri25 & 0x07) <<3;

  /* Pack Member: Pri24 */
  (pToData)[12] |= ((pFrom)->Pri24 & 0x07);

  /* Pack Member: Rsvd2 */
  (pToData)[11] |= ((pFrom)->Rsvd2) & 0xFF;

  /* Pack Member: Pri23 */
  (pToData)[10] |= ((pFrom)->Pri23 & 0x07) <<5;

  /* Pack Member: Pri22 */
  (pToData)[10] |= ((pFrom)->Pri22 & 0x07) <<2;

  /* Pack Member: Pri21 */
  (pToData)[9] |= ((pFrom)->Pri21 & 0x01) <<7;
  (pToData)[10] |= ((pFrom)->Pri21 >> 1) & 0x03;

  /* Pack Member: Pri20 */
  (pToData)[9] |= ((pFrom)->Pri20 & 0x07) <<4;

  /* Pack Member: Pri19 */
  (pToData)[9] |= ((pFrom)->Pri19 & 0x07) <<1;

  /* Pack Member: Pri18 */
  (pToData)[8] |= ((pFrom)->Pri18 & 0x03) <<6;
  (pToData)[9] |= ((pFrom)->Pri18 >> 2) & 0x01;

  /* Pack Member: Pri17 */
  (pToData)[8] |= ((pFrom)->Pri17 & 0x07) <<3;

  /* Pack Member: Pri16 */
  (pToData)[8] |= ((pFrom)->Pri16 & 0x07);

  /* Pack Member: Rsvd1 */
  (pToData)[7] |= ((pFrom)->Rsvd1) & 0xFF;

  /* Pack Member: Pri15 */
  (pToData)[6] |= ((pFrom)->Pri15 & 0x07) <<5;

  /* Pack Member: Pri14 */
  (pToData)[6] |= ((pFrom)->Pri14 & 0x07) <<2;

  /* Pack Member: Pri13 */
  (pToData)[5] |= ((pFrom)->Pri13 & 0x01) <<7;
  (pToData)[6] |= ((pFrom)->Pri13 >> 1) & 0x03;

  /* Pack Member: Pri12 */
  (pToData)[5] |= ((pFrom)->Pri12 & 0x07) <<4;

  /* Pack Member: Pri11 */
  (pToData)[5] |= ((pFrom)->Pri11 & 0x07) <<1;

  /* Pack Member: Pri10 */
  (pToData)[4] |= ((pFrom)->Pri10 & 0x03) <<6;
  (pToData)[5] |= ((pFrom)->Pri10 >> 2) & 0x01;

  /* Pack Member: Pri9 */
  (pToData)[4] |= ((pFrom)->Pri9 & 0x07) <<3;

  /* Pack Member: Pri8 */
  (pToData)[4] |= ((pFrom)->Pri8 & 0x07);

  /* Pack Member: Rsvd0 */
  (pToData)[3] |= ((pFrom)->Rsvd0) & 0xFF;

  /* Pack Member: Pri7 */
  (pToData)[2] |= ((pFrom)->Pri7 & 0x07) <<5;

  /* Pack Member: Pri6 */
  (pToData)[2] |= ((pFrom)->Pri6 & 0x07) <<2;

  /* Pack Member: Pri5 */
  (pToData)[1] |= ((pFrom)->Pri5 & 0x01) <<7;
  (pToData)[2] |= ((pFrom)->Pri5 >> 1) & 0x03;

  /* Pack Member: Pri4 */
  (pToData)[1] |= ((pFrom)->Pri4 & 0x07) <<4;

  /* Pack Member: Pri3 */
  (pToData)[1] |= ((pFrom)->Pri3 & 0x07) <<1;

  /* Pack Member: Pri2 */
  (pToData)[0] |= ((pFrom)->Pri2 & 0x03) <<6;
  (pToData)[1] |= ((pFrom)->Pri2 >> 2) & 0x01;

  /* Pack Member: Pri1 */
  (pToData)[0] |= ((pFrom)->Pri1 & 0x07) <<3;

  /* Pack Member: Pri0 */
  (pToData)[0] |= ((pFrom)->Pri0 & 0x07);

  return SB_ZF_SB_QE2000_ELIB_PRI_TABLE_SIZE_IN_BYTES;
}




/* Unpack from array of bytes into struct */
void
sbZfSbQe2000ElibPriTable_Unpack(sbZfSbQe2000ElibPriTable_t *pToStruct,
                                uint8 *pFromData,
                                uint32 nMaxToDataIndex) {
  COMPILER_UINT64 tmp;

  (void) tmp;


  /* Unpack operation based on little endian */

  /* Unpack Member: Rsvd7 */
  (pToStruct)->Rsvd7 =  (uint32)  (pFromData)[31] ;

  /* Unpack Member: Pri63 */
  (pToStruct)->Pri63 =  (uint32)  ((pFromData)[30] >> 5) & 0x07;

  /* Unpack Member: Pri62 */
  (pToStruct)->Pri62 =  (uint32)  ((pFromData)[30] >> 2) & 0x07;

  /* Unpack Member: Pri61 */
  (pToStruct)->Pri61 =  (uint32)  ((pFromData)[29] >> 7) & 0x01;
  (pToStruct)->Pri61 |=  (uint32)  ((pFromData)[30] & 0x03) << 1;

  /* Unpack Member: Pri60 */
  (pToStruct)->Pri60 =  (uint32)  ((pFromData)[29] >> 4) & 0x07;

  /* Unpack Member: Pri59 */
  (pToStruct)->Pri59 =  (uint32)  ((pFromData)[29] >> 1) & 0x07;

  /* Unpack Member: Pri58 */
  (pToStruct)->Pri58 =  (uint32)  ((pFromData)[28] >> 6) & 0x03;
  (pToStruct)->Pri58 |=  (uint32)  ((pFromData)[29] & 0x01) << 2;

  /* Unpack Member: Pri57 */
  (pToStruct)->Pri57 =  (uint32)  ((pFromData)[28] >> 3) & 0x07;

  /* Unpack Member: Pri56 */
  (pToStruct)->Pri56 =  (uint32)  ((pFromData)[28] ) & 0x07;

  /* Unpack Member: Rsvd6 */
  (pToStruct)->Rsvd6 =  (uint32)  (pFromData)[27] ;

  /* Unpack Member: Pri55 */
  (pToStruct)->Pri55 =  (uint32)  ((pFromData)[26] >> 5) & 0x07;

  /* Unpack Member: Pri54 */
  (pToStruct)->Pri54 =  (uint32)  ((pFromData)[26] >> 2) & 0x07;

  /* Unpack Member: Pri53 */
  (pToStruct)->Pri53 =  (uint32)  ((pFromData)[25] >> 7) & 0x01;
  (pToStruct)->Pri53 |=  (uint32)  ((pFromData)[26] & 0x03) << 1;

  /* Unpack Member: Pri52 */
  (pToStruct)->Pri52 =  (uint32)  ((pFromData)[25] >> 4) & 0x07;

  /* Unpack Member: Pri51 */
  (pToStruct)->Pri51 =  (uint32)  ((pFromData)[25] >> 1) & 0x07;

  /* Unpack Member: Pri50 */
  (pToStruct)->Pri50 =  (uint32)  ((pFromData)[24] >> 6) & 0x03;
  (pToStruct)->Pri50 |=  (uint32)  ((pFromData)[25] & 0x01) << 2;

  /* Unpack Member: Pri49 */
  (pToStruct)->Pri49 =  (uint32)  ((pFromData)[24] >> 3) & 0x07;

  /* Unpack Member: Pri48 */
  (pToStruct)->Pri48 =  (uint32)  ((pFromData)[24] ) & 0x07;

  /* Unpack Member: Rsvd5 */
  (pToStruct)->Rsvd5 =  (uint32)  (pFromData)[23] ;

  /* Unpack Member: Pri47 */
  (pToStruct)->Pri47 =  (uint32)  ((pFromData)[22] >> 5) & 0x07;

  /* Unpack Member: Pri46 */
  (pToStruct)->Pri46 =  (uint32)  ((pFromData)[22] >> 2) & 0x07;

  /* Unpack Member: Pri45 */
  (pToStruct)->Pri45 =  (uint32)  ((pFromData)[21] >> 7) & 0x01;
  (pToStruct)->Pri45 |=  (uint32)  ((pFromData)[22] & 0x03) << 1;

  /* Unpack Member: Pri44 */
  (pToStruct)->Pri44 =  (uint32)  ((pFromData)[21] >> 4) & 0x07;

  /* Unpack Member: Pri43 */
  (pToStruct)->Pri43 =  (uint32)  ((pFromData)[21] >> 1) & 0x07;

  /* Unpack Member: Pri42 */
  (pToStruct)->Pri42 =  (uint32)  ((pFromData)[20] >> 6) & 0x03;
  (pToStruct)->Pri42 |=  (uint32)  ((pFromData)[21] & 0x01) << 2;

  /* Unpack Member: Pri41 */
  (pToStruct)->Pri41 =  (uint32)  ((pFromData)[20] >> 3) & 0x07;

  /* Unpack Member: Pri40 */
  (pToStruct)->Pri40 =  (uint32)  ((pFromData)[20] ) & 0x07;

  /* Unpack Member: Rsvd4 */
  (pToStruct)->Rsvd4 =  (uint32)  (pFromData)[19] ;

  /* Unpack Member: Pri39 */
  (pToStruct)->Pri39 =  (uint32)  ((pFromData)[18] >> 5) & 0x07;

  /* Unpack Member: Pri38 */
  (pToStruct)->Pri38 =  (uint32)  ((pFromData)[18] >> 2) & 0x07;

  /* Unpack Member: Pri37 */
  (pToStruct)->Pri37 =  (uint32)  ((pFromData)[17] >> 7) & 0x01;
  (pToStruct)->Pri37 |=  (uint32)  ((pFromData)[18] & 0x03) << 1;

  /* Unpack Member: Pri36 */
  (pToStruct)->Pri36 =  (uint32)  ((pFromData)[17] >> 4) & 0x07;

  /* Unpack Member: Pri35 */
  (pToStruct)->Pri35 =  (uint32)  ((pFromData)[17] >> 1) & 0x07;

  /* Unpack Member: Pri34 */
  (pToStruct)->Pri34 =  (uint32)  ((pFromData)[16] >> 6) & 0x03;
  (pToStruct)->Pri34 |=  (uint32)  ((pFromData)[17] & 0x01) << 2;

  /* Unpack Member: Pri33 */
  (pToStruct)->Pri33 =  (uint32)  ((pFromData)[16] >> 3) & 0x07;

  /* Unpack Member: Pri32 */
  (pToStruct)->Pri32 =  (uint32)  ((pFromData)[16] ) & 0x07;

  /* Unpack Member: Rsvd3 */
  (pToStruct)->Rsvd3 =  (uint32)  (pFromData)[15] ;

  /* Unpack Member: Pri31 */
  (pToStruct)->Pri31 =  (uint32)  ((pFromData)[14] >> 5) & 0x07;

  /* Unpack Member: Pri30 */
  (pToStruct)->Pri30 =  (uint32)  ((pFromData)[14] >> 2) & 0x07;

  /* Unpack Member: Pri29 */
  (pToStruct)->Pri29 =  (uint32)  ((pFromData)[13] >> 7) & 0x01;
  (pToStruct)->Pri29 |=  (uint32)  ((pFromData)[14] & 0x03) << 1;

  /* Unpack Member: Pri28 */
  (pToStruct)->Pri28 =  (uint32)  ((pFromData)[13] >> 4) & 0x07;

  /* Unpack Member: Pri27 */
  (pToStruct)->Pri27 =  (uint32)  ((pFromData)[13] >> 1) & 0x07;

  /* Unpack Member: Pri26 */
  (pToStruct)->Pri26 =  (uint32)  ((pFromData)[12] >> 6) & 0x03;
  (pToStruct)->Pri26 |=  (uint32)  ((pFromData)[13] & 0x01) << 2;

  /* Unpack Member: Pri25 */
  (pToStruct)->Pri25 =  (uint32)  ((pFromData)[12] >> 3) & 0x07;

  /* Unpack Member: Pri24 */
  (pToStruct)->Pri24 =  (uint32)  ((pFromData)[12] ) & 0x07;

  /* Unpack Member: Rsvd2 */
  (pToStruct)->Rsvd2 =  (uint32)  (pFromData)[11] ;

  /* Unpack Member: Pri23 */
  (pToStruct)->Pri23 =  (uint32)  ((pFromData)[10] >> 5) & 0x07;

  /* Unpack Member: Pri22 */
  (pToStruct)->Pri22 =  (uint32)  ((pFromData)[10] >> 2) & 0x07;

  /* Unpack Member: Pri21 */
  (pToStruct)->Pri21 =  (uint32)  ((pFromData)[9] >> 7) & 0x01;
  (pToStruct)->Pri21 |=  (uint32)  ((pFromData)[10] & 0x03) << 1;

  /* Unpack Member: Pri20 */
  (pToStruct)->Pri20 =  (uint32)  ((pFromData)[9] >> 4) & 0x07;

  /* Unpack Member: Pri19 */
  (pToStruct)->Pri19 =  (uint32)  ((pFromData)[9] >> 1) & 0x07;

  /* Unpack Member: Pri18 */
  (pToStruct)->Pri18 =  (uint32)  ((pFromData)[8] >> 6) & 0x03;
  (pToStruct)->Pri18 |=  (uint32)  ((pFromData)[9] & 0x01) << 2;

  /* Unpack Member: Pri17 */
  (pToStruct)->Pri17 =  (uint32)  ((pFromData)[8] >> 3) & 0x07;

  /* Unpack Member: Pri16 */
  (pToStruct)->Pri16 =  (uint32)  ((pFromData)[8] ) & 0x07;

  /* Unpack Member: Rsvd1 */
  (pToStruct)->Rsvd1 =  (uint32)  (pFromData)[7] ;

  /* Unpack Member: Pri15 */
  (pToStruct)->Pri15 =  (uint32)  ((pFromData)[6] >> 5) & 0x07;

  /* Unpack Member: Pri14 */
  (pToStruct)->Pri14 =  (uint32)  ((pFromData)[6] >> 2) & 0x07;

  /* Unpack Member: Pri13 */
  (pToStruct)->Pri13 =  (uint32)  ((pFromData)[5] >> 7) & 0x01;
  (pToStruct)->Pri13 |=  (uint32)  ((pFromData)[6] & 0x03) << 1;

  /* Unpack Member: Pri12 */
  (pToStruct)->Pri12 =  (uint32)  ((pFromData)[5] >> 4) & 0x07;

  /* Unpack Member: Pri11 */
  (pToStruct)->Pri11 =  (uint32)  ((pFromData)[5] >> 1) & 0x07;

  /* Unpack Member: Pri10 */
  (pToStruct)->Pri10 =  (uint32)  ((pFromData)[4] >> 6) & 0x03;
  (pToStruct)->Pri10 |=  (uint32)  ((pFromData)[5] & 0x01) << 2;

  /* Unpack Member: Pri9 */
  (pToStruct)->Pri9 =  (uint32)  ((pFromData)[4] >> 3) & 0x07;

  /* Unpack Member: Pri8 */
  (pToStruct)->Pri8 =  (uint32)  ((pFromData)[4] ) & 0x07;

  /* Unpack Member: Rsvd0 */
  (pToStruct)->Rsvd0 =  (uint32)  (pFromData)[3] ;

  /* Unpack Member: Pri7 */
  (pToStruct)->Pri7 =  (uint32)  ((pFromData)[2] >> 5) & 0x07;

  /* Unpack Member: Pri6 */
  (pToStruct)->Pri6 =  (uint32)  ((pFromData)[2] >> 2) & 0x07;

  /* Unpack Member: Pri5 */
  (pToStruct)->Pri5 =  (uint32)  ((pFromData)[1] >> 7) & 0x01;
  (pToStruct)->Pri5 |=  (uint32)  ((pFromData)[2] & 0x03) << 1;

  /* Unpack Member: Pri4 */
  (pToStruct)->Pri4 =  (uint32)  ((pFromData)[1] >> 4) & 0x07;

  /* Unpack Member: Pri3 */
  (pToStruct)->Pri3 =  (uint32)  ((pFromData)[1] >> 1) & 0x07;

  /* Unpack Member: Pri2 */
  (pToStruct)->Pri2 =  (uint32)  ((pFromData)[0] >> 6) & 0x03;
  (pToStruct)->Pri2 |=  (uint32)  ((pFromData)[1] & 0x01) << 2;

  /* Unpack Member: Pri1 */
  (pToStruct)->Pri1 =  (uint32)  ((pFromData)[0] >> 3) & 0x07;

  /* Unpack Member: Pri0 */
  (pToStruct)->Pri0 =  (uint32)  ((pFromData)[0] ) & 0x07;

}



/* initialize an instance of this zframe */
void
sbZfSbQe2000ElibPriTable_InitInstance(sbZfSbQe2000ElibPriTable_t *pFrame) {

  pFrame->Rsvd7 =  (unsigned int)  0;
  pFrame->Pri63 =  (unsigned int)  0;
  pFrame->Pri62 =  (unsigned int)  0;
  pFrame->Pri61 =  (unsigned int)  0;
  pFrame->Pri60 =  (unsigned int)  0;
  pFrame->Pri59 =  (unsigned int)  0;
  pFrame->Pri58 =  (unsigned int)  0;
  pFrame->Pri57 =  (unsigned int)  0;
  pFrame->Pri56 =  (unsigned int)  0;
  pFrame->Rsvd6 =  (unsigned int)  0;
  pFrame->Pri55 =  (unsigned int)  0;
  pFrame->Pri54 =  (unsigned int)  0;
  pFrame->Pri53 =  (unsigned int)  0;
  pFrame->Pri52 =  (unsigned int)  0;
  pFrame->Pri51 =  (unsigned int)  0;
  pFrame->Pri50 =  (unsigned int)  0;
  pFrame->Pri49 =  (unsigned int)  0;
  pFrame->Pri48 =  (unsigned int)  0;
  pFrame->Rsvd5 =  (unsigned int)  0;
  pFrame->Pri47 =  (unsigned int)  0;
  pFrame->Pri46 =  (unsigned int)  0;
  pFrame->Pri45 =  (unsigned int)  0;
  pFrame->Pri44 =  (unsigned int)  0;
  pFrame->Pri43 =  (unsigned int)  0;
  pFrame->Pri42 =  (unsigned int)  0;
  pFrame->Pri41 =  (unsigned int)  0;
  pFrame->Pri40 =  (unsigned int)  0;
  pFrame->Rsvd4 =  (unsigned int)  0;
  pFrame->Pri39 =  (unsigned int)  0;
  pFrame->Pri38 =  (unsigned int)  0;
  pFrame->Pri37 =  (unsigned int)  0;
  pFrame->Pri36 =  (unsigned int)  0;
  pFrame->Pri35 =  (unsigned int)  0;
  pFrame->Pri34 =  (unsigned int)  0;
  pFrame->Pri33 =  (unsigned int)  0;
  pFrame->Pri32 =  (unsigned int)  0;
  pFrame->Rsvd3 =  (unsigned int)  0;
  pFrame->Pri31 =  (unsigned int)  0;
  pFrame->Pri30 =  (unsigned int)  0;
  pFrame->Pri29 =  (unsigned int)  0;
  pFrame->Pri28 =  (unsigned int)  0;
  pFrame->Pri27 =  (unsigned int)  0;
  pFrame->Pri26 =  (unsigned int)  0;
  pFrame->Pri25 =  (unsigned int)  0;
  pFrame->Pri24 =  (unsigned int)  0;
  pFrame->Rsvd2 =  (unsigned int)  0;
  pFrame->Pri23 =  (unsigned int)  0;
  pFrame->Pri22 =  (unsigned int)  0;
  pFrame->Pri21 =  (unsigned int)  0;
  pFrame->Pri20 =  (unsigned int)  0;
  pFrame->Pri19 =  (unsigned int)  0;
  pFrame->Pri18 =  (unsigned int)  0;
  pFrame->Pri17 =  (unsigned int)  0;
  pFrame->Pri16 =  (unsigned int)  0;
  pFrame->Rsvd1 =  (unsigned int)  0;
  pFrame->Pri15 =  (unsigned int)  0;
  pFrame->Pri14 =  (unsigned int)  0;
  pFrame->Pri13 =  (unsigned int)  0;
  pFrame->Pri12 =  (unsigned int)  0;
  pFrame->Pri11 =  (unsigned int)  0;
  pFrame->Pri10 =  (unsigned int)  0;
  pFrame->Pri9 =  (unsigned int)  0;
  pFrame->Pri8 =  (unsigned int)  0;
  pFrame->Rsvd0 =  (unsigned int)  0;
  pFrame->Pri7 =  (unsigned int)  0;
  pFrame->Pri6 =  (unsigned int)  0;
  pFrame->Pri5 =  (unsigned int)  0;
  pFrame->Pri4 =  (unsigned int)  0;
  pFrame->Pri3 =  (unsigned int)  0;
  pFrame->Pri2 =  (unsigned int)  0;
  pFrame->Pri1 =  (unsigned int)  0;
  pFrame->Pri0 =  (unsigned int)  0;

}



void sbZfSbQe2000ElibPriTableSet(sbZfSbQe2000ElibPriTable_t *pZf, UINT nIdx, UINT nPri)
{
    switch(nIdx)
    {
        case 0:  pZf->Pri0  = nPri; break;
        case 1:  pZf->Pri1  = nPri; break;
        case 2:  pZf->Pri2  = nPri; break;
        case 3:  pZf->Pri3  = nPri; break;
        case 4:  pZf->Pri4  = nPri; break;
        case 5:  pZf->Pri5  = nPri; break;
        case 6:  pZf->Pri6  = nPri; break;
        case 7:  pZf->Pri7  = nPri; break;
        case 8:  pZf->Pri8  = nPri; break;
        case 9:  pZf->Pri9  = nPri; break;

        case 10: pZf->Pri10 = nPri; break;
        case 11: pZf->Pri11 = nPri; break;
        case 12: pZf->Pri12 = nPri; break;
        case 13: pZf->Pri13 = nPri; break;
        case 14: pZf->Pri14 = nPri; break;
        case 15: pZf->Pri15 = nPri; break;
        case 16: pZf->Pri16 = nPri; break;
        case 17: pZf->Pri17 = nPri; break;
        case 18: pZf->Pri18 = nPri; break;
        case 19: pZf->Pri19 = nPri; break;

        case 20: pZf->Pri20 = nPri; break;
        case 21: pZf->Pri21 = nPri; break;
        case 22: pZf->Pri22 = nPri; break;
        case 23: pZf->Pri23 = nPri; break;
        case 24: pZf->Pri24 = nPri; break;
        case 25: pZf->Pri25 = nPri; break;
        case 26: pZf->Pri26 = nPri; break;
        case 27: pZf->Pri27 = nPri; break;
        case 28: pZf->Pri28 = nPri; break;
        case 29: pZf->Pri29 = nPri; break;

        case 30: pZf->Pri30 = nPri; break;
        case 31: pZf->Pri31 = nPri; break;
        case 32: pZf->Pri32 = nPri; break;
        case 33: pZf->Pri33 = nPri; break;
        case 34: pZf->Pri34 = nPri; break;
        case 35: pZf->Pri35 = nPri; break;
        case 36: pZf->Pri36 = nPri; break;
        case 37: pZf->Pri37 = nPri; break;
        case 38: pZf->Pri38 = nPri; break;
        case 39: pZf->Pri39 = nPri; break;

        case 40: pZf->Pri40 = nPri; break;
        case 41: pZf->Pri41 = nPri; break;
        case 42: pZf->Pri42 = nPri; break;
        case 43: pZf->Pri43 = nPri; break;
        case 44: pZf->Pri44 = nPri; break;
        case 45: pZf->Pri45 = nPri; break;
        case 46: pZf->Pri46 = nPri; break;
        case 47: pZf->Pri47 = nPri; break;
        case 48: pZf->Pri48 = nPri; break;
        case 49: pZf->Pri49 = nPri; break;

        case 50: pZf->Pri50 = nPri; break;
        case 51: pZf->Pri51 = nPri; break;
        case 52: pZf->Pri52 = nPri; break;
        case 53: pZf->Pri53 = nPri; break;
        case 54: pZf->Pri54 = nPri; break;
        case 55: pZf->Pri55 = nPri; break;
        case 56: pZf->Pri56 = nPri; break;
        case 57: pZf->Pri57 = nPri; break;
        case 58: pZf->Pri58 = nPri; break;
        case 59: pZf->Pri59 = nPri; break;

        case 60: pZf->Pri60 = nPri; break;
        case 61: pZf->Pri61 = nPri; break;
        case 62: pZf->Pri62 = nPri; break;
        case 63: pZf->Pri63 = nPri; break;

    }

    return;
}
UINT sbZfSbQe2000ElibPriTableGet(sbZfSbQe2000ElibPriTable_t *pZf, UINT nIdx)
{
    switch(nIdx)
    {
        case 0:  return pZf->Pri0; break;
        case 1:  return pZf->Pri1; break;
        case 2:  return pZf->Pri2; break;
        case 3:  return pZf->Pri3; break;
        case 4:  return pZf->Pri4; break;
        case 5:  return pZf->Pri5; break;
        case 6:  return pZf->Pri6; break;
        case 7:  return pZf->Pri7; break;
        case 8:  return pZf->Pri8; break;
        case 9:  return pZf->Pri9; break;

        case 10: return pZf->Pri10; break;
        case 11: return pZf->Pri11; break;
        case 12: return pZf->Pri12; break;
        case 13: return pZf->Pri13; break;
        case 14: return pZf->Pri14; break;
        case 15: return pZf->Pri15; break;
        case 16: return pZf->Pri16; break;
        case 17: return pZf->Pri17; break;
        case 18: return pZf->Pri18; break;
        case 19: return pZf->Pri19; break;

        case 20: return pZf->Pri20; break;
        case 21: return pZf->Pri21; break;
        case 22: return pZf->Pri22; break;
        case 23: return pZf->Pri23; break;
        case 24: return pZf->Pri24; break;
        case 25: return pZf->Pri25; break;
        case 26: return pZf->Pri26; break;
        case 27: return pZf->Pri27; break;
        case 28: return pZf->Pri28; break;
        case 29: return pZf->Pri29; break;

        case 30: return pZf->Pri30; break;
        case 31: return pZf->Pri31; break;
        case 32: return pZf->Pri32; break;
        case 33: return pZf->Pri33; break;
        case 34: return pZf->Pri34; break;
        case 35: return pZf->Pri35; break;
        case 36: return pZf->Pri36; break;
        case 37: return pZf->Pri37; break;
        case 38: return pZf->Pri38; break;
        case 39: return pZf->Pri39; break;

        case 40: return pZf->Pri40; break;
        case 41: return pZf->Pri41; break;
        case 42: return pZf->Pri42; break;
        case 43: return pZf->Pri43; break;
        case 44: return pZf->Pri44; break;
        case 45: return pZf->Pri45; break;
        case 46: return pZf->Pri46; break;
        case 47: return pZf->Pri47; break;
        case 48: return pZf->Pri48; break;
        case 49: return pZf->Pri49; break;

        case 50: return pZf->Pri50; break;
        case 51: return pZf->Pri51; break;
        case 52: return pZf->Pri52; break;
        case 53: return pZf->Pri53; break;
        case 54: return pZf->Pri54; break;
        case 55: return pZf->Pri55; break;
        case 56: return pZf->Pri56; break;
        case 57: return pZf->Pri57; break;
        case 58: return pZf->Pri58; break;
        case 59: return pZf->Pri59; break;

        case 60: return pZf->Pri60; break;
        case 61: return pZf->Pri61; break;
        case 62: return pZf->Pri62; break;
        case 63: return pZf->Pri63; break;

    }

    return( 0 );
}

