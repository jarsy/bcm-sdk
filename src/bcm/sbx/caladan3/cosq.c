/*
 * $Id: cosq.c$
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * Caladan3 Cosq API
 */
#if defined(BCM_CALADAN3_SUPPORT)

#ifdef _ERR_MSG_MODULE_NAME
  #error "_ERR_MSG_MODULE_NAME redefined"
#endif

#define _ERR_MSG_MODULE_NAME BSL_LS_BCM_COSQ

#include <shared/bsl.h>

#include <soc/drv.h>
#include <soc/sbx/sbx_drv.h>

#include <bcm/error.h>
#include <soc/sbx/caladan3.h>
#include <bcm_int/common/debug.h>
#include <bcm_int/sbx/caladan3/cosq.h>
#include <bcm/cosq.h>
#include <bcm_int/sbx/error.h>
#include <bcm_int/sbx/port.h>
#include <shared/idxres_fl.h>

#ifdef BCM_CALADAN3_G3P1_SUPPORT

#include <soc/sbx/caladan3/ocm.h>
#include <soc/sbx/g3p1/g3p1_defs.h>
#include <soc/sbx/g3p1/g3p1.h>
#include <soc/sbx/g3p1/g3p1_tmu.h>
#include <soc/sbx/g3p1/g3p1_ppe_tables.h>
#include <bcm_int/sbx/caladan3/allocator.h>
#include <bcm_int/sbx/caladan3/l3.h>
#include <bcm_int/sbx/caladan3/g3p1.h>
#endif

#include <soc/sbx/caladan3/port.h>
#include <soc/sbx/caladan3/sws.h>
#include <soc/sbx/caladan3.h>
#include <bcm_int/sbx_dispatch.h>


/* Info on source queue to dequeue attachment */
/* This is the default queue association when ucode does not override the destination queue */
/* the attach_id */
typedef struct {
    int sq_id; /*   0-127 */
    int dq_id; /* 128-255 */
}bcm_c3_cosq_attach_info_t;

#define BCM_C3_COSQ_QUEUE_INVALID 0xBABABADD

#define BCM_C3_COSQ_SQUEUE_MIN   0
#define BCM_C3_COSQ_SQUEUE_MAX 127
#define BCM_C3_COSQ_DQUEUE_MIN 128
#define BCM_C3_COSQ_DQUEUE_MAX 255
#define BCM_C3_COSQ_IS_INGRESS_SQUEUE(qid) (((qid) >= SOC_SBX_CALADAN3_SWS_LINE_SQUEUE_BASE) && \
                                             ((qid) < SOC_SBX_CALADAN3_SWS_FABRIC_SQUEUE_BASE))
#define BCM_C3_COSQ_IS_INGRESS_DQUEUE(qid) (((qid) >= SOC_SBX_CALADAN3_SWS_LINE_DQUEUE_BASE) && \
                                             ((qid) <= BCM_C3_COSQ_DQUEUE_MAX))


typedef enum bcm_c3_cosq_queue_state_e {
    bcm_c3_cosq_queue_state_free=0, /* queue is free, disabled and not attached */
    bcm_c3_cosq_queue_state_allocated, /* queue is allocated and enabled but not attached to a corresponding sq/dq */
    bcm_c3_cosq_queue_state_attached, /* queue is enabled and attached to a corresponding sq/dq      */
    bcm_c3_cosq_queue_state_detached
} bcm_c3_cosq_queue_state_enum_t;

static char * _bcm_c3_cosq_state_str[] = {
    "free",
    "allocated",
    "attached",
    "detached",
    "not valid"
};

/* Info on the associated queue info */
typedef struct {
    bcm_port_t                     port;
    bcm_cos_t                      cos;
    int                            attach_id;
    bcm_c3_cosq_queue_state_enum_t state;
} bcm_c3_cosq_queue_state_t;



/* cosq state info */
typedef struct {
    bcm_c3_cosq_queue_state_t *sq;
    bcm_c3_cosq_queue_state_t *dq;
    bcm_c3_cosq_attach_info_t *attach_info;
    shr_idxres_list_handle_t   attach_id_pool;   
    uint8                      max_cos_level;
    uint8                      init_done; 
} bcm_c3_cosq_state_t;


#define COSQ_STATE(unit) (bcm_c3_cosq_state[unit])
#define BCM_C3_COSQ_ATTACH_ID_MAX (SOC_SBX_CALADAN3_SWS_MAX_INGRESS_QUEUES + 1)

/* Is COSQ module initialized? (bcm_caladan3_cosq_init) successful */
#define BCM_C3_COSQ_IS_INIT(unit)   ((bcm_c3_cosq_state[unit] != NULL) && (bcm_c3_cosq_state[unit]->init_done))

#define BCM_C3_COSQ_UNIT_CHECK(unit) \
    do { \
    if (((unit) < 0) || ((unit) >= BCM_LOCAL_UNITS_MAX)) { return BCM_E_UNIT; }  \
    } while (0)
    
static bcm_c3_cosq_state_t *bcm_c3_cosq_state[SOC_MAX_NUM_DEVICES];


/* function to move dqueue to 0-127 range for storage in dq[] state */
static int
_bcm_c3_cosq_dqueue_lower(int dq_id)
{
    int dq_adjust = dq_id;

    if (dq_id >= BCM_C3_COSQ_DQUEUE_MIN){ 
        dq_adjust = (dq_id - BCM_C3_COSQ_DQUEUE_MIN);
    }
    return dq_adjust;
}
/* function to move dqueue to 128-255 range for normal use */
static int
_bcm_c3_cosq_dqueue_raise(int dq_id)
{
    int dq_adjust = dq_id;

    if ((dq_id >= 0) && (dq_id < BCM_C3_COSQ_DQUEUE_MIN)) { 
        dq_adjust = (dq_id + BCM_C3_COSQ_DQUEUE_MIN);
    }
    return dq_adjust;
}

int soc_sbx_caladan3_remove_pr_icc_entries_for_queue(int unit, int q_to_rem);

STATIC int _bcm_c3_cosq_cleanup(int unit);
STATIC int _bcm_c3_cosq_is_initialized(int unit);
STATIC int _bcm_c3_cosq_attach_id_allocate(int unit, int *attach_id, int is_with_id);
STATIC int _bcm_c3_cosq_attach_id_is_allocated(int unit, int attach_id);
STATIC int _bcm_c3_cosq_attach_id_free(int unit, int attach_id);

/*
 *   Function
 *      bcm_caladan3_cosq_detach
 *   Purpose
 *      Free the cosq state.
 *
 *   Parameters
 *      (IN)  unit   : unit number of the device
 *   Returns
 *       BCM_E_NONE - success
 *       BCM_E_*    - failure
 *   Notes
 *       This function frees the bcm layer cosq resources.
 */
int
bcm_caladan3_cosq_detach(int unit)
{
    BCM_INIT_FUNC_DEFS;
    BCM_IF_ERR_EXIT(_bcm_c3_cosq_cleanup(unit));
 exit:
    BCM_FUNC_RETURN;
}
/*
 *   Function
 *      bcm_caladan3_cosq_init
 *   Purpose
 *      Initialize the cosq state
 *
 *   Parameters
 *      (IN)  unit   : unit number of the device
 *   Returns
 *       BCM_E_NONE - success
 *       BCM_E_*    - failure
 *   Notes
 *       This function calls the soc layer to determine the queues
 *       which are currently in use and stores them in it's internal state.
 *       The association between source queues destination queues and ports
 *       are constructed.  An attach_id is allocated per sq/dq pair.
 */
