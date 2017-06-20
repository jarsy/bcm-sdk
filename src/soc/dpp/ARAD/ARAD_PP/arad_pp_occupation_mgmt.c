/* $Id: arad_pp_occupation_mgmt.c,v 1.72 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/

#ifdef _ERR_MSG_MODULE_NAME
#error "_ERR_MSG_MODULE_NAME redefined"
#endif

#define _ERR_MSG_MODULE_NAME BSL_SOC_MANAGEMENT

/*************
 * INCLUDES  *
 *************/
/* { */

#include <soc/dpp/SAND/Utils/sand_header.h>
#include <shared/bsl.h>
#include <shared/swstate/access/sw_state_access.h>
#include <shared/shr_occupation.h>
#include <soc/dcmn/error.h>
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_occupation_mgmt.h>

/* } */
/*************
 * MACROS    *
 *************/
/* { */

#define OCCUPATION_ACCESS_CALLBACKS(unit)  sw_state_access[unit].dpp.soc.arad.pp.occupation

/* } */
/*************
 * FUNCTIONS *
 *************/
/* { */

/* returns the SW-state-access callbacks of the given occupation type */
soc_error_t
arad_pp_occ_mgmt_access_callbacks_get(
   int unit,
   SOC_OCC_MGMT_TYPE occ_type,
   int(**load)(int, int, int*),
   int(**save)(int, int, int)
  )
{
    SOCDNX_INIT_FUNC_DEFS;

    switch (occ_type)
    {
    case SOC_OCC_MGMT_TYPE_INLIF:
        *load = OCCUPATION_ACCESS_CALLBACKS(unit).inlif_profile.get;
        *save = OCCUPATION_ACCESS_CALLBACKS(unit).inlif_profile.set;
        break;
    case SOC_OCC_MGMT_TYPE_OUTLIF:
        *load = OCCUPATION_ACCESS_CALLBACKS(unit).outlif_profile.get;
        *save = OCCUPATION_ACCESS_CALLBACKS(unit).outlif_profile.set;
        break;
    case SOC_OCC_MGMT_TYPE_RIF:
        *load = OCCUPATION_ACCESS_CALLBACKS(unit).rif_profile.get;
        *save = OCCUPATION_ACCESS_CALLBACKS(unit).rif_profile.set;
        break;

    /* Add more when new occupation types are introduced */

    default:
        SOCDNX_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_SOCDNX_MSG("Error, given occupation type is not supported\n")));
    }

exit:
    SOCDNX_FUNC_RETURN;
}

