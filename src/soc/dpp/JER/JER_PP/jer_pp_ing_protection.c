/* $Id: jer_pp_ing_protection.c,v 1.29 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/

#ifdef _ERR_MSG_MODULE_NAME
  #error "_ERR_MSG_MODULE_NAME redefined"
#endif

#define _ERR_MSG_MODULE_NAME BSL_SOC_INGRESS


/*************
 * INCLUDES  *
 *************/
/* { */
#include <shared/bsl.h>
#include <soc/dcmn/error.h>
#include <soc/dpp/SAND/Utils/sand_header.h>
#include <soc/dpp/drv.h>
#include <soc/dpp/JER/JER_PP/jer_pp_ing_protection.h>

#include <soc/mcm/memregs.h>
#include <soc/mcm/memacc.h>
#include <soc/mem.h>

/* } */
/*************
 * DEFINES   *
 *************/
/* { */

/* Ingress Protection Path table bit manipulation values */
#define JER_PP_INGRESS_PROTECTION_PATH_STATE_BITS               (1)

#define JER_PP_INGRESS_PROTECTION_PATH_NOF_ENTRY_OFFSET_BITS    (3)
#define JER_PP_INGRESS_PROTECTION_PATH_NOF_ENTRY_IDX_BITS       (13)

#define JER_PP_INGRESS_PROTECTION_PATH_ENTRY_OFFSET_MASK        (0x7)
#define JER_PP_INGRESS_PROTECTION_PATH_ENTRY_IDX_MASK           (0x7FF8)

/* } */
/*************
 * MACROS    *
 *************/
/* { */


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

/* 
 *  Local functions
 */ 

/*
 * Function:
 *      soc_jer_ingress_protection_state_verify
 * Purpose: 
 *      Validates ingress protection get/set parameters.
 *      The validated valus are supplied as by address. Thus,
 *      a NULL pointer enables to skip a specific parameter
 *      validation.
 * Parameters:
 *      unit    - Device Number
 *      protection_ndx - An index to the protection states table, in the
 *                  allowed protection state allocation range.
 *      path_state - Path state value
 * Returns:
 *      SOC_E_PARAM - Parameter validation failure
 */
soc_error_t soc_jer_ingress_protection_state_verify(
   int unit,
   uint32 *protection_ndx,
   uint8 *path_state)
{
    SOCDNX_INIT_FUNC_DEFS;

    /* Validate the Ingress Protection index */
    if (protection_ndx) {
        if (*protection_ndx >= SOC_DPP_DEFS_GET(unit, nof_failover_ingress_ids)) {
            SOCDNX_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_SOCDNX_MSG("Protection index out of range")));
        }
    }

    /* Validate the Path State */
    if (path_state) {
        if (*path_state > 1) {
            SOCDNX_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_SOCDNX_MSG("Path state out of range")));
        }
    }

    SOC_EXIT;
exit:
    SOCDNX_FUNC_RETURN;
}


/* 
 *  SOC API functions
 */ 

/*
 * Function:
 *      soc_jer_pp_ing_protection_init
 * Purpose: 
 *      Perform any Jericho specific initialization for the Ingress side
 *      protection, including Ingress and protection.
 *      The initialization will set the Ingress protection coupling mode and
 *      FEC protection accelerated reroute mode according to SOC Properties.
 * Parameters:
 *      unit    - Device Number
 * Returns:
 *      SOC_E_XXX   - HW Read or Write failure
 */
soc_error_t soc_jer_pp_ing_protection_init(int unit)
{
    uint32 field_val;
    int rv;
    SOCDNX_INIT_FUNC_DEFS;

    /* Set a HW Ingress protection coupled mode indication according to the
       equivalent SOC Property value */
    field_val = (SOC_DPP_IS_PROTECTION_INGRESS_COUPLED(unit)) ? 1 : 0;
    if (SOC_IS_JERICHO_PLUS_A0(unit)) {
        rv = soc_reg_above_64_field32_modify(unit, IHP_VTT_GENERAL_CONFIGS_1r, REG_PORT_ANY,  0, COUPLED_PATH_SELECT_POINTERf, field_val);
    }
    else {
        rv = soc_reg_field32_modify(unit, IHP_VTT_GENERAL_CONFIGS_1r, REG_PORT_ANY, COUPLED_PATH_SELECT_POINTERf, field_val);
    }
    SOCDNX_IF_ERR_EXIT(rv);

    /* Set a HW FEC protection accelerated reroute mode indication according to
       the equivalent SOC Property value */
    field_val = (SOC_DPP_IS_PROTECTION_FEC_ACCELERATED_REROUTE_MODE(unit)) ? 1 : 0;
    rv = soc_reg_field32_modify(unit, IHB_FER_GENERAL_CONFIGURATIONSr, REG_PORT_ANY, ENABLE_PATH_AND_FACILITY_PROTECTIONf, field_val);
    SOCDNX_IF_ERR_EXIT(rv);

exit:
    SOCDNX_FUNC_RETURN;
}


/*
 * Function:
 *      soc_jer_ingress_protection_state_set
 * Purpose: 
 *      Sets an ingress protection state to a HW table 
 *      Each table entry holds 8 protection states.
 *      In case of de-coupled mode, the MSB is set to the Ptotection-Path field
 *      of the In-LIF.
 * Parameters:
 *      unit    - Device Number
 *      protection_ndx - An index to the protection states table, in the
 *                  allowed protection state allocation range.
 *      path_state - The state value to be set
 * Returns:
 *      SOC_E_PARAM - Parameter validation failure
 *      SOC_E_XXX   - HW Read or Write failure
 */
