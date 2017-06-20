/*
 * $Id: memreg.h,v 1.10 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 */
#ifndef _MEMREG_
#define _MEMREG_

/*
 * Remove those not needed
 */
#define BCM_56504_A0
#define BCM_56504_B0
#define BCM_56218_A0
#define BCM_ALL_CHIPS

/* #define DEBUG */

#include "chips/allenum.h"

#define BROADCOM_VENDOR_ID      0x14e4

#define BCM56218_DEVICE_ID      0xb218
#define BCM56218_A0_REV_ID      1

/* CMIC Configuration Register */
/* RD_BRST_EN defaults to 1 for Lynx */

#define CMIC_CONFIG                     0x0000010C

#define CC_RD_BRST_EN                   0x00000001 /* enb PCI read bursts */
#define CC_WR_BRST_EN                   0x00000002 /* enb PCI write bursts */
#define CC_BE_CHECK_EN                  0x00000004 /* Big endian chk PCI Wr */
#define CC_MSTR_Q_MAX_EN                0x00000008 /* Queue 4 PCI mastr reqs */
#define CC_LINK_STAT_EN                 0x00000010 /* enb linkstat autoupd */
#define CC_RESET_CPS                    0x00000020 /* Drive CPS bus reset */
#define CC_ACT_LOW_INT                  0x00000040 /* INTA# is active low */
#define CC_SCHAN_ABORT                  0x00000080 /* abort S-Channel opern */
#define CC_STACK_ARCH_EN                0x00000100 /* (see databook) */
#define CC_UNTAG_EN                     0x00000200 /* (see databook) */
#define CC_LE_DMA_EN                    0x00000400 /* Little endian DMA fmt */
#define CC_I2C_EN                       0x00000800 /* enb CPU access to I2C */
#define CC_LINK_SCAN_GIG                0x00001000 /* Also scan gig ports */
#define CC_DMA_GARBAGE_COLL_EN          0x00008000 /* Collect 'purge' pkts */
#define CC_ALN_OPN_EN                   0x00002000 /* Unaligned DMA enable */
#define CC_PCI_RST_ON_INIT_EN           0x00010000
#define CC_DIS_TIME_STAMP_CTR           0x00020000
#define CC_SG_OPN_EN                    0x00040000 /* DMA S/G enable */
#define CC_RLD_OPN_EN                   0x00080000 /* DMA RLD enable */
#define CC_DIS_RLD_STAT_UPDATE          0x00100000
#define CC_STOP_AUTO_SCAN_ON_LCHG       0x00200000 /* LS stop in intr */
#define CC_ABORT_STAT_DMA               0x00400000
#define CC_ATO_SCAN_FROM_ID_ZERO        0x00800000 /* Scan from PHYID=0 */
#define CC_COS_QUALIFIED_DMA_RX_EN      0x01000000
#define CC_ABORT_TBL_DMA                0x02000000 /* Abort Table DMA op */
#define CC_EXT_MDIO_MSTR_DIS            0x04000000 /* Disable MDIO on ATE */
#define CC_EXTENDED_DCB_ENABLE          0x08000000 /* type7 dcb (5695) */
#define CC_INT_PHY_CLAUSE_45            0x08000000 /* MIIM 45 vs. 22 (5673) */

#define N_DMA_CHAN                      4

/* CMIC DMA Memory Address Registers (4 base addresses for 0 <= ch <= 3) */
#define CMIC_DMA_DESC(ch)               (0x00000110 + 4 * (ch))

/* CMIC I2C Registers */
#define CMIC_I2C_SLAVE_ADDR             0x00000120
#define CMIC_I2C_DATA                   0x00000124
#define CMIC_I2C_CTRL                   0x00000128
#define CMIC_I2C_STAT                   0x0000012C
#define CMIC_I2C_SLAVE_XADDR            0x00000130
#define CMIC_I2C_GP0                    0x00000134
#define CMIC_I2C_GP1                    0x00000138
#define CMIC_I2C_RESET                  0x0000013C

/*
 * Link Status (UP/Down) Register
 * Contains a port bitmap (see PBMP_*) of ports whose links are up.
 */
#define CMIC_LINK_STAT                  0x00000140

/** IRQ Register (RO) */
#define CMIC_IRQ_STAT                   0x00000144

/* IRQ Mask Register (R/W) */
#define CMIC_IRQ_MASK                   0x00000148

/* DMA Control Register */
#define CMIC_DMA_CTRL                   0x00000100


/* Endian selection register */
#define CMIC_ENDIAN_SELECT              0x00000174

#define ES_BIG_ENDIAN_PIO               0x01000001
#define ES_BIG_ENDIAN_DMA_PACKET        0x02020202
#define ES_BIG_ENDIAN_DMA_OTHER         0x04000004


#define DC_SOC_TO_MEM(ch)               (0x00 << (8 * (ch))) /* 0 <= ch <= 3 */
#define DC_MEM_TO_SOC(ch)               (0x01 << (8 * (ch)))
#define DC_DIRECTION_MASK(ch)           (0x01 << (8 * (ch)))

#define DC_MOD_BITMAP(ch)               (0x00 << (8 * (ch))) /* 0 <= ch <= 3 */
#define DC_NO_MOD_BITMAP(ch)            (0x02 << (8 * (ch)))
#define DC_MOD_BITMAP_MASK(ch)          (0x02 << (8 * (ch)))

#define DC_NO_ABORT_DMA(ch)             (0x00 << (8 * (ch))) /* 0 <= ch <= 3 */
#define DC_ABORT_DMA(ch)                (0x04 << (8 * (ch)))
#define DC_ABORT_DMA_MASK(ch)           (0x04 << (8 * (ch)))

