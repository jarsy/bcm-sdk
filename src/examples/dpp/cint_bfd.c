/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~BFD test~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/*
*
* $Id: cint_bfd.c,v 1.15 Broadcom SDK $
* $Copyright: (c) 2016 Broadcom.
* Broadcom Proprietary and Confidential. All rights reserved.$
* 
* File: cint_bfd.c
* Purpose: Example of using BFD APIs.
*
* Usage:
* 
* 
* 
* To run IPV4 test:
* BCM> cint examples/dpp/utility/cint_utils_mpls.c
* BCM> cint examples/dpp/utility/cint_utils_l3.c
* BCM> cint examples/dpp/utility/cint_utils_vlan.c
* BCM> cint examples/dpp/utility/cint_utils_global.c
* BCM> cint examples/dpp/utility/cint_utils_oam.c
* BCM> cint examples/dpp/cint_ip_route.c
* BCM> cint examples/dpp/cint_bfd.c
* If you run on ARAD+ you also require BCM> cint examples/dpp/cint_field_bfd_ipv4_single_hop.c
* BCM> cint
* bfd_ipv4_run_with_defaults(0,13,14,-1,0,BCM_BFD_ENDPOINT_IN_HW | BCM_BFD_ENDPOINT_MULTIHOP);
*
* To run MPLS test:
* BCM> cint examples/dpp/utility/cint_utils_global.c
* BCM> cint examples/dpp/utility/cint_utils_mpls.c
* BCM> cint examples/dpp/utility/cint_utils_l3.c
* BCM> cint examples/dpp/utility/cint_utils_vlan.c
* BCM> cint examples/dpp/utility/cint_utils_oam.c
* BCM> cint examples/dpp/cint_ip_route.c
* BCM> cint examples/dpp/cint_bfd.c
* BCM> cint
* bfd_mpls_run_with_defaults(0,13,14);
* exit;
*
* To run PWE tests:
* BCM> cint examples/dpp/utility/cint_utils_global.c
* BCM> cint examples/dpp/utility/cint_utils_l3.c
* BCM> cint examples/dpp/utility/cint_utils_l2.c
* BCM> cint examples/dpp/utility/cint_utils_multicast.c
* BCM> cint examples/dpp/utility/cint_utils_vlan.c
* BCM> cint examples/dpp/utility/cint_utils_mpls.c
* BCM> cint examples/dpp/utility/cint_utils_mpls_port.c
* BCM> cint examples/dpp/utility/cint_utils_oam.c
* BCM> cint examples/dpp/cint_port_tpid.c
* BCM> cint examples/dpp/cint_qos.c
* BCM> cint examples/dpp/cint_vswitch_vpls.c
* BCM> cint examples/dpp/cint_bfd.c
* BCM> cint examples/dpp/cint_mpls_lsr.c
* BCM> cint examples/dpp/cint_advanced_vlan_translation_mode.c
* cint
* int type = 1; or int type = 2; or int type = 4;
* #Type 1: BFD over CW (ACH)  over PWE
* #Type 2: PWE  over router alert
* #Type 4: BFD over G-ACH over GAL over PWE.
* bfd_pwe_run_with_defaults(0,13,14,type);
* 
* comments:
* 1) In order to prevent from OAMP send packets do: BCM.0> m OAMP_MODE_REGISTER TRANSMIT_ENABLE=0
* 2) gport, egress_if, tx_dest_system_port in different types:
* IP:
* bfd_endpoint_info.gport = 0; in lif - no need for this field in IPv4
* bfd_endpoint_info.egress_if = 0x40001002; next hop mac at encap-id... Taken from create_l3_egress(unit,flags,out_port,out_vlan,ing_intf_out,next_hop_mac, &fec, &encap_id)
* bfd_endpoint_info.tx_dest_system_port = 0x400000d; out port
* MPLS:
* bfd_endpoint_info.gport = 0; in lif: tunnel_switch.tunnel_id
* bfd_endpoint_info.egress_if =  0x40001000; out lif: tunnel_id in example_ip_to_tunnel
* bfd_endpoint_info.tx_dest_system_port = 0x400000d; out port
* PWE:
* bfd_endpoint_info.gport = 0x4c004000; in lif: tunnel_switch.tunnel_id
* bfd_endpoint_info.egress_if =  0x40001000; out lif: mpls_port->encap_id
* bfd_endpoint_info.tx_dest_system_port = 0x400000d; out port
* 
* 
* This cint also test bfd echo. In order to enable BFD echo set "bfd_echo"
* BCM> cint examples/dpp/utility/cint_utils_mpls.c
* BCM> cint examples/dpp/utility/cint_utils_l3.c
* BCM> cint examples/dpp/utility/cint_utils_vlan.c
* BCM> cint examples/dpp/utility/cint_utils_global.c
* BCM> cint examples/dpp/utility/cint_utils_oam.c
* BCM> cint examples/dpp/cint_ip_route.c
* BCM> cint examples/dpp/cint_bfd.c
* BCM> cint examples/dpp/cint_field_bfd_echo.c
* If you run on ARAD+ you also require BCM> cint examples/dpp/cint_field_bfd_ipv4_single_hop.c
* BCM> cint
* bfd_echo_example(0,13,14,0,-1,0);
* 
* To run BFD IPv4 single hop test:
* cd ../../../../regress/bcm
* BCM> cint examples/dpp/utility/cint_utils_global.c
* BCM> cint examples/dpp/utility/cint_utils_mpls.c
* BCM> cint examples/dpp/utility/cint_utils_l3.c
* BCM> cint examples/dpp/utility/cint_utils_vlan.c
* BCM> cint examples/dpp/utility/cint_utils_oam.c
* BCM> cint examples/dpp/cint_ip_route.c
* BCM> cint examples/dpp/cint_bfd.c
* BCM> cint examples/dpp/cint_field_bfd_ipv4_single_hop.c
* cint
* bfd_ipv4_run_with_defaults(0,13,14,-1,0,BCM_BFD_ENDPOINT_IN_HW);

*/

/* set in order to do rmep_id encoding using utility functions and not using APIs. */
int encoding = 0;
bcm_bfd_endpoint_t remote_id_system = 0;
bcm_bfd_endpoint_t local_id_system = 0;
int is_server = 0;
int bfd_echo_grp_pri = 9;

/* Globals - MAC addresses , ports & gports*/
int port_1 = 13; /* physical port (signal generator)*/
int port_2 = 14;
int mpls_label = 100;
int next_hop_mac;
int tunnel_id;
int mpls_label_termination = 200;

int timeout_events_count_bfd = 0;
int timeout_events_count_multi_bfd = 0;

int last_trap_code_created = 0;
int remote_gport_trap_code = 0;

/* GAL is a reserved label with value 13 */
int gal_label = 13;

int pwe_ach=1; /* May be used by PWE router alert types in Jericho, set to 1 by default*/

/*enable bfd echo, 1- implementation without lem, 2 - implementation with lem*/
int bfd_echo=0;
/*bfd echo field group*/
bcm_field_group_t echo_group;
/* bfd echo qualifier*/
int echo_qual_id;

/* Default bfd tx period */
int default_bfd_period = 100;

/*enable bfd ipv4 single hop*/
int single_hop_extended =0;

/* When enabled the IPv4 SIP, generated by the OAMP, is
* configured by the user and replaced by the PRGE. 
* Soc property bfd_extended_ipv4_src_ip must be enabled as well. */
int bfd_extended_sip = 0;

/*Trap code for snooping accelerated packets*/
int trap_code_4_snooping = 0;
int use_trap_with_snoop = 0;

/* Id of the created the created BFD EP*/
int bfd_id = 0;

/* Created endpoints information */
oam__ep_info_s bfd_ep1;
oam__ep_info_s bfd_ep2;
oam__ep_info_s bfd_ep3;

int bfd_create_trap_from_gport(int unit, bcm_gport_t gport, int* trap_code) {
    bcm_error_t rv;
    bcm_rx_trap_config_t trap_config;

    rv = bcm_rx_trap_type_create(unit, 0, bcmRxTrapUserDefine, trap_code);
    if (rv != BCM_E_NONE) {
       printf("(%s) \n",bcm_errmsg(rv));
       return rv;
    }

    bcm_rx_trap_config_t_init(&trap_config);
    trap_config.flags = BCM_RX_TRAP_UPDATE_DEST | BCM_RX_TRAP_TRAP;
    trap_config.dest_port = gport;

    rv = bcm_rx_trap_set(unit, *trap_code, &trap_config);
    if (rv != BCM_E_NONE) {
        printf("(%s) \n", bcm_errmsg(rv));
        return rv;
    }

    last_trap_code_created = *trap_code;
    printf("Trap created trap_code=%d \n", *trap_code);

    return rv;
}

int system_endpoint_to_endpoint_id_bfd(bcm_bfd_endpoint_info_t *mep_info) {
    if (!(mep_info->opcode_flags & BCM_BFD_ENDPOINT_IN_HW)) {
        printf("Can not create non-accelerated endpoint with id\n");
        return BCM_E_FAIL;
    }

    if (mep_info->flags & BCM_BFD_ENDPOINT_REMOTE_WITH_ID) {
        mep_info->remote_id |= (1 << 25 /*_BCM_OAM_REMOTE_MEP_INDEX_BIT*/);
    }
    return BCM_E_NONE;
}

int read_timeout_event_count_bfd(int expected_event_count) { 
    printf("timeout_events_count_bfd=%d\n",timeout_events_count_bfd);

    if (timeout_events_count_bfd==expected_event_count) {
        return BCM_E_NONE;
    }
    else {
        return BCM_E_FAIL;
    }
}

int read_timeout_event_count_multi_bfd(int expected_event_count) {
    printf("timeout_events_count=%d\n",timeout_events_count_multi_bfd);

    if (timeout_events_count_multi_bfd==expected_event_count) {
        return BCM_E_NONE;
    }
    else {
        return BCM_E_FAIL;
    }
}

/* Add term entry to perform pop
*/
int
mpls_add_term_entry_ex(int unit, int term_label, uint32 next_prtcl, bcm_mpls_tunnel_switch_t *entry)
{
    int rv;

    bcm_mpls_tunnel_switch_t_init(entry);
    entry->action = BCM_MPLS_SWITCH_ACTION_POP;

    /* 
     * Uniform: inherit TTL and EXP, 
     * in general valid options: 
     * both present (uniform) or none of them (Pipe)
     */
    entry->flags |= BCM_MPLS_SWITCH_OUTER_TTL|BCM_MPLS_SWITCH_OUTER_EXP;
    /* 
     * Next protocol indication: 
     * BCM_MPLS_SWITCH_NEXT_HEADER_IPV4 | BCM_MPLS_SWITCH_NEXT_HEADER_IPV6, or      
     * BCM_MPLS_SWITCH_NEXT_HEADER_L2, or 
     * 0 - unset(MPLS) 
     */

    /** presumably device_type has been resolved at this stage */
    if (device_type<device_type_arad_plus) {
        entry->flags |= next_prtcl;
    }

    /* incomming label */
    if (soc_property_get(unit , "mpls_termination_label_index_enable",0)) {
        BCM_MPLS_INDEXED_LABEL_SET(&entry->label, term_label, 1);
    }
    else {
        entry->label = term_label;
    }

    entry->qos_map_id = 0; /* qos not rellevant for BFD */

    rv = bcm_mpls_tunnel_switch_create(unit,entry);
    if (rv != BCM_E_NONE) {
        printf("Error, in bcm_mpls_tunnel_switch_create\n");
        return rv;
    }
    return rv;
}

