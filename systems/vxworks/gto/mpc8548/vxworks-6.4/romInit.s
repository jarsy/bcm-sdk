/* ROmInit.s - ROM bootcode for SBC8548 BSP */

/* $Id: romInit.s,v 1.9 2011/07/21 16:14:13 yshtil Exp $
 * Copyright (c) 2006-2007 Wind River Systems, Inc.
 *
 * The right to copy, distribute, modify or otherwise make use
 * of this software may be licensed only pursuant to the terms
 * of an applicable Wind River license agreement.
 */

/*
modification history
--------------------
01c,25may07,b_m  add BOOT_FLASH macro to support boot device selection.
01b,03apr07,b_m  MPC8548 rev.2 update.
01a,30jan06,kds  Modify from cds8548 01a.
*/
	.data
#define	_ASMLANGUAGE
#include <vxWorks.h>
#include <sysLib.h>
#include <asm.h>
#include <config.h>
#include <regs.h>
#include <arch/ppc/mmuE500Lib.h>

FUNC_EXPORT(romInit)
FUNC_EXPORT(_romInit)

FUNC_IMPORT(romStart)

#define WRITEADR(reg1,reg2,addr32,val) \
	lis	reg1, HI(addr32); \
	ori	reg1, reg1, LO(addr32); \
	lis	reg2, HI(val); \
	ori	reg2, reg2, LO(val); \
	stw	reg2, 0(reg1)

#define WRITEOFFSET(regbase,reg2,offset,val) \
        lis	reg2, HI(val); \
        ori	reg2, reg2, LO(val); \
	stw	reg2, offset(regbase);

	_WRS_TEXT_SEG_START

	.fill 0x100,1,0

FUNC_BEGIN(romInit)
FUNC_BEGIN(_romInit)
	bl     resetEntry

romWarm:
	bl     warm

        .ascii   "Copyright 1984-2006 Wind River Systems, Inc."
        .balign 4

cold:
	li    r2, BOOT_COLD
	b     start

	/* defines for memory initialization */
warm:
	mr     r2,r3

start:
        /* turn off exceptions */

        mfmsr   r3                      /* r3 = msr              */
        INT_MASK (r3, r4)               /* mask EE and CE bit    */
        rlwinm  r4, r4, 0, 20, 18       /* turn off _PPC_MSR_ME  */
        mtmsr   r4                      /* msr = r4              */
        isync

        xor     r0, r0, r0
        addi    r1, r0, -1

        mtspr   DEC, r0
        mtspr   TBL, r0
        mtspr   TBU, r0
        mtspr   TSR, r1
        mtspr   TCR, r0
        mtspr   ESR, r0                 /* clear Exception Syndrome Reg */
        mtspr   XER, r0                 /*  clear Fixed-Point Exception Reg */

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
	mtspr L1CSR1, r6	 /* Disable the Instruction cache */
        li   r6, 0x0002
        msync
	isync
	mtspr L1CSR1, r6	/* Invalidate the Instruction cache */
        isync
        li   r6, 0x0000
	msync
	isync
	mtspr L1CSR1, r6        /* temp disable the Instruction cache */

        isync
        li	r7, 0x0001
	msync
	isync
	mtspr	L1CSR1, r7		/* enable the instruction cache */

	msync
	isync

        /* Set the pciAutoconfig check to FALSE */

	xor     r5,r5,r5                /* Zero r5 */
	lis     r6,HIADJ(PCI_AUTO_CONFIG_ADRS)
	addi	r6,r6,LO(PCI_AUTO_CONFIG_ADRS)
	stw     r5,0(r6)

	xor     r6,r6,r6
	xor     r7,r7,r7
	mullw   r7,r7,r6
        lis     sp, HI(STACK_ADRS)
        ori     sp, sp, LO(STACK_ADRS)
        addi    sp, sp, -FRAMEBASESZ
        lis     r6, HI(romInit)
        ori     r6, r6, LO(romInit)
        lis     r7, HI(romStart)
        ori     r7, r7, LO(romStart)
        lis     r8, HI(ROM_TEXT_ADRS)
        ori     r8, r8, LO(ROM_TEXT_ADRS)
	sub	r6, r7, r6		/* routine - entry point */
	add	r6, r6, r8 		/* + ROM base */
        mtspr   LR, r6

        mr      r3, r2
        blr
FUNC_END(_romInit)
FUNC_END(romInit)


/***************************************************************************
*
* resetEntry - rom entry point
*
*
*/

#if   defined(_GNU_TOOL)
	.section .boot, "ax", @progbits
#elif defined(_DIAB_TOOL)
        .section .boot, 4, "rx"
#else
#error "Please add a correctly spelled .section directive for your toolchain."
#endif

