/*
 * $Id: $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File: cint_jer_fc_trigger_config_example.c
 */
/* 
 * Purpose: 
 *   
 *   Example of trigger FC for jericho device.
 *
 */

enum global_vsq_src_e {
    global_vsq_src_bdb,     /* For Jericho and QAX */
    global_vsq_src_mini_db, /* For Jericho */
    global_vsq_src_full_db, /* For Jericho */
    global_vsq_src_ocb_db,  /* For Jericho and QAX */
    global_vsq_src_ocb_pdb, /* For QAX */
    global_vsq_src_p0_db,   /* For QAX */
    global_vsq_src_p1_db,   /* For QAX */
    global_vsq_src_p0_pd,   /* For QAX */
    global_vsq_src_p1_pd,   /* For QAX */
    global_vsq_src_p0_byte, /* For QAX */
    global_vsq_src_p1_byte, /* For QAX */
    global_vsq_src_hdrm_db, /* For QAX */
    global_vsq_src_hdrm_pd  /* For QAX */
};

int cint_jer_test_fc_gl_trigger_set_example(int unit, int global_vsq_src, int priority, int shaped_port)
{
    int rv = BCM_E_NONE;
    bcm_cosq_vsq_info_t vsq_inf;
    bcm_gport_t vsq_gport;
    bcm_gport_t local_gport;

    bcm_cosq_threshold_t fc_threshold;

    /*Creating vsq*/
    vsq_inf.flags = BCM_COSQ_VSQ_GL;
    rv = bcm_cosq_gport_vsq_create(unit,&vsq_inf,&vsq_gport);
    if (rv != BCM_E_NONE) {
        printf("creating vsq failed(%d) in bcm_cosq_gport_vsq_create\n", rv);
        return rv;
    }

    /*Setting GL vsq threshold*/
    bcm_cosq_threshold_t_init(&fc_threshold);   
    switch (priority) {
        case 0:
            fc_threshold.priority = BCM_COSQ_HIGH_PRIORITY;
            break;
        case 1:
            fc_threshold.priority = BCM_COSQ_LOW_PRIORITY;
            break;
        defalut:
            if (priority != BCM_COSQ_HIGH_PRIORITY && priority != BCM_COSQ_LOW_PRIORITY)
            return BCM_E_PARAM;        
    }
    fc_threshold.flags = BCM_COSQ_THRESHOLD_INGRESS | BCM_COSQ_THRESHOLD_FLOW_CONTROL|BCM_COSQ_THRESHOLD_SET;
    switch (global_vsq_src) {
        case global_vsq_src_bdb:
            fc_threshold.type = bcmCosqThresholdBufferDescriptorBuffers;
            break;
        case global_vsq_src_mini_db: 
            fc_threshold.type = bcmCosqThresholdMiniDbuffs;
            fc_threshold.flags |= BCM_COSQ_THRESHOLD_MULTICAST;
            break;
        case global_vsq_src_full_db: 
            fc_threshold.type = bcmCosqThresholdFullDbuffs;
            fc_threshold.flags |= BCM_COSQ_THRESHOLD_MULTICAST;
            break;
        case global_vsq_src_ocb_db: 
            fc_threshold.type = bcmCosqThresholdDbuffs;
            fc_threshold.flags |= BCM_COSQ_THRESHOLD_OCB;
            break;
        case global_vsq_src_ocb_pdb: 
            fc_threshold.type = bcmCosqThresholdPacketDescriptorBuffers;
            fc_threshold.flags |= BCM_COSQ_THRESHOLD_OCB;
            break;
        case global_vsq_src_p0_db: 
            fc_threshold.type = bcmCosqThresholdDbuffs;
            fc_threshold.flags |= BCM_COSQ_THRESHOLD_POOL0;
            break;
        case global_vsq_src_p1_db: 
            fc_threshold.type = bcmCosqThresholdDbuffs;
            fc_threshold.flags |= BCM_COSQ_THRESHOLD_POOL1;
            break;
        case global_vsq_src_p0_pd: 
            fc_threshold.type = bcmCosqThresholdPacketDescriptors;
            fc_threshold.flags |= BCM_COSQ_THRESHOLD_POOL0;
            break;
        case global_vsq_src_p1_pd: 
            fc_threshold.type = bcmCosqThresholdPacketDescriptors;
            fc_threshold.flags |= BCM_COSQ_THRESHOLD_POOL1;
            break;
        case global_vsq_src_p0_byte: 
            fc_threshold.type = bcmCosqThresholdBytes;
            fc_threshold.flags |= BCM_COSQ_THRESHOLD_POOL0;
            break;
        case global_vsq_src_p1_byte: 
            fc_threshold.type = bcmCosqThresholdBytes;
            fc_threshold.flags |= BCM_COSQ_THRESHOLD_POOL1;
            break;
        case global_vsq_src_hdrm_db: 
            fc_threshold.type = bcmCosqThresholdDbuffs;
            fc_threshold.flags |= BCM_COSQ_THRESHOLD_HEADROOM;
            break;
        case global_vsq_src_hdrm_pd: 
            fc_threshold.type = bcmCosqThresholdPacketDescriptors;
            fc_threshold.flags |= BCM_COSQ_THRESHOLD_HEADROOM;
            break;
        default: 
            break;
    }
    fc_threshold.value = 0x7f0000; 
    rv = bcm_cosq_gport_threshold_set(unit, vsq_gport, 0, &fc_threshold);
    if (rv != BCM_E_NONE) {
        printf("Failed(%d) to set fc set threshold in cint_jer_test_fc_gl_trigger_set_example\n", rv);
        return rv;
    }

    fc_threshold.flags &= ~(BCM_COSQ_THRESHOLD_SET);
    fc_threshold.flags |= BCM_COSQ_THRESHOLD_CLEAR;
    fc_threshold.value = 0x7f0000; 
    rv = bcm_cosq_gport_threshold_set(unit, vsq_gport, 0, &fc_threshold);
    if (rv != BCM_E_NONE) {
        printf("Failed(%d) to set fc clear threshold in cint_jer_test_fc_gl_trigger_set_example\n", rv);
        return rv;
    }

    /*Shaping local port - for testing purposes {*/
    BCM_GPORT_LOCAL_SET(local_gport,shaped_port);
    rv = bcm_cosq_gport_bandwidth_set(unit,local_gport,0,0,1000000,0);
    if (rv != BCM_E_NONE) {
        printf("Failed(%d) to shape in bcm_cosq_gport_bandwidth_set. port %d\n", rv, shaped_port);
        return rv;
    }
    /*}*/
    return rv;
}  

