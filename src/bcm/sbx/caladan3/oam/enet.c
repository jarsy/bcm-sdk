/*
 * $Id: enet.c,v 1.32.8.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * Caladan3 Ethernet OAM functions
 */
#ifdef _ERR_MSG_MODULE_NAME
  #error "_ERR_MSG_MODULE_NAME redefined"
#endif

#define _ERR_MSG_MODULE_NAME BSL_LS_BCM_OAM

#include <shared/bsl.h>

#include <soc/debug.h>
#include <bcm/error.h>
#include <bcm/debug.h>
#include <bcm/stack.h>
#include <bcm_int/common/debug.h>
#include <bcm_int/sbx/caladan3/oam/oam.h>
#include <bcm_int/sbx/caladan3/oam/enet.h>
#include <bcm_int/sbx/caladan3/vswitch.h>
#include <soc/sbx/caladan3/port.h>
#include <soc/sbx/sbx_drv.h>
#include <soc/sbx/g3p1/g3p1_int.h>

/* extern bcm_c3_oam_state_t* bcm_c3_oam_state[SOC_MAX_NUM_DEVICES]; */
bcm_c3_oam_enet_state_t* bcm_c3_oam_enet_state[SOC_MAX_NUM_DEVICES];

#define OAM_ENET_STATE(unit) (bcm_c3_oam_enet_state[unit])

STATIC int 
_bcm_c3_oam_enet_validate_endpoint_info(int unit, bcm_oam_endpoint_info_t *endpoint_info);
STATIC int
_bcm_c3_oam_enet_endpoint_replace(int unit, bcm_oam_endpoint_info_t *endpoint_info);
STATIC void
_bcm_c3_oam_enet_init_endpoint_hash_key(int unit, bcm_c3_oam_endpoint_hash_key_t endpoint_key, 
                                       bcm_oam_endpoint_info_t *endpoint_info);
STATIC int
_bcm_c3_oam_enet_endpoint_state_set(int unit, bcm_oam_endpoint_info_t *endpoint_info, 
                                    bcm_oam_endpoint_t endpoint_id);
STATIC int
_bcm_c3_oam_enet_endpoint_state_clear(int unit, 
                                      bcm_oam_endpoint_t endpoint_id);
STATIC int
_bcm_c3_oam_enet_init_service_hash_key(int unit, 
                                       bcm_c3_oam_enet_service_hash_key_t service_key, 
                                       bcm_oam_endpoint_info_t *endpoint_info);
STATIC int
_bcm_c3_oam_enet_service_id_get(int unit, bcm_c3_oam_enet_service_hash_key_t service_key,
                                uint32 *service_id);
STATIC int
_bcm_c3_oam_enet_service_id_allocate(int unit, bcm_c3_oam_enet_service_hash_key_t service_key,
                                     uint32 *service_id);
STATIC int 
_bcm_c3_oam_enet_service_id_free(int unit, bcm_c3_oam_enet_service_hash_key_t service_key,
                                 uint32 service_id);

STATIC void 
_bcm_caladan3_oam_enet_egress_path_init(int unit, bcm_c3_oam_enet_egress_path_desc_t *egress_path);

STATIC int 
_bcm_c3_oam_enet_egress_path_get(int unit, bcm_c3_oam_enet_egress_path_desc_t *egress_path, uint32 endpoint_id);

STATIC int 
_bcm_c3_oam_enet_egress_path_alloc(int unit, bcm_c3_oam_enet_egress_path_desc_t *egress_path);
STATIC int 
_bcm_c3_oam_enet_egress_path_commit(int unit, bcm_c3_oam_enet_egress_path_desc_t *egress_path);
STATIC int 
_bcm_c3_oam_enet_egress_path_update(int unit, bcm_c3_oam_enet_egress_path_desc_t *egress_path, 
                                   bcm_oam_endpoint_info_t *endpoint_info);
STATIC int 
_bcm_c3_oam_enet_egress_path_free(int unit, bcm_c3_oam_enet_egress_path_desc_t *egress_path);
STATIC int 
_bcm_c3_oam_enet_endpoint_local_set(int unit, 
                                    bcm_oam_endpoint_info_t *endpoint_info, 
                                    bcm_oam_endpoint_t endpoint_id);
STATIC int 
_bcm_c3_oam_enet_endpoint_local_update(int unit, 
                                       bcm_oam_endpoint_info_t *endpoint_info, 
                                       bcm_oam_endpoint_t endpoint_id);
STATIC int 
_bcm_c3_oam_enet_endpoint_local_clear(int unit, 
                                      bcm_oam_endpoint_info_t *endpoint_info, 
                                      bcm_oam_endpoint_t endpoint_id);
STATIC int 
_bcm_c3_oam_enet_endpoint_remote_set(int unit, 
                                     bcm_oam_endpoint_info_t *endpoint_info, 
                                     bcm_oam_endpoint_t endpoint_id);
STATIC int 
_bcm_c3_oam_enet_endpoint_remote_update(int unit, 
                                        bcm_oam_endpoint_info_t *endpoint_info, 
                                        bcm_oam_endpoint_t endpoint_id);
STATIC int 
_bcm_c3_oam_enet_endpoint_remote_clear(int unit, 
                                       bcm_oam_endpoint_info_t *endpoint_info, 
                                       bcm_oam_endpoint_t endpoint_id);
STATIC int
_bcm_c3_oam_enet_oamrx_p2e_entry_clear(int unit, bcm_port_t port);
STATIC int
_bcm_c3_oam_enet_oamrx_p2e_entry_set(int unit, bcm_port_t port, int level, uint32 service_id, uint32 flags);   
STATIC int 
_bcm_c3_oam_enet_find_highest_level_non_intermediate_port_endpoint(int unit, soc_sbx_g3p1_oamrx_p2e_t *oamrx_p2e, int *highest_mdlevel);
STATIC int 
_bcm_c3_oam_enet_find_highest_level_non_intermediate_endpoint(int unit, soc_sbx_g3p1_oamrx_t *oamrx, int *highest_mdlevel);
STATIC int
_bcm_c3_oam_enet_oamrx_entry_set(int unit, bcm_port_t port, uint32 inner_vlan, uint32 outer_vlan, 
                                int level, uint32 service_id, uint32 vid_mode, uint32 flags);
STATIC
int _bcm_c3_oam_enet_ccm_period_to_interval_index(int unit, int ccm_period, uint32 *interval_index);
STATIC int
_bcm_c3_oam_enet_endpoint_remote_rdi_state_get(int unit, bcm_oam_endpoint_info_t *endpoint_info, uint8 *remote_rdi_state);

int bcm_c3_oam_enet_init(int unit, uint32 max_endpoints)
{
    int rv = BCM_E_NONE;
    uint32 oi2e_reserved_id = 0;
    soc_sbx_g3p1_oi2e_t oi2e;
    int size;
    uint32 endpoint = 0, current_endpoint = 0, first = TRUE;
    bcm_port_t port;

    BCM_INIT_FUNC_DEFS;

    /*
     * Create ENET OAM State - global OAM ENET state structure used for ETE block allocation
     */
    OAM_ENET_STATE(unit) = sal_alloc(sizeof(bcm_c3_oam_enet_state_t), "oam_enet_state");
    
    if (OAM_ENET_STATE(unit) == NULL) {
        BCM_ERR_EXIT_MSG(BCM_E_MEMORY, (_BCM_MSG("unit %d, unable to allocate oam_enet_state"), unit));
    }
    
    sal_memset(OAM_ENET_STATE(unit), 0, sizeof(bcm_c3_oam_enet_state_t));

    OAM_ENET_STATE(unit)->max_endpoints = max_endpoints;
    
    /* Can't allocate with contiguous flag, so allocate elements back to back and validate at the end */
    for (endpoint = 0; endpoint < max_endpoints; endpoint++) {
        rv = _sbx_caladan3_resource_alloc(unit, SBX_CALADAN3_USR_RES_ETE, 1,
                                          &current_endpoint, 0);
        if (BCM_FAILURE(rv)) {
            BCM_ERR_EXIT_MSG(rv, (_BCM_MSG("Failed to allocate ete: %d %s\n"),
                                  rv, bcm_errmsg(rv)));
        }
        if (first == TRUE) {
            OAM_ENET_STATE(unit)->ete_base = current_endpoint;
            first = FALSE;
        }
    }
    if (current_endpoint != (OAM_ENET_STATE(unit)->ete_base + max_endpoints - 1)) {
        BCM_ERR_EXIT_MSG(rv, (_BCM_MSG("Failed to allocate ete: last endpoint(%d) (%d)\n"),
                              current_endpoint, OAM_ENET_STATE(unit)->ete_base + max_endpoints));
    }

    rv = soc_sbx_g3p1_oam_ete_base_set(unit, OAM_ENET_STATE(unit)->ete_base);
    if (BCM_FAILURE(rv)) {
        BCM_ERR_EXIT_MSG(rv, (_BCM_MSG("Failed to push down ete base: %d %s\n"), rv, bcm_errmsg(rv)));
    }

    /* Allocate 1 OI2E entry */
    rv = _sbx_caladan3_resource_alloc(unit, SBX_CALADAN3_USR_RES_OHI, 1,
                                      &oi2e_reserved_id, 0);
    if (BCM_FAILURE(rv)) {
        BCM_ERR_EXIT_MSG(rv, (_BCM_MSG("Failed to allocate oi2e entry: %d %s\n"), rv, bcm_errmsg(rv)));
    }

    rv = soc_sbx_g3p1_oam_rsvd_oip_set(unit, oi2e_reserved_id);
    if (BCM_FAILURE(rv)) {
        BCM_ERR_EXIT_MSG(rv, (_BCM_MSG("Failed to push down oi2e reserved id: %d %s\n"), rv, bcm_errmsg(rv)));
    }

    soc_sbx_g3p1_oi2e_t_init(&oi2e);
    oi2e.eteptr   = 0;
    oi2e.counter  = 0;
    oi2e.ccounter = 0;
    oi2e.priclass = 0;
    oi2e.allpri   = 0;

    rv = soc_sbx_g3p1_oi2e_set(unit, oi2e_reserved_id, &oi2e);
   if (BCM_FAILURE(rv)) {
        BCM_ERR_EXIT_MSG(rv, (_BCM_MSG("oam enet failed to set oi2e reserved id: %d %s\n"), rv, bcm_errmsg(rv)));
    }

   /* Allocate service info  - skip 0 */
    rv = shr_idxres_list_create(&OAM_ENET_STATE(unit)->service_pool,
                                1, max_endpoints - 1,
                                0, max_endpoints - 1,
                                "oam enet service pool");
    if (BCM_FAILURE(rv)) {
        BCM_ERR_EXIT_MSG(BCM_E_INTERNAL, (_BCM_MSG("oam enet service_pool list creation failure")));
    }

    size = sizeof(bcm_c3_oam_enet_service_info_t) * max_endpoints;
    OAM_ENET_STATE(unit)->service_info = sal_alloc(size, "oam enet service info");

    if (OAM_ENET_STATE(unit)->service_info == NULL) {
        BCM_ERR_EXIT_MSG(BCM_E_INTERNAL, (_BCM_MSG("oam enet service info allocation failure")));
    }
    sal_memset(OAM_ENET_STATE(unit)->service_info, 0, size);

    rv = shr_htb_create(&OAM_ENET_STATE(unit)->service_htbl,
                        max_endpoints,
                        sizeof(bcm_c3_oam_enet_service_hash_key_t),
                        "service_htbl");
    
    if( BCM_FAILURE(rv)) {
        BCM_ERR_EXIT_MSG(BCM_E_INTERNAL, (_BCM_MSG("unit %d, service_htbl hash table creation failure"), unit));
    }

   /* Allocate peer_idxs */
    rv = shr_idxres_list_create(&OAM_ENET_STATE(unit)->peer_idx_pool,
                                1, max_endpoints - 1,
                                0, max_endpoints - 1,
                                "oam peer index pool");
    if (BCM_FAILURE(rv)) {
        BCM_ERR_EXIT_MSG(BCM_E_INTERNAL, (_BCM_MSG("unit %d, peer_idx_pool list creation failure"), unit));
    }

    size = sizeof(bcm_c3_oam_enet_endpoint_state_t) * max_endpoints;
    OAM_ENET_STATE(unit)->enet_endpoint_state = sal_alloc(size, "bcm_c3_oam_enet_endpoint_state");
    
    if (OAM_ENET_STATE(unit)->enet_endpoint_state == NULL) {
        BCM_ERR_EXIT_MSG(BCM_E_MEMORY, (_BCM_MSG("unit %d, bcm_c3_oam_enet_endpoint_state allocation failure"), unit));
    }
    sal_memset(OAM_ENET_STATE(unit)->enet_endpoint_state, 0, size);

    /* Set all mdlevels to forward by default on the port */
    for (port=0; port<SBX_MAX_PORTS; port++) {
        rv = _bcm_c3_oam_enet_oamrx_p2e_entry_clear(unit, port);
        if (BCM_FAILURE(rv)) { 
            BCM_ERR_EXIT_MSG(rv, (_BCM_MSG("unit %d, oamrx_p2e all forward failure"), unit));
        }
    }
 exit:
    BCM_FUNC_RETURN;
}
int bcm_c3_oam_enet_cleanup(int unit, uint32 max_endpoints)
{
    int rv = BCM_E_NONE;

    BCM_INIT_FUNC_DEFS;
    /*
     * Free ENET OAM State - global OAM ENET state structure used for ETE block allocation
     */

    /* These resources are freed previously in _sbx_caladan3_resource_uninit() (allocator.c)
     *  SBX_CALADAN3_USR_RES_ETE, SBX_CALADAN3_USR_RES_OHI so don't free them here.
     */

    rv = soc_sbx_g3p1_oam_ete_base_set(unit, 0);
    BCM_IF_ERR_EXIT(rv);

    if (OAM_ENET_STATE(unit)->service_pool) {
        shr_idxres_list_destroy(OAM_ENET_STATE(unit)->service_pool);
        OAM_ENET_STATE(unit)->service_pool = 0;
    }

    if (OAM_ENET_STATE(unit)->peer_idx_pool) {
        shr_idxres_list_destroy(OAM_ENET_STATE(unit)->peer_idx_pool);
        OAM_ENET_STATE(unit)->peer_idx_pool = 0;
    }

    if (OAM_ENET_STATE(unit)->service_htbl) {
        rv = shr_htb_destroy(&OAM_ENET_STATE(unit)->service_htbl, NULL);
        if( BCM_FAILURE(rv)) {
            BCM_ERR_EXIT_MSG(rv, (_BCM_MSG("unit %d, service hash table destroy failure"), unit));
        }
    }

    if (OAM_ENET_STATE(unit)->service_info) {
        sal_free(OAM_ENET_STATE(unit)->service_info);
    }

    if (OAM_ENET_STATE(unit)->enet_endpoint_state) {
        sal_free(OAM_ENET_STATE(unit)->enet_endpoint_state);
    }

    if (OAM_ENET_STATE(unit)) {
        sal_free(OAM_ENET_STATE(unit));
    }

 exit:
    BCM_FUNC_RETURN;
}
/*
 *   Function
 *      bcm_c3_oam_enet_endpoint_create
 *   Purpose
 *      Create an ethernet oam endpoint and commit to hardware
 *   Parameters
 *       unit           = BCM device number
 *       endpoint_info  = description of endpoint to create
 *   Returns
 *       BCM_E_*
 *  Notes:
 * 
 */
