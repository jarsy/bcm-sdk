/*
 * $Id: cint_bcm5693x_setup.c,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * This file contains examples for TM and inline TM configurations
 * using BCM56931/BCM56936
 *
 */


bcm_port_congestion_config_t sirius_congestion_info2;

int
bcm5693x_tme_common_config_vars(int unit)
{
    sal_config_set("bcm_num_cos", "8");
    /* use 3 for inline TM */
    sal_config_set("qe_tme_mode", "1");

    /* 
     * following lines must be changed depending on front panel port 
     * port configuration of the packet processor(s) attached to bcm5693x
     */
    sal_config_set("if_subports.port1", "2");
    sal_config_set("if_subports.port2", "2");
    sal_config_set("if_subports.port3", "24");
    sal_config_set("if_subports.port4", "24");

    return 0;
}

int
bcm5693x_tme_flow_control_setup(int          unit,
                                int          node,
                                bcm_module_t module_pp,
                                int          num_ports,
                                bcm_port_t   hg_port,
                                int          fifo_mask0,
                                int          fifo_mask1,
                                int          fifo_mask2,
                                int          fifo_mask3)
{
    int                          fifo, fifo_mask[4];
    int                          rv;
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
    rv = bcm_fabric_congestion_size_set(unit, module_pp, num_ports);
    if (rv != BCM_E_NONE) {
        printf("  bcm_fabric_congestion_size_set FAILED(%d, 0x%x)\n", module_pp, rv);
        return(rv);
    }

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

    sal_memset(&sirius_congestion_info2, 0, sizeof(sirius_congestion_info2));
    sirius_congestion_info2.flags = BCM_PORT_CONGESTION_CONFIG_E2ECC;
    /* sirius_congestion_info2.src_port = "0x1c00"; */
    sirius_congestion_info2.traffic_class = 1;
    sirius_congestion_info2.color=0;

    sirius_congestion_info2.src_mac = "AA:AA:AA:AA:AA:AA";
    sirius_congestion_info2.dest_mac = "00:10:18:20:30:01";

    sirius_congestion_info2.ethertype=0x2002;
    sirius_congestion_info2.opcode=0x220;

    modid = BCM_STK_NODE_TO_MOD(node);
    BCM_GPORT_MODPORT_SET(gport, modid, hg_port);

    BCM_GPORT_MODPORT_SET(sirius_congestion_info2.src_port,
                          module_pp,
                          0);
    rv = bcm_port_congestion_config_set(unit, gport, &sirius_congestion_info2);
    if (rv != BCM_E_NONE) {
        printf("  bcm_port_congestion_config_set FAILED(%d, 0x%x)\n", rv, rv);
        return(rv);
    }

    return 0;
}

int
bcm5693x_tme_Gport_Setup(int           unit,
                         bcm_module_t  sirius_modid,
                         int           num_switch_devices,
                         bcm_module_t *switch_modid,
                         int          *num_port_ranges_per_modid,
                         bcm_port_t   *base_port,
                         int          *num_ports_per_range,
                         int          *num_ports)

