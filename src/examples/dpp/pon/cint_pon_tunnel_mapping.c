/* $Id: cint_pon_tunnel_mapping.c,v 1.9 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
* 
* File: cint_pon_tunnel_mapping.c
* Purpose: An example of how to map tunnel_ID to PP port of PON application and set different properties (VTT lookup mode and VLAN range) per LLID profile.
*          The following CINT provides a calling sequence example to set the different tunnel_id to be mapped to different LLID profile and bring up one main PON services 1:1 Model.
*          ARAD ports are either network facing NNI ports (port numbers >= 128) or facing PON ports (port numbers 0-7).
*          Packets on the NNI ports are Ethernet packets, tagged with 0,1 or 2 VLAN tags identify the user and service (ONU), and class of service. 
*          Packets on the PON ports are Ethernet packets with an outermost VLAN-Tag that encodes the Logical Link Identification (LLID), i.e., service or ONU#. 
*          We refer to that tag as the Tunnel-ID (BCM APIs term) or PON-Channel ID (Arch term). The Tunnel-ID is prepended to the VLAN-Tag stack (as the outermost tag). 
*          Tunnel-ID it always present on packets on PON ports, which may have C-Tag and S-Tag as well. 
*          When packet is forwarded downstream from NNI-Port to PON-Ports, the incoming VLAN-header from the NNI port may be retained, stripped, or modified, and a Tunnel-ID is inserted.
*          When packet are forwarded upstream from PON-Port to a NNI-Ports, the Incoming VLAN-header from the PON port may be retained, stripped, or modified, and the Tunnel-ID is stripped.
*
* Calling sequence:
*
* Initialization:
*  1. Add the following port configureations to config-sand.bcm
*        ucode_port_129.BCM88650=10GBase-R15
*        ucode_port_5.BCM88650=10GBase-R14
*  2. Add the following PON application enabling configureations to config-sand.bcm
*        pon_application_support_enabled_5.BCM88650=TRUE
*        vlan_match_criteria_mode=PON_PCP_ETHERTYPE
*  3. Create PON PP port with different PON tunnel profile id 0.
*        - call bcm_port_pon_tunnel_add()
*  4. Map tunnel id 1000 to PON PP port 5 (indirectly mapped to PON tunnel profile id 0).
*        - call bcm_port_pon_tunnel_map_set()
*  5. Set TPIDs of PON pp port 5.
*        - call bcm_port_tpid_delete_all()
*        - call bcm_port_tpid_add()
*        - call bcm_port_inner_tpid_set()
*  6. Set VLAN domain of PON pp port 5 to 5.
*        - call bcm_port_class_set()
*  7. Enable additional port tunnel lookup in PON PP port 5.
*        - call bcm_vlan_control_port_set() with bcmVlanPortLookupTunnelEnable
*  8. Create PON PP port with different PON tunnel profile id 1.
*        - call bcm_port_pon_tunnel_add()
*  9. Map tunnel id 1001 to PON PP port 13 (indirectly mapped to PON tunnel profile id 1).
*        - call bcm_port_pon_tunnel_map_set()
*  10. Set TPIDs of PON pp port 13.
*        - call bcm_port_tpid_delete_all()
*        - call bcm_port_tpid_add()
*        - call bcm_port_inner_tpid_set()
*  11. Set VLAN domain of PON pp port 13 to 13.
*        - call bcm_port_class_set()
*  12. Enable ingore inner VLAN tag lookup in PON PP port 13.
*        - call bcm_vlan_control_port_set() with bcmVlanPortIgnoreInnerPktTag
*  13. Set TPIDs of NNI port 129.
*        - call bcm_port_tpid_delete_all()
*        - call bcm_port_tpid_add()
*        - call bcm_port_inner_tpid_set()
*  14. Remove Ports from VLAN 1.
*        - call bcm_vlan_gport_delete_all()
*  15. Disable membership in PON ports (SDK initialization already disabled membership in NNI ports).
*        - call bcm_port_vlan_member_set()
*
* 1:1 Service:
* Set up sequence:
*  1. Add VLAN range info to PON PP port if necessary:
*        - Call bcm_vlan_translate_action_range_add() with action bcmVlanActionCompressed.
*  2. Create PON LIF
*        - Call bcm_vlan_port_create() with following criterias:
*          BCM_VLAN_PORT_MATCH_PORT_TUNNEL, BCM_VLAN_PORT_MATCH_PORT_TUNNEL_VLAN
*  3. Set PON LIF ingress VLAN editor.
*        - Call bcm_vlan_translate_action_create()
*  4. Set PON LIF egress VLAN editor.
*        - Call bcm_vlan_translate_egress_action_add()
*  5. Create NNI LIF
*        - Call bcm_vlan_port_create()with following criterias:
*          BCM_VLAN_PORT_MATCH_PORT, BCM_VLAN_PORT_MATCH_PORT_VLAN, BCM_VLAN_PORT_MATCH_PORT_VLAN_STACKED
*  6. Cross connect the 2 LIFs
*        - Call bcm_vswitch_cross_connect_add()
* Clean up sequence:
*  1. Delete the cross connected LIFs.
*        - Call bcm_vswitch_cross_connect_delete()
*  2. Delete VLAN range info from PON PP port if necessary.
*        - Call bcm_vlan_translate_action_range_delete()       
*  3. Delete PON LIFs.
*        - Call bcm_vlan_port_destroy()
*  4. Delete NNI LIFs.
*        - Call bcm_vlan_port_destroy()
*
* Service Model:
* 1:1 Service:
*     N(1) PON Port 5  <-----------------------------  CrossConnect  ------------> 1 NNI Port 129
*     Tunnel-ID 1000 SVLAN 20 PON_PP_PORT 5 <------------------------------------> SVLAN 200
*     Tunnel-ID 1000 SVLAN 20 CVLAN 21 PON_PP_PORT 5 <------------------------_--> SVLAN 20 CVLAN 21
*     Tunnel-ID 1000 SVLAN range 50~60  <----------------------------------------> SVLAN 70
*     Tunnel-ID 1001 SVLAN 30 PON_PP_PORT 13 <-----------------------------------> SVLAN 300
*     Tunnel-ID 1001 SVLAN 30 CVLAN 31 PON_PP_PORT 13 <--------------------------> SVLAN 30 CVLAN 31
*     Tunnel-ID 1001 SVLAN range 80~90  <----------------------------------------> SVLAN 100
*     Tunnel-ID 1002 CVLAN 40 PON_PP_PORT 5 <------------------------------------> SVLAN 400
*
* Traffic:
*  1. PON Port 5 Tunnel-ID 1000 SVLAN 20 <-CrossConnect-> NNI Port 129 SVLAN 200
*        - From PON port:
*              -   ethernet header with any DA, SA 
*              -   Tunnel ID:1000
*              -   VLAN tag: VLAN tag type 0x8100, VID = 20
*        - From NNI port:
*              -   ethernet header with any DA, SA 
*              -   VLAN tag: VLAN tag type 0x8100, VID = 200
*
*  2. PON Port 5 Tunnel-ID 1000 SVLAN 20 CVLAN 21 <-CrossConnect-> NNI Port 129 SVLAN 20 CVLAN 21
*        - From PON port:
*              -   ethernet header with any DA, SA 
*              -   Tunnel ID:1000
*              -   VLAN tag: VLAN tag type 0x8100, VID = 20, VLAN tag type 0x9100, VID = 21
*        - From NNI port:
*              -   ethernet header with any DA, SA 
*              -   VLAN tag: VLAN tag type 0x8100, VID = 20, VLAN tag type 0x9100, VID = 21
*  3. PON Port 5 Tunnel-ID 1000 SVLAN range 50~60 <-CrossConnect-> NNI Port 129 SVLAN 70
*        - From PON port:
*              -   ethernet header with any DA, SA 
*              -   Tunnel ID:1000
*              -   VLAN tag: VLAN tag type 0x8100, VID = 55
*        - From NNI port:
*              -   ethernet header with any DA, SA 
*              -   VLAN tag: VLAN tag type 0x8100, VID = 70
*
*  4. PON Port 5 Tunnel-ID 1001 SVLAN 30 <-CrossConnect-> NNI Port 129 SVLAN 300
*        - From PON port:
*              -   ethernet header with any DA, SA 
*              -   Tunnel ID:1001
*              -   VLAN tag: VLAN tag type 0x8100, VID = 30
*        - From NNI port:
*              -   ethernet header with any DA, SA 
*              -   VLAN tag: VLAN tag type 0x8100, VID = 300
*
*  5. PON Port 5 Tunnel-ID 1001 SVLAN 30 CVLAN 31 <-CrossConnect-> NNI Port 129 SVLAN 30 CVLAN 31
*    - Upstream is matched by Port 5 Tunnel-ID 1001 SVLAN 30 --CrossConnect--> NNI Port 129 SVLAN 300, casue LLID profile ingore 2nd VLAN tag.
*    - Packets received in NNI port have outer VLAN 300.
*        - From PON port:
*              -   ethernet header with any DA, SA 
*              -   Tunnel ID:1001
*              -   VLAN tag: VLAN tag type 0x8100, VID = 30, VLAN tag type 0x9100, VID = 31
*    - Downstrean is macthed by NNI Port 129 SVLAN 30 CVLAN 31 --CrossConnect--> Port 5 Tunnel-ID 1001 SVLAN 30 CVLAN 31.
*    - Packets received in PON port have outer VLAN 30, inner VLAN 31.
*        - From NNI port:
*              -   ethernet header with any DA, SA 
*              -   VLAN tag: VLAN tag type 0x8100, VID = 30, VLAN tag type 0x9100, VID = 31
*  6. PON Port 5 Tunnel-ID 1001 SVLAN range 80~90 <-CrossConnect-> NNI Port 129 SVLAN 100
*        - From PON port:
*              -   ethernet header with any DA, SA 
*              -   Tunnel ID:1001
*              -   VLAN tag: VLAN tag type 0x8100, VID = 85
*        - From NNI port:
*              -   ethernet header with any DA, SA 
*              -   VLAN tag: VLAN tag type 0x8100, VID = 100
*
*  7. PON Port 5 Tunnel-ID 1002 SVLAN 40 <-CrossConnect-> NNI Port 129 SVLAN 400
*        - From PON port:
*              -   ethernet header with any DA, SA 
*              -   Tunnel ID:1002
*              -   VLAN tag: VLAN tag type 0x8100, VID = 40
*
* To Activate Above Settings Run:
*      BCM> cint examples/dpp/cint_port_tpid.c
*      BCM> cint examples/dpp/cint_pon_application.c
*      BCM> cint
*      cint> pon_tunnel_mapping_init(unit, pon_port, nni_port);
*      cint> pon_tunnel_mapping_1_1_service(unit);
*      cint> pon_tunnel_mapping_1_1_service_cleanup(unit);
*/