/* Creates and saves to SW-state the INLIF occupation map */
soc_error_t
arad_pp_occ_mgmt_init_inlif_occupation (int unit)
{
    soc_error_t             res;
    shr_occ_item_t          item;
    shr_occ_map_args_t      args;
    shr_occ_map_t           map;
    SHR_BITDCL              full_mask = 0xf; /* Inlif occupation consists of 4 bits */

    /* sw state access callbacks for saving and loading the occupation map */
    int (*load)(int,int,int*);
    int (*save)(int,int,int);

    SOCDNX_INIT_FUNC_DEFS;

    /* fill in sw state access callbacks */
    res = arad_pp_occ_mgmt_access_callbacks_get(unit, SOC_OCC_MGMT_TYPE_INLIF, &load, &save);
    SOCDNX_IF_ERR_EXIT(res);

    /* Initialize occupation map */
    shr_occ_map_args_init(unit, &args, SOC_OCC_MGMT_INLIF_SIZE, "INLIF", load, save);
    shr_occ_map_create(unit, &map, args, SOC_OCC_MGMT_FLAGS_NONE);

    /* Adding applications to occupation map */

    /* SAME INTERFACE */
    if (soc_property_get(unit, spn_BCM886XX_LOGICAL_INTERFACE_BRIDGE_FILTER_ENABLE, 0)) {
        /* For same interface we need one bit of the inlif occupation map, should be tranfered to the egress */
        SHR_BITDCL mask = 0x3; /* Only two lsb occupation map bits are transferred to egress */
        shr_occ_item_init(unit, &item, SOC_OCC_MGMT_INLIF_APP_SIMPLE_SAME_INTERFACE, &mask, 1);
        res = shr_occ_map_add_item(unit, &map, item, SOC_OCC_MGMT_FLAGS_NONE);
        SOCDNX_IF_ERR_EXIT(res);
    }

    /* DSCP MARKING */
    if (SOC_IS_ARADPLUS(unit) && soc_property_get(unit, spn_BCM886XX_QOS_L3_L2_MARKING, 0)) {
        /* For DSCP marking we need two bits of the inlif occupation map, should be tranfered to the egress */
        SHR_BITDCL mask = 0x3; /* Only two lsb occupation map bits are transferred to egress */
        shr_occ_item_init(unit, &item, SOC_OCC_MGMT_INLIF_APP_SIMPLE_DSCP_MARKING, &mask, 2);
        res = shr_occ_map_add_item(unit, &map, item, SOC_OCC_MGMT_FLAGS_NONE);
        SOCDNX_IF_ERR_EXIT(res);
    }

    /* DSCP preserve */
    if ((SOC_DPP_CONFIG(unit))->pp.routing_preserve_dscp & SOC_DPP_DSCP_PRESERVE_INLIF_CTRL_EN) {
        /* should be tranfered to the egress */
        SHR_BITDCL mask = 0x3; /* Only two lsb occupation map bits are transferred to egress */
        shr_occ_item_init(unit, &item, SOC_OCC_MGMT_INLIF_APP_PRESERVE_DSCP, &mask, 1);
        res = shr_occ_map_add_item(unit, &map, item, SOC_OCC_MGMT_FLAGS_NONE);
        SOCDNX_IF_ERR_EXIT(res);
    }

    /* STRICT URPF */
    if (SOC_IS_ARADPLUS_A0(unit) && soc_property_get(unit, spn_BCM886XX_L3_INGRESS_URPF_ENABLE, 0)) {
        shr_occ_item_init(unit, &item, SOC_OCC_MGMT_INLIF_APP_STRICT_URPF, &full_mask, 1);
        res = shr_occ_map_add_item(unit, &map, item, SOC_OCC_MGMT_FLAGS_NONE);
        SOCDNX_IF_ERR_EXIT(res);
    }

    /* OAM */
    if (SOC_IS_ARADPLUS(unit) && soc_property_get(unit, spn_BCM886XX_OAM_DEFAULT_PROFILE, 0)) {
        shr_occ_item_init(unit, &item, SOC_OCC_MGMT_INLIF_APP_OAM, &full_mask, soc_property_get(unit, spn_BCM886XX_OAM_DEFAULT_PROFILE, 0));
        res = shr_occ_map_add_item(unit, &map, item, SOC_OCC_MGMT_FLAGS_NONE);
        SOCDNX_IF_ERR_EXIT(res);
    }

    /* SPLIT HORIZON ORIENTATION */
    if (SOC_IS_JERICHO(unit) && soc_property_get(unit, spn_SPLIT_HORIZON_FORWARDING_GROUPS_MODE, 1)) {
        shr_occ_item_init(unit, &item, SOC_OCC_MGMT_INLIF_APP_ORIENTATION, &full_mask, 1);
        res = shr_occ_map_add_item(unit, &map, item, SOC_OCC_MGMT_FLAGS_NONE);
        SOCDNX_IF_ERR_EXIT(res);
    }

    /* DP profile mapping to PCP-DEI */
    if (SOC_IS_ARADPLUS(unit) && soc_property_get(unit, spn_QOS_POLICER_COLOR_MAPPING_PCP, 0)) {
        /* For DP mapping PCP-DEI we need two bits of the inlif occupation map, should be tranfered to the egress */
        SHR_BITDCL mask = 0x3; /* Only two lsb occupation map bits are transferred to egress */
        shr_occ_item_init(unit, &item, SOC_OCC_MGMT_INLIF_APP_POLICER_COLOR_MAPPING_PCP, &mask, 2);
        res = shr_occ_map_add_item(unit, &map, item, SOC_OCC_MGMT_FLAGS_NONE);
        SOCDNX_IF_ERR_EXIT(res);
    }

    /* Do not add more applications below */

    /* 
     * Solve constraints implied by applications, and save result to sw state. 
     * If successful, applications can be accessed using arad_pp_occ_mgmt_app_get/set.
     */
    res = shr_occ_map_solve(unit, &map, SOC_OCC_MGMT_FLAGS_NONE);
    SOCDNX_IF_ERR_EXIT(res);

exit:
    SOCDNX_FUNC_RETURN;
}