int bcm_caladan3_cosq_init(int unit)
{
    int                       rv = BCM_E_NONE;
    bcm_pbmp_t                pbmp;
    bcm_port_t                port, line_port, fabric_port;
    bcm_cos_t                 num_cos, num_fabric_cos;
    int                       sq_id, sq_id_line, sq_id_fabric;
    int                       dq_id, dq_id_line, dq_id_fabric;
    bcm_c3_cosq_queue_state_t *sq_line, *sq_fabric;
    bcm_c3_cosq_queue_state_t *dq_line, *dq_fabric;
    int                       attach_id;
    int                       cos;
    
    BCM_INIT_FUNC_DEFS;

    BCM_C3_COSQ_UNIT_CHECK(unit);
    
    if (BCM_C3_COSQ_IS_INIT(unit)) {
        BCM_IF_ERR_EXIT(bcm_caladan3_cosq_detach(unit));
    }

    num_cos = soc_property_get(unit, spn_BCM_NUM_COS, BCM_COS_COUNT);

    if (num_cos < 1) {
        num_cos = 1;
    } else if (NUM_COS(unit) && num_cos > NUM_COS(unit)) {
        num_cos = NUM_COS(unit);
    }

    NUM_COS(unit) = num_cos;

    BCM_IF_ERR_EXIT(bcm_sbx_cosq_config_set(unit, num_cos));

    /*
     * Create COSQ State - global COSQ state structure
     */
    COSQ_STATE(unit) = sal_alloc(sizeof(bcm_c3_cosq_state_t), "cosq_state");
    
    if (COSQ_STATE(unit) == NULL) {
        BCM_ERR_EXIT_MSG(BCM_E_MEMORY, (_BCM_MSG("unit %d, unable to allocate cosq_state"), unit));
    }
    
    sal_memset(COSQ_STATE(unit), 0, sizeof(bcm_c3_cosq_state_t));

    COSQ_STATE(unit)->max_cos_level = 8; 

    COSQ_STATE(unit)->sq = sal_alloc(sizeof(bcm_c3_cosq_queue_state_t)*SOC_SBX_CALADAN3_SWS_MAX_INGRESS_QUEUES, "cosq sq");
    
    if (COSQ_STATE(unit)->sq == NULL) {
        BCM_ERR_EXIT_MSG(BCM_E_MEMORY, (_BCM_MSG("unit %d, unable to allocate cosq sq state"), unit));
    }
    
    sal_memset(COSQ_STATE(unit)->sq, 0, sizeof(bcm_c3_cosq_queue_state_t)*SOC_SBX_CALADAN3_SWS_MAX_INGRESS_QUEUES);

    for (sq_id=BCM_C3_COSQ_SQUEUE_MIN; sq_id<BCM_C3_COSQ_SQUEUE_MAX; sq_id++) {
        COSQ_STATE(unit)->sq[sq_id].state = bcm_c3_cosq_queue_state_free; 
        COSQ_STATE(unit)->sq[sq_id].port = -1;
    }

    COSQ_STATE(unit)->dq = sal_alloc(sizeof(bcm_c3_cosq_queue_state_t)*SOC_SBX_CALADAN3_SWS_MAX_EGRESS_QUEUES, "cosq dq");
    
    if (COSQ_STATE(unit)->dq == NULL) {
        BCM_ERR_EXIT_MSG(BCM_E_MEMORY, (_BCM_MSG("unit %d, unable to allocate cosq dq state"), unit));
    }

    for (dq_id=BCM_C3_COSQ_DQUEUE_MIN; dq_id<BCM_C3_COSQ_DQUEUE_MAX; dq_id++) {
        COSQ_STATE(unit)->dq[_bcm_c3_cosq_dqueue_lower(dq_id)].state = bcm_c3_cosq_queue_state_free; 
        COSQ_STATE(unit)->dq[_bcm_c3_cosq_dqueue_lower(dq_id)].port = -1;
    }
    
    COSQ_STATE(unit)->attach_info = sal_alloc(sizeof(bcm_c3_cosq_attach_info_t)*BCM_C3_COSQ_ATTACH_ID_MAX , "cosq attach_info");
    
    if (COSQ_STATE(unit)->attach_info == NULL) {
        BCM_ERR_EXIT_MSG(BCM_E_MEMORY, (_BCM_MSG("unit %d, unable to allocate cosq attach_info state"), unit));
    }
    sal_memset(COSQ_STATE(unit)->attach_info, 0, sizeof(bcm_c3_cosq_attach_info_t)*BCM_C3_COSQ_ATTACH_ID_MAX);

    for (attach_id=0; attach_id < BCM_C3_COSQ_ATTACH_ID_MAX; attach_id++) {
        COSQ_STATE(unit)->attach_info[attach_id].sq_id = BCM_C3_COSQ_QUEUE_INVALID;
        COSQ_STATE(unit)->attach_info[attach_id].dq_id = BCM_C3_COSQ_QUEUE_INVALID;
    }

    /* Avoid using entry 0 */
    rv = shr_idxres_list_create(&COSQ_STATE(unit)->attach_id_pool,
                                1, BCM_C3_COSQ_ATTACH_ID_MAX,
                                0, BCM_C3_COSQ_ATTACH_ID_MAX,
                                "attach id pool");
    if (BCM_FAILURE(rv)) {
        BCM_ERR_EXIT_MSG(BCM_E_INTERNAL, (_BCM_MSG("unit %d, cosq attach_id_pool list creation failure"), unit));
    }

    /* loop through all current pbmp ports in the system. cosq must be after bcm port init */
    /* update local data base to get assocation between queues and ports */
    BCM_PBMP_ASSIGN(pbmp, PBMP_ALL(unit));

    BCM_PBMP_ITER(pbmp, port) {
        /* Skip ports not supported in certain environment (like SIM) */
        if (!SOC_PORT_VALID(unit, port) || (port >= SBX_MAX_PORTS)) {
            continue;
        }
        if (soc_sbx_caladan3_is_line_port(unit, port)) {

            line_port = port;

            rv = soc_sbx_caladan3_get_queues_from_port(unit, line_port, &sq_id_line, &dq_id_line, &num_cos);
        
            if (BCM_FAILURE(rv)) {
                BCM_ERR_EXIT_MSG(BCM_E_NOT_FOUND, 
                                 (_BCM_MSG("port(%d) exists but associated queues not found\n"), line_port));
            }

            LOG_INFO(BSL_LS_BCM_COSQ,
                     (BSL_META_U(unit,
                                 "line port(%2d) sq_id(%2d) dq_id(%d) num_cos(%2d)\n"),
             
                      line_port, sq_id_line, dq_id_line, num_cos));

            if (num_cos == 0) num_cos = 1;
        
            sq_line = &COSQ_STATE(unit)->sq[sq_id_line];

            dq_line = &COSQ_STATE(unit)->dq[_bcm_c3_cosq_dqueue_lower(dq_id_line)];

            for (cos = 0; cos<num_cos; cos++,sq_line++,dq_line++) {
                
                /* allocate line to fabric attach id */
                rv = _bcm_c3_cosq_attach_id_allocate(unit, &attach_id, FALSE /* not with_id */);            
                if (BCM_FAILURE(rv)) {
                    BCM_ERR_EXIT_MSG(BCM_E_RESOURCE, 
                                     (_BCM_MSG("unit(%d) cosq port(%d) attach_id allocation fails\n"), unit, port));
                }
                
                sq_line->port = line_port;
                sq_line->state = bcm_c3_cosq_queue_state_attached;
                sq_line->cos = cos;
                sq_line->attach_id = attach_id;
                
                dq_line->port = line_port;
                dq_line->cos = cos;

                rv = soc_sbx_caladan3_get_queues_from_fabric_port_info(unit, line_port, &sq_id_fabric,
                                                                       &dq_id_fabric, &num_fabric_cos, &fabric_port);

                sq_fabric = &COSQ_STATE(unit)->sq[sq_id_fabric];
                dq_fabric = &COSQ_STATE(unit)->dq[_bcm_c3_cosq_dqueue_lower(dq_id_fabric)];

                dq_fabric->attach_id = attach_id;
                dq_fabric->state = bcm_c3_cosq_queue_state_attached;
                COSQ_STATE(unit)->attach_info[attach_id].sq_id = sq_id_line;
                COSQ_STATE(unit)->attach_info[attach_id].dq_id = _bcm_c3_cosq_dqueue_raise(dq_id_fabric);

                LOG_INFO(BSL_LS_BCM_COSQ,
                         (BSL_META_U(unit,
                                     "line port(%2d) sq(%3d) fabric port(%d) dq(%3d) cos(%d) attach_id(%3d)\n"),
                 
                          line_port, sq_id_line, fabric_port, _bcm_c3_cosq_dqueue_raise(dq_id_fabric), cos, attach_id));

               /* allocate fabric to line attach id */
                rv = _bcm_c3_cosq_attach_id_allocate(unit, &attach_id, FALSE /* not with_id */);            
                if (BCM_FAILURE(rv)) {
                    BCM_ERR_EXIT_MSG(BCM_E_RESOURCE, 
                                     (_BCM_MSG("unit(%d) cosq port(%d) attach_id allocation fails\n"), unit, port));
                }
                
                sq_fabric->port = fabric_port;
                sq_fabric->state = bcm_c3_cosq_queue_state_attached;
                sq_fabric->cos = cos;
                sq_fabric->attach_id = attach_id;
                
                dq_fabric->port = fabric_port;
                dq_fabric->cos = cos;


                dq_line->attach_id = attach_id;
                dq_line->state = bcm_c3_cosq_queue_state_attached;

                COSQ_STATE(unit)->attach_info[attach_id].sq_id = sq_id_fabric;
                COSQ_STATE(unit)->attach_info[attach_id].dq_id = _bcm_c3_cosq_dqueue_raise(dq_id_line);

                LOG_INFO(BSL_LS_BCM_COSQ,
                         (BSL_META_U(unit,
                                     "fabric port(%2d) sq(%3d) line port(%d) dq(%3d) cos(%d) attach_id(%3d)\n"),
                 
                          fabric_port, sq_id_fabric, line_port, _bcm_c3_cosq_dqueue_raise(dq_id_line), cos, attach_id));

            }
        }
        
    }

    COSQ_STATE(unit)->init_done = TRUE;
    BCM_EXIT;
 exit:
    if (rv != BCM_E_NONE) {
        _bcm_c3_cosq_cleanup(unit);
    }
    BCM_FUNC_RETURN;

}

/*
 *   Function
 *      bcm_caladan3_cosq_gport_add
 *   Purpose
 *      Associate a pbmp port with a source queue or
 *      a destination queue.
 *
 *   Parameters
 *      (IN)  unit   : unit number of the device
 *   Returns
 *       BCM_E_NONE - success
 *       BCM_E_*    - failure
 *   Notes
 *       This function calls the soc layer to determine the queues
 *       which are currently in use and stores them in it's internal state.
 *       The association between source queues destination queues and ports
 *       are constructed.  An attach_id is allocated per sq/dq pair.
 */
