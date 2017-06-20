/*! \file template_mngr_types.h
 * 
 * Internal DNX template manager APIs 
 * 
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 */
#ifndef ALGO_TEMPLATE_MNGR_TYPES_INCLUDED
/*
 * { 
 */
#define ALGO_TEMPLATE_MNGR_TYPES_INCLUDED

#ifndef BCM_DNX_SUPPORT
#error "This file is for use by DNX (JR2) family only!"
#endif

/**
 *  
 *   \brief General types for template manager.
 *  
 */

/**
 * \brief The name of the template manager instance, used with all template manager functions. 
 */
typedef char *dnx_algo_template_name_t;

/**
 * \brief Callback to print the data stored in template manager. 
 *      The print should be in this format:
 *      [4 spaces ]< struct name >
 *      [4 spaces ]{
 *      [8 spaces     ]< field name >: < value > < value in hexa >
 *      [8 spaces     ]< sub struct name >:
 *      [8 spaces     ]{
 *      [12 spaces          ]< field name >: < value > < value in hexa >
 *      [8 spaces     ]}
 *      [8 spaces     ]< field name >: < value > < value in hexa >
 *      [4 spaces ]}
 *
 *  \par DIRECT INPUT:
 *    \param [in] data -
 *      Pointer of the struct to be printed.
 *  \par DIRECT OUTPUT:
 *      None
 *  \remark
 *    None
 *  \see
 *    dnx_algo_template_dump
 *****************************************************/
typedef void (
    *dnx_algo_template_print_data_cb) (
    const void *data);

/**
 * \brief Template creation information
 *
 * This structure contains the information required for creating a new template.
 * 
 *  \see 
 * dnx_algo_template_create
 */
typedef struct
{
    /*
     *
     * DNX_ALGO_TEMPLATE_INIT_* flags
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
     * A callback to print the content of the data saved in the template.
     */
    dnx_algo_template_print_data_cb print_cb;
    /*
     * 
     * If flag DNX_ALGO_TEMPLATE_CREATE_USE_DEFAULT_PROFILE is set, put here the data that it will contain.
     */
    void *default_data;
} dnx_algo_template_create_data_t;

/**
 * \brief This is the callback prototype used for iterating over templates. Not yet implemented.
 */
typedef shr_error_e(
    *dnx_algo_template_iterate_iter_cb) (
    int unit,
    int core_id,
    int profile,
    void *extra_arguments);

/*
 * } 
 */
#endif/*_ALGO_TEMPLATE_MNGR_CALLBACKS_INCLUDED__*/