#define DC_INTR_ON_PKT(ch)              (0x00 << (8 * (ch))) /* 0 <= ch <= 3 */
#define DC_INTR_ON_DESC(ch)             (0x08 << (8 * (ch)))
#define DC_INTR_ON_MASK(ch)             (0x08 << (8 * (ch)))

#define DC_NO_DROP_TX(ch)               (0x00 << (8 * (ch))) /* 0 <= ch <= 3 */
#define DC_DROP_TX(ch)                  (0x10 << (8 * (ch)))
#define DC_DROP_TX_MASK(ch)             (0x10 << (8 * (ch)))
#define DC_DROP_TX_ALL                  0x10101010

#define DC_CHAN_MASK(ch)                (0xff << (8 * (ch)))

/* DMA Status Register */
#define CMIC_DMA_STAT                   0x00000104

/* IRQ Mask bits */
#define IRQ_SCH_MSG_DONE                0x00000001 /* Bit 0 */
#define IRQ_ARL_MBUF                    0x00000002 /* Bit 1 */
#define IRQ_ARL_MBUF_DROP               0x00000004 /* Bit 2 */
#define IRQ_GBP_FULL                    0x00000008 /* Bit 3 */
#define IRQ_LINK_STAT_MOD               0x00000010 /* Bit 4 */
#define IRQ_ARL_DMA_XFER                0x00000020 /* Bit 5 */
#define IRQ_L2_MOD_FIFO_NOT_EMPTY       IRQ_ARL_DMA_XFER
#define IRQ_ARL_DMA_CNT0                0x00000040 /* Bit 6 */
#define IRQ_DESC_DONE(ch)               (0x00000080 << (2 * (ch)))
#define IRQ_CHAIN_DONE(ch)              (0x00000100 << (2 * (ch)))
#define IRQ_PCI_PARITY_ERR              0x00008000 /* Bit 15 */
#define IRQ_PCI_FATAL_ERR               0x00010000 /* Bit 16 */
#define IRQ_SCHAN_ERR                   0x00020000 /* Bit 17 */
#define IRQ_I2C_INTR                    0x00040000 /* bit 18 */
#define IRQ_MIIM_OP_DONE                0x00080000 /* Bit 19 */
#define IRQ_STAT_ITER_DONE              0x00100000 /* Bit 20 */
#define IRQ_BIT21                       0x00200000 /* bit 21 */
#define IRQ_MMU_IRQ_STAT                IRQ_BIT21  /* Bit 21 - XGS Fabric */
#define IRQ_ARL_ERROR                   IRQ_BIT21  /* Bit 21 - XGS Switch */
#define IRQ_IGBP_FULL                   0x00400000 /* Bit 22 */
#define IRQ_BIT23                       0x00800000 /* Unused */
#define IRQ_ARL_LPM_LO_PAR              0x01000000 /* Bit 24 */
#define IRQ_BIT25                       0x02000000 /* bit 25 */
#define IRQ_ARL_LPM_HI_PAR              IRQ_BIT25  /* bit 25 - 5665 etc. */
#define IRQ_BSE_CMDMEM_DONE             IRQ_BIT25  /* bit 25 - MCMD */
#define IRQ_MEM_CMD_DONE                IRQ_BIT25
#define IRQ_BIT26                       0x04000000 /* Bit 26 */
#define IRQ_ARL_L3_PAR                  IRQ_BIT26  /* Bit 26 - 5665 etc. */
#define IRQ_CSE_CMDMEM_DONE             IRQ_BIT26  /* bit 25 - MCMD */
#define IRQ_BIT27                       0x08000000 /* bit 27 */
#define IRQ_ARL_L2_PAR                  IRQ_BIT27  /* Bit 27 - 5665 etc.*/
#define IRQ_HSE_CMDMEM_DONE             IRQ_BIT27  /* bit 27 - MCMD */
#define IRQ_BIT28                       0x10000000 /* Bit 28 */
#define IRQ_ARL_VLAN_PAR                IRQ_BIT28  /* Bit 28 */
#define IRQ_MEM_FAIL                    IRQ_BIT28  /* Bit 28 */
#define IRQ_TDMA_DONE                   0x20000000 /* Bit 29 */
#define IRQ_TSLAM_DONE                  0x40000000 /* Bit 30 */
#define IRQ_BSAFE_OP_DONE               0x80000000 /* Bit 31 */

/* Hercules CMIC MMU Status/Mask Registers */
#define CMIC_MMUIRQ_STAT                0x1a0 /* One bit for each MMU */
#define CMIC_MMUIRQ_MASK                0x1a4 /* Mask of enables */

#define CMIC_MMUIRQ_ENABLE_MASK         0x1ff /* Set all MMU's enabled */

/* Memory Fault Register */
#define CMIC_MEM_FAIL                   0x0000014C

/* Ingress Back Pressure Warning Register */
#define CMIC_IGBP_WARN                  0x00000150

/* Ingress Back Pressure Discard Register */
#define CMIC_IGBP_DISCARD               0x00000154

/* MIIM Parameter Register */
#define CMIC_MIIM_PARAM                 0x00000158

#define MIIM_PARAM_ID_OFFSET            16
#define MIIM_PARAM_REG_ADDR_OFFSET      24      /* Ignored on Lynx; see */
                                                /* CMIC_MIIM_ADDRESS */
/* MIIM Read Data Register */
#define CMIC_MIIM_READ_DATA             0x0000015C

/* Scan Ports Register */
#define CMIC_SCAN_PORTS                 0x00000160

/* Counter DMA address register */
#define CMIC_STAT_DMA_ADDR              0x00000164

/* Counter DMA setup register */
#define CMIC_STAT_DMA_SETUP             0x00000168


/** for HELIX **/
#define CMIC_SCHAN_WORDS(unit)  22

#define CMIC_SCHAN_MESSAGE(unit, word)  (0x00000800 + 4 * (word))