FUNC_BEGIN(resetEntry)
FUNC_LABEL(_resetEntry)

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
	mtspr L1CSR1, r6	 /* Disable the Instruction cache */
        li   r6, 0x0002
        msync
	isync
	mtspr L1CSR1, r6	/* Invalidate the Instruction cache */
        isync
        li   r6, 0x0000
	msync
	isync
	mtspr L1CSR1, r6        /* temp disable the Instruction cache */


        isync
        li	r7, 0x0000      /* FIXME: Should be "li r7, 0x0001" to */
        msync                   /*        enable instruction cache.    */

	isync
	mtspr	L1CSR1, r7		/* enable the instruction cache */


	msync
	isync


        /* Clear SRR0, CSRR0, MCSRR0, SRR1, CSRR1 , MCSRR1, IVPR */

	xor   r0,r0,r0

        mtspr SRR0, r0          /* Save/restore register 0 */
        mtspr SRR1, r0          /* Save/restore register 1 */
        mtspr CSRR0, r0         /* Critical save/restore register 0 */
        mtspr CSRR1, r0         /* Critical save/restore register 1 */
        mtspr MCSRR0, r0        /* Machine check save/restore register 0 */
        mtspr MCSRR1, r0        /* Machine check save/restore register 1 */
        mtspr ESR, r0           /* Exception syndrone register */
        mtspr MCSR, r0          /* Machine check syndrone register */
        mtspr DEAR, r0          /* Data exception address register */
        mtspr DBCR0, r0         /* Debug control register 0 */
        mtspr DBCR1, r0         /* Debug control register 1 */
        mtspr DBCR2, r0         /* Debug control register 2 */
        mtspr IAC1, r0          /* Instruction address compare 1 */
        mtspr IAC2, r0          /* Instruction address compare 2 */
        mtspr DAC1, r0          /* Data address compare 1 */
        mtspr DAC2, r0          /* Data address compare 1 */

        mfspr r1, DBSR          /* Debug status register */
        mtspr DBSR, r1          /* DBSR bits corresponding to 1 bits in the
                                 * GPR is cleared using mtspr
                                 */

        mtspr PID0, r0          /* Process ID 0 register */
        mtspr PID1, r0          /* Process ID 1 register */
        mtspr PID2, r0          /* Process ID 2 register */
        mtspr TCR, r0           /* Timer control register */
        mtspr 1013, r0          /* Branch unit control and status register */

	mtspr MAS4, r0          /* MMU assist register 4 */
	mtspr MAS6, r0          /* MMU assist register 6 */

	isync
	lis   r1,0xfff0         /* IPVR[32-47] provides high-order 16 bits */
	ori   r1,r1,0xfff0      /* of the exception processing routines.   */
                                /* The 16-bit vector offsets are concatenated
                                 * to the right of IVPR[32-47] to form the
                                 * address of the exception processing routine.
                                 */
        mtspr IVPR, r1          /* Interrupt vector prefix register */

        /* Set up vector offsets */

        addi  r3, r0, 0x100
        mtspr IVOR0, r3         /* Critical input */
        addi  r3, r0, 0x200
        mtspr IVOR1, r3         /* Machine check */
        addi  r3, r0, 0x300
        mtspr IVOR2, r3         /* Data storage */
        addi  r3, r0, 0x400
        mtspr IVOR3, r3         /* Instruction storage */
        addi  r3, r0, 0x500
        mtspr IVOR4, r3         /* External input */
        addi  r3, r0, 0x600
        mtspr IVOR5, r3         /* Alignment */
        addi  r3, r0, 0x700
        mtspr IVOR6, r3         /* Program */

        /* skipping IVOR7 0x800: no FPU on e500 */

        addi  r3, r0, 0x900
        mtspr IVOR8, r3         /* System call */

        /* skipping IVOR9 0xa00: no aux processor on e500 */

        addi  r3, r0, 0xb00
        mtspr IVOR10, r3        /* Decrementer */
        addi  r3, r0, 0xc00
        mtspr IVOR11, r3        /* Fixed-interval timer interrupt */
        addi  r3, r0, 0xd00
        mtspr IVOR12, r3        /* Watchdog timer interrupt */
        addi  r3, r0, 0xe00
        mtspr IVOR13, r3        /* Data TLB error */
        addi  r3, r0, 0xf00
        mtspr IVOR14, r3        /* Instruction TLB error */
        addi  r3, r0, 0x1000
        mtspr IVOR15, r3        /* Debug */
        addi  r3, r0, 0x1100       /* SPU is e500 specific */
        mtspr IVOR32, r3        /* SPE APU unavailable */
        addi  r3, r0, 0x1200       /* FP data is e500 specific */
        mtspr IVOR33, r3        /* Embedded floating-point data exception */
        addi  r3, r0, 0x1300       /* FP round is e500 specific */
        mtspr IVOR34, r3        /* Embedded floating-point round exception */
        addi  r3, r0, 0x1400       /* perf mon is e500 specific */
        mtspr IVOR35, r3        /* Performance monitor */

	li    r2,0x1e           /* Invalidate TLB0 and TLB1 flash */
	mtspr MMUCSR0, r2
	isync

	li    r3,4
	li    r4,0
	tlbivax r4,r3           /* TLB invalidate */
	nop

	b postTable
