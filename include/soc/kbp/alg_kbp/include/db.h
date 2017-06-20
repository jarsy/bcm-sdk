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

#ifndef __DB_H
#define __DB_H

#include <stdint.h>

#include "errors.h"
#include "device.h"
#include "hw_limits.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 *
 * @file db.h
 *
 * APIs to manage databases in the KBP.
 * @addtogroup KBP_DB_API
 * @{
 */

/**
 * @brief Opaque user handle to the database stored in the KBP.
 */

struct kbp_db;

/**
 * @brief Opaque handle to an entry within a database.
 */

struct kbp_entry;

/**
 * @brief Opaque AD handle.
 */

struct kbp_ad;

/**
 * @brief Opaque HB handle.
 */

struct kbp_hb;


/**
 * @brief Opaque key handle.
 */

struct kbp_key;

/**
 * @brief Opaque associated-data database handle.
 */

struct kbp_ad_db;

/**
 * @brief db insertion mode
 */

enum kbp_db_insertion_mode {
    KBP_DB_INSERTION_MODE_NONE,
    KBP_DB_INSERTION_MODE_NORMAL,
    KBP_DB_INSERTION_MODE_RELATIVE
};


/**
 * @brief Relative insertion position.
 */

enum kbp_entry_position
{
    KBP_DB_ENTRY_ADD_BEFORE = 0,
    KBP_DB_ENTRY_ADD_AFTER = 1
};


/**
 * @brief Meta priority for entries on the KBP.
 *
 * Meta priority can be set using the following APIs :
 *
 *- kbp_db_set_property()
 *- kbp_entry_set_property()
 *
 * Properties can be read with  call to kbp_entry_get_info.
 */


enum kbp_db_entry_meta_priority {
KBP_ENTRY_META_PRIORITY_0 = 0, /* Highest Priority */
KBP_ENTRY_META_PRIORITY_1 = 1,
KBP_ENTRY_META_PRIORITY_2 = 2,
KBP_ENTRY_META_PRIORITY_3 = 3  /* Lowest Priority */
};

/**
 * @brief Opaque Hit Bit database handle.
 */

struct kbp_hb_db;

/**
 * @brief Properties for databases on the KBP.
 *
 * Properties that can be set using the following APIs (only
 * KBP_PROP_DESCRIPTION can be set for AD databases) :
 *
 *- kbp_db_set_property()
 *- kbp_ad_db_set_property()
 *
 * Properties can be read with equivalent call to get_property.
 */

enum kbp_db_properties {
    KBP_PROP_USE_MCOR,       /**< For ACL databases, use MCOR for the range at the specified offset. */
    KBP_PROP_MCOR_VALUE,     /**< For ACL databases, add a most commonly occurring range. */
    KBP_PROP_ALGORITHMIC,    /**< Indicates whether the database should be algorithmic or non-algorithmic. */
    KBP_PROP_INDEX_RANGE,    /**< Index range for user-side AD management. */
    KBP_PROP_INDEX_CALLBACK, /**< Register callback function to notify users when entries are moved. */
    KBP_PROP_MIN_PRIORITY,   /**< Hint on minimum priority used in the database. (Helps reduce shuffles.) */
    KBP_PROP_DESCRIPTION,    /**< String description for device/databases. */
    KBP_PROP_CASCADE_DEVICE, /**< Request this database be placed on specific cascaded KBP device. Valid only when cascade is detected */
    KBP_PROP_REDUCED_INDEX_CALLBACKS, /**< If set, reduces Index callbacks. */
    KBP_PROP_SAVE_IX_SPACE,           /**< Saves the index space for LPM with external AD */
    KBP_PROP_DMA_TAG,                 /**< DMA Tag for the database, KAPS specific. */
    KBP_PROP_ENTRY_META_PRIORITY,     /**< Default Meta priority for the entries added to the DB. */
    KBP_PROP_ENABLE_DB_COMPACTION,    /**< enable db compaction */
    KBP_PROP_MAX_CAPACITY,            /**< sets the maximum allowed capacity of a DB */
    KBP_PROP_AGE_COUNT,               /**< The count after which the entry is considered for aging */
    KBP_PROP_PAIR_WITH_DB,            /**< This db is paired with other db sharing same DBA resource */
    KBP_PROP_DEFER_DELETES,           /**< Defer deletes to install */
    KBP_PROP_SCALE_UP_CAPACITY,       /**< Scale up the DB capacity by vertically partitioning the DBA blocks */
    KBP_PROP_LOG,                     /**< Log the entry add, delete operations into a file */
    KBP_PROP_MAX_PRIORITY,            /**< Hint on maximum priority used in the database. (Helps reduce shuffles.) */
    KBP_PROP_INVALID                  /**< This should be the last entry */
};

/**
 * @brief Properties for entries on the KBP.
 *
 * Properties that can be set using the following API :
 *
 *- kbp_entry_set_property()
 * *
 * Properties can be read with call to kbp_entry_get_info.
 */

enum kbp_entry_properties {
    KBP_ENTRY_PROP_META_PRIORITY, /**< For LPM databases sets the entries meta priority. */
    KBP_ENTRY_PROP_INVALID        /**< This should be the last entry */
};



/**
 * Initializes a KBP database of the specified type. A database can
 * be a flat collection of entries or can be a collection of smaller
 * tables. A database can be partitioned into multiple smaller
 * databases or tables using the API call kbp_db_add_table(). A key
 * must be added to every database and table. Entries must be
 * added using the specific database handles only.
 *
 * The capacity argument presented to the database determines the
 * resources allocated to the device. The intent of this argument
 * is to handle the following situations.
 *
 * Zero value: If the capacity specified is zero the database
 * is treated as being dynamic in nature. The database can grow
 * and shrink based on available resources at that point in the device.
 *
 * non-zero value: Defines as minimum capacity required. Enough resources
 * are allocated to the database to satisfy the requested capacity.
 * Once the database reaches the requested capacity, if additional free
 * resources are available after satisfying the non-zero capacity values
 * of all other databases, this database will be allowed to grow into the
 * free space. If the databases reduces in size, the extra free resources
 * borrowed may be returned back into the free pool for other databases
 * to use, however, the base resources allocated to satisfy the capacity
 * argument specified are always maintained.
 *
 * @note currently as of KBP SDK 1.3.5 only static allocation of resources
 * is supported. The user is expected to provide a valid capacity value.
 * If zero capacity is provided, no resources will be allocated to the
 * database. Additionally there is no guarantee currently that the database
 * may grow into available free space.
 *
 * @param device Valid device handle.
 * @param type The database type. Can be ACL, LPM, or EM. ::kbp_db_type.
 * @param id A unique database identifier. This is a control-plane identifier only.
 * @param capacity The minimum required capacity for this database.
 * @param dbp Database handle initialized and returned on success.
 *
 * @return KBP_OK on success or an error code otherwise.
 */

kbp_status kbp_db_init(struct kbp_device *device, enum kbp_db_type type,
                       uint32_t id, uint32_t capacity, struct kbp_db **dbp);

/**
* Initializes a KBP database in the given device,
* using the above API kbp_db_init with the
* parameters of an already created DB 'ori_db'. Then marks this newly
* created db (bc_db) as a broadcast DB of the original db.
* Broadcast DB means all the entries added to orig_db, will also
* be broadcasted to newly created DB in the given device.
* No ADD, DELETE, and INSTALL operations should be performed on the
* broadcast DB, rest is as usual.
*
* @param device Valid device handle.
* @param ori_db Valid DB handle.
* @param bc_db Broadcast database handle initialized and returned on success.
*
* @return KBP_OK on success or an error code otherwise.
*/
kbp_status kbp_db_create_broadcast(struct kbp_device *device,
                                   struct kbp_db *ori_db, struct kbp_db **bc_db);


/**
* Searches the given device and returns the pointer to DB which
* matches with the given id and type and has no parent
*
* @param device Valid device handle.
* @param id DB id to search.
* @param type DB type to search.
*
* @return Pointer to the db on success, NULL otherwise.
*/

struct kbp_db* kbp_db_get_bc_db_on_device(struct kbp_device *device, struct kbp_db *inp_db);


/**
 * Destroys the database. Recovers memory and any hardware resources
 * reserved by this database. All entries with the database are
 * invalidated.
 *
 * @param db Valid database pointer to destroy.
 *
 * @return KBP_OK on success or an error code otherwise.
 */

kbp_status kbp_db_destroy(struct kbp_db *db);

/**
 * Assigns a specific hardware resource to the device.
 *
 * @param db Valid database handle.
 * @param resource Hardware resource defined by ::kbp_hw_resource.
 * @param ... Variable arguments defined by the property.
 *
 * @return KBP_OK on success or an error code otherwise.
 */

kbp_status kbp_db_set_resource(struct kbp_db *db, enum kbp_hw_resource resource, ...);

/**
 * Gets the resource information for this database.
 *
 * @param db Valid database handle.
 * @param resource Hardware resource defined by ::kbp_hw_resource.
 * @param ... Variable arguments defined by the property returned.
 *
 * @return KBP_OK on success or an error code otherwise.
 */

kbp_status kbp_db_get_resource(struct kbp_db *db, enum kbp_hw_resource resource, ...);

/**
 * Adds a key that defines the layout of the entries in the database. This
 * information is used to program the KPUs in the device.
 *
 * @see KEY
 *
 * Ranges, if used by ACL databases, must be described
 * by setting the key-type attribute to ranges in the API
 * kbp_key_add_field(). This information is
 * implicitly determines the appropriate
 * range technology to use and apply in the hardware.
 *
 * @param db Valid database handle.
 * @param key Valid key handle.
 *
 * @return KBP_OK on success or an error code otherwise.
 */

kbp_status kbp_db_set_key(struct kbp_db *db, struct kbp_key *key);

/**
 * Sets properties for the database. Available properties defined by the enumeration ::kbp_db_properties. The properties
 * should be followed by values within the ranges given below.
 *
 * |PROPERTY|VALUES|DEFAULT|
 * |:---:|:---:|:---:|
 * |KBP_PROP_MCOR_VALUE| 16-bit range value, that frequently occurs for ports | None (future use) |
 * |KBP_PROP_USE_MCOR| The range number starting from MSB of key specified by database. Count starts from zero | None (future use) |
 * |KBP_PROP_INDEX_CALLBACK| The callback function | None. No callback is performed |
 * |KBP_PROP_DESCRIPTION|String description|None|
 * |KBP_PROP_ALGORITHMIC|1, 0|Algorithmic. Passing zero disables algorithmic processing|
 * |KBP_PROP_REDUCED_INDEX_CALLBACKS|1, 0|Reduced Callbacks. Passing one enables reduced callbacks mode|
 *
 * @param db Valid initialized KBP database.
 * @param property ::kbp_db_properties.
 * @param ... The required number of arguments for the property.
 *
 * @return KBP_OK on success or an error code otherwise.
 */

kbp_status kbp_db_set_property(struct kbp_db *db, enum kbp_db_properties property, ...);

/**
 * Gets properties of the database. Using this API, one can verify that
 * the database is configured with the correct property values.
 *
 * @param db Valid initialized KBP database.
 * @param property ::kbp_db_properties.
 * @param ... The returned values of the property.
 *
 * @return KBP_OK on success or an error code otherwise.
 */

kbp_status kbp_db_get_property(struct kbp_db *db, enum kbp_db_properties property, ...);

/**
 * Pretty-prints the database information.
 *
 * @param db Initialized KBP database.
 * @param fp The file to print to.
 *
 * @return KBP_OK on success or an error code otherwise.
 */

kbp_status kbp_db_print(struct kbp_db *db, FILE * fp);

/**
 * Adds an Associated Data (AD) database to the database. This
 * allows the control plane to associate the two databases,
 * partition the hardware resources, and program the chip.
 *
 * @param db Valid database handle.
 * @param ad_db Valid associated-data database handle.
 *
 * @return KBP_OK on success or an error code otherwise.
 */

kbp_status kbp_db_set_ad(struct kbp_db *db, struct kbp_ad_db *ad_db);

/**
 * Adds an Hit Bit (HB) database to the database. This
 * allows the control plane to associate the two databases,
 * partition the hardware resources, and program the chip.
 *
 * @param db Valid database handle.
 * @param hb_db Valid Hit Bit database handle.
 *
 * @return KBP_OK on success or an error code otherwise.
 */

kbp_status kbp_db_set_hb(struct kbp_db *db, struct kbp_hb_db *hb_db);

/**
 * Initializes a table inside the database. This API returns a different
 * database handle. A key must be added to this database to be
 * able to add it to instructions or to add entries to it. All tables
 * within the database share the same resources and cannot be searched
 * in parallel with each other. To ensure correctness when creating
 * tables within a database, there must be distinguishing bits in the
 * key (typically referred to as Table ID) that prevent getting false
 * hits from other tables in the database. It is the responsibility of
 * the user to ensure such bits are present. To ensure correctness
 * when adding keys to database that is a collection of tables, one of
 * the key tuples must be of the type KBP_KEY_FIELD_TABLE_ID as described
 * in ::kbp_key_field_type. This tuple must occur in all the keys at the
 * same offset.
 *
 * @param db Initialized KBP database.
 * @param id Control-plane identifier only, unique within the database.
 * @param table Opaque database/table handle, initialized and returned on success.
 *
 * The returned database/table handle must then be associated with its
 * key and entries of this table must be added only using this handle.
 *
 * @return KBP_OK on success or an error code otherwise.
 */

kbp_status kbp_db_add_table(struct kbp_db *db, uint32_t id, struct kbp_db **table);

/**
 * Creates a multiaccess database/table. The database/table may be cloned as
 * many times as required to meet parallelism requirements. The control plane will
 * determine the most effective way of storing the databases/tables. After
 * cloning, a key must be associated with the cloned database/table.
 *
 * @param db Initialized KBP database/table.
 * @param id Control-plane identifier only; a unique ID.
 * @param clone Opaque database/table clone, initialized and returned on success.
 *
 * The returned database/table handle must then be associated with its
 * key.
 *
 * @return KBP_OK on success or an error code otherwise.
 */

kbp_status kbp_db_clone(struct kbp_db *db, uint32_t id, struct kbp_db **clone);

/**
 * Adds a single ACL entry (ACE) to the KBP database. The
 * database must be initialized as ACL type. The bits that
 * represent ranges should be set to don't cares, and the API
 * kbp_entry_add_range() should be used to specify the actual
 * range values the key is compared against.
 *
 * @param db Initialized KBP databases
 * @param data Data bits represented as array of bytes.
 * @param mask Don't-care mask for the bits (1 == don't care).
 * @param priority Priority of the entry (only 22 bits of 32-bit value are valid).
 * @param entry Opaque entry handle, initialized and returned to user on success.
 *
 * @return KBP_OK on success or an error code otherwise.
 */

kbp_status kbp_db_add_ace(struct kbp_db *db, uint8_t *data, uint8_t *mask,
                          uint32_t priority, struct kbp_entry **entry);


/**
 * This API overwrites the existing TCAM ACL entry and it is not coherent.
 *
 * @param db Initialized KBP databases
 * @param entry entry handle that needs to be updated.
 * @param data Data bits represented as array of bytes.
 * @param mask Don't-care mask for the bits (1 == don't care).
 *
 * @return KBP_OK on success or an error code otherwise.
 */

kbp_status kbp_db_update_ace(struct kbp_db *db, struct kbp_entry *entry, uint8_t *data, uint8_t *mask);

/**
 * Adds a single ACL entry (ACE) to the KBP database
 * in a relative position to an earlier entry. The
 * database must be initialized as ACL type. The bits that
 * represent ranges should be set to don't cares, and the API
 * kbp_entry_add_range() should be used to specify the actual
 * range values the key is compared against. The priority field
 * is used only when relative entry is NULL, SDk finds the entry which
 * is of equal priority and inserts the new entry before it. Only
 * either kbp_db_relative_add_ace or kbp_db_add_ace can be used for a DB,
 * both the API shall not be used to insert entries into the same DB, means
 * if kbp_db_add_ace has been used for a DB then kbp_db_relative_add_ace
 * should not be used and vice-versa.
 *
 * @param db Initialized KBP databases
 * @param data Data bits represented as array of bytes.
 * @param mask Don't-care mask for the bits (1 == don't care).
 * @param relative_entry the relative entry
 * @param before_or_after whether to insert before or after the relative entry.
 * @param priority Priority of the entry (only 22 bits of 32-bit value are valid).
 * @param entry Opaque entry handle, initialized and returned to user on success.
 *
 * @return KBP_OK on success or an error code otherwise.
 */
kbp_status kbp_db_relative_add_ace(struct kbp_db *db, uint8_t *data, uint8_t *mask,
                                   struct kbp_entry *relative_entry,
                                   enum kbp_entry_position entry_position,
                                   uint32_t priority,
                                   struct kbp_entry **entry);

/**
 * Adds a single LPM entry to the KBP database. The
 * database must be initialized as LPM type.
 *
 * @param db Initialized KBP database.
 * @param prefix Prefix to add.
 * @param length The length of the prefix in bits.
 * @param entry Pointer to opaque handle, initialized and returned to user on success.
 *
 * @return KBP_OK on success or an error code otherwise.
 */

kbp_status kbp_db_add_prefix(struct kbp_db *db, uint8_t *prefix, uint32_t length,
                             struct kbp_entry **entry);

/**
 * Adds a single exact-match entry to the KBP database. The
 * database must be initialized as EM type.
 *
 * @param db Initialized KBP database.
 * @param data The exact-match data. Width must match the database width.
 * @param entry Pointer to opaque handle, initialized and returned to user on success.
 *
 * @return KBP_OK on success or an error code otherwise.
 */


kbp_status kbp_db_add_em(struct kbp_db *db, uint8_t *data, struct kbp_entry **entry);

/**
 * Deletes the specified entry from the database. All
 * memory associated with the entry are recovered.
 * Any hardware resources associated with the entry are also
 * invalidated.
 *
 * @param db Initialized KBP database.
 * @param entry The entry to delete.
 *
 * @return KBP_OK on successful deletion of entry or an error code otherwise.
 */

kbp_status kbp_db_delete_entry(struct kbp_db *db, struct kbp_entry *entry);

/**
 * Sets properties for the entry. Available properties defined by the enumeration ::kbp_entry_properties. The properties
 * should be followed by values within the ranges given below.
 *
 * |PROPERTY|VALUES|DEFAULT|
 * |:---:|:---:|:---:|
 * |KBP_ENTRY_PROP_META_PRIORITY| items in enum kbp_db_entry_meta_priority | Meta priority set for the db. KBP_ENTRY_META_PRIORITY_3 if none set for db |
 *
 * @param entry Valid LPM entry.
 * @param property ::kbp_entry_properties.
 * @param ... The required number of arguments for the property.
 *
 * @return KBP_OK on success or an error code otherwise.
 */

kbp_status kbp_entry_set_property(struct kbp_db *db, struct kbp_entry *entry, enum kbp_entry_properties property, ...);

/**
 * Adds a range comparison to the ACL entry. The comparison
 * is treated as >= lo && <= hi. The range number specified
 * must correspond to the range field as specified in the
 * database key starting from the MSB size. Ranges are counted
 * starting from zero.
 *
 * @param db Initialized KBP database.
 * @param entry Valid handle returned using the call kbp_db_add_ace().
 * @param lo The low end of the range.
 * @param hi The high end of the range.
 * @param range_no The range number, starting from MSB side in the ACL key.
 *
 * @return KBP_OK on success or an error code otherwise.
 */

kbp_status kbp_entry_add_range(struct kbp_db *db, struct kbp_entry *entry,
                               uint16_t lo, uint16_t hi, int32_t range_no);

/**
 * Adds associated data to the database entry
 *
 * @param db Initialized KBP database.
 * @param entry Valid handle returned using the call kbp_db_add_ace() kbp_db_add_prefix(), or kbp_db_add_em().
 * @param ad Associated-data handle
 *
 * @return KBP_OK on success or an error code otherwise.
 */

kbp_status kbp_entry_add_ad(struct kbp_db *db, struct kbp_entry *entry, struct kbp_ad *ad);

/**
 * Adds Hit Bits to the database entry
 *
 * @param db Initialized KBP database.
 * @param entry Valid handle returned using the call kbp_db_add_ace() kbp_db_add_prefix(), or kbp_db_add_em().
 * @param hb Hit Bit data handle
 *
 * @return KBP_OK on success or an error code otherwise.
 */

kbp_status kbp_entry_add_hb(struct kbp_db *db, struct kbp_entry *entry, struct kbp_hb *hb);

/**
 * Pretty-prints the database entry.
 *
 * @param db Initialized KBP database.
 * @param entry Valid handle returned using the call kbp_acl_db_add_entry().
 * @param fp The file to print to.
 *
 * @return KBP_OK on success or an error code otherwise.
 */

kbp_status kbp_entry_print(struct kbp_db *db, struct kbp_entry *entry, FILE * fp);

/**
 * Compiles and installs any pending added/deleted entries in the KBP database.
 *
 * @param db Valid KBP database.
 *
 * @return KBP_OK on success or an error code otherwise.
 */

kbp_status kbp_db_install(struct kbp_db *db);

/**
 * Deletes all entries in the database/table.
 *
 * @param db Valid KBP database or table.
 *
 * @return KBP_OK on success or an error code otherwise.
 */

kbp_status kbp_db_delete_all_entries(struct kbp_db *db);


/**
 * Deletes all the pending entries in the database/table.
 *
 * @param db Valid KBP database or table.
 *
 * @return KBP_OK on success or an error code otherwise.
 */
kbp_status kbp_db_delete_all_pending_entries(struct kbp_db *db);

/**
 * @brief Statistics for the database.
 *
 * The user can obtain the current number of entries in an ACL/LPM
 * database, including number of entries due to Range expansion. One can
 * determine how Range expansion is impacting database capacity.
 * An estimate for the remaining capacity for the database can be obtained
 * by subtracting number of entries in database num_entries from
 * capacity_estimate.
 */

struct kbp_db_stats {
    uint32_t num_entries;       /**< Total entries in database. */
    uint32_t capacity_estimate; /**< Estimate of the number of entries the database can fit. */
    uint32_t range_expansion;   /**< Number of entries due to range expansion. */
};

/**
 * Returns the statistics for the KBP database.
 *
 * @param db Initialized KBP database.
 * @param stats Valid memory where stats are populated.
 *
 * @return KBP_OK on success or an error code otherwise.
 */

kbp_status kbp_db_stats(struct kbp_db *db, struct kbp_db_stats *stats);

/**
 * Prototype for index range callback function. When database entries are moved
 * on the device, the function is called back to notify the user to move
 * the associated data. If old_index is
 * -1, the entry is being installed for the first time. The
 * index callback function can be installed by calling the following function.
 *
 * kbp_db_set_property(db, KBP_PROP_INDEX_CALLBACK, <function_pointer>, handle);
 *
 * For ACL entries, due to range expansion, the callback function may be
 * called more than once for the same ACL entry. In this case, old_index will be set to -1
 * on the first call.
 */

typedef void (*kbp_db_index_callback)(void *handle, struct kbp_db *db, struct kbp_entry *entry,
                                      int32_t old_index, int32_t new_index);

/**
 * Looks up a prefix handle by providing the prefix and the length. This
 * API is provided for NPUs that potentially handle LPM databases in
 * a stateless manner without saving the entry handles. This API works
 * only for prefixes and may not have very high performance. There is
 * no equivalent functionality available for retrieving ACL entry
 * handles.
 *
 * @param db Valid database handle to which the prefix was originally added.
 * @param prefix Valid prefix.
 * @param length The length of the prefix in bits.
 * @param entry Pointer where the entry handle is returned, if found.
 *
 * @retval KBP_PREFIX_NOT_FOUND when entry cannot be found
 * @retval KBP_OK on success, and entry pointer is valid
 */

kbp_status kbp_db_get_prefix_handle(struct kbp_db *db, uint8_t *prefix, uint32_t length, struct kbp_entry **entry);

/**
 * Performs a software search in the database. It searches for the highest
 * matching ACL entry for ACLs, the exact-matching entry for exact-match searches, or
 * for the longest prefix match for LPM searches.
 *
 * This API can be used to
 * validate the SW structures of the database by comparing the returned
 * hit-index/AD with expected. The expected value must be calculated taking
 * into account the fact that the returned Index/AD depends on the priority
 * of stored entries / longest matched prefix.
 *
 * @param db Valid initialized KBP database.
 * @param key The search-key bits, which must match the key layout specified by the user when initializing the database.
 * @param entry Returned on a successful match. Should be referenced only when KBP_OK is returned. NULL entry means no hit.
 * @param index The hit index for the entry on a match. Should be referenced only when KBP_OK is returned. -1 means no match.
 * @param prio_len Returns the priority of the entry for ACLs, the prefix match length for LPM searches, or zero for exact-match searches.
 *
 * @return KBP_OK on success or an error code otherwise.
 */

kbp_status kbp_db_search(struct kbp_db *db, uint8_t *key, struct kbp_entry **entry,
                         int32_t *index, int32_t *prio_len);

/**
 * @brief Opaque entry iterator handle.
 */

struct kbp_entry_iter;

/**
 * Creates an iterator to iterate over all entries in the database.
 * Currently entries cannot be deleted when iterating over them
 *
 * @param db Valid database handle.
 * @param iter Iterator initialized and returned on success.
 *
 * @retval KBP_OK on success.
 * @retval KBP_OUT_OF_MEMORY if out of memory.
 */

kbp_status kbp_db_entry_iter_init(struct kbp_db *db, struct kbp_entry_iter **iter);

/**
 * Gets the next entry in the database, or NULL if the last entry.
 *
 * @param db Valid database handle.
 * @param iter Valid initialized iterator.
 * @param entry A non-NULL valid entry handle returned on success, or NULL after the last entry.
 *
 * @return KBP_OK on success or an error code otherwise.
 */

kbp_status kbp_db_entry_iter_next(struct kbp_db *db, struct kbp_entry_iter *iter, struct kbp_entry **entry);

/**
 * Reclaims the iterator resources.
 *
 * @param db Valid database handle.
 * @param iter Valid initialized iterator.
 *
 * @return KBP_OK on success or an error code otherwise.
 */

kbp_status kbp_db_entry_iter_destroy(struct kbp_db *db, struct kbp_entry_iter *iter);

/**
 * Returns the priority of an ACL entry, or prefix length of an LPM entry. For
 * exact-match entries, a value of zero is always filled in.
 *
 * @param db Valid database handle.
 * @param entry Valid KBP-entry handle.
 * @param prio_length Priority for ACL entries, prefix length for LPM, or zero for EM.
 *
 * @return KBP_OK on success or an error code otherwise.
 */

kbp_status kbp_entry_get_priority(struct kbp_db *db, struct kbp_entry *entry, uint32_t *prio_length);

/**
 * Returns the associated-data handle for an entry.
 *
 * @param db Valid database handle.
 * @param entry Valid KBP-entry handle.
 * @param ad Associated-data handle, returned on success.
 *
 * @return KBP_OK on success or an error code otherwise.
 */

kbp_status kbp_entry_get_ad(struct kbp_db *db, struct kbp_entry *entry, struct kbp_ad **ad);


/**
 * Returns the hit bit handle for an entry.
 *
 * @param db Valid database handle.
 * @param entry Valid KBP-entry handle.
 * @param hit bit handle, returned on success.
 *
 * @return KBP_OK on success or an error code otherwise.
 */
kbp_status kbp_entry_get_hb(struct kbp_db *db, struct kbp_entry *entry, struct kbp_hb **hb);




/**
 * Return the indexes for the KBP entry. For LPM and EM, there
 * is typically a single index at which the entry resides. For
 * ACL entries depending on range expansion, it is possible the
 * ACL entry expands to multiple locations in the KBP. An array of
 * indices are returned to the user. The index array returned
 * must be freed using the API kbp_entry_free_index_array().
 *
 * @param db Valid database handle.
 * @param entry Valid KBP entry handle.
 * @param nindices Set by API to return the number of valid indexes.
 * @param indices Array of indexes of length nindex which holds the indexes for this entry.
 *
 * @return KBP_OK on success or an error code otherwise.
 */

kbp_status kbp_entry_get_index(struct kbp_db *db, struct kbp_entry *entry, int32_t *nindices, int32_t **indices);

/**
 * Frees the index array returned by the API call kbp_entry_get_index().
 *
 * @param db Valid database handle.
 * @param indices Array of indexes returned by API kbp_entry_get_index().
 *
 * @return KBP_OK on success or an error code otherwise.
 */

kbp_status kbp_entry_free_index_array(struct kbp_db *db, int32_t *indices);

/**
 * The following structure is populated and returned
 * by the API kbp_entry_get_info().
 */

struct kbp_entry_info
{
    uint8_t width_8;   /**< The number of valid bytes in the data/mask array below. */
    uint8_t nranges;   /**< The number of valid ranges in the range array below. */
    uint8_t active;    /**< Status of the entry. If set to 1, the entry has been installed.
                            If not, it is pending, waiting to be installed. */

    uint8_t which_half; /**< Entry is placed on which half of DBA block. 0: on both halves,
                             1: on left half and 2: on right half. Default is 0. */

    uint16_t meta_priority; /* Meta priority of the entry */
    uint32_t prio_len; /**< Priority for ACL entry, prefix length for LPM, or zero for EM. */
    uint8_t data[KBP_HW_MAX_DBA_WIDTH_8]; /**< The data portion of the entry, which is width_8 bytes wide. */
    uint8_t mask[KBP_HW_MAX_DBA_WIDTH_8]; /**< The mask portion of the entry, which is width_8 bytes wide. */
    struct kbp_ad *ad_handle; /**< Used to return the AD handle if present or, if not, is set to NULL. */
    struct kbp_entry_rinfo {
        uint16_t lo;  /**< The low part of the range. */
        uint16_t hi;  /**< The high part of the range. */
    } rinfo[KBP_HW_MAX_RANGE_COMPARES]; /**< Range information, where nranges specifies the number of valid range segments. Ranges are ordered from MSB to LSB. */
};

/**
 * Obtains data/mask information for the given entry. The user is expected
 * to pass a valid entry-info structure, which is populated by the API.
 *
 * @param db Valid database handle.
 * @param entry Valid handle for the entry being retrieved.
 * @param info Populated and returned by API on success.
 *
 * @return KBP_OK on success or an error code otherwise.
 */

kbp_status kbp_entry_get_info(struct kbp_db *db, struct kbp_entry *entry, struct kbp_entry_info *info);

/**
 *@}
 */

/**
 * @addtogroup ISSU_API
 * @{
 */

/**
 * Marks an entry as in-use. This API must only be invoked in the
 * reconciliation phase for ISSU. At the end of the reconciliation
 * phase, any entries that are not marked in-use will be garbage-collected.
 *
 * @param db Valid database handle.
 * @param entry Valid entry handle.
 *
 * @return KBP_OK on success or an error code otherwise.
 */

kbp_status kbp_entry_set_used(struct kbp_db *db, struct kbp_entry *entry);

/**
 * Returns the new DB Handle after ISSU operation
 *
 *
 * @param device the KBP device handle
 * @param stale_ptr DB handle before ISSU.
 * @param dbp New DB handle.
 *
 * @retval KBP_OK on success and result structure is populated.
 * @retval KBP_INVALID_ARGUMENT for invalid or null parameters.
 */

kbp_status kbp_db_refresh_handle(struct kbp_device *device, struct kbp_db *stale_ptr, struct kbp_db **dbp);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif
#endif                          /* __DB_H */
