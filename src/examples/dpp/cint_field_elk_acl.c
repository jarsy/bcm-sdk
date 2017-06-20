
/* $Id: cint_field_elk_acl.c $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/

/* 
 * Using the KBP device as external TCAM lookups (ELK), this CINT describes 
 * a configuration examples of an ELK ACL. 
 *  
 * larg Qset 
 * DIP SIP sharing 
 * configure pmf to dropp packet that was hit in the kbp 
 *  
 */


bcm_field_group_t group_id_elk = 100;
bcm_field_group_t group_id_elk_sharing = 101;

int ACL_BASE_DQ_ID = 50;
int ACL_BASE_GROUP_INDEX = 0;
int PMF_BASE_GROUP_INDEX = 6;
int ACL_BASE_GROUP_PRIORITY = 40;
int PMF_BASE_GROUP_PRIORITY = 40;

int HIT_BIT_0_QUAL = 0;
int HIT_BIT_1_QUAL = 1;
int HIT_BIT_2_QUAL = 2;
int HIT_BIT_3_QUAL = 3;
int HIT_BIT_4_QUAL = 4;
int HIT_BIT_5_QUAL = 5;


/* this function configures in the KBP ACL with key size 160bit.
   this test validates that we are using the HW capabilities*/
int larg_qset(int unit) {
  int rv=0;
  int group_priority_elk = 41;
  bcm_field_group_config_t grp_elk;
  bcm_field_entry_t ent_elk;
  int presel_id_elk = 0; /* ID from 0 to 11 for Advanced mode */ 
  int presel_flags = 0; 

  /* Create Preselector */ 
  if(soc_property_get(unit, spn_FIELD_PRESEL_MGMT_ADVANCED_MODE, FALSE)) { 
      presel_flags = BCM_FIELD_QUALIFY_PRESEL_ADVANCED_MODE_STAGE_EXTERNAL; 
      rv = bcm_field_presel_create_stage_id(unit, bcmFieldStageExternal, presel_id_elk); 
      if (BCM_E_NONE != rv) { 
          printf("Error in bcm_field_presel_create_stage_id\n"); 
          return rv; 
      } 
  } else { 
      rv = bcm_field_presel_create(unit, &presel_id_elk); 
      if (BCM_E_NONE != rv) { 
          printf("Error in bcm_field_presel_create\n"); 
          return rv; 
      }
  }

  rv = bcm_field_qualify_Stage(unit, presel_id_elk | BCM_FIELD_QUALIFY_PRESEL | presel_flags, bcmFieldStageExternal); 
  if (BCM_E_NONE != rv) { 
      printf("Error in bcm_field_qualify_Stage\n"); 
      return rv; 
  } 

  rv = bcm_field_qualify_AppType(unit, presel_id_elk | BCM_FIELD_QUALIFY_PRESEL | presel_flags, bcmFieldAppTypeL2);
  if (rv) {
      printf("bcm_field_qualify_AppType error\n");
      return rv;
  }

  rv = bcm_field_qualify_AppType(unit, presel_id_elk | BCM_FIELD_QUALIFY_PRESEL | presel_flags, bcmFieldAppTypeIp4Ucast);
  if (rv) {
      printf("bcm_field_qualify_AppType error\n");
      return rv;
  }

  rv = bcm_field_qualify_AppType(unit, presel_id_elk | BCM_FIELD_QUALIFY_PRESEL | presel_flags, bcmFieldAppTypeIp4UcastRpf);
  if (rv) {
      printf("bcm_field_presel_create error\n");
      return rv;
  }
  
  bcm_field_group_config_t_init(&grp_elk); 
  BCM_FIELD_PRESEL_ADD(grp_elk.preselset, presel_id_elk);
  grp_elk.group = group_id_elk;
  grp_elk.priority = group_priority_elk ;
  grp_elk.mode =  bcmFieldGroupModeAuto;
  grp_elk.flags = BCM_FIELD_GROUP_CREATE_WITH_MODE | BCM_FIELD_GROUP_CREATE_WITH_ASET | BCM_FIELD_GROUP_CREATE_WITH_PRESELSET | BCM_FIELD_GROUP_CREATE_WITH_ID;
  
  BCM_FIELD_QSET_INIT(grp_elk.qset);
  BCM_FIELD_QSET_ADD(grp_elk.qset, bcmFieldQualifyStageExternal);
  BCM_FIELD_QSET_ADD(grp_elk.qset, bcmFieldQualifyVpn);
  BCM_FIELD_QSET_ADD(grp_elk.qset, bcmFieldQualifyIp4);
  BCM_FIELD_QSET_ADD(grp_elk.qset, bcmFieldQualifySrcIp);
  BCM_FIELD_QSET_ADD(grp_elk.qset, bcmFieldQualifyDstIp);
  BCM_FIELD_QSET_ADD(grp_elk.qset, bcmFieldQualifyL4SrcPort);
  BCM_FIELD_QSET_ADD(grp_elk.qset, bcmFieldQualifyL4DstPort);
  BCM_FIELD_QSET_ADD(grp_elk.qset, bcmFieldQualifyIpProtocol);
  BCM_FIELD_QSET_ADD(grp_elk.qset, bcmFieldQualifySrcPort);
  BCM_FIELD_QSET_ADD(grp_elk.qset, bcmFieldQualifyIpFrag);
  
  BCM_FIELD_ASET_INIT(grp_elk.aset);
  BCM_FIELD_ASET_ADD(grp_elk.aset, bcmFieldActionExternalValue3Set);

  rv = bcm_field_group_config_create(unit, &grp_elk);
  if (rv) {
      printf("bcm_field_group_config_create error\n");
      return rv;
  }

  /* Initialize the KBP */
  bshell(unit, "kbp deinit_appl");
  bshell(unit, "kbp init_appl");

  return rv;
}

int is_qax(int unit)
{
  bcm_info_t info;

  int rv = bcm_info_get(unit, &info);
  if (rv != BCM_E_NONE) {
      printf("Error in bcm_info_get\n");
      return rv;
  }

  return (info.device == 0x8470 ? 1 : 0);
}


/* this function configures the KBP with ACL key bigger than 160bit.
   DIP SIP sharing will not work in this case because the AppTypes are not at the same cluster IPv4 and IPv6*/
int dip_sip_sharing_elk_group_create_fail1(int unit) {
  int rv=0;
  int group_priority_elk = 42;
  bcm_field_group_config_t grp_elk;
  bcm_field_entry_t ent_elk;
  int presel_id_elk = 0; /* ID from 0 to 11 for Advanced mode */ 
  int presel_flags = 0; 

  /* Create Preselector */ 
  if(soc_property_get(unit, spn_FIELD_PRESEL_MGMT_ADVANCED_MODE, FALSE)) { 
      presel_flags = BCM_FIELD_QUALIFY_PRESEL_ADVANCED_MODE_STAGE_EXTERNAL; 
      rv = bcm_field_presel_create_stage_id(unit, bcmFieldStageExternal, presel_id_elk); 
      if (BCM_E_NONE != rv) { 
          printf("Error in bcm_field_presel_create_stage_id\n"); 
          return rv; 
      } 
  } else { 
      rv = bcm_field_presel_create(unit, &presel_id_elk); 
      if (BCM_E_NONE != rv) { 
          printf("Error in bcm_field_presel_create\n"); 
          return rv; 
      }
  }

  rv = bcm_field_qualify_Stage(unit, presel_id_elk | BCM_FIELD_QUALIFY_PRESEL | presel_flags, bcmFieldStageExternal); 
  if (BCM_E_NONE != rv) { 
      printf("Error in bcm_field_qualify_Stage\n"); 
      return rv; 
  } 

  rv = bcm_field_qualify_AppType(unit, presel_id_elk | BCM_FIELD_QUALIFY_PRESEL | presel_flags | presel_flags, bcmFieldAppTypeIp6Ucast);
  if (rv) {
      printf("bcm_field_qualify_AppType error\n");
      return rv;
  }

  rv = bcm_field_qualify_AppType(unit, presel_id_elk | BCM_FIELD_QUALIFY_PRESEL | presel_flags, bcmFieldAppTypeIp4Ucast);
  if (rv) {
      printf("bcm_field_qualify_AppType error\n");
      return rv;
  }
  
  bcm_field_group_config_t_init(&grp_elk); 
  BCM_FIELD_PRESEL_ADD(grp_elk.preselset, presel_id_elk);
  grp_elk.group = group_id_elk_sharing;
  grp_elk.priority = group_priority_elk ;
  grp_elk.mode =  bcmFieldGroupModeAuto;
  grp_elk.flags = BCM_FIELD_GROUP_CREATE_WITH_MODE | BCM_FIELD_GROUP_CREATE_WITH_ASET | BCM_FIELD_GROUP_CREATE_WITH_PRESELSET | BCM_FIELD_GROUP_CREATE_WITH_ID;
  
  BCM_FIELD_QSET_INIT(grp_elk.qset);
  BCM_FIELD_QSET_ADD(grp_elk.qset, bcmFieldQualifyStageExternal);
  BCM_FIELD_QSET_ADD(grp_elk.qset, bcmFieldQualifyDstIp6);
  BCM_FIELD_QSET_ADD(grp_elk.qset, bcmFieldQualifySrcIp6);
  BCM_FIELD_QSET_ADD(grp_elk.qset, bcmFieldQualifySrcPort);
  
  
  BCM_FIELD_ASET_INIT(grp_elk.aset);
  BCM_FIELD_ASET_ADD(grp_elk.aset, bcmFieldActionExternalValue1Set);


  rv = bcm_field_group_config_create(unit, &grp_elk);
  if (rv == 0) {
      printf("bcm_field_group_config_create error should fail, but passes\n");
      return rv;
  }

  return 0;
}

/* this function configures new ACL in the KBP with key size bigger than 160bit,
   in this case SW will use the DIP SIP sharing so program will be configured */
int ipv6_dip_sip_sharing_elk_group_create(int unit, int hit_bit) {
  int rv=0;
  int group_priority_elk = 42;
  bcm_field_group_config_t grp_elk;
  bcm_field_entry_t ent_elk;
  int presel_id_elk = 0; /* ID from 0 to 11 for Advanced mode */ 
  int presel_flags = 0; 

  /* Create Preselector */ 
  if(soc_property_get(unit, spn_FIELD_PRESEL_MGMT_ADVANCED_MODE, FALSE)) { 
      presel_flags = BCM_FIELD_QUALIFY_PRESEL_ADVANCED_MODE_STAGE_EXTERNAL; 
      rv = bcm_field_presel_create_stage_id(unit, bcmFieldStageExternal, presel_id_elk); 
      if (BCM_E_NONE != rv) { 
          printf("Error in bcm_field_presel_create_stage_id\n"); 
          return rv; 
      } 
  } else { 
      rv = bcm_field_presel_create(unit, &presel_id_elk); 
      if (BCM_E_NONE != rv) { 
          printf("Error in bcm_field_presel_create\n"); 
          return rv; 
      }
  }

  rv = bcm_field_qualify_Stage(unit, presel_id_elk | BCM_FIELD_QUALIFY_PRESEL | presel_flags, bcmFieldStageExternal); 
  if (BCM_E_NONE != rv) { 
      printf("Error in bcm_field_qualify_Stage\n"); 
      return rv; 
  } 

  rv = bcm_field_qualify_AppType(unit, presel_id_elk | BCM_FIELD_QUALIFY_PRESEL | presel_flags, bcmFieldAppTypeIp6Ucast);
  if (rv) {
      printf("bcm_field_qualify_AppType error\n");
      return rv;
  }

  rv = bcm_field_qualify_AppType(unit, presel_id_elk | BCM_FIELD_QUALIFY_PRESEL | presel_flags, bcmFieldAppTypeIp6UcastRpf);
  if (rv) {
      printf("bcm_field_qualify_AppType error\n");
      return rv;
  }

  rv = bcm_field_qualify_AppType(unit, presel_id_elk | BCM_FIELD_QUALIFY_PRESEL | presel_flags, bcmFieldAppTypeIp6Mcast);
  if (rv) {
      printf("bcm_field_qualify_AppType error\n");
      return rv;
  }
  
  bcm_field_group_config_t_init(&grp_elk); 
  BCM_FIELD_PRESEL_ADD(grp_elk.preselset, presel_id_elk);
  grp_elk.group = group_id_elk_sharing;
  grp_elk.priority = group_priority_elk ;
  grp_elk.mode =  bcmFieldGroupModeAuto;
  grp_elk.flags = BCM_FIELD_GROUP_CREATE_WITH_MODE | BCM_FIELD_GROUP_CREATE_WITH_ASET | BCM_FIELD_GROUP_CREATE_WITH_PRESELSET | BCM_FIELD_GROUP_CREATE_WITH_ID;
  
  BCM_FIELD_QSET_INIT(grp_elk.qset);
  BCM_FIELD_QSET_ADD(grp_elk.qset, bcmFieldQualifyStageExternal);
  BCM_FIELD_QSET_ADD(grp_elk.qset, bcmFieldQualifyDstIp6);
  BCM_FIELD_QSET_ADD(grp_elk.qset, bcmFieldQualifySrcIp6);
  BCM_FIELD_QSET_ADD(grp_elk.qset, bcmFieldQualifySrcPort);
  
  
  BCM_FIELD_ASET_INIT(grp_elk.aset);
  BCM_FIELD_ASET_ADD(grp_elk.aset, bcmFieldActionExternalValue0Set+hit_bit);


  rv = bcm_field_group_config_create(unit, &grp_elk);
  if (rv) {
      printf("bcm_field_group_config_create error\n");
      return rv;
  }

  return rv;
}