int bcm_caladan3_cosq_gport_add(int unit, bcm_gport_t physical_port,
                                int num_cos_levels, uint32 flags, bcm_gport_t *req_gport)
{
    int rv = BCM_E_NONE;
    uint8 add_cos = FALSE;
    bcm_port_t port;
    int sq_id;
    int dq_id;
#ifdef DIRECTADD
    int type;
#endif

    BCM_INIT_FUNC_DEFS;

    BCM_C3_COSQ_UNIT_CHECK(unit);

    BCM_IF_ERR_EXIT(_bcm_c3_cosq_is_initialized(unit));

    /* validate parameters */
    if (!BCM_GPORT_IS_LOCAL(physical_port) && BCM_GPORT_IS_SET(physical_port)) {
        BCM_ERR_EXIT_MSG(BCM_E_PARAM, 
                         (_BCM_MSG("unit(%d) physical_port must be of type GPORT_LOCAL or a non gport type port\n"), unit));
    }

    if (BCM_COSQ_GPORT_ADD_COS & flags) {
        add_cos = TRUE;
    }

    if ((add_cos == FALSE) && (num_cos_levels>1)) {
        BCM_ERR_EXIT_MSG(BCM_E_PARAM, 
                         (_BCM_MSG("unit(%d) when BCM_COSQ_GPORT_ADD_COS flag is not set, num_cos_levels(%d) must be 1\n"), unit, num_cos_levels));
    }

    
    if (num_cos_levels > 1) {
        BCM_ERR_EXIT_MSG(BCM_E_UNAVAIL,
                         (_BCM_MSG("unit(%d) BCM_COSQ_GPORT_ADD_COS currently unsupported\n"), unit));
    }
    
    if (!(BCM_COSQ_GPORT_WITH_ID & flags)) {
        BCM_ERR_EXIT_MSG(BCM_E_PARAM, 
                         (_BCM_MSG("unit(%d) BCM_COSQ_GPORT_WITH_ID must be set req_gport type SRC_QUEUE or DST_QUEUE\n"), unit));
    }

    if (BCM_GPORT_IS_LOCAL(physical_port)) {
        port = BCM_GPORT_LOCAL_GET(physical_port);
    } else {
        port = physical_port;
    }

    if (req_gport == NULL) {
        BCM_ERR_EXIT_MSG(BCM_E_PARAM, 
                         (_BCM_MSG("unit(%d) req_gport null pointer passed\n"), unit));
    }

    if ((!BCM_COSQ_GPORT_IS_SRC_QUEUE(*req_gport)) && (!BCM_COSQ_GPORT_IS_DST_QUEUE(*req_gport))) {
        BCM_ERR_EXIT_MSG(BCM_E_PARAM, 
                         (_BCM_MSG("unit(%d) req_gport must be of type SRC_QUEUE or DST_QUEUE\n"), unit));

    }

    if (BCM_COSQ_GPORT_IS_SRC_QUEUE(*req_gport)) {

        sq_id = BCM_COSQ_GPORT_SRC_QUEUE_GET(*req_gport); 

        if ((sq_id < BCM_C3_COSQ_SQUEUE_MIN) || (sq_id > BCM_C3_COSQ_SQUEUE_MAX)) {
            BCM_ERR_EXIT_MSG(BCM_E_PARAM, (_BCM_MSG("gport_add squeue(%d) out of range (max<128)\n"), sq_id));
        }

        if (COSQ_STATE(unit)->sq[sq_id].state != bcm_c3_cosq_queue_state_free) {
            BCM_ERR_EXIT_MSG(BCM_E_RESOURCE, 
                             (_BCM_MSG("unit(%d) sq_id(%d) not free\n"), unit, sq_id));            
        } 
    
        /* allocate queue in soc layer */
        rv = soc_sbx_caladan3_sws_queue_alloc(unit, sq_id, num_cos_levels);
        if (BCM_FAILURE(rv)) {
            BCM_ERR_EXIT_MSG(BCM_E_RESOURCE, 
                             (_BCM_MSG("unit(%d) error(%d) allocating sq_id(%d)\n"), unit, rv, sq_id));  
        }
#ifdef DIRECTADD
        /* call soc layer function to update bmp */
        type = BCM_C3_COSQ_IS_INGRESS_SQUEUE(sq_id) ? INGRESS_SQUEUE : EGRESS_SQUEUE;
        rv = soc_sbx_caladan3_port_queues_add(unit, port, sq_id, type);
        if (BCM_FAILURE(rv)) {
            BCM_ERR_EXIT_MSG(BCM_E_PARAM, (_BCM_MSG("unit(%d) port_queues_add squeue failed qid(%d)\n"), unit, sq_id));
        }
#endif

        

        COSQ_STATE(unit)->sq[sq_id].state = bcm_c3_cosq_queue_state_allocated;
        COSQ_STATE(unit)->sq[sq_id].port = port;

    } else if (BCM_COSQ_GPORT_IS_DST_QUEUE(*req_gport)) {

        dq_id = BCM_COSQ_GPORT_DST_QUEUE_GET(*req_gport); 

        if ((dq_id < BCM_C3_COSQ_DQUEUE_MIN) || (dq_id > BCM_C3_COSQ_DQUEUE_MAX)) {
            BCM_ERR_EXIT_MSG(BCM_E_PARAM, (_BCM_MSG("gport_add dq_id(%d) out of range (128-256)\n"), dq_id));
        }

        if (COSQ_STATE(unit)->dq[_bcm_c3_cosq_dqueue_lower(dq_id)].state != bcm_c3_cosq_queue_state_free) {
            BCM_ERR_EXIT_MSG(BCM_E_RESOURCE, 
                             (_BCM_MSG("unit(%d) dest queue(%d) not free\n"), unit, dq_id));   
        } 
    
        /* allocate queue in soc layer */
        rv = soc_sbx_caladan3_sws_queue_alloc(unit, dq_id, num_cos_levels);
        if (BCM_FAILURE(rv)) {
            BCM_ERR_EXIT_MSG(BCM_E_RESOURCE, 
                             (_BCM_MSG("unit(%d) error(%d) allocating dq_id(%d)\n"), unit, rv, dq_id));  
        }
#ifdef DIRECTADD
        /* call soc layer function to update bmp */
        type = BCM_C3_COSQ_IS_INGRESS_DQUEUE(dq_id) ? INGRESS_DQUEUE : EGRESS_DQUEUE;
        rv = soc_sbx_caladan3_port_queues_add(unit, port, dq_id, type);
        if (BCM_FAILURE(rv)) {
            BCM_ERR_EXIT_MSG(BCM_E_PARAM, (_BCM_MSG("unit(%d) port_queues_add dqueue failed qid(%d)\n"), unit, dq_id));
        }
#endif

        

        COSQ_STATE(unit)->dq[_bcm_c3_cosq_dqueue_lower(dq_id)].state = bcm_c3_cosq_queue_state_allocated;
        COSQ_STATE(unit)->dq[_bcm_c3_cosq_dqueue_lower(dq_id)].port = port;
    }

    BCM_EXIT;
 exit:

    BCM_FUNC_RETURN;
}

int bcm_caladan3_cosq_gport_get(int unit, bcm_gport_t gport, bcm_gport_t *physical_port,
                                int *num_cos_levels, uint32 *flags)
{
    int rv  COMPILER_ATTRIBUTE((unused));
    int sq_id;
    int dq_id;
    bcm_c3_cosq_queue_state_t *sq;
    bcm_c3_cosq_queue_state_t *dq;
    BCM_INIT_FUNC_DEFS;

    BCM_C3_COSQ_UNIT_CHECK(unit);
    
    BCM_IF_ERR_EXIT(_bcm_c3_cosq_is_initialized(unit));
    
    rv = BCM_E_NONE;
    /* queue is of type src_queue or dest queue */
    if ((!BCM_COSQ_GPORT_IS_SRC_QUEUE(gport)) && (!BCM_COSQ_GPORT_IS_DST_QUEUE(gport))) {
        BCM_ERR_EXIT_MSG(BCM_E_PARAM, (_BCM_MSG("gport type must be BCM_COSQ_GPORT_TYPE_SRC_QUEUE or BCM_COSQ_GPORT_TYPE_DST_QUEUE\n")));
    }

    if (physical_port == NULL) {
        BCM_ERR_EXIT_MSG(BCM_E_PARAM, (_BCM_MSG("null pointer passed in for physical_port parameter\n")));
    }
    if (num_cos_levels == NULL) {
        BCM_ERR_EXIT_MSG(BCM_E_PARAM, (_BCM_MSG("null pointer passed in for num_cos_levels parameter\n")));
    }
    if (flags == NULL) {
        BCM_ERR_EXIT_MSG(BCM_E_PARAM, (_BCM_MSG("null pointer passed in for flags parameter\n")));
    }

    /* no flags returned */
    *flags = 0;
    *num_cos_levels = 0;
    *physical_port = -1;

    if (BCM_COSQ_GPORT_IS_SRC_QUEUE(gport)) {
        sq_id = BCM_COSQ_GPORT_SRC_QUEUE_GET(gport);

        if ((sq_id < BCM_C3_COSQ_SQUEUE_MIN) || (sq_id > BCM_C3_COSQ_SQUEUE_MAX)) {
            BCM_ERR_EXIT_MSG(BCM_E_PARAM, (_BCM_MSG("gport_get squeue(%d) out of range (max<128)\n"), sq_id));
        }

        sq = &COSQ_STATE(unit)->sq[sq_id];    

        if (sq->state != bcm_c3_cosq_queue_state_free) {
            *num_cos_levels = sq->cos;
            BCM_GPORT_LOCAL_SET(*physical_port, sq->port); 
            rv = BCM_E_NONE;

            LOG_INFO(BSL_LS_BCM_COSQ,
                     (BSL_META_U(unit,
                                 "%s attach_id(%d) src_queue(%d)\n"),
                      _bcm_c3_cosq_state_str[sq->state], sq->attach_id, sq_id));

        } else {
            BCM_ERR_EXIT_NO_MSG(BCM_E_RESOURCE);
        }
    } else {
        dq_id = BCM_COSQ_GPORT_DST_QUEUE_GET(gport);

        dq_id = _bcm_c3_cosq_dqueue_raise(dq_id);

        if ((dq_id < BCM_C3_COSQ_DQUEUE_MIN) || (dq_id > BCM_C3_COSQ_DQUEUE_MAX)) {
            BCM_ERR_EXIT_MSG(BCM_E_PARAM, (_BCM_MSG("gport_get dqueue(%d) out of range (128-255)\n"), dq_id));
        }

        dq = &COSQ_STATE(unit)->sq[_bcm_c3_cosq_dqueue_lower(dq_id)];    

        if (dq->state != bcm_c3_cosq_queue_state_free) {
            *num_cos_levels = dq->cos;
            BCM_GPORT_LOCAL_SET(*physical_port, dq->port); 
            rv = BCM_E_NONE;

            LOG_INFO(BSL_LS_BCM_COSQ,
                     (BSL_META_U(unit,
                                 "%s attach_id(%d) dst_queue(%d)\n"),
                      _bcm_c3_cosq_state_str[dq->state], dq->attach_id, dq_id));

        } else {
            BCM_ERR_EXIT_NO_MSG(BCM_E_RESOURCE);
        }
    }
    BCM_EXIT;
 exit:

    BCM_FUNC_RETURN;
}

