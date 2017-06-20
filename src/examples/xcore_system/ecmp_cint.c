/*
 *  ECMP script 2009-sept-29 
 *
 * $Id: ecmp_cint.c,v 1.3 Broadcom SDK $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


/*
 * Global Configuration variables
 */

int feunit = 1;
int qeunit = 0;
int CMD_FAIL = -1;

bcm_module_t femod;    
bcm_stk_modid_get (feunit, &femod);
bcm_module_t qemod;
bcm_stk_modid_get (qeunit, &qemod);

/* ECMP configuration */
int ecmp_port_count = 4;
int ecmp_ports[ecmp_port_count] = { 11, 12, 13, 14 };
bcm_l3_intf_t ecmp_l3if[ecmp_port_count];
bcm_l3_egress_t ecmp_l3eg[ecmp_port_count];
bcm_if_t ecmp_egif[ecmp_port_count] = { 0 };
bcm_if_t ecmp_mpath_id = 0;
bcm_l3_route_t ecmp_l3rt;
int ecmp_ip_addr = 0x08ff0005;
int ecmp_ip_mask = 0xffffff00;

int ecmp_vlan_base = 0xa;
bcm_mac_t  ecmp_lsmac = {0x00, 0x4e, 0xa5, 0x00, 0x00, 0x00};
bcm_mac_t  ecmp_mac_base = {0x00, 0x4e, 0xa5, 0xcc, 0xdd, 0x00};
int ecmp_ttl_limit = 0x5;

/* Access port configuration */
int access_port_count = 1;
int access_ports[access_port_count] = { 0 };
bcm_l3_intf_t access_l3if[access_port_count];

bcm_mac_t  access_lsmac = {0x00, 0x4e, 0xa5, 0x00, 0x00, 0x00};
bcm_mac_t  access_mac_base = {0x00, 0x4e, 0xa5, 0xff, 0xff, 0x00};
int access_vlan_base = 0x100;
bcm_l3_egress_t access_l3eg[access_port_count];
bcm_if_t access_egif[access_port_count] = { 0 };
int access_ttl_limit = 0x5;

/* Utility Functions */

int fe_port_to_fe_gport (int feport) {
  bcm_gport_t fegport;
  bcm_stk_modid_get (feunit, &femod);
  BCM_GPORT_MODPORT_SET (fegport, &femod, &feport);
  return (fegport);
}

int qe_from_fe (int fe) {
    return (fe - 1);
}

int ecmp_vlan_get(int port) {
    return (ecmp_vlan_base + port);
}

/* construct MAC address based on port number */
int ecmp_mac_get(int port, char* mac) {
    mac[0] = ecmp_mac_base[0];
    mac[1] = ecmp_mac_base[1];
    mac[2] = ecmp_mac_base[2];
    mac[3] = ecmp_mac_base[3];
    mac[4] = ecmp_mac_base[4];
    mac[5] = port;
    return 0;

}

int access_vlan_get(int port) {
    return (access_vlan_base + port);
}

/* construct MAC address based on port number */
int access_mac_get(int port, char* mac) {
    mac[0] = access_mac_base[0];
    mac[1] = access_mac_base[1];
    mac[2] = access_mac_base[2];
    mac[3] = access_mac_base[3];
    mac[4] = access_mac_base[4];
    mac[5] = port;
    return 0;

}

int fe_vlan_new(int fe, int vlan, bcm_pbmp_t pbmp, bcm_pbmp_t u_pbmp) {

  int rc;

  rc = bcm_vlan_create (fe, vlan);
  if (BCM_FAILURE(rc)) {
    printf("BCM FAIL %d: %s\n", rc, bcm_errmsg(rc));
    return CMD_FAIL;
  }

  rc = bcm_vlan_port_add(fe, vlan, pbmp, u_pbmp);
  if (BCM_FAILURE(rc)) {
    printf("BCM FAIL %d: %s\n", rc, bcm_errmsg(rc));
    return CMD_FAIL;
  }

  return 0;
}

