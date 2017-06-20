/*
 *  MPLS script 2009-sept-29 
 *
 * $Id: mpls_cint.c,v 1.4 Broadcom SDK $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


/*
 * Global Configuration variables
 */

int uni_port = 24;
int nni_port = 25;

/* VLAN IDs for MPLS LER configuration */
int nni_vid = 2;
int uni_vid = 3;

/* VLAN IDs for MPLS PWE3 configuration */
int core_vid = 7;
int eth_vid  = 9;

bcm_mac_t  dmac_address_0 = {0x00, 0x4e, 0xa5, 0x00, 0x10, 0x01};
bcm_mac_t  smac_address_0 = {0x00, 0x4e, 0xa5, 0x00, 0x10, 0x00};
bcm_mac_t  dmac_address_1 = {0x00, 0x4e, 0xa5, 0x00, 0x40, 0x01};
bcm_mac_t  smac_address_1 = {0x00, 0x4e, 0xa5, 0x00, 0x40, 0x00};

bcm_mac_t  lerdmac = "00:4e:a5:F0:10:01";
bcm_mac_t  ipsmac  = "00:0a:0b:0c:0d:0e";

/* LER Labels */
int ingLerLabel = 3000;
int egrLerLabel = 2000;

bcm_ip_t   ipaddr     = 0x01010100;
bcm_ip_t   nni_ipaddr = 0x02020200;
bcm_ip_t   ipmask     = 0xffffff00;

/* PWE3 Labels */
int vc_label     = 0x2000;
int match_label  = 0x2000;
int tunnel_label = 0x4300;

int cir_rate  = 600000;
int cir_burst = 1500;
int pir_rate  = 600000;
int pir_burst = 1500;


int FP_PWE_ACL_ENTRY     = 4;
int FP_LER_SIP_ACL_ENTRY = 5;
int FP_IP_ACL_ENTRY      = 6;
int FP_LER_ACL_ENTRY     = 7;
int FP_L2_ACL_ENTRY      = 8;
int FP_IP_POL_ENTRY      = 10;
int FP_LER_POL_ENTRY     = 11;
int FP_LER_CTR_ENTRY     = 12;
int FP_IP_CTR_ENTRY      = 13;
int FP_IP_CTR_SMAC_ENTRY = 14;


bcm_field_qset_t qset_pol_ip;
bcm_field_qset_t qset_pol_ler;
bcm_field_qset_t qset_cos_ler;
bcm_field_qset_t qset_ctr_ler;
bcm_field_qset_t qset_cos_ler_sip;
bcm_field_qset_t qset_cos_ler_smac;
bcm_field_qset_t qset_ctr_ip_smac;
bcm_field_qset_t qset_ctr_ip;
bcm_field_qset_t qset_cos_l2;

bcm_field_qset_t qset_cos_pwe;

bcm_policer_t pol_id_ip;
bcm_policer_t pol_id_ler;
bcm_policer_t pol_id_l2;
bcm_policer_t pol_id_pwe;

int unit = 1;
int CMD_FAIL = -1;

bcm_module_t femod;    
bcm_stk_modid_get (unit, &femod);

uint64 lerctr;   /* {0x00000000 0x00000000} */
uint64 ipctr;    /* {0x00000000 0x00000000} */


/* Utility Functions */

int fe_port_to_fe_gport (int feport) {
    bcm_stk_modid_get (uint, &femod);
    BCM_GPORT_MODPORT_SET (fegport, &femod, &feport);
    return (fegport);
}

int config_mpls(void) {
  config_acl();
  config_mpls_pwe();
  config_mpls_ler();
  printf (" \n *** version 090929_v005 *** \n");
  return 0;
}

int config_acl(void) {
  fp_pol_ip(1);
  fp_pol_ler(1);
  fp_cos_pwe(1);
  fp_cos_l2(1);
  return 0;
}