/* creating l3 interface */
int l3_interface_init(int unit, int in_sysport, int out_sysport, int * _next_hop_mac_encap_id, int * _tunnel_id, int create_mpls_tunnel){
    int rv;
    int ing_intf_in;
    int ing_intf_out;
    int fec[2] = {0x0,0x0};
    int in_vlan = 1; 
    int out_vlan = 100;
    int vrf = 0;
    int host;
    int encap_id[2]={0};
    int route;
    int mask; 
    int l3_eg_int;
    int trap_id;
    bcm_mac_t mac_address  = {0x00, 0x0c, 0x00, 0x02, 0x00, 0x00};  /* my-MAC */
    bcm_mac_t next_hop_mac  = {0x00, 0x00, 0x00, 0x00, 0xcd, 0x1d}; /* next_hop_mac1 */
    bcm_mac_t next_hop_mac2  = {0x00, 0x00, 0x00, 0x00, 0xcd, 0x1d}; /* next_hop_mac2 */

    mpls__egress_tunnel_utils_s mpls_tunnel_properties;
    if (device_type==device_type_jericho) {
            encap_id[0] = 0x1000;
            encap_id[1] = 0x4000;
    }
    /*** create ingress router interface ***/
    rv = vlan__open_vlan_per_mc(unit, in_vlan, 0x1);  
    if (rv != BCM_E_NONE) {
        printf("Error, open_vlan=%d, in unit %d \n", in_vlan, unit);
    }
    rv = bcm_vlan_gport_add(unit, in_vlan, in_sysport, 0);
    if (rv != BCM_E_NONE && rv != BCM_E_EXISTS) {
        printf("fail add port(0x%08x) to vlan(%d)\n", in_sysport, in_vlan);
      return rv;
    }

    create_l3_intf_s intf;
    intf.vsi = in_vlan;
    intf.my_global_mac = mac_address;
    intf.my_lsb_mac = mac_address;
    intf.vrf_valid = 1;
    intf.vrf = vrf;
    intf.mtu_valid = 1;
    intf.mtu = 0;
    intf.mtu_forwarding = 0;

    rv = l3__intf_rif__create(unit, &intf);
    ing_intf_in = intf.rif;
    if (rv != BCM_E_NONE) {
        printf("Error, l3__intf_rif__create\n");
    }

    /*** create egress router interface ***/
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

    /*** create egress object 1 ***/
    if (create_mpls_tunnel) {
        /*** Create tunnel ***/
        *_tunnel_id = 0;

        mpls_tunnel_properties.label_in = mpls_label_termination;
        mpls_tunnel_properties.label_out = 0;
        mpls_tunnel_properties.next_pointer_intf = ing_intf_out;

        printf("Trying to create tunnel initiator\n");
        rv = mpls__create_tunnel_initiator__set(unit, &mpls_tunnel_properties);
        if (rv != BCM_E_NONE) {
           printf("Error, in mpls__create_tunnel_initiator__set\n");
           return rv;
        }

        *_tunnel_id = mpls_tunnel_properties.tunnel_id;

        printf("tunnel_id (egress_interface):0x%08x \n", *_tunnel_id);

        l3_eg_int = *_tunnel_id;
    }
    else {
        l3_eg_int = ing_intf_out;
    }

    /*** Create egress object1 ***/
    create_l3_egress_s l3eg;
    l3eg.out_tunnel_or_rif = l3_eg_int;
    sal_memcpy(l3eg.next_hop_mac_addr, next_hop_mac, 6);
    l3eg.vlan   = out_vlan;
    l3eg.arp_encap_id = encap_id[0];
    l3eg.fec_id = fec[0]; 
    l3eg.allocation_flags = 0;
    l3eg.out_gport = out_sysport;

    rv = l3__egress__create(unit,&l3eg);
    if (rv != BCM_E_NONE) {
        printf("Error, in l3__egress__create  \n");
        print rv;
    }
    encap_id[0] = l3eg.arp_encap_id;
    fec[0] = l3eg.fec_id;

    if(verbose >= 1) {
        printf("created FEC-id =0x%08x, in unit %d \n", fec[0], unit);
        printf("next hop mac at encap-id %08x, in unit %d\n", encap_id[0], unit);
    }

    if (!create_mpls_tunnel) {
        *_tunnel_id = encap_id[0];
    }

    /*** add host ***/

    if (single_hop_extended) {
        rv = add_host(unit, 0x12345678, vrf, fec[0]); 
    } else {
        rv = add_host(unit, 0x7fffff03, vrf, fec[0]); 
    } 
    if (rv != BCM_E_NONE) {
        printf("Error, create egress object, in_sysport=%d, in unit %d \n", in_sysport, unit);
    }




    /*** create egress object 2***/
    /* We're allocating a lif. out_sysport unit should be first, and it's already first */
    create_l3_egress_s l3eg1;
    l3eg1.out_tunnel_or_rif = l3_eg_int;
    sal_memcpy(l3eg1.next_hop_mac_addr, next_hop_mac2, 6);
    l3eg1.vlan   = out_vlan;
    l3eg1.arp_encap_id = encap_id[1];
    l3eg1.fec_id = fec[1]; 
    l3eg1.allocation_flags = 0;
    l3eg1.out_gport = out_sysport;

    rv = l3__egress__create(unit,&l3eg1);
    if (rv != BCM_E_NONE) {
        printf("Error, in bcm_vlan_create with vlan %d continue \n", l3eg.vlan);
        print rv;
    }
    encap_id[1] = l3eg1.arp_encap_id;
    fec[1] = l3eg1.fec_id;    
    *_next_hop_mac_encap_id = encap_id[1];

    if(verbose >= 1) {
        printf("created FEC-id =0x%08x, in unit %d\n", fec[1], unit);
        printf("next hop mac at encap-id %08x, in unit %d\n", encap_id[1], unit);
    }

    /*** add route point to FEC2 ***/
    if (!bfd_echo) {
        if (single_hop_extended) {
              route = 0x12345600;
        }
        else{
            route = 0x7fffff00;
        }
        mask  = 0xfffffff0;
        rv = add_route(unit, route, mask , vrf, fec[1]); 
        if (rv != BCM_E_NONE) {
            printf("Error, create egress object, in_sysport=%d in unit %d, \n", in_sysport, unit);
        } 
    }
    return rv;
}



int bfd_ipv4_example_init(int unit, int p1, int p2) {
    int rv;

    port_1 = p1;
    port_2 = p2;

    rv = l3_interface_init(unit, port_1, port_2, &next_hop_mac, &tunnel_id, 0);
    if (rv != BCM_E_NONE) {
      printf("(%s) \n",bcm_errmsg(rv));
      return rv;
    }

    return rv;
}


/* In BFD ACHO the PMF replace the classifier.
If the packet is identified as BFD echo (TCAM match- udp_src_port, dest_ip, local descrimiantor), 
The action is to set the trap code to bcmRxTrapBfdEchoOverIpv4 and Trap-Qualifier=BFD.Your-Discriminator[15:0].
The Trap-Qualifier is the MEP-ID used by the OAMP to access the MEP-DB */

int bfd_echo_field_classifier(int unit) {
    bcm_field_presel_set_t presel_set;  
    int presel_id;

    bcm_field_group_config_t grp;
    int group_id = 0; 

    bcm_field_aset_t aset;

    bcm_field_entry_t action_entry;

    bcm_field_entry_t ent;
    uint32 out_port=40;
    int sys_gport;
    bcm_field_stage_t stage;
    bcm_field_data_qualifier_t your_descriminator;
    bcm_field_data_qualifier_t trap_code;
    bcm_field_data_qualifier_t strength;
    int result=0;



    bcm_gport_t trap_gport;
    int rc;
      
    int trap_id;

     
    rc = bcm_rx_trap_type_get(unit,0, bcmRxTrapBfdEchoOverIpv4 ,&trap_id);
    if (rc != BCM_E_NONE) 
    {
    printf ("bcm_rx_trap_type_get failed: %x \n", rc);
    }
    BCM_GPORT_TRAP_SET(trap_gport, trap_id, 7,0); 


    /*create user define quelifier*/
    bcm_field_data_qualifier_t_init(&your_descriminator); /* BFD.Your-Discriminator[15:0] */
    your_descriminator.flags = BCM_FIELD_DATA_QUALIFIER_LENGTH_BIT_RES;
    your_descriminator.offset_base = bcmFieldDataOffsetBaseThirdHeader;
    your_descriminator.offset = 10; /* points to the lsb of you.descriminator */
    your_descriminator.length = 2 * 8; /* bits */
    result = bcm_field_data_qualifier_create(unit, &your_descriminator);
    if (BCM_E_NONE != result) {
    print bcm_field_show(unit,"");
    return result;
    }


    /* Initialized Field Group */
    bcm_field_group_config_t_init(&grp);

    /* 
     * Define Programabble Field Group
     */ 

    grp.priority = bfd_echo_grp_pri;


    /*add qualifiers*/
    BCM_FIELD_QSET_INIT(grp.qset);

    result = bcm_field_qset_data_qualifier_add(unit,
                                             grp.qset,
                                             your_descriminator.qual_id);
    if (BCM_E_NONE != result) {
        printf("Error in bcm_field_qset_data_qualifier_add Err %x\n",result);
        return result;
    }

    BCM_FIELD_QSET_ADD(grp.qset, bcmFieldQualifyDstIp); 
    BCM_FIELD_QSET_ADD(grp.qset, bcmFieldQualifyL4DstPort);



    /* create field group */
    result = bcm_field_group_config_create(unit, &grp);
    if (BCM_E_NONE != result) {
        printf("Error in bcm_field_group_config_create with Err %x\n",result);
        return result;
    }


    BCM_FIELD_ASET_INIT(aset);
    BCM_FIELD_ASET_ADD(aset, bcmFieldActionTrap);

    /* Attached the action to the field group */
    result = bcm_field_group_action_set(unit, grp.group, aset);
    if (BCM_E_NONE != result) {
        printf("Error in bcm_field_group_action_set Err %x\n",result);
        return result;
    }

    result = bcm_field_group_install(unit, grp.group);
    if (BCM_E_NONE != result) {
        printf("Error in bcm_field_group_install\n");
        return result;
    }

    echo_group=grp.group;
    echo_qual_id=your_descriminator.qual_id;

    return result;

}

