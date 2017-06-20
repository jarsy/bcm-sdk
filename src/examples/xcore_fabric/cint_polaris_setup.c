/*
 * $Id: cint_polaris_setup.c,v 1.22 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * Configuration examples for:
 *
 *        2xScorpion+4xSirius+2xPolaris
 *        1xScorpion+2xSirius+1xPolaris
 *
 * The script includes Polaris configuration only.
 *
 */
int num_cos=8;
int MAX_XBARS = 48;
int MAX_NODES = 72;
int rv;
/* int xcfg_remap[MAX_XBARS][MAX_NODES]; */
int MAX_XBARS_NODES = MAX_XBARS * MAX_NODES;
int TRUE=1;
int xcfg_map[MAX_XBARS_NODES];
int xbar = 0;

int node_ids[4]         = {16, 20, 24, 28};
int logical_node_ids[4] = {16, 20, 24, 28};

int NODE_QE2K = 1;
int NODE_SIRIUS = 2;

int
polaris_common_config_vars(int unit)
{
    
    sal_config_set("bcm_num_cos", "8");
    sal_config_set("qe_tme_mode", "0");

    return 0;
}

int
polaris_node_add(int        unit,
                 int        node,
                 int        logical_node,
                 bcm_pbmp_t sci_ports,
                 bcm_pbmp_t sfi_ports,
                 int        base_fabric_port,
                 int        num_ports_pp,
                 int        node_type)
{
    bcm_port_t   port;
    bcm_pbmp_t   pbmp;
    int          num_cos;
    bcm_gport_t  child_gport;
    bcm_gport_t  fabric_gport;
    bcm_gport_t  uc_gport;
    bcm_gport_t  mc_gport;
    int          idx;
    int          MAX_NODES = 72;
    bcm_module_t modid;


    if (node_type == NODE_SIRIUS) {
        rv = bcm_stk_module_protocol_set(unit, 
                                         BCM_STK_NODE_TO_MOD(logical_node),
                                         bcmModuleProtocol3);
    } else if (node_type == NODE_QE2K) {
        rv = bcm_stk_module_protocol_set(unit, 
                                         BCM_STK_NODE_TO_MOD(logical_node),
                                         bcmModuleProtocol1);
    }

    if (rv != BCM_E_NONE) {
        return rv;
    }

    BCM_PBMP_ITER(sci_ports, port) {
            rv = bcm_port_control_set(unit, port, bcmPortControlAbility, BCM_PORT_ABILITY_SCI);
            if (rv != BCM_E_NONE) {
                return rv;
            }
    }

    BCM_PBMP_ITER(sfi_ports, port) {
             if (node_type == NODE_SIRIUS) {
                 rv = bcm_port_control_set(unit, port, bcmPortControlAbility, 
                                       BCM_PORT_ABILITY_DUAL_SFI);
             } else if (node_type == NODE_QE2K) {
                 rv = bcm_port_control_set(unit, port, bcmPortControlAbility, 
                                       BCM_PORT_ABILITY_SFI);
             }

             if (rv != BCM_E_NONE) {
                return rv;
             }
    }

    /* BCM_PBMP_ASSIGN(pbmp, sfi_ports);
    BCM_PBMP_OR(pbmp, sci_ports); */
    BCM_PBMP_ITER(sci_ports, port) {
             rv = bcm_port_enable_set(unit, port, TRUE);
             if (rv != BCM_E_NONE) {
                 return rv;
             }
    }

    BCM_PBMP_ITER(sfi_ports, port) {
             rv = bcm_port_enable_set(unit, port, TRUE);
             if (rv != BCM_E_NONE) {
                 return rv;
             }
    }

    /*
     * Crossbar mapping
     */
    xbar = 0;
    BCM_PBMP_ITER(sfi_ports, port) {
             /* xcfg_remap[xbar++][node] = port; */
             idx = xbar * MAX_NODES + node;
             xcfg_map[idx] = port;
             printf("setting xbar %d node %d port %d idx %d\n",
                    xbar, node, port, idx); 
             xbar++;
    }

    bcm_cosq_config_get(unit, &num_cos);

    modid = BCM_STK_NODE_TO_MOD(node);

    rv = bcm_stk_module_enable(unit, BCM_STK_NODE_TO_MOD(node), -1, TRUE);
    if (rv != BCM_E_NONE) {
        return rv;
    }

    /*
     * Add queues for fabric ports corresponding to front panel ports
     */
    for (port = 0; port < num_ports_pp; port++) {
         BCM_GPORT_CHILD_SET(fabric_gport, modid, (base_fabric_port + port));
         child_gport = fabric_gport;
         rv = bcm_cosq_gport_add(unit, child_gport, num_cos, 0,
                                 &uc_gport);
         if (rv == 0) {
             printf("Created gport 0x%x: node %d port %d\n", uc_gport,
                     node, (base_fabric_port + port));
         } else {
             return rv;
         }
    }

    return rv;
}

