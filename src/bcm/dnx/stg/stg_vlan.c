/*! \file stg_vlan.c
 * $Id$
 *
 * Spanning Tree procedures per vlan for DNX.
 * Allows to associate a STG-ID with a VLAN.
 *
 */
/*
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#ifdef BSL_LOG_MODULE
#  error "BSL_LOG_MODULE redefined"
#endif
#define BSL_LOG_MODULE BSL_LS_BCMDNX_STG

/*
 * Include files.
 * {
 */
#include <soc/dnx/dbal/dbal.h>
#include <bcm/types.h>
#include <bcm_int/dnx/vlan/vlan.h>

/*
 * }
 */

/*!
 * \brief
 * Verify STG-ID and VLAN for BCM-API: bcm_dnx_stg_vlan_add(). \n
 */
static shr_error_e
dnx_stg_vlan_add_verify(
  int unit,
  bcm_stg_t stg,
  bcm_vlan_t vid)
{
  SHR_FUNC_INIT_VARS(unit);

  /*
   * VID Validation 
   */
  BCM_DNX_VLAN_CHK_ID(unit, vid);

  

exit:
  SHR_FUNC_EXIT;
}

/*!
 * \brief
 * Verify STG-ID and VLAN for BCM-API: bcm_dnx_stg_vlan_remove(). \n
 */
static shr_error_e
dnx_stg_vlan_remove_verify(
  int unit,
  bcm_stg_t stg,
  bcm_vlan_t vid)
{
  SHR_FUNC_INIT_VARS(unit);

  BCM_DNX_VLAN_CHK_ID(unit, vid);

  

exit:
  SHR_FUNC_EXIT;
}

/*!
 * \brief
 * Verify STG-ID and VLAN for BCM-API: bcm_dnx_stg_vlan_remove_all(). \n
 */
static shr_error_e
dnx_stg_vlan_remove_all_verify(
  int unit,
  bcm_stg_t stg)
{
  SHR_FUNC_INIT_VARS(unit);
  SHR_EXIT();

  

exit:
  SHR_FUNC_EXIT;
}

/**
 * \brief
 * Associate a VLAN with a Spanning Tree Group.
 *
 * \par DIRECT INPUT
 *    \param [in] unit -
 *     Relevant unit.
 *    \param [in] stg -
 *     Specific STG to associate to. 
 *   \param [in] vid -
  *     The incoming VSI ID, must be in the range of 0-4K
 *     VSI - Virtual Switching Instance is a generalization of the VLAN concept used primarily in advanced bridging 
 *     application. A VSI interconnects Logical Interfaces (LIFs).
 *     VSI is a logical partition of the MAC table and a flooding domain (comprising its member interfaces).
 *     For more information about VSI , see the Programmer's
 *     Guide PP document.
 * \par INDIRECT INPUT:
 *   * None
 * \par DIRECT OUTPUT:
 *   \retval Negative in case of an error.
 *   \retval Zero in case of NO ERROR
 * \par INDIRECT OUTPUT
 * * Write to HW VSI table (via DBAL table
 *   DBAL_TABLE_ING_VSI_INFO).
 * * Update STG DB (Link-list?)
  \remark 
 *  For full details, please refer to "Jericho 2 Simple Bridge
 *  SDD" doc.
 */

int
bcm_dnx_stg_vlan_add(
  int unit,
  bcm_stg_t stg,
  bcm_vlan_t vid)
{
  uint32 entry_handle_id;

  SHR_FUNC_INIT_VARS(unit);

  SHR_INVOKE_VERIFY_DNX(dnx_stg_vlan_add_verify(unit, stg, vid));

  

  

  /*
   * Update the VSI table for non B-VID values 
   */
  SHR_IF_ERR_EXIT(dbal_entry_handle_take(unit, DBAL_TABLE_ING_VSI_INFO, &entry_handle_id));
  dbal_entry_key_field32_set(unit, entry_handle_id, DBAL_FIELD_VSI, INST_SINGLE, vid);
  dbal_entry_value_field32_set(unit, entry_handle_id, DBAL_FIELD_RESULT_TYPE, INST_SINGLE, DBAL_RESULT_TYPE_ING_VSI_INFO_BASIC_FORMAT);
  dbal_entry_value_field32_set(unit, entry_handle_id, DBAL_FIELD_STP_TOPOLOGY_ID, INST_SINGLE, stg);
  SHR_IF_ERR_EXIT(dbal_entry_commit(unit, entry_handle_id, DBAL_COMMIT_NORMAL));

exit:
  SHR_FUNC_EXIT;
}