enum pon_service_mode_e {
    stag_to_stag2,         /*TUNNEL_ID + SVLAN <-> SVLAN'*/
    s_c_tag_to_s_c_tag    /*TUNNEL_ID + SVLAN + CVLAN <-> SVLAN' + CVLAN*/
};

enum lif_type_e {
    match_stag,
    match_s_c_tag
};

struct pon_tunnel_mapping_info_s{
    int service_mode;
    int up_lif_type;
    bcm_tunnel_id_t tunnel_id;
    uint16 tunnel_profile_id;
    int tunnel_lookup_mode;
    bcm_vlan_t up_svlan;
    bcm_vlan_t up_cvlan;
    bcm_vlan_t up_svlan_high;
    int down_lif_type;
    bcm_vlan_t down_svlan;
    bcm_vlan_t down_cvlan;
    bcm_gport_t pon_pp_port;
    bcm_gport_t pon_gport;
    bcm_gport_t nni_gport;
};

bcm_port_t pon_port = 5;
bcm_port_t nni_port = 129;
int num_of_tunnel_mapping_service = 0;
int verbose = 0;

pon_tunnel_mapping_info_s pon_tunnel_mapping[7];

bcm_field_entry_t ent;    
bcm_field_group_t grp;

int pon_tunnel_qualify_setup(int unit, int port_pon, int tunnel_id, int vlan)
{
    bcm_error_t rv = BCM_E_NONE;
    bcm_field_qset_t  qset;
    bcm_field_aset_t  aset;

    BCM_FIELD_QSET_INIT(qset);
    BCM_FIELD_QSET_ADD(qset, bcmFieldQualifyTunnelId);
    BCM_FIELD_QSET_ADD(qset, bcmFieldQualifyInPort);
    BCM_FIELD_ASET_INIT(aset);
    BCM_FIELD_ASET_ADD(aset, bcmFieldActionOuterVlanNew);
    rv = bcm_field_group_create(unit, qset, BCM_FIELD_GROUP_PRIO_ANY, &grp);	
    if (rv != BCM_E_NONE)	
    {       
        printf("ERROR:: bcm_field_group_create returned %s\n", bcm_errmsg(rv));     
        return rv;  
    }  
    
    rv = bcm_field_group_action_set(unit, grp, aset);
    if (rv != BCM_E_NONE)
    {
        printf("ERROR: bcm_field_group_action_set returned %s\n", bcm_errmsg(rv));
        return rv;
    }
    
    rv = bcm_field_entry_create(unit, grp, &ent);
    if (rv != BCM_E_NONE)
    {
        printf("ERROR: bcm_field_entry_create returned %s\n", bcm_errmsg(rv));
        return rv;
    }

    rv = bcm_field_entry_prio_set(unit, ent, BCM_FIELD_ENTRY_PRIO_DONT_CARE);    
    if (rv != BCM_E_NONE)    
    {        
        printf("ERROR: bcm_field_entry_prio_set returned %s\n", bcm_errmsg(rv));  
        return FALSE;
    }
    
    bcm_field_qualify_InPort(unit, ent, port_pon, 0xffffffff); 
    if (rv != BCM_E_NONE)
    {
        printf("ERROR: bcm_field_qualify_InPort returned %s\n", bcm_errmsg(rv));
    }

    rv = bcm_field_qualify_TunnelId(unit, ent, tunnel_id, 0x7ff);   
    if (rv != BCM_E_NONE)    
    {        
        printf("ERROR: bcm_field_qualify_TunnelId tunnel_id returned %s\n", bcm_errmsg(rv));        
        return FALSE;    
    }
    
    rv = bcm_field_action_add(unit, ent, bcmFieldActionOuterVlanNew, vlan, 0);
    if (rv != BCM_E_NONE)
    {
        printf("ERROR: bcm_field_action_add returned %s\n", bcm_errmsg(rv));
    }
    
    bcm_field_entry_install(unit, ent);
    if (rv != BCM_E_NONE)
    {
        printf("ERROR: bcm_field_entry_install returned %s\n", bcm_errmsg(rv));
    }

    return rv;
}