/* add entry to corresponding IPv6 table */
int ipv6_dip_sip_sharing_elk_entry_add(int unit, int hit_bit) {

  int rv=0;
  bcm_ip6_t ip6_mask =  {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
  bcm_ip6_t ip6_value = {0x1,0x00,0x16,0x00,0x35,0x00,0x70,0x00,0x00,0x00,0xdb,0x7,0x00,0x00,0x00,0x00};
  bcm_field_entry_t ent_elk;


  bcm_field_entry_create(unit, group_id_elk_sharing, &ent_elk);
  bcm_field_qualify_DstIp6(unit, ent_elk, ip6_value, ip6_mask);

  rv = bcm_field_action_add(unit, ent_elk, bcmFieldActionExternalValue0Set+hit_bit, 0, 0);
  if (rv) {
      printf("bcm_field_action_add error\n");
      return rv;
  }

  rv = bcm_field_entry_prio_set(unit, ent_elk, 10);
  if (rv) {
      printf("bcm_field_entry_prio_set error\n");
      return rv;
  }

  rv = bcm_field_entry_install(unit, ent_elk);
  if (rv) {
      printf("bcm_field_entry_install error\n");
      return rv;
  }

  return rv;
}




int ipv6_dip_sip_sharing_example(int unit, int hit_bit) {
  int rv=0;
  
  rv = ipv6_dip_sip_sharing_elk_group_create(unit,hit_bit);
  if (rv) {
      printf("dip_sip_sharing_elk_group_create error\n");
      return rv;
  }

  bshell(unit, "kbp deinit_appl");
  /* Initialize the KBP */
  if (is_qax(unit)) {
      bshell(unit, "kbp init_appl");
  } else {
      bshell(unit, "kbp init_appl see=1");
  }

  rv = ipv6_dip_sip_sharing_elk_entry_add(unit,hit_bit);
  if (rv) {
      printf("dip_sip_sharing_elk_group_create error\n");
      return rv;
  }

  return rv;
}


/* this function configures new ACL in the KBP with key size bigger than 160bit,
   in this case SW will use the DIP SIP sharing so program will be configured */
int ipv4_dip_sip_sharing_elk_group_create(int unit, int hit_bit) {
  int rv=0;
  int group_priority_elk = 42;
  bcm_field_group_config_t grp_elk;
  bcm_field_entry_t ent_elk;
  int presel_id_elk = 0; /* ID from 0 to 11 for Advanced mode */ 
  int presel_flags = 0; 

  /* Create Preselector */ 
  if(soc_property_get(unit, spn_FIELD_PRESEL_MGMT_ADVANCED_MODE, FALSE)) { 
      presel_flags = BCM_FIELD_QUALIFY_PRESEL_ADVANCED_MODE_STAGE_EXTERNAL; 
      rv = bcm_field_presel_create_stage_id(unit, bcmFieldStageExternal, presel_id_elk); 
      if (BCM_E_NONE != rv) { 
          printf("Error in bcm_field_presel_create_stage_id\n"); 
          return rv; 
      } 
  } else { 
      rv = bcm_field_presel_create(unit, &presel_id_elk); 
      if (BCM_E_NONE != rv) { 
          printf("Error in bcm_field_presel_create\n"); 
          return rv; 
      }
  } 

  rv = bcm_field_qualify_Stage(unit, presel_id_elk | BCM_FIELD_QUALIFY_PRESEL | presel_flags, bcmFieldStageExternal); 
  if (BCM_E_NONE != rv) { 
      printf("Error in bcm_field_qualify_Stage\n"); 
      return rv; 
  } 

  rv = bcm_field_qualify_AppType(unit, presel_id_elk | BCM_FIELD_QUALIFY_PRESEL | presel_flags, bcmFieldAppTypeIp4Ucast);
  if (rv) {
      printf("bcm_field_qualify_AppType 4Ucast error\n");
      return rv;
  }

  rv = bcm_field_qualify_AppType(unit, presel_id_elk | BCM_FIELD_QUALIFY_PRESEL | presel_flags, bcmFieldAppTypeIp4UcastRpf);
  if (rv) {
      printf("bcm_field_qualify_AppType 4UcastRpf error\n");
      return rv;
  }

  rv = bcm_field_qualify_AppType(unit, presel_id_elk | BCM_FIELD_QUALIFY_PRESEL | presel_flags, bcmFieldAppTypeCompIp4McastRpf);
  if (rv) {
      printf("bcm_field_qualify_AppType 4UcastRpf error\n");
      return rv;
  }
  
  bcm_field_group_config_t_init(&grp_elk); 
  BCM_FIELD_PRESEL_ADD(grp_elk.preselset, presel_id_elk);
  grp_elk.group = group_id_elk_sharing;
  grp_elk.priority = group_priority_elk ;
  grp_elk.mode =  bcmFieldGroupModeAuto;
  grp_elk.flags = BCM_FIELD_GROUP_CREATE_WITH_MODE | BCM_FIELD_GROUP_CREATE_WITH_ASET | BCM_FIELD_GROUP_CREATE_WITH_PRESELSET | BCM_FIELD_GROUP_CREATE_WITH_ID;
  
  BCM_FIELD_QSET_INIT(grp_elk.qset);
  BCM_FIELD_QSET_ADD(grp_elk.qset, bcmFieldQualifyStageExternal);
  BCM_FIELD_QSET_ADD(grp_elk.qset, bcmFieldQualifySrcIp);
  BCM_FIELD_QSET_ADD(grp_elk.qset, bcmFieldQualifyDstIp);
  BCM_FIELD_QSET_ADD(grp_elk.qset, bcmFieldQualifySrcMac);
  BCM_FIELD_QSET_ADD(grp_elk.qset, bcmFieldQualifyDstMac);
  BCM_FIELD_QSET_ADD(grp_elk.qset, bcmFieldQualifySrcPort);
  
  
  BCM_FIELD_ASET_INIT(grp_elk.aset);
  BCM_FIELD_ASET_ADD(grp_elk.aset, bcmFieldActionExternalValue0Set+hit_bit);


  rv = bcm_field_group_config_create(unit, &grp_elk);
  if (rv) {
      printf("bcm_field_group_config_create error\n");
      return rv;
  }

  return rv;
}

/* add entry to corresponding IPv4 table */
int ipv4_dip_sip_sharing_elk_entry_add(int unit, int hit_bit) {

  int rv=0;
  uint32 dst_ip = 0x7fffff03;
  uint32 dst_ip_mask = 0xffffffff;
  bcm_ip_t mc_ip = 0xE0E0E001; /* 224.224.224.1 */    
  bcm_field_entry_t ent_elk;


  bcm_field_entry_create(unit, group_id_elk_sharing, &ent_elk);
  rv = bcm_field_qualify_DstIp(unit, ent_elk, dst_ip, dst_ip_mask);
  if (rv) {
      printf("bcm_field_qualify_DstIp error\n");
      return rv;
  }
  rv = bcm_field_action_add(unit, ent_elk, bcmFieldActionExternalValue0Set+hit_bit, 0, 0);
  if (rv) {
      printf("bcm_field_action_add error\n");
      return rv;
  }

  rv = bcm_field_entry_prio_set(unit, ent_elk, 10);
  if (rv) {
      printf("bcm_field_entry_prio_set error\n");
      return rv;
  }

  rv = bcm_field_entry_install(unit, ent_elk);
  if (rv) {
      printf("bcm_field_entry_install error\n");
      return rv;
  }


  /* adding entry for multicast */

  bcm_field_entry_create(unit, group_id_elk_sharing, &ent_elk);
  rv = bcm_field_qualify_DstIp(unit, ent_elk, mc_ip, dst_ip_mask);
  if (rv) {
      printf("bcm_field_qualify_DstIp (MC) error\n");
      return rv;
  }

  rv = bcm_field_action_add(unit, ent_elk, bcmFieldActionExternalValue0Set+hit_bit, 0, 0);
  if (rv) {
      printf("bcm_field_action_add (MC) error\n");
      return rv;
  }

  rv = bcm_field_entry_prio_set(unit, ent_elk, 11);
  if (rv) {
      printf("bcm_field_entry_prio_set (MC) error\n");
      return rv;
  }

  rv = bcm_field_entry_install(unit, ent_elk);
  if (rv) {
      printf("bcm_field_entry_install (MC) error\n");
      return rv;
  }

  return rv;
}




int ipv4_dip_sip_sharing_example(int unit, int hit_bit) {
  int rv=0;
  
  rv = ipv4_dip_sip_sharing_elk_group_create(unit,hit_bit);
  if (rv) {
      printf("dip_sip_sharing_elk_group_create error\n");
      return rv;
  }

  bshell(unit, "kbp deinit_appl");
  /* Initialize the KBP */
  if (is_qax(unit)) {
      bshell(unit, "kbp init_appl");
  } else {
      bshell(unit, "kbp init_appl see=1");
  }

  rv = ipv4_dip_sip_sharing_elk_entry_add(unit,hit_bit);
  if (rv) {
      printf("dip_sip_sharing_elk_group_create error\n");
      return rv;
  }

  return rv;
}


/* this function configures the PMF to drop a packet if there was an hit on ACL-3*/
int dip_sip_sharing_pmf_configure(int unit, int is_ipv4, int hit_bit) {
  int rv=0;
  int group_id=3;
  int group_priority = 40;
  bcm_field_group_config_t grp;
  bcm_field_entry_t ent;
  int presel_id = 0; /* ID from 0 to 47 for Advanced mode */ 
  int presel_flags = 0; 

  /* Create Preselector */ 
  if(soc_property_get(unit, spn_FIELD_PRESEL_MGMT_ADVANCED_MODE, FALSE)) { 
      presel_flags = BCM_FIELD_QUALIFY_PRESEL_ADVANCED_MODE_STAGE_INGRESS; 
      rv = bcm_field_presel_create_stage_id(unit, bcmFieldStageIngress, presel_id); 
      if (BCM_E_NONE != rv) { 
          printf("Error in bcm_field_presel_create_stage_id\n"); 
          return rv; 
      } 
  } else { 
      rv = bcm_field_presel_create(unit, &presel_id); 
      if (BCM_E_NONE != rv) { 
          printf("Error in bcm_field_presel_create\n"); 
          return rv; 
      } 
  } 

  rv = bcm_field_qualify_Stage(unit, presel_id | BCM_FIELD_QUALIFY_PRESEL | presel_flags, bcmFieldStageIngress); 
  if (BCM_E_NONE != rv) { 
      printf("Error in bcm_field_qualify_Stage\n"); 
      return rv; 
  } 

  if (is_ipv4) {
      rv = bcm_field_qualify_AppType(unit, presel_id | BCM_FIELD_QUALIFY_PRESEL | presel_flags, bcmFieldAppTypeIp4UcastRpf);
      if (rv) {
          printf("bcm_field_qualify_AppType error\n");
          return rv;
      }

      rv = bcm_field_qualify_AppType(unit, presel_id | BCM_FIELD_QUALIFY_PRESEL | presel_flags, bcmFieldAppTypeIp4Ucast);
      if (rv) {
          printf("bcm_field_qualify_AppType error\n");
          return rv;
      }

      rv = bcm_field_qualify_AppType(unit, presel_id | BCM_FIELD_QUALIFY_PRESEL | presel_flags, bcmFieldAppTypeCompIp4McastRpf);
      if (rv) {
          printf("bcm_field_qualify_AppType error\n");
          return rv;
      }
  }else{

      rv = bcm_field_qualify_AppType(unit, presel_id | BCM_FIELD_QUALIFY_PRESEL | presel_flags, bcmFieldAppTypeIp6UcastRpf);
      if (rv) {
          printf("bcm_field_qualify_AppType error\n");
          return rv;
      }

      rv = bcm_field_qualify_AppType(unit, presel_id | BCM_FIELD_QUALIFY_PRESEL | presel_flags, bcmFieldAppTypeIp6Ucast);
      if (rv) {
          printf("bcm_field_qualify_AppType error\n");
          return rv;
      }

      rv = bcm_field_qualify_AppType(unit, presel_id | BCM_FIELD_QUALIFY_PRESEL | presel_flags, bcmFieldAppTypeIp6Mcast);
      if (rv) {
          printf("bcm_field_qualify_AppType error\n");
          return rv;
      }
  }
  switch (hit_bit) {
  case 0:
	  rv = bcm_field_qualify_ExternalHit0(unit, presel_id | BCM_FIELD_QUALIFY_PRESEL | presel_flags, 0x1, 0x1);
	  if (rv) {
		  printf("bcm_field_qualify_ExternalHit0 error\n");
		  return rv;
	  }
	  break; 
  case 1:
	  rv = bcm_field_qualify_ExternalHit1(unit, presel_id | BCM_FIELD_QUALIFY_PRESEL | presel_flags, 0x1, 0x1);
	  if (rv) {
		  printf("bcm_field_qualify_ExternalHit1 error\n");
		  return rv;
	  }
	  break; 
  case 2:
	   rv = bcm_field_qualify_ExternalHit2(unit, presel_id | BCM_FIELD_QUALIFY_PRESEL | presel_flags, 0x1, 0x1);
	   if (rv) {
		   printf("bcm_field_qualify_ExternalHit2 error\n");
		   return rv;
	   }
	   break; 
  case 3:
	  rv = bcm_field_qualify_ExternalHit3(unit, presel_id | BCM_FIELD_QUALIFY_PRESEL | presel_flags, 0x1, 0x1);
	  if (rv) {
		  printf("bcm_field_qualify_ExternalHit3 error\n");
		  return rv;
	  }
	  break; 
  }
  bcm_field_group_config_t_init(&grp); 
  BCM_FIELD_PRESEL_ADD(grp.preselset, presel_id);
  grp.group = group_id;
  grp.priority = group_priority ;
  grp.mode =  bcmFieldGroupModeAuto;
  grp.flags = BCM_FIELD_GROUP_CREATE_WITH_MODE | BCM_FIELD_GROUP_CREATE_WITH_ASET | BCM_FIELD_GROUP_CREATE_WITH_PRESELSET | BCM_FIELD_GROUP_CREATE_WITH_ID;
  
  BCM_FIELD_QSET_INIT(grp.qset);
  BCM_FIELD_QSET_ADD(grp.qset, bcmFieldQualifyStageIngress);
  BCM_FIELD_QSET_ADD(grp.qset, bcmFieldQualifySrcPort);
  
  BCM_FIELD_ASET_INIT(grp.aset);
  BCM_FIELD_ASET_ADD(grp.aset, bcmFieldActionDrop);


  rv = bcm_field_group_config_create(unit, &grp);
  if (rv) {
      printf("bcm_field_group_config_create error\n");
      return rv;
  }


  bcm_field_entry_create(unit, group_id, &ent);

  rv = bcm_field_action_add(unit, ent, bcmFieldActionDrop, 0, 0);
  if (rv) {
      printf("Error in bcm_field_action_add\n");
      return rv;
  }

  rv = bcm_field_entry_install(unit, ent);
  if (rv) {
      printf("bcm_field_entry_install error\n");
      return rv;
  }

  return rv;
}


int ipv4_dip_sip_sharing_semantic(int unit) {
  int rv=0;
  
  rv = dip_sip_sharing_elk_group_create_fail1(unit);
  if (rv) {
      printf("dip_sip_sharing_elk_group_create error in semantic\n");
      return rv;
  }

  return rv;
}

/*
*   set the pmf to drop packets for a spesific hit bit options (0-3) and a specific application
*/ 
int configure_pmf_action_by_hit_bit_example(int unit, int hit_bit, bcm_field_AppType_t app_type, bcm_field_action_t action_type, int param0, int param1, int presel) {

	int rv=0;
	bcm_field_group_config_t grp;
	bcm_field_entry_t ent;
    int ent_grp;
    int presel_id = presel; /* ID from 0 to 47 for Advanced mode */ 
    int presel_flags = 0; 

    /* Create Preselector */ 
    if(soc_property_get(unit, spn_FIELD_PRESEL_MGMT_ADVANCED_MODE, FALSE)) { 
      presel_flags = BCM_FIELD_QUALIFY_PRESEL_ADVANCED_MODE_STAGE_INGRESS; 
      rv = bcm_field_presel_create_stage_id(unit, bcmFieldStageIngress, presel_id); 
      if (BCM_E_NONE != rv) { 
          printf("Error in bcm_field_presel_create_stage_id\n"); 
          return rv; 
      } 
    } else { 
      rv = bcm_field_presel_create(unit, &presel_id); 
      if (BCM_E_NONE != rv) { 
          printf("Error in bcm_field_presel_create\n"); 
          return rv; 
      } 
    } 

    rv = bcm_field_qualify_Stage(unit, presel_id | BCM_FIELD_QUALIFY_PRESEL | presel_flags, bcmFieldStageIngress); 
    if (BCM_E_NONE != rv) { 
      printf("Error in bcm_field_qualify_Stage\n"); 
      return rv; 
    } 

	rv = bcm_field_qualify_AppType(unit, presel_id | BCM_FIELD_QUALIFY_PRESEL | presel_flags, app_type);
	if (rv) {
	  printf("bcm_field_qualify_AppType error\n");
	  return rv;
	}
	switch (hit_bit) {
	case HIT_BIT_0_QUAL:
		rv = bcm_field_qualify_ExternalHit0(unit, presel_id | BCM_FIELD_QUALIFY_PRESEL | presel_flags, 0x1, 0x1);
		if (rv) {
		  printf("bcm_field_qualify_ExternalHit0 error\n");
		  return rv;
		}
		break;
	case HIT_BIT_1_QUAL:
		rv = bcm_field_qualify_ExternalHit1(unit, presel_id | BCM_FIELD_QUALIFY_PRESEL | presel_flags, 0x1, 0x1);
		if (rv) {
		  printf("bcm_field_qualify_ExternalHit1 error\n");
		  return rv;
		}
		break;
	case HIT_BIT_2_QUAL:
		rv = bcm_field_qualify_ExternalHit2(unit, presel_id | BCM_FIELD_QUALIFY_PRESEL | presel_flags, 0x1, 0x1);
		if (rv) {
		  printf("bcm_field_qualify_ExternalHit2 error\n");
		  return rv;
		}
		break;
	case HIT_BIT_3_QUAL:
		rv = bcm_field_qualify_ExternalHit3(unit, presel_id | BCM_FIELD_QUALIFY_PRESEL | presel_flags, 0x1, 0x1);
		if (rv) {
		  printf("bcm_field_qualify_ExternalHit3 error\n");
		  return rv;
		}
		break;
	case HIT_BIT_4_QUAL:
		rv = bcm_field_qualify_ExternalHit4(unit, presel_id | BCM_FIELD_QUALIFY_PRESEL | presel_flags, 0x1, 0x1);
		if (rv) {
		  printf("bcm_field_qualify_ExternalHit4 error\n");
		  return rv;
		}
		break;
	case HIT_BIT_5_QUAL:
		rv = bcm_field_qualify_ExternalHit5(unit, presel_id | BCM_FIELD_QUALIFY_PRESEL | presel_flags, 0x1, 0x1);
		if (rv) {
		  printf("bcm_field_qualify_ExternalHit5 error\n");
		  return rv;
		}
		break;
	}

	bcm_field_group_config_t_init(&grp); 
	BCM_FIELD_PRESEL_ADD(grp.preselset, presel_id);
	grp.group    = PMF_BASE_GROUP_INDEX 	+ hit_bit;
	grp.priority = PMF_BASE_GROUP_PRIORITY  + hit_bit;
	grp.mode 	 = bcmFieldGroupModeAuto;
	grp.flags 	 = BCM_FIELD_GROUP_CREATE_WITH_MODE | BCM_FIELD_GROUP_CREATE_WITH_ASET | BCM_FIELD_GROUP_CREATE_WITH_PRESELSET | BCM_FIELD_GROUP_CREATE_WITH_ID;

	BCM_FIELD_QSET_INIT(grp.qset);
	BCM_FIELD_QSET_ADD(grp.qset, bcmFieldQualifyStageIngress);
	BCM_FIELD_QSET_ADD(grp.qset, bcmFieldQualifySrcPort);

	BCM_FIELD_ASET_INIT(grp.aset);
	BCM_FIELD_ASET_ADD(grp.aset, action_type);

	ent_grp = grp.group;
	rv = bcm_field_group_config_create(unit, &grp);
	if (rv) {
	  printf("bcm_field_group_config_create error\n");
	  return rv;
	}

	bcm_field_entry_create(unit, ent_grp, &ent);

	rv = bcm_field_action_add(unit, ent, action_type, param0, param1);
	if (rv) {
	  printf("Error in bcm_field_action_add\n");
	  return rv;
	}

	rv = bcm_field_entry_install(unit, ent);
	if (rv) {
	  printf("bcm_field_entry_install error\n");
	  return rv;
	}

	return rv;
}

/*
*   set the pmf to drop packets for a spesific hit bit options (0-3) and a specific application
*/ 
 int configure_pmf_action_by_hit_value_example(int unit, int hit_bit, uint64 hit_value, uint64 hit_mask, bcm_field_AppType_t app_type1, bcm_field_AppType_t app_type2, bcm_field_action_t action_type, int param0, int param1, int priority, int presel) {

	int rv=0;
	bcm_field_group_config_t grp;
	bcm_field_entry_t ent;
	int ent_grp;
    int presel_id = presel; /* ID from 0 to 47 for Advanced mode */ 
    int presel_flags = 0; 

    /* Create Preselector */ 
    if(soc_property_get(unit, spn_FIELD_PRESEL_MGMT_ADVANCED_MODE, FALSE)) { 
      presel_flags = BCM_FIELD_QUALIFY_PRESEL_ADVANCED_MODE_STAGE_INGRESS; 
      rv = bcm_field_presel_create_stage_id(unit, bcmFieldStageIngress, presel_id); 
      if (BCM_E_NONE != rv) { 
          printf("Error in bcm_field_presel_create_stage_id\n"); 
          return rv; 
      } 
    } else { 
      rv = bcm_field_presel_create(unit, &presel_id); 
      if (BCM_E_NONE != rv) { 
          printf("Error in bcm_field_presel_create\n"); 
          return rv; 
      } 
    } 

    rv = bcm_field_qualify_Stage(unit, presel_id | BCM_FIELD_QUALIFY_PRESEL | presel_flags, bcmFieldStageIngress); 
    if (BCM_E_NONE != rv) { 
      printf("Error in bcm_field_qualify_Stage\n"); 
      return rv; 
    } 

	rv = bcm_field_qualify_AppType(unit, presel_id | BCM_FIELD_QUALIFY_PRESEL | presel_flags, app_type1);
	if (rv) {
	  printf("bcm_field_qualify_AppType error\n");
	  return rv;
	}
	
	if (app_type2 != 0) {
		rv = bcm_field_qualify_AppType(unit, presel_id | BCM_FIELD_QUALIFY_PRESEL | presel_flags, app_type2);
		if (rv) {
		  printf("bcm_field_qualify_AppType error\n");
		  return rv;
		}
	}

	bcm_field_group_config_t_init(&grp); 
	BCM_FIELD_PRESEL_ADD(grp.preselset, presel_id);
	grp.group    = PMF_BASE_GROUP_INDEX 	+ hit_bit + priority;
	grp.priority = PMF_BASE_GROUP_PRIORITY  + hit_bit + priority;
	grp.mode 	 = bcmFieldGroupModeAuto;
	grp.flags 	 = BCM_FIELD_GROUP_CREATE_WITH_MODE | BCM_FIELD_GROUP_CREATE_WITH_ASET | BCM_FIELD_GROUP_CREATE_WITH_PRESELSET | BCM_FIELD_GROUP_CREATE_WITH_ID;

	BCM_FIELD_QSET_INIT(grp.qset);
	BCM_FIELD_QSET_ADD(grp.qset, bcmFieldQualifyExternalValue0);
	BCM_FIELD_QSET_ADD(grp.qset, bcmFieldQualifyExternalValue1);
	BCM_FIELD_QSET_ADD(grp.qset, bcmFieldQualifyExternalValue2);
	BCM_FIELD_QSET_ADD(grp.qset, bcmFieldQualifyExternalValue3);
	BCM_FIELD_QSET_ADD(grp.qset, bcmFieldQualifyExternalValue4);
	BCM_FIELD_QSET_ADD(grp.qset, bcmFieldQualifyExternalValue5);


	BCM_FIELD_ASET_INIT(grp.aset);
	BCM_FIELD_ASET_ADD(grp.aset, action_type);

	ent_grp = grp.group;
	rv = bcm_field_group_config_create(unit, &grp);
	if (rv) {
	  printf("bcm_field_group_config_create error\n");
	  return rv;
	}

	rv = bcm_field_entry_create(unit, ent_grp, &ent);
	if (rv) {
	  printf("Error in bcm_field_entry_create\n");
	  return rv;
	}

	switch (hit_bit) {
	case HIT_BIT_0_QUAL:
		rv = bcm_field_qualify_ExternalValue0(unit, ent, hit_value, hit_mask);
		if (rv) {
		  printf("bcm_field_qualify_ExternalValue0 error\n");
		  return rv;
		}
		break;
	case HIT_BIT_1_QUAL:
		rv = bcm_field_qualify_ExternalValue1(unit, ent, hit_value, hit_mask);
		if (rv) {
		  printf("bcm_field_qualify_ExternalValue1 error\n");
		  return rv;
		}
		break;
	case HIT_BIT_2_QUAL:
		rv = bcm_field_qualify_ExternalValue2(unit, ent, hit_value, hit_mask);
		if (rv) {
		  printf("bcm_field_qualify_ExternalValue2 error\n");
		  return rv;
		}
		break;
	case HIT_BIT_3_QUAL:
		rv = bcm_field_qualify_ExternalValue3(unit, ent, hit_value, hit_mask);
		if (rv) {
		  printf("bcm_field_qualify_ExternalValue3 error\n");
		  return rv;
		}
		break;
	case HIT_BIT_4_QUAL:
		rv = bcm_field_qualify_ExternalValue4(unit, ent, hit_value, hit_mask);
		if (rv) {
		  printf("bcm_field_qualify_ExternalValue4 error\n");
		  return rv;
		}
		break;
	case HIT_BIT_5_QUAL:
		rv = bcm_field_qualify_ExternalValue5(unit, ent, hit_value, hit_mask);
		if (rv) {
		  printf("bcm_field_qualify_ExternalValue5 error\n");
		  return rv;
		}
		break;
	}

	rv = bcm_field_action_add(unit, ent, action_type, param0, param1);
	if (rv) {
	  printf("Error in bcm_field_action_add\n");
	  return rv;
	}

	rv = bcm_field_entry_install(unit, ent);
	if (rv) {
	  printf("bcm_field_entry_install error\n");
	  return rv;
	}

	return rv;
}

/*
 * configure ivp4 ACL with 2 lookups: 
 * 1 - vlan  
 * 2 - dmac + vlan 
*/
int frwd_kbp_app_2_acls_kbp_config_example(int unit) {

	int rv=0;
	bcm_field_group_config_t grp_elk;
    int presel_id_elk = 0; /* ID from 0 to 11 for Advanced mode */ 
    int presel_flags = 0; 

    /* Create Preselector */ 
    if(soc_property_get(unit, spn_FIELD_PRESEL_MGMT_ADVANCED_MODE, FALSE)) { 
      presel_flags = BCM_FIELD_QUALIFY_PRESEL_ADVANCED_MODE_STAGE_EXTERNAL; 
      rv = bcm_field_presel_create_stage_id(unit, bcmFieldStageExternal, presel_id_elk); 
      if (BCM_E_NONE != rv) { 
          printf("Error in bcm_field_presel_create_stage_id\n"); 
          return rv; 
      } 
    } else { 
      rv = bcm_field_presel_create(unit, &presel_id_elk); 
      if (BCM_E_NONE != rv) { 
          printf("Error in bcm_field_presel_create\n"); 
          return rv; 
      }
    } 

    rv = bcm_field_qualify_Stage(unit, presel_id_elk | BCM_FIELD_QUALIFY_PRESEL | presel_flags, bcmFieldStageExternal); 
    if (BCM_E_NONE != rv) { 
      printf("Error in bcm_field_qualify_Stage\n"); 
      return rv; 
    } 

	rv = bcm_field_qualify_AppType(unit, presel_id_elk | BCM_FIELD_QUALIFY_PRESEL | presel_flags, bcmFieldAppTypeIp4Ucast);
	if (rv) {
	  printf("bcm_field_qualify_AppType Ip4Ucast error\n");
	  return rv;
	}

	bcm_field_group_config_t_init(&grp_elk); 
	BCM_FIELD_PRESEL_ADD(grp_elk.preselset, presel_id_elk);
	grp_elk.mode =  bcmFieldGroupModeAuto;
	grp_elk.flags = BCM_FIELD_GROUP_CREATE_WITH_MODE | BCM_FIELD_GROUP_CREATE_WITH_ASET | BCM_FIELD_GROUP_CREATE_WITH_PRESELSET | BCM_FIELD_GROUP_CREATE_WITH_ID;

	BCM_FIELD_QSET_INIT(grp_elk.qset);
	BCM_FIELD_ASET_INIT(grp_elk.aset);
	BCM_FIELD_QSET_ADD(grp_elk.qset, bcmFieldQualifyStageExternal);
	
	/* 1st: vlan only*/
	BCM_FIELD_QSET_ADD(grp_elk.qset, bcmFieldQualifyOuterVlanId);
	BCM_FIELD_ASET_ADD(grp_elk.aset, bcmFieldActionExternalValue1Set);
	grp_elk.group 	 = ACL_BASE_GROUP_INDEX+1;
	grp_elk.priority = ACL_BASE_GROUP_PRIORITY;

	rv = bcm_field_group_config_create(unit, &grp_elk);
	if (rv) {
	  printf("bcm_field_group_config_create 1 error \n");
	  return rv;
	} 

	/* 2nd: vlan + d mac*/
	BCM_FIELD_ASET_REMOVE(grp_elk.aset, bcmFieldActionExternalValue1Set);
	BCM_FIELD_ASET_ADD(grp_elk.aset, bcmFieldActionExternalValue3Set);
	BCM_FIELD_QSET_ADD(grp_elk.qset, bcmFieldQualifySrcMac);
	grp_elk.group 	 = ACL_BASE_GROUP_INDEX+3;
	grp_elk.priority++;

	rv = bcm_field_group_config_create(unit, &grp_elk);
	if (rv) {
	  printf("bcm_field_group_config_create 3 error \n");
	  return rv;
	}

	/* Initialize the KBP */
	bshell(unit, "kbp deinit_appl");
	bshell(unit, "kbp init_appl");

	return rv;
}

/*
*	set configuration of KBP tables and PMF dropping action 
*/
int frwd_kbp_app_2_acls_kbp_set_configurations(int unit) {
	int rv;
    int presel_id = 1; /* ingress presel id for advanced mode: between 1-47 as 0 is taken */
	rv = frwd_kbp_app_2_acls_kbp_config_example(unit);
	if (rv) {
		printf("frwd_kbp_app_2_acls_kbp_config_example error\n");
		return  rv;
	}

	rv = configure_pmf_action_by_hit_bit_example(unit,HIT_BIT_1_QUAL,bcmFieldAppTypeIp4Ucast, bcmFieldActionDrop, 0, 0, presel_id);
	if (rv) {
		printf("configure_pmf_action_by_hit_bit_example bit 1 error\n");
		return  rv;
	}

    presel_id++; /* Take next free preselector id for advanced mode */
	rv = configure_pmf_action_by_hit_bit_example(unit,HIT_BIT_3_QUAL,bcmFieldAppTypeIp4Ucast, bcmFieldActionDrop, 0, 0, presel_id);
	if (rv) {
		printf("configure_pmf_action_by_hit_bit_example bit 3 error\n");
		return  rv;
	}

	return rv;
}

/*
*   add an ACL entry according to the test: AT_Cint_external_frwd_kbp_app_2_acl_traffic
*/ 
int frwd_kbp_app_2_acls_kbp_entry_add_example(int unit, int index) {

	int i,rv;
	bcm_field_entry_t ent_elk;

	uint16 vlanId = 100;

	bcm_mac_t s_mac = {0x00, 0x00, 0x07, 0x00, 0x01, 0x00};	
	bcm_mac_t s_mac_mask = {0xff,0xff,0xff,0xff,0xff,0xff};
	
	bcm_field_entry_create(unit, ACL_BASE_GROUP_INDEX + index, &ent_elk);

	switch (index) {
	case 1:/* 1st: vlanId only*/
		rv = bcm_field_qualify_OuterVlanId(unit, ent_elk, vlanId, 0xFFF);
		if (rv) {
		  printf("bcm_field_qualify_OuterVlanId error, index = %d\n",index);
		  return rv;
		}
		break;
	case 3:/* 2nd: VlanId + s mac*/
		rv = bcm_field_qualify_OuterVlanId(unit, ent_elk, vlanId, 0xFFF);
		if (rv) {
		  printf("bcm_field_qualify_OuterVlanId error, index = %d\n",index);
		  return rv;
		}
		rv = bcm_field_qualify_SrcMac(unit, ent_elk, s_mac, s_mac_mask);
		if (rv) {
		  printf("bcm_field_qualify_SrcMac error, index = %d\n",index);
		  return rv;
		}
		break;
	default:
		printf("Unknown index value = %d",index);
		return -1;
	}

	rv = bcm_field_entry_prio_set(unit, ent_elk, 10);
	if (rv) {
	  printf("bcm_field_entry_prio_set error\n");
	  return rv;
	}

	rv = bcm_field_entry_install(unit, ent_elk);
	if (rv) {
	  printf("bcm_field_entry_install error\n");
	  return rv;
	}

	return rv;
}

/*
 * test for 4 mode of ACLs Entries: 
 * 		|****| = first group, |####| = second group 
 *  
 * scenario		 	LSB				MSB	
 * 	  0  		|**********|    |***       | 
 *    1  		|**********|    |***####   | 
 *    2  		|****###   |    |          | 
 *	  3  		|****######|    |###       | 
 *    4          freestyle
 */

int acl_kbp_multiple_acls_config_example(int unit, int scenario) {

	int rv=0;
	bcm_field_group_config_t grp_elk;
    int presel_id_elk = 0; /* ID from 0 to 11 for Advanced mode */ 
    int presel_flags = 0; 

    /* Create Preselector */ 
    if(soc_property_get(unit, spn_FIELD_PRESEL_MGMT_ADVANCED_MODE, FALSE)) { 
      presel_flags = BCM_FIELD_QUALIFY_PRESEL_ADVANCED_MODE_STAGE_EXTERNAL; 
      rv = bcm_field_presel_create_stage_id(unit, bcmFieldStageExternal, presel_id_elk); 
      if (BCM_E_NONE != rv) { 
          printf("Error in bcm_field_presel_create_stage_id\n"); 
          return rv; 
      } 
    } else { 
      rv = bcm_field_presel_create(unit, &presel_id_elk); 
      if (BCM_E_NONE != rv) { 
          printf("Error in bcm_field_presel_create\n"); 
          return rv; 
      }
    } 

    rv = bcm_field_qualify_Stage(unit, presel_id_elk | BCM_FIELD_QUALIFY_PRESEL | presel_flags, bcmFieldStageExternal); 
    if (BCM_E_NONE != rv) { 
      printf("Error in bcm_field_qualify_Stage\n"); 
      return rv; 
    } 

	rv = bcm_field_qualify_AppType(unit, presel_id_elk | BCM_FIELD_QUALIFY_PRESEL | presel_flags, bcmFieldAppTypeL2);
	if (rv) {
	  printf("bcm_field_qualify_AppType AppTypeL2 error\n");
	  return rv;
	}

	bcm_field_group_config_t_init(&grp_elk); 
	BCM_FIELD_PRESEL_ADD(grp_elk.preselset, presel_id_elk);
	grp_elk.mode =  bcmFieldGroupModeAuto;
	grp_elk.flags = BCM_FIELD_GROUP_CREATE_WITH_MODE | BCM_FIELD_GROUP_CREATE_WITH_ASET | BCM_FIELD_GROUP_CREATE_WITH_PRESELSET | BCM_FIELD_GROUP_CREATE_WITH_ID;

	BCM_FIELD_QSET_INIT(grp_elk.qset);
	BCM_FIELD_ASET_INIT(grp_elk.aset);
	BCM_FIELD_QSET_ADD(grp_elk.qset, bcmFieldQualifyStageExternal);
	grp_elk.group 	 = ACL_BASE_GROUP_INDEX;
	grp_elk.priority = ACL_BASE_GROUP_PRIORITY;
	

	switch (scenario) {
		case 0:
			BCM_FIELD_QSET_ADD(grp_elk.qset, bcmFieldQualifyDstMac);
			BCM_FIELD_QSET_ADD(grp_elk.qset, bcmFieldQualifySrcMac);
			BCM_FIELD_QSET_ADD(grp_elk.qset, bcmFieldQualifyEtherType);
			BCM_FIELD_ASET_ADD(grp_elk.aset, bcmFieldActionExternalValue0Set);

			rv = bcm_field_group_config_create(unit, &grp_elk);
			if (rv) {
			  printf("bcm_field_group_config_create (scenario,group)(0,0) error \n");
			  return rv;
			}
			break;
		case 1:
			BCM_FIELD_QSET_ADD(grp_elk.qset, bcmFieldQualifyDstMac);
			BCM_FIELD_QSET_ADD(grp_elk.qset, bcmFieldQualifySrcMac);
			BCM_FIELD_ASET_ADD(grp_elk.aset, bcmFieldActionExternalValue0Set);

			rv = bcm_field_group_config_create(unit, &grp_elk);
			if (rv) {
				printf("bcm_field_group_config_create (scenario,group)(1,0) error \n");
			  return rv;
			}

			grp_elk.group++;
			grp_elk.priority++;
			BCM_FIELD_QSET_REMOVE(grp_elk.qset, bcmFieldQualifySrcMac);
			BCM_FIELD_QSET_REMOVE(grp_elk.qset, bcmFieldQualifyDstMac);
			BCM_FIELD_QSET_ADD(grp_elk.qset, bcmFieldQualifyEtherType);
			BCM_FIELD_ASET_REMOVE(grp_elk.aset, bcmFieldActionExternalValue0Set);
			BCM_FIELD_ASET_ADD(grp_elk.aset, bcmFieldActionExternalValue1Set);
			
			rv = bcm_field_group_config_create(unit, &grp_elk);
			if (rv) {
				printf("bcm_field_group_config_create (scenario,group)(1,1) error \n");
			  return rv;
			}
			break;
		case 2:
			BCM_FIELD_QSET_ADD(grp_elk.qset, bcmFieldQualifyDstMac);
			BCM_FIELD_ASET_ADD(grp_elk.aset, bcmFieldActionExternalValue0Set);

			rv = bcm_field_group_config_create(unit, &grp_elk);
			if (rv) {
				printf("bcm_field_group_config_create (scenario,group)(2,0) error \n");
			  return rv;
			}

			grp_elk.group++;
			grp_elk.priority++;
			BCM_FIELD_QSET_REMOVE(grp_elk.qset, bcmFieldQualifyDstMac);
			BCM_FIELD_QSET_ADD(grp_elk.qset, bcmFieldQualifyOuterVlanId);
			BCM_FIELD_ASET_REMOVE(grp_elk.aset, bcmFieldActionExternalValue0Set);
			BCM_FIELD_ASET_ADD(grp_elk.aset, bcmFieldActionExternalValue1Set);
			
			rv = bcm_field_group_config_create(unit, &grp_elk);
			if (rv) {
				printf("bcm_field_group_config_create (scenario,group)(2,1) error \n");
			  return rv;
			}
			break;
		case 3:
			BCM_FIELD_QSET_ADD(grp_elk.qset, bcmFieldQualifyDstMac);
			BCM_FIELD_ASET_ADD(grp_elk.aset, bcmFieldActionExternalValue0Set);

			rv = bcm_field_group_config_create(unit, &grp_elk);
			if (rv) {
				printf("bcm_field_group_config_create (scenario,group)(1,0) error \n");
			  return rv;
			}

			grp_elk.group++;
			grp_elk.priority++;
			BCM_FIELD_QSET_REMOVE(grp_elk.qset, bcmFieldQualifyDstMac);
			BCM_FIELD_QSET_ADD(grp_elk.qset, bcmFieldQualifySrcMac);
			BCM_FIELD_QSET_ADD(grp_elk.qset, bcmFieldQualifyEtherType);
			BCM_FIELD_ASET_REMOVE(grp_elk.aset, bcmFieldActionExternalValue0Set);
			BCM_FIELD_ASET_ADD(grp_elk.aset, bcmFieldActionExternalValue1Set);
			
			rv = bcm_field_group_config_create(unit, &grp_elk);
			if (rv) {
				printf("bcm_field_group_config_create (scenario,group)(1,1) error \n");
			  return rv;
			}
			break;
		case 4:
			BCM_FIELD_QSET_ADD(grp_elk.qset, bcmFieldQualifyDstMac);
			BCM_FIELD_ASET_ADD(grp_elk.aset, bcmFieldActionExternalValue0Set);

			rv = bcm_field_group_config_create(unit, &grp_elk);
			if (rv) {
				printf("bcm_field_group_config_create (scenario,group)(4,0) error \n");
			  return rv;
			}

			grp_elk.group++;
			grp_elk.priority++;
			BCM_FIELD_QSET_REMOVE(grp_elk.qset, bcmFieldQualifyDstMac);
			BCM_FIELD_QSET_ADD(grp_elk.qset, bcmFieldQualifyEtherType);
			BCM_FIELD_ASET_REMOVE(grp_elk.aset, bcmFieldActionExternalValue0Set);
			BCM_FIELD_ASET_ADD(grp_elk.aset, bcmFieldActionExternalValue1Set);

			rv = bcm_field_group_config_create(unit, &grp_elk);
			if (rv) {
				printf("bcm_field_group_config_create (scenario,group)(4,1) error \n");
			  return rv;
			}
			
			grp_elk.group++;
			grp_elk.priority++;
			BCM_FIELD_QSET_REMOVE(grp_elk.qset, bcmFieldQualifyEtherType);
			BCM_FIELD_QSET_ADD(grp_elk.qset, bcmFieldQualifySrcPort);
			BCM_FIELD_ASET_REMOVE(grp_elk.aset, bcmFieldActionExternalValue1Set);
			BCM_FIELD_ASET_ADD(grp_elk.aset, bcmFieldActionExternalValue2Set);

			rv = bcm_field_group_config_create(unit, &grp_elk);
			if (rv) {
				printf("bcm_field_group_config_create (scenario,group)(4,2) error \n");
			  return rv;
			}
			
			grp_elk.group++;
			grp_elk.priority++;
			BCM_FIELD_QSET_REMOVE(grp_elk.qset, bcmFieldQualifySrcPort);
			BCM_FIELD_QSET_ADD(grp_elk.qset, bcmFieldQualifySrcMac);
			BCM_FIELD_ASET_REMOVE(grp_elk.aset, bcmFieldActionExternalValue2Set);
			BCM_FIELD_ASET_ADD(grp_elk.aset, bcmFieldActionExternalValue3Set);

			rv = bcm_field_group_config_create(unit, &grp_elk);
			if (rv) {
				printf("bcm_field_group_config_create (scenario,group)(4,3) error \n");
			  return rv;
			}

			break;
		default:
			printf("Unknown Scenarion: %d \n",scenario);
			return -1;
	}

	/* Initialize the KBP */
	bshell(unit, "kbp deinit_appl");
	bshell(unit, "kbp init_appl");

	return rv;
}

int acl_kbp_multiple_acls_add_entry_example(int unit, int scenario, int index) {

	int rv;
	int entry_index;
	bcm_field_entry_t ent_elk;

	uint16 EthType = 0x0800;
	uint16 vlan_id = 100;

	bcm_mac_t dst  = {0x00, 0x0c, 0x00, 0x02, 0x00, 0x00};
	bcm_mac_t src  = {0x00, 0x00, 0x07, 0x00, 0x01, 0x00};

	bcm_mac_t mask_0 = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
	bcm_mac_t mask_1 = {0xFF, 0xFF, 0xFF, 0xF0, 0x00, 0x00};
	bcm_mac_t mask_2 = {0xF0, 0xF0, 0x0F, 0xF0, 0x0F, 0x0F};
	bcm_mac_t mask_3 = {0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF};

	bcm_mac_t dst_freestyle  = {0x11, 0x02, 0x03, 0x04, 0x05, 0x66};
	bcm_mac_t src_freestyle  = {0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f};

	bcm_field_entry_create(unit, index, &ent_elk);

	entry_index = scenario*4+index;
	switch (entry_index) {
		case 0:
			rv = bcm_field_qualify_EtherType(unit, ent_elk, EthType, 0xFFFF);
			if (rv) {
			  printf("bcm_field_qualify_OuterVlanId error, entry_index = %d\n",entry_index);
			  return rv;
			}
			rv = bcm_field_qualify_DstMac(unit, ent_elk, dst, mask_1);
			if (rv) {
			  printf("bcm_field_qualify_DstMac error, entry_index = %d\n",entry_index);
			  return rv;
			}
			rv = bcm_field_qualify_SrcMac(unit, ent_elk, src, mask_1);
			if (rv) {
			  printf("bcm_field_qualify_SrcMac error, entry_index = %d\n",entry_index);
			  return rv;
			}
			break;
		case 4:
			rv = bcm_field_qualify_DstMac(unit, ent_elk, dst, mask_3);
			if (rv) {
			  printf("bcm_field_qualify_DstMac error, entry_index = %d\n",entry_index);
			  return rv;
			}
			rv = bcm_field_qualify_SrcMac(unit, ent_elk, src, mask_2);
			if (rv) {
			  printf("bcm_field_qualify_SrcMac error, entry_index = %d\n",entry_index);
			  return rv;
			}
			break;
		case 5:
			rv = bcm_field_qualify_EtherType(unit, ent_elk, EthType, 0xFFFF);
			if (rv) {
			  printf("bcm_field_qualify_EtherType error, entry_index = %d\n",entry_index);
			  return rv;
			}
			break;
		case 8:
			rv = bcm_field_qualify_DstMac(unit, ent_elk, dst, mask_0);
			if (rv) {
			  printf("bcm_field_qualify_DstMac error, entry_index = %d\n",entry_index);
			  return rv;
			}
			break;
		case 9:
			rv = bcm_field_qualify_OuterVlanId(unit, ent_elk, vlan_id, 0xFFF);
			if (rv) {
			  printf("bcm_field_qualify_OuterVlan error, entry_index = %d\n",entry_index);
			  return rv;
			}
			break;
		case 12:
			rv = bcm_field_qualify_DstMac(unit, ent_elk, dst, mask_0);
			if (rv) {
			  printf("bcm_field_qualify_DstMac error, entry_index = %d\n",entry_index);
			  return rv;
			}
			break;
		case 13:
			rv = bcm_field_qualify_SrcMac(unit, ent_elk, src, mask_0);
			if (rv) {
			  printf("bcm_field_qualify_SrcMac error, entry_index = %d\n",entry_index);
			  return rv;
			}
			rv = bcm_field_qualify_EtherType(unit, ent_elk, EthType, 0xFFFF);
			if (rv) {
			  printf("bcm_field_qualify_EtherType error, entry_index = %d\n",entry_index);
			  return rv;
			}
			break;
		case 16:
			rv = bcm_field_qualify_DstMac(unit, ent_elk, dst_freestyle, mask);
			if (rv) {
			  printf("bcm_field_qualify_DstMac error, entry_index = %d\n",entry_index);
			  return rv;
			}
			rv = bcm_field_action_add(unit, ent_elk, bcmFieldActionExternalValue0Set,0x12345678,0xabcd0000);
			if (rv) {
			  printf("bcm_field_action_add error\n");
			  return rv;
			}
			break;
		case 17:
		     rv = bcm_field_qualify_EtherType(unit, ent_elk, EthType, 0xFFFF);
			if (rv) {
			  printf("bcm_field_qualify_EtherType error, entry_index = %d\n",entry_index);
			  return rv;
			}
			rv = bcm_field_action_add(unit, ent_elk, bcmFieldActionExternalValue1Set,0x4321,0);
			if (rv) {
			  printf("bcm_field_action_add error\n");
			  return rv;
			}
			break;
		case 18:
			break;
		case 19:
			rv = bcm_field_qualify_SrcMac(unit, ent_elk, src_freestyle, mask_0);
			if (rv) {
			  printf("bcm_field_qualify_SrcMac error, entry_index = %d\n",entry_index);
			  return rv;
			}
			rv = bcm_field_action_add(unit, ent_elk, bcmFieldActionExternalValue3Set,0x87654321,0);
			if (rv) {
			  printf("bcm_field_action_add error\n");
			  return rv;
			}
			break;
		default:
			printf("Unknown entry_index value = %d, scenario = %d, index = %d\n",entry_index,scenario,index);
			return -1;
	}

	rv = bcm_field_entry_prio_set(unit, ent_elk, 10);
	if (rv) {
	  printf("bcm_field_entry_prio_set error\n");
	  return rv;
	}

	rv = bcm_field_entry_install(unit, ent_elk);
	if (rv) {
	  printf("bcm_field_entry_install error\n");
	  return rv;
	}

	return rv;
}

int acl_kbp_multiple_acls_set_configuration(int unit, int scenario) {
	int rv;
    int presel_id = 1; /* ingress presel id for advanced mode: between 1-47 as 0 is taken */

	rv = acl_kbp_multiple_acls_config_example(unit,scenario);
	if (rv) {
		printf("acl_kbp_multiple_acls_config_example error\n");
		return  rv;
	}

	rv = configure_pmf_action_by_hit_bit_example(unit,HIT_BIT_0_QUAL,bcmFieldAppTypeL2, bcmFieldActionDrop, 0, 0, presel_id);
	if (rv) {
		printf("configure_pmf_action_by_hit_bit_example bit 0 error\n");
		return  rv;
	}
	if (scenario != 0) {
		/* scenario 0 has only one group on search 0
		   scenarios 1-3 have 2 groups, on search 0 and search 1*/
        presel_id++; /* Use next free preselector ID for advanced mode */
		rv = configure_pmf_action_by_hit_bit_example(unit, HIT_BIT_1_QUAL, bcmFieldAppTypeL2, bcmFieldActionDrop, 0, 0, presel_id);
		if (rv) {
			printf("configure_pmf_action_by_hit_bit_example bit 2 error\n");
			return  rv;
		}
	}
	return rv;
}

int acl_kbp_check_entry_priority_config_example(int unit, int changed_result_size) {

	int rv=0;
	bcm_field_group_config_t grp_elk;
    int presel_id_elk = 0; /* ID from 0 to 11 for Advanced mode */ 
    int presel_flags = 0; 

    /* Create Preselector */ 
    if(soc_property_get(unit, spn_FIELD_PRESEL_MGMT_ADVANCED_MODE, FALSE)) { 
      presel_flags = BCM_FIELD_QUALIFY_PRESEL_ADVANCED_MODE_STAGE_EXTERNAL; 
      rv = bcm_field_presel_create_stage_id(unit, bcmFieldStageExternal, presel_id_elk); 
      if (BCM_E_NONE != rv) { 
          printf("Error in bcm_field_presel_create_stage_id\n"); 
          return rv; 
      } 
    } else { 
      rv = bcm_field_presel_create(unit, &presel_id_elk); 
      if (BCM_E_NONE != rv) { 
          printf("Error in bcm_field_presel_create\n"); 
          return rv; 
      }
    } 

    rv = bcm_field_qualify_Stage(unit, presel_id_elk | BCM_FIELD_QUALIFY_PRESEL | presel_flags, bcmFieldStageExternal); 
    if (BCM_E_NONE != rv) { 
      printf("Error in bcm_field_qualify_Stage\n"); 
      return rv; 
    } 

	rv = bcm_field_qualify_AppType(unit, presel_id_elk | BCM_FIELD_QUALIFY_PRESEL | presel_flags, bcmFieldAppTypeL2);
	if (rv) {
	  printf("bcm_field_qualify_AppType AppTypeL2 error\n");
	  return rv;
	}

	bcm_field_group_config_t_init(&grp_elk); 
	BCM_FIELD_PRESEL_ADD(grp_elk.preselset, presel_id_elk);
	grp_elk.mode =  bcmFieldGroupModeAuto;
	grp_elk.flags = BCM_FIELD_GROUP_CREATE_WITH_MODE | BCM_FIELD_GROUP_CREATE_WITH_ASET | BCM_FIELD_GROUP_CREATE_WITH_PRESELSET | BCM_FIELD_GROUP_CREATE_WITH_ID;

	BCM_FIELD_QSET_INIT(grp_elk.qset);
	BCM_FIELD_ASET_INIT(grp_elk.aset);
	BCM_FIELD_QSET_ADD(grp_elk.qset, bcmFieldQualifyStageExternal);
	grp_elk.group 	 = ACL_BASE_GROUP_INDEX;
	grp_elk.priority = ACL_BASE_GROUP_PRIORITY;

	BCM_FIELD_QSET_ADD(grp_elk.qset, bcmFieldQualifyDstMac);
	BCM_FIELD_QSET_ADD(grp_elk.qset, bcmFieldQualifySrcMac);
	BCM_FIELD_QSET_ADD(grp_elk.qset, bcmFieldQualifyEtherType);

	if (changed_result_size == 0) {
		BCM_FIELD_ASET_ADD(grp_elk.aset, bcmFieldActionExternalValue1Set);
	}else{
		BCM_FIELD_ASET_ADD(grp_elk.aset, bcmFieldActionExternalValue3Set);
	}

	rv = bcm_field_group_config_create(unit, &grp_elk);
	if (rv) {
	  printf("bcm_field_group_config_create error \n");
	  return rv;
	}

	/* Initialize the KBP */
	bshell(unit, "kbp deinit_appl");
	bshell(unit, "kbp init_appl");

	return rv;
}

int acl_kbp_check_entry_priority_add_entry_example(int unit, int index) {

	int rv;
	bcm_field_entry_t ent_elk;

	uint16 EthType = 0x0800;
	uint16 vlan_id = 100;

	bcm_mac_t dst  = {0x00, 0x0c, 0x00, 0x02, 0x00, 0x01};
	bcm_mac_t src  = {0x00, 0x00, 0x07, 0x00, 0x01, 0x00};

	bcm_mac_t mask = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

	bcm_field_entry_create(unit, ACL_BASE_GROUP_INDEX, &ent_elk);

	switch (index) {
		case 0:
			rv = bcm_field_qualify_EtherType(unit, ent_elk, EthType, 0xFFFF);
			if (rv) {
			  printf("bcm_field_qualify_EtherType error, index = %d\n",index);
			  return rv;
			}
			rv = bcm_field_qualify_DstMac(unit, ent_elk, dst, mask);
			if (rv) {
			  printf("bcm_field_qualify_DstMac error, index = %d\n",index);
			  return rv;
			}
			rv = bcm_field_action_add(unit, ent_elk, bcmFieldActionExternalValue1Set, is_device_or_above(unit, ARAD_PLUS) ? 0x87654321 : 0x4321, 0);
			if (rv) {
			  printf("bcm_field_action_add error\n");
			  return rv;
			}
			break;
		case 1:
			rv = bcm_field_qualify_SrcMac(unit, ent_elk, src, mask);
			if (rv) {
			  printf("bcm_field_qualify_SrcMac error, index = %d\n",index);
			  return rv;
			}
			rv = bcm_field_action_add(unit, ent_elk, bcmFieldActionExternalValue1Set, is_device_or_above(unit, ARAD_PLUS) ? 0x12345678 : 0x5678, 0);
			if (rv) {
			  printf("bcm_field_action_add error\n");
			  return rv;
			}
			break;
	case 2: /* support result change size*/
			rv = bcm_field_qualify_EtherType(unit, ent_elk, EthType, 0xFFFF);
			if (rv) {
			  printf("bcm_field_qualify_EtherType error, index = %d\n",index);
			  return rv;
			}
			rv = bcm_field_qualify_DstMac(unit, ent_elk, dst, mask);
			if (rv) {
			  printf("bcm_field_qualify_DstMac error, index = %d\n",index);
			  return rv;
			}
			rv = bcm_field_action_add(unit, ent_elk, bcmFieldActionExternalValue3Set, is_device_or_above(unit, ARAD_PLUS) ? 0x87654321 : 0x4321, 0);
			if (rv) {
			  printf("bcm_field_action_add error\n");
			  return rv;
			}
			break;
		default:
			printf("Unknown index = %d\n",index);
			return -1;
	}

	rv = bcm_field_entry_prio_set(unit, ent_elk, 10+index);
	if (rv) {
	  printf("bcm_field_entry_prio_set error\n");
	  return rv;
	}

	rv = bcm_field_entry_install(unit, ent_elk);
	if (rv) {
	  printf("bcm_field_entry_install error\n");
	  return rv;
	}

	return rv;
}

int acl_kbp_check_entry_priority_set_configuration(int unit, int size_change) {

	int rv;
    int presel_id = 1;

	uint64 val,mask;

	COMPILER_64_ZERO(val);
	COMPILER_64_ZERO(mask);

	if(is_device_or_above(unit, ARAD_PLUS)) {
		COMPILER_64_SET(val ,0x00000000,0x87654321);
		COMPILER_64_SET(mask,0x00000000,0xffffffff);
	} else {
		COMPILER_64_SET(val ,0x00000000,0x00004321);
		COMPILER_64_SET(mask,0x00000000,0x0000ffff);
	}

	rv = acl_kbp_check_entry_priority_config_example(unit, size_change);
	if (rv) {
		printf("acl_kbp_check_entry_priority_config_example error\n");
		return  rv;
	}

	if (size_change == 0) {
		rv = configure_pmf_action_by_hit_value_example(unit, HIT_BIT_1_QUAL, val, mask, bcmFieldAppTypeL2, 0, bcmFieldActionDrop, 0, 0, 0, presel_id);
		if (rv) {
			printf("configure_pmf_action_by_hit_value_example bit 1 error\n");
			return  rv;
		}
	}else{
		rv = configure_pmf_action_by_hit_value_example(unit, HIT_BIT_3_QUAL, val, mask, bcmFieldAppTypeL2, 0, bcmFieldActionDrop, 0, 0, 0, presel_id);
		if (rv) {
			printf("configure_pmf_action_by_hit_value_example bit 3 error\n");
			return  rv;
		}
	}
	return rv;
}

int acl_kbp_check_256_bit_result_fwd_rpf_config_example(int unit) {

	int rv=0;
	bcm_field_group_config_t grp_elk;
    int presel_id_elk = 0; /* ID from 0 to 11 for Advanced mode */ 
    int presel_flags = 0; 

    /* Create Preselector */ 
    if(soc_property_get(unit, spn_FIELD_PRESEL_MGMT_ADVANCED_MODE, FALSE)) { 
      presel_flags = BCM_FIELD_QUALIFY_PRESEL_ADVANCED_MODE_STAGE_EXTERNAL; 
      rv = bcm_field_presel_create_stage_id(unit, bcmFieldStageExternal, presel_id_elk); 
      if (BCM_E_NONE != rv) { 
          printf("Error in bcm_field_presel_create_stage_id\n"); 
          return rv; 
      } 
    } else { 
      rv = bcm_field_presel_create(unit, &presel_id_elk); 
      if (BCM_E_NONE != rv) { 
          printf("Error in bcm_field_presel_create\n"); 
          return rv; 
      }
    }

    rv = bcm_field_qualify_Stage(unit, presel_id_elk | BCM_FIELD_QUALIFY_PRESEL | presel_flags, bcmFieldStageExternal); 
    if (BCM_E_NONE != rv) { 
      printf("Error in bcm_field_qualify_Stage\n"); 
      return rv; 
    } 

	rv = bcm_field_qualify_AppType(unit, presel_id_elk | BCM_FIELD_QUALIFY_PRESEL | presel_flags, bcmFieldAppTypeL2);
	if (rv) {
	  printf("bcm_field_qualify_AppType AppTypeL2 error\n");
	  return rv;
	}

	bcm_field_group_config_t_init(&grp_elk); 
	BCM_FIELD_PRESEL_ADD(grp_elk.preselset, presel_id_elk);
	grp_elk.mode =  bcmFieldGroupModeAuto;
	grp_elk.flags = BCM_FIELD_GROUP_CREATE_WITH_MODE | BCM_FIELD_GROUP_CREATE_WITH_ASET | BCM_FIELD_GROUP_CREATE_WITH_PRESELSET | BCM_FIELD_GROUP_CREATE_WITH_ID;

	BCM_FIELD_QSET_INIT(grp_elk.qset);
	BCM_FIELD_ASET_INIT(grp_elk.aset);
	BCM_FIELD_QSET_ADD(grp_elk.qset, bcmFieldQualifyStageExternal);
	grp_elk.group 	 = ACL_BASE_GROUP_INDEX;
	grp_elk.priority = ACL_BASE_GROUP_PRIORITY;


	BCM_FIELD_QSET_ADD(grp_elk.qset, bcmFieldQualifyOuterVlanId);
	BCM_FIELD_ASET_ADD(grp_elk.aset, bcmFieldActionExternalValue0Set);
	rv = bcm_field_group_config_create(unit, &grp_elk);
	if (rv) {
	  printf("bcm_field_group_config_create 0 error \n");
	  return rv;
	}

	BCM_FIELD_QSET_REMOVE(grp_elk.qset, bcmFieldQualifyOuterVlanId);
	BCM_FIELD_QSET_ADD(grp_elk.qset, bcmFieldQualifyDstMac);
	BCM_FIELD_ASET_REMOVE(grp_elk.aset, bcmFieldActionExternalValue0Set);
	BCM_FIELD_ASET_ADD(grp_elk.aset, bcmFieldActionExternalValue1Set);
	grp_elk.group++;
	grp_elk.priority++;
	rv = bcm_field_group_config_create(unit, &grp_elk);
	if (rv) {
	  printf("bcm_field_group_config_create 1 error \n");
	  return rv;
	}
	
	BCM_FIELD_QSET_REMOVE(grp_elk.qset, bcmFieldQualifyDstMac);
	BCM_FIELD_QSET_ADD(grp_elk.qset, bcmFieldQualifySrcMac);
	BCM_FIELD_ASET_REMOVE(grp_elk.aset, bcmFieldActionExternalValue1Set);
	BCM_FIELD_ASET_ADD(grp_elk.aset, bcmFieldActionExternalValue2Set);
	grp_elk.group++;
	grp_elk.priority++;
	rv = bcm_field_group_config_create(unit, &grp_elk);
	if (rv) {
	  printf("bcm_field_group_config_create 2 error \n");
	  return rv;
	}

	BCM_FIELD_QSET_REMOVE(grp_elk.qset, bcmFieldQualifySrcMac);
	BCM_FIELD_QSET_ADD(grp_elk.qset, bcmFieldQualifyEtherType);
	BCM_FIELD_ASET_REMOVE(grp_elk.aset, bcmFieldActionExternalValue2Set);
	BCM_FIELD_ASET_ADD(grp_elk.aset, bcmFieldActionExternalValue3Set);
	grp_elk.group++;
	grp_elk.priority++;
	rv = bcm_field_group_config_create(unit, &grp_elk);
	if (rv) {
	  printf("bcm_field_group_config_create 3 error \n");
	  return rv;
	}

	/* Initialize the KBP */
	bshell(unit, "kbp deinit_appl");
	bshell(unit, "kbp init_appl");

	return rv;
}

int acl_kbp_check_256_bit_result_fwd_rpf_add_entry_example(int unit, int hitbit) {

	int rv;
	bcm_field_entry_t ent_elk;

	uint16 vlan_id = 100;
	uint16 EthType = 0x0800;
	bcm_mac_t dst  = {0x00, 0x0c, 0x00, 0x02, 0x00, 0x01};
	bcm_mac_t src  = {0x00, 0x00, 0x07, 0x00, 0x01, 0x00};

	bcm_mac_t mask = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

	bcm_field_entry_create(unit, ACL_BASE_GROUP_INDEX+hitbit, &ent_elk);

	switch (hitbit) {
		case 0:
			rv = bcm_field_qualify_OuterVlanId(unit, ent_elk, vlan_id, 0xFFF);
			if (rv) {
			  printf("bcm_field_qualify_OuterVlan error\n");
			  return rv;
			}
			rv = bcm_field_action_add(unit, ent_elk, bcmFieldActionExternalValue0Set,0x87654321,0xabcdefff);
			if (rv) {
			  printf("bcm_field_action_add error\n");
			  return rv;
			}
			break;
		case 1:
			rv = bcm_field_qualify_DstMac(unit, ent_elk, dst, mask);
			if (rv) {
			  printf("bcm_field_qualify_DstMac error\n");
			  return rv;
			}
			rv = bcm_field_action_add(unit, ent_elk, bcmFieldActionExternalValue1Set,0x87654321,0x12340000);
			if (rv) {
			  printf("bcm_field_action_add error\n");
			  return rv;
			}
			break;
		case 2:
			rv = bcm_field_qualify_SrcMac(unit, ent_elk, src, mask);
			if (rv) {
			  printf("bcm_field_qualify_SrcMac error\n");
			  return rv;
			}
			rv = bcm_field_action_add(unit, ent_elk, bcmFieldActionExternalValue2Set,0x12346578,0xabcdefff);
			if (rv) {
			  printf("bcm_field_action_add error\n");
			  return rv;
			}
			break;
		case 3:
			rv = bcm_field_qualify_EtherType(unit, ent_elk, EthType, 0xFFFF);
			if (rv) {
			  printf("bcm_field_qualify_EtherType error\n");
			  return rv;
			}
			rv = bcm_field_action_add(unit, ent_elk, bcmFieldActionExternalValue3Set,0x12348765,0x00000000);
			if (rv) {
			  printf("bcm_field_action_add error\n");
			  return rv;
			}
			break;
		default:
			printf("Unknown hitbit = %d\n",hitbit);
			return -1;
	}

	rv = bcm_field_entry_prio_set(unit, ent_elk, 10+hitbit);
	if (rv) {
	  printf("bcm_field_entry_prio_set error\n");
	  return rv;
	}

	rv = bcm_field_entry_install(unit, ent_elk);
	if (rv) {
	  printf("bcm_field_entry_install error\n");
	  return rv;
	}

	return rv;

}

int acl_kbp_check_256_bit_result_fwd_rpf_set_pmf_configuration(int unit,int hitbit) {

	int rv;
	uint64 val,mask;
    int presel_id = 1;

	switch (hitbit) {
	case 0:	/* 48 bit */
		COMPILER_64_ZERO(val);
		COMPILER_64_ZERO(mask);
		COMPILER_64_SET(val ,0x0000abcd,0x87654321);
		COMPILER_64_SET(mask,0x0000ffff,0xffffffff);
		rv = configure_pmf_action_by_hit_value_example(unit, HIT_BIT_0_QUAL, val, mask, bcmFieldAppTypeL2, 0, bcmFieldActionDrop, 0, 0, 0, presel_id);
		if (rv) {
			printf("configure_pmf_action_by_hit_value_example bit 0 error\n");
			return  rv;
		}
		break;
	case 1: /* 48 bit */
		COMPILER_64_ZERO(val);
		COMPILER_64_ZERO(mask);
		COMPILER_64_SET(val ,0x00001234,0x87654321);
		COMPILER_64_SET(mask,0x0000ffff,0xffffffff);
		rv = configure_pmf_action_by_hit_value_example(unit, HIT_BIT_1_QUAL, val, mask, bcmFieldAppTypeL2, 0, bcmFieldActionDrop, 0, 0, 0, presel_id);
		if (rv) {
			printf("configure_pmf_action_by_hit_value_example bit 1 error\n");
			return  rv;
		}
		break;
	case 2: /*24 bit*/
		COMPILER_64_ZERO(val);
		COMPILER_64_ZERO(mask);
		COMPILER_64_SET(val ,0x00000000,0x00cdefff);
		COMPILER_64_SET(mask,0x00000000,0x00ffffff);
		rv = configure_pmf_action_by_hit_value_example(unit, HIT_BIT_2_QUAL, val, mask, bcmFieldAppTypeL2, 0, bcmFieldActionDrop, 0, 0, 0, presel_id);
		if (rv) {
			printf("configure_pmf_action_by_hit_value_example bit 2 error\n");
			return  rv;
		}
		break;
	case 3: /*32 bit*/
		COMPILER_64_ZERO(val);
		COMPILER_64_ZERO(mask);
		COMPILER_64_SET(val ,0x00000000,0x12348765);
		COMPILER_64_SET(mask,0x00000000,0xffffffff);
		rv = configure_pmf_action_by_hit_value_example(unit, HIT_BIT_3_QUAL, val, mask, bcmFieldAppTypeL2, 0, bcmFieldActionDrop, 0, 0, 0, presel_id);
		if (rv) {
			printf("configure_pmf_action_by_hit_value_example bit 3 error\n");
			return  rv;
		}
		break;
	}
	return rv;
}

int acl_kbp_ipv6mc_config_example(int unit) {

	int rv=0;
	bcm_field_group_config_t grp_elk;
    int presel_id_elk = 0; /* ID from 0 to 11 for Advanced mode */ 
    int presel_flags = 0; 

    /* Create Preselector */ 
    if(soc_property_get(unit, spn_FIELD_PRESEL_MGMT_ADVANCED_MODE, FALSE)) { 
      presel_flags = BCM_FIELD_QUALIFY_PRESEL_ADVANCED_MODE_STAGE_EXTERNAL; 
      rv = bcm_field_presel_create_stage_id(unit, bcmFieldStageExternal, presel_id_elk); 
      if (BCM_E_NONE != rv) { 
          printf("Error in bcm_field_presel_create_stage_id\n"); 
          return rv; 
      } 
    } else { 
      rv = bcm_field_presel_create(unit, &presel_id_elk); 
      if (BCM_E_NONE != rv) { 
          printf("Error in bcm_field_presel_create\n"); 
          return rv; 
      }
    } 

    rv = bcm_field_qualify_Stage(unit, presel_id_elk | BCM_FIELD_QUALIFY_PRESEL | presel_flags, bcmFieldStageExternal); 
    if (BCM_E_NONE != rv) { 
      printf("Error in bcm_field_qualify_Stage\n"); 
      return rv; 
    } 

	rv = bcm_field_qualify_AppType(unit, presel_id_elk | BCM_FIELD_QUALIFY_PRESEL | presel_flags, bcmFieldAppTypeIp6Mcast);
	if (rv) {
	  printf("bcm_field_qualify_AppType bcmFieldAppTypeIp6Mcast error\n");
	  return rv;
	}

	bcm_field_group_config_t_init(&grp_elk); 
	BCM_FIELD_PRESEL_ADD(grp_elk.preselset, presel_id_elk);
	grp_elk.mode =  bcmFieldGroupModeAuto;
	grp_elk.flags = BCM_FIELD_GROUP_CREATE_WITH_MODE | BCM_FIELD_GROUP_CREATE_WITH_ASET | BCM_FIELD_GROUP_CREATE_WITH_PRESELSET | BCM_FIELD_GROUP_CREATE_WITH_ID;


	BCM_FIELD_QSET_INIT(grp_elk.qset);
	BCM_FIELD_ASET_INIT(grp_elk.aset);
	BCM_FIELD_QSET_ADD(grp_elk.qset, bcmFieldQualifyStageExternal);
	grp_elk.group 	 = ACL_BASE_GROUP_INDEX+1;
	grp_elk.priority = ACL_BASE_GROUP_PRIORITY+1;

	BCM_FIELD_QSET_ADD(grp_elk.qset, bcmFieldQualifyDstMac);
	BCM_FIELD_ASET_ADD(grp_elk.aset, bcmFieldActionExternalValue1Set);
	rv = bcm_field_group_config_create(unit, &grp_elk);
	if (rv) {
	  printf("bcm_field_group_config_create 1 error \n");
	  return rv;
	}

	BCM_FIELD_QSET_REMOVE(grp_elk.qset, bcmFieldQualifyDstMac);
	BCM_FIELD_QSET_ADD(grp_elk.qset, bcmFieldQualifyEtherType);
	BCM_FIELD_ASET_REMOVE(grp_elk.aset, bcmFieldActionExternalValue1Set);
	BCM_FIELD_ASET_ADD(grp_elk.aset, bcmFieldActionExternalValue3Set);
	grp_elk.group 	 = ACL_BASE_GROUP_INDEX+3;
	grp_elk.priority = ACL_BASE_GROUP_PRIORITY+3;

	rv = bcm_field_group_config_create(unit, &grp_elk);
	if (rv) {
	  printf("bcm_field_group_config_create 3 error \n");
	  return rv;
	}

	/* Initialize the KBP */
	bshell(unit, "kbp deinit_appl");
	bshell(unit, "kbp init_appl");

	return rv;
}

int acl_kbp_ipv6mc_add_entry_example(int unit, int hitbit) {

	int rv;
	bcm_field_entry_t ent_elk;

	bcm_mac_t dst  = {0x33, 0x33, 0x03, 0x02, 0x01, 0x00};
	bcm_mac_t mask = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
	uint16 EthType = 0x86dd;

	bcm_field_entry_create(unit, ACL_BASE_GROUP_INDEX+hitbit, &ent_elk);

	switch (hitbit) {
		case 3:
			rv = bcm_field_qualify_EtherType(unit, ent_elk, EthType, 0x0000);
			if (rv) {
			 printf("bcm_field_qualify_EtherType error\n");
			  return rv;
			}
			rv = bcm_field_action_add(unit, ent_elk, bcmFieldActionExternalValue3Set,0x87654321,0xabcde0000);
			if (rv) {
			  printf("bcm_field_action_add error\n");
			  return rv;
			}
			break;
		case 1:
			rv = bcm_field_qualify_DstMac(unit, ent_elk, dst, mask);
			if (rv) {
			  printf("bcm_field_qualify_DstMac error\n");
			  return rv;
			}
			rv = bcm_field_action_add(unit, ent_elk, bcmFieldActionExternalValue1Set,0x87654321,0x12340000);
			if (rv) {
			  printf("bcm_field_action_add error\n");
			  return rv;
			}
			break;
		default:
			printf("Unknown hitbit = %d\n",hitbit);
			return -1;
	}

	rv = bcm_field_entry_prio_set(unit, ent_elk, 10+hitbit);
	if (rv) {
	  printf("bcm_field_entry_prio_set error\n");
	  return rv;
	}

	rv = bcm_field_entry_install(unit, ent_elk);
	if (rv) {
	  printf("bcm_field_entry_install error\n");
	  return rv;
	}

	return rv;

}

int acl_kbp_ipv6mc_set_configuration(int unit) {
	int rv;
    int presel_id = 1; /* ingress presel id for advanced mode: between 1-47 as 0 is taken */

	rv = acl_kbp_ipv6mc_config_example(unit);
	if (rv) {
		printf("acl_kbp_ipv6mc_config_example error\n");
		return  rv;
	}

	rv = configure_pmf_action_by_hit_bit_example(unit,HIT_BIT_3_QUAL,bcmFieldAppTypeIp6Mcast, bcmFieldActionDrop, 0, 0, presel_id);
	if (rv) {
		printf("configure_pmf_action_by_hit_bit_example bit 3 error\n");
		return  rv;
	}

    presel_id++;
	rv = configure_pmf_action_by_hit_bit_example(unit,HIT_BIT_1_QUAL,bcmFieldAppTypeIp6Mcast, bcmFieldActionDrop, 0, 0, presel_id);
	if (rv) {
		printf("configure_pmf_action_by_hit_bit_example bit 1 error\n");
		return  rv;
	}

	return rv;
}


int acl_kbp_acls_4_5_basic_config_example(int unit, bcm_field_AppType_t app_type) {

	int rv=0;
	bcm_field_group_config_t grp_elk;
    int presel_id_elk = 0; /* ID from 0 to 11 for Advanced mode */ 
    int presel_flags = 0; 

    /* Create Preselector */ 
    if(soc_property_get(unit, spn_FIELD_PRESEL_MGMT_ADVANCED_MODE, FALSE)) { 
      presel_flags = BCM_FIELD_QUALIFY_PRESEL_ADVANCED_MODE_STAGE_EXTERNAL; 
      rv = bcm_field_presel_create_stage_id(unit, bcmFieldStageExternal, presel_id_elk); 
      if (BCM_E_NONE != rv) { 
          printf("Error in bcm_field_presel_create_stage_id\n"); 
          return rv; 
      } 
    } else { 
      rv = bcm_field_presel_create(unit, &presel_id_elk); 
      if (BCM_E_NONE != rv) { 
          printf("Error in bcm_field_presel_create\n"); 
          return rv; 
      }
    } 

    rv = bcm_field_qualify_Stage(unit, presel_id_elk | BCM_FIELD_QUALIFY_PRESEL | presel_flags, bcmFieldStageExternal); 
    if (BCM_E_NONE != rv) { 
      printf("Error in bcm_field_qualify_Stage\n"); 
      return rv; 
    } 

    rv = bcm_field_qualify_AppType(unit, presel_id_elk | BCM_FIELD_QUALIFY_PRESEL | presel_flags, app_type);
	if (rv) {
	  printf("bcm_field_qualify_AppType bcmFieldAppTypeIp6Mcast error\n");
	  return rv;
	}

	bcm_field_group_config_t_init(&grp_elk); 
	BCM_FIELD_PRESEL_ADD(grp_elk.preselset, presel_id_elk);
	grp_elk.mode =  bcmFieldGroupModeAuto;
	grp_elk.flags = BCM_FIELD_GROUP_CREATE_WITH_MODE | BCM_FIELD_GROUP_CREATE_WITH_ASET | BCM_FIELD_GROUP_CREATE_WITH_PRESELSET | BCM_FIELD_GROUP_CREATE_WITH_ID;


	BCM_FIELD_QSET_INIT(grp_elk.qset);
	BCM_FIELD_ASET_INIT(grp_elk.aset);
	BCM_FIELD_QSET_ADD(grp_elk.qset, bcmFieldQualifyStageExternal);
	BCM_FIELD_QSET_ADD(grp_elk.qset, bcmFieldQualifyOuterVlanId);

	grp_elk.group 	 = ACL_BASE_GROUP_INDEX + 1;
	grp_elk.priority = ACL_BASE_GROUP_PRIORITY + 1;
	BCM_FIELD_ASET_ADD(grp_elk.aset, bcmFieldActionExternalValue1Set);
	rv = bcm_field_group_config_create(unit, &grp_elk);
	if (rv) {
	  printf("bcm_field_group_config_create 1 error \n");
	  return rv;
	}

	grp_elk.group 	 = ACL_BASE_GROUP_INDEX + 3;
	grp_elk.priority = ACL_BASE_GROUP_PRIORITY + 3;
	BCM_FIELD_ASET_REMOVE(grp_elk.aset, bcmFieldActionExternalValue1Set);
	BCM_FIELD_ASET_ADD(grp_elk.aset, bcmFieldActionExternalValue3Set);
	rv = bcm_field_group_config_create(unit, &grp_elk);
	if (rv) {
	  printf("bcm_field_group_config_create 3 error \n");
	  return rv;
	}

	grp_elk.group 	 = ACL_BASE_GROUP_INDEX + 4;
	grp_elk.priority = ACL_BASE_GROUP_PRIORITY + 4;
	BCM_FIELD_ASET_REMOVE(grp_elk.aset, bcmFieldActionExternalValue3Set);
	BCM_FIELD_ASET_ADD(grp_elk.aset, bcmFieldActionExternalValue4Set);
	rv = bcm_field_group_config_create(unit, &grp_elk);
	if (rv) {
	  printf("bcm_field_group_config_create 4 error \n");
	  return rv;
    }

	grp_elk.group 	 = ACL_BASE_GROUP_INDEX + 5;
	grp_elk.priority = ACL_BASE_GROUP_PRIORITY + 5;
	BCM_FIELD_ASET_REMOVE(grp_elk.aset, bcmFieldActionExternalValue4Set);
	BCM_FIELD_ASET_ADD(grp_elk.aset, bcmFieldActionExternalValue5Set);
	rv = bcm_field_group_config_create(unit, &grp_elk);
	if (rv) {
	  printf("bcm_field_group_config_create 5 error \n");
	  return rv;
    }

	/* Initialize the KBP */
	bshell(unit, "kbp deinit_appl");
	bshell(unit, "kbp init_appl");

	return rv;
}

int acl_kbp_acls_4_5_basic_add_entry_example(int unit, int hitbit) {

	int rv;
	bcm_field_entry_t ent_elk;

	uint16 vlan = 100+hitbit;

	bcm_field_entry_create(unit, ACL_BASE_GROUP_INDEX+hitbit, &ent_elk);
	rv = bcm_field_qualify_OuterVlanId(unit, ent_elk, vlan, 0xFFF);
	if (rv) {
	  printf("bcm_field_qualify_OuterVlanId error\n");
	  return rv;
	}

	switch (hitbit) {
		case 0:
			rv = bcm_field_action_add(unit, ent_elk, bcmFieldActionExternalValue0Set,0x12121212,0x12121212);
			break;
		case 1:
			rv = bcm_field_action_add(unit, ent_elk, bcmFieldActionExternalValue1Set,0x34343434,0x34343434);
			break;
		case 2:
			rv = bcm_field_action_add(unit, ent_elk, bcmFieldActionExternalValue2Set,0x56565656,0x56565656);
			break;
		case 3:
			rv = bcm_field_action_add(unit, ent_elk, bcmFieldActionExternalValue3Set,0x78787878,0x78787878);
			break;
		case 4:
			rv = bcm_field_action_add(unit, ent_elk, bcmFieldActionExternalValue4Set,0xabababab,0xabababab);
			break;
		case 5:
			rv = bcm_field_action_add(unit, ent_elk, bcmFieldActionExternalValue5Set,0xcdcdcdcd,0xcdcdcdcd);
			break;
		default:
			printf("Unknown hitbit = %d\n",hitbit);
			return -1;
	}
	if (rv) {
	  printf("bcm_field_action_add hit bit =%d error\n",hitbit);
	  return rv;
	}

	rv = bcm_field_entry_prio_set(unit, ent_elk, 10+hitbit);
	if (rv) {
	  printf("bcm_field_entry_prio_set error\n");
	  return rv;
	}

	rv = bcm_field_entry_install(unit, ent_elk);
	if (rv) {
	  printf("bcm_field_entry_install error\n");
	  return rv;
	}

	return rv;

}

int acl_kbp_acls_4_5_basic_set_configuration_by_hit_bit(int unit, int is_frwrd_prgm) {
	int rv;
    int presel_id = 1; /* ingress presel id for advanced mode: between 1-47 as 0 is taken */

	bcm_field_AppType_t app_type;
	if (is_frwrd_prgm == 0) {
		app_type = bcmFieldAppTypeL2;
	}else{
		app_type = bcmFieldAppTypeIp4Ucast;
	}
	rv = acl_kbp_acls_4_5_basic_config_example(unit,app_type);
	if (rv) {
		printf("acl_kbp_acls_4_5_basic_config_example error\n");
		return  rv;
	}

	rv = configure_pmf_action_by_hit_bit_example(unit,HIT_BIT_1_QUAL,app_type, bcmFieldActionDrop, 0, 0, presel_id);
	if (rv) {
		printf("configure_pmf_action_by_hit_bit_example bit 1 error\n");
		return  rv;
	}

    presel_id++;
	rv = configure_pmf_action_by_hit_bit_example(unit,HIT_BIT_3_QUAL,app_type, bcmFieldActionDrop, 0, 0, presel_id);
	if (rv) {
		printf("configure_pmf_action_by_hit_bit_example bit 3 error\n");
		return  rv;
	}

    presel_id++;
	rv = configure_pmf_action_by_hit_bit_example(unit,HIT_BIT_4_QUAL,app_type, bcmFieldActionDrop, 0, 0, presel_id);
	if (rv) {
		printf("configure_pmf_action_by_hit_bit_example bit 4 error\n");
		return  rv;
	}

    presel_id++;
	rv = configure_pmf_action_by_hit_bit_example(unit,HIT_BIT_5_QUAL,app_type, bcmFieldActionDrop, 0, 0, presel_id);
	if (rv) {
		printf("configure_pmf_action_by_hit_bit_example bit 5 error\n");
		return  rv;
	}
	return rv;
}

int acl_kbp_acls_4_5_basic_set_configuration_by_results_value(int unit, int is_frwrd_prgm) {
	int rv;
	uint64 val,mask;
	bcm_field_AppType_t app_type;
    int presel_id = 1;

	if (is_frwrd_prgm == 0) {
		app_type = bcmFieldAppTypeL2;
	}else{
		app_type = bcmFieldAppTypeIp4Ucast;
	}

	rv = acl_kbp_acls_4_5_basic_config_example(unit,app_type);
	if (rv) {
		printf("acl_kbp_acls_4_5_basic_config_example error\n");
		return  rv;
	}

	/*search#1 - 32 bit */
	COMPILER_64_ZERO(val);
	COMPILER_64_ZERO(mask);
	COMPILER_64_SET(val ,0x00000000,0x34343434);
	COMPILER_64_SET(mask,0x00000000,0xffffffff);
	rv = configure_pmf_action_by_hit_value_example(unit, HIT_BIT_1_QUAL, val, mask, app_type, 0, bcmFieldActionDrop, 0, 0, 0, presel_id);
	if (rv) {
		printf("configure_pmf_action_by_hit_value_example bit 1 error\n");
		return  rv;
	}

	/*search#3 - 16 bit */
	COMPILER_64_ZERO(val);
	COMPILER_64_ZERO(mask);
	COMPILER_64_SET(val ,0x00000000,0x00007878);
	COMPILER_64_SET(mask,0x00000000,0x0000ffff);
    presel_id++;
	rv = configure_pmf_action_by_hit_value_example(unit, HIT_BIT_3_QUAL, val, mask, app_type, 0, bcmFieldActionDrop, 0, 0, 0, presel_id);
	if (rv) {
		printf("configure_pmf_action_by_hit_value_example bit 3 error\n");
		return  rv;
	}

	/*search#4 - 32 bit */
	COMPILER_64_ZERO(val);
	COMPILER_64_ZERO(mask);
	COMPILER_64_SET(val ,0x00000000,0xabababab);
	COMPILER_64_SET(mask,0x00000000,0xffffffff);
    presel_id++;
	rv = configure_pmf_action_by_hit_value_example(unit, HIT_BIT_4_QUAL, val, mask, app_type, 0, bcmFieldActionDrop, 0, 0, 0, presel_id);
	if (rv) {
		printf("configure_pmf_action_by_hit_value_example bit 4 error\n");
		return  rv;
	}

	/*search#5 - 32 bit */
	COMPILER_64_ZERO(val);
	COMPILER_64_ZERO(mask);
	COMPILER_64_SET(val ,0x00000000,0xcdcdcdcd);
	COMPILER_64_SET(mask,0x00000000,0xffffffff);
    presel_id++;
	rv = configure_pmf_action_by_hit_value_example(unit, HIT_BIT_5_QUAL, val, mask, app_type, 0, bcmFieldActionDrop, 0, 0, 0, presel_id);
	if (rv) {
		printf("configure_pmf_action_by_hit_value_example bit 5 error\n");
		return  rv;
	}
	return rv;
}

int acl_kbp_ipv4uc_ipv4dc_shared_acls_config_example(int unit) {

	int rv=0;
	bcm_field_group_config_t grp_elk;
    int presel_id_elk = 0; /* ID from 0 to 11 for Advanced mode */ 
    int presel_flags = 0; 

    /* Create Preselector */ 
    if(soc_property_get(unit, spn_FIELD_PRESEL_MGMT_ADVANCED_MODE, FALSE)) { 
      presel_flags = BCM_FIELD_QUALIFY_PRESEL_ADVANCED_MODE_STAGE_EXTERNAL; 
      rv = bcm_field_presel_create_stage_id(unit, bcmFieldStageExternal, presel_id_elk); 
      if (BCM_E_NONE != rv) { 
          printf("Error in bcm_field_presel_create_stage_id\n"); 
          return rv; 
      } 
    } else { 
      rv = bcm_field_presel_create(unit, &presel_id_elk); 
      if (BCM_E_NONE != rv) { 
          printf("Error in bcm_field_presel_create\n"); 
          return rv; 
      }
    } 

    rv = bcm_field_qualify_Stage(unit, presel_id_elk | BCM_FIELD_QUALIFY_PRESEL | presel_flags, bcmFieldStageExternal); 
    if (BCM_E_NONE != rv) { 
      printf("Error in bcm_field_qualify_Stage\n"); 
      return rv; 
    } 

	rv = bcm_field_qualify_AppType(unit, presel_id_elk | BCM_FIELD_QUALIFY_PRESEL | presel_flags, bcmFieldAppTypeIp4DoubleCapacity);
	if (rv) {
	  printf("bcm_field_qualify_AppType bcmFieldAppTypeIp4DoubleCapacity error\n");
	  return rv;
	}

	rv = bcm_field_qualify_AppType(unit, presel_id_elk | BCM_FIELD_QUALIFY_PRESEL | presel_flags, bcmFieldAppTypeIp4UcastRpf);
	if (rv) {
	  printf("bcm_field_qualify_AppType bcmFieldAppTypeIp4UcastRpf error\n");
	  return rv;
	}

    rv = bcm_field_qualify_AppType(unit, presel_id_elk | BCM_FIELD_QUALIFY_PRESEL | presel_flags, bcmFieldAppTypeIp4Ucast);
	if (rv) {
	  printf("bcm_field_qualify_AppType bcmFieldAppTypeIp4Ucast error\n");
	  return rv;
	}

	bcm_field_group_config_t_init(&grp_elk); 
	BCM_FIELD_PRESEL_ADD(grp_elk.preselset, presel_id_elk);
	grp_elk.mode =  bcmFieldGroupModeAuto;
	grp_elk.flags = BCM_FIELD_GROUP_CREATE_WITH_MODE | BCM_FIELD_GROUP_CREATE_WITH_ASET | BCM_FIELD_GROUP_CREATE_WITH_PRESELSET | BCM_FIELD_GROUP_CREATE_WITH_ID;

	BCM_FIELD_QSET_INIT(grp_elk.qset);
	BCM_FIELD_ASET_INIT(grp_elk.aset);
	BCM_FIELD_QSET_ADD(grp_elk.qset, bcmFieldQualifyStageExternal);

    /*
     * Create ACL group on search#1 
     * SrcMac 
    */
	BCM_FIELD_QSET_ADD(grp_elk.qset, bcmFieldQualifySrcMac);
	BCM_FIELD_ASET_ADD(grp_elk.aset, bcmFieldActionExternalValue1Set);
	grp_elk.group 	 = ACL_BASE_GROUP_INDEX + 1;
	grp_elk.priority = ACL_BASE_GROUP_PRIORITY + 1;
	rv = bcm_field_group_config_create(unit, &grp_elk);
	if (rv) {
	  printf("bcm_field_group_config_create 1 error \n");
	  return rv;
	}
	BCM_FIELD_QSET_REMOVE(grp_elk.qset, bcmFieldQualifySrcMac);
	BCM_FIELD_ASET_REMOVE(grp_elk.aset, bcmFieldActionExternalValue1Set);

    /*
     * Create ACL group on search#3 
     * EtherType 
    */
	BCM_FIELD_QSET_ADD(grp_elk.qset, bcmFieldQualifyEtherType);
	BCM_FIELD_ASET_ADD(grp_elk.aset, bcmFieldActionExternalValue3Set);
	grp_elk.group 	 = ACL_BASE_GROUP_INDEX + 3;
	grp_elk.priority = ACL_BASE_GROUP_PRIORITY + 3;
	rv = bcm_field_group_config_create(unit, &grp_elk);
	if (rv) {
	  printf("bcm_field_group_config_create 3 error \n");
	  return rv;
    }
	BCM_FIELD_QSET_REMOVE(grp_elk.qset, bcmFieldQualifyEtherType);
	BCM_FIELD_ASET_REMOVE(grp_elk.aset, bcmFieldActionExternalValue3Set);

    /*
     * Create ACL group on search#4 
     * SrcIp 
    */
	BCM_FIELD_QSET_ADD(grp_elk.qset, bcmFieldQualifySrcIp);
	BCM_FIELD_ASET_ADD(grp_elk.aset, bcmFieldActionExternalValue4Set);
	grp_elk.group 	 = ACL_BASE_GROUP_INDEX + 4;
	grp_elk.priority = ACL_BASE_GROUP_PRIORITY + 4;
	rv = bcm_field_group_config_create(unit, &grp_elk);
	if (rv) {
	  printf("bcm_field_group_config_create 4 error \n");
	  return rv;
    }
	BCM_FIELD_QSET_REMOVE(grp_elk.qset, bcmFieldQualifySrcIp);
	BCM_FIELD_ASET_REMOVE(grp_elk.aset, bcmFieldActionExternalValue4Set);

	/* Initialize the KBP */
	bshell(unit, "kbp deinit_appl");
	bshell(unit, "kbp init_appl");

    return rv;
}

int acl_kbp_ipv4uc_ipv4dc_shared_acls_add_entry_example(int unit, int hitbit) {

	int rv;
	bcm_field_entry_t ent_elk;

	uint16 EthType = 0x0800;
	
	bcm_mac_t src  = {0x00, 0x00, 0x07, 0x00, 0x01, 0x00};
	bcm_mac_t mask = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
	
	uint32 srcIp  = 0xc0800101;
	uint32 maskIp = 0xffffffff;
	bcm_field_entry_create(unit, ACL_BASE_GROUP_INDEX+hitbit, &ent_elk);

	switch (hitbit) {
	case HIT_BIT_1_QUAL:
		rv = bcm_field_qualify_SrcMac(unit, ent_elk, src, mask);
		if (rv) {
		  printf("bcm_field_qualify_SrcMac error\n");
		  return rv;
		}
		rv = bcm_field_action_add(unit, ent_elk, bcmFieldActionExternalValue1Set,0x12121212,0x12121212);
		break;
	case HIT_BIT_3_QUAL:
		rv = bcm_field_qualify_EtherType(unit, ent_elk, EthType, 0xFFFF);
		if (rv) {
		  printf("bcm_field_qualify_EtherType error\n");
		  return rv;
		}
		rv = bcm_field_action_add(unit, ent_elk, bcmFieldActionExternalValue3Set,0x34343434,0x34343434);
		break;
	case HIT_BIT_4_QUAL:
		rv = bcm_field_qualify_SrcIp(unit, ent_elk, srcIp, maskIp);
		if (rv) {
		  printf("bcm_field_qualify_SrcIp error\n");
		  return rv;
		}
		rv = bcm_field_action_add(unit, ent_elk, bcmFieldActionExternalValue4Set,0x56565656,0x56565656);
		break;
	default:
		printf("Unknown hitbit = %d\n",hitbit);
		return -1;
	}
	if (rv) {
	  printf("bcm_field_action_add hit bit =%d error\n",hitbit);
	  return rv;
	}

	rv = bcm_field_entry_prio_set(unit, ent_elk, 10+hitbit);
	if (rv) {
	  printf("bcm_field_entry_prio_set error\n");
	  return rv;
	}

	rv = bcm_field_entry_install(unit, ent_elk);
	if (rv) {
	  printf("bcm_field_entry_install error\n");
	  return rv;
	}

	return rv;
}

int acl_kbp_ipv4uc_ipv4dc_shared_acls_set_configuration_by_results_value(int unit, int hitbit) {
	int rv;
	uint64 val,mask;
    int presel_id = 1;

	switch (hitbit) {
	case HIT_BIT_1_QUAL:
		/*search#1 - 24 bit */
		COMPILER_64_ZERO(val);
		COMPILER_64_ZERO(mask);
		COMPILER_64_SET(val ,0x00000000,0x00001212);
		COMPILER_64_SET(mask,0x00000000,0x0000ffff);
		rv = configure_pmf_action_by_hit_value_example(unit, HIT_BIT_1_QUAL, val, mask, bcmFieldAppTypeIp4Ucast, bcmFieldAppTypeIp4DoubleCapacity, bcmFieldActionDrop, 0, 0, 0, presel_id);
		if (rv) {
			printf("configure_pmf_action_by_hit_value_example bit 1 bcmFieldAppTypeIp4Ucast error\n");
			return  rv;
		}
		break;
	case HIT_BIT_3_QUAL:
		/*search#3 - 16 bit */
		COMPILER_64_ZERO(val);
		COMPILER_64_ZERO(mask);
		COMPILER_64_SET(val ,0x00000000,0x34343434);
		COMPILER_64_SET(mask,0x00000000,0xffffffff);
		rv = configure_pmf_action_by_hit_value_example(unit, HIT_BIT_3_QUAL, val, mask, bcmFieldAppTypeIp4Ucast, bcmFieldAppTypeIp4DoubleCapacity, bcmFieldActionDrop, 0, 0, 0, presel_id);
		if (rv) {
			printf("configure_pmf_action_by_hit_value_example bit 3 bcmFieldAppTypeIp4Ucast error\n");
			return  rv;
		}
		break;
	case HIT_BIT_4_QUAL:
		/*search#4 - 32 bit */
		COMPILER_64_ZERO(val);
		COMPILER_64_ZERO(mask);
		COMPILER_64_SET(val ,0x00000000,0x00000000);
		COMPILER_64_SET(mask,0x00000000,0xffffffff);
		rv = configure_pmf_action_by_hit_value_example(unit, HIT_BIT_3_QUAL, val, mask, bcmFieldAppTypeIp4Ucast, bcmFieldAppTypeIp4DoubleCapacity, bcmFieldActionDrop, 0, 0, 20, presel_id);
		if (rv) {
			printf("configure_pmf_action_by_hit_value_example bit 3 bcmFieldAppTypeIp4Ucast error\n");
			return  rv;
		}
		COMPILER_64_ZERO(val);
		COMPILER_64_ZERO(mask);
		COMPILER_64_SET(val ,0x00000000,0x56565656);
		COMPILER_64_SET(mask,0x00000000,0xffffffff);
        presel_id++;
		rv = configure_pmf_action_by_hit_value_example(unit, HIT_BIT_4_QUAL, val, mask, bcmFieldAppTypeIp4Ucast, bcmFieldAppTypeIp4DoubleCapacity, bcmFieldActionDrop, 0, 0, 0, presel_id);
		if (rv) {
			printf("configure_pmf_action_by_hit_value_example bit 4 bcmFieldAppTypeIp4Ucast error\n");
			return  rv;
		}
		break;
	default:
		printf("Unknown hit bit %d\n",hitbit);
		return -1;
	}
	return rv;
}

int acl_kbp_ipv4ucrpf_ipv4dc_shared_acls_set_configuration_by_results_value(int unit, int hitbit) {
	int rv;
	uint64 val,mask;
    int presel_id = 1;

	switch (hitbit) {
	case HIT_BIT_1_QUAL:
		/*search#1 - 24 bit */
		COMPILER_64_ZERO(val);
		COMPILER_64_ZERO(mask);
		COMPILER_64_SET(val ,0x00000000,0x00001212);
		COMPILER_64_SET(mask,0x00000000,0x0000ffff);
		rv = configure_pmf_action_by_hit_value_example(unit, HIT_BIT_1_QUAL, val, mask, bcmFieldAppTypeIp4UcastRpf, bcmFieldAppTypeIp4DoubleCapacity, bcmFieldActionDrop, 0, 0, 0, presel_id);
		if (rv) {
			printf("configure_pmf_action_by_hit_value_example bit 1 bcmFieldAppTypeIp4UcastRpf error\n");
			return  rv;
		}
		break;
	case HIT_BIT_3_QUAL:
		/*search#3 - 16 bit */
		COMPILER_64_ZERO(val);
		COMPILER_64_ZERO(mask);
		COMPILER_64_SET(val ,0x00000000,0x34343434);
		COMPILER_64_SET(mask,0x00000000,0xffffffff);
		rv = configure_pmf_action_by_hit_value_example(unit, HIT_BIT_3_QUAL, val, mask, bcmFieldAppTypeIp4UcastRpf, bcmFieldAppTypeIp4DoubleCapacity, bcmFieldActionDrop, 0, 0, 0, presel_id);
		if (rv) {
			printf("configure_pmf_action_by_hit_value_example bit 3 bcmFieldAppTypeIp4UcastRpf error\n");
			return  rv;
		}
		break;
	case HIT_BIT_4_QUAL:
		COMPILER_64_ZERO(val);
		COMPILER_64_ZERO(mask);
		COMPILER_64_SET(val ,0x00000000,0x56565656);
		COMPILER_64_SET(mask,0x00000000,0xffffffff);
		rv = configure_pmf_action_by_hit_value_example(unit, HIT_BIT_4_QUAL, val, mask, bcmFieldAppTypeIp4UcastRpf, bcmFieldAppTypeIp4DoubleCapacity, bcmFieldActionDrop, 0, 0, 0, presel_id);
		if (rv) {
			printf("configure_pmf_action_by_hit_value_example bit 4 bcmFieldAppTypeIp4UcastRpf error\n");
			return  rv;
		}
		break;
	default:
		printf("Unknown hit bit %d\n",hitbit);
		return -1;
	}
	return rv;
}

/*
*   add an ACL entry according to the test: AT_Cint_external_frwd_kbp_mpls_acl_traffic
*/ 
int frwd_kbp_mpls_acls_kbp_entry_add_example(int unit, int index) {

   int i, rv;
   bcm_field_entry_t ent_elk;

   bcm_mac_t d_mac = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x11 };
   bcm_mac_t d_mac_mask = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };

   bcm_vlan_t vlan = 102;
   bcm_vlan_t vlan_mask = 0xFFF;

   bcm_field_entry_create(unit, ACL_BASE_GROUP_INDEX + index, &ent_elk);

   switch (index) {
      case 1:/* 1st: dmac */
         rv = bcm_field_qualify_DstMac(unit, ent_elk, d_mac, d_mac_mask);
         if (rv) {
            printf("bcm_field_qualify_DstMac error, index = %d\n", index);
            return rv;
         }
         break;
      case 3:/* 2nd: vlan */
         rv = bcm_field_qualify_OuterVlanId(unit, ent_elk, vlan, vlan_mask);
         if (rv) {
            printf("bcm_field_qualify_OuterVlanId error, index = %d\n", index);
            return rv;
         }
         break;
      default:
         printf("Unknown index value = %d",index);
         return -1;
   }

   rv = bcm_field_entry_prio_set(unit, ent_elk, 10);
   if (rv) {
      printf("bcm_field_entry_prio_set error\n");
      return rv;
   }

   rv = bcm_field_entry_install(unit, ent_elk);
   if (rv) {
      printf("bcm_field_entry_install error\n");
      return rv;
   }

   return rv;
}

