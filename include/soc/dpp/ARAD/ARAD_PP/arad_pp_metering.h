/* $Id: arad_pp_metering.h,v 1.16 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/

#ifndef __ARAD_PP_METERING_INCLUDED__
/* { */
#define __ARAD_PP_METERING_INCLUDED__

/*************
 * INCLUDES  *
 *************/
/* { */

#include <soc/dpp/SAND/Utils/sand_header.h>

#include <soc/dpp/SAND/Management/sand_general_macros.h>
#include <soc/dpp/SAND/Management/sand_error_code.h>

#include <soc/dpp/ARAD/ARAD_PP/arad_pp_framework.h>

#include <soc/dpp/PPC/ppc_api_metering.h>

/* } */
/*************
 * DEFINES   *
 *************/
/* { */

#define ARAD_PP_SW_DB_MULTI_SET_ETH_POLICER_PROFILE_METER_PROFILE_NOF_MEMBER 32
#define ARAD_PP_SW_DB_MULTI_SET_ETH_POLICER_GLOBAL_METER_PROFILE_NOF_MEMBER 8
/* IDR_ETHERNET_METER_CONFIGm has 1300 entry */
#define ARAD_PP_SW_DB_MULTI_SET_ETH_POLICER_CONFIG_METER_PROFILE_NOF_MEMBER_BIT 1300
#define ARAD_PP_SW_DB_MULTI_SET_ETH_POLICER_CONFIG_METER_PROFILE_NOF_MEMBER_BYTE 41
/* IDR_GLOBAL_METER_PROFILESm has 8 entry */
#define ARAD_PP_SW_DB_MULTI_SET_ETH_POLICER_ENABLE_STATUS_BIT_MAIN 1308

#define ARAD_BUCKET_EXP_MAX_VALUE (15)
#define ARAD_BUCKET_MANTISSA_MAX_VALUE (63)

/*************
 * MACROS    *
 *************/
/* { */

#define ARAD_PP_MTR_MEMORY(unit, memory)				SOC_IS_ARADPLUS_AND_BELOW(unit)? IDR_##memory : (SOC_IS_QAX(unit)? IMP_##memory: MRPS_##memory)
#define ARAD_PP_ETH_MTR_MEMORY(unit, memory)			SOC_IS_ARADPLUS_AND_BELOW(unit)? IDR_##memory : (SOC_IS_QAX(unit)? IEP_##memory: MTRPS_EM_##memory)
#define ARAD_PP_MTR_REG_PORT(unit, core_id)             SOC_IS_ARADPLUS_AND_BELOW(unit)? REG_PORT_ANY : (SOC_IS_QAX(unit)? REG_PORT_ANY: core_id)
#define ARAD_PP_MTR_MEM_BLOCK(unit, core_id)       		SOC_IS_ARADPLUS_AND_BELOW(unit)? MEM_BLOCK_ANY :(SOC_IS_QAX(unit)? MEM_BLOCK_ANY: MRPS_BLOCK(unit, core_id))
#define ARAD_PP_ETH_MTR_MEM_BLOCK(unit, core_id)       	SOC_IS_ARADPLUS_AND_BELOW(unit)? MEM_BLOCK_ANY :(SOC_IS_QAX(unit)? MEM_BLOCK_ANY: MTRPS_EM_BLOCK(unit, core_id))

#define JER_PP_MTR_MEMORY(unit, memory)					SOC_IS_QAX(unit)? IMP_##memory : MRPS_##memory

/*
	Convert bucket level from 23bit 2s compliment to signed integer
*/
#define ARAD_PP_MTR_BUCKET_TO_DECIMAL(__twos, __decimal) do {  \
				__decimal =  ((1 << 22) & (__twos)) ? (-1 * (((__twos) ^ 0x7FFFFF) + 1)) : (__twos); \
			} while (0)
/*
	Convert bucket level from unsigned to 23bit 2s compliment
*/
#define ARAD_PP_MTR_DECIMAL_TO_BUKCET(__decimal, __twos) do {  \
                __twos = /*((__decimal) < 0) ? ((((-1 * (__decimal)) & 0x7FFFFF) ^ 0x7FFFFF) + 1) :*/ ((__decimal) & 0x3FFFFF); \
			} while (0) ; \
            if(__twos <= 64) { \
                __twos = 0x7FE000; \
            }


typedef struct
{
  uint32 color_aware;
  uint32 coupling_flag;
  uint32 sharing_flag;
  uint32 cir_mantissa;
  uint32 cir_mantissa_exponent;
  uint32 reset_cir;
  uint32 cbs_mantissa_64;
  uint32 cbs_exponent;
  uint32 eir_mantissa;
  uint32 eir_mantissa_exponent;
  uint32 max_eir_mantissa;
  uint32 max_eir_mantissa_exponent;
  uint32 reset_eir;
  uint32 ebs_mantissa_64;
  uint32 ebs_exponent;

#ifdef BCM_88660_A0
  uint32 packet_mode;
#endif
} __ATTRIBUTE_PACKED__ ARAD_IDR_PRFCFG_TBL_DATA;

typedef struct
{
  uint32 rate_mantissa;
  uint32 rate_exp;
  uint32 burst_mantissa;
  uint32 burst_exp;
  uint32 packet_mode;
  uint32 meter_resolution;
  uint32 color_blind;
  uint32 pkt_adj_header_truncate;
} __ATTRIBUTE_PACKED__ ARAD_IDR_ETHERNET_METER_PROFILES_TBL_DATA;

typedef struct
{
  uint32 profile;
  uint32 global_meter_ptr;
  uint32 enable;
  uint32 parity;
} __ATTRIBUTE_PACKED__ ARAD_IDR_ETHERNET_METER_CONFIG_TBL_DATA;

typedef struct
{
  uint32 rate_mantissa;
  uint32 rate_exp;
  uint32 burst_mantissa;
  uint32 burst_exp;
  uint32 packet_mode;
  uint32 meter_resolution;
  uint32 enable;
  uint32 color_blind;
} __ATTRIBUTE_PACKED__ ARAD_IDR_GLOBAL_METER_PROFILES_TBL_DATA;
/* } */
/*************
 * MACROS    *
 *************/
