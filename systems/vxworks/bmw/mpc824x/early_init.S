#include <ppc_asm.tmpl>


/* Kahlua/MPC8240 defines */
#define VEN_DEV_ID	0x00021057	/* Vendor and Dev. ID for MPC106 */
#define KAHLUA_ID	0x00031057	/* Vendor & Dev Id for Kahlua's PCI */
#define BMC_BASE	0x80000000	/* Kahlua ID in PCI Memory space */
#define CHRP_REG_ADDR	0xfec00000	/* MPC107 Config, Map B */
#define CHRP_REG_DATA	0xfee00000	/* MPC107 Config, Map B */
#define PREP_REG_ADDR	0x80000cf8	/* MPC107 Config, Map A */
#define PREP_REG_DATA	0x80000cfc	/* MPC107 Config, Map A */
#define MPC107_PCI_CMD	0x80000004	/* MPC107 PCI cmd reg */
#define MPC107_PCI_STAT 0x80000006	/* MPC107 PCI status reg */
#define PROC_INT1_ADR	0x800000a8	/* MPC107 Processor i/f cfg1 */
#define PROC_INT2_ADR	0x800000ac	/* MPC107 Processor i/f cfg2 */
#define MEM_CONT1_ADR	0x800000f0	/* MPC107 Memory control config. 1 */
#define MEM_CONT2_ADR	0x800000f4	/* MPC107 Memory control config. 2 */
#define MEM_CONT3_ADR	0x800000f8	/* MPC107 Memory control config. 3 */
#define MEM_CONT4_ADR	0x800000fc	/* MPC107 Memory control config. 4 */
#define MEM_ERREN1_ADR	0x800000c0	/* MPC107 Memory error enable 1 */
#define MEM_START1_ADR	0x80000080	/* MPC107 Memory starting addr */
#define MEM_START2_ADR	0x80000084	/* MPC107 Memory starting addr-lo */
#define XMEM_START1_ADR 0x80000088	/* MPC107 Extended mem. start addr-hi*/
#define XMEM_START2_ADR 0x8000008c	/* MPC107 Extended mem. start addr-lo*/
#define MEM_END1_ADR	0x80000090	/* MPC107 Memory ending address */
#define MEM_END2_ADR	0x80000094	/* MPC107 Memory ending addr-lo */
#define XMEM_END1_ADR	0x80000098	/* MPC107 Extended mem. end addrs-hi */
#define XMEM_END2_ADR	0x8000009c	/* MPC107 Extended mem. end addrs-lo*/
#define OUT_DRV_CONT	0x80000073	/* MPC107 Output Driver Control reg */
#define MEM_EN_ADR	0x800000a0	/* Memory bank enable */
#define PAGE_MODE	0x800000a3	/* MPC107 Page Mode Counter/Timer */

#define MAP_B_CONFIG_ADDR_HIGH	0xfec0	/* Upper half of CONFIG_ADDR for Map B */
#define MAP_B_CONFIG_ADDR_LOW	0x0000	/* Lower half of CONFIG_ADDR for Map B */
#define MAP_B_CONFIG_DATA_HIGH	0xfee0	/* Upper half of CONFIG_DAT for Map B */
#define MAP_B_CONFIG_DATA_LOW	0x0000	/* Lower half of CONFIG_DAT for Map B */


#define CONFIG_ADDR_HIGH    MAP_B_CONFIG_ADDR_HIGH  /* Upper half of CONFIG_ADDR */
#define CONFIG_ADDR_LOW	    MAP_B_CONFIG_ADDR_LOW   /* Lower half of CONFIG_ADDR */
#define CONFIG_DATA_HIGH    MAP_B_CONFIG_DATA_HIGH  /* Upper half of CONFIG_DAT */
#define CONFIG_DATA_LOW	    MAP_B_CONFIG_DATA_LOW   /* Lower half of CONFIG_DAT */

#define CONFIG_ADDR	(CONFIG_ADDR_HIGH << 16 | CONFIG_ADDR_LOW)

#define CONFIG_DATA	(CONFIG_DATA_HIGH << 16 | CONFIG_DATA_LOW)