/* MPLS LER function */
/* Set egress port on IP>LER edge */
int config_mpls_ler () {
  
  int rc; 

  bcm_if_t l3egid;
  bcm_if_t l3egidIp;

  int ipIntfId;
  int lerIntfId;

  int ttl_limit = 8;

  /* lersmac is the ingress DMAC for LER>IP */
  bcm_mac_t lersmac;
  lersmac = smac_address_0;


  rc = bcm_vlan_create (unit, nni_vid);
  if (BCM_FAILURE(rc)) {
    printf("BCM FAIL %d: %s\n", rc, bcm_errmsg(rc));
    return CMD_FAIL;
  }
  rc = bcm_vlan_create (unit, uni_vid);
  if (BCM_FAILURE(rc)) {
    printf("BCM FAIL %d: %s\n", rc, bcm_errmsg(rc));
    return CMD_FAIL;
  }

  /* LER label */
  bcm_mpls_label_t ingress_ler_label = ingLerLabel;
  bcm_mpls_label_t egress_ler_label  = egrLerLabel; 

  printf ("MPLS LER configuration \n");

  bcm_l3_intf_t ler_l3if;
  bcm_l3_intf_t_init(&ler_l3if);

  ler_l3if.l3a_mac_addr = lersmac;
  ler_l3if.l3a_vid      = nni_vid;
  ler_l3if.l3a_ttl      = 0x2;
  ler_l3if.l3a_vrf      = BCM_L3_VRF_DEFAULT;
  rc = bcm_l3_intf_create (unit, &ler_l3if);
  if (BCM_FAILURE(rc)) {
    printf("BCM FAIL %d: %s\n", rc, bcm_errmsg(rc));
    return CMD_FAIL;
  }

  ipIntfId = ler_l3if.l3a_intf_id;

  /* Configure BCM_L3_EGRESS_CREATE using ler_l3eg struct type bcm_l3_egress_t */

  bcm_l3_egress_t ler_l3eg;
  bcm_l3_egress_t_init(&ler_l3eg);

  ler_l3eg.intf     = ipIntfId;
  ler_l3eg.mac_addr = lerdmac;
  ler_l3eg.vlan     = nni_vid;
  ler_l3eg.module   = femod;
  ler_l3eg.port     = nni_port;
     
  rc = bcm_l3_egress_create (unit, 0, &ler_l3eg, &l3egid);
  if (BCM_FAILURE(rc)) {
    printf("BCM FAIL %d: %s\n", rc, bcm_errmsg(rc));
    return CMD_FAIL;
  }

  /* Configure the label on egress */

  bcm_mpls_egress_label_t  label_array;
  bcm_mpls_egress_label_t_init(&label_array);
  
  label_array.flags = BCM_MPLS_EGRESS_LABEL_EXP_COPY;
  label_array.label = egress_ler_label;
  label_array.ttl   = ttl_limit;

  rc = bcm_mpls_tunnel_initiator_set (unit, ipIntfId, 1, &label_array);
  if (BCM_FAILURE(rc)) {
    printf("BCM FAIL %d: %s\n", rc, bcm_errmsg(rc));
    return CMD_FAIL;
  }

  /* Configure the LSP on ingress */
  bcm_mpls_tunnel_switch_t ler_switch;
  bcm_mpls_tunnel_switch_t_init(&ler_switch);

  ler_switch.flags  = BCM_MPLS_SWITCH_OUTER_TTL;
  ler_switch.label  = ingress_ler_label;
  ler_switch.action = BCM_MPLS_SWITCH_ACTION_POP;
  ler_switch.vpn    = 0;
  
  rc = bcm_mpls_tunnel_switch_add (unit, ler_switch);
  if (BCM_FAILURE(rc)) {
    printf("BCM FAIL %d: %s\n", rc, bcm_errmsg(rc));
    return CMD_FAIL;
  }

  /* Add the route after creating the MPLS ete */

  bcm_l3_route_t ip_l3rt;
  bcm_l3_route_t_init(&ip_l3rt);

  ip_l3rt.l3a_subnet    = ipaddr;
  ip_l3rt.l3a_ip_mask   = ipmask;
  ip_l3rt.l3a_vrf       = BCM_L3_VRF_DEFAULT;
  ip_l3rt.l3a_intf      = l3egid;
  ip_l3rt.l3a_modid     = femod;
  ip_l3rt.l3a_port_tgid = nni_port;
  
  rc = bcm_l3_route_add (unit, ip_l3rt);
  if (BCM_FAILURE(rc)) {
      printf("BCM FAIL %d: %s\n", rc, bcm_errmsg(rc));
      return CMD_FAIL;
  }

  /* configure BCM_L3_INTF_CREATE for LER interface */
 
  bcm_l3_intf_t_init(ler_l3if);
  
  ler_l3if.l3a_mac_addr = lerdmac;
  ler_l3if.l3a_vid      = uni_vid;
  ler_l3if.l3a_ttl      = 0x2;
  ler_l3if.l3a_vrf      = BCM_L3_VRF_DEFAULT;
  
  rc = bcm_l3_intf_create (unit, ler_l3if);
  if (BCM_FAILURE(rc)) {
      printf("BCM FAIL %d: %s\n", rc, bcm_errmsg(rc));
      return CMD_FAIL;
  }  

  lerIntfId = ler_l3if.l3a_intf_id;

  /* Configure the IPv4 Ethernet interface */
  /* Configure BCM_L3_EGRESS_CREATE for ipv4 exit */

  bcm_l3_egress_t_init(&ler_l3eg);

  ler_l3eg.intf     = lerIntfId;
  ler_l3eg.mac_addr = ipsmac;
  ler_l3eg.vlan     = uni_vid;
  ler_l3eg.module   = femod;
  ler_l3eg.port     = uni_port;
  
  rc = bcm_l3_egress_create (unit, 0, &ler_l3eg, &l3egidIp);
  if (BCM_FAILURE(rc)) {
    printf("BCM FAIL %d: %s\n", rc, bcm_errmsg(rc));
    return CMD_FAIL;
  }

  /* Add l3 route */
  bcm_l3_route_t ler_l3rt;
  bcm_l3_route_t_init(&ler_l3rt);

  ler_l3rt.l3a_subnet    = nni_ipaddr;
  ler_l3rt.l3a_ip_mask   = ipmask;
  ler_l3rt.l3a_vrf       = BCM_L3_VRF_DEFAULT;
  ler_l3rt.l3a_intf      = l3egidIp;
  ler_l3rt.l3a_modid     = femod;
  ler_l3rt.l3a_port_tgid = uni_port;

  rc = bcm_l3_route_add (unit, &ler_l3rt);
  if (BCM_FAILURE(rc)) {
    printf("BCM FAIL %d: %s\n", rc, bcm_errmsg(rc));
    return CMD_FAIL;
  }
}


