/* 
* $Id: dnx_wb_engine_defs.h,v 1.0 Broadcom SDK $ 
* $Copyright: (c) 2016 Broadcom.
* Broadcom Proprietary and Confidential. All rights reserved.$
*/
#ifndef _SOC_DNX_WB_ENGINE_DEFS_H_
#define _SOC_DNX_WB_ENGINE_DEFS_H_

#include <soc/wb_engine.h>
extern dnx_port_unit_info_t        dnx_ports_unit_info[SOC_MAX_NUM_DEVICES];
extern dnx_port_core_info_t        dnx_core_info[SOC_MAX_NUM_DEVICES][SOC_DNX_DEFS_MAX(NOF_CORES)];

#define SOC_DNX_OLD_BUFFERS_DECS\
         SOC_DNX_WB_ENGINE_BUFFER_JER2_ARAD_PP_SW_DB_IPV4_INFO:\
    case SOC_DNX_WB_ENGINE_BUFFER_JER2_ARAD_PP_RIF_TO_LIF_GROUP_MAP

#define DNX_COMPLEX_DS_ARR_MEM_ALLOCATOR(_wb_engine_var) \
    _wb_engine_var,                                                     \
    _wb_engine_var##_LAST = _wb_engine_var + (DNX_WB_ENGINE_MEM_ALLOCATOR_INNER_VARS_NUM - 1)


#define DNX_COMPLEX_DS_OCC_BM(_wb_engine_var) \
    _wb_engine_var,                                                     \
    _wb_engine_var##_LAST = _wb_engine_var + (DNX_WB_ENGINE_OCC_BM_INNER_VARS_NUM - 1)
#define DNX_COMPLEX_DS_PAT_TREE_AGREGATION(_wb_engine_var) \
    _wb_engine_var,                                                     \
    _wb_engine_var##_LAST = _wb_engine_var + (DNX_WB_ENGINE_PAT_TREE_AGREGATION_INNER_VARS_NUM - 1)



#define SOC_DNX_WB_ENGINE_VAR_NONE -1

#define SOC_DNX_WB_ENGINE (SOC_WB_ENGINE_PRIMARY)

typedef enum
{
    /* MEM_ALLOCATOR complex data structures, internal simple structures */

    /* simple variables in JER2_ARAD_PP_ARR_MEM_ALLOCATOR_T */
    DNX_WB_ENGINE_MEM_ALLOCATOR_FREE_LIST, 
    DNX_WB_ENGINE_MEM_ALLOCATOR_CACHE_ENABLED, 
    DNX_WB_ENGINE_MEM_ALLOCATOR_FREE_LIST_CACHE, 
    DNX_WB_ENGINE_MEM_ALLOCATOR_NOF_UPDATES, 

    /* arrays in JER2_ARAD_PP_ARR_MEM_ALLOCATOR_T */
    DNX_WB_ENGINE_MEM_ALLOCATOR_ARRAY,
    DNX_WB_ENGINE_MEM_ALLOCATOR_MEM_SHADOW,
    DNX_WB_ENGINE_MEM_ALLOCATOR_ARRAY_CACHE,
    DNX_WB_ENGINE_MEM_ALLOCATOR_MEM_SHADOW_CACHE,
    DNX_WB_ENGINE_MEM_ALLOCATOR_UPDATE_INDEXES,

    DNX_WB_ENGINE_MEM_ALLOCATOR_INNER_VARS_NUM

} SOC_DNX_WB_ENGINE_COMPLEX_DS_MEM_ALLOCATOR_INNER_VAR;

typedef enum
{
    /* GROUP_MEM_LL complex data structures, internal simple structures */

    /* simple variables in DNX_SAND_GROUP_MEM_LL_T */
    DNX_WB_ENGINE_GROUP_MEM_LL_CACHE_ENABLED, 

    /* arrays in JER2_ARAD_PP_ARR_GROUP_MEM_LL_T */
    DNX_WB_ENGINE_GROUP_MEM_LL_GROUPS,
    DNX_WB_ENGINE_GROUP_MEM_LL_MEMBERS,
    DNX_WB_ENGINE_GROUP_MEM_LL_GROUPS_CACHE,
    DNX_WB_ENGINE_GROUP_MEM_LL_MEMBERS_CACHE,

    DNX_WB_ENGINE_GROUP_MEM_LL_INNER_VARS_NUM

} SOC_DNX_WB_ENGINE_COMPLEX_DS_GROUP_MEM_LL_INNER_VAR;

