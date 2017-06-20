/*
 * $Id: drc_combo28.h,v 1.1.2.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * This file contains DPP DRC main structure and routine declarations for the Dram operation using PHY Combo28.
 *
 */
#ifndef _SOC_DPP_DRC_COMBO28_BIST_H
#define _SOC_DPP_DRC_COMBO28_BIST_H

#include <soc/shmoo_combo28.h>
#include <soc/dpp/ARAD/arad_api_dram.h>

/* Defines */

#define SOC_DPP_DRC_COMBO28_BIST_NOF_PATTERNS SOC_TMC_BIST_NOF_PATTERNS
#define SOC_DPP_DRC_COMBO28_BIST_NOF_SEEDS  SOC_TMC_BIST_NOF_SEEDS

/* BIST flags */
#define SOC_DPP_DRC_COMBO28_BIST_FLAGS_ADDRESS_SHIFT_MODE           SOC_TMC_DRAM_BIST_FLAGS_ADDRESS_SHIFT_MODE
#define SOC_DPP_DRC_COMBO28_BIST_FLAGS_INFINITE                     SOC_TMC_DRAM_BIST_FLAGS_INFINITE 
#define SOC_DPP_DRC_COMBO28_BIST_FLAGS_STOP                         SOC_TMC_DRAM_BIST_FLAGS_STOP
#define SOC_DPP_DRC_COMBO28_BIST_FLAGS_GET_DATA                     SOC_TMC_DRAM_BIST_FLAGS_GET_DATA
#define SOC_DPP_DRC_COMBO28_BIST_FLAGS_TWO_ADDRESS_MODE             SOC_TMC_DRAM_BIST_FLAGS_TWO_ADDRESS_MODE
#define SOC_DPP_DRC_COMBO28_BIST_FLAGS_BG_INTERLEAVE                SOC_TMC_DRAM_BIST_FLAGS_BG_INTERLEAVE 
#define SOC_DPP_DRC_COMBO28_BIST_FLAGS_SINGLE_BANK_TEST             SOC_TMC_DRAM_BIST_FLAGS_SINGLE_BANK_TEST
#define SOC_DPP_DRC_COMBO28_BIST_FLAGS_STAGGER_INCREMENT_MODE       SOC_TMC_DRAM_BIST_FLAGS_MPR_STAGGER_INCREMENT_MODE
#define SOC_DPP_DRC_COMBO28_BIST_FLAGS_MPR_READOUT_MODE             SOC_TMC_DRAM_BIST_FLAGS_MPR_READOUT_MODE
#define SOC_DPP_DRC_COMBO28_BIST_FLAGS_ADDRESS_PRBS_MODE            SOC_TMC_DRAM_BIST_FLAGS_ADDRESS_PRBS_MODE
#define SOC_DPP_DRC_COMBO28_BIST_FLAGS_USE_RANDOM_DATA_SEED         SOC_TMC_DRAM_BIST_FLAGS_USE_RANDOM_DATA_SEED
#define SOC_DPP_DRC_COMBO28_BIST_FLAGS_USE_RANDOM_DBI_SEED          SOC_TMC_DRAM_BIST_FLAGS_USE_RANDOM_DBI_SEED

#define SOC_DPP_DRC_COMBO28_BIST_FLAGS_USE_RANDOM_SEED ( SOC_DPP_DRC_COMBO28_BIST_FLAGS_USE_RANDOM_DATA_SEED | SOC_DPP_DRC_COMBO28_BIST_FLAGS_USE_RANDOM_DBI_SEED)

/* BIST Data patterns */
#define SOC_DPP_DRC_COMBO28_BIST_DATA_PATTERN_RANDOM_PRBS  SOC_TMC_DRAM_BIST_DATA_PATTERN_RANDOM_PRBS
#define SOC_DPP_DRC_COMBO28_BIST_DATA_PATTERN_BIT_MODE     SOC_TMC_DRAM_BIST_DATA_PATTERN_BIT_MODE   
#define SOC_DPP_DRC_COMBO28_BIST_DATA_PATTERN_SHIFT_MODE   SOC_TMC_DRAM_BIST_DATA_PATTERN_SHIFT_MODE 
#define SOC_DPP_DRC_COMBO28_BIST_DATA_PATTERN_ADDR_MODE    SOC_TMC_DRAM_BIST_DATA_PATTERN_ADDR_MODE  

