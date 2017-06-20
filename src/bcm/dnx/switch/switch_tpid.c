/*! \file switch_tpid.c
 * $Id$
 *
 * TPID - Tag Protocol Identifier.
 *
 */
/*
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#ifdef BSL_LOG_MODULE
#  error "BSL_LOG_MODULE redefined"
#endif
#define BSL_LOG_MODULE BSL_LS_BCMDNX_SWITCH_TPID

/*
 * Include files.
 * {
 */
#include <soc/dnx/dbal/dbal.h>
#include <bcm/types.h>
#include <bcm/switch.h>
#include <bcm_int/dnx/switch/switch_tpid.h>

/*
 * }
 */

/**
 * \brief
 * Verify TPID's parameters for BCM-API: bcm_dnx_switch_tpid_add(). \n
 */
static shr_error_e
dnx_switch_tpid_add_verify(
  int unit,
  uint32 options,
  bcm_switch_tpid_info_t * tpid_info)
{
  SHR_FUNC_INIT_VARS(unit);

  SHR_NULL_CHECK(tpid_info, _SHR_E_PARAM, "tpid_info");

  /*
   * Verify that TPID value is valid:
   */
  if (BCM_DNX_SWITCH_TPID_VALUE_INVALID == tpid_info->tpid_value)
  {
    SHR_ERR_EXIT(_SHR_E_PARAM, "tpid_value 0x%x is not valid!!!\n", BCM_DNX_SWITCH_TPID_VALUE_INVALID);
  }

exit:
  SHR_FUNC_EXIT;
}

/**
 * \brief
 * Verify TPID for BCM-API: bcm_dnx_switch_tpid_delete(). \n
 */
static shr_error_e
dnx_switch_tpid_delete_verify(
  int unit,
  bcm_switch_tpid_info_t * tpid_info)
{
  SHR_FUNC_INIT_VARS(unit);

  SHR_NULL_CHECK(tpid_info, _SHR_E_PARAM, "tpid_info");

  /*
   * Verify that TPID value is valid:
   */
  if (BCM_DNX_SWITCH_TPID_VALUE_INVALID == tpid_info->tpid_value)
  {
    SHR_ERR_EXIT(_SHR_E_PARAM, "tpid_value 0x%x is not valid thus cannot be deleted!!!\n",
                 BCM_DNX_SWITCH_TPID_VALUE_INVALID);
  }

exit:
  SHR_FUNC_EXIT;
}

/**
 * \brief
 * Verify TPID for BCM-API: bcm_dnx_switch_tpid_delete_all(). \n
 */
static shr_error_e
dnx_switch_tpid_delete_all_verify(
  int unit)
{
  SHR_FUNC_INIT_VARS(unit);

  SHR_EXIT();

exit:
  SHR_FUNC_EXIT;
}

/**
 * \brief
 * Verify STPID for BCM-API: bcm_dnx_switch_tpid_get_all(). \n
 */
static shr_error_e
dnx_switch_tpid_get_all_verify(
  int unit,
  int size,
  bcm_switch_tpid_info_t * tpid_info_array,
  int *count)
{
  SHR_FUNC_INIT_VARS(unit);

  SHR_NULL_CHECK(tpid_info_array, _SHR_E_PARAM, "tpid_info_array");

  SHR_NULL_CHECK(count, _SHR_E_PARAM, "count");

exit:
  SHR_FUNC_EXIT;
}

/**
 * \brief
 * Get the TPID value of a designated index in the parser's
 * TPIDs table.
 *
 * \par DIRECT INPUT
 *    \param [in] unit -
 *     Relevant unit.
 *    \param [in] tpid_index -
 *     Designated index to the table. 
 *   \param [in] tpid_value -
 *     Pointer to TPID value.
 * \par INDIRECT INPUT
 *     None.
 * \par DIRECT OUTPUT:
 *   \retval Negative in case of an error.
 *   \retval Zero in case of NO ERROR
 * \par INDIRECT OUTPUT
 *   tpid_value (see \ref tpid_info)
 * \remark 
 * This function is not a BCM API!! \n
 * It is implemented for SDK internal usage. \n
 */