int cint_jer_test_fc_gl_trigger_unset_example(int unit, int global_vsq_src, int priority, int shaped_port)
{
    int rv = BCM_E_NONE;
    bcm_cosq_vsq_info_t vsq_inf;
    bcm_gport_t vsq_gport;
    bcm_gport_t local_gport;

    bcm_cosq_threshold_t fc_threshold;

    /*Creating vsq*/
    vsq_inf.flags = BCM_COSQ_VSQ_GL;
    rv = bcm_cosq_gport_vsq_create(unit,&vsq_inf,&vsq_gport);
    if (rv != BCM_E_NONE) {
        printf("creating vsq failed(%d) in bcm_cosq_gport_vsq_create\n", rv);
        return rv;
    }

    /*Setting GL vsq threshold*/
    bcm_cosq_threshold_t_init(&fc_threshold);   
    switch (priority) {
        case 0:
            fc_threshold.priority = BCM_COSQ_HIGH_PRIORITY;
            break;
        case 1:
            fc_threshold.priority = BCM_COSQ_LOW_PRIORITY;
            break;
        defalut:
            if (priority != BCM_COSQ_HIGH_PRIORITY && priority != BCM_COSQ_LOW_PRIORITY)
            return BCM_E_PARAM;        
    }
    fc_threshold.flags = BCM_COSQ_THRESHOLD_INGRESS | BCM_COSQ_THRESHOLD_FLOW_CONTROL|BCM_COSQ_THRESHOLD_SET;
    switch (global_vsq_src) {
        case global_vsq_src_bdb:
            fc_threshold.type = bcmCosqThresholdBufferDescriptorBuffers;
            break;
        case global_vsq_src_mini_db: 
            fc_threshold.type = bcmCosqThresholdMiniDbuffs;
            fc_threshold.flags |= BCM_COSQ_THRESHOLD_MULTICAST;
            break;
        case global_vsq_src_full_db: 
            fc_threshold.type = bcmCosqThresholdFullDbuffs;
            fc_threshold.flags |= BCM_COSQ_THRESHOLD_MULTICAST;
            break;
        case global_vsq_src_ocb_db: 
            fc_threshold.type = bcmCosqThresholdDbuffs;
            fc_threshold.flags |= BCM_COSQ_THRESHOLD_OCB;
            break;
        case global_vsq_src_ocb_pdb: 
            fc_threshold.type = bcmCosqThresholdPacketDescriptorBuffers;
            fc_threshold.flags |= BCM_COSQ_THRESHOLD_OCB;
            break;
        case global_vsq_src_p0_db: 
            fc_threshold.type = bcmCosqThresholdDbuffs;
            fc_threshold.flags |= BCM_COSQ_THRESHOLD_POOL0;
            break;
        case global_vsq_src_p1_db: 
            fc_threshold.type = bcmCosqThresholdDbuffs;
            fc_threshold.flags |= BCM_COSQ_THRESHOLD_POOL1;
            break;
        case global_vsq_src_p0_pd: 
            fc_threshold.type = bcmCosqThresholdPacketDescriptors;
            fc_threshold.flags |= BCM_COSQ_THRESHOLD_POOL0;
            break;
        case global_vsq_src_p1_pd: 
            fc_threshold.type = bcmCosqThresholdPacketDescriptors;
            fc_threshold.flags |= BCM_COSQ_THRESHOLD_POOL1;
            break;
        case global_vsq_src_p0_byte: 
            fc_threshold.type = bcmCosqThresholdBytes;
            fc_threshold.flags |= BCM_COSQ_THRESHOLD_POOL0;
            break;
        case global_vsq_src_p1_byte: 
            fc_threshold.type = bcmCosqThresholdBytes;
            fc_threshold.flags |= BCM_COSQ_THRESHOLD_POOL1;
            break;
        case global_vsq_src_hdrm_db: 
            fc_threshold.type = bcmCosqThresholdDbuffs;
            fc_threshold.flags |= BCM_COSQ_THRESHOLD_HEADROOM;
            break;
        case global_vsq_src_hdrm_pd: 
            fc_threshold.type = bcmCosqThresholdPacketDescriptors;
            fc_threshold.flags |= BCM_COSQ_THRESHOLD_HEADROOM;
            break;
        default: 
            break;
    }
    fc_threshold.value = 0; 
    rv = bcm_cosq_gport_threshold_set(unit, vsq_gport, 0, &fc_threshold);
    if (rv != BCM_E_NONE) {
        printf("Failed(%d) to set fc set threshold in cint_jer_test_fc_gl_trigger_set_example\n", rv);
        return rv;
    }

    fc_threshold.flags &= ~(BCM_COSQ_THRESHOLD_SET);
    fc_threshold.flags |= BCM_COSQ_THRESHOLD_CLEAR;
    fc_threshold.value = 0; 
    rv = bcm_cosq_gport_threshold_set(unit, vsq_gport, 0, &fc_threshold);
    if (rv != BCM_E_NONE) {
        printf("Failed(%d) to set fc set threshold in cint_jer_test_fc_gl_trigger_set_example\n", rv);
        return rv;
    }

    /*Shaping local port - for testing purposes {*/
    BCM_GPORT_LOCAL_SET(local_gport,shaped_port);
    rv = bcm_cosq_gport_bandwidth_set(unit,local_gport,0,0,10000000,0);
    if (rv != BCM_E_NONE) {
        printf("Failed(%d) to shape in cint_arad_fc_oob_llfc_gen_config_test_example. port %d\n", rv, shaped_port);
        return rv;
    }
    /*}*/
    return rv;
}

