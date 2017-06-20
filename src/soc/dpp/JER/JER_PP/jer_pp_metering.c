#include <shared/bsl.h>
#include <soc/mcm/memregs.h> 
#if defined(BCM_88675_A0)
/* $Id: jer_pp_metering.c,v 1.0 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/

#ifdef _ERR_MSG_MODULE_NAME
  #error "_ERR_MSG_MODULE_NAME redefined"
#endif

#define _ERR_MSG_MODULE_NAME BSL_SOC_METERING
#include <shared/bsl.h>
#include <shared/swstate/access/sw_state_access.h>
#include <soc/dcmn/error.h>


/*************
 * INCLUDES  *
 *************/
/* { */

#include <soc/dpp/SAND/Utils/sand_header.h>
#include <soc/dpp/SAND/Management/sand_general_macros.h>
#include <soc/dpp/SAND/Management/sand_error_code.h>
#include <soc/dpp/SAND/Utils/sand_os_interface.h>
#include <soc/dpp/SAND/Utils/sand_framework.h>

#include <soc/dpp/JER/jer_ingress_traffic_mgmt.h>
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_metering.h>
#include <soc/dpp/JER/JER_PP/jer_pp_metering.h>
#include <soc/dpp/ARAD/arad_sw_db.h>
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_sw_db.h>
#include <soc/dpp/drv.h>

/* } */
/*************
 * DEFINES   *
 *************/
/* { */
/* These are the possible colors for the MRPS.
   The in color and out color are chosen from/according
   to these. The committed bucket is considered the 'green'
   bucket, while the excess bucket is considered the
   'yellow' bucket. There is no 'red' bucket.*/
typedef enum {
  JER_PP_MTR_PCD_COL_GREEN     = 0,
  JER_PP_MTR_PCD_COL_INVALID   = 1,
  JER_PP_MTR_PCD_COL_YELLOW    = 2,
  JER_PP_MTR_PCD_COL_RED       = 3,
  JER_PP_MTR_PCD_NOF_COLS      = 4
} jer_pp_mtr_pcd_col_t;

#define JER_PP_MTR_PCD_MCDA_COL_LSB                1
#define JER_PP_MTR_PCD_MCDB_COL_LSB                (SOC_IS_QUX(unit) ? 5 : 4)
#define JER_PP_MTR_PCD_COL_NOF_BITS                2

#define	JER_PP_MTR_VAL_EXP_MNT_EQ_CONST_MULTI_BS			1
#define	JER_PP_MTR_PROFILE_VAL_EXP_MNT_EQ_CONST_MNT_INC_BS	64
#define	JER_PP_MTR_PROFILE_VAL_EXP_MNT_EQ_CONST_MNT_DIV_IR	125
#define	JER_PP_MTR_PROFILE_VAL_EXP_MNT_EQ_CONST_MNT_INC_IR	0
/* The number of PCDs is dependent on the input bits (there are 8 for qux, 6 for arad/jer/qax)  */
#define JER_PP_ETH_POLICER_PCD_OPT_NUM    (SOC_IS_QUX(unit) ? 256 : 64)

/*
	Convert packet size from integer to 8bit 2s compliment
*/
#define JER_PP_MTR_PACKET_SIZE_TO_2S_COMPLIMENT(__integer)  (((__integer) < 0) ? ((((-1 * (__integer)) & 0xFF) ^ 0xFF) + 1) : ((__integer) & 0xFF))
/*
	Convert packet size from 8bit 2s compliment to integer
*/
#define JER_PP_MTR_2S_COMPLIMENT_TO_PACKET_SIZE(__twos)     (((__twos) & 0x80) ? (-1 * (((__twos) ^ 0xFF) + 1)) : (__twos))

#define JER_PP_MTR_2S_COMPLIMENT_MAX_VALUE      128
#define JER_PP_MTR_2S_COMPLIMENT_MIN_VALUE      -127

#define JER_PP_MTR_IN_PP_PORT_MAP_NOF_PROFILES  8
#define JER_PP_MTR_IN_PP_PORT_NOF				255

/* 
    Maps 3 bit pp_port_prf and 3 bit MapTo3 prf to row and cell of the InPpPortMapAndMtrPrfMap.
    Each cell holds a packet size delta which will be added to the packet size.
*/
#define JER_PP_MTR_IN_PP_PORT_AND_MTR_PRF_TO_TABLE_ROW(__pp_port_prf, __map_to_3_prf) 	(((__pp_port_prf & 0x07) << 1) + ((__map_to_3_prf & 0x04) >> 2))
#define JER_PP_MTR_MAP_TO_3_PRF_TO_TABLE_CELL(__map_to_3_prf) 	                        (__map_to_3_prf & 0x03)

/* } */
/*******************
 * STATIC PROTOTYPS *
 *******************/
/* { */

STATIC uint32 jer_pp_metering_init_header_compensation(int unit);
STATIC uint32 jer_pp_metering_packet_size_profile_key_get(int unit,JER_PACKET_SIZE_PROFILES_TBL_DATA  profile_tbl_data, uint32 *packet_size_profile_multiset_key);
STATIC uint32 jer_pp_metering_in_pp_port_map_and_mtr_prf_map_get(int unit, int core_id, int meter_group, uint32 map_to_3_prf, uint32 pp_port_prf, int *packet_size);
STATIC uint32 jer_pp_metering_in_pp_port_map_and_mtr_prf_map_set(int unit, int core_id, int meter_group, uint32 map_to_3_prf, uint32 pp_port_prf, int packet_size);
STATIC uint32 jer_pp_metering_in_pp_port_map_get(int unit, int core_id, int meter_group, uint32 pp_port, uint32 *pp_port_prf);
STATIC uint32 jer_pp_metering_in_pp_port_map_set(int unit, int core_id, int meter_group, uint32 pp_port, uint32 pp_port_prf);
STATIC uint32 jer_pp_mtr_policer_hdr_compensation_verify(int unit, int compensation_size);

/* } */
/*************
 * FUNCTIONS *
 *************/
/* { */

uint32
  jer_pp_metering_init_mrps_config(
		SOC_SAND_IN int unit
  )
{
uint32
	range_mode,
	reg_val,
	reg_val_a,
	reg_val_b;

uint32 
	glbl_cfg_reg,
	mcda_cfg_reg,
	mcdb_cfg_reg,
	mcda_refresh_reg,
	mcdb_refresh_reg,
	mcda_wrap_index_reg,
	mcdb_wrap_index_reg;

uint8
    sharing_mode;
int
	core_index;
 
	SOCDNX_INIT_FUNC_DEFS;

	range_mode = SOC_DPP_CONFIG(unit)->meter.meter_range_mode;
	sharing_mode = SOC_DPP_CONFIG(unit)->meter.sharing_mode;

	glbl_cfg_reg 		= JER_MRPS_REG_FORMAT_BY_CHIP(unit,GLBL_CFG);
	mcda_cfg_reg 		= JER_MRPS_REG_FORMAT_BY_CHIP(unit,MCDA_CFG);
	mcdb_cfg_reg 		= JER_MRPS_REG_FORMAT_BY_CHIP(unit,MCDB_CFG);
	mcda_refresh_reg 	= JER_MRPS_REG_FORMAT_BY_CHIP(unit,MCDA_REFRESH_CFG);
	mcdb_refresh_reg 	= JER_MRPS_REG_FORMAT_BY_CHIP(unit,MCDB_REFRESH_CFG);
	mcda_wrap_index_reg = JER_MRPS_REG_FORMAT_BY_CHIP(unit,MCDA_WRAP_INDEX);
	mcdb_wrap_index_reg = JER_MRPS_REG_FORMAT_BY_CHIP(unit,MCDB_WRAP_INDEX);

	/* Global configuration*/
	SOCDNX_IF_ERR_EXIT(soc_reg32_get(unit, glbl_cfg_reg, 0, 0, &reg_val));
	
	/* Init MCDs */
	soc_reg_field_set(unit, glbl_cfg_reg, &reg_val, MCDA_INITf, 0);
	soc_reg_field_set(unit, glbl_cfg_reg, &reg_val, MCDB_INITf, 0);

	/* Hierarchical Mode disabled by default */
	soc_reg_field_set(unit, glbl_cfg_reg, &reg_val, HIERARCHICAL_MODEf, 1);

	/* Set PacketModeEn to allow for meters with packet/sec rates */
    soc_reg_field_set(unit, glbl_cfg_reg, &reg_val, PACKET_MODE_ENf, 1);

	/* Check Range Mode and Sharing mode fit*/
	if (SOC_DPP_CORE_MODE_IS_SINGLE_CORE(unit) && !ARAD_PP_MTR_IS_SINGLE_CORE(unit) ){
		/*Single core device, Two MRPS cores*/
		if (64 == range_mode && SOC_PPC_MTR_SHARING_MODE_PARALLEL != sharing_mode) {
			SOCDNX_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_SOCDNX_MSG("Policer Sharing Mode have to be PARALLEL with Ingress Count 64.")));
		}
		if (128 == range_mode && SOC_PPC_MTR_SHARING_MODE_NONE != sharing_mode) {
			SOCDNX_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_SOCDNX_MSG("Policer Sharing Mode have to be NONE with Ingress Count 128.")));
		}
	}
	else { /* dual device cores (with two mrps cores) or single core device with single mrps core */
		if (32 == range_mode && SOC_PPC_MTR_SHARING_MODE_NONE == sharing_mode) {
			SOCDNX_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_SOCDNX_MSG("Policer Sharing Mode can't be NONE with Ingress Count 32.")));
		}
		if (64 == range_mode && SOC_PPC_MTR_SHARING_MODE_NONE != sharing_mode) {
			SOCDNX_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_SOCDNX_MSG("Policer Sharing Mode have to be NONE with Ingress Count 64.")));
		}
	}

	/* Enable the PCD map for Parallel and Serial modes */
	soc_reg_field_set(unit, glbl_cfg_reg, &reg_val, PCD_MAP_ENf, (SOC_PPC_MTR_SHARING_MODE_NONE == sharing_mode) ? 0 : 1);

	/*
		Device core mode | Meters | McqSizeSel | MrpsSecondPtrEn | McdSecondPtrEn | McqMcdsParallel |
	    -----------------|--------|------------|-----------------|----------------|-----------------|																					   |
		Single           | 2x64   |     0      |       1         |      1         |       1         |
		-----------------|--------|------------|-----------------|----------------|-----------------|
		Single           | 1x128  |     1      |       0         |      1         |       1         |
		-----------------|--------|------------|-----------------|----------------|-----------------|
		Dual             | 2x2x32 |     0      |       1         |      1         |   SharingMode   |
		-----------------|--------|------------|-----------------|----------------|-----------------|
		Dual             | 2x64   |     1      |       1         |      0         |       1         |
	*/
	if (SOC_DPP_CORE_MODE_IS_SINGLE_CORE(unit) && !ARAD_PP_MTR_IS_SINGLE_CORE(unit)) {
		/* single device core with two mrps cores*/
		soc_reg_field_set(unit, glbl_cfg_reg, &reg_val, MCQ_SIZE_SELf, (128 == range_mode)? 1 : 0);
		soc_reg_field_set(unit, glbl_cfg_reg, &reg_val, MRPS_SECOND_PTR_ENf, (64 == range_mode)? 1 : 0);
		soc_reg_field_set(unit, glbl_cfg_reg, &reg_val, MCD_SECOND_PTR_ENf, 1);
		soc_reg_field_set(unit, glbl_cfg_reg, &reg_val, MCQ_MCDS_PARALLELf, 1);

	} else {
		/* dual device cores (with two mrps cores) or single core device with single mrps core */
		soc_reg_field_set(unit, glbl_cfg_reg, &reg_val, MCQ_SIZE_SELf, (64 == range_mode)? 1 : 0);
		soc_reg_field_set(unit, glbl_cfg_reg, &reg_val, MRPS_SECOND_PTR_ENf, 1);
		soc_reg_field_set(unit, glbl_cfg_reg, &reg_val, MCD_SECOND_PTR_ENf, (32 == range_mode)? 1 : 0);
		soc_reg_field_set(unit, glbl_cfg_reg, &reg_val, MCQ_MCDS_PARALLELf, 
						  (64 == range_mode || SOC_PPC_MTR_SHARING_MODE_PARALLEL == sharing_mode) ? 1 : 0);
	}

	/* MEF 10.3 disabled by default */
	soc_reg_field_set(unit, glbl_cfg_reg, &reg_val, MEF_10_DOT_3_ENf, 0);

	if (SOC_IS_QAX(unit)) {
		/* in QAX - DUAL_FAP_MODE shoud be set to 1*/
		soc_reg_field_set(unit, glbl_cfg_reg, &reg_val, DUAL_FAP_MODEf, 1);
	}

	ARAD_PP_MTR_CORES_ITER(core_index){
		SOCDNX_IF_ERR_EXIT(soc_reg32_set(unit, glbl_cfg_reg, core_index, 0, reg_val));
	}

	reg_val_a = 0;
	reg_val_b = 0;

	/* Enable leaky buckets Update */
	soc_reg_field_set(unit, mcda_cfg_reg, &reg_val_a, MCDA_TIMER_ENf, 1);
	soc_reg_field_set(unit, mcdb_cfg_reg, &reg_val_b, MCDB_TIMER_ENf, 1);

	/* Enable leaky buckets Refresh */
	soc_reg_field_set(unit, mcda_cfg_reg, &reg_val_a, MCDA_REFRESH_ENf, 1);
	soc_reg_field_set(unit, mcdb_cfg_reg, &reg_val_b, MCDB_REFRESH_ENf, 1);

	/* Enable LFR */
	soc_reg_field_set(unit, mcda_cfg_reg, &reg_val_a, MCDA_CBL_RND_MODE_ENf, 1);
	soc_reg_field_set(unit, mcdb_cfg_reg, &reg_val_b, MCDB_CBL_RND_MODE_ENf, 1);

	/* Set default values to the rest */
	soc_reg_field_set(unit, mcda_cfg_reg, &reg_val_a, MCDA_ERR_COMP_ENABLEf, 1);
	soc_reg_field_set(unit, mcdb_cfg_reg, &reg_val_b, MCDB_ERR_COMP_ENABLEf, 1);

	soc_reg_field_set(unit, mcda_cfg_reg, &reg_val_a, MCDA_RND_RANGEf, 3);
	soc_reg_field_set(unit, mcdb_cfg_reg, &reg_val_b, MCDB_RND_RANGEf, 3);

	soc_reg_field_set(unit, mcda_cfg_reg, &reg_val_a, MCDA_BUBBLE_RATEf, 0xff);
	soc_reg_field_set(unit, mcdb_cfg_reg, &reg_val_b, MCDB_BUBBLE_RATEf, 0xff);

	ARAD_PP_MTR_CORES_ITER(core_index){
		SOCDNX_IF_ERR_EXIT(soc_reg32_set(unit, mcda_cfg_reg, core_index, 0, reg_val_a));
		SOCDNX_IF_ERR_EXIT(soc_reg32_set(unit, mcdb_cfg_reg, core_index, 0, reg_val_b));
	}

	reg_val_a = 0;
	reg_val_b = 0;

	/* Set default values for refresh quartet index range */
	soc_reg_field_set(unit, mcda_refresh_reg, &reg_val_a, MCDA_REFRESH_START_INDEXf, 0);
	soc_reg_field_set(unit, mcdb_refresh_reg, &reg_val_b, MCDB_REFRESH_START_INDEXf, 0);

	soc_reg_field_set(unit, mcda_refresh_reg, &reg_val_a, MCDA_REFRESH_END_INDEXf, 0x1fff);
	soc_reg_field_set(unit, mcdb_refresh_reg, &reg_val_b, MCDB_REFRESH_END_INDEXf, 0x1fff);

	ARAD_PP_MTR_CORES_ITER(core_index){
		SOCDNX_IF_ERR_EXIT(soc_reg32_set(unit, mcda_refresh_reg, core_index, 0, reg_val_a));
		SOCDNX_IF_ERR_EXIT(soc_reg32_set(unit, mcdb_refresh_reg, core_index, 0, reg_val_b));
	}

	reg_val_a = 0;
	reg_val_b = 0;

	/* Enable wrap prevention */
	soc_reg_field_set(unit, mcda_wrap_index_reg, &reg_val_a, MCDA_WRAP_INT_ENf, 1);
	soc_reg_field_set(unit, mcdb_wrap_index_reg, &reg_val_b, MCDB_WRAP_INT_ENf, 1);

	ARAD_PP_MTR_CORES_ITER(core_index){
		SOCDNX_IF_ERR_EXIT(soc_reg32_set(unit, mcda_wrap_index_reg, core_index, 0, reg_val_a));
		SOCDNX_IF_ERR_EXIT(soc_reg32_set(unit, mcdb_wrap_index_reg, core_index, 0, reg_val_b));
	}
	
	/* Init PCD for Parallel and Serial modes */
	if (sharing_mode != SOC_PPC_MTR_SHARING_MODE_NONE) {
		SOCDNX_SAND_IF_ERR_EXIT(arad_pp_metering_pcd_init(unit, sharing_mode));
	}

    /* in QAX, if MCDA_REFRESH is enable, the MPRS logic read some IMP memories every clock, and it can't be access by the CPU. */
    /* Here we define a bubble for the IMP in order to give access to the CPU */
    /* the bubble will be created only when CPU ask access. */
	if(SOC_IS_QAX(unit))
    {
        reg_val_a = 0;
        SOCDNX_IF_ERR_EXIT(READ_IMP_INDIRECT_COMMANDr(unit, &reg_val_a));
        soc_reg_field_set(unit, IMP_INDIRECT_COMMANDr, &reg_val_a, INDIRECT_COMMAND_TIMEOUTf, 0x10);
        SOCDNX_IF_ERR_EXIT(WRITE_IMP_INDIRECT_COMMANDr(unit, reg_val_a));
        reg_val_a = 0;
        SOCDNX_IF_ERR_EXIT(READ_IMP_INDIRECT_FORCE_BUBBLEr(unit, &reg_val_a));        
        soc_reg_field_set(unit, IMP_INDIRECT_FORCE_BUBBLEr, &reg_val_a, FORCE_BUBBLE_ENf, 1);
        soc_reg_field_set(unit, IMP_INDIRECT_FORCE_BUBBLEr, &reg_val_a, FORCE_BUBBLE_PERIODf, 0xa);
        soc_reg_field_set(unit, IMP_INDIRECT_FORCE_BUBBLEr, &reg_val_a, FORCE_BUBBLE_PULSE_WIDTHf, 1);    
        SOCDNX_IF_ERR_EXIT(WRITE_IMP_INDIRECT_FORCE_BUBBLEr(unit, reg_val_a));                
    }

