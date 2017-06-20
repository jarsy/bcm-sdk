/*
* $Id: dnx_wb_engine.h,v 1.58 Broadcom SDK $
* $Copyright: (c) 2016 Broadcom.
* Broadcom Proprietary and Confidential. All rights reserved.$
*
* 
*/ 
#ifndef _SOC_DNX_WB_ENGINE_H_
#define _SOC_DNX_WB_ENGINE_H_

#include <soc/dnx/legacy/port_sw_db.h>

#include <soc/dnx/legacy/port_sw_db.h>
/* #include <soc/dnx/legacy/wb_db_hash_tbl.h> */
#include <soc/dnx/legacy/port_map.h>

#include <soc/dnx/legacy/TMC/tmc_api_general.h>
#include <soc/dnx/legacy/ARAD/arad_sw_db.h>

#include <soc/dnx/legacy/dnx_wb_engine_defs.h>
#include <shared/bitop.h>


#define SOC_DNX_WB_ENGINE_DECLARATIONS_BEGIN\
    switch (buffer_id) {\
        case SOC_DNX_OLD_BUFFERS_DECS:\
            /* old buffers are handled in deprecated code */\
            break;\
        case SOC_DNX_WB_ENGINE_NOF_BUFFERS:\
            return SOC_E_INTERNAL;

#define SOC_DNX_WB_ENGINE_DECLARATIONS_END\
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

#define SOC_DNX_WB_ENGINE_ADD_BUFF(_buff, _buff_string, _upgrade_func, _version, _is_added_after_release)\
        break;\
    case _buff:\
        SOC_WB_ENGINE_ADD_BUFF(SOC_WB_ENGINE_PRIMARY, _buff, _buff_string, _upgrade_func, NULL, _version, 0x1 /*stored only in scache buffer*/, _is_added_after_release)\
        DNXC_IF_ERR_EXIT(rv);\
        SOC_DNX_WB_ENGINE_DYNAMIC_VAR_STATE_GET;

/* this is used only for data structure buffers that do hold data on memory outside of scache buffer */
#define SOC_DNX_WB_ENGINE_ADD_BUFF_WITH_MEMORY_DUPLICATIONS(_buff, _buff_string, _upgrade_func, _version, _is_added_after_release)\
        break;\
    case _buff:\
        SOC_WB_ENGINE_ADD_BUFF(SOC_WB_ENGINE_PRIMARY, _buff, _buff_string, _upgrade_func, NULL, _version, 0x0 /*copy is stored in scache buffer*/, _is_added_after_release)\
        DNXC_IF_ERR_EXIT(rv);\
        SOC_DNX_WB_ENGINE_DYNAMIC_VAR_STATE_GET;

/* ---------most-usefull----------- */
#define SOC_DNX_WB_ENGINE_ADD_VAR(_var, _var_string, _buffer, _data_size, _version_added)\
    SOC_WB_ENGINE_ADD_VAR(SOC_WB_ENGINE_PRIMARY, _var, _var_string, _buffer, _data_size, NULL, _version_added)\
    DNXC_IF_ERR_EXIT(rv);

#define SOC_DNX_WB_ENGINE_ADD_ARR(_var, _var_string, _buffer, _data_size, _arr_length, _version_added)\
    SOC_WB_ENGINE_ADD_ARR(SOC_WB_ENGINE_PRIMARY, _var, _var_string, _buffer, _data_size, NULL, _arr_length, _version_added)\
    DNXC_IF_ERR_EXIT(rv);

#define SOC_DNX_WB_ENGINE_ADD_2D_ARR(_var, _var_string, _buffer, _data_size, _outer_arr_length, _inner_arr_length, _version_added)\
    SOC_WB_ENGINE_ADD_2D_ARR(SOC_WB_ENGINE_PRIMARY, _var, _var_string, _buffer, _data_size, NULL, _outer_arr_length, _inner_arr_length, _version_added)\
    DNXC_IF_ERR_EXIT(rv);
/* -------------------------------- */





/********************************************************************************************/
/* set/get macros to be used to set/get vars/array entries that are handled by wb_engine    */
/* retun SOC_E_ERRORS                                                                       */
/********************************************************************************************/

#define SOC_DNX_WB_ENGINE_SET_DBL_ARR(unit, _var_id, _data_ptr, _outer_idx, _inner_idx)\
    SOC_WB_ENGINE_SET_DBL_ARR(unit, SOC_WB_ENGINE_PRIMARY, _var_id, _data_ptr, _outer_idx, _inner_idx)

#define SOC_DNX_WB_ENGINE_GET_DBL_ARR(unit, _var_id, _data_ptr, _outer_idx, _inner_idx)\
    SOC_WB_ENGINE_GET_DBL_ARR(unit, SOC_WB_ENGINE_PRIMARY, _var_id, _data_ptr, _outer_idx, _inner_idx)

#define SOC_DNX_WB_ENGINE_SET_ARR(unit, _var_id, _data_ptr, _idx)\
    SOC_WB_ENGINE_SET_ARR(unit, SOC_WB_ENGINE_PRIMARY, _var_id, _data_ptr, _idx)

#define SOC_DNX_WB_ENGINE_GET_ARR(unit, _var_id, _data_ptr, _idx)\
    SOC_WB_ENGINE_GET_ARR(unit, SOC_WB_ENGINE_PRIMARY, _var_id, _data_ptr, _idx)

#define SOC_DNX_WB_ENGINE_SET_VAR(unit, _var_id, _data_ptr)\
    SOC_WB_ENGINE_SET_VAR(unit, SOC_WB_ENGINE_PRIMARY, _var_id, _data_ptr)

#define SOC_DNX_WB_ENGINE_GET_VAR(unit, _var_id, _data_ptr)\
    SOC_WB_ENGINE_GET_VAR(unit, SOC_WB_ENGINE_PRIMARY, _var_id, _data_ptr)


/* set/get macros for changing multiple array entries at once */
#define SOC_DNX_WB_ENGINE_MEMCPY_ARR(unit, _var_id, _data_ptr, _inner_idx, _cpy_length) \
    soc_wb_engine_array_range_set_or_get(unit, SOC_WB_ENGINE_PRIMARY, (_var_id), (_inner_idx), (_cpy_length), 1, (uint8 *)(_data_ptr))

#define SOC_DNX_WB_ENGINE_MEMCPY_ARR_GET(unit, _var_id, _data_ptr, _inner_idx, _cpy_length) \
    soc_wb_engine_array_range_set_or_get(unit, SOC_WB_ENGINE_PRIMARY, (_var_id), (_inner_idx), (_cpy_length), 0, (uint8 *)(_data_ptr))

#define SOC_DNX_WB_ENGINE_MEMSET_ARR(unit, _var_id, _val) \
    soc_wb_engine_array_set(unit, SOC_WB_ENGINE_PRIMARY, (_var_id), (_val))


/* Table init functions */
int soc_dnx_wb_engine_init_buffer(int unit, int buffer_id);
int soc_dnx_wb_engine_deinit(int unit);
int soc_dnx_wb_engine_init(int unit);
int soc_dnx_wb_engine_sync(int unit);


/************************************************************************************************/
/*                                                                                              */
/*                              ADD BUFFERS and VARIABLES here                                  */
/*                      above this box - infrastructure implementation                          */
/*                        below this box - add your buffers\vars info                           */
/*                                                                                              */
/************************************************************************************************/


/* need to declare the buffers and variables in the two enums below, init them at dnx_wb_db.c */

/* original variables declaration -
   the variables\structs that hold the real data,
   pointed to by the wb_engine */
extern JER2_ARAD_SW_DB Jer2_arad_sw_db;
  
#ifdef BCM_WARM_BOOT_SUPPORT   
/* extern dnx_wb_hash_tbl_data_t wb_hash_tbl_data[DNX_SAND_MAX_DEVICE][SOC_DNX_WB_HASH_TBLS_NUM];  */
#endif /*BCM_WARM_BOOT_SUPPORT*/                                                           
                                                            
/* 
 * buffers - 
 * unique buffer names, strictly numbered enum. numbers should be only added, never changed. 
 * these numbers are used as unique id for the buffer to be saved on external storage.
 */
typedef enum
{
    SOC_DNX_WB_ENGINE_NOF_BUFFERS = 1

} SOC_DNX_WB_ENGINE_BUFFER;
/* 
 * variables, 
 * when adding a new var, always add it as the last var of his buffer variables!
 */
typedef enum
{

    /*always last*/
    SOC_DNX_WB_ENGINE_VAR_NOF_VARS = 1

} SOC_DNX_WB_ENGINE_VAR;
#endif /*_SOC_DNX_WB_ENGINE_H_*/
