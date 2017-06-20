/*
 * Misc system wide definitions
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 * $Id: bcmdefs.h,v 1.2 Broadcom SDK $
 */

#ifndef	_bcmdefs_h_
#define	_bcmdefs_h_

/*
 * One doesn't need to include this file explicitly, gets included automatically if
 * typedefs.h is included.
 */

/* Reclaiming text and data :
 * The following macros specify special linker sections that can be reclaimed
 * after a system is considered 'up'.
 */


#ifdef DONGLEBUILD

extern bool bcmreclaimed;

#define BCMATTACHDATA(_data)	__attribute__ ((__section__ (".dataini2." #_data))) _data
#define BCMATTACHFN(_fn)	__attribute__ ((__section__ (".textini2." #_fn))) _fn

#if defined(BCMRECLAIM)
#define BCMINITDATA(_data)	__attribute__ ((__section__ (".dataini1." #_data))) _data
#define BCMINITFN(_fn)		__attribute__ ((__section__ (".textini1." #_fn))) _fn
#define CONST
#else
#define BCMINITDATA(_data)	_data
#define BCMINITFN(_fn)		_fn
#define CONST	const
#endif

#ifdef BCMNODOWN
#define BCMUNINITFN(_fn)	BCMINITFN(_fn)
#else
#define BCMUNINITFN(_fn)	_fn
#endif

#else /* DONGLEBUILD */

#define bcmreclaimed 		0
#define BCMATTACHDATA(_data)	_data
#define BCMATTACHFN(_fn)	_fn
#define BCMINITDATA(_data)	_data
#define BCMINITFN(_fn)		_fn
#define BCMUNINITFN(_fn)	_fn
#define CONST	const

#endif /* DONGLEBUILD */

/* Put some library data/code into ROM to reduce RAM requirements */
#if defined(BCMROMOFFLOAD)
#define BCMROMDATA(_data)	__attribute__ ((__section__ (".datarom." #_data))) _data
#define BCMROMFN(_fn)		__attribute__ ((noinline, long_call)) _fn
#else
#define BCMROMDATA(_data)	_data
#define BCMROMFN(_fn)		_fn
#endif

/* Bus types */
#define	SI_BUS			0	/* SOC Interconnenct */
#define	PCI_BUS			1	/* PCI target */
#define	PCMCIA_BUS		2	/* PCMCIA target */
#define SDIO_BUS		3	/* SDIO target */
#define JTAG_BUS		4	/* JTAG */
#define USB_BUS			5	/* USB (does not support R/W REG) */
#define SPI_BUS			6	/* gSPI target */
#define RPC_BUS			7	/* RPC target */

/* Allows size optimization for single-bus image */
#ifdef BCMBUSTYPE
#define BUSTYPE(bus) 	(BCMBUSTYPE)
#else
#define BUSTYPE(bus) 	(bus)
#endif

/* Allows size optimization for SPROM support */
#if defined(BCMSPROMBUS)
#define SPROMBUS	(BCMSPROMBUS)
#elif defined(SI_PCMCIA_SROM)
#define SPROMBUS	(PCMCIA_BUS)
#else
#define SPROMBUS	(PCI_BUS)
#endif

/* Allows size optimization for single-chip image */
/* XXX This macro is NOT meant to encourage writing chip-specific code.
 * Use it only when it is appropriate for example in PMU PLL/CHIP/SWREG
 * controls and in chip specific workarounds.
 */
#ifdef BCMCHIPID
#define CHIPID(chip)	(BCMCHIPID)
#else
#define CHIPID(chip)	(chip)
#endif

/* Defines for DMA Address Width - Shared between OSL and HNDDMA */
#define DMADDR_MASK_32 0x0		/* Address mask for 32-bits */
#define DMADDR_MASK_30 0xc0000000	/* Address mask for 30-bits */
#define DMADDR_MASK_0  0xffffffff	/* Address mask for 0-bits (hi-part) */

#define	DMADDRWIDTH_30  30 /* 30-bit addressing capability */
#define	DMADDRWIDTH_32  32 /* 32-bit addressing capability */
#define	DMADDRWIDTH_63  63 /* 64-bit addressing capability */
#define	DMADDRWIDTH_64  64 /* 64-bit addressing capability */

/* packet headroom necessary to accomodate the largest header in the system, (i.e TXOFF).
 * By doing, we avoid the need  to allocate an extra buffer for the header when bridging to WL.
 * There is a compile time check in wlc.c which ensure that this value is at least as big
 * as TXOFF. This value is used in dma_rxfill (hnddma.c).
 */
#define BCMEXTRAHDROOM 160

/* Headroom required for dongle-to-host communication.  Packets allocated
 * locally in the dongle (e.g. for CDC ioctls or RNDIS messages) should
 * leave this much room in front for low-level message headers which may
 * be needed to get across the dongle bus to the host.  (These messages
 * don't go over the network, so room for the full WL header above would
 * be a waste.).
 */
#ifdef BCMUSBDEV
#define BCMDONGLEHDRSZ 0
#else
#define BCMDONGLEHDRSZ 12
#endif

#ifdef BCMDBG

#ifndef BCMDBG_ERR
#define BCMDBG_ERR
#endif /* BCMDBG_ERR */

#ifndef BCMDBG_ASSERT
#define BCMDBG_ASSERT
#endif /* BCMDBG_ASSERT */

#endif /* BCMDBG */

/* Brett's nifty macros for doing definition and get/set of bitfields
 * Usage example, e.g. a three-bit field (bits 4-6):
 *    #define <NAME>_M	BITFIELD_MASK(3)
 *    #define <NAME>_S	4
 * ...
 *    regval = R_REG(osh, &regs->regfoo);
 *    field = GFIELD(regval, <NAME>);
 *    regval = SFIELD(regval, <NAME>, 1);
 *    W_REG(osh, &regs->regfoo, regval);
 */
#define BITFIELD_MASK(width) \
		(((unsigned)1 << (width)) - 1)
#define GFIELD(val, field) \
		(((val) >> field ## _S) & field ## _M)
#define SFIELD(val, field, bits) \
		(((val) & (~(field ## _M << field ## _S))) | \
		 ((unsigned)(bits) << field ## _S))

/* define BCMSMALL to remove misc features for memory constrained enviroments */
#ifdef BCMSMALL
#undef	BCMSPACE
#define bcmspace	FALSE	/* if (bcmspace) code is discarded */
#else
#define	BCMSPACE
#define bcmspace	TRUE	/* if (bcmspace) code is retained */
#endif

/* Max. nvram variable table size */
#define	MAXSZ_NVRAM_VARS	4096	/* XXX should be reduced */

/* How the locator reduces its memory footprint without #ifdef'ing
 *
 * The locator uses the weak external symbol feature of the linker
 * plus the compiler's ability to place each function in a unique
 * text section to allow wl_locator.c to provide an alternate, typically
 * trivial, implementation for many functions.
 *
 * Many of these routines would normally be static but they must
 * be external for this technique to work.  Instead of placing these function's
 * prototypes in a module's public header file and inviting an improper public
 * usage, use the below LOCATOR_EXTERN macro in the module implementation
 * file, both on the function declaration and definition.  This will cause
 * these functions to be static in all builds except locator builds.
 *
 * This methodology also allows the optimizer to possibly discard
 * dead (static) functions in non locator builds as well as provide
 * more explicit/grep'able documentation of functions used by the
 * locator in this way.
 *
 */

#ifdef WL_LOCATOR
#define LOCATOR_EXTERN
#else
#define LOCATOR_EXTERN static
#endif

#endif /* _bcmdefs_h_ */