#define CMIC_SCHAN_WORDS_ALLOC 22

/* S-Channel Control Register */
#define CMIC_SCHAN_CTRL                 0x00000050

/*
 * SCHAN_CONTROL: control bits
 *
 *  SC_xxx_SET and SC_xxx_CLR can be WRITTEN to CMIC_SCHAN_CTRL.
 *  SC_xxx_TST can be masked against values READ from CMIC_SCHAN_CTRL.
 */
#define SC_MSG_START_SET                (0x80|0)
#define SC_MSG_START_CLR                (0x00|0)
#define SC_MSG_START_TST                0x00000001

#define SC_MSG_DONE_SET                 (0x80|1)
#define SC_MSG_DONE_CLR                 (0x00|1)
#define SC_MSG_DONE_TST                 0x00000002

#define SC_ARL_RDY_SET(msg)             (0x80|((msg)+2))   /* 0 <= msg <= 3 */
#define SC_ARL_RDY_CLR(msg)             (0x00|((msg)+2))
#define SC_ARL_RDY_TST(msg)             (0x00000004 << (msg))
#define SC_ARL_RDY_TST_ANY              (0x0000003c)

#define SC_ARL_DMA_EN_SET               (0x80|6)
#define SC_ARL_DMA_EN_CLR               (0x00|6)
#define SC_ARL_DMA_EN_TST               0x00000040

#define SC_ARL_DMA_DONE_SET             (0x80|7)
#define SC_ARL_DMA_DONE_CLR             (0x00|7)
#define SC_ARL_DMA_DONE_TST             0x00000080

#define SC_LINK_STAT_MSG_SET            (0x80|8)
#define SC_LINK_STAT_MSG_CLR            (0x00|8)
#define SC_LINK_STAT_MSG_TST            0x00000100

#define SC_PCI_FATAL_ERR_SET            (0x80|9)
#define SC_PCI_FATAL_ERR_CLR            (0x00|9)
#define SC_PCI_FATAL_ERR_TST            0x00000200

#define SC_PCI_PARITY_ERR_SET           (0x80|10)
#define SC_PCI_PARITY_ERR_CLR           (0x00|10)
#define SC_PCI_PARITY_ERR_TST           0x00000400

#define SC_ARL_MSG_RCV_OFF_SET          (0x80|11)
#define SC_ARL_MSG_RCV_OFF_CLR          (0x00|11)
#define SC_ARL_MSG_RCV_OFF_TST          0x00000800

#define SC_ARL_MSG_DROPPED_SET          (0x80|12)
#define SC_ARL_MSG_DROPPED_CLR          (0x00|12)
#define SC_ARL_MSG_DROPPED_TST          0x00001000

#define SC_ARL_DMA_XFER_DONE_SET        (0x80|13)
#define SC_ARL_DMA_XFER_DONE_CLR        (0x00|13)
#define SC_ARL_DMA_XFER_DONE_TST        0x00002000

#define SC_MIIM_SCAN_BUSY_TST           0x00004000

#define SC_GBP_OK_SET                   (0x80|15)
#define SC_GBP_OK_CLR                   (0x00|15)
#define SC_GBP_OK_TST                   0x00008000

#define SC_MIIM_RD_START_SET            (0x80|16)
#define SC_MIIM_RD_START_CLR            (0x00|16)
#define SC_MIIM_RD_START_TST            0x00010000

#define SC_MIIM_WR_START_SET            (0x80|17)
#define SC_MIIM_WR_START_CLR            (0x00|17)
#define SC_MIIM_WR_START_TST            0x00020000

#define SC_MIIM_OP_DONE_SET             (0x80|18)
#define SC_MIIM_OP_DONE_CLR             (0x00|18)
#define SC_MIIM_OP_DONE_TST             0x00040000

#define SC_MIIM_LINK_SCAN_EN_SET        (0x80|19)
#define SC_MIIM_LINK_SCAN_EN_CLR        (0x00|19)
#define SC_MIIM_LINK_SCAN_EN_TST        0x00080000

#define SC_MSG_NAK_SET                  (0x80|21)
#define SC_MSG_NAK_CLR                  (0x00|21)
#define SC_MSG_NAK_TST                  0x00200000

#define SC_MSG_TIMEOUT_SET              (0x80|22)
#define SC_MSG_TIMEOUT_CLR              (0x00|22)
#define SC_MSG_TIMEOUT_TST              0x00400000

/*
 * DMA_STAT: control bits
 *
 *  xxx_SET and xxx_CLR can be WRITTEN to CMIC_DMA_STAT
 *  xxx_TST can be masked against values read from CMIC_DMA_STAT.
 *  Argument required: 0 <= ch <= 3
 */
#define DS_DMA_EN_SET(ch)               (0x80|(ch))
#define DS_DMA_EN_CLR(ch)               (0x00|(ch))
#define DS_DMA_EN_TST(ch)               (0x00000001 << (ch))

#define DS_CHAIN_DONE_SET(ch)           (0x80|(4+(ch)))
#define DS_CHAIN_DONE_CLR(ch)           (0x00|(4+(ch)))
#define DS_CHAIN_DONE_TST(ch)           (0x00000010 << (ch))

#define DS_DESC_DONE_SET(ch)            (0x80|(8+(ch)))
#define DS_DESC_DONE_CLR(ch)            (0x00|(8+(ch)))
#define DS_DESC_DONE_TST(ch)            (0x00000100 << (ch))

#define DS_DMA_RESET                    0x1000
#define _DS_DMA_RESET                   DS_DMA_RESET

#define DS_STAT_DMA_DONE_SET            (0x80|13)
#define DS_STAT_DMA_DONE_CLR            (0x00|13)
#define DS_STAT_DMA_DONE_TST            0x2000

