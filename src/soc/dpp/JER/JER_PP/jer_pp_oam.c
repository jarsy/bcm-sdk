/* $Id: arad_pp_oam.c,v 1.111 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 * $
*/

#ifdef _ERR_MSG_MODULE_NAME
  #error "_ERR_MSG_MODULE_NAME redefined"
#endif
#define _ERR_MSG_MODULE_NAME BSL_SOC_OAM

/*************
 * INCLUDES  *
 *************/
/* { */

#include <shared/bsl.h>
#include <shared/swstate/access/sw_state_access.h>
#include <soc/dcmn/error.h>
#include <soc/dpp/SAND/Utils/sand_header.h>

#include <soc/dpp/SAND/Management/sand_general_macros.h>
#include <soc/dpp/SAND/Management/sand_error_code.h>
#include <soc/dpp/SAND/Utils/sand_os_interface.h>

#include <soc/mcm/memregs.h>
#include <soc/mcm/memacc.h>
#include <soc/mem.h>

#include <soc/dpp/ARAD/arad_chip_regs.h>
#include <soc/dpp/ARAD/arad_reg_access.h>
#include <soc/dpp/ARAD/arad_tbl_access.h>
#include <soc/dpp/mbcm_pp.h>


#include <soc/dpp/JER/JER_PP/jer_pp_oam.h>
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_oam.h>
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_oamp_pe.h>
#include <soc/dpp/PPC/ppc_api_oam.h>
#include <bcm_int/common/debug.h>


/* } */
/*************
 * DEFINES   *
 *************/
/* { */



/* mp_type_entry is a concatination of the bits OAM-is-BFD, MDL-MP-Type(2), Above-MDL-MEP-Bitmap-OR*/
#define _JER_PP_OAM_MP_TYPE_MAP_IS_BFD(mp_type_entry) (mp_type_entry >>3)
#define _JER_PP_OAM_MP_TYPE_MAP_MDL_MP_TYPE(mp_type_entry) ((mp_type_entry>>1) &0x3)
#define _JER_PP_OAM_MP_TYPE_MAP_ABOVE_MDL_MEP_BITMAP_OR(mp_type_entry) (mp_type_entry & 0x1) 
#define _JER_PP_OAM_PTCH_OPAQUE_VALUE 7


/**
 * Write on the table EPNI_CFG_MAPPING_TO_OAM_PCP according to 
 * tc and outlif profile. 
 *  The index  used is  {TC, OAM-LIF-Profile(2)}.
 */
#define JER_PP_OAM_SET_EGRESS_OAM_PCP_BY_OUTLIF_PROFILE_AND_TC(outlif_prof, packet_tc, oam_pcp) \
    do {\
    uint32 reg32=0; \
        soc_EPNI_CFG_MAPPING_TO_OAM_PCPm_field32_set(unit,&reg32,CFG_MAPPING_TO_OAM_PCPf, (oam_pcp) );\
        rv = WRITE_EPNI_CFG_MAPPING_TO_OAM_PCPm(unit,SOC_CORE_ALL,(((outlif_prof) & 0x3) | (((packet_tc) & 0x7) <<2)),&reg32 );\
        SOCDNX_IF_ERR_EXIT(rv);\
} while (0)
    

/* } */
/*************
 * MACROS    *
 *************/
/* { */

/* Sets the MP-Map according to the MD-Level for an ingress default endpoint (clears the map if _mdl<0)*/
#define JER_PP_OAM_DEFAULT_INGRESS_MEP_MDL_SET(_mep_index, _mdl, _reg64) \
            do { \
                    int _i = 16 * ((_mep_index) - ARAD_PP_OAM_DEFAULT_EP_INGRESS_0); \
                    int _j, mdl = (_mdl); \
                    for (_j=0; _j<=mdl; _j++) { \
                        COMPILER_64_BITSET(_reg64, _i+(2*_j)); \
                    } \
                    for (; _j<8; _j++) { \
                        COMPILER_64_BITCLR(_reg64, _i+(2*_j)); \
                    } \
            } while (0)

/* Sets the MP-Map according to the MD-Level for an egress default endpoint (clears the map if _mdl<0)*/
#define JER_PP_OAM_DEFAULT_EGRESS_MEP_MDL_SET(_mep_index, _mdl, _reg64) \
            do { \
                    int _i = 16 * ((_mep_index) - ARAD_PP_OAM_DEFAULT_EP_EGRESS_0); \
                    int _j, mdl = (_mdl); \
                    for (_j=0; _j<=mdl; _j++) { \
                        COMPILER_64_BITSET(_reg64, _i+(2*_j)+1); \
                    } \
                    for (; _j<8; _j++) { \
                        COMPILER_64_BITCLR(_reg64, _i+(2*_j)+1); \
                    } \
            } while (0)

/*
 The Key to the RSP TCAM is composed of Y-1731 (1b), { Y.1731 ? 0 (8b),OpCode (8b) : channel (16b) }
 Macro builds the TCAM key according to the above encoding.
*/
#define RSP_TCAM_KEY_SET(key, y1731, channel_or_opcode) \
	do {\
    uint32 y1731__ = (y1731) , opcode__ = (channel_or_opcode);\
	key=0;\
    SHR_BITCOPY_RANGE(&key, 16,&y1731__, 0,1);\
	SHR_BITCOPY_RANGE(&key, 0,&opcode__, 0,16);\
} while (0)

#define RSP_TCAM_OPCODE_TO_OAM_TS_FORMAT(opcode)\
    (((opcode)==SOC_PPC_OAM_ETHERNET_PDU_OPCODE_LMM) || ((opcode)==SOC_PPC_OAM_ETHERNET_PDU_OPCODE_SLM))? _ARAD_PP_OAM_SUBTYPE_LM : \
     ((opcode)==SOC_PPC_OAM_ETHERNET_PDU_OPCODE_DMM)?_ARAD_PP_OAM_SUBTYPE_DM_1588 :\
    _ARAD_PP_OAM_SUBTYPE_DEFAULT

/*
The Data in the RSP_TCAM is composed of  valid (1), OAM-TS-format (2),  opcode_or_channel_type (16) 
For OAM-TS format use macro RSP_TCAM_OPCODE_TO_OAM_TS_FORMAT*/
#define RSP_TCAM_DATA_SET(data, generate_response, oam_ts_format, opcode_or_channel_type)\
do {\
    uint32 opcode__ = (opcode_or_channel_type), gen__ = (generate_response), format__ = (oam_ts_format); \
    data=0;\
    SHR_BITCOPY_RANGE(&data,0,&opcode__,0,16);\
    SHR_BITCOPY_RANGE(&data,16,&format__,0,2);\
    SHR_BITCOPY_RANGE(&data,18,&gen__,0,1);\
} while (0)


/**
 * Translate ARAD_PP_OAM_DEFAULT_EP_EGRESS_X to X 
 */
#define JER_PP_OAM_EGRESS_DEFAULT_PROFILE_ENUM_TO_DEFAULT_PROF(egress_default_prof_enum) \
    (egress_default_prof_enum - ARAD_PP_OAM_DEFAULT_EP_EGRESS_0)



/* } */
/*************
 * TYPE DEFS *
 *************/
/* { */
/*
 * Struct represents fields from PPH BASE 
 */
 
typedef struct {
    uint8   field1;
    uint8   field2;
    uint8   field3;
    uint8   field4;
    uint8   field5;
    uint8   field6;
    uint8   field7;
    uint8   field8;
    uint16  field9;
    uint32  field10; 
} PPH_base_head;



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

/**********************************************/ 
/*                   static functions declerations*/
/**********************************************/ 

STATIC soc_error_t _soc_jer_pp_oam_set_opcode_n_table(int unit);
STATIC soc_error_t soc_jer_pp_oam_classifier_oam1_passive_entries_add(int unit);
STATIC soc_error_t soc_jer_pp_oam_init_response_table(int unit);
/**********************************************/ 
/* *****************************************    */              
/**********************************************/ 

/**
 * Clear the table OAMP_DM_TRIGER
 * 
 * @author atanasov (12/23/2015)
 * 
 * @param unit 
 * 
 * @return STATIC soc_error_t 
 */
STATIC soc_error_t soc_jer_pp_dm_triger_init(int unit){
    int rv;
    uint32 reg32;
    uint32 write_val = 1;
    SOCDNX_INIT_FUNC_DEFS;

    rv = WRITE_OAMP_ENABLE_DYNAMIC_MEMORY_ACCESSr(unit, write_val);
    SOCDNX_IF_ERR_EXIT(rv);

    reg32 = 0;

    rv = WRITE_OAMP_DM_TRIGERm(unit, MEM_BLOCK_ALL, 0, &reg32);
    SOCDNX_IF_ERR_EXIT(rv);

    write_val = 0;

    rv = WRITE_OAMP_ENABLE_DYNAMIC_MEMORY_ACCESSr(unit, write_val);
    SOCDNX_IF_ERR_EXIT(rv);

exit:
    SOCDNX_FUNC_RETURN;
}

/* Jericho specific classifier initializations */
soc_error_t soc_jer_pp_oam_classifier_init(int unit)
{
    int rv;
    soc_reg_above_64_val_t reg_above_64;
    uint32 reg32[1];
    int use_pcp_from_packet;
    int core;
    uint32 tc, outlif_profile;

    SOCDNX_INIT_FUNC_DEFS;

    if (SOC_IS_JERICHO_AND_BELOW(unit)) {
        rv = soc_jer_pp_oam_mp_type_config_init(unit);
        SOCDNX_IF_ERR_EXIT(rv);
    }

    use_pcp_from_packet = soc_property_get(unit, spn_OAM_PCP_VALUE_EXTRACT_FROM_PACKET, 1); /* default: use PCP from packet*/
    SOC_DPP_CORES_ITER(SOC_CORE_ALL, core) {
        /* PCP configurations.*/
        rv = READ_IHP_VTT_GENERAL_CONFIGS_1r(unit,core,reg_above_64);
        SOCDNX_IF_ERR_EXIT(rv);
        soc_reg_field_set(unit,IHP_VTT_GENERAL_CONFIGS_1r,reg_above_64,OAM_USE_PACKET_PCPf,use_pcp_from_packet );
        rv = WRITE_IHP_VTT_GENERAL_CONFIGS_1r(unit,core,reg_above_64);
        SOCDNX_IF_ERR_EXIT(rv);

        rv = READ_IHP_FLP_GENERAL_CFGr(unit,core,reg_above_64);
        SOCDNX_IF_ERR_EXIT(rv);
        /* Use Arad mode OAM-ID */
        soc_reg_above_64_field32_set(unit,IHP_FLP_GENERAL_CFGr,reg_above_64,OAM_ID_ARAD_MODEf,1 );
        /* Non OAM packets get subtype 0 (not the one defined in IHP_OAM_COUNTER_INCREMENT_BITMAPr*/
        soc_reg_above_64_field32_set(unit,IHP_FLP_GENERAL_CFGr,reg_above_64,EGRESS_NON_OAM_PACKET_SUB_TYPEf,0 );
        rv = WRITE_IHP_FLP_GENERAL_CFGr(unit,core,reg_above_64);
        SOCDNX_IF_ERR_EXIT(rv);
    }


    /* IP TOS to PCP, DS to PCP used only by RFC-6374. Not implemented.*/

    /*Set the Egress PCP setting (configured statically).*/
    for (outlif_profile=0 ; outlif_profile < 0x40  /*all outlif profiles*/ ;++outlif_profile) {
        /* Mapping of outlif profile (6 bits) to oam-lif-profile (2). */
        uint32 oam_outlif_profile;
        rv = MBCM_PP_DRIVER_CALL(unit, mbcm_pp_occ_mgmt_app_get,
                                 (unit,SOC_OCC_MGMT_TYPE_OUTLIF, SOC_OCC_MGMT_OUTLIF_APP_OAM_DEFAULT_MEP , ((uint32*)&outlif_profile), &oam_outlif_profile));
        SOCDNX_IF_ERR_EXIT(rv);

        *reg32=0;
        soc_EPNI_OUTLIF_TO_OAM_LIF_PROFILE_MAPm_field32_set(unit,reg32,OUTLIF_TO_OAM_LIF_PROFILE_MAPf,oam_outlif_profile);
        rv = WRITE_EPNI_OUTLIF_TO_OAM_LIF_PROFILE_MAPm(unit, SOC_CORE_ALL, outlif_profile, reg32);
        SOCDNX_IF_ERR_EXIT(rv);
    }

    for (outlif_profile=0 ; outlif_profile < 4  /*2 bit value*/; ++outlif_profile) {
        for (tc=0 ; tc < 8 /*3 bit value */ ; ++tc) {
            /* The default mapping will be OAM_PCP = TC | OAM-LIF-Profile(2)*/
            JER_PP_OAM_SET_EGRESS_OAM_PCP_BY_OUTLIF_PROFILE_AND_TC(outlif_profile, tc, outlif_profile | tc);
        }
    }

    /* Turn off Arad+ PCP chicken-bit*/
    rv = soc_reg_field32_modify(unit, EPNI_CFG_BUG_FIX_CHICKEN_BITS_REG_1r, REG_PORT_ANY, CFG_OAM_PCP_MAPPING_DISABLEf, 0);
    SOCDNX_IF_ERR_EXIT(rv);


exit:
    SOCDNX_FUNC_RETURN;
}

/* Jericho specific OAMP initializations */
soc_error_t soc_jer_pp_oam_oamp_init(int unit)
{
    int rv;
    soc_reg_above_64_val_t reg_above_64;
    uint32 reg32[1];

    SOCDNX_INIT_FUNC_DEFS;

    rv = soc_jer_pp_dm_triger_init(unit);
    SOCDNX_IF_ERR_EXIT(rv);

    /* BASE MAC SA FILL. Always zero.*/
    rv = READ_OAMP_CCM_MAC_SAr(unit,reg_above_64);
    SOCDNX_IF_ERR_EXIT(rv);
    soc_reg_above_64_field32_set(unit,OAMP_CCM_MAC_SAr,reg_above_64,BASE_MAC_SA_FILLf,0);
    rv = WRITE_OAMP_CCM_MAC_SAr(unit,reg_above_64);
    SOCDNX_IF_ERR_EXIT(rv);

    rv = READ_OAMP_UP_PTCHr(unit,reg32);
    SOCDNX_IF_ERR_EXIT(rv);
    soc_reg_field_set(unit,OAMP_UP_PTCHr, reg32, UP_PTCH_OPAQUE_PT_ATTR_PROFILE_0f, 0x7);
    soc_reg_field_set(unit,OAMP_UP_PTCHr, reg32, UP_PTCH_OPAQUE_PT_ATTR_PROFILE_1f, 0x7);
    soc_reg_field_set(unit,OAMP_UP_PTCHr, reg32, UP_PTCH_OPAQUE_PT_ATTR_MODEf, 0);/*use the MAC SA in the sensible way.*/
    rv = WRITE_OAMP_UP_PTCHr(unit,*reg32);
    SOCDNX_IF_ERR_EXIT(rv);

    rv = _soc_jer_pp_oam_set_opcode_n_table(unit);
    SOCDNX_IF_ERR_EXIT(rv);

    if (soc_property_get(unit, spn_ITMH_PROGRAMMABLE_MODE_ENABLE, 0)) {
        
        soc_reg_t reg = (SOC_IS_QAX_A0(unit) || SOC_IS_QUX(unit)) ? OAMP_REG_0130r : OAMP_TX_PPHr;
        soc_field_t field = (SOC_IS_QAX_A0(unit) || SOC_IS_QUX(unit)) ? FIELD_5_5f : TX_USE_JER_ITMHf;
        /* Enable Jericho ITMH */
        SOCDNX_IF_ERR_EXIT(soc_reg_field32_modify(unit, reg, REG_PORT_ANY,field, 1));
    }

    rv = soc_jer_pp_oam_init_response_table(unit);
    SOCDNX_IF_ERR_EXIT(rv);

exit:
    SOCDNX_FUNC_RETURN;
}


