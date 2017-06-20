/* $Id: cint_oam_egress_injection.c,v 1.10 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/

/**************************************************************************************************************************************************
 *
 * Purposes of this cint is demonstrate creating ETH-OAM endpoint's with VSI in order to count CCM packets together with data packets.
 *
 *
 * set following soc properties
 *   bcm886xx_vlan_translate_mode=1
 *   set of soc_properties that enable oam.
 *
 * This cint is creating 2 endpoints with VSI with two different OutLifs.
 * Each Lif have different Egress Vlan Editing profile(egress vlan editing is performed on such packets).
 * The cint demonstrate creating two diferent egress vlan editing profiles(one for each lif).
 * Both endpoints inject untagged packet and on egress side one or two tags added according vlan editing profile
 *
 * run:                                                                                                                                             *
 * cd ../../../../src/examples/dpp                                                                                                                  *
 * cint utility/cint_utils_global.c                                                                                                                 *
 * cint utility/cint_utils_vlan.c                                                                                                                   *
 * cint utility/cint_utils_port.c                                                                                                                   *
 * cint utility/cint_utils_l2.c                                                                                                                     *
 * cint utility/cint_utils_oam.c                                                                                                                    *
 * cint cint_oam_egress_injection.c                                                                                                                 *
 * cint                                                                                                                                             *
 * int unit = 0;                                                                                                                                    *
 * egress_injection__start_run(int unit,  egress_injection_s *param) - param could be NULL if you want to use default parameters                    *
 * OR                                                                                                                                               *
 * egress_injection_with_ports__start_run(int unit,  int in_port, int out_port) - if you want to change only ports(ingress/egress)                  *
 *                                                                                                                                                  *
 * This will create two endpoint that will start to send CCM packets.                                                                               *
 *                                                                                                                                                  *
 * All default values could be re-written with initialization of the global structure 'g_egress_injection', before calling the main  function       *
 * In order to re-write only part of values, call 'egress_injection_struct_get(egress_injection_s)' function and re-write values,                   *
 * prior calling the main function                                                                                                                  *
 */

/* **************************************************************************************************
  --------------          Global Variables Definition and Initialization  START     -----------------
 **************************************************************************************************** */
struct egress_injection_eve_s{
    vlan_edit_utils_s    egress_vlan_profile;
    vlan_action_utils_s  egress_vlan_action;
    bcm_vlan_t           outer_vlan;
    bcm_vlan_t           inner_vlan;
};

struct egress_injection_ep_s{
    bcm_oam_endpoint_t id;
    uint16 name;
    uint16 remote_name;
    int lm_counter;
    bcm_mac_t ep_dmac;
    bcm_mac_t ep_smac;
};

/* Main struct */
struct egress_injection_s{
    int sys_port[NUMBER_OF_PORTS];
    bcm_gport_t lif[2];
    int vsi;
    egress_injection_eve_s egress_vlan_edit[2];
    uint8 group_name[BCM_OAM_GROUP_NAME_LENGTH];
    uint8 group_name_48b[BCM_OAM_GROUP_NAME_LENGTH];
    int extra_group_data_index;
    egress_injection_ep_s  endpoint[2];
};

egress_injection_s g_egress_injection = {
/*************     Ports     **********************/
        /* PORT1 | PORT2 */
        { 200    , 201 },

/*************     Lif's    **********************/
        {-1, -1},
/*************     VSI      **********************/
        20,

/**********  Egress VLAN Editing   ***************/
        {
                {
                        { 10          ,     0       ,     2}, /* Edit Profile, Tag format, action_id */
                        { 2      ,   0x8100   , bcmVlanActionAdd   ,   0x8100   , bcmVlanActionAdd   },/* action */
                        100, /* Outer vlan*/
                        101  /* Inner vlan*/
                },
                {
                        {     11          ,     0       ,     3      },  /* Edit Profile, Tag format, action_id */
                        {   3      ,   0x8100   , bcmVlanActionAdd   ,   0        , bcmVlanActionNone   }, /* action */
                        202,  /* Outer vlan*/
                        0     /* Inner vlan*/
                }
        },
/**********        Endpoint         ***************/
        {1, 3, 2, 0xab, 0xcd},
        {0xa9, 0x23, 0x52, 0x8c, 0xd4, 0xb0, 0x18, 0x37,
         0xf4, 0x77, 0x41, 0x22, 0xe3, 0x70, 0xa4, 0x92,
         0x0f, 0x87, 0xff, 0x00, 0x9d, 0x42, 0x3b, 0x24,
         0x15, 0x3c, 0xd8, 0xa1, 0x43, 0x80, 0xb2, 0x62,
         0x5d, 0xa7, 0x66, 0xea, 0x27, 0x41, 0x93, 0x36,
         0x0d, 0x45, 0x9c, 0xab, 0x21, 0x79, 0x30, 0x4d},
         0x19bd, /* extra_group_data_index */
        {
            /* id | name | remote_name | lm_counter |   Destination MAC                   |    Source MAC */
            {-1   , 0x10 , 0xfd        , -1         , {0x00, 0x00, 0x00, 0x01, 0x01, 0x01}, {0x00, 0x00, 0x00, 0x11, 0x11, 0x11}},
            {-1   , 0x11 , 0xfd        , -1         , {0x00, 0x00, 0x00, 0x02, 0x02, 0x02}, {0x00, 0x00, 0x00, 0x22, 0x22, 0x22}}
        }
};

