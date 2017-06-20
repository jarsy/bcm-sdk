/*
 * $Id:$
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 */
#include <shared/bsl.h>
#include <soc/dnx/legacy/ps_db.h>
#include <soc/dnx/legacy/ARAD/arad_egr_queuing.h>
#include <soc/dnx/legacy/port_sw_db.h>
#include <shared/bitop.h>
#include <soc/drv.h>
#include <soc/dnx/legacy/drv.h>
#include <soc/dnx/legacy/mbcm.h>
#include <soc/dnxc/legacy/error.h>
 
#ifdef _ERR_MSG_MODULE_NAME
  #error "_ERR_MSG_MODULE_NAME redefined"
#endif

#define _ERR_MSG_MODULE_NAME BSL_SOC_PORT


#define JER2_ARAD_PS_DB_IS_PS_RCPU_TYPE(ps_id)(-1) /*JER2_ARAD_PS_RCPU_VALID_RANGE(ps_id)*/
#define JER2_ARAD_PS_DB_IS_PS_CPU_TYPE(ps_id) (-1) /*JER2_ARAD_PS_CPU_VALID_RANGE(ps_id)*/
#define JER2_ARAD_PS_DB_IS_PS_ERP_TYPE(ps_id) (-1) /*(JER2_ARAD_FAP_EGRESS_REPLICATION_BASE_Q_PAIR == JER2_ARAD_PS2BASE_PORT_TC(ps_id))*/
#define JER2_ARAD_PS_DB_IS_PS_OAMP_TYPE(ps_id) (-1) /*(JER2_ARAD_OAMP_PORT_ID == JER2_ARAD_PS2BASE_PORT_TC(ps_id))*/
#define JER2_ARAD_PS_DB_IS_PS_OLP_TYPE(ps_id) (-1) /*(JER2_ARAD_OLP_PORT_ID == JER2_ARAD_PS2BASE_PORT_TC(ps_id))*/

#define JER2_ARAD_PS_DB_NOF_PS_RESOURCES 32

#define JER2_ARAD_PS_MAX_NIF_INTERFACE 29 /* Including CPU */
#define JER2_ARAD_PS_RCY_INTERFACE     JER2_ARAD_PS_MAX_NIF_INTERFACE + 1
#define JER2_ARAD_PS_ERP_INTERFACE     JER2_ARAD_PS_MAX_NIF_INTERFACE + 2
#define JER2_ARAD_PS_OAMP_INTERFACE    JER2_ARAD_PS_MAX_NIF_INTERFACE + 3
#define JER2_ARAD_PS_OLP_INTERFACE     JER2_ARAD_PS_MAX_NIF_INTERFACE + 4
#define JER2_ARAD_PS_NON_CH_INTERFACE  JER2_ARAD_PS_MAX_NIF_INTERFACE + 5 /* Used for Jericho - non channalizedinterfaces can use the same PS */
#define JER2_ARAD_PS_RCPU_INTERFACE    JER2_ARAD_PS_MAX_NIF_INTERFACE + 6
#define JER2_ARAD_PS_DB_NOF_INTERFACES JER2_ARAD_PS_MAX_NIF_INTERFACE + 7

typedef enum jer2_arad_ps_db_resource_type_e {
    psResourceNif = 0,
    psResourceRcpu,
    psResourceCpu,
    psResourceOlp,
    psResourceErp,
    psResourceOamp,
    psResourceCount
} jer2_arad_ps_db_resource_type_t;

typedef struct jer2_arad_ps_db_data_s {
    uint32 allocated_binding_qs_bmap;
    uint32 allocated_non_binding_qs_bmap;
    uint32 prio_mode;
} jer2_arad_ps_db_data_t;

typedef struct jer2_arad_ps_db_resource_s {
    uint32 bitmap[1];
} jer2_arad_ps_db_resource_t;

typedef struct jer2_arad_ps_db_core_resources_s {
    jer2_arad_ps_db_data_t       ps[JER2_ARAD_PS_DB_NOF_PS_RESOURCES];
    jer2_arad_ps_db_resource_t   free_ps[psResourceCount];
    jer2_arad_ps_db_resource_t   allocated_ps[JER2_ARAD_PS_DB_NOF_INTERFACES];
} jer2_arad_ps_db_core_resources_t;