platpllTable:
	.long 0x00000203
	.long 0x04050607
	.long 0x08090A00
	.long 0x0C000000
	.long 0x10000000
	.long 0x14000000
postTable:

        /*
         * Write TLB entry for initial program memory page
         *
         * - Specify EPN, RPN, and TSIZE as appropriate for system
         * - Set valid bit
         * - Specify TID=0
         * - Specify TS=0 or else MSR[IS,DS] must be set to correspond to TS=1
         * - Specify storage attributes (W, I, M, G, E, U0 - U3) as appropriate
         * - Enable supervisor mode fetch, read, and write access (SX, SR, SW)
         */

        /*
         * TLB1 #0.  ROM - non-cached 0xf0000000 -> 0xfffffffff.
	 * 256MB
         * Attributes: SX/SW/SR **PROTECTED**
         */
        /* MMU Assit Register 0 (MAS0)
         *           35          44  47        63
         * +-----+--------+-----+------+-----+----+
         * | --- | TLBSEL | --- | ESEL | --- | NV |
         * +-----+--------+-----+------+-----+----+
         * TLBSEL  Selects TLB for access (0 = TLB0, 1 = TLB1)
         * ESEL    Entry select. Number of entry in selected array to be used
         *         for tlbwe.
         * NV      Next victim.
         */
        addis  r4,0,0x1000           /* TLBSEL = TLB1(CAM) , ESEL = 0 */
        ori    r4,r4,0x0000
        mtspr  MAS0, r4

        /* MAS Register 1 (MAS1)
         *  32    33          40 47        51  52   55
         * +---+-------+-----+-----+-----+----+-------+-----+
         * | V | IPROT | --- | TID | --- | TS | TSIZE | --- |
         * +---+-------+-----+-----+-----+----+-------+-----+
         * V      TLB valid bit
         * IPROT  Invalidate protect.
         *        1 = Entry is protected from invalidation, 0 = Otherwise.
         * TID    Translation identity.
         * TS     Translation space
         * TSIZE  Translation size
         *        0001  4KB     0010  16KB      0011  64KB      0100  256KB
         *        0101  1MB     0110  4MB       0111  16MB      1000  64MB
         *        1001  256MB   1010  1GB       1011   4GB
         */
        addis  r5,0,0xc000           /* V = 1, IPROT = 1, TID = 0*/
        ori    r5,r5,_MMU_TLB_SZ_256M  /* TS = 0, TSIZE = 256 MByte page size*/
        mtspr  MAS1, r5

        /* MAS Register 2 (MAS2)
         *  32 51        57   58  59  60  61  62  63
         * +-----+-----+----+----+---+---+---+---+---+
         * | EPN | --- | X0 | X1 | W | I | M | G | E |
         * +-----+-----+----+----+---+---+---+---+---+
         * EPN  Effective page number.
         * X0   Implementation-dependent page attribute.
         * X1   Implementation-dependent page attribute.
         * W    Write-through (0 = write-back, 1 = write through)
         * I    Caching-inhibited
         * M    Memory coherency required
         * G    Guarded
         * E    Endianness. (0 = big-endian, 1 = little-endian)
         */
        addis  r6,0,0xf000           /* EPN = 0xf0000000*/
        ori    r6,r6,0x000a          /* WIMGE = 01010 */
        mtspr  MAS2, r6

        /* MAS Register 3 (MAS3)
         *  32 51       54   57  58   59   60   61   62   63
         * +-----+-----+-------+----+----+----+----+----+----+
         * | RPN | --- | U0-U3 | UX | SX | UW | SW | UR | SR |
         * +-----+-----+-------+----+----+----+----+----+----+
         * RPN   Real page number
         * U0-U3 User attribute bits.
         * UX/SX User/supervisor execute permission  bit
         * UW/SW User/supervisor write permission bit
         * UR/SR User/supervisor read permission bit
         */
        addis  r7,0,0xf000           /* RPN = 0xf0000000*/
        ori    r7,r7,0x0015          /* Supervisor XWR*/
        mtspr  MAS3, r7
        isync
        msync
        tlbwe
        tlbsync

        /*
         * TLB1 #1.  Main SDRAM - Not Cached
	 * LOCAL_MEM_LOCAL_ADRS -> LOCAL_MEM_LOCAL_ADRS + 256MB 
         * Attributes: UX/UW/UR/SX/SW/SR
         */

        addis  r4,0,0x1001           /* TLBSEL = TLB1(CAM) , ESEL = 1*/
        ori    r4,r4,0x0000
        mtspr  MAS0, r4
        addis  r5,0,0xc000           /* V = 1, IPROT = 1, TID = 0*/
        ori    r5,r5,_MMU_TLB_SZ_256M  /* TS = 0, TSIZE = 256 MByte page size*/
        mtspr  MAS1, r5
        addis  r6,0,HI(LOCAL_MEM_LOCAL_ADRS) /* EPN = LOCAL_MEM_LOCAL_ADRS */
        ori    r6,r6,0x000a          /* WIMGE = 01010*/
        mtspr  MAS2, r6
        addis  r7,0,HI(LOCAL_MEM_LOCAL_ADRS)  /* RPN = LOCAL_MEM_LOCAL_ADRS */
        ori    r7,r7,0x003f          /* User/Supervisor XWR*/
        mtspr  MAS3, r7
        isync
        msync
        tlbwe
        tlbsync

