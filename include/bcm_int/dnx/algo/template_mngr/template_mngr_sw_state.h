/** \file template_mngr_sw_state.h
 * 
 * Internal DNX template manager APIs 
 * 
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 */
#ifndef ALGO_TEMPLATE_MNGR_SW_STATE_INCLUDED
/*
 * { 
 */
#define ALGO_TEMPLATE_MNGR_SW_STATE_INCLUDED

#ifndef BCM_DNX_SUPPORT
#error "This file is for use by DNX (JR2) family only!"
#endif

/**
* INCLUDE FILES:
* {
*/
#include <bcm_int/dnx/algo/template_mngr/template_mngr_api.h>
#include <shared/swstate/sw_state.h>
#include <shared/utilex/utilex_hashtable.h>
/**
 * }
 */

/**
 *  
 *   General types for internal template manager management.
 */
/**
 * \brief Template information.
 *
 * This structure contains the information required for managing and verifying template use.
 *   This data is taken from the dnx_algo_template_create call.
 * 
 * 
 */
typedef struct
{
    /*
     *
     * Flags used to create this template.
     */
    uint32 flags;
    /*
     *
     * First profile id of the template.
     */
    int first_profile;
    /*
     *
     * How many profiles are in the template.
     */
    int nof_profiles;
    /*
     *
     * Maximum number of pointers to each profile.
     */
    int max_references;
    /*
     * 
     * Default profile for the template. To be used if flag 
     * DNX_ALGO_TEMPLATE_INIT_USE_DEFAULT_PROFILE is set.
     */
    int default_profile;
    /*
     *
     * Size of the template's data.
     */
    int data_size;
    /*
     *
     * Internal id used by the internal algorithm. 
     */
    int algo_instance_id;
} dnx_algo_template_pool_data_t;

/**
 * \brief Module sw state
 *
 * This structure is used by sw state to store the template manager data.
 *   
 */
typedef struct
{
    /*
     *
     * A two dimentional array holding template pool data. First dimention is 
     *  res_index, second dimention is pool_core. 
     * Pool is per {template, core} for multi core templates, and per 
     * template otherwise.
     */
    PARSER_HINT_ARR_ARR dnx_algo_template_pool_data_t **pools_data;
    /*
     *
     * Holds the number of pools currently in use. Used for retrieving the next
     * available res index.
     */
    int template_in_use_count;
    /*
     * 
     * Pointer for the hash table used for mapping names to template indexes. 
     */
    UTILEX_HASH_TABLE_PTR hash_table_ptr;
} dnx_algo_template_mngr_info_t;

/*
 * } 
 */
#endif/*_ALGO_TEMPLATE_MNGR_SW_STATE_INCLUDED__*/
