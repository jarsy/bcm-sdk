/* romInit.s - general PPC 603/604 ROM initialization module
 * Copyright 1984-1998 Wind River Systems, Inc.
 * Copyright 1996-1998 Motorola, Inc.


modification history
--------------------
01g,26sep02, jmb Init RCS2 early to enable PLD and LEDs
01f,20aug02, jmb Remove the ppcboot code
01e,28jun02, jmb Recognize more PLL jumper patterns.
01d,22may02, jmb Fix "booting" print for 100 MHz RAM clock
01c,22Apr02, jmb Fix bus-width of RCS1 in MCCR4 ...it's 8-bit.
01b,29Mar02, jmb Modified to configure either 8240 or 8245 (Kahlua2)
                 "jmb" is Jimmy Blair, Broadcom
01a,02Feb99, My  Copied from Yellowknife platform and deleted unused code.


DESCRIPTION
This module contains the entry code for VxWorks images that start
running from ROM, such as 'bootrom' and 'vxWorks_rom'.
The entry point, romInit(), is the first code executed on power-up.
It performs the minimal setup needed to call
the generic C routine romStart() with parameter BOOT_COLD.

RomInit() typically masks interrupts in the processor, sets the initial
stack pointer (to STACK_ADRS which is defined in configAll.h), and
readies system memory by configuring the DRAM controller if necessary.
Other hardware and device initialization is performed later in the
BSP's sysHwInit() routine.

A second entry point in romInit.s is called romInitWarm(). It is called
by sysToMonitor() in sysLib.c to perform a warm boot.
The warm-start entry point must be written to allow a parameter on
the stack to be passed to romStart().

Execution environment:  
 
  Prior to SDRAM initialization, there is no data memory available,
  and consequently, no stack. However at least one level of subroutine
  call can be made, and possibly two, if the link register is saved in
  r28. If more persistant registers are needed, start with r22 and
  work down, but make sure such usage will not interfere with the SPD
  code.
        
Register usage:

  r0 - r22  are scratch registers, and are not saved across calls.

  r23   link register save
  r24   REG_ADDR
  r25   REG_DATA
  r26   K1 CPU ID
  r27   K2 CPU ID
  r28   CPU Type (until cacheEnableDone)
  r29   Hardware info (see doc below)
  r30   start_type (warm/cold)
  r31   unused
  
*/

#define	_ASMLANGUAGE
#include "vxWorks.h"
#include "sysLib.h"
#include "asm.h"
#include "config.h"
#include "regs.h"
#include "mpc107.h"
#include "sdram_spd.h"


/* defines */

/* $Id: romInit.s,v 1.4 2011/07/21 16:14:08 yshtil Exp $
 * Some releases of h/arch/ppc/toolPpc.h had a bad definition of
 * LOADPTR. So we will define it correctly. [REMOVE WHEN NO LONGER NEEDED].
 *
 * LOADPTR initializes a register with a 32 bit constant, presumably the
 * address of something.
 */

#undef LOADPTR
#define	LOADPTR(reg,const32) \
	  addis reg,r0,HIADJ(const32); addi reg,reg,LO(const32)

#define PUTC(ch)	li r3, ch; bl SEROUT


/* Set up MPC8245 memory configuration constants 
 
   Initial memory controller parameters are encoded in r29       

   Board info is encoded in 32 bits:

   00000000001111111111222222222233
   01234567890123456789012345678901
   --------------------------------
   ppppppppppppppppslbrmmmmbbbbbbbb     

   p    PCI device id
   s    SPD Probe Result
         1 = SPD error
         0 = SPD success
   l    Logical banks 
         0 = 2 logical banks
         1 = 4 logical banks
   b    Physical banks
         0 = 1 physical banks
         1 = 2 physical banks
   r    Device row addresses
         0 = 4096 row addresses
         1 = 8192 row addresses
   m    Physical bank size in 64MB units
         1 = 64MB
         2 = 128MB
         4 = 256MB
 */

#define MC1_ROW13_BANKBITS 0xAAAA
#define MC2_REFINT_ROW12   0x3a5        /* Values are conservative */
#define MC2_REFINT_ROW13   0x1d2

#define MC_MSAR1_64   0x4000
#define MC_XSAR1_64   0x0000
#define MC_MEAR1_64   0x7f3f
#define MC_XEAR1_64   0x0000

#define MC_MSAR1_128  0x8000
#define MC_XSAR1_128  0x0000
#define MC_MEAR1_128  0xff7f
#define MC_XEAR1_128  0x0000

#define MC_MSAR1_256  0x0000
#define MC_XSAR1_256  0x0100
#define MC_MEAR1_256  0xffff
#define MC_XEAR1_256  0x0100
        
#if SDRAM_ROWS == 13
#define MC1_BANKBITS 0xAAAA
#define MEM_DEFAULT_ROW SPD_MEM_CONF_ROW
#elif SDRAM_ROWS == 12
#define MC1_BANKBITS 0x0000
#define MEM_DEFAULT_ROW 0
#else
#error "SDRAM_ROWS must be 12 or 13"
#endif /* SDRAM_ROWS */


        
#if SDRAM_BANKS == 2
#define MEM_DEFAULT_BANK SPD_MEM_CONF_BANK
#elif SDRAM_BANKS == 1
#define MEM_DEFAULT_BANK 0
#else
#error "SDRAM_BANKS must be 1 or 2"
#endif /* SDRAM_BANKS */

