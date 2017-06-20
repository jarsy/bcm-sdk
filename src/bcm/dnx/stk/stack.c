/*! \file stack.c
 *
 * Stacking procedures for DNX.
 *
 * Here add DESCRIPTION.
 */
/*
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#ifdef BSL_LOG_MODULE
#  error "BSL_LOG_MODULE redefined"
#endif
#define BSL_LOG_MODULE BSL_LS_BCMDNX_STK
/*
 * Include files which are specifically for DNX. Final location.
 * {
 */
#include <shared/shrextend/shrextend_debug.h>
/*
 * }
 */
/*
 * Include files currently used for DNX. To be modified and moved to
 * final location.
 * {
 */
#include <soc/dnx/dbal/dbal.h>

/*
 * }
 */
/*
 * Include files.
 * {
 */
#include <shared/bslenum.h>
#include <bcm/error.h>
#include <bcm/types.h>
#include <bcm/stack.h>
/*
 * }
 */


/*!
 * \brief
 * Get the module ID of the local device 
 *
 * \par DIRECT INPUT
 *    \param [in] unit -
 *     Relevant unit.
 *    \param [in] modid - Output param. 
 *     Pointer to module ID of the local device to be filled by the function.
 * \par INDIRECT INPUT:
 *   * None
 * \par DIRECT OUTPUT:
 *   \retval Negative in case of an error. See shr_error_e, for example: MAC table is full
 *   \retval Zero in case of NO ERROR
 * \par INDIRECT OUTPUT
 *   * None
 */
int
bcm_dnx_stk_modid_get(
    int unit, 
    int *modid)
{
  SHR_FUNC_INIT_VARS(unit);

    
  *modid = 0;

  SHR_EXIT();

exit:
  SHR_FUNC_EXIT;
}
