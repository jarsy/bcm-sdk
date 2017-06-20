/*
 * $Id: jer_init.c Exp $
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

/*
 * Functions
 */

/* Dram configuration parameters Verify */
int soc_jer_dram_info_verify(int unit, soc_dpp_drc_combo28_info_t *drc_info) 
{
    int i;
    int dram_num_max=0;

    SOCDNX_INIT_FUNC_DEFS;

    SHR_BIT_ITER(drc_info->dram_bitmap, SOC_DPP_DEFS_MAX(HW_DRAM_INTERFACES_MAX), i){
        dram_num_max = i;
    }
    if (dram_num_max > SOC_DPP_DEFS_GET(unit, hw_dram_interfaces_max)) {
        SOCDNX_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_SOC_MSG("Error in %s: max port in dram bitamp=%d is not supported."), FUNCTION_NAME(), dram_num_max));
    }

    if (drc_info->dram_num > SOC_DPP_DEFS_GET(unit, hw_dram_interfaces_max) ) {
        SOCDNX_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_SOC_MSG("Error in %s: Number of dram=%d is not supported."),FUNCTION_NAME(), drc_info->dram_num));
    }

    if (SOC_IS_QAX(unit)) {
        switch (drc_info->dram_num) {
        case 0:
        case 1:
        case 2:
        case 3:
            break;
        default:
            SOCDNX_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_SOC_MSG("Error in %s: Number of drams=%d is not supported."), FUNCTION_NAME(), drc_info->dram_num));
        }
    } else {
        switch (drc_info->dram_num) {
        case 0:
        case 2:
        case 3:
        case 4:
        case 6:
        case 8:
            break;
        default:
            SOCDNX_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_SOC_MSG("Error in %s: Number of drams=%d is not supported."), FUNCTION_NAME(), drc_info->dram_num));
        }
    }

    switch (drc_info->nof_columns) {
    case 256:
    case 512:
    case 1024:
        break;
    default:
        SOCDNX_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_SOC_MSG("Error in %s: Number of columns=%d is not supported."), FUNCTION_NAME(), drc_info->nof_columns));
    }

    switch (drc_info->dram_type) {   
    case SHMOO_COMBO28_DRAM_TYPE_DDR4:
    case SHMOO_COMBO28_DRAM_TYPE_GDDR5:
        break;
    default:
        SOCDNX_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_SOC_MSG("Error in %s: Dram type=%d is not supported."), FUNCTION_NAME(), drc_info->dram_type));
    }

    if (drc_info->dram_freq < 800 || drc_info->dram_freq > 2000) {   
        SOCDNX_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_SOC_MSG("Error in %s: Dram freq=%d is not supported."), FUNCTION_NAME(), drc_info->dram_freq));
    }
    
    switch (drc_info->nof_banks) {   
    case 8:
    case 16:
        break;
    default:
        SOCDNX_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_SOC_MSG("Error in %s: Number of bankse=%d is not supported."), FUNCTION_NAME(), drc_info->nof_banks));
    }

    if (drc_info->dram_param.init_wait_period > 4000) {
        SOCDNX_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_SOC_MSG("Error in %s: Dram init wait period=%d is not supported."), FUNCTION_NAME(), drc_info->dram_param.init_wait_period));
    }

    switch (drc_info->ref_clk_mhz) {
    case 100:
        break;
    default:
        SOCDNX_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_SOC_MSG("Error in %s: ddr ref clk=%d(MHZ) is not supported."), FUNCTION_NAME(), drc_info->ref_clk_mhz));
    }
    
    switch (drc_info->auto_tune) {
    case 2:
    case 1:
    case 0:
    case 3:
        break;
    default:
        SOCDNX_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_SOC_MSG("Error in %s: Dram auto tune=%d is not supported."), FUNCTION_NAME(), drc_info->auto_tune));
    }
    
    switch (drc_info->bist_enable) {
    case 1:
    case 0:
        break;
    default:
        SOCDNX_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_SOC_MSG("Error in %s: Bist enable=%d is not supported."), FUNCTION_NAME(), drc_info->bist_enable));
    }
    
    switch (drc_info->gear_down_mode) {
    case 1:
    case 0:
        break;
    default:
        SOCDNX_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_SOC_MSG("Error in %s: Gear dowm mode=%d is not supported."), FUNCTION_NAME(), drc_info->gear_down_mode));
    }

    switch (drc_info->abi) {
    case 1:
    case 0:
        break;
    default:
        SOCDNX_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_SOC_MSG("Error in %s: Abi=%d is not supported."), FUNCTION_NAME(), drc_info->abi));
    }
   
    switch (drc_info->write_dbi) {
    case 1:
    case 0:
        break;
    default:
        SOCDNX_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_SOC_MSG("Error in %s: Write dbi=%d is not supported."), FUNCTION_NAME(), drc_info->write_dbi));
    }

    switch (drc_info->read_dbi) {
    case 1:
    case 0:
        break;
    default:
        SOCDNX_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_SOC_MSG("Error in %s: Read dbi=%d is not supported."), FUNCTION_NAME(), drc_info->read_dbi));
    }

    if (drc_info->cmd_par_latency > 8) {
        SOCDNX_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_SOC_MSG("Error in %s: Cmd par latency=%d is not supported."), FUNCTION_NAME(), drc_info->cmd_par_latency));
    }

exit:
    SOCDNX_FUNC_RETURN;
}