exit:
  SOCDNX_FUNC_RETURN;
}

uint32
  jer_pp_metering_init_mrpsEm_config(
		SOC_SAND_IN int unit
  )
{
uint32
	reg_val,
	reg_val_a,
	reg_val_b;

uint32 
	glbl_cfg_reg,
	mcda_cfg_reg,
	mcdb_cfg_reg,
	mcda_refresh_reg,
	mcdb_refresh_reg,
	mcda_wrap_index_reg,
	mcdb_wrap_index_reg;

int
	core_index;
 
	SOCDNX_INIT_FUNC_DEFS;

	glbl_cfg_reg 		= JER_MRPS_EM_REG_FORMAT_BY_CHIP(unit,GLBL_CFG);
	mcda_cfg_reg 		= JER_MRPS_EM_REG_FORMAT_BY_CHIP(unit,MCDA_CFG);
	mcdb_cfg_reg 		= JER_MRPS_EM_REG_FORMAT_BY_CHIP(unit,MCDB_CFG);
	mcda_refresh_reg 	= JER_MRPS_EM_REG_FORMAT_BY_CHIP(unit,MCDA_REFRESH_CFG);
	mcdb_refresh_reg 	= JER_MRPS_EM_REG_FORMAT_BY_CHIP(unit,MCDB_REFRESH_CFG);
	mcda_wrap_index_reg = JER_MRPS_EM_REG_FORMAT_BY_CHIP(unit,MCDA_WRAP_INDEX);
	mcdb_wrap_index_reg = JER_MRPS_EM_REG_FORMAT_BY_CHIP(unit,MCDB_WRAP_INDEX);

	/* Global configuration*/
	SOCDNX_IF_ERR_EXIT(soc_reg32_get(unit, glbl_cfg_reg, 0, 0, &reg_val));

	/* Init MCDs */
	soc_reg_field_set(unit, glbl_cfg_reg, &reg_val, MCDA_INITf, 0);
	soc_reg_field_set(unit, glbl_cfg_reg, &reg_val, MCDB_INITf, 0);

	/* Hierarchical Mode disabled by default */
	soc_reg_field_set(unit, glbl_cfg_reg, &reg_val, HIERARCHICAL_MODEf, 1);

	/* Set PacketModeEn to allow for meters with packet/sec rates */
    soc_reg_field_set(unit, glbl_cfg_reg, &reg_val, PACKET_MODE_ENf, 1);

	/* Enable the PCD map for Parallel and Serial modes */
	soc_reg_field_set(unit, glbl_cfg_reg, &reg_val, PCD_MAP_ENf, 1);

	soc_reg_field_set(unit, glbl_cfg_reg, &reg_val, MCQ_SIZE_SELf, 0);
	soc_reg_field_set(unit, glbl_cfg_reg, &reg_val, MRPS_SECOND_PTR_ENf, 1);
	soc_reg_field_set(unit, glbl_cfg_reg, &reg_val, MCD_SECOND_PTR_ENf, 1);
	soc_reg_field_set(unit, glbl_cfg_reg, &reg_val, MCQ_MCDS_PARALLELf, 1);	/* has to be 1 - Eth meter works only in parallel*/

	if (SOC_IS_QAX(unit)) {
		/* in QAX - DUAL_FAP_MODE shoud be set to 1*/
		soc_reg_field_set(unit, glbl_cfg_reg, &reg_val, DUAL_FAP_MODEf, 1);
	}

	ARAD_PP_MTR_CORES_ITER(core_index){
		SOCDNX_IF_ERR_EXIT(soc_reg32_set(unit, glbl_cfg_reg, core_index, 0, reg_val));
	}

	reg_val_a = 0;
	reg_val_b = 0;

	/* Enable leaky buckets Update */
	soc_reg_field_set(unit, mcda_cfg_reg, &reg_val_a, MCDA_TIMER_ENf, 1);
	soc_reg_field_set(unit, mcdb_cfg_reg, &reg_val_b, MCDB_TIMER_ENf, 1);

	/* Enable leaky buckets Refresh */
	soc_reg_field_set(unit, mcda_cfg_reg, &reg_val_a, MCDA_REFRESH_ENf, 1);
	soc_reg_field_set(unit, mcdb_cfg_reg, &reg_val_b, MCDB_REFRESH_ENf, 1);

	/* Enable LFR */
	soc_reg_field_set(unit, mcda_cfg_reg, &reg_val_a, MCDA_CBL_RND_MODE_ENf, 1);
	soc_reg_field_set(unit, mcdb_cfg_reg, &reg_val_b, MCDB_CBL_RND_MODE_ENf, 1);

	/* Enable LFR */
	soc_reg_field_set(unit, mcda_cfg_reg, &reg_val_a, MCDA_EBL_RND_MODE_ENf, 1);
	soc_reg_field_set(unit, mcdb_cfg_reg, &reg_val_b, MCDB_EBL_RND_MODE_ENf, 1);

	/* Set default values to the rest */
	soc_reg_field_set(unit, mcda_cfg_reg, &reg_val_a, MCDA_ERR_COMP_ENABLEf, 1);
	soc_reg_field_set(unit, mcdb_cfg_reg, &reg_val_b, MCDB_ERR_COMP_ENABLEf, 1);

	soc_reg_field_set(unit, mcda_cfg_reg, &reg_val_a, MCDA_RND_RANGEf, 3);
	soc_reg_field_set(unit, mcdb_cfg_reg, &reg_val_b, MCDB_RND_RANGEf, 3);

	soc_reg_field_set(unit, mcda_cfg_reg, &reg_val_a, MCDA_BUBBLE_RATEf, 0xff);
	soc_reg_field_set(unit, mcdb_cfg_reg, &reg_val_b, MCDB_BUBBLE_RATEf, 0xff);

	ARAD_PP_MTR_CORES_ITER(core_index){
		SOCDNX_IF_ERR_EXIT(soc_reg32_set(unit, mcda_cfg_reg, core_index, 0, reg_val_a));
		SOCDNX_IF_ERR_EXIT(soc_reg32_set(unit, mcdb_cfg_reg, core_index, 0, reg_val_b));
	}

	reg_val_a = 0;
	reg_val_b = 0;

	/* Set default values for refresh quartet index range */
	soc_reg_field_set(unit, mcda_refresh_reg, &reg_val_a, MCDA_REFRESH_START_INDEXf, 0);
	soc_reg_field_set(unit, mcdb_refresh_reg, &reg_val_b, MCDB_REFRESH_START_INDEXf, 0);

	soc_reg_field_set(unit, mcda_refresh_reg, &reg_val_a, MCDA_REFRESH_END_INDEXf, 0x13f);
	soc_reg_field_set(unit, mcdb_refresh_reg, &reg_val_b, MCDB_REFRESH_END_INDEXf, 0x1);

	ARAD_PP_MTR_CORES_ITER(core_index){
		SOCDNX_IF_ERR_EXIT(soc_reg32_set(unit, mcda_refresh_reg, core_index, 0, reg_val_a));
		SOCDNX_IF_ERR_EXIT(soc_reg32_set(unit, mcdb_refresh_reg, core_index, 0, reg_val_b));
	}

	reg_val_a = 0;
	reg_val_b = 0;

	/* Enable wrap prevention */
	soc_reg_field_set(unit, mcda_wrap_index_reg, &reg_val_a, MCDA_WRAP_INT_ENf, 1);
	soc_reg_field_set(unit, mcdb_wrap_index_reg, &reg_val_b, MCDB_WRAP_INT_ENf, 1);

	ARAD_PP_MTR_CORES_ITER(core_index){
		SOCDNX_IF_ERR_EXIT(soc_reg32_set(unit, mcda_wrap_index_reg, core_index, 0, reg_val_a));
		SOCDNX_IF_ERR_EXIT(soc_reg32_set(unit, mcdb_wrap_index_reg, core_index, 0, reg_val_b));
	}

    /* in QAX, if MCDA_REFRESH is enable, the MPRS logic read some IMP memories every clock, and it can't be access by the CPU. */
    /* Here we define a bubble for the IMP in order to give access to the CPU */
    /* the bubble will be created only when CPU ask access. */
	if(SOC_IS_QAX(unit))
    {
        reg_val_a = 0;
        SOCDNX_IF_ERR_EXIT(READ_IEP_INDIRECT_COMMANDr(unit, &reg_val_a));
        soc_reg_field_set(unit, IEP_INDIRECT_COMMANDr, &reg_val_a, INDIRECT_COMMAND_TIMEOUTf, 0x10);
        SOCDNX_IF_ERR_EXIT(WRITE_IEP_INDIRECT_COMMANDr(unit, reg_val_a));
        reg_val_a = 0;
        SOCDNX_IF_ERR_EXIT(READ_IEP_INDIRECT_FORCE_BUBBLEr(unit, &reg_val_a));        
        soc_reg_field_set(unit, IEP_INDIRECT_FORCE_BUBBLEr, &reg_val_a, FORCE_BUBBLE_ENf, 1);
        soc_reg_field_set(unit, IEP_INDIRECT_FORCE_BUBBLEr, &reg_val_a, FORCE_BUBBLE_PERIODf, 0xa);
        soc_reg_field_set(unit, IEP_INDIRECT_FORCE_BUBBLEr, &reg_val_a, FORCE_BUBBLE_PULSE_WIDTHf, 1);    
        SOCDNX_IF_ERR_EXIT(WRITE_IEP_INDIRECT_FORCE_BUBBLEr(unit, reg_val_a));                
    }	
exit:
  SOCDNX_FUNC_RETURN;
}

uint32 
  jer_pp_eth_policer_pcd_init(
     SOC_SAND_IN int unit)
{
	uint32
		/* Per meter color*/
		mcda_mtr_color,
		mcdb_mtr_color,	
		/* Final color decision for this PCD address*/
		final_color, 
		/* Update colors*/
		mcda_update_color,
		mcdb_update_color,
		addr_idx,
		pcd_line;
	uint32
		res;
	soc_mem_t
		mem = JER_MRPS_EM_MEM_FORMAT_BY_CHIP(unit,PCD_MAP);
	int
		core_index;

	SOCDNX_INIT_FUNC_DEFS;

	/* Fill PCD table*/
	for (addr_idx = 0; addr_idx < JER_PP_ETH_POLICER_PCD_OPT_NUM; addr_idx++) {

		mcda_mtr_color = mcdb_mtr_color = 0;
		mcda_update_color = mcdb_update_color = final_color = 0;

		/* Get the meter color from address. */
        SHR_BITCOPY_RANGE(&(mcda_mtr_color), 0, &addr_idx, 
                          JER_PP_MTR_PCD_MCDA_COL_LSB, JER_PP_MTR_PCD_COL_NOF_BITS);
		SHR_BITCOPY_RANGE(&(mcdb_mtr_color), 0, &addr_idx, 
                          JER_PP_MTR_PCD_MCDB_COL_LSB, JER_PP_MTR_PCD_COL_NOF_BITS);

		/* Meter Validity-
			1. Arad+
			The table is used only when both pointers are valid.
			2. Jericho
			The table is used even if one of the pointers is invalid. In this case, its color will be 'invalid',
			and the output color should be the other meter color.
		 
			Color Resolution-
			1. final color will be green only if all the valid policers color are green.
		    update color of the valid policers will be green in this case.
		 
		    2. any other case will cause red/invalid final and update colors for the valid/invalid policers correspondingly
		*/

		if ((mcda_mtr_color == JER_PP_MTR_PCD_COL_INVALID) && (mcdb_mtr_color == JER_PP_MTR_PCD_COL_INVALID)) {
			final_color = mcda_update_color = mcdb_update_color = JER_PP_MTR_PCD_COL_RED;
		}
		else if (mcda_mtr_color == JER_PP_MTR_PCD_COL_INVALID) {
			if (mcdb_mtr_color == JER_PP_MTR_PCD_COL_GREEN) {
				final_color = mcdb_update_color = JER_PP_MTR_PCD_COL_GREEN;
			}else{
				final_color = mcdb_update_color = JER_PP_MTR_PCD_COL_RED;
			}
			mcda_update_color = JER_PP_MTR_PCD_COL_RED;
		}
		else if (mcdb_mtr_color == JER_PP_MTR_PCD_COL_INVALID) {
			if (mcda_mtr_color == JER_PP_MTR_PCD_COL_GREEN) {
				final_color = mcda_update_color = JER_PP_MTR_PCD_COL_GREEN;
			}else{
				final_color = mcda_update_color = JER_PP_MTR_PCD_COL_RED;
			}
			mcdb_update_color = JER_PP_MTR_PCD_COL_RED;
		}
		else if ((mcda_mtr_color == JER_PP_MTR_PCD_COL_GREEN) && (mcdb_mtr_color == JER_PP_MTR_PCD_COL_GREEN)) {
			final_color = mcda_update_color = mcdb_update_color = JER_PP_MTR_PCD_COL_GREEN;
		}
		else {
			final_color = mcda_update_color = mcdb_update_color = JER_PP_MTR_PCD_COL_RED;
		}

		/* Set bucket update and out color to final color*/
		pcd_line = 0;
		soc_mem_field32_set(unit, mem, &pcd_line, MCDA_UPDATE_COLORf, mcda_update_color);
		soc_mem_field32_set(unit, mem, &pcd_line, MCDB_UPDATE_COLORf, mcdb_update_color);
		soc_mem_field32_set(unit, mem, &pcd_line, OUT_COLORf, final_color);

		/* Write entry to table*/
		ARAD_PP_MTR_CORES_ITER(core_index){
			res = soc_mem_write(unit, mem, ARAD_PP_ETH_MTR_MEM_BLOCK(unit, core_index), addr_idx, &pcd_line); 
			SOCDNX_IF_ERR_EXIT(res);
		}
	}

exit:
	SOCDNX_FUNC_RETURN;
}

