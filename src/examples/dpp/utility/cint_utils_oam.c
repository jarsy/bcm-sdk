/*
 * $Id: oam.c,v 1.148 2013/09/17 10:45:12  Exp $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:    cint_utils_oam.c
 * Purpose: Utility functions and variables to be used by all OAM/BFD cints
 */

 /* 
The cint  cint_queue_tests.c  must be included: used to get core from port*/

/* Enum signifiyng the device type. Should be ordered oldest device to newest*/
enum device_type_t { 
    device_type_arad_a0=0,
    device_type_arad_plus=1,
    device_type_jericho=2,
    device_type_jericho_b=3,
    device_type_jericho_plus=4,
    device_type_qax=5,
    device_type_qux=6
} ;

device_type_t device_type;

  /* Enum signifying the eth format header */
 enum eth_tag_type_t { 
      untagged=0,
      single_tag=1,
      double_tag=2
  } ;

eth_tag_type_t eth_tag_type;

/* Endpoint info */
struct oam__ep_info_s {
    int id;
	int rmep_id;
	bcm_gport_t gport;
};

int get_device_type(int unit, device_type_t *device_type)
{
  bcm_info_t info;

  int rv = bcm_info_get(unit, &info);
  if (rv != BCM_E_NONE) {
      printf("Error in bcm_info_get\n");
      print rv;
      return rv;
  }

  *device_type = (info.device == 0x8650 && info.revision == 0) ? device_type_arad_a0 :
                 (info.device == 0x8660) ? device_type_arad_plus :
                 (((info.device == 0x8375) || (info.device == 0x8675)) && info.revision == 0x11) ? device_type_jericho_b :
                 ((info.device == 0x8375) || (info.device == 0x8675)) ? device_type_jericho :
                 (info.device == 0x8470) ? device_type_qax :
                 (info.device == 0x8680) ? device_type_jericho_plus :
                 (info.device == 0x8270) ? device_type_qux :
                 -1;

    return rv;
}

/* HLM counters */
int counter_engines[2];
int counter_base_ids[4];

/****************************************************************/
/*                    VSWITCH INITIALIZATION FUNCTIONS                                               */
/****************************************************************/



/**
 * Part of vswitch initialization process.
 * 
 * 
 * @param unit 
 * @param known_mac_lsb 
 * @param known_vlan 
 * 
 * @return int 
 */
