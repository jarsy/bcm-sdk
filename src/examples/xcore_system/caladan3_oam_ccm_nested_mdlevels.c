/*
 * $Id: caladan3_oam_ccm_nested_mdlevels.c,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 */
/*
 * CALADAN3 OAM
 *
 * Nested MdLevels OAM Continuity Check  setup script
 *
 * This script configures a maintenance group and adds 2 local and remote
 * down MEP CCM endpoints to a given port, vlan and on different mdLevels.
 *
 * CCM messages are sent and then looped back to receive (port is in loopback)
 * once per second.
 *
 * To run the test on port 6, vlan 2 from bcm.user build directory:
 * with related 10g.soc, sbx.soc scripts.
 *

cd ../../../../src/examples/xcore_system
port xe6 loopback=mac
debugmod bcm oam
cint caladan3_oam_ccm_nested_mdlevels.c
cint
caladan3_oam_ccm_setup(6,2);
quit;
oam group show
oam endpoint show 1
oam endpoint show 2
oam endpoint show 3
oam endpoint show 4

* disable loopback and timeout should occur after ~3.5x interval (3.5s).

    port xe6 loopback=none
    bcm>unit(0) oam timer id(0x00000001) forced_timeout(0) timer_active_when_forced(1)
*
* Some relevant data on packet transmitted:
smac 00:02:03:04:05:06
dmac 01:c2:80:00:00:33 <- mdlevel encoding
vlan 2

cint
caladan3_oam_ccm_breakdown();

*/  
int C3_NUM_LOCAL_ENDPOINTS  = 2;
int C3_NUM_REMOTE_ENDPOINTS = 2;

int group_id;
bcm_oam_group_info_t    group_info;
bcm_oam_group_info_t    group_info_get;
bcm_oam_endpoint_info_t local_endpoint_info[2];
bcm_oam_endpoint_info_t remote_endpoint_info[2];

/* 
 *  48 char ASCII String
 *  Maintenance identifier 1: endpoints 106 and 206.
 */
char maid_str1[48] = {0x4d, 0x61, 0x69, 0x6e, 0x74, 0x65, 0x6e, 0x61,
                      0x6e, 0x63, 0x65, 0x20, 0x69, 0x64, 0x65, 0x6e,
                      0x74, 0x69, 0x66, 0x69, 0x65, 0x72, 0x20, 0x31,
                      0x3a, 0x20, 0x65, 0x6e, 0x64, 0x70, 0x6f, 0x69,
                      0x6e, 0x74, 0x73, 0x20, 0x31, 0x30, 0x36, 0x20,
                      0x61, 0x6e, 0x64, 0x20, 0x32, 0x30, 0x36, 0x2e};


int caladan3_oam_callback(int unit,
                          uint32 flags, 
                          bcm_oam_event_type_t event_type, 
                          bcm_oam_group_t group, 
                          bcm_oam_endpoint_t endpoint_id, 
                          void *user_data)
{
    int rc = BCM_E_NONE;
    bcm_oam_endpoint_info_t endpoint;

    bcm_oam_endpoint_info_t_init (&endpoint);

    if (event_type == bcmOAMEventEndpointCCMTimeout) {
        printf("oam endpoint(%d) bcmOAMEventEndpointCCMTimeout\n", endpoint_id);
    }
    return rc;
}

int caladan3_oam_group_create(int unit, 
                              uint8 *maid_str, 
                              bcm_oam_group_t *group_id)
{
    int                     rc = BCM_E_NONE;
    int                     i;
    /* 
     * Initialize group info structure
     */
    bcm_oam_group_info_t_init(&group_info);
    
    for (i=0; i<BCM_OAM_GROUP_NAME_LENGTH; i++) {
        group_info.name[i] = maid_str[i];
    }
    
    rc = bcm_oam_group_create(unit, &group_info);
    if (rc != BCM_E_NONE) {
        printf("bcm_oam_group_create failed: %d (%s)\n", rc, bcm_errmsg(rc));
        return rc;
    }
    printf("OAM group %d created\n", group_info.id);
    *group_id = group_info.id;

    return rc;
}
int caladan3_oam_endpoints_create(int unit, 
                                  int port, 
                                  int vlan, 
                                  int mdlevel, 
                                  bcm_oam_group_t group_id,
                                  int index)
{
    int                     rc           = BCM_E_NONE;
    bcm_module_t            modid        = 0;
    bcm_mac_t               local_mac    = {0x00, 0x02, 0x03, 0x04, 0x5, 0x6};
    bcm_mac_t               remote_mac   = {0x7, 0x8, 0x9, 0xa, 0xb, 0xc};
    int                     local_id;
    int                     peer_port    = port;
    int                     i;


    rc = bcm_stk_my_modid_get(unit, &modid);
    if (rc != BCM_E_NONE) {
        printf("bcm_stk_my_modid_get failed: %d (%s)\n", rc, bcm_errmsg(rc));
        return rc;
    }

    bcm_oam_endpoint_info_t_init(&local_endpoint_info[index]);
    local_endpoint_info[index].name = 100 + port + mdlevel;

    /* local endpoint mdlevel */
    BCM_GPORT_MODPORT_SET(local_endpoint_info[index].gport, modid, port);
    local_endpoint_info[index].group      = group_id;
    local_endpoint_info[index].vlan       = vlan;
    local_endpoint_info[index].level      = mdlevel;
    local_endpoint_info[index].flags      = BCM_OAM_ENDPOINT_PORT_STATE_TX;
    local_endpoint_info[index].type       = bcmOAMEndpointTypeEthernet;
    local_endpoint_info[index].ccm_period = BCM_OAM_ENDPOINT_CCM_PERIOD_1S;

    for (i=0; i<6; i++) {
        local_endpoint_info[index].src_mac_address[i] = local_mac[i];
    }

    rc = bcm_oam_endpoint_create(unit, &local_endpoint_info[index]);
    if (rc != BCM_E_NONE) {
        printf("bcm_oam_endpoint_create (local) failed: %d (%s)\n", rc, bcm_errmsg(rc));
        return rc;
    }

    printf("Local oam endpoint 0x%08x created\n", local_endpoint_info[index].id);

    bcm_oam_endpoint_info_t_init(&remote_endpoint_info[index]);
    remote_endpoint_info[index].name = 200 + port + mdlevel;

    remote_endpoint_info[index].local_id = local_endpoint_info[index].id;

    /* remote endpoint mdlevel 3 */
    BCM_GPORT_MODPORT_SET((remote_endpoint_info[index].gport), modid, peer_port);
    remote_endpoint_info[index].group      = group_id;
    remote_endpoint_info[index].vlan       = vlan;
    remote_endpoint_info[index].level      = mdlevel;
    remote_endpoint_info[index].flags      = BCM_OAM_ENDPOINT_REMOTE | BCM_OAM_ENDPOINT_CCM_RX;
    remote_endpoint_info[index].type       = bcmOAMEndpointTypeEthernet;
    remote_endpoint_info[index].ccm_period = BCM_OAM_ENDPOINT_CCM_PERIOD_1S;

    for (i=0; i<6; i++) {
        remote_endpoint_info[index].src_mac_address[i] = remote_mac[i]; 
    }

    rc = bcm_oam_endpoint_create(unit, &remote_endpoint_info[index]);
    
    if (rc != BCM_E_NONE) {
        printf("bcm_oam_endpoint_create (remote) failed: %d (%s)\n", rc, bcm_errmsg(rc));
        return rc;
    }
    
    printf("Remote oam endpoint 0x%08x created\n", remote_endpoint_info[index].id);

    return rc;
}

