/*~~~~~~~~~~~~~~~~~~~~~~~Mulitpoint VPLS Service~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/* $Id: cint_vswitch_vpls.c,v 1.22 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File: cint_vswitch_vpls.c
 * Purpose: Example of Open Multi-Point VPLS service and attach logical ports to the service. 
 *          The attached ports can be attachment circuits or PWEs.
 *
 * Attachment circuit (AC): Ethernet port attached to the service based on port-vlan-vlan, 
 *                          connect from access side. 
 * PWE: Virtual circuit attached to the service based on VC-label. Connect to the MPLS 
 *      network side.
 * 
 * For this service, multiple logical ports can be attached where each logical port can be:
 *
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~    
 * |                                             . . .       . . .                                       |
 * |                                           .       .   .       .                                     |
 * |                                       . .    +---------+     .   . .                                |
 * |                                     .        |         |  /\   .     .                              |
 * |                                   .          |   PE3   |  \\          .                             |
 * |                                   .       /\ | Switch  |/\ \\         .                             |
 * |                                    .      // +---------+\\  \\       .                              |
 * |                                   .      //              \\  +------+ .                             |
 * |                                  . +------+               \\ |TUNNEL| .                             |
 * |                                 .  |TUNNEL|                \\+------+.                              |
 * |                                 .  +------+                 \\  \\    .          . . .              |
 * |                  . .            .    //                      \\  \\    .      . .      .     .  .   |
 * |             .  .     . .       .    /\/    +------------+    \/\ \/\   .     . +----+    . .      . | 
 * |    . .   .   +----+     .  +---------+ <---| - - - - - -|---- +---------+   .  |UNIc|             . |
 * |  .     .     |UNIa|--------|         | ----| - - - - - -|---> |         |----- +----+  +----+   .   |           
 * |  .  +----+   +----+     .  |   PE1   |     |    MPLS    |     |   PE2   |--------------|UNId|     . |
 * |   . |UNIb|-----------------| Switch  | <---| - TUNNEL- -|---- | Switch  |--- +----+    +----+     . |
 * | .   +----+              .  +---------+ ----| - - - - - -|---> +---------+  . |UNIe|             .   |
 * |.            ETHERNET     .      .     . .  +------------+              .  .  +----+   ETHERNET .    |
 * | .     .       .     . . .         . .     .       .       .    .      .    .      .       .     .   |
 * |   . .   . . .   . .                         . . .   . . .   . .  . . .      . . .   . . .   . .     |
 * |                                                                                                     |
 * |           +-------------+      +---------------------------------+      +-------------------------+ |
 * |           | Ethr | Data |      | Ethr | MPLS | PWE | Ethr | Data |      | Ethr |S-TAG|C-TAG| Data | |
 * |           +-------------+      +---------------------------------+      +-------------------------+ | 
 * |                                                                                                     |
 * |                      +~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+                              |
 * |                      | Figure 13: Multipoing VPLS Service Attachment |                              |
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 *                                                                                                           
 *  This CINT configures a provider edge as shown in Figure 14.                                              
 *  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ 
 *  |  Access-P1: Access port, defined on port1 with outer-VID10 and inner-VID20 |
 *  |                                                . . .   . .                 |
 *  |                                        . .   .     @ .     .               |
 *  |                                      .     .      /PWE-2     .             |
 *  |                                      .           /            .            |
 *  |                                      .     +---------------+  .            |
 *  |       . .   . . .                   .    <-|---PWE(2009)---|- .            |
 *  |   . .     .       .                .      -|---PWE(1981)---|->             |
 *  |  . +--+             .             .        +---------------+.              |
 *  | .  |  |----------------------- +---------+ /TUNNELS(300,400) .             |
 *  | .  +--+  Access-P1<1,10,20>    |         |/                  .             |
 *  |  .                   .         |   PE1   @                  .              |
 *  | .   +--+              .        | Switch  |\                  .             |
 *  | .   |  |---------------------- +---------+ \                   .           |             
 *  |  .  +--+  Access-P2<1,15,30>        .       \TUNNELS(1000,1002) .          |
 *  |    .               .                  .     +---------------------+   .    |      
 *  |     .   ETHERNET .                     .  <-|---PWE(2010,in_port)-|-   .   |
 *  |       . .  . . .                     .     -|---PWE(1982,out_port)|->  .   |
 *  |                                      .      +---------------------+  .     |
 *  |                                      .            \            .           |
 *  |                                        .           \PWE-1  . .             |
 *  |                                          .       .  @    .                 |
 *  |                                            . . .   . . .                   |      
 *  |            +~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+                 |
 *  |            | Figure 14: CINT Provider Edge Configuration |                 |
 *  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 *
 * Explanation:
 *  -   Access-P2: Access port, defined on port 1 with outer-VID 15 and inner-VID 30.
 *  -   PWE-1: network port with incoming VC = 2010, egress-VC = 1982 defined over two tunnels 1000 and 1002.
 *  -   PWE-2: network port with incoming VC = 2009, egress-VC = 1981 defined over two tunnels, 300 and 400.
 *  -   For access ports outer-Tag TPID is 0x8100 and inner Tag TPID is 0x9100.
 *  -   Access-P1 and Access-P2 refer to the logical interface (attachment circuit).
 *  -   P2 refers to the physical ports Access-P1 and Access-P2 are attached to.
 *  -   PWE1 and PWE2 refers to MPLS network ports.
 *  -   P1 refers to the physical ports PWE1 and PWE2 are attached to.
 *
 * Headers:
 *  
 *   ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 *   |  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+  |
 *   |  | DA |SA||TIPD1 |Prio|VID||  MPLS    |  MPLS    ||   PWE    ||Eth||Data ||
 *   |  |0:11|  ||0x8100|    |100||Label:1000|Label:1002||Lable:2010||   ||     || 
 *   |  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+  |
 *   |                +~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+                  |
 *   |                | Figure 15: Packets Received from PWE1 |                  | 
 *   ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ 
 *
 *   ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 *   |  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+    |
 *   |  |DA|SA||TIPD1 |Prio|VID||  MPLS    |  MPLS    ||   PWE    ||Eth||Data |  |
 *   |  |  |  ||0x8100|    |100||Label:1000|Label:1002||Lable:1982||   ||     |  | 
 *   |  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+    |
 *   |                +~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+                 |
 *   |                | Figure 16: Packets Transmitted to PWE1 |                 | 
 *   ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~  
 *
 *   ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 *   |  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+  |
 *   |  | DA |SA||TIPD1 |Prio|VID||  MPLS   |  MPLS   ||   PWE    ||Eth||Data |  |
 *   |  |0:11|  ||0x8100|    |100||Label:300|Label:400||Lable:2009||   ||     |  | 
 *   |  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+  |
 *   |                +~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+                  |
 *   |                | Figure 17: Packets Received from PWE2 |                  | 
 *   ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~  
 *
 *   Packets Transmitted to PWE2 same as Packets Transmitted to PWE1, see Figure 16
 * 
 * Access side packets:
 * 
 *   ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 *   |                       tag1                 tag2                           |
 *   |   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+   |
 *   |   | DA | SA || TIPD1 | Prio | VID || TIPD2 | Prio | VID ||    Data    |   |
 *   |   |    |    || 0x8100|      | 10  || 0x9100|      | 20  ||            |   |
 *   |   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+   |
 *   |        +~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+       |
 *   |        | Figure 18: Packets Received/Transmitted on Access Port 1 |       |
 *   ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~  
 *
 *   ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 *   |                       tag1                 tag2                           |
 *   |   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+   |
 *   |   | DA | SA || TIPD1 | Prio | VID || TIPD2 | Prio | VID ||    Data    |   |
 *   |   |    |    || 0x8100|      | 15  || 0x9100|      | 30  ||            |   |
 *   |   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+   |
 *   |        +~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+       |
 *   |        | Figure 19: Packets Received/Transmitted on Access Port 2 |       |
 *   ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~  
 *
 * Calling sequence:
 *  -   Port TPIDs setting:
 *      -   For P2 set outer TPID to 0x8100 and inner TPID to 0x9100 (using cint_port_tpid.c).
 *      -   For P1 set outer TPID to 0x8100 and no inner TPID (using cint_port_tpid.c).
 *  -   Set up MPLS tunnels. Refer to mpls_tunnels_config().
 *      -   Set MPLS L2 termination (on ingress).
 *          -    MPLS packets arriving with DA 00:00:00:00:00:11 and VID 100 causes L2 termination.  
 *          -    Calls bcm_l2_tunnel_add().
 *      -   Add pop entries to MPLS switch.
 *          -    MPLS packet arriving from the MPLS network side with labels 1000 or 1002 the label/s 
 *               are popped (refer to mpls_add_pop_entry).
 *          -    The same applies to MPLS packets arriving from the MPLS network side 
 *               with labels 400 or 300.
 *          -    Calls bcm_mpls_tunnel_switch_create().
 *          -    Create MPLS L3 interface (on egress).
 *          -    Packet routed to this L3 interface is set with this MAC. 
 *          -    Calls bcm_l3_intf_create().
 *      -   Create MPLs tunnels over the created l3 interface.
 *          -    Packet routed to above interface is tunneled into MPLS tunnels. Where labels set to 
 *               1000 and 1002, respectively.
 *          -    Calls bcm_mpls_tunnel_initiator_create().
 *      -   Create egress object points to the above l3-interface.
 *          -    Packet routed to this egress-object is forwarded to P1 tunneled with above MPLS tunnels 
 *               and with the L3-interface VID and MAC.
 *          -    Calls bcm_l3_egress_create().
 *  -   Create multipoint VSI #6202 (refer to mpls_port__vswitch_vpls_vpn_create__set).
 *      -   calling bcm_mpls_vpn_id_create().
 *          -    You has to supply VPN ID and multicast groups. For this purpose, the following flags have 
 *               to be present BCM_MPLS_VPN_VPLS|BCM_MPLS_VPN_WITH_ID.
 *          Note that uc/mc/bc group have the same value as VSI. Another option is to set uc_group = VSI, 
 *          and mc = uc + 4k, and bc = mc + 4k.
 *      -   Use open_egress_mc_group() to open multicast group for flooding.
 *          -    Multicast group ID is equal to VPN ID.
 *  -   Add MPLS-ports to the VSI.
 *      -   vswitch_vpls_add_access_port_1/2 creates attachment circuit and add them to the Vswitch and 
 *          update the Multicast group.
 *      -   vswitch_vpls_add_network_port_1/2 creates PWE and add them to the VSI and update the 
 *          multicast group. 
 * 
 * Traffic:
 *  Flooding packets incoming from MPLS network:
 *  Send the following packet, where the DA of the inner Ethernet is unknown on VSI.
 *
 *  Flooding Packets Incoming from MPLS Network same as Packets Received from PWE1, see Figure 15
 *
 *  -   Packet is flooded (as DA is not known):
 *      -   Packet is received. 
 *          -    In access P1: physical port P2 with VLAN tag1: VID =10, VLAN tag2: VLAN ID = 20. 
 *          -    In access P2: physical port P2 with VLAN tag1 VID =15, VLAN tag2: VLAN ID = 30. 
 *          Note that the packet is not received on PWE1 or PWE2 due to split-horizon filtering. 
 *          A packet received from HUB is not flooded back to HUB.
 *  -   The SA of the packet is learned 6202.
 *      -   Call l2_print_mact(unit) to print MAC table content.
 *
 *  Flooding packets incoming access side.
 *  -   Send Ethernet packet from access-P1:
 *      -   Any SA
 *      -   Unknown DA
 *      -   VLAN tag1: VLAN tag type 0x8100, VID =10
 *      -   VLAN tag2: VLAN tag type 0x9100, VID =20 
 *
 *  Flooding Packets from Incoming Access Side same as Packets Received/Transmitted 
 *  on Access Port 1, see Figure 18.
 * 
 *  -   Packet is flooded (as DA is not known).
 *      -   Packet is received: 
 *          -   On physical port P1:
 *              -  PWE1,2: With VC label and MPLS tunnels as described above.
 *          -   On physical port P2:
 *              -   In P2: port 1 with VLAN tag1: VID =3, VLAN tag2: VLAN ID = 6.
 *          Note that the packet is not received on access P1, as packet was injected 
 *          from this logical port, that is, hair-pin filtering.
 *  -   The SA of the packet is learned on VSI 6202.
 *      -   Call l2_print_mact(unit) to print MAC table content. 
 *
 * Sending to known DA
 *  -   Send Ethernet packet from any logical interface PWE or Access port with known DA on VSI 6202 
 *      the packet will be forwarded to the specific logical port with required editing.
 *
 * Remarks:
 *  -   For this application at least two ports are needed, one for access side and one for MPLS network side
 *  -   User can statically add entry to the MACT pointing to a gport using 
 *      vswitch_add_l2_addr_to_gport(unit, gport_id)
 *  -   Set verbose = 1/0 to show/hide informative prints
 *  -   When calling vswitch_vpls_run(unit, extend_example), extend_example
 *      o   if 0: service includes 2 ports, one Access port, and one network (VPLS) port
 *      o   else: service includes 4 ports, two Access ports, and two network (VPLS) ports as 
 *          described in the above figure
 *  -   Calling vswitch_vpls_run() as is more than one time would fail because this call will try open same 
 *      VPN twice,  to avoid this user should set vswitch_vpls_info_1.vpn_id different value in order to 
 *      open new service.
 *  -   Call switch_pwe_tunnel function to switch PWE1 from mpls tunnel (1000,1002) to the other (300,400) that was prevously configured for PWE2.
 *      This configures only the multicast traffic to go throught the new tunnel, unicast traffic is not effected.
 *  -   make_before_break variable can be set in order to configure both mplt ports with same vc label, then remove the first port.
 *  -   vccv_type3_ttl variable can be set in order to configure VCCV type 3 on PWE1. This configuration will trap packets with TTL<=1 to the control plane.
 *  -   encap_optimized variable is used to enable EEDB optimized mode.
 *           In this mode of PWE over protected MPLS tunnel, only one EEDB entry is allocated for PWE OutLif (as opposed to two entries in the regular mode),
 *           which is pointed by the MACT (as opposed to being pointer by the FEC). The FEC in this mode points to MPLS tunnel EEDB entry.
 *           The protection switching in this mode is done by changing the PWE OutLif pointer using bcm_mpls_tunnel_initiator_create api.
 *           Thus EEDB resources are reduced by cost of protection switching time.
 *           In addition, in this mode FEC should contain EEI information instead of OutLif in the regular mode.
 *           To use this mode two apis should be set with flags:
 *                bcm_l3_egress_create flag BCM_L3_ENCAP_SPACE_OPTIMIZED should be set to configure FEC format to OutLif.
 *                bcm_mpls_port_add flag BCM_MPLS_PORT2_LEARN_ENCAP should be set when configuring ingress properties to enable OutLif learning.
 * - When working with fabric + egress multicast group, it is user responsible to configure fabric multicast group to send to the correct device.
 *       In case of working on single-device then fabric_connect_mode soc property must be set with SINGLE_FAP
 * 
 * To Activate Above Settings Run:
 *  	BCM> cint examples/dpp/utility/cint_utils_global.c
 *  	BCM> cint examples/dpp/utility/cint_utils_vlan.c
 *  	BCM> cint examples/dpp/utility/cint_utils_mpls.c
 *  	BCM> cint examples/dpp/utility/cint_utils_mpls_port.c
 *      BCM> cint examples/dpp/utility/cint_utils_multicast.c
 *      BCM> cint examples/dpp/utility/cint_utils_l2.c
 *      BCM> cint examples/dpp/utility/cint_utils_l3.c
 *      BCM> cint examples/dpp/cint_port_tpid.c
 *      BCM> cint examples/dpp/cint_advanced_vlan_translation_mode.c
 *      BCM> cint examples/dpp/cint_qos.c
 *      BCM> cint examples/dpp/cint_mpls_lsr.c
 *      BCM> cint examples/dpp/cint_vswitch_metro_mp.c
 *      BCM> cint examples/dpp/cint_vswitch_vpls.c
 *      BCM> cint
 *      cint> int rv;
 *      cint> rv = vswitch_vpls_run_with_defaults(unit, second_unit, extend_example); 
 * 
 * Script adjustment:
 *  User can adjust the following attribute of the application
 *  -   vpn_id:             VPN id to create 
 *  -   ac_in_port:         accesss port: physical port connect to customer 
 *  -   pwe_in_port:        network port: physical port connect to MPLs network (for incoming packets)
 *  -   pwe_out_port:       network port: physical port connect to MPLs network (for outgoing packets) 
 *  -   in_vc_label:        incomming VC label 
 *  -   eg_vc_label:        egress VC label 
 *  -   in_tunnel_label:    inner tunnel label 
 *  -   out_tunnel_label:   outer tunnel label 
 *  -   pwe_cw:             adding CW to all the PWEs
 *  -   is_gal:             configure PW-ACH termination for BFD vccv Type 4 packets.
 *  -   pwe_php:            No mpls tunnels for PWE interface
 *                          Note: in this case VLAN must be valid in bcm_l3_egress_create in order to infer the l3 interface.
 *
 */