/* Structure */ 
typedef enum
{
    SOC_DPP_DRC_COMBO28_BIST_MPR_MODE_SERIAL = 0,
    SOC_DPP_DRC_COMBO28_BIST_MPR_MODE_PARALLEL = 1,
    SOC_DPP_DRC_COMBO28_BIST_MPR_MODE_STAGARED = 2,
    SOC_DPP_DRC_COMBO28_BIST_MPR_DISABLE = 3
} SOC_DPP_DRC_COMBO28_BIST_MPR_MODE;

typedef SOC_TMC_DRAM_BIST_INFO  SOC_DPP_DRC_COMBO28_BIST_INFO;

typedef struct {
    /* the MPR mode */    
    SOC_DPP_DRC_COMBO28_BIST_MPR_MODE mpr_mode;

    /* the MPR location from which to read when MprReadoutMprLocation is asserted */    
    uint32 mpr_readout_mpr_location;

    /* patterns */
    uint8 mpr_pattern[SHMOO_COMBO28_BIST_MPR_NOF_PATTERNS];

} SOC_DPP_DRC_COMBO28_BIST_MPR_INFO;

typedef struct
{
    uint32 fifo_depth;
    /* Bist total number of commands */
    uint32 num_commands;
    /* Bist mode */
    SHMOO_COMBO28_GDDR5_BIST_MODE bist_mode;
    /* data pattern info */ 
    ARAD_DRAM_BIST_DATA_PATTERN_MODE data_pattern_mode;
    /* data patterns */
    uint64 data_pattern[SOC_DPP_DRC_COMBO28_BIST_NOF_PATTERNS];  
    /* data seed */
    uint32 prgs_data_seed[SOC_DPP_DRC_COMBO28_BIST_NOF_SEEDS]; 
    /* dbi pattern info */
    ARAD_DRAM_BIST_DATA_PATTERN_MODE dbi_pattern_mode;
    /* dbi patterns */
    uint8 dbi_pattern[SOC_DPP_DRC_COMBO28_BIST_NOF_PATTERNS];  
    /* dbi seed */
    uint32 prgs_dbi_seed;    
    /* edc pattern info */
    ARAD_DRAM_BIST_DATA_PATTERN_MODE edc_pattern_mode;
    /* edc patterns */
    uint8 edc_pattern[SOC_DPP_DRC_COMBO28_BIST_NOF_PATTERNS];  
    /* flags */
    uint32 bist_flags;
} SOC_DPP_DRC_COMBO28_GDDR5_BIST_INFO;

/*
 * DDR4 BIST configured
 */
int soc_dpp_drc_combo28_configure_bist(
    int unit,
    int drc_ndx,
    uint32 write_weight,
    uint32 read_weight,
    uint32 pattern_bit_mode,
    uint32 two_addr_mode,
    uint32 prbs_mode,
    uint32 address_shift_mode,
    uint32 data_shift_mode,
    uint32 data_addr_mode,
    uint32 bist_num_actions,
    uint32 bist_start_address,
    uint32 bist_end_address,
    uint32 bg_interleave,
    uint32 single_bank_test,
    uint32 address_prbs_mode,
    uint32 bist_pattern[SOC_DPP_DRC_COMBO28_BIST_NOF_PATTERNS],
    uint32 bist_data_seed[SOC_DPP_DRC_COMBO28_BIST_NOF_SEEDS]);

/*
 *  MPR Bist functtions
 */
int soc_dpp_drc_combo28_mpr_configure_bist(
    int unit,
    int dram_ndx,
    int mpr_stagger_mode,
    int mpr_stagger_increment_mode,
    int mpr_readout_mode,
    int mpr_readout_mpr_location,
    uint8 mpr_pattern[SHMOO_COMBO28_BIST_MPR_NOF_PATTERNS]);
  
int soc_dpp_drc_combo28_bist_test_start(
        int unit,
        int drc_ndx,
        SOC_DPP_DRC_COMBO28_BIST_INFO *info,
        SOC_DPP_DRC_COMBO28_BIST_MPR_INFO *mpr_info);

/*
 *  GDDR Bist functions
 */
int soc_dpp_drc_combo28_gddr5_bist_read_err_cnt(
    int unit,
    int dram_ndx,
    uint32 *bist_data_err_occur,
    uint32 *bist_dbi_err_occur,
    uint32 *bist_edc_err_occur,
    uint32 *bist_adt_err_occur);
  
int soc_dpp_drc_combo28_gddr5_bist_test_start(
    int unit,
    int dram_ndx,
    SOC_DPP_DRC_COMBO28_GDDR5_BIST_INFO *info);

#endif /* !_SOC_DPP_DRC_COMBO28_BIST_H  */