/* $Id: early_init.S,v 1.2 2011/07/21 16:14:08 yshtil Exp $
 *  configuration register 'addresses'.
 *  These are described in chaper 5 of the 8240 users manual.
 *  Where the register has an abreviation in the manual, this has
 *  been usaed here, otherwise a name in keeping with the norm has
 *  been invented.
 *  Note that some of these registers aren't documented in the manual.
 */

#define PCICR		0x80000004  /* PCI Command Register */
#define PCISR		0x80000006  /* PCI Status Register */
#define PIR		0x80000009  /* PCI Programming Interface Register */
#define PBCCR		0x8000000b  /* PCI Base Class Code Register */
#define PCLSR		0x8000000c  /* Processor Cache Line Size Register */
#define PLTR		0x8000000d  /* PCI Latancy Timer Register */
#define PHTR		0x8000000e  /* PCI Header Type Register */
#define BISTCTRL	0x8000000f  /* BIST Control */
#define LMBAR		0x80000010  /* Local Base Addres Register */
#define PCSRBAR		0x80000014  /* PCSR Base Address Register */
#define ILR		0x8000003c  /* PCI Interrupt Line Register */
#define IPR		0x8000003d  /* Interrupt Pin Register */
#define MINGNT		0x8000003e  /* MIN GNI */
#define MAXLAT		0x8000003f  /* MAX LAT */
#define PCIACR		0x80000046  /* PCI Arbiter Control Register */
#define PMCR1		0x80000070  /* Power management config. 1 */
#define PMCR2		0x80000072  /* Power management config. 2 */
#define ODCR		0x80000073  /* Output Driver Control Register */
#define CLKDCR		0x80000074  /* CLK Driver Control Register */
#define MIOCR1		0x80000076  /* Miscellaneous I/O Control Register 1 */
#define MIOCR2		0x80000077  /* Miscellaneous I/O Control Register 2 */
#define EUMBBAR		0x80000078  /* Embedded Utilities Memory Block Base Address Register */
#define EUMBBAR_VAL	0x80500000  /* PCI Relocation offset for EUMB region */
#define EUMBSIZE	0x00100000  /* Size of EUMB region */

#define MSAR1		0x80000080  /* Memory Starting Address Register 1 */
#define MSAR2		0x80000084  /* Memory Starting Address Register 2 */
#define EMSAR1		0x80000088  /* Extended Memory Starting Address Register 1*/
#define EMSAR2		0x8000008c  /* Extended Memory Starting Address Register 2*/
#define MEAR1		0x80000090  /* Memory Ending Address Register 1 */
#define MEAR2		0x80000094  /* Memory Ending Address Register 2 */
#define EMEAR1		0x80000098  /* Extended Memory Ending Address Register 1 */
#define EMEAR2		0x8000009c  /* Extended Memory Ending Address Register 2 */
#define MBER		0x800000a0  /* Memory bank Enable Register*/
#define MPMR		0x800000a3  /* Memory Page Mode Register (stores PGMAX) */
#define PICR1		0x800000a8  /* Processor Interface Configuration Register 1 */
#define PICR2		0x800000ac  /* Processor Interface Configuration Register 2 */
#define ECCSBECR	0x800000b8  /* ECC Single-Bit Error Counter Register */
#define ECCSBETR	0x800000b8  /* ECC Single-Bit Error Trigger Register */
#define ERRENR1		0x800000c0  /* Error Enableing Register 1 */
#define ERRENR2		0x800000c0  /* Error Enableing Register 2 */
#define ERRDR1		0x800000c1  /* Error Detection Register 1 */
#define IPBESR		0x800000c3  /* Internal Processor Error Status Register */
#define ERRDR2		0x800000c5  /* Error Detection Register 2 */
#define PBESR		0x800000c7  /* PCI Bus Error Status Register */
#define PBEAR		0x800000c8  /* Processor/PCI Bus Error Status Register */

/* XROM regs added by Jimmy */

#define ERCR1		0x800000d0  /* Extended ROM Configuration Register 1 */
#define ERCR2		0x800000d4  /* Extended ROM Configuration Register 2 */
#define ERCR3		0x800000d8  /* Extended ROM Configuration Register 3 */
#define ERCR4		0x800000dc  /* Extended ROM Configuration Register 4 */

