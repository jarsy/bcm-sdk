/*~~~~~~~~~~~~~~~~~~~~~~~Bridge Router: IPMC~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/* $Id: cint_igmp_example.c,v 1.25 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File: cint_igmp_example.c
 * Purpose: Example shows how to configure IGMP snooping and IP compatible-MC groups to
 * local copies.
 * The following CINT tries to apply IGMP and IP compatible-MC to local copies on
 * regular Router with packet IPoE and Overlay such as VXLAN and L2VPN such as VPLS.
 * Note: Overlay and L2VPN traffic is supported from ARAD+.
 
 * Test IGMP snooping for IGMPoIPoETH packets
 *                        IGMPoIPoETHoVXLANoUDPoIPoEth packets
 *                           after vxlan tunnel termination
 *                        IGMPoIPoETHoPWEoMPLSoEth packets
 *                           after vpls tunnel termination
 * Test local copies multicast for IPoEth packets.
 *                        IPoETHoVXLANoUDPoIPoEth packets
 *                           after vxlan tunnel termination
 *                        IPoETHoPWEoMPLSoEth packets
 *                           after vpls tunnel termination
 
 * Calling sequence:
 *     IGMP snooping:
 *      - Configure device to support igmp on incoming port
 *        calling bcm_switch_control_port_set(unit, port, bcmSwitchIgmpQueryFlood, val);
 *      - Configure trap: trap IGMP memberships type packets.
 *        bcmRxTrapIgmpMembershipQuery trap at the link layer block (for IPoEth packets)
 *        bcmRxTrapFrwrdIgmpMembershipQuery trap at the forwarding block, after tunnel termination. (for vxlan/vpls packets after tunnel termination)
 *        calling bcm_rx_trap_type_create to create the trap type calling bcm_rx_trap_set to create the trap
 *
 *     multicast:
 *      - Configure device to support multicast
 *          calling bcm_switch_control_set(unit,bcmSwitchL3McastL2,1);
 *      - simulate igmp to create multicast group and add port:
 *        - open a new multicast group
 *          calling bcm_multicast_create
 *        - add port and vlan to the multicast group:
 *          calling bcm_multicast_ingress_add
 *        - create the multicast group
 *          calling bcm_ipmc_add
 
 * Traffic: 
 * IGMP snooping
 * IPoETH: send packet with IGMP Membership query.
 * IpoETH over tunnel: send packet with IGMP Membership query.
 * packets are trapped at LL block (for IPoETH) and at Forwarding block (for IPoETH o tunnel)
   and sent as is to out_port
 * multicast
 * IPoETH: send multicast compatible packet. multicast group: 0.0.1.2 (28 LSB of 224.0.1.2)
 * IPoETHoTunnel: packets arrive from the core. Tunnel is terminated. then check that the
 * remaining packet is multicast compatible. The packet is duplicated to all members of the
 * group (3 packets are sent)
 *
 * How to run: IGMP snooping IGMPoIPoETH
 * BCM> cint ../../../../src/examples/dpp/cint_igmp_example.c
 * BCM> cint
 * int unit=0;
 * int in_port=200;
 * int out_port=201;
 * igmp_snooping_enable(unit,in_port,out_port,1);
 * exit;
 * BCM> tx 1 PSRC=200
 * DATA=0x01005E00010200020500000008004500001C000000004002D92AC0A8000BE00001021164EE9B00000000000102030405060708090A0B0C0D0E0F1011121314150000000000000000000000000000000001000002000000000000000000000000
 * expected results:
 * - packet is sent to port 201 (diag pp FDT)
 * - add a vlan header (outgoing packet = incoming packet + 4B)
 *
 *
 * How to run: IGMP multicast for IPoETH packet
 * BCM> cint ../../../../src/examples/dpp/cint_igmp_example.c
 * BCM> cint
 * int unit=0;
 * int in_port=200;
 * int out_port=201;
 * int out_port2=202;
 * int vsi=4000;
 * igmp_mc_example(unit,in_port,out_port,out_port2, vsi,igmp_example_ipoeth_my_mac_get());
 * bcm_port_untagged_vlan_set(unit,in_port,vsi);
 * exit;
 * tx 1 PSRC=200 DATA=0x01005E0001020002050000000800450000B60000000040FFD793C0A8000BE0000102000102030405060708090A0B0C0D0E0F101112131415161718191A1AAAAA0000000000000000000000000000000001000002000000000000000000000000
 * expected result:
 * - 3 copies are sent to port 200, 201,202. (diag counters g)
 *
 *
 * How to run: IGMP snooping for IGMPoIPoEthoVPLS 
 * BCM> cint ../../../../src/examples/dpp/cint_port_tpid.c                
 * BCM> cint ../../../../src/examples/dpp/cint_advanced_vlan_translation_mode.c
 * BCM> cint ../../../../src/examples/dpp/cint_qos.c                      
 * BCM> cint ../../../../src/examples/dpp/utility/cint_utils_l3.c 
 * BCM> cint ../../../../src/examples/dpp/cint_mpls_lsr.c                  
 * BCM> cint ../../../../src/examples/dpp/cint_vswitch_metro_mp.c         
 * BCM> cint ../../../../src/examples/dpp/cint_vswitch_vpls.c             
 * BCM> cint ../../../../src/examples/dpp/cint_igmp_example.c
 * BCM> cint
 *  int unit=0;
 *  int in_port=200;
 *  int out_port=201;
 *  int vpn=4000;
 *  int sec_unit=-1;
 *  int ext_example=0;
 * vswitch_vlps_info_init(unit,out_port,pwe_in_port,pwe_out_port,10,20,15,30,vpn);
 * vswitch_vpls_run(unit,sec_unit,ext_example);
 * igmp_snooping_enable(unit,in_port,out_port,1);
 * exit;
 * BCM>tx 1 PSRC=200
   DATA=0x000000000011000000000002810000648847003EA040007DA14001005E0001020000000000FF0800450000500000000040020000C0A80101E0000102116400000000000000000001000000000001020304000000000000000000000000000000
 * expected results:
 * - packet is sent to port 201 (diag pp FDT)
 * 
 *
 * How to run IGMP multicast for IPoETHoVPLS_tunnel 
 * BCM> cint ../../../../src/examples/dpp/cint_port_tpid.c                
 * BCM> cint ../../../../src/examples/dpp/cint_advanced_vlan_translation_mode.c
 * BCM> cint ../../../../src/examples/dpp/cint_qos.c                      
 * BCM> cint ../../../../src/examples/dpp/utility/cint_utils_l3.c 
 * BCM> cint ../../../../src/examples/dpp/cint_mpls_lsr.c                  
 * BCM> cint ../../../../src/examples/dpp/cint_vswitch_metro_mp.c         
 * BCM> cint ../../../../src/examples/dpp/cint_vswitch_vpls.c             
 * BCM> cint ../../../../src/examples/dpp/cint_igmp_example.c
 * BCM> cint
 *  int unit=0;
 *  int in_port=200;
 *  int out_port=201;
 *  int out_port2=202;
 *  int vpn=4000;
 *  int sec_unit=-1;
 *  int ext_example=0; 
 * vswitch_vlps_info_init(unit,out_port,pwe_in_port,pwe_out_port,10,20,15,30,vpn);
 * vswitch_vpls_run(unit,sec_unit,ext_example);
 * igmp_mc_example(unit,in_port,out_port,out_port2,vpn,vswitch_vpls_my_mac_get());
 * exit;
 * BCM>tx 1 PSRC=200
 *DATA=0x000000000011000000000002810000648847003EA040007DA14001005E0001020000000000FF08004500005000000000403D0000C0A80101E0000102001020300000000000000001000000000001020304000000000000000000000000000000
 * expected results:
 * - 3 packets are sent to port 200, 201, 202 (diag counters g)
 * - vpls tunnel is terminated: (diag pp TERMination_Info,
 * - outgoing packet = incoming packet - 22B)
 *   incoming packet: ETH(14) + VLAN(4) + MPLS(4) +PWE(4) + nativeETH(14)           + nativeIP(20) + PAYLOAD(36) = 96
 *   outcoming packet:                                      nativeETH(14) + VLAN(4) + nativeIP(20) + PAYLOAD(36) = 74
 *
 *
 * How to run IGMP snooping for IGMPoIPoEthoVXLAN_tunnel
 * soc properties:
 * bcm886xx_ip4_tunnel_termination_mode=0
 * bcm886xx_l2gre_enable=0
 * bcm886xx_vxlan_enable=1
 * bcm886xx_ether_ip_enable=0
 * bcm886xx_vxlan_tunnel_lookup_mode=2
 * bcm886xx_auxiliary_table_mode=1 (for arad/+ devices only)
 *
 * BCM> cint ../../../../src/examples/dpp/utility/cint_utils_global.c
 * BCM> cint ../../../../src/examples/dpp/utility/cint_utils_l3.c
 * BCM> cint ../../../../src/examples/dpp/cint_ip_route.c
 * BCM> cint ../../../../src/examples/dpp/cint_port_tpid.c     
 * BCM> cint ../../../../src/examples/dpp/cint_advanced_vlan_translation_mode.c      
 * BCM> cint ../../../../src/examples/dpp/cint_field_gre_learn_data_rebuild.c   
 * BCM> cint ../../../../src/examples/dpp/cint_ip_tunnel.c cint_ip_tunnel_term.c
 * BCM> cint ../../../../src/examples/dpp/cint_mact.c cint_vswitch_metro_mp.c   
 * BCM> cint ../../../../src/examples/dpp/cint_vxlan.c                          
 * BCM> cint_../../../../src/examples/dpp/igmp_example.c                  
 * BCM> cint
 * int unit=0;
 * int in_port=200;
 * int out_port=201;
 * int vpn=4000;
 * vxlan_example(unit,in_port,out_port,vpn);
 * igmp_snooping_enable(unit,in_port,out_port,1);
 * exit;
 * BCM>tx 1 PSRC=200
 *DATA=0x000c00020000000007000100810000140800450000560000000040112f85a0000001ab000011555555550042d257080000000013880001005e00010200020500000008004500001c0000000040020000c0a8000be0000102116400000000000000010203000000000000000000000000000008004500001c0000000040020000
 *
 * expected result
 * - packet is sent to port 201 (diag pp FDT)
 *
 *
 * How to run IGMP multicast for IPoEthpVXLAN_tunnel
 * soc properties:
 * bcm886xx_ip4_tunnel_termination_mode=0
 * bcm886xx_l2gre_enable=0
 * bcm886xx_vxlan_enable=1
 * bcm886xx_ether_ip_enable=0
 * bcm886xx_vxlan_tunnel_lookup_mode=2
 * bcm886xx_auxiliary_table_mode=1
 *
 * BCM> cint ../../../../src/examples/dpp/utility/cint_utils_global.c
 * BCM> cint ../../../../src/examples/dpp/utility/cint_utils_l3.c
 * BCM> cint ../../../../src/examples/dpp/cint_ip_route.c
 * BCM> cint ../../../../src/examples/dpp/cint_port_tpid.c     
 * BCM> cint ../../../../src/examples/dpp/cint_advanced_vlan_translation_mode.c      
 * BCM> cint ../../../../src/examples/dpp/cint_field_gre_learn_data_rebuild.c   
 * BCM> cint ../../../../src/examples/dpp/cint_ip_tunnel.c cint_ip_tunnel_term.c
 * BCM> cint ../../../../src/examples/dpp/cint_mact.c cint_vswitch_metro_mp.c   
 * BCM> cint ../../../../src/examples/dpp/cint_vxlan.c                          
 * BCM> cint ../../../../src/examples/dpp/cint_igmp_example.c                  
 * BCM> cint
 * set unit 0
 * int in_port=200;
 * int out_port=201;
 * int out_port2=202;
 * int vpn=4000;
 * vxlan_example(unit,in_port,out_port,vpn);
 * igmp_mc_example(unit,in_port,out_port,out_port2,vpn,vxlan_my_mac_get());
 * exit;
 * tx 1 PSRC=200
 *DATA=0x000c000200000000070001008100001408004500004a0000000040112f91a0000001ab000011555555550036e3a0080000000013880001005e00010200020500000008004500001400000000403d0000c0a8000be00001020001020300000000000008004500001400000000403d0000000008004500001400000000403d0000
 
 * expected result:
 * - 3 packets are sent to port 200, 201, 202 (diag counters g)
 * vxlan tunnel is terminated: (diag pp TERMination_Info.
 * outgoing packet = incoming packet - 50)
 *   incoming packet: ETH(14) + VLAN(4) + IP(20) + UDP(8) + VXLAN(8B) + nativeETH(14)           + nativeIP(20) + PAYLOAD(36) = 124
 *   outcoming packet:                                                  nativeETH(14) + VLAN(4) + nativeIP(20) + PAYLOAD(36) = 74

 
 * Default Settings: 
 * There are two examples, one for snooping and one multicast.
 * 
 * These examples configures the following: 
 *  igmp snooping
 *  igmp multicast
 * 
 * how to run:
cint examples/dpp/cint_igmp_example.c 
 */

