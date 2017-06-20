/*
 * $Id: cint_sirius_setup.c,v 1.39 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * Configuration example of 2xScorpion+4xSirius+2xPolaris
 * The script includes Sirius configuration only.
 * Scorpion | Scorpion
 * MODID    | Frontpanel
 *   28
 */

int           rv; 
int           num_child_gport;
int           num_egress_child_gport;
int           num_uc_gport = 0;
bcm_gport_t   *child_gport_id = NULL;
bcm_gport_t   *egress_gport_id = NULL;
bcm_gport_t   *uc_gport = NULL;
bcm_gport_t   *fifo_gport = NULL;
bcm_gport_t   mc_gport;
int           num_hg_intf;
uint32        *hg_subports = NULL;
int           TRUE = 1;

bcm_port_congestion_config_t sirius_congestion_info1;

int           COSQ_GPORT_WITH_ID = 1;


int
bcm88230_common_config_vars(int unit)
{
    sal_config_set("bcm_num_cos", "8");
    sal_config_set("qe_tme_mode", "0");
    sal_config_set("fabric_configuration", "1");
    sal_config_set("dual_local_grants", "1");

    return 0;
}

/* 
 * bcm88230_module_add() is the main routine for Sirius
 *
 * node_id      :  Local Node ID of the current device
 *      
 * remote_node_id : Any other Node ID to which PP is connected to in a 
 *                  multi-node system. Need to set queue on multi devices.
 *      
 * module_id_pp :  Module ID for Packet processor 
 *
 * base_port_pp :  Base port number on Packet Processor
 *
 * num_ports_pp :  Number of front panel ports on PP
 *
 */ 
int
bcm88230_module_add(int unit,
                    int node_id,
                    int remote_node_id,
                    int base_fabric_port,
                    bcm_module_t module_id_pp,
                    int base_port_pp,
                    int num_ports_pp,
                    int gport_flags)
{
    int                port;
    bcm_gport_t        fabric_gport;
    bcm_gport_t        switch_gport;
    bcm_module_t       modid;
    int                num_cos;
    int                flags;
    int                speed;
    int                cos;
    bcm_gport_t        egress_gport;
    bcm_multicast_t    mcgroup;
 
    modid = BCM_STK_NODE_TO_MOD(remote_node_id);

    /* BCM_IF_ERR0R_RETURN( */
    bcm_cosq_config_get(unit, &num_cos);

    /*
     * Switch to Fabric gport mapping
     *
     * Add UC queue group
     */
    for (port = 0; port < num_ports_pp; port++) {
         
         BCM_GPORT_CHILD_SET(fabric_gport, modid, (base_fabric_port + port));
         child_gport_id[port] = fabric_gport;
         BCM_GPORT_MODPORT_SET(switch_gport, module_id_pp, (base_port_pp + port));

         rv = bcm_stk_fabric_map_set(unit, switch_gport, fabric_gport);
         if (rv != BCM_E_NONE) {
             LOG_CLI((BSL_META_U(unit,
                                 "bcm_stk_fabric_map_set failed err=%d %s\n"), rv, bcm_errmsg(rv)));
             return rv;
         }
 
         printf("Mapping modid %d port %i, Switch Gport: 0x%x, Fabric Port: 0x%x\n",
                        module_id_pp, (base_port_pp + port), switch_gport, fabric_gport);

         flags = 0;
         if (gport_flags == COSQ_GPORT_WITH_ID) {
             BCM_GPORT_UCAST_QUEUE_GROUP_SET(uc_gport[num_uc_gport],
                                             (remote_node_id * 50 + port) * num_cos);
             flags = BCM_COSQ_GPORT_WITH_ID;
         }
         printf("Base queue: 0x%x \n", uc_gport[num_uc_gport]);

         rv = bcm_cosq_gport_add(unit, child_gport_id[port], num_cos, flags, 
                                 &uc_gport[num_uc_gport++]);
         if (rv != BCM_E_NONE) {
             printf("bcm_cosq_gport_add failed for Module %d Port %d\n",
                     module_id_pp, (base_port_pp + port));
             return rv;
         }

         /*
          * Shaper config: only for local fabric ports
          */
         if (node_id == remote_node_id) {
             speed = soc_property_port_get(unit, port,
                                           spn_PORT_INIT_SPEED, 1000);
             printf("Shaper: port %d speed %d\n", port, speed);
             BCM_GPORT_EGRESS_GROUP_SET(egress_gport, modid,
                                        (base_fabric_port + port));
             for (cos = 0; cos < 4; cos++) {
                  rv = bcm_cosq_gport_bandwidth_set(unit, egress_gport, cos,
                                                    0, speed * 1000 * 1000, 0);
             }

             /* port shaper */
             BCM_GPORT_EGRESS_CHILD_SET(egress_gport, modid, (base_fabric_port + port));
             rv = bcm_cosq_gport_bandwidth_set(unit, egress_gport, 0,
                                                       0, speed * 1000 * 1000, 0);
             if (rv == BCM_E_NONE) {
                 /*
                  * add ports to multicast group 
                  */
                 mcgroup = 1;
                 rv = bcm_multicast_egress_add(unit, mcgroup, fabric_gport,
                                                  0 /* encap_id */);
                 if (rv != BCM_E_NONE) {
                     printf("Error adding ports to the multicast group error(%d)\n", rv);
                     return rv;
                 }
             }                   
         }

    }

    return 0; 
}