/* ****************************************************************************/
/*****************************************************************************/

/**
 * Changing each level of mp_type_vector from from_mp_type, to
 * to_mp_type, starting from start_level, going down, until real
 * other entry is found.
 */
soc_error_t soc_jer_pp_oam_oem1_mp_type_vector_update(
        int unit,
        int start_level,
        uint32 matching_mp_type_vector,
        uint32 * mp_type_vector,
        uint8 from_mp_type,
        uint8 to_mp_type) {
    int level;
    uint8 mdl_mp_type;

    for (level=start_level; level>=SOC_PPC_OAM_ETH_MP_LEVEL_0; level--) {
        mdl_mp_type = JERICHO_PP_OAM_EXTRACT_MDL_MP_TYPE_FROM_MP_TYPE_VECTOR_BY_LEVEL(*mp_type_vector, level);

        if (mdl_mp_type != from_mp_type) {
            break;
        } else {
            if (mdl_mp_type == _JER_PP_OAM_MDL_MP_TYPE_PASSIVE_MATCH) {
                /* Changing 'fake' PASSIVE_MATCH entries to NO_MP entries */
                if (matching_mp_type_vector) {
                    mdl_mp_type = JERICHO_PP_OAM_EXTRACT_MDL_MP_TYPE_FROM_MP_TYPE_VECTOR_BY_LEVEL(matching_mp_type_vector, level);
                    if (mdl_mp_type == _JER_PP_OAM_MDL_MP_TYPE_ACTIVE_MATCH) {
                        break; /* 'real' PASSIVE_MATCH */
                    }
                }
            }
        }

        JERICHO_PP_OAM_SET_MDL_MP_TYPE_VECTOR_BY_LEVEL(*mp_type_vector,to_mp_type,level);
    }

    return SOC_SAND_OK;
}


/**
 * See details in header file
 * 
 * @author sinai (06/07/2016)
 * 
 * @param unit 
 * @param classifier_mep_entry 
 * @param is_active 
 * 
 * @return int 
 */
int soc_jer_pp_oam_counter_pointer_addition(int unit, const SOC_PPC_OAM_CLASSIFIER_MEP_ENTRY * classifier_mep_entry, int is_active) {
    int ingress_shift = ((classifier_mep_entry->flags & SOC_PPC_OAM_CLASSIFIER_MEP_ENTRY_FLAG_PCP) ? 3 : 0);
    int is_upmep = ((classifier_mep_entry->flags & SOC_PPC_OAM_CLASSIFIER_MEP_ENTRY_FLAG_UPMEP) != 0);
    return (SOC_IS_JERICHO(unit) ? ((is_upmep && !is_active) || (!is_upmep && is_active)) <<ingress_shift : 0);
}



soc_error_t soc_jer_pp_oam_oem1_mep_add(
        int unit,
        const SOC_PPC_OAM_CLASSIFIER_MEP_ENTRY * classifier_mep_entry,
        const SOC_PPC_OAM_CLASSIFIER_OEM1_ENTRY_KEY * oem1_key,
        const SOC_PPC_OAM_CLASSIFIER_OEM1_ENTRY_PAYLOAD * prev_payload,
        SOC_PPC_OAM_CLASSIFIER_OEM1_ENTRY_PAYLOAD * new_payload,
        uint8 is_active,
        uint8 update) {
    uint32 res = SOC_SAND_OK;
    SOC_PPC_OAM_CLASSIFIER_OEM1_ENTRY_KEY oem1_matching_key;
    SOC_PPC_OAM_CLASSIFIER_OEM1_ENTRY_PAYLOAD oem1_matching_payload;
    int is_mep;
    uint8 prev_mdl_mp_type, new_mdl_mp_type;
    uint32 new_mp_type_vector;

    SOC_SAND_INIT_ERROR_DEFINITIONS(0);

    is_mep = ((classifier_mep_entry->flags & SOC_PPC_OAM_CLASSIFIER_MEP_ENTRY_FLAG_ENDPOINT) != 0);

    /* first: Get the relavent MDL-MP-Type, by level */
    prev_mdl_mp_type = JERICHO_PP_OAM_EXTRACT_MDL_MP_TYPE_FROM_MP_TYPE_VECTOR_BY_LEVEL(prev_payload->mp_type_vector, classifier_mep_entry->md_level);
    if (!update && (prev_mdl_mp_type==_JER_PP_OAM_MDL_MP_TYPE_MIP || prev_mdl_mp_type==_JER_PP_OAM_MDL_MP_TYPE_ACTIVE_MATCH)) {
        SOC_SAND_SET_ERROR_MSG((_BSL_SOCDNX_SAND_MSG("Only one MEP/MIP per Level per LIF may be defined.")));
    }
    new_mdl_mp_type = (!is_mep) ? _JER_PP_OAM_MDL_MP_TYPE_MIP : is_active ? _JER_PP_OAM_MDL_MP_TYPE_ACTIVE_MATCH :
        _JER_PP_OAM_MDL_MP_TYPE_PASSIVE_MATCH;
    /*To address the issue of UPMEP packet on AC is being trapped by PWE DOWN MEP, Passive entry for MPLS-TP LSP/PWE 
      need to be added only when below custome feature is enabed on Jericho and above devices*/
    if ((is_active) || (!(SOC_PPC_OAM_IS_MEP_TYPE_Y1731(classifier_mep_entry->mep_type) &&
        (soc_property_suffix_num_get(unit, -1, spn_CUSTOM_FEATURE, "oam_downmep_pwe_classification", 0) == 0)))) {
        new_mp_type_vector = prev_payload->mp_type_vector;
        /* Update current level on vector */
        JERICHO_PP_OAM_SET_MDL_MP_TYPE_VECTOR_BY_LEVEL(new_mp_type_vector,new_mdl_mp_type,classifier_mep_entry->md_level);

        /* If entry is MIP, lower levels should also be updated on vector to PASSIVE_MATCH (to prevent traversing of lower level packets).
           Active / Passive entries already handled by the Not-Above-MDL-MEP-Bitmap-OR bit */
        if (new_mdl_mp_type == _JER_PP_OAM_MDL_MP_TYPE_MIP) {
            res = soc_jer_pp_oam_oem1_mp_type_vector_update(unit, classifier_mep_entry->md_level - 1, 0, &new_mp_type_vector,
                    _JER_PP_OAM_MDL_MP_TYPE_NO_MP, _JER_PP_OAM_MDL_MP_TYPE_PASSIVE_MATCH);
            SOC_SAND_CHECK_FUNC_RESULT(res, 83, exit);
        } else {
            /* entry is not MIP but there might be a MIP above, remove 'fake' PASSIVE_MATCH in lower levels */
            uint8 found = 0;
            /* Get OEM1 payload of the matching entry */
            oem1_matching_key.ingress = 1 - oem1_key->ingress;
            oem1_matching_key.oam_lif = (oem1_key->oam_lif == classifier_mep_entry->lif) ? classifier_mep_entry->passive_side_lif : classifier_mep_entry->lif;
            oem1_matching_key.your_discr = oem1_key->your_discr;
            if(oem1_matching_key.oam_lif != _BCM_OAM_INVALID_LIF){ /*non-ingress only case*/
                res = arad_pp_oam_classifier_oem1_entry_get_unsafe(unit, &oem1_matching_key, &oem1_matching_payload, &found);
                SOC_SAND_CHECK_FUNC_RESULT(res, 42, exit);
            }

            res = soc_jer_pp_oam_oem1_mp_type_vector_update(unit, classifier_mep_entry->md_level - 1, (found ? oem1_matching_payload.mp_type_vector : 0),
                    &new_mp_type_vector, _JER_PP_OAM_MDL_MP_TYPE_PASSIVE_MATCH, _JER_PP_OAM_MDL_MP_TYPE_NO_MP);
            SOC_SAND_CHECK_FUNC_RESULT(res, 83, exit);
        }

        new_payload->mp_type_vector = new_mp_type_vector;
    }
    if (is_mep) {
        /* For passive entries the mp profile doesn't matter. Assume the existing value (or zero if none exists) */
        new_payload->mp_profile = is_active ? classifier_mep_entry->non_acc_profile : prev_payload->mp_profile;
    } else {
        /* for MIPs the "passive" profile is also active, just on the other side. */
        new_payload->mp_profile = is_active ? classifier_mep_entry->non_acc_profile : classifier_mep_entry->non_acc_profile_passive;
    }

    new_payload->counter_ndx = 0;
    if (classifier_mep_entry->counter) {
        /* if counter is valid, update it based on direction and priority */
        new_payload->counter_ndx = classifier_mep_entry->counter + 
                                   soc_jer_pp_oam_counter_pointer_addition(unit, classifier_mep_entry,is_active);
    }

exit:
    SOC_SAND_EXIT_AND_SEND_ERROR_SOCDNX((_BSL_SOCDNX_SAND_MSG("Something went wrong")));
}

soc_error_t soc_jer_pp_oam_oem1_mep_delete(
        int unit,
        const SOC_PPC_OAM_CLASSIFIER_MEP_ENTRY * classifier_mep_entry,
        const SOC_PPC_OAM_CLASSIFIER_OEM1_ENTRY_KEY * oem1_key,
        uint32 * new_mp_type_vector) {
    uint32 res = SOC_SAND_OK;
    uint8 mdl_mp_type, matching_mdl_mp_type, is_first_real_upper_is_mip, found=0;
    int level;
    SOC_PPC_OAM_CLASSIFIER_OEM1_ENTRY_KEY oem1_matching_key;
    SOC_PPC_OAM_CLASSIFIER_OEM1_ENTRY_PAYLOAD oem1_payload, oem1_matching_payload;

    SOC_SAND_INIT_ERROR_DEFINITIONS(0);

    /* If an entry is deleted and the first 'real' upper entry
       (skipping NO_MP and fake PASSIVE_MATCH entries) is a mip:
          -> Change current and all lower level mp-type (until other real entry is found) to PASSIVE_MATCH
       If an active/passive entry is deleted and the first 'real' upper entry is not a mip:
          -> Change current mp-type to NO_MP
       If a mip entry is deleted and there is no mip above:
          -> Change current and all lower level mp-type (until other real entry is found) to NO_MP */

    /* Get OEM1 payload of the entry */
    res = arad_pp_oam_classifier_oem1_entry_get_unsafe(unit, oem1_key, &oem1_payload, &found);
    SOC_SAND_CHECK_FUNC_RESULT(res, 42, exit);
    if (!found) {
        SOC_SAND_SET_ERROR_CODE(ARAD_PP_OAM_CLASSIFIER_ENTRY_NOT_FOUND_ERR, 90, exit);
    }

    /* Get OEM1 payload of the matching entry (active side) */
    oem1_matching_key.ingress = 1 - oem1_key->ingress;
    oem1_matching_key.oam_lif = (oem1_key->oam_lif == classifier_mep_entry->lif) ? classifier_mep_entry->passive_side_lif : classifier_mep_entry->lif;
    oem1_matching_key.your_discr = oem1_key->your_discr;
    found = 0;
    if(oem1_matching_key.oam_lif != _BCM_OAM_INVALID_LIF) { /*non-ingress only case*/
        res = arad_pp_oam_classifier_oem1_entry_get_unsafe(unit, &oem1_matching_key, &oem1_matching_payload, &found);
        SOC_SAND_CHECK_FUNC_RESULT(res, 42, exit);
    }

    /* Check if first 'real' entry in a higher level is a MIP */
    is_first_real_upper_is_mip = 0;
    level = classifier_mep_entry->md_level + 1;
    if(level<SOC_PPC_OAM_NOF_ETH_MP_LEVELS) {
        mdl_mp_type = JERICHO_PP_OAM_EXTRACT_MDL_MP_TYPE_FROM_MP_TYPE_VECTOR_BY_LEVEL(oem1_payload.mp_type_vector, level);
        if (mdl_mp_type == _JER_PP_OAM_MDL_MP_TYPE_MIP) {
            is_first_real_upper_is_mip++;
        } else if (mdl_mp_type == _JER_PP_OAM_MDL_MP_TYPE_PASSIVE_MATCH) {
            if (found) { /* Entry might alredy deleted if there was no mp-type != NO_MP in any level. */
                matching_mdl_mp_type = JERICHO_PP_OAM_EXTRACT_MDL_MP_TYPE_FROM_MP_TYPE_VECTOR_BY_LEVEL(oem1_matching_payload.mp_type_vector, level);
                if (matching_mdl_mp_type != _JER_PP_OAM_MDL_MP_TYPE_ACTIVE_MATCH) { /* 'fake' passive, there is a MIP above */
                    is_first_real_upper_is_mip++;
                }
            } else { /* 'fake' passive, there is a MIP above */
                is_first_real_upper_is_mip++;
            }
        }
    }

    /* Get current mp-type */
    mdl_mp_type = JERICHO_PP_OAM_EXTRACT_MDL_MP_TYPE_FROM_MP_TYPE_VECTOR_BY_LEVEL(oem1_payload.mp_type_vector, classifier_mep_entry->md_level);

    *new_mp_type_vector = oem1_payload.mp_type_vector;

    if (is_first_real_upper_is_mip) {
        /* Change current mp-type to PASSIVE_MATCH */
        JERICHO_PP_OAM_SET_MDL_MP_TYPE_VECTOR_BY_LEVEL(*new_mp_type_vector, _JER_PP_OAM_MDL_MP_TYPE_PASSIVE_MATCH, classifier_mep_entry->md_level);

        /* Change each lower level mp-type to PASSIVE_MATCH until other real entry is found. */
        res = soc_jer_pp_oam_oem1_mp_type_vector_update(unit, classifier_mep_entry->md_level - 1, 0, new_mp_type_vector,
                                                        _JER_PP_OAM_MDL_MP_TYPE_NO_MP, _JER_PP_OAM_MDL_MP_TYPE_PASSIVE_MATCH);
        SOC_SAND_CHECK_FUNC_RESULT(res, 83, exit);
    } else {
        /* No higher MIP */
        /* Change current level entry to NO_MP */
        JERICHO_PP_OAM_SET_MDL_MP_TYPE_VECTOR_BY_LEVEL(*new_mp_type_vector, _JER_PP_OAM_MDL_MP_TYPE_NO_MP, classifier_mep_entry->md_level);

        if (mdl_mp_type == _JER_PP_OAM_MDL_MP_TYPE_MIP) {
            /* Change each lower level mp-type to NO_MP until other real entry is found. */
            res = soc_jer_pp_oam_oem1_mp_type_vector_update(unit, classifier_mep_entry->md_level - 1, (found ? oem1_matching_payload.mp_type_vector : 0),
                                                        new_mp_type_vector, _JER_PP_OAM_MDL_MP_TYPE_PASSIVE_MATCH, _JER_PP_OAM_MDL_MP_TYPE_NO_MP);
            SOC_SAND_CHECK_FUNC_RESULT(res, 83, exit);
        }
    }

exit:
    SOC_SAND_EXIT_AND_SEND_ERROR_SOCDNX((_BSL_SOCDNX_SAND_MSG("Something went wrong")));
}

