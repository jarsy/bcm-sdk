/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~1588~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/*
 * $Id$
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File: cint_1588.c
 * Purpose: 1588 protocol use examples
 *
 * BEFORE USING EXAMPLES IN THIS CINT:
 * ------------------------------------:
 * 1) 1588 is NOT SUPPORTED FOR ARAD A0, supported for ARAD B0 and above.
 * 2) Make sure the Makefile used includes PTP feature.
 * 3) Enable TS_PLL clock by enabling/un-commenting following soc property:
 *      num_queues_pci.BCM88650=40
 *      num_queues_uc1.BCM88650=8
 *      custom_feature_ptp_cosq_port.BCM88650=204
 *      ucode_port_204.BCM88650=CPU.40
 *      tm_port_header_type_in_204.BCM88650=ETH
 *      tm_port_header_type_out_204.BCM88650=ETH
 *      tm_port_header_type_in_0.BCM88650=INJECTED_2
 *      tm_port_header_type_out_0.BCM88650=TM
 *      ext_1588_mac_enable_0.BCM88650=1
 *      ext_1588_mac_enable_204.BCM88650=1
 *      phy_1588_dpll_frequency_lock.BCM88650=1
 *
 *    (otherwise the TS clock would not run, as a result the CF won`t be updated)
 * 4) In case the system contain more than one device, need to run Broadsync application (see 'Broadsync' section below).
 *
 * Provides the following functionalities:
 *
 * - 1588 one/two step TC (Transparent clock).
 * - trap/drop/snoop/fwd control over 1588 messages.
 *
 *    In transport clock mode, every fabric based system is observed by the network as a single transparent
 *    clock entity, with the residence time calculated from the moment the PTP packet enters the system through
 *    one port, to the time it exits through another.
 *    when 1 step TC is enabled:
 *       The system updates the correction field (CF) of Event messages
 *    when 2 step TC is enabled:
 *       The system records the TX time of Event messages in a FIFO (the application can later read the TX time from the FIFO,
 *       calculate residence time and update the relevant Follow up message)
 *
 *      Event Messages:
 *        1. Sync
 *        2. Delay_Req
 *        3. Pdelay_Req
 *        4. Pdelay_Resp
 *      General Messages:
 *        1. Announce
 *        2. System
 *        3. Follow_Up
 *        4. Delay_Resp
 *        5. Pdelay_Resp_Follow_Up
 *        6. Management
 *        7. Signaling
 *
 *    Supported 1588 encapsulations:
 *      follwing 1588 encapsulations are supported:
 *        1. 1588oE                    switched
 *        2. 1588oUDPoIPoE             switched/IP routed
 *        3. 1588oUDPoIPoIPoE          switched/IP routed/IP terminated
 *        4. 1588oUDPoIPoMPLSoE        switched/MPLS routed/MPLS terminated
 *        5. 1588oEoMPLSoE             switched/MPLS routed/MPLS terminated
 *    CF update:
 *        the CF (8 bytes) update is done in bytes number 2,3 (zero based).
 *        additionally, least significant bit can be changed for internal use.
 *
 *    Broadsync:
 *     1588 TC (Transparent clock) relay on clock synchronization between devices in the system i.e. Broadsync,
 *     for Broadsync configuration see BCM shell "bs config" command (see example in cint_time.c).
 *
 *
 *
 *
 *
 *
 */