#define MEM_DEFAULT_SZ ((SDRAM_BANK_SIZE & 0x03C0)<<2)
#define MEM_CONF_DEFAULT \
  (SPD_MEM_CONF_LBNK|MEM_DEFAULT_ROW|MEM_DEFAULT_BANK|MEM_DEFAULT_SZ)

	/* Exported internal functions */

	.data
	.globl	_romInit	/* start of system code */
	.globl	romInit		/* start of system code */
	.globl	_romInitWarm	/* start of system code */
	.globl	romInitWarm	/* start of system code */
	.globl	SEROUT		/* useful serial output routine */

	/* externals */

	.extern romStart	/* system initialization routine */

	.text
	.align 2


/******************************************************************************
*
* romInit - entry point for VxWorks in ROM
*

* romInit
*     (
*     int startType	/@ only used by 2nd entry point @/
*     )

*/

#ifndef ROM_RESIDENT
/*  Offset 0x100  by reserving 0x100 bytes of zero before the first
 *   executable instruction so we start at
 *   the reset vector address of 0x100
 *   This allows our old DataIO ROM burner to burn the ROM's correctly,
 *   and this works for all the targets except the ROM resident targets,
 *   hence why we encapsulate this pseudop with #ifndef ROM_RESIDENT.
 *   It is also necessary to manually change the Makefile macro
 *   HEX_FLAGS to -a 100 for the ROM_RESIDENT targets.
 */
	.space (0x100)
#endif	/*  ROM_RESIDENT  */

_romInit:
romInit:

	/* This is the cold boot entry (ROM_TEXT_ADRS) */

	bl	cold

	/*
	 * This warm boot entry point is defined as ROM_WARM_ADRS in config.h.
	 * It is defined as an offset from ROM_TEXT_ADRS.  Please make sure
	 * that the offset from _romInit to romInitWarm matches that specified
	 * in config.h
	 */

_romInitWarm:
romInitWarm:
	bl	warm

	/* copyright notice appears at beginning of ROM (in TEXT segment) */

	.ascii	 "Copyright 1984-1998 Wind River Systems, Inc."
	.align 2

cold:
	li	r30, BOOT_COLD
	bl	start		/* skip over next instruction */


warm:
	or	r30, r3, r3	/* startType to r30 */

start:
	/* Zero-out registers */

	addis	r0,r0,0
	mtspr	SPRG0,r0
	mtspr	SPRG1,r0
	mtspr	SPRG2,r0
	mtspr	SPRG3,r0

	/* initialize the stack pointer */

	LOADPTR (sp, STACK_ADRS)

	/* Set MPU/MSR to a known state. Turn on FP */

	LOADPTR (r3, _PPC_MSR_FP)
	sync
	mtmsr	r3
	isync

	/* Init the floating point control/status register */

	mtfsfi	7,0x0
	mtfsfi	6,0x0
	mtfsfi	5,0x0
	mtfsfi	4,0x0
	mtfsfi	3,0x0
	mtfsfi	2,0x0
	mtfsfi	1,0x0
	mtfsfi	0,0x0
	isync

	/* Set MPU/MSR to a known state. Turn off FP */

#if 0	/* Code removed so FP stays on */
        andi.	r3, r3, 0
	sync
	mtmsr 	r3
	isync
#endif

	/* Init the Segment registers */

	andi.	r3, r3, 0
	isync
	mtsr	0,r3
	isync
	mtsr	1,r3
	isync
	mtsr	2,r3
	isync
	mtsr	3,r3
	isync
	mtsr	4,r3
	isync
	mtsr	5,r3
	isync
	mtsr	6,r3
	isync
	mtsr	7,r3
	isync
	mtsr	8,r3
	isync
	mtsr	9,r3
	isync
	mtsr	10,r3
	isync
	mtsr	11,r3
	isync
	mtsr	12,r3
	isync
	mtsr	13,r3
	isync
	mtsr	14,r3
	isync
	mtsr	15,r3
	isync

	/* Turn off data and instruction cache control bits */

	mfspr	r3, HID0
	isync
	rlwinm	r4, r3, 0, 18, 15	/* r4 has ICE and DCE bits cleared */
	sync
	isync
	mtspr	HID0, r4		/* HID0 = r4 */
	isync

	/* Get cpu type */

	mfspr	r28, PVR
	rlwinm	r28, r28, 16, 16, 31

	/* invalidate the MPU's data/instruction caches */

	lis	r3, 0x0
	cmpli	0, 0, r28, CPU_TYPE_603
	beq	cpuIs603
	cmpli	0, 0, r28, CPU_TYPE_603E
	beq	cpuIs603
	cmpli	0, 0, r28, CPU_TYPE_603P
	beq	cpuIs603
	cmpli	0, 0, r28, CPU_TYPE_604R
	bne	cpuNot604R

cpuIs604R:
	lis	r3, 0x0
	mtspr	HID0, r3		/* disable the caches */
	isync
	ori	r4, r4, 0x0002		/* disable BTAC by setting bit 30 */

cpuNot604R:
	ori	r3, r3, (_PPC_HID0_ICFI | _PPC_HID0_DCFI)

cpuIs603:
	ori	r3, r3, (_PPC_HID0_ICE | _PPC_HID0_DCE)
	or	r4, r4, r3		/* set bits */
	sync
	isync
	mtspr	HID0, r4		/* HID0 = r4 */
	andc	r4, r4, r3		/* clear bits */
	isync
	cmpli	0, 0, r28, CPU_TYPE_604
	beq	cpuIs604
	cmpli	0, 0, r28, CPU_TYPE_604E
	beq	cpuIs604
	cmpli	0, 0, r28, CPU_TYPE_604R
	beq	cpuIs604
	mtspr	HID0, r4
	isync

#ifdef USER_I_CACHE_ENABLE
	b	instCacheOn603
#else
	b	cacheEnableDone
#endif

cpuIs604:
	LOADPTR (r5, 0x1000)		/* loop count, 0x1000 */
	mtspr	CTR, r5