{
    bcm_port_config_t    pcfg;
    bcm_pbmp_t           pbmp;
    bcm_port_t           port;
    int                  intf, intf_port, cpu_port;
    int                  idx, port_offset = 0, bcm_err;
    bcm_gport_t          switch_port, fabric_port, intf_cpu_gport;
    bcm_gport_t          fabric_child_port, sched_gport, prev_sched_gport;
    int                  no_ports, no_levels, no_cos, no_levels_to_alloc;
    int                  SIRIUS_CPU_HANDLE = 128;
    bcm_gport_t          multicast_gport, mc_intf_gport, multicast_child_gport;
    /* int                  TS_NO_LEVELS_TO_ALLOCATE = 5; */
    int                  TS_NO_LEVELS_TO_ALLOCATE = 3;
    bcm_module_t         MODEL_BCM56634_MODID  = 0;
    int                  MODEL_BCM56634_CPU_PORT = 50;
    int                  i, j, k, flags = 0, cos;
    int                  ports_per_range, speed, hg_subports[4];
    bcm_gport_t          intf_gport[4];
    bcm_gport_t          child_gport[132];
    bcm_gport_t          child_egress_gport[132];
    bcm_gport_t          egress_group_gport[132];
    bcm_gport_t          unicast_gport[132];
    bcm_gport_t          rq_child_gport[132];
    bcm_gport_t          rq_child_egress_gport[132];
    bcm_gport_t          rq_unicast_gport[132];
    bcm_multicast_t      mcgroup;
    bcm_fabric_distribution_t ds_id;
 
    BCM_IF_ERROR_RETURN(bcm_port_config_get(unit, &pcfg));

    sal_memset(&child_gport, -1, sizeof(fabric_port)*132);

    /* read SOC properties to determine the number of ports/channels on each */
    /* interface                                                    */
    for (intf = 0; intf < 4; intf++) {
        hg_subports[intf] = soc_property_port_get(unit, intf, spn_IF_SUBPORTS, 0);
        no_ports += hg_subports[intf];
    }

    /* setup HG interface gports. The port number is to be derived from "pbmp" */
    BCM_PBMP_ASSIGN(pbmp, pcfg.cpu);
    BCM_PBMP_OR(pbmp, pcfg.hg);

    BCM_PBMP_ITER(pbmp, port) {
        if (BCM_PBMP_MEMBER(pcfg.hg, port)  && port < 4) {
            BCM_GPORT_MODPORT_SET(intf_gport[port], sirius_modid, port);
        } else if (BCM_PBMP_MEMBER(pcfg.cpu, port)) {
            BCM_GPORT_MODPORT_SET(intf_cpu_gport, sirius_modid, port);
        }
    }

    /* 
     * req port hardcoded for now
     * need to expose it in bcm_port_config_t
     */
    BCM_GPORT_MODPORT_SET(mc_intf_gport, sirius_modid, 34);
    /* BCM_GPORT_MODPORT_SET(rq_intf_gport, sirius_modid, rq_port); */

    /* configure module id */
    bcm_err = bcm_stk_modid_set(unit, sirius_modid);
    if (BCM_FAILURE(bcm_err)) {
        printf("bcm_stk_modid_set() failed err=%d %s\n", bcm_err, bcm_errmsg(bcm_err));
        return(bcm_err);
    }

    if (soc_property_get(unit, spn_IF_SUBPORTS_CREATE, 0)) {
        /* setup the child gports */
        for (port = 0; port < no_ports; port++) {
            BCM_GPORT_CHILD_SET(child_gport[port], sirius_modid, port);
            BCM_GPORT_EGRESS_CHILD_SET(child_egress_gport[port], sirius_modid, port);
            BCM_GPORT_EGRESS_GROUP_SET(egress_group_gport[port], sirius_modid, port);
        }
        /* last port is CPU port */
        cpu_port = port;
        BCM_GPORT_CHILD_SET(child_gport[cpu_port], sirius_modid, SIRIUS_CPU_HANDLE);
        BCM_GPORT_EGRESS_CHILD_SET(child_egress_gport[cpu_port], sirius_modid, SIRIUS_CPU_HANDLE);

        printf("ports created at init bcm time\n");
    } else {
        /* setup the child gports */
        flags = 0;
        port = 0;
        for (intf = 0; intf < 4; intf++) {
            for (port_offset = 0; port_offset < hg_subports[intf]; port_offset++, port++) {
                bcm_err = bcm_fabric_port_create(unit, intf_gport[intf], port_offset,
                                                 flags, &child_gport[port]);
                if (BCM_FAILURE(bcm_err)) {
                    printf("bcm_fabric_port_create failed err=%d %s\n", bcm_err, bcm_errmsg(bcm_err));
                    return(bcm_err);
                } else {
                    BCM_GPORT_EGRESS_CHILD_SET(child_egress_gport[port],
                                               BCM_GPORT_CHILD_MODID_GET(child_gport[port]),
                                               BCM_GPORT_CHILD_PORT_GET(child_gport[port]));
                }
            }
        }

        /* last port is CPU port */
        cpu_port = port;
        BCM_GPORT_CHILD_SET(child_gport[cpu_port], sirius_modid, SIRIUS_CPU_HANDLE);

        bcm_err = bcm_fabric_port_create(unit, intf_cpu_gport, 0, flags, &child_gport[cpu_port]);
        if (BCM_FAILURE(bcm_err)) {
            printf("bcm_fabric_port_create failed err=%d %s\n", bcm_err, bcm_errmsg(bcm_err));
            return bcm_err;
        } else {
            printf("created port for cpu, handle (0x%x)\n", child_gport[cpu_port]);
            BCM_GPORT_EGRESS_CHILD_SET(child_egress_gport[cpu_port],
                                       BCM_GPORT_CHILD_MODID_GET(child_gport[cpu_port]),
                                       BCM_GPORT_CHILD_PORT_GET(child_gport[cpu_port]));
        }
    }

    fabric_child_port = 0;

    /* configure switch port to fabric port mapping, 1-to-1 mapping */
    for (i = 0; i < num_switch_devices; i++) {
       for (j = 0; j < num_port_ranges_per_modid[i]; j ++) {
            if (i == 0) {
                ports_per_range = num_ports_per_range[j];
            } else {
                idx = num_port_ranges_per_modid[i-1];
                ports_per_range = num_ports_per_range[idx + j];
            }
         num_ports[i] += ports_per_range;
         for (k = 0; k < ports_per_range; k++) {
            if (i == 0) {
            port = base_port[j] + k;
            } else {
                idx = num_port_ranges_per_modid[i-1];
                port = base_port[j + idx] + k;
            } 
            /* +1: on bcm56634, front panel ports start from 2 */
            BCM_GPORT_MODPORT_SET(switch_port, switch_modid[i], port);
            BCM_GPORT_EGRESS_CHILD_SET(fabric_port, sirius_modid, fabric_child_port++);

            bcm_err = bcm_stk_fabric_map_set(unit, switch_port, fabric_port);
            if (BCM_FAILURE(bcm_err)) {
                printf("bcm_stk_fabric_map_set failed err=%d %s\n", bcm_err, bcm_errmsg(bcm_err));
                return bcm_err;
            }

            printf("Mapping, Switch Gport: 0x%x, Fabric Port: 0x%x\n",
                                          switch_port, fabric_port);
        } /* for k */
      } /* for j */
    } /* for i */

      /*
        cpu_port = port;
        BCM_GPORT_MODPORT_SET(switch_port, MODEL_BCM56634_MODID, MODEL_BCM56634_CPU_PORT);
        BCM_GPORT_EGRESS_CHILD_SET(fabric_port, sirius_modid, SIRIUS_CPU_HANDLE);

        bcm_err = bcm_stk_fabric_map_set(unit, switch_port, fabric_port);
        if (BCM_FAILURE(bcm_err)) {
            printf("bcm_stk_fabric_map_set failed for CPU port err=%d %s\n", bcm_err, bcm_errmsg(bcm_err));
            return bcm_err;
        }
      */

    /* setup unicast logical ports/queue group for each of the ports/channels */
    no_cos = soc_property_get(unit, spn_BCM_NUM_COS, 16);
    flags = BCM_COSQ_GPORT_LOCAL;

    /* Allocate gports if fabric egress setup enabled */
    for (port = 0; port < no_ports; port++) {
         bcm_err = bcm_cosq_gport_add(unit, child_egress_gport[port],
                                      no_cos, flags,  &unicast_gport[port]);
         if (bcm_err != BCM_E_NONE){
             printf("Error adding queue group for phys_gport=0x%x\n", child_egress_gport[port]);
             return bcm_err;
         }

             for (cos = 0; cos < no_cos; cos++) {
                  bcm_err =  bcm_cosq_control_set(unit, 
                                                  unicast_gport[port],
                                                  cos,
                                                  bcmCosqControlFabricConnectMinUtilization,
                                                  0x40);
                  if (bcm_err != BCM_E_NONE){
                      printf("Error in cosq control set %d\n", bcm_err);
                      return bcm_err;
                  }
             }

             speed = soc_property_port_get(unit, port,
                                           spn_PORT_INIT_SPEED, 1000);
             printf("Shaper: port %d speed %d\n", port, speed);
             /*
             for (cos = 0; cos < 4; cos++) {
                  bcm_err = bcm_cosq_gport_bandwidth_set(unit, egress_group_gport[port], cos,
                                                    0, speed * 1000 * 1000, 0);
             }
             */

    } /* added cosq gports */

        /* setup the TS for each of the above allocated queue groups */
        /* The setup is done from Top to Bottom. It could also be    */
        /* done from Bottom to Top                                   */
        port = 0;
        no_cos = soc_property_get(unit, spn_BCM_NUM_COS, 16);
        flags = BCM_COSQ_GPORT_SCHEDULER;

        for (intf = 0; intf < 4; intf++) {
            for (intf_port = 0; intf_port < hg_subports[intf]; intf_port++) {
                if (hg_subports[intf] > 8) {
                    /* subport nodes at level 5 */
                    no_levels_to_alloc = 2;
                } else {
                    /* subport nodes at level 6 */
                    no_levels_to_alloc = 3;
                }
                for (no_levels = 0; no_levels < no_levels_to_alloc; no_levels++) {

                    /* could also specify the interface gport */
                    bcm_err = bcm_cosq_gport_add(unit, intf_gport[intf],
                                                 no_cos, flags,  &sched_gport);
                    if (bcm_err != BCM_E_NONE){
                        printf("Error adding scheduler group for phys_gport=0x%x\n",
                               child_gport[port]);
                        return bcm_err;
                    } else {
                        printf("Added scheduler gport 0x%x\n", sched_gport);
                    }

                    if (no_levels == 0) {
                        /* attach the scheduler to the port allocated resource */
                        bcm_err = bcm_cosq_gport_attach(unit, child_gport[port],
                                                        sched_gport, -1);
                        if (bcm_err != BCM_E_NONE){
                            printf("Error attaching scheduler to port resource, port 0x%x\n",
                                   child_gport[port]);
                            return bcm_err;
                        } else {
                            printf("Attached scheduler gport 0x%x to gport 0x%x\n",
                                                          sched_gport, child_gport[port]);
                        }
                    } else if (no_levels < (no_levels_to_alloc - 1)) {
                        /* attach scheduler to the previous scheduler */
                        bcm_err = bcm_cosq_gport_attach(unit, prev_sched_gport,
                                                        sched_gport, -1);
                        if (bcm_err != BCM_E_NONE){
                            printf("Error attaching intermediate schedulers, port 0x%x\n",
                                   child_gport[port]);
                            return bcm_err;
                        } else {
                            printf("Attached scheduler gport 0x%x to gport 0x%x\n",
                                                          sched_gport, prev_sched_gport);
                       }
                    } else { /* (no_levels = (no_levels_to_alloc - 1)) */

                        /* attach scheduler to the previous scheduler */
                        bcm_err = bcm_cosq_gport_attach(unit, prev_sched_gport,
                                                        sched_gport, -1);
                        if (bcm_err != BCM_E_NONE){
                            printf("Error attaching intermediate schedulers, port 0x%x\n",
                                   child_gport[port]);
                            return(CMD_FAIL);
                        } else {
                            printf("Attached scheduler gport 0x%x to gport 0x%x\n",
                                                          sched_gport, prev_sched_gport);
                        }

                            /* attach queue group to the scheduler */
                            bcm_err = bcm_cosq_gport_attach(unit, sched_gport,
                                                            unicast_gport[port], -1);
                            if (bcm_err != BCM_E_NONE){
                                printf("Error attaching unicast queue group to a scheduler, port 0x%x\n",
                                       child_gport[port]);
                                return bcm_err;
                            } else {
                                printf("Attached queue group gport 0x%x to gport 0x%x\n",
                                                              unicast_gport[port], sched_gport);
                            }
                    }

                    prev_sched_gport = sched_gport;
                }
                port++;
            }
        }

        
        if (0) {
        /* setup the TS for cpu queue groups */
        flags = BCM_COSQ_GPORT_SCHEDULER;
        no_levels_to_alloc = 5;
        cpu_port = no_ports;
        for (no_levels = 0; no_levels < no_levels_to_alloc; no_levels++) {
            /* could also specify the interface gport */
            bcm_err = bcm_cosq_gport_add(unit, intf_cpu_gport,
                                         no_cos, flags,  &sched_gport);
            if (bcm_err != BCM_E_NONE){
                printf("Error adding scheduler group for phys_gport=0x%x\n",
                       child_gport[cpu_port]);
                return bcm_err;
            }

            if (no_levels == 0) {
                /* attach the scheduler to the port allocated resource */
                bcm_err = bcm_cosq_gport_attach(unit, child_gport[cpu_port],
                                                sched_gport, -1);
                if (bcm_err != BCM_E_NONE){
                    printf("Error attaching scheduler to port resource, port 0x%x\n",
                           child_gport[cpu_port]);
                    return bcm_err;
                }
            } else if (no_levels < (no_levels_to_alloc - 1)) {
                /* attach scheduler to the previous scheduler */
                bcm_err = bcm_cosq_gport_attach(unit, prev_sched_gport,
                                                sched_gport, -1);
                if (bcm_err != BCM_E_NONE){
                    printf("Error attaching intermediate schedulers, port 0x%x\n",
                           child_gport[cpu_port]);
                    return bcm_err;
                }
            }
            else {  /* (no_levels = (no_levels_to_alloc - 1)) */

                /* attach scheduler to the previous scheduler */
                bcm_err = bcm_cosq_gport_attach(unit, prev_sched_gport,
                                                sched_gport, -1);
                if (bcm_err != BCM_E_NONE){
                    printf("Error attaching intermediate schedulers, port 0x%x\n",
                           child_gport[cpu_port]);
                    return bcm_err;
                }

                /* attach queue group to the scheduler */
                bcm_err = bcm_cosq_gport_attach(unit, sched_gport,
                                                unicast_gport[cpu_port], -1);
                if (bcm_err != BCM_E_NONE){
                    printf("Error attaching unicast queue group to a scheduler, port 0x%x\n",
                           child_gport[cpu_port]);
                    return bcm_err;
                }
           }
            prev_sched_gport = sched_gport;
        }
    }

    /* Create dummy ESET */
    ds_id = 0;

    /* setup multicast logical port - create local queue group first */
    bcm_err = bcm_cosq_fabric_distribution_add(unit, ds_id, no_cos,
                                               BCM_COSQ_GPORT_LOCAL /* flags */,
                                               &multicast_gport);
    if (bcm_err != BCM_E_NONE) {
        printf("Error creating multicast fabric distribution local queue group error(%d)\n", bcm_err);
        return(CMD_FAIL);
    }


    /*
     * Create the level 6 subport gport - for multicast, this isn't created at 
     *  init time like the others 
     */
    bcm_err = bcm_cosq_gport_add(unit, mc_intf_gport,
                                no_cos, flags,  &multicast_child_gport);

        if (bcm_err != BCM_E_NONE){
            printf("Error adding subport scheduler group for multicast gport=0x%x on interface gport=0x%x\n",
                   multicast_child_gport, mc_intf_gport);
            return bcm_err;
        }

        printf("multicast child_gport(0x%x)\n", multicast_child_gport);

        /* attach the scheduler to last requeue interface (level 6)  */
        bcm_err = bcm_cosq_gport_attach(unit, mc_intf_gport,
                                        multicast_child_gport, -1);
        if (bcm_err != BCM_E_NONE){
            printf("Error attaching scheduler to interface (for multicast) resource, port 0x%x\n",
                   multicast_child_gport);
            return (bcm_err);
        }


        for (no_levels = 0; no_levels < TS_NO_LEVELS_TO_ALLOCATE; no_levels++) {

            /* could also specify the interface gport */
            bcm_err = bcm_cosq_gport_add(unit, mc_intf_gport,
                                         no_cos, flags,  &sched_gport);
            if (bcm_err != BCM_E_NONE){
                printf("Error adding scheduler group for multicast gport=0x%x\n",
                       multicast_child_gport);
                return(bcm_err);
            }

            if (no_levels == 0) {
                /* attach the scheduler to the port allocated resource */
                bcm_err = bcm_cosq_gport_attach(unit, multicast_child_gport,
                                                sched_gport, -1);
                if (bcm_err != BCM_E_NONE){
                    printf("Error attaching scheduler to multicast port resource, port 0x%x\n",
                           multicast_child_gport);
                    return(bcm_err);
                }
            } else if (no_levels < (TS_NO_LEVELS_TO_ALLOCATE - 1)) {
                /* attach scheduler to the previous scheduler */
                bcm_err = bcm_cosq_gport_attach(unit, prev_sched_gport,
                                                sched_gport, -1);
               if (bcm_err != BCM_E_NONE){
                    printf("Error attaching intermediate schedulers, port 0x%x\n",
                           multicast_child_gport);
                    return(CMD_FAIL);
                }
            } else { /* (no_levels = (TS_NO_LEVELS_TO_ALLOCATE - 1)) */

                /* attach scheduler to the previous scheduler */
                bcm_err = bcm_cosq_gport_attach(unit, prev_sched_gport,
                                                sched_gport, -1);
                if (bcm_err != BCM_E_NONE){
                    printf("Error attaching intermediate schedulers, port 0x%x\n",
                           multicast_child_gport);
                    return(CMD_FAIL);
                }

                /* attach queue group to the scheduler */
                bcm_err = bcm_cosq_gport_attach(unit, sched_gport,
                                                multicast_gport, -1);
                if (bcm_err != BCM_E_NONE){
                    printf("Error attaching unicast queue group to scheduler 0x%x, port 0x%x\n",
                           sched_gport, child_gport[port]);
                    return(bcm_err);
                }
                printf("Attach unicast queue group to scheduler, port 0x%x\n",
                           sched_gport);
            }

            prev_sched_gport = sched_gport;
        }

    /* create multicast group 1 */
    mcgroup = 1;
    bcm_err = bcm_multicast_create(unit, BCM_MULTICAST_WITH_ID, &mcgroup);

    if (bcm_err != BCM_E_NONE) {
        printf("Error allocating multicast group error(%d)\n", bcm_err);
        return(bcm_err);
    }

        for (port = 0; port < no_ports; port++) {
            bcm_err = bcm_multicast_egress_add(unit, mcgroup,
                                               child_gport[port], 0 /* encap_id */);
            if (bcm_err != BCM_E_NONE) {
                printf("Error adding ports to the multicast group error(%d)\n", bcm_err);
               return(CMD_FAIL);
            } else {
                printf("Added multicast child gport 0x%x\n", child_gport[port]);
            }
        }

        /* Associate mcgroup with queue group */
        bcm_err = bcm_multicast_fabric_distribution_set(unit, mcgroup, ds_id);

        if (bcm_err != BCM_E_NONE) {
            printf("Error associating multicast group with ds_id\n");
        }

     return bcm_err;
}

