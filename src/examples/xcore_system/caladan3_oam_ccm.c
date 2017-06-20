/*
 * $Id: caladan3_oam_ccm.c,v 1.5 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 */
/*
 * CALADAN3 OAM
 *
 * Basic OAM Continuity Check  setup script
 *
 * This script configures a maintenance group and adds a local and remote
 * down MEP CCM endpoint to a given port.  Current support is for DOWN MEPs
 * only.
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
    cint caladan3_oam_ccm.c
    cint
    int port=6;
    int vlan=2;
    int with_id=FALSE;
    int is_upmep=FALSE;
    caladan3_oam_ccm_setup(port, vlan, with_id, is_upmep); 
    caladan3_oam_ccm_setup_get();
* disable loopback and timeout should occur after ~3.5x interval (3.5s).
    quit;
    port xe6 loopback=none
    bcm>unit(0) oam timer id(0x00000001) forced_timeout(0) timer_active_when_forced(1)
    cint;
    caladan3_oam_ccm_breakdown();
*
* Some relevant data on packet transmitted:
smac 00:02:03:04:05:06
dmac 01:c2:80:00:00:33 <- mdlevel encoding
vlan 2
ccm packet (msgtype=1) mepid=106 from source
*
* To set up a port based CCM (no vlan tag) run the following:

    cd ../../../../src/examples/xcore_system
    port xe6 loopback=mac
    debugmod bcm oam
    cint caladan3_oam_ccm.c
    cint
    int port=6;
    int vlan=0;
    int with_id=FALSE;
    caladan3_oam_ccm_setup(port, vlan, with_id); 
    caladan3_oam_ccm_setup_get();
*
* WITH_ID test:
    cd ../../../../src/examples/xcore_system
    port xe6 loopback=mac
    debugmod bcm oam
    cint caladan3_oam_ccm.c
    cint
    int port=6;
    int vlan=2;
    int with_id=TRUE;
    caladan3_oam_ccm_setup(port, vlan, with_id); 

*/  
int group_id = 1;
int local_endpoint_id = 1;
int remote_endpoint_id = 2;
bcm_oam_group_info_t    group_info;
bcm_oam_group_info_t    group_info_get;
bcm_oam_endpoint_info_t local_endpoint_info;
bcm_oam_endpoint_info_t local_endpoint_info_get;
bcm_oam_endpoint_info_t remote_endpoint_info;
bcm_oam_endpoint_info_t remote_endpoint_info_get;

/* 
 *  48 char ASCII String
 *  Maintenance identifier 1: endpoints 106 and 206.
 */