/* { */

#define ARAD_PP_MTR_CORES_ITER(index) \
    for(index = 0; index < SOC_DPP_CONFIG(unit)->meter.nof_meter_cores; index++)
#define ARAD_PP_MTR_IS_SINGLE_CORE(unit)              (SOC_DPP_CONFIG(unit)->meter.nof_meter_cores == 1)

/* } */
/*************
 * TYPE DEFS *
 *************/
/* { */

typedef enum
{
  /*
   * Auto generated. Do not edit following section {
   */
  ARAD_PP_MTR_METERS_GROUP_INFO_SET = ARAD_PP_PROC_DESC_BASE_METERING_FIRST,
  ARAD_PP_MTR_METERS_GROUP_INFO_SET_PRINT,
  ARAD_PP_MTR_METERS_GROUP_INFO_SET_UNSAFE,
  ARAD_PP_MTR_METERS_GROUP_INFO_SET_VERIFY,
  ARAD_PP_MTR_METERS_GROUP_INFO_GET,
  ARAD_PP_MTR_METERS_GROUP_INFO_GET_PRINT,
  ARAD_PP_MTR_METERS_GROUP_INFO_GET_VERIFY,
  ARAD_PP_MTR_METERS_GROUP_INFO_GET_UNSAFE,
  ARAD_PP_MTR_BW_PROFILE_ADD,
  ARAD_PP_MTR_BW_PROFILE_ADD_PRINT,
  ARAD_PP_MTR_BW_PROFILE_ADD_UNSAFE,
  ARAD_PP_MTR_BW_PROFILE_ADD_VERIFY,
  ARAD_PP_MTR_BW_PROFILE_GET,
  ARAD_PP_MTR_BW_PROFILE_GET_PRINT,
  ARAD_PP_MTR_BW_PROFILE_GET_UNSAFE,
  ARAD_PP_MTR_BW_PROFILE_GET_VERIFY,
  ARAD_PP_MTR_BW_PROFILE_REMOVE,
  ARAD_PP_MTR_BW_PROFILE_REMOVE_PRINT,
  ARAD_PP_MTR_BW_PROFILE_REMOVE_UNSAFE,
  ARAD_PP_MTR_BW_PROFILE_REMOVE_VERIFY,
  ARAD_PP_MTR_METER_INS_TO_BW_PROFILE_MAP_SET,
  ARAD_PP_MTR_METER_INS_TO_BW_PROFILE_MAP_SET_PRINT,
  ARAD_PP_MTR_METER_INS_TO_BW_PROFILE_MAP_SET_UNSAFE,
  ARAD_PP_MTR_METER_INS_TO_BW_PROFILE_MAP_SET_VERIFY,
  ARAD_PP_MTR_METER_INS_TO_BW_PROFILE_MAP_GET,
  ARAD_PP_MTR_METER_INS_TO_BW_PROFILE_MAP_GET_PRINT,
  ARAD_PP_MTR_METER_INS_TO_BW_PROFILE_MAP_GET_VERIFY,
  ARAD_PP_MTR_METER_INS_TO_BW_PROFILE_MAP_GET_UNSAFE,
  ARAD_PP_MTR_ETH_POLICER_ENABLE_SET,
  ARAD_PP_MTR_ETH_POLICER_ENABLE_SET_PRINT,
  ARAD_PP_MTR_ETH_POLICER_ENABLE_SET_UNSAFE,
  ARAD_PP_MTR_ETH_POLICER_ENABLE_SET_VERIFY,
  ARAD_PP_MTR_ETH_POLICER_ENABLE_GET,
  ARAD_PP_MTR_ETH_POLICER_ENABLE_GET_PRINT,
  ARAD_PP_MTR_ETH_POLICER_ENABLE_GET_VERIFY,
  ARAD_PP_MTR_ETH_POLICER_ENABLE_GET_UNSAFE,
  ARAD_PP_MTR_ETH_POLICER_PARAMS_SET,
  ARAD_PP_MTR_ETH_POLICER_PARAMS_SET_PRINT,
  ARAD_PP_MTR_ETH_POLICER_PARAMS_SET_UNSAFE,
  ARAD_PP_MTR_ETH_POLICER_PARAMS_SET_VERIFY,
  ARAD_PP_MTR_ETH_POLICER_PARAMS_GET,
  ARAD_PP_MTR_ETH_POLICER_PARAMS_GET_PRINT,
  ARAD_PP_MTR_ETH_POLICER_PARAMS_GET_VERIFY,
  ARAD_PP_MTR_ETH_POLICER_PARAMS_GET_UNSAFE,
  ARAD_PP_METERING_INIT_UNSAFE,
  ARAD_PP_MTR_ETH_POLICER_GLBL_PROFILE_SET,
  ARAD_PP_MTR_ETH_POLICER_GLBL_PROFILE_SET_PRINT,
  ARAD_PP_MTR_ETH_POLICER_GLBL_PROFILE_SET_UNSAFE,
  ARAD_PP_MTR_ETH_POLICER_GLBL_PROFILE_SET_VERIFY,
  ARAD_PP_MTR_ETH_POLICER_GLBL_PROFILE_GET,
  ARAD_PP_MTR_ETH_POLICER_GLBL_PROFILE_GET_PRINT,
  ARAD_PP_MTR_ETH_POLICER_GLBL_PROFILE_GET_VERIFY,
  ARAD_PP_MTR_ETH_POLICER_GLBL_PROFILE_GET_UNSAFE,
  ARAD_PP_MTR_ETH_POLICER_GLBL_PROFILE_MAP_SET,
  ARAD_PP_MTR_ETH_POLICER_GLBL_PROFILE_MAP_SET_PRINT,
  ARAD_PP_MTR_ETH_POLICER_GLBL_PROFILE_MAP_SET_UNSAFE,
  ARAD_PP_MTR_ETH_POLICER_GLBL_PROFILE_MAP_SET_VERIFY,
  ARAD_PP_MTR_ETH_POLICER_GLBL_PROFILE_MAP_GET,
  ARAD_PP_MTR_ETH_POLICER_GLBL_PROFILE_MAP_GET_PRINT,
  ARAD_PP_MTR_ETH_POLICER_GLBL_PROFILE_MAP_GET_VERIFY,
  ARAD_PP_MTR_ETH_POLICER_GLBL_PROFILE_MAP_GET_UNSAFE,
  ARAD_PP_METERING_GET_PROCS_PTR,
  ARAD_PP_METERING_GET_ERRS_PTR,
  /*
   * } Auto generated. Do not edit previous section.
   */
  ARAD_PP_MTR_IR_VAL_FROM_REVERSE_EXP_MNT,
  ARAD_PP_MTR_BS_VAL_TO_EXP_MNT,
  ARAD_PP_MTR_CHK_PROFILE_RATE,
  ARAD_PP_MTR_IR_VAL_TO_MAX_REV_EXP,
  ARAD_PP_MTR_PROFILE_RATE_TO_RES_EXP_MNT,
  ARAD_PP_METERING_PCD_INIT,

  /*
   * Last element. Do no touch.
   */
  ARAD_PP_METERING_PROCEDURE_DESC_LAST
} ARAD_PP_METERING_PROCEDURE_DESC;