/* **************************************************************************************************
  --------------          Global  Variables Definitions and Initialization  END       ---------------
 **************************************************************************************************** */

/* Initialization of main struct
 * Function allow to re-write default values
 *
 * INPUT:
 *   params: new values for egress_injection_s
 */
int egress_injection_init(int unit, egress_injection_s *param){

    if (param != NULL) {
       sal_memcpy(&g_egress_injection, param, sizeof(g_egress_injection));
    }

    advanced_vlan_translation_mode = soc_property_get(unit , "bcm886xx_vlan_translate_mode",0);

    if (!advanced_vlan_translation_mode ) {
        return BCM_E_PARAM;
    }

    return BCM_E_NONE;
}

/*
 * Return egress_injection_s information
 */
void egress_injection__struct_get(int unit, egress_injection_s *param){

    sal_memcpy( param, &g_egress_injection, sizeof(g_egress_injection));

    return;
}

/* This function runs the main test function with given ports
 *
 * INPUT: unit     - unit
 *        in_port  - ingress port
 *        out_port - egress port
 */
int egress_injection_with_ports__start_run(int unit,  int port1, int port2){
    int rv;

    egress_injection_s param;

    egress_injection__struct_get(unit, &param);

    param.sys_port[0] = port1;
    param.sys_port[1] = port2;

    return egress_injection__start_run(unit, &param, 0);
}

/* Same function as above, only with a 48 byte MAID
 *
 * INPUT: unit     - unit
 *        in_port  - ingress port
 *        out_port - egress port
 */
int egress_injection_with_ports__start_run_48b_MAID(int unit,  int port1, int port2){
    int rv;

    egress_injection_s param;

    egress_injection__struct_get(unit, &param);

    param.sys_port[0] = port1;
    param.sys_port[1] = port2;

    return egress_injection__start_run(unit, &param, 1);
}

/*****************************************************************************/
/*                OAM  Egress Injection(QAX)                                 */
/*****************************************************************************/

/*
 * Main function runs the egress injection example
 *
 * Main steps of configuration:
 *    1. Creating VSI
 *    2. Setting VLAN domain for ports.
 *    3. Create LIF's for each port
 *    4. Call function that define Egress VLAN Translation
 *    5. Call function that creaates enpoints
 *
 * INPUT: unit  - unit
 *        param - new values for egress_injection(in case it's NULL default values will be used).
 */
