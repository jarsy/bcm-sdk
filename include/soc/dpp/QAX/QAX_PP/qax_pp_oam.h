/*
 * $Id: $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File: qax_pp_oam.h
 */

#ifndef __QAX_PP_OAM_INCLUDED__
#define __QAX_PP_OAM_INCLUDED__

/*************
 * INCLUDES  *
 *************/
/* { */

#include <soc/dpp/JER/JER_PP/jer_pp_oam.h>
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_oam.h>

/* } */
/*************
 * DEFINES   *
 *************/
/* { */

#define _QAX_PP_HLM_BY_MDL(unit) (SOC_DPP_CONFIG(unit)->pp.oam_hierarchical_lm)

#define _QAX_PP_OAM_OEM1_KEY_SEL_OAM_LIF_INNER      0
#define _QAX_PP_OAM_OEM1_KEY_SEL_OAM_LIF_OUTER      1
#define _QAX_PP_OAM_OEM1_KEY_SEL_OAM_LIF            2
#define _QAX_PP_OAM_OEM1_KEY_SEL_NULL               3

#define _QAX_PP_OAM_HLM_EP_OPERATION_ADD      0
#define _QAX_PP_OAM_HLM_EP_OPERATION_UPDATE   1
#define _QAX_PP_OAM_HLM_EP_OPERATION_DELETE   2

#define _QAX_PP_OAM_HLM_ING_KEY_SELECT_REG_KEY_SIZE (1<<5)
#define _QAX_PP_OAM_HLM_EG_KEY_SELECT_REG_KEY_SIZE  (1<<5)

/* Banks 0-7 are active - contains actual MEP entries which the TX machine should process.
   Banks 8-11 contains only extra data which the TX machine should not process. */
#define _QAX_PP_OAM_OAMP_MEP_DB_NOF_BANKS          12
#define _QAX_PP_OAM_OAMP_MEP_DB_LAST_ACTIVE_BANK   7

/* } */
/*************
 * MACROS    *
 *************/
/* { */

/* Same conversion as in JER */
#define _QAX_PP_OAM_MP_TYPE_FROM_MDL_MP_TYPE(mdl_mp_type) _JER_PP_OAM_MP_TYPE_FROM_MDL_MP_TYPE(mdl_mp_type)

/* MDL-MP-TYPE encoding - Same as JER */
#define _QAX_PP_OAM_MDL_MP_TYPE_NO_MP               _JER_PP_OAM_MDL_MP_TYPE_NO_MP
#define _QAX_PP_OAM_MDL_MP_TYPE_MIP                 _JER_PP_OAM_MDL_MP_TYPE_MIP
#define _QAX_PP_OAM_MDL_MP_TYPE_ACTIVE_MATCH        _JER_PP_OAM_MDL_MP_TYPE_ACTIVE_MATCH
#define _QAX_PP_OAM_MDL_MP_TYPE_PASSIVE_MATCH       _JER_PP_OAM_MDL_MP_TYPE_PASSIVE_MATCH

/* OEM1 Key select payload by components */
#define _QAX_PP_OAM_OEM1_KEY_SEL_PAYLOAD(_key_sel_inner, _key_sel_outer, _mp_profile_sel, _your_disc_inner, _your_disc_outer) \
            (((_key_sel_inner)<<5)|((_key_sel_outer)<<3)|((_mp_profile_sel)<<2)|((_your_disc_inner)<<1)|(_your_disc_outer))

/* OEM1 Key select key components ({OAM-LIF-Outer-Valid, OAM-LIF-Inner-Valid, Lif-Equal-To-OAM-LIF-Outer, Lif-Equal-To-OAM-LIF-Inner, Your-Disc})*/
#define _QAX_PP_OAM_OEM1_KEY_SEL_INGRESS_KEY_OAM_LIF_OUTER_VALID(_key)      (((_key)&(1<<4))!=0)
#define _QAX_PP_OAM_OEM1_KEY_SEL_INGRESS_KEY_OAM_LIF_INNER_VALID(_key)      (((_key)&(1<<3))!=0)
#define _QAX_PP_OAM_OEM1_KEY_SEL_INGRESS_KEY_LIF_EQ_TO_OAM_LIF_OUTER(_key)  (((_key)&(1<<2))!=0)
#define _QAX_PP_OAM_OEM1_KEY_SEL_INGRESS_KEY_LIF_EQ_TO_OAM_LIF_INNER(_key)  (((_key)&(1<<1))!=0)
#define _QAX_PP_OAM_OEM1_KEY_SEL_INGRESS_KEY_YOUR_DISC(_key)                (((_key)&(1<<0))!=0)

