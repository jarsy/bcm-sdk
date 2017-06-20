/*
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
 
/* $ID: cint_field_kaps.c, v 1.2 2016/08/05 skoparan$
 *
 * File: cint_field_kaps.c
 * Purpose: configuration for PMF using KAPS direct table
 *
 * Main function:
 *   - field_direct_kaps_setup
 *
 * Other functions:
 *   - field_direct_kaps_destroy
 *
*/

/* Global variables of the IDs that would be destroyed  */
int nof_entries;
int entry_id[4];

/*
 * Main function to configure a DT group in KAPS
 */
int field_direct_kaps_setup(/* in */ int unit,
                            /* in */ bcm_field_group_t group, /* defines the BB used*/
                            /* in */ int group_priority,
                            /* in */ uint32 statId,
                            /* in */ bcm_port_t in_port,
                            /* in */ uint8 is_64 /* Set for 64 bit result, unset for 32 bit result*/)
{
    bcm_field_group_t grp;
    char *proc_name;

    proc_name = "field_direct_kaps_setup";
    bcm_field_presel_set_t psset;
    int presel_id;
    bcm_field_qset_t qset;
    bcm_field_aset_t aset;
    bcm_field_entry_t ent;
    bcm_gport_t local_gport;
    bcm_gport_t snoop_gport;
    bcm_field_group_config_t grp1;
    int auxRes;
    int result = BCM_E_NONE;
    uint32 key_base[4] = {0x0120, /* row 72, zone 0 */
                          0x0121, /* row 72, zone 1 - for 32-bit only*/
                          0x0122, /* row 72, zone 2 */
                          0x0123};/* row 72, zone 3 - for 32-bit only*/

    bcm_field_data_qualifier_t data_qual;
    uint8 data1[2], mask1[2];
    nof_entries = 0;
    int zone; /* counter for the loop */

    /* Create a data qualifier to control the key */
    result = bcm_field_control_set(unit, bcmFieldControlLargeDirectLuKeyLength, 14);
    if (BCM_E_NONE != result) {
        printf("Error in bcm_field_control_set with Err %x\n", result);
        return result;
    }

    bcm_field_data_qualifier_t_init(&data_qual);
    data_qual.flags = BCM_FIELD_DATA_QUALIFIER_OFFSET_PREDEFINED | BCM_FIELD_DATA_QUALIFIER_LENGTH_BIT_RES | BCM_FIELD_DATA_QUALIFIER_OFFSET_BIT_RES;
    data_qual.qualifier = bcmFieldQualifySrcMac; 
    data_qual.length = 14;
    data_qual.offset = 0;

    result = bcm_field_data_qualifier_create(unit, &data_qual);
    if (BCM_E_NONE != result) {
        printf("Error in bcm_field_data_qualifier_create with Err %x\n", result);
        return result;
    }

    /* Create a presel entity */
	result = bcm_field_presel_create(unit, presel_id);
	if (BCM_E_NONE != result) {
		printf("Error in bcm_field_presel_create_id\n");
		auxRes = bcm_field_presel_destroy(unit, presel_id);
        return result;
    }
    result = bcm_field_qualify_Stage(unit, presel_id | BCM_FIELD_QUALIFY_PRESEL, bcmFieldStageIngress);
    if (BCM_E_NONE != result) {
        printf("Error in bcm_field_qualify_Stage\n");
        return result;
    }
    result = bcm_field_qualify_InPort(unit, presel_id | BCM_FIELD_QUALIFY_PRESEL, in_port, 0xffffffff);	
    if (BCM_E_NONE != result) {
        printf("Error in bcm_field_qualify_InPort\n");
        return result;
    }
    BCM_FIELD_PRESEL_INIT(psset);
    BCM_FIELD_PRESEL_ADD(psset, presel_id);

    /* group ID */
    grp1.flags = BCM_FIELD_GROUP_CREATE_WITH_ID;
    grp1.group = group;
    printf("%s(): Group ID = %d\n", proc_name, group);

    /* priority */
    grp1.priority = group_priority;
    printf("%s(): Group priority = %d\n", proc_name, group_priority);

    /* Qualifiers Set */
    BCM_FIELD_QSET_INIT(grp1.qset);
    BCM_FIELD_QSET_ADD(grp1.qset, bcmFieldQualifyStageIngress);
    result = bcm_field_qset_data_qualifier_add(unit, &grp1.qset, data_qual.qual_id);
    if (BCM_E_NONE != result) {
        printf("bcm_field_qset_data_qualifier_add %x\n", result);
        return result;
    }

    /* Actions Set */
    grp1.flags |= BCM_FIELD_GROUP_CREATE_WITH_ASET;
    BCM_FIELD_ASET_INIT(grp1.aset);
    BCM_FIELD_ASET_ADD(grp1.aset, bcmFieldActionStat); /* 18-bit action for Jericho*/
    if(is_64) { /* Create ASET > 32 bits */
        BCM_FIELD_ASET_ADD(grp1.aset, bcmFieldActionDrop); /* 19-bit action, total = 37*/
    }

    /* Make the field group use a direct table */
    grp1.flags |= BCM_FIELD_GROUP_CREATE_WITH_MODE;
    grp1.mode = bcmFieldGroupModeDirect;
    printf("%s(): Group mode = Direct\n", proc_name);

    /* Make the direct table to be located in KAPS, by using LARGE group*/
    grp1.flags |= BCM_FIELD_GROUP_CREATE_LARGE;
    printf("%s(): Group size = LARGE\n", proc_name);

    /* Make the field group use preselset */
    grp1.flags |= BCM_FIELD_GROUP_CREATE_WITH_PRESELSET;
    grp1.preselset = psset;

    result = bcm_field_group_config_create(unit, &grp1);
    if (BCM_E_NONE != result) {
        printf("%s(): Error in bcm_field_group_config_create\n", proc_name);
        return result;
    }
    printf("%s(): Group created.\n", proc_name);

    /* Create entries */
    for (zone = 0; zone < 4; zone = zone + 1 + is_64 /* use zones 0,1,2,3 for 32-bit and 0,2 for 64-bit*/ ) {
        result = bcm_field_entry_create(unit, grp1.group, &ent);
        if (BCM_E_NONE != result) {
            printf("%s(): Error in bcm_field_entry_create\n", proc_name);
            auxRes = bcm_field_entry_destroy(unit, ent);
            auxRes = bcm_field_group_destroy(unit, grp);
            return result;
        }
        printf("%s(): Entry %d created.\n", proc_name, ent);
        entry_id[nof_entries++] = ent; /* Save in the global variable for destroy purposes*/
        /* Fill the key and mask for the data qualifier. Last 2 bits show the zone within row. */
        data1[0] = ((key_base[zone] >> 8) % 0x100) & 0x3F; /* Bits 13 to 8 */
        data1[1] = (key_base[zone] % 0x100);        /* Bits 7 to 0 */
        mask1[0] = 0x3F;
        mask1[1] = 0xFF;
        result = bcm_field_qualify_data(unit, ent, data_qual.qual_id, &data1, &mask1, 2);
        if (BCM_E_NONE != result) {
            printf("Error in bcm_field_qualify_data\n");
            return result;
        }
        printf("%s(): Qualifier added to entry.\n", proc_name);

        BCM_GPORT_LOCAL_SET(local_gport, 15);
        result = bcm_field_action_add(unit, ent, bcmFieldActionStat, statId, local_gport);
        if (BCM_E_NONE != result) {
            printf("%s(): Error in bcm_field_action_add\n", proc_name);
            auxRes = bcm_field_entry_destroy(unit,ent);
            auxRes = bcm_field_group_destroy(unit, grp);
            return result;
        }

        if(is_64) {
            result = bcm_field_action_add(unit, ent, bcmFieldActionDrop, 0, 0);
            if (BCM_E_NONE != result) {
                printf("%s(): Error in bcm_field_action_add\n", proc_name);
                auxRes = bcm_field_entry_destroy(unit,ent);
                auxRes = bcm_field_group_destroy(unit, grp);
                return result;
            }
        }
        printf("%s(): Action(s) added to entry %d.\n", proc_name, ent);

        result = bcm_field_entry_install(unit, ent);
        if (BCM_E_NONE != result) {
            printf("%s(): Error in bcm_field_entry_install\n", proc_name);
            auxRes = bcm_field_group_destroy(unit, grp);
            return result;
        }
        printf("%s(): Entry %d installed in zone %d.\n", proc_name, ent, zone);
    }

    return result;
}

/* 
 * Main function to destroy configured entry and preselector
 * Attention! Field groups in KAPS cannot be destroyed.
 */
int field_direct_kaps_destroy (/* in */ int unit) {
    int rv = BCM_E_NONE;
    char *proc_name;
    int i;

    proc_name = "field_direct_kaps_destroy";
    for (i = 0; i < nof_entries; i++) {
        rv = bcm_field_entry_destroy(unit, entry_id[i]);
        if (rv != BCM_E_NONE) {
            printf("%s():Error in function bcm_field_entry_destroy(), returned %d (%s)\n", proc_name, rv, bcm_errmsg(rv));
            return rv;
        }
    }
    return rv;
}