/*
 * bcm5693x_nx56634_tme_Setup
 *
 */
int
bcm5693x_nx56634_tme_Setup(int           unit,
                           bcm_module_t  sirius_modid,
                           int           num_switch_devices,
                           bcm_module_t *switch_modid,
                           bcm_port_t   *e2ecc_hg_port,
                           int          *num_port_ranges_per_modid,
                           bcm_port_t   *base_port,
                           int          *num_ports_per_range)
{
    int  i, rv = BCM_E_NONE;
    int  num_ports[] = {0, 0, 0, 0};

    bcm5693x_tme_common_config_vars(unit);

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

    rv = bcm5693x_tme_Gport_Setup(unit, 
                                  sirius_modid,
                                  num_switch_devices,
                                  switch_modid,
                                  num_port_ranges_per_modid,
                                  base_port,
                                  num_ports_per_range,
                                  num_ports);
    if (rv != BCM_E_NONE) {
        printf("Tme_Gport_Setup failed %d: %s\n", rv, bcm_errmsg(rv));
        return rv;
    }


    for (i = 0; i < num_switch_devices; i++) {
        rv = bcm5693x_tme_flow_control_setup(unit,
                                          BCM_STK_MOD_TO_NODE(sirius_modid),
                                          switch_modid[i],
                                          num_ports[i], 
                                          e2ecc_hg_port[i] /* hg_port    */,
                                          0x2 /* fifo_mask0, Sirius Cos0 (uc-ef),  Scorpion Cos1 */,
                                          0x1 /* fifo_mask1, Sirius Cos1 (uc-nef), Scorpion Cos0 */,
                                          0x4 /* fifo_mask2, Sirius Cos2 (mc-ef),  Scorpion Cos2 */,
                                          0x4 /* fifo_mask3, Sirius Cos3 (mc-nef), Scorpion Cos2 */);
         if (rv != BCM_E_NONE) {
             printf("Tme_flow_control_setup failed %d: %s\n", rv, bcm_errmsg(rv));
             return rv;
         }
    }

    return rv;
}

