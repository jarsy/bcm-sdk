/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~OAM test~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/*
 *
 * $Id: cint_oam.c,v 1.21 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 * 
 * File: cint_oam.c
 * Purpose: Example of using OAM APIs.
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
cint
int unit =0;
int port_1=13, port_2 = 14; port_3=15;
print oam_run_with_defaults(unit,port_1,port_2,port_3);

 * 
 * This cint uses cint_vswitch_metro_mp_single_vlan.c to build the following vswitch:
 *  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ 
 *  |                                                                       |
 *  |                   o  \                         o                      |
 *  |                       \  \<4>       <10,20>/  /                       |
 *  |                  /\ \  \   -----------    /  /                        |
 *  |                   \  \ \/ /  \   /\   \  /  / /\                      |
 *  |                <40>\  \  |    \  /     |\/ /  /                       |
 *  |                     \  \ |     \/      |  /  /<30>                    |
 *  |                       p3\|     /\      | /  /                         |
 *  |                          |    /  \     |/p1                           |             
 *  |                         /|   \/   \    |                              |
 *  |                        /  \  VSwitch  /                               | 
 *  |                   /\  /p2  -----------                                |
 *  |                <5>/  /  /                                             |
 *  |                  /  /  /<3>                                           |
 *  |                 /  /  /                                               |
 *  |                   /  \/                                               |
 *  |                  o                                                    |
 *  |                                                                       | 
 *  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * 
 * The following MAs:
 * 		1) MA with id 0 & long name format:       1-const; 32-long format; 13-length; all the rest - MA name
 *         long_name[BCM_OAM_GROUP_NAME_LENGTH] = {1, 32, 13, 1, 2, 3, 4, 5, 6, 7, 8, 9}
 * 		2) MA with id 1 & short name format:    1-const; 3-short format; 2-length; all the rest - MA name
 *        short_name[BCM_OAM_GROUP_NAME_LENGTH] = {1, 3, 2, 0xcd, 0xab}
 * 
 * The following MEPs:
 * 		1) MEP with id 0X400<MEP's LIF> : non-accellerated, mac address {0x00, 0x00, 0x00, 0x01, 0x02, 0x03}, in group 2, MD-Level 4, Down-MEP
 *                   ---> Packets recieved on port_1 with VLAN 0xa and MD-Level 4 will be trapped by this MEP.
 *                                  -- For example (from the bcm shell)
 *                                          tx 1 PtchSRCport=<port_1> DATA=0000000102030000556abcde8100000a8902802b000c00000000000000000000000000000000
 *  	2) MEP with id 4096: accellerated,     mac address {0x00, 0x00, 0x00, 0xff, 0xff, 0xff}, in group 2, MD-Level 5, Up-MEP
 *                   ---> Packets recieved on port_1 with VLAN 0xa and MD-Level 5 will be trapped by this MEP (at the egress, MEP resides on port_2, at the OutLIF)
 *  	    2.a) RMEP with id 0X2001000 and Name 255
 *                          ---> CCMs with Name 255 and the above Level/Vlan will be processed by the RMEP in the OAMP.
 *                                  -- For example (from the bcm shell)
 *                                          tx 1 PtchSRCport=13 DATA=0000000102040000556abcde8100000a8902a00101460000000000ff010302abcd00000000000000
 *                   ---> The OAMP will transmit CCMs with with VLAN 0x5, Up MEP. VLAN editing may be applied in the pipeline.
 *      3)  MEP with id 4097: accellerated,     mac address {0x00, 0x00, 0x00, 0x01, 0x02, 0xff}, in group 2, mdlevel 2, Down-MEP
 *                   ---> Packets recieved on port_1 with VLAN 0xa and MD-Level 4 will be trapped by this MEP.
 *          3.a) RMEP with id 0X2001001 and Name 17
 *                          ---> CCMs with Name 17 and the above Level/Vlan will be processed by the RMEP in the OAMP.
 *                   ---> The OAMP will transmit CCMs with with VLAN 0xa, Down MEP. 
 *                                  -- For example (from the bcm shell)
 *                                          tx 1 PtchSRCport=<port_1> DATA=0000000102030000556abcde8100000a8902402b000c00000000000000000000000000000000
 *
 *
 * 
 * In addition, get & delete APIs are used for testing.
 * 
 * comments:
 * 1) In order to prevent from OAMP send packets do: BCM.0> m OAMP_MODE_REGISTER TRANSMIT_ENABLE=0
 * 
 */
 
/*
 * Creating vswitch and adding mac table entries
 */

/* set in order to do rmep_id encoding using utility functions and not using APIs. */
int encoding = 0;
bcm_oam_endpoint_t remote_id_system = 0;
bcm_oam_endpoint_t local_id_system = 0;

/* Globals - MAC addresses , ports & gports*/
  bcm_multicast_t mc_group = 1234;
  bcm_mac_t mac_mep_1 = {0x00, 0x00, 0x00, 0x01, 0x02, 0x03};
  bcm_mac_t mac_mep_2 = {0x00, 0x00, 0x00, 0xff, 0xff, 0xff};
  bcm_mac_t mac_mep_3 = {0x00, 0x00, 0x00, 0x01, 0x02, 0xff};
  bcm_mac_t mac_mep_4 = {0x00, 0x00, 0x00, 0xff, 0xff, 0xfd};
  bcm_mac_t mac_mip = {0x00, 0x00, 0x00, 0x01, 0x02, 0xfe};
  bcm_mac_t src_mac_mep_2 = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05};
  bcm_mac_t src_mac_mep_3 = {0x00, 0x01, 0x02, 0x03, 0x04, 0x01};
  bcm_mac_t src_mac_mep_4 = {0x00, 0x01, 0x02, 0x03, 0x04, 0x07};
  bcm_mac_t mac_mep_2_mc = {0x01, 0x80, 0xc2, 0x00, 0x00, 0x35}; /* 01-80-c2-00-00-3 + md_level_2 */
  bcm_gport_t gport1, gport2, gport3; /* these are the ports created by the vswitch*/
  int port_1 = 13; /* physical port (signal generator)*/
  int port_2 = 14;
  int port_3 = 15;
  bcm_oam_group_info_t group_info_long_ma;
  bcm_oam_group_info_t group_info_short_ma;
  bcm_oam_group_info_t group_info_11b_ma;
  bcm_oam_group_info_t group_info_48b_ma;

  bcm_oam_endpoint_info_t mep_acc_info;
  bcm_oam_endpoint_info_t rmep_info;
  /*1-const; 32-long format; 13-length; all the rest - MA name*/
  uint8 long_name[BCM_OAM_GROUP_NAME_LENGTH] = {1, 32, 13, 01, 02, 03, 04, 05, 06, 07, 08, 09, 0xa, 0xb, 0xc, 0xd};
  /*1-const; 3-short format; 2-length; all the rest - MA name*/
  uint8 short_name[BCM_OAM_GROUP_NAME_LENGTH] = {1, 3, 2, 0xab, 0xcd};
  /*4-const; 3-MD-Length; MD-Name+Zero-padding; 2-const; MA-Length; MA-name*/
  uint8 str_11b_name[BCM_OAM_GROUP_NAME_LENGTH] = {4, 3, 'A', 'T', 'T', 00, 00, 02, 04, 'E', 'V', 'C', '1', 0, 0};
  /* 48 bytes format */
  uint8 str_48b_name[BCM_OAM_GROUP_NAME_LENGTH] = {1,2,3,4,5,6,7,8,9,1,2,3,4,5,6,7,8,9,1,2,3,4,5,6,7,8,9,1,2,3,4,5,6,7,8,9,1,2,3,4,5,6,7,8,9,1,2,3};

  int timeout_events_count = 0;
  int remote_event_count=0;
  int port_interface_event_count=0;
  int timeout_events_count_multi_oam[2] ={ 0 };
  bcm_vlan_t vsi;
  int md_level_mip = 7;
  bcm_oam_endpoint_info_t mip_info;
  int sampling_ratio = 0;
  int use_port_interface_status = 0;
  int use_11b_maid = 0;
  int use_48_maid = 0;

  /* Created endpoints information */
  oam__ep_info_s ep1;
  oam__ep_info_s ep2;
  oam__ep_info_s ep3;
  oam__ep_info_s ep4;
  oam__ep_info_s ep_def;
  oam__ep_info_s acc_ep_up;
  oam__ep_info_s acc_ep_down;

/* enable OAM statistics per up mep session, need to load the cint: cint_field_oam_statistics.c*/
int oam_up_statistics_enable=0;

/* Enable LOC events as soon as the remote endpoints are created */
int remote_meps_start_in_loc_enable=0;

/* Whether to add by default bcmOAMActionCountEnable when using oam_action_set */
int count_enable_on_oam_action = 1;

/* Indicates if bcmRxTrapOamUpMEP4 trap has already been created */
int gTrapUp4AlreadyCreated = 0;

/**
 * NTP timestamp format will be used by default, unless disabled 
 * by soc property 
 * 
 * @author sinai (09/12/2014)
 * 
 * @param unit 
 * 
 * @return bcm_oam_timestamp_format_t 
 */
bcm_oam_timestamp_format_t get_oam_timestamp_format(int unit) {
    return soc_property_get(unit , "oam_dm_ntp_enable",1)  ? bcmOAMTimestampFormatNTP : bcmOAMTimestampFormatIEEE1588v1;
}


/****************************************************************/
/*                              OAM UTILITIES FUNCTIONS                                                       */
/****************************************************************/

/**
 * Prints group id and MEG, then traverses and prints its endpoints
 *
 */
int print_group_eps(int unit, bcm_oam_group_info_t* info, void* user_data) {
    int i;
    printf("Group: %d\nName: 0x", info->id, group_name);
    for(i=0; i<BCM_OAM_GROUP_NAME_LENGTH; i++) {
        printf("%02x", info->name[i]);
    }
    printf("\n");
    bcm_oam_endpoint_traverse(unit, info->id, print_ep, (void*)0);
}

/**
 * Calls "diag oam ep" for a given endpoint
 *
 */
int print_ep(int unit, bcm_oam_endpoint_info_t* info, void* user_data) {
    char line[256];
    sprintf(line, "diag oam ep id=%d", info->id);
    bshell(unit, line);
}

/**
 * Traverses groups, prints group id and MEG, then traverses and calls diag on its endpoints
 *
 * @param unit
 *
 * @return int
 */
int oam_groups_print(int unit) {
    return bcm_oam_group_traverse(unit, print_group_eps, (void*)0);
}

/**
 * Set the ID when calling bcm_oam_endpoint_create() with the 
 * flag _WITH_ID set. 
 *  
 * 
 * @param mep_info 
 * 
 * @return int 
 */
int system_endpoint_to_endpoint_id_oam(bcm_oam_endpoint_info_t *mep_info) {
    if (mep_info->flags & BCM_OAM_ENDPOINT_REMOTE) {
        mep_info->id |= (1 << 25/*_BCM_OAM_REMOTE_MEP_INDEX_BIT*/);
        
    }
    else if (!(mep_info->opcode_flags & BCM_OAM_OPCODE_CCM_IN_HW)) {
        printf("Can not create non-accelerated endpoint with id\n");
		return BCM_E_FAIL;
    }
    return BCM_E_NONE;
}


/**
 * Functions used to read various event counters.
 * 
 * 
 * @param expected_event_count 
 * 
 * @return int 
 */
int read_timeout_event_count(int expected_event_count) {
	printf("timeout_events_count=%d\n",timeout_events_count);

if (timeout_events_count==expected_event_count) {
		return BCM_E_NONE;
	}
	else {
		return BCM_E_FAIL;
	}
}

int read_remote_timeout_event_count(int expected_count) {
    printf("timeout_events_count=%d\n",remote_event_count);

    if (remote_event_count==expected_count) {
        return BCM_E_NONE;
    }
    else {
        return BCM_E_FAIL;
    }
}

int read_port_interface_event_count(int expected_count) {
    printf("port_interface_event_count=%d\n",port_interface_event_count);


    if (port_interface_event_count==expected_count) {
        return BCM_E_NONE;
    }
    else {
        return BCM_E_FAIL;
    }
}

/**
 * Function verifies that for each of the two accelerated 
 * endpoints, the cb was called as many times as expected. 
 * 
 * @author sinai (24/11/2013)
 * 
 * @param expected_event_count 
 * 
 * @return int 
 */
int read_timeout_event_count_multi_oam(int expected_event_count_on_ep_1,int expected_event_count_on_ep_2) {
    int i;

    if (timeout_events_count_multi_oam[0] != expected_event_count_on_ep_1 ||  timeout_events_count_multi_oam[1] != expected_event_count_on_ep_2 ) {
        return BCM_E_FAIL;
    }
  
    return BCM_E_NONE;
}

/**
 * Wrapper function for transmitting packet, including PTCH.
 * 
 * 
 * @param unit 
 * @param data 
 * @param ptch0 - set local port from which packet should be 
 *              transmitted.
 * @param ptch1 
 * 
 * @return int 
 */
int oam_tx(int unit, char *data, int ptch0, int ptch1) {
    int ptch[2];
    ptch[0] = ptch0;
    ptch[1] = ptch1;

    return tx_packet_via_bcm(unit, data, ptch, 2);
}


