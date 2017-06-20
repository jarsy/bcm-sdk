/*
* $Id: dpp_wb_engine.h,v 1.58 Broadcom SDK $
* $Copyright: (c) 2016 Broadcom.
* Broadcom Proprietary and Confidential. All rights reserved.$
*
* 
*/ 
#ifndef _SOC_DPP_WB_ENGINE_H_
#define _SOC_DPP_WB_ENGINE_H_

#ifdef BCM_ARAD_SUPPORT
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_sw_db.h>
#include <soc/dpp/port_sw_db.h>
/* #include <soc/dpp/wb_db_hash_tbl.h> */
#endif /*BCM_ARAD_SUPPORT*/

#include <soc/dpp/PPD/ppd_api_oam.h>
#include <soc/dpp/port_sw_db.h>
/* #include <soc/dpp/wb_db_hash_tbl.h> */
#include <soc/dpp/port_map.h>

#ifdef BCM_PETRA_SUPPORT
#include <soc/dpp/TMC/tmc_api_general.h>
#endif /*BCM_PETRA_SUPPORT*/

#include <soc/dpp/ARAD/arad_tcam_mgmt.h>
#include <soc/dpp/dpp_wb_engine_defs.h>
#include <shared/bitop.h>
#include <bcm_int/dpp/ipmc.h>


#define SOC_DPP_WB_ENGINE_DECLARATIONS_BEGIN\
    switch (buffer_id) {\
        case SOC_DPP_OLD_BUFFERS_DECS:\
            /* old buffers are handled in deprecated code */\
            break;\
        case SOC_DPP_WB_ENGINE_NOF_BUFFERS:\
            return SOC_E_INTERNAL;

#define SOC_DPP_WB_ENGINE_DECLARATIONS_END\
            break;\
        default:\
            return SOC_E_INTERNAL;\
    }


/********************************/
/* dynamic vars addition macros */
/********************************/



/********************************/
/* static vars addition macros  */
/********************************/

#define SOC_DPP_WB_ENGINE_ADD_BUFF(_buff, _buff_string, _upgrade_func, _version, _is_added_after_release)\
        break;\
    case _buff:\
        SOC_WB_ENGINE_ADD_BUFF(SOC_WB_ENGINE_PRIMARY, _buff, _buff_string, _upgrade_func, NULL, _version, 0x1 /*stored only in scache buffer*/, _is_added_after_release)\
        SOCDNX_IF_ERR_EXIT(rv);\
        SOC_DPP_WB_ENGINE_DYNAMIC_VAR_STATE_GET;

/* this is used only for data structure buffers that do hold data on memory outside of scache buffer */
#define SOC_DPP_WB_ENGINE_ADD_BUFF_WITH_MEMORY_DUPLICATIONS(_buff, _buff_string, _upgrade_func, _version, _is_added_after_release)\
        break;\
    case _buff:\
        SOC_WB_ENGINE_ADD_BUFF(SOC_WB_ENGINE_PRIMARY, _buff, _buff_string, _upgrade_func, NULL, _version, 0x0 /*copy is stored in scache buffer*/, _is_added_after_release)\
        SOCDNX_IF_ERR_EXIT(rv);\
        SOC_DPP_WB_ENGINE_DYNAMIC_VAR_STATE_GET;

/* ---------most-usefull----------- */
#define SOC_DPP_WB_ENGINE_ADD_VAR(_var, _var_string, _buffer, _data_size, _version_added)\
    SOC_WB_ENGINE_ADD_VAR(SOC_WB_ENGINE_PRIMARY, _var, _var_string, _buffer, _data_size, NULL, _version_added)\
    SOCDNX_IF_ERR_EXIT(rv);

#define SOC_DPP_WB_ENGINE_ADD_ARR(_var, _var_string, _buffer, _data_size, _arr_length, _version_added)\
    SOC_WB_ENGINE_ADD_ARR(SOC_WB_ENGINE_PRIMARY, _var, _var_string, _buffer, _data_size, NULL, _arr_length, _version_added)\
    SOCDNX_IF_ERR_EXIT(rv);

#define SOC_DPP_WB_ENGINE_ADD_2D_ARR(_var, _var_string, _buffer, _data_size, _outer_arr_length, _inner_arr_length, _version_added)\
    SOC_WB_ENGINE_ADD_2D_ARR(SOC_WB_ENGINE_PRIMARY, _var, _var_string, _buffer, _data_size, NULL, _outer_arr_length, _inner_arr_length, _version_added)\
    SOCDNX_IF_ERR_EXIT(rv);
/* -------------------------------- */





/********************************************************************************************/
/* set/get macros to be used to set/get vars/array entries that are handled by wb_engine    */
/* retun SOC_E_ERRORS                                                                       */
/********************************************************************************************/

