/* 
 * $Id: allocator.h,v 1.31 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:        allocator.h
 * Purpose:     Internal routines to the BCM library for allocating
 *              gu2 resources.
 */

#ifndef _BCM_INT_SBX_FE2000_ALLOCATOR_H_
#define _BCM_INT_SBX_FE2000_ALLOCATOR_H_

#include <bcm/types.h>
#include <soc/sbx/sbx_drv.h>
#include <soc/sbx/sbDq.h>

#define  _SBX_GU2_RES_FLAGS_CONTIGOUS   0x1
#define  _SBX_GU2_RES_FLAGS_RESERVE     0x2

#define  _SBX_GU2_RES_UNUSED_PORT       0xff

#define _SBX_GU2_MIN_VALID_IFID          32
#define _SBX_GU2_MAX_VALID_IFID         (1 << 16)

#define _SBX_GU2_MPLSTP_RSVD_VSI        (SBX_MAX_VID + 1)

/*
 * Resource types visible to the user
 */
typedef enum {
    SBX_GU2K_USR_RES_FTE_L3,
    SBX_GU2K_USR_RES_FTE_L3_MPATH,
    SBX_GU2K_USR_RES_FTE_IPMC,
    SBX_GU2K_USR_RES_FTE_GLOBAL_GPORT,
    SBX_GU2K_USR_RES_FTE_LOCAL_GPORT,
    SBX_GU2K_USR_RES_FTE_L2MC,
    SBX_GU2K_USR_RES_FTE_MPLS,
    SBX_GU2K_USR_RES_FTE_UNICAST,
    SBX_GU2K_USR_RES_OHI,
    SBX_GU2K_USR_RES_ETE_L2,
    SBX_GU2K_USR_RES_ETE_ENCAP,
    SBX_GU2K_USR_RES_IFID,
    SBX_GU2K_USR_RES_VRF,
    SBX_GU2K_USR_RES_VSI,
    SBX_GU2K_USR_RES_LINE_VSI,
    SBX_GU2K_USR_RES_QOS_PROFILE,
    SBX_GU2K_USR_RES_QOS_EGR_REMARK,
    SBX_GU2K_USR_RES_LPORT,
    SBX_GU2K_USR_RES_MPLS_LPORT,
    SBX_GU2K_USR_RES_PROTECTION,
    SBX_GU2K_USR_RES_VPWS_UNI_LPORT,
    SBX_GU2K_USR_RES_FTE_VPWS_UNI_GPORT,
    SBX_GU2K_USR_RES_FTE_VPXS_PW_FO,
    SBX_GU2K_USR_RES_MAX
} _sbx_gu2_usr_res_types_t;

typedef enum {
    SBX_GU2K_HW_RES_FTE = 0,
    SBX_GU2K_HW_RES_GLOBAL_GPORT_FTE,
    SBX_GU2K_HW_RES_LOCAL_GPORT_FTE,
    SBX_GU2K_HW_RES_OHI,
    SBX_GU2K_HW_RES_ETE,
    SBX_GU2K_HW_RES_IFID,
    SBX_GU2K_HW_RES_VRF,
    SBX_GU2K_HW_RES_VSI,
    SBX_GU2K_HW_RES_LINE_VSI,        /* For VPWS and other forms of wires */
    SBX_GU2K_HW_RES_QOS_PROFILE,
    SBX_GU2K_HW_RES_QOS_EGR_REMARK,
    SBX_GU2K_HW_RES_LPORT,
    SBX_GU2K_HW_RES_MPLS_LPORT,      /* For MPLS  */
    SBX_GU2K_HW_RES_PROTECTION,
    SBX_GU2K_HW_RES_VPWS_UNI_LPORT,
    SBX_GU2K_HW_RES_VPWS_UNI_GPORT_FTE,
    SBX_GU2K_HW_RES_VPXS_PW_FO_FTE,  /* For PW Redundancy */
    SBX_GU2K_HW_RES_MAX
} _sbx_gu2_hw_res_t;

