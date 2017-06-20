
/*
 * $Id: jer2_jer_pll_synce.h Exp $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#ifndef _SOC_DNX_JER2_JER_PLL_SYNCE_H
#define _SOC_DNX_JER2_JER_PLL_SYNCE_H

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
    JER2_JER_PLL_TYPE_FABRIC_0 = 0,
    JER2_JER_PLL_TYPE_FABRIC_1 = 1,
    /*
    * nif
    */
    JER2_JER_PLL_TYPE_NIF_PMH = 2,
    JER2_JER_PLL_TYPE_NIF_PML_0 = 3,
    JER2_JER_PLL_TYPE_NIF_PML_1 = 4,
    /*
    * synce
    */
    JER2_JER_PLL_TYPE_SYNCE_0 = 5,
    JER2_JER_PLL_TYPE_SYNCE_1 = 6,
    /*
    * 1588
    */
    JER2_JER_PLL_TYPE_TS = 7,
    JER2_JER_PLL_TYPE_BS = 8,
    /*
    * number of jer2_jericho pll types
    */
    JER2_JER_NOF_PLL_TYPES = 9
}JER2_JER_PLL_TYPE;


/* 
 *  SyncE
 */
typedef enum
{
   /*
   *  Synchronous Ethernet signal - differential (two signals
   *  per clock) recovered clock, two differential outputs
   */
  JER2_JER_PLL_NIF_SYNCE_MODE_TWO_DIFF_CLK = 0,
  /*
   *  Synchronous Ethernet signal - recovered clock accompanied
   *  by a valid indication, two clk+valid outputs
   */
  JER2_JER_PLL_NIF_SYNCE_MODE_TWO_CLK_AND_VALID = 1,
  /*
   *  Number of types in ARAD_NIF_SYNCE_MODE
   */
  JER2_JER_PLL_NIF_NOF_SYNCE_MODES = 2
}JER2_JER_PLL_NIF_SYNCE_MODE;


typedef enum
{
  /*
   *  Clock Divider for the selected recovered clock rate
   *  (based on SerDes lane rate) - divide by 1
   */
  JER2_JER_PLL_NIF_SYNCE_CLK_DIV_1 = 0,
  /*
   *  Clock Divider for the selected recovered clock rate
   *  (based on SerDes lane rate) - divide by 2
   */
  JER2_JER_PLL_NIF_SYNCE_CLK_DIV_2 = 1,
  /*
   *  Clock Divider for the selected recovered clock rate
   *  (based on SerDes lane rate) - divide by 80
   */
  JER2_JER_PLL_NIF_NIF_SYNCE_CLK_DIV_4 = 2,
  /*
   *  Number of types in SOC_PB_NIF_SYNCE_CLK_DIV
   */
  JER2_JER_PLL_NIF_NOF_SYNCE_CLK_DIVS = 3
}JER2_JER_PLL_NIF_SYNCE_CLK_DIV;

/* 
 * Functions
 */
uint32 jer2_jer_synce_clk_div_set(int unit, uint32 synce_idx, JER2_JER_PLL_NIF_SYNCE_CLK_DIV   clk_div);
uint32 jer2_jer_synce_clk_div_get(int unit, uint32 synce_idx, JER2_JER_PLL_NIF_SYNCE_CLK_DIV*   clk_div);
uint32 jer2_jer_synce_clk_port_sel_set(int unit, uint32 synce_idx, soc_port_t port);
/* PLL_3 configurations used by both pll and sync init */
int jer2_jer_pll_3_set(int unit, JER2_JER_PLL_TYPE pll_type, uint32 ndiv, uint32 mdiv, uint32 pdiv, uint32 is_bypass);
int jer2_jer_plus_pll_set(int unit, JER2_JER_PLL_TYPE pll_type, uint32 ndiv, uint32 mdiv, uint32 pdiv, uint32 is_bypass);

#endif /* !_SOC_DNX_JER2_JER_PLL_SYNCE_H  */

