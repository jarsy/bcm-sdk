/* $Id: cint_ip_route.c,v 1.34 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


/*  Note: this cint also includes tests for 4 label push and protection*/

/* ********* 
  Globals/Aux Variables
 ********** */

/* debug prints */
int verbose = 1;
int L3_uc_rpf_mode = bcmL3IngressUrpfLoose;
/* Should be set if the SOC property bcm886XX_l3_ingress_urpf_enable is set. */
int urpf_mode_per_rif = 0;

/* Used outside basic_example function */
int ip_route_fec = 0;  
int ip_route_fec_with_id = 0;
int ip_route_fec_with_id2 = 0;
int default_intf = 0;    
int default_vrf = 0;

int default_mtu = 0;
int default_mtu_forwarding = 0;


int preserving_dscp=0; /*Preserving DSCP on a per out-port bases*/

int rif = 0; /* getting the out_rif from basic_example_inner*/

/********** 
  functions
 ********** */


/* to add support for double tagged/untagged packets override or replace this function*/
int double_tag_hook(int unit, bcm_gport_t port, int vlan){
	int rc = BCM_E_NONE;

	/* call bcm_port_tpid_add */

	/* call bcm_vlan_port_create once for double tagged packet*/

	/* call bcm_vlan_port_create again for untagged packet*/

	return rc;
}

/************************************
 * Utilities
 *
 ***********************************/

void print_ip_addr(uint32 host)
{
  int a,b,c,d;

  a = (host >> 24) & 0xff;
  b = (host >> 16) & 0xff;
  c = (host >> 8 ) & 0xff;
  d = host & 0xff;
  printf("%d.%d.%d.%d", a,b,c,d);
}


void l2_print_mac(bcm_mac_t mac_address){
    unsigned int a,b,c,d,e,f;
    a = 0xff & mac_address[0];
    b = 0xff & mac_address[1];
    c = 0xff & mac_address[2];
    d = 0xff & mac_address[3];
    e = 0xff & mac_address[4];
    f = 0xff & mac_address[5];
    printf("%02x:%02x:%02x:%02x:%02x:%02x |",
           a,b,c,
           d,e,f);
}

void print_host(char *type, uint32 host, int vrf)
{
  printf("%s  vrf:%d   ", type, vrf);
  if(verbose >= 1) {
      print_ip_addr(host);
  }
}

void print_route(char *type, uint32 host, uint32 mask, int vrf)
{
    printf("%s  vrf:%d   ", type, vrf);
    print_ip_addr(host);
    printf("/");
    print_ip_addr(mask);
}


/*
 * Add Route <vrf, addr, mask> --> intf
 * - addr: IP address 32 bit value
 * - mask 1: to consider 0: to ingore, for example for /24 mask = 0xffffff00
 * - vrf: max value depeneds on soc-property in arad.soc
 * - intf: egress object created using create_l3_egress
 *
 */
int add_route_inner(int unit, uint32 addr, uint32 mask, int vrf, int intf, int use_scale_route) {
  int rc;
  bcm_l3_route_t l3rt;

  bcm_l3_route_t_init(l3rt);

  if (lpm_raw_data_enabled) {
      l3rt.l3a_flags2 = BCM_L3_FLAGS2_RAW_ENTRY;
  } else {
      l3rt.l3a_flags = BCM_L3_RPF;
  }
  if(use_scale_route && soc_property_get(unit,"enhanced_fib_scale_prefix_length", 0)) {
      l3rt.l3a_flags2 |= BCM_L3_FLAGS2_SCALE_ROUTE;
  }
  l3rt.l3a_subnet = addr;
  l3rt.l3a_ip_mask = mask;
  l3rt.l3a_vrf = vrf;
  if (lpm_raw_data_enabled) {
      l3rt.l3a_intf = lpm_payload;  
  } else {
      l3rt.l3a_intf = intf;  
  }
  l3rt.l3a_modid = 0;
  l3rt.l3a_port_tgid = 0;

  rc = bcm_l3_route_add(unit, l3rt);
  if (rc != BCM_E_NONE) {
    printf ("bcm_l3_route_add failed: %x \n", rc);
  }

  if(verbose >= 1) {
      print_route("add route", addr, mask,vrf);
      printf("---> egress-object=0x%08x, \n", intf);
  }

  return rc;
}

int add_route(int unit, uint32 addr, uint32 mask, int vrf, int intf){
    return add_route_inner(unit,addr,mask,vrf,intf,0);
}


/*
 * set default Router destination for given VRF
 * - vrf: max value depeneds on soc-property in arad.soc
 * - is_mc: wether this set the default destination for UC packet or MC packet.
 * - intf: egress object created using create_l3_egress
 * - trap_port: trap destination.
 * 
 * if trap_port is vlid trap then it used for default destination. otherwise intf (egress-object) is used
 *
 */
int set_default_route(int unit, int vrf, uint8 is_mc, int intf, bcm_gport_t trap_port) {
  int rc;
  bcm_l3_route_t l3rt;

  bcm_l3_route_t_init(l3rt);

  l3rt.l3a_flags = 0;

  if(is_mc) {
      l3rt.l3a_flags |= BCM_L3_IPMC;
  }

  /* to indicate it's default route, set subnet to zero */
  l3rt.l3a_subnet = 0; 
  l3rt.l3a_ip_mask = 0;

  l3rt.l3a_vrf = vrf;
  l3rt.l3a_intf = intf;
  l3rt.l3a_port_tgid = trap_port;

  rc = bcm_l3_route_add(unit, l3rt);
  if (rc != BCM_E_NONE) {
    printf ("bcm_l3_route_add failed: %x \n", rc);
  }

  return rc;
}



/*
 * Add Route <vrf, addr, mask> --> intf
 * - addr: IP address 32 bit value
 * - mask 1: to consider 0: to ingore, for example for /24 mask = 0xffffff00
 * - vrf: max value depeneds on soc-property in arad.soc
 * - intf: egress object created using create_l3_egress
 *
 */
int get_route(int unit, uint32 addr, uint32 mask, int vrf, int* intf) {
  int rc;
  int modid;
  bcm_l3_route_t l3rt;

  bcm_l3_route_t_init(l3rt);

  bcm_stk_modid_get(unit, &modid);
  
  l3rt.l3a_flags = BCM_L3_RPF;
  l3rt.l3a_subnet = addr;
  l3rt.l3a_ip_mask = mask;
  l3rt.l3a_vrf = vrf;
  l3rt.l3a_intf = 0;
  l3rt.l3a_modid = modid;
  l3rt.l3a_port_tgid = 0;

  rc = bcm_l3_route_get(unit, l3rt);
  if (rc != BCM_E_NONE) {
    printf ("bcm_l3_route_add failed: %x \n", rc);
  }

  *intf = l3rt.l3a_intf;

  printf ("interface: 0x%08x \n", *intf);

  return rc;
}


/*
 * Add Host <vrf, addr> --> intf
 * - addr: IP address 32 bit value
 * - vrf: max value depeneds on soc-property in arad.soc
 * - intf: egress object created using create_l3_egress
 */
int add_host(int unit, uint32 addr, int vrf, int intf) {
  int rc;
  bcm_l3_host_t l3host;

  bcm_l3_host_t_init(&l3host);

  l3host.l3a_flags = 0;
  l3host.l3a_ip_addr = addr;
  l3host.l3a_vrf = vrf;
  l3host.l3a_intf = intf;
  l3host.l3a_port_tgid = 0;

  rc = bcm_l3_host_add(unit, &l3host);
  if (rc != BCM_E_NONE) {
    printf ("bcm_l3_host_add failed: %x \n", rc);
  }

  if(verbose >= 1) {
	  print_host("add host ", addr,vrf);
      printf("---> egress-object=0x%08x, \n", intf);
  }

  return rc;
}

/* in external tcam mode will add the entry to the LEM */
int add_host_to_local_db(int unit, uint32 addr, int vrf, int intf) {
  int rc;
  bcm_l3_host_t l3host;

  bcm_l3_host_t_init(&l3host);

  l3host.l3a_flags = BCM_L3_HOST_LOCAL;
  l3host.l3a_ip_addr = addr;
  l3host.l3a_vrf = vrf;
  l3host.l3a_intf = intf;
  l3host.l3a_port_tgid = 0;

  rc = bcm_l3_host_add(unit, &l3host);
  if (rc != BCM_E_NONE) {
    printf ("bcm_l3_host_add failed: %x \n", rc);
  }

  if(verbose >= 1) {
      print_host("add host ", addr,vrf);
      printf("---> egress-object=0x%08x, \n", intf);
  }

  return rc;
}


