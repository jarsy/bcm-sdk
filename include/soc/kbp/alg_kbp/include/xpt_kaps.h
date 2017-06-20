/*
 **************************************************************************************
 Copyright 2009-2017 Broadcom Corporation

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

#ifndef __XPT_KAPS_H
#define __XPT_KAPS_H

/**
 * @file xpt_kaps.h
 *
 * This module contains the functions and data structures for the transport layer of the KAPS device.
 */

#include <stdint.h>
#include "errors.h"


#ifdef __cplusplus
extern "C" {
#endif

#define KAPS_AD_WIDTH_8 (3)


/**
 *   @brief KAPS HW Commands
 */
enum kaps_cmd
{
    KAPS_CMD_READ,     /**< KAPS command to read through IBC */
    KAPS_CMD_WRITE,    /**< KAPS command to write through IBC */
    KAPS_CMD_EXTENDED, /**< KAPS command for extended functionality through IBC */
    KAPS_CMD_ACK       /**< KAPS IBC command acknowledgement*/
};



/**
 *   @brief KAPS HW Functions
 */
enum kaps_func
{
    KAPS_FUNC0,       /**< KAPS IBC function 0 **/
    KAPS_FUNC1,       /**< KAPS IBC function 1 **/
    KAPS_FUNC2,       /**< KAPS IBC function 2 **/
    KAPS_FUNC3,       /**< KAPS IBC function 3 **/
    KAPS_FUNC4,       /**< KAPS IBC function 4 **/
    KAPS_FUNC5,       /**< KAPS IBC function 5 **/
    KAPS_FUNC6,       /**< KAPS IBC function 6 **/
    KAPS_FUNC7,       /**< KAPS IBC function 7 **/
    KAPS_FUNC8,       /**< KAPS IBC function 8 **/
    KAPS_FUNC9,       /**< KAPS IBC function 9 **/
    KAPS_FUNC10,      /**< KAPS IBC function 10 **/
    KAPS_FUNC11,      /**< KAPS IBC function 11 **/
    KAPS_FUNC12,      /**< KAPS IBC function 12 **/
    KAPS_FUNC13,      /**< KAPS IBC function 13 **/
    KAPS_FUNC14,      /**< KAPS IBC function 14 **/
    KAPS_FUNC15,      /**< KAPS IBC function 15 **/
    KAPS_FUNC16       /**< Temporary IBC function 16*/
};

/**
 *   @brief KAPS Search interface
 */

enum kaps_search_interface
{
    KAPS_SEARCH_0_A,    /**< s0a search interface*/
    KAPS_SEARCH_0_B,    /**< s0b search interface*/
    KAPS_SEARCH_1_A,    /**< s1a search interface*/
    KAPS_SEARCH_1_B     /**< s1b search interface*/
};

/**
 * @brief Search result from the KAPS device
 */

struct kaps_search_result
{
    uint8_t ad_value[KAPS_AD_WIDTH_8];  /**< The associated data value of the matching entry*/
    uint32_t match_len; /**< Length of the best match */
};





/**
 * @addtogroup KAPS_XPT_APIS
 * @{
 */

/**
 * KAPS transport-layer implementation structure expected to be passed into
 * the call to kbp_device_init().
 */

struct kaps_xpt
{
    /**
    * The device type. Should always be the first member of the structure.
    * Should be set to KBP_DEVICE_KAPS.
    */
    uint32_t device_type;


    /**
    * Search function.
    *
    * @param xpt pointer to the xpt that was passed to kbp_device_init() is passed back here.
    * @param key the data to be searched in the KAPS device. For example, if five bytes of data have to be searched, these
    * bytes will be stored in key[0] to key[4].
    * @param search_interface the search interface (s0a...s1b) to be used to search the key.
    * @param kaps_result the result of the KAPS search is returned here.
    *
    * @return KBP_OK on success or an error code otherwise.
    */
    kbp_status (*kaps_search) (void *xpt, uint8_t *key, enum kaps_search_interface search_interface, struct kaps_search_result *kaps_result);


    /**
    * Function to send various private commands to KAPS
    *
    * @param xpt pointer to the xpt that was passed to kbp_device_init() is passed back here.
    * @param cmd Command type
    * @param func Function to be selected for command
    * @param blk_nr Block Number to send the instruction
    * @param row_nr Row Number to send the instruction
    * @param nbytes the number of bytes in payload
    * @param bytes the payload
    *
    * @return KBP_OK if successfully dispatched to device
    */
    kbp_status (*kaps_command)(void *xpt, enum kaps_cmd cmd, enum kaps_func func, uint32_t blk_nr, uint32_t row_nr, uint32_t nbytes, uint8_t *bytes);

    /**
    * Function to send various register read commands to KAPS
    *
    * @param xpt pointer to the xpt that was passed to kbp_device_init() is passed back here.
    * @param offset Offset of the register to be read
    * @param nbytes the number of bytes in payload
    * @param bytes the payload
    *
    * @return KBP_OK if successfully dispatched to device
    */
    kbp_status (*kaps_register_read)(void *xpt, uint32_t offset, uint32_t nbytes, uint8_t *bytes);

    /**
    * Function to send various register write commands to KAPS
    *
    * @param xpt pointer to the xpt that was passed to kbp_device_init() is passed back here.
    * @param offset Offset of the register to be written
    * @param nbytes the number of bytes in payload
    * @param bytes the payload
    *
    * @return KBP_OK if successfully dispatched to device
    */
    kbp_status (*kaps_register_write)(void *xpt, uint32_t offset, uint32_t nbytes, uint8_t *bytes);


    /**
    * Function to send hit bit read commands to KAPS
    *
    * @param xpt pointer to the xpt that was passed to kbp_device_init() is passed back here.
    * @param blk_nr block from which the hit bits should be read. starts from 0.
    * @param row_nr row from which the hit bits should be read
    * @param bytes the payload, the number of bytes is equal to KAPS_HB_ROW_WIDTH_8
    *
    * @return KBP_OK if successfully dispatched to device
    */
    kbp_status (*kaps_hb_read)(void *xpt, uint32_t blk_nr, uint32_t row_nr, uint8_t *bytes);

    /**
    * Function to send hit bit write commands to KAPS
    *
    * @param xpt pointer to the xpt that was passed to kbp_device_init() is passed back here.
    * @param blk_nr block where the hit bits should be written. starts from 0
    * @param row_nr row where the hit bits should be written
    * @param bytes the payload, the number of bytes is equal to KAPS_HB_ROW_WIDTH_8
    *
    * @return KBP_OK if successfully dispatched to device
    */
    kbp_status (*kaps_hb_write)(void *xpt, uint32_t blk_nr, uint32_t row_nr, uint8_t *bytes);




    /**
    * Function to dump the hit bits
    *
    * @param xpt pointer to the xpt that was passed to kbp_device_init() is passed back here.
    * @param start_blk_num starting hit bit block from where the hit bits should be collected. starts from 0
    * @param start_row_num starting row number in the start hit bit block from where the hit bits should be collected
    * @param end_blk_num ending hit bit block up to where the hit bits should be collected. starts from 0
    * @param end_row_num ending row number in the end hit bit block up to where the hit bits should be collected
    * @param clear_on_read clears the hit bits after reading them. Currently always set to 1
    * @param data  the collected hit bits will be stored here
    *
    * @return KBP_OK if successfully dispatched to device
    */
    kbp_status (*kaps_hb_dump)(void *xpt, uint32_t start_blk_num, uint32_t start_row_num, uint32_t end_blk_num,
                                uint32_t end_row_num, uint8_t clear_on_read, uint8_t *data);

    /**
    * Function to copy the hit bits
    *
    * @param xpt pointer to the xpt that was passed to kbp_device_init() is passed back here.
    * @param src_blk_num source hit bit block from where the hit bits should be copied. starts from 0
    * @param src_row_num row number in the source hit bit block from where the hit bits should be copied
    * @param dest_blk_num destination hit bit block to where the hit bits should be copied. starts from 0
    * @param dest_row_num row number in the destination  hit bit block to where the hit bits should be copied
    * @param source_mask mask bits applied after reading hit bits before the rotate operation
    * @param rotate_right number of bits by which the result should be rotated right
    * @param perform_clear indicates if the hit bits at the source be cleared after the read
    *
    * @return KBP_OK if successfully dispatched to device
    */
    kbp_status (*kaps_hb_copy)(void *xpt, uint32_t src_blk_num, uint32_t src_row_num, uint32_t dest_blk_num,
                                uint32_t dest_row_num, uint16_t source_mask, uint8_t rotate_right, uint8_t perform_clear);


};


/**
 * @}
 */

#ifdef __cplusplus
}
#endif


#endif /* __XPT_KAPS_H */

