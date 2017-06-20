/*
    Copyright 2001, Broadcom Corporation
    All Rights Reserved.
    
    This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
    the contents of this file may not be disclosed to third parties, copied or
    duplicated in any form, in whole or in part, without the prior written
    permission of Broadcom Corporation.
*/
/*******************************************************************************
 * $Id: bcmendian.h,v 1.1 2004/02/24 07:47:00 csm Exp $
 * local version of endian.h - byte order defines
 ******************************************************************************/

#ifndef _BCMENDIAN_H_
#define _BCMENDIAN_H_

#include <typedefs.h>

/* Byte swap a 16 bit value */
#define BCMSWAP16(val) \
	((uint16)( \
		(((uint16)(val) & (uint16)0x00ffU) << 8) | \
		(((uint16)(val) & (uint16)0xff00U) >> 8) ))
	
/* Byte swap a 32 bit value */
#define BCMSWAP32(val) \
	((uint32)( \
		(((uint32)(val) & (uint32)0x000000ffUL) << 24) | \
		(((uint32)(val) & (uint32)0x0000ff00UL) <<  8) | \
		(((uint32)(val) & (uint32)0x00ff0000UL) >>  8) | \
		(((uint32)(val) & (uint32)0xff000000UL) >> 24) ))

static INLINE uint16
bcmswap16(uint16 val)
{
	return BCMSWAP16(val);
}

static INLINE uint32
bcmswap32(uint32 val)
{
	return BCMSWAP32(val);
}

/* buf	- start of buffer of shorts to swap */
/* len  - byte length of buffer */
static INLINE void
bcmswap16_buf(uint16 *buf, uint len)
{
	len = len/2;

	while(len--){
		*buf = bcmswap16(*buf);
		buf++;
	}
}

#ifndef hton16
#ifndef IL_BIGENDIAN
#define HTON16(i) BCMSWAP16(i)
#define	hton16(i) bcmswap16(i)
#define	hton32(i) bcmswap32(i)
#define	ntoh16(i) bcmswap16(i)
#define	ntoh32(i) bcmswap32(i)
#define ltoh16(i) (i)
#define ltoh32(i) (i)
#define htol16(i) (i)
#define htol32(i) (i)
#else
#define HTON16(i) (i)
#define	hton16(i) (i)
#define	hton32(i) (i)
#define	ntoh16(i) (i)
#define	ntoh32(i) (i)
#define	ltoh16(i) bcmswap16(i)
#define	ltoh32(i) bcmswap32(i)
#define htol16(i) bcmswap16(i)
#define htol32(i) bcmswap32(i)
#endif
#endif

#ifndef IL_BIGENDIAN
#define ltoh16_buf(buf, i)
#define htol16_buf(buf, i)
#else
#define ltoh16_buf(buf, i) bcmswap16_buf((uint16*)buf, i)
#define htol16_buf(buf, i) bcmswap16_buf((uint16*)buf, i)
#endif

#if 0 /* fix it latter donno why static is not very friendly */
/*
* load 16-bit value from unaligned little endian byte array.
*/
static INLINE uint16
ltoh16_ua(uint8 *bytes)
{
	return (bytes[1]<<8)+bytes[0];
}

/*
* load 32-bit value from unaligned little endian byte array.
*/
static INLINE uint32
ltoh32_ua(uint8 *bytes)
{
	return (bytes[3]<<24)+(bytes[2]<<16)+(bytes[1]<<8)+bytes[0];
}

/*
* load 16-bit value from unaligned big(network) endian byte array.
*/
static INLINE uint16
ntoh16_ua(uint8 *bytes)
{
	return (bytes[0]<<8)+bytes[1];
}

/*
* load 32-bit value from unaligned big(network) endian byte array.
*/
static INLINE uint32
ntoh32_ua(uint8 *bytes)
{
	return (bytes[0]<<24)+(bytes[1]<<16)+(bytes[2]<<8)+bytes[3];
}
#else
uint16 ltoh16_ua(uint8 *bytes);
uint32 ltoh32_ua(uint8 *bytes);
uint16 ntoh16_ua(uint8 *bytes);
uint32 ntoh32_ua(uint8 *bytes);
#endif

#endif /* _BCMENDIAN_H_ */