/*
 * Add Host <vrf, addr> --> intf + dest_port
 * - addr: IP address 32 bit value
 * - vrf: max value depeneds on soc-property in arad.soc
 * - intf: egress interface (vlan or tunnel) 
 * - dest_port: destination port to send packet to
 */
int add_host_direct(int unit, uint32 addr, int vrf, int intf, int dest_port) {
  int rc;
  bcm_l3_host_t l3host;

  bcm_l3_host_t_init(l3host);

  l3host.l3a_flags = 0;
  l3host.l3a_ip_addr = addr;
  l3host.l3a_vrf = vrf;
  l3host.l3a_intf = intf;
  l3host.l3a_modid = 0;
  l3host.l3a_port_tgid = dest_port;
  l3host.encap_id = 0;

  rc = bcm_l3_host_add(unit, &l3host);
  if (rc != BCM_E_NONE) {
    printf ("bcm_l3_host_add failed: %x \n", rc);
  }

  if(verbose >= 1) {
	  print_host("add host ", addr,vrf);
      printf("---> egress-intf=0x%08x, port=%d, \n", intf,dest_port);
  }

  return rc;
}

/*
 * delete Host <vrf, addr>
 */
int delete_host(int unit, uint32 addr, int vrf) {
  int rc;
  bcm_l3_host_t l3host;

  bcm_l3_host_t_init(l3host);

  l3host.l3a_flags = 0;
  l3host.l3a_ip_addr = addr;
  l3host.l3a_vrf = vrf;
  l3host.l3a_modid = 0;
  l3host.l3a_port_tgid = 0;

  rc = bcm_l3_host_delete(unit, &l3host);
  if (rc != BCM_E_NONE) {
    printf ("delete_host failed: %x \n", rc);
  }
  return rc;
}

/*
 * delete Host from LEM table in external tcam mode 
 */
int delete_host_from_local_db(int unit, uint32 addr, int vrf) {
  int rc;
  bcm_l3_host_t l3host;

  bcm_l3_host_t_init(l3host);

  l3host.l3a_flags = BCM_L3_HOST_LOCAL;
  l3host.l3a_ip_addr = addr;
  l3host.l3a_vrf = vrf;
  l3host.l3a_modid = 0;
  l3host.l3a_port_tgid = 0;

  rc = bcm_l3_host_delete(unit, &l3host);
  if (rc != BCM_E_NONE) {
    printf ("delete_host failed: %x \n", rc);
  }
  return rc;
}

/*
 * get Host <vrf, addr> to intf mapping
 */
int get_host(int unit, uint32 addr, int vrf) {
  int rc;
  bcm_l3_host_t l3host;

  bcm_l3_host_t_init(l3host);

  l3host.l3a_ip_addr = addr;
  l3host.l3a_vrf = vrf;
  
  rc = bcm_l3_host_find(unit, &l3host);
  if (rc != BCM_E_NONE) {
    printf ("get_host failed: %x \n", rc);
  }
  print l3host;
  return rc;
}

/*
 * get Host from LEM in external tcam mode
 */
int get_host_from_local_db(int unit, uint32 addr, int vrf) {
  int rc;
  bcm_l3_host_t l3host;

  bcm_l3_host_t_init(l3host);

  l3host.l3a_ip_addr = addr;
  l3host.l3a_vrf = vrf;
  l3host.l3a_flags = BCM_L3_HOST_LOCAL;
  
  rc = bcm_l3_host_find(unit, &l3host);
  if (rc != BCM_E_NONE) {
    printf ("get_host failed: %x \n", rc);
  }
  print l3host;
  return rc;
}

/* 
   check if host entry was accessed by traffic lookup
   e.g. host_accessed(0,0x7fffff03,0,0);
*/
int host_accessed(int unit, uint32 addr, int vrf,uint8 clear_accessed ) {
    int rc;
    bcm_l3_host_t l3host;

    bcm_l3_host_t_init(l3host);

    if (clear_accessed) {
        l3host.l3a_flags |= BCM_L3_HIT_CLEAR;
    }

    l3host.l3a_ip_addr = addr;
    l3host.l3a_vrf = vrf;

    rc = bcm_l3_host_find(unit, &l3host);
    if (rc != BCM_E_NONE) {
      printf ("get_host failed: %x \n", rc);
    }
    if (l3host.l3a_flags & BCM_L3_HIT) {
        printf ("Accessed \n");
    }
    else{
        printf ("Not Accessed \n");
    }

    return rc;
}

/* 
   check if FEC entry was accessed by traffic lookup
   e.g. fec_accessed(0,0x7fffff03,0,0);
*/
int fec_accessed(int unit,bcm_if_t intf, uint8 clear_accessed ) {
  int rc;
  bcm_l3_egress_t l3eg;
  bcm_l3_egress_t_init(&l3eg);

  if (clear_accessed) {
      l3eg.flags |= BCM_L3_HIT_CLEAR;
  }

  rc = bcm_l3_egress_get(unit,intf,&l3eg);
  if (rc != BCM_E_NONE) {
    printf ("fec_get failed: %x \n", rc);
  }
  if (l3eg.flags & BCM_L3_HIT) {
      printf ("Accessed \n");
  }
  else{
      printf ("Not Accessed \n");
  }

  return rc;
}

/* delete functions */
int delete_all_hosts(int unit, int intf) {
  int rc;
  bcm_l3_host_t l3host;

  bcm_l3_host_t_init(l3host);

  l3host.l3a_flags = 0;
  l3host.l3a_intf = intf;

  rc = bcm_l3_host_delete_all(unit, l3host);
  if (rc != BCM_E_NONE) {
    printf ("bcm_l3_host_delete_all failed: %x \n", rc);
  }
  return rc;
}

/* add route */
int internal_ip_route(int unit, int route, int mask, int vrf, int fec, int use_scale_route){
	int rv = BCM_E_NONE;
    rv = add_route_inner(unit, route, mask, vrf, fec, use_scale_route);
    if (rv != BCM_E_NONE) {
		printf("Error, add_route\n");
		return rv;
    }
	return rv;
}

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
 *  - vlan 15.
 *  - DA = {0x00, 0x00, 0x00, 0x00, 0xcd, 0x1d}
 *  - SA = {0x00, 0x0c, 0x00, 0x02, 0x00, 0x00}
 *  TTL decremented
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
 *  - DA = {0x20, 0x00, 0x00, 0x00, 0xcd, 0x1d}
 *  - SA = {0x00, 0x0c, 0x00, 0x02, 0x00, 0x00}
 *  TTL decremented
 *  
 */
/* use raw data payload in KAPs LPM */
int lpm_raw_data_enabled =0;
/* raw payload in Kaps*/
int lpm_payload =0x1234;
/*configurable large direct lu key length (14/15/16/17)*/
int large_direct_lu_key_length=0;

