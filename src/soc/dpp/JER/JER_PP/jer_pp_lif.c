/* $Id: soc_jer_mymac.c,v 1.29 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/

#ifdef _ERR_MSG_MODULE_NAME
  #error "_ERR_MSG_MODULE_NAME redefined"
#endif

#define _ERR_MSG_MODULE_NAME BSL_SOC_LIF

/*************
 * INCLUDES  *
 *************/
/* { */
#include <shared/bsl.h>
#include <soc/dcmn/error.h>
#include <soc/dpp/SAND/Utils/sand_header.h>
#include <soc/dpp/drv.h>
#include <soc/dpp/PPC/ppc_api_lif.h>
#include <soc/dpp/JER/JER_PP/jer_pp_lif.h>


#include <soc/mcm/memregs.h>
#include <soc/mcm/memacc.h>
#include <soc/mem.h>

#ifdef CRASH_RECOVERY_SUPPORT
#include <soc/hwstate/hw_log.h>
#endif /* CRASH_RECOVERY_SUPPORT */

#ifdef SOC_DPP_IS_EM_HW_ENABLE
  #include <soc/dpp/ARAD/arad_sim_em.h>
#else
  #include <sim/dpp/ChipSim/chip_sim_em.h>
#endif


/* } */
/*************
 * DEFINES   *
 *************/
/* { */

/* Buffer size for the glem management register.*/
#define SOC_JER_LIF_GLEM_BUFFER_SIZE (2)

/* Actions for the glem management register. */
#define SOC_JER_LIF_GLEM_ACTION_WRITE (1)
#define SOC_JER_LIF_GLEM_ACTION_REMOVE (0)

/* Whether to print the glem access error or not. */
#define SOC_JER_LIF_GLEM_ACCESS_DEBUG (0)

#define JER_PP_EG_ENCAP_NOF_ENTRIES_PER_HALF_BANK (4096)


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

#ifdef SOC_DPP_IS_EM_HW_ENABLE
STATIC soc_error_t
soc_jer_lif_glem_entry_buffer_set(int unit, uint32 action_type, uint32 global_lif_id, uint32 egress_lif_id, uint32 *buffer);

/*STATIC */soc_error_t
soc_jer_lif_glem_access_assert_management_request_complete(int unit);

STATIC soc_error_t
soc_jer_lif_glem_access_write_to_machine(int unit, uint32 *buffer);
#endif /* SOC_DPP_IS_EM_HW_ENABLE */


