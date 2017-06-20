/*
 *
 * $Id: oam.c,v 1.0 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File: cint_oam_server.c
 * Purpose: Example of using OAMP server
 *
 * Usage:
 *
 * cd
 * cd ../../../../src/examples/dpp
 * cint utility/cint_utils_global.c
 * cint utility/cint_utils_oam.c
 * cint cint_multi_device_utils.c
 * cint cint_queue_tests.c
 * cint cint_oam_server.c
 *
 * cint
 * int client_unit=0, server_unit=1;
 * print oamp_server_example(server_unit, client_unit, 200, 201, 200, 201);
 *
 */

/* Globals */
  bcm_mac_t mac_mep_1 = {0x00, 0x00, 0x00, 0x01, 0x02, 0x03};
  bcm_mac_t mac_mep_2 = {0x00, 0x00, 0x00, 0xff, 0xff, 0xff};
  bcm_mac_t src_mac = {0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff};
  bcm_mac_t src_mac_1 = {0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xfe};
  /*1-const; 3-short format; 2-length; all the rest - MA name*/
  uint8 short_name[BCM_OAM_GROUP_NAME_LENGTH] = {1, 3, 2, 0xab, 0xcd};
  /*4-const; 3-MD-Length; MD-Name+Zero-padding; 2-const; MA-Length; MA-name*/
  uint8 str_11b_name[BCM_OAM_GROUP_NAME_LENGTH] = {4, 3, 'A', 'T', 'T', 00, 00, 02, 04, 'E', 'V', 'C', '1', 0, 0};

  bcm_vlan_port_t server_vp1, server_vp2, client_vp1, client_vp2;
  bcm_vswitch_cross_connect_t server_cross_connect, client_cross_connect;

  bcm_oam_endpoint_info_t ep_server_down, ep_server_up, ep_client_down, ep_client_up, rmep;
  int server_oamp_port = 232; /* proper apllication must be used so that this will actually be configured as the server OAMP port */
  int recycle_port_core0 = 41;
  int recycle_port_core1 = 42; /* Jericho: up MEP port and recycle port should match (in unit scope) */

  bcm_oam_group_info_t group_info_short_ma_client, group_info_short_ma_server;
  int counter_base_id_upmep, counter_base_id_downmep;
  bcm_rx_trap_config_t trap_to_server_oamp;

  int trap_code, trap_code_up;
  int down_mdl = 4;
  int up_mdl = 5;
  int use_11b_maid = 0;

  /* Created endpoints information */
  oam__ep_info_s server_upmep;
  oam__ep_info_s server_downmep;
  oam__ep_info_s client_upmep;
  oam__ep_info_s client_downmep;

/**
 *
 * Vlan-ports vp1 and vp2 will be created on the server unit and
 * will be cross-connected
 * Vlan-ports vp3 and vp4 will be created on the client unit and
 * will be cross-connected
 *
 * @author avive (25/11/2015)
 *
 * @param server_unit
 * @param client_unit
 * @param server_port1
 * @param server_port2
 * @param client_port1
 * @param client_port2
 *
 * @return int
 */
