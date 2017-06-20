/* sysALib.s - Assembly support file and init routines */
	
/* $Id: sysALib.s,v 1.3 2011/07/21 16:14:13 yshtil Exp $
 * Copyright (c) 2005 Wind River Systems, Inc.
 *
 * The right to copy, distribute, modify, or otherwise make use
 * of this software may be licensed only pursuant to the terms
 * of an applicable Wind River license agreement.
 */

/*
modification history
--------------------
01a,03oct05,dtr  Created from cds85xx/01m
*/

#define _ASMLANGUAGE
#include <vxWorks.h>
#include <asm.h>
#include <config.h>
#include <sysLib.h>
#include <sysL2Cache.h>
#include <arch/ppc/mmuE500Lib.h>
	
	FUNC_EXPORT(sysInit)
	FUNC_EXPORT(_sysInit)
	FUNC_EXPORT(sysIvprSet)
	FUNC_EXPORT(vxL2CTLSet)
	FUNC_EXPORT(vxL2CTLGet)
	FUNC_EXPORT(sysCacheFlush)
	FUNC_IMPORT(usrInit)
	FUNC_EXPORT(sysInWord)
	FUNC_EXPORT(sysOutWord)
	FUNC_EXPORT(sysInLong)
	FUNC_EXPORT(sysOutLong)
        FUNC_EXPORT(sysInByte)
	FUNC_EXPORT(sysOutByte)
	FUNC_EXPORT(sysPciRead32)
	FUNC_EXPORT(sysPciWrite32)
	FUNC_EXPORT(sysPciInByte)
	FUNC_EXPORT(sysPciOutByte)
	FUNC_EXPORT(sysPciInWord)
	FUNC_EXPORT(sysPciOutWord)
	FUNC_EXPORT(sysPciInLong)
	FUNC_EXPORT(sysPciOutLong)
        FUNC_EXPORT(sysPCGet)   /* get the value of the PC register */
        FUNC_EXPORT(sysL1Csr1Set)
        FUNC_EXPORT(sysTimeBaseLGet)
#ifdef INCLUDE_L1_IPARITY_HDLR_INBSP
        FUNC_EXPORT(sysIParityHandler)
        FUNC_EXPORT(sysIvor1Set)
        FUNC_EXPORT(jumpIParity)
#endif  /* INCLUDE_L1_IPARITY_HDLR_INBSP */
	FUNC_EXPORT(disableBranchPrediction)
	FUNC_EXPORT(vxSpGet)
	FUNC_EXPORT(vxPcGet)
	FUNC_EXPORT(sysTas)
	_WRS_TEXT_SEG_START
        DATA_IMPORT(inFullVxWorksImage)
	
#define	CACHE_ALIGN_SHIFT	5	/* Cache line size == 2**5 */
	
FUNC_LABEL(_sysInit)		
FUNC_BEGIN(sysInit)

	mr  r8, r3
        xor p0,p0,p0
	mtspr TCR,p0
	xor   r6, r6, r6
	msync
	isync
	mtspr L1CSR0, r6		/* Disable the Data cache */
        li   r6, 0x0002              
	msync
	isync
	mtspr L1CSR0, r6		/* Invalidate the Data cache */
        li    r6, 0x0000              
        msync
	isync
	mtspr L1CSR1, r6	 /* Disable the Instrunction cache */
        li   r6, 0x0002              
        msync
	isync
	mtspr L1CSR1, r6	/* Invalidate the Instruction cache */
        isync
        li   r6, 0x0000              
	msync
	isync
	mtspr L1CSR1, r6        /* Disable the Instruction cache*/
	msync
	isync

        mbar 0

#ifdef INCLUDE_BRANCH_PREDICTION
        li   r6, _PPC_BUCSR_FI
        mtspr 1013,r6
        li   r6, _PPC_BUCSR_E
        mtspr 1013,r6