/* Creates and saves to SW-state the OUTLIF occupation map */
soc_error_t
arad_pp_occ_mgmt_init_outlif_occupation (int unit)
{
    soc_error_t             res;
    shr_occ_item_t          item;
    shr_occ_map_args_t      args;
    shr_occ_map_t           map;
    SHR_BITDCL              full_mask = 0x3f; /* Outlif occupation map consists of 6 bits */
    SHR_BITDCL              mask;
    int                     nof_bits;

    /* sw state access callbacks for saving and loading the occupation map */
    int (*load)(int,int,int*);
    int (*save)(int,int,int);

    SOCDNX_INIT_FUNC_DEFS;

    /* fill in sw state access callbacks */
    res = arad_pp_occ_mgmt_access_callbacks_get(unit, SOC_OCC_MGMT_TYPE_OUTLIF, &load, &save);
    SOCDNX_IF_ERR_EXIT(res);
    /* Initialize occupation map */
    shr_occ_map_args_init(unit, &args, SOC_OCC_MGMT_OUTLIF_SIZE(unit), "OUTLIF", load, save);
    shr_occ_map_create(unit, &map, args, SOC_OCC_MGMT_FLAGS_NONE);

    /* Adding applications to occupation map */

    /* SPLIT HORIZON ORIENTATION */
    if (soc_property_get(unit, spn_SPLIT_HORIZON_FORWARDING_GROUPS_MODE, 1) == 1) {
        /* Allocate one bit for AC */
        mask = 0x7; /* only 3 LSB allowed */
        shr_occ_item_init(unit, &item, SOC_OCC_MGMT_OUTLIF_APP_ORIENTATION, &mask, 1);
        res = shr_occ_map_add_item(unit, &map, item, SOC_OCC_MGMT_FLAGS_NONE);
        SOCDNX_IF_ERR_EXIT(res);
        mask = 0x38;    /* only 3 MSB allowed */
        shr_occ_item_init(unit, &item, SOC_OCC_MGMT_OUTLIF_APP_ORIENTATION, &mask, 1);
        res = shr_occ_map_add_item(unit, &map, item, SOC_OCC_MGMT_FLAGS_NONE);
        SOCDNX_IF_ERR_EXIT(res);
    }
    else if (soc_property_get(unit, spn_SPLIT_HORIZON_FORWARDING_GROUPS_MODE, 1) == 2) {
        /* Allocate two bits for AC */
        if (SOC_IS_JERICHO_B0_AND_ABOVE(unit) && !SOC_IS_JERICHO_PLUS(unit) && soc_property_get(unit, "logical_port_mim_out", 0) && soc_property_get(unit, "bcm886xx_roo_enable", 0)) {
            /* Jericho B0 MIM ROO workaround.
             * In the case of MIM ROO linker layer in Jericho B0, the outlif profile is {1'b1, MAC-DA[47:43]}.
             * Therefore to avoid randomness, we statically map the outlif profiles with 1 in the MSbit to orientation = 3.
             */
            mask = 0x20; /* must allocate the MSbit for orientation */
            shr_occ_item_init(unit, &item, SOC_OCC_MGMT_OUTLIF_APP_ORIENTATION, &mask, 1);
            res = shr_occ_map_add_item(unit, &map, item, SOC_OCC_MGMT_FLAGS_NONE);
            SOCDNX_IF_ERR_EXIT(res);
            mask = 0x7;    /* the second bit can be from the LSbits */
            shr_occ_item_init(unit, &item, SOC_OCC_MGMT_OUTLIF_APP_ORIENTATION, &mask, 1);
            res = shr_occ_map_add_item(unit, &map, item, SOC_OCC_MGMT_FLAGS_NONE);
            SOCDNX_IF_ERR_EXIT(res);
        } else {
            mask = 0x7; /* only 3 LSB allowed */
            shr_occ_item_init(unit, &item, SOC_OCC_MGMT_OUTLIF_APP_ORIENTATION, &mask, 2);
            res = shr_occ_map_add_item(unit, &map, item, SOC_OCC_MGMT_FLAGS_NONE);
            SOCDNX_IF_ERR_EXIT(res);
        }
    }

    /* OAM PCP */
    /* Due to a HW bug no outlif profiles are availble in Jericho A0. Fixed on higher devices */
    if (SOC_IS_JERICHO_B0_AND_ABOVE(unit) && (SOC_DPP_CONFIG(unit)->pp.oam_enable)) {

        /* OAM PCP - allocate 1 bit is enabled */
        /* Due to a HW bug no outlif profiles are availble in Jericho A0. Fixed on higher devices */
        if (soc_property_get(unit, spn_OAM_PCP_MODE, 1)) {
            if (SOC_IS_JERICHO_PLUS(unit)) {
                mask = full_mask;
            } else {
                mask = 0x7; /* only 3 LSB allowed for AC in Jericho */
            }

            shr_occ_item_init(unit, &item, SOC_OCC_MGMT_OUTLIF_APP_OAM_PCP, &mask, 1);
            res = shr_occ_map_add_item(unit, &map, item, SOC_OCC_MGMT_FLAGS_NONE);
            SOCDNX_IF_ERR_EXIT(res); 
        }

        /* OAM DEFAULT MEP */
        /* Due to a HW bug no outlif profiles are availble in Jericho A0. Fixed on higher devices */
        if (soc_property_get(unit, spn_BCM886XX_OAM_DEFAULT_PROFILE_EGRESS, 0)) {
            if (SOC_IS_JERICHO_PLUS(unit)) {
                mask = full_mask;
            }
            else {
                mask = 0x7; /* only 3 LSB allowed for AC in Jericho */
            }

            shr_occ_item_init(unit, &item, SOC_OCC_MGMT_OUTLIF_APP_OAM_DEFAULT_MEP, &full_mask,
                              soc_property_get(unit, spn_BCM886XX_OAM_DEFAULT_PROFILE_EGRESS, 0));
            res = shr_occ_map_add_item(unit, &map, item, SOC_OCC_MGMT_FLAGS_NONE);
            SOCDNX_IF_ERR_EXIT(res);
        }
    }

    /* EVPN */
    if (SOC_IS_JERICHO(unit) && soc_property_get(unit, spn_EVPN_ENABLE, 0)) {
        shr_occ_item_init(unit, &item, SOC_OCC_MGMT_OUTLIF_APP_EVPN, &full_mask,1);
        res = shr_occ_map_add_item(unit, &map, item, SOC_OCC_MGMT_FLAGS_NONE);
        SOCDNX_IF_ERR_EXIT(res);
    }

    /* EG-PWE-COUNTING */
    if (SOC_IS_JERICHO(unit) && soc_property_suffix_num_get(unit, -1, spn_CUSTOM_FEATURE, "eg_pwe_counting", 0)) {
        shr_occ_item_init(unit, &item, SOC_OCC_MGMT_OUTLIF_APP_EG_PWE_COUNTING, &full_mask, 1);
        res = shr_occ_map_add_item(unit, &map, item, SOC_OCC_MGMT_FLAGS_NONE);
        SOCDNX_IF_ERR_EXIT(res);
    }

    if (SOC_IS_QAX(unit) && SOC_DPP_CONFIG(unit)->pp.oam_use_double_outlif_injection) {
        shr_occ_item_init(unit, &item, SOC_OCC_MGMT_OUTLIF_APP_EG_PWE_COUNTING, &full_mask, 1);
        res = shr_occ_map_add_item(unit, &map, item, SOC_OCC_MGMT_FLAGS_NONE);
        SOCDNX_IF_ERR_EXIT(res);
    }

    if (SOC_IS_JERICHO(unit) && soc_property_get(unit, spn_ROO_EXTENSION_LABEL_ENCAPSULATION, 0)) {
        /* Extension label must use a known bit of the outlif profile for the PRGE program that creates it.
        If this value is changed, the relevant programs also must be modified! */
        mask = 1;
        shr_occ_item_init(unit, &item, SOC_OCC_MGMT_OUTLIF_APP_MPLS_ENCAPSULATE_EXTENDED_LABEL, &mask,1);
        res = shr_occ_map_add_item(unit, &map, item, SOC_OCC_MGMT_FLAGS_NONE);
        SOCDNX_IF_ERR_EXIT(res);
    }

    if (soc_property_suffix_num_get(unit, -1, spn_CUSTOM_FEATURE, "prge_ptagged_load", 0) ||
        soc_property_suffix_num_get(unit, -1, spn_CUSTOM_FEATURE,  "lsr_manipulation", 0) ) {
        shr_occ_item_init(unit, &item, SOC_OCC_MGMT_OUTLIF_APP_CUSTOM_P_TAGGED_TYPE, &full_mask,3);
        res = shr_occ_map_add_item(unit, &map, item, SOC_OCC_MGMT_FLAGS_NONE);
        SOCDNX_IF_ERR_EXIT(res);
    }

    /* ELI using PRGE */
    if (SOC_IS_JERICHO(unit) && (!SOC_IS_JERICHO_PLUS(unit)) &&
         soc_property_get(unit, spn_MPLS_EGRESS_LABEL_ENTROPY_INDICATOR_ENABLE, 0)) {
        shr_occ_item_init(unit, &item, SOC_OCC_MGMT_OUTLIF_APP_MPLS_ENTROPY_LABEL_INDICATION, &full_mask,1);
        res = shr_occ_map_add_item(unit, &map, item, SOC_OCC_MGMT_FLAGS_NONE);
        SOCDNX_IF_ERR_EXIT(res);
    }

    /* MPLS push or swap encapsulation (Jericho B0 or PLUS) */
    if ((SOC_IS_JERICHO_B0(unit) || SOC_IS_QMX_B0(unit) || SOC_IS_JERICHO_PLUS_A0(unit) || SOC_IS_QUX(unit)) &&
         soc_property_get(unit, spn_MPLS_ENCAPSULATION_ACTION_SWAP_OR_PUSH_ENABLE, 0)) {
        shr_occ_item_init(unit, &item, SOC_OCC_MGMT_OUTLIF_APP_MPLS_PUSH_OR_SWAP, &full_mask,1);
        res = shr_occ_map_add_item(unit, &map, item, SOC_OCC_MGMT_FLAGS_NONE);
        SOCDNX_IF_ERR_EXIT(res);
    }

    if (SOC_IS_JERICHO_PLUS(unit)) {
        uint32 nof_bits = 0;
        shr_occ_item_init(unit, &item, SOC_OCC_MGMT_OUTLIF_APP_ROO_IS_L2_LIF, &full_mask,1);
        res = shr_occ_map_add_item(unit, &map, item, SOC_OCC_MGMT_FLAGS_NONE);
        SOCDNX_IF_ERR_EXIT(res);

        nof_bits = (soc_property_get(unit, spn_NOF_L2CP_EGRESS_PROFILES_MAX, 1) / 2) & 0x3;
        nof_bits = (nof_bits > 2) ? 2 : nof_bits;
        if (nof_bits != 0) {
            shr_occ_item_init(unit, &item, SOC_OCC_MGMT_OUTLIF_APP_L2CP_EGRESS_PROFILE, &full_mask, nof_bits);
            res = shr_occ_map_add_item(unit, &map, item, SOC_OCC_MGMT_FLAGS_NONE);
            SOCDNX_IF_ERR_EXIT(res);
        }
    }

    if (SOC_IS_JERICHO(unit) && soc_property_get(unit, spn_MPLS_EGRESS_LABEL_EXTENDED_ENCAPSULATION_MODE, 0)) {
        if (SOC_IS_JERICHO_AND_BELOW(unit)) {
            /* In Jericho ARP/LL EEDB entry has only 5 bit for outlif profile */
            mask = 0x1F;
        }
        else {
            if (SOC_IS_JERICHO_PLUS(unit) && soc_property_get(unit, spn_EVPN_ENABLE, 0)) {
                mask = 1; /* In EVPN, the bit should be fixed for the EVPN PRGE programs */
            }
            else {
                mask = full_mask;
            }
        }
        shr_occ_item_init(unit, &item, SOC_OCC_MGMT_OUTLIF_APP_ADDITIONAL_LABEL, &mask,1);
        res = shr_occ_map_add_item(unit, &map, item, SOC_OCC_MGMT_FLAGS_NONE);
        SOCDNX_IF_ERR_EXIT(res);

    }

    /* DSCP preserve */
    if ((SOC_DPP_CONFIG(unit))->pp.routing_preserve_dscp & SOC_DPP_DSCP_PRESERVE_OUTLIF_CTRL_EN) {
        mask = 0x1F; /* ARP/LL EEDB entry has only 5 bit in for outlif profile in current chips (include JR+ and QAX)-2016.8.12*/
        shr_occ_item_init(unit, &item, SOC_OCC_MGMT_OUTLIF_APP_PRESERVE_DSCP, &mask, 1);
        res = shr_occ_map_add_item(unit, &map, item, SOC_OCC_MGMT_FLAGS_NONE);
        SOCDNX_IF_ERR_EXIT(res);
    }

    if (SOC_IS_QUX(unit) && soc_property_get(unit, spn_LOGICAL_INTERFACE_OUT_QOS_INHERITANCE, 0)) {
        shr_occ_item_init(unit, &item, SOC_OCC_MGMT_OUTLIF_APP_TOS_INHERITANCE, &full_mask,1);
        res = shr_occ_map_add_item(unit, &map, item, SOC_OCC_MGMT_FLAGS_NONE);
        SOCDNX_IF_ERR_EXIT(res);
    }

    if (SOC_IS_QUX(unit) && soc_property_get(unit, spn_LOGICAL_INTERFACE_OUT_TTL_INHERITANCE, 0)) {
        shr_occ_item_init(unit, &item, SOC_OCC_MGMT_OUTLIF_APP_TTL_INHERITANCE, &full_mask,1);
        res = shr_occ_map_add_item(unit, &map, item, SOC_OCC_MGMT_FLAGS_NONE);
        SOCDNX_IF_ERR_EXIT(res);
    }


     /*MTU Filtering per Outlif*/
    if(SOC_IS_JERICHO_B0_AND_ABOVE(unit) && soc_property_get(unit, spn_TRAP_LIF_MTU_ENABLE, 0))
    {
        if(SOC_IS_JERICHO_B0(unit))
        {
            mask = 0x7; /* only 3 LSB allowed for AC in Jericho */
            nof_bits = 2; /*Only 4 out of 8 profiles allowed due to restriction of AC*/
        }
        else
        {
            mask = full_mask;
            nof_bits = 3;
        }
        shr_occ_item_init(unit, &item, SOC_OCC_MGMT_OUTLIF_APP_MTU_FILTER, &mask,nof_bits);
        res = shr_occ_map_add_item(unit, &map, item, SOC_OCC_MGMT_FLAGS_NONE);
        SOCDNX_IF_ERR_EXIT(res);
    }
    /* Do not add more applications below */

    /* 
     * Solve constraints implied by applications, and save result to sw state. 
     * If successful, applications can be accessed using arad_pp_occ_mgmt_app_get/set.
     */
    res = shr_occ_map_solve(unit, &map, SOC_OCC_MGMT_FLAGS_NONE);
    SOCDNX_IF_ERR_EXIT(res);

exit:
    SOCDNX_FUNC_RETURN;
}

