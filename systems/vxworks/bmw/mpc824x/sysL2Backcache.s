
/* $Id: sysL2Backcache.s,v 1.2 2011/07/21 16:14:08 yshtil Exp $ */
/* file: l2cache.s - defines L2 cache functions
 * Modification history
 * --------------------
 * 15Sep98,My	   Added autosize for 256K backside, and detect MPC740. 
 * 31Jul98,My&RGP  Created
 */

/* includes */

#define _ASMLANGUAGE
#include "vxWorks.h"
#include "sysL2Backcache.h"



/* globals */

	.globl  sysL2BackGlobalInv
	.globl	sysL2BackDisable
	.globl  sysL2BackEnable
	.globl  sysL2BackAutoSize
	.globl	sysL1DcacheEnable
	.globl  sysPVRReadBoot
	.globl  sysL1DcacheDisable
	.globl	sysL2CRWrite
	

/*********************************************************************
 *   sysL2BackGlobalInv - Globally invalidate L2 backside cache 
 *      This function reads the value of the l2cr register, disables
 *      the cache, before setting the global invalidate bit.  It waits
 *      for the L2CR[L2IP] bit is clear before returning to the caller.
 *   Input  - None
 *   Return - None
 */

sysL2BackGlobalInv:

	sync
	mfspr	r3, L2CR_REG
	andis.	r3, r3, L2CR_DISABLE_MASK_U	/* Disable L2  */
        mtspr   L2CR_REG, r3
        sync

	oris	r3, r3, L2CR_GLOBAL_INV_U	/* Set global invalidate bit */
	mtspr	L2CR_REG, r3
	sync

/*  Monitoring the L2CR[L2IP] bit to determine when the global 
 *  invalidation operation is completed.
 */


invalidate_in_progress:
	mfspr   r3, L2CR_REG
	andi.   r3, r3, 0x1
	beq     invalidate_in_progress
	blr


/*********************************************************************
 *  sysL2BackEnable - Set and enable L2 backside cache
 *  Input  - the value to be set is passed in r3
 *  return - None
 */

sysL2BackEnable:
	mtspr   L2CR_REG, r3
	isync 

	oris	r3, r3, L2CR_EN_U		/* Set the enable bit */
	mtspr	L2CR_REG, r3			/* Store to l2cr register */
	blr  



/*********************************************************************
 *  sysL2BackDisable - Disable the L2 backside cache                            
 *     The value of the l2cr register is read, the enable bit is 
 *     disabled and written back to the l2cr register.
 *  Input  - None
 *  Output - None
 */

sysL2BackDisable:

	sync
	mfspr	r3, L2CR_REG		   	/* Read L2 control register */
	andis.	r3, r3, L2CR_DISABLE_MASK_U  	/* Disable L2 backside */
	mtspr   L2CR_REG, r3     
	sync       
	blr				   	/* return to caller */



/*********************************************************************
 *  sysL2BackAutoSize - Autosize L2 Backside cache 	   
 *     L2 backside cache is available in MPC750 only.  The L2 cache size
 *     could be 1M, 512K, or 256K.  The size is programmed in the L2CR
 *     register.  To detect the correct size, this function assumes that
 *     the L2 cache size is 1M, the cache is put in test mode so that a 
 *     cache flush operation (dcbf) will write from L1 cache to L2. 1M
 *     of data is written to memory, then read back.  If the cache size
 *     is truely 1M, all data read back will be correct as expected.  If
 *     the cache size is 512 KB, half of the data will be correct, and 
 *     the other half is not, and so on.  Once the size is determined,
 *     the value that must be set in L2CR when the L2 backside cache is
 *     enabled is stored in r3.
 * 
 *     There is no real way to distinguish MPC740 and MPC750; they both
 *     have the same PRV number.  After writing to L2 backside cache, if
 *     the result is neither 1M, 512 KB, or 256 KB, it must be MPC740
 *     (no L2 backside cache, r3 is set to 0).
 *
 *  Input  - None
 *  Output - Value of L2CR is set in r3.
 */