int fec_id_used = 0;
int out_rif_used = 0;
int next_hop_used = 0;
int basic_example_inner(int *units_ids, int nof_units, int in_sysport, int out_sysport, int use_scale_route){
    int rv;
    int i, unit;
    int ing_intf_in; 
	int ing_intf_out; 
    int fec[2] = {0x0,0x0};      
    int flags = 0;
    int flags1 = 0;
    int in_vlan = 15; 
    int out_vlan = 100;
    int vrf = 0;
    int host;
    int encap_id[2]={0x0,0x0}; 
    int open_vlan = 1;
    int route;
    int mask;
    int result = 0;
    create_l3_egress_s l3_egress;
    bcm_mac_t mac_address  = { 0x00, 0x0c, 0x00, 0x02, 0x00, 0x00 };  /* my-MAC */
    bcm_mac_t next_hop_mac  = { 0x00, 0x00, 0x00, 0x00, 0xcd, 0x1d }; /* next_hop_mac1 */
    bcm_mac_t next_hop_mac2  = { 0x20, 0x00, 0x00, 0x00, 0xcd, 0x1d }; /* next_hop_mac2 */

    if (lpm_raw_data_enabled) {
        vrf = 1;
        fec[1] = lpm_payload;
    }

    /* In Arad+ the urpf mode is per RIF (if the SOC property bcm886XX_l3_ingress_urpf_enable is set). */
    urpf_mode_per_rif = soc_property_get(unit , "bcm886xx_l3_ingress_urpf_enable",0);

    if (!urpf_mode_per_rif) {
        /* Set uRPF global configuration */
        rv =  bcm_switch_control_set(unit, bcmSwitchL3UrpfMode, L3_uc_rpf_mode);
        if (rv != BCM_E_NONE) {
        return rv;
      }
    }

    /*** create ingress router interface ***/
    flags = 0;
    ing_intf_in = 0;
    units_array_make_local_first(units_ids, nof_units, in_sysport);
    for (i = 0 ; i < nof_units ; i++){
        unit = units_ids[i];
        /*rv = create_l3_intf(unit, flags, open_vlan, in_sysport, in_vlan, vrf, mac_address, &ing_intf_in); */
        rv = vlan__open_vlan_per_mc(unit,in_vlan, 0x1); 
        if (rv != BCM_E_NONE) {
            printf("Error, open_vlan=%d, in unit %d \n", in_vlan, unit);
        }

        rv = bcm_vlan_gport_add(unit, in_vlan, in_sysport, 0);
        if (rv != BCM_E_NONE && rv != BCM_E_EXISTS) {
            printf("fail add port(0x%08x) to vlan(%d)\n", in_sysport, vlan);
          return rv;
        }

        create_l3_intf_s intf;
        intf.vsi = in_vlan;
        intf.my_global_mac = mac_address;
        intf.my_lsb_mac = mac_address;
        intf.vrf_valid = 1;
        intf.vrf = vrf;
        intf.mtu_valid = 1;
        intf.mtu = default_mtu;
        intf.mtu_forwarding = default_mtu_forwarding;
        if (use_scale_route != 0) {
            intf.no_publc = 1;
        }
        if (urpf_mode_per_rif) {
            intf.rpf_valid = 1;
            intf.flags |= BCM_L3_RPF;
            intf.urpf_mode = L3_uc_rpf_mode;
        }
        rv = l3__intf_rif__create(unit, &intf);
        ing_intf_in = intf.rif;
        if (rv != BCM_E_NONE) {
            printf("Error, l3__intf_rif__create\n");
        }
        flags |= BCM_L3_WITH_ID;
    }

    /*** create egress router interface ***/
    ing_intf_out = 0;
    flags = 0;
    units_array_make_local_first(units_ids,nof_units,out_sysport);
    for (i = 0 ; i < nof_units ; i++){
        unit = units_ids[i];
        /*rv = create_l3_intf(unit,flags,open_vlan,out_sysport,out_vlan,vrf,mac_address, &ing_intf_out); */        
        rv = vlan__open_vlan_per_mc(unit, out_vlan, 0x1);  
        if (rv != BCM_E_NONE) {
            printf("Error, open_vlan=%d, in unit %d \n", out_vlan, unit);
        }
        rv = bcm_vlan_gport_add(unit, out_vlan, out_sysport, 0);
        if (rv != BCM_E_NONE && rv != BCM_E_EXISTS) {
            printf("fail add port(0x%08x) to vlan(%d)\n", out_sysport, out_vlan);
          return rv;
        }

        create_l3_intf_s intf;
        intf.vsi = out_vlan;
        intf.my_global_mac = mac_address;
        intf.my_lsb_mac = mac_address;
        intf.vrf_valid = 1;
        intf.vrf = vrf;
        intf.mtu_valid = 1;
        intf.mtu = default_mtu;
        intf.mtu_forwarding = default_mtu_forwarding;
        if (use_scale_route != 0) {
            intf.no_publc = 1;
        }
        if (urpf_mode_per_rif) {
            intf.rpf_valid = 1;
            intf.flags |= BCM_L3_RPF;
            intf.urpf_mode = L3_uc_rpf_mode;
        }
        rv = l3__intf_rif__create(unit, &intf);
        ing_intf_out = out_rif_used = intf.rif;        
        if (rv != BCM_E_NONE) {
            printf("Error, l3__intf_rif__create\n");
        }
        flags |= BCM_L3_WITH_ID;
    }

    /*** create egress object 1 ***/
    /* out_sysport unit is already first */
    l3_egress.allocation_flags = 0;
    for (i = 0 ; i < nof_units ; i++){
        unit = units_ids[i];
        /*rv = create_l3_egress(unit,flags1,out_sysport,out_vlan,ing_intf_out,next_hop_mac, &fec[0], &encap_id[0]); */
        l3_egress.out_gport = out_sysport;
        l3_egress.vlan = out_vlan;
        l3_egress.next_hop_mac_addr = next_hop_mac;
        l3_egress.out_tunnel_or_rif = ing_intf_out;
        l3_egress.arp_encap_id = encap_id[0];
		if (ip_route_fec_with_id != 0) {
			fec[0] = ip_route_fec_with_id;
			l3_egress.allocation_flags |= BCM_L3_WITH_ID;
		}
        l3_egress.fec_id = fec[0];

        rv = l3__egress__create(unit, &l3_egress);
        if (rv != BCM_E_NONE) {
            printf("Error, create egress object, out_sysport=%d, in unit %d\n", out_sysport, unit);
        }
        encap_id[0] = next_hop_used = l3_egress.arp_encap_id;
        ip_route_fec = fec[0] = l3_egress.fec_id;

        if(verbose >= 1) {
            printf("created FEC-id =0x%08x, in unit %d \n", fec[0], unit);
            printf("next hop mac at encap-id %08x, in unit %d\n", encap_id[0], unit);
        }
        l3_egress.allocation_flags |= BCM_L3_WITH_ID;
    }

    /*** add host point to FEC ***/
    if (!(lpm_raw_data_enabled)) {
        host = 0x7fffff03;
        /* Units order does not matter*/
        for (i = 0 ; i < nof_units ; i++){
            unit = units_ids[i];
            rv = add_host(unit, 0x7fffff03, vrf, fec[0]); 
            if (rv != BCM_E_NONE) {
                printf("Error, create egress object, in_sysport=%d, in unit %d \n", in_sysport, unit);
            }
        }
    }
     

    /*** create egress object 2***/
    /* out_sysport unit is already first */
    l3_egress.allocation_flags = 0;
    for (i = 0 ; i < nof_units ; i++){
        unit = units_ids[i];
        /*rv = create_l3_egress(unit,flags1,out_sysport,out_vlan,ing_intf_out,next_hop_mac2, &fec[1], &encap_id[1]); */
        l3_egress.out_gport = out_sysport;
        l3_egress.vlan = out_vlan;
        l3_egress.next_hop_mac_addr = next_hop_mac2;
        l3_egress.out_tunnel_or_rif = ing_intf_out;
        l3_egress.arp_encap_id = encap_id[1];
        if (ip_route_fec_with_id2 != 0) {
			fec[1] = ip_route_fec_with_id2;
			l3_egress.allocation_flags |= BCM_L3_WITH_ID;
		}
        l3_egress.fec_id = fec[1];
        if(lpm_raw_data_enabled) {
            l3_egress.allocation_flags |= BCM_L3_WITH_ID;
        }
        
        rv = l3__egress__create(unit, &l3_egress);
        if (rv != BCM_E_NONE) {
            printf("Error, create egress object, out_sysport=%d, in unit %d\n", out_sysport, unit);
        }
        encap_id[1] = l3_egress.arp_encap_id;
        fec[1] = l3_egress.fec_id;
        fec_id_used = fec[1];
        if(verbose >= 1) {
            printf("created FEC-id =0x%08x, \n in unit %d", fec[1], unit);
            printf("next hop mac at encap-id %08x, in unit %d\n", encap_id[1], unit);
        }
    }

    /*** add route point to FEC2 ***/
    route = 0x7fffff00;
    if(soc_property_get(unit , "enhanced_fib_scale_prefix_length",0)){
        mask  = 0xffff0000;
    }
    else{
        mask  = 0xfffffff0;
    }
    /* Units order does not matter*/
    for (i = 0 ; i < nof_units ; i++){
        unit = units_ids[i];
        printf("fec used is  = 0x%x (%d)\n", fec[1], fec[1]);

		rv = internal_ip_route(unit, route, mask, vrf, fec[1], use_scale_route) ;
		if (rv != BCM_E_NONE) {
			printf("Error, in function internal_ip_route, \n");
			return rv;
		}
        
    }
    /* redirect the packet in the pmf acording to the raw payload received in the kaps*/
    if (lpm_raw_data_enabled) {
        rv = kaps_raw_data_redirect(unit); 
        if (rv != BCM_E_NONE) {
            printf("Error, in function kaps_raw_data_redirect, \n");
            return rv;
        }
    }
    /*Preserving DSCP on a per out-port bases*/
    if (preserving_dscp) {
        rv = bcm_port_control_set(unit, out_sysport, bcmPortControlTCPriority, 1);
        if (rv != BCM_E_NONE) {
            printf("Error, in function bcm_port_control_set, \n");
            return rv;
        }
    }
    rif = out_vlan;
    return rv;
}


