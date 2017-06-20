/** \file multi_set_template.h
 * 
 * Internal DNX template manager APIs 
 * 
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 */
#ifndef MULTI_SET_TEMPLATE_INCLUDED
/*
 * { 
 */
#define MULTI_SET_TEMPLATE_INCLUDED

#ifndef BCM_DNX_SUPPORT
#error "This file is for use by DNX (JR2) family only!"
#endif

/**
* INCLUDE FILES:
* {
*/
#include <bcm_int/dnx/algo/template_mngr/template_mngr_types.h>
#include <shared/swstate/sw_state.h>
/**
 * }
 */

/**
 * Multi set template is the default algorithm for template manager. It is implemented as an advanced algorithm.
 *  
 * It's a wrapper for utilex_multi_set, but adds further logic on top of it: 
 *   - utilex_multi_set only supports profile ids from 0 to x. Each function in multi_set_template performs mapping
 *     from the external profile id to this internal profile id.
 *   - multi_set_template_exchange performs the complicated logic needed to fully support exchange operation.
 *  
 * It is highly recommeneded to use this library as an internal implementation of your advanced algorithm.
 */


/**
 * Allocate several instances of the same profile. Must be combined with the WITH_ID flag.
 *   Use the extra_arguments arguemnt as an integer when using this flag.
 */
#define MULTI_SET_TEMPLATE_ALLOCATE_SEVERAL  SAL_BIT(25)


/**
* \brief
*   Callback to create a new instance of the multi_set_template.
* 
*
*  \par DIRECT INPUT:
*    \param [in] unit -
*      Relevant unit.
*    \param [in] create_data -
*      Pointed memory contains setup parameters required for the
*      creation of the resouce. See \ref dnx_algo_template_create_data_t
*    \param [in] extra_arguments
*      Pointed memory to hold extra arguemnts required for creating this template. \n
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
*    multi_set_template_create
*    shr_error_e
*/
shr_error_e multi_set_template_create(
    int unit,
    dnx_algo_template_create_data_t * create_data,
    void *extra_arguments,
    int *algo_instance_id);

/**
* \brief
*   Callback to allocate a free profile of an advanced algorithm.
* 
*
*  \par DIRECT INPUT:
*    \param [in] unit -
*      Relevant unit.
*    \param [in] algo_instance_id -
*      Internal id for the algorithm.
*    \param [in] flags -
*      DNX_ALGO_TEMPLATE_ALLOCATE_* flags
*    \param [in] profile_data -
*      Pointer to memory holding template data to be saved to a profile.
*    \param [in] extra_arguments -
*      Pointer to memory holding extra arguments to be passed to the allocation function of an advanced algorithm.
*      If no advanced algorithm is used, use NULL.
*    \param [in] profile -
*      Pointer to place the allocated profile.
*      \b As \b output - \n
*        Holds the allocated profile.
*      \b As \b input - \n
*        If flag \ref DNX_ALGO_TEMPLATE_ALLOCATE_WITH_ID is set, pass the profile to be allocated here.
*    \param [in] first_reference
*      Pointer to memory to load output into. \n
*      \b As \b output - \n
*        Will be filled with boolean indication of the profile's prior existence. \n
*        If TRUE, this is the first reference to the profile, and the data needs to be
*          written to the relevant table.
*  \par DIRECT OUTPUT:
*    shr_error_e - 
*      Error return value
*  \remark
*    None
*  \see
*    multi_set_template_allocate
*    shr_error_e
*****************************************************/
shr_error_e multi_set_template_allocate(
    int unit,
    int algo_instance_id,
    uint32 flags,
    void *profile_data,
    void *extra_arguments,
    int *profile,
    uint8 * first_reference);