#define DS_STAT_DMA_ITER_DONE_SET       (0x80|14)
#define DS_STAT_DMA_ITER_DONE_CLR       (0x00|14)
#define DS_STAT_DMA_ITER_DONE_TST       0x4000

#define DS_STAT_DMA_ACTIVE              0x20000
#define DS_STAT_DMA_ERROR_SET           (0x80|16)
#define DS_STAT_DMA_ERROR_CLR           (0x00|16)
#define DS_STAT_DMA_ERROR               0x10000

#define DS_DMA_ACTIVE(ch)               (0x00040000 << (ch))

/* These actually return the DMA channel number of failure */
#define DS_PCI_PARITY_ERR(reg)          ((reg) >> 24 & 3)
#define DS_PCI_FATAL_ERR(reg)           ((reg) >> 29 & 3)

#define MILLISECOND_USEC                (1000)
#define SECOND_USEC                     (1000000)

#define SCHAN_TIMEOUT   (300 * MILLISECOND_USEC)
#define MIIM_TIMEOUT    (300 * MILLISECOND_USEC)

#define SOC_MEM_FLAG_READONLY   0x00000001 /* True if table is read-only */
#define SOC_MEM_FLAG_VALID      0x00000002 /* Some tables don't really exist */
#define SOC_MEM_FLAG_DEBUG      0x00000004 /* Access requires CMIC DebugMode */
#define SOC_MEM_FLAG_SORTED     0x00000008 /* Table is kept in sorted order */
#define SOC_MEM_FLAG_CBP        0x00000010 /* Table is part of CBP */
#define SOC_MEM_FLAG_CACHABLE   0x00000020 /* Reasonable to cache in S/W */
#define SOC_MEM_FLAG_BISTCBP    0x00000040 /* CBP has BIST for this memory */
#define SOC_MEM_FLAG_BISTEPIC   0x00000080 /* EPIC has BIST for this memory */
#define SOC_MEM_FLAG_BISTBIT    0x00003f00 /* Bit pos for BIST reg (0-63) */
#define SOC_MEM_FLAG_BISTBSHFT  8          /* Shift corresponding to BISTBIT */
#define SOC_MEM_FLAG_BISTFFP    0x00004000 /* Use FFPBIST for this memory */
#define SOC_MEM_FLAG_UNIFIED    0x00010000 /* Only one copy of table */
#define SOC_MEM_FLAG_HASHED     0x00020000 /* Hashed table */
#define SOC_MEM_FLAG_WORDADR    0x00040000 /* Requires special addressing */
#define SOC_MEM_FLAG_CAM        0x00080000 /* Memory is a CAM */
#define SOC_MEM_FLAG_AGGR       0x00100000 /* Memory is a AGGREGATE */
#define SOC_MEM_FLAG_CMD        0x00200000 /* Memory allows CMDMEM access */
#define SOC_MEM_FLAG_MONOLITH   0x04000000 /* no block in addressing */
#define SOC_MEM_FLAG_BE         0x08000000 /* Big endian */


#define SOC_BLK_NONE     0x000000  /* no port (used for empty port numbers) */
#define SOC_BLK_EPIC     0x000001  /* 10/100M ethernet ports */
#define SOC_BLK_GPIC     0x000002  /* 1G ethernet ports */
#define SOC_BLK_HPIC     0x000004  /* 10G higig ports (herc style) */
#define SOC_BLK_IPIC     0x000008  /* 10G higig ports (draco/lynx style) */
#define SOC_BLK_XPIC     0x000010  /* 10G ethernet ports */
#define SOC_BLK_CMIC     0x000020  /* CPU */
#define SOC_BLK_CPIC     0x000040  /* CPU (herc style) */
#define SOC_BLK_ARL      0x000080  /* Address Resolution Logic */
#define SOC_BLK_MMU      0x000100  /* Memory Management Unit */
#define SOC_BLK_MCU      0x000200  /* Memory Control Unit */
#define SOC_BLK_GPORT    0x000400  /* 1G Ethernet ports (FB/ER) */
#define SOC_BLK_XPORT    0x000800  /* 10G Ethernet/higig port(FB/ER)*/
#define SOC_BLK_IPIPE    0x001000  /* Ingress Pipeline (firebolt) */
#define SOC_BLK_IPIPE_HI 0x002000  /* Ingress Pipeline (Higig only) (FB) */
#define SOC_BLK_EPIPE    0x004000  /* Egress Pipeline (FB) */
#define SOC_BLK_EPIPE_HI 0x008000  /* Egress Pipeline (Higig only) (FB) */
#define SOC_BLK_IGR      0x010000  /* Ingress (ER) */
#define SOC_BLK_EGR      0x020000  /* Egress (ER) */
#define SOC_BLK_BSE      0x040000  /* Binary Search Engine (ER) */
#define SOC_BLK_CSE      0x080000  /* CAM Search Engine (ER) */
#define SOC_BLK_HSE      0x100000  /* Hash Search Engine (ER) */
#define SOC_BLK_BSAFE    0x200000  /* Broadsafe */
#define SOC_BLK_GXPORT    0x00400000  /* 10/2.5/1G port */

/*
 * S-Channel Message Types
 */

