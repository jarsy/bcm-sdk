/* hurricane2.h - Hurricane2 board header file */

/*
 * Copyright (c) 2013 Wind River Systems, Inc.
 *
 * The right to copy, distribute, modify or otherwise make use
 * of this software may be licensed only pursuant to the terms
 * of an applicable Wind River license agreement.
 */

/*
modification history
--------------------
01a,23sep13,dnb  created
*/

/*
This file contains I/O address and related constants for the
Broadcom Hurricane2 board.
*/

#ifndef __INChurricane2_h
#define __INChurricane2_h

#ifdef __cplusplus
extern "C" {
#endif

/* 
   memory map:

0x00000000 - 0x07ffffff		DRAM aliased to 0x60000000-0x67ffffff
0x08000000 - 0x0fffffff         PCIe 0 address match region
0x18000000 - 0x180fffff         Core register region
0x19000000 - 0x1902ffff         ARMCore register region
0x19030000 - 0x1903ffff         PCIe 0 MSI writes address space region
0x1b000000 - 0x1b00ffff         Internal scratch RAM
0x1c000000 - 0x1dffffff         NAND Flash region
0x1e000000 - 0x1fffffff         Serial Flash Region
0x2a000000 - 0x2adfffff         ARM R5 ATCM BTCM instruction and data cache
0x40000000 - 0x47ffffff         PCIe 1 Address match region
0x48000000 - 0x4fffffff         PCIe 2 Address match region
0x60000000 - 0xdfffffff         DDR2/3 SDRAM Large Region
0xfffd0000 - 0xfffeffff         Internal Bootrom 
0xffff0000 - 0xffff043f         Internal SKU-ROM region
0xffff1000 - 0xffff1fff         Enumeration ROM Register Region

*/
 

/* DRAM */
#if 0
#define HURRICANE2_DRAM_BASE_VIRT      (0x60000000UL)
#define HURRICANE2_DRAM_BASE_PHYS      (0x60000000UL)
#else
#define HURRICANE2_DRAM_BASE_VIRT      (0x80000000UL)
#define HURRICANE2_DRAM_BASE_PHYS      (0x80000000UL)
#endif
#define HURRICANE2_DRAM_SIZE           (SZ_512M)

/* Handy sizes */

#define SZ_1K                       (0x00000400)
#define SZ_4K                       (0x00001000)
#define SZ_8K                       (0x00002000)
#define SZ_16K                      (0x00004000)
#define SZ_64K                      (0x00010000)
#define SZ_128K                     (0x00020000)
#define SZ_256K                     (0x00040000)
#define SZ_512K                     (0x00080000)
#define SZ_1M                       (0x00100000)
#define SZ_2M                       (0x00200000)
#define SZ_4M                       (0x00400000)
#define SZ_8M                       (0x00800000)
#define SZ_16M                      (0x01000000)
#define SZ_32M                      (0x02000000)
#define SZ_64M                      (0x04000000)
#define SZ_128M                     (0x08000000)
#define SZ_256M                     (0x10000000)
#define SZ_512M                     (0x20000000)
#define SZ_1G                       (0x40000000)
#define SZ_2G                       (0x80000000)
#define SCTLR_BE                    (0x02000000)

#define IPROC_SCU_CONTROL           (0x19020000)

#define IPROC_GIC1_BASE		    IPROC_SCU_CONTROL

#define IPROC_GICCPU_CONTROL        (IPROC_GIC1_BASE + 0x0100)
#define IPROC_GICDIST_REG_BASE      (IPROC_GIC1_BASE + 0x1000)
#define IPROC_GIC_CPU_PRIORITY	    (IPROC_GIC1_BASE + 0x0104)
#define IPROC_GIC_CPU_POINT	    (IPROC_GIC1_BASE + 0x0108)
#define IPROC_GIC_CPU_ACK	    (IPROC_GIC1_BASE + 0x010C)
#define IPROC_GIC_CPU_END_INTR	    (IPROC_GIC1_BASE + 0x0110)
#define IPROC_GIC_CPU_RUNNING	    (IPROC_GIC1_BASE + 0x0114)
#define IPROC_GIC_CPU_PENDING	    (IPROC_GIC1_BASE + 0x0118)

#define IPROC_GIC_DIST_ENABLE_CLR1  (0x1180)

#define IPROC_PERIPH_BASE	    (0x19020000)
#define IPROC_PERIPH_GLB_TIM_REG_BASE	(IPROC_PERIPH_BASE + 0x200)
#define IPROC_GLB_TIM_COUNT_LO      (IPROC_PERIPH_GLB_TIM_REG_BASE)
#define IPROC_GLB_TIM_COUNT_HI      (IPROC_PERIPH_GLB_TIM_REG_BASE + 4)
#define IPROC_GLB_TIM_CTRL_ADDR     (IPROC_PERIPH_GLB_TIM_REG_BASE + 8)   

#define IPROC_PERIPH_PVT_TIM_REG_BASE	(IPROC_PERIPH_BASE + 0x600)
#define L2_CTRL_BASE                (IPROC_PERIPH_BASE + 0x2000)

#define IPROC_CCA_REG_BASE          (0x18000000)
#define IPROC_CCA_UART0_REG_BASE    (IPROC_CCA_REG_BASE + 0x300)
#define IPROC_CCA_UART1_REG_BASE    (IPROC_CCA_REG_BASE + 0x400)


#define IPROC_CCB_TIM0_REG_BASE	    0x18034000
#define IPROC_CCB_TIM1_REG_BASE     0x18035000

#define IPROC_CCB_TIM0_0_INT        129
#define IPROC_CCB_TIM0_1_INT        130
#define IPROC_CCB_TIM1_0_INT        131
#define IPROC_CCB_TIM1_1_INT        132

#define IPROC_SMBUS_BASE_ADDR       0x18038000

#define IPROC_GMAC0_REGBASE         0x18022000
#define IPROC_GMAC0_INT             234
#define IPROC_GMAC0_PHY             1

/* interrupt information */ 
#define SYS_INT_LEVELS_MAX          256

#define INT_PIN_PVT_TIM             29
#define INT_PIN_UART0               123
#define INT_PIN_UART1               123

/* arbitrary numbers for now */
#define SYS_CLK_RATE_MIN            10
#define SYS_CLK_RATE_MAX            8000

#define IPROC_ARM_CLK               1000000000

#define IPROC_PCIE_AXIB0_REG_BASE	  (0x18012000)
#define IPROC_PCIE_AXIB1_REG_BASE	  (0x18013000)
#define IPROC_PCIE_AXIB2_REG_BASE	  (0x18014000)

#define IPROC_PCIE_INT		    218

#define QSPI_REG_BASE				0x18027000
#define CRU_CONTROL_REG          		0x1803e000

#define DMU_CRU_RESET				0x1803f200

#define IPROC_REGISTER_READ(reg) \
    *(volatile UINT32 *)(reg)

#define IPROC_REGISTER_WRITE(reg, data)\
    *(volatile UINT32 *)(reg) = (data)


#ifdef __cplusplus
}
#endif

#endif    /* __INChurricane2_h */