/*
 * configure MPLS ACL with 2 lookups: 
 * 1 - dmac
 * 2 - vlan
*/
int frwd_kbp_mpls_acls_kbp_config_example(int unit) {

    int rv=0;
    bcm_field_group_config_t grp_elk;
    int presel_id_elk = 0; /* ID from 0 to 11 for Advanced mode */ 
    int presel_flags = 0; 

    /* Create Preselector */ 
    if(soc_property_get(unit, spn_FIELD_PRESEL_MGMT_ADVANCED_MODE, FALSE)) { 
      presel_flags = BCM_FIELD_QUALIFY_PRESEL_ADVANCED_MODE_STAGE_EXTERNAL; 
      rv = bcm_field_presel_create_stage_id(unit, bcmFieldStageExternal, presel_id_elk); 
      if (BCM_E_NONE != rv) { 
          printf("Error in bcm_field_presel_create_stage_id\n"); 
          return rv; 
      } 
    } else { 
      rv = bcm_field_presel_create(unit, &presel_id_elk); 
      if (BCM_E_NONE != rv) { 
          printf("Error in bcm_field_presel_create\n"); 
          return rv; 
      }
    } 

    rv = bcm_field_qualify_Stage(unit, presel_id_elk | BCM_FIELD_QUALIFY_PRESEL | presel_flags, bcmFieldStageExternal); 
    if (BCM_E_NONE != rv) { 
      printf("Error in bcm_field_qualify_Stage\n"); 
      return rv; 
    } 

   rv = bcm_field_qualify_AppType(unit, presel_id_elk | BCM_FIELD_QUALIFY_PRESEL | presel_flags, bcmFieldAppTypeMpls);
   if (rv) {
      printf("bcm_field_qualify_AppType Mpls error\n");
      return rv;
   }

   bcm_field_group_config_t_init(&grp_elk); 
   BCM_FIELD_PRESEL_ADD(grp_elk.preselset, presel_id_elk);
   grp_elk.mode = bcmFieldGroupModeAuto;
   grp_elk.flags = BCM_FIELD_GROUP_CREATE_WITH_MODE | BCM_FIELD_GROUP_CREATE_WITH_ASET | 
BCM_FIELD_GROUP_CREATE_WITH_PRESELSET | BCM_FIELD_GROUP_CREATE_WITH_ID;

   BCM_FIELD_QSET_INIT(grp_elk.qset);
   BCM_FIELD_ASET_INIT(grp_elk.aset);
   BCM_FIELD_QSET_ADD(grp_elk.qset, bcmFieldQualifyStageExternal);

   /* 1st: dmac */
   BCM_FIELD_QSET_ADD(grp_elk.qset, bcmFieldQualifyDstMac);
   BCM_FIELD_ASET_ADD(grp_elk.aset, bcmFieldActionExternalValue1Set);
   grp_elk.group = ACL_BASE_GROUP_INDEX + 1;
   grp_elk.priority = ACL_BASE_GROUP_PRIORITY;

   rv = bcm_field_group_config_create(unit, &grp_elk);
   if (rv) {
      printf("bcm_field_group_config_create 1 error\n");
      return rv;
   }

   /* 2nd: vlan */
   BCM_FIELD_ASET_REMOVE(grp_elk.aset, bcmFieldActionExternalValue1Set);
   BCM_FIELD_ASET_ADD(grp_elk.aset, bcmFieldActionExternalValue3Set);
   BCM_FIELD_QSET_REMOVE(grp_elk.qset, bcmFieldQualifyDstMac);
   BCM_FIELD_QSET_ADD(grp_elk.qset, bcmFieldQualifyOuterVlanId);
   grp_elk.group = ACL_BASE_GROUP_INDEX + 3;
   grp_elk.priority++;

   rv = bcm_field_group_config_create(unit, &grp_elk);
   if (rv) {
      printf("bcm_field_group_config_create 3 error\n");
      return rv;
   }

   /* Initialize the KBP */
   bshell(unit, "kbp deinit_appl");
   bshell(unit, "kbp init_appl");

   return rv;
}