#endif

	lis  r6, HIADJ(inFullVxWorksImage)
	addi r6, r6,LO(inFullVxWorksImage)
	li   r7, TRUE
	stw  r7, 0(r6)

	/* initialize the stack pointer */
	
	lis     sp, HIADJ(RAM_LOW_ADRS)
	addi    sp, sp, LO(RAM_LOW_ADRS)
	addi    sp, sp, -FRAMEBASESZ    /* get frame stack */
	mr      r3, r8
	
	b usrInit
FUNC_END(sysInit)


	.balign 32
/******************************************************************************
*
* vxL2CTLSet - Set the value of the L@ cache control register
*
* This routine returns the value written.
*
* SYNOPSIS
* \ss
* UINT32 sysL2CTLSet
*     (
*     UINT32 value,
*     UINT32 *addr
*     )
* \se
*
* RETURNS: This routine returns the value in the L2CTL reg.
*/
	
FUNC_BEGIN(vxL2CTLSet)
	mbar	0 
	isync
	stw	p0,0(p1)
	lwz	p0,0(p1)
	mbar	0
	isync
	blr
FUNC_END(vxL2CTLSet)

	.balign 32
/******************************************************************************
*
* vxL2CTLGet - Get the value of the L2 cache control register
*
* SYNOPSIS
* \ss
* UINT32 sysL2CTLGet
*     (
*     UINT32 *addr
*     )
* \se
*
* RETURNS: This routine returns the value in the L2CTL reg.
*/
FUNC_BEGIN(vxL2CTLGet)
	mbar    0
	isync
	lwz	p1,0x0(p0)
	addi    p0,p1,0x0
	mbar    0
	isync
	blr
FUNC_END(vxL2CTLGet)

/******************************************************************************
* disableBranchPrediction - disables branch prediction 
*
* SYNOPSIS
* \ss
* void disableBranchPrediction
*     (
*     void
*     )
* \se
*
* RETURNS: NONE
*/
FUNC_BEGIN(disableBranchPrediction)
        mfspr p0, 1013
        andi. p1, p0,LO(~_PPC_BUCSR_E)
        isync
        mtspr 1013,p1
        isync
        li    p0, _PPC_BUCSR_FI
        mtspr 1013,p0
        isync
        blr
FUNC_END(disableBranchPrediction)


/****************************************************
 * sysCacheFlush just flushes cache - assume int lock
 * p0 - cache line num
 * p1 - buffer origin
 * p2 - cache align size
 */
	
FUNC_BEGIN(sysCacheFlush)
	/*
	 * p3 contains the count of cache lines to be fetched & flushed.
	 * Convert to a count of pages covered, and fetch a word from
	 * each page to ensure that all addresses involved are in
	 * the TLB so that reloads do not disrupt the flush loop.
	 * A simple shift without round-up is sufficient because
	 * the p3 value is always a multiple of the shift count.
	 */
	srwi	p3, p0, MMU_RPN_SHIFT - CACHE_ALIGN_SHIFT
	mtspr	CTR, p3
        addi    p6,p1,0	
        li      p5,MMU_PAGE_SIZE
	subf    p3,p5,p1
	
	/*
	 * There might be a page boundary between here and the end of
	 * the function, so make sure both pages are in the I-TLB.
	 */
	b	cacheL2DisableLoadItlb
cacheL2DisableLoadDtlb:
	add     p3,p3,p5
	lbzu	p4,0(p3)
	bdnz	cacheL2DisableLoadDtlb
	mtctr   p0         /* Load counter with number of cache lines */
	subf	p1, p2, p1 /* buffer points to text  - cache line size */
l2DisableFlush:
        add	p1, p2, p1		  /* +  cache line size */
	lbzu	p3, 0x0(p1)	       	  /* flush the data cache block */
        bdnz    l2DisableFlush     /* loop till cache ctr is zero */
	sync
	isync
	mtctr   p0         /* Load counter with number of cache lines */
        addi    p1, p6, 0
	subf	p1, p2, p1 /* buffer points to text  - cache line size */
l2DisableClear:
	add	p1, p2, p1  /* point to next cache line */
	dcbf	0,p1			    /* flush newly-loaded line */
	bdnz	l2DisableClear	    /* repeat for all sets and ways */
	sync
	isync	
	blr