int
bcm88230_multicast_setup(int unit, int modid)
{
    int                   cos;
    bcm_port_config_t     pcfg;
    bcm_gport_t           internal_mcport;
    bcm_port_t            port;
    bcm_gport_t           gport;
    bcm_multicast_t       mcgroup;

    /* create one internal multicast fabric port */
    sal_memset(&pcfg, 0, sizeof(pcfg));
    BCM_IF_ERROR_RETURN(bcm_port_config_get(unit, &pcfg));

    /*
    rv = bcm_fabric_port_create(unit, gport, -1,
                                BCM_FABRIC_PORT_EGRESS_MULTICAST,
                                &internal_mcport);
    */
    /* shape the internal port: 10G */
    if (rv == BCM_E_NONE) {
        /*
        printf("Created internal multicast fabric port 0x%x\n", internal_mcport); 
        for (cos = 0; cos < 4; cos++) {
             BCM_IF_ERROR_RETURN
                (bcm_cosq_gport_bandwidth_set(unit, internal_mcport, 
                                              cos, 0, 
                                              10000 * 1000 * 1000, 0));
        }
        */
    }

    /* add Multicast group 1 */
    mcgroup = 1;
    rv = bcm_multicast_create(unit, BCM_MULTICAST_WITH_ID, &mcgroup);
    if (rv != BCM_E_NONE) {
        printf("Error creating multicast group\n");
        return rv;
    }

    return rv;
}

int
bcm88230_flow_control_setup(int          unit,
                            int          node,
                            bcm_module_t module_pp, 
                            bcm_port_t   hg_port, 
                            int          fifo_mask0,
                            int          fifo_mask1,
                            int          fifo_mask2,
                            int          fifo_mask3)

{
    int                          fifo_mask[4];
    int                          fifo;  
    bcm_gport_t                  gport; 
    bcm_module_t                 modid;
  
    fifo_mask[0] = fifo_mask0;
    fifo_mask[1] = fifo_mask1;
    fifo_mask[2] = fifo_mask2;
    fifo_mask[3] = fifo_mask3;
   
    for (fifo = 0; fifo < 4; fifo++) {
         rv = bcm_cosq_gport_flow_control_set(unit,
                                              BCM_GPORT_INVALID,
                                              fifo,
                                              fifo_mask[fifo]);
         if (rv != BCM_E_NONE) {
             printf("bcm_cosq_gport_flow_control_set failed fifo %d\n",
                    fifo);
             return rv;
         }
    }

    /* flow control message size */
    rv = bcm_fabric_congestion_size_set(unit, module_pp, 64);
    if (rv != BCM_E_NONE) {
        printf("  bcm_fabric_congestion_size_set FAILED(%d, 0x%x)\n", module_pp, rv);
        return(rv);
    }

    modid = BCM_STK_NODE_TO_MOD(node);
    BCM_GPORT_MODPORT_SET(gport, modid, hg_port);

    BCM_GPORT_MODPORT_SET(sirius_congestion_info1.src_port, 
                          module_pp, 
                          0);
    rv = bcm_port_congestion_config_set(unit, gport, &sirius_congestion_info1);
    if (rv != BCM_E_NONE) {
        printf("  bcm_port_congestion_config_set FAILED(%d, 0x%x)\n", rv, rv);
        return(rv);
    }

    return 0; 
}

/*
 * Global setup
 */
