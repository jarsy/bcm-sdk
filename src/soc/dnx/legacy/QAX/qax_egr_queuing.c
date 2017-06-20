/*
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
 
#ifdef _ERR_MSG_MODULE_NAME
  #error "_ERR_MSG_MODULE_NAME redefined"
#endif

#define _ERR_MSG_MODULE_NAME BSL_SOC_EGRESS
#include <shared/bsl.h>
#include <shared/swstate/access/sw_state_access.h>
#include <soc/register.h>
#include <soc/dnx/legacy/drv.h>
#include <soc/dnx/legacy/port_sw_db.h>
#include <soc/dnx/legacy/ARAD/arad_sw_db.h>
#include <soc/dnx/legacy/ARAD/arad_tbl_access.h>
#include <soc/dnx/legacy/ARAD/arad_ofp_rates.h>
#include <soc/dnx/legacy/JER/jer_egr_queuing.h>
#include <soc/dnx/legacy/ARAD/arad_egr_queuing.h>

#include <soc/dnx/legacy/QAX/qax_egr_queuing.h>

#define SOC_JER2_QAX_EGR_MAX_ALPHA_VALUE (7)
#define SOC_JER2_QAX_EGR_MIN_ALPHA_VALUE (-7)

static int
_soc_jer2_qax_convert_alpha_to_value(int unit, int alpha_value, uint32* field_val)
{
    DNXC_INIT_FUNC_DEFS;
    if (alpha_value > SOC_JER2_QAX_EGR_MAX_ALPHA_VALUE || alpha_value < SOC_JER2_QAX_EGR_MIN_ALPHA_VALUE)
    {
        DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("Invalid alpha value %d "), alpha_value));
    }

    else if (alpha_value == 0)
    {
        *field_val = 0;
    }
    else if (alpha_value > 0)
    {
        *field_val = alpha_value;
    }
    else
    {
        *field_val = -1*alpha_value;
        *field_val |= 0x8; /* MSB must be 1 to indicate negative number */
    }

exit:
    DNXC_FUNC_RETURN;
}

static int 
_soc_jer2_qax_convert_field_val_to_alpha(int unit, uint32 field_val, int* alpha_value)
{
    uint32 abs_value = 0;
    DNXC_INIT_FUNC_DEFS;
    abs_value = field_val & 0x7; /* 3 LS bits*/
    *alpha_value = abs_value;
    if (field_val & 0x8)
    {
        *alpha_value = *alpha_value * (-1);
    }

    DNXC_FUNC_RETURN;
}
/*
 * Get OFP FC threshold per q-pair (egress_tc) and threshold type.
 * Thresholds are of dbuff (256B) and packet descriptors (pd)
 */
static
  int
    jer2_qax_egr_ofp_fc_q_pair_thresh_get(
      DNX_SAND_IN  int                 unit,
      DNX_SAND_IN  int                 core,
      DNX_SAND_IN  uint32                 egress_tc,
      DNX_SAND_IN  uint32                 threshold_type,
      DNX_SAND_OUT JER2_ARAD_EGR_THRESH_INFO      *thresh_info
    )
{
  uint32
    offset;
  soc_reg_above_64_val_t data;

  DNXC_INIT_FUNC_DEFS;

  DNXC_NULL_CHECK(thresh_info);
  SOC_REG_ABOVE_64_CLEAR(data);
  /* QDCT offset */
  offset = JER2_ARAD_EGQ_QDCT_TABLE_KEY_ENTRY(threshold_type,egress_tc);

  /* Write PD threshold */
  DNXC_IF_ERR_EXIT(READ_EGQ_QDCT_TABLEm(unit, EGQ_BLOCK(unit, core), offset, data));
  thresh_info->packet_descriptors = soc_EGQ_QDCT_TABLEm_field32_get(unit, data, QUEUE_UC_PD_MAX_FC_THf);

  /* READ Dbuff threshold */
  DNXC_IF_ERR_EXIT(READ_EGQ_QQST_TABLEm(unit, EGQ_BLOCK(unit, core), offset, data));
  thresh_info->dbuff = soc_EGQ_QQST_TABLEm_field32_get(unit, data, QUEUE_UC_DB_MAX_FC_THf);

exit:
    DNXC_FUNC_RETURN;
}

int
  jer2_qax_egr_queuing_if_fc_uc_max_set(
	DNX_SAND_IN	int	unit,
    DNX_SAND_IN int core,
	DNX_SAND_IN	uint32	uc_if_profile_ndx,
	DNX_SAND_IN JER2_ARAD_EGR_QUEUING_IF_UC_FC	*info
  )
{
  soc_reg_above_64_val_t
    pd,
    size_256,
    pd_field,
    size_256_field;

  DNXC_INIT_FUNC_DEFS;
  DNXC_NULL_CHECK(info);

  SOC_REG_ABOVE_64_CLEAR(pd_field);
  SOC_REG_ABOVE_64_CLEAR(size_256_field);
  DNXC_IF_ERR_EXIT(READ_ECGM_CGM_UC_PD_INTERFACE_FC_MAX_THr(unit, pd));
  SHR_BITCOPY_RANGE(pd_field, 0, &info->pd_th, 0, JER2_ARAD_EGQ_PD_INTERFACE_NOF_BITS);

  /* CGM_CGM_UC_SIZE_256_INTERFACE_FC_THr */
  DNXC_IF_ERR_EXIT(READ_ECGM_CGM_UC_SIZE_256_INTERFACE_FC_MAX_THr(unit, size_256));
  SHR_BITCOPY_RANGE(size_256_field, 0, &info->size256_th, 0, JER2_ARAD_EGQ_SIZE_256_INTERFACE_NOF_BITS);

  switch(uc_if_profile_ndx) {
    case 0:
      soc_reg_above_64_field_set(unit, ECGM_CGM_UC_PD_INTERFACE_FC_MAX_THr, pd, CGM_UC_PD_INTERFACE_FC_MAX_TH_0f, pd_field);
      soc_reg_above_64_field_set(unit, ECGM_CGM_UC_SIZE_256_INTERFACE_FC_MAX_THr, size_256, CGM_UC_SIZE_256_INTERFACE_FC_MAX_TH_0f, size_256_field);
      break;
    case 1:
      soc_reg_above_64_field_set(unit, ECGM_CGM_UC_PD_INTERFACE_FC_MAX_THr, pd, CGM_UC_PD_INTERFACE_FC_MAX_TH_1f, pd_field);
      soc_reg_above_64_field_set(unit, ECGM_CGM_UC_SIZE_256_INTERFACE_FC_MAX_THr, size_256, CGM_UC_SIZE_256_INTERFACE_FC_MAX_TH_1f, size_256_field);
      break;
    case 2:
      soc_reg_above_64_field_set(unit, ECGM_CGM_UC_PD_INTERFACE_FC_MAX_THr, pd, CGM_UC_PD_INTERFACE_FC_MAX_TH_2f, pd_field);
      soc_reg_above_64_field_set(unit, ECGM_CGM_UC_SIZE_256_INTERFACE_FC_MAX_THr, size_256, CGM_UC_SIZE_256_INTERFACE_FC_MAX_TH_2f, size_256_field);
      break;
    case 3:
      soc_reg_above_64_field_set(unit, ECGM_CGM_UC_PD_INTERFACE_FC_MAX_THr, pd, CGM_UC_PD_INTERFACE_FC_MAX_TH_3f, pd_field);
      soc_reg_above_64_field_set(unit, ECGM_CGM_UC_SIZE_256_INTERFACE_FC_MAX_THr, size_256, CGM_UC_SIZE_256_INTERFACE_FC_MAX_TH_3f, size_256_field);
      break;
    case 4:
      soc_reg_above_64_field_set(unit, ECGM_CGM_UC_PD_INTERFACE_FC_MAX_THr, pd, CGM_UC_PD_INTERFACE_FC_MAX_TH_4f, pd_field);
      soc_reg_above_64_field_set(unit, ECGM_CGM_UC_SIZE_256_INTERFACE_FC_MAX_THr, size_256, CGM_UC_SIZE_256_INTERFACE_FC_MAX_TH_4f, size_256_field);
      break;
    case 5:
      soc_reg_above_64_field_set(unit, ECGM_CGM_UC_PD_INTERFACE_FC_MAX_THr, pd, CGM_UC_PD_INTERFACE_FC_MAX_TH_5f, pd_field);
      soc_reg_above_64_field_set(unit, ECGM_CGM_UC_SIZE_256_INTERFACE_FC_MAX_THr, size_256, CGM_UC_SIZE_256_INTERFACE_FC_MAX_TH_5f, size_256_field);
      break;
    case 6:
      soc_reg_above_64_field_set(unit, ECGM_CGM_UC_PD_INTERFACE_FC_MAX_THr, pd, CGM_UC_PD_INTERFACE_FC_MAX_TH_6f, pd_field);
      soc_reg_above_64_field_set(unit, ECGM_CGM_UC_SIZE_256_INTERFACE_FC_MAX_THr, size_256, CGM_UC_SIZE_256_INTERFACE_FC_MAX_TH_6f, size_256_field);
      break;
    case 7:
      soc_reg_above_64_field_set(unit, ECGM_CGM_UC_PD_INTERFACE_FC_MAX_THr, pd, CGM_UC_PD_INTERFACE_FC_MAX_TH_7f, pd_field);
      soc_reg_above_64_field_set(unit, ECGM_CGM_UC_SIZE_256_INTERFACE_FC_MAX_THr, size_256, CGM_UC_SIZE_256_INTERFACE_FC_MAX_TH_7f, size_256_field);
      break;
    default:
      DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("")));
  }

  DNXC_IF_ERR_EXIT(WRITE_ECGM_CGM_UC_PD_INTERFACE_FC_MAX_THr(unit, pd));
  DNXC_IF_ERR_EXIT(WRITE_ECGM_CGM_UC_SIZE_256_INTERFACE_FC_MAX_THr(unit, size_256));

exit:
  DNXC_FUNC_RETURN;
}


int
  jer2_qax_egr_queuing_if_fc_uc_set(
	DNX_SAND_IN	int	unit,
    DNX_SAND_IN int core,
	DNX_SAND_IN	uint32	uc_if_profile_ndx,
	DNX_SAND_IN JER2_ARAD_EGR_QUEUING_IF_UC_FC	*info
  )
{
    soc_reg_above_64_val_t reg_pd_min, reg_db_min;
    uint32 reg_pd_alpha, reg_db_alpha,field_val;
    DNXC_INIT_FUNC_DEFS;
    DNXC_IF_ERR_EXIT(READ_ECGM_CGM_UC_PD_INTERFACE_FC_MIN_THr(unit, reg_pd_min));
    DNXC_IF_ERR_EXIT(READ_ECGM_CGM_UC_PD_INTERFACE_FC_ALPHAr(unit, &reg_pd_alpha));
    DNXC_IF_ERR_EXIT(READ_ECGM_CGM_UC_SIZE_256_INTERFACE_FC_MIN_THr(unit, reg_db_min));
    DNXC_IF_ERR_EXIT(READ_ECGM_CGM_UC_SIZE_256_INTERFACE_FC_ALPHAr(unit, &reg_db_alpha));

    DNXC_IF_ERR_EXIT(_soc_jer2_qax_convert_alpha_to_value(unit, info->size256_th_alpha, &field_val));

    switch(uc_if_profile_ndx)
    {
        case 0:
            soc_reg_above_64_field32_set(unit, ECGM_CGM_UC_PD_INTERFACE_FC_MIN_THr, reg_pd_min, CGM_UC_PD_INTERFACE_FC_MIN_TH_0f, info->pd_th_min);
            soc_reg_field_set(unit, ECGM_CGM_UC_PD_INTERFACE_FC_ALPHAr, &reg_pd_alpha, CGM_UC_PD_INTERFACE_FC_ALPHA_0f, info->pd_th_alpha);
            soc_reg_above_64_field32_set(unit, ECGM_CGM_UC_SIZE_256_INTERFACE_FC_MIN_THr, reg_db_min, CGM_UC_SIZE_256_INTERFACE_FC_MIN_TH_0f, info->size256_th_min);
            soc_reg_field_set(unit, ECGM_CGM_UC_SIZE_256_INTERFACE_FC_ALPHAr, &reg_db_alpha, CGM_UC_SIZE_256_INTERFACE_FC_ALPHA_0f, field_val);
            break;
        case 1:
            soc_reg_above_64_field32_set(unit, ECGM_CGM_UC_PD_INTERFACE_FC_MIN_THr, reg_pd_min, CGM_UC_PD_INTERFACE_FC_MIN_TH_1f, info->pd_th_min);
            soc_reg_field_set(unit, ECGM_CGM_UC_PD_INTERFACE_FC_ALPHAr, &reg_pd_alpha, CGM_UC_PD_INTERFACE_FC_ALPHA_1f, info->pd_th_alpha);
            soc_reg_above_64_field32_set(unit, ECGM_CGM_UC_SIZE_256_INTERFACE_FC_MIN_THr, reg_db_min, CGM_UC_SIZE_256_INTERFACE_FC_MIN_TH_1f, info->size256_th_min);
            soc_reg_field_set(unit, ECGM_CGM_UC_SIZE_256_INTERFACE_FC_ALPHAr, &reg_db_alpha, CGM_UC_SIZE_256_INTERFACE_FC_ALPHA_1f, field_val);
            break;
        case 2:
            soc_reg_above_64_field32_set(unit, ECGM_CGM_UC_PD_INTERFACE_FC_MIN_THr, reg_pd_min, CGM_UC_PD_INTERFACE_FC_MIN_TH_2f, info->pd_th_min);
            soc_reg_field_set(unit, ECGM_CGM_UC_PD_INTERFACE_FC_ALPHAr, &reg_pd_alpha, CGM_UC_PD_INTERFACE_FC_ALPHA_2f, info->pd_th_alpha);
            soc_reg_above_64_field32_set(unit, ECGM_CGM_UC_SIZE_256_INTERFACE_FC_MIN_THr, reg_db_min, CGM_UC_SIZE_256_INTERFACE_FC_MIN_TH_2f, info->size256_th_min);
            soc_reg_field_set(unit, ECGM_CGM_UC_SIZE_256_INTERFACE_FC_ALPHAr, &reg_db_alpha, CGM_UC_SIZE_256_INTERFACE_FC_ALPHA_2f, field_val);
            break;
        case 3:
            soc_reg_above_64_field32_set(unit, ECGM_CGM_UC_PD_INTERFACE_FC_MIN_THr, reg_pd_min, CGM_UC_PD_INTERFACE_FC_MIN_TH_3f, info->pd_th_min);
            soc_reg_field_set(unit, ECGM_CGM_UC_PD_INTERFACE_FC_ALPHAr, &reg_pd_alpha, CGM_UC_PD_INTERFACE_FC_ALPHA_3f, info->pd_th_alpha);
            soc_reg_above_64_field32_set(unit, ECGM_CGM_UC_SIZE_256_INTERFACE_FC_MIN_THr, reg_db_min, CGM_UC_SIZE_256_INTERFACE_FC_MIN_TH_3f, info->size256_th_min);
            soc_reg_field_set(unit, ECGM_CGM_UC_SIZE_256_INTERFACE_FC_ALPHAr, &reg_db_alpha, CGM_UC_SIZE_256_INTERFACE_FC_ALPHA_3f, field_val);
            break;
        case 4:
            soc_reg_above_64_field32_set(unit, ECGM_CGM_UC_PD_INTERFACE_FC_MIN_THr, reg_pd_min, CGM_UC_PD_INTERFACE_FC_MIN_TH_4f, info->pd_th_min);
            soc_reg_field_set(unit, ECGM_CGM_UC_PD_INTERFACE_FC_ALPHAr, &reg_pd_alpha, CGM_UC_PD_INTERFACE_FC_ALPHA_4f, info->pd_th_alpha);
            soc_reg_above_64_field32_set(unit, ECGM_CGM_UC_SIZE_256_INTERFACE_FC_MIN_THr, reg_db_min, CGM_UC_SIZE_256_INTERFACE_FC_MIN_TH_4f, info->size256_th_min);
            soc_reg_field_set(unit, ECGM_CGM_UC_SIZE_256_INTERFACE_FC_ALPHAr, &reg_db_alpha, CGM_UC_SIZE_256_INTERFACE_FC_ALPHA_4f, field_val);
            break;
        case 5:
            soc_reg_above_64_field32_set(unit, ECGM_CGM_UC_PD_INTERFACE_FC_MIN_THr, reg_pd_min, CGM_UC_PD_INTERFACE_FC_MIN_TH_5f, info->pd_th_min);
            soc_reg_field_set(unit, ECGM_CGM_UC_PD_INTERFACE_FC_ALPHAr, &reg_pd_alpha, CGM_UC_PD_INTERFACE_FC_ALPHA_5f, info->pd_th_alpha);
            soc_reg_above_64_field32_set(unit, ECGM_CGM_UC_SIZE_256_INTERFACE_FC_MIN_THr, reg_db_min, CGM_UC_SIZE_256_INTERFACE_FC_MIN_TH_5f, info->size256_th_min);
            soc_reg_field_set(unit, ECGM_CGM_UC_SIZE_256_INTERFACE_FC_ALPHAr, &reg_db_alpha, CGM_UC_SIZE_256_INTERFACE_FC_ALPHA_5f, field_val);
            break;
        case 6:
            soc_reg_above_64_field32_set(unit, ECGM_CGM_UC_PD_INTERFACE_FC_MIN_THr, reg_pd_min, CGM_UC_PD_INTERFACE_FC_MIN_TH_6f, info->pd_th_min);
            soc_reg_field_set(unit, ECGM_CGM_UC_PD_INTERFACE_FC_ALPHAr, &reg_pd_alpha, CGM_UC_PD_INTERFACE_FC_ALPHA_6f, info->pd_th_alpha);
            soc_reg_above_64_field32_set(unit, ECGM_CGM_UC_SIZE_256_INTERFACE_FC_MIN_THr, reg_db_min, CGM_UC_SIZE_256_INTERFACE_FC_MIN_TH_6f, info->size256_th_min);
            soc_reg_field_set(unit, ECGM_CGM_UC_SIZE_256_INTERFACE_FC_ALPHAr, &reg_db_alpha, CGM_UC_SIZE_256_INTERFACE_FC_ALPHA_6f, field_val);
            break;
        case 7:
            soc_reg_above_64_field32_set(unit, ECGM_CGM_UC_PD_INTERFACE_FC_MIN_THr, reg_pd_min, CGM_UC_PD_INTERFACE_FC_MIN_TH_7f, info->pd_th_min);
            soc_reg_field_set(unit, ECGM_CGM_UC_PD_INTERFACE_FC_ALPHAr, &reg_pd_alpha, CGM_UC_PD_INTERFACE_FC_ALPHA_7f, info->pd_th_alpha);
            soc_reg_above_64_field32_set(unit, ECGM_CGM_UC_SIZE_256_INTERFACE_FC_MIN_THr, reg_db_min, CGM_UC_SIZE_256_INTERFACE_FC_MIN_TH_7f, info->size256_th_min);
            soc_reg_field_set(unit, ECGM_CGM_UC_SIZE_256_INTERFACE_FC_ALPHAr, &reg_db_alpha, CGM_UC_SIZE_256_INTERFACE_FC_ALPHA_7f, field_val);
            break;
    }

    DNXC_IF_ERR_EXIT(WRITE_ECGM_CGM_UC_PD_INTERFACE_FC_MIN_THr(unit, reg_pd_min));
    DNXC_IF_ERR_EXIT(WRITE_ECGM_CGM_UC_PD_INTERFACE_FC_ALPHAr(unit, reg_pd_alpha));
    DNXC_IF_ERR_EXIT(WRITE_ECGM_CGM_UC_SIZE_256_INTERFACE_FC_MIN_THr(unit, reg_db_min));
    DNXC_IF_ERR_EXIT(WRITE_ECGM_CGM_UC_SIZE_256_INTERFACE_FC_ALPHAr(unit, reg_db_alpha));

    DNXC_IF_ERR_EXIT(jer2_qax_egr_queuing_if_fc_uc_max_set(unit, core, uc_if_profile_ndx, info));

exit:
    DNXC_FUNC_RETURN;
}

int
  jer2_qax_egr_dev_fc_set(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  int                 core,
    DNX_SAND_IN  JER2_ARAD_EGR_FC_DEVICE_THRESH *thresh,
    DNX_SAND_OUT JER2_ARAD_EGR_FC_DEVICE_THRESH *exact_thresh
  )
{
  soc_reg_above_64_val_t
    data,
    field_val;

  DNXC_INIT_FUNC_DEFS;

  DNXC_NULL_CHECK(thresh);
  DNXC_NULL_CHECK(exact_thresh);

  DNXC_IF_ERR_EXIT(READ_ECGM_CONGESTION_MANAGEMENT_GLOBAL_FLOW_CONTROL_THRESHOLDSr(unit, data));

  /* Global { */
  /* Global FC DBuff */
  SOC_REG_ABOVE_64_CLEAR(field_val);
  SHR_BITCOPY_RANGE(field_val,0,&(thresh->global.buffers),0,JER2_ARAD_EGQ_THRESHOLD_DBUFF_NOF_BITS);
  soc_reg_above_64_field_set(unit, ECGM_CONGESTION_MANAGEMENT_GLOBAL_FLOW_CONTROL_THRESHOLDSr, data, TOTAL_DATA_BUFFERS_FLOW_CONTROL_LIMITf,field_val);
  exact_thresh->global.buffers = thresh->global.buffers;

  /* Global FC PD */
  SHR_BITCOPY_RANGE(field_val,0,&(thresh->global.descriptors),0,JER2_ARAD_EGQ_THRESHOLD_PD_NOF_BITS);
  soc_reg_above_64_field_set(unit, ECGM_CONGESTION_MANAGEMENT_GLOBAL_FLOW_CONTROL_THRESHOLDSr, data, TOTAL_PACKET_DESCRIPTORS_FLOW_CONTROLLIMITf,field_val);
  exact_thresh->global.descriptors = thresh->global.descriptors;

  /* Global } */

  /* UC { */
  /* UC FC DBuff */
  SOC_REG_ABOVE_64_CLEAR(field_val);
  SHR_BITCOPY_RANGE(field_val,0,&(thresh->scheduled.buffers),0,JER2_ARAD_EGQ_THRESHOLD_DBUFF_NOF_BITS);
  soc_reg_above_64_field_set(unit, ECGM_CONGESTION_MANAGEMENT_GLOBAL_FLOW_CONTROL_THRESHOLDSr, data, UNICAST_DATA_BUFFERS_FLOW_CONTROL_LIMITf, field_val);
  exact_thresh->scheduled.buffers = thresh->scheduled.buffers;

  /* UC FC PD */
  SHR_BITCOPY_RANGE(field_val,0,&(thresh->scheduled.descriptors),0,JER2_ARAD_EGQ_THRESHOLD_PD_NOF_BITS);
  soc_reg_above_64_field_set(unit, ECGM_CONGESTION_MANAGEMENT_GLOBAL_FLOW_CONTROL_THRESHOLDSr, data, UNICAST_PACKET_DESCRIPTORS_FLOW_CONTROL_LIMITf,field_val);
  exact_thresh->scheduled.descriptors = thresh->scheduled.descriptors;

  /* UC } */

  /* MC { */
  /* MC FC DBuff */
  SOC_REG_ABOVE_64_CLEAR(field_val);
  SHR_BITCOPY_RANGE(field_val,0,&(thresh->unscheduled.buffers),0,JER2_ARAD_EGQ_THRESHOLD_DBUFF_NOF_BITS);
  soc_reg_above_64_field_set(unit, ECGM_CONGESTION_MANAGEMENT_GLOBAL_FLOW_CONTROL_THRESHOLDSr, data, MULTICAST_DATA_BUFFERS_FLOW_CONTROL_LIMITf, field_val);
  exact_thresh->unscheduled.buffers = thresh->unscheduled.buffers;

  /* MC FC PD */
  SHR_BITCOPY_RANGE(field_val,0,&(thresh->unscheduled.descriptors),0,JER2_ARAD_EGQ_THRESHOLD_PD_NOF_BITS);
  soc_reg_above_64_field_set(unit, ECGM_CONGESTION_MANAGEMENT_GLOBAL_FLOW_CONTROL_THRESHOLDSr, data, MULTICAST_PACKET_DESCRIPTORS_FLOW_CONTROL_LIMITf,field_val);
  exact_thresh->unscheduled.descriptors = thresh->unscheduled.descriptors;

  /* CFG MC SP0 FC DBuff */
  SOC_REG_ABOVE_64_CLEAR(field_val);
  SHR_BITCOPY_RANGE(field_val,0,&(thresh->unscheduled_pool[0].buffers),0,JER2_ARAD_EGQ_THRESHOLD_DBUFF_NOF_BITS);
  soc_reg_above_64_field_set(unit, ECGM_CONGESTION_MANAGEMENT_GLOBAL_FLOW_CONTROL_THRESHOLDSr, data, MULTICAST_DATA_BUFFERS_SERVICE_POOL_0_FLOW_CONTROL_LIMITf, field_val);
  exact_thresh->unscheduled_pool[0].buffers = thresh->unscheduled_pool[0].buffers;

  /* CFG MC SP0 FC PD */
  SHR_BITCOPY_RANGE(field_val,0,&(thresh->unscheduled_pool[0].descriptors),0,JER2_ARAD_EGQ_THRESHOLD_PD_NOF_BITS);
  soc_reg_above_64_field_set(unit, ECGM_CONGESTION_MANAGEMENT_GLOBAL_FLOW_CONTROL_THRESHOLDSr, data, MULTICAST_PACKET_DESCRIPTORS_SERVICE_POOL_0_FLOW_CONTROL_LIMITf,field_val);
  exact_thresh->unscheduled_pool[0].descriptors = thresh->unscheduled_pool[0].descriptors;

  /* CFG MC SP1 FC DBuff */
  SOC_REG_ABOVE_64_CLEAR(field_val);
  SHR_BITCOPY_RANGE(field_val,0,&(thresh->unscheduled_pool[1].buffers),0,JER2_ARAD_EGQ_THRESHOLD_DBUFF_NOF_BITS);
  soc_reg_above_64_field_set(unit, ECGM_CONGESTION_MANAGEMENT_GLOBAL_FLOW_CONTROL_THRESHOLDSr, data, MULTICAST_DATA_BUFFERS_SERVICE_POOL_1_FLOW_CONTROL_LIMITf, field_val);
  exact_thresh->unscheduled_pool[1].buffers = thresh->unscheduled_pool[1].buffers;

  /* CFG MC SP0 FC PD */
  SHR_BITCOPY_RANGE(field_val,0,&(thresh->unscheduled_pool[1].descriptors),0,JER2_ARAD_EGQ_THRESHOLD_PD_NOF_BITS);
  soc_reg_above_64_field_set(unit, ECGM_CONGESTION_MANAGEMENT_GLOBAL_FLOW_CONTROL_THRESHOLDSr, data, MULTICAST_PACKET_DESCRIPTORS_SERVICE_POOL_1_FLOW_CONTROL_LIMITf,field_val);
  exact_thresh->unscheduled_pool[1].descriptors = thresh->unscheduled_pool[1].descriptors;

  /* MC } */

  DNXC_IF_ERR_EXIT(WRITE_ECGM_CONGESTION_MANAGEMENT_GLOBAL_FLOW_CONTROL_THRESHOLDSr(unit, data));

exit:
  DNXC_FUNC_RETURN;
}

int
  jer2_qax_egr_dev_fc_get(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  int                 core,
    DNX_SAND_OUT JER2_ARAD_EGR_FC_DEVICE_THRESH *thresh
  )
{
  soc_reg_above_64_val_t
    data,
    field_val;

  DNXC_INIT_FUNC_DEFS;

  DNXC_NULL_CHECK(thresh);

  jer2_arad_JER2_ARAD_EGR_FC_DEVICE_THRESH_clear(thresh);

  DNXC_IF_ERR_EXIT(READ_ECGM_CONGESTION_MANAGEMENT_GLOBAL_FLOW_CONTROL_THRESHOLDSr(unit, data));
  /* General { */
  /* General Dbuff */
  soc_reg_above_64_field_get(unit, ECGM_CONGESTION_MANAGEMENT_GLOBAL_FLOW_CONTROL_THRESHOLDSr, data, TOTAL_DATA_BUFFERS_FLOW_CONTROL_LIMITf, field_val);
  SHR_BITCOPY_RANGE(&(thresh->global.buffers),0,field_val,0,JER2_ARAD_EGQ_THRESHOLD_DBUFF_NOF_BITS);

  /* General PD */
  soc_reg_above_64_field_get(unit, ECGM_CONGESTION_MANAGEMENT_GLOBAL_FLOW_CONTROL_THRESHOLDSr, data, TOTAL_PACKET_DESCRIPTORS_FLOW_CONTROLLIMITf, field_val);
  SHR_BITCOPY_RANGE(&(thresh->global.descriptors),0,field_val,0,JER2_ARAD_EGQ_THRESHOLD_PD_NOF_BITS);

  /* General } */

  /* UC { */
  /* UC Dbuff */
  soc_reg_above_64_field_get(unit, ECGM_CONGESTION_MANAGEMENT_GLOBAL_FLOW_CONTROL_THRESHOLDSr, data, UNICAST_DATA_BUFFERS_FLOW_CONTROL_LIMITf, field_val);
  SHR_BITCOPY_RANGE(&(thresh->scheduled.buffers),0,field_val,0,JER2_ARAD_EGQ_THRESHOLD_DBUFF_NOF_BITS);

  /* UC PD */
  soc_reg_above_64_field_get(unit, ECGM_CONGESTION_MANAGEMENT_GLOBAL_FLOW_CONTROL_THRESHOLDSr, data, UNICAST_PACKET_DESCRIPTORS_FLOW_CONTROL_LIMITf, field_val);
  SHR_BITCOPY_RANGE(&(thresh->scheduled.descriptors),0,field_val,0,JER2_ARAD_EGQ_THRESHOLD_PD_NOF_BITS);

  /* UC } */

  /* MC { */
  /* MC FC DBuff */
  soc_reg_above_64_field_get(unit, ECGM_CONGESTION_MANAGEMENT_GLOBAL_FLOW_CONTROL_THRESHOLDSr, data, MULTICAST_DATA_BUFFERS_FLOW_CONTROL_LIMITf, field_val);
  SHR_BITCOPY_RANGE(&(thresh->unscheduled.buffers),0,field_val,0,JER2_ARAD_EGQ_THRESHOLD_DBUFF_NOF_BITS);

  /* MC FC PD */
  soc_reg_above_64_field_get(unit, ECGM_CONGESTION_MANAGEMENT_GLOBAL_FLOW_CONTROL_THRESHOLDSr, data, MULTICAST_PACKET_DESCRIPTORS_FLOW_CONTROL_LIMITf,field_val);
  SHR_BITCOPY_RANGE(&(thresh->unscheduled.descriptors),0,field_val,0,JER2_ARAD_EGQ_THRESHOLD_PD_NOF_BITS);

  /* CFG MC SP0 FC DBuff */
  soc_reg_above_64_field_get(unit, ECGM_CONGESTION_MANAGEMENT_GLOBAL_FLOW_CONTROL_THRESHOLDSr, data, MULTICAST_DATA_BUFFERS_SERVICE_POOL_0_FLOW_CONTROL_LIMITf, field_val);
  SHR_BITCOPY_RANGE(&(thresh->unscheduled_pool[0].buffers),0,field_val,0,JER2_ARAD_EGQ_THRESHOLD_DBUFF_NOF_BITS);

  /* CFG MC SP0 FC PD */
  soc_reg_above_64_field_get(unit, ECGM_CONGESTION_MANAGEMENT_GLOBAL_FLOW_CONTROL_THRESHOLDSr, data, MULTICAST_PACKET_DESCRIPTORS_SERVICE_POOL_0_FLOW_CONTROL_LIMITf,field_val);
  SHR_BITCOPY_RANGE(&(thresh->unscheduled_pool[0].descriptors),0,field_val,0,JER2_ARAD_EGQ_THRESHOLD_PD_NOF_BITS);

  /* CFG MC SP1 FC DBuff */
  soc_reg_above_64_field_get(unit, ECGM_CONGESTION_MANAGEMENT_GLOBAL_FLOW_CONTROL_THRESHOLDSr, data, MULTICAST_DATA_BUFFERS_SERVICE_POOL_1_FLOW_CONTROL_LIMITf, field_val);
  SHR_BITCOPY_RANGE(&(thresh->unscheduled_pool[1].buffers),0,field_val,0,JER2_ARAD_EGQ_THRESHOLD_DBUFF_NOF_BITS);

  /* CFG MC SP0 FC PD */
  soc_reg_above_64_field_get(unit, ECGM_CONGESTION_MANAGEMENT_GLOBAL_FLOW_CONTROL_THRESHOLDSr, data, MULTICAST_PACKET_DESCRIPTORS_SERVICE_POOL_1_FLOW_CONTROL_LIMITf,field_val);
  SHR_BITCOPY_RANGE(&(thresh->unscheduled_pool[1].descriptors),0,field_val,0,JER2_ARAD_EGQ_THRESHOLD_PD_NOF_BITS);

  /* MC } */

exit:
  DNXC_FUNC_RETURN;
}