typedef enum {
    SBX_GU2K_TABLE_NONE=0,
    SBX_GU2K_TABLE_FTE,
    SBX_GU2K_TABLE_VSI,
    SBX_GU2K_TABLE_OHI,
    SBX_GU2K_TABLE_ETE,
    SBX_GU2K_TABLE_VRF,
    SBX_GU2K_TABLE_QPROFILE,
    SBX_GU2K_TABLE_QMAP,
    SBX_GU2K_TABLE_QMAP_EGR,
    SBX_GU2K_TABLE_LPORT,
    SBX_GU2K_TABLE_PROT,
    SBX_GU2K_TABLE_IFID,
    SBX_GU2K_TABLE_MAX
} _sbx_gu2_hw_table_t;

typedef struct {
    uint32                   mask;
    char                     *name;
} _sbx_gu2_hw_table_attrs_t;


/* convert from external representation to internal */
typedef uint32 (*_sbx_gu2_ext_to_intl_f)(int unit, uint32 ext);
/* convert from internal representation to external */
typedef uint32 (*_sbx_gu2_intl_to_ext_f)(int unit, uint32 intl);

typedef struct {
    dq_t                      listNode;
    uint32                    high;
    uint32                    low;
} _sbx_gu2_resv_list_attrs_t;

typedef struct {
    uint32                    alloc_style;
    uint32                    base;
    uint32                    max_count;
    uint32                    blocking_factor; /* max possible block in aidx */
    uint32                    scaling_factor;
    char                      *name;
    uint32                    tables;
    dq_t                      resvList; /* reservation list */
    _sbx_gu2_ext_to_intl_f    e2i;
    _sbx_gu2_intl_to_ext_f    i2e;
} _sbx_gu2_hw_res_attrs_t;

/*
 *  This is a sort of pseudo-resource, managed by the lport resource.  It's
 *  intended to be used by modules that have to track additional data per
 *  lport (such as match criteria for PortVid or PortLabel).  Each unit's lport
 *  list is set up when the allocator is initialised.
 *
 *  For a given lport on a given unit, you need to use SBX_LPORT_DATAPTR, which
 *  works both for setting and getting.
 *
 *  Note that this is void*.  This is because there may be more than one module
 *  that uses logical ports and wants to keep associated data for each.  Only
 *  the module that owns a logical port should manipulate the pointer, and then
 *  only while it holds its own resource lock.
 */
typedef void *_sbx_lportList[SBX_LPORT_END - SBX_LPORT_BASE + 1];
typedef uint8 _sbx_lpTypeList[SBX_LPORT_END - SBX_LPORT_BASE + 1];
extern _sbx_lportList *_sbx_lport[BCM_MAX_NUM_UNITS];
extern _sbx_lpTypeList *_sbx_lptype[BCM_MAX_NUM_UNITS];
#define SBX_LPORT_DATAPTR(_unit,_lport) \
    ((*(_sbx_lport[_unit]))[((_lport) - SBX_LPORT_BASE)])
#define SBX_LPORT_TYPE(_unit,_lport) \
    ((*(_sbx_lptype[_unit]))[((_lport) - SBX_LPORT_BASE)])


/* Notes:
 * Logical ports for TB vlan, ports can be allocated without
 * GPORT creation. The following events will trigger logical port 
 * creation on them
 * [1] Apply policers
 * [2] Apply QOS 
 * [3] Apply Statistics - not yet supported 
 * [4] when oam is enabled on LSP
 * Reference count will be incremented by each of above cases and the 
 * last revertion will free back the logical port when reference count
 * is zero
 */
typedef enum {
    _LP_OWNER_POLICER,
    _LP_OWNER_QOS,
    _LP_OWNER_STATS,
    _LP_OWNER_OAM,
    _LP_OWNER_MAX
}_sbx_lp_owner_type_e_t;

typedef struct {
    uint32 refCount;
    _sbx_lp_owner_type_e_t owner;
} _sbx_lport_state_t;

