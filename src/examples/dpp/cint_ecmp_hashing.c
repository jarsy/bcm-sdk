/*
 * $Id: cint_ecmp_hashing.c,v 1.7 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * ECMP hashing example script
 *
 */

/* 
the cint creates an ECMP, containing 10 FEC entries, and points an IPV4 host and MPLS LSR to this ECMP. 
each FEC points at a LL entry with the same DA, but different out-port.
 
default ECMP hashing example: 
-----------------------------
run:
cint utility/cint_utils_l3.c 
cint utility/cint_utils_vlan.c 
cint cint_ip_route.c 
cint cint_ecmp_hashing.c 
cint 
print ecmp_hashing_main(unit, <in_port>, <out_port>, <ecmp_size>); 
 
traffic example: 
run: 
    1) ethernet header with DA 0:C:0:2:01:23 and vlan tag id 17
       and IPV4 header with DIP 10.0.255.0 and various SIPs (random)
    2) ethernet header with DA 0:C:0:2:01:23 and vlan tag id 17
       MPLS header with label_1 44 and various label_2 (incremental)
 
ecmp_hashing_main() does not change the default ECMP hashing configuration, which is to look at all the packet fields: 
for IPV4 packet - DIP, SIP, protocol, dest L4 port and src L4 port.
for MPLS packet - fisrt label, second label and third label. 
 
traffic will be divided equally between the ECMP memebers (FEC entries, each pointing to a different port). 
 
disable ECMP hashing example: 
----------------------------- 
run: 
disable_ecmp_hashing(unit); 
 
run same traffic. this time ECMP hashing will be set to "look at nothing". 
none of the header fields mentioned above will be used for hashing. 
in this case, all packets will be sent to one FEC entry. 

print ecmp_hash_func_config(unit, BCM_HASH_CONFIG_ROUND_ROBIN); 

ecmp_hash_func_config() with BCM_HASH_CONFIG_ROUND_ROBIN uses a counter that is incremented every packet, instead of polynomial hashing, 
so traffic will be divided equally between the ECMP memebers, although ECMP hashing is disabled. 

print ecmp_hash_func_config(unit, BCM_HASH_CONFIG_CRC16_0x101a1); 

return ECMP hashing hashing to be done according to some polynomial, like before.
 
ECMP hashing by IPV4 SIP example: 
---------------------------------
run: 
ecmp_hash_sip(unit); 
 
run same IPV4 traffic. this time ECMP hashing will be done according to the IPV4 SIP. 
in this case, all packets will be divided equally between the ECMP memebers (different ports). 
 
ECMP hashing by MPLS label2 example: 
------------------------------------
run: 
ecmp_hash_label2(unit); 
 
run same MPLS traffic. this time ECMP hashing will be done according to the MPLS label2. 
in this case, all packets will be divided equally between the ECMP memebers (different ports). 
 
ECMP hashing by SRC port: 
-------------------------
run: 
ecmp_src_port_enable(unit, 1);
 
run IPV4 traffic, with fixed SIP, from port 13. All packets will go to one of the ECMP memebers (one dest port). 
run same IPV4 traffic (with fixed SIP) from port 14. All packets will go to one of the ECMP memebers, but it will be a different one than before. 
Changing the SRC port will change the packets' destination, because hashing takes into account the SRC port.
 
ECMP hashing by 2 headers: 
-------------------------
run: 
ecmp_nof_headers_set(unit, <in_port>, 2);
 
run IPoIPoEth: 
    ethernet header with DA 0:C:0:2:01:23, fixed SA and vlan tag id 17
    IPV4 header with DIP 10.0.255.0, and fixed SIP
    and another IPV4 header with random DIP and random SIP
 
packets will be divided between all ECMP memebers (dest ports), because ECMP hashing is set to look at 2 headers, 
starting from the forwrding header, and it is also set to look at IPV4 SIP.

ECMP using ELI-BOS:
-------------------------
run:
ecmp_hashing_eli_bos(int unit, int inpotr, int outport)

This test creates an ECMP of size ELI_BOS_NUMBER_OF_INTERFACES  where, each member in the  ECMP switch the
first MPLS label to a different label to track the selected member.
*/

struct cint_ecmp_hashing_cfg_s {
  int ecmp_api_is_ecmp;
};

