/*
 * $Id: cosq.h,v 1.5 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * This file contains cosq module definitions internal to the BCM library.
 */

#ifndef _BCM_INT_TK371X_COSQ_H
#define _BCM_INT_TK371X_COSQ_H

#include <bcm/types.h>
#include <bcm/cosq.h>


#define BCM_TK371X_COSQ_LLID_INVALID         -1
#define BCM_TK371X_COSQ_QUEUE_MAX            32
#define BCM_TK371X_COSQ_QUEUE_GROUP_MAX      10

#define BCM_TK371X_COSQ_STAT_MAX             8

#define BCM_TK371X_COSQ_REPORT_SET_MAX       4

typedef struct bcm_tk371x_cosq_group_s {
    uint32   port_link; 
    uint32   numq;
    uint32   flags;
} bcm_tk371x_cosq_group_t;

typedef struct bcm_tk371x_cosq_stat_map_s {
    uint16  standard_id;
    uint16  tk_id;
} bcm_tk371x_cosq_stat_map_t;

extern int 
_bcm_tk371x_cosq_stat_sync(
        int unit);
        
extern int        
_bcm_tk371x_cosq_gport_get(
	    int unit,
	    bcm_port_t port,
	    int cosq,
	    bcm_gport_t *gport);
       
#endif /* _BCM_INT_TK371X_COSQ_H */