int
polaris_global_setup(int unit,
                     int num_cos)
{
    int                       i;
    int                       idx;
    int                       port;
    bcm_fabric_distribution_t ds_id;
    bcm_gport_t               mc_gport;
    int                       flags;

    rv = BCM_E_NONE;
 
    for (i=0; i < MAX_NODES; i++) {
        for (port=0; port < MAX_XBARS; port++) {
             /* xcfg_remap[port][i] = -1; */
             idx = port * MAX_NODES + i;
             xcfg_map[idx] = -1;
        }
    }

    bcm_cosq_config_set(unit, num_cos);

    /* multicast ports */
    /* setup multicast logical port - create queue group first */
    ds_id = 0;
    flags = 0;
    rv = bcm_cosq_fabric_distribution_add(unit, ds_id, num_cos,
                                          flags, &mc_gport);
    if (rv != BCM_E_NONE) {
        printf("Error creating multicast fabric distribution queue group error(%d)\n", rv);
        return rv;
    }

    return rv;
}

/*
 * This is called after enabling nodes and setting up 
 * crossbars
 */
int
polaris_crossbar_setup(int unit, int *pl_node_ids, int num_nodes)
{
    int    src;
    int    dst;
    int    port;
    int    bcm_err = 0;
    int    idxa;
    int    idxb;
    uint64 crossbar_mask = "0x3fffff";

    for (src=0; src < num_nodes; src++) {
       for (dst=0; dst < num_nodes; dst++) {
    
          for (port=0; port<MAX_XBARS; port++) {
               idxa = port * MAX_NODES + pl_node_ids[src];
               idxb = port * MAX_NODES + pl_node_ids[dst];
                 /* printf("src %d dst %d port %d\n",
                        xcfg_map[idxa],
                        xcfg_map[idxb],
                        port); */
             /* if (xcfg_remap[port][src] != -1 && xcfg_remap[port][dst] != -1) { */
             if (xcfg_map[idxa] != -1 && xcfg_map[idxb] != -1) {
                 printf("src %d dst %d port %d\n",
                        xcfg_map[idxa],
                        xcfg_map[idxb],
                        port);
                 rv = bcm_fabric_crossbar_connection_set(unit, 
                                                         port,
                                                         BCM_STK_NODE_TO_MOD(pl_node_ids[src]),
                                                         xcfg_map[idxa]/* xcfg_remap[port][src] */,
                                                         BCM_STK_NODE_TO_MOD(pl_node_ids[dst]), 
                                                         xcfg_map[idxb] /* xcfg_remap[port][dst]*/ );
                if (rv != BCM_E_NONE && bcm_err == BCM_E_NONE)
                    bcm_err = rv;
                }
          }/* port */
       }/* dst */
    }/* src */

   rv = bcm_fabric_crossbar_enable_set(unit, crossbar_mask);
    
   return rv;
}

 
int sfi_node_16_pl0[11] = {17, 18, 19, 64, 65, 66, 67, 68, 69, 70, 71};
int sfi_node_20_pl0[11] = {21, 22, 23, 72, 73, 74, 75, 76, 77, 78, 79};
int sfi_node_24_pl0[11] = {25, 26, 27, 80, 81, 82, 83, 84, 85, 86, 87};
int sfi_node_28_pl0[11] = {29, 30, 31, 88, 89, 90, 91, 92, 93, 94, 95};
 
