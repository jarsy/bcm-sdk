/*
 *  Copyright 2001, Broadcom Corporation
 *   All Rights Reserved.

 *  This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 *  the contents of this file may not be disclosed to third parties, copied or
 *  duplicated in any form, in whole or in part, without the prior written
 *  permission of Broadcom Corporation.
 */
/*
 * vxWorks 5.x OS Independent Layer
 *
 * Copyright(c) 2001 Broadcom Corp.
 * $Id: vx_osl.h,v 1.4 2010/01/15 02:36:07 alai Exp $
 */

#ifndef _vx_osl_h_
#define _vx_osl_h_

/* vxworks header files necessary for below */
#include <end.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <typedefs.h>
#include <vxshared.h>
#include <tickLib.h>
#include <sysLib.h>

#ifdef __mips__
#define OSL_UNCACHED(va)    ( \
    (((va) & 0x40000000) == 0x40000000) ? ((uint32)(va) | 0xa0000000) : \
    (((uint32)(va) & 0x1fffffff) | 0xa0000000) \
)
#define IS_KSEG2(_x)        (((_x) & 0xC0000000) == 0xC0000000)
#else
#define IS_KSEG2(_x)        0
#define OSL_UNCACHED(va)    (va)
#endif

#ifdef __mips__
#define OSL_CACHED(va) (((uint32)(va) & 0x1fffffff) | 0x80000000)
#else
#define OSL_CACHED(va) (va)
#endif

/* map/unmap physical to virtual */
/*  - assume a 1:1 mapping if KSEG2 addresses are used */
#define REG_MAP(pa, size)   ((void *)((IS_KSEG2(pa) ? (pa) : OSL_UNCACHED(pa))))
#define REG_UNMAP(va)       do {} while (0) /* nop */

#define BUSPROBE(val, addr) ({ \
    int __busprobe_err = vxMemProbe((char*)(addr), VX_READ, sizeof(val), (char*)&(val)); \
    if (__busprobe_err != OK) (val) = 0; \
    __busprobe_err; \
})
extern int vxMemProbe(char *adrs, int mode, int length, char *pVal);

#ifdef BCMDBG_ASSERT
#undef ASSERT
#define ASSERT(exp)     do {if (exp); else osl_assert(#exp, __FILE__, __LINE__);} while (0)
extern void osl_assert(char *exp, char *file, int line);
#else /* BCMDBG_ASSERT */
#define ASSERT(exp)
#endif /* BCMDBG_ASSERT */

/* register access macros */
#define wreg32(r, v)    (*(volatile uint32 *)(r) = (v))
#define rreg32(r)   (*(volatile uint32 *)(r))
#define wreg16(r, v)    (*(volatile uint16 *)(r) = (v))
#define rreg16(r)   (*(volatile uint16 *)(r))
#define wreg8(r, v) (*(volatile uint8 *)(r) = (v))
#define rreg8(r)    (*(volatile uint8 *)(r))

/* register access macros */
#define R_REG(osh, r)   ((sizeof *(r) == sizeof(uint32))? rreg32(r): \
            (uint32)((sizeof *(r) == sizeof(uint16))? rreg16(r): rreg8(r)))
#define W_REG(osh, r, v)    ((sizeof *(r) == sizeof(uint32))? wreg32(r, (uint32)v): \
            (uint32)((sizeof *(r) == sizeof(uint16))? wreg16(r, (uint16)v): \
            wreg8(r, (uint8)v)))

#define AND_REG(osh, r, v)  W_REG(osh, (r), R_REG(osh, r) & (v))
#define OR_REG(osh, r, v)   W_REG(osh, (r), R_REG(osh, r) | (v))

/* Host/Bus architecture specific swap. Noop for little endian systems, possible swap on big endian
 */
#define BUS_SWAP32(v)   (v)

/* shared memory access macros */
#define R_SM(a)         *(a)
#define W_SM(a, v)      (*(a) = (v))
#define BZERO_SM(a, len)    bzero((char*)a, len)

/* pick up osl required snprintf/vsnprintf */
#include "bcmstdlib.h"

/* bcopy, bcmp, and bzero */
#define bcopy(src, dst, len)    bcopy((char*)(src), (char*)(dst), (len))
#define bcmp(b1, b2, len)   bcmp((char*)(b1), (char*)(b2), (len))
#define bzero(b, len)       bzero((char*)b, (len))

#ifdef MALLOC
#undef MALLOC
#endif
#ifdef MFREE
#undef MFREE
#endif
#define MALLOC(osh, size)       ((void) osh, malloc(size))
#define MFREE(osh, addr, size)  free(addr)

#define MALLOCED(osh)       (0)
#define MALLOC_FAILED(osh)  (0)
#define MALLOC_DUMP(osh, b)

#define DMA_CONSISTENT_ALIGN    sizeof(int)
#define DMA_ALLOC_CONSISTENT(osh, size, pap, dmah) \
        osl_dma_alloc_consistent(osh, size, pap)
