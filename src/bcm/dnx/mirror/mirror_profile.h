/*
 * ! \file bcm_int/dnx/mirror/mirror_profile.h Internal DNX MIRROR PROFILE APIs
PIs $Copyright: (c) 2016 Broadcom.
PIs Broadcom Proprietary and Confidential. All rights reserved.$ 
 */

#ifndef _DNX_MIRROR_PROFILE_INCLUDED__
/*
 * { 
 */
#define _DNX_MIRROR_PROFILE_INCLUDED__

#ifndef BCM_DNX_SUPPORT
#error "This file is for use by DNX (JR2) family only!"
#endif

#include <bcm/mirror.h>
#include <bcm_int/dnx/algo/mirror/algo_mirror.h>

/*
 * MACROs
 * {
 */

/*
 * }
 */

/**
 * \brief - Allocatae and set in HW a mirror profile
 * 
 * \par DIRECT_INPUT:
 *   \param [in] unit - Unit ID
 *   \param [in] mirror_dest - Mirror profile attributes
 *   
 * \par INDIRECT INPUT:
 *   * None
 * \par DIRECT OUTPUT:
 *   shr_error_e 
 * \par INDIRECT OUTPUT
 *   * None
 * \remark
 *   * This function isn't fully documented, the main API is.
 * \see
 *   * For more information see the BCM API \ref bcm_dnx_mirror_destination_create.
 */
shr_error_e dnx_mirror_profile_create(
    int unit,
    bcm_mirror_destination_t * mirror_dest);

/**
 * \brief - Get mirror profile attributes (From HW and SW)
 * 
 * \par DIRECT_INPUT:
 *   \param [in] unit - Unit ID
 *   \param [in] mirror_dest_id - Mirror profile gport
 *   \param [in] mirror_dest - pointer to mirror profie attributes to fill
 *   
 * \par INDIRECT INPUT:
 *   * None
 * \par DIRECT OUTPUT:
 *   shr_error_e 
 * \par INDIRECT OUTPUT
 *   * None
 * \remark
 *   * This function isn't fully documented, the main API is.
 * \see
 *   * For more information see the BCM API \ref bcm_dnx_mirror_destination_get. 
 */
shr_error_e dnx_mirror_profile_get(
    int unit,
    bcm_gport_t mirror_dest_id,
    bcm_mirror_destination_t * mirror_dest);

/**
 * \brief - Deallocate a mirror profile and roll back HW to default values
 * 
 * \par DIRECT_INPUT:
 *   \param [in] unit - Unit ID
 *   \param [in] mirror_dest_id - Mirror profile gport
 *   
 * \par INDIRECT INPUT:
 *   * None
 * \par DIRECT OUTPUT:
 *   shr_error_e 
 * \par INDIRECT OUTPUT
 *   * None
 * \remark
 *   * None
 * \see
 *   * For more information see the BCM API \ref bcm_dnx_mirror_destination_destroy
 */
shr_error_e dnx_mirror_profile_destroy(
    int unit,
    bcm_gport_t mirror_dest_id);

/*
 * } 
 */
#endif/*_DNX_MIRROR_PROFILE_INCLUDED__*/
