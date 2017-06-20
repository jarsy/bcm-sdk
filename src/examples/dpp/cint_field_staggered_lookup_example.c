/*
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
 
bcm_field_group_config_t grp_de, grp_cascaded, grp_cascaded2, grp_tcam;
bcm_field_entry_t ent, ent_de,  ent_cascaded,ent_cascaded2 ;
    bcm_field_extraction_action_t ext_action;
    bcm_field_extraction_field_t ext_field;
    int first_presel_id, second_presel_id ;
    bcm_field_presel_set_t pset, pset2, pset3;
    int nof_groups_per_cycle=1;
    int first_presel_id =4;
    int second_presel_id =20;
    int presel_flags=0;
int unit=0;

void staggered_example_presel_create(int unit, int first_presel_id, int second_presel_id )
    {
        print bcm_field_presel_create_stage_id(unit, bcmFieldStageIngress, first_presel_id | BCM_FIELD_QUALIFY_PRESEL_ADVANCED_MODE_STAGE_INGRESS);

        BCM_FIELD_PRESEL_INIT(pset);
        BCM_FIELD_PRESEL_ADD(pset, first_presel_id);
        print bcm_field_qualify_Stage(unit, first_presel_id | BCM_FIELD_QUALIFY_PRESEL | BCM_FIELD_QUALIFY_PRESEL_ADVANCED_MODE_STAGE_INGRESS , bcmFieldStageIngress);
        print bcm_field_qualify_IpType(unit, first_presel_id | BCM_FIELD_QUALIFY_PRESEL | BCM_FIELD_QUALIFY_PRESEL_ADVANCED_MODE_STAGE_INGRESS, bcmFieldIpTypeIpv4Any);
        print bcm_field_qualify_InPort(unit, first_presel_id | BCM_FIELD_QUALIFY_PRESEL | BCM_FIELD_QUALIFY_PRESEL_ADVANCED_MODE_STAGE_INGRESS, 13, 0xffffffff);	

        presel_flags |= BCM_FIELD_QUALIFY_PRESEL_ADVANCED_MODE_STAGE_INGRESS | BCM_FIELD_QUALIFY_PRESEL_STAGGERED;
        print bcm_field_presel_create_stage_id(unit, bcmFieldStageIngress, second_presel_id |  BCM_FIELD_QUALIFY_PRESEL_STAGGERED);

        BCM_FIELD_PRESEL_INIT(pset2);
        BCM_FIELD_PRESEL_ADD(pset2, second_presel_id | BCM_FIELD_PRESEL_STAGGERED);
        print bcm_field_qualify_PreselId(unit, second_presel_id | presel_flags , first_presel_id, -1);
        print bcm_field_qualify_StaggeredPreselProfile2(unit, second_presel_id | presel_flags , 2, 0x3);
        

    }

int staggered_example_set_program_1_set(/* in */  int unit,
                                        /* in */  int in_port) 
{
  int result;


      int i = 0;
      int group;
    bcm_pbmp_t pbmRcy;
    bcm_pbmp_t pbmPhy;
    bcm_pbmp_t pbm_mask;
    bcm_field_aset_t aset, aset_tcam;

    BCM_PBMP_CLEAR(pbm_mask);
    for(i=0; i<256; i++) {
        BCM_PBMP_PORT_ADD(pbm_mask, i);
    }
    /* Physical ports: 13-17 */
    BCM_PBMP_CLEAR(pbmPhy);
    for(i=13; i<=17; i++) {
        BCM_PBMP_PORT_ADD(pbmPhy, i);
    }

    for (group=in_port; group < in_port+ nof_groups_per_cycle ; group ++) {


       /* Create regular field group  */
      bcm_field_group_config_t_init(&grp_tcam);
      grp_tcam.group = group;
      grp_tcam.preselset = pset;
      grp_tcam.flags = BCM_FIELD_GROUP_CREATE_WITH_MODE | BCM_FIELD_GROUP_CREATE_WITH_ID | BCM_FIELD_GROUP_CREATE_WITH_PRESELSET;
      grp_tcam.mode = bcmFieldGroupModeAuto;
      grp_tcam.priority = BCM_FIELD_GROUP_PRIO_ANY;

      BCM_FIELD_QSET_INIT(grp_tcam.qset);
      BCM_FIELD_QSET_ADD(grp_tcam.qset, bcmFieldQualifyStageIngress);
      BCM_FIELD_QSET_ADD(grp_tcam.qset, bcmFieldQualifyEtherType);

      result = bcm_field_group_config_create(unit, &grp_tcam);
      if (BCM_E_NONE != result) {
          printf("Error in bcm_field_group_config_create for group %d\n", grp_tcam.group);
          return result;
      }
      
      BCM_FIELD_ASET_INIT(aset_tcam);
      BCM_FIELD_ASET_ADD(aset_tcam, bcmFieldActionStaggeredPreselProfile2Set);

      result = bcm_field_group_action_set(unit, grp_tcam.group, aset_tcam);
      if (BCM_E_NONE != result) {
          printf("Error in bcm_field_group_action_set for group %d\n", grp_cascaded.group);
          return result;
      }



      result = bcm_field_entry_create(unit, grp_tcam.group, &ent);
      if (BCM_E_NONE != result) {
          printf("Error in bcm_field_entry_create\n");
          return result;
      }

      result = bcm_field_qualify_EtherType(unit, ent, 0x800, 0xffff);
      if (BCM_E_NONE != result) {
          printf("Error in bcm_field_qualify_InPort\n");
          return result;
      }

      result = bcm_field_action_add(unit, ent, bcmFieldActionStaggeredPreselProfile2Set, 2, 0x3);
      if (BCM_E_NONE != result) {
          printf("Error in bcm_field_action_add\n");
          return result;
      }

      result = bcm_field_entry_install(unit, ent);
      if (BCM_E_NONE != result) {
          printf("Error in bcm_field_entry_install\n");
          return result;
      }


  }

  return result;

} /* compare_field_group_set_a */

