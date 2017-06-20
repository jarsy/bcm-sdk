#include <soc/mcm/memregs.h>
#if defined(BCM_88650_A0)
/* $Id: arad_pp_kbp_entry_mgmt.c,
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 * $
*/
#ifdef _ERR_MSG_MODULE_NAME
  #error "_ERR_MSG_MODULE_NAME redefined"
#endif
#define _ERR_MSG_MODULE_NAME BSL_SOC_FORWARD

/*************
 * INCLUDES  *
 *************/
/* { */
#include <shared/bsl.h>
#include <shared/swstate/access/sw_state_access.h>
#include <soc/dpp/drv.h>

#include <soc/dpp/SAND/Utils/sand_header.h>
#include <soc/dcmn/error.h>
#include <soc/dpp/SAND/Management/sand_general_macros.h>
#include <soc/dpp/SAND/Management/sand_error_code.h>
#include <soc/dpp/SAND/Utils/sand_os_interface.h>
#include <soc/dpp/SAND/Utils/sand_multi_set.h>

#include <soc/dpp/ARAD/arad_sw_db.h>
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_framework.h>
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_frwrd_ipv6.h>
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_frwrd_ip_tcam.h>
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_kbp_entry_mgmt.h>
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_general.h>
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_flp_init.h>
#include <soc/dpp/ARAD/arad_tbl_access.h>
#include <soc/dpp/ARAD/arad_tcam_mgmt.h>
#include <soc/dpp/ARAD/arad_sw_db.h>
#include <soc/dpp/PPD/ppd_api_trap_mgmt.h>
#include <soc/dpp/SAND/Management/sand_low_level.h>

#include <soc/dcmn/dcmn_crash_recovery.h>
/* } */


#ifdef CRASH_RECOVERY_SUPPORT
extern soc_dcmn_cr_t *dcmn_cr_info[SOC_MAX_NUM_DEVICES];
#endif

#if defined(INCLUDE_KBP) && !defined(BCM_88030)

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

/* } */
/*************
 * GLOBALS   *
 *************/
/* { */

/* 
 * Conversion functions from application-dependent 
 * structure (already transformed in a LEM access key) 
 * to the regular TCAM entry handling functions: 
 * add, remove, get and get-block 
 *  
 * In ARAD_PP_LEM_ACCESS_KEY, only nof_params and param[] 
 * are used. 
 */ 
    ARAD_PP_TCAM_KBP_TABLE_ATTRIBUTES
        Arad_pp_tcam_kbp_table_attributes[ARAD_KBP_FRWRD_IP_NOF_TABLES] = {
            /* LSB-#bits-param0,   param1,       param2,      param3,        param4,     logical_entry_size_in_bytes */
            {       {{0, 32},     {32,12},       {0, 0},       {0, 0},       {0, 0},},        6}, /* IPV4_UC_RPF_0 */
            {       {{0, 32},     {32,12},       {0, 0},       {0, 0},       {0, 0},},        6}, /* IPV4_UC_RPF_1 */
            {       {{0, 16},     {16,32},      {48,32},     {80, 16},       {0, 0},},        12}, /* IPV4_MC */
            {       {{0,128},    {128,12},       {0, 0},       {0, 0},       {0, 0},},        18}, /* IPV6_UC_RPF_0 */
            {       {{0,128},    {128,12},       {0, 0},       {0, 0},       {0, 0},},        18}, /* IPV6_UC_RPF_1 */
            {       {{0,16},     {16,128},    {144,128},    {272, 16},       {0, 0},},        36}, /* IPV6_MC */
            {       {{0,20},      {20, 3},      {23, 8},      {31,12},       {0, 0},},        6}, /* LSR */
            {       {{0,16},       {0, 0},       {0, 0},       {0, 0},       {0, 0},},        2}, /* TRILL_UC */
            {       {{0,16},      {16,15},      {31, 1},       {0, 0},       {0, 0},},        4}, /* TRILL_MC */
            {       {{0,32},      {32,12},      {44, 24},     {0, 0},        {0, 0},},        9}, /* LSR_IP_SHARED */
            {       {{0,32},      {32,12},      {0, 0},       {0, 0},        {0, 0},},        6}, /* LSR_IP_SHARED_FOR_IP */
            {       {{0,24},      {0,0},        {0, 0},       {0, 0},        {0, 0},},        3}, /* LSR_IP_SHARED_FOR_LSR */
        };

/* } */
/*************
 * FUNCTIONS *
 *************/
/* { */

/* 
 * KBP access level functions: 
 * - once the buffer of the content and the actions is obtained, 
 * insertion / removal / get / get-block at low level 
 */

/* Build the buffer for payload - then encode according to KBP expectations */
 uint32
  arad_pp_kbp_route_payload_encode(
    SOC_SAND_IN  int                     unit,
    SOC_SAND_IN  ARAD_KBP_FRWRD_IP_TBL_ID   frwrd_table_id,
    SOC_SAND_IN  ARAD_PP_LEM_ACCESS_PAYLOAD *payload,
    SOC_SAND_OUT uint8                      *asso_data
  )
{
    uint32
        res,
        asso_data_buffer[SOC_DPP_TCAM_ACTION_ELK_KBP_MAX_LEN_LONGS];
    uint32
      table_size_in_bytes,
        table_payload_in_bytes;

    SOC_SAND_INIT_ERROR_DEFINITIONS(0);

    SOC_SAND_CHECK_NULL_INPUT(payload);
    SOC_SAND_CHECK_NULL_INPUT(asso_data);

    sal_memset(asso_data, 0x0, sizeof(uint8) * SOC_DPP_TCAM_ACTION_ELK_KBP_GET_LEN_BYTES(unit));
    sal_memset(asso_data_buffer, 0x0, sizeof(uint32) * SOC_DPP_TCAM_ACTION_ELK_KBP_MAX_LEN_LONGS);

    /* Build the buffer */
    /* Encode structure into data buffer */
    res = arad_pp_lem_access_payload_build(
            unit,
            payload,
            asso_data_buffer
          );
    SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);

    /*
     * Transform according to KBP expectations   
     */
    res = arad_kbp_table_size_get(unit, frwrd_table_id, &table_size_in_bytes, &table_payload_in_bytes); 
    SOC_SAND_CHECK_FUNC_RESULT(res,  71, exit);
    res = arad_pp_tcam_route_kbp_payload_buffer_encode(
            unit,
            table_payload_in_bytes,
            asso_data_buffer,
            asso_data
          );
    SOC_SAND_CHECK_FUNC_RESULT(res, 30, exit);

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR( "error in arad_pp_frwrd_ip_tcam_route_kbp_payload_encode()",0,0);
}

STATIC
uint32
  arad_pp_tcam_route_kbp_hw_get_block_unsafe(
    SOC_SAND_IN  int                                   unit,
    SOC_SAND_IN  ARAD_KBP_FRWRD_IP_TBL_ID                frwrd_table_id,
    SOC_SAND_INOUT  SOC_PPC_IP_ROUTING_TABLE_RANGE       *block_range_key,
    SOC_SAND_INOUT  ARAD_PP_IP_TCAM_ENTRY_KEY            *keys,
    SOC_SAND_INOUT  ARAD_TCAM_ACTION                     *actions,
    SOC_SAND_INOUT  uint32                  			 *nof_entries
  )
{
    uint32 res;
    SOC_SAND_HASH_TABLE_ITER entry_index;
    struct kbp_db 			*db_p = NULL;
    struct kbp_entry_iter 	*iter_p = NULL;
    struct kbp_entry 		*db_entry_p = NULL;
	struct kbp_ad_db 		*ad_db_p = NULL;
	struct kbp_entry_info 	db_entry_info;

    SOC_SAND_INIT_ERROR_DEFINITIONS(0);

    SOC_SAND_CHECK_NULL_INPUT(block_range_key);
    SOC_SAND_CHECK_NULL_INPUT(keys);
    SOC_SAND_CHECK_NULL_INPUT(actions);

	*nof_entries = 0;
	SOC_SAND_HASH_TABLE_ITER_SET_BEGIN(&entry_index);
	sal_memset(&db_entry_info,0,sizeof(struct kbp_entry_info));

	/*Get kbp_db*/
    res =  arad_kbp_alg_kbp_db_get(
            unit,
            frwrd_table_id, 
            &db_p
        );
    SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);
    SOC_SAND_CHECK_NULL_PTR(db_p, 20, exit);

	/*Get kbp_ad*/
	res = arad_kbp_alg_kbp_ad_db_get(
			unit, 
			frwrd_table_id, 
			&ad_db_p
		);
	SOC_SAND_CHECK_FUNC_RESULT(res, 30, exit);
    SOC_SAND_CHECK_NULL_PTR(ad_db_p, 40, exit);

	/*Get iterator for first entry in kbp table*/
    res = kbp_db_entry_iter_init(db_p, &iter_p);
    if(ARAD_KBP_TO_SOC_RESULT(res) != SOC_SAND_OK) {
        LOG_ERROR(BSL_LS_SOC_TCAM,
                  (BSL_META_U(unit, "Error in %s(): kbp_db_entry_iter_init failed: %s\n"),
                  FUNCTION_NAME(), kbp_get_status_string(res)));
    }
    ARAD_KBP_CHECK_FUNC_RESULT(res, 10, exit);
    SOC_SAND_CHECK_NULL_PTR(iter_p, 20, exit);

    res = kbp_db_entry_iter_next(db_p, iter_p, &db_entry_p);
    if(ARAD_KBP_TO_SOC_RESULT(res) != SOC_SAND_OK) {
        LOG_ERROR(BSL_LS_SOC_TCAM,
                  (BSL_META_U(unit, "Error in %s(): kbp_db_entry_iter_next failed: %s\n"),
                  FUNCTION_NAME(), kbp_get_status_string(res)));
    }
    ARAD_KBP_CHECK_FUNC_RESULT(res, 10, exit);

    /* Look for the first entry in range requested*/
    while ( db_entry_p && (entry_index < block_range_key->start.payload.arr[0])) {
        ++entry_index;
        res = kbp_db_entry_iter_next(db_p, iter_p, &db_entry_p);
        if(ARAD_KBP_TO_SOC_RESULT(res) != SOC_SAND_OK) {
            LOG_ERROR(BSL_LS_SOC_TCAM,
                      (BSL_META_U(unit, "Error in %s(): kbp_db_entry_iter_next failed: %s\n"),
    				  FUNCTION_NAME(), kbp_get_status_string(res)));
        }
        ARAD_KBP_CHECK_FUNC_RESULT(res, 10, exit);
    }

    /* Copy all entries in range*/
    while ( db_entry_p && (*nof_entries < block_range_key->entries_to_act)) {
		/* Retrive entry info*/
        res = kbp_entry_get_info(db_p, db_entry_p, &db_entry_info);
        if(ARAD_KBP_TO_SOC_RESULT(res) != SOC_SAND_OK) {
            LOG_ERROR(BSL_LS_SOC_TCAM,
                      (BSL_META_U(unit, "Error in %s(): kbp_entry_get_info failed: %s\n"),
    				  FUNCTION_NAME(), kbp_get_status_string(res)));
        }
        ARAD_KBP_CHECK_FUNC_RESULT(res, 40, exit);

        sal_memcpy(keys[*nof_entries].key.elk_fwd.m_data, db_entry_info.data, sizeof(uint8) * (db_entry_info.width_8));
        sal_memcpy(keys[*nof_entries].key.elk_fwd.m_mask, db_entry_info.mask, sizeof(uint8) * (db_entry_info.width_8));

		/* Retrive ad entry info*/
        SOC_SAND_CHECK_NULL_PTR(db_entry_info.ad_handle, 40, exit);
        sal_memset(actions[*nof_entries].elk_ad_value, 0x0, sizeof(uint8) * SOC_DPP_TCAM_ACTION_ELK_KBP_GET_LEN_BYTES(unit));

        res = kbp_ad_db_get(ad_db_p, db_entry_info.ad_handle, actions[*nof_entries].elk_ad_value);
        if(ARAD_KBP_TO_SOC_RESULT(res) != SOC_SAND_OK) {
            LOG_ERROR(BSL_LS_SOC_TCAM,
                      (BSL_META_U(unit, "Error in %s(): kbp_ad_db_get failed: %s\n"),
    				  FUNCTION_NAME(), kbp_get_status_string(res)));
        }
        ARAD_KBP_CHECK_FUNC_RESULT(res, 70, exit);
        SOC_SAND_CHECK_NULL_PTR(actions[*nof_entries].elk_ad_value, 80, exit);

		++(*nof_entries);
		sal_memset(&db_entry_info,0,sizeof(struct kbp_entry_info));
        res = kbp_db_entry_iter_next(db_p, iter_p, &db_entry_p);
        if(ARAD_KBP_TO_SOC_RESULT(res) != SOC_SAND_OK) {
            LOG_ERROR(BSL_LS_SOC_TCAM,
                      (BSL_META_U(unit, "Error in %s(): kbp_db_entry_iter_next failed: %s\n"),
    				  FUNCTION_NAME(), kbp_get_status_string(res)));
        }
        ARAD_KBP_CHECK_FUNC_RESULT(res, 10, exit);		
    }
	
	if( !db_entry_p)
	{
		block_range_key->start.payload.arr[0] = SOC_SAND_UINT_MAX;
		block_range_key->start.payload.arr[1] = SOC_SAND_UINT_MAX;
	}
	else
	{
		block_range_key->start.payload.arr[0] = entry_index + *nof_entries;
	}

	res = kbp_db_entry_iter_destroy(db_p, iter_p);
    if(ARAD_KBP_TO_SOC_RESULT(res) != SOC_SAND_OK) {
        LOG_ERROR(BSL_LS_SOC_TCAM,
                  (BSL_META_U(unit, "Error in %s(): kbp_db_entry_iter_destroy failed: %s\n"),
                  FUNCTION_NAME(), kbp_get_status_string(res)));
    }
    ARAD_KBP_CHECK_FUNC_RESULT(res, 10, exit);
	iter_p = NULL;
    
