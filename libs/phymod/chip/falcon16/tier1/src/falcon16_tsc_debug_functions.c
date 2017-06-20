/***********************************************************************************
 ***********************************************************************************
 *  File Name     :  falcon16_tsc_debug_functions.c                                  *
 *  Created On    :  03 Nov 2015                                                   *
 *  Created By    :  Brent Roberts                                                 *
 *  Description   :  APIs for Serdes IPs                                           *
 *  Revision      :   *
 *                                                                                 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$                                                           *
 *  No portions of this material may be reproduced in any form without             *
 *  the written permission of:                                                     *
 *      Broadcom Corporation                                                       *
 *      5300 California Avenue                                                     *
 *      Irvine, CA  92617                                                          *
 *                                                                                 *
 *  All information contained in this document is Broadcom Corporation             *
 *  company private proprietary, and trade secret.                                 *
 *                                                                                 *
 ***********************************************************************************
 ***********************************************************************************/

/** @file falcon16_tsc_debug_functions.c
 * Implementation of API debug functions
 */

#include "../include/falcon16_tsc_debug_functions.h"
#include "../include/falcon16_tsc_access.h"
#include "../include/falcon16_tsc_common.h"
#include "../include/falcon16_tsc_config.h"
#include "../include/falcon16_tsc_functions.h"
#include "../include/falcon16_tsc_internal.h"
#include "../include/falcon16_tsc_internal_error.h"
#include "../include/falcon16_tsc_prbs.h"
#include "../include/falcon16_tsc_select_defns.h"

/*************************/
/*  Stop/Resume uC Lane  */
/*************************/

err_code_t falcon16_tsc_stop_uc_lane(srds_access_t *sa__, uint8_t enable) {

  if (enable) {
    return(falcon16_tsc_pmd_uc_control(sa__, CMD_UC_CTRL_STOP_GRACEFULLY,100));
  }
  else {
    return(falcon16_tsc_pmd_uc_control(sa__, CMD_UC_CTRL_RESUME,50));
  }
}


err_code_t falcon16_tsc_stop_uc_lane_status(srds_access_t *sa__, uint8_t *uc_lane_stopped) {

  if(!uc_lane_stopped) {
      return(_error(ERR_CODE_BAD_PTR_OR_INVALID_INPUT));
  }

  ESTM(*uc_lane_stopped = rdv_usr_sts_micro_stopped());

  return (ERR_CODE_NONE);
}

/*******************************************************************/
/*  APIs to Write Core/Lane Config and User variables into uC RAM  */
/*******************************************************************/

err_code_t falcon16_tsc_set_usr_ctrl_core_event_log_level(srds_access_t *sa__, uint8_t core_event_log_level) {
  return(wrcv_usr_ctrl_core_event_log_level(core_event_log_level));
}

err_code_t falcon16_tsc_set_usr_ctrl_lane_event_log_level(srds_access_t *sa__, uint8_t lane_event_log_level) {
  return(wrv_usr_ctrl_lane_event_log_level(lane_event_log_level));
}

err_code_t falcon16_tsc_set_usr_ctrl_disable_startup(srds_access_t *sa__, struct falcon16_tsc_usr_ctrl_disable_functions_st set_val) {
  EFUN(falcon16_tsc_INTERNAL_update_usr_ctrl_disable_functions_byte(&set_val));
  return(wrv_usr_ctrl_disable_startup_functions_word(set_val.word));
}

err_code_t falcon16_tsc_set_usr_ctrl_disable_startup_dfe(srds_access_t *sa__, struct falcon16_tsc_usr_ctrl_disable_dfe_functions_st set_val) {
  EFUN(falcon16_tsc_INTERNAL_update_usr_ctrl_disable_dfe_functions_byte(&set_val));
  return(wrv_usr_ctrl_disable_startup_dfe_functions_byte(set_val.byte));
}

err_code_t falcon16_tsc_set_usr_ctrl_disable_steady_state(srds_access_t *sa__, struct falcon16_tsc_usr_ctrl_disable_functions_st set_val) {
  EFUN(falcon16_tsc_INTERNAL_update_usr_ctrl_disable_functions_byte(&set_val));
  return(wrv_usr_ctrl_disable_steady_state_functions_word(set_val.word));
}

err_code_t falcon16_tsc_set_usr_ctrl_disable_steady_state_dfe(srds_access_t *sa__, struct falcon16_tsc_usr_ctrl_disable_dfe_functions_st set_val) {
  EFUN(falcon16_tsc_INTERNAL_update_usr_ctrl_disable_dfe_functions_byte(&set_val));
  return(wrv_usr_ctrl_disable_steady_state_dfe_functions_byte(set_val.byte));
}

/******************************************************************/
/*  APIs to Read Core/Lane Config and User variables from uC RAM  */
/******************************************************************/

err_code_t falcon16_tsc_get_usr_ctrl_core_event_log_level(srds_access_t *sa__, uint8_t *core_event_log_level) {

  if(!core_event_log_level) {
     return(_error(ERR_CODE_BAD_PTR_OR_INVALID_INPUT));
  }

  ESTM(*core_event_log_level = rdcv_usr_ctrl_core_event_log_level());

  return (ERR_CODE_NONE);
}

err_code_t falcon16_tsc_get_usr_ctrl_lane_event_log_level(srds_access_t *sa__, uint8_t *lane_event_log_level) {

  if(!lane_event_log_level) {
     return(_error(ERR_CODE_BAD_PTR_OR_INVALID_INPUT));
  }

  ESTM(*lane_event_log_level = rdv_usr_ctrl_lane_event_log_level());
  return (ERR_CODE_NONE);
}

err_code_t falcon16_tsc_get_usr_ctrl_disable_startup(srds_access_t *sa__, struct falcon16_tsc_usr_ctrl_disable_functions_st *get_val) {

  if(!get_val) {
     return(_error(ERR_CODE_BAD_PTR_OR_INVALID_INPUT));
  }

  ESTM(get_val->word = rdv_usr_ctrl_disable_startup_functions_word());
  EFUN(falcon16_tsc_INTERNAL_update_usr_ctrl_disable_functions_st(get_val));
  return (ERR_CODE_NONE);
}

err_code_t falcon16_tsc_get_usr_ctrl_disable_startup_dfe(srds_access_t *sa__, struct falcon16_tsc_usr_ctrl_disable_dfe_functions_st *get_val) {

  if(!get_val) {
     return(_error(ERR_CODE_BAD_PTR_OR_INVALID_INPUT));
  }

  ESTM(get_val->byte = rdv_usr_ctrl_disable_startup_dfe_functions_byte());
  EFUN(falcon16_tsc_INTERNAL_update_usr_ctrl_disable_dfe_functions_st(get_val));
  return (ERR_CODE_NONE);
}

err_code_t falcon16_tsc_get_usr_ctrl_disable_steady_state(srds_access_t *sa__, struct falcon16_tsc_usr_ctrl_disable_functions_st *get_val) {

  if(!get_val) {
     return(_error(ERR_CODE_BAD_PTR_OR_INVALID_INPUT));
  }

  ESTM(get_val->word = rdv_usr_ctrl_disable_steady_state_functions_word());
  EFUN(falcon16_tsc_INTERNAL_update_usr_ctrl_disable_functions_st(get_val));
  return (ERR_CODE_NONE);
}

err_code_t falcon16_tsc_get_usr_ctrl_disable_steady_state_dfe(srds_access_t *sa__, struct falcon16_tsc_usr_ctrl_disable_dfe_functions_st *get_val) {

  if(!get_val) {
     return(_error(ERR_CODE_BAD_PTR_OR_INVALID_INPUT));
  }

  ESTM(get_val->byte = rdv_usr_ctrl_disable_steady_state_dfe_functions_byte());
  EFUN(falcon16_tsc_INTERNAL_update_usr_ctrl_disable_dfe_functions_st(get_val));
  return (ERR_CODE_NONE);
}