sysL2BackAutoSize:
	
	andi.   r0, r0, 0x0
        andi.   r5, r5, 0x0             /* Make r5 to be 0 */
	andi.	r30, r30, 0x0		/* Make r30 0 */

        addis   r4, r0, 0x0		/* Count index reg. */
        addis   r6, r0, WRITE_ADDR_U	/* Address to start writing */
        addis   r5, r0, L2_SIZE_1M_U	/* Memory chunk to write to */

l2SzWriteLoop:
	dcbz	r4, r6			/* zero out the cache line */
        stwx    r4, r4, r6		/* write index to a cache line */
	dcbf	r4, r6			/* flush cache to L2 */
        addi    r4, r4, L2_ADR_INCR	/* Increase the address index */
        cmp     0, 0, r4, r5
        blt     l2SzWriteLoop

        addis   r4, r0, 0x0		/* Count index  reg. - reset to 0 */

l2SzReadLoop:
	lwzx	r7, r4, r6		/* Load a word from the cache line */
	cmp	0, 0, r4, r7		/* Same as indexing value ? */
	bne	l2SkipCount    
	addi	r30, r30, 0x1		/* Count cache line read correctly */

l2SkipCount:
	dcbi    r4, r6			/* Invalidate the cache line */
	addi	r4, r4, L2_ADR_INCR	/* increase the address index */
	cmp	0, 0, r4, r5		/* Any memory space to read ? */
	blt	l2SzReadLoop

	addis	r7, r0, L2CR_1M_U	/* Load 1M l2cr value */
	cmpi	0, 0, r30, L2_SIZE_1M	/* 1M l2 backside cache ? */
	beq	l2AutoSizeDone
	addis	r7, r0, L2CR_HM_U	/* Load 512 KB l2cr value */
	cmpi	0, 0, r30, L2_SIZE_HM
	beq	l2AutoSizeDone
	addis	r7, r0, L2CR_QM_U	/* Load 256K l2cr reg. value */
	cmpi	0, 0, r30, L2_SIZE_QM
	beq	l2AutoSizeDone
	addis	r7, r0, 0x0		/* No L2 cache */

l2AutoSizeDone:
	addis	r30, r0, 0x0		/* Save the value of l2cr in r30 */
	addi	r30, r7, 0x0 
	addi	r3, r7, 0x0
	blr   


/**********************************************************************
 *  sysPVRRead - Read the content of the PVR register
 *	Once the PVR is read, the 16 least significant bits are shifted
 *	off.
 *  Input  - None
 *  Return - upper 16 bits of PVR stored in r3
 */

sysPVRReadBoot:

	mfspr r4, PVR_REG		/* read PVR  */
	srawi r3, r4, 16		/* shift off the least 16 bits */

	blr
  
/**********************************************************************
 *  sysPVRWrite - Write a value to the PVR register
 *  Input  - None
 *  Return - upper 16 bits of PVR stored in r3
 */


sysL2CRWrite:

	mtspr	L2CR_REG, r3
	blr
  
/**********************************************************************
 *  sysL1DcacheEnable - Enable the L1 Dcache
 *  Input  - None
 *  Return - None
 */

sysL1DcacheEnable:

        mfspr   r5,HID0_REG     		/* turn on  the D cache. */
        ori     r5,r5,L1_DCACHE_ENABLE    	/* Data cache only! */
        andi.   r6,r5,L1_DCACHE_INV_MASK    	/* clear the invalidate bit */
        mtspr   HID0_REG,r5
        isync
        sync
        mtspr   HID0_REG,r6
        isync
        sync
        blr 

/**********************************************************************
 *  sysL1DcacheDisable - Disable the L1 Dcache
 *  Input  - None
 *  Return - None
 */

sysL1DcacheDisable:

        mfspr   r5,HID0_REG
        andi.   r5,r5,L1_DCACHE_DISABLE
        mtspr   HID0_REG,r5
        isync
        sync
        blr          



