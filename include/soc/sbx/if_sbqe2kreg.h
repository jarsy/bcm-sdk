/******************************************************************************
** ==========================================================
** == if_sbqe2kreg.h - Register Definitions for SB QE2000  ==
** ==========================================================
**
** WORKING REVISION: $Id: if_sbqe2kreg.h,v 1.3 Broadcom SDK $
**
** $Copyright: (c) 2016 Broadcom.
** Broadcom Proprietary and Confidential. All rights reserved.$ 
**
** MODULE NAME:
**
**     if_sbqe2kreg.h
**
** ABSTRACT:
**
**     Register definitions for use with the sbqe2000 NIC driver
**
** LANGUAGE:
**
**     C
**
** AUTHORS:
**
**     Travis B. Sawyer
**
** CREATION DATE:
**
**     21-March-2005
**
******************************************************************************/
/* Interrupt register */
#define SBQE_PC_SOFT_RESET      0x004
#define SBQE_PCI_INTERRUPT	0x001c
#define SBQE_PCI_INTERRUPT_MASK	0x0020
#define  SBQE_PCI_INTR_SPARE		        0x80000000
#define  SBQE_PCI_INTR_DMA_DONE		        0x40000000
#define  SBQE_PCI_INTR_PCI_COMPLETION		0x20000000
#define  SBQE_PCI_INTR_TXDMA_ERROR	        0x00008000
#define  SBQE_PCI_INTR_SPIR1_ERROR	        0x00004000
#define  SBQE_PCI_INTR_SPIT1_ERROR		0x00002000
#define  SBQE_PCI_INTR_SPIR0_ERROR		0x00001000
#define  SBQE_PCI_INTR_SPIT0_ERROR		0x00000800
#define  SBQE_PCI_INTR_SCI_ERROR		0x00000200
#define  SBQE_PCI_INTR_SFI_ERROR		0x00000100
#define  SBQE_PCI_INTR_RXDMA_ERROR		0x00000080
#define  SBQE_PCI_INTR_QOS_ERROR		0x00000040
#define  SBQE_PCI_INTR_QMGR_ERROR		0x00000020
#define  SBQE_PCI_INTR_PMC_ERROR		0x00000010
#define  SBQE_PCI_INTR_EGRESS_IFACE_ERROR	0x00000008
#define  SBQE_PCI_INTR_SFI		        0x00000004
#define  SBQE_PCI_INTR_PCI_1			0x00000002
#define  SBQE_PCI_INTR_PCI_0			0x00000001


#define SBQE_PC_SFI_INT_MASK 0x0028



/* PCI error register 0 */
#define SBQE_PCI_ERROR0			0x0030
#define SBQE_PCI_ERROR0_MASK		0x0034

/* PCI error register 1 */
#define SBQE_PCI_ERROR1			0x0038
#define SBQE_PCI_ERROR1_MASK		0x003C
#define  SBQE_PCI_ERR1_GET_LARGE_DROP(r)	(((r) >> 24) & 0xff)
#define  SBQE_PCI_ERR1_GET_EOP_MISSING(r)	(((r) >> 20) & 0x0f)
#define  SBQE_PCI_ERR1_GET_EXTRA_SOP(r)		(((r) >> 16) & 0x0f)
#define  SBQE_PCI_ERR1_GET_RXPKT_ABRT(r)	(((r) >> 12) & 0x0f)
#define  SBQE_PCI_ERR1_ZERO_LENGTH              0x00000800
#define  SBQE_PCI_ERR1_TX_FATAL_PERR            0x00000400
#define  SBQE_PCI_ERR1_TXRING_UNDERFLOW		0x00000200
#define  SBQE_PCI_ERR1_TOOMANY_DMA_INIT         0x00000100
#define  SBQE_PCI_ERR1_BAD_TXPTR		0x00000080
#define  SBQE_PCI_ERR1_BAD_RINGPTR		0x00000040
#define  SBQE_PCI_ERR1_RXFIFO_UNDERFLOW		0x00000020
#define  SBQE_PCI_ERR1_RXFIFO_OVERFLOW		0x00000010
#define  SBQE_PCI_ERR1_GET_DROP_SMALL_PKT(r)    (((r) >> 0) & 0x0f)

/*
 * PCI control register
 */