cacheL2DisableLoadItlb:
	b	cacheL2DisableLoadDtlb
FUNC_END(sysCacheFlush)

/*****************************************************************************
*
* sysInByte - reads a byte from an io address.
*
* This function reads a byte from a specified io address.
*
* RETURNS: byte from address.

* UCHAR sysInByte
*     (
*     UCHAR *  pAddr		/@ Virtual I/O addr to read from @/
*     )

*/

FUNC_BEGIN(sysInByte)
	eieio			/* Sync I/O operation */
	sync
	lbzx	p0,r0,p0	/* Read byte from I/O space */
	bclr	20,0		/* Return to caller */
FUNC_END(sysInByte)
	
/******************************************************************************
*
* sysOutByte - writes a byte to an io address.
*
* This function writes a byte to a specified io address.
*
* RETURNS: N/A

* VOID sysOutByte
*     (
*     UCHAR *  pAddr,		/@ Virtual I/O addr to write to @/
*     UCHAR    data		/@ data to be written @/
*     )

*/

FUNC_BEGIN(sysOutByte)
	stbx	p1,r0,p0	/* Write a byte to PCI space */
	eieio			/* Sync I/O operation */
	sync
	bclr	20,0		/* Return to caller */
FUNC_END(sysOutByte)
	
/*****************************************************************************
*
* sysInWord - reads a word from an address, swapping the bytes.
*
* This function reads a swapped word from a specified 
* address.
*
* RETURNS:
* Returns swapped 16 bit data from the specified address.

* USHORT sysInWord
*     (
*     ULONG  address,		/@ addr to read from @/
*     )

*/

FUNC_BEGIN(sysInWord)
	eieio			/* Sync I/O operation */
	sync
        lhbrx   p0,r0,p0	/* Read and swap */
        bclr    20,0		/* Return to caller */
FUNC_END(sysInWord)


/*****************************************************************************
*
* sysOutWord - writes a word to an address swapping the bytes.
*
* This function writes a swapped word to a specified 
* address.
*
* RETURNS: N/A

* VOID sysOutWord
*     (
*     ULONG address,		/@ Virtual addr to write to @/
*     UINT16  data		/@ Data to be written       @/
*     )

*/

FUNC_BEGIN(sysOutWord)
        sthbrx  p1,r0,p0	/* Write with swap to address */
	eieio			/* Sync I/O operation */
	sync
        bclr    20,0		/* Return to caller */
FUNC_END(sysOutWord)

/*****************************************************************************
*
* sysInLong - reads a long from an address.
*
* This function reads a long from a specified PCI Config Space (little-endian)
* address.
*
* RETURNS:
* Returns 32 bit data from the specified register.  Note that for PCI systems
* if no target responds, the data returned to the CPU will be 0xffffffff.

* ULONG sysInLong
*     (
*     ULONG  address,		/@ Virtual addr to read from @/
*     )

*/

FUNC_BEGIN(sysInLong)
	eieio			/* Sync I/O operation */
	sync
        lwbrx   p0,r0,p0	/* Read and swap from address */
        bclr    20,0		/* Return to caller */
FUNC_END(sysInLong)


/******************************************************************************
*
* sysOutLong - write a swapped long to address.
*
* This routine will store a 32-bit data item (input as big-endian)
* into an address in little-endian mode.
*
* RETURNS: N/A

* VOID sysOutLong
*     (
*     ULONG   address,		/@ Virtual addr to write to @/
*     ULONG   data		/@ Data to be written @/
*     )

*/

FUNC_BEGIN(sysOutLong)
        stwbrx  p1,r0,p0	/* store data as little-endian */
	eieio			/* Sync I/O operation */
	sync
        bclr    20,0
FUNC_END(sysOutLong)

