/*
    Copyright 2001, Broadcom Corporation
    All Rights Reserved.
    
    This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
    the contents of this file may not be disclosed to third parties, copied or
    duplicated in any form, in whole or in part, without the prior written
    permission of Broadcom Corporation.
*/
/*
 * $Id: cacheALib.s,v 1.1 2004/02/24 07:47:00 csm Exp $
 */

#define _ASMLANGUAGE


#include <vxWorks.h>
#include "asm.h"

#include "arch/mips/ivMips.h"
#include "arch/mips/asmMips.h"
#include "arch/mips/archMips.h"
#include "arch/mips/esfMips.h"

#define WAR_ADDR	0xb8005f9c

#define CACHE_OP( code, type )			( ((code) << 2) | (type) )

#define ICACHE_INDEX_INVALIDATE			CACHE_OP(0x0, 0)
#define ICACHE_INDEX_LOAD_TAG			CACHE_OP(0x1, 0)
#define ICACHE_INDEX_STORE_TAG			CACHE_OP(0x2, 0)
#define DCACHE_INDEX_WRITEBACK_INVALIDATE	CACHE_OP(0x0, 1)
#define DCACHE_INDEX_LOAD_TAG			CACHE_OP(0x1, 1)
#define DCACHE_INDEX_STORE_TAG			CACHE_OP(0x2, 1)

#define ICACHE_ADDR_HIT_INVALIDATE		CACHE_OP(0x4, 0)
#define ICACHE_ADDR_FILL			CACHE_OP(0x5, 0)
#define ICACHE_ADDR_FETCH_LOCK			CACHE_OP(0x7, 0)
#define DCACHE_ADDR_HIT_INVALIDATE		CACHE_OP(0x4, 1)
#define DCACHE_ADDR_HIT_WRITEBACK_INVALIDATE	CACHE_OP(0x5, 1)
#define DCACHE_ADDR_HIT_WRITEBACK		CACHE_OP(0x6, 1)
#define DCACHE_ADDR_FETCH_LOCK			CACHE_OP(0x7, 1)


#define CACHELINE_SIZE                  16
#define INSTRUCTION_CACHE_SIZE          (16*1024)
#define DATA_CACHE_SIZE                 (16*1024)
#define KSEG0BASE                       0x80000000
#define KSEG1BASE	                0xA0000000
#define C0_TAGLO                        $28
#define C0_TAGHI                        $29
#define	ALIGN_4W		        0xf

	.text

        .globl  FlushAll_Dcache
	.globl  FlushAll_Icache
        .globl  Flush_Dcache
        .globl  Invalidate_Dcache
        .globl  Clear_Dcache
	.globl  Invalidate_Icache
        .globl  readtag
        .globl  loadtag
        .globl  reset_caches
        

/************************************************************************/
/* FlushAll_Dcache: invaldates the entire data cache 		        */
/*									*/
/*	Syntax: void Sys_Flush_Dcache(void)				*/
/*	  Note:								*/
/************************************************************************/
    .ent    FlushAll_Dcache 
FlushAll_Dcache:
     
        .set    noreorder
    

        mfc0    t7, C0_SR           # save SR 
        nop
        and     t0, t7, ~SR_IE      # dissable interrupts 
        mtc0    t0, C0_SR
        nop
        
        li      t0, DATA_CACHE_SIZE
        li      t1, CACHELINE_SIZE 
	
	mtc0    zero, C0_TAGLO
	mtc0    zero, C0_TAGHI  	    /* TagHi is not really used */
	
	/* Calc an address that will correspond to the first cache line */
	li	t2, KSEG0BASE

	/* Calc an address that will correspond to the last cache line  */
	addu	t3, t2, t0
	subu    t3, t1

	li	t0, WAR_ADDR

	/* Loop through all lines, invalidating each of them */
1:	
	lw	t4, 0(t0)
	.set mips3
	cache	DCACHE_INDEX_WRITEBACK_INVALIDATE, 0(t2)	/* clear tag */
	.set mips0
	bne	t2, t3, 1b
	addu	t2, t1
                
        mtc0    t7, C0_SR           # restore SR
        nop

	jr	ra
	nop
                                                 
        .set    reorder
        .end    FlushAll_Dcache


/************************************************************************/
/************************************************************************/
/************************************************************************/


/************************************************************************/
/* FlushAll_Icache: invaldates the entire instruction  cache               */
/*									*/
/*	Syntax: void Flush_Icache(void)				        */
/*	  Note:								*/
/************************************************************************/
	.ent	FlushAll_Icache
FlushAll_Icache:	
	
        .set    noreorder
        
        mfc0    t7, C0_SR           # save SR 
        nop
        and     t0, t7, ~SR_IE      # dissable interrupts 
        mtc0    t0, C0_SR
        nop
        
        li      t0, INSTRUCTION_CACHE_SIZE
        li      t1, CACHELINE_SIZE 
	
	mtc0    zero, C0_TAGLO 
	mtc0    zero, C0_TAGHI 	    /* TagHi is not really used */
	
	/* Calc an address that will correspond to the first cache line */
	li	t2, KSEG0BASE

	/* Calc an address that will correspond to the last cache line  */
	addu	t3, t2, t0
	subu    t3, t1

	/* Loop through all lines, invalidating each of them */
