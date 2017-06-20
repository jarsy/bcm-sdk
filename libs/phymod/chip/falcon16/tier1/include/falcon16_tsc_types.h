/*****************************************************************************************
 *****************************************************************************************
 *                                                                                       *
 *  Revision      :   *
 *                                                                                       *
 *  Description   :  Functions used internally and available in debug shell only         *
 *                                                                                       *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$                                                                 *
 *  No portions of this material may be reproduced in any form without                   *
 *  the written permission of:                                                           *
 *      Broadcom Corporation                                                             *
 *      5300 California Avenue                                                           *
 *      Irvine, CA  92617                                                                *
 *                                                                                       *
 *  All information contained in this document is Broadcom Corporation                   *
 *  company private proprietary, and trade secret.                                       *
 *                                                                                       *
 *****************************************************************************************
 *****************************************************************************************/

/** @file falcon16_tsc_types.h
 * Common types used by Serdes API functions
 */

#ifndef FALCON16_TSC_API_TYPES_H
#define FALCON16_TSC_API_TYPES_H

#include "falcon16_tsc_ipconfig.h"
#include "falcon16_tsc_usr_includes.h"

/*----------------------------------------*/
/*  Lane/Core structs (without bitfields) */
/*----------------------------------------*/

/** Lane Config Variable Structure in Microcode */
struct falcon16_tsc_uc_lane_config_field_st {
  uint8_t  lane_cfg_from_pcs ;
  uint8_t  an_enabled        ;
  uint8_t  dfe_on            ;
  uint8_t  dfe_lp_mode       ;
  uint8_t  force_brdfe_on    ;
  uint8_t  media_type        ;
  uint8_t  unreliable_los    ;
  uint8_t  scrambling_dis    ;
  uint8_t  cl72_auto_polarity_en ;
  uint8_t  cl72_restart_timeout_en;
  uint8_t  reserved          ;
};

/** Core Config Variable Structure in Microcode */
struct falcon16_tsc_uc_core_config_field_st {
  uint8_t  vco_rate          ;
  uint8_t  core_cfg_from_pcs ;
  uint8_t  reserved          ;
};

/** Lane Config Struct */
struct  falcon16_tsc_uc_lane_config_st {
  struct falcon16_tsc_uc_lane_config_field_st field;
  uint16_t word;
};

/** Core Config Struct */
struct  falcon16_tsc_uc_core_config_st {
  struct falcon16_tsc_uc_core_config_field_st field;
  uint16_t word;
  int vco_rate_in_Mhz; /* if >0 then will get converted and replace field.vco_rate when update is called */
};

/** Lane User Control Function Structure in Microcode */
struct falcon16_tsc_usr_ctrl_func_st {
  /** Main Peaking Filter Adaptation */
  uint16_t pf_adaptation           ;
  /** Low Frequency Peaking Filter adaptation */
  uint16_t pf2_adaptation          ;
  /** DC Offset Adaptation */
  uint16_t dc_adaptation           ;
  /** Variable Gain Amplifier adaptation */
  uint16_t vga_adaptation          ;
  /** Slicer Vertical Offset tuning */
  uint16_t slicer_voffset_tuning   ;
  /** Slicer Horizontal Offset tuning */
  uint16_t slicer_hoffset_tuning   ;
  /** Phase Offset adaptation */
  uint16_t phase_offset_adaptation ;
  /** Eye adaptation */
  uint16_t eye_adaptation          ;
  /** All adaptation */
  uint16_t all_adaptation          ;
  uint16_t reserved                ;
};

/** Lane User DFE Control Function Structure in Microcode */
struct falcon16_tsc_usr_ctrl_dfe_func_st {
  /** DFE tap 1 adaptation */
  uint8_t dfe_tap1_adaptation      ;
  /** DFE Fixed taps adaptation */
  uint8_t dfe_fx_taps_adaptation   ;
  /** DFE Floating taps adaptation */
  uint8_t dfe_fl_taps_adaptation   ;
  /** DFE tap Duty Cycle Distortion */
  uint8_t dfe_dcd_adaptation       ;
};

/** Lane User Control Disable Function Struct */
struct falcon16_tsc_usr_ctrl_disable_functions_st {
  struct falcon16_tsc_usr_ctrl_func_st field;
  uint16_t word;
};

/** Lane User Control Disable DFE Function Struct */
struct falcon16_tsc_usr_ctrl_disable_dfe_functions_st {
  struct falcon16_tsc_usr_ctrl_dfe_func_st field;
  uint8_t  byte;
};