/*
bcm5693x_nx56634_tme_Setup(int           unit,
                           bcm_module_t  sirius_modid,
                           int           num_switch_devices,
                           bcm_module_t *switch_modid,
                           bcm_port_t   *e2ecc_hg_ports,
                           int          *num_port_ranges_per_modid,
                           bcm_port_t   *base_port,
                           int          *num_ports_per_range)
*/

/* here is how you run it */
/*bcm_module_t global_switch_modids[] = {0, 2};
bcm_port_t   global_base_ports[] = {0, 24, 12, 25, 0, 24, 12, 25};
int          global_num_ports_per_range[] = {12, 1, 12, 1, 12, 1, 12, 1};
int          global_num_port_ranges_per_modid[] = {4, 4};
bcm_port_t   e2ecc_hg_ports[] = {1, 3};
*/
/* use the below hg ports for e2ecc in inline tm mode */
/*bcm_port_t   e2ecc_hg_ports[] = {5, 7}; */ 

/*
 * 2xTR2 + 1xSS inline TM setup
 *
 * Configures TR2 for 12x1G + 1x10G per HG, for all 4 HG ports
 * Assumes  config.bcm has following lines:
 *          if_subports.port1=13 (12x1G + 1x10G)
 *          if_subports.port2=13 (12x1G + 1x10G)
 *          if_subports.port3=13 (12x1G + 1x10G)
 *          if_subports.port4=13 (12x1G + 1x10G)
 */ 