typedef enum
{
  /*
   * Auto generated. Do not edit following section {
   */
  ARAD_PP_METERING_MTR_GROUP_NDX_OUT_OF_RANGE_ERR = ARAD_PP_ERR_DESC_BASE_METERING_FIRST,
  ARAD_PP_METERING_BW_PROFILE_NDX_OUT_OF_RANGE_ERR,
  ARAD_PP_METERING_SUCCESS_OUT_OF_RANGE_ERR,
  ARAD_PP_METERING_BW_PROFILE_ID_OUT_OF_RANGE_ERR,
  ARAD_PP_METERING_RESULT_USE_OUT_OF_RANGE_ERR,
  ARAD_PP_METERING_METERED_TYPE_NDX_OUT_OF_RANGE_ERR,
  ARAD_PP_METERING_FIRST_METERED_NDX_OUT_OF_RANGE_ERR,
  ARAD_PP_METERING_NOF_METER_INS_OUT_OF_RANGE_ERR,
  ARAD_PP_METERING_ETH_TYPE_NDX_OUT_OF_RANGE_ERR,
  ARAD_PP_METERING_GLBL_PROFILE_IDX_OUT_OF_RANGE_ERR,
  ARAD_PP_METERING_GROUP_OUT_OF_RANGE_ERR,
  ARAD_PP_METERING_ID_OUT_OF_RANGE_ERR,
  ARAD_PP_METERING_IS_HR_ENABLED_OUT_OF_RANGE_ERR,
  ARAD_PP_METERING_MAX_PACKET_SIZE_OUT_OF_RANGE_ERR,
  ARAD_PP_METERING_CIR_OUT_OF_RANGE_ERR,
  ARAD_PP_METERING_EIR_OUT_OF_RANGE_ERR,
  ARAD_PP_METERING_CBS_OUT_OF_RANGE_ERR,
  ARAD_PP_METERING_EBS_OUT_OF_RANGE_ERR,
  ARAD_PP_METERING_COLOR_MODE_OUT_OF_RANGE_ERR,
  ARAD_PP_METERING_TYPE_OUT_OF_RANGE_ERR,
  /*
   * } Auto generated. Do not edit previous section.
   */
  
  ARAD_PP_METERING_CHK_PROFILE_RATE_ERR,
  ARAD_PP_METERING_VAL_FROM_REVERSE_EXP_MNT_CLK_OF_ERR,
  ARAD_PP_METERING_UPDATE_ALL_COPIES_MAX_RANGE_ERR,
  ARAD_PP_METERING_BW_PROFILE_HR_RATE_ERR,
  ARAD_PP_METERING_BW_PROFILE_LR_RATE_ERR,
  ARAD_PP_MTR_ETH_POLICER_ADD_FAIL_ERR,
  ARAD_PP_MTR_ETH_POLICER_PARAMS_SET_VERIFY_OUT_OF_RANGE_ERR,
  ARAD_PP_METERING_POLICER_INGRESS_COUNT_INVALID_ERR,
  ARAD_PP_METERING_POLICER_INGRESS_SHARING_MODE_INVALID_ERR,
  ARAD_PP_METERING_MAX_IR_INVALID_ERR,
  ARAD_PP_METERING_POLICER_RESULT_PARALLEL_COLOR_MAPPING_INVALID_ERR,
  ARAD_PP_METERING_POLICER_RESULT_PARALLEL_BUCKET_UPDATE_INVALID_ERR,
  
  /*
   * Last element. Do no touch.
   */
  ARAD_PP_METERING_ERR_LAST
} ARAD_PP_METERING_ERR;

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

int
  arad_pp_mtr_translate_kbits_to_packets(
    SOC_SAND_IN int                       unit,
	SOC_SAND_IN int                       kbits
	);

int
  arad_pp_mtr_translate_packets_to_kbits(
    SOC_SAND_IN int                       unit,
	SOC_SAND_IN int                       packets
	);

uint32
  arad_pp_mtr_profile_rate_to_res_exp_mnt(
    SOC_SAND_IN int                       unit,
    SOC_SAND_IN uint32                       val,
    SOC_SAND_OUT uint32                      *rev_exp,
    SOC_SAND_OUT uint32                      *mnt,
    SOC_SAND_OUT uint32                      *exp
  );

uint32
  arad_pp_mtr_profile_burst_to_exp_mnt(
    SOC_SAND_IN int                       unit,
    SOC_SAND_IN uint32                       val,
    SOC_SAND_OUT uint32                      resolution, /* rate resolution */
    SOC_SAND_OUT uint32                      *mnt,
    SOC_SAND_OUT uint32                      *exp
  );