int
vswitch_metro_run(int unit, int known_mac_lsb, int known_vlan){
    int rv;
    /*bcm_vlan_t  vsi*/;
    bcm_mac_t kmac;
    int index;
    bcm_vlan_t kvlan;
    int flags, i;

    kmac[5] = known_mac_lsb;
    kvlan = known_vlan;
  
    /* set ports to identify double tags packets */
    port_tpid_init(vswitch_metro_mp_info.sysports[0],1,1);
    rv = port_tpid_set(unit);
    if (rv != BCM_E_NONE) {
       printf("Error, port_tpid_set, in unit %d\n", unit);
       print rv;
       return rv;
    }

    port_tpid_init(vswitch_metro_mp_info.sysports[1],1,1);
    rv = port_tpid_set(unit);
    if (rv != BCM_E_NONE) {
        printf("Error, port_tpid_set\n");
        print rv;
        return rv;
    }

    port_tpid_init(vswitch_metro_mp_info.sysports[2],1,1);
    rv = port_tpid_set(unit);
    if (rv != BCM_E_NONE) {
        printf("Error, port_tpid_set\n");
        print rv;
        return rv;
    }
    /* When using new vlan translation mode, tpid and vlan actions and mapping must be configured manually */
    if (advanced_vlan_translation_mode) {
        rv = vlan_translation_default_mode_init(unit);
        if (rv != BCM_E_NONE) {
            printf("Error, in vlan_translation_default_mode_init\n");
            return rv;
        }
    }

    for (index = 0; index < 12; index++) {
        if (single_vlan_tag && (((index % 2) != 0))) {
            continue;
        }
        rv = vlan__init_vlan(unit,vswitch_metro_mp_info.p_vlans[index]);
        if (rv != BCM_E_NONE && rv != BCM_E_EXISTS) {
            printf("Error, vlan__init_vlan\n");
            print rv;
            return rv;
        }
    }

	/* 1. create vswitch + create Multicast for flooding */
	rv = vswitch_create(unit, &vsi, 0);
	if (rv != BCM_E_NONE) {
		printf("Error, vswitch_create\n");
		print rv;
		return rv;
	}
    printf("Created vswitch vsi=0x%x\n", vsi);

	/* 2. create first vlan-port */

	flags = 0;
	gport1 = 0;
	rv = vswitch_metro_add_port(unit, 0, &gport1, flags);
	if (rv != BCM_E_NONE) {
		printf("Error, vswitch_metro_add_port_1\n");
		print rv;
		return rv;
	}
	if(verbose){
		printf("created vlan-port   0x%08x in unit %d\n",gport1, unit);
	}

	/* 3. add vlan-port to the vswitch and multicast */
	rv = vswitch_add_port(unit, vsi,vswitch_metro_mp_info.sysports[0], gport1);
	if (rv != BCM_E_NONE) {
		printf("Error, vswitch_add_port\n");
		return rv;
	}
	/* add another port to the vswitch */

	/* 4. create second vlan-port */
	flags = 0;
	gport2 = 0;
	rv = vswitch_metro_add_port(unit, 1, &gport2, flags);
	if (rv != BCM_E_NONE) {
		printf("Error, vswitch_metro_add_port_2\n");
		return rv;
	}
	if(verbose){
		printf("created vlan-port   0x%08x\n\r",gport2);
	}

	/* 5. add vlan-port to the vswitch and multicast */

	/* Local unit for sysport2 is already first */
	rv = vswitch_add_port(unit, vsi,vswitch_metro_mp_info.sysports[1], gport2);
	if (rv != BCM_E_NONE) {
		printf("Error, vswitch_add_port\n");
		return rv;
	}

	/* add a third port to the vswitch */
 
	/* 6. create third vlan-port */
	flags = 0;
	gport3 = 0;
	rv = vswitch_metro_add_port(unit, 2, &gport3, flags);
	if (rv != BCM_E_NONE) {
		printf("Error, vswitch_metro_add_port_2\n");
		return rv;
	}
	if(verbose){
		printf("created vlan=port   0x%08x in unit %d\n",gport3, unit);
	}

	/* 7. add vlan-port to the vswitch and multicast */ 
	rv = vswitch_add_port(unit, vsi,vswitch_metro_mp_info.sysports[2], gport3);
	if (rv != BCM_E_NONE) {
		printf("Error, vswitch_add_port\n");
		return rv;
	}

	rv = vswitch_add_l2_addr_to_gport(unit, gport3, kmac, kvlan);
	if (rv != BCM_E_NONE) {
		printf("Error, vswitch_add_l2_addr_to_gport\n");
		return rv;
	}
    return rv;
}


/**
 * Initialize vswitch for OAM example usage.
 * 
 * 
 * @param unit 
 * 
 * @return int 
 */
