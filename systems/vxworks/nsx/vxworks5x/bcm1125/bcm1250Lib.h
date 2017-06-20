/* bcm1250Lib.h - BCM1250 systems-on-chip header file */

/* Copyright 2002 Wind River Systems, Inc. */

/*********************************************************************
*
*  Copyright 2000,2001
*  Broadcom Corporation. All rights reserved.
*
*  This software is furnished under license to Wind River Systems, Inc.
*  and may be used only in accordance with the terms and conditions
*  of this license.  No title or ownership is transferred hereby.
********************************************************************* */

/* $Id: bcm1250Lib.h,v 1.3 2011/07/21 16:14:43 yshtil Exp $
 * This file has been developed or significantly modified by the
 * MIPS Center of Excellence Dedicated Engineering Staff.
 * This notice is as per the MIPS Center of Excellence Master Partner
 * Agreement, do not remove this notice without checking first with
 * WR/Platforms MIPS Center of Excellence engineering management.
 */

/*
modification history
--------------------
01b,17dec01,agf  Replace __ASSEMBLER__ with _ASMLANGUAGE
01a,05dec01,agf  created
*/

/*
DESCRIPTION
This file contains constants for the BCM 1250.  Register address
definitions for the various subsystems are provided, and most (but
not all) register field definitions are provided.

Naming schemes for the BCM 1250 constants are:

  M_xxx           MASK constant (identifies bits in a register). 
                  For multi-bit fields, all bits in the field will
                  be set.

  K_xxx           "Code" constant (value for data in a multi-bit
                  field).  The value is right justified.

  V_xxx           "Value" constant.  This is the same as the 
                  corresponding "K_xxx" constant, except it is
                  shifted to the correct position in the register.

  S_xxx           SHIFT constant.  This is the number of bits that
                  a field value (code) needs to be shifted 
                  (towards the left) to put the value in the right
                  position for the register.

  A_xxx           ADDRESS constant.  This will be a physical 
                  address.  Use the PHYS_TO_K1 macro to generate
                  a K1SEG address.

  R_xxx           RELATIVE offset constant.  This is an offset from
                  an A_xxx constant (usually the first register in
                  a group).
  
  G_xxx(X)        GET value.  This macro obtains a multi-bit field
                  from a register, masks it, and shifts it to
                  the bottom of the register (retrieving a K_xxx
                  value, for example).

  V_xxx(X)        VALUE.  This macro computes the value of a
                  K_xxx constant shifted to the correct position
                  in the register.
*/

#ifndef __INCbcm1250Libh
#define __INCbcm1250Libh

#include "vxWorks.h"