/*********************************************************************
 * NAME:
 *   soc_jer_lif_init
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Initializes the glem and eedb bank mapping.
 * INPUT:
 *   int            unit             - (IN) Identifier of the device to access.
 * REMARKS:
 *   
 * RETURNS:
 *   SOC_E_NONE  Otherwise.
*********************************************************************/
soc_error_t
soc_jer_lif_init(int unit){
    int rv;
    uint64 buffer;
    int nof_direct_banks; 
    SOCDNX_INIT_FUNC_DEFS;

    /* 
     *  First, set the bits that enable the GLEM.
     */
     
    /* Set the glem enables bit */
    rv = soc_reg_above_64_field32_modify(unit, EDB_GLEM_MANAGEMENT_UNIT_CONFIGURATION_REGISTERr, REG_PORT_ANY, 0, GLEM_MNGMNT_UNIT_ENABLEf,  0x1);
    SOCDNX_IF_ERR_EXIT(rv);

    /* Set the "use glem" bit for the EEDB. */
    rv = soc_reg_field32_modify(unit, EPNI_CFG_GLEM_LKUP_CONFIGr, REG_PORT_ANY, CFG_GLEM_LKUP_RESULT_PHASEf, 0x1);
    SOCDNX_IF_ERR_EXIT(rv);

    /* 
     *  Second, set the bits that decide for each 4k global lifs whether they are direct lifs or mapped lifs.
     *      On startup, the only direct banks are rif banks. 
     */

    /* 
     *  The mask should be all 1s (trap glem lookup failure), except if the bank is a direct bank.
     *  In Jericho, the lower banks are used for rif entries, and so are direct banks.
     *  In QAX, the rifs are located in another table. This register sets the banks by the EEDB index,
     *  so we shouldn't take the rifs into account.
     *  
     */
 
    if (SOC_IS_JERICHO_PLUS(unit)){
        COMPILER_64_ALLONES(buffer);
    } else if (SOC_IS_JERICHO_AND_BELOW(unit)) {
        nof_direct_banks = SOC_DPP_CONFIG(unit)->l3.nof_rifs / JER_PP_EG_ENCAP_NOF_ENTRIES_PER_HALF_BANK;
        COMPILER_64_MASK_CREATE(buffer, 64 /* nof bits in uint64*/ - nof_direct_banks, nof_direct_banks);
    } else {
        SOCDNX_EXIT_WITH_ERR(SOC_E_INTERNAL, (_BSL_SOCDNX_MSG("Glem does not exist on this chip.")));
    }
    
    /* There are two mapping_is_required registers, one for outlif and one for eei. They should be the same. */
    rv = soc_reg_above_64_field64_modify(unit, EPNI_CFG_OUTLIF_MAPPING_IS_REQUIREDr, REG_PORT_ANY, 0, CFG_OUTLIF_MAPPING_IS_REQUIREDf, buffer);
    SOCDNX_IF_ERR_EXIT(rv);

    rv = soc_reg_above_64_field64_modify(unit, EPNI_CFG_EEI_MAPPING_IS_REQUIREDr, REG_PORT_ANY, 0, CFG_EEI_MAPPING_IS_REQUIREDf, buffer);
    SOCDNX_IF_ERR_EXIT(rv);


    /* Add the illegal egress lif to the glem, so it skips the glem trap. */
    if (SOC_IS_JERICHO_A0(unit) || SOC_IS_QMX_A0(unit)) {
        rv = soc_jer_lif_glem_access_entry_add(unit, SOC_PPC_INVALID_GLOBAL_LIF, SOC_PPC_INVALID_GLOBAL_LIF);
        SOCDNX_IF_ERR_EXIT(rv);
    }

exit:
  SOCDNX_FUNC_RETURN;
}

