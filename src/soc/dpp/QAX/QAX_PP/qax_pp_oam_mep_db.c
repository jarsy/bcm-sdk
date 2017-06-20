/*
 * $Id: $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File: qax_pp_oam_mep_db.c
 */

#ifdef _ERR_MSG_MODULE_NAME
#error "_ERR_MSG_MODULE_NAME redefined"
#endif
#define _ERR_MSG_MODULE_NAME BSL_SOC_OAM
#include <shared/bsl.h>

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


#include <soc/dpp/QAX/QAX_PP/qax_pp_oam.h>
#include <soc/dpp/QAX/QAX_PP/qax_pp_oam_mep_db.h>
#include <soc/dpp/PPC/ppc_api_oam.h>


/* } */
/*************
 * DEFINES   *
 *************/
/* { */

/* } */
/*************
 * MACROS    *
 *************/
/* { */

/* MEP DB Access micros. */
#define SOC_QAX_PP_OAM_INTERNAL_READ_OAMP_MEP_DB(unit, blk, index, data) \
            soc_mem_array_read(unit, OAMP_MEP_DBm, OAMP_MEP_DB_ENTRY_ID_TO_BLOCK(index),\
                               blk, OAMP_MEP_DB_ENTRY_ID_TO_INDEX(index), data)

#define SOC_QAX_PP_OAM_INTERNAL_WRITE_OAMP_MEP_DB(unit, blk, index, data) \
            soc_mem_array_write(unit, OAMP_MEP_DBm, OAMP_MEP_DB_ENTRY_ID_TO_BLOCK(index),\
                               blk, OAMP_MEP_DB_ENTRY_ID_TO_INDEX(index), data)

#define SOC_QAX_PP_OAM_INTERNAL_WRITE_OAMP_MEP_DB_DM_STAT_ONE_WAYm(unit, blk, index, data) \
            SOC_QAX_PP_OAM_INTERNAL_WRITE_OAMP_MEP_DB(unit, blk, index, data)
#define SOC_QAX_PP_OAM_INTERNAL_READ_OAMP_MEP_DB_DM_STAT_ONE_WAYm(unit, blk, index, data) \
            SOC_QAX_PP_OAM_INTERNAL_READ_OAMP_MEP_DB(unit, blk, index, data)
#define SOC_QAX_PP_OAM_INTERNAL_WRITE_OAMP_MEP_DB_DM_STAT_TWO_WAYm(unit, blk, index, data) \
            SOC_QAX_PP_OAM_INTERNAL_WRITE_OAMP_MEP_DB(unit, blk, index, data)
#define SOC_QAX_PP_OAM_INTERNAL_READ_OAMP_MEP_DB_DM_STAT_TWO_WAYm(unit, blk, index, data) \
            SOC_QAX_PP_OAM_INTERNAL_READ_OAMP_MEP_DB(unit, blk, index, data)


/* 
 *  The key of OAMP_CLS_FLEX_CRC_TCAM is built of {PE-Profile(6),oam/bfd(1),gach/opcode(16)}.
 *  We can't access the sub fields directly, so we need to macro the hell out of this.
 */
#define QAX_PP_OAMP_CLS_FLEX_CRC_TCAM_OPCODE_LSB        (0) 
#define QAX_PP_OAMP_CLS_FLEX_CRC_TCAM_OPCODE_SIZE       (16)
#define QAX_PP_OAMP_CLS_FLEX_CRC_TCAM_OPCODE_MASK       (SOC_SAND_BITS_MASK(QAX_PP_OAMP_CLS_FLEX_CRC_TCAM_OPCODE_SIZE - 1, 0))
#define QAX_PP_OAMP_CLS_FLEX_CRC_TCAM_OPCODE_GET(key)   \
            (((key) >> QAX_PP_OAMP_CLS_FLEX_CRC_TCAM_OPCODE_LSB) & QAX_PP_OAMP_CLS_FLEX_CRC_TCAM_OPCODE_MASK)
#define QAX_PP_OAMP_CLS_FLEX_CRC_TCAM_OPCODE_SET(key, opcode) \
    (key) |= (((opcode) & QAX_PP_OAMP_CLS_FLEX_CRC_TCAM_OPCODE_MASK) << QAX_PP_OAMP_CLS_FLEX_CRC_TCAM_OPCODE_LSB) 

#define QAX_PP_OAMP_CLS_FLEX_CRC_TCAM_OAM_BFD_LSB       QAX_PP_OAMP_CLS_FLEX_CRC_TCAM_OPCODE_SIZE
#define QAX_PP_OAMP_CLS_FLEX_CRC_TCAM_OAM_BFD_SIZE      (1)
#define QAX_PP_OAMP_CLS_FLEX_CRC_TCAM_OAM_BFD_MASK      (SOC_SAND_BITS_MASK(QAX_PP_OAMP_CLS_FLEX_CRC_TCAM_OAM_BFD_SIZE - 1, 0))
#define QAX_PP_OAMP_CLS_FLEX_CRC_TCAM_OAM_BFD_GET(key)   \
            (((key) >> QAX_PP_OAMP_CLS_FLEX_CRC_TCAM_OAM_BFD_LSB) & QAX_PP_OAMP_CLS_FLEX_CRC_TCAM_OAM_BFD_MASK)
#define QAX_PP_OAMP_CLS_FLEX_CRC_TCAM_OAM_BFD_SET(key, oam_bfd) \
    (key) |= (((oam_bfd) & QAX_PP_OAMP_CLS_FLEX_CRC_TCAM_OAM_BFD_MASK) << QAX_PP_OAMP_CLS_FLEX_CRC_TCAM_OAM_BFD_LSB) 

#define QAX_PP_OAMP_CLS_FLEX_CRC_TCAM_PE_PROFILE_LSB   (QAX_PP_OAMP_CLS_FLEX_CRC_TCAM_OAM_BFD_LSB + QAX_PP_OAMP_CLS_FLEX_CRC_TCAM_OAM_BFD_SIZE)
#define QAX_PP_OAMP_CLS_FLEX_CRC_TCAM_PE_PROFILE_SIZE  (6)
#define QAX_PP_OAMP_CLS_FLEX_CRC_TCAM_PE_PROFILE_MASK  (SOC_SAND_BITS_MASK(QAX_PP_OAMP_CLS_FLEX_CRC_TCAM_PE_PROFILE_SIZE - 1, 0))
    #define QAX_PP_OAMP_CLS_FLEX_CRC_TCAM_PE_PROFILE_GET(key)   \
            (((key) >> QAX_PP_OAMP_CLS_FLEX_CRC_TCAM_PE_PROFILE_LSB) & QAX_PP_OAMP_CLS_FLEX_CRC_TCAM_PE_PROFILE_MASK)
#define QAX_PP_OAMP_CLS_FLEX_CRC_TCAM_PE_PROFILE_SET(key, pe_profile) \
    (key) |= (((pe_profile) & QAX_PP_OAMP_CLS_FLEX_CRC_TCAM_PE_PROFILE_MASK) << QAX_PP_OAMP_CLS_FLEX_CRC_TCAM_PE_PROFILE_LSB) 


/* 
 *  The data of OAMP_CLS_FLEX_CRC_TCAM is built of {CRCVal1/2(1),Mask_Sel(3)}. 
 *  We can't access the sub fields directly, so we need to macro the hell out of this.
 */
#define QAX_PP_OAMP_CLS_FLEX_CRC_TCAM_MASK_SEL_LSB      (0) 
#define QAX_PP_OAMP_CLS_FLEX_CRC_TCAM_MASK_SEL_SIZE     (3)
#define QAX_PP_OAMP_CLS_FLEX_CRC_TCAM_MASK_SEL_MASK     (SOC_SAND_BITS_MASK(0, QAX_PP_OAMP_CLS_FLEX_CRC_TCAM_MASK_SEL_SIZE - 1))
#define QAX_PP_OAMP_CLS_FLEX_CRC_TCAM_MASK_SEL_GET(data)   \
            (((data) >> QAX_PP_OAMP_CLS_FLEX_CRC_TCAM_MASK_SEL_LSB) & QAX_PP_OAMP_CLS_FLEX_CRC_TCAM_MASK_SEL_MASK)
#define QAX_PP_OAMP_CLS_FLEX_CRC_TCAM_MASK_SEL_SET(data, mask_sel) \
    (data) |= (((mask_sel) & QAX_PP_OAMP_CLS_FLEX_CRC_TCAM_MASK_SEL_MASK) << QAX_PP_OAMP_CLS_FLEX_CRC_TCAM_MASK_SEL_LSB) 

#define QAX_PP_OAMP_CLS_FLEX_CRC_TCAM_CRC_SEL_LSB      (QAX_PP_OAMP_CLS_FLEX_CRC_TCAM_MASK_SEL_SIZE) 
#define QAX_PP_OAMP_CLS_FLEX_CRC_TCAM_CRC_SEL_SIZE     (1)
#define QAX_PP_OAMP_CLS_FLEX_CRC_TCAM_CRC_SEL_MASK     (SOC_SAND_BITS_MASK(0, QAX_PP_OAMP_CLS_FLEX_CRC_TCAM_CRC_SEL_SIZE - 1))
#define QAX_PP_OAMP_CLS_FLEX_CRC_TCAM_CRC_SEL_GET(data)   \
            (((data) >> QAX_PP_OAMP_CLS_FLEX_CRC_TCAM_CRC_SEL_LSB) & QAX_PP_OAMP_CLS_FLEX_CRC_TCAM_CRC_SEL_MASK)
#define QAX_PP_OAMP_CLS_FLEX_CRC_TCAM_CRC_SEL_SET(data, crc_sel) \
    (data) |= (((crc_sel) & QAX_PP_OAMP_CLS_FLEX_CRC_TCAM_CRC_SEL_MASK) << QAX_PP_OAMP_CLS_FLEX_CRC_TCAM_CRC_SEL_LSB) 


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

/************************************
 * Static declerations
 ************************************/
/* { */

/*
 * Find where to add entries to the MEP-DB (May return REMOVE action
 * in case of update)
 */
STATIC
soc_error_t soc_qax_pp_oam_oamp_lm_dm_pointed_shared_find__add_update(
   int unit,
   ARAD_PP_OAM_OAMP_MEP_DB_LM_DM_ENTRIES_INFO *lm_dm_info
   );

/*
 * When removing LM/DM entry, this functions finds what specific
 * entry to remove
 */
STATIC
soc_error_t soc_qax_pp_oam_oamp_lm_dm_pointed_shared_find__remove(
   int unit,
   ARAD_PP_OAM_OAMP_MEP_DB_LM_DM_ENTRIES_INFO *lm_dm_info
   );

/*
 * Scan the MEP-DB starting with the supplied MEP and find
 * what has already been added to it (LM/DM/LM_STAT) and at which entries
 */
STATIC
soc_error_t soc_qax_pp_oam_oamp_lm_dm_mep_scan(
   int unit,
   ARAD_PP_OAM_OAMP_MEP_DB_LM_DM_ENTRIES_INFO *lm_dm_info
   );

/*
 * In order to add a shared entry, need to allocate it. This function
 * adds allocation request to expand/create the LM/DM entry chain to the
 * shared opbject lm_dm_info.
 */
STATIC
soc_error_t soc_qax_pp_oam_oamp_lm_dm_shared_entry_alloc_list_tail_add(
   int unit,
   ARAD_PP_OAM_OAMP_MEP_DB_LM_DM_ENTRIES_INFO *lm_dm_info
   );

/* Set the pointer to either the new LM/DM chain in the MEP entry or the flexible packet generation data entry. */
STATIC
soc_error_t soc_qax_pp_oam_mep_db_ptr_set(
   int unit,
   uint32 mep_idx,
   soc_field_t field,
   uint32 first_entry
   );

/* Get the pointer to either the new LM/DM chain in the MEP entry or the flexible packet generation data entry. */
STATIC
soc_error_t soc_qax_pp_oam_mep_db_ptr_get(
   int unit,
   uint32 mep_idx,
   soc_field_t field,
   int *first_entry
   );