int caladan3_oam_ccm_setup(int port, 
                           int vlan)
{
    int                     unit = 0;
    int                     rc = BCM_E_NONE;
    bcm_oam_event_types_t   oam_events;
    bcm_mac_t               local_mac    = {0x01, 0x02, 0x03, 0x04, 0x5, 0x6};
    bcm_mac_t               remote_mac   = {0x7, 0x8, 0x9, 0xa, 0xb, 0xc};
    int                     local_id;
    int                     remote_mepid = 200 + port;
    int                     peer_port    = port;
    int                     group_id = 0;
    int                     mdlevel;

    BCM_OAM_EVENT_TYPE_CLEAR_ALL(oam_events);
    BCM_OAM_EVENT_TYPE_SET(oam_events, bcmOAMEventEndpointCCMTimeout);
    bcm_oam_event_register(unit, oam_events, caladan3_oam_callback, (auto)0);

    rc = caladan3_oam_group_create(unit, maid_str1, &group_id);
    if (rc != BCM_E_NONE) {
        printf("group create failed %d (%s)\n", rc, bcm_errmsg(rc));
        return rc;
    }

    mdlevel = 3;
    rc = caladan3_oam_endpoints_create(unit, port, vlan, mdlevel, group_id, 0 /* 1st set at mdlevel 3 */);
    if (rc != BCM_E_NONE) {
        printf("1st set endpoints create failed mdlevel(%d) %d (%s)\n", mdlevel, rc, bcm_errmsg(rc));
        return rc;
    }

   mdlevel = 5;
   rc = caladan3_oam_endpoints_create(unit, port, vlan, mdlevel, group_id, 1 /* 2nd set at mdlevel 5 */);
    if (rc != BCM_E_NONE) {
        printf("2nd set endpoints create failed mdlevel(%d) %d (%s)\n", mdlevel, rc, bcm_errmsg(rc));
        return rc;
    }

    return rc;
}


int caladan3_oam_ccm_breakdown()
{
    int                     unit = 0;
    int                     rc = BCM_E_NONE;
    bcm_oam_event_types_t   oam_events;
    int                     index;


    for (index=0; index<2; index++) {
        rc = bcm_oam_endpoint_destroy(unit, remote_endpoint_info[index].id);
        if (rc != BCM_E_NONE) {
            printf("bcm_oam_endpoint_destroy (remote) failed: %d (%s)\n", rc, bcm_errmsg(rc));
            return rc;
        }

        printf("remote oam endpoint 0x%08x destroyed\n", remote_endpoint_info[index].id);

    
        rc = bcm_oam_endpoint_destroy(unit, local_endpoint_info[index].id);
    
        if (rc != BCM_E_NONE) {
            printf("bcm_oam_endpoint_destroy (local) failed: %d (%s)\n", rc, bcm_errmsg(rc));
            return rc;
        }
        
        printf("local oam endpoint 0x%08x destroyed\n", local_endpoint_info[index].id);
        
    }
    rc = bcm_oam_group_destroy(unit, group_info.id);
    if (rc != BCM_E_NONE) {
        printf("bcm_oam_group_destroy failed: %d (%s)\n", rc, bcm_errmsg(rc));
        return rc;
    }
    printf("group 0x%08x destroyed\n", group_info.id);

    BCM_OAM_EVENT_TYPE_CLEAR_ALL(oam_events);
    BCM_OAM_EVENT_TYPE_SET(oam_events, bcmOAMEventEndpointCCMTimeout);
    bcm_oam_event_unregister(unit, oam_events, caladan3_oam_callback);

    printf("oam event unregistered\n");
    return rc;
}
