/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~BFD test~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*
 *
 * $Id: cint_bfd_ipv6.c,v 1.15 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 * 
 * File: cint_bfd_ipv6.c
 * Purpose: Example of using BFD IPv6 APIs that tests for BFD connection to bring seesions UP.
 *
 * Soc Properties configuration:
 *     mcs_load_uc0=bfd_bhh to configure to load the mcs image.
 *     custom_feature_bfd_ipv6_mode=2 to configure to support the BFD over IPv6 by uKernel
 *     bfd_simple_password_keys=8 to configure the simple password auth key number.
 *     bfd_sha1_keys=8 to configure the sha1 auth key number
 *     port_priorities.BCM88650=8 to configure the number of priorities at egq.
 *     num_queues_pci=32 to configure the ARAD cosq numbers, from 0-32.
 *     num_queues_uc0=8 to configure the ARM core 0 cosq numbers, from 32-40. 
 *     ucode_port_204.BCM88650=CPU.32 to configure local port 204 to send the DSP packet. 
 *     tm_port_header_type_in_204.BCM88650=INJECTED_2_PP 
 *     tm_port_header_type_out_204.BCM88650=CPU to trap the BFD packet to uKernel with the FTMH and PPH header
 *
 * Service Model:
 *     Back-to-back tests using 2 ARAD_PLUS(A, B):
 *     1) Send the BFD IPv6 packet from port_a of arad_plus_a to port_b of arad_plus_b.
 *     2) Send the BFD IPv6 packet from port_b of arad_plus_b to port_a of arad_plus_a.
 * Service connection:
 *     --------------                 --------------
 *     | arad_plus_a  |<--------->| arad_plus_b  |
 *     --------------                 --------------
 *                        port_a       port_b
 * 
 * To run IPV6 test:
 * BCM> cint examples/dpp/utility/cint_utils_global.c
 * BCM> cint examples/dpp/utility/cint_utils_l3.c
 * BCM> cint examples/dpp/cint_ip_route.c
 * BCM> cint examples/dpp/cint_bfd.c 
 * BCM> cint examples/dpp/cint_bfd_ipv6.c 
 * BCM> cint examples/dpp/cint_field_bfd_ipv6_single_hop.c
 * BCM> cint 
 * print bfd_ipv6_single_hop_field_action(unit);
 * print bfd_ipv6_service_init(unit, in_port, out_port, type);
 * print bfd_ipv6_run_with_defaults(unit, endpoint_id, is_acc, is_single_hop, auth_type, auth_index, punt_rate);
 * 
 *  
 * comments:
 * 1) Normally, bfd_ipv6_trap(unit) is not used.
 *     But if the bfd_ipv6_single_hop_field_action(unit) not works to trap the BFD packet to uKernel, 
 *     we can use the bfd_ipv6_trap(unit) instead to trap all BFD single hop packet to uKernel for 
 *     debug purpose.
 * 2) For simple password auth and sha1 auth, you can select the key by key_index.
 */
 
/* debug prints */
int verbose = 1;

/* Test Type */
int type = 1; /* 1 - Single Tagged; 2 - Double Tagged */

vlan_edit_utils_s g_eve_edit_double_tagged = {3,    /* edit profile */
                                            0,    /* tag format */
                                            8};   /* action ID */

vlan_edit_utils_s g_eve_edit_lag = {4,    /* edit profile */
                                    0,    /* tag format */
                                    9};   /* action ID */
int bfd_in_port[2] = {13,15};
int bfd_out_port[2] = {14,16};
int bfd_remote_port = BCM_GPORT_INVALID;
int user_defined_trap_code = 0;
int bfd_dsp_port = 204;/* the bfd_dsp_port directly connect to the ARM core 0 */
int bfd_encap_id;
uint32 local_disc = 5010;
uint32 remote_disc = 10;
bcm_ip6_t src_ipv6 = {0xfe,0xc0,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x02,0x00,0x02,0x00,0x02};
bcm_ip6_t dst_ipv6 = {0xfe,0xc0,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x00,0x01,0x00,0x01};

/* Trunk info */
int is_trunk = 0;
int member_count=2; 

bcm_trunk_member_t  member[member_count];
bcm_trunk_info_t trunk_info[2];
uint32  trunk_flags = 0;
int trunk_gport[2];
bcm_trunk_t trunk_id[2]; 
/* BFD event count */
int array_num = bcmBFDEventCount;
uint32 bfd_event_count[array_num] = {0};

/* simple password auth configuration */
uint8 auth_sp_num = 4;
uint8 auth_sp_len[4] = { 8, 8, 16, 16};
uint8 auth_sp_key[4][16]= {
    {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f},
    {0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f},
    {0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f},
    {0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f}};

