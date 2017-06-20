/* $Id:  $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/

#ifndef __DNX_ERROR_H__
#define __DNX_ERROR_H__

#include <soc/error.h>
#include <soc/dnx/legacy/SAND/Management/sand_error_code.h>

/*
 *   Function
 *      dnx_translate_sand_success_failure
 *   Purpose
 *      Translate a DNX_SAND_SUCCESS_FAILURE into a Broadcom result code
 *   Parameters
 *      (in)  result = DNX_SAND result to translate
 *   Returns
 *      soc_error_t = Closest reasonable Broadcom result
 *   Notes
 *      If there is no reasonably close error, it returns
 *      SOC_E_FAIL instead of something more specific.
 */
int
dnx_translate_sand_success_failure(const DNX_SAND_SUCCESS_FAILURE result);

/*
 *   Function
 *      translate_sand_result
 *   Purpose
 *      Translate dnx_sand eror code into a Broadcom result code, and print
 *      (under debug flags) dnx_sand erorr information.
 *   Parameters
 *      (in)  result = DNX_SAND error code to translate (as returned from dnx_sand function
 *   Returns
 *       SOC_E_NONE dnx_sand function returned no error
 *       SOC_E_INTERNAL dnx_sand function returned an error
 *   Notes
 *      None.
 */
int
dnx_handle_sand_result(uint32 dnx_sand_result);

#define DNX_SAND_IF_ERR_RETURN(_sand_ret) \
    do { \
        soc_error_t _rv = dnx_handle_sand_result(_sand_ret); \
        if (_rv != SOC_E_NONE) { \
            return _rv; \
        } \
    } while (0)

#define DNX_SAND_IF_ERR_EXIT(_sand_ret) \
    do { \
        soc_error_t _rv = dnx_handle_sand_result(_sand_ret); \
        if (_rv != SOC_E_NONE) { \
            goto exit; \
        } \
    } while (0)


#define DNX_SAND_IF_FAIL_RETURN(_sand_success) \
    do { \
        _rv = dnx_translate_sand_success_failure(_sand_success); \
        if (_rv != SOC_E_NONE) { \
            return _rv; \
        } \
    } while (0)


#define DNX_SAND_IF_FAIL_EXIT(_sand_success) \
    do { \
        _rv = dnx_translate_sand_success_failure(_sand_success); \
        if (_rv != SOC_E_NONE) { \
            goto exit; \
        } \
    } while (0)


#define DNX_SAND_IF_ERR_OR_FAIL_RETURN(_sand_ret, _sand_success) \
    do { \
        DNX_SAND_IF_ERR_RETURN(_sand_ret); \
        DNX_SAND_IF_FAIL_RETURN(_sand_success); \
    } while (0)

#define DNX_SAND_FAILURE(_sand_ret) \
    ((dnx_handle_sand_result(_sand_ret)) < 0)
        
#define DNX_SAND_IF_ERROR_RETURN(_sand_ret)                                                \
    do {                                                                               \
        soc_error_t _rv = dnx_handle_sand_result(_sand_ret);                               \
        if (_rv != SOC_E_NONE) {                                                       \
	        _bsl_warn(_BSL_DNX_MSG("%d"),_rv);                                       \
            return _rv;                                                                \
        }                                                                              \
    } while (0)

#endif /* __DNX_ERROR_H__ */
