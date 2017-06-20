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

#ifndef __XPT_H
#define __XPT_H

#include "kbp_legacy.h"
#include "kbp_portable.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file xpt_12k.h
 *
 *  This module contains the functions and data structures for 12K transport layer.
 *
 *  It calls the device-specific transport routines internally. All requests to
 *  underlying devices go through this layer. The upper layer calls the
 *  kbp_xpt_get_request routine to get a pointer to a request object, which needs
 *  to be initialized  appropriately by the caller. Routine
 *  kbp_xpt_service_requests is called to trigger execution of the requests
 *  already in the queue.
 *  A "request" is a structure that has a request code to indicate
 *  read/write/compare operation, the data for the request, and address on the device
 *  Appropriate device-specific routines are called by xpt to execute the
 *  requests.
 *
 * @addtogroup O3S_XPT_APIS
 * @{
 */

#define     MAX_COMP_RESULT         (4)
#define     MAX_READ_SIZE           (80)
#define     INVALID_VAL             (-1)
#define     MAX_DATA_LEN_BYTES      (80)
#define     XPT_MAX_MESSAGE_SIZE    (255)
#define     INVALID_CTX_ADDR        (0xffff)
#define     XPT_MAGIC_NUMBER        (0x12344)
#define     NLM_XPT_OPCODE_CNT      (2)


/* Opcodes supported by the device */
#define NLM_OPCODE_NOP_BITS_8_6             (0x0)
#define NLM_OPCODE_NOP_BITS_5_0             (0x0)

#define NLM_OPCODE_REG_WRITE_BITS_8_6       (0x0)
#define NLM_OPCODE_REG_WRITE_BITS_5_0       (0x1)

#define NLM_OPCODE_DBA_WRITE_BITS_8_6       (0x0)
#define NLM_OPCODE_DBA_WRITE_BITS_5_0       (0x1)

#define NLM_OPCODE_UDA_WRITE_BITS_8_6       (0x0)
#define NLM_OPCODE_UDA_WRITE_BITS_5_0       (0x1)

#define NLM_OPCODE_REG_READ_BITS_8_6        (0x0)
#define NLM_OPCODE_REG_READ_BITS_5_0        (0x2)

#define NLM_OPCODE_UDA_READ_BITS_8_6        (0x0)
#define NLM_OPCODE_UDA_READ_BITS_5_0        (0x2)

#define NLM_OPCODE_DBA_READ_X_BITS_8_6      (0x0)
#define NLM_OPCODE_DBA_READ_X_BITS_5_0      (0x2)

#define NLM_OPCODE_DBA_READ_Y_BITS_8_6      (0x0)
#define NLM_OPCODE_DBA_READ_Y_BITS_5_0      (0x3)

#define NLM_OPCODE_EM_WRA_BITS_8_6          (0x0)
#define NLM_OPCODE_EM_WRA_BITS_5_0          (0x4)

#define NLM_OPCODE_EM_WRB_BITS_8_6          (0x0)
#define NLM_OPCODE_EM_WRB_BITS_5_0          (0x5)

#define NLM_OPCODE_EM_RDA_BITS_8_6          (0x0)
#define NLM_OPCODE_EM_RDA_BITS_5_0          (0x6)

#define NLM_OPCODE_EM_RDB_BITS_8_6          (0x0)
#define NLM_OPCODE_EM_RDB_BITS_5_0          (0x7)

#define NLM_OPCODE_BLOCK_COPY_BITS_8_6      (0x0)
#define NLM_OPCODE_BLOCK_COPY_BITS_5_0      (0x8)

#define NLM_OPCODE_BLOCK_MOVE_BITS_8_6      (0x0)
#define NLM_OPCODE_BLOCK_MOVE_BITS_5_0      (0x9)

#define NLM_OPCODE_BLOCK_CLEAR_BITS_8_6     (0x0)
#define NLM_OPCODE_BLOCK_CLEAR_BITS_5_0     (0xA)

#define NLM_OPCODE_BLOCK_EV_BITS_8_6        (0x0)
#define NLM_OPCODE_BLOCK_EV_BITS_5_0        (0xB)

#define NLM_OPCODE_LPMTYPE1_BITS_8_6            (0x0)
#define NLM_OPCODE_LPMTYPE1_BITS_5_0            (0xC)

#define NLM_OPCODE_LPMTYPE2_BITS_8_6            (0x0)
#define NLM_OPCODE_LPMTYPE2_BITS_5_0            (0xD)

#define NLM_OPCODE_LPMTYPE3_BITS_8_6            (0x0)
#define NLM_OPCODE_LPMTYPE3_BITS_5_0            (0xE)

#define NLM_OPCODE_CBWRITE_CMP1_BITS_8_6        (0x1)
#define NLM_OPCODE_CBWRITE_CMP2_BITS_8_6        (0x2)
#define NLM_OPCODE_CBWRITE_CMP3_BITS_8_6        (0x3)
#define NLM_OPCODE_CBWRITE_BITS_8_6             (0x4)
#define NLM_OPCODE_CBWLPM_BITS_8_6              (0x5)

/* ILA Data Word definitions */
#define NLM_DATA_WORD_LEN_IN_BYTES              (8)
#define NLM_MAX_DATA_WORDS                      (5)
#define NLM_LPMTYPE1_NUM_DATA_WORDS             (5)
#define NLM_LPMTYPE2_NUM_DATA_WORDS             (2)
#define NLM_LPMTYPE3_NUM_DATA_WORDS             (2)



/**
 * Describes a request for read/write/compare operations. Element
 * m_results should hold the address for a  valid Result structure. This
 * address should be initialized by caller only.
 */

typedef struct kbp_xpt_rqt_t {
    /* Definitions used in no-SMT/2-SMT modes (for SMT-0 in 2-SMT mode) */
    uint8_t m_opcode[NLM_XPT_OPCODE_CNT];
    uint32_t m_data_len;
    uint16_t m_ctx_addr;

    uint32_t m_address;
    uint8_t *m_data_p;
    uint8_t *m_mask_p;

    /* Definitions used for SMT-1 in 2-SMT mode */
    uint8_t m_opcode1[NLM_XPT_OPCODE_CNT];
    uint32_t m_data_len1;
    uint16_t m_ctx_addr1;


    uint8_t *m_data1_p;

    /* Following flag will be set when there are instructions on both SMTs */
    int32_t m_2smt;

    uint8_t m_smt_num;
    uint8_t m_port_num;
    uint8_t m_response_req;
    uint8_t m_format_data;
    int32_t m_ADorIdx;          /* set this bit if the GCR b'30 = 1 */

    void *m_result;
} kbp_xpt_rqt;

/**
 * Used to identify a request. The xpt layer will issue a
 * unique request ID to each request structure returned to the caller.
 */

typedef struct kbp_xpt_rqt_id_t {
    uint32_t m_giga_count;
    uint32_t m_count;
} kbp_xpt_rqt_id;

struct kbp_xpt_operations_t;

/**
 * Generic xpt structure that includes a pointer to the
 * actual XPT implementation structure
 */
typedef struct kbp_xpt_t {
    uint32_t device_type;   /**< Indicates the device type. Should always be the first member of the structure. */
    struct kbp_xpt_operations_t *m_oprs;
    void *m_derived;
    NlmCmAllocator *m_alloc;

    uint32_t m_xpt_chanel_id;

    int32_t m_locked;
    uint32_t xpt_magic;

    uint8_t m_port_mode;
    uint8_t m_smt_mode;

    uint32_t bc_bitmap;  /**< broadcast bitmap */

    uint8_t m_payload_size;
#ifdef NPX_NPS
    uint32_t m_xpt_side;
#endif

} kbp_xpt;

/**
 * Structure holds the list of callback operations the transport
 * layer will invoke. The user is expected to implement most of
 * these callback functions.
 */

typedef struct kbp_xpt_operations_t {
    /**
     * Sends a filled-in result structure to the upper layer of software.
     * It returns NULL if under either of the following circumstances:
     *   @li The ::kbp_xpt_rqt object is not in the in_use request list.
     *   @li The request is not completed.
     * If the ::kbp_xpt_rqt object passed to the function is NULL, the result for the
     * first request in the in_use request list is returned. If kbp_xpt_get_result
     * returns a valid result, the corresponding request object will be moved to the free-request list.
     *
     * @param self Pointer to valid transport layer handle.
     * @param rqt The request requiring a response. NULL returns the first available result.
     * @param o_reason Legacy reason code for older transport implementations.
     *
     * @retval NULL if no available result.
     * @retval ::kbp_xpt_rqt if valid result found.
     */

    void *(*kbp_xpt_get_result) (void *self, kbp_xpt_rqt *rqt, NlmReasonCode *o_reason);

    /**
     * Causes a rqt object to be moved from the in_use list to the free list.
     * It may be called when some error is encountered while formatting the requests or
     * before issuing ServiceRequest.
     *
     * @param self Pointer to valid transport layer handle.
     * @param rqt The request to recycle.
     * @param o_reason Legacy reason code for older transport implementations.
     *
     */

    void (*kbp_xpt_discard_result) (void *self, kbp_xpt_rqt *rqt, NlmReasonCode *o_reason);

    /**
     * Allocate a new request object for the caller. Only request memory is made
     * available. The request is not yet transmitted to the device.
     *
     * @param self Pointer to valid transport layer handle.
     * @param o_reason Legacy reason code for older transport implementations.
     *
     * @retval NULL if no available memory for request.
     * @retval ::kbp_xpt_rqt on success.
     */

    kbp_xpt_rqt *(*kbp_xpt_get_request) (void *self, NlmReasonCode *o_reason);

    /**
     * Clears all pending requests and moves all the requests to the
     * free-request list.
     *
     * @param self Pointer to valid transport layer handle.
     * @param o_reason Legacy reason code for older transport implementations.
     */

    void (*kbp_xpt_reset_requests) (void *self, NlmReasonCode *o_reason);

    /**
     * Triggers the execution of the requests in the
     * in_use request queue that are not executed already.
     * Requests are executed in FIFO order.
     *
     * @param self Pointer to valid transport layer handle.
     * @param o_reason Legacy reason code for older transport implementations.
     */

    void (*kbp_xpt_service_requests) (void *self, NlmReasonCode *o_reason);

    /**
     * Resets the device. Any work requests submitted earlier
     * will be cleared.
     *
     * @param self Pointer to valid transport layer handle.
     * @param o_reason Legacy reason code for older transport implementations.
     */

    NlmErrNum_t(*kbp_xpt_reset_device) (void *self, NlmReasonCode *o_reason);

    /**
     * Locks an XPT configuration so that it cannot be modified.
     *
     * @param self Pointer to valid transport layer handle.
     * @param num_devices The number of cascaded devices.
     * @param o_reason Legacy reason code for older transport implementations.
     */
    NlmErrNum_t(*kbp_xpt_lock_config) (void *, uint32_t, NlmReasonCode *);
} kbp_xpt_operations;

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif                          /* __XPT_H */