/* Clear the 'LAST' bit from the former last entry after adding a new
   entry to an LM/DM entry chain */
STATIC
soc_error_t soc_qax_pp_oam_mep_db_lm_dm_last_bit_write(
   int unit,
   ARAD_PP_OAM_OAMP_MEP_DB_LM_DM_ENTRIES_INFO *lm_dm_info,
   uint32 entry,
   uint8 val
   );

/*
 * soc_qax_pp_oam_oamp_lm_dm_shared_scan -
 * Prepares the data required for handeling the QAX style MEP-DB for
 * auxiliary functions.
 */
STATIC
soc_error_t soc_qax_pp_oam_oamp_lm_dm_shared_scan(
   int unit,
   int endpoint_id,
   ARAD_PP_OAM_OAMP_MEP_DB_LM_DM_ENTRIES_INFO *lm_dm_info
   );

/*
 * When deleting an entry, it needs to be added to a list of entries
 * that would be freed from the resource manager. This function adds
 * the entry to the list held in the shared object lm_dm_info
 */
STATIC
soc_error_t soc_qax_pp_oam_oamp_lm_dm_shared_entry_remove_list_add(
   int unit,
   ARAD_PP_OAM_OAMP_MEP_DB_LM_DM_ENTRIES_INFO *lm_dm_info,
   uint32 entry
   );

    /*
 * Function:
 *    _arad_pp_oam_internal_no_cache_read_oamp_mep_DBm
 * Purpose:
 *    Read HW MEP DM memory.
 *    When reading MEP DB, cached memory might not be
 *    updated according to the latest packet.
 *    This function cam be used to read the updated values
 *    without using the cache memory.
 */
STATIC
int _arad_pp_oam_internal_no_cache_read_oamp_mep_DBm(
   int unit,
   uint32 flags,
   int blk,
   uint32 index,
   void *data
   );

/*  
 * Read DM data and clears it from the reg_data buffer ('clear on
 * read'). The reg_data should be written back after this function
 */
STATIC
void _soc_qax_pp_oam_dm_two_way_entry_read(
   int unit,
   SOC_PPC_OAM_OAMP_DM_INFO_GET *dm_info,
   soc_reg_above_64_val_t reg_data
   );

/*  
 * Extract LM data from a LM_DB entry buffer 
 */
STATIC
void _soc_qax_pp_oam_lm_entry_read(
   int unit,
   SOC_PPC_OAM_OAMP_LM_INFO_GET *lm_info,
   soc_reg_above_64_val_t reg_data
   );

/*  
 * Read LM-STAT data and clears it from the reg_data buffer ('clear on
 * read'). The reg_data should be written back after this function.
 */
STATIC
void _soc_qax_pp_oam_lm_stat_entry_read(
   int unit,
   SOC_PPC_OAM_OAMP_LM_INFO_GET *lm_info,
   soc_reg_above_64_val_t reg_data
   );

/* } */
/************************************/

/************************************
 * Internal API implementation
 ************************************/
/* { */

/*
 * Find where to add or delete entries to/from the MEP-DB
 * for loss/delay add/update/delete
 */
soc_error_t
soc_qax_pp_oam_oamp_lm_dm_pointed_shared_find(int unit,
                                              ARAD_PP_OAM_OAMP_MEP_DB_LM_DM_ENTRIES_INFO *lm_dm_info) {

    SOCDNX_INIT_FUNC_DEFS;

    /* What do this MEP already has? */
    SOCDNX_IF_ERR_EXIT(
       soc_qax_pp_oam_oamp_lm_dm_mep_scan(unit, lm_dm_info));

    /* Dispatch the relevant find procedure */
    if (lm_dm_info->action_type == ARAD_PP_OAM_OAMP_MEP_DB_LM_DM_ENTRIES_ACTION_TYPE_ADD_UPDATE) {
        SOCDNX_IF_ERR_EXIT(
           soc_qax_pp_oam_oamp_lm_dm_pointed_shared_find__add_update(unit, lm_dm_info));
    } else {
        SOCDNX_IF_ERR_EXIT(
           soc_qax_pp_oam_oamp_lm_dm_pointed_shared_find__remove(unit, lm_dm_info));
    }
exit:
    SOCDNX_FUNC_RETURN;
}

/*
 * Add/Remove/Update LM/DM entries in the MEP-DB.
 * This assumes that soc_qax_pp_oam_oamp_lm_dm_pointed_shared_find was
 * already called and that the required entries were allocated.
 */
soc_error_t soc_qax_pp_oam_oamp_lm_dm_set(int unit,
                                          ARAD_PP_OAM_OAMP_MEP_DB_LM_DM_ENTRIES_INFO *lm_dm_info) {
    soc_error_t res;
    soc_reg_above_64_val_t reg_data, min_delay_field;

    SOCDNX_INIT_FUNC_DEFS;

    SOC_REG_ABOVE_64_CLEAR(reg_data);

    switch (lm_dm_info->action) {

    case ARAD_PP_OAM_OAMP_MEP_DB_LM_DM_ACTION_NONE:
        break;  /* Nothing to do */


    case ARAD_PP_OAM_OAMP_MEP_DB_LM_DM_ACTION_ADD_DM_1WAY:
        /* Just set the neccessary entry with preliminary values and LAST bit
           Clearing the last bit from the previous last-entry is done later */

        
        SOCDNX_EXIT_WITH_ERR(SOC_E_UNAVAIL, (_BSL_SOC_MSG("QAX 1-DM Not implemented yet")));
        break;


    case ARAD_PP_OAM_OAMP_MEP_DB_LM_DM_ACTION_ADD_DM_2WAY:
        /* Just set the neccessary entry with preliminary values and LAST bit
           Clearing the last bit from the previous last-entry is done later */
        soc_OAMP_MEP_DB_DM_STAT_ONE_WAYm_field32_set(unit, reg_data, MEP_TYPEf, SOC_PPC_OAM_MEP_TYPE_DM);
        SOC_REG_ABOVE_64_CLEAR(min_delay_field);
        SHR_BITSET_RANGE(min_delay_field, 0, soc_mem_field_length(unit, OAMP_MEP_DB_DM_STAT_TWO_WAYm, MIN_DELAYf));
        soc_OAMP_MEP_DB_DM_STAT_TWO_WAYm_field_set(unit, reg_data, MIN_DELAYf, min_delay_field);
        soc_OAMP_MEP_DB_DM_STAT_TWO_WAYm_field32_set(unit, reg_data, LAST_ENTRYf, 1);
        res = SOC_QAX_PP_OAM_INTERNAL_WRITE_OAMP_MEP_DB_DM_STAT_TWO_WAYm(unit, MEM_BLOCK_ALL,
                                                                         lm_dm_info->entries_to_add[0], reg_data);
        SOCDNX_IF_ERR_EXIT(res);
        break;


    case ARAD_PP_OAM_OAMP_MEP_DB_LM_DM_ACTION_ADD_LM:
    case ARAD_PP_OAM_OAMP_MEP_DB_LM_DM_ACTION_ADD_LM_WITH_STAT:
        /* First, add an LM entry. If not adding stat, set LAST bit. */
        soc_OAMP_MEP_DB_LM_DBm_field32_set(unit, reg_data, MEP_TYPEf, SOC_PPC_OAM_MEP_TYPE_LM);
        soc_OAMP_MEP_DB_LM_DBm_field32_set(unit, reg_data, LAST_ENTRYf,
                                           (lm_dm_info->action == ARAD_PP_OAM_OAMP_MEP_DB_LM_DM_ACTION_ADD_LM));
        soc_OAMP_MEP_DB_LM_DBm_field32_set(unit, reg_data, LM_CNTRS_VALIDf, 0);
        res = _ARAD_PP_OAM_INTERNAL_WRITE_OAMP_MEP_DB_LM_DBm(unit, MEM_BLOCK_ALL,
                                                             lm_dm_info->entries_to_add[0], reg_data);
        SOCDNX_IF_ERR_EXIT(res);
        if (lm_dm_info->action == ARAD_PP_OAM_OAMP_MEP_DB_LM_DM_ACTION_ADD_LM) {
            /* Not adding stat, break the switch statment. */
            break;
        }
        /* else - No break on purpose - add LM-Stat   */
    case ARAD_PP_OAM_OAMP_MEP_DB_LM_DM_ACTION_ADD_LM_STAT:
        /* Just set the neccessary entry with preliminary values and LAST bit
           Clearing the last bit from the previous last-entry is done later */
        soc_OAMP_MEP_DB_LM_STATm_field32_set(unit, reg_data, MEP_TYPEf, SOC_PPC_OAM_MEP_TYPE_LM_STAT);
        soc_OAMP_MEP_DB_LM_STATm_field32_set(unit, reg_data, LAST_ENTRYf, 1);
        res = _ARAD_PP_OAM_INTERNAL_WRITE_OAMP_MEP_DB_LM_STATm(unit, MEM_BLOCK_ALL,
                                                               lm_dm_info->entries_to_add[lm_dm_info->num_entries_to_add - 1], reg_data);
        SOCDNX_IF_ERR_EXIT(res);
        break;


    case ARAD_PP_OAM_OAMP_MEP_DB_LM_DM_ACTION_REMOVE_LM_STAT:
        /* Delete already has a handling logic for that. */
        res = soc_qax_pp_oam_oamp_lm_dm_delete(unit, lm_dm_info);
        SOCDNX_IF_ERR_EXIT(res);
        break;


    default:
        SOCDNX_EXIT_WITH_ERR(SOC_E_INTERNAL,
                             (_BSL_SOCDNX_MSG("Unsupported MEP-DB add/update action.")));
    }

    if (lm_dm_info->action != ARAD_PP_OAM_OAMP_MEP_DB_LM_DM_ACTION_NONE) {
        /* If this is the 1st LM/DM entry added, set the pointer from the MEP */
        if (lm_dm_info->mep_entry.lm_dm_ptr == 0) {
            res = soc_qax_pp_oam_mep_db_ptr_set(unit, lm_dm_info->endpoint_id, FLEX_LM_DM_PTRf, lm_dm_info->entries_to_add[0]);
            SOCDNX_IF_ERR_EXIT(res);

        }
        /* If this is not the first, and a new entry was added, clear the LAST
           bit in the previous chain-tail */
        else if (lm_dm_info->action != ARAD_PP_OAM_OAMP_MEP_DB_LM_DM_ACTION_REMOVE_LM_STAT) {
            res = soc_qax_pp_oam_mep_db_lm_dm_last_bit_write(unit, lm_dm_info, lm_dm_info->last_entry, 0);
            SOCDNX_IF_ERR_EXIT(res);
        }
    }

exit:
    SOCDNX_FUNC_RETURN;
}

/*
 * Remove LM/DM entries in the MEP-DB. This assumes that
 * soc_qax_pp_oam_oamp_lm_dm_pointed_shared_find was already called.
 */