int pon_tunnel_qualify_cleanup(int unit)
{
    bcm_error_t rv = BCM_E_NONE;    
    
    rv = bcm_field_entry_remove(unit, ent);
    if (rv != BCM_E_NONE)
    {
        printf("ERROR: bcm_field_entry_remove returned %s\n", bcm_errmsg(rv));
        return rv;
    }

    rv = bcm_field_entry_destroy(unit, ent);
    if (rv != BCM_E_NONE)
    {
        printf("ERROR: bcm_field_entry_destroy returned %s\n", bcm_errmsg(rv));
        return rv;
    }

    rv = bcm_field_group_destroy(unit, grp);
    if (rv != BCM_E_NONE)
    {
        printf("ERROR: bcm_field_group_destroy returned %s\n", bcm_errmsg(rv));
        return rv;
    }

    return rv;
}


int pon_tunnel_mapping_init(int unit, bcm_port_t port_pon, bcm_port_t port_nni)
{
    int rv = 0;
    int index;
    uint32 flags;
    bcm_gport_t port_tunnel_id;

    pon_port = port_pon;
    nni_port = port_nni;

    /* PON Port 5 Tunnel-ID 1000 SVLAN 20 <-CrossConnect-> NNI Port 129 SVLAN 200*/
    index = 0;
    pon_tunnel_mapping[index].service_mode = stag_to_stag2;
    pon_tunnel_mapping[index].tunnel_id  = 1000;
    pon_tunnel_mapping[index].tunnel_profile_id = 0;
    pon_tunnel_mapping[index].tunnel_lookup_mode = bcmVlanPortLookupTunnelEnable;
    pon_tunnel_mapping[index].up_lif_type = match_stag;
    pon_tunnel_mapping[index].up_svlan   = 20;
    pon_tunnel_mapping[index].down_lif_type = match_stag;
    pon_tunnel_mapping[index].down_svlan = 200;

    /* PON Port 5 Tunnel-ID 1000 SVLAN 20 CVLAN 21 <-CrossConnect-> NNI Port 129 SVLAN 20 CVLAN 21*/
    index++;
    pon_tunnel_mapping[index].service_mode = s_c_tag_to_s_c_tag;
    pon_tunnel_mapping[index].tunnel_id  = 1000;
    pon_tunnel_mapping[index].tunnel_profile_id = 0;    
    pon_tunnel_mapping[index].tunnel_lookup_mode = bcmVlanPortLookupTunnelEnable;
    pon_tunnel_mapping[index].up_lif_type = match_s_c_tag;
    pon_tunnel_mapping[index].up_svlan   = 20;
    pon_tunnel_mapping[index].up_cvlan   = 21;
    pon_tunnel_mapping[index].down_lif_type = match_s_c_tag;
    pon_tunnel_mapping[index].down_svlan = 20;
    pon_tunnel_mapping[index].down_cvlan = 21;

    /* PON Port 5 Tunnel-ID 1000 SVLAN range 50~60 <-CrossConnect-> NNI Port 129 SVLAN 70 */
    index++;
    pon_tunnel_mapping[index].service_mode = stag_to_stag2;
    pon_tunnel_mapping[index].tunnel_id  = 1000;
    pon_tunnel_mapping[index].tunnel_profile_id = 0;
    pon_tunnel_mapping[index].tunnel_lookup_mode = bcmVlanPortLookupTunnelEnable;
    pon_tunnel_mapping[index].up_lif_type = match_stag;
    pon_tunnel_mapping[index].up_svlan   = 50;
    pon_tunnel_mapping[index].up_svlan_high = 60;
    pon_tunnel_mapping[index].down_lif_type = match_stag;
    pon_tunnel_mapping[index].down_svlan = 70;

    /* PON Port 5 Tunnel-ID 1001 SVLAN 30 <-CrossConnect-> NNI Port 129 SVLAN 300*/
    index++;
    pon_tunnel_mapping[index].service_mode = stag_to_stag2;
    pon_tunnel_mapping[index].tunnel_id  = 1001;
    pon_tunnel_mapping[index].tunnel_profile_id = 1;    
    pon_tunnel_mapping[index].tunnel_lookup_mode = bcmVlanPortIgnoreInnerPktTag;
    pon_tunnel_mapping[index].up_lif_type = match_stag;
    pon_tunnel_mapping[index].up_svlan   = 30;
    pon_tunnel_mapping[index].down_lif_type = match_stag;
    pon_tunnel_mapping[index].down_svlan = 300;

    /* PON Port 5 Tunnel-ID 1001 SVLAN 30 CVLAN 31 <-CrossConnect-> NNI Port 129 SVLAN 30 CVLAN 31*/
    index++;
    pon_tunnel_mapping[index].service_mode = s_c_tag_to_s_c_tag;
    pon_tunnel_mapping[index].tunnel_id  = 1001;
    pon_tunnel_mapping[index].tunnel_profile_id = 1;
    pon_tunnel_mapping[index].tunnel_lookup_mode = bcmVlanPortIgnoreInnerPktTag;
    pon_tunnel_mapping[index].up_lif_type = match_s_c_tag;
    pon_tunnel_mapping[index].up_svlan   = 30;
    pon_tunnel_mapping[index].up_cvlan   = 31;
    pon_tunnel_mapping[index].down_lif_type = match_s_c_tag;
    pon_tunnel_mapping[index].down_svlan = 30;
    pon_tunnel_mapping[index].down_cvlan = 31;

    /* PON Port 5 Tunnel-ID 1001 SVLAN range 80~90 <-CrossConnect-> NNI Port 129 SVLAN 100 */
    index++;
    pon_tunnel_mapping[index].service_mode = stag_to_stag2;
    pon_tunnel_mapping[index].tunnel_id  = 1001;
    pon_tunnel_mapping[index].tunnel_profile_id = 1;
    pon_tunnel_mapping[index].tunnel_lookup_mode = bcmVlanPortIgnoreInnerPktTag;
    pon_tunnel_mapping[index].up_lif_type = match_stag;
    pon_tunnel_mapping[index].up_svlan = 80;
    pon_tunnel_mapping[index].up_svlan_high = 90;
    pon_tunnel_mapping[index].down_lif_type = match_stag;
    pon_tunnel_mapping[index].down_svlan = 100;

    /* PON Port 5 Tunnel-ID 1000 SVLAN 20 <-CrossConnect-> NNI Port 129 SVLAN 200*/
    index++;
    pon_tunnel_mapping[index].service_mode = stag_to_stag2;
    pon_tunnel_mapping[index].tunnel_id  = 1002;
    pon_tunnel_mapping[index].tunnel_profile_id = 0;
    pon_tunnel_mapping[index].tunnel_lookup_mode = bcmVlanPortLookupTunnelEnable;
    pon_tunnel_mapping[index].up_lif_type = match_stag;
    pon_tunnel_mapping[index].up_svlan   = 40;
    pon_tunnel_mapping[index].down_lif_type = match_stag;
    pon_tunnel_mapping[index].down_svlan = 400;

    num_of_tunnel_mapping_service = index+1;

    bcm_port_class_set(unit, pon_port, bcmPortClassId, pon_port);
    bcm_port_class_set(unit, nni_port, bcmPortClassId, nni_port);

    /* Port tunnel mapping */
    for (index = 0; index < num_of_tunnel_mapping_service; index++)
    {
        /* one tunnel_id configure once. */
        if ((index == 0) || (index == 3))
        {
            /* Add PON in PP port with profile id */
            flags = BCM_PORT_PON_TUNNEL_WITH_ID;
            port_tunnel_id = pon_tunnel_mapping[index].tunnel_profile_id;
            rv = bcm_port_pon_tunnel_add(unit, pon_port, flags, &port_tunnel_id);
            if (rv != BCM_E_NONE)
            {    
                printf("Error, bcm_port_pon_tunnel_add!\n");
                print rv;
                return rv;
            }

            /* Map tunnel_id to PON in pp Port */
            rv = bcm_port_pon_tunnel_map_set(unit, pon_port, pon_tunnel_mapping[index].tunnel_id, port_tunnel_id);
            if (rv != BCM_E_NONE)
            {    
                printf("Error, bcm_petra_port_pon_tunnel_map_set!\n");
                print rv;
                return rv;
            }

            /* Set TPIDs of PON in PP port. outer 0x8100, inner 0x9100 */
            port_tpid_init(port_tunnel_id, 1, 1);
            rv = port_tpid_set(unit);
            if (rv != BCM_E_NONE) {
                printf("Error, port_tpid_set\n");
                print rv;
                return rv;
            }
            
            bcm_port_class_set(unit, port_tunnel_id, bcmPortClassId, port_tunnel_id);
            
            /* Enable additional port tunnel lookup or ignore 2nd VLAN tag in PON ports */
            rv = bcm_vlan_control_port_set(unit, port_tunnel_id, pon_tunnel_mapping[index].tunnel_lookup_mode, 1);
            if (rv != BCM_E_NONE) {
                printf("Error, in bcm_vlan_control_port_set %s\n", pon_port, bcm_errmsg(rv));
            }
            pon_tunnel_mapping[index].pon_pp_port = port_tunnel_id;
        }
        else
        {
            pon_tunnel_mapping[index].pon_pp_port = pon_tunnel_mapping[index-1].pon_pp_port;
        }        
    }
    
    /* Set TPIDs of NNI port. outer 0x8100, inner 0x9100 */
    port_tpid_init(nni_port, 1, 1);
    rv = port_tpid_set(unit);
    if (rv != BCM_E_NONE) {
        printf("Error, port_tpid_set\n");
        print rv;
        return rv;
    }

    /* Remove Ports from VLAN 1 (Done by init application) */
    bcm_vlan_gport_delete_all(unit, 1);

    /* Disable membership in PON ports */
    rv = bcm_port_vlan_member_set(unit, pon_port, 0x0);
    if (rv != BCM_E_NONE) {
        printf("Error, bcm_port_vlan_member_set\n");
        print rv;
        return rv;
    }

    return 0;
}

