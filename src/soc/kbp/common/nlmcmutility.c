/*
 * $Id: nlmcmutility.c,v 1.2.8.1 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


 
#include "nlmcmutility.h"

/* 
This array has precalculated the mask value to be used
while trying to select a portion of the bits in the byte.
BitMaskArray[startBit][endBit] where startBit >= endBit
Suppose we want to get the bit mask for reading only bit 7 to bit 6
then we get the value from the array by looking up 
BitMaskArray[7][6] = 192 = 1100 0000
So only bit 7 and bit 6 are enabled
*/
nlm_u8 BitMaskArray[8][8] = { 
    {1,0,0,0,0,0,0,0},
    {3,2,0,0,0,0,0,0},
    {7,6,4,0,0,0,0,0},
    {15,14,12,8,0,0,0,0},
    {31,30,28,24,16,0,0,0},
    {63,62,60,56,48,32,0,0},
    {127,126,124,120,112,96,64,0},
    {255,254,252,248,240,224,192,128}
};



/*-----------------FUNCTIONS TO BE COPIED TO COMMON------------------ */

void WriteBitsInRegsAndWrapAround(nlm_u8* RegOf80Bits,
                     nlm_u8 len, 
                     nlm_u8 start,
                     nlm_u32 value,
                     nlm_u8 wrapBoundary)
{

    NlmCmAssert((start <= NLMCM_BITS_IN_REG ), "Invalid start and ending position value!");
    NlmCmAssert((wrapBoundary >= start), "Wrapping Position cannot be before start !");
    NlmCmAssert(((len -(wrapBoundary - start +1)) <= start), "Wrapping is causing Data to be over-written  !");

    if(wrapBoundary >= start+len-1 )
        WriteBitsInRegs(RegOf80Bits,(start+len-1), start, value);
    else
    {
        nlm_u32 max_val;
        nlm_u8 bit;
        nlm_u16 curr_bit_pos;
        nlm_u8 posn_in_this_byte;
        nlm_u16 actual_idx;
        nlm_u8 mask_data;
        nlm_u8 iter;
        NlmCmAssert((len <= 32), "Can not write more than 32-bit at a time !");
        
        if(len!=32)
        {
            max_val = (1 << len);
            max_val--;
        }
        else
            max_val = 0xffffffff;
        NlmCmAssert((value <= max_val), "Value is to big to write in the bit field!");
        
        curr_bit_pos = start;
        for(iter=0 ; iter < len ; iter++ )
        {
            bit =(nlm_u8) value & 1;
            actual_idx = (NLMCM_NUM_BYTES_PER_80_BITS - 1) - (curr_bit_pos/8);
            posn_in_this_byte = (nlm_u8) curr_bit_pos % 8;
            
            
            mask_data = 1;
            mask_data <<= posn_in_this_byte;
            
            if(bit)
            {
                RegOf80Bits[actual_idx] |= mask_data;   /* Writing the bit equal to 1 */
            }
            else
            {
                mask_data ^= 255;
                RegOf80Bits[actual_idx] &= mask_data;   /* Writing the bit equal to 0 */
            
            }
            value >>= 1;
            
            if(curr_bit_pos == wrapBoundary)
                curr_bit_pos = 0;
            else
                curr_bit_pos++;
        }
    
    }
}

void WriteBitsInRegs(nlm_u8* RegOf80Bits,
                     nlm_u8 end, 
                     nlm_u8 start,
                     nlm_u32 value)
{
    nlm_u16 len;
    nlm_u32 max_val;
    nlm_u8 bit;
    nlm_u16 curr_bit_pos;
    nlm_u8 posn_in_this_byte;
    nlm_u16 actual_idx;
    nlm_u8 mask_data;
    
    len = end - start +1;
    NlmCmAssert((end<=NLMCM_BITS_IN_REG && start<=end), "Invalid start and ending position value!");
    NlmCmAssert((len <= 32), "Can not write more than 32-bit at a time !");
    
    if(len!=32)
    {
        max_val = (1 << len);
        max_val--;
    }
    else
        max_val = 0xffffffff;
    NlmCmAssert((value <= max_val), "Value is to big to write in the bit field!");
    
    curr_bit_pos = start;
        
    for( ; curr_bit_pos <= end ; curr_bit_pos++ )
    {
        bit =(nlm_u8) value & 1;
        actual_idx = (NLMCM_NUM_BYTES_PER_80_BITS - 1) - (curr_bit_pos/8);
        posn_in_this_byte = (nlm_u8) curr_bit_pos % 8;
        
        
        mask_data = 1;
        mask_data <<= posn_in_this_byte;
        
        if(bit)
        {
            RegOf80Bits[actual_idx] |= mask_data;   /* Writing the bit equal to 1 */
        }
        else
        {
            mask_data ^= 255;
            RegOf80Bits[actual_idx] &= mask_data;   /* Writing the bit equal to 0 */
        
        }
        value >>= 1;
    }
}


