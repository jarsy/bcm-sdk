/* $Id: cint_utils_l3.c,v 1.10 2013/02/03 10:59:10    Mark Exp $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * This file provides L3 basic functionality and defines L3 global variables
 */

/* **************************************************************************************************
  --------------          Global Variables Definition and Initialization            -----------------
 *************************************************************************************************** */

/* Struct definitions */
struct l3_ipv4_route_entry_utils_s
{
    uint32 address;
    uint32 mask;
    bcm_ip6_t address6;
    bcm_ip6_t mask6;
    uint8 use_ipv6;
};

l3_ipv4_route_entry_utils_s g_l3_route_entry_utils    = { 0x01010100, /* address */
                                                          0xffffffff /*  mask    */ };



 /* ************************************************************************************************** */

/* Delete all l3 Interfaces*/
int l3__intf__delete_all(int unit){
    int rc;

    rc=bcm_l3_intf_delete_all(unit);

    if (rc != BCM_E_NONE) {
        printf("Error, bcm_l3_intf_delete_all\n"); 
    }

    return rc;
}

/* Delete l3 Interface*/
int l3__intf__delete(int unit, int intf)
{
    int rc;
    bcm_l3_intf_t l3if;

    bcm_l3_intf_t_init(l3if);

    l3if.l3a_intf_id = intf;

    rc = bcm_l3_intf_delete(unit, l3if);
    if (rc != BCM_E_NONE) {
        printf("Error, bcm_l3_intf_delete\n");
    }

    return rc;
}

/* Creating L3 interface */
struct create_l3_intf_s {
    /* Input */
    uint32 flags; /* BCM_L3_XXX */
    uint32 ingress_flags; /* BCM_L3_INGRESS_XXX */
    int no_publc; /* Used to force no public, public is forced if vrf = 0 or scale feature is turned on */
    int vsi;    
    bcm_mac_t my_global_mac;
    bcm_mac_t my_lsb_mac;    
    int vrf_valid; /* Do we need to set vrf */
    int vrf; 
    int rpf_valid; /* Do we need to set rpf */
    bcm_l3_ingress_urpf_mode_t urpf_mode; /* avail. when BCM_L3_RPF is set */
	int mtu_valid;
    int mtu; 
    int mtu_forwarding;
	int qos_map_valid;
	int qos_map_id;
	int ttl_valid;
	int ttl;
    uint8 native_routing_vlan_tags;

    /* Output */
    int rif;
    uint8 skip_mymac;   /* If true, mymac will not be set. Make sure you set it elsewhere. */
};

/*
 * create l3 interface - ingress/egress 
 *  Creates Router interface 
 *  - packets sent in from this interface identified by <port, vlan> with specificed MAC address is subject of routing
 *  - packets sent out through this interface will be encapsulated with <vlan, mac_addr>
 *  Parmeters:
 *  - flags: special controls set to zero.
 *  - open_vlan - if TRUE create given vlan, FALSE: vlan already opened juts use it
 *  - port - where interface is defined
 *  - vlan - router interface vlan
 *  - vrf - VRF to map to.
 *  - mac_addr - my MAC
 *  - intf - returned handle of opened l3-interface
 *  
 *  
 */