int
bcm88230_global_setup(int unit, 
                      int node,
                      int numcos, int total_num_fp)
{
    bcm_module_t                modid;
    int                         intf;
    bcm_port_config_t           pcfg;
    bcm_fabric_distribution_t   ds_id;
    bcm_multicast_t             mcgroup;
    bcm_port_t                  port;
    uint64                      crossbar_mask = "0x3fffff";
    uint32                      foo;
    int                         flags;

    modid = BCM_STK_NODE_TO_MOD(node);
    bcm_stk_modid_set(unit, modid);

    bcm_cosq_config_set(unit, numcos);

    bcm_stk_module_protocol_set(unit, modid, bcmModuleProtocol3);

    /*
     **** SerDes Config
     */ 
    sal_memset(&pcfg, 0, sizeof(pcfg));
    bcm_port_config_get(unit, &pcfg);

    BCM_PBMP_ITER(pcfg.sci, port) {
            bcm_port_control_set(unit, port, 
                         bcmPortControlAbility, 
                         BCM_PORT_ABILITY_SCI);
            bcm_port_enable_set(unit, port, TRUE);
    }

    BCM_PBMP_ITER(pcfg.sfi, port) {
          bcm_port_control_set(unit, port, 
                               bcmPortControlAbility, 
                               BCM_PORT_ABILITY_DUAL_SFI);
          bcm_port_enable_set(unit, port, TRUE);
     }

    bcm_fabric_crossbar_enable_set(unit, crossbar_mask);

    if (hg_subports == NULL) {

        BCM_PBMP_COUNT(pcfg.hg, num_hg_intf);
        hg_subports = sal_alloc(sizeof(foo) * num_hg_intf, "hg_subports");
        if (hg_subports == NULL) {
            return -1;
        }
    }
/*

    total_num_fp = 0;
    for (intf = 0; intf < num_hg_intf; intf++) {
         hg_subports[intf] = soc_property_port_get(unit, intf, spn_IF_SUBPORTS, 0);
         total_num_fp += hg_subports[intf];
    }
*/

    if (child_gport_id == NULL) {
        child_gport_id = sal_alloc(sizeof(mc_gport) * total_num_fp, "child gport");
        if (child_gport_id ==  NULL) {
            return -1;
        }
    }

    if (egress_gport_id == NULL) {
        egress_gport_id = sal_alloc(sizeof(mc_gport) * total_num_fp, "egress gport");
        if (egress_gport_id ==  NULL) {
            return -1;
        }
    }

    if (uc_gport == NULL) {
        uc_gport = sal_alloc(sizeof(uc_gport) * total_num_fp, "uc_gport");
        if (uc_gport ==  NULL) {
            return -1;
        }
    }

    if (fifo_gport == NULL) { 
        fifo_gport = sal_alloc(sizeof(mc_gport) * total_num_fp, "fifo_gport");
        if (fifo_gport ==  NULL) {
            return -1;
        }
    }

    /*
     * Add a UC queue group for CPU
     *
     * Add a MC queue group
     */
    mcgroup = 1;
    /* setup multicast logical port - create queue group first */
    ds_id = 0;
    flags = 0; 
    rv = bcm_cosq_fabric_distribution_add(unit, ds_id, numcos,
                                          flags, &mc_gport);
    if (rv != BCM_E_NONE) {
        printf("Error creating multicast fabric distribution queue group error(%d)\n", rv);
        return rv;
    }

    rv = bcm_multicast_fabric_distribution_set(unit, mcgroup, ds_id);
    if (rv != BCM_E_NONE) {
        printf("Error associating multicast group with ds_id\n");
        return rv;
    }

    /*
     * Miscellaneous config
     */
    rv = bcm_switch_control_set(unit, bcmSwitchPktAge, 0);
    if (rv != BCM_E_NONE) {
        printf("bcm_fabric_control_set bcmSwitchPktAge failed\n");
        return rv; 
    }

    /*
     * Set fabric redundancy mode
     * (Note: sets TIMESLOT_BURST_SIZE_BYTES, GG_CI_BP_BS*, GG_QM_BP_BS*).
     */ 
    /* rv = bcm_fabric_control_set(unit, bcmFabricRedundancyMode, bcmFabricRedManual );
    if (rv != BCM_E_NONE) {
        printf("bcm_fabric_control_set bcmFabricRedManual  failed\n");
        return rv; 
    }
    */

    /* initialize fc struct */
    /* Following is the corresponding configuration on Packet Processor. */
    /* the configuration is shown in the BE format/Network order */
    /* HG Header */
    /*   0_0 => 0xFB (K.SOP) */ 
    /*   0_1 => 0x11 (MC, TC=1) TC is 4-bits field and MC is one bit. MASK for TC is 4-bits*/
    /*   0_2 => 0x00 (MGID msb) */
    /*   0_3 => 0x00 (MGID lsb) */
    /*   1_0 => 0x1C (SRC_MODID) */
    /*   1_1 => 0x00 (SRC_PID) */ 
    /*   1_2 => 0x00 (Dont Care) */
    /*   1_3 => 0x00 (DP=0) */
    /*   2_[0-3] => 0x00000000 (Dont Care) */
    /*   3_[0-3] => 0x00000000 (Dont Care) */
    /* E2ECC Header */
    /*   4_0 => 01 (macda[47-40] */
    /*   4_1 => 30 (macda[39-32] */
    /*   4_2 => 20 (macda[31-24] */
    /*   4_3 => 18 (macda[23-16] */
    /*   5_0 => 10 (macda[15-8] */
    /*   5_1 => 00 (macda[7-0] */
    /*   5_2 => 00 (macsa[47-40] (Dont Care) */
    /*   5_3 => 30 (macsa[39-32] (Dont Care) */
    /*   6_0 => 20 (macsa[31-24] (Dont Care) */
    /*   6_1 => 18 (macsa[23-16] (Dont Care) */
    /*   6_2 => 10 (macsa[15-8] (Dont Care) */
    /*   6_3 => 00 (macsa[7-0] (Dont Care) */
    /*   7_0 => 0x20 (ethertype msb) */
    /*   7_1 => 0x02 (ethertype lsb) */
    /*   7_2 => 0x02 (opcode msb) */
    /*   7_3 => 0x20 (opcode lsb) */

    sal_memset(&sirius_congestion_info1, 0, sizeof(sirius_congestion_info1));
    sirius_congestion_info1.flags = BCM_PORT_CONGESTION_CONFIG_E2ECC;
    /* sirius_congestion_info1.src_port = "0x1c00"; */
    sirius_congestion_info1.traffic_class = 1;
    sirius_congestion_info1.color=0;
    
    sirius_congestion_info1.src_mac = "AA:AA:AA:AA:AA:AA"; 
    sirius_congestion_info1.dest_mac = "00:10:18:20:30:01";
   
    sirius_congestion_info1.ethertype=0x2002;
    sirius_congestion_info1.opcode=0x220;

    rv = bcm_fabric_control_set(unit, bcmFabricRedundancyMode, bcmFabricRedManual);
   
    return rv;
}

