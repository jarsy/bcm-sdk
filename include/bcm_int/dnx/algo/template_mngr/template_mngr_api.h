/** \file template_mngr_api.h
 * 
 * Internal DNX template manager APIs 
 * 
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 */
#ifndef ALGO_TEMPLATE_MNGR_API_INCLUDED
/*
 * { 
 */
#define ALGO_TEMPLATE_MNGR_API_INCLUDED

#ifndef BCM_DNX_SUPPORT
#error "This file is for use by DNX (JR2) family only!"
#endif

#include <bcm_int/dnx/algo/template_mngr/template_mngr_callbacks.h>
#include <bcm_int/dnx/algo/template_mngr/template_mngr_types.h>

/**
 * Flags for dnx_algo_template_create_data_t struct 
 *  
 * \see 
 * dnx_algo_template_create 
 */
/**
 * Duplicate this template per core.
 */
#define DNX_ALGO_TEMPLATE_CREATE_DUPLICATE_PER_CORE     SAL_BIT(0)
/**
 * This template uses an advanced algorithm.
 */
#define DNX_ALGO_TEMPLATE_CREATE_USE_ADVANCED_ALGORITHM SAL_BIT(1)
/** 
 * This template uses a default profile to which all entries are allocated on creation.
 */
#define DNX_ALGO_TEMPLATE_CREATE_USE_DEFAULT_PROFILE    SAL_BIT(2)

/**
 * Flags for dnx_algo_template_allocate and dnx_algo_template_exchange
 *  
 * \see 
 * dnx_algo_template_allocate 
 * dnx_algo_template_exchange
 */
/**
 * Allocate with a given ID.
 */
#define DNX_ALGO_TEMPLATE_ALLOCATE_WITH_ID                          SAL_BIT(0)
/**
 * On exchange operation, ignore input data. 
 *   This is used for setting the profile with the WITH_ID flag, without reading the data first.
 *   Therefore, it must be used WITH_ID.
 * \see dnx_algo_template_exchange 
 */
#define DNX_ALGO_TEMPLATE_EXCHANGE_IGNORE_DATA                      SAL_BIT(1)
/**
 * On exchange operation (see below), if the old template does not exist then an error message will not returned. 
 * \see dnx_algo_template_exchange 
 */
#define DNX_ALGO_TEMPLATE_EXCHANGE_IGNORE_NOT_EXIST_OLD_PROFILE    SAL_BIT(2)
/**
 *  Run exchange operation to see if it works but don't change sw state.
 *  \see dnx_algo_template_exchange 
 */
#define DNX_ALGO_TEMPLATE_EXCHANGE_TEST                             SAL_BIT(3)

/**
* \brief
*   Initialize template manager module.
* 
*
*  \par DIRECT INPUT:
*    \param [in] unit -
*      Relevant unit.
*  \par DIRECT OUTPUT:
*    \retval Zero if no error was detected
*    \retval Negative if error was detected. See \ref shr_error_e
*  \remark
*    None
*  \see
*    shr_error_e
*****************************************************/
shr_error_e dnx_algo_template_init(
    int unit);

/**
* \brief
*   Create a new instance of template manager.
* 
*
*  \par DIRECT INPUT:
*    \param [in] unit -
*      Relevant unit.
*    \param [in] core_id -
*      For template creation (always) must be _SHR_CORE_ALL.
*    \param [in] name -
*      The template will be created with this name, and will be referenced with this name from the other
*      template manager APIs.
*    \param [in] data -
*      Pointed memory contains setup parameters required for the
*      creation of the template. See \ref dnx_algo_template_create_data_t
*    \param [in] callbacks -
*      Pointed memory contains callbacks to functions used by advanced templates.
*      Leave NULL if data->flags doesn't contain \ref DNX_ALGO_TEMPLATE_CREATE_USE_ADVANCED_ALGORITHM
*    \param [in] extra_arguments
*      Pointer for extra arguments to be passed to the create function of an advanced algorithm.
*      Leave NULL if data->flags doesn't contain \ref DNX_ALGO_TEMPLATE_CREATE_USE_ADVANCED_ALGORITHM
*  \par DIRECT OUTPUT:
*    \retval Zero if no error was detected
*    \retval Negative if error was detected. See \ref shr_error_e
*  \remark
*    None
*  \see
*    dnx_algo_template_create_data_t
*    dnx_algo_template_advanced_alogrithm_cb_t
*    shr_error_e
*****************************************************/
shr_error_e dnx_algo_template_create(
    int unit,
    int core_id,
    dnx_algo_template_name_t name,
    dnx_algo_template_create_data_t * data,
    dnx_algo_template_advanced_alogrithm_cb_t * callbacks,
    void *extra_arguments);