int
  jer2_qax_egr_ofp_fc_set(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  int                 core,
    DNX_SAND_IN  uint32                 prio_ndx,
    DNX_SAND_IN  JER2_ARAD_EGR_PORT_THRESH_TYPE ofp_type_ndx,
    DNX_SAND_IN  JER2_ARAD_EGR_FC_OFP_THRESH   *thresh,
    DNX_SAND_OUT JER2_ARAD_EGR_FC_OFP_THRESH   *exact_thresh
  )
{
  uint32
    egress_tc;
  JER2_ARAD_EGR_THRESH_INFO
    thresh_info;
  soc_reg_above_64_val_t
    reg_pd,
    field_pd,
    reg_db,
    field_db,
    mem;
  uint32
    field_32;

  DNXC_INIT_FUNC_DEFS;

  DNXC_NULL_CHECK(thresh);
  DNXC_NULL_CHECK(exact_thresh);

  SOC_REG_ABOVE_64_CLEAR(reg_pd);
  SOC_REG_ABOVE_64_CLEAR(field_pd);
  SOC_REG_ABOVE_64_CLEAR(reg_db);
  SOC_REG_ABOVE_64_CLEAR(field_db);
  SOC_REG_ABOVE_64_CLEAR(mem);

  jer2_arad_JER2_ARAD_EGR_THRESH_INFO_clear(&thresh_info);

  if(prio_ndx == JER2_ARAD_EGR_Q_PRIO_ALL) {
    /* ECGM_CGM_MC_PD_TC_FC_THr */
    DNXC_IF_ERR_EXIT(READ_ECGM_CGM_MC_PD_TC_FC_THr(unit, reg_pd));
    SHR_BITCOPY_RANGE(field_pd, 0, &thresh->mc.descriptors, 0, JER2_ARAD_EGQ_THRESHOLD_PD_NOF_BITS);

    DNXC_IF_ERR_EXIT(READ_ECGM_CONGESTION_MANAGEMENT_MULTICAST_FLOW_CONTROL_PER_TC_THREHOSLDSr(unit, reg_db));
    SHR_BITCOPY_RANGE(field_db, 0, &thresh->mc.buffers, 0, JER2_ARAD_EGQ_THRESHOLD_DBUFF_NOF_BITS);

    switch(ofp_type_ndx) {
      case 0:
        soc_reg_above_64_field_set(unit, ECGM_CGM_MC_PD_TC_FC_THr, reg_pd, CGM_MC_PD_TC_FC_TH_0f, field_pd);
        soc_reg_above_64_field_set(unit, ECGM_CONGESTION_MANAGEMENT_MULTICAST_FLOW_CONTROL_PER_TC_THREHOSLDSr, reg_db, MULTICAST_DATA_BUFFERS_TC_0_FLOW_CONTROL_LIMITf, field_db);
        break;
      case 1:
        soc_reg_above_64_field_set(unit, ECGM_CGM_MC_PD_TC_FC_THr, reg_pd, CGM_MC_PD_TC_FC_TH_1f, field_pd);
        soc_reg_above_64_field_set(unit, ECGM_CONGESTION_MANAGEMENT_MULTICAST_FLOW_CONTROL_PER_TC_THREHOSLDSr, reg_db, MULTICAST_DATA_BUFFERS_TC_1_FLOW_CONTROL_LIMITf, field_db);
        break;
      case 2:
        soc_reg_above_64_field_set(unit, ECGM_CGM_MC_PD_TC_FC_THr, reg_pd, CGM_MC_PD_TC_FC_TH_2f, field_pd);
        soc_reg_above_64_field_set(unit, ECGM_CONGESTION_MANAGEMENT_MULTICAST_FLOW_CONTROL_PER_TC_THREHOSLDSr, reg_db, MULTICAST_DATA_BUFFERS_TC_2_FLOW_CONTROL_LIMITf, field_db);
        break;
      case 3:
        soc_reg_above_64_field_set(unit, ECGM_CGM_MC_PD_TC_FC_THr, reg_pd, CGM_MC_PD_TC_FC_TH_3f, field_pd);
        soc_reg_above_64_field_set(unit, ECGM_CONGESTION_MANAGEMENT_MULTICAST_FLOW_CONTROL_PER_TC_THREHOSLDSr, reg_db, MULTICAST_DATA_BUFFERS_TC_3_FLOW_CONTROL_LIMITf, field_db);
        break;
      case 4:
        soc_reg_above_64_field_set(unit, ECGM_CGM_MC_PD_TC_FC_THr, reg_pd, CGM_MC_PD_TC_FC_TH_4f, field_pd);
        soc_reg_above_64_field_set(unit, ECGM_CONGESTION_MANAGEMENT_MULTICAST_FLOW_CONTROL_PER_TC_THREHOSLDSr, reg_db, MULTICAST_DATA_BUFFERS_TC_4_FLOW_CONTROL_LIMITf, field_db);
        break;
      case 5:
        soc_reg_above_64_field_set(unit, ECGM_CGM_MC_PD_TC_FC_THr, reg_pd, CGM_MC_PD_TC_FC_TH_5f, field_pd);
        soc_reg_above_64_field_set(unit, ECGM_CONGESTION_MANAGEMENT_MULTICAST_FLOW_CONTROL_PER_TC_THREHOSLDSr, reg_db, MULTICAST_DATA_BUFFERS_TC_5_FLOW_CONTROL_LIMITf, field_db);
        break;
      case 6:
        soc_reg_above_64_field_set(unit, ECGM_CGM_MC_PD_TC_FC_THr, reg_pd, CGM_MC_PD_TC_FC_TH_6f, field_pd);
        soc_reg_above_64_field_set(unit, ECGM_CONGESTION_MANAGEMENT_MULTICAST_FLOW_CONTROL_PER_TC_THREHOSLDSr, reg_db, MULTICAST_DATA_BUFFERS_TC_6_FLOW_CONTROL_LIMITf, field_db);
        break;
      case 7:
        soc_reg_above_64_field_set(unit, ECGM_CGM_MC_PD_TC_FC_THr, reg_pd, CGM_MC_PD_TC_FC_TH_7f, field_pd);
        soc_reg_above_64_field_set(unit, ECGM_CONGESTION_MANAGEMENT_MULTICAST_FLOW_CONTROL_PER_TC_THREHOSLDSr, reg_db, MULTICAST_DATA_BUFFERS_TC_7_FLOW_CONTROL_LIMITf, field_db);
        break;
      default:
          break;
    }
    if(ofp_type_ndx < 8) {
        DNXC_IF_ERR_EXIT(WRITE_ECGM_CGM_MC_PD_TC_FC_THr(unit, reg_pd));
        DNXC_IF_ERR_EXIT(WRITE_ECGM_CONGESTION_MANAGEMENT_MULTICAST_FLOW_CONTROL_PER_TC_THREHOSLDSr(unit, reg_db));
    }

    /* EGQ_PDCT_TABLEm */
    DNXC_IF_ERR_EXIT(READ_EGQ_PDCT_TABLEm(unit, EGQ_BLOCK(unit, core), ofp_type_ndx, mem));
    field_32 = thresh->packet_descriptors;
    soc_EGQ_PDCT_TABLEm_field_set(unit, mem, PORT_UC_PD_MAX_FC_THf, &field_32);
    DNXC_IF_ERR_EXIT(WRITE_EGQ_PDCT_TABLEm(unit, EGQ_BLOCK(unit, core), ofp_type_ndx, mem));

    /* EGQ_PQST_TABLEm */
    DNXC_IF_ERR_EXIT(READ_EGQ_PQST_TABLEm(unit, EGQ_BLOCK(unit, core), ofp_type_ndx, mem));
    field_32 = thresh->data_buffers;
    soc_EGQ_PQST_TABLEm_field_set(unit, mem, PORT_UC_DB_MAX_FC_THf, &field_32);
    DNXC_IF_ERR_EXIT(WRITE_EGQ_PQST_TABLEm(unit, EGQ_BLOCK(unit, core), ofp_type_ndx, mem));
  }
  else
  {
      egress_tc = prio_ndx;

      thresh_info.dbuff = thresh->words;/* queue_words_consumed parameter used for Data buffers in Arad */
      thresh_info.packet_descriptors = thresh->packet_descriptors;

      DNXC_IF_ERR_EXIT(jer2_arad_egr_ofp_fc_q_pair_thresh_set_unsafe(
               unit,
               core,
               egress_tc,
               ofp_type_ndx,
               &thresh_info
             ));


      exact_thresh->words = thresh_info.dbuff;
      exact_thresh->packet_descriptors = thresh_info.packet_descriptors;
  }

exit:
  DNXC_FUNC_RETURN;
}

int
  jer2_qax_egr_ofp_fc_get(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  int                 core,
    DNX_SAND_IN  JER2_ARAD_EGR_Q_PRIO          prio_ndx,
    DNX_SAND_IN  JER2_ARAD_EGR_PORT_THRESH_TYPE ofp_type_ndx,
    DNX_SAND_OUT JER2_ARAD_EGR_FC_OFP_THRESH   *thresh
  )
{
  uint32
    egress_tc;
  JER2_ARAD_EGR_THRESH_INFO
    thresh_info;
  soc_reg_above_64_val_t
    reg_pd,
    field_pd,
    reg_db,
    field_db,
    mem;

  DNXC_INIT_FUNC_DEFS;

  DNXC_NULL_CHECK(thresh);

  if (prio_ndx != JER2_ARAD_EGR_Q_PRIO_ALL) {
      if (prio_ndx > JER2_ARAD_EGR_NOF_Q_PRIO-1) {
         DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("")));
      }
  }

  if (ofp_type_ndx > JER2_ARAD_EGR_PORT_NOF_THRESH_TYPES) {
      DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("")));
  }

  jer2_arad_JER2_ARAD_EGR_THRESH_INFO_clear(&thresh_info);

  if(prio_ndx == JER2_ARAD_EGR_Q_PRIO_ALL) {
    /* ECGM_CGM_MC_PD_TC_FC_THr */
    SOC_REG_ABOVE_64_CLEAR(reg_pd);
    DNXC_IF_ERR_EXIT(READ_ECGM_CGM_MC_PD_TC_FC_THr(unit, reg_pd));

    /* ECGM_CGM_MC_DB_TC_FC_THr */
    SOC_REG_ABOVE_64_CLEAR(reg_db);
    DNXC_IF_ERR_EXIT(READ_ECGM_CONGESTION_MANAGEMENT_MULTICAST_FLOW_CONTROL_PER_TC_THREHOSLDSr(unit, reg_db));

    switch(ofp_type_ndx) {
      case 0:
        soc_reg_above_64_field_get(unit, ECGM_CGM_MC_PD_TC_FC_THr, reg_pd, CGM_MC_PD_TC_FC_TH_0f, field_pd);
        soc_reg_above_64_field_get(unit, ECGM_CONGESTION_MANAGEMENT_MULTICAST_FLOW_CONTROL_PER_TC_THREHOSLDSr, reg_db, MULTICAST_DATA_BUFFERS_TC_0_FLOW_CONTROL_LIMITf, field_db);
        break;
      case 1:
        soc_reg_above_64_field_get(unit, ECGM_CGM_MC_PD_TC_FC_THr, reg_pd, CGM_MC_PD_TC_FC_TH_1f, field_pd);
        soc_reg_above_64_field_get(unit, ECGM_CONGESTION_MANAGEMENT_MULTICAST_FLOW_CONTROL_PER_TC_THREHOSLDSr, reg_db, MULTICAST_DATA_BUFFERS_TC_1_FLOW_CONTROL_LIMITf, field_db);
        break;
      case 2:
        soc_reg_above_64_field_get(unit, ECGM_CGM_MC_PD_TC_FC_THr, reg_pd, CGM_MC_PD_TC_FC_TH_2f, field_pd);
        soc_reg_above_64_field_get(unit, ECGM_CONGESTION_MANAGEMENT_MULTICAST_FLOW_CONTROL_PER_TC_THREHOSLDSr, reg_db, MULTICAST_DATA_BUFFERS_TC_2_FLOW_CONTROL_LIMITf, field_db);
        break;
      case 3:
        soc_reg_above_64_field_get(unit, ECGM_CGM_MC_PD_TC_FC_THr, reg_pd, CGM_MC_PD_TC_FC_TH_3f, field_pd);
        soc_reg_above_64_field_get(unit, ECGM_CONGESTION_MANAGEMENT_MULTICAST_FLOW_CONTROL_PER_TC_THREHOSLDSr, reg_db, MULTICAST_DATA_BUFFERS_TC_3_FLOW_CONTROL_LIMITf, field_db);
        break;
      case 4:
        soc_reg_above_64_field_get(unit, ECGM_CGM_MC_PD_TC_FC_THr, reg_pd, CGM_MC_PD_TC_FC_TH_4f, field_pd);
        soc_reg_above_64_field_get(unit, ECGM_CONGESTION_MANAGEMENT_MULTICAST_FLOW_CONTROL_PER_TC_THREHOSLDSr, reg_db, MULTICAST_DATA_BUFFERS_TC_4_FLOW_CONTROL_LIMITf, field_db);
        break;
      case 5:
        soc_reg_above_64_field_get(unit, ECGM_CGM_MC_PD_TC_FC_THr, reg_pd, CGM_MC_PD_TC_FC_TH_5f, field_pd);
        soc_reg_above_64_field_get(unit, ECGM_CONGESTION_MANAGEMENT_MULTICAST_FLOW_CONTROL_PER_TC_THREHOSLDSr, reg_db, MULTICAST_DATA_BUFFERS_TC_5_FLOW_CONTROL_LIMITf, field_db);
        break;
      case 6:
        soc_reg_above_64_field_get(unit, ECGM_CGM_MC_PD_TC_FC_THr, reg_pd, CGM_MC_PD_TC_FC_TH_6f, field_pd);
        soc_reg_above_64_field_get(unit, ECGM_CONGESTION_MANAGEMENT_MULTICAST_FLOW_CONTROL_PER_TC_THREHOSLDSr, reg_db, MULTICAST_DATA_BUFFERS_TC_6_FLOW_CONTROL_LIMITf, field_db);
        break;
      case 7:
        soc_reg_above_64_field_get(unit, ECGM_CGM_MC_PD_TC_FC_THr, reg_pd, CGM_MC_PD_TC_FC_TH_7f, field_pd);
        soc_reg_above_64_field_get(unit, ECGM_CONGESTION_MANAGEMENT_MULTICAST_FLOW_CONTROL_PER_TC_THREHOSLDSr, reg_db, MULTICAST_DATA_BUFFERS_TC_7_FLOW_CONTROL_LIMITf, field_db);
        break;
      default:
          break;
    }
    if(ofp_type_ndx < 7) {
/*
 * COVERITY
 *
 * The variable field_pd is always initiallized when ofp_type_ndx < 7.
 */
/* coverity[uninit_use] */
      SHR_BITCOPY_RANGE(&thresh->mc.descriptors, 0, field_pd, 0, JER2_ARAD_EGQ_THRESHOLD_PD_NOF_BITS);
      SHR_BITCOPY_RANGE(&thresh->mc.buffers, 0, field_db, 0, JER2_ARAD_EGQ_THRESHOLD_DBUFF_NOF_BITS);
    }

    /* EGQ_PDCT_TABLEm */
    DNXC_IF_ERR_EXIT(READ_EGQ_PDCT_TABLEm(unit, EGQ_BLOCK(unit, core), ofp_type_ndx, mem));
    soc_EGQ_PDCT_TABLEm_field_get(unit, mem, PORT_UC_PD_MAX_FC_THf, &thresh->packet_descriptors);

    /* EGQ_PQST_TABLEm */
    DNXC_IF_ERR_EXIT(READ_EGQ_PQST_TABLEm(unit, EGQ_BLOCK(unit, core), ofp_type_ndx, mem));
    soc_EGQ_PQST_TABLEm_field_get(unit, mem, PORT_UC_DB_MAX_FC_THf, &thresh->data_buffers);
  }
  else
  {
      egress_tc = prio_ndx;
      DNXC_IF_ERR_EXIT(jer2_qax_egr_ofp_fc_q_pair_thresh_get(
               unit,
               core,
               egress_tc,
               ofp_type_ndx,
               &thresh_info
             ));

      thresh->words = thresh_info.dbuff;/* queue_words_consumed parameter used for Data buffers in Arad */
      thresh->packet_descriptors = thresh_info.packet_descriptors;
  }
exit:
  DNXC_FUNC_RETURN;
}

int
  jer2_qax_egr_queuing_global_drop_set(
      DNX_SAND_IN  int   unit,
      DNX_SAND_IN  int   core,
      DNX_SAND_IN  soc_dnx_cosq_threshold_type_t threshold_type,
      DNX_SAND_IN  int    threshold_value,
      DNX_SAND_IN  soc_dnx_cosq_threshold_global_type_t drop_type
  )
{
    soc_reg_above_64_val_t reg;

    DNXC_INIT_FUNC_DEFS;

    if(threshold_type == soc_dnx_cosq_threshold_packet_descriptors || threshold_type == soc_dnx_cosq_threshold_available_packet_descriptors)
    {
        DNXC_IF_ERR_EXIT(READ_ECGM_CONGESTION_MANAGEMENT_GLOBAL_PD_THRESHOLDSr(unit, reg));
        switch (drop_type)
        {
            case soc_dnx_cosq_threshold_global_type_unicast:
                soc_reg_above_64_field32_set(unit, ECGM_CONGESTION_MANAGEMENT_GLOBAL_PD_THRESHOLDSr, reg, MAXIMUM_UNICAST_PACKET_DESCRIPTORSf, threshold_value);
                break;
            case soc_dnx_cosq_threshold_global_type_multicast:
                soc_reg_above_64_field32_set(unit, ECGM_CONGESTION_MANAGEMENT_GLOBAL_PD_THRESHOLDSr, reg, MAXIMUM_MULTICAST_PACKET_DESCRIPTORSf, threshold_value);
                break;
            case soc_dnx_cosq_threshold_global_type_service_pool_0:
                soc_reg_above_64_field32_set(unit, ECGM_CONGESTION_MANAGEMENT_GLOBAL_PD_THRESHOLDSr, reg, SERVICE_POOL_0_MAXIMUM_PACKET_DESCRIPTORSf, threshold_value);
                break;
            case soc_dnx_cosq_threshold_global_type_service_pool_1:
                soc_reg_above_64_field32_set(unit, ECGM_CONGESTION_MANAGEMENT_GLOBAL_PD_THRESHOLDSr, reg, SERVICE_POOL_1_MAXIMUM_PACKET_DESCRIPTORSf, threshold_value);
                break;
            case soc_dnx_cosq_threshold_global_type_total:
                soc_reg_above_64_field32_set(unit, ECGM_CONGESTION_MANAGEMENT_GLOBAL_PD_THRESHOLDSr, reg, MAXIMUM_TOTAL_PACKET_DESCRIPTORSf, threshold_value);
                break;
            default:
                break;
        }

        DNXC_IF_ERR_EXIT(WRITE_ECGM_CONGESTION_MANAGEMENT_GLOBAL_PD_THRESHOLDSr(unit, reg));
    }
    else if(threshold_type == soc_dnx_cosq_threshold_data_buffers || threshold_type == soc_dnx_cosq_threshold_available_data_buffers)
    {

        DNXC_IF_ERR_EXIT(READ_ECGM_CONGESTION_MANAGEMENT_GLOBAL_DB_THRESHOLDSr(unit, reg));
        switch (drop_type)
        {
            case soc_dnx_cosq_threshold_global_type_unicast:
                soc_reg_above_64_field32_set(unit, ECGM_CONGESTION_MANAGEMENT_GLOBAL_DB_THRESHOLDSr, reg, MAXIMUM_UNICAST_DATA_BUFFERSf, threshold_value);
                break;
            case soc_dnx_cosq_threshold_global_type_multicast:
                soc_reg_above_64_field32_set(unit, ECGM_CONGESTION_MANAGEMENT_GLOBAL_DB_THRESHOLDSr, reg, MAXIMUM_MULTICAST_DATA_BUFFERSf, threshold_value);
                break;
            case soc_dnx_cosq_threshold_global_type_service_pool_0:
                soc_reg_above_64_field32_set(unit, ECGM_CONGESTION_MANAGEMENT_GLOBAL_DB_THRESHOLDSr, reg, SERVICE_POOL_0_MAXIMUM_DATA_BUFFERSf, threshold_value);
                break;
            case soc_dnx_cosq_threshold_global_type_service_pool_1:
                soc_reg_above_64_field32_set(unit, ECGM_CONGESTION_MANAGEMENT_GLOBAL_DB_THRESHOLDSr, reg, SERVICE_POOL_1_MAXIMUM_DATA_BUFFERSf, threshold_value);
                break;
            case soc_dnx_cosq_threshold_global_type_total:
                soc_reg_above_64_field32_set(unit, ECGM_CONGESTION_MANAGEMENT_GLOBAL_DB_THRESHOLDSr, reg, MAXIMUM_TOTAL_DATA_BUFFERSf, threshold_value);
                break;
            default:
                break;
        }
        DNXC_IF_ERR_EXIT(WRITE_ECGM_CONGESTION_MANAGEMENT_GLOBAL_DB_THRESHOLDSr(unit, reg));
    }
    else
    {
        DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("")));
    }


exit:
  DNXC_FUNC_RETURN;
}

int
  jer2_qax_egr_queuing_global_drop_get(
      DNX_SAND_IN  int   unit,
      DNX_SAND_IN  int   core,
      DNX_SAND_IN  soc_dnx_cosq_threshold_type_t threshold_type,
      DNX_SAND_OUT int*    threshold_value,
      DNX_SAND_IN  soc_dnx_cosq_threshold_global_type_t drop_type
  )
{
    soc_reg_above_64_val_t reg;

    DNXC_INIT_FUNC_DEFS;

    if(threshold_type == soc_dnx_cosq_threshold_packet_descriptors || threshold_type == soc_dnx_cosq_threshold_available_packet_descriptors)
    {
        DNXC_IF_ERR_EXIT(READ_ECGM_CONGESTION_MANAGEMENT_GLOBAL_PD_THRESHOLDSr(unit, reg));
        switch (drop_type)
        {
            case soc_dnx_cosq_threshold_global_type_unicast:
                *threshold_value = soc_reg_above_64_field32_get(unit, ECGM_CONGESTION_MANAGEMENT_GLOBAL_PD_THRESHOLDSr, reg, MAXIMUM_UNICAST_PACKET_DESCRIPTORSf);
                break;
            case soc_dnx_cosq_threshold_global_type_multicast:
                *threshold_value = soc_reg_above_64_field32_get(unit, ECGM_CONGESTION_MANAGEMENT_GLOBAL_PD_THRESHOLDSr, reg, MAXIMUM_MULTICAST_PACKET_DESCRIPTORSf);
                break;
            case soc_dnx_cosq_threshold_global_type_service_pool_0:
                *threshold_value = soc_reg_above_64_field32_get(unit, ECGM_CONGESTION_MANAGEMENT_GLOBAL_PD_THRESHOLDSr, reg, SERVICE_POOL_0_MAXIMUM_PACKET_DESCRIPTORSf);
                break;
            case soc_dnx_cosq_threshold_global_type_service_pool_1:
                *threshold_value = soc_reg_above_64_field32_get(unit, ECGM_CONGESTION_MANAGEMENT_GLOBAL_PD_THRESHOLDSr, reg, SERVICE_POOL_1_MAXIMUM_PACKET_DESCRIPTORSf);
                break;
            case soc_dnx_cosq_threshold_global_type_total:
                *threshold_value = soc_reg_above_64_field32_get(unit, ECGM_CONGESTION_MANAGEMENT_GLOBAL_PD_THRESHOLDSr, reg, MAXIMUM_TOTAL_PACKET_DESCRIPTORSf);
                break;
            default:
                break;
        }
    }
    else if(threshold_type == soc_dnx_cosq_threshold_data_buffers || threshold_type == soc_dnx_cosq_threshold_available_data_buffers)
    {
        DNXC_IF_ERR_EXIT(READ_ECGM_CONGESTION_MANAGEMENT_GLOBAL_DB_THRESHOLDSr(unit, reg));
        switch (drop_type)
        {
            case soc_dnx_cosq_threshold_global_type_unicast:
                *threshold_value = soc_reg_above_64_field32_get(unit, ECGM_CONGESTION_MANAGEMENT_GLOBAL_DB_THRESHOLDSr, reg, MAXIMUM_UNICAST_DATA_BUFFERSf);
                break;
            case soc_dnx_cosq_threshold_global_type_multicast:
                *threshold_value = soc_reg_above_64_field32_get(unit, ECGM_CONGESTION_MANAGEMENT_GLOBAL_DB_THRESHOLDSr, reg, MAXIMUM_MULTICAST_DATA_BUFFERSf);
                break;
            case soc_dnx_cosq_threshold_global_type_service_pool_0:
                *threshold_value = soc_reg_above_64_field32_get(unit, ECGM_CONGESTION_MANAGEMENT_GLOBAL_DB_THRESHOLDSr, reg, SERVICE_POOL_0_MAXIMUM_DATA_BUFFERSf);
                break;
            case soc_dnx_cosq_threshold_global_type_service_pool_1:
                *threshold_value = soc_reg_above_64_field32_get(unit, ECGM_CONGESTION_MANAGEMENT_GLOBAL_DB_THRESHOLDSr, reg, SERVICE_POOL_1_MAXIMUM_DATA_BUFFERSf);
                break;
            case soc_dnx_cosq_threshold_global_type_total:
                *threshold_value = soc_reg_above_64_field32_get(unit, ECGM_CONGESTION_MANAGEMENT_GLOBAL_DB_THRESHOLDSr, reg, MAXIMUM_TOTAL_DATA_BUFFERSf);
                break;
            default:
                break;
        }
    }
    else
    {
        DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("")));
    }


exit:
    DNXC_FUNC_RETURN;

}