int bcm_caladan3_cosq_gport_delete(int unit, bcm_gport_t gport) {
    int rv COMPILER_ATTRIBUTE((unused));
    int sq_id;
    int dq_id;

    BCM_INIT_FUNC_DEFS;
    
    BCM_C3_COSQ_UNIT_CHECK(unit);
    
    BCM_IF_ERR_EXIT(_bcm_c3_cosq_is_initialized(unit));
    
    /* queue is of type src_queue or dest queue */
    if ((!BCM_COSQ_GPORT_IS_SRC_QUEUE(gport)) && (!BCM_COSQ_GPORT_IS_DST_QUEUE(gport))) {
        BCM_ERR_EXIT_MSG(BCM_E_PARAM, (_BCM_MSG("gport type must be BCM_COSQ_GPORT_TYPE_SRC_QUEUE or BCM_COSQ_GPORT_TYPE_DST_QUEUE\n")));
    }

    if (BCM_COSQ_GPORT_IS_SRC_QUEUE(gport)) {

        sq_id = BCM_COSQ_GPORT_SRC_QUEUE_GET(gport);

        rv = bcm_c3_cosq_queue_delete(unit, sq_id);

    }  else if(BCM_COSQ_GPORT_IS_DST_QUEUE(gport)) {

        dq_id = BCM_COSQ_GPORT_DST_QUEUE_GET(gport);

        rv = bcm_c3_cosq_queue_delete(unit, _bcm_c3_cosq_dqueue_raise(dq_id));
    }

    BCM_EXIT;

 exit:
    BCM_FUNC_RETURN;
}

int bcm_caladan3_cosq_gport_queue_attach(int unit, uint32 flags, bcm_gport_t ingress_queue, 
                                         bcm_cos_t ingress_int_pri, bcm_gport_t egress_queue, 
                                         bcm_cos_t egress_int_pri, int *attach_id)
{
    int rv = BCM_E_NONE;
    int sq_id;
    int dq_id;
    int my_attach_id;

    BCM_INIT_FUNC_DEFS;

    BCM_C3_COSQ_UNIT_CHECK(unit);

    BCM_IF_ERR_EXIT(_bcm_c3_cosq_is_initialized(unit));

    /* validate parameters */

    
    if ((ingress_int_pri > 0) || (egress_int_pri > 0)) {
        BCM_ERR_EXIT_MSG(BCM_E_UNAVAIL,
                         (_BCM_MSG("unit(%d) multiple cos levels per queue not currently supported\n"), unit));
    }
    
    if (!(BCM_COSQ_GPORT_WITH_ID & flags)) {
        BCM_ERR_EXIT_MSG(BCM_E_PARAM, 
                         (_BCM_MSG("unit(%d) BCM_COSQ_GPORT_WITH_ID must be set queue type SRC_QUEUE or DST_QUEUE\n"), unit));
    }

    if (attach_id == NULL) {
        BCM_ERR_EXIT_MSG(BCM_E_PARAM, 
                         (_BCM_MSG("unit(%d) attach_id null pointer passed\n"), unit));
    }

    if ((!BCM_COSQ_GPORT_IS_SRC_QUEUE(ingress_queue)) && (!BCM_COSQ_GPORT_IS_DST_QUEUE(egress_queue))) {
        BCM_ERR_EXIT_MSG(BCM_E_PARAM, 
                         (_BCM_MSG("unit(%d) ingress/egress must be of type SRC_QUEUE and DST_QUEUE respectively\n"), unit));
    }


    sq_id = BCM_COSQ_GPORT_SRC_QUEUE_GET(ingress_queue); 

    if ((sq_id < BCM_C3_COSQ_SQUEUE_MIN) || (sq_id > BCM_C3_COSQ_SQUEUE_MAX)) {
        BCM_ERR_EXIT_MSG(BCM_E_PARAM, (_BCM_MSG("queue_attach squeue(%d) out of range (max<128)\n"), sq_id));
    }
    
    if ((COSQ_STATE(unit)->sq[sq_id].state != bcm_c3_cosq_queue_state_allocated) && 
        (COSQ_STATE(unit)->sq[sq_id].state != bcm_c3_cosq_queue_state_detached)) {
        BCM_ERR_EXIT_MSG(BCM_E_RESOURCE, 
                         (_BCM_MSG("unit(%d) sq_id(%d) not allocated or not in detached state\n"), unit, sq_id));            
    } 
    

    dq_id = BCM_COSQ_GPORT_DST_QUEUE_GET(egress_queue); 
    
    if ((dq_id < BCM_C3_COSQ_DQUEUE_MIN) || (dq_id > BCM_C3_COSQ_DQUEUE_MAX)) {
        BCM_ERR_EXIT_MSG(BCM_E_PARAM, (_BCM_MSG("queue_attach dq_id(%d) out of range (128-256)\n"), dq_id));
    }

    if ((COSQ_STATE(unit)->dq[_bcm_c3_cosq_dqueue_lower(dq_id)].state != bcm_c3_cosq_queue_state_allocated) && 
        (COSQ_STATE(unit)->dq[_bcm_c3_cosq_dqueue_lower(dq_id)].state != bcm_c3_cosq_queue_state_detached)) {
        BCM_ERR_EXIT_MSG(BCM_E_RESOURCE, 
                         (_BCM_MSG("unit(%d) dq_id(%d) not allocated or not in detached state\n"), unit, dq_id));            
    } 

    /* allocate attach_id */
    rv = _bcm_c3_cosq_attach_id_allocate(unit, &my_attach_id, FALSE /* not with_id */);            
    if (BCM_FAILURE(rv)) {
        BCM_ERR_EXIT_MSG(BCM_E_RESOURCE, 
                         (_BCM_MSG("unit(%d) cosq sq_id(%d) attach_id allocation fails\n"), unit, sq_id));
    }
    
    rv = soc_sbx_caladan3_sws_qm_dest_queue_enable(unit, dq_id, TRUE /* enable */);
    if (BCM_FAILURE(rv)) {
        BCM_ERR_EXIT_MSG(rv, (_BCM_MSG("enable dq_id(%d) failed\n"), dq_id));
    }


#if CALADAN3_ENABLE_SQUEUE_WHEN_ATTACH
    rv = soc_sbx_caladan3_sws_qm_src_queue_enable(unit, sq_id, TRUE /* enable */);
    if (BCM_FAILURE(rv)) {
        BCM_ERR_EXIT_MSG(rv, (_BCM_MSG("enable sq_id(%d) failed\n"), sq_id));
    }
#endif

    /* call soc layer function to write hpte map */
    rv = soc_sbx_caladan3_sws_hpte_map_setup(unit, sq_id, dq_id);
    if (BCM_FAILURE(rv)) {
        BCM_ERR_EXIT_MSG(rv, (_BCM_MSG("hpte map write failed\n")));
    }

    /* populate attach_id structure and update q state with attach_id */
    COSQ_STATE(unit)->attach_info[my_attach_id].sq_id = sq_id;
    COSQ_STATE(unit)->attach_info[my_attach_id].dq_id = dq_id;
    COSQ_STATE(unit)->sq[sq_id].state = bcm_c3_cosq_queue_state_attached;
    COSQ_STATE(unit)->sq[sq_id].attach_id = my_attach_id;
    COSQ_STATE(unit)->dq[_bcm_c3_cosq_dqueue_lower(dq_id)].state = bcm_c3_cosq_queue_state_attached;
    COSQ_STATE(unit)->dq[_bcm_c3_cosq_dqueue_lower(dq_id)].attach_id = my_attach_id;
    *attach_id = my_attach_id;

    BCM_EXIT;
 exit:

    BCM_FUNC_RETURN;

}

