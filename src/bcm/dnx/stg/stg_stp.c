/*! \file stg_stp.c
 * $Id$
 *
 * Spanning Tree procedures per port for DNX.
 * Allows to set/get the STP state per STG index and Port
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
#include <bcm_int/dnx/algo/algo_gpm.h>

/*
 * }
 */

/*!
 * \brief
 * Verify STG-ID, Port and STP-State for BCM-API: bcm_dnx_stg_stp_set(). \n
 */
static shr_error_e
dnx_stg_stp_set_verify(
  int unit,
  bcm_stg_t stg,
  bcm_port_t port,
  int stp_state)
{
  SHR_FUNC_INIT_VARS(unit);
  SHR_EXIT();

  /*
   * STG Validation 
   */

  /*
   * Port validations 
   */

  /*
   * STP State validations 
   */

exit:
  SHR_FUNC_EXIT;
}

int
bcm_dnx_stg_stp_set(
  int unit,
  bcm_stg_t stg,
  bcm_port_t port,
  int stp_state)
{
  uint32 entry_handle_id;
  dnx_algo_gpm_gport_phy_info_t gport_info;

  SHR_FUNC_INIT_VARS(unit);

  SHR_INVOKE_VERIFY_DNX(dnx_stg_stp_set_verify(unit, stg, port, stp_state));

  /*
   * Get Port + Core.
   */
  SHR_IF_ERR_EXIT(dnx_algo_gpm_gport_phy_info_get(unit, port, DNX_ALGO_GPM_GPORT_TO_PHY_OP_NONE , &gport_info));

  

  /*
   * Write the STP state to the Ingress HW
   */
  SHR_IF_ERR_EXIT(dbal_entry_handle_take(unit, DBAL_TABLE_INGRESS_PORT_STP, &entry_handle_id));
  /* Why is it VLAN and not port */
  dbal_entry_key_field32_set(unit, entry_handle_id, DBAL_FIELD_VLAN_ID, INST_SINGLE, gport_info.internal_port_pp);
  dbal_entry_key_field32_set(unit, entry_handle_id, DBAL_FIELD_CORE_ID, INST_SINGLE, gport_info.internal_core_id);
  /* Need to add XML subtraction by 1 */
  dbal_entry_key_field32_set(unit, entry_handle_id, DBAL_FIELD_STG_ID, INST_SINGLE, stg);
  /* Need to add XML conversion */
  dbal_entry_value_field32_set(unit, entry_handle_id, DBAL_FIELD_STP_STATE, INST_SINGLE, stp_state);
  SHR_IF_ERR_EXIT(dbal_entry_commit(unit, entry_handle_id, DBAL_COMMIT_NORMAL));

  
  SHR_IF_ERR_EXIT(dbal_entry_handle_take(unit, DBAL_TABLE_EGRESS_PORT_STP, &entry_handle_id));
  dbal_entry_key_field32_set(unit, entry_handle_id, DBAL_FIELD_VLAN_ID, INST_SINGLE, gport_info.internal_port_pp);
  dbal_entry_key_field32_set(unit, entry_handle_id, DBAL_FIELD_CORE_ID, INST_SINGLE, gport_info.internal_core_id);
  dbal_entry_key_field32_set(unit, entry_handle_id, DBAL_FIELD_STG_ID, INST_SINGLE, stg);
  dbal_entry_value_field32_set(unit, entry_handle_id, DBAL_FIELD_STP_STATE, INST_SINGLE, stp_state); 
  SHR_IF_ERR_EXIT(dbal_entry_commit(unit, entry_handle_id, DBAL_COMMIT_NORMAL));

exit:
  SHR_FUNC_EXIT;
}