/* OAM-TS header:
 *
 * OAM-TS header is the following:
 * bits                       meaning
 * ===================================
 * 47:46                      type of OAM-TS extension: 0-OAM 
 * 45:43                      OAM-Sub-Type: 0-CCM; 1-LM; 2-DM (1588 ToD) 3-DM (NTP ToD)
 * 42                         Up-MEP ('1') or Down-MEP ('0')
 * 41:8                       OAM-TS-Data: Interpretation of OAM-TS-Data depends on the OAM-TS type, sub type, and location in the system.
 *										Location in the system may be:
 *										IRPP - following ITMH on an injected packet
 *										ETPP or Out I/F - following FTMH
 *										OAM-Data is a function of (Location, Sub-Type, MEP-Type, Inject/Trap):
 *										"	(IRPP, LM, Up, Inj) - NA
 *										"	(IRPP, LM, Dn, Inj) - Counter-Pointer // bypass
 *										"	(IRPP, DM, Up, Inj) - NA
 *										"	(IRPP, DM, Dn, Inj) - Null // bypass
 *										"	(IRPP, LM/DM, Up/Dn, Trp) - NA
 *										"	(ETPP, LM, Up, Inj) - Counter-Value // Stamp into packet
 *										"	(ETPP, LM, Dn, Inj) - Counter-Pointer // Read counter and stamp into packet
 *										"	(ETPP, DM, Up, Inj) - ToD // Stamp into packet
 *										"	(ETPP, DM, Dn, Inj) -Null //Read ToD and stamp into packet
 *
 *										"	(ETPP, LM, Up, Trp) - NA // ETPP build OAM-TS Header w/ Counter-Value
 *										"	(ETPP, LM, Dn, Trp) - Counter-Value // bypass to Out I/F
 *										"	(ETPP, DM, Up, Trp) - NA // ETPP build OAM-TS Header w/ ToD
 *  										"	(ETPP, DM, Dn, Trp) - ToD// bypass to Out I/F
 * 7:0                         Offset from end of System Headers to where to stamp/update.
 * 
 * parsing pkt_dm_down packet:
 * 0x800c000d|180000000016|000000010203000000ffffff8100|000a8902|A02f002000000000000003e70000
 *    ITMH       OAM-TS            ETH.                  VLAN       OAM
 * ITMH - send packet to port 13
 * OAM-TS - packet type DM, stamping offset - 16
 * 
 * parsing pkt_lm_down packet:
 * 0x800c000d|080000000516|000000ffffff0000000000018100|000a8902|802b000c000000000000000000000000
 *    ITMH       OAM-TS            ETH.                  VLAN       OAM
 * ITMH - send packet to port 13
 * OAM-TS - packet type LM, stamping offset - 16, counter id - 5
 *  
 * parsing pkt_lm_up packet: no headers
 * 0x000000010203000000ffffff8100|000a8902|A02b000c0000000000000000000000000
 *            ETH.                  VLAN       OAM
 */
int inject_dm_and_lm_packets(int unit) {
  bcm_error_t rv;
  char *pkt_dm_down, *pkt_dm_up, *pkt_lm_down, *pkt_lm_up;
  int ptch_down[2];
  int ptch_up[2];

  pkt_dm_down = "0x800c000d180000000016000000010203000000ffffff8100000a8902A02f002000000000000003e70000"; /*DM down*/
  pkt_lm_down = "0x800c000d080000000516000000ffffff0000000000018100000a8902802b000c000000000000000000000000"; /*LM down*/
  ptch_down[0] = 0; /* The next header is ITMH */
  ptch_down[1] = 0; /* in port is port_1 */

  pkt_dm_up = "0x000000010203000000ffffff810000058902A02f0020000000000000000000000000"; /*DM up*/
  pkt_lm_up = "0x000000010203000000ffffff810000058902A02b000c0000000000000000000000000"; /*LM up*/
  ptch_up[0] = 240; /* Next header should be deduced from the SSP; Opaque-PT-Attributes = 7 */
  ptch_up[1] = port_2; /* SSP = port_2 */

  rv = tx_packet_via_bcm(unit, pkt_lm_down, ptch_down, 2);
  rv = tx_packet_via_bcm(unit, pkt_dm_down, ptch_down, 2);
  rv = tx_packet_via_bcm(unit, pkt_lm_up, ptch_up, 2);
  rv = tx_packet_via_bcm(unit, pkt_dm_up, ptch_up, 2);
  return rv;
}



/*****************************************************************************/
/*                                        OAM BASIC EXAMPLE                                                                               */
/*****************************************************************************/



/**
 * Basic OAM example.
 * Creates vswitch on 3 given ports and the folowing endpoint: 
 * 1) Default endpoint 
 * 2) Non accelerated down MEP on port1
 * 3) Accelerated up MEP on port2 + RMEP 
 * 4) Accelerated down MEP on port1 + RMEP
 *  
 * 
 * @param unit 
 * @param port1 
 * @param port2 
 * @param port3 
 * 
 * @return int 
 */
int oam_run_with_defaults(int unit, int port1, int port2, int port3) {
  bcm_error_t rv;

  single_vlan_tag = 1;

  port_1 = port1;
  port_2 = port2;
  port_3 = port3;

/*enable oam statistics per mep session*/
  if (oam_up_statistics_enable) {
	  rv = oam_tx_up_stat(unit);
	  if (rv != BCM_E_NONE) {
		  printf("(%s) \n",bcm_errmsg(rv));
		  return rv;
	  }
      /*rv = oam_rx_down_stat(unit);
          if (rv != BCM_E_NONE) {
          printf("(%s) \n",bcm_errmsg(rv));
          return rv;
	  }*/
  }

  printf("Registering OAM events\n");
  rv = register_events(unit);
  if (rv != BCM_E_NONE) {
	  printf("(%s) \n",bcm_errmsg(rv));
	  return rv;
  }
  
  rv = oam_example(unit);
  return rv;
}

/**
* Cint that only uses bcm APIs to create non accelerated Up,
 * Down MEP
 *
 *
 * @param unit
 * @param port1
 * @param port2
 *
 * @return int
 */
int oam_standalone_example(int unit,int port1,int port2) {

    bcm_error_t rv = BCM_E_NONE;
    bcm_vlan_port_t vp1, vp2;
    bcm_oam_endpoint_info_t mep_not_acc_info;
    bcm_oam_group_info_t group_info_short_ma;
    uint8 short_name[BCM_OAM_GROUP_NAME_LENGTH] = {1, 3, 2, 0xab, 0xcd};
    int md_level_1 = 2;
    int md_level_2 = 5;
    int lm_counter_base_id_1;
    int lm_counter_base_id_2;
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

    vp1.criteria = BCM_VLAN_PORT_MATCH_PORT_VLAN;
    vp1.port = port1;
    vp1.match_vlan = 10;
    vp1.egress_vlan = 10;
    rv=bcm_vlan_port_create(unit,&vp1);
    if (rv != BCM_E_NONE) {
        printf("(%s) \n",bcm_errmsg(rv));
        return rv;
    }

    /*Endpoints will be created on 2 different LIFs. */
    rv = set_counter_source_and_engines(unit, &lm_counter_base_id_2, port1);
    BCM_IF_ERROR_RETURN(rv);
    rv = set_counter_source_and_engines(unit, &lm_counter_base_id_1, port2);
    BCM_IF_ERROR_RETURN(rv); 

    vp2.criteria = BCM_VLAN_PORT_MATCH_PORT_VLAN;
    vp2.port = port2;
    vp1.match_vlan = 12;
    vp2.egress_vlan = 12;
    rv=bcm_vlan_port_create(unit,&vp2);
    if (rv != BCM_E_NONE) {
        printf("(%s) \n",bcm_errmsg(rv));
        return rv;
    }

    bcm_vswitch_cross_connect_t_init(&cross_connect);

    cross_connect.port1 = vp1.vlan_port_id;
    cross_connect.port2 = vp2.vlan_port_id;

    rv = bcm_vswitch_cross_connect_add(unit, &cross_connect);
    if (rv != BCM_E_NONE) {
        printf("(%s) \n",bcm_errmsg(rv));
        return rv;
    }

     bcm_oam_group_info_t_init(&group_info_short_ma);
     sal_memcpy(group_info_short_ma.name, short_name, BCM_OAM_GROUP_NAME_LENGTH);
     rv = bcm_oam_group_create(unit, &group_info_short_ma);
     if( rv != BCM_E_NONE) {
         printf("MEG:\t (%s) \n",bcm_errmsg(rv));
         return rv;
     }

  /* 
  *  
  * Adding non acc down MEP
  */

/*TX*/
    bcm_oam_endpoint_info_t_init(&mep_not_acc_info);
    mep_not_acc_info.type = bcmOAMEndpointTypeEthernet;
    mep_not_acc_info.group = group_info_short_ma.id;
    mep_not_acc_info.level = md_level_1;

 /*RX*/
    mep_not_acc_info.gport = vp1.vlan_port_id;
    sal_memcpy(mep_not_acc_info.dst_mac_address, mac_mep_1, 6);
    mep_not_acc_info.lm_counter_base_id = lm_counter_base_id_1;
    mep_not_acc_info.timestamp_format = get_oam_timestamp_format(unit);

    printf("bcm_oam_endpoint_create mep_not_acc_info\n");
    mep_not_acc_info.timestamp_format = get_oam_timestamp_format(unit);
    rv = bcm_oam_endpoint_create(unit, &mep_not_acc_info);
    if (rv != BCM_E_NONE) {
    printf("(%s) \n",bcm_errmsg(rv));
    return rv;
    }
    printf("created MEP with id %d\n", mep_not_acc_info.id);

     /* 
  *  
  * Adding non acc UP MEP
  */

/*TX*/
    bcm_oam_endpoint_info_t_init(&mep_not_acc_info);
    mep_not_acc_info.type = bcmOAMEndpointTypeEthernet;
    mep_not_acc_info.group = group_info_short_ma.id;
    mep_not_acc_info.level = md_level_2;
    mep_not_acc_info.tx_gport = BCM_GPORT_INVALID; /*Up MEP requires gport invalid.*/
    mep_not_acc_info.flags |= BCM_OAM_ENDPOINT_UP_FACING;

 /*RX*/
    mep_not_acc_info.gport = vp2.vlan_port_id;
    sal_memcpy(mep_not_acc_info.dst_mac_address, mac_mep_1, 6);
    mep_not_acc_info.lm_counter_base_id = lm_counter_base_id_2;
    mep_not_acc_info.timestamp_format = get_oam_timestamp_format(unit);

    printf("bcm_oam_endpoint_create mep_not_acc_info\n");
    mep_not_acc_info.timestamp_format = get_oam_timestamp_format(unit);
    rv = bcm_oam_endpoint_create(unit, &mep_not_acc_info);
    if (rv != BCM_E_NONE) {
    printf("(%s) \n",bcm_errmsg(rv));
    return rv;
    }
    printf("created UP MEP with id %d\n", mep_not_acc_info.id);

    return rv;
}

/**
 * Basic OAM example. creates the following entities: 
 * 1) vswitch on which OAM endpoints are defined. 
 * 2) OAM group with short MA name (set on outgoing CCMs for
 * accelerated endpoints). 
 * 3) Default endpoint. 
 * 4) Non accelerated endpoint 
 * 5) Accelerated down MEP 
 * 6) Accelerated up MEP 
 * 
 * 
 * @param unit 
 * 
 * @return int 
 */
