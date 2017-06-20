
/* sysALib.s - generic PPC 603/604 system-dependent assembly routines 
 * Copyright 1984-1998 Wind River Systems, Inc. 
 * Copyright 1996-1998 Motorola, Inc. 


modification history
--------------------
01a,08Mar99,My	Copied from Yellowknife platform



DESCRIPTION
This module contains the entry code, sysInit(), for VxWorks images that start
running from RAM, such as 'vxWorks'. These images are loaded into memory
by some external program (e.g., a boot ROM) and then started.
The routine sysInit() must come first in the text segment. Its job is to perform
the minimal setup needed to call the generic C
routine usrInit() with parameter BOOT_COLD.

The routine sysInit() typically masks interrupts in the processor, sets the
initial stack pointer to _sysInit then jumps to usrInit.
Most other hardware and device initialization is performed later by
sysHwInit().
*/

/* includes */

#define _ASMLANGUAGE
#include "vxWorks.h"
#include "sysLib.h"
#include "config.h"
#include "regs.h"	
#include "asm.h"
#include "sysL2Backcache.h"

/* defines */

/* $Id: sysALib.s,v 1.3 2011/07/21 16:14:08 yshtil Exp $
 * Some releases of h/arch/ppc/toolPpc.h had bad definitions of
 * LOADPTR and LOADVAR. So we will define it correctly.
 * [REMOVE THESE FOR NEXT MAJOR RELEASE].
 *
 * LOADPTR initializes a register with a 32 bit constant, presumably the
 * address of something.
 */

#undef LOADPTR
#define	LOADPTR(reg,const32) \
	  addis reg,r0,HIADJ(const32); addi reg,reg,LO(const32)

/*
 * LOADVAR initializes a register with the contents of a specified memory
 * address. The difference being that the value loaded is the contents of
 * the memory location and not just the address of it.
 */

#undef LOADVAR
#define	LOADVAR(reg,addr32) \
	  addis reg,r0,HIADJ(addr32); lwz reg,LO(addr32)(reg)


/* globals */

	.globl	_sysInit		/* start of system code */
	.globl  sysInByte
	.globl  sysOutByte
        .globl  sysPciRead32
        .globl  sysPciWrite32
        .globl  sysPciInByte
        .globl  sysPciOutByte
        .globl  sysPciInWord
        .globl  sysPciOutWord
        .globl  sysPciInLong
        .globl  sysPciOutLong
        .globl  sysMemProbeSup
	.globl	sysPVRReadSys
	.globl	sysL2CRRead
	.globl	sysGetPPCTBU
	.globl	sysGetPPCTBL
	.globl	vxHid1Get
	.globl	vxSpGet
	.globl	vxPcGet
	.globl	SIO_OUT

	/* externals */

	.extern usrInit
	
	.text




/*******************************************************************************
*
* sysInit - start after boot
*
* This is the system start-up entry point for VxWorks in RAM, the
* first code executed after booting.  It disables interrupts, sets up
* the stack, and jumps to the C routine usrInit() in usrConfig.c.
*
* The initial stack is set to grow down from the address of sysInit().  This
* stack is used only by usrInit() and is never used again.  Memory for the
* stack must be accounted for when determining the system load address.
*
* NOTE: This routine should not be called by the user.
*
* RETURNS: N/A

* sysInit (void)              /@ THIS IS NOT A CALLABLE ROUTINE @/

*/

_sysInit:

	/* disable interrupts */

	xor	p0, p0, p0
        mtmsr   p0                      /* clear the MSR register  */


        /* insert protection from decrementer exceptions */

	xor	p0, p0, p0
	LOADPTR (p1, 0x4c000064)        /* load rfi (0x4c000064) to p1      */
        stw     p1, 0x900(r0)           /* store rfi at 0x00000900          */

	/* initialize the stack pointer */
	
	LOADPTR (sp, _sysInit)
	addi	sp, sp, -FRAMEBASESZ
	
	/* disable instruction and data caches */

        mfspr   r28, PVR
        rlwinm  r28, r28, 16, 16, 31

        cmpli   0, 0, r28, CPU_TYPE_604R
        beq     cpuType604R

	LOADPTR (p0, (_PPC_HID0_ICFI | _PPC_HID0_DCFI))
	sync
	isync
	mtspr	HID0, p0		/* first invalidate I and D caches */
        b       cacheInvalidateDone