/*
 * how to run:
 *
 * For example: enable one step TC for unit 0 port 13, 14 (both In-port and Out-port need to be configured.
 *              the packet timestamp is saved at the ingress, the packet CF is updated at the egress):
 *
 * cint ../../../../src/examples/dpp/cint_1588.c
 * cint
 * int rv;
 * rv = ieee_1588_port_set(unit,13,1);
 * print rv;
 * rv = ieee_1588_port_set(unit,14,1);
 * print rv;
 *
 * For example: disable one step TC for unit 0 port 13:
 *
 * cint ../../../../src/examples/dpp/cint_1588.c
 * cint
 * int rv;
 * rv = ieee_1588_port_set(unit,13,0);
 * print rv;
 *
 * For example: PTP master running
 *  #Load ukernel image
 *  mcsload 0 BCM88660_A0_0_bfd_bhh.srec initmcs=1
 *  mcsload 1 BCM88660_A0_1_ptpfull.srec
 *  #PTP configuration
 *  ptp Register Events
 *  ptp Register Signal
 *  ptp Stack Create
 *  ptp Debug STA SYA
 *  ptp register arp
 *  ptp Clock Create NumPorts=1 ID=00:0e:03:ff:fe:00:01:09 P1=80 tr=y MaxUnicastSlaves=100
 *  ptp Port Configure 1 MAC=00:0e:03:00:01:09 IP=192.168.1.9 MC=0 AI=1 SI=-6 DI=-6 TM=mac32 Vlan=2
 *  ptp slave add IP=192.168.1.10 MAC=00:0e:03:00:01:10 SI=-6 DI=-6 AI=1
 *  s IPT_FORCE_LOCAL_OR_FABRIC FORCE_LOCAL=1
 *  cint ../../../../src/examples/dpp/cint_1588.c
 *  cint
 *  print ieee_1588_run(unit, 3, 2);
 *  exit;
 *  # Enable 1pps
 *  ptp signals set p=1 freq=1 pl=y width=100000000
 *
 * For example: PTP slave running
 *  #Load ukernel image
 *  mcsload 0 BCM88660_A0_0_bfd_bhh.srec initmcs=1
 *  mcsload 1 BCM88660_A0_1_ptpfull.srec
 *  #PTP configuration
 *  ptp Register Events
 *  ptp Register Signal
 *  ptp Stack Create
 *  ptp Debug STA SYA
 *  ptp register arp
 *  ptp Clock Create NumPorts=1 ID=00:0e:03:ff:fe:00:01:10 P1=128 MaxUnicastSlaves=100
 *  ptp Port Configure 1 MAC=00:0e:03:00:01:10 IP=192.168.1.10 MC=0 AI=1 SI=-6 DI=-6 TM=mac32 Vlan=2
 *  ptp master add static=n IP=192.168.1.9 MAC=00:b0:ae:03:58:6f SI=-6 DI=-6 AI=1
 *  s IPT_FORCE_LOCAL_OR_FABRIC FORCE_LOCAL=1
 *  cint ../../../../src/examples/dpp/cint_1588.c
 *  cint
 *  print ieee_1588_run(unit, 3, 2);
 *  exit;
 *
 */



/*
 * enable = 1, enable  one step TC
 * enable = 0, disable one step TC
 */
int ieee_1588_port_set(int unit, bcm_gport_t port, int enable)
{
    int                           rv = BCM_E_NONE;
    bcm_port_timesync_config_t timesync_config;
    bcm_port_timesync_config_t *timesync_config_ptr;
    int config_count;

    if(0 == enable) {
        timesync_config_ptr = NULL;
        config_count        = 0;
    } else {

        /* flags to enable 1 step TC                                                                   */
        timesync_config.flags = (BCM_PORT_TIMESYNC_DEFAULT | BCM_PORT_TIMESYNC_ONE_STEP_TIMESTAMP);
        /* to enable 2 step TC use folowing flags instead                                              */
        /* timesync_config.flags = (BCM_PORT_TIMESYNC_DEFAULT | BCM_PORT_TIMESYNC_TWO_STEP_TIMESTAMP); */
        timesync_config.pkt_drop  = 0;
        timesync_config.pkt_tocpu = 0;

        /* for example to trap PDELAY_REQ message and drop PDELAY_RESP message use following            */
        /* timesync_config.pkt_tocpu |= BCM_PORT_TIMESYNC_PKT_PDELAY_REQ;                               */
        /* timesync_config.pkt_drop  |= BCM_PORT_TIMESYNC_PKT_PDELAY_RESP;                              */
        /*                                                                                              */
        /* 2 notes:                                                                                     */
        /* 1) each 1588 message can be added to pkt_tocpu bitmask or pkt_drop, not both.                */
        /*    in case no bit is turned on the packet will be forwarded.                                 */
        /* 2) prior to trap 1588 message, the 1588 trap need to be cofigures with a port,               */
        /*    for example, following will trap 1588 messages raised in pkt_tocpu bitmask to port 200:   */
        /*      cint ../../../../src/examples/dpp/cint_rx_trap_fap.c                                    */
        /*      cint                                                                                    */
        /*      int rv;                                                                                 */
        /*      int trap_id_handle;                                                                     */
        /*      rv = set_rx_trap(unit, bcmRxTrap1588, 0, &trap_id_handle, 200);                            */
        /*      print rv;                                                                               */
        /*      print trap_id_handle;                                                                   */


        timesync_config_ptr = &timesync_config;
        config_count        = 1;
    }

    rv =  bcm_port_timesync_config_set(unit, port, config_count, timesync_config_ptr);
    if (rv != BCM_E_NONE) {
        if(rv == BCM_E_UNAVAIL) {
            printf("Error, bcm_port_timesync_config_set 1588 is not supported for ARAD A0\n");
        } else {
            printf("Error, bcm_port_timesync_config_set rv = %d\n", rv);
        }
        return rv;
    }

    return rv;
}