extern int _sbx_gu2_reference_non_gport_lport_type(int unit, 
                                                   uint32 lport,
                                                   uint8 alloc, 
                                                   _sbx_lp_owner_type_e_t owner);

extern int _sbx_gu2_dereference_non_gport_lport_type(int unit, 
                                                     uint32 lport,
                                                     uint32 *refCount);

/*
 *   Function
 *      _sbx_gu2_resource_to_str
 *   Purpose
 *      DEBUG: returns a string representing the user resource types
 *   Parameters
 *      type    - (IN)  resource type
 *   Returns
 *      pointer to string of resource name
 */
#ifdef BROADCOM_DEBUG
extern const char*
_sbx_gu2_resource_to_str(_sbx_gu2_usr_res_types_t   type);
#endif


/*
 *   Function
 *      _sbx_gu2_resource_init
 *   Purpose
 *      Initialize the shr resource manager for all HW resources
 *      for the unit
 *   Parameters
 *      (IN) unit   : unit number of the device
 *   Returns
 *       BCM_E_NONE all resources are successfully initialized
 *       BCM_E_* as appropriate otherwise
 *   Notes
 *       Returns error is any of the resources cannot be initialized
 */
extern int
_sbx_gu2_resource_init(int unit);


/*
 *   Function
 *      _sbx_gu2_resource_alloc
 *   Purpose
 *      Allocate "count" number of resources.
 *   Parameters
 *      unit    - (IN)  unit number of the device
 *      type    - (IN)  resource type
 *                      (one of _sbx_gu2_usr_res_types_t)
 *      count   - (IN)  Number of resources required
 *      elements- (OUT) Resource Index return by the underlying allocator
 *      flags   - (IN)  _SBX_GU2_RES_FLAGS_CONTIGOUS
 *                      if the resources need to be contigous
 *   Returns
 *      BCM_E_NONE if element allocated successfully
 *      BCM_E_* as appropriate otherwise
 */
extern int
_sbx_gu2_resource_alloc(int                        unit,
                        _sbx_gu2_usr_res_types_t   type,
                        uint32                     count,
                        uint32                    *elements,
                        uint32                     flags);

/*
 *   Function
 *      _sbx_gu2_resource_free
 *   Purpose
 *      Free the resources specified in the array
 *   Parameters
 *      unit    - (IN)  unit number of the device
 *      type    - (IN)  resource type
 *                      (one of _sbx_gu2_usr_res_types_t)
 *      count   - (IN)  Number of resources to be freed
 *      elements- (IN)  Resource Ids to be freed
 *      flags   - (IN)  _SBX_GU2_RES_FLAGS_CONTIGOUS
 *   Returns
 *      BCM_E_NONE if element freed successfully
 *      BCM_E_* as appropriate otherwise
 *   NOTE:
 *      In case of block allocations, the user must pass the
 *      array back. Just the base element will not do.
 *      i.e. the array must be 'count' elements long
 */
extern int
_sbx_gu2_resource_free(int                        unit,
                       _sbx_gu2_usr_res_types_t   type,
                       uint32                     count,
                       uint32                    *elements,
                       uint32                     flags);
/*
 *   Function
 *      _sbx_gu2_resource_test
 *   Purpose
 *      Check to see if the specified resource is available
 *   Parameters
 *      unit    - (IN)  unit number of the device
 *      type    - (IN)  resource type
 *                      (one of _sbx_gu2_usr_res_types_t)
 *      element - (IN)  Resource Id to be checked
 *   Returns
 *      BCM_E_NOT_FOUND if resource is available
 *      BCM_E_EXIST     if resource is in use
 *      BCM_E_*         as appropriate otherwise
 */
extern int
_sbx_gu2_resource_test(int                        unit,
                       _sbx_gu2_usr_res_types_t   type,
                       uint32                     element);