/**
* \brief
*   Free one reference from old_profile, add one reference to data.
* 
*
*  \par DIRECT INPUT:
*    \param [in] unit -
*      Relevant unit.
*    \param [in] algo_instance_id -
*      Internal id for the algorithm.
*    \param [in] flags -
*      DNX_ALGO_TEMPLATE_ALLOCATE_* or DNX_ALGO_TEMPLATE_EXCHANGE_* flags.
*    \param [in] profile_data -
*      Pointer to memory holding template data to be saved to a profile.
*    \param [in] old_profile -
*      One reference will be freed from this profile.
*    \param [in] extra_arguments -
*      Pointer to memory holding extra arguments to be passed to the allocation function of an advanced algorithm.
*      If no advanced algorithm is used, use NULL.
*    \param [in] new_profile -
*      Pointer to place the allocated profile.
*      \b As \b output - \n
*        Holds the allocated profile.
*      \b As \b input - \n
*        If flag \ref DNX_ALGO_TEMPLATE_ALLOCATE_WITH_ID is set, pass the profile to be allocated here.
*    \param [in] first_reference
*      Pointer to memory to load output into. \n
*      \b As \b output - \n
*        Will be filled with boolean indication of the new profile's prior existence. \n
*    \param [in] last_reference -
*      Pointer to memory to load output into. \n
*      \b As \b output - \n
*        Will be filled with boolean indication of whether it's the last reference to old_profile.
*  \par DIRECT OUTPUT:
*    \retval Zero if no error was detected
*    \retval Negative if error was detected. See \ref shr_error_e
*  \remark
* 
*      Whether WITH_ID is provided or not, if the allocation / update was successful,
*      the template with ID provided in old_template will be freed.  Note this only
*      happens if the selection of a new template succeeds, so the old template
*      should not be cleared if this function fails.
*      If old_template wasn't allocated before, and the flag
*      DNX_ALGO_TEMPLATE_EXCHANGE_IGNORE_NOT_EXIST_OLD_TEMPLATE is not set,
*      then BCM_E_PARAM will be returned. If the flag is set, then a new refrence 
*      will be allocated without deleting the old one.
* 
*      If WITH_ID is set and the template specified in *template_param is not yet allocated,
*      this will allocate it and set its data to the specified data. 
* 
*      If WITH_ID is set but IGNORE_DATA is not, and the specified template in *template_param is
*      allocated, the data of the template will be changed to the new data.
*      However if the data already exists with a different index, then BCM_E_EXISTS
*      is returned (if the user wishes for the same key to be used for two different
*      indices, it is possible to insert the index itself in the data which will
*      automatically differentiate them).
*
*      If WITH_ID and IGNORE_DATA are both set, this will increment the reference
*      count for the specified template.
*      If the template does not exist then BCM_E_NOT_FOUND is returned.
* 
*      If WITH_ID is clear, this will look for a in-use template whose data are
*      'equal' to the specified data.  If it finds such a template, it will
*      increment that template's reference count and return it.  If it does not
*      find such a template, it marks an available template as in use, sets its
*      reference count to 1, copies the data to the template's data, and returns
*      this template.  If there is no 'equal' or free template, BCM_E_RESOURCE.
*
*
*      Whether WITH_ID is provided or not, on success the int pointed to by the
*      is_allocated argument will be updated.  It will be set TRUE if the new
*      template was free before, and FALSE if the new template was already in use.
*      If the value is TRUE, appropriate resources should be updated by the caller
*      (such as programming the data values to hardware registers).
*
*      It is not valid to specify IGNORE_DATA without WITH_ID.
*  \see
*    multi_set_template_allocate
*    multi_set_template_free
*    shr_error_e
*****************************************************/
shr_error_e multi_set_template_exchange(
    int unit,
    int algo_instance_id,
    uint32 flags,
    const void *profile_data,
    int old_profile,
    const void *extra_arguments,
    int *new_profile,
    uint8 * first_reference,
    uint8 * last_reference);