nlm_u32 ReadBitsInRegs(nlm_u8* RegOf80Bits,
                     nlm_u8 end, 
                     nlm_u8 start
                     )
{
    nlm_u32 value;
    nlm_u16 len;
    nlm_u8 bit;
    nlm_u16 curr_bit_pos;
    nlm_u16 curr_bit_pos_in_value;
    nlm_u8 posn_in_this_byte;
    nlm_u16 actual_idx;
    nlm_u32 mask_data;
    
    len = end - start +1;
    NlmCmAssert((len <= 32), "Can not write more than 32-bit at a time !");
            
    curr_bit_pos = start;
    curr_bit_pos_in_value = 0;
    value = 0;
    for( ; curr_bit_pos <= end ; curr_bit_pos++ )
    {
        
        actual_idx = (NLMCM_NUM_BYTES_PER_80_BITS - 1) - (curr_bit_pos/8);
        posn_in_this_byte = (nlm_u8) curr_bit_pos % 8;
        bit =(nlm_u8) ((RegOf80Bits[actual_idx]>>posn_in_this_byte)&1);
        
        mask_data = (1<< curr_bit_pos_in_value);
                
        if(bit)
        {
            
            value |= mask_data; /* Writing the bit equal to 1 */
        }
        else
        {
            mask_data ^= 0xffffffff;
            value &= mask_data; /* Writing the bit equal to 0 */
        
        }        
        curr_bit_pos_in_value++;
    }
    return value;
}
/* ------------------------------------------------------------------- */

nlm_u32 ReadBitsInArrray(
                         nlm_u8* Arr,
                         nlm_u32 ArrSize,
                         nlm_u32 EndPos,
                         nlm_u32 StartPos
                         )
{
    nlm_u32 value = 0;
    nlm_u32 len;
    nlm_u32 curr_bit_pos, curr_byte_pos;
    nlm_u32 mask_data = (nlm_u32) ~0;
    nlm_32  counter = 0, total_bits = 0;

    curr_byte_pos = ArrSize - 1 - (EndPos >> 3);
    curr_bit_pos = 7 - (EndPos & 0x7);

    len = EndPos - StartPos +1;
    counter = len + curr_bit_pos;
    if(len != 32)
        mask_data = ~(~0 << len);
    while(counter > 0 && total_bits < 32)
    {
        value = (value << 8) |(Arr[curr_byte_pos++]);
        counter -= 8;
        total_bits +=8;
    }

    if(len + curr_bit_pos > 32)
    {
        value <<= 8 - (StartPos & 0x7);
        value |= ((Arr[curr_byte_pos] >> (StartPos & 0x7)) & 0xFF);
    }
    else
    {
        value = value >> (StartPos & 0x7);
    }

    return (value & mask_data);
}


