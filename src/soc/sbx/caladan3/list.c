/*
 * $Id:$
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 * File:    list.c
 * Purpose: Caladan3 on LRP List Manager
 * Requires: LRP and OCM drivers
 */

#include <shared/bsl.h>

#include <soc/types.h>
#include <soc/drv.h>

#ifdef BCM_CALADAN3_SUPPORT
#include <soc/sbx/caladan3.h>
#include <soc/sbx/caladan3/lrp.h>
#include <soc/sbx/caladan3/list.h>
#include <soc/sbx/caladan3/ocm.h>
#include <soc/sbx/caladan3/util.h>
#include <soc/sbx/sbx_drv.h>
#include <soc/cm.h>
#include <soc/mem.h>

/*
 *  LRP List Manager
 *       LRP has 4 Service processors. Each SVP can support 4 lists.
 *       Each list can be linear or wheel type.
 *       Each list can operate in
 *           Host producer and ucode consumer, 
 *           Ucode producer and host consumer modes
 *           Ucode producer and ucode consumer modes
 *       This driver provides means to maintain list indexes
 *       List memory may be managed by the application itself, for eg on the OCM/TMU DM
 *       This driver attempts to provide wrapper for OCM based access
 *
 *  Supports 
 *  Traditional list and wheel modes,
 *  C3 bidirectional, Host Producer Ucode Consumer,
 *  Ucode Producer Ucode Consumer, Ucode Producer Host Consumer,
 *
 */

#define SOC_SBX_CALADAN3_NUM_LISTS_PER_SVP 4
#define SOC_SBX_CALADAN3_NUM_SVP           5
#define SOC_SBX_CALADAN3_NUM_LIST_MGR  ((SOC_SBX_CALADAN3_NUM_LISTS_PER_SVP) * (SOC_SBX_CALADAN3_NUM_SVP))

typedef struct sbx_caladan3_lrp_listmgr_s {
    int                                    index;        /* Listmgr number */
    int                                    inuse;        /* Used/free */
    uint32                                 entry_size;   /* Size of each entry in list */
    uint32                                 num_entries;  /* Num entries  in list*/
    int                                    read_offset;  /* track last read offset */
    int                                    write_offset; /* track last written offset */
    int                                    enq_threshold; /* enqueue threshold */
    int                                    deq_threshold; /* dequeue threshold */
    soc_sbx_caladan3_lrp_svp_t             svp;          /* svp on which the list exists */
    soc_sbx_caladan3_lrp_list_type_e_t     type;         /* type of list */
    soc_sbx_caladan3_lrp_list_mode_e_t     mode;         /* mode of operation */

    /* OCM based memory management */
    sbx_caladan3_ocm_port_alloc_t          allocid;      /* OCM port/segment info when driver manages mem */
    uint32                                 base_offset;  /* Base of the list, only used when this driver manages mem */
    uint32                                 max_offset;   /* only used when this driver manages mem */
    uint32                                 cur_w_offset;   /* only used when this driver manages mem */
    uint32                                 cur_r_offset;   /* only used when this driver manages mem */
  
    /* Callback */
    soc_sbx_caladan3_lrp_list_enqueue_cb_f enqueue_func; /* Callback on enqueue */
    soc_sbx_caladan3_lrp_list_dequeue_cb_f dequeue_func; /* Callback on dqueue */
    void                                  *dequeue_context;
    void                                  *enqueue_context;

    /* Stats */
    int                                    num_enqueues;
    int                                    num_dequeues;
} soc_sbx_caladan3_lrp_listmgr_t;

typedef struct {
    soc_sbx_caladan3_lrp_listmgr_t listmgr[SOC_SBX_CALADAN3_NUM_LIST_MGR];
} sbx_caladan3_lrp_list_cb_t;

soc_sbx_caladan3_lrp_svp_t mgr_svp_map[] = {
    SOC_SBX_CALADAN3_SVP0, SOC_SBX_CALADAN3_SVP0, SOC_SBX_CALADAN3_SVP0, SOC_SBX_CALADAN3_SVP0,
    SOC_SBX_CALADAN3_SVP1, SOC_SBX_CALADAN3_SVP1, SOC_SBX_CALADAN3_SVP1, SOC_SBX_CALADAN3_SVP1,
    SOC_SBX_CALADAN3_SVP2, SOC_SBX_CALADAN3_SVP2, SOC_SBX_CALADAN3_SVP2, SOC_SBX_CALADAN3_SVP2,
    SOC_SBX_CALADAN3_SVP3, SOC_SBX_CALADAN3_SVP3, SOC_SBX_CALADAN3_SVP3, SOC_SBX_CALADAN3_SVP3,
    SOC_SBX_CALADAN3_SVP4, SOC_SBX_CALADAN3_SVP4, SOC_SBX_CALADAN3_SVP4, SOC_SBX_CALADAN3_SVP4
};


sbx_caladan3_lrp_list_cb_t  _caladan3_list_manager[SOC_MAX_NUM_DEVICES];
 
#define LRP_LIST_CB(unit) &(_caladan3_list_manager[(unit)])
#define LRP_LIST_MGR(unit, index) &((LRP_LIST_CB((unit)))->listmgr[(index)])

char *listmgr_mode2str[] = { "traditional", "host_to_ucode", "ucode_to_host", "ucode_to_ucode" };
char *listmgr_type2str[] = { "wheel", "list", "circ_buffer" };

/*
 * Helpers
 */

/* Locate the listmgr */
soc_sbx_caladan3_lrp_listmgr_t *
listmgr_find_free(int unit, soc_sbx_caladan3_lrp_svp_t svp)
{
    int i;
    soc_sbx_caladan3_lrp_listmgr_t *listmgr = NULL;

    for (i=0; i < SOC_SBX_CALADAN3_NUM_LIST_MGR; i++) {
        listmgr = LRP_LIST_MGR(unit, i);
        if ((listmgr->svp == svp) && (listmgr->inuse == 0)) {
            return listmgr;
        }
    }
    return NULL;
}

