/** \file port_pp.h
 * $Id$
 * 
 * Internal DNX Port APIs 
 * 
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 */

#ifndef _PORT_PP_API_INCLUDED__
/*
 * { 
 */
#define _PORT_PP_API_INCLUDED__

#ifndef BCM_DNX_SUPPORT
#error "This file is for use by DNX (JR2) family only!"
#endif

#include <bcm/port.h>

/**
 * \brief Set VLAN domain on a port.
 *
 * This function is used to set both ingress and egress VLAN domain \n
 * HW tables on given pp port.
 *
 * \par DIRECT INPUT:
 *   \param [in] unit unit gport belongs to
 *   \param [in] port port to set vlan domain on
 *   \param [in] vlan_domain required VLAN domain
 * \par DIRECT OUTPUT:
 *   \retval Error indication according to shr_error_e enum
 */
shr_error_e dnx_port_pp_vlan_domain_set(
    int unit,
    bcm_port_t port,
    uint32 vlan_domain);


shr_error_e dnx_port_pp_vlan_domain_get(
    int unit,
    bcm_port_t port,
    uint32 * vlan_domain_p);

/**
 * \brief
 * Configure Default Port Match, called by 
 * bcm_dnx_port_match_add in case match criteria is PORT 
 *
 * \par DIRECT INPUT
 *    \param [in] unit -
 *     Relevant unit.
 *   \param [in] port -
 *     port - gport
 *   \param [in] match_info - bcm_dnx_port_match_add param.
 * \par DIRECT OUTPUT:
 *   \retval Negative in case of an error. See shr_error_e, for example: MAC table is full
 *   \retval Zero in case of NO ERROR
 */
shr_error_e dnx_port_pp_egress_match_port_add(
    int unit,
    bcm_gport_t port,
    bcm_port_match_info_t * match_info);

/**
 * \brief -
 * Initialize PP port module. As part of the initialization, the 
 * function configure default VLAN membership if values, called 
 * durring initialization of the device.
 *
 * \par DIRECT INPUT:
 *    \param [in] unit -
 *     Relevant unit.
 * \par DIRECT OUTPUT:
 *    \retval Error indication according to shr_error_e enum
 * \par INDIRECT OUTPUT:
 *    \retval
 *    Set default 1:1 mapping for VLAN membership according to
 *    Port x VLAN
 */
shr_error_e dnx_pp_port_init(
    int unit);

/**
 * \brief
 * Get the VLAN Membership-IF from local port.
 *
 * \par DIRECT INPUT:
 *    \param [in] unit -
 *     Relevant unit.
 *   \param [in] port -
 *     port - local port
 * \par INDIRECT INPUT:
 *   * None
 * \par DIRECT OUTPUT:
 *   \retval Negative in case of an error. See shr_error_e.
 *   \retval Zero in case of NO ERROR
 * \par INDIRECT OUTPUT
 *   * Read from Ingress Port table.
 * \remarks
 *  We assume that VLAN-membership-IF mapping from local port is
 *  symmetric.Because of that, it is enough to get the
 *  information from Ingress Port table.
 */
shr_error_e dnx_port_pp_vlan_membership_if_get(
    int unit,
    bcm_port_t port,
    uint32 * vlan_mem_if);

/**
 * \brief - Set the VLAN Membership-IF per local port.
 *
 * \par DIRECT_INPUT:
 *   \param [in] unit -
 *   Relevant unit.
 *   \param [in] port -
 *   Port - local port
 *   \param [in] vlan_mem_if -
 *   VLAN-membership-if
 * \par INDIRECT INPUT:
     None
 * \par DIRECT OUTPUT:
    None
 * \par INDIRECT OUTPUT
     Write to Ingress/Egress PORT TABLE
 * \remark
 *  We assume that VLAN-membership-IF mapping per local port is
 *  symmetric. Because of that, we need to set the appropriate
 *  information to Ingress and Egress Port tables.
 * \see
 */
shr_error_e dnx_port_pp_vlan_membership_if_set(
    int unit,
    bcm_port_t port,
    uint32 vlan_mem_if);
/*
 * }
 */
#endif/*_PORT_PP_API_INCLUDED__*/