shr_error_e
dnx_switch_tpid_value_get(
  int unit,
  int tpid_index,
  uint16 * tpid_value)
{
  uint32 entry_handle_id;

  SHR_FUNC_INIT_VARS(unit);

  SHR_NULL_CHECK(tpid_value, _SHR_E_PARAM, "tpid_value");

  if ((tpid_index < 0) || (tpid_index > BCM_DNX_SWITCH_TPID_NUM_OF_VALID_GLOBALS))
  {
    SHR_ERR_EXIT(_SHR_E_PARAM, "tpid_index %d is not valid!!!\n", tpid_index);
  }

  SHR_IF_ERR_EXIT(dbal_entry_handle_take(unit, DBAL_TABLE_SWITCH_TPID, &entry_handle_id));
  dbal_entry_key_field32_set(unit, entry_handle_id, DBAL_FIELD_TPID_INDEX, INST_SINGLE, tpid_index);
  
  {
    uint32 tpid_value_32bit;

    dbal_entry_value_field32_get(unit, entry_handle_id, DBAL_FIELD_TPID_VALUE, INST_SINGLE, &tpid_value_32bit);
    SHR_IF_ERR_EXIT(dbal_entry_get(unit, entry_handle_id, DBAL_COMMIT_NORMAL));

    *tpid_value = (uint16) tpid_value_32bit;
  }

exit:
  SHR_FUNC_EXIT;
}

/**
 * \brief
 * Set a TPID value at a designated index in the parser's TPIDs
 * table. 
 *
 * \par DIRECT INPUT
 *    \param [in] unit -
 *     Relevant unit.
 *    \param [in] tpid_index -
 *     Designated index to the table. 
 *   \param [in] tpid_value -
 *     TPID value.
 * \par INDIRECT INPUT
 *     None.
 * \par DIRECT OUTPUT:
 *   \retval Negative in case of an error.
 *   \retval Zero in case of NO ERROR
 * \par INDIRECT OUTPUT
 *   TPID HW table is updated (DBAL_TABLE_SWITCH_TPID)
 * \remark 
 * This function is not a BCM API!! \n
 * It is implemented for SDK internal usage. \n
 */
shr_error_e
dnx_switch_tpid_value_set(
  int unit,
  int tpid_index,
  uint16 tpid_value)
{
  uint32 entry_handle_id;

  SHR_FUNC_INIT_VARS(unit);

  if ((tpid_index < 0) || (tpid_index > BCM_DNX_SWITCH_TPID_NUM_OF_VALID_GLOBALS))
  {
    SHR_ERR_EXIT(_SHR_E_PARAM, "tpid_index %d is not valid!!!\n", tpid_index);
  }

  SHR_IF_ERR_EXIT(dbal_entry_handle_take(unit, DBAL_TABLE_SWITCH_TPID, &entry_handle_id));
  dbal_entry_key_field32_set(unit, entry_handle_id, DBAL_FIELD_TPID_INDEX, INST_SINGLE, tpid_index);
  dbal_entry_value_field16_set(unit, entry_handle_id, DBAL_FIELD_TPID_VALUE, INST_SINGLE, tpid_value);
  SHR_IF_ERR_EXIT(dbal_entry_commit(unit, entry_handle_id, DBAL_COMMIT_NORMAL));

exit:
  SHR_FUNC_EXIT;
}

/**
 * \brief
 * Get the index of a TPID value in the parser's TPIDs table.
 *
 * \par DIRECT INPUT
 *    \param [in] unit -
 *     Relevant unit.
 *    \param [in] tpid_index -
 *     Pointer to designated index at the table. 
 *   \param [in] tpid_value -
 *     TPID value.
 * \par INDIRECT INPUT
 *     None.
 * \par DIRECT OUTPUT:
 *   \retval Negative in case of an error.
 *   \retval Zero in case of NO ERROR
 * \par INDIRECT OUTPUT
 * * \par INDIRECT OUTPUT
 *  tpid_index (see \ref tpid_index)
 * \remark 
 * If TPID value is not found, the tpid_index is updated with 
 * BCM_DNX_SWITCH_TPID_INDEX_INVALID.\n 
 * This function is not a BCM API!! \n 
 * It is implemented for SDK internal usage. \n
 */