int
bcm_c3_oam_enet_endpoint_create(int unit, 
                                bcm_oam_endpoint_info_t *endpoint_info)
{
    int rv = BCM_E_NONE;
    int is_replace = FALSE;
    int is_with_id = FALSE;
    int is_remote = FALSE;
    int is_upmep  COMPILER_ATTRIBUTE((unused));
    bcm_oam_endpoint_t endpoint_id = 0;
    bcm_c3_oam_endpoint_hash_key_t endpoint_key;
    bcm_c3_oam_endpoint_state_t endpoint_state;
    BCM_INIT_FUNC_DEFS;

    is_upmep = FALSE;
    rv =  _bcm_c3_oam_enet_validate_endpoint_info(unit, endpoint_info);

    if (BCM_FAILURE(rv)) {
        BCM_ERR_EXIT_MSG(rv, (_BCM_MSG("Failed to validate specific service oam (ethernet) endpoint info\n")));
    }

    if ((endpoint_info->flags & BCM_OAM_ENDPOINT_REPLACE))     is_replace = TRUE;
    if ((endpoint_info->flags & BCM_OAM_ENDPOINT_WITH_ID))     is_with_id = TRUE;
    if ((endpoint_info->flags & BCM_OAM_ENDPOINT_REMOTE))      is_remote = TRUE;
    if ((endpoint_info->flags & BCM_OAM_ENDPOINT_UP_FACING))   is_upmep = TRUE;

    if (is_with_id) {
        endpoint_id = endpoint_info->id;
        if (bcm_c3_oam_is_endpoint_id_in_reserved_range(unit, endpoint_id)) {
            BCM_ERR_EXIT_MSG(BCM_E_PARAM, (_BCM_MSG("Endpoint %d in reserved range\n"), endpoint_id));
        } 
    }

    if (is_replace) {
        rv = _bcm_c3_oam_enet_endpoint_replace(unit, endpoint_info);
        BCM_RETURN_VAL_EXIT(rv);
    } 

    if (is_with_id && (bcm_c3_oam_endpoint_is_allocated(unit, endpoint_info->id) == TRUE)) {
        BCM_ERR_EXIT_MSG(BCM_E_PARAM, (_BCM_MSG("Endpoint %d already in use\n"), endpoint_id));
    }
        
    /* 
     * Determine hash key based upon endpoint info.  This must be unique for each endpoint.
     */
    _bcm_c3_oam_enet_init_endpoint_hash_key(unit, endpoint_key, endpoint_info);
    
    rv = bcm_c3_oam_endpoint_state_from_key_get(unit, endpoint_key, &endpoint_state, 0 /* don't remove */);

    if (BCM_SUCCESS(rv)) {
        BCM_ERR_EXIT_MSG(BCM_E_EXISTS, (_BCM_MSG("Error, endpoint_state entry already exists\n")));
    }

    /*
     * Allocate an endpoint_id from the list
     */
    BCM_IF_ERR_EXIT(bcm_c3_oam_endpoint_allocate(unit, &endpoint_id, is_with_id));

    endpoint_info->id = endpoint_id;

    /* 
     * After allocating the endpoint id, then set the key for the endpoint hash entry.
     * This sets up the hash to return the endpoint_id given the key
     */
    rv = bcm_c3_oam_endpoint_key_set(unit, endpoint_key, endpoint_id);
    if (BCM_FAILURE(rv)) {
        BCM_ERR_EXIT_MSG(BCM_E_PARAM, (_BCM_MSG("Unit(%d) error setting hash key for endpoint_id(%d)\n"),
                                       unit, endpoint_id));
    }

    /*
     * Get the endpoint_state data from the id
     */
    bcm_c3_oam_endpoint_state_from_id_get(unit, &endpoint_state, endpoint_id);

    /*
     * Configure the hardware for the local or remote endpoint
     */
    if (is_remote) {
        BCM_IF_ERR_EXIT(_bcm_c3_oam_enet_endpoint_remote_set(unit, endpoint_info, endpoint_id));       
    } else {
        BCM_IF_ERR_EXIT(_bcm_c3_oam_enet_endpoint_local_set(unit, endpoint_info, endpoint_id));
    }

    /*
     * Set the ethernet state then the oam state in the endpoint_state tables based upon
     * the endpoint_info passed in.  These should not return an error because of earlier error
     * checking unless there is a latent bug in the code.
     */
    BCM_IF_ERR_EXIT(_bcm_c3_oam_enet_endpoint_state_set(unit, endpoint_info, endpoint_id));
    BCM_IF_ERR_EXIT(bcm_c3_oam_endpoint_state_set(unit, endpoint_id, endpoint_info));

    /* 
     * Add this endpoint to the associated group's linked list so that delete of the group
     * can easily find all the associated endpoints for that group and walk through the list.
     */
    rv = bcm_c3_oam_group_info_endpoint_list_add(unit, 
                                                 endpoint_info->group,
                                                 endpoint_id);
    if (BCM_FAILURE(rv)) {
        BCM_ERR_EXIT_MSG(BCM_E_PARAM, (_BCM_MSG("Failed to add endpoint (0x%x) to group tracking state\n"), 
                                       endpoint_info->id));
    }

exit:
    if(BCM_FAILURE(rv)) {
        if (endpoint_id) {
            bcm_c3_oam_endpoint_free(unit, endpoint_id);
        }
    }

    BCM_FUNC_RETURN;
}

/*
 *   Function
 *      bcm_c3_oam_enet_endpoint_destroy
 *   Purpose
 *      Destroy an ethernet oam endpoint and commit to hardware
 *   Parameters
 *       unit           = BCM device number
 *       endpoint_info  = description of endpoint to create
 *   Returns
 *       BCM_E_*
 *  Notes:
 * 
 */
int
bcm_c3_oam_enet_endpoint_destroy(int unit, 
                                 bcm_oam_endpoint_info_t *endpoint_info)
{
    int rv = BCM_E_NONE;
    bcm_c3_oam_endpoint_hash_key_t endpoint_key;
    bcm_c3_oam_endpoint_state_t endpoint_state;
    int is_remote = FALSE;
    BCM_INIT_FUNC_DEFS;


    rv =  _bcm_c3_oam_enet_validate_endpoint_info(unit, endpoint_info);

    if (BCM_FAILURE(rv)) {
        BCM_ERR_EXIT_MSG(BCM_E_INTERNAL, (_BCM_MSG("Failed to validate endpoint_info related to id(%d) - corrupted state?\n"), endpoint_info->id));
    }

    if ((endpoint_info->flags & BCM_OAM_ENDPOINT_REMOTE))      is_remote = TRUE;
    /*
     * Update hardware for the local or remote endpoint destruction
     * remote endpoint must be destroyed first.
     */
    if (is_remote) {
        BCM_IF_ERR_EXIT(_bcm_c3_oam_enet_endpoint_remote_clear(unit, endpoint_info, endpoint_info->id));       
    } else {
        BCM_IF_ERR_EXIT(_bcm_c3_oam_enet_endpoint_local_clear(unit, endpoint_info, endpoint_info->id));
    }

    bcm_c3_oam_endpoint_state_from_id_get(unit, &endpoint_state, endpoint_info->id);
   
    /* remove endpoint from group list */
    rv = bcm_c3_oam_group_info_endpoint_list_remove(unit, endpoint_info->group, endpoint_info->id);
    BCM_IF_ERR_EXIT(rv);

     /* 
     * Determine hash key based upon endpoint info.  This must be unique for each endpoint.
     */
    _bcm_c3_oam_enet_init_endpoint_hash_key(unit, endpoint_key, endpoint_info);
    
    /* Free endpoint_state hash entry */
    rv = bcm_c3_oam_endpoint_state_from_key_get(unit, endpoint_key, &endpoint_state, 1 /* remove hash */);

    if (BCM_FAILURE(rv)) {
        BCM_ERR_EXIT_MSG(rv, (_BCM_MSG("Error, endpoint id(%d) endpoint_state hash doesn't exist error\n"), endpoint_info->id));
    }
    
    rv = _bcm_c3_oam_enet_endpoint_state_clear(unit, endpoint_info->id);
    if (BCM_FAILURE(rv)) {
        BCM_ERR_EXIT_MSG(rv, (_BCM_MSG("Error, endpoint id(%d) enet endpoint_state clear error\n"), endpoint_info->id));
    }

    rv = bcm_c3_oam_endpoint_state_clear(unit, endpoint_info->id);
    if (BCM_FAILURE(rv)) {
        BCM_ERR_EXIT_MSG(rv, (_BCM_MSG("Error, endpoint id(%d) state clear error\n"), endpoint_info->id));
    }


    rv = bcm_c3_oam_endpoint_free(unit, endpoint_info->id);
    if (BCM_FAILURE(rv)) {
        BCM_ERR_EXIT_MSG(rv, (_BCM_MSG("Error, endpoint id(%d) resource free error\n"), endpoint_info->id));
    }

exit:
    BCM_FUNC_RETURN;
}
int bcm_c3_oam_enet_endpoint_get(int unit, bcm_oam_endpoint_t endpoint_id, bcm_oam_endpoint_info_t *endpoint_info)
{
    int rv;
    int is_remote = FALSE;
    uint8 remote_rdi_state;

    BCM_INIT_FUNC_DEFS;
    if ((endpoint_info->flags & BCM_OAM_ENDPOINT_REMOTE))      is_remote = TRUE;

    if (is_remote) {
        rv = _bcm_c3_oam_enet_endpoint_remote_rdi_state_get(unit, endpoint_info, &remote_rdi_state);
        if (BCM_FAILURE(rv)) {
            BCM_ERR_EXIT_MSG(rv, (_BCM_MSG("Error, endpoint id(%d) remote rdi state get error\n"), endpoint_info->id));
        }
        endpoint_info->faults |= BCM_OAM_ENDPOINT_FAULT_REMOTE;
    }
exit:
    BCM_FUNC_RETURN;
}

STATIC int
_bcm_c3_oam_enet_endpoint_replace(int unit, 
                                  bcm_oam_endpoint_info_t *endpoint_info)
{
    int rv = BCM_E_NONE;
    int is_remote = FALSE;
    int is_upmep  COMPILER_ATTRIBUTE((unused));
    bcm_oam_endpoint_t endpoint_id = 0;
    bcm_c3_oam_endpoint_hash_key_t endpoint_key;
    bcm_c3_oam_endpoint_state_t endpoint_state;
    
    BCM_INIT_FUNC_DEFS;

    is_upmep = FALSE;
    if ((endpoint_info->flags & BCM_OAM_ENDPOINT_REMOTE))      is_remote = TRUE;
    if ((endpoint_info->flags & BCM_OAM_ENDPOINT_UP_FACING))   is_upmep = TRUE;

    /* 
     * Replace is used to set/clear RDI or to enable transmit of CCM after configuration
     * for replace, we will only support with_id.  This check was done during endpoint
     * validation.
     *
     * Validate endpoint hash
     */
    endpoint_id = endpoint_info->id;
    if (bcm_c3_oam_endpoint_is_allocated(unit, endpoint_id) == FALSE) {
        BCM_ERR_EXIT_MSG(BCM_E_PARAM, (_BCM_MSG("Endpoint %d cannot be updated as it has never been allocated\n"), endpoint_id));
    } 

    /* 
     * Determine hash key based upon endpoint info.  This must be unique for each endpoint.
     */
    _bcm_c3_oam_enet_init_endpoint_hash_key(unit, endpoint_key, endpoint_info);
    
    rv = bcm_c3_oam_endpoint_state_from_key_get(unit, endpoint_key, &endpoint_state, 0 /* don't remove */);

    if (BCM_FAILURE(rv)) {
        BCM_ERR_EXIT_MSG(BCM_E_PARAM, (_BCM_MSG("Error, endpoint_state entry doesn't exist\n")));
    }

    if (!is_remote) {
        rv = _bcm_c3_oam_enet_endpoint_local_update(unit, endpoint_info, endpoint_id);
    } else {
        rv = _bcm_c3_oam_enet_endpoint_remote_update(unit, endpoint_info, endpoint_id);
    }
    if (BCM_FAILURE(rv)) {
        BCM_ERR_EXIT_MSG(rv, (_BCM_MSG("Error updating endpoint id(%d)\n"), endpoint_id));
    }
exit:
    BCM_FUNC_RETURN;
}

/*
 *   Function
 *      bcm_c3_oam_enet_endpoint_info_state_get/set
 *   Purpose
 *      Provide the ethernet OAM protocol specific information in the endpoint_info
 *      data structures, allocate ethernet specific endpoint state, set ethernet
 *      specific endpoint state.
 *   Parameters
 *       unit           = BCM device number
 *       endpoint_info  = description of endpoint created
 *       enet_endpoint_state = pointer to the location where the internal endpoint
 *                             state (ethernet specific) is stored.
 *   Returns
 *       BCM_E_*
 *  Notes:
 * 
 */
int
bcm_c3_oam_enet_endpoint_info_state_get(int unit, 
                                        bcm_oam_endpoint_info_t *endpoint_info, 
                                        bcm_oam_endpoint_t endpoint_id)
{
    int rv = BCM_E_NONE;
    int is_remote = FALSE;
    BCM_INIT_FUNC_DEFS;

    if (endpoint_info == NULL) {
        rv = BCM_E_PARAM;
        BCM_ERR_EXIT_MSG(rv, (_BCM_MSG("Unit(%d) parameter error to retrieve enet endpoint info\n"),
                              endpoint_id));
    }

    if ((endpoint_info->flags & BCM_OAM_ENDPOINT_REMOTE))      is_remote = TRUE;

    endpoint_info->vlan = OAM_ENET_STATE(unit)->enet_endpoint_state[endpoint_id].vlan;

    sal_memcpy(endpoint_info->src_mac_address, 
               OAM_ENET_STATE(unit)->enet_endpoint_state[endpoint_id].src_mac_address,
               sizeof(bcm_mac_t));

    if (!is_remote) {
        endpoint_info->dst_mac_address[0] = 0x01;
        endpoint_info->dst_mac_address[1] = 0xC2;
        endpoint_info->dst_mac_address[2] = 0x80;
        endpoint_info->dst_mac_address[3] = 0x00;
        endpoint_info->dst_mac_address[4] = 0x00;
        endpoint_info->dst_mac_address[5] = 0x30 | endpoint_info->level; /* assumes caller has set this up */
    }
exit:
    BCM_FUNC_RETURN;
}

