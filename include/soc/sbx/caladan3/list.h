/*
 * $Id: $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * list.h : LRP List manager
 */

#ifndef _SBX_CALADAN3_LRP_LIST_H
#define _SBX_CALADAN3_LRP_LIST_H

#include <soc/sbx/caladan3/ocm.h>

/*
 *  LRP List Manager
 *       LRP has 4 Service processors. Each SVP can support 4 lists.
 *       A special SVP for debug operation is also provided, with 4 Lists.
 *
 *       The list manager does not maintain memory, it is the responsibility of the ucode application itself.
 *       List memory may exist on the OCM/DDR via TMU
 *
 *       This driver integrates limited OCM read/write operations for host based enqueue/dqueue
 *       Please see the access routines on how to use this feature
 *
 *  Types of operation:
 *      There are 3 types of lists, C1/C2 compatible List, C1/C2 compatible Wheel, Bidirectional list
 *
 *  Mode of operation:
 *      There are 4 modes of operation, Traditional for C1/C2 modes
 *      Host Producer Ucode Consumer, Ucode Producer Ucode consumer, Ucode producer Host consumer
 *      
 *
 *  Traditional C1 and C2 compatible Model
 *      Wheel: Setup the list, no enqueue required. Dqueues roll from 1 to num_entries
 *             in list.
 *      List:  One shot usage, enqueue n items, dqueue n items. 'n' is num_entries - 1
 *             index 0 is returned when list is empty.
 *
 *  Bidirectional Model
 *      Host Producer, Ucode Consumer
 *          The Host must create a list, update the data entries
 *          Then call this driver to update the num of data enqueued
 *          A call back routine is provided to notify the host application when list has been processed.
 *
 *      Ucode Producer, Ucode Consumer
 *          The Host must create a list
 *          Host can use the callback to get notification when enqueue or dqueue operations occur
 *
 *      Ucode Producer, Host Consumer
 *          The Host must create a list
 *          A call back routine is provided to notify the host application when items are enqueued
 * 
 *  Indexing note:
 *      Dequeue operation yeilds an "offset" to the ucode application. This offset is (1 << entry_size)
 *  So using entry_size 0, yeilds sequential offset with increment of 1. 
 *  Using entry_size = 0xf, yeilds offset in increments of 0x8000
 *
 *      Offset 0 is mostly used as invalid offset. So the first offset returned to ucode is 1.
 *  Ucode application should be aware of this, If ucode needs to use offset 0, then it needs to use a base address
 *  that accounts for this, ie, ucode_base_address = actual_base_address - ( 1 << entry_size)
 *
 *      If the ucode is also using offset 0 as invalid offset, then the memory allocated should account for
 *  range of offsets returned from 1 to num_entries. For eg, list with entry_size=0, num_entries=10, will get
 *  offsets from 1-10.
 *
 *      In wheel mode operation, ucode dqueue is the only operation to perform. This returns never ending sequence
 *  of offsets. Offset 0 is never returned.
 *
 *      In one-shot list mode of operation the number of valid entries is 1 less than the num_entries. 
 *  For eg, a list with 10 entries and entry_size=0, will have offset from 1-9. 
 *  Subsequent dqueues will return 0, indicating that the list is exhausted.
 *  This list will have to be setup again
 *
 */


typedef enum soc_sbx_caladan3_lrp_list_type_e_s {
    SOC_SBX_CALADAN3_LRP_LIST_TYPE_WHEEL = 0,    /* c1/c2 compatible for ucode wheel   */
    SOC_SBX_CALADAN3_LRP_LIST_TYPE_LIST = 1,     /* c1/c2 compatible for one shot list  */
    SOC_SBX_CALADAN3_LRP_LIST_TYPE_BIDIR = 2,    /* circular buffer with prod-cons paradigm */
    SOC_SBX_CALADAN3_LRP_LIST_TYPE_MAX
} soc_sbx_caladan3_lrp_list_type_e_t;

typedef enum soc_sbx_caladan3_lrp_list_mode_e_s {
    SOC_SBX_CALADAN3_LRP_LIST_MODE_TRADITIONAL,    /* C1/C2 compatiblity */
    SOC_SBX_CALADAN3_LRP_LIST_MODE_HOST_TO_UCODE,
    SOC_SBX_CALADAN3_LRP_LIST_MODE_UCODE_TO_HOST,
    SOC_SBX_CALADAN3_LRP_LIST_MODE_UCODE_TO_UCODE,
    SOC_SBX_CALADAN3_LRP_LIST_MODE_MAX
} soc_sbx_caladan3_lrp_list_mode_e_t;

typedef enum {
    SOC_SBX_CALADAN3_SVP0 = SOC_SBX_CALADAN3_OCM_LRP0_PORT,
    SOC_SBX_CALADAN3_SVP1 = SOC_SBX_CALADAN3_OCM_LRP1_PORT,
    SOC_SBX_CALADAN3_SVP2 = SOC_SBX_CALADAN3_OCM_LRP2_PORT,
    SOC_SBX_CALADAN3_SVP3 = SOC_SBX_CALADAN3_OCM_LRP3_PORT,
    SOC_SBX_CALADAN3_SVP4 = SOC_SBX_CALADAN3_OCM_LRP9_PORT
} soc_sbx_caladan3_lrp_svp_t;