soc_error_t soc_qax_pp_oam_oamp_lm_dm_delete(int unit,
                                             ARAD_PP_OAM_OAMP_MEP_DB_LM_DM_ENTRIES_INFO *lm_dm_info) {

    soc_error_t res;
    uint32 entry;
    uint32 new_last_entry;
    int entries_to_remove = 1;

    soc_reg_above_64_val_t reg_data;

    SOCDNX_INIT_FUNC_DEFS;

    switch (lm_dm_info->action) {


    case ARAD_PP_OAM_OAMP_MEP_DB_LM_DM_ACTION_REMOVE_LM:
        /* How many entries should be freed */
        entries_to_remove += (lm_dm_info->lm_stat_entry > 0); /* LM_STAT exist => remove another.*/

        /* Only a DM entry may be left (LM is removed with LM_STAT if exists) */
        if (lm_dm_info->dm_entry) {
            if (lm_dm_info->dm_entry != lm_dm_info->last_entry) {
                /* Set the LAST bit on the DM entry */
                res = soc_qax_pp_oam_mep_db_lm_dm_last_bit_write(unit, lm_dm_info, lm_dm_info->dm_entry, 1);
                SOCDNX_IF_ERR_EXIT(res);
            }
            if (lm_dm_info->dm_entry != lm_dm_info->mep_entry.lm_dm_ptr) {
                /* Move the DM entry to the chain head */
                res = _ARAD_PP_OAM_INTERNAL_READ_OAMP_MEP_DBm(unit, MEM_BLOCK_ANY,
                                                              lm_dm_info->dm_entry, reg_data);
                SOCDNX_IF_ERR_EXIT(res);
                res = _ARAD_PP_OAM_INTERNAL_WRITE_OAMP_MEP_DBm(unit, MEM_BLOCK_ANY,
                                                               lm_dm_info->mep_entry.lm_dm_ptr, reg_data);
                SOCDNX_IF_ERR_EXIT(res);
            }
        } else {
            /* No DM also. Nip the chain */
            res = soc_qax_pp_oam_mep_db_ptr_set(unit, lm_dm_info->endpoint_id, FLEX_LM_DM_PTRf, 0);
            SOCDNX_IF_ERR_EXIT(res);
        }
        break;


    case ARAD_PP_OAM_OAMP_MEP_DB_LM_DM_ACTION_REMOVE_DM:
    case ARAD_PP_OAM_OAMP_MEP_DB_LM_DM_ACTION_REMOVE_LM_STAT:
        entry = (lm_dm_info->action == ARAD_PP_OAM_OAMP_MEP_DB_LM_DM_ACTION_REMOVE_DM) ?
           lm_dm_info->dm_entry :
           lm_dm_info->lm_stat_entry;
        /*
         * Suppose we want to remove a DM entry, there are 3 options:
         * 1. MEP -> DM (LAST)
         *          ==> only nip the chain (set MEP.ptr to 0)
         * 2. MEP -> STUFF , ... , DM (LAST)
         *          ==>  only turn the LAST bit on the new last entry
         * 3. MEP -> STUFF , ... , DM , More STUFF , ... , more STUFF (LAST)
         *          ==> copy everything after the DM backwards
         * For LM_STAT, the same logic holds.
         */
        if (entry == lm_dm_info->last_entry) { /* options (1) or (2) */
            if (lm_dm_info->mep_entry.lm_dm_ptr == entry) { /* option (1) */
                /* Nip the chain */
                res = soc_qax_pp_oam_mep_db_ptr_set(unit, lm_dm_info->endpoint_id, FLEX_LM_DM_PTRf, 0);
                SOCDNX_IF_ERR_EXIT(res);
            } else { /* option (2) */
                /* Set the LAST bit on the new last entry */
                new_last_entry = ARAD_PP_OAM_OAMP_MEP_DB_LM_DM_CHAIN_PREV(unit, entry);
                res = soc_qax_pp_oam_mep_db_lm_dm_last_bit_write(unit, lm_dm_info, new_last_entry, 1);
                SOCDNX_IF_ERR_EXIT(res);
            }
        } else { /* option (3) */
            /* Copy everything after entry backwards */
            while (entry < lm_dm_info->last_entry) {
                uint32 next_entry = ARAD_PP_OAM_OAMP_MEP_DB_LM_DM_CHAIN_NEXT(unit, entry);
                SOC_REG_ABOVE_64_CLEAR(reg_data);
                res = _ARAD_PP_OAM_INTERNAL_READ_OAMP_MEP_DBm(unit, MEM_BLOCK_ANY, next_entry, reg_data);
                SOCDNX_IF_ERR_EXIT(res);
                res = _ARAD_PP_OAM_INTERNAL_WRITE_OAMP_MEP_DBm(unit, MEM_BLOCK_ANY, entry, reg_data);
                SOCDNX_IF_ERR_EXIT(res);
                entry = next_entry;
            }
        }
        break;


    default:
        SOCDNX_EXIT_WITH_ERR(SOC_E_INTERNAL,
                             (_BSL_SOCDNX_MSG("Unsupported MEP-DB remove action.")));
    }

    /* Add the freed entries to the deallocation list */
    res = soc_qax_pp_oam_oamp_lm_dm_shared_entry_remove_list_add(unit, lm_dm_info, lm_dm_info->last_entry);
    SOCDNX_IF_ERR_EXIT(res);
    entry = lm_dm_info->last_entry;
    while (--entries_to_remove) {
        entry = ARAD_PP_OAM_OAMP_MEP_DB_LM_DM_CHAIN_PREV(unit, entry);
        res = soc_qax_pp_oam_oamp_lm_dm_shared_entry_remove_list_add(unit, lm_dm_info, entry);
        SOCDNX_IF_ERR_EXIT(res);
    }

exit:
    SOCDNX_FUNC_RETURN;
}

/*
 * Search for LM and DM in the MEP DB.
 */
soc_error_t
soc_qax_pp_oam_oamp_lm_dm_search(int unit, uint32 endpoint_id, uint32 *found_bmp) {

    uint32 found_bitmap_lcl[1];

    ARAD_PP_OAM_OAMP_MEP_DB_LM_DM_ENTRIES_INFO lm_dm_info = { 0 };

    SOCDNX_INIT_FUNC_DEFS;

    /* Preperations */
    SOCDNX_IF_ERR_EXIT(soc_qax_pp_oam_oamp_lm_dm_shared_scan(unit,
                                                             endpoint_id,
                                                             &lm_dm_info));

    /*
     * For every entry found, set the required bit.
     */
    *found_bitmap_lcl = 0;
    if (lm_dm_info.lm_entry > 0) {
        SHR_BITSET(found_bitmap_lcl, SOC_PPC_OAM_MEP_TYPE_LM);
    }
    if (lm_dm_info.lm_stat_entry > 0) {
        SHR_BITSET(found_bitmap_lcl, SOC_PPC_OAM_MEP_TYPE_LM_STAT);
    }
    if (lm_dm_info.dm_entry > 0) {
        SHR_BITSET(found_bitmap_lcl, SOC_PPC_OAM_MEP_TYPE_DM);
    }

    *found_bmp = *found_bitmap_lcl;

exit:
    SOCDNX_FUNC_RETURN;
}

/*
 * Get delay measurement data.
 * If the Delay entry is first in the chain, the dm_info->entry_id will
 * be overwritten with the entry to allow the API to return delay_id to
 * the user.
 */
soc_error_t
soc_qax_pp_oam_oamp_dm_get(int unit, SOC_PPC_OAM_OAMP_DM_INFO_GET *dm_info, uint8 *is_1DM) {
    soc_error_t res;
    uint32 flags = SOC_MEM_NO_FLAGS;

    ARAD_PP_OAM_OAMP_MEP_DB_LM_DM_ENTRIES_INFO lm_dm_info = { 0 };

    soc_reg_above_64_val_t reg_data;

    SOCDNX_INIT_FUNC_DEFS;

    /* Preperations */
    SOCDNX_IF_ERR_EXIT(soc_qax_pp_oam_oamp_lm_dm_shared_scan(unit,
                                                             dm_info->entry_id,
                                                             &lm_dm_info));

    /* Some validation */
    if (lm_dm_info.dm_entry == 0) {
        SOCDNX_EXIT_WITH_ERR(SOC_E_PARAM,
                             (_BSL_SOCDNX_MSG("No DM entry for endpoint %d "),dm_info->entry_id));
    }

    /* Read the found entry */
    SOC_REG_ABOVE_64_CLEAR(reg_data);
    res = _arad_pp_oam_internal_no_cache_read_oamp_mep_DBm(unit, flags, MEM_BLOCK_ANY, lm_dm_info.dm_entry, &reg_data);
    SOCDNX_IF_ERR_EXIT(res);

    switch (soc_OAMP_MEP_DBm_field32_get(unit, reg_data, MEP_TYPEf)) {
    case SOC_PPC_OAM_MEP_TYPE_DM:
        *is_1DM = FALSE;
        _soc_qax_pp_oam_dm_two_way_entry_read(unit, dm_info, reg_data);
        res = SOC_QAX_PP_OAM_INTERNAL_WRITE_OAMP_MEP_DB_DM_STAT_TWO_WAYm(unit, MEM_BLOCK_ALL, lm_dm_info.dm_entry, reg_data);
        SOCDNX_IF_ERR_EXIT(res);
        break;
    case SOC_PPC_OAM_MEP_TYPE_DM_ONE_WAY:
        
        SOCDNX_EXIT_WITH_ERR(SOC_E_UNAVAIL, (_BSL_SOC_MSG("QAX 1-DM Not implemented yet")));
        break;
    default:
        SOCDNX_EXIT_WITH_ERR(SOC_E_INTERNAL,
                             (_BSL_SOCDNX_MSG("Something went wrong")));
    }

    /* Check if the entry needs to be returned */
    if (lm_dm_info.mep_entry.lm_dm_ptr == lm_dm_info.dm_entry) {
        /* DM entry is the 1st in the chain. Return the DM-entry as entry_id */
        dm_info->entry_id = lm_dm_info.dm_entry;
    } else {
        /* DM entry is later in the chain. Return entry_id=0 */
        dm_info->entry_id = 0;
    }

exit:
    SOCDNX_FUNC_RETURN;

}

/*
 * Get loss measurement data.
 * If the Loss entry is first in the chain, the lm_info->entry_id will
 * be overwritten with the entry to allow the API to return loss_id to
 * the user.
 */
soc_error_t
soc_qax_pp_oam_oamp_lm_get(int unit, SOC_PPC_OAM_OAMP_LM_INFO_GET *lm_info) {
    soc_error_t res;
    uint32 flags = SOC_MEM_NO_FLAGS;

    ARAD_PP_OAM_OAMP_MEP_DB_LM_DM_ENTRIES_INFO lm_dm_info = { 0 };

    soc_reg_above_64_val_t reg_data;

    SOCDNX_INIT_FUNC_DEFS;

    /* Preperations */
    SOCDNX_IF_ERR_EXIT(soc_qax_pp_oam_oamp_lm_dm_shared_scan(unit,
                                                             lm_info->entry_id,
                                                             &lm_dm_info));

    /* Some validation */
    if (lm_dm_info.lm_entry == 0) {
        SOCDNX_EXIT_WITH_ERR(SOC_E_PARAM,
                             (_BSL_SOCDNX_MSG("No LM entry for endpoint %d "),lm_info->entry_id));
    }

    /* Read the found entry */
    SOC_REG_ABOVE_64_CLEAR(reg_data);
    res = _arad_pp_oam_internal_no_cache_read_oamp_mep_DBm(unit, flags, MEM_BLOCK_ANY, lm_dm_info.lm_entry, &reg_data);
    SOCDNX_IF_ERR_EXIT(res);
    _soc_qax_pp_oam_lm_entry_read(unit, lm_info, reg_data);

    /*Now see if extended statistics are available.*/
    lm_info->is_extended = (lm_dm_info.lm_stat_entry > 0);
    if (lm_info->is_extended) {
        /* Read the LM-STAT entry */
        SOC_REG_ABOVE_64_CLEAR(reg_data);
        res = _arad_pp_oam_internal_no_cache_read_oamp_mep_DBm(unit, flags, MEM_BLOCK_ANY, lm_dm_info.lm_stat_entry, &reg_data);
        SOCDNX_IF_ERR_EXIT(res);

        _soc_qax_pp_oam_lm_stat_entry_read(unit, lm_info, reg_data);

        res = _ARAD_PP_OAM_INTERNAL_WRITE_OAMP_MEP_DBm(unit, MEM_BLOCK_ANY, lm_dm_info.lm_stat_entry, reg_data);
        SOCDNX_IF_ERR_EXIT(res);

    }

    /* Check if the entry needs to be returned */
    if (lm_dm_info.mep_entry.lm_dm_ptr == lm_dm_info.lm_entry) {
        /* LM is the 1st in the chain. Return the LM-entry as entry_id */
        lm_info->entry_id = lm_dm_info.lm_entry;
    } else {
        /* LM entry is later in the chain. Return entry_id=0 */
        lm_info->entry_id = 0;
    }

exit:
    SOCDNX_FUNC_RETURN;

}

/* } */

/**********************************
 * Static utils implementation
 **********************************/
