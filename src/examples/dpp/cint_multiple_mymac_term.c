/*
 * $Id: cint_l3_vrrp.c,v 1.9 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$ 
 *  
 *  
 * Multiple mymac mode allows l2 terminating all l3 protocol packets, according to the mac address.
 * Note that to use this functionality you have to disable vrrp apis in your bcm.user (bcm_l3_vrrp_*). 
 * You can work around this by adding l2 stations with the vrrp mask (00:00:5e:00:01:xx).
 * To use this feature, use soc properties 
 *      l3_vrrp_max_vid=0 
 *      l3_multiple_mymac_termination_enable=1
 * To have different terminating mac addresses for IPv4 and other l3 protocols, use soc property.
 *      l3_multiple_mymac_termination_mode=1
 * 
 * Then, use bcm_l2_station_add, as shown in multiple_mymac_termination_example. 
 *  
 * To run sample script: 
 * cd ../../../src/examples/dpp
 * cint cint_ip_route.c
 * cint cint_l3_vrrp.c 
 * cint cint_multiple_mymac_term.c 
 * cint
 * multiple_mymac_termination_example(<unit>, <in_port>, <out_port>, <ipv4_distinct>);
 */


 /*
 *  IPV4 packet (or any l3 protocol if no ipv4_distinction) to send from in_port:  
 *  - vlan = in_vlan (= 50).
 *  - DA = {0x00, 0x00, 0x6b, 0x6b, 0x01, 0x75}
 *  - IPV4 DIP = 0x7fffff00 (127.255.255.0)
 *  packet will be routed to out_port
 *  there will be termination (although DA is not MyMac)
 *  
 * Any l3 protocol packet to send from in_port:  
 *  - vlan = in_vlan (= 50).
 *  - DA =  {0x00, 0x00, 0x6b, 0x6b, 0x08, 0x75}
 *  
 *  Packets will be routed according to the mapping in create_traffic_mapping
 *  
 */
int multiple_mymac_termination_example(int unit, int in_port, int out_port, int ipv4_distinct){
    int rv;
    int station_id;
    bcm_l2_station_t station;
    bcm_mac_t mutliple_mac = {0x00, 0x00, 0x6b, 0x6b, 0x01, 0x75};
    bcm_mac_t mac_mask = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
    int in_vlan = 50;

    bcm_l2_station_t_init(&station);
    sal_memcpy(station.dst_mac, mutliple_mac, 6);
    sal_memcpy(station.dst_mac_mask, mac_mask, 6);
    station.flags = ipv4_distinct ? BCM_L2_STATION_IPV4 : 0;
    station.vlan = in_vlan;
    station.vlan_mask = 0xffff;
    rv = bcm_l2_station_add(unit, &station_id, &station);
    if (rv != BCM_E_NONE) {
        printf("Error, in bcm_l2_station_add\n");
        return rv;
    }

    if (verbose) {
        printf("Done creating first l2_station. station_id=0x%x\n", station_id);
        print station;
    }

    station.dst_mac[4] = 0x08;
    station.flags = 0;
    rv = bcm_l2_station_add(unit, &station_id, &station);
    if (rv != BCM_E_NONE) {
        printf("Error, in bcm_l2_station_add\n");
        return rv;
    }

    if (verbose) {
        printf("Done creating second l2_station. station_id=0x%x\n", station_id);
        print station;
    }

    rv = create_traffic_mapping(unit, in_port, out_port, in_vlan, 0);
    if (rv != BCM_E_NONE) {
        printf("Error, in create_traffic_mapping");
        return rv;
    }

    return rv;
}

