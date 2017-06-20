/*  *********************************************************************
    *  
    *  SDRAM init module            File: tlbmap.S
    *
    *  BCM5300x TLB mapping for high address of PCIE, DDR2 and flash
    *
    *********************************************************************  
    *
    *  Copyright 2009,2010,2011
    *  Broadcom Corporation. All rights reserved.
    *  
    *  This software is furnished under license and may be used and 
    *  copied only in accordance with the following terms and 
    *  conditions.  Subject to these conditions, you may download, 
    *  copy, install, use, modify and distribute modified or unmodified 
    *  copies of this software in source and/or binary form.  No title 
    *  or ownership is transferred hereby.
    *  
    *  1) Any source code used, modified or distributed must reproduce 
    *     and retain this copyright notice and list of conditions 
    *     as they appear in the source file.
    *  
    *  2) No right is granted to use any trade name, trademark, or 
    *     logo of Broadcom Corporation.  The "Broadcom Corporation" 
    *     name may not be used to endorse or promote products derived 
    *     from this software without the prior written permission of 
    *     Broadcom Corporation.
    *  
    *  3) THIS SOFTWARE IS PROVIDED "AS-IS" AND ANY EXPRESS OR
    *     IMPLIED WARRANTIES, INCLUDING BUT NOT LIMITED TO, ANY IMPLIED
    *     WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR 
    *     PURPOSE, OR NON-INFRINGEMENT ARE DISCLAIMED. IN NO EVENT 
    *     SHALL BROADCOM BE LIABLE FOR ANY DAMAGES WHATSOEVER, AND IN 
    *     PARTICULAR, BROADCOM SHALL NOT BE LIABLE FOR DIRECT, INDIRECT,
    *     INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES 
    *     (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
    *     GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
    *     BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY 
    *     OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR 
    *     TORT (INCLUDING NEGLIGENCE OR OTHERWISE), EVEN IF ADVISED OF 
    *     THE POSSIBILITY OF SUCH DAMAGE.
    ********************************************************************* */

#include "bcm53000.h"

#ifdef _MIPS_ARCH_MIPS32R2
#define HAZARD  ehb
#else
#define HAZARD  ssnop; ssnop; ssnop; ssnop; ssnop; ssnop; ssnop; ssnop;
#endif

    .globl tlb_map_init
/* $Id: tlbmap.s,v 1.2 2011/07/21 16:14:28 yshtil Exp $
 * Registers used: t0-t6
 */
    .ent tlb_map_init
tlb_map_init:
    .set    noreorder
    
#define TMP         t0
#define INDEX       t1
#define PAGEMASK    t2
#define ENTRYHI     t3
#define ENTRYLO0    t4
#define ENTRYLO1    t5

#define FLAGS_UNCACHED  (0x17)
#define FLAGS_CACHED_WT (0x07)
#define FLAGS_CACHED_WB (0x1f)
#define FLAGS_INVALID   (0x00)


    /*
     * Because of the complexity of alignment requirement, 
     * it's not that easy to support compile-time auto configuration
     * with supplied address/size parameters (for different platforms).
     * We hard-coded the addresses and page sizes here for the time being.
     */    

    move    t6, ra
    
    /* Index */
    li      INDEX, 0

    /*
     * PCIE1
     */
    li      PAGEMASK, SI_TLB_PCIE1_PAGEMASK
    li      ENTRYHI, SI_TLB_PCIE1_VIRT_UNCACHED
    li      ENTRYLO0, ((SI_TLB_PCIE1_PHYS >> 6) | FLAGS_UNCACHED)
    li      ENTRYLO1, (((SI_TLB_PCIE1_PHYS + SI_TLB_PCIE1_PAGESIZE) >> 6) \
                      | FLAGS_UNCACHED)
    la      TMP, tlb_map
    jalr    TMP
    nop
    li      ENTRYHI, SI_TLB_PCIE1_VIRT_CACHED
    li      ENTRYLO0, ((SI_TLB_PCIE1_PHYS >> 6) | FLAGS_CACHED_WT)
    li      ENTRYLO1, (((SI_TLB_PCIE1_PHYS + SI_TLB_PCIE1_PAGESIZE) >> 6) \
                      | FLAGS_CACHED_WT)
    la      TMP, tlb_map
    jalr    TMP
    nop

    /*
     * Flash
     */
    li      PAGEMASK, SI_TLB_FLASH_PAGEMASK
    li      ENTRYHI, SI_TLB_FLASH_VIRT_UNCACHED - SI_TLB_FLASH_PAGESIZE
    li      ENTRYLO0, ((0x10000000 >> 6) | FLAGS_INVALID)
    li      ENTRYLO1, ((SI_TLB_FLASH_PHYS >> 6) | FLAGS_UNCACHED)
    la      TMP, tlb_map
    jalr    TMP
    nop
    li      ENTRYHI, SI_TLB_FLASH_VIRT_CACHED - SI_TLB_FLASH_PAGESIZE
    li      ENTRYLO0, ((0x10000000 >> 6) | FLAGS_INVALID)
    li      ENTRYLO1, ((SI_TLB_FLASH_PHYS >> 6) | FLAGS_CACHED_WT)
    la      TMP, tlb_map
    jalr    TMP
    nop

    /*
     * DDR2
     */
    li      PAGEMASK, SI_TLB_SDRAM_PAGEMASK
    li      ENTRYHI, SI_TLB_SDRAM_VIRT_UNCACHED
    li      ENTRYLO0, ((SI_TLB_SDRAM_PHYS >> 6) | FLAGS_UNCACHED)
    li      ENTRYLO1, ((0x90000000 >> 6) | FLAGS_UNCACHED)
    la      TMP, tlb_map
    jalr    TMP
    nop
    li      ENTRYHI, SI_TLB_SDRAM_VIRT_CACHED
    li      ENTRYLO0, ((SI_TLB_SDRAM_PHYS >> 6) | FLAGS_CACHED_WT)
    li      ENTRYLO1, ((0x90000000 >> 6) | FLAGS_CACHED_WT)
    la      TMP, tlb_map
    jalr    TMP
    nop

    jr      t6
    nop
    
tlb_map:

    /* page size */
    move    TMP, PAGEMASK
    mtc0    TMP, C0_PAGEMASK
    HAZARD
    
    /* EntryHi: VFN2 */
    move    TMP, ENTRYHI
    mtc0    TMP, C0_TLBHI
    HAZARD
    
    /* EntryL0: even PFN + flags */
    move    TMP, ENTRYLO0
    mtc0    TMP, C0_TLBLO0
    HAZARD
    
    /* EntryL1: odd PFN + flags */
    move    TMP, ENTRYLO1
    mtc0    TMP, C0_TLBLO1
    HAZARD
    
    /* Write Index */
    mtc0    INDEX, C0_INX
    HAZARD
    
    /* Write to TLB entry */
    tlbwi
    HAZARD
    
    /* Increase index */
    add     INDEX, INDEX, 1

    /* Set wired entry count */
    mtc0    INDEX, C0_WIRED
    HAZARD
    
    jr      ra
    nop

    .set    reorder
    .end tlb_map_init