int l3__intf_rif__create(int unit, create_l3_intf_s *l3_intf){
    bcm_l3_intf_t l3if, l3if_old;
    int rc, station_id;
	bcm_l2_station_t station;
    bcm_l3_ingress_t l3_ing_if;
	int is_urpf_mode_per_rif = soc_property_get(unit , "bcm886xx_l3_ingress_urpf_enable",0);
    int enable_public = 0;

    /* Initialize a bcm_l3_intf_t structure. */
    bcm_l3_intf_t_init(&l3if);
    bcm_l3_intf_t_init(&l3if_old);
    bcm_l2_station_t_init(&station); 
    bcm_l3_ingress_t_init(&l3_ing_if);

    if (!l3_intf->skip_mymac) {
	    /* set my-Mac global MSB */
        station.flags = 0;
        sal_memcpy(station.dst_mac, l3_intf->my_global_mac, 6);
        station.src_port_mask = 0; /* port is not valid */
        station.vlan_mask = 0; /* vsi is not valid */
        station.dst_mac_mask[0] = 0xff; /* dst_mac my-Mac MSB mask is -1 */
        station.dst_mac_mask[1] = 0xff;
        station.dst_mac_mask[2] = 0xff;
        station.dst_mac_mask[3] = 0xff;
        station.dst_mac_mask[4] = 0xff;
        station.dst_mac_mask[5] = 0xff;

        rc = bcm_l2_station_add(unit, &station_id, &station);
        if (rc != BCM_E_NONE) {
            printf("Error, in bcm_l2_station_add\n");
            return rc;
        }
    }

    l3if.l3a_flags = BCM_L3_WITH_ID; /* WITH-ID or without-ID does not really matter. Anyway for now RIF equal VSI */
    if ((l3_intf->no_publc == 0) &&
        (/* Update the in_rif to have public searches enabled for vrf == 0 */
         (l3_intf->vrf == 0 && is_device_or_above(unit,JERICHO)) ||
         /* This feature changes the default non-public non-RPF program behavior, change in_rifs to use the public program instead */
         ((soc_property_get(unit, spn_ENHANCED_FIB_SCALE_PREFIX_LENGTH_IPV6_LONG, 0)) ||
          (soc_property_get(unit, spn_ENHANCED_FIB_SCALE_PREFIX_LENGTH_IPV6_SHORT, 0)) ||
          (soc_property_get(unit, spn_ENHANCED_FIB_SCALE_PREFIX_LENGTH, 0))
          )
         )
        )
    {
        l3_intf->vrf_valid = 1;
        enable_public = 1;
    }

    l3if.l3a_vid = l3_intf->vsi;
    l3if.l3a_ttl = 31; /* default settings */
	if (l3_intf->ttl_valid) {
        l3if.l3a_ttl = l3_intf->ttl;
    }
    l3if.l3a_mtu = 1524; /* default settings */
	if (l3_intf->mtu_valid) {
        l3if.l3a_mtu = l3_intf->mtu;
        l3if.l3a_mtu_forwarding = l3_intf->mtu_forwarding;
    }
    l3if.native_routing_vlan_tags = l3_intf->native_routing_vlan_tags;
    l3_intf->rif = l3if.l3a_intf_id = l3_intf->vsi; /* In DNX Arch VSI always equal RIF */
    
    sal_memcpy(l3if.l3a_mac_addr, l3_intf->my_lsb_mac, 6);
    sal_memcpy(l3if.l3a_mac_addr, l3_intf->my_global_mac, 4); /* ovewriting 4 MSB bytes with global MAC configuration*/    
    l3if.native_routing_vlan_tags = l3_intf->native_routing_vlan_tags; 


    l3if_old.l3a_intf_id = l3_intf->vsi;
    rc = bcm_l3_intf_get(unit, &l3if_old);
    if (rc == BCM_E_NONE) {
        /* if L3 INTF already exists, replace it*/
        l3if.l3a_flags = l3if.l3a_flags | BCM_L3_REPLACE;     
        printf("intf 0x%x already exist, just replacing: bcm_l3_intf_create\n",l3if_old.l3a_intf_id); 
    }
    
	if (l3_intf->qos_map_valid) {
		l3if.dscp_qos.qos_map_id = l3_intf->qos_map_id;
	}

    rc = bcm_l3_intf_create(unit, l3if);
    if (rc != BCM_E_NONE) {
        printf("Error, bcm_l3_intf_create %d\n",rc); 
        return rc;
    }

    if (l3_intf->vrf_valid || l3_intf->rpf_valid) {
        l3_ing_if.flags = BCM_L3_INGRESS_WITH_ID; /* must, as we update exist RIF */
        l3_ing_if.vrf = l3_intf->vrf;

        /* set RIF enable RPF*/
		/* In Arad+ the urpf mode is per RIF (if the SOC property bcm886XX_l3_ingress_urpf_enable is set). */
		if (l3_intf->rpf_valid && !is_urpf_mode_per_rif) {
	        /* Set uRPF global configuration */
	        rc = bcm_switch_control_set(unit, bcmSwitchL3UrpfMode, l3_intf->urpf_mode);
	        if (rc != BCM_E_NONE) {
	            return rc;
	        }
		}
        if (l3_intf->flags & BCM_L3_RPF) {
            l3_ing_if.urpf_mode = l3_intf->urpf_mode; 
        } else {
            l3_ing_if.urpf_mode = bcmL3IngressUrpfDisable;
        }
  
        if ((l3_intf->ingress_flags & BCM_L3_INGRESS_GLOBAL_ROUTE) || ((enable_public == 1) && is_device_or_above(unit,JERICHO))) {
            l3_ing_if.flags |= BCM_L3_INGRESS_GLOBAL_ROUTE;
        }
		if (l3_intf->ingress_flags & BCM_L3_INGRESS_DSCP_TRUST) {
            l3_ing_if.flags |= BCM_L3_INGRESS_DSCP_TRUST;
        }

		if (l3_intf->qos_map_valid) {
			l3_ing_if.qos_map_id = l3_intf->qos_map_id;
		}

        rc = bcm_l3_ingress_create(unit, l3_ing_if, l3if.l3a_intf_id);
        if (rc != BCM_E_NONE) {
            printf("Error, bcm_l3_ingress_create\n"); 
            return rc;
        }
		l3_intf->rif = l3if.l3a_intf_id;
    }

 	printf("created ingress interface = 0x%08x, on vlan = %d in unit %d, vrf = %d\n",l3_intf->rif,l3_intf->vsi,unit,l3_intf->vrf);
    printf("mac-address: %02x:%02x:%02x:%02x:%02x:%02x\n\r",l3_intf->my_global_mac[0],l3_intf->my_global_mac[1],l3_intf->my_global_mac[2],
		   l3_intf->my_global_mac[3],l3_intf->my_global_mac[4],l3_intf->my_global_mac[5]);

    return rc;
}