shr_error_e
dnx_switch_tpid_index_get(
  int unit,
  uint16 tpid_value,
  int *tpid_index)
{
  uint32 entry_handle_id;
  int tpid_tmp_index = 0;
  uint16 tpid_cur_value;

  SHR_FUNC_INIT_VARS(unit);

  SHR_NULL_CHECK(tpid_index, _SHR_E_PARAM, "tpid_index");

  SHR_IF_ERR_EXIT(dbal_entry_handle_take(unit, DBAL_TABLE_SWITCH_TPID, &entry_handle_id));

  *tpid_index = BCM_DNX_SWITCH_TPID_INDEX_INVALID;

  do
  {
    dbal_entry_key_field32_set(unit, entry_handle_id, DBAL_FIELD_TPID_INDEX, INST_SINGLE, tpid_tmp_index);
    
    {
      uint32 tpid_value_32bit;

      dbal_entry_value_field32_get(unit, entry_handle_id, DBAL_FIELD_TPID_VALUE, INST_SINGLE, &tpid_value_32bit);
      SHR_IF_ERR_EXIT(dbal_entry_get(unit, entry_handle_id, DBAL_COMMIT_KEEP_HANDLE));

      tpid_cur_value = (uint16) tpid_value_32bit;
    }

    if (tpid_cur_value == tpid_value)
    {
      /*
       * Match found! 
       */
      *tpid_index = tpid_tmp_index;
    }

    tpid_tmp_index++;
  }
  while ((tpid_tmp_index < BCM_DNX_SWITCH_TPID_NUM_OF_VALID_GLOBALS)
         && (*tpid_index == BCM_DNX_SWITCH_TPID_INDEX_INVALID));

  dbal_entry_handle_release(unit, entry_handle_id);

exit:
  SHR_FUNC_EXIT;
}

/**
 * \brief
 * BCM API: Adding a TPID value to the parser global TPID's 
 * table. 
 *
 * \par DIRECT INPUT
 *    \param [in] unit -
 *     Relevant unit.
 *    \param [in] options -
 *     TPID options - not supported at this stage. 
 *   \param [in] tpid_info -
 *     TPID information - only tpid_value is supported.
 * \par INDIRECT INPUT
 *   tpid_value (see \ref tpid_info)
 * \par DIRECT OUTPUT:
 *   \retval Negative in case of an error.
 *   \retval Zero in case of NO ERROR
 * \par INDIRECT OUTPUT
 * Write to TPID's table.
 * \remark 
 *  The options field is not supported at this stage.
 *  The tpid_info structure:
 *      - Only the tpid_value field will be supported for now.
 *      - The SDK will disallow adding the same TPID more than
 *          once.
 *      - An error is issued in case an addition is attempted
 *        while all 7 Global TPIDs are occupied (TPID 7 is
 *        reserved as NULL)
 * See the Programming Guide PP document for full details.
 */
int
bcm_dnx_switch_tpid_add(
  int unit,
  uint32 options,
  bcm_switch_tpid_info_t * tpid_info)
{
  uint32 entry_handle_id;
  uint16 tpid_cur_values[BCM_DNX_SWITCH_TPID_NUM_OF_VALID_GLOBALS];
  int first_vacant_tpid_entry = BCM_DNX_SWITCH_TPID_INDEX_INVALID;
  int tpid_index;

  SHR_FUNC_INIT_VARS(unit);

  SHR_INVOKE_VERIFY_DNX(dnx_switch_tpid_add_verify(unit, options, tpid_info));

  /*
   * Up to 7 different TPIDs can be set. 
   * Adding same TPID more than once is disallowed. 
   * Thus:
   *     - read all TPIDs values from TPID table.
   *     - check that the new TPID is not already exist.
   *     - update new TPID if there is a vacant entry.
   */

  SHR_IF_ERR_EXIT(dbal_entry_handle_take(unit, DBAL_TABLE_SWITCH_TPID, &entry_handle_id));

  /*
   * Read all TPIDs values from TPID table.
   * Check that the new TPID is not already exist. 
   * File the first vacant entry.
   */

  for (tpid_index = 0; tpid_index < BCM_DNX_SWITCH_TPID_NUM_OF_VALID_GLOBALS; tpid_index++)
  {
    dbal_entry_key_field32_set(unit, entry_handle_id, DBAL_FIELD_TPID_INDEX, INST_SINGLE, tpid_index);
    
    {
      uint32 tpid_value_32bit;

      dbal_entry_value_field32_get(unit, entry_handle_id, DBAL_FIELD_TPID_VALUE, INST_SINGLE, &tpid_value_32bit);
      SHR_IF_ERR_EXIT(dbal_entry_get(unit, entry_handle_id, DBAL_COMMIT_KEEP_HANDLE));

      tpid_cur_values[tpid_index] = (uint16) tpid_value_32bit;
    }

    if (tpid_cur_values[tpid_index] == tpid_info->tpid_value)
    {
      dbal_entry_handle_release(unit, entry_handle_id);

      SHR_ERR_EXIT(_SHR_E_CONFIG, "tpid_value 0x%x already exist in TPID table (at entry %d)!!!\n",
                   tpid_info->tpid_value, tpid_index);
    }

    if (BCM_DNX_SWITCH_TPID_VALUE_INVALID == tpid_cur_values[tpid_index])
    {
      if (BCM_DNX_SWITCH_TPID_INDEX_INVALID == first_vacant_tpid_entry)
      {
        first_vacant_tpid_entry = tpid_index;
      }
    }
  }

  /*
   * At this point, 'first_vacant_tpid_entry' contains the first vacant index or BCM_DNX_SWITCH_TPID_INDEX_INVALID if there is none. 
   * Update the TPID table with the new TPID if there is a vacant entry.
   */
  if (BCM_DNX_SWITCH_TPID_INDEX_INVALID == first_vacant_tpid_entry)
  {
    for (tpid_index = 0; tpid_index < BCM_DNX_SWITCH_TPID_NUM_OF_VALID_GLOBALS; tpid_index++)
    {
      LOG_INFO_EX(BSL_LOG_MODULE, "tpid_index %d, tpid_value = 0x%04X %s %s\n", tpid_index, tpid_cur_values[tpid_index],
                  EMPTY, EMPTY);
    }
    dbal_entry_handle_release(unit, entry_handle_id);

    SHR_ERR_EXIT(_SHR_E_FULL, "TPID table is full, cannot update with new value!!!\n");
  }

  dbal_entry_key_field32_set(unit, entry_handle_id, DBAL_FIELD_TPID_INDEX, INST_SINGLE, first_vacant_tpid_entry);
  dbal_entry_value_field16_set(unit, entry_handle_id, DBAL_FIELD_TPID_VALUE, INST_SINGLE, tpid_info->tpid_value);
  SHR_IF_ERR_EXIT(dbal_entry_commit(unit, entry_handle_id, DBAL_COMMIT_NORMAL));

exit:
  SHR_FUNC_EXIT;
}

