/* $Id: cint_ipv6_tunnel_term.c,v 1.4 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/
/* 
 * how to run:
cint;
cint_reset();
exit;
cint ../../../../src/examples/dpp/utility/cint_utils_l3.c 
cint ../../../../src/examples/dpp/cint_ip_route.c
cint ../../../../src/examples/dpp/cint_ip_tunnel_term.c 
cint ../../../../src/examples/dpp/cint_ipv6_tunnel_term.c 
cint
int rv;
verbose = 2; 
rv = ipv6_tunnel_term_example(unit,<in_port>,<out_port>);
 
tunnel no. 1 - from <in_port> run packet: 
    ethernet header with DA 00:0C:00:02:00:00, SA 00:00:07:00:01:00 and Vlan tag id 10
    ipv6 header with DIP ECEF:EEED:ECEB:EAE9:E8E7:E6E5:E4E3:E2E1 and any SIP
    ipv4 header with DIP 1.0.0.17 and SIP 192.128.1.4
the packet be forwarded to <out_port>, after termination of the ipv6 header, and arrive with: 
    ethernet header with DA 00:0C:00:02:00:01, SA 00:0C:00:02:00:00 and Vlan tag id 100
    ipv4 header with DIP 1.0.0.17 and SIP 192.128.1.4
 
tunnel no. 2 - from <in_port> run packet: 
    ethernet header with DA 00:0C:00:02:00:00, SA 00:00:07:00:01:00 and Vlan tag id 10
    ipv6 header with DIP 3555:5555:6666:6666:7777:7777:8888:8888 and any SIP
    ipv4 header with DIP 1.0.0.17 and SIP 192.128.1.5
the packet be forwarded to <out_port>, after termination of the ipv6 header, and arrive with: 
    ethernet header with DA 00:0C:00:02:00:02, SA 00:0C:00:02:00:00 and Vlan tag id 200
    ipv4 header with DIP 1.0.0.17 and SIP 192.128.1.5
*/ 

/* ********* 
  Globals/Aux Variables
 ********** */

/* debug prints */
int verbose = 1;

/********** 
  functions
 ********** */


/******* Run example ******/
 
/*
 * IP tunnel example 
 * - terminate IP tunnels. 
 * - do routing after termination
 */
/*
 * buid two IP tunnels termination
 * Tunnnel 1: 
 *   -  sip   = 160.0.0.17
 *   -  dip   =  161.0.0.17
 *   -  dscp  = 10;
 *   -  ttl   = 60;
 *   -  type  = bcmTunnelTypeIp4In4;
 *   -  Vlan  = 100
 *   -  Dmac  = 00:00:00:00:cd:1d
 *   -  Smac  = 00:0c:00:02:00:00
 *   + No Fec defined over this tunnel
 *  
 * Tunnnel 2: 
 *   -  sip   = 170.0.0.17
 *   -  dip   =  171.0.0.17
 *   -  dscp  = 11;
 *   -  ttl   = 50;
 *   -  type  = bcmTunnelTypeGre4In4;
 *   -  Vlan  = 100
 *   -  Dmac  = 20:00:00:00:cd:1d
 *   -  Smac  = 00:0c:00:02:00:00
 *   + Define FEC point to this tunnel
 *  
 *   returned value:
 *      intf_ids[] : array includes the interface-id to forward to the built tunnels.
 *         intf_ids[0] : is IP-tunnel Id.
 *         intf_ids[1] : is egress-object (FEC) points to the IP-tunnel
 */