exit:
	if (iter_p != NULL) {
		res = kbp_db_entry_iter_destroy(db_p, iter_p);
        if(ARAD_KBP_TO_SOC_RESULT(res) != SOC_SAND_OK) {
            LOG_ERROR(BSL_LS_SOC_TCAM,
                      (BSL_META_U(unit, "Error in %s(): kbp_db_entry_iter_init failed: %s\n"),
    				  FUNCTION_NAME(), kbp_get_status_string(res)));
        }
	}
	SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_frwrd_ip_tcam_route_kbp_get_hw_block_unsafe()", 0, 0); 
}

uint32
  arad_pp_kbp_do_search(
	int     unit,
	uint32  ltr_idx,
	uint8   *master_key
  )
{
    uint32 res;
    ARAD_KBP_FRWRD_IP_LTR   ltr_id=0;

	struct kbp_device *device_p		= NULL;
    struct kbp_instruction *inst_p 	= NULL;
    struct kbp_search_result search_rslt;

    SOC_SAND_INIT_ERROR_DEFINITIONS(0);

    sal_memset(&search_rslt,0,sizeof(struct kbp_search_result));

    res = arad_kbp_get_device_pointer(unit, &device_p);
    SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);
    SOC_SAND_CHECK_NULL_PTR(device_p, 20, exit);
    
	res = sw_state_access[unit].dpp.soc.arad.tm.kbp_info.Arad_kbp_ltr_config.ltr_id.get(unit, ltr_idx, &ltr_id);
	SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);
	
    res = arad_kbp_ltr_get_inst_pointer(unit, ltr_idx, &inst_p);
    SOC_SAND_CHECK_NULL_PTR(inst_p, 30, exit);

    res = kbp_instruction_search(inst_p, master_key, 0, &search_rslt);
    if(ARAD_KBP_TO_SOC_RESULT(res) != SOC_SAND_OK) {
        LOG_ERROR(BSL_LS_SOC_TCAM,
                  (BSL_META_U(unit, "Error in %s(): kbp_db_entry_iter_init failed: %s\n"),
                  FUNCTION_NAME(), kbp_get_status_string(res)));
        SOC_SAND_SET_ERROR_CODE(SOC_SAND_GEN_ERR, 40, exit);
    }

	arad_kbp_print_search_result(unit, ltr_idx, master_key, search_rslt);
	
exit:
  SOC_SAND_EXIT_AND_SEND_ERROR( "error in arad_pp_kbp_do_search()",ltr_idx,ltr_id);
}

 uint32
  arad_pp_kbp_route_payload_decode(
    SOC_SAND_IN  int                     unit,
    SOC_SAND_IN  ARAD_KBP_FRWRD_IP_TBL_ID   frwrd_table_id,
    SOC_SAND_IN  uint32                     lookup_id,
    SOC_SAND_IN  uint8                      *asso_data,
    SOC_SAND_OUT ARAD_PP_LEM_ACCESS_PAYLOAD *payload
  )
{
    uint32
        res,
        asso_data_buffer[SOC_DPP_TCAM_ACTION_ELK_KBP_MAX_LEN_LONGS];

    SOC_SAND_INIT_ERROR_DEFINITIONS(0);

    SOC_SAND_CHECK_NULL_INPUT(payload);
    SOC_SAND_CHECK_NULL_INPUT(asso_data);

    sal_memset(asso_data_buffer, 0x0, sizeof(uint32) * SOC_DPP_TCAM_ACTION_ELK_KBP_MAX_LEN_LONGS);
    ARAD_PP_LEM_ACCESS_PAYLOAD_clear(payload);

    res = arad_pp_tcam_route_kbp_payload_buffer_decode(
             unit,
             frwrd_table_id,
             asso_data,
             asso_data_buffer
          );
    SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

    /* Build the buffer */
    if (lookup_id == 0) {
        res = arad_pp_lem_access_payload_parse(
                unit,
                asso_data_buffer,
                ARAD_PP_LEM_ACCESS_NOF_KEY_TYPES,
                payload
              );
        SOC_SAND_CHECK_FUNC_RESULT(res, 30, exit);
    }
    else {
        
    }

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR( "error in arad_pp_frwrd_ip_tcam_route_kbp_payload_decode()",0,0);
}

/* Build the buffer for data and mask - then encode according to KBP expectations */
STATIC
 uint32
  arad_pp_kbp_route_buffer_encode(
    SOC_SAND_IN  int                     unit,
    SOC_SAND_IN  ARAD_KBP_FRWRD_IP_TBL_ID   frwrd_table_id,
    SOC_SAND_IN  ARAD_PP_LEM_ACCESS_KEY     *route_key_data,
    SOC_SAND_IN  ARAD_PP_LEM_ACCESS_KEY     *route_key_mask,
    SOC_SAND_OUT uint8                        *data,
    SOC_SAND_OUT uint8                        *mask
  )
{
    uint32
        res,
        param_ndx,
        buffer_lsb,
        buffer_data[ARAD_PP_FRWRD_IP_ELK_FWD_MAX_KEY_LENGTH_IN_LONGS],
        buffer_mask[ARAD_PP_FRWRD_IP_ELK_FWD_MAX_KEY_LENGTH_IN_LONGS];

    SOC_SAND_INIT_ERROR_DEFINITIONS(0);

    SOC_SAND_CHECK_NULL_INPUT(data);
    SOC_SAND_CHECK_NULL_INPUT(mask);

    sal_memset(data, 0x0, sizeof(uint8) * ARAD_PP_FRWRD_IP_ELK_FWD_MAX_KEY_LENGTH_IN_BYTES);
    sal_memset(mask, 0x0, sizeof(uint8) * ARAD_PP_FRWRD_IP_ELK_FWD_MAX_KEY_LENGTH_IN_BYTES);
    sal_memset(buffer_data, 0x0, sizeof(uint32) * ARAD_PP_FRWRD_IP_ELK_FWD_MAX_KEY_LENGTH_IN_LONGS);
    sal_memset(buffer_mask, 0x0, sizeof(uint32) * ARAD_PP_FRWRD_IP_ELK_FWD_MAX_KEY_LENGTH_IN_LONGS);

    /* Build the buffer */
    for (param_ndx = 0; param_ndx < route_key_data->nof_params; ++param_ndx)
    {
        /* Get the LSB of each parameter according to the static table per forwarding type */
        buffer_lsb = Arad_pp_tcam_kbp_table_attributes[frwrd_table_id].lsb_nof_bits[param_ndx][0/*lsb*/];
        SHR_BITCOPY_RANGE(buffer_data, buffer_lsb, route_key_data->param[param_ndx].value, 0 /* LSB */, route_key_data->param[param_ndx].nof_bits);
        SHR_BITCOPY_RANGE(buffer_mask, buffer_lsb, route_key_mask->param[param_ndx].value, 0 /* LSB */, route_key_data->param[param_ndx].nof_bits);
    }

    res = arad_pp_tcam_route_buffer_to_kbp_buffer_encode(
            unit, 
            Arad_pp_tcam_kbp_table_attributes[frwrd_table_id].logical_entry_size_in_bytes,
            buffer_data,
            buffer_mask,
            data,
            mask
            );
    SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR( "error in arad_pp_frwrd_ip_tcam_route_kbp_buffer_encode()",0,0);
}