STATIC uint32
  jer_pp_metering_init_eth_policer_default_config(
							SOC_SAND_IN int unit,
							SOC_SAND_IN int core_id
  )
{
	uint32
	  res;
	unsigned int
	  soc_sand_dev_id;

	soc_reg_above_64_val_t
	  eth_mtr_prfCfg_above_64_val;

	soc_mem_t
	  eth_mtr_prfCfg_fields_mem,
	  eth_mtr_prfCfg_0_mem,
	  eth_mtr_prfCfg_1_mem;

	  SOCDNX_INIT_FUNC_DEFS;
	  
	  soc_sand_dev_id = (unit);
	  SOC_REG_ABOVE_64_CLEAR(eth_mtr_prfCfg_above_64_val);

	  eth_mtr_prfCfg_fields_mem = JER_MRPS_EM_MEM_FORMAT_BY_CHIP(unit,MCDA_PRFCFG_SHARING_DIS); /* using prf cfg 1 because of sharing flag is disabled on eth. meter*/
	  eth_mtr_prfCfg_0_mem 		= JER_MRPS_EM_MEM_FORMAT_BY_CHIP(unit,MCDA_PRFCFG_0);
	  eth_mtr_prfCfg_1_mem 		= JER_MRPS_EM_MEM_FORMAT_BY_CHIP(unit,MCDA_PRFCFG_1);

	  soc_mem_field32_set(unit, eth_mtr_prfCfg_fields_mem, (void*)eth_mtr_prfCfg_above_64_val, RESET_CIRf     , 0);
	  soc_mem_field32_set(unit, eth_mtr_prfCfg_fields_mem, (void*)eth_mtr_prfCfg_above_64_val, CIR_MANTf      , 63);
	  soc_mem_field32_set(unit, eth_mtr_prfCfg_fields_mem, (void*)eth_mtr_prfCfg_above_64_val, CIR_MANT_EXPf  , 7);
	  soc_mem_field32_set(unit, eth_mtr_prfCfg_fields_mem, (void*)eth_mtr_prfCfg_above_64_val, CBS_MANT_64f   , 63);
	  soc_mem_field32_set(unit, eth_mtr_prfCfg_fields_mem, (void*)eth_mtr_prfCfg_above_64_val, CBS_EXPONENTf  , 15);
	  soc_mem_field32_set(unit, eth_mtr_prfCfg_fields_mem, (void*)eth_mtr_prfCfg_above_64_val, PACKET_MODEf	  , 0);
	  soc_mem_field32_set(unit, eth_mtr_prfCfg_fields_mem, (void*)eth_mtr_prfCfg_above_64_val, CIR_REV_EXP_2f , 0);
	  soc_mem_field32_set(unit, eth_mtr_prfCfg_fields_mem, (void*)eth_mtr_prfCfg_above_64_val, COLOR_AWAREf   , 0);
	  soc_mem_field32_set(unit, eth_mtr_prfCfg_fields_mem, (void*)eth_mtr_prfCfg_above_64_val, MTR_PROFILE_MAP_TO_4f , 9);

	  res = soc_mem_write(soc_sand_dev_id, eth_mtr_prfCfg_0_mem, ARAD_PP_ETH_MTR_MEM_BLOCK(unit, core_id), 0, (void*)eth_mtr_prfCfg_above_64_val); 
	  SOCDNX_IF_ERR_EXIT(res);
	  res = soc_mem_write(soc_sand_dev_id, eth_mtr_prfCfg_1_mem, ARAD_PP_ETH_MTR_MEM_BLOCK(unit, core_id), 0, (void*)eth_mtr_prfCfg_above_64_val); 
	  SOCDNX_IF_ERR_EXIT(res);

  exit:
	  SOCDNX_FUNC_RETURN;


}
uint32
  jer_pp_metering_init(
		SOC_SAND_IN int unit
  )
{
  uint32
	res,
	eth_mtr_profile_multiset_key = 0x0,
    eth_mtr_profile_multiset_ndx = 0x0;
  uint8
    eth_mtr_profile_multiset_first_appear = 0x0;
  int core_index;

  SOC_SAND_SUCCESS_FAILURE
    eth_mtr_profile_multiset_success = SOC_SAND_SUCCESS;
  ARAD_IDR_ETHERNET_METER_PROFILES_TBL_DATA
    profile_tbl_data = {0};

	SOCDNX_INIT_FUNC_DEFS;

	/*Init MRPS-In-DP and DP maps*/
	SOCDNX_IF_ERR_EXIT(jer_itm_setup_dp_map(unit));

	if (!SOC_IS_QAX(unit)) { /* In QAX the DG overwirte is done by default */
		/* Set the FTMH DP source to always be the output of the DP map. */
		/* When both meters are invalid(!!! in Arad+ it was required both should be valid), the meter DP will be the In-DP. */
		SOCDNX_IF_ERR_EXIT(soc_reg_above_64_field32_modify(unit, IDR_STATIC_CONFIGURATIONr, IDR_BLOCK(unit), 0, FTMH_DP_OVERWRITEf, 1));
	}
	SOCDNX_IF_ERR_EXIT(jer_pp_metering_init_mrps_config(unit));

	/*Eth. Policers SOC init*/
	SOCDNX_IF_ERR_EXIT(jer_pp_metering_init_mrpsEm_config(unit));
	SOCDNX_IF_ERR_EXIT(jer_pp_eth_policer_pcd_init(unit));

	/* set multiset 0 - so Only 31 policers are available */
	arad_pp_mtr_eth_policer_profile_key_get(
		unit,
		profile_tbl_data,
		&eth_mtr_profile_multiset_key
		);

	ARAD_PP_MTR_CORES_ITER(core_index){
		res = arad_sw_db_multiset_add(
			unit,
			core_index,
			ARAD_PP_SW_DB_MULTI_SET_ETH_POLICER_ETH_METER_PROFILE,
			&eth_mtr_profile_multiset_key,
			&eth_mtr_profile_multiset_ndx,
			&eth_mtr_profile_multiset_first_appear,
			&eth_mtr_profile_multiset_success
			);
		SOCDNX_SAND_IF_ERR_EXIT(res);

		SOCDNX_IF_ERR_EXIT(jer_pp_metering_init_eth_policer_default_config(unit,core_index));
	}

    SOCDNX_IF_ERR_EXIT(jer_pp_metering_init_header_compensation(unit)); 


exit:
  SOCDNX_FUNC_RETURN;
}

soc_error_t
  soc_jer_pp_mtr_policer_global_sharing_get(
    int                         unit,
	int                         core_id,
	int        					meter_id,
	int							meter_id_group,
	uint32* 					global_sharing_ptr
)
{
	unsigned int 			soc_sand_dev_id;
    uint32					soc_sand_rv;
	uint32 					mem_val[2];
	int mem,index;

	SOCDNX_INIT_FUNC_DEFS;
	SOCDNX_NULL_CHECK(global_sharing_ptr);
	soc_sand_dev_id = (unit);

	/* get whether the 4 meters are in hierarchical mode */
	mem = (meter_id_group == 0) ? JER_PP_MTR_MEMORY(unit, MCDA_PRFSELm) : JER_PP_MTR_MEMORY(unit, MCDB_PRFSELm);
	index = meter_id / 4;

	/* get the entry */
	soc_sand_rv = soc_mem_read(soc_sand_dev_id, mem, ARAD_PP_MTR_MEM_BLOCK(unit, core_id), index, mem_val);
	SOCDNX_IF_ERR_EXIT(soc_sand_rv);

	/* get the global_sharing */
	soc_mem_field_get(soc_sand_dev_id, mem, mem_val, GLOBAL_SHARINGf, global_sharing_ptr);
exit:
	SOCDNX_FUNC_RETURN;
}

soc_error_t
  soc_jer_pp_mtr_policer_global_sharing_set(
    int                         unit,
	int                         core_id,
	int        					meter_id,
	int							meter_group,
	uint32* 					global_sharing_ptr
)
{
	unsigned int 			soc_sand_dev_id;
    uint32					soc_sand_rv;
	uint32 					mem_val[2];
	int mem,index;

	SOCDNX_INIT_FUNC_DEFS;
	SOCDNX_NULL_CHECK(global_sharing_ptr);
	soc_sand_dev_id = (unit);

	/* get whether the 4 meters are in hierarchical mode */
	mem = (meter_group == 0) ? JER_PP_MTR_MEMORY(unit, MCDA_PRFSELm) : JER_PP_MTR_MEMORY(unit, MCDB_PRFSELm);
	
	index = meter_id / 4;

	/* get the entry */
	soc_sand_rv = soc_mem_read(soc_sand_dev_id, mem, ARAD_PP_MTR_MEM_BLOCK(unit, core_id), index, mem_val); 
	SOCDNX_IF_ERR_EXIT(soc_sand_rv);

	/* set the global_sharing */
	soc_mem_field_set(soc_sand_dev_id, mem, mem_val, GLOBAL_SHARINGf, global_sharing_ptr);

	soc_sand_rv = soc_mem_write(soc_sand_dev_id, mem, ARAD_PP_MTR_MEM_BLOCK(unit, core_id), index, mem_val); 
	SOCDNX_IF_ERR_EXIT(soc_sand_rv);

exit:
	SOCDNX_FUNC_RETURN;
}