int qe_vlan_new (int qe, int vlan, bcm_pbmp_t pbmp, bcm_pbmp_t u_pbmp) {
  int rc;
  int i;
  int qemod;
  bcm_gport_t qegport, fegport;
  int mcgflags = BCM_MULTICAST_WITH_ID | BCM_MULTICAST_TYPE_L2 | BCM_MULTICAST_DISABLE_SRC_KNOCKOUT;
  bcm_if_t encap;
  bcm_port_t port;

  /* create multicast group */
  rc = bcm_multicast_create(qe, mcgflags, vlan);
  if (BCM_FAILURE(rc)) {
    printf("BCM FAIL %d: %s\n", rc, bcm_errmsg(rc));
    return CMD_FAIL;
  }
  printf ("Multicast group 0x%x created\n", vlan);
  /* add ports */
  BCM_PBMP_ITER(pbmp, port) { 
    printf("  Adding port: 0x%x\n", port);

    fegport = fe_port_to_fe_gport(port);
    rc = bcm_stk_fabric_map_get(feunit, fegport, &qegport);
    if (BCM_FAILURE(rc)) {
      printf("BCM FAIL %d: %s\n", rc, bcm_errmsg(rc));
      return CMD_FAIL;
    }
    rc = bcm_multicast_l2_encap_get(feunit, vlan, fegport, vlan, &encap);
    if (BCM_FAILURE(rc)) {
      printf("BCM FAIL %d: %s\n", rc, bcm_errmsg(rc));
      return CMD_FAIL;
    }
    rc = bcm_multicast_egress_add(qeunit, vlan, qegport, encap);
    if (BCM_FAILURE(rc)) {
      printf("BCM FAIL %d: %s\n", rc, bcm_errmsg(rc));
      return CMD_FAIL;
    }
  }

}