/*
 * PWE Script for VPWS services
 *
 */
int config_mpls_pwe () {

  int rc;

  int eth_port = uni_port;
  int mpls_port = nni_port;

  int mpls_port_vc_label     = vc_label;
  int mpls_port_match_label  = match_label;
  int mpls_port_tunnel_label = tunnel_label;
    
  int vpnid;

  int mpls_gportid;
  int gportid;

  bcm_l2_addr_t  gu2_test_pwe3_l2addr;

  bcm_mpls_vpn_config_t vpn_config;
  bcm_mpls_vpn_config_t_init(&vpn_config);
 
  printf ("MPLS PWE3 configuration \n");

  vpn_config.flags  = BCM_MPLS_VPN_VPWS;
  vpn_config.vpn    = 0;

  rc = bcm_mpls_vpn_id_create (unit, &vpn_config);
  if (BCM_FAILURE(rc)) {
    printf("BCM FAIL %d: %s\n", rc, bcm_errmsg(rc));
    return CMD_FAIL;
  }

  vpnid = vpn_config.vpn;
  printf ("VPN created for VPWS, id: 0x%x \n", vpnid);

  bcm_mpls_port_t  mport;

  /*
   *  create VPNs for each Edge 
   */

  bcm_mpls_vpn_config_t_init(&vpn_config);
 
  vpn_config.flags = BCM_MPLS_VPN_VPWS;
  vpn_config.vpn   = 0;
  
  rc = bcm_mpls_vpn_id_create (unit, vpn_config);
  if (BCM_FAILURE(rc)) {
    printf("BCM FAIL %d: %s\n", rc, bcm_errmsg(rc));
    return CMD_FAIL;
  }
    
  vpnid = vpn_config.vpn;
  printf ("VPN created for VPWS, id: 0x%x \n", vpnid);
      
  bcm_gport_t eth_gport, mpls_gport;

  BCM_GPORT_MODPORT_SET (eth_gport, &femod, eth_port);
  BCM_GPORT_MODPORT_SET (mpls_gport, &femod, mpls_port);          
  
  /* create L3 interface */

  bcm_mac_t pwemac = dmac_address_0;

  bcm_l3_intf_t  pwe_l3if;
  bcm_l3_intf_t_init(&pwe_l3if);
      
  pwe_l3if.l3a_mac_addr = pwemac;
  pwe_l3if.l3a_vid      = core_vid;
  pwe_l3if.l3a_ttl      = 0x2;
      
  rc = bcm_l3_intf_create (unit, &pwe_l3if);
  if (BCM_FAILURE(rc)) {
    printf("BCM FAIL %d: %s\n", rc, bcm_errmsg(rc));
    return CMD_FAIL;
  }

  int pweIntfId;
  pweIntfId = pwe_l3if.l3a_intf_id;
      
  /* Configure the egress MPLS Port */
  bcm_mac_t pwedmac = dmac_address_0;
  bcm_if_t pwe_egid;

  bcm_l3_egress_t pwe_l3eg;
  bcm_l3_egress_t_init(&pwe_l3eg);
      
  pwe_l3eg.intf     = pweIntfId;
  pwe_l3eg.mac_addr = pwedmac;
  pwe_l3eg.vlan     = core_vid;
  pwe_l3eg.module   = femod;
  pwe_l3eg.port     = mpls_port;
      
  rc = bcm_l3_egress_create (unit, 0, &pwe_l3eg, &pwe_egid);
  if (BCM_FAILURE(rc)) {
    printf("BCM FAIL %d: %s\n", rc, bcm_errmsg(rc));
    return CMD_FAIL;
  }
 
  printf ("Created egress_l3_intf: 0x%x \n", pwe_egid);

  int vc_label     = mpls_port_vc_label;
  int match_label  = mpls_port_match_label;
  int tunnel_label = mpls_port_tunnel_label;
      
  /* Primary Tunnel Label */

  bcm_mpls_egress_label_t label_array;
  bcm_mpls_egress_label_t_init(&label_array);

  label_array.flags = BCM_MPLS_EGRESS_LABEL_TTL_SET;
  label_array.label = tunnel_label;
  label_array.ttl   = 8;
  label_array.exp   = 0;
      
  /* VC Label is configured on the MPLS Port Add */

  bcm_mpls_egress_label_t vc_label_array;
  bcm_mpls_egress_label_t_init(&vc_label_array);

  vc_label_array.flags = BCM_MPLS_EGRESS_LABEL_TTL_SET;
  vc_label_array.label = 0x80000 | vc_label; 
  vc_label_array.ttl   = 31;
  vc_label_array.exp   = 0;

  /*
   *   The Tunnel is configured wth just the tunnel (outer) label
   *    set elabel_list [list $label_array $vc_label_array]
   */
   
  rc = bcm_mpls_tunnel_initiator_set (unit, pweIntfId, 1, &label_array);
  if (BCM_FAILURE(rc)) {
    printf("BCM FAIL %d: %s\n", rc, bcm_errmsg(rc));
    return CMD_FAIL;
  }

  /* add MPLS Port tunnel to VPN */
  bcm_mpls_port_t_init(&mport);

  mport.port        = mpls_gport;
  mport.criteria    = BCM_MPLS_PORT_MATCH_LABEL;
  mport.match_label = match_label;

  policer_enable_pwe();

  mport.policer_id       = pol_id_pwe;
  mport.flags            = (BCM_MPLS_PORT_EGRESS_TUNNEL | BCM_MPLS_PORT_NETWORK);
  mport.egress_tunnel_if = pwe_egid;
  mport.egress_label     = vc_label_array;

  int  lp_index = match_label + 0x4000;
  printf  ("Need this patch:   BCM_SHELL> 1: g2p3set lp lpi=0x%x policer=0x%x \n", lp_index, pol_id_pwe);
     
  rc = bcm_mpls_port_add (unit, vpnid, &mport);
  if (BCM_FAILURE(rc)) {
    printf("BCM FAIL %d: %s\n", rc, bcm_errmsg(rc));
    return CMD_FAIL;
  }

  mpls_gportid = mport.mpls_port_id;

  printf  ("Port %d label 0x%x with match label 0x%x added to VPN id: 0x%x \n", 
		mpls_port, tunnel_label, match_label, vpnid); 

  /* add Ethernet Port to VPN */
  bcm_mpls_port_t_init(&mport);

  mport.port       = eth_gport;
  mport.criteria   = BCM_MPLS_PORT_MATCH_PORT_VLAN;
  mport.match_vlan = eth_vid;

  policer_enable_l2();
  
  mport.policer_id = pol_id_l2;

  /* 
   *  When adding Failover: 
   *    mport.failover_id  = fo;
   *    mport.failover_port_id = mpls_prot_gportid;
   */

  rc = bcm_mpls_port_add (unit, vpnid, &mport);
  if (BCM_FAILURE(rc)) {
    printf("BCM FAIL %d: %s\n", rc, bcm_errmsg(rc));
    return CMD_FAIL;
  }

  gportid = mport.mpls_port_id;

  printf ("Port %d vid 0x%x added to VPN id: 0x%x \n", eth_port, eth_vid, vpnid); 
      
  rc = bcm_vlan_stp_set (unit, $vpnid, $gportid, BCM_STG_STP_FORWARD);
  if (BCM_FAILURE(rc)) {
    printf("BCM FAIL %d: %s\n", rc, bcm_errmsg(rc));
    return CMD_FAIL;
  }
}