/*
 * soc_jer_pp_oam_set_inlif_profile_map
 * Adds a mapping from an inlif profile (4b) to an OAM lif profile (2b)
 */
soc_error_t
  soc_jer_pp_oam_inlif_profile_map_set(
     SOC_SAND_IN  int                                                  unit,
     SOC_SAND_IN  uint32                                               inlif_profile,
     SOC_SAND_IN  uint32                                               oam_profile
  ) {
    uint32 profile_map;
    int core;

    SOCDNX_INIT_FUNC_DEFS;

    SOC_DPP_CORES_ITER(SOC_CORE_ALL, core) {
        SOCDNX_IF_ERR_EXIT(READ_IHP_OAM_IN_LIF_PROFILE_MAPr(unit, core, &profile_map));
        SHR_BITCOPY_RANGE(&profile_map, (inlif_profile * 2), &oam_profile, 0, 2);
        SOCDNX_IF_ERR_EXIT(WRITE_IHP_OAM_IN_LIF_PROFILE_MAPr(unit, core, profile_map));
    }

exit:
    SOCDNX_FUNC_RETURN;
}


soc_error_t
  soc_jer_pp_oam_inlif_profile_map_get(
     SOC_SAND_IN  int                                                  unit,
     SOC_SAND_IN  uint32                                               inlif_profile,
     SOC_SAND_OUT uint32                                               *oam_profile
  ) {
    uint32 profile_map;

    SOCDNX_INIT_FUNC_DEFS;

    SOCDNX_NULL_CHECK(oam_profile);

    SOCDNX_IF_ERR_EXIT(READ_IHP_OAM_IN_LIF_PROFILE_MAPr(unit, SOC_CORE_DEFAULT, &profile_map));
    *oam_profile = (profile_map >> (inlif_profile * 2)) & 0x3;

exit:
    SOCDNX_FUNC_RETURN;
}


/*********************************************************************
 * OAM default profiles add/remove
 *********************************************************************/

soc_error_t 
_soc_jer_oam_set_default_egress_profile_by_profile(int unit, 
                                                   ARAD_PP_OAM_DEFAULT_EP_ID default_egress_prof, 
                                                   const SOC_PPC_OAM_CLASSIFIER_MEP_ENTRY   *classifier_mep_entry) {
    soc_field_t default_profile_counters[] = {COUNTER_FOR_EGRESS_PROFILE_0f,COUNTER_FOR_EGRESS_PROFILE_1f,
        COUNTER_FOR_EGRESS_PROFILE_2f,COUNTER_FOR_EGRESS_PROFILE_3f};
    soc_reg_above_64_val_t reg_above_64;
    uint64 reg64;
    int core;
    SOCDNX_INIT_FUNC_DEFS;

    SOC_REG_ABOVE_64_CLEAR(reg_above_64);
    COMPILER_64_SET(reg64, 0, 0);

    if (default_egress_prof < ARAD_PP_OAM_DEFAULT_EP_EGRESS_0) {
        SOCDNX_EXIT_WITH_ERR(SOC_E_INTERNAL,
                             (_BSL_SOCDNX_MSG("INTERNAL: function only for egress profile")));
    }

    SOC_DPP_CORES_ITER(SOC_CORE_ALL, core) {
        /* Set counter index for given OAM-lif-profile*/
        SOCDNX_IF_ERR_EXIT(READ_IHP_OAM_DEFAULT_COUNTERSr(unit, core, reg_above_64));
        soc_reg_above_64_field32_set(unit, IHP_OAM_DEFAULT_COUNTERSr, 
                                     reg_above_64, default_profile_counters[JER_PP_OAM_EGRESS_DEFAULT_PROFILE_ENUM_TO_DEFAULT_PROF(default_egress_prof)], 
                                     classifier_mep_entry->counter);
        SOCDNX_IF_ERR_EXIT(WRITE_IHP_OAM_DEFAULT_COUNTERSr(unit, core, reg_above_64)); 

        /* For egress default mp-map - set the first MDL odd bits in each OAM-lif-profiles and clear all the other odd bits */
        SOCDNX_IF_ERR_EXIT(READ_IHP_OAM_DEFAULT_MP_TYPE_VECTORr(unit, core, &reg64));
        JER_PP_OAM_DEFAULT_EGRESS_MEP_MDL_SET(default_egress_prof, classifier_mep_entry->md_level, reg64);
        SOCDNX_IF_ERR_EXIT(WRITE_IHP_OAM_DEFAULT_MP_TYPE_VECTORr(unit, core, reg64)); 
    }

exit:
    SOCDNX_FUNC_RETURN;
}



soc_error_t
  soc_jer_pp_oam_classifier_default_profile_add(
    SOC_SAND_IN  int                                unit,
    SOC_SAND_IN  ARAD_PP_OAM_DEFAULT_EP_ID          mep_index,
    SOC_SAND_IN  SOC_PPC_OAM_CLASSIFIER_MEP_ENTRY   *classifier_mep_entry,
    SOC_SAND_IN  uint8                              update_action_only
  )
{
    int rv;
    int core;

    uint64 reg64;
    soc_reg_above_64_val_t reg_above_64;

    SOCDNX_INIT_FUNC_DEFS;

    SOCDNX_NULL_CHECK(classifier_mep_entry);

    SOC_REG_ABOVE_64_CLEAR(reg_above_64);

    if (!update_action_only) {
        if (mep_index >= ARAD_PP_OAM_DEFAULT_EP_EGRESS_0) {
            /* Egress */
            if (!SOC_IS_JERICHO_B0_AND_ABOVE(unit)) {
                /* Due to HW bug, the outlif profile isn't reaching the classifier. A single default endpoint is available in the egress
                   by setting the same mp-map to all possible outlif profiles */
                ARAD_PP_OAM_DEFAULT_EP_ID ep_mep_index;
                for (ep_mep_index = ARAD_PP_OAM_DEFAULT_EP_EGRESS_0; ep_mep_index < ARAD_PP_OAM_DEFAULT_EP_NOF_IDS; ++ep_mep_index) {
                    SOCDNX_IF_ERR_EXIT(_soc_jer_oam_set_default_egress_profile_by_profile(unit, ep_mep_index, classifier_mep_entry));
                }
            } else {
                SOCDNX_IF_ERR_EXIT(_soc_jer_oam_set_default_egress_profile_by_profile(unit, mep_index, classifier_mep_entry));
            }

        } /* Egress end */
        else {
            /* Ingress */

            SOC_DPP_CORES_ITER(SOC_CORE_ALL, core) {
                /* Set counter index */
                SOCDNX_IF_ERR_EXIT(READ_IHP_OAM_DEFAULT_COUNTERSr(unit, core, reg_above_64));
                soc_reg_above_64_field32_set(unit, IHP_OAM_DEFAULT_COUNTERSr, reg_above_64, COUNTER_FOR_INGRESS_PROFILE_0f + (int)mep_index,
                                             classifier_mep_entry->counter);
                SOCDNX_IF_ERR_EXIT(WRITE_IHP_OAM_DEFAULT_COUNTERSr(unit, core, reg_above_64));

                /* Set MP map - Set the first MDL even bits for the specific OAM-lif-profile */
                SOCDNX_IF_ERR_EXIT(READ_IHP_OAM_DEFAULT_MP_TYPE_VECTORr(unit, core, &reg64));
                JER_PP_OAM_DEFAULT_INGRESS_MEP_MDL_SET(mep_index, classifier_mep_entry->md_level, reg64);
                SOCDNX_IF_ERR_EXIT(WRITE_IHP_OAM_DEFAULT_MP_TYPE_VECTORr(unit, core, reg64));
            }
        }
    }

    rv = soc_jer_pp_oam_classifier_default_profile_action_set(unit,mep_index,classifier_mep_entry);
    SOCDNX_IF_ERR_EXIT(rv);


exit:
    SOCDNX_FUNC_RETURN;
}


soc_error_t
  soc_jer_pp_oam_classifier_default_profile_remove(
    SOC_SAND_IN  int                                unit,
    SOC_SAND_IN  ARAD_PP_OAM_DEFAULT_EP_ID          mep_index
  ) {
    uint64 reg64;
    uint32 eg_mep_idx;
    int core;

    SOCDNX_INIT_FUNC_DEFS;

    if (mep_index >= ARAD_PP_OAM_DEFAULT_EP_EGRESS_0) {
        /* Egress */

        SOC_DPP_CORES_ITER(SOC_CORE_ALL, core) {
            /* Clear all odd bits */
            SOCDNX_IF_ERR_EXIT(READ_IHP_OAM_DEFAULT_MP_TYPE_VECTORr(unit, core, &reg64));
            for (eg_mep_idx=ARAD_PP_OAM_DEFAULT_EP_EGRESS_0; eg_mep_idx<=ARAD_PP_OAM_DEFAULT_EP_EGRESS_3; eg_mep_idx++) {
                JER_PP_OAM_DEFAULT_EGRESS_MEP_MDL_SET(eg_mep_idx, -1, reg64);
            }
            SOCDNX_IF_ERR_EXIT(WRITE_IHP_OAM_DEFAULT_MP_TYPE_VECTORr(unit, core, reg64));
        }

    }
    else {
        /* Ingress */

        SOC_DPP_CORES_ITER(SOC_CORE_ALL, core) {
            /* Clear even bits for the specific OAM-lif-profile */
            SOCDNX_IF_ERR_EXIT(READ_IHP_OAM_DEFAULT_MP_TYPE_VECTORr(unit, core, &reg64));
            JER_PP_OAM_DEFAULT_INGRESS_MEP_MDL_SET(mep_index, -1, reg64);
            SOCDNX_IF_ERR_EXIT(WRITE_IHP_OAM_DEFAULT_MP_TYPE_VECTORr(unit, core, reg64));
        }
    }

exit:
    SOCDNX_FUNC_RETURN;
}


soc_error_t
  soc_jer_pp_oam_classifier_default_profile_action_set(
    SOC_SAND_IN  int                                unit,
    SOC_SAND_IN  ARAD_PP_OAM_DEFAULT_EP_ID          mep_index,
    SOC_SAND_IN  SOC_PPC_OAM_CLASSIFIER_MEP_ENTRY   *classifier_mep_entry
  )
{
    uint32 reg32;
    int core;

    SOCDNX_INIT_FUNC_DEFS;

    SOCDNX_NULL_CHECK(classifier_mep_entry);

    if (mep_index >= ARAD_PP_OAM_DEFAULT_EP_EGRESS_0) {
        /* Egress */

        SOC_DPP_CORES_ITER(SOC_CORE_ALL, core) {
            /* Due to HW bug, the outlif profile isn't reaching the classifier. A single default endpoit is available in the egress
               by setting the same mp-map to all possible outlif profiles */

            /* Set MP-Profile For all egress OAM-Lif-profiles */
            SOCDNX_IF_ERR_EXIT(READ_IHP_OAM_DEFAULT_ACC_MEP_PROFILESr(unit, core, &reg32));
            soc_reg_field_set(unit, IHP_OAM_DEFAULT_ACC_MEP_PROFILESr, &reg32, ACC_MEP_PROFILE_FOR_EGRESS_PROFILE_0f,
                              classifier_mep_entry->non_acc_profile);
            soc_reg_field_set(unit, IHP_OAM_DEFAULT_ACC_MEP_PROFILESr, &reg32, ACC_MEP_PROFILE_FOR_EGRESS_PROFILE_1f,
                              classifier_mep_entry->non_acc_profile);
            soc_reg_field_set(unit, IHP_OAM_DEFAULT_ACC_MEP_PROFILESr, &reg32, ACC_MEP_PROFILE_FOR_EGRESS_PROFILE_2f,
                              classifier_mep_entry->non_acc_profile);
            soc_reg_field_set(unit, IHP_OAM_DEFAULT_ACC_MEP_PROFILESr, &reg32, ACC_MEP_PROFILE_FOR_EGRESS_PROFILE_3f,
                              classifier_mep_entry->non_acc_profile);
            SOCDNX_IF_ERR_EXIT(WRITE_IHP_OAM_DEFAULT_ACC_MEP_PROFILESr(unit, core, reg32));
        }

    } /* Egress end */
    else {
        /* Ingress */

        SOC_DPP_CORES_ITER(SOC_CORE_ALL, core) {
            /* Set MP-Profile For The specific relevant OAM-lif-profile */
            SOCDNX_IF_ERR_EXIT(READ_IHP_OAM_DEFAULT_ACC_MEP_PROFILESr(unit, core, &reg32));
            soc_reg_field_set(unit, IHP_OAM_DEFAULT_ACC_MEP_PROFILESr, &reg32, ACC_MEP_PROFILE_FOR_INGRESS_PROFILESf + mep_index,
                              classifier_mep_entry->non_acc_profile);
            SOCDNX_IF_ERR_EXIT(WRITE_IHP_OAM_DEFAULT_ACC_MEP_PROFILESr(unit, core, reg32));
        }
    }

exit:
    SOCDNX_FUNC_RETURN;
}



/* ****************************************************************************/
/*****************************************************************************/