struct cint_ecmp_hashing_data_s {
  /* After ecmp_hashing_main is run successfully, this will contain the ECMP object that was created. */
  /* Only works with the bcm_l3_egress_ecmp_* interfaces.*/
  bcm_l3_egress_ecmp_t ecmp;
  /* After ecmp_hashing_main is run successfully, this will contain the FEC objects that were created. */
  bcm_if_t egress_intfs[1000]; /* FECs */
};

cint_ecmp_hashing_cfg_s cint_ecmp_hashing_cfg = { 
  1 /* ecmp_api_is_ecmp */
};

cint_ecmp_hashing_data_s cint_ecmp_hashing_data;

/* delete functions */
int delete_host(int unit, int intf) {
  int rc;
  bcm_l3_host_t l3host;

  bcm_l3_host_t_init(l3host);

  l3host.l3a_flags = 0;
  l3host.l3a_intf = intf;

  rc = bcm_l3_host_delete_all(unit, l3host);
  if (rc != BCM_E_NONE) {
    printf ("bcm_l3_host_delete_all failed: %d \n", rc);
  }
  return rc;
}

/* 
disable_ecmp_hashing(): 
set ECMP hashing to "look at nothing". 
no part of the header will be used in hashing. 
in this case, hashing result will be the same for every packet that arrives.
*/ 
int disable_ecmp_hashing(int unit) {
  int rc;
  int arg = 0; /* arg = 0 so no field in the Eth/IPV4/MPLS header will be looked at */
  bcm_switch_control_t type;

  /* diable L2 hashing*/
  type = bcmSwitchHashL2Field0;
  rc = bcm_switch_control_set(unit, type, arg);
  if (rc != BCM_E_NONE) {
	printf ("bcm_petra_switch_control_set with type bcmSwitchHashL2Field0 failed: %d \n", rc);
  }

  /* disable IPV4 hashing */
  type = bcmSwitchHashIP4Field0;
  rc = bcm_switch_control_set(unit, type, arg);
  if (rc != BCM_E_NONE) {
    printf ("bcm_petra_switch_control_set with type bcmSwitchHashIP4Field0 failed: %d \n", rc);
  }

  /* disable MPLS hashing */
  type = bcmSwitchHashMPLSField0;
  rc = bcm_switch_control_set(unit, type, arg);
  if (rc != BCM_E_NONE) {
    printf ("bcm_petra_switch_control_set with type bcmSwitchHashMPLSField0 failed: %d \n", rc);
  }

  type = bcmSwitchHashMPLSField1;
  rc = bcm_switch_control_set(unit, type, arg);
  if (rc != BCM_E_NONE) {
    printf ("bcm_petra_switch_control_set with type bcmSwitchHashMPLSField1 failed: %d \n", rc);
  }

  return rc;
}

/* 
ecmp_hash_sip(): 
set ECMP hashing to done according to IPV4 SIP. 
in this case, hashing result will be the same for every packet with the same SIP.
*/ 
int ecmp_hash_sip(int unit) {
  int rc;
  int arg;
  bcm_switch_control_t type = bcmSwitchHashIP4Field0;

  /* only the whole SIP can be used for hashing, so both LO and HI must be used together */
  arg = BCM_HASH_FIELD_IP4SRC_LO | BCM_HASH_FIELD_IP4SRC_HI;
  rc = bcm_switch_control_set(unit, type, arg);
  if (rc != BCM_E_NONE) {
    printf ("bcm_petra_switch_control_set with type bcmSwitchHashIP4Field0 failed: %d \n", rc);
  }

  return rc;
}

/* 
ecmp_hash_sip(): 
set ECMP hashing to done according to IPV4 SIP. 
in this case, hashing result will be the same for every packet with the same SIP.
*/ 
int ecmp_hash_sipv6(int unit) {
  int rc;
  int arg;
  bcm_switch_control_t type = bcmSwitchHashIP6Field0;

  /* only the whole SIP can be used for hashing, so both LO and HI must be used together */
  arg = BCM_HASH_FIELD_IP6SRC_LO | BCM_HASH_FIELD_IP6SRC_HI;
  rc = bcm_switch_control_set(unit, type, arg);
  if (rc != BCM_E_NONE) {
    printf ("bcm_petra_switch_control_set with type bcmSwitchHashIP4Field0 failed: %d \n", rc);
  }

  return rc;
}