int sfi_node_16_pl1[11] = {17, 18, 19, 64, 65, 66, 67, 68, 69, 70, 71};
int sfi_node_20_pl1[11] = {21, 22, 23, 72, 73, 74, 75, 76, 77, 78, 79};
int sfi_node_24_pl1[11] = {25, 26, 27, 80, 81, 82, 83, 84, 85, 86, 87};
int sfi_node_28_pl1[11] = {29, 30, 31, 88, 89, 90, 91, 92, 93, 94, 95};
/* int fabric_port_base[4] = {1, 17, 1,  17}; */
int fabric_port_base[4] = {0, 0, 0, 0};


/*
 * polaris_2xScorpion_setup
 *
 * Top level function to configure Polaris in a 
 * 2xSco + 4xSS + 2xPL System
 */
int
polaris_2xScorpion_setup(int unit)
{
    bcm_pbmp_t sci_ports;
    bcm_pbmp_t sfi_ports;
    int        *pl_node_sfi;
    int         node, num_nodes;
    int         i;

    rv = polaris_global_setup(unit, 8);
    if (rv != BCM_E_NONE) {
        return rv;
    }

    num_nodes = sizeof(node_ids)/4;
    for (node = 0; node < num_nodes; node ++) {

         BCM_PBMP_CLEAR(sci_ports);
         BCM_PBMP_CLEAR(sfi_ports);

         if (unit == 0) {
             if (node_ids[node] == 16) {
                 pl_node_sfi = sfi_node_16_pl0;
             } else if (node_ids[node] == 20) {
                 pl_node_sfi = sfi_node_20_pl0;
             } else if (node_ids[node] == 24) {
                 pl_node_sfi = sfi_node_24_pl0;
             } else if (node_ids[node] == 28) {
                 pl_node_sfi = sfi_node_28_pl0;
             } 
         } else if (unit == 1) {
             if (node_ids[node] == 16) {
                 pl_node_sfi = sfi_node_16_pl1;
             } else if (node_ids[node] == 20) {
                 pl_node_sfi = sfi_node_20_pl1;
             } else if (node_ids[node] == 24) {
                 pl_node_sfi = sfi_node_24_pl1;
             } else if (node_ids[node] == 28) {
                 pl_node_sfi = sfi_node_28_pl1;
             } 
         }
         BCM_PBMP_PORT_ADD(sci_ports, node_ids[node]);
 
         for (i = 0; i < 11; i++) {
              BCM_PBMP_PORT_ADD(sfi_ports, *(pl_node_sfi + i));
         }
         rv = polaris_node_add(unit, 
                               node_ids[node], 
                               logical_node_ids[node], 
                               sci_ports, 
                               sfi_ports, 
                               fabric_port_base[node], 
                               8, 
                               NODE_SIRIUS);
         if (rv != BCM_E_NONE) {
             return rv;
         }
         
    }

    /*
     * Commit the crossbar
     */
    rv = polaris_crossbar_setup(unit, node_ids, num_nodes);

    printf("Done!\n");

    return rv;
}

/*
 * polaris_1xScorpion_setup
 *
 * Top level function to configure Polaris in a 
 * 1xSco + 2xSS + 1xPL System
 */