int bcm_caladan3_cosq_gport_queue_attach_get(int unit, bcm_gport_t ingress_queue, bcm_cos_t ingress_int_pri, 
                                             bcm_gport_t *egress_queue, bcm_cos_t *egress_int_pri, 
                                             int attach_id)
{
    int rv = BCM_E_NONE;
    int sq_id;
    bcm_c3_cosq_queue_state_t *sq;

    BCM_INIT_FUNC_DEFS;

    BCM_C3_COSQ_UNIT_CHECK(unit);

    BCM_IF_ERR_EXIT(_bcm_c3_cosq_is_initialized(unit));

    LOG_INFO(BSL_LS_BCM_COSQ,
             (BSL_META_U(unit,
                         "sq(0x%08x) attach_id(0x%08x) cos(%d)\n"),
              ingress_queue, attach_id, ingress_int_pri));

    if ((!egress_queue) || (!egress_int_pri)) {
        BCM_ERR_EXIT_MSG(BCM_E_PARAM, (_BCM_MSG("egress_queue/egress_int_pri pointers not passed in\n")));
    }

    /* ingress_queue is of type src_queue and queue is valid (0-127)*/
    if (!BCM_COSQ_GPORT_IS_SRC_QUEUE(ingress_queue)) {
        BCM_ERR_EXIT_MSG(BCM_E_PARAM, (_BCM_MSG("ingress_queue gport type unsupported\n")));
    }
    sq_id = BCM_COSQ_GPORT_SRC_QUEUE_GET(ingress_queue);
    if ((sq_id < BCM_C3_COSQ_SQUEUE_MIN) || (sq_id > BCM_C3_COSQ_SQUEUE_MAX)) {
        BCM_ERR_EXIT_MSG(BCM_E_PARAM, (_BCM_MSG("ingress_queue squeue(%d) out of range (max<128)\n"), sq_id));
    }
    /* ingress_int_pri is < max cos level */
    if ((ingress_int_pri < 0) || (ingress_int_pri > COSQ_STATE(unit)->max_cos_level)) {
        BCM_ERR_EXIT_MSG(BCM_E_PARAM, (_BCM_MSG("ingress_int_pri(%d) out of range\n"), ingress_int_pri));
    }
#define  NEED_UNTIL_PORTS_DONT_COME_UP_AT_STARTUP 1
    /* attach id is valid and in use */
    if ((attach_id >  BCM_C3_COSQ_ATTACH_ID_MAX) 
#ifdef NEED_UNTIL_PORTS_DONT_COME_UP_AT_STARTUP
        && (attach_id != -1)
#endif
        ) {
        BCM_ERR_EXIT_MSG(BCM_E_PARAM, (_BCM_MSG("attach_id(%d) out of range\n"), attach_id));
    }


#ifdef NEED_UNTIL_PORTS_DONT_COME_UP_AT_STARTUP
    /* If attach_id is -1, assume that the sq_id state has the correct attach_id - should only be used for deleting ports */
    if (attach_id != -1) {
#endif
        rv = _bcm_c3_cosq_attach_id_is_allocated(unit, attach_id);
        if (rv == FALSE) {
            BCM_ERR_EXIT_MSG(BCM_E_PARAM, (_BCM_MSG("attach_id(%d) not previously allocated internal state inconsistent\n"), attach_id));
        }

        /* use attach id as index and validate source queue matches */
        if (sq_id != COSQ_STATE(unit)->attach_info[attach_id].sq_id) {
            BCM_ERR_EXIT_MSG(BCM_E_PARAM, (_BCM_MSG("attach_id(%d) not associated with ingress queue(%d) internal state inconsistent\n"), attach_id, sq_id));
        }
#ifdef NEED_UNTIL_PORTS_DONT_COME_UP_AT_STARTUP
    }
#endif

    sq = &COSQ_STATE(unit)->sq[sq_id];
    if (!(sq->state == bcm_c3_cosq_queue_state_attached)) {

        LOG_WARN(BSL_LS_BCM_COSQ,
                 (BSL_META_U(unit,
                             "%s attach_id(%d) src_queue(%d)\n"),
                  _bcm_c3_cosq_state_str[sq->state], sq->attach_id, sq_id));

        BCM_ERR_EXIT_MSG(BCM_E_PARAM, (_BCM_MSG("ingress queue(%d) not attached\n"), sq_id));
    }


    BCM_COSQ_GPORT_DST_QUEUE_SET(*egress_queue, COSQ_STATE(unit)->attach_info[sq->attach_id].dq_id);
    *egress_int_pri = 0; /* unused */

    BCM_EXIT;
 exit:

    BCM_FUNC_RETURN;
}

int bcm_caladan3_cosq_gport_queue_detach(int unit, bcm_gport_t ingress_queue, bcm_cos_t ingress_int_pri, 
                                         int attach_id)
{
    int rv = BCM_E_NONE;
    int sq_id;
    int dq_id;
    bcm_c3_cosq_queue_state_t *sq;
    bcm_c3_cosq_queue_state_t *dq;

    BCM_INIT_FUNC_DEFS;

    BCM_C3_COSQ_UNIT_CHECK(unit);

    BCM_IF_ERR_EXIT(_bcm_c3_cosq_is_initialized(unit));


    /* ingress_queue is of type src_queue and queue is valid (0-127)*/
    if (!BCM_COSQ_GPORT_IS_SRC_QUEUE(ingress_queue)) {
        BCM_ERR_EXIT_MSG(BCM_E_PARAM, (_BCM_MSG("ingress_queue gport type unsupported\n")));
    }
    sq_id = BCM_COSQ_GPORT_SRC_QUEUE_GET(ingress_queue);
    if ((sq_id < BCM_C3_COSQ_SQUEUE_MIN) || (sq_id > BCM_C3_COSQ_SQUEUE_MAX)) {
        BCM_ERR_EXIT_MSG(BCM_E_PARAM, (_BCM_MSG("ingress_queue squeue(%d) out of range (max<128)\n"), sq_id));
    }
    /* ingress_int_pri is < max cos level */
    if ((ingress_int_pri < 0) || (ingress_int_pri > COSQ_STATE(unit)->max_cos_level)) {
        BCM_ERR_EXIT_MSG(BCM_E_PARAM, (_BCM_MSG("ingress_int_pri(%d) out of range\n"), ingress_int_pri));
    }
    /* attach id is valid and in use */
    if ((attach_id >  BCM_C3_COSQ_ATTACH_ID_MAX)
#ifdef NEED_UNTIL_PORTS_DONT_COME_UP_AT_STARTUP
        && (attach_id != -1)
#endif 
        ) {
        BCM_ERR_EXIT_MSG(BCM_E_PARAM, (_BCM_MSG("attach_id(%d) out of range\n"), attach_id));
    }

#ifdef NEED_UNTIL_PORTS_DONT_COME_UP_AT_STARTUP
    sq = &COSQ_STATE(unit)->sq[sq_id];
    attach_id = sq->attach_id;
#endif
    rv = _bcm_c3_cosq_attach_id_is_allocated(unit, attach_id);
    if (rv == FALSE) {
        BCM_ERR_EXIT_MSG(BCM_E_PARAM, (_BCM_MSG("attach_id(%d) not previously allocated internal state inconsistent\n"), attach_id));
    }


    /* use attach id as index and validate source queue matches */
    if (sq_id != COSQ_STATE(unit)->attach_info[attach_id].sq_id) {
        BCM_ERR_EXIT_MSG(BCM_E_PARAM, (_BCM_MSG("attach_id(%d) not associated with ingress queue(%d) internal state inconsistent\n"), attach_id, sq_id));
    }

    sq = &COSQ_STATE(unit)->sq[sq_id];
    if (!(sq->state == bcm_c3_cosq_queue_state_attached)) {
        BCM_ERR_EXIT_MSG(BCM_E_PARAM, (_BCM_MSG("ingress queue(%d) not attached\n"), sq_id));
    }

    /* call soc layer function to write hpte map */
    rv = soc_sbx_caladan3_sws_hpte_map_setup(unit, sq_id, 0 /* dq=0 */);
    if (BCM_FAILURE(rv)) {
        BCM_ERR_EXIT_MSG(rv, (_BCM_MSG("hpte map write failed\n")));
    }

    /* disable associated source queue in qm */
    rv = soc_sbx_caladan3_sws_qm_src_queue_enable(unit, sq_id, FALSE /* disable */);
    if (BCM_FAILURE(rv)) {
        BCM_ERR_EXIT_MSG(rv, (_BCM_MSG("hpte map write failed\n")));
    }
    /* mark as detached in cosq state */
    dq_id = COSQ_STATE(unit)->attach_info[attach_id].dq_id; 
    dq = &COSQ_STATE(unit)->dq[_bcm_c3_cosq_dqueue_lower(dq_id)];
    dq->state = bcm_c3_cosq_queue_state_detached;

    sq->state = bcm_c3_cosq_queue_state_detached;
    LOG_INFO(BSL_LS_BCM_COSQ,
             (BSL_META_U(unit,
                         "sq(0x%08x) attach_id(0x%08x) cos(%d) dq(%d) detached\n"),
              ingress_queue, attach_id, ingress_int_pri, dq_id));

    /* Do not free attach_id until delete */

    BCM_EXIT;
 exit:

    BCM_FUNC_RETURN;
}

/*************************************************************
 * Helper functions to be called by port or stack modules 
 *************************************************************/