#define AMBOR		0x800000e0  /* Address Map B Options Register */
#define MCCR1		0x800000f0  /* Memory Control Configuration Register 1 */
#define MCCR2		0x800000f4  /* Memory Control Configuration Register 2 */
#define MCCR3		0x800000f8  /* Memory Control Configuration Register 3 */
#define MCCR4		0x800000fc  /* Memory Control Configuration Register 4 */

/* some values for some of the above */

#define PICR1_CF_APARK		0x00000008
#define PICR1_LE_MODE		0x00000020
#define PICR1_ST_GATH_EN	0x00000040
#define PICR1_NO_BUSW_CK	0x00000080 /* no bus width check for flash writes */
#define PICR1_DEC		0x00000100 /* Time Base enable on 8245/8241 */
#define PICR1_CF_DPARK		0x00000200
#define PICR1_MCP_EN		0x00000800
#define PICR1_FLASH_WR_EN	0x00001000
#define PICR1_PROC_TYPE_MSK	0x00060000
#define PICR1_PROC_TYPE_603E	0x00040000
#define PICR1_RCS0		0x00100000

#define PICR2_CF_SNOOP_WS_MASK	0x000c0000
#define PICR2_CF_SNOOP_WS_0WS	0x00000000
#define PICR2_CF_SNOOP_WS_1WS	0x00040000
#define PICR2_CF_SNOOP_WS_2WS	0x00080000
#define PICR2_CF_SNOOP_WS_3WS	0x000c0000
#define PICR2_CF_APHASE_WS_MASK 0x0000000c
#define PICR2_CF_APHASE_WS_0WS	0x00000000
#define PICR2_CF_APHASE_WS_1WS	0x00000004
#define PICR2_CF_APHASE_WS_2WS	0x00000008
#define PICR2_CF_APHASE_WS_3WS	0x0000000c

#define MCCR1_ROMNAL_SHIFT	28
#define MCCR1_ROMNAL_MSK	0xf0000000
#define MCCR1_ROMFAL_SHIFT	23
#define MCCR1_ROMFAL_MSK	0x0f800000
#define MCCR1_DBUS_SIZE0TO1_SHIFT  21
#define MCCR1_BURST		0x00100000
#define MCCR1_MEMGO		0x00080000
#define MCCR1_SREN		0x00040000
#define MCCR1_SDRAM_EN		0x00020000
#define MCCR1_PCKEN		0x00010000

#define MCCR2_TS_WAIT_TIMER_MSK 0xe0000000
#define MCCR2_TS_WAIT_TIMER_SHIFT 29
#define MCCR2_ASRISE_MSK	0x1e000000
#define MCCR2_ASRISE_SHIFT	25
#define MCCR2_ASFALL_MSK	0x01e00000
#define MCCR2_ASFALL_SHIFT	21

#define MCCR2_INLINE_PAR_NOT_ECC    0x00100000
#define MCCR2_WRITE_PARITY_CHK	0x00080000
#define MCCR2_INLFRD_PARECC_CHK_EN  0x00040000
#define MCCR2_REFINT_MSK	0x0000fffc
#define MCCR2_REFINT_SHIFT	2
#define MCCR2_RSV_PG		0x00000002
#define MCCR2_PMW_PAR		0x00000001

#define MCCR3_BSTOPRE2TO5_MSK	0xf0000000 /*BSTOPRE[2-5]*/
#define MCCR3_BSTOPRE2TO5_SHIFT 28
#define MCCR3_REFREC_MSK	0x0f000000
#define MCCR3_REFREC_SHIFT	24