/* { */

/* See static declerations for doc */
STATIC
int _arad_pp_oam_internal_no_cache_read_oamp_mep_DBm(int unit, uint32 flags, int blk, uint32 index, void *data) {
    int rv;

    flags |= SOC_MEM_DONT_USE_CACHE;

    if (SOC_IS_QAX(unit)) {

        MEM_LOCK(unit, OAMP_MEP_DBm);
        rv = soc_mem_array_read_flags(unit, OAMP_MEP_DBm, OAMP_MEP_DB_ENTRY_ID_TO_BLOCK(index), blk, OAMP_MEP_DB_ENTRY_ID_TO_INDEX(index), data, flags);
        MEM_UNLOCK(unit, OAMP_MEP_DBm);

    } else {
        rv = soc_mem_read_no_cache(unit, flags, OAMP_MEP_DBm, 0, blk, index, data);
    }
    return rv;
}

/* See static declerations for doc */
STATIC
soc_error_t soc_qax_pp_oam_oamp_lm_dm_pointed_shared_find__add_update(int unit,
                   ARAD_PP_OAM_OAMP_MEP_DB_LM_DM_ENTRIES_INFO *lm_dm_info) {

    soc_error_t res;

    SOCDNX_INIT_FUNC_DEFS;

    if (lm_dm_info->lm_dm_entry->is_update) { /* UPDATE */

        /* Check if new entry is needed */
        if (lm_dm_info->lm_dm_entry->entry_type == SOC_PPC_OAM_LM_DM_ENTRY_TYPE_LM_STAT) {
            if (lm_dm_info->lm_stat_entry == 0) {
                /* Update LM -> LM+STAT */
                lm_dm_info->action = ARAD_PP_OAM_OAMP_MEP_DB_LM_DM_ACTION_ADD_LM_STAT;
                res = soc_qax_pp_oam_oamp_lm_dm_shared_entry_alloc_list_tail_add(unit, lm_dm_info);
                SOCDNX_IF_ERR_EXIT(res);
            }
            /* else - Nothing to do   */
        } else {
            /* update LM+STAT -> LM */
            if (lm_dm_info->lm_stat_entry > 0) {
                lm_dm_info->action = ARAD_PP_OAM_OAMP_MEP_DB_LM_DM_ACTION_REMOVE_LM_STAT;
            }
            /* else - Nothing to do   */
        }

    } /* UPDATE end */

    else { /* ADD */

        int entries_to_allocate = 1;
        lm_dm_info->action = ARAD_PP_OAM_OAMP_MEP_DB_LM_DM_ACTION_ADD_LM;

        switch (lm_dm_info->lm_dm_entry->entry_type) {

        case SOC_PPC_OAM_LM_DM_ENTRY_TYPE_LM_STAT:
            /* Add LM+STAT */
            lm_dm_info->action = ARAD_PP_OAM_OAMP_MEP_DB_LM_DM_ACTION_ADD_LM_WITH_STAT;
            entries_to_allocate = 2;
            /* No break on purpose */
        case SOC_PPC_OAM_LM_DM_ENTRY_TYPE_LM:
            /* ADD LM or LM+STAT */
            if (lm_dm_info->lm_entry) {
                SOCDNX_EXIT_WITH_ERR(SOC_E_PARAM,
                                     (_BSL_SOCDNX_MSG("Loss measurement was already added to MEP")));
            }
            break;

        case SOC_PPC_OAM_LM_DM_ENTRY_TYPE_DM:
            /* ADD DM */
            if (lm_dm_info->dm_entry) {
                SOCDNX_EXIT_WITH_ERR(SOC_E_PARAM,
                                     (_BSL_SOCDNX_MSG("Delay measurement was already added to MEP")));
            }
            lm_dm_info->action = ARAD_PP_OAM_OAMP_MEP_DB_LM_DM_ACTION_ADD_DM_2WAY;
            break;

        default:
            SOCDNX_EXIT_WITH_ERR(SOC_E_INTERNAL,
                                 (_BSL_SOCDNX_MSG("Undefined action.")));

        }

        /* Add the decided nof. entries to the allocation list. */
        for (; entries_to_allocate > 0; --entries_to_allocate) {
            res = soc_qax_pp_oam_oamp_lm_dm_shared_entry_alloc_list_tail_add(unit, lm_dm_info);
            SOCDNX_IF_ERR_EXIT(res);
        }
    } /* ADD end */

exit:
    SOCDNX_FUNC_RETURN;
}

/* See static declerations for doc */
STATIC
soc_error_t soc_qax_pp_oam_oamp_lm_dm_pointed_shared_find__remove(int unit,
                ARAD_PP_OAM_OAMP_MEP_DB_LM_DM_ENTRIES_INFO *lm_dm_info) {

    SOCDNX_INIT_FUNC_DEFS;

    switch (lm_dm_info->action_type) {


    case ARAD_PP_OAM_OAMP_MEP_DB_LM_DM_ENTRIES_ACTION_TYPE_REMOVE_LM:
        /* Remove loss measurement */
        if (lm_dm_info->lm_entry == 0) {
            SOCDNX_EXIT_WITH_ERR(SOC_E_NOT_FOUND,
                                 (_BSL_SOCDNX_MSG("Loss entry not found.")));
        }
        lm_dm_info->action = ARAD_PP_OAM_OAMP_MEP_DB_LM_DM_ACTION_REMOVE_LM;
        break;


    case ARAD_PP_OAM_OAMP_MEP_DB_LM_DM_ENTRIES_ACTION_TYPE_REMOVE_DM:
        /* Remove delay measurement */
        if (lm_dm_info->dm_entry == 0) {
            SOCDNX_EXIT_WITH_ERR(SOC_E_NOT_FOUND,
                                 (_BSL_SOCDNX_MSG("Delay entry not found.")));
        }
        lm_dm_info->action = ARAD_PP_OAM_OAMP_MEP_DB_LM_DM_ACTION_REMOVE_DM;
        break;

    default:
        SOCDNX_EXIT_WITH_ERR(SOC_E_INTERNAL,
                             (_BSL_SOCDNX_MSG("Undefined action.")));

    }
exit:
    SOCDNX_FUNC_RETURN;
}

/* See static declerations for doc */
STATIC
soc_error_t soc_qax_pp_oam_oamp_lm_dm_mep_scan(int unit,
                                ARAD_PP_OAM_OAMP_MEP_DB_LM_DM_ENTRIES_INFO *lm_dm_info) {

    soc_error_t res = SOC_E_NONE;
    uint32 entry;
    uint8 last_bit = 0;
    int chain_count;
    SOC_PPC_OAM_MEP_TYPE entry_type;
    soc_reg_above_64_val_t reg_above_64;

    SOCDNX_INIT_FUNC_DEFS;

    /* Is there even an entry chain? */
    if (lm_dm_info->mep_entry.lm_dm_ptr) {
        /* Scan it */
        entry = lm_dm_info->mep_entry.lm_dm_ptr;
        /* Iterate over all the entries in the LM-DM-Chain */
        for (chain_count = 0;
             (chain_count < ARAD_PP_OAM_OAMP_MEP_DB_LM_DM_SHARED_MAX_CHAIN_LEN(unit)) &&
             (last_bit == 0) &&
             (entry < ARAD_PP_OAM_OAMP_MEP_DB_MEP_ENTRIES_NOF(unit));
             ++chain_count) {
            SOC_REG_ABOVE_64_CLEAR(reg_above_64);
            /* Read the current entry */
            res = _ARAD_PP_OAM_INTERNAL_READ_OAMP_MEP_DBm(unit, MEM_BLOCK_ANY, entry, reg_above_64);
            SOCDNX_IF_ERR_EXIT(res);
            /* Extract the entry type from the read entry */
            entry_type = soc_OAMP_MEP_DBm_field32_get(unit, reg_above_64, MEP_TYPEf);
            /* Extract last entry bit according to the entry type */
            switch (entry_type) {
            case SOC_PPC_OAM_MEP_TYPE_DM:
                lm_dm_info->dm_entry = entry;
                last_bit = soc_OAMP_MEP_DB_DM_STAT_TWO_WAYm_field32_get(unit, reg_above_64, LAST_ENTRYf);
                break;
            case SOC_PPC_OAM_MEP_TYPE_DM_ONE_WAY:
                lm_dm_info->dm_entry = entry;
                last_bit = soc_OAMP_MEP_DB_DM_STAT_ONE_WAYm_field32_get(unit, reg_above_64, LAST_ENTRYf);
                break;
            case SOC_PPC_OAM_MEP_TYPE_LM:
                lm_dm_info->lm_entry = entry;
                last_bit = soc_OAMP_MEP_DB_LM_DBm_field32_get(unit, reg_above_64, LAST_ENTRYf);
                break;
            case SOC_PPC_OAM_MEP_TYPE_LM_STAT:
                lm_dm_info->lm_stat_entry = entry;
                last_bit = soc_OAMP_MEP_DB_LM_STATm_field32_get(unit, reg_above_64, LAST_ENTRYf);
                break;
            default:
                /* Should never get here */
                SOCDNX_EXIT_WITH_ERR(SOC_E_INTERNAL,
                                     (_BSL_SOCDNX_MSG("LM/DM chain ended abraptly without LAST_ENTRY bit set.")));
            }
            lm_dm_info->last_entry = entry;
            entry = ARAD_PP_OAM_OAMP_MEP_DB_LM_DM_CHAIN_NEXT(unit, entry);
        }
    } else {
        /* No chain, make sure last_entry shows that */
        lm_dm_info->last_entry = 0;
    }

    /* Set entry scanned */
    lm_dm_info->mep_scanned = TRUE;

exit:
    SOCDNX_FUNC_RETURN;
}

/* See static declerations for doc */
STATIC
soc_error_t soc_qax_pp_oam_oamp_lm_dm_shared_entry_alloc_list_tail_add(int unit,
                      ARAD_PP_OAM_OAMP_MEP_DB_LM_DM_ENTRIES_INFO *lm_dm_info) {
    uint32 entry;

    SOCDNX_INIT_FUNC_DEFS;

    if (lm_dm_info->num_entries_to_add) {
        /* This is not the 1st time this function is called
           for this action... */
        entry = ARAD_PP_OAM_OAMP_MEP_DB_LM_DM_CHAIN_NEXT(unit,
                                                         lm_dm_info->entries_to_add[lm_dm_info->num_entries_to_add - 1]);
    } else if (lm_dm_info->last_entry == 0) {
        /* No LM/DM entries yet.
         * Pointer to where to allocate the first is supplied
         */
        entry = lm_dm_info->lm_dm_entry->lm_dm_id;
    } else {
        /* Request to allocate the next entry in the chain if possible. */
        entry = ARAD_PP_OAM_OAMP_MEP_DB_LM_DM_CHAIN_NEXT(unit, lm_dm_info->last_entry);
    }

    if (entry > ARAD_PP_OAM_OAMP_MEP_DB_MEP_ENTRIES_NOF(unit)) {
        SOCDNX_EXIT_WITH_ERR(SOC_E_INTERNAL,
                             (_BSL_SOCDNX_MSG("No room to add another LM/DM statistics entry.")));
    }
    lm_dm_info->entries_to_add[lm_dm_info->num_entries_to_add++] = entry;

exit:
    SOCDNX_FUNC_RETURN;
}

/* See static declerations for doc */
STATIC
soc_error_t soc_qax_pp_oam_oamp_lm_dm_shared_entry_remove_list_add(int unit,
                            ARAD_PP_OAM_OAMP_MEP_DB_LM_DM_ENTRIES_INFO *lm_dm_info, uint32 entry) {
    SOCDNX_INIT_FUNC_DEFS;

    lm_dm_info->entries_to_remove[lm_dm_info->num_entries_to_remove++] = entry;

    SOC_EXIT;

exit:
    SOCDNX_FUNC_RETURN;
}