int ipv6_tunnel_term_example(int unit, int in_port, int out_port){
    int rv;
    bcm_if_t eg_intf_ids[2]; /* to include egress router interfaces */
    bcm_gport_t in_tunnel_gports[2]; /* to include IP tunnel interfaces interfaces */
    bcm_if_t ll_encap_id;
    ip_tunnel_term_glbl_info.tunnel_type_1 = bcmTunnelTypeIpAnyIn6;
    ip_tunnel_term_glbl_info.tunnel_type_2 = bcmTunnelTypeIpAnyIn6;
    /*** build tunnel terminations ***/
    rv = ipv6_tunnel_term_build_tunnel_terms(unit,in_port,out_port,eg_intf_ids,in_tunnel_gports, &ll_encap_id); 
    if (rv != BCM_E_NONE) {
        printf("Error, ipv6_tunnel_term_build_tunnel_terms, in_port=%d, \n", in_port);
    }

    /*** add routes to egress interfaces ***/
    rv = ipv4_tunnel_term_add_routes(unit,eg_intf_ids, ll_encap_id);
    if (rv != BCM_E_NONE) {
        printf("Error, ipv4_tunnel_term_add_routes, eg_intf_ids=0x%08x,%0x%08x \n", eg_intf_ids[0],eg_intf_ids[1]);
    }

    return rv;
}


int tunnel_term_ipany_greany_ipv6(int unit, int in_port, int out_port){
    int rv;
    bcm_if_t eg_intf_ids[2]; /* to include egress router interfaces */
    bcm_gport_t in_tunnel_gports[2]; /* to include IP tunnel interfaces interfaces */
    bcm_if_t ll_encap_id;
    ip_tunnel_term_glbl_info.tunnel_type_1 = bcmTunnelTypeIpAnyIn6;
    ip_tunnel_term_glbl_info.tunnel_type_2 = bcmTunnelTypeIpAnyIn6;
    /*** build tunnel terminations ***/
    rv = ipv6_tunnel_term_build_tunnel_terms(unit,in_port,out_port,eg_intf_ids,in_tunnel_gports, &ll_encap_id); 
    if (rv != BCM_E_NONE) {
        printf("Error, ipv6_tunnel_term_build_tunnel_terms, in_port=%d, \n", in_port);
    }

    /*** add routes to egress interfaces ***/
    rv = ipv4_tunnel_term_add_routes(unit,eg_intf_ids, ll_encap_id);
    if (rv != BCM_E_NONE) {
        printf("Error, ipv4_tunnel_term_add_routes, eg_intf_ids=0x%08x,%0x%08x \n", eg_intf_ids[0],eg_intf_ids[1]);
    }

    return rv;
}
 
