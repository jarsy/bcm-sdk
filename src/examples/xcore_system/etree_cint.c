/*
 *  E-Tree script 
 *
 * $Id: etree_cint.c,v 1.3 Broadcom SDK $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */

/* global configuration */
int qe = 0;
int fe = 1;
bcm_pbmp_t ROOT_pbmp;
bcm_pbmp_t LEAF_pbmp; 
BCM_PBMP_PORT_ADD (ROOT_pbmp, 0);
BCM_PBMP_PORT_ADD (ROOT_pbmp, 1);
BCM_PBMP_PORT_ADD (LEAF_pbmp, 2);
BCM_PBMP_PORT_ADD (LEAF_pbmp, 3);

void etree_config()
{

/* ----------------------------------------------------------------------------
 * Port Isolation via ETREE Service
 * --------------------------------
 *
 * Gu2K Application provides Port Isolation via ETREE Service.
 *
 * (1) Users maintain a port-isolation table per group as shown below:
 *
 * port-isolation table for group 0
 * ----------+-----------+-----------+-------------
 * ROOT_pbmp | LEAF_pbmp |  ROOT_vsi |  LEAF_vsi
 * ----------+-----------+-----------+-------------
 *       0x0 |       0x0 |       0x0 |       0x0
 * ----------+-----------+-----------+-------------
 *
 * where pbmp is a port bitmap.
 *
 * (2) define ROOT port(s) and LEAF port(s)
 *
 * port-isolation table for group 0
 * ----------+-----------+-----------+-------------
 * ROOT_pbmp | LEAF_pbmp |  ROOT_vsi |  LEAF_vsi
 * ----------+-----------+-----------+-------------
 *       0x8 |       0x7 |       0x0 |       0x0
 * ----------+-----------+-----------+-------------
 *
 * In this example, port 3 is a ROOT port and port {0,1,2} are LEAF ports.
 *
 * (3) Create ROOT and LEAF Virtual Switch Instances (VSIs)
 *
 * With port isolation enabled, learning and forwarding are performed based
 * on VSI rather than VID.
 */

   bcm_vlan_t *ROOT_vsi;
   bcm_vlan_t *LEAF_vsi;
   
   bcm_vswitch_create(fe, ROOT_vsi);
   bcm_vswitch_create(fe, LEAF_vsi);

/*
 * (4) Update the port-isolation table
 *
 * port-isolation table for group 0
 * ----------+-----------+-----------+-------------
 * ROOT_pbmp | LEAF_pbmp |  ROOT_vsi |  LEAF_vsi
 * ----------+-----------+-----------+-------------
 *       0x8 |       0x7 |    0x4000 |    0x4001
 * ----------+-----------+-----------+-------------
 *
 * bcm_vswitch_create() returns VSI ID.
 * In this example, ROOT and LEAF vsi(s) are 0x4000 and 0x4001 respectively
 *
 * (5) Add port(s) to the VSI. Users first create VLAN Logical Port(s) on
 * physical port(s). And add the LP(s) to the VSI.
 */

   bcm_port_t port;

   BCM_PBMP_ITER(ROOT_pbmp, port) {
      bcm_vlan_port_t *vlan_port;

      /* configure vlan_port: */
      vlan_port->criteria = BCM_VLAN_PORT_MATCH_PORT;
      vlan_port->port     = port;
      vlan_port->flags    = 0;

      bcm_vlan_port_create(fe, vlan_port);

      vlan_port->flags    = BCM_VLAN_PORT_WITH_ID;
      bcm_vswitch_port_add(fe, *ROOT_vsi, vlan_port->vlan_port_id);
   }

   BCM_PBMP_ITER(LEAF_pbmp, port) {
      bcm_vlan_port_t *vlan_port;

      /* configure vlan_port: */
      vlan_port->criteria = BCM_VLAN_PORT_MATCH_PORT;
      vlan_port->port     = port;
      vlan_port->flags    = 0;

      bcm_vlan_port_create(fe, vlan_port);

      vlan_port->flags    = BCM_VLAN_PORT_WITH_ID;
      bcm_vswitch_port_add(fe, *LEAF_vsi, vlan_port->vlan_port_id);
   }

/*
 * (5) Create multicast gorups for VSIs.
 *
 * Flooding domains are defined by multicast group.
 */

   bcm_multicast_t *ROOT_mcgrp;
   bcm_multicast_t *LEAF_mcgrp;

   bcm_multicast_create(qe, BCM_MULTICAST_TYPE_L2, ROOT_mcgrp);
   bcm_multicast_create(qe, BCM_MULTICAST_TYPE_L2, LEAF_mcgrp);

/*
 * (6) Add port(s) to the multicast group.
 *     ROOT_vsi members are ROOT and LEAF ports
 *     LEAF_vsi members are ROOT ports only
 */

   int modid; 

   bcm_stk_modid_get(fe, &modid);

   BCM_PBMP_ITER(ROOT_pbmp, port) {
      bcm_gport_t fe_gport;
      bcm_gport_t qe_gport;

      BCM_GPORT_MODPORT_SET(fe_gport, modid, port);
      bcm_stk_fabric_map_get(fe, fe_gport, &qe_gport);

      bcm_multicast_l2_encap_get(fe, ROOT_mcgrp, fe_gport, ROOT_vsi, &encap_id);
      bcm_multicast_egress_add(qe, ROOT_mcgrp, qe_gport, encap_id);

      bcm_multicast_l2_encap_get(fe, LEAF_mcgrp, fe_gport, LEAF_vsi, &encap_id);
      bcm_multicast_egress_add(qe, LEAF_mcgrp, qe_gport, encap_id);
   }

   BCM_PBMP_ITER(LEAF_pbmp, port) {
      bcm_gport_t fe_gport;
      bcm_gport_t qe_gport;

      BCM_GPORT_MODPORT_SET(fe_gport, modid, port);
      bcm_stk_fabric_map_get(fe, fe_gport, &qe_gport);

      bcm_multicast_l2_encap_get(fe, ROOT_mcgrp, fe_gport, LEAF_vsi, &encap_id);
      bcm_multicast_egress_add(qe, ROOT_mcgrp, qe_gport, encap_id);
   }


/*
 * (7) Configure MAC table via Learning.
 *
 * For Port Isolation, users need to add a MAC address to both ROOT and LEAF
 * VSIs. When adding the address to the LEAF VSI, users need to use the
 * BCM_L2_DISCARD_DST flag so that th
 *
 * When a packet is received from LEAF Port, Gu2K creates a Learn Exception
 * with LEAF VSI (LEAF_vsi). Users look up the port-isolation table and figure
 * out it is associated with a LEAF port. And users use the BCM_L2_DISCARD_DST
 * flag for the LEAF VSI so that it is dropped when the address is looked up
 * with LEAF VSI.
 */

   /* port_isolation_learn()  */

   bcm_mac_t smac;       /* (IN) MAC Address from Exception  */
   bcm_vlan_t vsi;       /* (IN) VSI from Exception  */
   int modid;            /* (IN) Module ID (Node ID) from Exception */
    bcm_port_t port;      /* (IN) Port ID (PoE) from Exception */
    bcm_vlan_t *LEAF_vsi; /* (IN) ROOT VLAN */
    bcm_vlan_t *ROOT_vsi; /* (IN) LEAF VLAN */

    bcm_l2_addr_t *ROOT_l2_addr;
    bcm_l2_addr_t *LEAF_l2_addr;
  
    bcm_l2_addr_t_init(ROOT_l2_addr, smac, *ROOT_vsi);
    bcm_l2_addr_t_init(LEAF_l2_addr, smac, *LEAF_vsi);
  
    ROOT_l2_addr->modid  = modid;
    ROOT_l2_addr->port   = port;
  
    LEAF_l2_addr->modid  = modid;
    LEAF_l2_addr->port   = port;
  
    if (vsi == *LEAF_vsi) {
       LEAF_l2_addr->flags |= BCM_L2_DISCARD_DST;
    }
  
    bcm_l2_addr_add(fe, ROOT_l2_addr);
    bcm_l2_addr_add(fe, LEAF_l2_addr);

/* ----------------------------------------------------------------------------
 *
 * (8) Configure L2 Multicast MAC table
 */

      bcm_multicast_t *l2_mcgrp
      bcm_multicast_create(qe, BCM_MULTICAST_TYPE_L2, l2_mcgrp);

      bcm_l2_addr_t *ROOT_l2_addr;
      bcm_l2_addr_t *LEAF_l2_addr;
      bcm_l2_addr_t_init(ROOT_l2_addr, l2mcastmac, *ROOT_vsi);
      bcm_l2_addr_t_init(LEAF_l2_addr, l2mcastmac, *LEAF_vsi);

      /* Let RootVlanGport = vlan gport of root port created with bcm_vlan_port_create */
      /* Let Leaf1VlanGport = vlan gport of root port created with bcm_vlan_port_create */

      ROOT_l2_addr->modid  = modid;
      ROOT_l2_addr->port   = RootVlanGport;
  
      LEAF_l2_addr->modid  = modid;
      LEAF_l2_addr->port   = Leaf1VlanGport;
  
      if (vsi == *LEAF_vsi) {
         LEAF_l2_addr->flags |= BCM_L2_DISCARD_DST;
      }
  
      bcm_multicast_l2_encap_get(fe, l2_mcgrp, RootVlanGport, ROOT_vsi, &encap_id);
      bcm_multicast_egress_add(qe, l2_mcgrp, qe_gport, encap_id);
      bcm_multicast_l2_encap_get(fe, l2_mcgrp, Leaf1VlanGport, ROOT_vsi, &encap_id);
      bcm_multicast_egress_add(qe, l2_mcgrp, qe_gport, encap_id);


      bcm_l2_addr_add(fe, ROOT_l2_addr);
      bcm_l2_addr_add(fe, LEAF_l2_addr);

      /* ---------------------------------------------------------------------------- */

}

