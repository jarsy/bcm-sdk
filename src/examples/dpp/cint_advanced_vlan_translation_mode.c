/* $Id: cint_advanced_vlan_translation_mode.c,v 1.10 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/

/* 
 * set following soc properties
 * # Specify the "bcm886xx_vlan_translate_mode" mode. 0: disabled, 1: enabled.
 * bcm886xx_vlan_translate_mode=1
 *  
 * Example 1:  
 * The cint creates a simple bridging application and ingress/egress vlan editing configuration.
 * In this example, we would like the TPID to be transparent in a "replace" vlan translation.
 * Meaning, for incoming packet with TPID 0x8100 and VID 10, the outgoing packet should be with TPID 0x8100 and VID 20.
 * For incoming packet with TPID 0x9100 and VID 10, the outgoing packet should be with TPID 0x9100 and VID 20.
 *  
 *  
 * run:
 * cd ../../../../src/examples/dpp
 * cint cint_advanced_vlan_translation_mode.c 
 * cint cint_port_tpid.c 
 * cint
 * int unit = 0;
 * ive_translation_main(unit,port1,port2); OR eve_translation_main(unit,port1,port2); 
 *  
 * 
 * Main configuration: 
 * Set both ports with 2 TPIDs (0x8100, 0x9100). 
 * Set LIF edit profile. 
 * Set different tag format per TPID. 
 * Set different action class per tag format.  
 * Set different action per action class (derived from edit profile and tag format) 
 *  
 *  
 * run packet:
 *      ethernet header with DA 00:11:22:33:44:55, SA 1 
 *      Vlan tag: vlan-id 10, vlan tag type 0x8100
 * from <port1>
 *       the packet will arrive at port 2, vlan-id 20, vlan tag type 0x8100
 * 
 * run packet:
 *      ethernet header with DA 00:11:22:33:44:55, SA 1 
 *      Vlan tag: vlan-id 10, vlan tag type 0x9100
 * from <port1>
 *       the packet will arrive at port 2, vlan-id 20, vlan tag type 0x9100
 *  
 *  
 * Example 2: 
 * Ingress VLAN translation: In case of no match in VTT SEM L2 LIF lookup a default action command per port introduced. 
 * User can set different actions per port by setting vlan_translation_action_id field in bcm_port_tpid_class_set.
 *  
 * run: 
 * cd ../../../../src/examples/dpp
 * cint cint_advanced_vlan_translation_mode.c 
 * cint cint_port_tpid.c 
 * cint
 * int unit = 0; 
 * ive_main_port_default_run(unit, port1,port2,port3, port4); 
 *  
 * Main configuration: 
 * Create action command 
 * Set port TPIDs 
 * Set default action command per port. 
 * Associate port TPIDs settings to default action command 
 * Set action command properties 
 *  
 * run packet:
 *      ethernet header with DA 00:00:00:00:00:11 
 *      Vlan tag: vlan-id 10, vlan tag type 0x8100
 * from <port1>
 *       the packet will arrive at port 4, untag
 *  
 * run packet:
 *      ethernet header with DA 00:00:00:00:00:12 
 *      Vlan tag: vlan-id 10, vlan tag type 0x8100
 * from <port2>
 *       the packet will arrive at port 4, vlan-id 10, inner-vlan-id 10
 *  
 * run packet:
 *      ethernet header with DA 00:00:00:00:00:13 
 *      Vlan tag: vlan-id 10, vlan tag type 0x8100
 * from <port3>
 *       the packet will arrive at port 4, vlan-id 10
 *  
 *  
 * Example 3:  
 * Default egress translation actions are performed when there is no match found for existing out-LIF. 
 * The user can change the default egress translation actions by creating a default out-LIF per port. 
 * In this example, we first test the default actions: add vlan (new vlan = default vsi) for untagged packets. NOP for single tagged packets.
 * Then we change the default actions: NOP for untagged packets. Replcae outer vlan and add inner vlan for single tagged packets.
 * Then we create a default out-LIF and configure new default actions: add outer vlan (30) for both untagged and single tagged packets.  
 *  
 *  
 * run:
 * cd ../../../../src/examples/dpp
 * cint cint_advanced_vlan_translation_mode.c 
 * cint cint_port_tpid.c 
 * cint
 * int unit = 0;
 * eve_default_translation_main(int unit, int port1, int port2); // set default configuration
 * eve_default_translation_change(int unit); // change default translation actions configuration
 * default_vlan_port_set(int unit, int port); // set new default actions 
 *  
 * 
 * Main configuration: 
 * Set both ports TPIDs.
 * Change default actions.
 * Set default out-LIF. 
 * Set LIF edit profile. 
 * Set default action command. 
 * Set action class.   
 *  
 *  
 * run packet:
 *      ethernet header with DA 00:11:22:33:44:55, SA 1, untagged 
 * from <port1>
 *       the packet will arrive at port 2, vlan-id 30, vlan tag type 0x8100
 * 
 * run packet:
 *      ethernet header with DA 00:11:22:33:44:55, SA 1 
 *      Vlan tag: vlan-id 10, vlan tag type 0x8100
 * from <port1>
 *       the packet will arrive at port 2, outer-vlan-id 30, vlan tag type 0x8100
 *                                         inner-vlan-id 10, vlan tag type 0x8100
 *  
 *  
 * Example 4: 
 * Ingress priority and non priority packets handling. 
 * The user can change the default handling of incoming priority and non priority packets by LLVP configuration. 
 * In this example, one port's behavior is default (incoming priority and non priority packets are treated the same). 
 * The second port is configured to pass non priority packets and drop priority packets. 
 * The third port is configured to set different tag format for priority packets and untagged packets, 
 * so they can have diferent translation actions. 
 *  
 *  
 * run:
 * cd ../../../../src/examples/dpp
 * cint cint_advanced_vlan_translation_mode.c 
 * cint cint_port_tpid.c 
 * cint
 * int unit = 0;
 * ive_priority_tags_main(int unit, int port1, int port2, int port3, int port4); 
 *  
 * 
 * Main configuration: 
 * Set ports TPIDs according to desired behavior 
 *  
 *  
 * run packet:
 *      ethernet header with DA 00:11:22:33:44:55, SA 1
 *      Vlan tag: vlan-id 1, vlan tag type 0x8100
 * from <port1>
 *       the packet will arrive at port 4, vlan-id 1, vlan tag type 0x8100
 * 
 * run packet:
 *      ethernet header with DA 00:11:22:33:44:55, SA 1 
 *      Vlan tag: vlan-id 0, vlan tag type 0x8100
 * from <port1>
 *       the packet will arrive at port 4, vlan-id 0, vlan tag type 0x8100
 *  
 * run packet:
 *      ethernet header with DA 00:11:22:33:44:55, SA 1
 *      Vlan tag: vlan-id 1, vlan tag type 0x8100
 * from <port2>
 *       the packet will arrive at port 4, vlan-id 1, vlan tag type 0x8100
 * 
 * run packet:
 *      ethernet header with DA 00:11:22:33:44:55, SA 1 
 *      Vlan tag: vlan-id 0, vlan tag type 0x8100
 * from <port2>
 *       packet is dropped.
 *  
 * run packet:
 *      ethernet header with DA 00:11:22:33:44:55, SA 1, untagged
 * from <port1>
 *       the packet will arrive at port 4, vlan-id 40, vlan tag type 0x8100
 * 
 * run packet:
 *      ethernet header with DA 00:11:22:33:44:55, SA 1 
 *      Vlan tag: vlan-id 0, vlan tag type 0x8100
 * from <port1>
 *       the packet will arrive at port 4, untagged
 *   
 *
 *
 * Example 5: 
 * Egress priority and non priority packets handling. 
 * The user can change the default handling of incoming priority and non priority packets by LLVP configuration. 
 * In this example, the RX port is configured to set different tag format for priority packets and untagged packets, 
 * so they can have diferent egress translation actions. 
 *  
 *  
 * run:
 * cd ../../../../src/examples/dpp
 * cint cint_advanced_vlan_translation_mode.c 
 * cint cint_port_tpid.c 
 * cint
 * int unit = 0;
 * eve_priority_tags_main(int unit, int port1, int port2); 
 *  
 * 
 * Main configuration: 
 * Set ports TPIDs according to desired behavior
 * Set default out-LIF. 
 * Set LIF edit profile. 
 * Set default action command. 
 * Set action class.  
 *
 *
 * run packet:
 *      ethernet header with DA 00:11:22:33:44:55, SA 1, untagged
 * from <port1>
 *       the packet will arrive at port 2, vlan-id 50, vlan tag type 0x8100
 * 
 * run packet:
 *      ethernet header with DA 00:11:22:33:44:55, SA 1 
 *      Vlan tag: vlan-id 0, vlan tag type 0x8100
 * from <port1>
 *       the packet will arrive at port 4, untagged
 *  
 */

int verbose = 1;
int ingress_qos_map_id = 0;
int egress_qos_map_id = 0;
int qos_mode = 0; /* When set, this variable enables qos mapping in AVT mode */

int advanced_vlan_translation_mode = 0;