loopDelay:
	nop
	bdnz	loopDelay
	isync
	mtspr	HID0, r4
	isync

	/* turn the Instruction cache ON for faster FLASH ROM boots */

#ifdef USER_I_CACHE_ENABLE

	ori	r4, r4, (_PPC_HID0_ICE | _PPC_HID0_ICFI)
	isync				/* Synchronize for ICE enable */
	b	writeReg4
instCacheOn603:
	ori	r4, r4, (_PPC_HID0_ICE | _PPC_HID0_ICFI)
	rlwinm	r3, r4, 0, 21, 19	/* clear the ICFI bit */

	/*
         * The setting of the instruction cache enable (ICE) bit must be
         * preceded by an isync instruction to prevent the cache from being
         * enabled or disabled while an instruction access is in progress.
         */
	isync
writeReg4:
	mtspr	HID0, r4		/* Enable Instr Cache & Inval cache */
	cmpli	0, 0, r28, CPU_TYPE_604
	beq	cacheEnableDone
	cmpli	0, 0, r28, CPU_TYPE_604E
	beq	cacheEnableDone

	mtspr	HID0, r3		/* using 2 consec instructions */
					/* PPC603 recommendation */
#endif
cacheEnableDone:

	/* Detect map A or B */

	addis	r25,r0, HI(CHRP_REG_ADDR)
	addis	r24,r0, HI(CHRP_REG_DATA)

	LOADPTR (r26, KAHLUA_ID)		/* Kahlua PCI controller ID */
	LOADPTR (r8, BMC_BASE)

	stwbrx	r8,0,(r25)
	lwbrx	r3,0,(r24)		/* Store read value to r3 */
	cmp	0,0,r3,r26
	beq	cr0, X4_KAHLUA_START

        /* It's not an 8240, is it an 8245? */

	LOADPTR (r26, KAHLUA2_ID)	/* Kahlua PCI controller ID */
	cmp	0,0,r3,r26
	beq	cr0, X4_KAHLUA_START

        /* Save the PCI controller type in r26 */
	mr	r26, r3

	LOADPTR (r25, PREP_REG_ADDR)
	LOADPTR (r24, PREP_REG_DATA)

X4_KAHLUA_START:
	/* MPC8245 changes begin here */
	LOADPTR (r3, PCI_COMMAND)	/* PCI command reg */
	stwbrx	r3,0,r25
	li	r4, 6			/* Command register value */
	sthbrx	r4, 0, r24

	LOADPTR (r3, PCI_STATUS)	/* PCI status reg */
	stwbrx	r3,0,r25
	li	r4, -1			/* Write-to-clear all bits */
	li	r3, 2			/* PCI_STATUS is at +2 offset */
	sthbrx	r4, r3, r24

	/*-------PROC_INT1_ADR */
	
	LOADPTR (r3, PROC_INT1_ADR)	/* Processor I/F Config 1 reg. */
	stwbrx	r3,0,r25
	LOADPTR (r4, 0xff141b98)
	stwbrx	r4,0,r24

	/*-------PROC_INT2_ADR */

	LOADPTR (r3, PROC_INT2_ADR)	/* Processor I/F Config 2 reg. */
	stwbrx	r3,0,r25
        lis	r4, 0x2000		/* Flush PCI config writes */
	stwbrx	r4,0,r24

	LOADPTR (r27, KAHLUA2_ID)	
        cmpl	0, 0, r26, r27
        bne     L1not8245

        /* MIOCR1 -- turn on bit for DLL delay */

	LOADPTR (r3, MIOCR1_ADR_X)	      
	stwbrx	r3,0,r25
        li      r4, 0x04
	stb	r4, MIOCR1_SHIFT(r24)

        /* For the MPC8245, set register 77 to %00100000 (see Errata #15) */
        /* SDRAM_CLK_DEL (0x77)*/

	LOADPTR (r3, MIOCR2_ADR_X)	      
	stwbrx	r3,0,r25
        li      r4, 0x10
	stb	r4, MIOCR2_SHIFT(r24)

        /* PMCR2 -- set PCI hold delay to <10>b for 33 MHz */

	LOADPTR (r3, PMCR2_ADR_X)	      
	stwbrx	r3,0,r25
        li      r4, 0x20
	stb	r4, PMCR2_SHIFT(r24)

        /*
        * Initialize RCS2 early since it's got the PLD with the board ID
        * registers and possibly contains the alphanumeric LEDs.
        */

        /* XROM enable, timing matches RCS0 */

        LOADPTR (r3, XROM_CFG1_ADR)
        stwbrx  r3,0,r25
        LOADPTR (r4, 0x84000000)
        stwbrx  r4,0,r24

        /* XROM at default addr, region size 64Kbyte */

        LOADPTR (r3, XROM_CFG3_ADR)
        stwbrx  r3,0,r25
        LOADPTR (r4, ((XROM_BASE_ADDR & 0x0ffff000) | 4))
        stwbrx  r4,0,r24

        /* Enable EXTROM */
	LOADPTR (r3, MEM_CONT4_ADR)
        stwbrx  r3,0,r25
	lwbrx	r4,0,r24
        oris    r4, r4, 0x0020          /* Set EXTROM */
	stwbrx	r4,0,r24
        
	/* Initialize EUMBBAR early since 8245 has internal UART in EUMB */

	LOADPTR (r3, EUMBBAR_REG)
	stwbrx	r3,0,r25
	LOADPTR (r4, EUMBBAR_VAL)
	stwbrx	r4,0,r24

        /* Load the board id into bits 24-31 */
	LOADPTR (r3, PLD_REG_BASE)
	lbz	r29, 0(r3)

        /* Load the CPU PCI Device ID into bits 0-15 */
        andis.  r3, r26, 0xffff
        or      r29, r29, r3

        /* Initialize the UART */
        bl      SerialInit

        /* If LOCAL_MEM_AUTOSIZE is defined, then include the SPD
           mechanism to detect and size attached memory. If not, just
           use whatever default memory size was configured in config.h
        */