int oam_example(int unit) {
  bcm_error_t rv;
  bcm_oam_group_info_t group_info_long_ma_test;
  bcm_oam_group_info_t *group_info;
  bcm_oam_endpoint_info_t mep_not_acc_info;
  bcm_oam_endpoint_info_t mep_not_acc_test_info;
  bcm_oam_endpoint_info_t default_info;

  int md_level_1 = 4;
  int md_level_2 = 5;
  int md_level_3 = 2;
  int md_level_4 = 1;
  int lm_counter_base_id_1  ;
  int lm_counter_base_id_2  ;

  int dev_type_val;

  rv = create_vswitch_and_mac_entries(unit);
  if (rv != BCM_E_NONE) {
	  printf("(%s) \n",bcm_errmsg(rv));
	  return rv;
  }
  
  rv = get_device_type(unit, &device_type);
  if (rv < 0) {
      printf("Error checking whether the device is arad+.\n");
      print rv;
      return rv;
  }

  printf("Creating two groups (short, long and 11b-maid name format)\n");
  bcm_oam_group_info_t_init(&group_info_long_ma);
  sal_memcpy(group_info_long_ma.name, long_name, BCM_OAM_GROUP_NAME_LENGTH);
  /* This is meant for QAX and above only */
  rv = get_device_type(unit, &device_type);
  BCM_IF_ERROR_RETURN(rv);
  if (device_type >= device_type_qax) {
      group_info_long_ma.group_name_index = 0x2014; /* Bank 8, entry 20 */
  }
  rv = bcm_oam_group_create(unit, &group_info_long_ma);
  if (rv != BCM_E_NONE) {
	  printf("(%s) \n",bcm_errmsg(rv));
	  return rv;
  }

  bcm_oam_group_info_t_init(&group_info_short_ma);
  sal_memcpy(group_info_short_ma.name, short_name, BCM_OAM_GROUP_NAME_LENGTH);
  if (device_type >= device_type_qax) {
      group_info_short_ma.group_name_index = 0x2015; /* Bank 8, entry 21 */
  }
  rv = bcm_oam_group_create(unit, &group_info_short_ma);
  if (rv != BCM_E_NONE) {
	  printf("(%s) \n",bcm_errmsg(rv));
	  return rv;
  }

  /*Endpoints will be created on 2 different LIFs. */
  rv = set_counter_source_and_engines(unit,&lm_counter_base_id_2,port_2);
  BCM_IF_ERROR_RETURN(rv); 
  rv = set_counter_source_and_engines(unit,&lm_counter_base_id_1,port_1);
  BCM_IF_ERROR_RETURN(rv); 

  if (use_11b_maid) {
      /* Add a group with sttring based 11b MAID */
      bcm_oam_group_info_t_init(&group_info_11b_ma);
      sal_memcpy(group_info_11b_ma.name, str_11b_name, BCM_OAM_GROUP_NAME_LENGTH);
      rv = bcm_oam_group_create(unit, &group_info_11b_ma);
      if (rv != BCM_E_NONE) {
          printf("(%s) \n", bcm_errmsg(rv));
          return rv;
      }
      /* Set the used group for the MEPs to this group */
      group_info = &group_info_11b_ma;
  } else if (use_48_maid){
      /* Add a group with flexible 48 Byte MAID */
      bcm_oam_group_info_t_init(&group_info_48b_ma);
      sal_memcpy(group_info_48b_ma.name, str_48b_name, BCM_OAM_GROUP_NAME_LENGTH);
      group_info_48b_ma.flags = BCM_OAM_GROUP_FLEXIBLE_MAID_48_BYTE;
      if (device_type >= device_type_qax) {
          group_info_48b_ma.group_name_index = 0x2016; /* Bank 8, entry 22 */
      }
      rv = bcm_oam_group_create(unit, &group_info_48b_ma);
      if (rv != BCM_E_NONE) {
          printf("(%s) \n", bcm_errmsg(rv));
          return rv;
      }
      /* Set the used group for the MEPs to this group */
       group_info = &group_info_48b_ma;
  }
  else {
      /* Set the used group for the MEPs to the group with the short name */
      group_info = &group_info_short_ma;
  }

  bcm_oam_group_info_t_init(&group_info_long_ma_test);
  printf("bcm_oam_group_get\n"); 
  rv = bcm_oam_group_get(unit, group_info_long_ma.id, &group_info_long_ma_test);
  if (rv != BCM_E_NONE) {
	  printf("(%s) \n",bcm_errmsg(rv));
	  return rv;
  }

  printf("bcm_oam_group_destroy\n"); 
  rv = bcm_oam_group_destroy(unit, group_info_long_ma.id);
  if (rv != BCM_E_NONE) {
	  printf("(%s) \n",bcm_errmsg(rv));
	  return rv;
  }

  if (device_type >= device_type_qax) {
      group_info_long_ma.group_name_index = 0x2017; /* Bank 8, entry 23 */
  }
  rv = bcm_oam_group_create(unit, &group_info_long_ma);
  if (rv != BCM_E_NONE) {
	  printf("(%s) \n",bcm_errmsg(rv));
	  return rv;
  }

  /*
  * Adding a default MEP
  */
  printf("Add default mep\n");
  bcm_oam_endpoint_info_t_init(&default_info);
  if (device_type < device_type_arad_plus) {
      default_info.id = -1;
  }
  else {
      default_info.id = BCM_OAM_ENDPOINT_DEFAULT_INGRESS0;
  }
  default_info.flags |= BCM_OAM_ENDPOINT_WITH_ID;
  default_info.timestamp_format = get_oam_timestamp_format(unit);
  rv = bcm_oam_endpoint_create(unit, &default_info);
  if (rv != BCM_E_NONE) {
      printf("(%s) \n",bcm_errmsg(rv));
      return rv;
  }

  ep_def.gport = default_info.gport;
  ep_def.id = default_info.id;

  /*
  * Adding non acc MEP
  */

  bcm_oam_endpoint_info_t_init(&mep_not_acc_info);
  mep_not_acc_info.type = bcmOAMEndpointTypeEthernet;
  mep_not_acc_info.group = group_info->id;
  mep_not_acc_info.level = md_level_1;
  mep_not_acc_info.gport = gport1;
  mep_not_acc_info.name = 0;     
  mep_not_acc_info.ccm_period = 0;
  sal_memcpy(mep_not_acc_info.dst_mac_address, mac_mep_1, 6);
  mep_not_acc_info.lm_counter_base_id  = lm_counter_base_id_1;

  printf("bcm_oam_endpoint_create mep_not_acc_info\n"); 
  mep_not_acc_info.timestamp_format = get_oam_timestamp_format(unit);
  rv = bcm_oam_endpoint_create(unit, &mep_not_acc_info);
  if (rv != BCM_E_NONE) {
	  printf("(%s) \n",bcm_errmsg(rv));
	  return rv;
  }
  printf("created MEP with id %d\n", mep_not_acc_info.id);
  ep1.gport = mep_not_acc_info.gport;
  ep1.id = mep_not_acc_info.id;

  /*
  * Adding acc MEP - upmep
  */

  bcm_oam_endpoint_info_t_init(&mep_acc_info);

  /*TX*/
  mep_acc_info.type = bcmOAMEndpointTypeEthernet;
  mep_acc_info.group = group_info->id;
  mep_acc_info.level = md_level_2;
  mep_acc_info.tx_gport = BCM_GPORT_INVALID; /*Up MEP requires gport invalid.*/
  mep_acc_info.name = 123;     
  mep_acc_info.ccm_period = 1;
  mep_acc_info.flags |= BCM_OAM_ENDPOINT_UP_FACING;
  mep_acc_info.opcode_flags |= BCM_OAM_OPCODE_CCM_IN_HW;

  mep_acc_info.vlan = 5;
  mep_acc_info.pkt_pri = mep_acc_info.pkt_pri = 0 + (2<<1); /* dei(1bit) + (pcp(3bit) << 1)*/
  mep_acc_info.outer_tpid = 0x8100;
  mep_acc_info.inner_vlan = 0;
  mep_acc_info.inner_pkt_pri = 0;     
  mep_acc_info.inner_tpid = 0;
  mep_acc_info.int_pri = 3 + (1<<2);
  mep_acc_info.sampling_ratio = sampling_ratio;
  if (device_type >= device_type_arad_plus) {
      /* Take RDI only from scanner*/
      mep_acc_info.flags2 =  BCM_OAM_ENDPOINT_FLAGS2_RDI_FROM_RX_DISABLE;

      if (use_port_interface_status) {
          mep_acc_info.flags |= BCM_OAM_ENDPOINT_PORT_STATE_UPDATE;
          mep_acc_info.port_state = BCM_OAM_PORT_TLV_UP;
      }
  }

  mep_acc_info.timestamp_format = get_oam_timestamp_format(unit);

  if (device_type < device_type_arad_plus) {
      src_mac_mep_2[5] = port_2; /* In devices older than Arad Plus the LSB of the src mac address must equal the local port. No such restriction in Arad+.*/
  }
  /* The MAC address that the CCM packets are sent with*/
  sal_memcpy(mep_acc_info.src_mac_address, src_mac_mep_2, 6);

  /*RX*/
  mep_acc_info.gport = gport2;
  sal_memcpy(mep_acc_info.dst_mac_address, mac_mep_2, 6);
  mep_acc_info.lm_counter_base_id = lm_counter_base_id_2;

  /* 
   * When OAM/BFD statistics enabled and MEP ACC is required to be counted, 
   * mep acc id 0 can't be used     
   */
  if (oam_up_statistics_enable) { 
      mep_acc_info.flags |= BCM_OAM_ENDPOINT_WITH_ID; 
      mep_acc_info.id = (device_type == device_type_qux) ? 1024 : 4096;
  }

  if (encoding) {
      printf("Encoding\n"); 
      mep_acc_info.flags |= BCM_OAM_ENDPOINT_WITH_ID;
      mep_acc_info.id = local_id_system;
      rv = system_endpoint_to_endpoint_id_oam(&mep_acc_info);
      if (rv != BCM_E_NONE){
          printf("Error, system_endpoint_to_endpoint_id_oam\n"); 
          print rv;
          return rv;
      }
  }

  printf("bcm_oam_endpoint_create mep_acc_info\n"); 
  rv = bcm_oam_endpoint_create(unit, &mep_acc_info);
  if (rv != BCM_E_NONE) {
	  printf("(%s) \n",bcm_errmsg(rv));
	  return rv;
  }
  printf("created MEP with id %d\n", mep_acc_info.id);

  ep2.gport = mep_acc_info.gport;
  ep2.id = mep_acc_info.id;

  /*
  * Adding Remote MEP
  */
  bcm_oam_endpoint_info_t_init(&rmep_info);
  rmep_info.name = 0xff;
  rmep_info.local_id = mep_acc_info.id;
  rmep_info.type = bcmOAMEndpointTypeEthernet;
  rmep_info.ccm_period = 0;
  rmep_info.flags |= BCM_OAM_ENDPOINT_REMOTE;
  rmep_info.loc_clear_threshold = 1;
  rmep_info.flags |= BCM_OAM_ENDPOINT_WITH_ID;
  rmep_info.id = mep_acc_info.id;
  if (device_type >= device_type_arad_plus) {
	  rmep_info.flags2 = BCM_OAM_ENDPOINT_FLAGS2_RDI_ON_RX_RDI | BCM_OAM_ENDPOINT_FLAGS2_RDI_ON_LOC;
      if (use_port_interface_status) {
          rmep_info.flags |= BCM_OAM_ENDPOINT_PORT_STATE_UPDATE;
          rmep_info.port_state = BCM_OAM_PORT_TLV_UP;
      }
  }

  if (encoding) {
      rmep_info.flags |= BCM_OAM_ENDPOINT_WITH_ID;
      
      rv = system_endpoint_to_endpoint_id_oam(&rmep_info);
      if (rv != BCM_E_NONE){
          printf("Error, system_endpoint_to_endpoint_id_oam\n"); 
          print rv;
          return rv;
      }
  }

  if (remote_meps_start_in_loc_enable) {
      rmep_info.faults |= BCM_OAM_ENDPOINT_FAULT_CCM_TIMEOUT;
      rmep_info.ccm_period = 1000;
  }

  printf("bcm_oam_endpoint_create RMEP\n");
  rv = bcm_oam_endpoint_create(unit, &rmep_info);
  if (rv != BCM_E_NONE) {
	  printf("(%s) \n",bcm_errmsg(rv));
	  return rv;
  }
  printf("created RMEP with id %d\n", rmep_info.id);

  ep2.rmep_id = rmep_info.id;

  /*
  * Adding acc MEP - downmep
  */

  bcm_oam_endpoint_info_t_init(&mep_acc_info);

  /*TX*/
  mep_acc_info.type = bcmOAMEndpointTypeEthernet;
  mep_acc_info.group = group_info->id;
  mep_acc_info.level = md_level_3;
  BCM_GPORT_SYSTEM_PORT_ID_SET(mep_acc_info.tx_gport, port_1);
  mep_acc_info.name = 456;
  mep_acc_info.ccm_period = 100;
  mep_acc_info.opcode_flags |= BCM_OAM_OPCODE_CCM_IN_HW;

  mep_acc_info.vlan = 10;
  mep_acc_info.pkt_pri = mep_acc_info.pkt_pri = 0 + (1<<1); /* dei(1bit) + (pcp(3bit) << 1)*/
  mep_acc_info.outer_tpid = 0x8100;
  mep_acc_info.inner_vlan = 0;
  mep_acc_info.inner_pkt_pri = 0;
  mep_acc_info.inner_tpid = 0;
  mep_acc_info.int_pri = 1 + (3<<2);

  if (device_type >= device_type_arad_plus) {
      /* Take RDI only from RX*/
      mep_acc_info.flags2 = BCM_OAM_ENDPOINT_FLAGS2_RDI_FROM_LOC_DISABLE ;

      if (use_port_interface_status) {
          mep_acc_info.flags |= BCM_OAM_ENDPOINT_INTERFACE_STATE_UPDATE;
          mep_acc_info.interface_state = BCM_OAM_INTERFACE_TLV_UP;
      }
  }

  if (device_type == device_type_arad_a0) {
      /* Arad A0 bug.*/
      src_mac_mep_3[5] = port_1;
  }

  /* The MAC address that the CCM packets are sent with*/
  sal_memcpy(mep_acc_info.src_mac_address, src_mac_mep_3, 6);

  /*RX*/
  mep_acc_info.gport = gport1;
  sal_memcpy(mep_acc_info.dst_mac_address, mac_mep_3, 6);
  mep_acc_info.lm_counter_base_id = lm_counter_base_id_1;
  mep_acc_info.timestamp_format = get_oam_timestamp_format(unit);

  printf("bcm_oam_endpoint_create mep_acc_info\n");
  rv = bcm_oam_endpoint_create(unit, &mep_acc_info);
  if (rv != BCM_E_NONE) {
	  printf("(%s) \n",bcm_errmsg(rv));
	  return rv;
  }
  printf("created MEP with id %d\n", mep_acc_info.id);

  ep3.gport = mep_acc_info.gport;
  ep3.id = mep_acc_info.id;

  /*
  * Adding Remote MEP
  */
  bcm_oam_endpoint_info_t_init(&rmep_info);
  rmep_info.name = 0x11;
  rmep_info.local_id = mep_acc_info.id;
  rmep_info.type = bcmOAMEndpointTypeEthernet;
  rmep_info.ccm_period = 0;
  rmep_info.flags |= BCM_OAM_ENDPOINT_REMOTE;
  rmep_info.loc_clear_threshold = 1;
  rmep_info.flags |= BCM_OAM_ENDPOINT_WITH_ID;
  rmep_info.id = mep_acc_info.id;
  if (device_type >= device_type_arad_plus) {
	  rmep_info.flags2 = BCM_OAM_ENDPOINT_FLAGS2_RDI_ON_RX_RDI | BCM_OAM_ENDPOINT_FLAGS2_RDI_ON_LOC;
      if (use_port_interface_status) {
          rmep_info.flags |= BCM_OAM_ENDPOINT_INTERFACE_STATE_UPDATE;
          rmep_info.interface_state = BCM_OAM_INTERFACE_TLV_UP;
      }
  }

  if (remote_meps_start_in_loc_enable) {
      rmep_info.faults |= BCM_OAM_ENDPOINT_FAULT_CCM_TIMEOUT;
      rmep_info.ccm_period = 1000;
  }

  printf("bcm_oam_endpoint_create RMEP\n");
  rv = bcm_oam_endpoint_create(unit, &rmep_info);
  if (rv != BCM_E_NONE) {
	  printf("(%s) \n",bcm_errmsg(rv));
	  return rv;
  }
  printf("created RMEP with id %d\n", rmep_info.id);

  ep3.rmep_id = rmep_info.id;

  if (use_port_interface_status) {
      /*
      * Adding acc MEP - downmep
      */

      bcm_oam_endpoint_info_t_init(&mep_acc_info);

      /*TX*/
      mep_acc_info.type = bcmOAMEndpointTypeEthernet;
      mep_acc_info.group = group_info->id;
      mep_acc_info.level = md_level_4;
      BCM_GPORT_SYSTEM_PORT_ID_SET(mep_acc_info.tx_gport, port_1);
      mep_acc_info.name = 789;
      mep_acc_info.ccm_period = 100;
      mep_acc_info.opcode_flags |= BCM_OAM_OPCODE_CCM_IN_HW;

      mep_acc_info.vlan = 10;
      mep_acc_info.pkt_pri = mep_acc_info.pkt_pri = 0 + (1<<1); /* dei(1bit) + (pcp(3bit) << 1)*/
      mep_acc_info.outer_tpid = 0x8100;
      mep_acc_info.inner_vlan = 0;
      mep_acc_info.inner_pkt_pri = 0;
      mep_acc_info.inner_tpid = 0;
      mep_acc_info.int_pri = 1 + (3<<2);
      mep_acc_info.timestamp_format = get_oam_timestamp_format(unit);

      if (device_type >= device_type_arad_plus) {
          /* Take RDI only from RX*/
          mep_acc_info.flags2 = BCM_OAM_ENDPOINT_FLAGS2_RDI_FROM_LOC_DISABLE ;

          if (use_port_interface_status) {
              mep_acc_info.flags |= BCM_OAM_ENDPOINT_INTERFACE_STATE_UPDATE|BCM_OAM_ENDPOINT_PORT_STATE_UPDATE;
              mep_acc_info.port_state = BCM_OAM_PORT_TLV_UP;
              mep_acc_info.interface_state = BCM_OAM_INTERFACE_TLV_UP;
          }
      }

      /* The MAC address that the CCM packets are sent with*/
      sal_memcpy(mep_acc_info.src_mac_address, src_mac_mep_4, 6);

      /*RX*/
      mep_acc_info.gport = gport2;
      sal_memcpy(mep_acc_info.dst_mac_address, mac_mep_4, 6);
      mep_acc_info.lm_counter_base_id = lm_counter_base_id_2;

      printf("bcm_oam_endpoint_create mep_acc_info\n");
      rv = bcm_oam_endpoint_create(unit, &mep_acc_info);
      if (rv != BCM_E_NONE) {
          printf("(%s) \n",bcm_errmsg(rv));
          return rv;
      }

      printf("created MEP with id %d\n", mep_acc_info.id);
	  ep4.gport = mep_acc_info.gport;
      ep4.id = mep_acc_info.id;

      /*
      * Adding Remote MEP
      */
      bcm_oam_endpoint_info_t_init(&rmep_info);
      rmep_info.name = 0x12;
      rmep_info.local_id = mep_acc_info.id;
      rmep_info.type = bcmOAMEndpointTypeEthernet;
      rmep_info.ccm_period = 0;
      rmep_info.flags |= BCM_OAM_ENDPOINT_REMOTE;
      rmep_info.loc_clear_threshold = 1;
      rmep_info.flags |= BCM_OAM_ENDPOINT_WITH_ID;
      rmep_info.id = mep_acc_info.id;
      if (device_type >= device_type_arad_plus) {
          rmep_info.flags2 = BCM_OAM_ENDPOINT_FLAGS2_RDI_ON_RX_RDI | BCM_OAM_ENDPOINT_FLAGS2_RDI_ON_LOC;
          if (use_port_interface_status) {
              rmep_info.flags |= BCM_OAM_ENDPOINT_INTERFACE_STATE_UPDATE|BCM_OAM_ENDPOINT_PORT_STATE_UPDATE;
              rmep_info.interface_state = BCM_OAM_INTERFACE_TLV_UP;
              rmep_info.port_state = BCM_OAM_PORT_TLV_UP;
          }
      }

      if (remote_meps_start_in_loc_enable) {
          rmep_info.faults |= BCM_OAM_ENDPOINT_FAULT_CCM_TIMEOUT;
          rmep_info.ccm_period = 1000;
      }

      printf("bcm_oam_endpoint_create RMEP\n");
      rv = bcm_oam_endpoint_create(unit, &rmep_info);
      if (rv != BCM_E_NONE) {
          printf("(%s) \n",bcm_errmsg(rv));
          return rv;
      }
      printf("created RMEP with id %d\n", rmep_info.id);
  }

  return rv;
}






