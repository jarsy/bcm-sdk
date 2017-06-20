/*
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * THIS SOFTWARE IS OFFERED "AS IS", AND BROADCOM GRANTS NO WARRANTIES OF ANY
 * KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE.
 * BROADCOM SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
 *
 * $Id: work_queue.h,v 1.12 Broadcom SDK $
 *
 * TAPS work queue library defines/interfaces
 *
 *-----------------------------------------------------------------------------*/
#ifndef _SBX_CALADN3_TAPS_WORK_QUEUE_H_
#define _SBX_CALADN3_TAPS_WORK_QUEUE_H_

#include <soc/sbx/sbDq.h>

typedef enum taps_work_type_e_s {
    TAPS_TCAM_WORK,
    TAPS_SBUCKET_WORK,
    TAPS_DBUCKET_DATA_WORK,
    TAPS_DBUCKET_WORK,
    TAPS_TCAM_PROPAGATION_WORK,
    TAPS_SBUCKET_PROPAGATION_WORK,
    TAPS_REDISTRIBUTE_STAGE1_WORK,
    TAPS_REDISTRIBUTE_STAGE2_WORK,
    TAPS_REDISTRIBUTE_STAGE3_WORK,
    TAPS_WORK_TYPE_MAX
} taps_work_type_e_t;

#define _TAPS_VALID_WORK_TYPE_(type) \
    (type >= TAPS_TCAM_WORK && type < TAPS_WORK_TYPE_MAX)

typedef struct taps_wgroup_s {
    dq_t  work_group_list_node;
    unsigned int wgroup; /* group id */
    unsigned int force_work_type_enable; /* force work type enable */
    taps_work_type_e_t forced_work_type; /* when force_work_type_enable is TRUE, always use this work type */
    dq_t  work_list[TAPS_WORK_TYPE_MAX]; /* work item list */
    uint8 host_share;
} taps_wgroup_t, *taps_wgroup_handle_t;

#define _TAPS_MAX_WGROUP_ (1) /* increase when batching */
typedef struct taps_wq_s {
    dq_t  work_group_list;
} taps_wq_t, *taps_wq_handle_t;

#define _WQ_DEQUEUE_DEFAULT_ (0) /* from head */
#define _WQ_DEQUEUE_TAIL_ (1)

/*
 *
 * Function:
 *   taps_work_queue_init
 * Purpose:
 *   Init work queue
 * Parameters
 *   (IN)  unit              : unit number of the device
 *   (OUT) p_wq              : return the handle of work queue object
 * Returns
 *   SOC_E_NONE - successfully created a work queue
 *   SOC_E_* as appropriate otherwise
 */
extern int taps_work_queue_init(int unit, taps_wq_handle_t *p_wq);

/*
 *
 * Function:
 *   taps_work_queue_destroy
 * Purpose:
 *   Destroy work queue
 * Parameters
 *   (IN)  unit            : unit number of the device
 *   (OUT) wq              : handle of work queue object
 * Returns
 *   SOC_E_NONE - successfully created a work queue
 *   SOC_E_* as appropriate otherwise
 */
extern int taps_work_queue_destroy(int unit, taps_wq_handle_t wq);

/*
 *
 * Function:
 *   taps_work_group_create
 * Purpose:
 *   creates a taps work group
 * Parameters
 *   (IN) unit         : unit number of the device
 *   (IN) work_queue   : handle returned by taps_work_queue_init
 *   (IN) work_group   : work group id
 * Returns
 *   SOC_E_NONE - successfully enqueued work payload object
 *   SOC_E_* as appropriate otherwise
 */
extern int taps_work_group_create(int unit, 
                                  taps_wq_handle_t work_queue, 
                                  unsigned int wgroup,
                                  taps_wgroup_handle_t *work_group_handle);

/*
 *
 * Function:
 *   taps_work_group_destroy
 * Purpose:
 *   destroy a taps work group
 * Parameters
 *   (IN) unit         : unit number of the device
 *   (IN) work_queue   : handle returned by taps_work_queue_init
 *   (IN) work_group   : work group id
 * Returns
 *   SOC_E_NONE - successfully enqueued work payload object
 *   SOC_E_* as appropriate otherwise
 */
extern int taps_work_group_destroy(int unit, 
                                   taps_wgroup_handle_t wghdl);

/*
 *
 * Function:
 *   taps_work_enqueue
 * Purpose:
 *   Enqueue work into work queue
 * Parameters
 *   (IN) unit         : unit number of the device
 *   (IN) work_group   : work group handle
 *   (IN) type         : work type to be enqueued
 *   (IN) work_item    : work item
 * Returns
 *   SOC_E_NONE - successfully enqueued work payload object
 *   SOC_E_* as appropriate otherwise
 */
extern int taps_work_enqueue(int unit, 
                             taps_wgroup_handle_t work_group,
                             taps_work_type_e_t type,
                             dq_p_t work_item);
/*
 *
 * Function:
 *   taps_work_dequeue
 * Purpose:
 *   Dequeue work queue. Return NULL indicate work_queue is empty
 * Parameters
 *   (IN) unit         : unit number of the device
 *   (IN) work_group   : work group handle
 *   (IN) type         : work type to be enqueued
 *   (OUT)work_item    : work item dequeued
 * Returns
 *   SOC_E_NONE - successfully dequeued work payload object
 *   SOC_E_EMPTY- work queue empty
 *   SOC_E_* as appropriate otherwise
 */
extern int taps_work_dequeue(int unit, 
                             taps_wgroup_handle_t work_group,
                             taps_work_type_e_t type,
                             dq_p_t *work_item,
                             /* used to dequeue last enqueued items comes handy with cleanup */
                             uint8 dq_tail /* 0 - default, 1 - pull out last enqueued item */ );

/*
 *
 * Function:
 *   taps_work_queue_iter_first
 * Returns
 *   SOC_E_NONE - if work available
 *   SOC_E_EMPTY- work queue empty
 *   SOC_E_* as appropriate otherwise
 */
extern int taps_work_queue_iter_first(int unit, 
                                      taps_wgroup_handle_t work_group,
                                      taps_work_type_e_t type,
                                      dq_p_t *work_item);

/*
 *
 * Function:
 *   taps_work_queue_iter_get_next
 * Returns
 *   SOC_E_NONE - if work available
 *   SOC_E_EMPTY- work queue empty
 *   SOC_E_* as appropriate otherwise
 */
extern int taps_work_queue_iter_get_next(int unit, 
                                         taps_wgroup_handle_t work_group,
                                         taps_work_type_e_t type,
                                         dq_p_t *work_item);
/*
 *
 * Function:
 *   taps_work_queue_stats
 * Returns
 *   SOC_E_NONE - if work available
 *   SOC_E_EMPTY- work queue empty
 *   SOC_E_* as appropriate otherwise
 */
extern int taps_work_queue_stats(int unit, 
                                 taps_wgroup_handle_t work_group,
                                 taps_work_type_e_t type);

#endif /* _SBX_CALADN3_TAPS_WORK_QUEUE_H_ */