/* Build the buffer for data and mask - then encode according to KBP expectations */
STATIC
 uint32
  arad_pp_kbp_route_buffer_decode(
    SOC_SAND_IN  int                         unit,
    SOC_SAND_IN  ARAD_KBP_FRWRD_IP_TBL_ID       frwrd_table_id,
    SOC_SAND_IN  uint8                          *data,
    SOC_SAND_IN  uint8                          *mask,
    SOC_SAND_OUT  ARAD_PP_LEM_ACCESS_KEY        *route_key_data,
    SOC_SAND_OUT  ARAD_PP_LEM_ACCESS_KEY        *route_key_mask
  )
{
    uint32
        param_ndx,
        byte_ndx,
        byte_array_idx,
        buffer_lsb,
        res,
        buffer_data[ARAD_PP_FRWRD_IP_ELK_FWD_MAX_KEY_LENGTH_IN_LONGS],
        buffer_mask[ARAD_PP_FRWRD_IP_ELK_FWD_MAX_KEY_LENGTH_IN_LONGS],
        fld_val;

    SOC_SAND_INIT_ERROR_DEFINITIONS(0);

    SOC_SAND_CHECK_NULL_INPUT(data);
    SOC_SAND_CHECK_NULL_INPUT(mask);

    ARAD_PP_LEM_ACCESS_KEY_clear(route_key_data);
    ARAD_PP_LEM_ACCESS_KEY_clear(route_key_mask);
    sal_memset(buffer_data, 0x0, sizeof(uint32) * ARAD_PP_FRWRD_IP_ELK_FWD_MAX_KEY_LENGTH_IN_LONGS);
    sal_memset(buffer_mask, 0x0, sizeof(uint32) * ARAD_PP_FRWRD_IP_ELK_FWD_MAX_KEY_LENGTH_IN_LONGS);

    /*
     * Transform according to KBP expectations: 
     *  - In KBP mask: 0 - care, 1 - don't care
     *  - Array indexation from max size-1 (e.g. 9 when the database size is 10 Bytes) to 0
     */
    for (byte_ndx = 0; byte_ndx < Arad_pp_tcam_kbp_table_attributes[frwrd_table_id].logical_entry_size_in_bytes; ++byte_ndx)
    {
        byte_array_idx = Arad_pp_tcam_kbp_table_attributes[frwrd_table_id].logical_entry_size_in_bytes - byte_ndx - 1;
        /* Build data */
        fld_val = data[byte_array_idx];
        SHR_BITCOPY_RANGE(buffer_data, (SOC_SAND_NOF_BITS_IN_BYTE * byte_ndx), &fld_val, 0, SOC_SAND_NOF_BITS_IN_BYTE);
        /* Build mask */
        fld_val = mask[byte_array_idx];
        fld_val = (~fld_val); /* Inverse of PPD convention here for mask: 0 - care, 1 - don't care */
        SHR_BITCOPY_RANGE(buffer_mask, (SOC_SAND_NOF_BITS_IN_BYTE * byte_ndx), &fld_val, 0, SOC_SAND_NOF_BITS_IN_BYTE);
    }

    /* Build the LEM params */
    for (param_ndx = 0; param_ndx < ARAD_PP_LEM_KEY_MAX_NOF_PARAMS; ++param_ndx)
    {
        res = arad_pp_tcam_kbp_lem_key_encode(
                unit,
                frwrd_table_id,
                param_ndx,
                0, /* do not try to set a value */
                route_key_data
              );
        SOC_SAND_CHECK_FUNC_RESULT(res, 12, exit);

        res = arad_pp_tcam_kbp_lem_key_encode(
                unit,
                frwrd_table_id,
                param_ndx,
                0, /* do not try to set a value */
                route_key_mask
              );
        SOC_SAND_CHECK_FUNC_RESULT(res, 14, exit);
    }

    /* Build the LEM params */
    for (param_ndx = 0; param_ndx < route_key_data->nof_params; ++param_ndx)
    {
        /* Get the LSB of each parameter according to the static table per forwarding type */
        buffer_lsb = Arad_pp_tcam_kbp_table_attributes[frwrd_table_id].lsb_nof_bits[param_ndx][0/*lsb*/];
        SHR_BITCOPY_RANGE(route_key_data->param[param_ndx].value, 0 /* LSB */, buffer_data, buffer_lsb, route_key_data->param[param_ndx].nof_bits);
        SHR_BITCOPY_RANGE(route_key_mask->param[param_ndx].value, 0 /* LSB */, buffer_mask, buffer_lsb, route_key_data->param[param_ndx].nof_bits);
    }


exit:
  SOC_SAND_EXIT_AND_SEND_ERROR( "error in arad_pp_frwrd_ip_tcam_route_kbp_buffer_decode()",0,0);
}

STATIC
uint32
  arad_pp_kbp_route_key_decode(
    SOC_SAND_IN  int                         unit,
    SOC_SAND_IN  uint8                       *data,
    SOC_SAND_IN  uint8                       *mask,
    SOC_SAND_IN  int                         valid_bytes,
    SOC_SAND_OUT uint32                      *route_key_data,
    SOC_SAND_OUT uint32                      *route_key_mask
  )
{
    uint32
        byte_ndx,
        byte_array_idx,
        fld_val;


    SOC_SAND_INIT_ERROR_DEFINITIONS(0);

    SOC_SAND_CHECK_NULL_INPUT(data);
    SOC_SAND_CHECK_NULL_INPUT(mask);

    /*
     * Transform according to KBP expectations: 
     *  - Array indexation from max size-1 (e.g. 9 when the database size is 10 Bytes) to 0
     */

    for (byte_ndx = 0; byte_ndx < valid_bytes; ++byte_ndx)
    {
        byte_array_idx = valid_bytes - byte_ndx - 1;
        fld_val = data[byte_array_idx];
        SHR_BITCOPY_RANGE(route_key_data, (SOC_SAND_NOF_BITS_IN_BYTE * byte_ndx), &fld_val, 0, SOC_SAND_NOF_BITS_IN_BYTE);
        fld_val = mask[byte_array_idx];
        fld_val = (~fld_val); /* Inverse of PPD convention here for mask: 0 - care, 1 - don't care */
        SHR_BITCOPY_RANGE(route_key_mask, (SOC_SAND_NOF_BITS_IN_BYTE * byte_ndx), &fld_val, 0, SOC_SAND_NOF_BITS_IN_BYTE);
    }

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR( "error in arad_pp_frwrd_ip_tcam_route_from_kbp_key_decode()",0,0);
}

/* Build/parse the LEM Key and a value */
uint32
  arad_pp_tcam_kbp_lem_key_encode(
    SOC_SAND_IN  int                     unit,
    SOC_SAND_IN  ARAD_KBP_FRWRD_IP_TBL_ID   frwrd_table_id,
    SOC_SAND_IN  uint32                     param_ndx,
    SOC_SAND_IN  uint32                     fld_val, /* only for values with less than 32 bits */
    SOC_SAND_INOUT ARAD_PP_LEM_ACCESS_KEY     *route_key
  )
{
    uint32
        value32;

    SOC_SAND_INIT_ERROR_DEFINITIONS(0);

    SOC_SAND_CHECK_NULL_INPUT(route_key);

    route_key->param[param_ndx].nof_bits = Arad_pp_tcam_kbp_table_attributes[frwrd_table_id].lsb_nof_bits[param_ndx][1 /* nof-bits */];
    if (route_key->param[param_ndx].nof_bits <= 32) {
        /* Copy only what is necessary */
        value32 = fld_val;
        SHR_BITCOPY_RANGE(route_key->param[param_ndx].value, 0, &value32, 0, route_key->param[param_ndx].nof_bits);
    }

    if (route_key->param[param_ndx].nof_bits) {
        /* increase the nof-params only if valid parameter: its nof-bits is not null */
        route_key->nof_params++;
    }
    
    ARAD_DO_NOTHING_AND_EXIT;

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR( "error in arad_pp_frwrd_ip_tcam_lem_key_encode()",0,0);
}

uint32
    arad_pp_tcam_kbp_table_clear(
      SOC_SAND_IN  int   unit,
      SOC_SAND_IN  uint32   frwrd_table_id
    )
{
    uint32
        res;
    struct kbp_db 
        *db_p = NULL;
    struct kbp_ad_db 
        *ad_db_p = NULL;

    SOC_SAND_INIT_ERROR_DEFINITIONS(0);

#ifdef CRASH_RECOVERY_SUPPORT
    if (ARAD_KBP_IS_CR_MODE(unit) == 1) {
        res = arad_kbp_cr_transaction_cmd(unit,TRUE);
        SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);
    } else {
        res = soc_dcmn_cr_suppress(unit,dcmn_cr_no_support_kaps_kbp);
        SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);
	}
#endif

    res = arad_kbp_alg_kbp_db_get(
              unit,
              frwrd_table_id, 
              &db_p
          );

    SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);
    SOC_SAND_CHECK_NULL_PTR(db_p, 20, exit);

    res = arad_kbp_alg_kbp_ad_db_get(
            unit,
            frwrd_table_id, 
            &ad_db_p
          );
    SOC_SAND_CHECK_FUNC_RESULT(res, 30, exit);
    SOC_SAND_CHECK_NULL_PTR(ad_db_p, 40, exit);

    res = kbp_db_delete_all_entries(db_p);
    if(ARAD_KBP_TO_SOC_RESULT(res) != SOC_SAND_OK) {
        LOG_ERROR(BSL_LS_SOC_TCAM,
                  (BSL_META_U(unit, "Error in %s(): kbp_db_delete_all_entries failed: %s\n"),
                  FUNCTION_NAME(), kbp_get_status_string(res)));
        SOC_SAND_SET_ERROR_CODE(SOC_SAND_GEN_ERR, 50, exit);
    }

    res = kbp_ad_db_delete_all_entries(ad_db_p);
    if(ARAD_KBP_TO_SOC_RESULT(res) != SOC_SAND_OK) {
        LOG_ERROR(BSL_LS_SOC_TCAM,
                  (BSL_META_U(unit, "Error in %s(): kbp_ad_db_delete_all_entries failed: %s\n"),
                  FUNCTION_NAME(), kbp_get_status_string(res)));
        SOC_SAND_SET_ERROR_CODE(SOC_SAND_GEN_ERR, 60, exit);
    }

#ifdef CRASH_RECOVERY_SUPPORT
    if (ARAD_KBP_IS_CR_MODE(unit) == 1) {
        dcmn_cr_info[unit]->kbp_dirty = 1;
        dcmn_cr_info[unit]->kbp_tbl_id = frwrd_table_id;
    }else
#endif
    {
        res = kbp_db_install(db_p);
        if(ARAD_KBP_TO_SOC_RESULT(res) != SOC_SAND_OK) {
            LOG_ERROR(BSL_LS_SOC_TCAM,(BSL_META_U(unit,"Error in %s(): kbp_db_install failed with error: %s!\n"), 
                                       FUNCTION_NAME(),kbp_get_status_string(res)));
            SOC_SAND_SET_ERROR_CODE(SOC_SAND_GEN_ERR, 110, exit);
        }
    }

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR( "error in arad_pp_frwrd_ip_tcam_kbp_table_clear()",0,0);
}

uint32
    arad_pp_tcam_kbp_lpm_route_get(
       SOC_SAND_IN  int         unit,
       SOC_SAND_IN  uint8       frwrd_table_id,
       SOC_SAND_IN  uint32      prefix_len,
       SOC_SAND_IN  uint8       *data,
       SOC_SAND_OUT uint8       *assoData,
       SOC_SAND_OUT uint8       *found
    )
{
    uint32
        res;
    struct kbp_db 
        *db_p = NULL;
    struct kbp_ad_db 
        *ad_db_p = NULL;
    struct kbp_ad 
        *ad_entry = NULL;
    struct kbp_entry
        *db_entry = NULL;

    SOC_SAND_INIT_ERROR_DEFINITIONS(0);

    *found = FALSE;

    res =  arad_kbp_alg_kbp_db_get(
            unit,
            frwrd_table_id, 
            &db_p
        );
    SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

    SOC_SAND_CHECK_NULL_PTR(db_p, 20, exit);

    /* Retrieve the db_entry */
    kbp_db_get_prefix_handle(db_p, (uint8*)data, prefix_len, &db_entry);
    if (!db_entry)
        goto exit;

    /* Retrieve the ad_entry */
    res =  arad_kbp_alg_kbp_ad_db_get(
            unit,
            frwrd_table_id, 
            &ad_db_p
        );
    SOC_SAND_CHECK_FUNC_RESULT(res, 30, exit);
    SOC_SAND_CHECK_NULL_PTR(ad_db_p, 40, exit);
    res = kbp_entry_get_ad(db_p, db_entry, &ad_entry);
    if(ARAD_KBP_TO_SOC_RESULT(res) != SOC_SAND_OK) {
        LOG_ERROR(BSL_LS_SOC_TCAM,
                  (BSL_META_U(unit, "Error in %s(): kbp_entry_get_ad failed: %s\n"),
                  FUNCTION_NAME(), kbp_get_status_string(res)));
        SOC_SAND_SET_ERROR_CODE(SOC_SAND_GEN_ERR, 80, exit);
    }

    SOC_SAND_CHECK_NULL_PTR(ad_entry, 90, exit);

    sal_memset(assoData, 0x0, sizeof(uint8) * SOC_DPP_TCAM_ACTION_ELK_KBP_GET_LEN_BYTES(unit));

    res = kbp_ad_db_get(ad_db_p, ad_entry, assoData);
    if(ARAD_KBP_TO_SOC_RESULT(res) != SOC_SAND_OK) {
        LOG_ERROR(BSL_LS_SOC_TCAM,
                  (BSL_META_U(unit, "Error in %s(): kbp_ad_db_get failed: %s\n"),
                  FUNCTION_NAME(), kbp_get_status_string(res)));
        SOC_SAND_SET_ERROR_CODE(SOC_SAND_GEN_ERR, 100, exit);
    }

    *found = TRUE;
    
exit:
  SOC_SAND_EXIT_AND_SEND_ERROR( "error in arad_pp_frwrd_ip_tcam_kbp_lpm_route_get()",0,0);
}