static jer2_arad_ps_db_core_resources_t core_resources[SOC_MAX_NUM_DEVICES][SOC_DNX_DEFS_MAX(NOF_CORES)];

static void 
jer2_arad_ps_db_ps_to_type(int unit, int ps_idx, int core, jer2_arad_ps_db_resource_type_t* type) {

    if(JER2_ARAD_PS_DB_IS_PS_ERP_TYPE(ps_idx)) {
        *type = (SOC_INFO(unit).erp_port[core] == -1) ? psResourceCpu : psResourceErp;
    } else if(JER2_ARAD_PS_DB_IS_PS_OAMP_TYPE(ps_idx)) {
        *type = (SOC_INFO(unit).oamp_port[core] == -1) ? psResourceCpu : psResourceOamp;
    } else if(JER2_ARAD_PS_DB_IS_PS_OLP_TYPE(ps_idx)) {
        *type = (SOC_INFO(unit).olp_port[core] == -1) ? psResourceCpu : psResourceOlp;
    } else if(JER2_ARAD_PS_DB_IS_PS_RCPU_TYPE(ps_idx)) {
        *type = psResourceRcpu;
    } else if(JER2_ARAD_PS_DB_IS_PS_CPU_TYPE(ps_idx)) {
        *type = psResourceCpu;
    } else {
        *type = psResourceNif;
    }
}

int
jer2_arad_ps_db_init(int unit) {

    int i, core;
    jer2_arad_ps_db_resource_type_t type;

    DNXC_INIT_FUNC_DEFS;

    sal_memset(core_resources[unit], 0, sizeof(jer2_arad_ps_db_core_resources_t)*SOC_DNX_DEFS_MAX(NOF_CORES));

    for(i=0 ; i<JER2_ARAD_PS_DB_NOF_PS_RESOURCES ; i++) {
        for(core = 0 ; core < SOC_DNX_DEFS_GET(unit, nof_cores) ; core++) {
            jer2_arad_ps_db_ps_to_type(unit, i, core, &type);
            SHR_BITSET(core_resources[unit][core].free_ps[type].bitmap, i);
        }
    }

    DNXC_FUNC_RETURN;
}

static int
jer2_arad_ps_db_try_allocate(int unit, jer2_arad_ps_db_data_t* ps, int core, int ps_id, int out_port_priority, int is_binding, int is_init, int* allocated, int* internal_id) {

    int i , already_allocated, priority_i, q_pair;
    uint32 queues[1];
    uint8 is_hr_free;

    DNXC_INIT_FUNC_DEFS;

    *allocated = 0;
    *internal_id = -1;
    is_hr_free = 0;

    if(out_port_priority != ps->prio_mode && ps->prio_mode != 0 && is_binding) {
        SOC_EXIT;
    }

    *queues = ps->allocated_binding_qs_bmap | ps->allocated_non_binding_qs_bmap;

    for(i=0 ; i<JER2_ARAD_EGR_NOF_Q_PAIRS_IN_PS ; i += out_port_priority) {
        SHR_BITTEST_RANGE(queues, i, out_port_priority, already_allocated);
        if(!already_allocated) {
            /* validate HRs are free (only when ps is allocated after init) */
            if(is_init) {
                is_hr_free = 1;
            } else { /* check all HRs (according to port priority ) */
                for(priority_i = 0; priority_i < out_port_priority; priority_i++) {
                    q_pair = ps_id * JER2_ARAD_EGR_NOF_Q_PAIRS_IN_PS + i + priority_i;
                    DNXC_IF_ERR_EXIT(soc_jer2_arad_validate_hr_is_free(unit, core, q_pair, &is_hr_free));
                    if(!is_hr_free) {
                        break;
                    }
                }
            }

            if (is_hr_free) {
                *allocated = 1;
                *internal_id = i;
                if(is_binding) {
                    SHR_BITSET_RANGE(&ps->allocated_binding_qs_bmap, i, out_port_priority);
                    ps->prio_mode = out_port_priority;
                } else {
                    SHR_BITSET_RANGE(&ps->allocated_non_binding_qs_bmap, i, out_port_priority);
                }
                break;
            }
        }
    }

exit:
    DNXC_FUNC_RETURN;
}

