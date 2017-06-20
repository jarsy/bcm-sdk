/* bcm1250DramInit.s: code to initialize & start DRAM controller on bcm1250

/* Copyright 2002 Wind River Systems, Inc. */
        .data
        .globl  copyright_wind_river

/*********************************************************************
*
*  Copyright 2000,2001
*  Broadcom Corporation. All rights reserved.
*
*  This software is furnished under license to Wind River Systems, Inc.
*  and may be used only in accordance with the terms and conditions
*  of this license.  No title or ownership is transferred hereby.
********************************************************************* */

/* $Id: bcm1250DramInit.s,v 1.3 2011/07/21 16:14:43 yshtil Exp $
 * This file has been developed or significantly modified by the
 * MIPS Center of Excellence Dedicated Engineering Staff.
 * This notice is as per the MIPS Center of Excellence Master Partner
 * Agreement, do not remove this notice without checking first with
 * WR/Platforms MIPS Center of Excellence engineering management.
 */

/*
modification history
--------------------
01b,11dec01,agf  coding standard updates
01a,05dec01,agf  created.
*/

/*
DESCRIPTION

This module attempts to initialize the DRAM controllers on
the BCM1250.  Each DRAM controller can control four chip
selects, or two double-sided DDR SDRAM DIMMs.  Therefore, at
most four DIMMs can be attached.

We will assume that all of the DIMMs are connected to the same
SMBUS serial bus, and are addressed sequentially starting from
module 0.   The first two DIMMs will be assigned to memory
controller #0 and the second two DIMMs will be assigned to
memory controller #1.  

There is one serial ROM per DIMM, and we will assume that the
front and back of the DIMM are the same memory configuration.
The first DIMM will be configured for CS0 and CS1, and the
second DIMM will be configured for CS2 and CS3.   If the DIMM
has only one side, it will be assigned to CS0 or CS2.

No interleaving will be configured by this routine, but it
should not be difficult to modify it should that be necessary.

This entire routine needs to run from registers (no read/write
data is allowed).

The steps to initialize the DRAM controller are:

    - Read the SPD, verify DDR SDRAMs or FCRAMs
    - Obtain #rows, #cols, #banks, and module size
    - Calculate row, column, and bank masks
    - Calculate chip selects
    - Calculate timing register.  Note that we assume that
      all banks will use the same timing.
    - Repeat for each DRAM.

DIMM0 -> MCTL0 : CS0, CS1	SPD Addr = 0x54
DIMM1 -> MCTL0 : CS2, CS3	SPD Addr = 0x55
DIMM2 -> MCTL1 : CS0, CS1	SPD Addr = 0x56
DIMM3 -> MCTL1 : CS2, CS3	SPD Addr = 0x57

DRAM Controller registers are programmed in the following order:

	   MC_CS_INTERLEAVE
	   MC_CS_ATTR
	   MC_TEST_DATA, MC_TEST_ECC

	   MC_CSx_ROWS, MC_CSx_COLS
    (repeated for each bank)

	   MC_CS_START, MC_CS_END

	   MC_CLOCK_CFG
    (delay)
	   MC_TIMING
	   MC_CONFIG
    (delay)
	   MC_DRAMMODE
    (delay after each mode setting ??)

Once the registers are initialized, the DRAM is activated by
sending it the following sequence of commands:

     PRE (precharge)
     EMRS (extended mode register set)
     MRS (mode register set)
     PRE (precharge) 
     AR (auto-refresh)
     AR (auto-refresh again)
     MRS (mode register set)

then wait 200 memory clock cycles without accessing DRAM.

Following initialization, the ECC bits must be cleared.  This
can be accomplished by disabling ECC checking on both memory
controllers, and then zeroing all memory via the mapping
in xkseg.

*********************************************************************

Address Bit Assignment Algorithm:

Good performance can be achieved by taking the following steps
when assigning address bits to the row, column, and interleave
masks.  You will need to know the following:

   - The number of rows, columns, and banks on the memory devices
   - The block size (larger tends to be better for sequential
     access)
   - Whether you will interleave chip-selects
   - Whether you will be using both memory controllers and want
     to interleave between them

By choosing the masks carefully you can maximize the number of
open SDRAM banks and reduce access times for nearby and sequential
accesses.

The diagram below depicts a physical address and the order
that the bits should be placed into the masks.  Start with the 
least significant bit and assign bits to the row, column, bank,
and interleave registers in the following order:

        <------------Physical Address--------------->
Bits:	RRRRRRR..R  CCCCCCCC..C  NN  BB  P  CC  xx000
Step:	    7           6        5   4   3  2     1

Where:
    R = Row Address Bit     (MC_CSX_ROW register)
    C = Column Address Bit  (MC_CSX_COL register)
    N = Chip Select         (MC_CS_INTERLEAVE)    
                            (when interleaving via chip selects)
    B = Bank Bit            (MC_CSX_BA register)
    P = Port Select bit     (MC_CONFIG register)  
                            (when interleaving memory channels)
    x = Does not matter     (MC_CSX_COL register) 
                            (internally driven by controller)
    0 = must be zero

When an address bit is "assigned" it is set in one of the masks
in the MC_CSX_ROW, MC_CSX_COL, MC_CSX_BA, or MC_CS_INTERLEAVE
registers.


1. The bottom 3 bits are ignored and should be set to zero.
   The next two bits are also ignored, but are considered
   to be column bits, so they should be taken from the 
   total column bits supported by the device.

2. The next two bits are used for column interleave.  For
   32-byte blocks (and no column interleave), do not use 
   any column bits.  For 64-byte blocks, use one column 
   bit, and for 128 byte blocks, use two column bits.  Subtract 
   the column bits assigned in this step from the total.

3. If you are using both memory controllers and wish to interleave
   between them, assign one bit for the controller interleave. The
   bit number is assigned in the MC_CONFIG register.

4. These bits represent the bank bits on the memory device.
   If the device has 4 banks, assign 2 bits in the MC_CSX_BA 
   register.

5. If you are interleaving via chip-selects, set one or two
   bits in the MC_CS_INTERLEAVE register for the bits that will
   be interleaved.

6. The remaining column bits are assigned in the MC_CSX_COL
   register.

7. The row bits are assigned in the MC_CSX_ROW register.
*/

        /* includes */

#define _ASMLANGUAGE
#include "vxWorks.h"                                                                                                    
#include "asm.h"                                                                                                    
#include "esf.h"                                                                                                    
#include "config.h"

#include "bcm1250.h"
#include "bcm1250Lib.h"

#ifdef _SENTOSA_
#include "sentosa.h"
#elif defined _RHONE_
#include "rhone.h"
#else
#include "swarm.h"
#endif

        /* defines */
#define LED 
#define LED_CHAR0       (32+8*3)

/**********************************************************************
*  Configuration
*/

#define CFG_DRAM_INTERLEAVE	0
#define CFG_DRAM_ECC		0
#define CFG_DRAM_CHANNELS	2
#define CFG_DRAM_SMBUS_CHANNEL	0
#define CFG_DRAM_SMBUS_BASE	0x54
#define CFG_DRAM_TYPE_0		JEDEC
#define CFG_DRAM_TYPE_1		JEDEC
#define CFG_DRAM_PAGE_POLICY	CASCHECK
#define CFG_DRAM_BLOCK_SIZE	32

