/*
 * $Id: caladan3_oam_ccm_extended.c,v 1.5 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 */
/*
 * CALADAN3 OAM 
 *
 * Basic OAM Continuity Check extended group/endpoint setup script
 *
 * This script configures 32 maintenance groups and adds a local and remote
 * down MEP CCM endpoint for each group.  Current support is for DOWN MEPs
 * only.
 *
 * The purpose of this script is to test the setup and breakdown of 32 maintenance groups
 * groups and associated endpoints.
 *
    port xe6 loopback=mac
    port xe7 loopback=mac
    cd ../../../../src/examples/xcore_system
    cint caladan3_oam_ccm_extended.c
    cint
    int port_start = 6;
    int port_end   = 7;
    int vlan_start = 2;
    int vlan_end   = 33;
    caladan3_oam_ccm_setup_extended(port_start, port_end, vlan_start, vlan_end);
    int group_count=64;
    caladan3_oam_ccm_breakdown_extended(group_count);
*/  

int group_id;
int local_endpoint_id;
int remote_endpoint_id;
bcm_oam_group_info_t    group_info[64];
bcm_oam_endpoint_info_t local_endpoint_info[64];
bcm_oam_endpoint_info_t remote_endpoint_info[64];

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


int caladan3_oam_ccm_setup(int port, int vlan, int index)
{
    int                     unit = 0;
    int                     rc = BCM_E_NONE;
    bcm_oam_event_types_t   oam_events;
    bcm_module_t            modid      = 0;
    bcm_mac_t               local_mac    = {0x00, 0x02, 0x03, 0x04, 0x05, 0x06};
    bcm_mac_t               remote_mac   = {0x00, 0x08, 0x09, 0x0a, 0x0b, 0x0c};
    int                     local_id;
    int                     i;
    int                     remote_mepid = 200 + index;
    int                     peer_port    = port;

    if (index > 32767-1) {
        return BCM_E_RESOURCE;
    }

    BCM_OAM_EVENT_TYPE_CLEAR_ALL(oam_events);
    BCM_OAM_EVENT_TYPE_SET(oam_events, bcmOAMEventEndpointCCMTimeout);
    bcm_oam_event_register(unit, oam_events, caladan3_oam_callback, (auto)0);

    /* 
     * Initialize group info structure
     */
    bcm_oam_group_info_t_init(&group_info[index]);

    sprintf(group_info[index].name, "Maintenance Identifier %d", index);

    rc = bcm_oam_group_create(unit, &group_info[index]);
    if (rc != BCM_E_NONE) {
        printf("bcm_oam_group_create failed: %d (%s)\n", rc, bcm_errmsg(rc));
        return rc;
    }

    group_id = group_info[index].id;

    if ((index & 0xf) > 14) {
        printf("group(%d) created\n", group_info[index].id);
    }
    rc = bcm_stk_my_modid_get(unit, &modid);
    if (rc != BCM_E_NONE) {
        printfs("bcm_stk_my_modid_get failed: %d (%s)\n", rc, bcm_errmsg(rc));
        return rc;
    }

    bcm_oam_endpoint_info_t_init(&local_endpoint_info[index]);
    local_endpoint_info[index].name =  100 + index;

    /* local endpoint */
    
    BCM_GPORT_MODPORT_SET(local_endpoint_info[index].gport, modid, port);
    local_endpoint_info[index].group      = group_id;
    local_endpoint_info[index].vlan       = vlan;
    local_endpoint_info[index].level      = 3;
    local_endpoint_info[index].flags      = 0; /* don't enable bubble */
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

    /* printf("Local oam endpoint 0x%08x created\n", local_endpoint_info[index].id); */

    local_endpoint_id = local_endpoint_info[index].id;

    bcm_oam_endpoint_info_t_init(&remote_endpoint_info[index]);
    remote_endpoint_info[index].name = 200 + port;

    remote_endpoint_info[index].local_id = local_endpoint_info[index].id;
    
    /* remote endpoint */
    BCM_GPORT_MODPORT_SET(remote_endpoint_info[index].gport, modid, peer_port);
    remote_endpoint_info[index].group      = group_id;
    remote_endpoint_info[index].vlan       = vlan;
    remote_endpoint_info[index].name       = remote_mepid;
    remote_endpoint_info[index].level      = 3;
    remote_endpoint_info[index].flags      = BCM_OAM_ENDPOINT_REMOTE;
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
    /* printf("Remote oam endpoint 0x%08x created\n", remote_endpoint_info[index].id); */
    
    /* Start CCM transmit - use replace flag with_id to enable ccm transmit */
    local_endpoint_info[index].flags  = BCM_OAM_ENDPOINT_REPLACE;
    local_endpoint_info[index].flags |= BCM_OAM_ENDPOINT_PORT_STATE_TX;
    local_endpoint_info[index].flags |= BCM_OAM_ENDPOINT_WITH_ID;

    rc = bcm_oam_endpoint_create(unit, &local_endpoint_info[index]);
    if (rc != BCM_E_NONE) {
        printf("bcm_oam_endpoint_create (local) failed: %d (%s)\n", rc, bcm_errmsg(rc));
        return rc;
    }

    /* Start CCM reception - use replace flag with id to enable ccm receive watchdog start */
    remote_endpoint_info[index].flags |= BCM_OAM_ENDPOINT_REPLACE;
    remote_endpoint_info[index].flags |= BCM_OAM_ENDPOINT_CCM_RX;
    remote_endpoint_info[index].flags |= BCM_OAM_ENDPOINT_WITH_ID;

    rc = bcm_oam_endpoint_create(unit, &remote_endpoint_info[index]);
    if (rc != BCM_E_NONE) {
        printf("bcm_oam_endpoint_create (local) failed: %d (%s)\n", rc, bcm_errmsg(rc));
        return rc;
    }
    return rc;
}
int caladan3_oam_ccm_setup_extended(bcm_port_t port_start, bcm_port_t port_end, int vlan_start, int vlan_end)
{
    int rv = BCM_E_NONE;
    int port;
    int vlan;
    int index = 0;
    for (port = port_start; port <= port_end; port++) {
        for (vlan = vlan_start; vlan <= vlan_end; vlan++) {
            rv = caladan3_oam_ccm_setup(port, vlan, index); 
            if (BCM_FAILURE(rv)) {
                printf("port(%d) vlan(%d) OAM CCM failure\n", port, vlan);
                return rv;
            }
            index++;
        }
    }
    printf("setup(%d) ccm groups with local and remote endpoints\n", index);
    return rv;
}
int caladan3_oam_ccm_breakdown_extended(int group_count)
{
    int rv = BCM_E_NONE;
    int index = 0;
    for (index=0; index<group_count; index++) {
        
        rv = caladan3_oam_ccm_breakdown(index); 
        if (BCM_FAILURE(rv)) {
            printf("index(%d) OAM CCM breakdown failure\n", index);
            return rv;
        }
    }
    printf("breakdown(%d) ccm groups and related endpoints\n", index);
    return rv;
}