uint32
  jer_pp_mtr_eth_policer_params_set(
    SOC_SAND_IN  int                         	unit,
	SOC_SAND_IN  int                         	core_id,
    SOC_SAND_IN  SOC_PPC_PORT                   port_ndx,
	SOC_SAND_IN  SOC_PPC_MTR_ETH_TYPE           eth_type_ndx,
    SOC_SAND_IN  SOC_PPC_MTR_BW_PROFILE_INFO    *policer_info
  )
{
  unsigned int
	soc_sand_dev_id;

  uint32
	res,
	profile,
	write_access_enable,
	init_bucket_level = 1,
	config_tbl_entry,
	config_array_index,
    status_tbl_offset,
    prfSel_tbl_offset,
	prfSel_tbl_entry,
	bckLvl_tbl_offset,
	bckLvl_tbl_entry;

  soc_reg_above_64_val_t
	eth_mtr_bck_lvl_above_64_val,
    eth_mtr_prfCfg_above_64_val;
  uint32
	backet_init_val = 0,
	eth_mtr_prfSel_val = 0,
	eth_mtr_config_val = 0;

  soc_mem_t
	eth_mtr_prfSel_mem,
	eth_mtr_config_mem,
	eth_mtr_prfCfg_0_mem,
	eth_mtr_prfCfg_1_mem,
	eth_mtr_prfCfg_fields_mem,
	eth_mtr_bck_lvl_mem,
	eth_mtr_enable_dyn_access;

  uint32
	cir, cbs;
  uint8
	info_enable;

  uint32
	eth_mtr_profile_multiset_key = 0x0,
    eth_mtr_profile_multiset_ndx = 0x0;
  uint8
    eth_mtr_profile_multiset_first_appear = 0x0,
    eth_mtr_profile_multiset_last_appear = 0x0,
    sw_db_enable_bit;

  SOC_SAND_SUCCESS_FAILURE
    eth_mtr_profile_multiset_success = SOC_SAND_SUCCESS;
  ARAD_IDR_ETHERNET_METER_PROFILES_TBL_DATA
    profile_tbl_data;

  uint32
	profile_field[4] = {PROFILE_0f, PROFILE_1f, PROFILE_2f, PROFILE_3f};
  uint32
	cbl_bucket_fld[4]= {CBL_0f	  , CBL_1f    , CBL_2f    , CBL_3f};

	SOCDNX_INIT_FUNC_DEFS;
	SOCDNX_NULL_CHECK(policer_info);
	SOC_REG_ABOVE_64_CLEAR(eth_mtr_prfCfg_above_64_val);
	SOC_REG_ABOVE_64_CLEAR(eth_mtr_bck_lvl_above_64_val);
	info_enable = SOC_SAND_BOOL2NUM_INVERSE(policer_info->disable_cir);

	soc_sand_dev_id = (unit);
	eth_mtr_prfSel_mem 			= JER_MRPS_EM_MEM_FORMAT_BY_CHIP(unit,MCDA_PRFSEL);
	eth_mtr_prfCfg_fields_mem 	= JER_MRPS_EM_MEM_FORMAT_BY_CHIP(unit,MCDA_PRFCFG_SHARING_DIS); /* using prf cfg 1 because of sharing flag is disabled on eth. meter*/
    eth_mtr_prfCfg_0_mem 		= JER_MRPS_EM_MEM_FORMAT_BY_CHIP(unit,MCDA_PRFCFG_0);
	eth_mtr_prfCfg_1_mem 		= JER_MRPS_EM_MEM_FORMAT_BY_CHIP(unit,MCDA_PRFCFG_1);
	eth_mtr_bck_lvl_mem			= JER_MRPS_EM_MEM_FORMAT_BY_CHIP(unit,MCDA_DYNAMIC);
	eth_mtr_enable_dyn_access   = JER_MRPS_EM_REG_FORMAT_BY_CHIP(unit,ENABLE_DYNAMIC_MEMORY_ACCESS);

	if (SOC_IS_QAX(unit)) {
		eth_mtr_config_mem 		= CGM_ETH_MTR_PTR_MAPm;
	}else{
		eth_mtr_config_mem 		= IDR_ETHERNET_METER_CONFIGm;
	}

	config_array_index = core_id;
	status_tbl_offset = config_tbl_entry  = port_ndx * SOC_PPC_NOF_MTR_ETH_TYPES + eth_type_ndx;
	/*
	    below: divide and modulu by 4.
	    the profile selection table format is:
							offset
			-------------------------------------------------------------
			| policer 0   | policer 1    | policer 2    | policer 3     |
		e	--------------------------------------------------------------
		n		.			.					.				.
		t		.			.					.				.
		r		.			.					.				.
		y	-------------------------------------------------------------
			| policer 4k  | policer 4k+1 | policer 4k+2 | policer 4k+3  |
			-------------------------------------------------------------
	*/
	bckLvl_tbl_entry  = prfSel_tbl_entry  = config_tbl_entry >> 2;  /* /4 */
	bckLvl_tbl_offset = prfSel_tbl_offset = config_tbl_entry & 0x3; /* %4 */

	/* Get values from tables */  
	res = soc_mem_read(soc_sand_dev_id, eth_mtr_prfSel_mem, ARAD_PP_ETH_MTR_MEM_BLOCK(unit, core_id), prfSel_tbl_entry, (void*)&eth_mtr_prfSel_val);
	SOCDNX_IF_ERR_EXIT(res);

	if (SOC_IS_QAX(unit)) {
		res = soc_mem_read(soc_sand_dev_id, eth_mtr_config_mem, MEM_BLOCK_ANY, config_tbl_entry, (void*)&eth_mtr_config_val);
		SOCDNX_IF_ERR_EXIT(res);
	}else{
		res = soc_mem_array_read(soc_sand_dev_id, eth_mtr_config_mem, config_array_index, MEM_BLOCK_ANY, config_tbl_entry, (void*)&eth_mtr_config_val);
		SOCDNX_IF_ERR_EXIT(res);
	}

	profile = soc_mem_field32_get(unit, eth_mtr_prfSel_mem, (void*)&eth_mtr_prfSel_val, profile_field[prfSel_tbl_offset]);

	res = arad_sw_db_multiset_get_enable_bit(
			unit,
			core_id,
			status_tbl_offset,
			&sw_db_enable_bit
			);
	SOCDNX_SAND_IF_ERR_EXIT(res);

    if (sw_db_enable_bit == TRUE)
	{
		/*profile is enable remove from multiset*/
		res = arad_sw_db_multiset_remove_by_index(
			unit,
			core_id,
			ARAD_PP_SW_DB_MULTI_SET_ETH_POLICER_ETH_METER_PROFILE,
			profile,
			&eth_mtr_profile_multiset_last_appear
			);
		SOCDNX_SAND_IF_ERR_EXIT(res);
    }
 
	cir = policer_info->cir;
	cbs = policer_info->cbs;

	/* cir is enable fill profile table */
	if (info_enable == TRUE)
	{
        if (cir != 0) {
            /* transfer user values (cir, cbs) to mnt, exp and res */
            res = arad_pp_mtr_profile_rate_to_res_exp_mnt(
                unit,
                cir,
                &profile_tbl_data.meter_resolution,
                &profile_tbl_data.rate_mantissa,
                &profile_tbl_data.rate_exp
                );
            SOCDNX_SAND_IF_ERR_EXIT(res);
        } else {
            profile_tbl_data.meter_resolution = 0;
            profile_tbl_data.rate_mantissa = 0;
            profile_tbl_data.rate_exp = 0;
        }

		/* res also affects the burst: actual-burst = user-burst << res */
		res = arad_pp_mtr_bs_val_to_exp_mnt(
			unit, 
			cbs,
			&profile_tbl_data.burst_exp,
			&profile_tbl_data.burst_mantissa
			);
		SOCDNX_SAND_IF_ERR_EXIT(res);

		arad_pp_mtr_verify_valid_bucket_size(unit,
											 profile_tbl_data.rate_mantissa,profile_tbl_data.rate_exp,
											 0,0,
											 &profile_tbl_data.burst_mantissa,&profile_tbl_data.burst_exp);

		profile_tbl_data.packet_mode = SOC_SAND_BOOL2NUM(policer_info->is_packet_mode);
		profile_tbl_data.pkt_adj_header_truncate = SOC_SAND_BOOL2NUM(policer_info->is_pkt_truncate);
		profile_tbl_data.color_blind = policer_info->color_mode == SOC_PPC_MTR_COLOR_MODE_BLIND ? 1 : 0;
		
		arad_pp_mtr_eth_policer_profile_key_get(
			unit,
			profile_tbl_data,
			&eth_mtr_profile_multiset_key
			);
    
		res = arad_sw_db_multiset_add(
			unit,
			core_id,
			ARAD_PP_SW_DB_MULTI_SET_ETH_POLICER_ETH_METER_PROFILE,
			&eth_mtr_profile_multiset_key,
			&eth_mtr_profile_multiset_ndx,
			&eth_mtr_profile_multiset_first_appear,
			&eth_mtr_profile_multiset_success
			);
		SOCDNX_SAND_IF_ERR_EXIT(res);

		if (eth_mtr_profile_multiset_success != SOC_SAND_SUCCESS)
		{ 
			if (sw_db_enable_bit == TRUE) {
				/* We already remove the old profile of this port from the multiset list
				   So, if from any reason we couldn't add the new profile to the multiset
				   we write again the old one (by reading it from the HW first
				*/
				res = soc_mem_read(soc_sand_dev_id, eth_mtr_prfCfg_0_mem, ARAD_PP_ETH_MTR_MEM_BLOCK(unit, core_id), profile, (void*)eth_mtr_prfCfg_above_64_val);
				SOCDNX_IF_ERR_EXIT(res);

				profile_tbl_data.rate_mantissa 	  = soc_mem_field32_get(unit, eth_mtr_prfCfg_fields_mem, (void*)eth_mtr_prfCfg_above_64_val, CIR_MANTf);
			
				profile_tbl_data.rate_exp 		  = soc_mem_field32_get(unit, eth_mtr_prfCfg_fields_mem, (void*)eth_mtr_prfCfg_above_64_val, CIR_MANT_EXPf);
				profile_tbl_data.burst_mantissa   = soc_mem_field32_get(unit, eth_mtr_prfCfg_fields_mem, (void*)eth_mtr_prfCfg_above_64_val, CBS_MANT_64f);
				profile_tbl_data.burst_exp 		  = soc_mem_field32_get(unit, eth_mtr_prfCfg_fields_mem, (void*)eth_mtr_prfCfg_above_64_val, CBS_EXPONENTf);
				profile_tbl_data.packet_mode 	  = soc_mem_field32_get(unit, eth_mtr_prfCfg_fields_mem, (void*)eth_mtr_prfCfg_above_64_val, PACKET_MODEf);
				profile_tbl_data.meter_resolution = soc_mem_field32_get(unit, eth_mtr_prfCfg_fields_mem, (void*)eth_mtr_prfCfg_above_64_val, CIR_REV_EXP_2f);
				profile_tbl_data.color_blind 	  = SOC_SAND_BOOL2NUM_INVERSE(soc_mem_field32_get(unit, eth_mtr_prfCfg_fields_mem, (void*)eth_mtr_prfCfg_above_64_val, COLOR_AWAREf));
				profile_tbl_data.pkt_adj_header_truncate = soc_mem_field32_get(unit, eth_mtr_prfCfg_fields_mem, (void*)eth_mtr_prfCfg_above_64_val, MTR_PROFILE_MAP_TO_4f);

				arad_pp_mtr_eth_policer_profile_key_get(
					unit,
					profile_tbl_data,
					&eth_mtr_profile_multiset_key
					);

				res = arad_sw_db_multiset_add(
					unit,
					core_id,
					ARAD_PP_SW_DB_MULTI_SET_ETH_POLICER_ETH_METER_PROFILE,
					&eth_mtr_profile_multiset_key,
					&eth_mtr_profile_multiset_ndx,
					&eth_mtr_profile_multiset_first_appear,
					&eth_mtr_profile_multiset_success
					);
				SOCDNX_SAND_IF_ERR_EXIT(res);

				/*Profile wasn't changed, no need to init the bucket level*/
				init_bucket_level = 0;
			}
			else{
				SOCDNX_EXIT_WITH_ERR(SOC_E_RESOURCE, (_BSL_SOCDNX_MSG("Policer set - out of resources.\n")));
			}
		}
	}

	/* Set sw_db enable bit */
	res = arad_pp_sw_db_eth_policer_config_status_bit_set(
		unit,
		core_id,
		status_tbl_offset,
		info_enable
		);

	SOCDNX_IF_ERR_EXIT(res);

	if (info_enable == TRUE) {
		/* Set profile table - if first */
		if (
			(eth_mtr_profile_multiset_first_appear == 0x1) || 											/* Set profile table - if first */
			((eth_mtr_profile_multiset_last_appear == 0x1) && (eth_mtr_profile_multiset_ndx != profile))/* Set profile table - if last */
			)
		{
			soc_mem_field32_set(unit, eth_mtr_prfCfg_fields_mem, (void*)eth_mtr_prfCfg_above_64_val, RESET_CIRf, policer_info->disable_cir);
			soc_mem_field32_set(unit, eth_mtr_prfCfg_fields_mem, (void*)eth_mtr_prfCfg_above_64_val, CIR_MANTf, profile_tbl_data.rate_mantissa);
			soc_mem_field32_set(unit, eth_mtr_prfCfg_fields_mem, (void*)eth_mtr_prfCfg_above_64_val, CIR_MANT_EXPf, profile_tbl_data.rate_exp);
			soc_mem_field32_set(unit, eth_mtr_prfCfg_fields_mem, (void*)eth_mtr_prfCfg_above_64_val, CBS_MANT_64f, profile_tbl_data.burst_mantissa);
			soc_mem_field32_set(unit, eth_mtr_prfCfg_fields_mem, (void*)eth_mtr_prfCfg_above_64_val, CBS_EXPONENTf, profile_tbl_data.burst_exp);
			soc_mem_field32_set(unit, eth_mtr_prfCfg_fields_mem, (void*)eth_mtr_prfCfg_above_64_val, PACKET_MODEf, profile_tbl_data.packet_mode);
			soc_mem_field32_set(unit, eth_mtr_prfCfg_fields_mem, (void*)eth_mtr_prfCfg_above_64_val, CIR_REV_EXP_2f, profile_tbl_data.meter_resolution);
			soc_mem_field32_set(unit, eth_mtr_prfCfg_fields_mem, (void*)eth_mtr_prfCfg_above_64_val, COLOR_AWAREf, (uint32)SOC_SAND_BOOL2NUM_INVERSE(profile_tbl_data.color_blind));
			/* 
			   All meter porfiles are mapped by default to MtrPrf3 0 (aka mtr_prf below).
			*/
			soc_mem_field32_set(unit, eth_mtr_prfCfg_fields_mem, (void*)eth_mtr_prfCfg_above_64_val, MTR_PROFILE_MAP_TO_3f, 0);
			/* 
			  mtr_profile_map_to4[0] ->  header_delta
			  mtr_profile_map_to4[1] ->  header_truncate
			  mtr_profile_map_to4[2] ->  header_append_size_ptr
			  mtr_profile_map_to4[3] ->  in_pp_port_and_mtr_prf_map
			 
			  Add the value of in_pp_port_and_mtr_prf_map and header_delta to the packet size.
			*/
			if (policer_info->is_pkt_truncate) {
				soc_mem_field32_set(unit, eth_mtr_prfCfg_fields_mem, (void *)eth_mtr_prfCfg_above_64_val, MTR_PROFILE_MAP_TO_4f, 11);
			}else{
				soc_mem_field32_set(unit, eth_mtr_prfCfg_fields_mem, (void *)eth_mtr_prfCfg_above_64_val, MTR_PROFILE_MAP_TO_4f, 9);
			}

			res = soc_mem_write(soc_sand_dev_id, eth_mtr_prfCfg_0_mem, ARAD_PP_ETH_MTR_MEM_BLOCK(unit, core_id), eth_mtr_profile_multiset_ndx, (void*)eth_mtr_prfCfg_above_64_val); 
			SOCDNX_IF_ERR_EXIT(res);
			res = soc_mem_write(soc_sand_dev_id, eth_mtr_prfCfg_1_mem, ARAD_PP_ETH_MTR_MEM_BLOCK(unit, core_id), eth_mtr_profile_multiset_ndx, (void*)eth_mtr_prfCfg_above_64_val); 
			SOCDNX_IF_ERR_EXIT(res);
		}
	}

	/* Set pointer in plfSel table */
	soc_mem_field32_set(unit, eth_mtr_prfSel_mem, (void*)&eth_mtr_prfSel_val, profile_field[prfSel_tbl_offset],eth_mtr_profile_multiset_ndx);
	res = soc_mem_write(soc_sand_dev_id, eth_mtr_prfSel_mem, ARAD_PP_ETH_MTR_MEM_BLOCK(unit, core_id), prfSel_tbl_entry, (void*)&eth_mtr_prfSel_val);
	SOCDNX_IF_ERR_EXIT(res);

	if (init_bucket_level == 1) {
		/* Set CBL to the bucket size of the new profile */
		/* a. check if we have write access for dynamic memory, if not- take it*/
		soc_reg32_get(unit, eth_mtr_enable_dyn_access, core_id, 0, (void*)&write_access_enable);
		if (0 == write_access_enable) {
			res = soc_reg32_set(unit, eth_mtr_enable_dyn_access, core_id, 0, 1);
			SOCDNX_IF_ERR_EXIT(res);
		}

		/* b. set matching CBL to CBS*/
		res = soc_mem_read(soc_sand_dev_id, eth_mtr_bck_lvl_mem, ARAD_PP_ETH_MTR_MEM_BLOCK(unit, core_id), bckLvl_tbl_entry, (void*)eth_mtr_bck_lvl_above_64_val); 
		SOCDNX_IF_ERR_EXIT(res);

		ARAD_PP_MTR_DECIMAL_TO_BUKCET(cbs, backet_init_val);
		soc_mem_field32_set(unit, eth_mtr_bck_lvl_mem, (void*)eth_mtr_bck_lvl_above_64_val, cbl_bucket_fld[bckLvl_tbl_offset],backet_init_val);

		res = soc_mem_write(soc_sand_dev_id, eth_mtr_bck_lvl_mem, ARAD_PP_ETH_MTR_MEM_BLOCK(unit, core_id), bckLvl_tbl_entry, (void*)eth_mtr_bck_lvl_above_64_val);
		SOCDNX_IF_ERR_EXIT(res);

		/* c. if we took write access, disable it*/
		if (0 == write_access_enable) {
			res = soc_reg32_set(unit, eth_mtr_enable_dyn_access, core_id, 0, 0);
			SOCDNX_IF_ERR_EXIT(res);
		}

	}

	/* Set the validity bit in the eth. meter config table */
	if (SOC_IS_QAX(unit)) {
		soc_mem_field32_set(unit, eth_mtr_config_mem, &eth_mtr_config_val, ETH_MTR_PTR_VALIDf, info_enable);
		res = soc_mem_write(soc_sand_dev_id, eth_mtr_config_mem, MEM_BLOCK_ANY, config_tbl_entry, (void*)&eth_mtr_config_val);
		SOCDNX_IF_ERR_EXIT(res);
	}else{
		soc_mem_field32_set(unit, eth_mtr_config_mem, &eth_mtr_config_val, ETHERNET_METER_PROFILE_VALIDf, info_enable);
		res = soc_mem_array_write(soc_sand_dev_id, eth_mtr_config_mem, config_array_index, MEM_BLOCK_ANY, config_tbl_entry, (void*)&eth_mtr_config_val);
		SOCDNX_IF_ERR_EXIT(res);
	}

exit:
	SOCDNX_FUNC_RETURN;
}