/*
 *   Function
 *      _sbx_gu2_shr_resource_uninit
 *   Purpose
 *      Destroy the all the resource managers for this unit
 *   Parameters
 *      (IN) unit   : unit number of the device
 *   Returns
 *       BCM_E_NONE all resources are successfully destroyed
 *       BCM_E_* as appropriate otherwise
 *   Notes
 *       Returns error is any of the resources cannot be destroyed
 */
extern int
_sbx_gu2_resource_uninit(int unit);


/*
 *   Function
 *      _sbx_gu2_alloc_wellknown_resources
 *   Purpose
 *      Allocate well-known resources per unit
 *      and save the same in the soc structure
 *      for sharing between modules.
 *
 *   Parameters
 *      (IN) unit   : unit number of the device
 *   Returns
 *       BCM_E_NONE - All required resources are allocated
 *       BCM_E_*    - failure
 *   Notes
 */
extern int
_sbx_gu2_alloc_wellknown_resources(int unit);

/*
 *   Function
 *      _sbx_gu2_ismac_idx_alloc
 *   Purpose
 *     allocate an ingress smac index for use in local station match
 *
 *   Parameters
 *      (IN)  unit     : unit number of the device
 *      (IN)  flags    : reserve or not
 *      (IN)  smac     : SMAC to allocate
 *      (IN)  port     : port to allocate (0xff not used) 
 *      (OUT) idx      : soc layer smac index to use
 *   Returns
 *       BCM_E_NONE - All required resources are allocated
 *       BCM_E_*    - failure
 *   Notes
 *     uses reference counts when the same MAC is allocated.
 *     No hw is ever written 
 */
extern int
_sbx_gu2_ismac_idx_alloc(int unit, uint32 flags, bcm_mac_t smac, bcm_port_t port, uint32* idx);

/*
 *   Function
 *      _sbx_gu2_ismac_idx_free
 *   Purpose
 *     free an ingress smac index
 *
 *   Parameters
 *      (IN)     unit   : unit number of the device
 *      (IN/OUT) smac   : SMAC to free (used if idx == ~0)
 *      (IN/OUT)  port  : port to allocate (0xff not used)
 *      (IN/OUT) idx    : soc layer smac index to free, or ~0 to scan
 *   Returns
 *       BCM_E_EMPTY  : ref count equals zero, HW may be cleared
 *       BCM_E_NONE   : ref count decremented, but still non-zero
 *       BCM_E_*    - failure
 *   Notes
 *     if idx == ~0, <smac> is used to scan the table for a match.
 *                   match idx will be returned
 *     if idx != ~0, the smac found will be copied to <smac>
 */
extern int
_sbx_gu2_ismac_idx_free(int unit, bcm_mac_t smac, bcm_port_t port, uint32 *idx);

/*
 *   Function
 *      _sbx_gu2_esmac_idx_alloc
 *   Purpose
 *     allocate an egress smac index
 *
 *   Parameters
 *      (IN)  unit     : unit number of the device
 *      (IN)  flags    : reserve, or not
 *      (IN)  smac     : SMAC to allocate
 *      (OUT) idx      : soc layer smac index to use
 *   Returns
 *       BCM_E_NONE - All required resources are allocated
 *       BCM_E_*    - failure
 *   Notes
 *     uses reference counts when the same MAC is allocated.
 *     No hw is ever written 
 */
extern int
_sbx_gu2_esmac_idx_alloc(int unit, uint32 flags, bcm_mac_t smac, uint32* idx);

/*
 *   Function
 *      _sbx_gu2_esmac_idx_free
 *   Purpose
 *     free an egress smac index
 *
 *   Parameters
 *      (IN)     unit   : unit number of the device
 *      (IN/OUT) smac   : SMAC to free (used if idx == ~0)
 *      (IN/OUT) idx    : soc layer smac index to free, or ~0 to scan
 *   Returns
 *       BCM_E_EMPTY  : ref count equals zero, HW may be cleared
 *       BCM_E_NONE   : ref count decremented, but still non-zero
 *       BCM_E_*    - failure
 *   Notes
 *     if idx == ~0, <smac> is used to scan the table for a match.
 *                   match idx will be returned
 *     if idx != ~0, the smac found will be copied to <smac>
 */