/*
 * Create PON LIF according to the LIF type.
 */
int pon_lif_create(int unit, int lif_type, bcm_gport_t port,
    bcm_tunnel_id_t tunnel_id, bcm_vlan_t up_svlan, 
    bcm_vlan_t up_cvlan, bcm_gport_t *gport)
{
    int rv;
    int index;
    bcm_vlan_port_t vlan_port;

    bcm_vlan_port_t_init(&vlan_port);
    vlan_port.port = port;
    /* Set ingress vlan editor of PON LIF to do nothing, when creating PON LIF */
    vlan_port.flags |= (BCM_VLAN_PORT_OUTER_VLAN_PRESERVE | BCM_VLAN_PORT_INNER_VLAN_PRESERVE);
    vlan_port.match_tunnel_value  = tunnel_id;
    vlan_port.egress_tunnel_value = tunnel_id;

    switch(lif_type) {
        case match_stag:
            vlan_port.criteria = BCM_VLAN_PORT_MATCH_PORT_TUNNEL_VLAN;
            vlan_port.match_vlan = up_svlan;
            break;
        case match_s_c_tag:
            vlan_port.criteria = BCM_VLAN_PORT_MATCH_PORT_TUNNEL_VLAN_STACKED;
            vlan_port.match_vlan        = up_svlan;
            vlan_port.match_inner_vlan  = up_cvlan;
            break;
        default:
            printf("ERR: pon_lif_create INVALID PARAMETER lif_type %d\n", lif_type);
            return BCM_E_PARAM;
    }

    if(verbose) {
        printf("pon_lif created:\n");
        print vlan_port;
    }
    
    rv = bcm_vlan_port_create(unit, &vlan_port);
    if (rv != BCM_E_NONE)
    {
        printf("pon_lif_create index %d failed! %s\n", bcm_errmsg(rv));
    }

    *gport = vlan_port.vlan_port_id;

    if(verbose) {
        print vlan_port.vlan_port_id;
    }
  
    return rv;
}