/*
 * Function returns application relevant mask in given occupation type. 
 * That is, which bits are occupied by the application. 
 * 
 * @param (in) unit 
 * @param (in) occ_type - inlif/outlif/rif/etc.
 * @param (in) application_type - application that is using occupation. 
 *             use SOC_OCC_MGMT_APP_USER if you want to get the user part.
 * @param (out) mask - the mask of the application in the occupation map.
 *  
 * @return soc_error_t
 */
soc_error_t arad_pp_occ_mgmt_get_app_mask(
   int                                                              unit,
   SOC_OCC_MGMT_TYPE                                                occ_type,
   int                                                              application_type,
   SHR_BITDCL                                                       *mask
   ){
    soc_error_t res;
    int(*load)(int, int, int*);
    int(*save)(int, int, int);

    SOCDNX_INIT_FUNC_DEFS;

    res = arad_pp_occ_mgmt_access_callbacks_get(unit, occ_type, &load, &save);
    SOCDNX_IF_ERR_EXIT(res);

    res = shr_occ_mgmt_item_mask_get(unit, load, application_type, mask);
    SOCDNX_IF_ERR_EXIT(res);

exit:
    SOCDNX_FUNC_RETURN;
}

/* 
 * Gets application's value from the full_occupation. 
 * Full occupation is a value read from HW, containing all data of the shared resource. 
 * An application may occupy non consecutive bits. The returned value will only be affected 
 * by the application-relevant bits. 
 *  
 * @param (in)  unit 
 * @param (in)  occ_type - inlif/outlif/rif/etc.
 * @param (in)  application_type - application that is using occupation. 
 *              use SOC_OCC_MGMT_APP_USER if you want to get the user part.
 * @param (in)  full_occupation - shared resource value as read from HW.
 * @param (out) val - application's value in full_occupation
 */