uint32
  arad_pp_mtr_ir_val_from_reverse_exp_mnt(
    SOC_SAND_IN int                       unit,
    SOC_SAND_IN uint32                       rev_exp,
    SOC_SAND_IN uint32                       mant_exp,
    SOC_SAND_IN uint32                       mnt,
    SOC_SAND_OUT uint32                      *val
  );

uint32
  arad_pp_mtr_bs_val_to_exp_mnt(
    SOC_SAND_IN int                       unit,
    SOC_SAND_IN uint32                       val,
    SOC_SAND_OUT uint32                      *exp,
    SOC_SAND_OUT uint32                      *mnt
  );

void
  arad_pp_mtr_verify_valid_bucket_size(
    SOC_SAND_IN 	int			unit,
    SOC_SAND_IN 	uint32      rate1_man,
    SOC_SAND_IN 	uint32      rate1_exp,
    SOC_SAND_IN 	uint32      rate2_man,
    SOC_SAND_IN 	uint32      rate2_exp,
    SOC_SAND_INOUT	uint32      *bucket_man,
    SOC_SAND_INOUT 	uint32      *bucket_exp
  );

void
  arad_pp_mtr_eth_policer_profile_key_get(
    SOC_SAND_IN  int                                               unit,
    SOC_SAND_IN  ARAD_IDR_ETHERNET_METER_PROFILES_TBL_DATA         profile_tbl_data,
    SOC_SAND_OUT uint32                                            *eth_mtr_profile_multiset_key
  );

uint32
  arad_pp_metering_init_unsafe(
    SOC_SAND_IN  int                                 unit
  );

/*********************************************************************
* NAME:
 *   arad_pp_metering_pcd_entry_set
 * TYPE:
 *   PROC
 * FUNCTION:
 * Sets a row in the pcd table which maps the result of two meters to
 * final metering color and in addition insturct each meter from which bucket
 * to reduce credits.
 * INPUT:
 *   SOC_SAND_IN  int                                 unit
 *   SOC_SAND_IN  SOC_PPC_MTR_COLOR_DECISION_INFO     *pcd_entry
 * REMARKS:
 * 
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32 arad_pp_metering_pcd_entry_set(
	SOC_SAND_IN int                                  unit, 
	SOC_SAND_IN SOC_PPC_MTR_COLOR_DECISION_INFO      *pcd_entry
  );

uint32 arad_pp_metering_pcd_entry_get(
	SOC_SAND_IN int                                  unit, 
	SOC_SAND_OUT SOC_PPC_MTR_COLOR_DECISION_INFO      *pcd_entry
  );

/*********************************************************************
* NAME:
 *   arad_pp_metering_dp_map_entry_set
 * TYPE:
 *   PROC
 * FUNCTION:
 * Sets a row in the dp map table which maps the input color, metering color,
 * meter-commandto ingress and egress color.
 * The defaults for this table are set in arad_itm_setup_dp_map
 * INPUT:
 *   SOC_SAND_IN  int                                 unit
 *   SOC_SAND_IN  SOC_PPC_MTR_COLOR_RESOLUTION_INFO   *dp_map_entry
 * REMARKS:
 * 
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32 arad_pp_metering_dp_map_entry_set(
	SOC_SAND_IN int                                  unit, 
	SOC_SAND_IN SOC_PPC_MTR_COLOR_RESOLUTION_INFO     *dp_map_entry
  );

uint32 arad_pp_metering_dp_map_entry_get(
	SOC_SAND_IN int                                  unit, 
	SOC_SAND_OUT SOC_PPC_MTR_COLOR_RESOLUTION_INFO     *dp_map_entry
  );

/*********************************************************************
* NAME:
 *   arad_pp_metering_pcd_init
 * TYPE:
 *   PROC
 * FUNCTION:
 * Initialize and set the PCD (parallel color decision) map.
 * Used both to allow more flexibility whe deciding on the out 
 * color and charged bucket in case of Parallel metetring and to
 * verify both updated buckets have sufficent credits in Serial 
 * metering. 
 * INPUT:
 *   SOC_SAND_IN  int                                 unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  uint8                               sharing_mode
 * REMARKS:
 * 
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32 arad_pp_metering_pcd_init(
	SOC_SAND_IN int                                  unit, 
	SOC_SAND_IN uint8                                sharing_mode
  );


/*********************************************************************
* NAME:
 *   arad_pp_mtr_meters_group_info_set_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Sets
 * INPUT:
 *   SOC_SAND_IN  int                                 unit -
 *     Identifier of the device to access.
  *   SOC_SAND_IN  int                                 core_id -
 *     Meter core
 *   SOC_SAND_IN  uint32                               mtr_group_ndx -
 *     Meters Group. Range 0 - 1.
 *   SOC_SAND_IN  SOC_PPC_MTR_GROUP_INFO               *mtr_group_info -
 *     Per metering group information
 * REMARKS:
 *   - Relevant only for Arad-B.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_mtr_meters_group_info_set_unsafe(
    SOC_SAND_IN  int                                  unit,
	SOC_SAND_IN  int                                     core_id,
    SOC_SAND_IN  uint32                                  mtr_group_ndx,
    SOC_SAND_IN  SOC_PPC_MTR_GROUP_INFO                  *mtr_group_info
  );

uint32
  arad_pp_mtr_meters_group_info_set_verify(
    SOC_SAND_IN  int                                  unit,
	SOC_SAND_IN  int                                     core_id,
    SOC_SAND_IN  uint32                                  mtr_group_ndx,
    SOC_SAND_IN  SOC_PPC_MTR_GROUP_INFO                  *mtr_group_info
  );

uint32
  arad_pp_mtr_meters_group_info_get_verify(
    SOC_SAND_IN  int                                  unit,
	SOC_SAND_IN  int                                     core_id,
    SOC_SAND_IN  uint32                                  mtr_group_ndx
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "arad_pp_mtr_meters_group_info_set_unsafe" API.
 *     Refer to "arad_pp_mtr_meters_group_info_set_unsafe" API
 *     for details.
*********************************************************************/
uint32
  arad_pp_mtr_meters_group_info_get_unsafe(
    SOC_SAND_IN  int                                  unit,
	SOC_SAND_IN  int                                     core_id,
    SOC_SAND_IN  uint32                                  mtr_group_ndx,
    SOC_SAND_OUT SOC_PPC_MTR_GROUP_INFO                  *mtr_group_info
  );

