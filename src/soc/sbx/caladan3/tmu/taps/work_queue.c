/*
 * $Id: work_queue.c,v 1.9 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 * File:    work_queue.c
 * Purpose: Caladan3 work queue library (for now, designed for TAPS only)
 * Requires:
 */

#ifdef BCM_CALADAN3_SUPPORT
#include <shared/bsl.h>

#include <soc/types.h>
#include <soc/drv.h>
#include <soc/sbx/sbDq.h>
#include <soc/sbx/caladan3/tmu/taps/work_queue.h>

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
int taps_work_queue_init(int unit, taps_wq_handle_t *p_wq) 
{
    int status = SOC_E_NONE;
    taps_wq_handle_t work_queue = NULL;

    if (p_wq == NULL) {
	return SOC_E_PARAM;
    }

    /* alloc memory */
    work_queue = sal_alloc(sizeof(taps_wq_t), "taps-wq");
    if (work_queue == NULL) {
	LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s: unit %d Failed to allocate work queue dbase memory\n"),
                   FUNCTION_NAME(), unit));
	return SOC_E_MEMORY;
    } else {
        DQ_INIT(&work_queue->work_group_list);
        /* return handle */
        *p_wq = work_queue;
    }

    return status;
}

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
int taps_work_queue_destroy(int unit, taps_wq_handle_t wq) 
{
    dq_p_t elem;

    if (wq == NULL) {
	return SOC_E_PARAM;
    }

    if(!DQ_EMPTY(&wq->work_group_list)){
        DQ_TRAVERSE(&wq->work_group_list, elem) {
            taps_work_group_destroy(unit, 
                  DQ_ELEMENT_GET(taps_wgroup_handle_t, elem, work_group_list_node));
        } DQ_TRAVERSE_END(&wq->work_group_list, elem);
    }

    sal_free(wq);

    return SOC_E_NONE;
}

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
int taps_work_group_create(int unit, 
                           taps_wq_handle_t work_queue, 
                           unsigned int wgroup,
                           taps_wgroup_handle_t *work_group_handle)
{
    taps_wgroup_handle_t wghdl = NULL;
    int index;

    if ((work_queue == NULL) || (wgroup > _TAPS_MAX_WGROUP_)) {
	return SOC_E_PARAM;
    }

    wghdl = sal_alloc(sizeof(taps_wgroup_t), "taps-wgroup");
    if (wghdl) {
        for (index=0; index < TAPS_WORK_TYPE_MAX; index++) {
            DQ_INIT(&wghdl->work_list[index]);
        }
        /* insert to work queue manager */
        DQ_INSERT_TAIL(&work_queue->work_group_list, &wghdl->work_group_list_node);
        wghdl->wgroup = wgroup;
	wghdl->force_work_type_enable = FALSE;
	wghdl->forced_work_type = TAPS_WORK_TYPE_MAX;
    wghdl->host_share = FALSE;
        *work_group_handle = wghdl;
    } else {
        return SOC_E_MEMORY;
    }

    return SOC_E_NONE;
}

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
int taps_work_group_destroy(int unit, 
                            taps_wgroup_handle_t wghdl)
{
    dq_p_t elem;
    int index;

    if (!wghdl || (wghdl && wghdl->wgroup > _TAPS_MAX_WGROUP_)) {
	return SOC_E_PARAM;
    }

    /* walk and remove all work items */
    for(index=0; index < TAPS_WORK_TYPE_MAX; index++) {
        if(!DQ_EMPTY(&wghdl->work_list[index])){
            DQ_TRAVERSE(&wghdl->work_list[index], elem) {
                DQ_REMOVE(elem);
            } DQ_TRAVERSE_END(&wghdl->work_list[index], elem);
        }
    }
    
    DQ_REMOVE(&wghdl->work_group_list_node);
    sal_free(wghdl);

    return SOC_E_NONE;
}

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
int taps_work_enqueue(int unit, 
                      taps_wgroup_handle_t work_group,
                      taps_work_type_e_t type,
                      dq_p_t work_item) 
{
    if (!work_group || !work_item || \
	(type < 0) || (type >= TAPS_WORK_TYPE_MAX) || \
        (work_group && work_group->wgroup > _TAPS_MAX_WGROUP_)) {
	return SOC_E_PARAM;
    }

    /* force all work items to be enqueued to specified work list */
    if (work_group->force_work_type_enable == TRUE) {
	type = work_group->forced_work_type;
    }

    /* insert the work item */
    DQ_INSERT_TAIL(&work_group->work_list[type], work_item);
    return SOC_E_NONE;
}

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
int taps_work_dequeue(int unit, 
                      taps_wgroup_handle_t work_group,
                      taps_work_type_e_t type,
                      dq_p_t *work_item,
                      /* used to dequeue last enqueued items comes handy with cleanup */
                      uint8 dq_tail /* 0 - default, 1 - pull out last enqueued item */)
{
    int status = SOC_E_NONE;
    dq_p_t elem=NULL;

    if (!work_group || !work_item || \
	(type < 0) || (type >= TAPS_WORK_TYPE_MAX) || \
        (work_group && work_group->wgroup > _TAPS_MAX_WGROUP_)) {
	return SOC_E_PARAM;
    }

    if(!DQ_EMPTY(&work_group->work_list[type])){
        if (dq_tail) {
            DQ_REMOVE_TAIL(&work_group->work_list[type], elem);
        } else {
            /* get the work item */
            DQ_REMOVE_HEAD(&work_group->work_list[type], elem);
        }
        *work_item = elem;
    } else {
        status = SOC_E_EMPTY;
    }

    return status;
}

