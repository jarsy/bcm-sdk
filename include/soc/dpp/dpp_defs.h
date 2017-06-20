/*
* $Id$
* $Copyright: (c) 2016 Broadcom.
* Broadcom Proprietary and Confidential. All rights reserved.$
*
* 
*/
#ifndef SOC_DPP_DPP_DEFS_H
#define SOC_DPP_DPP_DEFS_H


typedef enum soc_dpp_stat_path_drop_stage_e {
    soc_dpp_stat_path_drop_stage_none = 0,
    soc_dpp_stat_path_drop_stage_ingress_no_packet = 1,
    soc_dpp_stat_path_drop_stage_ingress_tm = 2,
    soc_dpp_stat_path_drop_stage_egress_tm = 3,

    /*must be last*/
    soc_dpp_stat_path_drop_stage_nof = 4
} soc_dpp_stat_path_drop_stage_t;


typedef struct soc_dpp_stat_path_info_s {
    int ingress_core;
    int egress_core;
    soc_dpp_stat_path_drop_stage_t drop;
} soc_dpp_stat_path_info_t;

/* PLL configuration structure */
typedef struct soc_dpp_pll_s {
    int p_div; /* Pre divider */
    int n_div; /* Feedback divider */
    int m0_div; /* M0 divider determines the frequency on Channel 0 */
    int m1_div; /* M1 divider determines the frequency on Channel 1 */
    int m4_div; /* M4 divider determines the frequency on Channel 4 */
    int m5_div; /* M5 divider determines the frequency on Channel 5 */
    int locked; /* PLL status */
#ifdef COMPILER_HAS_DOUBLE
    double vco; /* Voltage Control Oscilator */
    double ch0; /* Channel 0 */
    double ch1; /* Channel 1 */
    double ch4; /* Channel 4 */
    double ch5; /* Channel 5 */
    double ref_clk; /* Reference clock */
#else
    int vco; /* Voltage Control Oscilator */
    int ch0; /* Channel 0 */
    int ch1; /* Channel 1 */
    int ch4; /* Channel 4 */
    int ch5; /* Channel 5 */
    int ref_clk; /* Reference clock */
#endif
} soc_dpp_pll_t;

/* PLLs info structure */
typedef struct soc_dpp_pll_info_s {
    soc_dpp_pll_t core_pll;
    soc_dpp_pll_t uc_pll;
    soc_dpp_pll_t ts_pll;
    soc_dpp_pll_t bs_pll[2];
    soc_dpp_pll_t pmh_pll;
    soc_dpp_pll_t pml_pll[2];
    soc_dpp_pll_t fab_pll[2];
    soc_dpp_pll_t srd_pll[2];
    soc_dpp_pll_t ddr_pll[4];
} soc_dpp_pll_info_t;


#endif /* SOC_DPP_DPP_DEFS_H */

