/* $Id: arad_pp_oam.h,v 1.27 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/

#ifndef __JER_PP_OAM_INCLUDED__
/* { */
#define __JER_PP_OAM_INCLUDED__



#include <soc/dpp/SAND/Utils/sand_header.h>

#include <soc/dpp/SAND/Management/sand_general_macros.h>
#include <soc/dpp/SAND/Management/sand_error_code.h>
#include <soc/dpp/PPD/ppd_api_oam.h>
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_oam.h>

/* } */
/*************
 * DEFINES   *
 *************/
/* { */



/* use maximal mirror, forward strength*/
#define _JER_PP_OAM_MIRROR_STRENGTH 3
#define _JER_PP_OAM_FORWARD_STRENGTH 3

/* } */
/*************
 * MACROS    *
 *************/
/* { */
#define JER_PP_DO_NOTHING_AND_EXIT                                         \
          SOC_SAND_IGNORE_UNUSED_VAR(res);                                    \
          goto exit


/* MDL-MP-TYPE encoding*/
#define _JER_PP_OAM_MDL_MP_TYPE_NO_MP 0
#define _JER_PP_OAM_MDL_MP_TYPE_MIP 1
#define _JER_PP_OAM_MDL_MP_TYPE_ACTIVE_MATCH 2
#define _JER_PP_OAM_MDL_MP_TYPE_PASSIVE_MATCH 3


/**
 * mp-type-vector is composed of 8 pairs of bits, one pair per
 * level. Each pair of bits represent the mp-type. Getter and
 * setter macros. Returns one of the defines above.
 */
#define JERICHO_PP_OAM_EXTRACT_MDL_MP_TYPE_FROM_MP_TYPE_VECTOR_BY_LEVEL(mp_type_vector,level) (((mp_type_vector) >> ((level)*2)) & 0x3)

#define JERICHO_PP_OAM_SET_MDL_MP_TYPE_VECTOR_BY_LEVEL(mp_type_vector,mp_type,level)\
do {\
    uint32 temp_mp_type_vector = mp_type_vector, mp_type_temp=mp_type;\
    SHR_BITCOPY_RANGE(&temp_mp_type_vector, (level)*2 ,&mp_type_temp,0,2);\
    mp_type_vector = temp_mp_type_vector;\
}\
while (0)


/**
 *  Mapping is as following:
 *  MIP (1) --> MIP-match (0) ,
 *  active-match(2) --> active_match(1)
 *  passive-match(3)--> passive-match(2) */
#define _JER_PP_OAM_MP_TYPE_FROM_MDL_MP_TYPE(mdl_mp_type) (mdl_mp_type -1)

#define JER_PP_OAM_LOOPBACK_TST_INFO_FLAGS_LBM 0x1
#define JER_PP_OAM_LOOPBACK_TST_INFO_FLAGS_TST 0x2

/* } */
/*************
 * TYPE DEFS *
 *************/
/* { */

typedef struct {
    uint32                    endpoint_id;          /* Entry number of the endpoint. */
    uint8                     flags;                /* indicate lb or tst, lb: 0x1 (default value) , tst:0x2*/
    SOC_SAND_PP_MAC_ADDRESS   mac_address;          /* DA MAC address of outgoing LBMs */
    uint8                     pkt_pri;              /* Optional priority value to be set on Ethernet outer/sole VLAN tag PDP and DEI fields */
    uint8                     inner_pkt_pri;        /* Optional priority value to be set on Ethernet inner VLAN tag PDP and DEI fields */
    bcm_cos_t                 int_pri;              /* Optional priority fields on ITMH header */
} JER_PP_OAM_LOOPBACK_TST_INFO;

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


/**
 * Jerihco specific OAM initialization.
 * 
 * @author sinai (30/06/2014)
 * 
 * @param unit 
 * 
 * @return soc_error_t 
 */

/* Jericho specific classifier initializations */
soc_error_t soc_jer_pp_oam_classifier_init(int unit);

/* Jericho specific OAMP initializations */
soc_error_t soc_jer_pp_oam_oamp_init(int unit);

soc_error_t
  soc_jer_pp_oamp_control_sat_weight_get(
    SOC_SAND_IN  int                   unit,
    SOC_SAND_OUT uint32                *sat_weight
  );

/** 
 *   In Jericho, user allocates 2 counters per endpoint (LIF) - 
 *    one for the ingress and one for the egress. Egress gets
 *    counter given by the API (lm_counter_base_id, this is
 *    assumed to be even) ingress gets counter + 1
 *  
 *    However if LM PCP is used then 8 counters are needed per
 *    direction. The assumption in this case is that the counter
 *    from the API is a multiple of 16.
 *    So egress side gets counter given by API and ingress side
 *    gets counter + 8.
 *  
 * @author sinai (06/07/2016)
 * 
 * @param classifier_mep_entry 
 * @param is_active 
 * 
 * @return int - value needed to be added to counter
 */