/*
 * enable = 1, one step TC is enabled
 * enable = 0, one step TC is disabled
 */
int ieee_1588_port_get(int unit, bcm_gport_t port, int *enabled)
{
    int                           rv = BCM_E_NONE;
    bcm_port_timesync_config_t timesync_config;
    int array_count;

    rv =  bcm_port_timesync_config_get(unit, port, 1, timesync_config, &array_count);
    if (rv != BCM_E_NONE) {
        if(rv == BCM_E_UNAVAIL) {
            printf("Error, bcm_port_timesync_config_set 1588 is not supported for ARAD A0\n");
        } else {
            printf("Error, bcm_port_timesync_config_get rv = %d\n", rv);
        }
        return rv;
    }

    if(0 == array_count) {
        *enabled = 0;
        printf("1588 is DISABLED for port = %d \n", port);
    } else {
        *enabled = 1;
        printf("1588 is ENABLED  for port = %d \n", port);
    }

    return rv;
}

/*Support 1588 ingress disable and egress enable. The input parameter is ingress port*/
int ieee_1588_port_set_ingress_disable_and_egress_enable(int unit, bcm_gport_t port)
{
    int                           rv = BCM_E_NONE;
    bcm_port_timesync_config_t timesync_config;
    bcm_port_timesync_config_t *timesync_config_ptr;
    int config_count;

    /* flags to enable 1 step TC                                                                   */
    timesync_config.flags = (BCM_PORT_TIMESYNC_DEFAULT | BCM_PORT_TIMESYNC_ONE_STEP_TIMESTAMP);
    /* to enable 2 step TC use folowing flags instead                                              */
    /* timesync_config.flags = (BCM_PORT_TIMESYNC_DEFAULT | BCM_PORT_TIMESYNC_TWO_STEP_TIMESTAMP); */
    timesync_config.pkt_drop  = 0;
    timesync_config.pkt_tocpu = 0;

    /* for example to trap PDELAY_REQ message and drop PDELAY_RESP message use following            */
    /* timesync_config.pkt_tocpu |= BCM_PORT_TIMESYNC_PKT_PDELAY_REQ;                               */
    /* timesync_config.pkt_drop  |= BCM_PORT_TIMESYNC_PKT_PDELAY_RESP;                              */
    /*                                                                                              */
    /* 2 notes:                                                                                     */
    /* 1) each 1588 message can be added to pkt_tocpu bitmask or pkt_drop, not both.                */
    /*    in case no bit is turned on the packet will be forwarded.                                 */
    /* 2) prior to trap 1588 message, the 1588 trap need to be cofigures with a port,               */
    /*    for example, following will trap 1588 messages raised in pkt_tocpu bitmask to port 200:   */
    /*      cint ../../../../src/examples/dpp/cint_rx_trap_fap.c                                    */
    /*      cint                                                                                    */
    /*      int rv;                                                                                 */
    /*      int trap_id_handle;                                                                     */
    /*      rv = set_rx_trap(unit, bcmRxTrap1588, 0, &trap_id_handle, 200);                            */
    /*      print rv;                                                                               */
    /*      print trap_id_handle;                                                                   */


    timesync_config_ptr = &timesync_config;
    config_count = 0;

    rv =  bcm_port_timesync_config_set(unit, port, config_count, timesync_config_ptr);
    if (rv != BCM_E_NONE) {
        if(rv == BCM_E_UNAVAIL) {
            printf("Error, bcm_port_timesync_config_set 1588 is not supported for ARAD A0\n");
        } else {
            printf("Error, bcm_port_timesync_config_set rv = %d\n", rv);
        }
        return rv;
    }

    return rv;
}



