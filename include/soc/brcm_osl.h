/*
 * $Id: brcm_osl.h,v 1.8 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * Broadcom OS Independent Layer
 *
 * NOTE: This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied or
 * duplicated in any form, in whole or in part, without the prior written
 * permission of Broadcom Corporation.
 */

#ifndef _brcm_osl_h_
#define _brcm_osl_h_

#include <assert.h>

#include <shared/bsl.h>
#include <shared/et/typedefs.h>

#include <sal/core/libc.h>
#include <shared/alloc.h>
#ifdef BCM_ROBO_SUPPORT
#include <soc/robo/mcm/driver.h>
#endif

#include <soc/dma.h>
#include <soc/debug.h>

#include <soc/cm.h>

#define ASSERT    assert
#define	OSL_ROBO_PCMCIA_READ_ATTR(osh, offset, buf, size)
#define	OSL_ROBO_PCMCIA_WRITE_ATTR(osh, offset, buf, size)

#define	OSL_ROBO_PCI_READ_CONFIG(dev, offset, size) \
        soc_pci_conf_read((int)dev, offset)
#define	OSL_ROBO_PCI_WRITE_CONFIG(dev, offset, size, val) \
        soc_pci_conf_write((int)dev, offset, val)

#define osl_init()  /* dummy func */

/* register access macros */
#define	R_REG(u, r)             CMREAD((int)u, (uint32)r)
#define	W_REG(u, r, v)          CMWRITE((int)u, (uint32)r, v)
#define	AND_REG(u, r, v)        W_REG((int)u, (uint32)(r), \
                                      R_REG((int)u, (uint32)r) & (v))
#define	OR_REG(u, r, v)         W_REG((int)u, (uint32)(r), \
                                      R_REG((int)u, (uint32)r) | (v))

/* bcopy, bcmp, and bzero */
#define	bcopy(src, dst, len)    sal_memcpy(dst, src, len)
#define	bcmp(b1, b2, len)       sal_memcmp(b1, b2, len)
#define	bzero(b, len)           sal_memset(b, '\0', len)

#define	ET_MALLOC(size)            sal_alloc(size, (char *)"et module")
#define	ET_MFREE(addr, size)       sal_free((void*)(addr))

#define OSL_UNCACHED(va)        (va)

/* map/unmap physical to virtual I/O */
#define	REG_MAP(pa, size)	(pa)
#define	REG_UNMAP(va)

/* Host/Bus architecture specific swap. Noop for little endian systems,
 * possible swap on big endian */
/* #ifdef BE_HOST */
#if 0
#define BUS_SWAP32(v) __swap32(v)
#else
#define BUS_SWAP32(v)	(v)
#endif

#define	DMA_ALLOC_CONSISTENT(dev, size, pap)	soc_cm_salloc((int)dev, size, \
	                                                      (const char *)  \
							      "dma alloc")
#define	DMA_FREE_CONSISTENT(dev, va, size, pa)	soc_cm_sfree((int)dev, va)
#define	DMA_TX	DV_TX
#define	DMA_RX	DV_RX

#define	DMA_MAP(dev, va)			soc_cm_l2p((int)dev, (void *)va)
#define	DMA_UNMAP(dev, pa)			soc_cm_p2l((int)dev, \
	                                                   (sal_paddr_t)pa)

#ifndef __KERNEL__
#if !defined(VXWORKS) && !defined(__ECOS)
#define	OSL_DELAY(usec)				sal_usleep(usec)
#define OSL_SLEEP(usec)				sal_usleep(usec)
#else
#define	OSL_DELAY(usec)                \
    do {                               \
        volatile int ii;                        \
        for (ii=0; ii<usec*0xc8; ii++);  \
    } while (0);
#define OSL_SLEEP(usec)				sal_usleep(usec)
#endif
#else
#include <linux/delay.h>
#define	OSL_DELAY(usec)				udelay(usec)
#define OSL_SLEEP(usec)				udelay(usec)
#endif
#define OSL_IN_INTERRUPT()

/* shared (dma-able) memory access macros */
#define	R_SM(r)					*(r)
#define	W_SM(r, v)				(*(r) = (v))
#define	BZERO_SM(r, len)			sal_memset(r, '\0', len)


/* Memory types for packet primitives */
#define MEMORY_DDRRAM 0
#define MEMORY_SOCRAM 1
#define MEMORY_PCIMEM 2 

/* packet primitives */
#define	ET_PKTGET(dev, chan, len, send)	        et_pktget(dev, chan, len, send)
extern void *et_pktget(void *dev, int chan, uint len, bool send);
#define	ET_PKTFREE(dev, dv, send)		et_pktfree(dev, dv)
extern void et_pktfree(void *dev, void *dv);
#define	ET_PKTDATA(dev, dv)       ((uint8 *) \
		                ((((eth_dv_t*)dv)->dv_dcb) \
			         [((eth_dv_t*)dv)->dv_dcnt].dcb_vaddr))
#define	ET_PKTLEN(dev, dv)	       ((((eth_dv_t*)dv)->dv_dcb) \
		                [((eth_dv_t*)dv)->dv_dcnt].len)
#define	ET_PKTNEXT(dev, dv)       ((((eth_dv_t*)dv)->dv_dcb) \
		                [((eth_dv_t*)dv)->dv_dcnt++].next?dv:NULL)
#define	ET_PKTSETLEN(dev, dv, l)  (((((eth_dv_t*)dv)->dv_dcb) \
			        [((eth_dv_t*)dv)->dv_dcnt].len)=(l))

#define	ET_PKTPUSH(dev, dv, bytes)	  
#define	ET_PKTPULL(dev, dv, bytes)	      
#define ET_PKTPRIO(drv, dv)     0         
#define ET_PKTSETPRIO(drv, dv, x)   
#define ET_PKTFLAGTS(drv, dv)  0
/* PKTPUSH, PKTPULL, PKTDUP, PKTCOOKIE, PKTSETCOOKIE, PKTLINK, PKTSETLINK
 * are not supported in soc */
#define	PKTPUSH(drv, dv, bytes)	    
#define	PKTPULL(drv, dv, bytes)	    
#define	PKTDUP(drv, dv)
#define	PKTCOOKIE(dv)
#define	PKTSETCOOKIE(dv, x)
#define	PKTLINK(dv)
#define	PKTSETLINK(dv, x)

#undef printf
#define	printf					bsl_printf
#undef sprintf
#define	sprintf					sal_sprintf
#undef strcmp
#define	strcmp(s1, s2)				sal_strcmp(s1, s2)
#undef strlen
#define strlen(s)				sal_strlen(s)

#endif	/* _brcm_osl_h_ */