int ipv6_tunnel_term_build_tunnel_terms(int unit, int in_port, int out_port,bcm_if_t *eg_intf_ids, bcm_gport_t *in_tunnel_gports, bcm_if_t *ll_encap_id){
    int rv;

    int tunnel_vrf = ip_tunnel_term_glbl_info.tunnel_vrf; /* the router interface due to tunnel termination */
    int rif_vrf = ip_tunnel_term_glbl_info.rif_vrf;   /* the router interface from RIF (VSI) */
    int tunnel_rif; /* RIF from tunnel termination*/

    int egress_intf_1; /* egress object 1*/
    int egress_intf_2; /* egress object 2*/
    

    /* tunnel term 1 info*/
    bcm_tunnel_terminator_t tunnel_term_1;
    bcm_ip6_t dip1;
    bcm_ip6_t dip2;
    bcm_ip6_t dip_mask1;
    bcm_ip6_t dip_mask2;
    /* tunnel term 2 info: according to DIP  only*/
    bcm_tunnel_terminator_t tunnel_term_2;

    /*** build router interface ***/
    rv = ip_tunnel_term_open_route_interfaces(unit, in_port, out_port, rif_vrf, tunnel_vrf, &tunnel_rif, &egress_intf_1, &egress_intf_2, ll_encap_id);
    if (rv != BCM_E_NONE) {
        printf("Error, cip_tunnel_term_open_route_interfaces, in_port=%d, \n", in_port);
    }

    /* store egress interfaces, for routing */
    eg_intf_ids[0] = egress_intf_1;
    eg_intf_ids[1] = egress_intf_2;

    /* create IP tunnel terminators */
    /*** create IP tunnel terminator 1 ***/
    bcm_tunnel_terminator_t_init(&tunnel_term_1);
    /* key is DIP6 */
    sal_memcpy(tunnel_term_1.dip6, ip_tunnel_term_glbl_info.dip_ipv6, 16);
    sal_memcpy(tunnel_term_1.dip6_mask, ip_tunnel_term_glbl_info.mask_ipv6, 16);
    tunnel_term_1.tunnel_if = tunnel_rif;
    tunnel_term_1.type = ip_tunnel_term_glbl_info.tunnel_type_1; /* this is IPv6 termination */

    rv = bcm_tunnel_terminator_create(unit,&tunnel_term_1);
    if (rv != BCM_E_NONE) {
        printf("Error, bcm_tunnel_terminator_create_1, in_port=%d, \n", in_port);
    }
    if(verbose >= 1) {
        printf("created tunnel terminator_1 =0x%08x, \n", tunnel_term_1.tunnel_id);
        ip_tunnel_term_print_key("created term 1",&tunnel_term_1);
    }

    in_tunnel_gports[0] = tunnel_term_1.tunnel_id;
    
    /*** create IP tunnel terminator 2 ***/
    bcm_tunnel_terminator_t_init(&tunnel_term_2);
    /* key is DIP6 */
    sal_memcpy(tunnel_term_2.dip6, ip_tunnel_term_glbl_info.dip2_ipv6, 16);
    sal_memcpy(tunnel_term_2.dip6_mask, ip_tunnel_term_glbl_info.mask_ipv6, 16);
    tunnel_term_2.tunnel_if = -1; /* means don't overwite RIF */
    tunnel_term_2.type = ip_tunnel_term_glbl_info.tunnel_type_2;/* this is IPv4 termination */

    rv = bcm_tunnel_terminator_create(unit,&tunnel_term_2);
    if (rv != BCM_E_NONE) {
        printf("Error, bcm_tunnel_terminator_create_2, in_port=%d, \n", in_port);
    }
    if(verbose >= 1) {
        printf("created tunnel terminator_2 =0x%08x, \n", tunnel_term_2.tunnel_id);
        ip_tunnel_term_print_key("created term 2",&tunnel_term_2);
    }
    in_tunnel_gports[1] = tunnel_term_2.tunnel_id;

    return rv;
}
 
 
/*
 * buid two IP tunnels with following information:
 * Tunnnel 1: 
 *   -  dscp  = 10;
 *   -  ttl   = 60;
 *   -  type  = bcmTunnelTypeIp4In4;
 *   -  Vlan  = 100
 *   -  Dmac  = 00:00:00:00:cd:1d
 *   -  Smac  = 00:0c:00:02:00:00
 *   + No Fec defined over this tunnel
 *  
 * Tunnnel 2: 
 *   -  dscp  = 11;
 *   -  ttl   = 50;
 *   -  type  = bcmTunnelTypeGre4In4;
 *   -  Vlan  = 100
 *   -  Dmac  = 20:00:00:00:cd:1d
 *   -  Smac  = 00:0c:00:02:00:00
 *   + Define FEC point to this tunnel
 *  
 *   returned value:
 *      intf_ids[] : array includes the interface-id to forward to the built tunnels.
 *         intf_ids[0] : is IP-tunnel Id.
 *         intf_ids[1] : is egress-object (FEC) points to the IP-tunnel
 */
