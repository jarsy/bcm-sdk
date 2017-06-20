/*
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * $Id: bcm53000.h,v 1.2 Broadcom SDK $
 */

#ifndef _BCM53000_H_
#define _BCM53000_H_

/* Define the chip, to match legacy #ifdef's. */
#define BCM53000  1

/* Define the chip family */
#define BCM47XX  1

/* BCM53000 Address map */
#define BCM53000_SDRAM      0x00000000 /* 0-128MB Physical SDRAM */
#define BCM53000_SDRAM_SWAP 0x10000000 /* Byteswapped Physical SDRAM */
#define BCM53000_SDRAM2     0x80000000 /* Large 256MB SDRAM region */
#define BCM53000_PCI_MEM    0x08000000 /* Host Mode PCI mem space (128MB)*/
#define BCM53000_PCI_DMA    0x40000000 /* Client Mode PCI mem space (1GB)*/
#define BCM53000_ENUM       0x18000000 /* Beginning of core enum space */
#define BCM53000_FLASH      0x1fc00000 /* Flash region 0 for MIPS boot */
#define BCM53000_FLASH2     0x1c000000 /* Flash region 1 (60MB) */
#define BCM53000_FLASH3     0x20000000 /* Large 256MB flash region */

/* BCM53000 Core register space */
#define BCM53000_REG_CHIPC      0x18000000 /* Chipcommon  registers */
#define BCM53000_REG_GMACC      0x18001000 /* GMAC common core registers */
#define BCM53000_REG_GMAC0      0x18002000 /* GMAC0 core registers */
#define BCM53000_REG_MIPS74K    0x18003000 /* MIPS 74K core registers */
#define BCM53000_REG_USB0       0x18004000 /* USB core registers */
#define BCM53000_REG_PCIE0      0x18005000 /* PCIe 0 core registers */
#define BCM53000_REG_MEMC       0x18006000 /* MEMC core registers */
#define BCM53000_REG_SOCRAM     0x18007000 /* SOCRAM core registers */
#define BCM53000_REG_ALTA       0x18008000 /* ALTA Core registers */
#define BCM53000_REG_USB1       0x18009000 /* USB core registers */
#define BCM53000_REG_PL301B     0x1800a000 /* PL301b core registers */
#define BCM53000_REG_PL301C     0x1800b000 /* PL301c core registers */
#define BCM53000_REG_PL301D     0x1800c000 /* PL301d core registers */
#define BCM53000_REG_GMAC1      0x1800d000 /* GMAC1 core registers */
#define BCM53000_REG_PCIE1      0x1800e000 /* PCIe 1 core registers */
#define BCM53000_REG_DDRPHY     0x1800f000 /* DDR PHY core registers */

#define BCM53000_REG_UARTS       (BCM53000_REG_CHIPC + 0x300) /* UART regs */

#define BCM53000_EJTAG      0xff200000 /* MIPS EJTAG space (2M) */

/* Internal 16550-compatible UARTs */
#define BCM53000_UART0      (BCM53000_REG_UARTS + 0x00000000)
#define BCM53000_UART1      (BCM53000_REG_UARTS + 0x00000100)

/* Registers common to MIPS74 Core */
#define MIPS74K_FLASH_REGION_AUX       0x1C000000 /* FLASH Region 2*/
#define MIPS74K_FLASH_REGION           0x1FC00000 /* Boot FLASH Region  */

/* bcm4704 mapping to generic sb_bp identifiers */
/* XXX It would be better to discover this dynamically. */

/* BSP Abstraction, pickup names via bsp_config.h. */

#define AI_ENUM_BASE            BCM53000_ENUM