STATIC int
_bcm_c3_oam_enet_endpoint_remote_rdi_state_get(int unit, bcm_oam_endpoint_info_t *endpoint_info, uint8 *remote_rdi_state)
{
    int rv = BCM_E_NONE;
    soc_sbx_g3p1_oam_peer_state_t oam_peer_state;
    soc_sbx_g3p1_maidmep_t  g3p1_maidmep;
    uint32 service_id = 0;
    bcm_c3_oam_enet_service_hash_key_t service_key;
    uint32 peer_idx;
    bcm_oam_endpoint_info_t local_endpoint_info;

    BCM_INIT_FUNC_DEFS;
    
    if ((endpoint_info==NULL)) {
        BCM_ERR_EXIT_MSG(BCM_E_INTERNAL, (_BCM_MSG("Unit(%d) null endpoint_info pointer\n"), unit));
    }

    rv = bcm_caladan3_oam_endpoint_get(unit, endpoint_info->local_id, &local_endpoint_info);
    if (BCM_FAILURE(rv)) {
        BCM_ERR_EXIT_MSG(BCM_E_INTERNAL, (_BCM_MSG("Unit(%d) failed to get local endpoint info from id 0x%x: %d %s\n"), unit,
                                          endpoint_info->local_id, rv, bcm_errmsg(rv)));
    }

    _bcm_c3_oam_enet_init_service_hash_key(unit, 
                                           service_key, 
                                           endpoint_info);


    rv = _bcm_c3_oam_enet_service_id_get(unit, service_key, &service_id);
    if (BCM_FAILURE(rv)) {
        BCM_ERR_EXIT_MSG(rv, (_BCM_MSG("Unit(%d) Failed to get service id %d %s\n"),
                              unit, rv, bcm_errmsg(rv)));
    }

    soc_sbx_g3p1_maidmep_t_init(&g3p1_maidmep);

    rv = soc_sbx_g3p1_maidmep_get(unit, service_id, local_endpoint_info.name, BCM_C3_OAM_ENET_CCM_MESSAGE_TYPE,
                                  endpoint_info->level, &g3p1_maidmep);
    if (BCM_FAILURE(rv)) {
        BCM_ERR_EXIT_MSG(BCM_E_INTERNAL, (_BCM_MSG("Failed to get oam maidmep 0x%x 0x%x: %d %s\n"),
                                          endpoint_info->local_id, endpoint_info->name, rv, bcm_errmsg(rv)));
    }
    
    peer_idx = g3p1_maidmep.peer_idx;

    soc_sbx_g3p1_oam_peer_state_t_init(&oam_peer_state);
    rv = soc_sbx_g3p1_oam_peer_state_get(unit, peer_idx, &oam_peer_state);
    if (BCM_FAILURE(rv)) {
        BCM_ERR_EXIT_MSG(rv, (_BCM_MSG("Failed to get oam_peer entry for peer_idx(%d) for endpoint_id %d: %d %s\n"),
                              peer_idx, endpoint_info->id, rv, bcm_errmsg(rv)));
    }
    *remote_rdi_state = oam_peer_state.rdi_state;
    oam_peer_state.rdi_ack=0;
    rv = soc_sbx_g3p1_oam_peer_state_set(unit, peer_idx, &oam_peer_state);
    if (BCM_FAILURE(rv)) {
        BCM_ERR_EXIT_MSG(rv, (_BCM_MSG("Failed to set oam_peer entry for peer_idx(%d) for endpoint_id %d: %d %s\n"),
                              peer_idx, endpoint_info->id, rv, bcm_errmsg(rv)));
    }

 exit:
    BCM_FUNC_RETURN;
}
int bcm_c3_oam_enet_sdk_ccm_period_to_packet_ccm_period(int unit, int ccm_period, int *packet_ccm_period)
{
    int rv = BCM_E_NONE;
    if (packet_ccm_period == NULL) {
        rv = BCM_E_PARAM;
        return rv;
    }

    switch(ccm_period) {
    case (BCM_OAM_ENDPOINT_CCM_PERIOD_DISABLED):
        *packet_ccm_period = 0;
        break;
    case (BCM_OAM_ENDPOINT_CCM_PERIOD_3MS):
        *packet_ccm_period = 1;
        break;
    case (BCM_OAM_ENDPOINT_CCM_PERIOD_10MS):
        *packet_ccm_period = 2;
        break;
    case (BCM_OAM_ENDPOINT_CCM_PERIOD_100MS):
        *packet_ccm_period = 3;
        break;
    case (BCM_OAM_ENDPOINT_CCM_PERIOD_1S):
        *packet_ccm_period = 4;
        break;        
    case (BCM_OAM_ENDPOINT_CCM_PERIOD_10S):
        *packet_ccm_period = 5;
        break;        
    case (BCM_OAM_ENDPOINT_CCM_PERIOD_1M):
        *packet_ccm_period = 6;
        break;
    case (BCM_OAM_ENDPOINT_CCM_PERIOD_10M):
        *packet_ccm_period = 7;
        break;
    default:
        rv = BCM_E_PARAM;
        break;
    }
    return rv;
}

STATIC int
_bcm_c3_oam_enet_endpoint_state_set(int unit, 
                                    bcm_oam_endpoint_info_t *endpoint_info, 
                                    bcm_oam_endpoint_t endpoint_id)
{
    bcm_c3_oam_enet_endpoint_state_t *enet_endpoint_state;

    BCM_INIT_FUNC_DEFS;

    if (endpoint_info == NULL) {
       BCM_ERR_EXIT_MSG(BCM_E_PARAM, (_BCM_MSG("Unit(%d) parameter error to retrieve enet endpoint info id(%d)\n"),
                                      unit, endpoint_id));
    }
    if (endpoint_id > OAM_ENET_STATE(unit)->max_endpoints) {
        BCM_ERR_EXIT_MSG(BCM_E_PARAM, (_BCM_MSG("Unit(%d) parameter error endpoint_id(%d) > max\n"),
                                       unit, endpoint_id));
    }
    enet_endpoint_state = &OAM_ENET_STATE(unit)->enet_endpoint_state[endpoint_id];

    enet_endpoint_state->vlan = endpoint_info->vlan;

    sal_memcpy( enet_endpoint_state->src_mac_address,
                endpoint_info->src_mac_address,
                sizeof(bcm_mac_t));
exit:              
    BCM_FUNC_RETURN;
}

STATIC int
_bcm_c3_oam_enet_endpoint_state_clear(int unit, 
                                      bcm_oam_endpoint_t endpoint_id)
{
    bcm_c3_oam_enet_endpoint_state_t *enet_endpoint_state;

    BCM_INIT_FUNC_DEFS;

    if (endpoint_id > OAM_ENET_STATE(unit)->max_endpoints) {
        BCM_ERR_EXIT_MSG(BCM_E_PARAM, (_BCM_MSG("Unit(%d) parameter error endpoint_id(%d) > max\n"),
                                       unit, endpoint_id));
    }
    enet_endpoint_state = &OAM_ENET_STATE(unit)->enet_endpoint_state[endpoint_id];

    enet_endpoint_state->vlan = 0;

    sal_memset( enet_endpoint_state->src_mac_address,
                0,
                sizeof(bcm_mac_t));
exit:              
    BCM_FUNC_RETURN;
}

/*
 *   Function
 *      _bcm_c3_oam_enet_init_service_hash_key
 *   Purpose
 *      Create the key to the service handle allocation
 *  
 *   Parameters
 *       unit        = BCM device number
 *       endpoint_info     = endpoint info
 *       key         = key value to be returned
 * 
 *   Returns
 *       None
 *  Notes:
 */

STATIC int
_bcm_c3_oam_enet_init_service_hash_key(int unit, 
                                       bcm_c3_oam_enet_service_hash_key_t service_key, 
                                       bcm_oam_endpoint_info_t *endpoint_info)
{
    uint8* ptr = service_key;
    uint32 is_upmep = FALSE;
    uint8 vid_mode = 0; /* currently unused by ucode */

    BCM_INIT_FUNC_DEFS;

        sal_memset(service_key, 0, sizeof(bcm_c3_oam_enet_service_hash_key_t));
    
    if(endpoint_info == NULL) {
        BCM_ERR_EXIT_MSG(BCM_E_PARAM, (_BCM_MSG("unit %d, endpoint info null creating service hash"), unit));
    } else {
        if ((endpoint_info->flags & BCM_OAM_ENDPOINT_UP_FACING))   is_upmep = TRUE;

        sal_memcpy(ptr, &endpoint_info->vlan, sizeof(endpoint_info->vlan));
        ptr += sizeof(endpoint_info->vlan);

        sal_memcpy(ptr, &endpoint_info->inner_vlan, sizeof(endpoint_info->inner_vlan));
        ptr += sizeof(endpoint_info->inner_vlan);

        sal_memcpy(ptr, &endpoint_info->gport, sizeof(endpoint_info->gport));
        ptr += sizeof(endpoint_info->gport);
        
        sal_memcpy(ptr, &is_upmep, sizeof(is_upmep));
        ptr += sizeof(is_upmep);

        sal_memcpy(ptr, &vid_mode, sizeof(vid_mode));
        ptr += sizeof(vid_mode);

        if ((ptr - service_key) != BCM_C3_OAM_ENET_SERVICE_HASH_KEY_SIZE) {
            LOG_CLI((BSL_META_U(unit,
                                "service key size expected(%d) but got (%d)\n"), BCM_C3_OAM_ENET_SERVICE_HASH_KEY_SIZE, (uint32) (ptr-service_key)));
        }
    }
exit:              
    BCM_FUNC_RETURN;
}

STATIC int
_bcm_c3_oam_enet_service_id_get(int unit, bcm_c3_oam_enet_service_hash_key_t service_key,
                                uint32 *service_id)
{
    int rv = BCM_E_NONE;
    bcm_c3_oam_enet_service_info_t *service_info;
    BCM_INIT_FUNC_DEFS;

    *service_id = 0;

    rv = shr_htb_find(OAM_ENET_STATE(unit)->service_htbl, service_key,
                      (shr_htb_data_t *)&service_info,
                      0 /* == remove */);

    if (BCM_SUCCESS(rv)) {
        *service_id = service_info->id;
    }

    BCM_RETURN_VAL_EXIT(rv); 
exit:              
    BCM_FUNC_RETURN;
}
STATIC int
_bcm_c3_oam_enet_service_id_allocate(int unit, 
                                     bcm_c3_oam_enet_service_hash_key_t service_key,
                                     uint32 *service_id)
{
    int rv = BCM_E_NONE;
    bcm_c3_oam_enet_service_info_t *service_info;
    BCM_INIT_FUNC_DEFS;

    rv = shr_htb_find(OAM_ENET_STATE(unit)->service_htbl, service_key,
                      (shr_htb_data_t *)&service_info,
                      0 /* == remove */);

    if (BCM_SUCCESS(rv)) {
        /* increment reference count */
        service_info->ref_cnt++;
        *service_id = service_info->id;
        LOG_INFO(BSL_LS_BCM_OAM,
                 (BSL_META_U(unit,
                             "oam enet service handle(%d) has been allocated already, "
                             "use this one\n"),
                  service_info->id));
        BCM_EXIT;
    }

    /* Otherwise, need new service handle */
    rv = shr_idxres_list_alloc(OAM_ENET_STATE(unit)->service_pool, service_id);

    if (BCM_FAILURE(rv)) {
        BCM_ERR_EXIT_MSG(rv, (_BCM_MSG("oam enet service handle allocate: resource unavailable")));
    }

    service_info = &OAM_ENET_STATE(unit)->service_info[*service_id];
    service_info->id = *service_id;
    service_info->ref_cnt++;

    rv = shr_htb_insert(OAM_ENET_STATE(unit)->service_htbl, service_key, service_info);

    /* Play through if error */
    if (rv != BCM_E_NONE) {

        LOG_ERROR(BSL_LS_BCM_OAM,
                  (BSL_META_U(unit,
                              "Failed to insert service_id=%d: %d %s\n"),
                   *service_id, rv, bcm_errmsg(rv)));
        /* from original port of code, playing through if error */
        rv = BCM_E_NONE;
    }
exit:              
    BCM_FUNC_RETURN;
}

STATIC int 
_bcm_c3_oam_enet_service_id_free(int unit, bcm_c3_oam_enet_service_hash_key_t service_key,
                                 uint32 service_id)
{
    int rv = BCM_E_NONE;
    bcm_c3_oam_enet_service_info_t *service_info;

    BCM_INIT_FUNC_DEFS;

    rv = shr_htb_find(OAM_ENET_STATE(unit)->service_htbl, service_key,
                      (shr_htb_data_t *)&service_info,
                      0 /* == remove */);

    /* Play through if error */
    if (service_info->id != service_id) {
        LOG_ERROR(BSL_LS_BCM_OAM,
                  (BSL_META_U(unit,
                              "oam enet service mismatch service_id=%d "
                              "hash id(%d): %d %s\n"),
                   service_id, service_info->id, rv, bcm_errmsg(rv)));
    }

    service_info->ref_cnt--;
    if (service_info->ref_cnt == 0) {
        shr_idxres_list_free(OAM_ENET_STATE(unit)->service_pool, service_id);

        shr_htb_find(OAM_ENET_STATE(unit)->service_htbl, service_key,
                     (shr_htb_data_t *)&service_info,
                     1 /* == remove */);
    }

    BCM_RETURN_VAL_EXIT(rv);
 exit:
    BCM_FUNC_RETURN;
}