int
  jer2_qax_egr_queuing_sp_tc_drop_set(
    DNX_SAND_IN    int    unit,
    DNX_SAND_IN    int    core,
    DNX_SAND_IN    int    tc,
    DNX_SAND_IN    soc_dnx_cosq_threshold_type_t threshold_type,
    DNX_SAND_IN    int    threshold_value,
    DNX_SAND_IN    soc_dnx_cosq_threshold_global_type_t drop_type
  )
{
    soc_reg_above_64_val_t reg;

    DNXC_INIT_FUNC_DEFS;

    if (threshold_type == soc_dnx_cosq_threshold_data_buffers || threshold_type == soc_dnx_cosq_threshold_available_data_buffers)
    {
        DNXC_IF_ERR_EXIT(READ_ECGM_CONGESTION_MANAGEMENT_DB_SERVICE_POOL_MAXIMUM_THRESHOLDSr(unit, reg));
        if (drop_type == soc_dnx_cosq_threshold_global_type_service_pool_0)
        {
            switch (tc)
            {
                case 0:
                    soc_reg_above_64_field32_set(unit, ECGM_CONGESTION_MANAGEMENT_DB_SERVICE_POOL_MAXIMUM_THRESHOLDSr, reg, DB_SERVICE_POOL_0_TC_0_MAXIMUMLIMITf, threshold_value);
                    break;
                case 1:
                    soc_reg_above_64_field32_set(unit, ECGM_CONGESTION_MANAGEMENT_DB_SERVICE_POOL_MAXIMUM_THRESHOLDSr, reg, DB_SERVICE_POOL_0_TC_1_MAXIMUMLIMITf, threshold_value);
                    break;
                case 2:
                    soc_reg_above_64_field32_set(unit, ECGM_CONGESTION_MANAGEMENT_DB_SERVICE_POOL_MAXIMUM_THRESHOLDSr, reg, DB_SERVICE_POOL_0_TC_2_MAXIMUMLIMITf, threshold_value);
                    break;
                case 3:
                    soc_reg_above_64_field32_set(unit, ECGM_CONGESTION_MANAGEMENT_DB_SERVICE_POOL_MAXIMUM_THRESHOLDSr, reg, DB_SERVICE_POOL_0_TC_3_MAXIMUMLIMITf, threshold_value);
                    break;
                case 4:
                    soc_reg_above_64_field32_set(unit, ECGM_CONGESTION_MANAGEMENT_DB_SERVICE_POOL_MAXIMUM_THRESHOLDSr, reg, DB_SERVICE_POOL_0_TC_4_MAXIMUMLIMITf, threshold_value);
                    break;
                case 5:
                    soc_reg_above_64_field32_set(unit, ECGM_CONGESTION_MANAGEMENT_DB_SERVICE_POOL_MAXIMUM_THRESHOLDSr, reg, DB_SERVICE_POOL_0_TC_5_MAXIMUMLIMITf, threshold_value);
                    break;
                case 6:
                    soc_reg_above_64_field32_set(unit, ECGM_CONGESTION_MANAGEMENT_DB_SERVICE_POOL_MAXIMUM_THRESHOLDSr, reg, DB_SERVICE_POOL_0_TC_6_MAXIMUMLIMITf, threshold_value);
                    break;
                case 7:
                    soc_reg_above_64_field32_set(unit, ECGM_CONGESTION_MANAGEMENT_DB_SERVICE_POOL_MAXIMUM_THRESHOLDSr, reg, DB_SERVICE_POOL_0_TC_7_MAXIMUMLIMITf, threshold_value);
                    break;
            }
        }
        else /* service pool 1 */
        {
            switch (tc)
            {
                case 0:
                    soc_reg_above_64_field32_set(unit, ECGM_CONGESTION_MANAGEMENT_DB_SERVICE_POOL_MAXIMUM_THRESHOLDSr, reg, DB_SERVICE_POOL_1_TC_0_MAXIMUMLIMITf, threshold_value);
                    break;
                case 1:
                    soc_reg_above_64_field32_set(unit, ECGM_CONGESTION_MANAGEMENT_DB_SERVICE_POOL_MAXIMUM_THRESHOLDSr, reg, DB_SERVICE_POOL_1_TC_1_MAXIMUMLIMITf, threshold_value);
                    break;
                case 2:
                    soc_reg_above_64_field32_set(unit, ECGM_CONGESTION_MANAGEMENT_DB_SERVICE_POOL_MAXIMUM_THRESHOLDSr, reg, DB_SERVICE_POOL_1_TC_2_MAXIMUMLIMITf, threshold_value);
                    break;
                case 3:
                    soc_reg_above_64_field32_set(unit, ECGM_CONGESTION_MANAGEMENT_DB_SERVICE_POOL_MAXIMUM_THRESHOLDSr, reg, DB_SERVICE_POOL_1_TC_3_MAXIMUMLIMITf, threshold_value);
                    break;
                case 4:
                    soc_reg_above_64_field32_set(unit, ECGM_CONGESTION_MANAGEMENT_DB_SERVICE_POOL_MAXIMUM_THRESHOLDSr, reg, DB_SERVICE_POOL_1_TC_4_MAXIMUMLIMITf, threshold_value);
                    break;
                case 5:
                    soc_reg_above_64_field32_set(unit, ECGM_CONGESTION_MANAGEMENT_DB_SERVICE_POOL_MAXIMUM_THRESHOLDSr, reg, DB_SERVICE_POOL_1_TC_5_MAXIMUMLIMITf, threshold_value);
                    break;
                case 6:
                    soc_reg_above_64_field32_set(unit, ECGM_CONGESTION_MANAGEMENT_DB_SERVICE_POOL_MAXIMUM_THRESHOLDSr, reg, DB_SERVICE_POOL_1_TC_6_MAXIMUMLIMITf, threshold_value);
                    break;
                case 7:
                    soc_reg_above_64_field32_set(unit, ECGM_CONGESTION_MANAGEMENT_DB_SERVICE_POOL_MAXIMUM_THRESHOLDSr, reg, DB_SERVICE_POOL_1_TC_7_MAXIMUMLIMITf, threshold_value);
                    break;
            }
        }
        DNXC_IF_ERR_EXIT(WRITE_ECGM_CONGESTION_MANAGEMENT_DB_SERVICE_POOL_MAXIMUM_THRESHOLDSr(unit, reg));
    }
    else if (threshold_type == soc_dnx_cosq_threshold_packet_descriptors || soc_dnx_cosq_threshold_available_packet_descriptors)
    {
        DNXC_IF_ERR_EXIT(READ_ECGM_CONGESTION_MANAGEMENT_PD_SERVICE_POOL_THRESHOLDSr(unit, reg));

        if (drop_type == soc_dnx_cosq_threshold_global_type_service_pool_0)
        {
            switch (tc)
            {
                case 0:
                    soc_reg_above_64_field32_set(unit, ECGM_CONGESTION_MANAGEMENT_PD_SERVICE_POOL_THRESHOLDSr, reg, PD_SERVICE_POOL_0_TC_0_MAXIMUMLIMITf, threshold_value);
                    break;
                case 1:
                    soc_reg_above_64_field32_set(unit, ECGM_CONGESTION_MANAGEMENT_PD_SERVICE_POOL_THRESHOLDSr, reg, PD_SERVICE_POOL_0_TC_1_MAXIMUMLIMITf, threshold_value);
                    break;
                case 2:
                    soc_reg_above_64_field32_set(unit, ECGM_CONGESTION_MANAGEMENT_PD_SERVICE_POOL_THRESHOLDSr, reg, PD_SERVICE_POOL_0_TC_2_MAXIMUMLIMITf, threshold_value);
                    break;
                case 3:
                    soc_reg_above_64_field32_set(unit, ECGM_CONGESTION_MANAGEMENT_PD_SERVICE_POOL_THRESHOLDSr, reg, PD_SERVICE_POOL_0_TC_3_MAXIMUMLIMITf, threshold_value);
                    break;
                case 4:
                    soc_reg_above_64_field32_set(unit, ECGM_CONGESTION_MANAGEMENT_PD_SERVICE_POOL_THRESHOLDSr, reg, PD_SERVICE_POOL_0_TC_4_MAXIMUMLIMITf, threshold_value);
                    break;
                case 5:
                    soc_reg_above_64_field32_set(unit, ECGM_CONGESTION_MANAGEMENT_PD_SERVICE_POOL_THRESHOLDSr, reg, PD_SERVICE_POOL_0_TC_5_MAXIMUMLIMITf, threshold_value);
                    break;
                case 6:
                    soc_reg_above_64_field32_set(unit, ECGM_CONGESTION_MANAGEMENT_PD_SERVICE_POOL_THRESHOLDSr, reg, PD_SERVICE_POOL_0_TC_6_MAXIMUMLIMITf, threshold_value);
                    break;
                case 7:
                    soc_reg_above_64_field32_set(unit, ECGM_CONGESTION_MANAGEMENT_PD_SERVICE_POOL_THRESHOLDSr, reg, PD_SERVICE_POOL_0_TC_7_MAXIMUMLIMITf, threshold_value);
                    break;
            }
        }
        else /* service pool 1 */
        {
            switch (tc)
            {
                case 0:
                    soc_reg_above_64_field32_set(unit, ECGM_CONGESTION_MANAGEMENT_PD_SERVICE_POOL_THRESHOLDSr, reg, PD_SERVICE_POOL_1_TC_0_MAXIMUMLIMITf, threshold_value);
                    break;
                case 1:
                    soc_reg_above_64_field32_set(unit, ECGM_CONGESTION_MANAGEMENT_PD_SERVICE_POOL_THRESHOLDSr, reg, PD_SERVICE_POOL_1_TC_1_MAXIMUMLIMITf, threshold_value);
                    break;
                case 2:
                    soc_reg_above_64_field32_set(unit, ECGM_CONGESTION_MANAGEMENT_PD_SERVICE_POOL_THRESHOLDSr, reg, PD_SERVICE_POOL_1_TC_2_MAXIMUMLIMITf, threshold_value);
                    break;
                case 3:
                    soc_reg_above_64_field32_set(unit, ECGM_CONGESTION_MANAGEMENT_PD_SERVICE_POOL_THRESHOLDSr, reg, PD_SERVICE_POOL_1_TC_3_MAXIMUMLIMITf, threshold_value);
                    break;
                case 4:
                    soc_reg_above_64_field32_set(unit, ECGM_CONGESTION_MANAGEMENT_PD_SERVICE_POOL_THRESHOLDSr, reg, PD_SERVICE_POOL_1_TC_4_MAXIMUMLIMITf, threshold_value);
                    break;
                case 5:
                    soc_reg_above_64_field32_set(unit, ECGM_CONGESTION_MANAGEMENT_PD_SERVICE_POOL_THRESHOLDSr, reg, PD_SERVICE_POOL_1_TC_5_MAXIMUMLIMITf, threshold_value);
                    break;
                case 6:
                    soc_reg_above_64_field32_set(unit, ECGM_CONGESTION_MANAGEMENT_PD_SERVICE_POOL_THRESHOLDSr, reg, PD_SERVICE_POOL_1_TC_6_MAXIMUMLIMITf, threshold_value);
                    break;
                case 7:
                    soc_reg_above_64_field32_set(unit, ECGM_CONGESTION_MANAGEMENT_PD_SERVICE_POOL_THRESHOLDSr, reg, PD_SERVICE_POOL_1_TC_7_MAXIMUMLIMITf, threshold_value);
                    break;
            }
        }

        DNXC_IF_ERR_EXIT(WRITE_ECGM_CONGESTION_MANAGEMENT_PD_SERVICE_POOL_THRESHOLDSr(unit, reg));
    }



exit:
  DNXC_FUNC_RETURN;

}

int
  jer2_qax_egr_queuing_sp_tc_drop_get(
    DNX_SAND_IN    int    unit,
    DNX_SAND_IN    int    core,
    DNX_SAND_IN    int    tc,
    DNX_SAND_IN    soc_dnx_cosq_threshold_type_t threshold_type,
    DNX_SAND_OUT   int*   threshold_value,
    DNX_SAND_IN    soc_dnx_cosq_threshold_global_type_t drop_type
  )
{
    soc_reg_above_64_val_t reg;

    DNXC_INIT_FUNC_DEFS;

    if (threshold_type == soc_dnx_cosq_threshold_data_buffers || threshold_type == soc_dnx_cosq_threshold_available_data_buffers)
    {
        DNXC_IF_ERR_EXIT(READ_ECGM_CONGESTION_MANAGEMENT_DB_SERVICE_POOL_MAXIMUM_THRESHOLDSr(unit, reg));
        if (drop_type == soc_dnx_cosq_threshold_global_type_service_pool_0)
        {
            switch (tc)
            {
                case 0:
                    *threshold_value = soc_reg_above_64_field32_get(unit, ECGM_CONGESTION_MANAGEMENT_DB_SERVICE_POOL_MAXIMUM_THRESHOLDSr, reg, DB_SERVICE_POOL_0_TC_0_MAXIMUMLIMITf);
                    break;
                case 1:
                    *threshold_value = soc_reg_above_64_field32_get(unit, ECGM_CONGESTION_MANAGEMENT_DB_SERVICE_POOL_MAXIMUM_THRESHOLDSr, reg, DB_SERVICE_POOL_0_TC_1_MAXIMUMLIMITf);
                    break;
                case 2:
                    *threshold_value = soc_reg_above_64_field32_get(unit, ECGM_CONGESTION_MANAGEMENT_DB_SERVICE_POOL_MAXIMUM_THRESHOLDSr, reg, DB_SERVICE_POOL_0_TC_2_MAXIMUMLIMITf);
                    break;
                case 3:
                    *threshold_value = soc_reg_above_64_field32_get(unit, ECGM_CONGESTION_MANAGEMENT_DB_SERVICE_POOL_MAXIMUM_THRESHOLDSr, reg, DB_SERVICE_POOL_0_TC_3_MAXIMUMLIMITf);
                    break;
                case 4:
                    *threshold_value = soc_reg_above_64_field32_get(unit, ECGM_CONGESTION_MANAGEMENT_DB_SERVICE_POOL_MAXIMUM_THRESHOLDSr, reg, DB_SERVICE_POOL_0_TC_4_MAXIMUMLIMITf);
                    break;
                case 5:
                    *threshold_value = soc_reg_above_64_field32_get(unit, ECGM_CONGESTION_MANAGEMENT_DB_SERVICE_POOL_MAXIMUM_THRESHOLDSr, reg, DB_SERVICE_POOL_0_TC_5_MAXIMUMLIMITf);
                    break;
                case 6:
                    *threshold_value = soc_reg_above_64_field32_get(unit, ECGM_CONGESTION_MANAGEMENT_DB_SERVICE_POOL_MAXIMUM_THRESHOLDSr, reg, DB_SERVICE_POOL_0_TC_6_MAXIMUMLIMITf);
                    break;
                case 7:
                    *threshold_value = soc_reg_above_64_field32_get(unit, ECGM_CONGESTION_MANAGEMENT_DB_SERVICE_POOL_MAXIMUM_THRESHOLDSr, reg, DB_SERVICE_POOL_0_TC_7_MAXIMUMLIMITf);
                    break;
            }
        }
        else /* service pool 1 */
        {
            switch (tc)
            {
                case 0:
                    *threshold_value = soc_reg_above_64_field32_get(unit, ECGM_CONGESTION_MANAGEMENT_DB_SERVICE_POOL_MAXIMUM_THRESHOLDSr, reg, DB_SERVICE_POOL_1_TC_0_MAXIMUMLIMITf);
                    break;
                case 1:
                    *threshold_value = soc_reg_above_64_field32_get(unit, ECGM_CONGESTION_MANAGEMENT_DB_SERVICE_POOL_MAXIMUM_THRESHOLDSr, reg, DB_SERVICE_POOL_1_TC_1_MAXIMUMLIMITf);
                    break;
                case 2:
                    *threshold_value = soc_reg_above_64_field32_get(unit, ECGM_CONGESTION_MANAGEMENT_DB_SERVICE_POOL_MAXIMUM_THRESHOLDSr, reg, DB_SERVICE_POOL_1_TC_2_MAXIMUMLIMITf);
                    break;
                case 3:
                    *threshold_value = soc_reg_above_64_field32_get(unit, ECGM_CONGESTION_MANAGEMENT_DB_SERVICE_POOL_MAXIMUM_THRESHOLDSr, reg, DB_SERVICE_POOL_1_TC_3_MAXIMUMLIMITf);
                    break;
                case 4:
                    *threshold_value = soc_reg_above_64_field32_get(unit, ECGM_CONGESTION_MANAGEMENT_DB_SERVICE_POOL_MAXIMUM_THRESHOLDSr, reg, DB_SERVICE_POOL_1_TC_4_MAXIMUMLIMITf);
                    break;
                case 5:
                    *threshold_value = soc_reg_above_64_field32_get(unit, ECGM_CONGESTION_MANAGEMENT_DB_SERVICE_POOL_MAXIMUM_THRESHOLDSr, reg, DB_SERVICE_POOL_1_TC_5_MAXIMUMLIMITf);
                    break;
                case 6:
                    *threshold_value = soc_reg_above_64_field32_get(unit, ECGM_CONGESTION_MANAGEMENT_DB_SERVICE_POOL_MAXIMUM_THRESHOLDSr, reg, DB_SERVICE_POOL_1_TC_6_MAXIMUMLIMITf);
                    break;
                case 7:
                    *threshold_value = soc_reg_above_64_field32_get(unit, ECGM_CONGESTION_MANAGEMENT_DB_SERVICE_POOL_MAXIMUM_THRESHOLDSr, reg, DB_SERVICE_POOL_1_TC_7_MAXIMUMLIMITf);
                    break;
            }
        }
    }
    else if (threshold_type == soc_dnx_cosq_threshold_packet_descriptors || soc_dnx_cosq_threshold_available_packet_descriptors)
    {
        DNXC_IF_ERR_EXIT(READ_ECGM_CONGESTION_MANAGEMENT_PD_SERVICE_POOL_THRESHOLDSr(unit, reg));

        if (drop_type == soc_dnx_cosq_threshold_global_type_service_pool_0)
        {
            switch (tc)
            {
                case 0:
                    *threshold_value = soc_reg_above_64_field32_get(unit, ECGM_CONGESTION_MANAGEMENT_PD_SERVICE_POOL_THRESHOLDSr, reg, PD_SERVICE_POOL_0_TC_0_MAXIMUMLIMITf);
                    break;
                case 1:
                    *threshold_value = soc_reg_above_64_field32_get(unit, ECGM_CONGESTION_MANAGEMENT_PD_SERVICE_POOL_THRESHOLDSr, reg, PD_SERVICE_POOL_0_TC_1_MAXIMUMLIMITf);
                    break;
                case 2:
                    *threshold_value = soc_reg_above_64_field32_get(unit, ECGM_CONGESTION_MANAGEMENT_PD_SERVICE_POOL_THRESHOLDSr, reg, PD_SERVICE_POOL_0_TC_2_MAXIMUMLIMITf);
                    break;
                case 3:
                    *threshold_value = soc_reg_above_64_field32_get(unit, ECGM_CONGESTION_MANAGEMENT_PD_SERVICE_POOL_THRESHOLDSr, reg, PD_SERVICE_POOL_0_TC_3_MAXIMUMLIMITf);
                    break;
                case 4:
                    *threshold_value = soc_reg_above_64_field32_get(unit, ECGM_CONGESTION_MANAGEMENT_PD_SERVICE_POOL_THRESHOLDSr, reg, PD_SERVICE_POOL_0_TC_4_MAXIMUMLIMITf);
                    break;
                case 5:
                    *threshold_value = soc_reg_above_64_field32_get(unit, ECGM_CONGESTION_MANAGEMENT_PD_SERVICE_POOL_THRESHOLDSr, reg, PD_SERVICE_POOL_0_TC_5_MAXIMUMLIMITf);
                    break;
                case 6:
                    *threshold_value = soc_reg_above_64_field32_get(unit, ECGM_CONGESTION_MANAGEMENT_PD_SERVICE_POOL_THRESHOLDSr, reg, PD_SERVICE_POOL_0_TC_6_MAXIMUMLIMITf);
                    break;
                case 7:
                    *threshold_value = soc_reg_above_64_field32_get(unit, ECGM_CONGESTION_MANAGEMENT_PD_SERVICE_POOL_THRESHOLDSr, reg, PD_SERVICE_POOL_0_TC_7_MAXIMUMLIMITf);
                    break;
            }
        }
        else /* service pool 1 */
        {
            switch (tc)
            {
                case 0:
                    *threshold_value = soc_reg_above_64_field32_get(unit, ECGM_CONGESTION_MANAGEMENT_PD_SERVICE_POOL_THRESHOLDSr, reg, PD_SERVICE_POOL_1_TC_0_MAXIMUMLIMITf);
                    break;
                case 1:
                    *threshold_value = soc_reg_above_64_field32_get(unit, ECGM_CONGESTION_MANAGEMENT_PD_SERVICE_POOL_THRESHOLDSr, reg, PD_SERVICE_POOL_1_TC_1_MAXIMUMLIMITf);
                    break;
                case 2:
                    *threshold_value = soc_reg_above_64_field32_get(unit, ECGM_CONGESTION_MANAGEMENT_PD_SERVICE_POOL_THRESHOLDSr, reg, PD_SERVICE_POOL_1_TC_2_MAXIMUMLIMITf);
                    break;
                case 3:
                    *threshold_value = soc_reg_above_64_field32_get(unit, ECGM_CONGESTION_MANAGEMENT_PD_SERVICE_POOL_THRESHOLDSr, reg, PD_SERVICE_POOL_1_TC_3_MAXIMUMLIMITf);
                    break;
                case 4:
                    *threshold_value = soc_reg_above_64_field32_get(unit, ECGM_CONGESTION_MANAGEMENT_PD_SERVICE_POOL_THRESHOLDSr, reg, PD_SERVICE_POOL_1_TC_4_MAXIMUMLIMITf);
                    break;
                case 5:
                    *threshold_value = soc_reg_above_64_field32_get(unit, ECGM_CONGESTION_MANAGEMENT_PD_SERVICE_POOL_THRESHOLDSr, reg, PD_SERVICE_POOL_1_TC_5_MAXIMUMLIMITf);
                    break;
                case 6:
                    *threshold_value = soc_reg_above_64_field32_get(unit, ECGM_CONGESTION_MANAGEMENT_PD_SERVICE_POOL_THRESHOLDSr, reg, PD_SERVICE_POOL_1_TC_6_MAXIMUMLIMITf);
                    break;
                case 7:
                    *threshold_value = soc_reg_above_64_field32_get(unit, ECGM_CONGESTION_MANAGEMENT_PD_SERVICE_POOL_THRESHOLDSr, reg, PD_SERVICE_POOL_1_TC_7_MAXIMUMLIMITf);
                    break;
            }
        }
    }



exit:
  DNXC_FUNC_RETURN;

}

int
  jer2_qax_egr_queuing_sp_reserved_set(
    DNX_SAND_IN    int    unit,
    DNX_SAND_IN    int    core,
    DNX_SAND_IN    int    tc,
    DNX_SAND_IN    soc_dnx_cosq_threshold_type_t threshold_type,
    DNX_SAND_IN    int    threshold_value,
    DNX_SAND_IN    soc_dnx_cosq_threshold_global_type_t drop_type
  )
{
    soc_reg_above_64_val_t reg;
    uint64 reg_64;

    DNXC_INIT_FUNC_DEFS;
    if (tc == -1) /* Not per a spcific tc */
    {
        DNXC_IF_ERR_EXIT(READ_ECGM_CONGESTION_MANAGEMENT_MULTICAST_RESERVED_RESOURCESr(unit, &reg_64));
        if ((threshold_type == soc_dnx_cosq_threshold_packet_descriptors) || (threshold_type == soc_dnx_cosq_threshold_available_packet_descriptors))
        {
            switch (drop_type)
            {
                case soc_dnx_cosq_threshold_global_type_service_pool_0:
                    soc_reg64_field32_set(unit, ECGM_CONGESTION_MANAGEMENT_MULTICAST_RESERVED_RESOURCESr, &reg_64, RESERVED_P_DS_SERVICE_POOL_0f, threshold_value);
                    break;
                case soc_dnx_cosq_threshold_global_type_service_pool_1:
                    soc_reg64_field32_set(unit, ECGM_CONGESTION_MANAGEMENT_MULTICAST_RESERVED_RESOURCESr, &reg_64, RESERVED_P_DS_SERVICE_POOL_1f, threshold_value);
                    break;
                default:
                    break;
            }
        }
        else /*data buffers*/
        {
            switch (drop_type)
            {
                case soc_dnx_cosq_threshold_global_type_service_pool_0:
                    soc_reg64_field32_set(unit, ECGM_CONGESTION_MANAGEMENT_MULTICAST_RESERVED_RESOURCESr, &reg_64, RESERVED_D_BS_SERVICE_POOL_0f, threshold_value);
                    break;
                case soc_dnx_cosq_threshold_global_type_service_pool_1:
                    soc_reg64_field32_set(unit, ECGM_CONGESTION_MANAGEMENT_MULTICAST_RESERVED_RESOURCESr, &reg_64, RESERVED_D_BS_SERVICE_POOL_1f, threshold_value);
                    break;
                default:
                    break;
            }
        }

        DNXC_IF_ERR_EXIT(WRITE_ECGM_CONGESTION_MANAGEMENT_MULTICAST_RESERVED_RESOURCESr(unit, reg_64));
    }
    else /* specific TC */
    {
        if ((threshold_type == soc_dnx_cosq_threshold_data_buffers) || (threshold_type == soc_dnx_cosq_threshold_available_data_buffers))
        {
            DNXC_IF_ERR_EXIT(READ_ECGM_CONGESTION_MANAGEMENT_MULTICAST_RESERVED_RESOURCES_PER_CLASSr(unit, reg));

            if (drop_type == soc_dnx_cosq_threshold_global_type_service_pool_0)
            {
                switch (tc)
                {
                    case 0:
                        soc_reg_above_64_field32_set(unit, ECGM_CONGESTION_MANAGEMENT_MULTICAST_RESERVED_RESOURCES_PER_CLASSr, reg, RESERVED_DATA_BUFFERS_TC_0_SERVICE_POOL_0f, threshold_value);
                        break;
                    case 1:
                        soc_reg_above_64_field32_set(unit, ECGM_CONGESTION_MANAGEMENT_MULTICAST_RESERVED_RESOURCES_PER_CLASSr, reg, RESERVED_DATA_BUFFERS_TC_1_SERVICE_POOL_0f, threshold_value);
                        break;
                    case 2:
                        soc_reg_above_64_field32_set(unit, ECGM_CONGESTION_MANAGEMENT_MULTICAST_RESERVED_RESOURCES_PER_CLASSr, reg, RESERVED_DATA_BUFFERS_TC_2_SERVICE_POOL_0f, threshold_value);
                        break;
                    case 3:
                        soc_reg_above_64_field32_set(unit, ECGM_CONGESTION_MANAGEMENT_MULTICAST_RESERVED_RESOURCES_PER_CLASSr, reg, RESERVED_DATA_BUFFERS_TC_3_SERVICE_POOL_0f, threshold_value);
                        break;
                    case 4:
                        soc_reg_above_64_field32_set(unit, ECGM_CONGESTION_MANAGEMENT_MULTICAST_RESERVED_RESOURCES_PER_CLASSr, reg, RESERVED_DATA_BUFFERS_TC_4_SERVICE_POOL_0f, threshold_value);
                        break;
                    case 5:
                        soc_reg_above_64_field32_set(unit, ECGM_CONGESTION_MANAGEMENT_MULTICAST_RESERVED_RESOURCES_PER_CLASSr, reg, RESERVED_DATA_BUFFERS_TC_5_SERVICE_POOL_0f, threshold_value);
                        break;
                    case 6:
                        soc_reg_above_64_field32_set(unit, ECGM_CONGESTION_MANAGEMENT_MULTICAST_RESERVED_RESOURCES_PER_CLASSr, reg, RESERVED_DATA_BUFFERS_TC_6_SERVICE_POOL_0f, threshold_value);
                        break;
                    case 7:
                        soc_reg_above_64_field32_set(unit, ECGM_CONGESTION_MANAGEMENT_MULTICAST_RESERVED_RESOURCES_PER_CLASSr, reg, RESERVED_DATA_BUFFERS_TC_7_SERVICE_POOL_0f, threshold_value);
                        break;
                    default:
                        DNXC_EXIT_WITH_ERR(SOC_E_PARAM,(_BSL_DNXC_MSG("Invalid tc value %d\n"),tc));
                }
            }
            else /* service pool 1 */
            {
                switch (tc)
                {
                    case 0:
                        soc_reg_above_64_field32_set(unit, ECGM_CONGESTION_MANAGEMENT_MULTICAST_RESERVED_RESOURCES_PER_CLASSr, reg, RESERVED_DATA_BUFFERS_TC_0_SERVICE_POOL_1f, threshold_value);
                        break;
                    case 1:
                        soc_reg_above_64_field32_set(unit, ECGM_CONGESTION_MANAGEMENT_MULTICAST_RESERVED_RESOURCES_PER_CLASSr, reg, RESERVED_DATA_BUFFERS_TC_1_SERVICE_POOL_1f, threshold_value);
                        break;
                    case 2:
                        soc_reg_above_64_field32_set(unit, ECGM_CONGESTION_MANAGEMENT_MULTICAST_RESERVED_RESOURCES_PER_CLASSr, reg, RESERVED_DATA_BUFFERS_TC_2_SERVICE_POOL_1f, threshold_value);
                        break;
                    case 3:
                        soc_reg_above_64_field32_set(unit, ECGM_CONGESTION_MANAGEMENT_MULTICAST_RESERVED_RESOURCES_PER_CLASSr, reg, RESERVED_DATA_BUFFERS_TC_3_SERVICE_POOL_1f, threshold_value);
                        break;
                    case 4:
                        soc_reg_above_64_field32_set(unit, ECGM_CONGESTION_MANAGEMENT_MULTICAST_RESERVED_RESOURCES_PER_CLASSr, reg, RESERVED_DATA_BUFFERS_TC_4_SERVICE_POOL_1f, threshold_value);
                        break;
                    case 5:
                        soc_reg_above_64_field32_set(unit, ECGM_CONGESTION_MANAGEMENT_MULTICAST_RESERVED_RESOURCES_PER_CLASSr, reg, RESERVED_DATA_BUFFERS_TC_5_SERVICE_POOL_1f, threshold_value);
                        break;
                    case 6:
                        soc_reg_above_64_field32_set(unit, ECGM_CONGESTION_MANAGEMENT_MULTICAST_RESERVED_RESOURCES_PER_CLASSr, reg, RESERVED_DATA_BUFFERS_TC_6_SERVICE_POOL_1f, threshold_value);
                        break;
                    case 7:
                        soc_reg_above_64_field32_set(unit, ECGM_CONGESTION_MANAGEMENT_MULTICAST_RESERVED_RESOURCES_PER_CLASSr, reg, RESERVED_DATA_BUFFERS_TC_7_SERVICE_POOL_1f, threshold_value);
                        break;
                    default:
                        DNXC_EXIT_WITH_ERR(SOC_E_PARAM,(_BSL_DNXC_MSG("Invalid tc value %d\n"),tc));
                }
            }
            DNXC_IF_ERR_EXIT(WRITE_ECGM_CONGESTION_MANAGEMENT_MULTICAST_RESERVED_RESOURCES_PER_CLASSr(unit, reg));
        }
        else {
          DNXC_EXIT_WITH_ERR(SOC_E_PARAM,(_BSL_DNXC_MSG("TC specific threshold is not supported for this type %d\n"),threshold_type));
        }
    }


exit:
    DNXC_FUNC_RETURN;
}



int
  jer2_qax_egr_queuing_sp_reserved_get(
    DNX_SAND_IN    int    unit,
    DNX_SAND_IN    int    core,
    DNX_SAND_IN    int    tc,
    DNX_SAND_IN    soc_dnx_cosq_threshold_type_t threshold_type,
    DNX_SAND_OUT   int*    threshold_value,
    DNX_SAND_IN    soc_dnx_cosq_threshold_global_type_t drop_type
  )
{
    soc_reg_above_64_val_t reg;
    uint64 reg_64;

    DNXC_INIT_FUNC_DEFS;
    if (tc == -1) /* Not per a spcific tc */
    {
        DNXC_IF_ERR_EXIT(READ_ECGM_CONGESTION_MANAGEMENT_MULTICAST_RESERVED_RESOURCESr(unit, &reg_64));
        if ((threshold_type == soc_dnx_cosq_threshold_packet_descriptors) || (threshold_type == soc_dnx_cosq_threshold_available_packet_descriptors))
        {
            switch (drop_type)
            {
                case soc_dnx_cosq_threshold_global_type_service_pool_0:
                    *threshold_value = soc_reg64_field32_get(unit, ECGM_CONGESTION_MANAGEMENT_MULTICAST_RESERVED_RESOURCESr, reg_64, RESERVED_P_DS_SERVICE_POOL_0f);
                    break;
                case soc_dnx_cosq_threshold_global_type_service_pool_1:
                    *threshold_value = soc_reg64_field32_get(unit, ECGM_CONGESTION_MANAGEMENT_MULTICAST_RESERVED_RESOURCESr, reg_64, RESERVED_P_DS_SERVICE_POOL_1f);
                    break;
                default:
                    break;
            }
        }
        else /*data buffers*/
        {
            switch (drop_type)
            {
                case soc_dnx_cosq_threshold_global_type_service_pool_0:
                    *threshold_value = soc_reg64_field32_get(unit, ECGM_CONGESTION_MANAGEMENT_MULTICAST_RESERVED_RESOURCESr, reg_64, RESERVED_D_BS_SERVICE_POOL_0f);
                    break;
                case soc_dnx_cosq_threshold_global_type_service_pool_1:
                    *threshold_value = soc_reg64_field32_get(unit, ECGM_CONGESTION_MANAGEMENT_MULTICAST_RESERVED_RESOURCESr, reg_64, RESERVED_D_BS_SERVICE_POOL_1f);
                    break;
                default:
                    break;
            }
        }

    }
    else /* specific TC */
    {
        if ((threshold_type == soc_dnx_cosq_threshold_data_buffers) || (threshold_type == soc_dnx_cosq_threshold_available_data_buffers))
        {
            DNXC_IF_ERR_EXIT(READ_ECGM_CONGESTION_MANAGEMENT_MULTICAST_RESERVED_RESOURCES_PER_CLASSr(unit, reg));

            if (drop_type == soc_dnx_cosq_threshold_global_type_service_pool_0)
            {
                switch (tc)
                {
                    case 0:
                        *threshold_value = soc_reg_above_64_field32_get(unit, ECGM_CONGESTION_MANAGEMENT_MULTICAST_RESERVED_RESOURCES_PER_CLASSr, reg, RESERVED_DATA_BUFFERS_TC_0_SERVICE_POOL_0f);
                        break;
                    case 1:
                        *threshold_value = soc_reg_above_64_field32_get(unit, ECGM_CONGESTION_MANAGEMENT_MULTICAST_RESERVED_RESOURCES_PER_CLASSr, reg, RESERVED_DATA_BUFFERS_TC_1_SERVICE_POOL_0f);
                        break;
                    case 2:
                        *threshold_value = soc_reg_above_64_field32_get(unit, ECGM_CONGESTION_MANAGEMENT_MULTICAST_RESERVED_RESOURCES_PER_CLASSr, reg, RESERVED_DATA_BUFFERS_TC_2_SERVICE_POOL_0f);
                        break;
                    case 3:
                        *threshold_value = soc_reg_above_64_field32_get(unit, ECGM_CONGESTION_MANAGEMENT_MULTICAST_RESERVED_RESOURCES_PER_CLASSr, reg, RESERVED_DATA_BUFFERS_TC_3_SERVICE_POOL_0f);
                        break;
                    case 4:
                        *threshold_value = soc_reg_above_64_field32_get(unit, ECGM_CONGESTION_MANAGEMENT_MULTICAST_RESERVED_RESOURCES_PER_CLASSr, reg, RESERVED_DATA_BUFFERS_TC_4_SERVICE_POOL_0f);
                        break;
                    case 5:
                        *threshold_value = soc_reg_above_64_field32_get(unit, ECGM_CONGESTION_MANAGEMENT_MULTICAST_RESERVED_RESOURCES_PER_CLASSr, reg, RESERVED_DATA_BUFFERS_TC_5_SERVICE_POOL_0f);
                        break;
                    case 6:
                        *threshold_value = soc_reg_above_64_field32_get(unit, ECGM_CONGESTION_MANAGEMENT_MULTICAST_RESERVED_RESOURCES_PER_CLASSr, reg, RESERVED_DATA_BUFFERS_TC_6_SERVICE_POOL_0f);
                        break;
                    case 7:
                        *threshold_value = soc_reg_above_64_field32_get(unit, ECGM_CONGESTION_MANAGEMENT_MULTICAST_RESERVED_RESOURCES_PER_CLASSr, reg, RESERVED_DATA_BUFFERS_TC_7_SERVICE_POOL_0f);
                        break;
                    default:
                        DNXC_EXIT_WITH_ERR(SOC_E_PARAM,(_BSL_DNXC_MSG("Invalid tc value %d\n"),tc));
                }
            }
            else /* service pool 1 */
            {
                switch (tc)
                {
                    case 0:
                        *threshold_value = soc_reg_above_64_field32_get(unit, ECGM_CONGESTION_MANAGEMENT_MULTICAST_RESERVED_RESOURCES_PER_CLASSr, reg, RESERVED_DATA_BUFFERS_TC_0_SERVICE_POOL_1f);
                        break;
                    case 1:
                        *threshold_value = soc_reg_above_64_field32_get(unit, ECGM_CONGESTION_MANAGEMENT_MULTICAST_RESERVED_RESOURCES_PER_CLASSr, reg, RESERVED_DATA_BUFFERS_TC_1_SERVICE_POOL_1f);
                        break;
                    case 2:
                        *threshold_value = soc_reg_above_64_field32_get(unit, ECGM_CONGESTION_MANAGEMENT_MULTICAST_RESERVED_RESOURCES_PER_CLASSr, reg, RESERVED_DATA_BUFFERS_TC_2_SERVICE_POOL_1f);
                        break;
                    case 3:
                        *threshold_value = soc_reg_above_64_field32_get(unit, ECGM_CONGESTION_MANAGEMENT_MULTICAST_RESERVED_RESOURCES_PER_CLASSr, reg, RESERVED_DATA_BUFFERS_TC_3_SERVICE_POOL_1f);
                        break;
                    case 4:
                        *threshold_value = soc_reg_above_64_field32_get(unit, ECGM_CONGESTION_MANAGEMENT_MULTICAST_RESERVED_RESOURCES_PER_CLASSr, reg, RESERVED_DATA_BUFFERS_TC_4_SERVICE_POOL_1f);
                        break;
                    case 5:
                        *threshold_value = soc_reg_above_64_field32_get(unit, ECGM_CONGESTION_MANAGEMENT_MULTICAST_RESERVED_RESOURCES_PER_CLASSr, reg, RESERVED_DATA_BUFFERS_TC_5_SERVICE_POOL_1f);
                        break;
                    case 6:
                        *threshold_value = soc_reg_above_64_field32_get(unit, ECGM_CONGESTION_MANAGEMENT_MULTICAST_RESERVED_RESOURCES_PER_CLASSr, reg, RESERVED_DATA_BUFFERS_TC_6_SERVICE_POOL_1f);
                        break;
                    case 7:
                        *threshold_value = soc_reg_above_64_field32_get(unit, ECGM_CONGESTION_MANAGEMENT_MULTICAST_RESERVED_RESOURCES_PER_CLASSr, reg, RESERVED_DATA_BUFFERS_TC_7_SERVICE_POOL_1f);
                        break;
                    default:
                        DNXC_EXIT_WITH_ERR(SOC_E_PARAM,(_BSL_DNXC_MSG("Invalid tc value %d\n"),tc));
                }
            }
        }
        else {
         DNXC_EXIT_WITH_ERR(SOC_E_PARAM,(_BSL_DNXC_MSG("TC specific threshold is not supported for this type %d\n"),threshold_type));
        }
    }


exit:
    DNXC_FUNC_RETURN;
}