/* Set configuration for ingress vlan editing */
int
ive_translation_main(int unit, int port1, int port2) {
    bcm_gport_t vlan_port;
    bcm_vlan_t match_vid = 10;
    bcm_vlan_t new_vid = 20;
    int action_id_1=5, action_id_2=6;
    int rv;

    /* set class for both ports */
    rv = bcm_port_class_set(unit, port1, bcmPortClassId, port1);
    if (rv != BCM_E_NONE) {
        printf("Error, in bcm_port_class_set, port=%d, \n", port1);
        return rv;
    }

    rv = bcm_port_class_set(unit, port2, bcmPortClassId, port2);
    if (rv != BCM_E_NONE) {
        printf("Error, in bcm_port_class_set, port=%d, \n", port2);
        return rv;
    }

    /* set TPIDs for port1 */
    port_tpid_init(port1, 1, 1);
    rv = port_tpid_set(unit);
    if (rv != BCM_E_NONE) {
        printf("Error, port_tpid_set with port_1\n");
        print rv;
        return rv;
    }

    /* set TPIDs for port2 */
    port_tpid_init(port2, 1, 1);
    rv = port_tpid_set(unit);
    if (rv != BCM_E_NONE) {
        printf("Error, port_tpid_set with port_2\n");
        print rv;
        return rv;
    }

    /* create a vlan port:
       egress_vlan (out vlan) = match_vlan (in vlan) */
    rv = vlan_port_create(unit, port1, &vlan_port, match_vid, BCM_VLAN_PORT_MATCH_PORT_VLAN);
    if (rv != BCM_E_NONE) {
        printf("Error, vlan_port_create\n");
        return rv;
    }

    /* set port translation info */
    rv = vlan_port_translation_set(unit, new_vid, new_vid, vlan_port, 2, 1);
    if (rv != BCM_E_NONE) {
        printf("Error, vlan_port_translation_set\n");
        return rv;
    }

    /* set editing actions */
    rv = vlan_translate_action_set(unit, action_id_1, action_id_2, 1, bcmVlanActionReplace, bcmVlanActionNone);
    if (rv != BCM_E_NONE) {
        printf("Error, vlan_translate_action_set\n");
        return rv;
    }

    /* set action class in IVEC map */
    rv = vlan_translate_action_class_set(unit, action_id_1, action_id_2, 1);
    if (rv != BCM_E_NONE) {
        printf("Error, vlan_translate_action_class_set\n");
        return rv;
    }

    /* set static entry in MAC table */
    rv = l2_addr_add(unit, port2, match_vid);
    if (rv != BCM_E_NONE) {
        printf("Error, l2_addr_add\n");
        return rv;
    }

    /* Create VSI */
    rv = bcm_vlan_create(unit, match_vid);
    if (rv != BCM_E_NONE) {
        printf("Error, bcm_vlan_create\n");
        return rv;
    }

    return rv;
}

/* 
 * Fill Action 
 */ 
int
vlan_translation_action_add(int unit, int action_id, int flags, bcm_vlan_action_t outer_vid_source, bcm_vlan_action_t inner_vid_source, uint16 outer_tpid, uint16 inner_tpid)
{
    int rv;
    bcm_vlan_action_set_t action;

    /* Create action ID*/
    bcm_vlan_action_set_t_init(&action);
    
    action.dt_outer = outer_vid_source;
    action.dt_inner = inner_vid_source;
    action.outer_tpid = outer_tpid;
    action.inner_tpid = inner_tpid;

    /* Enables outer_pcp_dei_source, inner_pcp_dei_source to be determined according to pre-defined qos mapping*/
    if (advanced_vlan_translation_mode && qos_mode) {
        action.dt_inner_pkt_prio = bcmVlanActionAdd;
        action.dt_outer_pkt_prio = bcmVlanActionAdd;
    }
    
    rv = bcm_vlan_translate_action_id_set( unit, 
                                           flags,
                                           action_id,
                                           &action);
    if (rv != BCM_E_NONE) {
        printf("Error, bcm_vlan_translate_action_id_set: outer tpid: 0x%x inner tpid: 0x%x \n", 
               outer_tpid, inner_tpid);
        return rv;
    }

    return rv;
}

/* Set configuration for ingress vlan editing port default settings */
int
ive_main_port_default_run(int unit, int port1, int port2, int port3, int port4) {
    int rv;
    bcm_mac_t mac1  = {0x00, 0x00, 0x00, 0x00, 0x00, 0x11};
    bcm_mac_t mac2  = {0x00, 0x00, 0x00, 0x00, 0x00, 0x12};
    bcm_mac_t mac3  = {0x00, 0x00, 0x00, 0x00, 0x00, 0x13};
    bcm_l2_addr_t l2_addr;    
    int vid = 10;
    int action_id_1 = 20;
    int action_id_2 = 21;
    int action_id_3 = 22;
    int flags = BCM_VLAN_ACTION_SET_INGRESS | BCM_VLAN_ACTION_SET_WITH_ID;  

    /* this case should run in normal default translation model if device is jericho */
    if (is_device_or_above(unit, JERICHO)) {
        rv = bcm_switch_control_set(unit,bcmSwitchPortVlanTranslationAdvanced,0);
        if (rv != BCM_E_NONE){
            printf("Error, bcm_switch_control_set  rv %d\n", rv);
            return rv;
        }
    }
    
    /* Create new Action IDs */
    rv = bcm_vlan_translate_action_id_create( unit, flags, &action_id_1);
    if (rv != BCM_E_NONE) {
        printf("Error, bcm_vlan_translate_action_id_create\n");
        return rv;
    }

    rv = bcm_vlan_translate_action_id_create( unit, flags, &action_id_2);
    if (rv != BCM_E_NONE) {
        printf("Error, bcm_vlan_translate_action_id_create\n");
        return rv;
    }

    rv = bcm_vlan_translate_action_id_create( unit, flags, &action_id_3);
    if (rv != BCM_E_NONE) {
        printf("Error, bcm_vlan_translate_action_id_create\n");
        return rv;
    }

    /* Set Port TPIDs */
    port_tpid_init(port1,1,1);    
    /* Set Port Default VLAN-Action-CMD 1 */
    port_tpid_info1.vlan_transation_profile_id = action_id_1;
    printf("action_id_1 %d \n",action_id_1);
    rv = port_tpid_set(unit);
    if (rv != BCM_E_NONE) {
        printf("Error, port_tpid_set\n");
        return rv;
    }

    port_tpid_init(port2,1,1);
    /* Set Port Default VLAN-Action-CMD 2 */
    port_tpid_info1.vlan_transation_profile_id = action_id_2;
    printf("action_id_1 %d \n",action_id_2);
    rv = port_tpid_set(unit);
    if (rv != BCM_E_NONE) {
        printf("Error, port_tpid_set\n");
        return rv;
    }

    port_tpid_init(port3,1,1);
    /* Set Port Default VLAN-Action-CMD 3 */
    port_tpid_info1.vlan_transation_profile_id = action_id_3;
    printf("action_id_1 %d \n",action_id_3);
    rv = port_tpid_set(unit);
    if (rv != BCM_E_NONE) {
        printf("Error, port_tpid_set\n");
        return rv;
    }

    /* 
     * Set port1 Action command: 
     * Remove tag i.e. get out untagged. 
     * At the egress, default action command add VSI (=VLAN) 
     */
    rv = vlan_translation_action_add(unit, action_id_1, BCM_VLAN_ACTION_SET_INGRESS, bcmVlanActionDelete, bcmVlanActionNone, 0x8100, 0x9100);
    if (rv != BCM_E_NONE) {
        printf("Error, vlan_translation_action_add\n");
        return rv;
    }

    /*
     * Set port2 Action command: 
     * Add tag (Initial-VID). 
     */
    rv = vlan_translation_action_add(unit, action_id_2, BCM_VLAN_ACTION_SET_INGRESS, bcmVlanActionAdd, bcmVlanActionNone, 0x8100, 0x9100);
    if (rv != BCM_E_NONE) {
        printf("Error, vlan_translation_action_add\n");
        return rv;
    }
    
    /*  
     * Set port3 Action command: 
     * NOP 
     */
    rv = vlan_translation_action_add(unit, action_id_3, BCM_VLAN_ACTION_SET_INGRESS, bcmVlanActionNone, bcmVlanActionNone, 0x8100, 0x9100);
    if (rv != BCM_E_NONE) {
        printf("Error, vlan_translation_action_add\n");
        return rv;
    }
    
    /* set static entry in MAC table */
    bcm_l2_addr_t_init(&l2_addr, mac1, vid);
    l2_addr.port = port4;

    rv = bcm_l2_addr_add(unit, &l2_addr);
    if (rv != BCM_E_NONE) {
        printf("Error, bcm_l2_addr_add\n");
        return rv;
    }

    bcm_l2_addr_t_init(&l2_addr, mac2, vid);   
    l2_addr.port = port4;
     
    rv = bcm_l2_addr_add(unit, &l2_addr);
    if (rv != BCM_E_NONE) {
        printf("Error, bcm_l2_addr_add\n");
        return rv;
    }

    bcm_l2_addr_t_init(&l2_addr, mac3, vid);  
    l2_addr.port = port4;  
    rv = bcm_l2_addr_add(unit, &l2_addr);
    if (rv != BCM_E_NONE) {
        printf("Error, bcm_l2_addr_add\n");
        return rv;
    }

    return rv;
}

