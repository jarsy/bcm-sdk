/*~~~~~~~~~~~~~~~~~~~~~~~Auxilliary mpls related functions~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/* $Id: cint_utils_mpls.c,v 1.22 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File: cint_utils_mpls.c
 * Purpose: Provide mpls utility functions functions
 */

/* **************************************************************************************************
  --------------          Global Variables Definition and Initialization            -----------------
 *************************************************************************************************** */
/* Struct definitions */

struct mpls__egress_tunnel_utils_s
{
    int flags;
    int flags_out;
    int flags_out_2;
    int flags_out_3;
    int use_flags_out;
    int use_flags_out_2;
    int use_flags_out_3;
    int force_flags; /* Pass force_flags=1 for independent configuration; otherwise, flags will be configured automatically*/
    bcm_mpls_egress_action_t egress_action;
    bcm_mpls_label_t label_in;
    bcm_mpls_label_t label_out; /* 0 in case only one label is created */
    bcm_mpls_label_t label_out_2; /* 0 in case only 1-2 labels are created */
    bcm_mpls_label_t label_out_3; /* 0 in case only 1-3 labels are created */
    int next_pointer_intf;
    int tunnel_id; /* out parameter, created tunnel id */
    bcm_failover_t egress_failover_id;
    bcm_if_t egress_failover_if_id; 
    uint8 ttl;
    uint8 exp;
    uint8 ext_ttl;
    uint8 ext_exp;
    int with_exp; 
};

struct mpls__ingress_tunnel_utils_s
{
    int flags;
    bcm_mpls_label_t label; /* Incoming label value. */
    bcm_mpls_label_t second_label;  /* Incoming second label. */
    bcm_mpls_label_t swap_label; /* Label for egress, e.g. for swapping */
    bcm_mpls_switch_action_t action;    /* MPLS label action. */
    bcm_if_t tunnel_if; /* hierarchical interface, relevant for
                                           when action is
                                           BCM_MPLS_SWITCH_ACTION_POP. */
    bcm_vpn_t vpn; /* set this value only if action is pop */
    bcm_gport_t tunnel_id;              /* Tunnel ID. */
    bcm_failover_t failover_id; /* Failover id for protection */
    bcm_gport_t failover_tunnel_id; /* Failover Tunnel ID */ 
    bcm_if_t fec; /* FEC pointer */
};



/* Globals */
uint8 mpls_pipe_mode_exp_set = 0;
uint8 qax_egress_label_custom_flags = 0;

/* global exp value for a label*/
int mpls_exp = 0;
int ext_mpls_exp = 4;
int mpls_ttl = 20;
int ext_mpls_ttl = 60;
int mpls_default_egress_label_value = 0;
/* ****************************************************************************************************/

/*
 * Internal functions
 */

/* Init function for this file*/
int
mpls__init(int unit)
{
    int rv;

    rv = mpls__mpls_pipe_mode_exp_set(unit);
    if (rv != BCM_E_NONE) {
        printf("Error, in mpls__mpls_pipe_mode_exp_set\n");
        return rv;
    }

    return rv;
}

/* Sets mpls pipe mode exp*/
int mpls__mpls_pipe_mode_exp_set(int unit) {
    int rv = BCM_E_NONE;
    if (mpls_pipe_mode_exp_set) {
        rv = bcm_switch_control_set(unit, bcmSwitchMplsPipeTunnelLabelExpSet, mpls_pipe_mode_exp_set);
        if (rv != BCM_E_NONE) {
            printf("Error in bcm_switch_control_set\n");
            print rv;
            return rv;
        }
    }

    return rv;                
}