int caladan3_oam_ccm_breakdown(int index)
{
    int                     unit = 0;
    int                     rc = BCM_E_NONE;
    bcm_oam_event_types_t   oam_events;
 
    

    rc = bcm_oam_endpoint_destroy(unit, remote_endpoint_info[index].id);
    if (rc != BCM_E_NONE) {
        printf("bcm_oam_endpoint_destroy (remote) failed: %d (%s)\n", rc, bcm_errmsg(rc));
        return rc;
    }
    /*    printf("remote oam endpoint 0x%08x destroyed\n", remote_endpoint_info[index]); */

    rc = bcm_oam_endpoint_destroy(unit, local_endpoint_info[index].id);
    
    if (rc != BCM_E_NONE) {
        printf("bcm_oam_endpoint_destroy (local) failed: %d (%s)\n", rc, bcm_errmsg(rc));
        return rc;
    }
    /* printf("local oam endpoint 0x%08x destroyed\n", local_endpoint_info[index].id); */

    
    rc = bcm_oam_group_destroy(unit, group_info[index].id);
    if (rc != BCM_E_NONE) {
        printf("bcm_oam_group_destroy failed: %d (%s)\n", rc, bcm_errmsg(rc));
        return rc;
    }
    printf("group 0x%08x destroyed\n", group_info[index].id);

    return rc;
}