/* Set configuration for egress vlan editing */
int
eve_translation_main(int unit, int port1, int port2) {
    bcm_gport_t in_vlan_port, out_vlan_port;
    bcm_vlan_t match_vid = 10;
    bcm_vlan_t new_vid = 20;
    int action_id_1=5, action_id_2=6;
    int rv;

    /* set class for both ports */
    rv = bcm_port_class_set(unit, port1, bcmPortClassId, port1);
    if (rv != BCM_E_NONE) {
        printf("Error, in bcm_port_class_set, port=%d, \n", port1);
        return rv;
    }

    rv = bcm_port_class_set(unit, port2, bcmPortClassId, port2);
    if (rv != BCM_E_NONE) {
        printf("Error, in bcm_port_class_set, port=%d, \n", port2);
        return rv;
    }

    /* set TPIDs for port1 */
    port_tpid_init(port1, 1, 1);
    rv = port_tpid_set(unit);
    if (rv != BCM_E_NONE) {
        printf("Error, port_tpid_set with port_1\n");
        print rv;
        return rv;
    }

    /* set TPIDs for port2 */
    port_tpid_init(port2, 1, 1);
    rv = port_tpid_set(unit);
    if (rv != BCM_E_NONE) {
        printf("Error, port_tpid_set with port_2\n");
        print rv;
        return rv;
    }

    /* create a vlan port (in-Lif):
       egress_vlan (out vlan) = match_vlan (in vlan) */
    rv = vlan_port_create(unit, port1, &in_vlan_port, match_vid, BCM_VLAN_PORT_MATCH_PORT_VLAN);
    if (rv != BCM_E_NONE) {
        printf("Error, vlan_port_create\n");
        return rv;
    }

    /* create a vlan port (out-Lif):
    egress_vlan (out vlan) = match_vlan (in vlan) */
    rv = vlan_port_create(unit, port2, &out_vlan_port, match_vid, BCM_VLAN_PORT_MATCH_PORT_VLAN);
    if (rv != BCM_E_NONE) {
        printf("Error, vlan_port_create\n");
        return rv;
    }

    /* set NOP for ingress LIF */
    rv = vlan_port_translation_set(unit, match_vid, match_vid, in_vlan_port, 0, 1);
    if (rv != BCM_E_NONE) {
        printf("Error, vlan_port_translation_set\n");
        return rv;
    }

    /* set edit profile for egress LIF */
    rv = vlan_port_translation_set(unit, new_vid, new_vid, out_vlan_port, 2, 0);
    if (rv != BCM_E_NONE) {
        printf("Error, vlan_port_translation_set\n");
        return rv;
    }

    /* set editing actions*/
    rv = vlan_translate_action_set(unit, action_id_1, action_id_2, 0, bcmVlanActionReplace, bcmVlanActionNone);
    if (rv != BCM_E_NONE) {
        printf("Error, vlan_translate_action_set\n");
        return rv;
    }

    /* set action class in IVEC map */
    rv = vlan_translate_action_class_set(unit, action_id_1, action_id_2, 0);
    if (rv != BCM_E_NONE) {
        printf("Error, vlan_translate_action_class_set\n");
        return rv;
    }

    /* set static entry in MAC table */
    rv = l2_addr_add(unit, port2, match_vid);
    if (rv != BCM_E_NONE) {
        printf("Error, l2_addr_add\n");
        return rv;
    }

    /* Create VSI */
    rv = bcm_vlan_create(unit, match_vid);
    if (rv != BCM_E_NONE) {
        printf("Error, bcm_vlan_create\n");
        return rv;
    }

    return rv;
}

/* Set configuration for default egress vlan editing */
int
eve_default_translation_main(int unit, int port1, int port2) {
    bcm_gport_t in_vlan_port, out_vlan_port;
    bcm_vlan_t match_vid = 10;
    bcm_vlan_t default_vsi = 1;
    bcm_vlan_t default_vid = 30;
    int action_id_1=5, action_id_2=6;
    int rv;

    /* set class for both ports */
    rv = bcm_port_class_set(unit, port1, bcmPortClassId, port1);
    if (rv != BCM_E_NONE) {
        printf("Error, in bcm_port_class_set, port=%d, \n", port1);
        return rv;
    }

    rv = bcm_port_class_set(unit, port2, bcmPortClassId, port2);
    if (rv != BCM_E_NONE) {
        printf("Error, in bcm_port_class_set, port=%d, \n", port2);
        return rv;
    }

    /* set TPIDs for port1 */
    port_tpid_init(port1, 1, 1);
    rv = port_tpid_set(unit);
    if (rv != BCM_E_NONE) {
        printf("Error, port_tpid_set with port_1\n");
        print rv;
        return rv;
    }

    /* set TPIDs for port2 */
    port_tpid_init(port2, 1, 1);
    rv = port_tpid_set(unit);
    if (rv != BCM_E_NONE) {
        printf("Error, port_tpid_set with port_2\n");
        print rv;
        return rv;
    }

    /* create a vlan port (in-Lif):
       egress_vlan (out vlan) = match_vlan (in vlan) */
    rv = vlan_port_create(unit, port1, &in_vlan_port, match_vid, BCM_VLAN_PORT_MATCH_PORT_VLAN);
    if (rv != BCM_E_NONE) {
        printf("Error, vlan_port_create\n");
        return rv;
    }

    /* set NOP for ingress LIF */
    rv = vlan_port_translation_set(unit, match_vid, match_vid, in_vlan_port, 0, 1);
    if (rv != BCM_E_NONE) {
        printf("Error, vlan_port_translation_set\n");
        return rv;
    }

    /* set static entry in MAC table for default VSI */
    rv = l2_addr_add(unit, port2, default_vsi);
    if (rv != BCM_E_NONE) {
        printf("Error, l2_addr_add\n");
        return rv;
    }

    /* set static entry in MAC table for match vid*/
    rv = l2_addr_add(unit, port2, match_vid);
    if (rv != BCM_E_NONE) {
        printf("Error, l2_addr_add\n");
        return rv;
    }

    return rv;
}

/* Change configuration for default egress vlan editing actions */
int
eve_default_translation_change(int unit) {
    bcm_vlan_action_set_t action;
    int default_action_id_1 = 0, default_action_id_2 = 1;
    int rv;

    /* get the default action */
    rv = bcm_vlan_translate_action_id_get( unit, 
                                           BCM_VLAN_ACTION_SET_EGRESS,
                                           default_action_id_1,
                                           &action);
    if (rv != BCM_E_NONE) {
        printf("Error, bcm_vlan_translate_action_id_get \n");
        return rv;
    }  

    /* Change default action */
    action.dt_outer = bcmVlanActionNone;
    action.dt_outer_pkt_prio = bcmVlanActionNone;

    /* set the new default action */
    rv = bcm_vlan_translate_action_id_set( unit, 
                                           BCM_VLAN_ACTION_SET_EGRESS,
                                           default_action_id_1,
                                           &action);
    if (rv != BCM_E_NONE) {
        printf("Error, bcm_vlan_translate_action_id_set \n");
        return rv;
    }

    /* get the default action */
    rv = bcm_vlan_translate_action_id_get( unit, 
                                           BCM_VLAN_ACTION_SET_EGRESS,
                                           default_action_id_2,
                                           &action);
    if (rv != BCM_E_NONE) {
        printf("Error, bcm_vlan_translate_action_id_get \n");
        return rv;
    } 

    /* Change default action */
    action.dt_outer = bcmVlanActionMappedReplace;
    action.dt_inner = bcmVlanActionMappedAdd;

    /* set the new default action */
    rv = bcm_vlan_translate_action_id_set( unit, 
                                           BCM_VLAN_ACTION_SET_EGRESS,
                                           default_action_id_2,
                                           &action);
    if (rv != BCM_E_NONE) {
        printf("Error, bcm_vlan_translate_action_id_set \n");
        return rv;
    }

    return rv;
}

/* create new default out-LIF and set its translation edit profile */
int
default_vlan_port_set(int unit, int port) {
    bcm_gport_t default_vlan_port;
    bcm_vlan_t new_vid = 30;
    int action_id_1 = 5;
    int rv;
        
    /* create a new default vlan port (out-Lif) */
    rv = vlan_port_create(unit, port, &default_vlan_port, new_vid, BCM_VLAN_PORT_MATCH_PORT);
    if (rv != BCM_E_NONE) {
        printf("Error, vlan_port_create\n");
        return rv;
    }

    /* set edit profile for egress LIF */
    rv = vlan_port_translation_set(unit, new_vid, new_vid, default_vlan_port, 2, 0);
    if (rv != BCM_E_NONE) {
        printf("Error, vlan_port_translation_set\n");
        return rv;
    }

    /* set editing actions*/
    rv = vlan_translate_action_set(unit, action_id_1, 0, 0, bcmVlanActionAdd, bcmVlanActionNone);
    if (rv != BCM_E_NONE) {
        printf("Error, vlan_default_translate_action_set\n");
        return rv;
    }

    /* set action class */
    rv = vlan_default_translate_action_class_set(unit, action_id_1);
    if (rv != BCM_E_NONE) {
        printf("Error, vlan_translate_action_class_set\n");
        return rv;
    }

    /* set static entry in MAC table for new vid*/
    rv = l2_addr_add(unit, port, new_vid);
    if (rv != BCM_E_NONE) {
        printf("Error, l2_addr_add\n");
        return rv;
    }
}

/* create vlan_port and set vlan port translation */
int
create_untagged_vlan_port(
    int unit,
    bcm_port_t port_id,  
    bcm_gport_t *gport,
    bcm_vlan_t vlan /* incoming outer vlan and also outgoing vlan */, 
    bcm_vlan_t new_vid, 
	uint32 edit_class_id, 
	uint8 is_ingress
    ) {

	int rv=BCM_E_NONE;
	
    rv = vlan_port_create(unit, port_id, gport, vlan, BCM_VLAN_PORT_MATCH_PORT_INITIAL_VLAN);
    if (rv != BCM_E_NONE) {
        printf("Error, vlan_port_create\n");
        return rv;
    }

    /* set edit profile for ingress LIF */
    rv = vlan_port_translation_set(unit, new_vid, new_vid, *gport, edit_class_id, is_ingress);
    if (rv != BCM_E_NONE) {
        printf("Error, vlan_port_translation_set\n");
        return rv;
    }

	return rv;
}

/* Set ports TPID class to handle priority and non priority packets as desired
   port1: priority - pass, non priority - pass
   port2: priority - drop, non priority - pass
   port3: priority - delete vlan, non priority (untagged) - add vid */
