/* $Id: cint_egress_accpetable_frame_type.c,v 1.0 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
* File: cint_egress_accpetable_frame_type.c
* Purpose: Test egress frame type filter
*/



/* hepler function */
int is_tpid_invalid(uint32 tpid) {
    return (tpid == BCM_PORT_TPID_CLASS_TPID_INVALID);
}



int set_trap_to_drop(int unit, bcm_rx_trap_t trap) {
    int rv = BCM_E_NONE;
    bcm_rx_trap_config_t config;
    int flags = 0;
    int trap_id;

    bcm_rx_trap_config_t_init(&config);
             
    config.flags = (BCM_RX_TRAP_UPDATE_DEST); 
    config.trap_strength = 7;
    config.dest_port=BCM_GPORT_BLACK_HOLE;

    rv = bcm_rx_trap_type_create(unit,flags,trap,&trap_id);
    if (rv != BCM_E_NONE) {
        printf("Error, in trap create, trap %d \n", trap_id);
        return rv;
    }

    rv = bcm_rx_trap_set(unit,trap_id,&config);
    if (rv != BCM_E_NONE) {
        printf("autoneg_port_pair_restart failed");
        return rv;
    }

    return rv;
}

/*
 * 1. use default bridge.
 * 2. add tpids to  ports 
 * 3. ports[0]: configure tpid class: accept all ethernet tags.
 *    ports[1]: configure tpid class: accept all ethernet tags.
 *              Except untags.
 * 4. add in mac t pointer to 14 for da
 * 5. add in mac t pointer to 13 for da
 * 6. send untag packet from 13 to 14
 * 7. send tag packet
 * 
 */