/*********************************************************************
* NAME:
 *   soc_jer_lif_glem_entry_add
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Adds the {global_lif_id, egress_lif_id} mapping to the GLEM table.
 * INPUT:
 *   int            unit             - (IN) Identifier of the device to access.
 *   int            global_lif_id    - (IN) Global lif for this entry.
 *   int            egress_lif_id    - (IN) Egress lif to which the global lif will point. 
 * REMARKS:
 *   
 * RETURNS:
 *   SOC_E_***   If there was a problem writing to the table
 *   SOC_E_NONE  Otherwise.
*********************************************************************/
soc_error_t
soc_jer_lif_glem_access_entry_add(int unit, int global_lif_id, int egress_lif_id){
    int rv;

    uint32 uint_global_lif_id[1], uint_egress_lif_id[1];
#ifdef SOC_DPP_IS_EM_HW_ENABLE
    uint32 buffer[SOC_JER_LIF_GLEM_BUFFER_SIZE] = {0};
    uint32 reason;
    uint32
      is_failed;
    uint32 failure;
#endif
    uint8 is_success = TRUE;
    SOCDNX_INIT_FUNC_DEFS;
    
 
#ifdef SOC_DPP_IS_EM_HW_ENABLE
 
    /* Set entry buffer. action type : key : value  */

    rv = soc_jer_lif_glem_entry_buffer_set(unit, SOC_JER_LIF_GLEM_ACTION_WRITE, global_lif_id, egress_lif_id, buffer);
    SOCDNX_IF_ERR_EXIT(rv);



    /*
     * Write to machine register. 
     */
    rv = soc_jer_lif_glem_access_write_to_machine(unit, buffer);
    SOCDNX_IF_ERR_EXIT(rv);

    /* 
     * Get error, if any.
     */ 
    rv = READ_EDB_GLEM_MANAGEMENT_UNIT_FAILUREr(unit, &failure);
    SOCDNX_IF_ERR_EXIT(rv);
    
    is_success = TRUE;
    
    is_failed = soc_reg_field_get(unit, EDB_GLEM_MANAGEMENT_UNIT_FAILUREr, failure, GLEM_MNGMNT_UNIT_FAILURE_VALIDf);
    if (is_failed)
    {

        reason = soc_reg_field_get(unit, EDB_GLEM_MANAGEMENT_UNIT_FAILUREr, failure, GLEM_MNGMNT_UNIT_FAILURE_REASONf);

        switch(reason)
        {
        case 0x001:/*Notice: Cam table full*/
        case 0x002:/*Notice: Table coherency             */
        case 0x008:/*Notice: Reached max entry limit     */
        case 0x080:/*Notice: Change-fail non exist       */
        case 0x100:/*Notice: Change request over static  */
        case 0x200:/*Notice: Change non-exist from other */
        case 0x400:/*Notice: Change non-exist from self  */
            is_success = FALSE;
        break;
        default:
        break;
        }
    } else {
        reason = 0;
    }

    /*
     * Debug error code.
     */
#if SOC_JER_LIF_GLEM_ACCESS_DEBUG
    if (reason)
    {
      switch(reason)
      {
      case 0x001:
        LOG_INFO(BSL_LS_SOC_LIF,
                 (BSL_META_U(unit,
                             "Notice: Cam table full              ")));
        break;
      case 0x002:
        LOG_INFO(BSL_LS_SOC_LIF,
                 (BSL_META_U(unit,
                             "Notice: Table coherency             ")));
        break;
      case 0x004:
        LOG_INFO(BSL_LS_SOC_LIF,
                 (BSL_META_U(unit,
                             "Notice: Delete unknown key          ")));
        break;
      case 0x008:
        LOG_INFO(BSL_LS_SOC_LIF,
                 (BSL_META_U(unit,
                             "Notice: Reached max entry limit     ")));
        break;
      case 0x010:
        LOG_INFO(BSL_LS_SOC_LIF,
                 (BSL_META_U(unit,
                             "Notice: Inserted existing           ")));
        break;
      case 0x020:
        LOG_INFO(BSL_LS_SOC_LIF,
                 (BSL_META_U(unit,
                             "Notice: Learn request over static   ")));
        break;
      case 0x040:
        LOG_INFO(BSL_LS_SOC_LIF,
                 (BSL_META_U(unit,
                             "Notice: Learn over existing         ")));
        break;
      case 0x080:
        LOG_INFO(BSL_LS_SOC_LIF,
                 (BSL_META_U(unit,
                             "Notice: Change-fail non exist       ")));
        break;
      case 0x100:
        LOG_INFO(BSL_LS_SOC_LIF,
                 (BSL_META_U(unit,
                             "Notice: Change request over static  ")));
        break;
      case 0x200:
        LOG_INFO(BSL_LS_SOC_LIF,
                 (BSL_META_U(unit,
                             "Notice: Change non-exist from other ")));
        break;
      case 0x400:
        LOG_INFO(BSL_LS_SOC_LIF,
                 (BSL_META_U(unit,
                             "Notice: Change non-exist from self  ")));
        break;
      }
    }
#endif /* SOC_JER_LIF_GLEM_ACCESS_DEBUG */

#endif   /* SOC_DPP_IS_EM_HW_ENABLE */

    if (SOC_DPP_IS_EM_SIM_ENABLE(unit)) {
        /* If we're in SIM, or EM shadow is enabled, enter entry to shadow. */
        *uint_egress_lif_id = egress_lif_id;
        *uint_global_lif_id = global_lif_id;
        rv = chip_sim_exact_match_entry_add_unsafe(
                unit,
                ARAD_CHIP_SIM_GLEM_BASE,
                uint_global_lif_id,
                ARAD_CHIP_SIM_GLEM_KEY,
                uint_egress_lif_id,
                ARAD_CHIP_SIM_GLEM_PAYLOAD,
                &is_success
              );
        SOCDNX_IF_ERR_EXIT(rv);
    }

    if (!is_success){
        SOCDNX_EXIT_WITH_ERR(SOC_E_FULL, (_BSL_SOCDNX_MSG("Glem table is full")));
    }

exit:
  SOCDNX_FUNC_RETURN;
}

