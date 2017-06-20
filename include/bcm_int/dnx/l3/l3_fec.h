/*
 * ! \file bcm_int/dnx/l3/l3_fec.h Internal DNX L3 FEC APIs
PIs $Copyright: (c) 2016 Broadcom.
PIs Broadcom Proprietary and Confidential. All rights reserved.$ 
 */

#ifndef _L3_FEC_API_INCLUDED__
/*
 * { 
 */
#define _L3_FEC_API_INCLUDED__

#ifndef BCM_DNX_SUPPORT
#error "This file is for use by DNX (JR2) family only!"
#endif

#include <bcm/l3.h>

/*
 * FUNCTIONS
 * {
 */

/** 
 * \brief 
 * Verify function for bcm_l3_egress_create with 
 * BCM_L3_INGRESS_ONLY flag. 
 *  
 * \par DIRECT INPUT: 
 *   \param [in] unit - 
 * Relevant unit. 
 *   \param [in] flags - 
 * Similar to bcm_l3_egress_create api input 
 *   \param [in]
 * egr - 
 * Similar to bcm_l3_egress_create api input 
 *   \param [in] if_id - 
 * Similar to bcm_l3_egress_create api input
 *  
 * \par DIRECT OUTPUT:
 * shr_error_e -
 * Error return value
 * \remark 
 * None 
 * \see 
 * shr_error_e
 * ****************************************************
 */
shr_error_e dnx_l3_egress_create_fec_verify(
    int unit,
    uint32 flags,
    bcm_l3_egress_t * egr,
    bcm_if_t * if_id);

/**
* \brief
*   Allocation function for bcm_l3_egress_create with
*   BCM_L3_INGRESS_ONLY flag.
*   Allocats FEC index inside FEC table. 
*   Allocated index will be returned inside if_id parameter.
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
*    dnx_l3_egress_create_arp_hw_write
*****************************************************/
shr_error_e dnx_l3_egress_create_fec_allocate(
    int unit,
    uint32 flags,
    bcm_l3_egress_t * egr,
    bcm_if_t * if_id);

/**
* \brief
*   HW tables write function for bcm_l3_egress_create with
*   BCM_L3_INGRESS_ONLY flag.
*   Fills FEC table of index taken from if_id with destination and OutLif and/or OutRif information.
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
*    dnx_l3_egress_create_fec_allocate
*****************************************************/
shr_error_e dnx_l3_egress_create_fec_hw_write(
    int unit,
    uint32 flags,
    bcm_l3_egress_t * egr,
    bcm_if_t * if_id);

/*
 * }
 */

/*
 * } 
 */
#endif/*_L3_FEC_API_INCLUDED__*/