#define SBQE_PC_CORE_RESET              0x000c
#define  SBQE_PC_CORE_RB_RESET          0x01000000
#define SBQE_PCI_CONTROL 		0x0008
#define  SBQE_PCI_CONTROL_CII_ENABLE		0x00000001
#define  SBQE_PCI_CONTROL_CII_DISABLED_ACK	0x00000002


#define SBQE_PCI_COMPLETION_EVENT_FILTER 0x002c

/* Completion ring registers */
#define SBQE_PCI_RING_PTR		0x005c
#define SBQE_PCI_RING_SIZE		0x0060
#define SBQE_PCI_RING_CONSUMER		0x0064
#define SBQE_PCI_RING_PRODUCER		0x0068

/* Completion ring entry */
#define SBQE_PCI_RING_GET_TX(r)		(((r) >> 31) & 1)
#define SBQE_PCI_RING_GET_ERROR(r)	(((r) >> 30) & 1)
#define SBQE_PCI_RING_GET_OWNER(r)      (((r) >> 29) & 1)
#define SBQE_PCI_RING_GET_NUM_BUFS(r)	(((r) >> 14) & 0x3f)
#define SBQE_PCI_RING_GET_PKT_LEN(r)	(((r) >> 0) & 0x3fff)

#define SBQE_PCI_RING_ENTRY_SIZE	4

/* Rx buffer registers */
#define SBQE_PCI_RXBUF_SIZE		0x006c
#define  SBQE_PCI_RXBUF_SIZE_256		0x0
#define  SBQE_PCI_RXBUF_SIZE_512		0x1
#define  SBQE_PCI_RXBUF_SIZE_1024		0x2
#define  SBQE_PCI_RXBUF_SIZE_2048		0x3
#define  SBQE_PCI_RXBUF_SIZE_4096		0x4
#define  SBQE_PCI_RXBUF_SIZE_8192		0x5
#define  SBQE_PCI_RXBUF_SIZE_16384		0x6
#define SBQE_PCI_RXBUF_LOAD_0		0x0070

/* Tx ring registers */
#define SBQE_PCI_TXRING_PTR		0x00f0
#define SBQE_PCI_TXRING_SIZE		0x00f4
#define SBQE_PCI_TXRING_PRODUCER	0x00f8
#define SBQE_PCI_TXRING_CONSUMER	0x00fc

/*
 * A Tx ring entry has 4 words, where the first word has control info,
 * and the other 3 words (sometimes) data.
 */
#define SBQE_PCI_TXRING_CTL_OFFS	0 /* words */
#define  SBQE_PCI_TXRING_GET_TYPE(r)		(((r) >> 31) & 1)
#define  SBQE_PCI_TXRING_GET_SOP(r)		(((r) >> 30) & 1)
#define  SBQE_PCI_TXRING_GET_EOP(r)		(((r) >> 29) & 1)
#define  SBQE_PCI_TXRING_GET_LEN(r)		(((r) >> 8) & 0x7ff)
#define  SBQE_PCI_TXRING_MK(type, sop, eop, len) \
	(((type) << 31) | ((sop) << 30) | ((eop) << 29) | (len) << 8)
#define  SBQE_PCI_TYPE_SG		0
#define  SBQE_PCI_TYPE_IMM		1
#define SBQE_PCI_TXRING_ADDR_OFFS	1 /* words */
#define SBQE_PCI_TXRING_DATA_OFFS	1 /* words */

struct sbqe_txring_entry {
	uint32 hdr[4];
};
#define SBQE_PCI_TXRING_ENTRY_SIZE	16

/*
 * Statistics
 */
#define SBQE_PCI_RXSTATS_CTL    0x0048
#define SBQE_PCI_CNT_STATS_INIT 0x004c
#define SBQE_PCI_RXSTATS_ADDR   0x0050

#define SBQE_PCI_RXSTATS_DIS 0x00000001
#define SBQE_PCI_RXSTATS_ENB 0x00000000

/*
 * RX Fifo Debug Register
 */
#define SBQE_PC_RXBUF_FIFO_DEBUG 0x013C
#define SBQE_PC_RXBUF_FIFO_DEBUG_POP 0x2
#define SBQE_PC_RXBUF_FIFO_DEBUG_MT  0x1



/*
 * Packets to/from the QE have a route header attached.
 * The header is two words long. Following the route header
 * is a one word shim header.
 */
struct sbqe_header {
	uint32	rthdr0;
	uint32	rthdr1;
	uint32	shimhdr;
};
#define SBQE_HEADER_SIZE 12