#define BP_WARN_STATUS_MSG            0x01
#define BP_DISCARD_STATUS_MSG         0x02
#define COS_QSTAT_NOTIFY_MSG          0x03      /* Not on XGS */
#define IPIC_HOL_STAT_MSG             0x03      /* 5665 (alias) */
#define HOL_STAT_NOTIFY_MSG           0x04
#define READ_MEMORY_CMD_MSG           0x07
#define READ_MEMORY_ACK_MSG           0x08
#define WRITE_MEMORY_CMD_MSG          0x09
#define WRITE_MEMORY_ACK_MSG          0x0a
#define READ_REGISTER_CMD_MSG         0x0b
#define READ_REGISTER_ACK_MSG         0x0c
#define WRITE_REGISTER_CMD_MSG        0x0d
#define WRITE_REGISTER_ACK_MSG        0x0e
#define ARL_INSERT_CMD_MSG            0x0f
#define ARL_INSERT_DONE_MSG           0x10
#define ARL_DELETE_CMD_MSG            0x11
#define ARL_DELETE_DONE_MSG           0x12
#define LINKSTAT_NOTIFY_MSG           0x13      /* Strata I/II only */
#define MEMORY_FAIL_NOTIFY            0x14
#define INIT_CFAP_MSG                 0x15      /* 5690 only */
#define IPIC_GBP_FULL_MSG             0x15      /* 5665 (alias) */
#define IPIC_GBP_AVAIL_MSG            0x16      /* 5665 (alias) */
#define ENTER_DEBUG_MODE_MSG          0x17
#define EXIT_DEBUG_MODE_MSG           0x18
#define ARL_LOOKUP_CMD_MSG            0x19
#define L3_INSERT_CMD_MSG             0x1a
#define L3_INSERT_DONE_MSG            0x1b
#define L3_DELETE_CMD_MSG             0x1c
#define L3_DELETE_DONE_MSG            0x1d
#define L3_LOOKUP_CMD_MSG             0x1e      /* 5695 */
#define L2_LOOKUP_CMD_MSG             0x20      /* 56504 / 5630x / 5610x */
#define L2_LOOKUP_ACK_MSG             0x21      /* 56504 / 5630x / 5610x */
#define L3X2_LOOKUP_CMD_MSG           0x22      /* 56504 / 5630x / 5610x */
#define L3X2_LOOKUP_ACK_MSG           0x23      /* 56504 / 5630x / 5610x */

/* Standard MII Registers */

#define MII_CTRL_REG            0x00    /* MII Control Register : r/w */
#define MII_STAT_REG            0x01    /* MII Status Register: ro */
#define MII_PHY_ID0_REG         0x02    /* MII PHY ID register: r/w */
#define MII_PHY_ID1_REG         0x03    /* MII PHY ID register: r/w */
#define MII_ANA_REG             0x04    /* MII Auto-Neg Advertisement: r/w */
#define MII_ANP_REG             0x05    /* MII Auto-Neg Link Partner: ro */
#define MII_AN_EXP_REG          0x06    /* MII Auto-Neg Expansion: ro */
#define MII_GB_CTRL_REG         0x09    /* MII 1000Base-T control register */
#define MII_GB_STAT_REG         0x0a    /* MII 1000Base-T Status register */
#define MII_ESR_REG             0x0f    /* MII Extended Status register */


/* Non-standard MII Registers */

#define MII_ECR_REG             0x10    /* MII Extended Control Register */
#define MII_ASSR_REG            0x19    /* MII Auxiliary Status Summary Reg */
#define MII_GSR_REG             0x1c    /* MII General status (BROADCOM) */
#define MII_MSSEED_REG          0x1d    /* MII Master/slave seed (BROADCOM) */
#define MII_TEST2_REG           0x1f    /* MII Test reg (BROADCOM) */

/* MII Control Register: bit definitions */

#define MII_CTRL_SS_MSB         (1 << 6) /* Speed select, MSb */
#define MII_CTRL_CST            (1 << 7) /* Collision Signal test */
#define MII_CTRL_FD             (1 << 8) /* Full Duplex */
#define MII_CTRL_RAN            (1 << 9) /* Restart Autonegotiation */
#define MII_CTRL_IP             (1 << 10) /* Isolate Phy */
#define MII_CTRL_PD             (1 << 11) /* Power Down */
#define MII_CTRL_AE             (1 << 12) /* Autonegotiation enable */
#define MII_CTRL_SS_LSB         (1 << 13) /* Speed select, LSb */
#define MII_CTRL_LE             (1 << 14) /* Loopback enable */
#define MII_CTRL_RESET          (1 << 15) /* PHY reset */

#define MII_CTRL_SS(_x)         ((_x) & (MII_CTRL_SS_LSB|MII_CTRL_SS_MSB))
#define MII_CTRL_SS_10          0
#define MII_CTRL_SS_100         (MII_CTRL_SS_LSB)
#define MII_CTRL_SS_1000        (MII_CTRL_SS_MSB)
#define MII_CTRL_SS_INVALID     (MII_CTRL_SS_LSB | MII_CTRL_SS_MSB)
#define MII_CTRL_SS_MASK        (MII_CTRL_SS_LSB | MII_CTRL_SS_MSB)


/*=======================================================
 * S-Chan related structures
 *=======================================================*/

typedef struct schan_header_s {
#if defined(LE_HOST)
    uint32 cpu:1,               /* Bit 0 NAK on XGS3 */
           cos:3,
           ecode: 2,
           ebit:1,
           datalen:7,
           srcblk:6,            /* See socregs.h: xxx_SCH_BLK_NUM */
           dstblk:6,
           opcode:6;
#else
    uint32 opcode:6,            /* Bits 31:26 */
           dstblk:6,
           srcblk:6,
           datalen:7,
           ebit:1,
           ecode: 2,
           cos:3,
           cpu:1;               /* NAK on XGS3 */
#endif
} schan_header_t;

/*
 * Individual S-Channel message formats.
 * Different ways of peeking and poking at an S-Channel message
 * packet.  Applicable message types are listed inside each structure.
 */

typedef struct schan_msg_plain_s {
    /* GBP Full Notification */
    /* GBP Available Notification */
    /* Write Memory Ack */
    /* Write Register Ack */
    /* ARL Insert Complete */
    /* ARL Delete Complete */
    /* Memory Failed Notification */
    /* Initialize CFAP (Cell FAP) */
    /* Initialize SFAP (Slot FAP) */
    /* Enter Debug Mode */
    /* Exit Debug Mode */
    schan_header_t header;
} schan_msg_plain_t;