uint32
  jer_pp_mtr_eth_policer_params_get(
    SOC_SAND_IN  int                        	unit,
	SOC_SAND_IN  int                         	core_id,
    SOC_SAND_IN  SOC_PPC_PORT                   port_ndx,
    SOC_SAND_IN  SOC_PPC_MTR_ETH_TYPE           eth_type_ndx,
    SOC_SAND_OUT SOC_PPC_MTR_BW_PROFILE_INFO    *policer_info
  )
{
  unsigned int
	soc_sand_dev_id;

  uint32
	res,
	profile,
    status_tbl_offset,
    prfSel_tbl_offset,
	prfSel_tbl_entry;

  soc_reg_above_64_val_t
    eth_mtr_prfCfg_above_64_val;
  uint32
	eth_mtr_prfSel_val;

  int
	eth_mtr_prfSel_mem,
	eth_mtr_prfCfg_fields_mem,
    eth_mtr_prfCfg_mem;

  uint8
    sw_db_enable_bit;

  ARAD_IDR_ETHERNET_METER_PROFILES_TBL_DATA
    profile_tbl_data;

uint32
	profile_field[4] = {PROFILE_0f, PROFILE_1f, PROFILE_2f, PROFILE_3f};

	SOCDNX_INIT_FUNC_DEFS;
	SOCDNX_NULL_CHECK(policer_info);
	
	soc_sand_dev_id = (unit);
	eth_mtr_prfSel_mem 			= JER_MRPS_EM_MEM_FORMAT_BY_CHIP(unit,MCDA_PRFSEL);
	eth_mtr_prfCfg_fields_mem 	= JER_MRPS_EM_MEM_FORMAT_BY_CHIP(unit,MCDA_PRFCFG_SHARING_DIS); /* using prf cfg 1 because of sharing flag is disabled on eth. meter*/
	eth_mtr_prfCfg_mem 			= JER_MRPS_EM_MEM_FORMAT_BY_CHIP(unit,MCDA_PRFCFG_0); /* using prf cfg 1 because of sharing flag is disabled on eth. meter*/

	status_tbl_offset = port_ndx * SOC_PPC_NOF_MTR_ETH_TYPES + eth_type_ndx;
	/*
	    below: divide and modulu by 4.
	    the profile selection table format is:
							offset
			-------------------------------------------------------------
			| policer 0   | policer 1    | policer 2    | policer 3     |
		e	--------------------------------------------------------------
		n		.			.					.				.
		t		.			.					.				.
		r		.			.					.				.
		y	-------------------------------------------------------------
			| policer 4k  | policer 4k+1 | policer 4k+2 | policer 4k+3  |
			-------------------------------------------------------------
	*/
	prfSel_tbl_entry  = status_tbl_offset >> 2;  /* /4 */
	prfSel_tbl_offset = status_tbl_offset & 0x3; /* %4 */

	/* Get pointers from prfSel table*/
	res = soc_mem_read(soc_sand_dev_id, eth_mtr_prfSel_mem, ARAD_PP_ETH_MTR_MEM_BLOCK(unit, core_id), prfSel_tbl_entry, &eth_mtr_prfSel_val);
	SOCDNX_IF_ERR_EXIT(res);

	profile = soc_mem_field32_get(unit, eth_mtr_prfSel_mem, &eth_mtr_prfSel_val, profile_field[prfSel_tbl_offset]);

	res = arad_sw_db_multiset_get_enable_bit(
			unit,
			core_id,
			status_tbl_offset,
			&sw_db_enable_bit
			);
	SOCDNX_SAND_IF_ERR_EXIT(res);

	if (sw_db_enable_bit == TRUE) {
		/* Get committed mnt, exp values from prfCfg table */
		res = soc_mem_read(soc_sand_dev_id, eth_mtr_prfCfg_mem, ARAD_PP_ETH_MTR_MEM_BLOCK(unit, core_id), profile, (void*)eth_mtr_prfCfg_above_64_val);
		SOCDNX_IF_ERR_EXIT(res);

		profile_tbl_data.rate_mantissa 	  = soc_mem_field32_get(unit, eth_mtr_prfCfg_fields_mem, (void*)eth_mtr_prfCfg_above_64_val, CIR_MANTf);
		profile_tbl_data.rate_exp 	 	  = soc_mem_field32_get(unit, eth_mtr_prfCfg_fields_mem, (void*)eth_mtr_prfCfg_above_64_val, CIR_MANT_EXPf);
		profile_tbl_data.burst_mantissa   = soc_mem_field32_get(unit, eth_mtr_prfCfg_fields_mem, (void*)eth_mtr_prfCfg_above_64_val, CBS_MANT_64f);
		profile_tbl_data.burst_exp 		  = soc_mem_field32_get(unit, eth_mtr_prfCfg_fields_mem, (void*)eth_mtr_prfCfg_above_64_val, CBS_EXPONENTf);
		profile_tbl_data.packet_mode 	  = soc_mem_field32_get(unit, eth_mtr_prfCfg_fields_mem, (void*)eth_mtr_prfCfg_above_64_val, PACKET_MODEf);
		profile_tbl_data.pkt_adj_header_truncate = soc_mem_field32_get(unit, eth_mtr_prfCfg_fields_mem, (void*)eth_mtr_prfCfg_above_64_val, MTR_PROFILE_MAP_TO_4f);
		profile_tbl_data.meter_resolution = soc_mem_field32_get(unit, eth_mtr_prfCfg_fields_mem, (void*)eth_mtr_prfCfg_above_64_val, CIR_REV_EXP_2f);
		profile_tbl_data.color_blind 	  = SOC_SAND_BOOL2NUM_INVERSE(soc_mem_field32_get(unit, eth_mtr_prfCfg_fields_mem, (void*)eth_mtr_prfCfg_above_64_val, COLOR_AWAREf));

		/* calculate the policer_info parameters from the HW read values */
		policer_info->is_packet_mode = SOC_SAND_NUM2BOOL(profile_tbl_data.packet_mode);
		policer_info->is_pkt_truncate= SOC_SAND_NUM2BOOL(profile_tbl_data.pkt_adj_header_truncate & 0x2);
		policer_info->color_mode 	 = profile_tbl_data.color_blind ? SOC_PPC_MTR_COLOR_MODE_BLIND : SOC_PPC_MTR_COLOR_MODE_AWARE;
		policer_info->disable_cir 	 = soc_mem_field32_get(unit, eth_mtr_prfCfg_fields_mem, (void*)eth_mtr_prfCfg_above_64_val, RESET_CIRf);

		res = arad_pp_mtr_ir_val_from_reverse_exp_mnt(
			unit,
			profile_tbl_data.meter_resolution,
			profile_tbl_data.rate_exp,
			profile_tbl_data.rate_mantissa,
			&policer_info->cir
			);
		SOCDNX_SAND_IF_ERR_EXIT(res);

		res = soc_sand_compute_complex_to_mnt_exp(
			profile_tbl_data.burst_mantissa,
			profile_tbl_data.burst_exp,
			JER_PP_MTR_VAL_EXP_MNT_EQ_CONST_MULTI_BS,
			JER_PP_MTR_PROFILE_VAL_EXP_MNT_EQ_CONST_MNT_INC_BS,
			&policer_info->cbs
			);
		SOCDNX_SAND_IF_ERR_EXIT(res);
	}else{
		policer_info->cbs = 0;
		policer_info->cir = 0;
		policer_info->color_mode = 0;
		policer_info->is_packet_mode = 0;
		policer_info->is_pkt_truncate = 0;
		policer_info->color_mode = 0;
	}
exit:
	SOCDNX_FUNC_RETURN;
}

uint32
	jer_pp_mtr_eth_policer_glbl_profile_set(
	   SOC_SAND_IN int       						unit,
	   SOC_SAND_IN int								core_id,
	   SOC_SAND_IN uint32	                		policer_ndx,
	   SOC_SAND_IN SOC_PPC_MTR_BW_PROFILE_INFO    	*policer_info
	)
{
  unsigned int
	soc_sand_dev_id;

  uint32
	res;
  uint8
	glbl_info_enable,
	write_access_enable;
  uint32 cir,cbs;
  uint32
	prfCfg_tbl_entry,
	prfSel_tbl_offset,
	prfSel_tbl_entry,
	bckLvl_tbl_entry,
	bckLvl_tbl_offset;

  soc_reg_above_64_val_t
	eth_mtr_prfCfg_above_64_val,
	eth_mtr_bck_lvl_above_64_val;
  uint32
	eth_mtr_prfSel_val = 0,
	backet_init_val = 0;

  soc_mem_t
	eth_mtr_prfSel_mem,
	eth_mtr_prfCfg_fields_mem,
	eth_mtr_prfCfg_0_mem,
	eth_mtr_prfCfg_1_mem,
	eth_mtr_bck_lvl_mem,
	eth_mtr_enable_dyn_access;

	ARAD_IDR_ETHERNET_METER_PROFILES_TBL_DATA
	profile_tbl_data;

  uint32
	profile_field[4] = {PROFILE_0f, PROFILE_1f, PROFILE_2f, PROFILE_3f};
  uint32
	cbl_bucket_fld[4]= {CBL_0f	  , CBL_1f    , CBL_2f    , CBL_3f};

	SOCDNX_INIT_FUNC_DEFS;
	SOCDNX_NULL_CHECK(policer_info);
	SOC_REG_ABOVE_64_CLEAR(eth_mtr_prfCfg_above_64_val);
	SOC_REG_ABOVE_64_CLEAR(eth_mtr_bck_lvl_above_64_val);

	glbl_info_enable = SOC_SAND_BOOL2NUM_INVERSE(policer_info->disable_cir);

	soc_sand_dev_id = (unit);
	eth_mtr_prfSel_mem 			= JER_MRPS_EM_MEM_FORMAT_BY_CHIP(unit,MCDB_PRFSEL);
	eth_mtr_prfCfg_fields_mem 	= JER_MRPS_EM_MEM_FORMAT_BY_CHIP(unit,MCDA_PRFCFG_SHARING_DIS); /* using prf cfg 1 because of sharing flag is disabled on eth. meter*/
    eth_mtr_prfCfg_0_mem 		= JER_MRPS_EM_MEM_FORMAT_BY_CHIP(unit,MCDB_PRFCFG_0);
	eth_mtr_prfCfg_1_mem 		= JER_MRPS_EM_MEM_FORMAT_BY_CHIP(unit,MCDB_PRFCFG_1);
	eth_mtr_bck_lvl_mem			= JER_MRPS_EM_MEM_FORMAT_BY_CHIP(unit,MCDB_DYNAMIC);
	eth_mtr_enable_dyn_access   = JER_MRPS_EM_REG_FORMAT_BY_CHIP(unit,ENABLE_DYNAMIC_MEMORY_ACCESS);
	
	prfCfg_tbl_entry   = policer_ndx;
	/*
	  below: divide and modulu by 4.
	  the profile selection table format is:
						  offset
	e	--------------------------------------------------------------
	n	  | policer 0   | policer 1    | policer 2    | policer 3     |
	t	--------------------------------------------------------------
	r	  | policer 4   | policer 5    | policer 6    | policer 7     |
	y 	--------------------------------------------------------------
	*/
	bckLvl_tbl_entry = prfSel_tbl_entry  = prfCfg_tbl_entry >> 2;  /* /4 */
	bckLvl_tbl_offset= prfSel_tbl_offset = prfCfg_tbl_entry & 0x3; /* %4 */

	/* Get values from tables */  
	res = soc_mem_read(soc_sand_dev_id, eth_mtr_prfSel_mem, ARAD_PP_ETH_MTR_MEM_BLOCK(unit, core_id), prfSel_tbl_entry, (void*)&eth_mtr_prfSel_val);
	SOCDNX_IF_ERR_EXIT(res);

	cir = policer_info->cir;
	cbs = policer_info->cbs;

	if(glbl_info_enable == TRUE){
		/* transfer user values (cir, cbs) to mnt, exp and res */
		res = arad_pp_mtr_profile_rate_to_res_exp_mnt(
		   unit,
		   cir,
		   &profile_tbl_data.meter_resolution,
		   &profile_tbl_data.rate_mantissa,
		   &profile_tbl_data.rate_exp
		   );
		SOCDNX_SAND_IF_ERR_EXIT(res);

		/* res also affects the burst: actual-burst = user-burst << res */
		res = arad_pp_mtr_bs_val_to_exp_mnt(
			unit, 
			cbs,
			&profile_tbl_data.burst_exp,
			&profile_tbl_data.burst_mantissa
			);
		SOCDNX_SAND_IF_ERR_EXIT(res);

		arad_pp_mtr_verify_valid_bucket_size(unit,
											 profile_tbl_data.rate_mantissa,profile_tbl_data.rate_exp,
											 0,0,
											 &profile_tbl_data.burst_mantissa,&profile_tbl_data.burst_exp);

		res = arad_pp_sw_db_eth_policer_config_status_bit_set(
			unit,
			core_id,
			ARAD_PP_SW_DB_MULTI_SET_ETH_POLICER_CONFIG_METER_PROFILE_NOF_MEMBER_BIT + policer_ndx,
			glbl_info_enable
		);

		SOCDNX_SAND_IF_ERR_EXIT(res);
		profile_tbl_data.packet_mode = SOC_SAND_BOOL2NUM(policer_info->is_packet_mode);
		profile_tbl_data.color_blind = policer_info->color_mode == SOC_PPC_MTR_COLOR_MODE_BLIND ? 1 : 0;
		/* If the mode is color blind then all policers are color blind. */
		if (soc_property_get(unit, spn_RATE_COLOR_BLIND, 0) || policer_info->color_mode == SOC_PPC_MTR_COLOR_MODE_BLIND) {
		  profile_tbl_data.color_blind = 1;
		}
	
		/* write to MRPS_EM prfCfg table*/
		soc_mem_field32_set(unit, eth_mtr_prfCfg_fields_mem, (void*)eth_mtr_prfCfg_above_64_val, RESET_CIRf, policer_info->disable_cir);
		soc_mem_field32_set(unit, eth_mtr_prfCfg_fields_mem, (void*)eth_mtr_prfCfg_above_64_val, CIR_MANTf, profile_tbl_data.rate_mantissa);
		soc_mem_field32_set(unit, eth_mtr_prfCfg_fields_mem, (void*)eth_mtr_prfCfg_above_64_val, CIR_MANT_EXPf, profile_tbl_data.rate_exp);
		soc_mem_field32_set(unit, eth_mtr_prfCfg_fields_mem, (void*)eth_mtr_prfCfg_above_64_val, CBS_MANT_64f, profile_tbl_data.burst_mantissa);
		soc_mem_field32_set(unit, eth_mtr_prfCfg_fields_mem, (void*)eth_mtr_prfCfg_above_64_val, CBS_EXPONENTf, profile_tbl_data.burst_exp);
		soc_mem_field32_set(unit, eth_mtr_prfCfg_fields_mem, (void*)eth_mtr_prfCfg_above_64_val, PACKET_MODEf, profile_tbl_data.packet_mode);
		soc_mem_field32_set(unit, eth_mtr_prfCfg_fields_mem, (void*)eth_mtr_prfCfg_above_64_val, CIR_REV_EXP_2f, profile_tbl_data.meter_resolution);
		soc_mem_field32_set(unit, eth_mtr_prfCfg_fields_mem, (void*)eth_mtr_prfCfg_above_64_val, COLOR_AWAREf, (uint32)SOC_SAND_BOOL2NUM_INVERSE(profile_tbl_data.color_blind));

		/* 
			All meter porfiles are mapped by default to MtrPrf3 0 (aka mtr_prf below).
		*/
		soc_mem_field32_set(unit, eth_mtr_prfCfg_fields_mem, (void*)eth_mtr_prfCfg_above_64_val, MTR_PROFILE_MAP_TO_3f, 0);
		/*
			mtr_profile_map_to4[0] ->  header_delta 
			mtr_profile_map_to4[1] ->  header_truncate 
			mtr_profile_map_to4[2] ->  header_append_size_ptr 
			mtr_profile_map_to4[3] ->  in_pp_port_and_mtr_prf_map
		 
			Add the value of in_pp_port_and_mtr_prf_map and header_delta to the packet size.
		*/
		if (policer_info->is_pkt_truncate) {
			soc_mem_field32_set(unit, eth_mtr_prfCfg_fields_mem, (void *)eth_mtr_prfCfg_above_64_val, MTR_PROFILE_MAP_TO_4f, 11);
		}else{
			soc_mem_field32_set(unit, eth_mtr_prfCfg_fields_mem, (void *)eth_mtr_prfCfg_above_64_val, MTR_PROFILE_MAP_TO_4f, 9);
		}

		res = soc_mem_write(soc_sand_dev_id, eth_mtr_prfCfg_0_mem, ARAD_PP_ETH_MTR_MEM_BLOCK(unit, core_id), prfCfg_tbl_entry, (void*)eth_mtr_prfCfg_above_64_val); 
		SOCDNX_IF_ERR_EXIT(res);
		res = soc_mem_write(soc_sand_dev_id, eth_mtr_prfCfg_1_mem, ARAD_PP_ETH_MTR_MEM_BLOCK(unit, core_id), prfCfg_tbl_entry, (void*)eth_mtr_prfCfg_above_64_val); 
		SOCDNX_IF_ERR_EXIT(res);
	
		/* Set pointer in plfSel table */
		soc_mem_field32_set(unit, eth_mtr_prfSel_mem, &eth_mtr_prfSel_val, profile_field[prfSel_tbl_offset],policer_ndx);
		res = soc_mem_write(soc_sand_dev_id, eth_mtr_prfSel_mem, ARAD_PP_ETH_MTR_MEM_BLOCK(unit, core_id), prfSel_tbl_entry, (void*)&eth_mtr_prfSel_val);
		SOCDNX_IF_ERR_EXIT(res);
	}else{
		/*if (){ //if destroy
			res = arad_pp_sw_db_eth_policer_config_status_bit_set(
				unit,
				core_id,
				ARAD_PP_SW_DB_MULTI_SET_ETH_POLICER_CONFIG_METER_PROFILE_NOF_MEMBER_BIT + policer_ndx,
				glbl_info_enable
			);
			SOCDNX_SAND_IF_ERR_EXIT(res);
		 
			soc_mem_field32_set(unit, eth_mtr_config_mem, &eth_mtr_config_val, ETHERNET_METER_PROFILE_VALIDf, info_enable);
			res = soc_mem_array_write(soc_sand_dev_id, eth_mtr_config_mem, config_array_index, MEM_BLOCK_ANY, config_tbl_entry, (void*)&eth_mtr_config_val);
			SOCDNX_IF_ERR_EXIT(res); 
		}*/

		/* Set pointer in prfSel table to deault 0*/
		soc_mem_field32_set(unit, eth_mtr_prfSel_mem, &eth_mtr_prfSel_val, profile_field[prfSel_tbl_offset],0);
		res = soc_mem_write(soc_sand_dev_id, eth_mtr_prfSel_mem, ARAD_PP_ETH_MTR_MEM_BLOCK(unit, core_id), prfSel_tbl_entry, (void*)&eth_mtr_prfSel_val);
		SOCDNX_IF_ERR_EXIT(res);

		/* clear CBL from remaining tokens from previous profiles */
		/* a. check if we have write access for dynamic memory, if not- take it*/
		soc_reg32_get(unit, eth_mtr_enable_dyn_access, core_id, 0, (void*)&write_access_enable);
		if (0 == write_access_enable) {
			res = soc_reg32_set(unit, eth_mtr_enable_dyn_access, core_id, 0, 1);
			SOCDNX_IF_ERR_EXIT(res);
		}

		/* b. set matching CBL to -8193 (compensate for LFSR)*/
		res = soc_mem_read(soc_sand_dev_id, eth_mtr_bck_lvl_mem, ARAD_PP_ETH_MTR_MEM_BLOCK(unit, core_id), bckLvl_tbl_entry, (void*)eth_mtr_bck_lvl_above_64_val); 
		SOCDNX_IF_ERR_EXIT(res);

		backet_init_val = 0x7FDFFF;
		soc_mem_field32_set(unit, eth_mtr_bck_lvl_mem, (void*)eth_mtr_bck_lvl_above_64_val, cbl_bucket_fld[bckLvl_tbl_offset],backet_init_val);

		res = soc_mem_write(soc_sand_dev_id, eth_mtr_bck_lvl_mem, ARAD_PP_ETH_MTR_MEM_BLOCK(unit, core_id), bckLvl_tbl_entry, (void*)eth_mtr_bck_lvl_above_64_val);
		SOCDNX_IF_ERR_EXIT(res);

		/* c. if we took write access, disable it*/
		if (0 == write_access_enable) {
			res = soc_reg32_set(unit, eth_mtr_enable_dyn_access, core_id, 0, 0);
			SOCDNX_IF_ERR_EXIT(res);
		}
	}
exit:
	SOCDNX_FUNC_RETURN;
}

