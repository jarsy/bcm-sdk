/**********************************************************************************
 **********************************************************************************
 *  File Name     :  falcon16_pll_config.c                                        *
 *  Created On    :  23/12/2013                                                   *
 *  Created By    :  Kiran Divakar                                                *
 *  Description   :  Falcon16 PLL Configuration API                               *
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


/** @file falcon16_pll_config.c
 * Falcon16 PLL Configuration
 */


#include "../include/falcon16_tsc_config.h"
#include "../include/falcon16_tsc_functions.h"
#include "../include/falcon16_tsc_internal.h"
#include "../include/falcon16_tsc_internal_error.h"
#include "../include/falcon16_tsc_select_defns.h"

#define VCO_FREQ_KHZ_NEAR(vco_freq_khz_, near_val_) (   ((vco_freq_khz_) >= (near_val_) - 10000) \
                                                     || ((vco_freq_khz_) <= (near_val_) + 10000))

/* The pll_fracn_ndiv_int and pll_fracn_frac bitfields have this many bits. */
static const uint32_t pll_fracn_ndiv_int_bits = 10;
static const uint32_t pll_fracn_frac_bits     = 18;

err_code_t falcon16_tsc_INTERNAL_configure_pll(srds_access_t *sa__,
                                         enum falcon16_tsc_pll_refclk_enum refclk,
                                         enum falcon16_tsc_pll_div_enum div,
                                         uint32_t vco_freq_khz,
                                         uint8_t refclk_doubler_en) {
    uint32_t refclk_freq_hz;
    uint8_t reset_state;
    uint8_t int_configured;

    EFUN(falcon16_tsc_INTERNAL_resolve_pll_parameters(refclk, &refclk_freq_hz, &div, &vco_freq_khz, refclk_doubler_en));

    /* Use this to restore defaults if reprogramming the PLL under dp-reset (typically Auto-Neg FW) */
    /* EFUN(wrc_pll_mode(0x7));                */
    /* EFUN(wrc_ams_pll_vco2_15g(0x0));        */
    /* EFUN(wrc_ams_pll_fracn_ndiv_int(0x0));  */
    /* EFUN(wrc_ams_pll_fracn_div(0x0000));    */
    /* EFUN(wrc_ams_pll_fracn_div_17_16(0x0)); */
    /* EFUN(wrc_ams_pll_fracn_bypass(0x0));    */
    /* EFUN(wrc_ams_pll_fracn_divrange(0x0));  */
    /* EFUN(wrc_ams_pll_ditheren(0x0));        */
    /* EFUN(wrc_ams_pll_fracn_sel(0x0));       */
    /* EFUN(wrc_ams_pll_kvh_force(0x0));       */
    /* EFUN(wrc_ams_pll_force_kvh_bw(0x0));    */

    /* Use core_s_rstb to re-initialize all registers to default before calling this function. */
    ESTM(reset_state = rdc_core_dp_reset_state());

    if(reset_state < 7) {
        EFUN_PRINTF(("ERROR: falcon16_tsc_configure_pll(..) called without core_dp_s_rstb=0\n"));
        return (_error(ERR_CODE_CORE_DP_NOT_RESET));
    }

    /* Per AB on 13 Nov 2015:  Use VCO2 for 22.6 GHz and below.
     * The next supported frequency is 23.0 GHz, so let's use 22.8 GHz as the threshold.
     */
    EFUN(wrc_ams_pll_vco2_15g((vco_freq_khz < 22800000) ? 1 : 0));
    
    /* Enable refclk doubler */
    EFUN(wrc_ams_pll_refclk_doubler((refclk_doubler_en > 0) ? 0x1 : 0));
    EFUN(wrc_ams_pll_doubler_res   ((refclk_doubler_en > 0) ? 0x6 : 0));
    EFUN(wrc_ams_pll_doubler_cap   ((refclk_doubler_en > 0) ? 0x3 : 0));

    int_configured = 0;
    if (!SRDS_INTERNAL_IS_PLL_DIV_FRACTIONAL(div)) {
        uint8_t pll_mode;
        int_configured = 1;
        switch (SRDS_INTERNAL_GET_PLL_DIV_INTEGER(div)) {
            case  64: pll_mode =  0; break;
            case  66: pll_mode =  1; break;
            case  80: pll_mode =  2; break;
            case 128: pll_mode =  3; break;
            case 132: pll_mode =  4; break;
            case 140: pll_mode =  5; break;
            case 160: pll_mode =  6; break;
            case 165: pll_mode =  7; break;
            case 168: pll_mode =  8; break;
            case 170: pll_mode =  9; break;
            case 175: pll_mode = 10; break;
            case 180: pll_mode = 11; break;
            case 184: pll_mode = 12; break;
            case 200: pll_mode = 13; break;
            case 224: pll_mode = 14; break;
            case 264: pll_mode = 15; break;
            case  96: pll_mode = 16; break;
            case 120: pll_mode = 17; break;
            case 144: pll_mode = 18; break;
            case 198: pll_mode = 19; break;
            default: int_configured = 0;
        }
    
        if (int_configured) {
            EFUN(wrc_pll_mode(pll_mode));
        }
    }
    
    EFUN(wrc_ams_pll_fracn_sel(int_configured ? 0 : 1));

    if (!int_configured) {
        /* Either fractional mode was requested, or integer mode doesn't support the divisor.
         * Get information needed for fractional mode configuration.
         *
         * The value programmed into the pll_fracn_* bitfields which must account for the
         * initial div2 stage after the VCO.  For instance, a divide by 147.2 must be
         * programmed with an integer of 73 and a fraction of 0.6.
         *
         * Start with the div value, divided by 2, composed of an integer and a wide fractional value.
         */
        const uint8_t  div_fraction_width = 28; /* Must be less than 32 due to overflow detection below. */
        const uint16_t div_integer        = SRDS_INTERNAL_GET_PLL_DIV_INTEGER(div) >> 1;
        const uint32_t div_fraction       = (((SRDS_INTERNAL_GET_PLL_DIV_INTEGER(div) & 1) << (div_fraction_width-1))
                                             | SRDS_INTERNAL_GET_PLL_DIV_FRACTION_NUM(div, div_fraction_width-1));

        /* The div_fraction may have more precision than our pll_fracn_frac bitfield.
         * So round it.  Start by adding 1/2 LSB of the fraction div_fraction.
         */
        const uint32_t div_fraction_0p5 = 1 << (div_fraction_width - pll_fracn_frac_bits - 1);
        const uint32_t div_fraction_plus_0p5 = div_fraction + div_fraction_0p5;

        /* Did div_fraction_plus_p5 have a carry bit? */
        const uint32_t div_fraction_plus_p5_carry = div_fraction_plus_0p5 >> div_fraction_width;

        /* The final rounded div_fraction, including carry up to div_integer.
         * This removes the carry and implements the fixed point truncation.
         */
        const uint16_t pll_fracn_ndiv_int  = div_integer + div_fraction_plus_p5_carry;
        const uint32_t pll_fracn_div = ((div_fraction_plus_0p5 & ((1 << div_fraction_width)-1))
                                        >> (div_fraction_width - pll_fracn_frac_bits));

        if (pll_fracn_ndiv_int != (pll_fracn_ndiv_int & ((1 << pll_fracn_ndiv_int_bits)-1))) {
            EFUN_PRINTF(("ERROR:  PLL divide is too large for div value 0x%08X\n", div));
            return (_error(ERR_CODE_PLL_DIV_INVALID));
        }

        EFUN(wrc_pll_mode(0));
        if ((pll_fracn_ndiv_int < 12) || (pll_fracn_ndiv_int > 251)) {
            return (_error(ERR_CODE_INVALID_PLL_CFG));
        }
        EFUN(wrc_ams_pll_fracn_ndiv_int (pll_fracn_ndiv_int));
        EFUN(wrc_ams_pll_fracn_div      (pll_fracn_div & 0xFFFF));
        EFUN(wrc_ams_pll_fracn_div_17_16(pll_fracn_div >> 16));
        EFUN(wrc_ams_pll_fracn_bypass   (0));
        EFUN(wrc_ams_pll_fracn_divrange ((pll_fracn_ndiv_int < 60) ? 0 : 1));
        EFUN(wrc_ams_pll_ditheren       ((pll_fracn_div != 0) ? 1 : 0));
#if defined(wrc_ndiv_frac_valid)
        /* toggle ndiv_frac_valid high, then low to load in a new value for fracn_div. */
        EFUN(wrc_ndiv_frac_valid(1));
        EFUN(wrc_ndiv_frac_valid(0));
#endif
    }

    /* Determine when and how to override KVH. */
    {
        const uint8_t kvh_no_force = 255;
        uint8_t kvh_force = kvh_no_force;
        if (VCO_FREQ_KHZ_NEAR(vco_freq_khz, 23000000) && (div == FALCON16_TSC_PLL_DIV_147P2)) {
            /* AMS v0.99:  For 23 GHz with divide by 147.2, force KVH to 1 */
            kvh_force = 1;
        } else if (VCO_FREQ_KHZ_NEAR(vco_freq_khz, 24000000) && (div == FALCON16_TSC_PLL_DIV_240)) {
            /* AMS v0.99:  For 24 GHz with divide by 240, force KVH to 0 */
            kvh_force = 0;
        } else if (VCO_FREQ_KHZ_NEAR(vco_freq_khz, 24750000) && (div == FALCON16_TSC_PLL_DIV_158P4)) {
            /* AMS v0.99:  For 24.75 GHz with divide by 158.4, force KVH to 0 */
            kvh_force = 0;
        } else if (VCO_FREQ_KHZ_NEAR(vco_freq_khz, 28000000) && (div == FALCON16_TSC_PLL_DIV_280)) {
            /* AMS v0.99:  For 28 GHz with divide by 280, force KVH to 0 */
            kvh_force = 0;
        } else if (VCO_FREQ_KHZ_NEAR(vco_freq_khz, 27343750) && (div == FALCON16_TSC_PLL_DIV_175)) {
            /* AB on 21 Oct 2015:  KVH is forced to 0 for 27.34375 GHz. */
            kvh_force = 0;
        } else if ((refclk_doubler_en > 0) && (vco_freq_khz > 24000000)) {
            kvh_force = 0;
        } else if ((refclk_doubler_en > 0) && (vco_freq_khz > 22800000) && (vco_freq_khz <= 24000000)) {
            kvh_force = 1;
        } else if ((vco_freq_khz >= 20000000) && (vco_freq_khz <= 22800000)) {
            /* AB on 27 Oct 2015:  KVH is forced to 0 for VCO2 above 20.0 GHz and above. */
            kvh_force = 0;
        } else if (vco_freq_khz < 20000000) {
            /* AB on 13 Nov 2015:  KVH is forced to ??? (let's pick 3 for now) for VCO2 below 20.0 GHz. */
            kvh_force = 3;
        } else if (!int_configured) {
            /* For all fractional modes, KVH is forced to 0. */
            kvh_force = 0;
        }
        /* Per AB on 27 Oct 2015, vco_indicator and pll2rx_clkbw are always forced to 0. */
#if defined(wrc_ams_pll_vco_indicator)
        EFUN(wrc_ams_pll_vco_indicator(0));
#endif
        EFUN(wrc_ams_pll_pll2rx_clkbw(0));
        EFUN(wrc_ams_pll_kvh_force((kvh_force == kvh_no_force) ? 0 : kvh_force));
        EFUN(wrc_ams_pll_force_kvh_bw((kvh_force == kvh_no_force) ? 0 : 1));
    }

    /* Update core variables with the VCO rate. */
    {
        struct falcon16_tsc_uc_core_config_st core_config;
        EFUN(falcon16_tsc_get_uc_core_config(sa__, &core_config));
        core_config.vco_rate_in_Mhz = (vco_freq_khz + 500) / 1000;
        core_config.field.vco_rate = MHZ_TO_VCO_RATE(core_config.vco_rate_in_Mhz);
        EFUN(falcon16_tsc_INTERNAL_set_uc_core_config(sa__, core_config));
    }

    return (ERR_CODE_NONE);
} /* falcon16_tsc_configure_pll */