soc_error_t
arad_pp_occ_mgmt_app_get(
   int                         unit,
   SOC_OCC_MGMT_TYPE           occ_type,
   int                         application_type,
   SHR_BITDCL                  *full_occupation,
   uint32                      *val
   ) {
    soc_error_t res;
    uint32 read_opcode = 0;
    int(*load)(int, int, int*);
    int(*save)(int, int, int);

    SOCDNX_INIT_FUNC_DEFS;

    res = arad_pp_occ_mgmt_access_callbacks_get(unit, occ_type, &load, &save);
    SOCDNX_IF_ERR_EXIT(res);

    res = shr_occ_mgmt_item_access(unit, load, save, application_type, val, read_opcode, full_occupation);
    SOCDNX_IF_ERR_EXIT(res);

exit:
    SOCDNX_FUNC_RETURN;
}

/* 
 * Sets application's value into full_occupation. 
 * Full occupation is a value read from HW, containing all data of the shared resource. 
 * An application may occupy non consecutive bits. The input value will only affect 
 * the application-relevant bits. 
 * Note: make sure your value can be contained by the applications' number of bits. 
 *  
 * @param (in)  unit 
 * @param (in)  occ_type - inlif/outlif/rif/etc.
 * @param (in)  application_type - application that is using occupation. 
 *              use SOC_OCC_MGMT_APP_USER if you want to get the user part.
 * @param (in)  val - application's value to be set in full_occupation 
 * @param (out) full_occupation - shared resource value as read from HW. 
 */