/*********************************************************************
* NAME:
 *   arad_pp_mtr_bw_profile_add_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Add Bandwidth Profile and set it attributes
 * INPUT:
 *   SOC_SAND_IN  int                                 unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  int                                     core_id -
 *     Meter core ID.
 *   SOC_SAND_IN  uint32                                  mtr_group_ndx -
 *     Meters Group. In T20E has to be zero. In Arad-B Range 0
 *     - 1.
 *   SOC_SAND_IN  uint32                                  bw_profile_ndx -
 *     Bandwidth Profile ID
 *   SOC_SAND_IN  SOC_PPC_MTR_BW_PROFILE_INFO             *bw_profile_info -
 *     Bandwidth profile attributes
 *   SOC_SAND_OUT SOC_PPC_MTR_BW_PROFILE_INFO             *exact_bw_profile_info -
 *     Exact Bandwidth profile attributes as written to the
 *     device.
 *   SOC_SAND_OUT SOC_SAND_SUCCESS_FAILURE                      *success -
 *     Whether the operation succeeds (upon add). Operation may
 *     fail upon unavailable resources. In Arad-B this
 *     operation always success.
 * REMARKS:
 *   This function configures bandwith profile, to assign
 *   meter instance with this profile use
 *   soc_ppd_mtr_meter_ins_to_bw_profile_map_set()- Arad-B if
 *   High-rate metering is enabled then - 0-447 are use for
 *   normal profiles - 448-511 used for high rate profile. if
 *   High-rate metering is disabled then - 0-511 are use for
 *   normal profiles In Normal Profile: Information Rates
 *   (CIR and EIR) are comprised between 64 Kbps and 19 Gbps.
 *   The burst sizes (CBS and EBS) are comprised between 64B
 *   and 1,040,384B. In High-rate Profile: Information Rates
 *   (CIR and EIR) are between 9.6 Gbps and 120 Gbps. The
 *   burst sizes (CBS and EBS) are comprised between 64B and
 *   4,161,536B
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_mtr_bw_profile_add_unsafe(
    SOC_SAND_IN  int                                   unit,
	SOC_SAND_IN  int                                     core_id,
    SOC_SAND_IN  uint32                                  mtr_group_ndx,
    SOC_SAND_IN  uint32                                  bw_profile_ndx,
    SOC_SAND_IN  SOC_PPC_MTR_BW_PROFILE_INFO             *bw_profile_info,
    SOC_SAND_OUT SOC_PPC_MTR_BW_PROFILE_INFO             *exact_bw_profile_info,
    SOC_SAND_OUT SOC_SAND_SUCCESS_FAILURE                *success
  );

uint32
  arad_pp_mtr_bw_profile_add_verify(
    SOC_SAND_IN  int                                  unit,
	SOC_SAND_IN  int                                     core_id,
    SOC_SAND_IN  uint32                                  mtr_group_ndx,
    SOC_SAND_IN  uint32                                  bw_profile_ndx,
    SOC_SAND_IN  SOC_PPC_MTR_BW_PROFILE_INFO             *bw_profile_info
  );

/*********************************************************************
* NAME:
 *   arad_pp_mtr_bw_profile_get_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Get Bandwidth Profile attributes
 * INPUT:
 *   SOC_SAND_IN  int                                 unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  int                                     core_id -
 *     Meter core ID.
 *   SOC_SAND_IN  uint32                                  mtr_group_ndx -
 *     Meters Group. In T20E has to be zero.
 *     In Petra-B Range 0- 1.
 *   SOC_SAND_IN  uint32                                  bw_profile_ndx -
 *     Bandwidth Profile ID
 *   SOC_SAND_OUT SOC_PPC_MTR_BW_PROFILE_INFO             *bw_profile_info -
 *     Bandwidth profile attributes
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_mtr_bw_profile_get_unsafe(
    SOC_SAND_IN  int                                  unit,
	SOC_SAND_IN  int                                     core_id,
    SOC_SAND_IN  uint32                                  mtr_group_ndx,
    SOC_SAND_IN  uint32                                  bw_profile_ndx,
    SOC_SAND_OUT SOC_PPC_MTR_BW_PROFILE_INFO             *bw_profile_info
  );

uint32
  arad_pp_mtr_bw_profile_get_verify(
    SOC_SAND_IN  int                                  unit,
    SOC_SAND_IN  int                                     core_id,
    SOC_SAND_IN  uint32                                  mtr_group_ndx,
    SOC_SAND_IN  uint32                                  bw_profile_ndx
  );

/*********************************************************************
* NAME:
 *   arad_pp_mtr_bw_profile_remove_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Remove Bandwidth Profile
 * INPUT:
 *   SOC_SAND_IN  int                                 unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  int                                     core_id -
 *     Meter core ID.
 *   SOC_SAND_IN  uint32                                  mtr_group_ndx -
 *     Meters Group. In T20E has to be zero. In Arad-B Range 0
 *     - 1.
 *   SOC_SAND_IN  uint32                                  bw_profile_ndx -
 *     Bandwidth Profile ID
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_mtr_bw_profile_remove_unsafe(
    SOC_SAND_IN  int                                  unit,
	SOC_SAND_IN  int                                     core_id,
    SOC_SAND_IN  uint32                                  mtr_group_ndx,
    SOC_SAND_IN  uint32                                  bw_profile_ndx
  );

uint32
  arad_pp_mtr_bw_profile_remove_verify(
    SOC_SAND_IN  int                                  unit,
	SOC_SAND_IN  int                                     core_id,
    SOC_SAND_IN  uint32                                  mtr_group_ndx,
    SOC_SAND_IN  uint32                                  bw_profile_ndx
  );

/*********************************************************************
* NAME:
 *   arad_pp_mtr_meter_ins_to_bw_profile_map_set_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Set meter attributes by mapping meter instance to
 *   bandwidth profile.
 * INPUT:
 *   SOC_SAND_IN  int                                 unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  int                                     core_id -
 *     Meter core ID.
 *   SOC_SAND_IN  SOC_PPC_MTR_METER_ID                    *meter_ins_ndx -
 *     Metering Instance ID
 *   SOC_SAND_IN  uint32                                  bw_profile_id -
 *     bandwidth profile ID.
 * REMARKS:
 *   - in T20E the group in SOC_PPC_MTR_METER_ID has to be zero-
 *   in Arad-B the meter instance mapped into profile in the
 *   same group the meter instance belongs to.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_mtr_meter_ins_to_bw_profile_map_set_unsafe(
    SOC_SAND_IN  int                                  unit,
	SOC_SAND_IN  int                                     core_id,
    SOC_SAND_IN  SOC_PPC_MTR_METER_ID                    *meter_ins_ndx,
    SOC_SAND_IN  uint32                                  bw_profile_id
  );

uint32
  arad_pp_mtr_meter_ins_to_bw_profile_map_set_verify(
    SOC_SAND_IN  int                                  unit,
	SOC_SAND_IN  int                                     core_id,
    SOC_SAND_IN  SOC_PPC_MTR_METER_ID                    *meter_ins_ndx,
    SOC_SAND_IN  uint32                                  bw_profile_id
  );

uint32
  arad_pp_mtr_meter_ins_to_bw_profile_map_get_verify(
    SOC_SAND_IN  int                                 unit,
	SOC_SAND_IN  int                                    core_id,
    SOC_SAND_IN  SOC_PPC_MTR_METER_ID                   *meter_ins_ndx
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "arad_pp_mtr_meter_ins_to_bw_profile_map_set_unsafe" API.
 *     Refer to
 *     "arad_pp_mtr_meter_ins_to_bw_profile_map_set_unsafe" API
 *     for details.
*********************************************************************/
uint32
  arad_pp_mtr_meter_ins_to_bw_profile_map_get_unsafe(
    SOC_SAND_IN  int                                  unit,
	SOC_SAND_IN  int                                     core_id,
    SOC_SAND_IN  SOC_PPC_MTR_METER_ID                    *meter_ins_ndx,
    SOC_SAND_OUT uint32                                  *bw_profile_id
  );