int create_vswitch_and_mac_entries(int unit) {
  bcm_error_t rv;
  int flags;

  printf("Creating vswitch in unit %d\n", unit); 
  bcm_port_class_set(unit, port_1, bcmPortClassId, port_1);
  bcm_port_class_set(unit, port_2, bcmPortClassId, port_2);
  bcm_port_class_set(unit, port_3, bcmPortClassId, port_3);

  vswitch_metro_mp_info_init(port_1, port_2, port_3);

  rv = vswitch_metro_run(unit, 0xce, 3);
  if (rv != BCM_E_NONE){
	  printf("Error, in vswitch_metro_run\n");
	  return rv;
  }
  
  printf("Adding mep MAC addresses to MAC table\n");
  /* int l2_entry_add(int unit, bcm_mac_t mac, bcm_vlan_t vlan, int dest_type, int dest_gport, int dest_mc_id){*/
  rv = l2_entry_add(unit, mac_mep_1, 4096, 0, gport1, 0);                                       
  if (rv != BCM_E_NONE) {
      printf("(%s) \n",bcm_errmsg(rv));
      return rv;
  }

  rv = l2_entry_add(unit, mac_mep_2, 4096, 0, gport2, 0);                                       
  if (rv != BCM_E_NONE) {
	  printf("(%s) \n",bcm_errmsg(rv));
	  return rv;
  }

  printf("Creating multicast group\n");
  flags =  BCM_MULTICAST_INGRESS_GROUP | BCM_MULTICAST_WITH_ID;
  rv = bcm_multicast_create(unit, flags, mc_group);                                    
  if (rv != BCM_E_NONE) {
	  printf("(%s) \n",bcm_errmsg(rv));
	  return rv;
  }

  rv = bcm_multicast_ingress_add(unit, mc_group, port_1, 0);                                    
  if (rv != BCM_E_NONE) {
	  printf("(%s) \n",bcm_errmsg(rv));
	  return rv;
  }

  /* Adding MC mac address of mep 2 to the multicast group */
  rv = l2_entry_add(unit, mac_mep_2_mc, 4096, 1, 0, mc_group);                                       
  if (rv != BCM_E_NONE) {
      printf("(%s) \n",bcm_errmsg(rv));
      return rv;
  }

  return rv;
}

int NUMBER_OF_COUNTERS_PER_COUNTER_SOURCE = 0x4000; 
 /* Two copies: one per core*/
int current_lm_counter_base[2]={0};
int current_counter_engine[2]={0}; 
int next_available_counter_engine=0;/* Assuming no other application is calling counter_config_set()*/
int current_counter_id=16;

int counter_engine_status[16] = {0};

/* In order to count traffic over GREoIP tunnels this function should be called
 * The returned value (counter ID) should go into the field val in bcm_port_stat_set()
 * Each call allocates 1 counter IDs 
*/ 
int set_counter_source_and_engines_for_tunnel_counting(int unit, int * counter_base, int port){

    int rv=0;
    int counter_id;
    int core, tm_port;

    rv = get_core_and_tm_port_from_port(unit, port, &core, &tm_port);
    if (rv != BCM_E_NONE) {
        printf("Error, in get_core_and_tm_port_from_port\n");
        return rv;
    }

    if (current_counter_id == 16) {
        rv = set_counter_source_and_engines(unit, &counter_id, port);
        if (rv != BCM_E_NONE) {
            printf("Error, in get_core_and_tm_port_from_port\n");
            return rv;
        }
        current_counter_id =1;
    } else {
        current_counter_id ++;
    }

    *counter_base = (current_lm_counter_base[core] -16) + current_counter_id;
    
    return 0;

} 


/** 
 * Whenever creating an endpoint on a new LIF this function 
 *  should be called. The value returned in counter_base should
 *  go into the field lm_counter_base_id in
 *  bcm_oam_endpoint_create().
 *  
 *  Allocate counters. In Arad counters have been statically
 *  allocated via soc property, simply maintain a global
 *  variable.
 *  
 *  In Jericho Counter engines must be allocated on the fly.
 *  Each call to bcm_stat_counter_config_set() allocates 8 K
 *  counter IDs. Maintain a global counter and whenever counters
 *  ids have run out call counter_config_set() (i.e. on the
 *  first call, 8K-th call, etc.).
 *  Each call allocates 16 counter ids to account for PCP based
 *  LM.
 *  
 * 
 * @author sinai (28/12/2014)
 * 
 * @param unit 
 * @param counter_base - return value 
 * @param port - Port on which MEP is defined, used for 
 *             extracting core ID. All endpoints must be defined
 *             on same core.
 *         
 * 
 * @return int 
 */