/** Falcon Lane status Struct */
struct falcon16_tsc_detailed_lane_status_st {
    /** Frequency offset of local reference clock with respect to RX data in ppm */
    USR_DOUBLE ppm;
    /** Estimate for Main tap */
    USR_DOUBLE main_tap_est;
    /** PRBS Error Count */
    uint32_t prbs_chk_errcnt;
    /** One Hot DSC State */
    uint16_t dsc_sm[2];
    /** Horizontal left eye margin @ 1e-5 as seen by internal diagnostic slicer in mUI and mV */
    uint16_t heye_left;
    /** Horizontal right eye margin @ 1e-5 as seen by internal diagnostic slicer in mUI and mV */
    uint16_t heye_right;
    /** Vertical upper eye margin @ 1e-5 as seen by internal diagnostic slicer in mUI and mV */
    uint16_t veye_upper;
    /** Vertical lower eye margin @ 1e-5 as seen by internal diagnostic slicer in mUI and mV */
    uint16_t veye_lower;
    /** Link time in milliseconds */
    uint16_t link_time;
    /** Temperature Index */
    uint8_t  temp_idx;
    /** TX driver to disable high voltage control */
    uint8_t  ams_tx_drv_hv_disable;
    /** Analog Resistor Calibration value */
    uint8_t  ams_tx_ana_rescal;
    /* Analog Amplitude control */
    uint8_t  amp_ctrl;
    /** TX equalization FIR pre tap weight */
    uint8_t  pre_tap;
    /** TX equalization FIR main tap weight */
    uint8_t  main_tap;
    /** TX equalization FIR post1 tap weight */
    int8_t   post1_tap;
    /** TX equalization FIR post2 tap weight */
    int8_t   post2_tap;
    /** TX equalization FIR post3 tap weight */
    int8_t   post3_tap;
    /** TX equalization FIR rpara*/
    uint8_t  rpara;
    /** Signal Detect */
    uint8_t  sigdet;
    /** PMD RX Lock */
    uint8_t  pmd_lock;
    /** Variable Gain Amplifier settings */
    uint8_t  vga;
    /** Main Peaking Filter setting */
    uint8_t  pf;
    /** Low Frequency Peaking Filter settings */
    uint8_t  pf2;
    /** Data Slicer Threshold */
    int8_t   data_thresh;
    /** Phase Slicer Threshold */
    int8_t   phase_thresh;
    /** LMS Slicer Threshold */
    int8_t   lms_thresh;
    /** Horizontal offset between data and dataq */
    uint8_t  ddq_hoffset;
    /** Horizontal offset between phase and phaseq */
    uint8_t  ppq_hoffset;
    /** Horizontal offset between lms and lmsq */
    uint8_t  llq_hoffset;
    /** Horizontal offset between data and phase */
    uint8_t  dp_hoffset;
    /** Horizontal offset between data and lms */
    uint8_t  dl_hoffset;
    /** DFE tap values */
    int8_t   dfe[26][4];
    /** DC offset DAC control value */
    int8_t   dc_offset;
    /** Slicer calibration control codes (datap) */
    int8_t   thctrl_dp[4];
    /** Slicer calibration control codes (datan) */
    int8_t   thctrl_dn[4];
    /** Slicer calibration control codes (phasep) */
    int8_t   thctrl_zp[4];
    /** Slicer calibration control codes (phasen) */
    int8_t   thctrl_zn[4];
    /** Slicer calibration control codes (lms) */
    int8_t   thctrl_l[4];
    /** PRBS Checker Enable */
    uint8_t  prbs_chk_en;
    /** Order of PRBS Checker */
    uint8_t  prbs_chk_order;
    /** PRBS Checker Lock */
    uint8_t  prbs_chk_lock;
    /** lane_reset_state **/
    uint8_t reset_state;
    /** uC stopped state **/
    uint8_t stop_state;

};

/************************************************************************//**
 * @name Diagnostic Sampling
 *
 *//*************************************************************************/
/**@{*/

#ifdef STANDALONE_EVENT
#define DIAG_MAX_SAMPLES (64)
#else
/**
 * Diagnostic sample set size, FALCON16 variant.
 * Applies to collections of BER measurements, eye margins, etc.
 */
#define DIAG_MAX_SAMPLES (64)
#endif

/**@}*/

#endif