/* OEM1 Key select key components ({OAM-LIF-Outer-Valid, OAM-LIF-Inner-Valid, Packet-Is-OAM, Counter-Pointer-Valid, OAM-Injection})*/
#define _QAX_PP_OAM_OEM1_KEY_SEL_EGRESS_KEY_OAM_LIF_OUTER_VALID(_key)       (((_key)&(1<<4))!=0)
#define _QAX_PP_OAM_OEM1_KEY_SEL_EGRESS_KEY_OAM_LIF_INNER_VALID(_key)       (((_key)&(1<<3))!=0)
#define _QAX_PP_OAM_OEM1_KEY_SEL_EGRESS_KEY_PACKET_IS_OAM(_key)             (((_key)&(1<<2))!=0)
#define _QAX_PP_OAM_OEM1_KEY_SEL_EGRESS_KEY_COUNTER_PTR_VALID(_key)         (((_key)&(1<<1))!=0)
#define _QAX_PP_OAM_OEM1_KEY_SEL_EGRESS_KEY_OAM_INJECTION(_key)             (((_key)&(1<<0))!=0)

/* Macros required for diag oam counters oamp set command */
#define RX            1
#define TX            2
#define PUNT_RESPONSE 3

#define ALL                1
#define MEP                2
#define RMEP               3
#define OPCODE             4
#define CCM                5
#define LMM                6
#define DMM                7
#define SLM                8
#define BFD                9
#define PUNT              10
#define RESPONSE          11
#define RESPONSE_MEP      12
#define RESPONSE_OPCODE   13

/* } */
/*************
 * TYPE DEFS *
 *************/
/* { */

/* } */
/*************
 * GLOBALS   *
 *************/
/* { */

/* } */
/*************
 * FUNCTIONS *
 *************/
/* { */

/* QAX specific classifier initializations */
soc_error_t soc_qax_pp_oam_classifier_init(int unit);

/* QAX specific OAMP initializations */
soc_error_t soc_qax_pp_oam_oamp_init(int unit);

/* Punt handling profiles for QAX */
uint32
  soc_qax_pp_oam_oamp_punt_event_hendling_profile_set(
    SOC_SAND_IN int                                    unit,
    SOC_SAND_IN uint32                                 profile_ndx,
    SOC_SAND_IN SOC_PPC_OAM_OAMP_PUNT_PROFILE_DATA    *punt_profile_data
  );

uint32
  soc_qax_pp_oam_oamp_punt_event_hendling_profile_get(
    SOC_SAND_IN  int                                    unit,
    SOC_SAND_IN  uint32                                 profile_ndx,
    SOC_SAND_OUT SOC_PPC_OAM_OAMP_PUNT_PROFILE_DATA    *punt_profile_data
  );


/**
 * Configure OAMP supported trap codes per endpoint type
 * 
 * @author sinai (24/12/2015)
 * 
 * @param unit 
 * @param mep_type Endpoint type that is associated with the given trap code
 * @param trap_code Trap code that will be recognized by the OAMP as valid trap code.
 * 
 * @return soc_error_t 
 */
soc_error_t soc_qax_pp_oam_oamp_rx_trap_codes_delete(
                 int                                 unit,
                 SOC_PPC_OAM_MEP_TYPE                mep_type,
                 uint32                              trap_code
    );

soc_error_t soc_qax_pp_oam_oamp_rx_trap_codes_set(
                 int                                 unit,
                 SOC_PPC_OAM_MEP_TYPE                mep_type,
                 uint32                              trap_code
    ) ;


soc_error_t
 soc_qax_pp_oam_oamp_sd_sf_profile_set(
    SOC_SAND_IN  int                                   unit,
    SOC_SAND_IN  uint32                                 profile_ndx,
    SOC_SAND_IN  SOC_PPC_OAM_OAMP_SD_SF_PROFILE_DB     *sd_sf_profile_data
  );

soc_error_t
 soc_qax_pp_oam_oamp_sd_sf_profile_get(
    SOC_SAND_IN  int                                    unit,
    SOC_SAND_IN  uint32                                 profile_ndx,
    SOC_SAND_OUT  SOC_PPC_OAM_OAMP_SD_SF_PROFILE_DB     *sd_sf_profile_data
  );