soc_error_t soc_jer_pp_oam_oamp_eth1731_profile_set(
    int                                 unit,
    uint8                          profile_indx,
    const SOC_PPC_OAM_ETH1731_MEP_PROFILE_ENTRY     *eth1731_profile
  )
{
    int rv;
    soc_reg_above_64_val_t profile_entry;
    SOCDNX_INIT_FUNC_DEFS;

    SOC_REG_ABOVE_64_CLEAR(profile_entry);


    soc_OAMP_MEP_PROFILEm_field32_set(unit,profile_entry, DMM_RATEf, eth1731_profile->dmm_rate);
    soc_OAMP_MEP_PROFILEm_field32_set(unit,profile_entry, DMM_OFFSETf, eth1731_profile->dmm_offset);
    soc_OAMP_MEP_PROFILEm_field32_set(unit,profile_entry, DMR_OFFSETf, eth1731_profile->dmr_offset);

    soc_OAMP_MEP_PROFILEm_field32_set(unit, profile_entry, LMM_RATEf, eth1731_profile->lmm_rate);
    soc_OAMP_MEP_PROFILEm_field32_set(unit, profile_entry, LMM_OFFSETf, eth1731_profile->lmm_offset);
    soc_OAMP_MEP_PROFILEm_field32_set(unit, profile_entry, LMR_OFFSETf, eth1731_profile->lmr_offset);
    
    soc_OAMP_MEP_PROFILEm_field32_set(unit, profile_entry, PIGGYBACK_LMf, eth1731_profile->piggy_back_lm);

    soc_OAMP_MEP_PROFILEm_field32_set(unit, profile_entry, RDI_GEN_METHODf, eth1731_profile->rdi_gen_method);

    soc_OAMP_MEP_PROFILEm_field32_set(unit, profile_entry, LMM_DA_OUI_PROFILEf, eth1731_profile->lmm_da_oui_prof);

    soc_OAMP_MEP_PROFILEm_field32_set(unit, profile_entry, OPCODE_0_RATEf, eth1731_profile->opcode_0_rate);
    soc_OAMP_MEP_PROFILEm_field32_set(unit, profile_entry, OPCODE_1_RATEf, eth1731_profile->opcode_1_rate);

    if (SOC_IS_QAX(unit)) {
        /* profile.report_mode is 2 bit bitmap that indicate which report is active:
           ReportMode(2b) = |DM|LM|
           00 - None    01 - LM     10 - DM     11-Both
            * In order to check the LM use 2b'01 mask.
            * In order to check the DM use 2b'10 mask
        */
        soc_OAMP_MEP_PROFILEm_field32_set(unit, profile_entry, REPORT_MODE_DMf, (eth1731_profile->report_mode & 2) ? 1 : 0);
        soc_OAMP_MEP_PROFILEm_field32_set(unit, profile_entry, REPORT_MODE_LMf, (eth1731_profile->report_mode & 1) ? 1 : 0);
        /* For 48 byte MAID */
        soc_OAMP_MEP_PROFILEm_field32_set(unit, profile_entry, MAID_CHECK_DISf, eth1731_profile->maid_check_dis);
		
        if (soc_property_suffix_num_get(unit, -1, spn_CUSTOM_FEATURE, "oam_over_mpls_ignore_mdl", 0)) {
            /* When the above is set the OAMP may not preform MDL validity checks. This is somewhat redundant anyways.*/
            soc_OAMP_MEP_PROFILEm_field32_set(unit, profile_entry, MDL_CHECK_DISf, 1);
        }
    }
    else {
        /* In Jericho the report_mode is global for the entry.
           Once the report is set, its set for the LM and DM. */
        soc_OAMP_MEP_PROFILEm_field32_set(unit, profile_entry, REPORT_MODEf, ((eth1731_profile->report_mode & 3) == 3) ? 1 : 0);
    }
    soc_OAMP_MEP_PROFILEm_field32_set(unit, profile_entry, SLM_LMf, eth1731_profile->slm_lm);

    soc_OAMP_MEP_PROFILEm_field32_set(unit, profile_entry, OPCODE_BMAPf, 0xff);/* We can do without this duplicity*/


    rv = WRITE_OAMP_MEP_PROFILEm(unit, MEM_BLOCK_ANY,profile_indx, profile_entry );
    SOCDNX_IF_ERR_EXIT(rv);

    /* set the phase as well*/
    SOC_REG_ABOVE_64_CLEAR(profile_entry);
    soc_OAMP_MEP_SCAN_PROFILEm_field32_set(unit,profile_entry,DMM_CNTf,eth1731_profile->dmm_cnt );
    soc_OAMP_MEP_SCAN_PROFILEm_field32_set(unit,profile_entry,LMM_CNTf,eth1731_profile->lmm_cnt );
    soc_OAMP_MEP_SCAN_PROFILEm_field32_set(unit,profile_entry,CCM_CNTf, eth1731_profile->ccm_cnt);
    soc_OAMP_MEP_SCAN_PROFILEm_field32_set(unit,profile_entry,OP_0_CNTf,eth1731_profile->op_0_cnt );
    soc_OAMP_MEP_SCAN_PROFILEm_field32_set(unit,profile_entry,OP_1_CNTf,eth1731_profile->op_1_cnt );
    rv = WRITE_OAMP_MEP_SCAN_PROFILEm(unit,MEM_BLOCK_ANY,profile_indx, profile_entry);
    SOCDNX_IF_ERR_EXIT(rv);

exit:
    SOCDNX_FUNC_RETURN;
}

/* ****************************************************************************/
/*****************************************************************************/

soc_error_t soc_jer_pp_oam_oamp_eth1731_profile_get(
    int                                 unit,
    uint8                          profile_indx,
    SOC_PPC_OAM_ETH1731_MEP_PROFILE_ENTRY     *eth1731_profile
  )
{
    int rv;
    soc_reg_above_64_val_t profile_entry;
    SOCDNX_INIT_FUNC_DEFS;

    rv = READ_OAMP_MEP_PROFILEm(unit, MEM_BLOCK_ANY,profile_indx, profile_entry );
    SOCDNX_IF_ERR_EXIT(rv);

    eth1731_profile->dmm_offset  = soc_OAMP_MEP_PROFILEm_field32_get(unit, profile_entry, DMM_OFFSETf);
    eth1731_profile->dmr_offset  = soc_OAMP_MEP_PROFILEm_field32_get(unit, profile_entry, DMR_OFFSETf);
    eth1731_profile->dmm_rate  = soc_OAMP_MEP_PROFILEm_field32_get(unit, profile_entry, DMM_RATEf);

    eth1731_profile->lmm_offset  = soc_OAMP_MEP_PROFILEm_field32_get(unit, profile_entry, LMM_OFFSETf);
    eth1731_profile->lmr_offset  = soc_OAMP_MEP_PROFILEm_field32_get(unit, profile_entry, LMR_OFFSETf);
    eth1731_profile->lmm_rate  = soc_OAMP_MEP_PROFILEm_field32_get(unit, profile_entry, LMM_RATEf); 
    eth1731_profile->piggy_back_lm  = soc_OAMP_MEP_PROFILEm_field32_get(unit, profile_entry, PIGGYBACK_LMf); 
    eth1731_profile->slm_lm  = soc_OAMP_MEP_PROFILEm_field32_get(unit, profile_entry, SLM_LMf); 


    eth1731_profile->rdi_gen_method  = soc_OAMP_MEP_PROFILEm_field32_get(unit, profile_entry, RDI_GEN_METHODf); 

    eth1731_profile->lmm_da_oui_prof  = soc_OAMP_MEP_PROFILEm_field32_get(unit, profile_entry, LMM_DA_OUI_PROFILEf);

    if (SOC_IS_QAX(unit)) {
        /* profile.report_mode is 2 bit bitmap that indicate which report is active:
           ReportMode(2b) = |DM|LM|
           00 - None    01 - LM     10 - DM     11-Both
           In QAX all of the 4 combinations are possible.
            * In order to enable the LM reports, set the 1-st bit to "1"
            * In order to enable the DM reports, set the 2-nd bit to "1"
        */
        eth1731_profile->report_mode = soc_OAMP_MEP_PROFILEm_field32_get(unit, profile_entry, REPORT_MODE_LMf);
        eth1731_profile->report_mode |= (soc_OAMP_MEP_PROFILEm_field32_get(unit, profile_entry, REPORT_MODE_DMf) << 1);
    }
    else {
        /* In Jericho the report_mode is global for the entry, so only the "11" case is possible. */
        eth1731_profile->report_mode = soc_OAMP_MEP_PROFILEm_field32_get(unit, profile_entry, REPORT_MODEf) ? 3 : 0;
    }

    SOC_REG_ABOVE_64_CLEAR(profile_entry);

exit:
    SOCDNX_FUNC_RETURN;
}

/* ****************************************************************************/
/*****************************************************************************/


soc_error_t soc_jer_pp_oam_init_eci_tod(
   int                                 unit,
   uint8                                init_ntp,
   uint8                                init_1588
   )
{
    SOCDNX_INIT_FUNC_DEFS;

    /** NTP  */
    if (init_ntp) {
        /* write 0 to control register */
        SOCDNX_IF_ERR_EXIT(soc_reg_above_64_field32_modify(unit, ECI_GP_CONTROL_6r, REG_PORT_ANY, 0, TOD_NTP_EVENT_MUX_CONTROLf,  0));
        SOCDNX_IF_ERR_EXIT(soc_reg_above_64_field32_modify(unit, ECI_GP_CONTROL_8r, REG_PORT_ANY, 0, TOD_NTP_COUNT_ENABLEf,  0));
        SOCDNX_IF_ERR_EXIT(soc_reg_above_64_field32_modify(unit, ECI_GP_CONTROL_8r, REG_PORT_ANY, 0, TOD_NTP_LOAD_ENABLEf,  0));
        /* write value to frac sec lower register */
        SOCDNX_IF_ERR_EXIT(soc_reg_above_64_field32_modify(unit, ECI_GP_CONTROL_6r, REG_PORT_ANY, 0, TOD_NTP_TIME_FRAC_SEC_LOWERf,  0x13576543));
        /* write value to frac sec upper register */
        SOCDNX_IF_ERR_EXIT(soc_reg_above_64_field32_modify(unit, ECI_GP_CONTROL_6r, REG_PORT_ANY, 0, TOD_NTP_TIME_FRAC_SEC_UPPERf,  0x1ffff00));
        /* write value to frequency register (4 nS in binary fraction) */
        SOCDNX_IF_ERR_EXIT(soc_reg_above_64_field32_modify(unit, ECI_GP_CONTROL_6r, REG_PORT_ANY, 0, TOD_NTP_TIME_FREQ_CONTROLf,  0x44b82fa1));
        /* write value to time sec register  */
        SOCDNX_IF_ERR_EXIT(soc_reg_above_64_field32_modify(unit, ECI_GP_CONTROL_6r, REG_PORT_ANY, 0, TOD_NTP_ONE_SECf, 1));
        SOCDNX_IF_ERR_EXIT(soc_reg_above_64_field32_modify(unit, ECI_GP_CONTROL_6r, REG_PORT_ANY, 0, TOD_NTP_UPPER_SECf, 0x0804560));
        /* write to control register to load values */
        SOCDNX_IF_ERR_EXIT(soc_reg_above_64_field32_modify(unit, ECI_GP_CONTROL_8r, REG_PORT_ANY, 0, TOD_NTP_LOAD_ONCEf,  1));
        /* write to control register to enable counter */
        SOCDNX_IF_ERR_EXIT(soc_reg_above_64_field32_modify(unit, ECI_GP_CONTROL_8r, REG_PORT_ANY, 0, TOD_NTP_COUNT_ENABLEf,  1));
    }

    /** IEEE 1588 */
    if (init_1588) {
        /* write 0 to control register */
        SOCDNX_IF_ERR_EXIT(soc_reg_above_64_field32_modify(unit, ECI_GP_CONTROL_7r, REG_PORT_ANY, 0, TOD_IEEE_1588_EVENT_MUX_CONTROLf,  0));
        SOCDNX_IF_ERR_EXIT(soc_reg_above_64_field32_modify(unit, ECI_GP_CONTROL_8r, REG_PORT_ANY, 0, TOD_IEEE_1588_COUNT_ENABLEf,  0));
        SOCDNX_IF_ERR_EXIT(soc_reg_above_64_field32_modify(unit, ECI_GP_CONTROL_8r, REG_PORT_ANY, 0, TOD_IEEE_1588_LOAD_ENABLEf,  0));
        /* write value to frac sec lower register */
        SOCDNX_IF_ERR_EXIT(soc_reg_above_64_field32_modify(unit, ECI_GP_CONTROL_7r, REG_PORT_ANY, 0, TOD_IEEE_1588_TIME_FRAC_SEC_LOWERf,  0x13576543));
        /* write value to frac sec upper register */
        SOCDNX_IF_ERR_EXIT(soc_reg_above_64_field32_modify(unit, ECI_GP_CONTROL_7r, REG_PORT_ANY, 0, TOD_IEEE_1588_TIME_FRAC_SEC_UPPERf,  0x1ffff00));
        /* write value to frequency register (4 nS) */
        SOCDNX_IF_ERR_EXIT(soc_reg_above_64_field32_modify(unit, ECI_GP_CONTROL_7r, REG_PORT_ANY, 0, TOD_IEEE_1588_TIME_FREQ_CONTROLf,  0x10000000));
        /* write value to time sec register  */
        SOCDNX_IF_ERR_EXIT(soc_reg_above_64_field32_modify(unit, ECI_GP_CONTROL_7r, REG_PORT_ANY, 0, TOD_IEEE_1588_ONE_SECf, 1));
        SOCDNX_IF_ERR_EXIT(soc_reg_above_64_field32_modify(unit, ECI_GP_CONTROL_7r, REG_PORT_ANY, 0, TOD_IEEE_1588_UPPER_SECf, 0x0804560));
        /* write to control register to load values */
        SOCDNX_IF_ERR_EXIT(soc_reg_above_64_field32_modify(unit, ECI_GP_CONTROL_8r, REG_PORT_ANY, 0, TOD_IEEE_1588_LOAD_ONCEf,  1));
        /* write to control register to enable counter */
        SOCDNX_IF_ERR_EXIT(soc_reg_above_64_field32_modify(unit, ECI_GP_CONTROL_8r, REG_PORT_ANY, 0, TOD_IEEE_1588_COUNT_ENABLEf,  1));
    }

exit:
    SOCDNX_FUNC_RETURN;
}