/*****************************************************************************/
/*                                        OAM  MIP BASIC EXAMPLE                                                                        */
/*****************************************************************************/


int create_mip(int unit, int md_level_mip, bcm_oam_group_t group, bcm_gport_t gport, bcm_mac_t dst_mac) {
  bcm_error_t rv;
  /*
  * Adding a MIP
  */

  bcm_oam_endpoint_info_t_init(&mip_info);
  mip_info.type = bcmOAMEndpointTypeEthernet;
  mip_info.group = group;
  mip_info.level = md_level_mip;
  mip_info.gport = gport;
  mip_info.name = 0;     
  mip_info.ccm_period = 0;
  mip_info.flags |= BCM_OAM_ENDPOINT_INTERMEDIATE;
  sal_memcpy(mip_info.dst_mac_address, dst_mac, 6);

  printf("bcm_oam_endpoint_create mip_info\n"); 
  rv = bcm_oam_endpoint_create(unit, &mip_info);
  if (rv != BCM_E_NONE) {
	  printf("(%s) \n",bcm_errmsg(rv));
	  return rv;
  }
  printf("created MEP with id %d\n", mip_info.id);

  ep3.gport = mip_info.gport;
  ep3.id = mip_info.id;

  return rv;
}

/**
 * Create OAM group and MIP
 * 
 * 
 * @param unit 
 * 
 * @return int 
 */
int oam_create_mip_with_defaults (int unit) {
  int rv;

  printf("Creating group\n");
  bcm_oam_group_info_t_init(&group_info_short_ma);
  sal_memcpy(group_info_short_ma.name, short_name, BCM_OAM_GROUP_NAME_LENGTH);
  rv = bcm_oam_group_create(unit, &group_info_short_ma);
  if (rv != BCM_E_NONE) {
	  printf("(%s) \n",bcm_errmsg(rv));
	  return rv;
  }

  rv = create_mip(unit, md_level_mip, group_info_short_ma.id, gport2, mac_mep_2);
  if (rv != BCM_E_NONE) {
	  printf("(%s) \n",bcm_errmsg(rv));
	  return rv;
  }

  return rv;
}



/**
 * Equivalent to oam_run_with_defaults, but only creates a MIP.
 * MIP will be created on port2.
 * 
 * @param unit 
 * @param port1 
 * @param port2 
 * @param port3 
 * 
 * @return int 
 */
int oam_mip_only_run_with_defaults (int unit, int port1, int port2, int port3) {
  bcm_error_t rv;

  single_vlan_tag = 1;

  port_1 = port1;
  port_2 = port2;
  port_3 = port3;
  
  rv = oam_initialize_mip_settings(unit);
  if (rv != BCM_E_NONE) {
	  printf("(%s) \n",bcm_errmsg(rv));
	  return rv;
  }

  rv = oam_create_mip_with_defaults(unit);
   if (rv != BCM_E_NONE) {
	  printf("(%s) \n",bcm_errmsg(rv));
	  return rv;
  }

  return 0;
}


/**
 * Initialize settings and global variables for creating a MIP.
 * 
 * @param unit 
 * 
 * @return int 
 */
int oam_initialize_mip_settings(int unit) {
  bcm_error_t rv;

  int md_level_1 = 4;
  int md_level_2 = 5;
  int md_level_3 = 2;
  int lm_counter_base_id_1  = 5;
  int lm_counter_base_id_2  = 6;

  rv = create_vswitch_and_mac_entries(unit);
  if (rv != BCM_E_NONE) {
	  printf("(%s) \n",bcm_errmsg(rv));
	  return rv;
  }



  return rv;
}



/*****************************************************************************/
/*                                        OAM API WRAPPER FUNCTIONS AND EXAMPLES                                         */
/*****************************************************************************/


/*
 * default_ep_example
 *
 * This example uses the OAM default profile to create an upmep with MDL=5
 * and a downmep with MDL=3 for ports with OAM trap profile = 1.
 * If the inlif profiles are in simple mode (indicated by advanced_mode=0),
 * then port1's inlif profile bits allocated to the OAM trap profile are modified to 0x1
 * In advanced mode, a mapping is done between inlif-profile=5 -> oam-trap-profile=1 and
 * port1's inlif profile is set to 5.
 */
int default_ep_example(int unit, int port1, int port2, int port3, int advanced_mode) {
  bcm_error_t rv;
  bcm_oam_endpoint_info_t default_info;

  single_vlan_tag = 1;

  port_1 = port1;
  port_2 = port2;
  port_3 = port3;

  printf("create_vswitch_and_mac_entries\n");
  rv = create_vswitch_and_mac_entries(unit);
  if (rv != BCM_E_NONE) {
	  printf("(%s) \n",bcm_errmsg(rv));
	  return rv;
  }

  printf("Add default up mep (EGRESS0)\n");
  bcm_oam_endpoint_info_t_init(&default_info);
  default_info.id = BCM_OAM_ENDPOINT_DEFAULT_EGRESS0;
  default_info.flags |= BCM_OAM_ENDPOINT_WITH_ID|BCM_OAM_ENDPOINT_UP_FACING;
  default_info.level = 5;
  default_info.timestamp_format = get_oam_timestamp_format(unit);


  rv = bcm_oam_endpoint_create(unit, &default_info);
  if (rv != BCM_E_NONE) {
      printf("(%s) \n",bcm_errmsg(rv));
      return rv;
  }

  if (advanced_mode) {
      printf("Set LIF profile for port: %d\n", port1);
      rv = bcm_port_class_set(unit, gport1, bcmPortClassFieldIngress, 5 /* InLIF profile.*/);
      if (rv != BCM_E_NONE) {
          printf("(%s) \n",bcm_errmsg(rv));
          return rv;
      }
      printf("Set mapping from inlif profile 5 to OAM trap profile 1\n", port1);
  }
  else {
      printf("Set LIF profile for port: %d\n", port1);
  }
  rv = bcm_port_control_set(unit, gport1, bcmPortControlOamDefaultProfile, 1/*OAM trap profile */ );
  if (rv != BCM_E_NONE) {
      printf("(%s) \n",bcm_errmsg(rv));
      return rv;
  }

  printf("Add default down mep (INGRESS1)\n");
  bcm_oam_endpoint_info_t_init(&default_info);
  default_info.id = BCM_OAM_ENDPOINT_DEFAULT_INGRESS1;
  default_info.flags |= BCM_OAM_ENDPOINT_WITH_ID;
  default_info.level = 3;
  default_info.timestamp_format = get_oam_timestamp_format(unit);

  rv = bcm_oam_endpoint_create(unit, &default_info);
  if (rv != BCM_E_NONE) {
      printf("(%s) \n",bcm_errmsg(rv));
      return rv;
  }

  return rv;
}


/**
 * Set a default MEP on the egress. Applies to Jericho B and 
 * above. 
 * Note that by default all outLIFs use oam-outlif-profile 0. In
 * this example configure a specific outlif to have profile 
 * oam-outlif-profile and then define a MEP on 
 * oam-outlif-profile 1. 
 * 
 * @author sinai (01/11/2015)
 * 
 * @param unit 
 * @param port1 
 * @param port2 
 * @param port3 
 * 
 * @return int 
 */
int default_up_ep_example(int unit, int port1, int port2, int port3) {
    bcm_error_t rv;
    single_vlan_tag = 1;
    int oam_egress_profile = 2;
    bcm_oam_endpoint_info_t default_info;

    port_1 = port1;
    port_2 = port2;
    port_3 = port3;

    rv = get_device_type(unit, &device_type);
    if (rv < 0) {
        printf("Error checking whether the device is arad+.\n");
        print rv;
        return rv;
    }

    if (device_type < device_type_jericho_b) {
        print "Cint available for Jericho/QMX B0 and above";
        return 234;
    }

    rv = create_vswitch_and_mac_entries(unit);
    if (rv != BCM_E_NONE) {
        printf("(%s) \n", bcm_errmsg(rv));
        return rv;
    }

    /* Tie gport2, gport1 to outlif profile 2.*/
    rv = bcm_port_control_set(unit, gport1, bcmPortControlOamDefaultProfileEgress, oam_egress_profile);
    if (rv != BCM_E_NONE) {
        printf("(%s) \n", bcm_errmsg(rv));
        return rv;
    }
    rv = bcm_port_control_set(unit, gport2, bcmPortControlOamDefaultProfileEgress, oam_egress_profile);
    if (rv != BCM_E_NONE) {
        printf("(%s) \n", bcm_errmsg(rv));
        return rv;
    }

    
    /*  Now Configure an endpoint on oam-outLIF profile 2.*/
    printf("Add default up mep (EGRESS0)\n");
    bcm_oam_endpoint_info_t_init(&default_info);
    default_info.id = BCM_OAM_ENDPOINT_DEFAULT_EGRESS2;
    default_info.flags |= BCM_OAM_ENDPOINT_WITH_ID | BCM_OAM_ENDPOINT_UP_FACING;
    default_info.level = 5;
    default_info.timestamp_format = get_oam_timestamp_format(unit);


    rv = bcm_oam_endpoint_create(unit, &default_info);
    if (rv != BCM_E_NONE) {
        printf("(%s) \n", bcm_errmsg(rv));
        return rv;
    }
	
	return 0;

}

/**
 * Function creates an accelerated endpoint.
 *  group_info_short_ma  must be initialized before this
 *  function is called.
 *  Likewise the function create_vswitch_and_mac_entries() must
 *  also  be called before this function.
 *  
 * @author sinai (26/11/2013)
 * 
 * @param unit 
 * @param is_up - direction of the endpoint.
 * @param ts_format - one of bcmOAMTimestampFormatNTP or 
 *                  bcmOAMTimestampFormatIEEE1588v1
 * 
 * @return int 
 */
int create_acc_endpoint(int unit , int is_up, bcm_oam_timestamp_format_t ts_format) {
    bcm_oam_endpoint_info_t  acc_endpoint;
    int counter_base_id;
    int rv;

    /** Down*/
   bcm_oam_endpoint_info_t_init(&acc_endpoint);

  /*TX*/
  acc_endpoint.type = bcmOAMEndpointTypeEthernet;
  acc_endpoint.group = group_info_short_ma.id;
  acc_endpoint.opcode_flags |= BCM_OAM_OPCODE_CCM_IN_HW;
  acc_endpoint.outer_tpid = 0x8100;
  acc_endpoint.timestamp_format = ts_format;


  if (is_up) {
        /*TX*/
      rv = set_counter_source_and_engines(unit,&counter_base_id,port_2);
      BCM_IF_ERROR_RETURN(rv);
      acc_endpoint.lm_counter_base_id = counter_base_id;

      acc_endpoint.level = 5;
      acc_endpoint.name = 123;
      acc_endpoint.flags |= BCM_OAM_ENDPOINT_UP_FACING;

      acc_endpoint.vlan = 5;
      acc_endpoint.pkt_pri = 0 + (2<<1); /* dei(1bit) + (pcp(3bit) << 1)*/
      acc_endpoint.outer_tpid = 0x8100;
      acc_endpoint.int_pri = 3 + (1<<2);
      /* The MAC address that the CCM packets are sent with*/
      src_mac_mep_2[5] = port_2;
      sal_memcpy(acc_endpoint.src_mac_address, src_mac_mep_2, 6);
      /*RX*/
      acc_endpoint.gport = gport2;
      sal_memcpy(acc_endpoint.dst_mac_address, mac_mep_2, 6);
      acc_endpoint.tx_gport = BCM_GPORT_INVALID;
  } else { /** Down*/
      rv = set_counter_source_and_engines(unit,&counter_base_id,port_1);
      BCM_IF_ERROR_RETURN(rv);
      acc_endpoint.lm_counter_base_id = counter_base_id;

      acc_endpoint.level = 2;
      BCM_GPORT_SYSTEM_PORT_ID_SET(acc_endpoint.tx_gport, port_1);
      acc_endpoint.name = 456;
      acc_endpoint.ccm_period = 100;
      acc_endpoint.vlan = 10;
      acc_endpoint.pkt_pri = 0 + (1<<1); /* dei(1bit) + (pcp(3bit) << 1)*/
      acc_endpoint.int_pri = 1 + (3<<2);
      /* The MAC address that the CCM packets are sent with*/
      sal_memcpy(acc_endpoint.src_mac_address, src_mac_mep_3, 6);

      /*RX*/
      acc_endpoint.gport = gport1;
      sal_memcpy(acc_endpoint.dst_mac_address, mac_mep_3, 6);
  }

  rv = bcm_oam_endpoint_create(unit, &acc_endpoint);
  if (rv != BCM_E_NONE) {
      printf("(%s) \n", bcm_errmsg(rv));
      return rv;
  }
  if (is_up) { 
      acc_ep_up.id = acc_endpoint.id;
  } else { /* Down*/
      acc_ep_down.id = acc_endpoint.id;
  }

  return rv; 
}

