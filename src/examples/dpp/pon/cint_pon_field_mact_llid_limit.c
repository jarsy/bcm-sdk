/* $Id: cint_pon_field_mact_llid_limit.c,v 1.1 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/

/* Use first cycle's action to lookup in second cycle (cascaded) 
 * Two databases are created - the first is a regolar TCAM and the second is a Direct Table DB.
 * When a packet arrives, its tunnel ID (first 10 bits) is used to address the action table (direct table - second database).
 * The action holds a part of the key to be searched in the second cycle (in the TCAM - first database), 
 * along with other important firlds. 
 * In this CINT we create some entries for the Ditect Table database in order to allocate some bank for it to use.
 * In practice, these entries are not needed at this point, however without it there would be no bank.
 *
 * To Activate Above Settings Run:
 *      BCM> cint examples/dpp/pon/cint_pon_field_mact_llid_limit.c
 *      BCM> cint
 *      cint> pon_mact_llid_limit(unit, pon_port1, pon_port2);
 */
bcm_field_group_config_t grp2;
int pon_mact_llid_limit(int unit, int pon_port1, int pon_port2) 
{
    bcm_field_group_config_t grp2;
    bcm_field_aset_t aset2;
    bcm_field_entry_t ent[16];
    bcm_field_entry_t ent_drop[16];
    int result;
    int auxRes;
    int index;
    int group_priority2 = 6;
    bcm_field_data_qualifier_t dqsma;
    bcm_field_data_qualifier_t dqsma_dt;
    uint8 dqMask[2], dqData[2];

    bcm_field_group_config_t_init(&grp2);

    grp2.group = -1;
 
    for (index = 0; index < 16; index++)
    {
        ent[index] = -1;
        ent_drop[index] = -1;
    }
 
    /*
     *  Set the cascaded key length to 16 bits
     */
    result = bcm_field_control_set(unit,
                                   bcmFieldControlCascadedKeyWidth,
                                   16 /* bits in cascaded key */);
    if (BCM_E_NONE != result)
    {
        printf("Error, bcm_field_control_set result:%d\n", result);
        return result;
    }

    /* Create a Data qualifier for 16b including the Tunnel Id */
    bcm_field_data_qualifier_t_init(&dqsma); 
    dqsma.offset_base = bcmFieldDataOffsetBasePacketStart;
    dqsma.offset = 2; /* 2 bytes to skip */
    dqsma.length = 2; /* 16 bits */
    result = bcm_field_data_qualifier_create(unit, &dqsma);
    if (BCM_E_NONE != result)
    {
        printf("Error, bcm_field_data_qualifier_create result:%d\n", result);
        return result;
    }
    /*
     *  On PetraB / Arad, the second group in a cascade must be created first.
     *  This specific requirement may be masked in the future, but for right
     *  now, it is obligatory even at the BCM layer.
     */
    BCM_FIELD_QSET_INIT(grp2.qset);
    BCM_FIELD_QSET_ADD(grp2.qset, bcmFieldQualifyCascadedKeyValue);
    BCM_FIELD_QSET_ADD(grp2.qset, bcmFieldQualifyInPort);
    BCM_FIELD_QSET_ADD(grp2.qset, bcmFieldQualifyL2SrcHit);
    BCM_FIELD_QSET_ADD(grp2.qset, bcmFieldQualifyStageIngress);
    result = bcm_field_qset_data_qualifier_add(unit,
                                               &(grp2.qset),
                                               dqsma.qual_id);
    grp2.priority = group_priority2;
    if (BCM_E_NONE != result)
    {
        printf("Error, bcm_field_qset_data_qualifier_add result:%d\n", result);
        return result;
    }

    BCM_FIELD_ASET_INIT(aset2);
    BCM_FIELD_ASET_ADD(aset2, bcmFieldActionDrop);
    BCM_FIELD_ASET_ADD(aset2, bcmFieldActionDoNotLearn);
    result = bcm_field_group_config_create(unit, &grp2);
    if (BCM_E_NONE != result)
    {
        printf("Error, bcm_field_group_config_create result:%d\n", result);
        return result;
    }

    result = bcm_field_group_action_set(unit, grp2.group, aset2);
    if (BCM_E_NONE != result)
    {
        printf("Error, bcm_field_group_action_set result:%d\n", result);
        return result;
    }


    /*
     *  Now create the initial group 
     */
    bcm_field_data_qualifier_t_init(&dqsma_dt); 
    dqsma_dt.flags = BCM_FIELD_DATA_QUALIFIER_LENGTH_BIT_RES|BCM_FIELD_DATA_QUALIFIER_OFFSET_PREDEFINED;
    dqsma_dt.offset = 0; 
    dqsma_dt.length = 10; 
    dqsma_dt.qualifier = bcmFieldQualifyTunnelId; 
    result = bcm_field_data_qualifier_create(unit, &dqsma_dt);
    if (BCM_E_NONE != result)
    {
        printf("Error, bcm_field_data_qualifier_create result:%d\n", result);
        return result;
    }
	
    /* Add entries to the second group: when to drop */
    for (index = (pon_port1*2); index < ((pon_port2+1)*2); index++)
    {
        printf("index:%d\n", index);
	    result = bcm_field_entry_create(unit, grp2.group, &(ent[index]));
        if (BCM_E_NONE != result)
        {
            printf("Error, bcm_field_entry_create result:%d index:%d\n", result, index);
            return result;
        }

        /* Bit = index is up */
        result = bcm_field_qualify_CascadedKeyValue(unit, ent[index], (1 << index), (1 << index));
        if (BCM_E_NONE != result)
        {
            printf("Error, bcm_field_qualify_CascadedKeyValue result:%d index:%d\n", result, index);
            return result;
        }

        /* InPort = index >> 1 for the 3 first bits */
        result = bcm_field_qualify_InPort(unit, ent[index], (index >> 1), (~0));
        if (BCM_E_NONE != result)
        {
            printf("Error, bcm_field_qualify_InPort result:%d index:%d\n", result, index);
            return result;
        }

        /* SA lookup not found to drop */
        result = bcm_field_qualify_L2SrcHit(unit, ent[index], 0, 0x1);
        if (BCM_E_NONE != result)
        {
            printf("Error, bcm_field_qualify_L2SrcHit result:%d index:%d\n", result, index);
            return result;
        }

        /* Bit 10 of Tunnel Id up if needed */
        dqData[1] = 0x00;
        dqData[0] = 0x04 * (index % 2);
        dqMask[1] = 0x00;
        dqMask[0] = 0x04;
        result = bcm_field_qualify_data(unit,
                                        ent[index],
                                        dqsma.qual_id,
                                        &(dqData[0]),
                                        &(dqMask[0]),
                                        2 /* len here always in bytes */);
        if (BCM_E_NONE != result)
        {
            printf("Error, bcm_field_qualify_data result:%d index:%d\n", result, index);
            return result;
        }

        result = bcm_field_action_add(unit, ent[index], bcmFieldActionDrop, 0, 0);
        if (BCM_E_NONE != result)
        {
            printf("Error, bcm_field_action_add - bcmFieldActionDrop result:%d index:%d\n", result, index);
            return result;
        }

        result = bcm_field_action_add(unit, ent[index], bcmFieldActionDoNotLearn, 0, 0);
        if (BCM_E_NONE != result)
        {
            printf("Error, bcm_field_action_add - bcmFieldActionDoNotLearn result:%d index:%d\n", result, index);
            return result;
        }
    }

    /* Drop until learnt - packet are dropped until the SA is learnt at PON port */
    /* For PON port 1 */
    index = 0;
    result = bcm_field_entry_create(unit, grp2.group, &(ent_drop[index]));
    if (BCM_E_NONE != result)
    {
        printf("Error, bcm_field_entry_create result:%d index:%d\n", result, index);
        return result;
    }

    result = bcm_field_qualify_L2SrcHit(unit, ent_drop[index], 0, 0x1);
    if (BCM_E_NONE != result)
    {
        printf("Error, bcm_field_qualify_L2SrcHit result:%d index:%d\n", result, index);
        return result;
    }

    result = bcm_field_qualify_InPort(unit, ent_drop[index], pon_port1, (~0));
    if (BCM_E_NONE != result)
    {
        printf("Error, bcm_field_qualify_InPort result:%d index:%d\n", result, index);
        return result;
    }

    result = bcm_field_action_add(unit, ent_drop[index], bcmFieldActionDrop, 0, 0);
    if (BCM_E_NONE != result)
    {
        printf("Error, bcm_field_action_add result:%d index:%d\n", result, index);
        return result;
    }

    /* For PON port 2 */
    index++;
    result = bcm_field_entry_create(unit, grp2.group, &(ent_drop[index]));
    if (BCM_E_NONE != result)
    {
        printf("Error, bcm_field_entry_create result:%d index:%d\n", result, index);
        return result;
    }

    result = bcm_field_qualify_L2SrcHit(unit, ent_drop[index], 0, 0x1);
    if (BCM_E_NONE != result)
    {
        printf("Error, bcm_field_qualify_L2SrcHit result:%d index:%d\n", result, index);
        return result;
    }

    result = bcm_field_qualify_InPort(unit, ent_drop[index], pon_port2, (~0));
    if (BCM_E_NONE != result)
    {
        printf("Error, bcm_field_qualify_InPort result:%d index:%d\n", result, index);
        return result;
    }

    result = bcm_field_action_add(unit, ent_drop[index], bcmFieldActionDrop, 0, 0);
    if (BCM_E_NONE != result)
    {
        printf("Error, bcm_field_action_add result:%d index:%d\n", result, index);
        return result;
    }
 
    result = bcm_field_group_install(unit, grp2.group);
    if (BCM_E_NONE != result)
    {
        printf("Error, bcm_field_group_install result:%d\n", result);
        return result;
    }

    return result;
}