#define CAS25
#ifdef CAS25
#define V_MC_TIMING_VALUE       V_MC_tFIFO(2) | \
                                V_MC_tRFC(K_MC_tRFC_DEFAULT) | \
                                V_MC_tCwCr(K_MC_tCwCr_DEFAULT) | \
                                V_MC_tRCr(K_MC_tRCr_DEFAULT) | \
                                V_MC_tRCw(K_MC_tRCw_DEFAULT) | \
                                V_MC_tRRD(K_MC_tRRD_DEFAULT) | \
                                V_MC_tRP(K_MC_tRP_DEFAULT) | \
                                V_MC_tCwD(K_MC_tCwD_DEFAULT) | \
                                V_MC_tCrD(K_MC_tCrD_DEFAULT) | \
                                V_MC_tRCD(K_MC_tRCD_DEFAULT) | \
				M_tCrDh | \
                                M_MC_r2rIDLE_TWOCYCLES
#else
#define V_MC_TIMING_VALUE       V_MC_tFIFO(K_MC_tFIFO_DEFAULT) | \
                                V_MC_tRFC(K_MC_tRFC_DEFAULT) | \
                                V_MC_tCwCr(K_MC_tCwCr_DEFAULT) | \
                                V_MC_tRCr(K_MC_tRCr_DEFAULT) | \
                                V_MC_tRCw(K_MC_tRCw_DEFAULT) | \
                                V_MC_tRRD(K_MC_tRRD_DEFAULT) | \
                                V_MC_tRP(K_MC_tRP_DEFAULT) | \
                                V_MC_tCwD(K_MC_tCwD_DEFAULT) | \
                                V_MC_tCrD(K_MC_tCrD_DEFAULT) | \
                                V_MC_tRCD(K_MC_tRCD_DEFAULT) | \
                                M_MC_r2rIDLE_TWOCYCLES
#endif

#define V_MC_CLKCONFIG_VALUE         V_MC_ADDR_SKEW(0x0F) | \
                                     V_MC_DQO_SKEW(0x0F) | \
                                     V_MC_DQI_SKEW(0x8) | \
                                     V_MC_ADDR_DRIVE(0xF) | \
                                     V_MC_DATA_DRIVE(0xF) | \
                                     V_MC_CLOCK_DRIVE(0) | \
                                     V_MC_REF_RATE_DEFAULT 


/**********************************************************************
*  JEDEC constants (serial presence detect, DRAM commands, etc.)
*/

#define JEDEC_SPD_MEMTYPE	2
#define JEDEC_SPD_ROWS		3
#define JEDEC_SPD_COLS		4
#define JEDEC_SPD_SIDES		5
#define JEDEC_SPD_BANKS		17
#define JEDEC_SPD_DENSITY	31

#define JEDEC_MEMTYPE_DDRSDRAM	1
#define JEDEC_MEMTYPE_DDRSDRAM2	7
#define JEDEC_MEMTYPE_SDRAM	4

#define JEDEC_SDRAM_MRVAL	0x23	/* 8-byte bursts, sequential */
#define JEDEC_SDRAM_EMRVAL	0x00

/**********************************************************************
*  Configuration parameter values
*/

#define CLOSED			K_MC_CS_ATTR_CLOSED
#define CASCHECK		K_MC_CS_ATTR_CASCHECK
#define HINT			K_MC_CS_ATTR_HINT
#define OPEN			K_MC_CS_ATTR_OPEN

#define JEDEC			K_MC_DRAM_TYPE_JEDEC
#define FCRAM			K_MC_DRAM_TYPE_FCRAM
#define SGRAM			K_MC_DRAM_TYPE_SGRAM

#if CFG_DRAM_INTERLEAVE
#define CSMODE			V_MC_CS_MODE_INTLV_CS
#else
#define CSMODE			V_MC_CS_MODE_MSB_CS
#endif


        /* macros */

/* in archMips.h these macros are not assembler friendly, so fix for here */
#undef  PHYS_TO_K0
#define PHYS_TO_K0(pa) (K0BASE | (pa))
#undef  PHYS_TO_K1
#define PHYS_TO_K1(pa) (K1BASE | (pa))                                                                         

#define DRAMINFO(mcchan,smbusdev,rows,cols,banks,sides) \
      .byte mcchan,smbusdev,rows,cols,banks,sides,0,0

/*
 * Relocate an address.
 *
 * This macro is used to call routines in a position independent
 * manner. That way code can be relocated from ROM to RAM and 
 * still work properly. Note this will overwrite the ra register.
 */
#define LOADREL(toreg,address) \
        bal     9f; \
9:; \
        la      toreg,address; \
        addu    toreg,ra; \
        la      ra,9b; \
        subu    toreg,ra        


/**********************************************************************
*  Register aliases
*/

#define ret0              v1		/* generic return data */

#define param_sides       a0		/* parameters from table or SMBus */
#define param_banks	  a1
#define param_rows	  a2
#define param_cols	  a3

#define tmp0              t0		/* some scratch registers */
#define tmp1              t1
#define tmp2              t2
#define addr0             t3
#define addr1             t4
#define memctl_base	  t5 		/* KSEG1 of current MC chan */
#define memctl_csmask	  t6 		/* CS bitmask input arg for draminit */
#define param_csnum	  t7 		/* parameter from table or SMbus */

/* s0 unused */

#define param_ptr	  s1 		/* points at cur pos in param table */

#define ttlbits		  s2 		/* addr bits allocated so far */
#define curmemctl	  s2 		/* index of current mem controller */

#define ttlbytes	  s3 		/* total bytes allocated so far */
#define memctl_csinuse	  s4 		/* bitmask of CS's in use */
#define memctl_ctlinuse	  s5 		/* bitmap of controller chans in use */
/* s6, s7 unused */

#define draminit_ra	  t8 		/* return addr save */
#define readparam_ra	  t9 		/* return addr save */
#define smbus_offset	  k0 		/* SMBus device offset */
#define smbus_dev	  k1 		/* SMBus address */
#define smbus_base	  gp 		/* SMBus controller address */
#define dramcmds_ra	  sp 		/* return addr save */
#define smbus_ra	  sp 		/* return addr save */


/**********************************************************************
 *  Configuration data structure
 *
 * Structure as follows:
 *
 * Gloals:
 *    memtype
 *    mode (msb_cs or interleaved)
 *    interleave amt
 *    smbuschan
 *
 * struct draminit {
 *    u_char mcchan;
 *    u_char smbusdev;
 *    u_char rows;
 *    u_char cols;
 *    u_char banks;
 *    u_char chipsel;
 *    u_char reserved0,reserved1;
 *    };
 */

#define R_MCCHAN	0
#define R_SMBUSDEV	1
#define R_ROWS		2
#define R_COLS		3
#define R_BANKS		4
#define R_CSNUM		5
#define R_RESERVED0	6
#define R_RESERVED1	7
#define K_EOT		0xFF		/* end of table reached */
#define K_SMBUS_NODEV	0xFE		/* no device at SMBUS */
#define K_INVMEMTYPE	0xFD		/* invalid memory type installed */
#define K_INITRECSIZE	8		/* size of this record */

        /* globals */

        .globl  bcm1250_dram_init

        /* locals */

        .globl  bcm1250_smbus_init
        .globl  bcm1250_smbus_waitready
        .globl  bcm1250_smbus_readbyte
        .globl  bcm1250_readparams
        .globl  bcm1250_draminit_delay
        .globl  bcm1250_draminitcmds
