/*
 * $Id: $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File: qax_ingress_packet_queuing.c
 */

#ifdef _ERR_MSG_MODULE_NAME
  #error "_ERR_MSG_MODULE_NAME redefined"
#endif

#define _ERR_MSG_MODULE_NAME BSL_SOC_INIT

/*************
 * INCLUDES  *
 *************/
#include <shared/bsl.h>
#include <soc/dpp/drv.h>
#include <soc/dpp/QAX/qax_ingress_packet_queuing.h>
#include <soc/dpp/ARAD/arad_ingress_packet_queuing.h>

int
  qax_iqm_dynamic_tbl_get_unsafe(
    SOC_SAND_IN   int             unit,
    SOC_SAND_IN   int             core,
    SOC_SAND_IN   uint32          entry_offset,
    SOC_SAND_OUT  ARAD_IQM_DYNAMIC_TBL_DATA* IQM_dynamic_tbl_data
  )
{
    soc_reg_above_64_val_t data_above_64;

    SOCDNX_INIT_FUNC_DEFS;

    SOC_REG_ABOVE_64_CLEAR(data_above_64);
    SOCDNX_IF_ERR_EXIT(READ_CGM_VOQ_SIZEm(unit, CGM_BLOCK(unit, core), entry_offset, &data_above_64));

    IQM_dynamic_tbl_data->pq_inst_que_size = soc_CGM_VOQ_SIZEm_field32_get(unit, &data_above_64, WORDS_SIZEf);
    IQM_dynamic_tbl_data->pq_avrg_szie = soc_CGM_VOQ_SIZEm_field32_get(unit, &data_above_64, AVRG_SIZEf);

exit:
    SOCDNX_FUNC_RETURN;
}

int
qax_ipq_explicit_mapping_mode_info_get(
   SOC_SAND_IN  int                            unit,
   SOC_SAND_OUT ARAD_IPQ_EXPLICIT_MAPPING_MODE_INFO *info
   ) 
{

    uint32 fld_val;

    SOCDNX_INIT_FUNC_DEFS;

    arad_ARAD_IPQ_EXPLICIT_MAPPING_MODE_INFO_clear(info);

    SOCDNX_IF_ERR_EXIT(soc_reg_above_64_field32_read(unit, TAR_FLOW_BASE_QUEUEr, REG_PORT_ANY, 0, FLOW_BASE_QUEUE_0f, &fld_val));

    info->base_queue_id = fld_val;
    info->queue_id_add_not_decrement = TRUE;

exit:
    SOCDNX_FUNC_RETURN;
}

int
qax_ipq_explicit_mapping_mode_info_set(
   SOC_SAND_IN  int                            unit,
   SOC_SAND_IN ARAD_IPQ_EXPLICIT_MAPPING_MODE_INFO *info
   ) 
{

    uint32 fld_val, res;
    int profile_id;

    SOCDNX_INIT_FUNC_DEFS;
    
    res = arad_ipq_explicit_mapping_mode_info_verify(unit, info);
    SOCDNX_SAND_IF_ERR_EXIT(res);
    
    
   /*
    * All the Base_Q Flow is implemented trough the TAR table:
    * See Arch-PP-Spec figure 15: per flow, a profile is got to set its TC-mapping and its Base-Flow
    * No reason for a Base-Flow per Flow-Id, so a global one is set
    */
    fld_val = info->base_queue_id;
    for (profile_id = 0; profile_id < ARAD_NOF_INGRESS_FLOW_TC_MAPPING_PROFILES; ++profile_id) {
        SOCDNX_IF_ERR_EXIT(soc_reg_above_64_field32_modify(unit, TAR_FLOW_BASE_QUEUEr, REG_PORT_ANY,  0, FLOW_BASE_QUEUE_0f + profile_id,  fld_val));
    }

exit:
    SOCDNX_FUNC_RETURN;
}

