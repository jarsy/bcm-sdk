/* mot85xxPci.h - Wind River SBC8548 PCI Bridge setup header file */

/* $Id: mot85xxPci.h,v 1.2 2011/07/21 16:14:13 yshtil Exp $
 * Copyright (c) 2005-2006 Wind River Systems, Inc.
 *
 * The right to copy, distribute, modify or otherwise make use
 * of this software may be licensed only pursuant to the terms
 * of an applicable Wind River license agreement.
 */

/*
modification history
--------------------
01b,27jan06,dtr  Tidyup.
01a,04jul02,dtr  File created from cds85xx/01d.
*/

#ifndef _INCmot85xxPcih
#define _INCmot85xxPcih

#ifdef __cplusplus
    extern "C" {
#endif /* __cplusplus */


#define COMMAND_REGISTER_OFFSET         0x4
#define COMMAND_REGISTER_WIDTH          0x2
#define BRIDGE_BAR0_OFFSET              0x10
#define BRIDGE_BAR0_WIDTH               0x4

/* PCI 1 configuration space reg and int ack */

#define PCI_CFG_ADR_REG        (CCSBAR + 0x8000)
#define PCI_CFG_DATA_REG       (CCSBAR + 0x8004)
#define PCI_INT_ACK            (CCSBAR + 0x8008)

/* PCI 2 configuration space reg and int ack */

#define PCI2_CFG_ADR_REG        (CCSBAR + 0x9000)
#define PCI2_CFG_DATA_REG       (CCSBAR + 0x9004)
#define PCI2_INT_ACK            (CCSBAR + 0x9008)

/* PCI Express configuration space reg and int ack */

#define PCIEX_CFG_ADR_REG        (CCSBAR + 0xA000)
#define PCIEX_CFG_DATA_REG       (CCSBAR + 0xA004)

/* PCI 1 Outbound translation registers */

#undef CVPUINT32
#define CVPUINT32(base,addr)  (CAST(VUINT32 *)((base) + (addr)))

#define PCI_OUTBOUND_TRANS_ADRS_REG0(base)  CVPUINT32(base, 0x8c00)
#define PCI_OUTBOUND_TRANS_ADRS_REG1(base)  CVPUINT32(base, 0x8c20)
#define PCI_OUTBOUND_TRANS_ADRS_REG2(base)  CVPUINT32(base, 0x8c40)
#define PCI_OUTBOUND_TRANS_ADRS_REG3(base)  CVPUINT32(base, 0x8c60)
#define PCI_OUTBOUND_TRANS_ADRS_REG4(base)  CVPUINT32(base, 0x8c80)
#define PCI_OUTBOUND_TRANS_EXT_ADRS_REG0(base) CVPUINT32(base, 0x8c04)
#define PCI_OUTBOUND_TRANS_EXT_ADRS_REG1(base) CVPUINT32(base, 0x8c24)
#define PCI_OUTBOUND_TRANS_EXT_ADRS_REG2(base) CVPUINT32(base, 0x8c44)
#define PCI_OUTBOUND_TRANS_EXT_ADRS_REG3(base) CVPUINT32(base, 0x8c64)
#define PCI_OUTBOUND_TRANS_EXT_ADRS_REG4(base) CVPUINT32(base, 0x8c84)
#define PCI_OUTBOUND_BASE_ADRS_REG0(base) CVPUINT32(base, 0x8c08)
#define PCI_OUTBOUND_BASE_ADRS_REG1(base) CVPUINT32(base, 0x8c28)
#define PCI_OUTBOUND_BASE_ADRS_REG2(base) CVPUINT32(base, 0x8c48)
#define PCI_OUTBOUND_BASE_ADRS_REG3(base) CVPUINT32(base, 0x8c68)
#define PCI_OUTBOUND_BASE_ADRS_REG4(base) CVPUINT32(base, 0x8c88)

/* Outbound attributes register definitions */

#define PCI_OUTBOUND_ATTR_REG0(base)     CVPUINT32(base, 0x8c10)
#define PCI_OUTBOUND_ATTR_REG1(base)     CVPUINT32(base, 0x8c30)
#define PCI_OUTBOUND_ATTR_REG2(base)     CVPUINT32(base, 0x8c50)
#define PCI_OUTBOUND_ATTR_REG3(base)     CVPUINT32(base, 0x8c70)
#define PCI_OUTBOUND_ATTR_REG4(base)     CVPUINT32(base, 0x8c90)

/* PCI 2 Outbound translation registers */

#define PCI2_OUTBOUND_TRANS_ADRS_REG0(base)  CVPUINT32(base, 0x9c00)
#define PCI2_OUTBOUND_TRANS_ADRS_REG1(base)  CVPUINT32(base, 0x9c20)
#define PCI2_OUTBOUND_TRANS_ADRS_REG2(base)  CVPUINT32(base, 0x9c40)
#define PCI2_OUTBOUND_TRANS_ADRS_REG3(base) CVPUINT32(base, 0x9c60)
#define PCI2_OUTBOUND_TRANS_ADRS_REG4(base)  CVPUINT32(base, 0x9c80)
#define PCI2_OUTBOUND_TRANS_EXT_ADRS_REG0(base) CVPUINT32(base, 0x9c04)
#define PCI2_OUTBOUND_TRANS_EXT_ADRS_REG1(base) CVPUINT32(base, 0x9c24)
#define PCI2_OUTBOUND_TRANS_EXT_ADRS_REG2(base) CVPUINT32(base, 0x9c44)
#define PCI2_OUTBOUND_TRANS_EXT_ADRS_REG3(base) CVPUINT32(base, 0x9c64)
#define PCI2_OUTBOUND_TRANS_EXT_ADRS_REG4(base) CVPUINT32(base, 0x9c84)
#define PCI2_OUTBOUND_BASE_ADRS_REG0(base) CVPUINT32(base, 0x9c08)
#define PCI2_OUTBOUND_BASE_ADRS_REG1(base) CVPUINT32(base, 0x9c28)
#define PCI2_OUTBOUND_BASE_ADRS_REG2(base) CVPUINT32(base, 0x9c48)
#define PCI2_OUTBOUND_BASE_ADRS_REG3(base) CVPUINT32(base, 0x9c68)
#define PCI2_OUTBOUND_BASE_ADRS_REG4(base) CVPUINT32(base, 0x9c88)

/* Outbound attributes register definitions */

#define PCI2_OUTBOUND_ATTR_REG0(base) CVPUINT32(base, 0x9c10)
#define PCI2_OUTBOUND_ATTR_REG1(base) CVPUINT32(base, 0x9c30)
#define PCI2_OUTBOUND_ATTR_REG2(base) CVPUINT32(base, 0x9c50)
#define PCI2_OUTBOUND_ATTR_REG3(base) CVPUINT32(base, 0x9c70)
#define PCI2_OUTBOUND_ATTR_REG4(base) CVPUINT32(base, 0x9c90)

/* PCI Express Outbound translation registers */

#define PCIEX_OUTBOUND_TRANS_ADRS_REG0(base)     CVPUINT32(base, 0xac00)
#define PCIEX_OUTBOUND_TRANS_ADRS_REG1(base)     CVPUINT32(base, 0xac20)
#define PCIEX_OUTBOUND_TRANS_ADRS_REG2(base)     CVPUINT32(base, 0xac40)
#define PCIEX_OUTBOUND_TRANS_ADRS_REG3(base)     CVPUINT32(base, 0xac60)
#define PCIEX_OUTBOUND_TRANS_ADRS_REG4(base)     CVPUINT32(base, 0xac80)
#define PCIEX_OUTBOUND_TRANS_EXT_ADRS_REG0(base)     CVPUINT32(base, 0xac04)
#define PCIEX_OUTBOUND_TRANS_EXT_ADRS_REG1(base)     CVPUINT32(base, 0xac24)
#define PCIEX_OUTBOUND_TRANS_EXT_ADRS_REG2(base)     CVPUINT32(base, 0xac44)
#define PCIEX_OUTBOUND_TRANS_EXT_ADRS_REG3(base)     CVPUINT32(base, 0xac64)
#define PCIEX_OUTBOUND_TRANS_EXT_ADRS_REG4(base)     CVPUINT32(base, 0xac84)
#define PCIEX_OUTBOUND_BASE_ADRS_REG0(base)     CVPUINT32(base, 0xac08)
#define PCIEX_OUTBOUND_BASE_ADRS_REG1(base)     CVPUINT32(base, 0xac28)
#define PCIEX_OUTBOUND_BASE_ADRS_REG2(base)     CVPUINT32(base, 0xac48)
#define PCIEX_OUTBOUND_BASE_ADRS_REG3(base)     CVPUINT32(base, 0xac68)
#define PCIEX_OUTBOUND_BASE_ADRS_REG4(base)     CVPUINT32(base, 0xac88)

/* Outbound attributes register definitions */

#define PCIEX_OUTBOUND_ATTR_REG0(base)     CVPUINT32(base, 0xac10)
#define PCIEX_OUTBOUND_ATTR_REG1(base)     CVPUINT32(base, 0xac30)
#define PCIEX_OUTBOUND_ATTR_REG2(base)     CVPUINT32(base, 0xac50)
#define PCIEX_OUTBOUND_ATTR_REG3(base)     CVPUINT32(base, 0xac70)
#define PCIEX_OUTBOUND_ATTR_REG4(base)     CVPUINT32(base, 0xac90)

/* Outbound/Inbound Comparison mask register defines */

#define PCI_WINDOW_ENABLE_BIT 0x80000000
#define PCI_ATTR_BS_BIT       0x40000000
#define PCI_OUT_ATTR_RTT_MEM      0x00040000
#define PCI_OUT_ATTR_RTT_IO       0x00080000
#define PCI_OUT_ATTR_WTT_MEM      0x00004000
#define PCI_OUT_ATTR_WTT_IO       0x00008000
#define PCI_ATTR_WS_4K       0x0000000B
#define PCI_ATTR_WS_8K       0x0000000c
#define PCI_ATTR_WS_16K      0x0000000D
#define PCI_ATTR_WS_32K      0x0000000E
#define PCI_ATTR_WS_64K      0x0000000F
#define PCI_ATTR_WS_128K     0x00000010
#define PCI_ATTR_WS_256K     0x00000011
#define PCI_ATTR_WS_512K     0x00000012
#define PCI_ATTR_WS_1M       0x00000013
#define PCI_ATTR_WS_2M       0x00000014
#define PCI_ATTR_WS_4M       0x00000015
#define PCI_ATTR_WS_8M       0x00000016
#define PCI_ATTR_WS_16M      0x00000017
#define PCI_ATTR_WS_32M      0x00000018
#define PCI_ATTR_WS_64M      0x00000019
#define PCI_ATTR_WS_128M     0x0000001a
#define PCI_ATTR_WS_256M     0x0000001b
#define PCI_ATTR_WS_512M     0x0000001c
#define PCI_ATTR_WS_1G       0x0000001d
#define PCI_ATTR_WS_2G       0x0000001e
#define PCI_ATTR_WS_4G       0x0000001f
#define PCI_IN_ATTR_TGI_LM   0x00f00000
#define PCI_IN_ATTR_TGI_RIO  0x00c00000
#define PCI_IN_ATTR_RTT_RIO_READ 0x00040000
#define PCI_IN_ATTR_RTT_LM_READ_NO_SNOOP 0x00040000
#define PCI_IN_ATTR_RTT_LM_READ_SNOOP 0x00050000
#define PCI_IN_ATTR_RTT_LM_READ_UNLOCK_L2_CACHE_LINE 0x00070000
#define PCI_IN_ATTR_RTT_LM_WRITE_NO_SNOOP 0x00004000
#define PCI_IN_ATTR_RTT_LM_WRITE_SNOOP 0x00005000
#define PCI_IN_ATTR_RTT_LM_WRITE_ALLOC_L2_CACHE_LINE 0x00006000
#define PCI_IN_ATTR_RTT_LM_WRITE_ALLOC_LOCK_L2_CACHE_LINE 0x00007000

#define PCI_SNOOP_ENABLE        0x40000000
#define PCI_PREFETCHABLE        0x20000000

/* PCI 1 Inbound translation registers */

#define PCI_INBOUND_TRANS_ADRS_REG3(base)     CVPUINT32(base, 0x8DA0)
#define PCI_INBOUND_TRANS_EXT_ADRS_REG3(base) CVPUINT32(base, 0x8DA4)
#define PCI_INBOUND_BASE_ADRS_REG3(base)      CVPUINT32(base, 0x8DA8)
#define PCI_INBOUND_ATTR_REG3(base)           CVPUINT32(base, 0x8DB0)
#define PCI_INBOUND_TRANS_ADRS_REG2(base)     CVPUINT32(base, 0x8DC0)
#define PCI_INBOUND_BASE_ADRS_REG2(base)      CVPUINT32(base, 0x8DC8)
#define PCI_INBOUND_ATTR_REG2(base)           CVPUINT32(base, 0x8DD0)
#define PCI_INBOUND_TRANS_ADRS_REG1(base)     CVPUINT32(base, 0x8DE0)
#define PCI_INBOUND_BASE_ADRS_REG1(base)      CVPUINT32(base, 0x8DE8)
#define PCI_INBOUND_ATTR_REG1(base)           CVPUINT32(base, 0x8DF0)

/* PCI 2 Inbound translation registers */

#define PCI2_INBOUND_TRANS_ADRS_REG3(base)     CVPUINT32(base, 0x9DA0)
#define PCI2_INBOUND_TRANS_EXT_ADRS_REG3(base) CVPUINT32(base, 0x9DA4)
#define PCI2_INBOUND_BASE_ADRS_REG3(base)      CVPUINT32(base, 0x9DA8)
#define PCI2_INBOUND_ATTR_REG3(base)           CVPUINT32(base, 0x9DB0)
#define PCI2_INBOUND_TRANS_ADRS_REG2(base)     CVPUINT32(base, 0x9DC0)
#define PCI2_INBOUND_BASE_ADRS_REG2(base)      CVPUINT32(base, 0x9DC8)
#define PCI2_INBOUND_ATTR_REG2(base)           CVPUINT32(base, 0x9DD0)
#define PCI2_INBOUND_TRANS_ADRS_REG1(base)     CVPUINT32(base, 0x9DE0)
#define PCI2_INBOUND_BASE_ADRS_REG1(base)      CVPUINT32(base, 0x9DE8)
#define PCI2_INBOUND_ATTR_REG1(base)           CVPUINT32(base, 0x9DF0)

/* PCI Express Inbound translation registers */

#define PCIEX_INBOUND_TRANS_ADRS_REG3(base)     CVPUINT32(base, 0xaDA0)
#define PCIEX_INBOUND_TRANS_EXT_ADRS_REG3(base) CVPUINT32(base, 0xaDA4)
#define PCIEX_INBOUND_BASE_ADRS_REG3(base)      CVPUINT32(base, 0xaDA8)
#define PCIEX_INBOUND_ATTR_REG3(base)           CVPUINT32(base, 0xaDB0)
#define PCIEX_INBOUND_TRANS_ADRS_REG2(base)     CVPUINT32(base, 0xaDC0)
#define PCIEX_INBOUND_BASE_ADRS_REG2(base)      CVPUINT32(base, 0xaDC8)
#define PCIEX_INBOUND_ATTR_REG2(base)           CVPUINT32(base, 0xaDD0)
#define PCIEX_INBOUND_TRANS_ADRS_REG1(base)     CVPUINT32(base, 0xaDE0)
#define PCIEX_INBOUND_BASE_ADRS_REG1(base)      CVPUINT32(base, 0xaDE8)
#define PCIEX_INBOUND_ATTR_REG1(base)           CVPUINT32(base, 0xaDF0)

/* PCI 1 error Registers */

#define PCI_ERROR_DETECT_REG             0x8e00
#define PCI_ERROR_CAPTURE_DISABLE_REG    0x8e04
#define PCI_ERROR_ENABLE_REG             0x8e08
#define PCI_ERROR_ATTR_CAPTURE_REG       0x8e0c
#define PCI_ERROR_ADRS_CAPTURE_REG       0x8e10
#define PCI_ERROR_EXT_ADRS_CAPTURE_REG   0x8e14
#define PCI_ERROR_DATA_LOW_CAPTURE_REG   0x8e18
#define PCI_ERROR_DATA_HIGH_CAPTURE_REG  0x8e1c
#define PCI_ERROR_GASKET_TIMER_REG       0x8e20

/* Command status register defines */

#define BUS_MASTER_ENABLE_BIT   0x4
#define MEMORY_SPACE_ACCESS_ENABLE_BIT 0x2

#ifdef __cplusplus
    }
#endif /* __cplusplus */

#endif /* _INCmot85xxPcih */