uint32 soc_jer_pp_oam_tod_set(
   int                                 unit,
   uint8                               is_ntp,
   uint64                              data
   )
{
    uint32 time_frac = COMPILER_64_LO(data);
    uint32 time_sec = COMPILER_64_HI(data);
    uint32 tmp;
    uint32 res = SOC_SAND_OK;

    SOC_SAND_INIT_ERROR_DEFINITIONS(0);

    /** NTP  */
    if (is_ntp) {
        /* write to control register. Turn on load enable, Turn off count enable */
        SOC_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res, 10, exit, ARAD_REG_ACCESS_ERR, soc_reg_above_64_field32_modify(unit, ECI_GP_CONTROL_6r, REG_PORT_ANY, 0, TOD_NTP_EVENT_MUX_CONTROLf,  0));
        SOC_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res, 10, exit, ARAD_REG_ACCESS_ERR, soc_reg_above_64_field32_modify(unit, ECI_GP_CONTROL_8r, REG_PORT_ANY, 0, TOD_NTP_LOAD_ENABLEf,  1));
        SOC_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res, 10, exit, ARAD_REG_ACCESS_ERR, soc_reg_above_64_field32_modify(unit, ECI_GP_CONTROL_8r, REG_PORT_ANY, 0, TOD_NTP_COUNT_ENABLEf,  0));
        /* write value to frac sec lower register */
        SHR_BITCOPY_RANGE(&tmp, 26, &time_frac, 0, 6);
        SOC_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res, 10, exit, ARAD_REG_ACCESS_ERR, soc_reg_above_64_field32_modify(unit, ECI_GP_CONTROL_6r, REG_PORT_ANY, 0, TOD_NTP_TIME_FRAC_SEC_LOWERf,  tmp));
        /* write value to frac sec upper register */
        tmp=0;
        SHR_BITCOPY_RANGE(&tmp, 0, &time_frac, 6, 26);
        SOC_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res, 10, exit, ARAD_REG_ACCESS_ERR, soc_reg_above_64_field32_modify(unit, ECI_GP_CONTROL_6r, REG_PORT_ANY, 0, TOD_NTP_TIME_FRAC_SEC_UPPERf,  tmp));
        /* write value to frequency register (4 nS in binary fraction) */
        SOC_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res, 10, exit, ARAD_REG_ACCESS_ERR, soc_reg_above_64_field32_modify(unit, ECI_GP_CONTROL_6r, REG_PORT_ANY, 0, TOD_NTP_TIME_FREQ_CONTROLf,  0x44b82fa1));
        /* write value to time sec register  */
        tmp=0;
        SHR_BITCOPY_RANGE(&tmp, 0, &time_sec, 1, 31);
        SOC_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res, 10, exit, ARAD_REG_ACCESS_ERR, soc_reg_above_64_field32_modify(unit, ECI_GP_CONTROL_6r, REG_PORT_ANY, 0, TOD_NTP_ONE_SECf, (time_sec & 0x00000001)!=0 ));
        SOC_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res, 10, exit, ARAD_REG_ACCESS_ERR, soc_reg_above_64_field32_modify(unit, ECI_GP_CONTROL_6r, REG_PORT_ANY, 0, TOD_NTP_UPPER_SECf, tmp));
        /* write to control register to load values */
        SOC_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res, 10, exit, ARAD_REG_ACCESS_ERR, soc_reg_above_64_field32_modify(unit, ECI_GP_CONTROL_8r, REG_PORT_ANY, 0, TOD_NTP_LOAD_ONCEf,  1));
        /* write to control register .Turn off load enable, Turn on count enable */
        SOC_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res, 10, exit, ARAD_REG_ACCESS_ERR, soc_reg_above_64_field32_modify(unit, ECI_GP_CONTROL_8r, REG_PORT_ANY, 0, TOD_NTP_LOAD_ENABLEf,  0));
        SOC_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res, 10, exit, ARAD_REG_ACCESS_ERR, soc_reg_above_64_field32_modify(unit, ECI_GP_CONTROL_8r, REG_PORT_ANY, 0, TOD_NTP_COUNT_ENABLEf,  1));
    }
    else {/** IEEE 1588 */
        /* write to control register. Turn on load enable, Turn off count enable */
        SOC_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res, 10, exit, ARAD_REG_ACCESS_ERR, soc_reg_above_64_field32_modify(unit, ECI_GP_CONTROL_7r, REG_PORT_ANY, 0, TOD_IEEE_1588_EVENT_MUX_CONTROLf,  0));
        SOC_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res, 10, exit, ARAD_REG_ACCESS_ERR, soc_reg_above_64_field32_modify(unit, ECI_GP_CONTROL_8r, REG_PORT_ANY, 0, TOD_IEEE_1588_LOAD_ENABLEf,  1));
        SOC_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res, 10, exit, ARAD_REG_ACCESS_ERR, soc_reg_above_64_field32_modify(unit, ECI_GP_CONTROL_8r, REG_PORT_ANY, 0, TOD_IEEE_1588_COUNT_ENABLEf,  0));
        /* write value to frac sec lower register */
        SHR_BITCOPY_RANGE(&tmp, 26, &time_frac, 0, 6);
        SOC_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res, 10, exit, ARAD_REG_ACCESS_ERR, soc_reg_above_64_field32_modify(unit, ECI_GP_CONTROL_7r, REG_PORT_ANY, 0, TOD_IEEE_1588_TIME_FRAC_SEC_LOWERf,  tmp));
        /* write value to frac sec upper register */
        tmp=0;
        SHR_BITCOPY_RANGE(&tmp, 0, &time_frac, 6, 26);
        SOC_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res, 10, exit, ARAD_REG_ACCESS_ERR, soc_reg_above_64_field32_modify(unit, ECI_GP_CONTROL_7r, REG_PORT_ANY, 0, TOD_IEEE_1588_TIME_FRAC_SEC_UPPERf,  tmp));
        /* write value to frequency register (4 nS) */
        SOC_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res, 10, exit, ARAD_REG_ACCESS_ERR, soc_reg_above_64_field32_modify(unit, ECI_GP_CONTROL_7r, REG_PORT_ANY, 0, TOD_IEEE_1588_TIME_FREQ_CONTROLf,  0x10000000));
        /* write value to time sec register  */
        tmp=0;
        SHR_BITCOPY_RANGE(&tmp, 0, &time_sec, 1, 31);
        SOC_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res, 10, exit, ARAD_REG_ACCESS_ERR, soc_reg_above_64_field32_modify(unit, ECI_GP_CONTROL_7r, REG_PORT_ANY, 0, TOD_IEEE_1588_ONE_SECf, (time_sec & 0x00000001)!=0 ));
        SOC_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res, 10, exit, ARAD_REG_ACCESS_ERR, soc_reg_above_64_field32_modify(unit, ECI_GP_CONTROL_7r, REG_PORT_ANY, 0, TOD_IEEE_1588_UPPER_SECf, tmp));
        /* write to control register to load values */
        SOC_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res, 10, exit, ARAD_REG_ACCESS_ERR, soc_reg_above_64_field32_modify(unit, ECI_GP_CONTROL_8r, REG_PORT_ANY, 0, TOD_IEEE_1588_LOAD_ONCEf,  1));
        /* write to control register .Turn off load enable, Turn on count enable */
        SOC_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res, 10, exit, ARAD_REG_ACCESS_ERR, soc_reg_above_64_field32_modify(unit, ECI_GP_CONTROL_8r, REG_PORT_ANY, 0, TOD_IEEE_1588_LOAD_ENABLEf,  0));
        SOC_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res, 10, exit, ARAD_REG_ACCESS_ERR, soc_reg_above_64_field32_modify(unit, ECI_GP_CONTROL_8r, REG_PORT_ANY, 0, TOD_IEEE_1588_COUNT_ENABLEf,  1));
    }

exit:
    SOC_SAND_EXIT_AND_SEND_ERROR("error in jer_pp_oam_tod_set()", 0, 0);
}


soc_error_t soc_jer_pp_oam_sa_addr_msbs_set(
    int unit,
    int profile,
   const uint8 * msbs)
{
    soc_reg_above_64_val_t reg_above_64, field_above_64 = {0};
    soc_field_t base_mac_sa_profiles[] = {BASE_MAC_SA_PROFILE_0f, BASE_MAC_SA_PROFILE_1f };
    int rv;
    SOCDNX_INIT_FUNC_DEFS;

    if (profile < 0 || profile >= 2) {
        SOCDNX_EXIT_WITH_ERR(SOC_E_INTERNAL,  (_BSL_SOCDNX_MSG("Internal error: incorrect SA profile.")));
    }
    
    rv = READ_OAMP_CCM_MAC_SAr(unit, reg_above_64);
    SOCDNX_IF_ERR_EXIT(rv);
    field_above_64[0] = (msbs[1] << 24) + (msbs[2] << 16) + (msbs[3] << 8) + msbs[4];
    field_above_64[1] = msbs[0];

    soc_reg_above_64_field_set(unit, OAMP_CCM_MAC_SAr, reg_above_64, base_mac_sa_profiles[profile], field_above_64);

    rv = WRITE_OAMP_CCM_MAC_SAr(unit, reg_above_64);
    SOCDNX_IF_ERR_EXIT(rv); 

exit:
    SOCDNX_FUNC_RETURN;
}


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
    uint8 * msbs)
{
    soc_reg_above_64_val_t reg_above_64= {0}, field_above_64 = {0};
    soc_field_t base_mac_sa_profiles[] = { BASE_MAC_SA_PROFILE_0f, BASE_MAC_SA_PROFILE_1f }; 
    int rv;
    SOCDNX_INIT_FUNC_DEFS;

    if (profile < 0 || profile >= 2) {
        SOCDNX_EXIT_WITH_ERR(SOC_E_INTERNAL,  (_BSL_SOCDNX_MSG("Internal error: incorrect SA profile."))); 
    }

    rv = READ_OAMP_CCM_MAC_SAr(unit, reg_above_64);
    SOCDNX_IF_ERR_EXIT(rv);

    soc_reg_above_64_field_get(unit, OAMP_CCM_MAC_SAr, reg_above_64, base_mac_sa_profiles[profile], field_above_64);

    msbs[1] = field_above_64[0] >>24;
    msbs[2] = (field_above_64[0] >>16) & 0xff;
    msbs[3] = (field_above_64[0] >>8) & 0xff;
    msbs[4] = field_above_64[0] & 0xff;
    msbs[0] = field_above_64[1];


exit:
    SOCDNX_FUNC_RETURN;
}


soc_error_t soc_jer_pp_oam_dm_trigger_set(
   int unit,
   int endpoint_id)
{
    uint32 reg32;
    soc_field_t dm_trigger_field[] = { DM_TRIGER_0f, DM_TRIGER_1f, DM_TRIGER_2f, DM_TRIGER_3f,
    DM_TRIGER_4f, DM_TRIGER_5f,DM_TRIGER_6f, DM_TRIGER_7f}; 
    int rv;
    uint32 write_val = 1;
    int table_entry, reg_field;
    SOCDNX_INIT_FUNC_DEFS;

    table_entry = endpoint_id /8;
    reg_field = endpoint_id %8;
    SOCDNX_IF_ERR_EXIT(WRITE_OAMP_ENABLE_DYNAMIC_MEMORY_ACCESSr(unit , write_val));

    soc_OAMP_DM_TRIGERm_field32_set(unit,&reg32, dm_trigger_field[reg_field], 1);

    rv = WRITE_OAMP_DM_TRIGERm(unit, MEM_BLOCK_ANY,table_entry, &reg32 );
    SOCDNX_IF_ERR_EXIT(rv); 
	write_val = 0;
	SOCDNX_IF_ERR_EXIT(WRITE_OAMP_ENABLE_DYNAMIC_MEMORY_ACCESSr(unit , write_val));

exit:
    SOCDNX_FUNC_RETURN;

}


/**
 * Set the opcode info register. 
 * At this stage these are only configured to transmit AIS 
 * frames, one at period one second and another at period  one 
 * minute. 
 * 
 * @author sinai (13/07/2014)
 * 
 * @param unit 
 * 
 * @return soc_error_t 
 */
STATIC soc_error_t _soc_jer_pp_oam_set_opcode_n_table(int unit)
{
    int rv;
    soc_reg_above_64_val_t reg_above_64;
    SOCDNX_INIT_FUNC_DEFS;

    /* AIS frame with period one second*/
    SOC_REG_ABOVE_64_CLEAR(reg_above_64);
    soc_reg_above_64_field32_set(unit, OAMP_OPCODE_INFOr, reg_above_64, OPCODE_N_OPCODEf, SOC_PPC_OAM_ETHERNET_PDU_OPCODE_AIS);
    soc_reg_above_64_field32_set(unit, OAMP_OPCODE_INFOr, reg_above_64, OPCODE_N_VERSIONf, 0);
    soc_reg_above_64_field32_set(unit, OAMP_OPCODE_INFOr, reg_above_64, OPCODE_N_FLAGSf, 0x4); /* flag indicating 1 frame per second*/
    soc_reg_above_64_field32_set(unit, OAMP_OPCODE_INFOr, reg_above_64, OPCODE_N_TLV_OFFSETf, 0x0);
    soc_reg_above_64_field32_set(unit, OAMP_OPCODE_INFOr, reg_above_64, OPCODE_N_TLV_SIZEf, 17); /* This sets the entire PDU size and should be big enough to not be filtered in ingress */
    soc_reg_above_64_field32_set(unit, OAMP_OPCODE_INFOr, reg_above_64, OPCODE_N_TLV_PATTERN_TYPEf, 2); /* 32 bit pattern type */
    soc_reg_above_64_field32_set(unit, OAMP_OPCODE_INFOr, reg_above_64, OPCODE_N_TLV_PATTERNf, 0x03000900); /* TLV Type=3(data) Lenght=9(compatable with OPCODE_N_TLV_SIZEf) */

    rv = WRITE_OAMP_OPCODE_INFOr(unit,SOC_PPC_OAM_AIS_PERIOD_ONE_SECOND_OPCODE_ENTRY,reg_above_64 );
    SOCDNX_IF_ERR_EXIT(rv); 

    /* AIS frame with period one minute. All values except Flags identical to those above.*/
    soc_reg_above_64_field32_set(unit, OAMP_OPCODE_INFOr, reg_above_64, OPCODE_N_FLAGSf, 0x6); /* flag indicating 1 frame per minute*/
    rv = WRITE_OAMP_OPCODE_INFOr(unit,SOC_PPC_OAM_AIS_PERIOD_ONE_MINUTE_OPCODE_ENTRY,reg_above_64 );
    SOCDNX_IF_ERR_EXIT(rv); 

exit:
    SOCDNX_FUNC_RETURN;
}


soc_error_t soc_jer_pp_oam_egress_pcp_set_by_profile_and_tc(int unit, uint8 tc, uint8 outlif_profile,uint8 oam_pcp){
    int rv;
    SOCDNX_INIT_FUNC_DEFS;


    JER_PP_OAM_SET_EGRESS_OAM_PCP_BY_OUTLIF_PROFILE_AND_TC(outlif_profile, tc, oam_pcp);


exit:
    SOCDNX_FUNC_RETURN;
}

/* Adds static entries to OAM1, must be done after oam_init because trap codes are being used 
   Required only for OAM (not BFD) */