struct create_l3_egress_with_mpls_s {

    uint32 allocation_flags; /* BCM_L3_XXX */
    uint32 l3_flags; /* BCM_L3_XXX */    
    uint32 l3_flags2; /* BCM_L3_FLAGS2_XXX */

    /* ARP */
    int vlan; /* Outgoing vlan-VSI, relevant for ARP creation. In case set then SA MAC address is retreived from this VSI. */
    bcm_mac_t next_hop_mac_addr; /* Next-hop MAC address, relevant for ARP creation */
    int qos_map_id;                     /* General QOS map id */
    
    /* FEC */
    bcm_if_t out_tunnel_or_rif; /* *Outgoing intf, can be tunnel/rif, relevant for FEC creation */
    bcm_gport_t out_gport; /* *Outgoing port , relevant for FEC creation */
    bcm_failover_t failover_id;         /* Failover Object Index. */
    bcm_if_t failover_if_id;            /* Failover Egress Object index. */

    uint32 mpls_flags;
    bcm_mpls_label_t mpls_label;
    bcm_mpls_egress_action_t mpls_action;
    int mpls_ttl;
    int mpls_exp;

    /* Input/Output ID allocation */
    bcm_if_t fec_id; /* *FEC ID */
    bcm_if_t arp_encap_id; /* *ARP ID, may need for allocation ID or for FEC creation */
}

/* Creating L3 egress */
struct create_l3_egress_s {
    /* Input */
    uint32 allocation_flags; /* BCM_L3_XXX */
    uint32 l3_flags; /* BCM_L3_XXX */    
    uint32 l3_flags2; /* BCM_L3_FLAGS2_XXX */    

    /* ARP */
    int vlan; /* Outgoing vlan-VSI, relevant for ARP creation. In case set then SA MAC address is retreived from this VSI. */
    bcm_mac_t next_hop_mac_addr; /* Next-hop MAC address, relevant for ARP creation */
    int qos_map_id;                     /* General QOS map id */
    
    /* FEC */
    bcm_if_t out_tunnel_or_rif; /* *Outgoing intf, can be tunnel/rif, relevant for FEC creation */
    bcm_gport_t out_gport; /* *Outgoing port , relevant for FEC creation */
    bcm_failover_t failover_id;         /* Failover Object Index. */
    bcm_if_t failover_if_id;            /* Failover Egress Object index. */


    /* Input/Output ID allocation */
    bcm_if_t fec_id; /* *FEC ID */
    bcm_if_t arp_encap_id; /* *ARP ID, may need for allocation ID or for FEC creation */
};