soc_error_t
arad_pp_occ_mgmt_app_set(
   int                         unit,
   SOC_OCC_MGMT_TYPE           occ_type,
   int                         application_type,
   uint32                      val,
   SHR_BITDCL                  *full_occupation
   ) {
    soc_error_t res;
    uint32 write_opcode = 1;
    int(*load)(int, int, int*);
    int(*save)(int, int, int);

    SOCDNX_INIT_FUNC_DEFS;

    res = arad_pp_occ_mgmt_access_callbacks_get(unit, occ_type, &load, &save);
    SOCDNX_IF_ERR_EXIT(res);

    res = shr_occ_mgmt_item_access(unit, load, save, application_type, &val, write_opcode, full_occupation);
    SOCDNX_IF_ERR_EXIT(res);

exit:
    SOCDNX_FUNC_RETURN;
}

/* Initializes all of the occupation maps */
soc_error_t
arad_pp_occ_mgmt_init (int unit)
{
    uint8 is_allocated;
    soc_error_t res;
    SOCDNX_INIT_FUNC_DEFS;

    /* init SW state DB */
    res = sw_state_access[unit].dpp.soc.arad.pp.occupation.is_allocated(unit, &is_allocated);
    SOCDNX_IF_ERR_EXIT(res);

    if(!is_allocated) {
        res = sw_state_access[unit].dpp.soc.arad.pp.occupation.alloc(unit);
        SOCDNX_IF_ERR_EXIT(res);
    }

    /* init occupation maps */

    res = arad_pp_occ_mgmt_init_inlif_occupation(unit);
    SOCDNX_IF_ERR_EXIT(res);

    if (SOC_IS_JERICHO(unit)) {
        res = arad_pp_occ_mgmt_init_outlif_occupation(unit);
        SOCDNX_IF_ERR_EXIT(res);
    }

exit:
    SOCDNX_FUNC_RETURN;
}