int
polaris_1xScorpion_setup(int unit)
{
    bcm_pbmp_t sci_ports;
    bcm_pbmp_t sfi_ports;
    int        *pl_node_sfi;
    int         node, num_nodes;
    int         i;
    int        pl_2xSS_node_ids[2]         = {28, 24};
    int sfi_node_24_pl[22] = {25, 26, 27, 73, 74, 75, 68, 69, 70, 71, 64, \
                              65, 66, 67, 60, 61, 62, 63, 56, 57, 58, 59};
    int sfi_node_28_pl[22] = {29, 30, 31, 93, 94, 95, 88, 89, 90, 91, 84, \
                              85, 86, 87, 80, 81, 82, 83, 76, 77, 78, 79};

    rv = polaris_global_setup(unit, 8);
    if (rv != BCM_E_NONE) {
        return rv;
    }

    num_nodes = sizeof(pl_2xSS_node_ids)/4;
    for (node = 0; node < num_nodes; node ++) {

         BCM_PBMP_CLEAR(sci_ports);
         BCM_PBMP_CLEAR(sfi_ports);

         if (pl_2xSS_node_ids[node] == 24) {
             pl_node_sfi = sfi_node_24_pl;
         } else if (pl_2xSS_node_ids[node] == 28) {
             pl_node_sfi = sfi_node_28_pl;
         } 
         BCM_PBMP_PORT_ADD(sci_ports, pl_2xSS_node_ids[node]);
 
         for (i = 0; i < sizeof(sfi_node_24_pl)/4; i++) {
              BCM_PBMP_PORT_ADD(sfi_ports, *(pl_node_sfi + i));
         }
         rv = polaris_node_add(unit, 
                               pl_2xSS_node_ids[node], 
                               pl_2xSS_node_ids[node], 
                               sci_ports, 
                               sfi_ports, 
                               fabric_port_base[node], 
                               8, 
                               NODE_SIRIUS);
         if (rv != BCM_E_NONE) {
             return rv;
         }
         
    }

    /*
     * Commit the crossbar
     */
    rv = polaris_crossbar_setup(unit, pl_2xSS_node_ids, num_nodes);

    printf("Done!\n");

    return rv;
}

/*
 * polaris_qe2k_ss_interop_Setup
 *
 * Polaris configuration for a mixed QE (qe2000 + bcm88230) system
 *
 * System configuration: FE2KXT + QE2K
 *                       FE2KXT + SS (FIC)
 *                       PL 
 *
 */
int
polaris_qe2k_ss_interop_Setup(int unit,
                          int qe2k_node_id,
                          int ss_node_id)
{
    int        rv = BCM_E_NONE;
    bcm_port_t port;
    bcm_pbmp_t sci_ports;
    bcm_pbmp_t sfi_ports;
    int        node, node_type;
    int        pl_qe2k_ss_node_ids[2];
    int        *pl_node_sfi;

    /* Using 22 links to SS */
    int sfi_node_ss[22] = {8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, \
                           19, 20, 21, 22, 23, 25, 26, 27, 29, 30, 31};
 
    /* Using 18 links to QE2K */
    int sfi_node_qe2k[22] = {36, 37, 38, 39, 40, 41, 42, 43, 44, \
                             45, 46, 47, 48, 49, 50, 51, 52, 53, \
                             -1, -1, -1, -1};

    
    rv = polaris_global_setup(unit, num_cos);

    pl_qe2k_ss_node_ids[0] = ss_node_id;
    pl_qe2k_ss_node_ids[1] = qe2k_node_id;

    for (node = 0; node < 2; node ++) {
         BCM_PBMP_CLEAR(sci_ports);
         BCM_PBMP_CLEAR(sfi_ports);

         if (node == 0) {
             pl_node_sfi = sfi_node_ss; 
             node_type = NODE_SIRIUS;
         } else {
             pl_node_sfi = sfi_node_qe2k; 
             node_type = NODE_QE2K;
         }

         BCM_PBMP_PORT_ADD(sci_ports, pl_qe2k_ss_node_ids[node]);

         for (i = 0; i < 22; i++) {
              if (*pl_node_sfi != -1) {
                  BCM_PBMP_PORT_ADD(sfi_ports, *pl_node_sfi);
              }
              pl_node_sfi++;
         }

         rv = polaris_node_add(unit, 
                               pl_qe2k_ss_node_ids[node], 
                               pl_qe2k_ss_node_ids[node], 
                               sci_ports, 
                               sfi_ports,
                               fabric_port_base[node], 
                               8, 
                               node_type);
         if (rv != BCM_E_NONE) {
             return rv;
         }
    }

    /*
     * Commit the crossbar
     */
    rv = polaris_crossbar_setup(unit, pl_qe2k_ss_node_ids, 2);

    return rv;
}