#if CFG_DRAM_INTERLEAVE
        .globl  bcm1250_dram_intlv
#else
        .globl  bcm1250_dram_msbcs
#endif
        .globl  bcm1250_plldiv_to_mclk_max133
        .globl  bcm1250_dram_zero

		.set reorder

		.text

		.align 8
	

/**********************************************************************
 *  DRAM init table
 */

draminittab:

#define DEVADDR (CFG_DRAM_SMBUS_BASE)								
draminittab:

	#
	# Memory channel 0: Configure via SMBUS
	#

	#       Chan SMBusAdr   Rows Cols Bnks ChpSel
        #       ---- ---------  ---- ---- ---- ------
	DRAMINFO(0,  DEVADDR+0, 0,   0,   0,   0);
	DRAMINFO(0,  DEVADDR+0, 0,   0,   0,   1);
	DRAMINFO(0,  DEVADDR+1, 0,   0,   0,   2);
	DRAMINFO(0,  DEVADDR+1, 0,   0,   0,   3);

	#
	# Memory channel 1: Configure via SMBUS
	#

	#       Chan SMBusAdr   Rows Cols Bnks ChpSel
        #       ---- ---------  ---- ---- ---- ------
	DRAMINFO(1,  DEVADDR+2, 0,   0,   0,   0);
	DRAMINFO(1,  DEVADDR+2, 0,   0,   0,   1);
	DRAMINFO(1,  DEVADDR+3, 0,   0,   0,   2);
	DRAMINFO(1,  DEVADDR+3, 0,   0,   0,   3);

	#
	# End of table marker
	#

	DRAMINFO(K_EOT,0,0,0,0,0);


/**********************************************************************
*  bcm1250_smbus_init - Initialize SMBUS channel
*
*  Input parameters:
*          none
*
*  Return value:
*          smbus_base - KSEG1 address of SMBus channel
*
*  Registers used:
*          tmp0
*/
	
	.ent    bcm1250_smbus_init
bcm1250_smbus_init:
#ifdef LED
        li      tmp0,PHYS_TO_K1(LEDS_PHYS)
        li      tmp1, 'A'  
        sb      tmp1,LED_CHAR0(tmp0)                                                                                
#endif

		li	tmp0,K_SMB_FREQ_100KHZ			# 100KHz clock
		li	smbus_base,PHYS_TO_K1(A_SMB_BASE(CFG_DRAM_SMBUS_CHANNEL))
		sd	tmp0,R_SMB_FREQ(smbus_base)	# Write to clock register
		sd	zero,R_SMB_CONTROL(smbus_base)	# no interrupts, we will poll

		j	ra

	.end    bcm1250_smbus_init

/**********************************************************************
*  
* bcm1250_smbus_waitready -  Wait for SMBUS channel to be ready.
*  
*  Input parameters: 
*  	   smbus_base - SMBus channel base (K1seg addr)
*  	   
*  Return value:
*  	   ret0 - 0 if no error occured, else -1
*  
*  Registers used:
*  	   tmp0,tmp1
*/

        .ent    bcm1250_smbus_waitready
bcm1250_smbus_waitready:
	#
	# Wait for the BUSY bit to be clear
	#

1:      ld	tmp1,R_SMB_STATUS(smbus_base)	# Get status register bits
        andi	tmp0,tmp1,M_SMB_BUSY		# test busy bit into t0
        bnez	tmp0,1b				# still set?  Keep looping..

	#
	# Okay, now test the ERROR bit
	#

        andi	tmp1,tmp1,M_SMB_ERROR		# Isolate error bit
        sd	tmp1,R_SMB_STATUS(smbus_base)	# Clear the error bit.

	#
	# Return -1 if error
	#
        move	ret0,zero			# Assume success
        beq	tmp1,0,1f
        li	ret0,-1				# set to -1 if error bit set

1:      j	ra				# return

        .end    bcm1250_smbus_waitready


/**********************************************************************
* bcm1250_smbus_readbyte - Read a byte from a serial ROM attached 
*                         to an SMBus channel
*  
*  Input parameters: 
*      a0 smbus_dev - address of device on SMBUS
*      a1 smbus_offset - address of byte within device on SMBUS
*      a2 smbus_base - SMBus channel base address (K1seg addr)
*  	   
*  Return value:
*  	   ret0 - byte from device (-1 indicates an error)
*  
*  Registers used:
*  	   tmp0,tmp1
*/


        .ent    bcm1250_smbus_readbyte
bcm1250_smbus_readbyte:
        move	smbus_ra,ra			# save return address

	#
	# Wait for channel to be ready.
	#

        bal	bcm1250_smbus_waitready
        blt	ret0,0,1f

	#
	# Set up a READ BYTE command.  This command has no
	# associated data field, the command code is the data.
	#

        sd	smbus_offset,R_SMB_CMD(smbus_base)  # Write byte-address to cmd

        li	tmp1,V_SMB_TT(K_SMB_TT_CMD_RD1BYTE)
        or	tmp1,tmp1,smbus_dev		# OR in the address
        sd	tmp1,R_SMB_START(smbus_base)

	#
	# Wait for command to complete 
	#

        bal	bcm1250_smbus_waitready
        nop
        blt	ret0,0,1f


	#
	# Get data byte from the DATA register.
	#

        ld	ret0,R_SMB_DATA(smbus_base)
        and	ret0,0xFF		# Just keep one byte

	#
	# Done!
	#
1:      move	ra,smbus_ra
        j	ra

        .end    bcm1250_smbus_readbyte

/**********************************************************************
* 
* bcm1250_readparams - Read next set of parameters from the init table
*  
*  Input parameters: 
*  	   param_ptr - pointer to current place in param table
*	   smbus_base - must be zero on the first call
*  	   
*  Return value:
*      parameter regs filled in
*      ret0 == memctl number (0,1) if ok, else error code
*/

        .ent    bcm1250_readparams
bcm1250_readparams:
#ifdef LED
        li      tmp0,PHYS_TO_K1(LEDS_PHYS)
        li      tmp1, 'a'  
        sb      tmp1,LED_CHAR0(tmp0)           
#endif

        move	readparam_ra,ra

	#
	# Get the memory channel number, or EOT if at the end of the table
	#

        lbu	ret0,R_MCCHAN(param_ptr)
        beq	ret0,K_EOT,rpdone

#ifdef LED
        li      tmp0,PHYS_TO_K1(LEDS_PHYS)
        li      tmp1, 'b'  
        sb      tmp1,LED_CHAR0(tmp0)                                  
#endif

        li	memctl_base,PHYS_TO_K1(A_MC_BASE(0))
        beq	ret0,0,1f
        li	memctl_base,PHYS_TO_K1(A_MC_BASE(1))
1:

	#
	# Read the SMBUS device number.  If zero, well "manually"
	# set the parameters.
	#
		
        lbu	smbus_dev,R_SMBUSDEV(param_ptr)
        beq	smbus_dev,zero,notsmbus

	#
	# Setting parameters via SMBus.  bcm1250_smbus_init
	# sets the smbus_base register.
	#
        bne	smbus_base,zero,1f
        bal	bcm1250_smbus_init
1:

#ifdef LED
        li      tmp0,PHYS_TO_K1(LEDS_PHYS)
        li      tmp1, 'c'  
        sb      tmp1,LED_CHAR0(tmp0)                