typedef enum
{
    /* OCC_BM complex data structures, internal simple structures */

    /* simple variables in DNX_SAND_OCC_BM_T */
    DNX_WB_ENGINE_OCC_BM_CACHE_ENABLED, 

    /* arrays in DNX_SAND_OCC_BM_T */
    DNX_WB_ENGINE_OCC_BM_LEVELS_BUFFER, 
    DNX_WB_ENGINE_OCC_BM_LEVELS_CACHE_BUFFER, 

    DNX_WB_ENGINE_OCC_BM_INNER_VARS_NUM

} SOC_DNX_WB_ENGINE_COMPLEX_DS_OCC_BM_INNER_VAR;


typedef enum
{
    /* PAT_TREE_AGREGATION complex data structures, internal simple structures */

    /* PAT_TREE_AGREGATION is a collection of pat_tree that some structures are allocated only once, */
    /* i.e all of the pat_trees use pat_tree index 0 allocations */

    /* simple variables allocated to each pat_tree (thus added as one array for all the pat_trees) */
    DNX_WB_ENGINE_PAT_TREE_AGREGATION_ROOT, 
    DNX_WB_ENGINE_PAT_TREE_AGREGATION_CACHE_ENABLED, 
    DNX_WB_ENGINE_PAT_TREE_AGREGATION_ROOT_CACHE, 
    DNX_WB_ENGINE_PAT_TREE_AGREGATION_CACHE_CHANGE_HEAD, 
    DNX_WB_ENGINE_PAT_TREE_AGREGATION_CURRENT_NODE_PLACE, 

    /* 1 common allocation for all pat_trees */
    DNX_WB_ENGINE_PAT_TREE_AGREGATION_TREE_MEMORY, 
    DNX_COMPLEX_DS_OCC_BM(DNX_WB_ENGINE_PAT_TREE_AGREGATION_MEMORY_USE), 
    DNX_WB_ENGINE_PAT_TREE_AGREGATION_TREE_MEMORY_CACHE, 

    DNX_WB_ENGINE_PAT_TREE_AGREGATION_INNER_VARS_NUM

} SOC_DNX_WB_ENGINE_COMPLEX_DS_PAT_TREE_AGREGATION_INNER_VAR;

/* !!!!!!!!!!!!!!!!!!!!!! */
/* deprecated definitions */
/* !!!!!!!!!!!!!!!!!!!!!! */
#define SOC_DNX_WB_ENGINE_ADD_BUFF_DEPRECATED(_buff, _buff_string, _upgrade_func, _version)\
    SOC_WB_ENGINE_ADD_BUFF(SOC_WB_ENGINE_PRIMARY, _buff, _buff_string, _upgrade_func, NULL, _version, 0x0 /*not only copy*/, SOC_WB_ENGINE_PRE_RELEASE)\
    DNXC_IF_ERR_EXIT(rv);

#define SOC_DNX_WB_ENGINE_ADD_VAR_DEPRECATED(_var, _var_string, _buffer, _data_size, _orig_data_ptr, _version_added)\
    SOC_WB_ENGINE_ADD_VAR(SOC_WB_ENGINE_PRIMARY, _var, _var_string, _buffer, _data_size, _orig_data_ptr, _version_added)\
    DNXC_IF_ERR_EXIT(rv);

#define SOC_DNX_WB_ENGINE_ADD_ARR_DEPRECATED(_var, _var_string, _buffer, _data_size, _orig_data_ptr, _arr_length, _version_added)\
    SOC_WB_ENGINE_ADD_ARR(SOC_WB_ENGINE_PRIMARY, _var, _var_string, _buffer, _data_size, _orig_data_ptr, _arr_length, _version_added)\
    DNXC_IF_ERR_EXIT(rv);

