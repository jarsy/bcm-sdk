
/*
 * $Id: sandHalFrameHwQe2000FMVT.c,v 1.5 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 */

#include <sal/types.h>
#include <soc/sbx/sandHalFrameHwQe2000FMVT.h>
/*#include "sbWrappers.h" */

#define SWAPBAQB(n)  ((n & 0xFFFFFFFC) | (3 - (n & 0x00000003)))


int running_simulator = FALSE;

/* Pack from struct into array of bytes */
uint32
sandHalFrameHwQe2000FMVT_Pack(sandHalFrameHwQe2000FMVT_t *pFrom,
                              uint8 *pToData,
                              uint32 nMaxToDataIndex) {
    int i;

    for ( i=0; i<SAND_HAL_FRAME_HW_QE2000_FMVT_ENTRY_SIZE_IN_BYTES; i++ ) {
      (pToData)[i] = 0;
    }
    i = 0;

    /* Pack operation based on little endian */

    
    if (running_simulator) {

        /* Pack Member: m_nReserved */
        (pToData)[SWAPBAQB(31)] |= ((pFrom)->m_nReserved & 0x01) <<7;

        /* Pack Member: m_nnPortMap2 */
        (pToData)[SWAPBAQB(25)] |= (COMPILER_64_LO((pFrom)->m_nnPortMap2) & 0x07) <<5;
        (pToData)[SWAPBAQB(26)] |= (COMPILER_64_LO((pFrom)->m_nnPortMap2) >> 3) &0xFF;
        (pToData)[SWAPBAQB(27)] |= (COMPILER_64_LO((pFrom)->m_nnPortMap2) >> 11) &0xFF;
        (pToData)[SWAPBAQB(28)] |= (COMPILER_64_LO((pFrom)->m_nnPortMap2) >> 19) &0xFF;
        (pToData)[SWAPBAQB(29)] |= (COMPILER_64_LO((pFrom)->m_nnPortMap2) >> 27) &0x1F;
        (pToData)[SWAPBAQB(29)] |= (COMPILER_64_HI((pFrom)->m_nnPortMap2) & 0x07) <<5;
        (pToData)[SWAPBAQB(30)] |= (COMPILER_64_HI((pFrom)->m_nnPortMap2) >> 3) &0xFF;
        (pToData)[SWAPBAQB(31)] |= (COMPILER_64_HI((pFrom)->m_nnPortMap2) >> 11) & 0x7f;

        /* Pack Member: m_nMvtda2 */
        (pToData)[SWAPBAQB(23)] |= ((pFrom)->m_nMvtda2 & 0x01) <<7;
        (pToData)[SWAPBAQB(24)] |= ((pFrom)->m_nMvtda2 >> 1) &0xFF;
        (pToData)[SWAPBAQB(25)] |= ((pFrom)->m_nMvtda2 >> 9) & 0x1f;

        /* Pack Member: m_nMvtdb2 */
        (pToData)[SWAPBAQB(23)] |= ((pFrom)->m_nMvtdb2 & 0x0f) <<3;

        /* Pack Member: m_nNext2 */
        (pToData)[SWAPBAQB(21)] |= ((pFrom)->m_nNext2 & 0x1f) <<3;
        (pToData)[SWAPBAQB(22)] |= ((pFrom)->m_nNext2 >> 5) &0xFF;
        (pToData)[SWAPBAQB(23)] |= ((pFrom)->m_nNext2 >> 13) & 0x07;

        /* Pack Member: m_nKnockout2 */
        (pToData)[SWAPBAQB(21)] |= ((pFrom)->m_nKnockout2 & 0x01) <<2;

        /* Pack Member: m_nnPortMap1 */
        (pToData)[SWAPBAQB(15)] |= (COMPILER_64_LO((pFrom)->m_nnPortMap1)) & 0xFF;
        (pToData)[SWAPBAQB(16)] |= (COMPILER_64_LO((pFrom)->m_nnPortMap1) >> 8) &0xFF;
        (pToData)[SWAPBAQB(17)] |= (COMPILER_64_LO((pFrom)->m_nnPortMap1) >> 16) &0xFF;
        (pToData)[SWAPBAQB(18)] |= (COMPILER_64_LO((pFrom)->m_nnPortMap1) >> 24) &0xFF;
        (pToData)[SWAPBAQB(19)] |= (COMPILER_64_HI((pFrom)->m_nnPortMap1)) &0xFF;
        (pToData)[SWAPBAQB(20)] |= (COMPILER_64_HI((pFrom)->m_nnPortMap1) >> 8) &0xFF;
        (pToData)[SWAPBAQB(21)] |= (COMPILER_64_HI((pFrom)->m_nnPortMap1) >> 16) & 0x03;

        /* Pack Member: m_nMvtda1 */
        (pToData)[SWAPBAQB(13)] |= ((pFrom)->m_nMvtda1 & 0x3f) <<2;
        (pToData)[SWAPBAQB(14)] |= ((pFrom)->m_nMvtda1 >> 6) &0xFF;

        /* Pack Member: m_nMvtdb1 */
        (pToData)[SWAPBAQB(12)] |= ((pFrom)->m_nMvtdb1 & 0x03) <<6;
        (pToData)[SWAPBAQB(13)] |= ((pFrom)->m_nMvtdb1 >> 2) & 0x03;

        /* Pack Member: m_nNext1 */
        (pToData)[SWAPBAQB(10)] |= ((pFrom)->m_nNext1 & 0x03) <<6;
        (pToData)[SWAPBAQB(11)] |= ((pFrom)->m_nNext1 >> 2) &0xFF;
        (pToData)[SWAPBAQB(12)] |= ((pFrom)->m_nNext1 >> 10) & 0x3f;

        /* Pack Member: m_nKnockout1 */
        (pToData)[SWAPBAQB(10)] |= ((pFrom)->m_nKnockout1 & 0x01) <<5;

        /* Pack Member: m_nnPortMap0 */
        (pToData)[SWAPBAQB(4)] |= (COMPILER_64_LO((pFrom)->m_nnPortMap0) & 0x1f) <<3;
        (pToData)[SWAPBAQB(5)] |= (COMPILER_64_LO((pFrom)->m_nnPortMap0) >> 5) &0xFF;
        (pToData)[SWAPBAQB(6)] |= (COMPILER_64_LO((pFrom)->m_nnPortMap0) >> 13) &0xFF;
        (pToData)[SWAPBAQB(7)] |= (COMPILER_64_LO((pFrom)->m_nnPortMap0) >> 21) &0xFF;
        (pToData)[SWAPBAQB(8)] |= (COMPILER_64_LO((pFrom)->m_nnPortMap0) >> 29) &0x07;
        (pToData)[SWAPBAQB(8)] |= (COMPILER_64_HI((pFrom)->m_nnPortMap0) & 0x1f) <<3;
        (pToData)[SWAPBAQB(9)] |= (COMPILER_64_HI((pFrom)->m_nnPortMap0) >> 5) &0xFF;
        (pToData)[SWAPBAQB(10)]|= (COMPILER_64_HI((pFrom)->m_nnPortMap0) >> 13) & 0x1f;

        /* Pack Member: m_nMvtda0 */
        (pToData)[SWAPBAQB(2)] |= ((pFrom)->m_nMvtda0 & 0x07) <<5;
        (pToData)[SWAPBAQB(3)] |= ((pFrom)->m_nMvtda0 >> 3) &0xFF;
        (pToData)[SWAPBAQB(4)] |= ((pFrom)->m_nMvtda0 >> 11) & 0x07;

        /* Pack Member: m_nMvtdb0 */
        (pToData)[SWAPBAQB(2)] |= ((pFrom)->m_nMvtdb0 & 0x0f) <<1;

        /* Pack Member: m_nNext0 */
        (pToData)[SWAPBAQB(0)] |= ((pFrom)->m_nNext0 & 0x7f) <<1;
        (pToData)[SWAPBAQB(1)] |= ((pFrom)->m_nNext0 >> 7) &0xFF;
        (pToData)[SWAPBAQB(2)] |= ((pFrom)->m_nNext0 >> 15) & 0x01;

        /* Pack Member: m_nKnockout0 */
        (pToData)[SWAPBAQB(0)] |= ((pFrom)->m_nKnockout0 & 0x01);

    } else {

        /* Pack Member: m_nReserved */
        (pToData)[31] |= ((pFrom)->m_nReserved & 0x01) <<7;

        /* Pack Member: m_nnPortMap2 */
        (pToData)[25] |= (COMPILER_64_LO((pFrom)->m_nnPortMap2) & 0x07) <<5;
        (pToData)[26] |= (COMPILER_64_LO((pFrom)->m_nnPortMap2) >> 3) &0xFF;
        (pToData)[27] |= (COMPILER_64_LO((pFrom)->m_nnPortMap2) >> 11) &0xFF;
        (pToData)[28] |= (COMPILER_64_LO((pFrom)->m_nnPortMap2) >> 19) &0xFF;
        (pToData)[29] |= (COMPILER_64_LO((pFrom)->m_nnPortMap2) >> 27) &0x1F;
        (pToData)[29] |= (COMPILER_64_HI((pFrom)->m_nnPortMap2) & 0x07) << 5;
        (pToData)[30] |= (COMPILER_64_HI((pFrom)->m_nnPortMap2) >> 3) &0xFF;
        (pToData)[31] |= (COMPILER_64_HI((pFrom)->m_nnPortMap2) >> 11) & 0x7f;

        /* Pack Member: m_nMvtda2 */
        (pToData)[23] |= ((pFrom)->m_nMvtda2 & 0x01) <<7;
        (pToData)[24] |= ((pFrom)->m_nMvtda2 >> 1) &0xFF;
        (pToData)[25] |= ((pFrom)->m_nMvtda2 >> 9) & 0x1f;

        /* Pack Member: m_nMvtdb2 */
        (pToData)[23] |= ((pFrom)->m_nMvtdb2 & 0x0f) <<3;

        /* Pack Member: m_nNext2 */
        (pToData)[21] |= ((pFrom)->m_nNext2 & 0x1f) <<3;
        (pToData)[22] |= ((pFrom)->m_nNext2 >> 5) &0xFF;
        (pToData)[23] |= ((pFrom)->m_nNext2 >> 13) & 0x07;

        /* Pack Member: m_nKnockout2 */
        (pToData)[21] |= ((pFrom)->m_nKnockout2 & 0x01) <<2;

        /* Pack Member: m_nnPortMap1 */
        (pToData)[15] |= (COMPILER_64_LO((pFrom)->m_nnPortMap1)) & 0xFF;
        (pToData)[16] |= (COMPILER_64_LO((pFrom)->m_nnPortMap1) >> 8) &0xFF;
        (pToData)[17] |= (COMPILER_64_LO((pFrom)->m_nnPortMap1) >> 16) &0xFF;
        (pToData)[18] |= (COMPILER_64_LO((pFrom)->m_nnPortMap1) >> 24) &0xFF;
        (pToData)[19] |= (COMPILER_64_HI((pFrom)->m_nnPortMap1)) &0xFF;
        (pToData)[20] |= (COMPILER_64_HI((pFrom)->m_nnPortMap1) >> 8) &0xFF;
        (pToData)[21] |= (COMPILER_64_HI((pFrom)->m_nnPortMap1) >> 16) & 0x03;

        /* Pack Member: m_nMvtda1 */
        (pToData)[13] |= ((pFrom)->m_nMvtda1 & 0x3f) <<2;
        (pToData)[14] |= ((pFrom)->m_nMvtda1 >> 6) &0xFF;

        /* Pack Member: m_nMvtdb1 */
        (pToData)[12] |= ((pFrom)->m_nMvtdb1 & 0x03) <<6;
        (pToData)[13] |= ((pFrom)->m_nMvtdb1 >> 2) & 0x03;

        /* Pack Member: m_nNext1 */
        (pToData)[10] |= ((pFrom)->m_nNext1 & 0x03) <<6;
        (pToData)[11] |= ((pFrom)->m_nNext1 >> 2) &0xFF;
        (pToData)[12] |= ((pFrom)->m_nNext1 >> 10) & 0x3f;

        /* Pack Member: m_nKnockout1 */
        (pToData)[10] |= ((pFrom)->m_nKnockout1 & 0x01) <<5;

        /* Pack Member: m_nnPortMap0 */
        (pToData)[4] |= (COMPILER_64_LO((pFrom)->m_nnPortMap0) & 0x1f) <<3;
        (pToData)[5] |= (COMPILER_64_LO((pFrom)->m_nnPortMap0) >> 5) &0xFF;
        (pToData)[6] |= (COMPILER_64_LO((pFrom)->m_nnPortMap0) >> 13) &0xFF;
        (pToData)[7] |= (COMPILER_64_LO((pFrom)->m_nnPortMap0) >> 21) &0xFF;
        (pToData)[8] |= (COMPILER_64_LO((pFrom)->m_nnPortMap0) >> 29) &0x07;
        (pToData)[8] |= (COMPILER_64_HI((pFrom)->m_nnPortMap0) & 0x1f) <<3;
        (pToData)[9] |= (COMPILER_64_HI((pFrom)->m_nnPortMap0) >> 5) &0xFF;
        (pToData)[10] |= (COMPILER_64_HI((pFrom)->m_nnPortMap0) >> 13) & 0x1f;

        /* Pack Member: m_nMvtda0 */
        (pToData)[2] |= ((pFrom)->m_nMvtda0 & 0x07) <<5;
        (pToData)[3] |= ((pFrom)->m_nMvtda0 >> 3) &0xFF;
        (pToData)[4] |= ((pFrom)->m_nMvtda0 >> 11) & 0x07;

        /* Pack Member: m_nMvtdb0 */
        (pToData)[2] |= ((pFrom)->m_nMvtdb0 & 0x0f) <<1;

        /* Pack Member: m_nNext0 */
        (pToData)[0] |= ((pFrom)->m_nNext0 & 0x7f) <<1;
        (pToData)[1] |= ((pFrom)->m_nNext0 >> 7) &0xFF;
        (pToData)[2] |= ((pFrom)->m_nNext0 >> 15) & 0x01;

        /* Pack Member: m_nKnockout0 */
        (pToData)[0] |= ((pFrom)->m_nKnockout0 & 0x01);


    }

    return SAND_HAL_FRAME_HW_QE2000_FMVT_ENTRY_SIZE_IN_BYTES;
}