int ipv6_tunnel_build_tunnels(int unit, int out_port, int* intf_ids){
    int rv;
    int ing_intf_in;
    int ing_intf_out; 
    int fec[2] = {0x0,0x0};
    int flags;
    int out_vlan = 100;
    int encap_id[2]={0x0,0x0};
    bcm_mac_t mac_address  = {0x00, 0x0c, 0x00, 0x02, 0x00, 0x00};  /* my-MAC */
    bcm_tunnel_initiator_t tunnel_1;


    /* tunnel 1 info*/
    int dscp1 = 0; /* not supported for IPv6 */
    int flow_label1 = 0x53; 
    int ttl1 = 60; 
    int type1 = bcmTunnelTypeIp4In6; /*possible types Ip4In6 and Ip6In6 */
    bcm_mac_t next_hop_mac  = {0x00, 0x00, 0x00, 0x00, 0xcd, 0x1d}; /* next_hop_mac1 */
    int tunnel_itf1=0;
    bcm_ip6_t sip1;
    bcm_ip6_t dip1;
    /* tunnel 2 info */
    bcm_tunnel_initiator_t tunnel_2;
    int dscp2 = 0; /* not supported for IPv6 */
    int flow_label2 = 0x77;
    int ttl2 = 50;
    int type2= bcmTunnelTypeIp4In6;
    bcm_mac_t next_hop_mac2  = {0x20, 0x00, 0x00, 0x00, 0xcd, 0x1d}; /* next_hop_mac1 */
    int tunnel_itf2=0;
    bcm_ip6_t sip2;
    bcm_ip6_t dip2;


    sip1[15]= 0x01;    sip1[14]= 0x02;    sip1[13]= 0x03;    sip1[12]= 0x04;
    sip1[11]= 0x05;    sip1[10]= 0x06;    sip1[9] = 0x07;    sip1[8] = 0x08;
    sip1[7] = 0x09;    sip1[6] = 0x0a;    sip1[5] = 0x0b;    sip1[4] = 0x0c;
    sip1[3] = 0x0d;    sip1[2] = 0x0e;    sip1[1] = 0x0f;    sip1[0] = 0xfc;
  
    dip1[15]= 0xa1;    dip1[14]= 0xa2;    dip1[13]= 0xa3;    dip1[12]= 0xa4;
    dip1[11]= 0xa5;    dip1[10]= 0xa6;    dip1[9] = 0xa7;    dip1[8] = 0xa8;
    dip1[7] = 0xa9;    dip1[6] = 0xaa;    dip1[5] = 0xab;    dip1[4] = 0xac;
    dip1[3] = 0xad;    dip1[2] = 0xae;    dip1[1] = 0xaf;    dip1[0] = 0xac;
  
    sip2[15]= 0xc1;    sip2[14]= 0xc2;    sip2[13]= 0xc3;    sip2[12]= 0xc4;
    sip2[11]= 0xc5;    sip2[10]= 0xc6;    sip2[9] = 0xc7;    sip2[8] = 0xc8;
    sip2[7] = 0xc9;    sip2[6] = 0xca;    sip2[5] = 0xcb;    sip2[4] = 0xcc;
    sip2[3] = 0xcd;    sip2[2] = 0xce;    sip2[1] = 0xcf;    sip2[0] = 0xcc;
  
    dip2[15]= 0xe1;    dip2[14]= 0xe2;    dip2[13]= 0xe3;    dip2[12]= 0xe4;
    dip2[11]= 0xe5;    dip2[10]= 0xe6;    dip2[9] = 0xe7;    dip2[8] = 0xe8;
    dip2[7] = 0xe9;    dip2[6] = 0xea;    dip2[5] = 0xeb;    dip2[4] = 0xec;
    dip2[3] = 0xed;    dip2[2] = 0xee;    dip2[1] = 0xef;    dip2[0] = 0xec;
  
    /*** create egress router interface ***/
    rv = vlan__open_vlan_per_mc(unit, out_vlan, 0x1);  
    if (rv != BCM_E_NONE) {
        printf("Error, open_vlan=%d, in unit %d \n", out_vlan, unit);
    }
    rv = bcm_vlan_gport_add(unit, out_vlan, out_port, 0);
    if (rv != BCM_E_NONE && rv != BCM_E_EXISTS) {
        printf("fail add port(0x%08x) to vlan(%d)\n", out_port, out_vlan);
      return rv;
    }

    create_l3_intf_s intf;
    intf.vsi = out_vlan;
    intf.my_global_mac = mac_address;
    intf.my_lsb_mac = mac_address;
    
    rv = l3__intf_rif__create(unit, &intf);
    ing_intf_out = intf.rif;        
    if (rv != BCM_E_NONE) {
        printf("Error, l3__intf_rif__create\n");
    }
    
    /*** create IP tunnel 1 ***/
    bcm_tunnel_initiator_t_init(&tunnel_1);

    sal_memcpy(&(tunnel_1.dip6),&(dip1),16);
    sal_memcpy(&(tunnel_1.sip6),&(sip1),16);
    tunnel_1.flags = 0;
    tunnel_1.ttl = ttl1;
    tunnel_1.flow_label = flow_label1;
    tunnel_1.type = type1;
    tunnel_1.l3_intf_id = ing_intf_out;
    tunnel_itf1 = 0;
    rv = add_ip_tunnel(unit,&tunnel_itf1,&tunnel_1);
    if (rv != BCM_E_NONE) {
        printf("Error, add_ip_tunnel 1\n");
    }
    if(verbose >= 1) {
        printf("created IP tunnel_1 on intf:0x%08x \n",tunnel_itf1);
    }

    /*** create tunnel 2 ***/
    bcm_tunnel_initiator_t_init(&tunnel_2);
    sal_memcpy(&(tunnel_2.dip6),&(dip2),16);
    sal_memcpy(&(tunnel_2.sip6),&(sip2),16);
    tunnel_2.dscp = dscp2;
    tunnel_2.flags = 0;
    tunnel_2.ttl = ttl2;
    tunnel_2.flow_label = flow_label2;
    tunnel_2.type = type2;
    tunnel_2.l3_intf_id = ing_intf_out;
    tunnel_itf2 = 0;
    rv = add_ip_tunnel(unit,&tunnel_itf2,&tunnel_2);
    if (rv != BCM_E_NONE) {
        printf("Error, add_ip_tunnel 2\n");
    }
    if(verbose >= 1) {
        printf("created IP tunnel_2 on intf:0x%08x \n",tunnel_itf2);
    }

    /*** using egress object API set MAC address for tunnel 1 interface, without allocating FEC enty ***/
    create_l3_egress_s l3eg;
    sal_memcpy(l3eg.next_hop_mac_addr, next_hop_mac , 6);
    l3eg.out_tunnel_or_rif = tunnel_itf1;
    l3eg.vlan = out_vlan;
    l3eg.arp_encap_id = encap_id[0];

    rv = l3__egress_only_encap__create(unit,&l3eg);
    if (rv != BCM_E_NONE) {
        printf("Error, l3__egress_only_encap__create\n");
        return rv;
    }

    encap_id[0] = l3eg.arp_encap_id;
    if(verbose >= 1) {
        printf("no FEC is allocated FEC-id =0x%08x, \n", fec[0]);
        printf("next hop mac at encap-id 0x%08x, \n", encap_id[0]);
    }

    /*** create egress object 2: points into tunnel 2, with allocating FEC, and da-mac = next_hop_mac2  ***/
    create_l3_egress_s l3eg;
    sal_memcpy(l3eg.next_hop_mac_addr, next_hop_mac2 , 6);
    l3eg.out_tunnel_or_rif = tunnel_itf2;
    l3eg.fec_id = fec[1];
    l3eg.arp_encap_id = encap_id[1];

    rv = l3__egress__create(unit,&l3eg);
    if (rv != BCM_E_NONE) {
        printf("Error, l3__egress__create\n");
        return rv;
    }

    fec[1] = l3eg.fec_id;
    encap_id[1] = l3eg.arp_encap_id;
    if(verbose >= 1) {
        printf("created FEC-id =0x%08x, \n", fec[1]);
        printf("next hop mac at encap-id %08x, \n", encap_id[1]);
    }

   /* interface for tunnel_1 is IP-tunnel ID */
    intf_ids[0] = tunnel_itf1;

    /* interface for tunnel_2 is egress-object (FEC) */
    intf_ids[1] = fec[1];

    return rv;
}