int 
polaris_c2_ss_setup(int unit, int node1, int node2, int num_cos)
{
    bcm_pbmp_t  sci_ports;
    bcm_pbmp_t  sfi_ports;
    int        *pl_node_sfi;
    int         node;
    int         i;
    int         pl_2xSS_node_ids[2];
    
    int sfi_node_1_pl[22] = { 29, 30, 31, 25,  26, 27, 20, 21, 22, 23, \
                              16, 17, 18, 19,  12, 13, 14, 15, 8,   9, 10, 11};
    
    int sfi_node_2_pl[22] = {5, 6, 7, 1, 2, 3, 80, 81, 82, 83, 84, \
                              85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95};

    rv = polaris_global_setup(unit, num_cos);
    if (rv != BCM_E_NONE) {
        return rv;
    }

    pl_2xSS_node_ids[0] = node1;
    pl_2xSS_node_ids[1] = node2;

    for (node = 0; node < sizeof(pl_2xSS_node_ids)/4; node ++) {

         BCM_PBMP_CLEAR(sci_ports);
         BCM_PBMP_CLEAR(sfi_ports);

         if (pl_2xSS_node_ids[node] == node1) {
             pl_node_sfi = sfi_node_1_pl;
         } else if (pl_2xSS_node_ids[node] == node2) {
             pl_node_sfi = sfi_node_2_pl;
         }
         BCM_PBMP_PORT_ADD(sci_ports, pl_2xSS_node_ids[node]);

         for (i = 0; i < sizeof(sfi_node_2_pl)/4; i++) {
              BCM_PBMP_PORT_ADD(sfi_ports, *(pl_node_sfi + i));
         }
         rv = polaris_node_add(unit, 
                               pl_2xSS_node_ids[node], 
                               pl_2xSS_node_ids[node], 
                               sci_ports, 
                               sfi_ports, 
                               fabric_port_base[node], 
                               24,
                               NODE_SIRIUS);
         if (rv != BCM_E_NONE) {
             return rv;
         }

    }

    /*
     * Commit the crossbar
     */
    rv = polaris_crossbar_setup(unit, pl_2xSS_node_ids, 2);

    printf("polaris_c2_ss_setup: Done!");

    return rv;
}