int
  jer2_qax_egr_queuing_global_fc_set(
      DNX_SAND_IN  int   unit,
      DNX_SAND_IN  int   core,
      DNX_SAND_IN  soc_dnx_cosq_threshold_type_t threshold_type,
      DNX_SAND_IN  int    threshold_value,
      DNX_SAND_IN  soc_dnx_cosq_threshold_global_type_t drop_type
  )
{
    soc_reg_above_64_val_t reg;

    DNXC_INIT_FUNC_DEFS;
    DNXC_IF_ERR_EXIT(READ_ECGM_CONGESTION_MANAGEMENT_GLOBAL_FLOW_CONTROL_THRESHOLDSr(unit, reg));

    if (threshold_type == soc_dnx_cosq_threshold_data_buffers || threshold_type == soc_dnx_cosq_threshold_available_data_buffers )
    {
        switch (drop_type)
        {
            case soc_dnx_cosq_threshold_global_type_unicast:
                soc_reg_above_64_field32_set(unit, ECGM_CONGESTION_MANAGEMENT_GLOBAL_FLOW_CONTROL_THRESHOLDSr, reg, UNICAST_DATA_BUFFERS_FLOW_CONTROL_LIMITf, threshold_value);
                break;
            case soc_dnx_cosq_threshold_global_type_multicast:
                soc_reg_above_64_field32_set(unit, ECGM_CONGESTION_MANAGEMENT_GLOBAL_FLOW_CONTROL_THRESHOLDSr, reg, MULTICAST_DATA_BUFFERS_FLOW_CONTROL_LIMITf, threshold_value);
                break;
            case soc_dnx_cosq_threshold_global_type_total:
                soc_reg_above_64_field32_set(unit, ECGM_CONGESTION_MANAGEMENT_GLOBAL_FLOW_CONTROL_THRESHOLDSr, reg, TOTAL_DATA_BUFFERS_FLOW_CONTROL_LIMITf, threshold_value);
                break;
            case soc_dnx_cosq_threshold_global_type_service_pool_0:
                soc_reg_above_64_field32_set(unit, ECGM_CONGESTION_MANAGEMENT_GLOBAL_FLOW_CONTROL_THRESHOLDSr, reg, MULTICAST_DATA_BUFFERS_SERVICE_POOL_0_FLOW_CONTROL_LIMITf, threshold_value);
                break;
            case soc_dnx_cosq_threshold_global_type_service_pool_1:
                soc_reg_above_64_field32_set(unit, ECGM_CONGESTION_MANAGEMENT_GLOBAL_FLOW_CONTROL_THRESHOLDSr, reg, MULTICAST_DATA_BUFFERS_SERVICE_POOL_1_FLOW_CONTROL_LIMITf, threshold_value);
                break;
            default:

                break;
        }
    }
    else if (threshold_type == soc_dnx_cosq_threshold_packet_descriptors || threshold_type == soc_dnx_cosq_threshold_available_packet_descriptors || threshold_type == soc_dnx_cosq_threshold_buffer_descriptors)
    {
        switch (drop_type)
        {
            case soc_dnx_cosq_threshold_global_type_unicast:
                soc_reg_above_64_field32_set(unit, ECGM_CONGESTION_MANAGEMENT_GLOBAL_FLOW_CONTROL_THRESHOLDSr, reg, UNICAST_PACKET_DESCRIPTORS_FLOW_CONTROL_LIMITf, threshold_value);
                break;
            case soc_dnx_cosq_threshold_global_type_multicast:
                soc_reg_above_64_field32_set(unit, ECGM_CONGESTION_MANAGEMENT_GLOBAL_FLOW_CONTROL_THRESHOLDSr, reg, MULTICAST_PACKET_DESCRIPTORS_FLOW_CONTROL_LIMITf, threshold_value);
                break;
            case soc_dnx_cosq_threshold_global_type_total:
                soc_reg_above_64_field32_set(unit, ECGM_CONGESTION_MANAGEMENT_GLOBAL_FLOW_CONTROL_THRESHOLDSr, reg, TOTAL_PACKET_DESCRIPTORS_FLOW_CONTROLLIMITf, threshold_value);
                break;
            case soc_dnx_cosq_threshold_global_type_service_pool_0:
                soc_reg_above_64_field32_set(unit, ECGM_CONGESTION_MANAGEMENT_GLOBAL_FLOW_CONTROL_THRESHOLDSr, reg, MULTICAST_PACKET_DESCRIPTORS_SERVICE_POOL_0_FLOW_CONTROL_LIMITf, threshold_value);
                break;
            case soc_dnx_cosq_threshold_global_type_service_pool_1:
                soc_reg_above_64_field32_set(unit, ECGM_CONGESTION_MANAGEMENT_GLOBAL_FLOW_CONTROL_THRESHOLDSr, reg, MULTICAST_PACKET_DESCRIPTORS_SERVICE_POOL_1_FLOW_CONTROL_LIMITf, threshold_value);
                break;
            default:
                break;
        }
    }
    DNXC_IF_ERR_EXIT(WRITE_ECGM_CONGESTION_MANAGEMENT_GLOBAL_FLOW_CONTROL_THRESHOLDSr(unit, reg));



exit:
  DNXC_FUNC_RETURN;
}

int
  jer2_qax_egr_queuing_global_fc_get(
      DNX_SAND_IN  int   unit,
      DNX_SAND_IN  int   core,
      DNX_SAND_IN  soc_dnx_cosq_threshold_type_t threshold_type,
      DNX_SAND_OUT int*    threshold_value,
      DNX_SAND_IN  soc_dnx_cosq_threshold_global_type_t drop_type
  )
{
    soc_reg_above_64_val_t reg;

    DNXC_INIT_FUNC_DEFS;
    DNXC_IF_ERR_EXIT(READ_ECGM_CONGESTION_MANAGEMENT_GLOBAL_FLOW_CONTROL_THRESHOLDSr(unit, reg));

    if (threshold_type == soc_dnx_cosq_threshold_data_buffers || threshold_type == soc_dnx_cosq_threshold_available_data_buffers)
    {
        switch (drop_type)
        {
            case soc_dnx_cosq_threshold_global_type_unicast:
                *threshold_value = soc_reg_above_64_field32_get(unit, ECGM_CONGESTION_MANAGEMENT_GLOBAL_FLOW_CONTROL_THRESHOLDSr, reg, UNICAST_DATA_BUFFERS_FLOW_CONTROL_LIMITf);
                break;
            case soc_dnx_cosq_threshold_global_type_multicast:
                *threshold_value = soc_reg_above_64_field32_get(unit, ECGM_CONGESTION_MANAGEMENT_GLOBAL_FLOW_CONTROL_THRESHOLDSr, reg, MULTICAST_DATA_BUFFERS_FLOW_CONTROL_LIMITf);
                break;
            case soc_dnx_cosq_threshold_global_type_total:
                *threshold_value = soc_reg_above_64_field32_get(unit, ECGM_CONGESTION_MANAGEMENT_GLOBAL_FLOW_CONTROL_THRESHOLDSr, reg, TOTAL_DATA_BUFFERS_FLOW_CONTROL_LIMITf);
                break;
            case soc_dnx_cosq_threshold_global_type_service_pool_0:
                *threshold_value = soc_reg_above_64_field32_get(unit, ECGM_CONGESTION_MANAGEMENT_GLOBAL_FLOW_CONTROL_THRESHOLDSr, reg, MULTICAST_DATA_BUFFERS_SERVICE_POOL_0_FLOW_CONTROL_LIMITf);
                break;
            case soc_dnx_cosq_threshold_global_type_service_pool_1:
                *threshold_value = soc_reg_above_64_field32_get(unit, ECGM_CONGESTION_MANAGEMENT_GLOBAL_FLOW_CONTROL_THRESHOLDSr, reg, MULTICAST_DATA_BUFFERS_SERVICE_POOL_1_FLOW_CONTROL_LIMITf);
                break;
            default:
                break;
        }
    }
    else if (threshold_type == soc_dnx_cosq_threshold_packet_descriptors || threshold_type == soc_dnx_cosq_threshold_available_packet_descriptors || threshold_type == soc_dnx_cosq_threshold_buffer_descriptors)
    {
        switch (drop_type)
        {
            case soc_dnx_cosq_threshold_global_type_unicast:
                *threshold_value = soc_reg_above_64_field32_get(unit, ECGM_CONGESTION_MANAGEMENT_GLOBAL_FLOW_CONTROL_THRESHOLDSr, reg, UNICAST_PACKET_DESCRIPTORS_FLOW_CONTROL_LIMITf);
                break;
            case soc_dnx_cosq_threshold_global_type_multicast:
                *threshold_value = soc_reg_above_64_field32_get(unit, ECGM_CONGESTION_MANAGEMENT_GLOBAL_FLOW_CONTROL_THRESHOLDSr, reg, MULTICAST_PACKET_DESCRIPTORS_FLOW_CONTROL_LIMITf);
                break;
            case soc_dnx_cosq_threshold_global_type_total:
                *threshold_value = soc_reg_above_64_field32_get(unit, ECGM_CONGESTION_MANAGEMENT_GLOBAL_FLOW_CONTROL_THRESHOLDSr, reg, TOTAL_PACKET_DESCRIPTORS_FLOW_CONTROLLIMITf);
                break;
            case soc_dnx_cosq_threshold_global_type_service_pool_0:
                *threshold_value = soc_reg_above_64_field32_get(unit, ECGM_CONGESTION_MANAGEMENT_GLOBAL_FLOW_CONTROL_THRESHOLDSr, reg, MULTICAST_PACKET_DESCRIPTORS_SERVICE_POOL_0_FLOW_CONTROL_LIMITf);
                break;
            case soc_dnx_cosq_threshold_global_type_service_pool_1:
                *threshold_value = soc_reg_above_64_field32_get(unit, ECGM_CONGESTION_MANAGEMENT_GLOBAL_FLOW_CONTROL_THRESHOLDSr, reg, MULTICAST_PACKET_DESCRIPTORS_SERVICE_POOL_1_FLOW_CONTROL_LIMITf);
                break;
            default:
                break;
        }
    }


exit:
  DNXC_FUNC_RETURN;
}

int
  jer2_qax_egr_queuing_mc_tc_fc_set(
    DNX_SAND_IN    int    unit,
    DNX_SAND_IN    int    core,
    DNX_SAND_IN    int    tc,
    DNX_SAND_IN    soc_dnx_cosq_threshold_type_t threshold_type,
    DNX_SAND_IN    int    threshold_value
  )
{
    soc_reg_above_64_val_t reg;

    DNXC_INIT_FUNC_DEFS;

    if (threshold_type == soc_dnx_cosq_threshold_packet_descriptors || threshold_type == soc_dnx_cosq_threshold_available_packet_descriptors || threshold_type == soc_dnx_cosq_threshold_buffer_descriptors)
    {
        DNXC_IF_ERR_EXIT(READ_ECGM_CGM_MC_PD_TC_FC_THr(unit, reg));
        switch (tc)
        {
            case 0:
                soc_reg_above_64_field32_set(unit, ECGM_CGM_MC_PD_TC_FC_THr, reg, CGM_MC_PD_TC_FC_TH_0f, threshold_value);
                break;
            case 1:
                soc_reg_above_64_field32_set(unit, ECGM_CGM_MC_PD_TC_FC_THr, reg, CGM_MC_PD_TC_FC_TH_1f, threshold_value);
                break;
            case 2:
                soc_reg_above_64_field32_set(unit, ECGM_CGM_MC_PD_TC_FC_THr, reg, CGM_MC_PD_TC_FC_TH_2f, threshold_value);
                break;
            case 3:
                soc_reg_above_64_field32_set(unit, ECGM_CGM_MC_PD_TC_FC_THr, reg, CGM_MC_PD_TC_FC_TH_3f, threshold_value);
                break;
            case 4:
                soc_reg_above_64_field32_set(unit, ECGM_CGM_MC_PD_TC_FC_THr, reg, CGM_MC_PD_TC_FC_TH_4f, threshold_value);
                break;
            case 5:
                soc_reg_above_64_field32_set(unit, ECGM_CGM_MC_PD_TC_FC_THr, reg, CGM_MC_PD_TC_FC_TH_5f, threshold_value);
                break;
            case 6:
                soc_reg_above_64_field32_set(unit, ECGM_CGM_MC_PD_TC_FC_THr, reg, CGM_MC_PD_TC_FC_TH_6f, threshold_value);
                break;
            case 7:
                soc_reg_above_64_field32_set(unit, ECGM_CGM_MC_PD_TC_FC_THr, reg, CGM_MC_PD_TC_FC_TH_7f, threshold_value);
                break;
            default:
                break;
        }
        DNXC_IF_ERR_EXIT(WRITE_ECGM_CGM_MC_PD_TC_FC_THr(unit, reg));
    }
    else if (threshold_type == soc_dnx_cosq_threshold_data_buffers || threshold_type == soc_dnx_cosq_threshold_available_data_buffers) {
        DNXC_IF_ERR_EXIT(READ_ECGM_CONGESTION_MANAGEMENT_MULTICAST_FLOW_CONTROL_PER_TC_THREHOSLDSr(unit, reg));
        switch (tc)
        {
            case 0:
                soc_reg_above_64_field32_set(unit, ECGM_CONGESTION_MANAGEMENT_MULTICAST_FLOW_CONTROL_PER_TC_THREHOSLDSr, reg, MULTICAST_DATA_BUFFERS_TC_0_FLOW_CONTROL_LIMITf, threshold_value);
                break;
            case 1:
                soc_reg_above_64_field32_set(unit, ECGM_CONGESTION_MANAGEMENT_MULTICAST_FLOW_CONTROL_PER_TC_THREHOSLDSr, reg, MULTICAST_DATA_BUFFERS_TC_1_FLOW_CONTROL_LIMITf, threshold_value);
                break;
            case 2:
                soc_reg_above_64_field32_set(unit, ECGM_CONGESTION_MANAGEMENT_MULTICAST_FLOW_CONTROL_PER_TC_THREHOSLDSr, reg, MULTICAST_DATA_BUFFERS_TC_2_FLOW_CONTROL_LIMITf, threshold_value);
                break;
            case 3:
                soc_reg_above_64_field32_set(unit, ECGM_CONGESTION_MANAGEMENT_MULTICAST_FLOW_CONTROL_PER_TC_THREHOSLDSr, reg, MULTICAST_DATA_BUFFERS_TC_3_FLOW_CONTROL_LIMITf, threshold_value);
                break;
            case 4:
                soc_reg_above_64_field32_set(unit, ECGM_CONGESTION_MANAGEMENT_MULTICAST_FLOW_CONTROL_PER_TC_THREHOSLDSr, reg, MULTICAST_DATA_BUFFERS_TC_4_FLOW_CONTROL_LIMITf, threshold_value);
                break;
            case 5:
                soc_reg_above_64_field32_set(unit, ECGM_CONGESTION_MANAGEMENT_MULTICAST_FLOW_CONTROL_PER_TC_THREHOSLDSr, reg, MULTICAST_DATA_BUFFERS_TC_5_FLOW_CONTROL_LIMITf, threshold_value);
                break;
            case 6:
                soc_reg_above_64_field32_set(unit, ECGM_CONGESTION_MANAGEMENT_MULTICAST_FLOW_CONTROL_PER_TC_THREHOSLDSr, reg, MULTICAST_DATA_BUFFERS_TC_6_FLOW_CONTROL_LIMITf, threshold_value);
                break;
            case 7:
                soc_reg_above_64_field32_set(unit, ECGM_CONGESTION_MANAGEMENT_MULTICAST_FLOW_CONTROL_PER_TC_THREHOSLDSr, reg, MULTICAST_DATA_BUFFERS_TC_7_FLOW_CONTROL_LIMITf, threshold_value);
                break;
            default:
                break;
        }
        DNXC_IF_ERR_EXIT(WRITE_ECGM_CONGESTION_MANAGEMENT_MULTICAST_FLOW_CONTROL_PER_TC_THREHOSLDSr(unit, reg));
    }


exit:
    DNXC_FUNC_RETURN;
}

int
  jer2_qax_egr_queuing_mc_tc_fc_get(
    DNX_SAND_IN    int    unit,
    DNX_SAND_IN    int    core,
    DNX_SAND_IN    int    tc,
    DNX_SAND_IN    soc_dnx_cosq_threshold_type_t threshold_type,
    DNX_SAND_OUT   int*   threshold_value
  )
{
    soc_reg_above_64_val_t reg;

    DNXC_INIT_FUNC_DEFS;

    if (threshold_type == soc_dnx_cosq_threshold_packet_descriptors || threshold_type == soc_dnx_cosq_threshold_available_packet_descriptors || threshold_type == soc_dnx_cosq_threshold_buffer_descriptors) {
        DNXC_IF_ERR_EXIT(READ_ECGM_CGM_MC_PD_TC_FC_THr(unit, reg));
        switch (tc)
        {
            case 0:
                *threshold_value = soc_reg_above_64_field32_get(unit, ECGM_CGM_MC_PD_TC_FC_THr, reg, CGM_MC_PD_TC_FC_TH_0f);
                break;
            case 1:
                *threshold_value = soc_reg_above_64_field32_get(unit, ECGM_CGM_MC_PD_TC_FC_THr, reg, CGM_MC_PD_TC_FC_TH_1f);
                break;
            case 2:
                *threshold_value = soc_reg_above_64_field32_get(unit, ECGM_CGM_MC_PD_TC_FC_THr, reg, CGM_MC_PD_TC_FC_TH_2f);
                break;
            case 3:
                *threshold_value = soc_reg_above_64_field32_get(unit, ECGM_CGM_MC_PD_TC_FC_THr, reg, CGM_MC_PD_TC_FC_TH_3f);
                break;
            case 4:
                *threshold_value = soc_reg_above_64_field32_get(unit, ECGM_CGM_MC_PD_TC_FC_THr, reg, CGM_MC_PD_TC_FC_TH_4f);
                break;
            case 5:
                *threshold_value = soc_reg_above_64_field32_get(unit, ECGM_CGM_MC_PD_TC_FC_THr, reg, CGM_MC_PD_TC_FC_TH_5f);
                break;
            case 6:
                *threshold_value = soc_reg_above_64_field32_get(unit, ECGM_CGM_MC_PD_TC_FC_THr, reg, CGM_MC_PD_TC_FC_TH_6f);
                break;
            case 7:
                *threshold_value = soc_reg_above_64_field32_get(unit, ECGM_CGM_MC_PD_TC_FC_THr, reg, CGM_MC_PD_TC_FC_TH_7f);
                break;
            default:
                break;
        }
    }
    else if (threshold_type == soc_dnx_cosq_threshold_data_buffers || threshold_type == soc_dnx_cosq_threshold_available_data_buffers) {
        DNXC_IF_ERR_EXIT(READ_ECGM_CONGESTION_MANAGEMENT_MULTICAST_FLOW_CONTROL_PER_TC_THREHOSLDSr(unit, reg));
        switch (tc)
        {
            case 0:
                *threshold_value = soc_reg_above_64_field32_get(unit, ECGM_CONGESTION_MANAGEMENT_MULTICAST_FLOW_CONTROL_PER_TC_THREHOSLDSr, reg, MULTICAST_DATA_BUFFERS_TC_0_FLOW_CONTROL_LIMITf);
                break;
            case 1:
                *threshold_value = soc_reg_above_64_field32_get(unit, ECGM_CONGESTION_MANAGEMENT_MULTICAST_FLOW_CONTROL_PER_TC_THREHOSLDSr, reg, MULTICAST_DATA_BUFFERS_TC_1_FLOW_CONTROL_LIMITf);
                break;
            case 2:
                *threshold_value = soc_reg_above_64_field32_get(unit, ECGM_CONGESTION_MANAGEMENT_MULTICAST_FLOW_CONTROL_PER_TC_THREHOSLDSr, reg, MULTICAST_DATA_BUFFERS_TC_2_FLOW_CONTROL_LIMITf);
                break;
            case 3:
                *threshold_value = soc_reg_above_64_field32_get(unit, ECGM_CONGESTION_MANAGEMENT_MULTICAST_FLOW_CONTROL_PER_TC_THREHOSLDSr, reg, MULTICAST_DATA_BUFFERS_TC_3_FLOW_CONTROL_LIMITf);
                break;
            case 4:
                *threshold_value = soc_reg_above_64_field32_get(unit, ECGM_CONGESTION_MANAGEMENT_MULTICAST_FLOW_CONTROL_PER_TC_THREHOSLDSr, reg, MULTICAST_DATA_BUFFERS_TC_4_FLOW_CONTROL_LIMITf);
                break;
            case 5:
                *threshold_value = soc_reg_above_64_field32_get(unit, ECGM_CONGESTION_MANAGEMENT_MULTICAST_FLOW_CONTROL_PER_TC_THREHOSLDSr, reg, MULTICAST_DATA_BUFFERS_TC_5_FLOW_CONTROL_LIMITf);
                break;
            case 6:
                *threshold_value = soc_reg_above_64_field32_get(unit, ECGM_CONGESTION_MANAGEMENT_MULTICAST_FLOW_CONTROL_PER_TC_THREHOSLDSr, reg, MULTICAST_DATA_BUFFERS_TC_6_FLOW_CONTROL_LIMITf);
                break;
            case 7:
                *threshold_value = soc_reg_above_64_field32_get(unit, ECGM_CONGESTION_MANAGEMENT_MULTICAST_FLOW_CONTROL_PER_TC_THREHOSLDSr, reg, MULTICAST_DATA_BUFFERS_TC_7_FLOW_CONTROL_LIMITf);
                break;
            default:
                break;
        }
    }


exit:
    DNXC_FUNC_RETURN;
}