int egress_injection__start_run(int unit, egress_injection_s *param, int maid_48b)
{
    int rv;
    int i;
    l2__mact_properties_s mact_properties;

    rv = egress_injection_init(unit, param);

    /* Create VSI */
    rv  = bcm_vlan_create(unit, g_egress_injection.vsi);
    if (rv != BCM_E_NONE ) {
        printf("Error, bcm_vlan_create unit %d, vid %d, rv %d\n", unit, g_egress_injection.vsi, rv);
        return rv;
    }

    /* set VLAN domain to each port */
    for(i=0; i<NUMBER_OF_PORTS; i++) {
        rv = port__vlan_domain__set(unit, g_egress_injection.sys_port[i], g_egress_injection.sys_port[i]);
        if (rv != BCM_E_NONE) {
            printf("Error, in port__vlan_domain__set, port=%d, \n",  g_egress_injection.sys_port[i]);
            return rv;
        }

        /* Set outer and inner tpid */
        rv = port__tpid__set(unit, g_egress_injection.sys_port[i], g_egress_injection.egress_vlan_edit[i].outer_vlan, g_egress_injection.egress_vlan_edit[i].inner_vlan);
        if (rv != BCM_E_NONE) {
            printf("Error, in port__tpid__set, port=%d, \n",   g_egress_injection.sys_port[i]);
            return rv;
        }

        /* Define Tag Format for in_port
         * Untagged packet get tag format - '0'
         * Other packets(one tag) get tag format - '1'
         * Other packets(double-tag) get tag format - '2'
         */
        rv = port__basic_tpid_class__set(unit,g_egress_injection.sys_port[i]);
        if (rv != BCM_E_NONE) {
            printf("Error, in port__basic_tpid_class__set, port=%d, \n",  g_egress_injection.sys_port[i]);
            return rv;
        }

        /* Create LIF's for Port */
        rv = l2__port__create(unit,0,  g_egress_injection.sys_port[i],
                                       0,
                                       g_egress_injection.lif[i]);
        if (rv != BCM_E_NONE) {
            printf("Error, l2__port_vlan__create\n");
            return rv;
        }

        rv = bcm_vswitch_port_add(unit, g_egress_injection.vsi, g_egress_injection.lif[i]);
        BCM_IF_ERROR_RETURN(rv);

        mact_properties.gport_id = g_egress_injection.sys_port[i];
        sal_memcpy(mact_properties.mac_address, g_egress_injection.endpoint[i].ep_smac, 6);
        mact_properties.vlan = g_egress_injection.vsi;
        rv = l2__mact_entry_create( unit, mact_properties);
        if (rv != BCM_E_NONE) {
            printf("Fail  l2__mact_entry_create ");
            return rv;
        }
    }

    /* Set Egress VLAN Translation */
    rv = egress_injection_egress_vlan_translation_set(unit);
    if (rv != BCM_E_NONE) {
        printf("Fail  egress_injection_egress_vlan_translation_set ");
        return rv;
    }

    rv = egress_injection_ep_create(unit, maid_48b);
    return rv;
}