int policer_enable_ip (void) {
  int rc;
  bcm_policer_config_t pol_cfg;
  bcm_policer_config_t_init(&pol_cfg);

  pol_cfg.flags        = BCM_POLICER_DROP_RED;
  pol_cfg.mode         = bcmPolicerModeSrTcm;
  pol_cfg.ckbits_sec   = cir_rate;
  pol_cfg.ckbits_burst = cir_burst;
  pol_cfg.pkbits_sec   = pir_rate;
  pol_cfg.pkbits_burst = pir_burst;

  rc = bcm_policer_create (unit, &pol_cfg, &pol_id_ip);
  if (BCM_FAILURE(rc)) {
    printf("BCM FAIL %d: %s\n", rc, bcm_errmsg(rc));
    return CMD_FAIL;
  }

  printf("IPv4 UNI: created pol_id: %x \n", pol_id_ip);
}

int policer_enable_ler (void) {
  int rc;
  bcm_policer_config_t pol_cfg;
  bcm_policer_config_t_init(&pol_cfg);

  pol_cfg.flags        = BCM_POLICER_DROP_RED;
  pol_cfg.mode         = bcmPolicerModeSrTcm;
  pol_cfg.ckbits_sec   = cir_rate;
  pol_cfg.ckbits_burst = cir_burst;
  pol_cfg.pkbits_sec   = pir_rate;
  pol_cfg.pkbits_burst = pir_burst;

  rc = bcm_policer_create (unit, &pol_cfg, &pol_id_ler);
  if (BCM_FAILURE(rc)) {
    printf("BCM FAIL %d: %s\n", rc, bcm_errmsg(rc));
    return CMD_FAIL;
  }

  printf("LER NNI: created pol_id: %x \n", pol_id_ler);
}

