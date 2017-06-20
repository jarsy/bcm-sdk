/*
 **************************************************************************************
 Copyright 2014-2017 Broadcom Corporation

 This program is the proprietary software of Broadcom Corporation and/or its licensors,
 and may only be used, duplicated, modified or distributed pursuant to the terms and
 conditions of a separate, written license agreement executed between you and
 Broadcom (an "Authorized License").Except as set forth in an Authorized License,
 Broadcom grants no license (express or implied),right to use, or waiver of any kind
 with respect to the Software, and Broadcom expressly reserves all rights in and to
 the Software and all intellectual property rights therein.
 IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY
 WAY,AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.

 Except as expressly set forth in the Authorized License,

 1. This program, including its structure, sequence and organization, constitutes the
    valuable trade secrets of Broadcom, and you shall use all reasonable efforts to
    protect the confidentiality thereof,and to use this information only in connection
    with your use of Broadcom integrated circuit products.

 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS" AND WITH
    ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR WARRANTIES, EITHER
    EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO THE SOFTWARE.  BROADCOM
    SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY,
    NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR
    COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION.
    YOU ASSUME THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.

 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS LICENSORS
    BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR EXEMPLARY DAMAGES
    WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR USE OF OR INABILITY TO USE
    THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES;
    OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF
    OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING
    ANY FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 **************************************************************************************
 */

#ifndef __KBP_MEM_TEST_H
#define __KBP_MEM_TEST_H

/* include header */
#include "kbp_portable.h"
#include "device.h"

#ifdef __cplusplus
extern "C" {
#endif

struct kbp_mem_test_info {
    uint32_t debug_prints; /**< enable debug prints. */
    void *handle;          /**< Opaque handle passed to MDIO reset routines as first argument. */

    uint64_t test_duration_in_usec;  /**< time duration in microseconds (1e-6) for each pass of a test pattern */
    uint32_t modified_addr_length;   /**< length of array to store addresses. additional addresses will not be 
                                         recorded if all array entries are used */
    uint32_t modified_addr_count;    /**< record number of address entries */
    uint32_t *modified_addr;         /**< pointer to an array to store failed addresses */

    uint32_t inject_error_addr_length; /**< length of array containing addresses to inject error */
    uint32_t *inject_error_addr;      /**< pointer to an array containing addresses to inject error */

    /* callback for the delay, implemented by the application */
    kbp_status (*kbp_mem_test_usleep)(void *handle, uint64_t);

    /* callback for write to KBP 80-bit MDIO registers, implemented by the application */
    kbp_status (*kbp_mdio_register_write_80)(void *handle, uint8_t kbp_select, uint8_t device_id, uint16_t addr, uint16_t* wr_data);

    /* callback for read KBP 80-bit MDIO registers, implemented by the application */
    kbp_status (*kbp_mdio_register_read_80)(void *handle, uint8_t kbp_select, uint8_t device_id, uint16_t addr, uint16_t* rd_data);

    /* callback for write to KBP 64-bit MDIO registers, implemented by the application */
    kbp_status (*kbp_mdio_register_write_64)(void *handle, uint8_t kbp_select, uint8_t device_id, uint16_t addr, uint16_t* wr_data);

    /* callback for read KBP 64-bit MDIO registers, implemented by the application */
    kbp_status (*kbp_mdio_register_read_64)(void *handle, uint8_t kbp_select, uint8_t device_id, uint16_t addr, uint16_t* rd_data);
};

/**
 * @brief test X, Y and valid bit for each database entry
 *
 * @param [in] device - valid KBP device handle
 * @param info Opaque user provided handle passed to the callback, with additional information.
 *
 * @return KBP_OK on success or an error code
 */
kbp_status kbp_mem_test_dba(struct kbp_device *device, struct kbp_mem_test_info *info);


/**
 * @brief test to test user data array
 *
 * @param [in] device - valid KBP device handle
 * @param info Opaque user provided handle passed to the callback, with additional information.
 *
 * @return KBP_OK on success or an error code
 */
kbp_status kbp_mem_test_uda(struct kbp_device *device, struct kbp_mem_test_info *info);

#ifdef __cplusplus
}
#endif

#endif /* __KBP_MEM_TEST_H */