#ifdef __cplusplus
extern "C" {
#endif

         /* macros */

/*
 * Cast to 64-bit number.  Presumably the syntax is different in 
 * assembly language.
 *
 */

#if !defined(_ASMLANGUAGE)
#define _SB_MAKE64(x) ((UINT64)(x))
#define _SB_MAKE32(x) ((UINT32)(x))
#else
#define _SB_MAKE64(x) (x)
#define _SB_MAKE32(x) (x)
#endif


/*
 * Make a mask for 1 bit at position 'n'
 */

#define _SB_MAKEMASK1(n) (_SB_MAKE64(1) << _SB_MAKE64(n))
#define _SB_MAKEMASK1_32(n) (_SB_MAKE32(1) << _SB_MAKE32(n))

/*
 * Make a mask for 'v' bits at position 'n'
 */

#define _SB_MAKEMASK(v,n) (_SB_MAKE64((_SB_MAKE64(1)<<(v))-1) << _SB_MAKE64(n))
#define _SB_MAKEMASK_32(v,n) (_SB_MAKE32((_SB_MAKE32(1)<<(v))-1) << _SB_MAKE32(n))

/*
 * Make a value at 'v' at bit position 'n'
 */

#define _SB_MAKEVALUE(v,n) (_SB_MAKE64(v) << _SB_MAKE64(n))
#define _SB_MAKEVALUE_32(v,n) (_SB_MAKE32(v) << _SB_MAKE32(n))

#define _SB_GETVALUE(v,n,m) ((_SB_MAKE64(v) & _SB_MAKE64(m)) >> _SB_MAKE64(n))
#define _SB_GETVALUE_32(v,n,m) ((_SB_MAKE32(v) & _SB_MAKE32(m)) >> _SB_MAKE32(n))

/*
 * Macros to read/write on-chip registers
 */

#if !defined(_ASMLANGUAGE)
#define SBWRITECSR(csr,val) *((volatile UINT64 *) PHYS_TO_K1(csr)) = (val)
#define SBREADCSR(csr) (*((volatile UINT64 *) PHYS_TO_K1(csr)))
#endif /* _ASMLANGUAGE*/


         /* defines */

/*  *********************************************************************
    *  Some general notes:
    *  
    *  For the most part, when there is more than one peripheral
    *  of the same type on the SOC, the constants below will be
    *  offsets from the base of each peripheral.  For example,
    *  the MAC registers are described as offsets from the first
    *  MAC register, and there will be a MAC_REGISTER() macro
    *  to calculate the base address of a given MAC.  
    *  
    *  The information in this file is based on the BCM1250 SOC
    *  manual version 0.2, July 2000.
    ********************************************************************* */


/*  ********************************************************************* 
    * Memory Controller Registers
    ********************************************************************* */

#define A_MC_BASE_0                 0x0010051000
#define A_MC_BASE_1                 0x0010052000
#define MC_REGISTER_SPACING         0x1000

#define A_MC_BASE(ctlid)            ((ctlid)*MC_REGISTER_SPACING+A_MC_BASE_0)
#define A_MC_REGISTER(ctlid,reg)    (A_MC_BASE(ctlid)+(reg))

#define R_MC_CONFIG                 0x0000000100
#define R_MC_DRAMCMD                0x0000000120
#define R_MC_DRAMMODE               0x0000000140
#define R_MC_TIMING1                0x0000000160
#define R_MC_TIMING2                0x0000000180
#define R_MC_CS_START               0x00000001A0
#define R_MC_CS_END                 0x00000001C0
#define R_MC_CS_INTERLEAVE          0x00000001E0
#define S_MC_CS_STARTEND            16

#define R_MC_CSX_BASE               0x0000000200
#define R_MC_CSX_ROW                0x0000000000	/* relative to CSX_BASE, above */
#define R_MC_CSX_COL                0x0000000020	/* relative to CSX_BASE, above */
#define R_MC_CSX_BA                 0x0000000040	/* relative to CSX_BASE, above */
#define MC_CSX_SPACING              0x0000000060	/* relative to CSX_BASE, above */

#define R_MC_CS0_ROW                0x0000000200
#define R_MC_CS0_COL                0x0000000220
#define R_MC_CS0_BA                 0x0000000240
#define R_MC_CS1_ROW                0x0000000260
#define R_MC_CS1_COL                0x0000000280
#define R_MC_CS1_BA                 0x00000002A0
#define R_MC_CS2_ROW                0x00000002C0
#define R_MC_CS2_COL                0x00000002E0
#define R_MC_CS2_BA                 0x0000000300
#define R_MC_CS3_ROW                0x0000000320
#define R_MC_CS3_COL                0x0000000340
#define R_MC_CS3_BA                 0x0000000360
#define R_MC_CS_ATTR                0x0000000380
#define R_MC_TEST_DATA              0x0000000400
#define R_MC_TEST_ECC               0x0000000420
#define R_MC_MCLK_CFG               0x0000000500

/*  ********************************************************************* 
    * L2 Cache Control Registers
    ********************************************************************* */

#define A_L2_READ_ADDRESS           0x0010040018
#define A_L2_EEC_ADDRESS            0x0010040038
#define A_L2_WAY_DISABLE            0x0010041000
#define A_L2_MAKEDISABLE(x)         (A_L2_WAY_DISABLE | (((~(x))&0x0F) << 8))
#define A_L2_MGMT_TAG_BASE          0x00D0000000

/*  ********************************************************************* 
    * PCI Interface Registers
    ********************************************************************* */

#define A_PCI_TYPE00_HEADER         0x00DE000000
#define A_PCI_TYPE01_HEADER         0x00DE000800


/*  ********************************************************************* 
    * Ethernet DMA and MACs
    ********************************************************************* */

#define A_MAC_BASE_0                0x0010064000
#define A_MAC_BASE_1                0x0010065000
#define A_MAC_BASE_2                0x0010066000

#define MAC_SPACING                 0x1000
#define MAC_DMA_TXRX_SPACING        0x0400
#define MAC_DMA_CHANNEL_SPACING     0x0100
#define DMA_RX                      0
#define DMA_TX                      1
#define MAC_NUM_DMACHAN		    2		    /* channels per direction */

#define MAC_NUM_PORTS               3

#define A_MAC_CHANNEL_BASE(macnum)                  \
            (A_MAC_BASE_0 +                         \
             MAC_SPACING*(macnum))

#define A_MAC_REGISTER(macnum,reg)                  \
            (A_MAC_BASE_0 +                         \
             MAC_SPACING*(macnum) + (reg))


#define R_MAC_DMA_CHANNELS		0x800 /* Relative to A_MAC_CHANNEL_BASE */

#define A_MAC_DMA_CHANNEL_BASE(macnum,txrx,chan)    \
             ((A_MAC_CHANNEL_BASE(macnum)) +        \
             R_MAC_DMA_CHANNELS +                   \
             (MAC_DMA_TXRX_SPACING*(txrx)) +        \
             (MAC_DMA_CHANNEL_SPACING*(chan)))

#define R_MAC_DMA_CHANNEL_BASE(txrx,chan)    \
             (R_MAC_DMA_CHANNELS +                   \
             (MAC_DMA_TXRX_SPACING*(txrx)) +        \
             (MAC_DMA_CHANNEL_SPACING*(chan)))

#define A_MAC_DMA_REGISTER(macnum,txrx,chan,reg)           \
            (A_MAC_DMA_CHANNEL_BASE(macnum,txrx,chan) +    \
            (reg))

#define R_MAC_DMA_REGISTER(txrx,chan,reg)           \
            (R_MAC_DMA_CHANNEL_BASE(txrx,chan) +    \
            (reg))

/* 
 * DMA channel registers, relative to A_MAC_DMA_CHANNEL_BASE
 */

#define R_MAC_DMA_CONFIG0               0x00000000
#define R_MAC_DMA_CONFIG1               0x00000008
#define R_MAC_DMA_DSCR_BASE             0x00000010
#define R_MAC_DMA_DSCR_CNT              0x00000018
#define R_MAC_DMA_CUR_DSCRA             0x00000020
#define R_MAC_DMA_CUR_DSCRB             0x00000028
#define R_MAC_DMA_CUR_DSCRADDR          0x00000030

/*
 * RMON Counters
 */

#define R_MAC_RMON_TX_BYTES             0x00000000
#define R_MAC_RMON_COLLISIONS           0x00000008
#define R_MAC_RMON_LATE_COL             0x00000010
#define R_MAC_RMON_EX_COL               0x00000018
#define R_MAC_RMON_FCS_ERROR            0x00000020
#define R_MAC_RMON_TX_ABORT             0x00000028
/* Counter #6 (0x30) now reserved */
#define R_MAC_RMON_TX_BAD               0x00000038
#define R_MAC_RMON_TX_GOOD              0x00000040
#define R_MAC_RMON_TX_RUNT              0x00000048
#define R_MAC_RMON_TX_OVERSIZE          0x00000050
#define R_MAC_RMON_RX_BYTES             0x00000080
#define R_MAC_RMON_RX_MCAST             0x00000088
#define R_MAC_RMON_RX_BCAST             0x00000090
#define R_MAC_RMON_RX_BAD               0x00000098
#define R_MAC_RMON_RX_GOOD              0x000000A0
#define R_MAC_RMON_RX_RUNT              0x000000A8
#define R_MAC_RMON_RX_OVERSIZE          0x000000B0
#define R_MAC_RMON_RX_FCS_ERROR         0x000000B8
#define R_MAC_RMON_RX_LENGTH_ERROR      0x000000C0
#define R_MAC_RMON_RX_CODE_ERROR        0x000000C8
#define R_MAC_RMON_RX_ALIGN_ERROR       0x000000D0

/* Updated to spec 0.2 */
#define R_MAC_CFG                       0x00000100
#define R_MAC_THRSH_CFG                 0x00000108
#define R_MAC_VLANTAG                   0x00000110
#define R_MAC_FRAMECFG                  0x00000118
#define R_MAC_EOPCNT                    0x00000120
#define R_MAC_FIFO_PTRS                 0x00000130
#define R_MAC_ADFILTER_CFG              0x00000200
#define R_MAC_ETHERNET_ADDR             0x00000208
#define R_MAC_PKT_TYPE                  0x00000210
#define R_MAC_HASH_BASE                 0x00000240
#define R_MAC_ADDR_BASE                 0x00000280
#define R_MAC_CHLO0_BASE                0x00000300
#define R_MAC_CHUP0_BASE                0x00000320
#define R_MAC_ENABLE                    0x00000400
#define R_MAC_STATUS                    0x00000408
#define R_MAC_INT_MASK                  0x00000410
#define R_MAC_TXD_CTL                   0x00000420
#define R_MAC_MDIO                      0x00000428
#define R_MAC_DEBUG_STATUS              0x00000448

#define MAC_HASH_COUNT			8
#define MAC_ADDR_COUNT			8
#define MAC_CHMAP_COUNT			4


/*  ********************************************************************* 
    * DUART Registers
    ********************************************************************* */


#define R_DUART_NUM_PORTS           2

#define A_DUART                     0x0010060000

#define A_DUART_REG(r)

#define DUART_CHANREG_SPACING       0x100
#define A_DUART_CHANREG(chan,reg)   (A_DUART + DUART_CHANREG_SPACING*(chan) + (reg))
#define R_DUART_CHANREG(chan,reg)   (DUART_CHANREG_SPACING*(chan) + (reg))

#define R_DUART_MODE_REG_1	    0x100
#define R_DUART_MODE_REG_2	    0x110
#define R_DUART_STATUS              0x120
#define R_DUART_CLK_SEL             0x130
#define R_DUART_CMD                 0x150
#define R_DUART_RX_HOLD             0x160
#define R_DUART_TX_HOLD             0x170

/*
 * The IMR and ISR can't be addressed with A_DUART_CHANREG,
 * so use this macro instead.
 */

#define R_DUART_AUX_CTRL            0x310
#define R_DUART_ISR_A               0x320
#define R_DUART_IMR_A               0x330
#define R_DUART_ISR_B               0x340
#define R_DUART_IMR_B               0x350
#define R_DUART_OUT_PORT            0x360
#define R_DUART_OPCR                0x370

#define R_DUART_SET_OPR		    0x3B0
#define R_DUART_CLEAR_OPR	    0x3C0

#define DUART_IMRISR_SPACING        0x20

#define R_DUART_IMRREG(chan)	    (R_DUART_IMR_A + (chan)*DUART_IMRISR_SPACING)
#define R_DUART_ISRREG(chan)	    (R_DUART_ISR_A + (chan)*DUART_IMRISR_SPACING)

#define A_DUART_IMRREG(chan)	    (A_DUART + R_DUART_IMRREG(chan))
#define A_DUART_ISRREG(chan)	    (A_DUART + R_DUART_ISRREG(chan))

/*
 * These constants are the absolute addresses.
 */

#define A_DUART_MODE_REG_1_A        0x0010060100
#define A_DUART_MODE_REG_2_A        0x0010060110
#define A_DUART_STATUS_A            0x0010060120
#define A_DUART_CLK_SEL_A           0x0010060130
#define A_DUART_CMD_A               0x0010060150
#define A_DUART_RX_HOLD_A           0x0010060160
#define A_DUART_TX_HOLD_A           0x0010060170

#define A_DUART_MODE_REG_1_B        0x0010060200
#define A_DUART_MODE_REG_2_B        0x0010060210
#define A_DUART_STATUS_B            0x0010060220
#define A_DUART_CLK_SEL_B           0x0010060230
#define A_DUART_CMD_B               0x0010060250
#define A_DUART_RX_HOLD_B           0x0010060260
#define A_DUART_TX_HOLD_B           0x0010060270

#define A_DUART_INPORT_CHNG         0x0010060300
#define A_DUART_AUX_CTRL            0x0010060310
#define A_DUART_ISR_A               0x0010060320
#define A_DUART_IMR_A               0x0010060330
#define A_DUART_ISR_B               0x0010060340
#define A_DUART_IMR_B               0x0010060350
#define A_DUART_OUT_PORT            0x0010060360
#define A_DUART_OPCR                0x0010060370
#define A_DUART_IN_PORT             0x0010060380
#define A_DUART_ISR                 0x0010060390
#define A_DUART_IMR                 0x00100603A0
#define A_DUART_SET_OPR             0x00100603B0
#define A_DUART_CLEAR_OPR           0x00100603C0
#define A_DUART_INPORT_CHNG_A       0x00100603D0
#define A_DUART_INPORT_CHNG_B       0x00100603E0

/*  ********************************************************************* 
    * Synchronous Serial Registers
    ********************************************************************* */


#define A_SER_BASE_0                0x0010060400
#define A_SER_BASE_1                0x0010060800
#define SER_SPACING                 0x400

#define SER_DMA_TXRX_SPACING        0x80

#define SER_NUM_PORTS               2

#define A_SER_CHANNEL_BASE(sernum)                  \
            (A_SER_BASE_0 +                         \
             SER_SPACING*(sernum))

#define A_SER_REGISTER(sernum,reg)                  \
            (A_SER_BASE_0 +                         \
             SER_SPACING*(sernum) + (reg))


#define R_SER_DMA_CHANNELS		0   /* Relative to A_SER_BASE_x */

#define A_SER_DMA_CHANNEL_BASE(sernum,txrx)    \
             ((A_SER_CHANNEL_BASE(sernum)) +        \
             R_SER_DMA_CHANNELS +                   \
             (SER_DMA_TXRX_SPACING*(txrx)))

#define A_SER_DMA_REGISTER(sernum,txrx,reg)           \
            (A_SER_DMA_CHANNEL_BASE(sernum,txrx) +    \
            (reg))


/* 
 * DMA channel registers, relative to A_SER_DMA_CHANNEL_BASE
 */

#define R_SER_DMA_CONFIG0           0x00000000
#define R_SER_DMA_CONFIG1           0x00000008
#define R_SER_DMA_DSCR_BASE         0x00000010
#define R_SER_DMA_DSCR_CNT          0x00000018
#define R_SER_DMA_CUR_DSCRA         0x00000020
#define R_SER_DMA_CUR_DSCRB         0x00000028
#define R_SER_DMA_CUR_DSCRADDR      0x00000030

#define R_SER_DMA_CONFIG0_RX        0x00000000
#define R_SER_DMA_CONFIG1_RX        0x00000008
#define R_SER_DMA_DSCR_BASE_RX      0x00000010
#define R_SER_DMA_DSCR_COUNT_RX     0x00000018
#define R_SER_DMA_CUR_DSCR_A_RX     0x00000020
#define R_SER_DMA_CUR_DSCR_B_RX     0x00000028
#define R_SER_DMA_CUR_DSCR_ADDR_RX  0x00000030

#define R_SER_DMA_CONFIG0_TX        0x00000080
#define R_SER_DMA_CONFIG1_TX        0x00000088
#define R_SER_DMA_DSCR_BASE_TX      0x00000090
#define R_SER_DMA_DSCR_COUNT_TX     0x00000098
#define R_SER_DMA_CUR_DSCR_A_TX     0x000000A0
#define R_SER_DMA_CUR_DSCR_B_TX     0x000000A8
#define R_SER_DMA_CUR_DSCR_ADDR_TX  0x000000B0

#define R_SER_MODE                  0x00000100
#define R_SER_MINFRM_SZ             0x00000108
#define R_SER_MAXFRM_SZ             0x00000110
#define R_SER_ADDR                  0x00000118
#define R_SER_USR0_ADDR             0x00000120
#define R_SER_USR1_ADDR             0x00000128
#define R_SER_USR2_ADDR             0x00000130
#define R_SER_USR3_ADDR             0x00000138
#define R_SER_CMD                   0x00000140
#define R_SER_TX_RD_THRSH           0x00000160
#define R_SER_TX_WR_THRSH           0x00000168
#define R_SER_RX_RD_THRSH           0x00000170
#define R_SER_LINE_MODE		    0x00000178
#define R_SER_DMA_ENABLE            0x00000180
#define R_SER_INT_MASK              0x00000190
#define R_SER_STATUS                0x00000188
#define R_SER_STATUS_DEBUG          0x000001A8
#define R_SER_RX_TABLE_BASE         0x00000200
#define SER_RX_TABLE_COUNT          16
#define R_SER_TX_TABLE_BASE         0x00000300
#define SER_TX_TABLE_COUNT          16

/* RMON Counters */
#define R_SER_RMON_TX_BYTE_LO       0x000001C0
#define R_SER_RMON_TX_BYTE_HI       0x000001C8
#define R_SER_RMON_RX_BYTE_LO       0x000001D0
#define R_SER_RMON_RX_BYTE_HI       0x000001D8
#define R_SER_RMON_TX_UNDERRUN      0x000001E0
#define R_SER_RMON_RX_OVERFLOW      0x000001E8
#define R_SER_RMON_RX_ERRORS        0x000001F0
#define R_SER_RMON_RX_BADADDR       0x000001F8

/*  ********************************************************************* 
    * Generic Bus Registers
    ********************************************************************* */

#define IO_EXT_CFG_COUNT            8

#define A_IO_EXT_BASE		    0x0010061000
#define A_IO_EXT_REG(r)		    (A_IO_EXT_BASE + (r))

#define A_IO_EXT_CFG_BASE           0x0010061000
#define A_IO_EXT_MULT_SIZE_BASE     0x0010061100
#define A_IO_EXT_START_ADDR_BASE    0x0010061200
#define A_IO_EXT_TIME_CFG0_BASE     0x0010061600
#define A_IO_EXT_TIME_CFG1_BASE     0x0010061700

#define IO_EXT_REGISTER_SPACING	    8
#define A_IO_EXT_CS_BASE(cs)	    (A_IO_EXT_CFG_BASE+IO_EXT_REGISTER_SPACING*(cs))
#define R_IO_EXT_REG(reg,cs)	    ((cs)*IO_EXT_REGISTER_SPACING + (reg))

#define R_IO_EXT_CFG		    0x0000
#define R_IO_EXT_MULT_SIZE          0x0100
#define R_IO_EXT_START_ADDR	    0x0200
#define R_IO_EXT_TIME_CFG0          0x0600
#define R_IO_EXT_TIME_CFG1          0x0700


#define A_IO_INTERRUPT_STATUS       0x0010061A00
#define A_IO_INTERRUPT_DATA0        0x0010061A10
#define A_IO_INTERRUPT_DATA1        0x0010061A18
#define A_IO_INTERRUPT_DATA2        0x0010061A20
#define A_IO_INTERRUPT_DATA3        0x0010061A28
#define A_IO_INTERRUPT_ADDR0        0x0010061A30
#define A_IO_INTERRUPT_ADDR1        0x0010061A40
#define A_IO_INTERRUPT_PARITY       0x0010061A50
#define A_IO_PCMCIA_CFG             0x0010061A60
#define A_IO_PCMCIA_STATUS          0x0010061A70
#define A_IO_DRIVE_0		    0x0010061300
#define A_IO_DRIVE_1		    0x0010061308
#define A_IO_DRIVE_2		    0x0010061310
#define A_IO_DRIVE_3		    0x0010061318

#define R_IO_INTERRUPT_STATUS       0x0A00
#define R_IO_INTERRUPT_DATA0        0x0A10
#define R_IO_INTERRUPT_DATA1        0x0A18
#define R_IO_INTERRUPT_DATA2        0x0A20
#define R_IO_INTERRUPT_DATA3        0x0A28
#define R_IO_INTERRUPT_ADDR0        0x0A30
#define R_IO_INTERRUPT_ADDR1        0x0A40
#define R_IO_INTERRUPT_PARITY       0x0A50
#define R_IO_PCMCIA_CFG             0x0A60
#define R_IO_PCMCIA_STATUS          0x0A70

/*  ********************************************************************* 
    * GPIO Registers
    ********************************************************************* */

#define A_GPIO_CLR_EDGE             0x0010061A80
#define A_GPIO_INT_TYPE             0x0010061A88
#define A_GPIO_INPUT_INVERT         0x0010061A90
#define A_GPIO_GLITCH               0x0010061A98
#define A_GPIO_READ                 0x0010061AA0
#define A_GPIO_DIRECTION            0x0010061AA8
#define A_GPIO_PIN_CLR              0x0010061AB0
#define A_GPIO_PIN_SET              0x0010061AB8

#define A_GPIO_BASE		    0x0010061A80

#define R_GPIO_CLR_EDGE             0x00
#define R_GPIO_INT_TYPE             0x08
#define R_GPIO_INPUT_INVERT         0x10
#define R_GPIO_GLITCH               0x18
#define R_GPIO_READ                 0x20
#define R_GPIO_DIRECTION            0x28
#define R_GPIO_PIN_CLR              0x30
#define R_GPIO_PIN_SET              0x38

/*  ********************************************************************* 
    * SMBus Registers
    ********************************************************************* */

#define A_SMB_XTRA_0                0x0010060000
#define A_SMB_XTRA_1                0x0010060008
#define A_SMB_FREQ_0                0x0010060010
#define A_SMB_FREQ_1                0x0010060018
#define A_SMB_STATUS_0              0x0010060020
#define A_SMB_STATUS_1              0x0010060028
#define A_SMB_CMD_0                 0x0010060030
#define A_SMB_CMD_1                 0x0010060038
#define A_SMB_START_0               0x0010060040
#define A_SMB_START_1               0x0010060048
#define A_SMB_DATA_0                0x0010060050
#define A_SMB_DATA_1                0x0010060058
#define A_SMB_CONTROL_0             0x0010060060
#define A_SMB_CONTROL_1             0x0010060068
#define A_SMB_PEC_0                 0x0010060070
#define A_SMB_PEC_1                 0x0010060078

#define A_SMB_0                     0x0010060000
#define A_SMB_1                     0x0010060008
#define SMB_REGISTER_SPACING        0x8
#define A_SMB_BASE(idx)             (A_SMB_0+(idx)*SMB_REGISTER_SPACING)
#define A_SMB_REGISTER(idx,reg)     (A_SMB_BASE(idx)+(reg))

#define R_SMB_XTRA                  0x0000000000
#define R_SMB_FREQ                  0x0000000010
#define R_SMB_STATUS                0x0000000020
#define R_SMB_CMD                   0x0000000030
#define R_SMB_START                 0x0000000040
#define R_SMB_DATA                  0x0000000050
#define R_SMB_CONTROL               0x0000000060
#define R_SMB_PEC                   0x0000000070

/*  ********************************************************************* 
    * Timer Registers
    ********************************************************************* */

/*
 * Watchdog timers
 */

#define A_SCD_WDOG_0		    0x0010020050
#define A_SCD_WDOG_1                0x0010020150
#define SCD_WDOG_SPACING            0x100
#define SCD_NUM_WDOGS		    2
#define A_SCD_WDOG_BASE(w)          (A_SCD_WDOG_0+SCD_WDOG_SPACING*(w))
#define A_SCD_WDOG_REGISTER(w,r)    (A_SCD_WDOG_BASE(w) + (r))

#define R_SCD_WDOG_INIT		    0x0000000000
#define R_SCD_WDOG_CNT		    0x0000000008
#define R_SCD_WDOG_CFG		    0x0000000010

#define A_SCD_WDOG_INIT_0           0x0010020050
#define A_SCD_WDOG_CNT_0            0x0010020058
#define A_SCD_WDOG_CFG_0            0x0010020060

#define A_SCD_WDOG_INIT_1           0x0010020150
#define A_SCD_WDOG_CNT_1            0x0010020158
#define A_SCD_WDOG_CFG_1            0x0010020160

/*
 * Generic timers
 */

#define A_SCD_TIMER_0		    0x0010020070
#define A_SCD_TIMER_1               0x0010020078
#define A_SCD_TIMER_2		    0x0010020170
#define A_SCD_TIMER_3               0x0010020178
#define SCD_NUM_TIMERS		    4
#define A_SCD_TIMER_BASE(w)         (A_SCD_TIMER_0+0x08*((w)&1)+0x100*(((w)&2)>>1))
#define A_SCD_TIMER_REGISTER(w,r)   (A_SCD_TIMER_BASE(w) + (r))

#define R_SCD_TIMER_INIT	    0x0000000000
#define R_SCD_TIMER_CNT		    0x0000000010
#define R_SCD_TIMER_CFG		    0x0000000020

#define A_SCD_TIMER_INIT_0          0x0010020070
#define A_SCD_TIMER_CNT_0           0x0010020080
#define A_SCD_TIMER_CFG_0           0x0010020090

#define A_SCD_TIMER_INIT_1          0x0010020078
#define A_SCD_TIMER_CNT_1           0x0010020088
#define A_SCD_TIMER_CFG_1           0x0010020098

#define A_SCD_TIMER_INIT_2          0x0010020170
#define A_SCD_TIMER_CNT_2           0x0010020180
#define A_SCD_TIMER_CFG_2           0x0010020190

#define A_SCD_TIMER_INIT_3          0x0010020178
#define A_SCD_TIMER_CNT_3           0x0010020188
#define A_SCD_TIMER_CFG_3           0x0010020198

/*  ********************************************************************* 
    * System Control Registers
    ********************************************************************* */

#define A_SCD_SYSTEM_REVISION       0x0010020000
#define A_SCD_SYSTEM_CFG            0x0010020008

#define A_SCD_SCRATCH		    0x0010020C10	/* PASS2 */

/*  ********************************************************************* 
    * System Address Trap Registers
    ********************************************************************* */

#define A_ADDR_TRAP_INDEX           0x00100200B0
#define A_ADDR_TRAP_REG             0x00100200B8
#define A_ADDR_TRAP_UP_0            0x0010020400
#define A_ADDR_TRAP_UP_1            0x0010020408
#define A_ADDR_TRAP_UP_2            0x0010020410
#define A_ADDR_TRAP_UP_3            0x0010020418
#define A_ADDR_TRAP_DOWN_0          0x0010020420
#define A_ADDR_TRAP_DOWN_1          0x0010020428
#define A_ADDR_TRAP_DOWN_2          0x0010020430
#define A_ADDR_TRAP_DOWN_3          0x0010020438
#define A_ADDR_TRAP_CFG_0           0x0010020440
#define A_ADDR_TRAP_CFG_1           0x0010020448
#define A_ADDR_TRAP_CFG_2           0x0010020450
#define A_ADDR_TRAP_CFG_3           0x0010020458


/*  ********************************************************************* 
    * System Interrupt Mapper Registers
    ********************************************************************* */

#define A_IMR_CPU0_BASE                 0x0010020000
#define A_IMR_CPU1_BASE                 0x0010022000
#define IMR_REGISTER_SPACING            0x2000

#define A_IMR_MAPPER(cpu) (A_IMR_CPU0_BASE+(cpu)*IMR_REGISTER_SPACING)
#define A_IMR_REGISTER(cpu,reg) (A_IMR_MAPPER(cpu)+(reg))

#define R_IMR_INTERRUPT_DIAG            0x0010
#define R_IMR_INTERRUPT_MASK            0x0028
#define R_IMR_INTERRUPT_TRACE           0x0038
#define R_IMR_INTERRUPT_SOURCE_STATUS   0x0040
#define R_IMR_LDT_INTERRUPT_SET         0x0048
#define R_IMR_LDT_INTERRUPT             0x0018
#define R_IMR_LDT_INTERRUPT_CLR         0x0020
#define R_IMR_MAILBOX_CPU               0x00c0
#define R_IMR_ALIAS_MAILBOX_CPU         0x1000
#define R_IMR_MAILBOX_SET_CPU           0x00C8
#define R_IMR_ALIAS_MAILBOX_SET_CPU     0x1008
#define R_IMR_MAILBOX_CLR_CPU           0x00D0
#define R_IMR_INTERRUPT_STATUS_BASE     0x0100
#define R_IMR_INTERRUPT_STATUS_COUNT    7
#define R_IMR_INTERRUPT_MAP_BASE        0x0200
#define R_IMR_INTERRUPT_MAP_COUNT       64

/*  ********************************************************************* 
    * System Performance Counter Registers
    ********************************************************************* */

#define A_SCD_PERF_CNT_CFG          0x00100204C0
#define A_SCD_PERF_CNT_0            0x00100204D0
#define A_SCD_PERF_CNT_1            0x00100204D8
#define A_SCD_PERF_CNT_2            0x00100204E0
#define A_SCD_PERF_CNT_3            0x00100204E8

/*  ********************************************************************* 
    * System Bus Watcher Registers
    ********************************************************************* */

#define A_SCD_BUS_ERR_STATUS        0x0010020880
#define A_BUS_ERR_DATA_0            0x00100208A0
#define A_BUS_ERR_DATA_1            0x00100208A8
#define A_BUS_ERR_DATA_2            0x00100208B0
#define A_BUS_ERR_DATA_3            0x00100208B8
#define A_BUS_L2_ERRORS             0x00100208C0
#define A_BUS_MEM_IO_ERRORS         0x00100208C8

/*  ********************************************************************* 
    * System Debug Controller Registers
    ********************************************************************* */

#define A_SCD_JTAG_BASE             0x0010000000

/*  ********************************************************************* 
    * System Trace Buffer Registers
    ********************************************************************* */

#define A_SCD_TRACE_CFG             0x0010020A00
#define A_SCD_TRACE_READ            0x0010020A08
#define A_SCD_TRACE_EVENT_0         0x0010020A20
#define A_SCD_TRACE_EVENT_1         0x0010020A28
#define A_SCD_TRACE_EVENT_2         0x0010020A30
#define A_SCD_TRACE_EVENT_3         0x0010020A38
#define A_SCD_TRACE_SEQUENCE_0      0x0010020A40
#define A_SCD_TRACE_SEQUENCE_1      0x0010020A48
#define A_SCD_TRACE_SEQUENCE_2      0x0010020A50
#define A_SCD_TRACE_SEQUENCE_3      0x0010020A58
#define A_SCD_TRACE_EVENT_4         0x0010020A60
#define A_SCD_TRACE_EVENT_5         0x0010020A68
#define A_SCD_TRACE_EVENT_6         0x0010020A70
#define A_SCD_TRACE_EVENT_7         0x0010020A78
#define A_SCD_TRACE_SEQUENCE_4      0x0010020A80
#define A_SCD_TRACE_SEQUENCE_5      0x0010020A88
#define A_SCD_TRACE_SEQUENCE_6      0x0010020A90
#define A_SCD_TRACE_SEQUENCE_7      0x0010020A98

/*  ********************************************************************* 
    * System Generic DMA Registers
    ********************************************************************* */

#define A_DM_0		  	    0x0010020B00
#define A_DM_1		  	    0x0010020B20
#define A_DM_2			    0x0010020B40
#define A_DM_3			    0x0010020B60
#define DM_REGISTER_SPACING	    0x20
#define DM_NUM_CHANNELS		    4
#define A_DM_BASE(idx) (A_DM_0 + ((idx) * DM_REGISTER_SPACING))
#define A_DM_REGISTER(idx,reg) (A_DM_BASE(idx) + (reg))

#define R_DM_DSCR_BASE		    0x0000000000
#define R_DM_DSCR_COUNT		    0x0000000008
#define R_DM_CUR_DSCR_ADDR	    0x0000000010
#define R_DM_DSCR_BASE_DEBUG	    0x0000000018


/*  *********************************************************************
    *  Physical Address Map
    ********************************************************************* */

#define A_PHYS_MEMORY_0                 _SB_MAKE64(0x0000000000)
#define A_PHYS_MEMORY_SIZE              _SB_MAKE64((256*1024*1024))
#define A_PHYS_SYSTEM_CTL               _SB_MAKE64(0x0010000000)
#define A_PHYS_IO_SYSTEM                _SB_MAKE64(0x0010060000)
#define A_PHYS_GENBUS			_SB_MAKE64(0x0010090000)
#define A_PHYS_GENBUS_END		_SB_MAKE64(0x0040000000)
#define A_PHYS_LDTPCI_IO_MATCH_BYTES_32 _SB_MAKE64(0x0040000000)
#define A_PHYS_LDTPCI_IO_MATCH_BITS_32  _SB_MAKE64(0x0060000000)
#define A_PHYS_MEMORY_1                 _SB_MAKE64(0x0080000000)
#define A_PHYS_MEMORY_2                 _SB_MAKE64(0x0090000000)
#define A_PHYS_MEMORY_3                 _SB_MAKE64(0x00C0000000)
#define A_PHYS_L2_CACHE_TEST            _SB_MAKE64(0x00D0000000)
#define A_PHYS_LDT_SPECIAL_MATCH_BYTES  _SB_MAKE64(0x00D8000000)
#define A_PHYS_LDTPCI_IO_MATCH_BYTES    _SB_MAKE64(0x00DC000000)
#define A_PHYS_LDTPCI_CFG_MATCH_BYTES   _SB_MAKE64(0x00DE000000)
#define A_PHYS_LDT_SPECIAL_MATCH_BITS   _SB_MAKE64(0x00F8000000)
#define A_PHYS_LDTPCI_IO_MATCH_BITS     _SB_MAKE64(0x00FC000000)
#define A_PHYS_LDTPCI_CFG_MATCH_BITS    _SB_MAKE64(0x00FE000000)
#define A_PHYS_MEMORY_EXP               _SB_MAKE64(0x0100000000)
#define A_PHYS_MEMORY_EXP_SIZE          _SB_MAKE64((508*1024*1024*1024))
#define A_PHYS_LDT_EXP                  _SB_MAKE64(0x8000000000)
#define A_PHYS_PCI_FULLACCESS_BYTES     _SB_MAKE64(0xF000000000)
#define A_PHYS_PCI_FULLACCESS_BITS      _SB_MAKE64(0xF100000000)
#define A_PHYS_RESERVED                 _SB_MAKE64(0xF200000000)
#define A_PHYS_RESERVED_SPECIAL_LDT     _SB_MAKE64(0xFD00000000)

#define A_PHYS_L2CACHE_WAY_SIZE         _SB_MAKE64(0x0000020000)
#define PHYS_L2CACHE_NUM_WAYS           4
#define A_PHYS_L2CACHE_TOTAL_SIZE       _SB_MAKE64(0x0000080000)
#define A_PHYS_L2CACHE_WAY0             _SB_MAKE64(0x00D0180000)
#define A_PHYS_L2CACHE_WAY1             _SB_MAKE64(0x00D01A0000)
#define A_PHYS_L2CACHE_WAY2             _SB_MAKE64(0x00D01C0000)
#define A_PHYS_L2CACHE_WAY3             _SB_MAKE64(0x00D01E0000)


/*  *********************************************************************
    *
    *  The remainder are field definitions within the SOC registers
    *
    *  The information in this file is based on the BCM1250 SOC
    *  manual version 0.2, July 2000.
    ********************************************************************* */

/*  *********************************************************************
    *  System control/debug register constants
    ********************************************************************* */

/*
 * System Revision Register (Table 4-1)
 */

#define M_SYS_RESERVED		    _SB_MAKEMASK(8,0)

#define S_SYS_REVISION              _SB_MAKE64(8)
#define M_SYS_REVISION              _SB_MAKEMASK(8,S_SYS_REVISION)
#define V_SYS_REVISION(x)           _SB_MAKEVALUE(x,S_SYS_REVISION)
#define G_SYS_REVISION(x)           _SB_GETVALUE(x,S_SYS_REVISION,M_SYS_REVISION)

#define K_SYS_REVISION_PASS1	    1
#define K_SYS_REVISION_PASS2	    3
#define K_SYS_REVISION_PASS3	    4 /* XXX Unknown */

#define S_SYS_PART                  _SB_MAKE64(16)
#define M_SYS_PART                  _SB_MAKEMASK(16,S_SYS_PART)
#define V_SYS_PART(x)               _SB_MAKEVALUE(x,S_SYS_PART)
#define G_SYS_PART(x)               _SB_GETVALUE(x,S_SYS_PART,M_SYS_PART)

#define K_SYS_PART_BCM1250           0x1250
#define K_SYS_PART_SB1125           0x1125 

#define S_SYS_WID                   _SB_MAKE64(32)
#define M_SYS_WID                   _SB_MAKEMASK(32,S_SYS_WID)
#define V_SYS_WID(x)                _SB_MAKEVALUE(x,S_SYS_WID)
#define G_SYS_WID(x)                _SB_GETVALUE(x,S_SYS_WID,M_SYS_WID)

/*
 * System Config Register (Table 4-2)
 * Register: SCD_SYSTEM_CFG
 */

#define M_SYS_LDT_PLL_BYP           _SB_MAKEMASK1(3)
#define M_SYS_PCI_SYNC_TEST_MODE    _SB_MAKEMASK1(4)
#define M_SYS_IOB0_DIV              _SB_MAKEMASK1(5)
#define M_SYS_IOB1_DIV              _SB_MAKEMASK1(6)

#define S_SYS_PLL_DIV               _SB_MAKE64(7)
#define M_SYS_PLL_DIV               _SB_MAKEMASK(5,S_SYS_PLL_DIV)
#define V_SYS_PLL_DIV(x)            _SB_MAKEVALUE(x,S_SYS_PLL_DIV)
#define G_SYS_PLL_DIV(x)            _SB_GETVALUE(x,S_SYS_PLL_DIV,M_SYS_PLL_DIV)

#define M_SYS_SER0_ENABLE           _SB_MAKEMASK1(12)
#define M_SYS_SER0_RSTB_EN          _SB_MAKEMASK1(13)
#define M_SYS_SER1_ENABLE           _SB_MAKEMASK1(14)
#define M_SYS_SER1_RSTB_EN          _SB_MAKEMASK1(15)
#define M_SYS_PCMCIA_ENABLE         _SB_MAKEMASK1(16)

#define S_SYS_BOOT_MODE             _SB_MAKE64(17)
#define M_SYS_BOOT_MODE             _SB_MAKEMASK(2,S_SYS_BOOT_MODE)
#define V_SYS_BOOT_MODE(x)          _SB_MAKEVALUE(x,S_SYS_BOOT_MODE)
#define G_SYS_BOOT_MODE(x)          _SB_GETVALUE(x,S_SYS_BOOT_MODE,M_SYS_BOOT_MODE)
#define K_SYS_BOOT_MODE_ROM32       0
#define K_SYS_BOOT_MODE_ROM8        1
#define K_SYS_BOOT_MODE_SMBUS_SMALL 2
#define K_SYS_BOOT_MODE_SMBUS_BIG   3

#define M_SYS_PCI_HOST              _SB_MAKEMASK1(19)
#define M_SYS_PCI_ARBITER           _SB_MAKEMASK1(20)
#define M_SYS_SOUTH_ON_LDT          _SB_MAKEMASK1(21)
#define M_SYS_BIG_ENDIAN            _SB_MAKEMASK1(22)
#define M_SYS_GENCLK_EN             _SB_MAKEMASK1(23)
#define M_SYS_LDT_TEST_EN           _SB_MAKEMASK1(24)
#define M_SYS_GEN_PARITY_EN         _SB_MAKEMASK1(25)

#define S_SYS_CONFIG                26
#define M_SYS_CONFIG                _SB_MAKEMASK(6,S_SYS_CONFIG)
#define V_SYS_CONFIG(x)             _SB_MAKEVALUE(x,S_SYS_CONFIG)
#define G_SYS_CONFIG(x)             _SB_GETVALUE(x,S_SYS_CONFIG,M_SYS_CONFIG)

/* The following bits are writeable by JTAG only. */

#define M_SYS_CLKSTOP               _SB_MAKEMASK1(32)
#define M_SYS_CLKSTEP               _SB_MAKEMASK1(33)

#define S_SYS_CLKCOUNT              34
#define M_SYS_CLKCOUNT              _SB_MAKEMASK(8,S_SYS_CLKCOUNT)
#define V_SYS_CLKCOUNT(x)           _SB_MAKEVALUE(x,S_SYS_CLKCOUNT)
#define G_SYS_CLKCOUNT(x)           _SB_GETVALUE(x,S_SYS_CLKCOUNT,M_SYS_CLKCOUNT)

#define M_SYS_PLL_BYPASS            _SB_MAKEMASK1(42)

#define S_SYS_PLL_IREF		    43
#define M_SYS_PLL_IREF		    _SB_MAKEMASK(2,S_SYS_PLL_IREF)

#define S_SYS_PLL_VCO		    45
#define M_SYS_PLL_VCO		    _SB_MAKEMASK(2,S_SYS_PLL_VCO)

#define S_SYS_PLL_VREG		    47
#define M_SYS_PLL_VREG		    _SB_MAKEMASK(2,S_SYS_PLL_VREG)

#define M_SYS_MEM_RESET             _SB_MAKEMASK1(49)
#define M_SYS_L2C_RESET             _SB_MAKEMASK1(50)
#define M_SYS_IO_RESET_0            _SB_MAKEMASK1(51)
#define M_SYS_IO_RESET_1            _SB_MAKEMASK1(52)
#define M_SYS_SCD_RESET             _SB_MAKEMASK1(53)

/* End of bits writable by JTAG only. */

#define M_SYS_CPU_RESET_0           _SB_MAKEMASK1(54)
#define M_SYS_CPU_RESET_1           _SB_MAKEMASK1(55)

#define M_SYS_UNICPU0               _SB_MAKEMASK1(56)
#define M_SYS_UNICPU1               _SB_MAKEMASK1(57)

#define M_SYS_SB_SOFTRES            _SB_MAKEMASK1(58)
#define M_SYS_EXT_RESET             _SB_MAKEMASK1(59)
#define M_SYS_SYSTEM_RESET          _SB_MAKEMASK1(60)

#define M_SYS_MISR_MODE             _SB_MAKEMASK1(61)
#define M_SYS_MISR_RESET            _SB_MAKEMASK1(62)

/*
 * Mailbox Registers (Table 4-3)
 * Registers: SCD_MBOX_CPU_x
 */

#define S_MBOX_INT_3                0
#define M_MBOX_INT_3                _SB_MAKEMASK(16,S_MBOX_INT_3)
#define S_MBOX_INT_2                16
#define M_MBOX_INT_2                _SB_MAKEMASK(16,S_MBOX_INT_2)
#define S_MBOX_INT_1                32
#define M_MBOX_INT_1                _SB_MAKEMASK(16,S_MBOX_INT_1)
#define S_MBOX_INT_0                48
#define M_MBOX_INT_0                _SB_MAKEMASK(16,S_MBOX_INT_0)

/*
 * Watchdog Registers (Table 4-8) (Table 4-9) (Table 4-10)
 * Registers: SCD_WDOG_INIT_CNT_x
 */

#define V_SCD_WDOG_FREQ             1000000

#define S_SCD_WDOG_INIT             0
#define M_SCD_WDOG_INIT             _SB_MAKEMASK(13,S_SCD_WDOG_INIT)

#define S_SCD_WDOG_CNT              0
#define M_SCD_WDOG_CNT              _SB_MAKEMASK(13,S_SCD_WDOG_CNT)

#define M_SCD_WDOG_ENABLE           _SB_MAKEMASK1(0)

/*
 * Timer Registers (Table 4-11) (Table 4-12) (Table 4-13)
 */

#define V_SCD_TIMER_FREQ            1000000

#define S_SCD_TIMER_INIT            0
#define M_SCD_TIMER_INIT            _SB_MAKEMASK(20,S_SCD_TIMER_INIT)
#define V_SCD_TIMER_INIT(x)         _SB_MAKEVALUE(x,S_SCD_TIMER_INIT)
#define G_SCD_TIMER_INIT(x)         _SB_GETVALUE(x,S_SCD_TIMER_INIT,M_SCD_TIMER_INIT)

#define S_SCD_TIMER_CNT             0
#define M_SCD_TIMER_CNT             _SB_MAKEMASK(20,S_SCD_TIMER_CNT)
#define V_SCD_TIMER_CNT(x)         _SB_MAKEVALUE(x,S_SCD_TIMER_CNT)
#define G_SCD_TIMER_CNT(x)         _SB_GETVALUE(x,S_SCD_TIMER_CNT,M_SCD_TIMER_CNT)

#define M_SCD_TIMER_ENABLE          _SB_MAKEMASK1(0)
#define M_SCD_TIMER_MODE            _SB_MAKEMASK1(1)
#define M_SCD_TIMER_MODE_CONTINUOUS M_SCD_TIMER_MODE

/*
 * System Performance Counters
 */

#define S_SPC_CFG_SRC0            0
#define M_SPC_CFG_SRC0            _SB_MAKEMASK(8,S_SPC_CFG_SRC0)
#define V_SPC_CFG_SRC0(x)         _SB_MAKEVALUE(x,S_SPC_CFG_SRC0)
#define G_SPC_CFG_SRC0(x)         _SB_GETVALUE(x,S_SPC_CFG_SRC0,M_SPC_CFG_SRC0)

#define S_SPC_CFG_SRC1            8
#define M_SPC_CFG_SRC1            _SB_MAKEMASK(8,S_SPC_CFG_SRC1)
#define V_SPC_CFG_SRC1(x)         _SB_MAKEVALUE(x,S_SPC_CFG_SRC1)
#define G_SPC_CFG_SRC1(x)         _SB_GETVALUE(x,S_SPC_CFG_SRC1,M_SPC_CFG_SRC1)

#define S_SPC_CFG_SRC2            16
#define M_SPC_CFG_SRC2            _SB_MAKEMASK(8,S_SPC_CFG_SRC2)
#define V_SPC_CFG_SRC2(x)         _SB_MAKEVALUE(x,S_SPC_CFG_SRC2)
#define G_SPC_CFG_SRC2(x)         _SB_GETVALUE(x,S_SPC_CFG_SRC2,M_SPC_CFG_SRC2)

#define S_SPC_CFG_SRC3            24
#define M_SPC_CFG_SRC3            _SB_MAKEMASK(8,S_SPC_CFG_SRC3)
#define V_SPC_CFG_SRC3(x)         _SB_MAKEVALUE(x,S_SPC_CFG_SRC3)
#define G_SPC_CFG_SRC3(x)         _SB_GETVALUE(x,S_SPC_CFG_SRC3,M_SPC_CFG_SRC3)

#define M_SPC_CFG_CLEAR		_SB_MAKEMASK1(32)
#define M_SPC_CFG_ENABLE	_SB_MAKEMASK1(33)


/*
 * Bus Watcher
 */

#define S_SCD_BERR_TID            8
#define M_SCD_BERR_TID            _SB_MAKEMASK(10,S_SCD_BERR_TID)
#define V_SCD_BERR_TID(x)         _SB_MAKEVALUE(x,S_SCD_BERR_TID)
#define G_SCD_BERR_TID(x)         _SB_GETVALUE(x,S_SCD_BERR_TID,M_SCD_BERR_TID)

#define S_SCD_BERR_RID            18
#define M_SCD_BERR_RID            _SB_MAKEMASK(4,S_SCD_BERR_RID)
#define V_SCD_BERR_RID(x)         _SB_MAKEVALUE(x,S_SCD_BERR_RID)
#define G_SCD_BERR_RID(x)         _SB_GETVALUE(x,S_SCD_BERR_RID,M_SCD_BERR_RID)

#define S_SCD_BERR_DCODE            22
#define M_SCD_BERR_DCODE            _SB_MAKEMASK(3,S_SCD_BERR_DCODE)
#define V_SCD_BERR_DCODE(x)         _SB_MAKEVALUE(x,S_SCD_BERR_DCODE)
#define G_SCD_BERR_DCODE(x)         _SB_GETVALUE(x,S_SCD_BERR_DCODE,M_SCD_BERR_DCODE)

#define M_SCD_BERR_MULTERRS	_SB_MAKEMASK1(30)


#define S_SCD_L2ECC_CORR_D            0
#define M_SCD_L2ECC_CORR_D            _SB_MAKEMASK(8,S_SCD_L2ECC_CORR_D)
#define V_SCD_L2ECC_CORR_D(x)         _SB_MAKEVALUE(x,S_SCD_L2ECC_CORR_D)
#define G_SCD_L2ECC_CORR_D(x)         _SB_GETVALUE(x,S_SCD_L2ECC_CORR_D,M_SCD_L2ECC_CORR_D)

#define S_SCD_L2ECC_BAD_D            8
#define M_SCD_L2ECC_BAD_D            _SB_MAKEMASK(8,S_SCD_L2ECC_BAD_D)
#define V_SCD_L2ECC_BAD_D(x)         _SB_MAKEVALUE(x,S_SCD_L2ECC_BAD_D)
#define G_SCD_L2ECC_BAD_D(x)         _SB_GETVALUE(x,S_SCD_L2ECC_BAD_D,M_SCD_L2ECC_BAD_D)

#define S_SCD_L2ECC_CORR_T            16
#define M_SCD_L2ECC_CORR_T            _SB_MAKEMASK(8,S_SCD_L2ECC_CORR_T)
#define V_SCD_L2ECC_CORR_T(x)         _SB_MAKEVALUE(x,S_SCD_L2ECC_CORR_T)
#define G_SCD_L2ECC_CORR_T(x)         _SB_GETVALUE(x,S_SCD_L2ECC_CORR_T,M_SCD_L2ECC_CORR_T)

#define S_SCD_L2ECC_BAD_T            24
#define M_SCD_L2ECC_BAD_T            _SB_MAKEMASK(8,S_SCD_L2ECC_BAD_T)
#define V_SCD_L2ECC_BAD_T(x)         _SB_MAKEVALUE(x,S_SCD_L2ECC_BAD_T)
#define G_SCD_L2ECC_BAD_T(x)         _SB_GETVALUE(x,S_SCD_L2ECC_BAD_T,M_SCD_L2ECC_BAD_T)

#define S_SCD_MEM_ECC_CORR            0
#define M_SCD_MEM_ECC_CORR            _SB_MAKEMASK(8,S_SCD_MEM_ECC_CORR)
#define V_SCD_MEM_ECC_CORR(x)         _SB_MAKEVALUE(x,S_SCD_MEM_ECC_CORR)
#define G_SCD_MEM_ECC_CORR(x)         _SB_GETVALUE(x,S_SCD_MEM_ECC_CORR,M_SCD_MEM_ECC_CORR)

#define S_SCD_MEM_ECC_BAD            16
#define M_SCD_MEM_ECC_BAD            _SB_MAKEMASK(8,S_SCD_MEM_ECC_BAD)
#define V_SCD_MEM_ECC_BAD(x)         _SB_MAKEVALUE(x,S_SCD_MEM_ECC_BAD)
#define G_SCD_MEM_ECC_BAD(x)         _SB_GETVALUE(x,S_SCD_MEM_ECC_BAD,M_SCD_MEM_ECC_BAD)

#define S_SCD_MEM_BUSERR            24
#define M_SCD_MEM_BUSERR            _SB_MAKEMASK(8,S_SCD_MEM_BUSERR)
#define V_SCD_MEM_BUSERR(x)         _SB_MAKEVALUE(x,S_SCD_MEM_BUSERR)
#define G_SCD_MEM_BUSERR(x)         _SB_GETVALUE(x,S_SCD_MEM_BUSERR,M_SCD_MEM_BUSERR)


/*
 * Address Trap Registers
 */

#define M_ATRAP_INDEX		  _SB_MAKEMASK(4,0)
#define M_ATRAP_ADDRESS		  _SB_MAKEMASK(40,0)

#define S_ATRAP_CFG_CNT            0
#define M_ATRAP_CFG_CNT            _SB_MAKEMASK(3,S_ATRAP_CFG_CNT)
#define V_ATRAP_CFG_CNT(x)         _SB_MAKEVALUE(x,S_ATRAP_CFG_CNT)
#define G_ATRAP_CFG_CNT(x)         _SB_GETVALUE(x,S_ATRAP_CFG_CNT,M_ATRAP_CFG_CNT)

#define M_ATRAP_CFG_WRITE	   _SB_MAKEMASK1(3)
#define M_ATRAP_CFG_ALL	  	   _SB_MAKEMASK1(4)
#define M_ATRAP_CFG_INV	   	   _SB_MAKEMASK1(5)
#define M_ATRAP_CFG_USESRC	   _SB_MAKEMASK1(6)
#define M_ATRAP_CFG_SRCINV	   _SB_MAKEMASK1(7)

#define S_ATRAP_CFG_AGENTID     8
#define M_ATRAP_CFG_AGENTID     _SB_MAKEMASK(4,S_ATRAP_CFG_AGENTID)
#define V_ATRAP_CFG_AGENTID(x)  _SB_MAKEVALUE(x,S_ATRAP_CFG_AGENTID)
#define G_ATRAP_CFG_AGENTID(x)  _SB_GETVALUE(x,S_ATRAP_CFG_AGENTID,M_ATRAP_CFG_AGENTID)

#define K_BUS_AGENT_CPU0	0
#define K_BUS_AGENT_CPU1	1
#define K_BUS_AGENT_IOB0	2
#define K_BUS_AGENT_IOB1	3
#define K_BUS_AGENT_SCD	4
#define K_BUS_AGENT_RESERVED	5
#define K_BUS_AGENT_L2C	6
#define K_BUS_AGENT_MC	7

#define S_ATRAP_CFG_CATTR     12
#define M_ATRAP_CFG_CATTR     _SB_MAKEMASK(3,S_ATRAP_CFG_CATTR)
#define V_ATRAP_CFG_CATTR(x)  _SB_MAKEVALUE(x,S_ATRAP_CFG_CATTR)
#define G_ATRAP_CFG_CATTR(x)  _SB_GETVALUE(x,S_ATRAP_CFG_CATTR,M_ATRAP_CFG_CATTR)

#define K_ATRAP_CFG_CATTR_IGNORE	0
#define K_ATRAP_CFG_CATTR_UNC    	1
#define K_ATRAP_CFG_CATTR_CACHEABLE	2
#define K_ATRAP_CFG_CATTR_NONCOH  	3
#define K_ATRAP_CFG_CATTR_COHERENT	4
#define K_ATRAP_CFG_CATTR_NOTUNC	5
#define K_ATRAP_CFG_CATTR_NOTNONCOH	6
#define K_ATRAP_CFG_CATTR_NOTCOHERENT   7

/*
 * Trace Buffer Config register
 */

#define M_SCD_TRACE_CFG_RESET           _SB_MAKEMASK1(0)
#define M_SCD_TRACE_CFG_START_READ      _SB_MAKEMASK1(1)
#define M_SCD_TRACE_CFG_START           _SB_MAKEMASK1(2)
#define M_SCD_TRACE_CFG_STOP            _SB_MAKEMASK1(3)
#define M_SCD_TRACE_CFG_FREEZE          _SB_MAKEMASK1(4)
#define M_SCD_TRACE_CFG_FREEZE_FULL     _SB_MAKEMASK1(5)
#define M_SCD_TRACE_CFG_DEBUG_FULL      _SB_MAKEMASK1(6)
#define M_SCD_TRACE_CFG_FULL            _SB_MAKEMASK1(7)

#define S_SCD_TRACE_CFG_CUR_ADDR        10
#define M_SCD_TRACE_CFG_CUR_ADDR        _SB_MAKEMASK(8,S_SCD_TRACE_CFG_CUR_ADDR)
#define V_SCD_TRACE_CFG_CUR_ADDR(x)     _SB_MAKEVALUE(x,S_SCD_TRACE_CFG_CUR_ADDR)
#define G_SCD_TRACE_CFG_CUR_ADDR(x)     _SB_GETVALUE(x,S_SCD_TRACE_CFG_CUR_ADDR,M_SCD_TRACE_CFG_CUR_ADDR)

/*
 * Trace Event registers
 */

#define S_SCD_TREVT_ADDR_MATCH          0
#define M_SCD_TREVT_ADDR_MATCH          _SB_MAKEMASK(4,S_SCD_TREVT_ADDR_MATCH)
#define V_SCD_TREVT_ADDR_MATCH(x)       _SB_MAKEVALUE(x,S_SCD_TREVT_ADDR_MATCH)
#define G_SCD_TREVT_ADDR_MATCH(x)       _SB_GETVALUE(x,S_SCD_TREVT_ADDR_MATCH,M_SCD_TREVT_ADDR_MATCH)

#define M_SCD_TREVT_REQID_MATCH         _SB_MAKEMASK1(4)
#define M_SCD_TREVT_DATAID_MATCH        _SB_MAKEMASK1(5)
#define M_SCD_TREVT_RESPID_MATCH        _SB_MAKEMASK1(6)
#define M_SCD_TREVT_INTERRUPT           _SB_MAKEMASK1(7)
#define M_SCD_TREVT_DEBUG_PIN           _SB_MAKEMASK1(9)
#define M_SCD_TREVT_WRITE               _SB_MAKEMASK1(10)
#define M_SCD_TREVT_READ                _SB_MAKEMASK1(11)

#define S_SCD_TREVT_REQID               12
#define M_SCD_TREVT_REQID               _SB_MAKEMASK(4,S_SCD_TREVT_REQID)
#define V_SCD_TREVT_REQID(x)            _SB_MAKEVALUE(x,S_SCD_TREVT_REQID)
#define G_SCD_TREVT_REQID(x)            _SB_GETVALUE(x,S_SCD_TREVT_REQID,M_SCD_TREVT_REQID)

#define S_SCD_TREVT_RESPID              16
#define M_SCD_TREVT_RESPID              _SB_MAKEMASK(4,S_SCD_TREVT_RESPID)
#define V_SCD_TREVT_RESPID(x)           _SB_MAKEVALUE(x,S_SCD_TREVT_RESPID)
#define G_SCD_TREVT_RESPID(x)           _SB_GETVALUE(x,S_SCD_TREVT_RESPID,M_SCD_TREVT_RESPID)

#define S_SCD_TREVT_DATAID              20
#define M_SCD_TREVT_DATAID              _SB_MAKEMASK(4,S_SCD_TREVT_DATAID)
#define V_SCD_TREVT_DATAID(x)           _SB_MAKEVALUE(x,S_SCD_TREVT_DATAID)
#define G_SCD_TREVT_DATAID(x)           _SB_GETVALUE(x,S_SCD_TREVT_DATAID,M_SCD_TREVT_DATID)

#define S_SCD_TREVT_COUNT               24
#define M_SCD_TREVT_COUNT               _SB_MAKEMASK(8,S_SCD_TREVT_COUNT)
#define V_SCD_TREVT_COUNT(x)            _SB_MAKEVALUE(x,S_SCD_TREVT_COUNT)
#define G_SCD_TREVT_COUNT(x)            _SB_GETVALUE(x,S_SCD_TREVT_COUNT,M_SCD_TREVT_COUNT)

/*
 * Trace Sequence registers
 */

#define S_SCD_TRSEQ_EVENT4              0
#define M_SCD_TRSEQ_EVENT4              _SB_MAKEMASK(4,S_SCD_TRSEQ_EVENT4)
#define V_SCD_TRSEQ_EVENT4(x)           _SB_MAKEVALUE(x,S_SCD_TRSEQ_EVENT4)
#define G_SCD_TRSEQ_EVENT4(x)           _SB_GETVALUE(x,S_SCD_TRSEQ_EVENT4,M_SCD_TRSEQ_EVENT4)

#define S_SCD_TRSEQ_EVENT3              4
#define M_SCD_TRSEQ_EVENT3              _SB_MAKEMASK(4,S_SCD_TRSEQ_EVENT3)
#define V_SCD_TRSEQ_EVENT3(x)           _SB_MAKEVALUE(x,S_SCD_TRSEQ_EVENT3)
#define G_SCD_TRSEQ_EVENT3(x)           _SB_GETVALUE(x,S_SCD_TRSEQ_EVENT3,M_SCD_TRSEQ_EVENT3)

#define S_SCD_TRSEQ_EVENT2              8
#define M_SCD_TRSEQ_EVENT2              _SB_MAKEMASK(4,S_SCD_TRSEQ_EVENT2)
#define V_SCD_TRSEQ_EVENT2(x)           _SB_MAKEVALUE(x,S_SCD_TRSEQ_EVENT2)
#define G_SCD_TRSEQ_EVENT2(x)           _SB_GETVALUE(x,S_SCD_TRSEQ_EVENT2,M_SCD_TRSEQ_EVENT2)

#define S_SCD_TRSEQ_EVENT1              12
#define M_SCD_TRSEQ_EVENT1              _SB_MAKEMASK(4,S_SCD_TRSEQ_EVENT1)
#define V_SCD_TRSEQ_EVENT1(x)           _SB_MAKEVALUE(x,S_SCD_TRSEQ_EVENT1)
#define G_SCD_TRSEQ_EVENT1(x)           _SB_GETVALUE(x,S_SCD_TRSEQ_EVENT1,M_SCD_TRSEQ_EVENT1)

#define K_SCD_TRSEQ_E0                  0
#define K_SCD_TRSEQ_E1                  1
#define K_SCD_TRSEQ_E2                  2
#define K_SCD_TRSEQ_E3                  3
#define K_SCD_TRSEQ_E0_E1               4
#define K_SCD_TRSEQ_E1_E2               5
#define K_SCD_TRSEQ_E2_E3               6
#define K_SCD_TRSEQ_E0_E1_E2            7
#define K_SCD_TRSEQ_E0_E1_E2_E3         8
#define K_SCD_TRSEQ_E0E1                9
#define K_SCD_TRSEQ_E0E1E2              10
#define K_SCD_TRSEQ_E0E1E2E3            11
#define K_SCD_TRSEQ_E0E1_E2             12
#define K_SCD_TRSEQ_E0E1_E2E3           13
#define K_SCD_TRSEQ_E0E1_E2_E3          14
#define K_SCD_TRSEQ_IGNORED             15

#define K_SCD_TRSEQ_TRIGGER_ALL         (V_SCD_TRSEQ_EVENT1(K_SCD_TRSEQ_IGNORED) | \
                                         V_SCD_TRSEQ_EVENT2(K_SCD_TRSEQ_IGNORED) | \
                                         V_SCD_TRSEQ_EVENT3(K_SCD_TRSEQ_IGNORED) | \
                                         V_SCD_TRSEQ_EVENT4(K_SCD_TRSEQ_IGNORED))