int soc_jer_pp_oam_counter_pointer_addition(int unit, const SOC_PPC_OAM_CLASSIFIER_MEP_ENTRY * classifier_mep_entry, int is_active);


/**
 * Sets the new OEM1-payload according to the previous payload and new MEP.
 * 
 * @author sinai (29/06/2014)
 * 
 * @param unit 
 * @param classifier_mep_entry 
 * @param prev_payload 
 * @param new_payload 
 * @param is_active 
 * @param update 
 * 
 * @return soc_error_t 
 */
soc_error_t soc_jer_pp_oam_oem1_mep_add(
        int unit,
        const SOC_PPC_OAM_CLASSIFIER_MEP_ENTRY * classifier_mep_entry,
        const SOC_PPC_OAM_CLASSIFIER_OEM1_ENTRY_KEY * oem1_key,
        const SOC_PPC_OAM_CLASSIFIER_OEM1_ENTRY_PAYLOAD * prev_payload,
        SOC_PPC_OAM_CLASSIFIER_OEM1_ENTRY_PAYLOAD * new_payload,
        uint8 is_active,
        uint8 update);

/**
 * Delete OEM1-payload
 * MEP.
 *
 * @author avive (25/04/2016)
 *
 * @param unit
 * @param classifier_mep_entry
 * @param oem1_key
 * @param new_mp_type_vector
 *
 * @return soc_error_t
 */
soc_error_t soc_jer_pp_oam_oem1_mep_delete(
        int unit,
        const SOC_PPC_OAM_CLASSIFIER_MEP_ENTRY * classifier_mep_entry,
        const SOC_PPC_OAM_CLASSIFIER_OEM1_ENTRY_KEY * oem1_key,
        uint32 * new_mp_type_vector);

/**
 * Adds a default profile to the OAM classifier
 *
 * @param unit            - (IN) Device to be configured.
 * @param mep_index       - (IN) profile mep id.
 * @param classifier_mep_entry - (IN) Classifier entry
 *                             information
 * @param update_action_only - (IN) is it an update for the
 * action
 *
 * @return soc_error_t
 */
soc_error_t
  soc_jer_pp_oam_classifier_default_profile_add(
     SOC_SAND_IN  int                                unit,
     SOC_SAND_IN  ARAD_PP_OAM_DEFAULT_EP_ID          mep_index,
     SOC_SAND_IN  SOC_PPC_OAM_CLASSIFIER_MEP_ENTRY   *classifier_mep_entry,
     SOC_SAND_IN  uint8                              update_action_only
  );

/**
 * Removes a default profile from the OAM classifier
 *
 * @param unit (IN) Device to be configured.
 * @param mep_index profile mep id.
 *
 * @return soc_error_t
 */
soc_error_t
  soc_jer_pp_oam_classifier_default_profile_remove(
    SOC_SAND_IN  int                                unit,
    SOC_SAND_IN  ARAD_PP_OAM_DEFAULT_EP_ID          mep_index
  );


/**
 * Sets an action for a default OAM endpoint
 *
 * @param unit Device to be configured.
 * @param mep_index profile mep id.
 * @param classifier_mep_entry Classifier entry information
 *
 * @return soc_error_t
 */
soc_error_t
  soc_jer_pp_oam_classifier_default_profile_action_set(
    SOC_SAND_IN  int                                unit,
    SOC_SAND_IN  ARAD_PP_OAM_DEFAULT_EP_ID          mep_index,
    SOC_SAND_IN  SOC_PPC_OAM_CLASSIFIER_MEP_ENTRY   *classifier_mep_entry
  );

/**
 * soc_jer_pp_oam_set_inlif_profile_map
 * Adds a mapping from an inlif profile (4b) to an OAM lif profile (2b)
 */
soc_error_t
  soc_jer_pp_oam_inlif_profile_map_set(
     SOC_SAND_IN  int                                                  unit,
     SOC_SAND_IN  uint32                                               inlif_profile,
     SOC_SAND_IN  uint32                                               oam_profile
  );

soc_error_t
  soc_jer_pp_oam_inlif_profile_map_get(
     SOC_SAND_IN  int                                                  unit,
     SOC_SAND_IN  uint32                                               inlif_profile,
     SOC_SAND_OUT uint32                                               *oam_profile
  );

/**
 * Read/Write on the MEP-Profile table.
 * 
 * @author sinai (30/06/2014)
 * 
 * @param unit 
 * @param profile_indx - must be between 0 and 128.
 * @param eth1731_profile 
 * 
 * @return soc_error_t 
 */
soc_error_t soc_jer_pp_oam_oamp_eth1731_profile_set(
    int                                 unit,
    uint8                          profile_indx,
    const SOC_PPC_OAM_ETH1731_MEP_PROFILE_ENTRY     *eth1731_profile
  );

