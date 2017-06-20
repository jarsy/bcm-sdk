/*
 * ! \file algo_port.h Internal DNX Port Managment APIs
PIs $Copyright: (c) 2016 Broadcom.
PIs Broadcom Proprietary and Confidential. All rights reserved.$
 */

#ifndef ALGO_PORT_H_INCLUDED
/*
 * {
 */
#define ALGO_PORT_H_INCLUDED

#ifndef BCM_DNX_SUPPORT
#error "This file is for use by DNX (JR2) family only!"
#endif

/**
 * \brief - Utility provided to map local port pbmp to PP port 
 *       pbmp
 * 
 * \par DIRECT_INPUT:
 *   \param [in] unit - Relevant unit.
 *   \param [in] pbmp - logical ports bitmap.
 *   \param [in] pbmp_pp_arr - An array of PP ports bitmap per core. Will be cleared and filled by the procedure.
 * \par INDIRECT INPUT:
 *   * None
 * \par DIRECT OUTPUT:
 *   int Error in case member in pbmp is not valid, see more infromation in \n
 *   \see \ref dnx_algo_gpm_gport_phy_info_get \n
 *   int pbmp_pp_arr - See DIRECT_INPUT
 * \par INDIRECT OUTPUT
 *   * None
 * \remark
 *   * None
 * \see
 *   * For PBMP usage and its types see include/bcm/types.h
 */
shr_error_e algo_port_pbmp_to_pp_pbmp(
    int unit,
    bcm_pbmp_t pbmp,
    bcm_pbmp_t * pbmp_pp_arr);

/*
 * }
 */

/**
 * \brief - Utility provided to map PP port to  pbmp
 *
 * \par DIRECT_INPUT:
 *   \param [in] unit - Relevant unit.
 *   \param [in] pbmp_pp_arr - An array of PP ports bitmap per core.
 *   \param [in] pbmp - local ports bitmap.
 * \par INDIRECT INPUT:
 *   * None
 * \par DIRECT OUTPUT:
 *   int pbmp -Bitmap of the local ports
 * \par INDIRECT OUTPUT
 *   * None
 * \remark
 *   * Relies on a function that maps PP port to local port
 * \see
 *   * For PBMP usage and its types see include/bcm/types.h
 */

shr_error_e algo_port_pp_pbmp_to_local_pbmp(
    int unit,
    bcm_pbmp_t * pbmp_pp_arr,
    bcm_pbmp_t * pbmp);

/**
 * \brief - Utility provided to map PP port to local port
 *
 * \par DIRECT_INPUT:
 *   \param [in] unit -
 *  Relevant unit.
 *   \param [in] core_id -
 *  Relevant core id.
 *   \param [in] pp_port -
 *  Provided PP port.
 *   \param [in] local_port -
 *   Local port to be mapped.
 * \par INDIRECT INPUT:
     None
 * \par DIRECT OUTPUT:
 *   shr_error_e
 * \par INDIRECT OUTPUT
     None
 * \remark
 *   The mapping will be changed in the future to match to HW
 *   configuration. Currently since no HW configuration, there
 *   is a dummy mapping.
 * \see
     None
 */
shr_error_e algo_port_pp_to_local_port(
    int unit,
    int core_id,
    bcm_port_t pp_port,
    bcm_gport_t * local_port);

/**
 * \brief - Utility provided to map local port to PP port
 *
 * \par DIRECT_INPUT:
 *   \param [in] unit - Relevant unit.
 *   \param [in] local_port - Provided local port.
 *   \param [in] pp_port - Provided PP port.
 *   \param [in] core_id - Relevant Core id.
 * \par INDIRECT INPUT:
 *   * None
 * \par DIRECT OUTPUT:
 *   bcm_gport_t pp_port- the mapped PP port
 *   int core_id - core_id
 * \par INDIRECT OUTPUT
 *   * None
 * \remark
 *   The mapping will be changed in the future to match to HW configuration. Currently since no HW configuration, there is a dummy mapping.
 * \see
 *   * None
 */
shr_error_e algo_local_port_to_pp_port(
    int unit,
    bcm_port_t local_port,
    bcm_gport_t * pp_port,
    int *core_id);

#endif/*_ALGO_PORT_API_INCLUDED__*/