/*
 * bcm88230_2xScorpion_Setup
 *
 * Top level function to configure a Sirius device in
 * a "2xScoprion + 4xSirius + 2xPolaris"  System
 *
 */
int
bcm88230_2xScorpion_Setup(int unit, int local_node_id)
{
    int               node_ids[4] = {28, 24, 20, 16};
    bcm_module_t      module_ids[2] = {28, 24};
    int               base_fabric_port[4] = {1, 17, 1, 17};
    int               module;

    bcm88230_common_config_vars(unit);

    /* Need 6.5 Gbps serdes for Sco */
    sal_config_set("backplane_serdes_speed", "6500");
    sal_config_set("backplane_serdes_encoding", "0");

    rv = soc_init(unit); 
    if (rv != BCM_E_NONE) {
        printf("SOC init failed %d: %s\n", rv, bcm_errmsg(rv));
        return rv; 
    }
    
    rv = bcm_init(unit); 
    if (rv != BCM_E_NONE) {
        printf("BCM init failed %d: %s\n", rv, bcm_errmsg(rv));
        return rv; 
    }

    rv = bcm88230_global_setup(unit, local_node_id, 8, 32);
    if (rv != 0) {
        printf("bcm88230_global_setup failed on device %d node %d\n",
               unit, local_node_id);
        return rv;                
    } 

    if (rv == BCM_E_NONE) {
        rv = bcm88230_multicast_setup(unit,
                                      BCM_STK_NODE_TO_MOD(local_node_id));
    }

    /*
     * Add all scorpion modules 
     */
    for (module = 0; module < 2; module++) {
          
         rv = bcm88230_module_add(unit, 
                             local_node_id, /* local node ID */
                             node_ids[2*module], /* remote node ID */
                             0,  /* base fabric port # */
                             module_ids[module], /* Sco module ID */
                             base_fabric_port[2*module],  /* base port # */
                             8, /* # of ports  */
                             -1);
         if (rv != 0) {
             return rv;
         }
 
         rv = bcm88230_module_add(unit, 
                             local_node_id, /* local node ID */
                             node_ids[2*module+1], /* remote node ID */
                             0,  /* base fabric port # */
                             module_ids[module], /* Sco module ID */
                             base_fabric_port[2*module+1],  /* base port # */
                             8, /* # of ports  */
                             -1);
         if (rv != 0) {
             return rv;
         }

         rv = bcm88230_flow_control_setup(unit,
                                          local_node_id,
                                          module_ids[module],
                                          1   /* hg_port    */,
                                          0x2 /* fifo_mask0, Sirius Cos0 (uc-ef),  Scorpion Cos1 */,
                                          0x1 /* fifo_mask1, Sirius Cos1 (uc-nef), Scorpion Cos0 */,
                                          0x4 /* fifo_mask2, Sirius Cos2 (mc-ef),  Scorpion Cos2 */,
                                          0x4 /* fifo_mask3, Sirius Cos3 (mc-nef), Scorpion Cos2 */);
         if (rv != 0) {
             return rv;
         }


    } /* for each module */ 

    printf("Done!\n");

    return rv;
}

/*
 * bcm88230_1xScorpion_Setup
 *
 * Top level function to configure a Sirius device in
 * a "1xScoprion + 2xSirius + 1xPolaris"  System
 *
 */