/* See static declerations for doc */
STATIC
soc_error_t soc_qax_pp_oam_mep_db_ptr_set(int unit,
                    uint32 mep_idx,
                    soc_field_t field,
                    uint32 first_entry) {

    soc_error_t res;
    soc_reg_above_64_val_t reg_data;
    uint32 mep_type;

    SOCDNX_INIT_FUNC_DEFS;

    SOC_REG_ABOVE_64_CLEAR(reg_data);

    res = _ARAD_PP_OAM_INTERNAL_READ_OAMP_MEP_DBm(unit, MEM_BLOCK_ANY, mep_idx, reg_data);
    SOCDNX_IF_ERR_EXIT(res);

    mep_type = soc_mem_field32_get(unit, OAMP_MEP_DBm, reg_data, MEP_TYPEf); 
    
    switch (mep_type) {
    case SOC_PPC_OAM_MEP_TYPE_ETH_OAM:
        soc_OAMP_MEP_DBm_field32_set(unit, reg_data, field, first_entry);
        break;
    case SOC_PPC_OAM_MEP_TYPE_Y1731_O_MPLSTP:
    case SOC_PPC_OAM_MEP_TYPE_Y1731_O_PWE_GAL:
        soc_OAMP_MEP_DB_Y_1731_ON_MPLSTPm_field32_set(unit, reg_data, field, first_entry);
        break;
    case SOC_PPC_OAM_MEP_TYPE_Y1731_O_PWE:
        soc_OAMP_MEP_DB_Y_1731_ON_PWEm_field32_set(unit, reg_data, field, first_entry);
        break;
    case SOC_PPC_OAM_MEP_TYPE_BFD_O_IPV4_1_HOP:
    case SOC_PPC_OAM_MEP_TYPE_BFD_O_IPV4_M_HOP:
    case SOC_PPC_OAM_MEP_TYPE_BFD_O_MPLS:
    case SOC_PPC_OAM_MEP_TYPE_BFD_O_PWE:
        if (field == EXTRA_DATA_PTRf) {
            switch (mep_type) {
            case SOC_PPC_OAM_MEP_TYPE_BFD_O_IPV4_1_HOP:
                soc_OAMP_MEP_DB_BFD_ON_IPV4_ONE_HOPm_field32_set(unit, reg_data, field, first_entry);
                break;
            case SOC_PPC_OAM_MEP_TYPE_BFD_O_IPV4_M_HOP:
                soc_OAMP_MEP_DB_BFD_ON_IPV4_MULTI_HOPm_field32_set(unit, reg_data, field, first_entry);
                break;
            case SOC_PPC_OAM_MEP_TYPE_BFD_O_MPLS:
                soc_OAMP_MEP_DB_BFD_ON_MPLSm_field32_set(unit, reg_data, field, first_entry);
                break;
            case SOC_PPC_OAM_MEP_TYPE_BFD_O_PWE:
                soc_OAMP_MEP_DB_BFD_ON_PWEm_field32_set(unit, reg_data, field, first_entry);
                break;
            }
            break;
        }
        /* If not, just fall to the default case.*/
    default:
        SOCDNX_EXIT_WITH_ERR(SOC_E_UNAVAIL,
                             (_BSL_SOCDNX_MSG("Error - next entry pointer not supported for the MEP type used.")));
    }
    res = _ARAD_PP_OAM_INTERNAL_WRITE_OAMP_MEP_DBm(unit, MEM_BLOCK_ANY, mep_idx, reg_data);
    SOCDNX_IF_ERR_EXIT(res);

exit:
    SOCDNX_FUNC_RETURN;
}

/* See static declerations for doc */
STATIC
soc_error_t soc_qax_pp_oam_mep_db_ptr_get(int unit,
                    uint32 mep_idx,
                    soc_field_t field,
                    int *first_entry) {

    soc_error_t res;
    soc_reg_above_64_val_t reg_data;
    uint32 mep_type;

    SOCDNX_INIT_FUNC_DEFS;

    SOC_REG_ABOVE_64_CLEAR(reg_data);

    res = _ARAD_PP_OAM_INTERNAL_READ_OAMP_MEP_DBm(unit, MEM_BLOCK_ANY, mep_idx, reg_data);
    SOCDNX_IF_ERR_EXIT(res);

    mep_type = soc_mem_field32_get(unit, OAMP_MEP_DBm, reg_data, MEP_TYPEf); 
    
    switch (mep_type) {
    case SOC_PPC_OAM_MEP_TYPE_ETH_OAM:
        *first_entry = soc_OAMP_MEP_DBm_field32_get(unit, reg_data, field);
        break;
    case SOC_PPC_OAM_MEP_TYPE_Y1731_O_MPLSTP:
    case SOC_PPC_OAM_MEP_TYPE_Y1731_O_PWE_GAL:
        *first_entry = soc_OAMP_MEP_DB_Y_1731_ON_MPLSTPm_field32_get(unit, reg_data, field);
        break;
    case SOC_PPC_OAM_MEP_TYPE_Y1731_O_PWE:
        *first_entry = soc_OAMP_MEP_DB_Y_1731_ON_PWEm_field32_get(unit, reg_data, field);
        break;
    case SOC_PPC_OAM_MEP_TYPE_BFD_O_IPV4_1_HOP:
    case SOC_PPC_OAM_MEP_TYPE_BFD_O_IPV4_M_HOP:
    case SOC_PPC_OAM_MEP_TYPE_BFD_O_MPLS:
    case SOC_PPC_OAM_MEP_TYPE_BFD_O_PWE:
        if (field == EXTRA_DATA_PTRf) {
            switch (mep_type) {
            case SOC_PPC_OAM_MEP_TYPE_BFD_O_IPV4_1_HOP:
                *first_entry = soc_OAMP_MEP_DB_BFD_ON_IPV4_ONE_HOPm_field32_get(unit, reg_data, field);
                break;
            case SOC_PPC_OAM_MEP_TYPE_BFD_O_IPV4_M_HOP:
                *first_entry = soc_OAMP_MEP_DB_BFD_ON_IPV4_MULTI_HOPm_field32_get(unit, reg_data, field);
                break;
            case SOC_PPC_OAM_MEP_TYPE_BFD_O_MPLS:
                *first_entry = soc_OAMP_MEP_DB_BFD_ON_MPLSm_field32_get(unit, reg_data, field);
                break;
            case SOC_PPC_OAM_MEP_TYPE_BFD_O_PWE:
                *first_entry = soc_OAMP_MEP_DB_BFD_ON_PWEm_field32_get(unit, reg_data, field);
                break;
            }
            break;
        }
        /* If not, just fall to the default case.*/
    default:
        SOCDNX_EXIT_WITH_ERR(SOC_E_UNAVAIL,
                             (_BSL_SOCDNX_MSG("Error - next entry pointer not supported for the MEP type used.")));
    }

exit:
    SOCDNX_FUNC_RETURN;
}

/* See static declerations for doc */
STATIC
soc_error_t soc_qax_pp_oam_mep_db_lm_dm_last_bit_write(int unit,
                                                       ARAD_PP_OAM_OAMP_MEP_DB_LM_DM_ENTRIES_INFO *lm_dm_info, /* To resolve the entry type */
                                                       uint32 entry, uint8 val) {

    soc_error_t res;
    soc_reg_above_64_val_t reg_data;

    SOCDNX_INIT_FUNC_DEFS;

    SOC_REG_ABOVE_64_CLEAR(reg_data);

    res = _ARAD_PP_OAM_INTERNAL_READ_OAMP_MEP_DBm(unit, MEM_BLOCK_ANY, entry, reg_data);
    SOCDNX_IF_ERR_EXIT(res);
    if (lm_dm_info->lm_entry == entry) {
        soc_OAMP_MEP_DB_LM_DBm_field32_set(unit, reg_data, LAST_ENTRYf, val);
    } else if (lm_dm_info->lm_stat_entry == entry) {
        soc_OAMP_MEP_DB_LM_STATm_field32_set(unit, reg_data, LAST_ENTRYf, val);
    } else if (lm_dm_info->dm_entry == entry) {
        soc_OAMP_MEP_DB_DM_STAT_TWO_WAYm_field32_set(unit, reg_data, LAST_ENTRYf, val);
    } else {
        
        SOCDNX_EXIT_WITH_ERR(SOC_E_INTERNAL,
                             (_BSL_SOCDNX_MSG("Something went wrong.")));

    }
    res = _ARAD_PP_OAM_INTERNAL_WRITE_OAMP_MEP_DBm(unit, MEM_BLOCK_ANY, entry, reg_data);
    SOCDNX_IF_ERR_EXIT(res);

exit:
    SOCDNX_FUNC_RETURN;
}


/* See static declerations for doc */
STATIC
soc_error_t soc_qax_pp_oam_oamp_lm_dm_shared_scan(int unit, int endpoint_id,
                                                  ARAD_PP_OAM_OAMP_MEP_DB_LM_DM_ENTRIES_INFO *lm_dm_info) {
    soc_error_t res = SOC_E_NONE;

    SOCDNX_INIT_FUNC_DEFS;

    lm_dm_info->endpoint_id = endpoint_id;

    /* Get the MEP entry */
    res = arad_pp_oam_oamp_mep_db_entry_get_unsafe(unit, endpoint_id, &lm_dm_info->mep_entry);
    SOCDNX_SAND_IF_ERR_EXIT(res);

    /* Scan for LM/DM chain */
    res = soc_qax_pp_oam_oamp_lm_dm_mep_scan(unit, lm_dm_info);
    SOCDNX_IF_ERR_EXIT(res);

exit:
    SOCDNX_FUNC_RETURN;
}


/* See static declerations for doc */
STATIC
void _soc_qax_pp_oam_dm_two_way_entry_read(int unit, SOC_PPC_OAM_OAMP_DM_INFO_GET *dm_info,
                                           soc_reg_above_64_val_t reg_data) {

    soc_reg_above_64_val_t reg_field;

    SOC_REG_ABOVE_64_CLEAR(reg_field);
    soc_OAMP_MEP_DB_DM_STAT_TWO_WAYm_field_get(unit, reg_data, LAST_DELAYf, reg_field);
    SHR_BITCOPY_RANGE(&(dm_info->last_delay_sub_seconds), 0, reg_field, 0, 30);
    SHR_BITCOPY_RANGE(&(dm_info->last_delay_second), 0, reg_field, 30, 12);

    SOC_REG_ABOVE_64_CLEAR(reg_field);
    soc_OAMP_MEP_DB_DM_STAT_TWO_WAYm_field_get(unit, reg_data, MAX_DELAYf, reg_field);
    SHR_BITCOPY_RANGE(&(dm_info->max_delay_sub_seconds), 0, reg_field, 0, 30);
    SHR_BITCOPY_RANGE(&(dm_info->max_delay_second), 0, reg_field, 30, 12);

    SOC_REG_ABOVE_64_CLEAR(reg_field);
    soc_OAMP_MEP_DB_DM_STAT_TWO_WAYm_field_get(unit, reg_data, MIN_DELAYf, reg_field);
    SHR_BITCOPY_RANGE(&(dm_info->min_delay_sub_seconds), 0, reg_field, 0, 30);
    SHR_BITCOPY_RANGE(&(dm_info->min_delay_second), 0, reg_field, 30, 12);

    SOC_REG_ABOVE_64_CLEAR(reg_field);
    soc_OAMP_MEP_DB_DM_STAT_TWO_WAYm_field_set(unit, reg_data, MAX_DELAYf, reg_field);
    SHR_BITSET_RANGE(reg_field, 0, soc_mem_field_length(unit, OAMP_MEP_DB_DM_STAT_TWO_WAYm, MIN_DELAYf));
    soc_OAMP_MEP_DB_DM_STAT_TWO_WAYm_field_set(unit, reg_data, MIN_DELAYf, reg_field);

}