soc_error_t soc_jer_ingress_protection_state_set(
   int unit,
   uint32 protection_ndx,
   uint8 path_state)
{
    soc_error_t rv;
    uint32 tbl_idx, formated_path_state;
    uint32 ihp_vtt_path_select_entry_data;
    uint8 entry_offset;
    SOCDNX_INIT_FUNC_DEFS;

    /* Validate the input parameters */
    rv = soc_jer_ingress_protection_state_verify(unit, &protection_ndx, &path_state);
    SOCDNX_IF_ERR_EXIT(rv);

    /* Init the read buffer */
    ihp_vtt_path_select_entry_data = 0;

    /* Analyze the Protection Pointer value to get values that are required for the HW access:
        tbl_idx - The index to the Protection Path table is located in bits 3:14
        entry_offset - The offset within a Protection Path entry is located in bits 0:2 */
    entry_offset = protection_ndx & JER_PP_INGRESS_PROTECTION_PATH_ENTRY_OFFSET_MASK;
    tbl_idx = (protection_ndx & JER_PP_INGRESS_PROTECTION_PATH_ENTRY_IDX_MASK) >>
              JER_PP_INGRESS_PROTECTION_PATH_NOF_ENTRY_OFFSET_BITS;

    /* Read the required protection path entry from the HW */
    rv = READ_IHP_VTT_PATH_SELECTm(unit, MEM_BLOCK_ANY, tbl_idx, &ihp_vtt_path_select_entry_data);
    SOCDNX_IF_ERR_EXIT(rv);

    /* Set the supplied value after calculating the offset within the entry.
       Each entry has 8 Path Status values */
    formated_path_state = SOC_SAND_BOOL2NUM_INVERSE(path_state);
    SHR_BITCOPY_RANGE(&ihp_vtt_path_select_entry_data, entry_offset,
                      &formated_path_state, 0, JER_PP_INGRESS_PROTECTION_PATH_STATE_BITS);

    /* Write the modified protection path entry to the HW */
    if (SOC_IS_JERICHO_PLUS_A0(unit)) {
    	/* WA for Jericho+: indirect_wr_mask register masking mechanism is opposite for IHP_VTT_PATH_SELECT table */ 
    	SOCDNX_IF_ERR_EXIT(WRITE_IHP_INDIRECT_WR_MASKr(unit,SOC_CORE_ALL, 0));
    }
    rv = WRITE_IHP_VTT_PATH_SELECTm(unit, MEM_BLOCK_ALL, tbl_idx, &ihp_vtt_path_select_entry_data);
    SOCDNX_IF_ERR_EXIT(rv);
    if (SOC_IS_JERICHO_PLUS_A0(unit)) {
    	SOCDNX_IF_ERR_EXIT(WRITE_IHP_INDIRECT_WR_MASKr(unit,SOC_CORE_ALL, 0xffffffff));
    }    

exit:
    SOCDNX_FUNC_RETURN;
}


/*
 * Function:
 *      soc_jer_ingress_protection_state_get
 * Purpose: 
 *      Retrieves an ingress protection state from the HW table 
 *      Each table entry holds 8 protection states.
 * Parameters:
 *      unit    - Device Number
 *      protection_ndx - An index to the protection states table, in the
 *                  allowed protection state allocation range.
 *      path_state - The retrieved path state value
 * Returns:
 *      SOC_E_PARAM - Parameter validation failure
 *      SOC_E_XXX   - HW Read failure
 */
soc_error_t soc_jer_ingress_protection_state_get(
   int unit,
   uint32 protection_ndx,
   uint8 *path_state)
{
    soc_error_t rv;
    uint32 tbl_idx, formated_path_state;
    uint32 ihp_vtt_path_select_entry_data;
    uint8 entry_offset;
    SOCDNX_INIT_FUNC_DEFS;

    /* Validate the input parameters */
    rv = soc_jer_ingress_protection_state_verify(unit, &protection_ndx, NULL);
    SOCDNX_IF_ERR_EXIT(rv);

    /* Init the read buffers */
    ihp_vtt_path_select_entry_data = 0;
    formated_path_state = 0;

    /* Analyze the Protection Pointer value to get values that are required for the HW access:
        tbl_idx - The index to the Protection Path table is located in bits 3:14
        entry_offset - The offset within a Protection Path entry is located in bits 0:2 */
    entry_offset = protection_ndx & JER_PP_INGRESS_PROTECTION_PATH_ENTRY_OFFSET_MASK;
    tbl_idx = (protection_ndx & JER_PP_INGRESS_PROTECTION_PATH_ENTRY_IDX_MASK) >>
              JER_PP_INGRESS_PROTECTION_PATH_NOF_ENTRY_OFFSET_BITS;

    /* Read the required protection state entry from the HW */
    rv = READ_IHP_VTT_PATH_SELECTm(unit, MEM_BLOCK_ANY, tbl_idx, &ihp_vtt_path_select_entry_data);
    SOCDNX_IF_ERR_EXIT(rv);

    /* Get the requested value after calculating the offset within the entry.
       Each entry has 8 Path Status values */
    SHR_BITCOPY_RANGE(&formated_path_state, 0,
                      &ihp_vtt_path_select_entry_data, entry_offset, JER_PP_INGRESS_PROTECTION_PATH_STATE_BITS);
    *path_state = SOC_SAND_NUM2BOOL_INVERSE(formated_path_state);

exit:
    SOCDNX_FUNC_RETURN;
}

#include <soc/dpp/SAND/Utils/sand_footer.h>

/* } */