#define S_SCD_TRSEQ_FUNCTION            16
#define M_SCD_TRSEQ_FUNCTION            _SB_MAKEMASK(4,S_SCD_TRSEQ_FUNCTION)
#define V_SCD_TRSEQ_FUNCTION(x)         _SB_MAKEVALUE(x,S_SCD_TRSEQ_FUNCTION)
#define G_SCD_TRSEQ_FUNCTION(x)         _SB_GETVALUE(x,S_SCD_TRSEQ_FUNCTION,M_SCD_TRSEQ_FUNCTION)

#define K_SCD_TRSEQ_FUNC_NOP            0
#define K_SCD_TRSEQ_FUNC_START          1
#define K_SCD_TRSEQ_FUNC_STOP           2
#define K_SCD_TRSEQ_FUNC_FREEZE         3

#define V_SCD_TRSEQ_FUNC_NOP            V_SCD_TRSEQ_FUNCTION(K_SCD_TRSEQ_FUNC_NOP)
#define V_SCD_TRSEQ_FUNC_START          V_SCD_TRSEQ_FUNCTION(K_SCD_TRSEQ_FUNC_START)
#define V_SCD_TRSEQ_FUNC_STOP           V_SCD_TRSEQ_FUNCTION(K_SCD_TRSEQ_FUNC_STOP)
#define V_SCD_TRSEQ_FUNC_FREEZE         V_SCD_TRSEQ_FUNCTION(K_SCD_TRSEQ_FUNC_FREEZE)

#define M_SCD_TRSEQ_ASAMPLE             _SB_MAKEMASK1(18)
#define M_SCD_TRSEQ_DSAMPLE             _SB_MAKEMASK1(19)
#define M_SCD_TRSEQ_DEBUGPIN            _SB_MAKEMASK1(20)
#define M_SCD_TRSEQ_DEBUGCPU            _SB_MAKEMASK1(21)
#define M_SCD_TRSEQ_CLEARUSE            _SB_MAKEMASK1(22)


/*  *********************************************************************
    *  Interrupt Mapper Constants
    ********************************************************************* */

/*
 * Interrupt sources (Table 4-8, UM 0.2)
 * 
 * First, the interrupt numbers.
 */

#define K_INT_WATCHDOG_TIMER_0      0
#define K_INT_WATCHDOG_TIMER_1      1
#define K_INT_TIMER_0               2
#define K_INT_TIMER_1               3
#define K_INT_TIMER_2               4
#define K_INT_TIMER_3               5
#define K_INT_SMB_0                 6
#define K_INT_SMB_1                 7
#define K_INT_UART_0                8
#define K_INT_UART_1                9
#define K_INT_SER_0                 10
#define K_INT_SER_1                 11
#define K_INT_PCMCIA                12
#define K_INT_ADDR_TRAP             13
#define K_INT_PERF_CNT              14
#define K_INT_TRACE_FREEZE          15
#define K_INT_BAD_ECC               16
#define K_INT_COR_ECC               17
#define K_INT_IO_BUS                18
#define K_INT_MAC_0                 19
#define K_INT_MAC_1                 20
#define K_INT_MAC_2                 21
#define K_INT_DM_CH_0               22
#define K_INT_DM_CH_1               23
#define K_INT_DM_CH_2               24
#define K_INT_DM_CH_3               25
#define K_INT_MBOX_0                26
#define K_INT_MBOX_1                27
#define K_INT_MBOX_2                28
#define K_INT_MBOX_3                29
#define K_INT_SPARE_0               30
#define K_INT_SPARE_1               31
#define K_INT_GPIO_0                32
#define K_INT_GPIO_1                33
#define K_INT_GPIO_2                34
#define K_INT_GPIO_3                35
#define K_INT_GPIO_4                36
#define K_INT_GPIO_5                37
#define K_INT_GPIO_6                38
#define K_INT_GPIO_7                39
#define K_INT_GPIO_8                40
#define K_INT_GPIO_9                41
#define K_INT_GPIO_10               42
#define K_INT_GPIO_11               43
#define K_INT_GPIO_12               44
#define K_INT_GPIO_13               45
#define K_INT_GPIO_14               46
#define K_INT_GPIO_15               47
#define K_INT_LDT_FATAL             48
#define K_INT_LDT_NONFATAL          49
#define K_INT_LDT_SMI               50
#define K_INT_LDT_NMI               51
#define K_INT_LDT_INIT              52
#define K_INT_LDT_STARTUP           53
#define K_INT_LDT_EXT               54
#define K_INT_PCI_ERROR             55
#define K_INT_PCI_INTA              56
#define K_INT_PCI_INTB              57
#define K_INT_PCI_INTC              58
#define K_INT_PCI_INTD              59
#define K_INT_SPARE_2               60
#define K_INT_SPARE_3               61
#define K_INT_SPARE_4               62
#define K_INT_SPARE_5               63

/*
 * Mask values for each interrupt
 */

#define M_INT_WATCHDOG_TIMER_0      _SB_MAKEMASK1(K_INT_WATCHDOG_TIMER_0)
#define M_INT_WATCHDOG_TIMER_1      _SB_MAKEMASK1(K_INT_WATCHDOG_TIMER_1)
#define M_INT_TIMER_0               _SB_MAKEMASK1(K_INT_TIMER_0)
#define M_INT_TIMER_1               _SB_MAKEMASK1(K_INT_TIMER_1)
#define M_INT_TIMER_2               _SB_MAKEMASK1(K_INT_TIMER_2)
#define M_INT_TIMER_3               _SB_MAKEMASK1(K_INT_TIMER_3)
#define M_INT_SMB_0                 _SB_MAKEMASK1(K_INT_SMB_0)
#define M_INT_SMB_1                 _SB_MAKEMASK1(K_INT_SMB_1)
#define M_INT_UART_0                _SB_MAKEMASK1(K_INT_UART_0)
#define M_INT_UART_1                _SB_MAKEMASK1(K_INT_UART_1)
#define M_INT_SER_0                 _SB_MAKEMASK1(K_INT_SER_0)
#define M_INT_SER_1                 _SB_MAKEMASK1(K_INT_SER_1)
#define M_INT_PCMCIA                _SB_MAKEMASK1(K_INT_PCMCIA)
#define M_INT_ADDR_TRAP             _SB_MAKEMASK1(K_INT_ADDR_TRAP)
#define M_INT_PERF_CNT              _SB_MAKEMASK1(K_INT_PERF_CNT)
#define M_INT_TRACE_FREEZE          _SB_MAKEMASK1(K_INT_TRACE_FREEZE)
#define M_INT_BAD_ECC               _SB_MAKEMASK1(K_INT_BAD_ECC)
#define M_INT_COR_ECC               _SB_MAKEMASK1(K_INT_COR_ECC)
#define M_INT_IO_BUS                _SB_MAKEMASK1(K_INT_IO_BUS)
#define M_INT_MAC_0                 _SB_MAKEMASK1(K_INT_MAC_0)
#define M_INT_MAC_1                 _SB_MAKEMASK1(K_INT_MAC_1)
#define M_INT_MAC_2                 _SB_MAKEMASK1(K_INT_MAC_2)
#define M_INT_DM_CH_0               _SB_MAKEMASK1(K_INT_DM_CH_0)
#define M_INT_DM_CH_1               _SB_MAKEMASK1(K_INT_DM_CH_1)
#define M_INT_DM_CH_2               _SB_MAKEMASK1(K_INT_DM_CH_2)
#define M_INT_DM_CH_3               _SB_MAKEMASK1(K_INT_DM_CH_3)
#define M_INT_MBOX_0                _SB_MAKEMASK1(K_INT_MBOX_0)
#define M_INT_MBOX_1                _SB_MAKEMASK1(K_INT_MBOX_1)
#define M_INT_MBOX_2                _SB_MAKEMASK1(K_INT_MBOX_2)
#define M_INT_MBOX_3                _SB_MAKEMASK1(K_INT_MBOX_3)
#define M_INT_SPARE_0               _SB_MAKEMASK1(K_INT_SPARE_0)
#define M_INT_SPARE_1               _SB_MAKEMASK1(K_INT_SPARE_1)
#define M_INT_GPIO_0                _SB_MAKEMASK1(K_INT_GPIO_0)
#define M_INT_GPIO_1                _SB_MAKEMASK1(K_INT_GPIO_1)
#define M_INT_GPIO_2                _SB_MAKEMASK1(K_INT_GPIO_2)
#define M_INT_GPIO_3                _SB_MAKEMASK1(K_INT_GPIO_3)
#define M_INT_GPIO_4                _SB_MAKEMASK1(K_INT_GPIO_4)
#define M_INT_GPIO_5                _SB_MAKEMASK1(K_INT_GPIO_5)
#define M_INT_GPIO_6                _SB_MAKEMASK1(K_INT_GPIO_6)
#define M_INT_GPIO_7                _SB_MAKEMASK1(K_INT_GPIO_7)
#define M_INT_GPIO_8                _SB_MAKEMASK1(K_INT_GPIO_8)
#define M_INT_GPIO_9                _SB_MAKEMASK1(K_INT_GPIO_9)
#define M_INT_GPIO_10               _SB_MAKEMASK1(K_INT_GPIO_10)
#define M_INT_GPIO_11               _SB_MAKEMASK1(K_INT_GPIO_11)
#define M_INT_GPIO_12               _SB_MAKEMASK1(K_INT_GPIO_12)
#define M_INT_GPIO_13               _SB_MAKEMASK1(K_INT_GPIO_13)
#define M_INT_GPIO_14               _SB_MAKEMASK1(K_INT_GPIO_14)
#define M_INT_GPIO_15               _SB_MAKEMASK1(K_INT_GPIO_15)
#define M_INT_LDT_FATAL             _SB_MAKEMASK1(K_INT_LDT_FATAL)
#define M_INT_LDT_NONFATAL          _SB_MAKEMASK1(K_INT_LDT_NONFATAL)
#define M_INT_LDT_SMI               _SB_MAKEMASK1(K_INT_LDT_SMI)
#define M_INT_LDT_NMI               _SB_MAKEMASK1(K_INT_LDT_NMI)
#define M_INT_LDT_INIT              _SB_MAKEMASK1(K_INT_LDT_INIT)
#define M_INT_LDT_STARTUP           _SB_MAKEMASK1(K_INT_LDT_STARTUP)
#define M_INT_LDT_EXT               _SB_MAKEMASK1(K_INT_LDT_EXT)
#define M_INT_PCI_ERROR             _SB_MAKEMASK1(K_INT_PCI_ERROR)
#define M_INT_PCI_INTA              _SB_MAKEMASK1(K_INT_PCI_INTA)
#define M_INT_PCI_INTB              _SB_MAKEMASK1(K_INT_PCI_INTB)
#define M_INT_PCI_INTC              _SB_MAKEMASK1(K_INT_PCI_INTC)
#define M_INT_PCI_INTD              _SB_MAKEMASK1(K_INT_PCI_INTD)
#define M_INT_SPARE_2               _SB_MAKEMASK1(K_INT_SPARE_2)
#define M_INT_SPARE_3               _SB_MAKEMASK1(K_INT_SPARE_3)
#define M_INT_SPARE_4               _SB_MAKEMASK1(K_INT_SPARE_4)
#define M_INT_SPARE_5               _SB_MAKEMASK1(K_INT_SPARE_5)

/*
 * Interrupt mappings
 */

#define K_INT_MAP_I0	0		/* interrupt pins on processor */
#define K_INT_MAP_I1	1
#define K_INT_MAP_I2	2
#define K_INT_MAP_I3	3
#define K_INT_MAP_I4	4
#define K_INT_MAP_I5	5
#define K_INT_MAP_NMI	6		/* nonmaskable */
#define K_INT_MAP_DINT	7		/* debug interrupt */

/*
 * LDT Interrupt Set Register (table 4-5)
 */

#define S_INT_LDT_INTMSG	      0
#define M_INT_LDT_INTMSG              _SB_MAKEMASK(3,S_INT_LDT_INTMSG)
#define V_INT_LDT_INTMSG(x)           _SB_MAKEVALUE(x,S_INT_LDT_INTMSG)
#define G_INT_LDT_INTMSG(x)           _SB_GETVALUE(x,S_INT_LDT_INTMSG,M_INT_LDT_INTMSG)