soc_error_t
arad_pp_occ_mgmt_deinit (int unit)
{
    SOCDNX_INIT_FUNC_DEFS;
    SOCDNX_FUNC_RETURN;
}

/* 
 * please help me find a suitable name for this function :) 
 *  
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
   SHR_BITDCL           *mask_flipped)
{
    soc_error_t res;
    int size;
    int(*load)(int, int, int*);
    int(*save)(int, int, int);
    SHR_BITDCL* _mask = NULL;
    SOCDNX_INIT_FUNC_DEFS;

    /* get occupation map's size */
    res = arad_pp_occ_mgmt_access_callbacks_get(unit,occ_type,&load,&save);
    SOCDNX_IF_ERR_EXIT(res);
    res = (*load)(unit,0,&size);
    SOCDNX_IF_ERR_EXIT(res);

    /* get the app's mask relative to the map */
    SOCDNX_ALLOC(_mask, uint32, _SHR_BITDCLSIZE(size),"mask");
    sal_memset(_mask, 0, SHR_BITALLOCSIZE(size));
    res = arad_pp_occ_mgmt_get_app_mask(unit,occ_type,application_type,_mask);
    SOCDNX_IF_ERR_EXIT(res);

    /* fill in mask output param */
    if (mask != NULL) {
        SHR_BITCOPY_RANGE(mask, 0, _mask, 0, size);
    }

    /* fill in mask_flipped output param */
    if (mask_flipped != NULL) {
        SHR_BITNEGATE_RANGE(_mask,0,size,mask_flipped);
    }

    /* fill in full_occupation output param */
    if (full_occupation != NULL) {
        SHR_BITCLR_RANGE(full_occupation,0,SHR_BITALLOCSIZE(size));
        res = arad_pp_occ_mgmt_app_set(unit,occ_type,application_type,value,full_occupation);
        SOCDNX_IF_ERR_EXIT(res);
    }

exit:
    SOCDNX_FREE(_mask);
    SOCDNX_FUNC_RETURN;
}

/* } */
/****************
 * DIAG helpers *
 ****************/
/* { */

char*
arad_pp_occ_mgmt_get_profile_name(SOC_OCC_MGMT_TYPE occ_type) {
    switch (occ_type) {
    case SOC_OCC_MGMT_TYPE_INLIF: 
        return "InLIF Profile";
    case SOC_OCC_MGMT_TYPE_OUTLIF: 
        return "OutLIF Profile";
    case SOC_OCC_MGMT_TYPE_RIF: 
        return "RIF Profile";
    default:
        return "";
    }
}