/* 
ecmp_hash_label2(): 
set ECMP hashing to done according to MPLS label2. 
in this case, hashing result will be the same for every packet with the same label2.
*/ 
int ecmp_hash_label2(int unit) {
  int rc;
  int arg = BCM_HASH_FIELD_2ND_LABEL;
  bcm_switch_control_t type = bcmSwitchHashMPLSField0;

  rc = bcm_switch_control_set(unit, type, arg);
  if (rc != BCM_E_NONE) {
    printf ("bcm_petra_switch_control_set with type bcmSwitchHashMPLSField0 failed: %d \n", rc);
  }

  return rc;
}

/* 
ecmp_hash_l4_dest(): 
set ECMP hashing to done according to destination L4 port. 
*/ 
int ecmp_hash_l4_dst(int unit) {
  int rc;
  int arg = BCM_HASH_FIELD_DSTL4;
  bcm_switch_control_t type = bcmSwitchHashIP4Field0;

  rc = bcm_switch_control_set(unit, type, arg);
  if (rc != BCM_E_NONE) {
    printf ("bcm_petra_switch_control_set with type bcmSwitchHashIP4Field0 and arg %d failed: %d \n", arg, rc);
  }

  return rc;
}

/* 
ecmp_hash_func_config(): 
set ECMP hashing function (polynomial). 
bcm_hash_config = BCM_HASH_CONFIG_*
*/ 
int ecmp_hash_func_config(int unit, int bcm_hash_config) {
  int rc;
  bcm_switch_control_t type = bcmSwitchECMPHashConfig;

  rc = bcm_switch_control_set(unit, type, bcm_hash_config);
  if (rc != BCM_E_NONE) {
    printf ("bcm_petra_switch_control_set with type bcmSwitchECMPHashConfig failed: %d \n", rc);
  }

  return rc;
}

/* 
ecmp_src_port_enable(): 
Make the Source port a part of the ECMP hash.
arg = 1- enable, 0- disable. 
*/ 
int ecmp_src_port_enable(int unit, int arg) {
  int rc;
  bcm_switch_control_t type = bcmSwitchECMPHashSrcPortEnable;

  rc = bcm_switch_control_set(unit, type, arg);
  if (rc != BCM_E_NONE) {
    printf ("bcm_petra_switch_control_set with type bcmSwitchECMPHashSrcPortEnable failed: %d \n", rc);
  }

  return rc;
}

/* 
ecmp_nof_headers_set(): 
Selects the number of headers to consider in ECMP hashing.
nof_headers - can be 1/2. 
*/ 
int ecmp_nof_headers_set(int unit, int in_port, int nof_headers) {
  int rc;
  bcm_switch_control_t type = bcmSwitchECMPHashPktHeaderCount;

  rc = bcm_switch_control_port_set(unit, in_port, type, nof_headers);
  if (rc != BCM_E_NONE) {
    printf ("bcm_petra_switch_control_port_set with type bcmSwitchECMPHashPktHeaderCount failed: %d \n", rc);
  }

  return rc;
}