cpuType604R:
        li      p0, 0
        mtspr   HID0, p0        /* disable the caches */
        isync

        /* disable BTAC by setting bit 30 */

	LOADPTR (p0, (_PPC_HID0_ICFI | _PPC_HID0_DCFI | 0x0002))
        mtspr   HID0, p0

cacheInvalidateDone:

	/*
	 * Disable sequential instruction execution and 
	 * enable branch history table for the 604
	 */
	
	mfspr	p1,HID0
	ori	p1,p1,(_PPC_HID0_SIED | _PPC_HID0_BHTE )
	mtspr	HID0,p1
	
	/* disable instruction and data translations in the MMU */

	sync
	mfmsr	p0			/* get the value in msr */
					/* clear bits IR and DR */
	
	rlwinm	p1, p0, 0, _PPC_MSR_BIT_DR+1, _PPC_MSR_BIT_IR - 1
	
	mtmsr	p1			/* set the msr */
	sync				/* SYNC */

	/* initialize the BAT registers */

	li	p3,0	 		/* clear p3 */
	
	isync
	mtspr	IBAT0U,p3
	isync
	mtspr	IBAT0L,p3
	isync
	mtspr	IBAT1U,p3
	isync
	mtspr	IBAT1L,p3
	isync
	mtspr	IBAT2U,p3
	isync
	mtspr	IBAT2L,p3
	isync
	mtspr	IBAT3U,p3
	isync
	mtspr	IBAT3L,p3
	isync
	mtspr	DBAT0U,p3
	isync
	mtspr	DBAT0L,p3
	isync
	mtspr	DBAT1U,p3
	isync
	mtspr	DBAT1L,p3
	isync
	mtspr	DBAT2U,p3
	isync
	mtspr	DBAT2L,p3
	isync
	mtspr	DBAT3U,p3
	isync
	mtspr	DBAT3L,p3
	isync


	/* invalidate entries within both TLBs */

	li	p1,128
	xor	p0,p0,p0		/* p0 = 0    */
	mtctr	p1			/* CTR = 128  */

	isync				/* context sync req'd before tlbie */
sysALoop:
	tlbie	p0
	addi	p0,p0,0x1000		/* increment bits 15-19 */
	bdnz	sysALoop		/* decrement CTR, branch if CTR != 0 */
	sync				/* sync instr req'd after tlbie      */

	/* initialize Small Data Area (SDA) start address */

#if	FALSE				/* XXX TPR NO SDA for now */
	LOADPTR (r2, _SDA2_BASE_)
	LOADPTR (r13, _SDA_BASE_)
#endif

	li      p0, BOOT_WARM_AUTOBOOT
	b	usrInit			/* never returns - starts up kernel */


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

sysInByte:
	eieio			/* Sync I/O operation */
	lbzx	p0,r0,p0	/* Read byte from I/O space */
	bclr	20,0		/* Return to caller */

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

sysOutByte:
	stbx	p1,r0,p0	/* Write a byte to PCI space */
	bclr	20,0		/* Return to caller */


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

sysPciRead32:
	eieio			/* Sync I/O operation */
        lwbrx   p0,r0,p0	/* get the data and swap the bytes */
        stw     p0,0(p1)	/* store into address ptd. to by p1 */
        bclr    20,0


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

sysPciWrite32:
        stwbrx  p1,r0,p0	/* store data as little-endian */
        bclr    20,0


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

sysPciInByte:
	eieio			/* Sync I/O operation */
        lbzx    p0,r0,p0	/* Read byte from PCI space */
        bclr    20,0		/* Return to caller */

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

sysPciInWord:
	eieio			/* Sync I/O operation */
        lhbrx   p0,r0,p0	/* Read and swap from PCI space */
        bclr    20,0		/* Return to caller */

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

sysPciInLong:
	eieio			/* Sync I/O operation */
        lwbrx   p0,r0,p0	/* Read and swap from PCI space */
        bclr    20,0		/* Return to caller */

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

sysPciOutByte:
        stbx    p1,r0,p0	/* Write a byte to PCI space */
        bclr    20,0		/* Return to caller */

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