/*
 * set configuration of KBP tables and PMF dropping action 
*/
int frwd_kbp_mpls_acls_kbp_set_configurations(int unit) {
   int rv;
   int presel_id = 1; /* ingress presel id for advanced mode: between 1-47 as 0 is taken */

   rv = frwd_kbp_mpls_acls_kbp_config_example(unit);
   if(rv) {
      printf("acl_kbp_mpls_acls_kbp_config_example error\n");
      return  rv;
   }

   rv = configure_pmf_action_by_hit_bit_example(unit,HIT_BIT_1_QUAL,bcmFieldAppTypeMpls,bcmFieldActionDrop,0,0, presel_id);
   if(rv) {
      printf("configure_pmf_action_by_hit_bit_example bit 1 error\n");
      return  rv;
   }

   presel_id++;
   rv = configure_pmf_action_by_hit_bit_example(unit,HIT_BIT_3_QUAL,bcmFieldAppTypeMpls,bcmFieldActionDrop,0,0, presel_id);
   if(rv) {
      printf("configure_pmf_action_by_hit_bit_example bit 3 error\n");
      return  rv;
   }

   return rv;
}

/*
*   add an ACL entry according to the test: AT_Cint_external_no_frwd_kbp_acl_traffic
*/ 
int no_frwd_kbp_acl_entry_add_example(int unit, int index, int mac2, int ip6) {

   int i, rv;
   bcm_field_entry_t ent_elk;

   bcm_mac_t s_mac1 = { 0x11, 0x00, 0x00, 0x00, 0x00, 0x11 };
   bcm_mac_t s_mac2 = { 0x22, 0x00, 0x00, 0x00, 0x00, 0x22 };
   bcm_mac_t mac_mask = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };

   int ext_val1 = ip6 ? 0x00000066 : 0x00000044;
   int ext_val2 = ip6 ? 0x00000066 : 0x00000044;

   bcm_vlan_t vlan = ip6 ? 17 : 200;
   bcm_vlan_t vlan_mask = 0xfff;

   bcm_field_entry_create(unit, ACL_BASE_GROUP_INDEX + index, &ent_elk);

   switch (index) {
      case 1:/* 1st: smac */
         rv = bcm_field_qualify_SrcMac(unit, ent_elk, (mac2 ? s_mac2 : s_mac1), mac_mask);
         if (rv) {
            printf("bcm_field_qualify_SrcMac error, index = %d\n", index);
            return rv;
         }
         rv = bcm_field_action_add(unit, ent_elk, bcmFieldActionExternalValue1Set, ext_val1, ext_val2);
         if (rv) {
            printf("bcm_field_action_add error value1set, ext_val = %d\n", ext_val1);
            return rv;
         }
         break;
      case 4:/* 2nd: vlan */
         rv = bcm_field_qualify_OuterVlanId(unit, ent_elk, vlan, vlan_mask);
         if (rv) {
            printf("bcm_field_qualify_OuterVlanId error, index = %d\n", index);
            return rv;
         }
         rv = bcm_field_action_add(unit, ent_elk, bcmFieldActionExternalValue4Set, ext_val1, ext_val2);
         if (rv) {
            printf("bcm_field_action_add error value3set, ext_val = %d\n", ext_val1);
            return rv;
         }
         break;
      default:
         printf("Unknown index value = %d", index);
         return -1;
   }

   rv = bcm_field_entry_prio_set(unit, ent_elk, 10 + index);
   if (rv) {
      printf("bcm_field_entry_prio_set error\n");
      return rv;
   }

   rv = bcm_field_entry_install(unit, ent_elk);
   if (rv) {
      printf("bcm_field_entry_install error\n");
      return rv;
   }

   return rv;
}

/*
    no_frwd_kbp_acl_config
    1. smac
    2. vlan
*/
int no_frwd_kbp_acl_config_example(int unit, int ip6) {

   int rv = 0;
   bcm_field_group_config_t grp_elk;
   bcm_field_AppType_t appType = ip6 ? bcmFieldAppTypeIp6Ucast : bcmFieldAppTypeIp4Ucast;
   int presel_id_elk = 0; /* ID from 0 to 11 for Advanced mode */ 
   int presel_flags = 0; 

   /* Create Preselector */ 
   if(soc_property_get(unit, spn_FIELD_PRESEL_MGMT_ADVANCED_MODE, FALSE)) { 
     presel_flags = BCM_FIELD_QUALIFY_PRESEL_ADVANCED_MODE_STAGE_EXTERNAL; 
     rv = bcm_field_presel_create_stage_id(unit, bcmFieldStageExternal, presel_id_elk); 
     if (BCM_E_NONE != rv) { 
         printf("Error in bcm_field_presel_create_stage_id\n"); 
         return rv; 
     } 
   } else { 
     rv = bcm_field_presel_create(unit, &presel_id_elk); 
     if (BCM_E_NONE != rv) { 
         printf("Error in bcm_field_presel_create\n"); 
         return rv; 
     }
   } 

   rv = bcm_field_qualify_Stage(unit, presel_id_elk | BCM_FIELD_QUALIFY_PRESEL | presel_flags, bcmFieldStageExternal); 
   if (BCM_E_NONE != rv) { 
     printf("Error in bcm_field_qualify_Stage\n"); 
     return rv; 
   } 

   rv = bcm_field_qualify_AppType(unit, presel_id_elk | BCM_FIELD_QUALIFY_PRESEL | presel_flags, appType);
   if (rv) {
      printf("bcm_field_qualify_AppType error %d\n", ip6);
      return rv;
   }

   bcm_field_group_config_t_init(&grp_elk); 
   BCM_FIELD_PRESEL_ADD(grp_elk.preselset, presel_id_elk);
   grp_elk.mode = bcmFieldGroupModeAuto;
   grp_elk.flags =
            BCM_FIELD_GROUP_CREATE_WITH_MODE |
            BCM_FIELD_GROUP_CREATE_WITH_ASET |
            BCM_FIELD_GROUP_CREATE_WITH_PRESELSET |
            BCM_FIELD_GROUP_CREATE_WITH_ID;

   BCM_FIELD_QSET_INIT(grp_elk.qset);
   BCM_FIELD_ASET_INIT(grp_elk.aset);
   BCM_FIELD_QSET_ADD(grp_elk.qset, bcmFieldQualifyStageExternal);

   /* 1st: smac */
   BCM_FIELD_QSET_ADD(grp_elk.qset, bcmFieldQualifySrcMac);
   BCM_FIELD_ASET_ADD(grp_elk.aset, bcmFieldActionExternalValue1Set);
   grp_elk.group = ACL_BASE_GROUP_INDEX + 1;
   grp_elk.priority = ACL_BASE_GROUP_PRIORITY;

   rv = bcm_field_group_config_create(unit, &grp_elk);
   if (rv) {
      printf("bcm_field_group_config_create 1 error\n");
      return rv;
   }

   /* 2nd: vlan */
   BCM_FIELD_ASET_REMOVE(grp_elk.aset, bcmFieldActionExternalValue1Set);
   BCM_FIELD_ASET_ADD(grp_elk.aset, bcmFieldActionExternalValue4Set);
   BCM_FIELD_QSET_REMOVE(grp_elk.qset, bcmFieldQualifySrcMac);
   BCM_FIELD_QSET_ADD(grp_elk.qset, bcmFieldQualifyOuterVlanId);
   grp_elk.group = ACL_BASE_GROUP_INDEX + 4;
   grp_elk.priority++;

   rv = bcm_field_group_config_create(unit, &grp_elk);
   if (rv) {
      printf("bcm_field_group_config_create 3 error\n");
      return rv;
   }

   /* Initialize the KBP */
   bshell(unit, "kbp deinit_appl");
   bshell(unit, "kbp init_appl");

   return rv;
}

/*
 * set configuration of KBP tables and PMF dropping action 
*/
int no_frwd_kbp_acl_set_configurations(int unit, int ip6) {
   int rv = 0;
   uint64 hit_value, hit_mask;
   int presel_id = 1;

   rv = no_frwd_kbp_acl_config_example(unit, ip6);
   if(rv) {
      printf("no_frwd_kbp_acl_config_example error\n");
      return rv;
   }

   switch(ip6) {
       case 0:
           COMPILER_64_ZERO(hit_value);
           COMPILER_64_ZERO(hit_mask);
           COMPILER_64_SET(hit_value, 0x00000000, 0x00000044);
           COMPILER_64_SET(hit_mask, 0x00000000, 0x00000044);
           rv = configure_pmf_action_by_hit_value_example(unit,HIT_BIT_1_QUAL,hit_value,hit_mask,bcmFieldAppTypeIp4Ucast,0,bcmFieldActionDrop,0,0,0, presel_id);
           if(rv) {
              printf("configure_pmf_action_by_hit_value_example bit 1 value 4 error\n");
              return rv;
           }

           presel_id++;
           rv = configure_pmf_action_by_hit_value_example(unit,HIT_BIT_4_QUAL,hit_value,hit_mask,bcmFieldAppTypeIp4Ucast,0,bcmFieldActionDrop,0,0,0, presel_id);
           if(rv) {
              printf("configure_pmf_action_by_hit_value_example bit 3 value 4 error\n");
              return rv;
           }
           break;
       default:
           COMPILER_64_ZERO(hit_value);
           COMPILER_64_ZERO(hit_mask);
           COMPILER_64_SET(hit_value, 0x00000000, 0x00000066);
           COMPILER_64_SET(hit_mask, 0x00000000, 0x00000066);
           rv = configure_pmf_action_by_hit_value_example(unit,HIT_BIT_1_QUAL,hit_value,hit_mask,bcmFieldAppTypeIp6Ucast,0,bcmFieldActionDrop,0,0,6, presel_id);
           if(rv) {
              printf("configure_pmf_action_by_hit_value_example bit 1 value 6 error\n");
              return rv;
           }

           presel_id++;
           rv = configure_pmf_action_by_hit_value_example(unit,HIT_BIT_4_QUAL,hit_value,hit_mask,bcmFieldAppTypeIp6Ucast,0,bcmFieldActionDrop,0,0,6, presel_id);
           if(rv) {
              printf("configure_pmf_action_by_hit_value_example bit 3 value 6 error\n");
              return rv;
           }
           break;
   }

   return rv;
}

