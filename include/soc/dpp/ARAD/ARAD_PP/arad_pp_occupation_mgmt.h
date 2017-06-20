/* $Id: arad_pp_occupation_mgmt.h,v 1.28 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/
#ifndef __ARAD_PP_OCCUPATION_MGMT_INCLUDED__
/* { */
#define __ARAD_PP_OCCUPATION_MGMT_INCLUDED__

/*************
 * INCLUDES  *
 *************/
/* { */
#include <soc/dpp/SAND/Utils/sand_header.h>

#include <soc/dpp/dpp_config_imp_defs.h>
/* } */
/*************
 * DEFINES   *
 *************/
/* { */
#define SOC_OCC_MGMT_FLAGS_NONE                 (0)
#define SOC_OCC_MGMT_APP_USER                   (-1)

#define SOC_OCC_MGMT_INLIF_SIZE         (4)
#define SOC_OCC_MGMT_OUTLIF_SIZE(unit)  (SOC_DPP_IMP_DEFS_GET(unit, outlif_profile_nof_bits))
#define SOC_OCC_MGMT_OUTLIF_SIZE_MAX    (SOC_OCC_MGMT_OUTLIF_SIZE(unit)) /* Maximal number of Outlif bits (used only for diagnostic print) */
#define SOC_OCC_MGMT_RIF_SIZE           (1)

/* Two inlif profile bits are transferred to the egress */
/* These two bits have a total of four different configurations */
#define SOC_OCC_MGMT_NOF_INLIF_PROFILES_TRANSFERED_TO_EGRESS    (4)
/* This is a max of a bitmap */
#define SOC_OCC_MGMT_EG_FILTER_INLIF_PROFILE_BITMAP_MAX         ((1 << SOC_OCC_MGMT_NOF_INLIF_PROFILES_TRANSFERED_TO_EGRESS) - 1)

/* } */
/*************
 *   ENUMS   *
 *************/
/* { */
/* Inlif occupation management. */
typedef enum {
    /* EVB / MPLS MP (Same interface filtering) - used in Arad, Arad+, Jericho */
    SOC_OCC_MGMT_INLIF_APP_SIMPLE_SAME_INTERFACE = 0,
    /* OAM default lif occupation - used in Jericho */
    SOC_OCC_MGMT_INLIF_APP_OAM,
    /* PWE tagged mode - used in Jericho */
    SOC_OCC_MGMT_INLIF_APP_TAGGED_MODE,
    /* DSCP/EXP marking occupation when bridging - used in Arad+, Jericho */
    SOC_OCC_MGMT_INLIF_APP_SIMPLE_DSCP_MARKING,
    /* URPF - used in Arad */
    SOC_OCC_MGMT_INLIF_APP_STRICT_URPF, 
    /* Split Horizon Orientation - used in Jericho and above */
    SOC_OCC_MGMT_INLIF_APP_ORIENTATION,  
    /* Preserve DSCP for routed packets - used in Jericho */
    SOC_OCC_MGMT_INLIF_APP_PRESERVE_DSCP,  
    /*MAP DP to new PCP-DEI - used in Q-AX*/
    SOC_OCC_MGMT_INLIF_APP_POLICER_COLOR_MAPPING_PCP,
    SOC_OCC_MGMT_INLIF_APP_NOF
} SOC_OCC_MGMT_INLIF_APP;

/* Outlif occupation management. */
typedef enum {
    SOC_OCC_MGMT_OUTLIF_APP_OAM_PCP = 0,
    SOC_OCC_MGMT_OUTLIF_APP_OAM_DEFAULT_MEP,
    SOC_OCC_MGMT_OUTLIF_APP_TAGGED_MODE,
    /* Split Horizon Orientation - used in Jericho and above */
    SOC_OCC_MGMT_OUTLIF_APP_ORIENTATION,
    /* EVPN - used in Jericho and above */
    SOC_OCC_MGMT_OUTLIF_APP_EVPN,
    /* Egress PWE counting */
    SOC_OCC_MGMT_OUTLIF_APP_EG_PWE_COUNTING,
    SOC_OCC_MGMT_OUTLIF_APP_CUSTOM_P_TAGGED_TYPE,
    SOC_OCC_MGMT_OUTLIF_APP_MPLS_ENCAPSULATE_EXTENDED_LABEL,
    /* ELI addition by PRGE - used in Jericho only */
    SOC_OCC_MGMT_OUTLIF_APP_MPLS_ENTROPY_LABEL_INDICATION,
    /* MPLS Push or Swap - used in Jericho */
    SOC_OCC_MGMT_OUTLIF_APP_MPLS_PUSH_OR_SWAP,
    /* outlif is L2-LIF - used in QAX */
    SOC_OCC_MGMT_OUTLIF_APP_ROO_IS_L2_LIF,
    /* Additonal Labael (for instance for Segment Routing) - used in Jericho and above */
    SOC_OCC_MGMT_OUTLIF_APP_ADDITIONAL_LABEL,  
    /* Preserve DSCP for routed packets - used in Jericho */
    SOC_OCC_MGMT_OUTLIF_APP_PRESERVE_DSCP,  
    /*Outlif MTU filter mapping - used in JB0,QAX,QUX*/
    SOC_OCC_MGMT_OUTLIF_APP_MTU_FILTER,
    /*Outlif L2CP profile mapping - used in QAX and above*/
    SOC_OCC_MGMT_OUTLIF_APP_L2CP_EGRESS_PROFILE,
    /*Outlif profile ttl inheritance*/
    SOC_OCC_MGMT_OUTLIF_APP_TTL_INHERITANCE,
    /*Outlif profile tos inheritance*/
    SOC_OCC_MGMT_OUTLIF_APP_TOS_INHERITANCE,
    SOC_OCC_MGMT_OUTLIF_APP_NOF
} SOC_OCC_MGMT_OUTLIF_APP;

