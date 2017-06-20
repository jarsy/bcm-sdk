/*! \file res_mngr_types.h
 * 
 * Internal DNX resource manager APIs 
 * 
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 */
#ifndef ALGO_RES_MNGR_TYPES_INCLUDED
/*
 * { 
 */
#define ALGO_RES_MNGR_TYPES_INCLUDED

#ifndef BCM_DNX_SUPPORT
#error "This file is for use by DNX (JR2) family only!"
#endif

/**
 *  
 *   \brief General types for resource manager.
 *  
 */

/**
 * \brief The name of the resource manager instance, used with all resource manager functions. 
 */
typedef char *dnx_algo_res_name_t;

/**
 * \brief Resource creation information
 *
 * This structure contains the information required for creating a new resource.
 * 
 *  \see 
 * dnx_algo_res_create
 */
typedef struct
{
    /*
     * *
     * * DNX_ALGO_RES_INIT_* flags
     */
    uint32 flags;
    /*
     * *
     * * First element of the resource
     */
    int first_element;
    /*
     * *
     * * How many elements are in the resource.
     */
    int nof_elements;
} dnx_algo_res_create_data_t;

/**
 * \brief This is the callback prototype used for iterating over resources. Not yet implemented.
 */
typedef shr_error_e(
    *dnx_algo_res_iterate_iter_cb) (
    int unit,
    int core_id,
    int element,
    void *extra_arguments);

/*
 * } 
 */
#endif/*_ALGO_RES_MNGR_CALLBACKS_INCLUDED__*/
