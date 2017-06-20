/*
 * $Id: nlmcmcrc.c,v 1.2.8.1 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


 

#include "nlmcmcrc.h"
#include "nlmcmutility.h"


static nlm_u32  g_crcTable[256];

            
/* The actual polynomial for CRC-10 is 0x633. The MS bit will be right shifted out. So we 
skip the most significant 1 and write only 0x233
*/
static const nlm_u32 CRC10_POLYNOMIAL   = (0x233);
static const nlm_u32 CRC10_MASK         = (0x3FF);
static const nlm_u32 CRC10_WIDTH        = (10);




void
NlmCm__ComputeCRC10Table(void)
{
    nlm_u32 remainder = 0;
    nlm_u32 dividend = 0;
    nlm_u32 iter = 0;
    nlm_u32 width = CRC10_WIDTH;
    nlm_u32 topBit = 0;

    /*Compute the remainder for each possible 1-byte dividend */
    for (dividend = 0; dividend < 256; ++dividend)
    {
        /*Add zeroes in the LS portion */
        remainder = dividend << (width - 8);
        
        for (iter = 8; iter > 0; --iter)
        {
            topBit = (1 << (width - 1));

            if (remainder & topBit)
            {
                remainder = (remainder << 1) ^ CRC10_POLYNOMIAL;
            }
            else
            {
                remainder = (remainder << 1);
            }
        }
        
        g_crcTable[dividend] = remainder & CRC10_MASK;
    }

}


nlm_u32
NlmCm__NormalCRC10(
    nlm_u8 *data_p,
    nlm_u32 numBytes)
{
    nlm_u32 bitNr = 0 ;
    nlm_u32 bitVal = 0;
    nlm_u32 remainder = 0;
    nlm_u32 endBitPos = (numBytes * 8) -1;
    nlm_u32 posOfBitToShiftIn = endBitPos - CRC10_WIDTH - 1;

    remainder = ReadBitsInArrray(data_p, numBytes, endBitPos, endBitPos - CRC10_WIDTH ); 

    for (bitNr = numBytes*8 - 1; bitNr >= CRC10_WIDTH; --bitNr)
    {
        /* If the bit is a 1 */
        if (remainder & (1u << CRC10_WIDTH))
        {
            /* XOR the previous remainder with the divisor */
            remainder ^= CRC10_POLYNOMIAL;
        }

        if(bitNr == CRC10_WIDTH)
            break;

        /* Shift the next bit of the message into the remainder*/
        remainder = (remainder << 1);

        bitVal = ReadBitsInArrray(data_p, numBytes, posOfBitToShiftIn, posOfBitToShiftIn);

        if(bitVal)
            remainder = remainder | 1;

        --posOfBitToShiftIn;
    }

    remainder = (remainder & CRC10_MASK);

    return remainder;
}



nlm_u32
NlmCm__FastCRC10(
    nlm_u8 *data_p,
    nlm_u32 numBytes)
{
   nlm_u32  remainder = 0;
   nlm_u32 i = 0;
   nlm_u32 width = CRC10_WIDTH;
   nlm_u32 rightShift = width - 8;
   nlm_u8 t = 0;
   
   
   for(i = 0; i < numBytes; ++i)
   {
      t = (nlm_u8) ((remainder >> rightShift) & 0xFF);
      remainder = (remainder << 8) | *data_p++ ;
      remainder = remainder ^ g_crcTable[t];
   }

   remainder = remainder & CRC10_MASK;

  
   return remainder;
}


