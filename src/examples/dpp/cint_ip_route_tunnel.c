/* $Id: cint_ip_route_tunnel.c,v 1.7 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */

/* 
 * Sequence example to  
 * test: 
 * run: 
 * BCM> cint utility/cint_utils_mpls.c  
 * BCM> cint cint_qos.c 
 * BCM> cint utility/cint_utils_l3.c
 * BCM> cint cint_mpls_lsr.c 
 * BCM> cint cint_ip_route.c 
 * BMC> cint cint_mutli_device_utils.c 
 * BCM> cint cint_ip_route_tunnel.c
 * BCM> cint
 * cint> int nof_units = <nof_units>;
 * cint> int units[nof_units] = {<unit1>, <unit2>,...};    
 * cint> int outP = 13;
 * cint> int inP = 13;
 * cint> int outSysport, inSysport;
 * cint> port_to_system_port(unit1, outP, &outSysport);
 * cint> port_to_system_port(unit2, inP, &inSysport);
 * example_ip_to_tunnel(units_ids, nof_units,<inSysport>, <outSysport>); 
 *  
 *  
 * Note: this cint also includes tests for 4 label push and protection 
 */

/* ********* 
  Globals/Aux Variables
 ********** */

/* debug prints */
int verbose = 1;


/*
 * When using MPLS tunnel, whether to add EL/EL+ELI
 * 0 - No entropy
 * 1 - EL
 * 2 - EL+ELI
 */
int add_el_eli = 0;

/********** 
  functions
 ********** */

/******* Run example ******/
 
/*
 * packet will be routed from in_port to out-port 
 * HOST: 
 * packet to send: 
 *  - in port = in_port
 *  - vlan 15.
 *  - DA = {0x00, 0x0c, 0x00, 0x02, 0x00, 0x00}
 *  - DIP = 0x7fffff03 (127.255.255.03)
 * expected: 
 *  - out port = out_port
 *  - vlan 100.
 *  - DA = {0x00, 0x00, 0x00, 0x00, 0xcd, 0x1d}
 *  - SA = {0x00, 0x0c, 0x00, 0x02, 0x00, 0x00}
 *  TTL decremented
 *  MPLS label: label 200, exp 2, ttl 20
 *  
 * Route: 
 * packet to send: 
 *  - in port = in_port
 *  - vlan = 15.
 *  - DA = {0x00, 0x0c, 0x00, 0x02, 0x00, 0x00}
 *  - DIP = 0x7fffff00-0x7fffff0f except 0x7fffff03
 * expected: 
 *  - out port = out_port
 *  - vlan = 100.
 *  - DA = {0x00, 0x00, 0x00, 0x00, 0xcd, 0x1d}
 *  - SA = {0x00, 0x0c, 0x00, 0x02, 0x00, 0x00}
 *  TTL decremented
 *  MPLS label: label 200, exp 2, ttl 20
 */