uint32  
  jer_pp_mtr_eth_policer_glbl_profile_get(
	 SOC_SAND_IN  int                      		unit,
	 SOC_SAND_IN  int                      		core_id,
	 SOC_SAND_IN  uint32                      	glbl_profile_idx,
	 SOC_SAND_OUT SOC_PPC_MTR_BW_PROFILE_INFO 	*policer_info
  )
{
  unsigned int
	soc_sand_dev_id;

  uint32
	res,
	profile;
  uint8
	sw_db_enable_bit;

  uint32
	prfCfg_tbl_entry,
	prfSel_tbl_offset,
	prfSel_tbl_entry;

  soc_reg_above_64_val_t
	eth_mtr_prfCfg_above_64_val;
  uint32
	eth_mtr_prfSel_val = 0;

  soc_mem_t
	eth_mtr_prfSel_mem,
	eth_mtr_prfCfg_fields_mem,
	eth_mtr_prfCfg_mem;

	ARAD_IDR_ETHERNET_METER_PROFILES_TBL_DATA
	profile_tbl_data;

  uint32
    profile_field[4] = {PROFILE_0f, PROFILE_1f, PROFILE_2f, PROFILE_3f};

	SOCDNX_INIT_FUNC_DEFS;
	SOCDNX_NULL_CHECK(policer_info);
	SOC_REG_ABOVE_64_CLEAR(eth_mtr_prfCfg_above_64_val);
	SOC_PPC_MTR_BW_PROFILE_INFO_clear(policer_info);

	soc_sand_dev_id = (unit);
	eth_mtr_prfSel_mem 			= JER_MRPS_EM_MEM_FORMAT_BY_CHIP(unit,MCDB_PRFSEL);
	eth_mtr_prfCfg_fields_mem 	= JER_MRPS_EM_MEM_FORMAT_BY_CHIP(unit,MCDA_PRFCFG_SHARING_DIS); /* using prf cfg 1 because of sharing flag is disabled on eth. meter*/
	eth_mtr_prfCfg_mem 			= JER_MRPS_EM_MEM_FORMAT_BY_CHIP(unit,MCDB_PRFCFG_0); 			 /* using prf cfg 1 because of sharing flag is disabled on eth. meter*/

	prfCfg_tbl_entry = glbl_profile_idx;
	/*
	    below: divide and modulu by 4.
	    the profile selection table format is:
							offset
			-------------------------------------------------------------
			| policer 0   | policer 1    | policer 2    | policer 3     |
		e	--------------------------------------------------------------
		n		.			.					.				.
		t		.			.					.				.
		r		.			.					.				.
		y	-------------------------------------------------------------
			| policer 4k  | policer 4k+1 | policer 4k+2 | policer 4k+3  |
			-------------------------------------------------------------
	*/
	prfSel_tbl_entry  = prfCfg_tbl_entry >> 2;  /* /4 */
	prfSel_tbl_offset = prfCfg_tbl_entry & 0x3; /* %4 */

	/* Get pointers from prfSel table*/
	res = soc_mem_read(soc_sand_dev_id, eth_mtr_prfSel_mem, ARAD_PP_ETH_MTR_MEM_BLOCK(unit, core_id), prfSel_tbl_entry, &eth_mtr_prfSel_val);
	SOCDNX_IF_ERR_EXIT(res);

	profile = soc_mem_field32_get(unit, eth_mtr_prfSel_mem, &eth_mtr_prfSel_val, profile_field[prfSel_tbl_offset]);

	/* Get committed mnt, exp values from prfCfg table */
	res = soc_mem_read(soc_sand_dev_id, eth_mtr_prfCfg_mem, ARAD_PP_ETH_MTR_MEM_BLOCK(unit, core_id), profile, (void*)eth_mtr_prfCfg_above_64_val);
	SOCDNX_IF_ERR_EXIT(res);

	res = arad_sw_db_multiset_get_enable_bit(
			unit,
			core_id,
			ARAD_PP_SW_DB_MULTI_SET_ETH_POLICER_CONFIG_METER_PROFILE_NOF_MEMBER_BIT + prfCfg_tbl_entry,
			&sw_db_enable_bit
			);
	SOCDNX_SAND_IF_ERR_EXIT(res);

	if (sw_db_enable_bit == TRUE) {
		profile_tbl_data.rate_mantissa 	  = soc_mem_field32_get(unit, eth_mtr_prfCfg_fields_mem, (void*)eth_mtr_prfCfg_above_64_val, CIR_MANTf);
		profile_tbl_data.rate_exp 	 	  = soc_mem_field32_get(unit, eth_mtr_prfCfg_fields_mem, (void*)eth_mtr_prfCfg_above_64_val, CIR_MANT_EXPf);
		profile_tbl_data.burst_mantissa   = soc_mem_field32_get(unit, eth_mtr_prfCfg_fields_mem, (void*)eth_mtr_prfCfg_above_64_val, CBS_MANT_64f);
		profile_tbl_data.burst_exp 		  = soc_mem_field32_get(unit, eth_mtr_prfCfg_fields_mem, (void*)eth_mtr_prfCfg_above_64_val, CBS_EXPONENTf);
		profile_tbl_data.packet_mode 	  = soc_mem_field32_get(unit, eth_mtr_prfCfg_fields_mem, (void*)eth_mtr_prfCfg_above_64_val, PACKET_MODEf);
		profile_tbl_data.meter_resolution = soc_mem_field32_get(unit, eth_mtr_prfCfg_fields_mem, (void*)eth_mtr_prfCfg_above_64_val, CIR_REV_EXP_2f);
		profile_tbl_data.color_blind 	  = SOC_SAND_BOOL2NUM_INVERSE(soc_mem_field32_get(unit, eth_mtr_prfCfg_fields_mem, (void*)eth_mtr_prfCfg_above_64_val, COLOR_AWAREf));
		profile_tbl_data.pkt_adj_header_truncate = soc_mem_field32_get(unit, eth_mtr_prfCfg_fields_mem, (void*)eth_mtr_prfCfg_above_64_val, MTR_PROFILE_MAP_TO_4f);

		/* calculate the policer_info parameters from the HW read values */
		policer_info->is_packet_mode = SOC_SAND_NUM2BOOL(profile_tbl_data.packet_mode);
		policer_info->color_mode 	 = profile_tbl_data.color_blind ? SOC_PPC_MTR_COLOR_MODE_BLIND : SOC_PPC_MTR_COLOR_MODE_AWARE;
		policer_info->disable_cir 	 = soc_mem_field32_get(unit, eth_mtr_prfCfg_fields_mem, (void*)eth_mtr_prfCfg_above_64_val, RESET_CIRf);
		policer_info->is_pkt_truncate= SOC_SAND_NUM2BOOL(profile_tbl_data.pkt_adj_header_truncate & 0x2);

		res = arad_pp_mtr_ir_val_from_reverse_exp_mnt(
			unit,
			profile_tbl_data.meter_resolution,
			profile_tbl_data.rate_exp,
			profile_tbl_data.rate_mantissa,
			&policer_info->cir
			);
		SOCDNX_SAND_IF_ERR_EXIT(res);

		res = soc_sand_compute_complex_to_mnt_exp(
			profile_tbl_data.burst_mantissa,
			profile_tbl_data.burst_exp,
			JER_PP_MTR_VAL_EXP_MNT_EQ_CONST_MULTI_BS,
			JER_PP_MTR_PROFILE_VAL_EXP_MNT_EQ_CONST_MNT_INC_BS,
			&policer_info->cbs
			);
		SOCDNX_SAND_IF_ERR_EXIT(res);
	}else{
		policer_info->cbs = 0;
		policer_info->cir = 0;
		policer_info->color_mode = 0;
		policer_info->is_packet_mode = 0;
		policer_info->color_mode = 0;
	}

exit:
	 SOCDNX_FUNC_RETURN;
}