int set_counter_source_and_engines(int unit, int *counter_base, int port) {
   bcm_stat_counter_engine_t counter_engine;
   bcm_stat_counter_config_t counter_config;
   bcm_color_t colors[] = { bcmColorGreen, bcmColorYellow, bcmColorRed, bcmColorBlack };
   uint8    drop_fwd[] = { 0, 1 };
   int      index1, index2;
   int      rv;
   int      core, tm_port;
   uint64   arg;
   uint32   counter_engine_flags = 0;

   /* This should be initialized by now anyways, but check again*/
   rv = get_device_type(unit, &device_type);
   BCM_IF_ERROR_RETURN(rv);

   rv = get_core_and_tm_port_from_port(unit, port, &core, &tm_port);
   if (rv != BCM_E_NONE) {
      printf("Error, in get_core_and_tm_port_from_port\n");
      return rv;
   }

   NUMBER_OF_COUNTERS_PER_COUNTER_SOURCE = SOC_DPP_DEFS_GET_COUNTERS_PER_COUNTER_PROCESSOR(unit);

   if (device_type < device_type_jericho) {
      /* In Arad the counters have been statically allocated via soc properties. Furthurmore LM-PCP is not available.
         simply increment the counter and return.*/
      *counter_base = ++current_lm_counter_base[core];
      return 0;
   }

   if (device_type >= device_type_qax) {
       /* QAX may recevive the counter from a 2nd source.
          For this two work multiple sources per counter engine should be enbaled. */
       counter_engine_flags = BCM_STAT_COUNTER_MULTIPLE_SOURCES_PER_ENGINE;
   }

   if (current_lm_counter_base[core] % NUMBER_OF_COUNTERS_PER_COUNTER_SOURCE == 0) {
      /* Ran out of available counters. Allocate another bunch.*/
      if (next_available_counter_engine >= 16) {
         print "Out of counter engines.";
         return 444;
      }

      current_counter_engine[core] = next_available_counter_engine;
      current_lm_counter_base[core] = next_available_counter_engine * NUMBER_OF_COUNTERS_PER_COUNTER_SOURCE;
      ++next_available_counter_engine;

      counter_config.format.format_type = bcmStatCounterFormatPackets; /* This gives us 16K counter ids per engine, but only 29 bits for the counter.*/
      counter_config.format.counter_set_mapping.counter_set_size = 1; /* Since we are indifferent to color/droped-forwarded we only need one counter set.*/
      counter_config.format.counter_set_mapping.num_of_mapping_entries = 8; /* Using 8 entries for 8 PCP values.*/
      for (index1 = 0; index1 < bcmColorPreserve; index1++) {
         for (index2 = 0; index2 < 2; index2++) {
            /* Counter configuration is independent on the color, drop precedence.*/
            counter_config.format.counter_set_mapping.entry_mapping[index1 * 2 + index2].offset = 0; /* Must be zero since counter_set_size is 1 */
            counter_config.format.counter_set_mapping.entry_mapping[index1 * 2 + index2].entry.color = colors[index1];
            counter_config.format.counter_set_mapping.entry_mapping[index1 * 2 + index2].entry.is_forward_not_drop = drop_fwd[index2];
         }
      }
      counter_config.source.core_id = core;

      counter_config.source.pointer_range.start = current_lm_counter_base[core];
      counter_config.source.pointer_range.end = current_lm_counter_base[core] + NUMBER_OF_COUNTERS_PER_COUNTER_SOURCE - 1;

      counter_config.source.engine_source = bcmStatCounterSourceIngressOam;
      counter_engine.engine_id = current_counter_engine[core];
      counter_engine.flags = counter_engine_flags;
      rv = bcm_stat_counter_config_set(unit, &counter_engine, &counter_config);
      BCM_IF_ERROR_RETURN(rv);
      counter_engine_status[counter_engine.engine_id] = 1;
   }

   /* At this stage the counter source has been allocated. Return the counter base and get out.*/
   current_lm_counter_base[core] += 16; /* Increment by 16 to allow for PCP based LM.*/
   /* If PCP lm is used 16 counter sources are needed per MEP: 8 for the ingress and 8 for the egress.*/
   *counter_base = current_lm_counter_base[core]; /* ID 0 cannot be used - the value 0 is reserved to signify disabling LM counters.*/

   return 0;
}


