/* $Id: bcmcore_l1cache.s,v 1.4 2011/07/21 16:14:24 yshtil Exp $ */
/*  *********************************************************************
    *  SB1250 Board Support Package
    *  
    *  L1C initialization			File: bcmcore_l1cache.S
    *  
    *  This module contains code for various CPU cache operations
    *  
    *  Note: the init routines in this module rely on registers only,
    *        since DRAM may not be active yet.
    *
    *  Author:  Mitch Lichtenberg
    *  
    *********************************************************************  
    *
    *  Copyright 2000,2001
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

		.text

		.set push	
		.set mips32

/*  *********************************************************************
    *  Macros
    ********************************************************************* */


#define _MM_MAKEMASK(v,n) (((1<<(v))-1) << (n))

#define S_CFG_IS	22
#define M_CFG_IS	_MM_MAKEMASK(3,S_CFG_IS)

#define S_CFG_IA	16
#define M_CFG_IA	_MM_MAKEMASK(3,S_CFG_IA)

#define S_CFG_IL	19
#define M_CFG_IL	_MM_MAKEMASK(3,S_CFG_IL)

#define S_CFG_DS	13
#define M_CFG_DS	_MM_MAKEMASK(3,S_CFG_DS)

#define S_CFG_DA	7
#define M_CFG_DA	_MM_MAKEMASK(3,S_CFG_DA)

#define S_CFG_DL	10
#define M_CFG_DL	_MM_MAKEMASK(3,S_CFG_DL)


/*  *********************************************************************
    *  BCMCORE_L1CACHE_INIT()
    *  
    *  Initialize the L1 Cache
    *  
    *  Input parameters: 
    *  	   nothing
    *  	   
    *  Return value:
    *  	   nothing
    *  
    *  Registers used:
    *  	   t0,t1,t2
    ********************************************************************* */

       	.ent	sysCacheInit
        .set    noreorder
sysCacheInit:	

	/*
	 * Size the icache
	 */

		mfc0	t0,C0_CONFIG,1

	/* 
	 * Sets Per Way = 64 << CFG_IS
	 */

		li	t2,M_CFG_IS
		and	t2,t0
		srl	t2,S_CFG_IS
		li	t1,64
		sll	t1,t2

	/*
	 * Associativity = 1 + CFG_IA
	 */

		li	t2,M_CFG_IA
		and	t2,t0
		srl	t2,S_CFG_IA
		addiu	t2,1

	/* 
	 * Associativity * Sets Per Way
	 */

		multu	t1,t2
		mflo	t1
	
	/* 
	 * Line Size = 2 << CFG_IL
	 */

		li	t2,M_CFG_IL
		and	t2,t0
		srl	t2,S_CFG_IL
		li	t0,2
		sll	t2,t0,t2
		
	/* 
	 * Instruction Cache Size = 
	 * Associativity * Line Size * Sets Per Way
	 */

		multu	t1,t2
		mflo	t0

	/*
	 * Zero the icache
	 */

		mtc0	zero,C0_TAGLO
		mtc0	zero,C0_TAGHI
		li	t1,K0BASE
1:		sub	t0,t0,t2
		cache	Index_Store_Tag_I,0(t1)
		add	t1,t2
		bne	t0,zero,1b
		nop

	/*
	 * Size the dcache
	 */

		mfc0	t0,C0_CONFIG,1

	/* 
	 * Sets Per Way = 64 << CFG_DS
	 */

		li	t2,M_CFG_DS
		and	t2,t0
		srl	t2,S_CFG_DS
		li	t1,64
		sll	t1,t2


	/*
	 * Associativity = 1 + CFG_DA
	 */

		li	t2,M_CFG_DA
		and	t2,t0
		srl	t2,S_CFG_DA
		addiu	t2,1

	/* 
	 * Associativity * Sets Per Way
	 */

		multu	t1,t2
		mflo	t1
	
	/* 
	 * Line Size = 2 << CFG_DL
	 */

		li	t2,M_CFG_DL
		and	t2,t0
		srl	t2,S_CFG_DL
		li	t0,2
		sll	t2,t0,t2
		
	/* 
	 * Data Cache Size = 
	 * Associativity * Line Size * Sets Per Way
	 */

		multu	t1,t2
		mflo	t0

	/*
	 * Zero the dcache
	 */

		mtc0	zero,C0_TAGLO,2
		mtc0	zero,C0_TAGHI,2
		li	t1,K0BASE
1:		sub	t0,t0,t2
		cache	Index_Store_Tag_D,0(t1)
		add	t1,t2
		bne	t0,zero,1b
		nop

		jr	ra

        .set    reorder
      	.end	sysCacheInit

		.set pop

/*  *********************************************************************
    *  End
    ********************************************************************* */