#define SOC_DPP_WB_ENGINE_SET_DBL_ARR(unit, _var_id, _data_ptr, _outer_idx, _inner_idx)\
    SOC_WB_ENGINE_SET_DBL_ARR(unit, SOC_WB_ENGINE_PRIMARY, _var_id, _data_ptr, _outer_idx, _inner_idx)

#define SOC_DPP_WB_ENGINE_GET_DBL_ARR(unit, _var_id, _data_ptr, _outer_idx, _inner_idx)\
    SOC_WB_ENGINE_GET_DBL_ARR(unit, SOC_WB_ENGINE_PRIMARY, _var_id, _data_ptr, _outer_idx, _inner_idx)

#define SOC_DPP_WB_ENGINE_SET_ARR(unit, _var_id, _data_ptr, _idx)\
    SOC_WB_ENGINE_SET_ARR(unit, SOC_WB_ENGINE_PRIMARY, _var_id, _data_ptr, _idx)

#define SOC_DPP_WB_ENGINE_GET_ARR(unit, _var_id, _data_ptr, _idx)\
    SOC_WB_ENGINE_GET_ARR(unit, SOC_WB_ENGINE_PRIMARY, _var_id, _data_ptr, _idx)

#define SOC_DPP_WB_ENGINE_SET_VAR(unit, _var_id, _data_ptr)\
    SOC_WB_ENGINE_SET_VAR(unit, SOC_WB_ENGINE_PRIMARY, _var_id, _data_ptr)

#define SOC_DPP_WB_ENGINE_GET_VAR(unit, _var_id, _data_ptr)\
    SOC_WB_ENGINE_GET_VAR(unit, SOC_WB_ENGINE_PRIMARY, _var_id, _data_ptr)


/* set/get macros for changing multiple array entries at once */
#define SOC_DPP_WB_ENGINE_MEMCPY_ARR(unit, _var_id, _data_ptr, _inner_idx, _cpy_length) \
    soc_wb_engine_array_range_set_or_get(unit, SOC_WB_ENGINE_PRIMARY, (_var_id), (_inner_idx), (_cpy_length), 1, (uint8 *)(_data_ptr))

#define SOC_DPP_WB_ENGINE_MEMCPY_ARR_GET(unit, _var_id, _data_ptr, _inner_idx, _cpy_length) \
    soc_wb_engine_array_range_set_or_get(unit, SOC_WB_ENGINE_PRIMARY, (_var_id), (_inner_idx), (_cpy_length), 0, (uint8 *)(_data_ptr))

#define SOC_DPP_WB_ENGINE_MEMSET_ARR(unit, _var_id, _val) \
    soc_wb_engine_array_set(unit, SOC_WB_ENGINE_PRIMARY, (_var_id), (_val))

#ifdef BCM_ARAD_SUPPORT

/* Table init functions */
int soc_dpp_wb_engine_state_init_port(int unit);

int soc_dpp_wb_engine_Arad_pp_sw_db_get(ARAD_PP_SW_DB *sw_db);

#endif /*BCM_ARAD_SUPPORT*/

/* Other table structs */
/* Advanced VLAN editing action SW table structures */
typedef struct _bcm_dpp_vlan_translate_tag_action_s {
    bcm_vlan_action_t vid_action;
    bcm_vlan_action_t pcp_action;
    bcm_vlan_tpid_action_t tpid_action;
    uint16 tpid_val;
} _bcm_dpp_vlan_translate_tag_action_t;

typedef struct _bcm_dpp_vlan_translate_action_s {
    _bcm_dpp_vlan_translate_tag_action_t outer;
    _bcm_dpp_vlan_translate_tag_action_t inner;
    uint32 packet_is_tagged_after_eve;
} _bcm_dpp_vlan_translate_action_t;


int soc_dpp_wb_engine_init_buffer(int unit, int buffer_id);
int soc_dpp_wb_engine_deinit(int unit);
int soc_dpp_wb_engine_init(int unit);
int soc_dpp_wb_engine_sync(int unit);


/************************************************************************************************/
/*                                                                                              */
/*                              ADD BUFFERS and VARIABLES here                                  */
/*                      above this box - infrastructure implementation                          */
/*                        below this box - add your buffers\vars info                           */
/*                                                                                              */
/************************************************************************************************/

#define DPP_WB_ENGINE_NOF_LIFS (SOC_DPP_CONFIG(unit))->l2.nof_lifs

/* need to declare the buffers and variables in the two enums below, init them at dpp_wb_db.c */

/* original variables declaration -
   the variables\structs that hold the real data,
   pointed to by the wb_engine */
