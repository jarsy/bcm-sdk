/*! \file simple_bitmap_wrap.h
 * 
 * Internal DNX resource manager APIs 
 * 
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 */
#ifndef ALGO_SIMPLE_BITMAP_WRAP_INCLUDED
/* { */
#define ALGO_SIMPLE_BITMAP_WRAP_INCLUDED

#ifndef BCM_DNX_SUPPORT
#error "This file is for use by DNX (JR2) family only!"
#endif

#include <shared/shrextend/shrextend_debug.h>
#include <bcm_int/dnx/algo/res_mngr/res_mngr_api.h>


/*!
* \brief
*   Create a new instance of simple bitmap resource manager.
* 
*
*  \par DIRECT INPUT:
*    \param [in] unit -
*      Relevant unit.
*    \param [in] create_data -
*      Pointed memory contains setup parameters required for the
*      creation of the resouce. See \ref dnx_algo_res_create_data_t
*    \param [in] extra_arguments
*      Placeholder to fulfill prototype requirement. Should be NULL.
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
*    sw_state_res_bitmap_create
*    shr_error_e
*/
shr_error_e dnx_algo_res_simple_bitmap_create(
    int unit,
    dnx_algo_res_create_data_t * create_data,
    void *extra_arguments,
    int *algo_instance_id);

/*!
* \brief
*   Allocate a free element.
* 
*
*  \par DIRECT INPUT:
*    \param [in] unit -
*      Relevant unit.
*    \param [in] algo_instance_id -
*      Internal id for the simple bitmap algorithm.
*    \param [in] flags -
*      DNX_ALGO_RES_ALLOCATE_* flags
*    \param [in] extra_arguments -
*      Placeholder to fulfill prototype requirement. Should be NULL.
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
*    sw_state_res_bitmap_alloc
*    shr_error_e
*****************************************************/
shr_error_e dnx_algo_res_simple_bitmap_allocate(
    int unit,
    int algo_instance_id,
    uint32 flags,
    void *extra_arguments,
    int *element);

/*!
* \brief
*   Check whether an element was allocated.
* 
*
*  \par DIRECT INPUT:
*    \param [in] unit -
*      Relevant unit.
*    \param [in] algo_instance_id -
*      Internal id for the simple bitmap algorithm.
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
*    sw_state_res_bitmap_check
*    shr_error_e
*****************************************************/
shr_error_e dnx_algo_res_simple_bitmap_is_allocated(
    int unit,
    int algo_instance_id,
    int element,
    uint8 * is_allocated);

/*!
* \brief
*   Free allocated element.
* 
*
*  \par DIRECT INPUT:
*    \param [in] unit -
*      Relevant unit.
*    \param [in] algo_instance_id -
*      Internal id for the simple bitmap algorithm.
*    \param [in] element -
*      Element to be freed.
*  \par DIRECT OUTPUT:
*    shr_error_e - 
*      Error return value
*  \remark
*    None
*  \see
*    dnx_algo_res_free
*    sw_state_res_bitmap_free
*    shr_error_e
*****************************************************/
shr_error_e dnx_algo_res_simple_bitmap_free(
    int unit,
    int algo_instance_id,
    int element);

/*!
 * Free all elements. 
 * Not yet implemented. 
 *  
 * \see
 * dnx_algo_res_simple_bitmap_free_all
 */
shr_error_e dnx_algo_res_simple_bitmap_free_all(
    int unit,
    int algo_instance_id);

/*! 
 * Iterate over all allocated entries and perform cb. 
 * Not yet implemented. 
 *  
 * \see
 * dnx_algo_res_simple_bitmap_iterate
 */
shr_error_e dnx_algo_res_simple_bitmap_iterate(
    int unit,
    int algo_instance_id,
    dnx_algo_res_iterate_iter_cb cb);

/*! 
 * Print all valid entries. 
 * Not yet implemented. 
 *  
 * \see
 * dnx_algo_res_simple_bitmap_dump
 */
shr_error_e dnx_algo_res_simple_bitmap_dump(
    int unit,
    int algo_instance_id);

/*! 
 * Print all valid entries in range. 
 * Not yet implemented. 
 *  
 * \see
 * dnx_algo_res_simple_bitmap_dump_in_range
 */
shr_error_e dnx_algo_res_simple_bitmap_dump_in_range(
    int unit,
    int algo_instance_id,
    int first_element,
    int end);
/*! @}*/

#endif /*ALGO_SIMPLE_BITMAP_WRAP_INCLUDED */
