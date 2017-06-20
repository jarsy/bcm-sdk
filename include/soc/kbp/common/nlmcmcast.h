/*
 * $Id: nlmcmcast.h,v 1.2.8.1 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


 
#ifndef INCLUDED_NLMCMCAST_H
#define INCLUDED_NLMCMCAST_H

#ifndef __doxygen__

/* check for macro arg side effects */
#define NlmCmAssertStable_(name) NlmCmAssert_((name) == (name), "stable expression")
#define NlmCmAssertStable(name) NlmCmAssert((name) == (name), "stable expression")

/* detect unsafe size_t casts */
#define NlmCmSize_t__castUint32(val) \
    (NlmCmAssertStable_(val) \
     NlmCmAssert_((val) <= UINT32_MAX, "Illegal size_t cast") \
     (nlm_u32)val)

/* Define type-safe and value checking casting macros for common integral types */

/* nlm_8 => nlm_16, nlm_32, nlm_u8, nlm_u16, nlm_u32 */
#define NlmCmInt8__castInt16(val) \
    (NlmCmAssertStable_(val) NlmCmDbgOnly_(NlmCmInt8__Identity(val)) \
     (nlm_16)(val))

#define NlmCmInt8__castInt32(val) \
    (NlmCmAssertStable_(val) NlmCmDbgOnly_(NlmCmInt8__Identity(val)) \
     (nlm_32)(val))

#define NlmCmInt8__castUint8(val) \
    (NlmCmAssertStable_(val) NlmCmDbgOnly_(NlmCmInt8__Identity(val)) \
     NlmCmAssert_((val) >= 0, "value out of range") \
     (nlm_u8)(val))

#define NlmCmInt8__castUint16(val) \
    (NlmCmAssertStable_(val) NlmCmDbgOnly_(NlmCmInt8__Identity(val)) \
     NlmCmAssert_((val) >= 0, "value out of range") \
     (nlm_u16)(val))

#define NlmCmInt8__castUint32(val) \
    (NlmCmAssertStable_(val) NlmCmDbgOnly_(NlmCmInt8__Identity(val)) \
     NlmCmAssert_((val) >= 0, "value out of range") \
     (nlm_u32)(val))

/* nlm_16 => nlm_8, nlm_32, nlm_u8, nlm_u16, nlm_u32 */

/* N.B., the nlm_16 cast is to work around a quirk in a number
   of compilers. When passed an unsigned integer, some compilers
   silently zero-extend the signed constant resulting in erroneous
   checks against INT8_MIN.
*/
#define NlmCmInt16__castInt8(val) \
    (NlmCmAssertStable_(val) NlmCmDbgOnly_(NlmCmInt16__Identity(val)) \
     NlmCmAssert_((val) <= INT8_MAX, "value out of range") \
     NlmCmAssert_((nlm_16)(val) >= INT8_MIN, "value out of range") \
     (nlm_8)(val))

#define NlmCmInt16__castInt32(val) \
    (NlmCmAssertStable_(val) NlmCmDbgOnly_(NlmCmInt16__Identity(val)) \
     (nlm_32)(val))

#define NlmCmInt16__castUint8(val) \
    (NlmCmAssertStable_(val) NlmCmDbgOnly_(NlmCmInt16__Identity(val)) \
     NlmCmAssert_((val) >= 0, "value out of range") \
     NlmCmAssert_((nlm_u16)(val) <= UINT8_MAX, "value out of range") \
     (nlm_u8)(val))

#define NlmCmInt16__castUint16(val) \
    (NlmCmAssertStable_(val) NlmCmDbgOnly_(NlmCmInt16__Identity(val)) \
     NlmCmAssert_((val) >= 0, "value out of range") \
     (nlm_u16)(val))

#define NlmCmInt16__castUint32(val) \
    (NlmCmAssertStable_(val) NlmCmDbgOnly_(NlmCmInt16__Identity(val)) \
     NlmCmAssert_((val) >= 0, "value out of range") \
     (nlm_u32)(val))

