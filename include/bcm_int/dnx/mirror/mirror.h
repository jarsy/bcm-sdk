/*
 * ! \file bcm_int/dnx/mirror/mirror.h Internal DNX MIRROR APIs
PIs $Copyright: (c) 2016 Broadcom.
PIs Broadcom Proprietary and Confidential. All rights reserved.$ 
 */

#ifndef _DNX_MIRROR_API_INCLUDED__
/*
 * { 
 */
#define _DNX_MIRROR_API_INCLUDED__

#ifndef BCM_DNX_SUPPORT
#error "This file is for use by DNX (JR2) family only!"
#endif

/**
 * \brief - Initialize dnx mirror module
 * 
 * \par DIRECT_INPUT:
 *   \param [in] unit -  Unit-ID 
 *   
 * \par INDIRECT INPUT:
 *   * DNX data related SNIF module information
 * \par DIRECT OUTPUT:
 *   shr_error_e 
 * \par INDIRECT OUTPUT
 *   * Mirror HW related regs/mems
 * \remark
 *   * None
 * \see
 *   * None
 */
shr_error_e dnx_mirror_init(
    int unit);

/**
 * \brief - Deinitialize dnx mirror module
 * 
 * \par DIRECT_INPUT:
 *   \param [in] unit -  Unit-ID 
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
shr_error_e dnx_mirror_deinit(
    int unit);

/*
 * MACROs
 * {
 */

/*
 * }
 */

/*
 * } 
 */
#endif/*_DNX_MIRROR_API_INCLUDED__*/