int print_level = 1;


/********** 
  functions
 ********** */

bcm_mac_t igmp_example_ipoeth_my_mac_get() {
    bcm_mac_t my_mac = {0x00, 0x00, 0x00, 0x00, 0x00, 0x10};
    return my_mac;
}



/* 
 * Enables igmp snooping
 * Packets meeting the criteria that are going to port <port> will be duplicated and sent out of <out_port>
 * @port        : tx pkt destination
 * @out_port    : rx pkt source
 * @val         : enable igmp
 */
int igmp_snooping_enable(int unit, int port, int out_port, int val){
    int rv = 0;
    int flags = 0;
    rv = bcm_switch_control_port_set(unit, port, bcmSwitchIgmpQueryFlood, val);
    if (rv != BCM_E_NONE) {
        printf("Error, bcm_switch_control_port_set \n");
        return rv;
    }
    bcm_rx_trap_config_t conf;
    bcm_rx_trap_config_t_init(&conf);
    bcm_rx_trap_t traps[] = {bcmRxTrapIgmpMembershipQuery, bcmRxTrapFrwrdIgmpMembershipQuery};
    int trap_i;
    conf.flags = (BCM_RX_TRAP_UPDATE_DEST);
    conf.trap_strength = 7;
    conf.dest_port = out_port;
    int trap_id[10];
    int trap_id_cnt = 0;

    for (trap_i = 0; trap_i < sizeof(traps) / sizeof(traps[0]); ++trap_i) {
        trap_id[trap_i]=0xff;
        rv = bcm_rx_trap_type_create(unit, flags, traps[trap_i], &trap_id[trap_i]);
        if (rv != BCM_E_NONE) {
            printf("Error, in trap create, trap %d \n", trap_i);
            return rv;
        }

        rv = bcm_rx_trap_set(unit,trap_id[trap_i],&conf);
        if (rv != BCM_E_NONE) {
            printf("Error, in trap set \n");
            return rv;
        }
        trap_id_cnt++;
    }
    return BCM_E_NONE;
}