#if defined(LOCAL_MEM_AUTOSIZE)        

#include "sdram_spd.s"

	cmpi	0, 0, r3, 0
        bgt     set_mem_config

#endif /* defined(LOCAL_MEM_AUTOSIZE) */
set_mem_default:        
        LOADPTR (r3, (MEM_CONF_DEFAULT|SPD_MEM_CONF_ERR))
set_mem_config:
        or      r29, r29, r3

L1not8245:

        /* Toggle the DLL reset bit in AMBOR */

	LOADPTR (r3, MAP_OPTS_ADR)	
	stwbrx	r3,0,r25
	lbz	r4, 0(r24)

        andi.   r4, r4, 0xdf
	stb	r4, 0(r24)		/* Clear DLL_RESET */
        sync

        ori     r4, r4, 0x20		/* Set DLL_RESET */
	stb	r4, 0(r24)		
        sync

        andi.   r4, r4, 0xdf
	stb	r4, 0(r24)		/* Clear DLL_RESET */


	/*-------MCCR1 */

#ifdef INCLUDE_ECC
#define MC_ECC				1
#else /* INCLUDE_ECC */
#define MC_ECC				0
#endif /* INCLUDE_ECC */

#define MC1_ROMNAL			8		/* 0-15 */
#define MC1_ROMFAL			11		/* 0-31 */
#define MC1_DBUS_SIZE			0		/* 0-3, read only */
#define MC1_BURST			0		/* 0-1 */
#define MC1_MEMGO			0		/* 0-1 */
#define MC1_SREN			1		/* 0-1 */
#define MC1_RAM_TYPE			0		/* 0-1 */
#define MC1_PCKEN			MC_ECC		/* 0-1 */

/* Define these the same for one or two banks of memory */
#define MC_MSAR1 (SDRAM_BANK_SIZE << 8)
#define MC_MEAR1 (((2*SDRAM_BANK_SIZE) - 1) << 8 | (SDRAM_BANK_SIZE - 1))

	LOADPTR (r3, MEM_CONT1_ADR)	/* Set MCCR1 (F0) */
	stwbrx	r3,0,r25
	LOADPTR(r4, \
		MC1_ROMNAL << 28 | MC1_ROMFAL << 23 | \
		MC1_DBUS_SIZE << 21 | MC1_BURST << 20 | \
		MC1_MEMGO << 19 | MC1_SREN << 18 | \
		MC1_RAM_TYPE << 17 | MC1_PCKEN << 16)
        /* Check for 13 row device detected/configured */
        andi.   r3, r29, SPD_MEM_CONF_ROW
        beq     set_mem_row_done
	ori	r4, r4, MC1_ROW13_BANKBITS
set_mem_row_done:
	stwbrx	r4, 0, r24

	/*------- MCCR2 */

#define MC2_TS_WAIT_TIMER		0		/* 0-7 */
#define MC2_ASRISE			8		/* 0-15 */
#define MC2_ASFALL			4		/* 0-15 */
#define MC2_INLINE_PAR_NOT_ECC		0		/* 0-1 */
#define MC2_WRITE_PARITY_CHK_EN		MC_ECC		/* 0-1 */
#define MC2_INLRD_PARECC_CHK_EN		MC_ECC		/* 0-1 */
#define MC2_ECC_EN			0		/* 0-1 */
#define MC2_EDO				0		/* 0-1 */
/* 
*  N.B. This refresh interval looks good up to 85 MHz with Hynix SDRAM.
*  May need to be decreased for 100 MHz
*/
#define MC2_REFINT			0x3a5		/* 0-0x3fff */
#define MC2_RSV_PG			0		/* 0-1 */
#define MC2_RMW_PAR			MC_ECC		/* 0-1 */

	LOADPTR (r3, MEM_CONT2_ADR)	/* Set MCCR2 (F4) */
	stwbrx	r3,0,r25
        andi.   r4, r29, SPD_MEM_CONF_ROW
        beq     set_mem_refresh_12
	LOADPTR(r4, \
		MC2_TS_WAIT_TIMER << 29 | MC2_ASRISE << 25 | \
		MC2_ASFALL << 21 | MC2_INLINE_PAR_NOT_ECC << 20 | \
		MC2_WRITE_PARITY_CHK_EN << 19 | \
		MC2_INLRD_PARECC_CHK_EN << 18 | \
		MC2_ECC_EN << 17 | MC2_EDO << 16 | \
		MC2_REFINT_ROW13 << 2 | MC2_RSV_PG << 1 | MC2_RMW_PAR)
        b       set_mem_refresh_done
set_mem_refresh_12:     
	LOADPTR(r4, \
		MC2_TS_WAIT_TIMER << 29 | MC2_ASRISE << 25 | \
		MC2_ASFALL << 21 | MC2_INLINE_PAR_NOT_ECC << 20 | \
		MC2_WRITE_PARITY_CHK_EN << 19 | \
		MC2_INLRD_PARECC_CHK_EN << 18 | \
		MC2_ECC_EN << 17 | MC2_EDO << 16 | \
		MC2_REFINT_ROW12 << 2 | MC2_RSV_PG << 1 | MC2_RMW_PAR)
set_mem_refresh_done:     
        cmpl	0, 0, r26, r27		/* Check for Kahlua2 */
        bne     notK2
        /* clear Kahlua2 reserved bits */
	LOADPTR (r3, 0xfffcffff)
	and	r4, r4, r3
