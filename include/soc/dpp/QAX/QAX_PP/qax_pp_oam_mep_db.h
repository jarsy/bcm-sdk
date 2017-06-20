/*
 * $Id: $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File: qax_pp_oam_mep_db.h
 */

#ifndef __QAX_PP_OAM_MEP_DB_INCLUDED__
#define __QAX_PP_OAM_MEP_DB_INCLUDED__



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

typedef struct {
    SHR_BITDCL  *flexible_verification_use_indicator;
} qax_pp_oam_mep_db_info_t;

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
 * Find where to add or delete entries to/from the MEP-DB for loss/delay
 * add/update/delete.
 * The pointers in lm_dm_info should already be set to valid populated
 * entry buffers.
 * The action_type must be initialized to one of:
 *   ARAD_PP_OAM_OAMP_MEP_DB_LM_DM_ENTRIES_ACTION_TYPE_ADD_UPDATE
 *   ARAD_PP_OAM_OAMP_MEP_DB_LM_DM_ENTRIES_ACTION_TYPE_REMOVE
 *
 * Returns the scanning result (lm/dm/lm_stat/last entries) and what
 * entries need allocation or freeing.
 */
soc_error_t
soc_qax_pp_oam_oamp_lm_dm_pointed_shared_find(
   int unit,
   ARAD_PP_OAM_OAMP_MEP_DB_LM_DM_ENTRIES_INFO *lm_dm_info
   );

/*
 * Add/Update LM/DM entries in the MEP-DB.
 * This assumes that soc_qax_pp_oam_oamp_lm_dm_pointed_shared_find was
 * already called and that the required entries were allocated.
 */
soc_error_t
soc_qax_pp_oam_oamp_lm_dm_set(
   int unit,
   ARAD_PP_OAM_OAMP_MEP_DB_LM_DM_ENTRIES_INFO *lm_dm_info
   );

/*
 * Remove LM/DM entries in the MEP-DB. This assumes that
 * soc_qax_pp_oam_oamp_lm_dm_pointed_shared_find was already called.
 * After calling this, the entries that are no longer used must be freed.
 */
soc_error_t
soc_qax_pp_oam_oamp_lm_dm_delete(
   int unit,
   ARAD_PP_OAM_OAMP_MEP_DB_LM_DM_ENTRIES_INFO *lm_dm_info
   );

/*
 * soc_qax_pp_oam_oamp_lm_dm_search -
 * Search for existing LM and DM entries in the MEP-DB.
 *
 * INPUT:
 *   int  unit -
 *   uint32 endpoint_id - index of the endpoint in the OAMP
 *                        MEP DB (CCM entry)
 *   uint32 found_bitmap - For every mep type, SOC_PPC_OAM_MEP_TYPE_XXX,
 *                          (found_bitmap &  SOC_PPC_OAM_MEP_TYPE_XXX)
 *                          iff such an entry is associated with the given mep.
 */
soc_error_t
soc_qax_pp_oam_oamp_lm_dm_search(
   int unit,
   uint32 endpoint_id,
   uint32 *found_bmp
   );


/*
 * Get delay measurement data.
 * If the Delay entry is first in the chain, the dm_info->entry_id will
 * be overwritten with the entry to allow the API to return delay_id to
 * the user.
 */
soc_error_t soc_qax_pp_oam_oamp_dm_get(
   int unit,
   SOC_PPC_OAM_OAMP_DM_INFO_GET *dm_info,
   uint8 *is_1DM
   );

/*
 * Get loss measurement data.
 * If the Loss entry is first in the chain, the lm_info->entry_id will
 * be overwritten with the entry to allow the API to return loss_id to
 * the user.
 */
soc_error_t soc_qax_pp_oam_oamp_lm_get(
    int unit,
    SOC_PPC_OAM_OAMP_LM_INFO_GET *lm_info
    );

/*
 * NAME:
 *   qax_pp_oam_bfd_flexible_verification_init
 * FUNCTION:
 *   Configure a flexible verification in the OAMP.
 *   * Configures the MEP DB with an EXT_DATA_HDR entry.
 *   * Configures the CLS_FLEX_CRC_TCAM with the key and mask for the packet.
 *   * Configures the FLEX_VER_MASK_TEMP with the selected mask.
 * INPUT:
 *  unit    -(IN) unit indetifier
 *
 * RETURNS:
 *   SOC_E_NONE or errors.
 */

soc_error_t
qax_pp_oam_bfd_flexible_verification_init(int unit);

/*
 * NAME:
 *   qax_pp_oam_bfd_flexible_verification_set
 * FUNCTION:
 *   Configure a flexible verification in the OAMP.
 *   * Configures the MEP DB with an EXT_DATA_HDR entry.
 *   * Configures the CLS_FLEX_CRC_TCAM with the key and mask for the packet.
 *   * Configures the FLEX_VER_MASK_TEMP with the selected mask.
 * INPUT:
 *  unit    -(IN) unit indetifier
 *  info    -(IN) Data to be filled in the tables.
 *      mep_idx         -(IN) MEP DB entry index. Can be set to -1 to skip mep db configuration.
 *      mask_tbl_index  -(IN) The index in the mask table where the mask is stored.
 *                              Can be set to -1 to skip mask table configuration.
 *      crc_tcam_index  -(IN) The index in the flex crc tcam. Can be set to -1 to skip tcam configuration.
 *
 * RETURNS:
 *   SOC_E_NONE or errors.
 */