#if (LOCAL_MEM_SIZE > 0x10000000)
        /*
         * TLB1 #3.  Main SDRAM  - Not Cached
	 * LOCAL_MEM_LOCAL_ADRS + 256MB -> LOCAL_MEM_LOCAL_ADRS + LOCAL_MEM_SIZE
         * Attributes: UX/UW/UR/SX/SW/SR
         */

        addis  r4,0,0x1003           /* TLBSEL = TLB1(CAM) , ESEL = 3*/
        ori    r4,r4,0x0000
        mtspr  MAS0, r4
        addis  r5,0,0xc000           /* V = 1, IPROT = 1, TID = 0*/
        ori    r5,r5,_MMU_TLB_SZ_256M  /* TS = 0, TSIZE = 256 MByte page size*/
        mtspr  MAS1, r5
        addis  r6,0,HI(LOCAL_MEM_LOCAL_ADRS + 0x10000000) /* EPN = LOCAL_MEM_LOCAL_ADRS */
        ori    r6,r6,0x000a          /* WIMGE = 01010*/
        mtspr  MAS2, r6
        addis  r7,0,HI(LOCAL_MEM_LOCAL_ADRS + 0x10000000)  /* RPN = LOCAL_MEM_LOCAL_ADRS */
        ori    r7,r7,0x003f          /* User/Supervisor XWR*/
        mtspr  MAS3, r7
        isync
        msync
        tlbwe
        tlbsync
#endif

	/*
         * TLB1 #2.  CCSBAR
         * Attributes: UX/UW/UR/SX/SW/SR
         */

        addis  r4,0,0x1002          /* TLBSEL = TLB1(CAM) , ESEL = 2*/
        ori    r4,r4,0x0000
        mtspr  MAS0, r4
        addis  r5,0,0x8000           /* V = 1, IPROT = 0, TID = 0*/
        ori    r5,r5,_MMU_TLB_SZ_1M /* TS = 0, TSIZE = 64 MByte page size*/
        mtspr  MAS1, r5
        addis  r6,0,HI(CCSBAR) /* EPN = CCSBAR */
        ori    r6,r6,0x000a          /* WIMGE = 01010 */
        mtspr  MAS2, r6

        addis  r7,0,HI(CCSBAR) /* RPN = CCSBAR */
        ori    r7,r7,0x0015          /* Supervisor XWR*/
        mtspr  MAS3, r7
        isync
        msync
        tlbwe
        tlbsync

#ifdef INCLUDE_SYSLED
        /*
         * TLB1 #3.  LED - guarded 0xe8000000 -> 0xe8ffffff.
	 * 1MB
         * Attributes: SX/SW/SR
         */

        addis  r4,0,0x1004           /* TLBSEL = TLB1(CAM) , ESEL = 4 */
        ori    r4,r4,0x0000
        mtspr  MAS0, r4
        addis  r5,0,0xc000           /* V = 1, IPROT = 1, TID = 0*/
        ori    r5,r5,_MMU_TLB_SZ_1M  /* TS = 0, TSIZE = 1 MByte page size*/
        mtspr  MAS1, r5
        addis  r6,0,HI(ALPHA_LED_BASE_ADRS)  /* EPN = 0xe8000000*/
        ori    r6,r6,0x000a          /* WIMGE = 10110 */
        mtspr  MAS2, r6
        addis  r7,0,HI(ALPHA_LED_BASE_ADRS)  /* RPN = 0xe8000000*/
        ori    r7,r7,0x0015          /* Supervisor XWR*/
        mtspr  MAS3, r7
        tlbwe
        tlbsync
#endif /* 


        /********************************************/
        /* Setup the memory mapped register address */
        /*******************************************/
	isync
	sync

	lis     r6,HI(CCSBAR)           /* Load the new CCSBAR to r6 */
	ori     r6,r6, LO(CCSBAR)
	isync

        /* CCSRBAR (Default value = 0xfffff700)
         *        8        23
         * +-----+-----------+-----+
         * | --- | BASE_ADDR | --- |
         * +-----+-----------+-----+
         */
	srwi    r5, r6, 12              /* Adjust CCSBAR for BASE_ADDR field */

	lis     r7,HI(0xff700000)       /* Read current value of CCSRBAR */
	ori     r7,r7,LO(0xff700000)
	lwz     r4, 0(r7)

	isync
	sync
	mbar 0

	stw     r5,0(r7)                /* Write the new value to CCSRBAR */

	sync
	isync
	mbar 0

	lis    r5,0xffff                /* Perform a load of an address that */
	ori    r5, r5,0xf800            /* does not access configuration     */
	lwz    r4, 0 (r5)               /* space or the on-chip SRAM, but    */
	isync                           /* has an address mapping already in */
                                        /* effect (e.g, boot ROM)            */

	li     r4,0x2000                /* Busy wait for 0x2000 loop */
	mtctr   r4