notK2:
	stwbrx	r4,0,r24

	/*------- MCCR3 */

#define MC_BSTOPRE			0x079		/* 0-0x7ff */

#define MC3_BSTOPRE_U			(MC_BSTOPRE >> 4 & 0xf)
#define MC3_REFREC			8		/* 0-15 */
#define MC3_RDLAT			(4+MC_ECC)	/* 0-15 */
#define MC3_CPX				0		/* 0-1 */
#define MC3_RAS6P			0		/* 0-15 */
#define MC3_CAS5			0		/* 0-7 */
#define MC3_CP4				0		/* 0-7 */
#define MC3_CAS3			0		/* 0-7 */
#define MC3_RCD2			0		/* 0-7 */
#define MC3_RP1				0		/* 0-7 */

	LOADPTR (r3, MEM_CONT3_ADR)	/* Set MCCR3 (F8) */
	stwbrx	r3,0,r25
	LOADPTR(r4, \
		MC3_BSTOPRE_U << 28 | MC3_REFREC << 24 | \
		MC3_RDLAT << 20 | MC3_CPX << 19 | \
		MC3_RAS6P << 15 | MC3_CAS5 << 12 | MC3_CP4 << 9 | \
		MC3_CAS3 << 6 | MC3_RCD2 << 3 | MC3_RP1)
        cmpl	0, 0, r26, r27              /* Check for Kahlua2 */
        bne     notK2b
        /* clear Kahlua2 reserved bits */
	LOADPTR (r3, 0xff000000)
	and	r4, r4, r3
notK2b:
	stwbrx	r4,0,r24

	/*------- MCCR4 */

#define MC4_PRETOACT			3		/* 0-15 */
#define MC4_ACTOPRE			5		/* 0-15 */
#define MC4_WMODE			0		/* 0-1 */
#define MC4_INLINE			MC_ECC		/* 0-1 */
#define MC4_REGISTERED			(1-MC_ECC)	/* 0-1 */
#define MC4_BSTOPRE_UU			(MC_BSTOPRE >> 8 & 3)
#define MC4_REGDIMM			0		/* 0-1 */
#define MC4_SDMODE_CAS			2		/* 0-7 */
#define MC4_DBUS_RCS1			1		/* 0-1, 8-bit */
#define MC4_SDMODE_WRAP			0		/* 0-1 */
#define MC4_SDMODE_BURST		2		/* 0-7 */
#define MC4_ACTORW			3		/* 0-15 */
#define MC4_BSTOPRE_L			(MC_BSTOPRE & 0xf)

	LOADPTR (r3, MEM_CONT4_ADR)	/* Set MCCR4 (FC) */
	stwbrx	r3,0,r25
	LOADPTR(r4, \
		MC4_PRETOACT << 28 | MC4_ACTOPRE << 24 | \
		MC4_WMODE << 23 | MC4_INLINE << 22 | \
		MC4_REGISTERED << 20 | MC4_BSTOPRE_UU << 18 | \
		MC4_DBUS_RCS1 << 17 | \
		MC4_REGDIMM << 15 | MC4_SDMODE_CAS << 12 | \
		MC4_SDMODE_WRAP << 11 | MC4_SDMODE_BURST << 8 | \
		MC4_ACTORW << 4 | MC4_BSTOPRE_L)
        cmpl	0, 0, r26, r27                /* Check for Kahlua 2 */
        bne     notK2c
        /* Turn on Kahlua2 extended ROM space */
	LOADPTR (r3, 0x00200000)
	or	r4, r4, r3
notK2c:
	stwbrx	r4,0,r24

#ifdef INCLUDE_ECC
	/*------- MEM_ERREN1 */

	LOADPTR (r3, MEM_ERREN1_ADR)	/* Set MEM_ERREN1 (c0) */
	stwbrx	r3,0,r25
	lwbrx	r4,0,r24
	ori	r4,r4,4			/* Set MEM_PERR_EN */
	stwbrx	r4,0,r24
#endif /* INCLUDE_ECC */

	/*------- MSAR/MEAR */
        andi.   r26, r29, SPD_MEM_CONF_SZ
        
	cmpli	0, 0, r26, SPD_MEM_CONF_256
        bne     mem_size_128

	LOADPTR (r3, MEM_START1_ADR)	/* Set MSAR1 (80) */
	stwbrx	r3,0,r25
	LOADPTR (r4, MC_MSAR1_256)
	stwbrx	r4,0,r24
        
	LOADPTR (r3, XMEM_START1_ADR)	/* Set XSAR1 (88) */
	stwbrx	r3,0,r25
	LOADPTR (r4, MC_XSAR1_256)
	stwbrx	r4,0,r24

	LOADPTR (r3, MEM_END1_ADR)	/* Set MEAR1 (90) */
	stwbrx	r3,0,r25
	LOADPTR (r4, MC_MEAR1_256)
	stwbrx	r4,0,r24

	LOADPTR (r3, XMEM_END1_ADR)	/* Set XEAR1 (94) */
	stwbrx	r3,0,r25
	LOADPTR (r4, MC_XEAR1_256)
	stwbrx	r4,0,r24

        b       mem_size_done
        