/* Adjust dequeue thresholds */
int
listmgr_deq_event_threshold_adjust(int unit, int listmgr_id)
{
    uint32 regval = 0;
    int rv = SOC_E_NONE;
    soc_sbx_caladan3_lrp_listmgr_t *listmgr;

    listmgr = LRP_LIST_MGR(unit, listmgr_id);
    READ_LRB_LIST_CONFIG1_REGr(unit, listmgr->index, &regval);
    soc_reg_field_set(unit, LRB_LIST_CONFIG1_REGr, &regval, DEQ_THRESHOLDf, listmgr->deq_threshold);
    rv = WRITE_LRB_LIST_CONFIG1_REGr(unit, listmgr->index, regval);
    return rv;
}

/* Adjust enqueue thresholds */
int
listmgr_enq_event_threshold_adjust(int unit, int listmgr_id)
{
    uint32 regval = 0;
    int rv = SOC_E_NONE;
    soc_sbx_caladan3_lrp_listmgr_t *listmgr;

    listmgr = LRP_LIST_MGR(unit, listmgr_id);
    READ_LRB_LIST_CONFIG2_REGr(unit, listmgr->index, &regval);
    soc_reg_field_set(unit, LRB_LIST_CONFIG2_REGr, &regval, ENQ_THRESHOLDf, listmgr->enq_threshold);
    rv = WRITE_LRB_LIST_CONFIG2_REGr(unit, listmgr->index, regval);
    return rv;
}

/* Enqueue Callback */
void
listmgr_enqueue_callback(int unit, int listmgr_id, int curr_offset)
{
    int num_enqueues = 0;
    soc_sbx_caladan3_lrp_listmgr_t *listmgr;
    listmgr = LRP_LIST_MGR(unit, listmgr_id);
    if (listmgr->inuse && listmgr->enqueue_func) {
        num_enqueues = curr_offset - listmgr->write_offset;
        if (num_enqueues <= 0) { 
            num_enqueues += listmgr->num_entries;
        }
        listmgr->enqueue_func(unit, listmgr->enqueue_context, listmgr->write_offset, num_enqueues);
        listmgr->write_offset = curr_offset;
        listmgr->num_enqueues += num_enqueues;
        listmgr_enq_event_threshold_adjust(unit, listmgr_id);
    }
}

/* Dequeue Callback */
void
listmgr_dequeue_callback(int unit, int listmgr_id, int curr_offset)
{
    int num_dequeues = 0;
    soc_sbx_caladan3_lrp_listmgr_t *listmgr;
    listmgr = LRP_LIST_MGR(unit, listmgr_id);
    if (listmgr->inuse && listmgr->dequeue_func) {
        num_dequeues = curr_offset - listmgr->read_offset;
        if (num_dequeues <= 0) { 
            num_dequeues += listmgr->num_entries;
        }
        listmgr->dequeue_func(unit, listmgr->dequeue_context, listmgr->read_offset, num_dequeues);
        listmgr->read_offset = curr_offset;
        listmgr->num_dequeues += num_dequeues;
        listmgr_deq_event_threshold_adjust(unit, listmgr_id);
    }
}

/* enqueue event enable */
int
listmgr_enqueue_event_enable(int unit, int listmgr_id, int enable)
{
    uint32 regval = 0;
    uint32 event = 0;
    int rv = SOC_E_NONE;
    soc_sbx_caladan3_lrp_listmgr_t *listmgr;

    listmgr = LRP_LIST_MGR(unit, listmgr_id);
    if (listmgr->inuse && listmgr->enqueue_func) {
        READ_LRB_LIST_ENQ_EVENT_MASKr(unit, &regval);
        event = soc_reg_field_get(unit, LRB_LIST_ENQ_EVENT_MASKr, regval, ENQ_INTR_DISINTf);
        if (enable) {
            event &= ~(1 << listmgr->index);
        } else {
            event |= (1 << listmgr->index);
        }
        soc_reg_field_set(unit, LRB_LIST_ENQ_EVENT_MASKr, &regval, ENQ_INTR_DISINTf, event);
        rv = WRITE_LRB_LIST_ENQ_EVENT_MASKr(unit, regval);
        if (SOC_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "Unit %d, Cannot enable enqueue event for listmgr (%d)\n"), unit, listmgr->index));
            return rv;
        }
        READ_LRB_LIST_CONFIG0_REGr(unit, listmgr->index, &regval);
        if (enable) {
            soc_reg_field_set(unit, LRB_LIST_CONFIG0_REGr, &regval, ENABLE_ENQ_INTRf, 1);
        } else {
            soc_reg_field_set(unit, LRB_LIST_CONFIG0_REGr, &regval, ENABLE_ENQ_INTRf, 0);
        }
        rv = WRITE_LRB_LIST_CONFIG0_REGr(unit, listmgr->index, regval);
        if (SOC_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "Unit %d, Cannot set listmgr (%d) intr\n"), unit, listmgr->index));
            return rv;
        }
    }
    return SOC_E_NONE;
}

/* Dqueue event enable */
int
listmgr_dequeue_event_enable(int unit, int listmgr_id, int enable)
{
    uint32 regval = 0;
    uint32 event = 0;
    int rv = SOC_E_NONE;
    soc_sbx_caladan3_lrp_listmgr_t *listmgr;

    listmgr = LRP_LIST_MGR(unit, listmgr_id);
    if (listmgr->inuse && listmgr->dequeue_func) {
        READ_LRB_LIST_DEQ_EVENT_MASKr(unit, &regval);
        event = soc_reg_field_get(unit, LRB_LIST_DEQ_EVENT_MASKr, regval, DEQ_INTR_DISINTf);
        if (enable) {
            event &= ~(1 << listmgr->index);
        } else {
            event |= (1 << listmgr->index);
        }
        soc_reg_field_set(unit, LRB_LIST_DEQ_EVENT_MASKr, &regval, DEQ_INTR_DISINTf, event);
        rv = WRITE_LRB_LIST_DEQ_EVENT_MASKr(unit, regval);
        if (SOC_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "Unit %d, Cannot enable enqueue event for listmgr (%d)\n"), unit, listmgr->index));
            return rv;
        }
        READ_LRB_LIST_CONFIG0_REGr(unit, listmgr->index, &regval);
        if (enable) {
            soc_reg_field_set(unit, LRB_LIST_CONFIG0_REGr, &regval, ENABLE_DEQ_INTRf, 1);
        } else {
            soc_reg_field_set(unit, LRB_LIST_CONFIG0_REGr, &regval, ENABLE_DEQ_INTRf, 0);
        }
        rv = WRITE_LRB_LIST_CONFIG0_REGr(unit, listmgr->index, regval);
        if (SOC_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "Unit %d, Cannot set listmgr (%d) intr\n"), unit, listmgr->index));
            return rv;
        }
    }
    return SOC_E_NONE;
}