1:
	.set mips3
	cache	ICACHE_INDEX_INVALIDATE, 0(t2)	/* clear tag */
	.set	mips0
	bne	t2, t3, 1b
	addu	t2, t1
        
        mtc0    t7, C0_SR           # restore SR
        nop
        
        j       ra
        nop
                
        .end	FlushAll_Icache
        .set	reorder
        
/************************************************************************/
/************************************************************************/
/************************************************************************/


/************************************************************************/
/* Invalidate_Icache: Invaldates a portion of the Instruction Cache     */
/*								       	*/
/*	Syntax: void Invalidate_Icache(ulong base_addr, ulong bcount    */
/*	  Note:								*/
/************************************************************************/
	.ent	Invalidate_Icache
Invalidate_Icache:	
 	        
                .set	noreorder
	mfc0	t7, C0_SR	# save SR
    nop
    nop
	and	t0, t7, ~SR_IE	# disable interrupts
	mtc0	t0, C0_SR
	nop

	move	t5, a0
	li	t1, ~ALIGN_4W
	and 	t5, t1		# align start of cache 16 bytes (line)

	        .set 	reorder
	li	t0, ~0xf0000000	# make sure that t5 is in Kseg0
	and	t5, t0
	li	t0, K0BASE
	or	t5, t0

	move	t4, a1

1:
	addu	t4, a0		# arg0(startaddr)+min(cachesize,arg1)
	li	t0, ~0xf0000000	# make sure that t4 is in Kseg0
	and	t4, t0
	li	t0, K0BASE
	or	t4, t0
	
	la	t0, 1f	       	# switch to Kseg1
	li	t1, K1BASE
	or	t0, t1
	j	t0
	nop

1:
	move	t0, t5		# move loop variable to t0

2:      
        .set    mips3        
	cache	ICACHE_ADDR_HIT_INVALIDATE, 0x0000(t0)	            
        .set    mips0
	addu	t0, t0, 16	# Add cacheline length
	bltu	t0, t4, 2b	# while index 
	nop

	# restore status register
	        .set	noreorder
	mtc0	t7, C0_SR	# restore SR
	nop
	j	ra
	nop
	.end	Invalidate_Icache
	        .set	reorder
        

/************************************************************************/
/************************************************************************/
/************************************************************************/
        
	.ent	Clear_Dcache
Clear_Dcache:  

	.set	noreorder
        
	mfc0	t7, C0_SR	# save SR
        nop
        nop
        
        and	t0, t7, ~SR_IE	# disable interrupts
	mtc0	t0, C0_SR
        nop
	
        .set 	reorder

	move	t5, a0
	li      t1, ~ALIGN_4W
	and 	t5, t1		# align start of region to 16 bytes

	li	t0, ~0xf0000000	# make sure that t5 is in Kseg0
	and	t5, t0
	li	t0, K0BASE
	or	t5, t0

	move	t4, a1

1:
	addu	t4, a0		# arg0(startaddr)+min(cachesize, arg1)
	li	t0, ~0xf0000000	# make sure that t4 is in Kseg0
	and	t4, t0
	li	t0, K0BASE
	or	t4, t0
 
	move	t0, t5		
        
        li      t2, WAR_ADDR

2:      # Clear out the Data Cache!!
        .set    mips3
        lw      t1, 0(t2)
	cache	DCACHE_ADDR_HIT_WRITEBACK_INVALIDATE, 0x0000(t0) 
        .set    mips0        
        addu	t0, t0, 16	# Add cacheline length
	bltu	t0, t4, 2b	# while index < endaddr
	nop
          
	# restore status register
	.set	noreorder
	mtc0	t7, C0_SR	# restore SR
	nop
	j	ra
	nop

	.set	reorder
	.end	Clear_Dcache


/************************************************************************/
/************************************************************************/
/************************************************************************/
        

/************************************************************************/
/************************************************************************/
/************************************************************************/
        


/************************************************************************/
/* Invalidate_Dcache: Invaldates a portion of the Data Cache             */
/*									*/
/*	Syntax: void Invalidate_Dcache(ulong base_addr, ulong bcount)	*/
/*	  Note:								*/
/************************************************************************/
	.ent	Invalidate_Dcache
Invalidate_Dcache:  

	.set	noreorder
        
	mfc0	t7, C0_SR	# save SR
        nop
        
        and	t0, t7, ~SR_IE	# disable interrupts
	mtc0	t0, C0_SR
        nop
	
        .set 	reorder

	move	t5, a0
	li      t1, ~ALIGN_4W
	and 	t5, t1		# align start of region to 16 bytes

	li	t0, ~0xf0000000	# make sure that t5 is in Kseg0
	and	t5, t0
	li	t0, K0BASE
	or	t5, t0

	move	t4, a1