int msb_qualifier_dq_create(int unit, int dq_id, int length, int offset)
{
    int	rv = BCM_E_NONE;
    bcm_field_data_qualifier_t  data_qual;

    bcm_field_data_qualifier_t_init(&data_qual);

    data_qual.flags = BCM_FIELD_DATA_QUALIFIER_OFFSET_PREDEFINED |
                      BCM_FIELD_DATA_QUALIFIER_OFFSET_BIT_RES    |
                      BCM_FIELD_DATA_QUALIFIER_WITH_ID    |
                      BCM_FIELD_DATA_QUALIFIER_LENGTH_BIT_RES;
    data_qual.stage = bcmFieldStageExternal;

    data_qual.qualifier = bcmFieldQualifyInVPortWide;
    data_qual.offset = offset;
    data_qual.length = length;
    data_qual.qual_id = dq_id;

    rv = bcm_field_data_qualifier_create(unit, &data_qual);
    if (rv != BCM_E_NONE) {
        printf("DQ %d create failed rv=%d\n", dq_id, rv);
        return (rv);
    }
    return (rv);
}

int msb_qualifier_kbp_acl_config_example(int unit, int hit_bit) {

   int rv = 0;

   int presel_id_elk;
   bcm_field_group_config_t grp_elk;
   int action;

   rv = bcm_field_presel_create(unit, &presel_id_elk);
   if (rv) {
      printf("bcm_field_presel_create error\n");
      return rv;
   }

   rv = bcm_field_qualify_Stage(unit, presel_id_elk | BCM_FIELD_QUALIFY_PRESEL, bcmFieldStageExternal);
   if (rv) {
      printf("bcm_field_qualify_Stage error\n");
      return rv;
   }

   rv = bcm_field_qualify_AppType(unit, presel_id_elk | BCM_FIELD_QUALIFY_PRESEL, bcmFieldAppTypeIp4Ucast);
   if (rv) {
      printf("bcm_field_qualify_AppType error\n");
      return rv;
   }

   if (hit_bit == HIT_BIT_1_QUAL)
   {
       rv = msb_qualifier_dq_create(unit, ACL_BASE_DQ_ID + HIT_BIT_1_QUAL, 32, 0);
       if (rv) {
          printf("msb_qualifier_dq_create error, dq_id=51\n");
          return rv;
       }
       action = bcmFieldActionExternalValue1Set;
   } else if(hit_bit == HIT_BIT_3_QUAL){
       rv = msb_qualifier_dq_create(unit, ACL_BASE_DQ_ID+HIT_BIT_3_QUAL, 32, 16);
       if (rv) {
          printf("msb_qualifier_dq_create error, dq_id=53\n");
          return rv;
       }
       action = bcmFieldActionExternalValue3Set;
   } else if(hit_bit == HIT_BIT_4_QUAL){
       rv = msb_qualifier_dq_create(unit, ACL_BASE_DQ_ID+HIT_BIT_4_QUAL, 16, 4);
       if (rv) {
          printf("msb_qualifier_dq_create error, dq_id=54\n");
          return rv;
       }
       action = bcmFieldActionExternalValue4Set;
   }

   bcm_field_group_config_t_init(&grp_elk); 
   BCM_FIELD_PRESEL_ADD(grp_elk.preselset, presel_id_elk);
   grp_elk.mode = bcmFieldGroupModeAuto;
   grp_elk.flags = BCM_FIELD_GROUP_CREATE_WITH_MODE |
       BCM_FIELD_GROUP_CREATE_WITH_ASET |
       BCM_FIELD_GROUP_CREATE_WITH_PRESELSET |
       BCM_FIELD_GROUP_CREATE_WITH_ID;

   BCM_FIELD_QSET_INIT(grp_elk.qset);
   BCM_FIELD_ASET_INIT(grp_elk.aset);
   BCM_FIELD_QSET_ADD(grp_elk.qset, bcmFieldQualifyStageExternal);

   rv = bcm_field_qset_data_qualifier_add(unit, &grp_elk.qset, ACL_BASE_DQ_ID+hit_bit);
   if (rv) {
     printf("bcm_field_qset_data_qualifier_add failed rv=%d\n", rv);
     return rv;
   }
   BCM_FIELD_ASET_ADD(grp_elk.aset, action);
   grp_elk.group = ACL_BASE_GROUP_INDEX + hit_bit;
   grp_elk.priority = ACL_BASE_GROUP_PRIORITY + hit_bit;

   rv = bcm_field_group_config_create(unit, &grp_elk);
   if (rv) {
      printf("bcm_field_group_config_create error\n");
      return rv;
   }

   /* Initialize the KBP */
   bshell(unit, "kbp deinit_appl");
   bshell(unit, "kbp init_appl");

   return rv;
}

int msb_qualifier_kbp_acl_entry_add_example(int unit, int hit_bit) {

   int rv;
   int action;
   bcm_field_entry_t ent_elk;

   uint8 qual_data[4][4] = {
       {0x87,0x65,0x43,0x21}, /*hitbit=1*/ 
       {0xff,0xff,0xff,0xff}, /*hitbit=2 (invalid)*/ 
       {0xdc,0xba,0x87,0x65}, /*hitbit=3*/ 
       {0x54,0x32,0x00,0x00}  /*hitbit=4*/ 
   };

   uint8 qual_mask[4][4] = {
       {0xff,0xff,0xff,0xff}, /*hitbit=1*/ 
       {0xff,0xff,0xff,0xff}, /*hitbit=2 (invalid)*/ 
       {0xff,0xff,0xff,0xff}, /*hitbit=3*/ 
       {0xff,0xff,0x00,0x00}, /*hitbit=4*/ 
   };

   int qual_length[4] =  {
       4,
       0, /*invalid*/
       4,
       2
   };

   uint32 act_val1[4] =  {
       0x0000abab,
       0xffffffff, /*invalid*/
       0x12121212,
       0x56565656,
   };
   uint32 act_val2[4] =  {
       0x00000000,
       0xffffffff, /*invalid*/
       0x00000000,
       0x00000000,
   };

   bcm_field_entry_create(unit, ACL_BASE_GROUP_INDEX + hit_bit, &ent_elk);

   if (hit_bit == HIT_BIT_1_QUAL){
       action = bcmFieldActionExternalValue1Set;
   } else if(hit_bit == HIT_BIT_3_QUAL){
       action = bcmFieldActionExternalValue3Set;
   } else if(hit_bit == HIT_BIT_4_QUAL){
       action = bcmFieldActionExternalValue4Set;
   }

   rv = bcm_field_qualify_data(unit, ent_elk, ACL_BASE_DQ_ID+hit_bit,
                               qual_data[hit_bit-1], qual_mask[hit_bit-1], qual_length[hit_bit-1]);
   if (BCM_E_NONE != rv) {
      printf("Error in bcm_field_qualify_data %d\n", rv);
      return result;
   }

   rv = bcm_field_action_add(unit, ent_elk, action, act_val1[hit_bit-1], act_val2[hit_bit-1]);
   if (rv) {
      printf("bcm_field_action_add error, act_val1=0x%x, act_val20x%x\n", act_val1[hit_bit-1], act_val2[hit_bit-1]);
      return rv;
   }

   rv = bcm_field_entry_prio_set(unit, ent_elk, 10 + hit_bit);
   if (rv) {
      printf("bcm_field_entry_prio_set error\n");
      return rv;
   }

   rv = bcm_field_entry_install(unit, ent_elk);
   if (rv) {
      printf("bcm_field_entry_install error\n");
      return rv;
   }

   return rv;
}

int msb_qualifier_kbp_acl_set_configurations(int unit, int hit_bit) {
   int rv = 0;
   uint64 hit_value, hit_mask;
   int presel_id = 0;

   rv = msb_qualifier_kbp_acl_config_example(unit, hit_bit);
   if(rv) {
      printf("msb_qualifier_kbp_acl_set_configurations error hitbit=%d\n",hit_bit);
      return rv;
   }

   COMPILER_64_ZERO(hit_value);
   COMPILER_64_ZERO(hit_mask);

   switch (hit_bit)
   {
     case HIT_BIT_1_QUAL:
       COMPILER_64_SET(hit_value, 0x00000000, 0x0000abab);
       COMPILER_64_SET(hit_mask, 0x00000000, 0x0000ffff);
       rv = configure_pmf_action_by_hit_value_example(unit,HIT_BIT_1_QUAL,hit_value,hit_mask,bcmFieldAppTypeIp4Ucast,0,bcmFieldActionDrop,0,0,0,presel_id);
       if(rv) {
          printf("configure_pmf_action_by_hit_value_example bit 1 error\n");
          return rv;
       }
       break;
     case HIT_BIT_3_QUAL:
       COMPILER_64_SET(hit_value, 0x00000000, 0x12121212);
       COMPILER_64_SET(hit_mask, 0x00000000, 0xffffffff);
       presel_id++;
       rv = configure_pmf_action_by_hit_value_example(unit,HIT_BIT_3_QUAL,hit_value,hit_mask,bcmFieldAppTypeIp4Ucast,0,bcmFieldActionDrop,0,0,0,presel_id);
       if(rv) {
          printf("configure_pmf_action_by_hit_value_example bit 3 error\n");
          return rv;
       }
       break;
     case HIT_BIT_4_QUAL:
       COMPILER_64_SET(hit_value, 0x00000000, 0x56565656);
       COMPILER_64_SET(hit_mask, 0x00000000, 0xffffffff);
       presel_id++;
       rv = configure_pmf_action_by_hit_value_example(unit,HIT_BIT_4_QUAL,hit_value,hit_mask,bcmFieldAppTypeIp4Ucast,0,bcmFieldActionDrop,0,0,0,presel_id);
       if(rv) {
          printf("configure_pmf_action_by_hit_value_example bit 4 error\n");
          return rv;
       }
       break;
   }
   return rv;
}

/*
 * test for checkoing an ACLs with key which not byte-align. 
 * Testing key below and above 80 bits
 */

int acl_kbp_not_byte_aligned_key_config_example(int unit) {

	int rv=0;
	bcm_field_group_config_t grp_elk;
    int presel_id_elk = 0;
    int presel_flags = 0; 

    /* Create Preselector */ 
    if(soc_property_get(unit, spn_FIELD_PRESEL_MGMT_ADVANCED_MODE, FALSE)) { 
      presel_flags = BCM_FIELD_QUALIFY_PRESEL_ADVANCED_MODE_STAGE_EXTERNAL; 
      rv = bcm_field_presel_create_stage_id(unit, bcmFieldStageExternal, presel_id_elk); 
      if (BCM_E_NONE != rv) { 
          printf("Error in bcm_field_presel_create_stage_id\n"); 
          return rv; 
      } 
    } else { 
      rv = bcm_field_presel_create(unit, &presel_id_elk); 
      if (BCM_E_NONE != rv) { 
          printf("Error in bcm_field_presel_create\n"); 
          return rv; 
      }
    } 

    rv = bcm_field_qualify_Stage(unit, presel_id_elk | BCM_FIELD_QUALIFY_PRESEL | presel_flags, bcmFieldStageExternal); 
    if (BCM_E_NONE != rv) { 
      printf("Error in bcm_field_qualify_Stage\n"); 
      return rv; 
    } 

	rv = bcm_field_qualify_AppType(unit, presel_id_elk | BCM_FIELD_QUALIFY_PRESEL | presel_flags, bcmFieldAppTypeL2);
	if (rv) {
	  printf("bcm_field_qualify_AppType AppTypeL2 error\n");
	  return rv;
	}

	bcm_field_group_config_t_init(&grp_elk); 
	BCM_FIELD_PRESEL_ADD(grp_elk.preselset, presel_id_elk);
	grp_elk.mode =  bcmFieldGroupModeAuto;
	grp_elk.flags = BCM_FIELD_GROUP_CREATE_WITH_MODE | BCM_FIELD_GROUP_CREATE_WITH_ASET | BCM_FIELD_GROUP_CREATE_WITH_PRESELSET | BCM_FIELD_GROUP_CREATE_WITH_ID;

	BCM_FIELD_QSET_INIT(grp_elk.qset);
	BCM_FIELD_ASET_INIT(grp_elk.aset);
	BCM_FIELD_QSET_ADD(grp_elk.qset, bcmFieldQualifyStageExternal);
	grp_elk.group 	 = ACL_BASE_GROUP_INDEX;
	grp_elk.priority = ACL_BASE_GROUP_PRIORITY;
	
    /* 
     * Create an ACL with:
     * 32 bits src ip mac + 9 bits src pp port = 41 bits.
     */
    BCM_FIELD_QSET_ADD(grp_elk.qset, bcmFieldQualifySrcIp);
    BCM_FIELD_QSET_ADD(grp_elk.qset, bcmFieldQualifyInPort);
    BCM_FIELD_ASET_ADD(grp_elk.aset, bcmFieldActionExternalValue0Set);

    rv = bcm_field_group_config_create(unit, &grp_elk);
    if (rv) {
        printf("bcm_field_group_config_create short group error \n");
      return rv;
    }

    grp_elk.group++;
    grp_elk.priority++;
    BCM_FIELD_QSET_REMOVE(grp_elk.qset, bcmFieldQualifySrcIp);
    BCM_FIELD_ASET_REMOVE(grp_elk.aset, bcmFieldActionExternalValue0Set);
    /* 
     * Create an ACL with:
     * 48 bits src mac + 32 bits dst IP + 9 bits src pp port = 89 bits.
     */
    BCM_FIELD_QSET_ADD(grp_elk.qset, bcmFieldQualifySrcMac);
    BCM_FIELD_QSET_ADD(grp_elk.qset, bcmFieldQualifyDstIp);
    BCM_FIELD_ASET_ADD(grp_elk.aset, bcmFieldActionExternalValue1Set);
    
    rv = bcm_field_group_config_create(unit, &grp_elk);
    if (rv) {
        printf("bcm_field_group_config_create long group error \n");
      return rv;
    }

	/* Initialize the KBP */
	bshell(unit, "kbp deinit_appl");
	bshell(unit, "kbp init_appl");

	return rv;
}

int acl_kbp_not_byte_aligned_key_add_entry_example(int unit, int index, uint32 src_pp_port) {

	int rv;
	bcm_field_entry_t ent_elk;

	bcm_mac_t src_mac  = {0x00, 0x00, 0x07, 0x00, 0x01, 0x02};
    bcm_mac_t mac_mask  = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    
    bcm_ip_t src_ip = 0xc0800101;
    bcm_ip_t dst_ip = 0x7fffff03;
    bcm_ip_t ip_mask = 0xFFFFFFFF;

    uint32 src_pp_port_mask = 0xFFFFFFFF; 
          
	rv = bcm_field_entry_create(unit, index, &ent_elk);
    if (rv) {
      printf("bcm_field_entry_create error, index = %d\n",index);
      return rv;
    }

	switch (index) {
		case 0:
			rv = bcm_field_qualify_SrcIp(unit, ent_elk, src_ip, ip_mask);
			if (rv) {
			  printf("bcm_field_qualify_SrcIp error, index = %d\n",index);
			  return rv;
			}
			rv = bcm_field_qualify_InPort(unit, ent_elk, src_pp_port, src_pp_port_mask);
			if (rv) {
			  printf("bcm_field_qualify_InPort error, index = %d\n",index);
			  return rv;
			}
			break;
		case 1:
			rv = bcm_field_qualify_SrcMac(unit, ent_elk, src_mac, mac_mask);
			if (rv) {
			  printf("bcm_field_qualify_SrcMac error, index = %d\n",index);
			  return rv;
			}
			rv = bcm_field_qualify_DstIp(unit, ent_elk, dst_ip, ip_mask);
			if (rv) {
			  printf("bcm_field_qualify_DstIp error, index = %d\n",index);
			  return rv;
			}
			rv = bcm_field_qualify_InPort(unit, ent_elk, src_pp_port, src_pp_port_mask);
			if (rv) {
			  printf("bcm_field_qualify_InPort error, index = %d\n",index);
			  return rv;
			}
			break;
		default:
			printf("Unknown index=%d\n",index);
			return -1;
	}

	rv = bcm_field_entry_prio_set(unit, ent_elk, 10);
	if (rv) {
	  printf("bcm_field_entry_prio_set error\n");
	  return rv;
	}

	rv = bcm_field_entry_install(unit, ent_elk);
	if (rv) {
	  printf("bcm_field_entry_install error\n");
	  return rv;
	}

	return rv;
}

int acl_kbp_not_byte_aligned_key_set_configuration(int unit) {
	int rv;
    int presel_id = 1; /* ingress presel id for advanced mode: between 1-47 as 0 is taken */

	rv = acl_kbp_not_byte_aligned_key_config_example(unit);
	if (rv) {
		printf("acl_kbp_not_byte_aligned_key_config_example error\n");
		return  rv;
	}

	rv = configure_pmf_action_by_hit_bit_example(unit,HIT_BIT_0_QUAL,bcmFieldAppTypeL2, bcmFieldActionDrop, 0, 0, presel_id);
	if (rv) {
		printf("configure_pmf_action_by_hit_bit_example bit 0 error\n");
		return  rv;
	}

    presel_id++; /* Use next free preselector ID for advanced mode */

    rv = configure_pmf_action_by_hit_bit_example(unit, HIT_BIT_1_QUAL, bcmFieldAppTypeL2, bcmFieldActionDrop, 0, 0, presel_id);
    if (rv) {
        printf("configure_pmf_action_by_hit_bit_example bit 1 error\n");
        return  rv;
    }
	return rv;
}

int acl_iter_end = 128;
bcm_field_entry_t entrySrcIp[128];
bcm_field_entry_t entryDstIp[128];
int acl_sem_config_kbp_example(int unit) {

    int rv = 0;
    bcm_field_group_config_t grp;
    int presel_id = 1; /* ID from 0 to 11 for Advanced mode */ 
    int presel_flags = 0; 

    /* Create Preselector */ 
    if(soc_property_get(unit, spn_FIELD_PRESEL_MGMT_ADVANCED_MODE, FALSE)) { 
      presel_flags = BCM_FIELD_QUALIFY_PRESEL_ADVANCED_MODE_STAGE_EXTERNAL; 
      rv = bcm_field_presel_create_stage_id(unit, bcmFieldStageExternal, presel_id); 
      if (BCM_E_NONE != rv) { 
          printf("Error in bcm_field_presel_create_stage_id\n"); 
          return rv; 
      } 
    } else { 
      rv = bcm_field_presel_create(unit, &presel_id); 
      if (BCM_E_NONE != rv) { 
          printf("Error in bcm_field_presel_create\n"); 
          return rv; 
      }
    } 

    rv = bcm_field_qualify_Stage(unit, presel_id | BCM_FIELD_QUALIFY_PRESEL | presel_flags, bcmFieldStageExternal); 
    if (BCM_E_NONE != rv) { 
      printf("Error in bcm_field_qualify_Stage\n"); 
      return rv; 
    } 

    rv = bcm_field_qualify_AppType(unit, presel_id | BCM_FIELD_QUALIFY_PRESEL | presel_flags, bcmFieldAppTypeIp4Ucast);
    if (rv) {
        printf("bcm_field_qualify_AppType Any error\n");
        return rv;
    }

    bcm_field_group_config_t_init(&grp); 
    BCM_FIELD_PRESEL_ADD(grp.preselset, presel_id);
    grp.mode = bcmFieldGroupModeAuto;
    grp.flags =
            BCM_FIELD_GROUP_CREATE_WITH_MODE |
            BCM_FIELD_GROUP_CREATE_WITH_ASET |
            BCM_FIELD_GROUP_CREATE_WITH_PRESELSET |
            BCM_FIELD_GROUP_CREATE_WITH_ID;

    BCM_FIELD_QSET_INIT(grp.qset);
    BCM_FIELD_ASET_INIT(grp.aset);
    BCM_FIELD_QSET_ADD(grp.qset, bcmFieldQualifyStageExternal);

    /*SrcIp*/
    BCM_FIELD_QSET_ADD(grp.qset, bcmFieldQualifySrcIp);
    BCM_FIELD_ASET_ADD(grp.aset, bcmFieldActionExternalValue1Set);
    grp.group = ACL_BASE_GROUP_INDEX + HIT_BIT_1_QUAL;
    grp.priority = ACL_BASE_GROUP_PRIORITY;

    rv = bcm_field_group_config_create(unit, &grp);
    if (rv) {
        printf("bcm_field_group_config_create 1 error\n");
        return rv;
    }

    /*DstIp*/
    BCM_FIELD_ASET_REMOVE(grp.aset, bcmFieldActionExternalValue1Set);
    BCM_FIELD_ASET_ADD(grp.aset, bcmFieldActionExternalValue3Set);
    BCM_FIELD_QSET_REMOVE(grp.qset, bcmFieldQualifySrcIp);
    BCM_FIELD_QSET_ADD(grp.qset, bcmFieldQualifyDstIp);
    grp.group = ACL_BASE_GROUP_INDEX + HIT_BIT_3_QUAL;
    grp.priority++;

    rv = bcm_field_group_config_create(unit, &grp);
    if (rv) {
        printf("bcm_field_group_config_create 3 error\n");
        return rv;
    }

    /* Initialize the KBP */
    bshell(unit, "kbp deinit_appl");
    bshell(unit, "kbp init_appl");

    return rv;
}

int acl_sem_config_pmf_example(int unit) {

    int rv = 0;
    bcm_field_group_config_t grp;
    bcm_field_entry_t ent;
    int presel_id = 1; /* ID from 0 to 47 for Advanced mode */ 
    int presel_flags = 0; 

    /* Create Preselector */ 
    if(soc_property_get(unit, spn_FIELD_PRESEL_MGMT_ADVANCED_MODE, FALSE)) { 
      presel_flags = BCM_FIELD_QUALIFY_PRESEL_ADVANCED_MODE_STAGE_INGRESS; 
      rv = bcm_field_presel_create_stage_id(unit, bcmFieldStageIngress, presel_id); 
      if (BCM_E_NONE != rv) { 
          printf("Error in bcm_field_presel_create_stage_id\n"); 
          return rv; 
      } 
    } else { 
      rv = bcm_field_presel_create(unit, &presel_id); 
      if (BCM_E_NONE != rv) { 
          printf("Error in bcm_field_presel_create\n"); 
          return rv; 
      }
    } 

    rv = bcm_field_qualify_Stage(unit, presel_id | BCM_FIELD_QUALIFY_PRESEL | presel_flags, bcmFieldStageIngress); 
    if (BCM_E_NONE != rv) { 
      printf("Error in bcm_field_qualify_Stage\n"); 
      return rv; 
    } 

    rv = bcm_field_qualify_AppType(unit, presel_id | BCM_FIELD_QUALIFY_PRESEL | presel_flags, bcmFieldAppTypeIp4Ucast);
    if(rv) {
        printf("bcm_field_qualify_AppType error\n");
        return rv;
    }

    rv = bcm_field_qualify_ExternalHit1(unit, presel_id | BCM_FIELD_QUALIFY_PRESEL | presel_flags, 0x1, 0x1);
    if(rv) {
        printf("bcm_field_qualify_ExternalHit1 error\n");
        return rv;
    }

    bcm_field_group_config_t_init(&grp);
    BCM_FIELD_PRESEL_ADD(grp.preselset, presel_id);
    grp.group    = PMF_BASE_GROUP_INDEX 	+ HIT_BIT_1_QUAL;
    grp.priority = PMF_BASE_GROUP_PRIORITY  + HIT_BIT_1_QUAL;
    grp.mode 	 = bcmFieldGroupModeAuto;
    grp.flags 	 =
                BCM_FIELD_GROUP_CREATE_WITH_MODE |
                BCM_FIELD_GROUP_CREATE_WITH_ASET |
                BCM_FIELD_GROUP_CREATE_WITH_PRESELSET |
                BCM_FIELD_GROUP_CREATE_WITH_ID;

    BCM_FIELD_ASET_INIT(grp.aset);
    BCM_FIELD_ASET_ADD(grp.aset, bcmFieldActionDrop);

    BCM_FIELD_QSET_INIT(grp.qset);
    BCM_FIELD_QSET_ADD(grp.qset, bcmFieldQualifyStageIngress);
    BCM_FIELD_QSET_ADD(grp.qset, bcmFieldQualifyExternalValue1);

    rv = bcm_field_group_config_create(unit, &grp);
    if(rv) {
        printf("bcm_field_group_config_create error\n");
        return rv;
    }

    rv = bcm_field_entry_create(unit, grp.group, &ent);
    if(rv) {
        printf("Error in bcm_field_entry_create\n");
        return rv;
    }

    rv = bcm_field_action_add(unit, ent, bcmFieldActionDrop, 0, 0);
    if(rv) {
        printf("Error in bcm_field_action_add\n");
        return rv;
    }

    rv = bcm_field_entry_install(unit, ent);
    if(rv) {
        printf("bcm_field_entry_install error\n");
        return rv;
    }

    /*second*/

    presel_id = 2;

    /* Create Preselector */ 
    if(soc_property_get(unit, spn_FIELD_PRESEL_MGMT_ADVANCED_MODE, FALSE)) { 
      presel_flags = BCM_FIELD_QUALIFY_PRESEL_ADVANCED_MODE_STAGE_INGRESS; 
      rv = bcm_field_presel_create_stage_id(unit, bcmFieldStageIngress, presel_id); 
      if (BCM_E_NONE != rv) { 
          printf("Error in bcm_field_presel_create_stage_id\n"); 
          return rv; 
      } 
    } else { 
      rv = bcm_field_presel_create(unit, &presel_id); 
      if (BCM_E_NONE != rv) { 
          printf("Error in bcm_field_presel_create\n"); 
          return rv; 
      }
    } 

    rv = bcm_field_qualify_Stage(unit, presel_id | BCM_FIELD_QUALIFY_PRESEL | presel_flags, bcmFieldStageIngress); 
    if (BCM_E_NONE != rv) { 
      printf("Error in bcm_field_qualify_Stage\n"); 
      return rv; 
    }

    rv = bcm_field_qualify_AppType(unit, presel_id | BCM_FIELD_QUALIFY_PRESEL | presel_flags, bcmFieldAppTypeIp4Ucast);
    if(rv) {
        printf("bcm_field_qualify_AppType error\n");
        return rv;
    }

    rv = bcm_field_qualify_ExternalHit3(unit, presel_id | BCM_FIELD_QUALIFY_PRESEL | presel_flags, 0x1, 0x1);
    if(rv) {
        printf("bcm_field_qualify_ExternalHit3 error\n");
        return rv;
    }

    grp.group    = PMF_BASE_GROUP_INDEX 	+ HIT_BIT_3_QUAL;
    grp.priority = PMF_BASE_GROUP_PRIORITY  + HIT_BIT_3_QUAL;

    BCM_FIELD_QSET_REMOVE(grp.qset, bcmFieldQualifyExternalValue1);
    BCM_FIELD_QSET_ADD(grp.qset, bcmFieldQualifyExternalValue3);

    rv = bcm_field_group_config_create(unit, &grp);
    if(rv) {
        printf("bcm_field_group_config_create error\n");
        return rv;
    }

    rv = bcm_field_entry_create(unit, grp.group, &ent);
    if(rv) {
        printf("Error in bcm_field_entry_create\n");
        return rv;
    }

    rv = bcm_field_action_add(unit, ent, bcmFieldActionDrop, 0, 0);
    if(rv) {
        printf("Error in bcm_field_action_add\n");
        return rv;
    }

    rv = bcm_field_entry_install(unit, ent);
    if(rv) {
        printf("bcm_field_entry_install error\n");
        return rv;
    }

    return rv;
}

int acl_sem_kbp_acl_entry_add_blank_example(int unit, int index, int i) {

    int rv = 0;
    bcm_field_entry_t ent;

    if(index != HIT_BIT_1_QUAL && index != HIT_BIT_3_QUAL) {
        printf("Unknown index value = %d\n", index);
        return -1;
    }

    bcm_field_entry_create(unit, ACL_BASE_GROUP_INDEX + index, &ent);

    if(index == HIT_BIT_1_QUAL)
        entrySrcIp[i] = ent;
    else if(index == HIT_BIT_3_QUAL)
        entryDstIp[i] = ent;

    switch(index) {
        case HIT_BIT_1_QUAL:/* 1st: SrcIp */
            rv = bcm_field_action_add(unit, ent, bcmFieldActionExternalValue1Set, 0, 0);
            if (rv) {
                printf("bcm_field_action_add error value1set, index %d\n", i);
                return rv;
            }
            break;
        case HIT_BIT_3_QUAL:/* 2nd: DstIp */
            rv = bcm_field_action_add(unit, ent, bcmFieldActionExternalValue3Set, 0, 0);
            if (rv) {
                printf("bcm_field_action_add error value3set, index %d\n", i);
                return rv;
            }
            break;
        default:
            printf("Unknown index value = %d\n", index);
            return -1;
    }

    rv = bcm_field_entry_prio_set(unit, ent, 10 + index);
    if(rv) {
        printf("bcm_field_entry_prio_set error\n");
        return rv;
    }

    rv = bcm_field_entry_install(unit, ent);
    if(rv) {
        printf("bcm_field_entry_install error\n");
        return rv;
    }

    return rv;
}


int acl_sem_kbp_acl_entry_update_example(int unit, int index, bcm_ip_t ip_addr, bcm_ip_t ip_mask, int val1, int val2, int i) {

    int rv = 0;
    bcm_field_entry_t ent;

    if(index != HIT_BIT_1_QUAL && index != HIT_BIT_3_QUAL) {
        printf("Unknown index value = %d\n", index);
        return -1;
    }

    if(index == HIT_BIT_1_QUAL)
        ent = entrySrcIp[i];
    else if(index == HIT_BIT_3_QUAL)
        ent = entryDstIp[i];

    switch(index) {
        case HIT_BIT_1_QUAL:/* 1st: SrcIp */
            rv = bcm_field_qualify_SrcIp(unit, ent, ip_addr, ip_mask);
            if(rv) {
                printf("bcm_field_qualify_SrcIp error, index = %d\n", index);
                return rv;
            }
            rv = bcm_field_action_add(unit, ent, bcmFieldActionExternalValue1Set, val1, val2);
            if (rv) {
                printf("bcm_field_action_add error value1set, val = %d, val2 = %d\n", val1, val2);
                return rv;
            }
            break;
        case HIT_BIT_3_QUAL:/* 2nd: DstIp */
            rv = bcm_field_qualify_DstIp(unit, ent, ip_addr, ip_mask);
            if(rv) {
                printf("bcm_field_qualify_DstIp error, index = %d\n", index);
                return rv;
            }
            rv = bcm_field_action_add(unit, ent, bcmFieldActionExternalValue3Set, val1, val2);
            if (rv) {
                printf("bcm_field_action_add error value3set, val = %d, val2 = %d\n", val1, val2);
                return rv;
            }
            break;
        default:
            printf("Unknown index value = %d\n", index);
            return -1;
    }

    rv = bcm_field_entry_prio_set(unit, ent, 10 + index);
    if(rv) {
        printf("bcm_field_entry_prio_set error\n");
        return rv;
    }

    rv = bcm_field_entry_reinstall(unit, ent);
    if(rv) {
        printf("bcm_field_entry_reinstall error\n");
        return rv;
    }

    return rv;
}