/**
* \brief
*   Allocate a reference to profile_data. If this is the first reference, the first_reference
*     indication will be true. 
* 
*
*  \par DIRECT INPUT:
*    \param [in] unit -
*      Relevant unit.
*    \param [in] core_id -
*      Relevant core. If template is per core (created with DNX_ALGO_TEMPLATE_CREATE_DUPLICATE_PER_CORE flag),
*         it must be a valid core id, otherwise, must be _SHR_CORE_ANY.
*    \param [in] name -
*      Name used to create this template.
*    \param [in] flags -
*      DNX_ALGO_TEMPLATE_ALLOCATE_* flags.
*    \param [in] profile_data -
*      Pointer to memory holding template data to be saved to a profile.
*      \b As \b input - \n
*        Fill with the data to be saved.
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
*  \par DIRECT OUTPUT:
*    \retval Zero if no error was detected
*    \retval Negative if error was detected. See \ref shr_error_e
*  \remark
*      If \ref DNX_ALGO_TEMPLATE_ALLOCATE_WITH_ID (WITH_ID) is set and the specified
*      profile is not yet allocated, this will allocate it and set its data to the
*      specified data.
*
*      If WITH_ID is set and the specified profile is allocated, this will first
*      verify that the new data is 'equal' to the existing data for the profile,
*      and: if so, will increment the reference count for the specified profile;
*      if not, will return _SHR_E_EXISTS.
*
*      If WITH_ID is clear, this will look for a in-use profile whose data are
*      'equal' to the specified data.  If it finds such a profile, it will
*      increment that profile's reference count and return it.  If it does not
*      find such a profile, it marks an available profile as in use, sets its
*      reference count to 1, copies the data to the profile's data, and returns
*      this profile.  If there is no 'equal' or free profile, _SHR_E_RESOURCE.
*
*      Whether WITH_ID is provided or not, on success the int pointed to by the
*      is_allocated argument will be updated.  It will be set TRUE if the profile
*      was free before, and FALSE if the profile was already in use.  If the
*      value is TRUE, appropriate resources should be updated by the caller (such
*      as programming the data values to hardware registers).
*  
*  \see
*    dnx_algo_template_create_data_t
*    dnx_algo_template_advanced_alogrithm_cb_t
*    shr_error_e
*****************************************************/
shr_error_e dnx_algo_template_allocate(
    int unit,
    int core_id,
    dnx_algo_template_name_t name,
    uint32 flags,
    void *profile_data,
    void *extra_arguments,
    int *profile,
    uint8 * first_reference);