mem_size_128:   
       
	cmpli	0, 0, r26, SPD_MEM_CONF_128
        bne     mem_size_64

	LOADPTR (r3, MEM_START1_ADR)	/* Set MSAR1 (80) */
	stwbrx	r3,0,r25
	LOADPTR (r4, MC_MSAR1_128)
	stwbrx	r4,0,r24
        
	LOADPTR (r3, XMEM_START1_ADR)	/* Set MESAR1 (88) */
	stwbrx	r3,0,r25
	LOADPTR (r4, MC_XSAR1_128)
	stwbrx	r4,0,r24

	LOADPTR (r3, MEM_END1_ADR)	/* Set MEAR1 (90) */
	stwbrx	r3,0,r25
	LOADPTR (r4, MC_MEAR1_128)
	stwbrx	r4,0,r24

	LOADPTR (r3, XMEM_END1_ADR)	/* Set XEAR1 (98) */
	stwbrx	r3,0,r25
	LOADPTR (r4, MC_XEAR1_128)
	stwbrx	r4,0,r24

        b       mem_size_done

mem_size_64:    
        
	LOADPTR (r3, MEM_START1_ADR)	/* Set MSAR1 (80) */
	stwbrx	r3,0,r25
	LOADPTR (r4, MC_MSAR1_64)
	stwbrx	r4,0,r24
        
	LOADPTR (r3, XMEM_START1_ADR)	/* Set MESAR1 (88) */
	stwbrx	r3,0,r25
	LOADPTR (r4, MC_XSAR1_64)
	stwbrx	r4,0,r24

	LOADPTR (r3, MEM_END1_ADR)	/* Set MEAR1 (90) */
	stwbrx	r3,0,r25
	LOADPTR (r4, MC_MEAR1_64)
	stwbrx	r4,0,r24

	LOADPTR (r3, XMEM_END1_ADR)	/* Set XEAR1 (98) */
	stwbrx	r3,0,r25
	LOADPTR (r4, MC_XEAR1_64)
	stwbrx	r4,0,r24

mem_size_done:
 
        /* Physical banks 4-7 are not used */
	LOADPTR (r3, MEM_START2_ADR)	/* Set MSAR2 (84) */
	stwbrx	r3,0,r25
	LOADPTR (r4, 0xc0804000)
	stwbrx	r4,0,r24

	LOADPTR (r3, XMEM_START2_ADR)	/* Set XEAR2 (8c) */
	stwbrx	r3,0,r25
	LOADPTR (r4, 0x01010101)
	stwbrx	r4,0,r24

	LOADPTR (r3, MEM_END2_ADR)	/* MEEAR2 (94) */
	stwbrx	r3,0,r25
	LOADPTR (r4, 0x00000000)
	stwbrx	r4,0,r24

	LOADPTR (r3, XMEM_END2_ADR)	/* XEEAR2 (9c) */
	stwbrx	r3,0,r25
	LOADPTR (r4, 0x01010101)
	stwbrx	r4,0,r24

	/*-------ODCR */

	LOADPTR (r3, ODCR_ADR_X)	/* Set ODCR */
	stwbrx	r3,0,r25

	li	r4, 0x7f
	stb	r4, ODCR_SHIFT(r24)	/* ODCR is at +3 offset */

	/*-------MBEN */

	LOADPTR (r3, MEM_EN_ADR)	/* Set MBEN (a0) */
	stwbrx	r3,0,r25

        /* check number of physical banks */
        li      r4, 0x01                /* Enable one by default */
        andi.   r3, r29, SPD_MEM_CONF_BANK
        beq     set_mem_bank
	ori	r4, r4, 0x02		/* Enable two banks */
set_mem_bank:
	stb	r4, 0(r24)		/* MBEN is at +0 offset */

#if 0   /* Jimmy:  I think page made is broken */
	/*-------PGMAX */

	LOADPTR (r3, MPM_ADR_X)
	stwbrx	r3,0,r25
	li	r4, 0x32
	stb	r4, MPM_SHIFT(r24)		/* PAGE_MODE is at +3 offset */
#endif

	/* Wait before initializing other registers */

	lis	r4,0x0001
	mtctr	r4

KahluaX4wait200us:
	bdnz	KahluaX4wait200us

	/* Set MEMGO bit */

	LOADPTR (r3, MEM_CONT1_ADR)	/* MCCR1 (F0) |= PGMAX */
	stwbrx	r3,0,r25
	lwbrx	r4,0,r24			/* old MCCR1 */
	oris	r4,r4,0x0008		/* MEMGO=1 */
	stwbrx	r4, 0, r24

	/* Wait again */

	addis	r4,r0,0x0002
	ori	r4,r4,0xffff

	mtctr	r4

KahluaX4wait8ref:
	bdnz	KahluaX4wait8ref

	sync
	eieio

#if	FALSE	/* EABI SDA not supported yet */

	/* initialize r2 and r13 according to EABI standard */

	LOADPTR (r2, _SDA2_BASE_)
	LOADPTR (r13, _SDA_BASE_)
#endif

	/* Display a message indicating PROM has started */

	PUTC(13)
	PUTC(10)
	PUTC(10)
	PUTC('B')
	PUTC('o')
	PUTC('o')
	PUTC('t')
	PUTC('i')
	PUTC('n')
	PUTC('g')
	PUTC('.')
	PUTC('.')
	PUTC('.')
	PUTC(13)
	PUTC(10)
	PUTC(10)
        /* Print out board info */
        
 	PUTC('H')
 	PUTC('W')
        mr      r3, r29
        bl      hexprint
        bl      crlf

	/* go to C entry point */

	or	r3, r30, r30		/* put startType in r3 (p0) */
	addi	sp, sp, -FRAMEBASESZ	/* save one frame stack */

	LOADPTR (r6, romStart)
	LOADPTR (r7, romInit)
	LOADPTR (r8, ROM_TEXT_ADRS)

	sub	r6, r6, r7
	add	r6, r6, r8

	mtlr	r6		/* romStart - romInit + ROM_TEXT_ADRS */
	blr

