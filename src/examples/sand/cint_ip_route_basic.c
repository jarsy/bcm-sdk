/*
 * $Id: cint_ip_route_basic.c,v 1.34 Broadcom SDK $
 $Copyright: (c) 2016 Broadcom.
 Broadcom Proprietary and Confidential. All rights reserved.$ 
 */
/*
 * Example of a simple-router scenario 
 * Test Scenario 
 *
 * ./bcm.user
 * cint src/examples/dpp/utility/cint_utils_global.c
 * cint src/examples/dpp/cint_ip_route_basic.c
 * cint
 * basic_example(0,200,202);
 * exit;
 *
 * ETH-RIF packet 
 * tx 1 psrc = 200 data = 0x000c00020000000007000100080045000035000000008000fa45c08001017fffff02000102030405060708090a0b0c0d0e0f101112131415161718191a1b1c1d1e1f20
 * Received packets on unit 0 should be: 
 * Data:
 * 0x00000000cd1d000c0002000181000064080045000035000000007f00fb45c08001017fffff02000102030405060708090a0b0c0d0e0f101112131415161718191a1b1c1d1e1f20 
 */
 
/*
 * Utilities APIs
 */
/*
 * Add Route <vrf, addr, mask> --> intf
 * - addr: IP address 32 bit value
 * - mask 1: to consider 0: to ingore, for example for /24 mask = 0xffffff00
 * - vrf: VRF ID
 * - intf: egress object created using create_l3_egress (FEC)
 *
 */
int
add_route(
    int unit,
    uint32 addr,
    uint32 mask,
    int vrf,
    int intf)
{
    int rc;
    bcm_l3_route_t l3rt;
    bcm_l3_route_t_init(l3rt);

    l3rt.l3a_subnet = addr;
    l3rt.l3a_ip_mask = mask;
    l3rt.l3a_vrf = vrf;
    l3rt.l3a_intf = intf;       /* FEC-ID */
    if(soc_property_get(unit,"enhanced_fib_scale_prefix_length", 0)) {
      l3rt.l3a_flags2 |= BCM_L3_FLAGS2_SCALE_ROUTE;
    }
    rc = bcm_l3_route_add(unit, l3rt);
    if (rc != BCM_E_NONE)
    {
        printf("bcm_l3_route_add failed: %x \n", rc);
    }
    return rc;
}

/*
 * Set In-Port default ETH-RIF:
 * - in_port: Incoming port ID
 * - eth_rif: ETH-RIF 
 */
int
in_port_intf_set(
    int unit,
    int in_port,
    int eth_rif)
{
    bcm_vlan_port_t vlan_port;
    int rc;

    bcm_vlan_port_t_init(&vlan_port);
    vlan_port.port = in_port;
    vlan_port.vsi = eth_rif;
    vlan_port.criteria = BCM_VLAN_PORT_MATCH_PORT;
    vlan_port.flags = BCM_VLAN_PORT_CREATE_INGRESS_ONLY;

    rc = bcm_vlan_port_create(unit, vlan_port);
    if (rc != BCM_E_NONE)
    {
        printf("Error, bcm_vlan_port_create\n");
        return rc;
    }

    return rc;
}

/*
 * Set Out-Port default properties:
 * - out_port: Outgoing port ID
 */
int
out_port_set(
    int unit,
    int out_port)
{
    bcm_vlan_port_t vlan_port;
    bcm_port_match_info_t match_info;
    int rc;

    bcm_vlan_port_t_init(&vlan_port);
    bcm_port_match_info_t_init(&match_info);

    vlan_port.criteria = BCM_VLAN_PORT_MATCH_NONE;
    vlan_port.flags = BCM_VLAN_PORT_CREATE_EGRESS_ONLY;

    rc = bcm_vlan_port_create(unit, vlan_port);
    if (rc != BCM_E_NONE)
    {
        printf("Error, bcm_vlan_port_create\n");
        return rc;
    }

    match_info.match = BCM_PORT_MATCH_PORT;
    match_info.flags = BCM_PORT_MATCH_EGRESS_ONLY;
    match_info.port = out_port;
    rc = bcm_port_match_add(unit, vlan_port.vlan_port_id, &match_info);
    if (rc != 0)
    {
        printf("Error, in bcm_port_match_add \n");
        return rc;
    }

    return rc;
}

/*
 * Set In-VSI ETH-RIF properties:
 * - eth_rif_id: ETH-RIF ID
 * - vrf: VRF ID
 */
int
intf_ingress_rif_set(
    int unit,
    int eth_rif_id,
    int vrf)
{
    bcm_l3_ingress_t l3_ing_if;
    int rc;

    bcm_l3_ingress_t_init(&l3_ing_if);
    l3_ing_if.flags = BCM_L3_INGRESS_WITH_ID;   /* must, as we update exist RIF */
    l3_ing_if.vrf = vrf;

    rc = bcm_l3_ingress_create(unit, l3_ing_if, eth_rif_id);
    if (rc != BCM_E_NONE)
    {
        printf("Error, bcm_l3_ingress_create\n");
        return rc;
    }

    return rc;
}

/*
 * Create VSI ETH-RIF and set initial properties:
 * - my_mac - My-MAC address
 */
int
intf_eth_rif_create(
    int unit,
    int eth_rif_id,
    bcm_mac_t my_mac)
{
    bcm_l3_intf_t l3if;
    int rc;

    /*
     * Initialize a bcm_l3_intf_t structure. 
     */
    bcm_l3_intf_t_init(&l3if);
    l3if.l3a_flags = BCM_L3_WITH_ID;

    /*
     * My-MAC 
     */
    l3if.l3a_mac_addr = my_mac;
    l3if.l3a_intf_id = l3if.l3a_vid = eth_rif_id;

    rc = bcm_l3_intf_create(unit, l3if);
    if (rc != BCM_E_NONE)
    {
        printf("Error, bcm_l3_intf_create %d\n", rc);
        return rc;
    }

    return rc;
}