int oamp_server_create_vlan_ports(int server_unit, int client_unit, int server_port1, int server_port2, int client_port1, int client_port2) {
    int rv;

    /* Set port classification ID */
    rv = bcm_port_class_set(server_unit, server_port1, bcmPortClassId, server_port1); 
    if (rv != BCM_E_NONE) {
        printf("(%s) \n", bcm_errmsg(rv));
        return rv;
    }
    /* Set port classification ID */
    rv = bcm_port_class_set(server_unit, server_port2, bcmPortClassId, server_port2); 
        if (rv != BCM_E_NONE) {
        printf("(%s) \n", bcm_errmsg(rv));
        return rv;
    }
    /* Set port classification ID */
    rv = bcm_port_class_set(client_unit, client_port1, bcmPortClassId, client_port1);
        if (rv != BCM_E_NONE) {
        printf("(%s) \n", bcm_errmsg(rv));
        return rv;
    }
    /* Set port classification ID */
    rv = bcm_port_class_set(client_unit, client_port2, bcmPortClassId, client_port2);
        if (rv != BCM_E_NONE) {
        printf("(%s) \n", bcm_errmsg(rv));
        return rv;
    }

    /* Server VLAN port for the down MEP */
    server_vp1.criteria = BCM_VLAN_PORT_MATCH_PORT_VLAN;
    server_vp1.port = server_port1;
    server_vp1.match_vlan = 11;
    server_vp1.egress_vlan = 11;
    rv = bcm_vlan_port_create(server_unit,&server_vp1);
    if (rv != BCM_E_NONE) {
        printf("(%s) \n",bcm_errmsg(rv));
        return rv;
    }
    printf("Vlan-port created on server (unit %d) with id %d\n", server_unit, server_vp1.vlan_port_id);

    /* Server VLAN port for the up MEP */
    server_vp2.criteria = BCM_VLAN_PORT_MATCH_PORT_VLAN;
    server_vp2.port = server_port2;
    server_vp2.match_vlan = 12;
    server_vp2.egress_vlan = 12;
    rv = bcm_vlan_port_create(server_unit,&server_vp2);
    if (rv != BCM_E_NONE) {
        printf("(%s) \n",bcm_errmsg(rv));
        return rv;
    }
    printf("Vlan-port created on server (unit %d) with id %d\n", server_unit, server_vp2.vlan_port_id);

    /* Client VLAN port for the down MEP */
    client_vp1.criteria = BCM_VLAN_PORT_MATCH_PORT_VLAN;
    client_vp1.port = client_port1;
    client_vp1.match_vlan = 21;
    client_vp1.egress_vlan = 21;
    rv = bcm_vlan_port_create(client_unit,&client_vp1);
    if (rv != BCM_E_NONE) {
        printf("(%s) \n",bcm_errmsg(rv));
        return rv;
    }
    printf("Vlan-port created on client (unit %d) with id %d\n", client_unit, client_vp1.vlan_port_id);

    /* Client VLAN port for the up MEP */
    client_vp2.criteria = BCM_VLAN_PORT_MATCH_PORT_VLAN;
    client_vp2.port = client_port2;
    client_vp2.match_vlan = 22;
    client_vp2.egress_vlan = 22;
    rv = bcm_vlan_port_create(client_unit,&client_vp2);
    if (rv != BCM_E_NONE) {
        printf("(%s) \n",bcm_errmsg(rv));
        return rv;
    }
    printf("Vlan-port created on client (unit %d) with id %d\n", client_unit, client_vp2.vlan_port_id);

    /* Up MEPs are being trapped in the egress
       In oreder to create supportive environment for this, the two created VLAN ports will be cross-connected
       Packets destined for the up MEP on vp2 will be transmitted to vp1 */
    bcm_vswitch_cross_connect_t_init(&server_cross_connect);
    server_cross_connect.port1 = server_vp1.vlan_port_id;
    server_cross_connect.port2 = server_vp2.vlan_port_id;

    rv = bcm_vswitch_cross_connect_add(server_unit, &server_cross_connect);
    if (rv != BCM_E_NONE) {
        printf("(%s) \n",bcm_errmsg(rv));
        return rv;
    }

    bcm_vswitch_cross_connect_t_init(&client_cross_connect);
    client_cross_connect.port1 = client_vp1.vlan_port_id;
    client_cross_connect.port2 = client_vp2.vlan_port_id;

    rv = bcm_vswitch_cross_connect_add(client_unit, &client_cross_connect);
    if (rv != BCM_E_NONE) {
        printf("(%s) \n",bcm_errmsg(rv));
        return rv;
    }

    return 0;
}

/**
 *
 * ACC down MEP will reside on vp1 (server_vp1) + RMEP
 * ACC up MEP will reside on vp2 (server_vp2) + RMEP
 *
 * @author avive (25/11/2015)
 *
 * @param server_unit
 * @param client_unit
 * @param server_port1
 * @param server_port2
 * @param client_port1
 * @param client_port2
 *
 * @return int
 */