typedef struct schan_msg_bitmap_s {
    /* Back Pressure Warning Status */
    /* Back Pressure Discard Status */
    /* Link Status Notification (except 5695) */
    /* COS Queue Status Notification */
    /* HOL Status Notification */
    schan_header_t header;
    uint32 bitmap;
    uint32 bitmap_word1;  /* 5665 only, so far */
} schan_msg_bitmap_t;

typedef struct schan_msg_readcmd_s {
    /* Read Memory Command */
    /* Read Register Command */
    schan_header_t header;
    uint32 address;
} schan_msg_readcmd_t;

typedef struct schan_msg_readresp_s {
    /* Read Memory Ack */
    /* Read Register Ack */
    schan_header_t header;
    uint32 data[CMIC_SCHAN_WORDS_ALLOC - 1];
} schan_msg_readresp_t;

typedef struct schan_msg_writecmd_s {
    /* Write Memory Command */
    /* Write Register Command */
    schan_header_t header;
    uint32 address;
    uint32 data[CMIC_SCHAN_WORDS_ALLOC - 2];
} schan_msg_writecmd_t;

typedef struct schan_msg_arlins_s {
    /* ARL Insert Command */
    /* (Also: ARL Message Buffer Format) */
    /* (Also: ARL DMA Message Format) */
    schan_header_t header;
    uint32 data[3];
} schan_msg_arlins_t;

typedef struct schan_msg_arldel_s {
    /* ARL Delete Command */
    schan_header_t header;
    uint32 data[2];
} schan_msg_arldel_t;

typedef struct schan_msg_arllkup_s {
    /* ARL Lookup Command */
    schan_header_t header;
    uint32 address;
    uint32 data[2];
} schan_msg_arllkup_t;

typedef struct schan_msg_l3ins_s {
    /* L3 Insert Command */
    schan_header_t header;
    uint32 data[4];
} schan_msg_l3ins_t;

typedef struct schan_msg_l3del_s {
    /* L3 Delete Command */
    schan_header_t header;
    uint32 data[4];
} schan_msg_l3del_t;

typedef struct schan_msg_l3lkup_s {
    /* L3 Lookup Command */
    schan_header_t header;
    uint32 address;
    uint32 data[4];
} schan_msg_l3lkup_t;

typedef struct schan_msg_l2x2_s {
    /* L2 Insert/Delete/Lookup Command 56504 / 5630x / 5610x */
    schan_header_t header;
    uint32 data[3];
} schan_msg_l2x2_t;

typedef struct schan_msg_l3x2_s {
    /* L3 Insert/Delete/Lookup Command 56504 / 5630x / 5610x */
    schan_header_t header;
    uint32 data[13];
} schan_msg_l3x2_t;

/*
 * Union of all S-Channel message types (use to declare all message buffers)
 *
 * When building messages, address the union according to packet type.
 * When writing to PCI, address data.dwords.
 * When writing to I2C, address data.bytes.
 */

#define schan_msg_clear(m)      ((m)->header_dword = 0)

typedef union schan_msg_u {
    schan_header_t header;
    uint32 header_dword;
    schan_msg_plain_t plain;
    schan_msg_bitmap_t bitmap;
    schan_msg_readcmd_t readcmd;
    schan_msg_readresp_t readresp;
    schan_msg_writecmd_t writecmd;
    schan_msg_arlins_t arlins;
    schan_msg_arldel_t arldel;
    schan_msg_arllkup_t arllkup;
    schan_msg_l3ins_t l3ins;
    schan_msg_l3del_t l3del;
    schan_msg_l3lkup_t l3lkup;
    schan_msg_l2x2_t    l2x2;
    schan_msg_l3x2_t    l3x2;
    uint32 dwords[CMIC_SCHAN_WORDS_ALLOC];
    uint8 bytes[sizeof(uint32) * CMIC_SCHAN_WORDS_ALLOC];
} schan_msg_t;


/*=======================================================
 * Block related structures
 *=======================================================*/
#define SOC_MEM_BLOCK_VALID(unit, mem, blk)     \
                        ((blk) >= 0 && \
                         (blk) < SOC_MAX_NUM_BLKS && \
                         ((mem)->blocks & (1<<(blk))))

#define SOC_MEM_BLOCK_ITER(unit, mem, var) \
        for ((var) = (mem)->minblock; \
                (var) <= (mem)->maxblock; \
                (var)++) \
                if ( ((mem)->blocks & (1 << (var))) )

typedef struct {
    int                 type;           /* SOC_BLK_* */
    int                 number;         /* instance of type */
    int                 schan;          /* pic number for schan commands */
    int                 cmic;           /* pic number for cmic r/w commands */
} soc_block_info_t;

typedef uint32  soc_block_t;

/*
 * Chip types supported
 */
typedef enum soc_chip_types_e {
    SOC_CHIP_BCM56504_A0,
    SOC_CHIP_BCM56504_B0,
    SOC_CHIP_BCM56218_A0,
    SOC_CHIP_TYPES_COUNT
} soc_chip_types;

#define SOC_BLOCK_ANY   -1
#define MEM_BLOCK_ANY   -1
#define SOC_BLOCK_ALL   -1
#define MEM_BLOCK_ALL   SOC_BLOCK_ALL
#define COPYNO_ALL SOC_BLOCK_ALL

/*=======================================================
 * Port related structures
 *=======================================================*/
typedef struct {
    int                 blk;            /* index into soc_block_info array */
    int                 bindex;         /* index of port within block */
} soc_port_info_t;