/*******************************************/
/* deprecated dynamic vars addition macros */
/*******************************************/
#define SOC_DNX_WB_ENGINE_ADD_ARR_DEPRECATED_WITH_OFFSET(_var, _var_string, _buffer, _data_size, _orig_data_ptr, _arr_length, _inner_jump, _version_added) \
    SOC_WB_ENGINE_ADD_VAR_WITH_FEATURES(SOC_WB_ENGINE_PRIMARY, _var, _var_string, _buffer, _data_size, _orig_data_ptr, 1, _arr_length, 0xffffffff, _inner_jump, _version_added, 0xff, NULL)

/* !!!!!!!!!!!!!!!!!!!!!! */




/************************/
/* complex data structs */
/************************/
#define SOC_DNX_WB_ENGINE_ADD_COMPLEX_DS_ARR_MEM_ALLOCATOR(_var, _var_string, _buffer, _mem_allocator, _entry_size, _nof_entries, _support_caching, _version_added) \
    if(_buffer == buffer_id && _support_caching == TRUE )               \
    {                                                                   \
        SOC_DNX_WB_ENGINE_ADD_VAR_DEPRECATED(_var + DNX_WB_ENGINE_MEM_ALLOCATOR_FREE_LIST_CACHE, \
                                  _var_string,                          \
                                  _buffer,                              \
                                  sizeof(JER2_ARAD_PP_ARR_MEM_ALLOCATOR_PTR), \
                                  &(_mem_allocator.arr_mem_allocator_data.free_list_cache), \
                                  _version_added);                      \
        SOC_DNX_WB_ENGINE_ADD_ARR_DEPRECATED(_var + DNX_WB_ENGINE_MEM_ALLOCATOR_ARRAY_CACHE, \
                                  _var_string,                          \
                                  _buffer,                              \
                                  sizeof(JER2_ARAD_PP_ARR_MEM_ALLOCATOR_ENTRY), \
                                  _mem_allocator.arr_mem_allocator_data.array_cache, \
                                  _nof_entries,                         \
                                  _version_added);                      \
        SOC_DNX_WB_ENGINE_ADD_ARR_DEPRECATED(_var + DNX_WB_ENGINE_MEM_ALLOCATOR_MEM_SHADOW_CACHE, \
                                  _var_string,                          \
                                  _buffer,                              \
                                  sizeof(uint32),                       \
                                  _mem_allocator.arr_mem_allocator_data.mem_shadow_cache, \
                                  _entry_size * _nof_entries,           \
                                  _version_added);                      \
        SOC_DNX_WB_ENGINE_ADD_ARR_DEPRECATED(_var + DNX_WB_ENGINE_MEM_ALLOCATOR_UPDATE_INDEXES, \
                                  _var_string,                          \
                                  _buffer,                              \
                                  sizeof(JER2_ARAD_PP_ARR_MEM_ALLOCATOR_ENTRY), \
                                  _mem_allocator.arr_mem_allocator_data.update_indexes, \
                                  _nof_entries,                         \
                                  _version_added);                      \
    }                                                                   \
    SOC_DNX_WB_ENGINE_ADD_VAR_DEPRECATED(_var + DNX_WB_ENGINE_MEM_ALLOCATOR_FREE_LIST, \
                              _var_string,                              \
                              _buffer,                                  \
                              sizeof(JER2_ARAD_PP_ARR_MEM_ALLOCATOR_PTR),    \
                              &(_mem_allocator.arr_mem_allocator_data.free_list), \
                              _version_added);                          \
    SOC_DNX_WB_ENGINE_ADD_VAR_DEPRECATED(_var + DNX_WB_ENGINE_MEM_ALLOCATOR_CACHE_ENABLED, \
                              _var_string,                              \
                              _buffer,                                  \
                              sizeof(uint8),                            \
                              &(_mem_allocator.arr_mem_allocator_data.cache_enabled), \
                              _version_added);                          \
    SOC_DNX_WB_ENGINE_ADD_VAR_DEPRECATED(_var + DNX_WB_ENGINE_MEM_ALLOCATOR_NOF_UPDATES, \
                              _var_string,                              \
                              _buffer,                                  \
                              sizeof(uint32),                           \
                              &(_mem_allocator.arr_mem_allocator_data.nof_updates), \
                              _version_added);                          \
    SOC_DNX_WB_ENGINE_ADD_ARR_DEPRECATED(_var + DNX_WB_ENGINE_MEM_ALLOCATOR_ARRAY,     \
                              _var_string,                              \
                              _buffer,                                  \
                              sizeof(JER2_ARAD_PP_ARR_MEM_ALLOCATOR_ENTRY),  \
                              _mem_allocator.arr_mem_allocator_data.array, \
                              _nof_entries,                             \
                              _version_added);                          \
    SOC_DNX_WB_ENGINE_ADD_ARR_DEPRECATED(_var + DNX_WB_ENGINE_MEM_ALLOCATOR_MEM_SHADOW, \
                              _var_string,                              \
                              _buffer,                                  \
                              sizeof(uint32),                           \
                              _mem_allocator.arr_mem_allocator_data.mem_shadow, \
                              _entry_size * _nof_entries,               \
                              _version_added)