#define DMA_FREE_CONSISTENT(osh, va, size, pa, dmah) \
        osl_dma_free_consistent(osh, size, (void*)va, pa)

extern void *osl_dma_alloc_consistent(osl_t *osh, unsigned int size, ulong *pap);
extern void osl_dma_free_consistent(osl_t *osh, unsigned int size, void *va, ulong pa);

#define DMA_TX      0   /* TX direction */
#define DMA_RX      1   /* RX direction */
#define DMA_MAP(osh, va, size, direction, p, dmah) \
        osl_dma_map(osh, (void*)va, size, direction)
#define DMA_UNMAP(osh, pa, size, direction, p, dmah)    /* nop */
extern void *osl_dma_map(osl_t *osh, void *va, unsigned int size, unsigned int direction);

/* API for DMA addressing capability */
#define OSL_DMADDRWIDTH(osh, addrwidth) do {} while (0)

#define OSL_DELAY(us)   osl_delay((us))
extern void osl_delay(unsigned int u);

typedef struct pciinfo {
    unsigned int        bus;        /* pci bus */
    unsigned int        dev;        /* pci device */
    unsigned int        func;       /* pci function */
} pciinfo_t;

/* 
 * Number of bytes to reserve at the front of each cluster buffer
 * Note: "+2" is to make packet payload align at 4 byte boundary for vxWorks
 *       protocol stack.
 */
#define PKTRSV (60+2)

/* the largest reasonable packet buffer driver uses for ethernet MTU in bytes */
#define PKTBUFSZ ((int)(VXCLSIZE - sizeof(CL_BUF) - PKTRSV))    /* Packet buffer size for Vx */

/* Macros to encode and decode priority information for VxWorks */
#define BCMVXW_PRIO_SIGN    0x62636d00             /* 'b','c', 'm', 0 */
#define BCMVXW_CLR_PRIO(m)  (((M_BLK_ID)m)->pClBlk->clFreeArg3 = 0)
#define BCMVXW_ENCODE_PRIO(m, prio) (((M_BLK_ID)m)->pClBlk->clFreeArg3 = \
        (BCMVXW_PRIO_SIGN | ((prio) & 0xff)))
#define BCMVXW_DECODE_PRIO(m)   (((M_BLK_ID)m)->pClBlk->clFreeArg3 & 0xff)
#define BCMVXW_VALID_PRIO(m) \
        ((((M_BLK_ID)m)->pClBlk->pClFreeRtn == NULL) && \
         ((((M_BLK_ID)m)->pClBlk->clFreeArg3 & 0xffffff00) == BCMVXW_PRIO_SIGN))
         
/* Memory types for packet primitives */
#define MEMORY_DDRRAM 0
#define MEMORY_SOCRAM 1
#define MEMORY_PCIMEM 2 

/* packet primitives */
#define PKTGET(osh, len, send, mt)  osl_pktget(osh, len)
#define PKTFREE(osh, m, send, mt)   osl_pktfree((M_BLK_ID)m)
#define PKTDATA(osh, m)         ((uchar *)(((M_BLK_ID)m)->mBlkHdr.mData))
#define PKTLEN(osh, m)          (((M_BLK_ID)m)->mBlkHdr.mLen)
#define PKTHEADROOM(osh, m)     (((M_BLK_ID)m)->mBlkHdr.mFlags & M_EXT ? \
                    ((M_BLK_ID)m)->mBlkHdr.mData - \
                    ((M_BLK_ID)m)->pClBlk->clNode.pClBuf : 0)
#define PKTTAILROOM(osh, m)     (0)
#define PKTNEXT(osh, m)         (((M_BLK_ID)m)->mBlkHdr.mNext)
#define PKTSETNEXT(osh, m, n)       (((M_BLK_ID)m)->mBlkHdr.mNext = (M_BLK_ID)n)
#define PKTSETLEN(osh, m, len)      (((M_BLK_ID)m)->mBlkHdr.mLen = (len))
#define PKTPUSH(osh, m, nbytes)     osl_pktpush(osh, (M_BLK_ID)m, nbytes)
#define PKTPULL(osh, m, nbytes)     osl_pktpull(osh, (M_BLK_ID)m, nbytes)
#define PKTDUP(osh, m)          osl_pktdup(osh, m)
#define PKTTAG(m)           ((M_BLK_ID)((M_BLK_ID)m)->mBlkPktHdr.len)
#define PKTFRMNATIVE(osh, m)        osl_pkt_frmnative((osh), (M_BLK_ID)(m))
#define PKTTONATIVE(osh, p)     osl_pkt_tonative((osh), (p))
#define PKTLINK(m)          (((M_BLK_ID)m)->mBlkHdr.mNextPkt)
#define PKTSETLINK(m, n)        (((M_BLK_ID)m)->mBlkHdr.mNextPkt = (M_BLK_ID)n)
#define PKTPRIO(m) \
    ((M_HASCL(((M_BLK_ID)m)) && BCMVXW_VALID_PRIO((m)))? BCMVXW_DECODE_PRIO((m)) : 0)