#define K_INT_LDT_INTMSG_FIXED	      0
#define K_INT_LDT_INTMSG_ARBITRATED   1
#define K_INT_LDT_INTMSG_SMI	      2
#define K_INT_LDT_INTMSG_NMI	      3
#define K_INT_LDT_INTMSG_INIT	      4
#define K_INT_LDT_INTMSG_STARTUP      5
#define K_INT_LDT_INTMSG_EXTINT	      6
#define K_INT_LDT_INTMSG_RESERVED     7

#define M_INT_LDT_EDGETRIGGER         0
#define M_INT_LDT_LEVELTRIGGER        _SB_MAKEMASK1(3)

#define M_INT_LDT_PHYSICALDEST        0
#define M_INT_LDT_LOGICALDEST         _SB_MAKEMASK1(4)

#define S_INT_LDT_INTDEST             5
#define M_INT_LDT_INTDEST             _SB_MAKEMASK(10,S_INT_LDT_INTDEST)
#define V_INT_LDT_INTDEST(x)          _SB_MAKEVALUE(x,S_INT_LDT_INTDEST)
#define G_INT_LDT_INTDEST(x)          _SB_GETVALUE(x,S_INT_LDT_INTDEST,M_INT_LDT_INTDEST)

#define S_INT_LDT_VECTOR              13
#define M_INT_LDT_VECTOR              _SB_MAKEMASK(8,S_INT_LDT_VECTOR)
#define V_INT_LDT_VECTOR(x)           _SB_MAKEVALUE(x,S_INT_LDT_VECTOR)
#define G_INT_LDT_VECTOR(x)           _SB_GETVALUE(x,S_INT_LDT_VECTOR,M_INT_LDT_VECTOR)

/*
 * Vector format (Table 4-6)
 */

#define M_LDTVECT_RAISEINT		0x00
#define M_LDTVECT_RAISEMBOX             0x40


/*  *********************************************************************
    *  Level 2 Cache constants
    ********************************************************************* */

/*
 * Level 2 Cache Tag register (Table 5-3)
 */

#define S_L2C_TAG_MBZ               0
#define M_L2C_TAG_MBZ               _SB_MAKEMASK(5,S_L2C_TAG_MBZ)

#define S_L2C_TAG_INDEX             5
#define M_L2C_TAG_INDEX             _SB_MAKEMASK(12,S_L2C_TAG_INDEX)
#define V_L2C_TAG_INDEX(x)          _SB_MAKEVALUE(x,S_L2C_TAG_INDEX)
#define G_L2C_TAG_INDEX(x)          _SB_GETVALUE(x,S_L2C_TAG_INDEX,M_L2C_TAG_INDEX)

#define S_L2C_TAG_TAG               17
#define M_L2C_TAG_TAG               _SB_MAKEMASK(23,S_L2C_TAG_TAG)
#define V_L2C_TAG_TAG(x)            _SB_MAKEVALUE(x,S_L2C_TAG_TAG)
#define G_L2C_TAG_TAG(x)            _SB_GETVALUE(x,S_L2C_TAG_TAG,M_L2C_TAG_TAG)

#define S_L2C_TAG_ECC               40
#define M_L2C_TAG_ECC               _SB_MAKEMASK(6,S_L2C_TAG_ECC)
#define V_L2C_TAG_ECC(x)            _SB_MAKEVALUE(x,S_L2C_TAG_ECC)
#define G_L2C_TAG_ECC(x)            _SB_GETVALUE(x,S_L2C_TAG_ECC,M_L2C_TAG_ECC)

#define S_L2C_TAG_WAY               46
#define M_L2C_TAG_WAY               _SB_MAKEMASK(2,S_L2C_TAG_WAY)
#define V_L2C_TAG_WAY(x)            _SB_MAKEVALUE(x,S_L2C_TAG_WAY)
#define G_L2C_TAG_WAY(x)            _SB_GETVALUE(x,S_L2C_TAG_WAY,M_L2C_TAG_WAY)

#define M_L2C_TAG_DIRTY             _SB_MAKEMASK1(48)
#define M_L2C_TAG_VALID             _SB_MAKEMASK1(49)

/*
 * Format of level 2 cache management address (table 5-2)
 */

#define S_L2C_MGMT_INDEX            5
#define M_L2C_MGMT_INDEX            _SB_MAKEMASK(12,S_L2C_MGMT_INDEX)
#define V_L2C_MGMT_INDEX(x)         _SB_MAKEVALUE(x,S_L2C_MGMT_INDEX)
#define G_L2C_MGMT_INDEX(x)         _SB_GETVALUE(x,S_L2C_MGMT_INDEX,M_L2C_MGMT_INDEX)

#define S_L2C_MGMT_WAY              17
#define M_L2C_MGMT_WAY              _SB_MAKEMASK(2,S_L2C_MGMT_WAY)
#define V_L2C_MGMT_WAY(x)           _SB_MAKEVALUE(x,S_L2C_MGMT_WAY)
#define G_L2C_MGMT_WAY(x)           _SB_GETVALUE(x,S_L2C_MGMT_WAY,M_L2C_MGMT_WAY)

#define S_L2C_MGMT_TAG              21
#define M_L2C_MGMT_TAG              _SB_MAKEMASK(6,S_L2C_MGMT_TAG)
#define V_L2C_MGMT_TAG(x)           _SB_MAKEVALUE(x,S_L2C_MGMT_TAG)
#define G_L2C_MGMT_TAG(x)           _SB_GETVALUE(x,S_L2C_MGMT_TAG,M_L2C_MGMT_TAG)

#define M_L2C_MGMT_DIRTY            _SB_MAKEMASK1(19)
#define M_L2C_MGMT_VALID            _SB_MAKEMASK1(20)

#define A_L2C_MGMT_TAG_BASE         0x00D0000000


/*  *********************************************************************
    *  Memory Channel constants
    ********************************************************************* */

/*
 * Memory Channel Config Register (table 6-14)
 */

#define S_MC_RESERVED0              0
#define M_MC_RESERVED0              _SB_MAKEMASK(8,S_MC_RESERVED0)

#define S_MC_CHANNEL_SEL            8
#define M_MC_CHANNEL_SEL            _SB_MAKEMASK(8,S_MC_CHANNEL_SEL)
#define V_MC_CHANNEL_SEL(x)         _SB_MAKEVALUE(x,S_MC_CHANNEL_SEL)
#define G_MC_CHANNEL_SEL(x)         _SB_GETVALUE(x,S_MC_CHANNEL_SEL,M_MC_CHANNEL_SEL)

#define S_MC_BANK0_MAP              16
#define M_MC_BANK0_MAP              _SB_MAKEMASK(4,S_MC_BANK0_MAP)
#define V_MC_BANK0_MAP(x)           _SB_MAKEVALUE(x,S_MC_BANK0_MAP)
#define G_MC_BANK0_MAP(x)           _SB_GETVALUE(x,S_MC_BANK0_MAP,M_MC_BANK0_MAP)

#define K_MC_BANK0_MAP_DEFAULT      0x00
#define V_MC_BANK0_MAP_DEFAULT      V_MC_BANK0_MAP(K_MC_BANK0_MAP_DEFAULT)

#define S_MC_BANK1_MAP              20
#define M_MC_BANK1_MAP              _SB_MAKEMASK(4,S_MC_BANK1_MAP)
#define V_MC_BANK1_MAP(x)           _SB_MAKEVALUE(x,S_MC_BANK1_MAP)
#define G_MC_BANK1_MAP(x)           _SB_GETVALUE(x,S_MC_BANK1_MAP,M_MC_BANK1_MAP)

#define K_MC_BANK1_MAP_DEFAULT      0x08
#define V_MC_BANK1_MAP_DEFAULT      V_MC_BANK1_MAP(K_MC_BANK1_MAP_DEFAULT)

#define S_MC_BANK2_MAP              24
#define M_MC_BANK2_MAP              _SB_MAKEMASK(4,S_MC_BANK2_MAP)
#define V_MC_BANK2_MAP(x)           _SB_MAKEVALUE(x,S_MC_BANK2_MAP)
#define G_MC_BANK2_MAP(x)           _SB_GETVALUE(x,S_MC_BANK2_MAP,M_MC_BANK2_MAP)

#define K_MC_BANK2_MAP_DEFAULT      0x09
#define V_MC_BANK2_MAP_DEFAULT      V_MC_BANK2_MAP(K_MC_BANK2_MAP_DEFAULT)

#define S_MC_BANK3_MAP              28
#define M_MC_BANK3_MAP              _SB_MAKEMASK(4,S_MC_BANK3_MAP)
#define V_MC_BANK3_MAP(x)           _SB_MAKEVALUE(x,S_MC_BANK3_MAP)
#define G_MC_BANK3_MAP(x)           _SB_GETVALUE(x,S_MC_BANK3_MAP,M_MC_BANK3_MAP)

#define K_MC_BANK3_MAP_DEFAULT      0x0C
#define V_MC_BANK3_MAP_DEFAULT      V_MC_BANK3_MAP(K_MC_BANK3_MAP_DEFAULT)

#define M_MC_RESERVED1              _SB_MAKEMASK(8,32)

#define S_MC_QUEUE_SIZE		    40
#define M_MC_QUEUE_SIZE             _SB_MAKEMASK(4,S_MC_QUEUE_SIZE)
#define V_MC_QUEUE_SIZE(x)          _SB_MAKEVALUE(x,S_MC_QUEUE_SIZE)
#define G_MC_QUEUE_SIZE(x)          _SB_GETVALUE(x,S_MC_QUEUE_SIZE,M_MC_QUEUE_SIZE)
#define V_MC_QUEUE_SIZE_DEFAULT     V_MC_QUEUE_SIZE(0x0A)

#define S_MC_AGE_LIMIT              44
#define M_MC_AGE_LIMIT              _SB_MAKEMASK(4,S_MC_AGE_LIMIT)
#define V_MC_AGE_LIMIT(x)           _SB_MAKEVALUE(x,S_MC_AGE_LIMIT)
#define G_MC_AGE_LIMIT(x)           _SB_GETVALUE(x,S_MC_AGE_LIMIT,M_MC_AGE_LIMIT)
#define V_MC_AGE_LIMIT_DEFAULT      V_MC_AGE_LIMIT(8)

#define S_MC_WR_LIMIT               48
#define M_MC_WR_LIMIT               _SB_MAKEMASK(4,S_MC_WR_LIMIT)
#define V_MC_WR_LIMIT(x)            _SB_MAKEVALUE(x,S_MC_WR_LIMIT)
#define G_MC_WR_LIMIT(x)            _SB_GETVALUE(x,S_MC_WR_LIMIT,M_MC_WR_LIMIT)
#define V_MC_WR_LIMIT_DEFAULT       V_MC_WR_LIMIT(5)

#define M_MC_IOB1HIGHPRIORITY	    _SB_MAKEMASK1(52)

#define M_MC_RESERVED2              _SB_MAKEMASK(3,53)

#define S_MC_CS_MODE                56
#define M_MC_CS_MODE                _SB_MAKEMASK(4,S_MC_CS_MODE)
#define V_MC_CS_MODE(x)             _SB_MAKEVALUE(x,S_MC_CS_MODE)
#define G_MC_CS_MODE(x)             _SB_GETVALUE(x,S_MC_CS_MODE,M_MC_CS_MODE)

#define K_MC_CS_MODE_MSB_CS         0
#define K_MC_CS_MODE_INTLV_CS       15
#define K_MC_CS_MODE_MIXED_CS_10    12
#define K_MC_CS_MODE_MIXED_CS_30    6
#define K_MC_CS_MODE_MIXED_CS_32    3

#define V_MC_CS_MODE_MSB_CS         V_MC_CS_MODE(K_MC_CS_MODE_MSB_CS)
#define V_MC_CS_MODE_INTLV_CS       V_MC_CS_MODE(K_MC_CS_MODE_INTLV_CS)
#define V_MC_CS_MODE_MIXED_CS_10    V_MC_CS_MODE(K_MC_CS_MODE_MIXED_CS_10)
#define V_MC_CS_MODE_MIXED_CS_30    V_MC_CS_MODE(K_MC_CS_MODE_MIXED_CS_30)
#define V_MC_CS_MODE_MIXED_CS_32    V_MC_CS_MODE(K_MC_CS_MODE_MIXED_CS_32)

#define M_MC_ECC_DISABLE            _SB_MAKEMASK1(60)
#define M_MC_BERR_DISABLE           _SB_MAKEMASK1(61)
#define M_MC_FORCE_SEQ              _SB_MAKEMASK1(62)
#define M_MC_DEBUG                  _SB_MAKEMASK1(63)

#define V_MC_CONFIG_DEFAULT     V_MC_WR_LIMIT_DEFAULT | V_MC_AGE_LIMIT_DEFAULT | \
				V_MC_BANK0_MAP_DEFAULT | V_MC_BANK1_MAP_DEFAULT | \
				V_MC_BANK2_MAP_DEFAULT | V_MC_BANK3_MAP_DEFAULT | V_MC_CHANNEL_SEL(0) | \
                                M_MC_IOB1HIGHPRIORITY | V_MC_QUEUE_SIZE_DEFAULT


/*
 * Memory clock config register (Table 6-15)
 *
 * Note: this field has been updated to be consistent with the errata to 0.2
 */

#define S_MC_CLK_RATIO              0
#define M_MC_CLK_RATIO              _SB_MAKEMASK(4,S_MC_CLK_RATIO)
#define V_MC_CLK_RATIO(x)           _SB_MAKEVALUE(x,S_MC_CLK_RATIO)
#define G_MC_CLK_RATIO(x)           _SB_GETVALUE(x,S_MC_CLK_RATIO,M_MC_CLK_RATIO)

#define K_MC_CLK_RATIO_2X           4
#define K_MC_CLK_RATIO_25X          5
#define K_MC_CLK_RATIO_3X           6
#define K_MC_CLK_RATIO_35X          7
#define K_MC_CLK_RATIO_4X           8
#define K_MC_CLK_RATIO_45X	    9

#define V_MC_CLK_RATIO_2X	    V_MC_CLK_RATIO(K_MC_CLK_RATIO_2X)
#define V_MC_CLK_RATIO_25X          V_MC_CLK_RATIO(K_MC_CLK_RATIO_25X)
#define V_MC_CLK_RATIO_3X           V_MC_CLK_RATIO(K_MC_CLK_RATIO_3X)
#define V_MC_CLK_RATIO_35X          V_MC_CLK_RATIO(K_MC_CLK_RATIO_35X)
#define V_MC_CLK_RATIO_4X           V_MC_CLK_RATIO(K_MC_CLK_RATIO_4X)
#define V_MC_CLK_RATIO_45X          V_MC_CLK_RATIO(K_MC_CLK_RATIO_45X)
#define V_MC_CLK_RATIO_DEFAULT      V_MC_CLK_RATIO_25X

#define S_MC_REF_RATE                8
#define M_MC_REF_RATE                _SB_MAKEMASK(8,S_MC_REF_RATE)
#define V_MC_REF_RATE(x)             _SB_MAKEVALUE(x,S_MC_REF_RATE)
#define G_MC_REF_RATE(x)             _SB_GETVALUE(x,S_MC_REF_RATE,M_MC_REF_RATE)

#define K_MC_REF_RATE_100MHz         0x62
#define K_MC_REF_RATE_133MHz         0x81
#define K_MC_REF_RATE_200MHz         0xC4 

#define V_MC_REF_RATE_100MHz         V_MC_REF_RATE(K_MC_REF_RATE_100MHz)
#define V_MC_REF_RATE_133MHz         V_MC_REF_RATE(K_MC_REF_RATE_133MHz)
#define V_MC_REF_RATE_200MHz         V_MC_REF_RATE(K_MC_REF_RATE_200MHz)
#define V_MC_REF_RATE_DEFAULT        V_MC_REF_RATE_100MHz

#define S_MC_CLOCK_DRIVE             16
#define M_MC_CLOCK_DRIVE             _SB_MAKEMASK(4,S_MC_CLOCK_DRIVE)
#define V_MC_CLOCK_DRIVE(x)          _SB_MAKEVALUE(x,S_MC_CLOCK_DRIVE)
#define G_MC_CLOCK_DRIVE(x)          _SB_GETVALUE(x,S_MC_CLOCK_DRIVE,M_MC_CLOCK_DRIVE)
#define V_MC_CLOCK_DRIVE_DEFAULT     V_MC_CLOCK_DRIVE(0xF)

#define S_MC_DATA_DRIVE              20
#define M_MC_DATA_DRIVE              _SB_MAKEMASK(4,S_MC_DATA_DRIVE)
#define V_MC_DATA_DRIVE(x)           _SB_MAKEVALUE(x,S_MC_DATA_DRIVE)
#define G_MC_DATA_DRIVE(x)           _SB_GETVALUE(x,S_MC_DATA_DRIVE,M_MC_DATA_DRIVE)
#define V_MC_DATA_DRIVE_DEFAULT      V_MC_DATA_DRIVE(0x0)

#define S_MC_ADDR_DRIVE              24
#define M_MC_ADDR_DRIVE              _SB_MAKEMASK(4,S_MC_ADDR_DRIVE)
#define V_MC_ADDR_DRIVE(x)           _SB_MAKEVALUE(x,S_MC_ADDR_DRIVE)
#define G_MC_ADDR_DRIVE(x)           _SB_GETVALUE(x,S_MC_ADDR_DRIVE,M_MC_ADDR_DRIVE)
#define V_MC_ADDR_DRIVE_DEFAULT      V_MC_ADDR_DRIVE(0x0)

#define M_MC_DLL_BYPASS              _SB_MAKEMASK1(31)

#define S_MC_DQI_SKEW               32
#define M_MC_DQI_SKEW               _SB_MAKEMASK(8,S_MC_DQI_SKEW)
#define V_MC_DQI_SKEW(x)            _SB_MAKEVALUE(x,S_MC_DQI_SKEW)
#define G_MC_DQI_SKEW(x)            _SB_GETVALUE(x,S_MC_DQI_SKEW,M_MC_DQI_SKEW)
#define V_MC_DQI_SKEW_DEFAULT       V_MC_DQI_SKEW(0)

#define S_MC_DQO_SKEW               40
#define M_MC_DQO_SKEW               _SB_MAKEMASK(8,S_MC_DQO_SKEW)
#define V_MC_DQO_SKEW(x)            _SB_MAKEVALUE(x,S_MC_DQO_SKEW)
#define G_MC_DQO_SKEW(x)            _SB_GETVALUE(x,S_MC_DQO_SKEW,M_MC_DQO_SKEW)
#define V_MC_DQO_SKEW_DEFAULT       V_MC_DQO_SKEW(0)

#define S_MC_ADDR_SKEW               48
#define M_MC_ADDR_SKEW               _SB_MAKEMASK(8,S_MC_ADDR_SKEW)
#define V_MC_ADDR_SKEW(x)            _SB_MAKEVALUE(x,S_MC_ADDR_SKEW)
#define G_MC_ADDR_SKEW(x)            _SB_GETVALUE(x,S_MC_ADDR_SKEW,M_MC_ADDR_SKEW)
#define V_MC_ADDR_SKEW_DEFAULT       V_MC_ADDR_SKEW(0x0F)

#define S_MC_DLL_DEFAULT             56
#define M_MC_DLL_DEFAULT             _SB_MAKEMASK(8,S_MC_DLL_DEFAULT)
#define V_MC_DLL_DEFAULT(x)          _SB_MAKEVALUE(x,S_MC_DLL_DEFAULT)
#define G_MC_DLL_DEFAULT(x)          _SB_GETVALUE(x,S_MC_DLL_DEFAULT,M_MC_DLL_DEFAULT)
#define V_MC_DLL_DEFAULT_DEFAULT     V_MC_DLL_DEFAULT(0x10)

#define V_MC_CLKCONFIG_DEFAULT       V_MC_DLL_DEFAULT_DEFAULT |  \
                                     V_MC_ADDR_SKEW_DEFAULT | \
                                     V_MC_DQO_SKEW_DEFAULT | \
                                     V_MC_DQI_SKEW_DEFAULT | \
                                     V_MC_ADDR_DRIVE_DEFAULT | \
                                     V_MC_DATA_DRIVE_DEFAULT | \
                                     V_MC_CLOCK_DRIVE_DEFAULT | \
                                     V_MC_REF_RATE_DEFAULT 



/*
 * DRAM Command Register (Table 6-13)
 */

#define S_MC_COMMAND                0
#define M_MC_COMMAND                _SB_MAKEMASK(4,S_MC_COMMAND)
#define V_MC_COMMAND(x)             _SB_MAKEVALUE(x,S_MC_COMMAND)
#define G_MC_COMMAND(x)             _SB_GETVALUE(x,S_MC_COMMAND,M_MC_COMMAND)

#define K_MC_COMMAND_EMRS           0
#define K_MC_COMMAND_MRS            1
#define K_MC_COMMAND_PRE            2
#define K_MC_COMMAND_AR             3
#define K_MC_COMMAND_SETRFSH        4
#define K_MC_COMMAND_CLRRFSH        5
#define K_MC_COMMAND_SETPWRDN       6
#define K_MC_COMMAND_CLRPWRDN       7

#define V_MC_COMMAND_EMRS           V_MC_COMMAND(K_MC_COMMAND_EMRS)
#define V_MC_COMMAND_MRS            V_MC_COMMAND(K_MC_COMMAND_MRS)
#define V_MC_COMMAND_PRE            V_MC_COMMAND(K_MC_COMMAND_PRE)
#define V_MC_COMMAND_AR             V_MC_COMMAND(K_MC_COMMAND_AR)
#define V_MC_COMMAND_SETRFSH        V_MC_COMMAND(K_MC_COMMAND_SETRFSH)
#define V_MC_COMMAND_CLRRFSH        V_MC_COMMAND(K_MC_COMMAND_CLRRFSH)
#define V_MC_COMMAND_SETPWRDN       V_MC_COMMAND(K_MC_COMMAND_SETPWRDN)
#define V_MC_COMMAND_CLRPWRDN       V_MC_COMMAND(K_MC_COMMAND_CLRPWRDN)

#define M_MC_CS0                    _SB_MAKEMASK1(4)
#define M_MC_CS1                    _SB_MAKEMASK1(5)
#define M_MC_CS2                    _SB_MAKEMASK1(6)
#define M_MC_CS3                    _SB_MAKEMASK1(7)

/*
 * DRAM Mode Register (Table 6-14)
 */

#define S_MC_EMODE                  0
#define M_MC_EMODE                  _SB_MAKEMASK(15,S_MC_EMODE)
#define V_MC_EMODE(x)               _SB_MAKEVALUE(x,S_MC_EMODE)
#define G_MC_EMODE(x)               _SB_GETVALUE(x,S_MC_EMODE,M_MC_EMODE)
#define V_MC_EMODE_DEFAULT          V_MC_EMODE(0)

#define S_MC_MODE                   16
#define M_MC_MODE                   _SB_MAKEMASK(15,S_MC_MODE)
#define V_MC_MODE(x)                _SB_MAKEVALUE(x,S_MC_MODE)
#define G_MC_MODE(x)                _SB_GETVALUE(x,S_MC_MODE,M_MC_MODE)
#define V_MC_MODE_DEFAULT           V_MC_MODE(0x22)

#define S_MC_DRAM_TYPE              32
#define M_MC_DRAM_TYPE              _SB_MAKEMASK(3,S_MC_DRAM_TYPE)
#define V_MC_DRAM_TYPE(x)           _SB_MAKEVALUE(x,S_MC_DRAM_TYPE)
#define G_MC_DRAM_TYPE(x)           _SB_GETVALUE(x,S_MC_DRAM_TYPE,M_MC_DRAM_TYPE)

#define K_MC_DRAM_TYPE_JEDEC        0
#define K_MC_DRAM_TYPE_FCRAM        1
#define K_MC_DRAM_TYPE_SGRAM	    2

#define V_MC_DRAM_TYPE_JEDEC        V_MC_DRAM_TYPE(K_MC_DRAM_TYPE_JEDEC)
#define V_MC_DRAM_TYPE_FCRAM        V_MC_DRAM_TYPE(K_MC_DRAM_TYPE_FCRAM)
#define V_MC_DRAM_TYPE_SGRAM        V_MC_DRAM_TYPE(K_MC_DRAM_TYPE_SGRAM)

#define M_MC_EXTERNALDECODE	    _SB_MAKEMASK1(35)




/*
 * SDRAM Timing Register  (Table 6-15)
 */

#define M_MC_w2rIDLE_TWOCYCLES	  _SB_MAKEMASK1(62)
#define M_MC_r2wIDLE_TWOCYCLES	  _SB_MAKEMASK1(61)
#define M_MC_r2rIDLE_TWOCYCLES	  _SB_MAKEMASK1(60)

#define S_MC_tFIFO                56
#define M_MC_tFIFO                _SB_MAKEMASK(4,S_MC_tFIFO)
#define V_MC_tFIFO(x)             _SB_MAKEVALUE(x,S_MC_tFIFO)
#define K_MC_tFIFO_DEFAULT        1
#define V_MC_tFIFO_DEFAULT        V_MC_tFIFO(K_MC_tFIFO_DEFAULT)

#define S_MC_tRFC                 52
#define M_MC_tRFC                 _SB_MAKEMASK(4,S_MC_tRFC)
#define V_MC_tRFC(x)              _SB_MAKEVALUE(x,S_MC_tRFC)
#define K_MC_tRFC_DEFAULT         12
#define V_MC_tRFC_DEFAULT         V_MC_tRFC(K_MC_tRFC_DEFAULT)

#define S_MC_tCwCr                40
#define M_MC_tCwCr                _SB_MAKEMASK(4,S_MC_tCwCr)
#define V_MC_tCwCr(x)             _SB_MAKEVALUE(x,S_MC_tCwCr)
#define K_MC_tCwCr_DEFAULT        4
#define V_MC_tCwCr_DEFAULT        V_MC_tCwCr(K_MC_tCwCr_DEFAULT)

#define S_MC_tRCr                 28
#define M_MC_tRCr                 _SB_MAKEMASK(4,S_MC_tRCr)
#define V_MC_tRCr(x)              _SB_MAKEVALUE(x,S_MC_tRCr)
#define K_MC_tRCr_DEFAULT         9
#define V_MC_tRCr_DEFAULT         V_MC_tRCr(K_MC_tRCr_DEFAULT)