/****************************************/
/*  Serdes Register/Variable Dump APIs  */
/****************************************/

err_code_t falcon16_tsc_reg_dump(srds_access_t *sa__) {
  uint16_t addr, rddata;

  EFUN_PRINTF(("\n****  SERDES REGISTER DUMP    ****"));

  for (addr = 0x0; addr < 0x10; addr++) {
    if (!(addr % 16))  {
      EFUN_PRINTF(("\n%04x ",addr));
    }
    EFUN(falcon16_tsc_pmd_rdt_reg(sa__, addr,&rddata));
    EFUN_PRINTF(("%04x ",rddata));
  }

  for (addr = 0x90; addr < 0xA0; addr++) {
    if (!(addr % 16))  {
      EFUN_PRINTF(("\n%04x ",addr));
    }
    EFUN(falcon16_tsc_pmd_rdt_reg(sa__, addr,&rddata));
    EFUN_PRINTF(("%04x ",rddata));
  }

  for (addr = 0xD000; addr < 0xD1A0; addr++) {
    if (!(addr % 16))  {
      EFUN_PRINTF(("\n%04x ",addr));
    }
    EFUN(falcon16_tsc_pmd_rdt_reg(sa__, addr,&rddata));
    EFUN_PRINTF(("%04x ",rddata));
  }

  for (addr = 0xD1A0; addr < 0xD200; addr++) {
    if (!(addr % 16))  {
      EFUN_PRINTF(("\n%04x ",addr));
    }
    EFUN(falcon16_tsc_pmd_rdt_reg(sa__, addr,&rddata));
    EFUN_PRINTF(("%04x ",rddata));
  }

  for (addr = 0xD200; addr < 0xD230; addr++) {
    if (!(addr % 16))  {
      EFUN_PRINTF(("\n%04x ",addr));
    }
    EFUN(falcon16_tsc_pmd_rdt_reg(sa__, addr,&rddata));
    EFUN_PRINTF(("%04x ",rddata));
  }

  for (addr = 0xE000; addr < 0xE010; addr++) {
    if (!(addr % 16))  {
      EFUN_PRINTF(("\n%04x ",addr));
    }
    EFUN(falcon16_tsc_pmd_rdt_reg(sa__, addr,&rddata));
    EFUN_PRINTF(("%04x ",rddata));
  }

  for (addr = 0xFFD0; addr < 0xFFE0; addr++) {
    if (!(addr % 16))  {
      EFUN_PRINTF(("\n%04x ",addr));
    }
    EFUN(falcon16_tsc_pmd_rdt_reg(sa__, addr,&rddata));
    EFUN_PRINTF(("%04x ",rddata));
  }
  return (ERR_CODE_NONE);
}


err_code_t falcon16_tsc_uc_core_var_dump(srds_access_t *sa__) {
  uint8_t addr;

  EFUN_PRINTF(("\n**** SERDES UC CORE RAM VARIABLE DUMP ****"));

  for (addr = 0x0; addr < 0xFF; addr++) {
    if (!(addr % 26))  {
      EFUN_PRINTF(("\n%04x ",addr));
    }
    ESTM_PRINTF(("%02x ", falcon16_tsc_rdbc_uc_var(sa__, __ERR, addr)));
  }
  return (ERR_CODE_NONE);
}


err_code_t falcon16_tsc_uc_lane_var_dump(srds_access_t *sa__) {
  uint8_t     rx_lock, uc_stopped = 0;
  uint16_t    addr;

  EFUN_PRINTF(("\n**** SERDES UC LANE %d RAM VARIABLE DUMP ****",falcon16_tsc_get_lane(sa__)));

  ESTM(rx_lock = rd_pmd_rx_lock());

  if (rx_lock == 1) {
      ESTM(uc_stopped = rdv_usr_sts_micro_stopped());
      if (!uc_stopped) {
          EFUN(falcon16_tsc_stop_rx_adaptation(sa__, 1));
      }
  } else {
      EFUN(falcon16_tsc_pmd_uc_control(sa__, CMD_UC_CTRL_STOP_IMMEDIATE,200));
  }

  for (addr = 0x0; addr < LANE_VAR_RAM_SIZE; addr++) {
    if (!(addr % 26))  {
      EFUN_PRINTF(("\n%04x ",addr));
    }
    ESTM_PRINTF(("%02x ", falcon16_tsc_rdbl_uc_var(sa__, __ERR, addr)));
  }

  if (rx_lock == 1) {
      if (!uc_stopped) {
          EFUN(falcon16_tsc_stop_rx_adaptation(sa__, 0));
      }
  } else {
      EFUN(falcon16_tsc_stop_rx_adaptation(sa__, 0));
  }

  return (ERR_CODE_NONE);
}

/***************************************/
/*  API Function to Read Event Logger  */
/***************************************/

err_code_t falcon16_tsc_read_event_log(srds_access_t *sa__) {
    falcon16_tsc_INTERNAL_event_log_dump_state_t state;
    uint8_t micro_num = 0;
#if (NUM_MICROS > 1)
    for (; micro_num<NUM_MICROS; ++micro_num)
#endif
    {
        state.index = 0;
        state.line_start_index = 0;
        EFUN(falcon16_tsc_INTERNAL_read_event_log_with_callback(sa__, micro_num, 0, &state, falcon16_tsc_INTERNAL_event_log_dump_callback));
        EFUN(falcon16_tsc_INTERNAL_event_log_dump_callback(&state, 0, 0));
    }
    return(ERR_CODE_NONE);
}

/**********************************************/
/*  Loopback and Ultra-Low Latency Functions  */
/**********************************************/

/* Enable/Diasble Digital Loopback */
err_code_t falcon16_tsc_dig_lpbk(srds_access_t *sa__, uint8_t enable) {
  EFUN(wr_dig_lpbk_en(enable));                         /* 0 = diabled, 1 = enabled */
  return (ERR_CODE_NONE);
}


/**********************************/
/*  TX_PI Jitter Generation APIs  */
/**********************************/