static void
jer2_arad_ps_db_try_allocate_with_id(jer2_arad_ps_db_data_t* ps, int out_port_priority, int is_binding, int use_port_priority ,int internal_id, int* allocated) {

    int already_allocated;
    uint32 queues[1];

    *allocated = 0;

    if(out_port_priority != ps->prio_mode && ps->prio_mode != 0 && use_port_priority) {
        return;
    }

    *queues = ps->allocated_binding_qs_bmap | ps->allocated_non_binding_qs_bmap;


    SHR_BITTEST_RANGE(queues, internal_id, out_port_priority, already_allocated);
    if(!already_allocated) {
        *allocated = 1;
        if(is_binding) {
            SHR_BITSET_RANGE(&ps->allocated_binding_qs_bmap, internal_id, out_port_priority);        
        } else {
            SHR_BITSET_RANGE(&ps->allocated_non_binding_qs_bmap, internal_id, out_port_priority);
        }
        if (use_port_priority) {
            ps->prio_mode = out_port_priority;
        }
    }
    
}

static int
jer2_arad_ps_db_interface_get(int unit, soc_port_t port, uint32 *interface) {

    soc_port_if_t interface_type;
    int is_channelized, core, rv;
    uint32 tm_port;
    soc_dnx_config_jer2_arad_t *dnx_jer2_arad;
    
    DNXC_INIT_FUNC_DEFS;

    dnx_jer2_arad = SOC_DNX_CONFIG(unit)->jer2_arad;

    DNXC_IF_ERR_EXIT(dnx_port_sw_db_interface_type_get(unit, port, &interface_type)); 
    DNXC_IF_ERR_EXIT(dnx_port_sw_db_local_to_tm_port_get(unit, port, &tm_port, &core));

    switch(interface_type) {
        case SOC_PORT_IF_OLP:
            (*interface) = JER2_ARAD_PS_OLP_INTERFACE;
            break;
        case SOC_PORT_IF_OAMP:
            (*interface) = JER2_ARAD_PS_OAMP_INTERFACE;
            break;
        case SOC_PORT_IF_ERP:
            (*interface) = JER2_ARAD_PS_ERP_INTERFACE;
            break;
        case SOC_PORT_IF_RCY:
            (*interface) = JER2_ARAD_PS_RCY_INTERFACE;
            break;
        case SOC_PORT_IF_CPU:
            if (SOC_PBMP_MEMBER(dnx_jer2_arad->init.rcpu.slave_port_pbmp, port)) {
                (*interface) = JER2_ARAD_PS_RCPU_INTERFACE;
                break;
            }
            /* Fall through */
        default:
            /*NIF or CPU*/
            if(SOC_IS_ARADPLUS_AND_BELOW(unit)) {
                DNXC_IF_ERR_EXIT(dnx_port_sw_db_first_phy_port_get(unit, port, interface));
            } else {
                rv = MBCM_DNX_SOC_DRIVER_CALL(unit, mbcm_dnx_port2egress_offset, (unit, core, tm_port, interface /*egr_if*/));
                DNXC_IF_ERR_EXIT(rv);
                rv = dnx_port_sw_db_is_channelized_port_get(unit, port, &is_channelized);
                DNXC_IF_ERR_EXIT(rv);

                if((*interface) >= SOC_DNX_IMP_DEFS_GET(unit, nof_channelized_interfaces) || !is_channelized) {
                    (*interface) = JER2_ARAD_PS_NON_CH_INTERFACE;
                }
            }
            break;
    }

exit:
    DNXC_FUNC_RETURN;
}

