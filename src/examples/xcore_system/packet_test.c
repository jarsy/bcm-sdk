/*
 * $Id: packet_test.c,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 */

/* This script was developed and tested using the cint interpreter */

/* Global variables */
int         configuration_completed = FALSE;
int         c3_unit;
bcm_vlan_t  vid;
bcm_pbmp_t  pbmp, upbmp;
int         ingress_port;
int         egress_port;


/*
 *  This functions scans the set of units to find which
 * unit is the C3.
 */
int get_c3_unit(void)
{
    int                 i;
    bcm_info_t          unit_info;
    int                 rc;


    /* Find C3 unit */
    for (i = 0; bcm_unit_valid(i); i++) {
        rc = bcm_info_get(i, &unit_info);
        if (BCM_FAILURE(rc)) {
            printf("bcm_info_get failed for unit %d, error: %s\n", i, bcm_errmsg(rc));
            return rc;
        }
        printf("Unit: %d, device: 0x%x, revision: %d\n", i, unit_info.device, unit_info.revision);
        if (unit_info.device == 0x0038) {
            return i;
        }
    }

    return  BCM_E_NOT_FOUND;

}

/*
 *  This function performs the general setup for the packet forwarding.
 */
int test_setup(void)
{
    int                     rc;
    bcm_vlan_control_vlan_t vlan_control;


    if (configuration_completed) {
        return BCM_E_NONE;
    }
    printf("test_setup: Configuring for testing\n");

    /* Get the C3 unit number */
    c3_unit = get_c3_unit();
    if (c3_unit < 0) {
        printf("l2_forward_1: Failed to find C3\n");
        return BCM_E_FAIL;
    }

    /* Set the C3 modid */
    rc = bcm_stk_modid_set(c3_unit, 0);
    if (rc != BCM_E_NONE) {
        printf("test_setup: bcm_stk_modid_set failed: %s\n", bcm_errmsg(rc));
        return rc;
    }

    /* Put the HiGig in loopback - if not done by boot script */
    //bshell(c3_unit, "g3p1GlobalSet higig_loop_enable 1");

    /* Set ports */
    ingress_port = 0;
    egress_port = 1;

    /* Create vlan */
    vid = 3;
    rc = bcm_vlan_create(c3_unit, vid);
    if (rc != BCM_E_NONE && rc != BCM_E_EXISTS) {
        printf("test_setup: bcm_vlan_create failed: %s\n", bcm_errmsg(rc));
        return rc;
    }

    /* Add the test ports to the vlan */
    BCM_PBMP_CLEAR(pbmp);
    BCM_PBMP_CLEAR(upbmp);
    BCM_PBMP_PORT_ADD(pbmp, ingress_port);
    BCM_PBMP_PORT_ADD(pbmp, egress_port);
    rc = bcm_vlan_port_add(c3_unit, vid, pbmp, upbmp);
    if (rc != BCM_E_NONE) {
        printf("test_setup: bcm_vlan_port_add failed: %s\n", bcm_errmsg(rc));
        return rc;
    }

    /* Disable learning on the vlan */
    bcm_vlan_control_vlan_t_init(&vlan_control);
    vlan_control.flags = BCM_VLAN_LEARN_DISABLE;
    rc = bcm_vlan_control_vlan_set(c3_unit, vid, vlan_control);
    if (rc != BCM_E_NONE) {
        printf("test_setup: bcm_vlan_control_vlan_set failed: %s\n", bcm_errmsg(rc));
        return rc;
    }

    configuration_completed = TRUE;

    return BCM_E_NONE;
}

/*
 *  This function installs a single L2 forwarding entry.
 */
int l2_forward_1(void)
{
    int             rc;
    bcm_mac_t       dmac;
    int             modid;
    bcm_l2_addr_t   l2_addr;

    /* Make sure general configuration is complete */
    rc = test_setup();
    if (rc != BCM_E_NONE) {
        printf("l2_forward_1: test setup failed!\n");
        return rc;
    }

    /* Get C3 module ID */
    rc = bcm_stk_modid_get(c3_unit, &modid);
    if (rc != BCM_E_NONE) {
        printf("l2_forward_1: bcm_stk_modid_get failed: %s\n", bcm_errmsg(rc));
        return rc;
    }
    printf("l2_forward_1: C3 module ID: 0x%x\n", modid);

    /* Add an L2 forward entry */
    dmac[0] = 0x00;
    dmac[1] = 0x22;
    dmac[2] = 0x33;
    dmac[3] = 0x44;
    dmac[4] = 0x55;
    dmac[5] = 0x01;
    bcm_l2_addr_t_init(&l2_addr, dmac, vid);
    l2_addr.modid = modid;
    l2_addr.port = egress_port;
    l2_addr.flags = BCM_L2_STATIC;
    rc = bcm_l2_addr_add(c3_unit, &l2_addr);
    if (rc != BCM_E_NONE) {
        printf("l2_forward_1: bcm_l2_addr_add failed: %s\n", bcm_errmsg(rc));
        return rc;
    }

    return BCM_E_NONE;

}