/**
 * OAM_endpoint_action_set calling sequence example.
 * 
 * @param unit 
 * @param port - Must be BCM_GPORT_INVALID for actions requiring
 *             invalid gport. otherwise, trap
 * @param endpoint_id 
 * @param action_type 
 * @param opcode - OAM opcode upon which action will be applied
 * 
 * @return int 
 */
int oam_action_set(int unit, int dest_port, int endpoint_id, bcm_oam_action_type_t action_type, int opcode) {
    bcm_error_t rv;
    bcm_oam_endpoint_action_t action;
    int trap_code, trap_code2;
    bcm_rx_trap_config_t trap_config;
    bcm_rx_trap_t trap_type;
    bcm_oam_endpoint_info_t endpoint_info;

    bcm_oam_endpoint_action_t_init(&action);

    action.destination = dest_port;
    if (dest_port != BCM_GPORT_INVALID && !BCM_GPORT_IS_TRAP(dest_port)) {
        /* action.destination can only receive trap as destination. Allocate new trap */
        trap_type = bcmRxTrapUserDefine;

        /* if MEP is up, Different trap type is used to prevent two sets of system headers */
        rv = bcm_oam_endpoint_get(unit, endpoint_id, &endpoint_info);
        if (rv != BCM_E_NONE) {
            printf("(%s) \n", bcm_errmsg(rv));
            return rv;
        }
        if (endpoint_info.flags & BCM_OAM_ENDPOINT_UP_FACING) {
            trap_type = bcmRxTrapOamUpMEP4;
            /* This trap may already been created */
            if (gTrapUp4AlreadyCreated) {
                rv = bcm_rx_trap_type_get(unit, 0, trap_type, &trap_code);
            } else {
                gTrapUp4AlreadyCreated = 1;
                rv = bcm_rx_trap_type_create(unit, 0, trap_type, &trap_code);
            }
            if (rv != BCM_E_NONE) {
                printf("(%s) \n", bcm_errmsg(rv));
                return rv;
            }
        } else {
            rv = bcm_rx_trap_type_create(unit, 0, trap_type, &trap_code);
            if (rv != BCM_E_NONE) {
                printf("(%s) \n", bcm_errmsg(rv));
                return rv;
            }
        }

        bcm_rx_trap_config_t_init(&trap_config);
        trap_config.flags = BCM_RX_TRAP_UPDATE_DEST | BCM_RX_TRAP_TRAP;
        rv = port_to_system_port(unit, dest_port, &trap_config.dest_port);
        if (rv != BCM_E_NONE) {
            printf("(%s) \n", bcm_errmsg(rv));
            return rv;
        }

        rv = bcm_rx_trap_set(unit, trap_code, trap_config);
        if (rv != BCM_E_NONE) {
            printf("(%s) \n", bcm_errmsg(rv));
            return rv;
        }

        printf("Trap set, trap_code = %d \n", trap_code);
        BCM_GPORT_TRAP_SET(action.destination, trap_code, 7, 0);

        /* If MIP, two traps shoud be used. One for each direction. Setting the egress-trap, ingress-trap was set above */
        if (endpoint_info.flags & BCM_OAM_ENDPOINT_INTERMEDIATE) {
            trap_type = bcmRxTrapOamUpMEP4;

            /* This trap may already been created */
            if (gTrapUp4AlreadyCreated) {
                rv = bcm_rx_trap_type_get(unit, 0, trap_type, &trap_code2);
            } else {
                gTrapUp4AlreadyCreated = 1;
                rv = bcm_rx_trap_type_create(unit, 0, trap_type, &trap_code2);
            }
            if (rv != BCM_E_NONE) {
                printf("(%s) \n", bcm_errmsg(rv));
                return rv;
            }

            rv = bcm_rx_trap_set(unit, trap_code2, trap_config);
            if (rv != BCM_E_NONE) {
                printf("(%s) \n", bcm_errmsg(rv));
                return rv;
            }

            printf("Trap set, trap_code2 = %d \n", trap_code2);
            BCM_GPORT_TRAP_SET(action.destination2, trap_code2, 7, 0);
        }
    }

    BCM_OAM_OPCODE_SET(action, opcode);
    BCM_OAM_ACTION_SET(action, action_type);
    if (count_enable_on_oam_action) {
        BCM_OAM_ACTION_SET(action, bcmOAMActionCountEnable);
    }

    rv = bcm_oam_endpoint_action_set(unit, endpoint_id, &action);
    printf( "Action set created\n");

    return rv;
}

/**
 * Action profile for packets that causes trap bcmRxTrapOamPassive 
 *
 * This is an example script to assign action for passive demultiplexing.
 * Calling this API sets the global behavior of passive demultiplexing. 
 * For drop action, use inParam port = BCM_GPORT_TYPE_BLACK_HOLE
 *
 * @author Neeraj (06/08/2016)
 * 
 * @param unit 
 * @param port - Destination
 * @return int 
 */
int configure_oam_passive_level_trap(int unit, bcm_port_t port) {

    int rv;
    int trap_code;
    bcm_rx_trap_config_t  oam_trap_config;
    int trap_port;

    /* First, get the trap code used for OAM traps*/
    rv = bcm_rx_trap_type_get(unit,0/* flags */ , bcmRxTrapOamPassive, &trap_code);
    if (rv != BCM_E_NONE) {
        printf("(%s) \n",bcm_errmsg(rv));
    }

    /* Now the trap config. */
    rv = bcm_rx_trap_get(unit,trap_code, &oam_trap_config);
    if (rv != BCM_E_NONE) {
        printf("(%s) \n",bcm_errmsg(rv));
    }
    oam_trap_config.flags |= BCM_RX_TRAP_TRAP;

    /* Update the destination*/
    oam_trap_config.dest_port = port;

    /* Now update the trap config with the new destination*/
    rv = bcm_rx_trap_set(unit,trap_code, oam_trap_config);
    if (rv != BCM_E_NONE) {
        printf("(%s) \n",bcm_errmsg(rv));
    }
    return 0;
}
/**
 * Example shows how to trap CCMs to the CPU for MIPs.
 * (By default CCMs are forwarded). 
 *  
 * @author sinai (15/03/2016)
 * 
 * @param unit 
 * @param endpoint_id -assumed to be a MIP
 * 
 * @return int 
 */
int oam_action_set_for_mips_ccm(int unit, int endpoint_id) {
    bcm_error_t rv;
    bcm_oam_endpoint_action_t action;
    int trap_code, trap_code2;
    bcm_rx_trap_config_t trap_config;
    bcm_rx_trap_t trap_type;
    bcm_oam_endpoint_info_t endpoint_info;

    bcm_oam_endpoint_action_t_init(&action);

  /* First, get the trap code used for OAM traps*/
    rv = bcm_rx_trap_type_get(unit,0/* flags */ ,bcmRxTrapBfdOamDownMEP, &trap_code);
    if (rv != BCM_E_NONE) {
        printf("(%s) \n",bcm_errmsg(rv));
    }

    /* Now the same for Up direction*/
    rv = bcm_rx_trap_type_get(unit,0 ,bcmRxTrapOamUpMEP, &trap_code2);
    if (rv != BCM_E_NONE) {
        printf("(%s) \n",bcm_errmsg(rv));
    }

    /* Now set the parameters.*/
    BCM_GPORT_TRAP_SET(action.destination2, trap_code2, 7, 0);
    BCM_GPORT_TRAP_SET(action.destination, trap_code, 7, 0);

    BCM_OAM_OPCODE_SET(action, bcmOamOpcodeCCM);
    BCM_OAM_ACTION_SET(action, bcmOAMActionMcFwd);
    rv = bcm_oam_endpoint_action_set(unit, endpoint_id, &action);
    printf( "Action set created\n");

    return rv;
}

/* This is an example of using bcm_oam_event_register api.
 * A simple callback is created for CCM timeout event. 
 * After a mep and rmep are created, the callback is called 
 * whenever CCMTimeout event is generated. 
 */
int cb_oam(int unit,
           uint32 flags,
           bcm_oam_event_type_t event_type,
           bcm_oam_group_t group,
           bcm_oam_endpoint_t endpoint,
           void *user_data) {
    int et = event_type, gp = group, ep = endpoint;
    printf("UNIT: %d, FLAGS: %d, EVENT: %d, GROUP: %d, MEP: 0x%0x\n",unit,flags,et,gp,ep);

    if ((event_type == bcmOAMEventEndpointInterfaceDown) || (event_type == bcmOAMEventEndpointPortDown)) {
        port_interface_event_count++;
    } else {
        timeout_events_count++;
    }

    if (flags & BCM_OAM_EVENT_FLAGS_MULTIPLE) {
        timeout_events_count_multi_oam[endpoint & 0xff]++;
    }

    if (event_type == bcmOAMEventEndpointRemote || event_type == bcmOAMEventEndpointRemoteUp) {
        remote_event_count++;
    }

    return BCM_E_NONE;
}

int register_events(int unit) {
  bcm_error_t rv;
  bcm_oam_event_types_t timeout_event, timein_event, port_interface_event;

  BCM_OAM_EVENT_TYPE_SET(timeout_event, bcmOAMEventEndpointCCMTimeout);
  rv = bcm_oam_event_register(unit, timeout_event, cb_oam, (void*)1);
  if (rv != BCM_E_NONE) {
	  printf("(%s) \n",bcm_errmsg(rv));
	  return rv;
  }

  BCM_OAM_EVENT_TYPE_SET(timein_event, bcmOAMEventEndpointCCMTimein);
  rv = bcm_oam_event_register(unit, timein_event, cb_oam, (void*)2);
  if (rv != BCM_E_NONE) {
	  printf("(%s) \n",bcm_errmsg(rv));
	  return rv;
  }

  BCM_OAM_EVENT_TYPE_SET(timein_event, bcmOAMEventEndpointRemote);
  rv = bcm_oam_event_register(unit, timein_event, cb_oam, (void*)3);
  if (rv != BCM_E_NONE) {
	  printf("(%s) \n",bcm_errmsg(rv));
	  return rv;
  }

  BCM_OAM_EVENT_TYPE_SET(timein_event, bcmOAMEventEndpointRemoteUp);
  rv = bcm_oam_event_register(unit, timein_event, cb_oam, (void*)4);
  if (rv != BCM_E_NONE) {
	  printf("(%s) \n",bcm_errmsg(rv));
	  return rv;
  }

  BCM_OAM_EVENT_TYPE_CLEAR_ALL(port_interface_event);
  BCM_OAM_EVENT_TYPE_SET(port_interface_event, bcmOAMEventEndpointPortDown);
  rv = bcm_oam_event_register(unit, port_interface_event, cb_oam, (void*)5);
  if (rv != BCM_E_NONE) {
      printf("(%s) \n",bcm_errmsg(rv));
      return rv;
  }

  BCM_OAM_EVENT_TYPE_CLEAR_ALL(port_interface_event);
  BCM_OAM_EVENT_TYPE_SET(port_interface_event, bcmOAMEventEndpointInterfaceDown);
  rv = bcm_oam_event_register(unit, port_interface_event, cb_oam, (void*)6);
  if (rv != BCM_E_NONE) {
      printf("(%s) \n",bcm_errmsg(rv));
      return rv;
  }

  return rv;
}



/**
 * Snoop OAM packets by endpoint and opcode to given 
 * destination. 
 * Note: Only applies for enbpoints. For MIPs two traps with two 
 * consecutive trap codes must be allocated, one for the ingress 
 * and another for the egress. 
 * 
 * 
 * @param unit 
 * @param dest_snoop_port 
 * @param endpoint_id 
 * @param action_type 
 * @param opcode 
 * 
 * @return int 
 */
int oam_change_mep_destination_to_snoop(int unit, int dest_snoop_port, int endpoint_id, bcm_oam_action_type_t action_type, int opcode) {
    bcm_error_t rv;
    bcm_rx_trap_config_t trap_config_snoop;
    bcm_rx_snoop_config_t snoop_config_cpu;
    int snoop_cmnd;
    int trap_code;
    bcm_gport_t gport;

    rv = bcm_rx_snoop_create(unit, 0, &snoop_cmnd);
    if (rv != BCM_E_NONE) {
       printf("(%s) \n",bcm_errmsg(rv));
       return rv;
    }

    bcm_rx_snoop_config_t_init(&snoop_config_cpu);
    snoop_config_cpu.flags = (BCM_RX_SNOOP_UPDATE_DEST | BCM_RX_SNOOP_UPDATE_PRIO);
    snoop_config_cpu.dest_port = dest_snoop_port;
    snoop_config_cpu.size = -1;
    snoop_config_cpu.probability = 100000;

    rv =  bcm_rx_snoop_set(unit, snoop_cmnd, &snoop_config_cpu);
    if (rv != BCM_E_NONE) {
       printf("(%s) \n",bcm_errmsg(rv));
       return rv;
    }

    rv =  bcm_rx_trap_type_create(unit, 0, bcmRxTrapUserDefine, &trap_code);
    if (rv != BCM_E_NONE) {
       printf("(%s) \n",bcm_errmsg(rv));
       return rv;
    }
    printf("Trap created trap_code=%d \n",trap_code);

    bcm_rx_trap_config_t_init(&trap_config_snoop);
    trap_config_snoop.flags = 0;
    trap_config_snoop.trap_strength = 0;
    trap_config_snoop.snoop_cmnd = snoop_cmnd;

    rv = bcm_rx_trap_set(unit, trap_code, trap_config_snoop);
    if (rv != BCM_E_NONE) {
       printf("(%s) \n",bcm_errmsg(rv));
       return rv;
    }

    BCM_GPORT_TRAP_SET(gport, trap_code, 7, 3); /*Taken from default values*/

    bcm_oam_endpoint_action_t action;
    bcm_oam_endpoint_action_t_init(&action);
    action.destination = gport;
    BCM_OAM_OPCODE_SET(action, opcode); /*1-CCM*/
    BCM_OAM_ACTION_SET(action, action_type);  /*bcmOAMActionMcFwd,bcmOAMActionUcFwd*/
    rv =  bcm_oam_endpoint_action_set(unit, endpoint_id, &action);
    if (rv != BCM_E_NONE) {
       printf("(%s) \n",bcm_errmsg(rv));
       return rv;
   }

   return rv;
}