int 
jer2_arad_ps_db_alloc_binding_ps_with_id(int unit, soc_port_t port, int out_port_priority, int base_q_pair) {

    uint32 interface;
    int core, allocated, internal_id, i;
    int ps_is_free = 0, ps_type = 0, ps_idx;
    DNXC_INIT_FUNC_DEFS;

    ps_idx = base_q_pair / JER2_ARAD_EGR_NOF_Q_PAIRS_IN_PS;
    internal_id = base_q_pair % JER2_ARAD_EGR_NOF_Q_PAIRS_IN_PS;
    DNXC_IF_ERR_EXIT(jer2_arad_ps_db_interface_get(unit, port, &interface));

    DNXC_IF_ERR_EXIT(dnx_port_sw_db_core_get(unit, port, &core));

    /* check PS os allocated or not-bound to other interface */
    if(SHR_BITGET(core_resources[unit][core].allocated_ps[interface].bitmap, ps_idx)) {
        jer2_arad_ps_db_try_allocate_with_id(&core_resources[unit][core].ps[ps_idx], out_port_priority, 1, 1, internal_id, &allocated);
        if(!allocated) {
            DNXC_EXIT_WITH_ERR(SOC_E_FAIL, (_BSL_DNXC_MSG("Failed to allocate with-id qpairs for port %d"), port)); 

        }
    } else {
        /* Check free PS */
        ps_is_free = 0;
        for(i=0 ; i<psResourceCount ; i++) {
            if(SHR_BITGET(core_resources[unit][core].free_ps[i].bitmap, ps_idx)){
                ps_is_free = 1;
                ps_type = i;
                break;
            }
        }

        if(!ps_is_free) {
            DNXC_EXIT_WITH_ERR(SOC_E_FAIL, (_BSL_DNXC_MSG("Failed to allocate with-id qpairs for port %d, PS is used by other interface"), port)); 
        }

        /* allocate qpairs*/
        jer2_arad_ps_db_try_allocate_with_id(&core_resources[unit][core].ps[ps_idx], out_port_priority, 1, 1, internal_id, &allocated);
        if(allocated) {
            SHR_BITCLR(core_resources[unit][core].free_ps[ps_type].bitmap, ps_idx);
            SHR_BITSET(core_resources[unit][core].allocated_ps[interface].bitmap, ps_idx);
        } else {
            DNXC_EXIT_WITH_ERR(SOC_E_FAIL, (_BSL_DNXC_MSG("Failed to allocate with-id qpairs for port %d"), port)); 
        }
    }

exit:
    DNXC_FUNC_RETURN;
}

int 
jer2_arad_ps_db_find_free_binding_ps(int unit, soc_port_t port, int out_port_priority, int is_init, int* base_q_pair) {

    int i, j, core;
    int allocated = FALSE;
    int internal_id;
    uint32 interface, tm_port;
    jer2_arad_ps_db_resource_type_t types[psResourceCount];
    soc_port_if_t interface_type;
    soc_dnx_config_jer2_arad_t *dnx_jer2_arad;
    
    DNXC_INIT_FUNC_DEFS;

    dnx_jer2_arad = SOC_DNX_CONFIG(unit)->jer2_arad;

    *base_q_pair = -1;

    DNXC_IF_ERR_EXIT(dnx_port_sw_db_interface_type_get(unit, port, &interface_type));  
    DNXC_IF_ERR_EXIT(dnx_port_sw_db_local_to_tm_port_get(unit, port, &tm_port, &core));

    /*stage 1 - find allocated PS*/
    DNXC_IF_ERR_EXIT(jer2_arad_ps_db_interface_get(unit, port, &interface));

    for(i=0 ; i<JER2_ARAD_PS_DB_NOF_PS_RESOURCES ; i++) {
        if(SHR_BITGET(core_resources[unit][core].allocated_ps[interface].bitmap, i)) {
            DNXC_IF_ERR_EXIT(jer2_arad_ps_db_try_allocate(unit, &core_resources[unit][core].ps[i], core, i, out_port_priority, 1, is_init, &allocated, &internal_id));
            if(allocated) {
                *base_q_pair = i*JER2_ARAD_EGR_NOF_Q_PAIRS_IN_PS + internal_id;
                break;
            }
        }
    }

    /*stage 2 - allocate new PS*/
    if(!allocated) {
        for(i=0 ; i<psResourceCount ; i++) {
            types[i] = psResourceCount;
        }

        switch(interface_type) {
            case SOC_PORT_IF_CPU:
                if (SOC_PBMP_MEMBER(dnx_jer2_arad->init.rcpu.slave_port_pbmp, port)) {
                    types[0] = psResourceRcpu;
                } else {
                    types[0] = psResourceCpu;
                }
                break;
            case SOC_PORT_IF_OLP:
                types[0] = psResourceOlp;
                break;
            case SOC_PORT_IF_OAMP:
                types[0] = psResourceOamp;
                break;
            case SOC_PORT_IF_ERP:
                types[0] = psResourceErp;
                break;
            default:
                /*NIF*/
                types[0] = psResourceNif;
                types[1] = psResourceRcpu;
                types[2] = psResourceCpu;
                types[3] = psResourceErp;
                break;
        }

        j=0;
        while(types[j] != psResourceCount) {
            for(i=0 ; i<JER2_ARAD_PS_DB_NOF_PS_RESOURCES ; i++) {
                if(SHR_BITGET(core_resources[unit][core].free_ps[types[j]].bitmap, i)) {
                    DNXC_IF_ERR_EXIT(jer2_arad_ps_db_try_allocate(unit, &core_resources[unit][core].ps[i], core, i, out_port_priority, 1, is_init, &allocated, &internal_id));
                    if(allocated) {
                        SHR_BITCLR(core_resources[unit][core].free_ps[types[j]].bitmap, i);
                        SHR_BITSET(core_resources[unit][core].allocated_ps[interface].bitmap, i);
                        *base_q_pair = i*JER2_ARAD_EGR_NOF_Q_PAIRS_IN_PS + internal_id;
                        break;
                    }
                }
            }

            if(allocated) {
                break;
            }

            j++;
        }
    }
    if (!allocated) {
        DNXC_EXIT_WITH_ERR(SOC_E_FAIL, (_BSL_DNXC_MSG("no free PS port %d"), port)); 
    }

exit:
    DNXC_FUNC_RETURN;
}

