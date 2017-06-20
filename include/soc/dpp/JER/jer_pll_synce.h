
/*
 * $Id: jer_pll_synce.h Exp $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#ifndef _SOC_DPP_JER_PLL_SYNCE_H
#define _SOC_DPP_JER_PLL_SYNCE_H

/* 
 * Defines
 */

/*
 * typedefs
 */

/* This enum used as an index to pll register array. changing it require changing the register array */
typedef enum
{
    /*
    * fabric
    */
    JER_PLL_TYPE_FABRIC_0 = 0,
    JER_PLL_TYPE_FABRIC_1 = 1,
    /*
    * nif
    */
    JER_PLL_TYPE_NIF_PMH = 2,
    JER_PLL_TYPE_NIF_PML_0 = 3,
    JER_PLL_TYPE_NIF_PML_1 = 4,
    /*
    * synce
    */
    JER_PLL_TYPE_SYNCE_0 = 5,
    JER_PLL_TYPE_SYNCE_1 = 6,
    /*
    * 1588
    */
    JER_PLL_TYPE_TS = 7,
    JER_PLL_TYPE_BS = 8,
    /*
    * number of jericho pll types
    */
    JER_NOF_PLL_TYPES = 9
}JER_PLL_TYPE;

/* 
 * Functions
 */
uint32 jer_synce_clk_div_set(int unit, uint32 synce_idx, ARAD_NIF_SYNCE_CLK_DIV   clk_div);
uint32 jer_synce_clk_div_get(int unit, uint32 synce_idx, ARAD_NIF_SYNCE_CLK_DIV*   clk_div);
uint32 jer_synce_clk_port_sel_set(int unit, uint32 synce_idx, soc_port_t port);
/* PLL_3 configurations used by both pll and sync init */
int jer_pll_3_set(int unit, JER_PLL_TYPE pll_type, uint32 ndiv, uint32 mdiv, uint32 pdiv, uint32 is_bypass);
int jer_plus_pll_set(int unit, JER_PLL_TYPE pll_type, uint32 ndiv, uint32 mdiv, uint32 pdiv, uint32 is_bypass);

#endif /* !_SOC_DPP_JER_PLL_SYNCE_H  */