#define SOC_DNX_WB_ENGINE_ADD_COMPLEX_DS_DB_GROUP_MEM_LL(_var, _var_string, _buffer, _group_mem_ll, _nof_groups, _nof_elements, _support_caching, _version_added) \
    if(_buffer == buffer_id && _support_caching == TRUE )               \
    {                                                                   \
        SOC_DNX_WB_ENGINE_ADD_ARR_DEPRECATED(_var + DNX_WB_ENGINE_GROUP_MEM_LL_GROUPS_CACHE, \
                                  _var_string,                          \
                                  _buffer,                              \
                                  sizeof(DNX_SAND_GROUP_MEM_LL_GROUP_ENTRY), \
                                  _group_mem_ll->group_members_data.groups_cache, \
                                  _nof_groups,                          \
                                  _version_added);                      \
        SOC_DNX_WB_ENGINE_ADD_ARR_DEPRECATED(_var + DNX_WB_ENGINE_GROUP_MEM_LL_MEMBERS_CACHE, \
                                  _var_string,                          \
                                  _buffer,                              \
                                  sizeof(DNX_SAND_GROUP_MEM_LL_MEMBER_ENTRY), \
                                  _group_mem_ll->group_members_data.members_cache, \
                                  _nof_elements,                        \
                                  _version_added);                      \
    }                                                                   \
    SOC_DNX_WB_ENGINE_ADD_VAR_DEPRECATED(_var + DNX_WB_ENGINE_GROUP_MEM_LL_CACHE_ENABLED, \
                              _var_string,                              \
                              _buffer,                                  \
                              sizeof(uint8),                            \
                              &(_group_mem_ll->group_members_data.cache_enabled), \
                              _version_added);                          \
    SOC_DNX_WB_ENGINE_ADD_ARR_DEPRECATED(_var + DNX_WB_ENGINE_GROUP_MEM_LL_GROUPS,     \
                              _var_string,                              \
                              _buffer,                                  \
                              sizeof(DNX_SAND_GROUP_MEM_LL_GROUP_ENTRY), \
                              _group_mem_ll->group_members_data.groups, \
                              _nof_groups,                              \
                              _version_added);                          \
    SOC_DNX_WB_ENGINE_ADD_ARR_DEPRECATED(_var + DNX_WB_ENGINE_GROUP_MEM_LL_MEMBERS,    \
                              _var_string,                              \
                              _buffer,                                  \
                              sizeof(DNX_SAND_GROUP_MEM_LL_MEMBER_ENTRY), \
                              _group_mem_ll->group_members_data.members, \
                              _nof_elements,                            \
                              _version_added)


