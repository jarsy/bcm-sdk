/** \file algo_mirror.h
 * 
 * Internal DNX resource manager APIs 
 * 
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 */
#ifndef _ALGO_MIRROR_INCLUDED__
/*
 * { 
 */
#define _ALGO_MIRROR_INCLUDED__

#ifndef BCM_DNX_SUPPORT
#error "This file is for use by DNX (JR2) family only!"
#endif

/*
 * INCLUDE FILES:
 * {
 */

#include <shared/shrextend/shrextend_error.h>
#include <bcm/mirror.h>

/*
 * }
 */

/**
 * \brief - calculates the HW field for inbound/outbound mirroring/snooping probability. \n
 * The probability of mirroring a packet is: \n 
 * dividend >= divisor ? 1 : dividend / divisor \n 
 *  
 * The HW field is calculated as following. Given probability 'P' and HW field nof bits is PROB_NOF_BITS: \n 
 *  P = (2^PROB_NOF_BITS-prob_hw_field)/2^PROB_NOF_BITS
 * 
 * \par DIRECT_INPUT:
 *   \param [in] unit - unit id
 *   \param [in] dividend - dividend
 *   \param [in] divisor - divisor
 *   \param [in] prob_nof_bits - number of HW bits
 *   \param [in] prob_field - pointer to calculated HW field (probability)
 *   
 * \par INDIRECT INPUT:
 *   * None
 * \par DIRECT OUTPUT:
 *   shr_error_e 
 * \par INDIRECT OUTPUT
 *   * prob_field - pointer to calculated probability
 * \remark
 *   * None
 * \see
 *   * None
 */
shr_error_e dnx_algo_mirror_probability_get(
    int unit,
    uint32 dividend,
    uint32 divisor,
    int prob_nof_bits,
    uint32 * prob_field);

/**
 * \brief - allocate new sniffing profile. Each application (mirror, snoop, statistical sample) has its own 
 * allocation manager. 
 * 
 * \par DIRECT_INPUT:
 *   \param [in] unit - Unit ID
 *   \param [in] mirror_dest - Mirror profile attributes
 *   \param [in] action_profile_id - Mirror profile ID
 *   
 * \par INDIRECT INPUT:
 *   * None
 * \par DIRECT OUTPUT:
 *   shr_error_e 
 * \par INDIRECT OUTPUT
 *   * SNIF Resource manager - SNIF_INGRESS_MIRROR_PROFILES
 *  
 * \remark
 *   * None
 * \see
 *   * None
 */
shr_error_e dnx_algo_mirror_profile_allocate(
    int unit,
    bcm_mirror_destination_t * mirror_dest,
    int *action_profile_id);

/**
 * \brief - deallocate snif profile. Each application (mirror, snoop, statistical sample) has its own 
 * allocation manager.  
 * 
 * \par DIRECT_INPUT:
 *   \param [in] unit - Unit ID
 *   \param [in] mirror_dest_id - gport of mirror profile 
 *   
 * \par INDIRECT INPUT:
 *   * None
 * \par DIRECT OUTPUT:
 *   shr_error_e 
 * \par INDIRECT OUTPUT
 *   * SNIF Resource manager
 * \remark
 *   * None
 * \see
 *   * None
 */
shr_error_e dnx_algo_mirror_profile_deallocate(
    int unit,
    bcm_gport_t mirror_dest_id);

/**
 * \brief - Check if given snif profile is already allocated in resource manager
 * 
 * \par DIRECT_INPUT:
 *   \param [in] unit - Unit Id
 *   \param [in] mirror_dest_id - gport of mirror profile
 *   \param [in] is_allocated - return value:
 *     * 1 - profile is allocated \n
 *       0 - profile is not allocated
 *   
 * \par INDIRECT INPUT:
 *   * Snif allocation manager
 * \par DIRECT OUTPUT:
 *   shr_error_e 
 * \par INDIRECT OUTPUT
 *   * None
 * \remark
 *   * None
 * \see
 *   * dnx_algo_mirror_profile_deallocate(), dnx_algo_mirror_profile_allocate() functions \n
 *     for allocation and deallocation. 
 */
shr_error_e dnx_algo_mirror_profile_is_allocated(
    int unit,
    bcm_gport_t mirror_dest_id,
    uint8 * is_allocated);

/**
 * \brief - Initialize mirror algorithms
 * 
 * \par DIRECT_INPUT:
 *   \param [in] unit - Unit ID
 *   
 * \par INDIRECT INPUT:
 *   * None
 * \par DIRECT OUTPUT:
 *   shr_error_e 
 * \par INDIRECT OUTPUT
 *   * Snif resource manager
 * \remark
 *   * None
 * \see
 *   * None
 */
shr_error_e dnx_algo_mirror_init(
    int unit);

/**
 * \brief - Deinitialize mirror algorithms
 * 
 * \par DIRECT_INPUT:
 *   \param [in] unit - Unit ID
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
 *   * None
 */
shr_error_e dnx_algo_mirror_deinit(
    int unit);

/*
 * MACROs
 * {
 */

/**
 * \brief - Minimal snif profile ID, 0 is reserved for default action (no SNIF). \n 
 * The maximal value is extracted from DNX DATA. See dnx_data_snif.ingress.nof_profiles_get(unit). 
 */
#define DNX_ALGO_MIRROR_INGRESS_PROFILE_MIN 1
/**
 * \brief - Maximal snif profile ID. 
 * This value is extracted from DNX DATA, according to nof_profiles.
 */
#define DNX_ALGO_MIRROR_INGRESS_PROFILE_MAX(unit) (dnx_data_snif.ingress.nof_profiles_get(unit) - 1)
/**
 * \brief - Default mirror profile, indicates "no mirrorong" 
 */
#define DNX_ALGO_MIRROR_INGRESS_PROFILE_DEFAULT 0

/**
 * \brief - res mngr for mirror application 
 */
#define DNX_ALGO_SNIF_RES_MNGR_INGRESS_PROFILES_MIRROR        "SNIF_INGRESS_PROFILES_MIRROR"
/**
 * \brief - res mngr for snoop application 
 */
#define DNX_ALGO_SNIF_RES_MNGR_INGRESS_PROFILES_SNOOP         "SNIF_INGRESS_PROFILES_SNOOP"
/**
 * \brief - res mngr for statistical sampling application 
 */
#define DNX_ALGO_SNIF_RES_MNGR_INGRESS_PROFILES_STAT_SAMPLING "SNIF_INGRESS_PROFILES_STAT_SAMPLING"

/*
 * }
 */

/*
 * } 
 */
#endif/*_ALGO_MIRROR_INCLUDED__*/
