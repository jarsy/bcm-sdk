/*
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
 
/** \file lif_lib.h
 * $Id$ 
 * 
 * This file contains the APIs required to allocate and delete lifs.
 */

#ifndef  INCLUDE_LIF_LIB_H
#define  INCLUDE_LIF_LIB_H

/*************
 * INCLUDES  *
 *************/
/*
 * { 
 */
#include <bcm_int/dnx/algo/lif_mngr/lif_mapping.h>

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
 * \brief This flag is used to inidicate that the allocated global lif should have a specific ID.
 *  
 * \see  lif_lib_lif_allocate
 */
#define LIF_LIB_GLOBAL_LIF_WITH_ID   SAL_BIT(0)

/** 
 *  \brief Illegal lif indication.
 *  
 *  \see lif_lib_lif_deallocate
 */
#define LIF_LIB_INVALID             (-1)

/*
 * } 
 */
/*************
 * MACROS    *
 *************/
/*
 * { 
 */

/** 
 * \brief General utility macros used to validate that lif IDs are legal. 
 *   The information is taken from the lif module of dnx_device data. 
 *  
 * { 
 */
#define LIF_LIB_GLOBAL_IN_LIF_IS_LEGAL(_unit, _lif_id) \
            (_lif_id < dnx_data_lif.global_lif.nof_global_in_lifs_get(unit)) && (_lif_id >= 0)

#define LIF_LIB_GLOBAL_OUT_LIF_IS_LEGAL(_unit, _lif_id) \
            (_lif_id < dnx_data_lif.global_lif.nof_global_out_lifs_get(unit)) && (_lif_id >= 0)

#define LIF_LIB_LOCAL_IN_LIF_IS_LEGAL(_unit, _lif_id) \
            (_lif_id < dnx_data_lif.in_lif.nof_local_in_lifs_get(unit)) && (_lif_id >= 0)

#define LIF_LIB_LOCAL_OUT_LIF_IS_LEGAL(_unit, _lif_id) \
            (_lif_id < dnx_data_lif.out_lif.nof_local_out_lifs_get(unit)) && (_lif_id >= 0)

/** 
 * }
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

/**
 * \brief Lif application types for inlif allocation.
 *
 * /see
 * lif_lib_lif_allocate
 */
typedef enum
{
    /**
     * Invalid option, can't be used.
     */
    LIF_LIB_INGRESS_LIF_APP_TYPE_INVALID,
    /** 
     * First legal option.
     */
    LIF_LIB_INGRESS_LIF_APP_TYPE_FIRST,
    /** 
     * Local ingress lif of type ac.
     */
    LIF_LIB_INGRESS_LIF_APP_TYPE_AC = LIF_LIB_INGRESS_LIF_APP_TYPE_FIRST,
    /** 
     * Number of ingress lif types in use. 
     */
    LIF_LIB_INGRESS_LIF_APP_TYPE_COUNT
} lif_lib_ingress_lif_app_type_e;

/**
 * \brief Allocation info for local inlif allocation.
 *
 * Pass this struct to lif_lib_lif_allocate to allocate a local lif. 
 * 
 * /see
 * lif_lib_lif_allocate
 */
typedef struct
{
    /** 
     *  Ingress lif application type.
     */
    lif_lib_ingress_lif_app_type_e app_type;
    /** 
     *  Flags for local lif allocation. Currently not in use.
     */
    int local_lif_flags;
    /** 
     *  Local inlif id that was allocated.
     */
    int local_inlif;
} lif_lib_local_inlif_info_t;

/**
 * \brief Lif application types for outlif allocation.
 *
 * /see
 * lif_lib_lif_allocate
 */
typedef enum
{
    /**
     * Invalid option, can't be used.
     */
    LIF_LIB_EGRESS_LIF_APP_TYPE_INVALID,
    /** 
     * First legal lif app type.
     */
    LIF_LIB_EGRESS_LIF_APP_TYPE_FIRST,
    /** 
     * Local egress lif of type ac.
     */
    LIF_LIB_EGRESS_LIF_APP_TYPE_AC = LIF_LIB_EGRESS_LIF_APP_TYPE_FIRST,
    /** 
     * Number of egress lif types in use. 
     */
    LIF_LIB_EGRESS_LIF_APP_TYPE_COUNT
} lif_lib_egress_lif_app_type_e;

