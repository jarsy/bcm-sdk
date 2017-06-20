/*
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
 
/** \file lif_mapping.c
 *
 * Lif mapping database manager for DNX.
 *  
 * Currently not implemented, all functions are placeholders and implement one-to-one mapping. 
 */

#ifdef _ERR_MSG_MODULE_NAME
#error "_ERR_MSG_MODULE_NAME redefined"
#endif

#define _ERR_MSG_MODULE_NAME BSL_LS_BCMDNX_LIF

/*************
 * INCLUDES  *
 *************/
/*
 * { 
 */

#include <shared/shrextend/shrextend_debug.h>

/*
 * } 
 */
/*************
 * DEFINES   *
 *************/
/*
 * { 
 */

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

shr_error_e
dnx_algo_lif_mapping_init(
    int unit)
{
    return _SHR_E_NONE;
}

shr_error_e
dnx_algo_lif_mapping_deinit(
    int unit)
{
    return _SHR_E_NONE;
}

shr_error_e
dnx_algo_lif_mapping_create(
    int unit,
    uint32 flags,
    int global_lif,
    int local_lif)
{
    return _SHR_E_NONE;
}

shr_error_e
dnx_algo_lif_mapping_remove(
    int unit,
    uint32 flags,
    int global_lif,
    int local_lif)
{
    return _SHR_E_NONE;
}

shr_error_e
dnx_algo_lif_mapping_local_to_global_get(
    int unit,
    uint32 flags,
    int local_lif,
    int *global_lif)
{
    *global_lif = local_lif;
    return _SHR_E_NONE;
}

shr_error_e
dnx_algo_lif_mapping_global_to_local_get(
    int unit,
    int flags,
    int global_lif,
    int *local_lif)
{
    *local_lif = global_lif;
    return _SHR_E_NONE;
}

/*
 * } 
 */