int verbose1 = 1;
int default_vpn_id = 6202;
int default_cpu_port = 0;
int ingress_egress_separate_ports = 0;

bcm_if_t pwe_encap_id;
bcm_if_t second_mpls_intf;
bcm_gport_t network_port_id;

int pwe_cw = 0;
int is_gal = 0;
int encap_optimized = 0; /* Use EEDB optimized mode */
uint8 make_before_break = 0; /* configuring two mpls ports with same vc label */
uint8 vccv_type3_ttl = 0;
int pwe_network_group_id_1 = 1;
int pwe_network_group_id_2 = 1;
int ac_network_group_id_ingress_1 = 0;
int ac_network_group_id_egress_1 = 0;
int ac_network_group_id_ingress_2 = 0;
int ac_network_group_id_egress_2 = 0;
int test_default_ttl_inheritance = 0;
int pwe_php = 0;

    bcm_failover_t pwe_failover_id_fec;


int mpls_termination_label_index_enable = 0;

/* Used for multicast configuration */
int vlan_port_encap_id1;
int vlan_port_encap_id2;
int mpls_port_encap_id1;
int mpls_port_encap_id2;
int mpls_tunnel_encap_id1;
int mpls_tunnel_encap_id2;

struct vswitch_vpls_label_id_s {
	int label1;
	int id1;

	int label2;
	int id2;

	int label3;
	int id3;
};

vswitch_vpls_label_id_s vswitch_vpls_pairs;
bcm_gport_t exported_egress_mpls_port_id;

int port_based_pwe_enable = 0;

struct vswitch_vpls_info_s {

    int in_vc_label; /* incomming VC label */
    int eg_vc_label; /* egress VC label */
    int ac_in_port;  /* accesss port: physical port connect to customer */
    int pwe_in_port; /* network port: physical port connect to MPLs network (in) */
    int pwe_out_port; /* network port: physical port connect to MPLs network (out) */
  
    bcm_vpn_t vpn_id; /* VPN id to open */
  
    /* tunnels info */
    int in_tunnel_label;
    int out_tunnel_label;

    int tunnel_id;
    /* Routing interface to which mpls is connected */
    int rif;
    int encap_id;
    int mpls_port_id;
  
    int ac_port1_outer_vlan;
    int ac_port1_inner_vlan;
    int ac_port2_outer_vlan;
    int ac_port2_inner_vlan;

    int access_index_1 ;
    int access_index_2 ;
    int access_index_3 ;

    int ac_port1_flags;
    int ac_port2_flags;

    bcm_gport_t access_port_id;
};
vswitch_vpls_info_s vswitch_vpls_info_1;
vswitch_vpls_info_s vswitch_vpls_info_2;

bcm_mac_t vswitch_vpls_my_mac_get() {
    return mpls_lsr_my_mac_get();
}

