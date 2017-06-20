/*
 * $Id: error.h,v 1.1.270.1 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * Module: Error translation
 */

#ifndef _SOC_SBX_ERROR_H_
#define _SOC_SBX_ERROR_H_

#include <bcm/debug.h>
#include <bcm/error.h>
#include <soc/sbx/sbx_drv.h>

#include <soc/sbx/sbTypesGlue.h>
#include <soc/sbx/sbElibStatus.h>


#define BCM_SBX_IF_ERROR_RETURN(op)                                     \
    do                            {                                     \
        int __rv__;                                                     \
        if ((__rv__ = translate_sbx_result((op))) < 0)                  \
            return (__rv__);                                            \
    } while (0)


#define BCM_SBX_ELIB_IF_ERROR_RETURN(op)                                \
    do                            {                                     \
        int __rv__;                                                     \
        if ((__rv__ = translate_sbx_elib_result((op))) < 0)             \
            return (__rv__);                                            \
    } while (0)

/* A common debug message prefix */
#define _SBX_D(_u, string)   "[%d:%s]: " string, _u, FUNCTION_NAME()

/* A common unknown microcode warning message */
#define SBX_UNKNOWN_UCODE_WARN(_u)                              \
    LOG_WARN(BSL_LS_SOC_COMMON,                                 \
             (BSL_META_U(_u,                                    \
                    "uCode type %d is not supported\n"),        \
         SOC_SBX_CONTROL(_u)->ucodetype));


/*
 *   Function
 *      translate_sbx_result
 *   Purpose
 *      Translate a Sandburst result code into a Broadcom result code
 *   Parameters
 *      (in) sbStatus_t result = Sandburst result to translate
 *   Returns
 *      bcm_error_t = Closest reasonable Broadcom result
 *   Notes
 *      If there is no reasonably close error, it just guesses and returns
 *      BCM_E_FAIL instead of something more specific.  It is possible that
 *      this result is wrong if there's a 'pending' or similar result that is
 *      not included in the switch statement in this function.  Therefore,
 *      this must be updated whenever BCM_E_* is expanded or SB_* results are
 *      expanded, in order to ensure consistency to the intended behaviour.
 */
int
translate_sbx_result(const sbStatus_t result);

/*
 *   Function
 *      translate_sbx_elib_result
 *   Purpose
 *      Translate a Sandburst ELib result code into a Broadcom result code
 *   Parameters
 *      (in) sbElibStatus_et result = Sandburst ELib result to translate
 *   Returns
 *      bcm_error_t = Closest reasonable Broadcom result
 *   Notes
 *      If there is no reasonably close error, it just guesses and returns
 *      BCM_E_FAIL instead of something more specific.  It is possible that
 *      this result is wrong if there's a 'pending' or similar result that is
 *      not included in the switch statement in this function.  Therefore,
 *      this must be updated whenever BCM_E_* is expanded or SB_* results are
 *      expanded, in order to ensure consistency to the intended behaviour.
 */
int
translate_sbx_elib_result(const sbElibStatus_et result);

/*
 *   Function
 *      sbx_result_string
 *   Purpose
 *      Get the text representation of a Sandburst result code.
 *   Parameters
 *      (in) sbStatus_t sbxResult = Sandburst result to translate
 *   Returns
 *      const char * = Text String explaining the error.
 *   Notes
 *      Straight ripoff of the (unavailable) sbGetFeIlibStatusString()
 */
const char *
sbx_result_string(sbStatus_t status);

#endif /* _SOC_SBX_ERROR_H_ */