/* See static declerations for doc */
STATIC
void _soc_qax_pp_oam_lm_entry_read(int unit, SOC_PPC_OAM_OAMP_LM_INFO_GET *lm_info,
                                   soc_reg_above_64_val_t reg_data) {
    lm_info->my_tx = soc_OAMP_MEP_DB_LM_DBm_field32_get(unit, reg_data, MY_TXf);
    lm_info->my_rx = soc_OAMP_MEP_DB_LM_DBm_field32_get(unit, reg_data, MY_RXf);
    lm_info->peer_tx = soc_OAMP_MEP_DB_LM_DBm_field32_get(unit, reg_data, PEER_TXf);
    lm_info->peer_rx = soc_OAMP_MEP_DB_LM_DBm_field32_get(unit, reg_data, PEER_RXf);
}

/* See static declerations for doc */
STATIC
void _soc_qax_pp_oam_lm_stat_entry_read(int unit, SOC_PPC_OAM_OAMP_LM_INFO_GET *lm_info,
                                        soc_reg_above_64_val_t reg_data) {
    lm_info->last_lm_near = soc_OAMP_MEP_DB_LM_STATm_field32_get(unit, reg_data, LAST_LM_NEARf);
    lm_info->last_lm_far = soc_OAMP_MEP_DB_LM_STATm_field32_get(unit, reg_data, LAST_LM_FARf);
    lm_info->acc_lm_near = soc_OAMP_MEP_DB_LM_STATm_field32_get(unit, reg_data, ACC_LM_NEARf);
    lm_info->acc_lm_far = soc_OAMP_MEP_DB_LM_STATm_field32_get(unit, reg_data, ACC_LM_FARf);
    lm_info->max_lm_near = soc_OAMP_MEP_DB_LM_STATm_field32_get(unit, reg_data, MAX_LM_NEARf);
    lm_info->max_lm_far = soc_OAMP_MEP_DB_LM_STATm_field32_get(unit, reg_data, MAX_LM_FARf);

    /* acc/max fields are to be reset when read */
    soc_OAMP_MEP_DB_LM_STATm_field32_set(unit, reg_data, MAX_LM_NEARf, 0);
    soc_OAMP_MEP_DB_LM_STATm_field32_set(unit, reg_data, MAX_LM_FARf, 0);
    soc_OAMP_MEP_DB_LM_STATm_field32_set(unit, reg_data, ACC_LM_NEARf, 0);
    soc_OAMP_MEP_DB_LM_STATm_field32_set(unit, reg_data, ACC_LM_FARf, 0);
}

/* Build array of uint32 that contains the 48 byte MAID
 * So that it is read in the correct order by the
 * hardware
 */
STATIC
void _reverse_buffer_copy(SHR_BITDCL *dst_ptr,
                          CONST int dst_first,
                          CONST SHR_BITDCL *src_ptr,
                          int src_first,
                          int range) {
    int i, bits_from_left, bits_from_right, bits_to_copy, src_offset, buf_elem_size, initial_offset, first_copy;
    buf_elem_size = SHR_BITWID;
    initial_offset = src_first % buf_elem_size;

    /* First 32 byte dword is special, if it is incomplete - bits should be written from the right, and not the left! */
    if (initial_offset != 0) {
        first_copy = buf_elem_size - initial_offset;
        SHR_BITCOPY_RANGE(dst_ptr, dst_first + range - first_copy, src_ptr, src_first - initial_offset, first_copy);
        src_first += first_copy;
        range -= first_copy;
    }

    for (i = 0; i < ((range + buf_elem_size - 1) / buf_elem_size) ; i++) {
        bits_from_left = (i + 1) * buf_elem_size;
        bits_from_right = range >= bits_from_left ? range - bits_from_left : 0;
        bits_to_copy = range >= bits_from_left ? buf_elem_size : range % buf_elem_size;
        src_offset = range >= bits_from_left ? i * buf_elem_size : buf_elem_size * (i + 1) - bits_to_copy;
        SHR_BITCOPY_RANGE(dst_ptr, dst_first + bits_from_right, src_ptr, src_first + src_offset, bits_to_copy);
    }
}

soc_error_t
qax_pp_oam_bfd_flexible_verification_init(int unit){
    soc_error_t res;

    SOCDNX_INIT_FUNC_DEFS;

    if(!SOC_WARM_BOOT(unit)) {
        res = sw_state_access[unit].dpp.soc.qax.pp.mep_db.flexible_verification_use_indicator.alloc_bitmap(unit, ARAD_PP_OAM_OAMP_MEP_DB_MEP_ENTRIES_NOF(unit));
        SOCDNX_IF_ERR_EXIT(res);
    }

exit:
    SOCDNX_FUNC_RETURN;
}

soc_error_t
qax_pp_oam_bfd_flexible_verification_set(int unit, SOC_PPC_OAM_BFD_FLEXIBLE_VERIFICATION_INFO *info) {
    soc_error_t res;

    soc_reg_above_64_val_t reg_data, fld_above64_val, soft_init_reg_val;
    uint32 crc_msb_mask[SOC_PPC_OAM_OAMP_CRC_MASK_MSB_BYTE_SIZE / 4], flex_crc_tcam_key, flex_crc_tcam_data, flex_crc_tcam_mask;
    int byte;

    SOCDNX_INIT_FUNC_DEFS;

    SOCDNX_NULL_CHECK(info);

    /* 
     *  1. Configure the crc value in the MEP entry.
     */
    if (info->mep_idx != -1) {
        res = SOC_QAX_PP_OAM_INTERNAL_READ_OAMP_MEP_DB(unit, MEM_BLOCK_ALL, info->mep_idx, reg_data);
        SOCDNX_IF_ERR_EXIT(res);

        soc_mem_field32_set(unit, OAMP_MEP_DB_EXT_DATA_HDRm, reg_data, MEP_TYPEf, SOC_PPC_OAM_MEP_TYPE_EXT_DATA_HDR);
        soc_OAMP_MEP_DB_EXT_DATA_HDRm_field32_set(unit, reg_data, CHECK_CRC_VALUE_1f, info->crc_info.crc16_val1);
        soc_OAMP_MEP_DB_EXT_DATA_HDRm_field32_set(unit, reg_data, CHECK_CRC_VALUE_2f, info->crc_info.crc16_val2);

        res = SOC_QAX_PP_OAM_INTERNAL_WRITE_OAMP_MEP_DB(unit, MEM_BLOCK_ALL, info->mep_idx, reg_data);
        SOCDNX_IF_ERR_EXIT(res);

        /* Set indication that this entry is in use for verification in the SW DB. */
        res = sw_state_access[unit].dpp.soc.qax.pp.mep_db.flexible_verification_use_indicator.bit_set(unit, info->mep_idx);
        SOCDNX_IF_ERR_EXIT(res);
    }

    /*
     * 2. Configure the crc mask table.
     */
    if (info->mask_tbl_index != -1) {
        sal_memset(reg_data, 0, sizeof(reg_data));
        sal_memset(crc_msb_mask, 0, sizeof(crc_msb_mask));

        /* Convert the mask from uint8 array to a uint32 array (to avoid big/little endian bugs). */
        for (byte = 0; byte < SOC_PPC_OAM_OAMP_CRC_MASK_MSB_BYTE_SIZE; byte++) {
            crc_msb_mask[byte / 4] += info->crc_info.mask.msb_mask[byte] << ((byte % 4) * 8);
        }

        /* Write the fields. */
        soc_mem_field_set(unit, OAMP_FLEX_VER_MASK_TEMPm, reg_data, PER_BIT_MASKf, crc_msb_mask);
        soc_mem_field_set(unit, OAMP_FLEX_VER_MASK_TEMPm, reg_data, PER_BYTE_MASKf, info->crc_info.mask.lsbyte_mask);

        /* Hardware bug workaround - this block must be in soft reset when written to */
        SOCDNX_IF_ERR_EXIT(READ_ECI_BLOCKS_SOFT_INITr(unit,soft_init_reg_val));
        SOC_REG_ABOVE_64_CLEAR(fld_above64_val);
        SOC_REG_ABOVE_64_CREATE_MASK(fld_above64_val, 1, 0);
        soc_reg_above_64_field_set(unit, ECI_BLOCKS_SOFT_INITr, soft_init_reg_val, BLOCKS_SOFT_INIT_56f, fld_above64_val);
        SOCDNX_IF_ERR_EXIT(WRITE_ECI_BLOCKS_SOFT_INITr(unit, soft_init_reg_val));

        /* Write to HW. */
        res = soc_mem_write(unit, OAMP_FLEX_VER_MASK_TEMPm, MEM_BLOCK_ALL, info->mask_tbl_index, reg_data);

        /* Exit soft reset state */
        SOCDNX_IF_ERR_EXIT(READ_ECI_BLOCKS_SOFT_INITr(unit,soft_init_reg_val));
        SOC_REG_ABOVE_64_CLEAR(fld_above64_val);
        soc_reg_above_64_field_set(unit, ECI_BLOCKS_SOFT_INITr, soft_init_reg_val, BLOCKS_SOFT_INIT_56f, fld_above64_val);
        SOCDNX_IF_ERR_EXIT(WRITE_ECI_BLOCKS_SOFT_INITr(unit, soft_init_reg_val));

        SOCDNX_IF_ERR_EXIT(res);
    }

    /* 
     * 3. Configure the crc select tcam. 
     */
    if (info->crc_tcam_index != -1) {
        sal_memset(reg_data, 0, sizeof(reg_data));

        /* Fill the key. */
        flex_crc_tcam_key   = 0;

        QAX_PP_OAMP_CLS_FLEX_CRC_TCAM_OPCODE_SET(flex_crc_tcam_key, info->crc_tcam_info.opcode_bmp);

        QAX_PP_OAMP_CLS_FLEX_CRC_TCAM_OAM_BFD_SET(flex_crc_tcam_key, info->crc_tcam_info.oam_bfd);

        QAX_PP_OAMP_CLS_FLEX_CRC_TCAM_PE_PROFILE_SET(flex_crc_tcam_key, info->crc_tcam_info.mep_pe_profile);

        soc_mem_field32_set(unit, OAMP_CLS_FLEX_CRC_TCAMm, reg_data, KEYf, flex_crc_tcam_key);

        /* Fill the mask. */
        flex_crc_tcam_mask   = 0;

        QAX_PP_OAMP_CLS_FLEX_CRC_TCAM_OPCODE_SET(flex_crc_tcam_mask, info->crc_tcam_info.opcode_bmp_mask);

        QAX_PP_OAMP_CLS_FLEX_CRC_TCAM_OAM_BFD_SET(flex_crc_tcam_mask, info->crc_tcam_info.oam_bfd_mask);

        QAX_PP_OAMP_CLS_FLEX_CRC_TCAM_PE_PROFILE_SET(flex_crc_tcam_mask, info->crc_tcam_info.mep_pe_profile_mask);

        soc_mem_field32_set(unit, OAMP_CLS_FLEX_CRC_TCAMm, reg_data, MASKf, flex_crc_tcam_mask);

        /* Fill the data. */
        flex_crc_tcam_data  = 0;

        QAX_PP_OAMP_CLS_FLEX_CRC_TCAM_MASK_SEL_SET(flex_crc_tcam_data, info->crc_tcam_info.mask_tbl_index);

        QAX_PP_OAMP_CLS_FLEX_CRC_TCAM_CRC_SEL_SET(flex_crc_tcam_data, info->crc_tcam_info.crc_select);

        soc_mem_field32_set(unit, OAMP_CLS_FLEX_CRC_TCAMm, reg_data, DATf, flex_crc_tcam_data);

        soc_mem_field32_set(unit, OAMP_CLS_FLEX_CRC_TCAMm, reg_data, VALIDf, 1);

        res = soc_mem_write(unit, OAMP_CLS_FLEX_CRC_TCAMm, MEM_BLOCK_ALL, info->crc_tcam_index, reg_data);
        SOCDNX_IF_ERR_EXIT(res);
    }



exit:
    SOCDNX_FUNC_RETURN;
}