int
ive_priority_tags_main(int unit, int port1, int port2, int port3, int port4) {
    bcm_gport_t vlan_port;
    int action_id_1=5, action_id_2=6;;
    bcm_port_tpid_class_t port_tpid_class;
    bcm_vlan_translate_action_class_t action_class;
    bcm_vlan_action_set_t action;
    bcm_vlan_t default_vsi = 1;
    bcm_vlan_t new_vid = 40;
    int rv;

    /* set class for all ports */
    rv = bcm_port_class_set(unit, port1, bcmPortClassId, port1);
    if (rv != BCM_E_NONE) {
        printf("Error, in bcm_port_class_set, port=%d, \n", port1);
        return rv;
    }

    rv = bcm_port_class_set(unit, port2, bcmPortClassId, port2);
    if (rv != BCM_E_NONE) {
        printf("Error, in bcm_port_class_set, port=%d, \n", port2);
        return rv;
    }

    rv = bcm_port_class_set(unit, port3, bcmPortClassId, port3);
    if (rv != BCM_E_NONE) {
        printf("Error, in bcm_port_class_set, port=%d, \n", port3);
        return rv;
    }

    rv = bcm_port_class_set(unit, port4, bcmPortClassId, port4);
    if (rv != BCM_E_NONE) {
        printf("Error, in bcm_port_class_set, port=%d, \n", port4);
        return rv;
    }

    /* set TPIDs for port1 */
    port_tpid_init(port1, 1, 1);
    rv = port_tpid_set(unit);
    if (rv != BCM_E_NONE) {
        printf("Error, port_tpid_set with port_1\n");
        print rv;
        return rv;
    }

    /* set TPIDs for port2. Drop priority packets */
    port_tpid_init(port2, 1, 1);
    port_tpid_info1.tpid_class_flags = BCM_PORT_TPID_CLASS_OUTER_IS_PRIO | BCM_PORT_TPID_CLASS_DISCARD;
    rv = port_tpid_set(unit);
    if (rv != BCM_E_NONE) {
        printf("Error, port_tpid_set with port_2\n");
        print rv;
        return rv;
    }
    
    /* set TPIDs for port3 */
    port_tpid_init(port3, 1, 1);
    rv = port_tpid_set(unit);
    if (rv != BCM_E_NONE) {
        printf("Error, port_tpid_set with port_3\n");
        print rv;
        return rv;
    }
 
    /* set tag format 2 for priority packets */
    bcm_port_tpid_class_t_init(&port_tpid_class);
    port_tpid_class.port = port3;
    port_tpid_class.tpid1 = 0x8100;
    port_tpid_class.tpid2 = BCM_PORT_TPID_CLASS_TPID_ANY;
    port_tpid_class.tag_format_class_id = 2;
    port_tpid_class.flags = BCM_PORT_TPID_CLASS_OUTER_IS_PRIO;
    port_tpid_class.vlan_translation_action_id = 0;
    rv = bcm_port_tpid_class_set(unit, &port_tpid_class);
    if (rv != BCM_E_NONE) {
        printf("Error, in bcm_port_tpid_class_set, port=%d, \n", port);
        return rv;
    }

    /* set tag format 0 for untagged packets */
    bcm_port_tpid_class_t_init(&port_tpid_class);
    port_tpid_class.port = port3;
    port_tpid_class.tpid1 = BCM_PORT_TPID_CLASS_TPID_INVALID;
    port_tpid_class.tpid2 = BCM_PORT_TPID_CLASS_TPID_INVALID;
    port_tpid_class.tag_format_class_id = 0;  
    port_tpid_class.flags = BCM_PORT_TPID_CLASS_OUTER_NOT_PRIO;
    port_tpid_class.vlan_translation_action_id = 0;
    rv = bcm_port_tpid_class_set(unit, &port_tpid_class);
    if (rv != BCM_E_NONE) {
        printf("Error, in bcm_port_tpid_class_set, port=%d, \n", port);
        return rv;
    }

    /* set TPIDs for port4 */
    port_tpid_init(port4, 1, 1);
    rv = port_tpid_set(unit);
    if (rv != BCM_E_NONE) {
        printf("Error, port_tpid_set with port_4\n");
        print rv;
        return rv;
    }        

    /* create a vlan port:
       egress_vlan (out vlan) = match_vlan (in vlan) */
	create_untagged_vlan_port(unit, port3, vlan_port, default_vsi, new_vid, 2, 1);

    /* Create action ID 1 */
    rv = bcm_vlan_translate_action_id_create( unit, BCM_VLAN_ACTION_SET_INGRESS | BCM_VLAN_ACTION_SET_WITH_ID, &action_id_1);
    if (rv != BCM_E_NONE) {
        printf("Error, bcm_vlan_translate_action_id_create 1\n");
        return rv;
    }

    /* Create action ID 2 */
    rv = bcm_vlan_translate_action_id_create( unit, BCM_VLAN_ACTION_SET_INGRESS | BCM_VLAN_ACTION_SET_WITH_ID, &action_id_2);
    if (rv != BCM_E_NONE) {
        printf("Error, bcm_vlan_translate_action_id_create 2\n");
        return rv;
    }

    /* Set translation action 1. delete vlan. */
    bcm_vlan_action_set_t_init(&action);
    action.dt_outer = bcmVlanActionReplace;
    action.dt_inner = bcmVlanActionAdd;
    action.outer_tpid = 0x8100;
    action.inner_tpid = 0x9100;
    rv = bcm_vlan_translate_action_id_set( unit, 
                                           BCM_VLAN_ACTION_SET_INGRESS,
                                           action_id_1,
                                           &action);
    if (rv != BCM_E_NONE) {
        printf("Error, bcm_vlan_translate_action_id_set 1\n");
        return rv;
    } 

    /* Set translation action 2. add vlan. */
    bcm_vlan_action_set_t_init(&action);
    action.dt_outer = bcmVlanActionAdd;
    action.dt_inner = bcmVlanActionNone;
    action.outer_tpid = 0x8100;
    action.inner_tpid = 0x9100;
    rv = bcm_vlan_translate_action_id_set( unit, 
                                           BCM_VLAN_ACTION_SET_INGRESS,
                                           action_id_2,
                                           &action);
    if (rv != BCM_E_NONE) {
        printf("Error, bcm_vlan_translate_action_id_set 1\n");
        return rv;
    } 

    /* Set translation action class - priority packets */
    bcm_vlan_translate_action_class_t_init(&action_class);
    action_class.vlan_edit_class_id = 2;
    action_class.tag_format_class_id = 2;
    action_class.vlan_translation_action_id	= action_id_1;
    action_class.flags = BCM_VLAN_ACTION_SET_INGRESS;
    rv = bcm_vlan_translate_action_class_set( unit,  &action_class);
    if (rv != BCM_E_NONE) {
        printf("Error, bcm_vlan_translate_action_class_set\n");
        return rv;
    }

    /* Set translation action class - untagged packets */
    bcm_vlan_translate_action_class_t_init(&action_class);
    action_class.vlan_edit_class_id = 2;
    action_class.tag_format_class_id = 0;
    action_class.vlan_translation_action_id	= action_id_2;
    action_class.flags = BCM_VLAN_ACTION_SET_INGRESS;
    rv = bcm_vlan_translate_action_class_set( unit,  &action_class);
    if (rv != BCM_E_NONE) {
        printf("Error, bcm_vlan_translate_action_class_set\n");
        return rv;
    }

    /* set static entry in MAC table */
    rv = l2_addr_add(unit, port4, 1);
    if (rv != BCM_E_NONE) {
        printf("Error, l2_addr_add\n");
        return rv;
    }

    /* set static entry in MAC table */
    rv = l2_addr_add(unit, port4, new_vid);
    if (rv != BCM_E_NONE) {
        printf("Error, l2_addr_add\n");
        return rv;
    }

    return rv;
}

/* Set port TPID class to handle priority and non priority packets as desired.
   Egress vlan translation: priority - delete vlan, non priority (untagged) - add vid */