int
  jer2_qax_egr_queuing_dev_set(
    DNX_SAND_IN    int    unit,
    DNX_SAND_IN    int    core,
    DNX_SAND_IN          JER2_ARAD_EGR_QUEUING_DEV_TH    *info
  )
{
  soc_reg_above_64_val_t
    field,
    reg,
    mem;
  uint64
    reg_64;
  uint32
    field_32;
  uint8
    index;

  DNXC_INIT_FUNC_DEFS;
  DNXC_NULL_CHECK(info);
  /* ECGM_ECGM_GENERAL_PD_THr */
  DNXC_IF_ERR_EXIT(READ_ECGM_CONGESTION_MANAGEMENT_GLOBAL_PD_THRESHOLDSr(unit,  reg));
  SOC_REG_ABOVE_64_CLEAR(field);
  SHR_BITCOPY_RANGE(field, 0, &info->global.uc.descriptors, 0, JER2_ARAD_EGQ_THRESHOLD_PD_NOF_BITS);
  soc_reg_above_64_field_set(unit, ECGM_CONGESTION_MANAGEMENT_GLOBAL_PD_THRESHOLDSr, reg, MAXIMUM_UNICAST_PACKET_DESCRIPTORSf, field);

  SOC_REG_ABOVE_64_CLEAR(field);
  SHR_BITCOPY_RANGE(field, 0, &info->global.mc.descriptors, 0, JER2_ARAD_EGQ_THRESHOLD_PD_NOF_BITS);
  soc_reg_above_64_field_set(unit, ECGM_CONGESTION_MANAGEMENT_GLOBAL_PD_THRESHOLDSr, reg, MAXIMUM_MULTICAST_PACKET_DESCRIPTORSf, field);

  SOC_REG_ABOVE_64_CLEAR(field);
  SHR_BITCOPY_RANGE(field, 0, &info->global.total.descriptors, 0, JER2_ARAD_EGQ_THRESHOLD_PD_NOF_BITS);
  soc_reg_above_64_field_set(unit, ECGM_CONGESTION_MANAGEMENT_GLOBAL_PD_THRESHOLDSr, reg, MAXIMUM_TOTAL_PACKET_DESCRIPTORSf, field);

  SOC_REG_ABOVE_64_CLEAR(field);
  SHR_BITCOPY_RANGE(field, 0, &info->pool[0].mc.descriptors, 0, JER2_ARAD_EGQ_THRESHOLD_PD_NOF_BITS);
  soc_reg_above_64_field_set(unit, ECGM_CONGESTION_MANAGEMENT_GLOBAL_PD_THRESHOLDSr, reg, SERVICE_POOL_0_MAXIMUM_PACKET_DESCRIPTORSf, field);

  SOC_REG_ABOVE_64_CLEAR(field);
  SHR_BITCOPY_RANGE(field, 0, &info->pool[1].mc.descriptors, 0, JER2_ARAD_EGQ_THRESHOLD_PD_NOF_BITS);
  soc_reg_above_64_field_set(unit, ECGM_CONGESTION_MANAGEMENT_GLOBAL_PD_THRESHOLDSr, reg, SERVICE_POOL_1_MAXIMUM_PACKET_DESCRIPTORSf, field);
  DNXC_IF_ERR_EXIT(WRITE_ECGM_CONGESTION_MANAGEMENT_GLOBAL_PD_THRESHOLDSr(unit,  reg));

  /* ECGM_ECGM_GENERAL_DB_THr */
  DNXC_IF_ERR_EXIT(READ_ECGM_CONGESTION_MANAGEMENT_GLOBAL_DB_THRESHOLDSr(unit,  reg));
  SOC_REG_ABOVE_64_CLEAR(field);
  SHR_BITCOPY_RANGE(field, 0, &info->global.uc.buffers, 0, JER2_ARAD_EGQ_THRESHOLD_DBUFF_NOF_BITS);
  soc_reg_above_64_field_set(unit, ECGM_CONGESTION_MANAGEMENT_GLOBAL_DB_THRESHOLDSr, reg, MAXIMUM_UNICAST_DATA_BUFFERSf, field);
  SOC_REG_ABOVE_64_CLEAR(field);
  SHR_BITCOPY_RANGE(field, 0, &info->global.mc.buffers, 0, JER2_ARAD_EGQ_THRESHOLD_DBUFF_NOF_BITS);
  soc_reg_above_64_field_set(unit, ECGM_CONGESTION_MANAGEMENT_GLOBAL_DB_THRESHOLDSr, reg, MAXIMUM_MULTICAST_DATA_BUFFERSf, field);
  SOC_REG_ABOVE_64_CLEAR(field);
  SHR_BITCOPY_RANGE(field, 0, &info->global.total.buffers, 0, JER2_ARAD_EGQ_THRESHOLD_DBUFF_NOF_BITS);
  soc_reg_above_64_field_set(unit, ECGM_CONGESTION_MANAGEMENT_GLOBAL_DB_THRESHOLDSr, reg, MAXIMUM_TOTAL_DATA_BUFFERSf, field);
  SOC_REG_ABOVE_64_CLEAR(field);
  SHR_BITCOPY_RANGE(field, 0, &info->pool[0].mc.buffers, 0, JER2_ARAD_EGQ_THRESHOLD_DBUFF_NOF_BITS);
  soc_reg_above_64_field_set(unit, ECGM_CONGESTION_MANAGEMENT_GLOBAL_DB_THRESHOLDSr, reg, SERVICE_POOL_0_MAXIMUM_DATA_BUFFERSf, field);
  SOC_REG_ABOVE_64_CLEAR(field);
  SHR_BITCOPY_RANGE(field, 0, &info->pool[1].mc.buffers, 0, JER2_ARAD_EGQ_THRESHOLD_DBUFF_NOF_BITS);
  soc_reg_above_64_field_set(unit, ECGM_CONGESTION_MANAGEMENT_GLOBAL_DB_THRESHOLDSr, reg, SERVICE_POOL_1_MAXIMUM_DATA_BUFFERSf, field);
  DNXC_IF_ERR_EXIT(WRITE_ECGM_CONGESTION_MANAGEMENT_GLOBAL_DB_THRESHOLDSr(unit,  reg));

  /* ECGM_ECGM_MC_PD_SP_TC_THr */
  DNXC_IF_ERR_EXIT(READ_ECGM_CONGESTION_MANAGEMENT_PD_SERVICE_POOL_THRESHOLDSr(unit,  reg));
  SOC_REG_ABOVE_64_CLEAR(field);
  SHR_BITCOPY_RANGE(field, 0, &info->pool_tc[0][0].mc.descriptors, 0, JER2_ARAD_EGQ_THRESHOLD_PD_NOF_BITS);
  soc_reg_above_64_field_set(unit, ECGM_CONGESTION_MANAGEMENT_PD_SERVICE_POOL_THRESHOLDSr, reg, PD_SERVICE_POOL_0_TC_0_MAXIMUMLIMITf, field);
  SOC_REG_ABOVE_64_CLEAR(field);
  SHR_BITCOPY_RANGE(field, 0, &info->pool_tc[0][1].mc.descriptors, 0, JER2_ARAD_EGQ_THRESHOLD_PD_NOF_BITS);
  soc_reg_above_64_field_set(unit, ECGM_CONGESTION_MANAGEMENT_PD_SERVICE_POOL_THRESHOLDSr, reg, PD_SERVICE_POOL_0_TC_1_MAXIMUMLIMITf, field);
  SOC_REG_ABOVE_64_CLEAR(field);
  SHR_BITCOPY_RANGE(field, 0, &info->pool_tc[0][2].mc.descriptors, 0, JER2_ARAD_EGQ_THRESHOLD_PD_NOF_BITS);
  soc_reg_above_64_field_set(unit, ECGM_CONGESTION_MANAGEMENT_PD_SERVICE_POOL_THRESHOLDSr, reg, PD_SERVICE_POOL_0_TC_2_MAXIMUMLIMITf, field);
  SOC_REG_ABOVE_64_CLEAR(field);
  SHR_BITCOPY_RANGE(field, 0, &info->pool_tc[0][3].mc.descriptors, 0, JER2_ARAD_EGQ_THRESHOLD_PD_NOF_BITS);
  soc_reg_above_64_field_set(unit, ECGM_CONGESTION_MANAGEMENT_PD_SERVICE_POOL_THRESHOLDSr, reg, PD_SERVICE_POOL_0_TC_3_MAXIMUMLIMITf, field);
  SOC_REG_ABOVE_64_CLEAR(field);
  SHR_BITCOPY_RANGE(field, 0, &info->pool_tc[0][4].mc.descriptors, 0, JER2_ARAD_EGQ_THRESHOLD_PD_NOF_BITS);
  soc_reg_above_64_field_set(unit, ECGM_CONGESTION_MANAGEMENT_PD_SERVICE_POOL_THRESHOLDSr, reg, PD_SERVICE_POOL_0_TC_4_MAXIMUMLIMITf, field);
  SOC_REG_ABOVE_64_CLEAR(field);
  SHR_BITCOPY_RANGE(field, 0, &info->pool_tc[0][5].mc.descriptors, 0, JER2_ARAD_EGQ_THRESHOLD_PD_NOF_BITS);
  soc_reg_above_64_field_set(unit, ECGM_CONGESTION_MANAGEMENT_PD_SERVICE_POOL_THRESHOLDSr, reg, PD_SERVICE_POOL_0_TC_5_MAXIMUMLIMITf, field);
  SOC_REG_ABOVE_64_CLEAR(field);
  SHR_BITCOPY_RANGE(field, 0, &info->pool_tc[0][6].mc.descriptors, 0, JER2_ARAD_EGQ_THRESHOLD_PD_NOF_BITS);
  soc_reg_above_64_field_set(unit, ECGM_CONGESTION_MANAGEMENT_PD_SERVICE_POOL_THRESHOLDSr, reg, PD_SERVICE_POOL_0_TC_6_MAXIMUMLIMITf, field);
  SOC_REG_ABOVE_64_CLEAR(field);
  SHR_BITCOPY_RANGE(field, 0, &info->pool_tc[0][7].mc.descriptors, 0, JER2_ARAD_EGQ_THRESHOLD_PD_NOF_BITS);
  soc_reg_above_64_field_set(unit, ECGM_CONGESTION_MANAGEMENT_PD_SERVICE_POOL_THRESHOLDSr, reg, PD_SERVICE_POOL_0_TC_7_MAXIMUMLIMITf, field);
  SOC_REG_ABOVE_64_CLEAR(field);
  SHR_BITCOPY_RANGE(field, 0, &info->pool_tc[1][0].mc.descriptors, 0, JER2_ARAD_EGQ_THRESHOLD_PD_NOF_BITS);
  soc_reg_above_64_field_set(unit, ECGM_CONGESTION_MANAGEMENT_PD_SERVICE_POOL_THRESHOLDSr, reg, PD_SERVICE_POOL_1_TC_0_MAXIMUMLIMITf, field);
  SOC_REG_ABOVE_64_CLEAR(field);
  SHR_BITCOPY_RANGE(field, 0, &info->pool_tc[1][1].mc.descriptors, 0, JER2_ARAD_EGQ_THRESHOLD_PD_NOF_BITS);
  soc_reg_above_64_field_set(unit, ECGM_CONGESTION_MANAGEMENT_PD_SERVICE_POOL_THRESHOLDSr, reg, PD_SERVICE_POOL_1_TC_1_MAXIMUMLIMITf, field);
  SOC_REG_ABOVE_64_CLEAR(field);
  SHR_BITCOPY_RANGE(field, 0, &info->pool_tc[1][2].mc.descriptors, 0, JER2_ARAD_EGQ_THRESHOLD_PD_NOF_BITS);
  soc_reg_above_64_field_set(unit, ECGM_CONGESTION_MANAGEMENT_PD_SERVICE_POOL_THRESHOLDSr, reg, PD_SERVICE_POOL_1_TC_2_MAXIMUMLIMITf, field);
  SOC_REG_ABOVE_64_CLEAR(field);
  SHR_BITCOPY_RANGE(field, 0, &info->pool_tc[1][3].mc.descriptors, 0, JER2_ARAD_EGQ_THRESHOLD_PD_NOF_BITS);
  soc_reg_above_64_field_set(unit, ECGM_CONGESTION_MANAGEMENT_PD_SERVICE_POOL_THRESHOLDSr, reg, PD_SERVICE_POOL_1_TC_3_MAXIMUMLIMITf, field);
  SOC_REG_ABOVE_64_CLEAR(field);
  SHR_BITCOPY_RANGE(field, 0, &info->pool_tc[1][4].mc.descriptors, 0, JER2_ARAD_EGQ_THRESHOLD_PD_NOF_BITS);
  soc_reg_above_64_field_set(unit, ECGM_CONGESTION_MANAGEMENT_PD_SERVICE_POOL_THRESHOLDSr, reg, PD_SERVICE_POOL_1_TC_4_MAXIMUMLIMITf, field);
  SOC_REG_ABOVE_64_CLEAR(field);
  SHR_BITCOPY_RANGE(field, 0, &info->pool_tc[1][5].mc.descriptors, 0, JER2_ARAD_EGQ_THRESHOLD_PD_NOF_BITS);
  soc_reg_above_64_field_set(unit, ECGM_CONGESTION_MANAGEMENT_PD_SERVICE_POOL_THRESHOLDSr, reg, PD_SERVICE_POOL_1_TC_5_MAXIMUMLIMITf, field);
  SOC_REG_ABOVE_64_CLEAR(field);
  SHR_BITCOPY_RANGE(field, 0, &info->pool_tc[1][6].mc.descriptors, 0, JER2_ARAD_EGQ_THRESHOLD_PD_NOF_BITS);
  soc_reg_above_64_field_set(unit, ECGM_CONGESTION_MANAGEMENT_PD_SERVICE_POOL_THRESHOLDSr, reg, PD_SERVICE_POOL_1_TC_6_MAXIMUMLIMITf, field);
  SOC_REG_ABOVE_64_CLEAR(field);
  SHR_BITCOPY_RANGE(field, 0, &info->pool_tc[1][7].mc.descriptors, 0, JER2_ARAD_EGQ_THRESHOLD_PD_NOF_BITS);
  soc_reg_above_64_field_set(unit, ECGM_CONGESTION_MANAGEMENT_PD_SERVICE_POOL_THRESHOLDSr, reg, PD_SERVICE_POOL_1_TC_7_MAXIMUMLIMITf, field);
  DNXC_IF_ERR_EXIT(WRITE_ECGM_CONGESTION_MANAGEMENT_PD_SERVICE_POOL_THRESHOLDSr(unit,  reg));

  /* ECGM_ECGM_MC_DB_SP_TC_THr */
  DNXC_IF_ERR_EXIT(READ_ECGM_CONGESTION_MANAGEMENT_DB_SERVICE_POOL_MAXIMUM_THRESHOLDSr(unit,  reg));
  SOC_REG_ABOVE_64_CLEAR(field);
  SHR_BITCOPY_RANGE(field, 0, &info->pool_tc[0][0].mc.buffers, 0, JER2_ARAD_EGQ_THRESHOLD_DBUFF_NOF_BITS);
  soc_reg_above_64_field_set(unit, ECGM_CONGESTION_MANAGEMENT_DB_SERVICE_POOL_MAXIMUM_THRESHOLDSr, reg, DB_SERVICE_POOL_0_TC_0_MAXIMUMLIMITf, field);
  SOC_REG_ABOVE_64_CLEAR(field);
  SHR_BITCOPY_RANGE(field, 0, &info->pool_tc[0][1].mc.buffers, 0, JER2_ARAD_EGQ_THRESHOLD_DBUFF_NOF_BITS);
  soc_reg_above_64_field_set(unit, ECGM_CONGESTION_MANAGEMENT_DB_SERVICE_POOL_MAXIMUM_THRESHOLDSr, reg, DB_SERVICE_POOL_0_TC_1_MAXIMUMLIMITf, field);
  SOC_REG_ABOVE_64_CLEAR(field);
  SHR_BITCOPY_RANGE(field, 0, &info->pool_tc[0][2].mc.buffers, 0, JER2_ARAD_EGQ_THRESHOLD_DBUFF_NOF_BITS);
  soc_reg_above_64_field_set(unit, ECGM_CONGESTION_MANAGEMENT_DB_SERVICE_POOL_MAXIMUM_THRESHOLDSr, reg, DB_SERVICE_POOL_0_TC_2_MAXIMUMLIMITf, field);
  SOC_REG_ABOVE_64_CLEAR(field);
  SHR_BITCOPY_RANGE(field, 0, &info->pool_tc[0][3].mc.buffers, 0, JER2_ARAD_EGQ_THRESHOLD_DBUFF_NOF_BITS);
  soc_reg_above_64_field_set(unit, ECGM_CONGESTION_MANAGEMENT_DB_SERVICE_POOL_MAXIMUM_THRESHOLDSr, reg, DB_SERVICE_POOL_0_TC_3_MAXIMUMLIMITf, field);
  SOC_REG_ABOVE_64_CLEAR(field);
  SHR_BITCOPY_RANGE(field, 0, &info->pool_tc[0][4].mc.buffers, 0, JER2_ARAD_EGQ_THRESHOLD_DBUFF_NOF_BITS);
  soc_reg_above_64_field_set(unit, ECGM_CONGESTION_MANAGEMENT_DB_SERVICE_POOL_MAXIMUM_THRESHOLDSr, reg, DB_SERVICE_POOL_0_TC_4_MAXIMUMLIMITf, field);
  SOC_REG_ABOVE_64_CLEAR(field);
  SHR_BITCOPY_RANGE(field, 0, &info->pool_tc[0][5].mc.buffers, 0, JER2_ARAD_EGQ_THRESHOLD_DBUFF_NOF_BITS);
  soc_reg_above_64_field_set(unit, ECGM_CONGESTION_MANAGEMENT_DB_SERVICE_POOL_MAXIMUM_THRESHOLDSr, reg, DB_SERVICE_POOL_0_TC_5_MAXIMUMLIMITf, field);
  SOC_REG_ABOVE_64_CLEAR(field);
  SHR_BITCOPY_RANGE(field, 0, &info->pool_tc[0][6].mc.buffers, 0, JER2_ARAD_EGQ_THRESHOLD_DBUFF_NOF_BITS);
  soc_reg_above_64_field_set(unit, ECGM_CONGESTION_MANAGEMENT_DB_SERVICE_POOL_MAXIMUM_THRESHOLDSr, reg, DB_SERVICE_POOL_0_TC_6_MAXIMUMLIMITf, field);
  SOC_REG_ABOVE_64_CLEAR(field);
  SHR_BITCOPY_RANGE(field, 0, &info->pool_tc[0][7].mc.buffers, 0, JER2_ARAD_EGQ_THRESHOLD_DBUFF_NOF_BITS);
  soc_reg_above_64_field_set(unit, ECGM_CONGESTION_MANAGEMENT_DB_SERVICE_POOL_MAXIMUM_THRESHOLDSr, reg, DB_SERVICE_POOL_0_TC_7_MAXIMUMLIMITf, field);
  SOC_REG_ABOVE_64_CLEAR(field);
  SHR_BITCOPY_RANGE(field, 0, &info->pool_tc[1][0].mc.buffers, 0, JER2_ARAD_EGQ_THRESHOLD_DBUFF_NOF_BITS);
  soc_reg_above_64_field_set(unit, ECGM_CONGESTION_MANAGEMENT_DB_SERVICE_POOL_MAXIMUM_THRESHOLDSr, reg, DB_SERVICE_POOL_1_TC_0_MAXIMUMLIMITf, field);
  SOC_REG_ABOVE_64_CLEAR(field);
  SHR_BITCOPY_RANGE(field, 0, &info->pool_tc[1][1].mc.buffers, 0, JER2_ARAD_EGQ_THRESHOLD_DBUFF_NOF_BITS);
  soc_reg_above_64_field_set(unit, ECGM_CONGESTION_MANAGEMENT_DB_SERVICE_POOL_MAXIMUM_THRESHOLDSr, reg, DB_SERVICE_POOL_1_TC_1_MAXIMUMLIMITf, field);
  SOC_REG_ABOVE_64_CLEAR(field);
  SHR_BITCOPY_RANGE(field, 0, &info->pool_tc[1][2].mc.buffers, 0, JER2_ARAD_EGQ_THRESHOLD_DBUFF_NOF_BITS);
  soc_reg_above_64_field_set(unit, ECGM_CONGESTION_MANAGEMENT_DB_SERVICE_POOL_MAXIMUM_THRESHOLDSr, reg, DB_SERVICE_POOL_1_TC_2_MAXIMUMLIMITf, field);
  SOC_REG_ABOVE_64_CLEAR(field);
  SHR_BITCOPY_RANGE(field, 0, &info->pool_tc[1][3].mc.buffers, 0, JER2_ARAD_EGQ_THRESHOLD_DBUFF_NOF_BITS);
  soc_reg_above_64_field_set(unit, ECGM_CONGESTION_MANAGEMENT_DB_SERVICE_POOL_MAXIMUM_THRESHOLDSr, reg, DB_SERVICE_POOL_1_TC_3_MAXIMUMLIMITf, field);
  SOC_REG_ABOVE_64_CLEAR(field);
  SHR_BITCOPY_RANGE(field, 0, &info->pool_tc[1][4].mc.buffers, 0, JER2_ARAD_EGQ_THRESHOLD_DBUFF_NOF_BITS);
  soc_reg_above_64_field_set(unit, ECGM_CONGESTION_MANAGEMENT_DB_SERVICE_POOL_MAXIMUM_THRESHOLDSr, reg, DB_SERVICE_POOL_1_TC_4_MAXIMUMLIMITf, field);
  SOC_REG_ABOVE_64_CLEAR(field);
  SHR_BITCOPY_RANGE(field, 0, &info->pool_tc[1][5].mc.buffers, 0, JER2_ARAD_EGQ_THRESHOLD_DBUFF_NOF_BITS);
  soc_reg_above_64_field_set(unit, ECGM_CONGESTION_MANAGEMENT_DB_SERVICE_POOL_MAXIMUM_THRESHOLDSr, reg, DB_SERVICE_POOL_1_TC_5_MAXIMUMLIMITf, field);
  SOC_REG_ABOVE_64_CLEAR(field);
  SHR_BITCOPY_RANGE(field, 0, &info->pool_tc[1][6].mc.buffers, 0, JER2_ARAD_EGQ_THRESHOLD_DBUFF_NOF_BITS);
  soc_reg_above_64_field_set(unit, ECGM_CONGESTION_MANAGEMENT_DB_SERVICE_POOL_MAXIMUM_THRESHOLDSr, reg, DB_SERVICE_POOL_1_TC_6_MAXIMUMLIMITf, field);
  SOC_REG_ABOVE_64_CLEAR(field);
  SHR_BITCOPY_RANGE(field, 0, &info->pool_tc[1][7].mc.buffers, 0, JER2_ARAD_EGQ_THRESHOLD_DBUFF_NOF_BITS);
  soc_reg_above_64_field_set(unit, ECGM_CONGESTION_MANAGEMENT_DB_SERVICE_POOL_MAXIMUM_THRESHOLDSr, reg, DB_SERVICE_POOL_1_TC_7_MAXIMUMLIMITf, field);
  DNXC_IF_ERR_EXIT(WRITE_ECGM_CONGESTION_MANAGEMENT_DB_SERVICE_POOL_MAXIMUM_THRESHOLDSr(unit,  reg));

  /* ECGM_ECGM_MC_RSVD_MAX_VALr */
  DNXC_IF_ERR_EXIT(READ_ECGM_CONGESTION_MANAGEMENT_MULTICAST_RESERVED_RESOURCESr(unit,  &reg_64));
  soc_reg64_field32_set(unit, ECGM_CONGESTION_MANAGEMENT_MULTICAST_RESERVED_RESOURCESr, &reg_64, RESERVED_P_DS_SERVICE_POOL_0f, info->pool[0].reserved.descriptors);
  soc_reg64_field32_set(unit, ECGM_CONGESTION_MANAGEMENT_MULTICAST_RESERVED_RESOURCESr, &reg_64, RESERVED_P_DS_SERVICE_POOL_1f, info->pool[1].reserved.descriptors);
  soc_reg64_field32_set(unit, ECGM_CONGESTION_MANAGEMENT_MULTICAST_RESERVED_RESOURCESr, &reg_64, RESERVED_D_BS_SERVICE_POOL_0f, info->pool[0].reserved.buffers);
  soc_reg64_field32_set(unit, ECGM_CONGESTION_MANAGEMENT_MULTICAST_RESERVED_RESOURCESr, &reg_64, RESERVED_D_BS_SERVICE_POOL_1f, info->pool[1].reserved.buffers);
  DNXC_IF_ERR_EXIT(WRITE_ECGM_CONGESTION_MANAGEMENT_MULTICAST_RESERVED_RESOURCESr(unit,  reg_64));

  /* ECGM_ECGM_MC_RSVD_DB_SP_THr */
  DNXC_IF_ERR_EXIT(READ_ECGM_CONGESTION_MANAGEMENT_MULTICAST_RESERVED_RESOURCES_PER_CLASSr(unit,  reg));
  SOC_REG_ABOVE_64_CLEAR(field);
  SHR_BITCOPY_RANGE(field, 0, &info->pool_tc[0][0].reserved.buffers, 0, JER2_ARAD_EGQ_THRESHOLD_DBUFF_NOF_BITS);
  soc_reg_above_64_field_set(unit, ECGM_CONGESTION_MANAGEMENT_MULTICAST_RESERVED_RESOURCES_PER_CLASSr, reg, RESERVED_DATA_BUFFERS_TC_0_SERVICE_POOL_0f, field);
  SOC_REG_ABOVE_64_CLEAR(field);
  SHR_BITCOPY_RANGE(field, 0, &info->pool_tc[0][1].reserved.buffers, 0, JER2_ARAD_EGQ_THRESHOLD_DBUFF_NOF_BITS);
  soc_reg_above_64_field_set(unit, ECGM_CONGESTION_MANAGEMENT_MULTICAST_RESERVED_RESOURCES_PER_CLASSr, reg, RESERVED_DATA_BUFFERS_TC_1_SERVICE_POOL_0f, field);
  SOC_REG_ABOVE_64_CLEAR(field);
  SHR_BITCOPY_RANGE(field, 0, &info->pool_tc[0][2].reserved.buffers, 0, JER2_ARAD_EGQ_THRESHOLD_DBUFF_NOF_BITS);
  soc_reg_above_64_field_set(unit, ECGM_CONGESTION_MANAGEMENT_MULTICAST_RESERVED_RESOURCES_PER_CLASSr, reg, RESERVED_DATA_BUFFERS_TC_2_SERVICE_POOL_0f, field);
  SOC_REG_ABOVE_64_CLEAR(field);
  SHR_BITCOPY_RANGE(field, 0, &info->pool_tc[0][3].reserved.buffers, 0, JER2_ARAD_EGQ_THRESHOLD_DBUFF_NOF_BITS);
  soc_reg_above_64_field_set(unit, ECGM_CONGESTION_MANAGEMENT_MULTICAST_RESERVED_RESOURCES_PER_CLASSr, reg, RESERVED_DATA_BUFFERS_TC_3_SERVICE_POOL_0f, field);
  SOC_REG_ABOVE_64_CLEAR(field);
  SHR_BITCOPY_RANGE(field, 0, &info->pool_tc[0][4].reserved.buffers, 0, JER2_ARAD_EGQ_THRESHOLD_DBUFF_NOF_BITS);
  soc_reg_above_64_field_set(unit, ECGM_CONGESTION_MANAGEMENT_MULTICAST_RESERVED_RESOURCES_PER_CLASSr, reg, RESERVED_DATA_BUFFERS_TC_4_SERVICE_POOL_0f, field);
  SOC_REG_ABOVE_64_CLEAR(field);
  SHR_BITCOPY_RANGE(field, 0, &info->pool_tc[0][5].reserved.buffers, 0, JER2_ARAD_EGQ_THRESHOLD_DBUFF_NOF_BITS);
  soc_reg_above_64_field_set(unit, ECGM_CONGESTION_MANAGEMENT_MULTICAST_RESERVED_RESOURCES_PER_CLASSr, reg, RESERVED_DATA_BUFFERS_TC_5_SERVICE_POOL_0f, field);
  SOC_REG_ABOVE_64_CLEAR(field);
  SHR_BITCOPY_RANGE(field, 0, &info->pool_tc[0][6].reserved.buffers, 0, JER2_ARAD_EGQ_THRESHOLD_DBUFF_NOF_BITS);
  soc_reg_above_64_field_set(unit, ECGM_CONGESTION_MANAGEMENT_MULTICAST_RESERVED_RESOURCES_PER_CLASSr, reg, RESERVED_DATA_BUFFERS_TC_6_SERVICE_POOL_0f, field);
  SOC_REG_ABOVE_64_CLEAR(field);
  SHR_BITCOPY_RANGE(field, 0, &info->pool_tc[0][7].reserved.buffers, 0, JER2_ARAD_EGQ_THRESHOLD_DBUFF_NOF_BITS);
  soc_reg_above_64_field_set(unit, ECGM_CONGESTION_MANAGEMENT_MULTICAST_RESERVED_RESOURCES_PER_CLASSr, reg, RESERVED_DATA_BUFFERS_TC_7_SERVICE_POOL_0f, field);
  SOC_REG_ABOVE_64_CLEAR(field);
  SHR_BITCOPY_RANGE(field, 0, &info->pool_tc[1][0].reserved.buffers, 0, JER2_ARAD_EGQ_THRESHOLD_DBUFF_NOF_BITS);
  soc_reg_above_64_field_set(unit, ECGM_CONGESTION_MANAGEMENT_MULTICAST_RESERVED_RESOURCES_PER_CLASSr, reg, RESERVED_DATA_BUFFERS_TC_0_SERVICE_POOL_1f, field);
  SOC_REG_ABOVE_64_CLEAR(field);
  SHR_BITCOPY_RANGE(field, 0, &info->pool_tc[1][1].reserved.buffers, 0, JER2_ARAD_EGQ_THRESHOLD_DBUFF_NOF_BITS);
  soc_reg_above_64_field_set(unit, ECGM_CONGESTION_MANAGEMENT_MULTICAST_RESERVED_RESOURCES_PER_CLASSr, reg, RESERVED_DATA_BUFFERS_TC_1_SERVICE_POOL_1f, field);
  SOC_REG_ABOVE_64_CLEAR(field);
  SHR_BITCOPY_RANGE(field, 0, &info->pool_tc[1][2].reserved.buffers, 0, JER2_ARAD_EGQ_THRESHOLD_DBUFF_NOF_BITS);
  soc_reg_above_64_field_set(unit, ECGM_CONGESTION_MANAGEMENT_MULTICAST_RESERVED_RESOURCES_PER_CLASSr, reg, RESERVED_DATA_BUFFERS_TC_2_SERVICE_POOL_1f, field);
  SOC_REG_ABOVE_64_CLEAR(field);
  SHR_BITCOPY_RANGE(field, 0, &info->pool_tc[1][3].reserved.buffers, 0, JER2_ARAD_EGQ_THRESHOLD_DBUFF_NOF_BITS);
  soc_reg_above_64_field_set(unit, ECGM_CONGESTION_MANAGEMENT_MULTICAST_RESERVED_RESOURCES_PER_CLASSr, reg, RESERVED_DATA_BUFFERS_TC_3_SERVICE_POOL_1f, field);
  SOC_REG_ABOVE_64_CLEAR(field);
  SHR_BITCOPY_RANGE(field, 0, &info->pool_tc[1][4].reserved.buffers, 0, JER2_ARAD_EGQ_THRESHOLD_DBUFF_NOF_BITS);
  soc_reg_above_64_field_set(unit, ECGM_CONGESTION_MANAGEMENT_MULTICAST_RESERVED_RESOURCES_PER_CLASSr, reg, RESERVED_DATA_BUFFERS_TC_4_SERVICE_POOL_1f, field);
  SOC_REG_ABOVE_64_CLEAR(field);
  SHR_BITCOPY_RANGE(field, 0, &info->pool_tc[1][5].reserved.buffers, 0, JER2_ARAD_EGQ_THRESHOLD_DBUFF_NOF_BITS);
  soc_reg_above_64_field_set(unit, ECGM_CONGESTION_MANAGEMENT_MULTICAST_RESERVED_RESOURCES_PER_CLASSr, reg, RESERVED_DATA_BUFFERS_TC_5_SERVICE_POOL_1f, field);
  SOC_REG_ABOVE_64_CLEAR(field);
  SHR_BITCOPY_RANGE(field, 0, &info->pool_tc[1][6].reserved.buffers, 0, JER2_ARAD_EGQ_THRESHOLD_DBUFF_NOF_BITS);
  soc_reg_above_64_field_set(unit, ECGM_CONGESTION_MANAGEMENT_MULTICAST_RESERVED_RESOURCES_PER_CLASSr, reg, RESERVED_DATA_BUFFERS_TC_6_SERVICE_POOL_1f, field);
  SOC_REG_ABOVE_64_CLEAR(field);
  SHR_BITCOPY_RANGE(field, 0, &info->pool_tc[1][7].reserved.buffers, 0, JER2_ARAD_EGQ_THRESHOLD_DBUFF_NOF_BITS);
  soc_reg_above_64_field_set(unit, ECGM_CONGESTION_MANAGEMENT_MULTICAST_RESERVED_RESOURCES_PER_CLASSr, reg, RESERVED_DATA_BUFFERS_TC_7_SERVICE_POOL_1f, field);
  DNXC_IF_ERR_EXIT(WRITE_ECGM_CONGESTION_MANAGEMENT_MULTICAST_RESERVED_RESOURCES_PER_CLASSr(unit,  reg));

  /* EGQ_PDCT_TABLEm */
  for(index = 0; index < JER2_ARAD_NOF_THRESH_TYPES; ++index) {
    DNXC_IF_ERR_EXIT(READ_EGQ_PDCT_TABLEm(unit, EGQ_BLOCK(unit, core), index, mem));
    field_32 = info->thresh_type[index].uc.descriptors;
    soc_EGQ_PDCT_TABLEm_field_set(unit, mem, PORT_UC_PD_DIS_THf, &field_32);
    field_32 = info->thresh_type[index].mc_shared.descriptors;
    soc_EGQ_PDCT_TABLEm_field_set(unit, mem, PORT_MC_PD_SHARED_MAX_THf, &field_32);
    DNXC_IF_ERR_EXIT(WRITE_EGQ_PDCT_TABLEm(unit, EGQ_BLOCK(unit, core), index, mem));
  }

  /* EGQ_PQST_TABLEm */
  for(index = 0; index < JER2_ARAD_NOF_THRESH_TYPES; ++index) {
    DNXC_IF_ERR_EXIT(READ_EGQ_PQST_TABLEm(unit, EGQ_BLOCK(unit, core), index, mem));
    field_32 = info->thresh_type[index].uc.buffers;
    soc_EGQ_PQST_TABLEm_field_set(unit, mem, PORT_UC_DB_DIS_THf, &field_32);
    field_32 = info->thresh_type[index].mc_shared.buffers;
    soc_EGQ_PQST_TABLEm_field_set(unit, mem, PORT_MC_DB_SHARED_THf, &field_32);
    DNXC_IF_ERR_EXIT(WRITE_EGQ_PQST_TABLEm(unit, EGQ_BLOCK(unit, core), index, mem));
  }
  /* EGQ_QDCT_TABLEm */
  for(index = 0; index < JER2_ARAD_NOF_THRESH_TYPES*JER2_ARAD_NOF_TRAFFIC_CLASSES; ++index) {
    if(info->thresh_type[index/JER2_ARAD_NOF_TRAFFIC_CLASSES].reserved[index%JER2_ARAD_NOF_TRAFFIC_CLASSES].descriptors > SOC_DNX_DEFS_GET(unit, egq_qdct_pd_max_val))
    {
      DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("")));
    }

    DNXC_IF_ERR_EXIT(READ_EGQ_QDCT_TABLEm(unit, EGQ_BLOCK(unit, core), index, mem));
    field_32 = info->thresh_type[index/JER2_ARAD_NOF_TRAFFIC_CLASSES].reserved[index%JER2_ARAD_NOF_TRAFFIC_CLASSES].descriptors;
    soc_EGQ_QDCT_TABLEm_field_set(unit, mem, QUEUE_MC_PD_RSVD_THf, &field_32);
    DNXC_IF_ERR_EXIT(WRITE_EGQ_QDCT_TABLEm(unit, EGQ_BLOCK(unit, core), index, mem));
  }


exit:
  DNXC_FUNC_RETURN;
}

