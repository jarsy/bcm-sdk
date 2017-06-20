/*
 * $Id: qax_init.c Exp $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 */
#ifdef _ERR_MSG_MODULE_NAME 
    #error "_ERR_MSG_MODULE_NAME redefined" 
#endif
#define _ERR_MSG_MODULE_NAME BSL_SOC_DRAM

/* 
 * Includes
 */ 

#include <shared/bsl.h>

#include <soc/dcmn/error.h>

#include <soc/dpp/DRC/drc_combo28.h>
#include <soc/dpp/QAX/qax_dram.h>
/*
 * Functions
 */


/*
 * Function:
 *      soc_qax_dram_recovery_init
 * Purpose:
 *     initiates the dram recovery mechanism - dram gets too congested,
 *     writes to dram are delayed to give priority to reads from dram,
 *     this mechanism will start working once the configured admission
 *     tests will start to fail when dram is congested
 * Parameters:
 *      unit -  unit number
 * Returns:
 *      SOC_E_XXX
 */
int soc_qax_dram_recovery_init(int unit)
{
    uint64 reg64_val;
    soc_reg_above_64_val_t reg_above_64_val;

    SOCDNX_INIT_FUNC_DEFS;

    /* configure the MMU thresholds to detect congestion */
    SOCDNX_IF_ERR_EXIT(READ_MMU_GENERAL_CONFIGURATION_REGISTERr(unit, &reg64_val));
    soc_reg64_field32_set(unit, MMU_GENERAL_CONFIGURATION_REGISTERr, &reg64_val, DRAM_BLOCK_TOTAL_RAF_SIZE_THf, 0x190);
    soc_reg64_field32_set(unit, MMU_GENERAL_CONFIGURATION_REGISTERr, &reg64_val, DRAM_BLOCK_LEAKY_BUCKET_THf, 0x4);
    SOCDNX_IF_ERR_EXIT(WRITE_MMU_GENERAL_CONFIGURATION_REGISTERr(unit, reg64_val));

    /* Configure Admission tests */
    SOCDNX_IF_ERR_EXIT(READ_CGM_VOQ_DRAM_BLOCK_BMP_MASKr(unit, reg_above_64_val));
    /* set the following for each DP: 
        MASK            bit 0 = DP_LEVEL reject 
        MASK            bit 1 = lack of SRAM_PDBS 
        MASK            bit 2 = lack of SRAM_BUFFERS 
        DO NOT MASK     bit 3 = lack of DRAM_BDBS 
        DO NOT MASK     bit 4 = VOQ_DRAM_BLOCK 
        DO NOT MASK     bit 5 = VOQ_WORDS_MAX_QSIZE 
        MASK            bit 6 = VOQ_WRED 
        MASK            bit 7 = VOQ_SYS_RED 
        DO NOT MASK     bit 8 = VOQ_TOTAL_WORDS_SHARED_OCCUPANCY 
        MASK            bit 9 = VOQ_SRAM_WORDS_MAX_QSIZE 
        MASK            bit 10 = VOQ_TOTAL_SRAM_WORDS_SHARED_OCCUPANCY 
        MASK            bit 11 = VOQ_SRAM_PDS_MAX_QSIZE 
        MASK            bit 12 = VOQ_TOTAL_SRAM_PDS_SHARED_OCCUPANCY 
        MASK            bit 13 = VSQ_VSQA_WRED or VSQ_VSQB_WRED or VSQ_VSQC_WRED or VSQ_VSQD_WRED 
        MASK            bit 14 = VSQ_VSQE_WRED or VSQ_VSQF_WRED 
        MASK            bit 15 = VSQ_VSQA_WORDS_MAX_QSIZE or VSQ_VSQB_WORDS_MAX_QSIZE or VSQ_VSQC_WORDS_MAX_QSIZE or VSQ_VSQD_WORDS_MAX_QSIZE 
        MASK            bit 16 = VSQ_PB_VSQ_WORDS_MAX_QSIZE 
        MASK            bit 17 = VSQ_TOTAL_PB_VSQ_WORDS_SHARED_OCCUPANCY 
        MASK            bit 18 = VSQ_VSQA_SRAM_BUFFERS_MAX_QSIZE or VSQ_VSQB_SRAM_BUFFERS_MAX_QSIZE or VSQ_VSQC_SRAM_BUFFERS_MAX_QSIZE or VSQ_VSQD_SRAM_BUFFERS_MAX_QSIZE 
        MASK            bit 19 = VSQ_PB_VSQ_SRAM_BUFFERS_MAX_QSIZE 
        MASK            bit 20 = VSQ_TOTAL_PB_VSQ_SRAM_BUFFERS_SHARED_OCCUPANCY 
        MASK            bit 21 = VSQ_VSQA_SRAM_PDS_MAX_QSIZE or VSQ_VSQB_SRAM_PDS_MAX_QSIZE or VSQ_VSQC_SRAM_PDS_MAX_QSIZE or VSQ_VSQD_SRAM_PDS_MAX_QSIZE 
        MASK            bit 22 = VSQ_PB_VSQ_SRAM_PDS_MAX_QSIZE 
        MASK            bit 23 = VSQ_TOTAL_PB_VSQ_SRAM_PDS_SHARED_OCCUPANCY 
    */
    soc_reg_above_64_field32_set(unit, CGM_VOQ_DRAM_BLOCK_BMP_MASKr, reg_above_64_val, VOQ_DRAM_BLOCK_BMP_MASK_DP_0f, 0x138);
    soc_reg_above_64_field32_set(unit, CGM_VOQ_DRAM_BLOCK_BMP_MASKr, reg_above_64_val, VOQ_DRAM_BLOCK_BMP_MASK_DP_1f, 0x138);
    soc_reg_above_64_field32_set(unit, CGM_VOQ_DRAM_BLOCK_BMP_MASKr, reg_above_64_val, VOQ_DRAM_BLOCK_BMP_MASK_DP_2f, 0x138);
    soc_reg_above_64_field32_set(unit, CGM_VOQ_DRAM_BLOCK_BMP_MASKr, reg_above_64_val, VOQ_DRAM_BLOCK_BMP_MASK_DP_3f, 0x138);
    SOCDNX_IF_ERR_EXIT(WRITE_CGM_VOQ_DRAM_BLOCK_BMP_MASKr(unit, reg_above_64_val));

exit:
    SOCDNX_FUNC_RETURN;
}