int bcm_c3_cosq_info_dump(int unit)
{
    int rv = BCM_E_NONE;
    bcm_pbmp_t pbmp;
    bcm_port_t port, fabric_port, line_port;
    bcm_c3_cosq_port_queues_t *port_queues;
    int sq_id;
    int dq_id;
    bcm_c3_cosq_queue_state_t *sq;
    bcm_c3_cosq_queue_state_t *dq;
    bcm_c3_cosq_queue_state_enum_t state;
    int attach_id;
    int num_qs;

    BCM_INIT_FUNC_DEFS;
    BCM_C3_COSQ_UNIT_CHECK(unit);
    
    port_queues = sal_alloc(sizeof(bcm_c3_cosq_port_queues_t)*SBX_MAX_PORTS*2 , "temporary port queue state");
    if (port_queues == NULL) {
        BCM_ERR_EXIT_MSG(rv, (_BCM_MSG("Failed to allocate temporary port queue state\n")));
    }
    
    LOG_CLI((BSL_META_U(unit,
                        "\n+----------------------------------------------------------------------------+")));
    LOG_CLI((BSL_META_U(unit,
                        "\n|             Line                      |            Fabric                  |")));
    LOG_CLI((BSL_META_U(unit,
                        "\n+----------------------------------------------------------------------------+")));
    LOG_CLI((BSL_META_U(unit,
                        "\n| Port type    SQ   state   attach_id   |   Port  type   DQ    state         |")));
    LOG_CLI((BSL_META_U(unit,
                        "\n+----------------------------------------------------------------------------+")));

    BCM_PBMP_ASSIGN(pbmp, PBMP_ALL(unit));
    
    BCM_PBMP_ITER(pbmp, port) {

        num_qs = 0;

        rv = bcm_c3_cosq_queues_from_port_get(unit, port, &port_queues[port]);
        if (BCM_FAILURE(rv)) {
            BCM_ERR_EXIT_MSG(rv, (_BCM_MSG("error getting queue info from port\n")));
        }

        if (soc_sbx_caladan3_is_line_port(unit, port)) {

            SOC_PBMP_ITER(port_queues[port].sq_bmp, sq_id) {
                
                num_qs++;
                
                sq = &COSQ_STATE(unit)->sq[sq_id];
                state = sq->state;
                
                LOG_CLI((BSL_META_U(unit,
                                    "\n| %3d(%4s) %3d(%2x) %9s    %3d    |"),
                         port, 
                         SOC_PORT_NAME(unit, port),
                         sq_id,
                         sq_id,
                         _bcm_c3_cosq_state_str[state],
                         sq->attach_id));
                
                if (state == bcm_c3_cosq_queue_state_attached) {
                    attach_id = sq->attach_id;
                    dq_id = COSQ_STATE(unit)->attach_info[attach_id].dq_id;
                    
                    dq = &COSQ_STATE(unit)->dq[_bcm_c3_cosq_dqueue_lower(dq_id)];
                    state = dq->state;
                    fabric_port = dq->port;
                    
                   LOG_CLI((BSL_META_U(unit,
                                       " %3d(%4s) %3d(%2x) %9s        |"),
                            fabric_port, 
                            SOC_PORT_NAME(unit, fabric_port),
                            dq_id,
                            dq_id,
                            _bcm_c3_cosq_state_str[state]));
                } else {
                    LOG_CLI((BSL_META_U(unit,
                                        "                                    |")));
                }            
            }
            if (num_qs == 0) {
                LOG_CLI((BSL_META_U(unit,
                                    "\n| %3d(%4s)  no sqs                                                          |"),
                         port,
                         SOC_PORT_NAME(unit, port)));
            }
        }
    }
    
    LOG_CLI((BSL_META_U(unit,
                        "\n+----------------------------------------------------------------------------+\n")));
    LOG_CLI((BSL_META_U(unit,
                        "\n+----------------------------------------------------------------------------+")));
    LOG_CLI((BSL_META_U(unit,
                        "\n|             Fabric                    |             Line                   |")));
    LOG_CLI((BSL_META_U(unit,
                        "\n+----------------------------------------------------------------------------+")));
    LOG_CLI((BSL_META_U(unit,
                        "\n| Port type    SQ   state   attach_id   |   Port  type   DQ    state         |")));
    LOG_CLI((BSL_META_U(unit,
                        "\n+----------------------------------------------------------------------------+")));


    BCM_PBMP_ITER(pbmp, port) {

        num_qs = 0;

        rv = bcm_c3_cosq_queues_from_port_get(unit, port, &port_queues[port]);
        if (BCM_FAILURE(rv)) {
            BCM_ERR_EXIT_MSG(rv, (_BCM_MSG("error getting queue info from port\n")));
        }

        /* get the fabric queues */
        if (!soc_sbx_caladan3_is_line_port(unit, port)) {

            SOC_PBMP_ITER(port_queues[port].sq_bmp, sq_id) {
                
                num_qs++;
                
                sq = &COSQ_STATE(unit)->sq[sq_id];
                state = sq->state;
                
                LOG_CLI((BSL_META_U(unit,
                                    "\n| %3d(%4s) %3d(%2x) %9s    %3d    |"),
                         port, 
                         SOC_PORT_NAME(unit, port),
                         sq_id,
                         sq_id,
                         _bcm_c3_cosq_state_str[state],
                         sq->attach_id));
                
                if (state == bcm_c3_cosq_queue_state_attached) {
                    attach_id = sq->attach_id;
                    dq_id = COSQ_STATE(unit)->attach_info[attach_id].dq_id;
                    
                    dq = &COSQ_STATE(unit)->dq[_bcm_c3_cosq_dqueue_lower(dq_id)];
                    state = dq->state;
                    line_port = dq->port;
                    
                   LOG_CLI((BSL_META_U(unit,
                                       " %3d(%4s) %3d(%2x) %9s        |"),
                            line_port, 
                            SOC_PORT_NAME(unit, line_port),
                            dq_id,
                            dq_id,
                            _bcm_c3_cosq_state_str[state]));
                } else {
                    LOG_CLI((BSL_META_U(unit,
                                        "                                    |")));
                }            
            }
            if (num_qs == 0) {
                LOG_CLI((BSL_META_U(unit,
                                    "\n| %3d(%4s)  no sqs                                                          |"),
                         port,
                         SOC_PORT_NAME(unit, port)));
            }
        }
    }
    
    LOG_CLI((BSL_META_U(unit,
                        "\n+----------------------------------------------------------------------------+\n")));

    for (sq_id=BCM_C3_COSQ_SQUEUE_MIN; sq_id<=BCM_C3_COSQ_SQUEUE_MAX; sq_id++) {
        rv = bcm_c3_cosq_dest_port_from_sq_get(unit, sq_id, &port, &dq_id);
        if (BCM_FAILURE(rv) && (rv != BCM_E_RESOURCE)) {
            BCM_ERR_EXIT_MSG(rv, (_BCM_MSG("error getting dest port from sq\n")));       
        } else if (rv != BCM_E_RESOURCE){
            LOG_CLI((BSL_META_U(unit,
                                "\n| sq(%3d) to dst port %3d(%4s)"), sq_id, port, SOC_PORT_NAME(unit, port)));
        }
    }
    LOG_CLI((BSL_META_U(unit,
                        "\n+----------------------------------------------------------------------------+\n")));
    for (dq_id=BCM_C3_COSQ_DQUEUE_MIN; dq_id<=BCM_C3_COSQ_DQUEUE_MAX; dq_id++) {
        rv = bcm_c3_cosq_src_port_from_dq_get(unit, dq_id, &port, &sq_id);
        if (BCM_FAILURE(rv) && (rv != BCM_E_RESOURCE)) {
            BCM_ERR_EXIT_MSG(rv, (_BCM_MSG("error getting src port from dq\n")));       
        } else if (rv != BCM_E_RESOURCE){
            LOG_CLI((BSL_META_U(unit,
                                "\n| src port %3d(%4s) to dq(%3d)"), port, SOC_PORT_NAME(unit, port), dq_id));
        }
    }
    LOG_CLI((BSL_META_U(unit,
                        "\n+----------------------------------------------------------------------------+\n")));

    BCM_EXIT;
 exit:
    if (port_queues) {
        sal_free(port_queues);
    }
    BCM_FUNC_RETURN;
    
}

/* given a pbmp port, return the local sq and local dq associated with that port */
int bcm_c3_cosq_queues_from_port_get(int unit, bcm_port_t port, bcm_c3_cosq_port_queues_t *port_queues)
{
    int sq_id;
    int dq_id;
    bcm_c3_cosq_queue_state_t *sq;
    bcm_c3_cosq_queue_state_t *dq;

    BCM_INIT_FUNC_DEFS;

    BCM_C3_COSQ_UNIT_CHECK(unit);

    BCM_IF_ERR_EXIT(_bcm_c3_cosq_is_initialized(unit));

    if (port_queues == NULL) {
        BCM_ERR_EXIT_MSG(BCM_E_PARAM, (_BCM_MSG("port_queues pointer not passed\n")));
    }
    if (port >= SBX_MAX_PORTS) {
        BCM_ERR_EXIT_MSG(BCM_E_PARAM, (_BCM_MSG("port(%d) out of range (max<%d)\n"), port, SBX_MAX_PORTS));
    }

    SOC_PBMP_CLEAR(port_queues->sq_bmp);

    for (sq_id=BCM_C3_COSQ_SQUEUE_MIN; sq_id<BCM_C3_COSQ_SQUEUE_MAX; sq_id++) {
        sq = &COSQ_STATE(unit)->sq[sq_id];
        if ((sq->state !=  bcm_c3_cosq_queue_state_free) && (sq->port == port)) {
            SOC_PBMP_PORT_ADD(port_queues->sq_bmp, sq_id);
        } else {
            SOC_PBMP_PORT_REMOVE(port_queues->sq_bmp, sq_id);
        }
    }

    SOC_PBMP_CLEAR(port_queues->dq_bmp);

    for (dq_id=BCM_C3_COSQ_DQUEUE_MIN; dq_id<BCM_C3_COSQ_DQUEUE_MAX; dq_id++) {
        dq = &COSQ_STATE(unit)->dq[_bcm_c3_cosq_dqueue_lower(dq_id)];
        if ((dq->state !=  bcm_c3_cosq_queue_state_free) && (dq->port == port)) {
            SOC_PBMP_PORT_ADD(port_queues->dq_bmp, _bcm_c3_cosq_dqueue_lower(dq_id));
        } else {
            SOC_PBMP_PORT_REMOVE(port_queues->dq_bmp, _bcm_c3_cosq_dqueue_lower(dq_id));
        }
    }
    BCM_EXIT;
 exit:
    BCM_FUNC_RETURN;
}

