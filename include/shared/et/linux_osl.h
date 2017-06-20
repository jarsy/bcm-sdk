/*
 * $Id: linux_osl.h,v 1.8 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * Linux OS Independent Layer
 */

#ifndef _linux_osl_h_
#define _linux_osl_h_

/*
 * BINOSL selects the slightly slower function-call-based binary compatible osl.
 * Macros expand to calls to functions defined in linux_osl.c .
 */

/* linux header files necessary for below */
#ifndef BINOSL
#include <linuxver.h>
#endif
#include <shared/et/typedefs.h>

#ifdef BCMDBG
#undef ASSERT
#define ASSERT(exp)     do {if (exp) ; else osl_assert(#exp, __FILE__, __LINE__);} while (0)
extern void osl_assert(char *exp, char *file, int line);
#else
#define	ASSERT(exp)
#endif

#define	OSL_PCMCIA_READ_ATTR(osh, offset, buf, size)	osl_pcmcia_read_attr((osh), (offset), (buf), (size))
#define	OSL_PCMCIA_WRITE_ATTR(osh, offset, buf, size)	osl_pcmcia_write_attr((osh), (offset), (buf), (size))
extern void osl_pcmcia_read_attr(void *osh, uint offset, void *buf, int size);
extern void osl_pcmcia_write_attr(void *osh, uint offset, void *buf, int size);

#define	OSL_PCI_READ_CONFIG(loc, offset, size)		osl_pci_read_config(loc, offset, size)
#define	OSL_PCI_WRITE_CONFIG(loc, offset, size, val)	osl_pci_write_config(loc, offset, size, val)
extern uint32 osl_pci_read_config(void *loc, uint size, uint offset);
extern void osl_pci_write_config(void *loc, uint offset, uint size, uint val);

#define osl_init()	  /* dummy func */

#ifdef mips
#define	OSL_GETCYCLES(x)	(x = read_c0_count() * 2)
#elif defined(__i386__)
#define	OSL_GETCYCLES(x)	rdtscl(x)
#endif

#ifndef BINOSL	/* default fast inline version */

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,21))
#define	printf	printk
#endif
/* register access macros */
#if defined(KEYSTONE) || defined(IPROC_CMICD)
#define	R_REG(osh, r)	((sizeof *(r) == sizeof (uint32)) ? readl((uint32 *)(r)) : readw((uint16*)(r)))
#define	W_REG(osh, r, v)							\
	do {								\
		if (sizeof *(r) == sizeof (uint32))			\
			writel(v, (uint32*)r);					\
		else							\
			writew((uint16)(v), (uint16*)(r));	\
	} while (0)
#define	AND_REG(osh, r, v)	W_REG(osh, (r), R_REG(osh, r) & (v))
#define	OR_REG(osh, r, v)	W_REG((osh, r), R_REG(osh, r) | (v))
#else /* !KEYSTONE */
#define	R_REG(r)	((sizeof *(r) == sizeof (uint32)) ? readl((uint32 *)(r)) : readw((uint16*)(r)))
#define	W_REG(r, v)							\
	do {								\
		if (sizeof *(r) == sizeof (uint32))			\
			writel(v, (uint32*)r);					\
		else							\
			writew((uint16)(v), (uint16*)(r));	\
	} while (0)
#define	AND_REG(r, v)	W_REG((r), R_REG(r) & (v))
#define	OR_REG(r, v)	W_REG((r), R_REG(r) | (v))
#endif /* KEYSTONE */

/* bcopy, bcmp, and bzero */
#define	bcopy(src, dst, len)	memcpy(dst, src, len)
#define	bcmp(b1, b2, len)	memcmp(b1, b2, len)
#define	bzero(b, len)		memset(b, '\0', len)

#if defined(KEYSTONE) || defined(IPROC_CMICD)
typedef struct osl_info osl_t;
#define	MALLOC(osh, size)		kmalloc(size, GFP_ATOMIC)
#define	MFREE(osh, addr, size)	kfree((void*)(addr))
extern void osl_mfree(osl_t *osh, char *addr, uint size);
#else
#define	MALLOC(size)		kmalloc(size, GFP_ATOMIC)
#define	MFREE(addr, size)	osl_mfree((char*)(addr), size)
extern void osl_mfree(char *addr, uint size);
#endif

/* uncached virtual address */
#ifdef mips
#define OSL_UNCACHED(va)		KSEG1ADDR((va))
#else
#define OSL_UNCACHED(va)		(va)
#endif

/* map/unmap physical to virtual I/O */
#define	REG_MAP(pa, size)		ioremap_nocache((unsigned long)(pa), (unsigned long)(size))
#define	REG_UNMAP(va)			iounmap((void *)(va))

/* Host/Bus architecture specific swap. Noop for little endian systems, possible swap on big endian */
#ifdef IL_BIGENDIAN
#define BUS_SWAP32(v) __swap32(v)
#else
#define BUS_SWAP32(v)	(v)
#endif