/**
 *  Allocate counters for pmf.
 *  In Arad counters have been statically
 *  allocated via soc property, simply maintain a global
 *  variable.
 *
 *  In Jericho Counter engines must be allocated on the fly.
 *  Each call to bcm_stat_counter_config_set() allocates 16K counter IDs.
 *
 * @author mark (15/12/2015)
 *
 * @param unit
 * @param counter_offset - counter offset should be multiple of engine counter size(16K).
 * @param engine_id - allocate engine (0-15)
 * @param core      - core (0-1)
 * @return int
 */
int pmf_ccm_counter_engine_allocation(int unit, int counter_start, int engine_id, int core)
{
    bcm_stat_counter_engine_t counter_engine ;
    bcm_stat_counter_config_t counter_config ;
    bcm_color_t colors[] = { bcmColorGreen, bcmColorYellow, bcmColorRed, bcmColorBlack };
    uint8 drop_fwd[] = { 0,1 };
    int rv=0;
    int index1, index2;

    /* This should be initialized by now anyways, but check again*/
    rv = get_device_type(unit, &device_type);
    BCM_IF_ERROR_RETURN(rv);

    if (device_type<device_type_jericho ) {
        /* In Arad the counters have been statically allocated via soc properties. */
        return 0;
    }

    if (SOC_DPP_DEFS_GET_NOF_CORES(unit) > 1) {
        if (((sal_strcmp(soc_property_get_str(unit, spn_DEVICE_CORE_MODE), "SINGLE_CORE")) == 0) && (core == 1)) 
        {
            print "We are in single core mode! Counter shold not be initialized for core 1.";
            return 0;
        }
    }

    if(counter_engine_status[engine_id] == 1) {
        printf("Engine %d allocated already",engine_id);
        return 444;
    }

    NUMBER_OF_COUNTERS_PER_COUNTER_SOURCE = SOC_DPP_DEFS_GET_COUNTERS_PER_COUNTER_PROCESSOR(unit);

    if ( counter_start % NUMBER_OF_COUNTERS_PER_COUNTER_SOURCE != 0  ) {
            print "Start should be multiple of engine size(16K).";
            return 444;
    }

    counter_config.format.format_type = bcmStatCounterFormatPackets; /* This gives us 16K counter ids per engine, but only 29 bits for the counter.*/
    counter_config.format.counter_set_mapping.counter_set_size = 1;/* Since we are indifferent to color/droped-forwarded we only need one counter set.*/
    counter_config.format.counter_set_mapping.num_of_mapping_entries = 8; /* Using 8 entries for 8 PCP values.*/
    if(soc_property_get(unit , "oam_statistics_per_mep_enabled",1) == 2)
        counter_config.source.command_id = 1; /*stat1*/
    else
        counter_config.source.command_id = 0; /*stat0*/
    for (index1 = 0; index1 < bcmColorPreserve; index1++) {
        for (index2 = 0; index2 < 2; index2++) {
            /* Counter configuration is independent on the color, drop precedence.*/
            counter_config.format.counter_set_mapping.entry_mapping[index1 * 2 + index2].offset = 0; /* Must be zero since counter_set_size is 1 */
            counter_config.format.counter_set_mapping.entry_mapping[index1 * 2 + index2].entry.color = colors[index1];
            counter_config.format.counter_set_mapping.entry_mapping[index1 * 2 + index2].entry.is_forward_not_drop = drop_fwd[index2];
        }
    }
    counter_config.source.core_id = core;

    counter_config.source.pointer_range.start =(counter_start / NUMBER_OF_COUNTERS_PER_COUNTER_SOURCE)  *NUMBER_OF_COUNTERS_PER_COUNTER_SOURCE ;
    counter_config.source.pointer_range.end = ((counter_start / NUMBER_OF_COUNTERS_PER_COUNTER_SOURCE) +1)  *NUMBER_OF_COUNTERS_PER_COUNTER_SOURCE -1;

    counter_config.source.engine_source = bcmStatCounterSourceIngressField;
    counter_engine.engine_id=  engine_id;

    rv = bcm_stat_counter_config_set(unit, &counter_engine, &counter_config);
    BCM_IF_ERROR_RETURN(rv);
    counter_engine_status[engine_id]=1;

    return 0;
}