#define S_MC_tRCw                 24
#define M_MC_tRCw                 _SB_MAKEMASK(4,S_MC_tRCw)
#define V_MC_tRCw(x)              _SB_MAKEVALUE(x,S_MC_tRCw)
#define K_MC_tRCw_DEFAULT         10
#define V_MC_tRCw_DEFAULT         V_MC_tRCw(K_MC_tRCw_DEFAULT)

#define S_MC_tRRD                 20
#define M_MC_tRRD                 _SB_MAKEMASK(4,S_MC_tRRD)
#define V_MC_tRRD(x)              _SB_MAKEVALUE(x,S_MC_tRRD)
#define K_MC_tRRD_DEFAULT         2
#define V_MC_tRRD_DEFAULT         V_MC_tRRD(K_MC_tRRD_DEFAULT)

#define S_MC_tRP                  16
#define M_MC_tRP                  _SB_MAKEMASK(4,S_MC_tRP)
#define V_MC_tRP(x)               _SB_MAKEVALUE(x,S_MC_tRP)
#define K_MC_tRP_DEFAULT          4
#define V_MC_tRP_DEFAULT          V_MC_tRP(K_MC_tRP_DEFAULT)

#define S_MC_tCwD                 8
#define M_MC_tCwD                 _SB_MAKEMASK(4,S_MC_tCwD)
#define V_MC_tCwD(x)              _SB_MAKEVALUE(x,S_MC_tCwD)
#define K_MC_tCwD_DEFAULT         1
#define V_MC_tCwD_DEFAULT         V_MC_tCwD(K_MC_tCwD_DEFAULT)

#define M_tCrDh                   _SB_MAKEMASK1(7)

#define S_MC_tCrD                 4
#define M_MC_tCrD                 _SB_MAKEMASK(3,S_MC_tCrD)
#define V_MC_tCrD(x)              _SB_MAKEVALUE(x,S_MC_tCrD)
#define K_MC_tCrD_DEFAULT         2
#define V_MC_tCrD_DEFAULT         V_MC_tCrD(K_MC_tCrD_DEFAULT)

#define S_MC_tRCD                 0
#define M_MC_tRCD                 _SB_MAKEMASK(4,S_MC_tRCD)
#define V_MC_tRCD(x)              _SB_MAKEVALUE(x,S_MC_tRCD)
#define K_MC_tRCD_DEFAULT         3
#define V_MC_tRCD_DEFAULT         V_MC_tRCD(K_MC_tRCD_DEFAULT)

#define V_MC_TIMING_DEFAULT     V_MC_tFIFO(K_MC_tFIFO_DEFAULT) | \
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

/*
 * Errata says these are not the default
 *                               M_MC_w2rIDLE_TWOCYCLES | \
 *                               M_MC_r2wIDLE_TWOCYCLES | \
 */


/*
 * Chip Select Start Address Register (Table 6-17)
 */

#define S_MC_CS0_START              0
#define M_MC_CS0_START              _SB_MAKEMASK(16,S_MC_CS0_START)
#define V_MC_CS0_START(x)           _SB_MAKEVALUE(x,S_MC_CS0_START)
#define G_MC_CS0_START(x)           _SB_GETVALUE(x,S_MC_CS0_START,M_MC_CS0_START)

#define S_MC_CS1_START              16
#define M_MC_CS1_START              _SB_MAKEMASK(16,S_MC_CS1_START)
#define V_MC_CS1_START(x)           _SB_MAKEVALUE(x,S_MC_CS1_START)
#define G_MC_CS1_START(x)           _SB_GETVALUE(x,S_MC_CS1_START,M_MC_CS1_START)

#define S_MC_CS2_START              32
#define M_MC_CS2_START              _SB_MAKEMASK(16,S_MC_CS2_START)
#define V_MC_CS2_START(x)           _SB_MAKEVALUE(x,S_MC_CS2_START)
#define G_MC_CS2_START(x)           _SB_GETVALUE(x,S_MC_CS2_START,M_MC_CS2_START)

#define S_MC_CS3_START              48
#define M_MC_CS3_START              _SB_MAKEMASK(16,S_MC_CS3_START)
#define V_MC_CS3_START(x)           _SB_MAKEVALUE(x,S_MC_CS3_START)
#define G_MC_CS3_START(x)           _SB_GETVALUE(x,S_MC_CS3_START,M_MC_CS3_START)

/*
 * Chip Select End Address Register (Table 6-18)
 */

#define S_MC_CS0_END                0
#define M_MC_CS0_END                _SB_MAKEMASK(16,S_MC_CS0_END)
#define V_MC_CS0_END(x)             _SB_MAKEVALUE(x,S_MC_CS0_END)
#define G_MC_CS0_END(x)             _SB_GETVALUE(x,S_MC_CS0_END,M_MC_CS0_END)

#define S_MC_CS1_END                16
#define M_MC_CS1_END                _SB_MAKEMASK(16,S_MC_CS1_END)
#define V_MC_CS1_END(x)             _SB_MAKEVALUE(x,S_MC_CS1_END)
#define G_MC_CS1_END(x)             _SB_GETVALUE(x,S_MC_CS1_END,M_MC_CS1_END)

#define S_MC_CS2_END                32
#define M_MC_CS2_END                _SB_MAKEMASK(16,S_MC_CS2_END)
#define V_MC_CS2_END(x)             _SB_MAKEVALUE(x,S_MC_CS2_END)
#define G_MC_CS2_END(x)             _SB_GETVALUE(x,S_MC_CS2_END,M_MC_CS2_END)

#define S_MC_CS3_END                48
#define M_MC_CS3_END                _SB_MAKEMASK(16,S_MC_CS3_END)
#define V_MC_CS3_END(x)             _SB_MAKEVALUE(x,S_MC_CS3_END)
#define G_MC_CS3_END(x)             _SB_GETVALUE(x,S_MC_CS3_END,M_MC_CS3_END)

/*
 * Chip Select Interleave Register (Table 6-19)
 */

#define S_MC_INTLV_RESERVED         0
#define M_MC_INTLV_RESERVED         _SB_MAKEMASK(5,S_MC_INTLV_RESERVED)

#define S_MC_INTERLEAVE             7
#define M_MC_INTERLEAVE             _SB_MAKEMASK(18,S_MC_INTERLEAVE)
#define V_MC_INTERLEAVE(x)          _SB_MAKEVALUE(x,S_MC_INTERLEAVE)

#define S_MC_INTLV_MBZ              25
#define M_MC_INTLV_MBZ              _SB_MAKEMASK(39,S_MC_INTLV_MBZ)

/*
 * Row Address Bits Register (Table 6-20)
 */

#define S_MC_RAS_RESERVED           0
#define M_MC_RAS_RESERVED           _SB_MAKEMASK(5,S_MC_RAS_RESERVED)

#define S_MC_RAS_SELECT             12
#define M_MC_RAS_SELECT             _SB_MAKEMASK(25,S_MC_RAS_SELECT)
#define V_MC_RAS_SELECT(x)          _SB_MAKEVALUE(x,S_MC_RAS_SELECT)

#define S_MC_RAS_MBZ                37
#define M_MC_RAS_MBZ                _SB_MAKEMASK(27,S_MC_RAS_MBZ)


/*
 * Column Address Bits Register (Table 6-21)
 */

#define S_MC_CAS_RESERVED           0
#define M_MC_CAS_RESERVED           _SB_MAKEMASK(5,S_MC_CAS_RESERVED)

#define S_MC_CAS_SELECT             5
#define M_MC_CAS_SELECT             _SB_MAKEMASK(18,S_MC_CAS_SELECT)
#define V_MC_CAS_SELECT(x)          _SB_MAKEVALUE(x,S_MC_CAS_SELECT)

#define S_MC_CAS_MBZ                23
#define M_MC_CAS_MBZ                _SB_MAKEMASK(41,S_MC_CAS_MBZ)


/*
 * Bank Address Address Bits Register (Table 6-22)
 */

#define S_MC_BA_RESERVED            0
#define M_MC_BA_RESERVED            _SB_MAKEMASK(5,S_MC_BA_RESERVED)

#define S_MC_BA_SELECT              5
#define M_MC_BA_SELECT              _SB_MAKEMASK(20,S_MC_BA_SELECT)
#define V_MC_BA_SELECT(x)           _SB_MAKEVALUE(x,S_MC_BA_SELECT)

#define S_MC_BA_MBZ                 25
#define M_MC_BA_MBZ                 _SB_MAKEMASK(39,S_MC_BA_MBZ)

/*
 * Chip Select Attribute Register (Table 6-23)
 */

#define K_MC_CS_ATTR_CLOSED         0
#define K_MC_CS_ATTR_CASCHECK       1
#define K_MC_CS_ATTR_HINT           2
#define K_MC_CS_ATTR_OPEN           3

#define S_MC_CS0_PAGE               0
#define M_MC_CS0_PAGE               _SB_MAKEMASK(2,S_MC_CS0_PAGE)
#define V_MC_CS0_PAGE(x)            _SB_MAKEVALUE(x,S_MC_CS0_PAGE)
#define G_MC_CS0_PAGE(x)            _SB_GETVALUE(x,S_MC_CS0_PAGE,M_MC_CS0_PAGE)

#define S_MC_CS1_PAGE               16
#define M_MC_CS1_PAGE               _SB_MAKEMASK(2,S_MC_CS1_PAGE)
#define V_MC_CS1_PAGE(x)            _SB_MAKEVALUE(x,S_MC_CS1_PAGE)
#define G_MC_CS1_PAGE(x)            _SB_GETVALUE(x,S_MC_CS1_PAGE,M_MC_CS1_PAGE)

#define S_MC_CS2_PAGE               32
#define M_MC_CS2_PAGE               _SB_MAKEMASK(2,S_MC_CS2_PAGE)
#define V_MC_CS2_PAGE(x)            _SB_MAKEVALUE(x,S_MC_CS2_PAGE)
#define G_MC_CS2_PAGE(x)            _SB_GETVALUE(x,S_MC_CS2_PAGE,M_MC_CS2_PAGE)

#define S_MC_CS3_PAGE               48
#define M_MC_CS3_PAGE               _SB_MAKEMASK(2,S_MC_CS3_PAGE)
#define V_MC_CS3_PAGE(x)            _SB_MAKEVALUE(x,S_MC_CS3_PAGE)
#define G_MC_CS3_PAGE(x)            _SB_GETVALUE(x,S_MC_CS3_PAGE,M_MC_CS3_PAGE)

/*
 * ECC Test ECC Register (Table 6-25)
 */

#define S_MC_ECC_INVERT             0
#define M_MC_ECC_INVERT             _SB_MAKEMASK(8,S_MC_ECC_INVERT)


/*  *********************************************************************
    *  DMA constants
    ********************************************************************* */

/* 
 * Ethernet and Serial DMA Configuration Register 0  (Table 7-4)
 * Registers: DMA_CONFIG0_MAC_x_RX_CH_0 
 * Registers: DMA_CONFIG0_MAC_x_TX_CH_0
 * Registers: DMA_CONFIG0_SER_x_RX
 * Registers: DMA_CONFIG0_SER_x_TX
 */


#define M_DMA_DROP                  _SB_MAKEMASK1(0)
#define M_DMA_CHAIN_SEL             _SB_MAKEMASK1(1)
#define M_DMA_RESERVED1             _SB_MAKEMASK1(2)
#define M_DMA_EOP_INT_EN            _SB_MAKEMASK1(3)
#define M_DMA_HWM_INT_EN            _SB_MAKEMASK1(4)
#define M_DMA_LWM_INT_EN            _SB_MAKEMASK1(5)
#define M_DMA_TBX_EN                _SB_MAKEMASK1(6)
#define M_DMA_TDX_EN                _SB_MAKEMASK1(7)

#define S_DMA_INT_PKTCNT            _SB_MAKE64(8)
#define M_DMA_INT_PKTCNT            _SB_MAKEMASK(8,S_DMA_INT_PKTCNT)
#define V_DMA_INT_PKTCNT(x)         _SB_MAKEVALUE(x,S_DMA_INT_PKTCNT)
#define G_DMA_INT_PKTCNT(x)         _SB_GETVALUE(x,S_DMA_INT_PKTCNT,M_DMA_INT_PKTCNT)

#define S_DMA_RINGSZ                _SB_MAKE64(16)
#define M_DMA_RINGSZ                _SB_MAKEMASK(16,S_DMA_RINGSZ)
#define V_DMA_RINGSZ(x)             _SB_MAKEVALUE(x,S_DMA_RINGSZ)
#define G_DMA_RINGSZ(x)             _SB_GETVALUE(x,S_DMA_RINGSZ,M_DMA_RINGSZ)

#define S_DMA_HIGH_WATERMARK        _SB_MAKE64(32)
#define M_DMA_HIGH_WATERMARK        _SB_MAKEMASK(16,S_DMA_HIGH_WATERMARK)
#define V_DMA_HIGH_WATERMARK(x)     _SB_MAKEVALUE(x,S_DMA_HIGH_WATERMARK)
#define G_DMA_HIGH_WATERMARK(x)     _SB_GETVALUE(x,S_DMA_HIGH_WATERMARK,M_DMA_HIGH_WATERMARK)

#define S_DMA_LOW_WATERMARK         _SB_MAKE64(48)
#define M_DMA_LOW_WATERMARK         _SB_MAKEMASK(16,S_DMA_LOW_WATERMARK)
#define V_DMA_LOW_WATERMARK(x)      _SB_MAKEVALUE(x,S_DMA_LOW_WATERMARK)
#define G_DMA_LOW_WATERMARK(x)      _SB_GETVALUE(x,S_DMA_LOW_WATERMARK,M_DMA_LOW_WATERMARK)

/*
 * Ethernet and Serial DMA Configuration Register 2 (Table 7-5)
 * Registers: DMA_CONFIG1_MAC_x_RX_CH_0 
 * Registers: DMA_CONFIG1_DMA_x_TX_CH_0
 * Registers: DMA_CONFIG1_SER_x_RX
 * Registers: DMA_CONFIG1_SER_x_TX
 */

#define M_DMA_HDR_CF_EN             _SB_MAKEMASK1(0)
#define M_DMA_ASIC_XFR_EN           _SB_MAKEMASK1(1)
#define M_DMA_PRE_ADDR_EN           _SB_MAKEMASK1(2)
#define M_DMA_FLOW_CTL_EN           _SB_MAKEMASK1(3)
#define M_DMA_NO_DSCR_UPDT          _SB_MAKEMASK1(4)
#define M_DMA_L2CA		    _SB_MAKEMASK1(5)

#define M_DMA_MBZ1                  _SB_MAKEMASK(6,15)

#define S_DMA_HDR_SIZE              _SB_MAKE64(21)
#define M_DMA_HDR_SIZE              _SB_MAKEMASK(9,S_DMA_HDR_SIZE)
#define V_DMA_HDR_SIZE(x)           _SB_MAKEVALUE(x,S_DMA_HDR_SIZE)
#define G_DMA_HDR_SIZE(x)           _SB_GETVALUE(x,S_DMA_HDR_SIZE,M_DMA_HDR_SIZE)

#define M_DMA_MBZ2                  _SB_MAKEMASK(5,32)

#define S_DMA_ASICXFR_SIZE          _SB_MAKE64(37)
#define M_DMA_ASICXFR_SIZE          _SB_MAKEMASK(9,S_DMA_ASICXFR_SIZE)
#define V_DMA_ASICXFR_SIZE(x)       _SB_MAKEVALUE(x,S_DMA_ASICXFR_SIZE)
#define G_DMA_ASICXFR_SIZE(x)       _SB_GETVALUE(x,S_DMA_ASICXFR_SIZE,M_DMA_ASICXFR_SIZE)

#define S_DMA_INT_TIMEOUT           _SB_MAKE64(48)
#define M_DMA_INT_TIMEOUT           _SB_MAKEMASK(16,S_DMA_INT_TIMEOUT)
#define V_DMA_INT_TIMEOUT(x)        _SB_MAKEVALUE(x,S_DMA_INT_TIMEOUT)
#define G_DMA_INT_TIMEOUT(x)        _SB_GETVALUE(x,S_DMA_INT_TIMEOUT,M_DMA_INT_TIMEOUT)

/*
 * Ethernet and Serial DMA Descriptor base address (Table 7-6)
 */

#define M_DMA_DSCRBASE_MBZ          _SB_MAKEMASK(4,0)


/*
 * ASIC Mode Base Address (Table 7-7)
 */

#define M_DMA_ASIC_BASE_MBZ         _SB_MAKEMASK(20,0)

/*
 * DMA Descriptor Count Registers (Table 7-8)
 */
 
/* No bitfields */


/* 
 * Current Descriptor Address Register (Table 7-11)
 */

#define S_DMA_CURDSCR_ADDR          _SB_MAKE64(0)
#define M_DMA_CURDSCR_ADDR          _SB_MAKEMASK(40,S_DMA_CURDSCR_ADDR)
#define S_DMA_CURDSCR_COUNT         _SB_MAKE64(48)
#define M_DMA_CURDSCR_COUNT         _SB_MAKEMASK(16,S_DMA_CURDSCR_COUNT)

/*  *********************************************************************
    *  DMA Descriptors
    ********************************************************************* */

/*
 * Descriptor doubleword "A"  (Table 7-12)
 */

#define S_DMA_DSCRA_OFFSET          _SB_MAKE64(0)
#define M_DMA_DSCRA_OFFSET          _SB_MAKEMASK(5,S_DMA_DSCRA_OFFSET)

/* Note: Don't shift the address over, just mask it with the mask below */
#define S_DMA_DSCRA_A_ADDR          _SB_MAKE64(5)
#define M_DMA_DSCRA_A_ADDR          _SB_MAKEMASK(35,S_DMA_DSCRA_A_ADDR)

#define M_DMA_DSCRA_A_ADDR_OFFSET   (M_DMA_DSCRA_OFFSET | M_DMA_DSCRA_A_ADDR)

#define S_DMA_DSCRA_A_SIZE          _SB_MAKE64(40)
#define M_DMA_DSCRA_A_SIZE          _SB_MAKEMASK(9,S_DMA_DSCRA_A_SIZE)
#define V_DMA_DSCRA_A_SIZE(x)       _SB_MAKEVALUE(x,S_DMA_DSCRA_A_SIZE)
#define G_DMA_DSCRA_A_SIZE(x)       _SB_GETVALUE(x,S_DMA_DSCRA_A_SIZE,M_DMA_DSCRA_A_SIZE)

#define M_DMA_DSCRA_INTERRUPT       _SB_MAKEMASK1(49)
#define M_DMA_DSCRA_OFFSETB	    _SB_MAKEMASK1(50)

#define S_DMA_DSCRA_STATUS          _SB_MAKE64(51)
#define M_DMA_DSCRA_STATUS          _SB_MAKEMASK(13,S_DMA_DSCRA_STATUS)
#define V_DMA_DSCRA_STATUS(x)       _SB_MAKEVALUE(x,S_DMA_DSCRA_STATUS)
#define G_DMA_DSCRA_STATUS(x)       _SB_GETVALUE(x,S_DMA_DSCRA_STATUS,M_DMA_DSCRA_STATUS)

/*
 * Descriptor doubleword "B"  (Table 7-13)
 */


#define S_DMA_DSCRB_OPTIONS         _SB_MAKE64(0)
#define M_DMA_DSCRB_OPTIONS         _SB_MAKEMASK(4,S_DMA_DSCRB_OPTIONS)
#define V_DMA_DSCRB_OPTIONS(x)      _SB_MAKEVALUE(x,S_DMA_DSCRB_OPTIONS)
#define G_DMA_DSCRB_OPTIONS(x)      _SB_GETVALUE(x,S_DMA_DSCRB_OPTIONS,M_DMA_DSCRB_OPTIONS)

#define R_DMA_DSCRB_ADDR            _SB_MAKE64(0x10)

/* Note: Don't shift the address over, just mask it with the mask below */
#define S_DMA_DSCRB_B_ADDR          _SB_MAKE64(5)
#define M_DMA_DSCRB_B_ADDR          _SB_MAKEMASK(35,S_DMA_DSCRB_B_ADDR)

#define S_DMA_DSCRB_B_SIZE          _SB_MAKE64(40)
#define M_DMA_DSCRB_B_SIZE          _SB_MAKEMASK(9,S_DMA_DSCRB_B_SIZE)
#define V_DMA_DSCRB_B_SIZE(x)       _SB_MAKEVALUE(x,S_DMA_DSCRB_B_SIZE)
#define G_DMA_DSCRB_B_SIZE(x)       _SB_GETVALUE(x,S_DMA_DSCRB_B_SIZE,M_DMA_DSCRB_B_SIZE)

#define M_DMA_DSCRB_B_VALID         _SB_MAKEMASK1(49)

#define S_DMA_DSCRB_PKT_SIZE        _SB_MAKE64(50)
#define M_DMA_DSCRB_PKT_SIZE        _SB_MAKEMASK(14,S_DMA_DSCRB_PKT_SIZE)
#define V_DMA_DSCRB_PKT_SIZE(x)     _SB_MAKEVALUE(x,S_DMA_DSCRB_PKT_SIZE)
#define G_DMA_DSCRB_PKT_SIZE(x)     _SB_GETVALUE(x,S_DMA_DSCRB_PKT_SIZE,M_DMA_DSCRB_PKT_SIZE)

/* 
 * Ethernet Descriptor Status Bits (Table 7-15)
 */

#define M_DMA_ETHRX_BADIP4CS        _SB_MAKEMASK1(51)
#define M_DMA_ETHRX_DSCRERR	    _SB_MAKEMASK1(52)

#define S_DMA_ETHRX_RXCH            53
#define M_DMA_ETHRX_RXCH            _SB_MAKEMASK(2,S_DMA_ETHRX_RXCH)
#define V_DMA_ETHRX_RXCH(x)         _SB_MAKEVALUE(x,S_DMA_ETHRX_RXCH)
#define G_DMA_ETHRX_RXCH(x)         _SB_GETVALUE(x,S_DMA_ETHRX_RXCH,M_DMA_ETHRX_RXCH)

#define S_DMA_ETHRX_PKTTYPE         55
#define M_DMA_ETHRX_PKTTYPE         _SB_MAKEMASK(3,S_DMA_ETHRX_PKTTYPE)
#define V_DMA_ETHRX_PKTTYPE(x)      _SB_MAKEVALUE(x,S_DMA_ETHRX_PKTTYPE)
#define G_DMA_ETHRX_PKTTYPE(x)      _SB_GETVALUE(x,S_DMA_ETHRX_PKTTYPE,M_DMA_ETHRX_PKTTYPE)

#define K_DMA_ETHRX_PKTTYPE_IPV4    0
#define K_DMA_ETHRX_PKTTYPE_ARPV4   1
#define K_DMA_ETHRX_PKTTYPE_802     2
#define K_DMA_ETHRX_PKTTYPE_OTHER   3
#define K_DMA_ETHRX_PKTTYPE_USER0   4
#define K_DMA_ETHRX_PKTTYPE_USER1   5
#define K_DMA_ETHRX_PKTTYPE_USER2   6
#define K_DMA_ETHRX_PKTTYPE_USER3   7

#define M_DMA_ETHRX_MATCH_EXACT     _SB_MAKEMASK1(58)
#define M_DMA_ETHRX_MATCH_HASH      _SB_MAKEMASK1(59)
#define M_DMA_ETHRX_BCAST           _SB_MAKEMASK1(60)
#define M_DMA_ETHRX_MCAST           _SB_MAKEMASK1(61)
#define M_DMA_ETHRX_BAD	            _SB_MAKEMASK1(62)
#define M_DMA_ETHRX_SOP             _SB_MAKEMASK1(63)

/*
 * Ethernet Transmit Status Bits (Table 7-16)
 */

#define M_DMA_ETHTX_SOP	    	    _SB_MAKEMASK1(63)

/* 
 * Ethernet Transmit Options (Table 7-17)
 */

#define K_DMA_ETHTX_NOTSOP          _SB_MAKE64(0x00)
#define K_DMA_ETHTX_APPENDCRC       _SB_MAKE64(0x01)
#define K_DMA_ETHTX_REPLACECRC      _SB_MAKE64(0x02)
#define K_DMA_ETHTX_APPENDCRC_APPENDPAD _SB_MAKE64(0x03)
#define K_DMA_ETHTX_APPENDVLAN_REPLACECRC _SB_MAKE64(0x04)
#define K_DMA_ETHTX_REMOVEVLAN_REPLACECRC _SB_MAKE64(0x05)
#define K_DMA_ETHTX_REPLACEVLAN_REPLACECRC _SB_MAKE64(0x6)
#define K_DMA_ETHTX_NOMODS          _SB_MAKE64(0x07)
#define K_DMA_ETHTX_RESERVED1       _SB_MAKE64(0x08)
#define K_DMA_ETHTX_REPLACESADDR_APPENDCRC _SB_MAKE64(0x09)
#define K_DMA_ETHTX_REPLACESADDR_REPLACECRC _SB_MAKE64(0x0A)
#define K_DMA_ETHTX_REPLACESADDR_APPENDCRC_APPENDPAD _SB_MAKE64(0x0B)
#define K_DMA_ETHTX_REPLACESADDR_APPENDVLAN_REPLACECRC _SB_MAKE64(0x0C)
#define K_DMA_ETHTX_REPLACESADDR_REMOVEVLAN_REPLACECRC _SB_MAKE64(0x0D)
#define K_DMA_ETHTX_REPLACESADDR_REPLACEVLAN_REPLACECRC _SB_MAKE64(0x0E)
#define K_DMA_ETHTX_RESERVED2       _SB_MAKE64(0x0F)

/*
 * Serial Receive Options (Table 7-18)
 */
#define M_DMA_SERRX_CRC_ERROR       _SB_MAKEMASK1(56)
#define M_DMA_SERRX_ABORT           _SB_MAKEMASK1(57)
#define M_DMA_SERRX_OCTET_ERROR     _SB_MAKEMASK1(58)
#define M_DMA_SERRX_LONGFRAME_ERROR _SB_MAKEMASK1(59)
#define M_DMA_SERRX_SHORTFRAME_ERROR _SB_MAKEMASK1(60)
#define M_DMA_SERRX_OVERRUN_ERROR   _SB_MAKEMASK1(61)
#define M_DMA_SERRX_GOOD            _SB_MAKEMASK1(62)
#define M_DMA_SERRX_SOP             _SB_MAKEMASK1(63)