#endif
		li	smbus_offset,JEDEC_SPD_MEMTYPE
		bal	bcm1250_smbus_readbyte
		blt	ret0,zero,smberr

#ifdef LED
        li      tmp0,PHYS_TO_K1(LEDS_PHYS)
        li      tmp1, 'd'  
        sb      tmp1,LED_CHAR0(tmp0)                                   
#endif

        beq	ret0,JEDEC_MEMTYPE_DDRSDRAM,dramok
        beq	ret0,JEDEC_MEMTYPE_DDRSDRAM2,dramok
        b	badmemtype

dramok:	
        li	smbus_offset,JEDEC_SPD_SIDES
        bal	bcm1250_smbus_readbyte
        blt	ret0,zero,smberr
        move	param_sides,ret0

#ifdef LED
        li      tmp0,PHYS_TO_K1(LEDS_PHYS)
        li      tmp1, 'e'  
        sb      tmp1,LED_CHAR0(tmp0)           
#endif

        li	smbus_offset,JEDEC_SPD_BANKS
        bal	bcm1250_smbus_readbyte
        blt	ret0,zero,smberr

#ifdef LED
        li      tmp0,PHYS_TO_K1(LEDS_PHYS)
        li      tmp1, 'f'  
        sb      tmp1,LED_CHAR0(tmp0)                         
#endif

	# Determine how many bits the banks represent.  Unlike
	# the rows/columns, the bank byte says how *many* banks
 	# there are, not how many bits represent banks

        li      param_banks,1		# 2 banks = 1 bit
        beq	ret0,2,gotbanks

        li      param_banks,2		# 4 banks = 2 bits
        beq	ret0,4,gotbanks

        li	param_banks,3		# 8 banks = 3 bits
        beq	ret0,8,gotbanks

        li	param_banks,4		# 16 banks = 4 bits
        beq	ret0,16,gotbanks

        b	badmemtype		# invalid bank count

gotbanks:

        li	smbus_offset,JEDEC_SPD_ROWS
        bal	bcm1250_smbus_readbyte
        blt	ret0,zero,smberr
        move	param_rows,ret0

        li	smbus_offset,JEDEC_SPD_COLS
        bal	bcm1250_smbus_readbyte
        blt	ret0,zero,smberr
        move	param_cols,ret0

        lbu	param_csnum,R_CSNUM(param_ptr)

        lbu	ret0,R_MCCHAN(param_ptr)
        add	param_ptr,K_INITRECSIZE
        move	ra,readparam_ra
        j	ra

notsmbus:	
        lbu	param_rows,R_ROWS(param_ptr)
        lbu	param_cols,R_COLS(param_ptr)
        lbu	param_banks,R_BANKS(param_ptr)
        lbu	param_csnum,R_CSNUM(param_ptr)
        move	param_sides,zero
        add	param_ptr,K_INITRECSIZE
        move	ra,readparam_ra
        j	ra

badmemtype:	
        li	ret0,K_INVMEMTYPE
        add	param_ptr,K_INITRECSIZE
        move	ra,readparam_ra
        j	ra

smberr:	li	ret0,K_SMBUS_NODEV
	add	param_ptr,K_INITRECSIZE
rpdone:	move	ra,readparam_ra

#ifdef LED
        li      tmp0,PHYS_TO_K1(LEDS_PHYS)
        li      tmp1, 'g'  
        sb      tmp1,LED_CHAR0(tmp0)             
#endif

        j	ra

        .end    bcm1250_readparams


/**********************************************************************
*  
* bcm1250_draminit_delay - This little routine delays at least 
*                          200 microseconds.
*  
*  Input parameters: 
*  	   nothing
*  	   
*  Return value:
*  	   nothing.
*  	   
*  Registers used:
*  	   tmp0,tmp1
*/

/* 200 microseconds = 5KHz, so delay 1GHz/5Khz = 200,000 cycles */

#ifdef _FASTEMUL_
#define DRAMINIT_DELAY	25
#else
#define DRAMINIT_DELAY	 (1000000000/5000)
#endif

        .ent    bcm1250_draminit_delay
bcm1250_draminit_delay:
        li	tmp1,DRAMINIT_DELAY
        mtc0	zero,C0_COUNT

1:      mfc0	tmp0,C0_COUNT
        ssnop
        ssnop
        blt	tmp0,tmp1,1b

        j	ra

        .end    bcm1250_draminit_delay

/*  *********************************************************************
    *  MAKEDRAMMASK(dest,width,pos)
    *  
    *  Create a 64-bit mask for the DRAM config registers
    *  
    *  Input parameters: 
    *  	   dest - destination register
    *  	   width - number of '1' bits to set
    *  	   pos - position (from the right) of the first '1' bit
    *  	   
    *  Return value:
    *  	   dest register
    ********************************************************************* */

#define MAKEDRAMMASK(dest,width,pos)      \
		dli	dest,1	      ;	  \
		dsll	dest,dest,width ; \
		subu	dest,1	      ;	  \
		dsll	dest,dest,pos ;	  \



/**********************************************************************
* 
* bcm1250_draminitcmds - Issue the sequence of DRAM init commands.
*  
*  Input parameters: 
*      a0 - memory controller base address
*      a1 - chip select mask
*  	   
*  Return value:
*  	   nothing
*/

        .ent    bcm1250_draminitcmds
bcm1250_draminitcmds:
        move	dramcmds_ra,ra

        ori	tmp0,memctl_csmask,V_MC_COMMAND_CLRPWRDN
        sd	tmp0,R_MC_DRAMCMD(memctl_base)
        bal	bcm1250_draminit_delay

        ori	tmp0,memctl_csmask,V_MC_COMMAND_PRE
        sd	tmp0,R_MC_DRAMCMD(memctl_base)
        bal	bcm1250_draminit_delay

        ori	tmp0,memctl_csmask,V_MC_COMMAND_EMRS
        sd	tmp0,R_MC_DRAMCMD(memctl_base)
        bal	bcm1250_draminit_delay

	#
	# Set up for doing mode register writes to the SDRAMs
	# 
	# mode 0x62 is "sequential 4-byte bursts, CAS Latency 2.5"
	# mode 0x22 is "sequential 4-byte bursts, CAS Latency 2"
	#
	#
	# First time, we set bit 8 to reset the DLL
	#

#ifdef CAS25
        dli	tmp0,V_MC_EMODE_DEFAULT | V_MC_MODE(0x62) | \
                     V_MC_MODE(0x100) | V_MC_DRAM_TYPE(CFG_DRAM_TYPE_0)
        beq	curmemctl,0,1f
        dli	tmp0,V_MC_EMODE_DEFAULT | V_MC_MODE(0x62) | \
                     V_MC_MODE(0x100) | V_MC_DRAM_TYPE(CFG_DRAM_TYPE_1)
1:      sd	tmp0,R_MC_DRAMMODE(memctl_base)
#else
        dli	tmp0,V_MC_EMODE_DEFAULT | V_MC_MODE_DEFAULT | \
                     V_MC_MODE(0x100) | V_MC_DRAM_TYPE(CFG_DRAM_TYPE_0)
        beq	curmemctl,0,1f
        dli	tmp0,V_MC_EMODE_DEFAULT | V_MC_MODE_DEFAULT | \
                     V_MC_MODE(0x100) | V_MC_DRAM_TYPE(CFG_DRAM_TYPE_1)