uint32  
  jer_pp_mtr_eth_policer_glbl_profile_map_set(
    SOC_SAND_IN  int                  	unit,
	SOC_SAND_IN  int                  	core_id,
    SOC_SAND_IN  SOC_PPC_PORT           port,
    SOC_SAND_IN  SOC_PPC_MTR_ETH_TYPE   eth_type_ndx,
    SOC_SAND_IN  uint32                 glbl_profile_idx
  )
{
  unsigned int 
	soc_sand_dev_id;

  uint32
	res,
    config_tbl_entry,
	eth_mtr_config_val,
    glbl_mtr_ptr_val;

  soc_mem_t
	eth_mtr_config_mem;
  uint32
	glbl_prf_ptr_field,
	glbl_prf_valid_field;

  int
	config_array_index;

	SOCDNX_INIT_FUNC_DEFS;

	soc_sand_dev_id = (unit);
	config_array_index = core_id;
	config_tbl_entry   = port * SOC_PPC_NOF_MTR_ETH_TYPES + eth_type_ndx;
	glbl_mtr_ptr_val = glbl_profile_idx;

	glbl_prf_ptr_field   = SOC_IS_QAX(unit) ? GLBL_ETH_MTR_PTRf : GLOBAL_METER_PROFILEf ;
	glbl_prf_valid_field = SOC_IS_QAX(unit) ? GLBL_ETH_MTR_PTR_VALIDf : GLOBAL_METER_PROFILE_VALIDf ;
	eth_mtr_config_mem	 = SOC_IS_QAX(unit) ? CGM_ETH_MTR_PTR_MAPm : IDR_ETHERNET_METER_CONFIGm;

	/* Get pointers from config table*/
	if (SOC_IS_QAX(unit)) {
		res = soc_mem_read(soc_sand_dev_id, eth_mtr_config_mem, MEM_BLOCK_ANY, config_tbl_entry, (void*)&eth_mtr_config_val);
		SOCDNX_IF_ERR_EXIT(res);
	}else{
		res = soc_mem_array_read(soc_sand_dev_id, eth_mtr_config_mem, config_array_index, MEM_BLOCK_ANY, config_tbl_entry, (void*)&eth_mtr_config_val);
		SOCDNX_IF_ERR_EXIT(res);
	}

	/* Set the validity bit in the eth. meter config table */
	soc_mem_field32_set(unit, eth_mtr_config_mem, &eth_mtr_config_val, glbl_prf_ptr_field, glbl_mtr_ptr_val);
	if (glbl_profile_idx == 0) { /* profile index 0 means to unset the port from any policer */
		soc_mem_field32_set(unit, eth_mtr_config_mem, &eth_mtr_config_val, glbl_prf_valid_field, 0);
	}else{
		soc_mem_field32_set(unit, eth_mtr_config_mem, &eth_mtr_config_val, glbl_prf_valid_field, 1);
	}

	if (SOC_IS_QAX(unit)) {
		res = soc_mem_write(soc_sand_dev_id, eth_mtr_config_mem, MEM_BLOCK_ANY, config_tbl_entry, (void*)&eth_mtr_config_val);
		SOCDNX_IF_ERR_EXIT(res);
	}else{
		res = soc_mem_array_write(soc_sand_dev_id, eth_mtr_config_mem, config_array_index, MEM_BLOCK_ANY, config_tbl_entry, (void*)&eth_mtr_config_val);
		SOCDNX_IF_ERR_EXIT(res);
	}
  
exit:
	SOCDNX_FUNC_RETURN;
}

uint32  
  jer_pp_mtr_eth_policer_glbl_profile_map_get(
	 SOC_SAND_IN  int                  	unit,
	 SOC_SAND_IN  int                  	core_id,
	 SOC_SAND_IN  SOC_PPC_PORT          port,
	 SOC_SAND_IN  SOC_PPC_MTR_ETH_TYPE  eth_type_ndx,
	 SOC_SAND_OUT uint32                *glbl_profile_idx
   )
 {
  unsigned int 
	soc_sand_dev_id;

  uint32
	res,
	config_tbl_entry,
	eth_mtr_config_val,
	glbl_mtr_ptr_val;

  soc_mem_t
	eth_mtr_config_mem;
  uint32
	glbl_prf_ptr_field;
  int
	config_array_index;

	SOCDNX_INIT_FUNC_DEFS;
	SOCDNX_NULL_CHECK(glbl_profile_idx);

	soc_sand_dev_id = (unit);
	config_array_index = core_id;
	config_tbl_entry   = port * SOC_PPC_NOF_MTR_ETH_TYPES + eth_type_ndx;

	glbl_prf_ptr_field   = SOC_IS_QAX(unit) ? GLBL_ETH_MTR_PTRf : GLOBAL_METER_PROFILEf ;	
    eth_mtr_config_mem	 = SOC_IS_QAX(unit) ? CGM_ETH_MTR_PTR_MAPm : IDR_ETHERNET_METER_CONFIGm;

    /* Get pointers from config table*/
    if (SOC_IS_QAX(unit)) {
		res = soc_mem_read(soc_sand_dev_id, eth_mtr_config_mem, MEM_BLOCK_ANY, config_tbl_entry, (void*)&eth_mtr_config_val);
		SOCDNX_IF_ERR_EXIT(res);
	}else{
		res = soc_mem_array_read(soc_sand_dev_id, eth_mtr_config_mem, config_array_index, MEM_BLOCK_ANY, config_tbl_entry, (void*)&eth_mtr_config_val);
		SOCDNX_IF_ERR_EXIT(res);
	}

	soc_mem_field_get(unit, eth_mtr_config_mem, &eth_mtr_config_val, glbl_prf_ptr_field, &glbl_mtr_ptr_val);
    *glbl_profile_idx = glbl_mtr_ptr_val;

 exit:
	 SOCDNX_FUNC_RETURN;
}

uint32 
jer_pp_metering_init_header_compensation(
	int unit
	)
{
uint32
	res,
	packet_size_profile_multiset_key = 0x0,
    packet_size_profile_multiset_ndx = 0x0;
uint8
    packet_size_profile_multiset_first_appear = 0x0;
int
	pp_port, core_index;
SOC_SAND_SUCCESS_FAILURE
    packet_size_profile_multiset_success = SOC_SAND_SUCCESS;
JER_PACKET_SIZE_PROFILES_TBL_DATA
    profile_tbl_data = {0};

	SOCDNX_INIT_FUNC_DEFS;

	/*Init packet size profiles 
	  packet size would be:
	  packet_size = dram_packet_size + header_delta  + ipg_compensaton - per_port_compensation
	*/

	/* set multiset member 0 as the default profile which holds the state of the IPG compensation.
	   Only 7 packet size profiles are available per core*/
	res = jer_pp_metering_packet_size_profile_key_get(
		unit,
		profile_tbl_data,
		&packet_size_profile_multiset_key
		);
	SOCDNX_IF_ERR_EXIT(res);

	
	ARAD_PP_MTR_CORES_ITER(core_index){
		/* make sure the default profile won't be deleted by adding dummy reference to it*/
		res = arad_sw_db_multiset_add(
			unit,
			core_index,
			JER_PP_SW_DB_MULTI_SET_POLICER_SIZE_PROFILE,
			&packet_size_profile_multiset_key,
			&packet_size_profile_multiset_ndx,
			&packet_size_profile_multiset_first_appear,
			&packet_size_profile_multiset_success
			);
		SOCDNX_SAND_IF_ERR_EXIT(res);

		/* All pp-ports are mapped to the default profile on init, increase refcount accordingly*/
		for (pp_port = 0; pp_port < JER_PP_MTR_IN_PP_PORT_NOF; pp_port++) {
			res = arad_sw_db_multiset_add_by_index(
				unit,
				core_index,
				JER_PP_SW_DB_MULTI_SET_POLICER_SIZE_PROFILE,
				&packet_size_profile_multiset_key,
				packet_size_profile_multiset_ndx,
				&packet_size_profile_multiset_first_appear,
				&packet_size_profile_multiset_success
				);
			SOCDNX_SAND_IF_ERR_EXIT(res);
		}
	}

exit:
  SOCDNX_FUNC_RETURN;
}

uint32 
  jer_pp_metering_in_pp_port_map_and_mtr_prf_map_set(
	  int                     unit, 
	  int                     core_id, 
	  int                     meter_group, 
	  uint32                  map_to_3_prf, 
	  uint32                  pp_port_prf, 
	  int                     packet_size
  )
{
uint32
	line_number,
	cell_number,
	table_line,
	fldval,
	rv;
soc_mem_t
	mem[2][2];

soc_field_t
	field[2][4] = {{MCDA_IN_PP_PORT_MAP_AND_MTR_PRF_MAP_0f, 
		            MCDA_IN_PP_PORT_MAP_AND_MTR_PRF_MAP_1f, 
 		            MCDA_IN_PP_PORT_MAP_AND_MTR_PRF_MAP_2f,
		            MCDA_IN_PP_PORT_MAP_AND_MTR_PRF_MAP_3f},
				   {MCDB_IN_PP_PORT_MAP_AND_MTR_PRF_MAP_0f, 
		            MCDB_IN_PP_PORT_MAP_AND_MTR_PRF_MAP_1f, 
 		            MCDB_IN_PP_PORT_MAP_AND_MTR_PRF_MAP_2f,
		            MCDB_IN_PP_PORT_MAP_AND_MTR_PRF_MAP_3f}};

	SOCDNX_INIT_FUNC_DEFS;

	mem[0][0] = JER_MRPS_MEM_FORMAT_BY_CHIP(unit,MCDA_IN_PP_PORT_MAP_AND_MTR_PRF_MAP);
	mem[0][1] = JER_MRPS_MEM_FORMAT_BY_CHIP(unit,MCDB_IN_PP_PORT_MAP_AND_MTR_PRF_MAP);
	mem[1][0] = JER_MRPS_EM_MEM_FORMAT_BY_CHIP(unit,MCDA_IN_PP_PORT_MAP_AND_MTR_PRF_MAP);
	mem[1][1] = JER_MRPS_EM_MEM_FORMAT_BY_CHIP(unit,MCDB_IN_PP_PORT_MAP_AND_MTR_PRF_MAP);

	line_number = JER_PP_MTR_IN_PP_PORT_AND_MTR_PRF_TO_TABLE_ROW(pp_port_prf, map_to_3_prf);
	cell_number = JER_PP_MTR_MAP_TO_3_PRF_TO_TABLE_CELL(map_to_3_prf);

	rv = soc_mem_read(unit, mem[0][meter_group], ARAD_PP_MTR_MEM_BLOCK(unit, core_id), line_number, &table_line);
	SOCDNX_IF_ERR_EXIT(rv);

	fldval = JER_PP_MTR_PACKET_SIZE_TO_2S_COMPLIMENT(packet_size);

	soc_mem_field_set(unit, mem[0][meter_group], &table_line, field[meter_group][cell_number], &fldval);

	/*Write both to MRPS and MRPS_EM since it shares configuration, same memory structure*/
	rv = soc_mem_write(unit, mem[0][meter_group], ARAD_PP_MTR_MEM_BLOCK(unit, core_id), line_number, &table_line);
	SOCDNX_IF_ERR_EXIT(rv);

	rv = soc_mem_write(unit, mem[1][meter_group], ARAD_PP_ETH_MTR_MEM_BLOCK(unit, core_id), line_number, &table_line);
	SOCDNX_IF_ERR_EXIT(rv);

exit:
	SOCDNX_FUNC_RETURN;
}

uint32 
  jer_pp_metering_in_pp_port_map_and_mtr_prf_map_get(
	  int                     unit, 
	  int                     core_id, 
	  int                     meter_group, 
	  uint32                  map_to_3_prf, 
	  uint32                  pp_port_prf, 
	  int                     *packet_size
  )
{
uint32
    line_number,
	cell_number,
	table_line,
	fldval,
    rv;
soc_mem_t
	mem[2];

soc_field_t
	field[2][4] = {{MCDA_IN_PP_PORT_MAP_AND_MTR_PRF_MAP_0f, 
		            MCDA_IN_PP_PORT_MAP_AND_MTR_PRF_MAP_1f, 
 		            MCDA_IN_PP_PORT_MAP_AND_MTR_PRF_MAP_2f,
		            MCDA_IN_PP_PORT_MAP_AND_MTR_PRF_MAP_3f},
				   {MCDB_IN_PP_PORT_MAP_AND_MTR_PRF_MAP_0f, 
		            MCDB_IN_PP_PORT_MAP_AND_MTR_PRF_MAP_1f, 
 		            MCDB_IN_PP_PORT_MAP_AND_MTR_PRF_MAP_2f,
		            MCDB_IN_PP_PORT_MAP_AND_MTR_PRF_MAP_3f}};

	SOCDNX_INIT_FUNC_DEFS;
	SOCDNX_NULL_CHECK(packet_size);

	mem[0] = JER_MRPS_MEM_FORMAT_BY_CHIP(unit,MCDA_IN_PP_PORT_MAP_AND_MTR_PRF_MAP);
	mem[1] = JER_MRPS_MEM_FORMAT_BY_CHIP(unit,MCDB_IN_PP_PORT_MAP_AND_MTR_PRF_MAP);

	*packet_size = 0;

	line_number = JER_PP_MTR_IN_PP_PORT_AND_MTR_PRF_TO_TABLE_ROW(pp_port_prf, map_to_3_prf);
	cell_number = JER_PP_MTR_MAP_TO_3_PRF_TO_TABLE_CELL(map_to_3_prf);

	rv = soc_mem_read(unit, mem[meter_group], ARAD_PP_MTR_MEM_BLOCK(unit, core_id), line_number, &table_line);
	SOCDNX_IF_ERR_EXIT(rv);

	soc_mem_field_get(unit, mem[meter_group], &table_line, field[meter_group][cell_number], &fldval);

	*packet_size = JER_PP_MTR_2S_COMPLIMENT_TO_PACKET_SIZE(fldval);

exit:
	SOCDNX_FUNC_RETURN;
}

uint32 
  jer_pp_metering_in_pp_port_map_get(
	  int           unit, 
	  int           core_id, 
	  int           meter_group, 
	  uint32        pp_port,
	  uint32        *pp_port_prf)
{
uint32
    line_number, 
	cell_number,
	table_line,
	rv;
soc_mem_t
	mem[2];

soc_field_t
	port_profile_field[2][4] = {{MCDA_IN_PP_PORT_MAP_0f, 
		                         MCDA_IN_PP_PORT_MAP_1f, 
								 MCDA_IN_PP_PORT_MAP_2f, 
		                         MCDA_IN_PP_PORT_MAP_3f},
                                {MCDB_IN_PP_PORT_MAP_0f, 
		                         MCDB_IN_PP_PORT_MAP_1f, 
								 MCDB_IN_PP_PORT_MAP_2f, 
		                         MCDB_IN_PP_PORT_MAP_3f}};

	SOCDNX_INIT_FUNC_DEFS;
	SOCDNX_NULL_CHECK(pp_port_prf);

	mem[0] = JER_MRPS_MEM_FORMAT_BY_CHIP(unit,MCDA_IN_PP_PORT_MAP);
	mem[1] = JER_MRPS_MEM_FORMAT_BY_CHIP(unit,MCDB_IN_PP_PORT_MAP);

	/*First, get the number of the PP Profile*/
	line_number = pp_port / 4;
	cell_number = pp_port % 4;
	
	rv = soc_mem_read(unit, mem[meter_group], ARAD_PP_MTR_MEM_BLOCK(unit, core_id), line_number, &table_line);
	SOCDNX_IF_ERR_EXIT(rv);

	*pp_port_prf = soc_mem_field32_get(unit, mem[meter_group], &table_line, port_profile_field[meter_group][cell_number]);

exit:
	SOCDNX_FUNC_RETURN;
}

