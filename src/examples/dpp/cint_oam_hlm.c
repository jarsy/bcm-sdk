/*
 *
 * $Id: cint_oam_hlm.c,v 1.0 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 * 
 * File: cint_oam_hlm.c
 * Purpose: Example of using OAM Hierarchical loss measurement.
 *
 * Usage:
 * 
cd ../../../../src/examples/dpp
cint utility/cint_utils_global.c
cint utility/cint_utils_multicast.c
cint utility/cint_utils_mpls_port.c
cint utility/cint_utils_oam.c
cint cint_port_tpid.c
cint cint_advanced_vlan_translation_mode.c
cint cint_l2_mact.c
cint cint_vswitch_metro_mp.c
cint utility/cint_multi_device_utils.c
cint cint_queue_tests.c
cint cint_oam.c 
cint cint_oam_hlm.c 
cint
int unit = 0;
int port_1 = 13, port_2 = 14;
print oam_hierarchical_lm_example(unit,port_1,port_2);
 *
 * This CINT creates two vlan ports and cross-connect them.
 * 2 Down MMEPs of level 1 and 3 will be configured on P1
 * 2 Up MMEPs of level 4 and 6 will be configured on P2
 *
 *    Down MEPs                           Up MEPs
 *       (1)    +----+            +----+    (4)
 *       (3)    | P1 |------------| P2 |    (6)
 *              +----+            +----+
 *
 * Two counter engines are configured. MEPs on the same LIF will have
 * counter pointers from a differenet counter engine.
 */
  
int hierarchical_lm_is_acc = 0;

/**
 * Cint that demonstrate hierarchical loss measurement by MD-Level on
 * downmeps and upmeps
 *
 * @param unit
 * @param port1
 * @param port2
 *
 * @return int
 */