typedef struct
{
    /** 
     *  Egress lif application type.
     */
    lif_lib_egress_lif_app_type_e app_type;
    /** 
     *  Flags for local lif allocation. Currently not in use.
     */
    int local_lif_flags;
    /** 
     *  Local outlif id that was allocated.
     */
    int local_outlif;
} lif_lib_local_outlif_info_t;

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
 *   Allocate global lif and local lif, and create mapping between them.
 *  
 *   This API performs several functions:
 *   1. Allocate local and global lifs (ingress and egress, as required).
 *   2. Keep mapping of global to local lifs and vice versa in the sw state.
 *   3. If a new EEDB bank was allocated, set the EEDB bank phase.
 *   4. If an outlif was allocated, fill the GLEM with egress lif mapping.
 *  
 * \par DIRECT INPUT
 *   \param [in] unit -
 *     Identifier of the device to access.
 *   \param [in] flags -
 *     Currently supported flag is \ref DNX_ALGO_RES_ALLOCATE_WITH_ID \n
 *        to allocate the global lif with the ID given in global lif argument.
 *   \param [in] global_lif -
 *     Int pointer to memory to write output into. \n
 *     For \ref DNX_ALGO_RES_ALLOCATE_WITH_ID, this may also be indirect input.
 *     \b As \b output - \n
 *       This procedure loads pointed memory with the global lif allocated. \n
 *       Not used as output when \ref DNX_ALGO_RES_ALLOCATE_WITH_ID is used.
 *     \b As \b input - \n
 *       If flag DNX_ALGO_RES_ALLOCATE_WITH_ID is set, this holds
 *         the id to be allocated.
 *   \param [in] inlif_info -
 *     Pointer to memory for local inlif allocation input/output. \n
 *     If NULL, then this will be ignored, local inlif will not be allocated,
 *        and outlif_info must not be NULL.
 *     \b As \b input - \n
 *       All elements in inlif info are inputs, except for local_inlif.
 *     \b As \b output - \n
 *       This procedure loads the field inlif_info->local_inlif with the local in lif allocated.
 *   \param [in] outlif_info -
 *     Pointer to memory for local outlif allocation input/output. \n
 *     If NULL, then this will be ignored, local outlif will not be allocated,
 *        and inlif_info must not be NULL.
 *     \b As \b input - \n
 *       All elements in outlif info are inputs, except for local_outlif.
 *     \b As \b output - \n
 *       This procedure loads the field outlif_info->local_outlif with the local out lif allocated.
 * \par INDIRECT INPUT 
 *   Resource manager of the ingress, egress and global lifs.
 *   Device data used for input validation.
 *   \b *global_lif \n
 *     See DIRECT INPUT above
 *   \b *inlif_info \n
 *     See DIRECT INPUT above
 *   \b *outlif_info \n
 *     See DIRECT INPUT above
 * \par DIRECT OUTPUT
 *   \retval Zero if no error was detected
 *   \retval Negative if error was detected. See \ref shr_error_e
 * \par INDIRECT OUTPUT 
 *   Resource managers of the ingress, egress and global lifs. 
 *   \b *global_lif \n
 *     See DIRECT INPUT above
 *   \b *inlif_info \n
 *     See DIRECT INPUT above
 *   \b *outlif_info \n
 *     See DIRECT INPUT above
 * \remark
 *   None.
 */
shr_error_e lif_lib_lif_allocate(
    int unit,
    uint32 flags,
    int *global_lif,
    lif_lib_local_inlif_info_t * inlif_info,
    lif_lib_local_outlif_info_t * outlif_info);

/**
 * \brief
 *   Deallocate the given global lif and its local in/outlif.
 *  
 *   All the steps described in lif_lib_lif_allocte will be reverted.
 *  
 * \par DIRECT INPUT
 *   \param [in] unit -
 *     Identifier of the device to access.
 *   \param [in] global_lif -
 *     Global lif to be deallocated.
 *   \param [in] local_inlif -
 *     Local inlif to be deallocated, or LIF_LIB_INVALID to deallocate egress only.
 *   \param [in] local_outlif -
 *     Local outlif to be deallocated, or LIF_LIB_INVALID to deallocate ingress only.
 *  
 * \par INDIRECT INPUT 
 *   Resource manager of the ingress, egress and global lifs.
 *   Device data used for input validation.
 * \par DIRECT OUTPUT
 *   \retval Zero if no error was detected
 *   \retval Negative if error was detected. See \ref shr_error_e
 * \par INDIRECT OUTPUT
 *   Resource manager of the ingress, egress and global lifs.
 * \remark
 *   None.
 */
shr_error_e lif_lib_lif_deallocate(
    int unit,
    int global_lif,
    int local_inlif,
    int local_outlif);

/*
 * } 
 */

#endif /* INCLUDE_LIF_LIB_H */