/**********************************************************************
 *  void SEROUT(int char);
 *
 *  Configure Mousse COM1 to 9600 baud and write a character.
 *  Should be usable basically at any time.
 *
 *  r3 = character to output
 *  modifies r0,r4,r5,r6 (w/o init)
 */

SEROUT:
	/* r4 = intLock() */

	mfmsr	r4
	rlwinm	r5,r4,0,17,15
	mtmsr	r5
	isync

	/* r5 = serial port address, channel 0 */

	LOADPTR(r5, EUMBBAR_VAL | 0x4500)
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

/* Initialize serial port. 

   EUMBBAR must be initialized.

Input:
   None
Output:
   None
Modifies:
   r3,r4,r8
     
 */
SerialInit:
	LOADPTR (r3, CHRP_REG_ADDR)	
	LOADPTR (r4, PLL_CFG_ADR_X)	
	stwbrx	r4,0,r3
	LOADPTR (r3, CHRP_REG_DATA)
	lbz	r4, PLL_CFG_ADR_SHIFT(r3)

        /* 
        * r4 contains PLL_VALUE || <000>b 
        * Assume PLL is such that SDRAM clock is 66 MHZ,
        * unless it's set to 85 MHz or 100 MHz (assuming 33 MHz PCI clock)
        */
#define PLL_66MHZ 0xb0
#define PLL_85MHZ 0xc0
#define PLL_100MHZ 0x80
#define PLL_75MHZ_PCI50 0xe0
#define PLL_100MHZ_PCI50 0xc8

        cmpli   0, 0, r4, PLL_85MHZ
	beq uart85
        cmpli   0, 0, r4, PLL_100MHZ
	beq uart100
        cmpli   0, 0, r4, PLL_75MHZ_PCI50
	beq uart75
        cmpli   0, 0, r4, PLL_100MHZ_PCI50
	beq uart100
        cmpli   0, 0, r4, PLL_66MHZ
	beq uart66
uart66:
	LOADPTR (r4, 434)
	b uartgo
uart85:
	LOADPTR (r4, 543)
	b uartgo
uart100:
	LOADPTR (r4, 651)
        b uartgo
uart75:
	LOADPTR (r4, 488)
        b uartgo
uartgo:

	/* r8 = serial port address, channel 0 */

	LOADPTR(r8, EUMBBAR_VAL | 0x4500)

	/* Init serial port */
	li	r3, 0x83		/* Enable divisor latch */
	stb	r3, 3(r8)
	sync
	clrlwi  r3, r4, 24
	stb	r3, 0(r8)		/* baud_lo */
	rlwinm  r3, r4, 24, 24, 31
	stb	r3, 1(r8)		/* baud_hi */
	li	r3, 0x03
	stb	r3, 3(r8)		/* 8-n-1 */
	li	r3, 0x01
	stb	r3, 0x11(r8)		/* 2-pin mode */

	blr

        
        /* hexprint
        
           r3 = number to print
           destroys r3,r4,r5,r6,r7
           save/resores rl in r23
           calls SEROUT
           can only be called from main rtn, not sub
        */
hexprint:       
        mr      r7, r3
        mfspr   r23, LR
        rlwinm	r4,r7,4,28,31
        addic	r5,r4,48
        cmpwi	cr1,r5,58
        addic	r4,r4,55
        mfcr	r3
        rlwinm	r3,r3,5,31,31
        neg	r3,r3
        and	r5,r5,r3
        andc	r3,r4,r3
        or	r3,r5,r3
        bl	SEROUT
        rlwinm	r4,r7,8,28,31
        addic	r5,r4,48
        cmpwi	cr1,r5,58
        addic	r4,r4,55
        mfcr	r3
        rlwinm	r3,r3,5,31,31
        neg	r3,r3
        and	r5,r5,r3
        andc	r3,r4,r3
        or	r3,r5,r3
        bl	SEROUT
        rlwinm	r4,r7,12,28,31
        addic	r5,r4,48
        cmpwi	cr1,r5,58
        addic	r4,r4,55
        mfcr	r3
        rlwinm	r3,r3,5,31,31
        neg	r3,r3
        and	r5,r5,r3
        andc	r3,r4,r3
        or	r3,r5,r3
        bl	SEROUT
        rlwinm	r4,r7,16,28,31
        addic	r5,r4,48
        cmpwi	cr1,r5,58
        addic	r4,r4,55
        mfcr	r3
        rlwinm	r3,r3,5,31,31
        neg	r3,r3
        and	r5,r5,r3
        andc	r3,r4,r3
        or	r3,r5,r3
        bl	SEROUT
        rlwinm	r4,r7,20,28,31
        addic	r5,r4,48
        cmpwi	cr1,r5,58
        addic	r4,r4,55
        mfcr	r3
        rlwinm	r3,r3,5,31,31
        neg	r3,r3
        and	r5,r5,r3
        andc	r3,r4,r3
        or	r3,r5,r3
        bl	SEROUT
        rlwinm	r4,r7,24,28,31
        addic	r5,r4,48
        cmpwi	cr1,r5,58
        addic	r4,r4,55
        mfcr	r3
        rlwinm	r3,r3,5,31,31
        neg	r3,r3
        and	r5,r5,r3
        andc	r3,r4,r3
        or	r3,r5,r3
        bl	SEROUT
        rlwinm	r4,r7,28,28,31
        addic	r5,r4,48
        cmpwi	cr1,r5,58
        addic	r4,r4,55
        mfcr	r3
        rlwinm	r3,r3,5,31,31
        neg	r3,r3
        and	r5,r5,r3
        andc	r3,r4,r3
        or	r3,r5,r3
        bl	SEROUT
        clrlwi	r7,r7,28
        addi	r5,r7,48
        cmpwi	cr1,r5,58
        addi	r7,r7,55
        mfcr	r3
        rlwinm	r3,r3,5,31,31
        neg	r3,r3
        and	r4,r5,r3
        andc	r3,r7,r3
        or	r3,r4,r3
        bl	SEROUT
        mtspr   LR, r23
        blr
        
        /* Output a CR LF sequence 
        
           calls SEROUT
        */