STATIC int
_bcm_c3_oam_enet_validate_endpoint_info(int unit, 
                                        bcm_oam_endpoint_info_t *endpoint_info)
{
    int rv = BCM_E_NONE;
    bcm_oam_group_info_t group_info;
    BCM_INIT_FUNC_DEFS;

    if(!endpoint_info) {
        BCM_ERR_EXIT_MSG(BCM_E_PARAM, (_BCM_MSG("Null endpoint info\n")));
    } 
    if (endpoint_info->ccm_period > BCM_C3_OAM_SBX_MAX_PERIOD) {
        BCM_ERR_EXIT_MSG(BCM_E_PARAM, (_BCM_MSG("Maximim period supported by this device is %dms\n"),BCM_C3_OAM_SBX_MAX_PERIOD));

    }    
    if (endpoint_info->level > BCM_C3_OAM_SBX_MAX_MDLEVEL-1) {
        BCM_ERR_EXIT_MSG(BCM_E_PARAM, (_BCM_MSG("level invalid max=%d (0x%x)\n"), BCM_C3_OAM_SBX_MAX_MDLEVEL-1, endpoint_info->level));
    }
    if ((endpoint_info->flags & BCM_C3_OAM_CCM_TX_ENABLE) &&
        (endpoint_info->flags & BCM_OAM_ENDPOINT_INTERMEDIATE)) {
        BCM_ERR_EXIT_MSG(BCM_E_PARAM, (_BCM_MSG("Invalid flags (MIP and TX): 0x%08x \n"), endpoint_info->flags));

    } 
    if (BCM_GPORT_IS_TRUNK(endpoint_info->gport)) {
        BCM_ERR_EXIT_MSG(BCM_E_PARAM, (_BCM_MSG("Trunk unsupported\n")));

    } 
    if ((!BCM_GPORT_IS_LOCAL(endpoint_info->gport)) && (!BCM_GPORT_IS_MODPORT(endpoint_info->gport))) {
        BCM_ERR_EXIT_MSG(BCM_E_PARAM, (_BCM_MSG("only modport gport type supported 0x%x\n"),endpoint_info->gport));

    }

    if ((!(endpoint_info->flags & BCM_OAM_ENDPOINT_UP_FACING)) && 
        (endpoint_info->inner_vlan != 0) &&
        (endpoint_info->inner_vlan != _BCM_VLAN_G3P1_UNTAGGED_VID)) {
        BCM_ERR_EXIT_MSG(BCM_E_PARAM, (_BCM_MSG(" for down meps inner_vlan must be set to invalid (0 or 0xfff) 0x%x\n"), endpoint_info->inner_vlan));
    }
    
    rv = bcm_c3_oam_group_get(unit, endpoint_info->group, &group_info);
    if (BCM_FAILURE(rv)) {
        BCM_ERR_EXIT_MSG(BCM_E_PARAM, (_BCM_MSG(" group(%d) not created or invalid in endpoint_info structure\n"), endpoint_info->group));
    }

    if (!(endpoint_info->flags & BCM_OAM_ENDPOINT_WITH_ID) && (endpoint_info->flags & BCM_OAM_ENDPOINT_REPLACE)) {
        BCM_ERR_EXIT_MSG(BCM_E_PARAM, (_BCM_MSG("WITH_ID must accompany REPLACE endpoint\n")));
    }

exit:
    BCM_FUNC_RETURN;

}

/*
 *   Function
 *      _bcm_c3_oam_enet_init_endpoint_hash_key
 *   Purpose
 *      Create the key to the 
 *  group.name(MAID + group + name + level + direction + gport + (vid/mpls_label)
 *   Parameters
 *       unit        = BCM device number
 *       endpoint_info     = endpoint info
 *       key         = key value to be returned
 * 
 *   Returns
 *       None
 *  Notes:
 */

STATIC void
_bcm_c3_oam_enet_init_endpoint_hash_key(int unit, 
                                        bcm_c3_oam_endpoint_hash_key_t endpoint_key, 
                                        bcm_oam_endpoint_info_t *endpoint_info)
{
    uint8* ptr = endpoint_key;
    uint32 is_upmep = FALSE;
    bcm_oam_group_info_t group_info;

    BCM_INIT_FUNC_DEFS;
    
    sal_memset(endpoint_key, 0, sizeof(bcm_c3_oam_endpoint_hash_key_t));
    
    if(endpoint_info) {

        if ((endpoint_info->flags & BCM_OAM_ENDPOINT_UP_FACING))   is_upmep = TRUE;

        bcm_c3_oam_group_get(unit, endpoint_info->group, &group_info);

        sal_memcpy(ptr, group_info.name, BCM_OAM_GROUP_NAME_LENGTH);
        ptr += BCM_OAM_GROUP_NAME_LENGTH;

        sal_memcpy(ptr, &is_upmep, sizeof(is_upmep));
        ptr += sizeof(is_upmep);

        sal_memcpy(ptr, &endpoint_info->group, sizeof(endpoint_info->group));
        ptr += sizeof(endpoint_info->group);

        sal_memcpy(ptr, &endpoint_info->gport, sizeof(endpoint_info->gport));
        ptr += sizeof(endpoint_info->gport);
        
        sal_memcpy(ptr, &endpoint_info->level, sizeof(endpoint_info->level));
        ptr += sizeof(endpoint_info->level);

        sal_memcpy(ptr, &endpoint_info->vlan, sizeof(endpoint_info->vlan));
        ptr += sizeof(endpoint_info->vlan);

        sal_memcpy(ptr, &endpoint_info->inner_vlan, sizeof(endpoint_info->inner_vlan));
        ptr += sizeof(endpoint_info->inner_vlan);
  
        sal_memcpy(ptr, &endpoint_info->name, sizeof(endpoint_info->name));
        ptr += sizeof(endpoint_info->name);
    }
    if ((ptr - endpoint_key) != BCM_C3_OAM_ENDPOINT_HASH_KEY_SIZE) {
        LOG_CLI((BSL_META_U(unit,
                            "endpoint key size expected(%d) but got (%d)\n"), BCM_C3_OAM_ENDPOINT_HASH_KEY_SIZE, (uint32)(ptr-endpoint_key)));
    }

    BCM_FUNC_RETURN_VOID;
}
/* from lrp.c 
 *  static int _soc_sbx_caladan3_bubble_interval[SOC_SBX_CALADAN3_BUBBLE_INTERVAL_TABLE_SIZE_USED] = {0,           invalid  
 *                                                                                                 (6503),         3.329536 ms 
 *                                                                                                 (19531),        9.999872 ms 
 *                                                                                                 (195312),       99.999744 ms 
 *                                                                                                 (1953125),      1s           
 *                                                                                                 (19531250),     10s          
 *                                                                                                 (117187500),    1 min        
 *                                                                                                 (1171875000)};  10 min       
*/
STATIC
int _bcm_c3_oam_enet_ccm_period_to_interval_index(int unit, int ccm_period, uint32 *interval_index)
{
    int rv = BCM_E_NONE;
    if (interval_index == NULL) {
        rv = BCM_E_PARAM;
        return rv;
    }

    switch(ccm_period) {
    case (BCM_OAM_ENDPOINT_CCM_PERIOD_DISABLED):
        *interval_index = 0;
        break;
    case (BCM_OAM_ENDPOINT_CCM_PERIOD_3MS):
        *interval_index = 1;
        break;
    case (BCM_OAM_ENDPOINT_CCM_PERIOD_10MS):
        *interval_index = 2;
        break;
    case (BCM_OAM_ENDPOINT_CCM_PERIOD_100MS):
        *interval_index = 3;
        break;
    case (BCM_OAM_ENDPOINT_CCM_PERIOD_1S):
        *interval_index = 4;
        break;        
    case (BCM_OAM_ENDPOINT_CCM_PERIOD_10S):
        *interval_index = 5;
        break;        
    case (BCM_OAM_ENDPOINT_CCM_PERIOD_1M):
        *interval_index = 6;
        break;
    case (BCM_OAM_ENDPOINT_CCM_PERIOD_10M):
        *interval_index = 7;
        break;
    default:
        rv = BCM_E_PARAM;
        break;
    }
    return rv;
}

STATIC int 
_bcm_c3_oam_enet_endpoint_local_set(int unit, 
                                    bcm_oam_endpoint_info_t *endpoint_info, 
                                    bcm_oam_endpoint_t endpoint_id)
{
    int rv = BCM_E_NONE;
    int is_upmep = FALSE;
    int is_intermediate COMPILER_ATTRIBUTE((unused));
    int tx_enable = FALSE;
    soc_sbx_g3p1_oam_ep_t oam_ep;
    soc_sbx_g3p1_oamrx_t oamrx;
    bcm_port_t port;
    bcm_oam_group_info_t group_info;
    soc_sbx_g3p1_oam_local_t oam_local;
    uint32 inner_vlan;
    uint32 outer_vlan;
    uint32 vid_mode;
    uint32 interval_index = 0;
    int modid = 0;
    int my_modid = 0;
    int dqueue = 0;
    uint32 service_id = 0;
    bcm_c3_oam_enet_service_hash_key_t service_key;
    int port_based_oam = FALSE;
    int packet_ccm_period = 0;
    bcm_c3_oam_enet_egress_path_desc_t egress_path;
    int rdi_set = FALSE;

    BCM_INIT_FUNC_DEFS;

    is_intermediate = FALSE;
    _bcm_caladan3_oam_enet_egress_path_init(unit, &egress_path);

    /* sanity check pointers */
    if ((endpoint_info==NULL)) {
        BCM_ERR_EXIT_MSG(BCM_E_INTERNAL, (_BCM_MSG("Unit(%d) null endpoint_info pointer\n"), unit));
    }

    if ((endpoint_info->flags & BCM_OAM_ENDPOINT_UP_FACING))        is_upmep        = TRUE;
    if ((endpoint_info->flags & BCM_OAM_ENDPOINT_INTERMEDIATE))     is_intermediate = TRUE;
    if ((endpoint_info->flags & BCM_C3_OAM_CCM_TX_ENABLE))          tx_enable       = TRUE;
    if ((endpoint_info->flags & BCM_OAM_ENDPOINT_REMOTE_DEFECT_TX)) rdi_set         = TRUE;

    BCM_IF_ERR_EXIT(bcm_stk_my_modid_get(unit, &my_modid));

    if (BCM_GPORT_IS_MODPORT(endpoint_info->gport)) {
        port = BCM_GPORT_MODPORT_PORT_GET(endpoint_info->gport);        
        modid = BCM_GPORT_MODPORT_MODID_GET(endpoint_info->gport);
        if (modid != my_modid) {
            BCM_ERR_EXIT_MSG(BCM_E_PARAM,(_BCM_MSG("modid mismatch\n")));
        }
        
    } else if (BCM_GPORT_IS_LOCAL(endpoint_info->gport)){
        port = BCM_GPORT_LOCAL_GET(endpoint_info->gport);
    } else {
        BCM_ERR_EXIT_MSG(BCM_E_PARAM,(_BCM_MSG("gport type unsupported (0x%08x)\n"), endpoint_info->gport));
    }

    /* group */
    bcm_c3_oam_group_get(unit, endpoint_info->group, &group_info);

    if (is_upmep == FALSE) {

        rv = _bcm_c3_oam_enet_egress_path_alloc(unit, &egress_path);
    
        if (BCM_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_BCM_OAM,
                      (BSL_META_U(unit,
                                  "Failed to allocate/retrieve egress path for "
                                  "endpoint %d: %d %s\n"),
                       endpoint_info->id, rv, bcm_errmsg(rv)));
            BCM_RETURN_VAL_EXIT(rv);
        }
        
        if(BCM_SUCCESS(rv)) {
            rv = _bcm_c3_oam_enet_egress_path_update(unit, &egress_path, endpoint_info);
            if (BCM_FAILURE(rv)) {
                LOG_ERROR(BSL_LS_BCM_OAM,
                          (BSL_META_U(unit,
                                      "Failed to update egress path for EP %d\n"),
                           endpoint_info->id));
                BCM_RETURN_VAL_EXIT(rv);
            }
        }
    }
    
    soc_sbx_g3p1_oam_local_t_init(&oam_local);
    soc_sbx_g3p1_oamrx_t_init(&oamrx);
    soc_sbx_g3p1_oam_ep_t_init(&oam_ep);

    oam_ep.mdlvl         = endpoint_info->level;
    oam_ep.mepid         = endpoint_info->name;
    oam_ep.dir           = is_upmep;
    oam_ep.smac0         = endpoint_info->src_mac_address[0];
    oam_ep.smac1         = endpoint_info->src_mac_address[1];
    oam_ep.smac2         = endpoint_info->src_mac_address[2];
    oam_ep.smac3         = endpoint_info->src_mac_address[3];
    oam_ep.smac4         = endpoint_info->src_mac_address[4];
    oam_ep.smac5         = endpoint_info->src_mac_address[5];
    oam_ep.dmac0         = 0x01;
    oam_ep.dmac1         = 0xc2;
    oam_ep.dmac2         = 0x80;
    oam_ep.dmac3         = 0x00;
    oam_ep.dmac4         = 0x00;
    oam_ep.dmac5         = 0x30 | endpoint_info->level;
    oam_ep.tlv_offset    = 70;

    rv = bcm_c3_oam_enet_sdk_ccm_period_to_packet_ccm_period(unit, endpoint_info->ccm_period, &packet_ccm_period);
    if (BCM_FAILURE(rv)) {
        BCM_ERR_EXIT_MSG(BCM_E_INTERNAL,(_BCM_MSG("failed to get ccm period conversion\n")));
    }

    oam_ep.period        = packet_ccm_period;
    oam_ep.flags_rsvd    = 0;
    oam_ep.rdi           = rdi_set;
    oam_ep.opcode        = 1; /* CCM */
    oam_ep.version       = 0;

    if (is_upmep == TRUE) {
        BCM_IF_ERR_EXIT(soc_sbx_g3p1_vlan_ft_base_get(unit, &oam_ep.ftidx));
        
        oam_ep.ftidx += SBX_VSI_FROM_VID(endpoint_info->vlan);
    }

    rv = soc_sbx_caladan3_get_dqueue_from_port(unit, port, 0 /* direction */, 
                                               0 /* cos */, &dqueue);
    if (BCM_FAILURE(rv)) {
        BCM_ERR_EXIT_MSG(BCM_E_INTERNAL,(_BCM_MSG("failed to get SWS dqueue\n")));
    }
    oam_ep.dqueue = dqueue;
    oam_ep.dport  = port;

    if (endpoint_info->vlan != 0) {
        oam_ep.vid           = endpoint_info->vlan;
        oam_ep.num_vlan_tags = 1;
    } else {
        oam_ep.num_vlan_tags = 0;
    }

    rv = soc_sbx_g3p1_oam_ep_set(unit, endpoint_id, &oam_ep);
    if (BCM_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_BCM_OAM,
                  (BSL_META_U(unit,
                              "Failed to write oam_ep 0x%x: %d %s\n"),
                   endpoint_id, rv, bcm_errmsg(rv)));
    }

    vid_mode = 0;

    /* For the upmep case, endpoint_info->vlan=VSI and for the downmep case it equals VLAN */

    if ((endpoint_info->inner_vlan == 0) || (endpoint_info->inner_vlan == _BCM_VLAN_G3P1_UNTAGGED_VID)) {
        /* single or no tag */
        if (endpoint_info->vlan == 0) {
            /* no tag */
            outer_vlan = _BCM_VLAN_G3P1_UNTAGGED_VID;
            inner_vlan = _BCM_VLAN_G3P1_UNTAGGED_VID;
            port_based_oam = TRUE;
        } else {
            /* single tag */
            outer_vlan = endpoint_info->vlan;
            inner_vlan = _BCM_VLAN_G3P1_UNTAGGED_VID;
        }
    } else {
        /* double tag, always use inner vlan set to 0xfff with vid_mode=0 */
        outer_vlan = endpoint_info->vlan;
        inner_vlan = _BCM_VLAN_G3P1_UNTAGGED_VID;
    }
    
    
    /* Get service handle */
    _bcm_c3_oam_enet_init_service_hash_key(unit, 
                                           service_key, 
                                           endpoint_info);
    
    
    rv = _bcm_c3_oam_enet_service_id_allocate(unit, service_key, &service_id);
    if (BCM_FAILURE(rv)) {
        BCM_ERR_EXIT_MSG(rv, (_BCM_MSG("Unit(%d) Failed to allocate service id%d %s\n"),
                              unit, rv, bcm_errmsg(rv)));
    }
    
    if (port_based_oam) {
        BCM_IF_ERR_EXIT(_bcm_c3_oam_enet_oamrx_p2e_entry_set(unit, port, endpoint_info->level, service_id,
                                                             endpoint_info->flags));
        
    } else {
        BCM_IF_ERR_EXIT(_bcm_c3_oam_enet_oamrx_entry_set(unit, port, inner_vlan, outer_vlan, 
                                                         endpoint_info->level, service_id, vid_mode,
                                                         endpoint_info->flags));
    }

    /* the MEG ID field in the CCM is encoded by the application. 
     * all 48 will be written into the packet during transmit,
     * and all 48 will be read on reception.  however, the compare
     * on receive uses a hash over the entire MAID rather than
     * comparing each byte.
     */
    oam_local.maid_w0   = BCM_C3_OAM_MAID_PACK_TO_WORD(&group_info.name[0]);
    oam_local.maid_w1   = BCM_C3_OAM_MAID_PACK_TO_WORD(&group_info.name[4]);
    oam_local.maid_w2   = BCM_C3_OAM_MAID_PACK_TO_WORD(&group_info.name[8]);
    oam_local.maid_w3   = BCM_C3_OAM_MAID_PACK_TO_WORD(&group_info.name[12]);
    oam_local.maid_w4   = BCM_C3_OAM_MAID_PACK_TO_WORD(&group_info.name[16]);
    oam_local.maid_w5   = BCM_C3_OAM_MAID_PACK_TO_WORD(&group_info.name[20]);
    oam_local.maid_w6   = BCM_C3_OAM_MAID_PACK_TO_WORD(&group_info.name[24]);
    oam_local.maid_w7   = BCM_C3_OAM_MAID_PACK_TO_WORD(&group_info.name[28]);
    oam_local.maid_w8   = BCM_C3_OAM_MAID_PACK_TO_WORD(&group_info.name[32]);
    oam_local.maid_w9   = BCM_C3_OAM_MAID_PACK_TO_WORD(&group_info.name[36]);
    oam_local.maid_w10  = BCM_C3_OAM_MAID_PACK_TO_WORD(&group_info.name[40]);
    oam_local.maid_w11  = BCM_C3_OAM_MAID_PACK_TO_WORD(&group_info.name[44]);

    
    rv = soc_sbx_g3p1_oam_local_set(unit, endpoint_id, &oam_local);
    if (BCM_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_BCM_OAM,
                 (BSL_META_U(unit,
                             "Failed to write the second oamep 0x%x: %d %s\n"),
                  endpoint_id, rv, bcm_errmsg(rv)));
        BCM_RETURN_VAL_EXIT(rv);
    }

    /* Only set up bubble if tx is enabled
     */
    if (tx_enable) {
    
        rv = _bcm_c3_oam_enet_ccm_period_to_interval_index(unit, endpoint_info->ccm_period, &interval_index);
        if (BCM_FAILURE(rv)) {
            BCM_ERR_EXIT_MSG(BCM_E_INTERNAL,(_BCM_MSG("ccm_period setting invalid %d %s\n"), rv, bcm_errmsg(rv)));
        }

        rv = soc_sbx_g3p1_bubble_entry_set(unit, 
                                           endpoint_id, 
                                           0 /* count=0 for continuous mode */,
                                           interval_index, 
                                           0 /* task ingress */, 
                                           3 /* oam tx stream */,
                                           1 /* init */, 
                                           0 /* jitter_enable */,
                                           SOC_SBX_G3P1_BUBBLE_UPDATE_MODE_INSERT_CONTINUOUSLY);
        
        if (BCM_FAILURE(rv)) {
            BCM_ERR_EXIT_MSG(BCM_E_INTERNAL,(_BCM_MSG("write to bubble entry failed %d %s\n"), rv, bcm_errmsg(rv)));
        }
    }
