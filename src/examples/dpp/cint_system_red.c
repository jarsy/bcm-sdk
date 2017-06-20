/*~~~~~~~~~~~~~~~~~~~~~~~~~~ System RED ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*
 * $Id: cint_system_red.c,v 1.2 Broadcom SDK $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 * 
 * File: cint_system_red.c
 * Purpose: Example System RED Validation
 * Reference: BCM88650 TM Device Driver (System )
 * 
 * Environment:
 *
 * The following application example assumes the following system configuration:
 *
 * IMPORTANT: this cint configure only the System RED, it does not configure the system application (port, voq, scheduling...) 
 *
 *   gp:0x24000080   cos:8                Unicast Queue Group    mod: 0   port:12 voq: 128)
 *   gp:0x24000088   cos:8                Unicast Queue Group    mod: 0   port:13 voq: 136)
 *   gp:0x24000090   cos:8                Unicast Queue Group    mod: 0   port:14 voq: 144)
 *
 *                                    in-ports: 12, 13            
 *                                    out-ports: 14             
 *                                           --------              
 *                                    -------|      |         
 *                              in_ports     |      | out_port
 *                                    -------|      |--------------
 *                                           |      |              
 *                                           --------    
 *
 *                          
 *  Application description:
 *    1. PP application, including 2 fap_ports, connected to Traffic Generator.
 *    2. UC traffic configuration: 2 streams on each port, each stream has a different DP.
 *         Port_1: X% green and (1-X)% yellow.
 *         Port_2: (1-X)% green and X% yellow.
 *    3. System RED configuration is set to prefer  green packets, over yellow.
 *    4. Qs: Each port is destined to a different Q, both Qs goes to the same destination Port.
 *    5. EGQ/SCH: are configured to set RR between the Qs (no strict priority).
 *    6. Set high rate class drop thresholds (make sure only system red mechanism  is dropping).
 *
 *
 * Test Run:
 *    1. Configure system according to instruction  above
 *    2. Run config_system_red_param().   
 *    3. inject packets according to the instruction  above
 *    2. Expected Output:
 *         a. Validate that only low p. packets are dropped, according to S_RED thresholds.
 *         b. On port_1 the traffic bandwidth  should be X%     (only green pass)
 *         c. On port_2 the traffic bandwidth  should be (1-X)% (only green pass)
 *
 * Output example A:
 *    1. 10Gb port
 *    2. X = 90%
 *    3. for out_port, tc_1: EGQ_PQP_UNICAST_BYTES_COUNTER = 9.000387 Gbps
 *    4. for out_port, tc_2: EGQ_PQP_UNICAST_BYTES_COUNTER = 1.027330 Gbps
 *
 * Output example B:
 *    1. 10Gb port
 *    2. X = 70%
 *    3. for out_port, tc_1: EGQ_PQP_UNICAST_BYTES_COUNTER = 6.975541 Gbps
 *    4. for out_port, tc_2: EGQ_PQP_UNICAST_BYTES_COUNTER = 2.985351 Gbps
 *
 *
 * All the configuration example is in config_system_red_param():
 * Relevant soc properties configuration:
 *   system_red_enable=1
 *
 * Cint running:
  cd ../../../../src/examples/dpp
  cint cint_system_red.c
  cint
  int out_port_voq = 144;
  config_system_red_param_app(unit, out_port_voq);
 *
 *
 * System RED API using example:
 *   1. Setting Dbuff, BDB thresholds:
 *        Usage example: 
           system_red_buff_thrs_example(unit, 144);
 *
 * Remarks:
 *   1. Due to Arad-A0 Bug aging is supported only from Arad-B0.
 *   2. config_system_red_param() thresholds  adjusted to 10Gbps port.
 *   3. Dbuff, BDB thresholds are global, there for API does not use gport or cosq (need to insert valid values).
 *   4. In bcm_cosq_gport_threshold_set(), number of values (thresholds) is 4, and number of dps id 3 (4 bit Source-Q-size)
 *   5. formula for system RED drop probability:
 *     a. via bcm_cosq_gport_discard_set(), gport_size.size_max we are configuring DrpProb.
 *     b. drop probability = 1 - DrpProb / (64K - 1))
 *     c. DrpProb = 1 ==> drop-probability is not 0 but almost always drop.
 *     d. DrpProb = 64K - 1 ==> drop-probability = 1 (100%).
 *   6. Not validated for PB.
 *
 *
 * Relevant Registers/Tables:
   g IPS_IPS_GENERAL_CONFIGURATIONS 
   g IPS_SYSTEM_RED_AGING_CONFIGURATION
   g SCH_SYSTEM_RED_CONFIGURATION
   
   d IQM_SRCQRNG

   g IQM_FREE_UNICAST_DBUFF_RANGE_VALUES 
   g IQM_FREE_UNICAST_DBUFF_THRESHOLD_0 
   g IQM_FREE_UNICAST_DBUFF_THRESHOLD_1 
   g IQM_FREE_UNICAST_DBUFF_THRESHOLD_2 
   g IQM_FREE_FULL_MULTICAST_DBUFF_RANGE_VALUES 
   g IQM_FREE_FULL_MULTICAST_DBUFF_THRESHOLD_0 
   g IQM_FREE_FULL_MULTICAST_DBUFF_THRESHOLD_1 
   g IQM_FREE_FULL_MULTICAST_DBUFF_THRESHOLD_2  
   g IQM_FREE_BDB_RANGE_VALUES 
   g IQM_FREE_BDB_THRESHOLD_0 
   g IQM_FREE_BDB_THRESHOLD_1 
   g IQM_FREE_BDB_THRESHOLD_2 
   
   d IQM_SPRDPRM
   d IQM_SRDPRB
   
   d IPS_MAXQSZ
   
   PB:
     
   g FREEUNICASTDBUFFRANGEVALUES  
   g FREEUNICASTDBUFFTHRESHOLD0 
   g FREEUNICASTDBUFFTHRESHOLD1 
   g FREEUNICASTDBUFFTHRESHOLD2
   g FREEFULLMULTICASTDBUFFRANGEVALUES  
   g FREEFULLMULTICASTDBUFFTHRESHOLD0 
   g FREEFULLMULTICASTDBUFFTHRESHOLD1 
   g FREEFULLMULTICASTDBUFFTHRESHOLD2  
   g FREEBDBRANGEVALUES 
   g FREEBDBTHRESHOLD0 
   g FREEBDBTHRESHOLD1 
   g FREEBDBTHRESHOLD2 
   
   d SPRDPRM
   
     
   d SRCQRNG
   d SRDPRB
   d IPS_MAXQSZ
   
 */
 
 int config_system_red_param(int unit, int out_port_voq, int tc_1, int tc_2) {
  
    bcm_error_t rv = BCM_E_NONE ;
    int range_value_1[15]       ;
    int range_value_2[15]       ;
    
    int i;
    uint32 flags = 0x0;
    
    bcm_gport_t                 gport;
    bcm_cos_queue_t             cosq;
    bcm_fabric_config_discard_t discard;
    bcm_color_t                 color;    
    bcm_cosq_gport_size_t       gport_size;
    bcm_cosq_gport_discard_t    dp_discard;
    
    /* enable system RED */
    discard.flags = 0x0;
    discard.enable = 0x1;
    discard.age = 0x0;
    discard.age_mode = bcmFabricAgeModeReset;  /* bcmFabricAgeModeDecrement; */
    rv = bcm_fabric_config_discard_set(unit, discard);
    if (rv != BCM_E_NONE) {
        printf("Error, bcm_fabric_config_discard_set(), rv=%d.\n", rv);
        return rv;
    }
    
    /* IQM configurations */
    
    /* 1. set Q size levels */ 
    range_value_1[14] = 14000;     
    range_value_1[13] = 13000;     
    range_value_1[12] = 12000;     
    range_value_1[11] = 11000;     
    range_value_1[10] = 10000;     
    range_value_1[9 ] = 9000 ;     
    range_value_1[8 ] = 8000 ;     
    range_value_1[7 ] = 7000 ;     
    range_value_1[6 ] = 6000 ;     
    range_value_1[5 ] = 5000 ;     
    range_value_1[4 ] = 4000 ;     
    range_value_1[3 ] = 3000 ;     
    range_value_1[2 ] = 2000 ;     
    range_value_1[1 ] = 1000 ;     
    range_value_1[0 ] = 0000 ;     
    
    range_value_2[14] = 14000; 
    range_value_2[13] = 13000; 
    range_value_2[12] = 12000; 
    range_value_2[11] = 11000; 
    range_value_2[10] = 10000; 
    range_value_2[9 ] = 9000 ; 
    range_value_2[8 ] = 8000 ; 
    range_value_2[7 ] = 7000 ; 
    range_value_2[6 ] = 6000 ; 
    range_value_2[5 ] = 5000 ; 
    range_value_2[4 ] = 4000 ; 
    range_value_2[3 ] = 3000 ; 
    range_value_2[2 ] = 2000 ; 
    range_value_2[1 ] = 1500 ; 
    range_value_2[0 ] = 500 ; 

    flags = BCM_COSQ_GPORT_SIZE_COLOR_SYSTEM_RED;
    
    BCM_GPORT_UNICAST_QUEUE_GROUP_SET(gport, out_port_voq);
    
    cosq = tc_1;
    for (i=0 ; i < 15 ; i++) {
        color = i; /* 0-14 */    
        gport_size.size_max = range_value_1[i];
        rv = bcm_cosq_gport_color_size_set(unit, gport, cosq, color, flags, &gport_size);
        if (rv != BCM_E_NONE) {
            printf("Error, bcm_cosq_gport_color_size_set(), rv=%d.\n", rv);
            return rv;
        }
    }
    
    cosq = tc_2;
    for (i=0 ; i < 15 ; i++) {
        color = i; /* 0-15 */    
        gport_size.size_max = range_value_2[i];
        rv =  bcm_cosq_gport_color_size_set(unit, gport, cosq, color, flags, &gport_size);
        if (rv != BCM_E_NONE) {
            printf("Error, bcm_cosq_gport_color_size_set(), rv=%d.\n", rv);
            return rv;
        }
    }
    
    /* 2. set SRED parameters */
    dp_discard.gain = 0;
    BCM_GPORT_UNICAST_QUEUE_GROUP_SET(gport, out_port_voq);
        
    /* 
     * rate_class = 1; DP = 0 --> admit always
     * rate_class = 2; DP = 0 --> admit always
     */
    dp_discard.flags = BCM_COSQ_DISCARD_SYSTEM | BCM_COSQ_DISCARD_ENABLE | BCM_COSQ_DISCARD_PROBABILITY1 | BCM_COSQ_DISCARD_COLOR_GREEN;
    dp_discard.drop_probability = 0xffff; 
    dp_discard.min_thresh = 0xd; /* adm */
    dp_discard.max_thresh = 0xd; /* prb */
    cosq = tc_1; 
    rv = bcm_cosq_gport_discard_set(unit, gport, cosq, &dp_discard);
    if (rv != BCM_E_NONE) {
        printf("Error, bcm_cosq_gport_discard_set(), rv=%d.\n", rv);
        return rv;
    }
    cosq = tc_2;  
    rv = bcm_cosq_gport_discard_set(unit, gport, cosq, &dp_discard);
    if (rv != BCM_E_NONE) {
        printf("Error, bcm_cosq_gport_discard_set(), rv=%d.\n", rv);
        return rv;
    }
    
    dp_discard.flags = BCM_COSQ_DISCARD_SYSTEM | BCM_COSQ_DISCARD_ENABLE | BCM_COSQ_DISCARD_PROBABILITY2 | BCM_COSQ_DISCARD_COLOR_GREEN;
    dp_discard.drop_probability = 0xff00;
    dp_discard.min_thresh = 0xd; /* prb */
    dp_discard.max_thresh = 0xe; /* drp */
    cosq = tc_1; 
    rv = bcm_cosq_gport_discard_set(unit, gport, cosq, &dp_discard);
    if (rv != BCM_E_NONE) {
        printf("Error, bcm_cosq_gport_discard_set(), rv=%d.\n", rv);
        return rv;
    }
    
    cosq = tc_2; 
    rv = bcm_cosq_gport_discard_set(unit, gport, cosq, &dp_discard);
    if (rv != BCM_E_NONE) {
        printf("Error, bcm_cosq_gport_discard_set(), rv=%d.\n", rv);
        return rv;
    }
    
    /* 
     * rate_class = 1; DP = 1 --> drop  always
     * rate_class = 2; DP = 1 --> drop  always 
     */
    dp_discard.flags = BCM_COSQ_DISCARD_SYSTEM | BCM_COSQ_DISCARD_ENABLE | BCM_COSQ_DISCARD_PROBABILITY1 | BCM_COSQ_DISCARD_COLOR_YELLOW;
    dp_discard.drop_probability = 0xff;
    dp_discard.min_thresh = 0x7; /* adm */
    dp_discard.max_thresh = 0x7; /* prb */
    cosq = tc_1;
    rv = bcm_cosq_gport_discard_set(unit, gport, cosq, &dp_discard);
    if (rv != BCM_E_NONE) {
        printf("Error, bcm_cosq_gport_discard_set(), rv=%d.\n", rv);
        return rv;
    }
    cosq = tc_2;
    rv = bcm_cosq_gport_discard_set(unit, gport, cosq, &dp_discard);
    if (rv != BCM_E_NONE) {
        printf("Error, bcm_cosq_gport_discard_set(), rv=%d.\n", rv);
        return rv;
    }
    
    dp_discard.flags = BCM_COSQ_DISCARD_SYSTEM | BCM_COSQ_DISCARD_ENABLE | BCM_COSQ_DISCARD_PROBABILITY2 | BCM_COSQ_DISCARD_COLOR_YELLOW;
    dp_discard.drop_probability = 0x1;
    dp_discard.min_thresh = 0x7; /* prb */
    dp_discard.max_thresh = 0x8; /* drp */
    cosq = tc_1;
    rv = bcm_cosq_gport_discard_set(unit, gport, cosq, &dp_discard);
    if (rv != BCM_E_NONE) {
        printf("Error, bcm_cosq_gport_discard_set(), rv=%d.\n", rv);
        return rv;
    }
    cosq = tc_2;
    rv = bcm_cosq_gport_discard_set(unit, gport, cosq, &dp_discard);
    if (rv != BCM_E_NONE) {
        printf("Error, bcm_cosq_gport_discard_set(), rv=%d.\n", rv);
        return rv;
    }
      
    return rv;
};
 