/* TX_PI Sinusoidal or Spread-Spectrum (SSC) Jitter Generation  */
err_code_t falcon16_tsc_tx_pi_jitt_gen(srds_access_t *sa__, uint8_t enable, int16_t freq_override_val, enum srds_tx_pi_freq_jit_gen_enum jit_type, uint8_t tx_pi_jit_freq_idx, uint8_t tx_pi_jit_amp) {
    /* Added a limiting for the jitter amplitude index, per freq_idx */
    uint8_t max_amp_idx_r20_os1[] = {37, 42, 48, 56, 33, 39, 47, 58, 37, 42, 48, 56, 33, 39, 47, 58, 37, 42, 48, 56, 33, 39, 47, 58, 37, 42, 48, 56, 33, 39, 47, 58, 37, 42, 48, 56, 33, 39, 47, 58, 37, 42, 48, 56, 33, 39, 47, 58, 37, 42, 48, 56, 33, 39, 47, 58, 37, 48, 33, 47, 37, 33, 37, 37};

    /* Irrespective of the osr_mode, txpi runs @ os1. Thus the max amp idx values remain the same. */
    if (jit_type == TX_PI_SJ) {
        if (tx_pi_jit_amp > max_amp_idx_r20_os1[tx_pi_jit_freq_idx]) {
            tx_pi_jit_amp = max_amp_idx_r20_os1[tx_pi_jit_freq_idx];
        }
    }

    EFUN(falcon16_tsc_tx_pi_freq_override(sa__, enable, freq_override_val));

    if (enable) {
        EFUN(wr_tx_pi_jit_freq_idx(tx_pi_jit_freq_idx));
        EFUN(wr_tx_pi_jit_amp(tx_pi_jit_amp));

        if (jit_type == TX_PI_SSC_HIGH_FREQ) {
            EFUN(wr_tx_pi_jit_ssc_freq_mode(0x1));        /* SSC_FREQ_MODE:             0 = 6G SSC mode, 1 = 10G SSC mode */
            EFUN(wr_tx_pi_ssc_gen_en(0x1));               /* SSC jitter enable:         0 = disabled,    1 = enabled */
        }
        else if (jit_type == TX_PI_SSC_LOW_FREQ) {
            EFUN(wr_tx_pi_jit_ssc_freq_mode(0x0));        /* SSC_FREQ_MODE:             0 = 6G SSC mode, 1 = 10G SSC mode */
            EFUN(wr_tx_pi_ssc_gen_en(0x1));               /* SSC jitter enable:         0 = disabled,    1 = enabled */
        }
        else if (jit_type == TX_PI_SJ) {
            EFUN(wr_tx_pi_sj_gen_en(0x1));                /* Sinusoidal jitter enable:  0 = disabled,    1 = enabled */
        }
    }
    else {
        EFUN(wr_tx_pi_ssc_gen_en(0x0));                   /* SSC jitter enable:         0 = disabled,    1 = enabled */
        EFUN(wr_tx_pi_sj_gen_en(0x0));                    /* Sinusoidal jitter enable:  0 = disabled,    1 = enabled */
    }
  return (ERR_CODE_NONE);
}


/*******************************/
/*  Isolate Serdes Input Pins  */
/*******************************/

err_code_t falcon16_tsc_isolate_ctrl_pins(srds_access_t *sa__, uint8_t enable) {

    EFUN(falcon16_tsc_isolate_lane_ctrl_pins(sa__, enable));
    EFUN(falcon16_tsc_isolate_core_ctrl_pins(sa__, enable));

  return (ERR_CODE_NONE);
}

err_code_t falcon16_tsc_isolate_lane_ctrl_pins(srds_access_t *sa__, uint8_t enable) {

  if (enable) {
    EFUN(wr_pmd_ln_tx_h_pwrdn_pkill(0x1));
    EFUN(wr_pmd_ln_rx_h_pwrdn_pkill(0x1));
    EFUN(wr_pmd_ln_dp_h_rstb_pkill(0x1));
    EFUN(wr_pmd_ln_h_rstb_pkill(0x1));
    EFUN(wr_pmd_tx_disable_pkill(0x1));
  }
  else {
    EFUN(wr_pmd_ln_tx_h_pwrdn_pkill(0x0));
    EFUN(wr_pmd_ln_rx_h_pwrdn_pkill(0x0));
    EFUN(wr_pmd_ln_dp_h_rstb_pkill(0x0));
    EFUN(wr_pmd_ln_h_rstb_pkill(0x0));
    EFUN(wr_pmd_tx_disable_pkill(0x0));
  }
  return (ERR_CODE_NONE);
}

err_code_t falcon16_tsc_isolate_core_ctrl_pins(srds_access_t *sa__, uint8_t enable) {

  if (enable) {
    EFUN(wrc_pmd_core_dp_h_rstb_pkill(0x1));
  }
  else {
    EFUN(wrc_pmd_core_dp_h_rstb_pkill(0x0));
  }
  return (ERR_CODE_NONE);
}



