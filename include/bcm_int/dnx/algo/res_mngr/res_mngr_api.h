/** \file res_mngr_api.h
 * 
 * Internal DNX resource manager APIs 
 * 
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 */
#ifndef ALGO_RES_MNGR_API_INCLUDED
/* { */
#define ALGO_RES_MNGR_API_INCLUDED

#ifndef BCM_DNX_SUPPORT
#error "This file is for use by DNX (JR2) family only!"
#endif

#include <bcm_int/dnx/algo/res_mngr/res_mngr_callbacks.h>
#include <bcm_int/dnx/algo/res_mngr/res_mngr_types.h>

/**
 * Flags for dnx_algo_res_create_data_t struct 
 *  
 * \see 
 * dnx_algo_res_create 
 */
/**
 * Duplicate this resource per core.
 */
#define DNX_ALGO_RES_CREATE_DUPLICATE_PER_CORE      SAL_BIT(0)
/**
 * This resource uses an advanced algorithm.
 */
#define DNX_ALGO_RES_CREATE_USE_ADVANCED_ALGORITHM  SAL_BIT(1)

/**
 * Flags for dnx_algo_res_allocate 
 *  
 * \see 
 * dnx_algo_res_allocate
 */
/**
 * Allocate with a given ID.
 */
#define DNX_ALGO_RES_ALLOCATE_WITH_ID       SAL_BIT(0)

/**
* \brief
*   Create a new instance of resource manager.
* 
*
*  \par DIRECT INPUT:
*    \param [in] unit -
*      Relevant unit.
*    \param [in] core_id -
*      For resource creation must be _SHR_CORE_ALL.
*    \param [in] name -
*      The resource will be created with this name, and will be referenced with this name from the other
*      resource manager APIs.
*    \param [in] data -
*      Pointed memory contains setup parameters required for the
*      creation of the resouce. See \ref dnx_algo_res_create_data_t
*    \param [in] callbacks -
*      Pointed memory contains callbacks to functions used by advanced resources.
*      Leave NULL if data->flags doesn't contain \ref DNX_ALGO_RES_CREATE_USE_ADVANCED_ALGORITHM
*    \param [in] extra_arguments
*      Pointer for extra arguments to be passed to the create function of an advanced algorithm.
*      Leave NULL if data->flags doesn't contain \ref DNX_ALGO_RES_CREATE_USE_ADVANCED_ALGORITHM
*  \par DIRECT OUTPUT:
*    \retval Zero if no error was detected
*    \retval Negative if error was detected. See \ref shr_error_e
*  \remark
*    None
*  \see
*    dnx_algo_res_create_data_t
*    dnx_algo_res_advanced_alogrithm_cb_t
*    shr_error_e
*****************************************************/
shr_error_e dnx_algo_res_create(
    int unit,
    int core_id,
    dnx_algo_res_name_t name,
    dnx_algo_res_create_data_t * data,
    dnx_algo_res_advanced_alogrithm_cb_t * callbacks,
    void *extra_arguments);

/**
* \brief
*   Allocate a free element. 
* 
*
*  \par DIRECT INPUT:
*    \param [in] unit -
*      Relevant unit.
*    \param [in] core_id -
*      Relevant core. If resource is per core, it must be a valid core id, otherwise, must be _SHR_CORE_ANY.
*    \param [in] name -
*      Name used to create this resource.
*    \param [in] flags -
*      DNX_ALGO_RES_ALLOCATE_* flags. E.g., \ref DNX_ALGO_RES_ALLOCATE_WITH_ID
*    \param [in] extra_arguments -
*      Pointer to memory holding extra arguments to be passed to the allocation function of an advanced algorithm.
*      If no advanced algorithm is used, use NULL.
*    \param [in] element -
*      Pointer to place the allocated element.
*      \b As \b output - \n
*        Holds the allocated element.
*      \b As \b input - \n
*        If flag \ref DNX_ALGO_RES_ALLOCATE_WITH_ID is set, pass the element to be allocated here.
*  \par DIRECT OUTPUT:
*    \retval Zero if no error was detected
*    \retval Negative if error was detected. See \ref shr_error_e
*  \remark
*    None
*  \see
*    dnx_algo_res_create_data_t
*    dnx_algo_res_advanced_alogrithm_cb_t
*    shr_error_e
*****************************************************/
shr_error_e dnx_algo_res_allocate(
    int unit,
    int core_id,
    dnx_algo_res_name_t name,
    uint32 flags,
    void *extra_arguments,
    int *element);

/**
* \brief
*   Check whether an element was allocated.
* 
*
*  \par DIRECT INPUT:
*    \param [in] unit -
*      Relevant unit.
*    \param [in] core_id -
*      Relevant core. If resource is per core, it must be a valid core id, otherwise, must be _SHR_CORE_ANY.
*    \param [in] name -
*      Name used to create this resource.
*    \param [in] element -
*      Element to be checked.
*    \param [in] is_allocated -
*      Pointer to memory to load output into. \n
*      \b As \b output - \n
*        Will be filled with boolean indication of the element's existence.
*  \par DIRECT OUTPUT:
*    \retval Zero if no error was detected
*    \retval Negative if error was detected. See \ref shr_error_e
*  \remark
*    None
*  \see
*    dnx_algo_res_create_data_t
*    dnx_algo_res_advanced_alogrithm_cb_t
*    shr_error_e
*****************************************************/
shr_error_e dnx_algo_res_is_allocated(
    int unit,
    int core_id,
    dnx_algo_res_name_t name,
    int element,
    uint8 * is_allocated);

/**
* \brief
*   Free allocated element.
* 
*
*  \par DIRECT INPUT:
*    \param [in] unit -
*      Relevant unit.
*    \param [in] core_id -
*      Relevant core. If resource is per core, it must be a valid core id, otherwise, must be _SHR_CORE_ANY.
*    \param [in] name -
*      Name used to create this resource.
*    \param [in] element -
*      Element to be freed.
*  \par DIRECT OUTPUT:
*    \retval Zero if no error was detected
*    \retval Negative if error was detected. See \ref shr_error_e
*  \remark
*    None
*  \see
*    dnx_algo_res_create_data_t
*    dnx_algo_res_advanced_alogrithm_cb_t
*    shr_error_e
*****************************************************/
shr_error_e dnx_algo_res_free(
    int unit,
    int core_id,
    dnx_algo_res_name_t name,
    int element);

/* Free all elements. Not yet impelemented. */
shr_error_e dnx_algo_res_free_all(
    int unit,
    int core_id,
    dnx_algo_res_name_t name);

/* Iterate over all allocated entries and perform cb. Not yet impelemented. */
shr_error_e dnx_algo_res_iterate(
    int unit,
    int core_id,
    dnx_algo_res_name_t name,
    dnx_algo_res_iterate_iter_cb cb);

/* Print all valid entries. Not yet impelemented. */
shr_error_e dnx_algo_res_dump(
    int unit,
    int core_id,
    dnx_algo_res_name_t name);

/* Print all valid entries in range. Not yet impelemented. */
shr_error_e dnx_algo_res_dump_in_range(
    int unit,
    int core_id,
    dnx_algo_res_name_t name,
    int start,
    int end);

/* } */
#endif/*_ALGO_RES_MNGR_API_INCLUDED__*/