/**
 * \brief
 * Removes a VLAN from a spanning tree group. 
 * The VLAN is placed back in the default spanning tree group. 
 *
 * \par DIRECT INPUT
 *    \param [in] unit -
 *     Relevant unit.
 *    \param [in] stg -
 *     STG ID to use. 
 *   \param [in] vid -
 *     VSI ID
 * \par INDIRECT INPUT:
 *   * HW VSI table (DBAL table
 *   DBAL_TABLE_ING_VSI_INFO).
 * \par DIRECT OUTPUT:
 *   \retval Negative in case of an error.
 *   \retval Zero in case of NO ERROR
 * \par INDIRECT OUTPUT
 * * Write to HW VSI table (via DBAL table
 *   DBAL_TABLE_ING_VSI_INFO).
 * * Update STG DB (Link-list?)
  \remark 
 *  
 */
int
bcm_dnx_stg_vlan_remove(
  int unit,
  bcm_stg_t stg,
  bcm_vlan_t vid)
{
  uint32 entry_handle_id;
  bcm_stg_t stg_defl;
  bcm_stg_t stg_cur;
  uint32 field_val;

  int rv;

  SHR_FUNC_INIT_VARS(unit);

  SHR_INVOKE_VERIFY_DNX(dnx_stg_vlan_remove_verify(unit, stg, vid));

  

  
  stg_defl = BCM_STG_DEFAULT;
  stg_cur = stg_defl;

  if (stg == stg_defl)
  {
    /*
     * Removing of Default STG is prohibited. 
     */
    SHR_ERR_EXIT(_SHR_E_PARAM, "can't remove stg %d because it is DEFAULT STG\n", stg);
  }

  /*
   * Get the STG the VLAN is currently associated to 
   */

  SHR_IF_ERR_EXIT(dbal_entry_handle_take(unit, DBAL_TABLE_ING_VSI_INFO, &entry_handle_id));
  dbal_entry_key_field32_set(unit, entry_handle_id, DBAL_FIELD_VSI, INST_SINGLE, vid);

  rv = dbal_entry_get( unit, entry_handle_id, DBAL_GET_ALL_FIELDS_AND_KEEP_HANDLE); 
     
  if(rv) 
  { 
      dbal_entry_handle_release(unit, entry_handle_id);
      SHR_ERR_EXIT(rv, "entry get failed\n");
  }
  rv = dbal_entry_handle_value_field32_get(unit,entry_handle_id, DBAL_FIELD_STP_TOPOLOGY_ID, INST_SINGLE, &field_val); 
  dbal_entry_handle_release(unit, entry_handle_id);
  stg_cur = field_val;
  if(rv || (stg_cur != stg))
  {       
      SHR_ERR_EXIT(_SHR_E_PARAM,
                 "vid %d is currently associated with STG ID %d, can't disassociate stg %d from this vid\n", vid,
                 stg_cur, stg);
  }

  /*
   * Set STG ID field at the vid entry of VSI table back to default STG 
   */
  SHR_IF_ERR_EXIT(dbal_entry_handle_take(unit, DBAL_TABLE_ING_VSI_INFO, &entry_handle_id));
  dbal_entry_key_field32_set(unit, entry_handle_id, DBAL_FIELD_VSI, INST_SINGLE, vid);
  dbal_entry_value_field32_set(unit, entry_handle_id, DBAL_FIELD_RESULT_TYPE, INST_SINGLE, DBAL_RESULT_TYPE_ING_VSI_INFO_BASIC_FORMAT);
  dbal_entry_value_field32_set(unit, entry_handle_id, DBAL_FIELD_STP_TOPOLOGY_ID, INST_SINGLE, stg_defl);
  SHR_IF_ERR_EXIT(dbal_entry_commit(unit, entry_handle_id, DBAL_COMMIT_NORMAL));

  

exit:
  SHR_FUNC_EXIT;
}

/**
 * \brief
 * Remove all VLANs from a spanning tree group.
 * The VLANs are placed in the default spanning tree group.
 *
 * \par DIRECT INPUT
 *    \param [in] unit -
 *     Relevant unit.
 *    \param [in] stg -
 *     STG ID to use. 
 * \par INDIRECT INPUT:
 *   * HW VSI table (DBAL table
 *   DBAL_TABLE_ING_VSI_INFO).
  \par DIRECT OUTPUT:
 *   \retval Negative in case of an error.
 *   \retval Zero in case of NO ERROR
 * \par INDIRECT OUTPUT
 * * Write to HW VSI table (via DBAL table
 *   DBAL_TABLE_ING_VSI_INFO).
 * * Update STG DB (Link-list?)
  \remark 
 *  
 */
int
bcm_dnx_stg_vlan_remove_all(
  int unit,
  bcm_stg_t stg)
{
  bcm_stg_t stg_defl;

  SHR_FUNC_INIT_VARS(unit);

  SHR_INVOKE_VERIFY_DNX(dnx_stg_vlan_remove_all_verify(unit, stg));

  

  
  stg_defl = BCM_STG_DEFAULT;

  if (stg == stg_defl)
  {
    /*
     * Removing of Default STG is prohibited
     */
    SHR_ERR_EXIT(_SHR_E_PARAM, "can't remove stg %d because it is DEFAULT STG\n", stg);
  }

  

exit:
  SHR_FUNC_EXIT;
}