int
bcm88230_1xScorpion_Setup(int unit, int local_node_id)
{
    int               node_ids[2] = {28, 24};
    bcm_module_t      module_id = 28;
    int               base_fabric_port[2] = {1, 17};
    int               module;
    int               remote_node_id;
    int               remote_num_ports;

    bcm88230_common_config_vars(unit);

    /* Need 6.5 Gbps serdes for Sco */
    sal_config_set("backplane_serdes_speed", "6500");
    sal_config_set("backplane_serdes_encoding", "0");

    rv = soc_init(unit); 
    if (rv != BCM_E_NONE) {
        printf("SOC init failed %d: %s\n", rv, bcm_errmsg(rv));
        return rv; 
    }
    
    rv = bcm_init(unit); 
    if (rv != BCM_E_NONE) {
        printf("BCM init failed %d: %s\n", rv, bcm_errmsg(rv));
        return rv; 
    }

/*bcm88230_global_setup(int unit, int node, int numcos, int total_num_fp)*/
    rv = bcm88230_global_setup(unit, local_node_id, 8, 16);
    if (rv != 0) {
        printf("bcm88230_global_setup failed on device %d node %d\n",
               unit, local_node_id);
        return rv;                
    } 

    if (rv == BCM_E_NONE) {
        rv = bcm88230_multicast_setup(unit,
                                      BCM_STK_NODE_TO_MOD(local_node_id));
    }

    rv = bcm88230_module_add(unit, 
                             local_node_id, /* local node ID */
                             node_ids[0], /* remote node ID */
                             0,  /* base fabric port # */
                             module_id, /* Sco module ID */
                             base_fabric_port[0],  /* base port # */
                             8, /* # of ports  */
                             -1);
    if (rv != 0) {
        return rv;
    }
 
    rv = bcm88230_module_add(unit, 
                            local_node_id, /* local node ID */
                             node_ids[1], /* remote node ID */
                             0,  /* base fabric port # */
                             module_id, /* Sco module ID */
                             base_fabric_port[1],  /* base port # */
                             8, /* # of ports  */
                             -1);
    if (rv != 0) {
        return rv;
    }

    rv = bcm88230_flow_control_setup(unit,
                                     local_node_id,
                                     module_id,
                                     1   /* hg_port    */,
                                     0x2 /* fifo_mask0, Sirius Cos0 (uc-ef),  Scorpion Cos1 */,
                                     0x1 /* fifo_mask1, Sirius Cos1 (uc-nef), Scorpion Cos0 */,
                                     0x4 /* fifo_mask2, Sirius Cos2 (mc-ef),  Scorpion Cos2 */,
                                     0x4 /* fifo_mask3, Sirius Cos3 (mc-nef), Scorpion Cos2 */);
    if (rv != 0) {
        return rv;
    }

    
    printf("Done!\n");

    return rv;
}

int
bcm88230_hexapod_1xSco_Setup(void)
{
    bcm_port_t            port;
    bcm_gport_t           gport;
    int                   tid;
    bcm_trunk_info_t  ta_info;
    bcm_trunk_chip_info_t ta_chip_info;
    int                   unit;
    int                   node_ids[2] = {28, 24};
    int                   i = 0;
    bcm_port_config_t     pcfg;
    bcm_module_t          modid;
 
    /* BCM_IF_ERROR_RETURN */
      bcm_stk_modid_set(0, 
                          BCM_STK_NODE_TO_MOD(node_ids[0]));
    /* BCM_IF_ERROR_RETURN */
       bcm_stk_modid_set(1, 
                          BCM_STK_NODE_TO_MOD(node_ids[1]));

    unit = 0;/* unit=0,1 are BCM88230*/
    rv = bcm_trunk_chip_info_get(unit, &ta_chip_info);
    if (rv != BCM_E_NONE) {
        printf("Error (%d) in bcm_trunk_chip_info_get\n", rv);
        return rv;
    }

    tid = ta_chip_info.trunk_fabric_id_min;

    BCM_IF_ERROR_RETURN(bcm_port_config_get(unit, &pcfg));
    BCM_PBMP_ITER(pcfg.hg, port) {
       modid = BCM_STK_NODE_TO_MOD(node_ids[0]);
       BCM_GPORT_MODPORT_SET(gport, modid, port);
       ta_info.tm[i] = BCM_STK_NODE_TO_MOD(node_ids[0]);
       ta_info.tp[i] = gport;
       i++;

       modid = BCM_STK_NODE_TO_MOD(node_ids[1]);
       BCM_GPORT_MODPORT_SET(gport, modid, port);
       ta_info.tm[i] = BCM_STK_NODE_TO_MOD(node_ids[1]);
       ta_info.tp[i] = gport;
       i++;
    }
    ta_info.num_ports = i;

    for (unit = 0; unit < 2; unit++) { 
         rv = bcm_trunk_create(unit, BCM_TRUNK_FLAG_WITH_ID, &tid);
         if (rv != BCM_E_NONE) {
             printf("Error (%d) in bcm_trunk_create_id\n", rv);
            return rv;
         }

         rv = bcm_trunk_set(unit, tid, &ta_info);
         if (rv != BCM_E_NONE) {
             printf("Error (%d) in bcm_trunk_set\n", rv);
             return rv;
         }

         rv = bcm88230_1xScorpion_Setup(unit, node_ids[unit]);
         if (rv != BCM_E_NONE) {
             printf("Unit %d: bcm88230_1xScorpion_Setup failed %d\n", unit, rv);
             return rv;
         }
    }

    return rv;
}