exit:
    if (BCM_FAILURE(rv)) {
        if(egress_path.alloc_state) {
            _bcm_c3_oam_enet_egress_path_free(unit, &egress_path);
        }
    }
    BCM_FUNC_RETURN;
}
STATIC int 
_bcm_c3_oam_enet_endpoint_local_update(int unit, 
                                       bcm_oam_endpoint_info_t *endpoint_info, 
                                       bcm_oam_endpoint_t endpoint_id)
{
    int rv = BCM_E_NONE;
    int rdi_set = FALSE;
    int tx_enable = FALSE;
    soc_sbx_g3p1_oam_ep_t oam_ep;
    uint32 interval_index = 0;
    int packet_ccm_period = 0;
    
    BCM_INIT_FUNC_DEFS;
    
    /* sanity check pointers */
    if ((endpoint_info==NULL)) {
        BCM_ERR_EXIT_MSG(BCM_E_INTERNAL, (_BCM_MSG("Unit(%d) null endpoint_info pointer\n"), unit));
    }
    
    if ((endpoint_info->flags & BCM_C3_OAM_CCM_TX_ENABLE))          tx_enable = TRUE;
    if ((endpoint_info->flags & BCM_OAM_ENDPOINT_REMOTE_DEFECT_TX)) rdi_set = TRUE;
    
    soc_sbx_g3p1_oam_ep_t_init(&oam_ep);
    
    rv = soc_sbx_g3p1_oam_ep_get(unit, endpoint_id, &oam_ep);
    if (BCM_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_BCM_OAM,
                 (BSL_META_U(unit,
                             "Failed to get the first local CCM entry 0x%x: %d %s\n"),
                  endpoint_id, rv, bcm_errmsg(rv)));
        BCM_EXIT;
    }
    
    rv = bcm_c3_oam_enet_sdk_ccm_period_to_packet_ccm_period(unit, endpoint_info->ccm_period, &packet_ccm_period);
    if (BCM_FAILURE(rv)) {
        BCM_ERR_EXIT_MSG(BCM_E_INTERNAL,(_BCM_MSG("failed to get ccm period conversion\n")));
    }

    oam_ep.period        = packet_ccm_period;
    oam_ep.rdi           = rdi_set;

    rv = soc_sbx_g3p1_oam_ep_set(unit, endpoint_id, &oam_ep);
    if (BCM_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_BCM_OAM,
                 (BSL_META_U(unit,
                             "Failed to write oam_ep 0x%x: %d %s\n"),
                  endpoint_id, rv, bcm_errmsg(rv)));
    }


    rv = _bcm_c3_oam_enet_ccm_period_to_interval_index(unit, endpoint_info->ccm_period, &interval_index);
    if (BCM_FAILURE(rv)) {
        BCM_ERR_EXIT_MSG(BCM_E_INTERNAL,(_BCM_MSG("ccm_period setting invalid %d %s\n"), rv, bcm_errmsg(rv)));
    }
    if (tx_enable) {
    
        /* interval 1 (3.33ms), stream=3, mode=count, count=1 */  
        rv = soc_sbx_g3p1_bubble_entry_set(unit, 
                                           endpoint_id, 
                                           0 /* count=0 for continuous mode */,
                                           interval_index, 
                                           0 /* task ingress */, 
                                           3 /* oam tx stream */,
                                           1 /* init */, 
                                           0 /* jitter_enable */,
                                           SOC_SBX_G3P1_BUBBLE_UPDATE_MODE_INSERT_CONTINUOUSLY);
        if (BCM_FAILURE(rv)) {
            BCM_ERR_EXIT_MSG(BCM_E_INTERNAL,(_BCM_MSG("write to bubble entry failed %d %s\n"), rv, bcm_errmsg(rv)));
        }
    }
exit:
    BCM_FUNC_RETURN;
}
STATIC int 
_bcm_c3_oam_enet_endpoint_local_clear(int unit, 
                                      bcm_oam_endpoint_info_t *endpoint_info, 
                                      bcm_oam_endpoint_t endpoint_id)
{
    int rv = BCM_E_NONE;
    int is_upmep = FALSE;
    soc_sbx_g3p1_oam_ep_t oam_ep;
    soc_sbx_g3p1_oamrx_t oamrx;
    bcm_port_t port;
    soc_sbx_g3p1_oam_local_t oam_local;
    uint32 inner_vlan;
    uint32 outer_vlan;
    uint32 vid_mode;
    uint32 service_id = 0;
    bcm_c3_oam_enet_service_hash_key_t service_key;
    int port_based_oam = FALSE;
    bcm_c3_oam_enet_egress_path_desc_t egress_path;

    BCM_INIT_FUNC_DEFS;

    /* sanity check pointers */
    if ((endpoint_info==NULL)) {
        BCM_ERR_EXIT_MSG(BCM_E_INTERNAL, (_BCM_MSG("Unit(%d) null endpoint_info pointer\n"), unit));
    }

    if ((endpoint_info->flags & BCM_OAM_ENDPOINT_UP_FACING))        is_upmep        = TRUE;

    if (BCM_GPORT_IS_MODPORT(endpoint_info->gport)) {
        port = BCM_GPORT_MODPORT_PORT_GET(endpoint_info->gport);        
        
    } else if (BCM_GPORT_IS_LOCAL(endpoint_info->gport)){
        port = BCM_GPORT_LOCAL_GET(endpoint_info->gport);
    } else {
        BCM_ERR_EXIT_MSG(BCM_E_PARAM,(_BCM_MSG("gport type unsupported (0x%08x)\n"), endpoint_info->gport));
    }
 
    if (is_upmep == FALSE) {
        BCM_IF_ERR_EXIT( _bcm_c3_oam_enet_egress_path_get(unit, &egress_path, endpoint_id));
        BCM_IF_ERR_EXIT(_bcm_c3_oam_enet_egress_path_free(unit, &egress_path));
    }

    soc_sbx_g3p1_oam_local_t_init(&oam_local);
    soc_sbx_g3p1_oamrx_t_init(&oamrx);
    soc_sbx_g3p1_oam_ep_t_init(&oam_ep);

    rv = soc_sbx_g3p1_oam_ep_set(unit, endpoint_id, &oam_ep);
    if (BCM_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_BCM_OAM,
                  (BSL_META_U(unit,
                              "Failed to write oam_ep 0x%x: %d %s\n"),
                   endpoint_id, rv, bcm_errmsg(rv)));
        BCM_RETURN_VAL_EXIT(rv);
    }

    /* Get service handle */
    _bcm_c3_oam_enet_init_service_hash_key(unit, 
                                           service_key, 
                                           endpoint_info);
    

    /* Check if service is set up  */
    rv = _bcm_c3_oam_enet_service_id_get(unit, service_key, &service_id);
    if (BCM_FAILURE(rv)) {
        BCM_ERR_EXIT_MSG(rv, (_BCM_MSG("Unit(%d) Failed to get service id %d %s\n"),
                              unit, rv, bcm_errmsg(rv)));
    }
    rv = _bcm_c3_oam_enet_service_id_free(unit, service_key, service_id);
    if (BCM_FAILURE(rv)) {
        BCM_ERR_EXIT_MSG(rv, (_BCM_MSG("Unit(%d) Failed to free service id %d %s\n"),
                              unit, rv, bcm_errmsg(rv)));
    }

    vid_mode = 0;
    if ((endpoint_info->inner_vlan == 0) || (endpoint_info->inner_vlan == _BCM_VLAN_G3P1_UNTAGGED_VID)) {
        /* single or no tag */
        if (endpoint_info->vlan == 0) {
            /* no tag */
            outer_vlan = _BCM_VLAN_G3P1_UNTAGGED_VID;
            inner_vlan = _BCM_VLAN_G3P1_UNTAGGED_VID;
            port_based_oam = TRUE;
        } else {
            /* single tag */
            outer_vlan = endpoint_info->vlan;
            inner_vlan = _BCM_VLAN_G3P1_UNTAGGED_VID;
        }
    } else {
        /* double tag, always use inner vlan set to 0xfff with vid_mode=0 */
        outer_vlan = endpoint_info->vlan;
        inner_vlan = _BCM_VLAN_G3P1_UNTAGGED_VID;
    }

    if (port_based_oam) {
        BCM_IF_ERR_EXIT(_bcm_c3_oam_enet_oamrx_p2e_entry_clear(unit, port));
        
    } else {
        BCM_IF_ERR_EXIT(_bcm_c3_oam_enet_oamrx_entry_set(unit, port, inner_vlan, outer_vlan, 
                                                         endpoint_info->level, service_id, vid_mode,
                                                         endpoint_info->flags));
    }

    rv = soc_sbx_g3p1_oam_local_set(unit, endpoint_id, &oam_local);
    if (BCM_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_BCM_OAM,
                  (BSL_META_U(unit,
                              "Failed to write the second oamep 0x%x: %d %s\n"),
                   endpoint_id, rv, bcm_errmsg(rv)));
        BCM_IF_ERR_EXIT(rv);
    }

    
    /* interval 1 (3.33ms), stream=3, mode=count, count=1 */  
    rv = soc_sbx_g3p1_bubble_entry_set(unit, 
                                       endpoint_id, 
                                       0 /* count=0 for continuous mode */,
                                       0 /* interval_index */, 
                                       0 /* task ingress */, 
                                       0 /* oam tx stream */,
                                       0 /* init */, 
                                       0 /* jitter_enable */,
                                       SOC_SBX_G3P1_BUBBLE_UPDATE_MODE_DISABLED);

    if (BCM_FAILURE(rv)) {
        BCM_ERR_EXIT_MSG(BCM_E_INTERNAL,(_BCM_MSG("write to bubble entry failed %d %s\n"), rv, bcm_errmsg(rv)));
    }
