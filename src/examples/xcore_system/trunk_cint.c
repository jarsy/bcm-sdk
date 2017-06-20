/*
 *  Trunk script 2009-sept-29 
 *
 * $Id: trunk_cint.c,v 1.5 Broadcom SDK $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


/*
 * Global Configuration variables
 */

/* non-trunk vlan ports */
int vlan_port_count = 1;
int vlan_ports[vlan_port_count] = { 0 };

/* Trunk information */
bcm_trunk_add_info_t trunk_info;
int trunk_port_count = 2;
int trunk_ports[trunk_port_count] = { 11, 12 };
int trunk_designate_port = trunk_ports[0];

int trunk_vid = 2;
int trunk_id = -1;

/* Base MAC address for trunk L2 addresses */
bcm_mac_t  trunk_mac_base = {0x00, 0x4e, 0xa5, 0x00, 0x00, 0x00};

int feunit = 1;
int qeunit = 0;
int CMD_FAIL = -1;

bcm_module_t femod;    
bcm_stk_modid_get (feunit, &femod);
bcm_module_t qemod;
bcm_stk_modid_get (qeunit, &qemod);

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

/* construct MAC address based on trunk ID and port number */
int trunk_mac_get(int trunk_id, int port, char* mac) {
    /*sal_memcpy(mac, &trunk_mac_base, sizeof(trunk_mac_base));*/
    mac[0] = trunk_mac_base[0];
    mac[1] = trunk_mac_base[1];
    mac[2] = trunk_mac_base[2];
    mac[3] = trunk_mac_base[3];
    mac[4] = trunk_id;
    mac[5] = port;
    return 0;

}

int config_trunk(int *trunk_id, int port_count, int* port_list, int psc, int flags) {
  int i;
  int rc;

  /* create trunk */
  bcm_trunk_add_info_t_init(&trunk_info);
  trunk_info.psc = psc;
  trunk_info.flags = flags;

  /* add ports */
  trunk_info.num_ports = port_count;
  for (i=0; i < port_count; i++) {
    trunk_info.tp[i] = trunk_ports[i];
    trunk_info.tm[i] = femod;
  }

  /* create trunk */
  if (*trunk_id == BCM_TRUNK_INVALID) {
    rc = bcm_trunk_create(feunit, 0, trunk_id);
  } else {
    rc = bcm_trunk_create(feunit, BCM_TRUNK_FLAG_WITH_ID, trunk_id);
  }
  if (BCM_FAILURE(rc)) {
    printf("BCM FAIL %d: %s\n", rc, bcm_errmsg(rc));
    return CMD_FAIL;
  }
  printf ("Trunk 0x%x created\n", *trunk_id);

  rc = bcm_trunk_set(feunit, *trunk_id, trunk_info);
  if (BCM_FAILURE(rc)) {
    printf("BCM FAIL %d: %s\n", rc, bcm_errmsg(rc));
    return CMD_FAIL;
  }

  return 0;
}

int trunk_fe_vlan_new(int fe, int vlan, bcm_pbmp_t pbmp, bcm_pbmp_t u_pbmp) {

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

  printf("Vlan 0x%x created\n", vlan);
  return 0;
}

int trunk_qe_vlan_new (int qe, int vlan, bcm_pbmp_t pbmp, bcm_pbmp_t u_pbmp) {
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

int trunk_test_config() {
  int rc;
  bcm_pbmp_t pbmp;
  bcm_pbmp_t u_pbmp;
  int i;
  bcm_l2_addr_t t_l2addr;
  bcm_mac_t t_mac; 

  printf (" \n *** version 090929_v005 *** \n");

  /* create trunk on FE */
  trunk_id = -1;
  config_trunk(trunk_id, trunk_port_count, trunk_ports, BCM_TRUNK_PSC_DEFAULT, 0);

  /* create port bitmaps for FE 
   * ...includes all trunk member ports 
   */
  BCM_PBMP_CLEAR(pbmp);
  BCM_PBMP_CLEAR(u_pbmp);
  for (i=0; i < vlan_port_count; i++) {
    BCM_PBMP_PORT_ADD(pbmp, vlan_ports[i]);
    BCM_PBMP_PORT_ADD(u_pbmp, vlan_ports[i]);
  }
  for (i=0; i < trunk_port_count; i++) {
    BCM_PBMP_PORT_ADD(pbmp, trunk_ports[i]);
    BCM_PBMP_PORT_ADD(u_pbmp, trunk_ports[i]);
  }

  trunk_fe_vlan_new(feunit, trunk_vid, pbmp, u_pbmp);

  /* setup multicast group on the QE
   * ...only designate port of the trunk is included
   */
  BCM_PBMP_CLEAR(pbmp);
  BCM_PBMP_CLEAR(u_pbmp);
  for (i=0; i < vlan_port_count; i++) {
    BCM_PBMP_PORT_ADD(pbmp, vlan_ports[i]);
    BCM_PBMP_PORT_ADD(u_pbmp, vlan_ports[i]);
  }
  BCM_PBMP_PORT_ADD(pbmp, trunk_designate_port);
  BCM_PBMP_PORT_ADD(u_pbmp, trunk_designate_port);

  trunk_qe_vlan_new(qeunit, trunk_vid, pbmp, u_pbmp);

  /* add known L2 addresses to trunk */
  for (i=0; i < trunk_port_count; i++) {
    trunk_mac_get(trunk_id, trunk_ports[i], t_mac);
    printf("  adding MAC 0x%02x:0x%02x:0x%02x:0x%02x:0x%02x:0x%02x on VLAN 0x%x\n",
           t_mac[0], t_mac[1], t_mac[2], t_mac[3], t_mac[4], t_mac[5], trunk_vid);
    bcm_l2_addr_t_init(&t_l2addr, t_mac, trunk_vid);
    t_l2addr.tgid = trunk_id;
    t_l2addr.modid = femod;
    t_l2addr.flags =  BCM_L2_STATIC |  BCM_L2_TRUNK_MEMBER;
    rc = bcm_l2_addr_add(feunit, t_l2addr);
    if (BCM_FAILURE(rc)) {
      printf("BCM FAIL %d: %s\n", rc, bcm_errmsg(rc));
      return CMD_FAIL;
    }
  }

}

int trunk_test_unconfig() {
    int rc;

    rc = bcm_vlan_destroy(feunit, trunk_vid);
    if (BCM_FAILURE(rc)) {
      printf("BCM FAIL %d: %s\n", rc, bcm_errmsg(rc));
    }
    rc = bcm_trunk_destroy(feunit, trunk_id);
    if (BCM_FAILURE(rc)) {
      printf("BCM FAIL %d: %s\n", rc, bcm_errmsg(rc));
    }
    rc = bcm_multicast_destroy(qeunit, trunk_vid);
    if (BCM_FAILURE(rc)) {
      printf("BCM FAIL %d: %s\n", rc, bcm_errmsg(rc));
    }

    return 0;
}

