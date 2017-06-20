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

#ifndef __DEVICE_H
#define __DEVICE_H

/**
 * @file device.h
 *
 * This module provides the hardware-abstraction layer (HAL)
 * for the user.
 */

#include <stdint.h>
#include <stdio.h>

#include "errors.h"
#include "init.h"
#include "hw_limits.h"

#ifdef __cplusplus
extern "C" {
#endif



/**
 * @addtogroup DEVICE_API
 * @{
 */

/**
 * Opaque KBP device handle.
 */

struct kbp_device;

/**
 * Opaque allocator handle.
 */

struct kbp_allocator;

/**
 * Opaque user handle to the database stored in the KBP.
 */

struct kbp_db;

/**
 * Opaque handle to an entry within a database.
 */

struct kbp_entry;

/**
 * KBP SDK version string.
 */

#define KBP_SDK_VERSION "KBP SDK 1.4.10"


/**
 * Type of databases supported by software.
 */

enum kbp_db_type {
    KBP_DB_INVALID,/**< Error database type. */
    KBP_DB_ACL,    /**< Algorithmic and nonalgorithmic ACL databases. */
    KBP_DB_LPM,    /**< Longest-prefix-match databases. */
    KBP_DB_EM,     /**< Exact-match databases. */
    KBP_DB_AD,     /**< Associated data databases. */
    KBP_DB_DMA,    /**< Direct Memory Access databases. */
    KBP_DB_HB,     /**< Hit Bits databases. */
    KBP_DB_END     /**< This should be the last entry. */
};

/**
 * @brief Hardware resources that can be partitioned
 *
 * The available hardware resources that can be partitioned between databases. The following functions
 * can be invoked with the hardware resources to limit the databases to use
 * the specified resources only:
 *
 * @li kbp_db_set_resource()
 * @li kbp_ad_db_set_resource()
 *
 * can be invoked with the HW resources to limit the databases to use
 * the specified resources only. Optionally, the resource compiler can
 * be used to automatically assign hardware resources to the various databases
 * and functions. For LPM only databases, the database handle must be used to
 * set both UDA and DBA. For all other databases, the
 * database handle is used to set DBA, and the AD Database handle
 * is used to set UDA
 *
 * | RESOURCE | NUMBER OF ARGS | ARGUMENTS |
 * |:---: | :---: | :---: |
 * | KBP_HW_RESOURCE_DBA | 1 | uint32_t value specifying the number of ABs to use (for LPM, EM, ACL databases). |
 * | KBP_HW_RESOURCE_UDA | 1 | uint32_t value specifying the UDA in megabits (for algorithmic LPM, EM, ACL databases and for AD databases). |
 * | KBP_HW_RESOURCE_RANGE_UNITS | 1 | uint32_t value specifying the number of range units to use for MCOR/DIRPE (ACL databases only). |
 */

enum kbp_hw_resource {
    KBP_HW_RESOURCE_DBA,  /**< The number of ABs to use. */
    KBP_HW_RESOURCE_UDA,  /**< The UDA in megabits. */
    KBP_HW_RESOURCE_RANGE_UNITS, /**< The number of range units to use. */
    KBP_HW_RESOURCE_INVALID      /**< This should be the last entry. */
};


/**
 * @brief Specifes the range encoding types supported
 */
enum kbp_range_encoding_type {
    KBP_RANGE_3B_ENCODING,  /**< 2 bit to 3 bit encoding usually configured to DIRPE */
    KBP_RANGE_2B_ENCODING,  /**< 2 bit to 2 bit encoding */
    KBP_RANGE_NO_ENCODING   /**< No encoding */
};

/**
 * Initialize the KBP device. A valid transport layer is required.
 * On initialization, the device is dynamically queried to ascertain
 * its capabilities and the resources available.
 *
 * @param alloc Valid non-NULL allocator handle for portability.
 * @param type The device type ::kbp_device_type.
 * @param flags Flags OR'd together that allow the device to be set into specific modes. See ::kbp_device_flags.
 * @param xpt NULL or valid transport layer handle. Transport-layer functions are used to
 *              establish communication with the device.
 * @param config SerDes and lane-configuration information. Can be NULL to indicate the initialization
 *              has already occurred.
 * @param device Device handle initialized and returned on success. This handle can be used to
 *        address the global device.
 *
 * @return KBP_OK on success or an error code otherwise.
 */

kbp_status kbp_device_init(struct kbp_allocator *alloc, enum kbp_device_type type, uint32_t flags,
                           void *xpt, struct kbp_device_config *config, struct kbp_device **device);

/**
 * Destroys the device handle and frees up resources. The physical
 * device is left undisturbed.
 *
 * @param device The device handle.
 *
 * @return KBP_OK on success or an error code otherwise.
 */

kbp_status kbp_device_destroy(struct kbp_device *device);

/**
 * Locks the device. Once the device is locked, new databases/instructions
 * cannot be created.
 *
 * @param device Initialized pointer to the device.
 *
 * @return KBP_OK on success or an error code otherwise.
 */

kbp_status kbp_device_lock(struct kbp_device *device);

/**
 * Crash Recovery Status
 */
enum kbp_restore_status {
    KBP_RESTORE_NO_CHANGE,            /* The SDK was restored with no change in config, no pending actions */
    KBP_RESTORE_CHANGES_COMMITTED,    /* The SDK was restored and the pending actions were committed */
    KBP_RESTORE_CHANGES_ABORTED       /* The SDK was restored and the pending actions were aborted */
};


/**
 * IFSR parity error location status
 */
enum parity_row_status {
    KBP_ROW_INVALID,
    KBP_ROW_UNTOUCHED, /* this row un-touched neither re-written nor repaired */
    KBP_ROW_RE_WRITE,  /* this row touched and re-written by the internal db's, but not yet repaired */
    KBP_ROW_REPAIRED,  /* this row touched and repaired by the s/w, and not re-written by the internal db's */
    KBP_ROW_REPAIRED_RE_WRITE/* this row touched and repaired by the s/w, and re-written this location belongs to internal db's */

};


/**
 * Properties that can be set on the device.
 */
enum kbp_device_properties {
    KBP_DEVICE_PROP_DESCRIPTION,       /**< A string description for the device. */
    KBP_DEVICE_PROP_TOTAL_NUM_BB,      /**< Total number of Bucket Blocks. */
    KBP_DEVICE_PROP_RESOURCE_FN,       /**< Callback for resource processing. */
    KBP_DEVICE_PROP_LTR_SM_UPDATE,     /**< LTR Shadow Memory Update. */
    KBP_DEVICE_PROP_INTERRUPT,         /**< Enable for supported device interrupts */
    KBP_DEVICE_PROP_IFSR_THRESHOLD,    /**< Threshold for initiating a soft repair of DBA */
    KBP_DEVICE_PROP_LAST_PARITY_ERRORS,/**< Last seen dba parity error address of specific device [updated by kbp_device_fix_errors] */
    KBP_DEVICE_PROP_LAST_PARITY_ERRORS_STATUS,/**< Last seen dba parity error address's status of specific device
                                            [updated by kbp_device_fix_errors] for the status code refer the
                                            enum parity_row_status above */
    KBP_DEVICE_PROP_INJECT_PARITY_ERRORS,/**< Inject the parity errros by using debug regs, few bits in both X,Y row are flipped
                                              during the PIO writes on the DBA */
    KBP_DEVICE_PROP_NON_REPAIRABLE_ABS,  /**< Once all the redundant rows are used up AB cannot repaired
                                              further, this prop will list those ABs on query (DBA) */
    KBP_DEVICE_PROP_READ_TYPE,         /**< read type, 0: read from SM, 1: read from device/model and 2: read from device and compare against SM data
                                            Note: in blackhole mode read type cannot be other than 0, will return error */
    KBP_DEVICE_PROP_DEBUG_LEVEL,       /**< debug level, default 0, non zero value represent the level of debug prints
                                            Note: must be set before kbp_device_lock() and debug print APIs must called after
                                                  the kbp_device_lock() as instruction,entries might in inconsistent state */
    KBP_DEVICE_PROP_INST_LATENCY,      /**< Value between zero/one, two and eight, where the zero(==1) means issue instructions
                                            that take minimum amount of data bandwidth, and eight means maximum
                                            update rate, as longer latency control instructions can be issues. */
    KBP_DEVICE_PRE_CLEAR_ABS,          /**< Clear the ABs at the time of device init */
    KBP_DEVICE_PROP_LAST_UDA_PARITY_ERRORS,/**< Last seen uda parity error address of specific device [updated by kbp_device_fix_errors] */
    KBP_DEVICE_ADD_BROADCAST,          /**< Configures a device as broadcast device */
    KBP_DEVICE_ADD_SELECTIVE_BROADCAST,/**< Configures a device as a selective broadcast device */
    KBP_DEVICE_BROADCAST_AT_XPT,       /**< if this property is set then broadcast happens at XPT level,
                                            else it happens at hardware device driver level */
    KBP_DEVICE_PROP_CRASH_RECOVERY,    /**< Register the NV Read/Write callbacks required to support Crash Recovery */
    KBP_DEVICE_PROP_DEFER_LTR_WRITES,  /**< Defer LTR writes. use it for ISSU */
    KBP_DEVICE_PROP_CLR_DEV_ERR_BITS,  /**< Clears only device error/parity bits, else all bits in ESR : default all*/
    KBP_DEVICE_PROP_DUMP_ON_ASSERT,    /**< Dump software state to a file before asserting */
    KBP_DEVICE_PROP_INVALID            /**< This should be the last entry. */
};

/**
 * Sets a specific property for the device..
 *
 * @param device Valid device handle
 * @param property Device properties ::kbp_device_properties.
 * @param ... Variable arguments based on property.
 *
 * @return KBP_OK on success or an error code otherwise.
 */

kbp_status kbp_device_set_property(struct kbp_device *device, enum kbp_device_properties property, ...);

/**
 * Gets a specific property of the device.
 *
 * @param device Valid device handle.
 * @param property Device properties being queried ::kbp_device_properties.
 * @param ... Variable arguments based on property.
 *
 * @return KBP_OK on success or an error code otherwise.
 */

kbp_status kbp_device_get_property(struct kbp_device *device, enum kbp_device_properties property, ...);

/**
 * Obtains the SMT thread device handles. The device must have been initialized in SMT
 * mode. Databases and instructions must be created using the appropriate thread's device
 * handle.
 *
 * @param device Valid device handle returned by kbp_device_init().
 * @param tid The SMT thread ID. Currently supported ID's are zero and one.
 * @param thr_device The thread's device handle returned on success.
 *
 * @return KBP_OK on success or an error code otherwise.
 */

kbp_status kbp_device_thread_init(struct kbp_device *device, uint32_t tid, struct kbp_device **thr_device);

/**
 * Prints the device information. A visual HTML format with SVG
 * is dumped that works well on Firefox and Chrome browsers only.
 *
 * This function should be called only after the device is locked.
 * Calling on an unlocked device will result in an error being
 * returned and no HTML being generated.
 *
 * @param device The KBP device handle.
 * @param fp The file pointer to dump to.
 *
 * @retval KBP_OK on success.
 * @retval KBP_DEVICE_UNLOCKED when called on a device which has not been locked.
 */

kbp_status kbp_device_print_html(struct kbp_device *device, FILE *fp);

/**
 * Prints device information in text to the specified file.
 *
 * @param device The KBP device handle.
 * @param fp The file pointer to dump to.
 *
 * @retval KBP_OK on success or an error code otherwise.
 */

kbp_status kbp_device_print(struct kbp_device *device, FILE *fp);

/**
 * Return's the KBP SDK Version String which should not be modified and free'd
 *
 *
 * @retval KBP SDK Version String
 */

const char * kbp_device_get_sdk_version(void);


/**
 * Repair errors on device. The API will read the error status register
 * on all the devices (if cascade is enabled) and corrrect errors
 * on them if any are present. Currently only DBA parity errors are
 * corrected. On successful return errors are fixed if present, or an
 * error code is returned.
 *
 * @param device The KBP device handle.
 *
 * @retval KBP_OK on success an error code otherwise.
 */

kbp_status kbp_device_fix_errors(struct kbp_device *device);


/**
 * @}
 */

/**
 * @addtogroup ISSU_API
 * @{
 */

/**
 * Callback function to read saved state from nonvolatile memory.
 * The user implements this function as specified in the prototype.
 *
 * @param handle Handle passed to API kbp_device_save_state()/ kbp_device_restore_state()
 * @param buffer Valid buffer memory in which to read the data. Only the specified size must be copied, or a buffer overrun can result.
 * @param size The number of bytes to read.
 * @param offset The offset in nonvolatile memory to read from.
 *
 * @retval 0 on success and contents of buffer are valid.
 * @retval Nonzero on failure reading data; buffer contains garbage.
 */
typedef int32_t (*kbp_device_issu_read_fn)(void *handle, uint8_t *buffer, uint32_t size, uint32_t offset);

/**
 * Callback function to write state into nonvolatile memory.
 * The user implements this function as specified in the prototype.
 *
 * @param handle Handle passed to API kbp_device_save_state()/ kbp_device_restore_state()
 * @param buffer Valid buffer memory to take the data from and write into nonvolatile memory.
 * @param size The number of bytes to write.
 * @param offset The offset in the nonvolatile memory to write to.
 *
 * @retval 0 on success and contents of buffer were written out.
 * @retval Nonzero on failure writing data. This result is fatal.
 */
typedef int32_t (*kbp_device_issu_write_fn)(void *handle, uint8_t *buffer, uint32_t size, uint32_t offset);

/**
 * Perform a warm boot/ISSU. Save device state to nonvolatile memory. The
 * user is expected to provide implementations of read/write callbacks.
 * These callbacks are used to write out state into the nonvolatile
 * memory. Once the function is invoked, no further control-plane
 * operations are permitted. API kbp_device_destroy() can be called afterwards
 * to recover control-plane heap resources.
 *
 * @param device Valid device handle.
 * @param read_fn Callback to read data from nonvolatile memory.
 * @param write_fn Callback to write data to nonvolatile memory.
 * @param handle User handle which will be passed back through read_fn and write_fn.
 *
 * @return KBP_OK on success, or an error code otherwise.
 */

kbp_status kbp_device_save_state(struct kbp_device *device, kbp_device_issu_read_fn read_fn, kbp_device_issu_write_fn write_fn, void *handle);

/**
 * Perform a warm boot/ISSU. Save device state to nonvolatile memory. The
 * user is expected to provide implementations of read/write callbacks.
 * These callbacks are used to write out state into the nonvolatile
 * memory. Control Plane operations are permitted after invoking this API
 *
 * @param device Valid device handle.
 * @param read_fn Callback to read data from nonvolatile memory.
 * @param write_fn Callback to write data to nonvolatile memory.
 * @param handle User handle which will be passed back through read_fn and write_fn.
 *
 * @return KBP_OK on success, or an error code otherwise.
 */

kbp_status kbp_device_save_state_and_continue(struct kbp_device *device, kbp_device_issu_read_fn read_fn, kbp_device_issu_write_fn write_fn, void *handle);


/**
 * Perform a warm boot/ISSU restore. Restore device state from nonvolatile memory. The
 * user is expected to provide implementations of read/write callbacks.
 * These callbacks are used to read state from the nonvolatile
 * memory. The user is expected to know where the data was stored during
 * the save operation; the offsets specified in the callback are relative to this.
 *
 * @param device Valid device handle.
 * @param read_fn Callback to read data from nonvolatile memory.
 * @param write_fn Callback to write data to nonvolatile memory.
 * @param handle User handle which will be passed back through read_fn and write_fn
 *
 * @return KBP_OK on success, or an error code otherwise.
 */

kbp_status kbp_device_restore_state(struct kbp_device *device, kbp_device_issu_read_fn read_fn, kbp_device_issu_write_fn write_fn, void *handle);

/**
 * Signal start of reconciliation phase for ISSU. This is an
 * optional phase that signals marking all entries alive
 * in the user's software as visited. The window is demarked
 * by kbp_device_reconcile_start() and kbp_device_reconcile_end().
 * Also see kbp_entry_set_used().
 *
 * @param device Valid device handle.
 *
 * @return KBP_OK on success or an error code otherwise.
 */
kbp_status kbp_device_reconcile_start(struct kbp_device *device);

/**
 * Signal end of reconciliation phase for ISSU. This is an
 * optional phase that signals marking all entries alive
 * in the users software as visited. The window is demarked
 * by kbp_device_reconcile_start() and kbp_device_reconcile_end().
 * Also see kbp_entry_set_used().
 *
 * @param device Valid device handle.
 *
 * @return KBP_OK on success or an error code otherwise.
 */
kbp_status kbp_device_reconcile_end(struct kbp_device *device);

/**
 * @}
 */

/**
 * This is debug API
 * Given hardware index, will do look up and give the DB and Entry handle for ACLs
 * Only DB handle is returned back for LPM databases.
 *
 * @param device The KBP device handle.
 * @param hw_index The device index for with the handles are returned, for cascade devices, hw_index must include device_id.
 * @param db The database handle returned if hw_index lookup found, else NULL.
 * @param entry The entry handle returned if hw_index lookup and db found, else NULL.
 *   user can still use the db handle if it is not NULL
 *
 * @retval KBP_OK on success an error code otherwise.
 */

kbp_status kbp_device_lookup_index(struct kbp_device *device, uint32_t hw_index, struct kbp_db **db, struct kbp_entry **entry);

/**
 * Dumps the current state of the s/w (Key, Database properties, Entries, Instructions, Resources) in the XML Format.
 *
 */
kbp_status kbp_device_dump(FILE * fp, struct kbp_device * device);

/**
 * @addtogroup CRASH_RECOVERY_API
 * @{
 */

/**
 * This API call marks the start of the transaction in the code flow when Crash Recovery is enabled.
 *
 * @param device Valid device handle.
 *
 * @return KBP_OK on success or an error code otherwise.
 */

kbp_status kbp_device_start_transaction(struct kbp_device *device);

/**
 * This API call marks the end of the transaction in the code flow when Crash Recovery is enabled.
 *
 * @param device Valid device handle.
 *
 * @return KBP_OK on success or an error code otherwise.
 */

kbp_status kbp_device_end_transaction(struct kbp_device *device);

/**
 * This API returns the action taken on the pending entries during crash recovery.
 *
 * @param device Valid device handle.
 * @param status Status of pending entries
 *
 * @return KBP_OK on success or an error code otherwise.
 */
kbp_status kbp_device_query_restore_status(struct kbp_device *device, enum kbp_restore_status *status);

/**
 * This API clears the status of pending entries being maintained.
 *
 * @param device Valid device handle.
 *
 * @return KBP_OK on success or an error code otherwise.
 */
kbp_status kbp_device_clear_restore_status(struct kbp_device *device);

/**
 * @}
 */

/**
 * This API will flush the LTR writes to device
 *
 * @param device Valid device handle.
 *
 * @return KBP_OK on success or an error code otherwise.
 */
kbp_status kbp_device_flush_ltr_writes(struct kbp_device *device);


/**
 * This API will return the last error of the device
 *
 * @param device Valid device handle.
 *
 * @return verbose_error_string if any error or empty string if no error. DO NOT FREE THE RETURN STRING
 */

const char *kbp_device_get_last_error(struct kbp_device *device);

/**
 * Dumps the device core registers in address, data and name format
 *
 * @param device Valid device handle.
 * @param fp file to log the dump
 *
 * @return KBP_OK on success or an error code otherwise.
 */
kbp_status kbp_device_dump_core_regs(struct kbp_device *device, FILE *fp);

/**
 * Dumps the serdes registers in address, data and name format
 *
 * @param device Valid device handle.
 * @param fp file to log the dump
 *
 * @return KBP_OK on success or an error code otherwise.
 */
kbp_status kbp_device_dump_interface_regs(struct kbp_device *device, FILE *fp);

/**
 * Dumps Last result register data
 *
 * @param device Valid device handle.
 * @param only_reg_data 1:dumps only the last result register content, 0:dumps data in readable format
 * @param fp file to log the dump
 *
 * @return KBP_OK on success or an error code otherwise.
 */
kbp_status kbp_device_read_last_result(struct kbp_device *device, uint32_t only_reg_data, FILE *fp);

/**
 * Read DIE Temperature
 *
 * @param device valid device handle.
 * @param temperature DIE temperature value in degree centigrade
 *
 * @return KBP_OK on success or an error code otherwise.
 */
kbp_status kbp_device_read_die_temperature(struct kbp_device *device, int32_t *temperature);

/**
 * Read DIE Temperature
 *
 * @param device valid device handle.
 * @param voltage DIE voltage value in milli volts
 *
 * @return KBP_OK on success or an error code otherwise.
 */
kbp_status kbp_device_read_die_voltage(struct kbp_device *device, int32_t *voltage);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif
#endif /*__DEVICE_H */