int acl_sem_example(int unit) {

    int i, rv = 0;
    int SrcIp = 0x0ca80100;
    int DstIp = 0x0ca80200;
    int IpMask = 0xffffffff;
    int SrcIpVal = 0x100;
    int DstIpVal = 0x200;
    uint32 param0, param1;

    printf("        Configuring groups\n");
    rv = acl_sem_config_kbp_example(unit);
    if(rv) {
        printf("acl_sem_config_kbp_example error\n");
        return rv;
    }

    rv = acl_sem_config_pmf_example(unit);
    if(rv) {
        printf("acl_sem_config_pmf_example error\n");
        return rv;
    }

    printf("        Adding blank entries\n");
    for(i = 0; i < acl_iter_end; i++) {
        rv = acl_sem_kbp_acl_entry_add_blank_example(unit, HIT_BIT_1_QUAL, i);
        if(rv) {
            printf("acl_sem_kbp_acl_entry_add_blank_example SrcIp index %d error\n", i);
            return rv;
        }
        rv = acl_sem_kbp_acl_entry_add_blank_example(unit, HIT_BIT_3_QUAL, i);
        if(rv) {
            printf("acl_sem_kbp_acl_entry_add_blank_example DstIp index %d error\n", i);
            return rv;
        }
        SrcIp++; DstIp++; SrcIpVal++; DstIpVal++;
    }

    printf("        Validating blank entries\n");
    for(i = 0; i < acl_iter_end; i++) {
        rv = bcm_field_action_get(unit, entrySrcIp[i], bcmFieldActionExternalValue1Set, &param0, &param1);
        if(rv) {
            printf("bcm_field_action_get Value1Set error entrySrcIp[%d]: %x\n", i, SrcIp);
            return rv;
        }
        if(param0 != 0) {
            printf("Error validating entrySrcIp[%d]: %x, param0: %x, param1: %x\n", i, entrySrcIp[i], param0, param1);
            rv = -1;
        }
        rv = bcm_field_action_get(unit, entryDstIp[i], bcmFieldActionExternalValue3Set, &param0, &param1);
        if(rv) {
            printf("bcm_field_action_get Value1Set error entrySrcIp[%d]: %x\n", i, DstIp);
            return rv;
        }
        if(param0 != 0) {
            printf("Error validating entryDstIp[i]: %x, param0: %x, param1: %x\n", i, entryDstIp[i], param0, param1);
            rv = -1;
        }
    }

    printf("        Updating entries with qualifiers and actions\n");
    SrcIp = 0x0ca80100;
    DstIp = 0x0ca80200;
    SrcIpVal = 0x100;
    DstIpVal = 0x200;
    for(i = 0; i < acl_iter_end; i++) {
        rv = acl_sem_kbp_acl_entry_update_example(unit, HIT_BIT_1_QUAL, SrcIp, IpMask, SrcIpVal, SrcIpVal, i);
        if(rv) {
            printf("acl_sem_kbp_acl_entry_update_example SrcIp index %d error\n", i);
            return rv;
        }
        rv = acl_sem_kbp_acl_entry_update_example(unit, HIT_BIT_3_QUAL, DstIp, IpMask, DstIpVal, DstIpVal, i);
        if(rv) {
            printf("acl_sem_kbp_acl_entry_update_example DstIp index %d error\n", i);
            return rv;
        }
        SrcIp++; DstIp++; SrcIpVal++; DstIpVal++;
    }

    printf("        Validating entries\n");
    SrcIp = 0x0ca80100;
    SrcIpVal = 0x100;
    for(i = 0; i < acl_iter_end; i++) {
        rv = bcm_field_action_get(unit, entrySrcIp[i], bcmFieldActionExternalValue1Set, &param0, &param1);
        if(rv) {
            printf("bcm_field_action_get Value1Set error SrcIp: %x\n", SrcIp);
            return rv;
        }
        if(param0 != SrcIpVal) {
            printf("Error validating entrySrcIp: %x, SrcIp: %08x, param0: %x, param1: %x, SrcIpVal: %x\n", entrySrcIp[i], SrcIp, param0, param1, SrcIpVal);
            rv = -1;
        }
        SrcIp++; SrcIpVal++;
    }

    DstIp = 0x0ca80200;
    DstIpVal = 0x200;
    for(i = 0; i < acl_iter_end; i++) {
        rv = bcm_field_action_get(unit, entryDstIp[i], bcmFieldActionExternalValue3Set, &param0, &param1);
        if(rv) {
            printf("bcm_field_action_get Value3Set error DstIp: %x\n", DstIp);
            return rv;
        }
        if(param0 != DstIpVal) {
            printf("Error validating entryDstIp: %x, DstIp: %08x, param0: %x, param1: %x, DstIpVal: %x\n", entryDstIp[i], DstIp, param0, param1, DstIpVal);
            rv = -1;
        }
        DstIp++; DstIpVal++;
    }

    if(rv == 0) {
        printf("        Entries validated\n");
    } else {
        printf("        Entries validation failed\n");
        return rv;
    }

    printf("        Destroying entries\n");
    for(i = 0; i < acl_iter_end; i++) {
        rv = bcm_field_entry_destroy(unit, entrySrcIp[i]);
        if(rv) {
            printf("bcm_field_entry_destroy entrySrcIp[%d] error: %x\n", i, entrySrctIp[i]);
            return rv;
        }
        rv = bcm_field_entry_destroy(unit, entryDstIp[i]);
        if(rv) {
            printf("bcm_field_entry_destroy entryDstIp[%d] error: %x\n", i, entryDstIp[i]);
            return rv;
        }
    }

    printf("        Verifying correct destruction\n");
    SrcIp = 0x0ca80100;
    DstIp = 0x0ca80200;
    SrcIpVal = 0x100;
    DstIpVal = 0x200;
    param0 = 0x0;
    param1 = 0x0;
    for(i = 0; i < acl_iter_end; i++) {
        rv = bcm_field_action_get(unit, entrySrcIp[i], bcmFieldActionExternalValue1Set, &param0, &param1);
        if(rv == 0) {
            printf("bcm_field_action_get entrySrcIp[%d]: %x error\n", i, entrySrcIp[i]);
            return -1;
        }
        rv = bcm_field_action_get(unit, entryDstIp[i], bcmFieldActionExternalValue3Set, &param0, &param1);
        if(rv == 0) {
            printf("bcm_field_action_get entryDstIp[%d]: %x error\n", i, entryDstIp[i]);
            return -1;
        }
        SrcIp++; DstIp++; SrcIpVal++; DstIpVal++;
    }

    return 0;
}


int acl_ipv4dc_2nd_lookup_hit_config_kbp_example(int unit) {

    int rv = 0;
    bcm_field_group_config_t grp;
    int presel_id = 0; /* ID from 0 to 11 for Advanced mode */ 
    int presel_flags = 0; 

    /* Create Preselector */ 
    if(soc_property_get(unit, spn_FIELD_PRESEL_MGMT_ADVANCED_MODE, FALSE)) { 
      presel_flags = BCM_FIELD_QUALIFY_PRESEL_ADVANCED_MODE_STAGE_EXTERNAL; 
      rv = bcm_field_presel_create_stage_id(unit, bcmFieldStageExternal, presel_id); 
      if (BCM_E_NONE != rv) { 
          printf("Error in bcm_field_presel_create_stage_id\n"); 
          return rv; 
      } 
    } else { 
      rv = bcm_field_presel_create(unit, &presel_id); 
      if (BCM_E_NONE != rv) { 
          printf("Error in bcm_field_presel_create\n"); 
          return rv; 
      }
    } 

    rv = bcm_field_qualify_Stage(unit, presel_id | BCM_FIELD_QUALIFY_PRESEL | presel_flags, bcmFieldStageExternal); 
    if (BCM_E_NONE != rv) { 
      printf("Error in bcm_field_qualify_Stage\n"); 
      return rv; 
    } 

    rv = bcm_field_qualify_AppType(unit, presel_id | BCM_FIELD_QUALIFY_PRESEL | presel_flags, bcmFieldAppTypeIp4DoubleCapacity);
    if (rv) {
        printf("bcm_field_qualify_AppType Any error\n");
        return rv;
    }

    bcm_field_group_config_t_init(&grp); 
    BCM_FIELD_PRESEL_ADD(grp.preselset, presel_id);
    grp.mode = bcmFieldGroupModeAuto;
    grp.flags =
            BCM_FIELD_GROUP_CREATE_WITH_MODE |
            BCM_FIELD_GROUP_CREATE_WITH_ASET |
            BCM_FIELD_GROUP_CREATE_WITH_PRESELSET |
            BCM_FIELD_GROUP_CREATE_WITH_ID;

    BCM_FIELD_QSET_INIT(grp.qset);
    BCM_FIELD_ASET_INIT(grp.aset);
    BCM_FIELD_QSET_ADD(grp.qset, bcmFieldQualifyStageExternal);

    /*DstIp*/
    BCM_FIELD_QSET_ADD(grp.qset, bcmFieldQualifyDstIp);
    BCM_FIELD_ASET_ADD(grp.aset, bcmFieldActionExternalValue1Set);
    grp.group = ACL_BASE_GROUP_INDEX + HIT_BIT_1_QUAL;
    grp.priority = ACL_BASE_GROUP_PRIORITY;

    rv = bcm_field_group_config_create(unit, &grp);
    if (rv) {
        printf("bcm_field_group_config_create 1 error\n");
        return rv;
    }

    /* Initialize the KBP */
    bshell(unit, "kbp deinit_appl");
    bshell(unit, "kbp init_appl");

    return rv;
}


int acl_ipv4dc_2nd_lookup_hit_config_pmf_example(int unit) {

    int rv = 0;
    bcm_field_group_config_t grp;
    bcm_field_entry_t ent;
    uint64 hit_value, hit_mask;
    int presel_id = 1; /* ID from 0 to 47 for Advanced mode */ 
    int presel_flags = 0; 

    /* Create Preselector */ 
    if(soc_property_get(unit, spn_FIELD_PRESEL_MGMT_ADVANCED_MODE, FALSE)) { 
      presel_flags = BCM_FIELD_QUALIFY_PRESEL_ADVANCED_MODE_STAGE_INGRESS; 
      rv = bcm_field_presel_create_stage_id(unit, bcmFieldStageIngress, presel_id); 
      if (BCM_E_NONE != rv) { 
          printf("Error in bcm_field_presel_create_stage_id\n"); 
          return rv; 
      } 
    } else { 
      rv = bcm_field_presel_create(unit, &presel_id); 
      if (BCM_E_NONE != rv) { 
          printf("Error in bcm_field_presel_create\n"); 
          return rv; 
      }
    } 

    rv = bcm_field_qualify_Stage(unit, presel_id | BCM_FIELD_QUALIFY_PRESEL | presel_flags, bcmFieldStageIngress); 
    if (BCM_E_NONE != rv) { 
      printf("Error in bcm_field_qualify_Stage\n"); 
      return rv; 
    } 

    rv = bcm_field_qualify_AppType(unit, presel_id | BCM_FIELD_QUALIFY_PRESEL | presel_flags, bcmFieldAppTypeIp4DoubleCapacity);
    if(rv) {
        printf("bcm_field_qualify_AppType error\n");
        return rv;
    }

    bcm_field_group_config_t_init(&grp);
    BCM_FIELD_PRESEL_ADD(grp.preselset, presel_id);
    grp.group    = PMF_BASE_GROUP_INDEX 	+ HIT_BIT_1_QUAL;
    grp.priority = PMF_BASE_GROUP_PRIORITY  + HIT_BIT_1_QUAL;
    grp.mode 	 = bcmFieldGroupModeAuto;
    grp.flags 	 =
                BCM_FIELD_GROUP_CREATE_WITH_MODE |
                BCM_FIELD_GROUP_CREATE_WITH_ASET |
                BCM_FIELD_GROUP_CREATE_WITH_PRESELSET |
                BCM_FIELD_GROUP_CREATE_WITH_ID;

    BCM_FIELD_ASET_INIT(grp.aset);
    BCM_FIELD_ASET_ADD(grp.aset, bcmFieldActionDrop);

    BCM_FIELD_QSET_INIT(grp.qset);
    BCM_FIELD_QSET_ADD(grp.qset, bcmFieldQualifyStageIngress);
    BCM_FIELD_QSET_ADD(grp.qset, bcmFieldQualifyExternalHit0);
    BCM_FIELD_QSET_ADD(grp.qset, bcmFieldQualifyExternalHit1);
    BCM_FIELD_QSET_ADD(grp.qset, bcmFieldQualifyExternalValue1);

    rv = bcm_field_group_config_create(unit, &grp);
    if(rv) {
        printf("bcm_field_group_config_create error\n");
        return rv;
    }

    rv = bcm_field_entry_create(unit, grp.group, &ent);
    if(rv) {
        printf("Error in bcm_field_entry_create\n");
        return rv;
    }

	rv = bcm_field_qualify_ExternalHit0(unit, ent, 1, 1);
    if(rv) {
      printf("bcm_field_qualify_ExternalHit0 error\n");
      return rv;
    }

	rv = bcm_field_qualify_ExternalHit1(unit, ent, 1, 1);
    if(rv) {
      printf("bcm_field_qualify_ExternalHit1 error\n");
      return rv;
    }

    COMPILER_64_ZERO(hit_value);
    COMPILER_64_ZERO(hit_mask);
    COMPILER_64_SET(hit_value, 0x00000000, 0x00000001);
    COMPILER_64_SET(hit_mask, 0x00000000, 0x00000001);
    rv = bcm_field_qualify_ExternalValue1(unit, ent, hit_value, hit_mask);
    if(rv) {
      printf("bcm_field_qualify_ExternalValue0 error\n");
      return rv;
    }

    rv = bcm_field_action_add(unit, ent, bcmFieldActionDrop, 0, 0);
    if(rv) {
        printf("Error in bcm_field_action_add\n");
        return rv;
    }

    rv = bcm_field_entry_install(unit, ent);
    if(rv) {
        printf("bcm_field_entry_install error\n");
        return rv;
    }

    /* second hit */

    presel_id = 2;

    /* Create Preselector */ 
    if(soc_property_get(unit, spn_FIELD_PRESEL_MGMT_ADVANCED_MODE, FALSE)) { 
      presel_flags = BCM_FIELD_QUALIFY_PRESEL_ADVANCED_MODE_STAGE_INGRESS; 
      rv = bcm_field_presel_create_stage_id(unit, bcmFieldStageIngress, presel_id); 
      if (BCM_E_NONE != rv) { 
          printf("Error in bcm_field_presel_create_stage_id\n"); 
          return rv; 
      } 
    } else { 
      rv = bcm_field_presel_create(unit, &presel_id); 
      if (BCM_E_NONE != rv) { 
          printf("Error in bcm_field_presel_create\n"); 
          return rv; 
      }
    } 

    rv = bcm_field_qualify_Stage(unit, presel_id | BCM_FIELD_QUALIFY_PRESEL | presel_flags, bcmFieldStageIngress); 
    if (BCM_E_NONE != rv) { 
      printf("Error in bcm_field_qualify_Stage\n"); 
      return rv; 
    }

    rv = bcm_field_qualify_AppType(unit, presel_id | BCM_FIELD_QUALIFY_PRESEL | presel_flags, bcmFieldAppTypeIp4DoubleCapacity);
    if(rv) {
        printf("bcm_field_qualify_AppType error\n");
        return rv;
    }

    grp.group    = PMF_BASE_GROUP_INDEX 	+ HIT_BIT_1_QUAL + 6;
    grp.priority = PMF_BASE_GROUP_PRIORITY  + HIT_BIT_1_QUAL + 6;

    BCM_FIELD_QSET_REMOVE(grp.qset, bcmFieldQualifyExternalHit0);
    BCM_FIELD_QSET_ADD(grp.qset, bcmFieldQualifyExternalHit2);

    rv = bcm_field_group_config_create(unit, &grp);
    if(rv) {
        printf("bcm_field_group_config_create error\n");
        return rv;
    }

    rv = bcm_field_entry_create(unit, grp.group, &ent);
    if(rv) {
        printf("Error in bcm_field_entry_create\n");
        return rv;
    }

	rv = bcm_field_qualify_ExternalHit2(unit, ent, 1, 1);
    if(rv) {
      printf("bcm_field_qualify_ExternalHit2 error\n");
      return rv;
    }

	rv = bcm_field_qualify_ExternalHit1(unit, ent, 1, 1);
    if(rv) {
      printf("bcm_field_qualify_ExternalHit1 error\n");
      return rv;
    }

    COMPILER_64_ZERO(hit_value);
    COMPILER_64_ZERO(hit_mask);
    COMPILER_64_SET(hit_value, 0x00000000, 0x00000002);
    COMPILER_64_SET(hit_mask, 0x00000000, 0x00000002);
    rv = bcm_field_qualify_ExternalValue1(unit, ent, hit_value, hit_mask);
    if(rv) {
      printf("bcm_field_qualify_ExternalValue0 error\n");
      return rv;
    }

    rv = bcm_field_action_add(unit, ent, bcmFieldActionDrop, 0, 0);
    if(rv) {
        printf("Error in bcm_field_action_add\n");
        return rv;
    }

    rv = bcm_field_entry_install(unit, ent);
    if(rv) {
        printf("bcm_field_entry_install error\n");
        return rv;
    }

    return rv;
}


int acl_ipv4dc_2nd_lookup_hit_kbp_acl_entry_add_example(int unit, int route) {

    int rv = 0;
    int ip_addr = route ? 0x0b00ff01 : 0x0a000001;
    int ip_mask = route ? 0xffff0000 : 0xffffffff;
    int val1 = route ? 2 : 1;
    int val2 = route ? 2 : 1;
    bcm_field_entry_t ent;

    bcm_field_entry_create(unit, ACL_BASE_GROUP_INDEX + 1, &ent);

    rv = bcm_field_qualify_DstIp(unit, ent, ip_addr, ip_mask);
    if(rv) {
        printf("bcm_field_qualify_SrcIp error, ip: %x, mask: %x\n", ip_addr, ip_mask);
        return rv;
    }
    rv = bcm_field_action_add(unit, ent, bcmFieldActionExternalValue1Set, val1, val2);
    if (rv) {
        printf("bcm_field_action_add error value1set, val = %d, val2 = %d\n", val1, val2);
        return rv;
    }

    rv = bcm_field_entry_prio_set(unit, ent, 10 + route * 6);
    if(rv) {
        printf("bcm_field_entry_prio_set error\n");
        return rv;
    }

    rv = bcm_field_entry_install(unit, ent);
    if(rv) {
        printf("bcm_field_entry_install error\n");
        return rv;
    }

    return rv;
}

int acl_ipv4dc_2nd_lookup_hit_config_example(int unit) {

    int rv = 0;

    rv = acl_ipv4dc_2nd_lookup_hit_config_kbp_example(unit);
    if(rv) {
        printf("acl_ipv4dc_2nd_lookup_hitconfig_kbp_example error\n");
        return rv;
    }

    rv = acl_ipv4dc_2nd_lookup_hit_config_pmf_example(unit);
    if(rv) {
        printf("acl_ipv4dc_2nd_lookup_hit_config_pmf_example error\n");
        return rv;
    }

    return rv;
}

void ipv6_print(bcm_ip6_t *ipv6) {
    int i;
    printf("%02X%02X", ipv6[0], ipv6[1]);
    for(i = 2; i < 16; i += 2) {
        printf(":%02X%02X", ipv6[i], ipv6[i+1]);
    }
}

/*
    AT_Cint_external_acl_ipv6_dip_sip_sharing_Sem
*/
int acl_ipv6_dip_sip_sharing_sem_entries = 1024;
bcm_field_entry_t acl_ipv6_dip_sip_sharing_sem_entry[1024];

int ipv6_dip_sip_sharing_config_kbp_example(int unit, bcm_field_AppType_t app_type) {

    int rv = 0;
    bcm_field_group_config_t grp;
    int presel_id = 0; /* ID from 0 to 11 for Advanced mode */ 
    int presel_flags = 0; 

    if (app_type != bcmFieldAppTypeAny &&
        app_type != bcmFieldAppTypeIp6Ucast &&
        app_type != bcmFieldAppTypeIp6UcastRpf &&
        app_type != bcmFieldAppTypeIp6Mcast &&
        app_type != bcmFieldAppTypeIp6UcastPublic &&
        app_type != bcmFieldAppTypeIp6UcastRpfPublic)
    {
        printf("Unsupported AppType for IPv6 DIP SIP sharing\n");
        return -1;
    }

    /* Create Preselector */ 
    if(soc_property_get(unit, spn_FIELD_PRESEL_MGMT_ADVANCED_MODE, FALSE)) { 
      presel_flags = BCM_FIELD_QUALIFY_PRESEL_ADVANCED_MODE_STAGE_EXTERNAL; 
      rv = bcm_field_presel_create_stage_id(unit, bcmFieldStageExternal, presel_id); 
      if (BCM_E_NONE != rv) { 
          printf("Error in bcm_field_presel_create_stage_id\n"); 
          return rv; 
      } 
    } else { 
      rv = bcm_field_presel_create(unit, &presel_id); 
      if (BCM_E_NONE != rv) { 
          printf("Error in bcm_field_presel_create\n"); 
          return rv; 
      }
    } 

    rv = bcm_field_qualify_Stage(unit, presel_id | BCM_FIELD_QUALIFY_PRESEL | presel_flags, bcmFieldStageExternal); 
    if (BCM_E_NONE != rv) { 
      printf("Error in bcm_field_qualify_Stage\n"); 
      return rv; 
    }

    if(app_type == bcmFieldAppTypeAny) {
        rv = bcm_field_qualify_AppType(unit, presel_id | BCM_FIELD_QUALIFY_PRESEL | presel_flags, bcmFieldAppTypeIp6Ucast);
        if(rv) {
            printf("bcm_field_qualify_AppType Ip6Ucast error\n");
            return rv;
        }
        rv = bcm_field_qualify_AppType(unit, presel_id | BCM_FIELD_QUALIFY_PRESEL | presel_flags, bcmFieldAppTypeIp6UcastRpf);
        if(rv) {
            printf("bcm_field_qualify_AppType Ip6UcastRpf error\n");
            return rv;
        }
        rv = bcm_field_qualify_AppType(unit, presel_id | BCM_FIELD_QUALIFY_PRESEL | presel_flags, bcmFieldAppTypeIp6Mcast);
        if(rv) {
            printf("bcm_field_qualify_AppType Ip6Mcast error\n");
            return rv;
        }
        rv = bcm_field_qualify_AppType(unit, presel_id | BCM_FIELD_QUALIFY_PRESEL | presel_flags, bcmFieldAppTypeIp6UcastPublic);
        if(rv) {
            printf("bcm_field_qualify_AppType Ip6UcastPublic error\n");
            return rv;
        }
        rv = bcm_field_qualify_AppType(unit, presel_id | BCM_FIELD_QUALIFY_PRESEL | presel_flags, bcmFieldAppTypeIp6UcastRpfPublic);
        if(rv) {
            printf("bcm_field_qualify_AppType Ip6UcastRpfPublic error\n");
            return rv;
        }
    } else {
        rv = bcm_field_qualify_AppType(unit, presel_id | BCM_FIELD_QUALIFY_PRESEL | presel_flags, app_type);
        if(rv) {
            printf("bcm_field_qualify_AppType error\n");
            return rv;
        }
    }

    bcm_field_group_config_t_init(&grp);
    BCM_FIELD_PRESEL_ADD(grp.preselset, presel_id);
    grp.mode = bcmFieldGroupModeAuto;
    grp.flags =
            BCM_FIELD_GROUP_CREATE_WITH_MODE |
            BCM_FIELD_GROUP_CREATE_WITH_ASET |
            BCM_FIELD_GROUP_CREATE_WITH_PRESELSET |
            BCM_FIELD_GROUP_CREATE_WITH_ID;

    BCM_FIELD_QSET_INIT(grp.qset);
    BCM_FIELD_ASET_INIT(grp.aset);
    BCM_FIELD_QSET_ADD(grp.qset, bcmFieldQualifyStageExternal);

    /*DstIp+SrcIp*/
    BCM_FIELD_QSET_ADD(grp.qset, bcmFieldQualifyDstIp6);
    BCM_FIELD_QSET_ADD(grp.qset, bcmFieldQualifySrcIp6);
    BCM_FIELD_ASET_ADD(grp.aset, bcmFieldActionExternalValue1Set);
    grp.group = ACL_BASE_GROUP_INDEX + HIT_BIT_1_QUAL;
    grp.priority = ACL_BASE_GROUP_PRIORITY;

    rv = bcm_field_group_config_create(unit, &grp);
    if(rv) {
        printf("bcm_field_group_config_create error\n");
        return rv;
    }

    /* Initialize the KBP */
    bshell(unit, "kbp deinit_appl");
    bshell(unit, "kbp init_appl");

    return rv;
}

int acl_ipv6_dip_sip_sharing_acl_entry_add_example(int unit, bcm_ip6_t dip, bcm_ip6_t sip, int val, int entry_index) {

    int rv = 0;
    bcm_ip6_t ip_mask = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };
    bcm_field_entry_t ent;

    bcm_field_entry_create(unit, ACL_BASE_GROUP_INDEX + 1, &ent);

    rv = bcm_field_qualify_DstIp6(unit, ent, dip, ip_mask);
    if(rv) {
        printf("bcm_field_qualify_DstIp6 error, ip: ");
        ipv6_print(dip);
        printf("\n");
        return rv;
    }
    rv = bcm_field_qualify_SrcIp6(unit, ent, sip, ip_mask);
    if(rv) {
        printf("bcm_field_qualify_SrcIp6 error, ip6: ");
        ipv6_print(sip);
        printf("\n");
        return rv;
    }
    rv = bcm_field_action_add(unit, ent, bcmFieldActionExternalValue1Set, val, val);
    if(rv) {
        printf("bcm_field_action_add error value1set, val = %d, val2 = %d\n", val, val);
        return rv;
    }

    rv = bcm_field_entry_prio_set(unit, ent, 10);
    if(rv) {
        printf("bcm_field_entry_prio_set error\n");
        return rv;
    }

    rv = bcm_field_entry_install(unit, ent);
    if(rv) {
        printf("bcm_field_entry_install error\n");
        return rv;
    }

    acl_ipv6_dip_sip_sharing_sem_entry[entry_index] = ent;

    return rv;
}