#define AI_CHIPC_BASE           BCM53000_REG_CHIPC
#define AI_GMACC_BASE           BCM53000_REG_GMACC
#define AI_GMAC0_BASE           BCM53000_REG_GMAC0
#define AI_MIPS74K_BASE         BCM53000_REG_MIPS74K
#define AI_USB0_BASE            BCM53000_REG_USB0
#define AI_PCIE0_BASE           BCM53000_REG_PCIE0
#define AI_MEMC_BASE            BCM53000_REG_MEMC
#define AI_SOCRAM_BASE          BCM53000_REG_SOCRAM
#define AI_ALTA_BASE            BCM53000_REG_ALTA
#define AI_USB1_BASE            BCM53000_REG_USB1
#define AI_PL301B_BASE          BCM53000_REG_PL301B
#define AI_PL301C_BASE          BCM53000_REG_PL301C
#define AI_PL301D_BASE          BCM53000_REG_PL301D
#define AI_GMAC1_BASE           BCM53000_REG_GMAC1
#define AI_PCIE1_BASE           BCM53000_REG_PCIE1
#define AI_DDRPHY_BASE          BCM53000_REG_DDRPHY

#define AI_FLASH_SPACE          MIPS74K_FLASH_REGION

/* some code need this definition */
#define SB_ENUM_BASE            BCM53000_ENUM
#define SB_CHIPC_BASE           BCM53000_REG_CHIPC
#define SB_MIPS33_BASE          BCM53000_REG_MIPS74K
#define SB_PCI_BASE             BCM53000_REG_PCIE0
#define SB_ENET0_BASE           BCM53000_REG_GMAC0

/* 
 * TLB map to access PCIE/DDR2/flash beyond KSEG 
 *
 *  PCIE1
 *      0x40000000 -> 0x47ffffff    physical
 *      0xe0000000 -> 0xe7ffffff    virtual (uncached)
 *      0xc0000000 -> 0xc7ffffff    virtual (cached)
 *      length: 128MB
 *          
 *  Flash
 *      0x20000000 -> 0x2fffffff    physical
 *      0x30000000 -> 0x3fffffff    virtual (uncached)
 *      0x10000000 -> 0x1fffffff    virtual (cached)
 *      length: 256MB
 *          
 *  DDR2
 *      0x80000000 -> 0x8fffffff    physical
 *      0x60000000 -> 0x6fffffff    virtual (uncached)
 *      0x40000000 -> 0x4fffffff    virtual (cached)
 *      length: 256MB
 */
#define SI_TLB_PCIE1_PHYS           BCM53000_PCI_DMA
#define SI_TLB_PCIE1_VIRT_UNCACHED  0xe0000000
#define SI_TLB_PCIE1_VIRT_CACHED    0xc0000000
#define SI_TLB_PCIE1_SIZE           128*1024*1024
#define SI_TLB_PCIE1_PAGESIZE       64*1024*1024
#define SI_TLB_PCIE1_PAGEMASK       0x07FFE000
#define SI_TLB_FLASH_PHYS           BCM53000_FLASH3
#define SI_TLB_FLASH_VIRT_UNCACHED  0x30000000
#define SI_TLB_FLASH_VIRT_CACHED    0x10000000
#define SI_TLB_FLASH_SIZE           256*1024*1024
#define SI_TLB_FLASH_PAGESIZE       256*1024*1024
#define SI_TLB_FLASH_PAGEMASK       0x1FFFE000
#define SI_TLB_SDRAM_PHYS           BCM53000_SDRAM2
#define SI_TLB_SDRAM_VIRT_UNCACHED  0x60000000
#define SI_TLB_SDRAM_VIRT_CACHED    0x40000000
#define SI_TLB_SDRAM_SIZE           256*1024*1024
#define SI_TLB_SDRAM_PAGESIZE       256*1024*1024
#define SI_TLB_SDRAM_PAGEMASK       0x1FFFE000

#define BCM53000_HIMEM_SIZE           512*1024*1024
#define BCM53000_SDRAM_SZ 0x08000000

/* Define it to support large flash region */
#define PHYS_TO_FLASH_UNCADDR(x)  \
    ((x) + (SI_TLB_FLASH_VIRT_UNCACHED - SI_TLB_FLASH_PHYS))

/* Required for dram init (which is assembly cde) before si_scan */
#define AI_MEMC_SLAVE_BASE      0x18106000

#endif /* _BCM53000_H_ */