int bfd_echo_field_classifier_entry_add(int unit, bcm_field_group_t group, int qual_id,uint32 local_discr){

    bcm_field_group_config_t grp;
    int group_id = 0; 
    bcm_field_entry_t action_entry;
    bcm_field_entry_t ent;
    int sys_gport;
    bcm_error_t result=0;
    bcm_gport_t trap_gport;
    int trap_id;

    result = bcm_rx_trap_type_get(unit,0, bcmRxTrapBfdEchoOverIpv4 ,&trap_id);
    if (BCM_E_NONE != result) {
        printf ("bcm_rx_trap_type_get failed:,(%s) \n", bcm_errmsg(result));
        return result;
    }

    BCM_GPORT_TRAP_SET(trap_gport, trap_id, 7,0);

    /*create an entry*/
    result = bcm_field_entry_create(unit, group, &action_entry);
    if (BCM_E_NONE != result) {
        printf("Error in bcm_field_entry_create Err %x\n",bcm_errmsg(result));
        return result;
    }

    bcm_l4_port_t data_port=0x0ec9 ;
    bcm_l4_port_t mask_port = 0xffff;

    result = bcm_field_qualify_L4DstPort(unit, action_entry, data_port, mask_port);
    if (BCM_E_NONE != result) {
        printf("Error in bcm_field_qualify_L4SrcPort Err, (%s) \n",bcm_errmsg(result));
        return result;
        }

    bcm_ip_t data_ip=0x7fffff03;
    bcm_ip_t mask_ip=0xffffffff;
    result = bcm_field_qualify_DstIp(unit, action_entry, data_ip, mask_ip);

    if (BCM_E_NONE != result) {
        printf("Error in bcm_field_entry_create Err, (%s) \n",bcm_errmsg(result));
        return result;
    }

    uint16 tmp_local_discr=local_discr;
    uint8 data[2];

    data[1]=local_discr&0xff;
    local_discr=local_discr>>8;
    data[0]=local_discr&0xff;

    uint8 mask[2]= {0xff,0xff};
    result = bcm_field_qualify_data(unit, action_entry, qual_id, data, mask, 2);

    if (BCM_E_NONE != result) {
        printf("Error in bcm_field_qualify_data (%s) \n",bcm_errmsg(result));
        return result;
    } 

    result = bcm_field_action_add(unit, action_entry, bcmFieldActionTrap, trap_gport, tmp_local_discr);

    if (BCM_E_NONE != result) {
        auxRes = bcm_field_entry_destroy(unit,ent);
        auxRes = bcm_field_group_destroy(unit, grp);
        return result;
    }

    result = bcm_field_entry_install(unit, action_entry);

    if (BCM_E_NONE != result) {
        printf("Error in bcm_field_group_install (%s) \n",bcm_errmsg(result));
        return result;
    }

    return result;
}

int bfd_echo_example(int unit, int port1, int port2, int is_acc, int remote_port, int is_with_lem) {
    bcm_error_t rv;
    port_1 = port1;
    port_2 = port2;
    bcm_gport_t remote_gport;
    int trap_code;

    /*enable BFD echo*/
    if (is_with_lem) {
        bfd_echo = 2; 
    } else {
        bfd_echo = 1;
    }

    if (remote_port == -1) {
      remote_gport = BCM_GPORT_INVALID;
    } else {
        if (is_server) {
            BCM_GPORT_TRAP_SET(remote_gport, remote_port, 0, 0);
        } else {
            BCM_GPORT_LOCAL_SET(remote_gport, remote_port);

            /* remote_gport field must be a trap */
            rv = bfd_create_trap_from_gport(unit, remote_gport, &trap_code);
            if (rv != BCM_E_NONE) {
                printf("Error bfd_create_trap_from_gport.\n");
                return rv;
            }
            BCM_GPORT_TRAP_SET(remote_gport, trap_code, 7, 0);
        }
    }

    if (bfd_echo == 1) {
       rv = bfd_echo_field_classifier(unit);
       if (rv != BCM_E_NONE) {
         printf("Error bfd_echo_field_trap_qualifier.\n");
         return rv;
       }

       rv = bfd_echo_field_classifier_entry_add(unit, echo_group, echo_qual_id,0x4);
       if (BCM_E_NONE != rv) {
           printf("Error in bfd_echo_field_classifier_entry_add Err %x\n",rv);
           return rv;
       } 
    } else {
        rv = bfd_echo_with_lem_preselector(unit);
        if (rv != BCM_E_NONE) {
          printf("Error bfd_echo_with_lem_preselector.\n");
          return rv;
        } 
    }

    rv = bfd_ipv4_example(unit, remote_gport, BCM_BFD_ENDPOINT_IN_HW | BCM_BFD_ECHO | BCM_BFD_ENDPOINT_MULTIHOP);
    if (rv != BCM_E_NONE) {
      printf("Error bfd_ipv4_example.\n");
      return rv;
    }

    rv = register_events(unit);
    return rv;
}



/**
* Create BFD over IPV4 endpoint. Following variations 
* available: 
* 1) Accelerated/non accelerated 
* 2) Trapping incoming BFD frames to a configurable destination 
* 3) IPv4 one hop/multi hop 
* 
* @author sinai (23/09/2014)
* 
* @param unit 
* @param remote_gport 
* @param flags: supported options: 
*        BCM_BFD_ENDPOINT_MULTIHOP: multi hop/single hop
*        indication
*        BCM_BFD_ENDPOINT_IN_HW: accelerated in OAMP 
*        indication
*        BCM_BFD_ENDPOINT_MICRO_BFD: Jericho only 
* @return int 
*/ 
int bfd_ipv4_example(int unit, bcm_gport_t remote_gport, uint32 flags) {
    int rv;
    bcm_bfd_endpoint_info_t bfd_endpoint_info = {0};
    bcm_bfd_endpoint_info_t bfd_endpoint_test_info = {0};
    int ret;
    bcm_if_t next_if;
    int is_multi_hop = (flags &BCM_BFD_ENDPOINT_MULTIHOP) !=0;
    int trap_code;

    rv= get_device_type(unit,&device_type);
    BCM_IF_ERROR_RETURN(rv);

    rv = bfd_ipv4_example_init(unit, port_1, port_2);
    if (rv != BCM_E_NONE) {
        printf("(%s) \n",bcm_errmsg(rv));
        return rv;
    }
    printf("-------------init \n");


    if ((is_server || use_trap_with_snoop) && device_type<device_type_arad_plus) {
      printf("Error: Server can be supported only by arad+ devices and above.\n");
      return BCM_E_PARAM;
    }

    if (device_type<device_type_jericho) {
        /*
        * Adding BFD default endpoint
        */
        bcm_bfd_endpoint_info_t_init(&bfd_endpoint_info);

        BCM_GPORT_LOCAL_SET(bfd_endpoint_info.remote_gport, 15);

        /* remote_gport field must be a trap */
        rv = bfd_create_trap_from_gport(unit, bfd_endpoint_info.remote_gport, &trap_code);
        if (rv != BCM_E_NONE) {
            printf("Error bfd_create_trap_from_gport.\n");
            return rv;
        }
        BCM_GPORT_TRAP_SET(bfd_endpoint_info.remote_gport, trap_code, 7, 0);

        bfd_endpoint_info.flags = BCM_BFD_ENDPOINT_WITH_ID;
        if (device_type<device_type_arad_plus) {
            bfd_endpoint_info.id = -1;
        }
        else {
            bfd_endpoint_info.id = BCM_BFD_ENDPOINT_DEFAULT1;
        }
        printf("bcm_bfd_endpoint_create default\n");
        rv = bcm_bfd_endpoint_create(unit, &bfd_endpoint_info);
        if (rv != BCM_E_NONE) {
            printf("(%s) \n",bcm_errmsg(rv));
            return rv;
        }
        printf("created endpoint with id %d\n", bfd_endpoint_info.id);
    }
    /*
    * Adding BFDoIPV4 multi hop endpoint
    */

    /* If enabled, create data entry in EEDB that specifies the
    * configured IPv4 SIP and calculated checksum. */
    next_if = next_hop_mac;

    if (bfd_extended_sip) 
    {
      bcm_l3_intf_t bfd_extended_l3_intf;
      bcm_l3_intf_t_init(&bfd_extended_l3_intf);

      bcm_tunnel_initiator_t tunnel;
      bcm_tunnel_initiator_t_init(&tunnel);
      tunnel.type = bcmTunnelTypeIpAnyIn4;
      tunnel.sip = 0x10203040;
      tunnel.aux_data = 0x11ae;
      tunnel.dip = 0;
      tunnel.l3_intf_id = next_hop_mac;

      rv = bcm_tunnel_initiator_create(unit, &bfd_extended_l3_intf, &tunnel);
      if (rv != BCM_E_NONE) {
          printf("(%s) \n",bcm_errmsg(rv));
          return rv;
      }

      next_if = bfd_extended_l3_intf.l3a_intf_id;
    }

    bcm_bfd_endpoint_info_t_init(&bfd_endpoint_info);

    bfd_endpoint_info.type = bcmBFDTunnelTypeUdp;
    bfd_endpoint_info.flags = flags;

    /* if bfd_extended_sip is set, there is no need to use a SIP in the BFD endpoint */
    bfd_endpoint_info.src_ip_addr = bfd_extended_sip ? 0 : 0x30F0701;
    if ((bfd_endpoint_info.flags &BCM_BFD_ENDPOINT_IN_HW) !=0  && ((remote_gport==BCM_GPORT_INVALID) || is_server || use_trap_with_snoop)) {
      bcm_stk_sysport_gport_get(unit,port_1, &bfd_endpoint_info.tx_gport );

      bfd_endpoint_info.loc_clear_threshold = 1;
      bfd_endpoint_info.ip_ttl = 255;
      bfd_endpoint_info.ip_tos = 255;
      bfd_endpoint_info.udp_src_port = 0xC001;
      bfd_endpoint_info.egress_if = next_if;
      bfd_endpoint_info.int_pri = 1;
      bfd_endpoint_info.bfd_period = 100;
      if (is_multi_hop || device_type>=device_type_jericho) {
          /* dst IP address unavailable for Arad single hop.*/
          bfd_endpoint_info.dst_ip_addr =  0x7fffff03; 
          if (device_type>=device_type_jericho && !is_multi_hop) {
              bfd_endpoint_info.ip_subnet_length = 16; /* have the OAMP verify the 16 MSBs of incoming BFD packets.*/
          }
      }
      if (bfd_echo) {
          bfd_endpoint_info.flags |= BCM_BFD_ECHO;
          if (bfd_echo==2) {
              bfd_endpoint_info.udp_src_port = 0xffff;
              bfd_endpoint_info.src_ip_addr = 0x7fffff03;
              bfd_endpoint_info.ip_ttl = 254;
          }
      } else {
          bfd_endpoint_info.local_min_tx = 2;
          bfd_endpoint_info.local_min_rx = 0x1000; /* value in microseconds*/
          bfd_endpoint_info.local_state = 3;
          bfd_endpoint_info.local_flags = 0;
          bfd_endpoint_info.local_detect_mult = 208;
          bfd_endpoint_info.remote_detect_mult = 30;
          bfd_endpoint_info.remote_discr = 0x10002;
      }
      if (single_hop_extended || bfd_extended_sip) {
          bfd_endpoint_info.dst_ip_addr = 0x12345678;
      }
     

      if (is_server) {
          bfd_endpoint_info.flags |= BCM_BFD_ENDPOINT_HW_ACCELERATION_SET;
      }
    }

    bfd_endpoint_info.local_discr = 0x30004;
    bfd_endpoint_info.remote_gport = remote_gport;

    printf("bcm_bfd_endpoint_create bfd_endpoint_info\n"); 
    rv = bcm_bfd_endpoint_create(unit, &bfd_endpoint_info);
    if (rv != BCM_E_NONE) {
      printf("(%s) \n",bcm_errmsg(rv));
      return rv;
    }

    printf("created endpoint with id %d\n", bfd_endpoint_info.id);

    bcm_bfd_endpoint_info_t_init(&bfd_endpoint_test_info);
    printf("bcm_bfd_endpoint_get bfd_endpoint_test_info\n");
    rv = bcm_bfd_endpoint_get(unit, bfd_endpoint_info.id, &bfd_endpoint_test_info);
    if (rv != BCM_E_NONE) {
      printf("(%s) \n",bcm_errmsg(rv));
      return rv;
    }

    /* Here mep_not_acc_info and mep_not_acc_test_info are compared */
    ret = cmp_structs(&bfd_endpoint_info, &bfd_endpoint_test_info,bcmBFDTunnelTypeUdp) ;
    if (ret != 0) {
      printf("%d wrong fields in BFDoIPV4\n", ret);
    }

    printf("bcm_bfd_endpoint_destroy bfd_endpoint_info\n");
    rv = bcm_bfd_endpoint_destroy(unit, bfd_endpoint_info.id);
    if (rv != BCM_E_NONE) {
      printf("(%s) \n",bcm_errmsg(rv));
      return rv;
    }

    printf("bcm_bfd_endpoint_create bfd_endpoint_info\n");
    rv = bcm_bfd_endpoint_create(unit, &bfd_endpoint_info);
    if (rv != BCM_E_NONE) {
      printf("(%s) \n",bcm_errmsg(rv));
      return rv;
    }

    bfd_ep1.id = bfd_endpoint_info.id;

    return rv;
}

