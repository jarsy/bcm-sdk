/** \file res_mngr_internal.h
 * 
 * Internal DNX resource manager APIs 
 * 
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 */
#ifndef ALGO_RES_MNGR_INTERNAL_INCLUDED
/* { */
#define ALGO_RES_MNGR_INTERNAL_INCLUDED

#ifndef BCM_DNX_SUPPORT
#error "This file is for use by DNX (JR2) family only!"
#endif

/**
* INCLUDE FILES:
* {
*/
#include <bcm_int/dnx/algo/res_mngr/res_mngr_api.h>
#include <shared/swstate/sw_state.h>
#include <shared/utilex/utilex_hashtable.h>
/**
 * }
 */

#define DNX_ALGO_RES_MNGR_NOF_RESOURCES (100)

#define DNX_ALGO_RES_MNGR_MAX_NAME_LENGTH   (100)

/**
 *  
 *   General types for internal resource manager management.
 */
/**
 * \brief Resource information.
 *
 * This structure contains the information required for managing and verifying resource use.
 *   This data is taken from the dnx_algo_res_create call.
 * 
 * 
 */
typedef struct
{
    /*
     * *
     * * Flags used to create this resource.
     */
    uint32 flags;
    /*
     * *
     * * First element of the resource.
     */
    int first_element;
    /*
     * *
     * * How many elements are in the resource.
     */
    int nof_elements;
    /*
     * *
     * * Internal id used by the internal algorithm. 
     */
    int algo_instance_id;
} dnx_algo_res_pool_data_t;

/**
 * \brief Module sw state
 *
 * This structure is used by sw state to store the resource manager data.
 *   
 */
typedef struct
{
    /*
     * *
     * * A two dimentional array holding resource pool data. First dimention is 
     * *  res_index, second dimention is pool_core. 
     * * Pool is per {resource, core} for multi core resources, and per 
     * * resource otherwise.
     */
    PARSER_HINT_ARR_ARR dnx_algo_res_pool_data_t **pools_data;
    /*
     * *
     * * Holds the number of pools currently in use. Used for retrieving the next
     * * available res index.
     */
    int res_in_use_count;
    /*
     * * 
     * * Pointer for the hash table used for mapping names to resource indexes. 
     */
    UTILEX_HASH_TABLE_PTR hash_table_ptr;
} dnx_algo_res_mngr_info_t;

shr_error_e dnx_algo_res_init(
    int unit);

shr_error_e dnx_algo_res_deinit(
    int unit);

/* } */
#endif/*_ALGO_RES_MNGR_INTERNAL_INCLUDED__*/