sysPciOutWord:
        sthbrx  p1,r0,p0	/* Write with swap to PCI space */
        bclr    20,0		/* Return to caller */

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

sysPciOutLong:
        stwbrx  p1,r0,p0	/* Write big-endian long to little-endian */
        mr      p0,p1		/* PCI space */
        bclr    20,0		/* Return to caller */

/*******************************************************************************
*
* sysMemProbeSup - sysBusProbe support routine
*
* This routine is called to try to read byte, word, or long, as specified
* by length, from the specified source to the specified destination.
*
* RETURNS: OK if successful probe, else ERROR

* STATUS sysMemProbeSup
*     (
*     int         length, /@ length of cell to test (1, 2, 4) @/
*     char *      src,    /@ address to read @/
*     char *      dest    /@ address to write @/
*     )

*/

sysMemProbeSup:
        addi    p7, p0, 0       /* save length to p7 */
        xor     p0, p0, p0      /* set return status */
        cmpwi   p7, 1           /* check for byte access */
        bne     sbpShort        /* no, go check for short word access */
        lbz     p6, 0(p1)       /* load byte from source */
        stb     p6, 0(p2)       /* store byte to destination */
        isync                   /* enforce for immediate exception handling */
        blr
sbpShort:
        cmpwi   p7, 2           /* check for short word access */
        bne     sbpWord         /* no, check for word access */
        lhz     p6, 0(p1)       /* load half word from source */
        sth     p6, 0(p2)       /* store half word to destination */
        isync                   /* enforce for immediate exception handling */
        blr
sbpWord:
        cmpwi   p7, 4           /* check for short word access */
        bne     sysProbeExc     /* no, check for double word access */
        lwz     p6, 0(p1)       /* load half word from source */
        stw     p6, 0(p2)       /* store half word to destination */
        isync                   /* enforce for immediate exception handling */
        blr
sysProbeExc:
        li      p0, -1          /* shouldn't ever get here, but... */
        blr



/*********************************************************************
 *  sysL2CRRead - Read the content of the L2CR register, which 
 *  controls the L2 Backside cache on MPC750.                         
 *  Input  - None					    
 *  Output - value of l2cr
 */				   

sysL2CRRead:
	mfspr	r3, L2CR_REG 
	blr



/**********************************************************************
 *  sysPVRReadSys - Read the content of the PVR register
 *	Once the PVR is read, the 16 least significant bits are shifted
 *	off.
 *  Input  - None
 *  Return - upper 16 bits of PVR stored in r3
 */

sysPVRReadSys:

	mfspr r4, PVR_REG		/* read PVR  */
	rlwinm	r3, r4, 16, 16, 31
	blr

/**********************************************************************
 * Miscellaneous utility routines
 */

/*
 * Mark(#if 0) temporarily for compiler error on vxWorks 5.5.
 * Suggest by WindRiver.
 */
#if 0
vxHid1Get:				/* Not in vxWorks */
	.set	HID1, 1009
	mfspr	r3,HID1
	blr
#endif

vxSpGet:
	or	r3, r1, r1
	blr

vxPcGet:
	mfspr	r3,LR
	blr

/**********************************************************************
 *  void SIO_OUT(int char);
 *
 *  Configure Mousse COM1 to 9600 baud and write a character.
 *  Should be usable basically at any time.
 *
 *  r3 = character to output
 */

SIO_OUT:
	/* r4 = intLock() */
	mfmsr	r4
	rlwinm	r5,r4,0,17,15
	mtmsr	r5
	isync

	/* r5 = serial port address */
	LOADPTR(r5, 0xffe08080)

	/* Init serial port */
	li	r6, 0x83
	stb	r6, 3(r5)
	li	r6, 0x78
	stb	r6, 0(r5)
	li	r6, 0x00
	stb	r6, 1(r5)
	li	r6, 0x03
	stb	r6, 3(r5)
	li	r6, 0x0b
	stb	r6, 4(r5)

	/* Wait for transmit buffer available */
sowait:
	lbz	r6, 5(r5)
	andi.	r0, r6, 0x40
	bc	12, 2, sowait

	/* Transmit byte */
	stb	r3, 0(r5)

	/* intUnlock(r4) */
	rlwinm	r4,r4,0,16,16
	mfmsr	r5
	or	r4,r4,r5
	mtmsr	r4
	isync

	blr