/*
 * bcm88230_1x56634_Setup
 *
 *     to set a system with 48-ports, 12-ports on each HG interface
 * local_node_id is the local BCM88230 device since it is for a single node 
 * and single BCM56634, the node_ids and local_node_id are similar
 */
int
bcm88230_1x56634_Setup(int unit, int local_node_id)
{
    int               node_ids=28;
    int               module_ids=28;
    int               base_fabric_port=0;
    int               module;
    int               tid;
    bcm_trunk_info_t  ta_info;
    bcm_trunk_chip_info_t ta_chip_info;
    bcm_port_config_t     pcfg;
    bcm_port_t            port;
    bcm_gport_t           gport;
    int                   i = 0;

    bcm88230_common_config_vars(unit);

    rv = soc_init(unit); 
    if (rv != BCM_E_NONE) {
        printf("SOC init failed %d: %s\n", rv, bcm_errmsg(rv));
        return rv; 
    }
    
    rv = bcm_init(unit); 
    if (rv != BCM_E_NONE) {
        printf("BCM init failed %d: %s\n", rv, bcm_errmsg(rv));
        return rv; 
    }

    bcm_stk_modid_set(unit, BCM_STK_NODE_TO_MOD(node_ids));
	
    rv = bcm_trunk_chip_info_get(unit, &ta_chip_info);
    if (rv != BCM_E_NONE) {
        printf("Error (%d) in bcm_trunk_chip_info_get\n", rv);
        return rv;
    }

    tid = ta_chip_info.trunk_fabric_id_min;
    BCM_IF_ERROR_RETURN(bcm_port_config_get(unit, &pcfg));
	
    BCM_PBMP_ITER(pcfg.hg, port) {
       BCM_GPORT_MODPORT_SET(gport, 
                             BCM_STK_NODE_TO_MOD(node_ids), port);
       ta_info.tm[i] = BCM_STK_NODE_TO_MOD(node_ids);
       ta_info.tp[i] = gport;
       i++;
    }
    ta_info.num_ports = i;

    rv = bcm_trunk_create(unit, BCM_TRUNK_FLAG_WITH_ID, &tid);
    if (rv != BCM_E_NONE) {
	printf("Error (%d) in bcm_trunk_create_id\n", rv);
	return rv;
    }

    rv = bcm_trunk_set(unit, tid, &ta_info);
    if (rv != BCM_E_NONE) {
        printf("Error (%d) in bcm_trunk_set\n", rv);
        return rv;
    }
	
    /*bcm88230_global_setup(int unit, int node, int numcos, int total_num_fp)*/
    rv = bcm88230_global_setup(unit, local_node_id, 8, 48);
    if (rv != 0) {
        printf("bcm88230_global_setup failed on device %d node %d\n",
               unit, local_node_id);
        return rv;                
    } 

    if (rv == BCM_E_NONE) {
        rv = bcm88230_multicast_setup(unit,
                                      BCM_STK_NODE_TO_MOD(local_node_id));
    }

    for (module = 0; module < 1; module++) {
         printf("module: %d",module); 
         rv = bcm88230_module_add(unit, 
                             local_node_id, /* local node ID */
                             node_ids, /* remote node ID */
                             0,  /* base fabric port # */
                             module_ids, /* T2 module ID */
                             base_fabric_port,  /* base port # */
                             48, /* # of ports  */
                             -1);
         if (rv != 0) {
             return rv;
         }
    } /* for each module */ 
    if (local_node_id==28) {
         rv = bcm88230_flow_control_setup(unit,
                                          local_node_id,
                                          module_ids,
                                          1   /* hg_port    */,
                                          0x2 /* fifo_mask0, Sirius Cos0 (uc-ef),  Scorpion Cos1 */,
                                          0x1 /* fifo_mask1, Sirius Cos1 (uc-nef), Scorpion Cos0 */,
                                          0x4 /* fifo_mask2, Sirius Cos2 (mc-ef),  Scorpion Cos2 */,
                                          0x4 /* fifo_mask3, Sirius Cos3 (mc-nef), Scorpion Cos2 */);
         if (rv != 0) {
             return rv;
         }
     }



    printf("Done!\n");

    return rv;
}

/*
 * bcm88230_1xCaladan2_Setup
 *
 * C2-Sirius Configuration: C2 has 24 front panel ports
 *
 */