int
vswitch_vlps_info_init(int extend_example, int ac_port, int pwe_in_port, int pwe_out_port, int ac_port1_outer_vlan, int ac_port1_inner_vlan,
                       int ac_port2_outer_vlan, int ac_port2_inner_vlan, int vpn_id) {
    int rv = BCM_E_NONE;

    /* VPN id to create */
    if (vpn_id < 0) {
        vswitch_vpls_info_1.vpn_id = default_vpn_id;
    } else {
        vswitch_vpls_info_1.vpn_id = vpn_id; 
    }

    /* incomming packet attributes */
    vswitch_vpls_info_1.ac_in_port = ac_port;
    vswitch_vpls_info_1.pwe_in_port = pwe_in_port;
    vswitch_vpls_info_1.pwe_out_port = pwe_out_port;
    vswitch_vpls_info_1.in_vc_label = 2010;
    vswitch_vpls_info_1.eg_vc_label = 1982;

    /* tunnel info */
    vswitch_vpls_info_1.in_tunnel_label = 1000;
    vswitch_vpls_info_1.out_tunnel_label = 1002;
    
    vswitch_vpls_info_1.ac_port1_outer_vlan = ac_port1_outer_vlan;
    vswitch_vpls_info_1.ac_port1_inner_vlan = ac_port1_inner_vlan;
    vswitch_vpls_info_1.ac_port2_outer_vlan = ac_port2_outer_vlan;
    vswitch_vpls_info_1.ac_port2_inner_vlan = ac_port2_inner_vlan;

    vswitch_vpls_info_1.access_index_1 = 2;
    vswitch_vpls_info_1.access_index_2 = 3;
    vswitch_vpls_info_1.access_index_3 = 3;
    if (extend_example) {
        /* init the 2 other ports */

        /* incomming packet attributes */
        /* access port */
        vswitch_vpls_info_2.ac_in_port = ac_port;
        /* Core port */
        vswitch_vpls_info_2.pwe_in_port = pwe_in_port;
        vswitch_vpls_info_2.pwe_out_port = pwe_out_port;
        vswitch_vpls_info_2.in_vc_label = 2009;
        vswitch_vpls_info_2.eg_vc_label = 1981;
        /* tunnel info */
        vswitch_vpls_info_2.in_tunnel_label = 400;
        vswitch_vpls_info_2.out_tunnel_label = 300;

        vswitch_vpls_info_2.access_index_1 = 2;
        vswitch_vpls_info_2.access_index_2 = 3;
        vswitch_vpls_info_2.access_index_3 = 3; 
    }
    printf("vswitch_vpls_info_init %d\n", vswitch_vpls_info_1.vpn_id);

    egress_mc = encap_optimized;

    return rv;
}

/* initialize the tunnels for mpls routing
 * set termination, for MPLS label 1000,1002
 */