/*
 * Serial Transmit Status Bits (Table 7-20)
 */

#define M_DMA_SERTX_FLAG	    _SB_MAKEMASK1(63)

/*
 * Serial Transmit Options (Table 7-21)
 */

#define K_DMA_SERTX_RESERVED        _SB_MAKEMASK1(0)
#define K_DMA_SERTX_APPENDCRC       _SB_MAKEMASK1(1)
#define K_DMA_SERTX_APPENDPAD       _SB_MAKEMASK1(2)
#define K_DMA_SERTX_ABORT           _SB_MAKEMASK1(3)


/*  *********************************************************************
    *  Data Mover Registers
    ********************************************************************* */

/* 
 * Data Mover Descriptor Base Address Register (Table 7-22)
 * Register: DM_DSCR_BASE_0
 * Register: DM_DSCR_BASE_1
 * Register: DM_DSCR_BASE_2
 * Register: DM_DSCR_BASE_3
 */

#define M_DM_DSCR_BASE_MBZ          _SB_MAKEMASK(3,0)

/*  Note: Just mask the base address and then OR it in. */
#define S_DM_DSCR_BASE_ADDR         _SB_MAKE64(3)
#define M_DM_DSCR_BASE_ADDR         _SB_MAKEMASK(36,S_DM_DSCR_BASE_ADDR)

#define S_DM_DSCR_BASE_RINGSZ       _SB_MAKE64(40)
#define M_DM_DSCR_BASE_RINGSZ       _SB_MAKEMASK(16,S_DM_DSCR_BASE_RINGSZ)
#define V_DM_DSCR_BASE_RINGSZ(x)    _SB_MAKEVALUE(x,S_DM_DSCR_BASE_RINGSZ)
#define G_DM_DSCR_BASE_RINGSZ(x)    _SB_GETVALUE(x,S_DM_DSCR_BASE_RINGSZ,M_DM_DSCR_BASE_RINGSZ)

#define S_DM_DSCR_BASE_PRIORITY     _SB_MAKE64(56)
#define M_DM_DSCR_BASE_PRIORITY     _SB_MAKEMASK(3,S_DM_DSCR_BASE_PRIORITY)
#define V_DM_DSCR_BASE_PRIORITY(x)  _SB_MAKEVALUE(x,S_DM_DSCR_BASE_PRIORITY)
#define G_DM_DSCR_BASE_PRIORITY(x)  _SB_GETVALUE(x,S_DM_DSCR_BASE_PRIORITY,M_DM_DSCR_BASE_PRIORITY)

#define K_DM_DSCR_BASE_PRIORITY_1   0
#define K_DM_DSCR_BASE_PRIORITY_2   1
#define K_DM_DSCR_BASE_PRIORITY_4   2
#define K_DM_DSCR_BASE_PRIORITY_8   3
#define K_DM_DSCR_BASE_PRIORITY_16  4

#define M_DM_DSCR_BASE_ACTIVE       _SB_MAKEMASK1(59)
#define M_DM_DSCR_BASE_INTERRUPT    _SB_MAKEMASK1(60)
#define M_DM_DSCR_BASE_RESET        _SB_MAKEMASK1(61)	/* write register */
#define M_DM_DSCR_BASE_ERROR        _SB_MAKEMASK1(61)	/* read register */
#define M_DM_DSCR_BASE_ABORT        _SB_MAKEMASK1(62)
#define M_DM_DSCR_BASE_ENABL        _SB_MAKEMASK1(63)

/* 
 * Data Mover Descriptor Count Register (Table 7-25)
 */

/* no bitfields */

/*
 * Data Mover Current Descriptor Address (Table 7-24)
 * Register: DM_CUR_DSCR_ADDR_0
 * Register: DM_CUR_DSCR_ADDR_1
 * Register: DM_CUR_DSCR_ADDR_2
 * Register: DM_CUR_DSCR_ADDR_3
 */

#define S_DM_CUR_DSCR_DSCR_ADDR     _SB_MAKE64(0)
#define M_DM_CUR_DSCR_DSCR_ADDR     _SB_MAKEMASK(40,S_DM_CUR_DSCR_DSCR_ADDR)

#define S_DM_CUR_DSCR_DSCR_COUNT    _SB_MAKE64(48)
#define M_DM_CUR_DSCR_DSCR_COUNT    _SB_MAKEMASK(16,S_DM_CUR_DSCR_DSCR_COUNT)
#define V_DM_CUR_DSCR_DSCR_COUNT(r) _SB_MAKEVALUE(r,S_DM_CUR_DSCR_DSCR_COUNT)
#define G_DM_CUR_DSCR_DSCR_COUNT(r) _SB_GETVALUE(r,S_DM_CUR_DSCR_DSCR_COUNT,\
                                     M_DM_CUR_DSCR_DSCR_COUNT)

/*
 * Data Mover Descriptor Doubleword "A"  (Table 7-26)
 */

#define S_DM_DSCRA_DST_ADDR         _SB_MAKE64(0)
#define M_DM_DSCRA_DST_ADDR         _SB_MAKEMASK(40,S_DM_DSCRA_DST_ADDR)

#define M_DM_DSCRA_UN_DEST          _SB_MAKEMASK1(40)
#define M_DM_DSCRA_UN_SRC           _SB_MAKEMASK1(41)
#define M_DM_DSCRA_INTERRUPT        _SB_MAKEMASK1(42)
#define M_DM_DSCRA_THROTTLE         _SB_MAKEMASK1(43)

#define S_DM_DSCRA_DIR_DEST         _SB_MAKE64(44)
#define M_DM_DSCRA_DIR_DEST         _SB_MAKEMASK(2,S_DM_DSCRA_DIR_DEST)
#define V_DM_DSCRA_DIR_DEST(x)      _SB_MAKEVALUE(x,S_DM_DSCRA_DIR_DEST)
#define G_DM_DSCRA_DIR_DEST(x)      _SB_GETVALUE(x,S_DM_DSCRA_DIR_DEST,M_DM_DSCRA_DIR_DEST)

#define K_DM_DSCRA_DIR_DEST_INCR    0
#define K_DM_DSCRA_DIR_DEST_DECR    1
#define K_DM_DSCRA_DIR_DEST_CONST   2

#define V_DM_DSCRA_DIR_DEST_INCR    _SB_MAKEVALUE(K_DM_DSCRA_DIR_DEST_INCR,S_DM_DSCRA_DIR_DEST)
#define V_DM_DSCRA_DIR_DEST_DECR    _SB_MAKEVALUE(K_DM_DSCRA_DIR_DEST_DECR,S_DM_DSCRA_DIR_DEST)
#define V_DM_DSCRA_DIR_DEST_CONST   _SB_MAKEVALUE(K_DM_DSCRA_DIR_DEST_CONST,S_DM_DSCRA_DIR_DEST)

#define S_DM_DSCRA_DIR_SRC          _SB_MAKE64(46)
#define M_DM_DSCRA_DIR_SRC          _SB_MAKEMASK(2,S_DM_DSCRA_DIR_SRC)
#define V_DM_DSCRA_DIR_SRC(x)       _SB_MAKEVALUE(x,S_DM_DSCRA_DIR_SRC)
#define G_DM_DSCRA_DIR_SRC(x)       _SB_GETVALUE(x,S_DM_DSCRA_DIR_SRC,M_DM_DSCRA_DIR_SRC)

#define K_DM_DSCRA_DIR_SRC_INCR     0
#define K_DM_DSCRA_DIR_SRC_DECR     1
#define K_DM_DSCRA_DIR_SRC_CONST    2

#define V_DM_DSCRA_DIR_SRC_INCR     _SB_MAKEVALUE(K_DM_DSCRA_DIR_SRC_INCR,S_DM_DSCRA_DIR_SRC)
#define V_DM_DSCRA_DIR_SRC_DECR     _SB_MAKEVALUE(K_DM_DSCRA_DIR_SRC_DECR,S_DM_DSCRA_DIR_SRC)
#define V_DM_DSCRA_DIR_SRC_CONST    _SB_MAKEVALUE(K_DM_DSCRA_DIR_SRC_CONST,S_DM_DSCRA_DIR_SRC)


#define M_DM_DSCRA_ZERO_MEM         _SB_MAKEMASK1(48)
#define M_DM_DSCRA_PREFETCH         _SB_MAKEMASK1(49)
#define M_DM_DSCRA_L2C_DEST         _SB_MAKEMASK1(50)
#define M_DM_DSCRA_L2C_SRC          _SB_MAKEMASK1(51)

#define M_DM_DSCRA_RESERVED2        _SB_MAKEMASK(12,52)

/*
 * Data Mover Descriptor Doubleword "B"  (Table 7-25)
 */

#define S_DM_DSCRB_SRC_ADDR         _SB_MAKE64(0)
#define M_DM_DSCRB_SRC_ADDR         _SB_MAKEMASK(40,S_DM_DSCRB_SRC_ADDR)

#define S_DM_DSCRB_SRC_LENGTH       _SB_MAKE64(40)
#define M_DM_DSCRB_SRC_LENGTH       _SB_MAKEMASK(20,S_DM_DSCRB_SRC_LENGTH)
#define V_DM_DSCRB_SRC_LENGTH(x)    _SB_MAKEVALUE(x,S_DM_DSCRB_SRC_LENGTH)
#define G_DM_DSCRB_SRC_LENGTH(x)    _SB_GETVALUE(x,S_DM_DSCRB_SRC_LENGTH,M_DM_DSCRB_SRC_LENGTH)


/*  *********************************************************************
    *  Ethernet MAC Registers
    ********************************************************************* */

/*
 * MAC Configuration Register (Table 9-13)
 * Register: MAC_CFG_0
 * Register: MAC_CFG_1
 * Register: MAC_CFG_2
 */


/* Updated to spec 0.2 */

#define M_MAC_RESERVED0             _SB_MAKEMASK1(0)
#define M_MAC_TX_HOLD_SOP_EN        _SB_MAKEMASK1(1)
#define M_MAC_RETRY_EN              _SB_MAKEMASK1(2)
#define M_MAC_RET_DRPREQ_EN         _SB_MAKEMASK1(3)
#define M_MAC_RET_UFL_EN            _SB_MAKEMASK1(4)
#define M_MAC_BURST_EN              _SB_MAKEMASK1(5)

#define S_MAC_TX_PAUSE              _SB_MAKE64(6)
#define M_MAC_TX_PAUSE_CNT          _SB_MAKEMASK(3,S_MAC_TX_PAUSE)
#define V_MAC_TX_PAUSE_CNT(x)       _SB_MAKEVALUE(x,S_MAC_TX_PAUSE)

#define K_MAC_TX_PAUSE_CNT_512      0
#define K_MAC_TX_PAUSE_CNT_1K       1
#define K_MAC_TX_PAUSE_CNT_2K       2
#define K_MAC_TX_PAUSE_CNT_4K       3
#define K_MAC_TX_PAUSE_CNT_8K       4
#define K_MAC_TX_PAUSE_CNT_16K      5
#define K_MAC_TX_PAUSE_CNT_32K      6
#define K_MAC_TX_PAUSE_CNT_64K      7

#define V_MAC_TX_PAUSE_CNT_512      V_MAC_TX_PAUSE_CNT(K_MAC_TX_PAUSE_CNT_512)
#define V_MAC_TX_PAUSE_CNT_1K       V_MAC_TX_PAUSE_CNT(K_MAC_TX_PAUSE_CNT_1K)
#define V_MAC_TX_PAUSE_CNT_2K       V_MAC_TX_PAUSE_CNT(K_MAC_TX_PAUSE_CNT_2K)
#define V_MAC_TX_PAUSE_CNT_4K       V_MAC_TX_PAUSE_CNT(K_MAC_TX_PAUSE_CNT_4K)
#define V_MAC_TX_PAUSE_CNT_8K       V_MAC_TX_PAUSE_CNT(K_MAC_TX_PAUSE_CNT_8K)
#define V_MAC_TX_PAUSE_CNT_16K      V_MAC_TX_PAUSE_CNT(K_MAC_TX_PAUSE_CNT_16K)
#define V_MAC_TX_PAUSE_CNT_32K      V_MAC_TX_PAUSE_CNT(K_MAC_TX_PAUSE_CNT_32K)
#define V_MAC_TX_PAUSE_CNT_64K      V_MAC_TX_PAUSE_CNT(K_MAC_TX_PAUSE_CNT_64K)

#define M_MAC_RESERVED1             _SB_MAKEMASK(8,9)

#define M_MAC_AP_STAT_EN            _SB_MAKEMASK1(17)
#define M_MAC_RESERVED2		    _SB_MAKEMASK1(18)
#define M_MAC_DRP_ERRPKT_EN         _SB_MAKEMASK1(19)
#define M_MAC_DRP_FCSERRPKT_EN      _SB_MAKEMASK1(20)
#define M_MAC_DRP_CODEERRPKT_EN     _SB_MAKEMASK1(21)
#define M_MAC_DRP_DRBLERRPKT_EN     _SB_MAKEMASK1(22)
#define M_MAC_DRP_RNTPKT_EN         _SB_MAKEMASK1(23)
#define M_MAC_DRP_OSZPKT_EN         _SB_MAKEMASK1(24)
#define M_MAC_DRP_LENERRPKT_EN      _SB_MAKEMASK1(25)

#define M_MAC_RESERVED3             _SB_MAKEMASK(6,26)

#define M_MAC_BYPASS_SEL            _SB_MAKEMASK1(32)
#define M_MAC_HDX_EN                _SB_MAKEMASK1(33)

#define S_MAC_SPEED_SEL             _SB_MAKE64(34)
#define M_MAC_SPEED_SEL             _SB_MAKEMASK(2,S_MAC_SPEED_SEL)
#define V_MAC_SPEED_SEL(x)	    _SB_MAKEVALUE(x,S_MAC_SPEED_SEL)
#define G_MAC_SPEED_SEL(x)	    _SB_GETVALUE(x,S_MAC_SPEED_SEL,M_MAC_SPEED_SEL)

#define K_MAC_SPEED_SEL_10MBPS      0
#define K_MAC_SPEED_SEL_100MBPS     1
#define K_MAC_SPEED_SEL_1000MBPS    2
#define K_MAC_SPEED_SEL_RESERVED    3

#define V_MAC_SPEED_SEL_10MBPS      V_MAC_SPEED_SEL(K_MAC_SPEED_SEL_10MBPS)
#define V_MAC_SPEED_SEL_100MBPS     V_MAC_SPEED_SEL(K_MAC_SPEED_SEL_100MBPS)
#define V_MAC_SPEED_SEL_1000MBPS    V_MAC_SPEED_SEL(K_MAC_SPEED_SEL_1000MBPS)
#define V_MAC_SPEED_SEL_RESERVED    V_MAC_SPEED_SEL(K_MAC_SPEED_SEL_RESERVED)

#define M_MAC_TX_CLK_EDGE_SEL       _SB_MAKEMASK1(36)
#define M_MAC_LOOPBACK_SEL          _SB_MAKEMASK1(37)
#define M_MAC_FAST_SYNC             _SB_MAKEMASK1(38)
#define M_MAC_SS_EN                 _SB_MAKEMASK1(39)

#define S_MAC_BYPASS_CFG	    _SB_MAKE64(40)
#define M_MAC_BYPASS_CFG            _SB_MAKEMASK(2,S_MAC_BYPASS_CFG)
#define V_MAC_BYPASS_CFG(x)         _SB_MAKEVALUE(x,S_MAC_BYPASS_CFG)
#define G_MAC_BYPASS_CFG(x)         _SB_GETVALUE(x,S_MAC_BYPASS_CFG,M_MAC_BYPASS_CFG)

#define K_MAC_BYPASS_GMII	    0
#define K_MAC_BYPASS_ENCODED        1
#define K_MAC_BYPASS_SOP            2
#define K_MAC_BYPASS_EOP            3

#define M_MAC_BYPASS_16             _SB_MAKEMASK1(42)
#define M_MAC_BYPASS_FCS_CHK	    _SB_MAKEMASK1(43)

#define M_MAC_RESERVED4	    	    _SB_MAKEMASK(2,44)

#define S_MAC_BYPASS_IFG            _SB_MAKE64(46)
#define M_MAC_BYPASS_IFG            _SB_MAKEMASK(8,S_MAC_BYPASS_IFG)
#define V_MAC_BYPASS_IFG(x)	    _SB_MAKEVALUE(x,S_MAC_BYPASS_IFG)
#define G_MAC_BYPASS_IFG(x)	    _SB_GETVALUE(x,S_MAC_BYPASS_IFG,M_MAC_BYPASS_IFG)

#define K_MAC_FC_CMD_DISABLED       0
#define K_MAC_FC_CMD_ENABLED        1
#define K_MAC_FC_CMD_ENAB_FALSECARR 2

#define V_MAC_FC_CMD_DISABLED       V_MAC_FC_CMD(K_MAC_FC_CMD_DISABLED)
#define V_MAC_FC_CMD_ENABLED        V_MAC_FC_CMD(K_MAC_FC_CMD_ENABLED)
#define V_MAC_FC_CMD_ENAB_FALSECARR V_MAC_FC_CMD(K_MAC_FC_CMD_ENAB_FALSECARR)

#define M_MAC_FC_SEL                _SB_MAKEMASK1(54)

#define S_MAC_FC_CMD                _SB_MAKE64(55)
#define M_MAC_FC_CMD                _SB_MAKEMASK(2,S_MAC_FC_CMD)
#define V_MAC_FC_CMD(x)	            _SB_MAKEVALUE(x,S_MAC_FC_CMD)
#define G_MAC_FC_CMD(x)	            _SB_GETVALUE(x,S_MAC_FC_CMD,M_MAC_FC_CMD)

#define S_MAC_RX_CH_SEL             _SB_MAKE64(57)
#define M_MAC_RX_CH_SEL             _SB_MAKEMASK(7,S_MAC_RX_CH_SEL)
#define V_MAC_RX_CH_SEL(x)          _SB_MAKEVALUE(x,S_MAC_RX_CH_SEL)
#define G_MAC_RX_CH_SEL(x)          _SB_GETVALUE(x,S_MAC_RX_CH_SEL,M_MAC_RX_CH_SEL)


/*
 * MAC Enable Registers
 * Register: MAC_ENABLE_0
 * Register: MAC_ENABLE_1
 * Register: MAC_ENABLE_2
 */

#define M_MAC_RXDMA_EN0	            _SB_MAKEMASK1(0)
#define M_MAC_RXDMA_EN1	            _SB_MAKEMASK1(1)
#define M_MAC_TXDMA_EN0	            _SB_MAKEMASK1(4)
#define M_MAC_TXDMA_EN1	            _SB_MAKEMASK1(5)

#define M_MAC_PORT_RESET            _SB_MAKEMASK1(8)

#define M_MAC_RX_ENABLE             _SB_MAKEMASK1(10)
#define M_MAC_TX_ENABLE             _SB_MAKEMASK1(11)
#define M_MAC_BYP_RX_ENABLE         _SB_MAKEMASK1(12)
#define M_MAC_BYP_TX_ENABLE         _SB_MAKEMASK1(13)

/*
 * MAC DMA Control Register
 * Register: MAC_TXD_CTL_0
 * Register: MAC_TXD_CTL_1
 * Register: MAC_TXD_CTL_2
 */

#define S_MAC_TXD_WEIGHT0	    _SB_MAKE64(0)
#define M_MAC_TXD_WEIGHT0	    _SB_MAKEMASK(4,S_MAC_TXD_WEIGHT0)
#define V_MAC_TXD_WEIGHT0(x)        _SB_MAKEVALUE(x,S_MAC_TXD_WEIGHT0)
#define G_MAC_TXD_WEIGHT0(x)        _SB_GETVALUE(x,S_MAC_TXD_WEIGHT0,M_MAC_TXD_WEIGHT0)

#define S_MAC_TXD_WEIGHT1	    _SB_MAKE64(4)
#define M_MAC_TXD_WEIGHT1	    _SB_MAKEMASK(4,S_MAC_TXD_WEIGHT1)
#define V_MAC_TXD_WEIGHT1(x)        _SB_MAKEVALUE(x,S_MAC_TXD_WEIGHT1)
#define G_MAC_TXD_WEIGHT1(x)        _SB_GETVALUE(x,S_MAC_TXD_WEIGHT1,M_MAC_TXD_WEIGHT1)

/*
 * MAC Fifo Threshhold registers (Table 9-14)
 * Register: MAC_THRSH_CFG_0
 * Register: MAC_THRSH_CFG_1
 * Register: MAC_THRSH_CFG_2
 */

#define S_MAC_TX_WR_THRSH           _SB_MAKE64(0)
#define M_MAC_TX_WR_THRSH           _SB_MAKEMASK(6,S_MAC_TX_WR_THRSH)
#define V_MAC_TX_WR_THRSH(x)        _SB_MAKEVALUE(x,S_MAC_TX_WR_THRSH)
#define G_MAC_TX_WR_THRSH(x)        _SB_GETVALUE(x,S_MAC_TX_WR_THRSH,M_MAC_TX_WR_THRSH)

#define S_MAC_TX_RD_THRSH           _SB_MAKE64(8)
#define M_MAC_TX_RD_THRSH           _SB_MAKEMASK(6,S_MAC_TX_RD_THRSH)
#define V_MAC_TX_RD_THRSH(x)        _SB_MAKEVALUE(x,S_MAC_TX_RD_THRSH)
#define G_MAC_TX_RD_THRSH(x)        _SB_GETVALUE(x,S_MAC_TX_RD_THRSH,M_MAC_TX_RD_THRSH)

#define S_MAC_TX_RL_THRSH           _SB_MAKE64(16)
#define M_MAC_TX_RL_THRSH           _SB_MAKEMASK(4,S_MAC_TX_RL_THRSH)
#define V_MAC_TX_RL_THRSH(x)        _SB_MAKEVALUE(x,S_MAC_TX_RL_THRSH)
#define G_MAC_TX_RL_THRSH(x)        _SB_GETVALUE(x,S_MAC_TX_RL_THRSH,M_MAC_TX_RL_THRSH)

#define S_MAC_RX_PL_THRSH           _SB_MAKE64(24)
#define M_MAC_RX_PL_THRSH           _SB_MAKEMASK(6,S_MAC_RX_WR_THRSH)
#define V_MAC_RX_PL_THRSH(x)        _SB_MAKEVALUE(x,S_MAC_RX_PL_THRSH)
#define G_MAC_RX_PL_THRSH(x)        _SB_GETVALUE(x,S_MAC_RX_PL_THRSH,M_MAC_RX_PL_THRSH)

#define S_MAC_RX_RD_THRSH           _SB_MAKE64(32)
#define M_MAC_RX_RD_THRSH           _SB_MAKEMASK(6,S_MAC_RX_RD_THRSH)
#define V_MAC_RX_RD_THRSH(x)        _SB_MAKEVALUE(x,S_MAC_RX_RD_THRSH)
#define G_MAC_RX_RD_THRSH(x)        _SB_GETVALUE(x,S_MAC_RX_RD_THRSH,M_MAC_RX_RD_THRSH)

#define S_MAC_RX_RL_THRSH           _SB_MAKE64(40)
#define M_MAC_RX_RL_THRSH           _SB_MAKEMASK(6,S_MAC_RX_RL_THRSH)
#define V_MAC_RX_RL_THRSH(x)        _SB_MAKEVALUE(x,S_MAC_RX_RL_THRSH)
#define G_MAC_RX_RL_THRSH(x)        _SB_GETVALUE(x,S_MAC_RX_RL_THRSH,M_MAC_RX_RL_THRSH)

/*
 * MAC Frame Configuration Registers (Table 9-15)
 * Register: MAC_FRAME_CFG_0
 * Register: MAC_FRAME_CFG_1
 * Register: MAC_FRAME_CFG_2
 */

#define S_MAC_IFG_RX                _SB_MAKE64(0)
#define M_MAC_IFG_RX                _SB_MAKEMASK(6,S_MAC_IFG_RX)
#define V_MAC_IFG_RX(x)             _SB_MAKEVALUE(x,S_MAC_IFG_RX)
#define G_MAC_IFG_RX(x)             _SB_GETVALUE(x,S_MAC_IFG_RX,M_MAC_IFG_RX)

#define S_MAC_IFG_TX                _SB_MAKE64(6)
#define M_MAC_IFG_TX                _SB_MAKEMASK(6,S_MAC_IFG_TX)
#define V_MAC_IFG_TX(x)             _SB_MAKEVALUE(x,S_MAC_IFG_TX)
#define G_MAC_IFG_TX(x)             _SB_GETVALUE(x,S_MAC_IFG_TX,M_MAC_IFG_TX)

#define S_MAC_IFG_THRSH             _SB_MAKE64(12)
#define M_MAC_IFG_THRSH             _SB_MAKEMASK(6,S_MAC_IFG_THRSH)
#define V_MAC_IFG_THRSH(x)          _SB_MAKEVALUE(x,S_MAC_IFG_THRSH)
#define G_MAC_IFG_THRSH(x)          _SB_GETVALUE(x,S_MAC_IFG_THRSH,M_MAC_IFG_THRSH)

#define S_MAC_BACKOFF_SEL           _SB_MAKE64(18)
#define M_MAC_BACKOFF_SEL           _SB_MAKEMASK(4,S_MAC_BACKOFF_SEL)
#define V_MAC_BACKOFF_SEL(x)        _SB_MAKEVALUE(x,S_MAC_BACKOFF_SEL)
#define G_MAC_BACKOFF_SEL(x)        _SB_GETVALUE(x,S_MAC_BACKOFF_SEL,M_MAC_BACKOFF_SEL)

#define S_MAC_LFSR_SEED             _SB_MAKE64(22)
#define M_MAC_LFSR_SEED             _SB_MAKEMASK(8,S_MAC_LFSR_SEED)
#define V_MAC_LFSR_SEED(x)          _SB_MAKEVALUE(x,S_MAC_LFSR_SEED)
#define G_MAC_LFSR_SEED(x)          _SB_GETVALUE(x,S_MAC_LFSR_SEED,M_MAC_LFSR_SEED)