uint32
    arad_pp_tcam_kbp_lpm_route_add(
       SOC_SAND_IN  int                         unit,
       SOC_SAND_IN  uint32                      frwrd_table_id,
       SOC_SAND_IN  uint32                      data_indx,
       SOC_SAND_IN  uint32                      prefix_len,
       SOC_SAND_IN  uint8                       *data,
       SOC_SAND_IN  uint8                       *assoData,
       SOC_SAND_OUT SOC_SAND_SUCCESS_FAILURE    *success
    )
{
    int32
        res ;
    
    tableInfo 
        tbl_info;
    struct kbp_db 
       *db_p = NULL;
    struct kbp_ad_db 
       *ad_db_p = NULL;
    struct kbp_entry 
       *db_entry = NULL;
    struct kbp_ad 
       *ad_entry = NULL;
    uint8
        kbp_cache_mode;
    ARAD_SW_KBP_HANDLE
       location;
    int is_update = 0;
	uint8 diag_flag = 0;
	uint8 dummy_mask = 0xFF;


    SOC_SAND_INIT_ERROR_DEFINITIONS(0);

#ifdef CRASH_RECOVERY_SUPPORT
    if (ARAD_KBP_IS_CR_MODE(unit) == 1) {
        res = arad_kbp_cr_transaction_cmd(unit,TRUE);
        SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);
    } else {
        res = soc_dcmn_cr_suppress(unit,dcmn_cr_no_support_kaps_kbp);
        SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);
	}
#endif

    *success = SOC_SAND_SUCCESS;
    
    res = arad_kbp_gtm_table_info_get(
              unit,
              frwrd_table_id,
              &tbl_info
          );
    SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

    res = arad_kbp_alg_kbp_db_get(
              unit,
              frwrd_table_id, 
              &db_p
          );

    SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);

    SOC_SAND_CHECK_NULL_PTR(db_p, 30, exit);

    res =  arad_kbp_alg_kbp_ad_db_get(
            unit,
            frwrd_table_id, 
            &ad_db_p
        );
    SOC_SAND_CHECK_FUNC_RESULT(res, 50, exit);

    SOC_SAND_CHECK_NULL_PTR(ad_db_p, 30, exit);

    /* Check if the db_entry exists */
    kbp_db_get_prefix_handle(db_p, (uint8*)data, prefix_len, &db_entry);
    if (db_entry != NULL) {
        is_update = 1;
    }

    if (!is_update) {
        res = kbp_db_add_prefix(
                db_p, 
                (uint8*)data, 
                prefix_len,
                &db_entry);

        if(ARAD_KBP_TO_SOC_RESULT(res) != SOC_SAND_OK) {
           LOG_CLI((BSL_META_U(unit,"Error in %s(): Entry add : kbp_db_add_prefix with failed: %s!\n"),
                       FUNCTION_NAME(),
                       kbp_get_status_string(res)));

            *success = SOC_SAND_FAILURE_INTERNAL_ERR;
            SOC_SAND_SET_ERROR_CODE(SOC_SAND_GEN_ERR, 40, exit);
        }

		res = sw_state_access[unit].dpp.soc.arad.pp.frwrd_ip.kbp_diag_flag.entry_flag.get(unit, &diag_flag );
		SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 90, exit);

        res = kbp_ad_db_add_entry(
                ad_db_p,
                (uint8*)assoData,
                &ad_entry
              );

        if(ARAD_KBP_TO_SOC_RESULT(res) != SOC_SAND_OK) {
            kbp_db_delete_entry(db_p, db_entry); /* rollback */
            *success = SOC_SAND_FAILURE_INTERNAL_ERR;
            LOG_ERROR(BSL_LS_SOC_TCAM,
                      (BSL_META_U(unit, "Error in %s(): kbp_ad_db_add_entry failed: %s\n"),
    				  FUNCTION_NAME(), kbp_get_status_string(res)));
            SOC_SAND_SET_ERROR_CODE(SOC_SAND_GEN_ERR, 60, exit);
        }

		if(diag_flag == TRUE){
			arad_kbp_print_diag_entry_added(unit,&tbl_info,data,prefix_len,&dummy_mask,assoData);
		}

        res = kbp_entry_add_ad(
                db_p,
                db_entry,
                ad_entry
              );

        if(ARAD_KBP_TO_SOC_RESULT(res) != SOC_SAND_OK) {
            kbp_db_delete_entry(db_p, db_entry);
            kbp_ad_db_delete_entry(ad_db_p, ad_entry); /* rollback*/
            *success = SOC_SAND_FAILURE_INTERNAL_ERR;
            LOG_ERROR(BSL_LS_SOC_TCAM,
                      (BSL_META_U(unit, "Error in %s(): kbp_entry_add_ad failed: %s\n"),
    				  FUNCTION_NAME(), kbp_get_status_string(res)));
            SOC_SAND_SET_ERROR_CODE(SOC_SAND_GEN_ERR, 70, exit);
        }
    }
    else {
        res = kbp_entry_get_ad(db_p, db_entry, &ad_entry);
        if(ARAD_KBP_TO_SOC_RESULT(res) != SOC_SAND_OK) {
            *success = SOC_SAND_FAILURE_INTERNAL_ERR;
            LOG_ERROR(BSL_LS_SOC_TCAM,
                      (BSL_META_U(unit, "Error in %s(): kbp_entry_get_ad failed: %s\n"),
    				  FUNCTION_NAME(), kbp_get_status_string(res)));
            SOC_SAND_SET_ERROR_CODE(SOC_SAND_GEN_ERR, 70, exit);
        }

        res = kbp_ad_db_update_entry(ad_db_p, ad_entry, (uint8*)assoData);
        if(ARAD_KBP_TO_SOC_RESULT(res) != SOC_SAND_OK) {
            *success = SOC_SAND_FAILURE_INTERNAL_ERR;
            LOG_ERROR(BSL_LS_SOC_TCAM,
                      (BSL_META_U(unit, "Error in %s(): kbp_ad_db_update_entry failed: %s\n"),
    				  FUNCTION_NAME(), kbp_get_status_string(res)));
            SOC_SAND_SET_ERROR_CODE(SOC_SAND_GEN_ERR, 70, exit);
        }
    }

    res = sw_state_access[unit].dpp.soc.arad.pp.frwrd_ip.kbp_cache_mode.get(unit, &kbp_cache_mode);
    SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 75, exit);
    if(!SAL_BOOT_PLISIM){
        if (kbp_cache_mode == FALSE) {
#ifdef CRASH_RECOVERY_SUPPORT
            if (ARAD_KBP_IS_CR_MODE(unit) == 1) {
                dcmn_cr_info[unit]->kbp_dirty = 1;
                dcmn_cr_info[unit]->kbp_tbl_id = frwrd_table_id;
            } else
#endif
            {
#ifdef ARAD_PP_KBP_TIME_MEASUREMENTS
                soc_sand_ll_timer_set("ARAD_KBP_IPV4_TIMERS_LPM_ROUTE_ADD", ARAD_KBP_IPV4_TIMERS_LPM_ROUTE_ADD);
#endif 
                res = kbp_db_install(db_p);
#ifdef ARAD_PP_KBP_TIME_MEASUREMENTS
                soc_sand_ll_timer_stop(ARAD_KBP_IPV4_TIMERS_LPM_ROUTE_ADD);
#endif 
                if(ARAD_KBP_TO_SOC_RESULT(res) != SOC_SAND_OK) {
                   LOG_ERROR(BSL_LS_SOC_TCAM,
                             (BSL_META_U(unit,
                                         "Error in %s(): Entry add : kbp_db_install with failed: %s!\n"), 
                                         FUNCTION_NAME(),
                              kbp_get_status_string(res)));
                    kbp_db_delete_entry(db_p, db_entry); /* rollback*/
                    kbp_ad_db_delete_entry(ad_db_p, ad_entry);
                    *success = SOC_SAND_FAILURE_INTERNAL_ERR;
                    SOC_SAND_SET_ERROR_CODE(SOC_SAND_GEN_ERR, 80, exit);
                }
            }
        }
    }
    if ((frwrd_table_id == ARAD_KBP_FRWRD_TBL_ID_EXTENDED_IPV6) ||
 	(frwrd_table_id == ARAD_KBP_FRWRD_TBL_ID_EXTENDED_P2P) || 
	(frwrd_table_id == ARAD_KBP_FRWRD_TBL_ID_INRIF_MAPPING)) {
        location.db_entry = db_entry;
        res = sw_state_access[unit].dpp.soc.arad.pp.frwrd_ip.location_tbl.set(unit, data_indx, &location);
        SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 90, exit);
    }

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR( "error in arad_pp_frwrd_ip_tcam_kbp_lpm_route_add()",0,0);
}

uint32
    arad_pp_tcam_kbp_lpm_route_remove(
       SOC_SAND_IN  int                         unit,
       SOC_SAND_IN  uint32                      frwrd_table_id,
       SOC_SAND_IN  uint32                      prefix_len,
       SOC_SAND_IN  uint8                       *data
    )
{
    uint32
        res;
    uint8
        kbp_cache_mode;
    struct kbp_db 
        *db_p = NULL;
    struct kbp_ad_db 
        *ad_db_p = NULL;
    struct kbp_ad 
       *ad_entry = NULL;
    struct kbp_entry 
       *db_entry = NULL;
    
    SOC_SAND_INIT_ERROR_DEFINITIONS(0);

#ifdef CRASH_RECOVERY_SUPPORT
    if (ARAD_KBP_IS_CR_MODE(unit) == 1) {
        res = arad_kbp_cr_transaction_cmd(unit,TRUE);
        SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);
    } else {
        res = soc_dcmn_cr_suppress(unit,dcmn_cr_no_support_kaps_kbp);
        SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);
	}