soc_error_t
qax_pp_oam_bfd_flexible_verification_get(int unit, SOC_PPC_OAM_BFD_FLEXIBLE_VERIFICATION_INFO *info) {
    soc_error_t res;

    soc_reg_above_64_val_t reg_data;
    uint32 crc_msb_mask[SOC_PPC_OAM_OAMP_CRC_MASK_MSB_BYTE_SIZE / 4], flex_crc_tcam_key, flex_crc_tcam_data;
    int byte;
    uint8 crc_is_in_use;
    uint32 entry_type;
    SOCDNX_INIT_FUNC_DEFS;

    SOCDNX_NULL_CHECK(info);

    /* 
     *  1. Read the crc value in the MEP entry.
     */

    res = SOC_QAX_PP_OAM_INTERNAL_READ_OAMP_MEP_DB(unit, MEM_BLOCK_ALL, info->mep_idx, reg_data);
    SOCDNX_IF_ERR_EXIT(res);

    entry_type = soc_OAMP_MEP_DB_EXT_DATA_HDRm_field32_get(unit, reg_data, MEP_TYPEf);

    /* Get indication if this entry is in use for verification from the SW DB. */
    res = sw_state_access[unit].dpp.soc.qax.pp.mep_db.flexible_verification_use_indicator.bit_get(unit, info->mep_idx, &crc_is_in_use);
    SOCDNX_IF_ERR_EXIT(res);

    if ((entry_type != SOC_PPC_OAM_MEP_TYPE_EXT_DATA_HDR) || !crc_is_in_use) {
        SOCDNX_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_SOC_MSG("CRC is not used in this entry.")));
    }

    info->crc_info.crc16_val1 = soc_OAMP_MEP_DB_EXT_DATA_HDRm_field32_get(unit, reg_data, CHECK_CRC_VALUE_1f);
    info->crc_info.crc16_val2 = soc_OAMP_MEP_DB_EXT_DATA_HDRm_field32_get(unit, reg_data, CHECK_CRC_VALUE_2f);
    
    /* 
     * 2. Read the crc select tcam. 
     */

    res = soc_mem_read(unit, OAMP_CLS_FLEX_CRC_TCAMm, MEM_BLOCK_ALL, info->crc_tcam_index, reg_data);
    SOCDNX_IF_ERR_EXIT(res);

    if (soc_mem_field32_get(unit, OAMP_CLS_FLEX_CRC_TCAMm, reg_data, VALIDf)) {

        /* Get the data. */
        flex_crc_tcam_data = soc_mem_field32_get(unit, OAMP_CLS_FLEX_CRC_TCAMm, reg_data, DATf);

        info->crc_tcam_info.mask_tbl_index = QAX_PP_OAMP_CLS_FLEX_CRC_TCAM_MASK_SEL_GET(flex_crc_tcam_data);

        info->crc_tcam_info.crc_select = QAX_PP_OAMP_CLS_FLEX_CRC_TCAM_CRC_SEL_GET(flex_crc_tcam_data);

        if (info->mask_tbl_index == -1) {
            /* Get the mask index, if it wasn't given already. */
            info->mask_tbl_index = info->crc_tcam_info.mask_tbl_index;
        }

        /* Get the key. */
        flex_crc_tcam_key = soc_mem_field32_get(unit, OAMP_CLS_FLEX_CRC_TCAMm, reg_data, KEYf);

        info->crc_tcam_info.opcode_bmp = QAX_PP_OAMP_CLS_FLEX_CRC_TCAM_OPCODE_GET(flex_crc_tcam_key);

        info->crc_tcam_info.oam_bfd = QAX_PP_OAMP_CLS_FLEX_CRC_TCAM_OAM_BFD_GET(flex_crc_tcam_key);

        info->crc_tcam_info.mep_pe_profile = QAX_PP_OAMP_CLS_FLEX_CRC_TCAM_PE_PROFILE_GET(flex_crc_tcam_key);
    } else {
        info->crc_tcam_info.mask_tbl_index = -1;

        info->crc_tcam_info.crc_select = -1;

        info->crc_tcam_info.opcode_bmp = -1;

        info->crc_tcam_info.oam_bfd = -1;

        info->crc_tcam_info.mep_pe_profile = -1;
    }


    /*
     * 3. Read the crc mask table.
     */

    /* Read from HW. */
    res = soc_mem_read(unit, OAMP_FLEX_VER_MASK_TEMPm, MEM_BLOCK_ALL, info->mask_tbl_index, reg_data);
    SOCDNX_IF_ERR_EXIT(res);

    /* Write the fields. */
    soc_mem_field_get(unit, OAMP_FLEX_VER_MASK_TEMPm, reg_data, PER_BIT_MASKf, crc_msb_mask);
    soc_mem_field_get(unit, OAMP_FLEX_VER_MASK_TEMPm, reg_data, PER_BYTE_MASKf, info->crc_info.mask.lsbyte_mask);

    /* Convert the mask from uint32 array to a uint8 array (to avoid big/little endian bugs). */
    for (byte = 0; byte < SOC_PPC_OAM_OAMP_CRC_MASK_MSB_BYTE_SIZE; byte++) {
        info->crc_info.mask.msb_mask[byte] = (crc_msb_mask[byte / 4] >> (byte % 4)) & 0xff;
    }


exit:
    SOCDNX_FUNC_RETURN;
}


soc_error_t
qax_pp_oam_bfd_flexible_verification_delete(int unit, SOC_PPC_OAM_BFD_FLEXIBLE_VERIFICATION_INFO *info) {
    soc_error_t res;

    soc_reg_above_64_val_t reg_data;
    uint32 entry_type;
    uint8 crc_is_in_use;

    SOCDNX_INIT_FUNC_DEFS;

    SOCDNX_NULL_CHECK(info);

    /* 
     *  1. Delete the crc value from the MEP entry.
     */
    if (info->mep_idx != -1) {
        res = SOC_QAX_PP_OAM_INTERNAL_READ_OAMP_MEP_DB(unit, MEM_BLOCK_ALL, info->mep_idx, reg_data);
        SOCDNX_IF_ERR_EXIT(res);

        entry_type = soc_OAMP_MEP_DB_EXT_DATA_HDRm_field32_get(unit, reg_data, MEP_TYPEf);

        /* Get indication if this entry is in use for verification from the SW DB. */
        res = sw_state_access[unit].dpp.soc.qax.pp.mep_db.flexible_verification_use_indicator.bit_get(unit, info->mep_idx, &crc_is_in_use);
        SOCDNX_IF_ERR_EXIT(res);

        if ((entry_type != SOC_PPC_OAM_MEP_TYPE_EXT_DATA_HDR) || (!crc_is_in_use)) {
            SOCDNX_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_SOC_MSG("CRC is not used in this entry.")));
        }

        if (!soc_mem_field32_get(unit, OAMP_MEP_DB_EXT_DATA_HDRm, reg_data, EXT_DATA_LENGTHf)){
            /* If the extra data length is 0, it means this entry is not used for packet generation. We can just clear it. */
            sal_memset(reg_data, 0, sizeof(reg_data));
        } else {
            /* If the extra data length is >0, this entry is used for packet generation. Only clear the crc. */
            soc_OAMP_MEP_DB_EXT_DATA_HDRm_field32_set(unit, reg_data, CHECK_CRC_VALUE_1f, 0);
            soc_OAMP_MEP_DB_EXT_DATA_HDRm_field32_set(unit, reg_data, CHECK_CRC_VALUE_2f, 0);
        }

        res = SOC_QAX_PP_OAM_INTERNAL_WRITE_OAMP_MEP_DB(unit, MEM_BLOCK_ALL, info->mep_idx, reg_data);
        SOCDNX_IF_ERR_EXIT(res);

        /* Clear indication that this entry is in use for verification from the SW DB. */
        res = sw_state_access[unit].dpp.soc.qax.pp.mep_db.flexible_verification_use_indicator.bit_clear(unit, info->mep_idx);
        SOCDNX_IF_ERR_EXIT(res);
    }

    sal_memset(reg_data, 0, sizeof(reg_data));

    /*
     * 2. Clear the crc mask table entry.
     */
    if (info->mask_tbl_index != -1) {
        res = soc_mem_write(unit, OAMP_FLEX_VER_MASK_TEMPm, MEM_BLOCK_ALL, info->mask_tbl_index, reg_data);
        SOCDNX_IF_ERR_EXIT(res);
    }

    /* 
     * 3. Clear the crc select tcam. 
     */
    if (info->crc_tcam_index != -1) {
        res = soc_mem_write(unit, OAMP_CLS_FLEX_CRC_TCAMm, MEM_BLOCK_ALL, info->crc_tcam_index, reg_data);
        SOCDNX_IF_ERR_EXIT(res);
    }



exit:
    SOCDNX_FUNC_RETURN;
}


soc_error_t
qax_pp_oam_bfd_mep_db_ext_data_set(int unit, SOC_PPC_OAM_BFD_MEP_DB_EXT_DATA_INFO *info) {
    soc_error_t res;

    soc_reg_above_64_val_t reg_data;
    uint32      buf[SOC_PPC_OAM_BFD_MEP_DB_EXT_DATA_MAX_SIZE_UINT32];
    int bank, entry_idx, remaining_bits, bits_to_write;
    int extra_bits, current_entry, nof_entries;
    SOCDNX_INIT_FUNC_DEFS;

    SOCDNX_NULL_CHECK(info);

    sal_memset(reg_data, 0, sizeof(reg_data));
    sal_memset(buf, 0, sizeof(reg_data));

    /* 
     * Input validation.
     */
    if (info->data_size_in_bits > SOC_PPC_OAM_BFD_MEP_DB_EXT_DATA_MAX_SIZE_BITS) {
        SOCDNX_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_SOC_MSG("Data size is too big.")));
    }
    if ((info->mep_idx != -1) && (info->mep_idx > SOC_PPC_OAM_MAX_NUMBER_OF_LOCAL_MEPS(unit))) {
        SOCDNX_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_SOC_MSG("Mep idx is too high.")));
    }
    if (info->extension_idx > ARAD_PP_OAM_OAMP_MEP_DB_MEP_ENTRIES_NOF(unit)) {
        SOCDNX_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_SOC_MSG("Extestion idx is too high.")));
    }
    if (info->opcode_bmp & ~SOC_PPC_OAM_BFD_MEP_DB_EXT_OPCODE_BITMAP_LEGAL_OPCODE) {
        SOCDNX_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_SOC_MSG("Illegal bit set in opcode bitmap.")));
    }

    if(info->mep_idx != -1) {
        /* Connect MEP entry to the extension */
        res = soc_qax_pp_oam_mep_db_ptr_set(unit, info->mep_idx, EXTRA_DATA_PTRf, info->extension_idx);
        SOCDNX_IF_ERR_EXIT(res);
    }

    /*
     *  If user requested, configure the data entry.
     */

    if (info->data_size_in_bits > 0) {
        /* Find out how many entries we need. */
        nof_entries = 1; /* For the header. */
        extra_bits = info->data_size_in_bits - SOC_PPC_OAM_BFD_MEP_DB_EXT_DATA_FIRST_HDR_SIZE_BITS;

        if (extra_bits > 0) {
            nof_entries += extra_bits / SOC_PPC_OAM_BFD_MEP_DB_EXT_DATA_EXTRA_ENTRY_SIZE_BITS;
            nof_entries += (extra_bits % SOC_PPC_OAM_BFD_MEP_DB_EXT_DATA_EXTRA_ENTRY_SIZE_BITS) ? 1 : 0; /* Account for partial entries. */
        }

        bank = OAMP_MEP_DB_ENTRY_ID_TO_BLOCK(info->extension_idx);
        entry_idx = OAMP_MEP_DB_ENTRY_ID_TO_INDEX(info->extension_idx);

        /* Validate that the last bank would be legal. */
        if ((bank + nof_entries) > OAMP_MEP_DB_ENTRY_ID_TO_BLOCK(ARAD_PP_OAM_OAMP_MEP_DB_MEP_ENTRIES_NOF(unit))) {
            SOCDNX_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_SOC_MSG("Data is too large to fit in legal entries.")));
        }
        /*
         *  Configure the header entry.
         */
        res = soc_mem_array_read(unit, OAMP_MEP_DB_EXT_DATA_HDRm, bank, MEM_BLOCK_ALL, entry_idx, reg_data);
        SOCDNX_IF_ERR_EXIT(res);

        soc_mem_field32_set(unit, OAMP_MEP_DB_EXT_DATA_HDRm, reg_data, MEP_TYPEf, SOC_PPC_OAM_MEP_TYPE_EXT_DATA_HDR);
        soc_mem_field32_set(unit, OAMP_MEP_DB_EXT_DATA_HDRm, reg_data, OPCODES_TO_PREPENDf, info->opcode_bmp);
        soc_mem_field32_set(unit, OAMP_MEP_DB_EXT_DATA_HDRm, reg_data, EXT_DATA_LENGTHf, nof_entries);
        _reverse_buffer_copy(buf, 0, info->data, 0, SOC_PPC_OAM_BFD_MEP_DB_EXT_DATA_FIRST_HDR_SIZE_BITS);
        soc_mem_field_set(unit, OAMP_MEP_DB_EXT_DATA_HDRm, reg_data, VALUEf, buf);

        res = soc_mem_array_write(unit, OAMP_MEP_DB_EXT_DATA_HDRm, bank, MEM_BLOCK_ALL, entry_idx, reg_data);
        SOCDNX_IF_ERR_EXIT(res);

        /*
         * Configure the extra entries.
         */
        for (remaining_bits = extra_bits, current_entry = 1 ;
                remaining_bits > 0 ;
                    remaining_bits -= SOC_PPC_OAM_BFD_MEP_DB_EXT_DATA_EXTRA_ENTRY_SIZE_BITS, current_entry++) {
            sal_memset(reg_data, 0, sizeof(reg_data));
            sal_memset(buf, 0, sizeof(reg_data));

            /* Build the buffer for the extra entries. */
            bits_to_write = SOC_SAND_MIN(remaining_bits, SOC_PPC_OAM_BFD_MEP_DB_EXT_DATA_EXTRA_ENTRY_SIZE_BITS);
            /* The last entry is also important.  Hardware reads bits in reverse, so bits should be
               aligned to the "MSb" and zero padded from the "LSb" */
            _reverse_buffer_copy(buf, SOC_PPC_OAM_BFD_MEP_DB_EXT_DATA_EXTRA_ENTRY_SIZE_BITS - bits_to_write, info->data,
                                 SOC_PPC_OAM_BFD_MEP_DB_EXT_DATA_FIRST_HDR_SIZE_BITS + (current_entry - 1) * SOC_PPC_OAM_BFD_MEP_DB_EXT_DATA_EXTRA_ENTRY_SIZE_BITS,
                                 bits_to_write);
            soc_mem_field_set(unit, OAMP_MEP_DB_EXT_DATA_PLDm, reg_data, VALUEf, buf);
            soc_mem_field32_set(unit, OAMP_MEP_DB_EXT_DATA_PLDm, reg_data, MEP_TYPEf, SOC_PPC_OAM_MEP_TYPE_EXT_DATA_PLD);

            res = soc_mem_array_write(unit, OAMP_MEP_DB_EXT_DATA_PLDm, bank + current_entry, MEM_BLOCK_ALL, entry_idx, reg_data);
            SOCDNX_IF_ERR_EXIT(res);
        }
    }