uint32 
  jer_pp_metering_in_pp_port_map_set(
	  int           unit, 
	  int           core_id, 
	  int           meter_group, 
	  uint32        pp_port, 
	  uint32        pp_port_prf)
{
uint32
    line_number, 
	cell_number,
	table_line,
	fldval,
	rv;
soc_mem_t
	mem[2][2];

soc_field_t
	port_profile_field[2][4] = {{MCDA_IN_PP_PORT_MAP_0f, 
		                         MCDA_IN_PP_PORT_MAP_1f, 
								 MCDA_IN_PP_PORT_MAP_2f, 
		                         MCDA_IN_PP_PORT_MAP_3f},
                                {MCDB_IN_PP_PORT_MAP_0f, 
		                         MCDB_IN_PP_PORT_MAP_1f, 
								 MCDB_IN_PP_PORT_MAP_2f, 
		                         MCDB_IN_PP_PORT_MAP_3f}};

	SOCDNX_INIT_FUNC_DEFS;

	mem[0][0] = JER_MRPS_MEM_FORMAT_BY_CHIP(unit,MCDA_IN_PP_PORT_MAP);
	mem[0][1] = JER_MRPS_MEM_FORMAT_BY_CHIP(unit,MCDB_IN_PP_PORT_MAP);
	mem[1][0] = JER_MRPS_EM_MEM_FORMAT_BY_CHIP(unit,MCDA_IN_PP_PORT_MAP);
	mem[1][1] = JER_MRPS_EM_MEM_FORMAT_BY_CHIP(unit,MCDB_IN_PP_PORT_MAP);

	/*First, get the number of the PP Profile*/
	line_number = pp_port / 4;
	cell_number = pp_port % 4;
	
	rv = soc_mem_read(unit, mem[0][meter_group], ARAD_PP_MTR_MEM_BLOCK(unit, core_id), line_number, &table_line);
	SOCDNX_IF_ERR_EXIT(rv);

	fldval = pp_port_prf;
	soc_mem_field_set(unit, mem[0][meter_group], &table_line, port_profile_field[meter_group][cell_number], &fldval);

	/*Write both to MRPS and MRPS_EM since it shares configuration, same memory structure*/
	rv = soc_mem_write(unit, mem[0][meter_group], ARAD_PP_MTR_MEM_BLOCK(unit, core_id), line_number, &table_line);
	SOCDNX_IF_ERR_EXIT(rv);

	rv = soc_mem_write(unit, mem[1][meter_group], ARAD_PP_ETH_MTR_MEM_BLOCK(unit, core_id), line_number, &table_line);
	SOCDNX_IF_ERR_EXIT(rv);

exit:
	SOCDNX_FUNC_RETURN;
}

uint32
  jer_pp_metering_packet_size_profile_key_get(
    int                                unit,
    JER_PACKET_SIZE_PROFILES_TBL_DATA  profile_tbl_data,
	uint32                             *packet_size_profile_multiset_key
  )
{
	SOCDNX_INIT_FUNC_DEFS;
	SOCDNX_NULL_CHECK(packet_size_profile_multiset_key);

	*packet_size_profile_multiset_key = 0;
	*packet_size_profile_multiset_key |= SOC_SAND_SET_BITS_RANGE(profile_tbl_data.in_pp_port_size_delta,7,0);

exit:
	SOCDNX_FUNC_RETURN;
}

uint32
jer_pp_mtr_policer_ipg_compensation_set(
    int                         unit,
	uint8						ipg_compensation_enabled
)
{
uint32
	rv,
	meter_group, 
	pp_port_prf, 
	core_id;
uint8
	is_enabled;
int
	packet_size;

	SOCDNX_INIT_FUNC_DEFS;

	/*Check the packet size of the defualt profile*/
	rv = jer_pp_metering_in_pp_port_map_and_mtr_prf_map_get(unit, 0/*core*/, 0/*group*/, 0/*map_to_3_prf*/, 0/*pp_port_prf*/, &packet_size);
	SOCDNX_IF_ERR_EXIT(rv);

	is_enabled = ((packet_size == SOC_PPC_MTR_IPG_COMPENSATION_ENABLED_SIZE) ? TRUE : FALSE);

	/*No change is needed*/
	if (is_enabled == ipg_compensation_enabled) {
		SOC_EXIT;
	}

	ARAD_PP_MTR_CORES_ITER(core_id){
		/*Iterate over MCDA and MCDB*/
		for (meter_group = 0; meter_group < 2; meter_group++) {

			/*Change the default profile first*/
			rv = jer_pp_metering_in_pp_port_map_and_mtr_prf_map_set(unit, core_id, meter_group, 0/*map_to_3_prf*/, 0/*pp_port_prf*/, 
														ipg_compensation_enabled ? SOC_PPC_MTR_IPG_COMPENSATION_ENABLED_SIZE : SOC_PPC_MTR_IPG_COMPENSATION_DISABLED_SIZE);
			SOCDNX_IF_ERR_EXIT(rv);

			/*Iterate through all other profiles, add/reduce IPG size to each*/
			for (pp_port_prf = 1; pp_port_prf < JER_PP_MTR_IN_PP_PORT_MAP_NOF_PROFILES; pp_port_prf++) {

				rv = jer_pp_metering_in_pp_port_map_and_mtr_prf_map_get(unit, core_id, meter_group, 0/*map_to_3_prf*/, pp_port_prf, &packet_size);
				SOCDNX_IF_ERR_EXIT(rv);
					
				/*Calclulate packet size after change, make sure doesn't exceed the allowed value*/
				if (ipg_compensation_enabled) {
					packet_size += SOC_PPC_MTR_IPG_COMPENSATION_ENABLED_SIZE;

					/*Shouldn't happen since we limited the maximum in the first place*/
					if (packet_size > JER_PP_MTR_2S_COMPLIMENT_MAX_VALUE) {
						packet_size = JER_PP_MTR_2S_COMPLIMENT_MAX_VALUE;
					}
				} else{
					packet_size -= SOC_PPC_MTR_IPG_COMPENSATION_ENABLED_SIZE;

					/*Shouldn't happen since we limited the minimum in the first place*/
					if (packet_size < JER_PP_MTR_2S_COMPLIMENT_MIN_VALUE) {
						packet_size = JER_PP_MTR_2S_COMPLIMENT_MIN_VALUE;
					}
				}

				rv = jer_pp_metering_in_pp_port_map_and_mtr_prf_map_set(unit, core_id, meter_group, 0/*map_to_3_prf*/, pp_port_prf, packet_size);
				SOCDNX_IF_ERR_EXIT(rv);
			}
		}
	}

exit:
	SOCDNX_FUNC_RETURN;
}

uint32
jer_pp_mtr_policer_ipg_compensation_get(
    int                         unit,
	uint8						*ipg_compensation_enabled
)
{
uint32 
	rv;
int
	packet_size;

	SOCDNX_INIT_FUNC_DEFS;
	SOCDNX_NULL_CHECK(ipg_compensation_enabled);

	rv = jer_pp_metering_in_pp_port_map_and_mtr_prf_map_get(unit, 0/*core*/, 0/*group*/, 0/*map_to_3_prf*/, 0/*pp_port_prf*/, &packet_size);
	SOCDNX_IF_ERR_EXIT(rv);

	*ipg_compensation_enabled = ((packet_size == SOC_PPC_MTR_IPG_COMPENSATION_ENABLED_SIZE) ? TRUE : FALSE);

exit:
	SOCDNX_FUNC_RETURN;
}

uint32 
jer_pp_mtr_policer_hdr_compensation_verify(
	int             unit, 
	int             compensation_size
)
{
	SOCDNX_INIT_FUNC_DEFS;

	if ((compensation_size + SOC_PPC_MTR_IPG_COMPENSATION_ENABLED_SIZE > JER_PP_MTR_2S_COMPLIMENT_MAX_VALUE) ||
		(compensation_size < JER_PP_MTR_2S_COMPLIMENT_MIN_VALUE)){
		SOCDNX_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_SOCDNX_MSG("Requested compensation size is outside of allowed range [-108,+127].")));
	}

exit:
	SOCDNX_FUNC_RETURN;
}

uint32
jer_pp_mtr_policer_hdr_compensation_set(
    int                         unit,
	int                         core_id,
	uint32                      pp_port,
	int                         compensation_size
)
{
uint32
	rv,
	meter_group,
	previous_profile,
	packet_size_profile_multiset_key = 0,
	packet_size_profile_multiset_ndx = 0;
uint8
    packet_size_profile_multiset_last_appear = 0,
	packet_size_profile_multiset_first_appear = 0;
int
	previous_profile_compensation, 
	default_profile_compensation;
JER_PACKET_SIZE_PROFILES_TBL_DATA
    profile_tbl_data = {0};
SOC_SAND_SUCCESS_FAILURE
    packet_size_profile_multiset_success = SOC_SAND_SUCCESS;

	SOCDNX_INIT_FUNC_DEFS;

	/*Per port compensation is reduced from the packet size*/
	compensation_size *= -1;

	rv = jer_pp_mtr_policer_hdr_compensation_verify(unit, compensation_size);
	SOCDNX_IF_ERR_EXIT(rv);

	/*First, get the default packet size*/
	rv = jer_pp_metering_in_pp_port_map_and_mtr_prf_map_get(
		unit, 
		core_id, 
		0/*meter_group*/, 
		0 /*map_to_3_prf*/, 
		0 /*pp_port_prf*/, 
		&default_profile_compensation
		);
	SOCDNX_IF_ERR_EXIT(rv);

	/*Find the prvious profile for this port (shared for both groups)*/
	rv = jer_pp_metering_in_pp_port_map_get(
		unit, 
		core_id, 
		0/*meter group*/, 
		pp_port, 
		&previous_profile);
	SOCDNX_IF_ERR_EXIT(rv);

	/*Get compensation size for this profile*/
	rv = jer_pp_metering_in_pp_port_map_and_mtr_prf_map_get(
		unit, 
		core_id, 
		0/*meter_group*/, 
		0 /*map_to_3_prf*/, 
		previous_profile, 
		&previous_profile_compensation
		);
	SOCDNX_IF_ERR_EXIT(rv);

	/*Nothing to do*/
	if (compensation_size == previous_profile_compensation - default_profile_compensation) {
		SOC_EXIT;
	}

	/*Decrement reference count of previous profile*/
	rv = arad_sw_db_multiset_remove_by_index(
		unit,
		core_id,
        JER_PP_SW_DB_MULTI_SET_POLICER_SIZE_PROFILE,
        previous_profile,
        &packet_size_profile_multiset_last_appear
		);
	SOCDNX_SAND_IF_ERR_EXIT(rv);

	/*Calculate new multiset key*/
	profile_tbl_data.in_pp_port_size_delta = compensation_size;
	rv  = jer_pp_metering_packet_size_profile_key_get(unit, profile_tbl_data, &packet_size_profile_multiset_key);
	SOCDNX_IF_ERR_EXIT(rv);

	rv = arad_sw_db_multiset_add(
		unit,
		core_id,
		JER_PP_SW_DB_MULTI_SET_POLICER_SIZE_PROFILE,
		&packet_size_profile_multiset_key,
        &packet_size_profile_multiset_ndx,
        &packet_size_profile_multiset_first_appear,
        &packet_size_profile_multiset_success
		);
	SOCDNX_SAND_IF_ERR_EXIT(rv);

	/*Addition failed, revert*/
	if (packet_size_profile_multiset_success != SOC_SAND_SUCCESS) {

		/*Build key of previous profile*/
		profile_tbl_data.in_pp_port_size_delta = previous_profile_compensation - default_profile_compensation;
		rv  = jer_pp_metering_packet_size_profile_key_get(unit, profile_tbl_data, &packet_size_profile_multiset_key);
		SOCDNX_IF_ERR_EXIT(rv);

		rv = arad_sw_db_multiset_add_by_index(
			unit, 
			core_id, 
			JER_PP_SW_DB_MULTI_SET_POLICER_SIZE_PROFILE, 
			&packet_size_profile_multiset_key,
			previous_profile,
			&packet_size_profile_multiset_first_appear,
			&packet_size_profile_multiset_success 
			);
		SOCDNX_SAND_IF_ERR_EXIT(rv);
		
		if (packet_size_profile_multiset_success == SOC_SAND_SUCCESS){
			SOCDNX_EXIT_WITH_ERR(SOC_E_RESOURCE, (_BSL_SOCDNX_MSG("Exceeded the maximum number of metering compensation profiles.\n"
																  "Reverted to previous profile.")));
		}
		else {
			SOCDNX_EXIT_WITH_ERR(SOC_E_RESOURCE, (_BSL_SOCDNX_MSG("Exceeded the maximum number of metering compensation profiles.\n"
																  "Failed to revert to previous profile.")));
		}
	}

	/*Addition succeeded, the following is duplicated to two MCD's*/
	for (meter_group = 0; meter_group < 2; meter_group++) {

		/*Add new profile if needed*/
		if (packet_size_profile_multiset_first_appear) {
			rv = jer_pp_metering_in_pp_port_map_and_mtr_prf_map_set(
				unit, 
				core_id, 
				meter_group, 
				0 /*map_to_3_prf*/, 
				packet_size_profile_multiset_ndx, 
				compensation_size + default_profile_compensation/*actual size written is always added with default size*/
				);
			SOCDNX_IF_ERR_EXIT(rv);
		}

		/*Point the port to the new profile */
		rv = jer_pp_metering_in_pp_port_map_set(
			unit, 
			core_id, 
			meter_group, 
			pp_port, 
			packet_size_profile_multiset_ndx
			);
		SOCDNX_IF_ERR_EXIT(rv);

		/*Remove old profile by resetting it to default size only if: 
		  a. it's not used anymore
		  b. new_id!=old_id */
		if (packet_size_profile_multiset_last_appear && (previous_profile != packet_size_profile_multiset_ndx)) {
			rv = jer_pp_metering_in_pp_port_map_and_mtr_prf_map_set(
				unit, 
				core_id, 
				meter_group, 
				0 /*map_to_3_prf*/, 
				previous_profile, 
				default_profile_compensation
				);
			SOCDNX_IF_ERR_EXIT(rv);
		}
	}

exit:
	SOCDNX_FUNC_RETURN;
}

uint32
jer_pp_mtr_policer_hdr_compensation_get(
    int                         unit,
	int                         core_id,
	uint32                      pp_port,
	int                         *compensation_size
)
{
uint32
	rv, 
	pp_port_prf;
int
	port_compensation,
	default_compensation;

	SOCDNX_INIT_FUNC_DEFS;
	SOCDNX_NULL_CHECK(compensation_size);

	/*First, check what is the default compensation size, is shared for both cores and meter groups*/
	rv = jer_pp_metering_in_pp_port_map_and_mtr_prf_map_get(unit, 0/*core*/, 0/* meter group*/, 0/*map_to_3_prf*/, 0/*pp_port_prf*/, &default_compensation);
	SOCDNX_IF_ERR_EXIT(rv);

	/*Find pp_port_prf for this port (shared for both groups)*/
	rv = jer_pp_metering_in_pp_port_map_get(unit, core_id, 0/*meter group*/, pp_port, &pp_port_prf);
	SOCDNX_IF_ERR_EXIT(rv);

	/*Find compensation configured for this port*/
	rv = jer_pp_metering_in_pp_port_map_and_mtr_prf_map_get(unit, core_id, 0/* meter group*/, 0/*map_to_3_prf*/, pp_port_prf, &port_compensation);
	SOCDNX_IF_ERR_EXIT(rv);

	/*Default compensation is always added to the port compensation, per port compensation is negative*/
	*compensation_size = -1 * (port_compensation - default_compensation);

exit:
	SOCDNX_FUNC_RETURN;
}

/* } */
#include <soc/dpp/SAND/Utils/sand_footer.h>

#endif /*BCM_88675_A0*/