/* returns a destination port given a source queue */
int bcm_c3_cosq_dest_port_from_sq_get(int unit, int sq_id, bcm_port_t *dest_port, int *dest_dq_id) {
    int dq_id;
    bcm_c3_cosq_queue_state_t *sq;
    bcm_c3_cosq_queue_state_t *dq;
    int attach_id;
    int rv = BCM_E_NONE;

    BCM_INIT_FUNC_DEFS;

    BCM_C3_COSQ_UNIT_CHECK(unit);

    BCM_IF_ERR_EXIT(_bcm_c3_cosq_is_initialized(unit));

    /* validate parameters */
    if ((sq_id<BCM_C3_COSQ_SQUEUE_MIN) || (sq_id>BCM_C3_COSQ_SQUEUE_MAX)) {
        BCM_ERR_EXIT_MSG(BCM_E_PARAM, (_BCM_MSG("sq_id(%d) out of range (0<dq_id<128)\n"), sq_id));
    }

    if (dest_port == NULL) {
        BCM_ERR_EXIT_MSG(BCM_E_PARAM, (_BCM_MSG("dest_port pointer null\n")));
    }

    if (dest_dq_id == NULL) {
        BCM_ERR_EXIT_MSG(BCM_E_PARAM, (_BCM_MSG("dest_dq_id pointer null\n")));
    }

    /* get attach id of dq_id */
    sq = &COSQ_STATE(unit)->sq[sq_id];

    if (sq->state != bcm_c3_cosq_queue_state_attached) {
        BCM_ERR_EXIT_NO_MSG(BCM_E_RESOURCE);
    }

    /* get relevant sq_id from attach_id */
    attach_id = sq->attach_id;

    /* validate that attach id is allocated */
    rv = _bcm_c3_cosq_attach_id_is_allocated(unit, attach_id);
    if (rv == FALSE) {
        BCM_ERR_EXIT_MSG(BCM_E_INTERNAL, 
                         (_BCM_MSG("attach_id(%d) not alloc state inconsistent\n"), attach_id));
    }

    dq_id = COSQ_STATE(unit)->attach_info[attach_id].dq_id;
    dq = &COSQ_STATE(unit)->dq[_bcm_c3_cosq_dqueue_lower(dq_id)];

    if (dq->state != bcm_c3_cosq_queue_state_attached) {
        LOG_WARN(BSL_LS_BCM_COSQ,
                 (BSL_META_U(unit,
                             "%s attach_id(%d) dq_id(%d) not attached!\n"), 
                  _bcm_c3_cosq_state_str[dq->state], dq->attach_id, dq_id));
    }

    /* get port associated with dq_id */
    *dest_port = dq->port;
    *dest_dq_id = dq_id;
    BCM_EXIT;
 exit:
    BCM_FUNC_RETURN;
}


/* returns a source port given a destination queue */
int bcm_c3_cosq_src_port_from_dq_get(int unit, int dq_id, bcm_port_t *src_port, int *src_sq_id) {
    int sq_id;
    bcm_c3_cosq_queue_state_t *sq;
    bcm_c3_cosq_queue_state_t *dq;
    int attach_id;
    int rv = BCM_E_NONE;

    BCM_INIT_FUNC_DEFS;

    BCM_C3_COSQ_UNIT_CHECK(unit);

    BCM_IF_ERR_EXIT(_bcm_c3_cosq_is_initialized(unit));

    /* validate parameters */
    if ((dq_id<BCM_C3_COSQ_DQUEUE_MIN) || (dq_id>BCM_C3_COSQ_DQUEUE_MAX)) {
        BCM_ERR_EXIT_MSG(BCM_E_PARAM, (_BCM_MSG("dq_id(%d) out of range (128<dq_id<256)\n"), dq_id));
    }

    if (src_port == NULL) {
        BCM_ERR_EXIT_MSG(BCM_E_PARAM, (_BCM_MSG("port pointer null\n")));
    }

    if (src_sq_id == NULL) {
        BCM_ERR_EXIT_MSG(BCM_E_PARAM, (_BCM_MSG("src_sq_id pointer null\n")));
    }

    /* get attach id of dq_id */
    dq = &COSQ_STATE(unit)->dq[_bcm_c3_cosq_dqueue_lower(dq_id)];

    if (dq->state != bcm_c3_cosq_queue_state_attached) {
        BCM_ERR_EXIT_NO_MSG(BCM_E_RESOURCE);
    }

    /* get relevant sq_id from attach_id */
    attach_id = dq->attach_id;

    /* validate that attach id is allocated */
    rv = _bcm_c3_cosq_attach_id_is_allocated(unit, attach_id);
    if (rv == FALSE) {
        BCM_ERR_EXIT_MSG(BCM_E_INTERNAL, 
                         (_BCM_MSG("attach_id(%d) not alloc state inconsistent\n"), attach_id));
    }

    sq_id = COSQ_STATE(unit)->attach_info[attach_id].sq_id;
    sq = &COSQ_STATE(unit)->sq[sq_id];

    if (sq->state != bcm_c3_cosq_queue_state_attached) {
        LOG_WARN(BSL_LS_BCM_COSQ,
                 (BSL_META_U(unit,
                             "%s attach_id(%d) sq_id(%d) not attached!\n"), 
                  _bcm_c3_cosq_state_str[sq->state], sq->attach_id, sq_id));
    }

    /* get port associated with sq_id */
    *src_port = sq->port;
    *src_sq_id = sq_id;
    BCM_EXIT;
 exit:
    BCM_FUNC_RETURN;
}

/* source queue must be in the range 0-127 and dest queue 128-255 */
int bcm_c3_cosq_queue_delete(int unit, int queue)
{
    int rv = BCM_E_NONE;
    int sq_id;
    int dq_id;
    bcm_c3_cosq_queue_state_t *sq;
    bcm_c3_cosq_queue_state_t *dq;
    int attach_id;
    int type;

    BCM_INIT_FUNC_DEFS;

    BCM_C3_COSQ_UNIT_CHECK(unit);

    if ((queue < 0) || (queue > SOC_SBX_CALADAN3_SWS_MAX_QUEUE_ID)) {
        BCM_ERR_EXIT_MSG(BCM_E_PARAM, (_BCM_MSG("queue(%d) out of range (max<256)\n"), queue));
    }

    if ((queue >= BCM_C3_COSQ_SQUEUE_MIN) && (queue <=  BCM_C3_COSQ_SQUEUE_MAX)) {

        sq_id = queue;

        sq = &COSQ_STATE(unit)->sq[sq_id];

        attach_id = sq->attach_id;
 
        rv = _bcm_c3_cosq_attach_id_is_allocated(unit, attach_id);
        if (rv == FALSE) {
            BCM_ERR_EXIT_MSG(BCM_E_PARAM, (_BCM_MSG("attach_id(%d) not previously allocated internal state inconsistent\n"), attach_id));
        }

        /* call soc layer function to write hpte map */
        rv = soc_sbx_caladan3_sws_hpte_map_setup(unit, sq_id, 0 /* dq=0 */);
        if (BCM_FAILURE(rv)) {
            BCM_ERR_EXIT_MSG(rv, (_BCM_MSG("hpte map write failed\n")));
        }

        /* disable source queue in qm */
        rv = soc_sbx_caladan3_sws_qm_src_queue_enable(unit, sq_id, FALSE /* disable */);
        if (BCM_FAILURE(rv)) {
            BCM_ERR_EXIT_MSG(rv, (_BCM_MSG("src queue disable failed\n")));
        }

        /* Disable the squeue in the PR_ICC */
        rv = soc_sbx_caladan3_remove_pr_icc_entries_for_queue(unit, sq_id);
        if (BCM_FAILURE(rv)) {
            BCM_ERR_EXIT_MSG(rv, (_BCM_MSG("src queue delete from PR_ICC failed\n")));
        }

        rv = soc_sbx_caladan3_sws_queue_free(unit, sq_id, 1);
        if (BCM_FAILURE(rv)) {
            BCM_ERR_EXIT_MSG(rv, (_BCM_MSG("src queue delete failed\n")));
        }

        /* call soc layer function to update bmp */
        type = BCM_C3_COSQ_IS_INGRESS_SQUEUE(sq_id) ? INGRESS_SQUEUE : EGRESS_SQUEUE;
        rv = soc_sbx_caladan3_port_queues_remove(unit, sq->port, sq_id, type);
        if (BCM_FAILURE(rv)) {
            BCM_ERR_EXIT_MSG(BCM_E_PARAM, (_BCM_MSG("attach_id(%d) port_queues_remove failed\n"), attach_id));
        }


        sq->state = bcm_c3_cosq_queue_state_free;
        COSQ_STATE(unit)->attach_info[attach_id].sq_id = BCM_C3_COSQ_QUEUE_INVALID;

        if (COSQ_STATE(unit)->attach_info[attach_id].dq_id == BCM_C3_COSQ_QUEUE_INVALID) {
            rv = _bcm_c3_cosq_attach_id_free(unit, attach_id);
            if (BCM_FAILURE(rv)) {
                BCM_ERR_EXIT_MSG(BCM_E_PARAM, (_BCM_MSG("attach_id(%d) free error for sq(%d)\n"), attach_id, sq_id));
            }
        }
        LOG_INFO(BSL_LS_BCM_COSQ,
                 (BSL_META_U(unit,
                             "sq(0x%08x) deleted\n"),
                  sq_id));

    }  else {

        dq_id = queue;

        /* coverity[dead_error_begin] */
        if ((dq_id < BCM_C3_COSQ_DQUEUE_MIN) || (dq_id > BCM_C3_COSQ_DQUEUE_MAX)) {
            BCM_ERR_EXIT_MSG(BCM_E_PARAM, (_BCM_MSG("egress_queue dqueue(%d) out of range (128-255\n"), dq_id));
        }

        dq = &COSQ_STATE(unit)->dq[_bcm_c3_cosq_dqueue_lower(dq_id)];

        attach_id = dq->attach_id;
 
        rv = _bcm_c3_cosq_attach_id_is_allocated(unit, attach_id);
        if (rv == FALSE) {
            BCM_ERR_EXIT_MSG(BCM_E_PARAM, (_BCM_MSG("attach_id(%d) not previously allocated internal state inconsistent\n"), attach_id));
        }

        sq_id = COSQ_STATE(unit)->attach_info[attach_id].sq_id;

        if (sq_id != BCM_C3_COSQ_QUEUE_INVALID) {


            /* call soc layer function to write hpte map */
            rv = soc_sbx_caladan3_sws_hpte_map_setup(unit, sq_id, 0 /* dq=0 */);
            if (BCM_FAILURE(rv)) {
                BCM_ERR_EXIT_MSG(rv, (_BCM_MSG("hpte map write failed\n")));
            }
#ifdef SINGLE_SQ_PER_DQ
            /* disable associated source queue in qm */
            rv = soc_sbx_caladan3_sws_qm_src_queue_enable(unit, sq_id, FALSE /* disable */);
            if (BCM_FAILURE(rv)) {
                BCM_ERR_EXIT_MSG(rv, (_BCM_MSG("src queue disable failed\n")));
            }
#endif
        }

        /* disable dest queue in qm */
        rv = soc_sbx_caladan3_sws_qm_dest_queue_enable(unit, dq_id, FALSE /* disable */);
        if (BCM_FAILURE(rv)) {
            BCM_ERR_EXIT_MSG(rv, (_BCM_MSG("dst queue disable failed\n")));
        }

        rv = soc_sbx_caladan3_sws_queue_free(unit, dq_id, 1);
        if (BCM_FAILURE(rv)) {
            BCM_ERR_EXIT_MSG(rv, (_BCM_MSG("dest queue delete failed\n")));
        }

        /* call soc layer function to update bmp */
        if (sq_id != BCM_C3_COSQ_QUEUE_INVALID) {
            type = BCM_C3_COSQ_IS_INGRESS_DQUEUE(dq_id) ? INGRESS_DQUEUE : EGRESS_DQUEUE;
            rv = soc_sbx_caladan3_port_queues_remove(unit, dq->port, dq_id, type);
            if (BCM_FAILURE(rv)) {
                BCM_ERR_EXIT_MSG(BCM_E_PARAM, (_BCM_MSG("attach_id(%d) port_queues_remove egress failed\n"), attach_id));
            }
        }

        dq->state = bcm_c3_cosq_queue_state_free;
        COSQ_STATE(unit)->attach_info[attach_id].dq_id = BCM_C3_COSQ_QUEUE_INVALID;

        if (COSQ_STATE(unit)->attach_info[attach_id].sq_id == BCM_C3_COSQ_QUEUE_INVALID) {
            rv = _bcm_c3_cosq_attach_id_free(unit, attach_id);
            if (BCM_FAILURE(rv)) {
                BCM_ERR_EXIT_MSG(rv, (_BCM_MSG("attach_id(%d) free error for dq(%d)\n"), attach_id, dq_id));
            }
        }
        LOG_INFO(BSL_LS_BCM_COSQ,
                 (BSL_META_U(unit,
                             "dq(0x%08x) deleted\n"),
                  dq_id));
    }


    BCM_EXIT;

 exit:
    BCM_FUNC_RETURN;
}