/* End Helpers */
 

/*
 *   Function
 *     sbx_caladan3_lr_list_manager_dequeue_done
 *   Purpose
 *     Invoked on List dqueue events, invokes the callbacks if registered
 *   Parameters
 *      (IN) unit      : unit number of the device
 *   Returns
 *       SOC_E_NONE on success.
 */
int 
soc_sbx_caladan3_lr_list_manager_dequeue_done(int unit, int listmgr_id)
{
    uint32 offset = 0, regval = 0;
    READ_LRB_LIST_CONFIG3_REGr(unit, listmgr_id, &regval);
    offset = soc_reg_field_get(unit, LRB_LIST_CONFIG3_REGr, regval, READ_OFFSETf);
    listmgr_dequeue_callback(unit, listmgr_id, offset);
    regval = 0;
    READ_LRB_LIST_DEQ_EVENTr(unit, &regval);
    offset = soc_reg_field_get(unit, LRB_LIST_DEQ_EVENTr, regval, DEQ_INTRf);
    offset |= (1 << listmgr_id);
    soc_reg_field_set(unit, LRB_LIST_DEQ_EVENTr, &regval, DEQ_INTRf, offset);
    WRITE_LRB_LIST_DEQ_EVENTr(unit, regval);
    return SOC_E_NONE;
}


/*
 *   Function
 *     soc_sbx_caladan3_lr_list_manager_enqueue_done
 *   Purpose
 *     Invoked on List enqueue events, invokes the callback if registered
 *   Parameters
 *      (IN) unit      : unit number of the device
 *   Returns
 *       One of SOC_E_ error codes, SOC_E_NONE on success.
 */
int 
soc_sbx_caladan3_lr_list_manager_enqueue_done(int unit, int listmgr_id)
{
    uint32 offset = 0, regval = 0;
    READ_LRB_LIST_CONFIG4_REGr(unit, listmgr_id, &regval);
    offset = soc_reg_field_get(unit, LRB_LIST_CONFIG4_REGr, regval, WRITE_OFFSETf);
    listmgr_enqueue_callback(unit, listmgr_id, offset);
    regval = 0;
    READ_LRB_LIST_ENQ_EVENTr(unit, &regval);
    offset = soc_reg_field_get(unit, LRB_LIST_ENQ_EVENTr, regval, ENQ_INTRf);
    offset |= (1 << listmgr_id);
    soc_reg_field_set(unit, LRB_LIST_ENQ_EVENTr, &regval, ENQ_INTRf, offset);
    WRITE_LRB_LIST_ENQ_EVENTr(unit, regval);
    return SOC_E_NONE;
    
}

/*
 *   Function
 *     soc_sbx_caladan3_lr_list_init
 *   Purpose
 *      Init List manager parameters
 *   Parameters
 *      (IN) unit      : unit number of the device
 *   Returns
 *       One of SOC_E_ error codes, SOC_E_NONE on success.
 */
int 
soc_sbx_caladan3_lr_list_init(int unit)
{
    int i;
    soc_sbx_caladan3_lrp_listmgr_t *listmgr;

    for (i=0; i < SOC_SBX_CALADAN3_NUM_LIST_MGR; i++) {
        listmgr = LRP_LIST_MGR(unit, i);
        listmgr->svp = mgr_svp_map[i];
        listmgr->inuse = 0;
        listmgr->index = i;
        listmgr->num_entries = 0;
        listmgr->entry_size = 0;
        listmgr->read_offset = 0;
        listmgr->write_offset = 0;
        listmgr->enqueue_func = NULL;
        listmgr->dequeue_func = NULL;
        listmgr->enqueue_context = NULL;
        listmgr->dequeue_context = NULL;
        sal_memset(&(listmgr->allocid), 0, sizeof(sbx_caladan3_ocm_port_alloc_t));
        listmgr->base_offset = 1;
        listmgr->max_offset = 0;
        listmgr->cur_w_offset = 1; /* Index 0 is not used */
        listmgr->cur_r_offset = 1; /* Index 0 is not used */
        listmgr->type = SOC_SBX_CALADAN3_LRP_LIST_TYPE_WHEEL;
        listmgr->num_enqueues = 0;
        listmgr->num_dequeues = 0;
    }
    /* Enable the interrupts */
    soc_sbx_caladan3_lrb_isr_enable(unit);
    return SOC_E_NONE;
}

/*
 *   Function
 *     soc_sbx_caladan3_lr_list_memory_alloc
 *   Purpose
 *      Setup List memory in the given OCM Port and Segment
 *   Parameters
 *      (IN) unit       : unit number of the device
 *      (IN) port       : OCM memory Port, could be any ocm port not necessary the svp ports
 *      (IN) segment    : Segment for allocating memory
 *      (IN) entry_size : Size of each list item
 *      (IN) num_entries: Number of items in list
 *   Returns
 *       One of SOC_E_ error codes, SOC_E_NONE on success.
 */
int 
soc_sbx_caladan3_lr_list_memory_alloc(int unit,
                                      int listmgr_id,
                                      int port, 
                                      int segment,
                                      int num_entries,
                                      int entry_size)
{
    int rv;
    soc_sbx_caladan3_lrp_listmgr_t *listmgr;

    if ((listmgr_id >= SOC_SBX_CALADAN3_NUM_LIST_MGR) ||
        (listmgr_id < 0)) {
        return SOC_E_PARAM;
    }

    listmgr = LRP_LIST_MGR(unit, listmgr_id);

    if (!listmgr->inuse) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "Unit %d list manager is not enabled \n"), unit));
        return SOC_E_PARAM;
    }
    if (listmgr->allocid.size) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "Unit %d memory for list is already allocated \n"), unit));
        return SOC_E_PARAM;
    }
    sal_memset(&listmgr->allocid, 0, sizeof(sbx_caladan3_ocm_port_alloc_t));
    listmgr->allocid.port = port;
    listmgr->allocid.segment = segment;
    listmgr->allocid.datum_size = entry_size;
    listmgr->allocid.size = entry_size * num_entries;
    if (listmgr->type == SOC_SBX_CALADAN3_LRP_LIST_TYPE_WHEEL) {
        listmgr->base_offset = 0;
    } else {
        listmgr->base_offset = 1;
    }
    listmgr->max_offset = num_entries;

    rv = soc_sbx_caladan3_ocm_port_segment_alloc(unit, &listmgr->allocid);
    if (SOC_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "Unit %d Failed allocating memory for list port %d segment %d\n"),
                   unit, port, segment));
        return rv;
    }
    return SOC_E_NONE;
}