typedef struct soc_field_info_s {
        soc_field_t     field;
        uint16          len;        /* Bits in field */
        uint16          bp;             /* Least bit position of the field */
        unsigned char   flags;  /* Logical OR of SOCF_* */
} soc_field_info_t;

#if defined(BCM56218) || defined(BCM56018) || defined(BCM56218P48)
#define PORT_MAP_ALL                (0xffffffff)
#define PORT_MAP_ALL_HI             (0x3fffff)
#define PORT_MAP_GE                 (0xfffffffe)
#define PORT_MAP_GE_HI              (0x3fffff)
#elif defined(BCM56014) || defined(BCM56214)
#define PORT_MAP_ALL                (0x3fffffff)
#define PORT_MAP_ALL_HI             (0)
#define PORT_MAP_GE                 (0x3ffffffe)
#define PORT_MAP_GE_HI              (0)
#endif

#define HE_PBMP_ALL_ITER(u,_p)  HE__PORT_ITER((u),(_p), PORT_MAP_ALL, PORT_MAP_ALL_HI)
#define HE_PBMP_ALL_ALL_ITER(u,_p)  HE__PORT_ITER((u),(_p), PORT_MAP_ALL, PORT_MAP_ALL_HI)
#define HE_PBMP_GE_ITER(u,_p)   HE__PORT_ITER((u),(_p), PORT_MAP_GE, PORT_MAP_GE_HI)

#define BYTES2WORDS(x)          (((x) + 3) / 4)
#define COMPILER_REFERENCE(a)   ((void)(a))

#define _SHR_PBMP_PORT_SET(pbm, port)    ((pbm) = (1U << (port)))
#define _SHR_PBMP_PORT_ADD(pbm, port)    ((pbm) |= (1U << (port)))
#define _SHR_PBMP_PORT_REMOVE(pbm, port) ((pbm) &= ~(1U << (port)))
#define _SHR_PBMP_PORT_FLIP(pbm, port)   ((pbm) ^= (1U << (port)))
#define _SHR_PBMP_CLEAR(pbm)             ((pbm) = 0)

#define SOC_PBMP_PORT_SET(pbm, port)    _SHR_PBMP_PORT_SET(pbm, port)
#define SOC_PBMP_PORT_ADD(pbm, port)    _SHR_PBMP_PORT_ADD(pbm, port)
#define SOC_PBMP_PORT_REMOVE(pbm, port) _SHR_PBMP_PORT_REMOVE(pbm, port)
#define SOC_PBMP_PORT_FLIP(pbm, port)   _SHR_PBMP_PORT_FLIP(pbm, port)
#define SOC_PBMP_CLEAR(pbm)             _SHR_PBMP_CLEAR(pbm)


/*=======================================================
 * Register related structures
 *=======================================================*/

/*
 * If you add anything here, check soc/common.c for arrays
 * indexed by soc_regtype_t.
 */
typedef enum soc_regtype_t {
    soc_schan_reg,      /* Generic register read thru SCHAN */
    soc_genreg,         /* General soc registers */
    soc_portreg,        /* Port soc registers */
    soc_cosreg,         /* COS soc registers */
    soc_cpureg,         /* AKA PCI memory */
    soc_pci_cfg_reg,    /* PCI configuration space register */ 
    soc_phy_reg,        /* PHY register, access thru mii */
    soc_hostmem_w,      /* word */
    soc_hostmem_h,      /* half word */ 
    soc_hostmem_b,      /* byte */
    soc_invalidreg
} soc_regtype_t;


typedef struct soc_reg_info_t {
    soc_block_t        block;
    soc_regtype_t      regtype;         /* Also indicates invalid */
    int                numels;          /* If array, num els in array. */
                                        /* Otherwise -1. */
    uint32             offset;          /* Includes 2-bit form field */
#define SOC_REG_FLAG_64_BITS (1<<0)     /* Register is 64 bits wide */
#define SOC_REG_FLAG_COUNTER (1<<1)     /* Register is a counter */
#define SOC_REG_FLAG_ARRAY   (1<<2)     /* Register is an array */
#define SOC_REG_FLAG_NO_DGNL (1<<3)     /* Array does not have diagonal els */
#define SOC_REG_FLAG_RO      (1<<4)     /* Register is write only */
#define SOC_REG_FLAG_WO      (1<<5)     /* Register is read only */
#define SOC_REG_FLAG_ED_CNTR (1<<6)     /* Counter of discard/error */
#define SOC_REG_FLAG_SPECIAL (1<<7)     /* Counter requires special
                                           processing */
#define SOC_REG_FLAG_EMULATION  (1<<8)  /* Available only in emulation */
#define SOC_REG_FLAG_VARIANT1   (1<<9)  /* Not available in chip variants  */
#define SOC_REG_FLAG_VARIANT2   (1<<10) /* -- '' -- */
#define SOC_REG_FLAG_VARIANT3   (1<<11) /* -- '' -- */
#define SOC_REG_FLAG_VARIANT4   (1<<12) /* -- '' -- */

    uint32             flags;
    int                nFields;
    soc_field_info_t   *fields;
#if !defined(SOC_NO_RESET_VALS)
    uint32             rst_val_lo;
    uint32             rst_val_hi;      /* 64-bit regs only */
    uint32             rst_mask_lo;     /* 1 where resetVal is valid */
    uint32             rst_mask_hi;     /* 64-bit regs only */
#endif
    int                ctr_idx;         /* Counters only; sw idx */
} soc_reg_info_t;


/*=======================================================
 * Memory related structures
 *=======================================================*/

#define SOCF_LE         0x01    /* little endian */
#define SOCF_RO         0x02    /* read only */
#define SOCF_WO         0x04    /* write only */
#define SOCF_SC         0x08
#define SOCF_RES        0x10    /* reserved (do not test, etc.) */