int oamp_server_set_server_endpoints(int server_unit, int client_unit, int server_port1, int server_port2, int client_port1, int client_port2) {
    int rv;
    int recycle_port;

    /* Down-MEP */
    bcm_oam_endpoint_info_t_init(ep_server_down);
    BCM_GPORT_TRAP_SET(ep_server_down.remote_gport, trap_code, 7, 0); /* Required for handling the traps coming from the client */
    rv = port_to_system_port(client_unit, client_port1, &ep_server_down.tx_gport); /* Set the transmition port to be the down MEP's port on the client */
    if (rv != BCM_E_NONE) {
        printf("(%s) \n", bcm_errmsg(rv));
        return rv;
    }
    sal_memcpy(ep_server_down.src_mac_address, src_mac, 6);
    sal_memcpy(ep_server_down.dst_mac_address, mac_mep_1, 6);
    ep_server_down.gport = server_vp1.vlan_port_id; /*BCM_GPORT_INVALID*/
    ep_server_down.type = bcmOAMEndpointTypeEthernet;
    ep_server_down.group = group_info_short_ma_server.id;
    ep_server_down.level = down_mdl;
    ep_server_down.name = 101;
    ep_server_down.ccm_period = BCM_OAM_ENDPOINT_CCM_PERIOD_100MS;
    ep_server_down.timestamp_format = bcmOAMTimestampFormatIEEE1588v1;
    ep_server_down.opcode_flags = BCM_OAM_OPCODE_CCM_IN_HW;
    ep_server_down.vlan = 11;
    ep_server_down.outer_tpid = 0x8100;
    ep_server_down.lm_counter_base_id = counter_base_id_downmep;
    /* Adding TLV */
    ep_server_down.flags = BCM_OAM_ENDPOINT_INTERFACE_STATE_UPDATE;
    ep_server_down.interface_state = BCM_OAM_INTERFACE_TLV_UP;

    rv = bcm_oam_endpoint_create(server_unit, &ep_server_down);
    if (rv != BCM_E_NONE) {
        printf("(%s) \n", bcm_errmsg(rv));
        return rv;
    }
    printf("Down MEP created on server (unit %d) with id %d, tx_gport is: %d\n", server_unit, ep_server_down.id, ep_server_down.tx_gport);

    /* Set the RMEP entry associated with the above endpoint.*/
    bcm_oam_endpoint_info_t_init(rmep);
    rmep.name = 201;
    rmep.local_id = ep_server_down.id;
    rmep.type = bcmOAMEndpointTypeEthernet;
    rmep.ccm_period = BCM_OAM_ENDPOINT_CCM_PERIOD_100MS;
    rmep.flags |= BCM_OAM_ENDPOINT_REMOTE | BCM_OAM_ENDPOINT_WITH_ID;
    rmep.loc_clear_threshold = 1;
    rmep.id = ep_server_down.id;

    rv = bcm_oam_endpoint_create(server_unit, &rmep);
    if (rv != BCM_E_NONE) {
        printf("(%s) \n", bcm_errmsg(rv));
        return rv;
    }
    printf("RMEP created on server (unit %d) with id %d\n", server_unit, rmep.id);

    /* Store information on global struct */
    server_downmep.gport = ep_server_down.gport;
    server_downmep.id = ep_server_down.id;
    server_downmep.rmep_id = rmep.id;

    /* Setting the correct recycle port based on client's up MEP port core */
    recycle_port = recycle_port_core0;
    if (is_device_or_above(client_unit,JERICHO)) {
        int core, tm_port;

        rv = get_core_and_tm_port_from_port(client_unit,client_port2,&core,&tm_port);
        if (rv != BCM_E_NONE){
            printf("Error, in get_core_and_tm_port_from_port\n");
            return rv;
        }

        if (core) {
            recycle_port = recycle_port_core1;
        }
    }

    /* Up-MEP */
    bcm_oam_endpoint_info_t_init(ep_server_up);
    rv = port_to_system_port(client_unit, recycle_port, &ep_server_up.remote_gport); /* Injected packets directed to client's recycle port */
    if (rv != BCM_E_NONE) {
        printf("(%s) \n", bcm_errmsg(rv));
        return rv;
    }
    rv = port_to_system_port(client_unit, client_port2, &ep_server_up.tx_gport); /* After recycling, source port (on PTCH) will be the local client's port */
    if (rv != BCM_E_NONE) {
        printf("(%s) \n", bcm_errmsg(rv));
        return rv;
    }
    sal_memcpy(ep_server_up.src_mac_address, src_mac_1, 6);
    sal_memcpy(ep_server_up.dst_mac_address, mac_mep_2, 6);
    ep_server_up.gport = server_vp2.vlan_port_id;
    ep_server_up.type = bcmOAMEndpointTypeEthernet;
    ep_server_up.group = group_info_short_ma_server.id;
    ep_server_up.level = up_mdl;
    ep_server_up.name = 102;
    ep_server_up.ccm_period = BCM_OAM_ENDPOINT_CCM_PERIOD_100MS;
    ep_server_up.timestamp_format = bcmOAMTimestampFormatIEEE1588v1;
    ep_server_up.flags = BCM_OAM_ENDPOINT_UP_FACING;
    ep_server_up.opcode_flags = BCM_OAM_OPCODE_CCM_IN_HW;
    ep_server_up.vlan = 22; /* Should be according to tx_gport */
    ep_server_up.outer_tpid = 0x8100;
    ep_server_up.lm_counter_base_id = counter_base_id_upmep; /* For up-MEPs the counter base ID must be set for both the server and client MEPs */

    rv = bcm_oam_endpoint_create(server_unit, &ep_server_up);
    if (rv != BCM_E_NONE) {
        printf("(%s) \n", bcm_errmsg(rv));
        return rv;
    }
    printf("Up MEP created on server (unit %d) with id %d, tx_gport is: %d, remote_gport is: %d\n", server_unit, ep_server_up.id, ep_server_up.tx_gport, ep_server_up.remote_gport);

    /* Set the RMEP entry associated with the above endpoint.*/
    bcm_oam_endpoint_info_t_init(rmep);
    rmep.name = 202;
    rmep.local_id = ep_server_up.id;
    rmep.type = bcmOAMEndpointTypeEthernet;
    rmep.ccm_period = BCM_OAM_ENDPOINT_CCM_PERIOD_100MS;
    rmep.flags |= BCM_OAM_ENDPOINT_REMOTE | BCM_OAM_ENDPOINT_WITH_ID;
    rmep.loc_clear_threshold = 1;
    rmep.id = ep_server_up.id;

    rv = bcm_oam_endpoint_create(server_unit, &rmep);
    if (rv != BCM_E_NONE) {
        printf("(%s) \n", bcm_errmsg(rv));
        return rv;
    }
    printf("RMEP created on server (unit %d) with id %d\n", server_unit, rmep.id);

    /* Store information on global struct */
    server_upmep.gport = ep_server_up.gport;
    server_upmep.id = ep_server_up.id;
    server_upmep.rmep_id = rmep.id;

    return 0;
}