int ecmp_test_config() {
  int rc;
  bcm_pbmp_t pbmp;
  bcm_pbmp_t u_pbmp;
  int i;
  bcm_l2_addr_t t_l2addr;
  bcm_mac_t t_mac; 

  printf (" \n *** version 090929_v005 *** \n");

  /* Configuring ECMP interfaces */
  for (i=0; i<ecmp_port_count; i++) {
    /* create vlan */
    int vlan = ecmp_vlan_get(ecmp_ports[i]);
    BCM_PBMP_CLEAR(pbmp);
    BCM_PBMP_PORT_ADD(pbmp, ecmp_ports[i]);
    fe_vlan_new(feunit, vlan, pbmp, pbmp);
    qe_vlan_new(qeunit, vlan, pbmp, pbmp);
    printf("VLAN: 0x%x created on port 0x%x\n", vlan, ecmp_ports[i]);

    /* create L3 interface on the vlan */
    bcm_l3_intf_t* l3if = &ecmp_l3if[i];
    bcm_l3_intf_t_init(l3if);
    l3if->l3a_mac_addr = ecmp_lsmac;
    l3if->l3a_vid = vlan;
    l3if->l3a_ttl = ecmp_ttl_limit;
    rc = bcm_l3_intf_create (feunit, l3if);
    if (BCM_FAILURE(rc)) {
      printf("BCM FAIL %d: %s\n", rc, bcm_errmsg(rc));
      return CMD_FAIL;
    }
    printf("L3 interface created, MAC 0x%02x:0x%02x:0x%02x:0x%02x:0x%02x:0x%02x on VLAN 0x%x\n",
           l3if->l3a_mac_addr[0], 
           l3if->l3a_mac_addr[1], 
           l3if->l3a_mac_addr[2], 
           l3if->l3a_mac_addr[3], 
           l3if->l3a_mac_addr[4], 
           l3if->l3a_mac_addr[5], vlan);

    /* create L3 egress for the vlan, port */
    bcm_l3_egress_t *l3eg = &ecmp_l3eg[i];
    bcm_if_t *l3eg_path_id = &ecmp_egif[i];
    bcm_l3_egress_t_init(l3eg);
    l3eg->intf = l3if->l3a_intf_id;
    ecmp_mac_get(ecmp_ports[i], l3eg->mac_addr);
    l3eg->vlan = vlan;
    l3eg->module = femod;
    l3eg->port = ecmp_ports[i];
    rc = bcm_l3_egress_create(feunit, 0, l3eg, l3eg_path_id);
    if (BCM_FAILURE(rc)) {
      printf("BCM FAIL %d: %s\n", rc, bcm_errmsg(rc));
      return CMD_FAIL;
    }
    printf("L3 egress created, MAC 0x%02x:0x%02x:0x%02x:0x%02x:0x%02x:0x%02x on VLAN 0x%x\n",
           l3eg->mac_addr[0], 
           l3eg->mac_addr[1], 
           l3eg->mac_addr[2], 
           l3eg->mac_addr[3], 
           l3eg->mac_addr[4], 
           l3eg->mac_addr[5], vlan);

  }

  /* create multipath L3 egress */
printf("creating multipath\n");
  rc = bcm_l3_egress_multipath_create(feunit, 0, ecmp_port_count, ecmp_egif, &ecmp_mpath_id);
  if (BCM_FAILURE(rc)) {
    printf("BCM FAIL %d: %s\n", rc, bcm_errmsg(rc));
    return CMD_FAIL;
  }
  printf ("L3 egress multipath created: 0x%x\n", ecmp_mpath_id);

  /* add route */
  bcm_l3_route_t_init(&ecmp_l3rt);
  ecmp_l3rt.l3a_flags = BCM_L3_MULTIPATH;
  ecmp_l3rt.l3a_subnet = ecmp_ip_addr;
  ecmp_l3rt.l3a_ip_mask = ecmp_ip_mask;
  ecmp_l3rt.l3a_intf = ecmp_mpath_id;
  rc = bcm_l3_route_add(feunit, &ecmp_l3rt);
  if (BCM_FAILURE(rc)) {
    printf("BCM FAIL %d: %s\n", rc, bcm_errmsg(rc));
    return CMD_FAIL;
  }
  printf("L3 multipath route created, 0x%08x (0x%08x)\n", ecmp_l3rt.l3a_subnet, ecmp_l3rt.l3a_ip_mask);

  /* Configure Access interfaces */
  for (i=0; i<access_port_count; i++) {
    /* create vlan */
    int vlan = access_vlan_get(access_ports[i]);
    BCM_PBMP_CLEAR(pbmp);
    BCM_PBMP_PORT_ADD(pbmp, access_ports[i]);
    fe_vlan_new(feunit, vlan, pbmp, pbmp);
    qe_vlan_new(qeunit, vlan, pbmp, pbmp);
    printf("VLAN: 0x%x created on port 0x%x\n", vlan, access_ports[i]);

    /* create L3 interface on the vlan */
    bcm_l3_intf_t* l3if = &access_l3if[i];
    bcm_l3_intf_t_init(l3if);
    l3if->l3a_mac_addr = access_lsmac;
    l3if->l3a_vid = vlan;
    l3if->l3a_ttl = access_ttl_limit;
    rc = bcm_l3_intf_create (feunit, l3if);
    if (BCM_FAILURE(rc)) {
      printf("BCM FAIL %d: %s\n", rc, bcm_errmsg(rc));
      return CMD_FAIL;
    }
    printf("L3 interface created, MAC 0x%02x:0x%02x:0x%02x:0x%02x:0x%02x:0x%02x on VLAN 0x%x\n",
           l3if->l3a_mac_addr[0], 
           l3if->l3a_mac_addr[1], 
           l3if->l3a_mac_addr[2], 
           l3if->l3a_mac_addr[3], 
           l3if->l3a_mac_addr[4], 
           l3if->l3a_mac_addr[5], vlan);

    /* create L3 egress for the vlan, port */
    bcm_l3_egress_t *l3eg = &access_l3eg[i];
    bcm_if_t *l3eg_path_id = &access_egif[i];
    bcm_l3_egress_t_init(l3eg);
    l3eg->intf = l3if->l3a_intf_id;
    access_mac_get(access_ports[i], l3eg->mac_addr);
    l3eg->vlan = vlan;
    l3eg->module = femod;
    l3eg->port = access_ports[i];
    rc = bcm_l3_egress_create(feunit, 0, l3eg, l3eg_path_id);
    if (BCM_FAILURE(rc)) {
      printf("BCM FAIL %d: %s\n", rc, bcm_errmsg(rc));
      return CMD_FAIL;
    }
    printf("L3 egress created, MAC 0x%02x:0x%02x:0x%02x:0x%02x:0x%02x:0x%02x on VLAN 0x%x\n",
           l3eg->mac_addr[0], 
           l3eg->mac_addr[1], 
           l3eg->mac_addr[2], 
           l3eg->mac_addr[3], 
           l3eg->mac_addr[4], 
           l3eg->mac_addr[5], vlan);

  }

}

ecmp_test_config();