int egress_injection_egress_vlan_translation_set(int unit)
{
    int rv=0;
    int i;

    for(i=0; i<2; i++) {
        /* Egress VLAN Translation */
        rv = vlan__port_translation__set(unit, g_egress_injection.egress_vlan_edit[i].outer_vlan,
                g_egress_injection.egress_vlan_edit[i].inner_vlan,
                g_egress_injection.lif[i],
                g_egress_injection.egress_vlan_edit[i].egress_vlan_profile.edit_profile,
                0);
        if (rv != BCM_E_NONE) {
            printf("Fail  vlan__port_translation__set ");
            return rv;
        }

        rv = vlan__translate_action_with_id__set(unit, g_egress_injection.egress_vlan_edit[i].egress_vlan_action.action_id,
                g_egress_injection.egress_vlan_edit[i].egress_vlan_action.o_tpid,
                g_egress_injection.egress_vlan_edit[i].egress_vlan_action.o_action,
                g_egress_injection.egress_vlan_edit[i].egress_vlan_action.i_tpid,
                g_egress_injection.egress_vlan_edit[i].egress_vlan_action.i_action,
                0);

        if (rv != BCM_E_NONE) {
            printf("Fail  vlan__translate_action_with_id__set for action %d ", i);
            return rv;
        }

        rv = vlan__translate_action_class__set(unit, g_egress_injection.egress_vlan_edit[i].egress_vlan_action.action_id,
                g_egress_injection.egress_vlan_edit[i].egress_vlan_profile.edit_profile,
                g_egress_injection.egress_vlan_edit[i].egress_vlan_profile.tag_format,
                0);


        if (rv != BCM_E_NONE) {
            printf("Fail  vlan__egress_translate_action_class__set vlan edit profile %d  ", i);
            return rv;
        }
    }

    return rv;
}
/* Function creating 2 ETH-OAM endpointss and enabling counting */
int egress_injection_ep_create(int unit, int maid_48b) {
    bcm_oam_group_info_t group_info;
    bcm_oam_endpoint_info_t  acc_endpoint;
    bcm_oam_endpoint_info_t  remote_ep;
    bcm_oam_endpoint_action_t action;
    int i;
    int rv;

    bcm_oam_group_info_t_init(&group_info);

    if (maid_48b) {
        group_info.flags = BCM_OAM_GROUP_FLEXIBLE_MAID_48_BYTE;
        group_info.group_name_index = g_egress_injection.extra_group_data_index;
        sal_memcpy(group_info.name, g_egress_injection.group_name_48b, BCM_OAM_GROUP_NAME_LENGTH);
    }
    else {
        sal_memcpy(group_info.name, g_egress_injection.group_name, BCM_OAM_GROUP_NAME_LENGTH);
    }
    rv = bcm_oam_group_create(unit, &group_info);
    if( rv != BCM_E_NONE) {
        printf("Fail  bcm_oam_group_create");
        return rv;
    }

    rv = set_counter_source_and_engines(unit, &g_egress_injection.endpoint[1].lm_counter, g_egress_injection.sys_port[0]);
    if (rv != BCM_E_NONE) {
         printf("Fail  set_counter_source_and_engines MEP 0  ");
         return rv;
     }
    rv = set_counter_source_and_engines(unit, &g_egress_injection.endpoint[0].lm_counter, g_egress_injection.sys_port[1]);
    if (rv != BCM_E_NONE) {
         printf("Fail  set_counter_source_and_engines MEP 1  ");
         return rv;
     }

    for(i=0; i<2; i++){
        /* Endpoint Create */
        bcm_oam_endpoint_info_t_init(&acc_endpoint);
        acc_endpoint.type = bcmOAMEndpointTypeEthernet;
        acc_endpoint.group = group_info.id;
        acc_endpoint.name = g_egress_injection.endpoint[i].name;
        acc_endpoint.opcode_flags |= BCM_OAM_OPCODE_CCM_IN_HW;
        acc_endpoint.ccm_period = BCM_OAM_ENDPOINT_CCM_PERIOD_100MS;
        acc_endpoint.level = 1;
        acc_endpoint.vpn = g_egress_injection.vsi;
        BCM_GPORT_SYSTEM_PORT_ID_SET(acc_endpoint.tx_gport, g_egress_injection.sys_port[i]);

        acc_endpoint.gport = g_egress_injection.lif[i];
        sal_memcpy(acc_endpoint.dst_mac_address, g_egress_injection.endpoint[i].ep_dmac, 6);
        sal_memcpy(acc_endpoint.src_mac_address, g_egress_injection.endpoint[i].ep_smac, 6);
        acc_endpoint.lm_counter_base_id = g_egress_injection.endpoint[i].lm_counter;
        rv = bcm_oam_endpoint_create(unit, &acc_endpoint);

        if (rv != BCM_E_NONE) {
            printf("Fail  bcm_oam_endpoint_create MEP %d  ", i);
            return rv;
        } else
            printf("bcm_oam_endpoint_create MEP %d\n",i);

        g_egress_injection.endpoint[i].id = acc_endpoint.id;

        /* Remote Endpoint Create */
        bcm_oam_endpoint_info_t_init(&remote_ep);
        remote_ep.name = g_egress_injection.endpoint[i].remote_name;
        remote_ep.local_id = acc_endpoint.id;
        remote_ep.type = bcmOAMEndpointTypeEthernet;
        remote_ep.ccm_period = 0;
        remote_ep.flags |= BCM_OAM_ENDPOINT_REMOTE;
        remote_ep.loc_clear_threshold = 1;
        remote_ep.flags |= BCM_OAM_ENDPOINT_WITH_ID;
        remote_ep.id = acc_endpoint.id;
        remote_ep.flags2 = BCM_OAM_ENDPOINT2_RDI_ON_RX_RDI | BCM_OAM_ENDPOINT2_RDI_ON_LOC;

        rv = bcm_oam_endpoint_create(unit, &remote_ep);
        if (rv != BCM_E_NONE) {
            printf("Fail  bcm_oam_endpoint_create RMEP %d  ", i);
            return rv;
        } else
            printf("bcm_oam_endpoint_create RMEP %d\n",i);
    }

    /* Counter Enable */
    bcm_oam_endpoint_action_t_init(&action);
    BCM_OAM_OPCODE_SET(action,1);
    BCM_OAM_ACTION_SET(action, bcmOAMActionCountEnable);
    bcm_oam_endpoint_action_set(unit, 0, &action);
    if (rv != BCM_E_NONE) {
        printf("Fail  bcm_oam_endpoint_action_set %d  ", i);
        return rv;
    }

    bcm_oam_endpoint_action_set(unit, 1, &action);
    if (rv != BCM_E_NONE) {
        printf("Fail  bcm_oam_endpoint_action_set %d  ", i);
        return rv;
    }
   return 0;
}

