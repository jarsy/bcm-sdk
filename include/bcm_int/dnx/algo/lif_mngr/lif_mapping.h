/*
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
 
/** \file lif_mapping.h
 * $Id$ 
 * 
 * This file contains the APIs required to save and get global to local lif mapping. 
 *  
 * In the DNX line of devices, lifs are used as logical interfaces on which packets are 
 *   recieved and sent. The index for the physical tables that hold these lifs (inlif table
 *   and EEDB) are refered to as local lifs. However, the identifier of these lifs in the
 *   rest of the system is detached from the physical table index, and is refered to as the
 *   global lif.
 * Logically, Each allocated local lif is mapped to an allocated global lif, and vice versa. 
 *   However, the HW only keeps two of these mappings: Ingress local to global, and egress
 *   global to local. SW users need the missing two mappings: Ingress global to local and
 *   egress local to global. This moduel keeps this mapping. In addition, it shadows the
 *   mappings that exist in the HW, to save HW calls during the driver's run.
 *  
 */

#ifndef  INCLUDE_LIF_MAPPING_H
#define  INCLUDE_LIF_MAPPING_H

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
 * Flags for lif mapping.
 *  
 * \see 
 *  lif_mapping_create
 *  lif_mapping_remove
 *  lif_mapping_local_to_global_get
 *  lif_mapping_global_to_local_get
 */
/**
 * Create / get / remove ingress lif mapping.
 */
#define DNX_ALGO_LIF_INGRESS             SAL_BIT(0)
/**
 * Create / get / remove egress lif mapping.
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
 *   Init lif mapping module.
 *  
 * \par DIRECT INPUT
 *   \param [in] unit -
 *     Identifier of the device to access.
 * \par INDIRECT INPUT
 *   Lif submodule of DNX data, used to determine map sizes according to HW variables.
 * \par DIRECT OUTPUT
 *   \retval Zero if no error was detected
 *   \retval Negative if error was detected. See \ref shr_error_e
 * \par INDIRECT OUTPUT
 *   Ingress and egress global to local and local to global lif maps
 *     in the lif mapping sw state submodule are initialized by this function.
 * \remark
 *   None.
 */
shr_error_e dnx_algo_lif_mapping_init(
    int unit);

/**
 * \brief
 *   Deinit lif mapping module.
 *  
 * \par DIRECT INPUT
 *   \param [in] unit -
 *     Identifier of the device to access.
 * \par INDIRECT INPUT
 *   Ingress and egress global to local and local to global lif maps
 *     in the lif mapping sw state submodule.
 * \par DIRECT OUTPUT
 *   \retval Zero if no error was detected
 *   \retval Negative if error was detected. See \ref shr_error_e
 * \par INDIRECT OUTPUT
 *   Ingress and egress global to local and local to global lif maps
 *     in the lif mapping sw state submodule are deinitialized by this function.
 * \remark
 *   None.
 */
shr_error_e dnx_algo_lif_mapping_deinit(
    int unit);

/**
 * \brief
 *   Create mapping between the given local lif and global lif.
 *  
 * \par DIRECT INPUT
 *   \param [in] unit -
 *     Identifier of the device to access.
 *   \param [in] flags -
 *     Exactly one of \ref DNX_ALGO_LIF_INGRESS or \ref DNX_ALGO_LIF_EGRESS.
 *   \param [in] global_lif -
 *     Global lif to be mapped.
 *   \param [in] local_lif -
 *     Local lif to be mapped.
 * \par INDIRECT INPUT
 *   Ingress and egress global to local and local to global lif maps
 *     in the lif mapping sw state submodule.
 * \par DIRECT OUTPUT
 *   \retval Zero if no error was detected
 *   \retval Negative if error was detected. See \ref shr_error_e
 * \par INDIRECT OUTPUT
 *   Ingress and egress global to local and local to global lif maps
 *     in the lif mapping sw state submodule.
 * \remark
 *   None.
 */
shr_error_e dnx_algo_lif_mapping_create(
    int unit,
    uint32 flags,
    int global_lif,
    int local_lif);

/**
 * \brief
 *   Remove the mapping between the given local lif and global lif.
 *  
 * \par DIRECT INPUT
 *   \param [in] unit -
 *     Identifier of the device to access.
 *   \param [in] flags -
 *     Exactly one of \ref DNX_ALGO_LIF_INGRESS or \ref DNX_ALGO_LIF_EGRESS.
 *   \param [in] global_lif -
 *     Global lif to be unmapped.
 * \par INDIRECT INPUT
 *   Lif mapping sw state submodule.
 * \par DIRECT OUTPUT
 *   \retval Zero if no error was detected
 *   \retval Negative if error was detected. See \ref shr_error_e
 * \par INDIRECT OUTPUT
 *   Lif mapping sw state submodule.
 * \remark
 *   None.
 */
shr_error_e dnx_algo_lif_mapping_remove(
    int unit,
    uint32 flags,
    int global_lif);

/**
 * \brief
 *   Get the global lif mapped to this local lif.
 *  
 * \par DIRECT INPUT
 *   \param [in] unit -
 *     Identifier of the device to access.
 *   \param [in] flags -
 *     Exactly one of \ref DNX_ALGO_LIF_INGRESS or \ref DNX_ALGO_LIF_EGRESS.
 *   \param [in] local_lif -
 *     The local lif whose mapping we require.
 *   \param [in] global_lif -
 *     Int pointer to memory to write output into. \n
 *     \b As \b output - \n
 *       This procedure loads pointed memory with the global lif mapped
 *       from the given local lif.
 * \par INDIRECT INPUT
 *   Ingress and egress local to global lif maps in the lif mapping sw state submodule.
 * \par DIRECT OUTPUT 
 *   \retval \ref _SHR_E_NOT_FOUND if the global lif's mapping doesn't exist. 
 *   \retval Zero if no error was detected
 *   \retval Negative if error was detected. See \ref shr_error_e
 * \par INDIRECT OUTPUT
 *   \b *global_lif -\n
 *     See DIRECT INPUT above
 * \remark
 *   None.
 */
shr_error_e dnx_algo_lif_mapping_local_to_global_get(
    int unit,
    uint32 flags,
    int local_lif,
    int *global_lif);

/**
 * \brief
 *   Get the local lif mapped to this global lif.
 *  
 * \par DIRECT INPUT
 *   \param [in] unit -
 *     Identifier of the device to access.
 *   \param [in] flags -
 *     Exactly one of \ref DNX_ALGO_LIF_INGRESS or \ref DNX_ALGO_LIF_EGRESS.
 *   \param [in] global_lif -
 *     The global lif whose mapping we require.
 *   \param [in] local_lif -
 *     Int pointer to memory to write output into. \n
 *     \b As \b output - \n
 *       This procedure loads pointed memory with the local lif mapped
 *       from the given global lif.
 * \par INDIRECT INPUT
 *   Ingress and egress global to local lif maps in the lif mapping sw state submodule.
 * \par DIRECT OUTPUT 
 *   \retval \ref _SHR_E_NOT_FOUND if the global lif's mapping doesn't exist. 
 *   \retval Zero if no error was detected
 *   \retval Negative if error was detected. See \ref shr_error_e
 * \par INDIRECT OUTPUT
 *   \b *local_lif -\n
 *     See DIRECT INPUT above
 * \remark
 *   None.
 */
shr_error_e dnx_algo_lif_mapping_global_to_local_get(
    int unit,
    int flags,
    int global_lif,
    int *local_lif);

/*
 * } 
 */

#endif /* INCLUDE_LIF_MAPPING_H */