int
  jer2_qax_egr_queuing_dev_get(
    DNX_SAND_IN    int    unit,
    DNX_SAND_IN    int    core,
    DNX_SAND_OUT          JER2_ARAD_EGR_QUEUING_DEV_TH    *info
  )
{
  soc_reg_above_64_val_t
    field,
    reg,
    mem;
  uint64
    reg_64;
  uint8
    index;

  DNXC_INIT_FUNC_DEFS;
  DNXC_NULL_CHECK(info);

  jer2_arad_JER2_ARAD_EGR_QUEUING_DEV_TH_clear(info);
  /* CGM_CGM_GENERAL_PD_THr */
  DNXC_IF_ERR_EXIT(READ_ECGM_CONGESTION_MANAGEMENT_GLOBAL_PD_THRESHOLDSr(unit, reg));
  soc_reg_above_64_field_get(unit, ECGM_CONGESTION_MANAGEMENT_GLOBAL_PD_THRESHOLDSr, reg, MAXIMUM_UNICAST_PACKET_DESCRIPTORSf, field);
  SHR_BITCOPY_RANGE(&info->global.uc.descriptors, 0, field, 0, JER2_ARAD_EGQ_THRESHOLD_PD_NOF_BITS);
  soc_reg_above_64_field_get(unit, ECGM_CONGESTION_MANAGEMENT_GLOBAL_PD_THRESHOLDSr, reg, MAXIMUM_MULTICAST_PACKET_DESCRIPTORSf, field);
  SHR_BITCOPY_RANGE(&info->global.mc.descriptors, 0, field, 0, JER2_ARAD_EGQ_THRESHOLD_PD_NOF_BITS);
  soc_reg_above_64_field_get(unit, ECGM_CONGESTION_MANAGEMENT_GLOBAL_PD_THRESHOLDSr, reg, MAXIMUM_TOTAL_PACKET_DESCRIPTORSf, field);
  SHR_BITCOPY_RANGE(&info->global.total.descriptors, 0, field, 0, JER2_ARAD_EGQ_THRESHOLD_PD_NOF_BITS);
  soc_reg_above_64_field_get(unit, ECGM_CONGESTION_MANAGEMENT_GLOBAL_PD_THRESHOLDSr, reg, SERVICE_POOL_0_MAXIMUM_PACKET_DESCRIPTORSf, field);
  SHR_BITCOPY_RANGE(&info->pool[0].mc.descriptors, 0, field, 0, JER2_ARAD_EGQ_THRESHOLD_PD_NOF_BITS);
  soc_reg_above_64_field_get(unit, ECGM_CONGESTION_MANAGEMENT_GLOBAL_PD_THRESHOLDSr, reg, SERVICE_POOL_1_MAXIMUM_PACKET_DESCRIPTORSf, field);
  SHR_BITCOPY_RANGE(&info->pool[1].mc.descriptors, 0, field, 0, JER2_ARAD_EGQ_THRESHOLD_PD_NOF_BITS);

  /* CGM_CGM_GENERAL_DB_THr */
  DNXC_IF_ERR_EXIT(READ_ECGM_CONGESTION_MANAGEMENT_GLOBAL_DB_THRESHOLDSr(unit, reg));
  soc_reg_above_64_field_get(unit, ECGM_CONGESTION_MANAGEMENT_GLOBAL_DB_THRESHOLDSr, reg, MAXIMUM_UNICAST_DATA_BUFFERSf, field);
  SHR_BITCOPY_RANGE(&info->global.uc.buffers, 0, field, 0, JER2_ARAD_EGQ_THRESHOLD_DBUFF_NOF_BITS);
  soc_reg_above_64_field_get(unit, ECGM_CONGESTION_MANAGEMENT_GLOBAL_DB_THRESHOLDSr, reg, MAXIMUM_MULTICAST_DATA_BUFFERSf, field);
  SHR_BITCOPY_RANGE(&info->global.mc.buffers, 0, field, 0, JER2_ARAD_EGQ_THRESHOLD_DBUFF_NOF_BITS);
  soc_reg_above_64_field_get(unit, ECGM_CONGESTION_MANAGEMENT_GLOBAL_DB_THRESHOLDSr, reg, MAXIMUM_TOTAL_DATA_BUFFERSf, field);
  SHR_BITCOPY_RANGE(&info->global.total.buffers, 0, field, 0, JER2_ARAD_EGQ_THRESHOLD_DBUFF_NOF_BITS);
  soc_reg_above_64_field_get(unit, ECGM_CONGESTION_MANAGEMENT_GLOBAL_DB_THRESHOLDSr, reg, SERVICE_POOL_0_MAXIMUM_DATA_BUFFERSf, field);
  SHR_BITCOPY_RANGE(&info->pool[0].mc.buffers, 0, field, 0, JER2_ARAD_EGQ_THRESHOLD_DBUFF_NOF_BITS);
  soc_reg_above_64_field_get(unit, ECGM_CONGESTION_MANAGEMENT_GLOBAL_DB_THRESHOLDSr, reg, SERVICE_POOL_1_MAXIMUM_DATA_BUFFERSf, field);
  SHR_BITCOPY_RANGE(&info->pool[1].mc.buffers, 0, field, 0, JER2_ARAD_EGQ_THRESHOLD_DBUFF_NOF_BITS);

  /* CGM_CGM_MC_PD_SP_TC_THr */
  DNXC_IF_ERR_EXIT(READ_ECGM_CONGESTION_MANAGEMENT_PD_SERVICE_POOL_THRESHOLDSr(unit, reg));
  soc_reg_above_64_field_get(unit, ECGM_CONGESTION_MANAGEMENT_PD_SERVICE_POOL_THRESHOLDSr, reg, PD_SERVICE_POOL_0_TC_0_MAXIMUMLIMITf, field);
  SHR_BITCOPY_RANGE(&info->pool_tc[0][0].mc.descriptors, 0, field, 0, JER2_ARAD_EGQ_THRESHOLD_PD_NOF_BITS);
  soc_reg_above_64_field_get(unit, ECGM_CONGESTION_MANAGEMENT_PD_SERVICE_POOL_THRESHOLDSr, reg, PD_SERVICE_POOL_0_TC_1_MAXIMUMLIMITf, field);
  SHR_BITCOPY_RANGE(&info->pool_tc[0][1].mc.descriptors, 0, field, 0, JER2_ARAD_EGQ_THRESHOLD_PD_NOF_BITS);
  soc_reg_above_64_field_get(unit, ECGM_CONGESTION_MANAGEMENT_PD_SERVICE_POOL_THRESHOLDSr, reg, PD_SERVICE_POOL_0_TC_2_MAXIMUMLIMITf, field);
  SHR_BITCOPY_RANGE(&info->pool_tc[0][2].mc.descriptors, 0, field, 0, JER2_ARAD_EGQ_THRESHOLD_PD_NOF_BITS);
  soc_reg_above_64_field_get(unit, ECGM_CONGESTION_MANAGEMENT_PD_SERVICE_POOL_THRESHOLDSr, reg, PD_SERVICE_POOL_0_TC_3_MAXIMUMLIMITf, field);
  SHR_BITCOPY_RANGE(&info->pool_tc[0][3].mc.descriptors, 0, field, 0, JER2_ARAD_EGQ_THRESHOLD_PD_NOF_BITS);
  soc_reg_above_64_field_get(unit, ECGM_CONGESTION_MANAGEMENT_PD_SERVICE_POOL_THRESHOLDSr, reg, PD_SERVICE_POOL_0_TC_4_MAXIMUMLIMITf, field);
  SHR_BITCOPY_RANGE(&info->pool_tc[0][4].mc.descriptors, 0, field, 0, JER2_ARAD_EGQ_THRESHOLD_PD_NOF_BITS);
  soc_reg_above_64_field_get(unit, ECGM_CONGESTION_MANAGEMENT_PD_SERVICE_POOL_THRESHOLDSr, reg, PD_SERVICE_POOL_0_TC_5_MAXIMUMLIMITf, field);
  SHR_BITCOPY_RANGE(&info->pool_tc[0][5].mc.descriptors, 0, field, 0, JER2_ARAD_EGQ_THRESHOLD_PD_NOF_BITS);
  soc_reg_above_64_field_get(unit, ECGM_CONGESTION_MANAGEMENT_PD_SERVICE_POOL_THRESHOLDSr, reg, PD_SERVICE_POOL_0_TC_6_MAXIMUMLIMITf, field);
  SHR_BITCOPY_RANGE(&info->pool_tc[0][6].mc.descriptors, 0, field, 0, JER2_ARAD_EGQ_THRESHOLD_PD_NOF_BITS);
  soc_reg_above_64_field_get(unit, ECGM_CONGESTION_MANAGEMENT_PD_SERVICE_POOL_THRESHOLDSr, reg, PD_SERVICE_POOL_0_TC_7_MAXIMUMLIMITf, field);
  SHR_BITCOPY_RANGE(&info->pool_tc[0][7].mc.descriptors, 0, field, 0, JER2_ARAD_EGQ_THRESHOLD_PD_NOF_BITS);
  soc_reg_above_64_field_get(unit, ECGM_CONGESTION_MANAGEMENT_PD_SERVICE_POOL_THRESHOLDSr, reg, PD_SERVICE_POOL_1_TC_0_MAXIMUMLIMITf, field);
  SHR_BITCOPY_RANGE(&info->pool_tc[1][0].mc.descriptors, 0, field, 0, JER2_ARAD_EGQ_THRESHOLD_PD_NOF_BITS);
  soc_reg_above_64_field_get(unit, ECGM_CONGESTION_MANAGEMENT_PD_SERVICE_POOL_THRESHOLDSr, reg, PD_SERVICE_POOL_1_TC_1_MAXIMUMLIMITf, field);
  SHR_BITCOPY_RANGE(&info->pool_tc[1][1].mc.descriptors, 0, field, 0, JER2_ARAD_EGQ_THRESHOLD_PD_NOF_BITS);
  soc_reg_above_64_field_get(unit, ECGM_CONGESTION_MANAGEMENT_PD_SERVICE_POOL_THRESHOLDSr, reg, PD_SERVICE_POOL_1_TC_2_MAXIMUMLIMITf, field);
  SHR_BITCOPY_RANGE(&info->pool_tc[1][2].mc.descriptors, 0, field, 0, JER2_ARAD_EGQ_THRESHOLD_PD_NOF_BITS);
  soc_reg_above_64_field_get(unit, ECGM_CONGESTION_MANAGEMENT_PD_SERVICE_POOL_THRESHOLDSr, reg, PD_SERVICE_POOL_1_TC_3_MAXIMUMLIMITf, field);
  SHR_BITCOPY_RANGE(&info->pool_tc[1][3].mc.descriptors, 0, field, 0, JER2_ARAD_EGQ_THRESHOLD_PD_NOF_BITS);
  soc_reg_above_64_field_get(unit, ECGM_CONGESTION_MANAGEMENT_PD_SERVICE_POOL_THRESHOLDSr, reg, PD_SERVICE_POOL_1_TC_4_MAXIMUMLIMITf, field);
  SHR_BITCOPY_RANGE(&info->pool_tc[1][4].mc.descriptors, 0, field, 0, JER2_ARAD_EGQ_THRESHOLD_PD_NOF_BITS);
  soc_reg_above_64_field_get(unit, ECGM_CONGESTION_MANAGEMENT_PD_SERVICE_POOL_THRESHOLDSr, reg, PD_SERVICE_POOL_1_TC_5_MAXIMUMLIMITf, field);
  SHR_BITCOPY_RANGE(&info->pool_tc[1][5].mc.descriptors, 0, field, 0, JER2_ARAD_EGQ_THRESHOLD_PD_NOF_BITS);
  soc_reg_above_64_field_get(unit, ECGM_CONGESTION_MANAGEMENT_PD_SERVICE_POOL_THRESHOLDSr, reg, PD_SERVICE_POOL_1_TC_6_MAXIMUMLIMITf, field);
  SHR_BITCOPY_RANGE(&info->pool_tc[1][6].mc.descriptors, 0, field, 0, JER2_ARAD_EGQ_THRESHOLD_PD_NOF_BITS);
  soc_reg_above_64_field_get(unit, ECGM_CONGESTION_MANAGEMENT_PD_SERVICE_POOL_THRESHOLDSr, reg, PD_SERVICE_POOL_1_TC_7_MAXIMUMLIMITf, field);
  SHR_BITCOPY_RANGE(&info->pool_tc[1][7].mc.descriptors, 0, field, 0, JER2_ARAD_EGQ_THRESHOLD_PD_NOF_BITS);

  /* CGM_CGM_MC_DB_SP_TC_THr */
  DNXC_IF_ERR_EXIT(READ_ECGM_CONGESTION_MANAGEMENT_DB_SERVICE_POOL_MAXIMUM_THRESHOLDSr(unit, reg));
  soc_reg_above_64_field_get(unit, ECGM_CONGESTION_MANAGEMENT_DB_SERVICE_POOL_MAXIMUM_THRESHOLDSr, reg, DB_SERVICE_POOL_0_TC_0_MAXIMUMLIMITf, field);
  SHR_BITCOPY_RANGE(&info->pool_tc[0][0].mc.buffers, 0, field, 0, JER2_ARAD_EGQ_THRESHOLD_DBUFF_NOF_BITS);
  soc_reg_above_64_field_get(unit, ECGM_CONGESTION_MANAGEMENT_DB_SERVICE_POOL_MAXIMUM_THRESHOLDSr, reg, DB_SERVICE_POOL_0_TC_1_MAXIMUMLIMITf, field);
  SHR_BITCOPY_RANGE(&info->pool_tc[0][1].mc.buffers, 0, field, 0, JER2_ARAD_EGQ_THRESHOLD_DBUFF_NOF_BITS);
  soc_reg_above_64_field_get(unit, ECGM_CONGESTION_MANAGEMENT_DB_SERVICE_POOL_MAXIMUM_THRESHOLDSr, reg, DB_SERVICE_POOL_0_TC_2_MAXIMUMLIMITf, field);
  SHR_BITCOPY_RANGE(&info->pool_tc[0][2].mc.buffers, 0, field, 0, JER2_ARAD_EGQ_THRESHOLD_DBUFF_NOF_BITS);
  soc_reg_above_64_field_get(unit, ECGM_CONGESTION_MANAGEMENT_DB_SERVICE_POOL_MAXIMUM_THRESHOLDSr, reg, DB_SERVICE_POOL_0_TC_3_MAXIMUMLIMITf, field);
  SHR_BITCOPY_RANGE(&info->pool_tc[0][3].mc.buffers, 0, field, 0, JER2_ARAD_EGQ_THRESHOLD_DBUFF_NOF_BITS);
  soc_reg_above_64_field_get(unit, ECGM_CONGESTION_MANAGEMENT_DB_SERVICE_POOL_MAXIMUM_THRESHOLDSr, reg, DB_SERVICE_POOL_0_TC_4_MAXIMUMLIMITf, field);
  SHR_BITCOPY_RANGE(&info->pool_tc[0][4].mc.buffers, 0, field, 0, JER2_ARAD_EGQ_THRESHOLD_DBUFF_NOF_BITS);
  soc_reg_above_64_field_get(unit, ECGM_CONGESTION_MANAGEMENT_DB_SERVICE_POOL_MAXIMUM_THRESHOLDSr, reg, DB_SERVICE_POOL_0_TC_5_MAXIMUMLIMITf, field);
  SHR_BITCOPY_RANGE(&info->pool_tc[0][5].mc.buffers, 0, field, 0, JER2_ARAD_EGQ_THRESHOLD_DBUFF_NOF_BITS);
  soc_reg_above_64_field_get(unit, ECGM_CONGESTION_MANAGEMENT_DB_SERVICE_POOL_MAXIMUM_THRESHOLDSr, reg, DB_SERVICE_POOL_0_TC_6_MAXIMUMLIMITf, field);
  SHR_BITCOPY_RANGE(&info->pool_tc[0][6].mc.buffers, 0, field, 0, JER2_ARAD_EGQ_THRESHOLD_DBUFF_NOF_BITS);
  soc_reg_above_64_field_get(unit, ECGM_CONGESTION_MANAGEMENT_DB_SERVICE_POOL_MAXIMUM_THRESHOLDSr, reg, DB_SERVICE_POOL_0_TC_7_MAXIMUMLIMITf, field);
  SHR_BITCOPY_RANGE(&info->pool_tc[0][7].mc.buffers, 0, field, 0, JER2_ARAD_EGQ_THRESHOLD_DBUFF_NOF_BITS);
  soc_reg_above_64_field_get(unit, ECGM_CONGESTION_MANAGEMENT_DB_SERVICE_POOL_MAXIMUM_THRESHOLDSr, reg, DB_SERVICE_POOL_1_TC_0_MAXIMUMLIMITf, field);
  SHR_BITCOPY_RANGE(&info->pool_tc[1][0].mc.buffers, 0, field, 0, JER2_ARAD_EGQ_THRESHOLD_DBUFF_NOF_BITS);
  soc_reg_above_64_field_get(unit, ECGM_CONGESTION_MANAGEMENT_DB_SERVICE_POOL_MAXIMUM_THRESHOLDSr, reg, DB_SERVICE_POOL_1_TC_1_MAXIMUMLIMITf, field);
  SHR_BITCOPY_RANGE(&info->pool_tc[1][1].mc.buffers, 0, field, 0, JER2_ARAD_EGQ_THRESHOLD_DBUFF_NOF_BITS);
  soc_reg_above_64_field_get(unit, ECGM_CONGESTION_MANAGEMENT_DB_SERVICE_POOL_MAXIMUM_THRESHOLDSr, reg, DB_SERVICE_POOL_1_TC_2_MAXIMUMLIMITf, field);
  SHR_BITCOPY_RANGE(&info->pool_tc[1][2].mc.buffers, 0, field, 0, JER2_ARAD_EGQ_THRESHOLD_DBUFF_NOF_BITS);
  soc_reg_above_64_field_get(unit, ECGM_CONGESTION_MANAGEMENT_DB_SERVICE_POOL_MAXIMUM_THRESHOLDSr, reg, DB_SERVICE_POOL_1_TC_3_MAXIMUMLIMITf, field);
  SHR_BITCOPY_RANGE(&info->pool_tc[1][3].mc.buffers, 0, field, 0, JER2_ARAD_EGQ_THRESHOLD_DBUFF_NOF_BITS);
  soc_reg_above_64_field_get(unit, ECGM_CONGESTION_MANAGEMENT_DB_SERVICE_POOL_MAXIMUM_THRESHOLDSr, reg, DB_SERVICE_POOL_1_TC_4_MAXIMUMLIMITf, field);
  SHR_BITCOPY_RANGE(&info->pool_tc[1][4].mc.buffers, 0, field, 0, JER2_ARAD_EGQ_THRESHOLD_DBUFF_NOF_BITS);
  soc_reg_above_64_field_get(unit, ECGM_CONGESTION_MANAGEMENT_DB_SERVICE_POOL_MAXIMUM_THRESHOLDSr, reg, DB_SERVICE_POOL_1_TC_5_MAXIMUMLIMITf, field);
  SHR_BITCOPY_RANGE(&info->pool_tc[1][5].mc.buffers, 0, field, 0, JER2_ARAD_EGQ_THRESHOLD_DBUFF_NOF_BITS);
  soc_reg_above_64_field_get(unit, ECGM_CONGESTION_MANAGEMENT_DB_SERVICE_POOL_MAXIMUM_THRESHOLDSr, reg, DB_SERVICE_POOL_1_TC_6_MAXIMUMLIMITf, field);
  SHR_BITCOPY_RANGE(&info->pool_tc[1][6].mc.buffers, 0, field, 0, JER2_ARAD_EGQ_THRESHOLD_DBUFF_NOF_BITS);
  soc_reg_above_64_field_get(unit, ECGM_CONGESTION_MANAGEMENT_DB_SERVICE_POOL_MAXIMUM_THRESHOLDSr, reg, DB_SERVICE_POOL_1_TC_7_MAXIMUMLIMITf, field);
  SHR_BITCOPY_RANGE(&info->pool_tc[1][7].mc.buffers, 0, field, 0, JER2_ARAD_EGQ_THRESHOLD_DBUFF_NOF_BITS);

  /* CGM_CGM_MC_RSVD_MAX_VALr */
  DNXC_IF_ERR_EXIT(READ_ECGM_CONGESTION_MANAGEMENT_MULTICAST_RESERVED_RESOURCESr(unit, &reg_64));
  info->pool[0].reserved.descriptors = soc_reg64_field32_get(unit, ECGM_CONGESTION_MANAGEMENT_MULTICAST_RESERVED_RESOURCESr, reg_64, RESERVED_P_DS_SERVICE_POOL_0f);
  info->pool[1].reserved.descriptors = soc_reg64_field32_get(unit, ECGM_CONGESTION_MANAGEMENT_MULTICAST_RESERVED_RESOURCESr, reg_64, RESERVED_P_DS_SERVICE_POOL_1f);
  info->pool[0].reserved.buffers = soc_reg64_field32_get(unit, ECGM_CONGESTION_MANAGEMENT_MULTICAST_RESERVED_RESOURCESr, reg_64, RESERVED_D_BS_SERVICE_POOL_0f);
  info->pool[1].reserved.buffers = soc_reg64_field32_get(unit, ECGM_CONGESTION_MANAGEMENT_MULTICAST_RESERVED_RESOURCESr, reg_64, RESERVED_D_BS_SERVICE_POOL_1f);

  /* CGM_CGM_MC_RSVD_DB_SP_THr */
  DNXC_IF_ERR_EXIT(READ_ECGM_CONGESTION_MANAGEMENT_MULTICAST_RESERVED_RESOURCES_PER_CLASSr(unit, reg));
  soc_reg_above_64_field_get(unit, ECGM_CONGESTION_MANAGEMENT_MULTICAST_RESERVED_RESOURCES_PER_CLASSr, reg, RESERVED_DATA_BUFFERS_TC_0_SERVICE_POOL_0f, field);
  SHR_BITCOPY_RANGE(&info->pool_tc[0][0].reserved.buffers, 0, field, 0, JER2_ARAD_EGQ_THRESHOLD_DBUFF_NOF_BITS);
  soc_reg_above_64_field_get(unit, ECGM_CONGESTION_MANAGEMENT_MULTICAST_RESERVED_RESOURCES_PER_CLASSr, reg, RESERVED_DATA_BUFFERS_TC_1_SERVICE_POOL_0f, field);
  SHR_BITCOPY_RANGE(&info->pool_tc[0][1].reserved.buffers, 0, field, 0, JER2_ARAD_EGQ_THRESHOLD_DBUFF_NOF_BITS);
  soc_reg_above_64_field_get(unit, ECGM_CONGESTION_MANAGEMENT_MULTICAST_RESERVED_RESOURCES_PER_CLASSr, reg, RESERVED_DATA_BUFFERS_TC_2_SERVICE_POOL_0f, field);
  SHR_BITCOPY_RANGE(&info->pool_tc[0][2].reserved.buffers, 0, field, 0, JER2_ARAD_EGQ_THRESHOLD_DBUFF_NOF_BITS);
  soc_reg_above_64_field_get(unit, ECGM_CONGESTION_MANAGEMENT_MULTICAST_RESERVED_RESOURCES_PER_CLASSr, reg, RESERVED_DATA_BUFFERS_TC_3_SERVICE_POOL_0f, field);
  SHR_BITCOPY_RANGE(&info->pool_tc[0][3].reserved.buffers, 0, field, 0, JER2_ARAD_EGQ_THRESHOLD_DBUFF_NOF_BITS);
  soc_reg_above_64_field_get(unit, ECGM_CONGESTION_MANAGEMENT_MULTICAST_RESERVED_RESOURCES_PER_CLASSr, reg, RESERVED_DATA_BUFFERS_TC_4_SERVICE_POOL_0f, field);
  SHR_BITCOPY_RANGE(&info->pool_tc[0][4].reserved.buffers, 0, field, 0, JER2_ARAD_EGQ_THRESHOLD_DBUFF_NOF_BITS);
  soc_reg_above_64_field_get(unit, ECGM_CONGESTION_MANAGEMENT_MULTICAST_RESERVED_RESOURCES_PER_CLASSr, reg, RESERVED_DATA_BUFFERS_TC_5_SERVICE_POOL_0f, field);
  SHR_BITCOPY_RANGE(&info->pool_tc[0][5].reserved.buffers, 0, field, 0, JER2_ARAD_EGQ_THRESHOLD_DBUFF_NOF_BITS);
  soc_reg_above_64_field_get(unit, ECGM_CONGESTION_MANAGEMENT_MULTICAST_RESERVED_RESOURCES_PER_CLASSr, reg, RESERVED_DATA_BUFFERS_TC_6_SERVICE_POOL_0f, field);
  SHR_BITCOPY_RANGE(&info->pool_tc[0][6].reserved.buffers, 0, field, 0, JER2_ARAD_EGQ_THRESHOLD_DBUFF_NOF_BITS);
  soc_reg_above_64_field_get(unit, ECGM_CONGESTION_MANAGEMENT_MULTICAST_RESERVED_RESOURCES_PER_CLASSr, reg, RESERVED_DATA_BUFFERS_TC_7_SERVICE_POOL_0f, field);
  SHR_BITCOPY_RANGE(&info->pool_tc[0][7].reserved.buffers, 0, field, 0, JER2_ARAD_EGQ_THRESHOLD_DBUFF_NOF_BITS);
  soc_reg_above_64_field_get(unit, ECGM_CONGESTION_MANAGEMENT_MULTICAST_RESERVED_RESOURCES_PER_CLASSr, reg, RESERVED_DATA_BUFFERS_TC_0_SERVICE_POOL_1f, field);
  SHR_BITCOPY_RANGE(&info->pool_tc[1][0].reserved.buffers, 0, field, 0, JER2_ARAD_EGQ_THRESHOLD_DBUFF_NOF_BITS);
  soc_reg_above_64_field_get(unit, ECGM_CONGESTION_MANAGEMENT_MULTICAST_RESERVED_RESOURCES_PER_CLASSr, reg, RESERVED_DATA_BUFFERS_TC_1_SERVICE_POOL_1f, field);
  SHR_BITCOPY_RANGE(&info->pool_tc[1][1].reserved.buffers, 0, field, 0, JER2_ARAD_EGQ_THRESHOLD_DBUFF_NOF_BITS);
  soc_reg_above_64_field_get(unit, ECGM_CONGESTION_MANAGEMENT_MULTICAST_RESERVED_RESOURCES_PER_CLASSr, reg, RESERVED_DATA_BUFFERS_TC_2_SERVICE_POOL_1f, field);
  SHR_BITCOPY_RANGE(&info->pool_tc[1][2].reserved.buffers, 0, field, 0, JER2_ARAD_EGQ_THRESHOLD_DBUFF_NOF_BITS);
  soc_reg_above_64_field_get(unit, ECGM_CONGESTION_MANAGEMENT_MULTICAST_RESERVED_RESOURCES_PER_CLASSr, reg, RESERVED_DATA_BUFFERS_TC_3_SERVICE_POOL_1f, field);
  SHR_BITCOPY_RANGE(&info->pool_tc[1][3].reserved.buffers, 0, field, 0, JER2_ARAD_EGQ_THRESHOLD_DBUFF_NOF_BITS);
  soc_reg_above_64_field_get(unit, ECGM_CONGESTION_MANAGEMENT_MULTICAST_RESERVED_RESOURCES_PER_CLASSr, reg, RESERVED_DATA_BUFFERS_TC_4_SERVICE_POOL_1f, field);
  SHR_BITCOPY_RANGE(&info->pool_tc[1][4].reserved.buffers, 0, field, 0, JER2_ARAD_EGQ_THRESHOLD_DBUFF_NOF_BITS);
  soc_reg_above_64_field_get(unit, ECGM_CONGESTION_MANAGEMENT_MULTICAST_RESERVED_RESOURCES_PER_CLASSr, reg, RESERVED_DATA_BUFFERS_TC_5_SERVICE_POOL_1f, field);
  SHR_BITCOPY_RANGE(&info->pool_tc[1][5].reserved.buffers, 0, field, 0, JER2_ARAD_EGQ_THRESHOLD_DBUFF_NOF_BITS);
  soc_reg_above_64_field_get(unit, ECGM_CONGESTION_MANAGEMENT_MULTICAST_RESERVED_RESOURCES_PER_CLASSr, reg, RESERVED_DATA_BUFFERS_TC_6_SERVICE_POOL_1f, field);
  SHR_BITCOPY_RANGE(&info->pool_tc[1][6].reserved.buffers, 0, field, 0, JER2_ARAD_EGQ_THRESHOLD_DBUFF_NOF_BITS);
  soc_reg_above_64_field_get(unit, ECGM_CONGESTION_MANAGEMENT_MULTICAST_RESERVED_RESOURCES_PER_CLASSr, reg, RESERVED_DATA_BUFFERS_TC_7_SERVICE_POOL_1f, field);
  SHR_BITCOPY_RANGE(&info->pool_tc[1][7].reserved.buffers, 0, field, 0, JER2_ARAD_EGQ_THRESHOLD_DBUFF_NOF_BITS);

  /* EGQ_PDCT_TABLEm */
  for(index = 0; index < JER2_ARAD_NOF_THRESH_TYPES; ++index) {
    DNXC_IF_ERR_EXIT(READ_EGQ_PDCT_TABLEm(unit, EGQ_BLOCK(unit,core), index, mem));
    soc_EGQ_PDCT_TABLEm_field_get(unit, mem, PORT_UC_PD_DIS_THf, &info->thresh_type[index].uc.descriptors);
    soc_EGQ_PDCT_TABLEm_field_get(unit, mem, PORT_MC_PD_SHARED_MAX_THf, &info->thresh_type[index].mc_shared.descriptors);
  }


  /* EGQ_PQST_TABLEm */
  for(index = 0; index < JER2_ARAD_NOF_THRESH_TYPES; ++index) {
    DNXC_IF_ERR_EXIT(READ_EGQ_PQST_TABLEm(unit, EGQ_BLOCK(unit, core), index, mem));
    soc_EGQ_PQST_TABLEm_field_get(unit, mem, PORT_UC_DB_DIS_THf, &info->thresh_type[index].uc.buffers);
    soc_EGQ_PQST_TABLEm_field_get(unit, mem, PORT_MC_DB_SHARED_THf, &info->thresh_type[index].mc_shared.buffers);
  }
  /* EGQ_QDCT_TABLEm */
  for(index = 0; index < JER2_ARAD_NOF_THRESH_TYPES*JER2_ARAD_NOF_TRAFFIC_CLASSES; ++index) {

    DNXC_IF_ERR_EXIT(READ_EGQ_QDCT_TABLEm(unit, EGQ_BLOCK(unit, core), index, mem));
    soc_EGQ_QDCT_TABLEm_field_get(
                                    unit,
                                    mem,
                                    QUEUE_MC_PD_RSVD_THf,
                                    &info->thresh_type[index/JER2_ARAD_NOF_TRAFFIC_CLASSES].reserved[index%JER2_ARAD_NOF_TRAFFIC_CLASSES].descriptors
                                  );
  }


exit:
  DNXC_FUNC_RETURN;
}

int
  jer2_qax_egr_queuing_mc_cos_map_set(
    DNX_SAND_IN    int    unit,
    DNX_SAND_IN    int    core,
    DNX_SAND_IN    uint32    tc_ndx,
    DNX_SAND_IN    uint32    dp_ndx,
    DNX_SAND_IN JER2_ARAD_EGR_QUEUING_MC_COS_MAP    *info
  )
{
  uint32
    reg[1],
    field;
  uint8
    index;
  DNXC_INIT_FUNC_DEFS;
  DNXC_NULL_CHECK(info);

  if (tc_ndx > JER2_ARAD_EGR_Q_PRIORITY_TC_MAX) {
      DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("")));
  }
  if (dp_ndx > JER2_ARAD_EGR_Q_PRIORITY_DP_MAX) {
      DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("")));
  }

  /* ECGM_ECGM_MAP_TC_TO_SPr */
  DNXC_IF_ERR_EXIT(READ_ECGM_CGM_MAP_TC_TO_SPr(unit, reg));
  if(info->pool_id) {
    SHR_BITSET(reg, info->tc_group);
  }
  else
  {
    SHR_BITCLR(reg, info->tc_group);
  }
  DNXC_IF_ERR_EXIT(WRITE_ECGM_CGM_MAP_TC_TO_SPr(unit, *reg));

  /* EGQ_MC_SP_TC_MAPm */
  index = 0;
  index |= (dp_ndx);
  index |= (tc_ndx)<<2;

  DNXC_IF_ERR_EXIT(READ_EGQ_MC_SP_TC_MAPm(unit, EGQ_BLOCK(unit, core), index, reg));
  field = info->tc_group;
  soc_EGQ_MC_SP_TC_MAPm_field_set(unit, reg, CGM_MC_TCf, &field);
  /* CGM MC SE equals to DP index. CGM_MC_SE is being used later for CGM_DP_ELIGIBLE_TO_USE_RESOURCESr */
  field = dp_ndx;
  soc_EGQ_MC_SP_TC_MAPm_field_set(unit, reg, CGM_MC_SEf, &field);
  field = info->pool_id;
  soc_EGQ_MC_SP_TC_MAPm_field_set(unit, reg, CGM_MC_SPf, &field);
  DNXC_IF_ERR_EXIT(WRITE_EGQ_MC_SP_TC_MAPm(unit, EGQ_BLOCK(unit, core), index, reg));

  /* EGQ_MC_PRIORITY_LOOKUP_TABLEr */
  index = ((tc_ndx)<<2) | dp_ndx;
  *reg = 0;
  DNXC_IF_ERR_EXIT(READ_EGQ_MC_PRIORITY_LOOKUP_TABLEr(unit, core, reg));

  if(!info->pool_id) {
    SHR_BITSET(reg, index);
  }
  else
  {
    SHR_BITCLR(reg, index);
  }

  DNXC_IF_ERR_EXIT(WRITE_EGQ_MC_PRIORITY_LOOKUP_TABLEr(unit, core, *reg));
  /* ECGM_DP_ELIGIBLE_TO_USE_RESOURCESr */
  DNXC_IF_ERR_EXIT(READ_ECGM_CGM_DP_ELIGIBLE_TO_USE_RESOURCESr(unit, reg));
  index = ((info->pool_id) << 2) | dp_ndx;
  if(info->pool_eligibility) {
    SHR_BITSET(reg, index);
  }
  else
  {
    SHR_BITCLR(reg, index);
  }
  DNXC_IF_ERR_EXIT(WRITE_ECGM_CGM_DP_ELIGIBLE_TO_USE_RESOURCESr(unit, *reg));