#ifdef mips
#if defined(MODULE) && (LINUX_VERSION_CODE < KERNEL_VERSION(2,4,17))
#define BUSPROBE(val, addr)		panic("get_dbe() will not fixup a bus exception when compiled into a module")
#else
#include <asm/paccess.h>
#define	BUSPROBE(val, addr)		get_dbe(val, addr)
#endif
#else
#define	BUSPROBE(val, addr)		val = R_REG(addr)
#endif

#define	DMA_ALLOC_CONSISTENT(dev, size, pap)	pci_alloc_consistent(dev, size, (dma_addr_t*)(pap))
#define	DMA_FREE_CONSISTENT(dev, va, size, pa)	pci_free_consistent(dev, size, (void*)(va), (dma_addr_t)((uint)pa))
#define	DMA_TX	PCI_DMA_TODEVICE
#define	DMA_RX	PCI_DMA_FROMDEVICE

#define	DMA_MAP(dev, va, size, direction, p)	pci_map_single(dev, (void*)(va), size, direction)
#define	DMA_UNMAP(dev, pa, size, direction, p)	pci_unmap_single(dev, (uint32)(pa), size, direction)

#define	OSL_DELAY(usec)	udelay(usec)
#define OSL_SLEEP(usec) set_current_state(TASK_INTERRUPTIBLE); \
                        schedule_timeout((usec*HZ)/1000000);
#define OSL_IN_INTERRUPT() in_interrupt()
#include <linux/delay.h>

/* shared (dma-able) memory access macros */
#ifdef __arch_um__
extern void pci_flush_consistent(struct pci_dev *hwdev, void *cpu_addr,
				 size_t size, int direction);
#define F_SM(r,dir) pci_flush_consistent(NULL, (void*)(r), sizeof(*(r)), (dir))
#define R_SM(r) ({ F_SM(r, PCI_DMA_FROMDEVICE); *(r); })
#define W_SM(r, v) ({ *(r) = (v); F_SM(r, PCI_DMA_TODEVICE); })
#define BZERO_SM(r, len) ({ memset((r), '\0', (len)); F_SM(r, PCI_DMA_TODEVICE); })
#elif defined(ILSIM)
#define R_SM(r)		readl((uint)(r))
#define W_SM(r, v)	writel(v, (uint)(r))
#define BZERO_SM(r, len)	bzero_sm((uchar*)(r), len)
extern void bzero_sm(uchar *r, uint len);
#else
#define	R_SM(r)		*(r)
#define	W_SM(r, v)	(*(r) = (v))
#define	BZERO_SM(r, len)	memset(r, '\0', len)
#endif	/* ILSIM */

/* packet primitives */
#define	PKTGET(drv, len, send)		osl_pktget(drv, len, send)
extern void *osl_pktget(void *drv, uint len, bool send);
#define	PKTFREE(drv, skb, send)		osl_pktfree(skb)
extern void osl_pktfree(void *skb);
#define	PKTDATA(drv, skb)		((struct sk_buff*)(skb))->data
#define	PKTLEN(drv, skb)		((struct sk_buff*)(skb))->len
#define	PKTNEXT(drv, skb)		((struct sk_buff*)(skb))->next
#define	PKTSETLEN(drv, skb, len)	__skb_trim((struct sk_buff*)(skb), len)
#define	PKTPUSH(drv, skb, bytes)	skb_push((struct sk_buff*)(skb), bytes)
#define	PKTPULL(drv, skb, bytes)	skb_pull((struct sk_buff*)(skb), bytes)
#define	PKTDUP(drv, skb)		skb_clone((struct sk_buff*)(skb), GFP_ATOMIC)
#define	PKTCOOKIE(skb)			(void *)((struct sk_buff*)(skb))->csum
#define	PKTSETCOOKIE(skb, x)		((struct sk_buff*)(skb))->csum = (unsigned int)(x)
#define	PKTLINK(skb)			((struct sk_buff*)(skb))->prev
#define	PKTSETLINK(skb, x)		((struct sk_buff*)(skb))->prev = (struct sk_buff*)(x)

#else	/* BINOSL */                                    

#ifndef LINUX_OSL	/* linux_osl.c doesn't want this part */
/* {printf, sprintf, strcmp, strncpy, strlen} */
#undef printf
#define	printf				osl_printf
#undef sprintf
#define	sprintf				osl_sprintf
#undef strcmp
#define	strcmp(s1, s2)	osl_strcmp(s1, s2)
#undef strncpy
#undef strlen
#define strlen(s)			osl_strlen(s)
#undef strncpy
#define	strncpy(d, s, n)		osl_strncpy(d, s, n)
extern int osl_printf(char *format, ...);
extern int osl_sprintf(char *buf, char *format, ...);
extern int osl_strcmp(char *s1, char *s2);
extern int osl_strlen(char *s);
extern char *osl_strncpy(char *d, char *s, uint n);
#endif	/* LINUX_OSL */