crlf:   
        mfspr   r23, LR
        PUTC(13)
	PUTC(10)
        mtspr   LR, r23
        blr
    
/* eumbbar_read
        
  Read a value from an EUMBBAR register

  Input:  r3  register to read
  Output: r3  value
  Modifies: r4

*/
        
eumbbar_read:
        lis     r4, HI(EUMBBAR_VAL)
        sync
        lwbrx   r3, r3, r4
        blr
        
/* eumbbar_write
        
  Write a value to an EUMBBAR register

  Input:  r3  register to write
          r4  value to write to register
  Output: <none>
  Modifies: r5

*/

eumbbar_write:    
        lis     r5, HI(EUMBBAR_VAL)
        stwbrx  r4, r3, r5
        sync
        blr

    
/* led_write
        
   Write a msg to the LED display

  0x31323334 should show '1234' on the display
        
  Input:  r3  message
  Output: <none>
  Modifies: r4, r5

*/

led_write:
        lis     r5, HI(LED_REG_BASE)
        extrwi  r4, r3, 8, 0
        stb     r4, 0x2003(r5)
        extrwi  r4, r3, 8, 8
        stb     r4, 0x2002(r5)
        extrwi  r4, r3, 8, 16
        stb     r4, 0x2001(r5)
        stb     r3, 0x2000(r5)
        blr

        
/* long_delay
        
  Delay for about 1 second

  Input:  <none>
  Output: <none>
  Modifies: r3

*/   
       
long_delay:     
	LOADPTR (r3, 100000000)
	mtspr	CTR, r3
ld1:
	nop
	bdnz	ld1
        blr
        
/* short_delay
        
  Delay for about .25 second

  Input:  <none>
  Output: <none>
  Modifies: r3

*/   
       
short_delay:     
	LOADPTR (r3, 100000000/4)
	mtspr	CTR, r3
sd1:
	nop
	bdnz	sd1
        blr
        
#include "sysL2Backcache.s"

    .globl __pci_config_read_32
__pci_config_read_32:
    lis     r4, 0xfec0
    stwbrx   r3, r0, r4
    sync
    lis     r4, 0xfee0
    lwbrx   r3, 0, r4
    blr
    .globl __pci_config_read_16
__pci_config_read_16:
    lis     r4, 0xfec0
    andi.    r5, r3, 2
    stwbrx  r3, r0, r4
    sync
    oris     r4, r5, 0xfee0
    lhbrx    r3, r0, r4
    blr
    .globl __pci_config_read_8
__pci_config_read_8:
    lis     r4, 0xfec0
    andi.    r5, r3, 3
    stwbrx  r3, r0, r4
    sync
    oris     r4, r5, 0xfee0
    lbz      r3, 0(4)
    blr
    .globl __pci_config_write_32
__pci_config_write_32:
    lis     r5, 0xfec0
    stwbrx   r3, r0, r5
    sync
    lis      r5, 0xfee0
    stwbrx   r4, r0, r5
    sync
    blr
    .globl __pci_config_write_16
__pci_config_write_16:
    lis     r5, 0xfec0
    andi.    r6, r3, 2
    stwbrx  r3, r0, 5
    sync
    oris     r5, r6, 0xfee0
    sthbrx    r4, r0, r5
    sync
    blr
    .globl __pci_config_write_8
__pci_config_write_8:
    lis      r5, 0xfec0
    andi.    r6, r3, 3
    stwbrx   r3, r0, r5
    sync
    oris      r5, r6, 0xfee0
    stb       r4, 0(r5)
    sync
    blr
    .globl  in_8
in_8:
    oris    r3, r3, 0xfe00
    lbz     r3,0(r3)
    blr
    .globl  in_16
in_16:
    oris    r3, r3, 0xfe00
    lhbrx   r3, 0, r3
    blr
    .globl in_16_ne
in_16_ne:
    oris    r3, r3, 0xfe00
    lhzx    r3, 0, r3
    blr
    .globl  in_32
in_32:
    oris    r3, r3, 0xfe00
    lwbrx   r3, 0, r3
    blr
    .globl  out_8
out_8:
    oris    r3, r3, 0xfe00
    stb     r4, 0(r3)
    eieio
    blr
    .globl  out_16
out_16:
    oris    r3, r3, 0xfe00
    sthbrx  r4, 0, r3
    eieio
    blr
    .globl  out_16_ne
out_16_ne:
    oris    r3, r3, 0xfe00
    sth     r4, 0(r3)
    eieio
    blr
    .globl  out_32
out_32:
    oris    r3, r3, 0xfe00
    stwbrx  r4, 0, r3
    eieio
    blr
    .globl  read_8
read_8:
    lbz     r3,0(r3)
    blr
    .globl  read_16
read_16:
    lhbrx   r3, 0, r3
    blr
    .globl  read_32
read_32:
    lwbrx   r3, 0, r3
    blr
    .globl  read_32_ne
read_32_ne:
    lwz     r3, 0(r3)
    blr
    .globl  write_8
write_8:
    stb     r4, 0(r3)
    eieio
    blr
    .globl  write_16
write_16:
    sthbrx  r4, 0, r3
    eieio
    blr
    .globl  write_32
write_32:
    stwbrx  r4, 0, r3
    eieio
    blr
    .globl  write_32_ne
write_32_ne:
    stw     r4, 0(r3)
    eieio
    blr
  