1:      sd	tmp0,R_MC_DRAMMODE(memctl_base)
#endif

	#
	#
        ori	tmp0,memctl_csmask,V_MC_COMMAND_MRS 
        sd	tmp0,R_MC_DRAMCMD(memctl_base)
        bal	bcm1250_draminit_delay

        ori	tmp0,memctl_csmask,V_MC_COMMAND_PRE 
        sd	tmp0,R_MC_DRAMCMD(memctl_base)
        bal	bcm1250_draminit_delay

        ori	tmp0,memctl_csmask,V_MC_COMMAND_AR  
        sd	tmp0,R_MC_DRAMCMD(memctl_base)
        bal	bcm1250_draminit_delay

        ori	tmp0,memctl_csmask,V_MC_COMMAND_AR  
        sd	tmp0,R_MC_DRAMCMD(memctl_base)
        bal	bcm1250_draminit_delay

        #
	# Set up for doing mode register writes to the SDRAMs
	# 
	# mode 0x62 is "sequential 4-byte bursts, CAS Latency 2.5"
	# mode 0x22 is "sequential 4-byte bursts, CAS Latency 2"
	#
	# This time, we clear bit 8 to start the DLL
	#

#ifdef CAS25
        dli	tmp0,V_MC_EMODE_DEFAULT | V_MC_MODE(0x62) | \
                     V_MC_DRAM_TYPE(CFG_DRAM_TYPE_0)
        beq	curmemctl,0,1f
        dli	tmp0,V_MC_EMODE_DEFAULT | V_MC_MODE(0x62) | \
                     V_MC_DRAM_TYPE(CFG_DRAM_TYPE_1)
1:      sd	tmp0,R_MC_DRAMMODE(memctl_base)
#else
        dli	tmp0,V_MC_EMODE_DEFAULT | V_MC_MODE_DEFAULT | \
                     V_MC_DRAM_TYPE(CFG_DRAM_TYPE_0)
        beq	curmemctl,0,1f
        dli	tmp0,V_MC_EMODE_DEFAULT | V_MC_MODE_DEFAULT | \
                     V_MC_DRAM_TYPE(CFG_DRAM_TYPE_1)
1:      sd	tmp0,R_MC_DRAMMODE(memctl_base)
#endif

	#
	#

        ori	tmp0,memctl_csmask,V_MC_COMMAND_MRS 
        sd	tmp0,R_MC_DRAMCMD(memctl_base)
        bal	bcm1250_draminit_delay

        move	ra,dramcmds_ra
        j	ra

        .end    bcm1250_draminitcmds



/**********************************************************************
*  
* bcm1250_dram_msbcs  
*
*  Do row/column/bank initialization for MSB-CS (noninterleaved)
*  mode.  This is only separated out of the main loop to make things
*  read easier, it's not a generally useful subroutine by itself.
*  
*  Input parameters: 
*  	   lots of stuff
*  	   
*  Return value:
*  	   lots of stuff
*/

#if !CFG_DRAM_INTERLEAVE
        .ent    bcm1250_dram_msbcs
bcm1250_dram_msbcs:
	#
	# Three bits are taken by the byte width of the device.
	#

        li	ttlbits,3

	#
	# Calculate the address of the CSx registers for this
	# memory channel and chip select.
	#

        la	addr0,R_MC_CSX_BASE(memctl_base)
        li	tmp0,MC_CSX_SPACING
        mul	tmp0,tmp0,param_csnum
        add	addr0,tmp0


	#
	# The first two bits are always set and are part of the
	# column bits.  Actually, the MC ignores thee bits but
	# we set them here for clarity.
	#

        MAKEDRAMMASK(tmp0,2,ttlbits)
        sd	tmp0,R_MC_CSX_COL(addr0)
        sub	param_cols,2
        add	ttlbits,2


	#
	# Depending on the block size, configure 0, 1, or 2 
	# column bits for column interleave.
	#
#if (CFG_DRAM_BLOCK_SIZE == 32)
        move	 tmp1,zero			/* no column interleave */
#elif (CFG_RAM_BLOCK_SIZE == 64)
        MAKEDRAMMASK(tmp1,1,ttlbits)
        add	ttlbits,1			/* 64-byte column interleave */
        sub	param_cols,1
#else /* CFG_RAM_BLOCK_SIZE == 128 */
        MAKEDRAMMASK(tmp1,2,ttlbits)
        add	ttlbits,2			/* 128 byte column interleave */
        sub	param_cols,2
#endif

        or	tmp0,tmp1
        sd	tmp0,R_MC_CSX_COL(addr0)

	#
	# Make the bank mask.  
	#

        MAKEDRAMMASK(tmp0,param_banks,ttlbits)
        add	ttlbits,param_banks
        sd	tmp0,R_MC_CSX_BA(addr0)



	#
	# Do the rest of the column bits
	#

        MAKEDRAMMASK(tmp0,param_cols,ttlbits)
        ld	tmp2,R_MC_CSX_COL(addr0)
        or	tmp2,tmp0
        sd	tmp2,R_MC_CSX_COL(addr0)
        add	ttlbits,param_cols



	#
	# Calculate the row mask
	#

        MAKEDRAMMASK(tmp0,param_rows,ttlbits)
        sd	tmp0,R_MC_CSX_ROW(addr0)
        add	ttlbits,param_rows


	#
	# The total size (in bytes) of this DIMM is 1 << ttlbits. 
	#
        li	tmp0,1
        dsll	tmp0,ttlbits
        add	addr1,ttlbytes,tmp0		# addr1 = new end address


	# 
	# Generate the addresses for the start/end registers.  These
	# are 16 bit wide fields for each chip select.  Calculate the shift
	# amount for this CS.
	#
        sll	tmp0,param_csnum,4		# csnum*16

	#
	# At this point: 'ttlbytes' is the *start* address
	#                'addr1'    is the *end* address
	#                'tmp0'     is the shift amount for this chip select
	# 
	# Do the start address.
	#
        dsrl	ret0,ttlbytes,24		# chop off low bits
        dsll	ret0,ret0,tmp0			# move into position

        ld	tmp2,R_MC_CS_START(memctl_base)
        or	tmp2,ret0
        sd	tmp2,R_MC_CS_START(memctl_base)

	# 
	# Do the end address.
	#
        dsrl	ret0,addr1,24			# chop off low bits
        dsll	ret0,ret0,tmp0			# move into position

        ld	tmp2,R_MC_CS_END(memctl_base)
        or	tmp2,ret0
        sd	tmp2,R_MC_CS_END(memctl_base)

	#
	# Remember that we did this chip select
	#
        li	tmp0,1
        sll	tmp0,param_csnum
        or	memctl_csinuse,tmp0

	# 
	# Set new byte total
	#

        move	ttlbytes,addr1			# this is new total

        j	ra

        .end    bcm1250_dram_msbcs
#endif



/**********************************************************************
*  
*  bcm1250_dram_intlv
*  
*  Do row/column/bank initialization for 128-byte interleaved
*  mode.  This is only separated out of the main loop to make things
*  read easier, it's not a generally useful subroutine by itself.
*
*  Interleaved modes will assign address bits in the following
*  order:
*
*  RRRRRRRR...R CCCCCCCC...C NN BB P CC xx000
*
*  Where 'R' are row address bits, 
*        'C' are column address bits
*        'N' are chip-select bits
*        'B' are bank select bits
*        'P' is the channel (port) select
*        'x' is ignored by the MC, but will be set to '1'.
*
*  XXX Note: we don't interleave memory channels in this example.
*  
*  Input parameters: 
*  	   lots of stuff
*  	   
*  Return value:
*  	   lots of stuff
*/