int
mpls_tunnels_config(int unit, int second_unit, int extend_example){
    int CINT_NO_FLAGS = 0;
    int ingress_intf;
    int egress_intf;
    int encap_id;
    bcm_mpls_egress_label_t label_array[2];
    bcm_pbmp_t pbmp;
    /*bcm_mpls_egress_label_t label_array_2[2];*/
    int rv;
    int flags;
    bcm_mpls_label_t label_for_pop_entry_in, label_for_pop_entry_out;
    create_l3_intf_s intf;
    
    /* set mpls tunneling information with default values */
    mpls_lsr_init(vswitch_vpls_info_1.pwe_in_port,vswitch_vpls_info_1.pwe_out_port,0x11,0x22,5000,8000,100,200,0);
    mpls_lsr_info_1.eg_port = vswitch_vpls_info_1.pwe_out_port;
    
    /* open vlan */
    printf("open vlan %d\n", mpls_lsr_info_1.in_vid);
    rv = bcm_vlan_create(unit,mpls_lsr_info_1.in_vid);
    print rv;
    
    /* add vlan to pwe_port */
    BCM_PBMP_CLEAR(pbmp);
    BCM_PBMP_PORT_ADD(pbmp, mpls_lsr_info_1.eg_port);
    
    rv = bcm_vlan_port_add(unit, mpls_lsr_info_1.in_vid, pbmp, pbmp);
    if (rv != BCM_E_NONE) {
        printf("Error, in bcm_vlan_port_add, vlan=%d, \n", mpls_lsr_info_1.in_vid);
        return rv;
    }
    
    if (second_unit >= 0)
    {
        printf("open vlan %d\n", mpls_lsr_info_1.in_vid);
        rv = bcm_vlan_create(second_unit,mpls_lsr_info_1.in_vid);
        print rv;
        
        BCM_PBMP_CLEAR(pbmp);
        BCM_PBMP_PORT_ADD(pbmp, mpls_lsr_info_1.eg_port);
        
        rv = bcm_vlan_port_add(unit, mpls_lsr_info_1.in_vid, pbmp, pbmp);
        if (rv != BCM_E_NONE) {
            printf("Error, in bcm_vlan_port_add, vlan=%d, \n", mpls_lsr_info_1.in_vid);
            return rv;
        }
    }
    
    /* l2 termination for mpls routing: packet received on those VID MAC will be L2 terminated  */
    intf.vsi = mpls_lsr_info_1.in_vid;
    intf.my_global_mac = mpls_lsr_info_1.my_mac;
    intf.my_lsb_mac = mpls_lsr_info_1.my_mac;
    
    rv = l3__intf_rif__create(unit, &intf);
    ingress_intf = intf.rif;
    if (rv != BCM_E_NONE) {
    	printf("Error, in l3__intf_rif__create\n");
    	return rv;
    }
    
    if (second_unit >= 0)
    {
        rv = l3__intf_rif__create(second_unit, &intf);
    	ingress_intf = intf.rif;
    	if (rv != BCM_E_NONE) {
    		printf("Error, in l3__intf_rif__create\n");
    		return rv;
    	}
    }
    
    /*  create ingress object, packet will be routed to */
    intf.vsi = mpls_lsr_info_1.eg_vid;
    
    rv = l3__intf_rif__create(unit, &intf);
    vswitch_vpls_info_1.rif = ingress_intf = intf.rif;
    if (rv != BCM_E_NONE) {
    	printf("Error, in l3__intf_rif__create\n");
    	return rv;
    }
    
    if (second_unit >= 0)
    {
        rv = l3__intf_rif__create(second_unit, &intf);
    	ingress_intf = intf.rif;
    	if (rv != BCM_E_NONE) {
    		printf("Error, in l3__intf_rif__create\n");
    		return rv;
    	}
    }
    
    if (!pwe_php) {
        /* set tunnel over this l3 interface, so packet forwarded to this interface will be tunneled */
        bcm_mpls_egress_label_t_init(&label_array[0]);
        bcm_mpls_egress_label_t_init(&label_array[1]);
        label_array[0].exp = 2;
        label_array[0].flags = (BCM_MPLS_EGRESS_LABEL_TTL_SET | BCM_MPLS_EGRESS_LABEL_TTL_DECREMENT);
        if (!is_device_or_above(unit, ARAD_PLUS) || mpls_pipe_mode_exp_set) {
            label_array[0].flags |= BCM_MPLS_EGRESS_LABEL_EXP_SET;
        } else {
            label_array[0].flags |= BCM_MPLS_EGRESS_LABEL_EXP_COPY;
        }
        label_array[0].label = vswitch_vpls_info_1.in_tunnel_label;
        label_array[0].ttl = 20;

        label_array[1].exp = 4;
        label_array[1].flags = (BCM_MPLS_EGRESS_LABEL_TTL_SET | BCM_MPLS_EGRESS_LABEL_TTL_DECREMENT);
        if (!is_device_or_above(unit, ARAD_PLUS) || mpls_pipe_mode_exp_set) {
            label_array[1].flags |= BCM_MPLS_EGRESS_LABEL_EXP_SET;
        } else {
            label_array[1].flags |= BCM_MPLS_EGRESS_LABEL_EXP_COPY;
        }
        label_array[1].label = vswitch_vpls_info_1.out_tunnel_label;
        label_array[1].ttl = 40;
        label_array[1].l3_intf_id = ingress_intf;

        rv = bcm_mpls_tunnel_initiator_create(unit, 0, 2, label_array);
        if (rv != BCM_E_NONE) {
            printf("Error, in bcm_mpls_tunnel_initiator_create\n");
            return rv;
        }
        ingress_intf = label_array[0].tunnel_id;
        vswitch_vpls_info_1.tunnel_id = ingress_intf;

        mpls_tunnel_encap_id1 = ingress_intf = label_array[0].tunnel_id;

        if (second_unit >= 0) {
            rv = bcm_mpls_tunnel_initiator_create(second_unit, 0, 2, label_array);
            if (rv != BCM_E_NONE) {
                printf("Error, in bcm_mpls_tunnel_initiator_create\n");
                return rv;
            }
        }
    }

    flags = encap_optimized ? BCM_L3_ENCAP_SPACE_OPTIMIZED : CINT_NO_FLAGS;
    /* create egress object points to this tunnel/interface */
    if (second_unit >= 0)
    {
        egress_intf = 0;
        encap_id = 0;
        create_l3_egress_s l3eg;
        l3eg.out_tunnel_or_rif = ingress_intf;
        sal_memcpy(l3eg.next_hop_mac_addr, mpls_lsr_info_1.next_hop_mac, 6);
        l3eg.vlan   = mpls_lsr_info_1.eg_vid;
        l3eg.arp_encap_id = encap_id;
        l3eg.fec_id = egress_intf; 
        l3eg.l3_flags = flags;
        l3eg.out_gport = mpls_lsr_info_1.eg_port;

        rv = l3__egress__create(unit,&l3eg);
        if (rv != BCM_E_NONE) {
            printf("Error, in bcm_vlan_create with vlan %d continue \n", l3eg.vlan);
            print rv;
        }
        l3eg.allocation_flags = BCM_L3_WITH_ID|flags;

        rv = l3__egress__create(second_unit,&l3eg);
        if (rv != BCM_E_NONE) {
            printf("Error, in bcm_vlan_create with vlan %d continue \n", l3eg.vlan);
            print rv;
        }
        encap_id = l3eg.arp_encap_id;
        egress_intf = l3eg.fec_id;

    }
    else
    {
        create_l3_egress_s l3eg;
        egress_intf = 0;
        encap_id = 0;
        l3eg.out_tunnel_or_rif = pwe_php ? 0 : ingress_intf;
        sal_memcpy(l3eg.next_hop_mac_addr, mpls_lsr_info_1.next_hop_mac, 6);
        l3eg.vlan   = mpls_lsr_info_1.eg_vid;
        l3eg.arp_encap_id = encap_id;
        l3eg.fec_id = egress_intf; 
        l3eg.l3_flags = flags;
        l3eg.out_gport = mpls_lsr_info_1.eg_port;

        rv = l3__egress__create(unit,&l3eg);
        if (rv != BCM_E_NONE) {
            printf("Error, in bcm_vlan_create with vlan %d continue \n", l3eg.vlan);
            print rv;
        }
        encap_id = l3eg.arp_encap_id;
        egress_intf = l3eg.fec_id;
    }  
    mpls_lsr_info_1.encap_id = encap_id;
    if (!pwe_php) {
        
        mpls_termination_label_index_enable = soc_property_get(unit , "mpls_termination_label_index_enable",0);

        /* add switch entries to pop MPLS labels  */
        if (mpls_termination_label_index_enable) {
            BCM_MPLS_INDEXED_LABEL_SET(&label_for_pop_entry_out, vswitch_vpls_info_1.out_tunnel_label, vswitch_vpls_info_1.access_index_1);
            BCM_MPLS_INDEXED_LABEL_SET(&label_for_pop_entry_in, vswitch_vpls_info_1.in_tunnel_label, 1);
        }
        else {
            label_for_pop_entry_out = vswitch_vpls_info_1.out_tunnel_label;
            label_for_pop_entry_in = vswitch_vpls_info_1.in_tunnel_label;
        }


        rv = mpls_add_pop_entry(unit, label_for_pop_entry_out);
        if (rv != BCM_E_NONE) {
            printf("Error, in mpls_add_pop_entry\n");
            return rv;
        }
        if (second_unit >= 0)
        {
            rv = mpls_add_pop_entry(second_unit,label_for_pop_entry_out);
            if (rv != BCM_E_NONE) {
                printf("Error, in mpls_add_pop_entry on second unit\n");
                return rv;
            }
        }
        
        rv = mpls_add_pop_entry(unit,label_for_pop_entry_in);
        if (rv != BCM_E_NONE) {
            printf("Error, in mpls_add_pop_entry\n");
            return rv;
        }
        if (second_unit >= 0)
        {
            rv = mpls_add_pop_entry(second_unit,label_for_pop_entry_in);
            if (rv != BCM_E_NONE) {
                printf("Error, in mpls_add_pop_entry\n");
                return rv;
            }
        }
    }
    
    vswitch_vpls_shared_info_1.egress_intf = egress_intf;
    
    /* if extend example do the same for the second tunnel */
    if (extend_example) {
        if (mpls_termination_label_index_enable) {
            BCM_MPLS_INDEXED_LABEL_SET(&label_for_pop_entry_out, vswitch_vpls_info_2.out_tunnel_label, vswitch_vpls_info_2.access_index_1);
            BCM_MPLS_INDEXED_LABEL_SET(&label_for_pop_entry_in, vswitch_vpls_info_2.in_tunnel_label, 1);
        }
        else {
            label_for_pop_entry_out = vswitch_vpls_info_2.out_tunnel_label;
            label_for_pop_entry_in = vswitch_vpls_info_2.in_tunnel_label;
        }

        rv = mpls_add_pop_entry(unit,label_for_pop_entry_out);
        if (rv != BCM_E_NONE) {
            printf("Error, in mpls_add_pop_entry2\n");
            return rv;
        }
        if (second_unit >= 0)
        {
            rv = mpls_add_pop_entry(second_unit,label_for_pop_entry_out);
            if (rv != BCM_E_NONE) {
                printf("Error, in mpls_add_pop_entry2 second unit\n");
                return rv;
            }
        }
        
        rv = mpls_add_pop_entry(unit,label_for_pop_entry_in);
        if (rv != BCM_E_NONE) {
            printf("Error, in mpls_add_pop_entry2\n");
            return rv;
        }
        if (second_unit >= 0)
        {
            rv = mpls_add_pop_entry(second_unit,label_for_pop_entry_in);
            if (rv != BCM_E_NONE) {
                printf("Error, in mpls_add_pop_entry2\n");
                return rv;
            }
        }
        
        /*  create ingress object, packet will be routed to */
	    intf.vsi = mpls_lsr_info_1.in_vid;
	    intf.my_global_mac = mpls_lsr_info_1.my_mac;
	    intf.my_lsb_mac = mpls_lsr_info_1.my_mac;
	    
	    rv = l3__intf_rif__create(unit, &intf);
	    ingress_intf = intf.rif;
	    if (rv != BCM_E_NONE) {
	    	printf("Error, in l3__intf_rif__create\n");
	    	return rv;
	    }
        if (second_unit >= 0)
        {
            rv = l3__intf_rif__create(second_unit, &intf);
	    	ingress_intf = intf.rif;
	    	if (rv != BCM_E_NONE) {
	    		printf("Error, in l3__intf_rif__create\n");
	    		return rv;
	    	}
        }
        
        
        /* set tunnel over this l3 interface, so packet forwarded to this interface will be tunneled */
        bcm_mpls_egress_label_t_init(&label_array[0]);
        bcm_mpls_egress_label_t_init(&label_array[1]);
        label_array[0].exp = 2; 
        label_array[0].flags = (BCM_MPLS_EGRESS_LABEL_TTL_SET|BCM_MPLS_EGRESS_LABEL_TTL_DECREMENT);  
        if (!is_device_or_above(unit,ARAD_PLUS) || mpls_pipe_mode_exp_set) {
            label_array[0].flags |= BCM_MPLS_EGRESS_LABEL_EXP_SET;
        } else {
            label_array[0].flags |= BCM_MPLS_EGRESS_LABEL_EXP_COPY;
        }
        label_array[0].label = vswitch_vpls_info_2.in_tunnel_label;
        label_array[0].ttl = 20;
        
        label_array[1].exp = 4; 
        label_array[1].flags = (BCM_MPLS_EGRESS_LABEL_TTL_SET|BCM_MPLS_EGRESS_LABEL_TTL_DECREMENT);  
        if (!is_device_or_above(unit,ARAD_PLUS) || mpls_pipe_mode_exp_set) {
            label_array[1].flags |= BCM_MPLS_EGRESS_LABEL_EXP_SET;
        } else {
            label_array[1].flags |= BCM_MPLS_EGRESS_LABEL_EXP_COPY;
        }
        label_array[1].label = vswitch_vpls_info_2.out_tunnel_label;
        label_array[1].ttl = 40;
        label_array[1].l3_intf_id = ingress_intf;
        
        rv = bcm_mpls_tunnel_initiator_create(unit,0,2,label_array);
        if (rv != BCM_E_NONE) {
            printf("Error, in bcm_mpls_tunnel_initiator_create\n");
            return rv;
        }
        second_mpls_intf = label_array[0].tunnel_id;
        vswitch_vpls_info_2.tunnel_id = second_mpls_intf;
        if (second_unit >= 0)
        {
            rv = bcm_mpls_tunnel_initiator_create(second_unit,0,2,label_array);
            if (rv != BCM_E_NONE) {
                printf("Error, in bcm_mpls_tunnel_initiator_create\n");
                return rv;
            }
        }
        
        /* create egress object points to this tunne/interface */
        mpls_tunnel_encap_id2 = ingress_intf = label_array[0].tunnel_id;
        flags = encap_optimized ? BCM_L3_ENCAP_SPACE_OPTIMIZED : CINT_NO_FLAGS;
        if (second_unit >= 0)
        {
            create_l3_egress_s l3eg;
            egress_intf = 0;
            encap_id = 0;
            l3eg.out_tunnel_or_rif = ingress_intf;
            sal_memcpy(l3eg.next_hop_mac_addr, mpls_lsr_info_1.next_hop_mac, 6);
            l3eg.vlan   = mpls_lsr_info_1.eg_vid;
            l3eg.arp_encap_id = encap_id;
            l3eg.fec_id = egress_intf; 
            l3eg.l3_flags = flags;
            l3eg.out_gport = mpls_lsr_info_1.eg_port;

            rv = l3__egress__create(unit,&l3eg);
            if (rv != BCM_E_NONE) {
                printf("Error, in bcm_vlan_create with vlan %d continue \n", l3eg.vlan);
                print rv;
            }
            l3eg.allocation_flags = BCM_L3_WITH_ID|flags;

            rv = l3__egress__create(second_unit,&l3eg);
            if (rv != BCM_E_NONE) {
                printf("Error, in bcm_vlan_create with vlan %d continue \n", l3eg.vlan);
                print rv;
            }
            encap_id = l3eg.arp_encap_id;
            egress_intf = l3eg.fec_id;			
        } 
        else
        {
            create_l3_egress_s l3eg;
            egress_intf = 0;
            encap_id = 0;
            l3eg.out_tunnel_or_rif = ingress_intf;
            sal_memcpy(l3eg.next_hop_mac_addr, mpls_lsr_info_1.next_hop_mac, 6);
            l3eg.vlan   = mpls_lsr_info_1.eg_vid;
            l3eg.arp_encap_id = encap_id;
            l3eg.fec_id = egress_intf; 
            l3eg.l3_flags = flags;
            l3eg.out_gport = mpls_lsr_info_1.eg_port;
            
            rv = l3__egress__create(unit,&l3eg);
            if (rv != BCM_E_NONE) {
                printf("Error, in bcm_vlan_create with vlan %d continue \n", l3eg.vlan);
                print rv;
            }
            encap_id = l3eg.arp_encap_id;
            egress_intf = l3eg.fec_id;
        }     
        vswitch_vpls_shared_info_1.egress_intf2 = egress_intf;
    }
    
    return rv;
}