/**
 *
 * nonACC down MEP will reside on vp3 (client_vp1)
 * nonACC up MEP will reside on vp4 (client_vp2)
 *
 * @author avive (25/11/2015)
 *
 * @param client_unit
 * @param client_port1
 * @param client_port2
 *
 * @return int
 */
int oamp_server_set_client_endpoints(int server_unit, int client_unit, int server_port1, int server_port2, int client_port1, int client_port2) {
    int rv;

    /* Down-MEP */
    bcm_oam_endpoint_info_t_init(ep_client_down);
    BCM_GPORT_TRAP_SET(ep_client_down.remote_gport, trap_code, 7, 0);
    sal_memcpy(ep_client_down.dst_mac_address, mac_mep_1, 6);
    ep_client_down.gport = client_vp1.vlan_port_id;
    ep_client_down.id = ep_server_down.id; /* The client endpoint must be set up with the ID of the server endpoint. */
    ep_client_down.flags = BCM_OAM_ENDPOINT_WITH_ID;
    ep_client_down.type = bcmOAMEndpointTypeEthernet;
    ep_client_down.group = group_info_short_ma_client.id;
    ep_client_down.level = down_mdl;
    ep_client_down.timestamp_format = bcmOAMTimestampFormatIEEE1588v1;
    ep_client_down.lm_counter_base_id = counter_base_id_downmep;

    rv = bcm_oam_endpoint_create(client_unit, &ep_client_down);
    if (rv != BCM_E_NONE) {
        printf("(%s) \n", bcm_errmsg(rv));
        return rv;
    }
    printf("Down MEP created on client (unit %d) with id %d\n", client_unit, ep_client_down.id);

    /* Store information on global struct */
    client_downmep.gport = ep_client_down.gport;
    client_downmep.id = ep_client_down.id;

    /* Up-MEP*/
    bcm_oam_endpoint_info_t_init(ep_client_up);

    BCM_GPORT_TRAP_SET(ep_client_up.remote_gport, trap_code_up, 7, 0); /* Client's EPs remote_gport field must be a trap */
    sal_memcpy(ep_client_up.dst_mac_address, mac_mep_2, 6);
    ep_client_up.gport = client_vp2.vlan_port_id;
    ep_client_up.id = ep_server_up.id; /* The client endpoint must be set up with the ID of the server endpoint. */
    ep_client_up.flags = BCM_OAM_ENDPOINT_WITH_ID | BCM_OAM_ENDPOINT_UP_FACING;
    ep_client_up.type = bcmOAMEndpointTypeEthernet;
    ep_client_up.group = group_info_short_ma_client.id;
    ep_client_up.level = up_mdl;
    ep_client_up.timestamp_format = bcmOAMTimestampFormatIEEE1588v1;
    ep_client_up.lm_counter_base_id = counter_base_id_upmep;

    rv = bcm_oam_endpoint_create(client_unit, &ep_client_up);
    if (rv != BCM_E_NONE) {
        printf("(%s) \n", bcm_errmsg(rv));
        return rv;
    }
    printf("Up MEP created on client (unit %d) with id %d\n", client_unit, ep_client_up.id);

    /* Store information on global struct */
    client_upmep.gport = ep_client_up.gport;
    client_upmep.id = ep_client_up.id;

    return 0;
}