/******************************************************************************
*
* sysPciRead32 - read 32 bit PCI data
*
* This routine will read a 32-bit data item from PCI (I/O or
* memory) space.
*
* RETURNS: N/A

* VOID sysPciRead32
*     (
*     ULONG *  pAddr,		/@ Virtual addr to read from @/
*     ULONG *  pResult		/@ location to receive data @/
*     )

*/

FUNC_BEGIN(sysPciRead32)
	eieio			/* Sync I/O operation */
        lwbrx   p0,r0,p0	/* get the data and swap the bytes */
        stw     p0,0(p1)	/* store into address ptd. to by p1 */
        bclr    20,0
FUNC_END(sysPciRead32)

/******************************************************************************
*
* sysPciWrite32 - write a 32 bit data item to PCI space
*
* This routine will store a 32-bit data item (input as big-endian)
* into PCI (I/O or memory) space in little-endian mode.
*
* RETURNS: N/A

* VOID sysPciWrite32
*     (
*     ULONG *  pAddr,		/@ Virtual addr to write to @/
*     ULONG   data		/@ Data to be written @/
*     )

*/

FUNC_BEGIN(sysPciWrite32)
        stwbrx  p1,r0,p0	/* store data as little-endian */
        bclr    20,0
FUNC_END(sysPciWrite32)

/*****************************************************************************
*
* sysPciInByte - reads a byte from PCI Config Space.
*
* This function reads a byte from a specified PCI Config Space address.
*
* RETURNS:
* Returns 8 bit data from the specified register.  Note that for PCI systems
* if no target responds, the data returned to the CPU will be 0xff.

* UINT8 sysPciInByte
*     (
*     UINT8 *  pAddr,		/@ Virtual addr to read from @/
*     )

*/

FUNC_BEGIN(sysPciInByte)
	eieio			/* Sync I/O operation */
        lbzx    p0,r0,p0	/* Read byte from PCI space */
        bclr    20,0		/* Return to caller */
FUNC_END(sysPciInByte)
	
/*****************************************************************************
*
* sysPciInWord - reads a word (16-bit big-endian) from PCI Config Space.
*
* This function reads a word from a specified PCI Config Space (little-endian)
* address.
*
* RETURNS:
* Returns 16 bit data from the specified register.  Note that for PCI systems
* if no target responds, the data returned to the CPU will be 0xffff.

* USHORT sysPciInWord
*     (
*     USHORT *  pAddr,		/@ Virtual addr to read from @/
*     )

*/

FUNC_BEGIN(sysPciInWord)
	eieio			/* Sync I/O operation */
        lhbrx   p0,r0,p0	/* Read and swap from PCI space */
        bclr    20,0		/* Return to caller */
FUNC_END(sysPciInWord)
	
/*****************************************************************************
*
* sysPciInLong - reads a long (32-bit big-endian) from PCI Config Space.
*
* This function reads a long from a specified PCI Config Space (little-endian)
* address.
*
* RETURNS:
* Returns 32 bit data from the specified register.  Note that for PCI systems
* if no target responds, the data returned to the CPU will be 0xffffffff.

* ULONG sysPciInLong
*     (
*     ULONG *  pAddr,		/@ Virtual addr to read from @/
*     )

*/

FUNC_BEGIN(sysPciInLong)
	eieio			/* Sync I/O operation */
        lwbrx   p0,r0,p0	/* Read and swap from PCI space */
        bclr    20,0		/* Return to caller */
FUNC_END(sysPciInLong)
	
/******************************************************************************
*
* sysPciOutByte - writes a byte to PCI Config Space.
*
* This function writes a byte to a specified PCI Config Space address.
*
* RETURNS: N/A

* VOID sysPciOutByte
*     (
*     UINT8 *  pAddr,		/@ Virtual addr to write to @/
*     UINT8  data		/@ Data to be written       @/
*     )

*/

FUNC_BEGIN(sysPciOutByte)
        stbx    p1,r0,p0	/* Write a byte to PCI space */
        bclr    20,0		/* Return to caller */
FUNC_END(sysPciOutByte)
	