/**
 *  Function creates an accelerated endpoint.
 *  group_info_short_ma  must be initialized before this
 *  function is called.
 *  Likewise the function create_vswitch_and_mac_entries() must
 *  also  be called before this function.
 *  
 * @author liat 
 * 
 * @param unit 
 * @param is_up - direction of the endpoint.
 * @param eth_tag_type - vlan tag format.
 * 
 * @return int 
 */

int port_1_interface_2_both_3_status = 0;

int create_oam_accelerated_endpoint(int unit , int is_up, eth_tag_type_t eth_tag_type, int is_itmh_mc) {
   bcm_oam_endpoint_info_t  acc_endpoint;
   bcm_oam_endpoint_info_t_init(&acc_endpoint);

  /*TX*/
  acc_endpoint.type = bcmOAMEndpointTypeEthernet;
  acc_endpoint.group = group_info_short_ma.id;
  acc_endpoint.opcode_flags |= BCM_OAM_OPCODE_CCM_IN_HW;
  acc_endpoint.timestamp_format = get_oam_timestamp_format(unit);

  switch (eth_tag_type) {
  case untagged:
          acc_endpoint.level = 4;
		  acc_endpoint.vlan = 0; 
		  acc_endpoint.pkt_pri = 0;
		  acc_endpoint.outer_tpid = 0;
		  break;
  case single_tag:
        acc_endpoint.level = 2;
		  acc_endpoint.vlan = 5; 
		  acc_endpoint.pkt_pri = 0 + (2<<1); /* dei(1bit) + (pcp(3bit) << 1)*/
		  acc_endpoint.outer_tpid = 0x8100;
		  break;
	  case double_tag:
		  acc_endpoint.vlan = 10;
		  acc_endpoint.pkt_pri = mep_acc_info.pkt_pri = 0 + (1<<1); /* dei(1bit) + (pcp(3bit) << 1)*/
		  acc_endpoint.outer_tpid = 0x8100;
		  acc_endpoint.inner_vlan = 5;
		  acc_endpoint.inner_pkt_pri = 3;
		  acc_endpoint.inner_tpid = 0x8100;
          acc_endpoint.level = 3;
		  break;
	   default:
		   printf("Error, non valid eth_tag_type\n");
	  }

  if (is_up) {
        /*TX*/
      acc_endpoint.level = 5;
      acc_endpoint.name = 123;     
      acc_endpoint.flags |= BCM_OAM_ENDPOINT_UP_FACING;
      acc_endpoint.int_pri = 3 + (1<<2);
      /* The MAC address that the CCM packets are sent with*/
      src_mac_mep_2[5] = port_2;
      sal_memcpy(acc_endpoint.src_mac_address, src_mac_mep_2, 6);
      /*RX*/
      acc_endpoint.gport = gport2;
      sal_memcpy(acc_endpoint.dst_mac_address, mac_mep_2, 6);
      acc_endpoint.lm_counter_base_id = 6;
      acc_endpoint.tx_gport = BCM_GPORT_INVALID;
  } else { /** Down*/
      
      BCM_GPORT_SYSTEM_PORT_ID_SET(acc_endpoint.tx_gport, port_1);
      acc_endpoint.name = 456;     
      acc_endpoint.ccm_period = 100;
      acc_endpoint.int_pri = 1 + (3<<2);
      /* The MAC address that the CCM packets are sent with*/
        sal_memcpy(acc_endpoint.src_mac_address, src_mac_mep_3, 6);

        /*RX*/
        /* enable oamp injection with mc destination on the ITMH*/
        if (is_itmh_mc) {

            int rv =0;
            bcm_gport_t gport;
            bcm_multicast_t mc_group=0x5000;
            BCM_GPORT_MCAST_SET(gport, mc_group);
            int unit = 0;
            rv =  bcm_multicast_create(unit, BCM_MULTICAST_INGRESS_GROUP | BCM_MULTICAST_WITH_ID, &mc_group);
            BCM_IF_ERROR_RETURN(rv);

            rv =  bcm_multicast_ingress_add(unit, mc_group, 200, 130);
            BCM_IF_ERROR_RETURN(rv);

            rv =  bcm_multicast_ingress_add(unit, mc_group, 201, 140);
            BCM_IF_ERROR_RETURN(rv);

            acc_endpoint.tx_gport = gport;
            sal_memcpy(acc_endpoint.dst_mac_address, mac_mep_3, 6);
            acc_endpoint.lm_counter_base_id = 6;

        }
            acc_endpoint.gport = gport1;
            sal_memcpy(acc_endpoint.dst_mac_address, mac_mep_3, 6);
            acc_endpoint.lm_counter_base_id = 6;
    }

    if (port_1_interface_2_both_3_status == 1) {
        acc_endpoint.flags |= BCM_OAM_ENDPOINT_PORT_STATE_UPDATE;
        acc_endpoint.port_state = BCM_OAM_PORT_TLV_UP;
    } 
    else if(port_1_interface_2_both_3_status == 2) {
        acc_endpoint.flags |= BCM_OAM_ENDPOINT_INTERFACE_STATE_UPDATE;
        acc_endpoint.interface_state = BCM_OAM_INTERFACE_TLV_UP;
    }
    else if (port_1_interface_2_both_3_status == 3) {
        acc_endpoint.flags |= BCM_OAM_ENDPOINT_PORT_STATE_UPDATE | BCM_OAM_ENDPOINT_INTERFACE_STATE_UPDATE;
        acc_endpoint.port_state = BCM_OAM_PORT_TLV_UP;
        acc_endpoint.interface_state = BCM_OAM_INTERFACE_TLV_UP;
    }

  return  bcm_oam_endpoint_create(unit, &acc_endpoint);
}