int
eve_priority_tags_main(int unit, int port1, int port2) {
    bcm_gport_t vlan_port, in_vlan_port;
    int action_id_1=5, action_id_2=6;;
    bcm_port_tpid_class_t port_tpid_class;
    bcm_vlan_translate_action_class_t action_class;
    bcm_vlan_action_set_t action;
    bcm_vlan_t default_vsi = 1;
    bcm_vlan_t new_vid = 50;
    int rv;

    /* set class for both ports */
    rv = bcm_port_class_set(unit, port1, bcmPortClassId, port1);
    if (rv != BCM_E_NONE) {
        printf("Error, in bcm_port_class_set, port=%d, \n", port1);
        return rv;
    }

    rv = bcm_port_class_set(unit, port2, bcmPortClassId, port2);
    if (rv != BCM_E_NONE) {
        printf("Error, in bcm_port_class_set, port=%d, \n", port2);
        return rv;
    }

    /* set TPIDs for port1 */
    port_tpid_init(port1, 1, 1);
    rv = port_tpid_set(unit);
    if (rv != BCM_E_NONE) {
        printf("Error, port_tpid_set with port_1\n");
        print rv;
        return rv;
    }

    /* set TPIDs for port2. */
    port_tpid_init(port2, 1, 1);
    rv = port_tpid_set(unit);
    if (rv != BCM_E_NONE) {
        printf("Error, port_tpid_set with port_2\n");
        print rv;
        return rv;
    }
 
    /* set tag format 2 for priority packets */
    bcm_port_tpid_class_t_init(&port_tpid_class);
    port_tpid_class.port = port1;
    port_tpid_class.tpid1 = 0x8100;
    port_tpid_class.tpid2 = BCM_PORT_TPID_CLASS_TPID_ANY;
    port_tpid_class.tag_format_class_id = 2;
    port_tpid_class.flags = BCM_PORT_TPID_CLASS_OUTER_IS_PRIO;
    port_tpid_class.vlan_translation_action_id = 0;
    rv = bcm_port_tpid_class_set(unit, &port_tpid_class);
    if (rv != BCM_E_NONE) {
        printf("Error, in bcm_port_tpid_class_set, port=%d, \n", port);
        return rv;
    }

    /* set tag format 0 for untagged packets */
    bcm_port_tpid_class_t_init(&port_tpid_class);
    port_tpid_class.port = port1;
    port_tpid_class.tpid1 = BCM_PORT_TPID_CLASS_TPID_INVALID;
    port_tpid_class.tpid2 = BCM_PORT_TPID_CLASS_TPID_INVALID;
    port_tpid_class.tag_format_class_id = 0;  
    port_tpid_class.flags = BCM_PORT_TPID_CLASS_OUTER_NOT_PRIO;
    port_tpid_class.vlan_translation_action_id = 0;
    rv = bcm_port_tpid_class_set(unit, &port_tpid_class);
    if (rv != BCM_E_NONE) {
        printf("Error, in bcm_port_tpid_class_set, port=%d, \n", port);
        return rv;
    }        

    /* create a vlan port:
       egress_vlan (out vlan) = match_vlan (in vlan) */
    rv = vlan_port_create(unit, port2, &vlan_port, default_vsi, BCM_VLAN_PORT_MATCH_PORT_VLAN);
    if (rv != BCM_E_NONE) {
        printf("Error, vlan_port_create\n");
        return rv;
    }

    /* set edit profile for egress LIF */
    rv = vlan_port_translation_set(unit, new_vid, new_vid, vlan_port, 2, 0);
    if (rv != BCM_E_NONE) {
        printf("Error, vlan_port_translation_set\n");
        return rv;
    }

    /* Create action ID 1 */
    rv = bcm_vlan_translate_action_id_create( unit, BCM_VLAN_ACTION_SET_EGRESS | BCM_VLAN_ACTION_SET_WITH_ID, &action_id_1);
    if (rv != BCM_E_NONE) {
        printf("Error, bcm_vlan_translate_action_id_create 1\n");
        return rv;
    }

    /* Create action ID 2 */
    rv = bcm_vlan_translate_action_id_create( unit, BCM_VLAN_ACTION_SET_EGRESS | BCM_VLAN_ACTION_SET_WITH_ID, &action_id_2);
    if (rv != BCM_E_NONE) {
        printf("Error, bcm_vlan_translate_action_id_create 2\n");
        return rv;
    }

    /* Set translation action 1. delete vlan. */
    bcm_vlan_action_set_t_init(&action);
    action.dt_outer = bcmVlanActionDelete;
    action.dt_inner = bcmVlanActionNone;
    action.outer_tpid = 0x8100;
    action.inner_tpid = 0x9100;
    rv = bcm_vlan_translate_action_id_set( unit, 
                                           BCM_VLAN_ACTION_SET_EGRESS,
                                           action_id_1,
                                           &action);
    if (rv != BCM_E_NONE) {
        printf("Error, bcm_vlan_translate_action_id_set 1\n");
        return rv;
    } 

    /* Set translation action 2. add vlan. */
    bcm_vlan_action_set_t_init(&action);
    action.dt_outer = bcmVlanActionAdd;
    action.dt_inner = bcmVlanActionNone;
    action.outer_tpid = 0x8100;
    action.inner_tpid = 0x9100;
    rv = bcm_vlan_translate_action_id_set( unit, 
                                           BCM_VLAN_ACTION_SET_EGRESS,
                                           action_id_2,
                                           &action);
    if (rv != BCM_E_NONE) {
        printf("Error, bcm_vlan_translate_action_id_set 1\n");
        return rv;
    } 

    /* Set translation action class - priority packets */
    bcm_vlan_translate_action_class_t_init(&action_class);
    action_class.vlan_edit_class_id = 2;
    action_class.tag_format_class_id = 2;
    action_class.vlan_translation_action_id	= action_id_1;
    action_class.flags = BCM_VLAN_ACTION_SET_EGRESS;
    rv = bcm_vlan_translate_action_class_set( unit,  &action_class);
    if (rv != BCM_E_NONE) {
        printf("Error, bcm_vlan_translate_action_class_set\n");
        return rv;
    }

    /* Set translation action class - untagged packets */
    bcm_vlan_translate_action_class_t_init(&action_class);
    action_class.vlan_edit_class_id = 2;
    action_class.tag_format_class_id = 0;
    action_class.vlan_translation_action_id	= action_id_2;
    action_class.flags = BCM_VLAN_ACTION_SET_EGRESS;
    rv = bcm_vlan_translate_action_class_set( unit,  &action_class);
    if (rv != BCM_E_NONE) {
        printf("Error, bcm_vlan_translate_action_class_set\n");
        return rv;
    }

    /* set static entry in MAC table */
    rv = l2_addr_add(unit, port2, default_vsi);
    if (rv != BCM_E_NONE) {
        printf("Error, l2_addr_add\n");
        return rv;
    }

    /* set static entry in MAC table */
    rv = l2_addr_add(unit, port2, new_vid);
    if (rv != BCM_E_NONE) {
        printf("Error, l2_addr_add\n");
        return rv;
    }

    return rv;
}


/* create vlan_port (Logical interface identified by port-vlan) */
int
vlan_port_create(
    int unit,
    bcm_port_t port_id,  
    bcm_gport_t *gport,
    bcm_vlan_t vlan /* incoming outer vlan and also outgoing vlan */, 
    bcm_vlan_port_match_t criteria
    ) {

    int rv;
    bcm_vlan_port_t vp;

    bcm_vlan_port_t_init(&vp);
  
    vp.criteria = criteria;
    vp.port = port_id;
    vp.match_vlan = vlan; 
    vp.vsi = vlan;
    vp.flags = 0;
    rv = bcm_vlan_port_create(unit, &vp);
    if (rv != BCM_E_NONE) {
        printf("Error, bcm_vlan_port_create\n"); 
        print rv;
        return rv;
    }

    *gport = vp.vlan_port_id;  
    return BCM_E_NONE;
}

/* set vlan port translation, determine edit profile for LIF */
int
vlan_port_translation_set(int unit, bcm_vlan_t new_vid, bcm_vlan_t new_inner_vid, bcm_gport_t vlan_port, uint32 edit_class_id, uint8 is_ingress) {
    bcm_vlan_port_translation_t port_trans;
    int rv;
    
    /* Set port translation */
    bcm_vlan_port_translation_t_init(&port_trans);	
    port_trans.new_outer_vlan = new_vid;		
    port_trans.new_inner_vlan = new_inner_vid;
    port_trans.gport = vlan_port;
    port_trans.vlan_edit_class_id = edit_class_id;
    port_trans.flags = is_ingress ? BCM_VLAN_ACTION_SET_INGRESS : BCM_VLAN_ACTION_SET_EGRESS;
    rv = bcm_vlan_port_translation_set(unit, &port_trans);
    if (rv != BCM_E_NONE) {
        printf("Error, bcm_vlan_port_translation_set\n");
        return rv;
    }

    if (verbose) {
        printf("Configure %s vlan port: 0x%x with outer vlan: %d, inner vlan: %d, ve profile: %d \n", (is_ingress ? "ingress" : "egress"), vlan_port, new_vid, new_inner_vid, edit_class_id);
    }

    return rv;
}



/* set translation actions (replace) */
int
vlan_translate_action_set(int unit, int action_id_1, int action_id_2, uint8 is_ingress, bcm_vlan_action_t outer_action, bcm_vlan_action_t inner_action) {
    bcm_vlan_action_set_t action;
    uint32 flags;
    int rv;

    flags = is_ingress ? BCM_VLAN_ACTION_SET_INGRESS : BCM_VLAN_ACTION_SET_EGRESS;

    /* Create action IDs*/
    rv = bcm_vlan_translate_action_id_create( unit, flags | BCM_VLAN_ACTION_SET_WITH_ID, &action_id_1);
    if (rv != BCM_E_NONE) {
        printf("Error, bcm_vlan_translate_action_id_create\n");
        return rv;
    }

    if(action_id_2 != 0) {
        rv = bcm_vlan_translate_action_id_create( unit, flags | BCM_VLAN_ACTION_SET_WITH_ID, &action_id_2);
        if (rv != BCM_E_NONE) {
            printf("Error, bcm_vlan_translate_action_id_create\n");
            return rv;
        }
    }

    /* Set translation action 1. outer action, set TPID 0x8100. */
    bcm_vlan_action_set_t_init(&action);
    action.dt_outer = outer_action;
    action.dt_inner = inner_action;
    action.outer_tpid = 0x8100;
    rv = bcm_vlan_translate_action_id_set( unit, 
                                           flags,
                                           action_id_1,
                                           &action);
    if (rv != BCM_E_NONE) {
        printf("Error, bcm_vlan_translate_action_id_set 1\n");
        return rv;
    }  

    if(action_id_2 != 0) {
        /* Set translation action 2. outer action, set TPID 0x9100. */
        bcm_vlan_action_set_t_init(&action);
        action.dt_outer = outer_action;
        action.dt_inner = inner_action;
        action.outer_tpid = 0x9100;
        action.inner_tpid = 0x8100;                     
        rv = bcm_vlan_translate_action_id_set( unit, 
                                               flags,
                                               action_id_2,
                                               &action);
        if (rv != BCM_E_NONE) {
            printf("Error, bcm_vlan_translate_action_id_set 2\n");
            return rv;
        }  
    }
    
    return rv; 
}



/* set translation actions (replace) */
int
vlan_default_translate_action_set(int unit, int action_id_1) {
    bcm_vlan_action_set_t action;
    uint32 flags;
    int rv;

    flags = BCM_VLAN_ACTION_SET_EGRESS;

    /* Create action IDs*/
    rv = bcm_vlan_translate_action_id_create( unit, flags | BCM_VLAN_ACTION_SET_WITH_ID, &action_id_1);
    if (rv != BCM_E_NONE) {
        printf("Error, bcm_vlan_translate_action_id_create\n");
        return rv;
    }

    /* Set translation action 1. for incoming untagged packets. */
    bcm_vlan_action_set_t_init(&action);
    action.dt_outer = bcmVlanActionAdd;
    action.dt_inner = bcmVlanActionNone;
    action.outer_tpid = 0x8100;
    action.inner_tpid = 0x9100;
    rv = bcm_vlan_translate_action_id_set( unit, 
                                           flags,
                                           action_id_1,
                                           &action);
    if (rv != BCM_E_NONE) {
        printf("Error, bcm_vlan_translate_action_id_set 1\n");
        return rv;
    }  
    
    return rv; 
}