/**
 * An example of setting up the OAMP as a server.
 * Create an up MEP and a down MEP, each first on the server
 * side (transmitting and recieving CCMs), then on the client
 * side (trapping OAM PDUs)
 *
 * @author sinai (01/09/2014)
 * updated by avive (25/11/2015)
 *
 * @param server_unit
 * @param client_unit
 * @param server_port1 - ACC Down MEP will reside on a LIF
 *              created at this port
 * @param server_port2 - ACC Up MEP will reside on a LIF created
 *              at this port
 * @param client_port1 - Down MEP will reside on a LIF created
 *              at this port
 * @param client_port2 - Up MEP will reside on a LIF created at
 *              this port
 *
 * @return int
 *                 ___
 *             ___(   )_
 *           _(         )_
 *          (  Network ___)
 *         (_      ___)
 *          (_____)<-+
 *                   |
 *                   |
 *                   |
 *  _____________    |    _____________
 * |             |<--+-->|             |
 * |             |       |             |
 * |   Server    |       |   Client    |
 * |             |\     /|             | (nonACC down & up MEPS)
 * | (vp1)       |  \ /  | (vp3)       |
 * |   (vp2)     |  / \  |   (vp4)     |
 * |_____________|/     \|_____________|
 *     |
 *   __|___
 *  |      |
 *  | OAMP | (ACC down & up MEPS)
 *  |______|
 *
 */