/*
 * Create NNI LIF according to the LIF type.
 */
int nni_lif_create(int unit, int lif_type, int is_with_id, 
    bcm_tunnel_id_t tunnel_id, bcm_vlan_t down_svlan, bcm_vlan_t down_cvlan, 
    bcm_gport_t *gport, bcm_if_t *encap_id)
{
    int rv;
    int index;
    bcm_vlan_port_t vlan_port;
    bcm_vlan_action_set_t action;
    bcm_vlan_t vid;

    index = lif_type;
    
    bcm_vlan_port_t_init(&vlan_port);
    vlan_port.port = nni_port;
    if (is_with_id) /* Create NNI LIF with same LIF ID */
    {
        vlan_port.flags = BCM_VLAN_PORT_ENCAP_WITH_ID | BCM_VLAN_PORT_WITH_ID | 0x800 /* BCM_VLAN_PORT_MATCH_ONLY */;
        vlan_port.encap_id = *encap_id;
        vlan_port.vlan_port_id = *gport;
    }
    else
    {
        /* Set ingress vlan editor of NNI LIF to do nothing, when creating NNi LIF */
        vlan_port.flags |= (BCM_VLAN_PORT_OUTER_VLAN_PRESERVE | BCM_VLAN_PORT_INNER_VLAN_PRESERVE);
    }    

    switch(lif_type) {
        case match_stag:
            vlan_port.criteria = BCM_VLAN_PORT_MATCH_PORT_VLAN;
            vlan_port.match_vlan  = down_svlan;
            break;
        case match_s_c_tag:
            vlan_port.criteria = BCM_VLAN_PORT_MATCH_PORT_VLAN_STACKED;
            vlan_port.match_vlan        = down_svlan;
            vlan_port.match_inner_vlan  = down_cvlan;
            break;
        default:
            printf("ERR: nni_lif_create INVALID PARAMETER lif_type %d\n", lif_type);
            return BCM_E_PARAM;
    }

    if(verbose) {
        printf("nni_lif created:\n");
        print vlan_port;
    }

    rv = bcm_vlan_port_create(unit, &vlan_port);
    if (rv != BCM_E_NONE)
    {
        printf("nni_lif_create index failed! %s\n", bcm_errmsg(rv));
    }

    *gport = vlan_port.vlan_port_id;

    if(verbose) {
        print vlan_port.vlan_port_id;
    }

    /* Set egress vlan editor of NNI LIF to do nothing, when creating NNi LIF */
    bcm_vlan_action_set_t_init(&action);
    action.dt_outer = bcmVlanActionNone;
    action.dt_inner = bcmVlanActionNone;
    action.ot_outer = bcmVlanActionNone;
    action.it_outer = bcmVlanActionNone;
    rv = bcm_vlan_translate_egress_action_add(unit, vlan_port.vlan_port_id, BCM_VLAN_NONE, BCM_VLAN_NONE, &action);
    if (rv != BCM_E_NONE) {
      printf("Error, in bcm_vlan_translate_egress_action_add %s\n", bcm_errmsg(rv));
      return rv;
    }

    return rv;
}