void init_outer_label(int unit, bcm_mpls_egress_label_t* label_conf,
                      mpls__egress_tunnel_utils_s *mpls_tunnel_properties,
                      bcm_mpls_label_t label, uint32 flags) {

    bcm_mpls_egress_label_t_init(label_conf);
    label_conf->exp = mpls_tunnel_properties->with_exp ? mpls_tunnel_properties->ext_exp : ext_mpls_exp;
    label_conf->ttl = mpls_tunnel_properties->ext_ttl ? mpls_tunnel_properties->ext_ttl : ext_mpls_ttl;

    if (mpls_tunnel_properties->force_flags) {
        label_conf->flags = flags;
    } else {
        label_conf->flags |= (BCM_MPLS_EGRESS_LABEL_TTL_SET | BCM_MPLS_EGRESS_LABEL_TTL_DECREMENT | flags);
    }
    label_conf->tunnel_id = mpls_tunnel_properties->tunnel_id;
    if (!is_device_or_above(unit,ARAD_PLUS) || mpls_pipe_mode_exp_set) {
        label_conf->flags |= BCM_MPLS_EGRESS_LABEL_EXP_SET;
    } else {
        label_conf->flags |= BCM_MPLS_EGRESS_LABEL_EXP_COPY;
    }
    label_conf->label = label;
    label_conf->l3_intf_id = mpls_tunnel_properties->next_pointer_intf;

    if (mpls_tunnel_properties->flags & BCM_MPLS_EGRESS_LABEL_PROTECTION) {
        label_conf->egress_failover_id = mpls_tunnel_properties->egress_failover_id;
        label_conf->egress_failover_if_id = mpls_tunnel_properties->egress_failover_if_id;
    }

}

/*
* remove BCM_MPLS_EGRESS_LABEL_ENTROPY_INDICATION_ENABLE from flags for jericho and below devices.
*/
void
unset_entropy_flag(int unit, uint32* flags)
{
    if(!is_device_or_above(unit,JERICHO_PLUS)) {
        /* for non jericho_plus - entropy flag can be placed only on BOS label */
        *flags &= ~BCM_MPLS_EGRESS_LABEL_ENTROPY_INDICATION_ENABLE;
    }
}

/*
 * Functions calling BCM apis
 */