#define PKTSETPRIO(m, p) \
    do {if (M_HASCL(((M_BLK_ID)m)) && ((M_BLK_ID)m)->pClBlk->pClFreeRtn == NULL) \
    BCMVXW_ENCODE_PRIO((m), (p));} while (0)
#define PKTSHARED(m)                    (((M_BLK_ID)m)->pClBlk->clRefCnt > 1)
#define PKTALLOCED(osh)         (0)
#define PKTFLAGTS(osh, lb)      (0)

#ifdef BCMDBG_PKT
#define PKTLIST_DUMP(osh, buf)      ((void) buf)
#else /* BCMDBG_PKT */
#define PKTLIST_DUMP(osh, buf)
#endif /* BCMDBG_PKT */

extern osl_t *osl_attach(void *pdev, END_OBJ *endobj, bool pkttag);
extern void osl_detach(osl_t *osh);
extern void *osl_pktget(osl_t *osh, unsigned int len);
extern void osl_pktfree(M_BLK_ID m);
extern void *osl_pktdup(osl_t *osh, void *p);
extern uchar *osl_pktpush(osl_t *osh, M_BLK_ID m, unsigned int nbytes);
extern uchar *osl_pktpull(osl_t *osh, M_BLK_ID m, unsigned int nbytes);
extern unsigned int osl_pci_read_config(osl_t *osh, unsigned int offset, unsigned int size);
extern void osl_pci_write_config(osl_t *osh, unsigned int offset, unsigned int size, unsigned int val);
extern void osl_pcmcia_read_attr(osl_t *osh, unsigned int offset, void *buf, int size);
extern void osl_pcmcia_write_attr(osl_t *osh, unsigned int offset, void *buf, int size);
extern void* osl_pkt_frmnative(osl_t *osh, M_BLK_ID m);
extern M_BLK_ID osl_pkt_tonative(osl_t *osh, void *pkt);

extern void osl_pktfree_cb_set(osl_t *osh, pktfree_cb_fn_t tx_fn, void *tx_ctx);
#define PKTFREESETCB(osh, tx_fn, tx_ctx) osl_pktfree_cb_set(osh, tx_fn, tx_ctx)

#define OSL_ERROR(bcmerror) osl_error(bcmerror)
extern int osl_error(int bcmerror);

#define OSL_PCI_READ_CONFIG(loc, offset, size)         osl_pci_read_config(loc, offset, size)
#define OSL_PCI_WRITE_CONFIG(loc, offset, size, val)   osl_pci_write_config(loc, offset, size, val)

#define OSL_PCI_BUS(osh)    osl_pci_bus(osh)
#define OSL_PCI_SLOT(osh)   osl_pci_slot(osh)
extern unsigned int osl_pci_bus(osl_t *osh);
extern unsigned int osl_pci_slot(osl_t *osh);

#define OSL_PCMCIA_READ_ATTR(osh, offset, buf, size) osl_pcmcia_read_attr((osh), (offset), \
                (buf), (size))
#define OSL_PCMCIA_WRITE_ATTR(osh, offset, buf, size)   osl_pcmcia_write_attr((osh), (offset), \
                (buf), (size))

#ifdef __mips__
IMPORT unsigned long readCount(void);
#define OSL_GETCYCLES(x)    (x = readCount() * 2)
#elif __i386__
#define OSL_GETCYCLES(x)    __asm__ __volatile__("rdtsc" : "=a" (x) : : "edx")
#endif

/* get system up time in miliseconds */
#define OSL_SYSUPTIME()     ((uint32)tickGet() * (1000 / sysClkRateGet()))

/* PED: packet for shunt */
typedef struct _shpkt {
    uchar           *buffer;
    uchar           *buffer_p;  /* Physical addr of buffer for speed */
    uchar           *body_p;    /* Physical addr of (buffer + 32) for speed */
    struct _shpkt   *next;
} SHPKT;

/* PED */
extern void osl_shunt_pkt_init(void);
extern void *osl_pktget_shunt(void);
extern void osl_pktfree_shunt(SHPKT *p);
#define PKTPOLL_INIT_SHUNT()                osl_shunt_pkt_init()
#define PKTGET_SHUNT(osh, len, send, mt)    osl_pktget_shunt()
#define PKTFREE_SHUNT(osh, m, send, mt)     osl_pktfree_shunt((SHPKT *)m)
#define PKTDATA_SHUNT(osh, m)               (((SHPKT *)m)->buffer)
#define PKTDATAP_SHUNT(m)                   (((SHPKT *)m)->buffer_p)
#define PKTBODYP_SHUNT(m)                   (((SHPKT *)m)->body_p)

/* PED */
#define RD_REG(osh, r)       rreg32(r)
#define WR_REG(osh, r, v)    wreg32(r, (uint32)v)

#endif  /* _vx_osl_h_ */