/******************** 
 * Internal Routines 
 ********************/

/*  Verify that bcm_cosq_init() has successfully been run prior to the current function call */
STATIC int _bcm_c3_cosq_is_initialized(int unit)
{
    int rv = BCM_E_NONE;

    BCM_INIT_FUNC_DEFS;

    switch(SOC_SBX_CONTROL(unit)->ucodetype) {
#ifdef BCM_CALADAN3_SUPPORT
    case SOC_SBX_UCODE_TYPE_G3P1:
        if (!BCM_C3_COSQ_IS_INIT(unit)) {
            rv = BCM_E_INIT;
            BCM_ERR_EXIT_MSG(rv, (_BCM_MSG("unit %d, caladan3 cosq not initialized\n"), unit));
        }
        break;
#endif
    default:
        SBX_UNKNOWN_UCODE_WARN(unit);
        rv = BCM_E_CONFIG;
        BCM_EXIT;
    }
 exit:
    BCM_FUNC_RETURN;
}

STATIC int _bcm_c3_cosq_cleanup(int unit)
{
    BCM_INIT_FUNC_DEFS;

    if (COSQ_STATE(unit)->sq) {
        sal_free(COSQ_STATE(unit)->sq);
    }

    if (COSQ_STATE(unit)->dq) {
        sal_free(COSQ_STATE(unit)->dq);
    }

    if (COSQ_STATE(unit)->attach_info) {
        sal_free(COSQ_STATE(unit)->attach_info);
    }
    if (COSQ_STATE(unit)->attach_id_pool) {
        shr_idxres_list_destroy(COSQ_STATE(unit)->attach_id_pool);
        COSQ_STATE(unit)->attach_id_pool = 0;
    }

    sal_free(COSQ_STATE(unit));
    COSQ_STATE(unit) = NULL;
    BCM_EXIT;
 exit:
    BCM_FUNC_RETURN;
}


/* Returns true if allocated, false if not */
STATIC int _bcm_c3_cosq_attach_id_is_allocated(int unit, int attach_id)
{
    int rv = BCM_E_NONE;
    
    /* check if the requested ID has already been allocated. */
    rv = shr_idxres_list_elem_state(COSQ_STATE(unit)->attach_id_pool, attach_id);
    if (rv == BCM_E_NOT_FOUND) {
        return FALSE;
    } else {
        return TRUE;
    }
}

STATIC int _bcm_c3_cosq_attach_id_allocate(int unit, int *attach_id, int is_with_id)
{
    int rv = BCM_E_NONE;
    int id_is_allocated = TRUE;

    BCM_INIT_FUNC_DEFS;

    if (is_with_id == TRUE) {
        
        /* check if the requested ID has already been allocated. */
        id_is_allocated = _bcm_c3_cosq_attach_id_is_allocated(unit, *attach_id);
        
        if (id_is_allocated == FALSE) {
            /* reserve the requested endpoint-id */
            rv = shr_idxres_list_reserve(COSQ_STATE(unit)->attach_id_pool, *attach_id, *attach_id);
            
            if (rv != BCM_E_NONE) {
                BCM_ERR_EXIT_MSG(rv,(_BCM_MSG("attach_id(%d) could not be reserved\n"), *attach_id));
            }     
        } else {
            rv = BCM_E_RESOURCE;
            BCM_ERR_EXIT_MSG(rv,(_BCM_MSG("attach_id(%d) already allocated\n"), *attach_id));
        }
        
    } else { /* is_with_id == FALSE */
        rv = shr_idxres_list_alloc(COSQ_STATE(unit)->attach_id_pool, (uint32*)attach_id);
        if (BCM_FAILURE(rv)) {
            BCM_ERR_EXIT_MSG(rv,(_BCM_MSG("attach_id(%d) could not be allocated\n"), *attach_id));
        }
    }
 exit:
    BCM_FUNC_RETURN;
}

STATIC int _bcm_c3_cosq_attach_id_free(int unit, int attach_id)
{
  int rv COMPILER_ATTRIBUTE((unused));
    BCM_INIT_FUNC_DEFS;
    rv = shr_idxres_list_free(COSQ_STATE(unit)->attach_id_pool, attach_id);

    BCM_FUNC_RETURN;
}


/*
Currently queue depths are managed by the TDM specific header files.
Add bcm_cosq_gport_size_set() bcm_cosq_gport_size_get() to
set the min_pages_data, min_pages_header and max_pages.
*/

int bcm_caladan3_cosq_src_queue_get(int unit, int queue, uint32* bytes_min, uint32* bytes_max)
{
    sws_qm_source_queue_cfg_t   queue_cfg;
    int                         enable;
    int                         rv = BCM_E_NONE;
    uint32                      min_pages_data=0, min_pages_header=0, max_pages=0;

    rv = soc_sbx_caladan3_sws_qm_source_queue_get(unit, queue, &queue_cfg, &enable);
    if (rv == SOC_E_NONE)
    {
        if (enable != 0)
        {
            min_pages_data = queue_cfg.min_pages_data;
            min_pages_header = queue_cfg.min_pages_header;
            max_pages = queue_cfg.max_pages;

            if (bytes_min != NULL)
                *bytes_min = (min_pages_data + min_pages_header) * 256;
            if (bytes_max != NULL)
                *bytes_max = max_pages * 256;
        }
    }

    return rv;
}

int bcm_caladan3_cosq_src_queue_set(int unit, int queue, uint32 bytes_min, uint32 bytes_max)
{
    sws_qm_source_queue_cfg_t   queue_cfg;
    int                         enable;
    int                         rv = BCM_E_NONE;
    uint32                      min_pages_data, min_pages_header, max_pages;

    min_pages_data = bytes_min / 256;
    min_pages_data = min_pages_data / 2;
    min_pages_header = min_pages_data;
    max_pages = bytes_max / 256;
    rv = soc_sbx_caladan3_sws_qm_source_queue_get(unit, queue, &queue_cfg, &enable);
    if (rv == SOC_E_NONE)
    {
        queue_cfg.min_pages_data = min_pages_data;
        queue_cfg.min_pages_header = min_pages_header;
        queue_cfg.max_pages = max_pages;

        rv = soc_sbx_caladan3_sws_qm_source_queue_set(unit, queue, &queue_cfg, enable);
    }

    return rv;
}

#endif /* BCM_CALADAN3_SUPPORT */