exit:
    BCM_FUNC_RETURN;
}
STATIC int 
_bcm_c3_oam_enet_endpoint_remote_set(int unit, 
                                     bcm_oam_endpoint_info_t *endpoint_info, 
                                     bcm_oam_endpoint_t endpoint_id)
{
    int rv = BCM_E_NONE;
    uint32 watchdog_id = BCM_C3_OAM_INVALID_POLICER_ID;
    soc_sbx_g3p1_oam_peer_t oam_peer;
    bcm_oam_group_info_t group_info;
    soc_sbx_g3p1_maidmep_t  g3p1_maidmep;
    uint32 service_id = 0;
    bcm_c3_oam_enet_service_hash_key_t service_key;
    uint32 peer_idx;
    bcm_oam_endpoint_info_t local_endpoint_info;
    int started = 0;

    BCM_INIT_FUNC_DEFS;
    
    /* sanity check pointers */
    if ((endpoint_info==NULL)) {
        BCM_ERR_EXIT_MSG(BCM_E_INTERNAL, (_BCM_MSG("Unit(%d) null endpoint_info pointer\n"), unit));
    }

    soc_sbx_g3p1_maidmep_t_init(&g3p1_maidmep);
    
    BCM_IF_ERR_EXIT(shr_idxres_list_alloc(OAM_ENET_STATE(unit)->peer_idx_pool,
                                          (shr_idxres_element_t *)&peer_idx));


    g3p1_maidmep.peer_idx     = peer_idx;
    g3p1_maidmep.hit          = 1;
    

    _bcm_c3_oam_enet_init_service_hash_key(unit, 
                                           service_key, 
                                           endpoint_info);


    rv = _bcm_c3_oam_enet_service_id_get(unit, service_key, &service_id);
    if (BCM_FAILURE(rv)) {
        BCM_ERR_EXIT_MSG(rv, (_BCM_MSG("Unit(%d) Failed to get service id %d %s\n"),
                              unit, rv, bcm_errmsg(rv)));
    }

    /* Only reason to allocate here is to increment ref_cnt used for freeing
     */
    rv = _bcm_c3_oam_enet_service_id_allocate(unit, service_key, &service_id);
    if (BCM_FAILURE(rv)) {
        BCM_ERR_EXIT_MSG(rv, (_BCM_MSG("Unit(%d) Failed to allocate service id%d %s\n"),
                              unit, rv, bcm_errmsg(rv)));
    }

    rv = bcm_caladan3_oam_endpoint_get(unit, endpoint_info->local_id, &local_endpoint_info);
    if (BCM_FAILURE(rv)) {
        BCM_ERR_EXIT_MSG(BCM_E_INTERNAL, (_BCM_MSG("Unit(%d) failed to get local endpoint info from id 0x%x: %d %s\n"), unit,
                                          endpoint_info->local_id, rv, bcm_errmsg(rv)));
    }


    rv = soc_sbx_g3p1_maidmep_set(unit, service_id, local_endpoint_info.name, BCM_C3_OAM_ENET_CCM_MESSAGE_TYPE,
                                  endpoint_info->level, &g3p1_maidmep);
    if (BCM_FAILURE(rv)) {
        BCM_ERR_EXIT_MSG(BCM_E_INTERNAL, (_BCM_MSG("Unit(%d) failed to set oam maidmep 0x%x 0x%x: %d %s\n"), unit,
                                          endpoint_info->local_id, endpoint_info->name, rv, bcm_errmsg(rv)));
    }
    soc_sbx_g3p1_oam_peer_t_init(&oam_peer);
    
    rv = bcm_c3_oam_group_get(unit, endpoint_info->group, &group_info);
    
    oam_peer.maid_crc = bcm_c3_oam_maid_crc32_get(group_info.name);

    rv = bcm_c3_oam_enet_sdk_ccm_period_to_packet_ccm_period(unit, endpoint_info->ccm_period, (int32*)&oam_peer.period);
    if (BCM_FAILURE(rv)) {
        BCM_ERR_EXIT_MSG(BCM_E_INTERNAL,(_BCM_MSG("failed to get ccm period conversion\n")));
    }
    
    if (endpoint_info->ccm_period) {
        if (endpoint_info->flags & BCM_OAM_ENDPOINT_CCM_RX) {
            started = 1;
            
            rv = bcm_c3_oam_timer_allocate(unit, endpoint_id, endpoint_info->ccm_period,
                                           started,
                                           &watchdog_id);
            if (BCM_FAILURE(rv)) {
                LOG_ERROR(BSL_LS_BCM_OAM,
                          (BSL_META_U(unit,
                                      "Failed to allocated watchdog timer of period %d"
                                      " for epId 0x%x: %d %s\n"),
                           endpoint_info->ccm_period, endpoint_id, 
                           rv, bcm_errmsg(rv)));
            }

            oam_peer.pol_id_wdg = watchdog_id; /* watchdog_id is initialized to INVALID for local EP */
    
        }
    }
    rv = soc_sbx_g3p1_oam_peer_set(unit, peer_idx, &oam_peer);
    if (BCM_FAILURE(rv)) {
        BCM_ERR_EXIT_MSG(rv, (_BCM_MSG("Unit(%d) Failed to configure oam_peer %d %s\n"),
                              unit, rv, bcm_errmsg(rv)));
    }
 exit:
    BCM_FUNC_RETURN;
}
STATIC int 
_bcm_c3_oam_enet_endpoint_remote_update(int unit, 
                                        bcm_oam_endpoint_info_t *endpoint_info, 
                                        bcm_oam_endpoint_t endpoint_id)
{
    int rv = BCM_E_NONE;
    uint32 watchdog_id = BCM_C3_OAM_INVALID_POLICER_ID;
    soc_sbx_g3p1_oam_peer_t oam_peer;
    soc_sbx_g3p1_maidmep_t  g3p1_maidmep;
    uint32 service_id = 0;
    bcm_c3_oam_enet_service_hash_key_t service_key;
    uint32 peer_idx;
    bcm_oam_endpoint_info_t local_endpoint_info;
    int started = 0;

    BCM_INIT_FUNC_DEFS;
    
    /* sanity check pointers */
    if ((endpoint_info==NULL)) {
        BCM_ERR_EXIT_MSG(BCM_E_INTERNAL, (_BCM_MSG("Unit(%d) null endpoint_info pointer\n"), unit));
    }

    rv = bcm_caladan3_oam_endpoint_get(unit, endpoint_info->local_id, &local_endpoint_info);
    if (BCM_FAILURE(rv)) {
        BCM_ERR_EXIT_MSG(BCM_E_INTERNAL, (_BCM_MSG("Unit(%d) failed to get local endpoint info from id 0x%x: %d %s\n"), unit,
                                          endpoint_info->local_id, rv, bcm_errmsg(rv)));
    }

    _bcm_c3_oam_enet_init_service_hash_key(unit, 
                                           service_key, 
                                           endpoint_info);


    rv = _bcm_c3_oam_enet_service_id_get(unit, service_key, &service_id);
    if (BCM_FAILURE(rv)) {
        BCM_ERR_EXIT_MSG(rv, (_BCM_MSG("Unit(%d) Failed to get service id %d %s\n"),
                              unit, rv, bcm_errmsg(rv)));
    }

    soc_sbx_g3p1_maidmep_t_init(&g3p1_maidmep);

    rv = soc_sbx_g3p1_maidmep_get(unit, service_id, local_endpoint_info.name, BCM_C3_OAM_ENET_CCM_MESSAGE_TYPE,
                                  endpoint_info->level, &g3p1_maidmep);
    if (BCM_FAILURE(rv)) {
        BCM_ERR_EXIT_MSG(BCM_E_INTERNAL, (_BCM_MSG("Unit(%d) failed to get oam maidmep 0x%x 0x%x: %d %s\n"), unit,
                                          endpoint_info->local_id, endpoint_info->name, rv, bcm_errmsg(rv)));
    }
    
    peer_idx = g3p1_maidmep.peer_idx;

    
    if (endpoint_info->ccm_period) {
        if (endpoint_info->flags & BCM_OAM_ENDPOINT_CCM_RX) {

            soc_sbx_g3p1_oam_peer_t_init(&oam_peer);
            rv = soc_sbx_g3p1_oam_peer_get(unit, peer_idx, &oam_peer);
            if (BCM_FAILURE(rv)) {
                BCM_ERR_EXIT_MSG(BCM_E_INTERNAL, (_BCM_MSG("Failed to get oam_peer entry for peer_idx(%d)"
                                                           "for endpoint_id %d: %d %s\n"),
                                                  peer_idx, endpoint_id, rv, bcm_errmsg(rv)));
            }

            started = 1;
            rv = bcm_c3_oam_timer_allocate(unit, endpoint_id, endpoint_info->ccm_period,
                                           started,
                                           &watchdog_id);
            if (BCM_FAILURE(rv)) {
                BCM_ERR_EXIT_MSG(BCM_E_INTERNAL, (_BCM_MSG("Failed to allocated watchdog timer of period %dms"
                                                           " for endpoint_id %d: %d %s\n"),
                                                  endpoint_info->ccm_period, endpoint_id, 
                                                  rv, bcm_errmsg(rv)));
            }

            oam_peer.pol_id_wdg = watchdog_id; /* watchdog_id is initialized to INVALID for local EP */
    
            rv = soc_sbx_g3p1_oam_peer_set(unit, peer_idx, &oam_peer);
            if (BCM_FAILURE(rv)) {
                BCM_ERR_EXIT_MSG(rv, (_BCM_MSG("Unit(%d) Failed to configure oam_peer %d %s\n"),
                                      unit, rv, bcm_errmsg(rv)));
            }
        }
    }
 exit:
    BCM_FUNC_RETURN;
}
STATIC int 
_bcm_c3_oam_enet_endpoint_remote_clear(int unit, 
                                       bcm_oam_endpoint_info_t *endpoint_info, 
                                       bcm_oam_endpoint_t endpoint_id)
{
    int rv = BCM_E_NONE;
    uint32 watchdog_id = BCM_C3_OAM_INVALID_POLICER_ID;
    soc_sbx_g3p1_oam_peer_t oam_peer;
    soc_sbx_g3p1_maidmep_t  g3p1_maidmep;
    uint32 service_id = 0;
    bcm_c3_oam_enet_service_hash_key_t service_key;
    uint32 peer_idx;
    bcm_oam_endpoint_info_t local_endpoint_info;

    BCM_INIT_FUNC_DEFS;
    
    /* sanity check pointers */
    if ((endpoint_info==NULL)) {
        BCM_ERR_EXIT_MSG(BCM_E_INTERNAL, (_BCM_MSG("Unit(%d) null endpoint_info pointer\n"), unit));
    }

    soc_sbx_g3p1_maidmep_t_init(&g3p1_maidmep);

    _bcm_c3_oam_enet_init_service_hash_key(unit, 
                                           service_key, 
                                           endpoint_info);


    rv = _bcm_c3_oam_enet_service_id_get(unit, service_key, &service_id);
    if (BCM_FAILURE(rv)) {
        BCM_ERR_EXIT_MSG(rv, (_BCM_MSG("Unit(%d) Failed to get service handle %d %s\n"),
                              unit, rv, bcm_errmsg(rv)));
    }

    _bcm_c3_oam_enet_service_id_free(unit, service_key, service_id);

    rv = bcm_caladan3_oam_endpoint_get(unit, endpoint_info->local_id, &local_endpoint_info);
    if (BCM_FAILURE(rv)) {
        BCM_ERR_EXIT_MSG(BCM_E_INTERNAL, (_BCM_MSG("Unit(%d) failed to get local endpoint info from id 0x%x: %d %s\n"), unit,
                                          endpoint_info->local_id, rv, bcm_errmsg(rv)));
    }


    rv = soc_sbx_g3p1_maidmep_get(unit, service_id, local_endpoint_info.name, BCM_C3_OAM_ENET_CCM_MESSAGE_TYPE,
                                  endpoint_info->level, &g3p1_maidmep);

    if (BCM_FAILURE(rv)) {
        BCM_ERR_EXIT_MSG(BCM_E_INTERNAL, (_BCM_MSG("Unit(%d) failed to get oam maidmep 0x%x 0x%x: %d %s\n"), unit,
                                          endpoint_info->local_id, endpoint_info->name, rv, bcm_errmsg(rv)));
    }
    peer_idx = g3p1_maidmep.peer_idx;
    BCM_IF_ERR_EXIT(shr_idxres_list_free(OAM_ENET_STATE(unit)->peer_idx_pool, peer_idx));

    soc_sbx_g3p1_oam_peer_t_init(&oam_peer);
    
    BCM_IF_ERR_EXIT(soc_sbx_g3p1_oam_peer_get(unit, peer_idx, &oam_peer));


    watchdog_id = oam_peer.pol_id_wdg;

    if (endpoint_info->ccm_period) {
        /* use the "peer" entry's ID. */
        rv = bcm_c3_oam_timer_free(unit, watchdog_id);
        if (BCM_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_BCM_OAM,
                      (BSL_META_U(unit,
                                  "Failed to free watchdog timer for "
                                  "endpoint_id(%d): %d %s\n"),
                       endpoint_info->id, rv, bcm_errmsg(rv)));
        }
    }

    soc_sbx_g3p1_oam_peer_t_init(&oam_peer);
    
    rv = soc_sbx_g3p1_oam_peer_set(unit, peer_idx, &oam_peer);
    if (BCM_FAILURE(rv)) {
        BCM_ERR_EXIT_MSG(rv, (_BCM_MSG("Unit(%d) Failed to configure oam_peer %d %s\n"),
                              unit, rv, bcm_errmsg(rv)));
    }

    soc_sbx_g3p1_maidmep_t_init(&g3p1_maidmep);

    rv = soc_sbx_g3p1_maidmep_set(unit, service_id, local_endpoint_info.name, BCM_C3_OAM_ENET_CCM_MESSAGE_TYPE,
                                  endpoint_info->level, &g3p1_maidmep);

    if (BCM_FAILURE(rv)) {
        BCM_ERR_EXIT_MSG(BCM_E_INTERNAL, (_BCM_MSG("Unit(%d) failed to clear oam maidmep 0x%x 0x%x: %d %s\n"), unit,
                                          endpoint_info->local_id, endpoint_info->name, rv, bcm_errmsg(rv)));
    }

 exit:
    BCM_FUNC_RETURN;
}

/**************************************
 * Egress path functions for down mep *
 **************************************/
 
STATIC void
_bcm_caladan3_oam_enet_egress_path_init(int unit, 
                                        bcm_c3_oam_enet_egress_path_desc_t *egress_path)
{
    sal_memset(egress_path, 0, sizeof(bcm_c3_oam_enet_egress_path_desc_t));
}

/*
 *  Allocate & initialize FTE,OHI,ETE L2, & ETE Encap for DOWN meps only
 */
STATIC int 
_bcm_c3_oam_enet_egress_path_alloc(int unit, 
                                   bcm_c3_oam_enet_egress_path_desc_t *egress_path)
{
    
    BCM_INIT_FUNC_DEFS;

    soc_sbx_g3p1_ete_t_init(&egress_path->ete);
    
    /****************** config ETE *********************/
    egress_path->ete.ipttldec = FALSE;
    egress_path->ete.ttlcheck = FALSE;
    egress_path->ete.nosplitcheck = TRUE;
    egress_path->ete.dmacset = FALSE;
    egress_path->ete.dmacsetlsb = FALSE;
    egress_path->ete.smacset = FALSE;
    egress_path->ete.mtu = SBX_DEFAULT_MTU_SIZE;
    egress_path->ete.nostrip = FALSE;
    egress_path->ete.stpcheck = FALSE;
    
    BCM_C3_OAM_ENET_ALLOC_SET(ETE, egress_path->alloc_state);   

    BCM_EXIT;
 exit:
    BCM_FUNC_RETURN;
}
/*
 *  Get the egress path for the given oam endpoint index
 */