/**
* \brief
*   Get the data pointed by a template's profile.
*
*  \par DIRECT INPUT:
*    \param [in] unit -
*      Relevant unit.
*    \param [in] algo_instance_id -
*      Internal id for the algorithm.
*    \param [in] profile -
*      Profile to get the data from.
*    \param [in] ref_count -
*      Pointer to memory to load output into. \n
*      \b As \b output - \n
*        Will be filled with number of refernces to this profile.
*    \param [in] profile_data -
*      Pointer to memory to load output into. \n
*      \b As \b output - \n
*        Will be filled with data stored in this profile.
*  \par DIRECT OUTPUT:
*    \retval Zero if no error was detected
*    \retval Negative if error was detected. See \ref shr_error_e
*  \remark
*    None
*  \see
*    multi_set_template_create_data_t
*    multi_set_template_advanced_alogrithm_cb_t
*    shr_error_e
*****************************************************/
shr_error_e multi_set_template_profile_data_get(
    int unit,
    int algo_instance_id,
    int profile,
    int *ref_count,
    void *profile_data);

/**
* \brief
*   Get a template's profile by given data.
*
*  \par DIRECT INPUT:
*    \param [in] unit -
*      Relevant unit.
*    \param [in] algo_instance_id -
*      Internal id for the algorithm.
*    \param [in] profile_data -
*      Pointer to memory for data to lookup. \n
*      \b As \b input - \n
*        Fill this with the data to find.
*    \param [in] profile -
*      Pointer to place the allocated profile.
*      \b As \b output - \n
*        Holds the profile that holds given data.
*  
*  \par DIRECT OUTPUT:
*    \retval Zero if no error was detected
*    \retval Negative if error was detected. See \ref shr_error_e
*  \remark
*    None
*  \see
*    multi_set_template_create_data_t
*    multi_set_template_advanced_alogrithm_cb_t
*    shr_error_e
*****************************************************/
shr_error_e multi_set_template_profile_get(
    int unit,
    int algo_instance_id,
    const void *profile_data,
    int *profile);

/**
* \brief
*   Callback to free an allocated profile in an advanced algorithm.
* 
*
*  \par DIRECT INPUT:
*    \param [in] unit -
*      Relevant unit.
*    \param [in] algo_instance_id -
*      Internal id for the algorithm.
*    \param [in] profile -
*      Profile to be freed.
*    \param [in] last_reference -
*      Pointer to memory to load output into. \n
*      \b As \b output - \n
*        Will be filled with boolean indication of whether it's the last reference to this profile.
*  \par DIRECT OUTPUT:
*    shr_error_e - 
*      Error return value
*  \remark
*    None
*  \see
*    multi_set_template_free
*    shr_error_e
*****************************************************/
shr_error_e multi_set_template_free(
    int unit,
    int algo_instance_id,
    int profile,
    uint8 * last_reference);

/**
* \brief
*   Free all reference to all profiles of this template.
* 
*
*  \par DIRECT INPUT:
*    \param [in] unit -
*      Relevant unit.
*    \param [in] algo_instance_id -
*      Internal id for the algorithm.
*  \par DIRECT OUTPUT:
*    \retval Zero if no error was detected
*    \retval Negative if error was detected. See \ref shr_error_e
*  \remark
*    None
*  \see
*    multi_set_template_create_data_t
*    multi_set_template_advanced_alogrithm_cb_t
*    shr_error_e
*****************************************************/
shr_error_e multi_set_template_clear(
    int unit,
    int algo_instance_id);

shr_error_e multi_set_template_init(
    int unit);

/**
 *  
 *   Multi set template sw state.
 */
/**
 * \brief Template information.
 *
 * This structure contains the information required for managing mutli set template. 
 * 
 */
typedef struct
{
    /*
     *
     * First profile id of the template.
     */
    int first_profile;
    /*
     *
     * Size of the template's data.
     */
    int data_size;
} dnx_multi_set_template_pool_data_t;

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
    PARSER_HINT_ARR dnx_multi_set_template_pool_data_t *pools_data;
} dnx_multi_set_template_info_t;

/*
 * } 
 */
#endif/*_MULTI_SET_TEMPLATE_INCLUDED__*/