int
bcm88230_1xCaladan2_Setup(int unit, 
                          int local_node_id,
                          bcm_module_t c2_module_id)
{
    int rv = BCM_E_NONE;

    bcm88230_common_config_vars(unit);
    sal_config_set("if_protocol", "1");

    rv = soc_init(unit); 
    if (rv != BCM_E_NONE) {
        printf("SOC init failed %d: %s\n", rv, bcm_errmsg(rv));
        return rv; 
    }
    
    rv = bcm_init(unit); 
    if (rv != BCM_E_NONE) {
        printf("BCM init failed %d: %s\n", rv, bcm_errmsg(rv));
        return rv; 
    }

    rv = bcm88230_global_setup(unit, local_node_id, 8, 24);
    if (rv != 0) {
        printf("bcm88230_global_setup failed on device %d node %d\n",
               unit, local_node_id);
        return rv;
    }

    if (rv == BCM_E_NONE) {
        rv = bcm88230_multicast_setup(unit,
                                      BCM_STK_NODE_TO_MOD(local_node_id));
    }

    rv = bcm88230_module_add(unit,
                             local_node_id, /* local node ID */
                             local_node_id, /* remote node ID */
                             0,  /* base fabric port # */
                             c2_module_id, /* C2 module ID */
                             0,  /* base port # */
                             24, /* # of ports  */
                             COSQ_GPORT_WITH_ID); 
    if (rv != 0) {
        return rv;
    }

    rv = bcm88230_flow_control_setup(unit,
                                     local_node_id,
                                     c2_module_id,
                                     32   /* or 33 hg_port    */,
                                     0x2 /* fifo_mask0, Sirius Cos0 (uc-ef),  Scorpion Cos1 */,
                                     0x1 /* fifo_mask1, Sirius Cos1 (uc-nef), Scorpion Cos0 */,
                                     0x4 /* fifo_mask2, Sirius Cos2 (mc-ef),  Scorpion Cos2 */,
                                     0x4 /* fifo_mask3, Sirius Cos3 (mc-nef), Scorpion Cos2 */);
    return rv;
}

int
bcm88230_2xc2_setup(int unit,
                    int local_node_id,
                    bcm_module_t c2_mod1,
                    bcm_module_t c2_mod2,
                    bcm_port_t hg_c2_mod1,
                    bcm_port_t hg_c2_mod2)
{
    int               node;
    int               node_ids[2] = {28, 4};
    int               base_fabric_port[2] = {0, 24};
    int               module;
    int               remote_node_id;
    int               remote_num_ports;

    bcm88230_common_config_vars(unit);
    sal_config_set("if_protocol", "1");

    rv = soc_init(unit); 
    if (rv != BCM_E_NONE) {
        printf("SOC init failed %d: %s\n", rv, bcm_errmsg(rv));
        return rv; 
    }
    
    rv = bcm_init(unit); 
    if (rv != BCM_E_NONE) {
        printf("BCM init failed %d: %s\n", rv, bcm_errmsg(rv));
        return rv; 
    }

    rv = bcm88230_global_setup(unit, local_node_id, 16);
    if (rv != 0) {
        printf("bcm88230_global_setup failed on device %d node %d\n",
               unit, local_node_id);
        return rv;
    }

    if (rv == BCM_E_NONE) {
        rv = bcm88230_multicast_setup(unit,
                                      BCM_STK_NODE_TO_MOD(local_node_id));
    }

    for (node = 0; node < 2; node ++) {
         rv = bcm88230_module_add(unit,
                             local_node_id, /* local node ID */
                             node_ids[node], /* remote node ID */
                             0,  /* base fabric port # */
                             c2_mod1, /* c2 module ID */
                             0,  /* base port # */
                             24); /* # of ports  */
         if (rv != 0) {
             return rv;
         }

         rv = bcm88230_module_add(unit,
                             local_node_id, /* local node ID */
                             node_ids[node], /* remote node ID */
                             24,  /* base fabric port # */
                             c2_mod2, /* c2 module ID */
                             0,  /* base port # */
                             24); /* # of ports  */
        if (rv != 0) {
            return rv;
        }
    }

    rv = bcm88230_flow_control_setup(unit,
                                     local_node_id,
                                     c2_mod1,
                                     hg_c2_mod1,   /* hg_port    */
                                     0x2 /* fifo_mask0, Sirius Cos0 (uc-ef),  Scorpion Cos1 */,
                                     0x1 /* fifo_mask1, Sirius Cos1 (uc-nef), Scorpion Cos0 */,
                                     0x4 /* fifo_mask2, Sirius Cos2 (mc-ef),  Scorpion Cos2 */,
                                     0x4 /* fifo_mask3, Sirius Cos3 (mc-nef), Scorpion Cos2 */);
    if (rv != 0) {
        return rv;
    }

    rv = bcm88230_flow_control_setup(unit,
                                     local_node_id,
                                     c2_mod2,
                                     hg_c2_mod2,
                                     0x2 /* fifo_mask0, Sirius Cos0 (uc-ef),  Scorpion Cos1 */,
                                     0x1 /* fifo_mask1, Sirius Cos1 (uc-nef), Scorpion Cos0 */,
                                     0x4 /* fifo_mask2, Sirius Cos2 (mc-ef),  Scorpion Cos2 */,
                                     0x4 /* fifo_mask3, Sirius Cos3 (mc-nef), Scorpion Cos2 */);

    printf("bcm88230_2xc2_setup: Done!\n");

    return rv;
}