soc_error_t soc_jer_pp_oam_classifier_oam1_entries_add(int unit) {
    int rv;
    SOCDNX_INIT_FUNC_DEFS;

    rv = soc_jer_pp_oam_classifier_oam1_passive_entries_add(unit);
    SOCDNX_IF_ERR_EXIT(rv);

exit:
    SOCDNX_FUNC_RETURN;
}

/**
 * All Passive / below_highest / between_meps (QAX) entries are
 * configured statically. In these cases all other values the
 * output of the LIF-Action table is the same.
 *
 * @author sinai (25/11/2014)
 *
 * @param unit
 *
 * @return soc_error_t
 */
STATIC
soc_error_t soc_jer_pp_oam_classifier_oam1_passive_entries_add(int unit) {
    int rv;
    int opcode_profile, ingress,my_cfm_mac,  mp_profile;
    SOC_PPC_OAM_CLASSIFIER_OAM1_ENTRY_KEY    oam1_key;
    SOC_PPC_OAM_CLASSIFIER_OAM_ENTRY_PAYLOAD oam_payload;
    uint32 oam_mirror_profile_passive, oam_mirror_profile_level;
    uint32 oam_trap_code_passive, oam_trap_code_level;
    uint32 soc_sand_rv;
    _oam_oam_a_b_table_buffer_t oama_buffer;

    SOCDNX_INIT_FUNC_DEFS;

    soc_sand_rv = arad_pp_oam_classifier_oam1_allocate_sw_buffer(unit,&oama_buffer);
    SOCDNX_SAND_IF_ERR_RETURN(soc_sand_rv);

    SOC_PPC_OAM_CLASSIFIER_OAM1_ENTRY_KEY_clear(&oam1_key);
    SOC_PPC_OAM_CLASSIFIER_OAM_ENTRY_PAYLOAD_clear(&oam_payload);

    oam1_key.inject = 0;
    oam1_key.is_bfd = 0;

    /* payload is the same */
    oam_payload.meter_disable = 1;
    oam_payload.forward_disable = 1; /* default */
    oam_payload.counter_disable = 1;
    oam_payload.snoop_strength = 0;
    oam_payload.sub_type = soc_property_suffix_num_get(unit, -1, spn_CUSTOM_FEATURE,
                                                       "oam_additional_FTMH_on_error_packets", 0) ?
                                _ARAD_PP_OAM_SUBTYPE_CCM : _ARAD_PP_OAM_SUBTYPE_DEFAULT;

    /* Egress*/
    rv = sw_state_access[unit].dpp.soc.arad.pp.oam.mirror_profile_err_passive.get(unit, &oam_mirror_profile_passive);
    SOCDNX_IF_ERR_EXIT(rv);
    rv = sw_state_access[unit].dpp.soc.arad.pp.oam.mirror_profile_err_level.get(unit, &oam_mirror_profile_level);
    SOCDNX_IF_ERR_EXIT(rv);

    /* Ingress*/
    rv = sw_state_access[unit].dpp.soc.arad.pp.oam.trap_code_trap_to_cpu_passive.get(unit, &oam_trap_code_passive);
    SOCDNX_IF_ERR_EXIT(rv);
    rv = sw_state_access[unit].dpp.soc.arad.pp.oam.trap_code_trap_to_cpu_level.get(unit, &oam_trap_code_level);
    SOCDNX_IF_ERR_EXIT(rv);

    for (ingress=0; ingress<=1; ++ingress) {
        for (my_cfm_mac=0; my_cfm_mac<=1; ++my_cfm_mac) {
            /* Skip opcode=0, otherwise data packets may not be counted because counter_disable is set to 1 */
            for (opcode_profile=SOC_PPC_OAM_OPCODE_MAP_CCM; opcode_profile<SOC_PPC_OAM_OPCODE_MAP_COUNT; ++opcode_profile) {
                for (mp_profile=0; mp_profile < SOC_PPC_OAM_NON_ACC_PROFILES_ARAD_PLUS_NUM; ++mp_profile) {
                    /* set the key according to all the options*/
                    oam1_key.my_cfm_mac = my_cfm_mac;
                    oam1_key.opcode = opcode_profile;
                    oam1_key.mp_profile = mp_profile;
                    oam1_key.ingress = ingress;
                    
                    oam_payload.forwarding_strength = 7; /* default */
                    /* Now the payload */
                    if (ingress) {
                        oam_payload.up_map = 1; /* Passive trap: Up at the ingress, Down at the egress.*/
                        oam_payload.forwarding_strength = _ARAD_PP_OAM_TRAP_STRENGTH;
                    }

                    oam_payload.mirror_enable = !ingress;

                    /* Passive entries */
                    oam1_key.mp_type_jr = SOC_PPC_OAM_MP_TYPE_JERICHO_PASSIVE_MATCH;
                    oam_payload.cpu_trap_code = oam_trap_code_passive;
                    oam_payload.mirror_profile = oam_mirror_profile_passive & 0xff;
                    oam_payload.forwarding_strength = _ARAD_PP_OAM_PASSIVE_TRAP_STRENGTH;
                    soc_sand_rv = arad_pp_oam_classifier_oam1_entry_set_on_buffer(unit, &oam1_key, &oam_payload, &oama_buffer);
                    SOCDNX_SAND_IF_ERR_RETURN(soc_sand_rv);

                    /* Low level entries */
                    oam1_key.mp_type_jr = SOC_IS_JERICHO_PLUS(unit) ? SOC_PPC_OAM_MP_TYPE_QAX_BELOW_ALL : SOC_PPC_OAM_MP_TYPE_JERICHO_BELLOW_HIGHEST_MEP;
                    oam_payload.cpu_trap_code = oam_trap_code_level;
                    oam_payload.forwarding_strength = _ARAD_PP_OAM_LEVEL_TRAP_STRENGTH;
                    oam_payload.mirror_profile = oam_mirror_profile_level & 0xff;

                    soc_sand_rv = arad_pp_oam_classifier_oam1_entry_set_on_buffer(unit, &oam1_key, &oam_payload, &oama_buffer);
                    SOCDNX_SAND_IF_ERR_RETURN(soc_sand_rv);

                    if (SOC_IS_JERICHO_PLUS(unit)) {
                        /* Need to drop between MEPs as well  */
                        oam1_key.mp_type_jr = SOC_PPC_OAM_MP_TYPE_QAX_BETWEEN_MEPS;
                        oam_payload.counter_disable = 0; /* Count this packet for the lower MEP */
                        soc_sand_rv = arad_pp_oam_classifier_oam1_entry_set_on_buffer(unit, &oam1_key, &oam_payload, &oama_buffer);
                        SOCDNX_SAND_IF_ERR_EXIT(soc_sand_rv);

                        oam_payload.counter_disable = 1; /* Set it back to 1 */
                    }
                }
            }
        }
    }

    soc_sand_rv = arad_pp_oam_classifier_oam1_set_hw_unsafe(unit, &oama_buffer);
    SOCDNX_SAND_IF_ERR_RETURN(soc_sand_rv);

exit:
    arad_pp_oam_classifier_oam1_2_buffer_free(unit,&oama_buffer);
    SOCDNX_FUNC_RETURN;
}


/**
 * Respond to LMM, DMM, LTM, LBM and EXM, SLM packets with the
 * respective response packets.
 *
 * @author sinai (03/03/2015)
 *
 * @param unit
 *
 * @return STATIC soc_error_t
 */
STATIC
soc_error_t soc_jer_pp_oam_init_response_table(int unit) {
      int rv;
      soc_reg_above_64_val_t reg_above_64={0};
      uint32 data=0, key=0, oam_ts_format ;
      int valid_opcodes[] = {
          SOC_PPC_OAM_ETHERNET_PDU_OPCODE_LBM,
          SOC_PPC_OAM_ETHERNET_PDU_OPCODE_LTM,
          SOC_PPC_OAM_ETHERNET_PDU_OPCODE_EXM };
      int i;
      SOCDNX_INIT_FUNC_DEFS;

      for (i = 0; i < sizeof(valid_opcodes) / sizeof(int); ++i) {
          oam_ts_format = RSP_TCAM_OPCODE_TO_OAM_TS_FORMAT(valid_opcodes[i]);
          RSP_TCAM_KEY_SET(key, 1, valid_opcodes[i]);
          RSP_TCAM_DATA_SET(data, 1,oam_ts_format , valid_opcodes[i] -1 ); /* The response message's opcode is always one below of that of the original packet.*/

          soc_mem_field32_set(unit,OAMP_RSP_TCAMm,reg_above_64,DATf,data);
          soc_mem_field32_set(unit,OAMP_RSP_TCAMm,reg_above_64,KEYf,key);
          soc_mem_field32_set(unit,OAMP_RSP_TCAMm,reg_above_64,VALIDf,1);
          soc_mem_field32_set(unit,OAMP_RSP_TCAMm,reg_above_64,MASKf,0);/* Key based on all fields.*/

          rv = WRITE_OAMP_RSP_TCAMm(unit,MEM_BLOCK_ALL,i,reg_above_64);
          SOCDNX_IF_ERR_EXIT(rv);
      }

exit:
    SOCDNX_FUNC_RETURN;
}

soc_error_t soc_jer_pp_oam_mp_type_config_init(int unit) {
    int rv;
    uint32 mp_type_index;
    soc_reg_above_64_val_t reg_above_64;
    uint32 mp_type;

    SOCDNX_INIT_FUNC_DEFS;

    /* MP-type static configuration*/
    /* following encoding is used:
        0 - MIP-Match
        1 - Active-Match
        2 - Passive-Match
        3 - Below-Highest-MEP
        4 - Above-All
        5 - BFD
        table is accessed by: OAM-is-BFD, MDL-MP-Type(2), Above-MDL-MEP-Bitmap-OR
    */

    /* Key/Index | OAM-is-BFD | MDL-MP-Type | Not-Above-MDL-MEP-Bitmap-OR | Maps to MP-type
        0   0000 |     0        00 No-MP                 0                |  100   4 Above-All
        1   0001 |     0        00 No-MP                 1                |  011   3 Below-Highest-MEP
        2   0010 |     0        01 MIP                   0                |  000   0 MIP-Match
        3   0011 |     0        01 MIP                   1                |  000   0
        4   0100 |     0        10 Active-MEP            0                |  001   1 Active-Match
        5   0101 |     0        10 Active-MEP            1                |  001   1
        6   0110 |     0        11 Passive-MEP           0                |  010   2 Passive-Match
        7   0111 |     0        11 Passive-MEP           1                |  010   2
        8   1000 |     1        00 No-MP                 0                |  101   5 BFD
        9   1001 |     1        00 No-MP                 1                |  101   5
        10  1010 |     1        01 MIP                   0                |  101   5
        11  1011 |     1        01 MIP                   1                |  101   5
        12  1100 |     1        10 Active-MEP            0                |  101   5
        13  1101 |     1        10 Active-MEP            1                |  101   5
        14  1110 |     1        11 Passive-MEP           0                |  101   5
        15  1111 |     1        11 Passive-MEP           1                |  101   5
    */

    rv = READ_IHP_OAM_MP_TYPE_MAPr(unit, SOC_CORE_ALL, reg_above_64);
    SOCDNX_IF_ERR_EXIT(rv);

    for (mp_type_index=0; mp_type_index<0x10 /*all combinations of 4 bits*/; ++mp_type_index) {
        if (_JER_PP_OAM_MP_TYPE_MAP_IS_BFD(mp_type_index)) {
            mp_type = SOC_PPC_OAM_MP_TYPE_JERICHO_BFD;
        } else if (_JER_PP_OAM_MP_TYPE_MAP_MDL_MP_TYPE(mp_type_index)==_JER_PP_OAM_MDL_MP_TYPE_NO_MP) {
            mp_type = _JER_PP_OAM_MP_TYPE_MAP_ABOVE_MDL_MEP_BITMAP_OR(mp_type_index) ? SOC_PPC_OAM_MP_TYPE_JERICHO_BELLOW_HIGHEST_MEP :
                SOC_PPC_OAM_MP_TYPE_JERICHO_ABOVE_ALL;
        } else {
            mp_type = _JER_PP_OAM_MP_TYPE_FROM_MDL_MP_TYPE(_JER_PP_OAM_MP_TYPE_MAP_MDL_MP_TYPE(mp_type_index));
        }
        SHR_BITCOPY_RANGE(reg_above_64, (3 * mp_type_index), &mp_type, 0, 3);
    }

    rv = WRITE_IHP_OAM_MP_TYPE_MAPr(unit,SOC_CORE_ALL,reg_above_64);
    SOCDNX_IF_ERR_EXIT(rv);

exit:
    SOCDNX_FUNC_RETURN;
}



/* 
 *   SAT functions
 */ 
 
/* Init SAT appl */
soc_error_t soc_jer_pp_oam_sat_init(int unit)
{
    uint64        reg64;
    uint8 oam_pe_is_init = 0;
    uint8         is_allocated;
    bcm_error_t   rv ;

    SOCDNX_INIT_FUNC_DEFS;

    COMPILER_64_ZERO(reg64);    

    /* clock register set - Number of system clock cycles in 3.33 ms */
    SOCDNX_IF_ERR_EXIT(READ_OAMP_TIMER_CONFIGr(unit, &reg64));
    soc_reg64_field32_set(unit, OAMP_TIMER_CONFIGr, &reg64, NUM_CLOCKS_SECf, arad_chip_kilo_ticks_per_sec_get(unit)*1000);
    /* other fields should remain with default values*/
    SOCDNX_IF_ERR_EXIT(WRITE_OAMP_TIMER_CONFIGr(unit, reg64));

    if(!SOC_WARM_BOOT(unit)) {
        rv = OAM_ACCESS.is_allocated(unit, &is_allocated);
        BCMDNX_IF_ERR_EXIT(rv);

        if(!is_allocated) {
            rv = OAM_ACCESS.alloc(unit);
            BCMDNX_IF_ERR_EXIT(rv);
        }

    }

    /* init programmable editor */
    sw_state_access[unit].dpp.soc.arad.pp.oamp_pe.oamp_pe_init.get(unit, &oam_pe_is_init);
    if(!oam_pe_is_init){
        SOCDNX_IF_ERR_EXIT(arad_pp_oamp_pe_unsafe(unit));
        sw_state_access[unit].dpp.soc.arad.pp.oamp_pe.oamp_pe_init.set(unit, TRUE);
    }

exit:
    SOCDNX_FUNC_RETURN;
}


 /* 
  *  PTCH-2 Header format:
  */
