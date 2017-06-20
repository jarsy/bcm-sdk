/*
 * ! \file bcm_int/dnx/l3/l3.h Internal DNX L3 APIs
PIs $Copyright: (c) 2016 Broadcom.
PIs Broadcom Proprietary and Confidential. All rights reserved.$
 */

#ifndef _L3_API_INCLUDED__
/*
 * {
 */
#define _L3_API_INCLUDED__

#ifndef BCM_DNX_SUPPORT
#error "This file is for use by DNX (JR2) family only!"
#endif

/*
 * DEFINES
 * {
 */

#define L3_MAC_ADDR_SIZE_IN_BYTES          (6)

#define NOF_ENTRIES_ENABLERS_VECTOR        (32)

#define NOF_ENABLERS_PROFILE_BITS          (5)
#define NOF_ROUTING_PROFILES_DUPLICATES    (32)

#define IPV4_ENABLER_OFFSET                (1 << 4)
#define IPV4_MC_ENABLER_OFFSET             (1 << 5)
#define IPV6_ENABLER_OFFSET                (1 << 6)
#define IPV6_MC_ENABLER_OFFSET             (1 << 7)
#define MPLS_ENABLER_OFFSET                (1 << 8)

#define MSB_FIVE_OUT_OF_TEN                (0x3E0)
#define LSB_FIVE_OUT_OF_TEN                (0x1F)

#define CALCULATE_VSI_PROFILE(original_profile,default_profile)   ((original_profile & 0x3E0) | (default_profile & 0x1F))

/*
 * }
 */

/**
 * \brief - Initialize L3 module. \n
 *
 * \par DIRECT_INPUT:
 *   \param [in] unit - Unit-ID
 * \par INDIRECT INPUT:
 * \par DIRECT OUTPUT:
 *   shr_error_e
 */
shr_error_e dnx_l3_module_init(
    int unit);

/*
 * }
 */
#endif/*_L3_API_INCLUDED__*/