/*********************************************************************
*  Drc Init
*********************************************************************/
int soc_jer_dram_init_drc_soft_init(int unit, soc_dpp_drc_combo28_info_t *drc_info, uint32  init) {

    soc_reg_above_64_val_t
        soft_init_reg_val,
        fld_above64_val;
    int drc_ndx;
    const static 
        soc_field_t drc_init_field[] = {BLOCKS_SOFT_INIT_11f, BLOCKS_SOFT_INIT_12f, BLOCKS_SOFT_INIT_13f, BLOCKS_SOFT_INIT_14f, BLOCKS_SOFT_INIT_15f, BLOCKS_SOFT_INIT_16f, BLOCKS_SOFT_INIT_17f, BLOCKS_SOFT_INIT_18f};

    SOCDNX_INIT_FUNC_DEFS;

    if (init) {
        SOC_REG_ABOVE_64_CREATE_MASK(fld_above64_val, 1, 0);
    } else {
        SOC_REG_ABOVE_64_CLEAR(fld_above64_val);
    }

    SOCDNX_IF_ERR_EXIT(READ_ECI_BLOCKS_SOFT_INITr(unit,soft_init_reg_val));
    SHR_BIT_ITER(drc_info->dram_bitmap, SOC_DPP_DEFS_GET(unit, hw_dram_interfaces_max), drc_ndx) {
        soc_reg_above_64_field_set(unit, ECI_BLOCKS_SOFT_INITr, soft_init_reg_val, drc_init_field[drc_ndx], fld_above64_val);
    }
    SOCDNX_IF_ERR_EXIT(WRITE_ECI_BLOCKS_SOFT_INITr(unit,soft_init_reg_val));

    LOG_DEBUG(BSL_LS_SOC_DRAM, (BSL_META_U(unit, "%s(): Perform drc soft init=%d.\n"),FUNCTION_NAME(), init));

exit:
    SOCDNX_FUNC_RETURN;
}



/*
 * Function:
 *      soc_jer_dram_recovery_init
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

int soc_jer_dram_recovery_init(int unit)
{
    uint64 reg64_val;
    soc_reg_above_64_val_t reg_above_64_val;

    SOCDNX_INIT_FUNC_DEFS;

    /* configure the MMU thresholds to detect congestion */
    SOCDNX_IF_ERR_EXIT(READ_MMU_PDC_CONFIGURATION_REGISTERr(unit, &reg64_val));
    soc_reg64_field32_set(unit, MMU_PDC_CONFIGURATION_REGISTERr, &reg64_val, DRAM_BLOCK_TOTAL_RAF_SIZE_THf, 0x190);
    soc_reg64_field32_set(unit, MMU_PDC_CONFIGURATION_REGISTERr, &reg64_val, DRAM_BLOCK_LEAKY_BUCKET_THf, 0x4);
    SOCDNX_IF_ERR_EXIT(WRITE_MMU_PDC_CONFIGURATION_REGISTERr(unit, reg64_val));

    /* Configure Admission tests */
    SOCDNX_IF_ERR_EXIT(READ_IQM_DRAM_ELIGIBLE_RJCT_MASKr(unit, SOC_CORE_ALL, reg_above_64_val));
    /* set the following for each DP: 
    Yes       bit 0 - Mask reject test based ib Cfg-DP-Level-Rjct-Th
    Yes       bit 1 - Mask BDB protect test
    No        bit 2 - Mask Q-Num-Overflow reject test
    No        bit 3 - Mask ISP-Duplicate reject test
    No        bit 4 - Mask Resource-Error reject test
    No        bit 5 - Mask Q-Not-Valid reject test
    Yes       bit 6 - Mask Dram-Reject test
    No        bit 7 - Mask Q-Num-Overflow reject test
    No        bit 8 - Mask Buffer-Type reject test
    No        bit 9 - Mask DP level reject test
    Yes       bit 10 - Mask reject test based on free dynamic space
    Yes       bit 11 - Mask reject test based on occupied BDs
    Yes       bit 12 - Mask reject test based on free BDBs
    Yes       bit 13 - Mask reject test based on free DBs
    Yes       bit 14 - Mask reject test based on queue Max size - Words/Buffers/BDs
    No        bit 15 - Mask reject test based on queue WRED
    No        bit 16 - Mask reject test based on Interlock
    No        bit 17 - Mask reject test based on VOQ Fair-Adaptive dynamic threshold
    No        bit 18 - Mask reject test based on System RED
    No        bit 19 - Mask reject test based on VSQ Free Shared Resource
    No        bit 20 - Mask reject test based on Port/PG VSQ reject
    Yes       bit 21 - Mask reject test based on VSQ max size reject
    No        bit 22 - Mask reject test based on VSQ WRED reject
    No        bit 23 - Mask reject test based on MC Fair-Adaptive dynamic threshold
    */
    soc_reg_above_64_field32_set(unit, IQM_DRAM_ELIGIBLE_RJCT_MASKr, reg_above_64_val, DRAM_ELIGIBLE_RJCT_MASK_DP_0f, 0x207C43);
    soc_reg_above_64_field32_set(unit, IQM_DRAM_ELIGIBLE_RJCT_MASKr, reg_above_64_val, DRAM_ELIGIBLE_RJCT_MASK_DP_1f, 0x207C43);
    soc_reg_above_64_field32_set(unit, IQM_DRAM_ELIGIBLE_RJCT_MASKr, reg_above_64_val, DRAM_ELIGIBLE_RJCT_MASK_DP_2f, 0x207C43);
    soc_reg_above_64_field32_set(unit, IQM_DRAM_ELIGIBLE_RJCT_MASKr, reg_above_64_val, DRAM_ELIGIBLE_RJCT_MASK_DP_3f, 0x207C43);
    SOCDNX_IF_ERR_EXIT(WRITE_IQM_DRAM_ELIGIBLE_RJCT_MASKr(unit, SOC_CORE_ALL, reg_above_64_val));

exit:
    SOCDNX_FUNC_RETURN;
}