void WriteBitsInArray(
                      nlm_u8* Arr,
                      nlm_u32 ArrSize,
                      nlm_u32 EndPos,
                      nlm_u32 StartPos,
                      nlm_u32 Value
                      )
{   
    nlm_u32 startByteIdx;
    nlm_u32 endByteIdx;
    nlm_u32 byte;
    nlm_u32 len;
    nlm_u8 maskValue;   

    NlmCmAssert((EndPos >= StartPos), "endbit cannot be less than startbit");
    NlmCmAssert((EndPos < (ArrSize << 3)), "endBit exceeds the arr size");  

    len = (EndPos - StartPos + 1);
    NlmCmAssert((len <= 32), "Can not write more than 32-bit at a time !"); 
    /* Value is unsigned 32 bit variable, so it can not be greater than ~0.*/
    if(len != 32)
    {
    NlmCmAssert(Value <= (nlm_u32) (~(~0 << len)), "Value is to big to write in the bit field!");
    }

    startByteIdx = ArrSize - ((StartPos >> 3) + 1);
    endByteIdx = ArrSize - ((EndPos >> 3) + 1);  

    if(startByteIdx == endByteIdx)
    {        
        maskValue = (nlm_u8)(0xFE << ((EndPos  & 7)));
        maskValue |= ((1 << (StartPos  & 7)) - 1);
        Arr[startByteIdx] &= maskValue;
        Arr[startByteIdx] |= (nlm_u8)((Value << (StartPos  & 7)));
        return;
    }     
    if(StartPos & 7)
    {  
        maskValue = (nlm_u8)((1 << (StartPos  & 7)) - 1);
        Arr[startByteIdx] &= maskValue;
        Arr[startByteIdx] |= (nlm_u8)((Value << (StartPos & 7)));        
        startByteIdx--;
        Value >>= (8 - (StartPos  & 7));
    }
    for(byte = startByteIdx; byte > endByteIdx; byte--)
    {
        Arr[byte] = (nlm_u8)(Value);
        Value >>= 8;
    }
    maskValue = (nlm_u8)(0xFE << ((EndPos & 7)));
    Arr[byte] &= maskValue;
    Arr[byte] |= Value;    

}




/* ------------------------------------------------------------------- */
void PrintReginNibbles(nlm_u8* RegOf80Bits )
{
nlm_u8 i;
for(i = 0; i< 10 ; i++)
{
    NlmCm__printf( "%X",(RegOf80Bits[i])>>4 );
    NlmCm__printf( "%X ",(RegOf80Bits[i] & (0x0f)) );

}
NlmCm__printf("\n");
}


/* Fills ones in a column of an array where
startBit indicates start of the column and 
endBit indicates end of the column; datalen
gives the size of array in terms of bytes;
Other bits in the array remain unchanged*/
void NlmCm__FillOnes(
    nlm_u8 *data, 
    nlm_u32 datalen,
    nlm_u32 startBit,
    nlm_u32 endBit
    )
{
    nlm_u32 numOfBytes;
    nlm_u32 startByte;    

    NlmCmAssert((data != NULL), "Null Data Pointer !"); 
    NlmCmAssert(((startBit <= (datalen << 3)) && endBit <= startBit), 
        "Invalid start and ending position value!");    

    startByte = ((startBit + 8) >> 3); 
    numOfBytes = (((startBit - endBit) + 8) >> 3);   

    if(numOfBytes == 1)
    {
        WriteBitsInArray(data, datalen, startBit, endBit, 
            (~(0xFFFFFFFF << ((startBit - endBit) +  1))));
        return;
    }
    
    if((startBit & 0x7) != 7)
    {
        WriteBitsInArray(&data[datalen - startByte], 1, (startBit & 0x7),
            0, (~(0xFFFFFFFF << ((startBit + 1) & 0x7))));
        startByte--;
        numOfBytes--;
    }
    if(numOfBytes > 1)
    {
        NlmCm__memset(&data[datalen - startByte], 0xff, numOfBytes - 1);

        startByte = startByte + 1 - numOfBytes;                
    }    
    if(endBit < (startByte << 3))
    {
        WriteBitsInArray(data, datalen, (startByte << 3) - 1, 
            endBit, (~(0xFFFFFFFF << ((startByte << 3) - endBit))));
    }
}    


/* Fills zeroes in a column of an array where
startBit indicates start of the column and 
endBit indicates end of the column; datalen
gives the size of array in terms of bytes;
Other bits in the array remain unchanged*/
void NlmCm__FillZeroes(
    nlm_u8 *data, 
    nlm_u32 datalen,
    nlm_u32 startBit,
    nlm_u32 endBit
    )
{
    nlm_u32 numOfBytes;
    nlm_u32 startByte;        

    NlmCmAssert((data != NULL), "Null Data Pointer !"); 
    NlmCmAssert(((startBit <= (datalen << 3)) && endBit <= startBit), 
        "Invalid start and ending position value!");    
       
    startByte = ((startBit + 8) >> 3); 
    numOfBytes = (((startBit - endBit) + 8) >> 3);    
    
    if(numOfBytes == 1)
    {
        WriteBitsInArray(data, datalen, startBit, endBit, 0);
        return;
    }
    
    if((startBit & 0x7) != 7)
    {
        WriteBitsInArray(&data[datalen - startByte], 1, 
                         (startBit & 0x7), 0, 0);
        startByte--;
        numOfBytes--;
    }
    if(numOfBytes > 1)
    {
        NlmCm__memset(&data[datalen - startByte], 0x0, numOfBytes - 1);

        startByte = startByte + 1 - numOfBytes;                
    }    
    if(endBit < (startByte << 3))
    {
        WriteBitsInArray(data, datalen, (startByte << 3) - 1, 
                         endBit, 0);
    }
}