int basic_example(int *units_ids, int nof_units, int in_sysport, int out_sysport){
    return basic_example_inner(units_ids,nof_units,in_sysport,out_sysport,0);
}

int add_host_to_lem(int unit)
{
    int rv;

    rv = add_host_to_local_db(unit, 0x7fffff03, 0, fec_id_used);
    if (rv != BCM_E_NONE) {
            printf("Error, in function add_host_to_lem, \n");
            return rv;
    }

return rv;
}
 
/* 
 * set defaul route to send to out-port. 
 * out-port:  egress port for default route. 
 * is_trap: if FALSE default destination is FEC                                                            .
 *          otherwise default is trap-port.
 */
int default_route_config(int unit, int in_port, int out_port, int is_trap){
    int rv;
    int ing_intf_in; 
	int ing_intf_out; 
    int fec[2] = {0x0,0x0};      
    int flags = 0;
    int flags1 = 0;
    int in_vlan = 15; 
	int out_vlan = 101;
    int vrf = 0;
    int encap_id[2]={0x0,0x0}; 
    int open_vlan=1;
    bcm_l3_ingress_t l3_ing_if;
    bcm_mac_t mac_address  = {0x00, 0x0c, 0x00, 0x02, 0x00, 0x00};  /* my-MAC */
    bcm_mac_t next_hop_mac  = {0x00, 0x00, 0x00, 0x00, 0xcd, 0x1d}; /* next_hop_mac1 */
    create_l3_intf_s intf;
    create_l3_egress_s l3_eg;

    /* In Arad+ the urpf mode is per RIF (if the SOC property bcm886XX_l3_ingress_urpf_enable is set). */
    urpf_mode_per_rif = soc_property_get(unit , "bcm886xx_l3_ingress_urpf_enable",0);

    if (!urpf_mode_per_rif) {
      /* Set uRPF global configuration */
      rv =  bcm_switch_control_set(unit, bcmSwitchL3UrpfMode, L3_uc_rpf_mode);
      if (rv != BCM_E_NONE) {
        return rv;
      }
    }
    
    /*** create ingress router interface ***/
    rv = vlan__open_vlan_per_mc(unit,in_vlan, 0x1); 
    if (rv != BCM_E_NONE) {
        printf("Error, open_vlan=%d, in unit %d \n", in_vlan, unit);
    }    
    rv = bcm_vlan_gport_add(unit, in_vlan, in_port, 0);
    if (rv != BCM_E_NONE && rv != BCM_E_EXISTS) {
        printf("fail add port(0x%08x) to vlan(%d)\n", in_port, vlan);
      return rv;
    }
    intf.vsi = in_vlan;
    intf.my_global_mac = mac_address;
    intf.my_lsb_mac = mac_address;
    intf.vrf_valid = 1;
    intf.vrf = vrf;
    if (urpf_mode_per_rif) {
        intf.rpf_valid = 1;
        intf.flags |= BCM_L3_RPF;
        intf.urpf_mode = L3_uc_rpf_mode;
    }
    intf.ingress_flags |= BCM_L3_INGRESS_GLOBAL_ROUTE;    
    rv = l3__intf_rif__create(unit, &intf);
    ing_intf_in = intf.rif;    

    if (rv != BCM_E_NONE) {
        printf("Error, l3__intf_rif__create\n", in_port);
        return rv;
    }    

    /*** get ingress router interface ***/
    rv = bcm_l3_ingress_get(unit, ing_intf_in, &l3_ing_if);
    if (rv != BCM_E_NONE) {
        printf("Error, bcm_l3_ingress_get, ing_intf_in=%d, \n", ing_intf_in);
        return rv;
    }   

    /*** check BCM_L3_INGRESS_GLOBAL_ROUTE is set ***/    
    if ((l3_ing_if.flags & BCM_L3_INGRESS_GLOBAL_ROUTE) == 0) {
        printf("Error, BCM_L3_INGRESS_GLOBAL_ROUTE is not set\n");
        return rv;
    } 	

	/*** create egress router interface ***/
    rv = vlan__open_vlan_per_mc(unit, out_vlan, 0x1);  
    if (rv != BCM_E_NONE) {
        printf("Error, open_vlan=%d, in unit %d \n", out_vlan, unit);
    }    
    rv = bcm_vlan_gport_add(unit, out_vlan, out_port, 0);
    if (rv != BCM_E_NONE && rv != BCM_E_EXISTS) {
        printf("fail add port(0x%08x) to vlan(%d)\n", out_port, vlan);
      return rv;
    }

    intf.vsi = out_vlan;
    intf.my_global_mac = mac_address;
    intf.my_lsb_mac = mac_address;
    intf.vrf_valid = 1;
    intf.vrf = vrf;
    if (urpf_mode_per_rif) {
        intf.rpf_valid = 1;
        intf.flags |= BCM_L3_RPF;
        intf.urpf_mode = L3_uc_rpf_mode;
    }
    intf.ingress_flags |= BCM_L3_INGRESS_GLOBAL_ROUTE;
    rv = l3__intf_rif__create(unit, &intf);
    ing_intf_out = intf.rif;    

	if (rv != BCM_E_NONE) {
		printf("Error, l3__intf_rif__create\n");
        return rv;
	}

    /*** create egress object 1 ***/


    sal_memcpy(l3_eg.next_hop_mac_addr, next_hop_mac, 6);  
    
    l3_eg.out_tunnel_or_rif   = ing_intf_out;
    l3_eg.vlan   = out_vlan;
    l3_eg.out_gport  = out_port;
    l3_eg.allocation_flags = flags1;
    l3_eg.arp_encap_id = encap_id[0]; 
    l3_eg.fec_id = fec[0];
    
    
    rv = l3__egress__create(unit, &l3_eg);
    if (rv != BCM_E_NONE) {
        printf("Error, create egress object, out_port=%d, \n", out_port);
        return rv;
    }

    encap_id[0] = l3_eg.arp_encap_id;
    fec[0] = l3_eg.fec_id;

    if(verbose >= 1) {
      printf("created FEC-id =0x%08x, ", fec[0]);
      printf("encap-id = %08x", encap_id[0]);
    }

    if(verbose >= 2) {
      printf("outRIF = 0x%08x out-port =%d", ing_intf_out, fec[0]);
    }

    /* set default route to point to created FEC */
    rv = set_default_route(unit,vrf,0/*is-MC*/,fec[0],0);
    if (rv != BCM_E_NONE) {
        printf("Error, set_default_route\n");
    }
    printf("default route ---> egress-object=0x%08x, port=%d, \n", fec[1], out_port);

    return rv;
}
 
 
 
/*
 * packet will be routed from in_port to out-port 
 * packet processing doesn't consume FEC entry
 * host lookup returns: <dest-port, out-RIF, MAC (pointed by outlif)
 * HOST: 
 * packet to send: 
 *  - in port = in_port
 *  - vlan 15.
 *  - DA = {0x00, 0x0c, 0x00, 0x02, 0x00, 0x00}
 *  - DIP = 0x0a00ff00 (10.0.255.00)
 * expected: 
 *  - out port = out_port
 *  - vlan 100.
 *  - DA = {0x00, 0x00, 0x00, 0x03, 0x00, 0x00}
 *  - SA = {0x00, 0x0c, 0x00, 0x02, 0x00, 0x00}
 *  TTL decremented
 */