#define MCCR4_PRETOACT_MSK	0xf0000000
#define MCCR4_PRETOACT_SHIFT	28
#define MCCR4_ACTTOPRE_MSK	0x0f000000
#define MCCR4_ACTTOPRE_SHIFT	24
#define MCCR4_WMODE		0x00800000
#define MCCR4_INLINE		0x00400000
#define MCCR4_EXTROM		0x00200000 /* enables Extended ROM space */
#define MCCR4_REGISTERED	0x00100000
#define MCCR4_BSTOPRE0TO1_MSK	0x000c0000 /*BSTOPRE[0-1]*/
#define MCCR4_BSTOPRE0TO1_SHIFT 18
#define MCCR4_REGDIMM		0x00008000
#define MCCR4_SDMODE_MSK	0x00007f00
#define MCCR4_SDMODE_SHIFT	8
#define MCCR4_ACTTORW_MSK	0x000000f0
#define MCCR4_ACTTORW_SHIFT	4
#define MCCR4_BSTOPRE6TO9_MSK	0x0000000f /*BSTOPRE[6-9]*/
#define MCCR4_BSTOPRE6TO9_SHIFT 0
#define MCCR4_DBUS_SIZE2_SHIFT	17

#define MICR_ADDR_MASK		0x0ff00000
#define MICR_ADDR_SHIFT		20
#define MICR_EADDR_MASK		0x30000000
#define MICR_EADDR_SHIFT	28

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
  

.globl  early_init_f

early_init_f:
        mflr    r11
        lis     r10, 0x8000

        /* PCI Latency Timer */
        li      r4, 0x0d
        ori     r3, r10, PLTR@l
        bl      __pci_config_write_8
        
        /* Cache Line Size */
        li      r4, 0x08
        ori     r3, r10, PCLSR@l
        bl      __pci_config_write_8
        
        /* PCI Cmd */
        li      r4, 6
        ori     r3, r10, PCICR@l
        bl      __pci_config_write_16
        
#if 1
        /* PCI Stat */
        ori     r3, r10, PCISR@l
        bl      __pci_config_read_16
        ori     r4, r4, 0xffff
        ori     r3, r10, PCISR@l
        bl      __pci_config_write_16
#endif        

        /* PICR1 */
        lis     r4, 0xff14
        ori     r4, r4, 0x1b98
        ori     r3, r10, PICR1@l
        bl      __pci_config_write_32


        /* PICR2 */
        lis     r4, 0x0404
        ori     r4, r4, 0x0004
        ori     r3, r10, PICR2@l
        bl      __pci_config_write_32

	/* MIOCR1 */
        li      r4, 0x04
        ori     r3, r10, MIOCR1@l
        bl      __pci_config_write_8

	/* For the MPC8245, set register 77 to %00100000 (see Errata #15) */
	/* SDRAM_CLK_DEL (0x77)*/
        li      r4, 0x10
        ori     r3, r10, MIOCR2@l
        bl      __pci_config_write_8

        /* EUMBBAR */
        lis     r4, 0xfc00
        ori     r3, r10, EUMBBAR@l
        bl      __pci_config_write_32

	/* AMBOR */

       /* Even if Address Map B is not being used (though it should),
        * the memory DLL needs to be cleared/set/cleared before using memory.
	*/

        ori     r3, r10, AMBOR@l
        bl      __pci_config_read_8	/* get Current bits */

        andi.   r4, r4, 0xffdf
        ori     r3, r10, AMBOR@l
        bl      __pci_config_write_16	/* Clear DLL_RESET */

        ori    r4, r4, 0x0020
        ori     r3, r10, AMBOR@l
        bl      __pci_config_write_16	/* Set DLL_RESET */

        andi.   r4, r4, 0xffdf
        ori     r3, r10, AMBOR@l
        bl      __pci_config_write_16	/* Clear DLL_RESET */

#if 0
        /* ERCR1 */
        lis     r4, 0x8400		/* Enable RCS2, use RCS1 timing parms */
        ori     r4, r4, 0x0000
        ori     r3, r10, ERCR1@l
        bl      __pci_config_write_32
#endif

        /* ERCR1 */
        lis     r4, 0x8040		/* Enable RCS2, use supplied timings */
        ori     r4, r4, 0x8000
        ori     r3, r10, ERCR1@l
        bl      __pci_config_write_32

        /* ERCR2 */
        lis     r4, 0x0000		/* Disable RCS3 parms */
        ori     r4, r4, 0x0000
        ori     r3, r10, ERCR2@l
        bl      __pci_config_write_32

        /* ERCR3 */
        lis     r4, 0x0000		/* RCS3 at 0x70000000, 64K bytes */
        ori     r4, r4, 0x0004
        ori     r3, r10, ERCR2@l
        bl      __pci_config_write_32

	/* Preserve memgo bit */
        /* MCCR1 */
        /* Jimmy: 75a8 looks wrong! */

