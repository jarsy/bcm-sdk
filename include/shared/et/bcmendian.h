/*
 * $Id: bcmendian.h,v 1.4 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * local version of endian.h - byte order defines
 */

#ifndef _BCMENDIAN_H_
#define _BCMENDIAN_H_

#include <shared/et/typedefs.h>

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

/* Byte swap a 64 bit value */
#define BCMSWAP64(val) \
    ((uint64_t) ( \
        (((uint64_t) (val) & (uint64_t) 0x00000000000000ffULL) << 56) | \
        (((uint64_t) (val) & (uint64_t) 0x000000000000ff00ULL) << 40) | \
        (((uint64_t) (val) & (uint64_t) 0x0000000000ff0000ULL) << 24) | \
        (((uint64_t) (val) & (uint64_t) 0x00000000ff000000ULL) <<  8) | \
        (((uint64_t) (val) & (uint64_t) 0x000000ff00000000ULL) >>  8) | \
        (((uint64_t) (val) & (uint64_t) 0x0000ff0000000000ULL) >> 24) | \
        (((uint64_t) (val) & (uint64_t) 0x00ff000000000000ULL) >> 40) | \
        (((uint64_t) (val) & (uint64_t) 0xff00000000000000ULL) >> 56) ))

/* bufp - start of buffer of shorts to swap */
/* length - byte length of buffer */
#define BCMSWAP16_BUF(bufp,length)  \
    do {                            \
        uint16 *buf = bufp;         \
        uint len = (length) / 2;    \
        while((len)--) {            \
            *buf = BCMSWAP16(*buf); \
            buf++;                  \
        }                           \
    } while(0)

#ifndef hton16
#ifndef BE_HOST
#define HTON16(i) BCMSWAP16(i)
#define	hton16(i) BCMSWAP16(i)
#define	hton32(i) BCMSWAP32(i)
#define	ntoh16(i) BCMSWAP16(i)
#define	ntoh32(i) BCMSWAP32(i)
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
#define	ltoh16(i) BCMSWAP16(i)
#define	ltoh32(i) BCMSWAP32(i)
#define htol16(i) BCMSWAP16(i)
#define htol32(i) BCMSWAP32(i)
#endif
#endif

#ifndef BE_HOST
#define ltoh16_buf(buf, i)
#define htol16_buf(buf, i)
#else
#define ltoh16_buf(buf, i) BCMSWAP16_BUF((uint16*)buf, i)
#define htol16_buf(buf, i) BCMSWAP16_BUF((uint16*)buf, i)
#endif

#endif /* _BCMENDIAN_H_ */