#define S_MAC_SLOT_SIZE             _SB_MAKE64(30)
#define M_MAC_SLOT_SIZE             _SB_MAKEMASK(10,S_MAC_SLOT_SIZE)
#define V_MAC_SLOT_SIZE(x)          _SB_MAKEVALUE(x,S_MAC_SLOT_SIZE)
#define G_MAC_SLOT_SIZE(x)          _SB_GETVALUE(x,S_MAC_SLOT_SIZE,M_MAC_SLOT_SIZE)

#define S_MAC_MIN_FRAMESZ           _SB_MAKE64(40)
#define M_MAC_MIN_FRAMESZ           _SB_MAKEMASK(8,S_MAC_MIN_FRAMESZ)
#define V_MAC_MIN_FRAMESZ(x)        _SB_MAKEVALUE(x,S_MAC_MIN_FRAMESZ)
#define G_MAC_MIN_FRAMESZ(x)        _SB_GETVALUE(x,S_MAC_MIN_FRAMESZ,M_MAC_MIN_FRAMESZ)

#define S_MAC_MAX_FRAMESZ           _SB_MAKE64(48)
#define M_MAC_MAX_FRAMESZ           _SB_MAKEMASK(16,S_MAC_MAX_FRAMESZ)
#define V_MAC_MAX_FRAMESZ(x)        _SB_MAKEVALUE(x,S_MAC_MAX_FRAMESZ)
#define G_MAC_MAX_FRAMESZ(x)        _SB_GETVALUE(x,S_MAC_MAX_FRAMESZ,M_MAC_MAX_FRAMESZ)

/*
 * These constants are used to configure the fields within the Frame
 * Configuration Register.  
 */

#define K_MAC_IFG_RX_10             _SB_MAKE64(18)
#define K_MAC_IFG_RX_100            _SB_MAKE64(18)
#define K_MAC_IFG_RX_1000           _SB_MAKE64(6)

#define K_MAC_IFG_TX_10             _SB_MAKE64(20)
#define K_MAC_IFG_TX_100            _SB_MAKE64(20)
#define K_MAC_IFG_TX_1000           _SB_MAKE64(8)

#define K_MAC_IFG_THRSH_10          _SB_MAKE64(12)
#define K_MAC_IFG_THRSH_100         _SB_MAKE64(12)
#define K_MAC_IFG_THRSH_1000        _SB_MAKE64(4)

#define K_MAC_SLOT_SIZE_10          _SB_MAKE64(0)
#define K_MAC_SLOT_SIZE_100         _SB_MAKE64(0)
#define K_MAC_SLOT_SIZE_1000        _SB_MAKE64(0)

#define V_MAC_IFG_RX_10        V_MAC_IFG_RX(K_MAC_IFG_RX_10)
#define V_MAC_IFG_RX_100       V_MAC_IFG_RX(K_MAC_IFG_RX_100)
#define V_MAC_IFG_RX_1000      V_MAC_IFG_RX(K_MAC_IFG_RX_1000)

#define V_MAC_IFG_TX_10        V_MAC_IFG_TX(K_MAC_IFG_TX_10)
#define V_MAC_IFG_TX_100       V_MAC_IFG_TX(K_MAC_IFG_TX_100)
#define V_MAC_IFG_TX_1000      V_MAC_IFG_TX(K_MAC_IFG_TX_1000)

#define V_MAC_IFG_THRSH_10     V_MAC_IFG_THRSH(K_MAC_IFG_THRSH_10)
#define V_MAC_IFG_THRSH_100    V_MAC_IFG_THRSH(K_MAC_IFG_THRSH_100)
#define V_MAC_IFG_THRSH_1000   V_MAC_IFG_THRSH(K_MAC_IFG_THRSH_1000)

#define V_MAC_SLOT_SIZE_10     V_MAC_SLOT_SIZE(K_MAC_SLOT_SIZE_10)
#define V_MAC_SLOT_SIZE_100    V_MAC_SLOT_SIZE(K_MAC_SLOT_SIZE_100)
#define V_MAC_SLOT_SIZE_1000   V_MAC_SLOT_SIZE(K_MAC_SLOT_SIZE_1000)

#define K_MAC_MIN_FRAMESZ_DEFAULT   _SB_MAKE64(64)
#define K_MAC_MAX_FRAMESZ_DEFAULT   _SB_MAKE64(1518)

#define V_MAC_MIN_FRAMESZ_DEFAULT   V_MAC_MIN_FRAMESZ(K_MAC_MIN_FRAMESZ_DEFAULT)
#define V_MAC_MAX_FRAMESZ_DEFAULT   V_MAC_MAX_FRAMESZ(K_MAC_MAX_FRAMESZ_DEFAULT)

/*
 * MAC VLAN Tag Registers (Table 9-16)
 * Register: MAC_VLANTAG_0
 * Register: MAC_VLANTAG_1
 * Register: MAC_VLANTAG_2
 */

/* No bit fields: lower 32 bits of register are the tags */

/*
 * MAC Status Registers (Table 9-17)
 * Also used for the MAC Interrupt Mask Register (Table 9-18)
 * Register: MAC_STATUS_0
 * Register: MAC_STATUS_1
 * Register: MAC_STATUS_2
 * Register: MAC_INT_MASK_0
 * Register: MAC_INT_MASK_1
 * Register: MAC_INT_MASK_2
 */

/* 
 * Use these constants to shift the appropriate channel
 * into the CH0 position so the same tests can be used
 * on each channel.
 */

#define S_MAC_RX_CH0                _SB_MAKE64(0)
#define S_MAC_RX_CH1                _SB_MAKE64(8)
#define S_MAC_TX_CH0                _SB_MAKE64(16)
#define S_MAC_TX_CH1                _SB_MAKE64(24)

#define S_MAC_TXCHANNELS	    _SB_MAKE64(16)	/* this is 1st TX chan */
#define S_MAC_CHANWIDTH             _SB_MAKE64(8)	/* bits between channels */

/*
 *  These are the same as RX channel 0.  The idea here
 *  is that you'll use one of the "S_" things above
 *  and pass just the six bits to a DMA-channel-specific ISR
 */
#define M_MAC_INT_CHANNEL           _SB_MAKEMASK(8,0)
#define M_MAC_INT_EOP_COUNT         _SB_MAKEMASK1(0)
#define M_MAC_INT_EOP_TIMER         _SB_MAKEMASK1(1)
#define M_MAC_INT_EOP_SEEN          _SB_MAKEMASK1(2)
#define M_MAC_INT_HWM               _SB_MAKEMASK1(3)
#define M_MAC_INT_LWM               _SB_MAKEMASK1(4)
#define M_MAC_INT_DSCR              _SB_MAKEMASK1(5)
#define M_MAC_INT_ERR               _SB_MAKEMASK1(6)
#define M_MAC_INT_DZERO             _SB_MAKEMASK1(7)	/* only for TX channels */


#define M_MAC_RX_UNDRFL             _SB_MAKEMASK1(40)
#define M_MAC_RX_OVRFL              _SB_MAKEMASK1(41)
#define M_MAC_TX_UNDRFL             _SB_MAKEMASK1(42)
#define M_MAC_TX_OVRFL              _SB_MAKEMASK1(43)
#define M_MAC_LTCOL_ERR             _SB_MAKEMASK1(44)
#define M_MAC_EXCOL_ERR             _SB_MAKEMASK1(45)
#define M_MAC_CNTR_OVRFL_ERR        _SB_MAKEMASK1(46)

#define S_MAC_COUNTER_ADDR          _SB_MAKE64(47)
#define M_MAC_COUNTER_ADDR          _SB_MAKEMASK(5,S_MAC_COUNTER_ADDR)
#define V_MAC_COUNTER_ADDR(x)       _SB_MAKEVALUE(x,S_MAC_COUNTER_ADDR)
#define G_MAC_COUNTER_ADDR(x)       _SB_GETVALUE(x,S_MAC_COUNTER_ADDR,M_MAC_COUNTER_ADDR)

/*
 * MAC Fifo Pointer Registers (Table 9-19)    [Debug register]
 * Register: MAC_FIFO_PTRS_0
 * Register: MAC_FIFO_PTRS_1
 * Register: MAC_FIFO_PTRS_2
 */

#define S_MAC_TX_WRPTR              _SB_MAKE64(0)
#define M_MAC_TX_WRPTR              _SB_MAKEMASK(6,S_MAC_TX_WRPTR)
#define V_MAC_TX_WRPTR(x)           _SB_MAKEVALUE(x,S_MAC_TX_WRPTR)
#define G_MAC_TX_WRPTR(x)           _SB_GETVALUE(x,S_MAC_TX_WRPTR,M_MAC_TX_WRPTR)

#define S_MAC_TX_RDPTR              _SB_MAKE64(8)
#define M_MAC_TX_RDPTR              _SB_MAKEMASK(6,S_MAC_TX_RDPTR)
#define V_MAC_TX_RDPTR(x)           _SB_MAKEVALUE(x,S_MAC_TX_RDPTR)
#define G_MAC_TX_RDPTR(x)           _SB_GETVALUE(x,S_MAC_TX_RDPTR,M_MAC_TX_RDPTR)

#define S_MAC_RX_WRPTR              _SB_MAKE64(16)
#define M_MAC_RX_WRPTR              _SB_MAKEMASK(6,S_MAC_RX_WRPTR)
#define V_MAC_RX_WRPTR(x)           _SB_MAKEVALUE(x,S_MAC_RX_WRPTR)
#define G_MAC_RX_WRPTR(x)           _SB_GETVALUE(x,S_MAC_RX_WRPTR,M_MAC_TX_WRPTR)

#define S_MAC_RX_RDPTR              _SB_MAKE64(24)
#define M_MAC_RX_RDPTR              _SB_MAKEMASK(6,S_MAC_RX_RDPTR)
#define V_MAC_RX_RDPTR(x)           _SB_MAKEVALUE(x,S_MAC_RX_RDPTR)
#define G_MAC_RX_RDPTR(x)           _SB_GETVALUE(x,S_MAC_RX_RDPTR,M_MAC_TX_RDPTR)

/*
 * MAC Fifo End Of Packet Count Registers (Table 9-20)  [Debug register]
 * Register: MAC_EOPCNT_0
 * Register: MAC_EOPCNT_1
 * Register: MAC_EOPCNT_2
 */

#define S_MAC_TX_EOP_COUNTER        _SB_MAKE64(0)
#define M_MAC_TX_EOP_COUNTER        _SB_MAKEMASK(6,S_MAC_TX_EOP_COUNTER)
#define V_MAC_TX_EOP_COUNTER(x)     _SB_MAKEVALUE(x,S_MAC_TX_EOP_COUNTER)
#define G_MAC_TX_EOP_COUNTER(x)     _SB_GETVALUE(x,S_MAC_TX_EOP_COUNTER,M_MAC_TX_EOP_COUNTER)

#define S_MAC_RX_EOP_COUNTER        _SB_MAKE64(8)
#define M_MAC_RX_EOP_COUNTER        _SB_MAKEMASK(6,S_MAC_RX_EOP_COUNTER)
#define V_MAC_RX_EOP_COUNTER(x)     _SB_MAKEVALUE(x,S_MAC_RX_EOP_COUNTER)
#define G_MAC_RX_EOP_COUNTER(x)     _SB_GETVALUE(x,S_MAC_RX_EOP_COUNTER,M_MAC_RX_EOP_COUNTER)

/*
 * MAC Recieve Address Filter Exact Match Registers (Table 9-21)
 * Registers: MAC_ADDR0_0 through MAC_ADDR7_0
 * Registers: MAC_ADDR0_1 through MAC_ADDR7_1
 * Registers: MAC_ADDR0_2 through MAC_ADDR7_2
 */

/* No bitfields */

/*
 * MAC Recieve Address Filter Hash Match Registers (Table 9-22)
 * Registers: MAC_HASH0_0 through MAC_HASH7_0
 * Registers: MAC_HASH0_1 through MAC_HASH7_1
 * Registers: MAC_HASH0_2 through MAC_HASH7_2
 */

/* No bitfields */

/*
 * MAC Transmit Source Address Registers (Table 9-23)
 * Register: MAC_ETHERNET_ADDR_0
 * Register: MAC_ETHERNET_ADDR_1
 * Register: MAC_ETHERNET_ADDR_2
 */

/* No bitfields */

/*
 * MAC Packet Type Configuration Register
 * Register: MAC_TYPE_CFG_0
 * Register: MAC_TYPE_CFG_1
 * Register: MAC_TYPE_CFG_2
 */

#define S_TYPECFG_TYPESIZE      _SB_MAKE64(16)

#define S_TYPECFG_TYPE0		_SB_MAKE64(0)
#define M_TYPECFG_TYPE0         _SB_MAKEMASK(16,S_TYPECFG_TYPE0)
#define V_TYPECFG_TYPE0(x)      _SB_MAKEVALUE(x,S_TYPECFG_TYPE0)
#define G_TYPECFG_TYPE0(x)      _SB_GETVALUE(x,S_TYPECFG_TYPE0,M_TYPECFG_TYPE0)

#define S_TYPECFG_TYPE1		_SB_MAKE64(0)
#define M_TYPECFG_TYPE1         _SB_MAKEMASK(16,S_TYPECFG_TYPE1)
#define V_TYPECFG_TYPE1(x)      _SB_MAKEVALUE(x,S_TYPECFG_TYPE1)
#define G_TYPECFG_TYPE1(x)      _SB_GETVALUE(x,S_TYPECFG_TYPE1,M_TYPECFG_TYPE1)

#define S_TYPECFG_TYPE2		_SB_MAKE64(0)
#define M_TYPECFG_TYPE2         _SB_MAKEMASK(16,S_TYPECFG_TYPE2)
#define V_TYPECFG_TYPE2(x)      _SB_MAKEVALUE(x,S_TYPECFG_TYPE2)
#define G_TYPECFG_TYPE2(x)      _SB_GETVALUE(x,S_TYPECFG_TYPE2,M_TYPECFG_TYPE2)

#define S_TYPECFG_TYPE3		_SB_MAKE64(0)
#define M_TYPECFG_TYPE3         _SB_MAKEMASK(16,S_TYPECFG_TYPE3)
#define V_TYPECFG_TYPE3(x)      _SB_MAKEVALUE(x,S_TYPECFG_TYPE3)
#define G_TYPECFG_TYPE3(x)      _SB_GETVALUE(x,S_TYPECFG_TYPE3,M_TYPECFG_TYPE3)

/*
 * MAC Receive Address Filter Control Registers (Table 9-24)
 * Register: MAC_ADFILTER_CFG_0
 * Register: MAC_ADFILTER_CFG_1
 * Register: MAC_ADFILTER_CFG_2
 */

#define M_MAC_ALLPKT_EN	        _SB_MAKEMASK1(0)
#define M_MAC_UCAST_EN          _SB_MAKEMASK1(1)
#define M_MAC_UCAST_INV         _SB_MAKEMASK1(2)
#define M_MAC_MCAST_EN          _SB_MAKEMASK1(3)
#define M_MAC_MCAST_INV         _SB_MAKEMASK1(4)
#define M_MAC_BCAST_EN          _SB_MAKEMASK1(5)
#define M_MAC_DIRECT_INV        _SB_MAKEMASK1(6)

#define S_MAC_IPHDR_OFFSET      _SB_MAKE64(8)
#define M_MAC_IPHDR_OFFSET      _SB_MAKEMASK(8,S_MAC_IPHDR_OFFSET)
#define V_MAC_IPHDR_OFFSET(x)	_SB_MAKEVALUE(x,S_MAC_IPHDR_OFFSET)
#define G_MAC_IPHDR_OFFSET(x)	_SB_GETVALUE(x,S_MAC_IPHDR_OFFSET,M_MAC_IPHDR_OFFSET)

/*
 * MAC Receive Channel Select Registers (Table 9-25)
 */

/* no bitfields */

/*
 * MAC MII Management Interface Registers (Table 9-26)
 * Register: MAC_MDIO_0
 * Register: MAC_MDIO_1
 * Register: MAC_MDIO_2
 */

#define S_MAC_MDC		0
#define S_MAC_MDIO_DIR		1
#define S_MAC_MDIO_OUT		2
#define S_MAC_GENC		3
#define S_MAC_MDIO_IN		4

#define M_MAC_MDC		_SB_MAKEMASK1(S_MAC_MDC)
#define M_MAC_MDIO_DIR		_SB_MAKEMASK1(S_MAC_MDIO_DIR)
#define M_MAC_MDIO_DIR_INPUT	_SB_MAKEMASK1(S_MAC_MDIO_DIR)
#define M_MAC_MDIO_OUT		_SB_MAKEMASK1(S_MAC_MDIO_OUT)
#define M_MAC_GENC		_SB_MAKEMASK1(S_MAC_GENC)
#define M_MAC_MDIO_IN		_SB_MAKEMASK1(S_MAC_MDIO_IN)


/* ********************************************************************** 
   * DUART Registers
   ********************************************************************** */

/*
 * DUART Mode Register #1 (Table 10-3)
 * Register: DUART_MODE_REG_1_A
 * Register: DUART_MODE_REG_1_B
 */

#define S_DUART_BITS_PER_CHAR       0
#define M_DUART_BITS_PER_CHAR       _SB_MAKEMASK(2,S_DUART_BITS_PER_CHAR)
#define V_DUART_BITS_PER_CHAR(x)    _SB_MAKEVALUE(x,S_DUART_BITS_PER_CHAR)

#define K_DUART_BITS_PER_CHAR_RSV0  0
#define K_DUART_BITS_PER_CHAR_RSV1  1
#define K_DUART_BITS_PER_CHAR_7     2
#define K_DUART_BITS_PER_CHAR_8     3

#define V_DUART_BITS_PER_CHAR_RSV0  V_DUART_BITS_PER_CHAR(K_DUART_BITS_PER_CHAR_RSV0)
#define V_DUART_BITS_PER_CHAR_RSV1  V_DUART_BITS_PER_CHAR(K_DUART_BITS_PER_CHAR_RSV1)
#define V_DUART_BITS_PER_CHAR_7     V_DUART_BITS_PER_CHAR(K_DUART_BITS_PER_CHAR_7)
#define V_DUART_BITS_PER_CHAR_8     V_DUART_BITS_PER_CHAR(K_DUART_BITS_PER_CHAR_8)


#define M_DUART_PARITY_TYPE_EVEN    0x00
#define M_DUART_PARITY_TYPE_ODD     _SB_MAKEMASK1(3)

#define S_DUART_PARITY_MODE          3
#define M_DUART_PARITY_MODE         _SB_MAKEMASK(2,S_DUART_PARITY_MODE)
#define V_DUART_PARITY_MODE(x)      _SB_MAKEVALUE(x,S_DUART_PARITY_MODE)

#define K_DUART_PARITY_MODE_ADD       0
#define K_DUART_PARITY_MODE_ADD_FIXED 1
#define K_DUART_PARITY_MODE_NONE      2

#define V_DUART_PARITY_MODE_ADD       V_DUART_PARITY_MODE(K_DUART_PARITY_MODE_ADD)
#define V_DUART_PARITY_MODE_ADD_FIXED V_DUART_PARITY_MODE(K_DUART_PARITY_MODE_ADD_FIXED)
#define V_DUART_PARITY_MODE_NONE      V_DUART_PARITY_MODE(K_DUART_PARITY_MODE_NONE)

#define M_DUART_ERR_MODE            _SB_MAKEMASK1(5)    /* must be zero */

#define M_DUART_RX_IRQ_SEL_RXRDY    0
#define M_DUART_RX_IRQ_SEL_RXFULL   _SB_MAKEMASK1(6)

#define M_DUART_RX_RTS_ENA          _SB_MAKEMASK1(7)

/*
 * DUART Mode Register #2 (Table 10-4)
 * Register: DUART_MODE_REG_2_A
 * Register: DUART_MODE_REG_2_B
 */

#define M_DUART_MODE_RESERVED1      _SB_MAKEMASK(3,0)   /* ignored */

#define M_DUART_STOP_BIT_LEN_2      _SB_MAKEMASK1(3)
#define M_DUART_STOP_BIT_LEN_1      0

#define M_DUART_TX_CTS_ENA          _SB_MAKEMASK1(4)

#define M_DUART_MODE_RESERVED2      _SB_MAKEMASK1(5)    /* must be zero */

#define S_DUART_CHAN_MODE	    6
#define M_DUART_CHAN_MODE           _SB_MAKEMASK(2,S_DUART_CHAN_MODE)
#define V_DUART_CHAN_MODE(x)	    _SB_MAKEVALUE(x,S_DUART_CHAN_MODE)

#define K_DUART_CHAN_MODE_NORMAL    0
#define K_DUART_CHAN_MODE_LCL_LOOP  2
#define K_DUART_CHAN_MODE_REM_LOOP  3

#define V_DUART_CHAN_MODE_NORMAL    V_DUART_CHAN_MODE(K_DUART_CHAN_MODE_NORMAL)
#define V_DUART_CHAN_MODE_LCL_LOOP  V_DUART_CHAN_MODE(K_DUART_CHAN_MODE_LCL_LOOP)
#define V_DUART_CHAN_MODE_REM_LOOP  V_DUART_CHAN_MODE(K_DUART_CHAN_MODE_REM_LOOP)

/*
 * DUART Command Register (Table 10-5)
 * Register: DUART_CMD_A
 * Register: DUART_CMD_B
 */

#define M_DUART_RX_EN               _SB_MAKEMASK1(0)
#define M_DUART_RX_DIS              _SB_MAKEMASK1(1)
#define M_DUART_TX_EN               _SB_MAKEMASK1(2)
#define M_DUART_TX_DIS              _SB_MAKEMASK1(3)

#define S_DUART_MISC_CMD	    4
#define M_DUART_MISC_CMD            _SB_MAKEMASK(3,S_DUART_MISC_CMD)
#define V_DUART_MISC_CMD(x)         _SB_MAKEVALUE(x,S_DUART_MISC_CMD)

#define K_DUART_MISC_CMD_NOACTION0       0
#define K_DUART_MISC_CMD_NOACTION1       1
#define K_DUART_MISC_CMD_RESET_RX        2
#define K_DUART_MISC_CMD_RESET_TX        3
#define K_DUART_MISC_CMD_NOACTION4       4
#define K_DUART_MISC_CMD_RESET_BREAK_INT 5
#define K_DUART_MISC_CMD_START_BREAK     6
#define K_DUART_MISC_CMD_STOP_BREAK      7

#define V_DUART_MISC_CMD_NOACTION0       V_DUART_MISC_CMD(K_DUART_MISC_CMD_NOACTION0)
#define V_DUART_MISC_CMD_NOACTION1       V_DUART_MISC_CMD(K_DUART_MISC_CMD_NOACTION1)
#define V_DUART_MISC_CMD_RESET_RX        V_DUART_MISC_CMD(K_DUART_MISC_CMD_RESET_RX)
#define V_DUART_MISC_CMD_RESET_TX        V_DUART_MISC_CMD(K_DUART_MISC_CMD_RESET_TX)
#define V_DUART_MISC_CMD_NOACTION4       V_DUART_MISC_CMD(K_DUART_MISC_CMD_NOACTION4)
#define V_DUART_MISC_CMD_RESET_BREAK_INT V_DUART_MISC_CMD(K_DUART_MISC_CMD_RESET_BREAK_INT)
#define V_DUART_MISC_CMD_START_BREAK     V_DUART_MISC_CMD(K_DUART_MISC_CMD_START_BREAK)
#define V_DUART_MISC_CMD_STOP_BREAK      V_DUART_MISC_CMD(K_DUART_MISC_CMD_STOP_BREAK)

#define M_DUART_CMD_RESERVED             _SB_MAKEMASK1(7) 

/*
 * DUART Status Register (Table 10-6)
 * Register: DUART_STATUS_A
 * Register: DUART_STATUS_B
 * READ-ONLY
 */

#define M_DUART_RX_RDY              _SB_MAKEMASK1(0)
#define M_DUART_RX_FFUL             _SB_MAKEMASK1(1)
#define M_DUART_TX_RDY              _SB_MAKEMASK1(2)
#define M_DUART_TX_EMT              _SB_MAKEMASK1(3)
#define M_DUART_OVRUN_ERR           _SB_MAKEMASK1(4)
#define M_DUART_PARITY_ERR          _SB_MAKEMASK1(5)
#define M_DUART_FRM_ERR             _SB_MAKEMASK1(6)
#define M_DUART_RCVD_BRK            _SB_MAKEMASK1(7)

/*
 * DUART Baud Rate Register (Table 10-7)
 * Register: DUART_CLK_SEL_A 
 * Register: DUART_CLK_SEL_B
 */

#define M_DUART_CLK_COUNTER         _SB_MAKEMASK(12,0)
#define V_DUART_BAUD_RATE(x)        (100000000/((x)*20)-1)

/*
 * DUART Data Registers (Table 10-8 and 10-9)
 * Register: DUART_RX_HOLD_A
 * Register: DUART_RX_HOLD_B
 * Register: DUART_TX_HOLD_A
 * Register: DUART_TX_HOLD_B
 */

#define M_DUART_RX_DATA             _SB_MAKEMASK(8,0)
#define M_DUART_TX_DATA             _SB_MAKEMASK(8,0)

/*
 * DUART Input Port Register (Table 10-10)
 * Register: DUART_IN_PORT
 */

#define M_DUART_IN_PIN0_VAL         _SB_MAKEMASK1(0)
#define M_DUART_IN_PIN1_VAL         _SB_MAKEMASK1(1)
#define M_DUART_IN_PIN2_VAL         _SB_MAKEMASK1(2)
#define M_DUART_IN_PIN3_VAL         _SB_MAKEMASK1(3)
#define M_DUART_IN_PIN4_VAL         _SB_MAKEMASK1(4)
#define M_DUART_IN_PIN5_VAL         _SB_MAKEMASK1(5)
#define M_DUART_RIN0_PIN            _SB_MAKEMASK1(6)
#define M_DUART_RIN1_PIN            _SB_MAKEMASK1(7)