/**
 * Get OAMP gport
 *
 * @param unit
 */
int oamp_gport_get(int unit, bcm_gport_t *oamp_gport) {
    bcm_error_t rv;
    int count;
    bcm_gport_t gports[2];

    rv = bcm_port_internal_get(unit, BCM_PORT_INTERNAL_OAMP, 2, gports, &count);
    if (rv != BCM_E_NONE) {
        printf("(%s) \n",bcm_errmsg(rv));
        return rv;
    }

    if (count < 1) {
        printf("OAMP GPort not found.\n");
        return BCM_E_NOT_FOUND;
    }

    *oamp_gport = gports[0];
    return BCM_E_NONE;
}


/**
 * 
 * 
 * @author sinai (22/07/2015)
 * 
 * @param unit 
 * @param seconds 
 * @param frac_seconds 
 * @param using_1588 
 * 
 * @return int 
 */
int set_oam_tod(int unit, int seconds, int frac_seconds, int using_1588) {
    uint64 time;
    bcm_oam_control_type_t control_var = using_1588? bcmOamControl1588ToD : bcmOamControlNtpToD;

    COMPILER_64_SET(time, seconds, frac_seconds);
    return bcm_oam_control_set(unit,control_var,time );
}

/**
 * Setup all the necessary counters for hierarchical LM
 * Each packet can cause a single access to a counter engine so if we
 * want a packet to be counted on more then one counter, each must be on
 * a different engine.
 * This function allocates num_engines engines and
 * num_counters_per_engine counters on each engine. 
 * flags - holds the engine flags to be set at engine config 
 */