/* Example of using the default MEPs by lif profile */
int bfd_default_mep_example(int unit, int port1, int port2, char advanced_mode) {
    int rv;
    bcm_bfd_endpoint_info_t bfd_endpoint_info;
    bcm_mpls_tunnel_switch_t tunnel_switch;
    int tunnel_id, ret;
    int lif;
    int trap_code;

    port_1 = port1;
    port_2 = port2;

    tunnel_id = 99;

    rv = get_device_type(unit, &device_type);
    BCM_IF_ERROR_RETURN(rv); 

    rv = mpls__mpls_pipe_mode_exp_set(unit);
    if (rv != BCM_E_NONE) {
        printf("Error, in mpls__mpls_pipe_mode_exp_set\n");
        return rv;
    }

    rv = mpls_init(unit, &tunnel_switch, &tunnel_id, 1, port1, port2);
    if (rv != BCM_E_NONE) {
        printf("(%s) \n",bcm_errmsg(rv));
        return rv;
    }

    lif = tunnel_switch.tunnel_id;

    /* Set port profile */
    if (advanced_mode) {
        printf("Set LIF profile for port: %d\n", lif);
        rv = bcm_port_class_set(unit, lif, bcmPortClassFieldIngress, 5);
        if (rv != BCM_E_NONE) {
            printf("(%s) \n",bcm_errmsg(rv));
            return rv;
        }
        printf("Set mapping from inlif profile 5 to OAM trap profile 1\n");
    }
    else {
        printf("Set LIF profile for port: %d (LIF: 0x%x)\n", port1, lif);
    }
    rv = bcm_port_control_set(unit, lif, bcmPortControlOamDefaultProfile, 1);
    if (rv != BCM_E_NONE) {
        printf("(%s) \n",bcm_errmsg(rv));
        return rv;
    }

    /*
    * Adding BFD default endpoint
    */
    bcm_bfd_endpoint_info_t_init(&bfd_endpoint_info);

    if (advanced_mode) {
        /* This is not related to the Advanced mode, it only
           Divercifies the test */
        BCM_GPORT_LOCAL_SET(bfd_endpoint_info.remote_gport, port2);

        /* remote_gport field must be a trap */
        rv = bfd_create_trap_from_gport(unit, bfd_endpoint_info.remote_gport, &trap_code);
        if (rv != BCM_E_NONE) {
            printf("Error bfd_create_trap_from_gport.\n");
            return rv;
        }
        BCM_GPORT_TRAP_SET(bfd_endpoint_info.remote_gport, trap_code, 7, 0);
    } else {
        bfd_endpoint_info.remote_gport = BCM_GPORT_INVALID;
    }
    bfd_endpoint_info.flags = BCM_BFD_ENDPOINT_WITH_ID;
    bfd_endpoint_info.id = BCM_BFD_ENDPOINT_DEFAULT1;

    printf("bcm_bfd_endpoint_create default\n");
    rv = bcm_bfd_endpoint_create(unit, &bfd_endpoint_info);
    if (rv != BCM_E_NONE) {
        printf("(%s) \n",bcm_errmsg(rv));
        return rv;
    }
    printf("created endpoint with id %d\n", bfd_endpoint_info.id);

    return rv;
}


/*
* Creating mpls tunnel and termination. 
*/
int mpls_init(int unit, bcm_mpls_tunnel_switch_t *tunnel_switch, int *tunnel_id, int is_BFDCC, int port1, int port2) {
    bcm_error_t rv;
    uint32 next_prcl;
    int create_mpls_tunnel;

    port_1 = port1;
    port_2 = port2;


    rv= get_device_type(unit,&device_type);
    BCM_IF_ERROR_RETURN(rv);

    if (is_BFDCC) {
      create_mpls_tunnel = 1;
      next_prcl = 0;
    } else {
      create_mpls_tunnel = 0;
      next_prcl = BCM_MPLS_SWITCH_NEXT_HEADER_IPV4|BCM_MPLS_SWITCH_NEXT_HEADER_IPV6;
    }

    rv = l3_interface_init(unit, port_1, port_2, &next_hop_mac, tunnel_id, create_mpls_tunnel);
    if (rv != BCM_E_NONE) {
      printf("(%s) \n",bcm_errmsg(rv));
      return rv;
    }

    rv = mpls_add_term_entry_ex(unit, mpls_label, next_prcl, tunnel_switch);
    if (rv != BCM_E_NONE) {
      printf("(%s) \n",bcm_errmsg(rv));
      return rv;
    }

    if (is_BFDCC && (device_type <= device_type_arad_plus)) {
      /* In Jericho, GAL label is recognized by the special labels mechanism */
      rv = mpls_add_gal_entry(unit);
      if (rv != BCM_E_NONE) {
          printf("(%s) \n", bcm_errmsg(rv));
          return rv;
      }
    }


    return rv;
}

int bfd_mpls_example(int unit) {
    bcm_error_t rv;
    bcm_bfd_endpoint_info_t bfd_endpoint_info;
    bcm_bfd_endpoint_info_t bfd_endpoint_test_info;
    bcm_mpls_tunnel_switch_t tunnel_switch;
    int tunnel_id, ret;
    int bfd_supported_flags;

    rv = get_device_type(unit, &device_type);
    if (rv < 0) {
      printf("Error checking whether the device is arad+.\n");
      print rv;
      return rv;
    }

    rv = mpls_init(unit, &tunnel_switch, &tunnel_id,0,port_1, port_2);
    if (rv != BCM_E_NONE) {
      printf("(%s) \n",bcm_errmsg(rv));
      return rv;
    }

    /*
    * Adding BFDoMPLS endpoint
    */

    bcm_bfd_endpoint_info_t_init(&bfd_endpoint_info);

    bfd_endpoint_info.type = bcmBFDTunnelTypeMpls;
    if (device_type < device_type_jericho) {
      bfd_endpoint_info.gport = tunnel_switch.tunnel_id; /* in lif - instead of your discriminator (Only in Arad). */
    }
    bcm_stk_sysport_gport_get(unit,port_1, &bfd_endpoint_info.tx_gport );
    bfd_endpoint_info.dst_ip_addr = 0;
    bfd_endpoint_info.src_ip_addr = 0x30F0701;
    bfd_endpoint_info.ip_tos = 0xff;
    bfd_endpoint_info.ip_ttl = 1;
    bfd_endpoint_info.udp_src_port = 0xC001;
    bfd_endpoint_info.egress_if = tunnel_id; /* out lif */
    bfd_endpoint_info.int_pri = 1;
    bfd_endpoint_info.local_min_tx = 1;
    bfd_endpoint_info.local_min_rx = 3;
    bfd_endpoint_info.bfd_period = default_bfd_period;
    bfd_endpoint_info.local_detect_mult = 0xd0;
    bfd_endpoint_info.local_state = 3;
    bfd_endpoint_info.flags = BCM_BFD_ENDPOINT_IN_HW;
    bfd_endpoint_info.remote_discr = 0x10003;
    bfd_endpoint_info.remote_detect_mult = 30;
    bfd_endpoint_info.local_discr =  0x20002;
    bfd_endpoint_info.loc_clear_threshold =  1;
    bfd_supported_flags = soc_property_get(unit, "bfd_supported_flags_bitfield",0);
    if(bfd_supported_flags == 0x38) {
        bfd_endpoint_info.local_flags = 0;
    } else {
        bfd_endpoint_info.local_flags = 2;
    }
    bfd_endpoint_info.egress_label.label = mpls_label;
    bfd_endpoint_info.egress_label.ttl = 0xa;
    bfd_endpoint_info.egress_label.exp = 1;

    printf("bcm_bfd_endpoint_create bfd_endpoint_info\n"); 
    rv = bcm_bfd_endpoint_create(unit, &bfd_endpoint_info);
    if (rv != BCM_E_NONE) {
      printf("(%s) \n",bcm_errmsg(rv));
      return rv;
    }
    
    bfd_ep1.id = bfd_endpoint_info.id;
    printf("created endpoint with id %d\n", bfd_endpoint_info.id);

    bcm_bfd_endpoint_info_t_init(&bfd_endpoint_test_info);
    printf("bcm_bfd_endpoint_get bfd_endpoint_test_info\n");   
    rv = bcm_bfd_endpoint_get(unit, bfd_endpoint_info.id, &bfd_endpoint_test_info);
    if (rv != BCM_E_NONE) {
      printf("(%s) \n",bcm_errmsg(rv));
      return rv;
    }

    /* update --> change the min tx*/
    bfd_endpoint_info.local_min_tx = 2;
    bfd_endpoint_info.flags |= BCM_BFD_ENDPOINT_WITH_ID | BCM_BFD_ENDPOINT_UPDATE;
    printf("bcm_bfd_endpoint_get bfd_endpoint_test_info --> update\n");   
    rv = bcm_bfd_endpoint_create(unit, &bfd_endpoint_info);
    if (rv != BCM_E_NONE) {
      printf("(%s) \n",bcm_errmsg(rv));
      return rv;
    }

    bfd_endpoint_info.flags ^= BCM_BFD_ENDPOINT_WITH_ID | BCM_BFD_ENDPOINT_UPDATE;
    rv = bcm_bfd_endpoint_get(unit, bfd_endpoint_info.id, &bfd_endpoint_test_info);
    if (rv != BCM_E_NONE) {
      printf("(%s) \n",bcm_errmsg(rv));
      return rv;
    }

    /* Here mep_not_acc_info and mep_not_acc_test_info should be compared */
    rv = cmp_structs(&bfd_endpoint_info, &bfd_endpoint_test_info,bcmBFDTunnelTypeMpls ) ;
    if (rv!= 0) {
      printf("%d wrong fields in BFDoMPLS\n", rv);
    }

    mpls_label = 212;

    /* Create an additonal Endpoint on LIF defined by label 212.*/
    rv = mpls_init(unit, &tunnel_switch, &tunnel_id,0,port_1, port_2);
    if (rv != BCM_E_NONE) {
      printf("(%s) \n",bcm_errmsg(rv));
      return rv;
    }
    if (device_type < device_type_jericho) {
      bfd_endpoint_info.gport = tunnel_switch.tunnel_id; /* in lif - instead of your discriminator - Arad only.*/
    }
    bcm_stk_sysport_gport_get(unit,port_1, &bfd_endpoint_info.tx_gport );

    bfd_endpoint_info.egress_if = tunnel_id; /* out lif */
    bfd_endpoint_info.egress_label.label = mpls_label;
    bfd_endpoint_info.local_discr =  0x20003;
    rv = bcm_bfd_endpoint_create(unit, &bfd_endpoint_info);
    if (rv != BCM_E_NONE) {
      printf("(%s) \n",bcm_errmsg(rv));
      return rv;
    }
    bfd_ep2.id = bfd_endpoint_info.id;

    return rv;
}