/* Create egress object(FEC and ARP entry)*/
/*int l3__egress__create(int unit, uint32 flags,  int out_port, int vlan, int l3_eg_intf, bcm_mac_t next_hop_mac_addr, int *intf, int *encap_id){*/
int l3__egress__create(int unit, create_l3_egress_s *l3_egress) {
    int rc;
    bcm_l3_egress_t l3eg;
    bcm_if_t l3egid;
    bcm_l3_egress_t_init(&l3eg);

    /* FEC properties */
    l3eg.intf           = l3_egress->out_tunnel_or_rif;
    l3eg.port           = l3_egress->out_gport;  
    l3eg.encap_id       = l3_egress->arp_encap_id;
    /* FEC properties - protection */
    l3eg.failover_id    = l3_egress->failover_id;
    l3eg.failover_if_id = l3_egress->failover_if_id;
                    
    /* ARP */       
    l3eg.mac_addr   = l3_egress->next_hop_mac_addr;
    l3eg.vlan       = l3_egress->vlan;  
    l3eg.qos_map_id = l3_egress->qos_map_id;
                              
    l3eg.flags    = l3_egress->l3_flags ;
    l3eg.flags2   = l3_egress->l3_flags2;
    l3egid        = l3_egress->fec_id; 
    
    rc = bcm_l3_egress_create(unit, l3_egress->allocation_flags, &l3eg, &l3egid);
    if (rc != BCM_E_NONE) {
        printf("Error, create egress object, unit=%d, \n", unit);
        return rc;
    }
  
    l3_egress->fec_id = l3egid;
    l3_egress->arp_encap_id = l3eg.encap_id;
  
    if(verbose >= 1) {
        printf("unit %d: created FEC-id =0x%08x, ", unit, l3_egress->fec_id);        
        printf("encap-id = %08x", l3_egress->arp_encap_id );
    }

    if(verbose >= 2) {
        printf("outRIF = 0x%08x out-port =%d", l3_egress->out_tunnel_or_rif, l3_egress->out_gport);
    }

    if(verbose >= 1) {
        printf("\n");
    }

    return rc;
}

/* Set a FEC entry, without allocating ARP entry */
int l3__egress_only_fec__create(int unit, create_l3_egress_s *l3_egress) {
    int rc;
    bcm_l3_egress_t l3eg;
    bcm_if_t l3egid;
    bcm_l3_egress_t_init(&l3eg);

    /* FEC properties */
    l3eg.intf           = l3_egress->out_tunnel_or_rif;
    l3eg.port           = l3_egress->out_gport;  
    l3eg.encap_id       = l3_egress->arp_encap_id;
    l3eg.vlan           = l3_egress->vlan;
    /* FEC properties - protection */
    l3eg.failover_id    = l3_egress->failover_id;
    l3eg.failover_if_id = l3_egress->failover_if_id;
                              
    l3eg.flags    = l3_egress->l3_flags ;
    l3eg.flags2   = l3_egress->l3_flags2;
    l3egid        = l3_egress->fec_id;      
    
    rc = bcm_l3_egress_create(unit, BCM_L3_INGRESS_ONLY | l3_egress->allocation_flags, &l3eg, &l3egid);
    if (rc != BCM_E_NONE) {
        printf("Error, create egress object, unit=%d, \n", unit);
        return rc;
    }
  
    l3_egress->fec_id = l3egid;  
  
    if(verbose >= 1) {
        printf("unit %d: created FEC-id =0x%08x, ", unit, l3_egress->fec_id);              
    }

    if(verbose >= 2) {
        printf("outRIF = 0x%08x, encap = 0x%08x, out-port =%d", l3_egress->out_tunnel_or_rif,  l3_egress->arp_encap_id, l3_egress->out_gport);
    }

    if(verbose >= 1) {

        printf("\n");
    }

    return rc;
}

/* Set an ARP entry, without allocating FEC entry*/
/*int l3__egress_only_encap__create(int unit, int out_port, int vlan, int l3_eg_intf, bcm_mac_t next_hop_mac_addr, int *encap_id){*/
int l3__egress_only_encap__create(int unit, create_l3_egress_s *l3_egress) {

    int rc;
    bcm_l3_egress_t l3eg;
    bcm_if_t l3egid_null; /* not intersting */

    bcm_l3_egress_t_init(&l3eg);
                          
    l3eg.flags       = l3_egress->l3_flags;
    l3eg.flags2      = l3_egress->l3_flags2;
    l3eg.mac_addr    = l3_egress->next_hop_mac_addr; 
    l3eg.vlan        = l3_egress->vlan;      
    l3eg.encap_id    = l3_egress->arp_encap_id;
    l3eg.intf        = l3_egress->out_tunnel_or_rif;
    l3eg.qos_map_id  = l3_egress->qos_map_id;
    
    rc = bcm_l3_egress_create(unit, BCM_L3_EGRESS_ONLY | l3_egress->allocation_flags, &l3eg, &l3egid_null);

    if (rc != BCM_E_NONE) {
        printf("Error, create egress object, \n");
        return rc;
    }
  
    l3_egress->arp_encap_id = l3eg.encap_id;

    if(verbose >= 1) {
        printf("encap-id = %08x\n", l3_egress->arp_encap_id);
    }

    return rc;
}