/* bcm5693x_nx56634_tme_Setup(0, 
                           10000,
                           2,
                           global_switch_modids,
                           e2ecc_hg_ports, 
                           global_num_port_ranges_per_modid,
                           global_base_ports,
                           global_num_ports_per_range);
*/

/*
 * 48x1G + 4x10G TME setup
 */
/*
bcm_module_t global_switch_modids[] = {0, 2};
bcm_port_t   global_base_ports[] = {0, 24};
int          global_num_ports_per_range[] = {48, 4};
int          global_num_port_ranges_per_modid[] = {1, 1};
bcm_port_t   e2ecc_hg_ports[] = {1, 3};

bcm5693x_nx56634_tme_Setup(0,
                           10000,
                           2,
                           global_switch_modids,
                           e2ecc_hg_ports,
                           global_num_port_ranges_per_modid,
                           global_base_ports,
                           global_num_ports_per_range);
*/

/*
 * 1xTR2 + 1xSS TME setup
 *
 */
/*
bcm_port_t   global_base_ports[] = {0, 24};
int          global_num_ports_per_range[] = {48, 1, 12, 1};
int          global_num_port_ranges_per_modid[] = {1, 1};

bcm5693x_nx56634_tme_Setup(0,
                           10000,
                           1,
                           global_switch_modids,
                           e2ecc_hg_ports,
                           global_num_port_ranges_per_modid,
                           global_base_ports,
                           global_num_ports_per_range);
*/

bcm_module_t global_switch_modids[] = {0, 2};
bcm_port_t   global_base_ports[] = {0, 2, 0, 24};
int          global_num_ports_per_range[] = {2, 2, 24, 24};
int          global_num_port_ranges_per_modid[] = {2, 2};
bcm_port_t   e2ecc_hg_ports[] = {1, 3};




bcm5693x_nx56634_tme_Setup(0,
                           10000,
                           2,
                           global_switch_modids,
                           e2ecc_hg_ports,
                           global_num_port_ranges_per_modid,
                           global_base_ports,
                           global_num_ports_per_range);