int ip_host_no_fec_example(int unit, int in_port, int out_port){
    int rv;
    int ing_intf_in; 
    int ing_intf_out; 
    int fec[2];
    int encap_id[2];
    int flags = 0;
    int in_vlan = 15; 
    int out_vlan = 100;
    int vrf = 0;
    int host = 0x0a00ff00;
    int open_vlan=1;
    create_l3_intf_s intf;
    bcm_mac_t my_mac_address  = {0x00, 0x0c, 0x00, 0x02, 0x00, 0x00};  /* my-MAC */
    bcm_mac_t next_hop_mac  = {0x00, 0x00, 0x00, 0x03, 0x00, 0x00}; 
    bcm_l3_host_t l3host; 
    create_l3_egress_s l3_eg;

    /* In Arad+ the urpf mode is per RIF (if the SOC property bcm886XX_l3_ingress_urpf_enable is set). */
    urpf_mode_per_rif = soc_property_get(unit , "bcm886xx_l3_ingress_urpf_enable",0);

    if (!urpf_mode_per_rif) {
      /* Set uRPF global configuration */
      rv =  bcm_switch_control_set(unit, bcmSwitchL3UrpfMode, L3_uc_rpf_mode);
      if (rv != BCM_E_NONE) {
        return rv;
      }
    }

    /*** create ingress router interface ***/
    /*rv = create_l3_intf(unit,flags,open_vlan,in_port,in_vlan,vrf,my_mac_address, &ing_intf_in); */
    rv = vlan__open_vlan_per_mc(unit,in_vlan, 0x1); 
    if (rv != BCM_E_NONE) {
        printf("Error, open_vlan=%d, in unit %d \n", out_vlan, unit);
    }    
    rv = bcm_vlan_gport_add(unit, in_vlan, in_port, 0);
    if (rv != BCM_E_NONE && rv != BCM_E_EXISTS) {
        printf("fail add port(0x%08x) to vlan(%d)\n", in_port, vlan);
      return rv;
    }

    intf.vsi = in_vlan;
    intf.my_global_mac = my_mac_address;
    intf.my_lsb_mac = my_mac_address;
    intf.vrf_valid = 1;
    intf.vrf = vrf;
    if (urpf_mode_per_rif) {
        intf.rpf_valid = 1;
        intf.flags |= BCM_L3_RPF;
        intf.urpf_mode = L3_uc_rpf_mode;
    }
    rv = l3__intf_rif__create(unit, &intf);
    ing_intf_in = intf.rif;     

    if (rv != BCM_E_NONE) {
        printf("Error, l3__intf_rif__create\n");
    }

    /*** create egress router interface ***/
    /*rv = create_l3_intf(unit,flags,open_vlan,out_port,out_vlan,vrf,my_mac_address, &ing_intf_out); */
    rv = vlan__open_vlan_per_mc(unit, out_vlan, 0x1);  
    if (rv != BCM_E_NONE) {
        printf("Error, open_vlan=%d, in unit %d \n", out_vlan, unit);
    }
    rv = bcm_vlan_gport_add(unit, out_vlan, out_port, 0);
    if (rv != BCM_E_NONE && rv != BCM_E_EXISTS) {
        printf("fail add port(0x%08x) to vlan(%d)\n", out_port, out_vlan);
      return rv;
    }

    intf.vsi = in_vlan;
    intf.my_global_mac = my_mac_address;
    intf.my_lsb_mac = my_mac_address;
    intf.vrf_valid = 1;
    intf.vrf = vrf;
    if (urpf_mode_per_rif) {
        intf.rpf_valid = 1;
        intf.flags |= BCM_L3_RPF;
        intf.urpf_mode = L3_uc_rpf_mode;
    }
    rv = l3__intf_rif__create(unit, &intf);
    ing_intf_out = intf.rif;     
    if (rv != BCM_E_NONE) {
        printf("Error, l3__intf_rif__create\n");
    }

    /* use egress to set MAC address and outRIF, FEC is not created */

    fec[0] = 0;
    encap_id[0] = 0;
    /*** create egress object 1 ***/
    /* build FEC points to out-RIF and MAC address */

    /* set Flag to indicate not to allocate or update FEC entry */
    sal_memcpy(l3_eg.next_hop_mac_addr, next_hop_mac, 6);  
    
    l3_eg.out_tunnel_or_rif   = ing_intf_out;
    l3_eg.vlan   = out_vlan;
    l3_eg.out_gport  = out_port;
    l3_eg.l3_flags =  flags;
    l3_eg.arp_encap_id = encap_id[0];
    
    
    rv = l3__egress_only_encap__create(unit, &l3_eg);
    if (rv != BCM_E_NONE) {
        printf("Error, create egress object, out_port=%d, \n", out_port);
        return rv;
    }

    encap_id[0] = l3_eg.arp_encap_id;

    if(verbose >= 1) {
      printf("encap-id = %08x", encap_id[0]);
    }

    if(verbose >= 2) {
      printf("outRIF = 0x%08x out-port =%d", ing_intf_out, fec[0]);
    }
    /*** add host entry with OUT-RIF + MAC + eg-port ***/

    bcm_l3_host_t_init(&l3host);
    l3host.l3a_flags = 0;
    l3host.l3a_ip_addr = host;
    l3host.l3a_vrf = vrf;
    l3host.l3a_intf = 0; /* no interface VSI is taken from encap-id */
    l3host.l3a_modid = 0;
    l3host.l3a_port_tgid = out_port; /* egress port to send packet to */

    /* as encap id is valid (!=0), host will point to outRIF + outlif (MAC) + port */
    l3host.encap_id = encap_id[0];

    rv = bcm_l3_host_add(unit, &l3host);
    if (rv != BCM_E_NONE) {
        printf ("bcm_l3_host_add failed: %x \n", rv);
        printf("Error, create egress object, in_port=%d, \n", in_port);
    }
	print_host("add entry ", host,vrf);
    printf("---> egress-encap=0x%08x, port=%d, \n", encap_id[0], out_port);

    return rv;
}
 
 