err_code_t falcon16_tsc_log_full_pmd_state_noPRBS (srds_access_t *sa__, struct falcon16_tsc_detailed_lane_status_st *lane_st) {
    uint16_t reg_data;
    int8_t tmp;

    if(!lane_st) 
      return(_error(ERR_CODE_BAD_PTR_OR_INVALID_INPUT));

    ESTM(lane_st->pmd_lock = rd_pmd_rx_lock());

    lane_st->stop_state = 0;
    if (lane_st->pmd_lock == 1) {
        ESTM(lane_st->stop_state = rdv_usr_sts_micro_stopped());
        if (!lane_st->stop_state) {
          EFUN(falcon16_tsc_stop_rx_adaptation(sa__, 1));
      }
    } else {
        EFUN(falcon16_tsc_pmd_uc_control(sa__, CMD_UC_CTRL_STOP_IMMEDIATE,200));
    }
  
    ESTM(lane_st->reset_state = rd_lane_dp_reset_state());

    ESTM(lane_st->temp_idx = rdcv_temp_idx());
    ESTM(lane_st->ams_tx_drv_hv_disable = rd_ams_tx_drv_hv_disable());    
    ESTM(lane_st->ams_tx_ana_rescal = rd_ams_tx_ana_rescal());  
    EFUN(falcon16_tsc_read_tx_afe(sa__, TX_AFE_PRE, &tmp)); lane_st->pre_tap = tmp;
    EFUN(falcon16_tsc_read_tx_afe(sa__, TX_AFE_MAIN, &tmp)); lane_st->main_tap = tmp;
    EFUN(falcon16_tsc_read_tx_afe(sa__, TX_AFE_POST1, &lane_st->post1_tap));
    EFUN(falcon16_tsc_read_tx_afe(sa__, TX_AFE_POST2, &lane_st->post2_tap));
    EFUN(falcon16_tsc_read_tx_afe(sa__, TX_AFE_POST3, &lane_st->post3_tap));
    EFUN(falcon16_tsc_read_tx_afe(sa__, TX_AFE_RPARA, &tmp)); lane_st->rpara = tmp;
    ESTM(lane_st->sigdet = rd_signal_detect());
    ESTM(lane_st->dsc_sm[0] = rd_dsc_state_one_hot());
    ESTM(lane_st->dsc_sm[1] = rd_dsc_state_one_hot());
#ifdef SERDES_API_FLOATING_POINT 
    ESTM(lane_st->ppm = (((double)1e6/64/20/128/16)*(int16_t)(rd_cdr_integ_reg()/32)));
#else
    ESTM(lane_st->ppm = ((int16_t)(rd_cdr_integ_reg())*12/1000));
#endif
    ESTM(lane_st->vga = rd_rx_vga_ctrl());
    EFUN(falcon16_tsc_INTERNAL_get_rx_pf_main(sa__, &tmp)); lane_st->pf = tmp;
    EFUN(falcon16_tsc_INTERNAL_get_rx_pf2(sa__, &tmp)); lane_st->pf2 = tmp;
#ifdef SERDES_API_FLOATING_POINT 
    ESTM(lane_st->main_tap_est = rdv_usr_main_tap_est()/32.0);
#else
    ESTM(lane_st->main_tap_est = rdv_usr_main_tap_est()/32);
#endif
    EFUN(falcon16_tsc_INTERNAL_get_rx_dfe1(sa__, &lane_st->data_thresh));
    ESTM(lane_st->phase_thresh = rd_rx_phase_thresh_sel());
    ESTM(lane_st->lms_thresh = rd_rx_lms_thresh_sel());
    ESTM(reg_data = reg_rd_DSC_E_RX_PI_CNT_BIN_D());
    lane_st->ddq_hoffset = (uint8_t)dist_ccw(((reg_data>>8)&0xFF),(reg_data&0xFF));
    ESTM(reg_data = reg_rd_DSC_E_RX_PI_CNT_BIN_P());
    lane_st->ppq_hoffset = (uint8_t)dist_ccw(((reg_data>>8)&0xFF),(reg_data&0xFF));
    ESTM(reg_data = reg_rd_DSC_E_RX_PI_CNT_BIN_L());
    lane_st->llq_hoffset = (uint8_t)dist_ccw(((reg_data>>8)&0xFF),(reg_data&0xFF));
    ESTM(reg_data = reg_rd_DSC_E_RX_PI_CNT_BIN_PD());
    lane_st->dp_hoffset = (uint8_t)dist_cw(((reg_data>>8)&0xFF),(reg_data&0xFF));
    ESTM(reg_data = reg_rd_DSC_E_RX_PI_CNT_BIN_LD());
    lane_st->dl_hoffset = (uint8_t)dist_cw(((reg_data>>8)&0xFF),(reg_data&0xFF));
    ESTM(lane_st->dc_offset = rd_dc_offset_bin());
    ESTM(lane_st->dfe[1][0] = rd_rxa_dfe_tap2());
    ESTM(lane_st->dfe[1][1] = rd_rxb_dfe_tap2());
    ESTM(lane_st->dfe[1][2] = rd_rxc_dfe_tap2());
    ESTM(lane_st->dfe[1][3] = rd_rxd_dfe_tap2());
    ESTM(lane_st->dfe[2][0] = rd_rxa_dfe_tap3());
    ESTM(lane_st->dfe[2][1] = rd_rxb_dfe_tap3());
    ESTM(lane_st->dfe[2][2] = rd_rxc_dfe_tap3());
    ESTM(lane_st->dfe[2][3] = rd_rxd_dfe_tap3());
    ESTM(lane_st->dfe[3][0] = rd_rxa_dfe_tap4());
    ESTM(lane_st->dfe[3][1] = rd_rxb_dfe_tap4());
    ESTM(lane_st->dfe[3][2] = rd_rxc_dfe_tap4());
    ESTM(lane_st->dfe[3][3] = rd_rxd_dfe_tap4());
    ESTM(lane_st->dfe[4][0] = rd_rxa_dfe_tap5());
    ESTM(lane_st->dfe[4][1] = rd_rxb_dfe_tap5());
    ESTM(lane_st->dfe[4][2] = rd_rxc_dfe_tap5());
    ESTM(lane_st->dfe[4][3] = rd_rxd_dfe_tap5());
    ESTM(lane_st->dfe[5][0] = rd_rxa_dfe_tap6());
    ESTM(lane_st->dfe[5][1] = rd_rxb_dfe_tap6());
    ESTM(lane_st->dfe[5][2] = rd_rxc_dfe_tap6());
    ESTM(lane_st->dfe[5][3] = rd_rxd_dfe_tap6());
    ESTM(lane_st->dfe[6][0] = ((rd_rxa_dfe_tap7_mux()==0)?rd_rxa_dfe_tap7():0));
    ESTM(lane_st->dfe[6][1] = ((rd_rxb_dfe_tap7_mux()==0)?rd_rxb_dfe_tap7():0));
    ESTM(lane_st->dfe[6][2] = ((rd_rxc_dfe_tap7_mux()==0)?rd_rxc_dfe_tap7():0));
    ESTM(lane_st->dfe[6][3] = ((rd_rxd_dfe_tap7_mux()==0)?rd_rxd_dfe_tap7():0));
    ESTM(lane_st->dfe[7][0] = ((rd_rxa_dfe_tap8_mux()==0)?rd_rxa_dfe_tap8():0));
    ESTM(lane_st->dfe[7][1] = ((rd_rxb_dfe_tap8_mux()==0)?rd_rxb_dfe_tap8():0));
    ESTM(lane_st->dfe[7][2] = ((rd_rxc_dfe_tap8_mux()==0)?rd_rxc_dfe_tap8():0));
    ESTM(lane_st->dfe[7][3] = ((rd_rxd_dfe_tap8_mux()==0)?rd_rxd_dfe_tap8():0));
    ESTM(lane_st->dfe[8][0] = ((rd_rxa_dfe_tap9_mux()==0)?rd_rxa_dfe_tap9():0));
    ESTM(lane_st->dfe[8][1] = ((rd_rxb_dfe_tap9_mux()==0)?rd_rxb_dfe_tap9():0));
    ESTM(lane_st->dfe[8][2] = ((rd_rxc_dfe_tap9_mux()==0)?rd_rxc_dfe_tap9():0));
    ESTM(lane_st->dfe[8][3] = ((rd_rxd_dfe_tap9_mux()==0)?rd_rxd_dfe_tap9():0));
    ESTM(lane_st->dfe[9][0] = ((rd_rxa_dfe_tap10_mux()==0)?rd_rxa_dfe_tap10():0));
    ESTM(lane_st->dfe[9][1] = ((rd_rxb_dfe_tap10_mux()==0)?rd_rxb_dfe_tap10():0));
    ESTM(lane_st->dfe[9][2] = ((rd_rxc_dfe_tap10_mux()==0)?rd_rxc_dfe_tap10():0));
    ESTM(lane_st->dfe[9][3] = ((rd_rxd_dfe_tap10_mux()==0)?rd_rxd_dfe_tap10():0));
    ESTM(lane_st->dfe[10][0] = ((rd_rxa_dfe_tap7_mux()==1)?rd_rxa_dfe_tap7():(rd_rxa_dfe_tap11_mux()==0)?rd_rxa_dfe_tap11():0));
    ESTM(lane_st->dfe[10][1] = ((rd_rxb_dfe_tap7_mux()==1)?rd_rxb_dfe_tap7():(rd_rxb_dfe_tap11_mux()==0)?rd_rxb_dfe_tap11():0));
    ESTM(lane_st->dfe[10][2] = ((rd_rxc_dfe_tap7_mux()==1)?rd_rxc_dfe_tap7():(rd_rxc_dfe_tap11_mux()==0)?rd_rxc_dfe_tap11():0));
    ESTM(lane_st->dfe[10][3] = ((rd_rxd_dfe_tap7_mux()==1)?rd_rxd_dfe_tap7():(rd_rxd_dfe_tap11_mux()==0)?rd_rxd_dfe_tap11():0));
    ESTM(lane_st->dfe[11][0] = ((rd_rxa_dfe_tap8_mux()==1)?rd_rxa_dfe_tap8():(rd_rxa_dfe_tap12_mux()==0)?rd_rxa_dfe_tap12():0));
    ESTM(lane_st->dfe[11][1] = ((rd_rxb_dfe_tap8_mux()==1)?rd_rxb_dfe_tap8():(rd_rxb_dfe_tap12_mux()==0)?rd_rxb_dfe_tap12():0));
    ESTM(lane_st->dfe[11][2] = ((rd_rxc_dfe_tap8_mux()==1)?rd_rxc_dfe_tap8():(rd_rxc_dfe_tap12_mux()==0)?rd_rxc_dfe_tap12():0));
    ESTM(lane_st->dfe[11][3] = ((rd_rxd_dfe_tap8_mux()==1)?rd_rxd_dfe_tap8():(rd_rxd_dfe_tap12_mux()==0)?rd_rxd_dfe_tap12():0));
    ESTM(lane_st->dfe[12][0] = ((rd_rxa_dfe_tap9_mux()==1)?rd_rxa_dfe_tap9():(rd_rxa_dfe_tap13_mux()==0)?rd_rxa_dfe_tap13():0));
    ESTM(lane_st->dfe[12][1] = ((rd_rxb_dfe_tap9_mux()==1)?rd_rxb_dfe_tap9():(rd_rxb_dfe_tap13_mux()==0)?rd_rxb_dfe_tap13():0));
    ESTM(lane_st->dfe[12][2] = ((rd_rxc_dfe_tap9_mux()==1)?rd_rxc_dfe_tap9():(rd_rxc_dfe_tap13_mux()==0)?rd_rxc_dfe_tap13():0));
    ESTM(lane_st->dfe[12][3] = ((rd_rxd_dfe_tap9_mux()==1)?rd_rxd_dfe_tap9():(rd_rxd_dfe_tap13_mux()==0)?rd_rxd_dfe_tap13():0));
    ESTM(lane_st->dfe[13][0] = ((rd_rxa_dfe_tap10_mux()==1)?rd_rxa_dfe_tap10():(rd_rxa_dfe_tap14_mux()==0)?rd_rxa_dfe_tap14():0));
    ESTM(lane_st->dfe[13][1] = ((rd_rxb_dfe_tap10_mux()==1)?rd_rxb_dfe_tap10():(rd_rxb_dfe_tap14_mux()==0)?rd_rxb_dfe_tap14():0));
    ESTM(lane_st->dfe[13][2] = ((rd_rxc_dfe_tap10_mux()==1)?rd_rxc_dfe_tap10():(rd_rxc_dfe_tap14_mux()==0)?rd_rxc_dfe_tap14():0));
    ESTM(lane_st->dfe[13][3] = ((rd_rxd_dfe_tap10_mux()==1)?rd_rxd_dfe_tap10():(rd_rxd_dfe_tap14_mux()==0)?rd_rxd_dfe_tap14():0));
    ESTM(lane_st->dfe[14][0] = ((rd_rxa_dfe_tap7_mux()==2)?rd_rxa_dfe_tap7():(rd_rxa_dfe_tap11_mux()==1)?rd_rxa_dfe_tap11():0));
    ESTM(lane_st->dfe[14][1] = ((rd_rxb_dfe_tap7_mux()==2)?rd_rxb_dfe_tap7():(rd_rxb_dfe_tap11_mux()==1)?rd_rxb_dfe_tap11():0));
    ESTM(lane_st->dfe[14][2] = ((rd_rxc_dfe_tap7_mux()==2)?rd_rxc_dfe_tap7():(rd_rxc_dfe_tap11_mux()==1)?rd_rxc_dfe_tap11():0));
    ESTM(lane_st->dfe[14][3] = ((rd_rxd_dfe_tap7_mux()==2)?rd_rxd_dfe_tap7():(rd_rxd_dfe_tap11_mux()==1)?rd_rxd_dfe_tap11():0));
    ESTM(lane_st->dfe[15][0] = ((rd_rxa_dfe_tap8_mux()==2)?rd_rxa_dfe_tap8():(rd_rxa_dfe_tap12_mux()==1)?rd_rxa_dfe_tap12():0));
    ESTM(lane_st->dfe[15][1] = ((rd_rxb_dfe_tap8_mux()==2)?rd_rxb_dfe_tap8():(rd_rxb_dfe_tap12_mux()==1)?rd_rxb_dfe_tap12():0));
    ESTM(lane_st->dfe[15][2] = ((rd_rxc_dfe_tap8_mux()==2)?rd_rxc_dfe_tap8():(rd_rxc_dfe_tap12_mux()==1)?rd_rxc_dfe_tap12():0));
    ESTM(lane_st->dfe[15][3] = ((rd_rxd_dfe_tap8_mux()==2)?rd_rxd_dfe_tap8():(rd_rxd_dfe_tap12_mux()==1)?rd_rxd_dfe_tap12():0));
    ESTM(lane_st->dfe[16][0] = ((rd_rxa_dfe_tap9_mux()==2)?rd_rxa_dfe_tap9():(rd_rxa_dfe_tap13_mux()==1)?rd_rxa_dfe_tap13():0));
    ESTM(lane_st->dfe[16][1] = ((rd_rxb_dfe_tap9_mux()==2)?rd_rxb_dfe_tap9():(rd_rxb_dfe_tap13_mux()==1)?rd_rxb_dfe_tap13():0));
    ESTM(lane_st->dfe[16][2] = ((rd_rxc_dfe_tap9_mux()==2)?rd_rxc_dfe_tap9():(rd_rxc_dfe_tap13_mux()==1)?rd_rxc_dfe_tap13():0));
    ESTM(lane_st->dfe[16][3] = ((rd_rxd_dfe_tap9_mux()==2)?rd_rxd_dfe_tap9():(rd_rxd_dfe_tap13_mux()==1)?rd_rxd_dfe_tap13():0));
    ESTM(lane_st->dfe[17][0] = ((rd_rxa_dfe_tap10_mux()==2)?rd_rxa_dfe_tap10():(rd_rxa_dfe_tap14_mux()==1)?rd_rxa_dfe_tap14():0));
    ESTM(lane_st->dfe[17][1] = ((rd_rxb_dfe_tap10_mux()==2)?rd_rxb_dfe_tap10():(rd_rxb_dfe_tap14_mux()==1)?rd_rxb_dfe_tap14():0));
    ESTM(lane_st->dfe[17][2] = ((rd_rxc_dfe_tap10_mux()==2)?rd_rxc_dfe_tap10():(rd_rxc_dfe_tap14_mux()==1)?rd_rxc_dfe_tap14():0));
    ESTM(lane_st->dfe[17][3] = ((rd_rxd_dfe_tap10_mux()==2)?rd_rxd_dfe_tap10():(rd_rxd_dfe_tap14_mux()==1)?rd_rxd_dfe_tap14():0));
    ESTM(lane_st->dfe[18][0] = ((rd_rxa_dfe_tap7_mux()==3)?rd_rxa_dfe_tap7():(rd_rxa_dfe_tap11_mux()==2)?rd_rxa_dfe_tap11():0));
    ESTM(lane_st->dfe[18][1] = ((rd_rxb_dfe_tap7_mux()==3)?rd_rxb_dfe_tap7():(rd_rxb_dfe_tap11_mux()==2)?rd_rxb_dfe_tap11():0));
    ESTM(lane_st->dfe[18][2] = ((rd_rxc_dfe_tap7_mux()==3)?rd_rxc_dfe_tap7():(rd_rxc_dfe_tap11_mux()==2)?rd_rxc_dfe_tap11():0));
    ESTM(lane_st->dfe[18][3] = ((rd_rxd_dfe_tap7_mux()==3)?rd_rxd_dfe_tap7():(rd_rxd_dfe_tap11_mux()==2)?rd_rxd_dfe_tap11():0));
    ESTM(lane_st->dfe[19][0] = ((rd_rxa_dfe_tap8_mux()==3)?rd_rxa_dfe_tap8():(rd_rxa_dfe_tap12_mux()==2)?rd_rxa_dfe_tap12():0));
    ESTM(lane_st->dfe[19][1] = ((rd_rxb_dfe_tap8_mux()==3)?rd_rxb_dfe_tap8():(rd_rxb_dfe_tap12_mux()==2)?rd_rxb_dfe_tap12():0));
    ESTM(lane_st->dfe[19][2] = ((rd_rxc_dfe_tap8_mux()==3)?rd_rxc_dfe_tap8():(rd_rxc_dfe_tap12_mux()==2)?rd_rxc_dfe_tap12():0));
    ESTM(lane_st->dfe[19][3] = ((rd_rxd_dfe_tap8_mux()==3)?rd_rxd_dfe_tap8():(rd_rxd_dfe_tap12_mux()==2)?rd_rxd_dfe_tap12():0));
    ESTM(lane_st->dfe[20][0] = ((rd_rxa_dfe_tap9_mux()==3)?rd_rxa_dfe_tap9():(rd_rxa_dfe_tap13_mux()==2)?rd_rxa_dfe_tap13():0));
    ESTM(lane_st->dfe[20][1] = ((rd_rxb_dfe_tap9_mux()==3)?rd_rxb_dfe_tap9():(rd_rxb_dfe_tap13_mux()==2)?rd_rxb_dfe_tap13():0));
    ESTM(lane_st->dfe[20][2] = ((rd_rxc_dfe_tap9_mux()==3)?rd_rxc_dfe_tap9():(rd_rxc_dfe_tap13_mux()==2)?rd_rxc_dfe_tap13():0));
    ESTM(lane_st->dfe[20][3] = ((rd_rxd_dfe_tap9_mux()==3)?rd_rxd_dfe_tap9():(rd_rxd_dfe_tap13_mux()==2)?rd_rxd_dfe_tap13():0));
    ESTM(lane_st->dfe[21][0] = ((rd_rxa_dfe_tap10_mux()==3)?rd_rxa_dfe_tap10():(rd_rxa_dfe_tap14_mux()==2)?rd_rxa_dfe_tap14():0));
    ESTM(lane_st->dfe[21][1] = ((rd_rxb_dfe_tap10_mux()==3)?rd_rxb_dfe_tap10():(rd_rxb_dfe_tap14_mux()==2)?rd_rxb_dfe_tap14():0));
    ESTM(lane_st->dfe[21][2] = ((rd_rxc_dfe_tap10_mux()==3)?rd_rxc_dfe_tap10():(rd_rxc_dfe_tap14_mux()==2)?rd_rxc_dfe_tap14():0));
    ESTM(lane_st->dfe[21][3] = ((rd_rxd_dfe_tap10_mux()==3)?rd_rxd_dfe_tap10():(rd_rxd_dfe_tap14_mux()==2)?rd_rxd_dfe_tap14():0));
    ESTM(lane_st->dfe[22][0] = ((rd_rxa_dfe_tap11_mux()==3)?rd_rxa_dfe_tap11():0));
    ESTM(lane_st->dfe[22][1] = ((rd_rxb_dfe_tap11_mux()==3)?rd_rxb_dfe_tap11():0));
    ESTM(lane_st->dfe[22][2] = ((rd_rxc_dfe_tap11_mux()==3)?rd_rxc_dfe_tap11():0));
    ESTM(lane_st->dfe[22][3] = ((rd_rxd_dfe_tap11_mux()==3)?rd_rxd_dfe_tap11():0));
    ESTM(lane_st->dfe[23][0] = ((rd_rxa_dfe_tap12_mux()==3)?rd_rxa_dfe_tap12():0));
    ESTM(lane_st->dfe[23][1] = ((rd_rxb_dfe_tap12_mux()==3)?rd_rxb_dfe_tap12():0));
    ESTM(lane_st->dfe[23][2] = ((rd_rxc_dfe_tap12_mux()==3)?rd_rxc_dfe_tap12():0));
    ESTM(lane_st->dfe[23][3] = ((rd_rxd_dfe_tap12_mux()==3)?rd_rxd_dfe_tap12():0));
    ESTM(lane_st->dfe[24][0] = ((rd_rxa_dfe_tap13_mux()==3)?rd_rxa_dfe_tap13():0));
    ESTM(lane_st->dfe[24][1] = ((rd_rxb_dfe_tap13_mux()==3)?rd_rxb_dfe_tap13():0));
    ESTM(lane_st->dfe[24][2] = ((rd_rxc_dfe_tap13_mux()==3)?rd_rxc_dfe_tap13():0));
    ESTM(lane_st->dfe[24][3] = ((rd_rxd_dfe_tap13_mux()==3)?rd_rxd_dfe_tap13():0));
    ESTM(lane_st->dfe[25][0] = ((rd_rxa_dfe_tap14_mux()==3)?rd_rxa_dfe_tap14():0));
    ESTM(lane_st->dfe[25][1] = ((rd_rxb_dfe_tap14_mux()==3)?rd_rxb_dfe_tap14():0));
    ESTM(lane_st->dfe[25][2] = ((rd_rxc_dfe_tap14_mux()==3)?rd_rxc_dfe_tap14():0));
    ESTM(lane_st->dfe[25][3] = ((rd_rxd_dfe_tap14_mux()==3)?rd_rxd_dfe_tap14():0));
    ESTM(lane_st->thctrl_dp[0] = falcon16_tsc_INTERNAL_afe_slicer_offset_mapping(rd_rxa_slicer_offset_adj_dp()));
    ESTM(lane_st->thctrl_dp[1] = falcon16_tsc_INTERNAL_afe_slicer_offset_mapping(rd_rxb_slicer_offset_adj_dp()));
    ESTM(lane_st->thctrl_dp[2] = falcon16_tsc_INTERNAL_afe_slicer_offset_mapping(rd_rxc_slicer_offset_adj_dp()));
    ESTM(lane_st->thctrl_dp[3] = falcon16_tsc_INTERNAL_afe_slicer_offset_mapping(rd_rxd_slicer_offset_adj_dp()));
    ESTM(lane_st->thctrl_dn[0] = falcon16_tsc_INTERNAL_afe_slicer_offset_mapping(rd_rxa_slicer_offset_adj_dn()));
    ESTM(lane_st->thctrl_dn[1] = falcon16_tsc_INTERNAL_afe_slicer_offset_mapping(rd_rxb_slicer_offset_adj_dn()));
    ESTM(lane_st->thctrl_dn[2] = falcon16_tsc_INTERNAL_afe_slicer_offset_mapping(rd_rxc_slicer_offset_adj_dn()));
    ESTM(lane_st->thctrl_dn[3] = falcon16_tsc_INTERNAL_afe_slicer_offset_mapping(rd_rxd_slicer_offset_adj_dn()));
    ESTM(lane_st->thctrl_zp[0] = falcon16_tsc_INTERNAL_afe_slicer_offset_mapping(rd_rxa_slicer_offset_adj_zp()));
    ESTM(lane_st->thctrl_zp[1] = falcon16_tsc_INTERNAL_afe_slicer_offset_mapping(rd_rxb_slicer_offset_adj_zp()));
    ESTM(lane_st->thctrl_zp[2] = falcon16_tsc_INTERNAL_afe_slicer_offset_mapping(rd_rxc_slicer_offset_adj_zp()));
    ESTM(lane_st->thctrl_zp[3] = falcon16_tsc_INTERNAL_afe_slicer_offset_mapping(rd_rxd_slicer_offset_adj_zp()));
    ESTM(lane_st->thctrl_zn[0] = falcon16_tsc_INTERNAL_afe_slicer_offset_mapping(rd_rxa_slicer_offset_adj_zn()));
    ESTM(lane_st->thctrl_zn[1] = falcon16_tsc_INTERNAL_afe_slicer_offset_mapping(rd_rxb_slicer_offset_adj_zn()));
    ESTM(lane_st->thctrl_zn[2] = falcon16_tsc_INTERNAL_afe_slicer_offset_mapping(rd_rxc_slicer_offset_adj_zn()));
    ESTM(lane_st->thctrl_zn[3] = falcon16_tsc_INTERNAL_afe_slicer_offset_mapping(rd_rxd_slicer_offset_adj_zn()));
    ESTM(lane_st->thctrl_l[0] = falcon16_tsc_INTERNAL_afe_slicer_offset_mapping(rd_rxa_slicer_offset_adj_lms()));
    ESTM(lane_st->thctrl_l[1] = falcon16_tsc_INTERNAL_afe_slicer_offset_mapping(rd_rxb_slicer_offset_adj_lms()));
    ESTM(lane_st->thctrl_l[2] = falcon16_tsc_INTERNAL_afe_slicer_offset_mapping(rd_rxc_slicer_offset_adj_lms()));
    ESTM(lane_st->thctrl_l[3] = falcon16_tsc_INTERNAL_afe_slicer_offset_mapping(rd_rxd_slicer_offset_adj_lms()));
    ESTM(lane_st->heye_left = falcon16_tsc_INTERNAL_eye_to_mUI(sa__, rdv_usr_sts_heye_left()));
    ESTM(lane_st->heye_right = falcon16_tsc_INTERNAL_eye_to_mUI(sa__, rdv_usr_sts_heye_right()));
    ESTM(lane_st->veye_upper = falcon16_tsc_INTERNAL_eye_to_mV(sa__, rdv_usr_sts_veye_upper(), 0));
    ESTM(lane_st->veye_lower = falcon16_tsc_INTERNAL_eye_to_mV(sa__, rdv_usr_sts_veye_lower(), 0));
    ESTM(lane_st->link_time = (((uint32_t)rdv_usr_sts_link_time())*8)/10);

    if (lane_st->pmd_lock == 1) {
      if (!lane_st->stop_state) {
        EFUN(falcon16_tsc_stop_rx_adaptation(sa__, 0));
      }
    } else {
        EFUN(falcon16_tsc_stop_rx_adaptation(sa__, 0));
    }

    return(ERR_CODE_NONE);
}


