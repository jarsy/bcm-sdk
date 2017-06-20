/*
 **************************************************************************************
 Copyright 2014-2016 Broadcom Corporation

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

#ifndef __MODEL_H
#define __MODEL_H

#include <stdint.h>

#include "device.h"
#include "errors.h"
#include "init.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Statistics obtained from the C-Model
 */
struct kbp_sw_model_stats
{
    /**< Maximum number of DBA blocks lit by any key during a compare, per LTR */
    uint32_t max_num_dba_blocks_lit_per_key[KBP_HW_MAX_LTRS][KBP_HW_MAX_KPUS];
};

/**
 * Model properties that can be set by user
 */
enum kbp_sw_model_property
{
    KBP_SW_MODEL_DUMP,    /**< Dump PIO writes to file, a valid file pointer must be provided */
    KBP_SW_MODEL_UNROLL_COMPLEX_INST, /**< Logs the unrolled complex instruction by issuing
                                                                broken up instructions*/
    KBP_SW_MODEL_TRACE,   /**< Trace print the search results */
    KBP_SW_MODEL_TRIG_EVENTS,  /** Trigger callbacks for HW write events */
    KBP_SW_MODEL_INVALID  /**< This should be the last entry */
};


/**
 * @brief Configuration details for the KAPS model
 */
struct kbp_sw_model_kaps_config
{
    uint8_t id; /**< Device id */
    uint8_t sub_type;   /**< Device sub type */
    uint16_t total_num_rpb; /**< Total number of RPBs in the KAPS device*/
    uint16_t total_num_bb;  /**< Total number of BBs in the KAPS device */
    uint16_t num_rows_in_rpb; /**< Number of rows in one RPB */
    uint16_t num_rows_in_bb; /**< Number of rows in one BB */
};

/**
 * @brief configuration for 12K model
 */
struct kbp_sw_model_12k_config
{
    uint32_t num_devices;     /**< Number of cascaded devices */
    uint32_t dba_mb;          /**< 40 or 80Mb device */
    uint32_t algorithmic_lpm; /**< Algorithmic LPM support on device */
    uint32_t algorithmic_acl; /**< Algorithmic ACL support on device */
};

/**
 * @brief Configuration details for the OP model
 */
struct kbp_sw_model_op_config
{
    uint8_t dev_type;  /**< Non-zero values between <0x1-0x8> for internal Model testing */
};



/**
 * @brief Configuration details for the supported devices
 */
struct kbp_sw_model_config
{
    union {
        struct kbp_sw_model_kaps_config config_kaps;   /**< Configuration details for the KAPS model */
        struct kbp_sw_model_12k_config config_12k;      /**< Configuration details for 12K model */
        struct kbp_sw_model_op_config  config_op;       /**< Configuration details for OP model*/
    } u;
};

/**
 * Initialize the software model and return a transport layer
 *
 * @param alloc valid allocator handle
 * @param type the device type to initialize the model for ::kbp_device_type
 * @param flags ::kbp_device_flags OR'd together.
 * @param config has the configuration input details for creating the Model. If NULL, the default config is chosen
 * @param xpt pointer to the transport layer initialized by the function on success
 *
 * @return KBP_OK on success or an error code
 */

kbp_status kbp_sw_model_init(struct kbp_allocator *alloc, enum kbp_device_type type,
                             uint32_t flags, struct kbp_sw_model_config *config, void **xpt);

/**
 * Destroy the resources allocated by the software model
 *
 * @param xpt valid initialized transport layer handle returned by kbp_sw_model_init()
 *
 * @return KBP_OK on success or an error code
 */

kbp_status kbp_sw_model_destroy(void *xpt);

/**
 * Obtain the Model Statistics
 *
 * @param xpt valid initialized transport layer handle returned by kbp_sw_model_init()
 * @param stats the model statistics will be  returned here
 *
 * @return KBP_OK on success or an error code
 */

kbp_status kbp_sw_model_get_stats(void *xpt, struct kbp_sw_model_stats *stats);

/**
 * Set property attributes on the model
 *
 * @param xpt the model handle
 * @param property the model property to set in ::kbp_sw_model_property
 * @param ... the property value
 *
 * @return KBP_OK on success or an error code
 */

kbp_status kbp_sw_model_set_property(void *xpt, enum kbp_sw_model_property property, ...);

#ifdef __cplusplus
}
#endif
#endif /*__MODEL_H */