#endif

    res = arad_kbp_alg_kbp_db_get(unit, frwrd_table_id, &db_p);
    if(ARAD_KBP_TO_SOC_RESULT(res) != SOC_SAND_OK) {
        SOC_SAND_SET_ERROR_CODE(SOC_SAND_GEN_ERR, 10, exit);
    }
    SOC_SAND_CHECK_NULL_PTR(db_p, 20, exit);

    /* Retrieve the db_entry */
    res = kbp_db_get_prefix_handle(db_p, (uint8*)data, prefix_len, &db_entry);
    if(ARAD_KBP_TO_SOC_RESULT(res) != SOC_SAND_OK) {
        LOG_ERROR(BSL_LS_SOC_TCAM,
                  (BSL_META_U(unit, "Error in %s(): kbp_db_get_prefix_handle failed: %s\n"),
                  FUNCTION_NAME(), kbp_get_status_string(res)));
        SOC_SAND_SET_ERROR_CODE(SOC_SAND_GEN_ERR, 30, exit);
    }
    SOC_SAND_CHECK_NULL_PTR(db_p, 40, exit);

    /* Retrieve the ad_entry */
    res = kbp_entry_get_ad(db_p, db_entry, &ad_entry);
    if(ARAD_KBP_TO_SOC_RESULT(res) != SOC_SAND_OK) {
        LOG_ERROR(BSL_LS_SOC_TCAM,
                  (BSL_META_U(unit, "Error in %s(): kbp_entry_get_ad failed: %s\n"),
                  FUNCTION_NAME(), kbp_get_status_string(res)));
        SOC_SAND_SET_ERROR_CODE(SOC_SAND_GEN_ERR, 50, exit);
    }
    SOC_SAND_CHECK_NULL_PTR(ad_entry, 60, exit);

    res = kbp_db_delete_entry( 
            db_p,
            db_entry
          );
    if(ARAD_KBP_TO_SOC_RESULT(res) != SOC_SAND_OK) {
        LOG_ERROR(BSL_LS_SOC_TCAM,
                  (BSL_META_U(unit, "Error in %s(): kbp_db_delete_entry failed: %s\n"),
                  FUNCTION_NAME(), kbp_get_status_string(res)));
        SOC_SAND_SET_ERROR_CODE(SOC_SAND_GEN_ERR, 70, exit);
    }

    res = arad_kbp_alg_kbp_ad_db_get(
            unit,
            frwrd_table_id, 
            &ad_db_p
          );
    SOC_SAND_CHECK_FUNC_RESULT(res, 80, exit);

    res = kbp_ad_db_delete_entry(
            ad_db_p, 
            ad_entry
          );
    if(ARAD_KBP_TO_SOC_RESULT(res) != SOC_SAND_OK) {
       LOG_ERROR(BSL_LS_SOC_TCAM,
                 (BSL_META_U(unit,
                             "Error in %s(): kbp_ad_db_delete_entry failed with error: %s!\n"), 
                             FUNCTION_NAME(),
                  kbp_get_status_string(res)));
        SOC_SAND_SET_ERROR_CODE(SOC_SAND_GEN_ERR, 90, exit);
    } 

    res = sw_state_access[unit].dpp.soc.arad.pp.frwrd_ip.kbp_cache_mode.get(unit, &kbp_cache_mode);
    SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 100, exit);

    if(!SAL_BOOT_PLISIM){
        if (kbp_cache_mode == FALSE) {
#ifdef CRASH_RECOVERY_SUPPORT
            if (ARAD_KBP_IS_CR_MODE(unit) == 1) {
                dcmn_cr_info[unit]->kbp_dirty = 1;
                dcmn_cr_info[unit]->kbp_tbl_id = frwrd_table_id;
            }else
#endif
            {
#ifdef ARAD_PP_KBP_TIME_MEASUREMENTS
                soc_sand_ll_timer_set("ARAD_KBP_IPV4_TIMERS_LPM_ROUTE_REMOVE", ARAD_KBP_IPV4_TIMERS_LPM_ROUTE_REMOVE);
#endif 
                res = kbp_db_install(db_p);
#ifdef ARAD_PP_KBP_TIME_MEASUREMENTS
                soc_sand_ll_timer_stop(ARAD_KBP_IPV4_TIMERS_LPM_ROUTE_REMOVE);
#endif 
                if(ARAD_KBP_TO_SOC_RESULT(res) != SOC_SAND_OK) {
                    LOG_ERROR(BSL_LS_SOC_TCAM,(BSL_META_U(unit,"Error in %s(): kbp_db_install failed with error: %s!\n"), 
                                 FUNCTION_NAME(),kbp_get_status_string(res)));
                    SOC_SAND_SET_ERROR_CODE(SOC_SAND_GEN_ERR, 110, exit);
                }
            }
        }
    }
exit:
  SOC_SAND_EXIT_AND_SEND_ERROR( "error in arad_pp_frwrd_ip_tcam_kbp_lpm_route_remove()",0,0);
}


uint32
    arad_pp_tcam_kbp_route_get(
       SOC_SAND_IN  int             unit,
       SOC_SAND_IN  uint8           frwrd_table_id,
       SOC_SAND_IN  uint32          data_indx,
       SOC_SAND_OUT uint8           *data,
       SOC_SAND_OUT uint8           *mask,
       SOC_SAND_OUT uint8           *assoData,
       SOC_SAND_OUT uint32          *priority,
       SOC_SAND_OUT uint8           *found,
       SOC_SAND_OUT int             *valid_bytes
    )
{
    uint32
        res;
    struct kbp_db 
        *db_p = NULL;
    struct kbp_ad_db 
        *ad_db_p = NULL;
    ARAD_SW_KBP_HANDLE
        location;
    struct kbp_entry_info
        entry_info;

    SOC_SAND_INIT_ERROR_DEFINITIONS(0);

    *found = FALSE;

    res =  arad_kbp_alg_kbp_ad_db_get(
            unit,
            frwrd_table_id, 
            &ad_db_p
        );
    SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

    SOC_SAND_CHECK_NULL_PTR(ad_db_p, 20, exit);

    /* Retrieve the record index */
    res = sw_state_access[unit].dpp.soc.arad.pp.frwrd_ip.location_tbl.get(unit, data_indx, &location);
    SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 25, exit);

    /* Consider the entry not found if the db_entry is null */
    if (location.db_entry == NULL) {
        *found = FALSE;
        ARAD_DO_NOTHING_AND_EXIT;
    }
    SOC_SAND_CHECK_NULL_PTR(location.db_entry, 27, exit);    
    
    res = arad_kbp_alg_kbp_db_get(unit, frwrd_table_id, &db_p);
    SOC_SAND_CHECK_FUNC_RESULT(res, 30, exit);

    /* Retrieve the entry key */
    res = kbp_entry_get_info(db_p, location.db_entry, &entry_info);
    if(ARAD_KBP_TO_SOC_RESULT(res) != SOC_SAND_OK) {
        LOG_ERROR(BSL_LS_SOC_TCAM,
                  (BSL_META_U(unit, "Error in %s(): kbp_entry_get_info failed: %s\n"),
                  FUNCTION_NAME(), kbp_get_status_string(res)));
        SOC_SAND_SET_ERROR_CODE(SOC_SAND_GEN_ERR, 35, exit);
    }

    (*valid_bytes) = entry_info.width_8;
    sal_memcpy(data, entry_info.data, sizeof(uint8) * (entry_info.width_8));
    sal_memcpy(mask, entry_info.mask, sizeof(uint8) * (entry_info.width_8));

    SOC_SAND_CHECK_NULL_PTR(entry_info.ad_handle, 40, exit);

    sal_memset(assoData, 0x0, sizeof(uint8) * SOC_DPP_TCAM_ACTION_ELK_KBP_GET_LEN_BYTES(unit));

    res = kbp_ad_db_get(ad_db_p, entry_info.ad_handle, assoData);
    if(ARAD_KBP_TO_SOC_RESULT(res) != SOC_SAND_OK) {
        LOG_ERROR(BSL_LS_SOC_TCAM,
                  (BSL_META_U(unit, "Error in %s(): kbp_ad_db_get failed: %s\n"),
                  FUNCTION_NAME(), kbp_get_status_string(res)));
        SOC_SAND_SET_ERROR_CODE(SOC_SAND_GEN_ERR, 50, exit);
    }

    (*priority) = entry_info.prio_len;
    (*found) = TRUE;
    
exit:
  SOC_SAND_EXIT_AND_SEND_ERROR( "error in arad_pp_frwrd_ip_tcam_kbp_route_get()",0,0);
}

uint32
    arad_pp_tcam_kbp_route_add(
       SOC_SAND_IN  int                         unit,
       SOC_SAND_IN  uint32                      frwrd_table_id,
       SOC_SAND_IN  uint32                      data_indx,
       SOC_SAND_IN  uint8                       is_for_update,
       SOC_SAND_IN  uint32                      priority,
       SOC_SAND_IN  uint8                       *m_data,
       SOC_SAND_IN  uint8                       *m_mask,
       SOC_SAND_IN  uint8                       *assoData,
       SOC_SAND_OUT SOC_SAND_SUCCESS_FAILURE    *success
    )
{
    int32
        res ;
    
    tableInfo 
        tbl_info;
    struct kbp_db 
       *db_p = NULL;
    struct kbp_ad_db 
       *ad_db_p = NULL;
    struct kbp_entry 
       *db_entry = NULL;
    struct kbp_ad 
       *ad_entry = NULL;
    ARAD_SW_KBP_HANDLE
       location;
    uint8
        kbp_cache_mode;
	uint8 diag_flag = 0;

    SOC_SAND_INIT_ERROR_DEFINITIONS(0);

#ifdef CRASH_RECOVERY_SUPPORT
    if (ARAD_KBP_IS_CR_MODE(unit) == 1) {
        res = arad_kbp_cr_transaction_cmd(unit,TRUE);
        SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);
    } else {
        res = soc_dcmn_cr_suppress(unit,dcmn_cr_no_support_kaps_kbp);
        SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);
	}
