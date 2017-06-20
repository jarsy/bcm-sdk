/*
 **************************************************************************************
 Copyright 2015-2017 Broadcom Corporation

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

#ifndef __KBP_PCIE_H
#define __KBP_PCIE_H

#include <stdint.h>

#include "errors.h"
#include "init.h"
#include "allocator.h"

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @brief Opaque user handle to the search result returned by the instruction search.
 */

struct kbp_search_result;

/**
 * Properties that can be set with kbp_pcie_set_property()
 */
enum kbp_pcie_properties {
    KBP_PCIE_NON_DMA,            /**< Enable NON DMA mode */
    KBP_PCIE_VERBOSE_LEVEL,      /**< Set Vebose Level */
    KBP_PCIE_NEED_RESPONSE,      /**< Wait for response */
    KBP_PCIE_INTERRUPT,          /**< If called pass the signal number to raise interrupt against */
    KBP_PCIE_REPLACE_SEARCH,     /**< To replace the PCIe search path */
    KBP_PCIE_PROP_INVALID        /**< Invalid. This should be the last entry */
};

/**
 * PCIE advanced Settings
 */

struct kbp_pcie_settings
{
    uint32_t req_q_size;   /**< DMA request buffer size 0x1 - 0xd */
    uint32_t resp_q_size;  /**< DMA response buffer size 0x1 - 0xd */
};

/**
 * Properties to tailor the transport layer. Supported properties
 * are defined in ::kbp_pcie_properties
 *
 * |PROPERTY|VALUES|DEFAULT|
 * |:---:|:---:|:---:|
 * |KBP_PCIE_NON_DMA| Use non-DMA mode of comunication | False. Default is DMA enabled |
 * |KBP_PCIE_VERBOSE_LEVEL| Enable verbose printing to stdout | Default zero |
 * |KBP_PCIE_NEED_RESPONSE| Request response for every update | Default only reads/compares get responses |
 * |KBP_PCIE_INTERRUPT|Provide signal number to interrupt against. Enables MSI based interrupts|Disabled by default|
 * |KBP_PCIE_REPLACE_SEARCH| To replace the PCIe search path
 *
 * @param xpt pointer to the transport layer initialized by the function on success
 * @param ... variable arguments related to the property
 *
 * @returns KBP_OK on success or an error code
 */

kbp_status kbp_pcie_set_property(void *xpt, enum kbp_pcie_properties property, ...);

/**
 * Initialize the PCIe Driver and return transport layer handle
 *
 * @param type The device type. Currently only suported device is KBP_DEVICE_OP
 * @param flags The device configuration flags
 * @param device_no The device number on the board to connect to
 * @param alloc valid allocator handle
 * @param config Optional parameter if passed in the MDIO read/write callback function are populated
 * @param driver_settings Optional parameters required to configure the PCIe
 * @param xpt pointer to the transport layer initialized by the function on success
 *
 * @return KBP_OK on success or an error code
 */

kbp_status kbp_pcie_init(enum kbp_device_type type, uint32_t flags, uint32_t device_no, struct kbp_allocator *alloc,
                         struct kbp_device_config *config, struct kbp_pcie_settings *driver_settings,
                         void **xpt);

/**
 * Destroy the resources allocated for the pcie Driver
 *
 * @param alloc valid allocator handle
 * @param xpt valid initialized transport layer handle returned by kbp_op_pcie_init()
 * @return KBP_OK on success or an error code
 */

kbp_status kbp_pcie_destroy(struct kbp_allocator *alloc, void *xpt);

/**
 * PCIe soft reset reset
 *
 * @param xpt pointer to the op_xpt struct that contains the
 *            handle pointer
 * @return KBP_OK on success or an error code
 */

kbp_status kbp_pcie_soft_reset(void *xpt);

/**
* Property to replace the PCIe search path.
*
* @param handle pointer to the search handle
* @param ltr ltr number
* @param ctx context id
* @param key pointer to the search key
* @param key_len search key length
* @param result pointer to the search result structure
* @return KBP_OK on success or an error code
*/

typedef kbp_status (*kbp_op_search)(void *handle, uint32_t ltr, uint32_t ctx,
                                    const uint8_t *key, uint32_t key_len,
                                    struct kbp_search_result *result);

#ifdef __cplusplus
}
#endif
#endif /*__KBP_PCIE_H */