/* set translation action classes for different tag formats */
int
vlan_translate_action_class_set(int unit, int action_id_1, int action_id_2, uint8 is_ingress ) {
    bcm_vlan_translate_action_class_t action_class;
    int rv;

    /* Set translation action class for single tagged packets with TPID 0x8100 */
    bcm_vlan_translate_action_class_t_init(&action_class);
    action_class.vlan_edit_class_id = 2;
    action_class.tag_format_class_id = 2;
    action_class.vlan_translation_action_id	= action_id_1;
    action_class.flags = is_ingress ? BCM_VLAN_ACTION_SET_INGRESS : BCM_VLAN_ACTION_SET_EGRESS;
    rv = bcm_vlan_translate_action_class_set( unit,  &action_class);
    if (rv != BCM_E_NONE) {
        printf("Error, bcm_vlan_translate_action_class_set\n");
        return rv;
    }

    /* Set translation action class for single tagged packets with TPID 0x9100 */
    bcm_vlan_translate_action_class_t_init(&action_class);
    action_class.vlan_edit_class_id = 2;
    action_class.tag_format_class_id = 3;
    action_class.vlan_translation_action_id	= action_id_2;
    action_class.flags = is_ingress ? BCM_VLAN_ACTION_SET_INGRESS : BCM_VLAN_ACTION_SET_EGRESS;
    rv = bcm_vlan_translate_action_class_set( unit,  &action_class);
    if (rv != BCM_E_NONE) {
        printf("Error, bcm_vlan_translate_action_class_set\n");
        return rv;
    }

    return rv;
}

/* Set untagged and single tagged packets, matched with default out-LIF to use action ID */
int
vlan_default_translate_action_class_set(int unit, int action_id) {
    bcm_vlan_translate_action_class_t action_class;
    int rv;

    /* Set translation action class for single tagged packets with TPID 0x8100 */
    bcm_vlan_translate_action_class_t_init(&action_class);
    action_class.vlan_edit_class_id = 2;
    action_class.tag_format_class_id = 2;
    action_class.vlan_translation_action_id	= action_id;
    action_class.flags = BCM_VLAN_ACTION_SET_EGRESS;
    rv = bcm_vlan_translate_action_class_set( unit,  &action_class);
    if (rv != BCM_E_NONE) {
        printf("Error, bcm_vlan_translate_action_class_set\n");
        return rv;
    }

    /* Set translation action class for untagged */
    bcm_vlan_translate_action_class_t_init(&action_class);
    action_class.vlan_edit_class_id = 2;
    action_class.tag_format_class_id = 0;
    action_class.vlan_translation_action_id	= action_id;
    action_class.flags = BCM_VLAN_ACTION_SET_EGRESS;
    rv = bcm_vlan_translate_action_class_set( unit,  &action_class);
    if (rv != BCM_E_NONE) {
        printf("Error, bcm_vlan_translate_action_class_set\n");
        return rv;
    }

    return rv;
}

/* add static entry to MAC table */
int
l2_addr_add(
    int unit, 
    bcm_gport_t port, 
    uint16 vid
    ) {

    bcm_l2_addr_t l2addr;
    bcm_mac_t mac  = {0x00, 0x11, 0x22, 0x33, 0x44, 0x55};
    int rv;

    bcm_l2_addr_t_init(&l2addr, mac, vid);

    l2addr.port = port;

    rv = bcm_l2_addr_add(unit, &l2addr);
    if (rv != BCM_E_NONE) {
        printf("Error, bcm_l2_addr_add\n");
        print rv;
        return rv;
    }

    return BCM_E_NONE;
}

/* Constant values for the old vlan translation actions and profiles */
const int INGRESS_NOP_PROFILE                   = 0;
const int INGRESS_REMOVE_TAGS_PROFILE           = 4;
const int INGRESS_REMOVE_OUTER_TAG_ACTION       = 5;
const int EGRESS_NOP_PROFILE                    = 0;
const int EGRESS_REMOVE_TAGS_PROFILE            = 2;
const int EGRESS_REMOVE_TAGS_PUSH_1_PROFILE     = 5;
const int EGRESS_REMOVE_TAGS_PUSH_2_PROFILE     = 8;
const int EGRESS_UNTAGGED_OFFSET                = 0;
const int EGRESS_SINGLE_TAGGED_OFFSET           = 1;
const int EGRESS_DOUBLE_TAGGED_OFFSET           = 2;

/* Flags for trill initialization */
const int VLAN_TRANSLATION_TRILL_ACTION = (1 << 20);
const int VLAN_TRANSLATION_TRILL_TPID_ACTION = (1 << 19);
const int TRILL_SINGLE_VLAN_ACTION      = 4;
const int TRILL_DOUBLE_VLAN_ACTION      = 10;
const int TRILL_TTS_VLAN_ACTION         = 16;
const int TRILL_SINGLE_VLAN_PROFILE     = 4;
const int TRILL_DOUBLE_VLAN_PROFILE     = 5;
const int TRILL_TTS_VLAN_PROFILE        = 6;
const int TRILL_OT_OFFSET               = 0;
const int TRILL_DT_OFFSET               = 1;
const int TRILL_IT_OFFSET               = 2;
const int TRILL_UT_OFFSET               = 3;


/* In the old vlan translation mode, some actions and mappings were configured automatically.
   This function creates these actions, so old cints can be called without consequences.
 
    Egress and ingress actions are created in their respective functions, and then the tags and profile
    are mapped to their appropriate actions.  
   */
int 
vlan_translation_default_mode_init(int unit){
    int rv;

    advanced_vlan_translation_mode = soc_property_get(unit , "bcm886xx_vlan_translate_mode",0);


    if (!advanced_vlan_translation_mode ) {
        return BCM_E_NONE;
    }

    /*** CREATE INGRESS ACTIONS */
    rv = creat_default_ingress_actions(unit);
    if (rv != BCM_E_NONE) {
        printf("Error, in create_default_ingress_actions\n");
        return rv;
    }

    /*** CREATE  EGRESS ACTIONS ***/

    rv = create_default_egress_actions(unit, 0);
    if (rv != BCM_E_NONE) {
        printf("Error, in create_default_eggress_actions\n");
        return rv;
    }

    printf("Done configuring default vlan translation settings for unit %d\n", unit);
    
    return rv;
}


/* 
    Two ingress actions are created:
    REMOVE_TAGS: To remove vlan tags from the packet
    REMOVE_OUTER_TAG: To remove only the outer tag.
 
    Then, For each profile, we map the relevant tag to its action.
*/
int 
creat_default_ingress_actions(int unit){
    int rv;
    int in_action_id;

    /* Ingress action 5: remove outer tag, preserve inner */
    in_action_id = INGRESS_REMOVE_OUTER_TAG_ACTION;
    rv = vlan_translation_create_mapping(unit, in_action_id, bcmVlanActionDelete, bcmVlanActionNone, -1, 1);
    if (rv != BCM_E_NONE) {
        printf("Error: vlan_translation_create_mapping INGRESS_REMOVE_OUTER_TAG_ACTION\n");
        return rv;
    }

    /* CREATE INGRESS ACTION 4: remove tags, mapped from edit profile 1 and all tag formats */
    in_action_id = INGRESS_REMOVE_TAGS_PROFILE;
    rv = vlan_translation_create_mapping(unit, in_action_id, bcmVlanActionDelete, bcmVlanActionDelete, in_action_id, 1);
    if (rv != BCM_E_NONE) {
        printf("Error: vlan_translation_create_mapping INGRESS_REMOVE_TAGS_PROFILE\n");
        return rv;
    }

    return rv;
}