exit:
  DNXC_FUNC_RETURN;
}

int
  jer2_qax_egr_queuing_if_fc_set(
    DNX_SAND_IN    int    unit,
    DNX_SAND_IN    JER2_ARAD_INTERFACE_ID    if_ndx,
    DNX_SAND_IN          JER2_ARAD_EGR_QUEUING_IF_FC    *info
  )
{
  uint32
    if_internal_id = 0;
  soc_reg_above_64_val_t
    reg_above_64;
  uint32 
    nof_if_to_be_set = 1;
  uint32
      i;

  DNXC_INIT_FUNC_DEFS;
  DNXC_NULL_CHECK(info);

  
  DNXC_LEGACY_FIXME_ASSERT;
#ifdef FIXME_DNX_LEGACY
  if_internal_id = jer2_arad_nif2intern_id(unit, if_ndx);
  nif_type = jer2_arad_nif_id2type(if_ndx);
  ilkn_tdm_dedicated_queuing = SOC_DNX_CONFIG(unit)->jer2_arad->init.ilkn_tdm_dedicated_queuing;

  if ((nif_type == JER2_ARAD_NIF_TYPE_ILKN) && (ilkn_tdm_dedicated_queuing == JER2_ARAD_MGMT_ILKN_TDM_DEDICATED_QUEUING_MODE_ON)) {
      nof_if_to_be_set = 2;
  }

  if(if_internal_id == JER2_ARAD_NIF_ID_NONE)
  {
    DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("")));
  }
#endif 
  for (i=0; i <  nof_if_to_be_set; ++i,++if_internal_id) {
        DNXC_IF_ERR_EXIT(READ_ECGM_CGM_MAP_IF_2_THr(unit, reg_above_64));
        SHR_BITCOPY_RANGE(reg_above_64, if_internal_id*3, &info->uc_profile, 0, 3);
        DNXC_IF_ERR_EXIT(WRITE_ECGM_CGM_MAP_IF_2_THr(unit, reg_above_64));

        DNXC_IF_ERR_EXIT(READ_ECGM_CGM_MC_INTERFACE_MAP_THr(unit, reg_above_64));
        if(if_internal_id*2 > 31) {
        reg_above_64[1] |= (info->mc_pd_profile & 0x3) << (2 * if_internal_id - 32);
        }
        else
        {
        reg_above_64[0] |= (info->mc_pd_profile & 0x3) << (2 * if_internal_id);
        }
        DNXC_IF_ERR_EXIT(WRITE_ECGM_CGM_MC_INTERFACE_MAP_THr(unit, reg_above_64));
  }


exit:
  DNXC_FUNC_RETURN;
}


int
  jer2_qax_egr_queuing_if_fc_uc_get(
    DNX_SAND_IN    int    unit,
    DNX_SAND_IN    int    core,
    DNX_SAND_IN    uint32    uc_if_profile_ndx,
    DNX_SAND_OUT          JER2_ARAD_EGR_QUEUING_IF_UC_FC    *info
  )
{
  soc_reg_above_64_val_t
    pd,
	pd_min,
	pd_alpha,
    size_256,
	size_256_min,
	size_256_alpha;
	
	uint32  pd_field = 0,
			pd_min_field = 0,
			pd_alpha_field = 0,
			size_256_field = 0,
			size_256_min_field = 0,
			size_256_alpha_field = 0;
	
   int alpha;

  DNXC_INIT_FUNC_DEFS;
  DNXC_NULL_CHECK(info);

  jer2_arad_JER2_ARAD_EGR_QUEUING_IF_UC_FC_clear(info);
  /* ECGM_CGM_UC_PD_INTERFACE_FC_THr */
  DNXC_IF_ERR_EXIT(READ_ECGM_CGM_UC_PD_INTERFACE_FC_MAX_THr(unit, pd));
  DNXC_IF_ERR_EXIT(READ_ECGM_CGM_UC_PD_INTERFACE_FC_MIN_THr(unit, pd_min));
  DNXC_IF_ERR_EXIT(READ_ECGM_CGM_UC_PD_INTERFACE_FC_ALPHAr(unit, pd_alpha));
  /* ECGM_CGM_UC_SIZE_256_INTERFACE_FC_THr */
  DNXC_IF_ERR_EXIT(READ_ECGM_CGM_UC_SIZE_256_INTERFACE_FC_MAX_THr(unit, size_256));
  DNXC_IF_ERR_EXIT(READ_ECGM_CGM_UC_SIZE_256_INTERFACE_FC_MIN_THr(unit, size_256_min));
  DNXC_IF_ERR_EXIT(READ_ECGM_CGM_UC_SIZE_256_INTERFACE_FC_ALPHAr(unit, size_256_alpha));
  
  switch(uc_if_profile_ndx) {
    case 0:
      pd_field = soc_reg_above_64_field32_get(unit, ECGM_CGM_UC_PD_INTERFACE_FC_MAX_THr, pd, CGM_UC_PD_INTERFACE_FC_MAX_TH_0f);
	  pd_min_field = soc_reg_above_64_field32_get(unit, ECGM_CGM_UC_PD_INTERFACE_FC_MIN_THr, pd_min, CGM_UC_PD_INTERFACE_FC_MIN_TH_0f);
	  pd_alpha_field = soc_reg_above_64_field32_get(unit, ECGM_CGM_UC_PD_INTERFACE_FC_ALPHAr, pd_alpha, CGM_UC_PD_INTERFACE_FC_ALPHA_0f);
      size_256_field = soc_reg_above_64_field32_get(unit, ECGM_CGM_UC_SIZE_256_INTERFACE_FC_MAX_THr, size_256, CGM_UC_SIZE_256_INTERFACE_FC_MAX_TH_0f);
      size_256_min_field = soc_reg_above_64_field32_get(unit, ECGM_CGM_UC_SIZE_256_INTERFACE_FC_MIN_THr, size_256_min, CGM_UC_SIZE_256_INTERFACE_FC_MIN_TH_0f);
      size_256_alpha_field = soc_reg_above_64_field32_get(unit, ECGM_CGM_UC_SIZE_256_INTERFACE_FC_ALPHAr, size_256_alpha, CGM_UC_SIZE_256_INTERFACE_FC_ALPHA_0f);	  
      break;
    case 1:
      pd_field = soc_reg_above_64_field32_get(unit, ECGM_CGM_UC_PD_INTERFACE_FC_MAX_THr, pd, CGM_UC_PD_INTERFACE_FC_MAX_TH_1f);
	  pd_min_field = soc_reg_above_64_field32_get(unit, ECGM_CGM_UC_PD_INTERFACE_FC_MIN_THr, pd_min, CGM_UC_PD_INTERFACE_FC_MIN_TH_1f);
	  pd_alpha_field = soc_reg_above_64_field32_get(unit, ECGM_CGM_UC_PD_INTERFACE_FC_ALPHAr, pd_alpha, CGM_UC_PD_INTERFACE_FC_ALPHA_1f);
      size_256_field = soc_reg_above_64_field32_get(unit, ECGM_CGM_UC_SIZE_256_INTERFACE_FC_MAX_THr, size_256, CGM_UC_SIZE_256_INTERFACE_FC_MAX_TH_1f);
      size_256_min_field = soc_reg_above_64_field32_get(unit, ECGM_CGM_UC_SIZE_256_INTERFACE_FC_MIN_THr, size_256_min, CGM_UC_SIZE_256_INTERFACE_FC_MIN_TH_1f);
      size_256_alpha_field = soc_reg_above_64_field32_get(unit, ECGM_CGM_UC_SIZE_256_INTERFACE_FC_ALPHAr, size_256_alpha, CGM_UC_SIZE_256_INTERFACE_FC_ALPHA_1f);		 
      break;
    case 2:
      pd_field = soc_reg_above_64_field32_get(unit, ECGM_CGM_UC_PD_INTERFACE_FC_MAX_THr, pd, CGM_UC_PD_INTERFACE_FC_MAX_TH_2f);
	  pd_min_field = soc_reg_above_64_field32_get(unit, ECGM_CGM_UC_PD_INTERFACE_FC_MIN_THr, pd_min, CGM_UC_PD_INTERFACE_FC_MIN_TH_2f);
	  pd_alpha_field = soc_reg_above_64_field32_get(unit, ECGM_CGM_UC_PD_INTERFACE_FC_ALPHAr, pd_alpha, CGM_UC_PD_INTERFACE_FC_ALPHA_2f);
      size_256_field = soc_reg_above_64_field32_get(unit, ECGM_CGM_UC_SIZE_256_INTERFACE_FC_MAX_THr, size_256, CGM_UC_SIZE_256_INTERFACE_FC_MAX_TH_2f);
      size_256_min_field = soc_reg_above_64_field32_get(unit, ECGM_CGM_UC_SIZE_256_INTERFACE_FC_MIN_THr, size_256_min, CGM_UC_SIZE_256_INTERFACE_FC_MIN_TH_2f);
      size_256_alpha_field = soc_reg_above_64_field32_get(unit, ECGM_CGM_UC_SIZE_256_INTERFACE_FC_ALPHAr, size_256_alpha, CGM_UC_SIZE_256_INTERFACE_FC_ALPHA_2f);	 
      break;
    case 3:
      pd_field = soc_reg_above_64_field32_get(unit, ECGM_CGM_UC_PD_INTERFACE_FC_MAX_THr, pd, CGM_UC_PD_INTERFACE_FC_MAX_TH_3f);
	  pd_min_field = soc_reg_above_64_field32_get(unit, ECGM_CGM_UC_PD_INTERFACE_FC_MIN_THr, pd_min, CGM_UC_PD_INTERFACE_FC_MIN_TH_3f);
	  pd_alpha_field = soc_reg_above_64_field32_get(unit, ECGM_CGM_UC_PD_INTERFACE_FC_ALPHAr, pd_alpha, CGM_UC_PD_INTERFACE_FC_ALPHA_3f);
      size_256_field = soc_reg_above_64_field32_get(unit, ECGM_CGM_UC_SIZE_256_INTERFACE_FC_MAX_THr, size_256, CGM_UC_SIZE_256_INTERFACE_FC_MAX_TH_3f);
      size_256_min_field = soc_reg_above_64_field32_get(unit, ECGM_CGM_UC_SIZE_256_INTERFACE_FC_MIN_THr, size_256_min, CGM_UC_SIZE_256_INTERFACE_FC_MIN_TH_3f);
      size_256_alpha_field = soc_reg_above_64_field32_get(unit, ECGM_CGM_UC_SIZE_256_INTERFACE_FC_ALPHAr, size_256_alpha, CGM_UC_SIZE_256_INTERFACE_FC_ALPHA_3f);		 
      break;
    case 4:
      pd_field = soc_reg_above_64_field32_get(unit, ECGM_CGM_UC_PD_INTERFACE_FC_MAX_THr, pd, CGM_UC_PD_INTERFACE_FC_MAX_TH_4f);
	  pd_min_field = soc_reg_above_64_field32_get(unit, ECGM_CGM_UC_PD_INTERFACE_FC_MIN_THr, pd_min, CGM_UC_PD_INTERFACE_FC_MIN_TH_4f);
	  pd_alpha_field = soc_reg_above_64_field32_get(unit, ECGM_CGM_UC_PD_INTERFACE_FC_ALPHAr, pd_alpha, CGM_UC_PD_INTERFACE_FC_ALPHA_4f);
      size_256_field = soc_reg_above_64_field32_get(unit, ECGM_CGM_UC_SIZE_256_INTERFACE_FC_MAX_THr, size_256, CGM_UC_SIZE_256_INTERFACE_FC_MAX_TH_4f);
      size_256_min_field = soc_reg_above_64_field32_get(unit, ECGM_CGM_UC_SIZE_256_INTERFACE_FC_MIN_THr, size_256_min, CGM_UC_SIZE_256_INTERFACE_FC_MIN_TH_4f);
      size_256_alpha_field = soc_reg_above_64_field32_get(unit, ECGM_CGM_UC_SIZE_256_INTERFACE_FC_ALPHAr, size_256_alpha, CGM_UC_SIZE_256_INTERFACE_FC_ALPHA_4f);	 
      break;
    case 5:
      pd_field = soc_reg_above_64_field32_get(unit, ECGM_CGM_UC_PD_INTERFACE_FC_MAX_THr, pd, CGM_UC_PD_INTERFACE_FC_MAX_TH_5f);
	  pd_min_field = soc_reg_above_64_field32_get(unit, ECGM_CGM_UC_PD_INTERFACE_FC_MIN_THr, pd_min, CGM_UC_PD_INTERFACE_FC_MIN_TH_5f);
	  pd_alpha_field = soc_reg_above_64_field32_get(unit, ECGM_CGM_UC_PD_INTERFACE_FC_ALPHAr, pd_alpha, CGM_UC_PD_INTERFACE_FC_ALPHA_5f);
      size_256_field = soc_reg_above_64_field32_get(unit, ECGM_CGM_UC_SIZE_256_INTERFACE_FC_MAX_THr, size_256, CGM_UC_SIZE_256_INTERFACE_FC_MAX_TH_5f);
      size_256_min_field = soc_reg_above_64_field32_get(unit, ECGM_CGM_UC_SIZE_256_INTERFACE_FC_MIN_THr, size_256_min, CGM_UC_SIZE_256_INTERFACE_FC_MIN_TH_5f);
      size_256_alpha_field = soc_reg_above_64_field32_get(unit, ECGM_CGM_UC_SIZE_256_INTERFACE_FC_ALPHAr, size_256_alpha, CGM_UC_SIZE_256_INTERFACE_FC_ALPHA_5f);	 
      break;
    case 6:
      pd_field = soc_reg_above_64_field32_get(unit, ECGM_CGM_UC_PD_INTERFACE_FC_MAX_THr, pd, CGM_UC_PD_INTERFACE_FC_MAX_TH_6f);
	  pd_min_field = soc_reg_above_64_field32_get(unit, ECGM_CGM_UC_PD_INTERFACE_FC_MIN_THr, pd_min, CGM_UC_PD_INTERFACE_FC_MIN_TH_6f);
	  pd_alpha_field = soc_reg_above_64_field32_get(unit, ECGM_CGM_UC_PD_INTERFACE_FC_ALPHAr, pd_alpha, CGM_UC_PD_INTERFACE_FC_ALPHA_6f);
      size_256_field = soc_reg_above_64_field32_get(unit, ECGM_CGM_UC_SIZE_256_INTERFACE_FC_MAX_THr, size_256, CGM_UC_SIZE_256_INTERFACE_FC_MAX_TH_6f);
      size_256_min_field = soc_reg_above_64_field32_get(unit, ECGM_CGM_UC_SIZE_256_INTERFACE_FC_MIN_THr, size_256_min, CGM_UC_SIZE_256_INTERFACE_FC_MIN_TH_6f);
      size_256_alpha_field = soc_reg_above_64_field32_get(unit, ECGM_CGM_UC_SIZE_256_INTERFACE_FC_ALPHAr, size_256_alpha, CGM_UC_SIZE_256_INTERFACE_FC_ALPHA_6f);	
      break;
    case 7:
      pd_field = soc_reg_above_64_field32_get(unit, ECGM_CGM_UC_PD_INTERFACE_FC_MAX_THr, pd, CGM_UC_PD_INTERFACE_FC_MAX_TH_7f);
	  pd_min_field = soc_reg_above_64_field32_get(unit, ECGM_CGM_UC_PD_INTERFACE_FC_MIN_THr, pd_min, CGM_UC_PD_INTERFACE_FC_MIN_TH_7f);
	  pd_alpha_field = soc_reg_above_64_field32_get(unit, ECGM_CGM_UC_PD_INTERFACE_FC_ALPHAr, pd_alpha, CGM_UC_PD_INTERFACE_FC_ALPHA_7f);
      size_256_field = soc_reg_above_64_field32_get(unit, ECGM_CGM_UC_SIZE_256_INTERFACE_FC_MAX_THr, size_256, CGM_UC_SIZE_256_INTERFACE_FC_MAX_TH_7f);
      size_256_min_field = soc_reg_above_64_field32_get(unit, ECGM_CGM_UC_SIZE_256_INTERFACE_FC_MIN_THr, size_256_min, CGM_UC_SIZE_256_INTERFACE_FC_MIN_TH_7f);
      size_256_alpha_field = soc_reg_above_64_field32_get(unit, ECGM_CGM_UC_SIZE_256_INTERFACE_FC_ALPHAr, size_256_alpha, CGM_UC_SIZE_256_INTERFACE_FC_ALPHA_7f);	
      break;
    default:
      DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("Invalid profille ID")));
  }

  info->pd_th = pd_field;
  info->pd_th_min = pd_min_field;
  DNXC_IF_ERR_EXIT(_soc_jer2_qax_convert_field_val_to_alpha(unit, pd_alpha_field, &alpha));
  info->pd_th_alpha = alpha;
  
  info->size256_th = size_256_field;
  info->size256_th_min = size_256_min_field;
  DNXC_IF_ERR_EXIT(_soc_jer2_qax_convert_field_val_to_alpha(unit, size_256_alpha_field, &alpha));
  info->size256_th_alpha = alpha;

exit:
  DNXC_FUNC_RETURN;
}

int
  jer2_qax_egr_queuing_if_fc_mc_set(
    DNX_SAND_IN    int    unit,
    DNX_SAND_IN    int    core,
    DNX_SAND_IN    uint32    mc_if_profile_ndx,
    DNX_SAND_IN uint32    pd_th
  )
{
  uint64
    reg;
  DNXC_INIT_FUNC_DEFS;
  /* ECGM_CGM_MC_INTERFACE_PD_THr */
  DNXC_IF_ERR_EXIT(READ_ECGM_CGM_MC_INTERFACE_PD_THr(unit, &reg));
  switch(mc_if_profile_ndx) {
    case 0:
      soc_reg64_field32_set(unit, ECGM_CGM_MC_INTERFACE_PD_THr, &reg, CGM_MC_INTERFACE_PD_TH_0f, pd_th);
      break;
    case 1:
      soc_reg64_field32_set(unit, ECGM_CGM_MC_INTERFACE_PD_THr, &reg, CGM_MC_INTERFACE_PD_TH_1f, pd_th);
      break;
    case 2:
      soc_reg64_field32_set(unit, ECGM_CGM_MC_INTERFACE_PD_THr, &reg, CGM_MC_INTERFACE_PD_TH_2f, pd_th);
      break;
    case 3:
      soc_reg64_field32_set(unit, ECGM_CGM_MC_INTERFACE_PD_THr, &reg, CGM_MC_INTERFACE_PD_TH_3f, pd_th);
      break;
    default:
      DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("")));
  }
  DNXC_IF_ERR_EXIT(WRITE_ECGM_CGM_MC_INTERFACE_PD_THr(unit, reg));

exit:
  DNXC_FUNC_RETURN;
}

int
  jer2_qax_egr_queuing_if_fc_mc_get(
    DNX_SAND_IN     int    unit,
    DNX_SAND_IN     uint32    mc_if_profile_ndx,
    DNX_SAND_OUT uint32   *pd_th
  )
{
  uint64
    reg;
  uint32
    temp_pd_th = 0;
  DNXC_INIT_FUNC_DEFS;
  DNXC_NULL_CHECK(pd_th);
  DNXC_IF_ERR_EXIT(READ_ECGM_CGM_MC_INTERFACE_PD_THr(unit, &reg));
  switch(mc_if_profile_ndx) {
    case 0:
      temp_pd_th = soc_reg64_field32_get(unit, ECGM_CGM_MC_INTERFACE_PD_THr, reg, CGM_MC_INTERFACE_PD_TH_0f);
      break;
    case 1:
      temp_pd_th = soc_reg64_field32_get(unit, ECGM_CGM_MC_INTERFACE_PD_THr, reg, CGM_MC_INTERFACE_PD_TH_1f);
      break;
    case 2:
      temp_pd_th = soc_reg64_field32_get(unit, ECGM_CGM_MC_INTERFACE_PD_THr, reg, CGM_MC_INTERFACE_PD_TH_2f);
      break;
    case 3:
      temp_pd_th = soc_reg64_field32_get(unit, ECGM_CGM_MC_INTERFACE_PD_THr, reg, CGM_MC_INTERFACE_PD_TH_3f);
      break;
    default:
      DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("")));
  }

  *pd_th = temp_pd_th;

exit:
  DNXC_FUNC_RETURN;
}