/*
 * Set up the ingress VLAN editor of PON LIF according to service type.
 * Translate svlan or cvlan.
 */
int pon_port_ingress_vt_set(int unit, int service_type, bcm_tunnel_id_t tunnel_id, bcm_vlan_t down_svlan, bcm_vlan_t down_cvlan, bcm_gport_t pon_gport)
{
    int rv;
    bcm_vlan_action_set_t action;
    bcm_vlan_action_set_t_init(&action);

    switch (service_type)
    {
    case stag_to_stag2:
        /* PON LIF ingress action: replace outer tag. */
        action.ot_outer = bcmVlanActionReplace;
        action.dt_outer = bcmVlanActionReplace;
        action.new_outer_vlan = down_svlan;        
        break;

    case s_c_tag_to_s_c_tag:
        /* PON LIF ingress action: do nothing. */
        action.dt_outer = bcmVlanActionNone;
        action.dt_inner = bcmVlanActionNone;
        break;

    default:
        return BCM_E_PARAM;
    }

    rv = bcm_vlan_translate_action_create(unit, pon_gport, bcmVlanTranslateKeyPortOuter, BCM_VLAN_INVALID, BCM_VLAN_NONE, &action);
    if (rv != BCM_E_NONE) {
      printf("Error, in bcm_vlan_translate_action_create %s\n", bcm_errmsg(rv));
      return rv;
    }

    return rv;
}

