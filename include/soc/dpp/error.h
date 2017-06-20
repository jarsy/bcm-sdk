/* $Id:  $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/

#ifndef __DPP_ERROR_H__
#define __DPP_ERROR_H__

#include <soc/error.h>
#include <soc/dpp/SAND/Management/sand_error_code.h>

/*
 *   Function
 *      translate_sand_success_failure
 *   Purpose
 *      Translate a SOC_SAND_SUCCESS_FAILURE into a Broadcom result code
 *   Parameters
 *      (in)  result = SOC_SAND result to translate
 *   Returns
 *      soc_error_t = Closest reasonable Broadcom result
 *   Notes
 *      If there is no reasonably close error, it returns
 *      SOC_E_FAIL instead of something more specific.
 */
int
translate_sand_success_failure(const SOC_SAND_SUCCESS_FAILURE result);

/*
 *   Function
 *      translate_sand_result
 *   Purpose
 *      Translate soc_sand eror code into a Broadcom result code, and print
 *      (under debug flags) soc_sand erorr information.
 *   Parameters
 *      (in)  result = SOC_SAND error code to translate (as returned from soc_sand function
 *   Returns
 *       SOC_E_NONE soc_sand function returned no error
 *       SOC_E_INTERNAL soc_sand function returned an error
 *   Notes
 *      None.
 */
int
handle_sand_result(uint32 soc_sand_result);

#define SOC_SAND_IF_ERR_RETURN(_sand_ret) \
    do { \
        soc_error_t _rv = handle_sand_result(_sand_ret); \
        if (_rv != SOC_E_NONE) { \
            return _rv; \
        } \
    } while (0)

#define SOC_SAND_IF_ERR_EXIT(_sand_ret) \
    do { \
        soc_error_t _rv = handle_sand_result(_sand_ret); \
        if (_rv != SOC_E_NONE) { \
            goto exit; \
        } \
    } while (0)


#define SOC_SAND_IF_FAIL_RETURN(_sand_success) \
    do { \
        _rv = translate_sand_success_failure(_sand_success); \
        if (_rv != SOC_E_NONE) { \
            return _rv; \
        } \
    } while (0)


#define SOC_SAND_IF_FAIL_EXIT(_sand_success) \
    do { \
        _rv = translate_sand_success_failure(_sand_success); \
        if (_rv != SOC_E_NONE) { \
            goto exit; \
        } \
    } while (0)


#define SOC_SAND_IF_ERR_OR_FAIL_RETURN(_sand_ret, _sand_success) \
    do { \
        SOC_SAND_IF_ERR_RETURN(_sand_ret); \
        SOC_SAND_IF_FAIL_RETURN(_sand_success); \
    } while (0)

#define SOC_SAND_FAILURE(_sand_ret) \
    ((handle_sand_result(_sand_ret)) < 0)
        
#define SOC_SAND_IF_ERROR_RETURN(_sand_ret)                                                \
    do {                                                                               \
        soc_error_t _rv = handle_sand_result(_sand_ret);                               \
        if (_rv != SOC_E_NONE) {                                                       \
	        _bsl_warn(_BSL_SOCDNX_MSG("%d"),_rv);                                       \
            return _rv;                                                                \
        }                                                                              \
    } while (0)

#endif /* __DPP_ERROR_H__ */
