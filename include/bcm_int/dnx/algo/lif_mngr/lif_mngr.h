/*
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
 
/** \file lif_mngr.h
 * $Id$ 
 * 
 * This file contains the APIs required for lif algorithms.
 *  
 */

#ifndef  INCLUDE_LIF_MNGR_H
#define  INCLUDE_LIF_MNGR_H

/*************
 * INCLUDES  *
 *************/
/*
 * { 
 */

/*
 * } 
 */
/*************
 * DEFINES   *
 *************/
/*
 * { 
 */

/**
 * Flags for lif mngr APIs.
 *  
 * \see 
 *  lif_mapping_create
 *  lif_mapping_remove
 *  lif_mapping_local_to_global_get
 *  lif_mapping_global_to_local_get
 */
/**
 * Perform ingress lif operation.
 */
#define DNX_ALGO_LIF_INGRESS             SAL_BIT(0)
/**
 * Perform egress lif operation.
 */
#define DNX_ALGO_LIF_EGRESS              SAL_BIT(1)

/*
 * } 
 */
/*************
 * MACROS    *
 *************/
/*
 * { 
 */

/*
 * } 
 */
/*************
 * TYPE DEFS *
 *************/
/*
 * { 
 */

/*
 * } 
 */
/*************
 * GLOBALS   *
 *************/
/*
 * { 
 */

/*
 * } 
 */
/*************
 * FUNCTIONS *
 *************/
/*
 * { 
 */

/**
 * \brief
 *   Init lif mngr module, including the lif mappings and global/local lif alloaction
 *   submodules.
 *  
 * \par DIRECT INPUT
 *   \param [in] unit -
 *     Identifier of the device to access.
 * \par INDIRECT INPUT
 *   Lif table sizes from teh lif submodule DNX data, used to initialize lif resources.
 * \par DIRECT OUTPUT
 *   \retval Zero if no error was detected
 *   \retval Negative if error was detected. See \ref shr_error_e
 * \par INDIRECT OUTPUT
 *   Local and gloal lif resouce managers, lif mapping and lif allocation
 *     sw state sub modules are initialized by this function.
 * \remark
 *   None.
 */
shr_error_e dnx_algo_lif_mngr_init(
    int unit);

/**
 * \brief
 *   Deinit lif mngr module, including the lif mappings and global/local lif alloaction
 *   submodules.
 *  
 * \par DIRECT INPUT
 *   \param [in] unit -
 *     Identifier of the device to access.
 * \par INDIRECT INPUT
 *   Local and gloal lif resouce managers, lif mapping and lif allocation
 *     sw state sub modules.
 * \par DIRECT OUTPUT
 *   \retval Zero if no error was detected
 *   \retval Negative if error was detected. See \ref shr_error_e
 * \par INDIRECT OUTPUT
 *   Local and gloal lif resouce managers, lif mapping and lif allocation
 *     sw state sub modules are deinitialized by this function.
 * \remark
 *   None.
 */
shr_error_e dnx_algo_lif_mngr_deinit(
    int unit);

/**
 * \brief
 *     Given an egress global lif id, indicates whether it's a mapped lif, or a direct lif,
 *      according to HW setting.
 *     A non mapped lif (direct lif) is a lif that doesn't go through the GLEM and its
 *     local lif is equal to the global lif. 
 * \par DIRECT INPUT
 *   \param [in] unit -
 *     Identifier of the device to access.
 *   \param [in] global_lif -
 *     Global lif to be tested.
 *   \param [in] is_mapped -
 *     uint8 pointer to memory to write output into. \n
 *     \b As \b output - \n
 *       This procedure loads pointed memory with boolean indication of this lif's \n
 *       mapped/ direct status. TRUE is mapped and FALSE is direct.
 * \par INDIRECT INPUT
 *   Nof_rifs in the l3 device data submodule.
 * \par DIRECT OUTPUT
 *   \retval Zero if no error was detected
 *   \retval Negative if error was detected. See \ref shr_error_e
 * \par INDIRECT OUTPUT
 *   \b *is_mapped -\n
 *     See DIRECT INPUT above
 * \remark
 *   None.
*/
shr_error_e dnx_algo_global_lif_is_mapped(
    int unit,
    int global_lif,
    uint8 * is_mapped);

/*
 * } 
 */

#endif /* INCLUDE_LIF_MNGR_H */