int oamp_server_example(int server_unit, int client_unit, int server_port1, int server_port2, int client_port1, int client_port2) {
    int rv;

    if (!is_device_or_above(server_unit, ARAD_PLUS)) {
        printf("Server unit must be Arad+ or above\n");
        return 21;
    }

    /* Create two VLAN ports on each unit (client and server) for the MEPs */
    rv = oamp_server_create_vlan_ports(server_unit, client_unit, server_port1, server_port2, client_port1, client_port2);
    if (rv != BCM_E_NONE) {
        printf("(%s) \n",bcm_errmsg(rv));
        return rv;
    }

    /* Prepare the counters */
    rv = set_counter_source_and_engines(client_unit,&counter_base_id_downmep,client_port1);
    BCM_IF_ERROR_RETURN(rv);
    rv = set_counter_source_and_engines(client_unit,&counter_base_id_upmep,client_port2);
    BCM_IF_ERROR_RETURN(rv);

    /* OAM Groups must be created on server unit and client unit.*/
    bcm_oam_group_info_t_init(&group_info_short_ma_client);
    bcm_oam_group_info_t_init(&group_info_short_ma_server);
    if (use_11b_maid) {
        /* Using long MAID - 11B */
        sal_memcpy(group_info_short_ma_client.name, str_11b_name, BCM_OAM_GROUP_NAME_LENGTH);
        sal_memcpy(group_info_short_ma_server.name, str_11b_name, BCM_OAM_GROUP_NAME_LENGTH);
    } else {
        sal_memcpy(group_info_short_ma_client.name, short_name, BCM_OAM_GROUP_NAME_LENGTH);
        sal_memcpy(group_info_short_ma_server.name, short_name, BCM_OAM_GROUP_NAME_LENGTH);
    }

    /* The server side group is used to determine the MAID on CCMs, both those transmitted and the expected MAID on recieved CCMs.
       This is conveyed through the name field.*/
    rv = bcm_oam_group_create(server_unit, &group_info_short_ma_server);
    if (rv != BCM_E_NONE) {
        printf("(%s) \n",bcm_errmsg(rv));
        return rv;
    }
    /* The client side is only used for logically clustering endpoint together. Recieved/Transmitted packets are not affected by this*/
    rv = bcm_oam_group_create(client_unit, &group_info_short_ma_client);
    if (rv != BCM_E_NONE) {
        printf("(%s) \n",bcm_errmsg(rv));
        return rv;
    }

    /* Configure a trap which updates the destination to be the OAMP of the server unit.
       This trap will be used by the client's down MEP */
    rv = bcm_rx_trap_type_create(client_unit, 0, bcmRxTrapUserDefine, &trap_code);
    if (rv != BCM_E_NONE) {
        printf("(%s) \n", bcm_errmsg(rv));
        return rv;
    }

    bcm_rx_trap_config_t_init(&trap_to_server_oamp);
    trap_to_server_oamp.flags = BCM_RX_TRAP_UPDATE_DEST | BCM_RX_TRAP_TRAP;
    rv = port_to_system_port(server_unit, server_oamp_port, &trap_to_server_oamp.dest_port);
    if (rv != BCM_E_NONE) {
        printf("(%s) \n", bcm_errmsg(rv));
        return rv;
    }

    rv = bcm_rx_trap_set(client_unit, trap_code, trap_to_server_oamp);
    if (rv != BCM_E_NONE) {
        printf("(%s) \n", bcm_errmsg(rv));
        return rv;
    }
    printf("Trap created trap_code = %d \n", trap_code);

    /* Configure another trap which updates the destination to be the OAMP of the server unit.
       This trap will be used by the client's up MEP */
    rv = bcm_rx_trap_type_create(client_unit, 0, bcmRxTrapOamUpMEP4, &trap_code_up);
    if (rv != BCM_E_NONE) {
        printf("(%s) \n", bcm_errmsg(rv));
    }
    /* Using same trap configuration */
    rv = bcm_rx_trap_set(client_unit, trap_code_up, trap_to_server_oamp);
    if (rv != BCM_E_NONE) {
        printf("(%s) \n", bcm_errmsg(rv));
        return rv;
    }
    printf("Trap created trap_code = %d \n", trap_code_up);

    /* Configure PMF to fix TLV offset on Arad+ */
    if (!is_device_or_above(server_unit, JERICHO) || use_11b_maid) {
        rv = field__prge_action_code_add_entry(server_unit, 1, trap_code, 232/*dest_port (OAMP PP port)*/,
                                               1, 3 /*prge_action_code_oam_end_tlv_wa*/);
    }

    /* Set the server endpoints */
    rv = oamp_server_set_server_endpoints(server_unit, client_unit, server_port1, server_port2, client_port1, client_port2);
    if (rv != BCM_E_NONE) {
        printf("(%s) \n",bcm_errmsg(rv));
        return rv;
    }

    /* Set the client endpoints */
    /* It is the user responsibility to verify that the fields of the server and client device match. */
    rv = oamp_server_set_client_endpoints(server_unit, client_unit, server_port1, server_port2, client_port1, client_port2);
    if (rv != BCM_E_NONE) {
        printf("(%s) \n",bcm_errmsg(rv));
        return rv;
    }

    return 0;
}

/**
 * Replaces tx_gport / remote_gport and vlan of an up MEP
 * 
 * @author avive (15/12/2015)
 * 
 * @param server_unit
 * @param server_upmep_id
 * @param new_tx_unit
 * @param new_tx_port
 * @param new_entering_vlan
 * 
 * @return int
 */
