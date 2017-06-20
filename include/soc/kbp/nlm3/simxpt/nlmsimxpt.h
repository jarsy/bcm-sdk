/*
 * $Id: nlmsimxpt.h,v 1.1.6.3 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


 
#ifndef INCLUDED_NLMSIMXPT_H
#define INCLUDED_NLMSIMXPT_H

/*@@NlmSimXpt Module
 *  
 *  Summary 
 *  This module contains the functions and data structures for cmodel 
 *  transport layer.
 *  This module calls the cmodel specific routines internally.
 */

#ifndef NLMPLATFORM_BCM
#include <nlmcmbasic.h>
#include <nlmcmallocator.h>
#include <nlmxpt.h>
#else
#include <soc/kbp/common/nlmcmbasic.h>
#include <soc/kbp/common/nlmcmallocator.h>
#include <soc/kbp/nlm3/xpt/nlmxpt.h>
#endif

/*
 * NlmSimXpt__CreateXpt Creates the transport object xpt. The implementations of
 * all the operations of the xpt object is done by the simxpt module, but they
 * are not exposed to the base xpt layer. This routine internally creates an xpt
 * object, does the proper initialization of the object and returns this object
 * to caller. 
 */

extern NlmXpt*
kbp_simxpt_init(
    NlmCmAllocator          *alloc,        /*General purpose memory allocator */
    nlm_u32                 devType,
    nlm_u32                 max_rqt_count, /*Max request count */
    nlm_u32                 max_rslt_count, /*Max result count*/          
    nlm_u32                 chan_id   /* channel id */     
    ) ;

/*
 * kbp_simxpt_destroy frees all the memory associated with the xpt structure,
 * and also the memory associated with the derived structure which is the simxpt
 * object here. 
 */

extern void
kbp_simxpt_destroy(
    NlmXpt* self 
) ;


extern NlmXpt*
NlmSimXpt__Create(
    NlmCmAllocator          *alloc,        /*General purpose memory allocator */
    nlm_u32                 devType,
    nlm_u32                 max_rqt_count, /*Max request count */
    nlm_u32                 max_rslt_count, /*Max result count*/          
    nlm_u32                 chan_id   /* channel id */     
    ) ;

/*
 * NlmSimXpt__destroy frees all the memory associated with the xpt structure,
 * and also the memory associated with the derived structure which is the simxpt
 * object here. 
 */

extern void
NlmSimXpt__destroy(
    NlmXpt* self 
) ;

#ifndef NLMPLATFORM_BCM

extern NlmXpt*
bcm_kbp_simxpt_init(
    NlmCmAllocator          *alloc,        /*General purpose memory allocator */
    nlm_u32                 devType,
    nlm_u32                 max_rqt_count, /*Max request count */
    nlm_u32                 max_rslt_count, /*Max result count*/          
    nlm_u32                 chan_id   /* channel id */     
    ) ;

/*
 * kbp_simxpt_destroy frees all the memory associated with the xpt structure,
 * and also the memory associated with the derived structure which is the simxpt
 * object here. 
 */

extern void
bcm_kbp_simxpt_destroy(
    NlmXpt* self 
) ;

#endif

#endif
/*[]*/