/* sha1 auth configuration */
uint8 auth_sha1_num = 4;
uint32 auth_sha1_seq[4] = {0x11223344, 0x55667788, 0x99aabbcc, 0xddeeff00 };
uint8 auth_sha1_key[4][20]= {
    {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10, 0x11, 0x12, 0x13},
    {0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f, 0x20, 0x21, 0x22, 0x23},
    {0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f, 0x30, 0x31, 0x32, 0x33},
    {0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f, 0x40, 0x41, 0x42, 0x43}};

/*
 * Add IPV6 Host
 *
 */
int add_ipv6_host(int unit, bcm_ip6_t *addr, int vrf, int fec, int intf) {
  int rc;
  bcm_l3_host_t l3host;

  bcm_l3_host_t_init(l3host);

  l3host.l3a_flags =  BCM_L3_IP6;
  sal_memcpy(&(l3host.l3a_ip6_addr),(addr),16);
  l3host.l3a_vrf = vrf;
  /* dest is FEC + OutLif */
  l3host.l3a_intf = fec; /* fec */
  /* encap_id == 0 for IPv6 host entries in KAPS (Jericho and above) */
  l3host.encap_id = is_device_or_above(unit, JERICHO) ? 0 : intf;
  l3host.l3a_modid = 0;
  l3host.l3a_port_tgid = 0;

  rc = bcm_l3_host_add(unit, l3host);
  if (rc != BCM_E_NONE) {
    printf ("bcm_l3_host_add failed: %d \n", rc);
  }
  return rc;
}

/*
 *Initialize the BFD application configuration for local or remote device.
 */
int bfd_ipv6_service_init(int unit, int in_port, int in_port_2, int out_port, int out_port_2, int is_trunk, int remote_port) {
    int rv = 0;
    int i;
    
    bfd_in_port[0] = in_port;
    bfd_out_port[0] = out_port;

    bfd_remote_port = remote_port;

    if (is_trunk) {
        bfd_in_port[1] = in_port_2;
        bfd_out_port[1] = out_port_2;  
        
        for (i = 0; i < 2; i++ ) {
            /* Create Trunk Port */
            rv = bcm_trunk_create(unit, trunk_flags, &trunk_id[i]);
            if (rv != BCM_E_NONE) {
                printf("Error,create trunk id:0x0%x failed $rv\n",trunk_id[i]);
                return rv;
            }

            rv = bcm_trunk_psc_set(unit, trunk_id[i], BCM_TRUNK_PSC_ROUND_ROBIN);
            if (rv != BCM_E_NONE) {
                printf("Error in bcm_trunk_psc_set, trunk id:0x0%x failed $rv\n",trunk_id[i]);
                return rv;
            }
            
            BCM_GPORT_TRUNK_SET(trunk_gport[i], trunk_id[i]);
            printf("create trunk id:0x0%x, trunk_gport:0x0%x\n",trunk_id[i],trunk_gport[i]);

             /* Add port member into Trunk */
            sal_memset(&trunk_info[i], 0, sizeof(trunk_info[i]));
            sal_memset(member, 0, sizeof(member));
            trunk_info[i].psc= BCM_TRUNK_PSC_ROUND_ROBIN; 
            if (!i) {
                BCM_GPORT_SYSTEM_PORT_ID_SET(member[0].gport, bfd_in_port[0]);
                BCM_GPORT_SYSTEM_PORT_ID_SET(member[1].gport, bfd_in_port[1]);
            } else {
                BCM_GPORT_SYSTEM_PORT_ID_SET(member[0].gport, bfd_out_port[0]);
                BCM_GPORT_SYSTEM_PORT_ID_SET(member[1].gport, bfd_out_port[1]);
            }            
            rv = bcm_trunk_set(unit, trunk_id[i], &trunk_info[i], member_count, member);
            if (rv != BCM_E_NONE) {
                printf("Error,bcm_trunk_set failed $rv\n");
                return rv;
            }

            printf("configure trunk member for trunk id:0x0%x, trunk_gport:0x0%x\n",trunk_id[i],trunk_gport[i]);
            printf("trunk member gport: 0x0%x, 0x0%x\n",member[0].gport,member[1].gport);

        }
    }
    
    return rv;
}

/* 
 *creating l3 interface - ingress/egress
 */