/* New unified soc memories structure */
typedef struct soc_mem_info_s {
#ifdef DEBUG
    char*                 name;
#endif
    uint32                flags;          /* Logical OR of SOC_MEM_FLAG_xxx */
    /* soc_mem_cmp_t      cmp_fn;         Function to compare two entries */
    /* void               *null_entry;    Value when table entry is empty */
    int                   index_min;      /* Minimum table index */
    int                   index_max;      /* Maximum table index */
    uint16                minblock;       /* first blocks entry */
    uint16                maxblock;       /* last blocks entry */
    uint32                blocks;         /* bitmask of valid blocks */
    uint32                base;           /* Includes region offset */
    uint32                gran;           /* Phys addr granularity by index */
    uint16                bytes;
    uint16                nFields;
    soc_field_info_t      *fields;
} soc_mem_info_t;

/*
 * soc_init_chip_fun_t: chip initialization function
 * Used in bcm*.i files.
 */
typedef void (*soc_init_chip_fun_t) (void);


/*=======================================================
 * Chip Driver
 *=======================================================*/
typedef struct soc_driver_s {
    soc_chip_types      type;                   /* the chip type id */
    char                *chip_string;           /* chip string for var defs */
    uint16              pci_vendor;             /* nominal PCI vendor */
    uint16              pci_device;             /* nominal PCI device */
    uint8               pci_revision;           /* nominal PCI revision */
    soc_reg_info_t      **reg_info;             /* register array */
    soc_mem_info_t      **mem_info;             /* memory array */
    soc_block_info_t    *block_info;            /* block array */
    soc_port_info_t     *port_info;             /* port array */
    soc_init_chip_fun_t init;                   /* chip init function */
} soc_driver_t;

/*
 * MICROs
 */

#define SOC_DRIVER(unit)		(drivers[(unit)])
#define SOC_MEM_INFO(unit, mem) 	(*SOC_DRIVER(unit)->mem_info[mem])
#define SOC_REG_INFO(unit, reg) 	(*SOC_DRIVER(unit)->reg_info[reg])
#define SOC_REG_PTR(unit, reg)          (SOC_DRIVER(unit)->reg_info[reg])
#define SOC_BLOCK_INFO(unit, blk)       (SOC_DRIVER(unit)->block_info[blk])
#define SOC_PORT_INFO(unit, port)       (SOC_DRIVER(unit)->port_info[port])
#define SOC_CHIP_STRING(unit)           (SOC_DRIVER(unit)->chip_string)
#define SOC_BLOCK2OFFSET(unit, blk)     (SOC_BLOCK_INFO(unit,blk).cmic)

#define REG_PORT_ANY            (-10)
#define SOC_PORT_VALID_RANGE(port)      ((port) >= 0 && \
                                         (port) < SOC_MAX_NUM_PORTS)
#define SOC_PORT_VALID(unit,port)       (SOC_PORT_VALID_RANGE(port) && \
                                         SOC_PORT_TYPE(unit,port) != 0)
#define SOC_PORT_NAME(unit,port)        (SOC_INFO(unit).port_name[port])
#define SOC_PORT_OFFSET(unit,port)      (SOC_INFO(unit).port_offset[port])
#define SOC_PORT_TYPE(unit,port)        (SOC_INFO(unit).port_type[port])
#define SOC_PORT_BLOCK(unit, port)      (SOC_PORT_INFO(unit, port).blk)
#define SOC_PORT_BINDEX(unit, port)     (SOC_PORT_INFO(unit, port).bindex)

/* use PORT_MIN/_MAX to be more efficient than PBMP_ITER */
#define _SHR_PBMP_WBIT(port)            (1U<<(port))
#define _SHR_PBMP_MEMBER(bmp, port)     (((bmp) & _SHR_PBMP_WBIT(port)) != 0)

#define SOC_BLK_PORT    (SOC_BLK_EPIC | \
                         SOC_BLK_GPIC | \
                         SOC_BLK_HPIC | \
                         SOC_BLK_IPIC | \
                         SOC_BLK_XPIC | \
                         SOC_BLK_GPORT | \
                         SOC_BLK_XPORT | \
                         SOC_BLK_GXPORT | \
                         SOC_BLK_CPIC)
#define SOC_BLK_CPU     (SOC_BLK_CMIC | \
                         SOC_BLK_CPIC)
#define SOC_BLK_ETHER   (SOC_BLK_EPIC | \
                         SOC_BLK_GPIC | \
                         SOC_BLK_GPORT | \
                         SOC_BLK_XPORT | \
                         SOC_BLK_GXPORT | \
                         SOC_BLK_XPIC)
#define SOC_BLK_HIGIG   (SOC_BLK_HPIC | \
                         SOC_BLK_XPORT | \
                         SOC_BLK_GXPORT | \
                         SOC_BLK_IPIC)
#define SOC_BLK_NET     (SOC_BLK_ETHER | \
                         SOC_BLK_HIGIG)
#define SOC_BLK_HGPORT  (SOC_BLK_IPIPE_HI)

#define SOC_REG_ARRAY(unit, reg)     \
    (SOC_REG_INFO(unit, reg).flags & SOC_REG_FLAG_ARRAY)
#define SOC_REG_NUMELS(unit, reg)     \
    (SOC_REG_INFO(unit, reg).numels)

#include "chips/intenum.h"
#include "chips/memregs.h"

#include "chips/allfields.h"

uint32 soc_reg_addr(int unit, soc_reg_t reg, int port, int index);

uint32 soc_reg_field_get(int unit, soc_reg_t reg, uint32 current, soc_field_t field);

void soc_reg_field_set(int unit, soc_reg_t reg, uint32 *current,
                  soc_field_t field, uint32 value);
#endif