1:
	addu	t4, a0		# arg0(startaddr)+min(cachesize, arg1)
	li	t0, ~0xf0000000	# make sure that t4 is in Kseg0
	and	t4, t0
	li	t0, K0BASE
	or	t4, t0

	move	t0, t5		        
       	
2:      # Clear out the Data Cache!!
        .set    mips3
	cache	DCACHE_ADDR_HIT_WRITEBACK_INVALIDATE, 0x0000(t0) 
        .set    mips0        
        addu	t0, t0, 16	# Add cacheline length
	bltu	t0, t4, 2b	# while index < endaddr
	nop
          
	# restore status register
	.set	noreorder
	mtc0	t7, C0_SR	# restore SR
	nop
	j	ra
	nop

	.set	reorder
	.end	Invalidate_Dcache


/************************************************************************/
/************************************************************************/
/************************************************************************/
        

/************************************************************************/
/************************************************************************/
/************************************************************************/

/* DEBUG CODE */        

        .ent    loadtag
loadtag:
        
        .set noreorder
        .set    mips3
	cache	DCACHE_INDEX_LOAD_TAG, 0(a0) 
        .set    mips0        
        
        j       ra
        nop
        
        .set noreorder
        .end loadtag    

        .ent    readtag
readtag:
        
        .set noreorder
        mfc0    v0, $28
        nop
        
        j       ra
        nop
        
        .set noreorder
        .end readtag    


/************************************************************************/
/* Flush_Dcache: Invaldates a portion of the Data Cache             */
/*									*/
/*	Syntax: void Flush_Dcache(ulong base_addr, ulong bcount)	*/
/*	  Note:								*/
/************************************************************************/
	.ent	Flush_Dcache
Flush_Dcache:  

	.set	noreorder
        
	mfc0	t7, C0_SR	# save SR
        nop
        
        and	t0, t7, ~SR_IE	# disable interrupts
	mtc0	t0, C0_SR
        nop
	
        .set 	reorder

	move	t5, a0
	li      t1, ~ALIGN_4W
	and 	t5, t1		# align start of region to 16 bytes

	li	t0, ~0xf0000000	# make sure that t5 is in Kseg0
	and	t5, t0
	li	t0, K0BASE
	or	t5, t0

	move	t4, a1

1:
	addu	t4, a0		# arg0(startaddr)+min(cachesize, arg1)
	li	t0, ~0xf0000000	# make sure that t4 is in Kseg0
	and	t4, t0
	li	t0, K0BASE
	or	t4, t0

	move	t0, t5	      
        
        li      t2, WAR_ADDR         
	
2:      # Clear out the Data Cache!!
        .set    mips3
        lw      t1, 0(t2)
#if 1  /* Jimmy, this is original */
	cache	DCACHE_ADDR_HIT_WRITEBACK, 0x0000(t0) 
#else  /* Try flush + invalidate */
	cache	DCACHE_ADDR_HIT_WRITEBACK_INVALIDATE, 0x0000(t0) 
#endif
        .set    mips0        
        addu	t0, t0, 16	# Add cacheline length
	bltu	t0, t4, 2b	# while index < endaddr
	nop
          
	# restore status register
	.set	noreorder
	mtc0	t7, C0_SR	# restore SR
	nop
	j	ra
	nop

	.set	reorder
	.end	Flush_Dcache

/************************************************************************/
/* reset_caches: Init the D & I cache                                   */
/*									*/
/************************************************************************/
	.ent	reset_caches
reset_caches: 

        /* init the data cache */
        .set noreorder

        li      t0, DATA_CACHE_SIZE
        li      t1, CACHELINE_SIZE 
	
	mtc0    zero, C0_TAGLO 
	mtc0    zero, C0_TAGHI 	    /* TagHi is not really used */
	
	/* Calc an address that will correspond to the first cache line */
	li	t2, KSEG0BASE

	/* Calc an address that will correspond to the last cache line  */
	addu	t3, t2, t0
	subu    t3, t1

	/* Loop through all lines, invalidating each of them */
1:	
	.set mips3
	cache	DCACHE_INDEX_STORE_TAG, 0(t2)	/* clear tag */
	.set mips0
	bne	t2, t3, 1b
	addu	t2, t1
        
        /* init the instr. cache */
        
        li      t0, INSTRUCTION_CACHE_SIZE
        li      t1, CACHELINE_SIZE 
	
	mtc0    zero, C0_TAGLO 
	mtc0    zero, C0_TAGHI 	    /* TagHi is not really used */
	
	/* Calc an address that will correspond to the first cache line */
	li	t2, KSEG0BASE

	/* Calc an address that will correspond to the last cache line  */
	addu	t3, t2, t0
	subu    t3, t1

	/* Loop through all lines, invalidating each of them */
1:
	.set mips3
	cache	ICACHE_INDEX_STORE_TAG, 0(t2)	/* clear tag */
	.set	mips0
	bne	t2, t3, 1b
	addu	t2, t1

	jr	ra
	nop

        .set reorder
        .end reset_caches

/************************************************************************/
/************************************************************************/
/************************************************************************/