/*
 *  This function creates an ingress qos map and attaches it to the
 * ingress port used in these tests. The qos map inverts the priority
 * in the vlan tag of a received packet and writes it to the rcos
 * field in the route header.
 */
int setup_ingress_qos_map(void)
{
    int             rc;
    int             ingressMapId;
    bcm_qos_map_t   qosMap;
    int             pktPri;

    /* Make sure general configuration is complete */
    rc = test_setup();
    if (rc != BCM_E_NONE) {
        printf("setup_ingress_qos_map: test_setup failed!\n");
        return rc;
    }

    /* Create ingress QOS map */
    rc = bcm_qos_map_create(c3_unit, BCM_QOS_MAP_L2 | BCM_QOS_MAP_INGRESS, &ingressMapId);
    if (rc != BCM_E_NONE) {
        printf("setup_ingress_qos_map: bcm_qos_map_create failed: %s\n", bcm_errmsg(rc));
        return rc;
    }
    printf("setup_ingress_qos_map: ingress qos map id: %d\n", ingressMapId);

    /* Fill in ingress QOS map */
    for (pktPri = 0; pktPri < 8; pktPri++) {
        bcm_qos_map_t_init(&qosMap);

        /* Lookup keys */
        qosMap.pkt_pri = pktPri;
        qosMap.pkt_cfi = 0;

        /* Simple inverse mapping */
        qosMap.remark_int_pri = 7 - pktPri;
        qosMap.int_pri = pktPri;
        qosMap.policer_offset = pktPri;

        /* Add this mapping */
        rc = bcm_qos_map_add(c3_unit, BCM_QOS_MAP_L2, &qosMap, ingressMapId);
        if (rc != BCM_E_NONE) {
            printf("setup_ingress_qos_map: bcm_qos_map_add ingress %d failed: %s\n",
                pktPri, bcm_errmsg(rc));
            return rc;
        }
    }

    /* Set the map on the ingress port */
    rc = bcm_qos_port_map_set(c3_unit, ingress_port, ingressMapId, -1);
    if (rc != BCM_E_NONE) {
        printf("setup_ingress_qos_map: bcm_qos_port_map_set ingress failed: %s\n", bcm_errmsg(rc));
        return rc;
    }

}


/*
 *  This function creates an egress qos map and attaches it to the
 * egress port used in these tests. The qos map inverts the priority
 * in that is carried in the rcos field of the ERH and writes it to
 * priority field in the vlan tag.
 */
int setup_egress_qos_map(void)
{
    int             rc;
    int             egressMapId;
    bcm_qos_map_t   qosMap;
    int             intPri;

    /* Make sure general configuration is complete */
    rc = test_setup();
    if (rc != BCM_E_NONE) {
        printf("setup_egress_qos_map: test_setup failed!\n");
        return rc;
    }

    /* Create egress QOS map */
    rc = bcm_qos_map_create(c3_unit, BCM_QOS_MAP_L2 | BCM_QOS_MAP_EGRESS, &egressMapId);
    if (rc != BCM_E_NONE) {
        printf("setup_egress_qos_map: bcm_qos_map_create failed: %s\n", bcm_errmsg(rc));
        return rc;
    }
    printf("setup_egress_qos_map: egress qos map id: %d\n", egressMapId);

    /* Fill in egress QOS map */
    for (intPri = 0; intPri < 8; intPri++) {
        bcm_qos_map_t_init(&qosMap);

        /* Lookup keys */
        qosMap.remark_int_pri = intPri;
        qosMap.remark_color = 0;

        /* Simple inverse mapping */
        qosMap.pkt_pri = 7 - intPri;

        /* Add this mapping */
        rc = bcm_qos_map_add(c3_unit, BCM_QOS_MAP_L2, &qosMap, egressMapId);
        if (rc != BCM_E_NONE) {
            printf("setup_egress_qos_map: bcm_qos_map_add egress %d failed: %s\n",
                intPri, bcm_errmsg(rc));
            return rc;
        }
    }

    /* Set the map on the egress port */
    rc = bcm_qos_port_map_set(c3_unit, egress_port, -1, egressMapId);
    if (rc != BCM_E_NONE) {
        printf("setup_egress_qos_map: bcm_qos_port_map_set egress failed: %s\n", bcm_errmsg(rc));
        return rc;
    }

    return BCM_E_NONE;

}