char maid_str[48] = {0x4d, 0x61, 0x69, 0x6e, 0x74, 0x65, 0x6e, 0x61,
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


int caladan3_oam_ccm_setup(int port, int vlan, int with_id, int is_upmep)
{
    int                     unit = 0;
    int                     rc = BCM_E_NONE;
    bcm_oam_event_types_t   oam_events;
    bcm_module_t            modid      = 0;
    bcm_mac_t               local_mac    = {0x00, 0x02, 0x03, 0x04, 0x05, 0x06};
    bcm_mac_t               remote_mac   = {0x00, 0x08, 0x09, 0x0a, 0x0b, 0x0c};
    int                     local_id;
    int                     i;
    int                     remote_mepid = 200 + port;
    int                     peer_port    = port;


    BCM_OAM_EVENT_TYPE_CLEAR_ALL(oam_events);
    BCM_OAM_EVENT_TYPE_SET(oam_events, bcmOAMEventEndpointCCMTimeout);
    bcm_oam_event_register(unit, oam_events, caladan3_oam_callback, (auto)0);

    /* 
     * Initialize group info structure
     */
    bcm_oam_group_info_t_init(&group_info);
    
    for (i=0; i<BCM_OAM_GROUP_NAME_LENGTH; i++) {
        group_info.name[i] = maid_str[i];
    }

    if (with_id) {
        group_info.id = group_id;
        group_info.flags |= BCM_OAM_GROUP_WITH_ID;
    }
    rc = bcm_oam_group_create(unit, &group_info);
    if (rc != BCM_E_NONE) {
        printf("bcm_oam_group_create failed: %d (%s)\n", rc, bcm_errmsg(rc));
        return rc;
    }
    printf("OAM group %d created\n", group_info.id);
    group_id = group_info.id;

    rc = bcm_stk_my_modid_get(unit, &modid);
    if (rc != BCM_E_NONE) {
        printf("bcm_stk_my_modid_get failed: %d (%s)\n", rc, bcm_errmsg(rc));
        return rc;
    }

    bcm_oam_endpoint_info_t_init(&local_endpoint_info);
    local_endpoint_info.name = 100 + port;

    /* local endpoint */
    
    BCM_GPORT_MODPORT_SET(local_endpoint_info.gport, modid, port);
    local_endpoint_info.group      = group_id;
    local_endpoint_info.vlan       = vlan;
    local_endpoint_info.level      = 3;
    local_endpoint_info.flags      = 0; /* don't enable bubble */
    local_endpoint_info.type       = bcmOAMEndpointTypeEthernet;
    local_endpoint_info.ccm_period = BCM_OAM_ENDPOINT_CCM_PERIOD_1S;

    if (is_upmep) {
        local_endpoint_info.flags |= BCM_OAM_ENDPOINT_UP_FACING;
    }

    if (with_id) {
        local_endpoint_info.id     = local_endpoint_id;
        local_endpoint_info.flags |= BCM_OAM_ENDPOINT_WITH_ID;
    }

    for (i=0; i<6; i++) {
        local_endpoint_info.src_mac_address[i] = local_mac[i];
    }

    rc = bcm_oam_endpoint_create(unit, &local_endpoint_info);
    if (rc != BCM_E_NONE) {
        printf("bcm_oam_endpoint_create (local) failed: %d (%s)\n", rc, bcm_errmsg(rc));
        return rc;
    }

    printf("Local oam endpoint 0x%08x created\n", local_endpoint_info.id);

    local_endpoint_id = local_endpoint_info.id;

    bcm_oam_endpoint_info_t_init(&remote_endpoint_info);
    remote_endpoint_info.name = 200 + port;

    remote_endpoint_info.local_id = local_endpoint_info.id;
    
    /* remote endpoint */
    BCM_GPORT_MODPORT_SET(remote_endpoint_info.gport, modid, peer_port);
    remote_endpoint_info.group      = group_id;
    remote_endpoint_info.vlan       = vlan;
    remote_endpoint_info.name       = remote_mepid;
    remote_endpoint_info.level      = 3;
    remote_endpoint_info.flags      = BCM_OAM_ENDPOINT_REMOTE;
    remote_endpoint_info.type       = bcmOAMEndpointTypeEthernet;
    remote_endpoint_info.ccm_period = BCM_OAM_ENDPOINT_CCM_PERIOD_1S;

    if (is_upmep) {
        remote_endpoint_info.flags |= BCM_OAM_ENDPOINT_UP_FACING;
    }

    for (i=0; i<6; i++) {
        remote_endpoint_info.src_mac_address[i] = remote_mac[i]; 
    }

    if (with_id) {
        remote_endpoint_info.id     = remote_endpoint_id;
        remote_endpoint_info.flags |= BCM_OAM_ENDPOINT_WITH_ID;
    }    
    rc = bcm_oam_endpoint_create(unit, &remote_endpoint_info);
    
    if (rc != BCM_E_NONE) {
        printf("bcm_oam_endpoint_create (remote) failed: %d (%s)\n", rc, bcm_errmsg(rc));
        return rc;
    }
    printf("Remote oam endpoint 0x%08x created\n", remote_endpoint_info.id);
    
    /* Start CCM transmit - use replace flag with_id to enable ccm transmit and set RDI in packet */
    local_endpoint_info.flags  = BCM_OAM_ENDPOINT_REPLACE;
    local_endpoint_info.flags |= BCM_OAM_ENDPOINT_PORT_STATE_TX;
    local_endpoint_info.flags |= BCM_OAM_ENDPOINT_WITH_ID;
    local_endpoint_info.flags |= BCM_OAM_ENDPOINT_REMOTE_DEFECT_TX;

    if (is_upmep) {
        local_endpoint_info.flags |= BCM_OAM_ENDPOINT_UP_FACING;
    }

    rc = bcm_oam_endpoint_create(unit, &local_endpoint_info);
    if (rc != BCM_E_NONE) {
        printf("bcm_oam_endpoint_create (local) failed: %d (%s)\n", rc, bcm_errmsg(rc));
        return rc;
    }

    /* Start CCM reception - use replace flag with id to enable ccm receive watchdog start */
    remote_endpoint_info.flags |= BCM_OAM_ENDPOINT_REPLACE;
    remote_endpoint_info.flags |= BCM_OAM_ENDPOINT_CCM_RX;
    remote_endpoint_info.flags |= BCM_OAM_ENDPOINT_WITH_ID;

    if (is_upmep) {
        remote_endpoint_info.flags |= BCM_OAM_ENDPOINT_UP_FACING;
    }

    rc = bcm_oam_endpoint_create(unit, &remote_endpoint_info);
    if (rc != BCM_E_NONE) {
        printf("bcm_oam_endpoint_create (local) failed: %d (%s)\n", rc, bcm_errmsg(rc));
        return rc;
    }
    return rc;
}
int caladan3_oam_ccm_breakdown()
{
    int                     unit = 0;
    int                     rc = BCM_E_NONE;
    bcm_oam_event_types_t   oam_events;

 
    rc = bcm_oam_endpoint_destroy(unit, remote_endpoint_info.id);
    if (rc != BCM_E_NONE) {
        printf("bcm_oam_endpoint_destroy (remote) failed: %d (%s)\n", rc, bcm_errmsg(rc));
        return rc;
    }

    printf("remote oam endpoint 0x%08x destroyed\n", remote_endpoint_info.id);

    
    rc = bcm_oam_endpoint_destroy(unit, local_endpoint_info.id);
    
    if (rc != BCM_E_NONE) {
        printf("bcm_oam_endpoint_destroy (local) failed: %d (%s)\n", rc, bcm_errmsg(rc));
        return rc;
    }
    
    printf("local oam endpoint 0x%08x destroyed\n", local_endpoint_info.id);

    
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
int caladan3_oam_ccm_setup_get(void)
{
    int                     unit = 0;
    int                     rc = BCM_E_NONE;
    int                     local_id;
    int                     i;

    rc = bcm_oam_group_get(unit, group_info.id, &group_info_get);
    if (rc != BCM_E_NONE) {
        printf("bcm_oam_group_get failed: %d (%s)\n", rc, bcm_errmsg(rc));
        return rc;
    }
    for (i=0; i<BCM_OAM_GROUP_NAME_LENGTH; i++) {
        if (group_info_get.name[i] != maid_str[i]) {
            printf("group_info maid comparison failed byte(%d)\n", i);
            rc = BCM_E_FAIL;
            return rc;
        }
    }


    rc = bcm_oam_endpoint_get(unit, local_endpoint_info.id, &local_endpoint_info_get);
    if (rc != BCM_E_NONE) {
        printf("bcm_oam_endpoint_info_get (local) failed: %d (%s)\n", rc, bcm_errmsg(rc));
        rc = BCM_E_FAIL;
        return rc;
    }

    if (local_endpoint_info_get.name != local_endpoint_info.name) {
        printf("local_endpoint_info name comparison failed get(%d) written(%d)\n", 
               local_endpoint_info_get.name, local_endpoint_info.name);
        rc = BCM_E_FAIL;
        return rc;
    }

    if (local_endpoint_info_get.gport != local_endpoint_info.gport) {
        printf("local_endpoint_info gport comparison failed get(0x%x) written(0x%x)\n", 
               local_endpoint_info_get.gport, local_endpoint_info.gport);
        rc = BCM_E_FAIL;
        return rc;
    }
    if (local_endpoint_info_get.vlan != local_endpoint_info.vlan) {
        printf("local_endpoint_info vlan comparison failed get(0x%x) written(0x%x)\n", 
               local_endpoint_info_get.vlan, local_endpoint_info.vlan);
        rc = BCM_E_FAIL;
        return rc;
    }
    if (local_endpoint_info_get.level != local_endpoint_info.level) {
        printf("local_endpoint_info level comparison failed get(0x%x) written(0x%x)\n", 
               local_endpoint_info_get.level, local_endpoint_info.level);
        rc = BCM_E_FAIL;
        return rc;
    }
    if (local_endpoint_info_get.type != local_endpoint_info.type) {
        printf("local_endpoint_info type comparison failed get(0x%x) written(0x%x)\n", 
               local_endpoint_info_get.type, local_endpoint_info.type);
        rc = BCM_E_FAIL;
        return rc;
    }

    if (local_endpoint_info_get.ccm_period != local_endpoint_info.ccm_period) {
        printf("local_endpoint_info ccm_period comparison failed get(0x%x) written(0x%x)\n", 
               local_endpoint_info_get.ccm_period, local_endpoint_info.ccm_period);
        rc = BCM_E_FAIL;
        return rc;
    }

    for (i=0; i<6; i++) {
        if (local_endpoint_info_get.src_mac_address[i] != local_endpoint_info.src_mac_address[i]) {
            printf("local_endpoint_info src_mac_address comparison failed byte(%d) get(0x%x) written(0x%x)\n", 
                   i, local_endpoint_info_get.src_mac_address[i], local_endpoint_info.src_mac_address[i]);
            rc = BCM_E_FAIL;
            return rc;
        }
    }


    rc = bcm_oam_endpoint_get(unit, remote_endpoint_info.id, &remote_endpoint_info_get);
    
    if (rc != BCM_E_NONE) {
        printf("bcm_oam_endpoint_get (remote) failed: %d (%s)\n", rc, bcm_errmsg(rc));
        return rc;
    }
    
    if (remote_endpoint_info_get.name != remote_endpoint_info.name) {
        printf("remote_endpoint_info name comparison failed get(%d) written(%d)\n", 
               remote_endpoint_info_get.name, remote_endpoint_info.name);
        rc = BCM_E_FAIL;
        return rc;
    }

    if (remote_endpoint_info_get.gport != remote_endpoint_info.gport) {
        printf("remote_endpoint_info gport comparison failed get(0x%x) written(0x%x)\n", 
               remote_endpoint_info_get.gport, remote_endpoint_info.gport);
        rc = BCM_E_FAIL;
        return rc;
    }
    if (remote_endpoint_info_get.vlan != remote_endpoint_info.vlan) {
        printf("remote_endpoint_info vlan comparison failed get(0x%x) written(0x%x)\n", 
               remote_endpoint_info_get.vlan, remote_endpoint_info.vlan);
        rc = BCM_E_FAIL;
        return rc;
    }
    if (remote_endpoint_info_get.level != remote_endpoint_info.level) {
        printf("remote_endpoint_info level comparison failed get(0x%x) written(0x%x)\n", 
               remote_endpoint_info_get.level, remote_endpoint_info.level);
        rc = BCM_E_FAIL;
        return rc;
    }
    if (remote_endpoint_info_get.type != remote_endpoint_info.type) {
        printf("remote_endpoint_info type comparison failed get(0x%x) written(0x%x)\n", 
               remote_endpoint_info_get.type, remote_endpoint_info.type);
        rc = BCM_E_FAIL;
        return rc;
    }

    if (remote_endpoint_info_get.ccm_period != remote_endpoint_info.ccm_period) {
        printf("remote_endpoint_info ccm_period comparison failed get(0x%x) written(0x%x)\n", 
               remote_endpoint_info_get.ccm_period, remote_endpoint_info.ccm_period);
        rc = BCM_E_FAIL;
        return rc;
    }

    for (i=0; i<6; i++) {
        if (remote_endpoint_info_get.src_mac_address[i] != remote_endpoint_info.src_mac_address[i]) {
            printf("remote_endpoint_info src_mac_address comparison failed byte(%d) get(0x%x) written(0x%x)\n", 
                   i, remote_endpoint_info_get.src_mac_address[i], remote_endpoint_info.src_mac_address[i]);
            rc = BCM_E_FAIL;
            return rc;
        }
    }

    if (remote_endpoint_info_get.faults & BCM_OAM_ENDPOINT_FAULT_REMOTE) {
        printf("Expected RDI fault indicated from the remote endpoint.\n");
    }
    printf("get validation succeeded\n");
    return rc;
}
