/*
    Copyright 2001, Broadcom Corporation
    All Rights Reserved.
    
    This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
    the contents of this file may not be disclosed to third parties, copied or
    duplicated in any form, in whole or in part, without the prior written
    permission of Broadcom Corporation.
*/
/*
 * vxWorks 5.x OS Independent Layer
 *
 * Copyright(c) 2001 Broadcom Corp.
 * $Id: vx_osl.h,v 1.3 2007/10/08 22:31:08 iakramov Exp $
 */

#ifndef _vx_osl_h_
#define _vx_osl_h_

/* vxworks header files necessary for below */
#include <end.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <typedefs.h>

#ifdef __mips__
#define	OSL_UNCACHED(va)	(((uint32)(va) & 0x1fffffff) | 0xa0000000)
#define IS_KSEG2(_x)		(((_x) & 0xC0000000) == 0xC0000000)
#else
#define IS_KSEG2(_x)		0
#define	OSL_UNCACHED(va)	(va)
#endif

/* map/unmap physical to virtual */
/* 	- assume a 1:1 mapping if KSEG2 addresses are used */
#define	REG_MAP(pa, size)	(IS_KSEG2(pa) ? (pa) : OSL_UNCACHED(pa))
#define	REG_UNMAP(va)		/* nop */

#define BUSPROBE(val, addr) ({ \
	int __busprobe_err = vxMemProbe((char*)(addr), VX_READ, sizeof (val), (char*)&(val)); \
	if (__busprobe_err != OK) (val) = 0; \
	__busprobe_err; \
})
extern int vxMemProbe(char *adrs, int mode, int length, char *pVal);

#ifdef BCMDBG
#undef ASSERT
#define ASSERT(exp)     do {if (exp) ; else osl_assert(#exp, __FILE__, __LINE__);} while (0)
extern void osl_assert(char *exp, char *file, int line);
#else
#define	ASSERT(exp)
#endif

/* register access macros */
#ifdef MSI
#ifdef IL_BIGENDIAN
#define	R_REG(r)	((sizeof *(r) == sizeof (uint32))? \
	((uint32)( *(&((volatile uint16*)(r))[0]) | (*(&((volatile uint16*)(r))[1]) << 16))) \
	: *(r))
#define	W_REG(r, v)	((sizeof *(r) == sizeof (uint32))? \
	(*(&((volatile uint16*)r)[0]) = ((v) & 0xffff), *(&((volatile uint16*)r)[1]) = (((v) >> 16) & 0xffff)) \
	: (*(r) = (v)))
#else	/* LITTLE_ENDIAN */
#define	R_REG(r)	((sizeof *(r) == sizeof (uint32))? \
	((uint32)((*(&((volatile uint16*)(r))[0]) << 16) | *(&((volatile uint16*)(r))[1]))) \
	: *(r))
#define	W_REG(r, v)	((sizeof *(r) == sizeof (uint32))? \
	(*(&((volatile uint16*)r)[1]) = ((v) & 0xffff), *(&((volatile uint16*)r)[0]) = (((v) >> 16) & 0xffff)) \
	: (*(r) = (v)))
#endif	/* BIG_ENDIAN */
#else	/* PCI or SB */
#define wreg32(r,v)	(*(volatile uint32 *)(r) = (v))
#define rreg32(r)	(*(volatile uint32 *)(r))
#ifdef IL_BIGENDIAN
#define wreg8(r,v)	(*(volatile uint8 *)((uint32)r^3) = (v))
#define rreg8(r)	(*(volatile uint8 *)((uint32)r^3))
#define wreg16(r,v)	(*(volatile uint16 *)((uint32)r^2) = (v))
#define rreg16(r)	(*(volatile uint16 *)((uint32)r^2))
#else
#define wreg8(r,v)	(*(volatile uint8 *)(r) = (v))
#define rreg8(r)	(*(volatile uint8 *)(r))
#define wreg16(r,v)	(*(volatile uint16 *)(r) = (v))
#define rreg16(r)	(*(volatile uint16 *)(r))
#endif
/* register access macros */
#define	R_REG(r)	((sizeof *(r) == sizeof (uint32))? rreg32(r): rreg16(r))
#define	W_REG(r,v)	((sizeof *(r) == sizeof (uint32))? wreg32(r,(uint32)v): wreg16(r,(uint16)v))
#endif	/* MSI */

#define	AND_REG(r, v)	W_REG((r), R_REG(r) & (v))
#define	OR_REG(r, v)	W_REG((r), R_REG(r) | (v))