#endif

    *success = SOC_SAND_SUCCESS;

    res = arad_kbp_gtm_table_info_get(
              unit,
              frwrd_table_id,
              &tbl_info
          );
    SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

    res = arad_kbp_alg_kbp_db_get(
              unit,
              frwrd_table_id, 
              &db_p
          );

    SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);

    SOC_SAND_CHECK_NULL_PTR(db_p, 30, exit);

    res = kbp_db_add_ace(
            db_p, 
            (uint8*)m_data, 
            (uint8*)m_mask,
            priority,
            &db_entry);

    if(ARAD_KBP_TO_SOC_RESULT(res) != SOC_SAND_OK) {
        *success = SOC_SAND_FAILURE_INTERNAL_ERR;
        LOG_ERROR(BSL_LS_SOC_TCAM,
                  (BSL_META_U(unit, "Error in %s(): kbp_db_add_ace failed: %s\n"),
                  FUNCTION_NAME(), kbp_get_status_string(res)));
        SOC_SAND_SET_ERROR_CODE(SOC_SAND_GEN_ERR, 40, exit);
    }

	res = sw_state_access[unit].dpp.soc.arad.pp.frwrd_ip.kbp_diag_flag.entry_flag.get(unit, &diag_flag );
	SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 90, exit);

    res =  arad_kbp_alg_kbp_ad_db_get(
            unit,
            frwrd_table_id, 
            &ad_db_p
        );
    SOC_SAND_CHECK_FUNC_RESULT(res, 50, exit);

    res = kbp_ad_db_add_entry(
            ad_db_p,
            (uint8*)assoData,
            &ad_entry
          );

    if(ARAD_KBP_TO_SOC_RESULT(res) != SOC_SAND_OK) {
        kbp_db_delete_entry(db_p, db_entry); /* rollback */
        *success = SOC_SAND_FAILURE_INTERNAL_ERR;
        LOG_ERROR(BSL_LS_SOC_TCAM,
                  (BSL_META_U(unit, "Error in %s(): kbp_ad_db_add_entry failed: %s\n"),
                  FUNCTION_NAME(), kbp_get_status_string(res)));
        SOC_SAND_SET_ERROR_CODE(SOC_SAND_GEN_ERR, 60, exit);
    }

    if(diag_flag == TRUE){
            arad_kbp_print_diag_entry_added(unit,&tbl_info,m_data,0,m_mask,(uint8*)assoData);
    }
	
    res = kbp_entry_add_ad(
            db_p,
            db_entry,
            ad_entry
          );

    if(ARAD_KBP_TO_SOC_RESULT(res) != SOC_SAND_OK) {
        kbp_db_delete_entry(db_p, db_entry);
        kbp_ad_db_delete_entry(ad_db_p, ad_entry); /* rollback*/
        *success = SOC_SAND_FAILURE_INTERNAL_ERR;
        LOG_ERROR(BSL_LS_SOC_TCAM,
                  (BSL_META_U(unit, "Error in %s(): kbp_entry_add_ad failed: %s\n"),
                  FUNCTION_NAME(), kbp_get_status_string(res)));
        SOC_SAND_SET_ERROR_CODE(SOC_SAND_GEN_ERR, 70, exit);
    }

    res = sw_state_access[unit].dpp.soc.arad.pp.frwrd_ip.kbp_cache_mode.get(unit, &kbp_cache_mode);
    SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 75, exit);
    if(!SAL_BOOT_PLISIM){
        if (kbp_cache_mode == FALSE) {
#ifdef CRASH_RECOVERY_SUPPORT
            if (ARAD_KBP_IS_CR_MODE(unit) == 1) {
                dcmn_cr_info[unit]->kbp_dirty = 1;
                dcmn_cr_info[unit]->kbp_tbl_id = frwrd_table_id;
            }else 
#endif
            {
#ifdef ARAD_PP_KBP_TIME_MEASUREMENTS
                soc_sand_ll_timer_set("ARAD_KBP_ACL_TIMERS_ROUTE_ADD", ARAD_KBP_ACL_TIMERS_ROUTE_ADD);
#endif 
                res = kbp_db_install(db_p);
#ifdef ARAD_PP_KBP_TIME_MEASUREMENTS
                soc_sand_ll_timer_stop(ARAD_KBP_ACL_TIMERS_ROUTE_ADD);
#endif 
                if(ARAD_KBP_TO_SOC_RESULT(res) != SOC_SAND_OK) {
                    LOG_ERROR(BSL_LS_SOC_TCAM,
                              (BSL_META_U(unit,
                                          "Error in %s(): Entry add : kbp_db_install with failed: %s!\n"), 
                               FUNCTION_NAME(),
                               kbp_get_status_string(res)));
                    kbp_db_delete_entry(db_p, db_entry); /* rollback*/
                    kbp_ad_db_delete_entry(ad_db_p, ad_entry);
                    *success = SOC_SAND_FAILURE_INTERNAL_ERR;
                    SOC_SAND_SET_ERROR_CODE(SOC_SAND_GEN_ERR, 80, exit);
                }
            }
        }
    }
    /* In case of update, remove the existing entry before updating the location table */
    if (is_for_update == TRUE) {
        res = arad_pp_tcam_kbp_route_remove(
                  unit,
                  frwrd_table_id,
                  data_indx
                );
        SOC_SAND_CHECK_FUNC_RESULT(res, 15, exit);
    }

    /* Insert the location mapping in the table */
    location.db_entry = db_entry;
    res = sw_state_access[unit].dpp.soc.arad.pp.frwrd_ip.location_tbl.set(unit, data_indx, &location);
    SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 90, exit);

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR( "error in arad_pp_frwrd_ip_tcam_kbp_route_add()",0,0);
}

uint32
    arad_pp_tcam_kbp_route_remove(
       SOC_SAND_IN  int                      unit,
       SOC_SAND_IN  uint8                       frwrd_table_id,
       SOC_SAND_IN  uint32                      data_indx
    )
{
    uint32
        res;
    uint8
        kbp_cache_mode;
    struct kbp_db 
        *db_p = NULL;
    struct kbp_ad_db 
        *ad_db_p = NULL;
    struct kbp_entry
        *db_entry;
    struct kbp_ad 
       *ad_entry = NULL;
    ARAD_SW_KBP_HANDLE
        location;
    
    SOC_SAND_INIT_ERROR_DEFINITIONS(0);

#ifdef CRASH_RECOVERY_SUPPORT
    if (ARAD_KBP_IS_CR_MODE(unit) == 1) {
        res = arad_kbp_cr_transaction_cmd(unit,TRUE);
        SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);
    } else {
        res = soc_dcmn_cr_suppress(unit,dcmn_cr_no_support_kaps_kbp);
        SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);
	}
#endif

    /* Retrieve the record index */
    res = sw_state_access[unit].dpp.soc.arad.pp.frwrd_ip.location_tbl.get(unit, data_indx, &location);
    SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 5, exit);
    db_entry = location.db_entry;
    SOC_SAND_CHECK_NULL_PTR(db_entry, 10, exit);

    res = arad_kbp_alg_kbp_db_get(unit, frwrd_table_id, &db_p);
    SOC_SAND_CHECK_FUNC_RESULT(res, 30, exit);

    /* Retrieve the ad_entry */
    res = kbp_entry_get_ad(db_p, location.db_entry, &ad_entry);
    if(ARAD_KBP_TO_SOC_RESULT(res) != SOC_SAND_OK) {
        LOG_ERROR(BSL_LS_SOC_TCAM,
                  (BSL_META_U(unit, "Error in %s(): kbp_entry_get_ad failed: %s\n"),
                  FUNCTION_NAME(), kbp_get_status_string(res)));
        SOC_SAND_SET_ERROR_CODE(SOC_SAND_GEN_ERR, 37, exit);
    }
    SOC_SAND_CHECK_NULL_PTR(ad_entry, 20, exit);

    SOC_SAND_CHECK_NULL_PTR(db_p, 40, exit);

    res = kbp_db_delete_entry( 
            db_p,
            db_entry
          );
    if(ARAD_KBP_TO_SOC_RESULT(res) != SOC_SAND_OK) {
        LOG_ERROR(BSL_LS_SOC_TCAM,
                  (BSL_META_U(unit, "Error in %s(): kbp_db_delete_entry failed: %s\n"),
                  FUNCTION_NAME(), kbp_get_status_string(res)));
        SOC_SAND_SET_ERROR_CODE(SOC_SAND_GEN_ERR, 50, exit);
    }

    res = arad_kbp_alg_kbp_ad_db_get(
            unit,
            frwrd_table_id, 
            &ad_db_p
          );
    SOC_SAND_CHECK_FUNC_RESULT(res, 60, exit);

    res = kbp_ad_db_delete_entry(
            ad_db_p, 
            ad_entry
          );
    if(ARAD_KBP_TO_SOC_RESULT(res) != SOC_SAND_OK) {
        LOG_ERROR(BSL_LS_SOC_TCAM,
                  (BSL_META_U(unit,
                              "Error in %s(): kbp_ad_db_delete_entry failed with error: %s!\n"), 
                              FUNCTION_NAME(),
                   kbp_get_status_string(res)));
        SOC_SAND_SET_ERROR_CODE(SOC_SAND_GEN_ERR, 70, exit);
    } 

    res = sw_state_access[unit].dpp.soc.arad.pp.frwrd_ip.kbp_cache_mode.get(unit, &kbp_cache_mode);
    SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 75, exit);

    if(!SAL_BOOT_PLISIM){
        if (kbp_cache_mode == FALSE) {
#ifdef CRASH_RECOVERY_SUPPORT
            if (ARAD_KBP_IS_CR_MODE(unit) == 1) {
                dcmn_cr_info[unit]->kbp_dirty = 1;
                dcmn_cr_info[unit]->kbp_tbl_id = frwrd_table_id;
            }else
#endif
            {
#ifdef ARAD_PP_KBP_TIME_MEASUREMENTS
                soc_sand_ll_timer_set("ARAD_KBP_ACL_TIMERS_ROUTE_REMOVE", ARAD_KBP_ACL_TIMERS_ROUTE_REMOVE);
#endif 
                res = kbp_db_install(db_p);
#ifdef ARAD_PP_KBP_TIME_MEASUREMENTS
                soc_sand_ll_timer_stop(ARAD_KBP_ACL_TIMERS_ROUTE_REMOVE);
#endif 
                if(ARAD_KBP_TO_SOC_RESULT(res) != SOC_SAND_OK) {
                    LOG_ERROR(BSL_LS_SOC_TCAM,
                              (BSL_META_U(unit, "Error in %s(): kbp_db_install failed: %s\n"),
                              FUNCTION_NAME(), kbp_get_status_string(res)));
                    SOC_SAND_SET_ERROR_CODE(SOC_SAND_GEN_ERR, 80, exit);
                }
            }
        }
    }

    /* If remove was sucessfull clear location table index */
    sal_memset(&location, 0x0, sizeof(ARAD_SW_KBP_HANDLE));
    res = sw_state_access[unit].dpp.soc.arad.pp.frwrd_ip.location_tbl.set(unit, data_indx, &location);
    SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 90, exit);

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR( "error in arad_pp_frwrd_ip_tcam_kbp_route_remove()",0,0);
}


uint32
  arad_pp_tcam_kbp_tcam_entry_get(
    SOC_SAND_IN  int                        unit,
    SOC_SAND_IN  ARAD_KBP_FRWRD_IP_TBL_ID   frwrd_table_id,
    SOC_SAND_IN  uint32                     entry_id,
    SOC_SAND_OUT uint32                     data[ARAD_TCAM_ENTRY_MAX_LEN],
    SOC_SAND_OUT uint32                     mask[ARAD_TCAM_ENTRY_MAX_LEN],
    SOC_SAND_OUT uint32                     value[ARAD_TCAM_ACTION_MAX_LEN],
    SOC_SAND_OUT uint32                     *priority,    
    SOC_SAND_OUT uint8                      *found,
    SOC_SAND_OUT uint8                      *hit_bit
  )
{
    uint32
      res = SOC_SAND_OK;
    
    uint8 m_data[ARAD_PP_FRWRD_IP_ELK_FWD_MAX_KEY_LENGTH_IN_BYTES] = {0};
    uint8 m_mask[ARAD_PP_FRWRD_IP_ELK_FWD_MAX_KEY_LENGTH_IN_BYTES] = {0};
    uint8 elk_ad_value[ARAD_PP_FRWRD_IP_ELK_FWD_MAX_KEY_LENGTH_IN_BYTES] = {0};
    int nof_valid_bytes = 0;

    SOC_SAND_INIT_ERROR_DEFINITIONS(0);

    sal_memset(data,0,sizeof(ARAD_TCAM_ENTRY_MAX_LEN)*sizeof(uint32));
    sal_memset(mask,0,sizeof(ARAD_TCAM_ENTRY_MAX_LEN)*sizeof(uint32));
    sal_memset(value,0,sizeof(ARAD_TCAM_ENTRY_MAX_LEN)*sizeof(uint32));

    (*hit_bit) = 0; /* Featrue not supported in KBP */
      
    res = arad_pp_tcam_kbp_route_get(
            unit,
            frwrd_table_id,
            entry_id,
            m_data,
            m_mask,
            elk_ad_value,
            priority,
            found,
            &nof_valid_bytes
          );
    SOC_SAND_CHECK_FUNC_RESULT(res, 25, exit);

    /* Convert the entry key according to KBP constraints */
    res = arad_pp_kbp_route_key_decode(unit, m_data, m_mask, nof_valid_bytes, data, mask);
    SOC_SAND_CHECK_FUNC_RESULT(res, 30, exit);

    /* Convert the entry payload according to KBP constraints */
    res = arad_pp_tcam_route_kbp_payload_buffer_decode(unit, frwrd_table_id, elk_ad_value, value);
    SOC_SAND_CHECK_FUNC_RESULT(res, 27, exit);      

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_frwrd_ip_tcam_kbp_tcam_entry_get()", 0, 0);
}