/*
 *   Function
 *     soc_sbx_caladan3_lr_list_memory_free
 *   Purpose
 *      Free List memory in the given OCM Port and Segment
 *   Parameters
 *      (IN) unit      : unit number of the device
 *      (IN) port      : OCM memory Port, could be any ocm port not necessary the svp ports
 *      (IN) segment   : Segment for allocating memory
 *      (IN) size      : Size of each list item
 *      (IN) nitems    : Number of items in list
 *   Returns
 *       One of SOC_E_ error codes, SOC_E_NONE on success.
 */
int 
soc_sbx_caladan3_lr_list_memory_free(int unit, int listmgr_id)
{
    int rv = SOC_E_PARAM;
    soc_sbx_caladan3_lrp_listmgr_t *listmgr;

    if ((listmgr_id >= SOC_SBX_CALADAN3_NUM_LIST_MGR) ||
        (listmgr_id < 0)) {
        return SOC_E_PARAM;
    }

    listmgr = LRP_LIST_MGR(unit, listmgr_id);

    if (!listmgr->inuse) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "Unit %d list manager is not enabled \n"), unit));
        return SOC_E_PARAM;
    }
    if (listmgr->allocid.size) {
        rv = soc_sbx_caladan3_ocm_port_segment_free(unit, &listmgr->allocid);
        if (SOC_SUCCESS(rv)) {
            sal_memset(&listmgr->allocid, 0, sizeof(sbx_caladan3_ocm_port_alloc_t));
            if (listmgr->type == SOC_SBX_CALADAN3_LRP_LIST_TYPE_WHEEL) {
                listmgr->base_offset = 0;
            } else {
                listmgr->base_offset = 1;
            }
            listmgr->max_offset = 0;
            listmgr->cur_r_offset = 1; /* index 0 not used */
            listmgr->cur_w_offset = 1; /* index 0 not used */
        }
    } else {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "Unit %d memory not allocated for listmgr(%d)\n"),
                   unit, listmgr_id));
    }
    return rv;
}

/*
 *   Function
 *     soc_sbx_caladan3_lr_list_manager_init
 *   Purpose
 *      initializes LRP SVP List Manager for Wheel configuration
 *   Parameters
 *      (IN) unit         : unit number of the device
 *      (IN) svp          : service processor 
 *      (IN) mode         : list mode of operation
 *      (IN) type         : list or wheel
 *      (IN) entry_size   : size of each entry(soc_sbx_caladan3_lrp_list_esize_e_t)
 *      (IN) num_entries  : total number of entries in the list
 *                          index 0 is not usable in wheel mode, so total number is 1 less
 *      (IN) enqueue_threshold : default to 1, can be set to any positive number < num_entries
 *      (IN) dqueue_threshold  : default to 1, can be set to any positive number < num_entries
 *                             : use value of 0 if not interested in interrupts
 *      (IN) listmgr_id   : if allocation is requested use value -1,
 *                          If reinit is requested, any valid id
 *      (IN) reinit       : Reinit the listmgr 
 *   Returns
 *       SOC_E_NONE    - success
 *       SOC_E_TIMEOUT - command timed out
 */