int 
jer2_arad_ps_db_find_free_non_binding_ps(int unit, int core, int is_init, int* base_q_pair) {

    int i,
        allocated = FALSE,
        internal_id,
        any_allocated;

    DNXC_INIT_FUNC_DEFS;

    *base_q_pair = -1;

    /*stage 1 - try allocate in ERP\OLP\OAMP*/
    for(i=0 ; i<JER2_ARAD_PS_DB_NOF_PS_RESOURCES ; i++) {
        if(SHR_BITGET(core_resources[unit][core].free_ps[psResourceOlp].bitmap, i) ||
           SHR_BITGET(core_resources[unit][core].free_ps[psResourceOamp].bitmap, i) ||
           SHR_BITGET(core_resources[unit][core].free_ps[psResourceErp].bitmap, i)) {
           DNXC_IF_ERR_EXIT(jer2_arad_ps_db_try_allocate(unit, &core_resources[unit][core].ps[i], core, i, 1, 0, is_init, &allocated, &internal_id));
            if(allocated) {
                *base_q_pair = i*JER2_ARAD_EGR_NOF_Q_PAIRS_IN_PS + internal_id;
                break;
            }
        }
    }

    /*stage 2 - try allocate in allocated NIF*/
    if(!allocated) {
        for(i=0 ; i < JER2_ARAD_PS_DB_NOF_PS_RESOURCES ; i++) {
            SHR_BITTEST_RANGE(&core_resources[unit][core].ps[i].allocated_binding_qs_bmap, 0, JER2_ARAD_EGR_NOF_Q_PAIRS_IN_PS, any_allocated);
            if(any_allocated) {
                DNXC_IF_ERR_EXIT(jer2_arad_ps_db_try_allocate(unit, &core_resources[unit][core].ps[i], core, i, 1, 0, is_init, &allocated, &internal_id));
                if(allocated) {
                    *base_q_pair = i*JER2_ARAD_EGR_NOF_Q_PAIRS_IN_PS + internal_id;
                    break;
                }
            }
        }
    }

    /*stage 3 - try allocate in free resources*/
    if(!allocated) {
        for(i=0 ; i < JER2_ARAD_PS_DB_NOF_PS_RESOURCES ; i++) {
            SHR_BITTEST_RANGE(&core_resources[unit][core].ps[i].allocated_binding_qs_bmap, 0, JER2_ARAD_EGR_NOF_Q_PAIRS_IN_PS, any_allocated);
            if(!any_allocated) {
                DNXC_IF_ERR_EXIT(jer2_arad_ps_db_try_allocate(unit, &core_resources[unit][core].ps[i], core, i, 1, 0, is_init, &allocated, &internal_id));
                if(allocated) {
                    *base_q_pair = i*JER2_ARAD_EGR_NOF_Q_PAIRS_IN_PS + internal_id;
                    break;
                }
            }
        }
    }

    if (!allocated) {
        DNXC_EXIT_WITH_ERR(SOC_E_FAIL, (_BSL_DNXC_MSG("no free PS"))); 
    }
exit:   
    DNXC_FUNC_RETURN;
}