int bfd_ipv6_intf_init(int unit, int type, int is_trunk){
    int rv;
    int is_adv_vt;
    int in_vlan = 2; 
    int out_vlan = 200;
    int out_vsi = 11;
    int trunk_member_num = 1;
    int i;
    int inner_vlan;
    int profile;
    int action_id;

    if (is_trunk) {
        trunk_member_num = 2;
    }
    
    /*   Double Tagged   */
    int in_inner_vlan = 100; 
    int out_inner_vlan = 400;
    int vrf = 0;    
    int ing_intf_in; 
    int ing_intf_out; 
    int l3_eg_int;   
    int fec; 
    int encap_id; 
    bcm_mac_t mac_address  = {0x00, 0x0c, 0x00, 0x02, 0x00, 0x02};  /* my-MAC */
    bcm_mac_t next_hop_mac  = {0x00, 0x00, 0x00, 0x01, 0x02, 0x03}; /* next_hop_mac1 */
    bcm_vlan_port_t out_vlan_port[trunk_member_num];
    bcm_vlan_action_set_t action;
    
    /*  Advanced VLAN Translation Mode or Not  */
    is_adv_vt = soc_property_get(unit , "bcm886xx_vlan_translate_mode",0);

    for ( i = 0; i < trunk_member_num; i++ ) { 
        rv = bcm_port_class_set(unit, bfd_out_port[i], bcmPortClassId, bfd_out_port[i]);
        if (rv != BCM_E_NONE) {
            printf("fail set port(%d) class\n", bfd_out_port[i]);
          return rv;
        }
        
        bcm_vlan_port_t_init(&out_vlan_port[i]);
        out_vlan_port[i].flags |= (BCM_VLAN_PORT_OUTER_VLAN_PRESERVE | BCM_VLAN_PORT_INNER_VLAN_PRESERVE);
        if (type == 1) {
            out_vlan_port[i].criteria = BCM_VLAN_PORT_MATCH_PORT_VLAN;
        } else if (type == 2) {
            out_vlan_port[i].criteria = BCM_VLAN_PORT_MATCH_PORT_VLAN_STACKED;
            out_vlan_port[i].match_inner_vlan = out_inner_vlan;
            out_vlan_port[i].egress_inner_vlan = 55;
        }
        out_vlan_port[i].vsi = out_vsi;
        out_vlan_port[i].port = bfd_out_port[i];
        out_vlan_port[i].match_vlan = out_vsi;
        out_vlan_port[i].egress_vlan = 33;
        rv =  bcm_vlan_port_create(unit, &out_vlan_port[i]);
        if (rv != BCM_E_NONE) {
            printf("fail create port(%d) vlan(%d)\n", bfd_out_port[i], in_vlan);
          return rv;
        }
    }

    /*** create ingress router interface ***/
    rv = vlan__open_vlan_per_mc(unit, in_vlan, 0x1);  
    if (rv != BCM_E_NONE) {
    	printf("Error, open_vlan=%d, in unit %d \n", in_vlan, unit);
    }

    for ( i = 0; i < trunk_member_num ; i++ ) { 
        rv = bcm_vlan_gport_add(unit, in_vlan, bfd_in_port[i], 0);
        if (rv != BCM_E_NONE && rv != BCM_E_EXISTS) {
            printf("fail add port(0x%08x) to vlan(%d)\n", bfd_in_port[i], in_vlan);
          return rv;
        }
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
    
    rv = vlan__open_vlan_per_mc(unit, out_vlan, 0x1);  
    if (rv != BCM_E_NONE) {
    	printf("Error, open_vlan=%d, in unit %d \n", out_vlan, unit);
    }
    
    for ( i = 0; i < trunk_member_num ; i++ ) { 
        rv = bcm_vlan_gport_add(unit, out_vlan, bfd_out_port[i], 0);
        if (rv != BCM_E_NONE && rv != BCM_E_EXISTS) {
            printf("fail add port(0x%08x) to vlan(%d)\n", bfd_out_port[i], out_vlan);
          return rv;
        }
    }
    
    intf.vsi = out_vsi;
    
    rv = l3__intf_rif__create(unit, &intf);
    ing_intf_out = intf.rif;        
    if (rv != BCM_E_NONE) {
        printf("Error, l3__intf_rif__create\n");
    }

    l3_eg_int = ing_intf_out;

    /*** Create egress object1 ***/
    create_l3_egress_s l3eg;
    l3eg.out_tunnel_or_rif = l3_eg_int;
    sal_memcpy(l3eg.next_hop_mac_addr, next_hop_mac, 6);
    l3eg.vlan   = out_vsi;
    l3eg.arp_encap_id = encap_id;
    l3eg.fec_id = fec; 
    l3eg.allocation_flags = 0; 
    l3eg.out_gport = is_trunk ? trunk_gport[1] : bfd_out_port[0];

    rv = l3__egress__create(unit,&l3eg);
     if (rv != BCM_E_NONE) {
        printf("Error, create egress object, sysport=%d, in unit %d\n", sysport, unit);
    }

    encap_id = l3eg.arp_encap_id;
    fec = l3eg.fec_id;
    bfd_encap_id = encap_id;
    
    rv = add_ipv6_host(unit, &src_ipv6, vrf, fec, bfd_encap_id);
    if (rv != BCM_E_NONE) {
        printf("Error, add ipv6 host, in unit %d\n", unit);
    }

    /* set port translation info*/
    if (is_adv_vt) {
        for ( i = 0; i < trunk_member_num ; i++ ) {
            inner_vlan = (type == 1) ? 0 : out_inner_vlan;
            profile = is_trunk ? g_eve_edit_lag.edit_profile : ((type == 1) ? g_eve_edit_utils.edit_profile: g_eve_edit_double_tagged.edit_profile);
            rv = vlan_port_translation_set(unit, out_vlan, inner_vlan, out_vlan_port[i].vlan_port_id, profile, 0);
            if (rv != BCM_E_NONE) {
                printf("Error, vlan_port_translation_set\n");
                return rv;
            }
        }

        /* Create action IDs */
        action_id = is_trunk ? g_eve_edit_lag.action_id : ((type == 1) ? g_eve_edit_utils.action_id: g_eve_edit_double_tagged.action_id);
        rv = bcm_vlan_translate_action_id_create(unit, BCM_VLAN_ACTION_SET_EGRESS | BCM_VLAN_ACTION_SET_WITH_ID, &action_id);   
        if (rv != BCM_E_NONE) {
            printf("Error, bcm_vlan_translate_action_id_create\n");
            return rv;
        }

        /* Set translation action 1. outer action, set TPID 0x8100. */
        bcm_vlan_action_set_t_init(&action);
        action.dt_outer = bcmVlanActionAdd;
        if (type == 1) {
            action.dt_inner = bcmVlanActionNone;
        } else if (type == 2) {    
            action.dt_inner = bcmVlanActionAdd;
            action.dt_inner_pkt_prio = bcmVlanActionAdd;
            action.inner_tpid_action = bcmVlanTpidActionModify;
            action.outer_tpid_action = bcmVlanTpidActionNone;
            action.inner_tpid = 0x8100;
            action.new_outer_vlan = out_vlan;
            action.new_inner_vlan = out_inner_vlan;
            action.priority = 0;
        }
    
        action.dt_outer_pkt_prio = bcmVlanActionAdd;
        action.outer_tpid = 0x8100;

        rv = bcm_vlan_translate_action_id_set( unit,
                                                   BCM_VLAN_ACTION_SET_EGRESS,
                                                   action_id,
                                                   &action);

        if (rv != BCM_E_NONE) {
            printf("Error, bcm_vlan_translate_action_id_set\n");
            return rv;
        }  


        /* set action class */
        if (!is_trunk) {
            if (type == 1) {
                rv = vlan_default_translate_action_class_set(unit, g_eve_edit_utils.action_id);
                if (rv != BCM_E_NONE) {
                    printf("Error, vlan_default_translate_action_class_set\n");
                    return rv;
                }
            } else if (type == 2) {   
                bcm_vlan_translate_action_class_t action_class;
                bcm_vlan_translate_action_class_t_init(&action_class);
                action_class.flags = BCM_VLAN_ACTION_SET_EGRESS;
                action_class.vlan_edit_class_id = g_eve_edit_double_tagged.edit_profile;
                action_class.vlan_translation_action_id = g_eve_edit_double_tagged.action_id;
                action_class.tag_format_class_id = g_eve_edit_double_tagged.tag_format;
                rv = bcm_vlan_translate_action_class_set(unit, &action_class);
                if (rv != BCM_E_NONE) {
                    printf("Error, vlan_default_translate_action_class_set\n");
                    return rv;
                }
            }
        } else {
            bcm_vlan_translate_action_class_t action_class;
            bcm_vlan_translate_action_class_t_init(&action_class);
              
            action_class.flags = BCM_VLAN_ACTION_SET_EGRESS;
            action_class.vlan_edit_class_id = g_eve_edit_lag.edit_profile;
            action_class.vlan_translation_action_id = g_eve_edit_lag.action_id;
            action_class.tag_format_class_id = g_eve_edit_lag.tag_format;

            rv = bcm_vlan_translate_action_class_set(unit, &action_class);	
            if (rv != BCM_E_NONE) {
                printf("Error, bcm_vlan_translate_action_class_set\n");
                return rv;
            }  
        }
    } else {
        bcm_vlan_action_set_t_init(&action);
        action.ut_outer = bcmVlanActionAdd;
        action.ut_outer_pkt_prio = bcmVlanActionAdd;
        action.new_outer_vlan = out_vlan;
        action.priority = 0;
        
        if (type == 2) {
            action.ut_inner = bcmVlanActionAdd;
            action.ut_inner_pkt_prio = bcmVlanActionAdd;
            action.inner_tpid_action = bcmVlanTpidActionModify;
            action.inner_tpid = 0x8100;
            action.new_inner_vlan = out_inner_vlan;
        }

        for ( i = 0; i < trunk_member_num ; i++ ) {
            rv = bcm_vlan_translate_egress_action_add(unit, out_vlan_port[i].vlan_port_id, BCM_VLAN_NONE, BCM_VLAN_NONE, &action);
            if (rv != BCM_E_NONE) {
                printf("Error, bcm_vlan_translate_egress_action_add in unit %d sys_port %d \n", unit, bfd_out_port[i]);
            }
        }
    }
    
    if(verbose >= 1) {
        printf("created FEC-id =0x%08x, in unit %d \n", fec, unit);
        printf("next hop mac at encap-id %08x, in unit %d\n", bfd_encap_id, unit);
    }
    
    return rv;
}
/* 
 *Set the simple password auth for BFD over IPv6.
 */
int bfd_ipv6_auth_sp_set(int unit) {
    int rv = BCM_E_NONE;
    int i;
    bcm_bfd_auth_simple_password_t auth;

    for (i = 0; i < auth_sp_num; i++) {
        auth.length = auth_sp_len[i];
        sal_memcpy(auth.password, &auth_sp_key[i][0], auth.length);
        
        rv = bcm_bfd_auth_simple_password_set(unit, i, &auth);
        if (BCM_E_NONE != rv) {
    		printf("Error in bcm_bfd_auth_simple_password_set Err %x\n",rv);
            return rv;
    	}
    }

    return rv;
}

/* 
 *Set the sha1 auth for BFD over IPv6.
 */
int bfd_ipv6_auth_sha1_set(int unit) {
    int rv = BCM_E_NONE;
    int i;
    bcm_bfd_auth_sha1_t auth;

    for (i = 0; i < auth_sha1_num; i++) {
        auth.enable = 1;
        auth.sequence = auth_sha1_seq[i];
        sal_memcpy(auth.key, &auth_sha1_key[i][0], 20);        
        
        rv = bcm_bfd_auth_sha1_set(unit, i, &auth);
        if (BCM_E_NONE != rv) {
    		printf("Error in bfd_ipv6_auth_sha1_set Err %x\n",rv);
            return rv;
    	}
    }

    return rv;
}

/* 
 *Set the callback for BFD over IPv6.
 */
int bfd_ipv6_cb(int unit, uint32 flags, bcm_bfd_event_types_t event_types, 
    bcm_bfd_endpoint_t endpoint, void *user_data) {
    int event_i;
    
    printf("endpoint:%d\n", endpoint);
    for (event_i = 0; event_i < bcmBFDEventCount; event_i++) {
        if (BCM_BFD_EVENT_TYPE_GET(event_types, event_i)) {
            bfd_event_count[event_i]++;
            printf("    event:%d\n", event_i);
        }
    }

    return BCM_E_NONE;
}   

/* 
 *Set the event check for BFD over IPv6.
 */
int bfd_ipv6_event_check(int unit, bcm_bfd_event_type_t event, int expeted_value) {
    if (bfd_event_count[event] != expeted_value) {
        printf("Expected_event:%d, Actual event:%d\n", expeted_value, bfd_event_count[event]);
        
		return BCM_E_FAIL;
    } 

    return BCM_E_NONE;
}  

/* 
 *Set the events register for BFD over IPv6.
 */
int bcm_bfd_ipv6_events_register(int unit) {    
    bcm_error_t rv;
    bcm_bfd_event_types_t e;

    BCM_BFD_EVENT_TYPE_SET(e, bcmBFDEventStateChange);
    BCM_BFD_EVENT_TYPE_SET(e, bcmBFDEventRemoteStateDiag);
    BCM_BFD_EVENT_TYPE_SET(e, bcmBFDEventSessionDiscriminatorChange);
    BCM_BFD_EVENT_TYPE_SET(e, bcmBFDEventAuthenticationChange);
    BCM_BFD_EVENT_TYPE_SET(e, bcmBFDEventParameterChange);
    BCM_BFD_EVENT_TYPE_SET(e, bcmBFDEventSessionError);
    BCM_BFD_EVENT_TYPE_SET(e, bcmBFDEventEndpointTimeout);  
    BCM_BFD_EVENT_TYPE_SET(e, bcmBFDEventEndpointRemotePollBitSet);  
    BCM_BFD_EVENT_TYPE_SET(e, bcmBFDEventEndpointRemoteFinalBitSet);  
    rv = bcm_bfd_event_register(unit, e, bfd_ipv6_cb, NULL);
    if (BCM_E_NONE != rv) {
        printf("Error in bcm_bfd_event_register Err (%s) \n",bcm_errmsg(rv));
        return rv;
    }

    return rv;
}

/*
 *Initialize the BFD IPv6 application.
 */
int bfd_ipv6_example_init(int unit, bcm_bfd_auth_type_t auth_type, int type, int is_trunk) {
    int rv;

    if(verbose >= 1) {
        printf("Step: bfd_ipv6_intf_init\n");
    }
    rv = bfd_ipv6_intf_init(unit, type, is_trunk);
    if (rv != BCM_E_NONE) {
        printf("Error in bfd_ipv6_intf_init (%s) \n",bcm_errmsg(rv));
        return rv;
    }

    if (auth_type == bcmBFDAuthTypeSimplePassword) {
        if(verbose >= 1) {
            printf("Step: bfd_ipv6_auth_sp_set\n");
        }
        rv = bfd_ipv6_auth_sp_set(unit);
        if (rv != BCM_E_NONE) {
            printf("Error in bfd_ipv6_auth_sp_set (%s) \n",bcm_errmsg(rv));
            return rv;
        }
    } else if (auth_type == bcmBFDAuthTypeKeyedSHA1) {
        if(verbose >= 1) {
            printf("Step: bfd_ipv6_auth_sha1_set\n");
        }
        rv = bfd_ipv6_auth_sha1_set(unit);
        if (rv != BCM_E_NONE) {
            printf("Error in bfd_ipv6_auth_sha1_set (%s) \n",bcm_errmsg(rv));
            return rv;
        }
    }

    if(verbose >= 1) {
        printf("Step: bcm_bfd_ipv6_events_register\n");
    }
    rv = bcm_bfd_ipv6_events_register(unit);
    if (rv != BCM_E_NONE) {
        printf("Error in bcm_bfd_ipv6_events_register (%s) \n",bcm_errmsg(rv));
        return rv;
    }

    return rv;
}

/*
 *Trap the BFD IPv6.
 *  - ARAD hardware don't support the BFD IPv6.
 *  - Trap the BFD IPv6 single-hop packet by UDP destination port(3784) and IP ttl(255) to uKernel.
 */
int bfd_ipv6_trap(int unit) {
    
    int trap_id=0;
    int rv = BCM_E_NONE;
    bcm_rx_trap_config_t trap_config;
    bcm_field_qset_t qset;
    bcm_field_aset_t aset;
    bcm_field_entry_t ent;
    bcm_field_group_t grp;
    bcm_gport_t gport;
    
    
    /* create trap */
    rv = bcm_rx_trap_type_create(unit, 0, bcmRxTrapUserDefine, &trap_id);
    if (rv != BCM_E_NONE) {
        printf("Error in bcm_rx_trap_type_create (%s) \n",bcm_errmsg(rv));
        return rv;
    }
    
    /* configure trap attribute tot update destination */
    sal_memset(&trap_config, 0, sizeof(trap_config));
    trap_config.flags = (BCM_RX_TRAP_UPDATE_DEST | BCM_RX_TRAP_TRAP | BCM_RX_TRAP_REPLACE);
    trap_config.trap_strength = 0;
    trap_config.dest_port = bfd_dsp_port;
    
    rv = bcm_rx_trap_set(unit, trap_id, &trap_config);
    if (rv != BCM_E_NONE) {
        printf("Error in bcm_rx_trap_set (%s) \n",bcm_errmsg(rv));
        return rv;
    }    
    
    BCM_FIELD_QSET_INIT(qset);
    BCM_FIELD_QSET_ADD(qset, bcmFieldQualifyStageIngress);
    BCM_FIELD_QSET_ADD(qset, bcmFieldQualifyIp6);
    BCM_FIELD_QSET_ADD(qset, bcmFieldQualifyIpProtocol);
    BCM_FIELD_QSET_ADD(qset, bcmFieldQualifyL4DstPort);
    BCM_FIELD_QSET_ADD(qset, bcmFieldQualifyIp6HopLimit);    
    BCM_FIELD_ASET_INIT(aset);
    BCM_FIELD_ASET_ADD(aset, bcmFieldActionTrap);
    
    rv = bcm_field_group_create(unit, qset, 2, &grp);
    if (rv != BCM_E_NONE) {
        printf("Error in bcm_field_group_create (%s) \n",bcm_errmsg(rv));
        return rv;
    }  
    
    rv = bcm_field_group_action_set(unit, grp, aset);
    if (rv != BCM_E_NONE) {
        printf("Error in bcm_field_group_action_set (%s) \n",bcm_errmsg(rv));
        return rv;
    }  
    
    BCM_GPORT_TRAP_SET(gport, trap_id, 7, 0);
    
    rv = bcm_field_entry_create(unit, grp, &ent); 
    if (rv != BCM_E_NONE) {
        printf("Error in bcm_field_entry_create (%s) \n",bcm_errmsg(rv));
        return rv;
    }  

    /* IP protocal: 17(UDP) */
    rv = bcm_field_qualify_IpProtocol(unit, ent, 0x11, 0xff);
    if (rv != BCM_E_NONE) {
        printf("Error in bcm_field_qualify_IpProtocol (%s) \n",bcm_errmsg(rv));
        return rv;
    }
    /* UDP destionation port: 3784(1-hop) */
    rv = bcm_field_qualify_L4DstPort(unit, ent, 3784, 0xffff);
    if (rv != BCM_E_NONE) {
        printf("Error in bcm_field_qualify_L4DstPort (%s) \n",bcm_errmsg(rv));
        return rv;
    } 
    /* IP TTL: 255(1-hop) */
    rv = bcm_field_qualify_Ip6HopLimit(unit, ent, 255, 0xff);
    if (rv != BCM_E_NONE) {
        printf("Error in bcm_field_qualify_Ip6HopLimit (%s) \n",bcm_errmsg(rv));
        return rv;
    }  
    
    rv = bcm_field_action_add(unit, ent, bcmFieldActionTrap, gport, 0);
    if (rv != BCM_E_NONE) {
        printf("Error in bcm_field_action_add (%s) \n",bcm_errmsg(rv));
        return rv;
    }  
    
    rv = bcm_field_entry_install(unit, ent);
    if (rv != BCM_E_NONE) {
        printf("Error in bcm_field_entry_install (%s) \n",bcm_errmsg(rv));
        return rv;
    }  

    return rv;    
}

/**
 * Enable punt packets. 
 *  
 *  - BFD remote state change will PUNT the BFD packet.
 *  - The BFD PUNT destination can be configured by this function.
 */
int bfd_ipv6_run_with_punt(int unit, int dest_port) {
    bcm_rx_trap_config_t trap_config_protection;
    int trap_code=0x480; /* trap code on FHEI  will be (trap_code & 0xff), in this case 1.*/
    /* valid trap codes for oamp traps are 0x401 - 0x4ff */
    int rv;

    rv = bcm_rx_trap_type_create(unit, BCM_RX_TRAP_WITH_ID, bcmRxTrapOampRmepStateChange, &trap_code);
    if (rv != BCM_E_NONE) {
       printf("trap create: (%s) \n",bcm_errmsg(rv));
       return rv;
    }

    rv = port_to_system_port(unit, dest_port, &trap_config_protection.dest_port);
    if (rv != BCM_E_NONE){
        printf("Error, in port_to_system_port\n");
        return rv;
    }

    rv = bcm_rx_trap_set(unit, trap_code, trap_config_protection); 
    if (rv != BCM_E_NONE) {
       printf("trap set: (%s) \n",bcm_errmsg(rv));
       return rv;
    }

    return rv;
}

/*
 *BFD IPv6 test example.
 *  - Test the BFD IPv6 APIs whether works which include bcm_bfd_endpoint_create, 
 *     bcm_bfd_endpoint_get and bcm_bfd_endpoint_destroy.
 *  - Create the BFD IPv6 endpoint that will send the BFD IPv6 packet to remote BFD IPv6 
 *     device and receive BFD IPv6 packet from remote BFD IPv6 device.
 */
int bfd_ipv6_run_with_defaults(int unit, int endpoint_id, int is_acc, int is_single_hop, bcm_bfd_auth_type_t auth_type, int auth_index, int type, int is_trunk, int punt_rate) {
    int rv;
    bcm_bfd_endpoint_info_t bfd_endpoint_info = {0};
    bcm_bfd_endpoint_info_t bfd_endpoint_test_info = {0};
    int ret;

    if (!is_device_or_above(unit,ARAD_PLUS)) {
        printf("Error BFD IPv6 only support on arad+ / jericho.\n");
        return BCM_E_INTERNAL;
    }    

    rv = bfd_ipv6_example_init(unit, auth_type, type, is_trunk);
    if (rv != BCM_E_NONE) {
        printf("Error in bfd_ipv6_example_init (%s) \n",bcm_errmsg(rv));
        return rv;
    }

    if (verbose >= 1) {
        printf("Step: bcm_bfd_endpoint_create \n");
    }

    /* Adding BFDoIPV6 one hop endpoint */
    bcm_bfd_endpoint_info_t_init(&bfd_endpoint_info);

    bfd_endpoint_info.flags = BCM_BFD_ENDPOINT_WITH_ID | BCM_BFD_ENDPOINT_IPV6;
    if (!is_single_hop) {
        bfd_endpoint_info.flags |= BCM_BFD_ENDPOINT_MULTIHOP;
    }
    if (is_acc) {
        bfd_endpoint_info.flags |= BCM_BFD_ENDPOINT_IN_HW;
    }
    
    if (is_trunk) {
        bfd_endpoint_info.tx_gport = trunk_gport[1];
    } else {
        bcm_stk_sysport_gport_get(unit,bfd_out_port[0], &bfd_endpoint_info.tx_gport);
    }

    bfd_endpoint_info.id = endpoint_id;
    bfd_endpoint_info.type = bcmBFDTunnelTypeUdp;
    bfd_endpoint_info.ip_ttl = 255;
    bfd_endpoint_info.ip_tos = 0;
    sal_memcpy(&(bfd_endpoint_info.src_ip6_addr), &src_ipv6, 16);
    sal_memcpy(&(bfd_endpoint_info.dst_ip6_addr), &dst_ipv6, 16);
    bfd_endpoint_info.udp_src_port = 50000;
    bfd_endpoint_info.auth_index = 0;
    bfd_endpoint_info.auth = bcmBFDAuthTypeNone;
    bfd_endpoint_info.local_discr = local_disc;
	bfd_endpoint_info.local_state = bcmBFDStateDown;
    bfd_endpoint_info.local_min_tx = 100000;
    bfd_endpoint_info.local_min_rx = 100000;
    bfd_endpoint_info.local_detect_mult = 3;
    bfd_endpoint_info.remote_discr = remote_disc;
    bfd_endpoint_info.egress_if = bfd_encap_id;
    bfd_endpoint_info.auth = auth_type;
    if (auth_type != bcmBFDAuthTypeNone) {
        bfd_endpoint_info.auth_index = auth_index;
    }  
    if (punt_rate) {
        bfd_endpoint_info.sampling_ratio = punt_rate;
    }
    
    if (bfd_remote_port != BCM_GPORT_INVALID) {
        /* remote_gport field must be a trap */
        int trap_code;
        bcm_rx_trap_config_t trap_cpu;
        rv =  bcm_rx_trap_type_create(unit, 0, bcmRxTrapUserDefine, &trap_code);
        if (rv != BCM_E_NONE) {
           printf("Error in bcm_rx_trap_type_create (%s) \n",bcm_errmsg(rv));
           return rv;
        }
        bcm_rx_trap_config_t_init(&trap_cpu);
        trap_cpu.flags = BCM_RX_TRAP_UPDATE_DEST | BCM_RX_TRAP_TRAP;
        trap_cpu.dest_port = bfd_remote_port;
        rv = bcm_rx_trap_set(unit, trap_code, &trap_cpu);
        if (rv != BCM_E_NONE) {
            printf("Error in bcm_rx_trap_set (%s) \n",bcm_errmsg(rv));
            return rv;
        }
        printf("Trap created trap_code=%d \n",trap_code);
        user_defined_trap_code = trap_code;
        BCM_GPORT_TRAP_SET(bfd_endpoint_info.remote_gport, trap_code, 7, 0); /*Taken from default values*/
    }

    rv = bcm_bfd_endpoint_create(unit, &bfd_endpoint_info);
    if (rv != BCM_E_NONE) {
        printf("Error in bcm_bfd_endpoint_create (%s) \n",bcm_errmsg(rv));
        return rv;
    }
	
	if (verbose >= 1) {
        printf("Step: bfd endport id:0x%x\n", bfd_endpoint_info.id);
    }

    if (verbose >= 1) {
        printf("Step: bfd endport get \n");
    }

    bcm_bfd_endpoint_info_t_init(&bfd_endpoint_test_info);
    rv = bcm_bfd_endpoint_get(unit, bfd_endpoint_info.id, &bfd_endpoint_test_info);
    if (rv != BCM_E_NONE) {
      printf("Error in bcm_bfd_endpoint_get (%s) \n",bcm_errmsg(rv));
      return rv;
    }

    if (verbose >= 1) {
        printf("Step: bfd endport compare \n");
    }

    /* Here mep_not_acc_info and mep_not_acc_test_info are compared */
    ret = cmp_structs(&bfd_endpoint_info, &bfd_endpoint_test_info,bcmBFDTunnelTypeUdp ) ;
    if (ret != 0) {
        printf("%d different fields in BFDoIPV4\n", ret);
    }

    if (verbose >= 1) {
        printf("Step: bfd endport destroy \n");
    }

    rv = bcm_bfd_endpoint_destroy(unit, bfd_endpoint_info.id);
    if (rv != BCM_E_NONE) {
        printf("Error in bcm_bfd_endpoint_destroy (%s) \n",bcm_errmsg(rv));
        return rv;
    }

    if (verbose >= 1) {
        printf("Step: bfd endport re-create \n");
    }

    rv = bcm_bfd_endpoint_create(unit, &bfd_endpoint_info);
    if (rv != BCM_E_NONE) {
        printf("Error in bcm_bfd_endpoint_create (%s) \n",bcm_errmsg(rv));
        return rv;
	}

    return rv;
}

/*
 *BFD IPv6 test example.
 *  - Test the BFD IPv6 APIs whether works which include bcm_bfd_endpoint_create, 
 *     bcm_bfd_endpoint_get and bcm_bfd_endpoint_destroy.
 *  - Create the BFD IPv6 endpoint that will send the BFD IPv6 packet to remote BFD IPv6 
 *     device and receive BFD IPv6 packet from remote BFD IPv6 device.
 */
int bfd_ipv6_run_with_defaults_cleanup(int unit) {
    int rv;

    if (verbose >= 1) {
        printf("Step: bcm_bfd_endpoint_destroy_all \n");
    }

    if (bfd_remote_port != BCM_GPORT_INVALID) {
         bcm_rx_trap_type_destroy(unit, user_defined_trap_code);
    } 

    rv = bcm_bfd_endpoint_destroy_all(unit);
    if (rv != BCM_E_NONE) {
        printf("Error in bcm_bfd_endpoint_destroy (%s) \n",bcm_errmsg(rv));
        return rv;
    }

    if (verbose >= 1) {
        printf("Step: bcm_bfd_detach \n");
    }

    rv = bcm_bfd_detach(unit);
    if (rv != BCM_E_NONE) {
        printf("Error in bcm_bfd_detach (%s) \n",bcm_errmsg(rv));
        return rv;
    }

    return rv;
}