/* adding entry according to data and mask */
uint32
  arad_pp_tcam_kbp_tcam_entry_add(
	int 								unit,
	ARAD_KBP_FRWRD_IP_TBL_ID 			frwrd_table_id,
	uint32 								entry_id_ndx,
	uint8 								is_for_update,
	uint32 								priority,
	uint32 								data[ARAD_TCAM_ENTRY_MAX_LEN],
	uint32  							mask[ARAD_TCAM_ENTRY_MAX_LEN],
	uint32 								value[ARAD_TCAM_ACTION_MAX_LEN],
	SOC_SAND_IN SOC_PPC_FP_ENTRY_INFO	*info,
	SOC_SAND_SUCCESS_FAILURE 			*success
  )
{
  uint32  res = SOC_SAND_OK;
  uint32  logical_entry_size_in_bytes, table_payload_in_bytes;
  uint8   elk_data[ARAD_PP_FRWRD_IP_ELK_FWD_MAX_KEY_LENGTH_IN_BYTES] = {0};
  uint8   elk_mask[ARAD_PP_FRWRD_IP_ELK_FWD_MAX_KEY_LENGTH_IN_BYTES] = {0};
  uint8   elk_ad_value[SOC_DPP_TCAM_ACTION_ELK_KBP_MAX_LEN_BYTES] = {0};

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);
  
  res = arad_kbp_table_size_get(unit, frwrd_table_id, &logical_entry_size_in_bytes, &table_payload_in_bytes); /* For ACL, entry-size = table-size */
  SOC_SAND_CHECK_FUNC_RESULT(res,  71, exit);

  res = arad_pp_tcam_route_buffer_to_kbp_buffer_encode(
          unit,
          logical_entry_size_in_bytes,
          data,
          mask,
          elk_data,
          elk_mask
        );
  SOC_SAND_CHECK_FUNC_RESULT(res,  73, exit);

  res = arad_pp_tcam_route_kbp_payload_buffer_encode(
          unit,
          table_payload_in_bytes,
          value, /* limited to 96b */
          elk_ad_value);
  SOC_SAND_CHECK_FUNC_RESULT(res,  76, exit);

  if (info) {
      res = arad_pp_fp_dip_sip_sharing_handle(unit, frwrd_table_id - ARAD_KBP_ACL_TABLE_ID_OFFSET, info, elk_data, elk_mask);
      SOC_SAND_CHECK_FUNC_RESULT(res, 70, exit);
  }


  res = arad_pp_tcam_kbp_route_add(
          unit,
          frwrd_table_id,
          entry_id_ndx,
          is_for_update,
          priority,
          elk_data,
          elk_mask,
          elk_ad_value,
          success
        );

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_frwrd_ip_tcam_kbp_tcam_entry_add()", frwrd_table_id, entry_id_ndx);
}

uint32
  arad_pp_tcam_route_buffer_to_kbp_buffer_encode(
    SOC_SAND_IN  int                     unit,
    SOC_SAND_IN  uint32                     logical_entry_size_in_bytes,
    SOC_SAND_IN  uint32                     *buffer_data,
    SOC_SAND_IN  uint32                     *buffer_mask,
    SOC_SAND_OUT uint8                      *data,
    SOC_SAND_OUT uint8                      *mask
  )
{
    uint32
        byte_ndx,
        byte_array_idx,
        fld_val;

    SOC_SAND_INIT_ERROR_DEFINITIONS(0);

    SOC_SAND_CHECK_NULL_INPUT(data);
    SOC_SAND_CHECK_NULL_INPUT(mask);
    SOC_SAND_CHECK_NULL_INPUT(buffer_data);
    SOC_SAND_CHECK_NULL_INPUT(buffer_mask);

    /*
     * Transform according to KBP expectations: 
     *  - In KBP mask: 0 - care, 1 - don't care
     *  - Array indexation from max size-1 (e.g. 9 when the database size is 10 Bytes) to 0
     */
    for (byte_ndx = 0; byte_ndx < ARAD_PP_FRWRD_IP_ELK_FWD_MAX_KEY_LENGTH_IN_BYTES; ++byte_ndx)
    {
        mask[byte_ndx] = 0xFF;
    }

    for (byte_ndx = 0; byte_ndx < logical_entry_size_in_bytes; ++byte_ndx)
    {
        byte_array_idx = logical_entry_size_in_bytes - byte_ndx - 1;
        /* Build data */
        fld_val = 0;
        SHR_BITCOPY_RANGE(&fld_val, 0, buffer_data, (SOC_SAND_NOF_BITS_IN_BYTE * byte_ndx), SOC_SAND_NOF_BITS_IN_BYTE);
        data[byte_array_idx] = (uint8) (fld_val & 0xFF);
        /* Build mask */
        fld_val = 0;
        SHR_BITCOPY_RANGE(&fld_val, 0, buffer_mask, (SOC_SAND_NOF_BITS_IN_BYTE * byte_ndx), SOC_SAND_NOF_BITS_IN_BYTE);
        fld_val = (~fld_val); /* Inverse of PPD convention here for mask: 0 - care, 1 - don't care */
        mask[byte_array_idx] = (uint8) (fld_val & 0xFF);
    }

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR( "error in arad_pp_frwrd_ip_tcam_route_buffer_to_kbp_buffer_encode()",0,0);
}

/* Add entry according to LEM buffer */
uint32
  arad_pp_tcam_route_kbp_add_unsafe(
    SOC_SAND_IN  int                     unit,
    SOC_SAND_IN  ARAD_KBP_FRWRD_IP_TBL_ID   frwrd_table_id,
    SOC_SAND_IN  ARAD_PP_LEM_ACCESS_KEY     *route_key_data,
    SOC_SAND_IN  ARAD_PP_LEM_ACCESS_KEY     *route_key_mask,
    SOC_SAND_IN  uint32                     priority,
    SOC_SAND_IN  ARAD_PP_LEM_ACCESS_PAYLOAD *payload,
    SOC_SAND_OUT SOC_SAND_SUCCESS_FAILURE   *success
  )
{
    uint32
        res;
    ARAD_PP_IP_TCAM_ENTRY_KEY
      key;
    ARAD_TCAM_ACTION                                
        action;

    SOC_SAND_INIT_ERROR_DEFINITIONS(0);

    soc_sand_os_memset(&key, 0x0, sizeof(key));
    soc_sand_os_memset(&action, 0x0, sizeof(action));

    key.type = ARAD_IP_TCAM_FROM_KBP_FRWRD_IP_TBL_ID(frwrd_table_id);
    key.key.elk_fwd.priority = priority;

    /* 
     * Encode Key: 
     * - use LEM access logic to transform the LEM access key to a 
     * buffer (done both on data and mask) 
     * - transform the buffer to the order expected by KBP 
     */
    res = arad_pp_kbp_route_buffer_encode(
            unit,
            frwrd_table_id,
            route_key_data,
            route_key_mask,
            key.key.elk_fwd.m_data,
            key.key.elk_fwd.m_mask
          );
    SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit); 

    /* Encode action in a buffer */
    res = arad_pp_kbp_route_payload_encode(
            unit,
            frwrd_table_id,
            payload,
            action.elk_ad_value
          );
    SOC_SAND_CHECK_FUNC_RESULT(res, 15, exit);

    if (ARAD_IP_TCAM_ENTRY_TYPE_IS_KBP_FORWARDING(key.type)) {
      uint32 prefix_len = 0, table_size_in_bytes, table_payload_in_bytes;

        res = arad_kbp_table_size_get(unit, frwrd_table_id, &table_size_in_bytes, &table_payload_in_bytes);
        SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

        res = arad_pp_frwrd_ip_tcam_lpm_prefix_len_get(key.key.elk_fwd.m_mask, table_size_in_bytes, &prefix_len);
        SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);

        res = arad_pp_tcam_kbp_lpm_route_add(
                unit,
                frwrd_table_id,
                0,
                prefix_len,
                key.key.elk_fwd.m_data,
                action.elk_ad_value,
                success
              );

        SOC_SAND_CHECK_FUNC_RESULT(res, 00, exit);      
    }else{
         SOC_SAND_SET_ERROR_CODE(SOC_SAND_GEN_ERR, 10, exit);
    }

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR( "error in arad_pp_frwrd_ip_tcam_route_kbp_add_unsafe()",key.type,0);
}

/* Get entry according to LEM buffer */
uint32
  arad_pp_tcam_route_kbp_get_unsafe(
    SOC_SAND_IN  int                     unit,
    SOC_SAND_IN  ARAD_KBP_FRWRD_IP_TBL_ID   frwrd_table_id,
    SOC_SAND_IN  ARAD_PP_LEM_ACCESS_KEY     *route_key_data,
    SOC_SAND_IN  ARAD_PP_LEM_ACCESS_KEY     *route_key_mask,
    SOC_SAND_OUT uint32                     *priority,
    SOC_SAND_OUT ARAD_PP_LEM_ACCESS_PAYLOAD *payload,
    SOC_SAND_OUT uint8                      *found
  )
{
    uint32
        res;
    ARAD_PP_IP_TCAM_ENTRY_KEY
      key;
    ARAD_TCAM_ACTION                                
        action;

    SOC_SAND_INIT_ERROR_DEFINITIONS(0);

    soc_sand_os_memset(&key, 0x0, sizeof(key));
    soc_sand_os_memset(&action, 0x0, sizeof(action));

    *priority = 0;
    *found = 0;
    ARAD_PP_LEM_ACCESS_PAYLOAD_clear(payload);

    key.type = ARAD_IP_TCAM_FROM_KBP_FRWRD_IP_TBL_ID(frwrd_table_id);

    /* 
     * Encode Key: 
     * - use LEM access logic to transform the LEM access key to a 
     * buffer (done both on data and mask) 
     * - transform the buffer to the order expected by KBP 
     */
    res = arad_pp_kbp_route_buffer_encode(
            unit,
            frwrd_table_id,
            route_key_data,
            route_key_mask,
            key.key.elk_fwd.m_data,
            key.key.elk_fwd.m_mask
          );
    SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit); 

    if (ARAD_IP_TCAM_ENTRY_TYPE_IS_KBP_FORWARDING(key.type)) {
        /* Get record */
        uint32 prefix_len = 0, table_size_in_bytes, table_payload_in_bytes;
  
        res = arad_kbp_table_size_get(unit, ARAD_IP_TCAM_TO_KBP_FRWRD_IP_TBL_ID(key.type), &table_size_in_bytes, &table_payload_in_bytes);
        SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);
  
        res = arad_pp_frwrd_ip_tcam_lpm_prefix_len_get(key.key.elk_fwd.m_mask, table_size_in_bytes, &prefix_len);
        SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);
 
        res = arad_pp_tcam_kbp_lpm_route_get(
                unit,
                frwrd_table_id,
                prefix_len,
                key.key.elk_fwd.m_data,
                action.elk_ad_value,
                found
              );

        SOC_SAND_CHECK_FUNC_RESULT(res, 30, exit);        
    }
    else {
        /* Get record */
        /*res = arad_pp_frwrd_ip_tcam_route_get_unsafe(unit,&key,TRUE ,&action,found,&hit_bit);
        SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit); */

        SOC_SAND_EXIT_AND_SEND_ERROR( "error in no available DB",0,0);
    }

    /* Parse the action value if found */
    if (*found) 
    {
        *priority = key.key.elk_fwd.priority;
        /* Encode action in a buffer */
        res = arad_pp_kbp_route_payload_decode(
                unit,
                frwrd_table_id,
                0 /* lookup_id */,
                action.elk_ad_value,
                payload
              );
        SOC_SAND_CHECK_FUNC_RESULT(res, 15, exit); 
    }
    