int igmp_traps_destroy(int unit){
    bcm_rx_trap_t traps[] = {bcmRxTrapIgmpMembershipQuery, bcmRxTrapFrwrdIgmpMembershipQuery};
    int trap_i;
    int rv = 0;
    int trap_id[10];

    for (trap_i = 0; trap_i < sizeof(traps) / sizeof(traps[0]); ++trap_i) {
        rv = bcm_rx_trap_type_get(unit, 0, traps[trap_i], &trap_id[trap_i]);
        if (rv == BCM_E_NONE) {
            rv = bcm_rx_trap_type_destroy(unit, trap_id[trap_i]);
            if (rv != BCM_E_NONE) {
                printf("Error, in trap destroy, trap %d \n", trap_id[trap_i]);
                return rv;
            }
        }
    }
    return BCM_E_NONE;
}

/* 
 * An example of creating IP compatible-multicast groups
 * Enables igmp multicast
 * Packets going to <inP> will be replicated and sent through <inP>, <ouP> and <ouP2>
 * added to multicast group:
 * <inP,vlan>, <ouP,vlan> and <ouP2,vlan>
 * unit: 
 * @inP        : tx pkt destination, rx pkt source 
 * @ouP        : rx pkt source
 * @ouP2       : rx pkt source
 * @my_mac     : my mac 
 * 
 */