/* add switch entry to perform pop
 */
int
mpls_add_pop_entry(int unit, int in_label)
{
    int rv;
    bcm_mpls_tunnel_switch_t entry;
    
    bcm_mpls_tunnel_switch_t_init(&entry);
    entry.action = BCM_MPLS_SWITCH_ACTION_POP;
    /* TTL decrement has to be present */
    entry.flags = BCM_MPLS_SWITCH_TTL_DECREMENT;
    /* Uniform: inherit TTL and EXP, 
     * in general valid options: 
     *    both present (uniform) or none of them (Pipe)
     */
    entry.flags |= BCM_MPLS_SWITCH_OUTER_TTL|BCM_MPLS_SWITCH_OUTER_EXP;
    
    /* incoming label */
    entry.label = in_label;
    
    /* Enable when testing egress QOS, need to source cint_qos.c
     * This remarks the mpls egress packet
     */
    
    entry.qos_map_id = qos_map_id_mpls_ingress_get(unit);

    /* egress attributes */
    /* none as it just pop */
    
    rv = bcm_mpls_tunnel_switch_create(unit,&entry);
    if (rv != BCM_E_NONE) {
        printf("Error, in bcm_mpls_tunnel_switch_create\n");
        return rv;
    }

	vswitch_vpls_pairs.id1 = entry.tunnel_id ;
	vswitch_vpls_pairs.label1 = entry.label ;

    return rv;
}

/*
*   Fills the apropriate utility stucture with the common parameters
*   port_num_call - indicates which port call is calling the function
*/
void 
vswitch_vpls_common_port_properties_init(int unit, mpls_port__ingress_only_info_s *ingress_port, mpls_port__egress_only_info_s *egress_port, mpls_port_utils_s *default_port, int port_num_call){

    mpls_termination_label_index_enable = soc_property_get(unit , "mpls_termination_label_index_enable",0);
    int vc_label = make_before_break ? vswitch_vpls_info_1.in_vc_label : vswitch_vpls_info_2.in_vc_label;
    int egress_vc_label = port_num_call ? vswitch_vpls_info_2.eg_vc_label : vswitch_vpls_info_1.eg_vc_label;
    int egress_label_inheritance_flags = BCM_MPLS_EGRESS_LABEL_TTL_COPY | BCM_MPLS_EGRESS_LABEL_EXP_COPY;

    if(ingress_egress_separate_ports) {
         /* Setting network group according to pwe_network_group_id_1(2) */
        if (pwe_network_group_id_1&& !port_num_call) {
            ingress_port->network_group = pwe_network_group_id_1;
            egress_port->network_group = pwe_network_group_id_1;
        }
        else if (pwe_network_group_id_2 && port_num_call) {
            ingress_port->network_group = pwe_network_group_id_2;
            egress_port->network_group = pwe_network_group_id_2;
        }

        ingress_port->flags |= pwe_cw ? BCM_MPLS_PORT_CONTROL_WORD : 0;
        egress_port->flags |= pwe_cw ? BCM_MPLS_PORT_CONTROL_WORD : 0;

        if (port_based_pwe_enable) {
            ingress_port->ingress_matching_criteria = BCM_MPLS_PORT_MATCH_LABEL_PORT;
            BCM_GPORT_LOCAL_SET(ingress_port->port,port_num_call ? vswitch_vpls_info_2.pwe_in_port : vswitch_vpls_info_1.pwe_in_port);
        }
        else {
            ingress_port->ingress_matching_criteria = BCM_MPLS_PORT_MATCH_LABEL;
            ingress_port->port = port_num_call ? vswitch_vpls_info_2.pwe_in_port : vswitch_vpls_info_1.pwe_in_port;
        }

        ingress_port->vpn = vswitch_vpls_shared_info_1.vpn;
        if(port_num_call) {    
            if (mpls_termination_label_index_enable) {
                BCM_MPLS_INDEXED_LABEL_SET(ingress_port->ingress_pwe_label, vc_label, vswitch_vpls_info_2.access_index_3);
            }
            else {
                ingress_port->ingress_pwe_label = vc_label;
            }       
        }
        /* egress label: In ingress only used for learning
         *               In egress only used for pwe encpasulation information in EEDB multicast case
         */
        ingress_port->learn_egress_label.label = egress_vc_label;
        ingress_port->learn_egress_label.ttl = 10;
        if (test_default_ttl_inheritance) {
            ingress_port->learn_egress_label.flags = egress_label_inheritance_flags;
        } else {
            ingress_port->learn_egress_label.flags = BCM_MPLS_EGRESS_LABEL_TTL_SET;
        }

        egress_port->egress_label.label = egress_vc_label;
        egress_port->egress_label.ttl = 10;
        if (test_default_ttl_inheritance) {
            egress_port->egress_label.flags = egress_label_inheritance_flags;
        } else {
            egress_port->egress_label.flags = BCM_MPLS_EGRESS_LABEL_TTL_SET;  
        }


        egress_port->vccv_type =  vccv_type3_ttl;


    } else {

        if (pwe_network_group_id_1&& !port_num_call) {
            default_port->network_group = pwe_network_group_id_1;
        }
        else if (pwe_network_group_id_2 && port_num_call) {
            default_port->network_group = pwe_network_group_id_2;
        }

        default_port->flags |= pwe_cw ? BCM_MPLS_PORT_CONTROL_WORD : 0;
        if (port_based_pwe_enable) {
            default_port->ingress_matching_criteria = BCM_MPLS_PORT_MATCH_LABEL_PORT;
            BCM_GPORT_LOCAL_SET(default_port->port,port_num_call ? vswitch_vpls_info_2.pwe_in_port : vswitch_vpls_info_1.pwe_in_port);
        }
        else {
            default_port->ingress_matching_criteria = BCM_MPLS_PORT_MATCH_LABEL;
            default_port->port = port_num_call ? vswitch_vpls_info_2.pwe_in_port : vswitch_vpls_info_1.pwe_in_port;
        }
        default_port->vpn = vswitch_vpls_shared_info_1.vpn;
        if(port_num_call) {    
            if (mpls_termination_label_index_enable) {
                BCM_MPLS_INDEXED_LABEL_SET(default_port->ingress_pwe_label, vc_label, vswitch_vpls_info_2.access_index_3);
            }
            else {
                default_port->ingress_pwe_label = vc_label;
            }       
        }
         /* egress label: In ingress only used for learning
         *               In egress only used for pwe encpasulation information in EEDB multicast case
         */
        default_port->egress_pwe_label = egress_vc_label;
        default_port->ttl = 10;
        default_port->egress_label_flags =test_default_ttl_inheritance ? egress_label_inheritance_flags : 0; 

        default_port->vccv_type = vccv_type3_ttl;
        
    }

    

}

/*
*   Creates an ingress and egress PWE ports with parameters from the init function
*   port_num_call - indicates which port call is calling the function
*/
int
vswitch_vpls_ingress_egress_separate_ports_create(int unit_or_second, mpls_port__ingress_only_info_s *ingress_port, mpls_port__egress_only_info_s *egress_port, int port_num_call ){
        
    int rv;
    /* Ingress_only */     
        ingress_port->learn_egress_if = port_num_call ? vswitch_vpls_shared_info_1.egress_intf2 : vswitch_vpls_shared_info_1.egress_intf;
        if(!port_num_call) {
            if (mpls_termination_label_index_enable) {
                BCM_MPLS_INDEXED_LABEL_SET(ingress_port->ingress_pwe_label, vswitch_vpls_info_1.in_vc_label, vswitch_vpls_info_1.access_index_2);
            }
            else {
                ingress_port->ingress_pwe_label = vswitch_vpls_info_1.in_vc_label;
            }
        }
        /* WITH_ID flags is set in the utility, in case of ingress-egress configuration */
        ingress_port->mpls_port_id = port_num_call ? 0x18805002 : 0x18805000;
        ingress_port->encap_id = port_num_call ? 0x5002 : 0x5000;

        ingress_port->flags2 |= encap_optimized ? BCM_MPLS_PORT2_LEARN_ENCAP : 0;

        rv = mpls_port__ingress_only_create(unit_or_second, ingress_port);
        if (rv != BCM_E_NONE) {
            printf("Error, mpls_port__ingress_only_create \n");
            return rv;
        }

        if(verbose1){
            bcm_gport_t m_port_id = ingress_port->mpls_port_id;
            bcm_if_t enc_id = ingress_port->encap_id;
            printf("INGRESS: add port 0x%08x to vpn \n\r",m_port_id);
            printf("ENCAP ID: %d \n\r",enc_id);
        }
        if(!port_num_call) {
            mpls_lsr_info_1.mpls_port_id = ingress_port->mpls_port_id;
        }
        
        /* Egress_only */
        egress_port->mpls_port_id = ingress_port->mpls_port_id;
        egress_port->encap_id = ingress_port->encap_id;
        egress_port->egress_tunnel_if = vswitch_vpls_info_1.tunnel_id;
        egress_port->encap_optimized = encap_optimized;
        rv = mpls_port__egress_only_create(unit_or_second, egress_port);
        if (rv != BCM_E_NONE) {
            printf("Error, mpls_port__egress_only_create \n");
            return rv;
        }
        vswitch_vpls_info_1.encap_id = egress_port->encap_id;
        vswitch_vpls_info_1.mpls_port_id = egress_port->mpls_port_id;
        return rv;
}