int acl_ipv6_dip_sip_sharing_semantic(int unit) {

    int rv = 0;
    int index;
    int init_val = 0x400;
    int val;
    bcm_ip6_t init_dip = { 0x00, 0xd1, 0x00, 0xd1, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
    bcm_ip6_t init_sip = { 0x00, 0x51, 0x00, 0x51, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
    bcm_ip6_t dip, sip;
    uint32 param0, param1;

    printf("        Configuring external group\n");
    rv = ipv6_dip_sip_sharing_config_kbp_example(unit, bcmFieldAppTypeAny);
    if(rv) {
        printf("acl_ipv6_dip_sip_sharing_kbp_config_example error\n");
        return rv;
    }

    printf("        Adding entries\n");
    /* set values here */
    sal_memcpy(dip, init_dip, 16);
    sal_memcpy(sip, init_sip, 16);
    val = init_val;

    for(index = 0; index < acl_ipv6_dip_sip_sharing_sem_entries; index++) {
        rv = acl_ipv6_dip_sip_sharing_acl_entry_add_example(unit, dip, sip, val, index);
        if(rv) {
            printf("acl_ipv6_dip_sip_sharing_sem_acl_entry_add_example error: index %d\n", index);
            return rv;
        }

        /* increment values here */
        dip[15]++;
        if(dip[15] == 0) { dip[14]++; }
        sip[15]++;
        if(sip[15] == 0) { sip[14]++; }
        val++;
    }

    printf("        Validating entries\n");
    /* set values here */
    sal_memcpy(dip, init_dip, 16);
    sal_memcpy(sip, init_sip, 16);
    val = init_val;

    for(index = 0; index < acl_ipv6_dip_sip_sharing_sem_entries; index++) {
        rv = bcm_field_action_get(unit, acl_ipv6_dip_sip_sharing_sem_entry[index], bcmFieldActionExternalValue1Set, &param0, &param1);
        if(rv) {
            printf("bcm_field_action_get bcmFieldActionExternalValue1Set error: index %d | entry %d\n", index, acl_ipv6_dip_sip_sharing_sem_entry[index]);
            return rv;
        }
        if(param0 != val) {
            printf("Error validating entry: index %d | DIP: ", index);
            ipv6_print(dip);
            printf(" | SIP: ");
            ipv6_print(sip);
            printf("\n");
            rv = -1;
            break;
        }

        /* increment values here */
        dip[15]++;
        if(dip[15] == 0) { dip[14]++; }
        sip[15]++;
        if(sip[15] == 0) { sip[14]++; }
        val++;
    }

    if(rv == 0) {
        printf("        Entries validated\n");
    } else {
        printf("        Entries validation failed\n");
        return rv;
    }

    printf("        Destroying entries\n");
    for(index = 0; index < acl_ipv6_dip_sip_sharing_sem_entries; index++) {
        rv = bcm_field_entry_destroy(unit, acl_ipv6_dip_sip_sharing_sem_entry[index]);
        if(rv) {
            printf("bcm_field_entry_destroy error: index %d | entry %d\n", index, acl_ipv6_dip_sip_sharing_sem_entry[index]);
            return rv;
        }
    }

    printf("        Verifying correct destruction\n");
    /* set values here */
    val = init_val;

    for(index = 0; index < acl_ipv6_dip_sip_sharing_sem_entries; index++) {
        /* disable console log because of the next intrusive messages */
        bshell(unit, "console off");
        rv = bcm_field_action_get(unit, acl_ipv6_dip_sip_sharing_sem_entry[index], bcmFieldActionExternalValue1Set, &param0, &param1);
        bshell(unit, "console on");

        if(rv == 0) {
            printf("Found entry that should be destroyed: index %d | entry %d\n", index, acl_ipv6_dip_sip_sharing_sem_entry[index]);
            rv -1;
            break;
        }

        /* increment values here */
        val++;
    }

    if(rv != 0) {
        printf("        All entries destroyed correctly\n");
        rv = 0;
    } else {
        printf("        Entries destruction failed\n");
    }

    return rv;
}

/*
    AT_Cint_external_frwd_ipv6_dip_sip_sharing_acl_traffic
*/
int frwd_ipv6_dip_sip_sharing_config_pmf_example(int unit, bcm_field_AppType_t app_type) {

    int rv = 0;
    bcm_field_group_config_t grp;
    bcm_field_entry_t ent;
    uint64 hit_value, hit_mask;
    int presel_id = 0; /* ID from 0 to 47 for Advanced mode */ 
    int presel_flags = 0; 

    /* Create Preselector */ 
    if(soc_property_get(unit, spn_FIELD_PRESEL_MGMT_ADVANCED_MODE, FALSE)) { 
      presel_flags = BCM_FIELD_QUALIFY_PRESEL_ADVANCED_MODE_STAGE_INGRESS; 
      rv = bcm_field_presel_create_stage_id(unit, bcmFieldStageIngress, presel_id); 
      if (BCM_E_NONE != rv) { 
          printf("Error in bcm_field_presel_create_stage_id\n"); 
          return rv; 
      } 
    } else { 
      rv = bcm_field_presel_create(unit, &presel_id); 
      if (BCM_E_NONE != rv) { 
          printf("Error in bcm_field_presel_create\n"); 
          return rv; 
      }
    } 

    rv = bcm_field_qualify_Stage(unit, presel_id | BCM_FIELD_QUALIFY_PRESEL | presel_flags, bcmFieldStageIngress); 
    if (BCM_E_NONE != rv) { 
      printf("Error in bcm_field_qualify_Stage\n"); 
      return rv; 
    } 

    rv = bcm_field_qualify_AppType(unit, presel_id | BCM_FIELD_QUALIFY_PRESEL | presel_flags, app_type);
    if(rv) {
        printf("bcm_field_qualify_AppType error\n");
        return rv;
    }

    bcm_field_group_config_t_init(&grp);
    BCM_FIELD_PRESEL_ADD(grp.preselset, presel_id);
    grp.group    = PMF_BASE_GROUP_INDEX     + HIT_BIT_1_QUAL;
    grp.priority = PMF_BASE_GROUP_PRIORITY  + HIT_BIT_1_QUAL;
    grp.mode     = bcmFieldGroupModeAuto;
    grp.flags    =
                BCM_FIELD_GROUP_CREATE_WITH_MODE |
                BCM_FIELD_GROUP_CREATE_WITH_ASET |
                BCM_FIELD_GROUP_CREATE_WITH_PRESELSET |
                BCM_FIELD_GROUP_CREATE_WITH_ID;

    BCM_FIELD_ASET_INIT(grp.aset);
    BCM_FIELD_ASET_ADD(grp.aset, bcmFieldActionDrop);

    BCM_FIELD_QSET_INIT(grp.qset);
    BCM_FIELD_QSET_ADD(grp.qset, bcmFieldQualifyStageIngress);
    BCM_FIELD_QSET_ADD(grp.qset, bcmFieldQualifyExternalHit1);
    BCM_FIELD_QSET_ADD(grp.qset, bcmFieldQualifyExternalValue1);

    rv = bcm_field_group_config_create(unit, &grp);
    if(rv) {
        printf("bcm_field_group_config_create error\n");
        return rv;
    }

    rv = bcm_field_entry_create(unit, grp.group, &ent);
    if(rv) {
        printf("Error in bcm_field_entry_create\n");
        return rv;
    }

    rv = bcm_field_qualify_ExternalHit1(unit, ent, 1, 1);
    if(rv) {
      printf("bcm_field_qualify_ExternalHit1 error\n");
      return rv;
    }

    COMPILER_64_ZERO(hit_value);
    COMPILER_64_ZERO(hit_mask);
    COMPILER_64_SET(hit_value, 0x00000000, 0x11111111);
    COMPILER_64_SET(hit_mask, 0x00000000, 0x11111111);
    rv = bcm_field_qualify_ExternalValue1(unit, ent, hit_value, hit_mask);
    if(rv) {
      printf("bcm_field_qualify_ExternalValue1 error\n");
      return rv;
    }

    rv = bcm_field_action_add(unit, ent, bcmFieldActionDrop, 0, 0);
    if(rv) {
        printf("Error in bcm_field_action_add\n");
        return rv;
    }

    rv = bcm_field_entry_install(unit, ent);
    if(rv) {
        printf("bcm_field_entry_install error\n");
        return rv;
    }

    return rv;
}

int frwd_ipv6_dip_sip_sharing_acl_entry_add_example(int unit, bcm_field_AppType_t app_type) {

    int rv = 0;
    int val = 0x00000000;
    int val2 = 0x11111111;
    bcm_ip6_t sip = { 0x12, 0x34, 0x56, 0x78, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x87, 0x65, 0x43, 0x21 };
    bcm_ip6_t dip = { 0xfe, 0xdc, 0xba, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xab, 0xcd, 0xef };
    bcm_ip6_t mc_dip = { 0xff, 0x12, 0x34, 0x56, 0x78, 0x9a, 0xbc, 0xde, 0xff, 0xed, 0xcb, 0xa9, 0x87, 0x65, 0x43, 0x21 };
    bcm_ip6_t ip_mask = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };
    bcm_field_entry_t ent;

    if (app_type != bcmFieldAppTypeAny &&
        app_type != bcmFieldAppTypeIp6Ucast &&
        app_type != bcmFieldAppTypeIp6UcastRpf &&
        app_type != bcmFieldAppTypeIp6Mcast &&
        app_type != bcmFieldAppTypeIp6UcastPublic &&
        app_type != bcmFieldAppTypeIp6UcastRpfPublic)
    {
        printf("Invalid AppType\n");
        return -1;
    }

    if(app_type == bcmFieldAppTypeAny) {
        /* bcmFieldAppTypeAny is used to indicate FIB scale prefix feature */
        app_type = bcmFieldAppTypeIp6Ucast;
    }

    bcm_field_entry_create(unit, ACL_BASE_GROUP_INDEX + 1, &ent);

    rv = bcm_field_qualify_DstIp6(unit, ent, (app_type == bcmFieldAppTypeIp6Mcast) ? mc_dip : dip, ip_mask);
    if(rv) {
        printf("bcm_field_qualify_DstIp6 error, ip: ");
        ipv6_print(dip);
        printf("\n");
        return rv;
    }
    rv = bcm_field_qualify_SrcIp6(unit, ent, sip, ip_mask);
    if(rv) {
        printf("bcm_field_qualify_SrcIp6 error, ip6: ");
        ipv6_print(sip);
        printf("\n");
        return rv;
    }
    rv = bcm_field_action_add(unit, ent, bcmFieldActionExternalValue1Set, val, val2);
    if(rv) {
        printf("bcm_field_action_add error value1set, val = %d, val2 = %d\n", val, val2);
        return rv;
    }

    rv = bcm_field_entry_prio_set(unit, ent, 10);
    if(rv) {
        printf("bcm_field_entry_prio_set error\n");
        return rv;
    }

    rv = bcm_field_entry_install(unit, ent);
    if(rv) {
        printf("bcm_field_entry_install error\n");
        return rv;
    }

    return rv;
}

int frwd_ipv6_dip_sip_sharing_acl_config_example(int unit, bcm_field_AppType_t app_type) {

    int rv = 0;

    if (app_type != bcmFieldAppTypeAny &&
        app_type != bcmFieldAppTypeIp6Ucast &&
        app_type != bcmFieldAppTypeIp6UcastRpf &&
        app_type != bcmFieldAppTypeIp6Mcast &&
        app_type != bcmFieldAppTypeIp6UcastPublic &&
        app_type != bcmFieldAppTypeIp6UcastRpfPublic)
    {
        printf("Invalid AppType\n");
        return -1;
    }

    if(app_type == bcmFieldAppTypeAny) {
        /* bcmFieldAppTypeAny is used to indicate FIB scale prefix feature */
        app_type = bcmFieldAppTypeIp6Ucast;
    }

    rv = ipv6_dip_sip_sharing_config_kbp_example(unit, app_type);
    if(rv) {
        printf("acl_ipv6_dip_sip_sharing_kbp_config_example error\n");
        return rv;
    }

    rv = frwd_ipv6_dip_sip_sharing_config_pmf_example(unit, app_type);
    if(rv) {
        printf("frwd_ipv6_dip_sip_sharing_pmf_config_example error\n");
        return rv;
    }

    return rv;
}

int frwd_ipv6_dip_sip_sharing_acl_config_traffic(int unit, bcm_field_AppType_t app_type, int in_port, int out_port, int out_port2) {

    int rv = 0;
    int vlan = 100;
    int vrf = 100;
    bcm_l3_intf_t intf;
    bcm_l3_intf_t intf_host;
    bcm_l3_ingress_t ingress_intf;
    bcm_l3_egress_t egress_intf;
    bcm_l3_host_t l3host;
    bcm_l3_route_t l3route;
    bcm_multicast_t ipmc_group;
    bcm_ipmc_addr_t ipmc_addr;

    int is_rpf = 0;
    int fib_scale_prefix_enabled = 0;

    bcm_ip6_t host_ip6 = { 0xfe, 0xdc, 0xba, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xab, 0xcd, 0xef };
    bcm_ip6_t rpf_ip6 = { 0x12, 0x34, 0x56, 0x78, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x87, 0x65, 0x43, 0x21 };
    bcm_ip6_t mc_ip6 = { 0xff, 0x12, 0x34, 0x56, 0x78, 0x9a, 0xbc, 0xde, 0xff, 0xed, 0xcb, 0xa9, 0x87, 0x65, 0x43, 0x21 };
    bcm_ip6_t mask_ip6 = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };
    bcm_ip6_t fib_prefix_long = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xf0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
    bcm_ip6_t fib_prefix_short = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

    bcm_mac_t my_mac = { 0x00, 0x0a, 0x00, 0x00, 0x0b, 0x00 };
    bcm_mac_t next_hop_mac = { 0x22, 0x00, 0x00, 0x00, 0x00, 0x22 };

    bcm_if_t eg_int;

    if (app_type != bcmFieldAppTypeAny &&
        app_type != bcmFieldAppTypeIp6Ucast &&
        app_type != bcmFieldAppTypeIp6UcastRpf &&
        app_type != bcmFieldAppTypeIp6Mcast &&
        app_type != bcmFieldAppTypeIp6UcastPublic &&
        app_type != bcmFieldAppTypeIp6UcastRpfPublic)
    {
        printf("Invalid AppType\n");
        return -1;
    }

    if(app_type == bcmFieldAppTypeAny) {
        /* bcmFieldAppTypeAny is used to indicate FIB scale prefix feature */
        app_type = bcmFieldAppTypeIp6Ucast;
        fib_scale_prefix_enabled = 1;
    }

    if(app_type == bcmFieldAppTypeIp6UcastRpf || app_type == bcmFieldAppTypeIp6UcastRpfPublic) {
        is_rpf = 1;
    }

    if(app_type == bcmFieldAppTypeIp6UcastPublic || app_type == bcmFieldAppTypeIp6UcastRpfPublic) {
        vrf = 0;
    }

    if(app_type == bcmFieldAppTypeIp6Mcast) {
        /* MC forwarding */
        bcm_l3_intf_t_init(&intf);
        sal_memcpy(intf.l3a_mac_addr, my_mac, 6);
        intf.l3a_vrf = vrf;
        intf.l3a_vid = vlan;
        rv = bcm_l3_intf_create(unit, &intf);
        if(rv) {
            printf("Error, bcm_l3_intf_create MC\n");
            return rv;
        }

        rv = bcm_multicast_create(unit, BCM_MULTICAST_INGRESS_GROUP | BCM_MULTICAST_TYPE_L3 | BCM_MULTICAST_WITH_ID, ipmc_group);
        if(rv != BCM_E_NONE) {
            printf("Error, bcm_multicast_create\n");
            return rv;
        }

        rv = bcm_multicast_ingress_add(unit, ipmc_group, out_port, intf.l3a_intf_id);
        if(rv != BCM_E_NONE) {
            printf("Error, bcm_multicast_ingress_add\n");
            return rv;
        }

        rv = bcm_multicast_ingress_add(unit, ipmc_group, out_port2, intf.l3a_intf_id);
        if(rv != BCM_E_NONE) {
            printf("Error, bcm_multicast_ingress_add\n");
            return rv;
        }

        bcm_ipmc_addr_t_init(&ipmc_addr);
        ipmc_addr.flags = BCM_IPMC_SOURCE_PORT_NOCHECK | BCM_IPMC_IP6;
        sal_memcpy(ipmc_addr.mc_ip6_addr, mc_ip6, 16);
        sal_memcpy(ipmc_addr.mc_ip6_mask, mask_ip6, 16);
        ipmc_addr.vrf = vrf;
        ipmc_addr.group = ipmc_group;

        rv = bcm_ipmc_add(unit, ipmc_addr);
        if(rv != BCM_E_NONE) {
            printf("Error, bcm_ipmc_add\n");
            return rv;
        }

        return rv;
    }

    /* UC forwarding */
    bcm_l3_intf_t_init(&intf);
    sal_memcpy(intf.l3a_mac_addr, my_mac, 6);
    intf.l3a_vrf = vrf;
    intf.l3a_vid = vlan;
    if(is_rpf) {
        ingress_intf.flags |= BCM_L3_RPF;
        ingress_intf.urpf_mode = bcmL3IngressUrpfStrict;
    }
    rv = bcm_l3_intf_create(unit, &intf);
    if(rv) {
        printf("Error, bcm_l3_intf_create\n");
        return rv;
    }

    bcm_l3_ingress_t_init(&ingress_intf);
    ingress_intf.flags = BCM_L3_INGRESS_WITH_ID;
    ingress_intf.vrf = vrf;
    if(is_rpf) {
        ingress_intf.flags |= BCM_L3_RPF;
        ingress_intf.urpf_mode = bcmL3IngressUrpfStrict;
    }
    if(vrf == 0) {
        ingress_intf.flags |= BCM_L3_INGRESS_GLOBAL_ROUTE;
    }
    rv = bcm_l3_ingress_create(unit, &ingress_intf, &intf.l3a_intf_id);
    if(rv) {
        printf("Error, bcm_l3_ingress_create\n");
        return rv;
    }

    bcm_l3_intf_t_init(&intf_host);
    ingress_intf.flags = BCM_L3_INGRESS_WITH_ID;
    sal_memcpy(intf_host.l3a_mac_addr, my_mac, 6);
    intf_host.l3a_vrf = vrf;
    intf_host.l3a_vid = vlan + 1;
    rv = bcm_l3_intf_create(unit, &intf_host);
    if(rv) {
        printf("Error, bcm_l3_intf_create host\n");
        return rv;
    }

    bcm_l3_egress_t_init(&egress_intf);
    egress_intf.port = out_port;
    sal_memcpy(egress_intf.mac_addr, next_hop_mac, 6);
    egress_intf.intf = intf_host.l3a_intf_id;
    egress_intf.vlan = vlan;
    rv = bcm_l3_egress_create(unit, 0, &egress_intf, &eg_int);
    if(rv) {
        printf("Error, bcm_l3_egress_create\n");
        return rv;
    }

    if(fib_scale_prefix_enabled) {
        /* FIB scale prefix */
        bcm_l3_route_t_init(&l3route);
        l3route.l3a_flags = BCM_L3_IP6;
        sal_memcpy(l3route.l3a_ip6_net, host_ip6, 16);
        sal_memcpy(l3route.l3a_ip6_mask, fib_prefix_long, 16);
        l3route.l3a_vrf = vrf;
        l3route.l3a_intf = eg_int;

        rv = bcm_l3_route_add(unit, &l3route);
        if(rv) {
            printf("bcm_l3_route_add fib_scale_prefix_long error\n");
            return rv;
        }
        bcm_l3_route_t_init(&l3route);
        l3route.l3a_flags = BCM_L3_IP6;
        l3route.l3a_flags2 = BCM_L3_FLAGS2_SCALE_ROUTE;
        sal_memcpy(l3route.l3a_ip6_net, host_ip6, 16);
        sal_memcpy(l3route.l3a_ip6_mask, fib_prefix_short, 16);
        l3route.l3a_vrf = vrf;
        l3route.l3a_intf = eg_int;

        rv = bcm_l3_route_add(unit, &l3route);
        if(rv) {
            printf("bcm_l3_route_add fib_scale_prefix_short error\n");
            return rv;
        }

        return rv;
    }

    bcm_l3_host_t_init(&l3host);
    l3host.l3a_flags = BCM_L3_IP6;
    sal_memcpy(l3host.l3a_ip6_addr, host_ip6, 16);
    l3host.l3a_vrf = vrf;
    l3host.l3a_intf = eg_int;

    rv = bcm_l3_host_add(unit, &l3host);
    if(rv) {
        printf("bcm_l3_host_add error\n");
        return rv;
    }

    if(is_rpf) {
        bcm_l3_host_t_init(&l3host);
        l3host.l3a_flags = BCM_L3_IP6;
        sal_memcpy(l3host.l3a_ip6_addr, rpf_ip6, 16);
        l3host.l3a_vrf = vrf;
        l3host.l3a_intf = eg_int;

        rv = bcm_l3_host_add(unit, &l3host);
        if(rv) {
            printf("bcm_l3_host_add rpf error\n");
            return rv;
        }
    }

    return rv;
}

int acl_dc_48_24_config_example(int unit, int out_port, int dc24) {

    /* KBP/PMF variables */
    int i, rv = 0;
    int presel_id;
    bcm_field_group_config_t grp;
    bcm_field_entry_t ent;
    uint64 hit_value, hit_mask;

    uint32 fec1mask0 = dc24 ? 0x0000ffff : 0x0000ffff;
    uint32 fec1mask1 = dc24 ? 0xff000000 : 0xffffffff;
    uint32 fec2mask0 = dc24 ? 0x00000000 : 0x0000ffff;
    uint32 fec2mask1 = dc24 ? 0x00ffffff : 0xffffffff;

    bcm_if_t fec1val0;
    bcm_if_t fec1val1;
    bcm_if_t fec2val0;
    bcm_if_t fec2val1;

    uint32 val0 = 0x00000000;
    uint32 val0mask = 0x00000000;
    uint32 acl1val1 = dc24 ? 0x00f1111f : 0x0000f11f;
    uint32 acl1val1mask = dc24 ? 0x00ffffff : 0x0000ffff;
    uint32 acl3val1hit1 = 0xa5a5a5a5;
    uint32 acl3val1hit2 = 0xc2c2c2c2;
    uint32 acl3val1mask = 0xffffffff;
    uint32 acl4val1 = 0xf444444f;
    uint32 acl4val1mask = 0xffffffff;
    uint32 acl5val0 = 0x000000f5;
    uint32 acl5val0mask = 0x000000ff;
    uint32 acl5val1 = 0x5555555f;
    uint32 acl5val1mask = 0xffffffff;

    /* traffic variables */
    int vlan = 100;
    int vrf = 100;
    bcm_l3_intf_t intf;
    bcm_l3_intf_t intf2;
    bcm_l3_intf_t intf_host;
    bcm_l3_intf_t intf_host2;
    bcm_l3_ingress_t ingress_intf;
    bcm_l3_ingress_t ingress_intf2;
    bcm_l3_egress_t egress_intf;
    bcm_if_t eg_int1;
    bcm_if_t eg_int2;
    bcm_l3_host_t l3host;
    bcm_l3_route_t l3route;
    int ip_hit1 = 0x0a000001;
    int ip_hit2 = 0x0b000001;
    int ip_mask = 0xffffffff;
    bcm_mac_t my_mac = { 0x00, 0x0a, 0x00, 0x00, 0x0b, 0x00 };
    bcm_mac_t my_mac2 = { 0x00, 0x0a, 0x00, 0x00, 0x0b, 0x0c };
    bcm_mac_t next_hop_mac = { 0x11, 0x00, 0x00, 0x00, 0x00, 0x11 };
    bcm_mac_t next_hop_mac2 = { 0x22, 0x00, 0x00, 0x00, 0x00, 0x22 };


    /*----------------------------------------------------------------------------------------------
     * KBP config
     */
    rv = bcm_field_presel_create(unit, &presel_id);
    if(rv) {
        printf("bcm_field_presel_create error\n");
        return rv;
    }

    rv = bcm_field_qualify_Stage(unit, presel_id | BCM_FIELD_QUALIFY_PRESEL, bcmFieldStageExternal);
    if(rv) {
        printf("bcm_field_qualify_Stage External error\n");
        return rv;
    }

    rv = bcm_field_qualify_AppType(unit, presel_id | BCM_FIELD_QUALIFY_PRESEL, bcmFieldAppTypeIp4DoubleCapacity);
    if(rv) {
        printf("bcm_field_qualify_AppType error\n");
        return rv;
    }

    bcm_field_group_config_t_init(&grp);
    BCM_FIELD_PRESEL_ADD(grp.preselset, presel_id);
    grp.mode = bcmFieldGroupModeAuto;
    grp.flags =
            BCM_FIELD_GROUP_CREATE_WITH_MODE |
            BCM_FIELD_GROUP_CREATE_WITH_ASET |
            BCM_FIELD_GROUP_CREATE_WITH_PRESELSET |
            BCM_FIELD_GROUP_CREATE_WITH_ID;

    BCM_FIELD_QSET_INIT(grp.qset);
    BCM_FIELD_ASET_INIT(grp.aset);
    BCM_FIELD_QSET_ADD(grp.qset, bcmFieldQualifyStageExternal);

    BCM_FIELD_QSET_ADD(grp.qset, bcmFieldQualifyDstIp);
    BCM_FIELD_ASET_ADD(grp.aset, bcmFieldActionExternalValue1Set);
    grp.group = ACL_BASE_GROUP_INDEX + HIT_BIT_1_QUAL;
    grp.priority = ACL_BASE_GROUP_PRIORITY + HIT_BIT_1_QUAL;

    rv = bcm_field_group_config_create(unit, &grp);
    if(rv) {
        printf("bcm_field_group_config_create group 1 error\n");
        return rv;
    }

    BCM_FIELD_ASET_REMOVE(grp.aset, bcmFieldActionExternalValue1Set);
    BCM_FIELD_ASET_ADD(grp.aset, bcmFieldActionExternalValue3Set);
    grp.group = ACL_BASE_GROUP_INDEX + HIT_BIT_3_QUAL;
    grp.priority = ACL_BASE_GROUP_PRIORITY + HIT_BIT_3_QUAL;

    rv = bcm_field_group_config_create(unit, &grp);
    if(rv) {
        printf("bcm_field_group_config_create group 3 error\n");
        return rv;
    }

    BCM_FIELD_ASET_REMOVE(grp.aset, bcmFieldActionExternalValue3Set);
    BCM_FIELD_ASET_ADD(grp.aset, bcmFieldActionExternalValue4Set);
    grp.group = ACL_BASE_GROUP_INDEX + HIT_BIT_4_QUAL;
    grp.priority = ACL_BASE_GROUP_PRIORITY + HIT_BIT_4_QUAL;

    rv = bcm_field_group_config_create(unit, &grp);
    if(rv) {
        printf("bcm_field_group_config_create group 4 error\n");
        return rv;
    }

    if(dc24) {
        BCM_FIELD_ASET_REMOVE(grp.aset, bcmFieldActionExternalValue4Set);
        BCM_FIELD_ASET_ADD(grp.aset, bcmFieldActionExternalValue5Set);
        grp.group = ACL_BASE_GROUP_INDEX + HIT_BIT_5_QUAL;
        grp.priority = ACL_BASE_GROUP_PRIORITY + HIT_BIT_5_QUAL;

        rv = bcm_field_group_config_create(unit, &grp);
        if(rv) {
            printf("bcm_field_group_config_create group 5 error\n");
            return rv;
        }
    }

    /* Initialize the KBP */
    bshell(unit, "kbp deinit_appl");
    bshell(unit, "kbp init_appl");


    /*----------------------------------------------------------------------------------------------
     * traffic config
     */
    bcm_l3_intf_t_init(&intf);
    sal_memcpy(intf.l3a_mac_addr, my_mac, 6);
    intf.l3a_vrf = vrf;
    intf.l3a_vid = vlan;
    rv = bcm_l3_intf_create(unit, &intf);
    if(rv) {
        printf("Error, bcm_l3_intf_create\n");
        return rv;
    }

    bcm_l3_ingress_t_init(&ingress_intf);
    ingress_intf.flags =
                        BCM_L3_INGRESS_WITH_ID |
                        BCM_L3_INGRESS_EXT_IPV4_DOUBLE_CAPACITY |
                        BCM_L3_INGRESS_ROUTE_DISABLE_IP4_MCAST |
                        BCM_L3_INGRESS_ROUTE_DISABLE_IP6_MCAST;
    ingress_intf.vrf = vrf;
    rv = bcm_l3_ingress_create(unit, &ingress_intf, &intf.l3a_intf_id);
    if(rv) {
        printf("Error, bcm_l3_ingress_create\n");
        return rv;
    }



    bcm_l3_intf_t_init(&intf2);
    sal_memcpy(intf2.l3a_mac_addr, my_mac2, 6);
    intf2.l3a_vrf = vrf + 2;
    intf2.l3a_vid = vlan + 2;
    rv = bcm_l3_intf_create(unit, &intf2);
    if(rv) {
        printf("Error, bcm_l3_intf_create2\n");
        return rv;
    }

    bcm_l3_ingress_t_init(&ingress_intf2);
    ingress_intf2.flags =
                        BCM_L3_INGRESS_WITH_ID |
                        BCM_L3_INGRESS_EXT_IPV4_DOUBLE_CAPACITY |
                        BCM_L3_INGRESS_ROUTE_DISABLE_IP4_MCAST |
                        BCM_L3_INGRESS_ROUTE_DISABLE_IP6_MCAST;
    ingress_intf2.vrf = vrf + 2;
    rv = bcm_l3_ingress_create(unit, &ingress_intf2, &intf2.l3a_intf_id);
    if(rv) {
        printf("Error, bcm_l3_ingress_create2\n");
        return rv;
    }

    bcm_l3_intf_t_init(&intf_host);
    sal_memcpy(intf_host.l3a_mac_addr, my_mac, 6);
    intf_host.l3a_vrf = vrf;
    intf_host.l3a_vid = vlan + 1;
    rv = bcm_l3_intf_create(unit, &intf_host);
    if(rv) {
        printf("Error, bcm_l3_intf_create host\n");
        return rv;
    }
    bcm_l3_intf_t_init(&intf_host2);
    sal_memcpy(intf_host2.l3a_mac_addr, my_mac2, 6);
    intf_host2.l3a_vrf = vrf + 2;
    intf_host2.l3a_vid = vlan + 3;
    rv = bcm_l3_intf_create(unit, &intf_host2);
    if(rv) {
        printf("Error, bcm_l3_intf_create host2\n");
        return rv;
    }

    for(i = 0; i < 3; i++) {
        bcm_l3_egress_t_init(&egress_intf);
        egress_intf.port = out_port;
        sal_memcpy(egress_intf.mac_addr, next_hop_mac, 6);
        egress_intf.intf = intf_host.l3a_intf_id;
        egress_intf.vlan = vlan;
        rv = bcm_l3_egress_create(unit, 0, &egress_intf, &eg_int1);
        if(rv) {
            printf("Error, bcm_l3_egress_create\n");
            return rv;
        }
    }

    bcm_l3_egress_t_init(&egress_intf);
    egress_intf.port = out_port;
    sal_memcpy(egress_intf.mac_addr, next_hop_mac2, 6);
    egress_intf.intf = intf_host2.l3a_intf_id;
    egress_intf.vlan = vlan;
    rv = bcm_l3_egress_create(unit, 0, &egress_intf, &eg_int2);
    if(rv) {
        printf("Error, bcm_l3_egress_create2\n");
        return rv;
    }

    if(dc24) { /* IPv4 DC 24bit FWD supports only routes */
        bcm_l3_route_t_init(&l3route);
        l3route.l3a_subnet = ip_hit1;
        l3route.l3a_ip_mask = ip_mask;
        l3route.l3a_vrf = vrf;
        l3route.l3a_intf = eg_int1;
        l3route.l3a_flags2 = BCM_L3_FLAGS2_SCALE_ROUTE;
        for(i = 0; i < 4096; i++) {
            rv = bcm_l3_route_add(unit, &l3route);
            if(rv) {
                printf("Error, bcm_l3_route_add Failed, l3a_subnet=0x%x\n", l3route.l3a_subnet);
                return rv;
            }
            l3route.l3a_subnet++;
        }

        bcm_l3_route_t_init(&l3route);
        l3route.l3a_subnet = ip_hit2;
        l3route.l3a_ip_mask = ip_mask;
        l3route.l3a_vrf = vrf + 2;
        l3route.l3a_intf = eg_int2;
        l3route.l3a_flags2 = BCM_L3_FLAGS2_SCALE_ROUTE;
        rv = bcm_l3_route_add(unit, &l3route);
        if(rv) {
            printf("Error, bcm_l3_route_add Failed, l3a_subnet=0x%x\n", l3route.l3a_subnet);
            return rv;
        }
    } else {
        bcm_l3_host_t_init(&l3host);
        l3host.l3a_ip_addr = ip_hit1;
        l3host.l3a_vrf = vrf;
        l3host.l3a_intf = eg_int1;
        l3host.l3a_flags2 = BCM_L3_FLAGS2_SCALE_ROUTE;
        for(i = 0; i < 4096; i++) {
            rv = bcm_l3_host_add(unit, &l3host);
            if(rv) {
                printf("Error, bcm_l3_route_add Failed, l3a_ip_addr=0x%x\n", l3host.l3a_ip_addr);
                return rv;
            }
            l3host.l3a_ip_addr++;
        }

        bcm_l3_host_t_init(&l3host);
        l3host.l3a_ip_addr = ip_hit2;
        l3host.l3a_vrf = vrf + 2;
        l3host.l3a_intf = eg_int2;
        l3host.l3a_flags2 = BCM_L3_FLAGS2_SCALE_ROUTE;
        rv = bcm_l3_host_add(unit, &l3host);
        if(rv) {
            printf("Error, bcm_l3_route_add Failed, l3a_ip_addr=0x%x\n", l3host.l3a_ip_addr);
            return rv;
        }
    }



    /*----------------------------------------------------------------------------------------------
     * FMP config
     */
    presel_id = 0;
    rv = bcm_field_presel_create(unit, &presel_id);
    if(rv) {
        printf("bcm_field_presel_create error\n");
        return rv;
    }

    rv = bcm_field_qualify_Stage(unit, presel_id | BCM_FIELD_QUALIFY_PRESEL, bcmFieldStageIngress);
    if(rv) {
        printf("bcm_field_qualify_Stage error\n");
        return rv;
    }

    rv = bcm_field_qualify_AppType(unit, presel_id | BCM_FIELD_QUALIFY_PRESEL, bcmFieldAppTypeIp4DoubleCapacity);
    if(rv) {
        printf("bcm_field_qualify_AppType error\n");
        return rv;
    }

    bcm_field_group_config_t_init(&grp);
    BCM_FIELD_PRESEL_ADD(grp.preselset, presel_id);
    grp.group    = PMF_BASE_GROUP_INDEX     + HIT_BIT_1_QUAL;
    grp.priority = PMF_BASE_GROUP_PRIORITY  + HIT_BIT_1_QUAL;
    grp.mode     = bcmFieldGroupModeAuto;
    grp.flags    =
                BCM_FIELD_GROUP_CREATE_WITH_MODE |
                BCM_FIELD_GROUP_CREATE_WITH_ASET |
                BCM_FIELD_GROUP_CREATE_WITH_PRESELSET |
                BCM_FIELD_GROUP_CREATE_WITH_ID;

    BCM_FIELD_ASET_INIT(grp.aset);
    BCM_FIELD_ASET_ADD(grp.aset, bcmFieldActionDrop);

    BCM_FIELD_QSET_INIT(grp.qset);
    BCM_FIELD_QSET_ADD(grp.qset, bcmFieldQualifyStageIngress);
    BCM_FIELD_QSET_ADD(grp.qset, bcmFieldQualifyExternalHit0);
    BCM_FIELD_QSET_ADD(grp.qset, bcmFieldQualifyExternalValue0);
    BCM_FIELD_QSET_ADD(grp.qset, bcmFieldQualifyExternalHit1);
    BCM_FIELD_QSET_ADD(grp.qset, bcmFieldQualifyExternalValue1);
    BCM_FIELD_QSET_ADD(grp.qset, bcmFieldQualifyExternalHit3);
    BCM_FIELD_QSET_ADD(grp.qset, bcmFieldQualifyExternalValue3);
    BCM_FIELD_QSET_ADD(grp.qset, bcmFieldQualifyExternalHit4);
    BCM_FIELD_QSET_ADD(grp.qset, bcmFieldQualifyExternalValue4);
    if(dc24) {
        BCM_FIELD_QSET_ADD(grp.qset, bcmFieldQualifyExternalHit5);
        BCM_FIELD_QSET_ADD(grp.qset, bcmFieldQualifyExternalValue5);
    }

    rv = bcm_field_group_config_create(unit, &grp);
    if(rv) {
        printf("bcm_field_group_config_create error\n");
        return rv;
    }

    rv = bcm_field_entry_create(unit, grp.group, &ent);
    if(rv) {
        printf("Error in bcm_field_entry_create\n");
        return rv;
    }

    /* FWD-0 */
    rv = bcm_field_qualify_ExternalHit0(unit, ent, 1, 1);
    if(rv) {
      printf("bcm_field_qualify_ExternalHit0 error\n");
      return rv;
    }
    /* modify FEC id to get expected result */
    fec1val0 = dc24 ? (BCM_L3_ITF_VAL_GET(eg_int1) | 0x00020000) >> 8 : 0x00000000;
    fec1val1 = dc24 ? (BCM_L3_ITF_VAL_GET(eg_int1) | 0x00020000) << 24 : BCM_L3_ITF_VAL_GET(eg_int1) | 0x00020000;
    COMPILER_64_ZERO(hit_value);
    COMPILER_64_ZERO(hit_mask);
    COMPILER_64_SET(hit_value, fec1val0, fec1val1);
    COMPILER_64_SET(hit_mask, fec1mask0, fec1mask1);
    rv = bcm_field_qualify_ExternalValue0(unit, ent, hit_value, hit_mask);
    if(rv) {
        printf("bcm_field_qualify_ExternalValue0 error\n");
        return rv;
    }

    /* ACL-1 */
    rv = bcm_field_qualify_ExternalHit1(unit, ent, 1, 1);
    if(rv) {
      printf("bcm_field_qualify_ExternalHit1 error\n");
      return rv;
    }
    COMPILER_64_ZERO(hit_value);
    COMPILER_64_ZERO(hit_mask);
    COMPILER_64_SET(hit_value, val0, acl1val1);
    COMPILER_64_SET(hit_mask, val0mask, acl1val1mask);
    rv = bcm_field_qualify_ExternalValue1(unit, ent, hit_value, hit_mask);
    if(rv) {
        printf("bcm_field_qualify_ExternalValue1 error\n");
        return rv;
    }

    /* ACL-3*/
    rv = bcm_field_qualify_ExternalHit3(unit, ent, 1, 1);
    if(rv) {
      printf("bcm_field_qualify_ExternalHit3 error\n");
      return rv;
    }
    COMPILER_64_ZERO(hit_value);
    COMPILER_64_ZERO(hit_mask);
    COMPILER_64_SET(hit_value, val0, acl3val1hit1);
    COMPILER_64_SET(hit_mask, val0mask, acl3val1mask);
    rv = bcm_field_qualify_ExternalValue3(unit, ent, hit_value, hit_mask);
    if(rv) {
        printf("bcm_field_qualify_ExternalValue3 error\n");
        return rv;
    }

    /* ACL-4 */
    rv = bcm_field_qualify_ExternalHit4(unit, ent, 1, 1);
    if(rv) {
      printf("bcm_field_qualify_ExternalHit4 error\n");
      return rv;
    }
    COMPILER_64_ZERO(hit_value);
    COMPILER_64_ZERO(hit_mask);
    COMPILER_64_SET(hit_value, val0, acl4val1);
    COMPILER_64_SET(hit_mask, val0mask, acl4val1mask);
    rv = bcm_field_qualify_ExternalValue4(unit, ent, hit_value, hit_mask);
    if(rv) {
        printf("bcm_field_qualify_ExternalValue4 error\n");
        return rv;
    }

    if(dc24) {
        /* ACL-5 */
        rv = bcm_field_qualify_ExternalHit5(unit, ent, 1, 1);
        if(rv) {
            printf("bcm_field_qualify_ExternalHit5 error\n");
            return rv;
        }
        COMPILER_64_ZERO(hit_value);
        COMPILER_64_ZERO(hit_mask);
        COMPILER_64_SET(hit_value, acl5val0, acl5val1);
        COMPILER_64_SET(hit_mask, acl5val0mask, acl5val1mask);
        rv = bcm_field_qualify_ExternalValue5(unit, ent, hit_value, hit_mask);
        if(rv) {
            printf("bcm_field_qualify_ExternalValue5 error\n");
            return rv;
        }
    }

    rv = bcm_field_action_add(unit, ent, bcmFieldActionDrop, 0, 0);
    if(rv) {
        printf("Error in bcm_field_action_add\n");
        return rv;
    }

    rv = bcm_field_entry_install(unit, ent);
    if(rv) {
        printf("bcm_field_entry_install error\n");
        return rv;
    }

    /* second hit */

    presel_id = 0;
    rv = bcm_field_presel_create(unit, &presel_id);
    if(rv) {
        printf("bcm_field_presel_create error\n");
        return rv;
    }

    rv = bcm_field_qualify_Stage(unit, presel_id | BCM_FIELD_QUALIFY_PRESEL, bcmFieldStageIngress);
    if(rv) {
        printf("bcm_field_qualify_Stage error\n");
        return rv;
    }

    rv = bcm_field_qualify_AppType(unit, presel_id | BCM_FIELD_QUALIFY_PRESEL, bcmFieldAppTypeIp4DoubleCapacity);
    if(rv) {
        printf("bcm_field_qualify_AppType error\n");
        return rv;
    }

    grp.group    = PMF_BASE_GROUP_INDEX     + HIT_BIT_1_QUAL + 6;
    grp.priority = PMF_BASE_GROUP_PRIORITY  + HIT_BIT_1_QUAL + 6;

    BCM_FIELD_QSET_REMOVE(grp.qset, bcmFieldQualifyExternalHit0);
    BCM_FIELD_QSET_REMOVE(grp.qset, bcmFieldQualifyExternalValue0);
    BCM_FIELD_QSET_ADD(grp.qset, bcmFieldQualifyExternalHit2);
    BCM_FIELD_QSET_ADD(grp.qset, bcmFieldQualifyExternalValue2);

    rv = bcm_field_group_config_create(unit, &grp);
    if(rv) {
        printf("bcm_field_group_config_create error\n");
        return rv;
    }

    rv = bcm_field_entry_create(unit, grp.group, &ent);
    if(rv) {
        printf("Error in bcm_field_entry_create\n");
        return rv;
    }

    /* FWD-2 */
    rv = bcm_field_qualify_ExternalHit2(unit, ent, 1, 1);
    if(rv) {
        printf("bcm_field_qualify_ExternalHit2 error\n");
        return rv;
    }
    /* modify FEC id to get expected result */
    fec2val0 = 0x00000000;
    fec2val1 = BCM_L3_ITF_VAL_GET(eg_int2) | 0x00020000;
    COMPILER_64_ZERO(hit_value);
    COMPILER_64_ZERO(hit_mask);
    COMPILER_64_SET(hit_value, fec2val0, fec2val1);
    COMPILER_64_SET(hit_mask, fec2mask0, fec2mask1);
    rv = bcm_field_qualify_ExternalValue2(unit, ent, hit_value, hit_mask);
    if(rv) {
        printf("bcm_field_qualify_ExternalValue0 error\n");
        return rv;
    }

    /* ACL-1 */
    rv = bcm_field_qualify_ExternalHit1(unit, ent, 1, 1);
    if(rv) {
        printf("bcm_field_qualify_ExternalHit1 error\n");
        return rv;
    }
    COMPILER_64_ZERO(hit_value);
    COMPILER_64_ZERO(hit_mask);
    COMPILER_64_SET(hit_value, val0, acl1val1);
    COMPILER_64_SET(hit_mask, val0mask, acl1val1mask);
    rv = bcm_field_qualify_ExternalValue1(unit, ent, hit_value, hit_mask);
    if(rv) {
        printf("bcm_field_qualify_ExternalValue1 error\n");
        return rv;
    }

    /* ACL-3 */
    rv = bcm_field_qualify_ExternalHit3(unit, ent, 1, 1);
    if(rv) {
        printf("bcm_field_qualify_ExternalHit3 error\n");
        return rv;
    }
    COMPILER_64_ZERO(hit_value);
    COMPILER_64_ZERO(hit_mask);
    COMPILER_64_SET(hit_value, val0, acl3val1hit2);
    COMPILER_64_SET(hit_mask, val0mask, acl3val1mask);
    rv = bcm_field_qualify_ExternalValue3(unit, ent, hit_value, hit_mask);
    if(rv) {
        printf("bcm_field_qualify_ExternalValue3 error\n");
        return rv;
    }

    /* ACL-4 */
    rv = bcm_field_qualify_ExternalHit4(unit, ent, 1, 1);
    if(rv) {
        printf("bcm_field_qualify_ExternalHit4 error\n");
        return rv;
    }
    COMPILER_64_ZERO(hit_value);
    COMPILER_64_ZERO(hit_mask);
    COMPILER_64_SET(hit_value, val0, acl4val1);
    COMPILER_64_SET(hit_mask, val0mask, acl4val1mask);
    rv = bcm_field_qualify_ExternalValue4(unit, ent, hit_value, hit_mask);
    if(rv) {
        printf("bcm_field_qualify_ExternalValue4 error\n");
        return rv;
    }

    if(dc24) {
        /* ACL-5 */
        rv = bcm_field_qualify_ExternalHit5(unit, ent, 1, 1);
        if(rv) {
            printf("bcm_field_qualify_ExternalHit5 error\n");
            return rv;
        }
        COMPILER_64_ZERO(hit_value);
        COMPILER_64_ZERO(hit_mask);
        COMPILER_64_SET(hit_value, acl5val0, acl5val1);
        COMPILER_64_SET(hit_mask, acl5val0mask, acl5val1mask);
        rv = bcm_field_qualify_ExternalValue5(unit, ent, hit_value, hit_mask);
        if(rv) {
            printf("bcm_field_qualify_ExternalValue5 error\n");
            return rv;
        }
    }

    rv = bcm_field_action_add(unit, ent, bcmFieldActionDrop, 0, 0);
    if(rv) {
        printf("Error in bcm_field_action_add\n");
        return rv;
    }

    rv = bcm_field_entry_install(unit, ent);
    if(rv) {
        printf("bcm_field_entry_install error\n");
        return rv;
    }

    return rv;
}

