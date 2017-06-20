/* archMips66.h - MIPS architecture specific header */

/* $Id: archMips66.h,v 1.3 2011/07/21 16:14:24 yshtil Exp $
 * Copyright (c) 2009 Wind River Systems, Inc.
 *
 * The right to copy, distribute, modify or otherwise make use
 * of this software may be licensed only pursuant to the terms
 * of an applicable Wind River license agreement.
 */

/*
modification history
--------------------
01a,12Oct09,mdg  written from 03w version of archMips.h
*/

#ifndef __INCarchMips66h
#define __INCarchMips66h

#ifdef __cplusplus
extern "C" {
#endif

/* MIPS32/MIPS64 CP0 Register 16 Select 1 defines */

#define CFG1_MMUMASK        0x7e000000   /* MMU size field */
#define CFG1_MMUSHIFT       25
#define CFG1_ISMASK         0x01c00000   /* icache sets per way */
#define CFG1_ISSHIFT        22
#define CFG1_ILMASK         0x00380000   /* icache line size */
#define CFG1_ILSHIFT        19
#define CFG1_IAMASK         0x00070000   /* icache associativity */
#define CFG1_IASHIFT        16
#define CFG1_DSMASK         0x0000e000   /* dcache sets per way */
#define CFG1_DSSHIFT        13
#define CFG1_DLMASK         0x00001c00   /* dcache line size */
#define CFG1_DLSHIFT        10
#define CFG1_DAMASK         0x00000380   /* dcache associativity */
#define CFG1_DASHIFT        7
#define CFG1_PC             0x00000010   /* perf counter implemented */
#define CFG1_WR             0x00000008   /* watch reg implemented */
#define CFG1_CA             0x00000004   /* code compression implemented */
#define CFG1_EP             0x00000002   /* EJTAG implemented */
#define CFG1_FP             0x00000001   /* FPU implemented */
#define CFG1_CACHE_SETS_PER_WAY     64   /* base size for sets per way shifted
                                          * left by the value in the CFG1
                                          * sets per way field
                                          */

#define CFG1_CACHE_LINE_SIZE         2   /* base cache size shifted left by
                                          * the value in the CFG1 cache line
                                          * size field
                                          */
/* Mask for Mips Address Space */

#define ADDRESS_SPACE_MASK	(~(7<<29))

/* MMU Definitions */          

#define TLB_4K_PAGE_SIZE        0x00001000
#define TLB_16K_PAGE_SIZE       0x00004000
#define TLB_64K_PAGE_SIZE       0x00010000
#define TLB_256K_PAGE_SIZE      0x00040000
#define TLB_1M_PAGE_SIZE        0x00100000
#define TLB_4M_PAGE_SIZE        0x00400000
#define TLB_16M_PAGE_SIZE       0x01000000

#define TLB_4K_PAGE_SIZE_MASK   0x00000000 
#define TLB_16K_PAGE_SIZE_MASK  0x00006000 
#define TLB_64K_PAGE_SIZE_MASK  0x0001E000 
#define TLB_256K_PAGE_SIZE_MASK 0x0007E000 
#define TLB_1M_PAGE_SIZE_MASK   0x001FE000 
#define TLB_4M_PAGE_SIZE_MASK   0x007FE000 
#define TLB_16M_PAGE_SIZE_MASK  0x01FFE000 

#define MMU_R4K_PAGE_SHIFT          12     /* convert VM pagenum to VA */


#ifdef __cplusplus
}
#endif

#endif /* __INCarchMips66h */