int hierarchical_lm_counters_setup(int unit,
                                   int num_engines,
                                   int num_counters_per_engine,
                                   int *engine_arr,
                                   int *counter_base_ids,
                                   uint32 flags) {

    bcm_error_t rv = BCM_E_NONE;
    int engine_index, counter_index = 0, i;
    bcm_stat_counter_engine_t counter_engine;
    bcm_stat_counter_config_t counter_config;
    bcm_color_t colors[] = { bcmColorGreen, bcmColorYellow, bcmColorRed, bcmColorBlack };
    uint8 drop_fwd[] = { 0, 1 };
    int index1, index2;
    int NUMBER_OF_COUNTER_INCREASE_COMMANDS = 2;
    int START_ENGINE = 1;

    /* This is meant for QAX and above only  */
    rv = get_device_type(unit, &device_type);
    BCM_IF_ERROR_RETURN(rv);
    if (device_type < device_type_qax) {
        return BCM_E_UNAVAIL;
    }

    NUMBER_OF_COUNTERS_PER_COUNTER_SOURCE = SOC_DPP_DEFS_GET_COUNTERS_PER_COUNTER_PROCESSOR(unit);

    /* Setup counter engines */
    for (engine_index = START_ENGINE; engine_index < (num_engines + START_ENGINE); engine_index++) {

        counter_config.format.format_type = bcmStatCounterFormatPackets; /* This gives us 16K counter ids per engine, but only 29 bits for the counter.*/
        counter_config.format.counter_set_mapping.counter_set_size = 1; /* Since we are indifferent to color/droped-forwarded we only need one counter set.*/
        counter_config.format.counter_set_mapping.num_of_mapping_entries = 8; /* Using 8 entries for 8 PCP values.*/
        counter_config.source.pointer_range.start = (engine_index) * NUMBER_OF_COUNTERS_PER_COUNTER_SOURCE;
        counter_config.source.pointer_range.end = (engine_index + 1) * NUMBER_OF_COUNTERS_PER_COUNTER_SOURCE - 1;
        counter_config.source.engine_source = bcmStatCounterSourceIngressOam;
        counter_config.source.command_id = ((engine_index-START_ENGINE) % NUMBER_OF_COUNTER_INCREASE_COMMANDS);
        for (index1 = 0; index1 < bcmColorPreserve; index1++) {
            for (index2 = 0; index2 < 2; index2++) {
                /* Counter configuration is independent on the color, drop precedence.*/
                counter_config.format.counter_set_mapping.entry_mapping[index1 * 2 + index2].offset = 0; /* Must be zero since counter_set_size is 1 */
                counter_config.format.counter_set_mapping.entry_mapping[index1 * 2 + index2].entry.color = colors[index1];
                counter_config.format.counter_set_mapping.entry_mapping[index1 * 2 + index2].entry.is_forward_not_drop = drop_fwd[index2];
            }
        }

        counter_engine.engine_id = engine_index;
        counter_engine.flags = flags;
        /* Can be useful for debugging but adds a lot of clutter.
         * printf("Configuring counter engine %d\n", engine_index);
         * print counter_engine;
         * print counter_config;
         */

        rv = bcm_stat_counter_config_set(unit, &counter_engine, &counter_config);
        BCM_IF_ERROR_RETURN(rv);

        engine_arr[engine_index - START_ENGINE] = counter_engine.engine_id;
        for (i = 0; i < num_counters_per_engine; i++) {
            counter_base_ids[counter_index++] = counter_config.source.pointer_range.start + (16*(i+engine_index+1));
        }
    }

    return BCM_E_NONE;
}

int ChanTypeMissErr_trap_set(int unit, int dest_port_or_queue, int is_queue) {
    bcm_rx_trap_config_t trap_config;
    int trap_code=0x416; /* trap code on FHEI  will be (trap_code & 0xff), in this case 0x16.*/
    /* valid trap codes for oamp traps are 0x401 - 0x4ff */
    int rv;

    rv =  bcm_rx_trap_type_create(unit, BCM_RX_TRAP_WITH_ID, bcmRxTrapOampChanTypeMissErr, &trap_code);
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