/*
   9 Egress actions are created:
   For each type of packet tag format (untagged, single tagged or double tagged) we create three profiles:
        REMOVE_TAGS: to remove all vlan tags
        REMOVE_TAGS_PUSH_1: to remove all tags, and push a new one
        REMOVE_TAGS_PUSH_2: to remove all tags, and push two new ones
 
   Then, For each profile, we map the relevant tag to its action.
*/
int
create_default_egress_actions(int unit, int is_trill){
    int rv;
    int eg_action_id;
    int eg_profile;
    int trill_flag = (is_trill) ? VLAN_TRANSLATION_TRILL_ACTION : 0;


    /* First action type: remove tags */
    eg_profile = EGRESS_REMOVE_TAGS_PROFILE;

    /* Egress action 3: get single tagged, remove tags */
    eg_action_id = EGRESS_REMOVE_TAGS_PROFILE + EGRESS_SINGLE_TAGGED_OFFSET;
    eg_action_id |= trill_flag;
    rv = vlan_translation_create_mapping(unit, eg_action_id , bcmVlanActionDelete, bcmVlanActionNone, -1, 0);
    if (rv != BCM_E_NONE) {
        printf("Error: vlan_translation_create_mapping EGRESS_REMOVE_TAGS_PROFILE + EGRESS_SINGLE_TAGGED_OFFSET\n");
        return rv;
    }

    /* Egress action 4: get double tagged, remove tags */
    eg_action_id = EGRESS_REMOVE_TAGS_PROFILE + EGRESS_DOUBLE_TAGGED_OFFSET;
    eg_action_id |= trill_flag;
    rv = vlan_translation_create_mapping(unit, eg_action_id , bcmVlanActionDelete, bcmVlanActionDelete, -1, 0);
    if (rv != BCM_E_NONE) {
        printf("Error: vlan_translation_create_mapping EGRESS_REMOVE_TAGS_PROFILE + EGRESS_DOUBLE_TAGGED_OFFSET\n");
        return rv;
    }

    /* Egress action 2: get untagged, remove tags.
       Also, map all remove tags actions */
    eg_action_id = EGRESS_REMOVE_TAGS_PROFILE + EGRESS_UNTAGGED_OFFSET;
    eg_action_id |= trill_flag;
    rv = vlan_translation_create_mapping(unit, eg_action_id , bcmVlanActionNone, bcmVlanActionNone, eg_profile, 0);
    if (rv != BCM_E_NONE) {
        printf("Error: vlan_translation_create_mapping EGRESS_REMOVE_TAGS_PROFILE + EGRESS_UNTAGGED_OFFSET\n");
        return rv;
    }


    /* Second action type: remove tags, push 1 */
    eg_profile = EGRESS_REMOVE_TAGS_PUSH_1_PROFILE;

    /* Egress action 6: get single tagged, remove tags push 1 */
    eg_action_id = EGRESS_REMOVE_TAGS_PUSH_1_PROFILE + EGRESS_SINGLE_TAGGED_OFFSET;
    eg_action_id |= trill_flag;
    rv = vlan_translation_create_mapping(unit, eg_action_id , bcmVlanActionReplace, bcmVlanActionNone, -1, 0);
    if (rv != BCM_E_NONE) {
        printf("Error: vlan_translation_create_mapping EGRESS_REMOVE_TAGS_PUSH_1_PROFILE + EGRESS_SINGLE_TAGGED_OFFSET\n");
        return rv;
    }

    /* Egress action 7: get double tagged, remove tags push 1 */
    eg_action_id = EGRESS_REMOVE_TAGS_PUSH_1_PROFILE + EGRESS_DOUBLE_TAGGED_OFFSET;
    eg_action_id |= trill_flag;
    rv = vlan_translation_create_mapping(unit, eg_action_id , bcmVlanActionReplace, bcmVlanActionDelete, -1, 0);
    if (rv != BCM_E_NONE) {
        printf("Error: vlan_translation_create_mapping EGRESS_REMOVE_TAGS_PUSH_1_PROFILE + EGRESS_DOUBLE_TAGGED_OFFSET\n");
        return rv;
    }


    /* Egress action 5: get untagged, remove tags push 1
       Also, map all remove tags push 1 actions */
    eg_action_id = EGRESS_REMOVE_TAGS_PUSH_1_PROFILE + EGRESS_UNTAGGED_OFFSET;
    eg_action_id |= trill_flag;
    rv = vlan_translation_create_mapping(unit, eg_action_id , bcmVlanActionAdd, bcmVlanActionNone, eg_profile, 0);
    if (rv != BCM_E_NONE) {
        printf("Error: vlan_translation_create_mapping EGRESS_REMOVE_TAGS_PUSH_1_PROFILE + EGRESS_UNTAGGED_OFFSET\n");
        return rv;
    }


    /* Third action type: remove tags, push 2 */
    eg_profile = EGRESS_REMOVE_TAGS_PUSH_2_PROFILE;

    /* Egress action 6: get single tagged, remove tags push 2 */
    eg_action_id = EGRESS_REMOVE_TAGS_PUSH_2_PROFILE + EGRESS_SINGLE_TAGGED_OFFSET;
    eg_action_id |= trill_flag;
    rv = vlan_translation_create_mapping(unit, eg_action_id , bcmVlanActionReplace, bcmVlanActionAdd, -1 , 0);
    if (rv != BCM_E_NONE) {
        printf("Error: vlan_translation_create_mapping EGRESS_REMOVE_TAGS_PUSH_2_PROFILE + EGRESS_SINGLE_TAGGED_OFFSET\n");
        return rv;
    }


    /* Egress action 7: get double tagged, remove tags push 2 */
    eg_action_id = EGRESS_REMOVE_TAGS_PUSH_2_PROFILE + EGRESS_DOUBLE_TAGGED_OFFSET;
    eg_action_id |= trill_flag;
    rv = vlan_translation_create_mapping(unit, eg_action_id , bcmVlanActionReplace, bcmVlanActionReplace, -1, 0);
    if (rv != BCM_E_NONE) {
        printf("Error: vlan_translation_create_mapping EGRESS_REMOVE_TAGS_PUSH_2_PROFILE + EGRESS_DOUBLE_TAGGED_OFFSET\n");
        return rv;
    }


    /* Egress action 5: get untagged, remove tags push 2
       Also, map all remove tags push 1 actions */
    eg_action_id = EGRESS_REMOVE_TAGS_PUSH_2_PROFILE + EGRESS_UNTAGGED_OFFSET;
    eg_action_id |= trill_flag;
    rv = vlan_translation_create_mapping(unit, eg_action_id , bcmVlanActionAdd, bcmVlanActionAdd, eg_profile, 0);
    if (rv != BCM_E_NONE) {
        printf("Error: vlan_translation_create_mapping EGRESS_REMOVE_TAGS_PUSH_2_PROFILE + EGRESS_UNTAGGED_OFFSET\n");
        return rv;
    }

    return rv;
}


/*
 * Creates an action with the given id and configures it. 
 * If given an edit profile, map it to this action + OFFSET if it's an egress action.
 */
int
vlan_translation_create_mapping(int unit, int action_id, bcm_vlan_action_t outer_vid_source, bcm_vlan_action_t inner_vid_source, 
                                int edit_profile, uint8 is_ingress){

    int flags;
    int rv;
    char *ineg = (is_ingress) ? "ingress" : "egress";
    bcm_vlan_translate_action_class_t action_class;
    int tag_format;
    uint8 is_trill_vl_tpid = (action_id & VLAN_TRANSLATION_TRILL_TPID_ACTION) ? 1 : 0;
    uint8 is_trill = (action_id & VLAN_TRANSLATION_TRILL_ACTION) ? 1 : 0;
    /* 0x893B: native outer tpid for trill fgl. The vlan translation is done at IVE (is_ingress param), before trill encapsulation */
    uint16 outer_tpid = (is_trill && is_ingress && !is_trill_vl_tpid) ? 0x893B : 0x8100;
    uint16 inner_tpid = (is_trill) ? 0x893B : 0x9100;

    /* Remove the flags from the action id */
    action_id &= 0xff;

    /* Create new Action IDs */
    flags = (is_ingress) ? BCM_VLAN_ACTION_SET_INGRESS : BCM_VLAN_ACTION_SET_EGRESS;
    flags |= BCM_VLAN_ACTION_SET_WITH_ID;  
    rv = bcm_vlan_translate_action_id_create(unit, flags, &action_id);
    if (rv != BCM_E_NONE) {
        printf("Error, bcm_vlan_translate_action_id_create\n");
        return rv;
    }

    if (verbose) {
        printf("Created %s action id %d\n", ineg, action_id);
    }

    flags = (is_ingress) ? BCM_VLAN_ACTION_SET_INGRESS : BCM_VLAN_ACTION_SET_EGRESS;
    rv = vlan_translation_action_add(unit, action_id, flags, outer_vid_source, inner_vid_source, outer_tpid, inner_tpid);
    if (rv != BCM_E_NONE) {
        printf("Error, vlan_translation_action_add\n");
        return rv;
    }

    if (verbose) {
        printf("Created %s translation action %d\n", ineg, action_id);
    }

    /* In case we don't want to create mapping, return here. */
    if (edit_profile < 0) {
        return rv;
    }

    /* Map edit profile and all tag formats to command */
    for (tag_format = 0 ; tag_format < 16 ; tag_format++) {
        bcm_vlan_translate_action_class_t_init(&action_class);
        action_class.vlan_edit_class_id = edit_profile;
        action_class.tag_format_class_id = tag_format;

        if (is_ingress) {
            if (is_trill) {
                action_class.vlan_translation_action_id = (tag_format == 2) ? action_id + TRILL_OT_OFFSET
                                                            : (tag_format == 3) ? action_id + TRILL_IT_OFFSET
                                                            : (tag_format == 6) ? action_id + TRILL_DT_OFFSET
                                                            :  action_id;
            } else {
                action_class.vlan_translation_action_id	= (tag_format == 0) ? 0 /* Tag format 0 should be mapped to ingress action NOP */ 
                                            /* else if */  : (tag_format == 2) ? INGRESS_REMOVE_OUTER_TAG_ACTION /* Tag format 2 (one-tag) should be mapped to REMOVE_OUTER_TAG */
                                            /* else */     :  action_id;
            }
        } else {
            if (is_trill) {
                action_class.vlan_translation_action_id = (tag_format == 2) ? action_id + EGRESS_SINGLE_TAGGED_OFFSET
                                                            : (tag_format == 3) ? action_id + EGRESS_DOUBLE_TAGGED_OFFSET
                                                            : (tag_format == 6) ? action_id + EGRESS_DOUBLE_TAGGED_OFFSET
                                                            :  action_id;
            } else {
                action_class.vlan_translation_action_id = (tag_format == 2) ? action_id + EGRESS_SINGLE_TAGGED_OFFSET
                                                            : (tag_format == 6) ? action_id + EGRESS_DOUBLE_TAGGED_OFFSET
                                                            :  action_id;
            }
        }

        action_class.flags = flags; 
        rv = bcm_vlan_translate_action_class_set(unit,  &action_class);
        if (rv != BCM_E_NONE) {
            printf("Error, bcm_vlan_translate_action_class_set\n");
            return rv;
        }
    }

    if (verbose) {
        printf("Mapped %s edit profile %d and all tag_format to %s action %d\n", ineg, edit_profile, ineg, action_id);
    }

    return rv;
}

/* Destroys all ingress/ egress actions. */
int
vlan_translation_default_mode_destroy(int unit){
    int rv = BCM_E_NONE;

    if (advanced_vlan_translation_mode) {
        rv = bcm_vlan_translate_action_id_destroy_all(unit, BCM_VLAN_ACTION_SET_INGRESS);
        if (rv != BCM_E_NONE) {
            printf("Error: bcm_vlan_translate_action_id_destroy_all: Failed to clean ingress action ids\n");
        }
        rv = bcm_vlan_translate_action_id_destroy_all(unit, BCM_VLAN_ACTION_SET_EGRESS);
        if (rv != BCM_E_NONE) {
            printf("Error: bcm_vlan_translate_action_id_destroy_all: Failed to clean ingress action ids\n");
        }
    }
    return rv;
}

/* When calling bcm_vlan_port_create in the old vlan translation mode, a default mapping would have been created.
   Call this function with the vlan port you used when calling bcm_vlan_port_create to create this mapping */