int igmp_mc_example(int unit, int inP, int ouP, int ouP2, int vlan, bcm_mac_t my_mac){
    int rv = BCM_E_NONE;
    rv = bcm_switch_control_set(unit,bcmSwitchL3McastL2,1);
    if (rv != BCM_E_NONE) {
        printf("Error, bcm_switch_control_set \n");
        return rv;
    }
    bcm_ipmc_addr_t data;

    /* Set parameters: User can set different parameters. */
    bcm_ip_t mc_ip = 0xE0000102; /* 224.0.1.2 */
    bcm_ip_t src_ip = 0xC0800101; /* 192.128.1.1 */
    
    int ipmc_index = 5555;
    int dest_local_port_id   = ouP;
    int dest_local_port_id_2 = ouP2;
    int source_local_port_id = inP;
    int nof_dest_ports = 2, port_ndx;
    bcm_l3_intf_t intf, intf_ori;
    bcm_l3_ingress_t ingress_intf;

    /* Create L3 Intf and disable its IPMC */
    bcm_l3_intf_t_init(&intf);
    sal_memcpy(intf.l3a_mac_addr, my_mac, 6);
    intf.l3a_vid = vlan;    

    /* before creating L3 INTF, check whether L3 INTF already exists*/
    bcm_l3_intf_t_init(&intf_ori);
    intf_ori.l3a_intf_id = vlan;
    rv = bcm_l3_intf_get(unit, &intf_ori);
    if (rv == BCM_E_NONE) {
        /* if L3 INTF already exists, replace it*/
        intf.l3a_flags = BCM_L3_REPLACE | BCM_L3_WITH_ID;
        intf.l3a_intf_id = vlan;
    }
    
    rv = bcm_l3_intf_create(unit, &intf);
    if (rv != BCM_E_NONE) {
        printf("Error, bcm_l3_intf_create \n");
        return rv;
    }
    bcm_l3_ingress_t_init(&ingress_intf);
    ingress_intf.flags = BCM_L3_INGRESS_ROUTE_DISABLE_IP4_MCAST | BCM_L3_INGRESS_WITH_ID | BCM_L3_INGRESS_ROUTE_DISABLE_IP6_MCAST;
    rv = bcm_l3_ingress_create(unit, &ingress_intf, &intf.l3a_intf_id);
    if (rv != BCM_E_NONE) {
        printf("Error, bcm_l3_ingress_create\n");
        return rv;
    }

    rv = bcm_multicast_destroy(unit,ipmc_index);
    if (rv != BCM_E_NOT_FOUND && rv != BCM_E_NONE) {
        printf("Error, bcm_multicast_destroy \n");
        return rv;
    }

    /* Create the IP MC Group on given vlan */

    rv = bcm_multicast_create(unit, BCM_MULTICAST_INGRESS_GROUP | BCM_MULTICAST_TYPE_L3 | BCM_MULTICAST_WITH_ID, &ipmc_index);
    if (rv != BCM_E_NONE) {
        printf("Error, bcm_multicast_create \n");
        return rv;
    }

    rv = bcm_multicast_ingress_add(unit,ipmc_index,dest_local_port_id   ,vlan);
    if (rv != BCM_E_NONE) {
        printf("Error, bcm_multicast_ingress_add: port %d vlan: %d \n", dest_local_port_id   ,vlan);
        return rv;
    }

    rv = bcm_multicast_ingress_add(unit,ipmc_index,dest_local_port_id_2 ,vlan);
    if (rv != BCM_E_NONE) {
        printf("Error, bcm_multicast_ingress_add: port %d vlan: %d \n", dest_local_port_id_2   ,vlan);
        return rv;
    }

    rv =  bcm_multicast_ingress_add(unit,ipmc_index,source_local_port_id ,vlan);
    if (rv != BCM_E_NONE) {
        printf("Error, bcm_multicast_ingress_add: port %d vlan: %d \n", source_local_port_id   ,vlan);
        return rv;
    }

    /* Add ipmc group entry to LEM */

    bcm_ipmc_addr_t_init(&data);
    data.mc_ip_addr = mc_ip;
    data.s_ip_addr = 0x0;
    data.vid = vlan;
    data.flags = BCM_IPMC_L2;
    data.group = ipmc_index;
    /* Creates the entry */
    rv = bcm_ipmc_add(unit,&data);
    if (rv != BCM_E_NONE) {
        printf("Error, bcm_multicast_ingress_add \n");
        return rv;
    }

    return rv;
}
 
 