ccsrbarWait:
	nop
	bdnz    ccsrbarWait

        /**************************************/
        /* Configure Memory Map for Local Bus */
        /**************************************/

	/* Memory mapped region base address for Local Bus 0 and 1 */

	WRITEADR(r6,r7,M85XX_LAWBAR0(CCSBAR), 0xf0000)

	WRITEADR(r6,r7,M85XX_LAWAR0(CCSBAR),
		 LAWAR_ENABLE | LAWAR_TGTIF_LBC | LAWAR_SIZE_256MB )

	isync

        /* Memory mapped region base address for Local Bus 2 */  
	WRITEADR(r6,r7,M85XX_LAWBAR2(CCSBAR), 0xe8000)

	WRITEADR(r6,r7,M85XX_LAWAR2(CCSBAR),
		 LAWAR_ENABLE | LAWAR_TGTIF_LBC | LAWAR_SIZE_128MB )

	isync


        /* Flash 0 */
	/* load BR0 */
	WRITEADR(r6,r7,M85XX_BR0 (CCSBAR),0xf8001001)
	/* load OR0 */
	WRITEADR(r6,r7,M85XX_OR0 (CCSBAR),0xf8000ff7)

        /* Flash 1 */
        /* load BR1 */
        WRITEADR(r6,r7,M85XX_BR1 (CCSBAR),0xf0001001)
        /* load OR1 */
        WRITEADR(r6,r7,M85XX_OR1 (CCSBAR),0xf8000ff7)

#ifdef INCLUDE_SYSLED
        /* Alphanumeric LED Display */
        /* load BR2 */
        WRITEADR(r6,r7,M85XX_BR2 (CCSBAR),0xe8000801)
        /* load OR2 */
        WRITEADR(r6,r7,M85XX_OR2 (CCSBAR),0xfff00ff7)
#endif /* INCLUDE_SYSLED */

        isync

	isync
	sync
	mbar 0

	/* Initialise the Local Bus Controller */

	li      r4,0x2000
	mtctr   r4
	WRITEADR(r6,r7,M85XX_DCR0(CCSBAR),0xbc0f1bf0)
	WRITEADR(r6,r7,M85XX_DCR1(CCSBAR),0x00078080)
	WRITEADR(r6,r7,M85XX_LCRR(CCSBAR),0x00000002)

	isync
	sync
	mbar 0