/*
*   Creates a normal PWE port with parameters from the init function
*   port_num_call - indicates which port call is calling the function
*/
int
vswitch_vpls_default_port_create(int unit_or_second, mpls_port_utils_s *default_mpls_port, int port_num_call){

    int rv;
    mpls_termination_label_index_enable = soc_property_get(unit_or_second , "mpls_termination_label_index_enable",0);
    if(!port_num_call) {
        if (mpls_termination_label_index_enable) {
            BCM_MPLS_INDEXED_LABEL_SET(default_mpls_port->ingress_pwe_label, vswitch_vpls_info_1.in_vc_label, vswitch_vpls_info_1.access_index_3);
        }
        else {
            default_mpls_port->ingress_pwe_label = vswitch_vpls_info_1.in_vc_label;
        }
    }
 
    default_mpls_port->flags2 |= encap_optimized ? BCM_MPLS_PORT2_LEARN_ENCAP : 0;

    default_mpls_port->egress_mpls_tunnel_id = port_num_call ? vswitch_vpls_shared_info_1.egress_intf2 : vswitch_vpls_shared_info_1.egress_intf;
    rv = mpls_port__mp_create_ingress_egress_mpls_port(unit_or_second, default_mpls_port);
    if (rv != BCM_E_NONE) {
        printf("Error, mpls_port__mp_create_ingress_egress_mpls_port \n");
        return rv;
    }
            
    return rv; 

}


/*
 * create mpls-port
 *   vlan-port: is Logical interface identified by (port-vlan-vlan). 
 */
int
vswitch_vpls_add_network_port_1(int unit, int second_unit,  bcm_gport_t *port_id){
    int rv;
    bcm_mpls_port_t mpls_port_1;
    mpls_port__ingress_only_info_s ingress_port;
    mpls_port__egress_only_info_s egress_port;
    mpls_port_utils_s default_mpls_port;
    int unit_or_second = unit;
    int is_two_dev = 0;
    int port;
    bcm_gport_t egress_mpls_port_id;
    bcm_if_t last_encap_id;

    if (second_unit >= 0)
    {
        unit_or_second = second_unit;
        is_two_dev = 1;    
    }   
    
    /* Initialize the common properties of ingress/egress/joined PWE ports */
    vswitch_vpls_common_port_properties_init(unit_or_second, &ingress_port, &egress_port, &default_mpls_port, 0/*Which port calls the funcion*/);
    
    if (ingress_egress_separate_ports) {

        vswitch_vpls_ingress_egress_separate_ports_create(unit_or_second,&ingress_port,&egress_port, 0/*Which port calls the funcion*/);

        /* handle of the created gport */
        *port_id = ingress_port.mpls_port_id;

        printf("EGRESS: add port 0x%08x to vpn \n\r",egress_port.mpls_port_id);
        egress_mpls_port_id = egress_port.mpls_port_id;
        exported_egress_mpls_port_id = egress_port.mpls_port_id;
        
    } else {

        vswitch_vpls_default_port_create(unit_or_second,&default_mpls_port,0/*Which port calls the funcion*/);
              
        if(verbose1){
            printf("add port 0x%08x to vpn \n\r",default_mpls_port.mpls_port_id);
        }
        vswitch_vpls_pairs.id2 =  default_mpls_port.mpls_port_id ;
        vswitch_vpls_pairs.label2 = default_mpls_port.ingress_pwe_label ;

        mpls_lsr_info_1.mpls_port_id = default_mpls_port.mpls_port_id;

        /* handle of the created gport */
        *port_id = egress_mpls_port_id = default_mpls_port.mpls_port_id;
    }

    /* GAL support for BFDoGACHoGALoPWE encapsulation */
    if (is_gal) {
        ingress_port.gal_primary_tunnel_id = ingress_egress_separate_ports ? ingress_port.mpls_port_id : default_mpls_port.mpls_port_id;
        ingress_port.mpls_port_id = ingress_egress_separate_ports ? ingress_port.mpls_port_id +10 : default_mpls_port.mpls_port_id + 10; /* Should be different lif ID */
        ingress_port.flags |= BCM_MPLS_PORT_WITH_GAL | BCM_MPLS_PORT_CONTROL_WORD;
        ingress_port.vpn = vswitch_vpls_shared_info_1.vpn;

        rv = mpls_port__ingress_only_create(unit_or_second, &ingress_port);
        if (rv != BCM_E_NONE) {
            printf("Error,GAL mpls_port__ingress_only_create \n");
            return rv;
        }
		vswitch_vpls_pairs.id3 =  ingress_port.mpls_port_id ;
		vswitch_vpls_pairs.label3 = ingress_port.ingress_pwe_label ;
                last_encap_id = ingress_port.encap_id;
    }
    else
    {
        last_encap_id = ingress_egress_separate_ports ? egress_port.encap_id : default_mpls_port.encap_id;
    }

    
    BCM_L3_ITF_SET(pwe_encap_id, BCM_L3_ITF_TYPE_LIF, last_encap_id);

    if (is_two_dev)
    {
        mpls_port_1.flags |= BCM_MPLS_PORT_WITH_ID;
        rv = bcm_mpls_port_add(unit, vswitch_vpls_shared_info_1.vpn, &mpls_port_1);
        if (rv != BCM_E_NONE) {
            printf("Error, bcm_mpls_port_add two devices\n");
            return rv;
        }
    }

    mpls_port_encap_id1 = last_encap_id;
    if(verbose1){
        printf("mpls_port_encap_id1: 0x%08x\n\r",mpls_port_encap_id1);
    }
    
    /* update Multicast to have the added port  */
    port = vswitch_vpls_info_1.pwe_in_port;
    /* Adding egress mpls port to multicast because when egress and ingress are configured separately the multicast should point to the outlif */
    rv = multicast__mpls_port_add(unit_or_second, vswitch_vpls_shared_info_1.vpn, port, egress_mpls_port_id, encap_optimized);
    if (rv != BCM_E_NONE) {
        printf("Error, multicast__mpls_port_add\n");
        return rv;
    }
    if (is_two_dev)
    {
        rv = multicast__mpls_port_add(unit, vswitch_vpls_shared_info_1.vpn, port, egress_mpls_port_id, egress_mc);
        if (rv != BCM_E_NONE) {
            printf("Error, multicast__mpls_port_add\n");
            return rv;
        }
    }
    if(verbose1){
        printf("add port   0x%08x to multicast \n\r",egress_mpls_port_id);
    }
    
    return rv;
}

int
vswitch_vpls_add_network_port_2(int unit,  bcm_gport_t *port_id){
    int rv;
    bcm_mpls_port_t mpls_port_1;
    mpls_port__ingress_only_info_s ingress_port;
    mpls_port__egress_only_info_s egress_port;
    mpls_port_utils_s default_mpls_port;
    bcm_gport_t egress_mpls_port_id;
    int vc_label;
    
    vswitch_vpls_common_port_properties_init(unit, &ingress_port, &egress_port, &default_mpls_port, 1/*Which port calls the funcion*/);        
    
    if (ingress_egress_separate_ports) {


        vswitch_vpls_ingress_egress_separate_ports_create(unit,&ingress_port,&egress_port, 1/*Which port calls the funcion*/);

        /* handle of the created gport */
        *port_id = ingress_port.mpls_port_id;
        
        printf("EGRESS: add port 0x%08x to vpn \n\r",egress_port.mpls_port_id);
        egress_mpls_port_id = egress_port.mpls_port_id;
    } else {
        vswitch_vpls_default_port_create(unit,&default_mpls_port,1/*Which port calls the funcion*/);

        if(verbose1){
            printf("add port 0x%08x to vpn \n\r",default_mpls_port.mpls_port_id);
        }

        /* handle of the created gport */
        *port_id = egress_mpls_port_id = default_mpls_port.mpls_port_id;
    }
    
    mpls_port_encap_id2 = ingress_egress_separate_ports ? egress_port.encap_id : default_mpls_port.encap_id;
    if(verbose1){
        printf("mpls_port_encap_id2: 0x%08x\n\r",mpls_port_encap_id2);
    }

    /* update Multicast to have the added port  */
    rv = multicast__mpls_port_add(unit, vswitch_vpls_shared_info_1.vpn, vswitch_vpls_info_2.pwe_in_port , egress_mpls_port_id, egress_mc);
    if (rv != BCM_E_NONE) {
        printf("Error, multicast__mpls_port_add\n");
        return rv;
    }
    if(verbose1){
        printf("add port   0x%08x to multicast \n\r",egress_mpls_port_id);
    }
    
    return rv;
}

/*
 * create vlan-port
 *   vlan-port: is Logical interface identified by (port-vlan-vlan). 
 */
