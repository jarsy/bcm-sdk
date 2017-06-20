/**********************************************************************************
 **********************************************************************************
 *  File Name     :  merlin16_pll_config.c                                        *
 *  Created On    :  14/07/2013                                                   *
 *  Created By    :  Kiran Divakar                                                *
 *  Description   :  Merlin16 PLL Configuration API                               *
 *  Revision      :    *
 *                                                                                *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$                                                          *
 *  No portions of this material may be reproduced in any form without            *
 *  the written permission of:                                                    *
 *      Broadcom Corporation                                                      *
 *      5300 California Avenue                                                    *
 *      Irvine, CA  92617                                                         *
 *                                                                                *
 *  All information contained in this document is Broadcom Corporation            *
 *  company private proprietary, and trade secret.                                *
 *                                                                                *
 **********************************************************************************
 **********************************************************************************/

/** @file merlin16_pll_config.c
 * Merlin16 PLL Configuration
 */


#include "merlin16_config.h"
#include "merlin16_functions.h"
#include "merlin16_internal.h"
#include "merlin16_internal_error.h"
#include "merlin16_select_defns.h"

#define _ndiv_frac_l(x) ((x)&0xF)
#define _ndiv_frac_h(x) ((x)>>4)

#define _ndiv_frac_decode(l_, h_) (((l_) & 0xF) | (((h_) & 0x3FFF) << 4))

static const uint8_t pll_fraction_width = 18;

err_code_t merlin16_INTERNAL_configure_pll(srds_access_t *sa__,
                                         enum merlin16_pll_refclk_enum refclk,
                                         enum merlin16_pll_div_enum div,
                                         uint32_t vco_freq_khz,
                                         uint8_t refclk_doubler_en) {
    uint32_t refclk_freq_hz;
    uint8_t reset_state;

    EFUN(merlin16_INTERNAL_resolve_pll_parameters(refclk, &refclk_freq_hz, &div, &vco_freq_khz, 0));

    /* Use this to restore defaults if reprogramming the PLL under dp-reset (typically Auto-Neg FW) */
    /* EFUN(wrc_ams_pll_i_ndiv_int(0x42));                   */
    /* EFUN(wrc_ams_pll_i_ndiv_frac_h(_ndiv_frac_h(0x0)));   */
    /* EFUN(wrc_ams_pll_i_ndiv_frac_l(_ndiv_frac_l(0x0)));   */
    /* EFUN(wrc_ams_pll_i_pll_frac_mode(0x2));               */

    /* Use core_s_rstb to re-initialize all registers to default before calling this function. */
    ESTM(reset_state = rdc_core_dp_reset_state());

    if(reset_state < 7) {
        EFUN_PRINTF(("ERROR: merlin16_configure_pll(..) called without core_dp_s_rstb=0\n"));
        return (_error(ERR_CODE_CORE_DP_NOT_RESET));
    }
 
    EFUN(wrc_ams_pll_i_ndiv_int(SRDS_INTERNAL_GET_PLL_DIV_INTEGER(div)));
    {
        const uint32_t pll_fraction_num = SRDS_INTERNAL_GET_PLL_DIV_FRACTION_NUM(div, pll_fraction_width);
        EFUN(wrc_ams_pll_i_ndiv_frac_h(_ndiv_frac_h(pll_fraction_num)));
        EFUN(wrc_ams_pll_i_ndiv_frac_l(_ndiv_frac_l(pll_fraction_num)));
    }
    EFUN(wrc_ams_pll_i_pll_frac_mode(0x2));


    /* Toggling PLL mmd reset */
    EFUN(wrc_ams_pll_mmd_resetb(0x0));
    EFUN(wrc_ams_pll_mmd_resetb(0x1));

    /* NOTE: Might have to add some optimized PLL control bus settings post-DVT (See 28nm merlin_pll_config.c) */

    /* Update core variables with the VCO rate. */
    {
        struct merlin16_uc_core_config_st core_config;
        EFUN(merlin16_get_uc_core_config(sa__, &core_config));
        core_config.vco_rate_in_Mhz = (vco_freq_khz + 500) / 1000;
        core_config.field.vco_rate = MHZ_TO_VCO_RATE(core_config.vco_rate_in_Mhz);
        EFUN(merlin16_INTERNAL_set_uc_core_config(sa__, core_config));
    }

    return (ERR_CODE_NONE);

} /* merlin16_configure_pll */

err_code_t merlin16_INTERNAL_read_pll_div(srds_access_t *sa__, uint32_t *div) {
    uint16_t ndiv_int;
    uint32_t ndiv_frac;
    ESTM(ndiv_int = rdc_ams_pll_i_ndiv_int());
    ESTM(ndiv_frac = _ndiv_frac_decode(rdc_ams_pll_i_ndiv_frac_l(), rdc_ams_pll_i_ndiv_frac_h()));
    *div = SRDS_INTERNAL_COMPOSE_PLL_DIV(ndiv_int, ndiv_frac, pll_fraction_width);
    return (ERR_CODE_NONE);
}