/*
 * Example for System RED application
 */
 
int config_system_red_param_app(int unit, int out_port_voq) {
    
    int 
        rv = BCM_E_NONE,
        tc_1,
        tc_2;
        
        
    tc_1 = 0;
    tc_2 = 7;
    
    rv = config_system_red_param(unit, out_port_voq, tc_1, tc_2);
    if (rv != BCM_E_NONE) {
        printf("Error, config_system_red_param(), rv=%d.\n", rv);
        return rv;
    }
    
    return rv;
}
    
/*
 * System RED API using example - Setting Dbuff, BDB thresholds
 */
int system_red_buff_thrs_example(int unit, int out_port_voq)
{
    int 
        rv = BCM_E_NONE,
        i,
        val;
    bcm_gport_t          gport;
    bcm_cos_queue_t      cosq;
    bcm_cosq_threshold_t threshold;

    /* API does not use gport or cosq */
    BCM_GPORT_UNICAST_QUEUE_GROUP_SET(gport, out_port_voq);
    cosq = 2;
    
    for (i=0; i < 12; i++) {
    
        val = i;
        
        if (i < 4) {
            threshold.type = bcmCosqThresholdDbuffs;
        } else if (i < 8) {
            threshold.type = bcmCosqThresholdFullDbuffs;
        } else if (i < 12) {
            threshold.type = bcmCosqThresholdBufferDescriptorBuffers;
        } 
        
        if (i % 4 == 0x0) {
            threshold.flags = BCM_COSQ_THRESHOLD_COLOR_SYSTEM_RED | BCM_COSQ_THRESHOLD_RANGE_0;
        } else if (i % 4 == 0x1) {
            threshold.flags = BCM_COSQ_THRESHOLD_COLOR_SYSTEM_RED | BCM_COSQ_THRESHOLD_RANGE_1;
        } else if (i % 4 == 0x2) {
            threshold.flags = BCM_COSQ_THRESHOLD_COLOR_SYSTEM_RED | BCM_COSQ_THRESHOLD_RANGE_2;
        } else if (i % 4 == 0x3) {
            threshold.flags = BCM_COSQ_THRESHOLD_COLOR_SYSTEM_RED | BCM_COSQ_THRESHOLD_RANGE_3;
        }

        /* Get value Before */        
        threshold.value = 0;
        threshold.dp = 0;
        rv = bcm_cosq_gport_threshold_get(unit, gport, cosq, &threshold);
        if (rv != BCM_E_NONE) {
            printf("Error, bcm_cosq_gport_threshold_get(), rv=%d.\n", rv);
            return rv;
        }
        print threshold;
        
        threshold.value = 1 << val;
        threshold.dp = val++;
        printf("i=%x, val=0x%x, threshold.flags=0x%x, threshold.value=0x%x\n", i, val, threshold.flags, threshold.value);    
        rv = bcm_cosq_gport_threshold_set(unit, gport, cosq, &threshold);
        if (rv != BCM_E_NONE) {
            printf("Error, bcm_cosq_gport_threshold_get(), rv=%d.\n", rv);
            return rv;
        }
        
        /* Get value After */
        threshold.value = 0;
        threshold.dp = 0;
        rv = bcm_cosq_gport_threshold_get(unit, gport, cosq, &threshold);
        if (rv != BCM_E_NONE) {
            printf("Error, bcm_cosq_gport_threshold_get(), rv=%d.\n", rv);
            return rv;
        }
        print threshold;
    }
    
    return rv;
}