#if CFG_DRAM_INTERLEAVE
        .ent    bcm1250_dram_intlv
bcm1250_dram_intlv:
	#
	# Three bits are taken by the byte width of the device.
	#

        li	ttlbits,3


	#
	# The first two bits are always set and are part of the
	# column bits.  Actually, the MC ignores these bits but
	# we set them here for clarity.
	#

        MAKEDRAMMASK(tmp0,2,ttlbits)
        add	ttlbits,2	

        sd	tmp0,R_MC_CS0_COL(memctl_base)
        sub	param_cols,2

	#
	# Depending on the block size, configure 0, 1, or 2 
	# column bits for column interleave.
	#
#if (CFG_DRAM_BLOCK_SIZE == 32)
        move	 tmp1,zero			/* no column interleave */
#elif (CFG_RAM_BLOCK_SIZE == 64)
        MAKEDRAMMASK(tmp1,1,ttlbits)
        add	ttlbits,1			/* 64-byte column interleave */
        sub	param_cols,1
#else /* CFG_RAM_BLOCK_SIZE == 128 */
        MAKEDRAMMASK(tmp1,2,ttlbits)
        add	ttlbits,2			/* 128 byte column interleave */
        sub	param_cols,2
#endif

        or	tmp0,tmp1
        sd	tmp0,R_MC_CS0_COL(memctl_base)

	#
	# Do the port-select interleave (MC 0 and MC 1)
	#

	#
	# Now do the bank mask.
	#

        MAKEDRAMMASK(tmp0,param_banks,ttlbits)
        add	ttlbits,param_banks

        sd	tmp0,R_MC_CS0_BA(memctl_base)
        sd	tmp0,R_MC_CS1_BA(memctl_base)
        sd	tmp0,R_MC_CS2_BA(memctl_base)
        sd	tmp0,R_MC_CS3_BA(memctl_base)


	#
	# The next two bits are the interleave.  They represent
	# the four combinations of the two chip select values.
	#

        MAKEDRAMMASK(tmp0,2,ttlbits)
        add	ttlbits,2

        sd	tmp0,R_MC_CS_INTERLEAVE(memctl_base)

	#
	# Put in the remaining columns
	#

        MAKEDRAMMASK(tmp1,param_cols,ttlbits)
        add	ttlbits,param_cols

        ld	tmp0,R_MC_CS0_COL(memctl_base)
        or	tmp0,tmp1
        sd	tmp0,R_MC_CS0_COL(memctl_base)
        sd	tmp0,R_MC_CS1_COL(memctl_base)
        sd	tmp0,R_MC_CS2_COL(memctl_base)
        sd	tmp0,R_MC_CS3_COL(memctl_base)


	#
	# Calculate the row mask
	#

        MAKEDRAMMASK(tmp0,param_rows,ttlbits)
        add	ttlbits,param_rows

        sd	tmp0,R_MC_CS0_ROW(memctl_base)
        sd	tmp0,R_MC_CS1_ROW(memctl_base)
        sd	tmp0,R_MC_CS2_ROW(memctl_base)
        sd	tmp0,R_MC_CS3_ROW(memctl_base)


	#
	# The total size (in bytes) of this DIMM is 1 << ttlbits. 
	#
        li	tmp0,1
        dsll	tmp0,ttlbits

	#
	# tmp0 is now the size of this memory channel.  Note that
	# it is really 4x the size of the current DIMM , since
	# we started with two extra bits at the beginning.
	#

        add	addr1,ttlbytes,tmp0		# addr1 = new end address


	#
	# At this point: 'ttlbytes' is the *start* address
	#                'addr1'    is the *end* address
	# 
	# Do the start address.
	#
        dsrl	ret0,ttlbytes,24		# chop off low bits

        dsll	tmp2,ret0,S_MC_CS1_START
        or	ret0,tmp2			# OR into CS1
        dsll	tmp2,ret0,S_MC_CS2_START
        or	ret0,tmp2			# and duplicate to CS2, CS3
        sd	ret0,R_MC_CS_START(memctl_base)

	# 
	# Do the end address.
	#
        dsrl	ret0,addr1,24			# chop off low bits

        dsll	tmp2,ret0,S_MC_CS1_END
        or	ret0,tmp2			# OR into CS1
        dsll	tmp2,ret0,S_MC_CS2_END
        or	ret0,tmp2			# and duplicate to CS2, CS3
        sd	ret0,R_MC_CS_END(memctl_base)

	#
	# Remember which chip selects we used
	#
        li	memctl_csinuse,0x0F		# we use all 4 chip selects

	# 
	# Set new byte total 
	#

        move	ttlbytes,addr1			# this is new total

        j	ra

        .end    bcm1250_dram_intlv
#endif



/**********************************************************************
*
* bcm1250_dram_init
* 
*  Initialize DRAM connected to the specified DRAM controller
*  The DRAM will be configured without interleaving, as sequential
*  blocks of memory.
*  
*  Input parameters: 
*  	   a0 - zero to use default memconfig table
*           or KSEG1 address of mem config table
*  	   
*  Return value:
*  	   v0 - total amount of installed DRAM
*  
*  Registers used:
*  	   all
*/


        .ent    bcm1250_dram_init
bcm1250_dram_init:
        move	draminit_ra,ra

	#
	# Save pointer to table
	#

        move	param_ptr,a0

#ifdef LED
        li      tmp0,PHYS_TO_K1(LEDS_PHYS)
        li      tmp1, '0'  
        sb      tmp1,LED_CHAR0(tmp0)                                 
#endif 
	#
	# Begin by initializing the memory channels to some known state.  
	# Set the "BERR_DISABLE" bit for now while we initialize the channels,
	# this will be cleared again before the routine exits.
	#
#ifdef _RHONE_
        la	memctl_base,PHYS_TO_K1(A_MC_BASE(1))
        li	tmp1,1
#else
        la	memctl_base,PHYS_TO_K1(A_MC_BASE(0))
        li	tmp1,CFG_DRAM_CHANNELS
#endif

ctlinitlp:
        dli	tmp0,V_MC_CONFIG_DEFAULT | M_MC_ECC_DISABLE | \
                     V_MC_CS_MODE_MSB_CS | M_MC_BERR_DISABLE
        sd	tmp0,R_MC_CONFIG(memctl_base)

        sd	zero,R_MC_CS_START(memctl_base)
        sd	zero,R_MC_CS_END(memctl_base)
        sd	zero,R_MC_CS_INTERLEAVE(memctl_base)
        sd	zero,R_MC_CS_ATTR(memctl_base)
        sd	zero,R_MC_TEST_DATA(memctl_base)
        sd	zero,R_MC_TEST_ECC(memctl_base)

        add	memctl_base,MC_REGISTER_SPACING	
                                     /* loop for all configured controllers */
        sub	tmp1,1
        bgt	tmp1,0,ctlinitlp
				
#ifdef LED
        li      tmp0,PHYS_TO_K1(LEDS_PHYS)
        li      tmp1, '1'  
        sb      tmp1,LED_CHAR0(tmp0)                  