/* 
* Add GAL label in order to avoid trap unknown_label
* Point to egress-object: egress_intf, returned by create_l3_egress
*/
int
mpls_add_gal_entry(int unit)
{
    int rv;
    bcm_mpls_tunnel_switch_t entry;

    bcm_mpls_tunnel_switch_t_init(&entry);
    entry.action = BCM_MPLS_SWITCH_ACTION_NOP;
    /* TTL decrement has to be present */
    entry.flags = BCM_MPLS_SWITCH_TTL_DECREMENT;
    /* Uniform: inherit TTL and EXP, 
     * in general valid options: 
     * both present (uniform) or none of them (Pipe)
     */
    entry.flags |= BCM_MPLS_SWITCH_OUTER_TTL|BCM_MPLS_SWITCH_OUTER_EXP;

    /* GAL label */
    entry.label = gal_label;

    /* egress attribures*/   
    entry.egress_if = 0;    

    rv = bcm_mpls_tunnel_switch_create(unit,&entry);
    if (rv != BCM_E_NONE) {
        printf("Error, in bcm_mpls_tunnel_switch_create\n");
        return rv;
    }
    return rv;
}



/* Creates a BFD endpoint of type MPLS-TP (BFD-CC)
* 
*/
int bfdCC_example(int unit,int use_user_defined_trap) {
    bcm_error_t rv;
    bcm_bfd_endpoint_info_t bfd_endpoint_info;
    bcm_mpls_tunnel_switch_t tunnel_switch;
    int tunnel_id, ret;
    int bfd_supported_flags;

    rv = get_device_type(unit, &device_type);
    if (rv < 0) {
        printf("Error checking whether the device is arad+.\n");
        print rv;
        return rv;
    }

    tunnel_id = 99;

    rv = mpls_init(unit, &tunnel_switch, &tunnel_id,1,port_1, port_2);
    if (rv != BCM_E_NONE) {
        printf("(%s) \n",bcm_errmsg(rv));
        return rv;
    }

    /*
    * Adding BFDoMPLS endpoint
    */

    bcm_bfd_endpoint_info_t_init(&bfd_endpoint_info);

    bfd_endpoint_info.type = bcmBFDTunnelTypeMplsTpCc;
    bfd_endpoint_info.gport = tunnel_switch.tunnel_id; /* in lif - instead of your discriminator */
    bfd_endpoint_info.dst_ip_addr = 0;
    bfd_endpoint_info.src_ip_addr = 0;
    bfd_endpoint_info.ip_tos = 0;
    bfd_endpoint_info.ip_ttl = 0;
    bfd_endpoint_info.udp_src_port = 0;
    if (device_type == device_type_arad_plus) {
        /* In this case the OAMP only creates the GAL label. The MPLS label is added through the encapsulation bank.*/
        bfd_endpoint_info.egress_if = tunnel_id;
        /* Gal label properties*/
        bfd_endpoint_info.egress_label.ttl = 0x40;
        bfd_endpoint_info.egress_label.exp = 0;
        bfd_endpoint_info.egress_label.label =  gal_label;
    } else {
        /* In this case the OAMP creates the MPLS label as well as the GAL.*/
        bfd_endpoint_info.egress_if = next_hop_mac;
        /* MPLS label properties*/
        bfd_endpoint_info.egress_label.label =  mpls_label_termination;
        bfd_endpoint_info.egress_label.ttl = mpls_ttl;
        bfd_endpoint_info.egress_label.exp = mpls_exp;
    }

    bfd_endpoint_info.flags = BCM_BFD_ENDPOINT_IN_HW;

    /*Set the remote_gport to a user defined trap code.*/
    if (use_user_defined_trap) {
        int trap_code;
        bcm_rx_trap_config_t trap_cpu;
        rv =  bcm_rx_trap_type_create(unit, 0, bcmRxTrapUserDefine, &trap_code);
        if (rv != BCM_E_NONE) {
           printf("(%s) \n",bcm_errmsg(rv));
           return rv;
        }
        bcm_rx_trap_config_t_init(&trap_cpu);
        trap_cpu.flags = BCM_RX_TRAP_UPDATE_DEST | BCM_RX_TRAP_TRAP;
        trap_cpu.dest_port = port_2;
        rv = bcm_rx_trap_set(unit, trap_code, &trap_cpu);
        if (rv != BCM_E_NONE) {
            printf("(%s) \n", bcm_errmsg(rv));
            return rv;
        }
        printf("Trap created trap_code=%d \n",trap_code);
        BCM_GPORT_TRAP_SET(bfd_endpoint_info.remote_gport, trap_code, 7, 0); /*Taken from default values*/
    } else {
        /* set TX fields*/
        bfd_endpoint_info.int_pri = 5;
        bfd_endpoint_info.local_discr =  0x30004;
        bfd_endpoint_info.remote_discr = 0x10002;
        bfd_supported_flags = soc_property_get(unit, "bfd_supported_flags_bitfield",0);
        if(bfd_supported_flags == 0x38) {
            bfd_endpoint_info.local_flags = 0x10;
        } else {
            bfd_endpoint_info.local_flags = 0x12;
        }
        bfd_endpoint_info.local_state = 3;
        bfd_endpoint_info.local_min_tx = 0xa;
        bfd_endpoint_info.local_min_echo = 0xbadf00d;
        bfd_endpoint_info.local_min_rx = 0xa; 
        bfd_endpoint_info.bfd_period = 100;
        bfd_endpoint_info.local_detect_mult = 208; 
        bfd_endpoint_info.remote_detect_mult = 30;
        bcm_stk_sysport_gport_get(unit,port_1, &bfd_endpoint_info.tx_gport);
    }

    printf("bcm_bfd_endpoint_create bfd_endpoint_info\n"); 
    rv = bcm_bfd_endpoint_create(unit, &bfd_endpoint_info);
    if (rv != BCM_E_NONE) {
        printf("(%s) \n",bcm_errmsg(rv));
        return rv;
    }
    printf("created endpoint with id %d\n", bfd_endpoint_info.id);

    if (!use_user_defined_trap) {
        /* endpoint replace: change the local flags.*/
        bfd_endpoint_info.flags |= BCM_BFD_ENDPOINT_WITH_ID | BCM_BFD_ENDPOINT_UPDATE;
        bfd_supported_flags = soc_property_get(unit, "bfd_supported_flags_bitfield",0);
        if(bfd_supported_flags == 0x38) {
            bfd_endpoint_info.local_flags = 0x0;
        } else {
            bfd_endpoint_info.local_flags = 0x2;
        }
        printf("bcm_bfd_endpoint_replace: changing the local_flags\n"); 
        rv = bcm_bfd_endpoint_create(unit, &bfd_endpoint_info); 
        if (rv != BCM_E_NONE) {
            printf("(%s) \n", bcm_errmsg(rv));
            return rv;
        }
    }

    return rv;
}


/*
* Creating vpls tunnel and termination 
* is_pwe_gal is only applicable for Arad+, 
* Jericho does not need GAL termination or adding CW (These are both done at the OAMP).
*/
int pwe_init(int unit, uint8 is_pwe_gal) {
    bcm_error_t rv;
    bcm_mpls_tunnel_switch_t tunnel_switch;

    if (!is_pwe_gal && pwe_ach) {
      /*  in Jericho when a packet arrives with a GAL the MPLS termination mechanism knows to look for a CW. Hence in this case the LIF need not be configured with a CW. */
      pwe_cw = 1;
    }

    mpls_lsr_init(port_1,port_2, 0, next_hop_mac, mpls_label, mpls_label, 0, 0 ,0);
    rv = vswitch_vpls_run_with_defaults_dvapi(unit,port_1,port_2);
    if (rv != BCM_E_NONE) {
      printf("(%s) \n",bcm_errmsg(rv));
      return rv;
    }

    rv = mpls_add_term_entry_ex(unit, mpls_label, 0, &tunnel_switch);
    if (rv != BCM_E_NONE) {
      printf("(%s) \n",bcm_errmsg(rv));
      return rv;
    }
    return rv;
}