/* register access macros */
#define	R_REG(r)	((sizeof *(r) == sizeof (uint32)) ? osl_readl((volatile uint32*)(r)) : osl_readw((volatile uint16*)(r)))
#define	W_REG(r, v)							\
	do {								\
		if (sizeof *(r) == sizeof (uint32))			\
			osl_writel((uint32)(v), (volatile uint32*)(r));	\
		else							\
			osl_writew((uint16)(v), (volatile uint16*)(r));	\
	} while (0)
extern uint32 osl_readl(volatile uint32 *r);
extern uint16 osl_readw(volatile uint16 *r);
extern void osl_writel(uint32 v, volatile uint32 *r);
extern void osl_writew(uint16 v, volatile uint16 *r);
#define	AND_REG(r, v)	W_REG((r), R_REG(r) & (v))
#define	OR_REG(r, v)	W_REG((r), R_REG(r) | (v))

extern void bcopy(const void *src, void *dst, int len);
extern int bcmp(const void *b1, const void *b2, int len);
extern void bzero(void *b, int len);

#define	MALLOC(size)		osl_malloc(size)
#define	MFREE(addr, size)	osl_mfree((char*)(addr), size)
extern void *osl_malloc(uint size);
extern void osl_mfree(char *addr, uint size);

/* physical to uncached virtual address */
#define OSL_UNCACHED(pa, size)		osl_uncached(pa, size)
extern void *osl_uncached(uint pa, uint size);
/* virtual to physical address */
#define OSL_PHYSADDR(va)		osl_physaddr(va)
extern uint osl_physaddr(void *va);

/* map/unmap virtual to physical */
#define	CPU_MAP(pa, size)		osl_cpu_map(pa, size)
#define	CPU_UNMAP(va)			osl_cpu_unmap(va)
extern uint32 osl_cpu_map(uint32 pa, uint size);
extern void osl_cpu_unmap(uint32 va);

#define	BUSPROBE(val, addr)		osl_busprobe(&(val), addr)
extern void osl_busprobe(uint32 *val, uint32 addr);

#define	DMA_ALLOC_CONSISTENT(dev, size, pap)	osl_dma_alloc_consistent(dev, size, pap)
#define	DMA_FREE_CONSISTENT(dev, va, size, pa)	osl_dma_free_consistent(dev, (void*)(va), size, pa)
extern void *osl_dma_alloc_consistent(void *dev, uint size, void *pap);
extern void osl_dma_free_consistent(void *dev, void *va, uint size, void *pa);

#define	DMA_TX	1
#define	DMA_RX	2

#define	DMA_MAP(dev, va, size, direction, p)	osl_dma_map(dev, va, size, direction)
#define	DMA_UNMAP(dev, pa, size, direction, p)	osl_dma_unmap(dev, pa, size, direction)
extern uint osl_dma_map(void *dev, void *va, uint size, int direction);
extern void osl_dma_unmap(void *dev, uint pa, uint size, int direction);

#define	DELAY(usec)	osl_delay(usec)
extern void osl_delay(uint usec);

#define	R_SM(r)		*(r)
#define	W_SM(r, v)	(*(r) = (v))
#define	BZERO_SM(r, len)	bzero(r, len)

/* packet primitives */
#define	PKTGET(drv, len, send)		osl_pktget(drv, len, send)
#define	PKTFREE(drv, skb, send)		osl_pktfree(skb)
#define	PKTDATA(drv, skb)		osl_pktdata(drv, skb)
#define	PKTLEN(drv, skb)		osl_pktlen(drv, skb)
#define	PKTNEXT(drv, skb)		0
#define	PKTSETLEN(drv, skb, len)	osl_pktsetlen(drv, skb, len)
#define	PKTPUSH(drv, skb, bytes)	osl_pktpush(drv, skb, bytes)
#define	PKTPULL(drv, skb, bytes)	osl_pktpull(drv, skb, bytes)
#define	PKTDUP(drv, skb)		osl_pktdup(drv, skb)
#define	PKTLINK(skb)			osl_pktlink(skb)
#define	PKTSETLINK(skb, x)		osl_pktsetlink(skb, x)

extern void *osl_pktget(void *drv, uint len, bool send);
extern void osl_pktfree(void *skb);
extern uchar *osl_pktdata(void *drv, void *skb);
extern uint osl_pktlen(void *drv, void *skb);
extern void osl_pktsetlen(void *drv, void *skb, uint len);
extern uchar *osl_pktpush(void *drv, void *skb, int bytes);
extern uchar *osl_pktpull(void *drv, void *skb, int bytes);
extern void *osl_pktdup(void *drv, void *skb);
extern void *osl_pktlink(void *skb);
extern void osl_pktsetlink(void *skb, void *x);

#endif	/* BINOSL */

#endif	/* _linux_osl_h_ */