int policer_enable_l2 (void) {
  int rc;
  bcm_policer_config_t  pol_cfg;
  bcm_policer_config_t_init(&pol_cfg);

  pol_cfg.flags        = BCM_POLICER_DROP_RED;
  pol_cfg.mode         = bcmPolicerModeSrTcm;
  pol_cfg.ckbits_sec   = cir_rate;
  pol_cfg.ckbits_burst = cir_burst;
  pol_cfg.pkbits_sec   = pir_rate;
  pol_cfg.pkbits_burst = pir_burst;

  rc = bcm_policer_create (unit, &pol_cfg, &pol_id_l2);
  if (BCM_FAILURE(rc)) {
    printf("BCM FAIL %d: %s\n", rc, bcm_errmsg(rc));
    return CMD_FAIL;
  }

  printf("L2 UNI: created pol_id: %x \n", pol_id_l2);
}


int policer_enable_pwe () {
  int rc;
  bcm_policer_config_t pwe_pol_cfg;
  bcm_policer_config_t_init(&pwe_pol_cfg);

  pwe_pol_cfg.flags        = BCM_POLICER_DROP_RED;
  pwe_pol_cfg.mode         = bcmPolicerModeSrTcm;
  pwe_pol_cfg.ckbits_sec   = cir_rate;
  pwe_pol_cfg.ckbits_burst = cir_burst;
  pwe_pol_cfg.pkbits_sec   = pir_rate;
  pwe_pol_cfg.pkbits_burst = pir_burst;

  rc = bcm_policer_create (unit, &pwe_pol_cfg,  &pol_id_pwe);
  if (BCM_FAILURE(rc)) {
    printf("BCM FAIL %d: %s\n", rc, bcm_errmsg(rc));
    return CMD_FAIL;
  }

  printf("PWE3 NNI: created pol_id: %x \n", pol_id_pwe);
}