int
bcm88230_2xTriumph_setup(int unit, int local_node_id)
{
    int                   node_ids=28;
    bcm_module_t          XGS_MODIDS[] = {30,28};
    int                   E2ECC_PORT[] = {1,3};
    int                   XGS_NUM_PORTS[] = {4,48};
    int                   BASE_FABRIC_PORT[]={0,4};
    int                   BASE_PORT_XGS[]={0,0};
    int                   module;
    int                   tid_triumph1, tid_triumph2;
    bcm_trunk_info_t  ta_info_triumph1, ta_info_triumph2;
    bcm_trunk_chip_info_t ta_chip_info;
    bcm_port_config_t     pcfg;
    bcm_port_t            port;
    bcm_gport_t           gport;
    int                   i = 0;
    int                   total_num_of_ports=52;
    int                   num_cos = 8;


    bcm_stk_modid_set(0, BCM_STK_NODE_TO_MOD(node_ids));

    rv = bcm_trunk_chip_info_get(unit, &ta_chip_info);
    if (rv != BCM_E_NONE) {
        printf("Error (%d) in bcm_trunk_chip_info_get\n", rv);
        return rv;
    }
    tid_triumph1 = ta_chip_info.trunk_fabric_id_min;
    tid_triumph2 = ta_chip_info.trunk_fabric_id_min + 1;

    BCM_IF_ERROR_RETURN(bcm_port_config_get(unit, &pcfg));
    BCM_PBMP_ITER(pcfg.hg, port) {
       BCM_GPORT_MODPORT_SET(gport, BCM_STK_NODE_TO_MOD(node_ids), port);
       if (i < 2) {
           ta_info_triumph1.tm[i] = BCM_STK_NODE_TO_MOD(node_ids);
           ta_info_triumph1.tp[i] = gport;
           i++;

           if (i == 2) {
               ta_info_triumph1.num_ports = i;
           } 
       }
       else {
           ta_info_triumph2.tm[(i - 2)] = BCM_STK_NODE_TO_MOD(node_ids);
           ta_info_triumph2.tp[(i - 2)] = gport;
           i++;

           if (i == 4) {
               ta_info_triumph2.num_ports = (i - 2);
               break;
           } 
       }
    }

    /* HiGig Trunking for interconnection to Triumph1 */
    rv = bcm_trunk_create(unit, BCM_TRUNK_FLAG_WITH_ID, &tid_triumph1);
    if (rv != BCM_E_NONE) {
        printf("Error (%d) in bcm_trunk_create_id - Interconnection to Triumph1\n", rv);
        return rv;
    }

    rv = bcm_trunk_set(unit, tid_triumph1, &ta_info_triumph1);
    if (rv != BCM_E_NONE) {
        printf("Error (%d) in bcm_trunk_set - Interconnection to Triumph1\n", rv);
        return rv;
    }

    /* HiGig Trunking for interconnection to Triumph2 */
    rv = bcm_trunk_create(unit, BCM_TRUNK_FLAG_WITH_ID, &tid_triumph2);
    if (rv != BCM_E_NONE) {
        printf("Error (%d) in bcm_trunk_create_id - Interconnection to Triumph2\n", rv);
        return rv;
    }

    rv = bcm_trunk_set(unit, tid_triumph2, &ta_info_triumph2);
    if (rv != BCM_E_NONE) {
        printf("Error (%d) in bcm_trunk_set - Interconnection to Triumph2\n", rv);
        return rv;
    }


    rv = bcm88230_global_setup(unit, local_node_id, num_cos, total_num_of_ports);
    if (rv != 0) {
        printf("bcm88230_global_setup failed on device %d node %d\n",
               unit, local_node_id);
        return rv;
    }

    if (rv == BCM_E_NONE) {
        rv = bcm88230_multicast_setup(unit,
                                      BCM_STK_NODE_TO_MOD(local_node_id));
    }

    for (module = 0; module < sizeof(XGS_MODIDS)/4; module++) {
         printf("module: %d\n",module);
         rv = bcm88230_module_add(unit,
                             local_node_id, /* local node ID */
                             node_ids, /* remote node ID */
                             BASE_FABRIC_PORT[module],  /* base port # */
                             XGS_MODIDS[module], /* T2 module ID */
                             BASE_PORT_XGS[module],  /* base port # */
                             XGS_NUM_PORTS[module]); /* # of ports  */
         if (rv != 0) {
             return rv;
         }
    } /* for each module */

    for (module = 0; module < sizeof(XGS_MODIDS)/4; module++) {
        rv = bcm88230_flow_control_setup(unit,
                                          local_node_id,
                                          XGS_MODIDS[module], /* Triumph module ID */
                                          E2ECC_PORT[module],
                                          0x2 ,
                                          0x1 ,
                                          0x4 ,
                                          0x4 );
        if (rv != 0) {
            return rv;
        }
    }

    printf("Done!\n");

    return rv;
}


/* bcm88230_1x56634_Setup(0,28); */

/* bcm88230_2xScorpion_Setup(0,28); */

/* bcm88230_2xScorpion_Setup(0,28); */

/* bcm88230_1xCaladan2_Setup(0, 28, 28);  */

/* bcm88230_2xTriumph_setup(0,28); */