/*********************************************************************
* NAME:
 *   arad_pp_mtr_meter_ins_bucket_get
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Get a meter id and core, returns the current committed and excess
 *	 bucket levels.
 * INPUT:
 *  int                                     unit -
 *     Identifier of the device to access.
 *  int                                     core_id -
 *     Meter core ID.
 *  SOC_PPC_MTR_METER_ID                    *meter_ins_ndx -
 *     Metering Instance ID
 *  int                                    *cbl -
 *     Committed bucket level
 *  int                                    *ebl -
 *     Excess bucket level
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
soc_error_t
  arad_pp_mtr_meter_ins_bucket_get(
    int                                  unit,
	int                                  core_id,
    SOC_PPC_MTR_METER_ID                 *meter_ins_ndx,
    int                                  *cbl,
	int                                  *ebl
  );

/*********************************************************************
* NAME:
 *   arad_pp_mtr_eth_policer_enable_set_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Enable / Disable Ethernet policing.
 * INPUT:
 *   SOC_SAND_IN  int                                 unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  uint8                                 enable -
 *     TRUE: Enable Ethernet policing.
 * REMARKS:
 *   - Arad-B only, if called for T20E error is returned.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_mtr_eth_policer_enable_set_unsafe(
    SOC_SAND_IN  int                                unit,
    SOC_SAND_IN  uint8                                 enable
  );

uint32
  arad_pp_mtr_eth_policer_enable_set_verify(
    SOC_SAND_IN  int                                unit,
    SOC_SAND_IN  uint8                                 enable
  );

uint32
  arad_pp_mtr_eth_policer_enable_get_verify(
    SOC_SAND_IN  int                                 unit
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "arad_pp_mtr_eth_policer_enable_set_unsafe" API.
 *     Refer to "arad_pp_mtr_eth_policer_enable_set_unsafe" API
 *     for details.
*********************************************************************/
uint32
  arad_pp_mtr_eth_policer_enable_get_unsafe(
    SOC_SAND_IN  int                                unit,
    SOC_SAND_OUT uint8                                 *enable
  );