/*********************************************************************
* NAME:
 *   soc_jer_lif_glem_access_entry_remove
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Given a global_lif_id, removes the {global lif, egress lif} mapping from the glem table.
 * INPUT:
 *   int            unit             - (IN) Identifier of the device to access.
 *   int            global_lif_id    - (IN) Global lif to be removed.
 * REMARKS:
 *   
 * RETURNS:
 *   SOC_E_***   If there was a problem writing to the table
 *   SOC_E_NONE  Otherwise.
*********************************************************************/
soc_error_t
soc_jer_lif_glem_access_entry_remove(int unit, int global_lif_id){
    int rv;
    uint32 uint_global_lif_id[1];
#ifdef SOC_DPP_IS_EM_HW_ENABLE
    uint32
      buffer[SOC_JER_LIF_GLEM_BUFFER_SIZE] = {0};
#endif
    SOCDNX_INIT_FUNC_DEFS;
    
    
        
#ifdef SOC_DPP_IS_EM_HW_ENABLE
    
    /* 
     * Set buffer
     */
    rv = soc_jer_lif_glem_entry_buffer_set(unit, SOC_JER_LIF_GLEM_ACTION_REMOVE, global_lif_id, 0, buffer);
    SOCDNX_IF_ERR_EXIT(rv);


    /*
     * Write to machine register. 
     */
    rv = soc_jer_lif_glem_access_write_to_machine(unit, buffer);
    SOCDNX_IF_ERR_EXIT(rv);

#endif /* SOC_DPP_IS_EM_HW_ENABLE */

    if (SOC_DPP_IS_EM_SIM_ENABLE(unit)) {
        *uint_global_lif_id = global_lif_id;
        rv = chip_sim_exact_match_entry_remove_unsafe(
                unit,
                ARAD_CHIP_SIM_GLEM_BASE,
                uint_global_lif_id,
                ARAD_CHIP_SIM_GLEM_KEY
              );
        SOCDNX_IF_ERR_EXIT(rv);
    }


exit:
    SOCDNX_FUNC_RETURN;
}

