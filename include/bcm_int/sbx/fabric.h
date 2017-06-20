/*
 * $Id: fabric.h,v 1.22 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */

#ifndef _BCM_INT_SBX_FABRIC_H_
#define _BCM_INT_SBX_FABRIC_H_

#include <bcm/fabric.h>
#include <soc/sbx/sbFabCommon.h>

#define DEBUG_CROSSBAR_MAP 1

#define BCM_SBX_DS_ID_IS_RESOURCE_ALLOCATED(unit, ds_id)                                   \
            (*(ds_state_p[unit]->ds_grp_resource + (ds_id / 8)) & (1 << (ds_id % 8)))

#define BCM_SBX_DS_ID_SET_RESOURCE(unit, ds_id)                                            \
            (*(ds_state_p[unit]->ds_grp_resource + (ds_id / 8))) |= (1 << (ds_id % 8))

#define BCM_SBX_DS_ID_CLEAR_RESOURCE(unit, ds_id)                                          \
            (*(ds_state_p[unit]->ds_grp_resource + (ds_id / 8))) &= (~(1 << (ds_id % 8)))

#define BCM_SBX_DS_ID_IS_NODE_MEMBER(unit, ds_id, node)                                    \
            (*(ds_state_p[unit]->ds_grp_membership +                                       \
                  (ds_id * ds_state_p[unit]->num_bytes_ds_desc + (node / 8))) &            \
                                                                  (1 << (node % 8)))

#define BCM_SBX_DS_ID_IS_NODE_MEMBER(unit, ds_id, node)                                    \
            (*(ds_state_p[unit]->ds_grp_membership +                                       \
                  (ds_id * ds_state_p[unit]->num_bytes_ds_desc + (node / 8))) &            \
                                                                  (1 << (node % 8)))

#define BCM_SBX_DS_ID_SET_NODE_MEMBER(unit, ds_id, node)                                   \
            (*(ds_state_p[unit]->ds_grp_membership +                                       \
                  (ds_id * ds_state_p[unit]->num_bytes_ds_desc + (node / 8)))) |=          \
                                                                  (1 << (node % 8))

#define BCM_SBX_DS_ID_CLEAR_NODE_MEMBER(unit, ds_id, node)                                 \
            (*(ds_state_p[unit]->ds_grp_membership +                                       \
                  (ds_id * ds_state_p[unit]->num_bytes_ds_desc + (node / 8)))) &=          \
                                                                  (~(1 << (node % 8)))

#define BCM_SBX_DS_ID_CLEAR_ALL_NODE_MEMBER(unit, ds_id, iter)                             \
        {                                                                                  \
            for (iter = 0; iter < ds_state_p[unit]->num_bytes_ds_desc; iter++) {           \
                (*(ds_state_p[unit]->ds_grp_membership +                                   \
                  (ds_id * ds_state_p[unit]->num_bytes_ds_desc + iter))) = 0;              \
            }                                                                              \
        }

extern int
bcm_sbx_fabric_ds_id_to_ef_ds_id(int unit, bcm_fabric_distribution_t ds_id,
                                           bcm_fabric_distribution_t *ef_ds_id_p);

extern int
bcm_sbx_fabric_dsid_to_nef_dsid(int unit, bcm_fabric_distribution_t ds_id,
                                          bcm_fabric_distribution_t *nef_ds_id_p);

#define BCM_SBX_CROSSBAR_VALID(xbar) ((xbar != SB_FAB_DEVICE_INVALID_CROSSBAR) && (xbar < SB_FAB_DEVICE_MAX_NODE_CROSSBARS))

typedef struct bcm_sbx_xbar_state_s {
    uint8  xbar_in_use[SB_FAB_DEVICE_MAX_NODE_CROSSBARS];
    uint8  xbar_map[SB_FAB_DEVICE_MAX_LOGICAL_CROSSBARS][SB_FAB_DEVICE_MAX_NODES];
    uint8  lchan_to_pchan[SB_FAB_DEVICE_MAX_NODE_LOGICAL_DATA_CHANNELS];
    uint8  pchan_to_lchan[SB_FAB_DEVICE_MAX_NODE_PHYSICAL_DATA_CHANNELS];
    int8   xbar_nodetype[SB_FAB_DEVICE_MAX_LOGICAL_CROSSBARS];
} bcm_sbx_xbar_state_t;