/* Host/Bus architecture specific swap. Noop for little endian systems, possible swap on big endian */
#define BUS_SWAP32(v)	(v)

/* shared memory access macros */
#define	R_SM(a)		*(a)
#define	W_SM(a, v)	(*(a) = (v))
#define	BZERO_SM(a, len)	bzero((char*)a, len)

/* bcopy, bcmp, and bzero */
#define	bcopy(src, dst, len)	bcopy((char*)(src), (char*)(dst), (len))
#define	bcmp(b1, b2, len)	bcmp((char*)(b1), (char*)(b2), (len))
#define	bzero(b, len)		bzero((char*)b, (len))

#ifdef MALLOC
#undef MALLOC
#endif 
#define	MALLOC(size)		malloc(size)
#define	VX_OSL_MFREE(addr, size)	free(addr)

#define	DMA_ALLOC_CONSISTENT(dev, size, pap)	osl_dma_alloc_consistent(dev, size, pap)
#define	DMA_FREE_CONSISTENT(dev, va, size, pa)	osl_dma_free_consistent(dev, size, (void*)va, pa)
extern void *osl_dma_alloc_consistent(void *dev, uint size, void **pap);
extern void osl_dma_free_consistent(void *dev, uint size, void *va, void *pa);

#define	DMA_TX	0
#define	DMA_RX	1
#define	DMA_MAP(dev, va, size, direction, p)	osl_dma_map(dev, (void*)va, size, direction)
#define	DMA_UNMAP(dev, pa, size, direction, p)	/* nop */
extern void *osl_dma_map(void *dev, void *va, uint size, uint direction);

#define	OSL_DELAY(us)	osl_delay((us))
extern void osl_delay (unsigned int u);
extern void delayUsec (unsigned int u);

/* packet primitives */
#define	PKTGET(drv, len, send)		osl_pktget(drv, len, send)
#define	PKTFREE(drv, m, send)		osl_pktfree(drv, (M_BLK_ID) m, send)
#define	PKTDATA(drv, m)			(((M_BLK_ID)m)->mBlkHdr.mData)
#define	PKTLEN(drv, m)			(((M_BLK_ID)m)->mBlkHdr.mLen)
#define	PKTNEXT(drv, m)			(((M_BLK_ID)m)->mBlkHdr.mNext)
#define	PKTSETNEXT(m, n)		(((M_BLK_ID)m)->mBlkHdr.mNext = (M_BLK_ID)n)
#define	PKTSETLEN(drv, m, len)		(((M_BLK_ID)m)->mBlkHdr.mLen = (len))
#define	PKTPUSH(drv, m, nbytes)		osl_pktpush(drv, (M_BLK_ID)m, nbytes)
#define	PKTPULL(drv, m, nbytes)		osl_pktpull(drv, (M_BLK_ID)m, nbytes)
#define	PKTDUP(drv, m)			osl_pktdup(drv, m)
#define	PKTCOOKIE(m)			((M_BLK_ID)((M_BLK_ID)m)->mBlkPktHdr.len)
#define	PKTSETCOOKIE(m, n)		(((M_BLK_ID)m)->mBlkPktHdr.len = (int)n)
#define	PKTLINK(m)			(((M_BLK_ID)m)->mBlkHdr.mNextPkt)
#define	PKTSETLINK(m, n)		(((M_BLK_ID)m)->mBlkHdr.mNextPkt = (M_BLK_ID)n)

extern void osl_init(void);
extern void *osl_pktget(void *drv, uint len, bool send);
extern void osl_pktfree(void *drv, M_BLK_ID m, bool send);
extern void *osl_pktdup(void *drv, void *p);
extern uchar *osl_pktpush(void *drv, M_BLK_ID m, uint nbytes);
extern uchar *osl_pktpull(void *drv, M_BLK_ID m, uint nbytes);

#ifdef __mips__
IMPORT unsigned long readCount(void);
#define	OSL_GETCYCLES(x)	(x = readCount() * 2)
#elif __i386__
#define	OSL_GETCYCLES(x)	__asm__ __volatile__("rdtsc" : "=a" (x) : : "edx")
#endif

/* vx def's used by vx_osl.c */
#define	VXCLSIZE	2044
#define	M_HEADROOM(m)	((m)->mBlkHdr.mData - (m)->pClBlk->clNode.pClBuf)


#endif	/* _vx_osl_h_ */
