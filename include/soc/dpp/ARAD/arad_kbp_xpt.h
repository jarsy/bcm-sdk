/* $Id: arad_kbp_xpt.h,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/

#ifndef __ARAD_KBP_XPT_INCLUDED__
/* { */
#define __ARAD_KBP_XPT_INCLUDED__

/*************
 * INCLUDES  *
 *************/
/* { */
#include <soc/dpp/SAND/Utils/sand_header.h>

/* } */
/*************
 * DEFINES   *
 *************/
/* { */

/* } */

/*************
 * MACROS    *
 *************/
/* { */

/* } */

/*************
 * TYPE DEFS *
 *************/
/* { */

/* } */

/*************
 * GLOBALS   *
 *************/
/* { */

/* } */

/*************
 * FUNCTIONS *
 *************/
/* { */

/*@@NlmAradXpt Module
 *  
 *  Summary 
 *  This module contains the functions and data structures for cmodel 
 *  transport layer.
 *  This module calls the cmodel specific routines internally.
 */

#include <soc/kbp/alg_kbp/include/default_allocator.h>
#include <soc/kbp/alg_kbp/include/xpt_12k.h>
#include <soc/kbp/alg_kbp/include/xpt_op.h>

struct kbp_search_result;

/*
 * NlmAradXpt__CreateXpt Creates the transport object xpt. The implementations of
 * all the operations of the xpt object is done by the aradxpt module, but they
 * are not exposed to the base xpt layer. This routine internally creates an xpt
 * object, does the proper initialization of the object and returns this object
 * to caller. 
 */
#define NlmXpt kbp_xpt
#define NlmXptRqt kbp_xpt_rqt
#define NlmRequestId kbp_xpt_rqt_id
#define NlmXpt_operations kbp_xpt_operations

extern NlmXpt*
arad_kbp_xpt_init(
    int                     unit,
    uint32                   core,
    NlmCmAllocator          *alloc,        /*General purpose memory allocator */
    uint32                 max_rqt_count, /*Max request count */
    uint32                 max_rslt_count, /*Max result count*/
    uint32                 chan_id   /* channel id */
    ) ;

/*
 * kbp_aradxpt_destroy frees all the memory associated with the xpt structure,
 * and also the memory associated with the derived structure which is the aradxpt
 * object here. 
 */

extern void
arad_kbp_xpt_destroy(
    NlmXpt* self 
) ;


extern NlmXpt*
arad_Nlm_Xpt_Create(
    int                     unit,
    uint32                  core,
    NlmCmAllocator          *alloc,        /*General purpose memory allocator */
    uint32                 max_rqt_count, /*Max request count */
    uint32                 max_rslt_count, /*Max result count*/
    uint32                 chan_id   /* channel id */
    ) ;

/*
 * NlmAradXpt__destroy frees all the memory associated with the xpt structure,
 * and also the memory associated with the derived structure which is the aradxpt
 * object here. 
 */

extern void
arad_Nlm_Xpt_destroy(
    NlmXpt* self 
) ;

/*
 * Specific to BCM15K, where control operations go through PCIE, however, we would
 * like all compares to still go through ROP to the KBP, we will override the
 * compare function
 */

extern kbp_status 
arad_op_search(
    void *hdl,
    uint32_t ltr,
    uint32_t ctx,
    const uint8_t *key,
    uint32_t key_len,
    struct kbp_search_result *result);

/*
 * Initialize the transport for BCM15K. Only handles compares
 */

extern void *
arad_kbp_op_xpt_init(
    int unit,
    nlm_u32 core,
    struct kbp_allocator *alloc);

/*
 * Destroy the Optimus Prime transport for compares
 */
extern void
arad_kbp_op_xpt_destroy(
    void* self
);

/* { */

#include <soc/dpp/SAND/Utils/sand_footer.h>

/* } __ARAD_KBP_XPT_INCLUDED__ */

#endif
