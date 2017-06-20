/*
 * $Id: cint_c2_setup.c,v 1.3 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * Configuration examples for:
 *
 *        2xCaladan2+1xSirius+1xPolaris
 *
 * The script includes Caladan(2) configuration only.
 *
 */

void
bcm88025_bcm88230_config_vars(void)
{
    sal_config_set("qe_spi_0", "0");
    sal_config_set("qe_spi_1", "0");
    sal_config_set("bcm88025_ucode.0", "c2_g2p3");
    sal_config_set("bcm88025_ucode.1", "c2ss_g2p3");
    /* Define multicast QID for this configuration */
    sal_config_set("sbx_mc_qid_base", "200");

    sal_config_set("ucode_num_ports.1", "25");

    sal_config_set("ucode_port.port1.1", "ge0.0:hg0");
    sal_config_set("ucode_port.port2.1", "ge0.1:hg0");
    sal_config_set("ucode_port.port3.1", "ge0.2:hg0");
    sal_config_set("ucode_port.port4.1", "ge0.3:hg0");
    sal_config_set("ucode_port.port5.1", "ge0.4:hg0");
    sal_config_set("ucode_port.port6.1", "ge0.5:hg0");
    sal_config_set("ucode_port.port7.1", "ge0.6:hg0");
    sal_config_set("ucode_port.port8.1", "ge0.7:hg0");
    sal_config_set("ucode_port.port9.1", "ge0.8:hg0");
    sal_config_set("ucode_port.port10.1", "ge0.9:hg0");
    sal_config_set("ucode_port.port11.1", "ge0.10:hg0");
    sal_config_set("ucode_port.port12.1", "ge0.11:hg0");
    sal_config_set("ucode_port.port13.1", "ge1.0:hg1");
    sal_config_set("ucode_port.port14.1", "ge1.1:hg1");
    sal_config_set("ucode_port.port15.1", "ge1.2:hg1");
    sal_config_set("ucode_port.port16.1", "ge1.3:hg1");
    sal_config_set("ucode_port.port17.1", "ge1.4:hg1");
    sal_config_set("ucode_port.port18.1", "ge1.5:hg1");
    sal_config_set("ucode_port.port19.1", "ge1.6:hg1");
    sal_config_set("ucode_port.port20.1", "ge1.7:hg1");
    sal_config_set("ucode_port.port21.1", "ge1.8:hg1");
    sal_config_set("ucode_port.port22.1", "ge1.9:hg1");
    sal_config_set("ucode_port.port23.1", "ge1.10:hg1");
    sal_config_set("ucode_port.port24.1", "ge1.11:hg1");
    sal_config_set("ucode_port.port32.1", "pci0:hg1");

    /* Required for XG ports in HiGig */
    sal_config_set("10g_is_cx4_port25", "0");
    sal_config_set("10g_is_cx4_port26", "0");
    sal_config_set("pbmp_xport_xe", "0x3000000");
    sal_config_set("bcm88025_skip_vcdl_fine_tuning", "1");
}

int
bcm88025_2xc2_1xSS_setup(int unit,
                         bcm_module_t local_modid,
                         bcm_module_t c2_modid1,
                         bcm_module_t c2_modid2,
                         bcm_module_t sirius_modid)
{
    bcm_port_t                   port;
    bcm_gport_t                  gport, switch_gport, fabric_gport;
    bcm_l2_addr_t                l2_addr;
    int                          rv;
    bcm_port_congestion_config_t c2_congestion_info1;
    bcm_port_config_t            pcfg;

    bcm88025_bcm88230_config_vars();

    soc_init(unit);

    bcm_init(unit);

    bcm_port_config_get(unit, &pcfg);

    for (port = 0; port < 24; port ++) {
	BCM_GPORT_MODPORT_SET(switch_gport, c2_modid1, port);
	BCM_GPORT_CHILD_SET(fabric_gport, sirius_modid, port);
	rv = bcm_stk_fabric_map_set(unit,switch_gport, fabric_gport);
        if (BCM_FAILURE(rv)) {
            return rv;
        }

        /* Add MACs pointing to each front panel port */
        sal_memset(&l2_addr, 0, sizeof(l2_addr));
        l2_addr.modid = c2_modid1;
        l2_addr.port  = port;
        l2_addr.mac[0] = 0x0;
        l2_addr.mac[1] = 0x0;
        l2_addr.mac[2] = 0x0;
        l2_addr.mac[3] = 0x0;
        l2_addr.mac[4] = (c2_modid1 & 0xff);
        l2_addr.mac[5] = port;
        l2_addr.vid    = 1;
        rv = bcm_l2_addr_add(unit, &l2_addr);
        if (BCM_FAILURE(rv)) {
            return rv;
        }

    }
      
    for (port=24; port < 48; port ++) {
	BCM_GPORT_MODPORT_SET(switch_gport, c2_modid2, port-24);
	BCM_GPORT_CHILD_SET(fabric_gport, sirius_modid, port);
	rv = bcm_stk_fabric_map_set(unit,switch_gport, fabric_gport);
        if (BCM_FAILURE(rv)) {
            return rv;
        }

        /* Add MACs pointing to each front panel port */
        sal_memset(&l2_addr, 0, sizeof(l2_addr));
        l2_addr.modid = c2_modid2;
        l2_addr.port  = port - 24;
        l2_addr.mac[0] = 0x0;
        l2_addr.mac[1] = 0x0;
        l2_addr.mac[2] = 0x0;
        l2_addr.mac[3] = 0x0;
        l2_addr.mac[4] = (c2_modid2 & 0xff);
        l2_addr.mac[5] = port - 24;
        l2_addr.vid    = 1;
        rv = bcm_l2_addr_add(unit, &l2_addr);
        if (BCM_FAILURE(rv)) {
            return rv;
        }
    }


    rv = bcm_stk_modid_set(unit, local_modid);


    /* 
     * Configure to send e2ecc periodically
     */
    sal_memset(&c2_congestion_info1, 0, sizeof(c2_congestion_info1)); 
    c2_congestion_info1.flags = BCM_PORT_CONGESTION_CONFIG_E2ECC;
    c2_congestion_info1.src_port = (local_modid << 8);
    c2_congestion_info1.traffic_class = 1;
    c2_congestion_info1.color=1;
    c2_congestion_info1.port_bits=64;
    c2_congestion_info1.multicast_id=0;
    c2_congestion_info1.packets_per_sec=0x200;
  
 
    c2_congestion_info1.src_mac  = "00:10:18:20:30:00";
    c2_congestion_info1.dest_mac = "00:10:18:20:30:01";
    c2_congestion_info1.ethertype=0xc0d0;
  
    c2_congestion_info1.opcode=0xe0f0;
    c2_congestion_info1.opcode=0xe0f0;

    BCM_PBMP_ITER(pcfg.hg, port) {
       break;
    } 
    
    /*32 is my higig port*/
    BCM_GPORT_MODPORT_SET(gport, local_modid, port); 
  
    rv = bcm_port_congestion_config_set(unit, gport, &c2_congestion_info1);
  
    return rv;
}

bcm88025_2xc2_1xSS_setup(1, 0, 0, 1, 10000);