/* nlm_32 => nlm_8, nlm_16, nlm_u8, nlm_u16, nlm_u32 */

/* N.B., the nlm_16 cast is to work around a quirk in a number
   of compilers. When passed an unsigned integer, some compilers
   silently zero-extend the signed constant resulting in erroneous
   checks against INT8_MIN.
*/
#define NlmCmInt32__castInt8(val) \
    (NlmCmAssertStable_(val) NlmCmDbgOnly_(NlmCmInt32__Identity(val)) \
     NlmCmAssert_((val) <= INT8_MAX, "value out of range") \
     NlmCmAssert_((nlm_32)(val) >= INT8_MIN, "value out of range") \
     (nlm_8)(val))
/* N.B., the nlm_16 cast is to work around a quirk in a number
   of compilers. When passed an unsigned integer, some compilers
   silently zero-extend the signed constant resulting in erroneous
   checks against INT16_MIN.
*/
#define NlmCmInt32__castInt16(val) \
    (NlmCmAssertStable_(val) NlmCmDbgOnly_(NlmCmInt32__Identity(val)) \
     NlmCmAssert_((val) <= INT16_MAX, "value out of range") \
     NlmCmAssert_((nlm_32)(val) >= INT16_MIN, "value out of range") \
     (nlm_16)(val))

#define NlmCmInt32__castUint8(val) \
    (NlmCmAssertStable_(val) NlmCmDbgOnly_(NlmCmInt32__Identity(val)) \
     NlmCmAssert_((val) >= 0, "value out of range") \
     NlmCmAssert_((nlm_u32)(val) <= UINT8_MAX, "value out of range") \
     (nlm_u8)(val))

#define NlmCmInt32__castUint16(val) \
    (NlmCmAssertStable_(val) NlmCmDbgOnly_(NlmCmInt32__Identity(val)) \
     NlmCmAssert_((val) >= 0, "value out of range") \
     NlmCmAssert_((nlm_u32)(val) <= UINT16_MAX, "value out of range") \
     (nlm_u16)(val))

#define NlmCmInt32__castUint32(val) \
    (NlmCmAssertStable_(val) NlmCmDbgOnly_(NlmCmInt32__Identity(val)) \
     NlmCmAssert_((val) >= 0, "value out of range") \
     (nlm_u32)(val))

/* nlm_u8 => nlm_8, nlm_16, nlm_32, nlm_u16, nlm_u32 */
#define NlmCmUint8__castInt8(val) \
    (NlmCmAssertStable_(val) NlmCmDbgOnly_(NlmCmUint8__Identity(val)) \
     NlmCmAssert_((val) <= (nlm_u8)INT8_MAX, "value out of range") \
     (nlm_8)(val))

#define NlmCmUint8__castInt16(val) \
    (NlmCmAssertStable_(val) NlmCmDbgOnly_(NlmCmUint8__Identity(val)) \
     (nlm_16)(val))

#define NlmCmUint8__castInt32(val) \
    (NlmCmAssertStable_(val) NlmCmDbgOnly_(NlmCmUint8__Identity(val)) \
     (nlm_32)(val))

#define NlmCmUint8__castUint16(val) \
    (NlmCmAssertStable_(val) NlmCmDbgOnly_(NlmCmUint8__Identity(val)) \
     (nlm_u16)(val))

#define NlmCmUint8__castUint32(val) \
    (NlmCmAssertStable_(val) NlmCmDbgOnly_(NlmCmUint8__Identity(val)) \
     (nlm_u32)(val))

/* nlm_u16 => nlm_8, nlm_16, nlm_32, nlm_u8, nlm_u32 */
#define NlmCmUint16__castInt8(val) \
    (NlmCmAssertStable_(val) NlmCmDbgOnly_(NlmCmUint16__Identity(val)) \
     NlmCmAssert_((val) <= (nlm_u16)INT8_MAX, "value out of range") \
     (nlm_8)(val))