int egress_acceptable_frame_type_run(int unit, int out_port_1, int out_port_2, int enable_egress_trap) {
    int rv; 
    
    uint8 is_outer_prio;
    /* ports */
    int port_nbr = 2;
    bcm_port_t ports[port_nbr] = {out_port_1, out_port_2};
    int port_index;
    bcm_port_tpid_class_t tpid_class;

    /* tpids */
    int tpid_nbr = 2;
    uint16 tpids[tpid_nbr] = {0x8100, 0x9100};
    int tpid_index;

    /* tpids + special tpid value: untag */
    int port_tpid_class_tpids_nbr=tpid_nbr + 1;
    uint32 port_tpid_class_tpids[port_tpid_class_tpids_nbr] = {0x8100, 0x9100, BCM_PORT_TPID_CLASS_TPID_INVALID};
    int port_tpid_class_tpid_index1, port_tpid_class_tpid_index2;

    /* mac table variables */
    bcm_mac_t mac;
    int fid_untag = 1;
    int fid_tag = 20;

    /* temp shlomi, configure action id 0 to do nothing */
    bcm_vlan_action_set_t action;
    int action_id;

    for (port_index=0;port_index<port_nbr;port_index++) {
        bcm_port_tpid_delete_all(unit, ports[port_index]);
    }

    /* add tpids to ports */
    for (port_index=0;port_index<port_nbr;port_index++) {
        for (tpid_index=0; tpid_index<tpid_nbr;tpid_index++) {
            bcm_port_tpid_add(unit, ports[port_index], tpids[tpid_index], 0);
            if (rv != BCM_E_NONE) {
                printf("Error, in bcm_port_tpid_add, port=%d, \n", ports[port_index]);
                return rv;
            }
        }
    }
    /* setup tpid class for each port, for each allowed tpids */
    /* loop on tpid index for tpid1 from 0 - SOC_PPD_NOF_LLP_PARSE_tpid_index1S */
    for (port_tpid_class_tpid_index1=0; port_tpid_class_tpid_index1<port_tpid_class_tpids_nbr; port_tpid_class_tpid_index1++) {
        /* loop on tpid index for tpid2 from 0 - SOC_PPD_NOF_LLP_PARSE_tpid_index1S */
        for (port_tpid_class_tpid_index2=0; port_tpid_class_tpid_index2<port_tpid_class_tpids_nbr; port_tpid_class_tpid_index2++) {
            /* loop on priority bit */
            for (is_outer_prio=FALSE; is_outer_prio<TRUE; is_outer_prio++) {
                /* loop on each port */
                for (port_index=0;port_index<port_nbr;port_index++) {
                    /* configure tpid class for a tag structure */
                    bcm_port_tpid_class_t_init(&tpid_class);
                    /* tpid 1*/
                    tpid_class.tpid1=port_tpid_class_tpids[port_tpid_class_tpid_index1];
                    /* tpid 2*/
                    tpid_class.tpid2=port_tpid_class_tpids[port_tpid_class_tpid_index2];
                    /* priority */
                    if (is_outer_prio) {
                        tpid_class.flags |= BCM_PORT_TPID_CLASS_OUTER_IS_PRIO;
                    }
                    /*temp shlomi: overwrite EVE action id 0*/
                    bcm_vlan_action_set_t_init(&action);
                    action_id = 0;
                    action.dt_outer = bcmVlanActionNone;
                    action.dt_inner = bcmVlanActionNone;
                    action.dt_outer_pkt_prio = bcmVlanActionNone;
                    action.dt_inner_pkt_prio = bcmVlanActionNone;
                    rv = bcm_vlan_translate_action_id_set(unit, BCM_VLAN_ACTION_SET_EGRESS,
                                action_id, &action);
                    if (rv != BCM_E_NONE) {
                        printf("bcm_vlan_translate_action_id_set failed");
                        return rv;
                    }


                    /*tag format: if untag: 0 */
                    if (is_tpid_invalid(tpid_class.tpid1) && is_tpid_invalid(tpid_class.tpid2)) {
                        tpid_class.tag_format_class_id = 0;
                    } 
                    /* tag format: if 1 tag: 7 else 3 */
                    else if (!is_tpid_invalid(tpid_class.tpid1) && is_tpid_invalid(tpid_class.tpid2)) {
                        tpid_class.tag_format_class_id = 7;
                    } else {
                        tpid_class.tag_format_class_id = 3;
                    }
                    /* port */
                    tpid_class.port = ports[port_index];
                    /* if port[1] && untag, drop */
                    if (port_index == 1 && is_tpid_invalid(tpid_class.tpid1) && is_tpid_invalid(tpid_class.tpid2)) {
                        tpid_class.flags |= BCM_PORT_TPID_CLASS_DISCARD;
                    }


                    /* temp, test if tagged drop 
                    if (port_index == 1 && !is_tpid_invalid(tpid_class.tpid1) && is_tpid_invalid(tpid_class.tpid2)) {
                        tpid_class.flags |= BCM_PORT_TPID_CLASS_DISCARD;
                    } 
                    if (port_index == 1 && is_tpid_invalid(tpid_class.tpid1) && !is_tpid_invalid(tpid_class.tpid2)) {
                        tpid_class.flags |= BCM_PORT_TPID_CLASS_DISCARD;
                    } 
                    */

                    /* set tpid class for class structure*/
                    bcm_port_tpid_class_set(unit, tpid_class);

                }
            }
        }
    }

    /* set trap */
    if (enable_egress_trap) {
        set_trap_to_drop(unit, bcmRxTrapEgDiscardFrameTypeFilter);
    }

    /* add mact entries 
     * fid      mac                port
     *   1    00:00:00:00:02       13 (port[0])
     *  20    00:00:00:00:02       13 (port[0])
     *   1    00:00:00:00:03       14 (port[1])
     *  20    00:00:00:00:03       14 (port[1])
     */
    mac[0] = mac[1] = mac[2] = mac[3] = mac[4] = 0;

    mac[5] = 2;

    vlan_basic_bridge_add_l2_addr_to_gport(unit, ports[0], mac, fid_untag);
    vlan_basic_bridge_add_l2_addr_to_gport(unit, ports[0], mac, fid_tag);

    mac[5] = 3;

    vlan_basic_bridge_add_l2_addr_to_gport(unit, ports[1], mac, fid_untag);
    vlan_basic_bridge_add_l2_addr_to_gport(unit, ports[1], mac, fid_tag);

}