/* Creating EEI with MPLS Label and Push Command
 * No Out-Lif created. 
**/
int l3_egress__mpls_push_command__create(int unit, create_l3_egress_with_mpls_s *l3_egress) {
    int rc;
    bcm_l3_egress_t l3eg;
    bcm_if_t l3egid;
    bcm_l3_egress_t_init(&l3eg);

    /* FEC properties */
    l3eg.intf           = l3_egress->out_tunnel_or_rif;
    l3eg.port           = l3_egress->out_gport;  

    /* FEC properties - protection */
    l3eg.failover_id    = l3_egress->failover_id;
    l3eg.failover_if_id = l3_egress->failover_if_id;
                                
    l3eg.flags    = l3_egress->l3_flags;
    l3eg.flags2   = l3_egress->l3_flags2;
    l3egid        = l3_egress->fec_id; 

    l3eg.mpls_flags     = l3_egress->mpls_flags;
    l3eg.mpls_label     = l3_egress->mpls_label;
    l3eg.mpls_action    = l3_egress->mpls_action;
    l3eg.mpls_ttl       = l3_egress->mpls_ttl;
    l3eg.mpls_exp       = l3_egress->mpls_exp;
    
    rc = bcm_l3_egress_create(unit, BCM_L3_INGRESS_ONLY | l3_egress->allocation_flags, &l3eg, &l3egid);
    if (rc != BCM_E_NONE) {
        printf("Error, create egress object, unit=%d, \n", unit);
        return rc;
    }
  
    l3_egress->fec_id = l3egid;
    l3_egress->arp_encap_id = l3eg.encap_id;
  
    if(verbose >= 1) {
        printf("unit %d: created FEC-id =0x%08x, ", unit, l3_egress->fec_id);        
        printf("encap-id = %08x", l3_egress->arp_encap_id );
        printf("INSIDE UTIL\n");
    }

    if(verbose >= 2) {
        printf("outRIF = 0x%08x out-port =%d", l3_egress->out_tunnel_or_rif, l3_egress->out_gport);
    }

    if(verbose >= 1) {
        printf("\n");
    }

    return rc;
}


/* update rpf according to flag
   and update vrf */
int l3__update_rif_vrf_rpf(int unit, bcm_l3_intf_t* intf, int vrf, int rpf_flag) {

    int rv = BCM_E_NONE;
    bcm_l3_ingress_t ingress_intf;
    bcm_l3_intf_t aux_intf;

    aux_intf.l3a_vid = intf->l3a_vid;
    aux_intf.l3a_mac_addr = intf->l3a_mac_addr;
    rv = bcm_l3_intf_find(unit, &aux_intf);
    if (rv != BCM_E_NONE) {
        printf("Error, bcm_l3_intf_find\n");
        return rv;
    }

    bcm_l3_ingress_t_init(&ingress_intf);
    ingress_intf.flags = BCM_L3_INGRESS_WITH_ID;  /* must, as we update exist RIF */


    if (rpf_flag & BCM_L3_RPF) {
      ingress_intf.urpf_mode = L3_uc_rpf_mode; /* RPF mode is loose has to match global configuration or Disabled */
    }
    else{
      ingress_intf.urpf_mode = bcmL3IngressUrpfDisable; 
    }
    ingress_intf.vrf = vrf;

    rv = bcm_l3_ingress_create(unit, &ingress_intf, intf->l3a_intf_id);
    if (rv != BCM_E_NONE) {
        printf("Error, bcm_l3_ingress_create\n");
        return rv;
    }

    return rv;

}


/* add ipv4 route entry to FEC*/
int l3__ipv4_route__add(int unit, l3_ipv4_route_entry_utils_s route_entry, int vrf, int intf) {

    int rc;
    bcm_l3_route_t l3rt;

    bcm_l3_route_t_init(l3rt);

    l3rt.l3a_flags |= BCM_L3_RPF;

    if (route_entry.use_ipv6) {
        sal_memcpy(l3rt.l3a_ip6_mask, route_entry.mask6, sizeof(route_entry.mask6));
        sal_memcpy(l3rt.l3a_ip6_net, route_entry.address6, sizeof(route_entry.address6));
        l3rt.l3a_flags |= BCM_L3_IP6;
    } else {
        l3rt.l3a_subnet = route_entry.address;
        l3rt.l3a_ip_mask = route_entry.mask;
    }
    l3rt.l3a_vrf = vrf;
    l3rt.l3a_intf = intf;
    l3rt.l3a_modid = 0;
    l3rt.l3a_port_tgid = 0;

    rc = bcm_l3_route_add(unit, l3rt);
    if (rc != BCM_E_NONE) {
        printf ("bcm_l3_route_add failed: %x \n", rc);
    }

    if(verbose >= 1) {
        printf("l3__ipv4_route__add address:0x%x, mask:0x%x, vrf:%d ", route_entry.address, route_entry.mask,vrf);
        printf("---> egress-object=0x%08x, \n", intf);
    }

    return rc;
}