/* Copies the data specified by array(data) to the column
in the array specified by array(o_data) where
startBit indicates start of the column and 
endBit indicates end of the column; datalen
gives the size of array in terms of bytes;
Other bits in the array(o_data) remain unchanged */
void NlmCm__CopyData(    
    nlm_u8 *o_data,
    nlm_u8 *data, 
    nlm_u32 datalen,
    nlm_u32 startBit,
    nlm_u32 endBit
    )
{
    nlm_u32 numOfBytes;
    nlm_u32 startByte;        

    NlmCmAssert((o_data != NULL), "Null Output Data Pointer !"); 
    NlmCmAssert((data != NULL), "Null Data Pointer !"); 
    NlmCmAssert(((startBit <= (datalen << 3)) && endBit <= startBit), 
        "Invalid start and ending position value!");    
        
    startByte = ((startBit + 8) >> 3); 
    numOfBytes = (((startBit - endBit) + 8) >> 3);    

    if(numOfBytes == 1)
    {
        WriteBitsInArray(o_data, datalen, startBit, endBit, 
                         ReadBitsInArrray(data, datalen, 
                                          startBit, endBit));
        return;
    }

    if((startBit & 0x7) != 7)
    {
        WriteBitsInArray(&o_data[datalen - startByte], 1, (startBit & 0x7), 0,
                         ReadBitsInArrray(&data[datalen - startByte], 1, 
                                          (startBit & 0x7), 0));
        startByte--;
        numOfBytes--;
    }
    
    if(numOfBytes > 1)
    {
        NlmCm__memcpy(&o_data[datalen - startByte], &data[datalen - startByte], 
                      numOfBytes - 1);

        startByte = startByte + 1 - numOfBytes;                
    }    
    if(endBit < (startByte << 3))
    {
        WriteBitsInArray(o_data, datalen, (startByte << 3) - 1, endBit,
                         ReadBitsInArrray(data, datalen, 
                                        (startByte << 3) - 1, endBit));
    }
}


/* From the source byte, the bits between startBit and endBit are
written into the targetByte at the same location*/
void WriteBitsInByte(nlm_u8* targetByte_p,
                  nlm_u8 sourceByte,
                  nlm_u8  startBit,
                  nlm_u8  endBit)
{
    
    nlm_u8 maskValue = BitMaskArray[startBit][endBit];
    
    sourceByte &= maskValue;
    *targetByte_p &= ~maskValue;
    *targetByte_p |= sourceByte;
    
}



void WriteFirstNBytes(
    nlm_u8* arr,
    nlm_u32 numBytes,
    nlm_u32 Value)
{
    nlm_u32 i = 0;
    nlm_32 shift = 24;

    NlmCmAssert(numBytes <= sizeof(Value), "NumBytes should be <= 4 \n");

    while(numBytes != 0)
    {
        arr[i] = (nlm_u8) (Value  >> shift);  
        ++i;
        --numBytes;
        shift -= 8;
    }
}



nlm_u64 
LongReadBitsInArray(
            nlm_u8* Arr,
            nlm_u32 ArrSize,
            nlm_u32 EndPos,
            nlm_u32 StartPos
            )
{
    nlm_u64 value = 0;
    nlm_u32 len;
    nlm_u32 curr_bit_pos, curr_byte_pos;
    nlm_u64 mask_data =  ~((nlm_u64)0);
    nlm_32  counter = 0, total_bits = 0;

    curr_byte_pos = ArrSize - 1 - (EndPos >> 3);
    curr_bit_pos = 7 - (EndPos & 0x7);

    len = EndPos - StartPos +1;
    counter = len + curr_bit_pos;
    if(len != 64)
        mask_data = ~( (~(nlm_u64)0) << len);

    while(counter > 0 && total_bits < 64)
    {
        value = (value << 8) |(Arr[curr_byte_pos++]);
        counter -= 8;
        total_bits +=8;
    }

    if(len + curr_bit_pos > 64)
    {
        value <<= 8 - (StartPos & 0x7);
        value |= ((Arr[curr_byte_pos] >> (StartPos & 0x7)) & 0xFF);
    }
    else
    {
        value = value >> (StartPos & 0x7);
    }

    return (value & mask_data);
}