/* 
ecmp_hashing_main():
1) create ECMP, containing <ecmp_size> FEC entries, each FEC points at a LL entry with the same DA, but different out-port
2) Add IPV4 host entry and point to ECMP.
3) Add switch entry to swap MPLS labels and also send to ECMP
 
ecmp_size = the size of the ECMP that will be created (how many FECs will it contain) 
            this will also determine the number of out-ports that will be used (because each FEC points to a different out-port)
out_port  = the out ports numbers will be: <out_port>, <out_port> + 1, ... , <out_port> + <ecmp_size> -1
*/ 
int ecmp_hashing_main(int unit, int in_port, int out_port, int ecmp_size) {

  int CINT_NO_FLAGS = 0;
  int rc, i;
  int vrf = 5;
  int in_vlan = 17;
  int out_vlan = 17;
  bcm_pbmp_t pbmp, ubmp;

  if (ecmp_size > 1000) {
    printf("Error - Max ECMP size allowed is 1000.\n");
    return BCM_E_PARAM;
  }

  int ingress_intfs[2]; /* in-RIF and out-RIF */  
  int encap_ids[ecmp_size];
  bcm_if_t multipath_id; /* ECMP */

  bcm_mac_t mac_address  = {0x00, 0x0c, 0x00, 0x02, 0x01, 0x23};  /* my-MAC */
  bcm_mac_t next_mac_address  = {0x00, 0x00, 0x00, 0x00, 0xcd, 0x1d}; /* outgoing DA */

  int host = 0x0a00ff00; /* 10.0.255.0 */
  bcm_l3_host_t l3host;

  int in_label = 44;
  int eg_label = 66;
  bcm_mpls_tunnel_switch_t mpls_tunnel_info;

  BCM_PBMP_CLEAR(ubmp);

  /* create in-RIF */
  rc = vlan__open_vlan_per_mc(unit, in_vlan, 0x1);
  if (rc != BCM_E_NONE) {
  	printf("Error, open_vlan=%d, in unit %d \n", in_vlan, unit);
  }
  rc = bcm_vlan_gport_add(unit, in_vlan, in_port, 0);
  if (rc != BCM_E_NONE && rc != BCM_E_EXISTS) {
  	printf("fail add port(0x%08x) to vlan(%d)\n", in_port, in_vlan);
    return rc;
  }

  create_l3_intf_s intf;
  intf.vsi = in_vlan;
  intf.my_global_mac = mac_address;
  intf.my_lsb_mac = mac_address;
  intf.vrf_valid = 1;
  intf.vrf = vrf;

  rc = l3__intf_rif__create(unit, &intf);
  ingress_intfs[0] = intf.rif;        
  if (rc != BCM_E_NONE) {
  	printf("Error, l3__intf_rif__create\n");
  }

  /* create out-RIF */
  rc = vlan__open_vlan_per_mc(unit, out_vlan, 0x1);  
  if (rc != BCM_E_NONE) {
  	printf("Error, open_vlan=%d, in unit %d \n", out_vlan, unit);
  }
  rc = bcm_vlan_gport_add(unit, out_vlan, out_port, 0);
  if (rc != BCM_E_NONE && rc != BCM_E_EXISTS) {
  	printf("fail add port(0x%08x) to vlan(%d)\n", out_port, out_vlan);
    return rc;
  }

  intf.vsi = out_vlan;

  rc = l3__intf_rif__create(unit, &intf);
  ingress_intfs[1] = intf.rif;        
  if (rc != BCM_E_NONE) {
  	printf("Error, l3__intf_rif__create\n");
  }

  /* create 10 FEC entries (all with the same out-RIF)
     each FEC will point to a different out-port.
     also set vlan-port membership. each out-port will have a different vlan */
  out_port--;
  for (i = 0; i < ecmp_size; i++) {

      out_port++;
      out_vlan++;

      BCM_PBMP_CLEAR(pbmp);
      BCM_PBMP_PORT_ADD(pbmp, out_port);

      /* create FEC */
    create_l3_egress_s l3eg;
    sal_memcpy(l3eg.next_hop_mac_addr, next_mac_address , 6);
    l3eg.allocation_flags = CINT_NO_FLAGS;
    l3eg.l3_flags = CINT_NO_FLAGS;
    l3eg.out_tunnel_or_rif = ingress_intfs[1];
    l3eg.out_gport = out_port;
    l3eg.vlan = out_vlan;
    l3eg.fec_id = cint_ecmp_hashing_data.egress_intfs[i];
    l3eg.arp_encap_id = encap_ids[i];

    rc = l3__egress__create(unit,&l3eg);
    if (rc != BCM_E_NONE) {
        printf("Error, l3__egress__create\n");
        return rc;
    }

    cint_ecmp_hashing_data.egress_intfs[i] = l3eg.fec_id;
    encap_ids[i] = l3eg.arp_encap_id; 

      /* set vlan port membership for out-vlan and out-port */
      rc = bcm_vlan_port_add(unit, out_vlan, pbmp, ubmp);
      if (rc != BCM_E_NONE) {
        printf ("bcm_vlan_port_add no. %d failed: %d \n", i, rc);
        print rc;
      }  
  }

  if (cint_ecmp_hashing_cfg.ecmp_api_is_ecmp) {
    bcm_l3_egress_ecmp_t_init(&cint_ecmp_hashing_data.ecmp);
    cint_ecmp_hashing_data.ecmp.max_paths = ecmp_size;

    /* create an ECMP, containing the FEC entries */
    rc = bcm_l3_egress_ecmp_create(unit, &cint_ecmp_hashing_data.ecmp, ecmp_size, cint_ecmp_hashing_data.egress_intfs);
    if (rc != BCM_E_NONE) {
      printf ("bcm_l3_egress_ecmp_create failed: %d \n", rc);
      return rc;
    }
  } else {
    rc = bcm_l3_egress_multipath_create(unit, CINT_NO_FLAGS, ecmp_size, cint_ecmp_hashing_data.egress_intfs, &multipath_id);
    if (rc != BCM_E_NONE) {
      printf ("bcm_l3_egress_multipath_create failed: %d \n", rc);
      return rc;
    }
  }

  /* add host entry and point to the ECMP */
  bcm_l3_host_t_init(&l3host);
  l3host.l3a_ip_addr = host;
  l3host.l3a_vrf = vrf;
  if (cint_ecmp_hashing_cfg.ecmp_api_is_ecmp) {
    l3host.l3a_intf = cint_ecmp_hashing_data.ecmp.ecmp_intf;
  } else {
    l3host.l3a_intf = multipath_id; /* ECMP */
  }

  rc = bcm_l3_host_add(unit, &l3host);
  if (rc != BCM_E_NONE) {
    printf ("bcm_l3_host_add failed: %x \n", rc);
    return rc;
  }

  /* add switch entry to swap labels and map to ECMP */
  bcm_mpls_tunnel_switch_t_init(&mpls_tunnel_info);
  mpls_tunnel_info.action = BCM_MPLS_SWITCH_ACTION_SWAP;
  mpls_tunnel_info.flags = BCM_MPLS_SWITCH_TTL_DECREMENT; /* TTL decrement has to be present */
  mpls_tunnel_info.flags |= BCM_MPLS_SWITCH_OUTER_TTL|BCM_MPLS_SWITCH_OUTER_EXP;
  mpls_tunnel_info.label = in_label; /* incomming label */
  mpls_tunnel_info.egress_label.label = eg_label; /* outgoing (egress) label */
  if (cint_ecmp_hashing_cfg.ecmp_api_is_ecmp) {
    mpls_tunnel_info.egress_if = cint_ecmp_hashing_data.ecmp.ecmp_intf;
  } else {
    mpls_tunnel_info.egress_if = multipath_id; /* ECMP */
  }

  rc = bcm_mpls_tunnel_switch_create(unit, &mpls_tunnel_info);
  if (rc != BCM_E_NONE) {
    printf ("bcm_mpls_tunnel_switch_create failed: %x \n", rc);
    return rc;
  }

  return 0;
}