/*
 *
 * Function:
 *   taps_work_queue_iter_first
 * Returns
 *   SOC_E_NONE - if work available
 *   SOC_E_EMPTY- work queue empty
 *   SOC_E_* as appropriate otherwise
 */
int taps_work_queue_iter_first(int unit, 
                               taps_wgroup_handle_t work_group,
                               taps_work_type_e_t type,
                               dq_p_t *work_item)
{
    if (!work_group || !work_item || \
	(type < 0) || (type >= TAPS_WORK_TYPE_MAX) || \
        (work_group && work_group->wgroup > _TAPS_MAX_WGROUP_)) {
	return SOC_E_PARAM;
    }

    if(DQ_EMPTY(&work_group->work_list[type])) {
        return SOC_E_EMPTY;
    }

    *work_item = DQ_HEAD(&work_group->work_list[type], dq_p_t);
    return SOC_E_NONE;
}

/*
 *
 * Function:
 *   taps_work_queue_iter_get_next
 * Returns
 *   SOC_E_NONE - if work available
 *   SOC_E_EMPTY- work queue empty
 *   SOC_E_* as appropriate otherwise
 */
int taps_work_queue_iter_get_next(int unit, 
                                  taps_wgroup_handle_t work_group,
                                  taps_work_type_e_t type,
                                  dq_p_t *work_item)
{
    if (!work_group || !work_item || \
	(type < 0) || (type >= TAPS_WORK_TYPE_MAX) || \
        (work_group && work_group->wgroup > _TAPS_MAX_WGROUP_)) {
	return SOC_E_PARAM;
    }

    if(DQ_EMPTY(&work_group->work_list[type])) {
        return SOC_E_EMPTY;
    }

    if (DQ_TAIL(&work_group->work_list[type],dq_p_t) == *work_item) {
        return SOC_E_LIMIT;
    }


    *work_item = DQ_NEXT(*work_item,dq_p_t);
    return SOC_E_NONE;
}

/*
 *
 * Function:
 *   taps_work_queue_stats
 * Returns
 *   SOC_E_NONE - if work available
 *   SOC_E_EMPTY- work queue empty
 *   SOC_E_* as appropriate otherwise
 */
int taps_work_queue_stats(int unit, 
                          taps_wgroup_handle_t work_group,
                          taps_work_type_e_t type)
{
    if (!work_group || \
	(type < 0) || (type >= TAPS_WORK_TYPE_MAX) || \
        (work_group && work_group->wgroup > _TAPS_MAX_WGROUP_)) {
	return SOC_E_PARAM;
    }

    if(DQ_EMPTY(&work_group->work_list[type])){
        return SOC_E_EMPTY;
    } else {
        return SOC_E_NONE;
    }
}

#endif /* BCM_CALADAN3_SUPPORT */