int 
polaris_c2_ss_setup(int unit, int node1, int node2, int num_cos)
{
    bcm_pbmp_t                sci_ports;
    bcm_pbmp_t                sfi_ports;
    int                       *pl_node_sfi;
    int                       node, i;
    int                       pl_2xSS_node_ids[2];
    bcm_fabric_distribution_t ds_id;
    bcm_gport_t               mc_gport;
    uint32                    flags;
    bcm_port_t                port;

    int sfi_node_1_pl[22] = { 29, 30, 31, 25,  26, 27, 20, 21, 22, 23, \
                              16, 17, 18, 19,  12, 13, 14, 15, 8,   9, 10, 11};

    int sfi_node_2_pl[22] = {5, 6, 7, 1, 2, 3, 80, 81, 82, 83, 84, \
                              85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95};

    rv = bcm_cosq_config_set(unit, num_cos);
    if (rv != BCM_E_NONE) {
        return rv;
    }

    for (i=0; i < MAX_NODES; i++) {
        for (port=0; port < MAX_XBARS; port++) {
             /* xcfg_remap[port][i] = -1; */
             idx = port * MAX_NODES + i;
             xcfg_map[idx] = -1;
        }
    }

    pl_2xSS_node_ids[0] = node1;
    pl_2xSS_node_ids[1] = node2;

    for (node = 0; node < sizeof(pl_2xSS_node_ids)/4; node ++) {

         BCM_PBMP_CLEAR(sci_ports);
         BCM_PBMP_CLEAR(sfi_ports);

         if (pl_2xSS_node_ids[node] == node1) {
             pl_node_sfi = sfi_node_1_pl;
         } else if (pl_2xSS_node_ids[node] == node2) {
             pl_node_sfi = sfi_node_2_pl;
         } 
         BCM_PBMP_PORT_ADD(sci_ports, pl_2xSS_node_ids[node]);
 
         for (i = 0; i < sizeof(sfi_node_2_pl)/4; i++) {
              BCM_PBMP_PORT_ADD(sfi_ports, *(pl_node_sfi + i));
         }
         rv = polaris_node_add(unit, 
                               pl_2xSS_node_ids[node], 
                               pl_2xSS_node_ids[node], 
                               sci_ports, 
                               sfi_ports, 
                               fabric_port_base[node], 
                               48,
                               NODE_SIRIUS);

         if (rv != BCM_E_NONE) {
             return rv;
         }
         
    }

   
    /* multicast ports */
    /* setup multicast logical port - create queue group first */
    ds_id = 0;
    flags = 0;
    rv = bcm_cosq_fabric_distribution_add(unit, ds_id, num_cos,
                                          flags, &mc_gport);
    if (rv != BCM_E_NONE) {
        printf("Error creating multicast fabric distribution queue group error(%d)\n", rv);
        return rv;
    }

    /*
     * Commit the crossbar
     */
    rv = polaris_crossbar_setup(unit, pl_2xSS_node_ids, 2);

    printf("polaris_c2_ss_setup: Done!");


    return rv;
}

/*
 * polaris_1x56634_setup
 *
 * Top level function to configure Polaris in a
 * 1xT2(56634) + 1xSS + 1xPL System
 */
int
polaris_1xXGS_setup(int unit)
{
    bcm_pbmp_t sci_ports;
    bcm_pbmp_t sfi_ports;
    int        *pl_node_sfi;
    int        node;
    int        i;
    int        pl_1xSS_node_id = 28;
    int sfi_node_28_pl[22] = {29, 30, 31, 93, 94, 95, 88, 89, 90, 91, 84, \
                              85, 86, 87, 80, 81, 82, 83, 76, 77, 78, 79};

    bcm_cosq_config_set(unit, num_cos);

    rv = polaris_global_setup(unit, num_cos);
    if (rv != BCM_E_NONE) {
        return rv;
    }

    for (node = 0; node < 1; node++) {

         BCM_PBMP_CLEAR(sci_ports);
         BCM_PBMP_CLEAR(sfi_ports);

         pl_node_sfi = sfi_node_28_pl;
         BCM_PBMP_PORT_ADD(sci_ports, pl_1xSS_node_id);

         for (i = 0; i < sizeof(sfi_node_28_pl)/4; i++) {
              BCM_PBMP_PORT_ADD(sfi_ports, *(pl_node_sfi + i));
         }

         rv = polaris_node_add(unit,
                               pl_1xSS_node_id,
                               pl_1xSS_node_id,
                               sci_ports,
                               sfi_ports,
                               fabric_port_base[node],
                               48,
                               NODE_SIRIUS);
         if (rv != BCM_E_NONE) {
             return rv;
         }

    }

    /*
     * Commit the crossbar
     */
    rv = polaris_crossbar_setup(unit, &pl_1xSS_node_id, 1);

    printf("Done!\n");

    return rv;
}

polaris_1xXGS_setup(0);
