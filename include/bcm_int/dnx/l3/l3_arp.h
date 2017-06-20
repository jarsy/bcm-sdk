/** \file bcm_int/dnx/l3/l3_arp.h
 * 
 * Internal DNX L3 ARP APIs 
 * 
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 */

#ifndef _L3_ARP_API_INCLUDED__
/*
 * { 
 */
#define _L3_ARP_API_INCLUDED__

#ifndef BCM_DNX_SUPPORT
#error "This file is for use by DNX (JR2) family only!"
#endif

#include <bcm/l3.h>

/*
 * MACROs
 * {
 */

/*
 * }
 */

/*
 * FUNCTIONS
 * {
 */

/**
* \brief
*   Verify function for bcm_l3_egress_create with
*   BCM_L3_EGRESS_ONLY flag.
*
*  \par DIRECT INPUT:
*    \param [in] unit -
*      Relevant unit.
*    \param [in] flags -
*      Similar to bcm_l3_egress_create api input
*    \param [in] egr -
*      Similar to bcm_l3_egress_create api input
*    \param [in] if_id -
*      Similar to bcm_l3_egress_create api input
*  \par DIRECT OUTPUT:
*    shr_error_e - 
*      Error return value
*  \remark
*    None
*  \see
*    shr_error_e
*****************************************************/
shr_error_e dnx_l3_egress_create_arp_verify(
    int unit,
    uint32 flags,
    bcm_l3_egress_t * egr,
    bcm_if_t * if_id);

/**
* \brief
*   Allocation function for bcm_l3_egress_create with
*   BCM_L3_EGRESS_ONLY flag.
*   Allocats global and local lif.
*   Allocated index will be filled inside encap_id parameter.
*   Also ll_local_lif, ll_global_lif store the allocation result.
*
*  \par DIRECT INPUT:
*    \param [in] unit -
*      Relevant unit.
*    \param [in] flags -
*      Similar to bcm_l3_egress_create api input
*    \param [in] egr -
*      Similar to bcm_l3_egress_create api input
*    \param [in] if_id -
*      Similar to bcm_l3_egress_create api input
*    \param [in] ll_local_lif -
*      Pointer to location to be loaded by ll_local_lif.
*    \param [in] ll_global_lif -
*      Pointer to location to be loaded by ll_global_lif.
*  \par INDIRECT OUTPUT:
*    \param [out] *ll_local_lif -
*      Local lif id that was allocated.
*    \param [out] *ll_global_lif -
*      Global lif id that was allocated.
*    shr_error_e - 
*      Error return value
*  \remark
*    None
*  \see
*    shr_error_e
*    dnx_l3_egress_create_arp_hw_write
*****************************************************/
shr_error_e dnx_l3_egress_create_arp_allocate(
    int unit,
    uint32 flags,
    bcm_l3_egress_t * egr,
    bcm_if_t * if_id,
    uint32 * ll_local_lif,
    uint32 * ll_global_lif);

/**
* \brief
*   HW tables write function for bcm_l3_egress_create with
*   BCM_L3_EGRESS_ONLY flag.
*   Fills ARP EEDB entry with index ll_local_lif.
*
*  \par DIRECT INPUT:
*    \param [in] unit -
*      Relevant unit.
*    \param [in] flags -
*      Similar to bcm_l3_egress_create api input
*    \param [in] egr -
*      Similar to bcm_l3_egress_create api input
*    \param [in] if_id -
*      Similar to bcm_l3_egress_create api input
*    \param [in] ll_local_lif -
*      Local lif id allocated for ARP
*  \par DIRECT OUTPUT:
*    shr_error_e - 
*      Error return value
*  \remark
*    None
*  \see
*    shr_error_e
*    dnx_l3_egress_create_arp_allocate
*****************************************************/
shr_error_e dnx_l3_egress_create_arp_hw_write(
    int unit,
    uint32 flags,
    bcm_l3_egress_t * egr,
    bcm_if_t * if_id,
    uint32 ll_local_lif);

/*
 * }
 */

/*
 * } 
 */
#endif/*_L3_ARP_API_INCLUDED__*/