/******************************************************************************
*
* sysPciOutWord - writes a word (16-bit big-endian) to PCI Config Space.
*
* This function writes a word to a specified PCI Config Space (little-endian)
* address.
*
* RETURNS: N/A

* VOID sysPciOutWord
*     (
*     USHORT *  pAddr,		/@ Virtual addr to write to @/
*     USHORT  data		/@ Data to be written       @/
*     )

*/

FUNC_BEGIN(sysPciOutWord)
        sthbrx  p1,r0,p0	/* Write with swap to PCI space */
        bclr    20,0		/* Return to caller */
FUNC_END(sysPciOutWord)
	
/******************************************************************************
*
* sysPciOutLong - writes a long (32-bit big-endian) to PCI Config Space.
*
* This function writes a long to a specified PCI Config Space (little-endian)
* address.
*
* RETURNS: N/A

* VOID sysPciOutLong
*     (
*     ULONG *  pAddr,		/@ Virtual addr to write to @/
*     ULONG  data		/@ Data to be written       @/
*     )

*/

FUNC_BEGIN(sysPciOutLong)
        stwbrx  p1,r0,p0	/* Write big-endian long to little-endian */
        mr      p0,p1		/* PCI space */
        bclr    20,0		/* Return to caller */
FUNC_END(sysPciOutLong)

/******************************************************************************
*
* sysPCGet - Get the value of the PC (Program Counter)
*
* This routine returns the value of the PC.
*
* SYNOPSIS
* \ss
* UINT32 sysPCGet
*     (
*     void
*     )
* \se
*
* RETURNS: the Program Counter Register (PC) value.
*/

FUNC_BEGIN(sysPCGet)
        mflr	r4 /* Save LR value */

        bl      Next    /* Set PC */
Next:
        mflr    r3 /* Get PC */
		mtlr    r4 /* Restor LR value */
	blr
FUNC_END(sysPCGet)

/******************************************************************************
*
* sysTimeBaseLGet - Get lower half of Time Base Register
*
* SYNOPSIS
* \ss
* UINT32 sysTimeBaseLGet(void)
* \se
*
* This routine will read the contents the lower half of the Time
* Base Register (TBL - TBR 268).
*
* RETURNS: value of TBR 268 (in r3)
*/

FUNC_BEGIN(sysTimeBaseLGet)
    mfspr       r3, 268
    bclr        20,0                    /* Return to caller */
FUNC_END(sysTimeBaseLGet)

/******************************************************************************
*
* sysL1Csr1Set - Set the value of L1CSR1
*
* SYNOPSIS
* \ss
* void sysL1Csr1Set
*     (
*     UINT32
*     )
* \se
*
* RETURNS: none
*/

FUNC_BEGIN(sysL1Csr1Set)
        msync
        isync
        mtspr   L1CSR1, r3
        msync
        isync
        blr
FUNC_END(sysL1Csr1Set)

FUNC_BEGIN(sysIvprSet)
		mtspr  IVPR,p0
		blr
FUNC_END(sysIvprSet)

#if defined(INCLUDE_L1_IPARITY_HDLR)
# include "sysL1ICacheParity.s"
#elif defined(INCLUDE_L1_IPARITY_HDLR_INBSP)
	
#define DETECT_EXCHDL_ADRS(ivor)  \
        mfspr   p0, IVPR;         \
        mfspr   p1, ivor;         \
        or      p1, p1, p0;       \
        mfspr   p0, MCSRR0;       \
        cmpw    p0, p1;           \
        beq     faultDetected;

/*********************************************************************
 *
 * sysIParityHandler - This routine is call for a machine check. 
 * This routine will invalidate the instruction cache for the address 
 * in MCSRRO. If only instruction parity error then it will return from
 * machine check else it will go to standard machine check handler.
 */ 