//	lis     r4, 0x75a8               // Safe Local ROM = 11+3 clocks
  	lis     r4, 0x75a0               // Safe Local ROM = 11+3 clocks
//      lis     r4, 0x73a0               // Fast Local ROM = 7+3 clocks
//      oris    r4, r4, 0x0010           // Burst ROM/Flash enable
//      oris    r4, r4, 0x0004           // Self-refresh enable

//      ori     r4,r4,0xFFFF            // 16Mbit  2bank SDRAM
//      ori     r4,r4,0xAAAA            // 256Mbit 4bank SDRAM (8245 only)
//      ori     r4,r4,0x5555            // 64Mbit  2bank SDRAM
        ori     r4,r4,0x0000            // 64Mbit  4bank SDRAM

        ori     r3, r10, MCCR1@l
        bl      __pci_config_write_32

        /* MCCR2 */

        lis     r4,0x0000
/*      oris    r4,r4,0x4000            // TS_WAIT_TIMER = 3 clocks */
        oris    r4,r4,0x1000            // ASRISE = 8 clocks 
        oris    r4,r4,0x0080            // ASFALL = 8 clocks 
/*      oris    r4,r4,0x0010            // SDRAM Parity (else ECC) */
/*      oris    r4,r4,0x0008            // Write parity check */
/*      oris    r4,r4,0x0004            // SDRAM inline reads */


/* Select a refresh rate; it needs to match the bus speed; if too */
/* slow, data may be lost; if too fast, performance is lost.  We */
/* use the fastest value so we run at all speeds. */
/* Refresh = (15600ns/busclk) - (213 (see UM)). */

/*      ori     r4,r4,0x1d2c            // 133 MHz mem bus        = 1867 */
/*      ori     r4,r4,0x150c            // 100 MHz mem bus        = 1347 */
/*      ori     r4,r4,0x10fc            //  83 MHz mem bus        = 1087 */
#if 0
        ori     r4,r4,0x0cc4            /*  66 MHz mem bus        =  817 */
#endif
#if 1
/* Jimmy: experiment with more frequent refresh */
      ori     r4,r4,0x04cc            //  33 MHz mem bus (SAFE) =  307 */
#endif
/*      ori     r4,r4,0x04cc            //  33 MHz mem bus (SAFE) =  307 */

/*      ori     r4,r4,0x0002            // Reserve a page */
/*      ori     r4,r4,0x0001            // RWM parity */

        ori     r3, r10, MCCR2@l
        bl      __pci_config_write_32


        /* MCCR3 */
        lis     r4,0x0000               // BSTOPRE_M = 7 (see A/N)
        oris    r4,r4,0x0500            // REFREC    = 8 clocks
        ori     r3, r10, MCCR3@l
        bl      __pci_config_write_32

        /* MCCR4 */  /* Jimmy: 2/26 turn on registered buffer mode */
        lis     r4, 0x2000              // PRETOACT = 3 clocks
        oris    r4,r4,0x0400            // ACTOPRE  = 5 clocks
//      oris    r4,r4,0x0080            // Enable 8-beat burst (32-bit bus)
//        oris    r4,r4,0x0040            // Enable Inline ECC/Parity
        oris    r4,r4,0x0020            // EXTROM enabled
        oris    r4,r4,0x0010            // Registered buffers
//      oris    r4,r4,0x0000            // BSTOPRE_U = 0 (see A/N)
        oris    r4,r4,0x0002            // DBUS_SIZ[2] (8 bit on RCS1)

//      ori     r4,r4,0x8000            // Registered DIMMs
        ori     r4,r4,0x2000          //CAS Latency (CL=3) (see RDLAT) 
//        ori     r4,r4,0x2000            // CAS Latency (CL=2) (see RDLAT)
//      ori     r4,r4,0x0300            // Sequential wrap/8-beat burst
        ori     r4,r4,0x0200            // Sequential wrap/4-beat burst
        ori     r4,r4,0x0030            // ACTORW  = 3 clocks
        ori     r4,r4,0x0009            // BSTOPRE_L = 9 (see A/N)

        ori     r3, r10, MCCR4@l
        bl      __pci_config_write_32

	/* MSAR1 */