/**
* \brief
*   This function performs several replace operations:
*     1. Free one reference from old_profile, and add one to new_profile.
*     2. Change the data in new_profile. This is used by using the WITH_ID flag
*        without the IGNORE_DATA flag.
* 
*
*  \par DIRECT INPUT:
*    \param [in] unit -
*      Relevant unit.
*    \param [in] core_id -
*      Relevant core. If template is per core (created with DNX_ALGO_TEMPLATE_CREATE_DUPLICATE_PER_CORE flag),
*         it must be a valid core id, otherwise, must be _SHR_CORE_ANY.
*    \param [in] name -
*      Name used to create this template.
*    \param [in] flags -
*      DNX_ALGO_TEMPLATE_ALLOCATE_* or DNX_ALGO_TEMPLATE_EXCHANGE_* flags. See remarks below.
*    \param [in] profile_data -
*      Pointer to memory holding template data to be saved to a profile. Not in use if IGNORE_DATA flag is set.
*      \b As \b input - \n
*        Fill with the data to be saved.
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
*      Whether DNX_ALGO_TEMPLATE_ALLOCATE_WITH_ID is provided or not, if the allocation / update was successful,
*      the profile with ID provided in old_profile will be freed.  Note this only
*      happens if the selection of a new profile succeeds, so the old profile
*      should not be cleared if this function fails.
*      If old_profile wasn't allocated before, and the flag
*      DNX_ALGO_TEMPLATE_EXCHANGE_IGNORE_NOT_EXIST_OLD_TEMPLATE is not set,
*      then _SHR_E_PARAM will be returned. If the flag is set, then a new refrence 
*      will be allocated without deleting the old one.
* 
*      If DNX_ALGO_TEMPLATE_ALLOCATE_WITH_ID is set and the profile specified in *new_profile is not yet allocated,
*      this will allocate it and set its data to the specified data. 
* 
*      If DNX_ALGO_TEMPLATE_ALLOCATE_WITH_ID is set but DNX_ALGO_TEMPLATE_EXCHANGE_IGNORE_DATA is not,
*      and the specified profile in *new_profile is
*      allocated, the data of the profile will be changed to the new data.
*      However if the data already exists with a different profile, then _SHR_E_EXISTS
*      is returned.
*
*      If DNX_ALGO_TEMPLATE_ALLOCATE_WITH_ID and DNX_ALGO_TEMPLATE_EXCHANGE_IGNORE_DATA are both set,
*      this will increment the reference count for the specified profile. This combination is used
*      to skip reading the data of the new profile before 
*      If the profile does not exist then _SHR_E_NOT_FOUND is returned.
* 
*      If DNX_ALGO_TEMPLATE_ALLOCATE_WITH_ID is clear, this procedure will look for a in-use profile whose data is
*      identical to profile_data.  If it finds such a profile, it will
*      increment that profile's reference count and return it. If it does not
*      find such a profile, it will allocate a new free profile.
*      If there is no 'equal' or free profile, _SHR_E_RESOURCE.
*
*
*      Whether WITH_ID is provided or not, on success the int pointed to by the
*      first_reference argument will be updated.  It will be set TRUE if the new
*      profile was free before, and FALSE if the new profile was already in use.
*      If the value is TRUE, appropriate resources should be updated by the caller
*      (such as programming the data values to hardware registers).
*
*      It is not valid to specify IGNORE_DATA without WITH_ID.
*  \see
*    dnx_algo_template_allocate
*    dnx_algo_template_free
*    shr_error_e
*****************************************************/
shr_error_e dnx_algo_template_exchange(
    int unit,
    int core_id,
    dnx_algo_template_name_t name,
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
*    \param [in] core_id -
*      Relevant core. If template is per core (created with DNX_ALGO_TEMPLATE_CREATE_DUPLICATE_PER_CORE flag),
*        it must be a valid core id, otherwise, must be _SHR_CORE_ANY.
*    \param [in] name -
*      Name used to create this template.
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
*    dnx_algo_template_create_data_t
*    dnx_algo_template_advanced_alogrithm_cb_t
*    shr_error_e
*****************************************************/
shr_error_e dnx_algo_template_profile_data_get(
    int unit,
    int core_id,
    dnx_algo_template_name_t name,
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
*    \param [in] core_id -
*      Relevant core. If template is per core (created with DNX_ALGO_TEMPLATE_CREATE_DUPLICATE_PER_CORE flag),
*         it must be a valid core id, otherwise, must be _SHR_CORE_ANY.
*    \param [in] name -
*      Name used to create this template.
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
*    dnx_algo_template_create_data_t
*    dnx_algo_template_advanced_alogrithm_cb_t
*    shr_error_e
*****************************************************/
shr_error_e dnx_template_profile_get(
    int unit,
    int core_id,
    dnx_algo_template_name_t name,
    const void *profile_data,
    int *profile);

/**
* \brief
*   Free a reference to a template profile.
* 
*
*  \par DIRECT INPUT:
*    \param [in] unit -
*      Relevant unit.
*    \param [in] core_id -
*      Relevant core. If template is per core (created with DNX_ALGO_TEMPLATE_CREATE_DUPLICATE_PER_CORE flag),
*         it must be a valid core id, otherwise, must be _SHR_CORE_ANY.
*    \param [in] name -
*      Name used to create this template.
*    \param [in] profile -
*      Profile to be freed.
*    \param [in] last_reference -
*      Pointer to memory to load output into. \n
*      \b As \b output - \n
*        Will be filled with boolean indication of whether it's the last reference to this profile.
*  \par DIRECT OUTPUT:
*    \retval Zero if no error was detected
*    \retval Negative if error was detected. See \ref shr_error_e
*  \remark
*    None
*  \see
*    dnx_algo_template_create_data_t
*    dnx_algo_template_advanced_alogrithm_cb_t
*    shr_error_e
*****************************************************/
shr_error_e dnx_algo_template_free(
    int unit,
    int core_id,
    dnx_algo_template_name_t name,
    int profile,
    uint8 * last_reference);

/**
* \brief
*   Free all reference to all profiles of this template. 
*
*  \par DIRECT INPUT:
*    \param [in] unit -
*      Relevant unit.
*    \param [in] core_id -
*      Relevant core. If template is per core (created with DNX_ALGO_TEMPLATE_CREATE_DUPLICATE_PER_CORE flag),
*         it must be a valid core id, otherwise, must be _SHR_CORE_ANY.
*    \param [in] name -
*      Name used to create this template.
*  \par DIRECT OUTPUT:
*    \retval Zero if no error was detected
*    \retval Negative if error was detected. See \ref shr_error_e
*  \remark
*    None
*  \see
*    dnx_algo_template_create_data_t
*    dnx_algo_template_advanced_alogrithm_cb_t
*    shr_error_e
*****************************************************/
shr_error_e dnx_algo_template_clear(
    int unit,
    int core_id,
    dnx_algo_template_name_t name);

/**
* \brief
*   Initialize the template manager module.
*
*  \par DIRECT INPUT:
*    \param [in] unit -
*      Relevant unit.
*  \par DIRECT OUTPUT:
*    \retval Zero if no error was detected
*    \retval Negative if error was detected. See \ref shr_error_e
*  \remark
*    None
*  \see
*    shr_error_e
*****************************************************/
shr_error_e dnx_algo_template_init(
    int unit);

/**
* \brief
*   Deinitialize the template manager module.
*
*  \par DIRECT INPUT:
*    \param [in] unit -
*      Relevant unit.
*  \par DIRECT OUTPUT:
*    \retval Zero if no error was detected
*    \retval Negative if error was detected. See \ref shr_error_e
*  \remark
*    None
*  \see
*    shr_error_e
*****************************************************/
shr_error_e dnx_algo_template_deinit(
    int unit);

/*
 * Free all references to a profiles. Not yet implemented. 
 */
shr_error_e dnx_algo_template_free_all(
    int unit,
    int core_id,
    dnx_algo_template_name_t name,
    int profile);

/*
 * Iterate over all allocated entries and perform cb. Not yet implemented. 
 */
shr_error_e dnx_algo_template_iterate(
    int unit,
    int core_id,
    dnx_algo_template_name_t name,
    dnx_algo_template_iterate_iter_cb cb);

/*
 * Print all valid entries. Not yet implemented. 
 */
shr_error_e dnx_algo_template_dump(
    int unit,
    int core_id,
    dnx_algo_template_name_t name);

/*
 * Print all valid entries in range. Not yet implemented. 
 */
shr_error_e dnx_algo_template_dump_in_range(
    int unit,
    int core_id,
    dnx_algo_template_name_t name,
    int start,
    int end);

/*
 * } 
 */
#endif/*_ALGO_TEMPLATE_MNGR_API_INCLUDED__*/