STATIC int
_bcm_c3_oam_enet_egress_path_get(int unit, 
                                 bcm_c3_oam_enet_egress_path_desc_t *egress_path, 
                                 uint32 endpoint_id)
{
    int rv = BCM_E_NONE;
    
    BCM_INIT_FUNC_DEFS;

    egress_path->alloc_state = 0;


    egress_path->ete_index = OAM_ENET_STATE(unit)->ete_base + endpoint_id;
    rv = soc_sbx_g3p1_ete_get(unit, egress_path->ete_index, &egress_path->ete);
    if (BCM_FAILURE(rv)) {
        BCM_ERR_EXIT_MSG(rv, (_BCM_MSG("Failed to get ete 0x%x on replace: %d %s\n"),
                              egress_path->ete_index, rv, bcm_errmsg(rv)));
    }
    BCM_C3_OAM_ENET_ALLOC_SET(ETE, egress_path->alloc_state);

    BCM_EXIT;
 exit:
    BCM_FUNC_RETURN;
}

/*
 * Update the egress path based on the endpoint, and commit to hardware
 */
STATIC int 
_bcm_c3_oam_enet_egress_path_update(int unit,
                                    bcm_c3_oam_enet_egress_path_desc_t *egress_path, 
                                    bcm_oam_endpoint_info_t *endpoint_info)
{
    int is_upmep  COMPILER_ATTRIBUTE((unused));

    BCM_INIT_FUNC_DEFS;
    is_upmep = FALSE;

    if (!egress_path || !endpoint_info) {
        BCM_ERR_EXIT_MSG(BCM_E_PARAM, (_BCM_MSG("NULL pointer\n")));
    }
    
    if ((endpoint_info->flags & BCM_OAM_ENDPOINT_UP_FACING)) is_upmep = TRUE;

    egress_path->ete_index = OAM_ENET_STATE(unit)->ete_base + endpoint_info->id;    
    
    /* when customer port this pkt_pri field will
     * be copied into the vlan tag.pricfi.  cfi is always zero.
     */
    egress_path->ete.defpricfi = (endpoint_info->pkt_pri << 1) + 0;
    
    /* when p2e.customer = 0, (provider port) the remark table, rcos, and rdp
     * are all specified in the int_pri field.
     * 7 bits table, 3 bits rcos, 2 bits rdp
     * set the table number in the encap ETE, and the rcos/rdp into the oamEp entry.
     */
    if(endpoint_info->flags & BCM_OAM_ENDPOINT_USE_QOS_MAP) {
        egress_path->ete.remark = (endpoint_info->egr_map) & 0x7f;
        LOG_INFO(BSL_LS_BCM_OAM,
                 (BSL_META_U(unit,
                             "Set remark index (QOS map) from egr_map %d (0x%04x)\n"), 
                  egress_path->ete.remark, egress_path->ete.remark));
    } else {
        egress_path->ete.remark = ((int)endpoint_info->int_pri >> 5) & 0x7f;
        LOG_INFO(BSL_LS_BCM_OAM,
                 (BSL_META_U(unit,
                             "Set remark index (QOS map) from int_pri %d (0x%04x)\n"), 
                  egress_path->ete.remark, egress_path->ete.remark));
    }
    /* BCM_VLAN_VALID is:  0 < vid < 0x1000
     * G3P1 reserves 0xFFF as an ETE flag specifying "untagged-vid".
     */
    egress_path->ete.usevid = TRUE;
    egress_path->ete.vid = _BCM_VLAN_G3P1_UNTAGGED_VID;
#ifdef TWO_TAG_DOWNMEP_SUPPORT    
    if ((BCM_VLAN_VALID(endpoint_info->vlan)) && (endpoint_info->vlan != _BCM_VLAN_G3P1_UNTAGGED_VID)) {
        egress_path->ete.vid = endpoint_info->vlan;
        LOG_INFO(BSL_LS_BCM_OAM,
                 (BSL_META_U(unit,
                             "Tagged local endpoint using vid %d (0x%04x)\n"), 
                  egress_path->ete.vid, egress_path->ete.vid));
    } else {
        egress_path->ete.vid = _BCM_VLAN_G3P1_UNTAGGED_VID;
        LOG_INFO(BSL_LS_BCM_OAM,
                 (BSL_META_U(unit,
                             "Untagged local endpoint\n")));
    }
#endif    

    BCM_IF_ERR_EXIT(_bcm_c3_oam_enet_egress_path_commit(unit, egress_path));

    BCM_EXIT;
exit:
    BCM_FUNC_RETURN;
}
STATIC int
_bcm_c3_oam_enet_egress_path_commit(int unit, 
                                   bcm_c3_oam_enet_egress_path_desc_t *egress_path)
{
    int rv;
    BCM_INIT_FUNC_DEFS;

    rv = soc_sbx_g3p1_ete_set(unit, egress_path->ete_index, &egress_path->ete);
    if (BCM_FAILURE(rv)) {
        BCM_ERR_EXIT_MSG(rv, (_BCM_MSG("unable to write ete[0x%08x]: %d (%s)\n"),
                              egress_path->ete_index, rv, bcm_errmsg(rv)));
    }
 exit:
    BCM_FUNC_RETURN;

}

STATIC int 
_bcm_c3_oam_enet_egress_path_free(int unit, 
                                  bcm_c3_oam_enet_egress_path_desc_t *egress_path)
{
    int rv = BCM_E_NONE;

    BCM_INIT_FUNC_DEFS;

    LOG_INFO(BSL_LS_BCM_OAM,
             (BSL_META_U(unit,
                         "Freeing resource ete_index=0x%08x\n"),
              egress_path->ete_index));
        
    soc_sbx_g3p1_ete_t_init(&egress_path->ete);
        
    /* none of these errors are fatal, they are here for diagnostics only */
        
    if (BCM_C3_OAM_ENET_ALLOC_GET(ETE, egress_path->alloc_state)) {
            
        rv = soc_sbx_g3p1_ete_set(unit, egress_path->ete_index, &egress_path->ete);
        if (BCM_FAILURE(rv)) {
            BCM_ERR_EXIT_MSG(rv, (_BCM_MSG("unable to clear ete[0x%08x]: %d (%s)\n"),
                                  egress_path->ete_index, rv, bcm_errmsg(rv)));
        }
        BCM_C3_OAM_ENET_ALLOC_CLEAR(ETE, egress_path->alloc_state);
    }
 exit:
    BCM_FUNC_RETURN;
}

/* Set oamrx p2e entry to forward */
STATIC int
_bcm_c3_oam_enet_oamrx_p2e_entry_clear(int unit, bcm_port_t port)
{
    int rv = BCM_E_NONE;
    soc_sbx_g3p1_oamrx_p2e_t  oamrx_p2e;
    uint32 redirect;

    BCM_INIT_FUNC_DEFS;

    soc_sbx_g3p1_oamrx_p2e_t_init(&oamrx_p2e);

    oamrx_p2e.svc_hdl   = 0;
    
    oamrx_p2e.mdlvl_0_f = 1;  /* f forward */
    oamrx_p2e.mdlvl_1_f = 1;
    oamrx_p2e.mdlvl_2_f = 1;
    oamrx_p2e.mdlvl_3_f = 1;
    oamrx_p2e.mdlvl_4_f = 1;
    oamrx_p2e.mdlvl_5_f = 1;
    oamrx_p2e.mdlvl_6_f = 1;
    oamrx_p2e.mdlvl_7_f = 1;

    redirect = 0;
    rv = soc_sbx_g3p1_oamrx_p2e_set(unit, redirect, port, &oamrx_p2e);
    if (BCM_FAILURE(rv)) {
        BCM_ERR_EXIT_MSG(BCM_E_INTERNAL,(_BCM_MSG("write to oamrx_p2e failed %d %s\n"), rv, bcm_errmsg(rv)));
    }
    
    redirect = 1;
    rv = soc_sbx_g3p1_oamrx_p2e_set(unit, redirect, port, &oamrx_p2e);
    if (BCM_FAILURE(rv)) {
        BCM_ERR_EXIT_MSG(BCM_E_INTERNAL,(_BCM_MSG("write to oamrx_p2e failed %d %s\n"), rv, bcm_errmsg(rv)));
    }
   exit:
    BCM_FUNC_RETURN;

}
STATIC int 
_bcm_c3_oam_enet_find_highest_level_non_intermediate_port_endpoint(int unit, soc_sbx_g3p1_oamrx_p2e_t *oamrx_p2e, int *highest_mdlevel)
{
    int rv = BCM_E_NONE;
    int mdlevel;
    BCM_INIT_FUNC_DEFS;

    *highest_mdlevel = 0;

    for (mdlevel=0; mdlevel<BCM_C3_OAM_SBX_MAX_MDLEVEL; mdlevel++) {
        switch(mdlevel) {
        case (0):
            if ((oamrx_p2e->mdlvl_0_p == 1) && (oamrx_p2e->mdlvl_0_m == 0)) {
                *highest_mdlevel=mdlevel;
            }
            break;
        case(1):
            if ((oamrx_p2e->mdlvl_1_p == 1) && (oamrx_p2e->mdlvl_1_m == 0)) {
                *highest_mdlevel=mdlevel;
            }
            break;
        case(2):
            if ((oamrx_p2e->mdlvl_2_p == 1) && (oamrx_p2e->mdlvl_2_m == 0)) {
                *highest_mdlevel=mdlevel;
            }
            break;
        case(3):
            if ((oamrx_p2e->mdlvl_3_p == 1) && (oamrx_p2e->mdlvl_3_m == 0)) {
                *highest_mdlevel=mdlevel;
            }
            break;
        case(4):
            if ((oamrx_p2e->mdlvl_4_p == 1) && (oamrx_p2e->mdlvl_4_m == 0)) {
                *highest_mdlevel=mdlevel;
            }
            break;
        case(5):
            if ((oamrx_p2e->mdlvl_5_p == 1) && (oamrx_p2e->mdlvl_5_m == 0)) {
                *highest_mdlevel=mdlevel;
            }            break;
        case(6):
            if ((oamrx_p2e->mdlvl_6_p == 1) && (oamrx_p2e->mdlvl_6_m == 0)) {
                *highest_mdlevel=mdlevel;
            }
            break;
        case(7):
            if ((oamrx_p2e->mdlvl_7_p == 1) && (oamrx_p2e->mdlvl_7_m == 0)) {
                *highest_mdlevel=mdlevel;
            }
            break;
	/* coverity[dead_error_begin] */
        default:
            rv = BCM_E_INTERNAL;
            BCM_ERR_EXIT_MSG(rv,(_BCM_MSG("mdlevel out of range\n")));
            break;
        }
    }
exit:
    BCM_FUNC_RETURN;
}