int 
soc_sbx_caladan3_lr_list_manager_init(int unit, 
                                      soc_sbx_caladan3_lrp_svp_t svp,
                                      soc_sbx_caladan3_lrp_list_mode_e_t mode,
                                      soc_sbx_caladan3_lrp_list_type_e_t type,
                                      soc_sbx_caladan3_lrp_list_esize_e_t entry_size,
                                      uint32 num_entries,
                                      int enqueue_threshold,
                                      int dqueue_threshold,
                                      int *listmgr_id,
                                      int reinit)
{
    int    rv = SOC_E_NONE;
    uint32 regval = 0;
    soc_sbx_caladan3_lrp_listmgr_t *listmgr;

    if ((!listmgr_id) || (*listmgr_id >= SOC_SBX_CALADAN3_NUM_LIST_MGR)) {
        return SOC_E_PARAM;
    }
    if (*listmgr_id < 0) {
        listmgr = listmgr_find_free(unit, svp);
        if (!listmgr) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "Unit %d, Cannot find free Listmgr for svp (%d)\n"), unit, svp));
            return SOC_E_PARAM;
        }
    } else {
        listmgr = LRP_LIST_MGR(unit, *listmgr_id);
        if (listmgr->svp != svp) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "Unit %d, Listmgr (%d) does not use SVP %d\n"), unit, listmgr->index, svp));
            return SOC_E_PARAM;
        }
        if (listmgr->inuse && !reinit) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "Unit %d, Listmgr (%d) already in use \n"), unit, listmgr->index));
            return SOC_E_PARAM;
        }
        if (listmgr->allocid.size) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "Unit %d, Listmgr (%d) still using ocm memory, cannot reinit\n"),
                       unit, listmgr->index));
            return SOC_E_PARAM;
        }
    }
    if (((mode == SOC_SBX_CALADAN3_LRP_LIST_MODE_TRADITIONAL) &&
             ((type != SOC_SBX_CALADAN3_LRP_LIST_TYPE_WHEEL) && (type != SOC_SBX_CALADAN3_LRP_LIST_TYPE_LIST))) ||
        ((mode != SOC_SBX_CALADAN3_LRP_LIST_MODE_TRADITIONAL) &&
              ((type == SOC_SBX_CALADAN3_LRP_LIST_TYPE_WHEEL) || (type == SOC_SBX_CALADAN3_LRP_LIST_TYPE_LIST))))
    {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "Unit %d, Listmgr (%d) incompatible mode(%s) and type (%s)\n"),
                       unit, listmgr->index, listmgr_mode2str[mode], listmgr_type2str[type]));
            return SOC_E_PARAM;
    }
         
    if (reinit) {
        regval = 0;
        soc_reg_field_set(unit, LRB_LIST_CONFIG0_REGr, &regval, ENABLE_UCODE_DEQf, 0);
        soc_reg_field_set(unit, LRB_LIST_CONFIG0_REGr, &regval, ENABLE_UCODE_ENQf, 0);
        soc_reg_field_set(unit, LRB_LIST_CONFIG0_REGr, &regval, ENABLEf, 0);
        rv = WRITE_LRB_LIST_CONFIG0_REGr(unit, listmgr->index, regval);
        if (SOC_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "Unit %d, Cannot reset listmgr (%d)\n"), unit, listmgr->index));
            return rv;
        }
    }

    regval = 0;
    listmgr->read_offset = 1;
    soc_reg_field_set(unit, LRB_LIST_CONFIG3_REGr, &regval, READ_OFFSETf, 1);
    rv = WRITE_LRB_LIST_CONFIG3_REGr(unit, listmgr->index, regval);
    if (SOC_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "Unit %d, Cannot set READOFFSET for listmgr (%d)\n"), unit, listmgr->index));
        return rv;
    }
  
    regval = 0;
    listmgr->type = type;
    listmgr->mode = mode;
    if ((enqueue_threshold < 0) || (enqueue_threshold > num_entries)) {
        listmgr->enq_threshold = 1;
    } else {
        listmgr->enq_threshold = enqueue_threshold;
    }
    if ((dqueue_threshold < 0) || (dqueue_threshold > num_entries)) {
        listmgr->deq_threshold = 1;
    } else {
        listmgr->deq_threshold = dqueue_threshold;
    }
    switch (mode) {
    case SOC_SBX_CALADAN3_LRP_LIST_MODE_TRADITIONAL:
        if (type == SOC_SBX_CALADAN3_LRP_LIST_TYPE_WHEEL) {
            listmgr->write_offset = 0;
            soc_reg_field_set(unit, LRB_LIST_CONFIG4_REGr, &regval, WRITE_OFFSETf, 0);
        } else {
            /* type is SOC_SBX_CALADAN3_LRP_LIST_TYPE_LIST */
            listmgr->write_offset = num_entries;
            soc_reg_field_set(unit, LRB_LIST_CONFIG4_REGr, &regval, WRITE_OFFSETf, listmgr->write_offset);
        }
        break;
    default:
        /* bidir mode */
        listmgr->write_offset = 1;
        soc_reg_field_set(unit, LRB_LIST_CONFIG4_REGr, &regval, WRITE_OFFSETf, 1);
    }
    rv = WRITE_LRB_LIST_CONFIG4_REGr(unit, listmgr->index, regval);
    if (SOC_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "Unit %d, Cannot set WRITEOFFSET for listmgr (%d)\n"), unit, listmgr->index));
        return rv;
    }

    switch (mode) {
    case SOC_SBX_CALADAN3_LRP_LIST_MODE_TRADITIONAL:
        regval = 0;
        soc_reg_field_set(unit, LRB_LIST_CONFIG1_REGr, &regval, DEQ_THRESHOLDf, listmgr->deq_threshold);
        rv = WRITE_LRB_LIST_CONFIG1_REGr(unit, listmgr->index, regval);
        if (SOC_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "Unit %d, Cannot set DEQ Threshold on listmgr (%d)\n"), unit, listmgr->index));
            return rv;
        }
        break;
    default:
        /* bidir mode */
        if ((mode == SOC_SBX_CALADAN3_LRP_LIST_MODE_HOST_TO_UCODE) ||
            (mode == SOC_SBX_CALADAN3_LRP_LIST_MODE_UCODE_TO_UCODE)) {
            /* Ucode is the consumer */
            regval = 0;
            soc_reg_field_set(unit, LRB_LIST_CONFIG1_REGr, &regval, DEQ_THRESHOLDf, listmgr->deq_threshold);
            rv = WRITE_LRB_LIST_CONFIG1_REGr(unit, listmgr->index, regval);
            if (SOC_FAILURE(rv)) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "Unit %d, Cannot set DEQ Threshold on listmgr (%d)\n"), unit, listmgr->index));
                return rv;
            }
        }
        if ((mode == SOC_SBX_CALADAN3_LRP_LIST_MODE_UCODE_TO_HOST) ||
            (mode == SOC_SBX_CALADAN3_LRP_LIST_MODE_UCODE_TO_UCODE)) {
            /* Ucode is the producer */
            regval = 0;
            soc_reg_field_set(unit, LRB_LIST_CONFIG2_REGr, &regval, ENQ_THRESHOLDf, listmgr->enq_threshold);
            rv = WRITE_LRB_LIST_CONFIG2_REGr(unit, listmgr->index, regval);
            if (SOC_FAILURE(rv)) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "Unit %d, Cannot set ENQ Threshold on listmgr (%d)\n"), unit, listmgr->index));
                return rv;
            }
        }
    }
 
    switch (mode) {
    case SOC_SBX_CALADAN3_LRP_LIST_MODE_TRADITIONAL:
        soc_reg_field_set(unit, LRB_LIST_CONFIG0_REGr, &regval, ENABLE_UCODE_DEQf, 1);
        soc_reg_field_set(unit, LRB_LIST_CONFIG0_REGr, &regval, ENABLE_UCODE_ENQf, 0);
        break;
    default:
        /* bidir mode */
        regval = 0;
        if ((mode == SOC_SBX_CALADAN3_LRP_LIST_MODE_HOST_TO_UCODE) ||
            (mode == SOC_SBX_CALADAN3_LRP_LIST_MODE_UCODE_TO_UCODE)) {
            /* Ucode is the consumer */
            soc_reg_field_set(unit, LRB_LIST_CONFIG0_REGr, &regval, ENABLE_UCODE_DEQf, 1);
        }
        if ((mode == SOC_SBX_CALADAN3_LRP_LIST_MODE_UCODE_TO_HOST) ||
            (mode == SOC_SBX_CALADAN3_LRP_LIST_MODE_UCODE_TO_UCODE)) {
            /* Ucode is the Producer */
            soc_reg_field_set(unit, LRB_LIST_CONFIG0_REGr, &regval, ENABLE_UCODE_ENQf, 1);
        }
    }
    listmgr->entry_size = entry_size;
    soc_reg_field_set(unit, LRB_LIST_CONFIG0_REGr, &regval, SIZEf, listmgr->entry_size);
    listmgr->num_entries = num_entries;
    soc_reg_field_set(unit, LRB_LIST_CONFIG0_REGr, &regval, ENTRIESf, listmgr->num_entries);
    soc_reg_field_set(unit, LRB_LIST_CONFIG0_REGr, &regval, ENABLEf, 1);
    rv = WRITE_LRB_LIST_CONFIG0_REGr(unit, listmgr->index, regval);

    if (SOC_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "Unit %d, Cannot enable listmgr (%d)\n"), unit, listmgr->index));
        return rv;
    }

    listmgr->inuse = 1;
    *listmgr_id = listmgr->index;

    LOG_VERBOSE(BSL_LS_SOC_COMMON,
                (BSL_META_U(unit,
                            "Unit %d, list manager %d initialized as %s in %s mode \n"),
                 unit, listmgr->index, listmgr_type2str[type], 
                 listmgr_mode2str[mode]));
                               
    return rv;
}