void 
LongWriteBitsInArray(
            nlm_u8* Arr,
            nlm_u32 ArrSize,
            nlm_u32 EndPos,
            nlm_u32 StartPos,
            nlm_u64 Value
            )
{   
    nlm_u32 startByteIdx;
    nlm_u32 endByteIdx;
    nlm_u32 byteIdx;
    nlm_u32 len;
    nlm_u8 maskValue;   

    NlmCmAssert((EndPos >= StartPos), "endbit cannot be less than startbit");
    NlmCmAssert((EndPos < (ArrSize << 3)), "endBit exceeds the arr size");  

    len = (EndPos - StartPos + 1);
    NlmCmAssert((len <= 64), "Can not write more than 32-bit at a time !"); 
    /* Value is unsigned 32 bit variable, so it can not be greater than ~0.*/
    if(len != 64)
    {
    NlmCmAssert(Value <= (~(~((nlm_u64)0) << len)), "Value is to big to write in the bit field!");
    }

    startByteIdx = ArrSize - ((StartPos >> 3) + 1);
    endByteIdx = ArrSize - ((EndPos >> 3) + 1);  

    if(startByteIdx == endByteIdx)
    {        
        maskValue = (nlm_u8)(0xFE << ((EndPos  & 7)));
        maskValue |= ((1 << (StartPos  & 7)) - 1);
        Arr[startByteIdx] &= maskValue;
        Arr[startByteIdx] |= (nlm_u8)((Value << (StartPos  & 7)));
        return;
    }     
    if(StartPos & 7)
    {  
        maskValue = (nlm_u8)((1 << (StartPos  & 7)) - 1);
        Arr[startByteIdx] &= maskValue;
        Arr[startByteIdx] |= (nlm_u8)((Value << (StartPos & 7)));        
        startByteIdx--;
        Value >>= (8 - (StartPos  & 7));
    }
    for(byteIdx = startByteIdx; byteIdx > endByteIdx; byteIdx--)
    {
        Arr[byteIdx] = (nlm_u8)(Value);
        Value >>= 8;
    }
    maskValue = (nlm_u8)(0xFE << ((EndPos & 7)));
    Arr[byteIdx] &= maskValue;
    Arr[byteIdx] |= Value;    

}



void
FlipBitsInArray(
    nlm_u8 *data_p,
    nlm_u32 totalNumBytes,
    nlm_u32 endPos,
    nlm_u32 startPos)
{
    nlm_u8  maskValue = 0xFF;
    nlm_u8 numBitsInByte = 0, numBitsToFlip = 0;
    nlm_u32 startByteIdx = 0, endByteIdx = 0, byteIdx = 0;

    
    startByteIdx = totalNumBytes - ((startPos >> 3) + 1);
    
    endByteIdx = totalNumBytes - ((endPos >> 3) + 1);  

    if(startByteIdx == endByteIdx)
    {
        numBitsInByte = (nlm_u8)((endPos & 7) - (startPos & 7) + 1);

        maskValue = (nlm_u8)(0xFF >> (8 - numBitsInByte) );
        
        maskValue =  (nlm_u8)(maskValue << (startPos  & 7));
        
        data_p[startByteIdx] ^= maskValue;

        return ;
    }

    /*Flip the first byte */
    
    maskValue = 0xFF;

    maskValue = (nlm_u8)(maskValue << (startPos & 7));
    
    data_p[startByteIdx] ^= maskValue;
    
    startByteIdx--;


    
    for(byteIdx = startByteIdx; byteIdx > endByteIdx; byteIdx--)
    {
        data_p[byteIdx] ^= 0xFF;
    }


    /*Flip the last byte */
    
    numBitsToFlip = (nlm_u8)((endPos & 7) + 1);

    maskValue = (nlm_u8)( 0xFF >>  (8 - numBitsToFlip));
    
    data_p[byteIdx] ^= maskValue;




    return ;

}




/**/