exit:
  SOC_SAND_EXIT_AND_SEND_ERROR( "error in arad_pp_frwrd_ip_tcam_route_kbp_get_unsafe()",0,0);
}

/* Remove entry according to data and mask */
uint32
  arad_pp_tcam_route_kbp_remove_unsafe(
    SOC_SAND_IN  int                     unit,
    SOC_SAND_IN  ARAD_KBP_FRWRD_IP_TBL_ID   frwrd_table_id,
    SOC_SAND_IN  ARAD_PP_LEM_ACCESS_KEY     *route_key_data,
    SOC_SAND_IN  ARAD_PP_LEM_ACCESS_KEY     *route_key_mask
  )
{
    uint32
        res;
    ARAD_PP_IP_TCAM_ENTRY_KEY
      key;

    SOC_SAND_INIT_ERROR_DEFINITIONS(0);

    soc_sand_os_memset(&key, 0x0, sizeof(key));

    key.type = ARAD_IP_TCAM_FROM_KBP_FRWRD_IP_TBL_ID(frwrd_table_id);

    /* 
     * Encode Key: 
     * - use LEM access logic to transform the LEM access key to a 
     * buffer (done both on data and mask) 
     * - transform the buffer to the order expected by KBP 
     */
    res = arad_pp_kbp_route_buffer_encode(
            unit,
            frwrd_table_id,
            route_key_data,
            route_key_mask,
            key.key.elk_fwd.m_data,
            key.key.elk_fwd.m_mask
          );
    SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit); 

    if (ARAD_IP_TCAM_ENTRY_TYPE_IS_KBP_FORWARDING(key.type)) {

      uint32 prefix_len = 0, table_size_in_bytes, table_payload_in_bytes;
  
      res = arad_kbp_table_size_get(unit, ARAD_IP_TCAM_TO_KBP_FRWRD_IP_TBL_ID(key.type), &table_size_in_bytes, &table_payload_in_bytes);
      SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);
      
      res = arad_pp_frwrd_ip_tcam_lpm_prefix_len_get(key.key.elk_fwd.m_mask, table_size_in_bytes, &prefix_len);
      SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);
 
      res = arad_pp_tcam_kbp_lpm_route_remove(
                unit,
                ARAD_IP_TCAM_TO_KBP_FRWRD_IP_TBL_ID(key.type),
                prefix_len,
                key.key.elk_fwd.m_data
              );

      SOC_SAND_CHECK_FUNC_RESULT(res, 30, exit);  
    }else{
        SOC_SAND_EXIT_AND_SEND_ERROR( "error in arad_pp_frwrd_ip_tcam_route_kbp_remove_unsafe()",key.type,0);
    }
    
exit:
  SOC_SAND_EXIT_AND_SEND_ERROR( "error in arad_pp_frwrd_ip_tcam_route_kbp_remove_unsafe()",0,0);
}

/* Build the buffer for payload - then encode according to KBP expectations */
uint32
  arad_pp_tcam_route_kbp_payload_buffer_encode(
    SOC_SAND_IN  int                     unit,
    SOC_SAND_IN  uint32                     table_payload_in_bytes,
    SOC_SAND_IN  uint32                     *asso_data_buffer,
    SOC_SAND_OUT uint8                      *asso_data
  )
{
    uint32
        byte_ndx,
        byte_array_idx,
        fld_val;

    SOC_SAND_INIT_ERROR_DEFINITIONS(0);

    SOC_SAND_CHECK_NULL_INPUT(asso_data_buffer);
    SOC_SAND_CHECK_NULL_INPUT(asso_data);

    sal_memset(asso_data, 0x0, sizeof(uint8) * SOC_DPP_TCAM_ACTION_ELK_KBP_GET_LEN_BYTES(unit));

    /*
     * Transform according to KBP expectations: 
     *  - Array indexation from max size-1 (e.g. 3 when the payload size is 4 Bytes) to 0
     */
    for (byte_ndx = 0; byte_ndx < table_payload_in_bytes; ++byte_ndx)
    {
        byte_array_idx = table_payload_in_bytes - byte_ndx - 1;
        /* Build data */
        fld_val = 0;
        SHR_BITCOPY_RANGE(&fld_val, 0, asso_data_buffer, (SOC_SAND_NOF_BITS_IN_BYTE * byte_ndx), SOC_SAND_NOF_BITS_IN_BYTE);
        asso_data[byte_array_idx] = (uint8) (fld_val & 0xFF);
    }

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR( "error in arad_pp_frwrd_ip_tcam_route_to_kbp_payload_buffer_encode()",0,0);
}


/* Build the buffer for payload - then encode according to KBP expectations */
uint32
  arad_pp_kbp_route_kbp_payload_buffer_encode(
    SOC_SAND_IN  int                     unit,
    SOC_SAND_IN  uint32                     table_payload_in_bytes,
    SOC_SAND_IN  uint32                     *asso_data_buffer,
    SOC_SAND_OUT uint8                      *asso_data
  )
{
    uint32
        byte_ndx,
        byte_array_idx,
        fld_val;

    SOC_SAND_INIT_ERROR_DEFINITIONS(0);

    SOC_SAND_CHECK_NULL_INPUT(asso_data_buffer);
    SOC_SAND_CHECK_NULL_INPUT(asso_data);

    sal_memset(asso_data, 0x0, sizeof(uint8) * SOC_DPP_TCAM_ACTION_ELK_KBP_GET_LEN_BYTES(unit));

    /*
     * Transform according to KBP expectations: 
     *  - Array indexation from max size-1 (e.g. 3 when the payload size is 4 Bytes) to 0
     */
    for (byte_ndx = 0; byte_ndx < table_payload_in_bytes; ++byte_ndx)
    {
        byte_array_idx = table_payload_in_bytes - byte_ndx - 1;
        /* Build data */
        fld_val = 0;
        SHR_BITCOPY_RANGE(&fld_val, 0, asso_data_buffer, 32+(SOC_SAND_NOF_BITS_IN_BYTE * byte_ndx), SOC_SAND_NOF_BITS_IN_BYTE);
        asso_data[byte_array_idx] = (uint8) (fld_val & 0xFF);
    }

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR( "error in arad_pp_frwrd_ip_tcam_route_to_kbp_payload_buffer_encode()",0,0);
}

/* Build the buffer for payload - then encode according to KBP expectations */
uint32
  arad_pp_tcam_route_kbp_payload_buffer_decode(
    SOC_SAND_IN  int                     unit,
    SOC_SAND_IN  ARAD_KBP_FRWRD_IP_TBL_ID   frwrd_table_id,
    SOC_SAND_IN  uint8                      *asso_data,
    SOC_SAND_OUT uint32                     *asso_data_buffer
  )
{
    uint32
        byte_ndx,
        byte_array_idx,
        payload_size_in_bytes,
        table_size_in_bytes,
        res,
        fld_val;

    SOC_SAND_INIT_ERROR_DEFINITIONS(0);

    SOC_SAND_CHECK_NULL_INPUT(asso_data_buffer);
    SOC_SAND_CHECK_NULL_INPUT(asso_data);

    res = arad_kbp_table_size_get(unit, frwrd_table_id, &table_size_in_bytes, &payload_size_in_bytes); 
    SOC_SAND_CHECK_FUNC_RESULT(res,  71, exit);

    /*
     * Transform according to KBP expectations: 
     *  - Array indexation from max size-1 (e.g. 9 when the database size is 10 Bytes) to 0
     */
    for (byte_ndx = 0; byte_ndx < payload_size_in_bytes; ++byte_ndx)
    {
        byte_array_idx = payload_size_in_bytes - byte_ndx - 1;
        fld_val = asso_data[byte_array_idx];
        SHR_BITCOPY_RANGE(asso_data_buffer, (SOC_SAND_NOF_BITS_IN_BYTE * byte_ndx), &fld_val, 0, SOC_SAND_NOF_BITS_IN_BYTE);
    }

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR( "error in arad_pp_frwrd_ip_tcam_route_from_kbp_payload_buffer_decode()",0,0);
}

uint32
  arad_pp_tcam_route_kbp_get_block_unsafe(
    SOC_SAND_IN  int                                     unit,
    SOC_SAND_IN  ARAD_KBP_FRWRD_IP_TBL_ID                   frwrd_table_id,
    SOC_SAND_INOUT  SOC_PPC_IP_ROUTING_TABLE_RANGE          *block_range_key,
    SOC_SAND_OUT  ARAD_PP_LEM_ACCESS_KEY                    *route_key_data,
    SOC_SAND_OUT  ARAD_PP_LEM_ACCESS_KEY                    *route_key_mask,
    SOC_SAND_OUT ARAD_PP_LEM_ACCESS_PAYLOAD                 *payload,
    SOC_SAND_OUT uint32                                     *nof_entries
  )
{
    uint32
      res = SOC_SAND_OK,
      indx;
    ARAD_PP_IP_TCAM_ENTRY_KEY
      *keys = NULL;
    ARAD_TCAM_ACTION
      *actions = NULL;

    SOC_SAND_INIT_ERROR_DEFINITIONS(0);

    SOC_SAND_CHECK_NULL_INPUT(block_range_key);
    SOC_SAND_CHECK_NULL_INPUT(route_key_data);
    SOC_SAND_CHECK_NULL_INPUT(route_key_mask);
    SOC_SAND_CHECK_NULL_INPUT(payload);
    SOC_SAND_CHECK_NULL_INPUT(nof_entries);

	if ((block_range_key->entries_to_scan == 0) || (block_range_key->entries_to_act == 0)) {
		ARAD_DO_NOTHING_AND_EXIT;
	}

    ARAD_ALLOC_ANY_SIZE(keys, ARAD_PP_IP_TCAM_ENTRY_KEY, block_range_key->entries_to_act, "keys arad_pp_frwrd_ip_tcam_route_kbp_get_block_unsafe");
    ARAD_ALLOC_ANY_SIZE(actions, ARAD_TCAM_ACTION, block_range_key->entries_to_act, "actions arad_pp_frwrd_ip_tcam_route_kbp_get_block_unsafe");

    res = arad_pp_tcam_route_kbp_hw_get_block_unsafe(
             unit,
             frwrd_table_id,
             block_range_key,
             keys,
             actions,
             nof_entries
           );

    SOC_SAND_CHECK_FUNC_RESULT(res, 60, exit);

    for(indx = 0; indx < *nof_entries; ++indx)
    {
        /* Convert the entries */
        res = arad_pp_kbp_route_buffer_decode(
                unit,
                frwrd_table_id,
                keys[indx].key.elk_fwd.m_data,
                keys[indx].key.elk_fwd.m_mask,
                &route_key_data[indx],
                &route_key_mask[indx]
             );
        SOC_SAND_CHECK_FUNC_RESULT(res, 70, exit);

        /* Translate the actions */
        res = arad_pp_kbp_route_payload_decode(
                unit,
                frwrd_table_id,
                0 /* lookup_id */,
                &(actions[indx].elk_ad_value[0]),
                &payload[indx]
              );
        SOC_SAND_CHECK_FUNC_RESULT(res, 80, exit); 
    }


exit:
  if(keys != NULL)
  {
      soc_sand_os_free(keys);
  }
  if(actions != NULL)
  {
      soc_sand_os_free(actions);
  }
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_frwrd_ip_tcam_route_kbp_get_block_unsafe()", 0, 0);
}

#endif /* defined(INCLUDE_KBP) && !defined(BCM_88030) */

/* } */

#include <soc/dpp/SAND/Utils/sand_footer.h>

#endif /* of #if defined(BCM_88650_A0) */