/*********************************************************************
* NAME:
 *   arad_pp_mtr_eth_policer_params_set_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Set policer attributes of the Ethernet policer. Enable
 *   policing per ingress port and Ethernet type.
 * INPUT:
 *   SOC_SAND_IN  int                                 unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_PORT                                port_ndx -
 *     Port ID
 *   SOC_SAND_IN  SOC_PPC_MTR_ETH_TYPE                        eth_type_ndx -
 *     Ethernet traffic type (UC/BC/...)
 *   SOC_SAND_IN  SOC_PPC_MTR_BW_PROFILE_INFO                 *policer_info -
 *     Policer attributes
 * REMARKS:
 *   - Arad-B only, if called for T20E error is returned.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_mtr_eth_policer_params_set_unsafe(
    SOC_SAND_IN  int                                      unit,
    SOC_SAND_IN  SOC_PPC_PORT                                port_ndx,
    SOC_SAND_IN  SOC_PPC_MTR_ETH_TYPE                        eth_type_ndx,
    SOC_SAND_IN  SOC_PPC_MTR_BW_PROFILE_INFO                 *policer_info
  );

uint32
  arad_pp_mtr_eth_policer_params_set_verify(
    SOC_SAND_IN  int                                      unit,
    SOC_SAND_IN  SOC_PPC_PORT                                port_ndx,
    SOC_SAND_IN  SOC_PPC_MTR_ETH_TYPE                        eth_type_ndx,
    SOC_SAND_IN  SOC_PPC_MTR_BW_PROFILE_INFO                 *policer_info
  );

uint32
  arad_pp_mtr_eth_policer_params_get_verify(
    SOC_SAND_IN  int                                      unit,
    SOC_SAND_IN  SOC_PPC_PORT                                port_ndx,
    SOC_SAND_IN  SOC_PPC_MTR_ETH_TYPE                        eth_type_ndx
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "arad_pp_mtr_eth_policer_params_set_unsafe" API.
 *     Refer to "arad_pp_mtr_eth_policer_params_set_unsafe" API
 *     for details.
*********************************************************************/
uint32
  arad_pp_mtr_eth_policer_params_get_unsafe(
    SOC_SAND_IN  int                                      unit,
    SOC_SAND_IN  SOC_PPC_PORT                                port_ndx,
    SOC_SAND_IN  SOC_PPC_MTR_ETH_TYPE                        eth_type_ndx,
    SOC_SAND_OUT SOC_PPC_MTR_BW_PROFILE_INFO                 *policer_info
  );

/*********************************************************************
* NAME:         
 *   arad_pp_mtr_eth_policer_glbl_profile_set_unsafe
 * TYPE:         
 *   PROC        
 * FUNCTION:       
 *   Set Ethernet policer Global Profile attributes.         
 * INPUT:
 *   SOC_SAND_IN  int                 unit - 
 *     Identifier of the device to access.                     
 *   SOC_SAND_IN  uint32                  glbl_profile_idx - 
 *     Global Profile index                                    
 *   SOC_SAND_IN  SOC_PPC_MTR_BW_PROFILE_INFO *policer_info - 
 *     Policer attributes                                      
 * REMARKS:         
 *   - uses only cir, cbs, cir_disable fileds from 
 *   SOC_PPC_MTR_BW_PROFILE_INFO to configure policer attributes. 
 * RETURNS:         
 *   OK or ERROR indication.
*********************************************************************/
uint32  
  arad_pp_mtr_eth_policer_glbl_profile_set_unsafe(
    SOC_SAND_IN  int                      unit,
    SOC_SAND_IN  uint32                      glbl_profile_idx,
    SOC_SAND_IN  SOC_PPC_MTR_BW_PROFILE_INFO *policer_info
  );

uint32  
  arad_pp_mtr_eth_policer_glbl_profile_set_verify(
    SOC_SAND_IN  int                       unit,
    SOC_SAND_IN  uint32                       glbl_profile_idx,
    SOC_SAND_IN  SOC_PPC_MTR_BW_PROFILE_INFO *policer_info
  );

uint32  
  arad_pp_mtr_eth_policer_glbl_profile_get_verify(
    SOC_SAND_IN  int                  unit,
    SOC_SAND_IN  uint32                  glbl_profile_idx
  );

/*********************************************************************
*     Gets the configuration set by the 
 *     "arad_pp_mtr_eth_policer_glbl_profile_set_unsafe" API.    
 *     Refer to "arad_pp_mtr_eth_policer_glbl_profile_set_unsafe" 
 *     API for details.                                        
*********************************************************************/
uint32  
  arad_pp_mtr_eth_policer_glbl_profile_get_unsafe(
    SOC_SAND_IN  int                      unit,
    SOC_SAND_IN  uint32                      glbl_profile_idx,
    SOC_SAND_OUT SOC_PPC_MTR_BW_PROFILE_INFO *policer_info
  );

/*********************************************************************
* NAME:         
 *   arad_pp_mtr_eth_policer_glbl_profile_map_set_unsafe
 * TYPE:         
 *   PROC        
 * FUNCTION:       
 *   Map Ethernet policer per ingress port and Ethernet type 
 *   to Ethernet policer Global Profile.                     
 * INPUT:
 *   SOC_SAND_IN  int                 unit - 
 *     Identifier of the device to access.                     
 *   SOC_SAND_IN  SOC_PPC_PORT                port_ndx - 
 *     Port ID                                                 
 *   SOC_SAND_IN  SOC_PPC_MTR_ETH_TYPE        eth_type_ndx - 
 *     Ethernet traffic type (UC/BC/...)                       
 *   SOC_SAND_IN  uint32                  glbl_profile_idx - 
 *     Global Profile index                                    
 * REMARKS:         
 *   None                                                    
 * RETURNS:         
 *   OK or ERROR indication.
*********************************************************************/
uint32  
  arad_pp_mtr_eth_policer_glbl_profile_map_set_unsafe(
    SOC_SAND_IN  int                      unit,
    SOC_SAND_IN  SOC_PPC_PORT                port_ndx,
    SOC_SAND_IN  SOC_PPC_MTR_ETH_TYPE        eth_type_ndx,
    SOC_SAND_IN  uint32                      glbl_profile_idx
  );

uint32  
  arad_pp_mtr_eth_policer_glbl_profile_map_set_verify(
    SOC_SAND_IN  int                      unit,
    SOC_SAND_IN  SOC_PPC_PORT                port_ndx,
    SOC_SAND_IN  SOC_PPC_MTR_ETH_TYPE        eth_type_ndx,
    SOC_SAND_IN  uint32                      glbl_profile_idx
  );

uint32  
  arad_pp_mtr_eth_policer_glbl_profile_map_get_verify(
    SOC_SAND_IN  int                      unit,
    SOC_SAND_IN  SOC_PPC_PORT                port_ndx,
    SOC_SAND_IN  SOC_PPC_MTR_ETH_TYPE        eth_type_ndx
  );