/* add ipv4 route entry to host table */
int l3__ipv4_route_to_overlay_host__add(int unit, uint32 ipv4_address, int vrf, int encap_id, int intf, int fec)
{

    int rc;

    bcm_l3_host_t l3_host;      
    bcm_l3_host_t_init(l3_host);  

    /* key of host entry */
    l3_host.l3a_vrf = vrf;               /* router interface */ 
    l3_host.l3a_ip_addr = ipv4_address;  /* ip host entry */

    /* data of host entry */
    l3_host.l3a_port_tgid = fec;         /* overlay tunnel: vxlan gport */
    l3_host.l3a_intf = intf;             /* out rif */
    l3_host.encap_id = encap_id;         /* arp pointer: encap/eei entry */

    rc = bcm_l3_host_add(unit, &l3_host);         
    print l3_host;
    if (rc != BCM_E_NONE) {
      printf("Error, bcm_l3_host_add\n");      
    }

    return rc;
}


/*
 * Add Route default
 *
 */
int l3__ipv6_route__add(int *units_ids, int nof_units) {

  int rc;
  bcm_l3_route_t l3rt;
  bcm_ip6_t addr_int;
  bcm_ip6_t mask_int;
  int unit, i;

  bcm_l3_route_t_init(&l3rt);

  /* UC IPV6 DIP: */
  addr_int[15]= 0; /* LSB */
  addr_int[14]= 0;
  addr_int[13]= 0;
  addr_int[12]= 0;
  addr_int[11]= 0x7;
  addr_int[10]= 0xdb;
  addr_int[9] = 0;
  addr_int[8] = 0;
  addr_int[7] = 0;
  addr_int[6] = 0x70;
  addr_int[5] = 0;
  addr_int[4] = 0x35;
  addr_int[3] = 0;
  addr_int[2] = 0x16;
  addr_int[1] = 0;
  addr_int[0] = 0x1; /* MSB */

  /* UC IPV6 DIP MASK: */
  mask_int[15]= 0xff;
  mask_int[14]= 0xff;
  mask_int[13]= 0xff;
  mask_int[12]= 0xff;
  mask_int[11]= 0xff;
  mask_int[10]= 0xff;
  mask_int[9] = 0xff;
  mask_int[8] = 0xff;
  mask_int[7] = 0;
  mask_int[6] = 0;
  mask_int[5] = 0;
  mask_int[4] = 0;
  mask_int[3] = 0;
  mask_int[2] = 0;
  mask_int[1] = 0;
  mask_int[0] = 0;

  sal_memcpy(&(l3rt.l3a_ip6_net),(addr_int),16);
  sal_memcpy(&(l3rt.l3a_ip6_mask),(mask_int),16);
  l3rt.l3a_vrf = default_vrf;
  l3rt.l3a_intf = ipv6_fap_default_intf;

  l3rt.l3a_flags = BCM_L3_IP6;
  l3rt.l3a_modid = 0;
  l3rt.l3a_port_tgid = 0;

  for (i = 0 ; i < nof_units ; i++){
      unit = units_ids[i];

      rc = bcm_l3_route_add(unit, l3rt);
      if (rc != BCM_E_NONE) {
        printf ("bcm_l3_route_add failed: %d \n", rc);
      }
  }

  return rc;
}

/*
 * get hit bit after running bcm_l3_egress_get
 *
 */
int L3EgressGetHit(int unit, int intf_id,int clear) {

    if((clear != 0) && (clear != 1)){
        return 0;
    }

    int rv;
    bcm_l3_egress_t intf_temp1;
    bcm_l3_egress_t_init(&intf_temp1);

    if(clear == 1){
        intf_temp1.flags = BCM_L3_HIT_CLEAR;
    }

    rv = bcm_l3_egress_get(unit,intf_id,&intf_temp1);

    if(rv != BCM_E_NONE){
        return -1;
    }

    else {
        if ((intf_temp1.flags & BCM_L3_HIT) != 0){
            return 1;
        }
        else {
            return 0;
        }
    }
}