#endif

	#

	#
	# Get ready for marching down our initialization table
	#

        move	smbus_base,zero         /* this starts out as zero */
        move	ttlbytes,zero           /* no memory yet */
        move	memctl_ctlinuse,zero    /* haven't inited any controllers */
        move	addr1,zero              /* to detect switching controllers */

        bne	param_ptr,zero,initloop /* go if user supplied table */

#ifdef LED
        li      tmp0,PHYS_TO_K1(LEDS_PHYS)
        li      tmp1, '2'  
        sb      tmp1,LED_CHAR0(tmp0)                    
#endif

	#
	# Find the table.  Do this in a PIC way, for certain operating
	# systems dont like us directly looking at offsets in the text
	# segments.
	#

        LOADREL(param_ptr,draminittab)

#ifdef LED
        li      tmp0,PHYS_TO_K1(LEDS_PHYS)
        li      tmp1, '3'  
        sb      tmp1,LED_CHAR0(tmp0)                     
#endif

initloop:	bal	bcm1250_readparams

        beq	ret0,K_EOT,regsdone
        beq	ret0,K_INVMEMTYPE,initloop
        beq	ret0,K_SMBUS_NODEV,initloop

        beq	addr1,memctl_base,1f        /* still on the same controller? */
        move	memctl_csinuse,zero         /* haven't done any chip selects */
1:
	#
	# If the chip select number is odd and we dont have
	# a two-sided DIMM, skip this one.  Sides will also be
	# zero if were doing this manually (i.e., not by SPD)
	#
	# For Interleaved mode, we only care about the parameters for CS0
	# We will assume that proper DIMMs have been installed (DIMMs
	# must be two-sided)
	#

#if CFG_DRAM_INTERLEAVE
        bne	param_csnum,zero,initloop
        beq	param_sides,zero,csok		/* 0 sides = manual, assume 2 */
        beq	param_sides,2,csok		/* 2 sides = ok */
        b	initloop			/* 1 side = bad */
#else
        beq	param_sides,zero,csok		/* 0 sides = manual */

        and	tmp0,param_csnum,1		/* test for oddness */
        beq	tmp0,zero,csok			/* even is always okay */
        beq	param_sides,2,csok		/* 2 sides and CS 1,3 */
        b	initloop			/* 1 side and CS 1,3 */
#endif
csok:
	#
	# Keep track of the fact that we saw this controller.
	# The controller number is in ret0.
	#

        li	tmp0,1
        dsll	tmp0,ret0
        or	memctl_ctlinuse,tmp0


	#
	# Start counting bits.  We keep track of how many bits weve
	# "allocated" so far in order to figure total device size
	# and to properly assign rows, columns, and banks.
	# 
	# We want to make numbers like this:
	#
	#   Row:    0000011111110000000000000000
	#   Col:    0000000000001111111110011000
	#   Bank:   0000000000000000000001100000
	#
	# The total number of "1" bits correspond to the numbers of
	# row, col, bank bits (from the table or the SPD)
	# retrieved from the SPD device.  The two bits 
	# by themselves in the "Col" register dont really need to be
	# set, but make the masks look better in debug output.  They
	# are always column bits, since they are part of the 32-byte
	# bursts.  The bottom 3 bits are zeroes, since
	# the DIMMs are 64 bits wide.
	#
	# If interleaving, two additional column bits are stolen
	# and are placed in the interleave register, just before
	# the bank bits
	#

#ifdef LED
        li      tmp0,PHYS_TO_K1(LEDS_PHYS)
        li      tmp1, '4'  
        sb      tmp1,LED_CHAR0(tmp0)              
#endif

#if CFG_DRAM_INTERLEAVE
        bal	bcm1250_dram_intlv
#else
        bal	bcm1250_dram_msbcs
#endif

#ifdef LED
        li      tmp0,PHYS_TO_K1(LEDS_PHYS)
        li      tmp1, '5'  
        sb      tmp1,LED_CHAR0(tmp0)                   
#endif

	#
	# go back to the beginning
	# 

        move	addr1,memctl_base
        b	initloop

/*
 * We get here when all the start/end and DRAM geometry registers
 * are initialized.
 */

regsdone:	

#ifdef LED
        li      tmp0,PHYS_TO_K1(LEDS_PHYS)
        li      tmp1, '6'  
        sb      tmp1,LED_CHAR0(tmp0)                  
#endif

        la	memctl_base,PHYS_TO_K1(A_MC_BASE(0))
        move	curmemctl,zero

reginitloop:

	#
	# Test to see if we want to init this controller
	#

        li	tmp0,1
        sll	tmp0,curmemctl
        and	tmp0,memctl_ctlinuse
        beq	tmp0,zero,skipreginit

	#
	# Program the clock config register.  This starts the clock to the
	# SDRAMs.  Need to wait 200us after doing this. (6.4.6.1)
	#  


        li	tmp0,PHYS_TO_K1(A_SCD_SYSTEM_CFG)
        ld	tmp0,0(tmp0)
        andi	tmp0,tmp0,M_SYS_PLL_DIV
        dsrl	tmp0,tmp0,S_SYS_PLL_DIV
        LOADREL(tmp1,bcm1250_plldiv_to_mclk_max133)
        addu	tmp1,tmp1,tmp0
        lbu	tmp1,0(tmp1)
#ifdef _RHONE
	dli     tmp1,V_MC_CLK_RATIO_3X   # force div 6, temp fix for pass1 1125
#endif
        dli	tmp0,V_MC_CLKCONFIG_VALUE
        or	tmp0,tmp0,tmp1
        sd	tmp0,R_MC_MCLK_CFG(memctl_base)


        bal	bcm1250_draminit_delay

	#
	# Set up the memory controller config and timing registers
	#

        dli	tmp0,V_MC_CONFIG_DEFAULT | M_MC_ECC_DISABLE | CSMODE | \
                     M_MC_BERR_DISABLE
        sd	tmp0,R_MC_CONFIG(memctl_base)

        dli	tmp0,V_MC_TIMING_VALUE 
        sd	tmp0,R_MC_TIMING1(memctl_base)


	#
	# Set the page policy
	#

        dli	tmp0,V_MC_CS0_PAGE(CFG_DRAM_PAGE_POLICY) | \
                     V_MC_CS1_PAGE(CFG_DRAM_PAGE_POLICY) | \
	             V_MC_CS2_PAGE(CFG_DRAM_PAGE_POLICY) | \
                     V_MC_CS3_PAGE(CFG_DRAM_PAGE_POLICY) 
        sd	tmp0,R_MC_CS_ATTR(memctl_base)



	#
	# Okay, now do the following sequence:
	# PRE-EMRS-MRS-PRE-AR-AR-MRS.  Do this for each chip select,
	# one at a time.
	#

        li	memctl_csmask,M_MC_CS0
        bal	bcm1250_draminitcmds

        li	memctl_csmask,M_MC_CS1
        bal	bcm1250_draminitcmds

        li	memctl_csmask,M_MC_CS2
        bal	bcm1250_draminitcmds

        li	memctl_csmask,M_MC_CS3
        bal	bcm1250_draminitcmds

	#
	# Kill the BERR_DISABLE bit for this controller
	#

        ld	tmp0,R_MC_CONFIG(memctl_base)
        dli	tmp1,~(M_MC_BERR_DISABLE)
        and	tmp0,tmp0,tmp1
        sd	tmp0,R_MC_CONFIG(memctl_base)

	#
	# Do the other controller if necessary
	#