int oam_hierarchical_lm_example(int unit, int port1, int port2) {
    bcm_error_t rv = BCM_E_NONE;
    bcm_vlan_port_t vp1, vp2;
    bcm_oam_endpoint_info_t mep_1, mep_3, mep_4, mep_6;
    bcm_oam_group_info_t group_info_short_ma;
    uint8 short_name[BCM_OAM_GROUP_NAME_LENGTH] = { 1, 3, 2, 0xab, 0xcd };
    int down_mdl_1 = 3;
    int down_mdl_2 = 1;
    int up_mdl_1 = 6;
    int up_mdl_2 = 4;
    bcm_vswitch_cross_connect_t cross_connect;

    /* Set port classification ID */
    rv = bcm_port_class_set(unit, port1, bcmPortClassId, port1);
    if (rv != BCM_E_NONE) {
        printf("(%s) \n", bcm_errmsg(rv));
        return rv;
    }
    /* Set port classification ID */
    rv = bcm_port_class_set(unit, port2, bcmPortClassId, port2);
    if (rv != BCM_E_NONE) {
        printf("(%s) \n", bcm_errmsg(rv));
        return rv;
    }

    /* This is meant for QAX and above only  */
    rv = get_device_type(unit, &device_type);
    BCM_IF_ERROR_RETURN(rv);
    if (device_type < device_type_qax) {
        return BCM_E_UNAVAIL;
    }

    /* Allocate counter engines and counters */
    rv = hierarchical_lm_counters_setup(unit, 2, 2, counter_engines, counter_base_ids, BCM_STAT_COUNTER_MULTIPLE_SOURCES_PER_ENGINE);
    BCM_IF_ERROR_RETURN(rv);

    printf("Allocated counters:\n");
    print counter_base_ids;

    vp1.criteria = BCM_VLAN_PORT_MATCH_PORT_VLAN;
    vp1.port = port1;
    vp1.match_vlan = 0x10;
    vp1.egress_vlan = 0x10;
    rv = bcm_vlan_port_create(unit, &vp1);
    BCM_IF_ERROR_RETURN(rv);

    vp2.criteria = BCM_VLAN_PORT_MATCH_PORT_VLAN;
    vp2.port = port2;
    vp2.match_vlan = 0x20;
    vp2.egress_vlan = 0x20;
    rv = bcm_vlan_port_create(unit, &vp2);
    BCM_IF_ERROR_RETURN(rv);

    bcm_vswitch_cross_connect_t_init(&cross_connect);

    cross_connect.port1 = vp1.vlan_port_id;
    gport1 = vp1.vlan_port_id;
    cross_connect.port2 = vp2.vlan_port_id;
    gport2 = vp2.vlan_port_id;

    rv = bcm_vswitch_cross_connect_add(unit, &cross_connect);
    BCM_IF_ERROR_RETURN(rv);

    bcm_oam_group_info_t_init(&group_info_short_ma);
    sal_memcpy(group_info_short_ma.name, short_name, BCM_OAM_GROUP_NAME_LENGTH);
    rv = bcm_oam_group_create(unit, &group_info_short_ma);
    BCM_IF_ERROR_RETURN(rv);

    if (!hierarchical_lm_is_acc) {
        /*Adding non acc down MEP*/
        /*TX*/
        bcm_oam_endpoint_info_t_init(&mep_1);
        mep_1.type = bcmOAMEndpointTypeEthernet;
        mep_1.group = group_info_short_ma.id;
        mep_1.level = down_mdl_1;

        /*RX*/
    mep_1.gport = gport1;
        sal_memcpy(mep_1.dst_mac_address, mac_mep_1, 6);
        mep_1.lm_counter_base_id = counter_base_ids[0];
        mep_1.timestamp_format = get_oam_timestamp_format(unit);
    } else {
        /*
         * Adding acc up MEP
         */

        /*TX*/
        bcm_oam_endpoint_info_t_init(&mep_1);
        mep_1.type = bcmOAMEndpointTypeEthernet;
        mep_1.group =  group_info_short_ma.id;
        mep_1.level = down_mdl_1;
        BCM_GPORT_SYSTEM_PORT_ID_SET(mep_1.tx_gport, port_1);
        mep_1.name = 123;     
        mep_1.ccm_period = BCM_OAM_ENDPOINT_CCM_PERIOD_100MS;
        mep_1.opcode_flags |= BCM_OAM_OPCODE_CCM_IN_HW;
        mep_1.vlan = 0x10;
        mep_1.vpn = 0x20; /* Add VSI in order to support HLM down mep injection counting */
        mep_1.pkt_pri = mep_acc_info.pkt_pri = 0 + (2<<1); /* dei(1bit) + (pcp(3bit) << 1)*/
        mep_1.outer_tpid = 0x8100;
        mep_1.inner_vlan = 0;
        mep_1.inner_pkt_pri = 0;     
        mep_1.inner_tpid = 0;
        mep_1.int_pri = 3 + (1<<2);
        mep_1.sampling_ratio = sampling_ratio;
        /* Take RDI only from scanner*/
        mep_1.flags2 =  BCM_OAM_ENDPOINT_FLAGS2_RDI_FROM_RX_DISABLE;
        mep_1.timestamp_format = get_oam_timestamp_format(unit);
        /* The MAC address that the CCM packets are sent with*/
        sal_memcpy(mep_1.src_mac_address, src_mac_mep_3, 6);

        /*RX*/
        mep_1.gport = vp1.vlan_port_id;
        sal_memcpy(mep_1.dst_mac_address, mac_mep_1, 6);
        mep_1.lm_counter_base_id = counter_base_ids[0];
    }

    printf("bcm_oam_endpoint_create mep_1\n");
    rv = bcm_oam_endpoint_create(unit, &mep_1);
    BCM_IF_ERROR_RETURN(rv);
    printf("created MEP with id %d\n", mep_1.id);

    ep1.gport = mep_1.gport;
    ep1.id = mep_1.id;

    /*
     * Adding another non acc down MEP
     */

    /*TX*/
    bcm_oam_endpoint_info_t_init(&mep_3);
    mep_3.type = bcmOAMEndpointTypeEthernet;
    mep_3.group = group_info_short_ma.id;
    mep_3.level = down_mdl_2;

    /*RX*/
    mep_3.gport = gport1;
    sal_memcpy(mep_3.dst_mac_address, mac_mep_1, 6);
    mep_3.lm_counter_base_id = counter_base_ids[2];
    mep_3.timestamp_format = get_oam_timestamp_format(unit);

    printf("bcm_oam_endpoint_create mep_3\n");
    rv = bcm_oam_endpoint_create(unit, &mep_3);
    BCM_IF_ERROR_RETURN(rv);
    printf("created MEP with id %d\n", mep_3.id);

    ep2.gport = mep_3.gport;
    ep2.id = mep_3.id;

     if (!hierarchical_lm_is_acc) {
        /*Adding non acc up MEP*/

        /*TX*/
        bcm_oam_endpoint_info_t_init(&mep_4);
        mep_4.type = bcmOAMEndpointTypeEthernet;
        mep_4.group = group_info_short_ma.id;
        mep_4.level = up_mdl_1;
        mep_4.tx_gport = BCM_GPORT_INVALID;
        mep_4.flags = BCM_OAM_ENDPOINT_UP_FACING;

        /*RX*/
    mep_4.gport = gport2;
        sal_memcpy(mep_4.dst_mac_address, mac_mep_2, 6);
        mep_4.lm_counter_base_id = counter_base_ids[1];
        mep_4.timestamp_format = get_oam_timestamp_format(unit);

    } else { /*Adding acc up MEP*/
        /*TX*/
        bcm_oam_endpoint_info_t_init(&mep_4);
        mep_4.type = bcmOAMEndpointTypeEthernet;
        mep_4.group =  group_info_short_ma.id;
        mep_4.ccm_period = BCM_OAM_ENDPOINT_CCM_PERIOD_100MS;
        mep_4.level = up_mdl_1;
        mep_4.tx_gport = BCM_GPORT_INVALID; /*Up MEP requires gport invalid.*/
        mep_4.name = 123;     
        mep_4.flags |= BCM_OAM_ENDPOINT_UP_FACING;
        mep_4.opcode_flags |= BCM_OAM_OPCODE_CCM_IN_HW;

        mep_4.vlan = 0x20;
        mep_4.pkt_pri = mep_acc_info.pkt_pri = 0 + (2<<1); /* dei(1bit) + (pcp(3bit) << 1)*/
        mep_4.outer_tpid = 0x8100;
        mep_4.inner_vlan = 0;
        mep_4.inner_pkt_pri = 0;     
        mep_4.inner_tpid = 0;
        mep_4.int_pri = 3 + (1<<2);
        mep_4.sampling_ratio = sampling_ratio;
        /* Take RDI only from scanner*/
        mep_4.flags2 =  BCM_OAM_ENDPOINT_FLAGS2_RDI_FROM_RX_DISABLE;
        mep_4.timestamp_format = get_oam_timestamp_format(unit);
        /* The MAC address that the CCM packets are sent with*/
        sal_memcpy(mep_4.src_mac_address, src_mac_mep_2, 6);

        /*RX*/
        mep_4.gport = vp2.vlan_port_id;
        sal_memcpy(mep_4.dst_mac_address, mac_mep_2, 6);
        mep_4.lm_counter_base_id = counter_base_ids[1];
    }


    printf("bcm_oam_endpoint_create mep_4\n");
    rv = bcm_oam_endpoint_create(unit, &mep_4);
    BCM_IF_ERROR_RETURN(rv);
    printf("created MEP with id %d\n", mep_4.id);

    ep3.gport = mep_4.gport;
    ep3.id = mep_4.id;

    /*
     * Adding another non acc up MEP
     */

    /*TX*/
    bcm_oam_endpoint_info_t_init(&mep_6);
    mep_6.type = bcmOAMEndpointTypeEthernet;
    mep_6.group = group_info_short_ma.id;
    mep_6.level = up_mdl_2;
    mep_6.tx_gport = BCM_GPORT_INVALID;
    mep_6.flags = BCM_OAM_ENDPOINT_UP_FACING;

    /*RX*/
    mep_6.gport = gport2;
    sal_memcpy(mep_6.dst_mac_address, mac_mep_2, 6);
    mep_6.lm_counter_base_id = counter_base_ids[3];
    mep_6.timestamp_format = get_oam_timestamp_format(unit);

    printf("bcm_oam_endpoint_create mep_6\n");
    rv = bcm_oam_endpoint_create(unit, &mep_6);
    BCM_IF_ERROR_RETURN(rv);
    printf("created MEP with id %d\n", mep_6.id);

    ep4.gport = mep_6.gport;
    ep4.id = mep_6.id;

    return BCM_E_NONE;
}