int policer_rate_pwe (int rate) {
  int rc;
  pwe_pol_cfg.flags        = BCM_POLICER_DROP_RED;
  pwe_pol_cfg.mode         = bcmPolicerModeSrTcm;
  pwe_pol_cfg.ckbits_sec   = rate;
  pwe_pol_cfg.ckbits_burst = rate;
  pwe_pol_cfg.pkbits_sec   = rate;
  pwe_pol_cfg.pkbits_burst = rate;
    
  rc = bcm_policer_set (unit, &pol_id_pwe, &pwe_pol_cfg);
  if (BCM_FAILURE(rc)) {
    printf("BCM FAIL %d: %s\n", rc, bcm_errmsg(rc));
    return CMD_FAIL;
  }
}


int policer_rate_l2 (int rate) {
  int rc;
  bcm_policer_config_t pol_cfg;
  bcm_policer_config_t_init(&pol_cfg);
  
  pol_cfg.flags        = BCM_POLICER_DROP_RED;
  pol_cfg.mode         = bcmPolicerModeSrTcm;
  pol_cfg.ckbits_sec   = rate;
  pol_cfg.ckbits_burst = rate;
  pol_cfg.pkbits_sec   = rate;
  pol_cfg.pkbits_burst = rate;
    
  rc = bcm_policer_set (unit, &pol_id_l2, &pol_cfg);
  if (BCM_FAILURE(rc)) {
    printf("BCM FAIL %d: %s\n", rc, bcm_errmsg(rc));
    return CMD_FAIL;
  }
}


int policer_rate_ler (int rate)  {
  int rc;
  bcm_policer_config_t pol_cfg;
  bcm_policer_config_t_init(&pol_cfg);
  
  pol_cfg.flags        = BCM_POLICER_DROP_RED;
  pol_cfg.mode         = bcmPolicerModeSrTcm;
  pol_cfg.ckbits_sec   = rate;
  pol_cfg.ckbits_burst = rate;
  pol_cfg.pkbits_sec   = rate;
  pol_cfg.pkbits_burst = rate;
    
  rc = bcm_policer_set (unit, &pol_id_ler, &pol_cfg);
  if (BCM_FAILURE(rc)) {
    printf("BCM FAIL %d: %s\n", rc, bcm_errmsg(rc));
    return CMD_FAIL;
  }
}


int policer_rate_ip (int rate)  {
  int rc;
  bcm_policer_config_t pol_cfg;
  bcm_policer_config_t_init(&pol_cfg);

  pol_cfg.flags        = BCM_POLICER_DROP_RED;
  pol_cfg.mode         = bcmPolicerModeSrTcm;
  pol_cfg.ckbits_sec   = rate;
  pol_cfg.ckbits_burst = rate;
  pol_cfg.pkbits_sec   = rate;
  pol_cfg.pkbits_burst = rate;
    
  rc = bcm_policer_set (unit, &pol_id_ip, &pol_cfg);
  if (BCM_FAILURE(rc)) {
    printf("BCM FAIL %d: %s\n", rc, bcm_errmsg(rc));
    return CMD_FAIL;
  }
}

