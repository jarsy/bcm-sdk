/*
 *         
 * $Id: merlin16.c,v 1.2.2.26 Broadcom SDK $
 * 
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *         
 *     
 */
/*
 *  $Copyright: (c) 2014 Broadcom Corporation All Rights Reserved.$
 *  $Id$
*/

#include <phymod/phymod.h>
#include "merlin16_cfg_seq.h" 
#include "merlin16_functions.h"
#include <phymod/chip/bcmi_tsce16_xgxs_defs.h>
#include "merlin16_fields.h"
#include "merlin16_dependencies.h"
#include "merlin16_field_access.h"
#include "merlin16_debug_functions.h"

/** Macro to execute a statement with automatic return of error LESTM(Local ESTM) to fix coverity error */
#define LESTM(statement) do {err_code_t __err = ERR_CODE_NONE; statement; if (__err) return(__err); } while(0)
/** Macro to invoke a function with automatic return of error */
#define LEFUN(fun) do {err_code_t __err = (fun); if (__err) return(__err); } while(0)

err_code_t merlin16_uc_active_set(const phymod_access_t *sa__, uint32_t enable)           /* set microcontroller active or not  */
{
    err_code_t __err;
    __err=ERR_CODE_NONE;
    __err=wrc_uc_active(enable); if (__err) return(__err);

    return ERR_CODE_NONE;
}

err_code_t merlin16_uc_active_get(const phymod_access_t *sa__, uint32_t *enable)           /* get microcontroller active or not  */
{
    err_code_t __err;
    __err=ERR_CODE_NONE;
    *enable=rdc_uc_active();
    if (__err) return(__err);

    return ERR_CODE_NONE;
}

err_code_t merlin16_pmd_ln_h_rstb_pkill_override(const phymod_access_t *sa__, uint16_t val) 
{
    err_code_t __err;
    __err=ERR_CODE_NONE;
    __err=wr_pmd_ln_h_rstb_pkill(val); if (__err) return(__err);

    return ERR_CODE_NONE;
}

err_code_t merlin16_core_soft_reset_release(const phymod_access_t *sa__, uint32_t enable)   /* release the pmd core soft reset */
{
    err_code_t __err;
    __err=ERR_CODE_NONE;
    __err=wrc_core_dp_s_rstb(enable); if (__err) return(__err);

    return ERR_CODE_NONE;
}

err_code_t merlin16_lane_soft_reset_release(const phymod_access_t *sa__, uint32_t enable)   /* release the pmd core soft reset */
{
    err_code_t __err;
    __err=ERR_CODE_NONE;
    __err=wr_ln_dp_s_rstb(enable); if (__err) return(__err);

    return ERR_CODE_NONE;
}

err_code_t merlin16_pmd_tx_disable_pin_dis_set(const phymod_access_t *sa__, uint32_t enable)
{
    err_code_t __err;
    __err=ERR_CODE_NONE;
    __err=wr_pmd_tx_disable_pkill(enable); if (__err) return(__err);

    return ERR_CODE_NONE;
}

err_code_t merlin16_pmd_loopback_set(const phymod_access_t *sa__, uint32_t enable)
{
    err_code_t __err;
    __err=ERR_CODE_NONE;

    __err=merlin16_dig_lpbk(sa__, enable); if (__err) return(__err);
    __err=wr_signal_detect_frc(1);         if (__err) return(__err);
    __err=wr_signal_detect_frc_val(1);     if (__err) return(__err);

    return ERR_CODE_NONE;
}

err_code_t merlin16_pmd_loopback_get(const phymod_access_t *sa__, uint32_t *enable)   
{
    err_code_t __err;
    __err=ERR_CODE_NONE;
    *enable=rd_dig_lpbk_en();
    if (__err) return(__err);

    return ERR_CODE_NONE;
}

err_code_t merlin16_rmt_lpbk_get(const phymod_access_t *sa__, uint32_t *lpbk)
{
    err_code_t __err;
    __err=ERR_CODE_NONE;
    *lpbk=rd_rmt_lpbk_en();
    if (__err) return(__err);

    return ERR_CODE_NONE;
}

err_code_t merlin16_osr_mode_get(const phymod_access_t *sa__, int *osr_mode)
{
    int osr_forced;
    err_code_t __err;
    __err=ERR_CODE_NONE;
    osr_forced = rd_osr_mode_frc();
    if (osr_forced) {
        *osr_mode = rd_osr_mode_frc_val();
        if (__err) 
            return(__err);
    } else {
        *osr_mode = rd_osr_mode_pin();
        if (__err) 
            return (__err);
    }

    return ERR_CODE_NONE;
}

int merlin16_osr_mode_to_enum(int osr_mode, phymod_osr_mode_t* osr_mode_en)
{
    switch (osr_mode) {
        case 0: *osr_mode_en = phymodOversampleMode1; break;
        case 1: *osr_mode_en = phymodOversampleMode2; break;
        case 2: *osr_mode_en = phymodOversampleMode3; break;
        case 3: *osr_mode_en = phymodOversampleMode3P3; break;
        case 4: *osr_mode_en = phymodOversampleMode4; break;
        case 5: *osr_mode_en = phymodOversampleMode5; break;
        case 6: *osr_mode_en = phymodOversampleMode7P5; break;
        case 7: *osr_mode_en = phymodOversampleMode8; break;
        case 8: *osr_mode_en = phymodOversampleMode8P25; break;
        case 9: *osr_mode_en = phymodOversampleMode10; break;
        default:
            PHYMOD_RETURN_WITH_ERR(PHYMOD_E_INTERNAL, (_PHYMOD_MSG("unsupported OS mode %d"), osr_mode));
    }
    return PHYMOD_E_NONE;
}

err_code_t merlin16_pmd_cl72_enable_get(const phymod_access_t* sa__, uint32_t* cl72_en)
{
    err_code_t __err;
    __err=ERR_CODE_NONE;
    *cl72_en = rd_cl72_ieee_training_enable(); if (__err) return(__err);
    return ERR_CODE_NONE;
}

err_code_t merlin16_polarity_set(const phymod_access_t *sa__, uint32_t tx_polarity, uint32_t rx_polarity)
{
    err_code_t __err;
    __err=ERR_CODE_NONE;
    __err=wr_tx_pmd_dp_invert(tx_polarity); if (__err) return(__err);
    __err=wr_rx_pmd_dp_invert(rx_polarity); if (__err) return(__err);
    return ERR_CODE_NONE;
}

err_code_t merlin16_polarity_get(const phymod_access_t *sa__, uint32_t* tx_polarity, uint32_t* rx_polarity)
{
    err_code_t __err;
    __err=ERR_CODE_NONE;
    *tx_polarity=rd_tx_pmd_dp_invert(); if (__err) return(__err);
    *rx_polarity=rd_rx_pmd_dp_invert(); if (__err) return(__err);
    return ERR_CODE_NONE;
}