/*
 * Set up the egress VLAN editor of PON LIF according to service type.
 * Add tunnel_ID and translate svlan or cvlan.
 */
int pon_port_egress_vt_set(int unit, int service_type, bcm_tunnel_id_t tunnel_id, 
    bcm_vlan_t up_svlan, bcm_vlan_t up_cvlan, bcm_gport_t pon_gport)
{
    int rv;
    bcm_vlan_action_set_t action;
    bcm_vlan_action_set_t_init(&action);

    switch (service_type)
    {
    case stag_to_stag2:
        /* PON LIF Egress action: replace outer tag and add inner tag. */
        action.ot_outer = bcmVlanActionReplace;
        action.ot_inner = bcmVlanActionAdd;
        action.new_outer_vlan = tunnel_id;
        action.new_inner_vlan = up_svlan;
        break;

    case s_c_tag_to_s_c_tag:
        /* PON LIF Egress action: add tunnel_ID. */
        action.dt_outer = bcmVlanActionAdd;
        action.new_outer_vlan = tunnel_id;
        break;

    default:
        return BCM_E_PARAM;
    }

    rv = bcm_vlan_translate_egress_action_add(unit, pon_gport, BCM_VLAN_NONE, BCM_VLAN_NONE, &action);
    if (rv != BCM_E_NONE) {
      printf("Error, in bcm_vlan_translate_egress_action_add %s\n", bcm_errmsg(rv));
      return rv;
    }

    return rv;
}

/*
 * Set up 1:1 sercies, using port cross connect.
 */