/* rthdr0 */
#define SBQE_RT_HDR_GET_LEN(r)			(((r) >> 0) & 0x3fff)
#define SBQE_RT_HDR_GET_T(r)			(((r) >> 14) & 0x1)
#define SBQE_RT_HDR_GET_E(r)			(((r) >> 15) & 0x1)
#define SBQE_RT_HDR_GET_DP(r)			(((r) >> 16) & 0x3)
#define SBQE_RT_HDR_GET_COS(r)			(((r) >> 18) & 0x7)
#define SBQE_RT_HDR_GET_PORT(r)			(((r) >> 21) & 0x1f)
#define SBQE_RT_HDR_GET_NODE(r)			(((r) >> 26) & 0x3f)
#define SBQE_RT_HDR_MAKE_0(node, port, cos, dp, ecn, t, l2len) \
	(((node) << 26) | ((port) << 21) | ((cos) << 18) | ((dp) << 16) | ((ecn) << 15) | ((t) << 14) | ((l2len) << 0))

/* rthdr1 */
#define SBQE_RT_HDR_GET_OUT_HDR_INDEX(r)	(((r) >> 8) & 0x1fff)
#define SBQE_RT_HDR_GET_LCL_DP(r)		(((r) >> 21) & 0x3)
#define SBQE_RT_HDR_GET_LCL_COS(r)		(((r) >> 23) & 0x7)
#define SBQE_RT_HDR_GET_SH(r)			(((r) >> 26) & 0x1)
#define SBQE_RT_HDR_GET_MC(r)			(((r) >> 27) & 0x1)
#define SBQE_RT_HDR_MAKE_1(mc, sh, lclCos, lclDp, oix) \
	(((mc) << 27) | ((sh) << 26) | ((lclCos) << 23) | ((lclDp) << 21) | ((oix) << 8))

/* shimhdr */
#define SBQE_SHIM_GET_INGRTTL(r)		(((r) >>  0) & 0xff)
#define SBQE_SHIM_GET_S(r)			(((r) >>  8) & 0x01)
#define SBQE_SHIM_GET_OIX(r)			(((r) >> 12) & 0x1ffff)
#define SBQE_SHIM_GET_SWOP(r)			(((r) >> 30) & 0x03)
#define SBQE_RT_HDR_MAKE_SHIM(swop, oix, s, ingrTTL) \
	(((swop) << 30) | ((oix) << 12) | ((s) << 8) | ((ingrTTL) << 0))

#define SBQE_SHIM_EX_GET_EXIDX(r)		(((r) >> 12) & 0x7f)
#define SBQE_SHIM_EX_GET_RXNODE(r)		(((r) >> 19) & 0x3f)
#define SBQE_SHIM_EX_GET_RXPORT(r)		(((r) >> 25) & 0x1f)
#define SBQE_RT_HDR_MAKE_SHIM_EX(swop, rxPort, rxNode, eix, s, ingrTTL) \
	(((swop) << 30) | ((rxPort) << 25) | ((rxNode) << 19) | ((eix) << 11) | ((s) << 8) | ((ingrTTL) << 0))

#define SBQE_STUFF_PAD 2	/* number of pad bytes when S is set */

#define SWOP_NORMAL 1
#define SWOP_BRIDGED 2

#define SBQE_MAX_PORT_NO 18
#define SBQE_MAX_NODE_NO 16	/* XXX not true with the new MA */

#define SBQE_PORT_PROC 17
#define SBQE_PORT_MC 18

/** Exceptions **/