/*********************************************************************
*     Gets the configuration set by the 
 *     "arad_pp_mtr_eth_policer_glbl_profile_map_set_unsafe" API. 
 *     Refer to 
 *     "arad_pp_mtr_eth_policer_glbl_profile_map_set_unsafe" API 
 *     for details.                                            
*********************************************************************/
uint32  
  arad_pp_mtr_eth_policer_glbl_profile_map_get_unsafe(
    SOC_SAND_IN  int                      unit,
    SOC_SAND_IN  SOC_PPC_PORT                port_ndx,
    SOC_SAND_IN  SOC_PPC_MTR_ETH_TYPE        eth_type_ndx,
    SOC_SAND_OUT uint32                      *glbl_profile_idx
  );

/*********************************************************************
* NAME:
 *   arad_pp_metering_get_procs_ptr
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Get the pointer to the list of procedures of the
 *   arad_pp_api_metering module.
 * INPUT:
 * REMARKS:
 *
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
CONST SOC_PROCEDURE_DESC_ELEMENT*
  arad_pp_metering_get_procs_ptr(void);

/*********************************************************************
* NAME:
 *   arad_pp_metering_get_errs_ptr
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Get the pointer to the list of errors of the
 *   arad_pp_api_metering module.
 * INPUT:
 * REMARKS:
 *
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
CONST SOC_ERROR_DESC_ELEMENT*
  arad_pp_metering_get_errs_ptr(void);

uint32
  SOC_PPC_MTR_METER_ID_verify(
    SOC_SAND_IN  SOC_PPC_MTR_METER_ID *info
  );
uint32
  SOC_PPC_MTR_GROUP_INFO_verify(
    SOC_SAND_IN  SOC_PPC_MTR_GROUP_INFO *info
  );

uint32
  SOC_PPC_MTR_BW_PROFILE_INFO_verify(
    SOC_SAND_IN  int                         unit,
    SOC_SAND_IN  SOC_PPC_MTR_BW_PROFILE_INFO *info
  );

/*********************************************************************
* NAME:
*     soc_jer_pp_mtr_policer_global_sharing_get
* FUNCTION:
*     gets the global_sharing rate flag with respect to GLOBAL_SHARINGf
* INPUT:
*   int                         unit				- Identifier of the device to access
*   int                         core_id				- specify meter core - for MRPS block (used only in jericho)
*	int        					meter_id			- specify meter id
*	int							meter_group			- specify meter memory section
*	uint32* 					global_sharing_ptr
* RETURNS:
*   OK or ERROR indication.
* REMARKS:
*   Called as part of the reading info sequence.
*   Called from the mbcm_pp dispatcher.
*********************************************************************/
soc_error_t
  arad_pp_mtr_policer_global_sharing_get(
    int                         unit,
	int                         core_id,
	int        					meter_id,
	int							meter_group,
	uint32* 					global_sharing_ptr
);

/*********************************************************************
* NAME:
*     arad_pp_mtr_policer_global_sharing_set
* FUNCTION:
*     sets the global_sharing rate flag with respect to GLOBAL_SHARINGf
* INPUT:
*   int                         unit				- Identifier of the device to access
*   int                         core_id				- specify meter core - for MRPS block (used only in jericho)
*	int        					meter_id			- specify meter id
*	int							meter_group			- specify meter memory section
*	uint32* 					global_sharing_ptr
* RETURNS:
*   OK or ERROR indication.
* REMARKS:
*   Called as part of the reading info sequence.
*   Called from the mbcm_pp dispatcher.
*********************************************************************/
soc_error_t
  arad_pp_mtr_policer_global_sharing_set(
    int                         unit,
	int                         core_id,
	int        					meter_id,
	int							meter_group,
	uint32* 					global_sharing_ptr
);

/*********************************************************************
* NAME:
*     arad_pp_mtr_policer_ipg_compensation_set
* FUNCTION:
*     Enables the global IPG compensation
*     If TRUE then a constant value of 20 will be added to packet size for meter calculations.
*     If FALSE then nothing is added. 
* 	  This is relevant both for the ethernet policer and for the MRPS. Arad+ only.
*
* INPUT:
*   int                         unit
*   	- Identifier of the device to access
*   uint8						ipg_compensation_enabled
*   	- Should global compensation be enabled or not
* RETURNS:
*   OK or ERROR indication.
* REMARKS:
*   Called from the mbcm_pp dispatcher.
*********************************************************************/
uint32
arad_pp_mtr_policer_ipg_compensation_set(
    int                         unit,
	uint8						ipg_compensation_enabled
);

uint32
  arad_pp_mtr_policer_ipg_compensation_get(
    int                         unit,
	uint8						*ipg_compensation_enabled
);

/*********************************************************************
* NAME:
*     arad_pp_mtr_ir_val_to_max_rev_exp_optimized_for_bucket_rate
* FUNCTION:
* First calculate the highest shared rev_exp as described in arad_pp_mtr_ir_val_to_max_rev_exp. 
* Afterwards look for a lower rev_exp, as long as the accuracy of the rates are unaffected by it. 
*
* INPUT:
*   rates - The rev_exp found will match these rates.
*   nof_rates - The number of rates in the rates array.
*   rev_exp - The returned rev_exp.
* RETURNS:
*   OK or ERROR indication.
* REMARKS:
*   Typically we expect that the highest rev_exp has a high exp even for the lowest rate, which are 
*   directly eliminated without contributing to accuracy. 
*   Therefore if the exponent is positive, it can always be eliminated in HW along with the rev_exp. 
*   Afterwards the mantissa can also be optimized, however this may hurt accuracy (however the 
*   function tries this too as long as accuracy is unaffected). 

*********************************************************************/
uint32
  arad_pp_mtr_ir_val_to_max_rev_exp_optimized_for_bucket_rate(
    SOC_SAND_IN int                          unit,
    SOC_SAND_IN uint32                       *rates,
    SOC_SAND_IN uint32                       nof_rates,
    SOC_SAND_OUT uint32                      *rev_exp
  );

/* } */

#include <soc/dpp/SAND/Utils/sand_footer.h>

/* } __ARAD_PP_METERING_INCLUDED__*/
#endif