dllDelay4:
	nop
	bdnz    dllDelay4


	/* Memory mapped region base address */

	WRITEADR(r6,r7,M85XX_LAWBAR1(CCSBAR),
		 DDR_SDRAM_LOCAL_ADRS >> LAWBAR_ADRS_SHIFT)

	WRITEADR(r6,r7,M85XX_LAWAR1(CCSBAR),
		 LAWAR_ENABLE | LAWAR_TGTIF_DDRSDRAM | LAWAR_SIZE_512MB )

	isync

        /* POR PLL Status Register(PORPLLSR)
         *    10        15       16             17         26        30
         * +-+------------+--------------+--------------+-+------------+---+
         * |-| e500_Ratio | PCI1_clk_sel | PCI2_clk_sel |-| Plat_Ratio | 0 |
         * +-+------------+--------------+--------------+-+------------+---+
         * e500_Ratio    Clock ratio between the e500 core and the CCB clock.
         *               000010 1:1         000110 3:1
         *               000011 3:2         000111 7:2
         *               000100 2:1         001000 4:1
         *               000101 5:2         001001 9:2
         * PCI1_clk_sel  Clock used for PCI1/PCI-X
         *               0 PCI1 runs off of PCI1_CLK
         *               1 PCI1 runs off of SYSCLK
         * PCI2_clk_sel  Clock used for PCI2
         *               0 PCI2 runs off of PCI2_CLK
         *               1 PCI2 runs off of SYSCLK
         * Plat_Ratio    Clock ratio between the CCB clock and SYSCLK
         *               00010 2:1, 00011 3:1, 00100 4:1, etc.,
         */
	lis	r7, HI(M85XX_PORPLLSR(CCSBAR))
	ori     r7, r7, LO(M85XX_PORPLLSR(CCSBAR))
	lwz     r7, 0(r7)
	andi.   r7, r7, 0x3e
	srwi    r7, r7, 1

	/* Get multiplier from table */

	lis     r8, HI(0xffffffff)
	ori     r8, r8, LO(platpllTable)
	add     r8, r8, r7
	lbz     r8, 0(r8)

	cmpwi   r8,0 /* Test for unsupported freq */
	beq     checkStop  /* Jump to 0 */

	/* Initialize the DDR Memory controller */

        lis	r6, HI(DDRBA)
        ori	r6, r6, LO(DDRBA)		/* r6 = DDR base */

        /* DDR SDRAM Clock Control (DDR_SDRAM_CLK_CNTL)
         *        5          8
         * +-----+------------+-----+
         * | --- | CLK_ADJUST | --- |
         * +-----+------------+-----+
         * CLK_ADJUST  Clock adjust
         *   0000 Clock will be launched aligned with address/command
         *   0001 Clock will be launched 1/8 applied cycle after address/command
         *    ....
         *   0111 Clock will be launched 7/8 applied cycle after address/command
         */
	WRITEOFFSET(r6,r7,(DDR_SDRAM_CLK_CTRL), 0x02000000)

        /* Chip Select Memory Bound (CSn_BNDS)
         *        4   15       20  31
         * +-----+------+-----+------+
         * | --- |  SA  | --- |  EA  |
         * +-----+------+-----+------+
         * SA  Starting address for chip select. This value is compared against
         *     the 12 msbs of the 36-bit address.
         * EA  Ending address for chip select.
         * DDR SDRAM Address 0x0 0000 0000 - 0x0 1FFF FFFF (512 MB)
         */
	WRITEOFFSET(r6,r7,(CS0_BNDS), 0x000001ff)
	WRITEOFFSET(r6,r7,(CS1_BNDS), 0x00000000)
	WRITEOFFSET(r6,r7,(CS2_BNDS), 0x00000000)
	WRITEOFFSET(r6,r7,(CS3_BNDS), 0x00000000)

        /* Chip Select Configuration (CSn_CONFIG)
         *     0           8    9         11     13        15
         * +-------+---+-------+------------+---+------------+
         * | CS_EN | - | AP_EN | ODT_RD_CFG | - | ODT_WR_CFG |
         * +-------+---+-------+------------+---+------------+
         *  16        17     21         23     29         31
         * +------------+---+-------------+---+-------------+
         * | BA_BITS_CS | - | ROW_BITS_CS | - | COL_BITS_CS |
         * +------------+---+-------------+---+-------------+
         * CS_EN        Chip select enable
         * AP_EN        Chip select auto-precharge enable
         * ODT_RD_CFG   ODT for read configuration. Only for DDR2.
         * ODT_WR_CFG   ODT for write configuration. Only for DDR2.
         * BA_BITS_CS   Number of bank bits for SDRAM on chip select.
         *              00  2 logical bank bits
         *              01  3 logical bank bits
         * ROW_BITS_CS  Number of row bits for SDRAM on chip select.
         *              000  12 row bits     011  15 row bits
         *              001  13 row bits     100  16 row bits
         *              010  14 row bits
         * COL_BITS_CS  Number of column bit on chip select. (For DDR)
         *              000  8 column bits   010  10 column bits
         *              001  9 column bits   011  11 column bits
         * Micron DDR SDRAM
         *
         *                MT9VDDT3272H MT9VDDT6472H    MT9VDDT12872H
         * Size           256MB        512MB           1GB
         * Refresh count  8K           8K              8K
         * Row addr       8K(A0-A12)   8K(A0-A12)      16K(A0-A13)
         * Device bank    4(BA0,BA1)   4(BA0,BA1)      4(BA0,BA1)
         * Column addr    1K(A0-A9)    2K(A0-A9, A11)  2K(A0-A9, A11)
         * Module Rank    1(S0#)       1(S0#)          1(S0#)
         */
	WRITEOFFSET(r6,r7,(CS0_CONFIG), 0x80800103)

	WRITEOFFSET(r6,r7,(CS1_CONFIG), 0x00000000)
	WRITEOFFSET(r6,r7,(CS2_CONFIG), 0x00000000)
	WRITEOFFSET(r6,r7,(CS3_CONFIG), 0x00000000)

	/* Assume that platform ratio is correctly set to 400MHz */
