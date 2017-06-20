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

#ifndef __KBP_HB_H
#define __KBP_HB_H

 /**
  * @file kbp_hb.h
  *
  * Hit Bit implementation
  */

#include <stdint.h>
#include <stdio.h>

#include "errors.h"
#include "device.h"
#include "db.h"

#ifdef __cplusplus
 extern "C" {
#endif

 /**
  * * @addtogroup HB_API
  * @{
  */

 /**
  * Opaque Hit Bit database handle.
  */

 struct kbp_hb_db;

 /**
  * Opaque HB handle.
  */

 struct kbp_hb;

 /**
  * Creates an Hit Bit database
  *
  * @param device Valid device handle.
  * @param id Database ID. This is a control-plane identifier only.
  * @param capacity The number of hit bits managed by this database. Zero means dynamic capacity.
           Dynamic capacity is not supported at this time
  * @param hb_dbp Database handle, initialized and returned on success.
  *
  * @returns KBP_OK on success or an error code otherwise.
  */

 kbp_status kbp_hb_db_init(struct kbp_device *device, uint32_t id, uint32_t capacity,
                           struct kbp_hb_db **hb_dbp);

 /**
  * Destroys the Hit Bit database
  *
  * @param hb_db Valid database handle to destroy.
  *
  * @returns KBP_OK on success or an error code otherwise.
  */

 kbp_status kbp_hb_db_destroy(struct kbp_hb_db *hb_db);

 /**
  * Sets properties for the database.
  *
  * @param hb_db Valid database handle.
  * @param property The property to set, defined by ::kbp_db_properties.
  * @param ... Variable arguments to set properties.
  *
  * @return KBP_OK on success or an error code otherwise.
  */

 kbp_status kbp_hb_db_set_property(struct kbp_hb_db *hb_db, enum kbp_db_properties property, ...);

 /**
  * Gets properties for the database.
  *
  * @param hb_db valid database handle
  * @param property The property to get defined by ::kbp_db_properties
  * @param ... variable arguments to get properties into
  *
  * @return KBP_OK on success or an error code
  */

 kbp_status kbp_hb_db_get_property(struct kbp_hb_db *hb_db, enum kbp_db_properties property, ...);

 /**
  * Creates the hit bit entry.
  *
  * @param hb_db Valid database handle.
  * @param hb Hit Bit handle returned on success. Can be used to associate LPM database entries.
  *
  * @return KBP_OK on success or an error code otherwise.
  */

 kbp_status kbp_hb_db_add_entry(struct kbp_hb_db *hb_db, struct kbp_hb **hb);

 /**
  * Deletes the Hit Bit entry. If any database entries continue to point to the
  * Hit Bit, the operation will fail.
  *
  * @param hb_db Valid database handle.
  * @param hb Hit Bit handle to delete.
  *
  * @return KBP_OK on success or an error code otherwise.
  */

 kbp_status kbp_hb_db_delete_entry(struct kbp_hb_db *hb_db, struct kbp_hb *hb);

 /**
  * Deletes all Hit Bit entries in the database. Assumes there are
  * no live references to the Hit Bits present
  *
  * @param hb_db Valid AD database handle.
  *
  * @return KBP_OK on success or an error code otherwise.
  */

 kbp_status kbp_hb_db_delete_all_entries(struct kbp_hb_db *hb_db);

 /**
  * Performs a timer tick on the Hit Bit database. The timer operations
  * allows software to collect the Hit Bit information for all the
  * entries active in the database. No entries are aged or impacted,
  * the timer is used as a signal to count unreached entries for
  * aging subsequently.
  *
  * @param hb_db valid Hit bit database handle
  *
  * @return KBP_OK on success or an error code
  */
 kbp_status kbp_hb_db_timer(struct kbp_hb_db *hb_db);


/**
  * Returns the list of entries that are aged based on the age count specified.
  * The API fills the provided pre-allocated entry buffer with aged entries upto a
  * max of buf_size. The actual number of entries returned is filled in num_entries.
  *
  * @param hb_db valid Hit bit database handle
  * @param buf_size size of the entry handle buffer provided
  * @param num_entries returns the number of entries that are filled
  * @param entries Buffer to fill the aged entry handles
  *
  * @return KBP_OK on success or an error code
  */

kbp_status kbp_db_get_aged_entries (struct kbp_hb_db *hb_db, uint32_t buf_size, uint32_t *num_entries,
                                    struct kbp_entry **entries);


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
kbp_status kbp_hb_db_refresh_handle(struct kbp_device *device, struct kbp_hb_db *stale_ptr, struct kbp_hb_db **dbp);



/**
 * @brief Opaque entry iterator handle.
 */

struct kbp_aged_entry_iter;

/**
 * Creates an iterator to iterate over all aged entries in the database.
 * Currently entries cannot be deleted when iterating over them
 *
 * @param db Valid hit bit database handle.
 * @param iter Iterator initialized and returned on success.
 *
 * @retval KBP_OK on success.
 * @retval KBP_OUT_OF_MEMORY if out of memory.
 */

kbp_status kbp_hb_db_aged_entry_iter_init(struct kbp_hb_db *hb_db, struct kbp_aged_entry_iter **iter);

/**
 * Gets the next aged entry in the database, or NULL if the last entry.
 *
 * @param db Valid hit bit database handle.
 * @param iter Valid initialized iterator.
 * @param entry A non-NULL valid entry handle returned on success, or NULL after the last entry.
 *
 * @return KBP_OK on success or an error code otherwise.
 */

kbp_status kbp_hb_db_aged_entry_iter_next(struct kbp_hb_db *hb_db, struct kbp_aged_entry_iter *iter, struct kbp_entry **entry);

/**
 * Reclaims the iterator resources.
 *
 * @param db Valid hit bit database handle.
 * @param iter Valid initialized iterator.
 *
 * @return KBP_OK on success or an error code otherwise.
 */
kbp_status kbp_hb_db_aged_entry_iter_destroy(struct kbp_hb_db *hb_db, struct kbp_aged_entry_iter *iter);


/**
 * Gets the idle count for the hit bit
 *
 * @param db Valid hit bit database handle.
 * @param hb hit bit entry for which we need the idle count
 * @param idle_count the idle count for the entry is returned here
 *
 * @return KBP_OK on success or an error code otherwise.
 */
kbp_status kbp_hb_entry_get_idle_count(struct kbp_hb_db *hb_db, struct kbp_hb *hb, uint32_t *idle_count);

/**
 * Sets the idle count for the hit bit
 *
 * @param db Valid hit bit database handle.
 * @param hb hit bit entry for which we need to set the idle count
 * @param idle_count the idle count which should be set
 *
 * @return KBP_OK on success or an error code otherwise.
 */
kbp_status kbp_hb_entry_set_idle_count(struct kbp_hb_db *hb_db, struct kbp_hb *hb, uint32_t idle_count);


/**
 * Gets the bit value for the hit bit
 *
 * @param db Valid hit bit database handle.
 * @param hb hit bit entry for which we need the bit value
 * @param bit value the bit value for the entry is returned here
 * @param clear_on_read if clear_on_read is 1, then the hit bit will be cleared in hardware
 *
 * @return KBP_OK on success or an error code otherwise.
 */
kbp_status kbp_hb_entry_get_bit_value(struct kbp_hb_db *hb_db, struct kbp_hb *hb_handle, uint32_t *bit_value, uint8_t clear_on_read);



/**
 * @}
 */


#ifdef __cplusplus
 }
#endif
#endif /*__KBP_HB_H */