/*
vsq_flag :  Designate the VSQ Group A~C
      BCM_COSQ_VSQ_CT
      BCM_COSQ_VSQ_CTTC
      BCM_COSQ_VSQ_CTCC
*/
int cint_jer_test_fc_vsq_trigger_set_example(int unit, uint32 vsq_flag, int shaped_port, bcm_gport_t *vsq_gport)
{
    int rv = BCM_E_NONE;
    bcm_cosq_vsq_info_t vsq_info;
    bcm_gport_t local_gport;
    bcm_cosq_pfc_config_t fc_threshold;

    BCM_GPORT_LOCAL_SET(local_gport, shaped_port);
    /* limit local port speed as 1G - for testing purposes */
    rv = bcm_cosq_gport_bandwidth_set(unit, local_gport, 0, 0, 1000000, 0);
    if (rv != BCM_E_NONE) {
        printf("Failed(%d) to shape in bcm_cosq_gport_bandwidth_set. port= %d\n", rv, shaped_port);
        return rv;
    }

    rv = bcm_fabric_control_set(0, bcmFabricVsqCategory, bcmFabricVsqCatagoryMode2);
    if (rv != BCM_E_NONE) {
        printf("Set bcmFabricVsqCatagoryMode2 failed(%d) in cint_jer_test_fc_vsq_trigger_set_example\n", rv);
        return rv;
    }

    /* Creating VSQ */
    vsq_info.flags = vsq_flag;
    vsq_info.ct_id = 2;  /* Catagory 2 for general VOQ */
    vsq_info.core_id = 0;
    vsq_info.src_port = local_gport;
    rv = bcm_cosq_gport_vsq_create(unit, &vsq_info, vsq_gport);
    if (rv != BCM_E_NONE) {
        printf("creating vsq failed(%d) in bcm_cosq_gport_vsq_create\n", rv);
        return rv;
    }

    /*Setting VSQ threshold*/
    sal_memset(&fc_threshold, 0, sizeof(fc_threshold));
    fc_threshold.xoff_threshold_bd = 100;
    fc_threshold.xon_threshold_bd = 200; 
    rv = bcm_cosq_pfc_config_set(unit, *vsq_gport, 0, 0, &fc_threshold); 
    if (rv != BCM_E_NONE) {
        printf("Failed(%d) to set fc set VSQ threshold in cint_jer_test_fc_vsq_trigger_set_example\n", rv);
        return rv;
    }

    return rv;
}  