/*
 * packet will be routed from in_port to out-port 
 * packet processing use FEC for outRIF, 
 * yet the next-hop mac is pointed directly from host lookup
 * host lookup returns: <dest-port, FEC, MAC (pointed by outlif)>
 * FEC returns out-RIF.
  * HOST: 
 * packet to send: 
 *  - in port = in_port
 *  - vlan 15.
 *  - DA = {0x00, 0x0c, 0x00, 0x02, 0x00, 0x00}
 *  - DIP = 0x0a00ff01 (10.0.255.01)
 * expected: 
 *  - out port = out_port
 *  - vlan 100.
 *  - DA = {0x00, 0x00, 0x00, 0x03, 0x00, 0x01}
 *  - SA = {0x00, 0x0c, 0x00, 0x02, 0x00, 0x00}
 *  TTL decremented
 */
 int ip_host_fec_per_outrif(int unit, int in_port, int out_port){
    int rv;
    int ing_intf_in; 
    int ing_intf_out; 
    int fec[2];
    int encap_id[2];
    int flags = 0;
    int in_vlan = 15; 
    int out_vlan = 100;
    int vrf = 0;
    int host = 0x0a00ff01;
    int open_vlan=1;
    bcm_mac_t my_mac_address  = {0x00, 0x0c, 0x00, 0x02, 0x00, 0x00};  /* my-MAC */
    bcm_mac_t next_hop_mac  = {0x00, 0x00, 0x00, 0x03, 0x00, 0x01};
    bcm_l3_host_t l3host;
    create_l3_intf_s intf;
    create_l3_egress_s l3_eg;

    /* In Arad+ the urpf mode is per RIF (if the SOC property bcm886XX_l3_ingress_urpf_enable is set). */
    urpf_mode_per_rif = soc_property_get(unit , "bcm886xx_l3_ingress_urpf_enable",0);

    if (!urpf_mode_per_rif) {
      /* Set uRPF global configuration */
      rv =  bcm_switch_control_set(unit, bcmSwitchL3UrpfMode, L3_uc_rpf_mode);
      if (rv != BCM_E_NONE) {
        return rv;
      }
    }

    /*** create ingress router interface ***/
    /*rv = create_l3_intf(unit,flags,open_vlan,in_port,in_vlan,vrf,my_mac_address, &ing_intf_in); */
    rv = vlan__open_vlan_per_mc(unit,in_vlan, 0x1); 
    if (rv != BCM_E_NONE) {
        printf("Error, open_vlan=%d, in unit %d \n", in_vlan, unit);
    }    
    rv = bcm_vlan_gport_add(unit, in_vlan, in_port, 0);
    if (rv != BCM_E_NONE && rv != BCM_E_EXISTS) {
        printf("fail add port(0x%08x) to vlan(%d)\n", in_port, in_vlan);
      return rv;
    }

    intf.vsi = in_vlan;
    intf.my_global_mac = my_mac_address;
    intf.my_lsb_mac = my_mac_address;
    intf.vrf_valid = 1;
    intf.vrf = vrf;
    if (urpf_mode_per_rif) {
        intf.rpf_valid = 1;
        intf.flags |= BCM_L3_RPF;
        intf.urpf_mode = L3_uc_rpf_mode;
    }
    rv = l3__intf_rif__create(unit, &intf);
    ing_intf_in = intf.rif;     
    if (rv != BCM_E_NONE) {
        printf("Error, l3__intf_rif__create\n");
    }

    /*** create egress router interface ***/
    /*rv = create_l3_intf(unit,flags,open_vlan,out_port,out_vlan,vrf,my_mac_address, &ing_intf_out); */
    rv = vlan__open_vlan_per_mc(unit, out_vlan, 0x1);  
    if (rv != BCM_E_NONE) {
        printf("Error, open_vlan=%d, in unit %d \n", out_vlan, unit);
    }    
    rv = bcm_vlan_gport_add(unit, out_vlan, out_port, 0);
    if (rv != BCM_E_NONE && rv != BCM_E_EXISTS) {
        printf("fail add port(0x%08x) to vlan(%d)\n", out_port, out_vlan);
      return rv;
    }

    intf.vsi = out_vlan;
    intf.my_global_mac = my_mac_address;
    intf.my_lsb_mac = my_mac_address;
    intf.vrf_valid = 1;
    intf.vrf = vrf;
    if (urpf_mode_per_rif) {
        intf.rpf_valid = 1;
        intf.flags |= BCM_L3_RPF;
        intf.urpf_mode = L3_uc_rpf_mode;
    }
    rv = l3__intf_rif__create(unit, &intf);
    ing_intf_out = intf.rif;     

    if (rv != BCM_E_NONE) {
        printf("Error, l3__intf_rif__create\n");
    }

    fec[0] = 0;
    encap_id[0] = 0;
    /*** create egress object 1 ***/
    /* build FEC points to out-RIF and MAC address */
    sal_memcpy(l3_eg.next_hop_mac_addr, next_hop_mac, 6);  
    
    l3_eg.out_tunnel_or_rif   = ing_intf_out;
    l3_eg.vlan   = out_vlan;
    l3_eg.out_gport  = out_port;
    l3_eg.allocation_flags = flags;
    l3_eg.arp_encap_id = encap_id[0]; 
    l3_eg.fec_id = fec[0];
    
    
    rv = l3__egress__create(unit, &l3_eg);
    if (rv != BCM_E_NONE) {
        printf("Error, create egress object, out_port=%d, \n", out_port);
        return rv;
    }

    encap_id[0] = l3_eg.arp_encap_id;
    fec[0] = l3_eg.fec_id;

    if(verbose >= 1) {
      printf("created FEC-id =0x%08x, ", fec[0]);
      printf("encap-id = %08x", encap_id[0]);
    }

    if(verbose >= 2) {
      printf("outRIF = 0x%08x out-port =%d", ing_intf_out, fec[0]);
    }

    /*** add host entry with OUT-RIF + MAC + eg-port ***/
    
	rv = add_host(unit, host, vrf, fec[0]);
	if (rv != BCM_E_NONE) {
        printf("Error, add_host\n");
    }
    print_host("add entry ", host, vrf);
    printf("---> egress-intf=0x%08x, egress-mac: port=%d, \n", ing_intf_out, out_port);

    return rv;
}


/*
 * packet will be routed from in_port to out-port 
 * packet processing use FEC for outRIF, 
 * yet the next-hop mac is pointed directly from host lookup
 * host lookup returns: <dest-port, FEC, MAC (pointed by outlif), MAC-LSB 4b (carried as is)
 * FEC returns out-RIF.
 * HOST: 
 * packet to send: 
 *  - in port = in_port
 *  - vlan 1.
 *  - DA = {0x00, 0x0c, 0x00, 0x02, 0x00, 0x00}
 *  - DIP = 0x0a00ff01 (10.0.255.01)
 * expected: 
 *  - out port = out_port
 *  - vlan 100.
 *  - DA = {0x00, 0x00, 0x00, 0x03, 0x00, 0x00 + mac_lsb}
 *  - SA = {0x00, 0x0c, 0x00, 0x02, 0x00, 0x00}
 *  TTL decremented
 *  Parameters:
 *  - Incoming port (in_port)
 *  - Outgoing port (out_port)
 *  - MAC LSB (mac_lsb). Expected DA will be DA + mac_lsb.
 *  For example: mac_lsb = 1 then DA = {0x00, 0x00, 0x00, 0x03, 0x00, 0x02}
 */
 int ip_host_fec_per_outrif_arp_extension(int unit, int in_port, int out_port, int mac_lsb){
    int rv;
    int ing_intf_in; 
    int ing_intf_out; 
    int fec[2];
    int encap_id[2];
    int flags = 0;
    int in_vlan = 1; 
    int out_vlan = 100;
    int vrf = 0;
    int host = 0x0a00ff01;
    int open_vlan=1;
    bcm_mac_t my_mac_address  = {0x00, 0x0c, 0x00, 0x02, 0x00, 0x00};  /* my-MAC */
    bcm_mac_t next_hop_mac  = {0x00, 0x00, 0x00, 0x03, 0x00, 0x00};
    bcm_l3_host_t l3host;
    create_l3_intf_s intf;

    /* In Arad+ the urpf mode is per RIF (if the SOC property bcm886XX_l3_ingress_urpf_enable is set). */
    urpf_mode_per_rif = soc_property_get(unit , "bcm886xx_l3_ingress_urpf_enable",0);

    if (!urpf_mode_per_rif) {
      /* Set uRPF global configuration */
      rv =  bcm_switch_control_set(unit, bcmSwitchL3UrpfMode, L3_uc_rpf_mode);
      if (rv != BCM_E_NONE) {
        return rv;
      }
    }
    /*** create ingress router interface ***/
    /*rv = create_l3_intf(unit,flags,open_vlan,in_port,in_vlan,vrf,my_mac_address, &ing_intf_in); */
    rv = vlan__open_vlan_per_mc(unit,in_vlan, 0x1); 
    if (rv != BCM_E_NONE) {
        printf("Error, open_vlan=%d, in unit %d \n", in_vlan, unit);
    }    
    rv = bcm_vlan_gport_add(unit, in_vlan, in_port, 0);
    if (rv != BCM_E_NONE && rv != BCM_E_EXISTS) {
        printf("fail add port(0x%08x) to vlan(%d)\n", in_port, in_vlan);
      return rv;
    }

    intf.vsi = in_vlan;
    intf.my_global_mac = my_mac_address;
    intf.my_lsb_mac = my_mac_address;
    intf.vrf_valid = 1;
    intf.vrf = vrf;
    if (urpf_mode_per_rif) {
        intf.rpf_valid = 1;
        intf.flags |= BCM_L3_RPF;
        intf.urpf_mode = L3_uc_rpf_mode;
    }
    rv = l3__intf_rif__create(unit, &intf);
    ing_intf_in = intf.rif;     
    if (rv != BCM_E_NONE) {
        printf("Error, l3__intf_rif__create\n");
    }

    /*** create egress router interface ***/
    /*rv = create_l3_intf(unit,flags,open_vlan,out_port,out_vlan,vrf,my_mac_address, &ing_intf_out); */
    rv = vlan__open_vlan_per_mc(unit, out_vlan, 0x1);  
    if (rv != BCM_E_NONE) {
        printf("Error, open_vlan=%d, in unit %d \n", out_vlan, unit);
    }    
    rv = bcm_vlan_gport_add(unit, out_vlan, out_port, 0);
    if (rv != BCM_E_NONE && rv != BCM_E_EXISTS) {
        printf("fail add port(0x%08x) to vlan(%d)\n", out_port, out_vlan);
      return rv;
    }

    intf.vsi = out_vlan;
    intf.my_global_mac = my_mac_address;
    intf.my_lsb_mac = my_mac_address;
    intf.vrf_valid = 1;
    intf.vrf = vrf;
    if (urpf_mode_per_rif) {
        intf.rpf_valid = 1;
        intf.flags |= BCM_L3_RPF;
        intf.urpf_mode = L3_uc_rpf_mode;
    }
    rv = l3__intf_rif__create(unit, &intf);
    ing_intf_out = intf.rif;    

    if (rv != BCM_E_NONE) {
        printf("Error, l3__intf_rif__create\n");
    }

    fec[0] = 0;
    encap_id[0] = 0;
    /*** create egress object 1 ***/
    /* build FEC points to out-RIF and create ARP */
    /* Create ARP */
    create_l3_egress_s l3_egress;
    l3_egress.vlan = out_vlan;
    l3_egress.next_hop_mac_addr = next_hop_mac;
    l3_egress.arp_encap_id = encap_id[0];          
    rv = l3__egress_only_encap__create(unit, &l3_egress);
    if (rv != BCM_E_NONE) {
      printf("Error, in l3__egress_only_encap__create\n");
      return rv;
    }
    /* FEC must be format B, points to Out-RIF */
    l3_egress.out_gport = out_port;
    encap_id[0] = l3_egress.arp_encap_id;
    l3_egress.out_tunnel_or_rif = ing_intf_out;
    l3_egress.arp_encap_id = 0;
    rv = l3__egress_only_fec__create(unit, &l3_egress);
    if (rv != BCM_E_NONE) {
      printf("Error, in l3__egress_only_fec__create\n");
      return rv;
    }
    fec[0] = l3_egress.fec_id;
    printf("created FEC-id =0x%08x, \n", fec[0]);
    printf("next hop mac at encap-id %08x, \n", encap_id[0]);


    /*** add host entry with OUT-RIF + MAC + MAC-LSB + eg-port ***/
    
    bcm_l3_host_t_init(&l3host);
    l3host.l3a_flags = 0;
    l3host.l3a_ip_addr = host;
    l3host.l3a_vrf = vrf;
    l3host.l3a_intf = fec[0]; /* point to FEC to get out-interface  */
    l3host.l3a_modid = 0;
    l3host.l3a_nexthop_mac[5] = mac_lsb; /* MAC-LSB 4b carried as is to egress */
    /* set encap id to point to MAC address */
    /* as encap id is valid (!=0), host will point to FEC + outlif (MAC), and FEC will be "fixed" not to overwrite outlif */
    l3host.encap_id = encap_id[0];
    
    rv = bcm_l3_host_add(unit, &l3host);
    if (rv != BCM_E_NONE) {
        printf ("bcm_l3_host_add failed: %x \n", rv);
        printf("Error, create egress object, in_port=%d, \n", in_port);
    }
    print_host("add entry ", host,vrf);
    printf("---> egress-intf=0x%08x, egress-mac: port=%d, \n", ing_intf_out, out_port);

    return rv;
}