int pon_tunnel_mapping_1_1_service(int unit)
{
    int rv;
    int index, is_with_id = 0;
    bcm_vswitch_cross_connect_t gports;
    int service_mode;
    bcm_tunnel_id_t tunnel_id;
    bcm_vlan_t up_svlan;
    bcm_vlan_t up_svlan_high;
    bcm_vlan_t up_cvlan;
    bcm_vlan_t down_svlan;
    bcm_vlan_t down_cvlan;
    bcm_if_t encap_id;
    bcm_gport_t pon_gport, nni_gport, pon_pp_port;
    int pon_lif_type, nni_lif_type;    
    bcm_vlan_action_set_t action;

    for (index = 0; index < num_of_tunnel_mapping_service; index++)
    {
        pon_gport = 0;
        nni_gport = 0;
        service_mode  = pon_tunnel_mapping[index].service_mode;
        tunnel_id     = pon_tunnel_mapping[index].tunnel_id;
        pon_lif_type  = pon_tunnel_mapping[index].up_lif_type;
        up_svlan      = pon_tunnel_mapping[index].up_svlan;
        up_svlan_high = pon_tunnel_mapping[index].up_svlan_high;
        up_cvlan      = pon_tunnel_mapping[index].up_cvlan;
        nni_lif_type  = pon_tunnel_mapping[index].down_lif_type;
        down_svlan    = pon_tunnel_mapping[index].down_svlan;
        down_cvlan    = pon_tunnel_mapping[index].down_cvlan;
        pon_pp_port   = pon_tunnel_mapping[index].pon_pp_port;

        /* Set different VLAN ranges for each tunnel id */
        if (up_svlan_high != 0)
        {
            action.ot_outer = bcmVlanActionCompressed;
            action.new_outer_vlan = up_svlan;
            rv = bcm_vlan_translate_action_range_add(unit, pon_tunnel_mapping[index].pon_pp_port, up_svlan, up_svlan_high, BCM_VLAN_INVALID, BCM_VLAN_INVALID, &action);
            if (rv != BCM_E_NONE)
            {
                printf("error, bcm_vlan_translate_action_range_add!\n");
                print rv;
                return rv;
            }
        }

        /* Create PON LIF */        
        pon_lif_create(unit, pon_lif_type, pon_pp_port, tunnel_id, up_svlan, up_cvlan, &pon_gport);

        /* Set PON LIF ingress VLAN editor */
        pon_port_ingress_vt_set(unit, service_mode, tunnel_id, down_svlan, down_cvlan, pon_gport);

        /* Set PON LIF egress VLAN editor */
        pon_port_egress_vt_set(unit, service_mode, tunnel_id, up_svlan, up_cvlan, pon_gport);

        /* Create NNI LIF */
        nni_lif_create(unit, nni_lif_type, is_with_id, tunnel_id, down_svlan, down_cvlan, &nni_gport, &encap_id);

        pon_tunnel_mapping[index].pon_gport = pon_gport;
        pon_tunnel_mapping[index].nni_gport = nni_gport;

        /* Cross connect the 2 LIFs */
        bcm_vswitch_cross_connect_t_init(&gports);

        gports.port1 = pon_gport;
        gports.port2 = nni_gport;
        rv = bcm_vswitch_cross_connect_add(unit, &gports);
        if (rv != BCM_E_NONE) {
            printf("Error, bcm_vswitch_cross_connect_add\n");
            print rv;
            return rv;
        }
    }

    return rv;
}

/*
 * clean up the configurations of 1:1 sercies.
 */
int pon_tunnel_mapping_1_1_service_cleanup(int unit)
{
    int rv;
    int index;    
    bcm_vlan_t up_svlan;
    bcm_vlan_t up_svlan_high;
    bcm_vswitch_cross_connect_t gports;

    for (index = 0; index < num_of_tunnel_mapping_service; index++)
    {   
        /* Delete the cross connected LIFs */
        gports.port1 = pon_tunnel_mapping[index].pon_gport;
        gports.port2 = pon_tunnel_mapping[index].nni_gport;
        rv = bcm_vswitch_cross_connect_delete(unit, &gports);
        if (rv != BCM_E_NONE) {
            printf("Error, bcm_vswitch_cross_connect_delete\n");
            print rv;
            return rv;
        }
        
        /* Delete different VLAN ranges for each tunnel id */
        up_svlan      = pon_tunnel_mapping[index].up_svlan;
        up_svlan_high = pon_tunnel_mapping[index].up_svlan_high;
        if (up_svlan_high != 0)
        {
            rv = bcm_vlan_translate_action_range_delete(unit, pon_tunnel_mapping[index].pon_pp_port, up_svlan, up_svlan_high, BCM_VLAN_INVALID, BCM_VLAN_INVALID);
            if (rv != BCM_E_NONE)
            {
                printf("error, bcm_vlan_translate_action_range_delete!\n");
                print rv;
                return rv;
            }
        }

        /* Delete PON LIF */
        rv = bcm_vlan_port_destroy(unit, pon_tunnel_mapping[index].pon_gport);
        if (rv != BCM_E_NONE) {
            printf("Error, bcm_vlan_port_destroy \n");
            return rv;
        }

        /* Delete NNI LIF */
        rv = bcm_vlan_port_destroy(unit, pon_tunnel_mapping[index].nni_gport);
        if (rv != BCM_E_NONE) {
            printf("Error, bcm_vlan_port_destroy \n");
            return rv;
        }
    }

    return rv;        
}