int 
jer2_arad_ps_db_find_free_non_binding_ps_with_id(int unit, soc_port_t port, int base_q_pair) {
    uint32 out_port_priority;
    int core, allocated, internal_id, ps_idx;

    DNXC_INIT_FUNC_DEFS;

    ps_idx = base_q_pair / JER2_ARAD_EGR_NOF_Q_PAIRS_IN_PS;
    internal_id = base_q_pair % JER2_ARAD_EGR_NOF_Q_PAIRS_IN_PS;

    DNXC_IF_ERR_EXIT(dnx_port_sw_db_core_get(unit, port, &core));
    DNXC_IF_ERR_EXIT(dnx_port_sw_db_local_to_out_port_priority_get(unit, port, &out_port_priority));

    /* allocate qpairs*/
    jer2_arad_ps_db_try_allocate_with_id(&core_resources[unit][core].ps[ps_idx], out_port_priority, 0, 0, internal_id, &allocated);
    if(!allocated) {
        DNXC_EXIT_WITH_ERR(SOC_E_FAIL, (_BSL_DNXC_MSG("Failed to allocate with-id qpairs for port %d"), port)); 
    }

exit:
    DNXC_FUNC_RETURN;
}

int 
jer2_arad_ps_db_release_binding_ps(int unit, soc_port_t port, int base_q_pair) {

    int idx, core;
    int internal_id;
    int any_allocated;
    uint32 interface, tm_port;
    jer2_arad_ps_db_resource_type_t type;
    soc_port_if_t interface_type;

    DNXC_INIT_FUNC_DEFS;

    DNXC_IF_ERR_EXIT(dnx_port_sw_db_local_to_tm_port_get(unit, port, &tm_port, &core));

    idx = (base_q_pair) / JER2_ARAD_EGR_NOF_Q_PAIRS_IN_PS;
    internal_id = (base_q_pair) % JER2_ARAD_EGR_NOF_Q_PAIRS_IN_PS;

    SHR_BITCLR_RANGE(&core_resources[unit][core].ps[idx].allocated_binding_qs_bmap, internal_id, core_resources[unit][core].ps[idx].prio_mode);

    /*if last binding port - need to release PS*/
    SHR_BITTEST_RANGE(&core_resources[unit][core].ps[idx].allocated_binding_qs_bmap, 0, JER2_ARAD_EGR_NOF_Q_PAIRS_IN_PS, any_allocated);
    if(!any_allocated) {

        /* in case ERP port exists and the PS is ERP PS, then don't set PS priority to 0 */
        if (!((SOC_INFO(unit).erp_port[core] != -1) && JER2_ARAD_PS_DB_IS_PS_ERP_TYPE(idx))) {
            core_resources[unit][core].ps[idx].prio_mode = 0;
        }

        jer2_arad_ps_db_ps_to_type(unit, idx, core, &type);
        SHR_BITSET(core_resources[unit][core].free_ps[type].bitmap, idx);

        DNXC_IF_ERR_EXIT(dnx_port_sw_db_interface_type_get(unit, port, &interface_type));  

        /*stage 1 - find allocated PS*/
        DNXC_IF_ERR_EXIT(jer2_arad_ps_db_interface_get(unit, port, &interface));
        SHR_BITCLR(core_resources[unit][core].allocated_ps[interface].bitmap, idx);
    }

exit:
    DNXC_FUNC_RETURN;
}

int 
jer2_arad_ps_db_release_non_binding_ps(int unit, int core, int out_port_priority, int base_q_pair) {

    int idx;
    int internal_id;

    DNXC_INIT_FUNC_DEFS;

    idx = (base_q_pair) / JER2_ARAD_EGR_NOF_Q_PAIRS_IN_PS;
    internal_id = (base_q_pair) % JER2_ARAD_EGR_NOF_Q_PAIRS_IN_PS;

    SHR_BITCLR_RANGE(&core_resources[unit][core].ps[idx].allocated_non_binding_qs_bmap, internal_id, out_port_priority);

    DNXC_FUNC_RETURN;
}

#undef _ERR_MSG_MODULE_NAME