err_code_t falcon16_tsc_log_full_pmd_state (srds_access_t *sa__, struct falcon16_tsc_detailed_lane_status_st *lane_st) {
    uint16_t reg_data;

    if(!lane_st) 
      return(_error(ERR_CODE_BAD_PTR_OR_INVALID_INPUT));

    falcon16_tsc_log_full_pmd_state_noPRBS(sa__, lane_st);

    ESTM(lane_st->prbs_chk_en = rd_prbs_chk_en());
    ESTM(reg_data = rd_prbs_chk_mode_sel());
    lane_st->prbs_chk_order = (reg_data==0)?7:
                              (reg_data==1)?9:
                              (reg_data==2)?11:
                              (reg_data==3)?15:
                              (reg_data==4)?23:
                              (reg_data==5)?31:
                              (reg_data==6)?58:0;
    EFUN(falcon16_tsc_prbs_chk_lock_state(sa__, &lane_st->prbs_chk_lock));
    EFUN(falcon16_tsc_prbs_err_count_ll(sa__, &lane_st->prbs_chk_errcnt));

    return(ERR_CODE_NONE);
}

err_code_t falcon16_tsc_disp_full_pmd_state (srds_access_t *sa__, struct falcon16_tsc_detailed_lane_status_st *lane_st, uint8_t num_lanes) {
    uint8_t i;

    EFUN_PRINTF(( "\n\n"));
    EFUN_PRINTF(( "------------------------------------------------------------------------\n"));
    EFUN_PRINTF(( "Falcon16 PMD State\n"));
    EFUN_PRINTF(( "%-16s\t%12s%12s%12s%12s\n", "PARAMETER","LN0","LN1","LN2","LN3"));
    EFUN_PRINTF(( "------------------------------------------------------------------------\n"));
    DISP_LN_VARS ("HEYE Left (mUI)",heye_left,"%12d");
    DISP_LN_VARS ("HEYE Rght (mUI)",heye_right,"%12d");
    DISP_LN_VARS ("VEYE Top  (mV)",veye_upper,"%12d");
    DISP_LN_VARS ("VEYE Bott (mV)",veye_lower,"%12d");
#ifdef SERDES_API_FLOATING_POINT
    DISP_LN_VARS ("Link Time (ms)",link_time/10.0,"%12.1f");
#else
    DISP_LN_VARS ("Link Time (ms)",link_time/10,"%12d");
#endif
    DISP_LN_VARS ("Ln Reset State",reset_state,"%12d");
    DISP_LN_VARS ("uC Stop",stop_state,"%12d");

    EFUN_PRINTF(( "------------------------------------------------------------------------\n"));
    DISP_LN_VARS ("TX drv_hv_disable",ams_tx_drv_hv_disable,"%12d");
    DISP_LN_VARS ("TX ana_rescal",ams_tx_ana_rescal,"%12d");
    DISP_LN_VARS ("TX pre_tap",pre_tap,"%12d");
    DISP_LN_VARS ("TX main_tap",main_tap,"%12d");
    DISP_LN_VARS ("TX post1_tap",post1_tap,"%12d");
    DISP_LN_VARS ("TX post2_tap",post2_tap,"%12d");
    DISP_LN_VARS ("TX post3_tap",post3_tap,"%12d");
    DISP_LN_VARS ("TX rpara",rpara,"%12d");
    EFUN_PRINTF(( "------------------------------------------------------------------------\n"));
    DISP_LN_VARS ("Sigdet",sigdet,"%12d");
    DISP_LN_VARS ("PMD_lock",pmd_lock,"%12d");
    DISP_LN_VARS ("DSC_SM (1st read)",dsc_sm[0],"       %4xh");
    DISP_LN_VARS ("DSC_SM (2nd read)",dsc_sm[1],"       %4xh");
    EFUN_PRINTF(( "------------------------------------------------------------------------\n"));
#ifdef SERDES_API_FLOATING_POINT
    DISP_LN_VARS ("PPM",ppm,"%12.2f");
#else
    DISP_LN_VARS ("PPM",ppm,"%12d");
#endif
    DISP_LN_VARS ("VGA",vga,"%12d");
    DISP_LN_VARS ("PF",pf,"%12d");
    DISP_LN_VARS ("PF2",pf2,"%12d");
#ifdef SERDES_API_FLOATING_POINT
    DISP_LN_VARS ("main_tap",main_tap_est,"%12.2f");
#else
    DISP_LN_VARS ("main_tap",main_tap_est,"%12d");
#endif
    DISP_LN_VARS ("data_thresh",data_thresh,"%12d");
    DISP_LN_VARS ("phase_thresh",phase_thresh,"%12d");
    DISP_LN_VARS ("lms_thresh",lms_thresh,"%12d");
    EFUN_PRINTF(( "------------------------------------------------------------------------\n"));
    DISP_LN_VARS ("d/dq hoffset",ddq_hoffset,"%12u");
    DISP_LN_VARS ("p/pq hoffset",ppq_hoffset,"%12u");
    DISP_LN_VARS ("l/lq hoffset",llq_hoffset,"%12u");
    DISP_LN_VARS ("d/p hoffset",dp_hoffset,"%12u");
    DISP_LN_VARS ("d/l hoffset",dl_hoffset,"%12u");
    EFUN_PRINTF(( "------------------------------------------------------------------------\n"));
    DISP_LNQ_VARS("dfe[2][a,b]",dfe[1][0],dfe[1][1],"%8d,%3d");
    DISP_LNQ_VARS("dfe[2][c,d]",dfe[1][2],dfe[1][3],"%8d,%3d");
    DISP_LNQ_VARS("dfe[3][a,b]",dfe[2][0],dfe[2][1],"%8d,%3d");
    DISP_LNQ_VARS("dfe[3][c,d]",dfe[2][2],dfe[2][3],"%8d,%3d");
    DISP_LNQ_VARS("dfe[4][a,b]",dfe[3][0],dfe[3][1],"%8d,%3d");
    DISP_LNQ_VARS("dfe[4][c,d]",dfe[3][2],dfe[3][3],"%8d,%3d");
    DISP_LNQ_VARS("dfe[5][a,b]",dfe[4][0],dfe[4][1],"%8d,%3d");
    DISP_LNQ_VARS("dfe[5][c,d]",dfe[4][2],dfe[4][3],"%8d,%3d");
    DISP_LNQ_VARS("dfe[6][a,b]",dfe[5][0],dfe[5][1],"%8d,%3d");
    DISP_LNQ_VARS("dfe[6][c,d]",dfe[5][2],dfe[5][3],"%8d,%3d");
    EFUN_PRINTF(( "------------------------------------------------------------------------\n"));
    DISP_LNQ_VARS("dfe[7][a,b]",dfe[6][0],dfe[6][1],"%8d,%3d");
    DISP_LNQ_VARS("dfe[7][c,d]",dfe[6][2],dfe[6][3],"%8d,%3d");
    DISP_LNQ_VARS("dfe[8][a,b]",dfe[7][0],dfe[7][1],"%8d,%3d");
    DISP_LNQ_VARS("dfe[8][c,d]",dfe[7][2],dfe[7][3],"%8d,%3d");
    DISP_LNQ_VARS("dfe[9][a,b]",dfe[8][0],dfe[8][1],"%8d,%3d");
    DISP_LNQ_VARS("dfe[9][c,d]",dfe[8][2],dfe[8][3],"%8d,%3d");
    DISP_LNQ_VARS("dfe[10][a,b]",dfe[9][0],dfe[9][1],"%8d,%3d");
    DISP_LNQ_VARS("dfe[10][c,d]",dfe[9][2],dfe[9][3],"%8d,%3d");
    DISP_LNQ_VARS("dfe[11][a,b]",dfe[10][0],dfe[10][1],"%8d,%3d");
    DISP_LNQ_VARS("dfe[11][c,d]",dfe[10][2],dfe[10][3],"%8d,%3d");
    DISP_LNQ_VARS("dfe[12][a,b]",dfe[11][0],dfe[11][1],"%8d,%3d");
    DISP_LNQ_VARS("dfe[12][c,d]",dfe[11][2],dfe[11][3],"%8d,%3d");
    DISP_LNQ_VARS("dfe[13][a,b]",dfe[12][0],dfe[12][1],"%8d,%3d");
    DISP_LNQ_VARS("dfe[13][c,d]",dfe[12][2],dfe[12][3],"%8d,%3d");
    DISP_LNQ_VARS("dfe[14][a,b]",dfe[13][0],dfe[13][1],"%8d,%3d");
    DISP_LNQ_VARS("dfe[14][c,d]",dfe[13][2],dfe[13][3],"%8d,%3d");
    DISP_LNQ_VARS("dfe[15][a,b]",dfe[14][0],dfe[14][1],"%8d,%3d");
    DISP_LNQ_VARS("dfe[15][c,d]",dfe[14][2],dfe[14][3],"%8d,%3d");
    DISP_LNQ_VARS("dfe[16][a,b]",dfe[15][0],dfe[15][1],"%8d,%3d");
    DISP_LNQ_VARS("dfe[16][c,d]",dfe[15][2],dfe[15][3],"%8d,%3d");
    DISP_LNQ_VARS("dfe[17][a,b]",dfe[16][0],dfe[16][1],"%8d,%3d");
    DISP_LNQ_VARS("dfe[17][c,d]",dfe[16][2],dfe[16][3],"%8d,%3d");
    DISP_LNQ_VARS("dfe[18][a,b]",dfe[17][0],dfe[17][1],"%8d,%3d");
    DISP_LNQ_VARS("dfe[18][c,d]",dfe[17][2],dfe[17][3],"%8d,%3d");
    DISP_LNQ_VARS("dfe[19][a,b]",dfe[18][0],dfe[18][1],"%8d,%3d");
    DISP_LNQ_VARS("dfe[19][c,d]",dfe[18][2],dfe[18][3],"%8d,%3d");
    DISP_LNQ_VARS("dfe[20][a,b]",dfe[19][0],dfe[19][1],"%8d,%3d");
    DISP_LNQ_VARS("dfe[20][c,d]",dfe[19][2],dfe[19][3],"%8d,%3d");
    DISP_LNQ_VARS("dfe[21][a,b]",dfe[20][0],dfe[20][1],"%8d,%3d");
    DISP_LNQ_VARS("dfe[21][c,d]",dfe[20][2],dfe[20][3],"%8d,%3d");
    DISP_LNQ_VARS("dfe[22][a,b]",dfe[21][0],dfe[21][1],"%8d,%3d");
    DISP_LNQ_VARS("dfe[22][c,d]",dfe[21][2],dfe[21][3],"%8d,%3d");
    DISP_LNQ_VARS("dfe[23][a,b]",dfe[22][0],dfe[22][1],"%8d,%3d");
    DISP_LNQ_VARS("dfe[23][c,d]",dfe[22][2],dfe[22][3],"%8d,%3d");
    DISP_LNQ_VARS("dfe[24][a,b]",dfe[23][0],dfe[23][1],"%8d,%3d");
    DISP_LNQ_VARS("dfe[24][c,d]",dfe[23][2],dfe[23][3],"%8d,%3d");
    DISP_LNQ_VARS("dfe[25][a,b]",dfe[24][0],dfe[24][1],"%8d,%3d");
    DISP_LNQ_VARS("dfe[25][c,d]",dfe[24][2],dfe[24][3],"%8d,%3d");
    DISP_LNQ_VARS("dfe[26][a,b]",dfe[25][0],dfe[25][1],"%8d,%3d");
    DISP_LNQ_VARS("dfe[26][c,d]",dfe[25][2],dfe[25][3],"%8d,%3d");
    EFUN_PRINTF(( "------------------------------------------------------------------------\n"));
    DISP_LN_VARS ("dc_offset",dc_offset,"%12d");
    DISP_LNQ_VARS("data_p[a,b]",thctrl_dp[0],thctrl_dp[1],"%8d,%3d");
    DISP_LNQ_VARS("data_p[c,d]",thctrl_dp[2],thctrl_dp[3],"%8d,%3d");
    DISP_LNQ_VARS("data_n[a,b]",thctrl_dn[0],thctrl_dn[1],"%8d,%3d");
    DISP_LNQ_VARS("data_n[c,d]",thctrl_dn[2],thctrl_dn[3],"%8d,%3d");
    DISP_LNQ_VARS("phase_p[a,b]",thctrl_zp[0],thctrl_zp[1],"%8d,%3d");
    DISP_LNQ_VARS("phase_p[c,d]",thctrl_zp[2],thctrl_zp[3],"%8d,%3d");
    DISP_LNQ_VARS("phase_n[a,b]",thctrl_zn[0],thctrl_zn[1],"%8d,%3d");
    DISP_LNQ_VARS("phase_n[c,d]",thctrl_zn[2],thctrl_zn[3],"%8d,%3d");
    DISP_LNQ_VARS("lms[a,b]",thctrl_l[0],thctrl_l[1],"%8d,%3d");
    DISP_LNQ_VARS("lms[c,d]",thctrl_l[2],thctrl_l[3],"%8d,%3d");
    EFUN_PRINTF(( "------------------------------------------------------------------------\n"));
    DISP_LN_VARS ("PRBS_CHECKER",prbs_chk_en,"%12u");
    DISP_LN_VARS ("PRBS_ORDER",prbs_chk_order,"%12u");
    DISP_LN_VARS ("PRBS_LOCK",prbs_chk_lock,"%12u");
    DISP_LN_VARS ("PRBS_ERRCNT",prbs_chk_errcnt,"%12u");
    EFUN_PRINTF(( "------------------------------------------------------------------------\n"));
    EFUN_PRINTF(( "\n"));

    return (ERR_CODE_NONE);
}