soc_error_t
 soc_qax_pp_oam_oamp_sd_sf_1711_config_set(
    SOC_SAND_IN  int                                   unit,
    SOC_SAND_IN  uint8                                 d_excess_thresh,
    SOC_SAND_IN  uint8                                 clr_low_thresh,
    SOC_SAND_IN  uint8                                 clr_high_thresh,
    SOC_SAND_IN  uint8                                 num_entry
  );

  soc_error_t
 soc_qax_pp_oam_oamp_sd_sf_1711_config_get(
    SOC_SAND_IN  int                                      unit,
    SOC_SAND_INOUT  uint8                                 *d_excess_thresh,
    SOC_SAND_INOUT  uint8                                 *clr_low_thresh,
    SOC_SAND_INOUT  uint8                                 *clr_high_thresh,
    SOC_SAND_INOUT  uint8                                 *num_entry
  );


soc_error_t
 soc_qax_pp_oam_oamp_sd_sf_scanner_set(
    SOC_SAND_IN  int                             unit,
    SOC_SAND_IN  uint8                           scan_index
  );

soc_error_t
  soc_qax_pp_oam_oamp_sd_sf_1711_db_set(
    SOC_SAND_IN  int                   unit,
    SOC_SAND_IN  uint32                rmep_index,
    SOC_SAND_IN  uint32                y1711_sd_sf_id,
    SOC_SAND_IN  uint32                sd_sf_1711_db_format,
    SOC_SAND_IN  uint8                 ccm_tx_rate,
    SOC_SAND_IN  uint8                 alert_method
  );

soc_error_t
  soc_qax_pp_oam_oamp_sd_sf_1711_db_get(
    SOC_SAND_IN  int                      unit,
    SOC_SAND_IN  uint32                   y1711_sd_sf_id,
    SOC_SAND_OUT  SOC_PPC_OAM_OAMP_SD_SF_Y_1711_DB_ENTRY  *sd_sf_1711_entry
  );

soc_error_t
  soc_qax_pp_oam_oamp_sd_sf_db_set(
    SOC_SAND_IN  int                      unit,
    SOC_SAND_IN  uint32                   rmep_index,
    SOC_SAND_IN  uint32                   sd_sf_db_index,
    SOC_PPC_OAM_OAMP_SD_SF_DB_ENTRY       *sd_sf_entry
  );

soc_error_t
  soc_qax_pp_oam_oamp_sd_sf_db_get(
    SOC_SAND_IN  int                   unit,
    SOC_SAND_IN  uint32                sd_sf_db_index,    
     SOC_PPC_OAM_OAMP_SD_SF_DB_ENTRY   *sd_sf_entry
  );

soc_error_t
  soc_qax_pp_oam_oamp_rmep_db_ext_get(
    SOC_SAND_IN  int                          unit,
    SOC_SAND_IN  uint32                       rmep_index,
    SOC_SAND_OUT  SOC_PPC_OAM_OAMP_RMEP_DB_EXT_ENTRY  *rmep_db_ext_entry
  );

soc_error_t
  soc_qax_pp_oam_slm_set(
    int unit,
    int is_slm
  );

soc_error_t
  soc_qax_pp_oam_classifier_oem1_entry_set_for_hlm(
    int unit,
    const SOC_PPC_OAM_CLASSIFIER_OEM1_ENTRY_KEY * oem1_key,
    const SOC_PPC_OAM_CLASSIFIER_OEM1_ENTRY_PAYLOAD * oem1_new_payload,
    const SOC_PPC_OAM_CLASSIFIER_MEP_ENTRY * classifier_mep_entry,
    uint8 operation /* 0:Add, 1:Update, 2:Delete */
  );

/* "Egress Inject" entries: Only relevant for Down Mep Ethernet OAM. (accelerate & active)*/
soc_error_t soc_qax_pp_oam_classifier_oam1_2_entries_insert_egress_inject( SOC_SAND_IN int unit,
                                                                           SOC_SAND_IN    SOC_PPC_OAM_LIF_PROFILE_DATA *profile_data,
                                                                           SOC_SAND_INOUT _oam_oam_a_b_table_buffer_t  *oama_buffer,
                                                                           SOC_SAND_INOUT _oam_oam_a_b_table_buffer_t  *oamb_buffer);

soc_error_t
soc_qax_diag_oamp_counter_set(int unit, uint8 type, uint8 filter, uint16 value);
soc_error_t
soc_qax_diag_oamp_counter_get(int unit, uint8 type, uint8 *filter, uint16 *value, uint32 *counter_val);

/* } */
#endif /* __QAX_PP_OAM_INCLUDED__ */