STATIC int
_bcm_c3_oam_enet_oamrx_p2e_entry_set(int unit, bcm_port_t port, int level, uint32 service_id, uint32 flags)
{
    int rv = BCM_E_NONE;
    soc_sbx_g3p1_oamrx_p2e_t  oamrx_p2e;
    int mdlevel;
    int is_intermediate = FALSE;
    int highest_mdlevel = 0;
 
    BCM_INIT_FUNC_DEFS;

    if ((flags & BCM_OAM_ENDPOINT_INTERMEDIATE)) is_intermediate = TRUE;

    soc_sbx_g3p1_oamrx_p2e_t_init(&oamrx_p2e);

    rv = soc_sbx_g3p1_oamrx_p2e_get(unit, 0, port, &oamrx_p2e);
    if (BCM_FAILURE(rv)) {
        BCM_ERR_EXIT_MSG(BCM_E_INTERNAL,(_BCM_MSG("read of oamrx_p2e failed %d %s\n"), rv, bcm_errmsg(rv)));
    }
   /* First program current mdlevel passed in as follows:
     * my mdlevel
     * set p to indicated endpoint
     * if intermediate, also set m
     */
    switch (level) 
    {
        case (0):
            oamrx_p2e.mdlvl_0_p = 1; /* p endpoint */
            oamrx_p2e.mdlvl_0_f = 0;
            oamrx_p2e.mdlvl_0_d = 0;

            if (is_intermediate) {
                oamrx_p2e.mdlvl_0_m = 1; /* m intermediate */
            } else {
                oamrx_p2e.mdlvl_0_m = 0;
            }
            break;
        case(1):
            oamrx_p2e.mdlvl_1_p = 1;
            oamrx_p2e.mdlvl_1_f = 0;
            oamrx_p2e.mdlvl_1_d = 0;

            if (is_intermediate) {
                oamrx_p2e.mdlvl_1_m = 1;
            }else {
                oamrx_p2e.mdlvl_1_m = 0;
            }

            break;
        case(2):
            oamrx_p2e.mdlvl_2_p = 1;
            oamrx_p2e.mdlvl_2_f = 0;
            oamrx_p2e.mdlvl_2_d = 0;
            if (is_intermediate) {
                oamrx_p2e.mdlvl_2_m = 1;
            }else {
                oamrx_p2e.mdlvl_2_m = 0;
            }
            break;
        case(3):
            oamrx_p2e.mdlvl_3_p = 1;
            oamrx_p2e.mdlvl_3_f = 0;
            oamrx_p2e.mdlvl_3_d = 0;

            if (is_intermediate) {
                oamrx_p2e.mdlvl_3_m = 1;
            }else {
                oamrx_p2e.mdlvl_3_m = 0;
            }
            break;
        case(4):
            oamrx_p2e.mdlvl_4_p = 1;
            oamrx_p2e.mdlvl_4_f = 0;
            oamrx_p2e.mdlvl_4_d = 0;

            if (is_intermediate) {
                oamrx_p2e.mdlvl_4_m = 1;
            }else {
                oamrx_p2e.mdlvl_4_m = 0;
            }
            break;
        case(5):
            oamrx_p2e.mdlvl_5_p = 1;
            oamrx_p2e.mdlvl_5_f = 0;
            oamrx_p2e.mdlvl_5_d = 0;
            if (is_intermediate) {
                oamrx_p2e.mdlvl_5_m = 1;
            }else {
                oamrx_p2e.mdlvl_5_m = 0;
            }
            break;
        case(6):
            oamrx_p2e.mdlvl_6_p = 1;
            oamrx_p2e.mdlvl_6_f = 0;
            oamrx_p2e.mdlvl_6_d = 0;

            if (is_intermediate) {
                oamrx_p2e.mdlvl_6_m = 1;
            }else {
                oamrx_p2e.mdlvl_6_m = 0;
            }
            break;
        case(7):
            oamrx_p2e.mdlvl_7_p = 1;
            oamrx_p2e.mdlvl_7_f = 0;
            oamrx_p2e.mdlvl_7_d = 0;

            if (is_intermediate) {
                oamrx_p2e.mdlvl_7_m = 1;
            }else {
                oamrx_p2e.mdlvl_7_m = 0;
            }
            break;
        default:
            break;
    }

    /* Find highest endpoints in use for this service */
    _bcm_c3_oam_enet_find_highest_level_non_intermediate_port_endpoint(unit, &oamrx_p2e, &highest_mdlevel);

    /* Set above this level to forward */
    if (highest_mdlevel < BCM_C3_OAM_SBX_MAX_MDLEVEL-1) {
        for (mdlevel=highest_mdlevel+1; mdlevel<BCM_C3_OAM_SBX_MAX_MDLEVEL; mdlevel++) {
            switch (mdlevel) {
            case (0):
                oamrx_p2e.mdlvl_0_f = 1;  /* f forward */
                oamrx_p2e.mdlvl_0_d = 0;  /* d drop */
                break;
            case(1):
                oamrx_p2e.mdlvl_1_f = 1;
                oamrx_p2e.mdlvl_1_d = 0;
                break;
            case(2):
                oamrx_p2e.mdlvl_2_f = 1;
                oamrx_p2e.mdlvl_2_d = 0;
                break;
            case(3):
                oamrx_p2e.mdlvl_3_f = 1;
                oamrx_p2e.mdlvl_3_d = 0;
                break;
            case(4):
                oamrx_p2e.mdlvl_4_f = 1;
                oamrx_p2e.mdlvl_4_d = 0;
                break;
            case(5):
                oamrx_p2e.mdlvl_5_f = 1;
                oamrx_p2e.mdlvl_5_d = 0;
                break;
            case(6):
                oamrx_p2e.mdlvl_6_f = 1;
                oamrx_p2e.mdlvl_6_d = 0;
                break;
           case(7):
                oamrx_p2e.mdlvl_7_f = 1;
                oamrx_p2e.mdlvl_7_d = 0;
                break;
            default:
                break;
            }
        }
    }
    if (highest_mdlevel > 0) {
        for (mdlevel=0; mdlevel<highest_mdlevel; mdlevel++) {
            switch (mdlevel) {
            case (0):
                if (oamrx_p2e.mdlvl_0_p != 1) {
                    oamrx_p2e.mdlvl_0_d = 1;  /* d drop */
                    oamrx_p2e.mdlvl_0_f = 0;  /* f forward */
                }
                break;
            case(1):
                if (oamrx_p2e.mdlvl_1_p != 1) {
                    oamrx_p2e.mdlvl_1_d = 1;
                    oamrx_p2e.mdlvl_1_f = 0;
                }
                break;
            case(2):
                if (oamrx_p2e.mdlvl_2_p != 1) {
                    oamrx_p2e.mdlvl_2_d = 1;
                    oamrx_p2e.mdlvl_2_f = 0;
                }
                break;
            case(3):
                if (oamrx_p2e.mdlvl_3_p != 1) {
                    oamrx_p2e.mdlvl_3_d = 1;
                    oamrx_p2e.mdlvl_3_f = 0;
                }
                break;
            case(4):
                if (oamrx_p2e.mdlvl_4_p != 1) {
                    oamrx_p2e.mdlvl_4_d = 1;
                    oamrx_p2e.mdlvl_4_f = 0;
                }
                break;
            case(5):
                if (oamrx_p2e.mdlvl_5_p != 1) {
                    oamrx_p2e.mdlvl_5_d = 1;
                    oamrx_p2e.mdlvl_5_f = 0;
                }
                break;
            case(6):
                if (oamrx_p2e.mdlvl_6_p != 1) {
                    oamrx_p2e.mdlvl_6_d = 1;
                    oamrx_p2e.mdlvl_6_f = 0;
                }
                break;
            case(7):
                if (oamrx_p2e.mdlvl_7_p != 1) {
                    oamrx_p2e.mdlvl_7_d = 1;
                    oamrx_p2e.mdlvl_7_f = 0;
                }
                break;
            default:
                break;
            }
        }
    }
    oamrx_p2e.svc_hdl    = (int)service_id;    

    rv = soc_sbx_g3p1_oamrx_p2e_set(unit, 0, port, &oamrx_p2e);
    if (BCM_FAILURE(rv)) {
        BCM_ERR_EXIT_MSG(BCM_E_INTERNAL,(_BCM_MSG("write to oamrx_p2e failed %d %s\n"), rv, bcm_errmsg(rv)));
    }
    
   exit:
    BCM_FUNC_RETURN;
}
STATIC int 
_bcm_c3_oam_enet_find_highest_level_non_intermediate_endpoint(int unit, soc_sbx_g3p1_oamrx_t *oamrx, int *highest_mdlevel)
{
    int rv = BCM_E_NONE;
    int mdlevel;
    BCM_INIT_FUNC_DEFS;

    *highest_mdlevel = 0;

    for (mdlevel=0; mdlevel<BCM_C3_OAM_SBX_MAX_MDLEVEL; mdlevel++) {
        switch(mdlevel) {
        case (0):
            if ((oamrx->mdlvl_0_p == 1) && (oamrx->mdlvl_0_m == 0)) {
                *highest_mdlevel=mdlevel;
            }
            break;
        case(1):
            if ((oamrx->mdlvl_1_p == 1) && (oamrx->mdlvl_1_m == 0)) {
                *highest_mdlevel=mdlevel;
            }
            break;
        case(2):
            if ((oamrx->mdlvl_2_p == 1) && (oamrx->mdlvl_2_m == 0)) {
                *highest_mdlevel=mdlevel;
            }
            break;
        case(3):
            if ((oamrx->mdlvl_3_p == 1) && (oamrx->mdlvl_3_m == 0)) {
                *highest_mdlevel=mdlevel;
            }
            break;
        case(4):
            if ((oamrx->mdlvl_4_p == 1) && (oamrx->mdlvl_4_m == 0)) {
                *highest_mdlevel=mdlevel;
            }
            break;
        case(5):
            if ((oamrx->mdlvl_5_p == 1) && (oamrx->mdlvl_5_m == 0)) {
                *highest_mdlevel=mdlevel;
            }            break;
        case(6):
            if ((oamrx->mdlvl_6_p == 1) && (oamrx->mdlvl_6_m == 0)) {
                *highest_mdlevel=mdlevel;
            }
            break;
        case(7):
            if ((oamrx->mdlvl_7_p == 1) && (oamrx->mdlvl_7_m == 0)) {
                *highest_mdlevel=mdlevel;
            }
            break;
        /* coverity[dead_error_begin] */
        default:
            rv = BCM_E_INTERNAL;
            BCM_ERR_EXIT_MSG(rv,(_BCM_MSG("mdlevel out of range\n")));
            break;
        }
    }
exit:
    BCM_FUNC_RETURN;
}

STATIC int
_bcm_c3_oam_enet_oamrx_entry_set(int unit, bcm_port_t port, uint32 inner_vlan, uint32 outer_vlan, 
                                 int level, uint32 service_id, uint32 vid_mode, uint32 flags)
{
    int rv = BCM_E_NONE;
    soc_sbx_g3p1_oamrx_t  oamrx;
    int mdlevel;
    int is_upmep = FALSE;
    int is_intermediate = FALSE;
    int highest_mdlevel = 0;

    BCM_INIT_FUNC_DEFS;

    if ((flags & BCM_OAM_ENDPOINT_UP_FACING))    is_upmep   = TRUE;
    if ((flags & BCM_OAM_ENDPOINT_INTERMEDIATE)) is_intermediate = TRUE;
  
    soc_sbx_g3p1_oamrx_t_init(&oamrx);

    /* Read entry, if already set up, this will come back, otherwise, we can overwrite */
    rv = soc_sbx_g3p1_oamrx_get(unit, inner_vlan, outer_vlan, is_upmep, port, vid_mode, &oamrx);
    if (rv == SOC_E_NOT_FOUND) {
        LOG_INFO(BSL_LS_BCM_OAM,
                 (BSL_META_U(unit,
                             "oamrx entry does not exist (%s) (may be ok if oam "
                             "service is not yet configured for any mdlevel)\n"),
                  bcm_errmsg(rv)));
    }

    /* ******************************************************************************************
     * For endpoints, we process messages that hit the configured MdLevel (so set the p bit). 
     * We are supposed to drop (set d bit) messages trying to pass through at lower MdLevels
     * than a configured endpoint.  And messages at higher MdLevels can be forwarded (set f bit)
     * past an endpoint.
     *
     * Endpoints can be nested.  We could have two endpoints on the same port, same VID, same 
     * maintenance group, but at two different MdLevels.  Lets say one is at MdLevel 2 and the
     * other at 5.  We would block (drop) on Mdlevel 0, 1, 3, & 4.  We would process at MdLevels 2
     * and 5.  And forward on MdLevels 6 and 7.
     *
     * MIPs are a little different.  MIPs do not block at lower MdLevels.  In the above example, 
     * we could add a MIP at MdLevel 7.  So we would then process at MdLevels 2, 5, & 7 (set p 
     * for 2,5,7 and also set m for 7).  Block at 0,1,3, & 4.  And forward at 6.
     **********************************************************************************************
     */

    /* First program current mdlevel passed in as follows:
     * my mdlevel
     * set p to indicated endpoint
     * if intermediate, also set m
     */
    switch (level) 
    {
        case (0):
            oamrx.mdlvl_0_p = 1; /* p endpoint */
            oamrx.mdlvl_0_f = 0;
            oamrx.mdlvl_0_d = 0;

            if (is_intermediate) {
                oamrx.mdlvl_0_m = 1; /* m intermediate */
            } else {
                oamrx.mdlvl_0_m = 0;
            }
            break;
        case(1):
            oamrx.mdlvl_1_p = 1;
            oamrx.mdlvl_1_f = 0;
            oamrx.mdlvl_1_d = 0;

            if (is_intermediate) {
                oamrx.mdlvl_1_m = 1;
            }else {
                oamrx.mdlvl_1_m = 0;
            }

            break;
        case(2):
            oamrx.mdlvl_2_p = 1;
            oamrx.mdlvl_2_f = 0;
            oamrx.mdlvl_2_d = 0;
            if (is_intermediate) {
                oamrx.mdlvl_2_m = 1;
            }else {
                oamrx.mdlvl_2_m = 0;
            }
            break;
        case(3):
            oamrx.mdlvl_3_p = 1;
            oamrx.mdlvl_3_f = 0;
            oamrx.mdlvl_3_d = 0;

            if (is_intermediate) {
                oamrx.mdlvl_3_m = 1;
            }else {
                oamrx.mdlvl_3_m = 0;
            }
            break;
        case(4):
            oamrx.mdlvl_4_p = 1;
            oamrx.mdlvl_4_f = 0;
            oamrx.mdlvl_4_d = 0;

            if (is_intermediate) {
                oamrx.mdlvl_4_m = 1;
            }else {
                oamrx.mdlvl_4_m = 0;
            }
            break;
        case(5):
            oamrx.mdlvl_5_p = 1;
            oamrx.mdlvl_5_f = 0;
            oamrx.mdlvl_5_d = 0;
            if (is_intermediate) {
                oamrx.mdlvl_5_m = 1;
            }else {
                oamrx.mdlvl_5_m = 0;
            }
            break;
        case(6):
            oamrx.mdlvl_6_p = 1;
            oamrx.mdlvl_6_f = 0;
            oamrx.mdlvl_6_d = 0;

            if (is_intermediate) {
                oamrx.mdlvl_6_m = 1;
            }else {
                oamrx.mdlvl_6_m = 0;
            }
            break;
        case(7):
            oamrx.mdlvl_7_p = 1;
            oamrx.mdlvl_7_f = 0;
            oamrx.mdlvl_7_d = 0;

            if (is_intermediate) {
                oamrx.mdlvl_7_m = 1;
            }else {
                oamrx.mdlvl_7_m = 0;
            }
            break;
        default:
            break;
    }

    /* Find highest endpoints in use for this service */
    _bcm_c3_oam_enet_find_highest_level_non_intermediate_endpoint(unit, &oamrx, &highest_mdlevel);

    /* Set above this level to forward */
    if (highest_mdlevel < BCM_C3_OAM_SBX_MAX_MDLEVEL-1) {
        for (mdlevel=highest_mdlevel+1; mdlevel<BCM_C3_OAM_SBX_MAX_MDLEVEL; mdlevel++) {
            switch (mdlevel) {
            case (0):
                oamrx.mdlvl_0_f = 1;  /* f forward */
                oamrx.mdlvl_0_d = 0;  /* d drop */
                break;
            case(1):
                oamrx.mdlvl_1_f = 1;
                oamrx.mdlvl_1_d = 0;
                break;
            case(2):
                oamrx.mdlvl_2_f = 1;
                oamrx.mdlvl_2_d = 0;
                break;
            case(3):
                oamrx.mdlvl_3_f = 1;
                oamrx.mdlvl_3_d = 0;
                break;
            case(4):
                oamrx.mdlvl_4_f = 1;
                oamrx.mdlvl_4_d = 0;
                break;
            case(5):
                oamrx.mdlvl_5_f = 1;
                oamrx.mdlvl_5_d = 0;
                break;
            case(6):
                oamrx.mdlvl_6_f = 1;
                oamrx.mdlvl_6_d = 0;
                break;
            case(7):
                oamrx.mdlvl_7_f = 1;
                oamrx.mdlvl_7_d = 0;
                break;
            default:
                break;
            }
        }
    }
    if (highest_mdlevel > 0) {
        for (mdlevel=0; mdlevel<highest_mdlevel; mdlevel++) {
            switch (mdlevel) {
            case (0):
                if (oamrx.mdlvl_0_p != 1) {
                    oamrx.mdlvl_0_d = 1;  /* d drop */
                    oamrx.mdlvl_0_f = 0;  /* f forward */
                }
                break;
            case(1):
                if (oamrx.mdlvl_1_p != 1) {
                    oamrx.mdlvl_1_d = 1;
                    oamrx.mdlvl_1_f = 0;
                }
                break;
            case(2):
                if (oamrx.mdlvl_2_p != 1) {
                    oamrx.mdlvl_2_d = 1;
                    oamrx.mdlvl_2_f = 0;
                }
                break;
            case(3):
                if (oamrx.mdlvl_3_p != 1) {
                    oamrx.mdlvl_3_d = 1;
                    oamrx.mdlvl_3_f = 0;
                }
                break;
            case(4):
                if (oamrx.mdlvl_4_p != 1) {
                    oamrx.mdlvl_4_d = 1;
                    oamrx.mdlvl_4_f = 0;
                }
                break;
            case(5):
                if (oamrx.mdlvl_5_p != 1) {
                    oamrx.mdlvl_5_d = 1;
                    oamrx.mdlvl_5_f = 0;
                }
                break;
            case(6):
                if (oamrx.mdlvl_6_p != 1) {
                    oamrx.mdlvl_6_d = 1;
                    oamrx.mdlvl_6_f = 0;
                }
                break;
            case(7):
                if (oamrx.mdlvl_7_p != 1) {
                    oamrx.mdlvl_7_d = 1;
                    oamrx.mdlvl_7_f = 0;
                }
                break;
            default:
                break;
            }
        }
    }
    oamrx.svc_hdl    = (int)service_id;
    oamrx.hit = 1;
    rv = soc_sbx_g3p1_oamrx_set(unit, inner_vlan, outer_vlan, is_upmep, port, vid_mode, &oamrx);
    if (BCM_FAILURE(rv)) {
        BCM_ERR_EXIT_MSG(BCM_E_INTERNAL,(_BCM_MSG("write to oamrx failed %d %s\n"), rv, bcm_errmsg(rv)));
    }    
    
   exit:
    BCM_FUNC_RETURN;
}