int preserve_dscp_per_lif = 0;
int mpls_tunnel_used = 0;
int example_ip_to_tunnel(int *units_ids, int nof_units, int in_sysport, int out_sysport){
    int rv;
    int unit, i;
    int ing_intf_in;
    int ing_intf_out; 
    int fec[2] = {0x0,0x0};
    int in_vlan = 15; 
    int out_vlan = 100;
    int vrf = 0;
    int host;
    int encap_id[2]={0x0,0x0}; 
    int open_vlan=1;
    int route;
    int mask; 
    int tunnel_id, tunnel_id2 = 0;
	int flags = 0;
    bcm_mac_t mac_address  = {0x00, 0x0c, 0x00, 0x02, 0x00, 0x00};  /* my-MAC */
    bcm_mac_t next_hop_mac  = {0x00, 0x00, 0x00, 0x00, 0xcd, 0x1d}; /* next_hop_mac1 */
    bcm_mac_t next_hop_mac2  = {0x00, 0x00, 0x00, 0x00, 0xcd, 0x1e}; /* next_hop_mac1 */

    mpls__egress_tunnel_utils_s mpls_tunnel_properties;
    mpls__egress_tunnel_utils_s mpls_tunnel_properties2;

    create_l3_intf_s intf;

    /*** create ingress router interface ***/
    ing_intf_in = 0;
    units_array_make_local_first(units_ids, nof_units, in_sysport);
    for (i = 0 ; i < nof_units ; i++){
        unit = units_ids[i];

		rv = vlan__open_vlan_per_mc(unit, in_vlan, 0x1);  
        if (rv != BCM_E_NONE) {
            printf("Error, open_vlan=%d, in unit %d \n", in_vlan, unit);
        }

        if (preserve_dscp_per_lif & 0x1) {
            bcm_vlan_port_t vlan_port;
            bcm_vlan_port_t_init(&vlan_port);
            vlan_port.port=in_sysport;
            vlan_port.criteria = BCM_VLAN_PORT_MATCH_PORT_VLAN;
            vlan_port.match_vlan = in_vlan;
            vlan_port.egress_vlan = in_vlan;
            vlan_port.vsi = in_vlan;
            rv = bcm_vlan_port_create(unit, &vlan_port);
            if (rv != BCM_E_NONE) {
                printf("fail create VLAN port, port(0x%08x), vlan(%d)\n", in_sysport, in_vlan);
                return rv;
            }
            
            rv = bcm_port_control_set(0, vlan_port.vlan_port_id, bcmPortControlPreserveDscpIngress, 1);
            if (rv != BCM_E_NONE) {
                printf("bcm_port_control_set bcmPortControlPreserveDscpIngress failed, port(0x08x)\n", vlan_port.vlan_port_id);
                return rv;
            }
        } else {
            rv = bcm_vlan_gport_add(unit, in_vlan, in_sysport, 0);
            if (rv != BCM_E_NONE && rv != BCM_E_EXISTS) {
                printf("fail add port(0x%08x) to vlan(%d)\n", in_sysport, in_vlan);
              return rv;
            }
        }

        intf.vsi = in_vlan;
        intf.my_global_mac = mac_address;
        intf.my_lsb_mac = mac_address;
        intf.vrf_valid = 1;
        intf.vrf = vrf;

        rv = l3__intf_rif__create(unit, &intf);
        ing_intf_in = intf.rif;        
        if (rv != BCM_E_NONE) {
            printf("Error, l3__intf_rif__create\n");
        }
    }

    /*** create egress router interface ***/
    ing_intf_out = 0;
    units_array_make_local_first(units_ids,nof_units,out_sysport);
    for (i = 0 ; i < nof_units ; i++){
        unit = units_ids[i];

		rv = vlan__open_vlan_per_mc(unit, out_vlan, 0x1);  
        if (rv != BCM_E_NONE) {
            printf("Error, open_vlan=%d, in unit %d \n", out_vlan, unit);
        }
        rv = bcm_vlan_gport_add(unit, out_vlan, out_sysport, 0);
        if (rv != BCM_E_NONE && rv != BCM_E_EXISTS) {
            printf("fail add port(0x%08x) to vlan(%d)\n", out_sysport, out_vlan);
          return rv;
        }

        intf.vsi = out_vlan;

        rv = l3__intf_rif__create(unit, &intf);
        ing_intf_out = intf.rif;        
        if (rv != BCM_E_NONE) {
            printf("Error, l3__intf_rif__create\n");
        }
    }

    /*** create egress object 1 ***/
    /*** Create tunnel ***/
    /* We're allocating a lif. out_sysport unit should be first, and it's already first */
    tunnel_id = 0;
    for (i = 0 ; i < nof_units ; i++){
        unit = units_ids[i];

        mpls_tunnel_properties.label_in = 200;
		mpls_tunnel_properties.label_out = 0;
		mpls_tunnel_properties.next_pointer_intf = ing_intf_out;
        if (add_el_eli >= 2) {
            mpls_tunnel_properties.flags |= (BCM_MPLS_EGRESS_LABEL_ENTROPY_ENABLE|
                                             BCM_MPLS_EGRESS_LABEL_ENTROPY_INDICATION_ENABLE);
            if (add_el_eli == 3) {
                mpls_tunnel_properties.label_out = 0x300;
                mpls_tunnel_properties.label_out_2 = 0x320;
                mpls_tunnel_properties.label_out_3 = 0x330;
            }
        }
        else if (add_el_eli == 1) {
            mpls_tunnel_properties.flags |= BCM_MPLS_EGRESS_LABEL_ENTROPY_ENABLE;
        }
        printf("Trying to create tunnel initiator\n");
		rv = mpls__create_tunnel_initiator__set(unit, &mpls_tunnel_properties);
		if (rv != BCM_E_NONE) {
           printf("Error, in mpls__create_tunnel_initiator__set\n");
           return rv;
        }
        /* having a tunnel id != 0 is equivalent to being WITH_ID*/

        tunnel_id = mpls_tunnel_used = mpls_tunnel_properties.tunnel_id;
        printf("Created Tunnel ID: 0x%08x\n",tunnel_id);

        mpls_tunnel_properties2.label_in = 200;
		mpls_tunnel_properties2.label_out = 0;
		mpls_tunnel_properties2.next_pointer_intf = ing_intf_out;
        if (add_el_eli >= 2) {
            mpls_tunnel_properties2.flags |= (BCM_MPLS_EGRESS_LABEL_ENTROPY_ENABLE|
                                             BCM_MPLS_EGRESS_LABEL_ENTROPY_INDICATION_ENABLE);
            if (add_el_eli == 3) {
                mpls_tunnel_properties2.label_out = 0x300;
                mpls_tunnel_properties2.label_out_2 = 0x320;
                mpls_tunnel_properties2.label_out_3 = 0x330;
            }
        }
        else if (add_el_eli == 1) {
            mpls_tunnel_properties2.flags |= BCM_MPLS_EGRESS_LABEL_ENTROPY_ENABLE;
        }
        printf("Trying to create tunnel initiator\n");
		rv = mpls__create_tunnel_initiator__set(unit, &mpls_tunnel_properties2);
		if (rv != BCM_E_NONE) {
           printf("Error, in mpls__create_tunnel_initiator__set\n");
           return rv;
        }
        /* having a tunnel id != 0 is equivalent to being WITH_ID*/

        tunnel_id2 = mpls_tunnel_properties2.tunnel_id;
        printf("Created Tunnel ID: 0x%08x\n",tunnel_id2);

        if (preserve_dscp_per_lif & 0x2) {
            bcm_gport_t gport;
            BCM_L3_ITF_LIF_TO_GPORT_TUNNEL(gport, tunnel_id);
            rv = bcm_port_control_set(unit, gport, bcmPortControlPreserveDscpEgress, 1);
            if (rv != BCM_E_NONE) {
                printf("bcm_port_control_set bcmPortControlPreserveDscpEgress failed, port(0x08x)\n", gport);
                return rv;
            }
        }
    }

    /*** Create egress object1 with the tunnel_id ***/
    for (i = 0 ; i < nof_units ; i++){
        create_l3_egress_s l3eg;
        unit = units_ids[i]; 
        sal_memcpy(l3eg.next_hop_mac_addr, next_hop_mac , 6);
        l3eg.allocation_flags = flags;
        l3eg.out_tunnel_or_rif = tunnel_id;
        l3eg.out_gport = out_sysport;
        l3eg.vlan = out_vlan;
        l3eg.fec_id = fec[0];
        l3eg.arp_encap_id = encap_id[0];

        rv = l3__egress__create(unit,&l3eg);
        if (rv != BCM_E_NONE) {
            printf("Error, l3__egress__create\n");
            return rv;
        }

        fec[0] = l3eg.fec_id;
        encap_id[0] = l3eg.arp_encap_id;
        if(verbose >= 1) {
            printf("created FEC-id =0x%08x, in unit %d \n", fec[0], unit);
            printf("next hop mac at encap-id %08x, in unit %d\n", encap_id[0], unit);
        }
        flags |= BCM_L3_WITH_ID;
    }

    /*** add host point to FEC ***/
    host = 0x7fffff03;
    /* Units order does not matter*/
    for (i = 0 ; i < nof_units ; i++){
        unit = units_ids[i];
        rv = add_host(unit, 0x7fffff03, vrf, fec[0]); 
        if (rv != BCM_E_NONE) {
            printf("Error, create egress object, in_sysport=%d, in unit %d \n", in_sysport, unit);
        }
    }
    flags = 0;
    
    /*** create egress object 2***/
    /* We're allocating a lif. out_sysport unit should be first, and it's already first */    
    for (i = 0 ; i < nof_units ; i++){
        create_l3_egress_s l3eg1;
        unit = units_ids[i];
        sal_memcpy(l3eg1.next_hop_mac_addr, next_hop_mac2 , 6);
        l3eg1.allocation_flags = flags; 
        l3eg1.out_tunnel_or_rif = tunnel_id2;
        l3eg1.vlan = out_vlan;
        l3eg1.out_gport = out_sysport;
        l3eg1.fec_id = fec[1];
        l3eg1.arp_encap_id = encap_id[1];

        rv = l3__egress__create(unit,&l3eg1);
        if (rv != BCM_E_NONE) {
            printf("Error, l3__egress__create\n");
            return rv;
        }

        fec[1] = l3eg1.fec_id;
        encap_id[1] = l3eg1.arp_encap_id;
        if(verbose >= 1) {
            printf("created FEC-id =0x%08x, \n in unit %d", fec[1], unit);
            printf("next hop mac at encap-id %08x, in unit %d\n", encap_id[1], unit);
        }
        flags |= BCM_L3_WITH_ID;
    }

    /*** add route point to FEC2 ***/
    route = 0x7fffff00;
    mask  = 0xfffffff0;
    /* Units order does not matter*/
    for (i = 0 ; i < nof_units ; i++){
        unit = units_ids[i];
        rv = add_route(unit, route, mask , vrf, fec[1]); 
        if (rv != BCM_E_NONE) {
            printf("Error, create egress object, in_sysport=%d in unit %d, \n", in_sysport, unit);
        }
    }


    return rv;
}
