/*
 * $Id: msgq.h,v 1.1 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:     msgq.h
 * Purpose:
 *
 */

#ifndef _SOC_EA_MSGQ_H
#define _SOC_EA_MSGQ_H

#include <sal/types.h>
#include <sal/core/sync.h>

#define SOC_EA_MSGQ_OK        0x00000000
#define SOC_EA_MSGQ_BAD_PARAM 0x80000001
#define SOC_EA_MSGQ_NO_MEM    0x80000002
#define SOC_EA_MSGQ_FULL      0x80000003
#define SOC_EA_MSGQ_EMPTY     0x80000004
#define SOC_EA_MSGQ_TIMEOUT   0x80000005
#define SOC_EA_MSGQ_FAULT     0x80000006

typedef struct soc_ea_msgq_node_s {
    uint8        *buf;
    int          len;
} soc_ea_msgq_node_t;

typedef struct soc_ea_msgq_s {
    uint8                   *base_ptr;
    struct soc_ea_msgq_s    *self;
    sal_sem_t               sem;
    int                     max_msgs;
    int                     max_msg_len;
    int                     head;
    int                     tail;
    int                     count;
    int                     ecode;
    soc_ea_msgq_node_t      *nodes;
} soc_ea_msgq_t;

extern soc_ea_msgq_t * soc_ea_msgq_create (char *name, int max_msg_len, int msg_num);
extern int soc_ea_msgq_destroy (soc_ea_msgq_t *msgq);
extern int soc_ea_msgq_send (soc_ea_msgq_t *msgq, char *msg, int len);
extern int soc_ea_msgq_recv (soc_ea_msgq_t *msgq, uint8 *buf, int max_bytes, int usec);
extern int soc_ea_msgq_count_get (soc_ea_msgq_t *msgq);
extern int soc_ea_msgq_error_get (soc_ea_msgq_t *msgq);
extern int soc_ea_msgq_clear (soc_ea_msgq_t *msgq);
extern void soc_ea_msgq_show (soc_ea_msgq_t *msgq);

#endif /* _SOC_EA_MSGQ_H */