soc_error_t 
qax_pp_oam_bfd_flexible_verification_set(
   int unit, 
   SOC_PPC_OAM_BFD_FLEXIBLE_VERIFICATION_INFO *info
   );


/*
 * NAME:
 *   qax_pp_oam_bfd_flexible_verification_get
 * FUNCTION:
 *   Get flexible verification configuration from the OAMP.
 * INPUT:
 *  unit    -(IN) unit indetifier
 *  info
 *      mep_idx         -(IN) MEP DB entry index.
 *      crc_tcam_index  -(IN) The index in the flex crc tcam.
 *      mask_tbl_index  -(IN or OUT) The index in the mask table where the mask is stored. Can also be obtained from the tcam.
 *      everything else -(OUT)
 *
 * RETURNS:
 *   SOC_E_NONE or errors.
 */
soc_error_t 
qax_pp_oam_bfd_flexible_verification_get(
   int unit, 
   SOC_PPC_OAM_BFD_FLEXIBLE_VERIFICATION_INFO *info
   );

/*
 * NAME:
 *   qax_pp_oam_bfd_flexible_verification_delete
 * FUNCTION:
 *   Clear flexible verification configuration from the OAMP.
 * INPUT:
 *  unit    -(IN) unit indetifier
 *  info
 *      mep_idx         -(IN) MEP DB entry index. Can be set to -1 to avoid clearing.
 *      crc_tcam_index  -(IN) The index in the flex crc tcam. Can be set to -1 to avoid clearing.
 *      mask_tbl_index  -(IN) The index in the mask table where the mask is stored. Can be set to -1 to avoid clearing.
 *      everything else - not in use.
 *
 * RETURNS:
 *   SOC_E_NONE or errors.
 */
soc_error_t
qax_pp_oam_bfd_flexible_verification_delete(int unit, SOC_PPC_OAM_BFD_FLEXIBLE_VERIFICATION_INFO *info);

/*
 * NAME:
 *   qax_pp_oam_bfd_mep_db_ext_data_set
 * FUNCTION:
 *   Configure flexible packet generation over several entries in the MEP-DB.
 *      Data will be written in mep_idx and in consecutive MEP-DB banks with the same index.
 *
 * INPUT:
 *  unit    -(IN) unit indetifier
 *  info
 *      mep_idx         -(IN) MEP DB entry index. (-1 means no MEP is configured.)
 *      extesion_index  -(IN) First extesion entry index.
 *      data            -(IN) Data to be appeneded to the created packet.
 *      data_size_in_bits-(IN)Self explanatory.
 *      opcode_bmp      -(IN) Opcode to be prepended to the packet.
 *
 *
 * RETURNS:
 *   SOC_E_NONE or errors.
 */
soc_error_t
qax_pp_oam_bfd_mep_db_ext_data_set(int unit, SOC_PPC_OAM_BFD_MEP_DB_EXT_DATA_INFO *info);


/*
 * NAME:
 *   qax_pp_oam_bfd_mep_db_ext_data_get
 * FUNCTION:
 *   Get flexible packet generation configuration from several entries in the MEP-DB.
 *      Data will be read from mep_idx and from consecutive MEP-DB banks with the same index.
 * INPUT:
 *  unit    -(IN) unit indetifier
 *  info
 *      mep_idx         -(IN) MEP DB entry index.
 *      extesion_index  -(IN or OUT) Can be filled with the extesion index, or it can be retrieved from mep index.
 *      data            -(OUT) Data to be appeneded to the created packet.
 *      data_size_in_bits-(OUT)Self explanatory.
 *      opcode_bmp      -(OUT) Opcode to be prepended to the packet.
 *
 * RETURNS:
 *   SOC_E_NONE or errors.
 */
soc_error_t
qax_pp_oam_bfd_mep_db_ext_data_get(int unit, SOC_PPC_OAM_BFD_MEP_DB_EXT_DATA_INFO *info);



/*
 * NAME:
 *   qax_pp_oam_bfd_mep_db_ext_data_delete
 * FUNCTION:
 *   Delete flexible packet generation configuration from several entries in the MEP-DB.
 *      Data will be deleted from mep_idx and from consecutive MEP-DB banks with the same index.
 * INPUT:
 *  unit            -(IN) unit indetifier
 *  extesion_index  -(IN) First extesion entry index.
 *
 * RETURNS:
 *   SOC_E_NONE or errors.
 */
soc_error_t
qax_pp_oam_bfd_mep_db_ext_data_delete(int unit, int extension_idx);

/* } */
#endif /* __QAX_PP_OAM_MEP_DB_INCLUDED__ */