int staggered_example_set_program_2_set(/* in */  int unit,
                                        /* in */  int in_port) 
{
  int result;


      int i = 0;
      int group;

    bcm_field_aset_t aset, aset_tcam;

      for (group=in_port; group < in_port+ nof_groups_per_cycle ; group ++) {


       /* Create regular field group  */
      bcm_field_group_config_t_init(&grp_tcam);
      grp_tcam.group = 20+group;
      grp_tcam.preselset = pset2;
      grp_tcam.flags = BCM_FIELD_GROUP_CREATE_WITH_MODE | BCM_FIELD_GROUP_CREATE_WITH_ID | BCM_FIELD_GROUP_CREATE_WITH_PRESELSET;
      grp_tcam.mode = bcmFieldGroupModeAuto;
      grp_tcam.priority = BCM_FIELD_GROUP_PRIO_ANY;

      BCM_FIELD_QSET_INIT(grp_tcam.qset);
      BCM_FIELD_QSET_ADD(grp_tcam.qset, bcmFieldQualifyStageIngress);
      BCM_FIELD_QSET_ADD(grp_tcam.qset, bcmFieldQualifySrcIp);

      result = bcm_field_group_config_create(unit, &grp_tcam);
      if (BCM_E_NONE != result) {
          printf("Error in bcm_field_group_config_create for group %d\n", grp_tcam.group);
          return result;
      }
      
      BCM_FIELD_ASET_INIT(aset_tcam);
      /*BCM_FIELD_ASET_ADD(aset_tcam, bcmFieldActionVrfSet);*/
      BCM_FIELD_ASET_ADD(aset_tcam, bcmFieldActionForward);

      result = bcm_field_group_action_set(unit, grp_tcam.group, aset_tcam);
      if (BCM_E_NONE != result) {
          printf("Error in bcm_field_group_action_set for group %d\n", grp_cascaded.group);
          return result;
      }



      result = bcm_field_entry_create(unit, grp_tcam.group, &ent);
      if (BCM_E_NONE != result) {
          printf("Error in bcm_field_entry_create\n");
          return result;
      }

      result = bcm_field_qualify_SrcIp(unit, ent, 0xaabbccdd, 0xffffffff);
      if (BCM_E_NONE != result) {
          printf("Error in bcm_field_qualify_InPort\n");
          return result;
      }

      result = bcm_field_action_add(unit, ent, bcmFieldActionForward, 0x7ffff, 0);
      if (BCM_E_NONE != result) {
          printf("Error in bcm_field_action_add\n");
          return result;
      }

      result = bcm_field_entry_install(unit, ent);
      if (BCM_E_NONE != result) {
          printf("Error in bcm_field_entry_install\n");
          return result;
      }
   }

  return result;

} /* compare_field_group_set_a */



int staggered_example_set(int unit)
{
  int result = BCM_E_NONE;

  /* Create LSB field group */
  staggered_example_presel_create(unit, 4, 20);

  result =  staggered_example_set_program_1_set(unit, 13);
  if (BCM_E_NONE != result) {
      printf("Error in staggered_example_set_program_1_set\n");
      return result;
  }


  result =  staggered_example_set_program_2_set(unit, 13);
  if (BCM_E_NONE != result) {
      printf("Error in staggered_example_set_program_2_set\n");
      return result;
  }

}