/**
* Create PWE endpoint for the 4 different VCCV types.
* 
* @author sinai (23/09/2014)
* 
* @param unit 
* @param vccv_type 
* 
* @return int 
*/
int bfd_pwe_example(int unit , int vccv_type) {
    bcm_error_t rv;
    bcm_bfd_endpoint_info_t bfd_endpoint_info;
    bcm_bfd_endpoint_info_t bfd_endpoint_test_info;
    bcm_mpls_tunnel_switch_t tunnel_switch;
    int ret;
    int bfd_supported_flags;

    rv = get_device_type(unit, &device_type);
    if (rv < 0) {
      printf("Error checking whether the device is arad+.\n");
      print rv;
      return rv;
    }

    rv = pwe_init(unit, 0); 
    if (rv != BCM_E_NONE) {
      printf("(%s) \n",bcm_errmsg(rv));
      return rv;
    }

    /*
    * Adding BFDoPWE with control word endpoint
    */
    bcm_bfd_endpoint_info_t_init(&bfd_endpoint_info);

    if (vccv_type == 4) {
        bfd_endpoint_info.type = bcmBFDTunnelTypePweGal;
    } else if (vccv_type == 2) {
        bfd_endpoint_info.type = bcmBFDTunnelTypePweRouterAlert;
        if (pwe_ach) {
            /* For Router Alert Types the PWE-ACH label is optional. supported via flag.*/
            bfd_endpoint_info.flags |= BCM_BFD_ENDPOINT_PWE_ACH;
        }
    } else if (vccv_type == 3) {
        bfd_endpoint_info.type = bcmBFDTunnelTypePweTtl;
    } else {
        bfd_endpoint_info.type = bcmBFDTunnelTypePweControlWord;
    }

    bfd_endpoint_info.gport = mpls_lsr_info_1.mpls_port_id; /*in lif: mpls_port->mpls_port_id */ 
    bcm_stk_sysport_gport_get(unit,port_1, &bfd_endpoint_info.tx_gport);

    bfd_endpoint_info.dst_ip_addr = 0;
    bfd_endpoint_info.src_ip_addr = 0;
    bfd_endpoint_info.ip_tos = 0;
    bfd_endpoint_info.ip_ttl = 0;
    bfd_endpoint_info.udp_src_port = 0;
    bfd_endpoint_info.egress_if = mpls_lsr_info_1.encap_id; /* out lif: mpls_port->encap_id */
    bfd_endpoint_info.int_pri = 1;
    bfd_endpoint_info.local_min_tx = 2;
    bfd_endpoint_info.local_min_rx = 3;
    bfd_endpoint_info.bfd_period = 100;
    bfd_endpoint_info.local_detect_mult = 208;
    bfd_endpoint_info.local_state = 3;
    bfd_endpoint_info.flags |= BCM_BFD_ENDPOINT_IN_HW;
    bfd_endpoint_info.remote_discr = 0x10001;
    bfd_endpoint_info.remote_detect_mult = 30;
    bfd_endpoint_info.local_discr =  0x34;
    bfd_supported_flags = soc_property_get(unit, "bfd_supported_flags_bitfield",0);
    if(bfd_supported_flags == 0x38) {
        bfd_endpoint_info.local_flags = 0;
    } else {
        bfd_endpoint_info.local_flags = 2;
    }
    bfd_endpoint_info.egress_label.label = 2010; /* PWE Label */
    bfd_endpoint_info.egress_label.ttl = (vccv_type==3) ? 1 : 0xa;
    bfd_endpoint_info.egress_label.exp = 1;

    printf("bcm_bfd_endpoint_create bfd_endpoint_info\n"); 
    rv = bcm_bfd_endpoint_create(unit, &bfd_endpoint_info);
    if (rv != BCM_E_NONE) {
      printf("(%s) \n",bcm_errmsg(rv));
      return rv;
    }

    printf("created endpoint with id %d\n", bfd_endpoint_info.id);

    bcm_bfd_endpoint_info_t_init(&bfd_endpoint_test_info);
    printf("bcm_bfd_endpoint_get bfd_endpoint_test_info\n");   
    rv = bcm_bfd_endpoint_get(unit, bfd_endpoint_info.id, &bfd_endpoint_test_info);
    if (rv != BCM_E_NONE) {
      printf("(%s) \n",bcm_errmsg(rv));
      return rv;
    }

    /* Here mep_not_acc_info and mep_not_acc_test_info should be compared */
    ret =cmp_structs(&bfd_endpoint_info, &bfd_endpoint_test_info,bcmBFDTunnelTypePweControlWord ) ;
    if (ret != 0) {
      printf("%d wrong fields in BFDoPWE\n", ret);
    }

    printf("bcm_bfd_endpoint_destroy bfd_endpoint_info\n"); 
    rv = bcm_bfd_endpoint_destroy(unit, bfd_endpoint_info.id);
    if (rv != BCM_E_NONE) {
      printf("(%s) \n",bcm_errmsg(rv));
      return rv;
    }

    printf("bcm_bfd_endpoint_create bfd_endpoint_info\n"); 
    rv = bcm_bfd_endpoint_create(unit, &bfd_endpoint_info);
    if (rv != BCM_E_NONE) {
      printf("(%s) \n",bcm_errmsg(rv));
      return rv;
    }

    bfd_ep1.gport = bfd_endpoint_info.gport;
    bfd_ep1.id = bfd_endpoint_info.id;

    return rv;
}

/* Creates a BFD endpoint of type MPLS-TP (BFD-CC)
* 
*/
int bfd_pwe_gal_example(int unit) {
    bcm_error_t rv;
    bcm_bfd_endpoint_info_t bfd_endpoint_info;
    bcm_mpls_tunnel_switch_t tunnel_switch;
    int tunnel_id, ret;

    rv = get_device_type(unit, &device_type);
    if (rv < 0) {
        printf("Error checking whether the device is arad+.\n");
        print rv;
        return rv;
    }

    if (device_type<device_type_arad_plus) {
        printf("bcmBFDTunnelTypePweGal is supported only on Arad+\n");
        return rv;
    }

    is_gal = 1; /* Enabling GAL termination */
    rv = pwe_init(unit, 1);
    if (rv != BCM_E_NONE) {
        printf("(%s) \n", bcm_errmsg(rv));
        return rv;
    }

    /*
    * Adding BFDoGACHoGALoPWE endpoint
    */

    bcm_bfd_endpoint_info_t_init(&bfd_endpoint_info);

    bfd_endpoint_info.type = bcmBFDTunnelTypePweGal;
    bfd_endpoint_info.gport = network_port_id; /*in lif: mpls_port->mpls_port_id */ 
    bfd_endpoint_info.dst_ip_addr = 0;
    bfd_endpoint_info.src_ip_addr = 0;
    bfd_endpoint_info.ip_tos = 0;
    bfd_endpoint_info.ip_ttl = 0;
    bfd_endpoint_info.udp_src_port = 0;
    bfd_endpoint_info.egress_if = BCM_GPORT_SUB_TYPE_LIF_VAL_GET(network_port_id);
    BCM_L3_ITF_SET(bfd_endpoint_info.egress_if, BCM_L3_ITF_TYPE_LIF, bfd_endpoint_info.egress_if);
    bfd_endpoint_info.flags = BCM_BFD_ENDPOINT_IN_HW;
    bfd_endpoint_info.local_discr =  0x30004;
    bfd_endpoint_info.egress_label.label = gal_label;
    bfd_endpoint_info.egress_label.ttl = 0x40;
    bfd_endpoint_info.egress_label.exp = 0;

    /* set TX fields*/
    bfd_endpoint_info.int_pri = 5;
    bfd_endpoint_info.remote_discr = 0x10002;
    bfd_endpoint_info.local_flags = 0x12;
    bfd_endpoint_info.local_state = 3;
    bfd_endpoint_info.local_min_tx = 0xa;
    bfd_endpoint_info.loc_clear_threshold =  1;
    bfd_endpoint_info.local_min_echo = 0xbadf00d;
    bfd_endpoint_info.local_min_rx = 0xa;
    bfd_endpoint_info.bfd_period = 100;
    bfd_endpoint_info.local_detect_mult = 208;
    bfd_endpoint_info.remote_detect_mult = 30;
    bcm_stk_sysport_gport_get(unit,port_1, &bfd_endpoint_info.tx_gport);
    printf("bcm_bfd_endpoint_create bfd_endpoint_info\n");
    rv = bcm_bfd_endpoint_create(unit, &bfd_endpoint_info);
    if (rv != BCM_E_NONE) {
        printf("(%s) \n",bcm_errmsg(rv));
        return rv;
    }
    bfd_ep1.id = bfd_endpoint_info.id;
    printf("created endpoint with id %d\n", bfd_endpoint_info.id);

    return rv;
}

/* This is an example of using bcm_bfd_event_register api.
* A simple callback is created for CCM timeout event. 
* After a mep and rmep are created, the callback is called 
* whenever CCMTimeout event is generated. 
*/
int cb_bfd(
int unit, 
uint32 flags, 
bcm_bfd_event_types_t event_types, 
bcm_bfd_endpoint_t endpoint, 
void *user_data)
{
    int event_i;
    int rv;
    print unit;
    print flags;

    for (event_i = 0; event_i < bcmBFDEventCount; event_i++) {

        if (BCM_BFD_EVENT_TYPE_GET(event_types, event_i)) {
            print event_i;
            rv = get_device_type(unit, &device_type);
            if (rv < 0) {
                printf("Error checking whether the device is arad+.\n");
                print rv;
                return rv;
            }
            if (bcmBFDEventStateChange==event_i && device_type>=device_type_arad_plus ) {
                /* The event change interrupt does not convey information about the new flags/state/diags.
                   Instead it possible to get these through endpoint_get (in Arad+)*/
                bcm_bfd_endpoint_info_t ep_get;
                rv = bcm_bfd_endpoint_get(unit, endpoint, &ep_get);
                if (rv != BCM_E_NONE) {
                    printf("(%s) \n", bcm_errmsg(rv));
                    return rv;
                }
                printf("Flags is : 0x%x\n state is \n", ep_get.remote_flags); 
                print ep_get.remote_state;
                print "diag is:";
                print ep_get.remote_diag;

            }
        }
    }
    print endpoint;
    timeout_events_count_bfd++;
    if (flags & BCM_OAM_EVENT_FLAGS_MULTIPLE) {
        ++timeout_events_count_multi_bfd;
    }
    return BCM_E_NONE;
}