DDRInit400:
        /* clock will be launched 1/2 applied cycle after address/command */
	WRITEOFFSET(r6,r7,(DDR_SDRAM_CLK_CTRL), 0x02000000)

        /* tRFC = 70ns = 14 clk (at 200MHz) */ 
	WRITEOFFSET(r6,r7,(EXTENDED_REF_REC), 0x00010000)

        /*
         * RWT          Read-to-write turnaround. (default CL - WL + BL/2 + 2)
         * WRT          Write-to-read turnaround. (default WL - CL + BL/2 + 1)
         * RRT          Read-to-read turnaround. (default 3 cycles)
         * WWT          Write-to-write turnaround. (default 2 cycles)
         * ACT_PD_EXT   Active powerdown exit timing.
         * PRE_PD_EXIT  Precharge powerdown exit timing.
         * ODT_PD_EXIT  ODT powerdown exit timing.
         * MRS_CYC      Mode register set cycle time.
         */
	WRITEOFFSET(r6,r7,(TIMING_CFG_0), 0x3f110102)

        /* PRETOACT(tRP)   15ns  (3 clk)
         * ACTTOPRE(tRAS)  40ns  (8 clk)
         * ACTTORW(tRCD)   15ns  (3 clk)
         * CASLAT                (2 clk)
         * REFREC(tRFC)    70ns  (14 clk)
         * WRREC(tWR)      15ns  (3 clk)
         * ACTTOACT(tRRD)  10ns  (2 clk)
         * WRTORD(tWTR)          (2 clk)
         */
	WRITEOFFSET(r6,r7,(TIMING_CFG_1), 0x30356322)

        /* ADD_LAT         0
         * CPO             %11111 (auto calibration)
         * WR_LAT          4 cycles
         * RD_TO_PRE       2 cycles
         * WR_DATA_DELAY   1 (2/8 clk delay)
         * CKE_PLS         3 cycles
         * FOUR_ACT        7 cycles
         */ 
	WRITEOFFSET(r6,r7,(TIMING_CFG_2), 0x0f884441)

        /* Enable DDR interface only after all DDR registers are configured. */ 
#ifdef INCLUDE_DDR_ECC
	WRITEOFFSET(r6,r7,(DDR_SDRAM_CFG), 0x62208000)
#else /* INCLUDE_DDR_ECC */
	WRITEOFFSET(r6,r7,(DDR_SDRAM_CFG), 0x42208000)
#endif /* INCLUDE_DDR_ECC */

        /* */
	WRITEOFFSET(r6,r7,(DDR_SDRAM_CFG_2), 0x00001000)

        /* SDMODE = Normal, CL(3), Sequential, Burst(4 bytes) */
	WRITEOFFSET(r6,r7,(DDR_SDRAM_MODE_CFG), 0x40000132)

	WRITEOFFSET(r6,r7,(DDR_SDRAM_MODE_CFG_2), 0x00000000)
	WRITEOFFSET(r6,r7,(DDR_SDRAM_MD_CNTL), 0x0)

        /* Refresh Interfal (tREFI) = 7.8us (1560clk) */
	WRITEOFFSET(r6,r7,(DDR_SDRAM_INTERVAL), 0x05080000)

	WRITEOFFSET(r6,r7,(DDR_DATA_INIT), 0x0000000)
#if 0
	WRITEOFFSET(r6,r7,(0xf08), 0x0000200)
#endif

finalDDRInit:
	lis    r4,HI(CCSBAR|DDR_IO_OVCR)
	ori    r4,r4,LO(CCSBAR|DDR_IO_OVCR)
	lis    r7,0x9000
	stw    r7,0(r4)

  	isync

	li      r4,0x2000
	mtctr   r4
ddrDelay:
	nop
	bdnz    ddrDelay

#ifdef INCLUDE_DDR_ECC
	WRITEOFFSET(r6,r7,(DDR_SDRAM_CFG), 0xE2208000)
#else /* INCLUDE_DDR_ECC */
	WRITEOFFSET(r6,r7,(DDR_SDRAM_CFG), 0xC2208000)
#endif /* INCLUDE_DDR_ECC */

	isync


	/*
	 * Now that memory is stable we reset TLB entries for standard
	 * operation
	 */

        /*
         * TLB1 #0.  ROM - writethrough cached 0xf0000000 -> 0xffffffff.
	 * 256MB
         * Attributes: SX/SW/SR **PROTECTED**
         */

        addis  r4,0,0x1000           /* TLBSEL = TLB1(CAM) , ESEL = 0 */
        ori    r4,r4,0x0000
        mtspr  MAS0, r4
        addis  r5,0,0xc000           /* V = 1, IPROT = 1, TID = 0*/
        ori    r5,r5,_MMU_TLB_SZ_256M  /* TS = 0, TSIZE = 16 MByte page size*/
        mtspr  MAS1, r5
        addis  r6,0,0xf000           /* EPN = 0xf0000000*/
        ori    r6,r6,0x0016          /* WIMGE = 10110 */
        mtspr  MAS2, r6
        addis  r7,0,0xf000           /* RPN = 0xf0000000*/
        ori    r7,r7,0x0015          /* Supervisor XWR*/
        mtspr  MAS3, r7
        tlbwe
        tlbsync

        /*
         * TLB1 #1.  Main SDRAM - Cached with Coherence
	 * LOCAL_MEM_LOCAL_ADRS -> LOCAL_MEM_LOCAL_ADRS + LOCAL_MEM_SIZE
         * Attributes: UX/UW/UR/SX/SW/SR
         */

        addis  r4,0,0x1001           /* TLBSEL = TLB1(CAM) , ESEL = 1*/
        ori    r4,r4,0x0000
        mtspr  MAS0, r4
        addis  r5,0,0xc000           /* V = 1, IPROT = 1, TID = 0*/
        ori    r5,r5,_MMU_TLB_SZ_256M  /* TS = 0, TSIZE = 512 MByte page size*/
        mtspr  MAS1, r5
        addis  r6,0,HI(LOCAL_MEM_LOCAL_ADRS) /* EPN = LOCAL_MEM_LOCAL_ADRS */
        ori    r6,r6,0x0004          /* WIMGE = 00000 */
        mtspr  MAS2, r6
        addis  r7,0,HI(LOCAL_MEM_LOCAL_ADRS)  /* RPN = LOCAL_MEM_LOCAL_ADRS */
        ori    r7,r7,0x003f          /* User/Supervisor XWR*/
        mtspr  MAS3, r7
        tlbwe
        tlbsync

