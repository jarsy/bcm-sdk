/*
 * $Id: $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File: jer2_qax_ingress_packet_queuing.c
 */

#ifdef _ERR_MSG_MODULE_NAME
  #error "_ERR_MSG_MODULE_NAME redefined"
#endif

#define _ERR_MSG_MODULE_NAME BSL_SOC_INIT

/*************
 * INCLUDES  *
 *************/
#include <shared/bsl.h>
#include <soc/dnx/legacy/drv.h>
#include <soc/dnx/legacy/error.h>
#include <soc/dnx/legacy/QAX/qax_ingress_packet_queuing.h>
#include <soc/dnx/legacy/ARAD/arad_ingress_packet_queuing.h>

int
  jer2_qax_iqm_dynamic_tbl_get_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   int             core,
    DNX_SAND_IN   uint32          entry_offset,
    DNX_SAND_OUT  JER2_ARAD_IQM_DYNAMIC_TBL_DATA* IQM_dynamic_tbl_data
  )
{
    soc_reg_above_64_val_t data_above_64;

    DNXC_INIT_FUNC_DEFS;

    SOC_REG_ABOVE_64_CLEAR(data_above_64);
    DNXC_IF_ERR_EXIT(READ_CGM_VOQ_SIZEm(unit, CGM_BLOCK(unit, core), entry_offset, &data_above_64));

    IQM_dynamic_tbl_data->pq_inst_que_size = soc_CGM_VOQ_SIZEm_field32_get(unit, &data_above_64, WORDS_SIZEf);
    IQM_dynamic_tbl_data->pq_avrg_szie = soc_CGM_VOQ_SIZEm_field32_get(unit, &data_above_64, AVRG_SIZEf);

exit:
    DNXC_FUNC_RETURN;
}

int
jer2_qax_ipq_explicit_mapping_mode_info_get(
   DNX_SAND_IN  int                            unit,
   DNX_SAND_OUT JER2_ARAD_IPQ_EXPLICIT_MAPPING_MODE_INFO *info
   ) 
{

    uint32 fld_val;

    DNXC_INIT_FUNC_DEFS;

    jer2_arad_JER2_ARAD_IPQ_EXPLICIT_MAPPING_MODE_INFO_clear(info);

    DNXC_IF_ERR_EXIT(soc_reg_above_64_field32_read(unit, TAR_FLOW_BASE_QUEUEr, REG_PORT_ANY, 0, FLOW_BASE_QUEUE_0f, &fld_val));

    info->base_queue_id = fld_val;
    info->queue_id_add_not_decrement = TRUE;

exit:
    DNXC_FUNC_RETURN;
}

int
jer2_qax_ipq_explicit_mapping_mode_info_set(
   DNX_SAND_IN  int                            unit,
   DNX_SAND_IN JER2_ARAD_IPQ_EXPLICIT_MAPPING_MODE_INFO *info
   ) 
{

    uint32 fld_val, res;
    int profile_id;

    DNXC_INIT_FUNC_DEFS;
    
    res = jer2_arad_ipq_explicit_mapping_mode_info_verify(unit, info);
    DNXC_SAND_IF_ERR_EXIT(res);
    
    
   /*
    * All the Base_Q Flow is implemented trough the TAR table:
    * See Arch-PP-Spec figure 15: per flow, a profile is got to set its TC-mapping and its Base-Flow
    * No reason for a Base-Flow per Flow-Id, so a global one is set
    */
    fld_val = info->base_queue_id;
    for (profile_id = 0; profile_id < JER2_ARAD_NOF_INGRESS_FLOW_TC_MAPPING_PROFILES; ++profile_id) {
        DNXC_IF_ERR_EXIT(soc_reg_above_64_field32_modify(unit, TAR_FLOW_BASE_QUEUEr, REG_PORT_ANY,  0, FLOW_BASE_QUEUE_0f + profile_id,  fld_val));
    }

exit:
    DNXC_FUNC_RETURN;
}

int
jer2_qax_ipq_default_invalid_queue_set(
   DNX_SAND_IN  int            unit,
   DNX_SAND_IN  int            core,
   DNX_SAND_IN  uint32         queue_id,
   DNX_SAND_IN  int            enable) 
{
    DNXC_INIT_FUNC_DEFS;

    if (enable) {
        DNXC_IF_ERR_EXIT(WRITE_TAR_INVALID_DESTINATION_QUEUEr(unit, queue_id));
    } else { /* mark as invalid */
        DNXC_IF_ERR_EXIT(WRITE_TAR_INVALID_DESTINATION_QUEUEr(unit, JER2_ARAD_IPQ_DESTINATION_ID_INVALID_QUEUE(unit)));
    }
            
exit:
    DNXC_FUNC_RETURN;
}

int
jer2_qax_ipq_default_invalid_queue_get(
   DNX_SAND_IN  int            unit,
   DNX_SAND_IN  int            core,
   DNX_SAND_OUT uint32         *queue_id,
   DNX_SAND_OUT int            *enable) 
{
    DNXC_INIT_FUNC_DEFS;

    DNXC_IF_ERR_EXIT(READ_TAR_INVALID_DESTINATION_QUEUEr(unit, queue_id));
    
    if (*queue_id != JER2_ARAD_IPQ_DESTINATION_ID_INVALID_QUEUE(unit)) {
        *enable = 1;
    } else {
        *enable = 0;
    }

exit:
    DNXC_FUNC_RETURN;
}

#undef _ERR_MSG_MODULE_NAME