int register_events(int unit) {
    bcm_error_t rv;
    bcm_bfd_event_types_t e;

    BCM_BFD_EVENT_TYPE_SET(e, bcmBFDEventEndpointTimeout);
    rv = bcm_bfd_event_register(unit, e, cb_bfd, (void*)2);
    BCM_IF_ERROR_RETURN(rv);

    BCM_BFD_EVENT_TYPE_SET(e, bcmBFDEventEndpointTimein);
    rv = bcm_bfd_event_register(unit, e, cb_bfd, (void*) 1 );
    BCM_IF_ERROR_RETURN(rv);

    BCM_BFD_EVENT_TYPE_SET(e, bcmBFDEventStateChange);
    rv = bcm_bfd_event_register(unit, e, cb_bfd, (void*) 1 );
    BCM_IF_ERROR_RETURN(rv);

    return rv;
}


    /**
    * 
    * 
    * @author sinai (03/09/2015)
    * 
    * @param unit 
    * @param port1 Port on which the endpoint resides
    * @param port2 
    * @param remote_port trap to a non default destination (may be 
    *  				  -1)
    * @param is_extended_sip 
    * @param flags: supported options: 
    *        BCM_BFD_ENDPOINT_MULTIHOP: multi hop/single hop
    *        indication
    *        BCM_BFD_ENDPOINT_IN_HW: accelerated in OAMP 
    *        indication
    *        BCM_BFD_ENDPOINT_MICRO_BFD: Jericho only
    * @param remote_port_fwd_strength:
    *           Indicates the trap strength to be set in
    *           remote_gport trap variable. 0 to indicate default
    *           behavior of setting 7.
    * 
    * @return int 
    */
int bfd_ipv4_run_with_defaults(int unit, int port1, int port2,  int remote_port,  int is_extended_sip, uint32 flags, int remote_port_fwd_strength) {
    bcm_error_t rv;

    port_1 = port1;
    port_2 = port2;
    bcm_gport_t remote_gport;
    int trap_code;

    /*int is_single_hop , int is_acc, */

    if (is_extended_sip && soc_property_get(unit , "bfd_extended_ipv4_src_ip",1)) {
        bfd_extended_sip = 1;
    }

    rv = get_device_type(unit, &device_type);
    if (rv < 0) {
      printf("Error checking whether the device is arad+.\n");
      print rv;
      return rv;
    }

    /* Using LEM/PMF to demultiplex BFD endpoints for:  BFD single hop or Micro BFD, when the soc property is on. */
    if ( ((flags & BCM_BFD_ENDPOINT_MULTIHOP)==0  || (flags & BCM_BFD_ENDPOINT_MICRO_BFD )!=0) 
       && soc_property_get(unit, "bfd_ipv4_single_hop_extended", 1) 
       && device_type>=device_type_arad_plus) {
      single_hop_extended = 1;
    }

    /* Micro BFD may only be set with soc property and may not be set with multihop flags*/
    if ((flags & BCM_BFD_ENDPOINT_MICRO_BFD )!=0 && soc_property_get(unit, "bfd_ipv4_single_hop_extended", 1) ==0 ) {
      printf("Error: MICRO_BFD may only be configured with bfd_ipv4_single_hop_extended soc property.\n");
      print 34;
    }

    if ((flags & BCM_BFD_ENDPOINT_MICRO_BFD )!=0 && (flags & BCM_BFD_ENDPOINT_MULTIHOP )!=0 ) {
      printf("Error: BCM_BFD_ENDPOINT_MICRO_BFD may only be configured with BCM_BFD_ENDPOINT_MULTIHOP.\n");
      print 34;
    }

    rv = mpls__mpls_pipe_mode_exp_set(unit);
    if (rv != BCM_E_NONE) {
      printf("Error, in mpls__mpls_pipe_mode_exp_set\n");
      return rv;
    }

    if (remote_port == -1) {
      remote_gport = BCM_GPORT_INVALID;
    }
    else {
      if (is_server || use_trap_with_snoop) {
          BCM_GPORT_TRAP_SET(remote_gport, remote_port, remote_port_fwd_strength?remote_port_fwd_strength:7, 3);
      }
      else {
          printf("Setting remote_gport to port 0x%08x with strength %d\n",
                  remote_port, remote_port_fwd_strength?remote_port_fwd_strength:7);
          BCM_GPORT_LOCAL_SET(remote_gport, remote_port);

          /* remote_gport field must be a trap */
          rv = bfd_create_trap_from_gport(unit, remote_gport, &trap_code);
          if (rv != BCM_E_NONE) {
              printf("Error bfd_create_trap_from_gport.\n");
              return rv;
          }
          BCM_GPORT_TRAP_SET(remote_gport, trap_code, remote_port_fwd_strength?remote_port_fwd_strength:7, 0);
          remote_gport_trap_code = trap_code;
      }
    }

    /* in case of sigle hop extended the PMF should do the clsification*/
    if (single_hop_extended) {
    rv = bfd_ipv4_single_hop_field_action(unit);
    if (rv != BCM_E_NONE) {
      printf("Error bfd_ipv4_single_hop_field_trap_qualifier.\n");
      return rv;
    }
    }

    rv = bfd_ipv4_example(unit, remote_gport, flags); 
    if (rv != BCM_E_NONE) {
      printf("(%s) \n",bcm_errmsg(rv));
      return rv;
    }

    rv = register_events(unit);
    return rv;
}

int bfd_mpls_run_with_defaults(int unit, int port1, int port2) {
    bcm_error_t rv;

    port_1 = port1;
    port_2 = port2;

    rv = mpls__mpls_pipe_mode_exp_set(unit);
    if (rv != BCM_E_NONE) {
      printf("Error, in mpls__mpls_pipe_mode_exp_set\n");
      return rv;
    }

    rv =  bfd_mpls_example(unit);
    if (rv != BCM_E_NONE) {
      printf("(%s) \n",bcm_errmsg(rv));
      return rv;
    }

    rv = register_events(unit);
    return rv;
}


int bfd_cc_run_with_defaults(int unit, int port1, int port2, int use_user_defined_trap ) {
    bcm_error_t rv;

    port_1 = port1;
    port_2 = port2; 

    rv = mpls__mpls_pipe_mode_exp_set(unit);
    if (rv != BCM_E_NONE) {
      printf("Error, in mpls__mpls_pipe_mode_exp_set\n");
      return rv;
    }

    rv = bfdCC_example(unit, use_user_defined_trap);
    if (rv != BCM_E_NONE) {
      printf("(%s) \n",bcm_errmsg(rv));
      return rv;
    }

    rv = register_events(unit);
    return rv;
}


    /**
    * BFD over PWE, according to VCCV Type: 
    * Type 1: BFD over CW (ACH)  over PWE 
    * Type 2: PWE  over router alert 
    * Type 3: PWE TTL=1 (unsupported) 
    * Type 4: BFD over G-ACH over GAL over PWE.
    *  
    * @author sinai (23/09/2014)
    * 
    * @param unit 
    * @param port1 
    * @param port2 
    * @param vccv_type - must be 1,2,4
    * 
    * @return int 
    */
int bfd_pwe_run_with_defaults(int unit, int port1, int port2, int vccv_type) {
    bcm_error_t rv;

    port_1 = port1;
    port_2 = port2;

    rv = mpls__mpls_pipe_mode_exp_set(unit);
    if (rv != BCM_E_NONE) {
        printf("Error, in mpls__mpls_pipe_mode_exp_set\n");
        return rv;
    }

    rv = get_device_type(unit, &device_type);
    if (rv < 0) {
        printf("Error checking whether the device is arad+.\n");
        print rv;
        return rv;
    }

    /* In QAX all 4 VCCV types are supported */
    if (device_type<device_type_qax) {
        if (device_type>=device_type_jericho) {
             /* In Jericho VCCV type 3 is not supported */
            if (vccv_type != 1 && vccv_type != 2 && vccv_type != 4) {
                printf("Only VCCV Types 1,2,4 supported\n");
                return 99;
            }
        } else if (device_type==device_type_arad_plus) {
            /* In Arad Plus VCCV types 2,3 are not supported */
            if (vccv_type != 1 &&  vccv_type != 4) {
                printf("Only VCCV Types 1,4 supported\n");
                return 99;
            }
        } else {
            if (vccv_type != 1) {
                /* In Arad only VCCV type 1 is supported */
                printf("Only VCCV Type 1 supported\n");
                return 99;
            }
        }
    }


    if (device_type==device_type_arad_plus && vccv_type == 4) {
        /* PWE GAL handled serparatly in Arad+*/
        rv =  bfd_pwe_gal_example(unit);
        if (rv != BCM_E_NONE) {
            printf("(%s) \n", bcm_errmsg(rv));
            return rv;
        }
    }

    rv =  bfd_pwe_example(unit, vccv_type);
    if (rv != BCM_E_NONE) {
        printf("(%s) \n", bcm_errmsg(rv));
        return rv;
    }

    rv = register_events(unit);
    return rv;
}

int bfd_pwe_gal_run_with_defaults(int unit, int port1, int port2) {
    bcm_error_t rv;

    port_1 = port1;
    port_2 = port2;

    rv = mpls__mpls_pipe_mode_exp_set(unit);
    if (rv != BCM_E_NONE) {
      printf("Error, in mpls__mpls_pipe_mode_exp_set\n");
      return rv;
    }

    rv =  bfd_pwe_gal_example(unit);
    if (rv != BCM_E_NONE) {
      printf("(%s) \n",bcm_errmsg(rv));
      return rv;
    }

    rv = register_events(unit);
    return rv;
}


    /* 
    * function compares endpoint_info_t created with bfd_mpls_endpoint_info_create() and a struct returned from bfd_endpoint_info_get().
    *  type may be bcmBFDTunnelTypePweControlWord,  bcmBFDTunnelTypeMpls or bcmBFDTunnelTypeUdp
    *  function returns 0 upon success, number of wrong fields upon failure.
    */