err_code_t falcon16_tsc_INTERNAL_read_pll_div(srds_access_t *sa__, uint32_t *div) {
    uint8_t fracn_sel;
    ESTM(fracn_sel = rdc_ams_pll_fracn_sel());

    if (fracn_sel) {
        uint16_t pll_fracn_ndiv_int;
        uint32_t pll_fracn_div;
        ESTM(pll_fracn_ndiv_int = rdc_ams_pll_fracn_ndiv_int());
        ESTM(pll_fracn_div = rdc_ams_pll_fracn_div() | (rdc_ams_pll_fracn_div_17_16() << 16));

        /* The value programmed into the pll_fracn_* bitfields which must
         * account for the initial div2 stage after the VCO.  For instance, a
         * divide by 147.2 must be programmed with an integer of 73 and a
         * fraction of 0.6.
         *
         * So multiply the bitfield reads by 2.
         */
        pll_fracn_ndiv_int <<= 1;
        pll_fracn_div      <<= 1;
        
        {
            /* If the post-multiplied fractional part overflows, then apply the carry to
             * the integer part.
             */
            const uint32_t pll_fracn_div_masked = pll_fracn_div & ((1 << pll_fracn_frac_bits)-1);
            if (pll_fracn_div_masked != pll_fracn_div) {
                ++pll_fracn_ndiv_int;
                pll_fracn_div = pll_fracn_div_masked;
            }
        }
    
        *div = SRDS_INTERNAL_COMPOSE_PLL_DIV(pll_fracn_ndiv_int, pll_fracn_div, pll_fracn_frac_bits);
    } else {
        uint8_t pll_mode;
        ESTM(pll_mode = rdc_pll_mode());
        switch (pll_mode) {
            case  0: *div = SRDS_INTERNAL_COMPOSE_PLL_DIV(  64, 0, 18); break;
            case  1: *div = SRDS_INTERNAL_COMPOSE_PLL_DIV(  66, 0, 18); break;
            case  2: *div = SRDS_INTERNAL_COMPOSE_PLL_DIV(  80, 0, 18); break;
            case  3: *div = SRDS_INTERNAL_COMPOSE_PLL_DIV( 128, 0, 18); break;
            case  4: *div = SRDS_INTERNAL_COMPOSE_PLL_DIV( 132, 0, 18); break;
            case  5: *div = SRDS_INTERNAL_COMPOSE_PLL_DIV( 140, 0, 18); break;
            case  6: *div = SRDS_INTERNAL_COMPOSE_PLL_DIV( 160, 0, 18); break;
            case  7: *div = SRDS_INTERNAL_COMPOSE_PLL_DIV( 165, 0, 18); break;
            case  8: *div = SRDS_INTERNAL_COMPOSE_PLL_DIV( 168, 0, 18); break;
            case  9: *div = SRDS_INTERNAL_COMPOSE_PLL_DIV( 170, 0, 18); break;
            case 10: *div = SRDS_INTERNAL_COMPOSE_PLL_DIV( 175, 0, 18); break;
            case 11: *div = SRDS_INTERNAL_COMPOSE_PLL_DIV( 180, 0, 18); break;
            case 12: *div = SRDS_INTERNAL_COMPOSE_PLL_DIV( 184, 0, 18); break;
            case 13: *div = SRDS_INTERNAL_COMPOSE_PLL_DIV( 200, 0, 18); break;
            case 14: *div = SRDS_INTERNAL_COMPOSE_PLL_DIV( 224, 0, 18); break;
            case 15: *div = SRDS_INTERNAL_COMPOSE_PLL_DIV( 264, 0, 18); break;
            case 16: *div = SRDS_INTERNAL_COMPOSE_PLL_DIV(  96, 0, 18); break;
            case 17: *div = SRDS_INTERNAL_COMPOSE_PLL_DIV( 120, 0, 18); break;
            case 18: *div = SRDS_INTERNAL_COMPOSE_PLL_DIV( 144, 0, 18); break;
            case 19: *div = SRDS_INTERNAL_COMPOSE_PLL_DIV( 198, 0, 18); break;
            default:
                EFUN_PRINTF(("ERROR: falcon16_tsc_INTERNAL_read_pll_div() found invalid pll_mode value:  %d\n", pll_mode));
                return (_error(ERR_CODE_PLL_DIV_INVALID));
        }
    }
    return (ERR_CODE_NONE);
}