/* The port number is based on followed soc configuration
 *     num_queues_pci.BCM88375=40
 *     num_queues_uc1.BCM88375=8
 *     custom_feature_ptp_cosq_port.BCM88375=204
 */
int uc_port = 204;
int ieee_1588_trap(int unit, int port) {

    int trap_id=0;
    int rv = BCM_E_NONE;
    bcm_rx_trap_config_t trap_config;
    bcm_field_qset_t qset;
    bcm_field_aset_t aset;
    bcm_field_entry_t ent;
    bcm_field_group_t grp;
    bcm_gport_t gport;

    /* create trap */
    rv = bcm_rx_trap_type_create(unit, 0, bcmRxTrapUserDefine, &trap_id);
    if (rv != BCM_E_NONE) {
        printf("Error in bcm_rx_trap_type_create (%s) \n",bcm_errmsg(rv));
        return rv;
    }

    /* Configure trap attribute to update destination */
    sal_memset(&trap_config, 0, sizeof(trap_config));
    trap_config.flags = (BCM_RX_TRAP_UPDATE_DEST | BCM_RX_TRAP_TRAP | BCM_RX_TRAP_REPLACE);
    trap_config.trap_strength = 0;
    trap_config.dest_port = uc_port;

    rv = bcm_rx_trap_set(unit, trap_id, &trap_config);
    if (rv != BCM_E_NONE) {
        printf("Error in bcm_rx_trap_set (%s) \n",bcm_errmsg(rv));
        return rv;
    }
    BCM_GPORT_TRAP_SET(gport, trap_id, 7, 0);

    BCM_FIELD_QSET_INIT(qset);
    BCM_FIELD_QSET_ADD(qset, bcmFieldQualifyStageIngress);
    BCM_FIELD_QSET_ADD(qset, bcmFieldQualifyL4DstPort);
    BCM_FIELD_QSET_ADD(qset, bcmFieldQualifyInPort);
    BCM_FIELD_ASET_INIT(aset);
    BCM_FIELD_ASET_ADD(aset, bcmFieldActionTrap);

    rv = bcm_field_group_create(unit, qset, 2, &grp);
    if (rv != BCM_E_NONE) {
        printf("Error in bcm_field_group_create (%s) \n",bcm_errmsg(rv));
        return rv;
    }

    rv = bcm_field_group_action_set(unit, grp, aset);
    if (rv != BCM_E_NONE) {
        printf("Error in bcm_field_group_action_set (%s) \n",bcm_errmsg(rv));
        return rv;
    }

    rv = bcm_field_entry_create(unit, grp, &ent);
    if (rv != BCM_E_NONE) {
        printf("Error in bcm_field_entry_create for L4 port 319 (%s) \n",bcm_errmsg(rv));
        return rv;
    }

    rv = bcm_field_qualify_InPort(unit, ent, port, 0xffffffff);
    if (rv != BCM_E_NONE)
    {
        printf("Error in bcm_field_qualify_InPort for L4 port 319 (%s) \n",bcm_errmsg(rv));
        return rv;
    }

    /* Port 319 is dedicated for 1588 protocol */
    rv = bcm_field_qualify_L4DstPort(unit, ent, 319, 0xffff);
    if (rv != BCM_E_NONE) {
        printf("Error in bcm_field_qualify_L4DstPort for L4 port 319 (%s) \n",bcm_errmsg(rv));
        return rv;
    }

    rv = bcm_field_action_add(unit, ent, bcmFieldActionTrap, gport, 0);
    if (rv != BCM_E_NONE) {
        printf("Error in bcm_field_action_add for L4 port 319 (%s) \n",bcm_errmsg(rv));
        return rv;
    }

    rv = bcm_field_entry_install(unit, ent);
    if (rv != BCM_E_NONE) {
        printf("Error in bcm_field_entry_install for L4 port 319 (%s) \n",bcm_errmsg(rv));
        return rv;
    }

    rv = bcm_field_entry_create(unit, grp, &ent);
    if (rv != BCM_E_NONE) {
        printf("Error in bcm_field_entry_create for L4 port:320 (%s) \n",bcm_errmsg(rv));
        return rv;
    }

    rv = bcm_field_qualify_InPort(unit, ent, port, 0xffffffff);
    if (rv != BCM_E_NONE)
    {
        printf("Error in bcm_field_qualify_InPort for L4 port:320 (%s) \n",bcm_errmsg(rv));
        return rv;
    }

    /* Port 320 is dedicated for 1588 protocol */
    rv = bcm_field_qualify_L4DstPort(unit, ent, 320, 0xffff);
    if (rv != BCM_E_NONE) {
        printf("Error in bcm_field_qualify_L4DstPort for L4 port:320 (%s) \n",bcm_errmsg(rv));
        return rv;
    }

    rv = bcm_field_action_add(unit, ent, bcmFieldActionTrap, gport, 0);
    if (rv != BCM_E_NONE) {
        printf("Error in bcm_field_action_add for L4 port:320 (%s) \n",bcm_errmsg(rv));
        return rv;
    }

    rv = bcm_field_entry_install(unit, ent);
    if (rv != BCM_E_NONE) {
        printf("Error in bcm_field_entry_install for L4 port:320 (%s) \n",bcm_errmsg(rv));
        return rv;
    }

    return rv;
}