/*
 *   Function
 *     soc_sbx_caladan3_lr_list_manager_enqueue_event_enable
 *   Purpose
 *      Enable/Disable Enqueue Event
 *   Parameters
 *      (IN) unit       : unit number of the device
 *      (IN) listmgr_id : list manager index
 *      (IN) enable      : enable = 1, disable = 0
 *   Returns
 *       SOC_E_NONE    - success
 */
int 
soc_sbx_caladan3_lr_list_manager_enqueue_event_enable(int unit, 
                                                  int listmgr_id,
                                                  int enable)
{
    return (listmgr_enqueue_event_enable(unit, listmgr_id, enable));
}
/*
 *   Function
 *     soc_sbx_caladan3_lr_list_manager_dequeue_event_enable
 *   Purpose
 *      Enable or disable dqueue envent
 *   Parameters
 *      (IN) unit        : unit number of the device
 *      (IN) listmgr_id  : list manager index
 *      (IN) enable      : enable = 1, disable = 0
 *   Returns
 *       SOC_E_NONE    - success
 */
int 
soc_sbx_caladan3_lr_list_manager_dequeue_event_enable(int unit, 
                                                  int listmgr_id,
                                                  int enable)
{
    return (listmgr_dequeue_event_enable(unit, listmgr_id, enable));
}

/*
 *   Function
 *     soc_sbx_caladan3_lr_list_manager_register_callback
 *   Purpose
 *      Register a callback for enqueue or dqueue notifications
 *   Parameters
 *      (IN) unit      : unit number of the device
 *      (IN) listmgr_id  : list manager index
 *      (IN) enqueue_func: Callback on enqueue 
 *      (IN) dequeue_func: Callback on dqueue 
 *   Returns
 *       SOC_E_NONE    - success
 */
int 
soc_sbx_caladan3_lr_list_manager_register_callback(int unit, 
                                               int listmgr_id,
                                               soc_sbx_caladan3_lrp_list_enqueue_cb_f enq_func,
                                               void *enq_context,
                                               soc_sbx_caladan3_lrp_list_dequeue_cb_f deq_func,
                                               void *deq_context)
{
    soc_sbx_caladan3_lrp_listmgr_t *listmgr;

    if ((listmgr_id >= SOC_SBX_CALADAN3_NUM_LIST_MGR) ||
        (listmgr_id < 0)) {
        return SOC_E_PARAM;
    }
    listmgr = LRP_LIST_MGR(unit, listmgr_id);
    if (!listmgr->inuse) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "Unit %d, listmgr (%d) not in use\n"), unit, listmgr->index));
        return SOC_E_PARAM;
    }
    if (listmgr->type == SOC_SBX_CALADAN3_LRP_LIST_TYPE_WHEEL) {
        LOG_WARN(BSL_LS_SOC_COMMON,
                 (BSL_META_U(unit,
                             "Unit %d, listmgr (%d) is %s type, no callbacks are generated\n"),
                  unit, listmgr->index, listmgr_type2str[listmgr->type]));
    }
    listmgr->enqueue_func = enq_func;
    listmgr->dequeue_func = deq_func;
    listmgr->enqueue_context = enq_context;
    listmgr->dequeue_context = deq_context;
    if (enq_func) {
        listmgr_enqueue_event_enable(unit, listmgr_id, 1);
    } else {
        listmgr_enqueue_event_enable(unit, listmgr_id, 0);
    }
    if (deq_func) {
        listmgr_dequeue_event_enable(unit, listmgr_id, 1);
    } else {
        listmgr_dequeue_event_enable(unit, listmgr_id, 0);
    }
    return SOC_E_NONE;
}

/*
 *   Function
 *     soc_sbx_caladan3_lr_list_manager_set_enqueue_offset
 *   Purpose
 *      Host Enqueue operation, update the enqueue offset. No callback invoked
 *   Parameters
 *      (IN) unit       : unit number of the device
 *      (IN) listmgr_id : list manager index
 *      (IN) offset     : update write_offset to this value
 *   Returns
 *       SOC_E_NONE    - success
 */
int
soc_sbx_caladan3_lr_list_manager_set_enqueue_offset(int unit, int listmgr_id, int offset) 
{
    uint32 regval = 0;
    soc_sbx_caladan3_lrp_listmgr_t *listmgr;

    if ((listmgr_id >= SOC_SBX_CALADAN3_NUM_LIST_MGR) ||
        (listmgr_id < 0)) {
        return SOC_E_PARAM;
    }
    listmgr = LRP_LIST_MGR(unit, listmgr_id);
    if (!listmgr->inuse) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "Unit %d, listmgr (%d) not in use\n"), unit, listmgr->index));
        return SOC_E_PARAM;
    }
    READ_LRB_LIST_CONFIG4_REGr(unit, listmgr_id, &regval);
    soc_reg_field_set(unit, LRB_LIST_CONFIG4_REGr, &regval, WRITE_OFFSETf, offset);
    WRITE_LRB_LIST_CONFIG4_REGr(unit, listmgr_id, regval);
    listmgr->write_offset = offset;
    return SOC_E_NONE;
}