#if (LOCAL_MEM_SIZE > 0x10000000)
        /*
         * TLB1 #3.  Main SDRAM - Cached with Coherence
	 * LOCAL_MEM_LOCAL_ADRS -> LOCAL_MEM_LOCAL_ADRS + LOCAL_MEM_SIZE
         * Attributes: UX/UW/UR/SX/SW/SR
         */

        addis  r4,0,0x1003           /* TLBSEL = TLB1(CAM) , ESEL = 3*/
        ori    r4,r4,0x0000
        mtspr  MAS0, r4
        addis  r5,0,0xc000           /* V = 1, IPROT = 1, TID = 0*/
        ori    r5,r5,_MMU_TLB_SZ_256M  /* TS = 0, TSIZE = 512 MByte page size*/
        mtspr  MAS1, r5
        addis  r6,0,HI(LOCAL_MEM_LOCAL_ADRS + 0x10000000) /* EPN = LOCAL_MEM_LOCAL_ADRS */
        ori    r6,r6,0x0004          /* WIMGE = 00000 */
        mtspr  MAS2, r6
        addis  r7,0,HI(LOCAL_MEM_LOCAL_ADRS + 0x10000000)  /* RPN = LOCAL_MEM_LOCAL_ADRS */
        ori    r7,r7,0x003f          /* User/Supervisor XWR*/
        mtspr  MAS3, r7
        tlbwe
        tlbsync
#endif

        /*
         * TLB1 #2.  CCSRBAR - guarded 0xe0000000 -> 0xe0ffffff.
	 * 1MB
         * Attributes: SX/SW/SR
         */

        addis  r4,0,0x1002           /* TLBSEL = TLB1(CAM) , ESEL = 2 */
        ori    r4,r4,0x0000
        mtspr  MAS0, r4
        addis  r5,0,0xc000           /* V = 1, IPROT = 0, TID = 0*/
        ori    r5,r5,_MMU_TLB_SZ_1M  /* TS = 0, TSIZE = 1 MByte page size*/
        mtspr  MAS1, r5
        addis  r6,0,HI(CCSBAR)           /* EPN = 0xe0000000*/
        ori    r6,r6,0x000a          /* WIMGE = 10110 */
        mtspr  MAS2, r6
        addis  r7,0,HI(CCSBAR)           /* RPN = 0xe0000000*/
        ori    r7,r7,0x003f          /* Supervisor XWR*/
        mtspr  MAS3, r7
        tlbwe
        tlbsync

#ifdef INCLUDE_SYSLED
        /*
         * TLB1 #4.  LED - guarded 0xe8000000 -> 0xe8ffffff.
	 * 1MB
         * Attributes: SX/SW/SR
         */

        addis  r4,0,0x1004           /* TLBSEL = TLB1(CAM) , ESEL = 4 */
        ori    r4,r4,0x0000
        mtspr  MAS0, r4
        addis  r5,0,0xc000           /* V = 1, IPROT = 0, TID = 0*/
        ori    r5,r5,_MMU_TLB_SZ_1M  /* TS = 0, TSIZE = 1 MByte page size*/
        mtspr  MAS1, r5
        addis  r6,0,HI(ALPHA_LED_BASE_ADRS)  /* EPN = 0xe8000000*/
        ori    r6,r6,0x000a          /* WIMGE = 10110 */
        mtspr  MAS2, r6
        addis  r7,0,HI(ALPHA_LED_BASE_ADRS)  /* RPN = 0xe8000000*/
        ori    r7,r7,0x003f          /* Supervisor XWR*/
        mtspr  MAS3, r7
        tlbwe
        tlbsync
#endif /* INCLUDE_SYSLED */

        b  cold
checkStop:
	ba 0x0
FUNC_END(resetEntry)


#if   defined(_GNU_TOOL)
	.section .reset, "ax", @progbits
#elif defined(_DIAB_TOOL)
	.section .reset, 4, "rx"
#else
#error "Please add a correctly spelled .section directive for your toolchain."
#endif
FUNC_BEGIN(resetVector)
	b	resetEntry
FUNC_END(resetVector)