char*
arad_pp_occ_mgmt_get_app_name(SOC_OCC_MGMT_TYPE occ_type, int app) {

    if (app == SOC_OCC_MGMT_APP_USER) {
        return"Unreserved";
    }

    switch (occ_type) {

    /* INLIF PROFILE */
    case SOC_OCC_MGMT_TYPE_INLIF: 
        switch (app) {
        case SOC_OCC_MGMT_INLIF_APP_SIMPLE_SAME_INTERFACE:
            return "EVB / MPLS MP (Same interface filtering)";
        case SOC_OCC_MGMT_INLIF_APP_OAM:
           return "OAM default lif occupation map";
        case SOC_OCC_MGMT_INLIF_APP_TAGGED_MODE:
           return "PWE tagged mode";
        case SOC_OCC_MGMT_INLIF_APP_SIMPLE_DSCP_MARKING:
           return "DSCP/EXP marking profile when bridging";
        case SOC_OCC_MGMT_INLIF_APP_STRICT_URPF:
           return "URPF";
        case SOC_OCC_MGMT_INLIF_APP_ORIENTATION:
           return "Split Horizon Orientation";
        case SOC_OCC_MGMT_INLIF_APP_PRESERVE_DSCP:
           return "DSCP remark preserve";
	    case SOC_OCC_MGMT_INLIF_APP_POLICER_COLOR_MAPPING_PCP:
           return "DP profile for PCP mapping when bridging";
        default:
            return "Invalid";
        }

    /* OUTLIF PROFILE */
    case SOC_OCC_MGMT_TYPE_OUTLIF: 
        switch (app) {
        case SOC_OCC_MGMT_OUTLIF_APP_OAM_PCP:
           return "OAM PCP lif";
        case SOC_OCC_MGMT_OUTLIF_APP_OAM_DEFAULT_MEP:
           return "OAM default";
        case SOC_OCC_MGMT_OUTLIF_APP_TAGGED_MODE:
           return "PWE tagged mode";
        case SOC_OCC_MGMT_OUTLIF_APP_ORIENTATION:
           return "Split Horizon Orientation";
        case SOC_OCC_MGMT_OUTLIF_APP_EVPN:
           return "EVPN";
        case SOC_OCC_MGMT_OUTLIF_APP_EG_PWE_COUNTING:
            return "Egress PWE counting";
        case SOC_OCC_MGMT_OUTLIF_APP_CUSTOM_P_TAGGED_TYPE:
           return "ptagged outlif type";
        case SOC_OCC_MGMT_OUTLIF_APP_MPLS_ENCAPSULATE_EXTENDED_LABEL:
           return "Mpls encapsulate extended label";
        case SOC_OCC_MGMT_OUTLIF_APP_MPLS_ENTROPY_LABEL_INDICATION:
           return "Mpls encapsulate entropy label indication";
        case SOC_OCC_MGMT_OUTLIF_APP_MPLS_PUSH_OR_SWAP:
           return "Mpls encapsulate swap or push action according to fwd code";
        case SOC_OCC_MGMT_OUTLIF_APP_ROO_IS_L2_LIF:
            return "in ROO application, tunnel outlif is L2_LIF";
        case SOC_OCC_MGMT_OUTLIF_APP_ADDITIONAL_LABEL:
            return "Mpls encapsultion extended label";
        case SOC_OCC_MGMT_OUTLIF_APP_PRESERVE_DSCP:
           return "DSCP remark preserve";
        case SOC_OCC_MGMT_OUTLIF_APP_MTU_FILTER:
           return "MTU Filter profile";
        case SOC_OCC_MGMT_OUTLIF_APP_L2CP_EGRESS_PROFILE:
           return "L2CP egress profile";
        case SOC_OCC_MGMT_OUTLIF_APP_TOS_INHERITANCE:
           return "TOS inheritance ";
        case SOC_OCC_MGMT_OUTLIF_APP_TTL_INHERITANCE:
           return "TTL inheritance ";
        default:
            return "Invalid";
        }

    /* RIF PROFILE */
    case SOC_OCC_MGMT_TYPE_RIF: 
        return "RIF_PROFILE";

    default:
        return "Invalid";
    }
}

soc_error_t
arad_pp_occ_mgmt_diag_info_get(int unit, SOC_OCC_MGMT_TYPE occ_type, char* apps[])
{
    uint32 res;
	int i;
    int occupation[256];

    int(*load)(int, int, int*);
    int(*save)(int, int, int);

	SOCDNX_INIT_FUNC_DEFS;

    apps[0] = arad_pp_occ_mgmt_get_profile_name(occ_type);

    res = arad_pp_occ_mgmt_access_callbacks_get(unit,occ_type,&load,&save);
    SOCDNX_IF_ERR_EXIT(res);

    res = shr_occ_mgmt_solution_load(unit,load,occupation);
    SOCDNX_IF_ERR_EXIT(res);

    for (i = 1; i <= occupation[0]; i++ ) {
        apps[i] = arad_pp_occ_mgmt_get_app_name(occ_type, occupation[i]);
    }

exit:
    SOCDNX_FUNC_RETURN;
}

/* } */

#include <soc/dpp/SAND/Utils/sand_footer.h>