/**
 * Count SLM packets for a given endpoint. 
 * This API does the following: 
 * 1)For the LIF on which the given endpoint resides data 
 * packets will not be counted 
 * 2)SLM, SLR, LMM, and LMR packets will be counted for ALL 
 * LIFs. 
 * 
 * @param unit 
 * @param endpoint_id 
 * 
 * @return int 
 */
int slm_set(int unit, int endpoint_id)  {
    bcm_error_t rv;
    bcm_oam_endpoint_action_t action;

    bcm_oam_endpoint_action_t_init(&action);
    action.destination = BCM_GPORT_INVALID;
    BCM_OAM_OPCODE_SET(action, 55); /*SLM*/
    BCM_OAM_OPCODE_SET(action, 54); /*SLR*/

    /* Note that as long is the user requires to count SLM packets the action
       bcmOAMActionSLMEnable Should be added for all subsequnt calls to bcm_oam_endpoint_action_set*/
    BCM_OAM_ACTION_SET(action, bcmOAMActionSLMEnable);
    BCM_OAM_ACTION_SET(action, bcmOAMActionCountEnable);
    rv =  bcm_oam_endpoint_action_set(unit, endpoint_id, &action);
    if (rv != BCM_E_NONE) {
       printf("(%s) \n",bcm_errmsg(rv));
       return rv;
   }

   return rv;
}



/* Arad/Arad+ hard coded values */
int snoop_up_mip = 2;
int snoop_down_mip = 1;


/**
 * Function changes the MIP snoop destination for both 
 * directions (up and down). 
 * Function assumes the soc property "egress_snooping_advanced" 
 * is set to 1. 
 * 
 * @author sinai (22/01/2014)
 * 
 * @param unit 
 * @param dest_snoop_port - destination for both directions.
 * @param mip_endpoint_id 
 * @param action_type 
 * @param opcode 
 * 
 * @return int 
 */
int mip_egress_snooping_advanced(int unit, int dest_snoop_port, int mip_endpoint_id, bcm_oam_action_type_t action_type, int opcode) {
    bcm_error_t rv;
    bcm_rx_trap_config_t trap_config_snoop;
    bcm_rx_snoop_config_t snoop_config_cpu;
    int snoop_cmnd = snoop_down_mip; /* Arad/Arad+ only - higher devices need to create a snoop command */
    int trap_code;
    bcm_gport_t gport;

    bcm_rx_snoop_config_t_init(&snoop_config_cpu);
    snoop_config_cpu.flags = (BCM_RX_SNOOP_UPDATE_DEST | BCM_RX_SNOOP_UPDATE_PRIO);
    snoop_config_cpu.dest_port = dest_snoop_port;
    snoop_config_cpu.size = -1;
    snoop_config_cpu.probability = 100000;

    rv = bcm_rx_snoop_set(unit, snoop_cmnd, &snoop_config_cpu);
    if (rv != BCM_E_NONE) {
        printf("(%s) \n", bcm_errmsg(rv));
        return rv;
    }

    /* create ONLY the trap for the down direction */
    rv = bcm_rx_trap_type_create(unit, 0, bcmRxTrapUserDefine, &trap_code);
    if (rv != BCM_E_NONE) {
        printf("(%s) \n", bcm_errmsg(rv));
        return rv;
    }
    printf("Trap created trap_code=%d \n", trap_code);

    bcm_rx_trap_config_t_init(&trap_config_snoop);
    trap_config_snoop.flags = 0;
    trap_config_snoop.trap_strength = 0; 
    trap_config_snoop.snoop_cmnd = snoop_cmnd;

    rv = bcm_rx_trap_set(unit, trap_code, trap_config_snoop);
    if (rv != BCM_E_NONE) {
        printf("(%s) \n", bcm_errmsg(rv));
        return rv;
    }

    BCM_GPORT_TRAP_SET(gport, trap_code, 7, 3); /*Taken from default values*/

    bcm_oam_endpoint_action_t action;
    bcm_oam_endpoint_action_t_init(&action);
    action.destination = gport;
    /* For Arad+ Advanced egress snooping destination2 must not be set
       even though the endpoint is a MIP. Anyway both directions will
       be configured. This is a special case. */
    action.destination2 = BCM_GPORT_INVALID;
    BCM_OAM_OPCODE_SET(action, opcode);
    BCM_OAM_ACTION_SET(action, action_type); /*bcmOAMActionMcFwd,bcmOAMActionUcFwd*/
    rv = bcm_oam_endpoint_action_set(unit, mip_endpoint_id, &action);
    if (rv != BCM_E_NONE) {
       printf("(%s) \n",bcm_errmsg(rv));
       return rv;
   }

   return rv;
}

/**
 * Jericho and above only. 
 * Function changes the MIP to snoop OAM PDU types (by opcode) 
 * for both directions (up and down). The function also 
 * demonstrate re-configuring the snoop action by setting a 
 * different snoop port if dest_snoop_port!=BCM_GPORT_INVALID 
 *
 * @param unit
 * @param dest_snoop_port
 * @param mip_endpoint_id
 * @param action_type
 * @param opcode
 *
 * @return int
 */
int mip_set_snoop_by_opcode(int unit, int dest_snoop_port, int mip_endpoint_id, bcm_oam_action_type_t action_type, int opcode) {
    bcm_error_t rv;
    bcm_oam_endpoint_action_t action;
    int trap_id, trap_id_up;

    /* Get trap-code for oam snooped packets */
    rv = bcm_rx_trap_type_get(unit,0,bcmRxTrapSnoopOamPacket,&trap_id);
    if (rv != BCM_E_NONE) {
        printf("(%s) \n", bcm_errmsg(rv));
        return rv;
    }

    /* Now the same for Up direction */
    rv = bcm_rx_trap_type_get(unit,0 ,bcmRxTrapOamUpMEP3, &trap_id_up);
    if (rv != BCM_E_NONE) {
        printf("(%s) \n",bcm_errmsg(rv));
    }

    if (dest_snoop_port != BCM_GPORT_INVALID) {
        /* If the dest_snoop_port is valid, reconfigure the snoop command */

        bcm_rx_trap_config_t trap_config, trap_config_up;
        bcm_rx_snoop_config_t snoop_config;
        int snoop_cmd;

        /* First, retreive the trap configuration using the trap-code */
        rv = bcm_rx_trap_get(unit, trap_id, &trap_config);
        if (rv != BCM_E_NONE) {
            printf("(%s) \n", bcm_errmsg(rv));
            return rv;
        }

        /* The trap is connected to a snoop command. Get the snoop command ID from the trap configuration */
        snoop_cmd = trap_config.snoop_cmnd;

        /* Retrieve the snoop command configuration */
        rv = bcm_rx_snoop_get(unit, snoop_cmd, &snoop_config);
        if (rv != BCM_E_NONE) {
            printf("(%s) \n", bcm_errmsg(rv));
            return rv;
        }

        /* Modify the neccesary field */
        snoop_config.dest_port = dest_snoop_port;

        /* Write the modified configuration back */
        rv = bcm_rx_snoop_set(unit, snoop_cmd, &snoop_config);
        if (rv != BCM_E_NONE) {
            printf("(%s) \n", bcm_errmsg(rv));
            return rv;
        }

        /* Using the same trap configuration for Up MEP as Down MEP*/
        /* QAX devices uses trapping instead of snooping for egress snooping.
           Configuring accordingly if that's the case. */
        /* Retreive the trap configuration using the up trap-code */
        rv = bcm_rx_trap_get(unit, trap_id_up, &trap_config_up);
        if (rv != BCM_E_NONE) {
            printf("(%s) \n", bcm_errmsg(rv));
            return rv;
        }

        if (!trap_config_up.snoop_cmnd) {
            /* No snoop command on trap. Device is QAX. Using trap instead of snoop. */
            trap_config_up.dest_port = dest_snoop_port;

            rv = bcm_rx_trap_set(unit, trap_id_up, trap_config_up);
        } else {
            rv = bcm_rx_trap_set(unit, trap_id_up, trap_config);
        }
        if (rv != BCM_E_NONE) {
            printf("(%s) \n",bcm_errmsg(rv));
        }
    }

    /* Set an action of trapping packets with the specified opcode */
    bcm_oam_endpoint_action_t_init(&action);
    BCM_GPORT_TRAP_SET(action.destination, trap_id, 7, 3);
    BCM_GPORT_TRAP_SET(action.destination2, trap_id_up, 7, 3);
    BCM_OAM_OPCODE_SET(action, opcode);
    BCM_OAM_ACTION_SET(action, action_type); /*bcmOAMActionMcFwd,bcmOAMActionUcFwd*/

    /* Set the action for the MIP */
    rv = bcm_oam_endpoint_action_set(unit, mip_endpoint_id, &action);
    if (rv != BCM_E_NONE) {
        printf("(%s) \n", bcm_errmsg(rv));
        return rv;
    }

    return rv;
}


/**
 * enable protection packets in addition to interrupts. Global
 * setting for all OAM. Function assumes OAM has been 
 * inititalized. 
 * Configure the protection header as raw data 
 *  
 * @author sinai (13/02/2014)
 * 
 * @param unit 
 * @param dest_port
 * 
 * @return int 
 */