skipreginit:	add	curmemctl,1
        add	memctl_base,MC_REGISTER_SPACING
        blt	curmemctl,CFG_DRAM_CHANNELS,reginitloop

	#
	# If using ECC, zero memory
	#

#if CFG_DRAM_ECC
	#
	# Zero memory
	#
        move	addr0,ttlbytes
#ifdef _FASTEMUL_
        li	addr0,65536			/* fake 64K */
#endif
        bal	bcm1250_dram_zero

	#
	# Turn ECC back on
	#
        li	memctl_base,PHYS_TO_K1(A_MC_BASE_0)
        move	curmemctl,zero

eccinitlp:	li	tmp0,1
        sll	tmp0,curmemctl
        and	tmp0,memctl_ctlinuse
        beq	tmp0,zero,skipeccinit

        ld	tmp0,R_MC_CONFIG(memctl_base)
        dli	tmp1,~(M_MC_ECC_DISABLE)
        and	tmp0,tmp0,tmp1
        sd	tmp0,R_MC_CONFIG(memctl_base)

skipeccinit:	add	curmemctl,1
        add	memctl_base,MC_REGISTER_SPACING
        ble	curmemctl,CFG_DRAM_CHANNELS,eccinitlp

#endif /* CFG_DRAM_ECC */

	#
	# Return the total amount of memory initialized
	#

        move	v0,ttlbytes
        move	ra,draminit_ra
        j	ra

        .end    bcm1250_dram_init

/*************************************************************************
*
*  bcm1250_plldiv_to_mclk_max133
*
*  This table maps the PLL divide ratio in system_cfg[11:7] to
*  the mc_clock_cfg[3:0] value to get the fastest DRAM clock that is
*  at most 133 Mhz.  Assumes 4 <= system_cfg[11:7] <= 22 
*/
        .ent    bcm1250_plldiv_to_mclk_max133
bcm1250_plldiv_to_mclk_max133:
	.byte	V_MC_CLK_RATIO_2X, V_MC_CLK_RATIO_2X  # undefined cpu speeds
	.byte	V_MC_CLK_RATIO_2X, V_MC_CLK_RATIO_2X  # undefined cpu speeds
	.byte	V_MC_CLK_RATIO_2X	#  200 Mhz cpu 	50   Mhz mem
	.byte	V_MC_CLK_RATIO_2X	#  250 Mhz cpu 	62.5 Mhz mem
	.byte	V_MC_CLK_RATIO_2X	#  300 Mhz cpu 	75   Mhz mem
	.byte	V_MC_CLK_RATIO_2X	#  350 Mhz cpu 	87.5 Mhz mem
	.byte	V_MC_CLK_RATIO_2X	#  400 Mhz cpu 100   Mhz mem
	.byte	V_MC_CLK_RATIO_2X	#  450 Mhz cpu 112.5 Mhz mem
#ifdef CAS25
 	.byte	V_MC_CLK_RATIO_2X	#  500 Mhz cpu 125   Mhz mem /* XXX */
#else
	.byte	V_MC_CLK_RATIO_25X	#  500 Mhz cpu 100   Mhz mem /* XXX */
#endif
	.byte	V_MC_CLK_RATIO_25X	#  550 Mhz cpu 110   Mhz mem
	.byte	V_MC_CLK_RATIO_25X	#  600 Mhz cpu 120   Mhz mem
	.byte	V_MC_CLK_RATIO_25X	#  650 Mhz cpu 130   Mhz mem
	.byte	V_MC_CLK_RATIO_3X	#  700 Mhz cpu 116.7 Mhz mem
	.byte	V_MC_CLK_RATIO_3X	#  750 Mhz cpu 125   Mhz mem
	.byte	V_MC_CLK_RATIO_3X	#  800 Mhz cpu 133.3 Mhz mem
	.byte	V_MC_CLK_RATIO_35X	#  850 Mhz cpu 121.4 Mhz mem
	.byte	V_MC_CLK_RATIO_35X	#  900 Mhz cpu 128.6 Mhz mem
	.byte	V_MC_CLK_RATIO_4X	#  950 Mhz cpu 118.8 Mhz mem
	.byte	V_MC_CLK_RATIO_4X	# 1000 Mhz cpu 125   Mhz mem
	.byte	V_MC_CLK_RATIO_4X	# 1050 Mhz cpu 131.3 Mhz mem
	.byte	V_MC_CLK_RATIO_45X	# 1100 Mhz cpu 122.2 Mhz mem
        .end    bcm1250_plldiv_to_mclk_max133
	.align 3


/**********************************************************************
*  ZEROSEG(x)
*  
*  Zero a segment of memory, up to some maximum
*  
*  Input parameters: 
*  	   x - physical address of section to zero
*  	   t2 - maximum number of bytes to zero
*  	   a1 - minimum number of bytes to zero (bytes remaining)
*  	   
*  Return value:
*  	   a1 - bytes now remaining, zero if done
*
*  Registers used:
*  	   t0,t1
*/


#define PHYS_TO_XKPHYS(x) (0x8000000000000000|(x))
#define ZEROSEG(x)   \
	li      t0,x	; \
        bnez	t0,1f	; \
        or	t0,K0BASE ; \
        b	2f ; \
1:      dli	t0,PHYS_TO_XKPHYS(x) ; \
2:      move	t1,a1 ; /* this is how many bytes left */ \
        ble	t1,t2,2f ; /* go if smaller than chunk size */ \
        move	t1,t2 ; /* use smaller size */ \
2:      sub	a1,t1 ; /* subtract out of total */ \
1:      sd	zero,(t0) ; /* store 8 bytes */ \
        sd	zero,8(t0) ; \
        sd	zero,16(t0) ; \
        sd	zero,24(t0) ; \
        add	t0,32 ; /* next 8 bytes	*/ \
        sub	t1,32 ; /* 8 bytes left to do */ \
        bgt	t1,zero,1b


/**********************************************************************
*  
*  bcm1250_dram_zero
*  
*  Zero entire DRAM area to initialize the ECC bits.  Use KXSEG
*  and 64-bit addressing to handle the parts > 256MB. 
*
*  For now, we only handle the first 1GB of memory.
*
*  Input parameters: 
*  	   a0 - size of DRAM area in bytes
*  	   
*  Return value:
*  	   nothing
*/

        .ent    bcm1250_dram_zero
bcm1250_dram_zero:
	# Save the old status register, and set the KX bit.

        mfc0	t7,C0_SR

	# This is the size of each chunk

        li	t2,A_PHYS_MEMORY_SIZE	# 256MB per chunk
        move	a1,a0			# total bytes to clear

	# Do the first 256MB of memory

        ZEROSEG(A_PHYS_MEMORY_0)
        beq	a1,zero,sdzdone

	# Beyond the first 256MB requires setting the KX bit.

        or	t0,t7,SR_KX
        mtc0	t0,C0_SR

        ZEROSEG(A_PHYS_MEMORY_1)
        beq	a1,zero,sdzdone

        ZEROSEG(A_PHYS_MEMORY_2)
        beq	a1,zero,sdzdone

        ZEROSEG(A_PHYS_MEMORY_3)

/*
 *		beq	a1,zero,sdzdone
 *
 *		ZEROSEG(A_PHYS_MEMORY_EXP)
 */

	#
	# Restore old KX bit setting
	#

sdzdone:	mtc0	t7,C0_SR

	# done!

	j	ra

        .end    bcm1250_dram_zero