#define SOC_DNX_WB_ENGINE_ADD_COMPLEX_DS_OCC_BM(_var, _var_string, _buffer, _occ_bm, _array_size, _support_cache, _version_added) \
    if(_buffer == buffer_id && _support_cache == TRUE )                 \
    {                                                                   \
        SOC_DNX_WB_ENGINE_ADD_ARR_DEPRECATED(_var + DNX_WB_ENGINE_OCC_BM_LEVELS_CACHE_BUFFER, \
                                  _var_string,                          \
                                  _buffer,                              \
                                  sizeof(uint8),                        \
                                  _occ_bm->levels_cache_buffer,         \
                                  _array_size,                          \
                                  _version_added);                      \
    }                                                                   \
    SOC_DNX_WB_ENGINE_ADD_VAR_DEPRECATED(_var + DNX_WB_ENGINE_OCC_BM_CACHE_ENABLED,    \
                              _var_string,                              \
                              _buffer,                                  \
                              sizeof(uint8),                            \
                              &(_occ_bm->cache_enabled),                \
                              _version_added);                          \
    SOC_DNX_WB_ENGINE_ADD_ARR_DEPRECATED(_var + DNX_WB_ENGINE_OCC_BM_LEVELS_BUFFER,    \
                              _var_string,                              \
                              _buffer,                                  \
                              sizeof(uint8),                            \
                              _occ_bm->levels_buffer,                   \
                              _array_size,                              \
                              _version_added);                          

#define SOC_DNX_WB_ENGINE_ADD_COMPLEX_DS_PAT_TREE_AGREGATION(_var, _var_string, _buffer, _pat_tree_0, _nof_pat_trees, _pat_tree_size, \
                                                             _pat_tree_support_cache, _occ_bm_array_size, _occ_bm_support_cache, _version_added) \
    SOC_DNX_WB_ENGINE_ADD_ARR_DEPRECATED_WITH_OFFSET(_var + DNX_WB_ENGINE_PAT_TREE_AGREGATION_ROOT, \
                                          _var_string,                  \
                                          _buffer,                      \
                                          sizeof(DNX_SAND_PAT_TREE_NODE_PLACE), \
                                          &(_pat_tree_0.pat_tree_data.root), \
                                          _nof_pat_trees,               \
                                          sizeof(DNX_SAND_PAT_TREE_INFO), \
                                          _version_added);              \
    SOC_DNX_WB_ENGINE_ADD_ARR_DEPRECATED_WITH_OFFSET(_var + DNX_WB_ENGINE_PAT_TREE_AGREGATION_CACHE_ENABLED, \
                                          _var_string,                  \
                                          _buffer,                      \
                                          sizeof(uint8),                \
                                          &(_pat_tree_0.pat_tree_data.cache_enabled), \
                                          _nof_pat_trees,               \
                                          sizeof(DNX_SAND_PAT_TREE_INFO), \
                                          _version_added);              \
    SOC_DNX_WB_ENGINE_ADD_ARR_DEPRECATED_WITH_OFFSET(_var + DNX_WB_ENGINE_PAT_TREE_AGREGATION_ROOT_CACHE, \
                                          _var_string,                  \
                                          _buffer,                      \
                                          sizeof(DNX_SAND_PAT_TREE_NODE_PLACE), \
                                          &(_pat_tree_0.pat_tree_data.root_cache), \
                                          _nof_pat_trees,               \
                                          sizeof(DNX_SAND_PAT_TREE_INFO), \
                                          _version_added);              \
    SOC_DNX_WB_ENGINE_ADD_ARR_DEPRECATED_WITH_OFFSET(_var + DNX_WB_ENGINE_PAT_TREE_AGREGATION_CACHE_CHANGE_HEAD, \
                                          _var_string,                  \
                                          _buffer,                      \
                                          sizeof(DNX_SAND_PAT_TREE_NODE_KEY), \
                                          &(_pat_tree_0.pat_tree_data.cache_change_head), \
                                          _nof_pat_trees,               \
                                          sizeof(DNX_SAND_PAT_TREE_INFO), \
                                          _version_added);              \
    SOC_DNX_WB_ENGINE_ADD_ARR_DEPRECATED_WITH_OFFSET(_var + DNX_WB_ENGINE_PAT_TREE_AGREGATION_CURRENT_NODE_PLACE, \
                                          _var_string,                  \
                                          _buffer,                      \
                                          sizeof(DNX_SAND_PAT_TREE_NODE_PLACE), \
                                          &(_pat_tree_0.pat_tree_data.current_node_place), \
                                          _nof_pat_trees,               \
                                          sizeof(DNX_SAND_PAT_TREE_INFO), \
                                          _version_added);              \
    SOC_DNX_WB_ENGINE_ADD_ARR_DEPRECATED(_var + DNX_WB_ENGINE_PAT_TREE_AGREGATION_TREE_MEMORY, \
                              _var_string,                              \
                              _buffer,                                  \
                              sizeof(DNX_SAND_PAT_TREE_NODE),           \
                              _pat_tree_0.pat_tree_data.tree_memory,    \
                              _pat_tree_size,                           \
                              _version_added);                          \
    if(_buffer == buffer_id && _pat_tree_support_cache == TRUE )        \
    {                                                                   \
        SOC_DNX_WB_ENGINE_ADD_ARR_DEPRECATED(_var + DNX_WB_ENGINE_PAT_TREE_AGREGATION_TREE_MEMORY_CACHE, \
                                  _var_string,                          \
                                  _buffer,                              \
                                  sizeof(DNX_SAND_PAT_TREE_NODE),       \
                                  _pat_tree_0.pat_tree_data.tree_memory_cache, \
                                  _pat_tree_size,                       \
                                  _version_added);                      \
    }                                                                   \
    SOC_DNX_WB_ENGINE_ADD_COMPLEX_DS_OCC_BM(_var + DNX_WB_ENGINE_PAT_TREE_AGREGATION_MEMORY_USE, \
                                            _var_string,                \
                                            _buffer,                    \
                                            _pat_tree_0.pat_tree_data.memory_use_ptr, \
                                            _occ_bm_array_size,         \
                                            _occ_bm_support_cache,      \
                                            _version_added)                           