int cint_jer_test_fc_vsq_trigger_unset_example(int unit, uint32 vsq_flag, int shaped_port, bcm_gport_t *vsq_gport)
{
    int rv = BCM_E_NONE;
    bcm_cosq_vsq_info_t vsq_info;
    bcm_gport_t local_gport;
    bcm_cosq_pfc_config_t fc_threshold;

    BCM_GPORT_LOCAL_SET(local_gport, shaped_port);
    /* Recovering local port speed as 10G */
    rv = bcm_cosq_gport_bandwidth_set(unit, local_gport, 0, 0, 10000000, 0);
    if (rv != BCM_E_NONE) {
        printf("Failed(%d) to shape in bcm_cosq_gport_bandwidth_set. port= %d\n", rv, shaped_port);
        return rv;
    }

    rv = bcm_fabric_control_set(0, bcmFabricVsqCategory, bcmFabricVsqCatagoryMode2);
    if (rv != BCM_E_NONE) {
        printf("Set bcmFabricVsqCatagoryMode2 failed(%d) in cint_jer_test_fc_vsq_trigger_unset_example\n", rv);
        return rv;
    }

    /* Creating VSQ */
    vsq_info.flags = vsq_flag;
    vsq_info.ct_id = 2;  /* Catagory 2 for general VOQ */
    vsq_info.core_id = 0;
    vsq_info.src_port = local_gport;
    rv = bcm_cosq_gport_vsq_create(unit, &vsq_info, vsq_gport);
    if (rv != BCM_E_NONE) {
        printf("creating vsq failed(%d) in bcm_cosq_gport_vsq_create\n", rv);
        return rv;
    }

    /*Setting VSQ threshold*/
    sal_memset(&fc_threshold, 0, sizeof(fc_threshold));
    fc_threshold.xoff_threshold_bd = 0;
    fc_threshold.xon_threshold_bd = 0; 
    rv = bcm_cosq_pfc_config_set(unit, *vsq_gport, 0, 0, &fc_threshold); 
    if (rv != BCM_E_NONE) {
        printf("Failed(%d) to set fc unset VSQ threshold in cint_jer_test_fc_vsq_trigger_unset_example\n", rv);
        return rv;
    }

    return rv;
}  