#define NlmCmUint16__castInt16(val) \
    (NlmCmAssertStable_(val) NlmCmDbgOnly_(NlmCmUint16__Identity(val)) \
     NlmCmAssert_((val) <= (nlm_u16)INT16_MAX, "value out of range") \
     (nlm_16)(val))

#define NlmCmUint16__castInt32(val) \
    (NlmCmAssertStable_(val) NlmCmDbgOnly_(NlmCmUint16__Identity(val)) \
     (nlm_32)(val))

#define NlmCmUint16__castUint8(val) \
    (NlmCmAssertStable_(val) NlmCmDbgOnly_(NlmCmUint16__Identity(val)) \
     NlmCmAssert_((val) <= UINT8_MAX, "value out of range") \
     (nlm_u8)(val))

#define NlmCmUint16__castUint32(val) \
    (NlmCmAssertStable_(val) NlmCmDbgOnly_(NlmCmUint16__Identity(val)) \
     (nlm_u32)(val))

/* nlm_u32 => nlm_8, nlm_16, nlm_32, int, nlm_u8, nlm_u16, unsigned int */
#define NlmCmUint32__castInt8(val) \
    (NlmCmAssertStable_(val) NlmCmDbgOnly_(NlmCmUint32__Identity(val)) \
     NlmCmAssert_((val) <= (nlm_u8)INT8_MAX, "value out of range") \
     (nlm_8)(val))

#define NlmCmUint32__castInt16(val) \
    (NlmCmAssertStable_(val) NlmCmDbgOnly_(NlmCmUint32__Identity(val)) \
     NlmCmAssert_((val) <= (nlm_u16)INT16_MAX, "value out of range") \
     (nlm_16)(val))

#define NlmCmUint32__castInt32(val) \
    (NlmCmAssertStable_(val) NlmCmDbgOnly_(NlmCmUint32__Identity(val)) \
     NlmCmAssert_((val) <= (nlm_u32)INT32_MAX, "value out of range") \
     (nlm_32)(val))

#define NlmCmUint32__castUint8(val) \
    (NlmCmAssertStable_(val) NlmCmAssert_( \
     NlmCmUint32__Identity(val) <= UINT8_MAX, "value out of range") \
     (nlm_u8)(val))

#define NlmCmUint32__castUint16(val) \
    (NlmCmAssertStable_(val) NlmCmAssert_( \
     NlmCmUint32__Identity(val) <= UINT16_MAX, "value out of range") \
     (nlm_u16)(val))

#define NlmCmVoidStar__castUintVS(val) \
    ((uintvs_t)NlmCmVoidStar__Identity(val))

#define NlmCmUintVS__castVoidStar(val) \
    ((void*)(NlmCmUintVS__Identity(val)))

#define NlmCmVoidStar__castUint8Star(val) \
    ((nlm_u8*)NlmCmVoidStar__Identity(val))


#define NlmCmDouble__castUint32(val) \
    (NlmCmAssertStable_(val) \
    NlmCmAssert_((val) >= 0 && (val) <=UINT32_MAX,"value out of range.") \
    (nlm_u32)(val))

#define NlmCmUint32__castDouble(val) \
    (NlmCmAssertStable_(val) \
    NlmCmAssert_(sizeof(nlm_u32)<=sizeof(double),"possible value out of range.") \
    (double)(val))

/*================================================================*/

#define NlmCmUint8Star__castCharStar(val) \
    ((char*)NlmCmUint8Star__Identity(val))

#define NlmCmCharStar__castUint8Star(val) \
    ((nlm_u8*)NlmCmCharStar__Identity(val))

#define NlmCmCharStar__constcast(val) \
    ( NlmCmDbgOnly_(NlmCmConstCharStar__Identity(val)) (char*)(val) )

/*================================================================*/

#endif /*__doxygen__*/


#endif

/*[]*/