/*
 *   Function
 *     soc_sbx_caladan3_lr_list_manager_enqueue_item
 *   Purpose
 *      Host enqueue operation, invokes the enqueue call back
 *   Parameters
 *      (IN) unit       : unit number of the device
 *      (IN) listmgr_id : list manager index
 *      (IN) data       : data to enqueue, used only when this driver manages memory
 *   Returns
 *       SOC_E_NONE    - success
 */
int 
soc_sbx_caladan3_lr_list_manager_enqueue_item(int unit, 
                                          int listmgr_id,
                                          unsigned char *data,
                                          int length)
{
    uint32 offset = 0, regval = 0;
    int ocm_index_max = 0, num_index_wrap = 0;
    int num_ocm_entries = 0, num_list_entries = 0, num_list_avail = 0;
    soc_sbx_caladan3_lrp_listmgr_t *listmgr;
    int rv = SOC_E_NONE, width=0;

    if ((listmgr_id >= SOC_SBX_CALADAN3_NUM_LIST_MGR) ||
        (listmgr_id < 0)) {
        return SOC_E_PARAM;
    }
    listmgr = LRP_LIST_MGR(unit, listmgr_id);
    if (!listmgr->inuse) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "Unit %d, listmgr (%d) not in use\n"), unit, listmgr->index));
        return SOC_E_PARAM;
    }
    /* Check space availability */
    width = listmgr->allocid.datum_size >> 3;
    num_list_entries = length / (1 << listmgr->entry_size);
    if (listmgr->allocid.size) {
        num_list_entries = (length + width) / width;
        num_list_entries = (num_list_entries / (1 << listmgr->entry_size));
    }
    READ_LRB_LIST_CONFIG3_REGr(unit, listmgr_id, &regval);
    offset = soc_reg_field_get(unit, LRB_LIST_CONFIG3_REGr, regval, READ_OFFSETf);
    num_list_avail = listmgr->write_offset - offset;
    if (num_list_avail == listmgr->num_entries) {
        return SOC_E_FULL;
    }
    if (num_list_avail <= 0) {
        num_list_avail += listmgr->num_entries;
    }
    /* Stop if we will overflow */
    if (num_list_entries > num_list_avail) {
        return SOC_E_RESOURCE;
    }

    if (listmgr->allocid.size) {
        if (data) {
            num_ocm_entries = (length + width) / width;
            if (num_ocm_entries > 0) {
                ocm_index_max = listmgr->cur_w_offset + num_ocm_entries;
                if (ocm_index_max > listmgr->max_offset) {
                    num_index_wrap = ocm_index_max - listmgr->max_offset;
                    ocm_index_max = listmgr->max_offset;
                }
                rv = soc_sbx_caladan3_ocm_port_mem_write(unit, listmgr->allocid.port,
                                                         listmgr->allocid.segment,
                                                         listmgr->cur_w_offset,
                                                         ocm_index_max, (uint32*)data);
                if (SOC_FAILURE(rv)) {
                    LOG_ERROR(BSL_LS_SOC_COMMON,
                              (BSL_META_U(unit,
                                          "Unit %d, listmgr (%d) Enqueue data failed\n"), unit, listmgr->index));
                    return rv;
                }
                if (num_index_wrap > 0) {
                    data += (ocm_index_max - listmgr->cur_w_offset + 1) * width;
                    rv = soc_sbx_caladan3_ocm_port_mem_write(unit, listmgr->allocid.port,
                                                             listmgr->allocid.segment,
                                                             1, num_index_wrap + 1, (uint32*)data);
                    if (SOC_FAILURE(rv)) {
                        LOG_ERROR(BSL_LS_SOC_COMMON,
                                  (BSL_META_U(unit,
                                              "Unit %d, listmgr (%d) Enqueue wrapped data failed\n"), unit, listmgr->index));
                        return rv;
                    }
                }
                listmgr->cur_w_offset = (listmgr->cur_w_offset + num_ocm_entries) % listmgr->max_offset;
            }
        } else {
            LOG_WARN(BSL_LS_SOC_COMMON,
                     (BSL_META_U(unit,
                                 "Unit %d, listmgr (%d) manages memory, but data not provided\n"), 
                      unit, listmgr->index));
        }
    } else {
        if (data) {
            LOG_WARN(BSL_LS_SOC_COMMON,
                     (BSL_META_U(unit,
                                 "Unit %d, listmgr (%d) doesnt manage memory, but data provided\n"), 
                      unit, listmgr->index));
        }
    }
    READ_LRB_LIST_CONFIG4_REGr(unit, listmgr_id, &regval);
    offset = soc_reg_field_get(unit, LRB_LIST_CONFIG4_REGr, regval, WRITE_OFFSETf);
    offset += num_list_entries;

    if (offset > listmgr->num_entries) { offset += 1; /* account for jump over offset 0 */ }
    offset %= (listmgr->num_entries + 1); /* Wrap around */
    if (offset == 0) { offset = 1; /* jump over offset 0 */ }
    soc_reg_field_set(unit, LRB_LIST_CONFIG4_REGr, &regval, WRITE_OFFSETf, offset);
    WRITE_LRB_LIST_CONFIG4_REGr(unit, listmgr_id, regval);

    listmgr_enqueue_callback(unit, listmgr_id, offset);
    listmgr->write_offset = offset;
    return SOC_E_NONE;
}

/*
 *   Function
 *     soc_sbx_caladan3_lr_list_manager_set_dequeue_offset
 *   Purpose
 *      Host Dequeue operation, update the dqueue offset. No callback invoked
 *   Parameters
 *      (IN) unit       : unit number of the device
 *      (IN) listmgr_id : list manager index
 *      (IN) offset     : update read_offset to this value
 *   Returns
 *       SOC_E_NONE    - success
 */