int _insert_ptch2_for_tst_lb(uint8* buffer, int byte_start_offset, 
     uint8 next_header_is_itmh, 
     uint8 opaque_value, 
     uint8 pp_ssp) 
{
     int offset = byte_start_offset;
     int parser_program_ctl = 0;
     parser_program_ctl = next_header_is_itmh ? 0 : 1;
     buffer[offset++] = (parser_program_ctl<<7) | (opaque_value<<4);
     buffer[offset++] = pp_ssp;
     
     return offset;
}

  /* 
  *  Arad-style ITMH Header format:
  */
 
int _insert_itmh_for_tst_lb_arad(uint8* buffer, int byte_start_offset, 
      int fwd_dest_info, 
     uint8 fwd_tc, 
     uint8 fwd_dp, 
     uint8 snoop_cmd, 
     uint8 in_mirr_flag, 
     uint8 pph_type)
{
     int offset = byte_start_offset;

     buffer[offset++] = ((pph_type&0x3)<<6)|((in_mirr_flag&0x1)<<5)|((snoop_cmd&0xf)<<1)|((fwd_tc&0x4)>>2);
     buffer[offset++] = ((fwd_tc&0x3)<<6) | ((fwd_dp&0x3)<<4) | ((fwd_dest_info&0xf0000)>>16);
     buffer[offset++] = (fwd_dest_info&0xff00)>>8;
     buffer[offset++] = (fwd_dest_info&0xff);
     
     return offset;
}
 /* 
  *  Jericho-style ITMH Header format:
  */
 
int _insert_itmh_for_tst_lb_jericho(uint8* buffer, int byte_start_offset, 
     int fwd_dest_info, 
     uint8 fwd_tc, 
     uint8 fwd_dp, 
     uint8 snoop_cmd, 
     uint8 in_mirr_flag, 
     uint8 pph_type, 
	 uint8 extern_flag)
{
     int offset = byte_start_offset;
     int dest_exten = extern_flag;
  
     buffer[offset++] = ((pph_type&0x3)<<6)|((in_mirr_flag&0x1)<<5)|((fwd_dp&0x3)<<3)|((fwd_dest_info&0x70000)>>16);
     buffer[offset++] = ((fwd_dest_info&0xff00)>>8);
     buffer[offset++] = (fwd_dest_info&0xff);
     buffer[offset++] = ((snoop_cmd&0xf)<<4)|((fwd_tc&0x7)<<1)|(dest_exten&0x1);
     
     return offset;
}
int _insert_itmh_extension_for_tst_lb(uint8* buffer, int byte_start_offset, int fwd_dst_info)
{
     int offset = byte_start_offset;
     buffer[offset++] = (fwd_dst_info&0xF0000)>>16;
     buffer[offset++] = (fwd_dst_info&0xff00)>>8;
     buffer[offset++] = (fwd_dst_info&0xFF);
     return offset;
}
int _insert_itmh_extension_jericho(uint8* buffer, int byte_start_offset, uint8 type, int value)
{
     int offset = byte_start_offset;
     buffer[offset++] = (type<<5) | ((value & 0xF0000)>>16);
     buffer[offset++] = (value&0xff00)>>8;
     buffer[offset++] = (value&0xFF);
     return offset;
}

int _insert_eth_for_tst_lb(uint8* buffer, int byte_start_offset,
     bcm_mac_t dest_addr,
     bcm_mac_t src_addr, 
     uint16* tpids,
     bcm_vlan_t* vlans,
     int no_tags,
     uint16 ether_type) 
{
     int i = 0, offset = byte_start_offset; 
     for (i = 0; i < 6; i++) {
         buffer[offset++] = dest_addr[i];
     }
     for (i = 0; i < 6; i++) {
         buffer[offset++] = src_addr[i];
     }
     i = 0;
     while (no_tags) {
         buffer[offset++] = (tpids[i]>>8)&0xff;
         buffer[offset++] = tpids[i]&0xff;
         buffer[offset++] = ((vlans[i]>>8)&0xff) ;/*(vlan_pri[i]<<5)|((vlan_cfi[i]&&0x1)<<4)|((vlans[i]>>8)&0xff);*/
         buffer[offset++] = vlans[i]&0xff;
         --no_tags;
         i++;
     }
     
     buffer[offset++] = (ether_type>>8)&0xff;
     buffer[offset++] = ether_type&0xff;
     
     return offset;
}
 
int _insert_pph_for_tst_lb(uint8* buffer, const PPH_base_head* pph_fields, int byte_start_offset)
{
     int offset = byte_start_offset;
     /* missing fields are always zero anyways.*/
     buffer[offset++] =  ((pph_fields->field1&0x3)<<4)|(pph_fields->field2&0xf) ;
     buffer[offset++] = ((pph_fields->field3&0x7f)<<1)|(pph_fields->field4&0x1);
     buffer[offset++] = ((pph_fields->field5&0x3)<<6)|((pph_fields->field6&0x3)<<4)|((pph_fields->field7&0x1)<<3)
                                             |((pph_fields->field8&0x1)<<2) |((pph_fields->field9&0xC000)>>14);
     buffer[offset++] = ((pph_fields->field9&0x3FC0)>>6);
     buffer[offset++] =((pph_fields->field9&0x7F)<<2)|((pph_fields->field10&0x30000)>>16);
     buffer[offset++] = ((pph_fields->field10&0xff00)>>8);
     buffer[offset++] = (pph_fields->field10&0xff);
     
     return offset;
}
 
int _insert_mpls_label_for_tst_lb(uint8* buffer,  int label, uint8 exp, uint8 bos, uint8 ttl, int byte_start_offset)
{
     int offset = byte_start_offset;
     
     buffer[offset++] = ((label&0xFF000)>>12);
     buffer[offset++] = ((label&0xFF0)>>4);
     buffer[offset++] = ((label&0xF)<<4)|((exp&0x7)<<1)|(bos&0x1);
     buffer[offset++] = ttl;
 
     return offset;
}
 
int _insert_4_bytes_for_tst_lb(uint8* buffer,   uint32 stuff, int byte_start_offset)
{
     int offset = byte_start_offset;
     
     buffer[offset++] = ((stuff&0xFF000000)>>24);
     buffer[offset++] = ((stuff&0xFF0000)>>16);
     buffer[offset++] = ((stuff&0xFF00)>>8);
     buffer[offset++] = stuff&0xFF;
 
     return offset;
}
 
int _insert_oam_pdu_header_for_tst_lb(uint8* buffer, int level,int is_lbm, int byte_start_offset)
{
     int offset = byte_start_offset;
	 
     uint8 opcode = (is_lbm & JER_PP_OAM_LOOPBACK_TST_INFO_FLAGS_LBM) ? SOC_PPC_OAM_ETHERNET_PDU_OPCODE_LBM: 
     SOC_PPC_OAM_ETHERNET_PDU_OPCODE_TST; 
         
     buffer[offset++] = ((level&0x7)<<5);
     buffer[offset++] = opcode;
     buffer[offset++] = 0;/*Flags*/
     buffer[offset++] = 4;/*TLV offset*/
 
     return offset;
}
 
 soc_error_t
 soc_jer_pp_oam_loopback_tst_info_init(SOC_SAND_IN int unit,
    SOC_SAND_INOUT JER_PP_OAM_LOOPBACK_TST_INFO *lb_tst_info)
 {
     uint32 res = SOC_SAND_OK;
     SOC_SAND_INIT_ERROR_DEFINITIONS(0);

     sal_memset(lb_tst_info, 0, sizeof(JER_PP_OAM_LOOPBACK_TST_INFO));
     lb_tst_info->flags = JER_PP_OAM_LOOPBACK_TST_INFO_FLAGS_LBM;
     lb_tst_info->int_pri = -1;
     lb_tst_info->pkt_pri = 0xff;
     lb_tst_info->inner_pkt_pri = 0xff;
     
     JER_PP_DO_NOTHING_AND_EXIT;
exit:
     SOC_SAND_EXIT_AND_SEND_ERROR_SOCDNX((_BSL_SOCDNX_SAND_MSG("Something went wrong")));
 }