/* Field Processor Configuration */
int fp_pol_ler (int cos) {
    bcm_mac_t dmac = lerdmac;
    bcm_mac_t smac = ipsmac;
    bcm_mac_t mac_mask = "ff:ff:ff:ff:ff:ff";

    bcm_field_qset_t_init(&qset_pol_ler);

    bcm_field_group_create_id      (unit, qset_pol_ler, 3, FP_LER_POL_ENTRY);
    bcm_field_entry_create_id      (unit, FP_LER_POL_ENTRY, FP_LER_POL_ENTRY);
    bcm_field_qualify_SrcMac       (unit, FP_LER_POL_ENTRY, smac, mac_mask);
    bcm_field_action_add           (unit, FP_LER_POL_ENTRY, bcmFieldActionCosQNew, cos, cos);

    policer_enable_ler();
    bcm_field_entry_policer_attach (unit, FP_LER_POL_ENTRY, 0, pol_id_ler);
    bcm_field_entry_install        (unit, FP_LER_POL_ENTRY);
}

int fp_cosq_ler (int cos) {
    bcm_mac_t dmac = lerdmac;
    bcm_mac_t smac = ipsmac;
    bcm_mac_t mac_mask = "ff:ff:ff:ff:ff:ff";

    bcm_field_qset_t_init(&qset_cos_ler);

    bcm_field_group_create_id   (unit, qset_cos_ler, 1, FP_LER_ACL_ENTRY);
    bcm_field_entry_create_id   (unit, FP_LER_ACL_ENTRY,  FP_LER_ACL_ENTRY);
    bcm_field_qualify_DstMac    (unit, FP_LER_ACL_ENTRY, smac, mac_mask);
    bcm_field_action_add        (unit, FP_LER_ACL_ENTRY, bcmFieldActionCosQNew, cos, cos);
    bcm_field_entry_install     (unit, FP_LER_ACL_ENTRY);
}


int fp_ctr_ler (int cos) {
    bcm_mac_t mac_smac = ipsmac;
    bcm_mac_t mac_mask = "ff:ff:ff:ff:ff:ff";
 
    bcm_field_qset_t_init(&qset_ctr_ler);

    bcm_field_group_create_id   (unit, qset_ctr_ler, 6,  FP_LER_CTR_ENTRY);
    bcm_field_entry_create_id   (unit, FP_LER_CTR_ENTRY,  FP_LER_CTR_ENTRY);
    bcm_field_qualify_SrcMac    (unit, FP_LER_CTR_ENTRY, mac_smac,  mac_mask);
    bcm_field_action_add        (unit, FP_LER_CTR_ENTRY, bcmFieldActionUpdateCounter,  cos, cos);
    bcm_field_entry_install     (unit, FP_LER_CTR_ENTRY);
}


int fp_ler_sip (int cos) {
    bcm_ip_t sip = 0x01010101;
    bcm_ip_t mask = 0xffffffff;
    
    int ethtype = 0x8847;
    int etmask  = 0xffff;

    bcm_field_qset_t_init(&qset_cos_ler_sip);

    bcm_field_group_create_id   (unit, qset_cos_ler_sip, 4, FP_LER_SIP_ACL_ENTRY);
    bcm_field_entry_create_id   (unit, FP_LER_SIP_ACL_ENTRY, FP_LER_SIP_ACL_ENTRY);
    bcm_field_qualify_SrcIp     (unit, FP_LER_SIP_ACL_ENTRY, sip, mask);
    bcm_field_qualify_EtherType (unit, FP_LER_SIP_ACL_ENTRY, ethtype, etmask);


    bcm_field_action_add        (unit, FP_LER_SIP_ACL_ENTRY, bcmFieldActionCosQNew, cos, cos);
    bcm_field_entry_install     (unit, FP_LER_SIP_ACL_ENTRY);
}