#define SBQE_ING_EXC_EMPTY_RECORD         (0x00)
#define SBQE_ING_EXC_SPI4_RX_DIS_PORT_ERR (0x01)
#define SBQE_ING_EXC_SPI4_RX_DPAR_ERR     (0x02)
#define SBQE_ING_EXC_SPI4_RX_FRM_ERR      (0x03)
#define SBQE_ING_EXC_SPI4_RX_PHY_ERR      (0x04)
#define SBQE_ING_EXC_SPI4_RX_MIN_PKT      (0x05)
#define SBQE_ING_EXC_SPI4_RX_MRU_EXC      (0x06)
#define SBQE_ING_EXC_SPI4_RX_OVFL         (0x07)
#define SBQE_ING_EXC_L2_PARSE_ERR         (0x08)
#define SBQE_ING_EXC_TEST_DROP            (0x09)
#define SBQE_ING_EXC_INV_PPP_ADDR_CTL     (0x10)
#define SBQE_ING_EXC_INV_PPP_PID          (0x11)
#define SBQE_ING_EXC_IEEE_8021D           (0x14)
#define SBQE_ING_EXC_NON_CFI_ENET         (0x15)
#define SBQE_ING_EXC_DIS_BRIDGE           (0x16)
#define SBQE_ING_EXC_UNK_USR_TYPE         (0x18)
#define SBQE_ING_EXC_DIS_IPV4_ROUTING     (0x19)
#define SBQE_ING_EXC_DIS_MPLS_FORWARD     (0x1A)
#define SBQE_ING_EXC_UNK_MPLS_LBL         (0x1B)
#define SBQE_ING_EXC_THIRD_LBL_NOT_RT     (0x1C)
#define SBQE_ING_EXC_BEOM_LEN_ERR         (0x1D)
#define SBQE_ING_EXC_STACKED_SNAP         (0x1E)
#define SBQE_ING_EXC_IPV4_RUNT_PKT        (0x20)
#define SBQE_ING_EXC_IPV4_OPTIONS         (0x21)
#define SBQE_ING_EXC_INV_IPV4_CHECKSUM    (0x22)
#define SBQE_ING_EXC_INV_IPV4_VER         (0x23)
#define SBQE_ING_EXC_IPV4_RUNT_HDR        (0x24)
#define SBQE_ING_EXC_IPV4_LEN_ERR         (0x25)
#define SBQE_ING_EXC_IPV4_PKT_LEN_ERR     (0x26)
#define SBQE_ING_EXC_IPV4_INV_SA          (0x27)
#define SBQE_ING_EXC_IPV4_INV_DA          (0x28)
#define SBQE_ING_EXC_IPV4_USR_ADDR0       (0x30)
#define SBQE_ING_EXC_IPV4_USR_ADDR1       (0x31)
#define SBQE_ING_EXC_IPV4_USR_ADDR2       (0x32)
#define SBQE_ING_EXC_IPV4_USR_ADDR3       (0x33)
#define SBQE_ING_EXC_USR_TYPE0            (0x40)
#define SBQE_ING_EXC_USR_TYPE1            (0x41)
#define SBQE_ING_EXC_USR_TYPE2            (0x42)
#define SBQE_ING_EXC_USR_TYPE3            (0x43)
#define SBQE_ING_EXC_USR_TYPE4            (0x44)
#define SBQE_ING_EXC_USR_TYPE5            (0x45)
#define SBQE_ING_EXC_USR_TYPE6            (0x46)
#define SBQE_ING_EXC_USR_TYPE7            (0x47)
#define SBQE_ING_EXC_USR_TYPE8            (0x48)
#define SBQE_ING_EXC_USR_TYPE9            (0x49)
#define SBQE_ING_EXC_USR_TYPE10           (0x4A)
#define SBQE_ING_EXC_USR_TYPE11           (0x4B)
#define SBQE_ING_EXC_IPMC_MAC             (0x4C)
#define SBQE_ING_EXC_STP_STATE_DROP       (0x50)
#define SBQE_ING_EXC_SMAC_DROP            (0x51)
#define SBQE_ING_EXC_SMAC_UNKNOWN_DROP    (0x52)
#define SBQE_ING_EXC_ETH_DST_DROP         (0x53)
#define SBQE_ING_EXC_SMACDMAC_ON_SAMESEG  (0x54)
#define SBQE_ING_EXC_STP_LEARNING         (0x55)
#define SBQE_ING_EXC_IPV4_SA_DROP         (0x57)
#define SBQE_ING_EXC_SA_ON_WRONG_PORT     (0x58)
#define SBQE_ING_EXC_INV_FTE              (0x60)
#define SBQE_ING_EXC_MTU_EXCEEDED         (0x61)
#define SBQE_ING_EXC_TTL_EXPIRED          (0x62)
#define SBQE_ING_EXC_POLICER_DROP         (0x63)
#define SBQE_EGR_EXC_INV_ETE              (0x64)
#define SBQE_EGR_EXC_SPLIT_HORIZON        (0x65)
#define SBQE_EGR_EXC_PVID_DROP            (0x66)
#define SBQE_ING_EXC_DI_CHECK_FAIL        (0x67)
#define SBQE_ING_EXC_IPV4_MC_MISS         (0x68)

#define SBQE_EXC_IPV4 SBQE_ING_EXC_USR_TYPE0
#define SBQE_EXC_ARP SBQE_ING_EXC_USR_TYPE3