/* Size of each entry */
typedef enum soc_sbx_caladan3_lrp_list_esize_e_s {
    SOC_SBX_CALADAN3_1B = 0,
    SOC_SBX_CALADAN3_2B = 1,
    SOC_SBX_CALADAN3_4B = 2,
    SOC_SBX_CALADAN3_8B = 3,
    SOC_SBX_CALADAN3_16B = 4,
    SOC_SBX_CALADAN3_32B = 5,
    SOC_SBX_CALADAN3_64B = 6,
    SOC_SBX_CALADAN3_128B = 7,
    SOC_SBX_CALADAN3_256B = 8,
    SOC_SBX_CALADAN3_512B = 9,
    SOC_SBX_CALADAN3_1024B = 10,
    SOC_SBX_CALADAN3_2048B = 11,
    SOC_SBX_CALADAN3_4096B = 12,
    SOC_SBX_CALADAN3_8192B = 13,
    SOC_SBX_CALADAN3_16384B = 14,
    SOC_SBX_CALADAN3_32768B = 15
} soc_sbx_caladan3_lrp_list_esize_e_t;

/* These are the ucode operations */
typedef enum soc_sbx_caladan3_lrp_list_operation_e_s {
    SOC_SBX_CALADAN3_LRP_LIST_GET_READ_PTR  = 0,
    SOC_SBX_CALADAN3_LRP_LIST_DEQUEUE       = 1,
    SOC_SBX_CALADAN3_LRP_LIST_GET_WRITE_PTR = 2,
    SOC_SBX_CALADAN3_LRP_LIST_ENQUEUE       = 3
} soc_sbx_caladan3_lrp_list_operation_t;


/*
 * Listmgr Call back functions
 *    enqueue_callback_f is called whenever the list has items enqueued by the microcode
 *    dequeue_callback_f is called whenever the list has items dequeued by the microcode
 *    prev_offset is the offset tracked by the driver and nentries indicate how may entries were read/written
 */
typedef void (*soc_sbx_caladan3_lrp_list_enqueue_cb_f)(int unit, void *context, int prev_offset, int nentries);
typedef void (*soc_sbx_caladan3_lrp_list_dequeue_cb_f)(int unit, void *context, int prev_offset, int nentries);

/* Internal functions */
int 
soc_sbx_caladan3_lr_list_manager_enqueue_done(int unit, int listmgr_id);
int 
soc_sbx_caladan3_lr_list_manager_dequeue_done(int unit, int listmgr_id);
int
soc_sbx_caladan3_lr_list_init(int unit);



/* User accessible functions */

/*
 *   Function
 *     sbx_caladan3_lr_list_manager_init
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
 *      (IN) enqueue_threshold  : default to 1, can be set to any positive number < num_entries
 *      (IN) dqueue_threshold   : default to 1, can be set to any positive number < num_entries
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
                                      int reinit);
/*
 *   Function
 *     sbx_caladan3_lr_list_memory_alloc
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
                                      int entry_size);

/*
 *   Function
 *     sbx_caladan3_lr_list_memory_free
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
soc_sbx_caladan3_lr_list_memory_free(int unit, int listmgr_id);

/*
 *   Function
 *     sbx_caladan3_lr_list_manager_enqueue_event_enable
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
                                                  int enable);
/*
 *   Function
 *     sbx_caladan3_lr_list_manager_dequeue_event_enable
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
                                                  int enable);

/*
 *   Function
 *     sbx_caladan3_lr_list_manager_register_callback
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
                                               void *deq_context);
/*
 *   Function
 *     sbx_caladan3_lr_list_manager_dequeue_item
 *   Purpose
 *      Host Dequeue operation, invokes the call back directly rather than from isr
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
                                              int *length);
/*
 *   Function
 *     sbx_caladan3_lr_list_manager_enqueue_item
 *   Purpose
 *      Host enqueue operation, invokes the call back directly rather than from isr
 *   Parameters
 *      (IN) unit       : unit number of the device
 *      (IN) listmgr_id : list manager index
 *      (IN) data       : data to enqueue, used only when this driver manages memory
 *      (IN) lenth      : length of data to enqueue, in bytes
 *   Returns
 *       SOC_E_NONE    - success
 */
int 
soc_sbx_caladan3_lr_list_manager_enqueue_item(int unit, 
                                              int listmgr_id,
                                              unsigned char *data,
                                              int length);


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
soc_sbx_caladan3_lr_list_manager_set_enqueue_offset(int unit, int listmgr_id, int offset);


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
soc_sbx_caladan3_lr_list_manager_set_dequeue_offset(int unit, int listmgr_id, int offset);

#endif