soc_error_t 
soc_jer_pp_oam_oamp_lb_tst_header_set (SOC_SAND_IN int unit, 
     SOC_SAND_IN JER_PP_OAM_LOOPBACK_TST_INFO *lb_tst_info,
     SOC_SAND_INOUT uint8* header_buffer, 
     SOC_SAND_OUT int *header_offset)
{
    uint32 res = SOC_SAND_OK;
    SOC_PPC_OAM_OAMP_MEP_DB_ENTRY  mep_db_entry;
    uint32 short_name, entry;
    int next_is_itmh;
    uint32  ssp;
    int offset = 0;
    soc_reg_above_64_val_t      reg_data, reg_field;
    uint8 src_mac_address_msbs[5]={0};
    bcm_port_t   _mode_port = 0;
    uint32       tm_port = 0;
    uint32       _sys_phy_port_id = 0;
    bcm_module_t _modid = 0;
    int core = 0;
    uint32 user_header_0_size = 0, user_header_1_size,udh_size = 0;
    uint32 user_header_egress_pmf_offset_0_dummy = 0, user_header_egress_pmf_offset_1_dummy = 0;
    uint8 extern_flag = 0;
    uint32 pwe_gach = 0;
    uint8 bos = 1;

	
    SOC_SAND_INIT_ERROR_DEFINITIONS(0);
    SOC_SAND_CHECK_NULL_INPUT(header_buffer);
    SOC_SAND_CHECK_NULL_INPUT(header_offset);
    SOC_PPC_OAM_OAMP_MEP_DB_ENTRY_clear(&mep_db_entry);
    res = arad_pp_oam_oamp_mep_db_entry_get_internal_unsafe(unit,lb_tst_info->endpoint_id,&short_name,&mep_db_entry);
    SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

    /* 1.1 Prepare internal header: PTCH */
    next_is_itmh =  (mep_db_entry.mep_type==SOC_PPC_OAM_MEP_TYPE_Y1731_O_PWE || mep_db_entry.mep_type==SOC_PPC_OAM_MEP_TYPE_Y1731_O_MPLSTP || mep_db_entry.mep_type==SOC_PPC_OAM_MEP_TYPE_Y1731_O_PWE_GAL || (mep_db_entry.mep_type==SOC_PPC_OAM_MEP_TYPE_ETH_OAM && !mep_db_entry.is_upmep ) );
    SOC_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  20, exit, ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_read(unit, OAMP_DOWN_PTCHr, REG_PORT_ANY, 0, DOWN_PTCH_PP_SSPf, &ssp));
    ssp =  (mep_db_entry.mep_type==SOC_PPC_OAM_MEP_TYPE_ETH_OAM && mep_db_entry.is_upmep )? mep_db_entry.local_port : ssp;
    offset = _insert_ptch2_for_tst_lb(header_buffer, offset, next_is_itmh, _JER_PP_OAM_PTCH_OPAQUE_VALUE,ssp);
    
    /*build itmh header*/
    if (next_is_itmh) {
     soc_field_t  fwd_tc[] = {CCM_CLASS_0f,CCM_CLASS_1f,CCM_CLASS_2f,CCM_CLASS_3f,CCM_CLASS_4f,CCM_CLASS_5f,CCM_CLASS_6f,CCM_CLASS_7f};
     soc_field_t fwd_dp[] = {CCM_DP_0f, CCM_DP_1f, CCM_DP_2f, CCM_DP_3f, CCM_DP_4f, CCM_DP_5f, CCM_DP_6f, CCM_DP_7f};
     uint32 tc= 0;
     uint32 dp = 0;
     uint32 fwd_dst_info = 0;
     SOC_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res, 30, exit, ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_read(unit, OAMP_PR_2_FW_DTCr, REG_PORT_ANY, 0, fwd_tc[mep_db_entry.priority], &tc));
     SOC_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res, 40, exit, ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_read(unit, OAMP_PR_2_FWDDPr, REG_PORT_ANY, 0, fwd_dp[mep_db_entry.priority], &dp));

     if(SOC_IS_QAX(unit)){
         SOC_SAND_SOC_IF_ERROR_RETURN(res, 60, exit, READ_OAMP_LOCAL_PORT_2_SYSTEM_PORTm(unit, MEM_BLOCK_ANY, mep_db_entry.local_port, &entry));
     }
     else{
         SOC_SAND_SOC_IF_ERROR_RETURN(res, 60, exit, READ_OAMP_MEM_20000m(unit, MEM_BLOCK_ANY, mep_db_entry.local_port, &entry)); /*  LOCAL_PORT_2_SYSTEM_PORT register*/
     }

     /*arad Style*/
     if(soc_property_get(unit, spn_ITMH_PROGRAMMABLE_MODE_ENABLE, FALSE) ==0 ){
       if (mep_db_entry.mep_type == SOC_PPC_OAM_MEP_TYPE_ETH_OAM) {
             fwd_dst_info =  0xc0000 | (entry&0xffff);
       } 
       else if ((mep_db_entry.mep_type == SOC_PPC_OAM_MEP_TYPE_Y1731_O_MPLSTP)||(mep_db_entry.mep_type == SOC_PPC_OAM_MEP_TYPE_Y1731_O_PWE)||
                (mep_db_entry.mep_type == SOC_PPC_OAM_MEP_TYPE_Y1731_O_PWE_GAL)){
             fwd_dst_info = 0xa0000 | mep_db_entry.egress_if;
       } 
       else {
           SOC_SAND_SET_ERROR_MSG((_BSL_SOCDNX_SAND_MSG("currently just support lb & tst in  ETH /MPLS-TP/PWE OAM mode")));
       }
        /* In case int_pri is set, DP and TC should be taken from it */
        if (lb_tst_info->int_pri != -1) {
            tc = ((uint32)(lb_tst_info->int_pri & 0x1F)) >> 2;
            dp = ((uint32)lb_tst_info->int_pri) & 0x3;
        }       
       offset = _insert_itmh_for_tst_lb_arad(header_buffer, offset, fwd_dst_info, tc, dp, 0, 0, (mep_db_entry.mep_type==SOC_PPC_OAM_MEP_TYPE_Y1731_O_MPLSTP  || mep_db_entry.mep_type==SOC_PPC_OAM_MEP_TYPE_Y1731_O_PWE || mep_db_entry.mep_type==SOC_PPC_OAM_MEP_TYPE_Y1731_O_PWE_GAL));
     }
     else{/*jericho Style*/
         if (mep_db_entry.mep_type == SOC_PPC_OAM_MEP_TYPE_ETH_OAM) {
            fwd_dst_info =  0x10000 | (entry&0xffff);
         }
         else if((mep_db_entry.mep_type == SOC_PPC_OAM_MEP_TYPE_Y1731_O_MPLSTP)||(mep_db_entry.mep_type == SOC_PPC_OAM_MEP_TYPE_Y1731_O_PWE)||
                (mep_db_entry.mep_type == SOC_PPC_OAM_MEP_TYPE_Y1731_O_PWE_GAL)) {
             bcm_petra_stk_my_modid_get(unit, &_modid);
             _mode_port = mep_db_entry.local_port;
             
             MBCM_DPP_DRIVER_CALL(unit, mbcm_dpp_local_to_tm_port_get, (unit, _mode_port, &tm_port, &core));
			 
             /* get physical system port, identify <mod,port>*/
             MBCM_DPP_DRIVER_CALL(unit,mbcm_dpp_local_to_sys_phys_port_map_get,(unit,_modid,tm_port,&_sys_phy_port_id));
             fwd_dst_info =  0x10000 | _sys_phy_port_id;
             extern_flag = 1;
         }
         else {
             SOC_SAND_SET_ERROR_MSG((_BSL_SOCDNX_SAND_MSG("currently just support lb & tst in  ETH /MPLS-TP OAM mode")));
         }
         offset = _insert_itmh_for_tst_lb_jericho(header_buffer, offset, fwd_dst_info, tc, dp, 0, 0, (mep_db_entry.mep_type==SOC_PPC_OAM_MEP_TYPE_Y1731_O_MPLSTP  || mep_db_entry.mep_type==SOC_PPC_OAM_MEP_TYPE_Y1731_O_PWE || mep_db_entry.mep_type==SOC_PPC_OAM_MEP_TYPE_Y1731_O_PWE_GAL),extern_flag);
     }
    }

    res = arad_pmf_db_fes_user_header_sizes_get(
        unit,
        &user_header_0_size,
        &user_header_1_size,
        &user_header_egress_pmf_offset_0_dummy,
        &user_header_egress_pmf_offset_1_dummy
        );
    SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);
    udh_size = user_header_1_size / 8 + user_header_0_size / 8; 
    
    if (mep_db_entry.mep_type == SOC_PPC_OAM_MEP_TYPE_ETH_OAM) {/*Prepare Ethernet header */
     uint16 vlans[2], tpids[2];
     soc_field_t tpid_field[] = {CCM_TPID_0f, CCM_TPID_1f, CCM_TPID_2f, CCM_TPID_3f};
     uint64 uint64_field;
     uint16 ether_type = 0x8902;
     bcm_mac_t peer_d_mac;
     bcm_mac_t gtf_smac;
     int i =0;
     uint32 sa_gen_profile;
     soc_reg_above_64_val_t entry_above_64;
     SOC_SAND_SOC_IF_ERROR_RETURN(res, 70, exit, _ARAD_PP_OAM_INTERNAL_READ_OAMP_MEP_DBm(unit, MEM_BLOCK_ANY, lb_tst_info->endpoint_id, entry_above_64)); 

     sa_gen_profile = soc_OAMP_MEP_DBm_field32_get(unit,&entry_above_64,SA_GEN_PROFILEf);
     
     /* SA gen profile: bits [0:7] represent the SA LSB, bit 8 represetns which profile is taken from OAMP_CCM_SA_MAC*/
     mep_db_entry.src_mac_msb_profile = (sa_gen_profile >> 8 ) & 0x1;
     mep_db_entry.src_mac_lsb = sa_gen_profile & 0xff;
     
     soc_jer_pp_oam_sa_addr_msbs_get( unit,mep_db_entry.src_mac_msb_profile,src_mac_address_msbs);
     gtf_smac[0] = src_mac_address_msbs[0];
     gtf_smac[1] = src_mac_address_msbs[1];
     gtf_smac[2] = src_mac_address_msbs[2];
     gtf_smac[3] = src_mac_address_msbs[3];
     gtf_smac[4] = src_mac_address_msbs[4];
     
     gtf_smac[5] = mep_db_entry.src_mac_lsb;
 
 

     SOC_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  80,  exit, ARAD_REG_ACCESS_ERR,soc_reg_above_64_field64_read(unit, OAMP_CCM_TPID_MAPr, REG_PORT_ANY, 0, tpid_field[mep_db_entry.outer_tpid], &uint64_field));
     tpids[0] = COMPILER_64_LO(uint64_field);
    
     vlans[0]= mep_db_entry.outer_vid_dei_pcp;
     if (lb_tst_info->pkt_pri != 0xff) {
         /* Using requested priority (PCP/DEI) */
         vlans[0] = ((lb_tst_info->pkt_pri & 0xf) << 12) + (mep_db_entry.outer_vid_dei_pcp & 0xfff);
     }
     if (mep_db_entry.tags_num == 2) {
         vlans[1]= mep_db_entry.inner_vid_dei_pcp;
         if (lb_tst_info->inner_pkt_pri != 0xff) {
             /* Using requested priority (PCP/DEI) */
             vlans[1] = ((lb_tst_info->inner_pkt_pri & 0xf) << 12) + (mep_db_entry.inner_vid_dei_pcp & 0xfff);
         }
         SOC_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  90,  exit, ARAD_REG_ACCESS_ERR,soc_reg_above_64_field64_read(unit, OAMP_CCM_TPID_MAPr, REG_PORT_ANY, 0, tpid_field[mep_db_entry.inner_tpid], &uint64_field));
         tpids[1] = COMPILER_64_LO(uint64_field);
    
     } 
    
    for(i=0; i<6;i++){
        peer_d_mac[i]=lb_tst_info->mac_address.address[i];
    }
    if((!mep_db_entry.is_upmep) && (soc_property_suffix_num_get(unit, -1, spn_CUSTOM_FEATURE, "injection_with_user_header_enable", 0 /*DEFAULT VALUE*/) == 1))
    {   
        offset = offset + udh_size;
    }
    
    offset = _insert_eth_for_tst_lb(header_buffer, offset,peer_d_mac, gtf_smac, tpids,vlans, mep_db_entry.tags_num,ether_type);
    } 
    else{
     /* MPLS TP or PWE. Either way the PPH is the same.*/
     PPH_base_head pph;
     uint8 exp, ttl;
     soc_field_t mpls_pwe_prf_fields[] = {MPLS_PWE_PROFILE_0f, MPLS_PWE_PROFILE_1f, MPLS_PWE_PROFILE_2f, MPLS_PWE_PROFILE_3f, MPLS_PWE_PROFILE_4f, 
     MPLS_PWE_PROFILE_5f, MPLS_PWE_PROFILE_6f, MPLS_PWE_PROFILE_7f, MPLS_PWE_PROFILE_8f, MPLS_PWE_PROFILE_9f, MPLS_PWE_PROFILE_10f, 
     MPLS_PWE_PROFILE_11f, MPLS_PWE_PROFILE_12f, MPLS_PWE_PROFILE_13f, MPLS_PWE_PROFILE_14f, MPLS_PWE_PROFILE_15f};
     soc_reg_t   reg = (SOC_IS_QAX_A0(unit) || SOC_IS_QUX(unit)) ? OAMP_REG_0130r : OAMP_TX_PPHr;
     soc_field_t field6 = (SOC_IS_QAX_A0(unit) || SOC_IS_QUX(unit)) ? FIELD_6_7f: TX_PPH_IN_LIF_ORIENTATIONf;
     soc_field_t field7 = (SOC_IS_QAX_A0(unit) || SOC_IS_QUX(unit)) ? FIELD_11_11f: TX_PPH_UNKNOWN_DAf;
     soc_field_t field9 = (SOC_IS_QAX_A0(unit) || SOC_IS_QUX(unit)) ? FIELD_12_27f: TX_PPH_VSI_OR_VRFf;
     soc_field_t field10 = (SOC_IS_QAX_A0(unit) || SOC_IS_QUX(unit)) ? FIELD_28_45f: TX_PPH_IN_LIF_OR_IN_RIFf;
     
     if(soc_property_get(unit, spn_ITMH_PROGRAMMABLE_MODE_ENABLE, FALSE) ==0 ){
         offset = _insert_itmh_extension_for_tst_lb(header_buffer, offset,0xc0000 | mep_db_entry.local_port);/* why 18&19 bit value is 1 */
     }
     else {
         offset = _insert_itmh_extension_jericho(header_buffer, offset,0,mep_db_entry.egress_if);/* outlif type is zero */
     }
     res = soc_reg_above_64_get(unit, reg, SOC_CORE_DEFAULT, 0, reg_data);
     SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 10, exit);
     
     pph.field1 = 0;
     pph.field2 = 0x5; /*_ARAD_PP_OAM_FWD_CODE_MPLS *mpls/PWE*/
     pph.field3 = udh_size;
     pph.field4 = 0;
     pph.field5 = 0;
     pph.field6 = soc_reg_above_64_field32_get(unit, reg, reg_data, field6);
     pph.field7 = soc_reg_above_64_field32_get(unit, reg, reg_data, field7);
     pph.field8 = 0;
     pph.field9 = soc_reg_above_64_field32_get(unit, reg, reg_data, field9);
     pph.field10 = soc_reg_above_64_field32_get(unit, reg, reg_data, field10);
    
     offset = _insert_pph_for_tst_lb(header_buffer, &pph, offset);
    
    /* UDH comes right after the PPH and before the payload*/
    offset = offset + udh_size;
    
     SOC_REG_ABOVE_64_CLEAR(reg_data);
     SOC_REG_ABOVE_64_CLEAR(reg_field);
     SOC_SAND_SOC_IF_ERROR_RETURN(res, 110, exit, READ_OAMP_MPLS_PWE_PROFILEr(unit, reg_data));
     soc_reg_above_64_field_get(unit, OAMP_MPLS_PWE_PROFILEr, reg_data, mpls_pwe_prf_fields[mep_db_entry.push_profile],reg_field); 
    
     ttl = reg_field[0] & 0xff; /*first 8 bits*/
     exp = (reg_field[0] & 0x700) >> 8 ; /* next 3 bits*/
      
      if (mep_db_entry.mep_type == SOC_PPC_OAM_MEP_TYPE_Y1731_O_MPLSTP) {
         /* Packet format: PTCH2 o ITMH o PPH o LSP o GAL+GACH o OAM*/
         uint32 gal_gach;
    
         offset = _insert_mpls_label_for_tst_lb(header_buffer, mep_db_entry.label,exp, 0, ttl, offset);
         SOC_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res, 120, exit  , ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_read(unit, OAMP_Y_1731O_MPLSTP_GALr, REG_PORT_ANY, 0, Y_1731O_MPLSTP_GALf, &gal_gach));
         offset= _insert_4_bytes_for_tst_lb(header_buffer ,gal_gach,offset);
         SOC_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res, 130, exit  , ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_read(unit, OAMP_Y_1731O_MPLSTP_GACHr, REG_PORT_ANY, 0, Y_1731O_MPLSTP_GACHf, &gal_gach));
         offset= _insert_4_bytes_for_tst_lb(header_buffer ,gal_gach,offset);
     } else if (mep_db_entry.mep_type == SOC_PPC_OAM_MEP_TYPE_Y1731_O_PWE ||
                mep_db_entry.mep_type == SOC_PPC_OAM_MEP_TYPE_Y1731_O_PWE_GAL) {
         offset = _insert_mpls_label_for_tst_lb(header_buffer, mep_db_entry.label,exp, bos, ttl, offset);
         SOC_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res, 130, exit  , ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_read(unit,OAMP_Y_1731O_PWE_GACHr, REG_PORT_ANY, 0, Y_1731O_PWE_GACHf, &pwe_gach));
         offset= _insert_4_bytes_for_tst_lb(header_buffer ,pwe_gach,offset);

     }
    }
    
    /* build OAM header*/
    offset= _insert_oam_pdu_header_for_tst_lb(header_buffer, mep_db_entry.mdl, lb_tst_info->flags, offset);
    
    *header_offset = offset;

exit:
      SOC_SAND_EXIT_AND_SEND_ERROR_SOCDNX((_BSL_SOCDNX_SAND_MSG("Something went wrong")));
}


/* Initialization required from init sequence. */
soc_error_t soc_jer_pp_oam_init_from_init_sequence(int unit)
{
    uint32 reg32;
    int core;
    int rv;
    SOCDNX_INIT_FUNC_DEFS;

    SOC_DPP_CORES_ITER(SOC_CORE_ALL, core) {
        if (SOC_IS_JERICHO_AND_BELOW(unit)) {
            /* This correctly configures the counter FIFO in the egress.*/
			/* Must be done before traffic has been sent, thus should be done in the 
			init sequence*/
            rv = READ_EPNI_CFG_LINK_FIFOS_FIXED_LATENCY_SETTINGr(unit,core,&reg32);
            SOCDNX_IF_ERR_EXIT(rv);
            soc_reg_field_set(unit,EPNI_CFG_LINK_FIFOS_FIXED_LATENCY_SETTINGr,&reg32, CFG_LINK_P_22_FIXED_LATENCY_SETTINGf, 0xd);
            rv = WRITE_EPNI_CFG_LINK_FIFOS_FIXED_LATENCY_SETTINGr(unit,core,reg32);
            SOCDNX_IF_ERR_EXIT(rv);
        }
    }
exit:
    SOCDNX_FUNC_RETURN;
}

/*********************************************************************
* NAME:
 *   soc_jer_oamp_sat_arb_weight_set
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Set oamp sat arbiter weight
 * INPUT:
 *   int unit
 *   uint32 sat weight value
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
soc_error_t soc_jer_oamp_sat_arb_weight_set(int unit,uint32 sat_arbiter_weight){
   uint32   oamp_arbiter_weight;
   int rv = BCM_E_NONE;

   BCMDNX_INIT_FUNC_DEFS;

   rv = READ_OAMP_ARBITER_WEIGHTr(unit, &oamp_arbiter_weight);
   BCMDNX_IF_ERR_EXIT(rv);

   soc_reg_field_set(unit, OAMP_ARBITER_WEIGHTr, &oamp_arbiter_weight, SAT_ARB_WEIGHTf, sat_arbiter_weight );   

   rv = WRITE_OAMP_ARBITER_WEIGHTr(unit, oamp_arbiter_weight);
   BCMDNX_IF_ERR_EXIT(rv);

   BCM_EXIT;
exit:
    BCMDNX_FUNC_RETURN;
}

soc_error_t
  soc_jer_pp_oamp_control_sat_weight_get(
    SOC_SAND_IN  int                   unit,
    SOC_SAND_OUT uint32                *sat_weight
  )
{
    uint32 oamp_arbiter_weight;
    uint32 soc_sand_rv;

    SOCDNX_INIT_FUNC_DEFS;

    SOCDNX_NULL_CHECK(sat_weight);

    soc_sand_rv = READ_OAMP_ARBITER_WEIGHTr(unit, &oamp_arbiter_weight);
    SOCDNX_SAND_IF_ERR_EXIT(soc_sand_rv);
    *sat_weight = soc_reg_field_get(unit, OAMP_ARBITER_WEIGHTr, oamp_arbiter_weight, SAT_ARB_WEIGHTf);

exit:
    SOCDNX_FUNC_RETURN;
}

#include <soc/dpp/SAND/Utils/sand_footer.h>