soc_error_t soc_jer_pp_oam_oamp_eth1731_profile_get(
    int                                 unit,
    uint8                          profile_indx,
    SOC_PPC_OAM_ETH1731_MEP_PROFILE_ENTRY     *eth1731_profile
  );

/**
 * Initialize mp-type mapping
 * 
 * @author avive (07/06/2016)
 * 
 * @param unit 
 * 
 * @return soc_error_t 
 */
soc_error_t soc_jer_pp_oam_mp_type_config_init(int unit);

/**
 * Initialize the TOD configurations at the ECI. Used for enabling NTP, 1588 timestamping.
 * 
 * @author sinai (30/06/2014)
 * 
 * @param unit
 * @param init_ntp - initialize the NTP block
 * @param init_1588 - initialize the IEEE_1588 block
 * 
 * @return soc_error_t 
 */
soc_error_t soc_jer_pp_oam_init_eci_tod(
    int                                 unit,
   uint8                                init_ntp,
   uint8                                init_1588
   );



/**
 * Set the TOD configurations at the ECI. Used for setting NTP, 1588 timestamping.
 *
 * @author rmantelm (16/07/2015)
 *
 * @param unit
 * @param init_ntp - initialize the NTP block
 * @param data - values to be set: LO 32 bits - time_frac, Hi 32 bits - time_sec
 *
 * @return uint32
 */
uint32 soc_jer_pp_oam_tod_set(
    int                                 unit,
   uint8                                is_ntp,
   uint64                               data
   );



/**
 * Set MSBs of global SA address of outgoing PDUs.
 * 
 * @author sinai (26/06/2014)
 * 
 * @param unit 
 * @param profile - 0 or 1 
 * @param msbs - Assumed to be an array of 5 bytes
 * 
 * @return soc_error_t 
 */
soc_error_t soc_jer_pp_oam_sa_addr_msbs_set(
    int unit,
    int profile,
   const uint8 * msbs);


/**
 * Get MSBs of global SA address of outgoing PDUs.
 * 
 * @author sinai (26/06/2014)
 * 
 * @param unit 
 * @param profile - 0 or 1 
 * @param msbs - Assumed to be an array of 5 bytes
 * 
 * @return soc_error_t 
 */
soc_error_t soc_jer_pp_oam_sa_addr_msbs_get(
    int unit,
    int profile,
    uint8 * msbs);



/**
 * Trigger a one shot DMM, per endpoint. 
 * 
 * @author sinai (08/07/2014)
 * 
 * @param unit 
 * @param endpoint_id 
 * 
 * @return soc_error_t 
 */
soc_error_t soc_jer_pp_oam_dm_trigger_set(
   int unit,
   int endpoint_id);

/**
 * Adds all required static entries to OAM1. 
 * 
 * @author avive (09/06/2016)
 * 
 * @param unit 
 * 
 * @return soc_error_t 
 */
soc_error_t soc_jer_pp_oam_classifier_oam1_entries_add(
   int unit);

/**
 * Set the table EPNI_CFG_MAPPING_TO_OAM_PCP. 
 *  Determines the mapping of
 *  outlif profile + packet TC ---> OAM PCP.
 *  
 * 
 * @author sinai (14/10/2014)
 * 
 * @param unit 
 * @param tc 
 * @param outlif_profile 
 * @param oam_pcp 
 * 
 * @return soc_error_t 
 */
soc_error_t soc_jer_pp_oam_egress_pcp_set_by_profile_and_tc(
   int unit,
   uint8 tc,
   uint8 outlif_profile,
   uint8 oam_pcp);

/* Init sat on appl */
soc_error_t soc_jer_pp_oam_sat_init(int unit);

soc_error_t
    soc_jer_pp_oam_loopback_tst_info_init(SOC_SAND_IN int unit,
       SOC_SAND_INOUT JER_PP_OAM_LOOPBACK_TST_INFO *lb_tst_info);

/* config tst/lb sat header*/  
 soc_error_t 
 soc_jer_pp_oam_oamp_lb_tst_header_set (SOC_SAND_IN int unit, 
      SOC_SAND_IN JER_PP_OAM_LOOPBACK_TST_INFO *lb_tst_info,
      SOC_SAND_INOUT uint8* header_buffer, 
      SOC_SAND_OUT int *header_offset);


/* OAM initialization required to be done at init sequence*/
soc_error_t soc_jer_pp_oam_init_from_init_sequence(int unit);

soc_error_t soc_arad_oamp_cpu_port_dp_tc_set(int unit,uint32 dp,uint32 tc);
soc_error_t soc_arad_oamp_tmx_arb_weight_set(int unit,uint32 txm_arbiter_weight);
soc_error_t soc_jer_oamp_sat_arb_weight_set(int unit,uint32 sat_arbiter_weight);
soc_error_t soc_arad_oamp_response_weight_set(int unit,uint32 rsp_arbiter_weight);

#include <soc/dpp/SAND/Utils/sand_footer.h>

/* } __ARAD_PP_OAM_INCLUDED__*/
#endif