#define READ_REGISTER_NO_PIPE(reg, out_variable) \
  if (READ_##reg(unit, &(out_variable)) != SOC_E_NONE) { \
    DNXC_EXIT_WITH_ERR(SOC_E_FAIL, (_BSL_DNXC_MSG("Failed to read "#reg" unit %d core %d"), unit, core)); \
  }


#define READ_REGISTER_ARRAY_NO_PIPE(reg, i, out_variable) \
  if (READ_##reg(unit, i, &(out_variable)) != SOC_E_NONE) { \
    DNXC_EXIT_WITH_ERR(SOC_E_FAIL, (_BSL_DNXC_MSG("Failed to read "#reg" at index %d unit %d"), i, unit)); \
  }

#define READ_REGISTER_ARRAY(reg, i, out_variable) \
  if (READ_##reg(unit, i, &(out_variable)) != SOC_E_NONE) { \
    DNXC_EXIT_WITH_ERR(SOC_E_FAIL, (_BSL_DNXC_MSG("Failed to read "#reg" at index %d unit %d core %d"), i, unit, core)); \
  }

#define READ_MEMORY(mem, index1, index2, dma_mem) \
  { \
    int rv = soc_mem_array_read_range(unit, mem, 0, EGQ_BLOCK(unit, core), index1, index2, dma_mem); \
    if (rv != SOC_E_NONE) { \
      DNXC_EXIT_WITH_ERR(SOC_E_FAIL, (_BSL_DNXC_MSG("Failed to read "#mem" at indices %d-%d unit %d: %s"), index1, index2, unit, soc_errmsg(rv))); \
    } \
  }

#define CLEAR_MEMORY(mem, index1, index2, dma_mem) \
  { \
    int rv; \
    *dma_mem = 0; \
    rv = jer2_arad_fill_partial_table_with_entry(unit, mem, 0, 0, EGQ_BLOCK(unit, core), index1, index2, (void*)dma_mem); \
    if (rv != SOC_E_NONE) { \
      DNXC_EXIT_WITH_ERR(SOC_E_FAIL, (_BSL_DNXC_MSG("Failed to re-initialize "#mem" at indices %d-%d unit %d: %s"), index1, index2, unit, soc_errmsg(rv))); \
    } \
  }

#define OTM_PORTS_LAST_ARRAY_INDEX   (2*JER2_ARAD_EGR_CGM_OTM_PORTS_NUM-1)
#define QUEUES_LAST_ARRAY_INDEX   (2*JER2_ARAD_EGR_CGM_OTM_PORTS_NUM-1)
#define IF_LAST_ARRAY_INDEX     (2*JER2_ARAD_EGR_CGM_IF_NUM-1)



int soc_jer2_qax_egr_congestion_statistics_get(
      DNX_SAND_IN int unit,
      DNX_SAND_IN int core,
      DNX_SAND_INOUT JER2_ARAD_EGR_CGM_CONGENSTION_STATS *cur_stats,   /* place current statistics output here */
      DNX_SAND_INOUT JER2_ARAD_EGR_CGM_CONGENSTION_STATS *max_stats,   /* place maximum statistics output here */
      DNX_SAND_INOUT JER2_ARAD_EGR_CGM_CONGENSTION_COUNTERS *counters, /* place counters output here */
      DNX_SAND_IN int disable_updates /* should the function disable maximum statistics updates when it collects them */
  )
{
  int i;
  int updates_are_disabled = 0;
  uint32 value;
  uint32 *dma_buf = 0;
  uint32 *buf_ptr;
#ifdef BCM_88675_A0
  int dynamic_mem_access = 0;
#endif
  DNXC_INIT_FUNC_DEFS;

  if (cur_stats != NULL || max_stats != NULL) { /* allocate DMA memory if needed */
    int mem_size = 8 *   /* 4 byte words * 2 (UC, MC) * MAX{OTM_PORTS_LAST_ARRAY_INDEX, QUEUES_LAST_ARRAY_INDEX} */
      (OTM_PORTS_LAST_ARRAY_INDEX > QUEUES_LAST_ARRAY_INDEX ? OTM_PORTS_LAST_ARRAY_INDEX : QUEUES_LAST_ARRAY_INDEX);

    dma_buf = soc_cm_salloc(unit, mem_size, "cgm_statistics_mem"); /* allocate DMA memory buffer */
    if (dma_buf == NULL) {
        DNXC_EXIT_WITH_ERR(SOC_E_MEMORY, (_BSL_DNXC_MSG("Failed to allocate dma memory for statistics data")));
    }
  }

  if (cur_stats != NULL) { /* collect current value statistics */

    /* register providing current values */

    /* Current total number of allocated packet descriptors */
    READ_REGISTER_NO_PIPE(ECGM_TOTAL_PACKET_DESCRIPTORS_COUNTERr, value);
    cur_stats->pd = value; /* place the value into the 16 bits integer */
    /* cur_stats->pd= soc_reg_field_get(unit, ECGM_TOTAL_PACKET_DESCRIPTORS_COUNTERr, value, NUMBER_OF_ALLOCATED_PACKET_DESCRIPTORSf); */

    /* Current total number of allocated Data Buffers */
    READ_REGISTER_NO_PIPE(ECGM_TOTAL_DATA_BUFFERS_COUNTERr, value);
    cur_stats->db = value; /* place the value into the 16 bits integer */
    /* cur_stats->db= soc_reg_field_get(unit, ECGM_TOTAL_DATA_BUFFERS_COUNTERr, value, NUMBER_OF_ALLOCATED_DATA_BUFFERSf); */

    /* Current number of packet descriptors allocated to unicast packets */
    READ_REGISTER_NO_PIPE(ECGM_UNICAST_PACKET_DESCRIPTORS_COUNTERr, value);
    cur_stats->uc_pd = value; /* place the value into the 16 bits integer */
    /* cur_stats->uc_pd= soc_reg_field_get(unit, ECGM_UNICAST_PACKET_DESCRIPTORS_COUNTERr, value, UNICAST_PACKET_DESCRIPTORS_COUNTERf); */

    /* Current number of packet descriptors allocated to multicast replication packets */
    READ_REGISTER_NO_PIPE(ECGM_MULTICAST_REPLICATIONS_PACKET_DESCRIPTORS_COUNTERr, value);
    cur_stats->mc_pd = value; /* place the value into the 16 bits integer */
    /* cur_stats->mc_pd= soc_reg_field_get(unit, ECGM_MULTICAST_REPLICATIONS_PACKET_DESCRIPTORS_COUNTERr, value, NUMBER_OF_ALLOCATED_PACKET_DESCRIPTORSf); */

    /* Current number of Data Buffers allocated to unicast packets */
    READ_REGISTER_NO_PIPE(ECGM_UNICAST_DATA_BUFFERS_COUNTERr, value);
    cur_stats->uc_db = value; /* place the value into the 16 bits integer */
    /* cur_stats->uc_db= soc_reg_field_get(unit, ECGM_UNICAST_DATA_BUFFERS_COUNTERr, value, NUMBER_OF_UNICAST_DATA_BUFFERSf); */

    /* Current number of Data Buffers allocated to multicast packets, regardless of number of replications */
    READ_REGISTER_NO_PIPE(ECGM_MULTICAST_DATA_BUFFERS_COUNTERr, value);
    cur_stats->mc_db = value; /* place the value into the 16 bits integer */
    /* cur_stats->mc_db= soc_reg_field_get(unit, ECGM_MULTICAST_DATA_BUFFERS_COUNTERr, value, NUMBER_OF_MULTICAST_DATA_BUFFERSf); */

    /* Unicast and Multicast current packet descriptors per interface, for MC the data is size in 128 bytes */
    READ_MEMORY(EGQ_FDCMm, 0, IF_LAST_ARRAY_INDEX, dma_buf);
    for (i = 0, buf_ptr = dma_buf; i < JER2_ARAD_EGR_CGM_IF_NUM; ++i, ++buf_ptr) {
      cur_stats->uc_pd_if[i] = soc_mem_field32_get(unit, EGQ_FDCMm, buf_ptr, FDCMf);
    }
    for (i = 0; i < JER2_ARAD_EGR_CGM_IF_NUM; ++i, ++buf_ptr) {
      cur_stats->mc_pd_if[i] = soc_mem_field32_get(unit, EGQ_FDCMm, buf_ptr, FDCMf);
    }

    /* The size of this interface contributed by unicast and multicast packets, where the size is measured in 256B units. For MC the data is size in 128 bytes */
    READ_MEMORY(EGQ_FQSMm, 0, IF_LAST_ARRAY_INDEX, dma_buf);
    for (i = 0, buf_ptr = dma_buf; i < JER2_ARAD_EGR_CGM_IF_NUM; ++i, ++buf_ptr) {
        cur_stats->uc_size_256_if[i] = soc_mem_field32_get(unit, EGQ_FQSMm, buf_ptr, FQSMf);
    }
    for (i = 0; i < JER2_ARAD_EGR_CGM_IF_NUM; ++i, ++buf_ptr) {
        cur_stats->mc_size_256_if[i] = soc_mem_field32_get(unit, EGQ_FQSMm, buf_ptr, FQSMf);
    }

    /*
    The memories need to be written to init the maximum value; unlike the registers which auto-initialize.
    This why CLEAR_MEMORY() is used below.
    */

    /* Unicast and Multicast current packet descriptors per OTM-Port */
    READ_MEMORY(EGQ_PDCMm, 0, OTM_PORTS_LAST_ARRAY_INDEX, dma_buf);
    for (i = 0, buf_ptr = dma_buf; i < JER2_ARAD_EGR_CGM_OTM_PORTS_NUM; ++i, ++buf_ptr) {
      cur_stats->uc_pd_port[i] = soc_mem_field32_get(unit, EGQ_PDCMm, buf_ptr, PDCMf); /* 15b field */
    }
    for (i = 0; i < JER2_ARAD_EGR_CGM_OTM_PORTS_NUM; ++i, ++buf_ptr) {
      cur_stats->mc_pd_port[i] = soc_mem_field32_get(unit, EGQ_PDCMm, buf_ptr, PDCMf); /* 15b field */
    }

    /* Unicast and Multicast current Packet descriptors per queue */
    READ_MEMORY(EGQ_QDCMm, 0, QUEUES_LAST_ARRAY_INDEX, dma_buf);
    for (i = 0, buf_ptr = dma_buf; i < JER2_ARAD_EGR_CGM_QUEUES_NUM; ++i, ++buf_ptr) {
      cur_stats->uc_pd_queue[i] = soc_mem_field32_get(unit, EGQ_QDCMm, buf_ptr, QDCMf); /* 15b field */
    }
    for (i = 0; i < JER2_ARAD_EGR_CGM_QUEUES_NUM; ++i, ++buf_ptr) {
      cur_stats->mc_pd_queue[i] = soc_mem_field32_get(unit, EGQ_QDCMm, buf_ptr, QDCMf); /* 15b field */
    }

    /* Unicast and Multicast current data buffers per OTM-Port, for MC the data is size in 256 bytes */
    READ_MEMORY(EGQ_PQSMm, 0, OTM_PORTS_LAST_ARRAY_INDEX, dma_buf);
    for (i = 0, buf_ptr = dma_buf; i < JER2_ARAD_EGR_CGM_OTM_PORTS_NUM; ++i, ++buf_ptr) {
      cur_stats->uc_db_port[i] = soc_mem_field32_get(unit, EGQ_PQSMm, buf_ptr, PQSMf); /* 18b field */
    }
    for (i = 0; i < JER2_ARAD_EGR_CGM_OTM_PORTS_NUM; ++i, ++buf_ptr) {
      cur_stats->mc_db_port[i] = soc_mem_field32_get(unit, EGQ_PQSMm, buf_ptr, PQSMf); /* 18b field */
    }

    /* Unicast and Multicast current data buffers per queue, for MC the data is size in 256 bytes */
    READ_MEMORY(EGQ_QQSMm, 0, QUEUES_LAST_ARRAY_INDEX, dma_buf);
    for (i = 0, buf_ptr = dma_buf; i < JER2_ARAD_EGR_CGM_QUEUES_NUM; ++i, ++buf_ptr) {
      cur_stats->uc_db_queue[i] = soc_mem_field32_get(unit, EGQ_QQSMm, buf_ptr, QQSMf); /* 18b field */
    }
    for (i = 0; i < JER2_ARAD_EGR_CGM_QUEUES_NUM; ++i, ++buf_ptr) {
      cur_stats->mc_db_queue[i] = soc_mem_field32_get(unit, EGQ_QQSMm, buf_ptr, QQSMf); /* 18b field */
    }


    /* Current number of packet descriptors allocated to multicast replication packets bound to Service Pool0 */
    READ_REGISTER_NO_PIPE(ECGM_SERVICE_POOL_0_MULTICAST_PACKET_DESCRIPTORS_COUNTERr, value);
    cur_stats->mc_pd_sp[0] = value; /* place the value into the 16 bits integer */
    /* cur_stats->mc_pd_sp[0] = soc_reg_field_get(unit, ECGM_SERVICE_POOL_0_MULTICAST_PACKET_DESCRIPTORS_COUNTERr, value, NUMBER_OF_MULTICAST_SP_0_PACKET_DESCRIPTORSf); */

    /* Current number of packet descriptors allocated to multicast replication packets bound to Service Pool1 */
    READ_REGISTER_NO_PIPE(ECGM_SERVICE_POOL_1_MULTICAST_PACKET_DESCRIPTORS_COUNTERr, value);
    cur_stats->mc_pd_sp[1] = value; /* place the value into the 16 bits integer */
    /* cur_stats->mc_pd_sp[1] = soc_reg_field_get(unit, ECGM_SERVICE_POOL_1_MULTICAST_PACKET_DESCRIPTORS_COUNTERr, value, NUMBER_OF_MULTICAST_SP_1_PACKET_DESCRIPTORSf); */

    /* Current number of Data Buffers allocated to multicast packets bound to Service Pool0 */
    READ_REGISTER_NO_PIPE(ECGM_SERVICE_POOL_0_MULTICAST_DATA_BUFFERS_COUNTERr, value);
    cur_stats->mc_db_sp[0] = value; /* place the value into the 16 bits integer */
    /* cur_stats->mc_db_sp[0] = soc_reg_field_get(unit, ECGM_SERVICE_POOL_0_MULTICAST_DATA_BUFFERS_COUNTERr, value, NUMBER_OF_MULTICAST_SP_0_DATA_BUFFERSf); */

    /* Current number of Data Buffers allocated to multicast packets bound to Service Pool1 */
    READ_REGISTER_NO_PIPE(ECGM_SERVICE_POOL_1_MULTICAST_DATA_BUFFERS_COUNTERr, value);
    cur_stats->mc_db_sp[1] = value; /* place the value into the 16 bits integer */
    /* cur_stats->mc_db_sp[1] = soc_reg_field_get(unit, ECGM_SERVICE_POOL_1_MULTICAST_DATA_BUFFERS_COUNTERr, value, NUMBER_OF_MULTICAST_SP_1_DATA_BUFFERSf); */


    /* The number of MC-PD'S - Per SP per TC. Indicates the value of Multicast Packet Descriptors Counter ( Per SP per TC). Low 8 counters are for service-pool-0 and high 8 counters are for service-pool-1. */
    for (i = SOC_REG_NUMELS(unit, ECGM_MC_PD_SP_TC_CNTr); i;) { /* 16 array elements */
      --i;
      READ_REGISTER_ARRAY(ECGM_MC_PD_SP_TC_CNTr, i, value);
      cur_stats->mc_pd_sp_tc[i] = value; /* place the value into the 16 bits integer */
      /* cur_stats->mc_pd_sp_tc[i] = soc_reg_field_get(unit, ECGM_MC_PD_SP_TC_CNTr, value, MC_PD_SP_TC_CNT_Nf); */
    }

    /* The number of MC-DB'S - Per SP per TC. Indicates the value of Multicast Data buffers Counter ( Per SP per TC). Low 8 counters are for service-pool-0 and high 8 counters are for service-pool-1. */
    for (i = SOC_REG_NUMELS(unit, ECGM_MC_DB_SP_TC_CNTr); i;) { /* 16 array elements */
      --i;
      READ_REGISTER_ARRAY(ECGM_MC_DB_SP_TC_CNTr, i, value);
      cur_stats->mc_db_sp_tc[i] = value; /* place the value into the 16 bits integer */
      /* cur_stats->mc_db_sp_tc[i] = soc_reg_field_get(unit, ECGM_MC_DB_SP_TC_CNTr, value, MC_DB_SP_TC_CNT_Nf); */
    }

    /* Current number of available reserved packet descriptors in Service Pool0. This counter is loaded by the CPU with the maximum number of reserved resource and is decreased for every occupied reserved resource and increased whenever such a resource is reclaimed. */
    READ_REGISTER_NO_PIPE(ECGM_SERVICE_POOL_0_RESERVED_PACKET_DESCRIPTORSr, value);
    cur_stats->mc_rsvd_pd_sp[0] = value; /* place the value into the 16 bits integer */
    /* cur_stats->mc_rsvd_pd_sp[0] = soc_reg_field_get(unit, ECGM_SERVICE_POOL_0_RESERVED_PACKET_DESCRIPTORSr, value, AVAILABLE_SP_0_RESERVED_PACKET_DESCRIPTORSf); */

    /* Current number of available reserved packet descriptors in Service Pool1. This counter is loaded by the CPU with the maximum number of reserved resource and is decreased for every occupied reserved resource and increased whenever such a resource is reclaimed. */
    READ_REGISTER_NO_PIPE(ECGM_SERVICE_POOL_1_RESERVED_PACKET_DESCRIPTORSr, value);
    cur_stats->mc_rsvd_pd_sp[1] = value; /* place the value into the 16 bits integer */
    /* cur_stats->mc_rsvd_pd_sp[1] = soc_reg_field_get(unit, ECGM_SERVICE_POOL_1_RESERVED_PACKET_DESCRIPTORSr, value,AVAILABLE_SP_1_RESERVED_PACKET_DESCRIPTORSf); */

    /* Current number of available reserved Data Buffers in Service Pool0. This counter is loaded by the CPU with the maximum number of reserved resource and is decreased for every occupied reserved resource and increased whenever such a resource is reclaimed. */
    READ_REGISTER_NO_PIPE(ECGM_SERVICE_POOL_0_RESERVED_DATA_BUFFERSr, value);
    cur_stats->mc_rsvd_db_sp[0] = value; /* place the value into the 16 bits integer */
    /* cur_stats->mc_rsvd_db_sp[0] = soc_reg_field_get(unit, ECGM_SERVICE_POOL_0_RESERVED_DATA_BUFFERSr, value, AVAILABLE_SP_0_RESERVED_DATA_BUFFERSf); */

    /* Current number of available reserved Data Buffers in Service Pool1. This counter is loaded by the CPU with the maximum number of reserved resource and is decreased for every occupied reserved resource and increased whenever such a resource is reclaimed. */
    READ_REGISTER_NO_PIPE(ECGM_SERVICE_POOL_1_RESERVED_DATA_BUFFERSr, value);
    cur_stats->mc_rsvd_db_sp[1] = value; /* place the value into the 16 bits integer */
    /* cur_stats->mc_rsvd_db_sp[1] = soc_reg_field_get(unit, ECGM_SERVICE_POOL_1_RESERVED_DATA_BUFFERSr, value, AVAILABLE_SP_1_RESERVED_DATA_BUFFERSf); */

  } /* end of current value collection */


  if (max_stats != NULL) { /* collect maximum value statistics */

    /* disable maximum statistics updated if requested to do so */
    if (disable_updates) {
      if (WRITE_ECGM_STATISTICS_TRACKING_CONTROLr(unit, 1)!= SOC_E_NONE) {
        DNXC_EXIT_WITH_ERR(SOC_E_FAIL, (_BSL_DNXC_MSG("Failed to disable maximum statistics updates")));
      }
      updates_are_disabled = 1; /* we need to later enable updates */
    }


    /* Indicates the maximum value that the Total Packet Descriptors Counter has reached since the last time it was read by the CPU. */
    READ_REGISTER_NO_PIPE(ECGM_CONGESTION_TRACKING_TOTAL_PD_MAX_VALUEr, value);
    max_stats->pd = value; /* place the value into the 16 bits integer */
    /* max_stats->pd = soc_reg_field_get(unit, ECGM_CONGESTION_TRACKING_TOTAL_PD_MAX_VALUEr, value, CONGESTION_TRACKING_TOTAL_PD_MAX_VALUEf); */

    /* Indicates the maximum value that the Total Data Buffers Counter has reached since the last time it was read by the CPU. */
    READ_REGISTER_NO_PIPE(ECGM_CONGESTION_TRACKING_TOTAL_DB_MAX_VALUEr, value);
    max_stats->db = value; /* place the value into the 16 bits integer */
    /* max_stats->db = soc_reg_field_get(unit, ECGM_CONGESTION_TRACKING_TOTAL_DB_MAX_VALUEr, value, CONGESTION_TRACKING_TOTAL_DB_MAX_VALUEf); */

    /* Indicates the maximum value that the Unicast Packet Descriptors Counter has reached since the last time it was read by the CPU. */
    READ_REGISTER_NO_PIPE(ECGM_CONGESTION_TRACKING_UNICAST_PD_MAX_VALUEr, value);
    max_stats->uc_pd = value; /* place the value into the 16 bits integer */
    /* max_stats->uc_pd = soc_reg_field_get(unit, ECGM_CONGESTION_TRACKING_UNICAST_PD_MAX_VALUEr, value, CONGESTION_TRACKING_UNICAST_PD_MAX_VALUEf); */

    /* Indicates the maximum value that the Multicast Packet Descriptors Counter has reached since the last time it was read by the CPU. */
    READ_REGISTER_NO_PIPE(ECGM_CONGESTION_TRACKING_MULTICAST_PD_MAX_VALUEr, value);
    max_stats->mc_pd = value; /* place the value into the 16 bits integer */
    /* max_stats->mc_pd = soc_reg_field_get(unit, ECGM_CONGESTION_TRACKING_MULTICAST_PD_MAX_VALUEr, value, CONGESTION_TRACKING_MULTICAST_PD_MAX_VALUEf); */

    /* Indicates the maximum value that the Unicast Data Buffers Counter has reached since the last time it was read by the CPU. */
    READ_REGISTER_NO_PIPE(ECGM_CONGESTION_TRACKING_UNICAST_DB_MAX_VALUEr, value);
    max_stats->uc_db = value; /* place the value into the 16 bits integer */
    /* max_stats->uc_db = soc_reg_field_get(unit, ECGM_CONGESTION_TRACKING_UNICAST_DB_MAX_VALUEr, value, CONGESTION_TRACKING_UNICAST_DB_MAX_VALUEf); */

    /* Indicates the maximum value that the Multicast Data Buffers Counter has reached since the last time it was read by the CPU. */
    READ_REGISTER_NO_PIPE(ECGM_CONGESTION_TRACKING_MULTICAST_DB_MAX_VALUEr, value);
    max_stats->mc_db = value; /* place the value into the 16 bits integer */
    /* max_stats->mc_db = soc_reg_field_get(unit, ECGM_CONGESTION_TRACKING_MULTICAST_DB_MAX_VALUEr, value, CONGESTION_TRACKING_MULTICAST_DB_MAX_VALUEf); */

    /*
    The memories need to be written to init the maximum value; unlike the registers which auto-initialize.
    This why CLEAR_MEMORY() is used below.
    */
#ifdef BCM_88675_A0
    if (SOC_IS_JERICHO(unit)) {
        /* This register needs to be set in order to write to dynamic tables. */
        /* In Jericho the following tables are dynamic */
        if (READ_EGQ_ENABLE_DYNAMIC_MEMORY_ACCESSr(unit, core, &value) != SOC_E_NONE) {
            DNXC_EXIT_WITH_ERR(SOC_E_FAIL, (_BSL_DNXC_MSG("Failed to read EGQ_ENABLE_DYNAMIC_MEMORY_ACCESSr at unit %d core %d"), unit, core));
        }

        if (0 == value) {
            if (WRITE_EGQ_ENABLE_DYNAMIC_MEMORY_ACCESSr(unit, core, 1) != SOC_E_NONE) {
                DNXC_EXIT_WITH_ERR(SOC_E_FAIL, (_BSL_DNXC_MSG("Failed to write EGQ_ENABLE_DYNAMIC_MEMORY_ACCESSr at unit %d core %d"), unit, core));
            }
            dynamic_mem_access = 1;
        }
    }
#endif

    /* Indicates the maximum value that the Interface Unicast and Multicast Packet Descriptors Counters has reached since the last time it was read by the CPU. For MC the data is size in 128 bytes */
    READ_MEMORY(EGQ_FDCMAXm, 0, IF_LAST_ARRAY_INDEX, dma_buf);
    for (i = 0, buf_ptr = dma_buf; i < JER2_ARAD_EGR_CGM_IF_NUM; ++i, ++buf_ptr) {
        max_stats->uc_pd_if[i] = soc_mem_field32_get(unit, EGQ_FDCMAXm, buf_ptr, FDCMAXf);
    }
    for (i = 0; i < JER2_ARAD_EGR_CGM_IF_NUM; ++i, ++buf_ptr) {
        max_stats->mc_pd_if[i] = soc_mem_field32_get(unit, EGQ_FDCMAXm, buf_ptr, FDCMAXf);
    }
    CLEAR_MEMORY(EGQ_FDCMAXm, 0, IF_LAST_ARRAY_INDEX, dma_buf);

    /* Indicates the maximum Interface length, contributed by unicast packets, where length is mesured in units of 256 bytes. For MC the data is size in 128 bytes */
    READ_MEMORY(EGQ_FQSMAXm, 0, IF_LAST_ARRAY_INDEX, dma_buf);
    for (i = 0, buf_ptr = dma_buf; i < JER2_ARAD_EGR_CGM_IF_NUM; ++i, ++buf_ptr) {
        max_stats->uc_size_256_if[i] = soc_mem_field32_get(unit, EGQ_FQSMAXm, buf_ptr, FQSMAXf);
    }
    for (i = 0; i < JER2_ARAD_EGR_CGM_IF_NUM; ++i, ++buf_ptr) {
        max_stats->mc_size_256_if[i] = soc_mem_field32_get(unit, EGQ_FQSMAXm, buf_ptr, FQSMAXf);
    }
    CLEAR_MEMORY(EGQ_FQSMAXm, 0, IF_LAST_ARRAY_INDEX, dma_buf);

    /* Unicast and Multicast maximum packet descriptors per OTM-Port */
    READ_MEMORY(EGQ_PDCMAXm, 0, OTM_PORTS_LAST_ARRAY_INDEX, dma_buf);
    for (i = 0, buf_ptr = dma_buf; i < JER2_ARAD_EGR_CGM_OTM_PORTS_NUM; ++i, ++buf_ptr) {
      max_stats->uc_pd_port[i] = soc_mem_field32_get(unit, EGQ_PDCMAXm, buf_ptr, PDCMAXf); /* 15b field */
    }
    for (i = 0; i < JER2_ARAD_EGR_CGM_OTM_PORTS_NUM; ++i, ++buf_ptr) {
      max_stats->mc_pd_port[i] = soc_mem_field32_get(unit, EGQ_PDCMAXm, buf_ptr, PDCMAXf); /* 15b field */
    }
    CLEAR_MEMORY(EGQ_PDCMAXm, 0, OTM_PORTS_LAST_ARRAY_INDEX, dma_buf);

    /* Unicast and Multicast maximum Packet descriptors per queue */
    READ_MEMORY(EGQ_QDCMAXm, 0, QUEUES_LAST_ARRAY_INDEX, dma_buf);
    for (i = 0, buf_ptr = dma_buf; i < JER2_ARAD_EGR_CGM_QUEUES_NUM; ++i, ++buf_ptr) {
      max_stats->uc_pd_queue[i] = soc_mem_field32_get(unit, EGQ_QDCMAXm, buf_ptr, QDCMAXf); /* 15b field */
    }
    for (i = 0; i < JER2_ARAD_EGR_CGM_QUEUES_NUM; ++i, ++buf_ptr) {
      max_stats->mc_pd_queue[i] = soc_mem_field32_get(unit, EGQ_QDCMAXm, buf_ptr, QDCMAXf); /* 15b field */
    }
    CLEAR_MEMORY(EGQ_QDCMAXm, 0, QUEUES_LAST_ARRAY_INDEX, dma_buf);

    /* Unicast and Multicast maximum data buffers per OTM-Port */
    READ_MEMORY(EGQ_PQSMAXm, 0, OTM_PORTS_LAST_ARRAY_INDEX, dma_buf);
    for (i = 0, buf_ptr = dma_buf; i < JER2_ARAD_EGR_CGM_OTM_PORTS_NUM; ++i, ++buf_ptr) {
      max_stats->uc_db_port[i] = soc_mem_field32_get(unit, EGQ_PQSMAXm, buf_ptr, PQSMAXf); /* 18b field */
    }
    for (i = 0; i < JER2_ARAD_EGR_CGM_OTM_PORTS_NUM; ++i, ++buf_ptr) {
      max_stats->mc_db_port[i] = soc_mem_field32_get(unit, EGQ_PQSMAXm, buf_ptr, PQSMAXf); /* 18b field */
    }
    CLEAR_MEMORY(EGQ_PQSMAXm, 0, OTM_PORTS_LAST_ARRAY_INDEX, dma_buf);

    /* Unicast and Multicast maximum data buffers per queue */
    READ_MEMORY(EGQ_QQSMAXm, 0, QUEUES_LAST_ARRAY_INDEX, dma_buf);
    for (i = 0, buf_ptr = dma_buf; i < JER2_ARAD_EGR_CGM_QUEUES_NUM; ++i, ++buf_ptr) {
      max_stats->uc_db_queue[i] = soc_mem_field32_get(unit, EGQ_QQSMAXm, buf_ptr, QQSMAXf); /* 18b field */
    }
    for (i = 0; i < JER2_ARAD_EGR_CGM_QUEUES_NUM; ++i, ++buf_ptr) {
      max_stats->mc_db_queue[i] = soc_mem_field32_get(unit, EGQ_QQSMAXm, buf_ptr, QQSMAXf); /* 18b field */
    }
    CLEAR_MEMORY(EGQ_QQSMAXm, 0, QUEUES_LAST_ARRAY_INDEX, dma_buf);

#ifdef BCM_88675_A0
    if (SOC_IS_JERICHO(unit)) {
        /* Restore to previous value */
        if (dynamic_mem_access) {
            dynamic_mem_access = 0;
            if (WRITE_EGQ_ENABLE_DYNAMIC_MEMORY_ACCESSr(unit, core, 0) != SOC_E_NONE) {
                DNXC_EXIT_WITH_ERR(SOC_E_FAIL, (_BSL_DNXC_MSG("Failed to write EGQ_ENABLE_DYNAMIC_MEMORY_ACCESSr at unit %d core %d"), unit, core));
            }
        }
    }
#endif


    /* Indicates the maximum value that the Service Pool0 Multicast Packet Descriptors Counter has reached since the last time it was read by the CPU. */
    READ_REGISTER_NO_PIPE(ECGM_CONGESTION_TRACKING_MULTICAST_PDSP_0_MAX_VALUEr, value);
    max_stats->mc_pd_sp[0] = value; /* place the value into the 16 bits integer */
    /* max_stats->mc_pd_sp[0] = soc_reg_field_get(unit, ECGM_CONGESTION_TRACKING_MULTICAST_PDSP_0_MAX_VALUEr, value, CONGESTION_TRACKING_MULTICAST_PDSP_0_MAX_VALUEf); */

    /* Indicates the maximum value that the Service Pool1 Multicast Packet Descriptors Counter has reached since the last time it was read by the CPU. */
    READ_REGISTER_NO_PIPE(ECGM_CONGESTION_TRACKING_MULTICAST_PDSP_1_MAX_VALUEr, value);
    max_stats->mc_pd_sp[1] = value; /* place the value into the 16 bits integer */
    /* max_stats->mc_pd_sp[1] = soc_reg_field_get(unit, ECGM_CONGESTION_TRACKING_MULTICAST_PDSP_1_MAX_VALUEr, value, CONGESTION_TRACKING_MULTICAST_PDSP_1_MAX_VALUEf); */

    /* Indicates the maximum value that the Service Pool0 Multicast Data Buffers Counter has reached since the last time it was read by the CPU. */
    READ_REGISTER_NO_PIPE(ECGM_CONGESTION_TRACKING_MULTICAST_DBSP_0_MAX_VALUEr, value);
    max_stats->mc_db_sp[0] = value; /* place the value into the 16 bits integer */
    /* max_stats->mc_db_sp[0] = soc_reg_field_get(unit, ECGM_CONGESTION_TRACKING_MULTICAST_DBSP_0_MAX_VALUEr, value, CONGESTION_TRACKING_MULTICAST_DBSP_0_MAX_VALUf); */

    /* Indicates the maximum value that the Service Pool1 Multicast Data Buffers Counter has reached since the last time it was read by the CPU. */
    READ_REGISTER_NO_PIPE(ECGM_CONGESTION_TRACKING_MULTICAST_DBSP_1_MAX_VALUEr, value);
    max_stats->mc_db_sp[1] = value; /* place the value into the 16 bits integer */
    /* max_stats->mc_db_sp[1] = soc_reg_field_get(unit, ECGM_CONGESTION_TRACKING_MULTICAST_DBSP_1_MAX_VALUEr, value, CONGESTION_TRACKING_MULTICAST_DBSP_1_MAX_VALUEf); */

    /* The max number of MC-PD'S - Per SP per TC. Indicates the maximum value of Multicast Packet Descriptors Counter ( Per SP per TC). Low 8 counters are for service-pool-0 and high 8 counters are for service-pool-1. */
    for (i = SOC_REG_NUMELS(unit, ECGM_CONGESTION_TRACKING_MULTICAST_P_DPER_SP_TC_MAX_VALUEr); i;) { /* 16 array elements */
      --i;
      if (READ_ECGM_CONGESTION_TRACKING_MULTICAST_P_DPER_SP_TC_MAX_VALUEr(unit, i, &value) != SOC_E_NONE) { \
          DNXC_EXIT_WITH_ERR(SOC_E_FAIL, (_BSL_DNXC_MSG("Failed to read ECGM_CONGESTION_TRACKING_MULTICAST_P_DPER_SP_TC_MAX_VALUEr at index %d unit %d core %d"), i, unit, core)); \
      }
      max_stats->mc_pd_sp_tc[i] = value; /* place the value into the 16 bits integer */
      /* max_stats->mc_pd_sp_tc[i] = soc_reg_field_get(unit, ECGM_CONGESTION_TRACKING_MULTICAST_P_DPER_SP_TC_MAX_VALUEr, value, CONGESTION_TRACKING_MULTICAST_P_DPER_SP_TC_MAX_VALUEf); */
    }

    /* The max number of MC-DB'S - Per SP per TC. Indicates the maximum value of Multicast Data Buffers Counter ( Per SP per TC). Low 8 counters are for service-pool-0 and high 8 counters are for service-pool-1. */
    for (i = SOC_REG_NUMELS(unit, ECGM_CONGESTION_TRACKING_MULTICAST_D_BPER_SP_TC_MAX_VALUEr); i;) { /* 16 array elements */
      --i;
      if (READ_ECGM_CONGESTION_TRACKING_MULTICAST_D_BPER_SP_TC_MAX_VALUEr(unit, i, &value) != SOC_E_NONE) { \
          DNXC_EXIT_WITH_ERR(SOC_E_FAIL, (_BSL_DNXC_MSG("Failed to read ECGM_CONGESTION_TRACKING_MULTICAST_D_BPER_SP_TC_MAX_VALUEr at index %d unit %d core %d"), i, unit, core)); \
      }
      max_stats->mc_db_sp_tc[i] = value; /* place the value into the 16 bits integer */
      /* max_stats->mc_db_sp_tc[i] = soc_reg_field_get(unit, ECGM_CONGESTION_TRACKING_MULTICAST_D_BPER_SP_TC_MAX_VALUEr, value, CONGESTION_TRACKING_MULTICAST_D_BPER_SP_TC_MAX_VALUEf); */
    }

    /* disable maximum statistics updated if requested to do so */
    if (updates_are_disabled) {
      if (WRITE_ECGM_STATISTICS_TRACKING_CONTROLr(unit, 0) != SOC_E_NONE) {
        DNXC_EXIT_WITH_ERR(SOC_E_FAIL, (_BSL_DNXC_MSG("Failed to re-enable maximum statistics updates")));
      }
      updates_are_disabled = 0;
    }

    /* these statistics are not supported for maximum values */
    max_stats->mc_rsvd_pd_sp[0] = max_stats->mc_rsvd_pd_sp[1] =
    max_stats->mc_rsvd_db_sp[0] = max_stats->mc_rsvd_db_sp[1] = 0;

  } /* end of maximujm value collection */

  if (counters != NULL) {
    /* registers providing counter values */

    /* counts the number of packets that were dropped due to lack of unicast packet descriptors */
    READ_REGISTER_NO_PIPE(ECGM_UNICAST_PACKET_DESCRIPTORS_DROP_COUNTERr, counters->uc_pd_dropped);
    /* counters->uc_pd_dropped = soc_reg_field_get(unit, ECGM_UNICAST_PACKET_DESCRIPTORS_DROP_COUNTERr, value, UNICAST_PACKET_DESCRIPTORS_DROP_COUNTERf); */

    /* counts the number of packets that were dropped due to lack of multicast packet descriptors */
    READ_REGISTER_NO_PIPE(ECGM_MULTICAST_REPLICATIONS_PACKET_DESCRIPTORS_DROP_COUNTERr, counters->mc_rep_pd_dropped);
    /* counters->mc_rep_pd_dropped = soc_reg_field_get(unit, ECGM_MULTICAST_REPLICATIONS_PACKET_DESCRIPTORS_DROP_COUNTERr, value, MULTICAST_REPLICATIONS_PACKET_DESCRIPTORS_DROP_COUNTERf); */

    /* counts the number of packets that were dropped by the RQP due to lack of unicast data buffers */
    READ_REGISTER_NO_PIPE(ECGM_UNICAST_DATA_BUFFERS_DROP_BY_RQP_COUNTERr, counters->uc_db_dropped_by_rqp);
    /* counters->uc_db_dropped_by_rqp = soc_reg_field_get(unit, ECGM_UNICAST_DATA_BUFFERS_DROP_BY_RQP_COUNTERr, value, UNICAST_DATA_BUFFERS_DROP_BY_RQP_COUNTERf); */

    /* counts the number of packets that were dropped by the PQP due to lack of unicast data buffers */
    READ_REGISTER_NO_PIPE(ECGM_UNICAST_DATA_BUFFERS_DROP_BY_PQP_COUNTERr, counters->uc_db_dropped_by_pqp);
    /* counters->uc_db_dropped_by_pqp = soc_reg_field_get(unit, ECGM_UNICAST_DATA_BUFFERS_DROP_BY_PQP_COUNTERr, value, UNICAST_DATA_BUFFERS_DROP_BY_PQP_COUNTERf); */

    /* Counts the number of packets that were dropped due to lack of multicast data buffers. Note that this counter does not count each replication drop but rather the packet drops before replication, i.e. when a packet with n replication is dropped due to lack of multicast data buffers, the counter is increased by one regardless of number of replications. */
    READ_REGISTER_NO_PIPE(ECGM_MULTICAST_DATA_BUFFERS_DROP_COUNTERr, counters->mc_db_dropped);
    /* counters->mc_db_dropped = soc_reg_field_get(unit, ECGM_MULTICAST_DATA_BUFFERS_DROP_COUNTERr, value, MULTICAST_DATA_BUFFERS_DROP_COUNTERf); */

    /* Counts the number of multicast replications that were dropped due to reaching to the maximum port or queue length, where length is mesured in units of 256 bytes. */
    READ_REGISTER_NO_PIPE(ECGM_MULTICAST_REPLICATIONS_QUEUE_LENGTH_DROP_COUNTERr, counters->mc_rep_db_dropped);
    /* counters->mc_rep_db_dropped = soc_reg_field_get(unit, ECGM_MULTICAST_REPLICATIONS_QUEUE_LENGTH_DROP_COUNTERr, value, MULTICAST_REPLICATIONS_QUEUE_LENGTH_DROP_COUNTER)f; */
  }

exit:
#ifdef BCM_88675_A0
    if (SOC_IS_JERICHO(unit)) {
        /* Restore to previous value if didn't do that already */
        if (dynamic_mem_access) {
            if (WRITE_EGQ_ENABLE_DYNAMIC_MEMORY_ACCESSr(unit, core, 0) != SOC_E_NONE) {
                DNXC_EXIT_WITH_ERR(SOC_E_FAIL, (_BSL_DNXC_MSG("Failed to write EGQ_ENABLE_DYNAMIC_MEMORY_ACCESSr at unit %d core %d"), unit, core));
            }
        }
    }
#endif

    /* disable maximum statistics updated if requested to do so */
    if (updates_are_disabled) {
        if(WRITE_ECGM_STATISTICS_TRACKING_CONTROLr(unit, 0) != SOC_E_NONE) {
            LOG_ERROR(BSL_LS_SOC_EGRESS,
                    (BSL_META_U(unit,
                            "Failed write to register ECGM_STATISTICS_TRACKING_CONTRO")));
        }
    }

    if (dma_buf) { /* free DMA memory if it was allocated */
        soc_cm_sfree(unit, dma_buf);
    }
    DNXC_FUNC_RETURN;
}