/**
 * \brief
 * BCM_API:Delete a configured TPID from the parser TPID's
 * table.
 *
 * \par DIRECT INPUT
 *    \param [in] unit -
 *     Relevant unit.
 *   \param [in] tpid_info -
 *     TPID information - only tpid_value is supported.
 * \par INDIRECT INPUT:
 *   tpid_value (see \ref tpid_info)
 * \par DIRECT OUTPUT:
 * *   * None
    \retval Negative in case of an error.
 *   \retval Zero in case of NO ERROR
 * \par INDIRECT OUTPUT
 * TPID HW table (DBAL_TABLE_SWITCH_TPID).
 * * None.
  \remark 
 *  An error is returned if the TPID value isn't found.
 *  The deleted entry is filled with the invalid TPID value, i.e
 *  0x0000 (BCM_DNX_SWITCH_TPID_VALUE_INVALID).
 * See the Programming Guide PP document for full details.
 */
int
bcm_dnx_switch_tpid_delete(
  int unit,
  bcm_switch_tpid_info_t * tpid_info)
{
  uint32 entry_handle_id;
  int tpid_index = 0;
  uint16 tpid_value;
  int tpid_found = FALSE;

  SHR_FUNC_INIT_VARS(unit);

  SHR_INVOKE_VERIFY_DNX(dnx_switch_tpid_delete_verify(unit, tpid_info));

  SHR_IF_ERR_EXIT(dbal_entry_handle_take(unit, DBAL_TABLE_SWITCH_TPID, &entry_handle_id));

  do
  {
    
    {
      uint32 tpid_value_32bit;

      dbal_entry_key_field32_set(unit, entry_handle_id, DBAL_FIELD_TPID_INDEX, INST_SINGLE, tpid_index);
      dbal_entry_value_field32_get(unit, entry_handle_id, DBAL_FIELD_TPID_VALUE, INST_SINGLE, &tpid_value_32bit);
      SHR_IF_ERR_EXIT(dbal_entry_get(unit, entry_handle_id, DBAL_COMMIT_KEEP_HANDLE));

      tpid_value = (uint16) tpid_value_32bit;
    }

    if (tpid_info->tpid_value == tpid_value)
    {
      tpid_found = TRUE;

      /*
       * Invalidate the matched entry in the TPID table
       */

      dbal_entry_value_field16_set(unit, entry_handle_id, DBAL_FIELD_TPID_VALUE, INST_SINGLE, BCM_DNX_SWITCH_TPID_VALUE_INVALID);
      SHR_IF_ERR_EXIT(dbal_entry_commit(unit, entry_handle_id, DBAL_COMMIT_NORMAL));
    }

    tpid_index++;
  }
  while ((tpid_index < BCM_DNX_SWITCH_TPID_NUM_OF_VALID_GLOBALS) && (tpid_found == FALSE));

  if (FALSE == tpid_found)
  {
    dbal_entry_handle_release(unit, entry_handle_id);

    SHR_ERR_EXIT(_SHR_E_PARAM, "TPID value 0x%x was not found in TPID table\n", tpid_info->tpid_value);
  }

exit:
  SHR_FUNC_EXIT;
}