/*
 * Set a FEC entry, without allocating ARP entry 
 * - fec_id - FEC-ID
 * - out_rif - Outgoing ETH-RIF
 * - arp - Outgoing ARP pointer
 * - out_port - destination Outgoing port
 */

int
l3__egress_only_fec__create(
    int unit,
    int fec_id,
    int out_rif,
    int arp,
    int out_port)
{
    int rc;
    bcm_l3_egress_t l3eg;
    bcm_if_t l3egid;
    bcm_l3_egress_t_init(&l3eg);

    /*
     * FEC properties 
     */
    l3eg.intf = out_rif;
    l3eg.port = out_port;
    l3eg.encap_id = arp;
    l3egid = fec_id;

    rc = bcm_l3_egress_create(unit, BCM_L3_INGRESS_ONLY | BCM_L3_WITH_ID, &l3eg, &l3egid);
    if (rc != BCM_E_NONE)
    {
        printf("Error, create egress object, unit=%d, \n", unit);
        return rc;
    }

    return rc;
}

/*
 * Set an ARP entry, without allocating FEC entry
 * - arp_id - ARP-ID
 * - da - Destination MAC addresss
 */
int
l3__egress_only_encap__create(
    int unit,
    int arp_id,
    bcm_mac_t da)
{

    int rc;
    bcm_l3_egress_t l3eg;
    bcm_if_t l3egid_null;       /* not intersting */

    bcm_l3_egress_t_init(&l3eg);

    l3eg.mac_addr = da;
    l3eg.encap_id = arp_id;

    rc = bcm_l3_egress_create(unit, BCM_L3_EGRESS_ONLY, &l3eg, &l3egid_null);
    if (rc != BCM_E_NONE)
    {
        printf("Error, create egress object, \n");
        return rc;
    }

    return rc;
}

/******* Run example ******/

/*
 * packet will be routed from in_port to out-port 
 *
 * Route:
 * packet to send:
 *  - in port = in_port
 *  - DA = {0x00, 0x0c, 0x00, 0x02, 0x00, 0x00}
 *  - DIP = 0x7fffff00-0x7fffff0f except 0x7fffff03
 * expected:
 *  - out port = out_port
 *  - vlan = 100.
 *  - DA = {0x20, 0x00, 0x00, 0x00, 0xcd, 0x1d}
 *  - SA = {0x00, 0x0c, 0x00, 0x02, 0x00, 0x01}
 *  TTL decremented
 *
 */
/*
 * example: 
 int unit = 0;
 int in_port = 200;
 int out_port = 201;
 */
int
basic_example(
    int unit,
    int in_port,
    int out_port)
{
    int rv;
    int intf_in = 15;           /* Incoming packet ETH-RIF */
    int intf_out = 100;         /* Outgoing packet ETH-RIF */
    int fec = 5000;
    int vrf = 1;
    int encap_id = 9000;        /* ARP-Link-layer */
    bcm_mac_t intf_in_mac_address = { 0x00, 0x0c, 0x00, 0x02, 0x00, 0x00 };     /* my-MAC */
    bcm_mac_t intf_out_mac_address = { 0x00, 0x0c, 0x00, 0x02, 0x00, 0x01 };    /* my-MAC */
    bcm_mac_t arp_next_hop_mac = { 0x00, 0x00, 0x00, 0x00, 0xcd, 0x1d };        /* next_hop_mac */
    uint32 route = 0x7fffff00;
    uint32 mask = 0xfffffff0;

    /*
     * 1. Set In-Port to In ETh-RIF 
     */
    rv = in_port_intf_set(unit, in_port, intf_in);
    if (rv != BCM_E_NONE)
    {
        printf("Error, in_port_intf_set intf_in\n");
        return rv;
    }

    /*
     * 2. Set Out-Port default properties 
     */
    rv = out_port_set(unit, out_port);
    if (rv != BCM_E_NONE)
    {
        printf("Error, out_port_intf_set intf_out\n");
        return rv;
    }

    /*
     * 3. Create ETH-RIF and set its properties 
     */
    rv = intf_eth_rif_create(unit, intf_in, intf_in_mac_address);
    if (rv != BCM_E_NONE)
    {
        printf("Error, intf_eth_rif_create intf_in\n");
        return rv;
    }
    rv = intf_eth_rif_create(unit, intf_out, intf_out_mac_address);
    if (rv != BCM_E_NONE)
    {
        printf("Error, intf_eth_rif_create intf_out\n");
    }

    /*
     * 4. Set Incoming ETH-RIF properties 
     */
    rv = intf_ingress_rif_set(unit, intf_in, vrf);
    if (rv != BCM_E_NONE)
    {
        printf("Error, intf_eth_rif_create intf_out\n");
    }

    /*
     * 5. Create FEC and set its properties 
     */
    rv = l3__egress_only_fec__create(unit, fec, intf_out, encap_id, out_port);
    if (rv != BCM_E_NONE)
    {
        printf("Error, create egress object FEC only\n");
    }

    /*
     * 6. Create ARP and set its properties
     */
    rv = l3__egress_only_encap__create(unit, encap_id, arp_next_hop_mac);
    if (rv != BCM_E_NONE)
    {
        printf("Error, create egress object ARP only\n");
    }

    /*
     * 7. Add Route entry
     */
    rv = add_route(unit, route, mask, vrf, fec);
    if (rv != BCM_E_NONE)
    {
        printf("Error, in function internal_ip_route, \n");
        return rv;
    }

    return rv;
}