int cmp_structs(bcm_bfd_endpoint_info_t* endpoint_create,bcm_bfd_endpoint_info_t* endpoint_get, int type  ){
    int rv =0;
    if (endpoint_create->flags != endpoint_get->flags ) {
      printf("Error: endpoint_info_t struct returned from endpoint_info_get not the same as the one created with endpoint_create() - flags fields are different.\n");
      ++rv;
    }
    if (endpoint_create->id != endpoint_get->id ) {
      printf("Error: endpoint_info_t struct returned from endpoint_info_get not the same as the one created with endpoint_create() - id fields are different.\n");
      ++rv;
    }
    if (endpoint_create->type != endpoint_get->type ) {
      printf("Error: endpoint_info_t struct returned from endpoint_info_get not the same as the one created with endpoint_create() - type fields are different.\n");
      ++rv;
    }
    if (endpoint_create->gport != endpoint_get->gport ) {
      printf("Error: endpoint_info_t struct returned from endpoint_info_get not the same as the one created with endpoint_create() - gport fields do not match.\n Expected %d, got %d.\n",
                endpoint_create->gport, endpoint_get->gport);
      ++rv;
    }
    if (endpoint_create->remote_gport != endpoint_get->remote_gport ) {
      printf("Error: endpoint_info_t struct returned from endpoint_info_get not the same as the one created with endpoint_create() - gport fields do not match.\n");
      ++rv;
    }

    if (endpoint_create->src_ip_addr != endpoint_get->src_ip_addr ) {
      printf("Error: endpoint_info_t struct returned from endpoint_info_get not the same as the one created with endpoint_create() - src_ip_addr fields do not match. Expected %d, got %d\n",
                endpoint_create->src_ip_addr, endpoint_get->src_ip_addr );
      ++rv;
    }
    if (endpoint_create->ip_tos != endpoint_get->ip_tos ) {
      printf("Error: endpoint_info_t struct returned from endpoint_info_get not the same as the one created with endpoint_create() - ip_tos fields do not match.\n");
      ++rv;
    }
    if (endpoint_create->ip_ttl != endpoint_get->ip_ttl ) {
      printf("Error: endpoint_info_t struct returned from endpoint_info_get not the same as the one created with endpoint_create() - ip_ttl fields do not match.\n");
     ++rv;
    }
    if (endpoint_create->udp_src_port != endpoint_get->udp_src_port ) {
      printf("Error: endpoint_info_t struct returned from endpoint_info_get not the same as the one created with endpoint_create() - udp_src_port fields do not match.\n");
     ++rv;
    }
    if (endpoint_create->egress_if !=endpoint_get->egress_if ) {
      printf("Error: endpoint_info_t struct returned from endpoint_info_get not the same as the one created with endpoint_create() - egress_if fields do not match.\n");
      ++rv;
    }
    if (endpoint_create->int_pri != endpoint_get->int_pri ) {
      printf("Error: endpoint_info_t struct returned from endpoint_info_get not the same as the one created with endpoint_create() - int_pri fields do not match.\n");
      ++rv;
    }
    if (endpoint_create->local_discr != endpoint_get->local_discr ) {
      printf("Error: endpoint_info_t struct returned from endpoint_info_get not the same as the one created with endpoint_create() - local_discr fields do not match.\n");
      ++rv;
    }
    if (endpoint_create->local_min_tx != endpoint_get->local_min_tx ) {
      printf("Error: endpoint_info_t struct returned from endpoint_info_get not the same as the one created with endpoint_create() - local_min_tx fields do not match.\n");
      ++rv;
    }
    if (endpoint_create->bfd_period - endpoint_get->bfd_period > 10) {
      printf("Error: endpoint_info_t struct returned from endpoint_info_get not the same as the one created with endpoint_create() - bfd_period fields do not match.\n");
      ++rv;
    }
    if (endpoint_create->local_min_rx != endpoint_get->local_min_rx ) {
      printf("Error: endpoint_info_t struct returned from endpoint_info_get not the same as the one created with endpoint_create() - local_min_rx fields do not match.\n");
      ++rv;
    }
    if (endpoint_create->local_detect_mult != endpoint_get->local_detect_mult ) {
      printf("Error: endpoint_info_t struct returned from endpoint_info_get not the same as the one created with endpoint_create() - local_detect_mult fields do not match.\n");
      ++rv;
    }
    if (endpoint_create->local_state !=endpoint_get->local_state ) {
      printf("Error: endpoint_info_t struct returned from endpoint_info_get not the same as the one created with endpoint_create() - local_state fields do not match.\n");
      ++rv;
    }
    if (endpoint_create->remote_discr != endpoint_get->remote_discr) {
      printf("Error: endpoint_info_t struct returned from endpoint_info_get not the same as the one created with endpoint_create() - remote_discr fields do not match.\n");
      ++rv;
    }
    if (endpoint_create->local_diag != endpoint_get->local_diag) {
      printf("Error: endpoint_info_t struct returned from endpoint_info_get not the same as the one created with endpoint_create() - local_diag fields do not match.\n");
      ++rv;
    }
    if (type == bcmBFDTunnelTypeUdp && ((endpoint_create->flags & BCM_BFD_ENDPOINT_MULTIHOP)==0)) { /*ip single hop - dest ip should be zero.*/
       if (endpoint_get->dst_ip_addr != 0 ) {
          printf("Error, BFDoIPV4: dst_ip_addr field in endpoint_info_t field returned from ednpoint_info_get non zero on single hop ip encapsulation. Dest addr is %d\n",endpoint_get->dst_ip_addr);
          ++rv;
      }
    }
    else{ /* in other cases dest_addresses must match*/
      if (endpoint_get->dst_ip_addr != endpoint_create->dst_ip_addr ) {
          printf("Error: endpoint_info_t struct returned from endpoint_info_get not the same as the one created with endpoint_create() - dst_ip_addr fields do not match.\n");
          ++rv;
      }
    }

    return rv;
      
}


    /**
    * Creates a trap with snoop command to specified trap and snoop
    * GPorts. Returns the created trap code in *trap_code.
    */
int trap_snoop_create(int unit, bcm_gport_t trap_gport,
                      bcm_gport_t snoop_gport, int* trap_code) {

    bcm_error_t rv;
    bcm_oam_endpoint_action_t action;
    bcm_gport_t gport;
    bcm_rx_trap_config_t trap_config_snoop;
    bcm_rx_snoop_config_t snoop_config_cpu;
    int snoop_cmnd;

    /* First allocate a new snoop command */
    rv = bcm_rx_snoop_create(unit, 0/*flags*/, &snoop_cmnd);
    if (rv != BCM_E_NONE) {
        printf("(%s) \n",bcm_errmsg(rv));
        return rv;
    }
    print snoop_cmnd;

    /* Setup the new snoop */
    bcm_rx_snoop_config_t_init(&snoop_config_cpu);
    snoop_config_cpu.flags = (BCM_RX_SNOOP_UPDATE_DEST | BCM_RX_SNOOP_UPDATE_PRIO);
    snoop_config_cpu.dest_port = snoop_gport;
    snoop_config_cpu.size = -1;
    snoop_config_cpu.probability = 100000;

    print snoop_config_cpu;

    rv =  bcm_rx_snoop_set(unit, snoop_cmnd, &snoop_config_cpu);
    if (rv != BCM_E_NONE) {
        printf("(%s) \n",bcm_errmsg(rv));
        return rv;
    }

    /* Now create a new trap code */
    rv =  bcm_rx_trap_type_create(unit, 0, bcmRxTrapUserDefine, trap_code);
    if (rv != BCM_E_NONE) {
        printf("(%s) \n",bcm_errmsg(rv));
        return rv;
    }
    print *trap_code;

    /* Setup the new trap */
    bcm_rx_trap_config_t_init(&trap_config_snoop);
    trap_config_snoop.flags = BCM_RX_TRAP_UPDATE_DEST
        | BCM_RX_TRAP_UPDATE_FORWARDING_HEADER | BCM_RX_TRAP_TRAP;
    trap_config_snoop.snoop_strength = 3;
    trap_config_snoop.snoop_cmnd = snoop_cmnd; /*Connect the snoop command to the trap*/
    trap_config_snoop.forwarding_header = bcmRxTrapForwardingHeaderOamBfdPdu;
    trap_config_snoop.dest_port = trap_gport;

    rv = bcm_rx_trap_set(unit, *trap_code, &trap_config_snoop);
    if (rv != BCM_E_NONE) {
        printf("(%s) \n",bcm_errmsg(rv));
    }
    return rv;

}

    /**
    *  Configure snooping to CPU packets that are trapped to the
    *  OAMP.
    *  Use this function to create a new trap code with OAMP
    *  destination that's connected to a snoop to snoop_port
    *  command.
    *
    */
int acc_snoop_init(int unit, int snoop_port) {

    bcm_error_t rv;
    bcm_gport_t oamp_gport;

    rv = oamp_gport_get(unit, &oamp_gport);
    if (rv != BCM_E_NONE) {
        printf("(%s) \n",bcm_errmsg(rv));
        return rv;
    }
    print oamp_gport;

    rv = trap_snoop_create(unit, oamp_gport, snoop_port, &trap_code_4_snooping);
    if (rv != BCM_E_NONE) {
        printf("(%s) \n",bcm_errmsg(rv));
    }
    return rv;
}

/*bfd ipv4 single tunnel add and delete operation*/
int bfd_ipv4_tunnel_operation(int unit, int out_port) {
    bcm_error_t rv;
    int out_vlan = 100;
    bcm_mac_t mac_address  = ip_tunnel_my_mac_get();/* my-MAC {0x00, 0x0c, 0x00, 0x02, 0x00, 0x00};  */
    bcm_mac_t next_hop_mac = {0x00, 0x00, 0x00, 0x00, 0xcd, 0x1d}; /* next_hop_mac1 */
    bcm_tunnel_initiator_t tunnel_1;

    int tunnel_itf1=0;

    /*** create egress router interface ***/
    rv = vlan__open_vlan_per_mc(unit, out_vlan, 0x1);
    if (rv != BCM_E_NONE) {
        printf("Error, open_vlan=%d, in unit %d \n", out_vlan, unit);
        return rv;
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
    if (rv != BCM_E_NONE) {
        printf("Error, l3__intf_rif__create\n");
        return rv;
    }

    /*** create IP tunnel ***/
    bcm_tunnel_initiator_t_init(&tunnel_1);
    tunnel_1.dip = 0x0;
    tunnel_1.sip = 0xA0000011; /* 160.0.0.17*/
    tunnel_1.dscp = 10;
    tunnel_1.flags = 0x0;
    tunnel_1.ttl = 60;
    tunnel_1.type = bcmTunnelTypeIpAnyIn4;
    tunnel_1.vlan = out_vlan;
    tunnel_1.dscp_sel = bcmTunnelDscpAssign;

    rv = add_ip_tunnel(unit,&tunnel_itf1,&tunnel_1);
    if (rv != BCM_E_NONE) {
        printf("Error, add_ip_tunnel 1\n");
    }

    rv = remove_ip_tunnel(unit,tunnel_itf1);
    if (rv != BCM_E_NONE) {
        printf("Error, remove_ip_tunnel 1\n");
    }

    return rv;
}

/* This function create UserDefined trap with CPU destination
 * And create ep=0(your discriminator 'o) that will that will be trapped
 */
int bfd_your_discriminator_null(int unit) {

    bcm_error_t rv;
    bcm_bfd_endpoint_info_t bfd_endpoint_info;
    int trap_code;

     /* Create trap to CPU */
     rv = bfd_create_trap_from_gport(unit, 0, &trap_code);

    /* Create ep 0 */
    bcm_bfd_endpoint_info_t_init(&bfd_endpoint_info);

    bfd_endpoint_info.type = bcmBFDTunnelTypeUdp;
    bfd_endpoint_info.gport = BCM_GPORT_INVALID;
    bfd_endpoint_info.dst_ip_addr = 0;
    bfd_endpoint_info.src_ip_addr = 0;
    bfd_endpoint_info.ip_tos = 0;
    bfd_endpoint_info.ip_ttl = 0;
    bfd_endpoint_info.udp_src_port = 0;
    bfd_endpoint_info.flags = 0;
    bfd_endpoint_info.local_discr =  0x0;

    BCM_GPORT_TRAP_SET(bfd_endpoint_info.remote_gport, trap_code, 7, 0); /*Taken from default values*/
    rv = bcm_bfd_endpoint_create(0, &bfd_endpoint_info);

    return rv;
}