/* Rif occupation management - TBD */
typedef enum {
    SOC_OCC_MGMT_RIF_APP_NOF
} SOC_OCC_MGMT_RIF_APP;

/* Types of occupation maps */
typedef enum {
    SOC_OCC_MGMT_TYPE_INLIF = 0,
    SOC_OCC_MGMT_TYPE_OUTLIF,
    SOC_OCC_MGMT_TYPE_RIF,
    SOC_OCC_MGMT_TYPE_NOF
} SOC_OCC_MGMT_TYPE;

/************* 
 * STRUCTS
 *************/
typedef struct {
    /* In all profiles allocate one extra bit */
    int inlif_profile  [SOC_OCC_MGMT_INLIF_SIZE + 1];
    int outlif_profile [SOC_DPP_IMP_DEFS_MAX(OUTLIF_PROFILE_NOF_BITS) + 1];
    int rif_profile    [SOC_OCC_MGMT_RIF_SIZE + 1];
} ARAD_PP_OCCUPATION;

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

soc_error_t
arad_pp_occ_mgmt_init (int unit);

soc_error_t
arad_pp_occ_mgmt_deinit (int unit);

soc_error_t
arad_pp_occ_mgmt_get_app_mask(
   int                         unit,
   SOC_OCC_MGMT_TYPE           occ_type,
   int                         application_type,
   SHR_BITDCL                  *mask);

soc_error_t
arad_pp_occ_mgmt_app_get(
   int                         unit,
   SOC_OCC_MGMT_TYPE           occ_type,
   int                         application_type,
   SHR_BITDCL                  *full_occupation,
   uint32                      *val);

soc_error_t
arad_pp_occ_mgmt_app_set(
   int                         unit,
   SOC_OCC_MGMT_TYPE           occ_type,
   int                         application_type,
   uint32                      val,
   SHR_BITDCL                  *full_occupation);

soc_error_t 
arad_pp_occ_mgmt_diag_info_get(int unit, SOC_OCC_MGMT_TYPE occ_type, char* apps[]);

/*  
 * Returns arguments required for TCAM initialization 
 *  
 * @param (in)  unit 
 * @param (in)  occ_type: 
 *                  inlif/outlif/rif/etc.
 * @param (in)  application_type: 
 *                  application that is using occupation. 
 *                  use SOC_OCC_MGMT_APP_USER if you want to get the user part.
 * @param (in)  value: 
 *                  the value of the application in the occupation map. 
 *  
 * @param (out) full_occupation: 
 *                  the full state of the hardware that TCAM should expect for the given
 *                  input occupation type, application type, and value.
 * @param (out) mask: 
 *                  signals which bits of the full occupation are being used by the given
 *                  application type. 1 = used by app, 0 = not used by app.
 * @param (out) mask_flipped: 
 *                  for convenience, this is the negation ('flip') of mask.
 *  
 * NOTE: Out params may be passed in as NULL, in which case they'll be ignored.
 */
soc_error_t
arad_pp_occ_mgmt_tcam_args_get (
   int                  unit, 
   SOC_OCC_MGMT_TYPE    occ_type,
   int                  application_type,
   uint32               value,
   SHR_BITDCL           *full_occupation,
   SHR_BITDCL           *mask,
   SHR_BITDCL           *mask_flipped);

/* } */

#include <soc/dpp/SAND/Utils/sand_footer.h>

/* } __ARAD_PP_OCCUPATION_MGMT_INCLUDED__*/
#endif