int fp_ler_smac (int cos) {
    bcm_mac_t smac_addr = ipsmac;
    bcm_mac_t mac_mask = "ff:ff:ff:ff:ff:ff";

    bcm_field_qset_t_init(&qset_cos_ler_smac);

    bcm_field_group_create_id   (unit, qset_cos_ler_smac, 4, FP_LER_SIP_ACL_ENTRY);
    bcm_field_entry_create_id   (unit, FP_LER_SIP_ACL_ENTRY, FP_LER_SIP_ACL_ENTRY);
    bcm_field_qualify_SrcMac    (unit, FP_LER_SIP_ACL_ENTRY, smac_addr, mac_mask);
    bcm_field_action_add        (unit, FP_LER_SIP_ACL_ENTRY, bcmFieldActionCosQNew, cos, cos);
    bcm_field_entry_install     (unit, FP_LER_SIP_ACL_ENTRY);
}


int fp_ctr_ip_smac (int cos) {
    bcm_mac_t smac = "00:10:94:00:00:01";
    bcm_mac_t mac_mask = "ff:ff:ff:ff:ff:ff";

    bcm_field_qset_t_init(&qset_ctr_ip_smac);

    bcm_field_group_create_id (unit, qset_ctr_ip_smac, 7, FP_IP_CTR_SMAC_ENTRY);
    bcm_field_entry_create_id (unit, FP_IP_CTR_SMAC_ENTRY, FP_IP_CTR_SMAC_ENTRY);
    bcm_field_qualify_SrcMac  (unit, FP_IP_CTR_SMAC_ENTRY, smac_addr, mac_mask);
    bcm_field_action_add      (unit, FP_IP_CTR_SMAC_ENTRY, bcmFieldActionUpdateCounter, cos, cos);

    bcm_field_entry_install   (unit, FP_IP_CTR_SMAC_ENTRY);
}



int fp_pol_ip (int cos) {
    bcm_ip_t   sip  = 0x02020202;
    bcm_ip_t   mask = 0xffffffff;

    policer_enable_ip();
    bcm_field_qset_t_init (qset_pol_ip);

    bcm_field_group_create_id      (unit, qset_pol_ip, 2, FP_IP_POL_ENTRY);
    bcm_field_entry_create_id      (unit, FP_IP_POL_ENTRY, FP_IP_POL_ENTRY);
    bcm_field_qualify_SrcIp        (unit, FP_IP_POL_ENTRY, sip, mask);
    bcm_field_action_add           (unit, FP_IP_POL_ENTRY, bcmFieldActionCosQNew, cos, cos);
    bcm_field_entry_policer_attach (unit, FP_IP_POL_ENTRY, 0, pol_id_ip);
    bcm_field_entry_install        (unit, FP_IP_POL_ENTRY);

}

int fp_cos_l2 (int cos) {
    bcm_mac_t   mac_addr = "00:00:00:00:00:02";
    bcm_mac_t   mac_mask = "ff:ff:ff:ff:ff:ff";
    
    bcm_field_qset_t_init(&qset_cos_l2);

    bcm_field_group_create_id (unit, qset_cos_l2,  5, FP_L2_ACL_ENTRY);
    bcm_field_entry_create_id (unit, FP_L2_ACL_ENTRY, FP_L2_ACL_ENTRY);
    bcm_field_qualify_DstMac  (unit, FP_L2_ACL_ENTRY, mac_addr, mac_mask);
    bcm_field_action_add      (unit, FP_L2_ACL_ENTRY, bcmFieldActionCosQNew, cos, cos);
    bcm_field_entry_install   (unit, FP_L2_ACL_ENTRY);
}


int fp_cos_pwe (int cos) {
    /* inner mac */
    bcm_mac_t   mac_addr = "00:10:94:00:00:02";
    bcm_mac_t   mac_mask = "ff:ff:ff:ff:ff:ff";

    bcm_field_qset_t_init(&qset_cos_pwe);

    bcm_field_group_create_id  (unit, qset_cos_pwe, 1, FP_PWE_ACL_ENTRY);
    bcm_field_entry_create_id  (unit, FP_PWE_ACL_ENTRY, FP_PWE_ACL_ENTRY);
    bcm_field_qualify_SrcMac   (unit, FP_PWE_ACL_ENTRY, mac_addr, mac_mask);
    bcm_field_action_add       (unit, FP_PWE_ACL_ENTRY, bcmFieldActionCosQNew, cos, cos);
    bcm_field_entry_install    (unit, FP_PWE_ACL_ENTRY);
}