#define SOC_DNX_ADD_JER2_ARAD_BUFFS_DEPRECATED_CODE
    
#ifdef BCM_WARM_BOOT_SUPPORT
#define SOC_DNX_WB_ENGINE_DYNAMIC_VAR_STATE_GET\
    /*special treatment for restoring dynamic vars*/\
    if (SOC_WARM_BOOT(unit) && buffer_is_dynamic) {\
        rv = soc_wb_engine_buffer_dynamic_vars_state_get(unit, SOC_DNX_WB_ENGINE, buffer_id, &buffer_header);\
        DNXC_IF_ERR_EXIT(rv);\
    }
#else /*BCM_WARM_BOOT_SUPPORT*/
#define SOC_DNX_WB_ENGINE_DYNAMIC_VAR_STATE_GET\
    /*special treatment for restoring dynamic vars*/\
    if (SOC_WARM_BOOT(unit) && buffer_is_dynamic) {\
        rv = SOC_E_NONE;\
        DNXC_IF_ERR_EXIT(rv);\
    }
#endif /*BCM_WARM_BOOT_SUPPORT*/


#define SOC_DNX_ADD_JER2_ARAD_VARS_DEPRECATED_CODE

#define SOC_DNX_ALL_JER2_ARAD_VARS_DEPRECATED_CODE\
    SOC_DNX_ADD_JER2_ARAD_VARS_DEPRECATED_CODE /* deprecated code */



#define SOC_DNX_WB_ENGINE_DEPRECATED_CODE\
    /* --------- buffers inside these macros were added to wb_engine using an old implementation ----------- */\
    /* -- that is now deprected and shouldn't be used as an example implementation for adding new buffers -- */\
    SOC_DNX_ADD_JER2_ARAD_BUFFS_DEPRECATED_CODE;      /* deprecated code */\
    /* ------------------------------------------------------------------------------------------------------ */\
\
    SOC_DNX_WB_ENGINE_DYNAMIC_VAR_STATE_GET;\
\
    /* --------- variables inside these macros were added to wb_engine using an old implementation ----------- */\
    /* -- that is now deprected and shouldn't be used as an example implementation for adding new variables -- */\
    SOC_DNX_ALL_JER2_ARAD_VARS_DEPRECATED_CODE;


#endif /*_SOC_DNX_WB_ENGINE_DEPRECATED_H_*/
