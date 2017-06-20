/*
 * $Id: jer2_jer_regs.h,v 1.0 Broadcom SDK $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 */
#ifndef _JER2_JER_REGS_H_
#define _JER2_JER_REGS_H_


int soc_jer2_jer_regs_eci_access_check(int unit);

/*
 * Function:
 *      soc_jer2_jer_regs_blocks_access_check_regs
 * Purpose:
 *      performs a perliminary sanity check for access to few registers of different sizes.
 * Parameters:
 *      unit            - Device Number
 * Returns:
 *      SOC_E_XXX
 */
int soc_jer2_jer_regs_blocks_access_check_regs(int unit);

/*
 * Function:
 *      soc_jer2_jer_regs_blocks_access_check_mems
 * Purpose:
 *      performs a perliminary sanity check for access a memory.
 *      SHOULD NOT BE USED ON WIDE MEMORIES!
 * Parameters:
 *      unit            - Device Number
 *      test_mem        - Tested memory
 *      block_num       - Block number, should be filled using <block name>_BLOCK(unit, instance) MACRO
 * Returns:
 *      SOC_E_XXX
 */
int soc_jer2_jer_regs_blocks_access_check_mem(int unit, soc_mem_t test_mem, int block_num);

/*
 * Function:
 *      soc_jer2_jer_regs_blocks_access_check_dma
 * Purpose:
 *      performs a perliminary sanity check for access via dma.
 * Parameters:
 *      unit            - Device Number
 * Returns:
 *      SOC_E_XXX
 */
int soc_jer2_jer_regs_blocks_access_check_dma(int unit);

/*
 * Function:
 *      soc_jer2_jer_regs_blocks_access_check
 * Purpose:
 *      performs a perliminary sanity check for few registers and memories of differnet size, and checks dma access
 * Parameters:
 *      unit            - Device Number
 * Returns:
 *      SOC_E_XXX
 */
int soc_jer2_jer_regs_blocks_access_check(int unit);

int soc_jer2_jer_brdc_fsrd_blk_id_set(int unit);

#endif /* _JER2_JER_REGS_H_ */