/**
 * \brief
 * BCM_API: Delete all configured TPIDs from parser TPIDs table.
 *
 * \par DIRECT INPUT
 *    \param [in] unit -
 *     Relevant unit.
 * \par INDIRECT INPUT:
 *   * None
 * \par DIRECT OUTPUT:
 * *   * None
    \retval Negative in case of an error.
 *   \retval Zero in case of NO ERROR
 * \par INDIRECT OUTPUT
 * TPID HW table (DBAL_TABLE_SWITCH_TPID).
 * * None.
  \remark 
 *  The deleted entries are filled with the invalid TPID value,
 *  i.e 0x0000 (BCM_DNX_SWITCH_TPID_VALUE_INVALID).
 * See the Programming Guide PP document for full details.
 */

int
bcm_dnx_switch_tpid_delete_all(
  int unit)
{
  SHR_FUNC_INIT_VARS(unit);

  SHR_INVOKE_VERIFY_DNX(dnx_switch_tpid_delete_all_verify(unit));

  SHR_IF_ERR_EXIT(dbal_table_clear(unit, DBAL_TABLE_SWITCH_TPID));

exit:
  SHR_FUNC_EXIT;
}

/**
 * \brief
 * BCM_API: Returns all the configured TPID values at the parser
 * TPID's table.
 *
 * \par DIRECT INPUT
 *    \param [in] unit -
 *     Relevant unit.
 *    \param [in] size -
 *     The size field won't be used.
 *    \param [in] tpid_info_array -
 *     Pointer to array of bcm_switch_tpid_info_t.
  \par INDIRECT INPUT:
 *   * None
 * \par DIRECT OUTPUT:
 *    \param [out] count -
 *     The total numbers of valid TPIDs.
    \retval Negative in case of an error.
 *   \retval Zero in case of NO ERROR
 * \par INDIRECT OUTPUT
 *   tpid_value (see \ref tpid_info_array)
 *  \remark 
 *  The size field won't be used.
 *  The tpid_info_array field is filled with only valid TPIDs
 *  value, i.e TPID != 0x0000
 *  (BCM_DNX_SWITCH_TPID_VALUE_INVALID).
 *  The count field returns total number of valid TPID values.
 * See the Programming Guide PP document for full details.
 */
int
bcm_dnx_switch_tpid_get_all(
  int unit,
  int size,
  bcm_switch_tpid_info_t * tpid_info_array,
  int *count)
{
  uint32 entry_handle_id;
  int tpid_index = 0;
  uint16 tpid_value;
  int tpid_count_valid = 0;

  SHR_FUNC_INIT_VARS(unit);

  SHR_INVOKE_VERIFY_DNX(dnx_switch_tpid_get_all_verify(unit, size, tpid_info_array, count));

  SHR_IF_ERR_EXIT(dbal_entry_handle_take(unit, DBAL_TABLE_SWITCH_TPID, &entry_handle_id));

  /*
   * Read all TPIDs values from TPID table.
   */

  do
  {
    
    {
      uint32 tpid_value_32bit;

      dbal_entry_key_field32_set(unit, entry_handle_id, DBAL_FIELD_TPID_INDEX, INST_SINGLE, tpid_index);
      dbal_entry_value_field32_get(unit, entry_handle_id, DBAL_FIELD_TPID_VALUE, INST_SINGLE, &tpid_value_32bit);
      SHR_IF_ERR_EXIT(dbal_entry_get(unit, entry_handle_id, DBAL_COMMIT_KEEP_HANDLE));

      tpid_value = (uint16) tpid_value_32bit;
    }

    if (BCM_DNX_SWITCH_TPID_VALUE_INVALID != tpid_value)
    {
      tpid_info_array->tpid_value = tpid_value;
      tpid_info_array++;
      tpid_count_valid++;
    }
    tpid_index++;

  }
  while (tpid_index < BCM_DNX_SWITCH_TPID_NUM_OF_VALID_GLOBALS);

  dbal_entry_handle_release(unit, entry_handle_id);

  *count = tpid_count_valid;

exit:
  SHR_FUNC_EXIT;
}
