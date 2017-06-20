/*
 * $Id: nlmcmutility.h,v 1.2.8.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


 
#ifndef INCLUDED_NLMCM_UTILITY_H
#define INCLUDED_NLMCM_UTILITY_H

#ifndef NLMPLATFORM_BCM
#include "nlmcmbasic.h"
#include "nlmcmallocator.h"
#include "nlmcmexterncstart.h"
#else
#include <soc/kbp/common/nlmcmbasic.h>
#include <soc/kbp/common/nlmcmallocator.h>
#include <soc/kbp/common/nlmcmexterncstart.h>
#endif

#define NLMCM_NUM_BYTES_PER_80_BITS 10
#define NLMCM_BITS_IN_REG           80
/*------------------FUNCTION FOR COMMON---------------------*/
extern
nlm_u32 ReadBitsInRegs(nlm_u8* RegOf80Bits,
                     nlm_u8 end, 
                     nlm_u8 start
                     );
extern                   
void WriteBitsInRegs(nlm_u8* RegOf80Bits,
                     nlm_u8 end, 
                     nlm_u8 start,
                     nlm_u32 value);
extern
void WriteBitsInRegsAndWrapAround(nlm_u8* RegOf80Bits,
                     nlm_u8 len, 
                     nlm_u8 start,
                     nlm_u32 value,
                     nlm_u8 wrapBoundary);
extern 
void PrintReginNibbles(nlm_u8* RegOf80Bits );

/*----------------------------------------------------------------*/


/*----------------------------------------------------------------*/
extern
nlm_u32 ReadBitsInArrray(
                         nlm_u8* Arr,
                         nlm_u32 ArrSize,
                         nlm_u32 EndPos,
                         nlm_u32 StartPos
                         );

extern
void WriteBitsInArray(
                      nlm_u8* Arr,
                      nlm_u32 ArrSize,
                      nlm_u32 EndPos,
                      nlm_u32 StartPos,
                      nlm_u32 Value
                      );
/*----------------------------------------------------------------*/

extern 
void NlmCm__FillOnes(
                     nlm_u8 *data, 
                     nlm_u32 datalen,
                     nlm_u32 startBit,
                     nlm_u32 endBit
                     );

extern 
void NlmCm__FillZeroes(
                       nlm_u8 *data, 
                       nlm_u32 datalen,
                       nlm_u32 startBit,
                       nlm_u32 endBit
                       );

extern 
void NlmCm__CopyData(    
            nlm_u8 *o_data,
            nlm_u8 *data, 
            nlm_u32 datalen,
            nlm_u32 startBit,
            nlm_u32 endBit
            );

void WriteBitsInByte(
        nlm_u8* targetByte_p,
        nlm_u8 sourceByte,
        nlm_u8  startBit,
        nlm_u8  endBit);


void WriteFirstNBytes(
    nlm_u8* arr,
    nlm_u32 numBytes,
    nlm_u32 Value);


nlm_u64 
LongReadBitsInArray(
            nlm_u8* Arr,
            nlm_u32 ArrSize,
            nlm_u32 EndPos,
            nlm_u32 StartPos
            );

void 
LongWriteBitsInArray(
            nlm_u8* Arr,
            nlm_u32 ArrSize,
            nlm_u32 EndPos,
            nlm_u32 StartPos,
            nlm_u64 Value
            );



void
FlipBitsInArray(
    nlm_u8 *data_p,
    nlm_u32 totalNumBytes,
    nlm_u32 endPos,
    nlm_u32 startPos);

#ifndef NLMPLATFORM_BCM
#include "nlmcmexterncend.h"
#else
#include <soc/kbp/common/nlmcmexterncend.h>
#endif

#endif /* INCLUDED_NLMCM_UTILITY_H */
/**/