int
qax_ipq_default_invalid_queue_set(
   SOC_SAND_IN  int            unit,
   SOC_SAND_IN  int            core,
   SOC_SAND_IN  uint32         queue_id,
   SOC_SAND_IN  int            enable) 
{
    SOCDNX_INIT_FUNC_DEFS;

    if (enable) {
        SOCDNX_IF_ERR_EXIT(WRITE_TAR_INVALID_DESTINATION_QUEUEr(unit, queue_id));
    } else { /* mark as invalid */
        SOCDNX_IF_ERR_EXIT(WRITE_TAR_INVALID_DESTINATION_QUEUEr(unit, ARAD_IPQ_DESTINATION_ID_INVALID_QUEUE(unit)));
    }
            
exit:
    SOCDNX_FUNC_RETURN;
}

int
qax_ipq_default_invalid_queue_get(
   SOC_SAND_IN  int            unit,
   SOC_SAND_IN  int            core,
   SOC_SAND_OUT uint32         *queue_id,
   SOC_SAND_OUT int            *enable) 
{
    SOCDNX_INIT_FUNC_DEFS;

    SOCDNX_IF_ERR_EXIT(READ_TAR_INVALID_DESTINATION_QUEUEr(unit, queue_id));
    
    if (*queue_id != ARAD_IPQ_DESTINATION_ID_INVALID_QUEUE(unit)) {
        *enable = 1;
    } else {
        *enable = 0;
    }

exit:
    SOCDNX_FUNC_RETURN;
}

int
  qax_ipq_traffic_class_multicast_priority_map_set(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  uint32              traffic_class,
    SOC_SAND_IN  uint8               enable
  )
{
    uint32 data = 0;
    uint32 field_val[1] = {0}; /* using array because of BITSET coverity issue */
    SOCDNX_INIT_FUNC_DEFS;

    if (traffic_class > SOC_TMC_NOF_TRAFFIC_CLASSES) {
        SOCDNX_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_SOCDNX_MSG("TC %u is out of range\n"), traffic_class));
    }

    SOCDNX_IF_ERR_EXIT(READ_CGM_MC_TC_PRIORITY_MAPr(unit, &data));

    field_val[0] = soc_reg_field_get(unit, CGM_MC_TC_PRIORITY_MAPr, data, MC_TC_PRIORITY_MAPf);
    if (enable) {
        SHR_BITSET(field_val, traffic_class);
    } else {
        SHR_BITCLR(field_val, traffic_class);
    }
    soc_reg_field_set(unit, CGM_MC_TC_PRIORITY_MAPr, &data, MC_TC_PRIORITY_MAPf, field_val[0]);

    SOCDNX_IF_ERR_EXIT(WRITE_CGM_MC_TC_PRIORITY_MAPr(unit, data));

exit:
    SOCDNX_FUNC_RETURN;
}

int
  qax_ipq_traffic_class_multicast_priority_map_get(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  uint32              traffic_class,
    SOC_SAND_OUT uint8               *enable
  )
{
    uint32 data = 0;
    uint32 field_val[1] = {0}; /* using array because of BITGET coverity issue */
    SOCDNX_INIT_FUNC_DEFS;

    SOCDNX_NULL_CHECK(enable);

    if (traffic_class > SOC_TMC_NOF_TRAFFIC_CLASSES) {
        SOCDNX_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_SOCDNX_MSG("TC %u is out of range\n"), traffic_class));
    }

    SOCDNX_IF_ERR_EXIT(READ_CGM_MC_TC_PRIORITY_MAPr(unit, &data));

    field_val[0] = soc_reg_field_get(unit, CGM_MC_TC_PRIORITY_MAPr, data, MC_TC_PRIORITY_MAPf);
    *enable = SHR_BITGET(field_val, traffic_class) ? TRUE : FALSE;

exit:
    SOCDNX_FUNC_RETURN;
}

#undef _ERR_MSG_MODULE_NAME