/*
 *  Runs the regular basic_example on one unit instead of an array of units
 *  Used to call this functionality from Dvapi tests
 */
int basic_example_single_unit(int unit, int in_port, int out_port) {
    int unit_ids[1];
    unit_ids[0] = unit;
    return basic_example(unit_ids,1,in_port,out_port);
}


/* Purpose: pmf parse the raw payload reult from the LPM (KAPS).
 *  	if the result from the kaps= lpm_payload (0x12345)
 *  	then forward the packet to port 13
 *
 * Usage:
 * when adding an entry to the LPM with api: "bcm_l3_route_add()"
 * use the flag "BCM_L3_FLAGS2_RAW_ENTRY"
 * see example in cint_ip_route; basic_example() with "lpm_raw_data_enabled=1"
 * 
 * Add PMF rule to parse the result from the KAPS
 */

int kaps_raw_data_redirect(int unit) {


    bcm_field_group_config_t grp;

    bcm_field_aset_t aset;

    bcm_field_entry_t action_entry=1;

    int sys_gport;

    int result = 0;

    bcm_gport_t dst_gport;

    BCM_GPORT_LOCAL_SET(dst_gport, 202);

    bcm_field_data_qualifier_t data_qual;

    if (large_direct_lu_key_length) {
        result = bcm_field_control_set(unit, bcmFieldControlLargeDirectLuKeyLength, large_direct_lu_key_length);
        if (BCM_E_NONE != result) {
            printf("Error in bcm_field_control_set with Err %x\n", result);
            return result;
        }

        bcm_field_data_qualifier_t_init(&data_qual);
        data_qual.flags = BCM_FIELD_DATA_QUALIFIER_OFFSET_PREDEFINED | BCM_FIELD_DATA_QUALIFIER_LENGTH_BIT_RES | BCM_FIELD_DATA_QUALIFIER_OFFSET_BIT_RES;
        data_qual.qualifier = bcmFieldQualifyL3DestRouteValue; 
        data_qual.length = large_direct_lu_key_length;
        data_qual.offset = 0;

        result = bcm_field_data_qualifier_create(unit, &data_qual);
        if (BCM_E_NONE != result) {
            printf("Error in bcm_field_data_qualifier_create with Err %x\n", result);
            return result;
        }
    }


    int presel_id;
    bcm_field_stage_t stage;

    bcm_field_presel_set_t presel_set;

    /*Define preselector*/
    result = bcm_field_presel_create(unit, &presel_id);
    if (BCM_E_NONE != result) {
        printf("Error in bcm_field_presel_create_id\n");
        return result;
    } else {
        printf("bcm_field_presel_create_id: %x\n", presel_id);
    }

    BCM_FIELD_PRESEL_INIT(presel_set);
    BCM_FIELD_PRESEL_ADD(presel_set, presel_id);

    stage = bcmFieldStageIngress;
    result = bcm_field_qualify_Stage(unit, presel_id | BCM_FIELD_QUALIFY_PRESEL, stage);

    /* Initialized Field Group */
    bcm_field_group_config_t_init(&grp);
	if (!large_direct_lu_key_length) {
    	grp.flags = BCM_FIELD_GROUP_CREATE_WITH_PRESELSET;
	} else {
    	grp.flags = BCM_FIELD_GROUP_CREATE_WITH_PRESELSET | BCM_FIELD_GROUP_CREATE_WITH_MODE | BCM_FIELD_GROUP_CREATE_LARGE | BCM_FIELD_GROUP_CREATE_WITH_ID;
    	grp.mode = bcmFieldGroupModeDirect;
        grp.group = 22 + large_direct_lu_key_length;
	}

    grp.preselset = presel_set;

    /*add qualifiers*/
    BCM_FIELD_QSET_INIT(grp.qset);
    
    if (!large_direct_lu_key_length) {
        BCM_FIELD_QSET_ADD(grp.qset, bcmFieldQualifyL3DestRouteValue);
    } else {
        result = bcm_field_qset_data_qualifier_add(unit, &grp.qset, data_qual.qual_id);
        if (BCM_E_NONE != result) {
            printf("bcm_field_qset_data_qualifier_add %x\n", result);
            return result;
        }
    }

    /* create field group */
    result = bcm_field_group_config_create(unit, &grp);
    if (BCM_E_NONE != result) {
        printf("Error in bcm_field_group_config_create with Err %x\n", result);
        return result;
    }


    BCM_FIELD_ASET_INIT(aset);

    BCM_FIELD_ASET_ADD(aset, bcmFieldActionRedirect);


    /* Attached the action to the field group */


    result = bcm_field_group_action_set(unit, grp.group, aset);
    if (BCM_E_NONE != result) {
        printf("Error in bcm_field_group_action_set Err %x\n", result);
        return result;
    }

    /*create an entry*/
    result = bcm_field_entry_create_id(unit, grp.group, action_entry);
    if (BCM_E_NONE != result) {
        printf("Error in bcm_field_entry_create Err %x\n", result);
        return result;
    }
    

    if (!large_direct_lu_key_length) {
        uint64 data;
        uint64 mask;
        COMPILER_64_SET(data, 0x00000000, lpm_payload);
        COMPILER_64_SET(mask, 0x00000000, 0xfffff);
        result = bcm_field_qualify_L3DestRouteValue(unit, action_entry, data, mask);
        if (BCM_E_NONE != result) {
            return result;
        }
    } else {
        int byte; /* iterator */
        int kaps_mask = (1 << large_direct_lu_key_length) - 1; /* large_direct_lu_key_length bits set */
        int size = ((large_direct_lu_key_length - 1) / 8) + 1; /* bytes of data / mask */
        uint8 data1[size], mask1[size];
        for(byte = 0; byte < size; byte++) {
            data1[byte] = (lpm_payload >> ((size - byte - 1) * 8)) % 256;
            mask1[byte] = (kaps_mask >> ((size - byte - 1) * 8)) % 256;
        }

        result = bcm_field_qualify_data(unit, action_entry, data_qual.qual_id, &data1, &mask1, size);
        if (BCM_E_NONE != result) {
            printf("Error in bcm_field_qualify_data\n");
            return result;
        }
    }


    result = bcm_field_action_add(unit, action_entry, bcmFieldActionRedirect, 0, dst_gport);
    if (BCM_E_NONE != result) {
        printf("Error in bcm_field_action_add\n");
        return result;
    }


    result = bcm_field_group_install(unit, grp.group);
    if (BCM_E_NONE != result) {
        printf("Error in bcm_field_group_install\n");
        return result;
    }

    return result;
}