#ifdef BCM_ARAD_SUPPORT
extern ARAD_SW_DB Arad_sw_db;
  
#ifdef BCM_WARM_BOOT_SUPPORT   
/* extern dpp_wb_hash_tbl_data_t wb_hash_tbl_data[SOC_SAND_MAX_DEVICE][SOC_DPP_WB_HASH_TBLS_NUM];  */
#endif /*BCM_WARM_BOOT_SUPPORT*/                                                           
#endif /*BCM_ARAD_SUPPORT*/                                                             
  
/* 
 * buffers - 
 * unique buffer names, strictly numbered enum. numbers should be only added, never changed. 
 * these numbers are used as unique id for the buffer to be saved on external storage.
 */
typedef enum
{
    SOC_DPP_WB_ENGINE_BUFFER_ARAD_PP_SW_DB_IPV4_INFO                 =  8,
    SOC_DPP_WB_ENGINE_BUFFER_ARAD_PP_RIF_TO_LIF_GROUP_MAP            = 42,
  /*NOTE THAT 252 IS THE HIGHEST BUFFER INDEX CURRENTLY ALLOWED!     = 255,*/

    SOC_DPP_WB_ENGINE_NOF_BUFFERS                                    

} SOC_DPP_WB_ENGINE_BUFFER;


/* 
 * variables, 
 * when adding a new var, always add it as the last var of his buffer variables!
 */
typedef enum
{
    /* SOC_DPP_WB_ENGINE_BUFFER_ARAD_PP_SW_DB_IPV4_INFO buffer */
    COMPLEX_DS_ARR_MEM_ALLOCATOR(SOC_DPP_WB_ENGINE_BUFFER_ARAD_PP_SW_DB_IPV4_INFO_ARR_MEM_ALLOCATOR_1),
    COMPLEX_DS_ARR_MEM_ALLOCATOR(SOC_DPP_WB_ENGINE_BUFFER_ARAD_PP_SW_DB_IPV4_INFO_ARR_MEM_ALLOCATOR_2),
    COMPLEX_DS_ARR_MEM_ALLOCATOR(SOC_DPP_WB_ENGINE_BUFFER_ARAD_PP_SW_DB_IPV4_INFO_ARR_MEM_ALLOCATOR_3),
    COMPLEX_DS_ARR_MEM_ALLOCATOR(SOC_DPP_WB_ENGINE_BUFFER_ARAD_PP_SW_DB_IPV4_INFO_ARR_MEM_ALLOCATOR_4),
    COMPLEX_DS_ARR_MEM_ALLOCATOR(SOC_DPP_WB_ENGINE_BUFFER_ARAD_PP_SW_DB_IPV4_INFO_ARR_MEM_ALLOCATOR_5),
    COMPLEX_DS_ARR_MEM_ALLOCATOR(SOC_DPP_WB_ENGINE_BUFFER_ARAD_PP_SW_DB_IPV4_INFO_ARR_MEM_ALLOCATOR_6),

    COMPLEX_DS_ARR_MEM_ALLOCATOR(SOC_DPP_WB_ENGINE_BUFFER_ARAD_PP_SW_DB_IPV4_INFO_GROUP_MEM_LL_1),
    COMPLEX_DS_ARR_MEM_ALLOCATOR(SOC_DPP_WB_ENGINE_BUFFER_ARAD_PP_SW_DB_IPV4_INFO_GROUP_MEM_LL_2),
    COMPLEX_DS_ARR_MEM_ALLOCATOR(SOC_DPP_WB_ENGINE_BUFFER_ARAD_PP_SW_DB_IPV4_INFO_GROUP_MEM_LL_3),
    COMPLEX_DS_ARR_MEM_ALLOCATOR(SOC_DPP_WB_ENGINE_BUFFER_ARAD_PP_SW_DB_IPV4_INFO_GROUP_MEM_LL_4),
    COMPLEX_DS_ARR_MEM_ALLOCATOR(SOC_DPP_WB_ENGINE_BUFFER_ARAD_PP_SW_DB_IPV4_INFO_GROUP_MEM_LL_5),
    COMPLEX_DS_ARR_MEM_ALLOCATOR(SOC_DPP_WB_ENGINE_BUFFER_ARAD_PP_SW_DB_RIF_TO_LIF_GROUP_MEM_LL),

    COMPLEX_DS_PAT_TREE_AGREGATION(SOC_DPP_WB_ENGINE_BUFFER_ARAD_PP_SW_DB_IPV4_INFO_LPMS),

    /*always last*/
    SOC_DPP_WB_ENGINE_VAR_NOF_VARS

} SOC_DPP_WB_ENGINE_VAR;

#endif /*_SOC_DPP_WB_ENGINE_H_*/