/*
 * DUART Input Port Change Status Register (Tables 10-11, 10-12, and 10-13)
 * Register: DUART_INPORT_CHNG
 */

#define S_DUART_IN_PIN_VAL          0
#define M_DUART_IN_PIN_VAL          _SB_MAKEMASK(4,S_DUART_IN_PIN_VAL)

#define S_DUART_IN_PIN_CHNG         4
#define M_DUART_IN_PIN_CHNG         _SB_MAKEMASK(4,S_DUART_IN_PIN_CHNG)

/*
 * DUART Output port control register (Table 10-14)
 * Register: DUART_OPCR
 */

#define M_DUART_OPCR_RESERVED0      _SB_MAKEMASK1(0)   /* must be zero */
#define M_DUART_OPC2_SEL            _SB_MAKEMASK1(1)
#define M_DUART_OPCR_RESERVED1      _SB_MAKEMASK1(2)   /* must be zero */
#define M_DUART_OPC3_SEL            _SB_MAKEMASK1(3)
#define M_DUART_OPCR_RESERVED2      _SB_MAKEMASK(4,4)  /* must be zero */

/*
 * DUART Aux Control Register (Table 10-15)
 * Register: DUART_AUX_CTRL
 */

#define M_DUART_IP0_CHNG_ENA        _SB_MAKEMASK1(0)
#define M_DUART_IP1_CHNG_ENA        _SB_MAKEMASK1(1)
#define M_DUART_IP2_CHNG_ENA        _SB_MAKEMASK1(2)
#define M_DUART_IP3_CHNG_ENA        _SB_MAKEMASK1(3)
#define M_DUART_ACR_RESERVED        _SB_MAKEMASK(4,4)

/*
 * DUART Interrupt Status Register (Table 10-16)
 * Register: DUART_ISR
 */

#define M_DUART_ISR_TX_A            _SB_MAKEMASK1(0)
#define M_DUART_ISR_RX_A            _SB_MAKEMASK1(1)
#define M_DUART_ISR_BRK_A           _SB_MAKEMASK1(2)
#define M_DUART_ISR_IN_A            _SB_MAKEMASK1(3)
#define M_DUART_ISR_TX_B            _SB_MAKEMASK1(4)
#define M_DUART_ISR_RX_B            _SB_MAKEMASK1(5)
#define M_DUART_ISR_BRK_B           _SB_MAKEMASK1(6)
#define M_DUART_ISR_IN_B            _SB_MAKEMASK1(7)

/*
 * DUART Channel A Interrupt Status Register (Table 10-17)
 * DUART Channel B Interrupt Status Register (Table 10-18)
 * Register: DUART_ISR_A
 * Register: DUART_ISR_B
 */

#define M_DUART_ISR_TX              _SB_MAKEMASK1(0)
#define M_DUART_ISR_RX              _SB_MAKEMASK1(1)
#define M_DUART_ISR_BRK             _SB_MAKEMASK1(2)
#define M_DUART_ISR_IN              _SB_MAKEMASK1(3)
#define M_DUART_ISR_RESERVED        _SB_MAKEMASK(4,4)

/*
 * DUART Interrupt Mask Register (Table 10-19)
 * Register: DUART_IMR
 */

#define M_DUART_IMR_TX_A            _SB_MAKEMASK1(0)
#define M_DUART_IMR_RX_A            _SB_MAKEMASK1(1)
#define M_DUART_IMR_BRK_A           _SB_MAKEMASK1(2)
#define M_DUART_IMR_IN_A            _SB_MAKEMASK1(3)
#define M_DUART_IMR_ALL_A	    _SB_MAKEMASK(4,4)

#define M_DUART_IMR_TX_B            _SB_MAKEMASK1(4)
#define M_DUART_IMR_RX_B            _SB_MAKEMASK1(5)
#define M_DUART_IMR_BRK_B           _SB_MAKEMASK1(5)
#define M_DUART_IMR_IN_B            _SB_MAKEMASK1(7)
#define M_DUART_IMR_ALL_B           _SB_MAKEMASK(4,4)

/*
 * DUART Channel A Interrupt Mask Register (Table 10-20)
 * DUART Channel B Interrupt Mask Register (Table 10-21)
 * Register: DUART_IMR_A
 * Register: DUART_IMR_B
 */

#define M_DUART_IMR_TX              _SB_MAKEMASK1(0)
#define M_DUART_IMR_RX              _SB_MAKEMASK1(1)
#define M_DUART_IMR_BRK             _SB_MAKEMASK1(2)
#define M_DUART_IMR_IN              _SB_MAKEMASK1(3)
#define M_DUART_IMR_ALL		    _SB_MAKEMASK(4,0)
#define M_DUART_IMR_RESERVED        _SB_MAKEMASK(4,4)


/*
 * DUART Output Port Set Register (Table 10-22)
 * Register: DUART_SET_OPR
 */

#define M_DUART_SET_OPR0            _SB_MAKEMASK1(0)
#define M_DUART_SET_OPR1            _SB_MAKEMASK1(1)
#define M_DUART_SET_OPR2            _SB_MAKEMASK1(2)
#define M_DUART_SET_OPR3            _SB_MAKEMASK1(3)
#define M_DUART_OPSR_RESERVED       _SB_MAKEMASK(4,4)

/*
 * DUART Output Port Clear Register (Table 10-23)
 * Register: DUART_CLEAR_OPR
 */

#define M_DUART_CLR_OPR0            _SB_MAKEMASK1(0)
#define M_DUART_CLR_OPR1            _SB_MAKEMASK1(1)
#define M_DUART_CLR_OPR2            _SB_MAKEMASK1(2)
#define M_DUART_CLR_OPR3            _SB_MAKEMASK1(3)
#define M_DUART_OPCR_RESERVED       _SB_MAKEMASK(4,4)

/*
 * DUART Output Port RTS Register (Table 10-24)
 * Register: DUART_OUT_PORT
 */

#define M_DUART_OUT_PIN_SET0        _SB_MAKEMASK1(0)
#define M_DUART_OUT_PIN_SET1        _SB_MAKEMASK1(1)
#define M_DUART_OUT_PIN_CLR0        _SB_MAKEMASK1(2)
#define M_DUART_OUT_PIN_CLR1        _SB_MAKEMASK1(3)
#define M_DUART_OPRR_RESERVED       _SB_MAKEMASK(4,4)

#define M_DUART_OUT_PIN_SET(chan) \
    (chan == 0 ? M_DUART_OUT_PIN_SET0 : M_DUART_OUT_PIN_SET1)
#define M_DUART_OUT_PIN_CLR(chan) \
    (chan == 0 ? M_DUART_OUT_PIN_CLR0 : M_DUART_OUT_PIN_CLR1)

/*
 * To be added: Synchronous Serial definitions
 */


/* ********************************************************************** 
   * Generic Bus constants
   ********************************************************************** */

/*
 * Generic Bus Region Configuration Registers (Table 11-4)
 */

#define M_IO_RDY_ACTIVE		_SB_MAKEMASK1(0)
#define M_IO_ENA_RDY		_SB_MAKEMASK1(1)

#define S_IO_WIDTH_SEL		2
#define M_IO_WIDTH_SEL		_SB_MAKEMASK(2,S_IO_WIDTH_SEL)
#define K_IO_WIDTH_SEL_1	0
#define K_IO_WIDTH_SEL_2	1
#define K_IO_WIDTH_SEL_4	3
#define V_IO_WIDTH_SEL(x)	_SB_MAKEVALUE(x,S_IO_WIDTH_SEL)
#define G_IO_WIDTH_SEL(x)	_SB_GETVALUE(x,S_IO_WIDTH_SEL,M_IO_WIDTH_SEL)

#define M_IO_PARITY_ENA		_SB_MAKEMASK1(4)
#define M_IO_PARITY_ODD		_SB_MAKEMASK1(6)
#define M_IO_NONMUX		_SB_MAKEMASK1(7)

#define S_IO_TIMEOUT		8
#define M_IO_TIMEOUT		_SB_MAKEMASK(8,S_IO_TIMEOUT)
#define V_IO_TIMEOUT(x)		_SB_MAKEVALUE(x,S_IO_TIMEOUT)
#define G_IO_TIMEOUT(x)		_SB_GETVALUE(x,S_IO_TIMEOUT,M_IO_TIMEOUT)

/*
 * Generic Bus Region Size register (Table 11-5)
 */

#define S_IO_MULT_SIZE		0
#define M_IO_MULT_SIZE		_SB_MAKEMASK(12,S_IO_MULT_SIZE)
#define V_IO_MULT_SIZE(x)	_SB_MAKEVALUE(x,S_IO_MULT_SIZE)
#define G_IO_MULT_SIZE(x)	_SB_GETVALUE(x,S_IO_MULT_SIZE,M_IO_MULT_SIZE)

#define S_IO_REGSIZE		16	 /* # bits to shift size for this reg */

/*
 * Generic Bus Region Address (Table 11-6)
 */

#define S_IO_START_ADDR		0
#define M_IO_START_ADDR		_SB_MAKEMASK(14,S_IO_START_ADDR)
#define V_IO_START_ADDR(x)	_SB_MAKEVALUE(x,S_IO_START_ADDR)
#define G_IO_START_ADDR(x)	_SB_GETVALUE(x,S_IO_START_ADDR,M_IO_START_ADDR)

#define S_IO_ADDRBASE		16	 /* # bits to shift addr for this reg */

/*
 * Generic Bus Region 0 Timing Registers (Table 11-7)
 */

#define S_IO_ALE_WIDTH		0
#define M_IO_ALE_WIDTH		_SB_MAKEMASK(3,S_IO_ALE_WIDTH)
#define V_IO_ALE_WIDTH(x)	_SB_MAKEVALUE(x,S_IO_ALE_WIDTH)
#define G_IO_ALE_WIDTH(x)	_SB_GETVALUE(x,S_IO_ALE_WIDTH,M_IO_ALE_WIDTH)

#define S_IO_ALE_TO_CS		4
#define M_IO_ALE_TO_CS		_SB_MAKEMASK(2,S_IO_ALE_TO_CS)
#define V_IO_ALE_TO_CS(x)	_SB_MAKEVALUE(x,S_IO_ALE_TO_CS)
#define G_IO_ALE_TO_CS(x)	_SB_GETVALUE(x,S_IO_ALE_TO_CS,M_IO_ALE_TO_CS)

#define S_IO_CS_WIDTH		8
#define M_IO_CS_WIDTH		_SB_MAKEMASK(5,S_IO_CS_WIDTH)
#define V_IO_CS_WIDTH(x)	_SB_MAKEVALUE(x,S_IO_CS_WIDTH)
#define G_IO_CS_WIDTH(x)	_SB_GETVALUE(x,S_IO_CS_WIDTH,M_IO_CS_WIDTH)

#define S_IO_RDY_SMPLE		13
#define M_IO_RDY_SMPLE		_SB_MAKEMASK(3,S_IO_RDY_SMPLE)
#define V_IO_RDY_SMPLE(x)	_SB_MAKEVALUE(x,S_IO_RDY_SMPLE)
#define G_IO_RDY_SMPLE(x)	_SB_GETVALUE(x,S_IO_RDY_SMPLE,M_IO_RDY_SMPLE)


/*
 * Generic Bus Timing 1 Registers (Table 11-8)
 */

#define S_IO_ALE_TO_WRITE	0
#define M_IO_ALE_TO_WRITE	_SB_MAKEMASK(3,S_IO_ALE_TO_WRITE)
#define V_IO_ALE_TO_WRITE(x)	_SB_MAKEVALUE(x,S_IO_ALE_TO_WRITE)
#define G_IO_ALE_TO_WRITE(x)	_SB_GETVALUE(x,S_IO_ALE_TO_WRITE,M_IO_ALE_TO_WRITE)

#define S_IO_WRITE_WIDTH	4
#define M_IO_WRITE_WIDTH	_SB_MAKEMASK(4,S_IO_WRITE_WIDTH)
#define V_IO_WRITE_WIDTH(x)	_SB_MAKEVALUE(x,S_IO_WRITE_WIDTH)
#define G_IO_WRITE_WIDTH(x)	_SB_GETVALUE(x,S_IO_WRITE_WIDTH,M_IO_WRITE_WIDTH)

#define S_IO_IDLE_CYCLE		8
#define M_IO_IDLE_CYCLE		_SB_MAKEMASK(4,S_IO_IDLE_CYCLE)
#define V_IO_IDLE_CYCLE(x)	_SB_MAKEVALUE(x,S_IO_IDLE_CYCLE)
#define G_IO_IDLE_CYCLE(x)	_SB_GETVALUE(x,S_IO_IDLE_CYCLE,M_IO_IDLE_CYCLE)

#define S_IO_CS_TO_OE		12
#define M_IO_CS_TO_OE		_SB_MAKEMASK(2,S_IO_CS_TO_OE)
#define V_IO_CS_TO_OE(x)	_SB_MAKEVALUE(x,S_IO_CS_TO_OE)
#define G_IO_CS_TO_OE(x)	_SB_GETVALUE(x,S_IO_CS_TO_OE,M_IO_CS_TO_OE)

#define S_IO_OE_TO_CS		14
#define M_IO_OE_TO_CS		_SB_MAKEMASK(2,S_IO_OE_TO_CS)
#define V_IO_OE_TO_CS(x)	_SB_MAKEVALUE(x,S_IO_OE_TO_CS)
#define G_IO_OE_TO_CS(x)	_SB_GETVALUE(x,S_IO_OE_TO_CS,M_IO_OE_TO_CS)

/*
 * Generic Bus Interrupt Status Register (Table 11-9)
 */

#define M_IO_CS_ERR_INT		_SB_MAKEMASK(0,8)
#define M_IO_CS0_ERR_INT	_SB_MAKEMASK1(0)
#define M_IO_CS1_ERR_INT	_SB_MAKEMASK1(1)
#define M_IO_CS2_ERR_INT	_SB_MAKEMASK1(2)
#define M_IO_CS3_ERR_INT	_SB_MAKEMASK1(3)
#define M_IO_CS4_ERR_INT	_SB_MAKEMASK1(4)
#define M_IO_CS5_ERR_INT	_SB_MAKEMASK1(5)
#define M_IO_CS6_ERR_INT	_SB_MAKEMASK1(6)
#define M_IO_CS7_ERR_INT	_SB_MAKEMASK1(7)

#define M_IO_RD_PAR_INT		_SB_MAKEMASK1(9)
#define M_IO_TIMEOUT_INT	_SB_MAKEMASK1(10)
#define M_IO_ILL_ADDR_INT	_SB_MAKEMASK1(11)
#define M_IO_MULT_CS_INT	_SB_MAKEMASK1(12)

/*
 * PCMCIA configuration register (Table 12-6)
 */

#define M_PCMCIA_CFG_ATTRMEM	_SB_MAKEMASK1(0)
#define M_PCMCIA_CFG_3VEN	_SB_MAKEMASK1(1)
#define M_PCMCIA_CFG_5VEN	_SB_MAKEMASK1(2)
#define M_PCMCIA_CFG_VPPEN	_SB_MAKEMASK1(3)
#define M_PCMCIA_CFG_RESET	_SB_MAKEMASK1(4)
#define M_PCMCIA_CFG_APWRONEN	_SB_MAKEMASK1(5)
#define M_PCMCIA_CFG_CDMASK	_SB_MAKEMASK1(6)
#define M_PCMCIA_CFG_WPMASK	_SB_MAKEMASK1(7)
#define M_PCMCIA_CFG_RDYMASK	_SB_MAKEMASK1(8)
#define M_PCMCIA_CFG_PWRCTL	_SB_MAKEMASK1(9)

/*
 * PCMCIA status register (Table 12-7)
 */

#define M_PCMCIA_STATUS_CD1	_SB_MAKEMASK1(0)
#define M_PCMCIA_STATUS_CD2	_SB_MAKEMASK1(1)
#define M_PCMCIA_STATUS_VS1	_SB_MAKEMASK1(2)
#define M_PCMCIA_STATUS_VS2	_SB_MAKEMASK1(3)
#define M_PCMCIA_STATUS_WP	_SB_MAKEMASK1(4)
#define M_PCMCIA_STATUS_RDY	_SB_MAKEMASK1(5)
#define M_PCMCIA_STATUS_3VEN	_SB_MAKEMASK1(6)
#define M_PCMCIA_STATUS_5VEN	_SB_MAKEMASK1(7)
#define M_PCMCIA_STATUS_CDCHG	_SB_MAKEMASK1(8)
#define M_PCMCIA_STATUS_WPCHG	_SB_MAKEMASK1(9)
#define M_PCMCIA_STATUS_RDYCHG	_SB_MAKEMASK1(10)

/*
 * GPIO Interrupt Type Register (table 13-3)
 */

#define K_GPIO_INTR_DISABLE	0
#define K_GPIO_INTR_EDGE	1
#define K_GPIO_INTR_LEVEL	2
#define K_GPIO_INTR_SPLIT	3

#define S_GPIO_INTR_TYPEX(n)	(((n)/2)*2)
#define M_GPIO_INTR_TYPEX(n)	_SB_MAKEMASK(2,S_GPIO_INTR_TYPEX(n))
#define V_GPIO_INTR_TYPEX(n,x)	_SB_MAKEVALUE(x,S_GPIO_INTR_TYPEX(n))
#define G_GPIO_INTR_TYPEX(n,x)	_SB_GETVALUE(x,S_GPIO_INTR_TYPEX(n),M_GPIO_INTR_TYPEX(n))

#define S_GPIO_INTR_TYPE0	0
#define M_GPIO_INTR_TYPE0	_SB_MAKEMASK(2,S_GPIO_INTR_TYPE0)
#define V_GPIO_INTR_TYPE0(x)	_SB_MAKEVALUE(x,S_GPIO_INTR_TYPE0)
#define G_GPIO_INTR_TYPE0(x)	_SB_GETVALUE(x,S_GPIO_INTR_TYPE0,M_GPIO_INTR_TYPE0)

#define S_GPIO_INTR_TYPE2	2
#define M_GPIO_INTR_TYPE2	_SB_MAKEMASK(2,S_GPIO_INTR_TYPE2)
#define V_GPIO_INTR_TYPE2(x)	_SB_MAKEVALUE(x,S_GPIO_INTR_TYPE2)
#define G_GPIO_INTR_TYPE2(x)	_SB_GETVALUE(x,S_GPIO_INTR_TYPE2,M_GPIO_INTR_TYPE2)

#define S_GPIO_INTR_TYPE4	4
#define M_GPIO_INTR_TYPE4	_SB_MAKEMASK(2,S_GPIO_INTR_TYPE4)
#define V_GPIO_INTR_TYPE4(x)	_SB_MAKEVALUE(x,S_GPIO_INTR_TYPE4)
#define G_GPIO_INTR_TYPE4(x)	_SB_GETVALUE(x,S_GPIO_INTR_TYPE4,M_GPIO_INTR_TYPE4)

#define S_GPIO_INTR_TYPE6	6
#define M_GPIO_INTR_TYPE6	_SB_MAKEMASK(2,S_GPIO_INTR_TYPE6)
#define V_GPIO_INTR_TYPE6(x)	_SB_MAKEVALUE(x,S_GPIO_INTR_TYPE6)
#define G_GPIO_INTR_TYPE6(x)	_SB_GETVALUE(x,S_GPIO_INTR_TYPE6,M_GPIO_INTR_TYPE6)

#define S_GPIO_INTR_TYPE8	8
#define M_GPIO_INTR_TYPE8	_SB_MAKEMASK(2,S_GPIO_INTR_TYPE8)
#define V_GPIO_INTR_TYPE8(x)	_SB_MAKEVALUE(x,S_GPIO_INTR_TYPE8)
#define G_GPIO_INTR_TYPE8(x)	_SB_GETVALUE(x,S_GPIO_INTR_TYPE8,M_GPIO_INTR_TYPE8)

#define S_GPIO_INTR_TYPE10	10
#define M_GPIO_INTR_TYPE10	_SB_MAKEMASK(2,S_GPIO_INTR_TYPE10)
#define V_GPIO_INTR_TYPE10(x)	_SB_MAKEVALUE(x,S_GPIO_INTR_TYPE10)
#define G_GPIO_INTR_TYPE10(x)	_SB_GETVALUE(x,S_GPIO_INTR_TYPE10,M_GPIO_INTR_TYPE10)

#define S_GPIO_INTR_TYPE12	12
#define M_GPIO_INTR_TYPE12	_SB_MAKEMASK(2,S_GPIO_INTR_TYPE12)
#define V_GPIO_INTR_TYPE12(x)	_SB_MAKEVALUE(x,S_GPIO_INTR_TYPE12)
#define G_GPIO_INTR_TYPE12(x)	_SB_GETVALUE(x,S_GPIO_INTR_TYPE12,M_GPIO_INTR_TYPE12)

#define S_GPIO_INTR_TYPE14	14
#define M_GPIO_INTR_TYPE14	_SB_MAKEMASK(2,S_GPIO_INTR_TYPE14)
#define V_GPIO_INTR_TYPE14(x)	_SB_MAKEVALUE(x,S_GPIO_INTR_TYPE14)
#define G_GPIO_INTR_TYPE14(x)	_SB_GETVALUE(x,S_GPIO_INTR_TYPE14,M_GPIO_INTR_TYPE14)


/* ********************************************************************** 
   * System Management Bus constants
   ********************************************************************** */

/*
 * SMBus Clock Frequency Register (Table 14-2)
 */

#define S_SMB_FREQ_DIV              0
#define M_SMB_FREQ_DIV              _SB_MAKEMASK(13,S_SMB_FREQ_DIV)
#define V_SMB_FREQ_DIV(x)           _SB_MAKEVALUE(x,S_SMB_FREQ_DIV)

#define K_SMB_FREQ_400KHZ	    0x1F
#define K_SMB_FREQ_100KHZ	    0x7D

#define S_SMB_CMD                   0
#define M_SMB_CMD                   _SB_MAKEMASK(8,S_SMB_CMD)
#define V_SMB_CMD(x)                _SB_MAKEVALUE(x,S_SMB_CMD)

/*
 * SMBus control register (Table 14-4)
 */

#define M_SMB_ERR_INTR              _SB_MAKEMASK1(0)
#define M_SMB_FINISH_INTR           _SB_MAKEMASK1(1)
#define M_SMB_DATA_OUT              _SB_MAKEMASK1(4)
#define M_SMB_DATA_DIR              _SB_MAKEMASK1(5)
#define M_SMB_DATA_DIR_OUTPUT       M_SMB_DATA_DIR
#define M_SMB_CLK_OUT               _SB_MAKEMASK1(6)
#define M_SMB_DIRECT_ENABLE         _SB_MAKEMASK1(7)

/*
 * SMBus status registers (Table 14-5)
 */

#define M_SMB_BUSY                  _SB_MAKEMASK1(0)
#define M_SMB_ERROR                 _SB_MAKEMASK1(1)
#define M_SMB_ERROR_TYPE            _SB_MAKEMASK1(2)
#define M_SMB_REF                   _SB_MAKEMASK1(6)
#define M_SMB_DATA_IN               _SB_MAKEMASK1(7)

/*
 * SMBus Start/Command registers (Table 14-9)
 */

#define S_SMB_ADDR                  0
#define M_SMB_ADDR                  _SB_MAKEMASK(7,S_SMB_ADDR)
#define V_SMB_ADDR(x)               _SB_MAKEVALUE(x,S_SMB_ADDR)
#define G_SMB_ADDR(x)               _SB_GETVALUE(x,S_SMB_ADDR,M_SMB_ADDR)

#define M_SMB_QDATA                 _SB_MAKEMASK1(7)

#define S_SMB_TT                    8
#define M_SMB_TT                    _SB_MAKEMASK(3,S_SMB_TT)
#define V_SMB_TT(x)                 _SB_MAKEVALUE(x,S_SMB_TT)
#define G_SMB_TT(x)                 _SB_GETVALUE(x,S_SMB_TT,M_SMB_TT)

#define K_SMB_TT_WR1BYTE            0
#define K_SMB_TT_WR2BYTE            1
#define K_SMB_TT_WR3BYTE            2
#define K_SMB_TT_CMD_RD1BYTE        3
#define K_SMB_TT_CMD_RD2BYTE        4
#define K_SMB_TT_RD1BYTE            5
#define K_SMB_TT_QUICKCMD           6
#define K_SMB_TT_EEPROMREAD         7

#define V_SMB_TT_WR1BYTE	    V_SMB_TT(K_SMB_TT_WR1BYTE)
#define V_SMB_TT_WR2BYTE	    V_SMB_TT(K_SMB_TT_WR2BYTE)
#define V_SMB_TT_WR3BYTE	    V_SMB_TT(K_SMB_TT_WR3BYTE)
#define V_SMB_TT_CMD_RD1BYTE	    V_SMB_TT(K_SMB_TT_CMD_RD1BYTE)
#define V_SMB_TT_CMD_RD2BYTE	    V_SMB_TT(K_SMB_TT_CMD_RD2BYTE)
#define V_SMB_TT_RD1BYTE	    V_SMB_TT(K_SMB_TT_RD1BYTE)
#define V_SMB_TT_QUICKCMD	    V_SMB_TT(K_SMB_TT_QUICKCMD)
#define V_SMB_TT_EEPROMREAD	    V_SMB_TT(K_SMB_TT_EEPROMREAD)

#define M_SMB_PEC                   _SB_MAKEMASK1(15)

/*
 * SMBus Data Register (Table 14-6) and SMBus Extra Register (Table 14-7)
 */

#define S_SMB_LB                    0
#define M_SMB_LB                    _SB_MAKEMASK(8,S_SMB_LB)
#define V_SMB_LB(x)                 _SB_MAKEVALUE(x,S_SMB_LB)

#define S_SMB_MB                    8
#define M_SMB_MB                    _SB_MAKEMASK(8,S_SMB_MB)
#define V_SMB_MB(x)                 _SB_MAKEVALUE(x,S_SMB_MB)


/*
 * SMBus Packet Error Check register (Table 14-8)
 */

#define S_SPEC_PEC                  0
#define M_SPEC_PEC                  _SB_MAKEMASK(8,S_SPEC_PEC)
#define V_SPEC_MB(x)                _SB_MAKEVALUE(x,S_SPEC_PEC)

#endif /* __INCbcm1250Libh */