int ieee_1588_run(int unit, int port, int vlan) {
    int rv = BCM_E_NONE;
    rv = ieee_1588_port_set(unit, uc_port, 1);
    if (rv != BCM_E_NONE) {
        printf("Error in ieee_1588_port_set for port:%d (%s) \n", uc_port, bcm_errmsg(rv));
        return rv;
    }

    rv = ieee_1588_port_set(unit, port, 1);
    if (rv != BCM_E_NONE) {
        printf("Error in ieee_1588_port_set for port:%d (%s) \n", port, bcm_errmsg(rv));
        return rv;
    }

    rv = ieee_1588_trap(unit, port);
    if (rv != BCM_E_NONE) {
        printf("Error in ieee_1588_trap (%s) \n", port, bcm_errmsg(rv));
        return rv;
    }

    /* Setup general bridge service for vlan 2 */
    rv = bcm_vlan_create(unit, vlan);
    if (rv != BCM_E_NONE) {
        printf("Error in bcm_vlan_create vlan:%d (%s) \n", vlan, bcm_errmsg(rv));
        return rv;
    }

    rv = bcm_vlan_gport_add(unit, vlan, port, 0);
    if (rv != BCM_E_NONE) {
        printf("Error in bcm_vlan_gport_add vlan:%d port:%d(%s) \n", vlan, port, bcm_errmsg(rv));
        return rv;
    }

    rv = bcm_vlan_gport_add(unit, vlan, uc_port, 0);
    if (rv != BCM_E_NONE) {
        printf("Error in bcm_vlan_gport_add vlan:%d port:%d (%s) \n", vlan, port, bcm_errmsg(rv));
        return rv;
    }

    return rv;
}