exit:
    SOCDNX_FUNC_RETURN;
}

soc_error_t
qax_pp_oam_bfd_mep_db_ext_data_get(int unit, SOC_PPC_OAM_BFD_MEP_DB_EXT_DATA_INFO *info){
    soc_error_t res;

    soc_reg_above_64_val_t reg_data;
    uint32      buf[SOC_PPC_OAM_BFD_MEP_DB_EXT_DATA_MAX_SIZE_UINT32];
    int bank, entry_idx;
    int current_entry, nof_extra_entries;
    uint32 entry_type;
    SOCDNX_INIT_FUNC_DEFS;

    SOCDNX_NULL_CHECK(info);

    sal_memset(reg_data, 0, sizeof(reg_data));
    sal_memset(buf, 0, sizeof(reg_data));
    sal_memset(info->data, 0, sizeof(info->data));

    
    if (!info->extension_idx) {
        /* Extension idx wasn't given, get it from the mep idx.*/ 
        if (info->mep_idx > SOC_PPC_OAM_MAX_NUMBER_OF_LOCAL_MEPS(unit)) {
            SOCDNX_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_SOC_MSG("Mep idx is too high.")));
        }
        res = soc_qax_pp_oam_mep_db_ptr_get(unit, info->mep_idx, EXTRA_DATA_PTRf, &info->extension_idx);
        SOCDNX_IF_ERR_EXIT(res);

        if (!info->extension_idx) {
            SOCDNX_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_SOC_MSG("Mep entry doesn't have an extesion.")));
        }
    } 


    if (info->extension_idx > ARAD_PP_OAM_OAMP_MEP_DB_MEP_ENTRIES_NOF(unit)) {
        SOCDNX_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_SOC_MSG("Extension idx is too high.")));
    }

    bank = OAMP_MEP_DB_ENTRY_ID_TO_BLOCK(info->extension_idx);
    entry_idx = OAMP_MEP_DB_ENTRY_ID_TO_INDEX(info->extension_idx);
    
    /* 
     *  Get the header entry.
     */
    res = soc_mem_array_read(unit, OAMP_MEP_DB_EXT_DATA_HDRm, bank, MEM_BLOCK_ALL, entry_idx, reg_data);
    SOCDNX_IF_ERR_EXIT(res);

    entry_type = soc_mem_field32_get(unit, OAMP_MEP_DB_EXT_DATA_HDRm, reg_data, MEP_TYPEf);

    if (entry_type != SOC_PPC_OAM_MEP_TYPE_EXT_DATA_HDR) {
        SOCDNX_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_SOC_MSG("Extesion entry is not of extesion type.")));
    }

    info->opcode_bmp = soc_mem_field32_get(unit, OAMP_MEP_DB_EXT_DATA_HDRm, reg_data, OPCODES_TO_PREPENDf);
    nof_extra_entries = soc_mem_field32_get(unit, OAMP_MEP_DB_EXT_DATA_HDRm, reg_data, EXT_DATA_LENGTHf) - 1 /*Not counting the header entry */;

    if (info->data_size_in_bits == 0) {
        /* This is not actually accurate, but a good upper limit. */
        info->data_size_in_bits = SOC_PPC_OAM_BFD_MEP_DB_EXT_DATA_FIRST_HDR_SIZE_BITS + nof_extra_entries * SOC_PPC_OAM_BFD_MEP_DB_EXT_DATA_EXTRA_ENTRY_SIZE_BITS;
    }

    soc_mem_field_get(unit, OAMP_MEP_DB_EXT_DATA_HDRm, reg_data, VALUEf, buf);
    SHR_BITCOPY_RANGE(info->data, 0, buf, 0, SOC_PPC_OAM_BFD_MEP_DB_EXT_DATA_FIRST_HDR_SIZE_BITS);

    /*
     * Get the extra entries.
     */
    
    for (current_entry = 0 ; current_entry < nof_extra_entries ; current_entry++) {
        sal_memset(reg_data, 0, sizeof(reg_data));
        sal_memset(buf, 0, sizeof(reg_data));

        res = soc_mem_array_read(unit, OAMP_MEP_DB_EXT_DATA_PLDm, bank + current_entry, MEM_BLOCK_ALL, entry_idx, reg_data);
        SOCDNX_IF_ERR_EXIT(res);

        entry_type = soc_mem_field32_get(unit, OAMP_MEP_DB_EXT_DATA_HDRm, reg_data, MEP_TYPEf);

        if (entry_type != SOC_PPC_OAM_MEP_TYPE_EXT_DATA_PLD) {
            SOCDNX_EXIT_WITH_ERR(SOC_E_INTERNAL, (_BSL_SOC_MSG("Extra extesion entry is not of extesion type.")));
        }

        /* Copy the data to a buffer. */
        soc_mem_field_get(unit, OAMP_MEP_DB_EXT_DATA_PLDm, reg_data, VALUEf, buf);

        SHR_BITCOPY_RANGE(info->data,
                          SOC_PPC_OAM_BFD_MEP_DB_EXT_DATA_FIRST_HDR_SIZE_BITS + current_entry * SOC_PPC_OAM_BFD_MEP_DB_EXT_DATA_EXTRA_ENTRY_SIZE_BITS,
                          buf,
                          0,
                          SOC_PPC_OAM_BFD_MEP_DB_EXT_DATA_EXTRA_ENTRY_SIZE_BITS);
    }

exit:
    SOCDNX_FUNC_RETURN;
}


soc_error_t
qax_pp_oam_bfd_mep_db_ext_data_delete(int unit, int extension_idx){
    soc_error_t res;

    soc_reg_above_64_val_t reg_data, buf;
    int bank, entry_idx;
    int current_entry, nof_extra_entries;
    uint8 crc_in_use;
    uint32 entry_type;
    SOCDNX_INIT_FUNC_DEFS;

    if (extension_idx > ARAD_PP_OAM_OAMP_MEP_DB_MEP_ENTRIES_NOF(unit)) {
        SOCDNX_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_SOC_MSG("Extension idx is too high.")));
    }


    /* Find out how many entries we need. */
    bank = OAMP_MEP_DB_ENTRY_ID_TO_BLOCK(extension_idx);
    entry_idx = OAMP_MEP_DB_ENTRY_ID_TO_INDEX(extension_idx);

    /*
     *  Get number of extra entries from the header entry.
     */
    res = soc_mem_array_read(unit, OAMP_MEP_DB_EXT_DATA_HDRm, bank, MEM_BLOCK_ALL, entry_idx, reg_data);
    SOCDNX_IF_ERR_EXIT(res);

    entry_type = soc_OAMP_MEP_DB_EXT_DATA_HDRm_field32_get(unit, reg_data, MEP_TYPEf);

    if (entry_type != SOC_PPC_OAM_MEP_TYPE_EXT_DATA_HDR) {
        SOCDNX_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_SOC_MSG("Extesion entry is not of extesion type.")));
    }

    nof_extra_entries   = soc_mem_field32_get(unit, OAMP_MEP_DB_EXT_DATA_HDRm, reg_data, EXT_DATA_LENGTHf) - 1 /*Not counting the header entry */;
    
    /* Get indication if this entry is in use for verification from the SW DB. */
    res = sw_state_access[unit].dpp.soc.qax.pp.mep_db.flexible_verification_use_indicator.bit_get(unit, extension_idx, &crc_in_use);
    SOCDNX_IF_ERR_EXIT(res);

    /*
     * Delete the header entry.
    */
    if (!crc_in_use) {
        /* We don't use crc in this entry. Just write an empty entry. */
        sal_memset(reg_data, 0, sizeof(reg_data));
    } else {
        /* We use crc in this entry. Clear only the relevant fields. */
        soc_mem_field32_set(unit, OAMP_MEP_DB_EXT_DATA_HDRm, reg_data, OPCODES_TO_PREPENDf, 0);
        soc_mem_field32_set(unit, OAMP_MEP_DB_EXT_DATA_HDRm, reg_data, EXT_DATA_LENGTHf, 0);
        sal_memset(buf, 0, sizeof(buf));
        soc_mem_field_set(unit, OAMP_MEP_DB_EXT_DATA_HDRm, reg_data, VALUEf, buf);
    }
    res = soc_mem_array_write(unit, OAMP_MEP_DB_EXT_DATA_HDRm, bank, MEM_BLOCK_ALL, entry_idx, reg_data);
    SOCDNX_IF_ERR_EXIT(res);

    /*
     * Clear the extra entries.
     */
    sal_memset(reg_data, 0, sizeof(reg_data));
    for (current_entry = 1 ; current_entry <= nof_extra_entries ; current_entry++) {
        res = soc_mem_array_write(unit, OAMP_MEP_DB_EXT_DATA_PLDm, bank + current_entry, MEM_BLOCK_ALL, entry_idx, reg_data);
        SOCDNX_IF_ERR_EXIT(res);
    }

exit:
    SOCDNX_FUNC_RETURN;
}
/* } */

/* } */
#include <soc/dpp/SAND/Utils/sand_footer.h>