int
vlan_translation_vlan_port_create_to_translation(int unit, bcm_vlan_port_t *vlan_port){
    int rv;
    int port_id = vlan_port->vlan_port_id;
    bcm_vlan_t match_vlan = vlan_port->match_vlan;
    bcm_vlan_t match_inner_vlan = vlan_port->match_inner_vlan;
    bcm_vlan_t egress_vlan = vlan_port->egress_vlan;
    bcm_vlan_t egress_inner_vlan = vlan_port->egress_inner_vlan;
    int ingress_edit_profile = INGRESS_REMOVE_TAGS_PROFILE; 
    int egress_edit_profile; 


    /* Determine egress action */
    if ((vlan_port->egress_inner_vlan == BCM_VLAN_INVALID && vlan_port->egress_vlan != BCM_VLAN_INVALID) || 
            vlan_port->criteria == BCM_VLAN_PORT_MATCH_PORT_VLAN || 
            vlan_port->criteria == BCM_VLAN_PORT_MATCH_PORT_INITIAL_VLAN) {
        egress_edit_profile = EGRESS_REMOVE_TAGS_PUSH_1_PROFILE;
    } else if ((vlan_port->egress_inner_vlan == BCM_VLAN_INVALID && vlan_port->egress_vlan == BCM_VLAN_INVALID) ||
               vlan_port->criteria == BCM_VLAN_PORT_MATCH_PORT) {
        egress_edit_profile = EGRESS_REMOVE_TAGS_PROFILE;
    } else if (vlan_port->egress_vlan == 0 && vlan_port->egress_inner_vlan == 0 &&
               (vlan_port->flags & BCM_VLAN_PORT_INNER_VLAN_PRESERVE) &&
               (vlan_port->flags & BCM_VLAN_PORT_OUTER_VLAN_PRESERVE)){
        egress_edit_profile = EGRESS_NOP_PROFILE; 
        ingress_edit_profile = INGRESS_NOP_PROFILE; /* Change ingress mapping as well in this case (to NOP) */
    } else {
        egress_edit_profile = EGRESS_REMOVE_TAGS_PUSH_2_PROFILE;
    }

    
    /* Configre ingress editing */
    rv = vlan_port_translation_set(unit, match_vlan, match_inner_vlan, port_id, ingress_edit_profile, 1);
    if (rv != BCM_E_NONE) {
        printf("Error, vlan_port_translation_set, ingress profile\n");
        return rv;
    }

    /* Configure egress editing*/
    rv = vlan_port_translation_set(unit, egress_vlan, egress_inner_vlan, port_id, egress_edit_profile, 0);
    if (rv != BCM_E_NONE) {
        printf("Error, vlan_port_translation_set, egress profile\n");
        return rv;
    }

    return rv;
}

/* Note: QOS mapping in this context is performed as part of vlan translation, mainly pcp manilpulation.
 *  We have three functions dedicated for this:
 *  set_qos_default_settings(): sets the port default priority for an untagged packet.
 *  add_qos_mapping(): Define a mapping and a mapping type; Define the mapping inputs, either inner or outer tag. 
 *                     For instance: we map an incoming outer-tag pcp to a new outer-tag pcp.
 *  set_qos_mapping(): Attach the mapping to a {port X vlan} object ,as part of the vlan translation setting.
 *  
 *  After performing a mapping, we need to set the (vlan translation)action->priority field to be the QOS profile mapping id in case of action Add/Replace.
 */


/* First, we set the port default priority for an untagged packet.
 * Second, we set a global drop precedence default assignment. 
 * parameters: 1. port: The port upon which we set the default priority. 
 *             2. priority: This is the port default priority.
 *             3. color: This value is used as a global drop precedence. 
 */
int set_qos_default_settings(int unit, bcm_port_t port, int priority, int color){

    int rv = BCM_E_NONE;

    rv = bcm_port_untagged_priority_set(unit, port, priority);
    if (rv != BCM_E_NONE) {
        printf("Error, bcm_port_untagged_priority_set\n");
        return rv;
    }

    rv = bcm_port_control_set(unit, -1, bcmPortControlDropPrecedence, color);
    if (rv != BCM_E_NONE) {
        printf("Error, bcm_port_control_set\n");
        return rv;
    }

    return rv;
}


/*
 * This function performs two steps: 
 * 1. (optional) Allocate a QOS map id.  
 * 2. Add mapping entries. 
 * parameters: 1. creation_flags: In our context we want to pass:  flags == (BCM_QOS_MAP_INGRESS or BCM_QOS_MAP_EGRESS)| BCM_QOS_MAP_L2_VLAN_PCP; 
 *             2. addition_flags: pass $::BCM_QOS_MAP_L2_OUTER_TAG for outer tag, BCM_QOS_MAP_L2_INNER_TAG for inner tag,
 *                BCM_QOS_MAP_L2_UNTAGGED for untagged.
 *             3. map_id: the id given in  bcm_qos_map_create. If we only want to add a mapping with no create, pass a value after performing create.
 *             4. map_struct: contains information relevant for the mapping. This will hold the actual mapping between the old pcp to the new one.
 *             5. color: a value that is mapped to a certain priority profile.
 *             6. internal_priority: pass the old pcp value (inner or outer tag pcp).
 *             7. packet_priority: pass the new pcp value.
 *             8. with_create == 1: create a mapping id; with_create==0: Don;t create a mapping id. bcm_qos_map_add will be performed
 *                assuming a valid mapping id has been passed.
 */
/* int add_qos_mapping(int unit , uint32 creation_flags, uint32 addition_flags, int *map_id, bcm_qos_map_t *map_struct, bcm_color_t color,  uint8 internal_priority, int packet_priority, int with_create){*/
int add_qos_mapping(int unit , uint32 creation_flags, uint32 addition_flags, bcm_color_t color,  uint8 internal_priority, int packet_priority, int qos_map_id, int with_create){

    int rv = BCM_E_NONE;
    bcm_qos_map_t map_struct;

    bcm_qos_map_t_init(&map_struct);

    if (with_create) {
        rv = bcm_qos_map_create(unit, creation_flags, &qos_map_id);
        if (rv != BCM_E_NONE) {
            printf("Error, bcm_qos_map_create\n");
            return rv;
        }
    }

    if (creation_flags & BCM_QOS_MAP_INGRESS) {
        ingress_qos_map_id = qos_map_id;
    }
    else{
        egress_qos_map_id = qos_map_id;
    }


    map_struct.int_pri = internal_priority;
    map_struct.pkt_pri = packet_priority;
    map_struct.pkt_cfi = 0;
    if (color > 0) {
        map_struct.color = color;
    }
    
    addition_flags |= (BCM_QOS_MAP_L2 | BCM_QOS_MAP_L2_VLAN_PCP);
    
    rv = bcm_qos_map_add(unit, addition_flags, map_struct, qos_map_id);
    if (rv != BCM_E_NONE) {
        printf("Error, bcm_qos_map_create\n");
        return rv;
    }

    return rv;
}


/*
 * After creating  mapping entries, we want to attach them to a vlan_port. 
 * parameters: 1. port: vlan_port id of the vlan_port to which this mapping is attached.
 *             2. ingress_map: ingress map id.
 *             3. egress_map: egress map id.
 */
int set_qos_mapping(int unit, bcm_gport_t port, int ingress_map, int egress_map){

    int rv = BCM_E_NONE;

    rv = bcm_qos_port_map_set(unit, port, ingress_map, egress_map);
    if (rv != BCM_E_NONE) {
        printf("Error, bcm_qos_port_map_set\n");
        return rv;
    }

    return rv;
}

int get_ingress_qos_map_id(){
    return ingress_qos_map_id;
}

int get_egress_qos_map_id(){
    return egress_qos_map_id;
}

/* Sets qos mapping in AVT mode */
int set_qos_mode() {
	int rv = BCM_E_NONE;

    qos_mode = 1;

	return rv;
}

int ive_eve_translation_set(int unit,
                        bcm_gport_t lif,
                        int outer_tpid,
                        int inner_tpid,
                        bcm_vlan_action_t outer_action,
                        bcm_vlan_action_t inner_action,
                        bcm_vlan_t new_outer_vid,
                        bcm_vlan_t new_inner_vid,
                        uint32 vlan_edit_profile,
                        uint16 tag_format, 
                        uint8 is_ive)
{
    bcm_vlan_action_set_t action;
    bcm_vlan_translate_action_class_t action_class;
    bcm_vlan_port_translation_t port_trans;
    int action_id_1, rv;

    /* set edit profile for  ingress/egress LIF */
    bcm_vlan_port_translation_t_init(&port_trans);
    port_trans.new_outer_vlan = new_outer_vid;
    port_trans.new_inner_vlan = new_inner_vid;
    port_trans.gport = lif;
    port_trans.vlan_edit_class_id = vlan_edit_profile;
    port_trans.flags = is_ive ? BCM_VLAN_ACTION_SET_INGRESS:BCM_VLAN_ACTION_SET_EGRESS;
    rv = bcm_vlan_port_translation_set(unit, &port_trans);
    if (rv != BCM_E_NONE) {
        printf("Error, bcm_vlan_port_translation_set\n");
        return rv;
    }

    /* Create action ID*/
    rv = bcm_vlan_translate_action_id_create( unit, (is_ive ? BCM_VLAN_ACTION_SET_INGRESS:BCM_VLAN_ACTION_SET_EGRESS), &action_id_1);
    if (rv != BCM_E_NONE) {
        printf("Error, bcm_vlan_translate_action_id_create\n");
        return rv;
    }

    /* Set translation action */
    bcm_vlan_action_set_t_init(&action);
    action.outer_tpid = outer_tpid;
    action.inner_tpid = inner_tpid;	
    action.dt_outer = outer_action;
    action.dt_inner = inner_action;
    rv = bcm_vlan_translate_action_id_set( unit,
                                            (is_ive ? BCM_VLAN_ACTION_SET_INGRESS:BCM_VLAN_ACTION_SET_EGRESS),
                                            action_id_1,
                                            &action);
    if (rv != BCM_E_NONE) {
        printf("Error, bcm_vlan_translate_action_id_set\n");
        return rv;
    }

    /* Set translation action class (map edit_profile & tag_format to action_id) */
    bcm_vlan_translate_action_class_t_init(&action_class);
    action_class.vlan_edit_class_id = vlan_edit_profile;
    action_class.tag_format_class_id = tag_format;
    action_class.vlan_translation_action_id	= action_id_1;
    action_class.flags = (is_ive ? BCM_VLAN_ACTION_SET_INGRESS:BCM_VLAN_ACTION_SET_EGRESS);
    rv = bcm_vlan_translate_action_class_set( unit,  &action_class);
    if (rv != BCM_E_NONE) {
        printf("Error, bcm_vlan_translate_action_class_set\n");
        return rv;
    }
    

    return BCM_E_NONE;
}