/* Unpack from array of bytes into struct */
void
sandHalFrameHwQe2000FMVT_Unpack(sandHalFrameHwQe2000FMVT_t *pToStruct,
                                uint8 *pFromData,
                                uint32 nMaxToDataIndex) {
    int64 tmp, uuVal;

    /* Unpack operation based on little endian */

    
    if (running_simulator) {
        /* Unpack Member: m_nReserved */
        (pToStruct)->m_nReserved =  (uint32)  ((pFromData)[SWAPBAQB(31)] >> 7) & 0x01;

        /* Unpack Member: m_nnPortMap2 */
        COMPILER_64_ZERO((pToStruct)->m_nnPortMap2);
        COMPILER_64_SET(tmp, 0,((pFromData)[SWAPBAQB(25)]));
        COMPILER_64_SHR(tmp, 5);
        COMPILER_64_SET(uuVal, 0, 0x07);
        COMPILER_64_AND(tmp, uuVal);
        COMPILER_64_OR((pToStruct)->m_nnPortMap2, tmp);

        COMPILER_64_SET(tmp, 0,((pFromData)[SWAPBAQB(26)]));
        COMPILER_64_SHL(tmp, 3);
        COMPILER_64_OR((pToStruct)->m_nnPortMap2, tmp);

        COMPILER_64_SET(tmp, 0,((pFromData)[SWAPBAQB(27)]));
        COMPILER_64_SHL(tmp, 11);
        COMPILER_64_OR((pToStruct)->m_nnPortMap2, tmp);

        COMPILER_64_SET(tmp, 0,((pFromData)[SWAPBAQB(28)]));
        COMPILER_64_SHL(tmp, 19);
        COMPILER_64_OR((pToStruct)->m_nnPortMap2, tmp);

        COMPILER_64_SET(tmp, 0,((pFromData)[SWAPBAQB(29)]));
        COMPILER_64_SHL(tmp, 27);
        COMPILER_64_OR((pToStruct)->m_nnPortMap2, tmp);
        /**
        (pToStruct)->m_nnPortMap2 =  (uint64)  ((pFromData)[SWAPBAQB(25)]>> 5) & 0x07;
        (pToStruct)->m_nnPortMap2 |=  (uint64)  (pFromData)[SWAPBAQB(26)] << 3;
        (pToStruct)->m_nnPortMap2 |=  (uint64)  (pFromData)[SWAPBAQB(27)] << 11;
        (pToStruct)->m_nnPortMap2 |=  (uint64)  (pFromData)[SWAPBAQB(28)] << 19;
        (pToStruct)->m_nnPortMap2 |=  (uint64)  (pFromData)[SWAPBAQB(29)] << 27;
        **/

        COMPILER_64_SET(tmp, 0,(pFromData)[SWAPBAQB(30)]);
        COMPILER_64_SHL(tmp, 35);
        COMPILER_64_OR((pToStruct)->m_nnPortMap2, tmp);
        /* (pToStruct)->m_nnPortMap2 |=  (uint64)  (pFromData)[SWAPBAQB(30)] << 35; */

        COMPILER_64_SET(tmp, 0,((pFromData)[SWAPBAQB(31)] & 0x7f));
        COMPILER_64_SHL(tmp, 43);
        COMPILER_64_OR((pToStruct)->m_nnPortMap2, tmp);
        /* (pToStruct)->m_nnPortMap2 |=  (uint64)  ((pFromData)[SWAPBAQB(31)] & 0x7f) << 43; */

        /* Unpack Member: m_nMvtda2 */
        (pToStruct)->m_nMvtda2 =  (uint32)  ((pFromData)[SWAPBAQB(23)] >> 7) & 0x01;
        (pToStruct)->m_nMvtda2 |=  (uint32)  (pFromData)[SWAPBAQB(24)] << 1;
        (pToStruct)->m_nMvtda2 |=  (uint32)  ((pFromData)[SWAPBAQB(25)] & 0x1f) << 9;

        /* Unpack Member: m_nMvtdb2 */
        (pToStruct)->m_nMvtdb2 =  (uint32)  ((pFromData)[SWAPBAQB(23)] >> 3) & 0x0f;

        /* Unpack Member: m_nNext2 */
        (pToStruct)->m_nNext2 =  (uint32)  ((pFromData)[SWAPBAQB(21)] >> 3) & 0x1f;
        (pToStruct)->m_nNext2 |=  (uint32)  (pFromData)[SWAPBAQB(22)] << 5;
        (pToStruct)->m_nNext2 |=  (uint32)  ((pFromData)[SWAPBAQB(23)] & 0x07) << 13;

        /* Unpack Member: m_nKnockout2 */
        (pToStruct)->m_nKnockout2 =  (uint32)  ((pFromData)[SWAPBAQB(21)] >> 2) & 0x01;

        /* Unpack Member: m_nnPortMap1 */
        COMPILER_64_SET((pToStruct)->m_nnPortMap1, 0, (pFromData)[SWAPBAQB(15)]);
        /* (pToStruct)->m_nnPortMap1 =  (uint64)  (pFromData)[SWAPBAQB(15)] ; */

        COMPILER_64_SET(tmp, 0,(pFromData)[SWAPBAQB(16)]);
        COMPILER_64_SHL(tmp, 8);
        COMPILER_64_OR((pToStruct)->m_nnPortMap1, tmp);
        /* (pToStruct)->m_nnPortMap1 |=  (uint64)  (pFromData)[SWAPBAQB(16)] << 8; */

        COMPILER_64_SET(tmp, 0,(pFromData)[SWAPBAQB(17)]);
        COMPILER_64_SHL(tmp, 16);
        COMPILER_64_OR((pToStruct)->m_nnPortMap1, tmp);
        /* (pToStruct)->m_nnPortMap1 |=  (uint64)  (pFromData)[SWAPBAQB(17)] << 16; */

        COMPILER_64_SET(tmp, 0,(pFromData)[SWAPBAQB(18)]);
        COMPILER_64_SHL(tmp, 24);
        COMPILER_64_OR((pToStruct)->m_nnPortMap1, tmp);
        /* (pToStruct)->m_nnPortMap1 |=  (uint64)  (pFromData)[SWAPBAQB(18)] << 24; */

        COMPILER_64_SET(tmp, 0,((pFromData)[SWAPBAQB(19)]));
        COMPILER_64_SHL(tmp, 32);
        COMPILER_64_OR((pToStruct)->m_nnPortMap1, tmp);
        /* (pToStruct)->m_nnPortMap1 |=  (uint64)  (pFromData)[SWAPBAQB(19)] << 32; */

        COMPILER_64_SET(tmp, 0,((pFromData)[SWAPBAQB(20)]));
        COMPILER_64_SHL(tmp, 40);
        COMPILER_64_OR((pToStruct)->m_nnPortMap1, tmp);
        /* (pToStruct)->m_nnPortMap1 |=  (uint64)  (pFromData)[SWAPBAQB(20)] << 40; */

        COMPILER_64_SET(tmp, 0,((pFromData)[SWAPBAQB(21)] & 0x03));
        COMPILER_64_SHL(tmp, 48);
        COMPILER_64_OR((pToStruct)->m_nnPortMap1, tmp);
        /* (pToStruct)->m_nnPortMap1 |=  (uint64)  ((pFromData)[SWAPBAQB(21)] & 0x03) << 48; */

        /* Unpack Member: m_nMvtda1 */
        (pToStruct)->m_nMvtda1 =  (uint32)  ((pFromData)[SWAPBAQB(13)] >> 2) & 0x3f;
        (pToStruct)->m_nMvtda1 |=  (uint32)  (pFromData)[SWAPBAQB(14)] << 6;

        /* Unpack Member: m_nMvtdb1 */
        (pToStruct)->m_nMvtdb1 =  (uint32)  ((pFromData)[SWAPBAQB(12)] >> 6) & 0x03;
        (pToStruct)->m_nMvtdb1 |=  (uint32)  ((pFromData)[SWAPBAQB(13)] & 0x03) << 2;

        /* Unpack Member: m_nNext1 */
        (pToStruct)->m_nNext1 =  (uint32)  ((pFromData)[SWAPBAQB(10)] >> 6) & 0x03;
        (pToStruct)->m_nNext1 |=  (uint32)  (pFromData)[SWAPBAQB(11)] << 2;
        (pToStruct)->m_nNext1 |=  (uint32)  ((pFromData)[SWAPBAQB(12)] & 0x3f) << 10;

        /* Unpack Member: m_nKnockout1 */
        (pToStruct)->m_nKnockout1 =  (uint32)  ((pFromData)[SWAPBAQB(10)] >> 5) & 0x01;

        /* Unpack Member: m_nnPortMap0 */
        COMPILER_64_SET(tmp, 0,((pFromData)[SWAPBAQB(4)]));
        COMPILER_64_SHR(tmp, 3);
        COMPILER_64_SET(uuVal, 0, 0x1f);
        COMPILER_64_AND(tmp, uuVal);
        COMPILER_64_ZERO((pToStruct)->m_nnPortMap0);
        COMPILER_64_OR((pToStruct)->m_nnPortMap0, tmp);
        /* (pToStruct)->m_nnPortMap0 =  (uint64)  ((pFromData)[SWAPBAQB(4)] >> 3) & 0x1f; */

        COMPILER_64_SET(tmp, 0,((pFromData)[SWAPBAQB(5)]));
        COMPILER_64_SHL(tmp, 5);
        COMPILER_64_OR((pToStruct)->m_nnPortMap0, tmp);
        /* (pToStruct)->m_nnPortMap0 |=  (uint64)  (pFromData)[SWAPBAQB(5)] << 5; */

        COMPILER_64_SET(tmp, 0,((pFromData)[SWAPBAQB(6)]));
        COMPILER_64_SHL(tmp, 13);
        COMPILER_64_OR((pToStruct)->m_nnPortMap0, tmp);
        /* (pToStruct)->m_nnPortMap0 |=  (uint64)  (pFromData)[SWAPBAQB(6)] << 13; */

        COMPILER_64_SET(tmp, 0,((pFromData)[SWAPBAQB(7)]));
        COMPILER_64_SHL(tmp, 21);
        COMPILER_64_OR((pToStruct)->m_nnPortMap0, tmp);
        /* (pToStruct)->m_nnPortMap0 |=  (uint64)  (pFromData)[SWAPBAQB(7)] << 21; */

        COMPILER_64_SET(tmp, 0,((pFromData)[SWAPBAQB(8)]));
        COMPILER_64_SHL(tmp, 29);
        COMPILER_64_OR((pToStruct)->m_nnPortMap0, tmp);
        /* (pToStruct)->m_nnPortMap0 |=  (uint64)  (pFromData)[SWAPBAQB(8)] << 29; */

        COMPILER_64_SET(tmp, 0,((pFromData)[SWAPBAQB(9)]));
        COMPILER_64_SHL(tmp, 37);
        COMPILER_64_OR((pToStruct)->m_nnPortMap0, tmp);
        /* (pToStruct)->m_nnPortMap0 |=  (uint64)  (pFromData)[SWAPBAQB(9)] << 37; */

        COMPILER_64_SET(tmp, 0,((pFromData)[SWAPBAQB(10)] & 0x1f));
        COMPILER_64_SHL(tmp, 45);
        COMPILER_64_OR((pToStruct)->m_nnPortMap0, tmp);
        /* (pToStruct)->m_nnPortMap0 |=  (uint64)  ((pFromData)[SWAPBAQB(10)] & 0x1f) << 45; */

        /* Unpack Member: m_nMvtda0 */
        (pToStruct)->m_nMvtda0 =  (uint32)  ((pFromData)[SWAPBAQB(2)] >> 5) & 0x07;
        (pToStruct)->m_nMvtda0 |=  (uint32)  (pFromData)[SWAPBAQB(3)] << 3;
        (pToStruct)->m_nMvtda0 |=  (uint32)  ((pFromData)[SWAPBAQB(4)] & 0x07) << 11;

        /* Unpack Member: m_nMvtdb0 */
        (pToStruct)->m_nMvtdb0 =  (uint32)  ((pFromData)[SWAPBAQB(2)] >> 1) & 0x0f;

        /* Unpack Member: m_nNext0 */
        (pToStruct)->m_nNext0 =  (uint32)  ((pFromData)[SWAPBAQB(0)] >> 1) & 0x7f;
        (pToStruct)->m_nNext0 |=  (uint32)  (pFromData)[SWAPBAQB(1)] << 7;
        (pToStruct)->m_nNext0 |=  (uint32)  ((pFromData)[SWAPBAQB(2)] & 0x01) << 15;

        /* Unpack Member: m_nKnockout0 */
        (pToStruct)->m_nKnockout0 =  (uint32)  ((pFromData)[SWAPBAQB(0)] ) & 0x01;
    } else {
        /* Unpack Member: m_nReserved */
        (pToStruct)->m_nReserved =  (uint32)  ((pFromData)[31] >> 7) & 0x01;

        /* Unpack Member: m_nnPortMap2 */
        COMPILER_64_ZERO((pToStruct)->m_nnPortMap2);
        COMPILER_64_SET(tmp, 0,((pFromData)[25]));
        COMPILER_64_SHR(tmp, 5);
        COMPILER_64_SET(uuVal, 0, 0x07);
        COMPILER_64_AND(tmp, uuVal);
        COMPILER_64_OR((pToStruct)->m_nnPortMap2, tmp);

        COMPILER_64_SET(tmp, 0,((pFromData)[26]));
        COMPILER_64_SHL(tmp, 3);
        COMPILER_64_OR((pToStruct)->m_nnPortMap2, tmp);

        COMPILER_64_SET(tmp, 0,((pFromData)[27]));
        COMPILER_64_SHL(tmp, 11);
        COMPILER_64_OR((pToStruct)->m_nnPortMap2, tmp);

        COMPILER_64_SET(tmp, 0,((pFromData)[28]));
        COMPILER_64_SHL(tmp, 19);
        COMPILER_64_OR((pToStruct)->m_nnPortMap2, tmp);

        COMPILER_64_SET(tmp, 0, ((pFromData)[29]));
        COMPILER_64_SHL(tmp, 27);
        COMPILER_64_OR((pToStruct)->m_nnPortMap2, tmp);
        /**
        (pToStruct)->m_nnPortMap2 =  (uint64)  ((pFromData)[25] >> 5) & 0x07;
        (pToStruct)->m_nnPortMap2 |=  (uint64)  (pFromData)[26] << 3;
        (pToStruct)->m_nnPortMap2 |=  (uint64)  (pFromData)[27] << 11;
        (pToStruct)->m_nnPortMap2 |=  (uint64)  (pFromData)[28] << 19;
        (pToStruct)->m_nnPortMap2 |=  (uint64)  (pFromData)[29] << 27;
        **/

        COMPILER_64_SET(tmp, 0, (pFromData)[30]);
        COMPILER_64_SHL(tmp, 35);
        COMPILER_64_OR((pToStruct)->m_nnPortMap2, tmp);
        /* (pToStruct)->m_nnPortMap2 |=  (uint64)  (pFromData)[30] << 35; */

        COMPILER_64_SET(tmp, 0, ((pFromData)[31] & 0x7f));
        COMPILER_64_SHL(tmp, 43);
        COMPILER_64_OR((pToStruct)->m_nnPortMap2, tmp);
        /* (pToStruct)->m_nnPortMap2 |=  (uint64)  ((pFromData)[31] & 0x7f) << 43; */

        /* Unpack Member: m_nMvtda2 */
        (pToStruct)->m_nMvtda2 =  (uint32)  ((pFromData)[23] >> 7) & 0x01;
        (pToStruct)->m_nMvtda2 |=  (uint32)  (pFromData)[24] << 1;
        (pToStruct)->m_nMvtda2 |=  (uint32)  ((pFromData)[25] & 0x1f) << 9;

        /* Unpack Member: m_nMvtdb2 */
        (pToStruct)->m_nMvtdb2 =  (uint32)  ((pFromData)[23] >> 3) & 0x0f;

        /* Unpack Member: m_nNext2 */
        (pToStruct)->m_nNext2 =  (uint32)  ((pFromData)[21] >> 3) & 0x1f;
        (pToStruct)->m_nNext2 |=  (uint32)  (pFromData)[22] << 5;
        (pToStruct)->m_nNext2 |=  (uint32)  ((pFromData)[23] & 0x07) << 13;

        /* Unpack Member: m_nKnockout2 */
        (pToStruct)->m_nKnockout2 =  (uint32)  ((pFromData)[21] >> 2) & 0x01;

        /* Unpack Member: m_nnPortMap1 */
        COMPILER_64_SET((pToStruct)->m_nnPortMap1, 0, (pFromData)[15]);

        COMPILER_64_SET(tmp, 0, (pFromData)[16]);
        COMPILER_64_SHL(tmp, 8);
        COMPILER_64_OR((pToStruct)->m_nnPortMap1, tmp);

        COMPILER_64_SET(tmp, 0, (pFromData)[17]);
        COMPILER_64_SHL(tmp, 16);
        COMPILER_64_OR((pToStruct)->m_nnPortMap1, tmp);

        COMPILER_64_SET(tmp, 0, (pFromData)[18]);
        COMPILER_64_SHL(tmp, 24);
        COMPILER_64_OR((pToStruct)->m_nnPortMap1, tmp);

        /**
        (pToStruct)->m_nnPortMap1 =  (uint64)  (pFromData)[15] ;
        (pToStruct)->m_nnPortMap1 |=  (uint64)  (pFromData)[16] << 8;
        (pToStruct)->m_nnPortMap1 |=  (uint64)  (pFromData)[17] << 16;
        (pToStruct)->m_nnPortMap1 |=  (uint64)  (pFromData)[18] << 24;
        **/

        COMPILER_64_SET(tmp, 0, ((pFromData)[19]));
        COMPILER_64_SHL(tmp, 32);
        COMPILER_64_OR((pToStruct)->m_nnPortMap1, tmp);
        /* (pToStruct)->m_nnPortMap1 |=  (uint64)  (pFromData)[19] << 32; */

        COMPILER_64_SET(tmp, 0, ((pFromData)[20]));
        COMPILER_64_SHL(tmp, 40);
        COMPILER_64_OR((pToStruct)->m_nnPortMap1, tmp);
        /* (pToStruct)->m_nnPortMap1 |=  (uint64)  (pFromData)[20] << 40; */

        COMPILER_64_SET(tmp, 0, ((pFromData)[21] & 0x03));
        COMPILER_64_SHL(tmp, 48);
        COMPILER_64_OR((pToStruct)->m_nnPortMap1, tmp);
        /* (pToStruct)->m_nnPortMap1 |=  (uint64)  ((pFromData)[21] & 0x03) << 48; */

        /* Unpack Member: m_nMvtda1 */
        (pToStruct)->m_nMvtda1 =  (uint32)  ((pFromData)[13] >> 2) & 0x3f;
        (pToStruct)->m_nMvtda1 |=  (uint32)  (pFromData)[14] << 6;

        /* Unpack Member: m_nMvtdb1 */
        (pToStruct)->m_nMvtdb1 =  (uint32)  ((pFromData)[12] >> 6) & 0x03;
        (pToStruct)->m_nMvtdb1 |=  (uint32)  ((pFromData)[13] & 0x03) << 2;

        /* Unpack Member: m_nNext1 */
        (pToStruct)->m_nNext1 =  (uint32)  ((pFromData)[10] >> 6) & 0x03;
        (pToStruct)->m_nNext1 |=  (uint32)  (pFromData)[11] << 2;
        (pToStruct)->m_nNext1 |=  (uint32)  ((pFromData)[12] & 0x3f) << 10;

        /* Unpack Member: m_nKnockout1 */
        (pToStruct)->m_nKnockout1 =  (uint32)  ((pFromData)[10] >> 5) & 0x01;

        /* Unpack Member: m_nnPortMap0 */
        COMPILER_64_SET(tmp, 0, ((pFromData)[4]));
        COMPILER_64_SHR(tmp, 3);
        COMPILER_64_SET(uuVal, 0, 0x1f);
        COMPILER_64_AND(tmp, uuVal);
        COMPILER_64_ZERO((pToStruct)->m_nnPortMap0);
        COMPILER_64_OR((pToStruct)->m_nnPortMap0, tmp);

        COMPILER_64_SET(tmp, 0, ((pFromData)[5]));
        COMPILER_64_SHL(tmp, 5);
        COMPILER_64_OR((pToStruct)->m_nnPortMap0, tmp);

        COMPILER_64_SET(tmp, 0, ((pFromData)[6]));
        COMPILER_64_SHL(tmp, 13);
        COMPILER_64_OR((pToStruct)->m_nnPortMap0, tmp);

        COMPILER_64_SET(tmp, 0, ((pFromData)[7]));
        COMPILER_64_SHL(tmp, 21);
        COMPILER_64_OR((pToStruct)->m_nnPortMap0, tmp);

        COMPILER_64_SET(tmp, 0, ((pFromData)[8]));
        COMPILER_64_SHL(tmp, 29);
        COMPILER_64_OR((pToStruct)->m_nnPortMap0, tmp);

        /* (pToStruct)->m_nnPortMap0 =  (uint64)  ((pFromData)[4] >> 3) & 0x1f;
        (pToStruct)->m_nnPortMap0 |=  (uint64)  (pFromData)[5] << 5;
        (pToStruct)->m_nnPortMap0 |=  (uint64)  (pFromData)[6] << 13;
        (pToStruct)->m_nnPortMap0 |=  (uint64)  (pFromData)[7] << 21;
        (pToStruct)->m_nnPortMap0 |=  (uint64)  (pFromData)[8] << 29; */

        COMPILER_64_SET(tmp, 0, ((pFromData)[9]));
        COMPILER_64_SHL(tmp, 37);
        COMPILER_64_OR((pToStruct)->m_nnPortMap0, tmp);
        /* (pToStruct)->m_nnPortMap0 |=  (uint64)  (pFromData)[9] << 37; */

        COMPILER_64_SET(tmp, 0, ((pFromData)[10] & 0x1f));
        COMPILER_64_SHL(tmp, 45);
        COMPILER_64_OR((pToStruct)->m_nnPortMap0, tmp);
        /* (pToStruct)->m_nnPortMap0 |=  (uint64)  ((pFromData)[10] & 0x1f) << 45; */

        /* Unpack Member: m_nMvtda0 */
        (pToStruct)->m_nMvtda0 =  (uint32)  ((pFromData)[2] >> 5) & 0x07;
        (pToStruct)->m_nMvtda0 |=  (uint32)  (pFromData)[3] << 3;
        (pToStruct)->m_nMvtda0 |=  (uint32)  ((pFromData)[4] & 0x07) << 11;

        /* Unpack Member: m_nMvtdb0 */
        (pToStruct)->m_nMvtdb0 =  (uint32)  ((pFromData)[2] >> 1) & 0x0f;

        /* Unpack Member: m_nNext0 */
        (pToStruct)->m_nNext0 =  (uint32)  ((pFromData)[0] >> 1) & 0x7f;
        (pToStruct)->m_nNext0 |=  (uint32)  (pFromData)[1] << 7;
        (pToStruct)->m_nNext0 |=  (uint32)  ((pFromData)[2] & 0x01) << 15;

        /* Unpack Member: m_nKnockout0 */
        (pToStruct)->m_nKnockout0 =  (uint32)  ((pFromData)[0] ) & 0x01;
    }
}


/* initialize an instance of this zframe */
void
sandHalFrameHwQe2000FMVT_InitInstance(sandHalFrameHwQe2000FMVT_t *pFrame) {

  pFrame->m_nReserved =  (uint32)  0;
  COMPILER_64_ZERO(pFrame->m_nnPortMap2);
  pFrame->m_nMvtda2 =  (uint32)  0;
  pFrame->m_nMvtdb2 =  (uint32)  0;
  pFrame->m_nNext2 =  (uint32)  0;
  pFrame->m_nKnockout2 =  (uint32)  0;
  COMPILER_64_ZERO(pFrame->m_nnPortMap1);
  pFrame->m_nMvtda1 =  (uint32)  0;
  pFrame->m_nMvtdb1 =  (uint32)  0;
  pFrame->m_nNext1 =  (uint32)  0;
  pFrame->m_nKnockout1 =  (uint32)  0;
  COMPILER_64_ZERO(pFrame->m_nnPortMap0);
  pFrame->m_nMvtda0 =  (uint32)  0;
  pFrame->m_nMvtdb0 =  (uint32)  0;
  pFrame->m_nNext0 =  (uint32)  0;
  pFrame->m_nKnockout0 =  (uint32)  0;

}