/*
 * This cint creates one IN-RIF with VPN enable and two LEM entries (public and private) with the same
 * DIP but with different FECs to test the LEM public capability.
 */
int test_lem_public_entry(int unit, int outPort0, int outPort1) {

    bcm_l3_egress_t l3eg;
    bcm_l3_host_t l3host;
    bcm_l3_intf_t intf;
    bcm_l3_ingress_t ingress_intf;
    bcm_mac_t my_mac_address  = { 0x00, 0x0c, 0x00, 0x02, 0x00, 0x00 };  /* my-MAC */
    bcm_mac_t out_mac         = { 0x00, 0x45, 0x45, 0x45, 0x45, 0x00 };
    uint32 IPaddr             = 0x22664500;
    int rv, fecId, i;
    int vlan_vrf = 300;
    int out_port[2]; 


    out_port[0] = outPort0;
    out_port[1] = outPort1;


    /* create L3 interface */
    bcm_l3_intf_t_init(&intf);
    sal_memcpy(intf.l3a_mac_addr, my_mac_address, 6);
    intf.l3a_vid = vlan_vrf;
    intf.l3a_vrf = vlan_vrf;
    rv = bcm_l3_intf_create(unit, intf);
    if (rv != BCM_E_NONE) {
        printf("Error, bcm_l3_intf_create\n");
        return rv;
    }

    bcm_l3_ingress_t_init(&ingress_intf);
    ingress_intf.flags = BCM_L3_INGRESS_WITH_ID | BCM_L3_INGRESS_GLOBAL_ROUTE;
    ingress_intf.vrf= vlan_vrf;
    rv = bcm_l3_ingress_create(unit, &ingress_intf, &intf.l3a_intf_id);
    if (rv != BCM_E_NONE) {
        printf("Error, bcm_l3_intf_create \n");
    }


    for(i = 0; i < 2; i++) {

        bcm_l3_egress_t_init(&l3eg);
        out_mac[5] = i;
        sal_memcpy(l3eg.mac_addr, out_mac, 6);
        l3eg.vlan   = 400 + i;
        l3eg.port   = out_port[i];
        l3eg.flags  = BCM_L3_EGRESS_ONLY;
        rv = bcm_l3_egress_create(unit, 0, &l3eg, &fecId);
        if (rv != BCM_E_NONE) {
          printf ("bcm_l3_egress_create failed \n");
          return rv;
        }

        bcm_l3_host_t_init(&l3host);
        l3host.l3a_ip_addr = IPaddr;
        l3host.l3a_vrf = vlan_vrf * (1-i); /* Add one public entry and one private entry */
        l3host.l3a_intf = fecId;
        l3host.encap_id = l3eg.encap_id;

        rv = bcm_l3_host_add(unit, &l3host);
        if (rv != BCM_E_NONE) {
            printf ("bcm_l3_host_add failed: %x \n", rv);
            return rv;
        }
    }
    return rv;
}

int NOF_FECS = 100;
bcm_if_t FECS_ID[NOF_FECS];
/*
 * Create NOF_FECS MC entries which each entry results to a different FEC
 */
int create_fecs_and_ipmc_entries(int unit, int outPort0,int outPort1,int outPort2,int outPort3) {
    int i,j;
    int ipmc_index_offset = diag_test_mc(unit,"get_nof_egress_bitmap_groups");
    bcm_multicast_t mc_id[4];
    bcm_error_t rv ;
    bcm_mac_t my_mac_address     = {0x00, 0x0c, 0x00, 0x02, 0x00, 0x00};  /* my-MAC */
    bcm_l3_intf_t intf;
    int flags = BCM_MULTICAST_EGRESS_GROUP | BCM_MULTICAST_TYPE_L3 | BCM_MULTICAST_WITH_ID;
    bcm_gport_t gport;
    bcm_l3_egress_t l3eg;
    bcm_if_t fec = 0;
    bcm_gport_t mc_gport[4];
    bcm_ipmc_addr_t data;
    int ports[4];
    sal_srand(sal_time());

    ports[0] = outPort0;
    ports[1] = outPort1;
    ports[2] = outPort2;
    ports[3] = outPort3;


    for(j = 0;j < 4;j++) {
        mc_id[j] = ipmc_index_offset + (2 * j);
        rv = bcm_multicast_create(unit, flags, &mc_id[j]);
        BCM_GPORT_LOCAL_SET(gport,ports[j]);
        rv = bcm_multicast_egress_add(unit, mc_id[j], gport, 0);
        BCM_GPORT_MCAST_SET(mc_gport[j],mc_id[j]);
    }


    /* create L3 interface */
    bcm_l3_intf_t_init(&intf);
    sal_memcpy(intf.l3a_mac_addr, my_mac_address, 6);
    intf.l3a_vid = 400;
    intf.l3a_vrf = 400;
    rv = bcm_l3_intf_create(unit, intf);
    if (rv != BCM_E_NONE) {
     printf("Error, bcm_l3_intf_create\n");
     return rv;
    }

    /* CREATE FECS AND ROUTES */
    for (j = 0;j < NOF_FECS;j++) {

        bcm_l3_egress_t_init(&l3eg);
        flags = BCM_L3_INGRESS_ONLY;
        l3eg.port = mc_gport[sal_rand() % 4];
        l3eg.flags =  BCM_L3_RPF | BCM_L3_IPMC;
        rv = bcm_l3_egress_create(unit, flags, &l3eg, &fec);
        if (rv != BCM_E_NONE) {
            printf("Error, bcm_l3_egress_create\n");
            return rv;
        }

        FECS_ID[j] = fec;

        bcm_ipmc_addr_t_init(&data);
        data.flags = BCM_IPMC_SOURCE_PORT_NOCHECK;
        data.mc_ip_addr = 0xE1010101;
        data.mc_ip_mask = -1;
        data.vrf = 400;
        data.s_ip_addr = 0x04070100 + j;
        data.s_ip_mask = -1;
        data.l3a_intf = fec;
        rv = bcm_ipmc_add(unit,&data);
        if (rv != BCM_E_NONE) {
            printf("Error, bcm_ipmc_add\n");
            return rv;
        }

    }

    return rv;
}
/*
 * Given a FEC index to get a FEC ID this cint return whether the FEC was access or not
 */
int was_fec_got_hit(int unit,int fec_number) {
    bcm_l3_egress_t l3eg;
    bcm_l3_egress_get(unit,FECS_ID[fec_number],&l3eg);
    if((l3eg.flags & BCM_L3_HIT) == 0 ) {
        return 0;
    }
    return 1;
}