int acl_dc_48_24_entry_add_example(int unit, int hit2, int dc24) {

    int rv;
    bcm_field_entry_t ent_elk;

    uint32 ip_addr = hit2 ? 0x0b000001 : 0x0a000001;
    uint32 ip_mask = 0xffffffff;
    uint32 acl1val0 = dc24 ? 0x00000000 : 0x00000000;
    uint32 acl1val1 = dc24 ? 0x00f1111f : 0x0000f11f;
    uint32 acl3val0 = 0x00000000;
    uint32 acl3val1 = hit2 ? 0xc2c2c2c2 : 0xa5a5a5a5;
    uint32 acl4val0 = 0x00000000;
    uint32 acl4val1 = 0xf444444f;
    uint32 acl5val0 = 0x5555555f;
    uint32 acl5val1 = 0xf5000000;
    int hitbit;
    int offset = 20;

    /* ACL-1 */
    hitbit = HIT_BIT_1_QUAL;
    bcm_field_entry_create(unit, ACL_BASE_GROUP_INDEX + hitbit, &ent_elk);
    rv = bcm_field_qualify_DstIp(unit, ent_elk, ip_addr, ip_mask);
    if(rv) {
        printf("bcm_field_qualify_DstIp error\n");
        return rv;
    }

    rv = bcm_field_action_add(unit, ent_elk, bcmFieldActionExternalValue1Set, acl1val0, acl1val1);
    if(rv) {
        printf("bcm_field_action_add %d error\n", hitbit);
        return rv;
    }

    rv = bcm_field_entry_prio_set(unit, ent_elk, offset + hitbit);
    if(rv) {
        printf("bcm_field_entry_prio_set %d error\n", hitbit);
        return rv;
    }

    rv = bcm_field_entry_install(unit, ent_elk);
    if(rv) {
        printf("bcm_field_entry_install %d error\n", hitbit);
        return rv;
    }

    /* ACL-3 */
    hitbit = HIT_BIT_3_QUAL;
    bcm_field_entry_create(unit, ACL_BASE_GROUP_INDEX + hitbit, &ent_elk);
    rv = bcm_field_qualify_DstIp(unit, ent_elk, ip_addr, ip_mask);
    if(rv) {
        printf("bcm_field_qualify_DstIp error\n");
        return rv;
    }

    rv = bcm_field_action_add(unit, ent_elk, bcmFieldActionExternalValue3Set, acl3val0, acl3val1);
    if(rv) {
        printf("bcm_field_action_add %d error\n", hitbit);
        return rv;
    }

    rv = bcm_field_entry_prio_set(unit, ent_elk, offset + hitbit);
    if(rv) {
        printf("bcm_field_entry_prio_set %d error\n", hitbit);
        return rv;
    }

    rv = bcm_field_entry_install(unit, ent_elk);
    if(rv) {
        printf("bcm_field_entry_install %d error\n", hitbit);
        return rv;
    }

    /* ACL-4 */
    hitbit = HIT_BIT_4_QUAL;
    bcm_field_entry_create(unit, ACL_BASE_GROUP_INDEX + hitbit, &ent_elk);
    rv = bcm_field_qualify_DstIp(unit, ent_elk, ip_addr, ip_mask);
    if(rv) {
        printf("bcm_field_qualify_DstIp error\n");
        return rv;
    }

    rv = bcm_field_action_add(unit, ent_elk, bcmFieldActionExternalValue4Set, acl4val0, acl4val1);
    if(rv) {
        printf("bcm_field_action_add %d error\n", hitbit);
        return rv;
    }

    rv = bcm_field_entry_prio_set(unit, ent_elk, offset + hitbit);
    if(rv) {
        printf("bcm_field_entry_prio_set %d error\n", hitbit);
        return rv;
    }

    rv = bcm_field_entry_install(unit, ent_elk);
    if(rv) {
        printf("bcm_field_entry_install %d error\n", hitbit);
        return rv;
    }

    if(dc24) {
        /* ACL-5 */
        hitbit = HIT_BIT_5_QUAL;
        bcm_field_entry_create(unit, ACL_BASE_GROUP_INDEX + hitbit, &ent_elk);
        rv = bcm_field_qualify_DstIp(unit, ent_elk, ip_addr, ip_mask);
        if(rv) {
            printf("bcm_field_qualify_DstIp error\n");
            return rv;
        }

        rv = bcm_field_action_add(unit, ent_elk, bcmFieldActionExternalValue5Set, acl5val0, acl5val1);
        if(rv) {
            printf("bcm_field_action_add %d error\n", hitbit);
            return rv;
        }

        rv = bcm_field_entry_prio_set(unit, ent_elk, offset + hitbit);
        if(rv) {
            printf("bcm_field_entry_prio_set %d error\n", hitbit);
            return rv;
        }

        rv = bcm_field_entry_install(unit, ent_elk);
        if(rv) {
            printf("bcm_field_entry_install %d error\n", hitbit);
            return rv;
        }
    }

    return rv;
}

int acl_ipv4dc_24_ipv6_no_kbp_fwd_config_traffic(int unit, int out_port) {

    int i, rv = 0;
    int vlan = 100;
    int vrf = 100;
    bcm_l3_intf_t intf;
    bcm_l3_intf_t intf_host;
    bcm_l3_ingress_t ingress_intf;
    bcm_l3_egress_t egress_intf;
    bcm_l3_host_t l3host;
    bcm_if_t eg_int;

    bcm_ip6_t host_ip6 = { 0x12, 0x34, 0x56, 0x78, 0x9a, 0xbc, 0xde, 0xf0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01 };
    bcm_ip6_t mask_ip6 = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };
    int host_incr[4] = { 0, 2, 3, 4 };

    bcm_mac_t my_mac = { 0x00, 0x0a, 0x00, 0x00, 0x0b, 0x00 };
    bcm_mac_t next_hop_mac = { 0x22, 0x00, 0x00, 0x00, 0x00, 0x22 };



    bcm_l3_intf_t_init(&intf);
    sal_memcpy(intf.l3a_mac_addr, my_mac, 6);
    intf.l3a_vrf = vrf;
    intf.l3a_vid = vlan;
    rv = bcm_l3_intf_create(unit, &intf);
    if(rv) {
        printf("Error, bcm_l3_intf_create\n");
        return rv;
    }

    bcm_l3_ingress_t_init(&ingress_intf);
    ingress_intf.flags = BCM_L3_INGRESS_WITH_ID;
    ingress_intf.vrf = vrf;
    rv = bcm_l3_ingress_create(unit, &ingress_intf, &intf.l3a_intf_id);
    if(rv) {
        printf("Error, bcm_l3_ingress_create\n");
        return rv;
    }

    bcm_l3_intf_t_init(&intf_host);
    ingress_intf.flags = BCM_L3_INGRESS_WITH_ID;
    sal_memcpy(intf_host.l3a_mac_addr, my_mac, 6);
    intf_host.l3a_vrf = vrf;
    intf_host.l3a_vid = vlan + 1;
    rv = bcm_l3_intf_create(unit, &intf_host);
    if(rv) {
        printf("Error, bcm_l3_intf_create host\n");
        return rv;
    }

    bcm_l3_egress_t_init(&egress_intf);
    egress_intf.port = out_port;
    sal_memcpy(egress_intf.mac_addr, next_hop_mac, 6);
    egress_intf.intf = intf_host.l3a_intf_id;
    egress_intf.vlan = vlan;
    rv = bcm_l3_egress_create(unit, 0, &egress_intf, &eg_int);
    if(rv) {
        printf("Error, bcm_l3_egress_create\n");
        return rv;
    }

    for(i = 0; i < 4; i++) {
        bcm_l3_host_t_init(&l3host);
        l3host.l3a_flags = BCM_L3_IP6;
        sal_memcpy(l3host.l3a_ip6_addr, host_ip6, 16);
        l3host.l3a_ip6_addr[15] += host_incr[i];
        l3host.l3a_vrf = vrf;
        l3host.l3a_intf = eg_int;

        rv = bcm_l3_host_add(unit, &l3host);
        if(rv) {
            printf("bcm_l3_host_add error\n");
            return rv;
        }
    }

    return rv;
}

int acl_ipv4dc_24_ipv6_no_kbp_fwd_config_example(int unit) {

    int rv = 0;
    int presel_id;
    bcm_field_group_config_t grp;
    bcm_field_entry_t ent;
    uint64 hit_value, hit_mask;

    /* KBP config */
    rv = bcm_field_presel_create(unit, &presel_id);
    if(rv) {
        printf("bcm_field_presel_create error\n");
        return rv;
    }

    rv = bcm_field_qualify_Stage(unit, presel_id | BCM_FIELD_QUALIFY_PRESEL, bcmFieldStageExternal);
    if(rv) {
        printf("bcm_field_qualify_Stage External error\n");
        return rv;
    }

    rv = bcm_field_qualify_AppType(unit, presel_id | BCM_FIELD_QUALIFY_PRESEL, bcmFieldAppTypeIp6Ucast);
    if(rv) {
        printf("bcm_field_qualify_AppType error\n");
        return rv;
    }

    bcm_field_group_config_t_init(&grp);
    BCM_FIELD_PRESEL_ADD(grp.preselset, presel_id);
    grp.mode = bcmFieldGroupModeAuto;
    grp.flags =
            BCM_FIELD_GROUP_CREATE_WITH_MODE |
            BCM_FIELD_GROUP_CREATE_WITH_ASET |
            BCM_FIELD_GROUP_CREATE_WITH_PRESELSET |
            BCM_FIELD_GROUP_CREATE_WITH_ID;

    BCM_FIELD_QSET_INIT(grp.qset);
    BCM_FIELD_ASET_INIT(grp.aset);
    BCM_FIELD_QSET_ADD(grp.qset, bcmFieldQualifyStageExternal);

    BCM_FIELD_QSET_ADD(grp.qset, bcmFieldQualifyDstIp6);
    BCM_FIELD_ASET_ADD(grp.aset, bcmFieldActionExternalValue1Set);
    grp.group = ACL_BASE_GROUP_INDEX + HIT_BIT_1_QUAL;
    grp.priority = ACL_BASE_GROUP_PRIORITY + HIT_BIT_1_QUAL;

    rv = bcm_field_group_config_create(unit, &grp);
    if(rv) {
        printf("bcm_field_group_config_create group 1 error\n");
        return rv;
    }

    BCM_FIELD_ASET_REMOVE(grp.aset, bcmFieldActionExternalValue1Set);
    BCM_FIELD_ASET_ADD(grp.aset, bcmFieldActionExternalValue3Set);
    grp.group = ACL_BASE_GROUP_INDEX + HIT_BIT_3_QUAL;
    grp.priority = ACL_BASE_GROUP_PRIORITY + HIT_BIT_3_QUAL;

    rv = bcm_field_group_config_create(unit, &grp);
    if(rv) {
        printf("bcm_field_group_config_create group 3 error\n");
        return rv;
    }

    BCM_FIELD_ASET_REMOVE(grp.aset, bcmFieldActionExternalValue3Set);
    BCM_FIELD_ASET_ADD(grp.aset, bcmFieldActionExternalValue4Set);
    grp.group = ACL_BASE_GROUP_INDEX + HIT_BIT_4_QUAL;
    grp.priority = ACL_BASE_GROUP_PRIORITY + HIT_BIT_4_QUAL;

    rv = bcm_field_group_config_create(unit, &grp);
    if(rv) {
        printf("bcm_field_group_config_create group 4 error\n");
        return rv;
    }

    BCM_FIELD_ASET_REMOVE(grp.aset, bcmFieldActionExternalValue4Set);
    BCM_FIELD_ASET_ADD(grp.aset, bcmFieldActionExternalValue5Set);
    grp.group = ACL_BASE_GROUP_INDEX + HIT_BIT_5_QUAL;
    grp.priority = ACL_BASE_GROUP_PRIORITY + HIT_BIT_5_QUAL;

    rv = bcm_field_group_config_create(unit, &grp);
    if(rv) {
        printf("bcm_field_group_config_create group 5 error\n");
        return rv;
    }

    /* Initialize the KBP */
    bshell(unit, "kbp deinit_appl");
    bshell(unit, "kbp init_appl");


    /* FMP config */

    /* ACL-1 */
    presel_id = 0;
    rv = bcm_field_presel_create(unit, &presel_id);
    if(rv) {
        printf("bcm_field_presel_create error\n");
        return rv;
    }

    rv = bcm_field_qualify_Stage(unit, presel_id | BCM_FIELD_QUALIFY_PRESEL, bcmFieldStageIngress);
    if(rv) {
        printf("bcm_field_qualify_Stage error\n");
        return rv;
    }

    rv = bcm_field_qualify_AppType(unit, presel_id | BCM_FIELD_QUALIFY_PRESEL, bcmFieldAppTypeIp6Ucast);
    if(rv) {
        printf("bcm_field_qualify_AppType error\n");
        return rv;
    }

    bcm_field_group_config_t_init(&grp);
    BCM_FIELD_PRESEL_ADD(grp.preselset, presel_id);
    grp.mode     = bcmFieldGroupModeAuto;
    grp.flags    =
                BCM_FIELD_GROUP_CREATE_WITH_MODE |
                BCM_FIELD_GROUP_CREATE_WITH_ASET |
                BCM_FIELD_GROUP_CREATE_WITH_PRESELSET |
                BCM_FIELD_GROUP_CREATE_WITH_ID;

    BCM_FIELD_ASET_INIT(grp.aset);
    BCM_FIELD_ASET_ADD(grp.aset, bcmFieldActionDrop);

    BCM_FIELD_QSET_INIT(grp.qset);
    BCM_FIELD_QSET_ADD(grp.qset, bcmFieldQualifyStageIngress);

    BCM_FIELD_QSET_ADD(grp.qset, bcmFieldQualifyExternalHit1);
    BCM_FIELD_QSET_ADD(grp.qset, bcmFieldQualifyExternalValue1);
    grp.group    = PMF_BASE_GROUP_INDEX     + HIT_BIT_1_QUAL;
    grp.priority = PMF_BASE_GROUP_PRIORITY  + HIT_BIT_1_QUAL;

    rv = bcm_field_group_config_create(unit, &grp);
    if(rv) {
        printf("bcm_field_group_config_create error\n");
        return rv;
    }

    rv = bcm_field_entry_create(unit, grp.group, &ent);
    if(rv) {
        printf("Error in bcm_field_entry_create\n");
        return rv;
    }

    rv = bcm_field_qualify_ExternalHit1(unit, ent, 1, 1);
    if(rv) {
        printf("bcm_field_qualify_ExternalHit1 error\n");
        return rv;
    }

    COMPILER_64_ZERO(hit_value);
    COMPILER_64_ZERO(hit_mask);
    COMPILER_64_SET(hit_value, 0x00000000, 0x00f1111f);
    COMPILER_64_SET(hit_mask, 0x00000000, 0x00f1111f);
    rv = bcm_field_qualify_ExternalValue1(unit, ent, hit_value, hit_mask);
    if(rv) {
        printf("bcm_field_qualify_ExternalValue1 error\n");
        return rv;
    }

    rv = bcm_field_action_add(unit, ent, bcmFieldActionDrop, 0, 0);
    if(rv) {
        printf("Error in bcm_field_action_add\n");
        return rv;
    }

    rv = bcm_field_entry_install(unit, ent);
    if(rv) {
        printf("bcm_field_entry_install error\n");
        return rv;
    }

    /* ACL-3 */
    presel_id = 0;
    rv = bcm_field_presel_create(unit, &presel_id);
    if(rv) {
        printf("bcm_field_presel_create error\n");
        return rv;
    }

    rv = bcm_field_qualify_Stage(unit, presel_id | BCM_FIELD_QUALIFY_PRESEL, bcmFieldStageIngress);
    if(rv) {
        printf("bcm_field_qualify_Stage error\n");
        return rv;
    }

    rv = bcm_field_qualify_AppType(unit, presel_id | BCM_FIELD_QUALIFY_PRESEL, bcmFieldAppTypeIp6Ucast);
    if(rv) {
        printf("bcm_field_qualify_AppType error\n");
        return rv;
    }

    grp.group    = PMF_BASE_GROUP_INDEX     + HIT_BIT_3_QUAL;
    grp.priority = PMF_BASE_GROUP_PRIORITY  + HIT_BIT_3_QUAL;

    BCM_FIELD_QSET_REMOVE(grp.qset, bcmFieldQualifyExternalHit1);
    BCM_FIELD_QSET_REMOVE(grp.qset, bcmFieldQualifyExternalValue1);
    BCM_FIELD_QSET_ADD(grp.qset, bcmFieldQualifyExternalHit3);
    BCM_FIELD_QSET_ADD(grp.qset, bcmFieldQualifyExternalValue3);

    rv = bcm_field_group_config_create(unit, &grp);
    if(rv) {
        printf("bcm_field_group_config_create error\n");
        return rv;
    }

    rv = bcm_field_entry_create(unit, grp.group, &ent);
    if(rv) {
        printf("Error in bcm_field_entry_create\n");
        return rv;
    }

    rv = bcm_field_qualify_ExternalHit3(unit, ent, 1, 1);
    if(rv) {
        printf("bcm_field_qualify_ExternalHit1 error\n");
        return rv;
    }

    COMPILER_64_ZERO(hit_value);
    COMPILER_64_ZERO(hit_mask);
    COMPILER_64_SET(hit_value, 0x00000000, 0xf333333f);
    COMPILER_64_SET(hit_mask, 0x00000000, 0xf333333f);
    rv = bcm_field_qualify_ExternalValue3(unit, ent, hit_value, hit_mask);
    if(rv) {
        printf("bcm_field_qualify_ExternalValue1 error\n");
        return rv;
    }

    rv = bcm_field_action_add(unit, ent, bcmFieldActionDrop, 0, 0);
    if(rv) {
        printf("Error in bcm_field_action_add\n");
        return rv;
    }

    rv = bcm_field_entry_install(unit, ent);
    if(rv) {
        printf("bcm_field_entry_install error\n");
        return rv;
    }

    /* ACL-4 */
    presel_id = 0;
    rv = bcm_field_presel_create(unit, &presel_id);
    if(rv) {
        printf("bcm_field_presel_create error\n");
        return rv;
    }

    rv = bcm_field_qualify_Stage(unit, presel_id | BCM_FIELD_QUALIFY_PRESEL, bcmFieldStageIngress);
    if(rv) {
        printf("bcm_field_qualify_Stage error\n");
        return rv;
    }

    rv = bcm_field_qualify_AppType(unit, presel_id | BCM_FIELD_QUALIFY_PRESEL, bcmFieldAppTypeIp6Ucast);
    if(rv) {
        printf("bcm_field_qualify_AppType error\n");
        return rv;
    }

    grp.group    = PMF_BASE_GROUP_INDEX     + HIT_BIT_4_QUAL;
    grp.priority = PMF_BASE_GROUP_PRIORITY  + HIT_BIT_4_QUAL;

    BCM_FIELD_QSET_REMOVE(grp.qset, bcmFieldQualifyExternalHit3);
    BCM_FIELD_QSET_REMOVE(grp.qset, bcmFieldQualifyExternalValue3);
    BCM_FIELD_QSET_ADD(grp.qset, bcmFieldQualifyExternalHit4);
    BCM_FIELD_QSET_ADD(grp.qset, bcmFieldQualifyExternalValue4);

    rv = bcm_field_group_config_create(unit, &grp);
    if(rv) {
        printf("bcm_field_group_config_create error\n");
        return rv;
    }

    rv = bcm_field_entry_create(unit, grp.group, &ent);
    if(rv) {
        printf("Error in bcm_field_entry_create\n");
        return rv;
    }

    rv = bcm_field_qualify_ExternalHit4(unit, ent, 1, 1);
    if(rv) {
        printf("bcm_field_qualify_ExternalHit1 error\n");
        return rv;
    }

    COMPILER_64_ZERO(hit_value);
    COMPILER_64_ZERO(hit_mask);
    COMPILER_64_SET(hit_value, 0x00000000, 0xf444444f);
    COMPILER_64_SET(hit_mask, 0x00000000, 0xf444444f);
    rv = bcm_field_qualify_ExternalValue4(unit, ent, hit_value, hit_mask);
    if(rv) {
        printf("bcm_field_qualify_ExternalValue1 error\n");
        return rv;
    }

    rv = bcm_field_action_add(unit, ent, bcmFieldActionDrop, 0, 0);
    if(rv) {
        printf("Error in bcm_field_action_add\n");
        return rv;
    }

    rv = bcm_field_entry_install(unit, ent);
    if(rv) {
        printf("bcm_field_entry_install error\n");
        return rv;
    }

    /* ACL-5 */
    presel_id = 0;
    rv = bcm_field_presel_create(unit, &presel_id);
    if(rv) {
        printf("bcm_field_presel_create error\n");
        return rv;
    }

    rv = bcm_field_qualify_Stage(unit, presel_id | BCM_FIELD_QUALIFY_PRESEL, bcmFieldStageIngress);
    if(rv) {
        printf("bcm_field_qualify_Stage error\n");
        return rv;
    }

    rv = bcm_field_qualify_AppType(unit, presel_id | BCM_FIELD_QUALIFY_PRESEL, bcmFieldAppTypeIp6Ucast);
    if(rv) {
        printf("bcm_field_qualify_AppType error\n");
        return rv;
    }

    grp.group    = PMF_BASE_GROUP_INDEX     + HIT_BIT_5_QUAL;
    grp.priority = PMF_BASE_GROUP_PRIORITY  + HIT_BIT_5_QUAL;

    BCM_FIELD_QSET_REMOVE(grp.qset, bcmFieldQualifyExternalHit4);
    BCM_FIELD_QSET_REMOVE(grp.qset, bcmFieldQualifyExternalValue4);
    BCM_FIELD_QSET_ADD(grp.qset, bcmFieldQualifyExternalHit5);
    BCM_FIELD_QSET_ADD(grp.qset, bcmFieldQualifyExternalValue5);

    rv = bcm_field_group_config_create(unit, &grp);
    if(rv) {
        printf("bcm_field_group_config_create error\n");
        return rv;
    }

    rv = bcm_field_entry_create(unit, grp.group, &ent);
    if(rv) {
        printf("Error in bcm_field_entry_create\n");
        return rv;
    }

    rv = bcm_field_qualify_ExternalHit5(unit, ent, 1, 1);
    if(rv) {
        printf("bcm_field_qualify_ExternalHit1 error\n");
        return rv;
    }

    COMPILER_64_ZERO(hit_value);
    COMPILER_64_ZERO(hit_mask);
    COMPILER_64_SET(hit_value, 0x000000f5, 0x5555555f);
    COMPILER_64_SET(hit_mask, 0x000000f5, 0x5555555f);
    rv = bcm_field_qualify_ExternalValue5(unit, ent, hit_value, hit_mask);
    if(rv) {
        printf("bcm_field_qualify_ExternalValue1 error\n");
        return rv;
    }

    rv = bcm_field_action_add(unit, ent, bcmFieldActionDrop, 0, 0);
    if(rv) {
        printf("Error in bcm_field_action_add\n");
        return rv;
    }

    rv = bcm_field_entry_install(unit, ent);
    if(rv) {
        printf("bcm_field_entry_install error\n");
        return rv;
    }

    return rv;
}

int acl_ipv4dc_24_ipv6_no_kbp_fwd_acl_entry_add(int unit, int hitbit) {

    int rv = 0;
    int val0 = 0x00000000;
    int val1 = 0x00f1111f;
    bcm_field_action_t action = bcmFieldActionExternalValue1Set;
    bcm_ip6_t dip = { 0x12, 0x34, 0x56, 0x78, 0x9a, 0xbc, 0xde, 0xf0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01 };
    bcm_ip6_t ip_mask = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };
    bcm_field_entry_t ent;

    if(hitbit != HIT_BIT_1_QUAL && hitbit != HIT_BIT_3_QUAL && hitbit != HIT_BIT_4_QUAL && hitbit != HIT_BIT_5_QUAL) {
        printf("Invalid hitbit %d\n", hitbit);
        return -1;
    }

    switch(hitbit) {
        case HIT_BIT_1_QUAL:
            /* all defaults are set for hitbit 1 */
            break;
        case HIT_BIT_3_QUAL:
            dip[15] += 2;
            action = bcmFieldActionExternalValue3Set;
            val0 = 0x00000000;
            val1 = 0xf333333f;
            break;
        case HIT_BIT_4_QUAL:
            dip[15] += 3;
            action = bcmFieldActionExternalValue4Set;
            val0 = 0x00000000;
            val1 = 0xf444444f;
            break;
        case HIT_BIT_5_QUAL:
            dip[15] += 4;
            action = bcmFieldActionExternalValue5Set;
            val0 = 0x5555555f;
            val1 = 0xf5000000;
            break;
        default:
            printf("This shouldn't happen\n");
            return -1;
    }

    bcm_field_entry_create(unit, ACL_BASE_GROUP_INDEX + hitbit, &ent);

    rv = bcm_field_qualify_DstIp6(unit, ent, dip, ip_mask);
    if(rv) {
        printf("bcm_field_qualify_DstIp6 error, ip: ");
        ipv6_print(dip);
        printf("\n");
        return rv;
    }
    rv = bcm_field_action_add(unit, ent, action, val0, val1);
    if(rv) {
        printf("bcm_field_action_add error value1set, val0 = %d, val1 = %d\n", val0, val1);
        return rv;
    }

    rv = bcm_field_entry_prio_set(unit, ent, 10);
    if(rv) {
        printf("bcm_field_entry_prio_set error\n");
        return rv;
    }

    rv = bcm_field_entry_install(unit, ent);
    if(rv) {
        printf("bcm_field_entry_install error\n");
        return rv;
    }

    return rv;
}

int acl_ipv4dc_24_ipv6_no_kbp_fwd_acl_all_entries_add(int unit) {

    int rv = 0;

    rv = acl_ipv4dc_24_ipv6_no_kbp_fwd_acl_entry_add(unit, HIT_BIT_1_QUAL);
    if(rv) {
        printf("acl_ipv4dc_24_ipv6_no_kbp_fwd_acl_entry_add 1 failed\n");
    }
    rv = acl_ipv4dc_24_ipv6_no_kbp_fwd_acl_entry_add(unit, HIT_BIT_3_QUAL);
    if(rv) {
        printf("acl_ipv4dc_24_ipv6_no_kbp_fwd_acl_entry_add 3 failed\n");
    }
    rv = acl_ipv4dc_24_ipv6_no_kbp_fwd_acl_entry_add(unit, HIT_BIT_4_QUAL);
    if(rv) {
        printf("acl_ipv4dc_24_ipv6_no_kbp_fwd_acl_entry_add 4 failed\n");
    }
    rv = acl_ipv4dc_24_ipv6_no_kbp_fwd_acl_entry_add(unit, HIT_BIT_5_QUAL);
    if(rv) {
        printf("acl_ipv4dc_24_ipv6_no_kbp_fwd_acl_entry_add 5 failed\n");
    }

    return rv;
}