FUNC_BEGIN(sysIParityHandler)
        /* Save registers used */
     
        mtspr   SPRG4_W ,p0
        mtspr   SPRG5_W ,p1
        mfcr    p0
        mtspr   SPRG6_W ,p0
            
        /* check for ICPERR */
        mfspr   p1, MCSR
        andis.  p1, p1, 0x4000
        beq     ppcE500Mch_norm

        /* check if mcsrr0 is pointing to 1st instr of exception handler */
        DETECT_EXCHDL_ADRS(IVOR0)
        DETECT_EXCHDL_ADRS(IVOR1)
        DETECT_EXCHDL_ADRS(IVOR2)
        DETECT_EXCHDL_ADRS(IVOR3)
        DETECT_EXCHDL_ADRS(IVOR4)
        DETECT_EXCHDL_ADRS(IVOR5)
        DETECT_EXCHDL_ADRS(IVOR6)
        DETECT_EXCHDL_ADRS(IVOR8)
        DETECT_EXCHDL_ADRS(IVOR10)
        DETECT_EXCHDL_ADRS(IVOR11)
        DETECT_EXCHDL_ADRS(IVOR12)
        DETECT_EXCHDL_ADRS(IVOR13)
        DETECT_EXCHDL_ADRS(IVOR14)
        DETECT_EXCHDL_ADRS(IVOR15)
        DETECT_EXCHDL_ADRS(IVOR32)
        DETECT_EXCHDL_ADRS(IVOR33)
        DETECT_EXCHDL_ADRS(IVOR34)
        DETECT_EXCHDL_ADRS(IVOR35)

        /* p0 here has mcsrr0 value, round to cache line boundary */
        rlwinm  p0, p0, 0, 0, 31 - CACHE_ALIGN_SHIFT 
        /* invalidate instruction cache */
        icbi    r0, p0
        isync
#ifdef INCLUDE_SHOW_ROUTINES
        /* Add 1 to instrParityCount to measure no of parity errors */ 
        lis     p0, HIADJ(instrParityCount)
        addi    p0, p0, LO(instrParityCount)
        lwz     p1, 0(p0)
        addi    p1, p1, 1
        stw     p1, 0(p0)
#endif
        /* return after invalidate */
        mfspr   p0, SPRG6_R
        mtcr    p0
        mfspr   p0, SPRG4_R
        mfspr   p1, SPRG5_R
        isync     
        rfmci /*.long   0x4c00004c*/

ppcE500Mch_norm:
        mfspr   p0, SPRG6_R
        mtcr    p0
        mfspr   p0, SPRG4_R
        mfspr   p1, SPRG5_R
        ba      0x200       /* _EXC_OFF_MACH */

faultDetected:
        /* rebooting, no need to save regs */
        bl      chipErrataCpu29Print
        li      p0, BOOT_NORMAL
        b       sysToMonitor      /* reset */
FUNC_END(sysIParityHandler)

/* Branch to above handler copied to _EXC_OFF_END */
FUNC_BEGIN(jumpIParity)
	ba     sysIParityHandler
FUNC_END(jumpIParity)

/****************************************************************
 * sysIvor1Set - fills in value of IVOR1 register to override 
 * standard machine check handler for L1 instruction parity recovery.
 */
FUNC_BEGIN(sysIvor1Set)
        mtspr IVOR1, p0
	blr
FUNC_END(sysIvor1Set)

#endif  /* INCLUDE_L1_IPARITY_HDLR */

/*******************************************************************************
*
* vxTas - this routine performs the atomic test and set for the PowerPC arch.
*
* RETURN: None.
*/

FUNC_BEGIN(sysTas)
	lis	r4, 0x8000	/* set the upper bit of r4 */
	ori     r4, r4, 0x0000
	eieio			/* simple ordered store using eieio */

	lwarx	r5, 0, r3	/* load and reserve */
	cmpwi	r5, 0		/* done if word */
	bne	sysTasEnd	/* not equal to 0 */

	stwcx.	r4, 0, r3	/* try to store non-zero */
	eieio			/* preserve load/store order */
	bne-	sysTas
	li	r3, 0x01
	blr
sysTasEnd:
	li	r3, 0
	blr
FUNC_END(sysTas)

vxSpGet:
	or	r3, r1, r1
	blr

vxPcGet:
	mfspr	r3,LR
	blr