extern int
_sbx_gu2_esmac_idx_free(int unit, bcm_mac_t smac, uint32 *idx);

/*
 *   Function
 *      _sbx_gu2_alloc_range_get
 *   Purpose
 *     Retrieve the range of valid IDs for the given resource
 *
 *   Parameters
 *      (IN)  unit   : unit number of the device
 *      (OUT) first  : first valid ID
 *      (OUT) last   : last valid ID
 *   Returns
 *       BCM_E_NONE - All required resources are allocated
 *       BCM_E_*    - failure
 *   Notes
 */
extern int 
_sbx_gu2_alloc_range_get(int unit, _sbx_gu2_usr_res_types_t type,
                         uint32 *first, uint32 *last);

/*
 *   Function
 *      _sbx_gu2_alloc_range_get
 *   Purpose
 *     Retrieve the range of valid IDs for the given resource
 *
 *   Parameters
 *      (IN)  unit   : unit number of the device
 *      (OUT) first  : first valid ID
 *      (OUT) last   : last valid ID
 *   Returns
 *       BCM_E_NONE - All required resources are allocated
 *       BCM_E_*    - failure
 *   Notes
 */
extern int 
_sbx_gu2_range_reserve(int unit, _sbx_gu2_usr_res_types_t type,
                       int highOrLow, uint32 val);

/*
 *   Function
 *      _sbx_gu2_get_global_fte_type
 *   Purpose
 *     Get Global FTE GPORT type
 *
 *   Parameters
 *      (IN) unit   : unit number of the device
 *      (IN) ftidx  : FTIndex
 *      (OUT)type   : FT GPORT type
 *   Returns
 *       BCM_E_NONE - success
 *       BCM_E_*    - failure
 */
extern int _sbx_gu2_get_global_fte_type(int     unit,
                                        uint32  ftidx, 
                                        uint8  *type);

/*
 *   Function
 *      _sbx_gu2_set_global_fte_type
 *   Purpose
 *     Set Global FTE GPORT type
 *
 *   Parameters
 *      (IN) unit   : unit number of the device
 *      (IN) ftidx  : FTIndex
 *      (OUT)type   : FT GPORT type
 *   Returns
 *       BCM_E_NONE - success
 *       BCM_E_*    - failure
 */
extern int _sbx_gu2_set_global_fte_type(int     unit,
                                        uint32  ftidx, 
                                        uint8   type);

/*
 *   Function
 *      _sbx_gu2_get_vsi_type
 *   Purpose
 *     Get Type of the VSI
 *
 *   Parameters
 *      (IN) unit   : unit number of the device
 *      (IN) ftidx  : VSI
 *      (OUT)type   : VSI GPORT type
 *   Returns
 *       BCM_E_NONE - success
 *       BCM_E_*    - failure
 */
int _sbx_gu2_get_vsi_type(int unit, uint32 vsi, uint8 *type);

/*
 *   Function
 *      _sbx_gu2_set_vsi_type
 *   Purpose
 *     Get Type of the VSI
 *
 *   Parameters
 *      (IN) unit   : unit number of the device
 *      (IN) ftidx  : VSI
 *      (OUT)type   : VSI GPORT type
 *   Returns
 *       BCM_E_NONE - success
 *       BCM_E_*    - failure
 */
int _sbx_gu2_set_vsi_type(int unit, uint32 vsi, uint8 type);


int _sbx_gu2_resource_alloc_counters(int unit,
                                     uint32 *counterId, 
                                     int type,
                                     uint8 numCounter);

int _sbx_gu2_resource_free_counters(int unit,
                                    uint32 counterId,
                                    int type);

#endif /* _BCM_INT_SBX_FE2000_ALLOCATOR_H_ */