/* set egress action over this l3 interface, so packet forwarded to this interface will be tunneled/swapped/popped */
int 
mpls__create_tunnel_initiator__set(int unit, mpls__egress_tunnel_utils_s *mpls_tunnel_properties) {
    bcm_mpls_egress_label_t label_array[4];
    int num_labels;
    int rv;

    rv = mpls__init(unit);
    if (rv != BCM_E_NONE) {
        printf("Error, in mpls__init\n");
        return rv;
    }

    bcm_mpls_egress_label_t_init(&label_array[0]);
    label_array[0].exp = mpls_tunnel_properties->with_exp ? mpls_tunnel_properties->exp : mpls_exp; 

    if(mpls_tunnel_properties->force_flags) {
        label_array[0].flags = mpls_tunnel_properties->flags;
    } else {
        label_array[0].flags = (mpls_tunnel_properties->flags|BCM_MPLS_EGRESS_LABEL_TTL_SET|BCM_MPLS_EGRESS_LABEL_TTL_DECREMENT);
    }
    if (!is_device_or_above(unit,ARAD_PLUS) || mpls_pipe_mode_exp_set) {
        label_array[0].flags |= BCM_MPLS_EGRESS_LABEL_EXP_SET;
    } else {
        label_array[0].flags |= BCM_MPLS_EGRESS_LABEL_EXP_COPY;
    }
    if (mpls_tunnel_properties->egress_action==BCM_MPLS_EGRESS_ACTION_PHP) {
      label_array[0].flags |= BCM_MPLS_EGRESS_LABEL_PHP_IPV4;
    }
	if (mpls_tunnel_properties->flags & BCM_MPLS_EGRESS_LABEL_ACTION_VALID) {
        label_array[0].flags |= BCM_MPLS_EGRESS_LABEL_ACTION_VALID;
        label_array[0].action = mpls_tunnel_properties->egress_action;
	}
	label_array[0].label = mpls_tunnel_properties->label_in;
    label_array[0].ttl = mpls_tunnel_properties->ttl ? mpls_tunnel_properties->ttl : mpls_ttl;
    label_array[0].l3_intf_id = mpls_tunnel_properties->next_pointer_intf;
    if (mpls_tunnel_properties->tunnel_id != 0) {
        label_array[0].flags |= BCM_MPLS_EGRESS_LABEL_WITH_ID;
        label_array[0].tunnel_id = mpls_tunnel_properties->tunnel_id;
    }
    num_labels = 1;

    if (mpls_tunnel_properties->flags & BCM_MPLS_EGRESS_LABEL_PROTECTION) {
        label_array[0].egress_failover_id = mpls_tunnel_properties->egress_failover_id;
        label_array[0].egress_failover_if_id = mpls_tunnel_properties->egress_failover_if_id;
    }

    if (mpls_tunnel_properties->label_out != mpls_default_egress_label_value) {

        uint32 flags = 0;

        flags = mpls_tunnel_properties->use_flags_out ? mpls_tunnel_properties->flags_out : mpls_tunnel_properties->flags;
        unset_entropy_flag(unit, &flags);

        init_outer_label(unit, &label_array[1], mpls_tunnel_properties, mpls_tunnel_properties->label_out,flags);
        num_labels++;

        if (mpls_tunnel_properties->label_out_2 != mpls_default_egress_label_value) {

            flags = mpls_tunnel_properties->use_flags_out_2 ? mpls_tunnel_properties->flags_out_2 : mpls_tunnel_properties->flags;
            unset_entropy_flag(unit, &flags);

            init_outer_label(unit, &label_array[2], mpls_tunnel_properties, mpls_tunnel_properties->label_out_2,flags);
            num_labels++;

            if (mpls_tunnel_properties->label_out_3 != mpls_default_egress_label_value) {

                flags = mpls_tunnel_properties->use_flags_out_3 ? mpls_tunnel_properties->flags_out_3 : mpls_tunnel_properties->flags;
                unset_entropy_flag(unit, &flags);

                init_outer_label(unit, &label_array[3], mpls_tunnel_properties, mpls_tunnel_properties->label_out_3,flags);
                num_labels++;

            }
        }
    }

    rv = bcm_mpls_tunnel_initiator_create(unit,0,num_labels,label_array);
    if (rv != BCM_E_NONE) {
        printf("Error, in bcm_mpls_tunnel_initiator_create\n");
        return rv;
    }

    mpls_tunnel_properties->tunnel_id = label_array[0].tunnel_id;

    return rv;
}


/* Add switch entry 
 */
int
mpls__add_switch_entry(int unit, mpls__ingress_tunnel_utils_s *mpls_tunnel_properties)
{
    int rv;
    bcm_mpls_tunnel_switch_t entry;
    
    bcm_mpls_tunnel_switch_t_init(&entry);
    entry.action = mpls_tunnel_properties->action;
    /* TTL decrement has to be present 
     * Uniform: inherit TTL and EXP, 
     * in general valid options: 
     * both present (uniform) or none of them (Pipe)
     */
    entry.flags = mpls_tunnel_properties->flags | BCM_MPLS_SWITCH_TTL_DECREMENT|BCM_MPLS_SWITCH_OUTER_TTL|BCM_MPLS_SWITCH_OUTER_EXP;

    /* incoming label */
    entry.label = mpls_tunnel_properties->label;
    entry.second_label = mpls_tunnel_properties->second_label;
    if (mpls_tunnel_properties->swap_label != 0) {
        entry.egress_label.label = mpls_tunnel_properties->swap_label;
    }
    entry.tunnel_id = mpls_tunnel_properties->tunnel_id;
    entry.failover_id = mpls_tunnel_properties->failover_id;
    entry.failover_tunnel_id = mpls_tunnel_properties->failover_tunnel_id;
    entry.egress_if = mpls_tunnel_properties->fec;

    /* populate vpn information */
    entry.vpn = mpls_tunnel_properties->vpn;
    
    rv = bcm_mpls_tunnel_switch_create(unit,&entry);
    if (rv != BCM_E_NONE) {
        printf("Error, in bcm_mpls_tunnel_switch_create\n");
        return rv;
    }

    mpls_tunnel_properties->tunnel_id = entry.tunnel_id;
    return rv;
}

