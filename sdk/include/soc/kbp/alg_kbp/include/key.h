/*
 **************************************************************************************
 Copyright 2012-2017 Broadcom Corporation

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

#ifndef __KEY_H
#define __KEY_H

/**
 * @file key.h
 *
 * This module helps define the key layout for databases and instructions.
 * This forms the basis for extraction of subkeys from master keys specified
 * as part of instructions. This allows the control-plane API
 * to program the key-processing units (KPUS) in hardware.
 */

#include <stdint.h>

#include "errors.h"
#include "device.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @addtogroup KEY_API
 * @{
 */

/**
 * Opaque key handle
 */

struct kbp_key;

/**
 * The type of the key field. This helps
 * the control plane identify the type of the
 * field. This information is mainly used for
 * programming the KPUs and effective range
 * handling in hardware.
 */

enum kbp_key_field_type {
    KBP_KEY_FIELD_TERNARY,/**< Arbitrary data/mask field with don't cares. */
    KBP_KEY_FIELD_PREFIX, /**< An IPv4 or IPv6 address prefix. */
    KBP_KEY_FIELD_EM,     /**< Exact-match field that does not have don't cares (for example, Table ID). */
    KBP_KEY_FIELD_RANGE,  /**< 16-bit range field that can be compared as >= or <= (for example, ports). */
    KBP_KEY_FIELD_TABLE_ID, /**< Equivalent to exact match, identifies the table identifier
                               tuple when the database is a collection of multiple tables. */
    KBP_KEY_FIELD_HOLE,   /**< Arbitrary number of bytes in the key, which are zero-filled.
                             It is assumed the entry in DBA will not compare this portion of the key.
                             This can be used to express BMR functionality */
    /* This should be the last entry */
    KBP_KEY_FIELD_DUMMY_FILL, /**< Ignores the field in the key specified for instructions only */
    KBP_KEY_FIELD_INVALID
};

/**
 * Creates a new key, which will be used to define the layout of entries.
 *
 * @param device Valid KBP device handle.
 * @param key The initialized key, returned on success.
 *
 * @retval KBP_OUT_OF_MEMORY when the key cannot be allocated.
 * @retval KBP_INVALID_ARGUMENT if the arguments are NULL or invalid.
 * @retval KBP_OK on success.
 */

kbp_status kbp_key_init(struct kbp_device *device, struct kbp_key **key);

/**
 * Adds a field to the key. Repeated calls to add field
 * helps to construct the key. The field names provided are
 * not interpreted in any way. However,  to program the KPU correctly, the field names
 * in the databases must match the field names in the instruction
 * key. The field width must be a multiple of eight bits.
 *
 * @param key Valid key handle.
 * @param name Field name.
 * @param width_1 Width of the field in bits. Must be multiple of eight bits.
 * @param type The type of the field defined by ::kbp_key_field_type.
 *
 * @retval KBP_OUT_OF_MEMORY when the key field cannot be allocated.
 * @retval KBP_INVALID_ARGUMENT if the arguments are NULL or invalid.
 * @retval KBP_OK on success.
 */

kbp_status kbp_key_add_field(struct kbp_key *key, char *name, uint32_t width_1, enum kbp_key_field_type type);

/**
 * Verifies that the database_key can be derived from the specified master key.
 *
 * @param master_key Valid key handle to the master key.
 * @param db_key Valid key handle to the database key that specifies the layout records.
 * @param error_field Pointer to the name of the field (if any) that causes an error. The memory
 *                    returned must not be freed. Can be NULL if user is not interested in the error field.
 *
 * @retval KBP_OK if the database key can be derived from the master key.
 * @retval KBP_KEY_FIELD_MISSING if a field specified in the database key is missing from the master key.
 * @retval KBP_KEY_GRAN_ERROR if the database key can be derived from the master key only from byte boundaries.
 * @retval KBP_INVALID_ARGUMENT if the arguments are NULL or invalid.
 */

kbp_status kbp_key_verify(struct kbp_key *master_key, struct kbp_key *db_key, char **error_field);

/**
 * Pretty-print the key information.
 *
 * @param key Valid key handle.
 * @param fp Pointer to the file where the contents are to be dumped.
 *
 * @retval KBP_INVALID_ARGUMENT if the arguments are NULL or invalid.
 * @retval KBP_OK on success.
 */

kbp_status kbp_key_print(struct kbp_key *key, FILE * fp);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif
#endif /*__KEY_H */
