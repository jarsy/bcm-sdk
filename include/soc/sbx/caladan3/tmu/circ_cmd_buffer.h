/*
 * $Id: circ_cmd_buffer.h,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 * Purpose: Thread safe circular buffer 
 */

#ifndef _SBX_CALADN3_CIRC_BUF_H_
#define _SBX_CALADN3_CIRC_BUF_H_

#include <soc/types.h>
#include <soc/sbx/caladan3/tmu/cmd.h>

typedef struct circ_cmd_buffer_s {
    int read_pos;
    int write_pos;
    soc_sbx_caladan3_tmu_cmd_t **buffer;
    int length;
    sal_mutex_t mutex;  
} circ_cmd_buffer_t;

extern int circ_cmd_buffer_initFromBuf(int unit, circ_cmd_buffer_t *cbuf, 
                                      soc_sbx_caladan3_tmu_cmd_t **buffer, 
                                      int length);
extern int circ_cmd_buffer_destroy(int unit, circ_cmd_buffer_t *cbuf);
extern int circ_cmd_buffer_full(int unit, circ_cmd_buffer_t *cbuf);
extern int circ_cmd_buffer_empty(int unit, circ_cmd_buffer_t *cbuf);
extern int circ_cmd_buffer_put(int unit, circ_cmd_buffer_t *cbuf, soc_sbx_caladan3_tmu_cmd_t *element, uint8 overflow);
extern int circ_cmd_buffer_get(int unit, circ_cmd_buffer_t *cbuf, soc_sbx_caladan3_tmu_cmd_t **element);
extern void circ_cmd_buffer_printf(int unit, circ_cmd_buffer_t *cbuf);

#endif