int
vswitch_vpls_add_access_port_1(int unit, int second_unit,  bcm_gport_t *port_id){
    int rv;  
    bcm_vlan_port_t vlan_port_1;
    int unit_or_second = unit;
    int is_two_dev = 0;
    int port;
    
    port = vswitch_vpls_info_1.ac_in_port;
    
    if (second_unit >= 0)
    {
        unit_or_second = second_unit;
        is_two_dev = 1;
    }
    /* add port, according to port_vlan_vlan */
    bcm_vlan_port_t_init(&vlan_port_1);

    /* Setting network group according to ac_network_group_id_1 */
    if (is_device_or_above(unit,JERICHO) && (soc_property_get(unit , "split_horizon_forwarding_groups_mode",1)==2)) {
        vlan_port_1.ingress_network_group_id = ac_network_group_id_ingress_1;
        vlan_port_1.egress_network_group_id = ac_network_group_id_egress_1;
    }
    
    /* set port attribures, key <port-vlan-vlan>*/
    vlan_port_1.criteria = BCM_VLAN_PORT_MATCH_PORT_VLAN_STACKED;
    vlan_port_1.port = vswitch_vpls_info_1.ac_in_port;
    vlan_port_1.match_vlan = vswitch_vpls_info_1.ac_port1_outer_vlan;
    vlan_port_1.match_inner_vlan = vswitch_vpls_info_1.ac_port1_inner_vlan; 
    vlan_port_1.egress_vlan = vswitch_vpls_info_1.ac_port1_outer_vlan;
    vlan_port_1.egress_inner_vlan = vswitch_vpls_info_1.ac_port1_inner_vlan;
    vlan_port_1.flags = vswitch_vpls_info_1.ac_port1_flags;
    rv = bcm_vlan_port_create(unit, &vlan_port_1);
    if (rv != BCM_E_NONE) {
        printf("Error, bcm_vlan_port_create\n");
        return rv;
    }

    *port_id = vlan_port_1.vlan_port_id;

    vlan_port_encap_id1 = vlan_port_1.encap_id;
    if(verbose1){
        printf("vlan_port_encap_id1: 0x%08x\n\r",vlan_port_encap_id1);
    }


    /* In advanced vlan translation mode, bcm_vlan_port_create does not create ingress / egress
       action mapping. This is here to compensate. */
    if (advanced_vlan_translation_mode) {
        rv = vlan_translation_vlan_port_create_to_translation(unit, &vlan_port_1);
        if (rv != BCM_E_NONE) {
            printf("Error: vlan_translation_vlan_port_create_to_translation\n");
        }
    }


    rv = bcm_vswitch_port_add(unit, vswitch_vpls_shared_info_1.vpn, vlan_port_1.vlan_port_id);
    if (rv != BCM_E_NONE) {
        printf("Error, bcm_vswitch_port_add\n");
        return rv;
    }

    if (is_two_dev)
    {
        vlan_port_1.flags |= BCM_VLAN_PORT_WITH_ID;
        rv = bcm_vlan_port_create(second_unit, &vlan_port_1);
        if (rv != BCM_E_NONE) {
            printf("Error, bcm_vlan_port_add\n");
            return rv;
        }

        /* In advanced vlan translation mode, bcm_vlan_port_create does not create ingress / egress
           action mapping. This is here to compensate. */
        if (advanced_vlan_translation_mode) {
            rv = vlan_translation_vlan_port_create_to_translation(second_unit, &vlan_port_1);
            if (rv != BCM_E_NONE) {
                printf("Error: vlan_translation_vlan_port_create_to_translation\n");
            }
        }


        rv = bcm_vswitch_port_add(second_unit, vswitch_vpls_shared_info_1.vpn, vlan_port_1.vlan_port_id);
        if (rv != BCM_E_NONE) {
            printf("Error, bcm_vswitch_port_add\n");
            return rv;
        }
    }
    
    /* update Multicast to have the added port  */

    rv = multicast__vlan_port_add(unit, vswitch_vpls_info_1.vpn_id, port , vlan_port_1.vlan_port_id, egress_mc);
    if (rv != BCM_E_NONE) {
        printf("Error, multicast__vlan_port_add\n");
        return rv;
    }
    if (is_two_dev)
    {
        rv = multicast__vlan_port_add(second_unit, vswitch_vpls_info_1.vpn_id, port , vlan_port_1.vlan_port_id, egress_mc);
        if (rv != BCM_E_NONE) {
            printf("Error, multicast__vlan_port_add\n");
            return rv;
        }
    }
    if(verbose1){
        printf("add port   0x%08x to multicast \n\r",vlan_port_1.vlan_port_id);
    }
    
    return rv;
}

int
vswitch_vpls_add_access_port_2(int unit,  bcm_gport_t *port_id){
    int rv;
    bcm_vlan_port_t vlan_port_1;
    /* add port, according to port_vlan_vlan */
    bcm_vlan_port_t_init(&vlan_port_1);
    
    /* Setting network group according to ac_network_group_id_2 */
    if (is_device_or_above(unit,JERICHO) && (soc_property_get(unit , "split_horizon_forwarding_groups_mode",1)==2)) {
        vlan_port_1.ingress_network_group_id = ac_network_group_id_ingress_2;
        vlan_port_1.egress_network_group_id = ac_network_group_id_egress_2;
    }

    /* set port attribures, key <port-vlan-vlan>*/
    vlan_port_1.criteria = BCM_VLAN_PORT_MATCH_PORT_VLAN_STACKED;
    vlan_port_1.port = vswitch_vpls_info_2.ac_in_port;
    vlan_port_1.match_vlan = vswitch_vpls_info_1.ac_port2_outer_vlan;;
    vlan_port_1.match_inner_vlan = vswitch_vpls_info_1.ac_port2_inner_vlan; ; 
    vlan_port_1.egress_vlan = vswitch_vpls_info_1.ac_port2_outer_vlan;
    vlan_port_1.egress_inner_vlan = vswitch_vpls_info_1.ac_port2_inner_vlan;
    vlan_port_1.flags = vswitch_vpls_info_1.ac_port2_flags;
    rv = bcm_vlan_port_create(unit, &vlan_port_1);
    if (rv != BCM_E_NONE) {
        printf("Error, bcm_vlan_port_add\n");
        return rv;
    }

    rv = bcm_vswitch_port_add(unit, vswitch_vpls_shared_info_1.vpn, vlan_port_1.vlan_port_id);
    if (rv != BCM_E_NONE) {
        printf("Error, bcm_vswitch_port_add\n");
        return rv;
    }

    /* In advanced vlan translation mode, bcm_vlan_port_create does not create ingress / egress
        action mapping. This is here to compensate. */
    if (advanced_vlan_translation_mode) {
        rv = vlan_translation_vlan_port_create_to_translation(unit, &vlan_port_1);
        if (rv != BCM_E_NONE) {
            printf("Error: vlan_translation_vlan_port_create_to_translation\n");
        }
    }
    

    vlan_port_encap_id2 = vlan_port_1.encap_id;
    if(verbose1){
        printf("vlan_port_encap_id2: 0x%08x\n\r",vlan_port_encap_id2);
    }

    *port_id = vlan_port_1.vlan_port_id;
    
    /* update Multicast to have the added port  */
    rv = multicast__vlan_port_add(unit, vswitch_vpls_info_1.vpn_id, vswitch_vpls_info_2.ac_in_port , vlan_port_1.vlan_port_id, egress_mc);
    if (rv != BCM_E_NONE) {
        printf("Error, multicast__vlan_port_add\n");
        return rv;
    }
    if(verbose1){
        printf("add port   0x%08x to multicast \n\r",vlan_port_1.vlan_port_id);
    }
    
    return rv;
}