#if 0
        lis     r4, 0xffffff00
#endif
        lis     r4, 0xc0804000@h
        ori     r4, r4, 0xc0804000@l
        ori     r3, r10, MSAR1@l
        bl      __pci_config_write_32

	/* MSAR2 */
#if 0
        lis     r4, 0xffffffff
#endif
        lis     r4, 0xc0804000@h
        ori     r4, r4, 0xc0804000@l
        ori     r3, r10, MSAR2@l
        bl      __pci_config_write_32

	/* MESAR1 */
#if 0
        lis     r4, 0x03030300@h
        li      r4, 0x03030300@l
#endif
        lis     r4, 0x00000000@h
        ori     r4, r4, 0x00000000@l
        ori     r3, r10, EMSAR1@l
        bl      __pci_config_write_32

	/* MESAR2 */
#if 0
        lis     r4, 0x03030303@h
        li      r4, 0x03030303@l
#endif
        lis     r4, 0x01010101@h
        ori     r4, r4, 0x01010101@l
        ori     r3, r10, EMSAR2@l
        bl      __pci_config_write_32

	/* MEAR1 */
#if 0
        lis     r4, 0xffffff7f@h
        li      r4, 0xffffff7f@l
#endif
        lis     r4, 0xffbf7f3f@h
        ori     r4, r4, 0xffbf7f3f@l
        ori     r3, r10, MEAR1@l
        bl      __pci_config_write_32

	/* MEAR2 */
#if 0
        lis     r4, 0xffffffff@h
        li      r4, 0xffffffff@l
#endif
        lis     r4, 0xffbf7f3f@h
        ori     r4, r4, 0xffbf7f3f@l
        ori     r3, r10, MEAR2@l
        bl      __pci_config_write_32

	/* MEEAR1 */
#if 0
        lis     r4, 0x03030300@h
        li      r4, 0x03030300@l
#endif
        lis     r4, 0x00000000@h
        ori     r4, r4, 0x00000000@l
        ori     r3, r10, EMEAR1@l
        bl      __pci_config_write_32

	/* MEEAR2 */
#if 0
        lis     r4, 0x03030303@h
        li      r4, 0x03030303@l
#endif
        lis     r4, 0x01010101@h
        ori     r4, r4, 0x01010101@l
        ori     r3, r10, EMEAR2@l
        bl      __pci_config_write_32

	/* ODCR */
#if 0
// Jimmy: this value looks weird
        li      r4, 0x1c
#endif
        li      r4, 0x7f
        ori     r3, r10, ODCR@l
        bl      __pci_config_write_8

	/* MBER */
        li      r4, 0x01
        ori     r3, r10, MBER@l
        bl      __pci_config_write_8

        /* Page CTR aka PGMAX */
        li      r4, 0x32
        ori     r3, r10, 0x70
        bl      __pci_config_write_8
        
#if 0
	/* CLK Drive */
        ori     r4, r10, 0xfc01 /* Top bit will be ignored */
        ori     r3, r10, 0x74
        bl      __pci_config_write_16
#endif

	/* delay */ 
        lis     r7, 1
        mtctr   r7
label1: 	bdnz    label1

        /* Set memgo bit */
        /* MCCR1 */
        ori     r3, r10, MCCR1@l
        bl      __pci_config_read_32
        lis	r7, 0x0008
        or	r4, r3, r7
        ori     r3, r10, MCCR1@l
        bl      __pci_config_write_32

	/* delay again */ 
        lis     r7, 1
        mtctr   r7
label2: 	bdnz    label2

#if 0
/* Infinite loop, write then read */
loop:
        lis     r7, 0xffff
        mtctr   r7
        li	r3, 0x5004
        lis	r4, 0xa0a0
        ori	r4, r4, 0x5050
	bl write_32_ne
        li	r3, 0x5004
	bl read_32_ne
      	bdnz    loop
#endif

        mtlr    r11
        blr