/*
 * This cint use to test ELI-BOS ECMP feature, it creates an ECMP group of size ELI_BOS_NUMBER_OF_INTERFACES.
 * Each FEC in the ECMP group swap the first MPLS label to a different label than the others FECs to identify
 * the ECMP member from which the in-coming packet passed.
 */
int ELI_BOS_NUMBER_OF_INTERFACES = 200;
int ecmp_hashing_eli_bos(int unit, int inpotr, int outport) {

    int rv, i;
    int tunnel_id;
    int in_label = 0x100;
    int out_label = 0x1000;
    uint32 testLAbel  = 0xCCCCC;
    bcm_if_t ecmpInterfaces[ELI_BOS_NUMBER_OF_INTERFACES];
    bcm_l3_intf_t l3InIf;
    bcm_if_t l3egid, ecmpFecId;
    bcm_l3_egress_t l3eg;
    bcm_mac_t mac_l3_ingress_add = {0x00, 0x00, 0x00, 0x00, 0xcd, 0x1d};
    bcm_mac_t mac_l3_engress_add = {0x00, 0x00, 0x00, 0x00, 0x66, 0x22};

    bcm_mpls_tunnel_switch_t entry;
    bcm_mpls_egress_label_t label_array[1];



    /*Create L3 interface - in-rif*/
    bcm_l3_intf_t_init(l3InIf);

    sal_memcpy(l3InIf.l3a_mac_addr, mac_l3_ingress_add, 6);
    l3InIf.l3a_vid = 400;
    l3InIf.l3a_vrf = 400;
    rv = bcm_l3_intf_create(unit, &l3InIf);
    if (rv != BCM_E_NONE) {
       return rv;
    }


    /*Create L3 egress*/
    bcm_l3_egress_t_init(&l3eg);

    sal_memcpy(l3eg.mac_addr, mac_l3_engress_add, 6);
    l3eg.vlan   = 400;
    l3eg.port   = outport;
    l3eg.flags  = BCM_L3_EGRESS_ONLY;

    rv = bcm_l3_egress_create(unit, 0, &l3eg, &l3egid);
    if (rv != BCM_E_NONE) {
      printf ("bcm_l3_egress_create failed \n");
      return rv;
    }

    /*
     * Create FECs that points to a switch tunnel with different labels to identify a different ECMP path.
     */

    for(i=0; i < ELI_BOS_NUMBER_OF_INTERFACES;i++) {

        label_array[0].exp = 0;
        label_array[0].label = out_label + i;
        label_array[0].flags = BCM_MPLS_EGRESS_LABEL_ACTION_VALID|BCM_MPLS_EGRESS_LABEL_TTL_SET|BCM_MPLS_EGRESS_LABEL_TTL_DECREMENT;
        label_array[0].flags |= BCM_MPLS_EGRESS_LABEL_EXP_SET;
        label_array[0].action = BCM_MPLS_EGRESS_ACTION_SWAP;
        label_array[0].l3_intf_id = l3egid;
        label_array[0].ttl = 30;

        rv = bcm_mpls_tunnel_initiator_create(unit,0,1,label_array);
        if (rv != BCM_E_NONE) {
            printf("Error, bcm_mpls_tunnel_initiator_create Label = 0x%x\n", label_array[0].label);
            return rv;
        }

        tunnel_id = label_array[0].tunnel_id;

        bcm_l3_egress_t l3eg_fec;
        bcm_l3_egress_t_init(&l3eg_fec);

        l3eg_fec.intf = tunnel_id;
        l3eg_fec.port = outport;
        l3eg_fec.vlan = 400;

        rv = bcm_l3_egress_create(unit, BCM_L3_INGRESS_ONLY, &l3eg_fec, &ecmpFecId);
        if (rv != BCM_E_NONE) {
           printf("Error, bcm_l3_egress_create\n");
           return rv;
        }

        ecmpInterfaces[i] = ecmpFecId;
    }

    /* create ECMP group */
    bcm_l3_egress_ecmp_t ecmpIf;
     bcm_l3_egress_ecmp_t_init(&ecmpIf);
     ecmpIf.max_paths = ELI_BOS_NUMBER_OF_INTERFACES;
     rv = bcm_l3_egress_ecmp_create(unit, &ecmpIf, ELI_BOS_NUMBER_OF_INTERFACES, ecmpInterfaces);
     if (rv != BCM_E_NONE) {
       printf ("bcm_l3_egress_create failed \n");
       return rv;
     }


    /*create MPLS LSR */
    for(i=0; i < 21 ;i++) {
        bcm_mpls_tunnel_switch_t_init(&entry);
        entry.action = BCM_MPLS_SWITCH_ACTION_NOP;
        entry.flags = BCM_MPLS_SWITCH_TTL_DECREMENT;
        entry.flags |= BCM_MPLS_SWITCH_OUTER_TTL|BCM_MPLS_SWITCH_OUTER_EXP;
        entry.label = flipBitInMPLSlabel(testLAbel,i);
        printf("i = %d Label 0x%x\n",i,entry.label);
        entry.egress_if = ecmpIf.ecmp_intf;
        entry.port = inpotr;

        rv = bcm_mpls_tunnel_switch_create(unit,&entry);
        if (rv != BCM_E_NONE) {
            printf("Error, in bcm_mpls_tunnel_switch_create\n");
            return rv;
        }
    }

    return rv;
}
/*
 * Flip One bit in the MPLS label
 */
uint32 flipBitInMPLSlabel(uint32 label, uint32 bit) {

    if (((1 << bit) & label) > 0) {
        label &=  (~(1 << bit));
    } else {
        label |= (1 << bit);
    }
    return (0xFFFFF & label);
}