/*********************************************************************
* NAME:
 *   soc_jer_lif_glem_access_entry_by_key_get
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Given a global_lif_id, returns the egress lif id it's mapped to, and whether the entry has been accessed or not. 
 * INPUT:
 *   int            unit             - (IN) Identifier of the device to access.
 *   int            global_lif_id    - (IN) Global lif to be removed.
 *   int            egress_lif_id    - (OUT)Will be filled with the egress lif's id.
 *   uint8          accessed         - (OUT)Will be TRUE if entry has been accessed, FALSE otherwise.
 *   uint8          is_found         - (OUT)TRUE if global lif exists, FALSE otherwise.
 * REMARKS:
 *   
 * RETURNS:
 *   SOC_E_***   If there was a problem reading from the table
 *   SOC_E_NONE  Otherwise.
*********************************************************************/
soc_error_t
soc_jer_lif_glem_access_entry_by_key_get(int unit, int global_lif_id, int *egress_lif_id, uint8 *accessed, uint8 *is_found){
    int rv;
    uint32 fld_val[1];
    uint32 reg_val1[1];
    SOCDNX_INIT_FUNC_DEFS;

    /* Input checks. No null check for accessed - it's optional. */
    SOCDNX_NULL_CHECK(egress_lif_id);
    SOCDNX_NULL_CHECK(is_found);
    
#ifdef SOC_DPP_IS_EM_HW_ENABLE
    /* If we don't use SW shadowing for EMs, or if the user wants to know if the entry was accessed, then we have to
     *    read from HW.
     */
    if (!SOC_DPP_IS_EM_SIM_ENABLE(unit) || accessed) {
        /*
         *    Write the key to the diagnostic register.
         */
        *fld_val = global_lif_id;
        rv = WRITE_EDB_GLEM_DIAGNOSTICS_KEYr(unit, *fld_val);
        SOCDNX_IF_ERR_EXIT(rv);

        /*
         *    Set the trigger
         */
        *fld_val = 0x1;
        rv = soc_reg_field32_modify(unit, EDB_GLEM_DIAGNOSTICSr, REG_PORT_ANY, GLEM_DIAGNOSTICS_LOOKUPf, *fld_val);
        SOCDNX_IF_ERR_EXIT(rv);

        /*
         *    Poll on the trigger bit before getting the result
         */
        rv = (arad_polling(unit, ARAD_TIMEOUT, ARAD_MIN_POLLS, EDB_GLEM_DIAGNOSTICSr, 0/*REG_PORT_ANY*/, 0, GLEM_DIAGNOSTICS_LOOKUPf, 0));
        SOCDNX_SAND_IF_ERR_EXIT(rv);

        /*
         *    Get the lookup result
         */
        rv = READ_EDB_GLEM_DIAGNOSTICS_LOOKUP_RESULTr(unit, reg_val1);
        SOCDNX_IF_ERR_EXIT(rv);

        *is_found = soc_reg_field_get(unit, EDB_GLEM_DIAGNOSTICS_LOOKUP_RESULTr, *reg_val1, GLEM_ENTRY_FOUNDf);
        

        if (!*is_found){
            SOC_EXIT;
        }

        *egress_lif_id = soc_reg_field_get(unit, EDB_GLEM_DIAGNOSTICS_LOOKUP_RESULTr, *reg_val1, GLEM_ENTRY_PAYLOADf);

        if (accessed) {
            *accessed = soc_reg_field_get(unit, EDB_GLEM_DIAGNOSTICS_LOOKUP_RESULTr, *reg_val1, GLEM_ENTRY_ACCESSEDf);
        }
    } else 

#endif /* SOC_DPP_IS_EM_HW_ENABLE */
    {
        /* 
         *  If we're in SIM, or EM shadow is enabled, and user doesn't need to know accessed state, read from the SW shadow.
         *  
         */
        *reg_val1 = global_lif_id;
        rv = chip_sim_exact_match_entry_get_unsafe(
                unit,
                ARAD_CHIP_SIM_GLEM_BASE,
                reg_val1,
                ARAD_CHIP_SIM_GLEM_KEY,
                fld_val,
                ARAD_CHIP_SIM_GLEM_PAYLOAD,
                is_found
              );
        SOCDNX_IF_ERR_EXIT(rv);
        
        if (*is_found) {
            *egress_lif_id = *fld_val;
        }
    }


exit:
    SOCDNX_FUNC_RETURN;
}
#ifdef SOC_DPP_IS_EM_HW_ENABLE
/*********************************************************************
* NAME:
 *   soc_jer_lif_glem_access_assert_management_request_complete
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Checks the glem interrupt register to confirm that all glem actions are done and new commands can be written to it.
 * INPUT:
 *   int            unit             - (IN) Identifier of the device to access.
 * REMARKS:
 *   
 * RETURNS:
 *   SOC_E_BUSY  If the table can't be accessed right now.
 *   SOC_E_***   If there was a problem reading the register
 *   SOC_E_NONE  Otherwise.
*********************************************************************/
/*STATIC */soc_error_t
soc_jer_lif_glem_access_assert_management_request_complete(int unit){
    int rv;
    uint32 temp;
    SOCDNX_INIT_FUNC_DEFS;

    rv = soc_reg_above_64_field32_read(unit, EDB_GLEM_INTERRUPT_REGISTER_ONEr, REG_PORT_ANY, 0, GLEM_MANAGEMENT_COMPLETEDf, &temp);
    SOCDNX_IF_ERR_EXIT(rv);

    /* If the management completed bit is set, the table can't be written to. */
    if (temp){
        SOCDNX_EXIT_WITH_ERR(SOC_E_BUSY, (_BSL_SOCDNX_MSG("Glem table manager is busy")));
    }
exit:
    SOCDNX_FUNC_RETURN;
}

