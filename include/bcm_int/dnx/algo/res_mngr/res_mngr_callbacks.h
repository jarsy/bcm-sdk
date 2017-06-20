/** \file res_mngr_api.h
 * 
 * Internal DNX resource manager APIs 
 * 
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 */
#ifndef ALGO_RES_MNGR_CALLBACKS_INCLUDED
/*
 * { 
 */
#define ALGO_RES_MNGR_CALLBACKS_INCLUDED

#ifndef BCM_DNX_SUPPORT
#error "This file is for use by DNX (JR2) family only!"
#endif

#include <bcm_int/dnx/algo/res_mngr/res_mngr_types.h>

/** 
 * Callbacks for dnx_algo_res_create.
 * 
 * Note that these callbacks don't take core_id as an argument.
 *   If the advanced algorithm requires core as an argument, it must be provided in the extra arguments structure.
 * 
 * Upon creation, the algorithm must create a unique identifier (algo_instance_id) for each instance. This identifier will be used
 *   to manage this resource when the other APIs are called.
 *   If the algorithm is unique (has only one instance), the identifier has no significance.
 *   If the algorithm is unique and uses multi_set_template as internal library, it's recommended to
 *     use the multi_set_template's indentifier.
 *
 * \see
 * dnx_algo_res_create
 */

/**
* \brief
*   Callback to create a new instance of the advanced algorithm.
* 
*
*  \par DIRECT INPUT:
*    \param [in] unit -
*      Relevant unit.
*    \param [in] create_data -
*      Pointed memory contains setup parameters required for the
*      creation of the resouce. See \ref dnx_algo_res_create_data_t
*    \param [in] extra_arguments
*      Pointed memory to hold extra arguemnts required for creating this resource. \n
*      \b As \b input - \n
*       The content of this pointer depends on the algorithm. It's the algorithm's \n
*       responsibility to verify it.
*       If the algorithm doesn't require extra arguemnts, this can be set to NULL.
*    \param [in] algo_instance_id
*      Pointer to place the Algorithm instance id that was created.
*      \b As \b output - \n
*        Holds the created instance id.
*  \par DIRECT OUTPUT:
*    \retval Zero if no error was detected
*    \retval Negative if error was detected. See \ref shr_error_e
*  \remark
*    None
*  \see
*    dnx_algo_res_create
*    shr_error_e
*/
typedef shr_error_e(
    *dnx_algo_res_create_cb) (
    int unit,
    dnx_algo_res_create_data_t * create_data,
    void *extra_arguemnts,
    int *algo_instance_id);

/**
* \brief
*   Callback to allocate a free element of an advanced algorithm.
* 
*
*  \par DIRECT INPUT:
*    \param [in] unit -
*      Relevant unit.
*    \param [in] algo_instance_id -
*      Internal id for the algorithm.
*    \param [in] flags -
*      DNX_ALGO_RES_ALLOCATE_* flags
*    \param [in] extra_arguments
*      Pointed memory to hold extra arguemnts required for allocating this element. \n
*      \b As \b input - \n
*       The content of this pointer depends on the algorithm. It's the algorithm's \n
*       responsibility to verify it.
*       If the algorithm doesn't require extra arguemnts, this can be set to NULL.
*    \param [in] element -
*      Pointer to place the allocated element.
*      \b As \b output - \n
*        Holds the allocated element.
*      \b As \b input - \n
*        If flag \ref DNX_ALGO_RES_ALLOCATE_WITH_ID is set, pass the element to be allocated here.
*  \par INDIRECT OUTPUT:
*    *element -
*      Value of the allocated element.
*  \par DIRECT OUTPUT:
*    shr_error_e - 
*      Error return value
*  \remark
*    None
*  \see
*    dnx_algo_res_allocate
*    shr_error_e
*****************************************************/
typedef shr_error_e(
    *dnx_algo_res_allocate_cb) (
    int unit,
    int algo_instance_id,
    uint32 flags,
    void *extra_arguments,
    int *element);

/**
* \brief
*   Callback to check whether an element was allocated in an adavnced algorithm.
* 
*
*  \par DIRECT INPUT:
*    \param [in] unit -
*      Relevant unit.
*    \param [in] algo_instance_id -
*      Internal id for the algorithm.
*    \param [in] element -
*      Element to be checked.
*    \param [in] is_allocated -
*      Pointer to be filled with allocated / free indication.
*  \par INDIRECT OUTPUT:
*    *is_allocated -
*       boolean indication of the element's existence.
*  \par DIRECT OUTPUT:
*    shr_error_e - 
*      Error return value
*  \remark
*    None
*  \see
*    dnx_algo_res_allocate
*    shr_error_e
*****************************************************/
typedef shr_error_e(
    *dnx_algo_res_is_allocated_cb) (
    int unit,
    int algo_instance_id,
    int element,
    uint8 * is_allocated);

/**
* \brief
*   Callback to free an allocated element in an advanced algorithm.
* 
*
*  \par DIRECT INPUT:
*    \param [in] unit -
*      Relevant unit.
*    \param [in] algo_instance_id -
*      Internal id for the algorithm.
*    \param [in] element -
*      Element to be freed.
*  \par DIRECT OUTPUT:
*    shr_error_e - 
*      Error return value
*  \remark
*    None
*  \see
*    dnx_algo_res_free
*    shr_error_e
*****************************************************/
typedef shr_error_e(
    *dnx_algo_res_free_cb) (
    int unit,
    int algo_instance_id,
    int element);

/*
 * Free all elements callback. Not yet implemented.
 */
typedef shr_error_e(
    *dnx_algo_res_free_all_cb) (
    int unit,
    int algo_instance_id);

/*
 * Iterate over all allocated entries and perform cb. Not yet implemented.
 */
typedef shr_error_e(
    *dnx_algo_res_iterate_cb) (
    int unit,
    int algo_instance_id,
    dnx_algo_res_iterate_iter_cb cb);

/*
 * Print all valid entries callback. Not yet implemented.
 */
typedef shr_error_e(
    *dnx_algo_res_dump_cb) (
    int unit,
    int algo_instance_id);

/*
 * Print all valid entries in range callback. Not yet implemented.
 */
typedef shr_error_e(
    *dnx_algo_res_dump_in_range_cb) (
    int unit,
    int algo_instance_id,
    int start,
    int end);

/**
 * \brief Callback struct for dnx_algo_res_create.
 *
 * This structure contains the callbacks that will be called when an advanced resource
 *  is accessed.
 * 
 *  The callbacks for create, allocate, is_allocated and free are mandatory, and the rest are optional.
 *   
 *  \see 
 * dnx_algo_res_create
 */
typedef struct
{
    dnx_algo_res_create_cb create_cb;       /* Mandatory */
    dnx_algo_res_allocate_cb allocate_cb;   /* Mandatory */
    dnx_algo_res_is_allocated_cb is_allocated_cb; /* Mandatory */
    dnx_algo_res_free_cb free_cb;           /* Mandatory */
    dnx_algo_res_free_all_cb free_all_cb; 
    dnx_algo_res_iterate_cb iterate_cb;
    dnx_algo_res_dump_cb dump_cb;
    dnx_algo_res_dump_in_range_cb dump_in_range_cb;
} dnx_algo_res_advanced_alogrithm_cb_t;

/*
 * } 
 */
#endif/*_ALGO_RES_MNGR_CALLBACKS_INCLUDED__*/