int
soc_sbx_caladan3_lr_list_manager_set_dequeue_offset(int unit, int listmgr_id, int offset) 
{
    uint32 regval = 0;
    soc_sbx_caladan3_lrp_listmgr_t *listmgr;

    if ((listmgr_id >= SOC_SBX_CALADAN3_NUM_LIST_MGR) ||
        (listmgr_id < 0)) {
        return SOC_E_PARAM;
    }
    listmgr = LRP_LIST_MGR(unit, listmgr_id);
    if (!listmgr->inuse) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "Unit %d, listmgr (%d) not in use\n"), unit, listmgr->index));
        return SOC_E_PARAM;
    }
    READ_LRB_LIST_CONFIG3_REGr(unit, listmgr_id, &regval);
    soc_reg_field_set(unit, LRB_LIST_CONFIG3_REGr, &regval, READ_OFFSETf, offset);
    WRITE_LRB_LIST_CONFIG3_REGr(unit, listmgr_id, regval);
    listmgr->read_offset = offset;
    return SOC_E_NONE;
}

/*
 *   Function
 *     soc_sbx_caladan3_lr_list_manager_dequeue_item
 *   Purpose
 *      Host Dequeue operation, invokes the dequeue call back
 *   Parameters
 *      (IN) unit       : unit number of the device
 *      (IN) listmgr_id : list manager index
 *      (OUT) data       : data ptr to receive the dqueued item, used only when driver manages memory
 *      (IN/OUT) length  : data len of received item, used only when driver manages memory
 *                       : when called provide max size of 'data' buffer
 *   Returns
 *       SOC_E_NONE    - success
 */
int 
soc_sbx_caladan3_lr_list_manager_dequeue_item(int unit, 
                                          int listmgr_id,
                                          unsigned char *data,
                                          int *length)
{
    uint32 offset = 0, regval = 0;
    soc_sbx_caladan3_lrp_listmgr_t *listmgr;
    int ocm_index_max = 0, num_index_wrap = 0;
    int num_ocm_entries = 0, num_list_entries = 0, num_list_avail = 0;
    int width = 0, rv = SOC_E_NONE;

    if ((listmgr_id >= SOC_SBX_CALADAN3_NUM_LIST_MGR) ||
        (listmgr_id < 0)) {
        return SOC_E_PARAM;
    }
    listmgr = LRP_LIST_MGR(unit, listmgr_id);
    if (!listmgr->inuse) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "Unit %d, listmgr (%d) not in use\n"), unit, listmgr->index));
        return SOC_E_PARAM;
    }
    if (!length) {
        return SOC_E_PARAM;
    }
    if (*length <= 0) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "Unit %d, listmgr (%d) data buffer sized zero \n"), unit, listmgr->index));
        return SOC_E_PARAM;
    }

    /* Check the availablity of data */
    num_list_entries = *length/(1 << listmgr->entry_size);
    width = listmgr->allocid.datum_size >> 3;
    if (listmgr->allocid.size) {
        num_list_entries = ((*length + width) / width);
        num_list_entries = (num_list_entries / (1 << listmgr->entry_size));
    }
    READ_LRB_LIST_CONFIG4_REGr(unit, listmgr_id, &regval);
    offset = soc_reg_field_get(unit, LRB_LIST_CONFIG4_REGr, regval, WRITE_OFFSETf);
    num_list_avail = offset - listmgr->read_offset;
    if (num_list_avail < 0) {
        num_list_avail += listmgr->num_entries - 1;
    }
    if (num_list_avail == 0) {
        return SOC_E_EMPTY;
    }
    /* Read least of available memory or elements */
    if (num_list_entries > num_list_avail) {
        num_list_entries = num_list_avail;
    }
    if (listmgr->allocid.size) {
        if (!data) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "Unit %d, listmgr (%d) manages memory, memory for item not provided\n"), 
                       unit, listmgr->index));
            return SOC_E_PARAM;
        } else {
            if (*length < width) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "Unit %d, listmgr (%d) data buffer not sized for even 1 element \n"), 
                           unit, listmgr->index));
                return SOC_E_PARAM;
            }
            num_ocm_entries = *length / width;
            if (num_ocm_entries > num_list_avail) {
                num_ocm_entries = num_list_avail;
            }
            if (num_ocm_entries > listmgr->max_offset) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "Unit %d, listmgr (%d) No of entries to read exceeds memory size\n"), 
                           unit, listmgr->index));
                return SOC_E_PARAM;
            }
            ocm_index_max = listmgr->cur_r_offset + num_ocm_entries;
            if (ocm_index_max > listmgr->max_offset) {
                num_index_wrap = ocm_index_max - listmgr->max_offset;
                ocm_index_max = listmgr->max_offset;
            }
            rv = soc_sbx_caladan3_ocm_port_mem_read(unit, listmgr->allocid.port,
                                                     listmgr->allocid.segment,
                                                     listmgr->cur_r_offset,
                                                     ocm_index_max, (uint32*)data);
            if (SOC_FAILURE(rv)) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "Unit %d, listmgr (%d) Enqueue data failed\n"), unit, listmgr->index));
                return rv;
            }
            if (num_index_wrap) {
                /* Read wrapped data */
                data +=  (ocm_index_max - listmgr->cur_r_offset + 1) * width;
                rv = soc_sbx_caladan3_ocm_port_mem_read(unit, listmgr->allocid.port,
                                                         listmgr->allocid.segment,
                                                         1, num_index_wrap + 1, (uint32*)data);
                if (SOC_FAILURE(rv)) {
                    LOG_ERROR(BSL_LS_SOC_COMMON,
                              (BSL_META_U(unit,
                                          "Unit %d, listmgr (%d) Enqueue wrapped data failed\n"), unit, listmgr->index));
                    return rv;
                }
            }
            listmgr->cur_r_offset = (listmgr->cur_r_offset + num_ocm_entries) % listmgr->max_offset;
        }
    }
    READ_LRB_LIST_CONFIG3_REGr(unit, listmgr_id, &regval);
    offset = soc_reg_field_get(unit, LRB_LIST_CONFIG3_REGr, regval, READ_OFFSETf);
    offset += num_list_entries;

    if (offset > listmgr->num_entries) { offset += 1; /* account for jump over offset 0 */ }
    offset %= (listmgr->num_entries + 1); /* Wrap around */
    if (offset == 0) { offset = 1; /* jump over offset 0 */ }
    soc_reg_field_set(unit, LRB_LIST_CONFIG3_REGr, &regval, READ_OFFSETf, offset);
    WRITE_LRB_LIST_CONFIG3_REGr(unit, listmgr_id, regval);

    listmgr_dequeue_callback(unit, listmgr_id, offset);
    listmgr->read_offset = offset;
    return SOC_E_NONE;
}

#endif