/**
 * Cint that demonstrate hierarchical loss measurement by LIF on
 * downmeps
 *
 * @param unit
 * @param port1
 * @param port2
 *
 * @return int
 */
int oam_hierarchical_lif_lm_example(int unit, int in_sysport, int out_sysport) {

    bcm_error_t rv = BCM_E_NONE;
    bcm_gport_t gports[3];
    int term_labels[3] = {0x100, 0x200, 0x300};
    bcm_oam_endpoint_info_t meps[3];
    bcm_oam_endpoint_info_t def_mep;
    bcm_oam_group_info_t group_info_short_ma;
    uint8 short_name[BCM_OAM_GROUP_NAME_LENGTH] = { 1, 3, 2, 0xab, 0xcd };
    int mdl = 4;
    int counters_to_use[3] = {0,2,1};
    int i;

    /* Creates MPLS switch with:
     * My-MAC :0x11
     * Next-hop mac: 0x22
     * in-label: 5000
     * out-label: 8000
     * in-vlan: 100
     * out-vlan: 200
     */
    mpls_lsr_init(in_sysport, out_sysport, 0x11, 0x22, 5000, 8000, 100,200,0);
    rv = mpls_lsr_config(&unit, 1/*nof-units*/, 0/*extend example*/);
    BCM_IF_ERROR_RETURN(rv);

    /* Setup counters */
    rv = hierarchical_lm_counters_setup(unit, 2, 2, counter_engines, counter_base_ids, BCM_STAT_COUNTER_MULTIPLE_SOURCES_PER_ENGINE);
    BCM_IF_ERROR_RETURN(rv);
    printf("Allocated counters:\n");
    print counter_base_ids;

    /* Create OAM group */
    bcm_oam_group_info_t_init(&group_info_short_ma);
    sal_memcpy(group_info_short_ma.name, short_name, BCM_OAM_GROUP_NAME_LENGTH);
    rv = bcm_oam_group_create(unit, &group_info_short_ma);
    BCM_IF_ERROR_RETURN(rv);

    /* Add inlifs and MEPs   */
    for (i = 0; i < 3; i++) {
        /* InLIF   */
        rv = mpls_add_term_entry(unit, term_labels[i], 0);
        BCM_IF_ERROR_RETURN(rv);
        /*
         * print ingress_tunnel_id_indexed[0];
         * print ingress_tunnel_id_indexed[1];
         * print ingress_tunnel_id_indexed[2];
         */
        gports[i] = ingress_tunnel_id_indexed[i];
        printf("Added termination of label 0x%05x -- GPort: 0x%x\n",term_labels[i], gports[i]);

        /* MEP */
        bcm_oam_endpoint_info_t_init(&meps[i]);
        meps[i].type = bcmOAMEndpointTypeBHHMPLS;
        meps[i].group = group_info_short_ma.id;
        meps[i].level = mdl;
        meps[i].gport = gports[i];
        meps[i].lm_counter_base_id = counter_base_ids[counters_to_use[i]];
        meps[i].timestamp_format = bcmOAMTimestampFormatIEEE1588v1;

        printf("bcm_oam_endpoint_create meps[%d]\n", i);
        rv = bcm_oam_endpoint_create(unit, &meps[i]);
        BCM_IF_ERROR_RETURN(rv);
        printf("created MEP with id 0x%04x\n\tcounter: 0x%04x\n", meps[i].id, meps[i].lm_counter_base_id);
    }

    /* Add default endpoint on 3rd LIF (Used for trapping OAM on more than 2 lifs hierarchy). */
    bcm_oam_endpoint_info_t_init(&def_mep);
    def_mep.level = mdl;
    def_mep.gport = gports[2];
    def_mep.lm_counter_base_id = counter_base_ids[3];
    def_mep.timestamp_format = bcmOAMTimestampFormatIEEE1588v1;
    def_mep.id = BCM_OAM_ENDPOINT_DEFAULT_INGRESS0;
    def_mep.flags |= BCM_OAM_ENDPOINT_WITH_ID;
    printf("bcm_oam_endpoint_create default_mep\n", i);
    rv = bcm_oam_endpoint_create(unit, &def_mep);
    BCM_IF_ERROR_RETURN(rv);
    printf("created MEP with id 0x%04x\n\tcounter: 0x%04x\n", def_mep.id, def_mep.lm_counter_base_id);

    /* Add dummy terminated label (Used to align labels lookup in ISEM tables) */
    rv = mpls_add_term_entry(unit, 0xfff, 0);
    BCM_IF_ERROR_RETURN(rv);

    return BCM_E_NONE;
}