int enable_oam_protection_packets_raw_header(int unit, int dest_port) {
    
    int rv;

    rv = get_device_type(unit, &device_type);
    if (rv < 0) {
        printf("Error checking whether the device is arad+.\n");
        print rv;
        return rv;
    }

    bcm_pkt_blk_t packet_header;
    bcm_pkt_blk_t_init(&packet_header);

    /* PTCH (2B) + ITMH (4B) + Internal header */
    if (device_type < device_type_jericho) {
        unsigned char arr_data[] = { 0x70,0xe8,0x40,0x0c,0x00,0xcb,0x17,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }; 
        packet_header.data=arr_data; 
    } else {
        unsigned char arr_data[] = { 0x70, 0xe8, 0x41, 0x00, 0xcb, 0x00, 0x17, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
        /* Encode Destination system-port into header to its place */
        arr_data[4] = dest_port&0xff;
        arr_data[3] = (dest_port&0xff00)>>8;
        packet_header.data=arr_data; 
    }
    
    packet_header.len=30;
 
    rv = bcm_oam_protection_packet_header_set(unit,&packet_header);
    if (rv != BCM_E_NONE){
        printf("Error, in bcm_oam_protection_packet_header_set\n");
        return rv;
    }

    return rv;
}

/**
 * enable protection packets in addition to interrupts. Global
 * setting for all OAM. Function assumes OAM has been 
 * inititalized. 
 *  
 * @author sinai (13/02/2014)
 * 
 * @param unit 
 * @param dest_port
 * 
 * @return int 
 */
int enable_oam_protection_packets(int unit, int dest_port) {
    bcm_rx_trap_config_t trap_config_protection;
    int trap_code=0x401; /* trap code on FHEI  will be (trap_code & 0xff), in this case 1.*/
    /* valid trap codes for oamp traps are 0x401 - 0x4ff */
    int rv;

    rv = bcm_rx_trap_type_create(unit, BCM_RX_TRAP_WITH_ID, bcmRxTrapOampProtection, &trap_code);
    if (rv != BCM_E_NONE) {
       printf("trap create: (%s) \n",bcm_errmsg(rv));
       return rv;
    }

    rv = port_to_system_port(unit, dest_port, &trap_config_protection.dest_port);
    if (rv != BCM_E_NONE){
        printf("Error, in port_to_system_port\n");
        return rv;
    }

    rv = bcm_rx_trap_set(unit, trap_code, trap_config_protection); 
    if (rv != BCM_E_NONE) {
       printf("trap set: (%s) \n",bcm_errmsg(rv));
       return rv;
    }

    return rv;
}
/**
 * sets trap for OAM MAID error to specific port
 *  
 * @author aviv (14/12/2014)
 * 
 * @param unit 
 * @param dest_port - should be local port.
 * 
 * @return int 
 */
int maid_trap_set(int unit, int dest_port_or_queue, int is_queue) {
	bcm_rx_trap_config_t trap_config;
	int trap_code=0x416; /* trap code on FHEI  will be (trap_code & 0xff), in this case 0x16.*/
	/* valid trap codes for oamp traps are 0x401 - 0x4ff */
	int rv;

	rv =  bcm_rx_trap_type_create(unit, BCM_RX_TRAP_WITH_ID, bcmRxTrapOampMaidErr, &trap_code);
    if (rv != BCM_E_NONE) {
       printf("trap create: (%s) \n",bcm_errmsg(rv));
       return rv;
   }

    if (is_queue) {
        BCM_GPORT_UNICAST_QUEUE_GROUP_SET(trap_config.dest_port, dest_port_or_queue);
    }
    else {
        BCM_GPORT_SYSTEM_PORT_ID_SET(trap_config.dest_port, dest_port_or_queue);
    }

	rv = bcm_rx_trap_set(unit, trap_code, trap_config); 
    if (rv != BCM_E_NONE) {
       printf("trap set: (%s) \n",bcm_errmsg(rv));
       return rv;
   }

	return rv;
}

/**
 * Create an accelerated down MEP with ID 4095 and level 4
 * The MEP belongs to a long MA group which makes its CCMs 
 * include a long MEG id. vlan = 10 
 *  
 * @author Aviv (20/05/2014)
 * 
 * @param unit 
 * @param port - physical port to be used 
 * 
 * @return int 
 */
int oam_long_ma_example(int unit, int port1, int port2, int port3) {
  bcm_error_t rv;
  int counter_base_id;

  single_vlan_tag = 1;

  port_1 = port1;
  port_2 = port2;
  port_3 = port3;

  rv = get_device_type(unit, &device_type);
  if (rv < 0) {
      printf("Error checking whether the device is arad+.\n");
      print rv;
      return rv;
  }

  rv = create_vswitch_and_mac_entries(unit);
  if (rv != BCM_E_NONE) {
	  printf("(%s) \n",bcm_errmsg(rv));
	  return rv;
  }

  printf("Creating group (long name format)\n");
  bcm_oam_group_info_t_init(&group_info_long_ma);
  sal_memcpy(group_info_long_ma.name, long_name, BCM_OAM_GROUP_NAME_LENGTH);
  rv = bcm_oam_group_create(unit, &group_info_long_ma);
  if (rv != BCM_E_NONE) {
	  printf("(%s) \n",bcm_errmsg(rv));
	  return rv;
  }

  rv = set_counter_source_and_engines(unit,&counter_base_id,port1);
  BCM_IF_ERROR_RETURN(rv); 


  /* Adding acc MEP - downmep */
  bcm_oam_endpoint_info_t_init(&mep_acc_info);
  mep_acc_info.id = (device_type == device_type_qux) ? 255 : 4095;
  /*TX*/
  mep_acc_info.type = bcmOAMEndpointTypeEthernet;
  mep_acc_info.group = group_info_long_ma.id;
  mep_acc_info.level = 4;
  BCM_GPORT_SYSTEM_PORT_ID_SET(mep_acc_info.tx_gport, port_1);
  mep_acc_info.name = 456;
  mep_acc_info.ccm_period = BCM_OAM_ENDPOINT_CCM_PERIOD_100MS;
  mep_acc_info.flags |= BCM_OAM_ENDPOINT_WITH_ID;
  mep_acc_info.opcode_flags |= BCM_OAM_OPCODE_CCM_IN_HW;
  mep_acc_info.vlan = 10;
  mep_acc_info.pkt_pri = 0 +(2<<1);  /*dei(1bit) + (pcp(3bit) << 1) */
  mep_acc_info.outer_tpid = 0x8100;
  mep_acc_info.inner_vlan = 20;
  mep_acc_info.inner_pkt_pri = 0;     
  mep_acc_info.inner_tpid = 0x9100;
  mep_acc_info.int_pri = 3 +(1<<2);
  mep_acc_info.timestamp_format = get_oam_timestamp_format(unit);

  if (device_type >= device_type_arad_plus) {
      /* Take RDI only from RX*/
      mep_acc_info.flags2 = BCM_OAM_ENDPOINT_FLAGS2_RDI_FROM_LOC_DISABLE;
  }

  /* The MAC address that the CCM packets are sent with*/
  src_mac_mep_3[5] = port_1;
  sal_memcpy(mep_acc_info.src_mac_address, src_mac_mep_3, 6);

  /*RX*/
  mep_acc_info.gport = gport1;
  sal_memcpy(mep_acc_info.dst_mac_address, mac_mep_3, 6);
  mep_acc_info.lm_counter_base_id = counter_base_id;

  printf("bcm_oam_endpoint_create mep_acc_info\n"); 
  rv = bcm_oam_endpoint_create(unit, &mep_acc_info);
  if (rv != BCM_E_NONE) {
	  printf("(%s) \n",bcm_errmsg(rv));
	  return rv;
  }

  printf("created MEP with id %d\n", mep_acc_info.id);

  /* Adding Remote MEP */
  bcm_oam_endpoint_info_t_init(&rmep_info);
  rmep_info.name = 460;
  rmep_info.local_id = mep_acc_info.id;
  rmep_info.type = bcmOAMEndpointTypeEthernet;
  rmep_info.ccm_period = 0;
  rmep_info.flags |= BCM_OAM_ENDPOINT_REMOTE;
  rmep_info.loc_clear_threshold = 1;
  rmep_info.flags |= BCM_OAM_ENDPOINT_WITH_ID;
  rmep_info.id = mep_acc_info.id; 
  if (device_type >= device_type_arad_plus) {
	  rmep_info.flags2 = BCM_OAM_ENDPOINT_FLAGS2_RDI_ON_RX_RDI | BCM_OAM_ENDPOINT_FLAGS2_RDI_ON_LOC;
  }

  printf("bcm_oam_endpoint_create RMEP\n"); 
  rv = bcm_oam_endpoint_create(unit, &rmep_info);
  if (rv != BCM_E_NONE) {
	  printf("(%s) \n",bcm_errmsg(rv));
	  return rv;
  }
  printf("created RMEP with id %d\n", rmep_info.id);

  rv = register_events(unit);
  return rv;
}


/**
 * Sets trapping of packets to specific 1 or 2 MEPs or MIPs to a gport
 * and snooping those packets to a 2nd gport.
 */
int trap_snoop_set(int unit, bcm_oam_endpoint_t endpoint1, bcm_oam_endpoint_t endpoint2,
                   bcm_gport_t trap_gport, bcm_gport_t snoop_gport) {
    bcm_error_t rv;
    bcm_oam_endpoint_action_t action;
    bcm_gport_t gport, gport_up;
    bcm_rx_trap_config_t trap_config_snoop;
    bcm_rx_snoop_config_t snoop_config_cpu;
    int snoop_cmnd;
    int trap_code, trap_code_up;
    bcm_oam_endpoint_info_t endpoint_info1;
    bcm_oam_endpoint_info_t endpoint_info2;
    int is_ep1_up, is_ep2_up;

    bcm_oam_endpoint_info_t_init(&endpoint_info1);
    bcm_oam_endpoint_info_t_init(&endpoint_info2);

    /* Get the endpoints to check their direction*/
    if (endpoint1 != -1) {
        rv = bcm_oam_endpoint_get(unit, endpoint1, &endpoint_info1);
        if (rv != BCM_E_NONE) {
            printf("(%s) \n", bcm_errmsg(rv));
            return rv;
        }
        is_ep1_up = (endpoint_info1.flags & BCM_OAM_ENDPOINT_UP_FACING) > 0;
    }
    if (endpoint2 != -1) {
        rv = bcm_oam_endpoint_get(unit, endpoint2, &endpoint_info2);
        if (rv != BCM_E_NONE) {
            printf("(%s) \n", bcm_errmsg(rv));
            return rv;
        }
        is_ep2_up = (endpoint_info2.flags & BCM_OAM_ENDPOINT_UP_FACING) > 0;
    }

    /* First allocate a new snoop command */
    rv = bcm_rx_snoop_create(unit, 0/*flags*/, &snoop_cmnd);
    if (rv != BCM_E_NONE) {
        printf("(%s) \n",bcm_errmsg(rv));
        return rv;
    }
    print snoop_cmnd;

    /* Setup the new snoop */
    bcm_rx_snoop_config_t_init(&snoop_config_cpu);
    snoop_config_cpu.flags = (BCM_RX_SNOOP_UPDATE_DEST | BCM_RX_SNOOP_UPDATE_PRIO);
    snoop_config_cpu.dest_port = snoop_gport;
    snoop_config_cpu.size = -1;
    snoop_config_cpu.probability = 100000;

    rv =  bcm_rx_snoop_set(unit, snoop_cmnd, &snoop_config_cpu);
    if (rv != BCM_E_NONE) {
        printf("(%s) \n",bcm_errmsg(rv));
        return rv;
    }

    /* Now create a new trap code, each direction require different kind of trap
       Up MWP requires an FTMH trap (to prevent duplicated set of system headers)
       while Down MEP can use a user defined trap */
    rv = bcm_rx_trap_type_create(unit, 0, bcmRxTrapUserDefine, &trap_code);
    if (rv != BCM_E_NONE) {
        printf("(%s) \n",bcm_errmsg(rv));
        return rv;
    }
    rv = bcm_rx_trap_type_get(unit, 0, bcmRxTrapOamUpMEP2, &trap_code_up);
    if (rv != BCM_E_NONE) {
        printf("(%s) \n", bcm_errmsg(rv));
    }

    /* Setup the new trap */
    bcm_rx_trap_config_t_init(&trap_config_snoop);
    trap_config_snoop.flags = BCM_RX_TRAP_UPDATE_DEST | BCM_RX_TRAP_UPDATE_FORWARDING_HEADER | BCM_RX_TRAP_TRAP;
    trap_config_snoop.snoop_strength = 3;
    trap_config_snoop.snoop_cmnd = snoop_cmnd; /* Connect the snoop command to the trap */
    trap_config_snoop.forwarding_header = bcmRxTrapForwardingHeaderOamBfdPdu;
    trap_config_snoop.dest_port = trap_gport;

    rv = bcm_rx_trap_set(unit, trap_code, &trap_config_snoop);
    if (rv != BCM_E_NONE) {
        printf("(%s) \n",bcm_errmsg(rv));
        return rv;
    }
    rv = bcm_rx_trap_set(unit, trap_code_up, &trap_config_snoop);
    if (rv != BCM_E_NONE) {
        printf("(%s) \n",bcm_errmsg(rv));
        return rv;
    }

    /* Trap representation as a GPort */
    BCM_GPORT_TRAP_SET(gport, trap_code, 7, 3); /*Taken from default values*/
    BCM_GPORT_TRAP_SET(gport_up, trap_code_up, 7, 3); /*Taken from default values*/

    /* Setup an action that uses the new trap+snoop for MC CCMs */
    bcm_oam_endpoint_action_t_init(&action);
    BCM_OAM_OPCODE_SET(action, 1); /*1-CCM*/
    BCM_OAM_ACTION_SET(action, bcmOAMActionMcFwd);  /*CCMs are multicast*/

    /* Set the action for the required endpoint(s) */
    if (endpoint1 > -1) {
        action.destination = is_ep1_up ? gport_up : gport;
        rv = bcm_oam_endpoint_action_set(unit, endpoint1, &action);
        if (rv != BCM_E_NONE) {
            printf("(%s) \n",bcm_errmsg(rv));
            return rv;
        }
    }

    /* Set the action for the required endpoint(s) */
    if (endpoint2 > -1) {
        action.destination = is_ep2_up ? gport_up : gport;
        rv = bcm_oam_endpoint_action_set(unit, endpoint2, &action);
        if (rv != BCM_E_NONE) {
            printf("(%s) \n",bcm_errmsg(rv));
            return rv;
        }
    }

    return rv;
}
/**
 * Sets trapping of packets to specific 1 or 2 MEPs or MIPs to a gport
 * and snooping those packets to a 2nd gport. Overwrite
 * FHEI values using stamping in the IPT with trap_id and
 * trap_qualifier per snoop-command.
 */
int trap_snoop_fhei_stamping_set(int unit,
                                 bcm_oam_endpoint_t endpoint1,
                                 bcm_oam_endpoint_t endpoint2,
                                 bcm_gport_t trap_gport,
                                 bcm_gport_t snoop_gport,
                                 int stamp_trap_id,
                                 int stamp_trap_qualifier) {
    bcm_error_t rv;
    bcm_oam_endpoint_action_t action;
    bcm_gport_t gport, gport_up;
    bcm_rx_trap_config_t trap_config_snoop;
    bcm_mirror_destination_t snoop_dest;
    int snoop_cmnd;
    int trap_code, trap_code_up;
    bcm_oam_endpoint_info_t endpoint_info1;
    bcm_oam_endpoint_info_t endpoint_info2;
    int is_ep1_up, is_ep2_up;

    bcm_oam_endpoint_info_t_init(&endpoint_info1);
    bcm_oam_endpoint_info_t_init(&endpoint_info2);

    /* Get the endpoints to check their direction*/
    if (endpoint1 != -1) {
        rv = bcm_oam_endpoint_get(unit, endpoint1, &endpoint_info1);
        if (rv != BCM_E_NONE) {
            printf("(%s) \n", bcm_errmsg(rv));
            return rv;
        }
        is_ep1_up = (endpoint_info1.flags & BCM_OAM_ENDPOINT_UP_FACING) > 0;
    }
    if (endpoint2 != -1) {
        rv = bcm_oam_endpoint_get(unit, endpoint2, &endpoint_info2);
        if (rv != BCM_E_NONE) {
            printf("(%s) \n", bcm_errmsg(rv));
            return rv;
        }
        is_ep2_up = (endpoint_info2.flags & BCM_OAM_ENDPOINT_UP_FACING) > 0;
    }

    rv = get_device_type(unit, &device_type);
    if (rv < 0) {
        printf("Error checking whether the device is arad+.\n");
        print rv;
        return rv;
    }

    if (device_type < device_type_jericho) {
        printf("FHEI IPT stamping is supported for jericho only.\n");
        return BCM_E_UNAVAIL;
    }

    /* Now create a new trap code, each direction require different kind of trap
       Up MWP requires an FTMH trap (to prevent duplicated set of system headers)
       while Down MEP can use a user defined trap */
    rv = bcm_rx_trap_type_create(unit, 0, bcmRxTrapUserDefine, &trap_code);
    if (rv != BCM_E_NONE) {
        printf("(%s) \n",bcm_errmsg(rv));
        return rv;
    }
    rv = bcm_rx_trap_type_get(unit, 0, bcmRxTrapOamUpMEP2, &trap_code_up);
    if (rv != BCM_E_NONE) {
        printf("(%s) \n", bcm_errmsg(rv));
    }

    bcm_mirror_destination_t_init(&snoop_dest);
    snoop_dest.flags = BCM_MIRROR_DEST_IS_SNOOP;
    snoop_dest.packet_control_updates.valid = BCM_MIRROR_PKT_HEADER_UPDATE_PRIO | BCM_MIRROR_PKT_HEADER_UPDATE_FABRIC_HEADER_EDITING;
    snoop_dest.gport = snoop_gport;
    snoop_dest.packet_control_updates.prio = 0;
    snoop_dest.packet_copy_size = 0;
    snoop_dest.sample_rate_dividend = 100000;
    snoop_dest.sample_rate_divisor  = 100000;
    snoop_dest.packet_control_updates.header_fabric.internal_valid = 1;
    snoop_dest.packet_control_updates.header_fabric.internal.trap_id = stamp_trap_id;
    snoop_dest.packet_control_updates.header_fabric.internal.trap_qualifier = stamp_trap_qualifier;
    rv = bcm_mirror_destination_create(unit, &snoop_dest);
    if (rv != BCM_E_NONE) {
        printf("(%s) \n",bcm_errmsg(rv));
        return rv;
    }
    snoop_cmnd = BCM_GPORT_MIRROR_GET(snoop_dest.mirror_dest_id);

    /* Setup the new trap */
    bcm_rx_trap_config_t_init(&trap_config_snoop);
    trap_config_snoop.flags = BCM_RX_TRAP_UPDATE_DEST | BCM_RX_TRAP_UPDATE_FORWARDING_HEADER | BCM_RX_TRAP_TRAP;
    trap_config_snoop.snoop_strength = 3;
    trap_config_snoop.snoop_cmnd = snoop_cmnd; /* Connect the snoop command to the trap */
    trap_config_snoop.forwarding_header = bcmRxTrapForwardingHeaderOamBfdPdu;
    trap_config_snoop.dest_port = trap_gport;
    rv = bcm_rx_trap_set(unit, trap_code, &trap_config_snoop);
    if (rv != BCM_E_NONE) {
        printf("(%s) \n",bcm_errmsg(rv));
        return rv;
    }
    rv = bcm_rx_trap_set(unit, trap_code_up, &trap_config_snoop);
    if (rv != BCM_E_NONE) {
        printf("(%s) \n",bcm_errmsg(rv));
        return rv;
    }

    /* Trap representation as a GPort */
    BCM_GPORT_TRAP_SET(gport, trap_code, 7, 3); /*Taken from default values*/
    BCM_GPORT_TRAP_SET(gport_up, trap_code_up, 7, 3); /*Taken from default values*/

    /* Setup an action that uses the new trap+snoop for MC CCMs */
    bcm_oam_endpoint_action_t_init(&action);
    BCM_OAM_OPCODE_SET(action, 1); /*1-CCM*/
    BCM_OAM_ACTION_SET(action, bcmOAMActionMcFwd);  /*CCMs are multicast*/

    /* Set the action for the required endpoint(s) */
    if (endpoint1 > -1) {
        action.destination = is_ep1_up ? gport_up : gport;
        rv = bcm_oam_endpoint_action_set(unit, endpoint1, &action);
        if (rv != BCM_E_NONE) {
            printf("(%s) \n",bcm_errmsg(rv));
            return rv;
        }
    }

    /* Set the action for the required endpoint(s) */
    if (endpoint2 > -1) {
        action.destination = is_ep2_up ? gport_up : gport;
        rv = bcm_oam_endpoint_action_set(unit, endpoint2, &action);
        if (rv != BCM_E_NONE) {
            printf("(%s) \n",bcm_errmsg(rv));
            return rv;
        }
    }

    return rv;
}
/**
 *  Configure snooping to CPU packets that are trapped to the
 *  OAMP.
 *  Use this function to set the snooping of CCMs trapped by up
 *  to 2 accelerated endpoints.
 *
 *  @param endpoint1
 *      MEP ID or -1 to ignore
 *  @param endpoint2
 *      MEP ID or -1 to ignore
 *
 */
int acc_snoop_set(int unit, bcm_oam_endpoint_t endpoint1, bcm_oam_endpoint_t endpoint2) {
    bcm_error_t rv;
    bcm_gport_t oamp_gport;

    rv = oamp_gport_get(unit, &oamp_gport);
    if (rv != BCM_E_NONE) {
        printf("(%s) \n",bcm_errmsg(rv));
        return rv;
    }
    print oamp_gport;

    rv = trap_snoop_set(unit, endpoint1, endpoint2, oamp_gport, 0/*snoop to CPU port*/);
    if (rv != BCM_E_NONE) {
        printf("(%s) \n",bcm_errmsg(rv));
    }

    return rv;
}

/**
 *  Configure snooping to CPU packets that are trapped to the
 *  OAMP.
 *  Use this function to set the snooping of CCMs trapped by up
 *  to 2 accelerated endpoints.
 *  And overwrite these values using stamping in the
 *  IPT with trap_id and trap_qualifier per snoop-command.
 *  
 *  @param endpoint1
 *      MEP ID or -1 to ignore
 *  @param endpoint2
 *      MEP ID or -1 to ignore
 *
 */
int acc_snoop_fhei_stamping_set(int unit, bcm_oam_endpoint_t endpoint1, bcm_oam_endpoint_t endpoint2, int stamp_trap_id, int stamp_trap_qualifier) {
    bcm_error_t rv;
    bcm_gport_t oamp_gport;

    rv = get_device_type(unit, &device_type);
    if (rv < 0) {
        printf("Error checking whether the device is arad+.\n");
        print rv;
        return rv;
    }

    if (device_type < device_type_jericho) {
        printf("FHEI IPT stamping is supported for jericho only.\n");
        return BCM_E_UNAVAIL;
    }

    rv = oamp_gport_get(unit, &oamp_gport);
    if (rv != BCM_E_NONE) {
        printf("(%s) \n",bcm_errmsg(rv));
        return rv;
    }
    print oamp_gport;

    rv = trap_snoop_fhei_stamping_set(unit, endpoint1, endpoint2, oamp_gport, 0/*snoop to CPU port*/, stamp_trap_id, stamp_trap_qualifier);
    if (rv != BCM_E_NONE) {
        printf("(%s) \n",bcm_errmsg(rv));
    }
    return rv;
}

/**
 * Function sets the OAM default trapping for all Up MEPs and 
 * Down MEPs. 
 * Also displays an example of getting the trap number for OAM 
 * traps. 
 * 
 * 
 * @param unit 
 * @param oam_trapping_port 
 * 
 * @return int 
 */
int configure_oam_default_trap(int unit, int oam_trapping_port) {
    int rv;
    int trap_code;
    bcm_rx_trap_config_t  oam_trap_config;
    int trap_port;

    /* First, get the trap code used for OAM traps*/
    rv = bcm_rx_trap_type_get(unit,0/* flags */ ,bcmRxTrapBfdOamDownMEP, &trap_code);
    if (rv != BCM_E_NONE) {
        printf("(%s) \n",bcm_errmsg(rv));
    }

    /* Now the trap config. */
    rv = bcm_rx_trap_get(unit,trap_code, &oam_trap_config);
    if (rv != BCM_E_NONE) {
        printf("(%s) \n",bcm_errmsg(rv));
    }

    /* Due to a bug this must be filled manually.*/
    oam_trap_config.flags |= BCM_RX_TRAP_TRAP;

    /* Update the destination*/
    BCM_GPORT_LOCAL_SET(oam_trap_config.dest_port, oam_trapping_port);

    /* Now update the trap config with the new destination*/
    rv = bcm_rx_trap_set(unit,trap_code, oam_trap_config);
    if (rv != BCM_E_NONE) {
        printf("(%s) \n",bcm_errmsg(rv));
    }

    /* Now the same for Up direction*/
    rv = bcm_rx_trap_type_get(unit,0 ,bcmRxTrapOamUpMEP, &trap_code);
    if (rv != BCM_E_NONE) {
        printf("(%s) \n",bcm_errmsg(rv));
    }

    /* Using the same trap configuration for Up MEP as Down MEP*/
    rv = bcm_rx_trap_set(unit,trap_code, oam_trap_config);
    if (rv != BCM_E_NONE) {
        printf("(%s) \n",bcm_errmsg(rv));
    }

    /* Note that it may be advisable to do the same with the traps
       bcmRxTrapOamLevel
       bcmRxTrapOamPassive
       */

    return rv;
}

/**
 * Function that sets up MEP snooping destination for QAX and
 * above devices. This snooping is quite 'fake'. a trap is being
 * usen instead of an actual snoop command. The reason for this
 * is the inability to keep original outlif on the snooped
 * packet system headers.
 *
 * @param unit
 *
 * @return int
 */
int configure_default_up_snooping_destination(int unit, int new_trapping_port) {
    int rv;
    int trap_code;
    bcm_rx_trap_config_t oam_trap_config;
    int cpu_port = 0;

    /* This is meant for QAX and above only */
    rv = get_device_type(unit, &device_type);
    BCM_IF_ERROR_RETURN(rv);
    if (device_type < device_type_qax) {
        return BCM_E_UNAVAIL;
    }

    /* First, get the trap code dedicated for up MEP snooping. */
    rv = bcm_rx_trap_type_get(unit,0 ,bcmRxTrapOamUpMEP3, &trap_code);
    if (rv != BCM_E_NONE) {
        printf("(%s) \n",bcm_errmsg(rv));
    }

    /* Now the trap config. */
    bcm_rx_trap_config_t_init(&oam_trap_config);
    rv = bcm_rx_trap_get(unit, trap_code, &oam_trap_config);
    if (rv != BCM_E_NONE) {
        printf("(%s) \n",bcm_errmsg(rv));
    }

    /* Update the destination. */
    BCM_GPORT_LOCAL_SET(oam_trap_config.dest_port, new_trapping_port);

    /* Update the trap config with the new destination. */
    rv = bcm_rx_trap_set(unit, trap_code, oam_trap_config);
    if (rv != BCM_E_NONE) {
        printf("(%s) \n",bcm_errmsg(rv));
    }

    return rv;
}

/**
 * #Demonstrate RDI generation from multiple remote peers: 
 * #Create a Down-MEP ,create 3 RMEPs for this MEP. 
 * 
 * 
 * @param unit 
 * @param port1 
 * @param port2 
 * @param port3 
 * 
 * @return int 
 */
int oam_multipoint_rdi_assertion(int unit, int port1, int port2, int port3) {

    bcm_error_t rv;
    bcm_oam_group_info_t *group_info;
    bcm_oam_endpoint_info_t mep_not_acc_info;

    int i=20; /*Initial RMEP ID*/

    port_1 = port1;
    port_2 = port2;
    port_3 = port3; 

    int md_level = 3;
    int lm_counter_base_id_1;
    
    rv = create_vswitch_and_mac_entries(unit);
    if (rv != BCM_E_NONE) {
        printf("(%s) \n",bcm_errmsg(rv));
        return rv;
    }
    
    printf("Creating MA group (short name format)\n");
    bcm_oam_group_info_t_init(&group_info_short_ma);
    sal_memcpy(group_info_short_ma.name, short_name, BCM_OAM_GROUP_NAME_LENGTH);
    
    rv = bcm_oam_group_create(unit, &group_info_short_ma);
    if (rv != BCM_E_NONE) {
        printf("(%s) \n",bcm_errmsg(rv));
        return rv;
    }
    group_info = &group_info_short_ma;
    rv = set_counter_source_and_engines(unit,&lm_counter_base_id_1,port_1);
    BCM_IF_ERROR_RETURN(rv); 

    /*
    * Adding acc MEP - downmep
    */
    
    bcm_oam_endpoint_info_t_init(&mep_acc_info);
    
    /*TX*/
    mep_acc_info.type = bcmOAMEndpointTypeEthernet;
    mep_acc_info.group = group_info->id;
    mep_acc_info.level = md_level;
    BCM_GPORT_SYSTEM_PORT_ID_SET(mep_acc_info.tx_gport, port_1);
    mep_acc_info.name = 456;
    mep_acc_info.ccm_period = 100;
    mep_acc_info.opcode_flags |= BCM_OAM_OPCODE_CCM_IN_HW;
    
    mep_acc_info.vlan = 10;
    mep_acc_info.pkt_pri = mep_acc_info.pkt_pri = 0 + (1<<1); /* dei(1bit) + (pcp(3bit) << 1)*/
    mep_acc_info.outer_tpid = 0x8100;
    mep_acc_info.inner_vlan = 0;
    mep_acc_info.inner_pkt_pri = 0;
    mep_acc_info.inner_tpid = 0;
    mep_acc_info.int_pri = 1 + (3<<2);
   
    if (device_type >= device_type_arad_plus) {
        /* Take RDI only from RX*/
        mep_acc_info.flags2 = BCM_OAM_ENDPOINT_FLAGS2_RDI_FROM_LOC_DISABLE ;
    }
    /* The MAC address that the CCM packets are sent with*/
    sal_memcpy(mep_acc_info.src_mac_address, src_mac_mep_3, 6);
    
    /*RX*/
    mep_acc_info.gport = gport1;
    sal_memcpy(mep_acc_info.dst_mac_address, mac_mep_3, 6);
    mep_acc_info.lm_counter_base_id = lm_counter_base_id_1;
    mep_acc_info.timestamp_format = get_oam_timestamp_format(unit);
    
    printf("bcm_oam_endpoint_create mep_acc_info\n");
    rv = bcm_oam_endpoint_create(unit, &mep_acc_info);
    if (rv != BCM_E_NONE) {
    printf("(%s) \n",bcm_errmsg(rv));
    return rv;
    }
    printf("created MEP with id %d\n", mep_acc_info.id);
    
    ep1.gport = mep_acc_info.gport;
    ep1.id = mep_acc_info.id;
    
    /*
    * Adding Remote MEP
    */
      
    bcm_oam_endpoint_info_t_init(&rmep_info);
    rmep_info.local_id = mep_acc_info.id;
    rmep_info.type = bcmOAMEndpointTypeEthernet;
    rmep_info.ccm_period = 0;
    rmep_info.flags |= BCM_OAM_ENDPOINT_REMOTE;
    rmep_info.loc_clear_threshold = 1;
    if (device_type >= device_type_arad_plus) {
         /* Take RDI only from RX*/
        rmep_info.flags2 = BCM_OAM_ENDPOINT_FLAGS2_RDI_ON_RX_RDI;
    }

    /*Create three RMEPs for the MEP*/
    for (i; i<23; i++){
        rmep_info.name = i;
        printf("bcm_oam_endpoint_create RMEP with NAME=%d\n",i);
        rv = bcm_oam_endpoint_create(unit, &rmep_info);
        if (rv != BCM_E_NONE) {
            printf("(%s) \n",bcm_errmsg(rv));
            return rv;
        }
        printf("created RMEP with id %d\n", rmep_info.id);
    }  
      
    return rv;
}

/**
 * Unregister OAM events registered using funtion register_events 

 * 
 * 
 * @param unit 
 * @return int 
 */
int unregister_events(int unit) {
  bcm_error_t rv;
  bcm_oam_event_types_t timeout_event, timein_event, port_interface_event;

  BCM_OAM_EVENT_TYPE_SET(timeout_event, bcmOAMEventEndpointCCMTimeout);
  rv = bcm_oam_event_unregister(unit, timeout_event, cb_oam);
  if (rv != BCM_E_NONE) {
	  printf("(%s) \n",bcm_errmsg(rv));
	  return rv;
  }

  BCM_OAM_EVENT_TYPE_SET(timein_event, bcmOAMEventEndpointCCMTimein);
  rv = bcm_oam_event_unregister(unit, timein_event, cb_oam);
  if (rv != BCM_E_NONE) {
	  printf("(%s) \n",bcm_errmsg(rv));
	  return rv;
  }

  BCM_OAM_EVENT_TYPE_SET(timein_event, bcmOAMEventEndpointRemote);
  rv = bcm_oam_event_unregister(unit, timein_event, cb_oam);
  if (rv != BCM_E_NONE) {
	  printf("(%s) \n",bcm_errmsg(rv));
	  return rv;
  }

  BCM_OAM_EVENT_TYPE_SET(timein_event, bcmOAMEventEndpointRemoteUp);
  rv = bcm_oam_event_unregister(unit, timein_event, cb_oam);
  if (rv != BCM_E_NONE) {
	  printf("(%s) \n",bcm_errmsg(rv));
	  return rv;
  }

  BCM_OAM_EVENT_TYPE_CLEAR_ALL(port_interface_event);
  BCM_OAM_EVENT_TYPE_SET(port_interface_event, bcmOAMEventEndpointPortDown);
  rv = bcm_oam_event_unregister(unit, port_interface_event, cb_oam);
  if (rv != BCM_E_NONE) {
      printf("(%s) \n",bcm_errmsg(rv));
      return rv;
  }

  BCM_OAM_EVENT_TYPE_CLEAR_ALL(port_interface_event);
  BCM_OAM_EVENT_TYPE_SET(port_interface_event, bcmOAMEventEndpointInterfaceDown);
  rv = bcm_oam_event_unregister(unit, port_interface_event, cb_oam);
  if (rv != BCM_E_NONE) {
      printf("(%s) \n",bcm_errmsg(rv));
      return rv;
  }

  return rv;
}