typedef struct bcm_sbx_fabric_state_s {
    bcm_fabric_control_redundancy_handler_t red_f;
    soc_scache_handle_t                     handle;
    uint32 timeslot_size;
    uint32 scaled;
    uint32 old_demand_scale;
    bcm_sbx_xbar_state_t *xbar_state;
    uint8  *congestion_size; /* indexed by [modid] - modid is in the range of 0 - 255 */
    uint32 egress_aging;
} bcm_sbx_fabric_state_t;

typedef struct bcm_sbx_ds_state_s {
    int       initialized;
    uint32    max_ds_ids;          /* maximum number of distribution groups/ESETS */
                                   /* supported by device                         */
    uint16    num_nodes;           /* current number of configured nodes          */
    uint16    max_nodes;           /* maximum number of nodes supported by device */
    uint32    num_bytes_ds_desc;   /* number of bytes required per distribution   */
                                   /* group for describing node membership        */
    uint32    num_ds_grps_used;    /* number of distribution groups/ESETs         */

    uint8     *ds_grp_resource;    /* resorce allocation bit map */
    uint8     *ds_grp_membership;  /* resource membership information */
    uint8     *mc_full_eval_min_state; /* per ds_id state                                               */
                                       /* if set, hw will forward over backplane if any member can send */
                                       /* (no mc FIFO backpressure), if cleared, hw will only forward   */
                                       /* over backplane if all members can send (all nodes w/o mc bp)  */
} bcm_sbx_ds_state_t;

/* Function prototype for common function */
int32
bcm_sbx_fabric_get_timeslot_size(int32 nTotalLogicalCrossbars,
				 int32 bSetTimeslotSizeForHalfBus);

int
bcm_sbx_adjust_demand_scale(int unit, uint64 xbars,
                  int adjust, int *adjusted_demand_shift, uint32 *adjusted_clocks_per_epoch);

int32
bcm_sbx_fabric_get_rate_conversion(int unit, int32 nTotalLogicalCrossbars);

int32
bcm_sbx_fabric_timeslot_burst_size_bytes_set(int unit, int els);

int
bcm_sbx_fabric_timeslot_burst_size_bytes_get(int unit, int src_node_type, 
                                             int dest_node_type, 
                                             int num_channels, 
                                             int *bytes_per_timeslot);
int
bcm_sbx_fabric_lxbar_alloc(int unit, 
                           int *lxbar, 
                           int with_id);

int
bcm_sbx_fabric_lxbar_free(int unit, 
                          int lxbar);

int
bcm_sbx_fabric_xbar_for_sfi_get(int unit, 
                                int node, 
                                int sfi_port, 
                                int *lxbar, 
                                int *lxbar1);

int
bcm_sbx_fabric_sfi_for_xbar_get(int unit, 
                                int xbar, 
                                int node, 
                                int *sfi_port);

int
bcm_sbx_fabric_sfi_for_xbar_set(int unit, 
                                int xbar, 
                                int node, 
                                int sfi_port);

int
bcm_sbx_fabric_nodetype_for_xbar_get(int unit, 
                                     int xbar, 
                                     int *nt_qe2k);

int
bcm_sbx_fabric_nodetype_for_xbar_set(int unit,
                                     int xbar,
                                     int nt_qe2k);

#ifdef BCM_WARM_BOOT_SUPPORT
extern int
bcm_sbx_wb_fabric_state_sync(int unit, int sync);
#endif /* BCM_WARM_BOOT_SUPPORT */

#ifdef BCM_WARM_BOOT_SUPPORT_SW_DUMP
void 
bcm_sbx_wb_fabric_sw_dump(int unit);
#endif /* BCM_WARM_BOOT_SUPPORT_SW_DUMP */

#endif  /* _BCM_INT_SBX_FABRIC_H_ */