/*********************************************************************
* NAME:
 *   soc_jer_lif_glem_access_write_to_machine
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Given a glem register buffer, writes the buffer to the glem management machine. 
 * INPUT:
 *   int            unit             - (IN) Identifier of the device to access.
 *   uint32         buffer           - (IN) Buffer to write to the management machine.
 * REMARKS:
 *   
 * RETURNS:
 *   SOC_E_BUSY  If the table can't be accessed right now.
 *   SOC_E_***   If there was a problem writing to the register
 *   SOC_E_NONE  Otherwise.
*********************************************************************/
STATIC soc_error_t
soc_jer_lif_glem_access_write_to_machine(int unit, uint32 *buffer){
    int rv = SOC_E_NONE;
    SOCDNX_INIT_FUNC_DEFS;

    SOCDNX_NULL_CHECK(buffer);

    /*
     * Write to machine register.
     */
    rv = soc_mem_write(unit, EDB_GLEM_MANAGEMENT_REQUESTm, EDB_BLOCK(unit), 0, buffer);
    SOCDNX_IF_ERR_EXIT(rv);

    /* 
     * Poll to confirm that writing is complete.
     */

    rv = arad_polling(unit, ARAD_TIMEOUT, ARAD_MIN_POLLS, EDB_GLEM_INTERRUPT_REGISTER_ONEr, REG_PORT_ANY, 0, GLEM_MANAGEMENT_COMPLETEDf, 1);
    SOCDNX_SAND_IF_ERR_EXIT(rv);

    /* 
     * clear management_completed by writing 1
     */
    
    rv = soc_reg_field32_modify(unit, EDB_GLEM_INTERRUPT_REGISTER_ONEr, REG_PORT_ANY, GLEM_MANAGEMENT_COMPLETEDf, 1);
    SOCDNX_IF_ERR_EXIT(rv);

exit:
    SOCDNX_FUNC_RETURN;
}


/*********************************************************************
* NAME:
 *   soc_jer_lif_glem_entry_buffer_set
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Given an action type, glem entry and buffer, fills the buffer with the action to be written to the management machine.
 * INPUT:
 *   int            unit             - (IN) Identifier of the device to access.
 *   uint32         action_type      - (IN) SOC_JER_LIF_GLEM_ACTION_WRITE or SOC_JER_LIF_GLEM_ACTION_REMOVE
 *   uint32         global_lif_id    - (IN) Global lif to be written to the glem table.
 *   uint32         egress_lif_id    - (IN) Egress lif to be written to the glem table.
 *   uint32         buffer           - (OUT) Will be filled with the management command. 
 * REMARKS:
 *   
 * RETURNS:
 *   SOC_E_BUSY  If the table can't be accessed right now.
 *   SOC_E_***   If there was a problem writing to the register
 *   SOC_E_NONE  Otherwise.
*********************************************************************/
STATIC soc_error_t
soc_jer_lif_glem_entry_buffer_set(int unit, uint32 action_type, uint32 global_lif_id, uint32 egress_lif_id, uint32 *buffer){
    SOCDNX_INIT_FUNC_DEFS;

    SOCDNX_NULL_CHECK(buffer);

    /* Reset the buffer. */
    sal_memset(buffer, 0, sizeof(uint32) * SOC_JER_LIF_GLEM_BUFFER_SIZE);

    /* Write action type */
    soc_EDB_GLEM_MANAGEMENT_REQUESTm_field32_set(unit, buffer, GLEM_TYPEf, action_type);

    /* Write global lif */
    soc_EDB_GLEM_MANAGEMENT_REQUESTm_field32_set(unit, buffer, GLEM_KEYf, global_lif_id);

    /* Write egress lif */
    soc_EDB_GLEM_MANAGEMENT_REQUESTm_field32_set(unit, buffer, GLEM_PAYLOADf, egress_lif_id);

    SOC_EXIT;
exit:
    SOCDNX_FUNC_RETURN;
}
#endif /* SOC_DPP_IS_EM_HW_ENABLE */

#include <soc/dpp/SAND/Utils/sand_footer.h>
/* } */

