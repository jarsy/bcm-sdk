/* 
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 * $Id: etu_xpt.h,v 1.1 Broadcom SDK $
 */

#include <sal/types.h>

void
soc_sbx_caladan3_etu_xpt_destroy(void *self);
void *
soc_sbx_caladan3_etu_xpt_create(
                                 int        unit,
                                 uint32     devType,
                                 uint16     speed_mode,
                                 uint32     max_rqt_count,  
                                 uint16     opr_mode,      
                                 uint32     chan_id);