int
vswitch_vpls_run(int unit, int second_unit, int extend_example){

    int nof_outer_tpids;
    int nof_inner_tpids;
    bcm_gport_t access_port_id2;
    bcm_gport_t network_port_id2;
    bcm_mpls_vpn_config_t vpn_info;
    bcm_mpls_port_t mpls_port_1;
    bcm_mpls_port_t mpls_port_2;
    bcm_vlan_t  vpn;
    int rv;
    int unit_or_second = second_unit;
    int trap_code;
    bcm_rx_trap_config_t trap_config;
    bcm_multicast_t mc_id;
    bcm_multicast_replication_t reps[5] = {{0}};

    /* init values */
    /*vswitch_vlps_info_init(extend_example);*/
  
    /* set ports to identify double tags packets */

    if (second_unit < 0)
    {
        unit_or_second = unit;
    }

    if (pwe_cw) {
        rv = bcm_switch_control_set(unit, bcmSwitchMplsPWControlWord, pwe_cw);
        if (rv != BCM_E_NONE) {
            printf("Error, bcm_switch_control_set\n");
            return rv;
        }
    }

    /* Configure TTL1 trap to control plane used for VCCV type 3 */
    if (vccv_type3_ttl) {
        rv =  bcm_rx_trap_type_create(unit, 0/*flags*/, bcmRxTrapMplsTtl1, &trap_code);
        if (rv != BCM_E_NONE) {
           printf("(%s) \n",bcm_errmsg(rv));
           return rv;
        }

        bcm_rx_trap_config_t_init(&trap_config);
        trap_config.flags = (BCM_RX_TRAP_UPDATE_DEST | BCM_RX_TRAP_TRAP);
        trap_config.trap_strength = 7;
        trap_config.dest_port = default_cpu_port;

        rv = bcm_rx_trap_set(unit, trap_code, trap_config);
        if (rv != BCM_E_NONE) {
           printf("(%s) \n",bcm_errmsg(rv));
           return rv;
        }
    }
  
    printf("vswitch_vpls_run %d\n", vswitch_vpls_info_1.vpn_id);

    port_tpid_init(vswitch_vpls_info_1.pwe_in_port,1,0);
    rv = port_tpid_set(unit_or_second);
    if (rv != BCM_E_NONE) {
        printf("Error, port_tpid_set\n");
        return rv;
    }
    
    port_tpid_init(vswitch_vpls_info_1.ac_in_port,1,1);
    rv = port_tpid_set(unit);
    if (rv != BCM_E_NONE) {
        printf("Error, port_tpid_set\n");
        return rv;
    }

    /* In advanced vlan translation mode, the default ingress/ egress actions and mapping
        are not configured. This is here to compensate. */
    if (advanced_vlan_translation_mode) {
        rv = vlan_translation_default_mode_init(unit);
        if (rv != BCM_E_NONE) {
            printf("Error, in vlan_translation_default_mode_init\n");
            return rv;
        }
    }
   
    /* Init vlan */  
    vlan__init_vlan(unit,vswitch_vpls_info_1.ac_port1_outer_vlan);
    vlan__init_vlan(unit,vswitch_vpls_info_1.ac_port1_inner_vlan);
    vlan__init_vlan(unit,vswitch_vpls_info_1.ac_port2_outer_vlan);
    vlan__init_vlan(unit,vswitch_vpls_info_1.ac_port2_inner_vlan);

    if (second_unit >= 0)
    {
        vlan__init_vlan(second_unit,vswitch_vpls_info_1.ac_port1_outer_vlan);
        vlan__init_vlan(second_unit,vswitch_vpls_info_1.ac_port1_inner_vlan);
        vlan__init_vlan(second_unit,vswitch_vpls_info_1.ac_port2_outer_vlan);
        vlan__init_vlan(second_unit,vswitch_vpls_info_1.ac_port2_inner_vlan);
    } 

    if (extend_example) {

        /* define one more pwe_in_port and one more ac_in_port */
        port_tpid_init(vswitch_vpls_info_2.pwe_in_port,1,0);
        rv = port_tpid_set(unit_or_second);
        if (rv != BCM_E_NONE) {
            printf("Error, port_tpid_set\n");
            return rv;
        }
    
        port_tpid_init(vswitch_vpls_info_2.ac_in_port,1,1);
        rv = port_tpid_set(unit);
        if (rv != BCM_E_NONE) {
            printf("Error, port_tpid_set\n");
            return rv;
        }
    } 
    printf("vswitch_vpls_run configure mpls tunnels\n");
    /* configure MPLS tunnels */
    rv  = mpls_tunnels_config(unit, second_unit, extend_example);
    if (rv != BCM_E_NONE) {
        printf("Error, in mpls_tunnels_config\n");
        return rv;
    }

    printf("vswitch_vpls_run create vswitch\n");
    /* create vswitch */
    rv = mpls_port__vswitch_vpls_vpn_create__set(unit, vswitch_vpls_info_1.vpn_id);
    if (rv != BCM_E_NONE) {
        printf("Error, in mpls_port__vswitch_vpls_vpn_create__set\n");
        return rv;
    }
    if (second_unit >= 0)
    {
        rv = mpls_port__vswitch_vpls_vpn_create__set(second_unit, vswitch_vpls_info_1.vpn_id);
        if (rv != BCM_E_NONE) {
            printf("Error, in mpls_port__vswitch_vpls_vpn_create__set\n");
            return rv;
        }
    }

    printf("vswitch_vpls_run add vlan access port\n");
    /* add mpls access port */
    rv = vswitch_vpls_add_access_port_1(unit, second_unit, &vswitch_vpls_info_1.access_port_id);
    if (rv != BCM_E_NONE) {
        printf("Error, in vswitch_vpls_add_access_port_1\n");
        return rv;
    }
    printf("Added access port\n");

    printf("vswitch_vpls_run add mpls network port\n");
    /* add mpls network port */
    rv = vswitch_vpls_add_network_port_1(unit, second_unit, &network_port_id);
    if (rv != BCM_E_NONE) {
        printf("Error, in vswitch_vpls_add_network_port_1\n");
        return rv;
    }
    printf("Added network port\n");

    if (extend_example) {
  
        /* add one more mpls_access_port and one more mpls_network_port */

        /* add mpls access port */
        rv = vswitch_vpls_add_access_port_2(unit_or_second, &access_port_id2);
        if (rv != BCM_E_NONE) {
            printf("Error, in vswitch_vpls_add_access_port_2\n");
            return rv;
        }

        /* add mpls network port */
        rv = vswitch_vpls_add_network_port_2(unit, &network_port_id2);
        if (rv != BCM_E_NONE) {
            printf("Error, in vswitch_vpls_add_network_port_2\n");
            return rv;
        }
    }

    if (is_device_or_above(unit,JERICHO) && encap_optimized) {
            mc_id = default_vpn_id;
            bcm_multicast_replication_t_init(reps);
            bcm_multicast_replication_t_init(reps+1);
            bcm_multicast_replication_t_init(reps+2);
            bcm_multicast_replication_t_init(reps+3);

            /* AC outlif 1 */
            BCM_GPORT_LOCAL_SET(reps[0].port, vswitch_vpls_info_1.ac_in_port);
            reps[0].encap1 = vlan_port_encap_id1;

            /* AC outlif 2 */
            BCM_GPORT_LOCAL_SET(reps[1].port, vswitch_vpls_info_1.ac_in_port);
            reps[1].encap1 = vlan_port_encap_id2;

            /* PWE outlif 1 with second CUD for MPLS tunnel */
            BCM_GPORT_LOCAL_SET(reps[2].port, vswitch_vpls_info_1.pwe_out_port);
            reps[2].encap1 = mpls_port_encap_id1;
            /* In Arad Mode need to set 2 MSBs to 0b11 */
            reps[2].encap2 = soc_property_get(unit , "system_is_arad_in_system",0) ? BCM_L3_ITF_VAL_GET(mpls_tunnel_encap_id1) | 0x30000 : BCM_L3_ITF_VAL_GET(mpls_tunnel_encap_id1);
            reps[2].flags = BCM_MUTICAST_REPLICATION_ENCAP2_VALID;

            /* PWE outlif 2 with second CUD for MPLS tunnel */
            BCM_GPORT_LOCAL_SET(reps[3].port, vswitch_vpls_info_1.pwe_out_port);
            reps[3].encap1 = mpls_port_encap_id2;
            /* In Arad Mode need to set 2 MSBs to 0b11 */
            reps[3].encap2 = soc_property_get(unit , "system_is_arad_in_system",0) ? BCM_L3_ITF_VAL_GET(mpls_tunnel_encap_id2) | 0x30000 : BCM_L3_ITF_VAL_GET(mpls_tunnel_encap_id2);
            reps[3].flags = BCM_MUTICAST_REPLICATION_ENCAP2_VALID;

            bcm_multicast_create(unit, BCM_MULTICAST_WITH_ID|BCM_MULTICAST_EGRESS_GROUP, &mc_id);
            bcm_multicast_set(unit, mc_id, 0, 4, reps);
    }

    return rv;
}

/* Switch PWE1 from mpls tunnel (1000,1002) to the other (300,400) that was prevously configured for PWE2.
   This configures only the multicast traffic to go throught the new tunnel, unicast traffic is not effected. */
int
switch_pwe_tunnel(int unit) {
    int rv = BCM_E_NONE;
    bcm_mpls_egress_label_t label_array[1];
    int label_count;

    rv = bcm_mpls_tunnel_initiator_get(unit, pwe_encap_id, 1, &label_array, &label_count);
    if (rv != BCM_E_NONE) {
        printf("Error, in bcm_mpls_tunnel_initiator_get\n");
        return rv;
    }
    label_array[0].flags |= BCM_MPLS_EGRESS_LABEL_WITH_ID | BCM_MPLS_EGRESS_LABEL_REPLACE;
    label_array[0].tunnel_id = pwe_encap_id;
    label_array[0].l3_intf_id = second_mpls_intf;
    rv = bcm_mpls_tunnel_initiator_create(unit, 0, 1, label_array);
    if (rv != BCM_E_NONE) {
        printf("Error, in bcm_mpls_tunnel_initiator_create\n");
        return rv;
    }

    return rv; 
}

/* Delete PWE1, all the traffic should go throuth PWE2 (termination) */
int
delete_pwe_lif(int unit) {
    int rv = BCM_E_NONE;
    bcm_mpls_egress_label_t label_array[1];
    int label_count;

    rv = bcm_mpls_port_delete(unit, vswitch_vpls_shared_info_1.vpn, network_port_id);
    if (rv != BCM_E_NONE) {
        printf("Error, bcm_mpls_port_delete\n");
        return rv;
    }

    return rv;
}

int
vswitch_vpls_fill_mact(int unit){
    int rv = BCM_E_NONE;
    bcm_mac_t ac_mac = {0x07,0x08,0x09,0x0A,0x0B,0x0C};
    bcm_mac_t pwe_mac = {0x01,0x02,0x03,0x04,0x05,0x06}; 

    rv = vswitch_add_l2_addr_to_gport(unit, vswitch_vpls_info_1.access_port_id, ac_mac, vswitch_vpls_shared_info_1.vpn);
    if (rv != BCM_E_NONE) {
        printf("Error, in vswitch_add_l2_addr_to_gport\n");
        return rv;
    }

    rv = vswitch_add_l2_addr_to_gport(unit, network_port_id, pwe_mac, vswitch_vpls_shared_info_1.vpn);
    if (rv != BCM_E_NONE) {
        printf("Error, in vswitch_add_l2_addr_to_gport\n");
        return rv;
    }

    return rv;
}


int
vswitch_vpls_run_with_defaults(int unit, int second_unit, int extend_example){

    rv = mpls__mpls_pipe_mode_exp_set(unit);
    if (rv != BCM_E_NONE) {
        printf("Error, in mpls__mpls_pipe_mode_exp_set\n");
        return rv;
    }

    mpls_termination_label_index_enable = soc_property_get(unit , "mpls_termination_label_index_enable",0);
    vswitch_vlps_info_init(extend_example,1,13,13,10,20,15,30,default_vpn_id);
    return vswitch_vpls_run(unit, second_unit, extend_example);
}

int
vswitch_vpls_run_with_defaults_dvapi(int unit, int acP, int pweP){
    int second_unit = -1;
    int extend_example = 1;

    int rv = BCM_E_NONE;

    rv = mpls__mpls_pipe_mode_exp_set(unit);
    if (rv != BCM_E_NONE) {
        printf("Error, in mpls__mpls_pipe_mode_exp_set\n");
        return rv;
    }

    mpls_termination_label_index_enable = soc_property_get(unit , "mpls_termination_label_index_enable",0);
    vswitch_vlps_info_init(extend_example,acP,pweP,pweP,10,20,15,30,default_vpn_id);
    return vswitch_vpls_run(unit, second_unit, extend_example);
}

/* 
 * This is an example of push profile allocation by user.
 * bcm_mpls_port_add api is used in this cae to allocate push profile with id given by the user.
 * This profile can be later used in bcm_mpls_port_add (PWE creation) and bcm_mpls_tunnel_initiator. 
 * To do this the proprties given here as function parameters should be configured in these apis. 
 */ 
int
vswitch_vpls_allocate_push_profile(int unit, int exp, int ttl, int has_cw, int push_profile){
    int rv = BCM_E_NONE;
    bcm_mpls_port_t mpls_port_push;

    bcm_mpls_port_t_init(&mpls_port_push);
    mpls_port_push.flags = BCM_MPLS_PORT_WITH_ID;
    mpls_port_push.mpls_port_id = push_profile;
    mpls_port_push.egress_label.exp = exp;
    mpls_port_push.egress_label.ttl = ttl;
    mpls_port_push.flags |= has_cw ? BCM_MPLS_PORT_CONTROL_WORD : 0;
    mpls_port_push.flags2 = BCM_MPLS_PORT2_TUNNEL_PUSH_INFO;

    rv = bcm_mpls_port_add(unit, 0, &mpls_port_push);
    if (rv != BCM_E_NONE) {
        printf("Error, bcm_mpls_port_add\n");
        return rv;
    }

    return rv;
}

