/*******************************************************************************
 *
 * Copyright 2011-2017 Broadcom Corporation
 *
 * This program is the proprietary software of Broadcom Corporation and/or its
 * licensors, and may only be used, duplicated, modified or distributed pursuant
 * to the terms and conditions of a separate, written license agreement executed
 * between you and Broadcom (an "Authorized License").  Except as set forth in an
 * Authorized License, Broadcom grants no license (express or implied), right to
 * use, or waiver of any kind with respect to the Software, and Broadcom expressly
 * reserves all rights in and to the Software and all intellectual property rights
 * therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS
 * SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE
 * ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization,
 * constitutes the valuable trade secrets of Broadcom, and you shall use all
 * reasonable efforts to protect the confidentiality thereof, and to use this
 * information only in connection with your use of Broadcom integrated circuit
 * products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 * "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
 * OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT
 * TO THE SOFTWARE. BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 * OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 * LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 * OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 * USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
 * OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT,
 * OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 * USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 * ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 * LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 * ANY LIMITED REMEDY.
 *
 *******************************************************************************/

#ifndef __ERRORS_H
#define __ERRORS_H


#ifdef __cplusplus
extern "C" {
#endif

/**
 *
 * @file errors.h
 *
 * Errors and fault reporting functions
 *
 */

/**
 * @addtogroup ERRORS_API
 * @{
 */

/**
 * @brief Status and error codes returned by all API functions.
 */

typedef enum kbp_status { /** \def KBP_INC_SEL(name,string)
                              Select between enumeration and error string */
#define KBP_INC_SEL(name,string) name,
#include "error_tbl.def"
#undef KBP_INC_SEL
    KBP_STATUS_LAST_UNUSED
} kbp_status;

/**
 * Function to convert a status code to a string. The returned
 * string should not be overwritten or freed by the caller.
 *
 * @param status Code to be converted.
 *
 * @retval string Verbose description of status code if valid.
 * @retval NULL if invalid status code is passed in.
 *
 * @warning The returned string should not be modified or freed.
 */

const char *kbp_get_status_string(kbp_status status);

/**
 * @brief Convenience macro to wrap function calls and capture error codes and their descriptions.
 *
 * A verbose error is printed when an error occurs.
 */

#define KBP_TRY(A)                                                      \
    do                                                                  \
    {                                                                   \
        kbp_status __tmp_status = A;                                    \
        if (__tmp_status != KBP_OK)                                     \
        {                                                               \
            kbp_printf(#A" failed: %s\n", kbp_get_status_string(__tmp_status)); \
            return __tmp_status;                                        \
        }                                                               \
    }                                                                   \
    while(0)

/**
 * @brief Convenience macro to wrap function calls and capture error codes.
 *
 * This API is silent; it does not output any verbose messages.
 */

#define KBP_STRY(A)                                                     \
    do                                                                  \
    {                                                                   \
        kbp_status __tmp_status = A;                                    \
        if (__tmp_status != KBP_OK)                                     \
        {                                                               \
            return __tmp_status;                                        \
        }                                                               \
    } while(0)

/**
 * @}
 */

#ifdef __cplusplus
}
#endif
#endif                          /* __ERRORS_H */