int oamp_server_switch_upmep_tx_unit(int server_unit, int server_upmep_id, int new_tx_unit, int new_tx_port, int new_entering_vlan) {
    int rv;
    int recycle_port;

    /* Get the server up MEP */
    bcm_oam_endpoint_info_t_init(ep_server_up);
    rv = bcm_oam_endpoint_get(server_unit, server_upmep_id, &ep_server_up);
    if (rv != BCM_E_NONE) {
        printf("(%s) \n", bcm_errmsg(rv));
        return rv;
    }

    /* Setting the correct recycle port based on tx_unit's up MEP port core */
    recycle_port = recycle_port_core0;
    if (is_device_or_above(new_tx_unit,JERICHO)) {
        int core, tm_port;

        rv = get_core_and_tm_port_from_port(new_tx_unit,new_tx_port,&core,&tm_port);
        if (rv != BCM_E_NONE){
            printf("Error, in get_core_and_tm_port_from_port\n");
            return rv;
        }

        if (core) {
            recycle_port = recycle_port_core1;
        }
    }

    /* Set the new remote_gport, tx_gport and vlan*/
    rv = port_to_system_port(new_tx_unit, recycle_port, &ep_server_up.remote_gport); /* Injected packets directed to new_remote_gport */
    if (rv != BCM_E_NONE) {
        printf("(%s) \n", bcm_errmsg(rv));
        return rv;
    }
    rv = port_to_system_port(new_tx_unit, new_tx_port, &ep_server_up.tx_gport); /* After recycling, source port (on PTCH) will be new_tx_gport */
    if (rv != BCM_E_NONE) {
        printf("(%s) \n", bcm_errmsg(rv));
        return rv;
    }
    ep_server_up.vlan = new_entering_vlan; /* Should be according to tx_gport */

    /* Update the server up MEP endpoint */
    ep_server_up.flags |= BCM_OAM_ENDPOINT_WITH_ID | BCM_OAM_ENDPOINT_REPLACE;
    rv = bcm_oam_endpoint_create(server_unit, &ep_server_up);
    if (rv != BCM_E_NONE) {
        printf("(%s) \n", bcm_errmsg(rv));
        return rv;
    }

    printf("Up MEP 0x%x on server unit %d will now transmit injected packets through unit %d, port %d, vlan %d\n", server_upmep_id, server_unit, new_tx_unit, new_tx_port, new_entering_vlan);

    return 0;
}

/*
 * Sets the key to be [OAM-Trap-code, OAMP-dest-port] and sets the value for that key
 * That value is used to identify packets trapped to the OAMP and apply different workarounds if necessary.
 */
int field_oam_advanced_features(int unit) {
    int result;
    bcm_field_group_t grp_tcam = 1;
    int group_priority = 10;
    int qual_id = 1;
    int trap_id;
    int flags = 0;

    /*Get trap_id value for OAM trap*/
    result = bcm_rx_trap_type_get(unit,flags,bcmRxTrapOamEthAccelerated,&trap_id);
    if (BCM_E_NONE != result) {
        printf("Error in bcm_rx_trap_type_get\n");
        return result;
    }

    /* Create field group for OAM EndTLV WA */
    result = field__prge_action_create_field_group(unit, group_priority, grp_tcam, qual_id);
    if (BCM_E_NONE != result) {
        printf("Error in prge_end_tlv_create_field_group\n");
        return result;
    }

    /* Set action code for the created group */
    printf("Adding entry to mark OAM packets for advances processing\n");
    /* Default trap, for up MEPs */
    result = field__prge_action_code_add_entry(unit, grp_tcam, trap_id, 232/*dest_port (OAMP PP port)*/,
                                               qual_id, 3 /*prge_action_code_oam_end_tlv_wa*/);
    if (BCM_E_NONE != result) {
        printf("Error in prge_end_tlv_add_entry\n");
        return result;
    }
    /* Configured trap (the one created in oamp_server_example), for down MEPs */
    result = field__prge_action_code_add_entry(unit, grp_tcam, trap_code, 232/*dest_port (OAMP PP port)*/,
                                               qual_id, 3 /*prge_action_code_oam_end_tlv_wa*/);
    if (BCM_E_NONE != result) {
        printf("Error in prge_end_tlv_add_entry\n");
        return result;
    }

    return result;
}

